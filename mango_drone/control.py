import time
import os
import platform
import sys

from dronekit import connect, VehicleMode,LocationGlobal,LocationGlobalRelative
from pymavlink import mavutil
#############################

targetAltitude=1
manualArm=False
############DRONEKIT#################


#########FUNCTIONS###########

class drone_controller:
    def __init__(self, connection_string):
        self.vehicle = connect(connection_string, wait_ready=True)

    def takeoff(self, altitude):
        self.vehicle.mode = VehicleMode("GUIDED")
        
        self.vehicle.armed = True

        # Confirm vehicle armed before attempting to take off
        while not self.vehicle.armed:
            print(" Waiting for arming...")
            time.sleep(1)

        self.vehicle.simple_takeoff(altitude)

        while True:
            if self.vehicle.location.global_relative_frame.alt >= altitude * 0.95:
                print("Reached target altitude")
                break
            time.sleep(1)

    def land(self):
        self.vehicle.mode = VehicleMode("LAND")
        
        while True:
            if self.vehicle.location.global_relative_frame.alt <= 0.1:
                break
            time.sleep(1)
        
        self.vehicle.close()
   
    def send_body_ned_velocity(self, velocity_x, velocity_y, velocity_z, duration):
        msg = self.vehicle.message_factory.set_position_target_local_ned_encode(
                0,       # time_boot_ms (not used)
                0, 0,    # target system, target component
                mavutil.mavlink.MAV_FRAME_BODY_NED, # frame Needs to be MAV_FRAME_BODY_NED for forward/back left/right control.
                0b0000111111000111, # type_mask
                0, 0, 0, # x, y, z positions (not used)
                velocity_x, velocity_y, velocity_z, # m/s
                0, 0, 0, # x, y, z acceleration
                0, 0)
        for x in range(0,duration):
            self.vehicle.send_mavlink(msg)
            time.sleep(1)
   
    def move_forward(self):
        velocity_x = 0
        velocity_y = 1
        velocity_z = 0
        duration = 2
        self.send_body_ned_velocity(velocity_x, velocity_y, velocity_z, duration)
        
    def move_backward(self):
        velocity_x = 0
        velocity_y = -1
        velocity_z = 0
        duration = 2
        self.send_body_ned_velocity(velocity_x, velocity_y, velocity_z, duration)

    def move_left(self):
        velocity_x = -1
        velocity_y = 0
        velocity_z = 0
        duration = 1
        self.send_body_ned_velocity(velocity_x, velocity_y, velocity_z, duration)

    def move_right(self):
        velocity_x = 1
        velocity_y = 0
        velocity_z = 0
        duration = 1
        self.send_body_ned_velocity(velocity_x, velocity_y, velocity_z, duration)
    
    def move_up(self):
        velocity_x = 0
        velocity_y = 0
        velocity_z = -1
        duration = 1
        self.send_body_ned_velocity(velocity_x, velocity_y, velocity_z, duration)


    def move_down(self):
        velocity_x = 0
        velocity_y = 0
        velocity_z = 1
        duration = 1
        self.send_body_ned_velocity(velocity_x, velocity_y, velocity_z, duration)

'''
    def move_left(self, distance):
        current_location = self.vehicle.location.global_frame
        target_location = current_location.offset_distance(distance, -90, 0)

        self.move_to_location(target_location)

    def move_right(self, distance):
        current_location = self.vehicle.location.global_frame
        target_location = current_location.offset_distance(distance, 90, 0)

        self.move_to_location(target_location)

    def move_forward(self, distance):
        current_location = self.vehicle.location.global_frame
        target_location = current_location.offset_distance(distance, 0, 0)

        self.move_to_location(target_location)

    def move_backward(self, distance):
        current_location = self.vehicle.location.global_frame
        target_location = current_location.offset_distance(distance, 180, 0)

        self.move_to_location(target_location)

    def move_up(self, altitude):
        current_altitude = self.vehicle.location.global_relative_frame.alt
        target_altitude = current_altitude + altitude

        self.move_to_altitude(target_altitude)

    def move_down(self, altitude):
        current_altitude = self.vehicle.location.global_relative_frame.alt
        target_altitude = current_altitude - altitude

        self.move_to_altitude(target_altitude)

    def move_to_location(self, target_location):
        msg = self.vehicle.message_factory.set_position_target_local_ned_encode(
            0, 0, 0,
            mavutil.mavlink.MAV_FRAME_LOCAL_NED,
            0b0000111111111000,
            target_location.lat, target_location.lon, target_location.alt,
            0, 0, 0,
            0, 0, 0,
            0, 0
        )

        self.vehicle.send_mavlink(msg)
        self.vehicle.flush()

        while self.vehicle.location.global_frame.distance(target_location) > 0.5:
            time.sleep(1)
    def move_to_altitude(self, target_altitude):
        self.vehicle.simple_goto(
            location=self.vehicle.location.global_frame,
            altitude=target_altitude
        )

        while abs(self.vehicle.location.global_relative_frame.alt - target_altitude) > 0.5:
            time.sleep(1)


    def close_connection(self):
        self.vehicle.close()
'''
############MAIN###############
if __name__ == '__main__':
    connection_string = '/dev/ttyACM0'
    control = drone_controller(connection_string)
    
    control.takeoff(2)
    print("takeoff act!!")
    '''
    control.move_forward()
    print("forward act!!")
    time.sleep(1)
    
    control.move_left()
    print("left act!!")
    time.sleep(1)
    '''
    control.move_right(2)
    print("right act!!")
    time.sleep(1)
    '''
    control.move_backward()
    print("back act!!")
    time.sleep(1)
    
    control.move_up()
    print("up act!!")
    time.sleep(1)
    control.move_down(2)
    print("down act!!")
    time.sleep(1)
    '''
    control.land()
    print("land act!!")
