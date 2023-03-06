import numpy as np
import cv2 as cv
from matplotlib import pyplot as plt   

cap = cv.VideoCapture(0)
flag = True
width = 7
height = 7

on_off = {True : "on", False : "off"}

print('-'*20)
print("core width:\n\t+2 - a\n\t-2 - s\ncore height:\n\t+2 - z\n\t-2 - x\nturn on/off Gaussian blur - w\nquit - q")
print('-'*20)

if not cap.isOpened():
    print("Cannot open camera")
    exit()
while True:
    # Capture frame-by-frame
    ret, frame = cap.read()
    # if frame is read correctly ret is True
    if not ret:
        print("Can't receive frame (stream end?). Exiting ...")
        break
    # Our operations on the frame come here
    if flag:
        frame = cv.GaussianBlur(frame, (width, height), 1.3)
    cv.putText(frame, f"{width} {height}", (490, 450), cv.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), thickness=2)
    cv.putText(frame, f"{on_off[flag]}", (50, 50), cv.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), thickness=2)
    # Display the resulting frame
    cv.imshow('frame', frame)
    key = cv.waitKey(1)
    if key == ord('q'):
        break
    if key == ord('w'):
        flag = not flag
    if key == ord('a'):
        width += 2
    if key == ord('s'):
        if width > 1:
            width -= 2
    if key == ord('z'):
        height += 2
    if key == ord('x'):
        if height > 1:
            height -= 2
# When everything done, release the capture
# cap.release()
cv.destroyAllWindows()