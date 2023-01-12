/******
 * Functions to acquire the
 * joystick readings
 * 
 * Created by Massimo Tasso, January, 1, 2023
 * Released under GPLv3 License - see LICENSE file for details.
 ******/

#define JOYSTICK_X A0
#define JOYSTICK_Y A1
#define JOYSTICK_BUTTON 13

#define JOYSTICK_INTERVAL_MILLIS  10
#define MILD_R 600
#define STRONG_R 950
#define MILD_L 400
#define STRONG_L 50

#define MILD_T 600
#define STRONG_T 950
#define MILD_B 400
#define STRONG_B 50


// prevoius sensor values, per non rimandare sempre lo stesso dato
// mi occorre qua in scopo globale per non reinizializzarli ad ogni iterazione
double px = 0, py = 0 , pz = 0;
String p_j_out = "";
unsigned long last_j_read = millis();


void initJoystick(){
  pinMode(JOYSTICK_BUTTON, INPUT_PULLUP);
  pinMode(JOYSTICK_X, INPUT_PULLUP);
  pinMode(JOYSTICK_Y, INPUT_PULLUP);
//  pinMode(LED_BUILTIN,OUTPUT); 
  delay(10);
}

String handleJoystick(){
  if((millis() - last_j_read) < JOYSTICK_INTERVAL_MILLIS) {
    return "";
  } else {
    last_j_read = millis();
    int x = analogRead(JOYSTICK_X);
    int y = analogRead(JOYSTICK_Y);
    int button = !digitalRead(JOYSTICK_BUTTON);
    String j_out = "";
  
    if(x > STRONG_R)
      j_out += ", RS";
    if(x > MILD_R && x <= STRONG_R)
      j_out += ", RM";
    if(x < STRONG_L)
      j_out += ", LS";
    if(x < MILD_L && x >= STRONG_L)
      j_out += ", LM";
  
    if(y > STRONG_T)
      j_out += ", TS";
    if(y > MILD_T && y <= STRONG_T)
      j_out += ", TM";
    if(y < STRONG_B)
      j_out += ", BS";
    if(y < MILD_B && y >= STRONG_B)
      j_out += ", BM";
    if(x < MILD_R && x > MILD_L && y > MILD_B && y < MILD_T)
      j_out = ", center";
    if(button){
      j_out = ", button";
    }
    return j_out;
  }
}

void checkJoystick(){
  String j_out = handleJoystick();
  if((j_out != "") && (p_j_out != j_out)){
   Serial.println("JOYSTICK"+j_out);
   Serial1.println("JOYSTICK"+j_out);
   p_j_out = j_out;
  }
};
