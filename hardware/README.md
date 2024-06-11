## Hardware description
The Hardware for GAZER system is composed of following items:
* 1 x Raspberry Pi 3
* 1 x Teensy 4.0
* 1 x PCB - Gerber files available in this repository
* 2 x Stepper motor controller. Tested with DRV8825. Any other model with same pinout, and capable of 32 µsteps per step, should be ok
* 1 x Analogue Joystick with 5 pins connector, 2 axis 1 button
* 2 x 4 pins cables and connectors for the sensors
* 1 x 5 pins cable and connectors for the joystick
* 1 x MEMS Magnetometer. Suppeorted: LIS3MDL and PNI RM3100
* 1 x MEMS Accelerometer as untilter. Supported: MMA8451 and MPU6050
* 1 x MEMS Accelerometer for Attitude. Supported LSM6DSV

### PCB Assembly
![alt text](img/board-view-front.jpg)

![alt text](img/board-view-back.jpg)

The PCB shows clear indications about what needs to be plugged where. Just solder the two 100uF capacitors (beware of polarity), the sockets and pin connectors where they need to be and plug in the components.

**Note**
The 40 pin socket to the Raspberry PI needs to be positioned on the back of the PCB (solder on the front).

The two sensors sockets are interchangeable, sensors block and accelerometer can be plugged in either one.
