#!/usr/bin/python

# ========================================================
# VFD Modular Clock - Raspberry PI Editiion
# 
# (C) 2013 Akafugu Corporation
# (C) 2013 William B Phelps - wm@usa.net
#
#
# This program is free software; you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.  See the GNU General Public License for more details.
#

# 
# systemload.py
# Shows the current CPU load of the system
# Requires https://code.google.com/p/psutil/
#

import psutil
import RPi.GPIO as GPIO
import sched, signal, time
from datetime import datetime
from vfdspi import *

print "milliseconds.py"
print "Simple example that shows the current time with millisecond resolution"
print "Requires IV-18 shield or other 8-digit or bigger shield"

setBrt(10) # max brightness
setScroll(0)

def handleSigTERM(signum, frame):
	print "kill signal"
	sayBye()
def handleCtrlC(signum, frame):
	print "Ctrl-C"
	sayBye()

signal.signal(signal.SIGTERM, handleSigTERM)
signal.signal(signal.SIGINT, handleCtrlC)

if getDigits() <= 4:
	offset = 0
else:
	offset = 3

# main loop
while(True):
	ld = psutil.cpu_percent(interval=0)
	print ("%d" % ld)
	if (ld != 0):
		clear()
		display(offset, "%d" % ld)
	time.sleep(0.5)
