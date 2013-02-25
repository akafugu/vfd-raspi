# alarmTime class 

# ===========================================================================
# SPI Clock Example using hardware SPI with SPIDEV - https://github.com/doceme/py-spidev
# William B Phelps - wm@usa.net
# created 20 February 2013
# updated 24 February 2013 - rewrite piezo/beep
# ===========================================================================

# simplified time class for alarm time, with
#  time in seconds, hour, minute

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
