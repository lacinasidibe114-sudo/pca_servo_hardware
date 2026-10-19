#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// Paramètres de calibration (à ajuster pour tes servos Hitec)
const int SERVO_MIN = 150; // Impulsion pour 0°
const int SERVO_MAX = 600; // Impulsion pour 180°
const int NB_MAX_SERVOS = 16; // Le PCA9685 gère jusqu'à 16 servos

int last_positions[NB_MAX_SERVOS]; 

void setup() {
  Serial.begin(115200);
  pwm.begin();
  pwm.setPWMFreq(50); // Fréquence standard 50Hz
  
  // Initialisation du tableau des dernières positions
  for (int i = 0; i < NB_MAX_SERVOS; i++) {
    last_positions[i] = -1;
    pwm.setPWM(i, 0, map(90, 0, 180, SERVO_MIN, SERVO_MAX)); // Centrer au démarrage
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
            angle = constrain(angle, 0.0, 180.0);
            
            // Anti-jitter
            if (abs(angle - last_positions[pin]) > 0.1) {
              int pulse = map((int)angle, 0, 180, SERVO_MIN, SERVO_MAX);
              pwm.setPWM(pin, 0, pulse);
              last_positions[pin] = (int)angle;
            }
          }
        }
      }
      start_idx = space_idx + 1;
    }
  }
}