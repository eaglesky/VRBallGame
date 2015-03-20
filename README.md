# VRBallGame
Course project for CS290I-Mixed and Augmented Reality at UCSB

Demo video:

<a href="https://www.youtube.com/watch?v=-UsmVecCVEg" target="_blank"><img src="http://img.youtube.com/vi/-UsmVecCVEg/0.jpg" alt="demo" width="480" height="360" border="2" /></a>


Detailed report can be found at [http://eaglesky.github.io/VRBallGame/](http://eaglesky.github.io/VRBallGame/)

## Authors

Yalun Qin([allenchin1990@gmail.com](mailto:allenchin1990@gmail.com))

Yuxiang Wang([jdit89@gmail.com](mailto:jdit89@gmail.com))

## System requirement:

Binary files can only be executed on 64-bit Windows.

If you want to build the source code, you need to have Visual Studio 2010/2012 and Unity3D 5 installed on your machine.

**Note that the reason why our code can only be built on Windows is just because of the UDP API we used. You can replace it with the corresponding API in your OS(Mac or Linux) and then you should be able to build the code on that OS, since both the face tracker and Unity are cross-platform. It would be great if you could come up with a way to make that API cross-platform too, and you're welcome to submit a pull request for this.** 

## Building the application

To build the application, you need to build two programs, one of which is the face tracker located in the OpenCV folder, the other
is the Unity Game program located in the Unity3D folder. You can just follow the instruction in OpenCV folder to build the first 
program. The second program can be built using Unity3D.

## Running the programs

To run the first program, you must provide -p option followed by the path to the paremeter file. We've included one that you can try(/OpenCV/my_parameters.txt). Refer to the readme files in the OpenCV folder for
 information of other options. To run the second program, you just need to generate the binary file using Unity3D and run it 
 directly. 
 
We've provided executables in Bin folder. (SimpleCLM.exe is the first program and vr.exe is the second one, only tested on 64-bit Windows machine)

The parameter file is customizable(and should be). The format is as follows:

```
Physical measurement
X coordinate of screen center relative to the camera
Y coordinate of screen center relative to the camera
Width of screen
Height of screen
X coordiante of principal point
Y coordinate of principal point
Fx
Fy
```

The first 5 parameters are all in centimeter. It is recommended that you do all the above calibration, however if you're lazy, please at least do the first five measurements. The more measurement you do, the better gaming experience you'll get. You can get the last four parameters by using OpenCV. Also you may ignore this and just use the default parameter file that we provided, and if you are satisfied with the result, don't change anything.

The order of running the two programs doesn't really matter, but it is recommended that you run the face tracker first and see if the frame rate is good (average >= 15). The frame rate is affected by the lighting condition around the player. Usually it can be as high as 30FPS if the player's face is brighter than the background. After you've got this work properly, you can launch the Unity program and enjoy the game!

 
## Game Guide and Tips

### Game control

If you want to exit the tracker, switch to it and press 'q' key.
In the game program, press 'r' to reset the ball if it is out. 

### Visual indicators

It is very important to pay attention to the visual indicators while you are playing the game. Please refer to our report for more details about them. Basically the indicators are :

* Yellow rectangle, which is the bounding box of your head in the **Camera image** . You must make sure that this always stays within your screen range, otherwise it indicates that your head is out of the field of view of your camera and the tracking result is not correct at all, and you will not be able to see a correct scence. 

* Blue circle, which indicates where you are blowing at. Red color of it means that your blow action is detected and also the ball is within your blowing range. Yellow color of it means that your blow action is detected but the ball is not within your blowing range. Original blue color means all the other cases. 

* Moving frame, which can be seen as something like a shadow of the fireball, indicating the depth of the ball in the tunnel.

### Blow tips

* Make sure you've changed your mouth as close to a round shape as possible, if you want to "blow". Otherwise, keep your mouth closed. Refer to Figure 6 and the paragraph below in our report for more information.

* Don't keep blowing. Impulse often works better.
