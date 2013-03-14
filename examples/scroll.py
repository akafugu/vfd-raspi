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
# scroll.py
# Shows a simple scrolling message
#

import RPi.GPIO as GPIO
import sched, signal, time
from datetime import datetime
from vfdspi import *

print "scroll.py"
print "Shows a simple scrolling message"

setBrt(10) # max brightness
setDots(0)
clear()
setScroll(1)

def handleSigTERM(signum, frame):
	print "kill signal"
	sayBye()
def handleCtrlC(signum, frame):
	print "Ctrl-C"
	sayBye()

signal.signal(signal.SIGTERM, handleSigTERM)
signal.signal(signal.SIGINT, handleCtrlC)

msg = "This is a long scrolling message that seemingly goes on forever and ever        "

# main loop
while(True):
	scroll(0, msg, 0, 0.4)
