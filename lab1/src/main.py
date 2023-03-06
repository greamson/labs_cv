
import cv2 as cv
from matplotlib import pyplot as plt
import numpy as np
import time
import os

def gaussian_filter(img, K_size=3, sigma=1.3):

    H, W, C = img.shape
 
    ## Zero padding
    pad = K_size // 2
    out = np.zeros((H + pad * 2, W + pad * 2, C), dtype=float)
    out[pad: pad + H, pad: pad + W] = img.copy().astype(float)
    
    ## prepare Kernel
    K = np.zeros((K_size, K_size), dtype=float)
    for x in range(-pad, -pad + K_size):
        for y in range(-pad, -pad + K_size):
            K[y + pad, x + pad] = np.exp( -(x ** 2 + y ** 2) / (2 * (sigma ** 2)))
    K /= (2 * np.pi * sigma * sigma)
    K /= K.sum()
    tmp = out.copy()

    # filtering
    for y in range(H):
        for x in range(W):
            for c in range(C):
                out[pad + y, pad + x, c] = np.sum(K * tmp[y: y + K_size, x: x + K_size, c])

    out = np.clip(out, 0, 255)
    out = out[pad: pad + H, pad: pad + W].astype(np.uint8)
    return out

if __name__ == '__main__':
    img = cv.imread('img.jpg')
    coreSizes = (3, 5, 7, 9, 11, 13, 23, 47, 79, 99)
    results = np.zeros((3, len(coreSizes)))

    for i in range(len(coreSizes)):
        t0 = time.process_time()
        img_1 = cv.GaussianBlur(img, (coreSizes[i], coreSizes[i]), 1.3)
        t1 = time.process_time() - t0
        results[0][i] = t1

        img_2 = gaussian_filter(img, coreSizes[i], 1.3)
        t2 = time.process_time() - t1
        results[1][i] = t2

        os.system(f"3/main img.jpg {coreSizes[i]}")
        t3 = time.process_time() - t2
        results[2][i] = t3

        cv.imwrite(f"results/res1_{i}.png", img_1)
        cv.imwrite(f"results/res2_{i}.png", img_2)


    fig, axs = plt.subplots(nrows= 3 , ncols= 1 )
    fig.suptitle('Зависимость производительности от размера ядра')
    axs[2].set_xlabel("Размер ядра")
    axs[0].set_ylabel("Время(с)")
    axs[1].set_ylabel("Время(с)")
    axs[2].set_ylabel("Время(с)")
    axs[0].plot(coreSizes, results[0], 'r',  label='python+opencv')
    axs[1].plot(coreSizes, results[1], 'g', label='python native')
    axs[2].plot(coreSizes, results[2], 'b', label='C++')
    fig.legend()
    plt.show()  

    file = open("results.txt", "w")
    for i in range(3):
        file.write(f"{i} method: \n")
        for el in results[i]:
            file.write('\t'+str(el)+'\n')
    
    cv.waitKey(0)
    cv.destroyAllWindows()