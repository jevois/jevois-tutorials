#!/usr/bin/python

# Needed packages: sudo apt-get install python python-tk

# This tutorial is a simple program that allows one to play with two low-level camera sensor parameters that relate to
# correction for lens sensitivity fall-off with eccentricity.

serdev = '/dev/ttyACM0' # serial device of JeVois

from Tkinter import *
import serial
import time

####################################################################################################
# Send a command to JeVois and show response
def send_command(cmd):
    print "HOST>> " + cmd
    ser.write(cmd + '\n')
    out = ''
    time.sleep(0.1)
    while ser.inWaiting() > 0:
        out += ser.read(1)
    if out != '':
        print "JEVOIS>> " + out, # the final comma suppresses extra newline, since JeVois already sends one

####################################################################################################
# Callback when radius slider is moved
def update_radius(val):
    send_command('setcamreg 0x65 {}'.format(val))
    
####################################################################################################
# Callback when factor slider is moved
def update_factor(val):
    send_command('setcamreg 0x64 {}'.format(val))
    
####################################################################################################
# Main code
ser = serial.Serial(serdev, 115200, timeout=1)
send_command('ping')                   # should return ALIVE
send_command('setpar camreg true')     # enable low-level access to camera sensor registers
send_command('setcamreg 0x66 1')       # enable lens correction

master = Tk()

w1l = Label(master, text = "Radius")
w1l.pack()

w1 = Scale(master, from_=0, to=255, tickinterval=32, length=600, orient=HORIZONTAL, command=update_radius)
w1.set(0x80)
w1.pack()

w2l = Label(master, text = "Correction factor")
w2l.pack()

w2 = Scale(master, from_=0, to=255, tickinterval=32, length=600, orient=HORIZONTAL, command=update_factor)
w2.set(0x10)
w2.pack()

mainloop()

