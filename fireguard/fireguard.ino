#include <Servo.h>

// =====================================================
// SMART FIRE EXTINGUISHING SYSTEM
// Arduino UNO
// Developed By Tanvir
=====================================================

// ----------- PIN CONFIGURATION -----------------------

const byte FLAME1 = 2;
const byte FLAME2 = 3;
const byte FLAME3 = 4;

const byte GAS_SENSOR = A0;

const byte PUMP_RELAY = 5;
const byte SERVO_PIN = 6;
const byte ALARM = 7;
const byte FIRE_LED = 8;
const byte EMERGENCY_RELAY = 9;
const byte MAIN_AC_RELAY = 10;

// ----------- SETTINGS --------------------------------

// Flame sensor modules usually output LOW when flame
// is detected.
const bool FLAME_ACTIVE_LOW = true;

// Adjust according to your MQ-6 module.
const int GAS_THRESHOLD = 400;

// Most relay modules are ACTIVE LOW.
const bool RELAY_ACTIVE_LOW = true;

// Servo sweep range
const int SERVO_MIN = 30;
const int SERVO_MAX = 150;

const int SERVO_STEP = 2;
const unsigned long SERVO_DELAY = 20;

// -----------------------------------------------------

Servo sprayServo;

int servoAngle = 90;
int servoDirection = SERVO_STEP;

unsigned long lastServoMove = 0;

// =====================================================
// RELAY CONTROL
// =====================================================

void relay(byte pin, bool state)
{
  if (RELAY_ACTIVE_LOW)
  {
    digitalWrite(pin, state ? LOW : HIGH);
  }
  else
  {
    digitalWrite(pin, state ? HIGH : LOW);
  }
}

// =====================================================
// FLAME DETECTION
// =====================================================

bool flame(byte pin)
{
  if (FLAME_ACTIVE_LOW)
    return digitalRead(pin) == LOW;
  else
    return digitalRead(pin) == HIGH;
}

// =====================================================
// SERVO SWEEP
// =====================================================

void sweepServo()
{
  if (millis() - lastServoMove >= SERVO_DELAY)
  {
    lastServoMove = millis();

    servoAngle += servoDirection;

    if (servoAngle >= SERVO_MAX)
    {
      servoAngle = SERVO_MAX;
      servoDirection = -SERVO_STEP;
    }

    if (servoAngle <= SERVO_MIN)
    {
      servoAngle = SERVO_MIN;
      servoDirection = SERVO_STEP;
    }

    sprayServo.write(servoAngle);
  }
}

// =====================================================
// NORMAL MODE
// =====================================================

void normalMode()
{
  // Pump OFF
  relay(PUMP_RELAY, false);

  // Servo centered
  sprayServo.write(90);

  // Alarm OFF
  digitalWrite(ALARM, LOW);

  // Fire LED OFF
  digitalWrite(FIRE_LED, LOW);

  // Emergency lights OFF
  relay(EMERGENCY_RELAY, false);

  // Main AC ON
  relay(MAIN_AC_RELAY, true);
}

// =====================================================
// FIRE MODE
// =====================================================

void fireMode()
{
  // Fire indication
  digitalWrite(FIRE_LED, HIGH);

  // Alarm ON
  digitalWrite(ALARM, HIGH);

  // Main AC OFF
  relay(MAIN_AC_RELAY, false);

  // Emergency lights ON
  relay(EMERGENCY_RELAY, true);

  // Water pump ON
  relay(PUMP_RELAY, true);

  // Sweep water spray
  sweepServo();
}

// =====================================================
// GAS MODE
// =====================================================

void gasMode()
{
  // Alarm ON
  digitalWrite(ALARM, HIGH);

  // Main AC OFF
  relay(MAIN_AC_RELAY, false);

  // Emergency lights ON
  relay(EMERGENCY_RELAY, true);

  // Pump MUST remain OFF
  relay(PUMP_RELAY, false);

  // Servo MUST remain OFF/center
  sprayServo.write(90);

  // Fire LED OFF
  digitalWrite(FIRE_LED, LOW);
}

// =====================================================
// SETUP
// =====================================================

void setup()
{
  // Flame sensors
  pinMode(FLAME1, INPUT);
  pinMode(FLAME2, INPUT);
  pinMode(FLAME3, INPUT);

  // Outputs
  pinMode(PUMP_RELAY, OUTPUT);
  pinMode(ALARM, OUTPUT);
  pinMode(FIRE_LED, OUTPUT);
  pinMode(EMERGENCY_RELAY, OUTPUT);
  pinMode(MAIN_AC_RELAY, OUTPUT);

  // Servo
  sprayServo.attach(SERVO_PIN);
  sprayServo.write(90);

  // Start in NORMAL mode
  normalMode();
}

// =====================================================
// MAIN LOOP
// =====================================================

void loop()
{
  // Read three flame sensors
  bool fire1 = flame(FLAME1);
  bool fire2 = flame(FLAME2);
  bool fire3 = flame(FLAME3);

  // Any flame sensor detects fire
  bool fireDetected = fire1 || fire2 || fire3;

  // Read MQ-6
  int gasValue = analogRead(GAS_SENSOR);

  bool gasDetected = gasValue >= GAS_THRESHOLD;

  // =================================================
  // PRIORITY:
  // FIRE > GAS > NORMAL
  // =================================================

  if (fireDetected)
  {
    fireMode();
  }

  else if (gasDetected)
  {
    gasMode();
  }

  else
  {
    normalMode();
  }
}