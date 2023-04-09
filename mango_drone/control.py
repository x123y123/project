import time
import os
import platform
import sys

from dronekit import connect, VehicleMode,LocationGlobal,LocationGlobalRelative
from pymavlink import mavutil


######### performance monitor  ##########
from performance_model import performance_monitor
######### DRONEKIT ############
#x for(+)_back(-)
#y let(-)_right(+)
#z up(-)_down(+)
######### FUNCTIONS ###########

class drone_controller:
#This class can be used easily for drone controlling.
    @performance_monitor
    def __init__(self, connection_string: str) -> None:
        """
        Function: __init__()
        --------------------
        Initialize and connect to pixhawk
        
        connection_string: port id on host device

        return: None
        """
        self.vehicle = connect(connection_string, wait_ready=True)

    @performance_monitor
    def takeoff(self, altitude: float) -> None:
        """
        Function: takeoff()
        --------------------
        Let drone takeoff and change to mode to GUIDED mode
        
        altitude: the target height(meters) you want to takeoff

        return: None
        """
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

    @performance_monitor
    def land(self) -> None:
        """
        Function: land()
        --------------------
        Let drone land by changing the mode to LAND mode,
        and check the altitude at the same time.

        return: None
        """
        self.vehicle.mode = VehicleMode("LAND")
        
        while True:
            if self.vehicle.location.global_relative_frame.alt <= 0.1:
                break
            time.sleep(1)
        
        self.vehicle.close()
    
    @performance_monitor
    def move_right(self) -> None:
        """
        Function: move_right()
        --------------------
        Let drone move right by sending mavlink message.
        
        return: None
        """
        velocity_x = 0
        velocity_y = 0.25
        velocity_z = 0
        
        msg = self.vehicle.message_factory.set_position_target_local_ned_encode(
                0,       # time_boot_ms (not used)
                0, 0,    # target system, target component
                mavutil.mavlink.MAV_FRAME_BODY_OFFSET_NED, # frame Needs to be MAV_FRAME_BODY_NED for forward/back left/right control.
                0b0000111111000111, # type_mask
                0, 0, 0, # x, y, z positions (not used)
                velocity_x, velocity_y, velocity_z, # m/s
                0, 0, 0, # x, y, z acceleration
                0, 0)
        start_time = time.time()
        self.vehicle.send_mavlink(msg)
        self.vehicle.flush()
        end_time = time.time()
        latency = end_time - start_time
        print(f"move_right latency: {latency}")
        
    @performance_monitor
    def move_left(self) -> None:
        """
        Function: move_left()
        --------------------
        Let drone move left by sending mavlink message.
        
        return: None
        """
        
        velocity_x = 0
        velocity_y = -0.25
        velocity_z = 0
        
        
        msg = self.vehicle.message_factory.set_position_target_local_ned_encode(
                0,       # time_boot_ms (not used)
                0, 0,    # target system, target component
                mavutil.mavlink.MAV_FRAME_BODY_OFFSET_NED, # frame Needs to be MAV_FRAME_BODY_NED for forward/back left/right control.
                0b0000111111000111, # type_mask
                0, 0, 0, # x, y, z positions (not used)
                velocity_x, velocity_y, velocity_z, # m/s
                0, 0, 0, # x, y, z acceleration
                0, 0)
        start_time = time.time()
        self.vehicle.send_mavlink(msg)
        self.vehicle.flush()
        end_time = time.time()
        latency = end_time - start_time
        print(f"move_left latency: {latency}")


    @performance_monitor
    def move_backward(self) -> None:
        """
        Function: move_backward()
        --------------------
        Let drone move backward by sending mavlink message.
        
        return: None
        """
        velocity_x = -0.25
        velocity_y = 0
        velocity_z = 0
        
        msg = self.vehicle.message_factory.set_position_target_local_ned_encode(
                0,       # time_boot_ms (not used)
                0, 0,    # target system, target component
                mavutil.mavlink.MAV_FRAME_BODY_OFFSET_NED, # frame Needs to be MAV_FRAME_BODY_NED for forward/back left/right control.
                0b0000111111000111, # type_mask
                0, 0, 0, # x, y, z positions (not used)
                velocity_x, velocity_y, velocity_z, # m/s
                0, 0, 0, # x, y, z acceleration
                0, 0)
        start_time = time.time()
        self.vehicle.send_mavlink(msg)
        self.vehicle.flush()
        end_time = time.time()
        latency = end_time - start_time
        print(f"move_backward latency: {latency}")

    @performance_monitor
    def move_forward(self) -> None:
        """
        Function: move_forward()
        --------------------
        Let drone move forward by sending mavlink message.
        
        return: None
        """
        velocity_x = 0.25
        velocity_y = 0
        velocity_z = 0
        
        msg = self.vehicle.message_factory.set_position_target_local_ned_encode(
                0,       # time_boot_ms (not used)
                0, 0,    # target system, target component
                mavutil.mavlink.MAV_FRAME_BODY_OFFSET_NED, # frame Needs to be MAV_FRAME_BODY_NED for forward/back left/right control.
                0b0000111111000111, # type_mask
                0, 0, 0, # x, y, z positions (not used)
                velocity_x, velocity_y, velocity_z, # m/s
                0, 0, 0, # x, y, z acceleration
                0, 0)
        start_time = time.time()
        self.vehicle.send_mavlink(msg)
        self.vehicle.flush()
        end_time = time.time()
        latency = end_time - start_time
        print(f"move_forward latency: {latency}")
    
    @performance_monitor
    def move_up(self) -> None:
        """
        Function: move_up()
        --------------------
        Let drone move up by sending mavlink message.
        
        return: None
        """
        velocity_x = 0
        velocity_y = 0
        velocity_z = -0.25
        
        msg = self.vehicle.message_factory.set_position_target_local_ned_encode(
                0,       # time_boot_ms (not used)
                0, 0,    # target system, target component
                mavutil.mavlink.MAV_FRAME_BODY_OFFSET_NED, # frame Needs to be MAV_FRAME_BODY_NED for forward/back left/right control.
                0b0000111111000111, # type_mask
                0, 0, 0, # x, y, z positions (not used)
                velocity_x, velocity_y, velocity_z, # m/s
                0, 0, 0, # x, y, z acceleration
                0, 0)
        start_time = time.time()
        self.vehicle.send_mavlink(msg)
        self.vehicle.flush()
        end_time = time.time()
        latency = end_time - start_time
        print(f"move_up latency: {latency}")


    @performance_monitor
    def move_down(self) -> None:
        """
        Function: move_down()
        --------------------
        Let drone move down by sending mavlink message.
        
        return: None
        """
        velocity_x = 0
        velocity_y = 0
        velocity_z = 0.25
        
        msg = self.vehicle.message_factory.set_position_target_local_ned_encode(
                0,       # time_boot_ms (not used)
                0, 0,    # target system, target component
                mavutil.mavlink.MAV_FRAME_BODY_OFFSET_NED, # frame Needs to be MAV_FRAME_BODY_NED for forward/back left/right control.
                0b0000111111000111, # type_mask
                0, 0, 0, # x, y, z positions (not used)
                velocity_x, velocity_y, velocity_z, # m/s
                0, 0, 0, # x, y, z acceleration
                0, 0)
        start_time = time.time()
        self.vehicle.send_mavlink(msg)
        self.vehicle.flush()
        end_time = time.time()
        latency = end_time - start_time
        print(f"move_down latency: {latency}")
"""
def close_connection(self):
        self.vehicle.close()
"""
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
