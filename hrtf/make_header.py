# hrtf/load_hrtf_mit_kemar.py
#
# -------------------------------------------------
# Imports HRTF data and save the data in a header
# for use in C++ hrtf projects
# -------------------------------------------------
#
# Part of BiRR package
# (c) OD - 2023
# https://github.com/odoare/pynoverb

import numpy as np
import os
import wave as _wave
import numpy as _np
import scipy.fft as _fft
from copy import deepcopy
from scipy.signal import savgol_filter, resample
import matplotlib.pyplot as plt

BIGVALUE = '1e9'
ELEVATIONS = np.array([-40,-30,-20,-10,0,10,20,30,40,50,60,70,80,90])
AZIMUTHSTEPS = np.array([6.43,6,5,5,5,5,5,6,6.43,8,10,15,30,360])
CHEMIN = os.path.dirname(os.path.realpath(__file__))+'/'
L44 = 128
L48 = 139
L88 = 256
L96 = 278

def _wav2array(nchannels, sampwidth, data):
    """data must be the string containing the bytes from the wav file."""
    num_samples, remainder = divmod(len(data), sampwidth * nchannels)
    if remainder > 0:
        raise ValueError('The length of data is not a multiple of '
                         'sampwidth * num_channels.')
    if sampwidth > 4:
        raise ValueError("sampwidth must not be greater than 4.")

    if sampwidth == 3:
        a = _np.empty((num_samples, nchannels, 4), dtype=_np.uint8)
        raw_bytes = _np.fromstring(data, dtype=_np.uint8)
        a[:, :, :sampwidth] = raw_bytes.reshape(-1, nchannels, sampwidth)
        a[:, :, sampwidth:] = (a[:, :, sampwidth - 1:sampwidth] >> 7) * 255
        result = a.view('<i4').reshape(a.shape[:-1])
    else:
        # 8 bit samples are stored as unsigned ints; others as signed ints.
        dt_char = 'u' if sampwidth == 1 else 'i'
        a = _np.fromstring(data, dtype='<%s%d' % (dt_char, sampwidth))
        result = a.reshape(-1, nchannels)
    return result


def readwav(file):
    """
    Read a WAV file.

    Parameters
    ----------
    file : string or file object
        Either the name of a file or an open file pointer.

    Return Values
    -------------
    rate : float
        The sampling frequency (i.e. frame rate)
    sampwidth : float
        The sample width, in bytes.  E.g. for a 24 bit WAV file,
        sampwidth is 3.
    data : numpy array
        The array containing the data.  The shape of the array is
        (num_samples, num_channels).  num_channels is the number of
        audio channels (1 for mono, 2 for stereo).

    Notes
    -----
    This function uses the `wave` module of the Python standard libary
    to read the WAV file, so it has the same limitations as that library.
    In particular, the function does not read compressed WAV files.

    """
    wav = _wave.open(file)
    rate = wav.getframerate()
    nchannels = wav.getnchannels()
    sampwidth = wav.getsampwidth()
    nframes = wav.getnframes()
    data = wav.readframes(nframes)
    wav.close()
    array = _wav2array(nchannels, sampwidth, data)
    return rate, sampwidth, array


def writewav24(filename, rate, data):
    """
    Create a 24 bit wav file.

    Parameters
    ----------
    filename : string
        Name of the file to create.
    rate : float
        The sampling frequency (i.e. frame rate) of the data.
    data : array-like collection of integer or floating point values
        data must be "array-like", either 1- or 2-dimensional.  If it
        is 2-d, the rows are the frames (i.e. samples) and the columns
        are the channels.

    Notes
    -----
    The data is assumed to be signed, and the values are assumed to be
    within the range -1..1.  Floating point values are
    converted to integers.  The data is rescaled or normalized before
    writing it to the file.

    Example
    -------
    Create a 3 second 440 Hz sine wave.

    >>> rate = 22050  # samples per second
    >>> T = 3         # sample duration (seconds)
    >>> f = 440.0     # sound frequency (Hz)
    >>> t = np.linspace(0, T, T*rate, endpoint=False)
    >>> x = (2**23 - 1) * np.sin(2 * np.pi * f * t)
    >>> writewav24("sine24.wav", rate, x)

    """
    a32 = _np.asarray(data*(2**23-1), dtype=_np.int32)
    if a32.ndim == 1:
        # Convert to a 2D array with a single column.
        a32.shape = a32.shape + (1,)
    # By shifting first 0 bits, then 8, then 16, the resulting output
    # is 24 bit little-endian.
    a8 = (a32.reshape(a32.shape + (1,)) >> _np.array([0, 8, 16])) & 255
    wavdata = a8.astype(_np.uint8).tostring()

    w = _wave.open(filename, 'wb')
    w.setnchannels(a32.shape[1])
    w.setsampwidth(3)
    w.setframerate(rate)
    w.writeframes(wavdata)
    w.close()

def list2cpparray(l):
    out = '{ '
    for i,e in enumerate(l):
        if i==len(l)-1:
            out +=  str(e)   
        else:
            out += str(e)+', '
    out += '}'
    return out

def list2cpparray2(l,n,big_value):
    out = '{ '
    for i in range(n):
        if i==n-1:
            out += '{:.2f}'.format(l[i])+'f' if i<len(l) else big_value   
        else:
            out += '{:.2f}'.format(l[i])+'f'+', ' if i<len(l) else big_value+', '
    out += '}'
    return out

def list2cpparray3(l):
    out = '{ '
    for i,e in enumerate(l):
        if i==len(l)-1:
            out +=  '{:.7e}'.format(e)+'f'   
        else:
            out += '{:.7e}'.format(e)+'f'+', '
    out += '}'
    return out

def load():
    # location = os.path.dirname(os.path.realpath(__file__))
    # chemin = location+'/'
    # Load HRTFs
    # ELEVATIONS = np.array([-40,-30,-20,-10,0,10,20,30,40,50,60,70,80,90])
    # AZIMUTHSTEPS = np.array([6.43,6,5,5,5,5,5,6,6.43,8,10,15,30,360])
    lmax = len(range(0,360,5))
    #AZIMUTH = range(0,360,5)
    #ELEVATION = 0

    NORM = 2**15
    lhrtf = np.zeros((len(ELEVATIONS),lmax,L44))
    rhrtf = np.zeros((len(ELEVATIONS),lmax,L44))
    for ind1,el in enumerate(ELEVATIONS):
        #print(ind1)
        for ind2,az in enumerate(np.arange(0,360,AZIMUTHSTEPS[ind1])):
            if az<=180:
                fich = CHEMIN+'mitcompact/H'+str(el)+'e'+str(round(az)).zfill(3)+'a.wav'
                #print(fich)
                #fs,y = wavfile.read(fich)
                fs, sw, y = readwav(file=fich)
                # print(type(y))
                # print(y.max())
                lhrtf[ind1,ind2,:] = y[:,0]
                rhrtf[ind1,ind2,:] = y[:,1]
            else:
                try:
                    fich = CHEMIN+'mitcompact/H'+str(el)+'e'+str(int(np.round(360-az))).zfill(3)+'a.wav'
                    #print(fich)
                    # fs,y = wavfile.read(fich)
                    fs, sw, y = readwav(file=fich)
                    # print(type(y))
                except:
                    fich = CHEMIN+'mitcompact/H'+str(el)+'e'+str(int(np.round(360-az)+1)).zfill(3)+'a.wav'
                    #print(fich)
                    # fs,y = wavfile.read(fich)
                    fs, sw, y = readwav(file=fich)
                    # print(type(y))
                lhrtf[ind1,ind2,:] = y[:,1]
                rhrtf[ind1,ind2,:] = y[:,0]

    lhrtf = lhrtf/NORM
    rhrtf = rhrtf/NORM

    return lhrtf, rhrtf

def normalize(lhrtf,rhrtf,elev_ref=4,azim_ref=0, hf_correction=False):
    """ Filter all the RIRs relative to one reference RIR
    """
    lh = deepcopy(lhrtf)
    rh = deepcopy(rhrtf)
    l = len(lhrtf[0][0][:])
    filt = (1/savgol_filter(_np.abs(_fft.rfft(lhrtf[elev_ref][azim_ref][:])),20,5))
    if hf_correction:
        correction = np.linspace(0.,1.,round(l/2+1))
    else:
        correction = np.zeros(round(l/2+1))
    filt = (1-correction)*filt+correction

    #filt = 1/_np.abs(_fft.rfft(lhrtf[elev_ref][azim_ref][:]))
    for ind1,el in enumerate(ELEVATIONS):
        for ind2,az in enumerate(np.arange(0,360,AZIMUTHSTEPS[ind1])):
            lh[ind1][ind2][:] = _fft.irfft(_fft.rfft(lhrtf[ind1][ind2][:])*filt)
            rh[ind1][ind2][:] = _fft.irfft(_fft.rfft(rhrtf[ind1][ind2][:])*filt)
    return lh, rh

def resample_hrtf(lhrtf,rhrtf,fs1,fs2):

    l = len(lhrtf[0][0][:])
    # print("l="+str(l)+"\n")
    lmax = len(range(0,360,5))

    newlength = int(round(l*fs2/fs1))
    lh = np.zeros((len(ELEVATIONS),lmax,newlength))
    rh = np.zeros((len(ELEVATIONS),lmax,newlength))
    for ind1,e in enumerate(ELEVATIONS):
        azim = np.arange(0,360,AZIMUTHSTEPS[ind1])
        for ind2,a in enumerate(azim):
            lh[ind1][ind2] = _np.zeros(newlength)
            rh[ind1][ind2] = _np.zeros(newlength)
            lh[ind1][ind2][:] = (fs1/fs2)*resample(lhrtf[ind1][ind2][:],newlength)
            rh[ind1][ind2][:] = (fs1/fs2)*resample(rhrtf[ind1][ind2][:],newlength)
    
    return lh,rh,newlength


def save(lh,rh, ext='44'):

    newlength = len(lh[0][0][:])

    azim_lengths = list()
    for i,a in enumerate(AZIMUTHSTEPS):
        azim_lengths.append(round(360/min(AZIMUTHSTEPS)))
    maxl = max(azim_lengths)

    with open(CHEMIN+'hrtf'+ext+'.h',mode='w') as f:
        f.write('#define NELEV '+str(len(ELEVATIONS))+'\n')
        f.write('#define NAZIM '+str(maxl)+'\n')
        f.write('#define NSAMP'+ext+' '+str(newlength)+'\n')
        f.write('#define BIGVALUE '+BIGVALUE+'\n')
        
        f.write('const float elevations[NELEV]=')
        f.write(list2cpparray(ELEVATIONS))
        f.write(';\n')

        f.write('const float azimuths[NELEV][NAZIM]={')
        for i,e in enumerate(ELEVATIONS):
            azims = np.arange(0,360,AZIMUTHSTEPS[i])
            f.write(list2cpparray2(azims,maxl,'BIGVALUE'))
            f.write(',\n')
        f.write('};\n')
        
        f.write('float lhrtf'+ext+'[NELEV][NAZIM][NSAMP'+ext+']={')
        for i,e in enumerate(ELEVATIONS):
            f.write('{')
            azim = np.arange(0,360,AZIMUTHSTEPS[i])
            for j,a in enumerate(azim):
                f.write(list2cpparray3(lh[i,j,:]))
                if j==len(azim)-1:
                    f.write('\n')
                else:
                    f.write(',\n')
            f.write('}')
            if i==len(ELEVATIONS)-1:
                f.write('\n')
            else:
                f.write(',\n')
        f.write('};\n')

        f.write('float rhrtf'+ext+'[NELEV][NAZIM][NSAMP'+ext+']={')
        for i,e in enumerate(ELEVATIONS):
            f.write('{')
            azim = np.arange(0,360,AZIMUTHSTEPS[i])
            for j,a in enumerate(azim):
                f.write(list2cpparray3(rh[i,j,:]))
                if j==len(azim)-1:
                    f.write('\n')
                else:
                    f.write(',\n')
            f.write('}')
            if i==len(ELEVATIONS)-1:
                f.write('\n')
            else:
                f.write(',\n')
        f.write('};\n')

if __name__ == '__main__':
    lhrtf, rhrtf = load()
    lhrtfn, rhrtfn = normalize(lhrtf,rhrtf,hf_correction=False)
    lhrtfnc, rhrtfnc = normalize(lhrtf,rhrtf,hf_correction=True)

    l44 = len(lhrtfnc[0][0][:])
    save(lhrtfnc,rhrtfnc)

    lhrtf48,rhrtf48,l48 = resample_hrtf(lhrtfnc,rhrtfnc,44100,48000)
    save(lhrtf48,rhrtf48, ext='48')

    lhrtf88,rhrtf88,l88 = resample_hrtf(lhrtfnc,rhrtfnc,44100,88200)
    save(lhrtf88,rhrtf88, ext='88')

    lhrtf96,rhrtf96,l96 = resample_hrtf(lhrtfnc,rhrtfnc,44100,96000)
    save(lhrtf96,rhrtf96, ext='96')


    # lhrtf48,rhrtf48,l48 = resample_hrtf(lhrtfnc,rhrtfnc,44100,48000)
    # save(lhrtf48,rhrtf48, ext='48')


    # lhrtf48,rhrtf48,l48 = resample_hrtf(lhrtfn,rhrtfn,44100,48000)
    # save(lhrtf48,rhrtf48, ext='48')

    # t44 = _np.arange(l44)/44100
    # print(t44)
    # t48 = _np.arange(l48)/48000
    # plt.plot(t44,lhrtf[0][0][:],t44,lhrtfn[0][0][:],t44,lhrtfnc[0][0][:])
    # # plt.plot(t48,lhrtf48[0][0][:],t48,lhrtf48c[0][0][:])
    # plt.show()
