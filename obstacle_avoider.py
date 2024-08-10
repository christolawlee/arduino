from arduino_alvik import ArduinoAlvik
from time import sleep_ms
import sys

alvik = ArduinoAlvik()
alvik.begin()
sleep_ms(5000)  # waiting for the robot to setup
distance = 5
degrees = 5.00
speed = 13.00

while (True):

    distance_l, distance_cl, distance_c, distance_r, distance_cr  = alvik.get_distance()
    sleep_ms(1)
    print(distance_c)

    if distance_c < distance:
        alvik.rotate(degrees, 'deg')
    elif distance_cl < distance:
        alvik.rotate(degrees, 'deg')
    elif distance_cr < distance:
        alvik.rotate(degrees, 'deg')
    elif distance_l < distance:
        alvik.rotate(degrees, 'deg')
    elif distance_r < distance:
        alvik.rotate(degrees, 'deg')
    else:
        alvik.drive(speed, 0.0, linear_unit='cm/s')
