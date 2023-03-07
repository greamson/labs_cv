import numpy as np
import cv2
from matplotlib import pyplot as plt
num = 4
templates = np.array([[230, 950, 600, 1100],
		             [100, 330, 630, 820],
		             [30, 170, 270, 430],
		             [290, 410, 1160, 1280],
		             [50, 160, 670, 760]])
# print(templates[num][0])
# exit()
# [230:950, 600:1100] for 1 image
# sub_image = full_image[100:330, 630:820]  for 2 image
# sub_image = full_image[30:170, 270:430]   for 3 image
# sub_image = full_image[290:410, 1160:1280]for 4 image
# sub_image = full_image[50:160, 670:760]   for 5 image

query_img = cv2.imread(f'imgs/7.jpg')
# original_img = query_img[templates[num-1][0]:templates[num-1][1], templates[num-1][2]:templates[num-1][3]].copy()
original_img = cv2.imread('imgs/7_rubic.jpg')
query_img_bw = cv2.cvtColor(query_img, cv2.IMREAD_GRAYSCALE)
original_img_bw = cv2.cvtColor(original_img, cv2.IMREAD_GRAYSCALE)

orb = cv2.ORB_create()
queryKP, queryDes = orb.detectAndCompute(query_img_bw,None)
trainKP, trainDes = orb.detectAndCompute(original_img_bw,None)

matcher = cv2.BFMatcher(cv2.NORM_HAMMING, crossCheck=True)
matches = matcher.match(queryDes,trainDes)
matches = sorted(matches, key = lambda x:x.distance)

src_pts = np.float32([queryKP[m.queryIdx].pt for m in matches[:40]]).reshape(-1,1,2)
dst_pts = np.float32([trainKP[m.trainIdx].pt for m in matches[:40]]).reshape(-1,1,2)
M, mask = cv2.findHomography(src_pts, dst_pts, cv2.RANSAC,5.0)
matchesMask = mask.ravel().tolist()
pts = src_pts[mask==1]
min_x, min_y = np.int32(pts.min(axis=0))
max_x, max_y = np.int32(pts.max(axis=0))

a = cv2.rectangle(query_img,(min_x, min_y), (max_x,max_y), 255,2)
final_img = cv2.drawMatches(a, queryKP,
                            original_img, trainKP, matches[:40],None)

final_img = cv2.resize(final_img, (1000, 1000))
cv2.imshow("Result", final_img)
cv2.waitKey(0)

isWritten = cv2.imwrite(f"results/ORB/7_rubic.png", final_img)
# print(isWritten)
if isWritten:
	print('Image is successfully saved as file.')