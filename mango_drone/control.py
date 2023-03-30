import time
import os
import platform
import sys

from dronekit import connect, VehicleMode,LocationGlobal,LocationGlobalRelative
from pymavlink import mavutil

############DRONEKIT#################
#x for(+)_back(-)
#y let(-)_right(+)
#z up(-)_down(+)
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
    
    def move_right(self):
        velocity_x = 0
        velocity_y = 0.25
        velocity_z = 0
        duration = 2
        
        msg = self.vehicle.message_factory.set_position_target_local_ned_encode(
                0,       # time_boot_ms (not used)
                0, 0,    # target system, target component
                #mavutil.mavlink.MAV_FRAME_BODY_NED, # frame Needs to be MAV_FRAME_BODY_NED for forward/back left/right control.
                mavutil.mavlink.MAV_FRAME_BODY_OFFSET_NED, # frame Needs to be MAV_FRAME_BODY_NED for forward/back left/right control.
                0b0000111111000111, # type_mask
                0, 0, 0, # x, y, z positions (not used)
                velocity_x, velocity_y, velocity_z, # m/s
                0, 0, 0, # x, y, z acceleration
                0, 0)
        for x in range(0,duration):
            self.vehicle.send_mavlink(msg)
            time.sleep(1)
        self.vehicle.flush()
        
    def move_left(self):
        velocity_x = 0
        velocity_y = -0.25
        velocity_z = 0
        duration = 2
        
        msg = self.vehicle.message_factory.set_position_target_local_ned_encode(
                0,       # time_boot_ms (not used)
                0, 0,    # target system, target component
                #mavutil.mavlink.MAV_FRAME_BODY_NED, # frame Needs to be MAV_FRAME_BODY_NED for forward/back left/right control.
                mavutil.mavlink.MAV_FRAME_BODY_OFFSET_NED, # frame Needs to be MAV_FRAME_BODY_NED for forward/back left/right control.
                0b0000111111000111, # type_mask
                0, 0, 0, # x, y, z positions (not used)
                velocity_x, velocity_y, velocity_z, # m/s
                0, 0, 0, # x, y, z acceleration
                0, 0)
        for x in range(0,duration):
            self.vehicle.send_mavlink(msg)
            time.sleep(1)
        self.vehicle.flush()


    def move_backward(self):
        velocity_x = -0.25
        velocity_y = 0
        velocity_z = 0
        duration = 2
        
        msg = self.vehicle.message_factory.set_position_target_local_ned_encode(
                0,       # time_boot_ms (not used)
                0, 0,    # target system, target component
                #mavutil.mavlink.MAV_FRAME_BODY_NED, # frame Needs to be MAV_FRAME_BODY_NED for forward/back left/right control.
                mavutil.mavlink.MAV_FRAME_BODY_OFFSET_NED, # frame Needs to be MAV_FRAME_BODY_NED for forward/back left/right control.
                0b0000111111000111, # type_mask
                0, 0, 0, # x, y, z positions (not used)
                velocity_x, velocity_y, velocity_z, # m/s
                0, 0, 0, # x, y, z acceleration
                0, 0)
        for x in range(0,duration):
            self.vehicle.send_mavlink(msg)
            time.sleep(1)
        self.vehicle.flush()

    def move_forward(self):
        velocity_x = 0.25
        velocity_y = 0
        velocity_z = 0
        duration = 2
        
        msg = self.vehicle.message_factory.set_position_target_local_ned_encode(
                0,       # time_boot_ms (not used)
                0, 0,    # target system, target component
                #mavutil.mavlink.MAV_FRAME_BODY_NED, # frame Needs to be MAV_FRAME_BODY_NED for forward/back left/right control.
                mavutil.mavlink.MAV_FRAME_BODY_OFFSET_NED, # frame Needs to be MAV_FRAME_BODY_NED for forward/back left/right control.
                0b0000111111000111, # type_mask
                0, 0, 0, # x, y, z positions (not used)
                velocity_x, velocity_y, velocity_z, # m/s
                0, 0, 0, # x, y, z acceleration
                0, 0)
        for x in range(0,duration):
            self.vehicle.send_mavlink(msg)
            time.sleep(1)
        self.vehicle.flush()
    
    def move_up(self):
        velocity_x = 0
        velocity_y = 0
        velocity_z = -0.25
        duration = 2
        
        msg = self.vehicle.message_factory.set_position_target_local_ned_encode(
                0,       # time_boot_ms (not used)
                0, 0,    # target system, target component
                #mavutil.mavlink.MAV_FRAME_BODY_NED, # frame Needs to be MAV_FRAME_BODY_NED for forward/back left/right control.
                mavutil.mavlink.MAV_FRAME_BODY_OFFSET_NED, # frame Needs to be MAV_FRAME_BODY_NED for forward/back left/right control.
                0b0000111111000111, # type_mask
                0, 0, 0, # x, y, z positions (not used)
                velocity_x, velocity_y, velocity_z, # m/s
                0, 0, 0, # x, y, z acceleration
                0, 0)
        for x in range(0,duration):
            self.vehicle.send_mavlink(msg)
            time.sleep(1)
        self.vehicle.flush()


    def move_down(self):
        velocity_x = 0
        velocity_y = 0
        velocity_z = 0.25
        duration = 2
        msg = self.vehicle.message_factory.set_position_target_local_ned_encode(
                0,       # time_boot_ms (not used)
                0, 0,    # target system, target component
                #mavutil.mavlink.MAV_FRAME_BODY_NED, # frame Needs to be MAV_FRAME_BODY_NED for forward/back left/right control.
                mavutil.mavlink.MAV_FRAME_BODY_OFFSET_NED, # frame Needs to be MAV_FRAME_BODY_NED for forward/back left/right control.
                0b0000111111000111, # type_mask
                0, 0, 0, # x, y, z positions (not used)
                velocity_x, velocity_y, velocity_z, # m/s
                0, 0, 0, # x, y, z acceleration
                0, 0)
        for x in range(0,duration):
            self.vehicle.send_mavlink(msg)
            time.sleep(1)
        self.vehicle.flush()


'''
    def close_connection(self):
        self.vehicle.close()
'''
############MAIN###############
if __name__ == '__main__':
    connection_string = '/dev/ttyACM0'
    control = drone_controller(connection_string)
    
    control.takeoff(2)
    print("takeoff act!!")
    
    control.move_forward()
    print("forward act!!")
    time.sleep(1)
    
    control.move_left()
    print("left act!!")
    time.sleep(1)
    
    control.move_right()
    print("right act!!")
    time.sleep(1)
    
    control.move_backward()
    print("back act!!")
    time.sleep(1)
    
    control.move_up()
    print("up act!!")
    time.sleep(1)
    control.move_down()
    print("down act!!")
    time.sleep(1)
    
    control.land()
    print("land act!!")
