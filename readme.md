## Binaural Room Reverb (BiRR) 2D and 3D

### BiRR3D
![image info](./doc/BiRR3D_screen.png)

### BiRR2D
![image info](./doc/BiRR2D_screen.png)

BiRR2D and BiRR3D are room simulators that accurately emulates the sound produced by a monophonic and omnidirectionnal source perceived by a listener in a rectangular (2D) or parallelepipedic (3D) room.

User can control $(x,y,z)$ dimensions of the room, and position of source and listener in the room. The orientation of the listener can also be adjusted. Wall properties are adjusted with only two parameters:

- Wall absorbtion : a coefficient between 0 an 1, representing a reflection coefficient. Amplitude of wave is multiplied by this coefficient at each wall rebound. Higher values produce a shorter reverb.

- Wall high frequency absobtion : this parameter controls the high frequency absobtion for each wall rebound. Higher values produce darker reberb sound.



The input is stereophonic but only the first channel is used in the current state of the plugin. The output sound is stereo reverberberated input with one of the following configurations:

- Two cardioid microphones in XY configuration

- One cardioid and one eight-pattern microphones, in MS configuration

- One omni and one eight-pattern microphone, in MS configuration

- Binaural receptor based on the HRTF provided by MIT medialab ().

