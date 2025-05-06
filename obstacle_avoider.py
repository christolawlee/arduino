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

while True:
    # Small delay to prevent excessive polling
    sleep_ms(10)

    # Get distance readings from ToF sensors
    distances = alvik.get_distance()
    if len(distances) != 5:
        raise ValueError(f"Expected 5 distance values from get_distance(), but got: {len(distances)}")
    
    # Unpack distances
    distance_l, distance_cl, distance_c, distance_cr, distance_r = distances

    # Print distance readings for debugging
    print(f"Distances - Left: {distance_l} mm, Center Left: {distance_cl} mm, Center: {distance_c} mm, Center Right: {distance_cr} mm, Right: {distance_r} mm")

    # Obstacle avoidance logic
    if distance_c < safe_distance:
        print(f"Obstacle detected ahead at {distance_c} mm! Rotating...")
        alvik.stop()
        alvik.rotate(rotation_angle, 'deg')  # Rotate 90 degrees to the right
        sleep_ms(1000)
    elif distance_cl < safe_distance:
        print(f"Obstacle detected on center-left at {distance_cl} mm! Rotating right...")
        alvik.stop()
        alvik.rotate(rotation_angle, 'deg')
        sleep_ms(1000)
    elif distance_cr < safe_distance:
        print(f"Obstacle detected on center-right at {distance_cr} mm! Rotating left...")
        alvik.stop()
        alvik.rotate(-rotation_angle, 'deg')  # Rotate 90 degrees to the left
        sleep_ms(1000)
    elif distance_l < safe_distance:
        print(f"Obstacle detected on the left at {distance_l} mm! Rotating right...")
        alvik.stop()
        alvik.rotate(rotation_angle, 'deg')
        sleep_ms(1000)
    elif distance_r < safe_distance:
        print(f"Obstacle detected on the right at {distance_r} mm! Rotating left...")
        alvik.stop()
        alvik.rotate(-rotation_angle, 'deg')
        sleep_ms(1000)
    else:
        print("Path is clear. Moving forward...")
        alvik.move_forward(speed)  # Move forward if no obstacles are detected
