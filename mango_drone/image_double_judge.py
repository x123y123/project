import jetson.inference
import jetson.utils
import threading
import time




#net = jetson.inference.detectNet("ssd-mobilenet-v2", threshold=0.5)
net = jetson.inference.detectNet(model="./models/model0111/ssd-mobilenet.onnx", labels="models/model0111/labels.txt", input_blob="input_0", output_cvg="scores", output_bbox="boxes", threshold=0.95)
net2 = jetson.inference.detectNet(model="./models/model0111/ssd-mobilenet.onnx", labels="models/model0111/labels.txt", input_blob="input_0", output_cvg="scores", output_bbox="boxes", threshold=0.95)

camera = jetson.utils.videoSource("/dev/video0")      # '/dev/video0' for V4L2
display = jetson.utils.videoOutput("display://0") # 'my_video.mp4' for file

img = None
task_output = None
task2_output = None
def rgb_task():
    #while display.IsStreaming():
    while True:
        print("task1 ACT!")
        time.sleep(1)
        detections = net.Detect(img)
    
        max_bb_center = 0.0
        max_area_det = 0.0
        width_bb = 0.0
        height_bb = 0.0
        for detection in detections:
            print('@@@@@@@@@@@@@@@@@@')
            print(detection.ClassID)
            print(detection.Area)
            print('!!!!!!!!!!!!!!!!!!')
            if detection.Area > max_area_det:
                max_area_det = detection.Area                                       
                max_bb_center = detection.Center
                width_bb = detection.Center[0]
                height_bb = detection.Center[1]
                print(type(max_bb_center[0]))
        #jetson.utils.cudaDrawCircle(img, (width_bb, height_bb), 10, (255, 0, 0))
        #jetson.utils.cudaDrawCircle(img, (img.width/2, img.height/2), 10, (255, 0, 0))
        #jetson.utils.cudaDrawCircle(img, (0, 0), 10, (0, 255, 0))
            #display.Render(img)
            task_output = (width_bb, height_bb)
            print("task1: {}".format(task_output))
            
def rgb_task2():
    #while display.IsStreaming():
    while True:
        print("task2 ACT!")
        time.sleep(1)
        detections = net2.Detect(img)
    
        max_bb_center = 0.0
        max_area_det = 0.0
        width_bb = 0.0
        height_bb = 0.0
        for detection in detections:
            print('@@@@@@@@@@@@@@@@@@')
            print(detection.ClassID)
            print(detection.Area)
            print('!!!!!!!!!!!!!!!!!!')
            if detection.Area > max_area_det:
                max_area_det = detection.Area                                       
                max_bb_center = detection.Center
                width_bb = detection.Center[0]
                height_bb = detection.Center[1]
                print(type(max_bb_center[0]))
        #jetson.utils.cudaDrawCircle(img, (width_bb, height_bb), 10, (255, 0, 0))
        #jetson.utils.cudaDrawCircle(img, (img.width/2, img.height/2), 10, (255, 0, 0))
        #jetson.utils.cudaDrawCircle(img, (0, 0), 10, (0, 255, 0))
            #display.Render(img)
            task2_output = (width_bb, height_bb)
            print("task2: {}".format(task2_output))
            
if __name__ == '__main__':
    t = threading.Thread(target = rgb_task)
    t2 = threading.Thread(target = rgb_task2)
    
    t.start()
    t2.start()
    
    
    
    while True:
        print("main ACT!")
        img = camera.Capture()
        time.sleep(1)
        
    t.join()
    t2.join()
    
    print("finish")
            
