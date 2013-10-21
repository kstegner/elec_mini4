import msvcrt
import hellousb
import time
import datetime
import sys
import csv
from time import sleep

hello = hellousb.hellousb()

KEY_W = chr(119)
KEY_S = chr(115)
KEY_E = chr(101)
KEY_D = chr(100)

FREQ_INC = 10
DUTY_INC = 1000
VAL1 = 0 				#set variables to 0
VAL2 = 0

hello.set_vals(VAL1, VAL2)	#writes initial servo positions

#INTRO TEXT	
print ('----------------\nBrett Rowley & Katherine Stegner - Olin College, 2013\n----------------')

while True:
	if msvcrt.kbhit():
		key = msvcrt.getch()

		if key == KEY_W:
			VAL1 += FREQ_INC
			print VAL1
		elif key == KEY_S:
			VAL1 -= FREQ_INC
			print VAL1
		elif key == KEY_E:
			VAL2 += DUTY_INC
			print VAL2
		elif key == KEY_D:
			VAL2 -= DUTY_INC
			print VAL2
	hello.set_vals(VAL1, VAL2)


