from arduino_alvik import ArduinoAlvik
from time import sleep_ms
import sys

alvik = ArduinoAlvik()
alvik.begin()
sleep_ms(5000)  # waiting for the robot to setup
distance = 700
degrees = 45.00
speed = 5.00

def run():
    while (True):
    
        # distance_l, distance_cl, distance_c, distance_r, distance_cr  = alvik.get_distance()
        # sleep_ms(1)
        # print(distance_c)
    
        # if distance_c < distance:
        #     alvik.rotate(degrees, 'deg')
        # elif distance_cl < distance:
        #     alvik.rotate(degrees, 'deg')
        # elif distance_cr < distance:
        #     alvik.rotate(degrees, 'deg')
        # elif distance_l < distance:
        #     alvik.rotate(degrees, 'deg')
        # elif distance_r < distance:
        #     alvik.rotate(degrees, 'deg')
        # else:
        #     alvik.drive(speed, 0.0, linear_unit='cm/s')
        
        # Rotate the robot 90 degrees to the right
        alvik.rotate(90, 'deg')
        sleep_ms(1000)  # Wait for 1 second
        # Rotate the robot 90 degrees to the left
        alvik.rotate(-90, 'deg')
        sleep_ms(1000)  # Wait for 1 second
