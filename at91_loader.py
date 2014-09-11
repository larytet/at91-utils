#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""AT91 loader.

Usage:
  at91_loader.py -h | --help
  at91_loader.py --version
  at91_loader.py  -i | --interactive
  at91_loader.py run [--device=<STR>] [--filename=<STR>] [--address=<HEX>] [--interactive]  
  at91_loader.py dump [--device=<STR>] --address=<HEX> [--size=<INT>]  [--interactive] 
  at91_loader.py read [--device=<STR>] --address=<HEX> [--interactive]
  at91_loader.py write [--device=<STR>] --address=<HEX> (--data=<HEX> | --value=<HEX>) [--interactive]

Options:
  -h --help            Show this screen.
  --version            Show version.
  -d --device=<STR>    Serial device [default: /dev/ttyACM0]
  -a --address=<HEX>   Address where dump memory starts or code is loaded [default: 308000]
  -f --filename=<STR>  BIN file for execution [default: ./applets/mk/firmware.bin]
  -s --size=<INT>         Size of the dump [default: 256]
  --data=<HEX>         Data to write  
  -v --value=<HEX>       Same as --data  
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

import sys
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

# Open a file for reading or writing
# Returns handle to the open file and result code False/True
def openFile(filename, flag):
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
        (self.dumpAddressStr, self.dumpSizeStr) = ('0x308000', '256')
        (self.runFilename, self.runAddressStr) = ('./applets/mk/firmware.bin', '308000')
    
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
            print "Run default args: addrress={0} size={1}".format(self.runFilename, self.runAddressStr)
            
        
    def help_status(self):
        print "Print systems status, like last commands, last used address, connection state"
        print "Usage:status [brief|full]"
    
    def do_dump(self, line):
        words = line.split()
        if (len(words) == 2):
            result = printDump(self.at91, words[0], words[1])
            if (result):
                (self.dumpAddressStr, self.dumpSizeStr) = (words[0], words[1])
        elif (len(words) == 0): 
            printDump(self.at91, self.dumpAddressStr, self.dumpSizeStr)
        else:
            self.help_dump()
        print        
    
    def help_dump(self):
        print "Dump memory"
        print "Usage:dump [address=<HEX>] [size=<INT>]"
        print "Default args: addrress={0} size={1}".format(self.dumpAddressStr, self.dumpSizeStr)

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

    def help_run(self):
        print "Load and run code"
        print "Usage:dump [fileaname=<STR>] [address=<HEX>]"
        print "Default args: filename={0} address={1}".format(self.runFilename, self.runAddressStr)
        
    def do_read(self, line):
        print "Read memory"

    def do_write(self, line):
        print "Write memory"

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
        tty.timeout=0.1;
        tty.writeTimeout=0;
        tty.xonxoff=False;
        tty.rtscts=False;
        tty.dsrdtr=False;
        self.tty = tty
        self.device = device

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
        write data represented to the tty
        '''
        tty = self.tty
        result = False
        if (not tty.isOpen()):
            return False
        try:
            tty.flush();
            tty.write(data);
            result = True
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
            result = True
        except Exception:
            logger.error("Failed to read TTY")
            #traceback.print_exc();
    
        return (result, s)

    def name(self):
        return self.device


class AT91(threading.Thread):
    
    '''
    Keep connection alive, polls the connection while idle and makes sure that SAM-BA still responds 
    '''        
    INIT_COMMAND = "N#"
    def __init__(self, device):
        super(AT91, self).__init__()
        self.lock = threading.Lock()
        self.exitFlag = False
        self.isConnected = False
        self.tty = SerialConnection(device, 115200)
        
    def run(self):
        '''
        Overloaded Thread.run
        '''
        while (not self.exitFlag):
            
            self.lock.acquire()
            self.__checkConnection()
            self.lock.release()
            
            time.sleep(0.5)

    def __checkConnection(self):
        isConnected = self.__isConnected()
        if (not isConnected):
            self.tty.connect()
        self.__updateConnectionStatus(isConnected)

    def waitConnection(self, timeout=1.0):
        loopsTotal = 10
        loops = 0
        while (not self.isConnected):
            self.lock.acquire()
            self.__checkConnection()
            self.lock.release()
            
            time.sleep(timeout/loopsTotal)
            loops = loops + 1
            if (loops > loopsTotal):
                break;
            
        return self.isConnected
        
    def __isConnected(self):
        while (True):
            result = self.tty.write(AT91.INIT_COMMAND)
            if (not result):
                break
            
            result = False
            (result, s) = self.tty.read(2)
            if (not result):
                break
            
            result = False
            if (s == ""):
                break
            
            result = False
            if (s != "\n\r"):
                logger.error("Read unexpected data {0}".format(s));
                break

            result = True
            break
        
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
        
    def executeCode(self, address, entryPoint, code, timeout=0.2):
        '''
        Load binary code to the specified location, execute, wait for completion
        '''
        self.lock.acquire()
        
        # copy the code 
        s = "S{0},{1}#".format(buildhexstring(address), buildhexstring(len(code)))  
        self.tty.write(s)
        self.tty.write(code)
        
        # execute the code
        s = "G{0}#".format(buildhexstring(address))  
        self.tty.write(s)
        
        time.sleep(timeout)
        
        self.lock.release()

    def dump(self, address, size):
        '''
        Read memory from the device
        '''
        s = "R{0},{1}#".format(buildhexstring(address), buildhexstring(size))  
        self.lock.acquire()
        self.tty.write(s)
        (result, data) = self.tty.read(size)
        self.lock.release()
        
        return (result, data)
    
    def write(self, address, data):
        '''
        Write memory
        '''
        self.lock.acquire()
        self.lock.release()

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
            s = buildhexstring(ord(d), 2)
            print s,
            count = count + 1
            if ((count % 16 == 0) and (count < totalBytes)):
                print
                print "{0}: ".format(buildhexstring(addressInt+count, 8)),
                
    return True

def executeCode(at91, filename, addressStr):
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
    
        at91.executeCode(addressInt, addressInt, data)    
        break
    
    if (file != None):
        file.close()
        
        
    
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
        
    # Enter main command loop if interactive mode is enabled
    if (arguments['--interactive']):
        print "" 
        c = cmdGroundLevel()
        c.init(at91)
        c.cmdloop()
    else:
        at91.cancel()
