#!/usr/bin/python

# ===========================================================================
# SPI Clock Example using hardware SPI with SPIDEV - https://github.com/doceme/py-spidev
# William B Phelps - wm@usa.net
# created 16 February 2013
# updated 25 February 2013 - rewrite piezo/beep
# ===========================================================================

import RPi.GPIO as GPIO
import sched, signal, time
from datetime import datetime
from alarmTime import alarmTime
from vfdspi import *

#CE0  = 8
#MISO = 9
#MOSI = 10
#SCLK = 11

S1 = 22
S2 = 27
S3 = 17  # alarm on/off

alarmTime = alarmTime(7*60)
alarmSet = False # on if setting alarm
alarmEnabled = False # alarm switch in On position
alarmOn = False # is alarm beeping?

digits = getDigits() # size of display
shield = getShield() # shield type

GPIO.setmode(GPIO.BCM)

if hasattr(GPIO, 'setwarnings'):
	GPIO.setwarnings(False)

GPIO.setup(S1, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(S2, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(S3, GPIO.IN, pull_up_down=GPIO.PUD_UP)
	
print "Raspi VFD test1"
print "shield " + getShieldStr() + ", digits " + str(digits)
print "volume " + str(getVol()) + ", bright " + str(getBrt())

# test beep interaction with SPI
clear()
setScroll(0)
beep(440, 100, False)
display(0," 440")
time.sleep(0.2)
beep(880, 100, False)
display(0," 880")
time.sleep(0.2)
beep(440, 100, False)
display(0," 440")
time.sleep(1)

clear()
scroll(0,"0123456789", 0, 0.2)
#setDots(0b11111111)
time.sleep(1)
for b in range(0,10):
	setBrt(b)
	time.sleep(0.2)
#setDots(0b00110011)
#time.sleep(1)
#setDots(0b01010101)
clear()

#timeString = time.strftime("%I%M")
#print timeString

# Globals
S1state = False
S2state = False
S3state = False
lastShowTime = datetime.now()

def chkButtons():
	global menuState, menuTime
	global S1push, S2push, S1state, S2state, S3state
	S1push = False
	S2push = False
	st1 = not GPIO.input(S1)
	if (st1 != S1state):
		menuTime = time.time()
		if (st1):
			S1push = True
			tick()
		S1state = st1
	st2 = not GPIO.input(S2)
	if (st2 != S2state):
		menuTime = time.time()
		if (st2):
			S2push = True
			tick()
		S2state = st2
	st3 = GPIO.input(S3)
	if (st3 != S3state):
		menuTime = time.time()
		if (st3):
			alarmEnable(True)
		else:
			alarmEnable(False)
		S3state = st3

menuNames4 = ['', 'alrm', 'brt ', 'vol ', 'end ']
menuNames6 = ['', 'alarm ', 'bright', 'volume', 'end   ']
menuNames8 = ['', 'alarm   ', 'bright  ', 'volume  ', 'end     ']
menuState = 0
actionState = 0
menuTime = time.time

def menuEnd():
	global menuState, actionState, alarmSet
	time.sleep(0.5)
	menuState = 0
	actionState = 0
	alarmSet = False

def doMenu():
	global menuState, menuTime
	menuState += 1
	if (digits == 4):
		display(0,menuNames4[menuState])
	else:
		setDots(0b00000000)
		display(0,menuNames8[menuState])
	if (menuState == len(menuNames4)-1):
		menuEnd()

def showAlarm():
	if (digits == 4):
		timestr = '{:>4}'.format(alarmTime)
		display(0,timestr)
	else:
		timestr = '{:>8}'.format(alarmTime)
		display(0,timestr)
		setDots(0b00000100)
	
def actAlarm(update):
	global alarmSet, alarmSetCnt, alarmSpeed
	alarmSetCnt = 0 # reset counter
	alarmSpeed = 1 # reset speed
	alarmSet = True
	showAlarm()
	
def actBright(update):
	bright = getBrt()
	if (update): # not first time here?
		bright = (bright+1) if (bright<10) else 0
	s = '{:>4}'.format(bright)
	display(0,s)
	setBrt(bright)
def actVol(update):
	volume = getVol()
	if (update): # not first time here
		volume = (volume+1) if (volume < 10) else 0
		setVol(volume)
	s = '{:>4}'.format(volume)
	display(0,s)
	beep(440, 100)
def actEnd(update):
	s = 0 # python nop?
  
action = { 1:actAlarm, 2:actBright, 3:actVol, 4:actEnd}

def doAction():  # S2 pushed
	global actionState, menuState
	if (menuState > 0):
		if (actionState == menuState):
			action[menuState](True)
		else:
			actionState = menuState
			action[menuState](False)
	else:
		showDate()

def showDate():
	now = datetime.now()
	timestr = '{:%y-%m-%d}'.format(now) # date
	if (digits == 4):
		scroll(0,"  " + timestr,2, 0.3)
		timestr = '{:%I%M}'.format(now) # time
		scroll(0,timestr, 0, 0.3) # scroll time back in
	else:
		display(0,timestr)
		# Toggle dots during date display
		if (now.second % 2):
			setDots(0b00000001)
		else:
			setDots(0b00000000)

def showTime(now):
	global lastShowTime
	if (now.second != lastShowTime.second): # run once a second
		lastShowTime = now
		if (now.second >= 57) and (now.second <= 59): # show date
			showDate()
		else:
			if (digits == 4):
				#timestr = '{:%I%M}'.format(now)
				timestr = "{:%I%M}".format(now).lstrip('0').rjust(4)
				display(0,timestr)
			else:
				#timestr = '{:  %I%M%S}'.format(now)
				timestr = "{:%I%M%S}".format(now).lstrip('0').rjust(8)
				display(0,timestr)
				# Toggle dots
				if (now.second % 2):
					setDots(0b00010101)
				else:
					setDots(0b00010100)
				setIV18Dot(now.hour > 11)
		# adjust brightness according to time
		if (now.second == 0):  # once a minute
			setBrt(2) # dim briefly to show top of minute
			time.sleep(0.1)
			if (now.hour < 7) or (now.hour >= 22):
				setBrt(2) # dim for night time
			elif (now.hour >= 18):
				setBrt(7) # dim slightly
			else:
				setBrt(10) # max bright

def sayBye():
	clear()
	if (digits == 4):
		display(0, "bye ")
	else:
		display(0,"  bye  ")
	time.sleep(1.0)
	clear() # clear
	exit(0)

alarmSetCnt = 0
alarmSpeed = 1
def setSpeed():
	global alarmSetCnt, alarmSpeed
	alarmSetCnt += 1
	if alarmSetCnt > 30: # button held for 3 seconds
		alarmSpeed = 60 # set by minutes

def alarmEnable(set):
	global alarmEnabled
	alarmEnabled = set
	setIV18Dash(set)
	if (alarmEnabled):
		showAlarm()
		time.sleep(1)
	else:
		if (shield == 1):
			scroll(0,"alrm off")
		else:
			setDots(0b00000000)
			display(0,"alrm off")
		time.sleep(1)
	
def setAlarm():
	global S1state, S2state, alarmTime, menuTime, alarmSetCnt, alarmSpeed
	if (S1state):
		setSpeed()
		alarmTime.time = alarmTime.time - alarmSpeed
		menuTime = time.time()
	elif (S2state):
		setSpeed()
		alarmTime.time = alarmTime.time + alarmSpeed
		menuTime = time.time()
	else: # buttons up, reset speed
		alarmSetCnt = 0
		alarmSpeed = 1
	showAlarm()
	
def checkAlarm(now):
	global alarmTime
	if alarmEnabled:
		if (now.hour == alarmTime.hour) and (now.minute == alarmTime.minute):
			alarmOn = True
			beep(880,200)
		else:
			alarmOn = False

def handleSigTERM(signum, frame):
	print "kill signal"
	sayBye()
def handleCtrlC(signum, frame):
	print "Ctrl-C"
	sayBye()

signal.signal(signal.SIGTERM, handleSigTERM)
signal.signal(signal.SIGINT, handleCtrlC)

then = datetime.now()
# main loop - check buttons, do menu, display time
while(True):
	chkButtons()
	if (alarmSet):
		if (time.time() - menuTime > 2):
			menuEnd() # menu timed out
		else:
			setAlarm()
	elif (S1push):
		doMenu()
	elif (S2push):
		doAction()
	elif (menuState > 0):
		if (time.time() - menuTime > 2):
			menuEnd() # menu timed out
	else: # not in menu, show the time
		now = datetime.now()
		if (now.second != then.second): # reduce SPI traffic
			checkAlarm(now) # must come after date/time display (beep bug?) ???
			then = now # now & then are one
			if (S2state):
				showDate()
			else:
				showTime(now)
	# Wait 100 ms
	time.sleep(0.1)
