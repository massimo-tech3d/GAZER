
/******
 * This module is required in case of metallic OTA.
 * The calibration procedure brings the OTA to vertical ~90° altitude (zenit) and collects magnetometer samples
 * for subsequent hard/soft iron cancellation
 * 
 * The problem is that the netallic OTA, when in positions different than the zenit, perturbates the magnetometer readings compared to the compensated values
 * this causes a shift in azimuth readings.
 * 
 * This module collects samples to compensate for this effect:
 * The OTA is slowly swung from zenit to horizontal - 0° altitude - without changing the azimut which stay fixed at 180°, south
 * 
 * A hasmap is created for every helf degree altitude storing the measured deviation from 180°, which should be the correct reading
 * 
 * When calculating the aim position of the scope, the correct azimuth is simply calculated by subtracting the deviation for that OTA altitude from the measured azimuth value.
 * 
 * Created by Massimo Tasso, October, 13, 2023
 * Released under GPLv3 License - see LICENSE file for details.
 ******/
 
#include <unordered_map>

#define COMP_SAMPLES 180  // every 1/2°
unordered_map < int, float > hashmap;
int last_compensation = 0;

int init_compensation(int seconds){
  return (int)(seconds / COMP_SAMPLES);
}

/*
 * Creates a hash as an int of the integer part of the value time 10 and adding 5 or 10 if needed for roundup
 * examples
 * 75.24887 --> 750  75 * 10 + 0
 * 75.43105 --> 755  75 * 10 + 5
 * 75.61005 --> 755  75 * 10 + 5
 * 75.86702 --> 760  75 * 10 + 10
 * 
 */
int make_hash(float value){
  int i_part = (int)value;
  float dec = value - i_part;
  int add = 10;
  if(dec < 0.25)
    add = 0;
  if((dec >= 0.25) && (dec < 0.75))
    add = 5;
  return i_part * 10 + add;
}

int add_compensation_sample(float target, float altitude, float azimuth){
  float delta = target - azimuth;
  
  if(last_compensation < COMP_SAMPLES) {
    int hash = make_hash(altitude);

    hashmap.insert({hash, delta});
    last_compensation++;
    Serial.print("Adding to compensation map. Altitude: ");Serial.print(altitude);Serial.print(" Azimuth: ");Serial.println(azimuth);
    Serial.print("Adding to compensation map. delta Az: ");Serial.print(delta);Serial.print(" Hash: ");Serial.println(hash);
    return COMP_SAMPLES - last_compensation +1;
  } else {
    return 0;
  }
}

float m_compensate(float azimuth, float altitude){
  float delta = 0.0;
  int hash = make_hash(azimuth);
  if(hashmap.find(hash) != hashmap.end()) {
    delta = hashmap[hash];
  }
  // TODO handle the case where hash is NOT found --> something like find the closest up or down
  return azimuth - delta;
}
