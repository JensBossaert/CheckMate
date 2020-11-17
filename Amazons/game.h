/* ABSTRACT GAME CLASS */

class Game : public ButtonManager, public AnimationManager {
  public:
    /* Global managing all animations on the board.*/
    //AnimationManager animator;
    /* Global managing all button presses and releases.*/
    //ButtonManager controller;

    /* Initialize pins and led strip in constructor */
    Game() {
      initializeBoard();
      colorBackground();
    }

    /* Methods to be implemented. */
    //
    virtual void gameSetup() {};
    //
    virtual void gameLoop() {};
    //
    virtual void buttonPressed(Cell cell) {};
    //
    virtual void buttonReleased(Cell cell) {};
    //
    virtual void buttonHeldFor(Cell cell, Time t) {};

    /* Set the background for the game board. */
    virtual void colorBackground() {
      for (int i = 0; i < 64; ++i)
        leds[i] = Color(0x000000);
    }
    
    /* Reset the current session. */
    void reset() {
      //clearAnimations();//todo ???
      gameSetup();
    }

    /* Error visualisation. */
    ErrorAnimation errorAnimation;

    void throwError() {
      if (errorAnimation.isFinished())
        startAnimation(&errorAnimation);
      else
        errorAnimation.reset();
    }
};

// ——————————————————————————————————————————————————————————————————

Game* game;

/*
  void myResetTest();

  void buttonHeldFor(Cell cell, Time t) {
  if (t > 3000 && (cell.x() == 0 || cell.x() == 7) && (cell.y() == 0 || cell.y() == 7))
    myResetTest();
  game->buttonHeldFor(cell, t);
  }




  void colorRing(unsigned short n, Color color) {
  //n = 0, 1, 2, 3
  for (int i = n; i < 8 - n; ++i) {
    leds[Cell(n, i).xy] = color;
    leds[Cell(i, 7 - n).xy] = color;
    leds[Cell(7 - n, i).xy] = color;
    leds[Cell(i, n).xy] = color;
  }
  }

  void myResetTest() {
  //blink rings to center
  Color resetColor = 0xffffff;
  colorRing(3, resetColor);
  writeLeds();
  delay(32);
  colorRing(3, 0x000000);
  colorRing(2, resetColor);
  writeLeds();
  delay(32);
  colorRing(2, 0x000000);
  colorRing(1, resetColor);
  writeLeds();
  delay(32);
  colorRing(1, 0x000000);
  colorRing(0, resetColor);
  writeLeds();
  delay(32);
  colorRing(0, 0x000000);
  writeLeds();
  hardreset();
  }*/
