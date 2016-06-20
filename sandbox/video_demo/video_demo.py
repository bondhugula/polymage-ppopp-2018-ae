import ctypes
import numpy as np
import time
from cv2 import *
import sys
from common import clock, draw_str
from PIL import Image, ImageFilter
from numba import jit

@jit("float32[::](float32[::],int64,float64,int64,int64)",cache=True,nogil=True)
def unsharp_mask_cv(image,weight,thresh,rows,cols):
    mask=image
    kernelx = np.array([1,4,6,4,1],np.float32) / 16
    kernely = np.array([[1],[4],[6],[4],[1]],np.float32) / 16
    blury = sepFilter2D(image,-1,kernelx,kernely)
    sharpen = addWeighted(image,(1 + weight),blury,(-weight),0)
    th,choose = threshold(absdiff(image,blury),thresh,1,THRESH_BINARY)
    choose = choose.astype(bool)
    np.copyto(mask,sharpen,'same_kind',choose)
    return mask

@jit("uint8[::](uint8[::])",cache=True,nogil=True)
def unsharp_mask_pil(image):
    im = Image.fromarray(image)
    m = im.filter(ImageFilter.UnsharpMask(radius=2,percent=130,threshold=1))
    mask = np.array(m)
    return mask

@jit(nogil=True,cache=True,nopython=True)
def mini(x,y):
	if x < y:
		return x
	else:
		return y

@jit(nogil=True,cache=True,nopython=True)
def maxi(x,y):
	if x > y:
		return x
	else:
		return y

@jit("float32[::](uint8[::],float64,float64,int64,int64)",nogil=True,cache=True)
def unsharp_mask_numba(image,weight,threshold,r,c):
	image_f = np.float32(image) / 255.0
	mask = image_f
	tilex = 32
	tiley = 256
	im = np.zeros((3,(r+4),(c+4)),np.float32)

	for i in range(r+4):
		for j in range(c+4):
			for k in range(3):
				im[k,i,j] = image_f[i,j,k]

	blurx = np.zeros((3,tilex,tiley + 6),dtype=np.float32)
	blury = np.zeros((3,tilex,tiley + 6),dtype=np.float32)
	sharpen = np.zeros((3,tilex,tiley + 6),dtype=np.float32)

	for ti1 in range((int(r + 1) / tilex) + 1):
		ct0 = mini(r + 1,(tilex * ti1) + tilex - 1)
		ct1 = maxi(2,(tilex * ti1))
		ct4 = mini(r + 1,(tilex * ti1) + tilex - 1)
		ct5 = maxi(2,(tilex * ti1))
		ct8 = mini(r + 1,(tilex * ti1) + tilex - 1)
		ct9 = maxi(2,tilex * ti1)
		ct12 = mini(r + 1,(tilex * ti1) + tilex - 1)
		ct13 = maxi(2,tilex * ti1)
		for ti2 in range(-1,(int(c + 3) / tiley) + 1):
			ct2 = mini(c + 3,(tiley*ti2) + tiley + 5)
			ct3 = maxi(0,(tiley * ti2))
			ct6 = mini(c + 1,(tiley * ti2) + tiley + 4)
			ct7 = maxi(2,(tiley * ti2) + 1)
			ct10=mini(c + 1,(tiley * ti2) + tiley + 3)
			ct11=maxi(2,(tiley * ti2) + 2)
			ct14=mini(c + 1,(tiley * ti2) + tiley + 2)
			ct15=maxi(2,(tiley * ti2) + 3)
			for i0 in range(3):
				for i1 in range(ct1,ct0 + 1):
					for i2 in range(ct3,ct2 + 1):
						#blurx[i0,(-32*ti1)+i1,(-256*ti2)+i2] = (image_f[i1-2,i2,i0] + image_f[i1-1,i2,i0]*4 + image_f[i1,i2,i0]*6 + image_f[i1+1,i2,i0]*4 + image_f[i1+2,i2,i0])
						#blurx[i0,(-32*ti1)+i1,(-256*ti2)+i2] *= 0.0625
						blurx[i0,(-tilex*ti1)+i1,(-tiley*ti2)+i2] = (im[i0,i1-2,i2]/16 + im[i0,i1-1,i2]*0.25 + im[i0,i1,i2]*6/16 + im[i0,i1+1,i2]*0.25 + im[i0,i1+2,i2]/16)


			for i0 in range(3):
				for i1 in range(ct5,ct4 + 1):
					for i2 in range(ct7,ct6 + 1):
						blury[i0,(-tilex*ti1)+i1,(-tiley*ti2)+i2] = (blurx[i0,(-tilex*ti1)+i1,-2+(-tiley*ti2)+i2]/16) + (blurx[i0,(-tilex*ti1)+i1,-1+(-tiley*ti2)+i2] * 0.25) + (blurx[i0,(-tilex*ti1)+i1,(-tiley*ti2)+i2] * 6.0/16) + (blurx[i0,(-tilex*ti1)+i1,1 + (-tiley*ti2)+i2] * 0.25) + (blurx[i0,(-tilex*ti1)+i1,2 + (-tiley*ti2)+i2]/16)


			for i0 in range(3):
				for i1 in range(ct9,ct8 + 1):
					for i2 in range(ct11,ct10 + 1):
						#sharpen[i0,(-32*ti1)+i1,(-256*ti2)+i2] = image_f[i1,i2,i0] * (1+weight) - blury[i0,(-32*ti1)+i1,(-256*ti2)+i2] * weight

						sharpen[i0,(-tilex*ti1)+i1,(-tiley*ti2)+i2] = im[i0,i1,i2] * (1+weight) - blury[i0,(-tilex*ti1)+i1,(-tiley*ti2)+i2] * weight


			for i0 in range(3):
				for i1 in range(ct13,ct12 + 1):
					for i2 in range(ct15,ct14 + 1):
						#if image_f[i1,i2,i0] - blury[i0,(-32*ti1)+i1,(-256*ti2)+i2] < threshold:
							#mask[i1,i2,i0]=image_f[i1,i2,i0]
						if im[i0,i1,i2] - blury[i0,(-tilex*ti1)+i1,(-tiley*ti2)+i2] < threshold:
							mask[i1,i2,i0] = im[i0,i1,i2]
						else:
							mask[i1,i2,i0] = sharpen[i0,(-tilex*ti1)+i1,(-tiley*ti2)+i2]

	return mask

# load polymage shared libraries
libharris = ctypes.cdll.LoadLibrary("./harris.so")
libharris_naive = ctypes.cdll.LoadLibrary("./harris_naive.so")
libunsharp = ctypes.cdll.LoadLibrary("./unsharp.so")
libunsharp_naive = ctypes.cdll.LoadLibrary("./unsharp_naive.so")
libbilateral = ctypes.cdll.LoadLibrary("./bilateral.so")
libbilateral_naive = ctypes.cdll.LoadLibrary("./bilateral_naive.so")
liblaplacian = ctypes.cdll.LoadLibrary("./laplacian.so")
liblaplacian_naive = ctypes.cdll.LoadLibrary("./laplacian_naive.so")

harris = libharris.pipeline_harris
harris_naive = libharris_naive.pipeline_harris_naive

unsharp = libunsharp.pipeline_mask
unsharp_naive = libunsharp_naive.pipeline_mask_naive

bilateral = libbilateral.pipeline_bilateral
bilateral_naive = libbilateral_naive.pipeline_bilateral_naive

laplacian = liblaplacian.pipeline_laplacian
laplacian_naive = liblaplacian_naive.pipeline_laplacian_naive

cap = VideoCapture(sys.argv[1])

cv_mode = False
naive_mode = False
pil_mode = False
numba_mode = False

harris_mode = False
unsharp_mode = False
bilateral_mode = False
laplacian_mode = False

thresh = 0.001
weight = 3

levels = 4
alpha = 1.0/(levels-1)
beta = 1.0

modes = ['Unsharp Mask (Naive)','Unsharp Mask (Opt)','Laplacian (Naive)','Laplacian (Opt)',\
            'Bilateral (Naive)','Bilateral (Opt)','Harris (OpenCV)','Unsharp Mask (OpenCV)', \
            'Harris (Naive)','Harris (Opt)', 'Unsharp Mask (PIL)', 'Unsharp Mask (Numba)']

"""Dictionary for accumulators"""
sums = {}
for mode in modes:
    sums[mode] = 0.0

"""Dictionary for frames"""
frames = {}
for mode in modes:
    frames[mode] = 0

libharris_naive.pool_init()
libharris.pool_init()

libunsharp_naive.pool_init()
libunsharp.pool_init()

liblaplacian_naive.pool_init()
liblaplacian.pool_init()

libbilateral_naive.pool_init()
libbilateral.pool_init()

while(cap.isOpened()):
    ret, frame = cap.read()
    frameStart = clock()
    rows = frame.shape[0]
    cols = frame.shape[1]
    if harris_mode:
        if cv_mode:
            gray = cvtColor(frame, COLOR_BGR2GRAY)
            gray = np.float32(gray) / 4.0
            res = cornerHarris(gray, 3, 3, 0.04)
        else:
            res = np.empty((rows, cols), np.float32)
            if naive_mode:
                harris_naive(ctypes.c_int(cols-2), \
                             ctypes.c_int(rows-2), \
                             ctypes.c_void_p(frame.ctypes.data), \
                             ctypes.c_void_p(res.ctypes.data))
            else:
                harris(ctypes.c_int(cols-2), \
                       ctypes.c_int(rows-2), \
                       ctypes.c_void_p(frame.ctypes.data), \
                       ctypes.c_void_p(res.ctypes.data))

    elif unsharp_mode:
        if cv_mode:
            frame = np.float32(frame) / 255.0
            res = unsharp_mask_cv(frame,weight,thresh,rows,cols)
        elif pil_mode:
            res = unsharp_mask_pil(frame)
        elif numba_mode:
            res = unsharp_mask_numba(frame,weight,thresh,rows-4,cols-4)
        else:
            res = np.empty((rows-4, cols-4, 3), np.float32)
            if naive_mode:
                unsharp_naive(ctypes.c_int(cols - 4), \
                          ctypes.c_int(rows - 4), \
                          ctypes.c_float(thresh), \
                          ctypes.c_float(weight), \
                          ctypes.c_void_p(frame.ctypes.data), \
                          ctypes.c_void_p(res.ctypes.data))
            else:
                unsharp(ctypes.c_int(cols-4), \
                    ctypes.c_int(rows-4), \
                    ctypes.c_float(thresh), \
                    ctypes.c_float(weight), \
                    ctypes.c_void_p(frame.ctypes.data), \
                    ctypes.c_void_p(res.ctypes.data))

    elif laplacian_mode:
        total_pad = 92
        # result array
        res = np.empty((rows, cols, 3), np.uint8)

        if naive_mode:
            laplacian_naive(ctypes.c_int(cols+total_pad), \
                            ctypes.c_int(rows+total_pad), \
                            ctypes.c_float(alpha), \
                            ctypes.c_float(beta), \
                            ctypes.c_void_p(frame.ctypes.data), \
                            ctypes.c_void_p(res.ctypes.data))
        else:
            laplacian(ctypes.c_int(cols+total_pad), \
                      ctypes.c_int(rows+total_pad), \
                      ctypes.c_float(alpha), \
                      ctypes.c_float(beta), \
                      ctypes.c_void_p(frame.ctypes.data), \
                      ctypes.c_void_p(res.ctypes.data))

    elif bilateral_mode:
        res = np.empty((rows, cols), np.float32)
        if naive_mode:
            bilateral_naive(ctypes.c_int(cols+56), \
                            ctypes.c_int(rows+56), \
                            ctypes.c_void_p(frame.ctypes.data), \
                            ctypes.c_void_p(res.ctypes.data))
        else:
            bilateral(ctypes.c_int(cols+56), \
                      ctypes.c_int(rows+56), \
                      ctypes.c_void_p(frame.ctypes.data), \
                      ctypes.c_void_p(res.ctypes.data))


    else:
        res = frame

    frameEnd = clock()
    value = frameEnd*1000-frameStart*1000

    """Conditions to sum the values of frame delay accumulators and frame counters deoending on the mode"""
    if harris_mode:
        if cv_mode:
            sums['Harris (OpenCV)'] += value
            frames['Harris (OpenCV)'] += 1
        elif naive_mode:
            sums['Harris (Naive)'] += value
            frames['Harris (Naive)'] += 1
        else:
            sums['Harris (Opt)'] += value
            frames['Harris (Opt)'] += 1
    elif unsharp_mode:
        if cv_mode:
            sums['Unsharp Mask (OpenCV)'] += value
            frames['Unsharp Mask (OpenCV)'] += 1
        elif pil_mode:
            sums['Unsharp Mask (PIL)'] += value
            frames['Unsharp Mask (PIL)'] += 1
        elif numba_mode:
            sums['Unsharp Mask (Numba)'] += value
            frames['Unsharp Mask (Numba)'] += 1
        elif naive_mode:
            sums['Unsharp Mask (Naive)'] += value
            frames['Unsharp Mask (Naive)'] += 1
        else:
            sums['Unsharp Mask (Opt)'] += value
            frames['Unsharp Mask (Opt)'] += 1

    elif laplacian_mode:
        if naive_mode:
            sums['Laplacian (Naive)'] += value
            frames['Laplacian (Naive)'] += 1
        else:
            sums['Laplacian (Opt)'] += value
            frames['Laplacian (Opt)'] += 1

    elif bilateral_mode:
        if naive_mode:
            sums['Bilateral (Naive)'] += value
            frames['Bilateral (Naive)'] += 1
        else:
            sums['Bilateral (Opt)'] += value
            frames['Bilateral (Opt)'] += 1

    rectangle(res, (0, 0), (750, 150), (255, 255, 255), thickness=cv.CV_FILLED)
    draw_str(res, (40, 40),      "frame interval :  %.1f ms" % value)
    if (cv_mode and harris_mode) or (cv_mode and unsharp_mode):
        draw_str(res, (40, 80),  "Pipeline        :  " + str("OpenCV"))
    elif pil_mode and unsharp_mode:
        draw_str(res, (40, 80),  "Pipeline        :  " + str("PIL"))
    elif numba_mode and unsharp_mode:
        draw_str(res, (40, 80),  "Pipeline        :  " + str("Numba"))
    elif bilateral_mode or harris_mode or unsharp_mode or laplacian_mode:
        if naive_mode:
            draw_str(res, (40, 80),  "Pipeline        :  " + str("PolyMage (Naive)"))
        else:
            draw_str(res, (40, 80),  "Pipeline        :  " + str("PolyMage (Opt)"))
    else:
        draw_str(res, (40, 80),  "Pipeline        :  ")

    if harris_mode:
        draw_str(res, (40, 120), "Benchmark    :  " + str("Harris Corner"))
    elif bilateral_mode:
        draw_str(res, (40, 120), "Benchmark    :  " + str("Bilateral Grid"))
    elif unsharp_mode:
        draw_str(res, (40, 120), "Benchmark    :  " + str("Unsharp Mask"))
    elif laplacian_mode:
        draw_str(res, (40, 120), "Benchmark    :  " + str("Local Laplacian"))
    else:
        draw_str(res, (40, 120), "Benchmark    :  ")

    imshow('Video', res)

    ch = 0xFF & waitKey(1)
    if ch == ord('q'):
        break
    if ch == ord(' '):
        cv_mode = not cv_mode
        pil_mode = False
        naive_mode = False
    if ch == ord('n'):
        naive_mode = not naive_mode
        pil_mode = False
        numba_mode = False
        cv_mode = False
    if ch == ord('p'):
        pil_mode = not pil_mode
        numba_mode = False
        naive_mode = False
        cv_mode = False
    if ch == ord('m'):
        numba_mode = not numba_mode
        pil_mode = False
        naive_mode = False
        cv_mode = False
    if ch == ord('h'):
        harris_mode = not harris_mode
        bilateral_mode = False
        unsharp_mode = False
        laplacian_mode = False
        pil_mode = False
        naive_mode = False
    if ch == ord('u'):
        unsharp_mode = not unsharp_mode
        bilateral_mode = False
        harris_mode = False
        laplacian_mode = False
    if ch == ord('l'):
        laplacian_mode = not laplacian_mode
        unsharp_mode = False
        bilateral_mode = False
        harris_mode = False
        pil_mode = False
        naive_mode = False
        cv_mode = False
    if ch == ord('b'):
        bilateral_mode = not bilateral_mode
        harris_mode = False
        unsharp_mode = False
        laplacian_mode = False
        pil_mode = False
        naive_mode = False
        cv_mode = False


libharris_naive.pool_destroy()
libharris.pool_destroy()

libunsharp_naive.pool_destroy()
libunsharp.pool_destroy()

liblaplacian_naive.pool_destroy()
liblaplacian.pool_destroy()

libbilateral_naive.pool_destroy()
libbilateral.pool_destroy()

cap.release()
destroyAllWindows()

"""Printing values with dictionary"""
for mode in frames:
    if frames[mode]!=0:
        print "Average frame delay for ",mode," is - ",sums[mode]/frames[mode],"ms"
