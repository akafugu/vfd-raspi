#!/usr/bin/python

import RPi.GPIO as GPIO
import time
import datetime
import spidev
import signal

# ===========================================================================
# SPI Clock Example using hardware SPI with SPIDEV
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

SPI(0x82)  # clear
SPIwrite("01234567")
#setDots(0b11111111)
time.sleep(1)
for b in range(0,10):
	setBrt(b)
	time.sleep(1)
#setDots(0b00110011)
#time.sleep(1)
#setDots(0b01010101)
SPI(0x82)  # reset

#timeString  = time.strftime("%I%M")
#print timeString

# Globals
S1status = False
S2status = False
S3status = False
saveTime = datetime.datetime.now()

def chkButtons():
	global S1status, S2status, S3status
	st = not GPIO.input(S1)
	if (st != S1status):
		if (st):
			print "S1 on"
			setDash(1)
		else:
			print "S1 off"
			setDash(0)
			S1status = st
	st = not GPIO.input(S2)
	if (st != S2status):
		if (st):
			print "S2 on"
			setDot(1)
		else:
			print "S2 off"
			setDot(0)
		S2status = st
	st = GPIO.input(S3)
	if (st != S3status):
		if (st):
			print "S3 on"
			setDot(1)
		else:
			print "S3 off"
			setDot(0)
		S3status = st

def showTime():
	global saveTime
	setPos(0)
	now = datetime.datetime.now()
	if (now.second != saveTime.second):  # run once a second
		saveTime = now
		if (now.second >= 57) and (now.second <= 59):  # show date
			timestr = '{:%y-%m-%d}'.format(now)
			SPIwrite(timestr)
			# Toggle dots during date display
			if (now.second % 2):
				setDots(0b00000001)
			else:
				setDots(0b00000000)
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

setDots(0b00010100)

def sayBye():
	SPI(0x82)  # clear
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

# Continually update the time on the VFD display
while(True):
	chkButtons()
	showTime()
	# Wait 100 ms
	time.sleep(0.1)
