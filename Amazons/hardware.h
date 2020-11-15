// ——————————————————————————————————————————————————————————————————

/* TIME TYPE */
typedef unsigned long Time;

/* STACK IMPLEMENTATION */
#include "stack.h"

/* PAIR IMPLEMENTATION */
template<typename A, typename B>
struct Pair {
  A first;
  B second;
};

// ——————————————————————————————————————————————————————————————————

/* CELL CLASS */

class Cell {
  public:
    //internal variables and components
    byte xy;
    byte x() {
      return xy & 7;
    }
    byte y() {
      return xy >> 3;
    }
    //constructor
    Cell(byte x0 = 0, byte y0 = 0) {
      xy = (y0 & 7) << 3 | (x0 & 7);
    }
    //equality definitions
    bool operator == (const Cell& other) {
      return this->xy == other.xy;
    }
    /*bool operator != (const Cell& other) {
      return this->xy != other.xy;
      }*/
};

//"non-existing" cell (outside the board)
class NullCell : public Cell {
  public:
    NullCell () {
      // 10000000
      xy = 1 << 7;
    }
};

// ——————————————————————————————————————————————————————————————————

/* Declare pins. */
#define resetPin 3
#define ledDataPin 5
#define readButtonPin 9
#define sel1Pin 10
#define sel2Pin 11
#define sel3Pin 12
#define latchPin 13 //dom idee, want hier zit de led :(

/* Declare led strip. */
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel strip(64, ledDataPin, NEO_GRB + NEO_KHZ800);

/* Initialize pins and led strip. */
void initializeBoard() {
  pinMode(sel1Pin, OUTPUT);
  pinMode(sel2Pin, OUTPUT);
  pinMode(sel3Pin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(readButtonPin, INPUT);
  digitalWrite(resetPin, HIGH);
  pinMode(resetPin, OUTPUT);
  strip.begin();
}

// ——————————————————————————————————————————————————————————————————

/* Allow for new code uploads. */
void hardreset() {
  digitalWrite(resetPin, LOW);
}
// ——————————————————————————————————————————————————————————————————

/* Placeholders, to be defined in main file! */
void buttonPressed(Cell cell);
void buttonReleased(Cell cell);
void buttonHeldFor(Cell cell, Time t);

/* Detect button presses and releases. */
enum ButtonState {buttonUp = 0, buttonDown = 1};
ButtonState buttonStates[64];
Time lastTimePressed;

void detectButtons() {
  for (int R = 0; R < 8; ++R) {
    digitalWrite(latchPin, LOW);
    digitalWrite(sel1Pin, R & 1 ? HIGH : LOW);
    digitalWrite(sel2Pin, R & 2 ? HIGH : LOW);
    digitalWrite(sel3Pin, R & 4 ? HIGH : LOW);
    digitalWrite(latchPin, HIGH);
    for (int C = 0; C < 8; ++C) {
      digitalWrite(sel1Pin, C & 4 ? HIGH : LOW);
      digitalWrite(sel2Pin, C & 2 ? HIGH : LOW);
      digitalWrite(sel3Pin, C & 1 ? HIGH : LOW);
      Cell currentCell = Cell(R, 7 - C);
      if (digitalRead(readButtonPin)) {
        if (buttonStates[currentCell.xy] == buttonUp) {
          //just been pressed
          lastTimePressed = millis();
          buttonPressed(currentCell);
          buttonStates[currentCell.xy] = buttonDown;
        }
      } else if (buttonStates[currentCell.xy] == buttonDown) {
        //just been released
        buttonHeldFor(currentCell, millis() - lastTimePressed);
        buttonReleased(currentCell);
        buttonStates[currentCell.xy] = buttonUp;
      }
    }
  }
}

// ——————————————————————————————————————————————————————————————————
