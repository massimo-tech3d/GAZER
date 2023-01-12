The sketch gazza-flat-magnetometer requires the magnetometer to be positioned flat on the mount and not be tilted with the telescope tube.

Accelerometer of course needs to be bound to the scope tube and tilt together with it.

The system performs the ***mandatory*** magnetometer calibration by rotating the scope tube to the zenith and then spinning 360° round trip. In this meantime, the sketch collects magnetometer readings and calculates the accurate calibration.

Normally the calibration procedure is coordinated by the Elixir software on the Raspberry, initiated by the user.

For testing purposes calibration can also be initiated from arduino IDE console by issuing the command "calibstart XYZ" where XYZ is the duration required in seconds, after connecting the optional USB cable to the workstation.

### Supported MEMS are:
* Magnetometers: ST LIS3MDL (reccommended), NXP FXOS_8700
* Accelerometer: NXP FXOS_8700

As a joystick, any analog joystick for arduino will fit. Needs X and Y movements and pushbutton function. Wiring explained in the hardware section of this repository.
