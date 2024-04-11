# installation instructions

flash the firmware.x.y.fw file to a microsd card and plug it in the Raspberry Pi3

(firmware.fw sha256: 3307be91864f4990a0d53673387666f02c0a14a20f214d5dde0356f2f8def148

 on Ubuntu it can be checked with the command **sha256sum firmware.x.y.fw** if the file is ok
 you should get the number above)

If you prefer to compile from source code:
* Ensure elixir is up to date (1.13) and run *mix install nerves* (see https://nerves-project.org/)
* Enter both the ui and firmware directories and execute "mix deps.get"
* execute
	* *export MIX_TARGET=rpi3*
	* *export MIX_ENV=prod*
	* *mix firmware*
	* *mix burn* to flash the resulting firmware.fw file to a microsd card. Then plug it in the Raspberry Pi3
* if you have already flashed the firmware once, you can reflash by simply connecting the RPi to the lan and executing *mix upload nerves.local* from the firmware directory.

## 	post installation instructions
When booted the Raspberry will provide it's own access point, gazer, to which the smartphone can connect.

If the browser doesn't open GAZER home page automatically, it can be reached by visiting the address 192.168.3.1 with any browser after half a minute or so.

From the same smartphone launch the app "GPSd Forwarder" (available on Play Store and github), wait for GPS acquisition and activate the data forwarding to 192.168.3.1:1111
the app can be switched off as soon as the GAZER home page shows correct coordinates and time.

After everything is connected, and GPS position is reflected in the webapp, the calibrations should be performed. Tap on the Calibration tab, read the instructions and proceed in the order suggested.

Now you can search for celestial objects to observe using one of the three widgets provided:

#### Planet search
to find the Moon and the 8 planets (Pluto included) by name
#### Deep Sky Object
by specifying the main characteristics (Magnitude, type of object etc.) of the object desired, a list of compatible objects will be provided for you to choose.
Objects lower than 15° from the horizon are considered not visible and will not be shown.
#### Deep Sky Object by catalogue and number
the full NGC and IC catalogues are included. For example NGC 1976 or M 42 will take you straight to the Orion Nebula.
