# Binaural Room Reverb (BiRR) 2D and 3D

### BiRR3D
![image info](./doc/BiRR3D_screen.png)

### BiRR2D
![image info](./doc/BiRR2D_screen.png)

## Info

BiRR2D and BiRR3D are room simulators that accurately emulate the sound produced by a monophonic and omnidirectionnal source perceived by a listener in a rectangular (2D) or parallelepipedic (3D) room.

User can control $(x,y,z)$ dimensions of the room, and position of source and listener in the room. The orientation of the listener can also be adjusted. Wall properties are adjusted with only two parameters:

- Wall absorbtion : a coefficient between 0 an 1, representing a reflection coefficient. The acoustic wave amplitude is multiplied by this coefficient at each wall rebound. Higher values produce a shorter reverb.

- Wall high frequency absobtion : this parameter controls the high frequency absorbtion for each wall rebound. Higher values produce darker reberb sound.

The input is stereophonic and for now, left and right signal are mixed to one mono signal sent to the reverb. It is the same as if both input signals were placed at the same position in the virtual space. The output sound is a stereo reverberberated sound with one of the following configurations:

- Two cardioid microphones in XY configuration

- One cardioid and one eight-pattern microphones, in MS configuration

- One omni and one eight-pattern microphone, in MS configuration

- Binaural receptor based on the HRTF provided by MIT medialab ().

## Direct path and reflections paths

when a parameter is changed, two stereo impluse responses are calculted and sent to convolution processors:

- The direct path impluse response.

- The impulse response due to multiple reflections on walls.

The effect level of direct path and reflections can be adjusted separately. These can be sought as dry and wet parameters of the reverb, although the prodeuced sound is physically acurate when both parameters are equal.

## Stereo width

