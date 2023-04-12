import jetson.inference
import jetson.utils
from  control import drone_controller
import time

def main():
    cnt = 0
    max_area_det = 0.0
    width_bb = 0.0
    height_bb = 0.0
    
    net = jetson.inference.detectNet(
                                     model = "models/model0114/ssd-mobilenet.onnx", 
                                     labels = "models/model0114/labels.txt", 
                                     input_blob = "input_0", 
                                     output_cvg = "scores", 
                                     output_bbox = "boxes", 
                                     threshold = 0.95
                                    )
    input = jetson.utils.videoSource("/dev/video0")      # '/dev/video0' for V4L2
    #display = jetson.utils.videoOutput("display://0") # 'my_video.mp4' for file
    display = jetson.utils.videoOutput("/home/uav/code/mango_drone/test.mp4") # 'my_video.mp4' for file

    connection_string = '/dev/ttyACM0'
    drone = drone_controller(connection_string)
    drone.takeoff(1.25)
    #time.sleep(1)

    while True:
        img = input.Capture()
        detections = net.Detect(img)
        
        imgcenter_width = img.width / 2
        imgcenter_height = img.height / 2
        max_area_det = 0.0
        width_bb = 0.0
        height_bb = 0.0
        for detection in detections:
            width_bb = detection.Center[0]
            height_bb = detection.Center[1]
            #if detection.Area > max_area_det:
                #max_area_det = detection.Area                                       
        
        if(len(detections) > 0):
            print("ACT!")
            if ((height_bb - imgcenter_height) != 0):
                if (height_bb < imgcenter_height):
                    print("move_up")
                    drone.move_up()

                elif (height_bb > imgcenter_height):
                    print("move_down")
                    drone.move_down()
            
            if ((width_bb - imgcenter_width) != 0):
                if (width_bb < imgcenter_width):
                    print("move_left")
                    drone.move_left()
            
                elif (width_bb > imgcenter_width):
                    print("move_right")
                    drone.move_right()

#        jetson.utils.cudaDrawCircle(img, (width_bb, height_bb), 10, (255, 0, 0))
#        jetson.utils.cudaDrawCircle(img, (img.width/2, img.height/2), 10, (255, 0, 0))
        display.Render(img)
        # exit on input EOS
        if not input.IsStreaming():
            break

    drone.land()
    print("program finish!")

if __name__ == '__main__':
    main()
