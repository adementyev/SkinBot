#acessing the Raspberry Pi Camera with OpenCV and PythonPython

# import the necessary packages
from picamera.array import PiRGBArray
from picamera import PiCamera
import time
import cv2
import math
import numpy as np



def nothing(x):
    pass

cv2.namedWindow('image')
# initialize the camera and grab a reference to the raw camera capture
camera = PiCamera()
camera.resolution = (640, 480)
camera.framerate = 32
rawCapture = PiRGBArray(camera, size=(640, 480))

# allow the camera to warmup
time.sleep(0.1)

cv2.createTrackbar('lowerThresholdSlider',  'image', 0, 120 ,nothing)
cv2.createTrackbar('upperThresholdSlider',  'image', 130, 255 ,nothing)
cv2.createTrackbar('CircleSizeSliderMax',      'image',0 ,100, nothing)
cv2.createTrackbar('CircleSizeSliderMin',      'image',0 ,50, nothing)

cv2.setTrackbarPos('lowerThresholdSlider','image',0);
cv2.setTrackbarPos('upperThresholdSlider','image',23);
cv2.setTrackbarPos('CircleSizeSliderMin','image',12);
cv2.setTrackbarPos('CircleSizeSliderMax','image',34);


font = cv2.FONT_HERSHEY_SIMPLEX
center1=0
center2=0

# capture frames from the camera
for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
	# grab the raw NumPy array representing the image, then initialize the timestamp
	# and occupied/unoccupied text
	
	#define the image range
	image = frame.array
	#ret,thresh = cv2.threshold(image,127,255,cv2.THRESH_BINARY)
	imageGrayScale = cv2.cvtColor(image,cv2.COLOR_BGR2HSV);

	
	#Get the trackbar position
	lowerThreshold = cv2.getTrackbarPos('lowerThresholdSlider','image')
	upperThreshold = cv2.getTrackbarPos('upperThresholdSlider','image')
	CircleSizeMax = cv2.getTrackbarPos('CircleSizeSliderMax','image')
	CircleSizeMin = cv2.getTrackbarPos('CircleSizeSliderMin','image')
	
	lower_blue = np.array([lowerThreshold,50,50])
	upper_blue = np.array([upperThreshold,255,255])
	
	mask = cv2.inRange(imageGrayScale, lower_blue,upper_blue);
	
	
	im2, contours,hierarchy = cv2.findContours(mask,1,2)
	#reshapedContours = contours[0].reshape(-1,2)
	#(x,y),radius = cv2.minEnclosingCircle(contours)
	cv2.drawContours(image,contours,-1,(255,0,0),1)
	
	numberOfContourCircles = 0;
	for index in range(len(contours)):
            cnt=contours[index]
            (x,y),radius = cv2.minEnclosingCircle(cnt)
            center=(int(x),int(y))
            radius=int(radius)
            if radius<CircleSizeMax and radius >CircleSizeMin:
                cv2.circle(image,center,radius,(0,255,0),2)
                cv2.putText(image,str(radius),center,font,1,(255,255,255),2,cv2.LINE_AA)
                print("radius: ", radius, " x,y: " , center)
                numberOfContourCircles=numberOfContourCircles+1
                if numberOfContourCircles==1:
                    center1=center
                if numberOfContourCircles==2:
                    center2=center
                    
        

	if numberOfContourCircles==2:
            cv2.putText(image,"found markers",(30,30),font,1,(255,255,255),2,cv2.LINE_AA)
            cv2.line(image,center1,center2,(0,0,255),5)
            dx = center1[0] - center2[0]
            dy = center1[1] - center2[1]
            if dx!=0 and dy!=0: 
                theta = math.degrees(math.atan(dy/dx))
                midx=(center1[0] + center2[0])/2
                midy=(center1[1] + center2[1])/2
                distanceToCenter = math.sqrt((midy-(480/2))*(midy-(480/2)) + (midx - (640/2))*(midx - (640/2)))
                thetaDisplay = (int)(theta)
                distanceToCenterDisplay = (int)(distanceToCenter) 
                cv2.putText(image,"angle: " + str(thetaDisplay),(30,70),font,1,(255,255,255),2,cv2.LINE_AA)
                cv2.line(image,(int(midx),int(midy)),(int(640/2),int(480/2)),(0,0,255),5)
                cv2.circle(image,(int(640/2),int(480/2)),10,(0,255,255),2)
                cv2.putText(image,"distance: " + str(distanceToCenterDisplay),(30,100),font,1,(255,255,255),2,cv2.LINE_AA)
                
	cv2.imshow("Original", image)
	cv2.imshow("Gray", mask)
	#cv2.imshow("Threshold", imageHSV)
	#cv2.imshow("Frame", image)
	key = cv2.waitKey(1) & 0xFF
	#ret,thresh = cv2.threshold(image,127,255,0)
	print ("total contours: ", len(contours))
	#print ("done!")
        #cv2.imshow("Frame",thresh)
	# clear the stream in preparation for the next frame
	rawCapture.truncate(0)

 	# if the `q` key was pressed, break from the loop
	if key == ord("q"):
		break
