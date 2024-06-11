#include "sensor.h"


class MagRM3100: Sensor {

    public:
        bool init_magnetometer() {
            uint8_t revid;
            
            // Serial.println("Starting RM3100");
            initSPI();
            
            revid = readReg(RM3100_REVID_REG);
            if(revid!=0x22) {
                return false;
            }
            changeCycleCount(initialCC); //set the cycle count;
            cycleCount = readReg(RM3100_CCX1_REG);
            cycleCount = (cycleCount << 8) | readReg(RM3100_CCX0_REG);
            
            gain = (0.3671 * (float)cycleCount) + 1.5; //linear equation to calculate the gain from cycle count
            // Serial.print("Gain = "); //display gain; default gain should be around 75 for the default cycle count of 200
            // Serial.println(gain);
            // Serial.println();

            // Enable transmission to take continuous measurement with Alarm functions off
            writeReg(RM3100_CMM_REG, 0x79);

            initSensor(K_ERR_RM, K_Q_RM);
            return true;
        }

        /*
        * Look at the sensor block and call north the axis pointing forward
        * east and down consequently.
        * This method then swaps the axes in the correct way.
        */
        void axes(float *mag_raw) {
            float north = -mag_raw[0];  // the axis pointing forward is -x, [0]
            float east  = -mag_raw[1];  // the axis pointing right is -y, [1]
            float down  = -mag_raw[2];  // the axis pointing down is -z [2]
            swap_axes(north, east, down, mag_raw);
        }

        void readings(float *mag_raw) {
            readMag(mag_raw);
            smooth_readings(mag_raw);
            mag_axes(mag_raw);
        }

        int dataRate(void) {
            uint8_t tmrc = readReg(RM3100_TMRC_REG);
            // Serial.print("TMRC = 0x"); //REVID ID should be 0x22
            // Serial.println(tmrc, HEX);
            switch(tmrc) {
                case 0x94:
                    return 150;
                case 0x95:
                    return 75;
                case 0x96:
                    return 37;
                case 0x97:
                    return 18;
                default:
                    return 9;
            }
        }

    private:
        // SC18IS601B bridge values
        // #define BRIDGE_SPICLK_1843_kHz 0B00  // 1.8 MBit/s -- too fast for RM3100
        #define BRIDGE_SPICLK_461_kHz  0B01  // 461 kbit/s
        // #define BRIDGE_SPICLK_115_kHz  0B10  // 115 kbit/s
        // #define BRIDGE_SPICLK_58_kHz   0B11  // 58 kbit/s
        #define BRIDGE_SPIMODE_0       0B00  // CPOL: 0  CPHA: 0
        // #define BRIDGE_SPIMODE_1       0B01  // CPOL: 0  CPHA: 1
        // #define BRIDGE_SPIMODE_2       0B10  // CPOL: 1  CPHA: 0
        // #define BRIDGE_SPIMODE_3       0B11  // CPOL: 1  CPHA: 1
        // #define SLAVENUM 0                   // slave number of RM3100
        #define SPI_SPEED BRIDGE_SPICLK_461_kHz
        // #define SPI_MODE BRIDGE_SPIMODE_0
        #define BRIDGE_CONFIG_SPI_CMD  0xF0  // CONFIGURE SPI FUNCTION ON BRIDGE
        #define I2CAddress 0x28              // Hexadecimal slave I2C address for bridge

        // RM3100 internal register values without the R/W bit
        #define RM3100_REVID_REG  0x36   // Hexadecimal address for the Revid internal register
        // #define RM3100_POLL_REG   0x00   // Hexadecimal address for the Poll internal register
        #define RM3100_CMM_REG    0x01   // Hexadecimal address for the Continuous Measurement Mode internal register
        #define RM3100_STATUS_REG 0x34   // Hexadecimal address for the Status internal register
        #define RM3100_TMRC_REG   0x0B   // Hexadecimal address for the Status internal register
        // #define RM3100_CCX1_REG   0x04   // Hexadecimal address for the Cycle Count X1 internal register
        #define RM3100_CCX0_REG   0x05   // Hexadecimal address for the Cycle Count X0 internal register
        // #define RM3100_CCY1_REG   0x06   // Hexadecimal address for the Cycle Count X1 internal register
        // #define RM3100_CCY0_REG   0x07   // Hexadecimal address for the Cycle Count X0 internal register
        // #define RM3100_CCZ1_REG   0x08   // Hexadecimal address for the Cycle Count X1 internal register
        // #define RM3100_CCZ0_REG   0x09   // Hexadecimal address for the Cycle Count X0 internal register
        //options
        #define initialCC 200  // Cycle count default = 200 (lower cycle count = higher data rates but lower resolution)
        /*
         * CC    Sensitivity     Noise    Max sample rate
         *  50      50 nT        30 nT        1600/3
         * 100      26 nT        20 nT         850/3
         * 200      13 nT        15 nT         440/3
         * 400     (8 nT)?      (10 nT)?       200/3
         */

        float K_ERR_RM = 0.08;   // kalman filter error estimate - value defined observing plots of raw mag axis reading and kalman smoothed
        float K_Q_RM   = 0.01;   // kalman filter process variance - value defined observing plots of raw mag axis reading and kalman smoothed
        float gain;
        uint16_t cycleCount;

        /*
        * Initialises the bridge SPI
        */
        void initSPI() {
            Wire.beginTransmission(I2CAddress);
            Wire.write(BRIDGE_CONFIG_SPI_CMD);
            Wire.write(0x00 | SPI_SPEED); // order bit 0, mode bits 00  => 0x00 | SPI_SPEED
            Wire.endTransmission();
        }

        /*
        * reg is the 7 bit value of the RM3100 register's address (without the R/W bit)
        */
        uint8_t readReg(uint8_t reg) {
            uint8_t data=0;
            Wire.beginTransmission(I2CAddress);
            Wire.write(0x01);                 // function to send to SS0 of bridge
            Wire.write(reg | 0x80);           // data payload 1 byte --> address of register to be read  or 0x80 to set the read bit
            Wire.write(0xFF);
            Wire.write(0xFF);
            Wire.endTransmission();
            delay(1);
            Wire.requestFrom(I2CAddress, 2);  // 2 reads, 1st is dummy, 2nd is value
            while (Wire.available()) {        // peripheral may send less than requested
                data = Wire.read();           // receive a byte as character
            }
            return data;
        }

        /*
        * reg is the 7 bit value of the RM3100 register's address (without the R/W bit)
        * value is the 8 bit data being written
        */
        void writeReg(uint8_t reg, uint8_t value) {
            Wire.beginTransmission(I2CAddress);
            Wire.write(0x01);       // function to send to SS0 of bridge
            Wire.write(reg & 0x7F); //AND with 0x7F to make first bit(read/write bit) low for write
            Wire.write(value);
            Wire.endTransmission();
        }

        /*
        * newCC is the new cycle count value (16 bits) to change the data acquisition
        * This is the first function being called that writes something to the RM3100
        * 
        * Since the first byte written (cold start) always fails, this function starts
        * by executing a dummy write.
        */
        void changeCycleCount(uint16_t newCC){
            uint8_t CCMSB = (newCC & 0xFF00) >> 8; //get the most significant byte
            uint8_t CCLSB = newCC & 0xFF; //get the least significant byte

            // dummy write 1 byte because the first write after a cold start ALWAYS fails
            writeReg(RM3100_CCX1_REG, CCMSB);
            delay(1);

            Wire.beginTransmission(I2CAddress);
            Wire.write(0x01);  // function to send to SS0
            Wire.write(RM3100_CCX1_REG & 0x7F); // data payload 1 byte --> AND with 0x7F to make first bit(read/write bit) low for write
            Wire.write(CCMSB);  //write new cycle count to ccx1
            Wire.write(CCLSB);  //write new cycle count to ccx0
            Wire.write(CCMSB);  //write new cycle count to ccy1
            Wire.write(CCLSB);  //write new cycle count to ccy0
            Wire.write(CCMSB);  //write new cycle count to ccz1
            Wire.write(CCLSB);  //write new cycle count to ccz0
            Wire.endTransmission();
        }

        void readMag(float *mag) {
            long x = 0;
            long y = 0;
            long z = 0;
            uint8_t x2,x1,x0,y2,y1,y0,z2,z1,z0;

            //wait until data is ready using polling method
            while((readReg(RM3100_STATUS_REG) & 0x80) != 0x80); //read internal status register

            //read measurements
            Wire.beginTransmission(I2CAddress);
            Wire.write(0x01);         // function to send to SS0 of bridge
            Wire.write(0xA4);         // data payload 1 byte --> address of register to be read  or 0x80 to set the read bit
            Wire.write(0xFF);
            Wire.write(0xFF);
            Wire.write(0xFF);
            Wire.write(0xFF);
            Wire.write(0xFF);
            Wire.write(0xFF);
            Wire.write(0xFF);
            Wire.write(0xFF);
            Wire.write(0xFF);
            Wire.endTransmission();
            delay(1);
            Wire.requestFrom(I2CAddress, 10);
            Wire.read(); // consume dummy data generated by the bridge
            x2 = Wire.read();  // receive a byte as character
            x1 = Wire.read();  // receive a byte as character
            x0 = Wire.read();  // receive a byte as character
            y2 = Wire.read();  // receive a byte as character
            y1 = Wire.read();  // receive a byte as character
            y0 = Wire.read();  // receive a byte as character
            z2 = Wire.read();  // receive a byte as character
            z1 = Wire.read();  // receive a byte as character
            z0 = Wire.read();  // receive a byte as character
            // Serial.print("READINGS Z  - z2: ");Serial.print(z2, HEX);Serial.print(" z1: ");Serial.print(z1, HEX);Serial.print(" z0: ");Serial.println(z0, HEX);

            //special bit manipulation since there is not a 24 bit signed int data type
            if (x2 & 0x80) x = 0xFF;
            if (y2 & 0x80) y = 0xFF;
            if (z2 & 0x80) z = 0xFF;

            //format results into single 32 bit signed value
            x = (x * 256 * 256 * 256) | (int32_t)(x2) * 256 * 256 | (uint16_t)(x1) * 256 | x0;
            y = (y * 256 * 256 * 256) | (int32_t)(y2) * 256 * 256 | (uint16_t)(y1) * 256 | y0;
            // z = (z * 256 * 256 * 256) | (int32_t)(z2) * 256 * 256 | (uint16_t)(z1) * 256 | z0;
            /************************
             * NOTE: for some f*cking reason the z-line, and ONLY the z-line, does NOT work on Teensy 4.0 - it DOES work as it should in ESP32
             *       on Teensy 4.0 it loses the first byte in case the value is negative. Negative z values are therefore completely screwed up.
             * 
             *       The workaround I found is processing the z axis separately as follows.
             *************************/
            z = z << 24;
            z += ((uint32_t)z2 << 16);
            z += ((uint16_t)z1 << 8);
            z += z0;
            
            // calculate magnitude of results -- hard iron screws it up
            // double uT = sqrt(pow(((float)(x)/gain),2) + pow(((float)(y)/gain),2)+ pow(((float)(z)/gain),2));
            mag[0] = (float)(x)/gain;
            mag[1] = (float)(y)/gain;
            mag[2] = (float)(z)/gain;
        }
}