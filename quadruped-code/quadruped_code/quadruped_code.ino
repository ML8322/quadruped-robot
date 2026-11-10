#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <SoftwareSerial.h>


Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
SoftwareSerial BTSerial(2, 3);  // RX, TX


// Calibrated min/max pulse values for each servo
int servoMins[8] = {80, 0, 100, 90, 120, 70, 100, 130};
int servoMaxs[8] = {500, 460, 490, 470, 480, 480, 480, 580};


// Inverted knees: Servo 1 (BL) and Servo 5 (FR)
bool isInverted[8] = {false, true, false, false, false, true, false, false};


// === Global Flags ===
bool rightLegIsUp = false;
bool leftLegIsUp = false;
bool controlFR = false;
bool controlFL = false;
bool alternateMode = false;


// Track current movement mode
char currentCommand = 'S';  // 'S' = stop


// === Servo Utilities ===


int degreeToPulse(int servoIndex, int degree) {
if (isInverted[servoIndex]) degree = 180 - degree;
return map(degree, 0, 180, servoMins[servoIndex], servoMaxs[servoIndex]);
}


void moveServo(int servoIndex, int angle) {
int pulse = degreeToPulse(servoIndex, angle);
pwm.setPWM(servoIndex, 0, pulse);
}


// Quartic easing with fewer steps and shorter duration
void moveServoSmooth(int servoIndex, int targetAngle, int duration = 60) {
int currentAngle = 90;  // Still assuming midpoint; adjust if tracking actual position
int steps = 6;


for (int i = 0; i <= steps; i++) {
float t = (float)i / steps;
float curve = (t < 0.5) ? 8 * pow(t, 4) : 1 - pow(-2 * t + 2, 4) / 2;
int intermediateAngle = currentAngle + (targetAngle - currentAngle) * curve;
moveServo(servoIndex, intermediateAngle);
delay(duration / steps);
}
}


void moveServoSmooth(uint8_t servo, int targetAngle, int delayPerStep = 10) {
static int currentAngle[16] = {90};  // Track current angle of each servo
int start = currentAngle[servo];
int step = (targetAngle > start) ? 1 : -1;


for (int angle = start; angle != targetAngle; angle += step) {
  int pulse = map(angle, 0, 180, servoMins[servo], servoMaxs[servo]);
  pwm.setPWM(servo, 0, pulse);
  delay(delayPerStep);
}


// Final position
int pulse = map(targetAngle, 0, 180, servoMins[servo], servoMaxs[servo]);
pwm.setPWM(servo, 0, pulse);
currentAngle[servo] = targetAngle;
}


void liftLeg(int kneeServo) {
moveServoSmooth(kneeServo, 20, 100);
}


void lowerLeg(int kneeServo) {
moveServoSmooth(kneeServo, 0, 100);
}


void liftLeg2(int kneeServo) {
moveServoSmooth(kneeServo, 110, 100);
}


void swingLegForward(int hipServo) {
moveServoSmooth(hipServo, 150, 100);
}


void swingLegBackward(int hipServo) {
moveServoSmooth(hipServo, 40, 100);
}


void centerHip(int hipServo) {
moveServoSmooth(hipServo, 90, 100);
}


void BLswingLegBackward(int hipServo) {
moveServoSmooth(hipServo, 40, 100);
}


void BRswingLegForward(int hipServo) {
moveServoSmooth(hipServo, 130, 100);
}


void FLswingLegBackward(int hipServo) {
moveServoSmooth(hipServo, 40, 100);
}


void liftLeg3(int kneeServo) {
moveServoSmooth(kneeServo, 40, 100);
}


void standPose() {
// Step 1: Move hips first
moveServo(0, 90);
moveServo(2, 90);
moveServo(4, 90);
moveServo(6, 90);
delay(1000);


// Step 2: Then lower all legs
moveServo(1, 0);
moveServo(3, 0);
moveServo(5, 0);
moveServo(7, 0);
delay(200);
}


// === Movement Functions ===


void walkForward() {
// Step 1: Move FL + BR legs forward simultaneously
liftLeg(3);               // FL knee servo
liftLeg(7);               // BR knee servo
FLswingLegBackward(2);    // FL hip forward
BRswingLegForward(6);     // BR hip forward


lowerLeg(3);        
lowerLeg(7);        


delay(50);


// Body shift forward after Step 1
moveServo(0, 150);       // BL hip forward (hold position)
moveServo(2, 90);        // FL hip forward (swing)
moveServo(4, 40);        // FR hip backward (hold position)
moveServo(6, 90);        // BR hip backward (swing)
delay(50);


// Step 2: Move FR + BL legs forward simultaneously
liftLeg(5);              // FR knee servo
liftLeg(1);              // BL knee servo
swingLegForward(4);      // FR hip forward
BLswingLegBackward(0);   // BL hip forward (your custom backward function)


lowerLeg(5);        
lowerLeg(1);        


delay(50);


// Body shift forward after Step 2
moveServo(0, 90);        // BL hip neutral after swing
moveServo(2, 150);       // FL hip neutral after swing
moveServo(4, 90);        // FR hip neutral after swing
moveServo(6, 40);        // BR hip neutral after swing
delay(50);
}


void walkBackward() {
// Step 1: Move FL + BR legs backward simultaneously
liftLeg(3);               // FL knee servo
liftLeg(7);               // BR knee servo
swingLegForward(2);       // FL hip backward
swingLegBackward(6);      // BR hip backward
lowerLeg(3);        
lowerLeg(7);        


delay(100);


// Body shift backward after Step 1
moveServo(0, 30);        // BL hip backward (hold position)
moveServo(2, 90);        // FL hip backward (swing)
moveServo(4, 150);       // FR hip backward (hold position)
moveServo(6, 90);        // BR hip backward (swing)
delay(50);


// Step 2: Move FR + BL legs backward simultaneously
liftLeg(5);               // FR knee servo
liftLeg(1);               // BL knee servo
swingLegBackward(4);      // FR hip backward
swingLegForward(0);       // BL hip backward (your custom backward function)


lowerLeg(5);        
lowerLeg(1);


delay(100);


// Body shift backward after Step 2
moveServo(0, 90);        // BL hip neutral after swing
moveServo(2, 30);        // FL hip neutral after swing
moveServo(4, 90);        // FR hip neutral after swing
moveServo(6, 150);       // BR hip neutral after swing
delay(50);
}


void turnRight() {
// Step 1: BR leg step
liftLeg(7);         
swingLegBackward(6);     // REVERSED inside function
lowerLeg(7);        
delay(100);


// Step 2: FL leg step
liftLeg(3);         
swingLegBackward(2);
lowerLeg(3);        
delay(100);


// Step 3: BL leg step
liftLeg(1);         
swingLegBackward(0);
lowerLeg(1);        
delay(100);


// Step 4: FR leg step
liftLeg(5);         
swingLegBackward(4);     // REVERSED inside function
lowerLeg(5);        
delay(100);


// Step 5: Shift robot — hips swing in opposite direction gradually
moveServo(0, 135);  // BL hip forward
moveServo(2, 135);  // FL hip forward
moveServo(4, 135);  // FR hip FORWARD (reversed from backward)
moveServo(6, 135);  // BR hip FORWARD (reversed from backward)
delay(50);


delay(180);
}


void turnLeft() {
// Step 1: BL leg step
liftLeg(1);         
swingLegForward(0);    // BL hip reversed inside function
lowerLeg(1);        
delay(100);


// Step 2: FR leg step
liftLeg(5);         
swingLegForward(4);     // Normal
lowerLeg(5);        
delay(100);


// Step 3: BR leg step
liftLeg(7);         
swingLegForward(6);     // Normal
lowerLeg(7);        
delay(100);


// Step 4: FL leg step
liftLeg(3);         
swingLegForward(2);    // FL hip reversed inside function
lowerLeg(3);        
delay(100);


// Step 5: Shift robot — hips swing in opposite direction gradually
moveServo(0, 45);  // BL hip backward (reversed)
moveServo(2, 45);  // FL hip backward (reversed)
moveServo(4, 45);  // FR hip backward (normal)
moveServo(6, 45);  // BR hip backward (normal)
delay(50);


delay(250);
}


void shiftRight() {
if (!rightLegIsUp) {
  liftLeg(7);
  moveServoSmooth(6, 130);
  lowerLeg(7);
  delay(200);


  liftLeg(3);
  moveServoSmooth(2, 50);
  lowerLeg(3);
  delay(200);


  liftLeg(1);
  moveServoSmooth(0, 130);
  lowerLeg(1);
  delay(200);


  moveServoSmooth(4, 130);
  liftLeg2(5);
  delay(200);


  rightLegIsUp = true;
  leftLegIsUp = false;
  controlFR = true;
  controlFL = false;
}
}


void shiftLeft() {
if (!leftLegIsUp) {
  liftLeg(1);
  moveServoSmooth(0, 50);
  lowerLeg(1);
  delay(200);


  liftLeg(5);
  moveServoSmooth(4, 130);
  lowerLeg(5);
  delay(200);


  liftLeg(7);
  moveServoSmooth(6, 50);
  lowerLeg(7);
  delay(200);


  moveServoSmooth(2, 50);
  liftLeg2(3);
  delay(200);


  leftLegIsUp = true;
  rightLegIsUp = false;
  controlFL = true;
  controlFR = false;
}
}


void standpose() {
standPose();
rightLegIsUp = false;
leftLegIsUp = false;
controlFR = false;
controlFL = false;
}


void waveRight() {
if (controlFR) {
  moveServoSmooth(4, 30);    // FR hip swing forward
} else if (controlFL) {
  moveServoSmooth(2, 40);    // FL hip swing forward
}
}


void waveLeft() {
if (controlFR) {
  moveServoSmooth(4, 170);   // FR hip swing backward
} else if (controlFL) {
  moveServoSmooth(2, 140);   // FL hip swing backward
}
}


void waveUp() {
if (controlFR) moveServoSmooth(5, 140);
else if (controlFL) moveServoSmooth(3, 140);
}


void waveDown() {
if (controlFR) lowerLeg(5);
else if (controlFL) lowerLeg(3);
}


void restpose() {
// Step 1: Raise all legs
moveServoSmooth(1, 150, 5);
moveServoSmooth(3, 185, 5);
moveServoSmooth(5, 170, 5);
moveServoSmooth(7, 155, 5);
delay(1000);




// Step 2: Move hips inwards
moveServo(0, 5);
moveServo(2, 175);
moveServo(4, 0);
moveServo(6, 180);
delay(500);
}


void walkForward2() {
// Step 1: Move FL + BR legs forward simultaneously
liftLeg3(3);               // FL knee servo
liftLeg3(7);               // BR knee servo
FLswingLegBackward(2);    // FL hip forward
BRswingLegForward(6);     // BR hip forward


lowerLeg(3);         
lowerLeg(7);         


delay(200);


// Body shift forward after Step 1
moveServo(0, 130);       // BL hip forward (hold position)
moveServo(2, 90);        // FL hip forward (swing)
moveServo(4, 50);        // FR hip backward (hold position)
moveServo(6, 90);        // BR hip backward (swing)
delay(200);


// Step 2: Move FR + BL legs forward simultaneously
liftLeg3(5);              // FR knee servo
liftLeg3(1);              // BL knee servo
swingLegForward(4);      // FR hip forward
BLswingLegBackward(0);   // BL hip forward (your custom backward function)


lowerLeg(5);         
lowerLeg(1);         


delay(200);


// Body shift forward after Step 2
moveServo(0, 90);        // BL hip neutral after swing
moveServo(2, 130);       // FL hip neutral after swing
moveServo(4, 90);        // FR hip neutral after swing
moveServo(6, 50);        // BR hip neutral after swing
delay(200);
}


void walkBackward2() {
// Step 1: Move FL + BR legs backward simultaneously
liftLeg3(3);               // FL knee servo
liftLeg3(7);               // BR knee servo
swingLegForward(2);       // FL hip backward
swingLegBackward(6);      // BR hip backward
lowerLeg(3);        
lowerLeg(7);        


delay(200);


// Body shift backward after Step 1
moveServo(0, 50);        // BL hip backward (hold position)
moveServo(2, 90);        // FL hip backward (swing)
moveServo(4, 130);       // FR hip backward (hold position)
moveServo(6, 90);        // BR hip backward (swing)
delay(200);


// Step 2: Move FR + BL legs backward simultaneously
liftLeg3(5);               // FR knee servo
liftLeg3(1);               // BL knee servo
swingLegBackward(4);      // FR hip backward
swingLegForward(0);       // BL hip backward (your custom backward function)


lowerLeg(5);        
lowerLeg(1);


delay(200);


// Body shift backward after Step 2
moveServo(0, 90);        // BL hip neutral after swing
moveServo(2, 50);        // FL hip neutral after swing
moveServo(4, 90);        // FR hip neutral after swing
moveServo(6, 130);       // BR hip neutral after swing
delay(200);
}


void turnRight2() {
// Step 1: BR leg step
liftLeg3(7);          
swingLegBackward(6);     // REVERSED inside function
lowerLeg(7);         
delay(200);


// Step 2: FL leg step
liftLeg3(3);          
swingLegBackward(2); 
lowerLeg(3);         
delay(200);


// Step 3: BL leg step
liftLeg3(1);          
swingLegBackward(0); 
lowerLeg(1);         
delay(200);


// Step 4: FR leg step
liftLeg3(5);          
swingLegBackward(4);     // REVERSED inside function
lowerLeg(5);         
delay(200);


// Step 5: Shift robot — hips swing in opposite direction gradually
 moveServo(0, 135);  // BL hip forward
 moveServo(2, 135);  // FL hip forward
 moveServo(4, 135);  // FR hip FORWARD (reversed from backward)
 moveServo(6, 135);  // BR hip FORWARD (reversed from backward)
 delay(50);


delay(400);
}


void turnLeft2() {
// Step 1: BL leg step
liftLeg3(1);          
swingLegForward(0);    // BL hip reversed inside function
lowerLeg(1);         
delay(200);


// Step 2: FR leg step
liftLeg3(5);          
swingLegForward(4);     // Normal
lowerLeg(5);         
delay(200);


// Step 3: BR leg step
liftLeg3(7);          
swingLegForward(6);     // Normal
lowerLeg(7);         
delay(200);


// Step 4: FL leg step
liftLeg3(3);          
swingLegForward(2);    // FL hip reversed inside function
lowerLeg(3);         
delay(200);


// Step 5: Shift robot — hips swing in opposite direction gradually
 moveServo(0, 45);  // BL hip backward (reversed)
 moveServo(2, 45);  // FL hip backward (reversed)
 moveServo(4, 45);  // FR hip backward (normal)
 moveServo(6, 45);  // BR hip backward (normal)
 delay(50);


delay(400);
}


void shift() {


 moveServo(0, 135);  // BL hip backward (reversed)
 moveServo(2, 135);  // FL hip backward (reversed)
 moveServo(4, 45);  // FR hip backward (normal)
 moveServo(6, 45);  // BR hip backward (normal)
 delay(200);


 moveServo(0, 90);  // BL hip backward (reversed)
 moveServo(2, 90);  // FL hip backward (reversed)
 moveServo(4, 90);  // FR hip backward (normal)
 moveServo(6, 90);  // BR hip backward (normal)
 delay(200);
}


// === Arduino Setup & Bluetooth Control ===


void setup() {
Serial.begin(9600);
BTSerial.begin(9600);
pwm.begin();
pwm.setPWMFreq(50);
delay(10);
standPose();
}


void loop() {
 if (BTSerial.available()) {
   char incoming = BTSerial.read();
   Serial.println(incoming);


   static bool aPressed = false;
   static bool cPressed = false;


   // Track button presses
   if (incoming == 'a') aPressed = true;
   if (incoming == 'c') cPressed = true;


   // Toggle alternate mode if both pressed
   if (aPressed && cPressed) {
     alternateMode = true;
     aPressed = false;
     cPressed = false;
     Serial.println("Alternate mode activated");


     // Play shift animation to signal alternate mode is active
     shift();


     return;  // Skip rest of loop to avoid triggering other actions
   }


   // Cancel alternate mode if only 'a' pressed
   if (incoming == 'a' && !cPressed) {
     alternateMode = false;
     Serial.println("Alternate mode deactivated");
     standpose();
     return;
   }


   if (controlFR || controlFL) {
     // Control lifted leg
     switch (incoming) {
       case 'e': waveUp(); break;
       case 'g': waveDown(); break;
       case 'f': waveRight(); break;
       case 'h': waveLeft(); break;
       case 'c': restpose(); break;
       default: break;
     }
   } else {
     // Movement control
     if (alternateMode) {
       switch (incoming) {
         case 'e': walkForward2(); break;
         case 'g': walkBackward2(); break;
         case 'f': turnRight2(); break;
         case 'h': turnLeft2(); break;
         case 'b': shiftRight(); break;
         case 'd': shiftLeft(); break;
         case 'c': restpose(); break;
         default: break;
       }
     } else {
       switch (incoming) {
         case 'e': walkForward(); break;
         case 'g': walkBackward(); break;
         case 'f': turnRight(); break;
         case 'h': turnLeft(); break;
         case 'b': shiftRight(); break;
         case 'd': shiftLeft(); break;
         case 'c': restpose(); break;
         default: break;
       }
     }
   }
 }
}
