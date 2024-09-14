#%%

from make_header import *
import matplotlib.pyplot as plt
import numpy as np
import scipy.fft as fft
from scipy.signal import savgol_filter, iirnotch, lfilter

%matplotlib tk

hl,hr = load()

hfreqs = fft.rfftfreq(d=1/44100.,n=128)

plt.plot(hl[4][0][:])

plt.figure(1)
hlval = np.abs(fft.rfft(hl[4][0][:]))
hrval = np.abs(fft.rfft(hr[4][0][:]))

#%% CORRECTION Type 1
correction = np.linspace(0.,1.,65)

print(len(hlval))

hlvalc = (1-correction)*hlval+correction
hrvalc = (1-correction)*hrval+correction

plt.figure(2)
plt.plot(correction)

#%% CORRECTION Type 2


#%%

plt.figure(2)
plt.semilogx(hfreqs,20*np.log10(abs(1/hlval)))
# plt.semilogx(hfreqs,)
plt.xlim([20,20000])

#%%
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
plt.plot(fft.irfft(fft.rfft(hl[4][0][:])/hlvalsmooth))

# %%
