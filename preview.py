import numpy as np
import cv2

cap = cv2.VideoCapture(0)
n=0

while(True):
    # Capture frame-by-frame
    ret, frame = cap.read()
    #  768, 430,
    # 480 640
    frame = frame[:, 186:454, :]
    frame = cv2.resize(frame, (430,768), interpolation = cv2.INTER_AREA)
    # Our operations on the frame come here
    # rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    frame[:,:,0] = frame[:,:,0] *0.8
    frame[:,:,1] = frame[:,:,1] *0.8
    frame[:,:,2] = frame[:,:,2] *0.8

    # Display the resulting frame
    cv2.imshow('frame', frame)
    if cv2.waitKey(1) & 0xFF == ord('c'):
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        cv2.imwrite('image'+str(n)+'.png', gray)
        n+=1

# When everything done, release the capture
cap.release()
cv2.destroyAllWindows()