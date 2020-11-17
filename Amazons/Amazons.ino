// AMAZONS
/* Jens Bossaert
   https://boardgamegeek.com/boardgame/2125/amazons
   macOS Catalina 10.15.7
   Arduino IDE 1.8.13
*/

/* Set up the hardware definitions. */
#include "hardware.h"

/* Set up the graphic definitions. */
#include "graphics.h"

/* Set up generic game template. */
#include "game.h"

// ——————————————————————————————————————————————————————————————————

/* GAME IMPLEMENTATION*/

//todo: fix and move to "graphics.h"
class MoveAnimation : public Animation {
  private:
    //internal data
    Cell cellFrom;
    Cell cellTo;
    short dx;
    short dy;
    Time stepTime;

    //calculate position from current time
    Cell move() {
      short p = floor(timing() / stepTime) + 1;
      //short p = floor(timing() / stepTime + .5);
      return Cell(cellFrom.x() + p * dx, cellFrom.y() + p * dy);
    }

  public:
    //constructor
    MoveAnimation(Cell cellFrom0, Cell cellTo0, Color color0, Time stepTime0 = 1000) :
      cellFrom(cellFrom0), cellTo(cellTo0) {
      stepTime = stepTime0;
      dx = cellTo.x() - cellFrom.x();
      dy = cellTo.y() - cellFrom.y();
      animationDuration = stepTime * max(abs(dx), abs(dy)) - .5 * stepTime;
      dx = (dx > 0) - (dx < 0);
      dy = (dy > 0) - (dy < 0);
      animationColor = color0;
    }

    //animate function
    void animate() override {
      leds[cellTo.xy] = Color(0x000000);
      //writeLeds();
      //if (!(move() == cellFrom))
      leds[move().xy] = animationColor;
    }
};

// ——————————————————————————————————————————————————————————————————

class Amazons : public Game {
  private:
    enum CellType {blank, shot, player1, player2};
    enum GameState {selectMoveFrom, selectMoveTo, selectShoot, gameOver};

  public:
    /* Set up the game definitions. */
    Color player1Color = Color(0x11ff33);
    Color player2Color = Color(0x3311ff);
    Color fireColor = Color(0xcc6600);

    /* Define internal data. */
    CellType cellTypes[64];
    GameState state;
    Cell player1Pieces[4];
    Cell player2Pieces[4];
    Cell activeCell;
    bool player1Turn;

    /* Define graphical layers. */
    PulseAnimation allPiecesAnimation = PulseAnimation();
    PulseAnimation onePieceAnimation = PulseAnimation();

    /* Game setup. */
    void gameSetup() override {
      //starting positions
      player1Pieces[0] = Cell(0, 2);
      player1Pieces[1] = Cell(2, 0);
      player1Pieces[2] = Cell(5, 0);
      player1Pieces[3] = Cell(7, 2);
      player2Pieces[0] = Cell(0, 5);
      player2Pieces[1] = Cell(2, 7);
      player2Pieces[2] = Cell(5, 7);
      player2Pieces[3] = Cell(7, 5);
      //
      player1Turn = true;
      state = selectMoveFrom;
      //
      for (int i = 0; i < 64; ++i)
        cellTypes[i] = blank;
      for (int i = 0; i < 4; ++i) {
        cellTypes[player1Pieces[i].xy] = player1;
        cellTypes[player2Pieces[i].xy] = player2;
      }
      //
      colorBackground();
      setAnimations();
    }

    /* Auxiliary methods. */
    CellType activePlayer() {
      return player1Turn ? player1 : player2;
    }

    CellType opposingPlayer() {
      return player1Turn ? player2 : player1;
    }

    Cell* activePieces() {
      return player1Turn ? player1Pieces : player2Pieces;
    }

    Color activeColor() {
      return player1Turn ? player1Color : player2Color;
    }

    Color opposingColor() {
      return player1Turn ? player2Color : player1Color;
    }

    /* Game move check methods. */
    bool canReachInDirection(byte x, byte y, short ix, short iy, Cell to, Color mark = Color(0x000000)) {
      while ((x += ix) < 8 && (y += iy) < 8 && cellTypes[Cell(x, y).xy] == blank) {
        leds[Cell(x, y).xy] = mark;
        if (Cell(x, y) == to)
          return true;
      }
      return false;
      /*do {
        x += ix;
        y += iy;
        if (cellTypes[Cell(x, y).xy] != blank)
          break;
        leds[Cell(x, y).xy] = activeColor().darker(1);
        if (Cell(x, y) == to && x < 8 && y < 8)
          return true;
        } while (x < 8 && y < 8);
        return false;*/
    }

    bool canReach(Cell from, Cell to, Color mark = Color(0x000000)) {
      //return cellTypes[to.xy] == blank && (from.x() == to.x() || from.y() == to.y());
      /*while (++x < 8 && cellTypes[Cell(x, y).xy] == blank)
        if (Cell(x, y) == to)
          return true;*/
      bool reach = false;
      byte x = from.x();
      byte y = from.y();
      reach |= canReachInDirection(x, y,  1,  0, to, mark); //east
      reach |= canReachInDirection(x, y,  1,  1, to, mark); //north east
      reach |= canReachInDirection(x, y,  0,  1, to, mark); //north
      reach |= canReachInDirection(x, y, -1,  1, to, mark); //north west
      reach |= canReachInDirection(x, y, -1,  0, to, mark); //west
      reach |= canReachInDirection(x, y, -1, -1, to, mark); //south west
      reach |= canReachInDirection(x, y,  0, -1, to, mark); //south
      reach |= canReachInDirection(x, y,  1, -1, to, mark); //south east
      return reach;
    }

    /* Game move methods. */
    bool moveFrom(Cell from) {
      if (state == selectMoveFrom && cellTypes[from.xy] == activePlayer()) {
        activeCell = from;
        state = selectMoveTo;
        return true;
      } else
        return false;
    }

    bool moveTo(Cell to) {
      if (state == selectMoveTo && activeCell.xy == to.xy) {
        //put piece back
        state = selectMoveFrom;
        return true;
      } else if (state == selectMoveTo && canReach(activeCell, to)) {
        //valid move
        cellTypes[activeCell.xy] = blank;
        cellTypes[to.xy] = activePlayer();
        state = selectShoot;
        for (int i = 0; i < 4; ++i)
          if (activePieces()[i].xy == activeCell.xy)
            activePieces()[i] = to;
        activeCell = to;
        return true;
      } else
        return false;
    }

    bool shootCell(Cell to) {
      if (state == selectShoot && canReach(activeCell, to)) {
        //valid shot
        cellTypes[to.xy] = shot;
        state = selectMoveFrom;
        player1Turn = !player1Turn;
        return true;
      } else
        return false;
    }

    /* Game graphic methods. */
    void colorBackground() override {
      for (int x = 0; x < 8; ++x)
        for (int y = 0; y < 8; ++y) {
          Cell currentCell = Cell(x, y);
          switch (cellTypes[currentCell.xy]) {
            case player1:
              leds[currentCell.xy] = player1Color;
              break;
            case player2:
              leds[currentCell.xy] = player2Color;
              break;
            case shot:
              leds[currentCell.xy] = fireColor;
              break;
            default:
              leds[currentCell.xy] = Color(0x000000);
          }
        }
    }

    void colorMoveHints() {
      if (state == selectMoveTo)
        //NullCell is non-existing, forcing to check the whole board
        canReach(activeCell, NullCell, activeColor().darker(6));
      if (state == selectShoot)
        canReach(activeCell, NullCell, Color(0x050200));
    }

    void setAnimations() {
      switch (state) {
        case selectMoveFrom:
          onePieceAnimation.finish();
          allPiecesAnimation = PulseAnimation(activeColor(), 1000);
          for (int i = 0; i < 4; ++i)
            if (hasLegalMoveWith(activePieces()[i]))
              allPiecesAnimation.addCell(activePieces()[i]);
          startAnimation(&allPiecesAnimation);
          break;
        case selectMoveTo:
          allPiecesAnimation.finish();
          onePieceAnimation = PulseAnimation(activeColor(), 1000);
          onePieceAnimation.addCell(activeCell);
          startAnimation(&onePieceAnimation);
          break;
        case selectShoot:
          onePieceAnimation.clearMask();
          onePieceAnimation.addCell(activeCell);
          onePieceAnimation.setPeriod(333);
          break;
        default:
          break;
      }
    }

    /* Game loop. */
    void gameLoop() override {
      readButtons();
      colorBackground();
      colorMoveHints();
      animate();
    };

    /* Game end checks. */
    bool hasLegalMoveWith(Cell piece) {
      bool hasMove = false;
      uint8_t x = piece.x();
      uint8_t y = piece.y();
      //check if at least one neighbor (or piece itself) has type blank
      for (int i = (x == 0 ? 0 : -1); i <= (x == 7 ? 0 : 1); ++i)
        for (int j = (y == 0 ? 0 : -1); j <= (y == 7 ? 0 : 1); ++j)
          hasMove |= cellTypes[Cell(x + i, y + j).xy] == blank;
      return hasMove;
    }

    bool hasLegalMove() {
      bool hasMove = false;
      for (int i = 0; i < 4; ++i)
        hasMove |= hasLegalMoveWith(activePieces()[i]);
      return hasMove;
    }

    /* Button methods. */
    void buttonPressed(Cell pressed) override {
      Cell currentCell = activeCell;
      switch (state) {
        case selectMoveFrom:
          if (moveFrom(pressed)) {//made a valid choice
            setAnimations();
          } else {
            if (cellTypes[pressed.xy] == (opposingPlayer())) //wrong player!
              throwError();
            else //made an invalid blank choice, but who cares
            {};
          }
          break;
        case selectMoveTo:
          if (moveTo(pressed)) {
            if (state == selectShoot) { //made a valid move
              startAnimation(new MoveAnimation(currentCell, pressed, activeColor(), 64), true);
              setAnimations();
            } else //undone selection
              setAnimations();
          } else //made an invalid move
            throwError();
          break;
        case selectShoot:
          if (shootCell(pressed)) //made a valid shot
            if (hasLegalMove()) {//continue to opponent
              setAnimations();
              startAnimation(new MoveAnimation(currentCell, pressed, fireColor, 32), true);
            } else { //game over
              state = gameOver;
              onePieceAnimation.finish();
              startAnimation(new SwirlAnimation(opposingColor()));
            } else //made an invalid shot
            throwError();
          break;
        case gameOver:
          break;
      }
    }
};

// ——————————————————————————————————————————————————————————————————

/* Initialize pins and led strip, and set up a new game. */
void setup() {
  game = new Amazons();
  game->gameSetup();
}

/* Check for button presses and update animations. */
void loop() {
  game->gameLoop();
}
