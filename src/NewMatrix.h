#ifndef NEWMATRIX_H_
#define NEWMATRIX_H_

#include <Arduino.h>
#include <MatrixSnake.hpp>
#include "Particle.h"
#include "Rocket.h"

#ifndef PATTERN_TICKER
#define PATTERN_TICKER          MATRIX_PATTERN_TICKER // Was changed from PATTERN_TICKER https://github.com/ArminJo/NeoPatterns/commit/759da4d4f27a92e2dbdf8d2795f0f9e0126d58e9
#endif
#ifndef PATTERN_SNAKE
#define PATTERN_SNAKE           SPECIAL_PATTERN_SNAKE // Was changed from PATTERN_SNAKE https://github.com/ArminJo/NeoPatterns/commit/759da4d4f27a92e2dbdf8d2795f0f9e0126d58e9
#endif
#define PATTERN_PLASMA          (PATTERN_SNAKE + 1)
#define PATTERN_PLASMA4         (PATTERN_SNAKE + 2)
#define PATTERN_SMOOTH          (PATTERN_SNAKE + 3)
#define PATTERN_SPARKLE         (PATTERN_SNAKE + 4)
#define PATTERN_GAMEOFLIFE      (PATTERN_SNAKE + 5)
#define PATTERN_FIREFLAT        (PATTERN_SNAKE + 6)
#define PATTERN_FIREWORKS       (PATTERN_SNAKE + 7)
#define PATTERN_DROP            (PATTERN_SNAKE + 8)
#define PATTERN_RING            (PATTERN_SNAKE + 9)
#define PATTERN_FILL            (PATTERN_SNAKE + 10)
#define PATTERN_SPARKLEW        (PATTERN_SNAKE + 11)
#define PATTERN_RANDOMFILL      (PATTERN_SNAKE + 12)
#define LAST_NEWMATRIX_PATTERN  (PATTERN_SNAKE + 12)

class NewMatrix: public MatrixSnake {
public:
    NewMatrix();
    NewMatrix(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, uint8_t aTypeOfPixel,
            void (*aPatternCompletionCallback)(NeoPatterns*)=NULL);

    bool init(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, uint8_t aTypeOfPixel,
            void (*aPatternCompletionCallback)(NeoPatterns*)=NULL);
            
    void fillw(uint32_t c, uint8_t w, uint16_t first, uint16_t count);
    void setMatrixPixelColorw(uint8_t aColumnX, uint8_t aRowY, uint8_t aRed, uint8_t aGreen, uint8_t aBlue, uint8_t aWhite);

    bool update();
    void None();

    void SetColor1(uint32_t color);
    void SetColor2(uint32_t color);

    void SetInterval(uint8_t interval);
    void Reverse();

    void Plasma(float phase = 0, float phaseIncrement = 0.08, float colorStretch = 0.11, uint8_t interval = 60); // 0.08 and 0.11   // 0.03 und 0.3
    void PlasmaUpdate();
    void Plasma4(float phase = 0, float phaseIncrement = 0.08, float colorStretch = 0.11, uint8_t interval = 60); // 0.08 and 0.11   // 0.03 und 0.3
    void Plasma4Update();
    void Smooth(uint8_t wheelSpeed = 16, uint8_t smoothing = 80, uint8_t strength = 50, uint8_t interval = 40);
    void SmoothUpdate();
    void Sparkle(uint8_t interval = 50, uint8_t CreationTimeout = 100, float inc = 2, float dec = 0.95);
    void SparkleUpdate();
    void Sparklew(uint8_t interval = 50, uint8_t CreationTimeout = 100, float inc = 2, float dec = 0.95);
    void SparklewUpdate();

    bool GameOfLifecolorful = false;
    bool GameOfLifeDimup = false;
    void GameOfLife(uint16_t interval = 1000, uint8_t count = 32, bool colorful = false, String initialSeed = "", bool dimup = false);
    void GameOfLifeUpdate();
    void FireFlat(uint8_t interval = 100);
    void FireFlatUpdate();
    void Fireworks();
    void FireworksUpdate();
    void explosion(int pos, float rocketspeed);
    void Drop(uint8_t interval = 100);
    void DropUpdate();
    void Rings(uint8_t interval = 100);
    void RingsUpdate();
    void Fill(uint32_t color, uint8_t w = 0);
    void FillUpdate();
    void RandomFill();
    void RandomFillUpdate();
    
    bool infinite_repetitions = false;

    uint8_t xyToPos(int x, int y);  
    uint8_t getAverage(uint8_t array[], uint8_t i, int x, int y);

#define EXPLOSION_SIZE_MIN 5
#define EXPLOSION_SIZE_MAX 10
    // 60 LED Strip: 50, 100, 0.985 is a good choice (with Interval = 25)
    // Start 0, with maximum speed (100), the rocket should explode at the LATEST at position 50 (of 60). (Which is 10 pixels before maximum)
    // ROCKET_SPEED_MAX should not be >100, as this would skip LEDs.
#define ROCKET_LAUNCH_TIMEOUT_MIN 1000
#define ROCKET_LAUNCH_TIMEOUT_MAX 3000

    uint32_t maxRocketID = 0;
    uint32_t maxParticleID = 0;
    uint32_t currentRocketMillis = 0;
    uint32_t rocketTimeout;
    float explosion_speed = 0.25f;
    uint8_t rocket_speed_min = 50;
    uint8_t rocket_speed_max = 100;
    double rocket_slowdown = 0.985f;

// Drop (Middle high, than to both sides diming out)
#define MAX_DROPS 10
#define MAX_RINGS 1


private:

    /** Plasma **/
    float PlasmaPhase;
    float PlasmaPhaseIncrement;
    float PlasmaColorStretch;

    /** Smooth **/
    byte wPos;
    uint8_t wPosSlow;     
    uint8_t WheelSpeed;
    uint8_t Smoothing;
    uint8_t Strength;
    uint8_t movingPoint_x;
    uint8_t movingPoint_y;
    uint8_t *pixelR;
    uint8_t *pixelG;
    uint8_t *pixelB;
    uint8_t *pixelR_buffer;
    uint8_t *pixelG_buffer;
    uint8_t *pixelB_buffer;

    /** Sparkle **/
    uint32_t currentSparkleMillis = 0;
    uint32_t SparkleMillisInterval = 100;
    float SparkleInc = 2;
    float SparkleDec = 0.95;
    std::vector <Particle> particle_arr;

    /** Rocket **/
    std::vector <Rocket> rocket_arr;

    // Rings
    uint8_t *ring;
    uint8_t *ringBrightness;
    uint8_t *ringDistance;

    // Drops
    uint8_t *drop;
    uint8_t *dropBrightness;

// Convenient 2D point structure
    struct Point {
      double x;
      double y;
    };
};

#endif /* NEWMATRIX_H_ */
