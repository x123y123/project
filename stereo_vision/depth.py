import numpy as np
import cv2

baseline = 6
focal = 2.6
alpha = 73
bias = 10 # will minus

def translate(image, x, y):
    M = np.float32([[1, 0, x], [0, 1, y]])
    shifted = cv2.warpAffine(image, M, (image.shape[1], image.shape[0]))
 
    return shifted
 
def gstreamer_pipeline(
        sensor_id,
        sensor_mode=3,
        capture_width=640,
        capture_height=480,
        display_width=640,
        display_height=480,
        framerate=20,
        flip_method=0,
):
    return (
            "nvarguscamerasrc sensor-id=%d sensor-mode=%d ! "
            "video/x-raw(memory:NVMM), "
            "width=(int)%d, height=(int)%d, "
            "format=(string)NV12, framerate=(fraction)%d/1 ! "
            "nvvidconv flip-method=%d ! "
            "video/x-raw, width=(int)%d, height=(int)%d, format=(string)BGRx ! "
            "videoconvert ! "
            "video/x-raw, format=(string)BGR ! appsink"
            % (
                sensor_id,
                sensor_mode,
                capture_width,
                capture_height,
                framerate,
                flip_method,
                display_width,
                display_height,
            )
     )
"""
def update(val = 0):
     
    blockSize = cv2.getTrackbarPos('blockSize', 'disparity')
 
    stereo.setP1(8*3*blockSize**2);
    stereo.setP2(32*3*blockSize**2);
 
    #print ('computing disparity...')
"""
def main():
    l_camera = cv2.VideoCapture(gstreamer_pipeline(sensor_id=0),cv2.CAP_GSTREAMER)
    r_camera = cv2.VideoCapture(gstreamer_pipeline(sensor_id=1),cv2.CAP_GSTREAMER)
         
    #create windows
    #cv2.namedWindow('left_Webcam', cv2.WINDOW_NORMAL)
    #cv2.namedWindow('right_Webcam', cv2.WINDOW_NORMAL)
    #cv2.namedWindow('disparity', cv2.WINDOW_NORMAL)
         
    l_camera.set(cv2.CAP_PROP_FRAME_WIDTH, 640);   
    l_camera.set(cv2.CAP_PROP_FRAME_HEIGHT, 480);
         
    r_camera.set(cv2.CAP_PROP_FRAME_WIDTH, 640);   
    r_camera.set(cv2.CAP_PROP_FRAME_HEIGHT, 480);  
                 
         
    blockSize = 5
         
    #cv2.createTrackbar('blockSize', 'disparity', blockSize, 60, update)   
         
    while(cv2.waitKey(1) & 0xFF != ord('q')):
        ret1, left_frame = l_camera.read()
        ret2, right_frame = r_camera.read()
                  
    
        right_frame = translate(right_frame,0,0)
             
        # our operations on the frame come here
        #left_frame = cv2.flip(left_frame,0)
        #right_frame = cv2.flip(right_frame,0)
        #cv2.imwrite('left.jpg',left_frame)
        #cv2.imwrite('right.jpg',right_frame)
        gray_left = cv2.cvtColor(left_frame, cv2.COLOR_BGR2GRAY)
        gray_right = cv2.cvtColor(right_frame, cv2.COLOR_BGR2GRAY)
        gray_left = cv2.flip(gray_left,0)
        gray_right = cv2.flip(gray_right,0)
        #cv2.imshow('left_Webcam', gray_left)
        #cv2.imshow('right_Webcam', gray_right)
             
             
        stereo = cv2.StereoSGBM_create(
                     minDisparity=6,
                     numDisparities=54,
                     blockSize=5,
                     uniquenessRatio = 5,
                     speckleWindowSize = 50,
                     speckleRange = 1,
                     disp12MaxDiff = 50,
                     P1 = 8*3*blockSize**2,
                     P2 = 32*3*blockSize**2)
            
        disparity = stereo.compute(gray_left, gray_right)
        disparity_normal = cv2.normalize(disparity, disparity ,0, 255,cv2.NORM_MINMAX)
        image = np.array(disparity_normal, dtype = np.uint8)
        disparity_color = cv2.applyColorMap(image, cv2.COLORMAP_BONE)
        #disparity = np.hstack((disparity_color, left_frame))
        disparity = cv2.flip(disparity_color,0)
        disparity = cv2.erode(disparity, None, iterations=1)
        disparity = cv2.dilate(disparity, None, iterations=1)
        disparity = cv2.flip(disparity,0)
            
        ######
            
        height_right, width_right = gray_right.shape
        height_left, width_left = gray_left.shape
            
        if width_right == width_left:
            f_pixel = (width_right * 0.5) / np.tan(alpha * 0.5 * np.pi/180)
        else:
            print('Left and right camera frames do not have the same pixel width')
        
        height, width = disparity.shape[:2]
        center = (int(width/2), int(height/2))
        #disparity_center = disparity[center[0]][center[1]]
        disparity_center = disparity[320,240]
        disp = int(np.mean(disparity_center))
        depth = ((baseline * f_pixel) / disp) - bias
        print(f"Depth: {depth} cm")
        #cv2.imshow('disparity', disparity)
            
        ######
    # When everything done, release the capture
    l_camera.release()
    r_camera.release()
    #cv2.destroyAllWindows()

if __name__ == '__main__':
    main()
