typedef enum {
  VALVE_CLOSED,
  VALVE_OPENED,
  VALVE_PAUSED
} ProgramState;

typedef enum {
  SENSOR_IDLE,
  SENSOR_TRIGGERED
} SensorState;

// KONFIGURASJON
const int RELE_PIN = 8;
const int SENSOR_PIN = 3;

// hvor lenge vannventilen er åpen
const unsigned long MAX_VALVE_OPEN = 5000;
// hvor lang pause det må gå før ventilen kan åpnes igjen
const unsigned long MIN_VALVE_PAUSE = 3000;
// SLUTT KONFIGURASJON

ProgramState programState = VALVE_CLOSED;
unsigned long valveOpenTime = 0;

SensorState sensorState = SENSOR_IDLE;
unsigned long lastValveCloseTime = 0;

void setup() {
  pinMode(SENSOR_PIN, INPUT);
  pinMode(RELE_PIN, OUTPUT);

  digitalWrite(RELE_PIN, LOW);
}

void open_valve() {
  digitalWrite(RELE_PIN, HIGH);
  // spiller av en lyd på en piezo når ventilen åpnes
  tone(5, 200, 500);
}

void close_valve() {
  digitalWrite(RELE_PIN, LOW);
}

bool updateSensorState() {
  int sensorValue = digitalRead(SENSOR_PIN);
  bool lowToHighEdge = false;

  switch (sensorState) {
    case SENSOR_IDLE:
      if (sensorValue == HIGH) {
        lowToHighEdge = true;
        sensorState = SENSOR_TRIGGERED;
      }
    break;

    // håndterer et vedvarende HIGH-signal slik
    // at ikke ventilen åpnes igjen uten at signalet først går til LOW
    case SENSOR_TRIGGERED:
      if (sensorValue == LOW) {
        sensorState = SENSOR_IDLE;
      }
    break;
  }
}

void updateProgramState(bool motionDetected) {
  switch (programState) {
    case VALVE_CLOSED:
      if (motionDetected) {
        programState = VALVE_OPENED;
        valveOpenTime = millis();
        open_valve();
      }
    break;

    case VALVE_OPENED: {
      unsigned long valveOpenDuration = millis() - valveOpenTime;

      if (valveOpenDuration >= MAX_VALVE_OPEN) {
        programState = VALVE_PAUSED;
        valveOpenTime = 0;
        lastValveCloseTime = millis();
        close_valve();
      }
      break;
    }

    case VALVE_PAUSED: {
      unsigned long valveClosedDuration = millis() - lastValveCloseTime;
      if (valveClosedDuration >= MIN_VALVE_PAUSE) {
        lastValveCloseTime = 0;
        programState = VALVE_CLOSED;
      }
      break;
    }
  }
}

void loop() {
  bool motionDetected = updateSensorState();
  updateProgramState(motionDetected);
}
