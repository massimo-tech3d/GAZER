/******
 * This module is required in case of metallic OTA too close to the magnetometer.
 * The calibration procedure brings the OTA to vertical ~90° altitude (zenit) and collects magnetometer 
 * samples for subsequent hard/soft iron cancellation
 * 
 * The problem addressed is that the netallic OTA, when in positions different than the zenit (where it is during
 * calibration), perturbates the magnetometer readings compared to the compensated values. This causes a shift in 
 * azimuth readings.
 * 
 * This module collects samples to compensate for this effect:
 * The OTA is slowly swung from zenit to horizontal - 0° altitude - without changing the azimut which 
 * stays fixed at 180°, south, or whatever orientation.
 * 
 * A hasmap is created for every helf degree altitude storing the measured deviation from 180°, which 
 * should be the correct reading.
 * 
 * When calculating the aim position of the scope, the correct azimuth is simply calculated by subtracting 
 * the deviation for that OTA altitude from the measured azimuth value.
 * 
 * NOTE: using magnetometer compensation is entirely OPTIONAL. Use it only as a last resource if your OTA does
 *       interfere with the magnetic field.
 *       Even if it does, it would be much better to find a different positioning for the magnetometer sensor.
 * 
 * Created by Massimo Tasso, October, 13, 2023
 * Released under GPLv3 License - see LICENSE file for details.
 ******/
 
#include <unordered_map>
#include "defines.h"

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

int add_compensation_sample(float target, float altitude, float azimuth, uint8_t debug){
  float delta = target - azimuth;
  
  if(last_compensation < COMP_SAMPLES) {
    int hash = make_hash(altitude);

    hashmap.insert({hash, delta});
    last_compensation++;
    if((uint8_t)(debug | ~DEBUG_MAG_COMP)==255) {
      Serial.print("Adding to compensation map. Altitude: ");Serial.print(altitude);Serial.print(" Azimuth: ");Serial.print(azimuth);Serial.print(" Target: ");Serial.println(target);
      Serial.print("Adding to compensation map. delta Az: ");Serial.print(delta);Serial.print(" Hash: ");Serial.println(hash);
    }
    return COMP_SAMPLES - last_compensation +1;
  } else {
    return 0;
  }
}

/* 
 * searches for the hash. if not found tries the hash right above. If the search
 * reaches the top possible hash value, repeats the search going down until found,
 * When found, the corresponding delta is inserted in the hasmap, associated to the hash
 * 
 * Recursive, however only a handful of steps are expected before finding an hash
 * otherwise the problems are greater that a stack overflow. The entire calibration
 * procedure should be restared, S.O. or not S.O.
 */ 
float find_hash(int hash, bool reverse=false) {
  if(hashmap.find(hash) != hashmap.end()) {
    return hashmap[hash];
  } else if((!reverse) && (hash <= 900)) { // if not at top
    Serial.print("Hash ");Serial.print(hash);Serial.println(" not found. Going up");
    hash = hash+5;  // crawls up
    float delta = find_hash(hash, reverse);
    hashmap.insert({hash-5, delta});
    return delta;
  } else {
    Serial.print("Hash ");Serial.print(hash);Serial.println(" not found. Going down");
    hash = hash-5;  // crawls down
    if(hash < 0) {
      return 0;
    }
    float delta = find_hash(hash, true);
    hashmap.insert({hash+5, delta});
    return delta;    
  }
}

float m_compensate(float azimuth, float altitude, bool ready){
  if(ready) {
      float delta;
      int hash = make_hash(altitude);
      delta = find_hash(hash);
      return azimuth + delta;
  } else {
    return azimuth;
  }
}

// https://stackoverflow.com/questions/50870951/iterating-over-unordered-map-c selected answer
void print_compensation_values(){
  for(auto& it: hashmap) {
    Serial.print("ALT: ");Serial.print(it.first);Serial.print(" DELTA: ");Serial.println(it.second);
  }
}
