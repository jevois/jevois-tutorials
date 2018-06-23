#!/usr/bin/python

# Needed packages: sudo apt install python-serial

# This tutorial is a simple program that allows one to read and parse serial messages from JeVois

serdev = '/dev/ttyACM0' # serial device of JeVois

import serial
import time

with serial.Serial(serdev, 115200, timeout=1) as ser:
    while 1:
        # Read a whole line and strip any trailing line ending character:
        line = ser.readline().rstrip()
        print "received: {}".format(line)

        # Split the line into tokens:
        tok = line.split()

        # Skip if timeout or malformed line:
        if len(tok) < 1: continue

        # Skip if not a standardized "Normal 2D" message:
        # See http://jevois.org/doc/UserSerialStyle.html
        if tok[0] != 'N2': continue

        # From now on, we hence expect: N2 id x y w h
        if len(tok) != 6: continue

        # Assign some named Python variables to the tokens:
        key, id, x, y, w, h = tok

        print "Found ArUco {} at ({},{}) size {}x{}".format(id, x, y, w, h)
        
