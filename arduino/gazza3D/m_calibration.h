/* 
 *  Calibration of flat magnetometer
 *  1- collection of magnetometer samples by turning the telescope 360° forth and back
 *  2- intepolates the ellipse best fitting the samples (by calling the irons() function)
 *  3- fills the module global variables
 *   - alfa, beta --> offsets of ellipse center --> Hard Iron
 *   - sigma --> ratio between axes -- how far from a circle is actually the ellipse
 *   - R[4] --> rotation matrix defining the angle between major axis and x axis
 *   - RI[4] --> inverse of R[]
 *  
 *  after having calculated the parameters above, every sample can be calibrated by the following sequence:
 *  A- Correcting the HARD IRON offset
 *  B- Applying the SOFT IRON rotation - this is necessary so the following rescaling does not defom the shape
 *  C- Applying the SOFT IRON rescaling
 *  D- Applying the SOFT IRON derotation
 * 
 * Created by Massimo Tasso, January, 1, 2023
 * Released under GPLv3 License - see LICENSE file for details.
 */
#include "irons3d.h"
#include <vector>
#include "defines.h"
//#include "untilter.h"
//#include "untilt_functions.h"

#define SAMPLES 1000  // 500
#define NUMBEROFAXIS 3

/********** PARAMETERS FOR 3D CALIBRATION ***************/
vector<vector<double>> cal_3d_mat = {{1,0,0}, {0,1,0}, {0,0,1}};;
vector<double> cal_3d_center = {0,0,0};

/********** ELLIPSE STUFF FOR PLANAR CALIBRATION ********/
vector<vector<double>> mag_samples(SAMPLES, vector<double>(3));
vector<vector<double>> acc_samples(SAMPLES, vector<double>(3));
vector<vector<double>> orig_mag_samples(SAMPLES, vector<double>(3));
int last_entry = 0;

double alfa = 0;              // ellipse -  x axis
double beta = 0;              // ellipse -  y axis
double sigma = 1;             // ellipse -  scale factor
float R[4] = {1, 0, 0, 1};    // point rotation matrix
float RI[4] = {1, 0, 0, 1};   // point derotation matrix
double phi, a_axis, b_axis;


void printIrons();
void SendEllipse();

long init_cal(int duration) {
    last_entry = 0;
    return (long)(duration / SAMPLES);
}

/**
 * During the calibration phase adds a new sample to the samples vector necessary to calculate the ellipse fitting to a circle
 * After enough points have been added, initiates the actual ellipse fitting and HI/SI parameters (calculateIrons)
 */
int add_sample(float *mag, float *acc, uint8_t debug) {
  if(last_entry < SAMPLES) {
      orig_mag_samples[last_entry][0] = mag[0];
      orig_mag_samples[last_entry][1] = mag[1];
      orig_mag_samples[last_entry][2] = mag[2];
      mag_samples[last_entry][0] = mag[0];
      mag_samples[last_entry][1] = mag[1];
      mag_samples[last_entry][2] = mag[2];
      acc_samples[last_entry][0] = acc[0];
      acc_samples[last_entry][1] = acc[1];
      acc_samples[last_entry][2] = acc[2];
      last_entry++;
      return SAMPLES - last_entry +1;
  } else {
      ellipsoid_fit(mag_samples, cal_3d_mat, cal_3d_center);
      return 0;
  }
}

void m_calibrate(float *mag, float *acc, uint8_t debug) {
    vector<double> vmag = {mag[0], mag[1], mag[2]};
    vector<double> calibrated = calibrate(vmag, cal_3d_mat, cal_3d_center);
    mag[0] = (float)calibrated[0];
    mag[1] = (float)calibrated[1];
    mag[2] = (float)calibrated[2];
}

// FUNCTIONS FOR OFFLINE ANALYSIS

float getAlfa(){
  return alfa;
}

float getBeta(){
  return beta;
}

void printIrons() {
    Serial.print("Offset X: ");Serial.print(getAlfa());Serial.print(" Offset Y: ");Serial.println(getBeta());
    Serial.println("IRON HI/SI");
    Serial.print("alfa ");Serial.print(alfa);Serial.print(" beta ");Serial.println(beta);
    Serial.print("[");Serial.print(R[0]);Serial.print(" ");Serial.println(R[1]);
    Serial.print(" ");Serial.print(R[2]);Serial.print(" ");Serial.print(R[3]),Serial.println("]");
    Serial.print("Sigma ");Serial.println(sigma);
}


void SendSamples() {
    for(int i=0; i<SAMPLES; i++){
        Serial.print("MAG_SAMPLE,");Serial.print(mag_samples[i][0]);Serial.print(",");Serial.println(mag_samples[i][1]);
        Serial.print("ACC_SAMPLE,");Serial.print(acc_samples[i][0]);Serial.print(",");Serial.println(acc_samples[i][1]);
    }
    Serial.println("END,");
}

void SendEllipse(){
    Serial.print("ALFA,");Serial.println(alfa);
    Serial.print("BETA,");Serial.println(beta);
    Serial.print("A_AXIS,");Serial.println(a_axis);
    Serial.print("B_AXIS,");Serial.println(b_axis);
    Serial.print("PHI,");Serial.println(phi);
    Serial.println("END,");
}

void print_carousel_data() {
    for(int i=0; i<SAMPLES; i++){
        // samples = uncalibrated carousel, samp = calibrated
        Serial.print("CAROUSEL,");
        Serial.print(mag_samples[i][0],4);Serial.print(",");Serial.print(mag_samples[i][1],4);Serial.print(",");Serial.print(mag_samples[i][2],4);Serial.print(",");
        Serial.print(acc_samples[i][0],4);Serial.print(",");Serial.print(acc_samples[i][1],4);Serial.print(",");Serial.print(acc_samples[i][2],4);Serial.print(",");
        float s[3];
        s[0] = (float)mag_samples[i][0];
        s[1] = (float)mag_samples[i][1];
        s[2] = (float)mag_samples[i][2];
        float a[3];
        a[0] = (float)acc_samples[i][0];
        a[1] = (float)acc_samples[i][1];
        a[2] = (float)acc_samples[i][2];
        m_calibrate(s, a, false);
        Serial.print(s[0],4);Serial.print(",");Serial.print(s[1],4);Serial.print(",");Serial.print(s[2],4);Serial.print(",");
        Serial.print(orig_mag_samples[i][0],4);Serial.print(",");Serial.print(orig_mag_samples[i][1],4);Serial.print(",");Serial.print(orig_mag_samples[i][2],4);
        Serial.println();
    }
    Serial.println("END");
}
