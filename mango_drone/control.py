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
    def __init__(self, connection string):
        self.vehicle = connect(connection string, wait_ready=True)

    def takeoff(self, altitude):
        self.vehicle.mode = VehicleMode("GUIDED")
        self.vehicle.simple_takeoff(altitude)

        while True:
            if self.vehicle.location.global_relative_frame.alt >= altitude * 0.95:
                break
            time.sleep(1)

    def land(self):
        self.vehicle.mode = VehicleMode("LAND")
        
        while True:
            if self.vehicle.location.global_relative_frame.alt <= 0.1:
                break
            time.sleep(1)
        
        self.vehicle.close()
    
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

############MAIN###############
if __name__ == '__main__':
    connection_string = '/dev/ttyACM0'
    control = drone_controller(connection_string)

