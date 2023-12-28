#%%

from make_header import *
import matplotlib.pyplot as plt
import numpy as np
import scipy.fft as fft
from scipy.signal import savgol_filter

%matplotlib tk

hl,hr = load()

plt.plot(hl[4][0][:])

plt.figure(1)
hlval = np.abs(fft.rfft(hl[4][0][:]))
hrval = np.abs(fft.rfft(hl[4][0][:]))

plt.figure(2)
hfreqs = fft.rfftfreq(d=1/44100.,n=128)
plt.semilogx(hfreqs,hlval)
plt.semilogx(hfreqs,hrval/hlval)
plt.xlim([20,20000])

plt.figure(3)
hlvalsmooth = savgol_filter(hlval, 20, 5)
plt.plot(hfreqs,hlval)
plt.plot(hfreqs,hlvalsmooth)
#plt.xlim([20,20000])

plt.figure(4)
plt.semilogx(hfreqs,hlval)
plt.semilogx(hfreqs,hlval/(np.sqrt(hlvalsmooth)))


plt.figure(5)
hfreqs = fft.rfftfreq(d=1/44100.,n=128)
plt.plot(hl[4][0][:])
plt.plot(fft.irfft(fft.rfft(hl[4][0][:])/hlval))
plt.plot(fft.irfft(fft.rfft(hl[4][0][:])/(np.sqrt(hlvalsmooth))))

# %%
