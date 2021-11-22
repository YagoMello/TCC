import numpy as np
import matplotlib.pyplot as plt
import cv2 as cv
import sys

normalize = lambda im: np.interp(im, (im.min(), im.max()), (0, 1))

def pad_zeros(im):
  rs, cs = im.shape
  return np.pad(im, ((int(rs/2),int(rs/2)),(int(cs/2),int(cs/2))), 'constant')

img_rgba = plt.imread(sys.argv[1] + sys.argv[2] + "." + sys.argv[3], cv.IMREAD_UNCHANGED)
img = normalize(pad_zeros(cv.cvtColor(img_rgba, cv.COLOR_BGR2GRAY)))

rows, cols = img.shape

img_blur = cv.GaussianBlur(img, (65, 65), 2)
img_diff = np.clip(img_blur - img, 0, 1)

#plt.imsave("filt.png", img_diff, cmap='gray')

img_blue = np.ndarray((img_diff.shape[0], img_diff.shape[1], 4), np.float32)
img_blue[:, :, 0] = 0.25
img_blue[:, :, 1] = 0.60
img_blue[:, :, 2] = 1.00
img_blue[:, :, 3] = normalize(img_diff)

plt.imsave(sys.argv[1] + sys.argv[2] + "-highlight." + sys.argv[3], img_blue)
 
