The sketch gazza requires a magnetometer and two accelerometers.
We call **Accelerometer** the sensor employed to measure the attitude of the OTA and we call **Sensor Block** the combination of magnetometer and the second accelerometer used to compensate potential unlevel of the magnetometer.

The *Sensor Block* needs to be positioned on the mount so that it rotates together with the azimuth axis.

The *Accelerometer* is used to measure the attitude of the OTA and, of course, needs to be bound to the OTA and tilt together with it.

The system performs the ***mandatory*** magnetometer calibration by rotating the scope tube to the zenith and then spinning the azimuth 360° round trip. In this meantime, the microcontroller collects magnetometer readings and calculates the accurate 2D calibration by:
1) fitting the ellipse from the readings
2) calculating Hard Iron error from the center of the ellipse
3) calculating Soft Iron matrix from the major axis of the ellipse and it's rotation from the x-axis

these HI / SI errors will then be cancelled from every subsequent mag reading. The calibrated mag readings will also be untilted, before calculating the azimuth.

Since a full 3D calibration is not possible on a telescope, we miss the vertical components of the Soft Iron (the Hard Iron can be fully calculated).
This means the untilt functionality is reliable only for relatively small angles, say 10° ==> when setting up the telescope, you only need to grossly level the mount/tripod. **No need to perfectly level to a fraction of degree !**

Depending on your mount, if the sensor block cannot be positioned far enough from a metallic OTA, this may create magnetic interferences with the magnetometer when changing attitude, compared to the position during calibration.

An additional ***optional*** compensation step may then be required to compensate this intereference. It is anyway strongly reccommended to choose a different placement of the Sensor Block, if at all possible.

During the comnpensation step, the scope is rotated to Azimuth 180° (South) and Altitude 90° (Vertical, should already be vertical after the previous calibration phase).

The scope is then slowly rotated (1° per second) to horizontal position. During this swing, the system reads the Azimuth every 0.5° and stores the deviation from 180°. This deviation will be added to the actual Azimuth readings (depending on Altitude) during normal telescope operations.

**This second step is not necessary in aluminium or wooden made OTAs, such as Dobsonians, or if the Sensor Block is 20/25 cm away from the OTA**

The calibration procedure is coordinated by the Elixir software on the Raspberry, and is initiated by the user.

For testing purposes calibration can also be initiated from arduino IDE console by issuing the command "m_calibration XYZ" where XYZ is the duration required in seconds, after connecting the optional USB cable to the workstation.

To test the second calibration step, the command is "m_cal_to_horizontal XYZ".

## Hardware parts

### Microcontroller
The magnetometer calibration and untilting require extensive float operations. Therefore the AVR Microcontrollers are not suitable. The reccommended choice is a Teensy 4.0 board.

We have briefly tested the skecth also on an ESP32 WROOM board and it can be used. We preferred the Teensy over the ESP32 because of its smaller footprint.

The sketch, and the PCB, only support the Teensy 4.0. Support for the ESP, as well as other same level micros, would require changing the pinout mappings in the sketch and changing the PCB design.

### Supported MEMS are:
* Magnetometers: ST LIS3MDL (reccommended), PNI RM3100 (reccommended).
* Untilt accelerometers: NXP MMA8451 (reccommended), InvenSense MPU6050
* Accelerometer: ST LSM6DSV

Datasheets:
* ST LIS3MDL https://www.st.com/resource/en/datasheet/lis3mdl.pdf
* PNI RM3100 https://www.tri-m.com/products/pni/RM3100-User-Manual.pdf
* NXP MMA8451 https://www.nxp.com/docs/en/data-sheet/MMA8451Q.pdf
* IS MPU6050 https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf
* ST LSM6DSV https://www.st.com/resource/en/datasheet/lsm6dsv.pdf

other sensors can be employed by writing the relevant (simple) header file

The usual breakout boards vendors will have BBs for all these sensors, always choose those with i2c interface. Exception is the RM3100 whose BB is sold directly on the PNI site. Keep away from the chinese ones sold on Amazon, they are low quality and do not support i2c interface. During the development, the PNI BB was not available and we had to use a SPI to i2c converter, which complicates a lot the Sensor BLock hardware.

For this reason the RM3100 is not the reccommended choice, otherwise it might be because the sensor is by far the less noisy of the lot. If you can access the PNI BB, that could be a very good choice (though we haven't tested it).

### Joystick
any analog joystick for arduino with 5 contacts will fit. Just needs X and Y movements and pushbutton function. Wiring is explained in the hardware section of this repository.
