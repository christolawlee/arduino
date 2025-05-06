from arduino_alvik import ArduinoAlvik
from time import sleep_ms

# Initialize the ArduinoAlvik object
alvik = ArduinoAlvik()
alvik.begin()
sleep_ms(5000)  # Wait for the robot to set up

# Parameters
safe_distance_outer = 15  # Minimum safe distance in cm
safe_distance_inner = 10
safe_distance_center = 5
speed = 100.00        # Movement speed
reverse_distance = -5.00
rotation_angle = 15  # Angle to rotate when avoiding obstacles
pause_time = 100
last_obstacle = 1

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
    print(f"Distances - Left: {distance_l} cm, Center Left: {distance_cl} cm, Center: {distance_c} cm, Center Right: {distance_cr} cm, Right: {distance_r} cm")

    # Obstacle avoidance logic
    if distance_c < safe_distance_center:
        print(f"Obstacle detected ahead at {distance_c} cm! Rotating...")
        alvik.brake()
        # alvik.move(reverse_distance)
        if last_obstacle == 1:
            alvik.rotate(rotation_angle, 'deg')  # Rotate 90 degrees to the right
        else:
            alvik.rotate(-rotation_angle, 'deg')  # Rotate 90 degrees to the right
        sleep_ms(pause_time)
    elif distance_cl < safe_distance_inner:
        last_obstacle = 1
        print(f"Obstacle detected on center-left at {distance_cl} cm! Rotating right...")
        alvik.brake()
        # alvik.move(reverse_distance)
        alvik.rotate(-rotation_angle, 'deg')
        sleep_ms(pause_time)
    elif distance_l < safe_distance_outer:
        last_obstacle = 1
        print(f"Obstacle detected on the left at {distance_l} cm! Rotating right...")
        alvik.brake()
        # alvik.move(reverse_distance)
        alvik.rotate(-rotation_angle, 'deg')
        sleep_ms(pause_time)
    elif distance_cr < safe_distance_inner:
        last_obstacle = 0
        print(f"Obstacle detected on center-right at {distance_cr} cm! Rotating left...")
        alvik.brake()
        # alvik.move(reverse_distance)
        alvik.rotate(rotation_angle, 'deg')  # Rotate 90 degrees to the left
        sleep_ms(pause_time)
    elif distance_r < safe_distance_outer:
        last_obstacle = 0
        print(f"Obstacle detected on the right at {distance_r} cm! Rotating left...")
        alvik.brake()
        # alvik.move(reverse_distance)
        alvik.rotate(rotation_angle, 'deg')
        sleep_ms(pause_time)
    else:
        print("Path is clear. Moving forward...")
        alvik.set_wheels_speed(speed, speed)  # Move forward if no obstacles are detected
