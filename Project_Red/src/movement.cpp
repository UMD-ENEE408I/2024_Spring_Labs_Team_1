#include "movement.hpp"

#define USER_DONALD
// #define USER_ASHER
// #define USER_STEPHEN

#ifdef USER_DONALD
  #define MPU_YAW_DRIFT 0.01     // Drift correction for turning
  #define MPU_ANG_FIX 1.25         // Turn angle correction
  #define ENC_FIX_FRWD -5         // Forward movement encoder correction
  #define ENC_FIX_BKWD -5       // Backward movement encoder correction
#elif defined(USER_ASHER)
  #define MPU_YAW_DRIFT 0.005         // Drift correction for turning
  #define MPU_ANG_FIX 1.25         // Turn angle correction
  #define ENC_FIX_FRWD 0          // Forward movement encoder correction
  #define ENC_FIX_BKWD 0          // Backward movement encoder correction
#else
  #define MPU_YAW_DRIFT 0         // Drift correction for turning
  #define MPU_ANG_FIX 1.25         // Turn angle correction
  #define ENC_FIX_FRWD 0          // Forward movement encoder correction
  #define ENC_FIX_BKWD 0          // Backward movement encoder correction
#endif




/******************************************************************
 *                          Variables
 ******************************************************************
 */

// Motors
#define M_BACK_BOOST 2     // PWM boost for reversing the motor

const unsigned int M1_IN_1 = 13;
const unsigned int M1_IN_2 = 12;
const unsigned int M2_IN_1 = 25;
const unsigned int M2_IN_2 = 14;

const unsigned int M1_IN_1_CHANNEL = 8;
const unsigned int M1_IN_2_CHANNEL = 9;
const unsigned int M2_IN_1_CHANNEL = 10;
const unsigned int M2_IN_2_CHANNEL = 11;

const unsigned int M1_I_SENSE = 35;
const unsigned int M2_I_SENSE = 34;

const unsigned int PWM_BASE = 92;
const unsigned int PWM_MAX = 255;
const unsigned int PWM_MIN = 68;

const int freq = 5000;
const int resolution = 8;


// Line sensor
Adafruit_MCP3008 adc1;
Adafruit_MCP3008 adc2;

const unsigned int ADC_1_CS = 2;
const unsigned int ADC_2_CS = 17;

int adc1_buf[8];
int adc2_buf[8];

uint8_t lineArray[13];

const float LINE_MID = 5;
float pos_prev = LINE_MID;


// MPU
Adafruit_MPU6050 mpu;


// Encoders
const unsigned int M1_ENC_A = 39;
const unsigned int M1_ENC_B = 38;
const unsigned int M2_ENC_A = 37;
const unsigned int M2_ENC_B = 36;


// Transition Variable
int transition;


/******************************************************************
 *                          Functions
 ******************************************************************
 */


/*
 * Peripheral initialization, put in setup()
 */

void movement_Init() {
  ledcSetup(M1_IN_1_CHANNEL, freq, resolution);
  ledcSetup(M1_IN_2_CHANNEL, freq, resolution);
  ledcSetup(M2_IN_1_CHANNEL, freq, resolution);
  ledcSetup(M2_IN_2_CHANNEL, freq, resolution);

  ledcAttachPin(M1_IN_1, M1_IN_1_CHANNEL);
  ledcAttachPin(M1_IN_2, M1_IN_2_CHANNEL);
  ledcAttachPin(M2_IN_1, M2_IN_1_CHANNEL);
  ledcAttachPin(M2_IN_2, M2_IN_2_CHANNEL);

  adc1.begin(ADC_1_CS);  
  adc2.begin(ADC_2_CS);

  pinMode(M1_I_SENSE, INPUT);
  pinMode(M2_I_SENSE, INPUT);

  while (!mpu.begin()) delay(10);

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}



/*
 *  Basic motor control funcitons
 */

void M1_forward(int pwm_value) {
  ledcWrite(M1_IN_1_CHANNEL, 0);
  ledcWrite(M1_IN_2_CHANNEL, pwm_value);
}

void M2_forward(int pwm_value) {
  ledcWrite(M2_IN_1_CHANNEL, 0);
  ledcWrite(M2_IN_2_CHANNEL, pwm_value);
}

void M1_backward(int pwm_value) {
  ledcWrite(M1_IN_1_CHANNEL, pwm_value+M_BACK_BOOST);
  ledcWrite(M1_IN_2_CHANNEL, 0);
}

void M2_backward(int pwm_value) {
  ledcWrite(M2_IN_1_CHANNEL, pwm_value+M_BACK_BOOST);
  ledcWrite(M2_IN_2_CHANNEL, 0);
}

void M1_stop() {
  ledcWrite(M1_IN_1_CHANNEL, PWM_MAX);
  ledcWrite(M1_IN_2_CHANNEL, PWM_MAX);
}

void M2_stop() {
  ledcWrite(M2_IN_1_CHANNEL, PWM_MAX);
  ledcWrite(M2_IN_2_CHANNEL, PWM_MAX);
}



/*
 *  Line sensor functions
 */


/* 
 * Read line sensor data to find which sensors see white
 */
void readLineSensor() {
  // Read line sensor data
  for (int i = 0; i < 8; i++) {
    adc1_buf[i] = adc1.readADC(i);
    adc2_buf[i] = adc2.readADC(i);
  }
  
  // Digital conversion
  for (int i = 0; i < 7; i++) {
    if (adc1_buf[i]>640) {
      lineArray[2*i] = 0;
    } else {
      lineArray[2*i] = 1;
    }

    if (i<6) {
      if (adc2_buf[i]>640){
        lineArray[2*i+1] = 0;
      } else {
        lineArray[2*i+1] = 1;
      }
    }
  }
}


/* 
 * Using line sensor data, find position of mouse on line
 * 
 * return
 *   pos - Current position of mouse on line
 */
float getLinePosition() {
  float pos = 0;
  uint8_t white_count = 0;

  for (int i = 0; i < 13; i++) {
    if (lineArray[i] == 1) {
      pos += i;
      white_count+=1;
    }
  }

  if (white_count == 0) {
    return pos_prev;
  }
  return pos/white_count;
}



/*
 *  MPU functions
 */


/* 
 * Update current angle based on MPU values
 *   *curr_angle - Current angle the mouse is facing
 *   *g_prev     - Previous g.gyro.z value; updated in function
 *   *t_prev     - Previous millis() value; updated in function
 * 
 * If you do not yet have values for these paramters, use default:
 *   *curr_angle = 0
 *   *g_prev     = 0
 *   *t_prev     = millis()
 */
void updateAngle(float *curr_angle, float *g_prev, unsigned long *t_prev) {
  sensors_event_t a, g, temp; // A struct that holds mpu reading
  float g_0 = 0;              // Current gyroscope reading
  unsigned long t_delta;      // Change in milis() time (ms)
  
  // Latest MPU update
  mpu.getEvent(&a, &g, &temp);
  g_0 = g.gyro.z - MPU_YAW_DRIFT;
  
  // Update time elapsed
  t_delta = millis() - *t_prev;
  
  // Run the integral and increment angle
  *curr_angle += MPU_ANG_FIX*(180/3.1416)*(t_delta*0.001)*(g_0 + *g_prev)/2;
  
  // Update "previous" values
  *g_prev = g_0;
  *t_prev = t_delta + *t_prev;
}



/*
 *  Movement functions
 */


/*
 *  Moves the mouse forward (or backward) by a certain amount, used IMU for PID
 *    enc1 - Pointer to encoder 1 (in reality, can also be enc2)
 *    goal_mm - Amount to move forward (mm)
 *      goal_mm > 0 - Move forward
 *      goal_mm < 0 - Move backward
 */
void moveForwardDist(bool forward, float goal_ms) {
  unsigned long t_start = millis();
  unsigned long t_tot = t_start;

  
  do {
    updateMoveForwardPID(forward);
    delay(10);
    t_tot = millis() - t_start;
  } while(t_tot < goal_ms);
  M1_stop();
  M2_stop();

}


/*
 *  Turns the mouse by a specified amount
 *    goal - Amount to turn (degrees)
 *      goal > 0 - Counter clock-wise
 *      goal < 0 - Clock-wise
 */
void turnAngle(float goal) {
  float curr_angle = 0;             // current angle of mouse
  float g_prev = 0;                 // previous g.gyro.z reading
  unsigned long t_prev = millis();  // previous milis() time (ms)

  // ccw vs. cw
  if (goal > 0) {
    M1_backward(PWM_BASE);
    M2_forward(PWM_BASE);
  } else {
    M1_forward(PWM_BASE);
    M2_backward(PWM_BASE);
  }

  // Poll until goal is reached
  while (abs(curr_angle) < abs(goal)) updateAngle(&curr_angle, &g_prev, &t_prev);
  
  M1_stop();
  M2_stop();
}


/*
 *  Turns a corner using encoders and the IMU
 *    enc1 - Pointer to encoder 1 (in reality, can also be enc2)
 *    ccw  - If true, turns the mouse left
 */
void turnCorner(Encoder enc1, bool ccw) {
  // move forward until axis is aligned
  moveForwardDist(true, 200);

  delay(100);

  // turn 90 degrees
  if(ccw) {
    turnAngle(90);
  } else {
    turnAngle(-90);
  }
}


/*
 *  Moves the mouse forward (or backward) using IMU for PID
 *    forward - If true mouse moves forward, else it moves backward
 */
void updateMoveForwardPID(bool forward) {
  static float g_prev = 0;
  static unsigned long t_prev = millis(); 

  float pid_val = 0;
  float Kp = 2.5;
  float Kd = 30;
  float Ki = 0.015;

  static float error = 0;
  static float prev_error = 0;
  static float total_error = 0;


  // too long between updates, reset PID
  if((millis() - t_prev) >= 100) {
    g_prev = 0;
    t_prev = millis(); 

    error = 0;
    prev_error = 0;
    total_error = 0;
  }


  // update PID
  updateAngle(&error, &g_prev, &t_prev);
  total_error += error;

  pid_val = Kp*error + Kd*(error-prev_error) + Ki*total_error;
  
  prev_error = error;

  // apply PID
  if(forward) {
    M1_forward(PWM_BASE + pid_val);
    M2_forward(PWM_BASE - pid_val);
  } else {
    M1_backward(PWM_BASE - pid_val);
    M2_backward(PWM_BASE + pid_val);
  }
}


/*
 *  Follows a line, function updates the lineFollow PID
 *    enc1 - Pointer to encoder 1 (in reality, can also be enc2)
 */
void updateLineFollow(int boost) {
  static float t_prev = millis();
  float pos;

  static float error = 0;
  static float prev_error = 0;

  float pid_val = 0;
  float Kp = 5;
  float Kd = 50;

  uint8_t intersection = 0;
  
  // too long between updates, reset PID
  if((millis() - t_prev) >= 100) {
    error = 0;
    prev_error = 0;
  }
  t_prev = millis(); 


  // update PID
  readLineSensor();

  pos = getLinePosition();
  error = pos - LINE_MID;

  pid_val = Kp*error + Kd*(error-prev_error);


  // apply PID
  M1_forward(PWM_BASE + boost - pid_val);
  M2_forward(PWM_BASE + boost + pid_val);

  // update "previous" values
  pos_prev = pos;
  prev_error = error;
}


/*
 *  Follows a line, function updates the lineFollow PID
 *    enc1 - Pointer to encoder 1 (in reality, can also be enc2)
 */
void updateLineFollowInter(Encoder enc1) {
  static float t_prev = millis();
  float pos;

  static float error = 0;
  static float prev_error = 0;

  float pid_val = 0;
  float Kp = 5;
  float Kd = 25;

  uint8_t intersection = 0;
  
  // too long between updates, reset PID
  if((millis() - t_prev) >= 100) {
    error = 0;
    prev_error = 0;
  }
  t_prev = millis(); 


  // update PID
  readLineSensor();

  pos = getLinePosition();
  error = pos - LINE_MID;

  pid_val = Kp*error + Kd*(error-prev_error);


  // apply PID
  M1_forward(PWM_BASE - pid_val);
  M2_forward(PWM_BASE + pid_val);


  // check for corner/intersection/ new section
  if((lineArray[0] == 1 || lineArray[12] == 1) && abs(prev_error) <= 4) {
    intersection = intersectionDetect(enc1);

    if(!intersection) {
      transition = 1;
    } else if(intersection >= 3) {
      // maze solving
    }
  }


  // update "previous" values
  pos_prev = pos;
  prev_error = error;
}


/*  
 *  Detect the type of intersection the mouse is at
 * 
 *  return
 *    0 - New section
 *    1 - Left turn
 *    2 - Right turn
 *    3 -  T  intersection
 *    4 - -|  intersection
 *    5 -  |- intersection
 *    6 - -|- intersection
 */
int intersectionDetect(Encoder enc1) {
  // move slightly forward to make sure aligned
  delay(10);

  M1_stop();
  M2_stop();

  readLineSensor();

  uint8_t right_prev = lineArray[0];
  uint8_t left_prev = lineArray[12];

  // move forward again
  moveForwardDist(true, 250); //TODO TEST
  delay(50);

  readLineSensor();

  // determine intersection type
  if((lineArray[0] == 1 && lineArray[12] == 1)) { // 0
    return 0;                                       // new section

  } else if(lineArray[6] == 0) {                  // 1,2,3
    if(left_prev == 1 && right_prev == 1) {         // T intersection
      return 3;
    } else if(right_prev == 1) {                    // right turn
      turnAngle(90);
      return 2;
    } else {                                        // left turn
      turnAngle(-90);
      return 1;
    }
  } else {                                        // 4,5,6
    if(left_prev == 1 && right_prev == 1) {         // -|- intersection
      return 6;
    } else if(right_prev == 1) {                    //  |- intersection
      return 5;
    } else {                                        // -|  intersection
      return 4;
    }
  }
}