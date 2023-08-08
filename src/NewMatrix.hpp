#ifndef NEWMATRIX_HPP_
#define NEWMATRIX_HPP_
#include "NewMatrix.h"
//#include "Particle.hpp"
NewMatrix::NewMatrix() : // @suppress("Class members should be properly initialized")
        NeoPixel(), MatrixSnake() {
}

/* TODO: Clear, if effect has finished */

// Constructor - calls base-class constructor to initialize strip
NewMatrix::NewMatrix(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, uint8_t aTypeOfPixel, // @suppress("Class members should be properly initialized")
  void (*aPatternCompletionCallback)(NeoPatterns*)) :
  NeoPixel(aColumns * aRows, aPin, aTypeOfPixel), MatrixSnake(aColumns, aRows, aPin, aMatrixGeometry, aTypeOfPixel,
    aPatternCompletionCallback) {}

bool NewMatrix::init(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, uint8_t aTypeOfPixel,
        void (*aPatternCompletionCallback)(NeoPatterns*)) {
  // Für den Smooth-Effekt
  // Allocate a zero initialized block of memory big enough to hold "pixels" uint8_t.
    pixelR = ( uint8_t* ) calloc( aColumns * aRows, sizeof( uint8_t ) );
    pixelG = ( uint8_t* ) calloc( aColumns * aRows, sizeof( uint8_t ) );
    pixelB = ( uint8_t* ) calloc( aColumns * aRows, sizeof( uint8_t ) );
    pixelR_buffer = ( uint8_t* ) calloc( aColumns * aRows, sizeof( uint8_t ) );
    pixelG_buffer = ( uint8_t* ) calloc( aColumns * aRows, sizeof( uint8_t ) );
    pixelB_buffer = ( uint8_t* ) calloc( aColumns * aRows, sizeof( uint8_t ) );          
    NeoPixel::init(aColumns * aRows, aPin, aTypeOfPixel);
    return MatrixSnake::init(aColumns, aRows, aPin, aMatrixGeometry, aTypeOfPixel, aPatternCompletionCallback);
}


/* Additional routines, missing in original Matrix */
void NewMatrix::fillw(uint32_t c, uint8_t w, uint16_t first, uint16_t count) {
  uint16_t i, end;

  if (first >= numLEDs) {
    return; // If first LED is past end of strip, nothing to do
  }

  // Calculate the index ONE AFTER the last pixel to fill
  if (count == 0) {
    // Fill to end of strip
    end = numLEDs;
  } else {
    // Ensure that the loop won't go past the last pixel
    end = first + count;
    if (end > numLEDs)
      end = numLEDs;
  }

  for (i = first; i < end; i++) {
    // setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w)
    uint8_t r = (uint8_t)(c >> 16), g = (uint8_t)(c >> 8), b = (uint8_t)c;
    this->setPixelColor(i, r, g, b, w);
  }
}

void NewMatrix::setMatrixPixelColorw(uint8_t aColumnX, uint8_t aRowY, uint8_t aRed, uint8_t aGreen, uint8_t aBlue, uint8_t aWhite) {
    if (aColumnX < Columns && aRowY < Rows) {
#ifdef TRACE
        printPin();
        Serial.print(F("set x="));
        Serial.print(aColumnX);
        Serial.print(F(" y="));
        Serial.print(aRowY);
        Serial.print(F(" n="));
        Serial.print(LayoutMapping(aColumnX, aRowY));
        Serial.print(F(" Color="));
        Serial.print(aRed);
        Serial.print('|');
        Serial.print(aGreen);
        Serial.print('|');
        Serial.println(aBlue);
#endif
#ifndef SUPPORT_ONLY_DEFAULT_GEOMETRY
        if (LayoutMappingFunction == NULL) {
            setPixelColor(LayoutMapping(aColumnX, aRowY), aRed, aGreen, aBlue, aWhite);
        } else {
            setPixelColor(LayoutMappingFunction(aColumnX, aRowY, Columns, Rows), aRed, aGreen, aBlue, aWhite);
        }
#else
    setPixelColor((Columns * (Rows - aRowY) - aColumnX) - 1, aRed, aGreen, aBlue);
#endif
#ifdef TRACE
    } else {
        printPin();
        Serial.print(F("skip x="));
        Serial.print(aColumnX);
        Serial.print(F(" y="));
        Serial.println(aRowY);
#endif
    }
}

/*
 * Update the pattern.
 * Returns true if update has really happened in order to give the caller a chance to manually change parameters after each update (like color etc.)
 * Calls snake input handler and after each interval calls snake update routine or MatrixNeoPatterns::Update().
 */
bool NewMatrix::update() {

    if ((millis() - lastUpdate) > Interval) {
        // time to update
        if (infinite_repetitions) { TotalStepCounter = 256; }
        switch (ActivePattern)
        {
            case PATTERN_PLASMA:
                PlasmaUpdate();
                break;
            case PATTERN_PLASMA4:
                Plasma4Update();
                break;
            case PATTERN_SMOOTH:
                SmoothUpdate();
                break;
            case PATTERN_SPARKLE:
                SparkleUpdate();
                break;
            case PATTERN_GAMEOFLIFE:
                GameOfLifeUpdate();
                break;
            case PATTERN_FIREFLAT:
                FireFlatUpdate();
                break;
            case PATTERN_FIREWORKS:
                FireworksUpdate();
                break;
            case PATTERN_DROP:
                DropUpdate();
                break;
            case PATTERN_RING:
                RingsUpdate();
                break;
            case PATTERN_FILL:
                FillUpdate();
                break;
            case PATTERN_SPARKLEW:
                SparklewUpdate();
                break;
            case PATTERN_RANDOMFILL:
                RandomFillUpdate();
                break;
            default:
                MatrixSnake::update();
                break;
        }
        return true;
    }
    return false;
}

void NewMatrix::SetInterval(uint8_t interval) {
  Interval = interval;
}

void NewMatrix::Reverse() {
  Direction = ((Direction+2)&2);
}


/****************** Plasma ******************/

// Based upon https://github.com/johncarl81/neopixelplasma
void NewMatrix::Plasma(float phase, float phaseIncrement, float colorStretch, uint8_t interval)
{
  ActivePattern = PATTERN_PLASMA;
  Interval = interval;
  PlasmaPhase = phase;
  PlasmaPhaseIncrement = phaseIncrement;
  PlasmaColorStretch = colorStretch;
}

void NewMatrix::PlasmaUpdate()
{
PlasmaPhase += PlasmaPhaseIncrement;
  // int edge = (int)sqrt(numPixels());
  int xedge = Columns;
  int yedge = Rows;
  // The two points move along Lissajious curves, see: http://en.wikipedia.org/wiki/Lissajous_curve
  // The sin() function returns values in the range of -1.0..1.0, so scale these to our desired ranges.
  // The phase value is multiplied by various constants; I chose these semi-randomly, to produce a nice motion.
  Point p1 = { (sin(PlasmaPhase * 1.000) + 1.0) * (xedge / 2), (sin(PlasmaPhase * 1.310) + 1.0) * (yedge / 2) };
  Point p2 = { (sin(PlasmaPhase * 1.770) + 1.0) * (xedge / 2), (sin(PlasmaPhase * 2.865) + 1.0) * (yedge / 2) };
  // Point p3 = { (sin(PlasmaPhase * 0.250) + 1.0) * (xedge / 2), (sin(PlasmaPhase * 0.750) + 1.0) * (yedge / 2)};
  Point p3 = { (sin(PlasmaPhase * 0.150) + 1.0) * (xedge / 2), (sin(PlasmaPhase * 0.350) + 1.0) * (yedge / 2)};

  byte row, col;

  // For each row...
  for ( row = 0; row < yedge; row++ ) {
    float row_f = float(row);  // Optimization: Keep a floating point value of the row number, instead of recasting it repeatedly.

    // For each column...
    for ( col = 0; col < xedge; col++ ) {
      float col_f = float(col);  // Optimization.

      // Calculate the distance between this LED, and p1.
      Point dist1 = { col_f - p1.x, row_f - p1.y };  // The vector from p1 to this LED.
      float distance1 = sqrt( dist1.x * dist1.x + dist1.y * dist1.y );

      // Calculate the distance between this LED, and p2.
      Point dist2 = { col_f - p2.x, row_f - p2.y };  // The vector from p2 to this LED.
      float distance2 = sqrt( dist2.x * dist2.x + dist2.y * dist2.y );

      // Calculate the distance between this LED, and p3.
      Point dist3 = { col_f - p3.x, row_f - p3.y };  // The vector from p3 to this LED.
      float distance3 = sqrt( dist3.x * dist3.x + dist3.y * dist3.y );

      // Warp the distance with a sin() function. As the distance value increases, the LEDs will get light,dark,light,dark,etc...
      // You can use a cos() for slightly different shading, or experiment with other functions. Go crazy!
      float color_1 = distance1;  // range: 0.0...1.0
      float color_2 = distance2;
      float color_3 = distance3;
      float color_4 = (sin( distance1 * distance2 * PlasmaColorStretch )) + 2.0 * 0.5;
      // float color_4 = (cos( distance1 * distance2 * PlasmaColorStretch )) + 2.0 * 0.5;

      // Square the color_f value to weight it towards 0. The image will be darker and have higher contrast.
      color_1 *= color_1 * color_4;
      color_2 *= color_2 * color_4;
      color_3 *= color_3 * color_4;
      color_4 *= color_4;

      // Scale the color up to 0..7 . Max brightness is 7.
      setMatrixPixelColor(row, col, Color(color_1, color_2, color_3));
    }
  }
  show();
  lastUpdate = millis();
}

/****************** Plasma4 ******************/

// Based upon https://github.com/johncarl81/neopixelplasma
void NewMatrix::Plasma4(float phase, float phaseIncrement, float colorStretch, uint8_t interval)
{
  ActivePattern = PATTERN_PLASMA4;
  Interval = interval;
  PlasmaPhase = phase;
  PlasmaPhaseIncrement = phaseIncrement;
  PlasmaColorStretch = colorStretch;
}

void NewMatrix::Plasma4Update()
{
PlasmaPhase += PlasmaPhaseIncrement;
  // int edge = (int)sqrt(numPixels());
  int xedge = Columns;
  int yedge = Rows;
  // The two points move along Lissajious curves, see: http://en.wikipedia.org/wiki/Lissajous_curve
  // The sin() function returns values in the range of -1.0..1.0, so scale these to our desired ranges.
  // The phase value is multiplied by various constants; I chose these semi-randomly, to produce a nice motion.
  Point p1 = { (sin(PlasmaPhase * 1.000) + 1.0) * (xedge / 2), (sin(PlasmaPhase * 1.310) + 1.0) * (yedge / 2) };
  Point p2 = { (sin(PlasmaPhase * 1.770) + 1.0) * (xedge / 2), (sin(PlasmaPhase * 2.865) + 1.0) * (yedge / 2) };
  // Point p3 = { (sin(PlasmaPhase * 0.250) + 1.0) * (xedge / 2), (sin(PlasmaPhase * 0.750) + 1.0) * (yedge / 2)};
  Point p3 = { (sin(PlasmaPhase * 0.150) + 1.0) * (xedge / 2), (sin(PlasmaPhase * 0.350) + 1.0) * (yedge / 2)};
  Point p4 = { (sin(PlasmaPhase * 0.650) + 1.0) * (xedge / 2), (sin(PlasmaPhase * 0.250) + 1.0) * (yedge / 2)};

  byte row, col;

  // For each row...
  for ( row = 0; row < yedge; row++ ) {
    float row_f = float(row);  // Optimization: Keep a floating point value of the row number, instead of recasting it repeatedly.

    // For each column...
    for ( col = 0; col < xedge; col++ ) {
      float col_f = float(col);  // Optimization.

      // Calculate the distance between this LED, and p1.
      Point dist1 = { col_f - p1.x, row_f - p1.y };  // The vector from p1 to this LED.
      float distance1 = sqrt( dist1.x * dist1.x + dist1.y * dist1.y );

      // Calculate the distance between this LED, and p2.
      Point dist2 = { col_f - p2.x, row_f - p2.y };  // The vector from p2 to this LED.
      float distance2 = sqrt( dist2.x * dist2.x + dist2.y * dist2.y );

      // Calculate the distance between this LED, and p3.
      Point dist3 = { col_f - p3.x, row_f - p3.y };  // The vector from p3 to this LED.
      float distance3 = sqrt( dist3.x * dist3.x + dist3.y * dist3.y );

      // Calculate the distance between this LED, and p4.
      Point dist4 = { col_f - p4.x, row_f - p4.y };  // The vector from p3 to this LED.
      float distance4 = sqrt( dist4.x * dist4.x + dist4.y * dist4.y );

      // Warp the distance with a sin() function. As the distance value increases, the LEDs will get light,dark,light,dark,etc...
      // You can use a cos() for slightly different shading, or experiment with other functions. Go crazy!
      float color_1 = distance1;  // range: 0.0...1.0
      float color_2 = distance2;
      float color_3 = distance3;
      float color_4 = (sin( distance1 * distance4 * PlasmaColorStretch )) + 2.0 * 0.5;
      // float color_4 = (cos( distance1 * distance2 * PlasmaColorStretch )) + 2.0 * 0.5;

      // Square the color_f value to weight it towards 0. The image will be darker and have higher contrast.
      color_1 *= color_1 * color_4;
      color_2 *= color_2 * color_4;
      color_3 *= color_3 * color_4;
      color_4 *= color_4;

      // Scale the color up to 0..7 . Max brightness is 7.
      setMatrixPixelColor(row, col, Color(color_1, color_2, color_3));
    }
  }
  show();
  lastUpdate = millis();
}


/****************** Smooth ******************/

void NewMatrix::Smooth(uint8_t wheelSpeed, uint8_t smoothing, uint8_t strength, uint8_t interval) {
  /* TODO: Funktioniert dies nur für 8x8? */
  ActivePattern = PATTERN_SMOOTH;
  Interval = interval;
  Index = 0;
  WheelSpeed = wheelSpeed;
  Smoothing = smoothing;
  Strength = strength;
  movingPoint_x = 3;
  movingPoint_y = 3;
  // Clear buffer (from previous or different effects)
  for (int i = 0; i < numPixels(); i++) {
    pixelR_buffer[i] = 0;
    pixelG_buffer[i] = 0;
    pixelB_buffer[i] = 0;
  }
}

void NewMatrix::SmoothUpdate() {
  uint32_t c = Wheel(wPos);
  wPosSlow += WheelSpeed;
  wPos = (wPos + (wPosSlow / 10) ) % 255;
  wPosSlow = wPosSlow % 16;

  uint8_t r = (uint8_t)(c >> 16);
  uint8_t g = (uint8_t)(c >>  8);
  uint8_t b = (uint8_t)c;

  movingPoint_x = movingPoint_x + 8 + random(-random(0, 1 + 1), random(0, 1 + 1) + 1);
  movingPoint_y = movingPoint_y + 8 + random(-random(0, 1 + 1), random(0, 1 + 1) + 1);
  if (movingPoint_x < 8) {
    movingPoint_x = 8 - movingPoint_x;
  } else if (movingPoint_x >= 16) {
    movingPoint_x = 22 - movingPoint_x;
  } else {
    movingPoint_x -= 8;
  }

  if (movingPoint_y < 8) {
    movingPoint_y = 8 - movingPoint_y;
  } else if (movingPoint_y >= 16) {
    movingPoint_y = 22 - movingPoint_y;
  } else {
    movingPoint_y -= 8;
  }
  uint8_t startx = movingPoint_x;
  uint8_t starty = movingPoint_y;

  for (int i = 0; i < Strength; i++) {

    movingPoint_x = startx + 8 + random(-random(0, 2 + 1), random(0, 2 + 1) + 1);
    movingPoint_y = starty + 8 + random(-random(0, 2 + 1), random(0, 2 + 1) + 1);

    if (movingPoint_x < 8) {
      movingPoint_x = 8 - movingPoint_x;
    } else if (movingPoint_x >= 16) {
      movingPoint_x = 22 - movingPoint_x;
    } else {
      movingPoint_x -= 8;
    }

    if (movingPoint_y < 8) {
      movingPoint_y = 8 - movingPoint_y;
    } else if (movingPoint_y >= 16) {
      movingPoint_y = 22 - movingPoint_y;
    } else {
      movingPoint_y -= 8;
    }

    if (pixelR[xyToPos(movingPoint_x, movingPoint_y)] < r) {
      pixelR[xyToPos(movingPoint_x, movingPoint_y)]++;
    } else if (pixelR[xyToPos(movingPoint_x, movingPoint_y)] > r) {
      pixelR[xyToPos(movingPoint_x, movingPoint_y)]--;
    }
    if (pixelG[xyToPos(movingPoint_x, movingPoint_y)] < g) {
      pixelG[xyToPos(movingPoint_x, movingPoint_y)]++;
    } else if (pixelG[xyToPos(movingPoint_x, movingPoint_y)] > g) {
      pixelG[xyToPos(movingPoint_x, movingPoint_y)]--;
    }
    if (pixelB[xyToPos(movingPoint_x, movingPoint_y)] < b) {
      pixelB[xyToPos(movingPoint_x, movingPoint_y)]++;
    } else if (pixelB[xyToPos(movingPoint_x, movingPoint_y)] > b) {
      pixelB[xyToPos(movingPoint_x, movingPoint_y)]--;
    }
  }

  movingPoint_x = startx;
  movingPoint_y = starty;

  for (int i = 0; i < numPixels(); i++) {
    pixelR_buffer[i] = (Smoothing / 100.0) * pixelR[i] + (1.0 - (Smoothing / 100.0)) * getAverage(pixelR, i, 0, 0);
    pixelG_buffer[i] = (Smoothing / 100.0) * pixelG[i] + (1.0 - (Smoothing / 100.0)) * getAverage(pixelG, i, 0, 0);
    pixelB_buffer[i] = (Smoothing / 100.0) * pixelB[i] + (1.0 - (Smoothing / 100.0)) * getAverage(pixelB, i, 0, 0);

  }

  for (int i = 0; i < numPixels(); i++) {
    pixelR[i] = pixelR_buffer[i];
    pixelG[i] = pixelG_buffer[i];
    pixelB[i] = pixelB_buffer[i];
    setPixelColor(i, pixelR[i], pixelG[i], pixelB[i]);
  }
  show();
  lastUpdate = millis();
}

/****************** Sparkle ******************/
void NewMatrix::Sparkle(uint8_t interval, uint8_t CreationTimeout, float inc, float dec) {
  None();
  ActivePattern = PATTERN_SPARKLE;
  Interval = interval;
  SparkleInc = inc;
  SparkleDec = dec;
  SparkleMillisInterval = CreationTimeout;
}

void NewMatrix::SparkleUpdate() {
  if (millis() > currentSparkleMillis + SparkleMillisInterval)
  {
    // Spawn new Sparkle
    // Serial.printf("Spawning Sparkle");
    Particle tmpp = Particle(this, random(-1, this->Rows), random(-1, this->Columns), 20, SparkleInc, SparkleDec);
    particle_arr.push_back(tmpp);
    currentSparkleMillis = millis();
  }

  clear();

  // Iterate through all particles
  for (std::vector<Particle>::iterator it = particle_arr.begin(); it != particle_arr.end(); ++it)
  {
    Particle & p = *it;
    p.update();
    // Serial.print(".");
    // Erase Particles which are too dark
    if (p.brightness() < 20)
    {
      it = particle_arr.erase(it); // After erasing, it is now pointing the next element.
      --it;
    }
  }
  show();
}

/****************** Sparklew ******************/
void NewMatrix::Sparklew(uint8_t interval, uint8_t CreationTimeout, float inc, float dec) {
  None();
  ActivePattern = PATTERN_SPARKLEW;
  Interval = interval;
  SparkleInc = inc;
  SparkleDec = dec;
  SparkleMillisInterval = CreationTimeout;
}

void NewMatrix::SparklewUpdate() {
  if (millis() > currentSparkleMillis + SparkleMillisInterval)
  {
    // Spawn new Sparkle
    // Serial.printf("Spawning Sparkle");
    Particle tmpp = Particle(this, random(0, this->Rows), random(0, this->Columns), 20, SparkleInc, SparkleDec);
    particle_arr.push_back(tmpp);
    currentSparkleMillis = millis();
  }

  clear();

  // Iterate through all particles
  for (std::vector<Particle>::iterator it = particle_arr.begin(); it != particle_arr.end(); ++it)
  {
    Particle & p = *it;
    p.updatew();
    // Serial.print(".");
    // Erase Particles which are too dark
    if (p.brightness() < 20)
    {
      it = particle_arr.erase(it); // After erasing, it is now pointing the next element.
      --it;
    }
  }
  show();
}


/****************** GameOfLife ******************/
uint16_t gameoflife_init_interval;
uint8_t gameoflife_init_count;
bool gameoflife_init_colorful;
String gameoflife_init_initialSeed;
bool gameoflife_init_dimup;
void NewMatrix::GameOfLife(uint16_t interval, uint8_t count, bool colorful, String initialSeed, bool dimup)
{
  gameoflife_init_interval = interval;
  gameoflife_init_count = count;
  gameoflife_init_colorful = colorful;
  gameoflife_init_initialSeed = initialSeed;
  gameoflife_init_dimup = dimup;
  GameOfLifecolorful = colorful;
  GameOfLifeDimup = dimup;
  None();
  ActivePattern = PATTERN_GAMEOFLIFE;
  Interval = interval;
  clear();
  int i=0;
  if (initialSeed == "") {
    for (i=0; i<count; i++)
    {
        if (GameOfLifeDimup) {
          setMatrixPixelColor(random(0, this->Rows), random(0, this->Columns), Color(255, 50, 50));
        } else {
          setMatrixPixelColor(random(0, this->Rows), random(0, this->Columns), Color(255, 255, 255));
        }
        

    }
  } else {
    if (initialSeed.length() == Rows*Columns)
    {
      for (int x=0; x<Rows; x++)
      {
        for (int y=0; y<Columns; y++)
        {
          if (initialSeed.substring(y*8+x,y*8+x+1) == "1") {
            if (GameOfLifeDimup) {
              setMatrixPixelColor(x, y, Color(255,50,50));
            } else {
              setMatrixPixelColor(x, y, Color(255,255,255));
            }
          } else {
            setMatrixPixelColor(x, y, Color(0,0,0));
          }
        }
      }
    }
  }
 
  show();

}

void NewMatrix::GameOfLifeUpdate()
{
  int x = 0;
  int y = 0;
  
  color32_t disp[this->Rows][this->Columns];
  uint8_t neighbours[this->Rows][this->Columns];
  color32_t ndisp[this->Rows][this->Columns];
  for (x=0; x<Rows; x++)
  {
    for (y=0; y<Columns; y++)
    {
      disp[y][x] = getMatrixPixelColor(x,y);
      neighbours[y][x] = 0;
    }
  }
  // Check for neighbours
  for (x=0; x<Rows; x++)
  {
    for (y=0; y<Columns; y++)
    {
      if (disp[y][x] == Color(255,255,255) || disp[y][x] == Color(0,255,0) || (getRedPart(disp[y][x]) == 255 && GameOfLifeDimup))
      {
        neighbours[y-1<0?this->Columns-1:y-1][x-1<0?this->Rows-1:x-1]+=1;
        neighbours[y-1<0?this->Columns-1:y-1][x]+=1;
        neighbours[y-1<0?this->Columns-1:y-1][x+1>this->Rows-1?0:x+1]+=1;
        neighbours[y][x-1<0?this->Rows-1:x-1]+=1;
        neighbours[y][x+1>this->Rows-1?0:x+1]+=1;
        neighbours[y+1>this->Columns-1?0:y+1][x-1<0?this->Rows-1:x-1]+=1;
        neighbours[y+1>this->Columns-1?0:y+1][x]+=1;
        neighbours[y+1>this->Columns-1?0:y+1][x+1>this->Rows-1?0:x+1]+=1;
        
      }
    }
  }

  // Die oberste Zeile zeigt teilweise nicht einfach weiß, sondern blinkt in unterschiedlichen Farben
  // Beispiel: gameoflifec|0000001100000001000000000000000000000000000000000000000000000000
  // gameoflife|0000001100000001110000000000000000000000000000000000000000000000
  for (x=0; x<Rows; x++)
  {
    for (y=0; y<Columns; y++)
    {
      if (disp[y][x] == Color(255,255,255) || disp[y][x] == Color(0,255,0) || (getRedPart(disp[y][x]) == 255 && GameOfLifeDimup))
      {
        // Stay alive with two or three neighbours, die otherwise.
        if (neighbours[y][x] == 2 || neighbours[y][x] == 3)
        {
          if (GameOfLifeDimup) {
            ndisp[y][x] = Color(255,min((int)(getGreenPart(disp[y][x])*1.5),255),min((int)(getBluePart(disp[y][x])*1.5),255));
          } else {
            ndisp[y][x] = Color(255,255,255);
          }
        } else {
          if (GameOfLifecolorful) {
            ndisp[y][x] = Color(255,0,0); // Dead
          } else {
            ndisp[y][x] = Color(0,0,0);
          }
        }
      } else {
        // Born a new cell if there are exactly three neighbours.
        if (neighbours[y][x] == 3)
        {
          if (GameOfLifecolorful) {
            ndisp[y][x] = Color(0,255,0);
          } else {
            if (GameOfLifeDimup) {
              ndisp[y][x] = Color(255,50,50);
            } else {
              ndisp[y][x] = Color(255,255,255);
            }
          }
        } else {
          ndisp[y][x] = Color(0,0,0); // To delete dead red cells
        }
      }
    }
  }

  //setMatrixPixelColor(random(0, this->Rows), random(0, this->Columns), Color(random(0,100), random(0,100), random(0,100)));
  uint32_t deadcells = 0;
  for (x=0; x<Rows; x++)
  {
    for (y=0; y<Columns; y++)
    {
      if (ndisp[y][x] == Color(0,0,0)) {
        deadcells++;
      }
      setMatrixPixelColor(x, y, ndisp[y][x]);
    }
  }
  show();
  if (deadcells == getNumberOfPixels())
  {
    GameOfLife(gameoflife_init_interval, gameoflife_init_count, gameoflife_init_colorful, gameoflife_init_initialSeed, gameoflife_init_dimup);
  }
  lastUpdate = millis();
}

/********** FIRE ********/

void NewMatrix::FireFlat(uint8_t interval)
{
  ActivePattern = PATTERN_FIREFLAT;
  Interval = interval;
  // TotalSteps = 255;
  Index = 0;
}

void NewMatrix::FireFlatUpdate()
{
  int r = 255;
  int g = r - 140;
  int b = 0;

  for (int i = 0; i < numPixels(); i++) {
    int flicker = random(0, 70);
    int r1 = r - flicker;
    int g1 = g - flicker;
    int b1 = b - flicker;
    if (g1 < 0) g1 = 0;
    if (r1 < 0) r1 = 0;
    if (b1 < 0) b1 = 0;
    setPixelColor(i, r1, g1, b1);
  }
  show();
  Interval = random(50, 150);
}

/********** FIRE END ****/

/********** FIREWORKS ********/

// Manchmal noch instabil und lässt den ESP abstürzen. Müsste mal mit Serial mal gedebuggt werden...
void NewMatrix::Fireworks()
{
  ActivePattern = PATTERN_FIREWORKS;
  Interval = 20; // 12ms ist so ziemlich die untere Grenze, durch die Berechnugen und Speicherzugriffe.
  // Calculate "good" explosion speed
  // 60 LED Strip: 50, 100, 0.985 is a good choice (with Interval = 25)
  // Start 0, with maximum speed (100), the rocket should explode at the LATEST at position 50 (of 60). (Which is 10 pixels before maximum)
  // rocket_speed_max should not be >100, as this would skip LEDs.
  explosion_speed = 0.25f;
  rocket_speed_max = 100;
  rocket_slowdown = pow(explosion_speed, (float)((float)1 / (float)(2 * numPixels() - 10))); // 0.985f;
  rocket_speed_min = int(log(explosion_speed) / ((numPixels() / 4) * log(rocket_slowdown))) + 1;
  if (rocket_speed_min / 100 < explosion_speed)
  {
    rocket_speed_min += explosion_speed * 100;
  }
  // OnDebugOutput(String(rocket_slowdown, 6));
  // OnDebugOutput(String(rocket_speed_min));
}

/** Debug Output
  haus/RGB5m/strip/DEBUG Start:  0: Speed: 0.960 Pos: 0
  haus/RGB5m/strip/DEBUG Explode 0: Speed 0.249 Pos: 149 Iterations: 282  // Die Anzahl der Iterationen ist gut berechnet (0.96 -- 282 von 290 max). Nur wurde der Slowdown nicht mit einberechnet
*/

void NewMatrix::explosion(int pos, float rocketspeed)
{
  uint8_t hue = random(0, 256);
  uint8_t explosionsize = random(EXPLOSION_SIZE_MIN, EXPLOSION_SIZE_MAX + 1);
  for (int i = 0; i < explosionsize; i++)
  {
    particle_arr.push_back(Particle(this, pos + i - 3, (float)(((float)random(-50, 50)) / 100) + rocketspeed / 2, hue, 1, 0.99f));
  }

}

void NewMatrix::FireworksUpdate()
{
  if (millis() > currentRocketMillis + rocketTimeout)
  {
    // Start a new rocket
    if (random(0, 2) == 0)
    {
      Rocket tmpr = Rocket(this, 0, (float)(((float)random(rocket_speed_min, rocket_speed_max)) / 100), rocket_slowdown);
      // OnDebugOutput(String("Start:  ") + String(tmpr.id()) + String(": Speed: ") + String(tmpr.rocketspeed(), 3) + String(" Pos: 0"));
      rocket_arr.push_back(tmpr);
    }
    else
    {
      Rocket tmpr = Rocket(this, numPixels(), -(float)(((float)random(rocket_speed_min, rocket_speed_max)) / 100), rocket_slowdown);
      // OnDebugOutput(String("Start:  ") + String(tmpr.id()) + String(": Speed: ") + String(tmpr.rocketspeed(), 3) + String(" Pos: ") + String(numPixels()));
      rocket_arr.push_back(tmpr);
    }
    rocketTimeout = random(ROCKET_LAUNCH_TIMEOUT_MIN, ROCKET_LAUNCH_TIMEOUT_MAX + 1);
    currentRocketMillis = millis();
  }

  clear();

  // Iterate through all particles
  for (std::vector<Particle>::iterator it = particle_arr.begin(); it != particle_arr.end(); ++it)
  {
    Particle & p = *it;
    p.update();
    // Erase Particles which are too dark
    if (p.brightness() < 0.1)
    {
      it = particle_arr.erase(it); // After erasing, it is now pointing the next element.
      --it;
    }
  }

  // Iterate through all rockets
  for (std::vector<Rocket>::iterator it = rocket_arr.begin(); it != rocket_arr.end(); ++it)
  {
    Rocket & r = *it;
    // Create Trail on old position
    particle_arr.push_back(Particle(this, r.pos(), 0, 20, 0.3));

    r.update();
    if ((r.rocketspeed() <= explosion_speed) && (r.rocketspeed() >= -explosion_speed))
    {
      // OnDebugOutput(String("Explode ") + String(r.id()) + String(": Speed ") + String(r.rocketspeed(), 3)  + String(" Pos: ") + String(r.pos()) + String(" Iterations: ") + String(r.iteration()));
      explosion( r.pos(), r.rocketspeed());
      it = rocket_arr.erase(it); // After erasing, it is now pointing the next element.
      --it;
    }
  }
  show();
}

/********** FIREWORKS END ****/

/********** DROP ********/

void NewMatrix::Drop(uint8_t interval)
{
  ActivePattern = PATTERN_DROP;
  Interval = interval;
  // TotalSteps = 255;
  Index = 0;
  for (int i = 0; i < 10; i++) {
    drop[i] = 0;
    dropBrightness[i] = 0;
  }
  clear();
}

void NewMatrix::DropUpdate()
{
  // Generate new drop?
  if (random(0, 100) > 50)
  {
    Serial.println("Will generate a new drop");
    // New drop
    // Find first free drop and discard, if no free place
    for (int i = 0; i < MAX_DROPS; i++) {
      if (drop[i] == 0)
      {
        Serial.print("Found a free position for a drop: ");
        // Random position
        drop[i] = random(0, numPixels());
        dropBrightness[i] = 255; // Initial brightness
        Serial.print(i);
        Serial.print(" pos ");
        Serial.println(drop[i]);
        break;
      }
    }
  }

  // Work for all other drops
  for (int i = 0; i < MAX_DROPS; i++) {
    if (drop[i] > 0)
    {
      Serial.print("Updating drop on ");
      Serial.println(i);
      // Current drop
      // dropBrightness[i] = dropBrightness[i]>>1;
      dropBrightness[i] *= 0.9;
      if (dropBrightness[i] <= 8)
      {
        // Brightness to zero for all neighbours
        dropBrightness[i] = 0;
      }
      setPixelColor(drop[i], 0, 0, dropBrightness[i]); // TODO: Other colors?
      // Set neighbouring drops
      int nBright;
      for (int neighbour = 1; neighbour < 5; neighbour++) {
        //nBright = dropBrightness[i] >> neighbour;
        nBright = dropBrightness[i];
        for (int j = 1; j < neighbour; j++)
        {
          nBright *= 0.6;
        }
        Serial.print(neighbour);
        Serial.print(": ");
        Serial.println(nBright);
        if ((drop[i] - neighbour) >= 0)
        {
          setPixelColor(drop[i] - neighbour, 0, 0, nBright);
        }
        if ((drop[i] + neighbour) <= numPixels())
        {
          setPixelColor(drop[i] + neighbour, 0, 0, nBright);
        }
      }
      if (dropBrightness[i] <= 8)
      {
        // Disable this drop
        drop[i] = 0;
      }

    }
  }
  show();
}

/********** DROP END ****/

/********** RINGS ********/

void NewMatrix::Rings(uint8_t interval)
{
  ActivePattern = PATTERN_RING;
  Interval = interval;
  // TotalSteps = 255;
  Index = 0;
  for (int i = 0; i < 10; i++) {
    ring[i] = 0;
    ringBrightness[i] = 0;
    ringDistance[i] = 0;
  }
  clear();
}

void NewMatrix::RingsUpdate()
{
  // Generate new ring?
  if (random(0, 100) > 50)
  {
    Serial.println("Will generate a new ring");
    // New ring
    // Find first free ring and discard, if no free place
    for (int i = 0; i < MAX_RINGS; i++) {
      if (ring[i] == 0)
      {
        Serial.print("Found a free position for a ring: ");
        // Random position
        ring[i] = random(0, numPixels());
        ringBrightness[i] = 255; // Initial brightness
        ringDistance[i] = 0;
        Serial.print(i);
        Serial.print(" pos ");
        Serial.println(ring[i]);
        break;
      }
    }
  }

  // Work for all other rings
  for (int i = 0; i < MAX_RINGS; i++) {
    if (ring[i] > 0)
    {
      Serial.print("Updating ring on ");
      Serial.println(i);
      // Center of the ring
      ringBrightness[i] *= 0.9;
      if (ringBrightness[i] <= 8)
      {
        // Brightness to zero for the middle
        ringBrightness[i] = 0;
      }
      // TODO: It won't work like this?
      setMatrixPixelColor(ring[i] % Columns, round(ring[i] / Rows), Color(0,0,ringBrightness[i])); // TODO: Other colors?
      // setPixelColor(ring[i], 0, 0, ringBrightness[i]);  // OLD
      // Set neighbouring rings
      int nBright;
      // Maximum distance for rings is 10
      // General idea: Start with the middle (max brightness), continue left and right with brightness * 0.9
      // For each step, dim current brightness for ALL pixels simply by 0.7, below thershold -> off
      ringDistance[i]++;
      // Neighbours: Color of middle, dimmed by 0.9 to max distance
      for (int neighbour = 1; neighbour < ringDistance[i]; neighbour++)
      {
        Serial.print("Neighbour ");
        Serial.print(neighbour);
        nBright = 255;
        if (ringBrightness[i] == 0)
        {
          nBright = 0;
        }
        else
        {
          for (int j = 0; j < ringDistance[i] - neighbour; j++)
          {
            nBright *= 0.8;
          }
          nBright *= (1 - 0.1 * ringDistance[i]);
        }

        if (nBright < 10) {
          nBright = 0;
        }
        Serial.print(" brightness: ");
        Serial.println(nBright);
        if ((ring[i] - neighbour) >= 0)
        {
          setPixelColor(ring[i] - neighbour, 0, 0, nBright);
        }
        if ((ring[i] + neighbour) <= numPixels())
        {
          setPixelColor(ring[i] + neighbour, 0, 0, nBright);
        }

      }

      if (ringBrightness[i] <= 8)
      {
        // Disable this ring
        ring[i] = 0;
      }

    }
  }
  show();
}

/********** RINGS END ****/


/********** FILL ********/

void NewMatrix::Fill(uint32_t color, uint8_t w)
{
  ActivePattern = PATTERN_FILL;
  // setPixelColor(n, red, green, blue, white);
  if (w>0)
  {
    fillw(color, w, 0, numPixels());
  } else {
    fill(color, 0, numPixels());
  }
  // fill()
}

void NewMatrix::FillUpdate()
{
  show();
}

/********** FILL END ****/

// 

/********** RandomFill ********/

void NewMatrix::RandomFill()
{
  ActivePattern = PATTERN_RANDOMFILL;
  // setPixelColor(n, red, green, blue, white);
  // fill()
  clear();
}

void NewMatrix::RandomFillUpdate()
{
  // random(0, this->Rows), random(0, this->Columns)
  uint8_t targetx = random(-1, this->Rows+1);
  uint8_t targety = random(-1, this->Columns+1);
  if (getMatrixPixelColor(targetx, targety)==0){
    setMatrixPixelColor(targetx, targety, Color(10, 10, 10));
  }
  show();
}

/********** RandomFill END ****/


/****************** None ******************/

void NewMatrix::None() {
  if (ActivePattern != PATTERN_NONE) {
    clear();
    show();
  }
  ActivePattern = PATTERN_NONE;
}

/****************** Helper functions ******************/

// Convert x y pixel position to matrix position
uint8_t NewMatrix::xyToPos(int x, int y) {
  if (y % 2 == 0) {
    return (x + (y * 8));
  } else {
    return ((7 - x) + (y * 8));
  }
}

uint8_t NewMatrix::getAverage(uint8_t array[], uint8_t i, int x, int y)
{
  // TODO: This currently works only with 8x8 (64 pixel)!
  uint16_t sum = 0;
  uint8_t count = 0;
  if (i >= 8) { //up
    sum += array[i - 8];
    count++;
  }
  if (i < (64 - 8)) { //down
    sum += array[i + 8];
    count++;
  }
  if (i >= 1) { //left
    sum += array[i - 1];
    count++;
  }
  if (i < (64 - 1)) { //right
    sum += array[i + 1];
    count++;
  }
  return sum / count;
}

void NewMatrix::SetColor1(uint32_t color) {
  Color1 = color;
}
void NewMatrix::SetColor2(uint32_t color) {
  LongValue1.Color2 = color;
}

#endif