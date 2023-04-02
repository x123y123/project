import jetson.inference
import jetson.utils
import control

def main():
    cnt = 0
    max_area_det = 0.0
    width_bb = 0.0
    height_bb = 0.0
    
    net = jetson.inference.detectNet(
                                     model = "models/model0113/ssd-mobilenet.onnx", 
                                     labels = "models/model0113/labels.txt", 
                                     input_blob = "input_0", 
                                     output_cvg = "scores", 
                                     output_bbox = "boxes", 
                                     threshold = 0.95
                                    )
    input = jetson.utils.videoSource("/dev/video0")      # '/dev/video0' for V4L2
    #display = jetson.utils.videoOutput("display://0") # 'my_video.mp4' for file

    connection_string = '/dev/ttyACM0'
    drone = drone_controller(connection_string)
    drone.takeoff(1)

    while True:
        img = input.Capture()
        detections = net.Detect(img)
    
        max_area_det = 0.0
        width_bb = 0.0
        height_bb = 0.0
        for detection in detections:
            print('@@@@@@@@@@@@@@@@@@')
            print(detection)
            print(detection.ClassID)
            print(detection.Area)
            print('!!!!!!!!!!!!!!!!!!')
            if detection.Area > max_area_det:
                max_area_det = detection.Area                                       
                width_bb = detection.Center[0]
                height_bb = detection.Center[1]

        if(not len(detections)):
            drone.move_up()
            print("move_up")
            cnt = cnt + 1
        else:
            if (not (width_bb - input.width)):
                if (width_bb < input.width):
                    drone.move_left()
                    print("move_left")
            
                elif (width_bb > input.width):
                    drone.move_right()
                    print("move_right")

            if (not (height_bb - input.height)):
                if (height_bb < input.height):
                    drone.move_down()
                    print("move_down")

                elif (height_bb > input.height):
                    drone.move_up()
                    print("move_up")
            
        if (cnt > 5):
            drone.move_down()
            print("Too high to danger!")
#       jetson.utils.cudaDrawCircle(img, (width_bb, height_bb), 10, (255, 0, 0))
#       jetson.utils.cudaDrawCircle(img, (img.width/2, img.height/2), 10, (255, 0, 0))
#       display.Render(img)
        # exit on input EOS
        if not input.IsStreaming():
            break

    drone.land()
    print("program finish!")

if __name__ = '__main__':
    main()
