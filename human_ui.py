import msvcrt
import hellousb
import time
import datetime
import sys
import csv
from time import sleep

hello = hellousb.hellousb()

KEY_W = chr(119) 		#define the keys to be used in the interface program
KEY_A = chr(97)
KEY_S = chr(115)
KEY_D = chr(100)
KEY_P = chr(112)

# MAX = 2**16 - 1 		#put limits on servo motion
MAX = 2**15 - 1 		#this is the range used for the test scan (~90 deg)
MIN = 0
FIDELITY = 20			#number of steps along each sweep, determines resolution of scan
INC_VAL = MAX/FIDELITY 	#set the value to increment servo position by
TILT_VAL = 30000 		#set initial values of PAN and TILT
PAN_VAL = 30000			#in this case, the transducers will start straight up

PING = 0 				#set variables to 0
SCAN = 0
convert = (1/526.4)		#conerts time of flight to distance based on calibration

f=open('range_data2.txt', 'w')		#opens the text files used to store data, deletes existing data

hello.set_vals(PAN_VAL, TILT_VAL)	#writes initial servo positions

#INTRO TEXT
print ('----------------\nReady to start! Use W,A,S,D keys to move, press P\nto see options for distance ranging.')
print ('----------------\nBrett Rowley & Katherine Stegner - Olin College, 2013\n----------------')

while True:
	if msvcrt.kbhit():
		key = msvcrt.getch()

		#MANUAL CONTROLS
		if key == KEY_W:
			TILT_VAL += INC_VAL
		elif key == KEY_S:
			TILT_VAL -= INC_VAL
		elif key == KEY_D:
			PAN_VAL -= INC_VAL
		elif key == KEY_A:
			PAN_VAL += INC_VAL

		#CONFIRMATION TO START AUTOMATED SWEEP
		if key == KEY_P:
			CONFIRM = raw_input("Automated Scan (1) or Single Ping (2)?")	
			#asks user to choose between single ping or auto. sweep
			#if auto, set pos to 0,0 and toggle SCAN																
			if CONFIRM == '1':																		
				PAN_VAL = 0
				TILT_VAL = 0
				hello.set_vals(PAN_VAL, TILT_VAL)		
				SCAN =not SCAN 						#toggle SCAN variable (see below)
				print 'ULTRASONIC MAPPING STARTED'
				print 'Hit Ctrl+C to abort'
			#if single, run PING once
			elif CONFIRM == '2':
				ping = hello.get_ping()				#call GET_PING once
				ping = float(ping[0])				#store the time of flight value as ping
				dist = (ping - 23313) * convert		#conert from time to distance
				print ping 							#print both values for operator to see
				print dist
			#if neither, print Cancelled
			elif CONFIRM != '1' or CONFIRM != '2':
				print 'Cancelled'
	
	#AUTOMATED SWEEP
	if SCAN == True:
		while (PAN_VAL < MAX):			#while number of pan steps is lower than desired number
			hello.set_vals(PAN_VAL, TILT_VAL)

			ping = hello.get_ping()		#calls ping, stores values in variables
			ping = float(ping[0])
			dist = (ping -23313) * convert
			print ping
			
			PAN_DEG = round((PAN_VAL*(0.0027466)), 2)	#converts servo positions to degrees, stores in variables
			TILT_DEG = round((TILT_VAL*(0.0027466)), 2)

			f=open('range_data.txt', 'a')	#opens text file in append mode to add data in new lines
			f.write(repr(PAN_DEG) + ', ' + repr(TILT_DEG) + ', ' + repr(dist) + '\n')	#write servo positions and measured distance to file

			sleep(0.5)	#wait for file to finish writing

			PAN_VAL += INC_VAL		#increment pan position

		else:						#once pan sweep complete, reset and increment tilt
			PAN_VAL = MIN
			TILT_VAL += INC_VAL
			if TILT_VAL > MAX:
				print ('Sweep Completed, consult range_plot.m for results')
				SCAN =not SCAN
				

	#BUFFER TO PREVENT OVERRUNING SERVOS
	if PAN_VAL > MAX:
		PAN_VAL = MAX
	if PAN_VAL < MIN:
		PAN_VAL = MIN
	if TILT_VAL > MAX:
		TILT_VAL = MAX
	if TILT_VAL < MIN:
		TILT_VAL = MIN

	hello.set_vals(PAN_VAL, TILT_VAL)
	# print hello.get_ping()


