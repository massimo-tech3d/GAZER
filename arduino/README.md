The repository includes two sketches, Gazza2D and Gazza3D.

Their role is to read the sensors and calculate Altitude and Azimuth of the Telescope OTA for the main Gazer app, which run on the connected Raspberry Pi 3.

The sensors required are an accelerometer (rotating with the OTA) to calculate the Altidude, and a Magnetometer to calculate the Azimuth.

The difference between the two sketches is how the magnetometer is handled.

#2D
The 2D Sketch requires the magnetometer to be positioned flat on the mount and rotate with it, disregarding the OTA inclinations. It uses a second accelerometer on the same board of the magnetometer to compensate for potential off levels - tripod and mount or a mismounted magnetometer board, **Small degrees off level (say up to 5° or 10° at best) can be compensated by the second accelerometer**

The second accelerometer is strongly reccommended but the system will work also without it. In this case you should ensure that the telescope mount and the magnetometer itself are perfectly level, or the Azimuth readings will be way off.

The sensor calibration carousels 360° back and forth the azimuth axis, with the OTA conveniently positioned verticallly, and then performs 2D ellipse fitting of the sensors readings to calculate hard iron and soft iron correction parameters.

The carouselling requires approximately 4 minutes.

#3D
The 3D Sketch requires magnetometer and accelerometer on the same board on the OTA rotating with the OTA both in Azimuth and Altitude. The second accelerometer in not required.

The sensor calibration carousels 360° back and forth two times the azimuth axis, with the OTA positioned at 0° (horizontal), 45°, 90° and 135° (backtilted), and then performs 3D ellipsoid fitting of the sensors readings to calculate hard iron and soft iron correction parameters.

The carouselling requires approximately 10 minutes.

##Which should be chosen
It depends on you mount and setup. For dobsons you should chose the 2D version, since it does not backtilt the OTA and because there will not be problems in finding a good place to install the magnetomer board, away at least 20cm from motors, The aluminium/wooden OTA will not cause interferences.

It also takes less time to calibrate.

Telescopes mounted on other types of platform can also use the 3D skecth, if you can position the magnetometer away from motors and away from the OTA (if this is made of ferromagnetic metal).

If this is not the case, it is necessary to use the 3D version and place the sensors directy on the OTA. It is not a problem if this is ferromagnetic, because the magnetometer will not be moving with respect to it, therefore the magnetic pertubations can be compensated by the calibration procedure.

In case of 2D version and magnetometer too close to metallic OTA, this will disturb the magnetic field when changing it's altitude. There is a compensation function to limit this phenomenon but it is reccomended to avoid it at all, rather than compensating it.

In sinthesys the main advantages of the two setups are:
* 2D version
	- faster calibration procedure
	- OTA vertical during carouselling - no issues with space requirements
	- does not backtilt the OTA during calibration
* 3D version
	- less number of sensors
	- just one sensor board and one sensor cable


In both sketches the magnetometer calibration procedure is intiated by the user and coordinated by the Elixir software on the Raspberry.

The Accelerometer calibration is much simpler, the OTA is placed horizontal and a reading is taken via a button on the phone GUI. The same is then repeatd with the OTA in vertical position.

# Hardware parts

## Microcontroller
The magnetometer calibration and untilting require extensive float operations. Therefore the AVR Microcontrollers are not suitable. The reccommended choice is a Teensy 4.0 board.

We have briefly tested the skecth also on an ESP32 WROOM board and it compiles and runs, however we have not done any extensive testing. We preferred the Teensy over the ESP32 because of its smaller footprint.

The sketch, and the PCB, only support the Teensy 4.0. Support for the ESP, as well as other same level micros, would require changing the pinout mappings in the sketch and changing the PCB design.

## Supported MEMS are:
3D version:
* Magnetometers: ST LIS3MDL (reccommended), PNI RM3100
* Accelerometer: ST LSM6DSV (reccommended), InvenSense MPU6050

2D version:
* Magnetometers: ST LIS3MDL (reccommended), PNI RM3100
* Accelerometer: ST LSM6DSV (reccommended), NXP MMA8451
* Untilt accelerometers: NXP MMA8451 (reccommended), InvenSense MPU6050

Note: the two accelerometers must be different. Not possible to use MMA8451 in both positions.


Datasheets:
* ST LIS3MDL https://www.st.com/resource/en/datasheet/lis3mdl.pdf
* ST LSM6DSV https://www.st.com/resource/en/datasheet/lsm6dsv.pdf
* PNI RM3100 https://www.tri-m.com/products/pni/RM3100-User-Manual.pdf
* NXP MMA8451 https://www.nxp.com/docs/en/data-sheet/MMA8451Q.pdf
* IS MPU6050 https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf

other sensors, supporting i2c interface, can be employed by writing the relevant (simple) header file

The usual breakout boards vendors will have BBs for all these sensors. Exception is the RM3100 whose BB is sold directly on the PNI site. Keep away from the chinese ones sold on Amazon and labelled "Military grade", they are poor quality and do not support i2c interface. During the development, the PNI BB was not available and we had to use an SPI to i2c converter, which complicated a lot the Sensor Block hardware.

For this reason the RM3100 is not the reccommended choice otherwise it might be, because the sensor is by far the less noisy of the lot. If you can access the PNI BB, that could be a very good choice (though we haven't tested it).

## Joystick
any analog joystick for arduino with 5 contacts will fit. Just needs X and Y movements and pushbutton function. Wiring is explained in the hardware section of this repository.
