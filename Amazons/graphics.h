/* COLOR CLASS */

struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint32_t rgb() {
    return ((uint32_t) r << 16) | ((uint32_t) g << 8) | b;
  }

  //constructors
  Color(uint32_t rgb0 = 0x000000) :
    r((rgb0 >> 16) & 0xff), g((rgb0 >> 8) & 0xff), b(rgb0 & 0xff) {}
  Color(uint8_t r0, uint8_t g0, uint8_t b0) :
    r(r0), g(g0), b(b0) {}

  //linear interpolation
  Color mixWith(Color color2, uint8_t p) {
    color2.r = ((255 - p) * r + p * color2.r) / 255;
    color2.g = ((255 - p) * g + p * color2.g) / 255;
    color2.b = ((255 - p) * b + p * color2.b) / 255;
    return color2;
  }
  //p = 0 => black, p = 255 => no difference
  Color darker(uint8_t p) {
    return mixWith(Color(0x000000), 255 - p);
  }
};

// ——————————————————————————————————————————————————————————————————

/* Global holding the colors to be displayed.*/
Color leds[64];

// ——————————————————————————————————————————————————————————————————

/* ANIMATION BASE CLASS */

class Animation {
  public:
    //internal data
    Time animationStart;// = millis();
    Time animationDuration = 0;
    Color animationColor = Color(0x000000);

    //non-instantiable constructor
    //Animation () {};

    //destructor
    virtual ~Animation() = default;

    //animate function
    virtual void animate();

    //color method
    virtual void setColor(Color color0) {
      animationColor = color0;
    }

    //timing purposes
    Time timing() {
      return millis() - animationStart;
    }

    void reset() {
      animationStart = millis();
    }

    virtual bool isFinished() {
      return animationDuration && timing() > animationDuration;
    }

    virtual void finish() {
      animationDuration = timing();
    }

    void finishNow() {
      animationDuration = timing();
    }
};

// ——————————————————————————————————————————————————————————————————

/* MASKED ANIMATION SUBCLASS */

class MaskedAnimation : public virtual Animation {
  protected:
    //internal data
    uint64_t mask;

    //non-instantiable constructor
    MaskedAnimation(uint64_t mask0 = 0) :
      //"uint64_t mask0 = ~0ull" all colored
      mask(mask0) {};

  public:
    virtual void animate() {};

    //mask manipulation methods
    void clearMask() {
      mask = 0;
    }

    void addCell(Cell cell) {
      mask |= 1ull << cell.xy;
    }

    void removeCell(Cell cell) {
      mask &= 0ull << cell.xy;
    }
};

// ——————————————————————————————————————————————————————————————————

/* PERIODIC ANIMATION SUBCLASS */

class PeriodicAnimation : public virtual Animation {
  protected:
    //internal data
    Time period;

    //non-instantiable constructor
    PeriodicAnimation(Time period0 = 1000) :
      period(period0) {};

  public:
    virtual void animate() {};

    //period manipulation methods
    void setPeriod(Time p) {
      period = p;
    }

    //timing purposes
    void finish() override {
      Time t = timing();
      Time r = t % period;
      animationDuration = r ? t + period - r : t;
    }
};

// ——————————————————————————————————————————————————————————————————

/* STATIC ONE-COLOR "ANIMATION" */

class StaticOneColorAnimation : public MaskedAnimation {
  public:
    //constructor
    StaticOneColorAnimation(Color color0) {
      animationColor = color0;
    }

    //animate function
    void animate() override {
      for (int i = 0; i < 64; ++i) {
        if (mask & (1ull << i)) {
          leds[i] = animationColor;
        }
      }
    }
};

// ——————————————————————————————————————————————————————————————————

/* STATIC "ANIMATION" */
//(just an image really)

class StaticAnimation : public Animation {
  private:
    //internal data
    Color internalColors[64];

  public:
    //constructor
    StaticAnimation() {}

    void setColor(Color color) override {
      for (int i = 0; i < 64; ++i)
        internalColors[i] = color;
    }

    void setColor(Color colors[]) {
      for (int i = 0; i < 64; ++i)
        internalColors[i] = colors[i];
    }

    void setColor(Cell cell, Color color) {
      internalColors[cell.xy] = color;
    }

    //animate function
    void animate() override {
      for (int i = 0; i < 64; ++i)
        leds[i] = internalColors[i];
    }
};

// ——————————————————————————————————————————————————————————————————

/* PULSE ANIMATION */

class PulseAnimation : public PeriodicAnimation, public MaskedAnimation {
  private:
    //internal data
    Color animationColorBis;

    //calculate shade from current time
    Color pulse() {
      //map t to 0 => period/2 => 0 => period/2 => ...
      Time t = timing() % period;
      t = (t < period / 2) ? t : (period - t);
      return animationColor.mixWith(animationColorBis, 2 * 255 * t / period);
    }

  public:
    //constructors
    PulseAnimation(Color colorA0, Color colorB0, Time period0, uint64_t mask0) :
      MaskedAnimation(mask0), animationColorBis(colorB0) {
      animationColor = colorA0;
      period = period0;
    }
    PulseAnimation(Color colorA0 = Color(0x000000), Time period0 = 1000, Cell cell0 = NullCell) :
      PulseAnimation(colorA0, colorA0.darker(64), period0, cell0.xy >> 7 ? 0 : 1ull << cell0.xy) {}

    void setColor(Color colorA0, Color colorB0) {
      animationColor = colorA0;
      animationColorBis = colorB0;
    }

    void setColor(Color color0) override {
      animationColor = color0;
      animationColorBis = color0.darker(64);
    }

    //animate function
    void animate() override {
      Color pulseColor = pulse();
      for (int i = 0; i < 64; ++i) {
        if (mask & (1ull << i)) {
          leds[i] = pulseColor;
        }
      }
    }
};

// ——————————————————————————————————————————————————————————————————

/* ERROR ANIMATION */

class ErrorAnimation : public Animation {
  public:
    ErrorAnimation() {
      animationDuration = 1000;
      animationColor = Color(0xff0000);
    }

    //animate function
    void animate() override {
      if (!(timing() & 128))
        for (int i = 0; i < 8; ++i) {
          leds[Cell(0, i).xy] = animationColor;
          leds[Cell(i, 7).xy] = animationColor;
          leds[Cell(7, i).xy] = animationColor;
          leds[Cell(i, 0).xy] = animationColor;
        }
    }
};

// ——————————————————————————————————————————————————————————————————

/* SWIRL ANIMATION */

class SwirlAnimation : public Animation {
  private:
    //internal data
    unsigned int animationSpeed;

  public:
    //constructors
    SwirlAnimation(Color color0, unsigned int animationSpeed0 = 70) :
      animationSpeed(animationSpeed0) {
      animationColor = color0;
    }

    //animate function
    void animate() override {
      uint8_t i = (timing() / animationSpeed) % 7;
      leds[Cell(0, i).xy] = animationColor;
      leds[Cell(i, 7).xy] = animationColor;
      leds[Cell(7, 7 - i).xy] = animationColor;
      leds[Cell(7 - i, 0).xy] = animationColor;
    }
};

// ——————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————

/* ANIMATION MANAGER */

class AnimationManager {
  private:
    Stack<Pair<Animation*, bool>> animationStack;

    void killFinishedAnimations() {
      animationStack.filter([] (Pair<Animation*, bool> &pair) {
        if (pair.first->isFinished()) {
          if (pair.second)
            delete pair.first;
          return false;
        } else
          return true;
      });
    }

  public:
    void startAnimation(Animation* animation, bool shouldDestruct = false) {
      animation->reset();
      animationStack.push({animation, shouldDestruct});
    }

    /* Set the background for the game board. */
    virtual void colorBackground();

    unsigned int numberOfLayers() const {
      return animationStack.size();
    }

    /* Send leds to the led strip, "snakewise". */
    void writeLeds() {
      for (int i = 0; i < 64; ++i)
        strip.setPixelColor(i & 8 ? i ^ 7 : i, leds[i].rgb());
      strip.show();
    }

    void clearAnimations() {
      for (Pair<Animation*, bool> &pair : animationStack)
        pair.first->finishNow();
      killFinishedAnimations();
    }

    void animate() {
      killFinishedAnimations();
      for (Pair<Animation*, bool> &pair : animationStack)
        pair.first->animate();
      writeLeds();
    }
};
