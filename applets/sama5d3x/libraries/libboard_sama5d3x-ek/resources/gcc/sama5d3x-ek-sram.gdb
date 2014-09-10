#------------------------------------------------
# MCK initialization script for the sam9xx5
#------------------------------------------------
# Step1: Connect to the J-Link gdb server
define reset
target remote localhost:2331
monitor reset

# Step2: Load file(eg. getting-started project)
load

# toggle remap bits
set *0xFFFFE100 = 0x03

#mon reg pc=0x300000
#info reg

end
