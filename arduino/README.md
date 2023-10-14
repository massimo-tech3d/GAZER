The sketch gazza-flat-magnetometer requires the magnetometer to be positioned flat on the mount and not be tilted with the telescope tube.

Accelerometer of course needs to be bound to the scope tube and tilt together with it.

The system performs the ***mandatory*** magnetometer calibration by rotating the scope tube to the zenith and then spinning 360° round trip. In this meantime, the sketch collects magnetometer readings and calculates the accurate calibration.

Subsequently, the scope is rotated to Azimuth 180° (South) and Altitude 90° (Vertical, should already be vertical after the previous calibration phase).

The scope is then slowly rotated (1° per second) to horizontal position. During this swing the system reads the Azimuth every 0.5° and stores the deviation from 180°. This deviation will be added to the actual Azimuth readings (depending on Altitude) during normal telescope operations.

This is needed because the OTA of most telescopes is made of metal. Since the magnetometer is calibrated when OTA is vertical, the changes to the OTA altitude introduce further perturbations in the magnetic field which are not accounted in the calibration and need to be compensated.

**This second step is not necessary in aluminium or wooden made OTAs, such as Dobsonians.**

Normally the calibration procedure is coordinated by the Elixir software on the Raspberry, initiated by the user.

For testing purposes calibration can also be initiated from arduino IDE console by issuing the command "m_calibration XYZ" where XYZ is the duration required in seconds, after connecting the optional USB cable to the workstation.

To test the second calibration step, the command is "m_cal_to_horizontal XYZ".

### Supported MEMS are:
* Magnetometers: PNI RM3100 (reccommended), ST LIS3MDL, NXP FXOS_8700 (discontinued)
* Accelerometer: NXP MMA8451 (reccommended), NXP FXOS_8700 (discontinued)

Note: the FOXS_8700 combines magnetometer and accelerometer on the same chip. For use with the gazza-flat-magnetometer sketch you'd need to use two of these chips because the two sensors must be mounted in diferent positions.

As a joystick, any analog joystick for arduino will fit. Needs X and Y movements and pushbutton function. Wiring explained in the hardware section of this repository.
