#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

const int NB_MAX_SERVOS = 16; 

// =========================================================================
// CALIBRATION PAR CANAL (En microsecondes)
// Valeurs par défaut : 600 µs (0°) à 2400 µs (180°)
// Ajuste ces tableaux selon le servo branché sur chaque canal !
// =========================================================================
int US_MIN[16] = {600, 600, 600, 600,  600,  600,  600,  600,  600,  600,  600,  600,  600,  600,  600,  600};
int US_MAX[16] = {2400, 2400, 2400, 2400, 2400, 2400, 2400, 2400, 2400, 2400, 2400, 2400, 2400, 2400, 2400, 2400};

// Si tu as un servo 270° (ex: DS3225), tu peux augmenter son US_MAX.
// Exemple pour le canal 3 :
// US_MAX[3] = 2700; 

int last_positions[NB_MAX_SERVOS]; 

void setup() {
  Serial.begin(115200);
  pwm.begin();
  pwm.setPWMFreq(50); // Fréquence standard 50Hz
  
  for (int i = 0; i < NB_MAX_SERVOS; i++) {
    last_positions[i] = -1;
    // Centrage initial à 90°
    int center_us = (US_MIN[i] + US_MAX[i]) / 2;
    pwm.writeMicroseconds(i, center_us); 
  }
}

void loop() {
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    
    // Parse la chaîne "P0:90.0 P1:45.0 P2:180.0"
    int start_idx = 0;
    while (start_idx < cmd.length()) {
      int space_idx = cmd.indexOf(' ', start_idx);
      if (space_idx == -1) space_idx = cmd.length();
      
      String token = cmd.substring(start_idx, space_idx);
      
      if (token.startsWith("P")) {
        int colon_idx = token.indexOf(':');
        if (colon_idx != -1) {
          int pin = token.substring(1, colon_idx).toInt();
          float angle = token.substring(colon_idx + 1).toFloat();
          
          if (pin >= 0 && pin < NB_MAX_SERVOS) {
            // Sécurité : on limite l'angle entre 0 et 180
            angle = constrain(angle, 0.0, 180.0);
            
            // Anti-jitter
            if (abs(angle - last_positions[pin]) > 0.1) {
              
              // CONVERSION ANGLE -> MICROSECONDES (selon le canal)
              // map() convertit 0-180 en US_MIN[pin] - US_MAX[pin]
              long pulse_us = map((long)angle, 0, 180, US_MIN[pin], US_MAX[pin]);
              
              // Envoi de l'impulsion précise au PCA9685
              pwm.writeMicroseconds(pin, pulse_us);
              
              last_positions[pin] = (int)angle;
            }
          }
        }
      }
      start_idx = space_idx + 1;
    }
  }
}