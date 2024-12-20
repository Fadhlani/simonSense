#include <FastLED.h>

#define NUM_SENSORS 8 //total number of sensors
#define LEDS_PER_SENSOR 3 //number of leds per panel/sensor

#define LED_TYPE WS2812 //type of LED used
#define NUM_LEDS (NUM_SENSORS * LEDS_PER_SENSOR) //number of leds used per led array
#define BRIGHTNESS 100 //brightness for the led array
#define LED_PIN 9 //led pin

#define DEFAULT_THRESHOLD 500  //sensor threshold

int sensorPins[NUM_SENSORS] = {
  A0, //sensor #1
  A1, //sensor #2
  A2, //sensor #3
  A3, //sensor #4
  A4, //sensor #5
  A5, //sensor #6
  A6, //sensor #7
  A7, //sensor #8
}; //sensor pins

//define sensor thresholds in case of inconsistency with specific sensors, set to "DEFAULT_THRESHOLD" if no change necessary
int sensorThresholds[NUM_SENSORS] = {
  DEFAULT_THRESHOLD, //sensor #1
  DEFAULT_THRESHOLD, //sensor #2
  DEFAULT_THRESHOLD, //sensor #3
  DEFAULT_THRESHOLD, //sensor #4
  DEFAULT_THRESHOLD, //sensor #5
  DEFAULT_THRESHOLD, //sensor #6
  DEFAULT_THRESHOLD, //sensor #7
  DEFAULT_THRESHOLD, //sensor #8
};

//define function that checks sensor with specific threhsold
bool isPressed(int sensor, int sensorVal) {
  return sensorVal > sensorThresholds[sensor];
}

//touch response variables
bool ledStates[NUM_SENSORS] = {false}; //track led states for switch functionality
bool previousStates[NUM_SENSORS] = {false} ; //keep a track of previous sensor states

CRGB leds[NUM_LEDS];

bool testMode = false;

//define colors for each led-sensor pair
CRGB colors[NUM_SENSORS] = {
  0xFF0000, //color for sensor's 1 LEDS - RED
  0xFFFF00, //color for sensor's 2 LEDS - YELLOW
  0x008000, //color for sensor's 3 LEDS - GREEN
  0x0000FF, //color for sensor's 4 LEDS - BLUE
  0xFF0074, //color for sensor's 5 LEDS - PINK
  0xFFFFFF, //color for sensor's 6 LEDS - WHITE
  0x00FFFF, //color for sensor's 7 LEDS - CYAN
  0x5C00AD, //color for sensor's 8 LEDS - PURPLE
};

//simon sense variables
#define MAX_GAME_LENGTH 10
bool simonSenseMode = false;
int gameSequence[MAX_GAME_LENGTH] = {};
int gameLen = 2; //starting game length
int playerIndex = 0;
bool playerTurn = false;
bool gameActive = false;



void setup() {
  delay(2000); //safety delay

  FastLED.addLeds<LED_TYPE, LED_PIN, GRB>(leds,NUM_LEDS);
  
  FastLED.setBrightness(BRIGHTNESS); //set led brightness

  if (testMode) {
    Serial.begin(9600);
  }
}

void loop() {
  //Mode Toggle logic
  if(isPressed(5,analogRead(sensorPins[5])) && isPressed(7,analogRead(sensorPins[7]))) {
    simonSenseMode = !simonSenseMode;
    initializeSeq();
    resetStates();
    delay(400);
  }
  
  if(simonSenseMode) {
    simonSense();
    delay(250);
  } else {
    for(int i = 0; i < NUM_SENSORS; i++) {
      touchResponse(i);
    }
  }
  FastLED.show();
  delay(25);

}

void touchResponse(int sensor) {
  int sensorVal = analogRead(sensorPins[sensor]);

  //logging and testing functionality
  if(testMode) {
    Serial.print(" Sensor: ");
    Serial.print(sensor + 1);
    Serial.print(" value: ");
    Serial.println(sensorVal);
    Serial.print("LED State: ");
    Serial.println(ledStates[sensor]);
    delay(250);
  }

  //check if sensor is pressed and it wasnt pressed before
  
  if(isPressed(sensor, sensorVal) && !previousStates[sensor]) {
    ledStates[sensor] = !ledStates[sensor];
    previousStates[sensor] = true;
  }

  if(!isPressed(sensor,sensorVal)) {
    previousStates[sensor] = false;
  }
  
  int ledStart = sensor * LEDS_PER_SENSOR;
  int ledEnd = ledStart + LEDS_PER_SENSOR;

  for(int i = ledStart; i < ledEnd; i++) {
    leds[i] = ledStates[sensor] ? colors[sensor] : CRGB(0,0,0);
  }
}

void simonSense() {

  if(!gameActive) {
    //game is not ongoing, display sequence.
    for(int i = 0; i < gameLen; i++) {
      int sensorIndex = gameSequence[i];
      showColors(sensorIndex, colors[sensorIndex]);
      delay(500);
      clearColors(sensorIndex);
      delay(250);
    }
    playerTurn = true;
    gameActive = true;
    playerIndex = 0;
  }
  if(playerTurn) {
    //Check player inputs

    for(int i = 0; i < NUM_SENSORS; i++) {
      int sensorVal = analogRead(sensorPins[i]);
      
      if(testMode) {
        Serial.print("Sequence: ");
        Serial.print(gameSequence[i]);
        Serial.print("Sensor: ");
        Serial.print(i + 1);
        Serial.print(" value: ");
        Serial.println(sensorVal);
        delay(250);
      }
      
      if(isPressed(i,sensorVal) && !previousStates[i]) {
        if(i == gameSequence[playerIndex]) {
          //correct input recieved, show correct input
          showColors(i, colors[i]);
          delay(250);
          clearColors(i);
          //move player to next color index required
          playerIndex++;
          if(playerIndex >= gameLen) {
            //player has finished all sequence, add to sequence
            gameLen++;
            if(gameLen > MAX_GAME_LENGTH) {
              //player finished defined max game, player win condition. show win, then go back to touch responsive mode.
              gameWin();
              delay(500);
              simonSenseMode = false;
              return;
            }
            addToSequence();
            delay(250);
            resetGame();
            return;
          }
        } else {
          //wrong input recieved, game is lost, reset game
          gameLoss();
          delay(500);
          resetGame();
          return;
        }
        previousStates[i] = true;
      }
      if(!isPressed(i,sensorVal)) {
        previousStates[i] = false;
      }
    }
  }
}

void showColors(int sensorIndex, CRGB color) {
  int ledStart = sensorIndex * LEDS_PER_SENSOR;
  int ledEnd = ledStart + LEDS_PER_SENSOR;

  for(int i = ledStart; i < ledEnd; i++) {
    leds[i] = color;
  }
  FastLED.show();
}

void clearColors(int sensorIndex) {
  int ledStart = sensorIndex * LEDS_PER_SENSOR;
  int ledEnd = ledStart + LEDS_PER_SENSOR;

  for(int i = ledStart; i < ledEnd; i++) {
    leds[i] = CRGB(0,0,0);
  }
  FastLED.show();
}

void resetStates() {
  //reset sensor and led states.
  for(int i = 0; i < NUM_SENSORS; i++) {
    previousStates[i] = false;
  }
  for(int i = 0; i < (NUM_SENSORS * LEDS_PER_SENSOR);i++) {
    leds[i] = CRGB(0,0,0);
  }
}

void addToSequence() {
  gameSequence[gameLen] = random(0,NUM_SENSORS);
}

void gameWin() {
  for(int i = 0; i < (NUM_SENSORS * LEDS_PER_SENSOR);i++) {
    leds[i] = CRGB::Green;
  }
  FastLED.show();
  delay(250);
  for(int i = 0; i < (NUM_SENSORS * LEDS_PER_SENSOR);i++) {
    leds[i] = CRGB(0,0,0);
  }
  FastLED.show();
  delay(250);
  for(int i = 0; i < (NUM_SENSORS * LEDS_PER_SENSOR);i++) {
    leds[i] = CRGB::Green;
  }
  FastLED.show();
  delay(250);
  for(int i = 0; i < (NUM_SENSORS * LEDS_PER_SENSOR);i++) {
    leds[i] = CRGB::Green;
  }
  FastLED.show();
}

void gameLoss() {
  gameLen = 2;
  for(int i = 0; i < (NUM_SENSORS * LEDS_PER_SENSOR);i++) {
    leds[i] = CRGB::Red;
  }
  FastLED.show();
  delay(250);
  for(int i = 0; i < (NUM_SENSORS * LEDS_PER_SENSOR);i++) {
    leds[i] = CRGB(0,0,0);
  }
  FastLED.show();
  delay(250);
  for(int i = 0; i < (NUM_SENSORS * LEDS_PER_SENSOR);i++) {
    leds[i] = CRGB::Red;
  }
  FastLED.show();
  delay(250);
  for(int i = 0; i < (NUM_SENSORS * LEDS_PER_SENSOR);i++) {
    leds[i] = CRGB(0,0,0);
  }
  FastLED.show();
  initializeSeq();
}

void resetGame() {
  //start player from index 0 again
  playerTurn = false;
  gameActive = false;
  playerIndex = 0;
  resetStates();
}

//initialize new starting sequence function.
void initializeSeq() {
  for(int i = 0; i < gameLen; i++) {
    gameSequence[i] = random(0, NUM_SENSORS);
  }
}
