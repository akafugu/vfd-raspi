#!/usr/bin/python

import RPi.GPIO as GPIO
import sched, signal, time
from datetime import datetime
from vfdspi import *

# ===========================================================================
# SPI Clock piezo test using hardware SPI with SPIDEV
# William B Phelps - wm@usa.net
# created 20 February 2013
# updated 20 February 2013 - rewrite piezo/beep
# ===========================================================================

S1 = 22
S2 = 27
S3 = 17  # alarm on/off

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)

GPIO.setup(S1, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(S2, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(S3, GPIO.IN, pull_up_down=GPIO.PUD_UP)

print "Raspi VFD piezo test"
print "shield " + str(getShield()) + ", digits " + str(getDigits())
print "volume " + str(getVol()) + ", bright " + str(getBrt())

#clear()
#setvol(10)
# demo volume steps
#for v in range(0, 129, 11):
#	setVol(v)
#	beep(4000,100)
#	display(2,'{:04}'.format(v))
#	time.sleep(0.1)
#time.sleep(1)

#clear()
# test beep interaction with SPI
#for t in range(110,4400,110):
#	beep(t, 100)
#	display(4,str(t))
#	time.sleep(0.1)

#clear()

# values for each note in an octave
#notes = { 'Cn':0, 'Cs':1, 'Df':1, 'Dn':2, 'Ds':3, 'Ef':3, 'En':4, 'Fn':5, 
# 'Fs':6, 'Gf':6, 'Gn':7, 'Gs':8, 'Af':8, 'An':9, 'As':10, 'Bf':10, 'Bn':11 }
Cn=0; Cs=1; Df=1; Dn=2; Ds=3; Ef=3; En=4; Fn=5; Fs=6; Gf=6; Gn=7; Gs=8; Af=8; An=9; As=10; Bf=10; Bn=11
freqs = [ 130.81, 138.59, 146.83, 155.56, 164.81, 174.61, 185.00, 196.00, 207.65, 220.00, 233.08, 246.94]
tempo = 18
def N(note, octave, timing):
	global tempo
#	n = notes[note]
	n = note
	f = int(freqs[n]*(2**(octave-5)))
	t = timing*tempo
	print note, octave, timing, n, f, t
#	display(0,'{:>8}'.format(f))
	beep(f, t)
	time.sleep(4*tempo/1000.0)
def PAUSE(timing):
	global tempo
	t = timing*tempo
#	print "pause", t
#	display(0, "   pause")
	time.sleep(t/1000.0)

def reveille():
	N(Gn,6,8), N(Cn,7,8), N(En,7,4), N(Cn,7,4); N(Gn,6,8),
	N(En,7,8), N(Cn,7,8), N(En,7,4), N(Cn,7,4); N(Gn,6,8),
	N(En,7,8), N(Cn,7,8), N(En,7,4), N(Cn,7,4); N(Gn,6,8), N(Cn,7,8), N(En,7,16), N(Cn,7,8),
	PAUSE(8)
	N(Gn,6,8), N(Cn,7,8), N(En,7,4), N(Cn,7,4); N(Gn,6,8),
	N(En,7,8), N(Cn,7,8), N(En,7,4), N(Cn,7,4); N(Gn,6,8),
	N(En,7,8), N(Cn,7,8), N(En,7,4), N(Cn,7,4); N(Gn,6,8), N(Gn,6,8), N(Cn,7,24),
	PAUSE(16)
	N(En,7,8), N(En,7,8), N(En,7,8), N(En,7,8); N(En,7,8), N(Gn,7,16), N(En,7,8),
	PAUSE(4)
	N(Cn,7,8), N(En,7,8), N(Cn,7,8), N(En,7,8); N(Cn,7,8), N(En,7,16), N(Cn,7,8),
	PAUSE(4)
	N(En,7,8), N(En,7,8), N(En,7,8), N(En,7,8); N(En,7,8), N(Gn,7,16), N(En,7,8),
	PAUSE(4)
	N(Cn,7,8), N(En,7,8), N(Cn,7,8), N(Gn,6,8); N(Gn,6,8), N(Cn,7,24),
	PAUSE(8)

def BigBen():
	N(Bn,5,32), N(Gn,5,32), N(An,5,32), N(Dn,5,32), N(Gn,5,32),
	N(An,5,32), N(Bn,5,32), N(Gn,5,32), N(Bn,5,32), N(An,5,32),
	N(Gn,5,32), N(Dn,5,32), N(Dn,5,32), N(An,5,32), N(Bn,5,32), N(Gn,5,32),
	PAUSE(32), N(Gn,5,64),
	PAUSE(32), N(Gn,5,64),
	PAUSE(32), N(Gn,5,64),
	PAUSE(32), N(Gn,5,64)

def xmas():
	N(Dn,6,16), N(Gn,6,16), N(Gn,6,8), N(An,6,8), N(Gn,6,8), N(Fs,6,8),	N(En,6,16), N(En,6,16),
	N(En,6,16),	N(An,6,16), N(An,6,8),  N(Bn,6,8), N(An,6,8), N(Gn,6,8), N(Fs,6,16), N(Dn,6,16),
	N(Dn,6,16), N(Bn,6,16), N(Bn,6,8),  N(Cn,7,8), N(Bn,6,8), N(An,6,8), N(Gn,6,16), N(En,6,16),
	N(En,6,8), N(En,6,8); N(En,6,16), N(An,6,16), N(Fs,6,16), N(Gn,6,32),
	PAUSE(16),
	N(Dn,6,16),	N(Gn,6,16), N(Gn,6,16), N(Gn,6,16),	N(Fs,6,32),
	N(Fs,6,16),	N(Gn,6,16), N(Fs,6,16), N(En,6,16),	N(Dn,6,32),
	N(Bn,6,16),	N(Cn,7,16), N(Bn,6,16), N(An,6,16),	N(Dn,7,16), N(Dn,6,16), N(Dn,6,8),
	N(Dn,6,8), N(En,6,16), N(An,6,16), N(Fs,6,16), N(Gn,6,32)

#E,E,G,A,E,E,D,D#,E,E,G,A,E,E,D,D#, C,A,E,C,A,D#,C,A,D,Gb,G.
def missimp():
	N(En,7,16), N(En,7,16), N(Gn,7,8), N(An,7,8), N(En,7,16), N(En,7,16), N(Dn,7,8), N(Ds,7,8), 
	N(En,7,16), N(En,7,16), N(Gn,7,8), N(An,7,8), N(En,7,16), N(En,7,16), N(Dn,7,8), N(Ds,7,8), 
	PAUSE(16)
	N(Cn,7,8), N(An,7,8), N(En,7,8), PAUSE(4), 
	N(Cn,6,8), N(An,7,8), N(Ds,7,8), PAUSE(4),
	N(Cn,7,8), N(An,7,8), N(Dn,7,8), PAUSE(4),
	N(Gf,7,32), N(Gn,7,32), 
	PAUSE(16)
	

setVol(128) # loud
#BigBen()
#xmas()
#reveille()

missimp()