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
 */

#include "irons.h"
#include <vector>

#define SAMPLES 1000  // 500
#define NUMBEROFAXIS 3

#define TO_DEG 180.0/PI

/********** ELLIPSE STUFF FOR PLANAR CALIBRATION ********/
vector<vector<double>> samples;
int last_entry = 0;

double alfa = 1;      // ellipse -  x axis
double beta = 1;      // ellipse -  y axis
double sigma = 1;     // ellipse -  scale factor
double my_alfa = 1;   // ellipse -  x axis
double my_beta = 1;   // ellipse -  y axis

float R[4] = {1, 0, 0, 1};
float RI[4] = {1, 0, 0, 1};

// TESTING -- CAN BE DELETED and uncomment declarations in calculateIrons
double phi, a_axis, b_axis;

void init_cal() {
  samples.resize(SAMPLES , vector<double>(2, 0));
}

long init_cal(int duration) {
  samples.resize(SAMPLES , vector<double>(2, 0));
  return (long)(duration / SAMPLES);
}

void printIrons();

/**
 * Calculates the HI/SI parameters from the samples array
 * results:
 *   alpha and beta - ellipse center offsets / Hard Iron offsets
 *   sigma - deformation factor to transform to a circle
 *   R - rotation matrix to derotate the ellipse
 *   RI - rotation matrix to counter derotate the ellipse
 */
 void calculateIrons(){
//  double phi, a_axis, b_axis;

// the angle phi is returned in radians
  irons(samples, alfa, beta, phi, a_axis, b_axis);
  sigma = a_axis/b_axis;

  float sint = sin(phi);
  float cost = cos(phi);
  float msint = sin(-phi);
  float mcost = cos(-phi);
  R[0] = cost;
  R[1] = sint;
  R[2] = -sint;
  R[3] = cost;
  RI[0] = mcost;
  RI[1] = msint;
  RI[2] = -msint;
  RI[3] = mcost;
}


/**
 * During the calibration phase adds a new sample to the samples vector necessary to calculate the ellipse fitting to a circle
 * After enough points have been added, initiates the actual ellipse fitting and HI/SI parameters (calculateIrons)
 */
int add_sample(float *mag, float *acc) {
//  Serial.print("addsample -- \tx: ");Serial.print(mag[0]);Serial.print("\ty: ");Serial.print(mag[1]);Serial.print("\tz: ");Serial.println(mag[2]);
  if(last_entry < SAMPLES) {
//    Serial.println("addsample -- flattening");
//    flatten(mag, acc);
//    Serial.println("addsample -- flattened");
    samples[last_entry][0] = mag[0];
    samples[last_entry][1] = mag[1];
    last_entry++;
    return SAMPLES - last_entry +1;
  } else {
    calculateIrons();
    printIrons();
    return 0;
  }
}

/**
 * flattens the magnetometer reading and returns the calibrated reading
 */
//void m_calibrate(float *mag, float *acc) {
void m_calibrate(float *mag) {
  // APPLY HARD IRON OFFSET
  float Cx = mag[0] - (float) alfa;
  float Cy = mag[1] - (float) beta;

  // APPLY SOFT IRON ROTATION
  float rotx = Cx*R[0] + Cy*R[1];  // v(Bx By) * R
  float roty = Cx*R[2] + Cy*R[3];  // rotates

  // APPLY SOFT IRON RESCALING
  float scalx = rotx / (float) sigma;   // scales
  float scaly = roty;

  // APPLY SOFT IRON REROTATION
  float derotx = scalx*RI[0] + scaly*RI[1];  // v(xy) * RI
  float deroty = scalx*RI[2] + scaly*RI[3];  // derotates
  mag[0] = derotx;
  mag[1] = deroty;
}

float getAlfa(){
  return alfa;
}

float getBeta(){
  return beta;
}

void printIrons() {
  Serial.println("IRON HI/SI");
  Serial.print("alfa ");Serial.print(alfa);Serial.print(" beta ");Serial.println(beta);
  Serial.print("[");Serial.print(R[0]);Serial.print(" ");Serial.println(R[1]);
  Serial.print(" ");Serial.print(R[2]);Serial.print(" ");Serial.print(R[3]),Serial.println("]");
  Serial.print("Sigma ");Serial.println(sigma);
}

// TESTING FUNCTIONS ONLY -- CAN BE DELETED

void SendSamples() {
  for(int i=0; i<SAMPLES; i++){
    Serial.print("SAMPLE,");Serial.print(samples[i][0]);Serial.print(",");Serial.println(samples[i][1]);
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
