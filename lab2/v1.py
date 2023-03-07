import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle
from skimage.metrics import structural_similarity as ssim
import cv2


def template_match(image, template, step=2):
    full_w,full_h = image.shape[:2]
    sub_w,sub_h = template.shape[:2]
    winW = 0
    found = False
    while winW < full_w - sub_w and found == False:
        winH = 0
        while winH < full_h - sub_h:
            window = image[winW:winW+sub_w, winH:winH+sub_h]
            if ssim(template, window) > 0.80:
                found = True
                print("found", ssim(template, window))
                ig, ax = plt.subplots()
                plt.imshow(image, cmap='gray')
                rect = Rectangle((winH, winW), sub_h, sub_w, edgecolor='r', facecolor='none')
                print(winW, winH, sub_w, sub_h)
                ax.add_patch(rect)
                plt.show()
                cv2.waitKey(0)
                break
            winH += step
        winW += step
    print("didnt found")

full_image =  cv2.imread("imgs/apple_tree.jpg", 0)
sub_image = cv2.imread("imgs/apple.jpg", 0)
# cv2.imwrite("imgs/apple.jpg", sub_image)
# plt.imshow(sub_image, cmap='gray')
# plt.show()


# plt.imshow(full_image, cmap='gray')
# plt.show()

# cv2.waitKey(0)

template_match(full_image, sub_image, 10)

    

# sub_image = full_image[230:950, 600:1100] for 1 image
# sub_image = full_image[100:330, 630:820]  for 2 image
# sub_image = full_image[30:170, 270:430]   for 3 image
# sub_image = full_image[290:410, 1160:1280]for 4 image
# sub_image = full_image[50:160, 670:760]   for 5 image
# sub_image = full_image[680:1200, 540:1000]for 6 image
# sub_image = full_image[1530:1930, 740:1020]for keys
# sub_image = full_image[830:1280, 140:570]  for rubic
# sub_image = full_image[920:1220, 1070:1400]for bracelet