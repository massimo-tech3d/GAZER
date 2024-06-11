/*
 * The accelerator calibration consists in taking the readings in the two extreme positions with telescope resting still:
 * - horizontal telescope where AX should be 0g and AZ should be 1g
 * - vertical telescope where AX should be 1g and AZ should be 0g
 * 
 * AY does not matter at all, it's not used.
 * 
 * The values are then separately averaged to calculate the calX and calZ calibration values
 * NOTA: credo che basti la lettura del solo valore a 0g. X con telescopio horizzontale e Y con telescopio verticale
 * 
 * Accelerometer readings are then calibrated by multipling the reading for the gain and subtracting the offset
 * 
 * NXP AN7399
 * --------------------------------------
 * 
 * o = offset = -X0
 * g = gain = N/X1 = 16384 / X1
 * 
 * Xc = X * g + o
 * 
 * Created by Massimo Tasso, July, 13, 2023
 * Released under GPLv3 License - see LICENSE file for details.
 */

//#define MAX_RANGE 9.81       // when m/s^2 readings -9.81..9.81
#define MAX_RANGE 1            // when m/s^2 readings -9.81..9.81

float oX = 0;  // offset X
float oZ = 0;  // offset Z
float gX = 1;  // gain X
float gZ = 1;  // gain Z

float x0 = 0, x1 = 0, z0 = 0, z1 = 0;  // Readings x/z at 0g and 1g

/*
 * Calculates the calibration from the readings on the two stop points: horizontal telescope and vertical telescope
 */
void calc_a_calibration(){
  oX = -x0;
  oZ = -z0;
  gX = MAX_RANGE / x1;
  gZ = MAX_RANGE / z1;
}

/*
 * Reads X / Z value when they should be 0g / 1g i.e. with scope in horizontal position
 */
void read_horizontal_accel(){
  float accel[3];
  int iter = 5;
  x0 = 0;
  z1 = 0;
  for(int i=0; i<5; i++){
    accel_readings(accel, false);
    x0 += accel[0];
    z1 += accel[2];
    delay(100);
  }
  x0 /= iter;
  z1 /= iter;
  calc_a_calibration();
  Serial.println("Acc cal horizontal reading acquired");
  Serial1.println("Acc cal horizontal reading acquired");
}

/*
 * Reads X / Z value when they should be 1g / 0g i.e. with scope in horizontal position
 */
void read_vertical_accel(){
  float accel[3];
  int iter = 5;
  x1 = 0;
  z0 = 0;
  for(int i=0; i<5; i++){
    accel_readings(accel, false);
    x1 += accel[0];
    z0 += accel[2];
    delay(100);
  }
  x1 /= iter;
  z0 /= iter;
  calc_a_calibration();
  Serial.println("Acc cal vertical reading acquired");
  Serial1.println("Acc cal vertical reading acquired");
}

/*
 * Returns a calibrated reading
 */
void a_calibrate(float *accel){
  accel[0] = accel[0] * gX + oX;
  accel[2] = accel[2] * gZ + oZ;
}

/*
 * Sets the calibration values from the Raspberry, if they have been calculated previously
 */
void set_a_calibration(float ox, float gx, float oz, float gz){
  oX = ox;
  oZ = oz;
  gX = gx;
  gZ = gz;
}

/*
 * Returns the current calibration values, to be saved on the Raspberry Pi
 */
void export_a_calibration(){
  Serial.print("A_CAL,OX,");Serial.print(oX);Serial.print(",GX,");Serial.print(gX);Serial.print(",OZ,");Serial.print(oZ);Serial.print(",GZ,");Serial.println(gZ);
  Serial1.print("A_CAL, ");Serial1.print(oX);Serial1.print(", ");Serial1.print(gX);Serial1.print(", ");Serial1.print(oZ);Serial1.print(", ");Serial1.println(gZ);
}

void print_a_cal(){
  Serial.println();
  Serial.print("X offset:");Serial.print(oX);Serial.print("\tX gain:");Serial.println(gX);
  Serial.print("Z offset:");Serial.print(oZ);Serial.print("\tZ gain:");Serial.println(gZ);
  Serial.println();
}
