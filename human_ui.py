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

FREQ_INC = 1
DUTY_INC = 1000
VAL1 = 0 				#set variables to 0
VAL2 = 0

#INTRO TEXT	
print ('----------------\nBrett Rowley & Katherine Stegner - Olin College, 2013\n----------------')

while True:
	if msvcrt.kbhit():
		key = msvcrt.getch()

		if key == KEY_W:
			VAL1 += DUTY_INC
		elif key == KEY_S:
			VAL1 -= DUTY_INC

	if VAL1 <= 0:
		VAL1 = 0
	elif VAL1 >= 65536:
		VAL1 = 65500

	hello.set_vals(VAL1, VAL2)


