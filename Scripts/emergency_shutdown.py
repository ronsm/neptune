#
# server.js
#
# This is the Node application responsible for providing communication
# between the HTML-based GUI and the hardware of the Raspberry Pi, including
# the I2C and GPIO functionality.
#
# LICENSE: This source file is subject to a Creative Commons
# Attribution-NonCommercial 4.0 International (CC BY-NC 4.0) License.
# Full details of this license are available online at:
# https://creativecommons.org/licenses/by-nc/4.0/.
#
# @package    Neptune
# @author     Ronnie Smith <ronniesmith@outlook.com>
# @copyright  2017, Ronnie Smith
# @license    https://creativecommons.org/licenses/by-nc/4.0/ CC BY-NC 4.0
# @version    1.0
# @link       https://github.com/ronsm/neptune
#
# ATTRIBUTIONS: This project uses and derives open source code and packages from
# various authors, which are attributed here where possible.
#    1) This script based on "shutdown_pi.py" by IPV1
#       https://wwww.element14.com/community/docs/DOC-78055/
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
