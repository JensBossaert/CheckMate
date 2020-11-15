/* ABSTRACT GAME CLASS */

class Game {
  public:
    /* Global managing all animations on the board.*/
    AnimationManager animator;

    /* Methods to be implemented. */
    virtual void gameSetup() {};
    virtual void buttonPressed(Cell cell) {};
    virtual void buttonReleased(Cell cell) {};
    virtual void buttonHeldFor(Cell cell, Time t) {};
    virtual void gameLoop() {
      animator.animate();
    };

    virtual void colorBackground() {
      for (int i = 0; i < 64; ++i)
        leds[i] = Color(0x000000);
    }

    /* Error visualisation. */
    ErrorAnimation errorAnimation;

    void throwError() {
      if (errorAnimation.isFinished())
        animator.startAnimation(&errorAnimation);
      else
        errorAnimation.reset();
    }
};

// ——————————————————————————————————————————————————————————————————

Game* game;

/* Pass button methods to game. */
void buttonPressed(Cell cell) {
  game->buttonPressed(cell);
}

void buttonReleased(Cell cell) {
  game->buttonReleased(cell);
}

void buttonHeldFor(Cell cell, Time t) {
  if (t > 4000 && (cell.x() == 0 || cell.x() == 7) && (cell.y() || cell.y() == 7))
    hardreset();
  game->buttonHeldFor(cell, t);
}
