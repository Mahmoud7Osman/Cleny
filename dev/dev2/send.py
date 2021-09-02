#!/usr/bin/env python
import threading
import string
import base64
import urllib.request
import urllib.parse
import os
import time
import sys
import random

try:
    import requests
except ImportError:
    print('Error !! : Some dependencies are not installed')
    print('Error Occured!!!\nUse Non-Supported Version')
    input('Press Any Key To Use Non-supported Version')
    os.system('bash send.sh --sendsms')

# colors
yellow='\033[93m'
gren='\033[92m'
cyan='\033[96m'
pink='\033[95m'
red='\033[91m'
b='\033[1m'
W = '\033[0m'
colors = ['\033[1;31m', '\033[1;32m', '\033[1;33m', '\033[1;34m', '\033[1;35m', '\033[1;36m']

def clr():
	if os.name == 'nt':
		os.system('cls')
	else:
		os.system('clear')

print ("Welcome To Digle Anonymous SMS Sender Device")
time.sleep(1)
print ("This Digle Device Is Used to send Anonymous SMS")
time.sleep(1)
def Track() :
   exit()
def update():
   exit()
while True:
		cc = input("\tEnter Country Code (Without +) : ")
		if '+' in cc:
		        tc = list(cc)
		        tc.remove('+')
		        cc = ''.join(tc)
		        cc = cc.strip()
		if len(cc) >= 4 or len(cc) < 1:
		        print('\n\nInvalid Country Code..\n\t\tCountry Codes Are Generally 1-3 digits...\n')
		        continue
		pn = input("Enter Phone Number : +" + cc + " ")
		if len(pn) <= 6:
		        print('\n\nInvalid Phone Number..\n')
		        continue
		numbe = cc + pn
		if not numbe.isdigit():
		            print('\n\nPhone Number Must Consist Of Numbers Only\n')
		            continue
		receiver = '+' + numbe
		text = input("Enter Message to send : ")
		
		resp = requests.post('https://textbelt.com/text',{
			'phone' : receiver,
			'message' : text ,
			'key' : 'textbelt'
		})
		
		print(resp.json())
		exit() 
