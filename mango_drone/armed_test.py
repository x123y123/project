from dronekit import connect, VehicleMode, LocationGlobalRelative, APIException
import time
import socket
#import exceptions
import math
import argparse

def connectMyCopter():
    parser = argparse.ArgumentParser(description='Example showing how to set and clear vehicle channel-override information.')
    parser.add_argument('--connect', 
                           help="vehicle connection target string. If not specified, SITL automatically started and used.")
    args = parser.parse_args()
    connection_string = args.connect
    vehicle = connect(connection_string, wait_ready=True)
    return vehicle

def arm():
    while vehicle.is_armable == False:
        print("Waiting for vehicle to become armable...")
        time.sleep(1)
    print("!!!!!!!!!!! Vehicle is now armable ~~~~~~~~~")
    print("")

    vehicle.armed = True
    while vehicle.is_armable == False:
        print("Waiting for vehicle to become armable...")
        time.sleep(1)
    print("!!!!!!!!!!! Vehicle is now armable ~~~~~~~~~")
    print("props are spinning. LOOK OUT!!!")
    
    return None

vehicle = connectMyCopter()
vehicle.mode = VehicleMode("GUIDED_NOGPS")
arm()
print("End of testing")

