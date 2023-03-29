import jetson.inference
import jetson.utils

#net = jetson.inference.detectNet("ssd-mobilenet-v2", threshold=0.5)
net = jetson.inference.detectNet(model="./models/model0111/ssd-mobilenet.onnx", labels="models/model0111/labels.txt", input_blob="input_0", output_cvg="scores", output_bbox="boxes", threshold=0.95)
camera = jetson.utils.videoSource("/dev/video0")      # '/dev/video0' for V4L2
display = jetson.utils.videoOutput("display://0") # 'my_video.mp4' for file

#while display.IsStreaming():
while True:
    img = camera.Capture()
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
    jetson.utils.cudaDrawCircle(img, (width_bb, height_bb), 10, (255, 0, 0))
    jetson.utils.cudaDrawCircle(img, (img.width/2, img.height/2), 10, (255, 0, 0))
#    jetson.utils.cudaDrawCircle(img, (0, 0), 10, (0, 255, 0))
    display.Render(img)
