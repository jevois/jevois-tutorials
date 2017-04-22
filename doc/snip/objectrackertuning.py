#!/usr/bin/python

# Needed packages: sudo apt-get install python python-tk

# This tutorial is a simple program that allows one to adjust the hue, saturation, and value ranges of the ObjectTracker
# module using sliders

serdev = '/dev/ttyACM0' # serial device of JeVois

from Tkinter import *
import serial
import time


# default values for Hue, Saturation, and Value ranges:
hmin = 95
hmax = 110
smin = 100
smax = 255
vmin = 60
vmax = 253

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
def update_hmin(val):
    global hmin
    global hmax
    hmin = val
    send_command('setpar hrange {0}...{1}'.format(hmin, hmax))
    
####################################################################################################
def update_hmax(val):
    global hmin
    global hmax
    hmax = val
    send_command('setpar hrange {0}...{1}'.format(hmin, hmax))
    
####################################################################################################
def update_smin(val):
    global smin
    global smax
    smin = val
    send_command('setpar srange {0}...{1}'.format(smin, smax))
    
####################################################################################################
def update_smax(val):
    global smin
    global smax
    smax = val
    send_command('setpar srange {0}...{1}'.format(smin, smax))

####################################################################################################
def update_vmin(val):
    global vmin
    global vmax
    vmin = val
    send_command('setpar vrange {0}...{1}'.format(vmin, vmax))
    
####################################################################################################
def update_vmax(val):
    global vmin
    global vmax
    vmax = val
    send_command('setpar vrange {0}...{1}'.format(vmin, vmax))
    
####################################################################################################
# Main code
ser = serial.Serial(serdev, 115200, timeout=1)
send_command('ping')                   # should return ALIVE

master = Tk()

w1 = Label(master, text = "Hue min")
w1.pack()
w2 = Scale(master, from_=0, to=255, tickinterval=32, length=600, orient=HORIZONTAL, command=update_hmin)
w2.set(hmin)
w2.pack()

w3 = Label(master, text = "Hue max")
w3.pack()
w4 = Scale(master, from_=0, to=255, tickinterval=32, length=600, orient=HORIZONTAL, command=update_hmax)
w4.set(hmax)
w4.pack()

w5 = Label(master, text = "Saturation min")
w5.pack()
w6 = Scale(master, from_=0, to=255, tickinterval=32, length=600, orient=HORIZONTAL, command=update_smin)
w6.set(smin)
w6.pack()

w7 = Label(master, text = "Saturation max")
w7.pack()
w8 = Scale(master, from_=0, to=255, tickinterval=32, length=600, orient=HORIZONTAL, command=update_smax)
w8.set(smax)
w8.pack()

w9 = Label(master, text = "Value min")
w9.pack()
w10 = Scale(master, from_=0, to=255, tickinterval=32, length=600, orient=HORIZONTAL, command=update_vmin)
w10.set(vmin)
w10.pack()

w11 = Label(master, text = "Value max")
w11.pack()
w12 = Scale(master, from_=0, to=255, tickinterval=32, length=600, orient=HORIZONTAL, command=update_vmax)
w12.set(vmax)
w12.pack()

mainloop()

