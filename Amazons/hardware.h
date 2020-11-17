// ——————————————————————————————————————————————————————————————————

/* TIME TYPE */
typedef unsigned long Time;

/* STACK CLASS */
#include "stack.h"

/* PAIR CLASS*/
template<typename A, typename B>
struct Pair {
  A first;
  B second;
};

/* CELL CLASS */

struct Cell {
  //internal variables and components
  byte xy;

  byte x() {
    return xy & 7;
  }
  byte y() {
    return xy >> 3;
  }

  //constructor
  Cell(byte x0, byte y0) :
    xy ((y0 & 7) << 3 | (x0 & 7)) {}

  Cell(byte xy0 = 0) :
    xy(xy0) {}

  //equality definitions
  bool operator == (const Cell & other) {
    return this->xy == other.xy;
  }

  /*bool operator != (const Cell& other) {
    return this->xy != other.xy;
    }*/

  bool isCorner() {
    return (x() == 0 || x() == 7) && (y() == 0 || y() == 7);
  }
};

//"non-existing" cell (outside the board)
Cell NullCell = Cell(1 << 7);

// ——————————————————————————————————————————————————————————————————

/* Declare pins. */
#define resetPin 3
#define ledDataPin 5
#define readButtonPin 9
#define sel1Pin 10
#define sel2Pin 11
#define sel3Pin 12
#define latchPin 13

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

/* Allow for new code uploads. */
void hardreset() {
  digitalWrite(resetPin, LOW);
}

// ——————————————————————————————————————————————————————————————————

/* BUTTON MANAGER */

class ButtonManager {
  private:
    enum ButtonState {buttonUp = 0, buttonDown = 1};
    ButtonState buttonStates[64];

    Stack<Pair<Cell, Time>> buttonPressStack;

    void buttonPressedEvent(Cell cell) {
      buttonPressed(cell);
      buttonPressStack.push({cell, millis()});
    }

    void buttonReleasedEvent(Cell cell) {
      buttonReleased(cell);
      buttonPressStack.filter([&] (Pair<Cell, Time> &pair) {
        if (cell == pair.first) {
          buttonHeldFor(cell, millis() - pair.second);
          return false;
        } else
          return true;
      });
    }

  protected:
    void readButtons() {
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
              buttonPressedEvent(currentCell);
              buttonStates[currentCell.xy] = buttonDown;
            }
          } else if (buttonStates[currentCell.xy] == buttonDown) {
            //just been released
            buttonReleasedEvent(currentCell);
            buttonStates[currentCell.xy] = buttonUp;
          }
        }
      }
      checkReset();
    }

  public:
    //to be defined in Game
    virtual void buttonPressed(Cell);
    virtual void buttonReleased(Cell);
    virtual void buttonHeldFor(Cell, Time);
    virtual void reset();

    unsigned int numberOfButtonsPressed() const {
      return buttonPressStack.size();
    }

    void checkReset() {
      uint8_t i = 0;
      for (Pair<Cell, Time> &pair : buttonPressStack)
        if (millis() - pair.second > 3000 && pair.first.isCorner())
          i++;
      if (i >= 2)
        hardreset();
      else if (i == 1)
        reset();
    }
};
