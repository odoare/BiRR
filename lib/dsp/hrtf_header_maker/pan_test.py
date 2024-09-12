#%%
import numpy as np
import matplotlib.pyplot as plt
theta = np.linspace(-np.pi,np.pi,10000)
##plt.plot(theta,yl,theta,yr,theta,yl+yr)
##plt.show()

def cardio(theta):
    return 0.5*(1+np.cos(theta))

def eight(theta):
    return np.sin(theta)

# Stereo
sr = np.abs(np.sin(0.5*theta+np.pi/4))
sl = np.abs(-np.sin(0.5*theta-np.pi/4))

# XY
xyl = cardio(theta-np.pi/4)
xyr = cardio(theta+np.pi/4)

# MS
mid = cardio(theta)
side = eight(theta)
msl = mid+side
msr = mid-side


#plt.plot(theta,sr, theta, xyl, theta, msl)
#plt.polar(theta, xyl, theta, xyr)
#plt.show()
plt.polar(theta,mid,theta,abs(side))
# %%
