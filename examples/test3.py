#!/usr/bin/python

import RPi.GPIO as GPIO
import sched, signal, time
from datetime import datetime
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

IV18seg0 = 0 # iv-18 segment 0 bits

class alarmTime(object): # class for alarm time, handles minute/hour wrap
	def __init__(self, time):
		self.time = time
	@property
	def time(self):
		return self.__dict__['time']
	@property
	def hour(self):
		return int(self.__dict__['time']/60)
	@property
	def minute(self):
		return self.__dict__['time']%60
	@time.setter
	def time(self, value):
		self.__dict__['time'] = value%1440
	@hour.setter
	def hour(self, value):
		self.__dict__['time'] = value*60 + self.time%60
	@minute.setter
	def minute(self, value):
		self.__dict__['time'] = value%60 + int(self.time/60)*60
	def __str__(self):
		return '{:02}{:02}'.format(self.hour, self.minute)

alarmTime = alarmTime(7*60)
alarmSet = False # on if setting alarm
alarmEnabled = False # alarm switch in On position
alarmOn = False # is alarm beeping?

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)

GPIO.setup(S1, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(S2, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(S3, GPIO.IN, pull_up_down=GPIO.PUD_UP)

spi = spidev.SpiDev()
spi.open(0,0)
spi.mode = 0
spi.max_speed_hz = 200000 # 200khz

def SPI(b):
	b1 = [b]
	b2 = spi.xfer(b1)
	time.sleep(0.001)  # delay needed to keep SPI in sync
	return b2[0]

def clear():
	SPI(0x81)
def sync():
	SPI(0x80)
def getBrt():
	SPI(0x82)  # brightness
	b = SPI(255) # get current value
	return b
def setBrt(b):
	SPI(0x82)  # brightness
	SPI(b)
def SPIwrite(str):
	for i in range(len(str)):
		SPI(ord(str[i]))
def getDigits():
	SPI(0x8B) # request digits
	b = SPI(0)
	return b
def getShield():
	SPI(0x8C) # request shield
	b = SPI(0)
	return b
def setDots(d):
	SPI(0x85)  # dots
	SPI(d)
def setPos(n):
	SPI(0x89)  # position
	SPI(n)
def setIV18Dot(b):
	global IV18seg0
	if (b>0):
		IV18seg0 = 0b10000000 | IV18seg0
	else:
		IV18seg0 = 0b01111111 & IV18seg0
	SPI(0x86)  # dot flag
	SPI(IV18seg0)
def setIV18Dash(b):
	global IV18seg0
	if (b>0):
		IV18seg0 = 0b01000000 | IV18seg0
	else:
		IV18seg0 = 0b10111111 & IV18seg0
	SPI(0x86)  # dash flag
	SPI(IV18seg0)
def getVol():
	SPI(0x90)  # volume
	b = SPI(255)  # get current value
	return(b)
def setVol(b):
	SPI(0x90)  # volume
	SPI(b)  # set new value
def beep(b, c):
	SPI(0x91) # beep
	SPI(b/10) # max 2550 hz
	SPI(c/10) # max 2550 ms 
	time.sleep((c/1000) + 0.200)
	b1 = 0
	while b1 != 0x80:
		b1 = SPI(0x80)  # wait for beep to finish
#	print "beep " + str(b1)
#	time.sleep(0.100) # ???
def tick():
	SPI(0x92)  # tick the speaker
	b1 = 0
	time.sleep(0.020) # tick takes time

print "Raspi VFD test1"
print "shield " + str(getShield()) + ", digits " + str(getDigits())
print "volume " + str(getVol()) + ", bright " + str(getBrt())

beep(440,50)
time.sleep(0.025)
beep(880,50)
time.sleep(0.025)
beep(440,50)

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

menuNames = ['', 'alarm   ', 'bright  ', 'volume  ', 'end     ']
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
	setDots(0b00000000)
	setPos(0)
	SPIwrite(menuNames[menuState])
	if (menuState == len(menuNames)-1):
		menuEnd()

def showAlarm():
	setPos(0)
#	timestr = '  {:02}:{:02}  '.format(g_alarmTime)
	timestr = '{:>8}'.format(alarmTime)
	setDots(0b00000100)
	SPIwrite(timestr)
	
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
	s = '{:>8}'.format(bright)
	setPos(0)
	SPIwrite(s)
	setBrt(bright)
def actVol(update):
	volume = getVol()
	if (update): # not first time here?
		volume = 1 if (volume == 0) else 0
		setVol(volume)
	s = '     hi' if (volume == 1) else '     lo'
	setPos(0)
	SPIwrite(s)
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
	setPos(0)
	now = datetime.now()
	timestr = '{:%y-%m-%d}'.format(now)
	SPIwrite(timestr)
	# Toggle dots during date display
	if (now.second % 2):
		setDots(0b00000001)
	else:
		setDots(0b00000000)

def showTime(now):
	global lastShowTime
	setPos(0)
	if (now.second != lastShowTime.second): # run once a second
		lastShowTime = now
		if (now.second >= 57) and (now.second <= 59): # show date
			showDate()
		else:
			timestr = '{:  %I%M%S}'.format(now)
			SPIwrite(timestr)
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
	SPIwrite("  bye  ")
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
		setPos(0)
		SPIwrite("alrm off")
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
			then = now # now & then are one
			if (S2state):
				showDate()
			else:
				showTime(now)
			checkAlarm(now) # must come after date/time display (beep bug?)
	# Wait 100 ms
	time.sleep(0.1)
