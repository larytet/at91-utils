#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""AT91 loader.

Usage:
  at91_loader.py -h | --help
  at91_loader.py --version
  at91_loader.py [--device=<STR>] -i | --interactive  
  at91_loader.py run [--device=<STR>] [--filename=<STR>] [--address=<HEX>] [--interactive]  
  at91_loader.py dump [--device=<STR>] --address=<HEX> [--size=<INT>]  [--interactive] 
  at91_loader.py read [--device=<STR>] --address=<HEX> [--interactive]
  at91_loader.py write [--device=<STR>] --address=<HEX> --data=<HEX> [--interactive]

Options:
  -h --help            Show this screen.
  --version            Show version.
  -d --device=<STR>    Serial device [default: /dev/ttyUSB0]
  -a --address=<HEX>   Address where dump memory starts or code is loaded [default: 308000]
  -f --filename=<STR>  BIN file for execution [default: ./applets/mk/firmware.bin]
  -s --size=<INT>      Size of the dump [default: 256]
  --data=<HEX>         Data to write  
  -i --interactive     Interactive mode
"""

import cmd
from docopt import docopt
import logging
from collections import namedtuple
from time import sleep
import threading
import subprocess
import serial, random, time

import sys, traceback
import os
import re



NamedListener = namedtuple("NamedListener", ['name', 'callback'])


def convertToInt(s, base):
    value = None;
    try:
        value = int(s, base);
        result = True;
    except:
        logger.error("Bad formed number '{0}'".format(s));
        result = False;
    return (result, value);

def convertToFloat(s):
    value = None;
    try:
        value = float(s)
        result = True;
    except:
        logger.error("Bad formed number '{0}'".format(s));
        result = False;
    return (result, value);

 
 
def openFile(filename, flag):
    '''
    Open a file for reading or writing
    Returns handle to the open file and result code False/True
    '''
    try:
        fileHandle = open(filename, flag) # read text file
    except Exception:
        logger.error('Failed to open file {0}'.format(filename))
        print sys.exc_info()
        return (False, None)
    else:
        return (True, fileHandle)
    

def buildhexstring(value, width=0, prefix=''):
    valueStr = hex(value)
    valueStr = valueStr.lstrip("0x")
    valueStr = valueStr.rstrip("L")
    valueStr = valueStr.upper();
    if (width > 0):
        valueStr = valueStr.zfill(width)   # add zeros the left

    valueStr = prefix + valueStr

    return valueStr

def converBinToInt(d):
    result = (ord(d[0]) << 0) | (ord(d[1]) << 8) | (ord(d[2]) << 16) | (ord(d[3]) << 24)
    return result


class StatManager:
    '''
    A single place where references to all blocks off debug counters is stored
    '''
    def __init__(self):
        self.groups = {}

    class Block:
        def __init__(self, name):
            '''
            @param name is a name of the block. 
            Useful when there are many instances of the same set of counters. 
            For example "eth0", "eth1"
            '''
            self.name = name
            self.ignoreFields = []
            
            #  All fields added so far are in the ignore list
            for fieldName in self.__dict__:
                self.ignoreFields.append(fieldName)

        
        def addField(self, (name, initialValue)):
            '''
            Add a field with specified name
            @param name is a name of the counter, for example "tx"
            '''
            self.__dict__[name] = initialValue

        def addFields(self, fields):
            '''
            @param fields list of tuples (field name, iniitial value)
            '''
            for f in fields:
                self.addField(f)

        def addFieldsInt(self, fields):
            '''
            @param fields list of field names
            '''
            for f in fields:
                self.addField((f, 0))
            
    def addCounters(self, groupName, block):
        '''
        Add a block of counters to the specified group
        @param groupName is a name of the group, for example "Network traffic"
        @param block is an object of type Block
        '''
        if (not groupName in self.groups):
            self.groups[groupName] = []
        group = self.groups[groupName]
        group.append(block) 

    def __isPrintableField(self, block, fieldName):
        result = fieldName in block.ignoreFields
        return (not result)
        
    def printGroup(self, groupName):
        '''
        Print counters from the specified by name group 
        '''
        counters = self.groups[groupName]
        if (len(counters) <= 0):
            return
        fieldPattern = "{:>14}"
            
        # Print column names
        print fieldPattern.format(groupName),
        o = counters[0]
        for fieldName in o.__dict__:
            if (self.__isPrintableField(o, fieldName)):
                print fieldPattern.format(fieldName),
        print

        separatorLength = 14
        separator = ""
        while (separatorLength > 0):
            separator = separator + "-"
            separatorLength = separatorLength - 1
             
        fields = len(o.__dict__)  + 1 - len(o.ignoreFields)
        while (fields > 0):
            fields = fields - 1
            print separator,
        print
        
        # Print table data
        for counter in counters:
            # Print the name of the counter block
            print fieldPattern.format(counter.name),
            for fieldName in counter.__dict__:
                if (self.__isPrintableField(counter, fieldName)):
                    print fieldPattern.format(counter.__dict__[fieldName]),
            print    
        
    def printAll(self):
        '''
        Print counters from all registered groups
        '''
        for groupName in self.groups:
            self.printGroup(groupName)
            print
            

statManager = StatManager()

class Sound:
    def __init__(self):
        self.lock = threading.Lock()
        self.__canceled = False
        self.__disabled = False
        self.__running = False
        
    def cancel(self):
        self.__canceled = True

    def disable(self):
        self.__disabled = True

    def playSoundThread(self, args):
        '''
        Sometime ALSA returns error 
        /usr/bin/play WARN alsa: under-run
        '''
        
        if (self.__canceled or self.__disabled or self.__running):
            return
        
        self.lock.acquire()
        
        self.__running = True
        
        for _ in range(10):
            if (self.__canceled or self.__disabled):
                break
            os.system("/usr/bin/play --no-show-progress --null --channels 1 synth 0.05 sine 400")
            
        self.__running = False
        
        self.lock.release()

    def playSound(self):
        '''
        Execute system 'beep' in the backgorund
        '''
        self.__canceled = False
        threading.Thread(target=self.playSoundThread, args=([None])).start()    

beepSound = Sound()
 
class cmdGroundLevel(cmd.Cmd):
    '''
    Interactive command line interface 
    '''
    
    def init(self, at91):
        '''
        I can not subclass old style class
        '''
        self.at91 = at91
        self.cmdLoop = CmdLoop(at91)

        (self.dumpAddressStr, self.dumpSizeStr) = ('0x308000', '256')
        (self.runFilename, self.runAddressStr) = ('./applets/mk/firmware.bin', '308000')
        (self.writeAddressStr, self.writeValueStr) = ('0x308000', '11AE3255')
        self.readAddressStr = '0x308000'
        (self.flashAppletFilename, self.flashRunAddressStr, self.flashImage, self.flashAddressStr) = ('./applets/mk/firmware.bin', '0x308000', 'sama5d3xek-nandflashboot-uboot-3.6.0.bin', "0")
        (self.cmdCommand, self.cmdPayload) = "0x03", ""
    
    
    def emptyline(self):
        '''
        If empty line repeat last sent command, unless this is run
        '''
        lastcmd = self.lastcmd
        if (lastcmd.startswith("run")):
            pass
        if (lastcmd.startswith("command")):
            pass
        elif (lastcmd != ""):
            self.onecmd(lastcmd.strip())
        else:
            pass

    def precmd(self, line):
        '''
        Handle simple scripts - single line which contains commands separated by ';'
        '''
        if (";" in line):
            commands = line.split(";")
            for command in commands:
                if (command != ""):
                    self.onecmd(command)
            return "none"
        else:
            return line
              
    def do_none(self, line):
        pass

    def do_sleep(self, line):
        (result, t) = convertToFloat(line)
        if (result):
            time.sleep(t)
        else:
            self.help_sleep()
        
    def help_sleep(self):
        print "Delay execution"
        print "Usage:sleep secs"
        print "Example:sleep 0.1"
        
      
    def do_status(self, line):
        
        # Get status
        if (self.at91.isConnected):
            isConnectedStr = "up"
        else:
            isConnectedStr = "down"
            
        # Print status
        print "Connection: {0}".format(isConnectedStr)
        if (line == 'full'):
            print "Dump default args: addrress={0} size={1}".format(self.dumpAddressStr, self.dumpSizeStr)
            print "Run default args: filename={0} address={1}".format(self.runFilename, self.runAddressStr)
            print "Write default args: addrress={0} value={1}".format(self.writeAddressStr, self.writeValueStr)
            print "Read default args: addrress={0}".format(self.readAddressStr)
            print "Flash default args: applet={0}, image={1}, addrress={2}".format(self.flashAppletFilename, self.flashRunAddressStr, self.flashImage, self.flashAddressStr)
            print "Command default args: command {0}, payload={1}".format(self.cmdCommand, self.cmdPayload)
            
            
        
    def help_status(self):
        print "Print systems status, like last commands, last used address, connection state"
        print "Usage:status [brief|full]"
    
    def do_dump(self, line):
        words = line.split()
        if (len(words) == 2):
            result = printDump(self.at91, words[0], words[1])
            if (result):
                (self.dumpAddressStr, self.dumpSizeStr) = (words[0], words[1])
        elif (len(words) == 1): 
            result = printDump(self.at91, words[0], self.dumpSizeStr)
            if (result):
                self.dumpAddressStr = words[0]
        elif (len(words) == 0): 
            printDump(self.at91, self.dumpAddressStr, self.dumpSizeStr)
        else:
            self.help_dump()
        print        
    
    def help_dump(self):
        print "Dump memory"
        print "Usage:dump [address=<HEX>] [size=<INT>]"
        print "Default args: addrress={0} size={1}".format(self.dumpAddressStr, self.dumpSizeStr)
        
    def do_statistics(self, line):
        statManager.printAll()

    def help_statistics(self):
        print "Print debug statistics"

    def do_run(self, line):
        words = line.split()
        if (len(words) == 2):
            result = executeCode(at91, words[0], words[1])
            if (result):
                (self.runFilename, self.runAddressStr) = (words[0], words[1])
        elif (len(words) == 0): 
            executeCode(self.at91, self.runFilename, self.runAddressStr)
        else:
            self.help_run()

        waitOuput(at91)
        #self.at91.connectionPollEnable(True)

    def help_run(self):
        print "Load and run code"
        print "Usage:run [fileaname=<STR>] [address=<HEX>]"
        print "Default args: filename={0} address={1}".format(self.runFilename, self.runAddressStr)

    def do_flash(self, line):
        words = line.split()
        while (True):
            if (len(words) == 0):
                result = executeCode(at91, self.flashAppletFilename, self.flashRunAddressStr)
                waitOuput(at91)
                if (not result):
                    break
                
                self.cmdLoop.sendCommand(self.cmdLoop.CMD_EXIT)
                waitOuput(at91)
            break
                
        self.at91.connectionPollEnable(True)

        #  Load applet
        #  Load image
        #  Send erase flash 
        #  Write data 
        
    def help_flash(self):
        print "Program serial flash with BIN file"
        print "Usage:flash [applet=<STR>] [appletAddress=<HEX>] [imageFile=<STR>] [flashAddress=<HEX>]"
        print "Default args: applet={0} appletAddress={1} image={2} flashAddress={3}".format(self.flashAppletFilename, self.flashImage, self.flashAddressStr)

    def do_command(self, line):
        self.at91.connectionPollEnable(False)
        
        words = line.split()
        if (len(words) == 0):
            (result, cmdCommand) = convertToInt(self.cmdCommand, 16)
            if (not result): return; 
            self.cmdLoop.sendCommand(cmdCommand)
        if (len(words) == 1):
            (result, cmdCommand) = convertToInt(words[0], 16)
            if (not result): return; 
            self.cmdLoop.sendCommand(cmdCommand)
            self.cmdCommand = words[0]
            
        waitOuput(at91, 1.0)
        self.at91.connectionPollEnable(True)
        
        
    def help_command(self):
        print "Send command to the applet"
        print "Usage:command  command=<HEX>"
        print "Default args: command={0} payload='{1}'".format(self.cmdCommand, self.cmdPayload)
        
    def do_read(self, line):
        words = line.split()
        if (len(words) == 1):
            result = readMemory(self.at91, words[0])
            if (result):
                self.readAddressStr = words[0]
        elif (len(words) == 0): 
            readMemory(self.at91, self.readAddressStr)
        else:
            self.help_read()

    def help_read(self):
        print "Usage:read [address=<HEX>]"
        print "Default args: addrress={0}".format(self.readAddressStr)

    def do_write(self, line):
        words = line.split()
        if (len(words) == 2):
            result = writeData(self.at91, words[0], words[1])
            if (result):
                (self.writeAddressStr, self.writeValueStr) = (words[0], words[1])
        elif (len(words) == 0): 
            writeData(self.at91, self.writeAddressStr, self.writeValueStr)
        else:
            self.help_write()
        print        
        

    def help_write(self):
        print "Write memory"
        print "Usage:write [address=<HEX>] [value=<HEX>]"
        print "Default args: address={0} value={1}".format(self.writeAddressStr, self.writeValueStr)

    def do_exit(self, line):
        self.closeAll()
        
    def do_quit(self, line):
        self.closeAll()
    BEEP_COMMANDS_TEST = ['test', 'start']
        
    BEEP_COMMANDS_CANCEL = ['cancel', 'exit', 'stop']
    BEEP_COMMANDS_COMPLETION = ['cancel', 'test']
    def do_beep(self, command):
        '''
        beep [cancel|test]
        '''        
        if ((command in self.BEEP_COMMANDS_TEST) or (not command)):
            beepSound.playSound()
        elif (command in self.BEEP_COMMANDS_CANCEL):
            beepSound.cancel()
        
    def complete_beep(self, text, line, begidx, endidx):
        if (not text):
            completions = self.BEEP_COMMANDS_COMPLETION[:]
        else:
            completions = []
            for f in self.BEEP_COMMANDS_COMPLETION:
                if (f.startswith(text)):
                    completions.append(f)

        return completions

    def help_beep(self):
        print "Test system beep"
        print "Usage: beep [test|start|cancel|exit|stop]"

    def closeAll(self):
        beepSound.disable()
        at91.cancel()
        return exit(0)
    

class SerialConnection:
    
    
    def __init__(self, device, rate):
        tty = serial.Serial();
        tty.port = device;
        tty.baudrate=rate;
        tty.bytesize=serial.EIGHTBITS;
        tty.parity=serial.PARITY_NONE;
        tty.stopbits=serial.STOPBITS_ONE;
        tty.timeout=0.01;
        tty.writeTimeout=0;
        tty.xonxoff=False;
        tty.rtscts=False;
        tty.dsrdtr=False;
        self.tty = tty
        self.device = device
        self.stat = StatManager.Block("")
        self.stat.addFieldsInt(["rx", "tx", "rxFailed", "txFailed", "flushed"])
        statManager.addCounters("SerialConnection", self.stat)

    def reset(self):
        tty = self.tty
        result = False

        if (os.name != 'nt'):
            try:
                isOpen = tty.isOpen();
                rateOrig = tty.getBaudrate();
                if (isOpen):                 # if open close first - I can not (?) set rate for the opened I/O in Python
                    tty.close();

                tty.setBaudrate(0)
                tty.open();
                time.sleep(0.05);              # minicom waits 1s
                tty.close();

                tty.setBaudrate(rateOrig)     # i am going to leave the serial in the original rate and state
                if (isOpen):
                    tty.open();
                result = True
            except Exception:
                pass
                # traceback.print_exc();
                # logger.error("Failed to toggle baud rate for the I/O")
                
            return result

    def connect(self):
        while (True):
            result = False;
            try:
                self.reset()
                self.tty.open()
                result = True;
            except serial.SerialException:
                break;
            
            break;
            
        return (result);

    def disconnect(self):
        tty = self.tty
        isOpen = tty.isOpen();
        if (isOpen):
            try:
                tty.close()
            except serial.SerialException:
                traceback.print_exc();

    def flush(self):
        try:
            self.tty.flush();
        except Exception:
            traceback.print_exc();

    def write(self, data):
        '''
        write data to the tty
        @param data:string to write 
        '''
        tty = self.tty
        result = False
        if (not tty.isOpen()):
            return False
        try:
            tty.flush();
            tty.write(data);
            self.stat.tx = self.stat.tx + 1
            result = True
        except Exception:
            self.stat.txFailed = self.stat.txFailed + 1
            #tty.flush();
            #traceback.print_exc();
            
        return result

    def write2(self, data):
        '''
        write data to the tty
        @param data:string to write 
        '''
        tty = self.tty
        result = False
        if (not tty.isOpen()):
            return False
        try:
            tty.write(data);
        except Exception:
            pass
            #tty.flush();
            #traceback.print_exc();
            
        return result

    def read(self, expectedCount):
        '''
        read all characters, similar to flush, buit i wait for timeout
        '''
        
        tty = self.tty
        s = ''
        result = False
        try:
            s = tty.read(expectedCount)
            self.stat.rx = self.stat.rx + 1
            result = True
        except Exception:
            self.stat.rxFailed = self.stat.rxFailed + 1
            #logger.error("Failed to read TTY")
            #traceback.print_exc();
    
        return (result, s)
    
    def swFlush(self):
        count = 0
        s = ""
        while (True):   
            (result, data) = self.read(2)
            if (not result):
                break
            if (data == None):
                break
            if (data == ""):
                break
            if (len(data) == 0):
                break
            count = count + len(data)
            s = s + data
            
        #if (count > 0):
        #    logger.error("Flush {0} bytes '{1}'".format(count, s))
        self.stat.flushed = self.stat.flushed + count
        
        return count
        

    def name(self):
        return self.device

class CmdLoop:
    '''
    Methods to send command to the applet.
    Command contains command IP, command size, optional payload and checksum
    '''
    CMD_PING            = 0x03
    CMD_EXIT            = 0x04

    def __init__(self, at91):
        self.at91 = at91 
    
    def calculateChecksum(self, data):
        '''
        Command loop of the applet requires simple checksum in the end 
        of the message 
        '''
        sum = 0;
        for b in data:
            sum = sum + (b & 0xFF);
            sum = sum & 0xFF
        sum = (~sum)+1
        sum = sum & 0xFF
        return sum;
    
    def buildCommand(self, command, payload):
        '''
        Concatenate command, payload and checksum
        '''
        if (payload != None):
            command = [command] + [len(payload)+1] + payload;
        else:
            command = [command] + [0x01];
        cs = self.calculateChecksum(command)
        command = command + [cs];
        
        return command;
    
    def sendCommand(self, command, payload=None):
        '''
        @param command:CMD_PING, CMD_EXIT, ... 
        '''
        command = self.buildCommand(command, payload)
        s = "".join(map(chr, command))
        self.at91.getTTY().write2(s)
        s = ""
        for c in command:
            s = s + "{0}".format(buildhexstring(c, 2)) + " "
        logger.info("Sent command {}".format(s))
    
    
class AT91(threading.Thread):
    
    '''
    Keep connection alive, polls the connection while idle and makes sure that SAM-BA still responds 
    '''        
    def __init__(self, device):
        super(AT91, self).__init__()
        self.lock = threading.Lock()
        self.exitFlag = False
        self.isConnected = False
        self.skipConnectionPoll = False
        self.tty = SerialConnection(device, 115200)
        self.stat = StatManager.Block("")
        self.stat.addFieldsInt(["dump", "read", "write", "failedRead", "failedWrite", "executeCode", "checkFailed", "check", "initOk", "init4", "initBadRsp", "initNoRsp"])
        statManager.addCounters("AT91", self.stat)
        
    def run(self):
        '''
        Check conneciton periodically
        '''
        while (not self.exitFlag):
            
            if (not self.skipConnectionPoll):
                self.stat.check = self.stat.check + 1
            
                self.lock.acquire()
                self.__checkConnection()
                self.lock.release()
            
            time.sleep(0.05)
            
    def __checkConnection(self):
        isConnected = self.__isConnected()
        if (not isConnected):
            self.stat.checkFailed = self.stat.checkFailed + 1
            self.tty.connect()
        self.__updateConnectionStatus(isConnected)

    def connectionPollEnable(self, enable):
        '''
        Allows to disable conneciton polling 
        For example if waiting for output from the applet
        '''
        self.skipConnectionPoll = not enable

    def getOutput(self, expectedLength=80):
        self.lock.acquire()
        (result, s) = self.tty.read(expectedLength)
        self.lock.release()
        
        return (result, s)

    def sendData(self, data):
        self.lock.acquire()
        result = self.tty.write(data)
        self.lock.release()
        
        return result
        
    def waitConnection(self, timeout=1.0):
        loopsTotal = 10
        loops = 0
        self.isConnected = False 
        while (True):

            if (self.isConnected):
                break
            
            time.sleep(timeout/loopsTotal)
            loops = loops + 1
            if (loops > loopsTotal):
                break;
            
        return self.isConnected
        
    def __isConnected(self):
        while (True):
            INIT_COMMAND = "N#"
            result = self.tty.write(INIT_COMMAND)
            if (not result):
                self.stat.failedWrite = self.stat.failedWrite + 1 
                break
            
            result = False
            (result, s) = self.tty.read(4)
            if (not result):
                self.stat.failedRead = self.stat.failedRead + 1 
                break
            
            result = False
            if (s == ""):
                self.stat.initNoRsp = self.stat.initNoRsp + 1 
                break
            
            result = False

            s1 = ""
            for c in s:
                s1 = s1 + "{0}".format(buildhexstring(ord(c), 2)) + " "
                
            if (not "\n\r" in s):
                logger.error("Read unexpected data '{0}'".format(s1));
                break
            
            if (len(s) == 2):
                self.stat.initOk = self.stat.initOk + 1
            elif (len(s) == 4):
                self.stat.initOk = self.stat.init4 + 1
            else:
                self.stat.initBadRsp = self.stat.initBadRsp + 1
                logger.error("Read unexpected data '{0}'".format(s1));
                

            result = True
            break

        self.tty.swFlush()
        
        return result;
        
    def __updateConnectionStatus(self, isConnected):
        if (self.isConnected != isConnected):
            self.isConnected = isConnected
            self.__printConnectionStatus()

    def __printConnectionStatus(self):
        if (self.isConnected):
            status = "up"
        else:
            status = "down"
        logger.info("Connection {0} is {1}".format(self.tty.name(), status));

        
            
    def cancel(self):
        self.exitFlag = True
        
    def executeCode(self, address, entryPoint, code, timeout=0.0):
        '''
        Load binary code to the specified location, execute, wait for completion
        '''
        s2 = "G{0}#".format(buildhexstring(address))
        
        self.stat.executeCode = self.stat.executeCode + 1
        self.lock.acquire()
        
        # copy the code 
        words = len(code)/4
        index  = 0
        while (words > 0):
            
            data = code[index:index+4]
            value = converBinToInt(data)
            self.__write(address, value)

            address = address + 4
            words = words - 1
            index = index + 4
        
        self.tty.swFlush()
        # execute the code
        self.tty.write(s2)
        
        if (timeout > 0):
            time.sleep(timeout)
        
        self.lock.release()

    def dump(self, address, size):
        '''
        Read memory from the device
        '''
          
        self.stat.dump = self.stat.dump + 1
        words = size/4
        
        data = ""
        self.lock.acquire()
        
        while (words > 0):
            (result, d) = self.__read(address)
            words = words - 1
            address = address + 4
            if (result and (len(d)> 0)):
                data = data + d
            else:
                break
        
        self.tty.swFlush()
        self.lock.release()
        
        return (result, data)

    def getTTY(self):
        return self.tty
    
    def __write(self, address, data):
        '''
        Write memory
        '''
        s = "W{0},{1}#".format(buildhexstring(address), buildhexstring(data))
        
        self.tty.write(s)
        
        
        return True

    
    def write(self, address, data):
        '''
        Write memory
        '''
        self.stat.write = self.stat.write + 1
        
        self.lock.acquire()
        
        result = self.__write(address, data)
        
        self.tty.swFlush()
        self.lock.release()

        return result

    def __read(self, address):
        '''
        Read memory from the device
        '''
        s = "w{0},4#".format(buildhexstring(address))
          
        self.tty.write(s)
        (result, data) = self.tty.read(4)
        
        return (result, data)

    def read(self, address):
        '''
        Read memory from the device
        '''
        self.stat.read = self.stat.read + 1

        self.lock.acquire()

        (result, data) = self.__read(address)        
        
        self.tty.swFlush()
        
        self.lock.release()
        
        return (result, data)

def binToHex(data):
    result = ""
    for d in data:
        ordD = ord(d)
        s = buildhexstring(ordD, 2)
        result = result + s
        
    return result
    
def readMemory(at91, addressStr):
    (result, addressInt) = convertToInt(addressStr, 16)    
    if (not result):
       logger.error("Address '{0}' is not valid hexadecimal integer".format(addressStr))
       return result
   
    (result, data) = at91.read(addressInt)
    
    if (result and (len(data) > 0)):
        s = binToHex(data)
        print s
    else:
       logger.error("Failed to read result={0}, len={1}".format(result, len(data)))
         
    
    return (result, data)

def printDump(at91, addressStr, sizeStr):
    
    (result, addressInt) = convertToInt(addressStr, 16)    
    if (not result):
       logger.error("Address '{0}' is not valid hexadecimal integer".format(addressStr))
       return result
    
    (result, sizeInt) = convertToInt(sizeStr, 10)
    if (not result):
       logger.error("Size '{0}' is not valid integer".format(sizeStr))
       return result

    blockSize = 128
    count = 0
    blocks = 0
    totalBytes = sizeInt 
    print "{0}: ".format(buildhexstring(addressInt, 8)),
    asciiLine = ""
    while (sizeInt > 0):    
        if (sizeInt > blockSize):
            (result, data) = at91.dump(addressInt+blocks*blockSize, blockSize)
        else:
            (result, data) = at91.dump(addressInt+blocks*blockSize, sizeInt)
        if (not result):
            logger.error("Failed to dump {0}".format(buildhexstring(addressInt, 8)))
            return 
        sizeInt = sizeInt - blockSize
        blocks = blocks + 1
    
        for d in data:
            ordD = ord(d)
            s = buildhexstring(ordD, 2)
            if ((ordD >= 0x20) and (ordD <= 0x7f)):
                asciiLine = asciiLine + d
            else:
                asciiLine = asciiLine + "."
            print s,
            count = count + 1
            if ((count % 16 == 0) and (count < totalBytes)):
                print " {0}".format(asciiLine)
                asciiLine = ""
                print "{0}: ".format(buildhexstring(addressInt+count, 8)),
    print " {0}".format(asciiLine)
                
    return True

def executeCode(at91, filename, addressStr, timeout=0.1):
    file = None

    (result, addressInt) = convertToInt(addressStr, 16)    
    if (not result):
       logger.error("Address '{0}' is not valid hexadecimal integer".format(addressStr))
       return
    
    while (True):
        (result, file) = openFile(filename, "rb")
        if (not result):
            logger.error("Failed to open file '{0}' for reading".format(filename))
            break;

        fileSize = os.path.getsize(filename)
        if (fileSize <= 0):
            logger.error("Failed to get size of the file '{0}'".format(filename))
            break;
   
        data = file.read()
        if (len(data) != fileSize):
            logger.error("Read {0} bytes instead of {1} from file {2}".format(len(data), fileSize, filename))
            break;
    
        logger.info("Load Code {0}".format(filename))
        at91.connectionPollEnable(False)

        at91.executeCode(addressInt, addressInt, data)
        logger.info("Code {0} is running, wait for output".format(filename))

        waitOuput(at91, timeout)

        break
    
    if (file != None):
        file.close()

def waitOuput(at91, timeout=1):
    LOOPS = 5
    loops = LOOPS
    
    # Force floating point calculation
    loopDelay = (1.0*timeout)/loops

    while (loops > 0):
        (result, s) = at91.getOutput(80)
        if (result and (len(s) > 0)):
            logger.info(">{}".format(s))
            loops = LOOPS
            
        loops = loops - 1
        time.sleep(loopDelay) 
        
def writeData(at91, addressStr, valueStr):
    
    (result, addressInt) = convertToInt(addressStr, 16)    
    if (not result):
       logger.error("Address '{0}' is not valid hexadecimal integer".format(addressStr))
       return result
    
    (result, valueInt) = convertToInt(valueStr, 16)
    if (not result):
       logger.error("Value '{0}' is not valid hexadecimal integer".format(valueStr))
       return result

    at91.write(addressInt, valueInt)
   
    return True 
    
if __name__ == '__main__':
    arguments = docopt(__doc__, version='AT91 loader 0.1')

    logging.basicConfig()    
    logger = logging.getLogger('at91_loader')
    logger.setLevel(logging.INFO)    

    device = arguments['--device']

    
    logger.info("Use device {0}".format(device));
    at91 = AT91(device)
    at91.start()
    
    at91.waitConnection()
        
    if (arguments['dump']):
        printDump(at91, arguments['--address'], arguments['--size'])
        
    if (arguments['run']):
        executeCode(at91, arguments['--filename'], arguments['--address'])
        self.at91.connectionPollEnable(True)

    if (arguments['write']):
        writeData(at91, arguments['--address'], arguments['--data'])
        
    # Enter main command loop if interactive mode is enabled
    if (arguments['--interactive']):
        print "" 
        c = cmdGroundLevel()
        c.init(at91)
        c.cmdloop()
    else:
        at91.cancel()
