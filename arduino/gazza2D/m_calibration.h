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

#include "irons2D.h"
#include <vector>
#include "defines.h"

#define SAMPLES 1000  // 500
#define NUMBEROFAXIS 3

/********** ELLIPSE STUFF FOR PLANAR CALIBRATION ********/
vector<vector<double>> mag_samples(SAMPLES, vector<double>(3));
vector<vector<double>> acc_samples(SAMPLES, vector<double>(3));
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


/*
 * returns the dot product of two vectors
 */
double dotP(vector<double> v1, vector<double> v2) {
    return v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2];
}
/*
 * returns the module of a vector
 */
double vectorModule(vector<double> v) {
    return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

/*
 * returns the angle between two vectors
 */
double vectorsAngle(vector<double> v1, vector<double> v2) {
    double dot = dotP(v1, v2);
    double mods = vectorModule(v1) * vectorModule(v2);
    return acos(dot/mods);
}

vector<double> normalizeVector(vector<double> v) {
    vector<double> result(3);
    float sq = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    result[0] = v[0]/sq;
    result[1] = v[1]/sq;
    result[2] = v[2]/sq;
    return result;
}

/*******************************************************************************************
 * Returns the tilt angle theta of the ROTAX (ALT AXIS) from the vertical 
 *         How much is the scope mount is off level
 * 
 * When ROTAX is vertical the acc points fall in the same small volume (ideally same point,
 * if there was no noise). So if the points fall within a small volume we assume the
 * ROTAX is vertical [0, 0, 1].
 * 
 * Otherwise the readings are always disposed on a circle and the sin of the radius is the
 * inclination angle of the ROTAX.
 * 
 * We then rotate, by this angle, all readings to the world frame
 */
int check_off_level(vector<vector<double>>& avf, uint8_t debug) {
    vector<double> VERTICAL = {0.0, 0.0, 0.1};
    int N = avf.size();
    int itheta;
        
    // 1. checks if there is a measurable inclination
    vector<double> avm(3);
    for (int i = 0; i < N; i++) {
        avm[0] += avf[i][0];
        avm[1] += avf[i][1];
        avm[2] += avf[i][2];
    }
    avm[0] /= N;
    avm[1] /= N;
    avm[2] /= N;

    // from mean values of the three axes we calculate avmd, average mean distance of points from the origin
    // and find if they are within the RMS tolerance
    double avmd = 0;
    for (int i = 0; i < N; i++) {
        double sum = pow(avf[i][0] - avm[0], 2) + pow(avf[i][1] - avm[1], 2) + pow(avf[i][2] - avm[2], 2);
        avmd += sqrt(sum);
    }
    avmd /= N;            // planefit quality ~ avg distance w.r.t. center
    if((debug | ~DEBUG_MAG_CAL)==255) {
        Serial.print("calc_aderotM - avmd ");Serial.println(avmd, 4);
        Serial.print("calc_aderotM - 6*aRMS ");Serial.println(6*aRMS, 4);  // aRMS defined in sensors.h because it is device dependent
        Serial.print("calc_aderotM - aRMS ");Serial.println(aRMS, 4);
    }
    if(avmd<=(6*aRMS)) {      // cluster of points indicates rotax is vertical
        if((debug | ~DEBUG_MAG_CAL)==255) {
            Serial.println("calc_aderotM - points clustered within RMS");
        }
        return 0;  // ROTAX vertical --> tilt angle = 0°
    } else {                          // else the ROTAX is not vertical and needs to be calculated
        double theta=0;               // ROTAX/Vertical angle
        for(int i=0; i<N; i++) {
            avf[i] = normalizeVector(avf[i]);
            theta += vectorsAngle(avf[i], VERTICAL);
        }
        theta /= N;
        
        if((debug | ~DEBUG_MAG_CAL)==255) {
            Serial.print("ROTAX inclination ");Serial.print(theta*RadToDeg);Serial.println("°");
        }

        itheta = (int)(theta*RadToDeg+0.5);
        Serial.print("Theta ");Serial.println(itheta);
        return itheta;
    }
}

/**
 * Calculates the HI/SI parameters from the samples array
 * results:
 *   alpha and beta - ellipse center offsets / Hard Iron offsets
 *   sigma - deformation factor to transform to a circle
 *   R - rotation matrix to derotate the ellipse
 *   RI - rotation matrix to counter derotate the circle (rescaled ellipse)
 */
 void calculateIrons(vector<vector<double>>& v){
    irons2D(v, alfa, beta, phi, a_axis, b_axis);

    sigma = a_axis/b_axis;
  
    double sint = sin(phi);
    double cost = cos(phi);
    double msint = sin(-phi);
    double mcost = cos(-phi);
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
int add_sample(float *mag, float *acc, uint8_t debug) {
  if(last_entry < SAMPLES) {
      mag_samples[last_entry][0] = mag[0];
      mag_samples[last_entry][1] = mag[1];
      mag_samples[last_entry][2] = mag[2];
      acc_samples[last_entry][0] = acc[0];
      acc_samples[last_entry][1] = acc[1];
      acc_samples[last_entry][2] = acc[2];
      last_entry++;
      return SAMPLES - last_entry +1;
  } else {
      calculateIrons(mag_samples);
      if((debug | ~DEBUG_MAG_COMP)==255) {
        printIrons();
      }
      int theta = check_off_level(acc_samples, debug);
      Serial.print("MOUNT_LEVEL, ");Serial.println(theta);
      Serial1.print("MOUNT_LEVEL, ");Serial1.println(theta);
      return 0;
  }
}

/**
 * calibrates the magnetometer reading and returns it
 */
void m_calibrate(float *mag, float *acc, uint8_t debug) {
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
        Serial.print(s[0],4);Serial.print(",");Serial.print(s[1],4);Serial.print(",");Serial.print(s[2],4);
    }
    Serial.println("END");
}
