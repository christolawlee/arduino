from arduino_alvik import ArduinoAlvik
from time import sleep_ms

# Initialize the ArduinoAlvik object
alvik = ArduinoAlvik()
alvik.begin()
sleep_ms(5000)  # Wait for the robot to set up

# Parameters
safe_distance = 200  # Minimum safe distance in mm
speed = 20.00        # Movement speed
rotation_angle = 90  # Angle to rotate when avoiding obstacles

# Sensor IDs (based on the API documentation)
SENSOR_LEFT = 0
SENSOR_CENTER_LEFT = 1
SENSOR_CENTER = 2
SENSOR_CENTER_RIGHT = 3
SENSOR_RIGHT = 4

while True:
    # Small delay to prevent excessive polling
    sleep_ms(10)

    # Get distance readings from ToF sensors
    distance_l = alvik.get_distance(SENSOR_LEFT)
    distance_cl = alvik.get_distance(SENSOR_CENTER_LEFT)
    distance_c = alvik.get_distance(SENSOR_CENTER)
    distance_cr = alvik.get_distance(SENSOR_CENTER_RIGHT)
    distance_r = alvik.get_distance(SENSOR_RIGHT)

    # Print distance readings for debugging
    print(f"Distances - Left: {distance_l} mm, Center Left: {distance_cl} mm, Center: {distance_c} mm, Center Right: {distance_cr} mm, Right: {distance_r} mm")

    # Obstacle avoidance logic
    if distance_c < safe_distance:
        print(f"Obstacle detected ahead at {distance_c} mm! Rotating...")
        alvik.stop()  # Stop the robot
        alvik.rotate(rotation_angle, 'deg')  # Rotate 90 degrees to the right
        sleep_ms(1000)  # Wait for the rotation to complete
    elif distance_cl < safe_distance:
        print(f"Obstacle detected on center-left at {distance_cl} mm! Rotating right...")
        alvik.stop()
        alvik.rotate(rotation_angle, 'deg')  # Rotate 90 degrees to the right
        sleep_ms(1000)
    elif distance_cr < safe_distance:
        print(f"Obstacle detected on center-right at {distance_cr} mm! Rotating left...")
        alvik.stop()
        alvik.rotate(-rotation_angle, 'deg')  # Rotate 90 degrees to the left
        sleep_ms(1000)
    elif distance_l < safe_distance:
        print(f"Obstacle detected on the left at {distance_l} mm! Rotating right...")
        alvik.stop()
        alvik.rotate(rotation_angle, 'deg')  # Rotate 90 degrees to the right
        sleep_ms(1000)
    elif distance_r < safe_distance:
        print(f"Obstacle detected on the right at {distance_r} mm! Rotating left...")
        alvik.stop()
        alvik.rotate(-rotation_angle, 'deg')  # Rotate 90 degrees to the left
        sleep_ms(1000)
    else:
        print("Path is clear. Moving forward...")
        alvik.move_forward(speed)  # Move forward if no obstacles are detected
