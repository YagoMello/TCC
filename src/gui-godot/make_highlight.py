import numpy as np
import matplotlib.pyplot as plt
import cv2 as cv
import sys

but_n = 1
k = 6
color = [0.25, 0.6, 1]

# varias funcoes uteis
dist = lambda x, y: np.sqrt(x**2 + y**2)
in_circle = lambda mp, r, x, y: r > dist(x - mp[0], y - mp[1])
to_lmag = lambda im: 20*np.log(np.abs(im)+1.)
cut = lambda im: im[0:int(im.shape[0]/2+0.5),0:int(im.shape[1]/2+0.5)]
ifft = lambda im: np.real(cut(np.fft.ifft2(np.fft.ifftshift(im))))
normalize = lambda im: np.interp(im, (im.min(), im.max()), (0, 1))

# a funcao que aplica o filtro
def filt(im_in, fn):
  im = im_in.copy()
  rs, cs = im.shape
  for i in range(0, rs-1):
    for j in range(0, cs-1):
      im[i, j] = fn(im, i, j, (rs/2.,cs/2.))
  return im

# faz o pad simetrico que o marcelo recomendou
def pad(im):
  rs, cs = im.shape
  return np.pad(im, ((0,rs),(0,cs)), 'symmetric')

def pad_zeros(im):
  rs, cs = im.shape
  diffr = int(max(rs - cs, 0) / 2)
  diffc = int(max(cs - rs, 0) / 2)
  pad_size = int(max(rs, cs))
  return np.pad(im, ((pad_size + diffc,pad_size + diffc),(pad_size + diffr,pad_size + diffr)), 'constant')

# pega a tf a partir da funcao
def get_tf(fn, dim):
  im = np.ones(dim)
  return filt(im, fn)


# AGORA SIM COMECA O CODIGO AQUI
# le a imagem
img_rgba = plt.imread(sys.argv[1] + sys.argv[2] + "." + sys.argv[3], cv.IMREAD_UNCHANGED)
img = pad_zeros(normalize(cv.cvtColor(img_rgba, cv.COLOR_BGR2GRAY)))

rows, cols = img.shape

# faz a fft
img_fft = np.fft.fftshift(np.fft.fft2(pad(img)))

# mostra a fft
#plt.imshow(to_lmag(img_fft), cmap='gray')
#plt.show()

def get_d0(dpi, m, f):
  dx = 25.4/dpi
  du = 1/(m * dx)
  return f / du

#k = get_d0(300, cols, 1.5)
#print(k)

# isso resume tudo que eh feito
def run_fn(img, img_fft, fn):
  # pega a img no dominio da frequencia e filtra
  img_fil_fft = filt(img_fft, fn)
  # faz a fft inversa
  img_fil = ifft(img_fil_fft)
  # mostra o filtro usado
  #plt.imshow(to_lmag(get_tf(fn, img_fft.shape)), cmap='gray')
  #plt.show()
  # mostra a magnitude na frequencia
  #plt.imshow(to_lmag(img_fil_fft), cmap='gray')
  #plt.show()
  # mostra a imagem original
  #plt.imshow(img, cmap='gray')
  #plt.show()
  # mostra a imagem filtrada
  #plt.imsave("filt.png", np.real(img_fil), cmap='gray')
  #plt.show()
  img_real = np.real(img_fil)
  return normalize(img_real)

# define a funcao q filtra
none = lambda im, i, j, mp: im[i, j]
ideal = lambda im, i, j, mp: im[i, j] if in_circle(mp, k, i, j) else 0
but = lambda im, i, j, mp: im[i, j] / (1+(dist(i-mp[0], j-mp[1])/k)**but_n)
but_hp = lambda im, i, j, mp: im[i, j] * (1 - 1/(1+(dist(i-mp[0], j-mp[1])/k)**but_n))
exp = lambda im, i, j, mp: im[i, j] * np.exp(-((i-mp[0])**2 + (j-mp[1])**2)/(2*(k**2)))

img_blur = run_fn(img, img_fft, but)

border_offset = max(np.max(img_blur[0,:]), np.max(img_blur[:,0]), np.max(img_blur[-1,:]), np.max(img_blur[-1,:]))

img_diff = np.clip(img_blur - img - border_offset, 0, 1)
#plt.imsave("filt2.png", img_diff, cmap='gray')


img_blue = np.ndarray((img_diff.shape[0], img_diff.shape[1], 4), np.float32)
img_blue[:, :, 0] = color[0]
img_blue[:, :, 1] = color[1]
img_blue[:, :, 2] = color[2]
img_blue[:, :, 3] = normalize(img_diff)
plt.imsave(sys.argv[1] + sys.argv[2] + "-highlight." + sys.argv[3], img_blue)

