#!/usr/bin/python
import time
import spidev

#CE0  = 8
#MISO = 9
#MOSI = 10
#SCLK = 11

spi = spidev.SpiDev()
spi.open(0,0)
spi.mode = 0
#spi.max_speed_hz = 200000 # 200khz
_IV18seg0 = 0 # iv-18 segment 0 bits

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
	global _IV18seg0
	if (b>0):
		_IV18seg0 = 0b10000000 | _IV18seg0
	else:
		_IV18seg0 = 0b01111111 & _IV18seg0
	SPI(0x86)  # dot flag
	SPI(_IV18seg0)
def setIV18Dash(b):
	global _IV18seg0
	if (b>0):
		_IV18seg0 = 0b01000000 | _IV18seg0
	else:
		_IV18seg0 = 0b10111111 & _IV18seg0
	SPI(0x86)  # dash flag
	SPI(_IV18seg0)
def getVol():
	SPI(0x90)  # volume
	b = SPI(255)  # get current value
	return(b)
def setVol(b):
	SPI(0x90)  # volume
	SPI(b)  # set new value
def beep(f, t, w=True):
	SPI(0x91) # beep
	SPI(f/10) # max 2550 hz
	SPI(t/10) # max 2550 ms 
	if (w):
		time.sleep(t/1000.0) # wait for beep
def tick():
	SPI(0x93)  # tick the speaker
	b1 = 0
	time.sleep(0.020) # tick takes time

def display(pos, str):
	setPos(pos)
	SPIwrite(str)
