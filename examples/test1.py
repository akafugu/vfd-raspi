#!/usr/bin/python

import RPi.GPIO as GPIO
import sched, time, datetime, signal
import spidev

# ===========================================================================
# SPI Clock Example using hardware SPI with SPIDEV
# 16 February 2013
# William B Phelps - wm@usa.net
# ===========================================================================

#CE0  = 8
#MISO = 9
#MOSI = 10
#SCLK = 11
S1 = 22
S2 = 27
S3 = 17  # alarm on/off

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)

GPIO.setup(S1, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(S2, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(S3, GPIO.IN, pull_up_down=GPIO.PUD_UP)

spi = spidev.SpiDev()
spi.open(0,0)
spi.mode = 0
#spi.max_speed_hz = 250000
#spi.max_speed_hz = 10000

def SPI(b):
	b1 = [b]
	b2 = spi.xfer(b1)
	time.sleep(0.001)
def clear():
	SPI(0x82)
def SPIwrite(str):
	for i in range(len(str)):
		SPI(ord(str[i]))
def setNum(n):
	SPI(0x88)
	SPI(n & 0xFF)
	SPI(n >> 8)
def setDots(d):
	SPI(0x85)  # dots
	SPI(d)
def setPos(n):
	SPI(0x89)  # position
	SPI(n)
def setBrt(b):
	SPI(0x80)  # brightness
	SPI(b)
def setDot(b):
	SPI(0x86)  # dot flag
	SPI(b)
def setDash(b):
	SPI(0x87)  # dash flag
	SPI(b)

print "Raspi VFD test1"

clear()
SPIwrite("01234567")
#setDots(0b11111111)
time.sleep(1)
for b in range(0,10):
	setBrt(b)
	time.sleep(0.2)
#setDots(0b00110011)
#time.sleep(1)
#setDots(0b01010101)
clear()

#timeString  = time.strftime("%I%M")
#print timeString

# Globals
S1state = False
S2state = False
S3state = False
saveTime = datetime.datetime.now()

def chkButtons():
	global S1push, S2push, menuState, menuTime
	global S1state, S2state, S3state
	S1push = False
	S2push = False
	st1 = not GPIO.input(S1)
	st2 = not GPIO.input(S2)
	st3 = GPIO.input(S3)
	if (st1 != S1state):
		menuTime = time.time()
		if (st1):
			S1push = True
			setDash(1)
		else:
			setDash(0)
		S1state = st1
	if (st2 != S2state):
		menuTime = time.time()
		if (st2):
			S2push = True
			setDot(1)
		else:
			setDot(0)
		S2state = st2
	if (st3 != S3state):
		menuTime = time.time()
		if (st3):
#			S3push = True
			print "S3 on"
			setDot(1)
		else:
			print "S3 off"
			setDot(0)
		S3state = st3

g_alarm = False
g_bright = 10

menuNames = ['', 'alarm   ', 'bright  ', 'end     ']
menuState = 0
actionState = 0
menuTime = time.time

def menuAlarm():
	global menuState
def menuBright():
	global menuState
def menuEnd():
	global menuState, actionState
	time.sleep(0.5)
	menuState = 0
	actionState = 0
menu = { 1:menuAlarm, 2:menuBright, 3:menuEnd }

def doMenu():
	global menuState, menuTime
	menuState += 1
	setDots(0)
	setPos(0)
	SPIwrite(menuNames[menuState])
	menu[menuState]()
	
def setAlarm(update):
	global g_alarm
	
def setBright(update):
	global g_bright, menuState, actionState
	if (update):  # not first time here?
		g_bright = (g_bright+1) if (g_bright<10) else 0
	s = '{:>8}'.format(g_bright)
	setPos(0)
	SPIwrite(s)
	setBrt(g_bright)
	
def setEnd(update):
	s = 0  # python nop?
	
action = { 1:setAlarm, 2:setBright, 3:setEnd}

def doAction():  # S2 pushed
	global actionState, menuState, menuTime
	if (menuState > 0):
		if (actionState == menuState):
			action[menuState](True)
		else:
			actionState = menuState
			action[menuState](False)
	else:
		showDate()

def showDate():
	setPos(0)
	now = datetime.datetime.now()
	timestr = '{:%y-%m-%d}'.format(now)
	SPIwrite(timestr)
	# Toggle dots during date display
	if (now.second % 2):
		setDots(0b00000001)
	else:
		setDots(0b00000000)

def showTime(now):
	global saveTime
	setPos(0)
	if (now.second != saveTime.second):  # run once a second
		saveTime = now
		if (now.second >= 57) and (now.second <= 59):  # show date
			showDate()
		else:
			timestr = '{:  %I%M%S}'.format(now)
			SPIwrite(timestr)
			# Toggle dots
			if (now.second % 2):
				setDots(0b00010101)
			else:
				setDots(0b00010100)
		# adjust brightness according to time
		if (now.second == 0):  # once a minute
			setBrt(2)  # dim briefly to show top of minute
			time.sleep(0.1)
			if (now.hour < 7) or (now.hour >= 22):
				setBrt(2)  # dim for night time
			elif (now.hour >= 18):
				setBrt(7)  # dim slightly
			else:
				setBrt(10)  # max bright

def sayBye():
	clear()
	SPIwrite("  bye  ")
	time.sleep(2.0)
	SPI(0x82)  # clear
	exit(0)

def handleSigTERM(signum, frame):
	print "kill signal"
	sayBye()
def handleCtrlC(signum, frame):
	print "Ctrl-C"
	sayBye()

signal.signal(signal.SIGTERM, handleSigTERM)
signal.signal(signal.SIGINT, handleCtrlC)

# main loop - check buttons, do menu, display time
while(True):
	now = datetime.datetime.now()
	chkButtons()
	if (S1push):
		doMenu()
	elif (S2push):
		doAction()
	elif (S2state):
		showDate()
	elif (menuState > 0):
		if (time.time() - menuTime > 2):
			menuEnd()  # menu timed out
	else:  # not in menu, show the time
		showTime(now)
	# Wait 100 ms
	time.sleep(0.1)
