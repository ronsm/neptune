#
# emergency_shutdown.py
#
# This script initiates a shutdown of the Raspberry Pi when a logical low signal
# is detected on GPIO pin #18 (BCM numbering). The signal should be provided by
# some form of 'low battery' detector so that the Pi can shut down safely before
# power runs out.
#
# @package    Neptune
# @author     Ronnie Smith <ronniesmith@outlook.com>
# @version    1.1
# @link       https://github.com/ronsm/neptune
#

#!/bin/python

import RPi.GPIO as GPIO
import time
import os

GPIO.setmode(GPIO.BCM)
GPIO.setup(18, GPIO.IN, pull_up_down = GPIO.PUD_UP)

def Shutdown(channel):
    os.system("sudo shutdown -h now")

GPIO.add_event_detect(18, GPIO.FALLING, callback = Shutdown, bouncetime = 2000)

while 1:
    time.sleep(1)
