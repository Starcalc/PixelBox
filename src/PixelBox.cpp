/*
 *  PixelBox.cpp by Stefan Magerstedt
 *  Effects inspired by Armin Joachimsmeyer
 *  Uses https://github.com/ArminJo/NeoPatterns.
 *
 *  NewPixelBox is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

#define ENABLE_PATTERNS_FOR_MATRIX_AND_SNAKE_DEMO_HANDLER
#define ENABLE_PATTERNS_FOR_SNAKE_AUTORUN
#define ENABLE_PATTERN_RAINBOW_CYCLE
#define ENABLE_PATTERN_COLOR_WIPE
#define ENABLE_PATTERN_FADE
#define ENABLE_PATTERN_SCANNER_EXTENDED
#define ENABLE_PATTERN_FLASH
#define ENABLE_PATTERN_PROCESS_SELECTIVE
#define ENABLE_PATTERN_STRIPES
#define ENABLE_PATTERN_HEARTBEAT
#define ENABLE_PATTERN_BOUNCING_BALL
#define ENABLE_MATRIX_PATTERN_FIRE
#define ENABLE_MATRIX_PATTERN_SNOW

#define NOWIFI // Disable WIFI alltogether
// #define APONLY // Disable Homie with Wifi and MQTT, but open an AP with a Web Application

#include <Arduino.h>
#include "NewMatrix.hpp"
#include "Particle.hpp"
#include "Rocket.hpp"
#include <LongUnion.h> // for faster random()
#ifndef NOWIFI
#include <ArduinoOTA.h>
#include "fixWiFi.h"
#include <Homie.h>

#else
#include <Homie/Config.hpp>
HomieInternals::Interface* homieinterface;
#endif


HomieSetting<long> rowSetting("NEOPIXEL_MATRIX_NUM_ROWS", "Rows in the matrix");
HomieSetting<long> colSetting("NEOPIXEL_MATRIX_NUM_COLUMNS", "Columns in the matrix");
HomieSetting<const char*> pinSetting("PIN_NEOPIXEL_MATRIX", "Pin connected");
HomieSetting<const char*> CONFIGURATIONSetting("NEO_MATRIX_CONFIGURATION", "Geometry CONFIGURATION");
HomieSetting<const char*> PERMUTATIONSetting("NEO_PERMUTATION", "RGB Permutation");
HomieSetting<const char*> SPEEDSetting("NEO_SPEED", "Speed of Neopixels NEO_KHZ400 or NEO_KHZ800");



/* 
    NeoPixelMatrix ist eine "NewMatrix"
    NewMatrix:: MatrixSnake:: MatrixNeoPatterns:: MatrixNeoPixel,NeoPatterns :: NeoPixel
    TODO: Alle bisherigen Effekte wieder realisieren
    TODO: Farben einbinden und entsprechend reagieren (z.B. bei Theaterchase)
    TODO: Fontgröße anpassbar machen aufgrund der jeweils genutzten Dimensionen (PROGMEM aller Fonts?!)
    TODO: Weitere Fontgrößen einbauen, in MatrixNeoPixel liegen "nur" die bis 8x8 vor.
          https://www.mikrocontroller.net/topic/54860
          Vielleicht den Font auf das FS ablegen und dann dynamisch nachladen, wenn man die Größe kennt?
          Allerdings nutzen alle Funktionen von MatrixNeoPatterns den PGM-Zugriff, um den Font zu erzeugen
    TODO: Bitmaps z.B. per MQTT erhalten und diese dann darstellen, genau wie die PGM-Varianten. Ggfs.
          muss man hierfür noch eine neue Variante bauen, die analog wie die PGM funktioniert.
*/
NewMatrix NeoPixelMatrix = NewMatrix();
char lastText[1024];
int lastinterval = 40;
uint32_t lasttextcolor = COLOR32_WHITE;
uint8_t lastbrightness = 255;

uint32_t lastUpdateCounter = 0;
uint32_t updateInterval = 1000 * 10; // 1000 * 30 = 30s
uint32_t effectCounter = 0;

bool stopAfterCompletion;
bool backup_stopAfterCompletion;
void StripComplete(NeoPatterns * aLedsPtr) {
// TODO: Wenn nicht gestoppt werden soll, soll der Effekt noch einmal angestoßen werden.
// Den letzten Effekt merken? Inklusive Aufruf? Oder jeden Effekt umschreiben, dass er ewig läuft wie z.B. Text?
  if (stopAfterCompletion)
  {
    // Next effect, repeat, etc.
  } else {
    if (NeoPixelMatrix.ActivePattern == PATTERN_TICKER) {
      NeoPixelMatrix.Ticker(lastText, lasttextcolor, COLOR32_BLACK, lastinterval, DIRECTION_LEFT);
    }
//    Serial.println("Do not stop after completion.");
  }
  return;
}

void MatrixInit()  {
  /* Lesen der Matrix-Parameter aus der config.json */
  uint8_t localpin;
  if (strcmp(pinSetting.get(), "D0") == 0) {
    localpin = D0;
  } else if (strcmp(pinSetting.get(), "D1") == 0) {
    localpin = D1;
  } else if (strcmp(pinSetting.get(), "D2") == 0) {
    localpin = D2;
  } else if (strcmp(pinSetting.get(), "D3") == 0) {
    localpin = D3;
  } else if (strcmp(pinSetting.get(), "D4") == 0) {
    localpin = D4;
  } else if (strcmp(pinSetting.get(), "D5") == 0) {
    localpin = D5;
  } else if (strcmp(pinSetting.get(), "D6") == 0) {
    localpin = D6;
  } else if (strcmp(pinSetting.get(), "D7") == 0) {
    localpin = D7;
  } else if (strcmp(pinSetting.get(), "D8") == 0) {
    localpin = D8;
  } else {
    localpin = atoi(pinSetting.get());
  }

  String confsetting = CONFIGURATIONSetting.get();
  uint8_t geometry = 0;

  Serial.println(confsetting);

// TODO: https://learn.adafruit.com/adafruit-neopixel-uberguide/neomatrix-library
//       "Other formats" mit eigener Geometrie sind möglich, da kann man auch 16bit-Integers
//       übergeben. Wenn man also in der Konfigurationsdatei irgendeinen Parameter hinterlegt,
//       dass es sich um einen Streifen handelt, könnte das wieder gehen.

  if (confsetting.indexOf("NEO_MATRIX_TOP") > -1) { geometry = geometry | NEO_MATRIX_TOP; }
  if (confsetting.indexOf("NEO_MATRIX_BOTTOM") > -1) { geometry = geometry | NEO_MATRIX_BOTTOM; }
  if (confsetting.indexOf("NEO_MATRIX_LEFT") > -1) { geometry = geometry | NEO_MATRIX_LEFT; }
  if (confsetting.indexOf("NEO_MATRIX_RIGHT") > -1) { geometry = geometry | NEO_MATRIX_RIGHT; }
  if (confsetting.indexOf("NEO_MATRIX_CORNER") > -1) { geometry = geometry | NEO_MATRIX_CORNER; }
  if (confsetting.indexOf("NEO_MATRIX_ROWS") > -1) { geometry = geometry | NEO_MATRIX_ROWS; }
  if (confsetting.indexOf("NEO_MATRIX_COLUMNS") > -1) { geometry = geometry | NEO_MATRIX_COLUMNS; }
  if (confsetting.indexOf("NEO_MATRIX_AXIS") > -1) { geometry = geometry | NEO_MATRIX_AXIS; }
  if (confsetting.indexOf("NEO_MATRIX_PROGRESSIVE") > -1) { geometry = geometry | NEO_MATRIX_PROGRESSIVE; }
  if (confsetting.indexOf("NEO_MATRIX_ZIGZAG") > -1) { geometry = geometry | NEO_MATRIX_ZIGZAG; }
  if (confsetting.indexOf("NEO_MATRIX_SEQUENCE") > -1) { geometry = geometry | NEO_MATRIX_SEQUENCE; }
  
  uint8_t permutation = 0;
  if (strcmp(PERMUTATIONSetting.get(), "NEO_RGB") == 0) { permutation = permutation | NEO_RGB; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_RBG") == 0) { permutation = permutation | NEO_RBG; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_GRB") == 0) { permutation = permutation | NEO_GRB; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_GBR") == 0) { permutation = permutation | NEO_GBR; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_BRG") == 0) { permutation = permutation | NEO_BRG; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_BGR") == 0) { permutation = permutation | NEO_BGR; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_WRGB") == 0) { permutation = permutation | NEO_WRGB; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_WRBG") == 0) { permutation = permutation | NEO_WRBG; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_WGRB") == 0) { permutation = permutation | NEO_WGRB; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_WGBR") == 0) { permutation = permutation | NEO_WGBR; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_WBRG") == 0) { permutation = permutation | NEO_WBRG; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_WBGR") == 0) { permutation = permutation | NEO_WBGR; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_RWGB") == 0) { permutation = permutation | NEO_RWGB; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_RWBG") == 0) { permutation = permutation | NEO_RWBG; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_RGWB") == 0) { permutation = permutation | NEO_RGWB; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_RGBW") == 0) { permutation = permutation | NEO_RGBW; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_RBWG") == 0) { permutation = permutation | NEO_RBWG; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_RBGW") == 0) { permutation = permutation | NEO_RBGW; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_GWRB") == 0) { permutation = permutation | NEO_GWRB; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_GWBR") == 0) { permutation = permutation | NEO_GWBR; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_GRWB") == 0) { permutation = permutation | NEO_GRWB; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_GRBW") == 0) { permutation = permutation | NEO_GRBW; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_GBWR") == 0) { permutation = permutation | NEO_GBWR; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_GBRW") == 0) { permutation = permutation | NEO_GBRW; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_BWRG") == 0) { permutation = permutation | NEO_BWRG; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_BWGR") == 0) { permutation = permutation | NEO_BWGR; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_BRWG") == 0) { permutation = permutation | NEO_BRWG; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_BRGW") == 0) { permutation = permutation | NEO_BRGW; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_BGWR") == 0) { permutation = permutation | NEO_BGWR; } else
  if (strcmp(PERMUTATIONSetting.get(), "NEO_BGRW") == 0) { permutation = permutation | NEO_BGRW; }

  if (strcmp(SPEEDSetting.get(), "NEO_KHZ400") == 0) { permutation = permutation | NEO_KHZ400; } else
  if (strcmp(SPEEDSetting.get(), "NEO_KHZ800") == 0) { permutation = permutation | NEO_KHZ800; } 

  NeoPixelMatrix.init(colSetting.get(), rowSetting.get(), localpin, geometry, permutation, &StripComplete);
}

#ifndef NOWIFI
HomieNode homieNode("pixel", "pixel", "commands");
#endif

bool onSetReverse(const HomieRange & range, const String & value) {
  NeoPixelMatrix.Reverse();
#ifndef NOWIFI
  homieNode.setProperty("reverse").send(value);
#endif
  return true;
}

bool onSetBrightness(const HomieRange& range, const String& value) {
	long brightness = value.toInt();
	if (brightness < 0 || brightness > 255) {
		return false;
	}
	NeoPixelMatrix.setBrightness(brightness);
#ifndef NOWIFI
	homieNode.setProperty("brightness").send(value);
#endif
	return true;
}

bool is_number(String s)
{
  uint16_t Numbers = 0;
  for(uint16_t i = 0; i < s.length(); i++)
  {
    if (s[i] >= '0' && s[i] <= '9')
    {
      Numbers ++;
    }
  }
  if (Numbers == s.length())
  { return true; } else { return false; }
}

uint32_t parseColor(String value) {
  if (value.charAt(0) == '#') { //solid fill
    String color = value.substring(1);
    int number = (int) strtol( &color[0], NULL, 16);
    // Split them up into r, g, b values
    int r = number >> 16;
    int g = number >> 8 & 0xFF;
    int b = number & 0xFF;
    return COLOR32(r, g, b);
  }
  return 0;
}

bool onSetColor0(const HomieRange& range, const String& value) {
  if (value.charAt(0) == '#') {
    NeoPixelMatrix.SetColor1(parseColor(value));
  } else {
    if (!is_number(value)) return false;
    NeoPixelMatrix.SetColor1(value.toInt());
  }
#ifndef NOWIFI
  homieNode.setProperty("color0").setRange(range).send(value);
#endif
  //homieNode.setProperty("color_" + String(range.index)).send(value);
  return true;
}

bool onSetColor1(const HomieRange& range, const String& value) {
  if (value.charAt(0) == '#') {
    NeoPixelMatrix.SetColor2(parseColor(value));
  } else {
    if (!is_number(value)) return false;
    NeoPixelMatrix.SetColor2(value.toInt());
  }
#ifndef NOWIFI
  homieNode.setProperty("color1").setRange(range).send(value);
#endif
  //homieNode.setProperty("color_" + String(range.index)).send(value);
  return true;
}

/***
 * Effektidee: Zwei farbliche "Masken" (veränderbar) für die Darstellung des Textes. Dabei wird die eine für den
 * Hintergrund genutzt, die andere für den eigentlichen Text. So sind dynamische Regenbogenschriftarten möglich,
 * auch innerhalb eines Buchstabens. Oder sogar schneller wechselnd als der Text? (Mit einem zweiten Interval?)
 * 
 * TODO: Randomscanner, rainbowtext, Colors @ rocket and scanner, fire (for flat stripes like in RGB5m), random, randomfade, stars, moving picture (given by MQTT or UDP or...? - maybe store it on the SPIFFS, if possible)
 * aLedsPtr->Stripes(NeoPatterns::Wheel(tColor), 1, NeoPatterns::Wheel(tColor + 0x80), 2, 2 * aLedsPtr->numPixels(),
                tDuration * 2, (tColor & DIRECTION_DOWN));
***/
bool onSetEffect(const HomieRange& range, const String& value) {
  Serial.println(value);
  LongUnion tRandom;
  tRandom.Long = random();
  uint8_t tColor = tRandom.UBytes[1];
  uint8_t tDuration = ((tRandom.UBytes[1] * (80 - 40)) >> 8) + 40;
  NeoPixelMatrix.OnPatternComplete = &StripComplete;
  stopAfterCompletion = false;
  int validparams = 0;
  int sep = value.indexOf("|");
  String param1, param2, param3, param4;
  String command = value;
  if (sep > 0)
  {
    // Parameter given
    command = value.substring(0, sep);
    String parameters = value.substring(sep + 1);
    param1 = parameters;
    validparams = 1;
    int sep2 = parameters.indexOf("|");
    if (sep2 > 0) {
      // Es gibt mindestens zwei Parameter
      param1 = parameters.substring(0, sep2);
      parameters = parameters.substring(sep2 + 1);
      param2 = parameters;
      validparams = 2;
      int sep3 = parameters.indexOf("|");
      if (sep3 > 0)
      {
        // Es gibt mindestens drei Parameter
        param2 = parameters.substring(0, sep3);
        parameters = parameters.substring(sep3 + 1);
        param3 = parameters;
        validparams = 3;
        int sep4 = parameters.indexOf("|");
        if (sep4 > 0)
        {
          // Es gibt mindestens vier Parameter
          param3 = parameters.substring(0, sep4);
          param4 = parameters.substring(sep4 + 1);
          validparams = 4;
        }
      }
    }
  }
	if (command == "text") {
    String stext;
    int interval = 40;
    uint32_t textcolor = COLOR32_WHITE;
    switch (validparams) {
          case 0:
            stext = "(No text given)";
            break;
          case 1:
            stext = param1;
            break;
          case 2:
            stext = param1;
            interval = param2.toInt();
            break;
          case 3:
            stext = param1;
            interval = param2.toInt();
            textcolor = parseColor(param3);
            break;
        }
    lastinterval = interval;
    lasttextcolor = textcolor;
    strncpy(lastText, stext.c_str(), (strlen(stext.c_str())>1022) ? 1022 : strlen(stext.c_str()));
    lastText[strlen(stext.c_str())] = 0;
#ifndef NOWIFI
    homieNode.setProperty("scrollinfo").send("Scroll " + stext + " with interval " + String(interval) + " and color " + String(textcolor));
#endif
    // TODO: Color1 und Color2 verwenden, wie eingestellt?
    NeoPixelMatrix.Ticker(lastText, textcolor, COLOR32_BLACK, interval, DIRECTION_LEFT);
	}
  else if (command == "scanner") {
      NeoPixelMatrix.infinite_repetitions = true;
      NeoPixelMatrix.ScannerExtended(NeoPixelMatrix.Color(255,0,0), 5, 40, 2, FLAG_SCANNER_EXT_CYLON);
   }
  else if (command == "rocket") {
      NeoPixelMatrix.infinite_repetitions = true;
      NeoPixelMatrix.ScannerExtended(NeoPixelMatrix.Color(255,0,0), 5, 40, 2, FLAG_SCANNER_EXT_ROCKET);
  }
  else if (command == "none" || command == "off") {
    NeoPixelMatrix.infinite_repetitions = false;
    NeoPixelMatrix.None();
  }
  else if (command == "snow") {
    NeoPixelMatrix.infinite_repetitions = true;
    NeoPixelMatrix.Snow();
  }
  else if (command == "randomscanner") {
    // TODO: Randomscanner
  //   strip.Scanner(strip.Color(255, 0, 0), 40, true);
  }
  else if (command == "larsonspiral") {
    // TODO: Larsonspiral
  //   strip.Scanner(strip.Color(255, 0, 0), 40, true, true);
  }
  else if (command == "rainbowcycle") {
    NeoPixelMatrix.infinite_repetitions = true;
    NeoPixelMatrix.RainbowCycle(20, DIRECTION_DOWN);
  }
  else if (command == "theaterchase" || command == "chase") {
    /* TODO: Auf gesetzte Farben reagieren */
    // NeoPixelMatrix.SetColor1(value.toInt());
    NeoPixelMatrix.Stripes(NeoPatterns::Wheel(tColor), 1, NeoPatterns::Wheel(tColor + 0x80), 2, 2 * NeoPixelMatrix.numPixels(),
              tDuration * 2, (tColor & DIRECTION_DOWN));
  }
  else if (command == "fade") {
    // TODO: Instable
    NeoPixelMatrix.infinite_repetitions = true;
    NeoPixelMatrix.Fade(NeoPatterns::Wheel(tColor), NeoPatterns::Wheel(tColor + 0x80), 64, 0);
  //   strip.Fade(strip.Color(255, 0, 0), strip.Color(0, 0, 255), 200, 100);
  }
  else if (command == "randomfade") {
  //   strip.RandomFade();
  }
  else if (command == "random") {
  //   strip.Random();
  }
  else if (command == "smooth") { //example: smooth|[wheelspeed]|[smoothing]|[strength]     wheelspeed=1-255, smoothing=0-100, strength=1-255
  //   strip.Smooth(16, 80, 50, 40);
    NeoPixelMatrix.Smooth(16, 80, 50, 40);
  }
  else if (command == "plasma") {
      NeoPixelMatrix.Plasma();
  }
  else if (command == "plasma4") {
      NeoPixelMatrix.Plasma4();
  }
	else if (command == "heartbeat") {
    NeoPixelMatrix.infinite_repetitions = true;
    NeoPixelMatrix.Heartbeat(NeoPixelMatrix.Color(255,0,0), 20 , 0 , 0);
	}
	else if (command == "heart") {
    NeoPixelMatrix.MovingPicturePGM(heart8x8, COLOR32_RED_HALF, COLOR32_BLACK, 0, -1, 100, 100, DIRECTION_DOWN);
	}
	else if (command == "ball") {
    NeoPixelMatrix.BouncingBall(NeoPatterns::Wheel(tColor), NeoPixelMatrix.numPixels() - 1);
	}
	else if (command == "stripes") {
    // NeoPixelMatrix.Direction = DIRECTION_DOWN;
    NeoPixelMatrix.infinite_repetitions = true;
    NeoPixelMatrix.Stripes(NeoPixelMatrix.Color1, 5, NeoPixelMatrix.LongValue1.Color2, 1, 2 * NeoPixelMatrix.numPixels(), tDuration * 2, (tColor & DIRECTION_DOWN));
	}
	else if (command == "striped") {
    // TODO: Test me!
    NeoPixelMatrix.infinite_repetitions = true;
    NeoPixelMatrix.StripesD(NeoPatterns::Wheel(tColor), 5, NeoPatterns::Wheel(tColor + 0x80), 3, 2 * NeoPixelMatrix.numPixels(),
                tDuration * 2, (tColor & DIRECTION_DOWN));
	}
  else if (command == "stars") {
    // NeoPixelMatrix.initMultipleFallingStars(aLedsPtr, COLOR32_WHITE_HALF, 7, tDuration, 3, &allPatternsRandomHandler);
  }
  else if (command == "gameoflife") {
    switch (validparams) {
        case 0:
          // homieNode.setProperty("gameoflifeinfo").send("GameOfLife with interval " + String(1000) + " and count " + String(32));
          NeoPixelMatrix.GameOfLife(250, 32);
          break;
        case 1:
#ifndef NOWIFI
          homieNode.setProperty("gameoflifeinfo").send("GameOfLife with " + String(param1));
#endif
          NeoPixelMatrix.GameOfLife(250, 32, false, param1);
          break;
    }
  }
  else if (command == "gameoflifec") {
    switch (validparams) {
        case 0:
          NeoPixelMatrix.GameOfLife(250, 32, true);
          break;
        case 1:
#ifndef NOWIFI
          homieNode.setProperty("gameoflifecinfo").send("GameOfLife with " + String(param1));
#endif
          NeoPixelMatrix.GameOfLife(250, 32, true, param1);
          break;
    }
  }
  else if (command == "gameoflifed") {
    switch (validparams) {
        case 0:
          NeoPixelMatrix.GameOfLife(250, 32, false, "", true);
          break;
        case 1:
#ifndef NOWIFI
          homieNode.setProperty("gameoflifedinfo").send("GameOfLifeDim with " + String(param1));
#endif
          NeoPixelMatrix.GameOfLife(250, 32, false, param1, true);
          break;
    }
  }
  else if (command == "snake") {
    NeoPixelMatrix.OnPatternComplete = &SnakeAutorunCompleteHandler;
    initSnakeAutorun(&NeoPixelMatrix, 200, COLOR32_BLUE, 5);
  }
  else if (command == "randomfill")
  { 
    // Fills "slowly" all pixels with a random colour
    // 21 hell, 9 dunkel  (-1, +1)
    // bei   "NEOPIXEL_MATRIX_NUM_ROWS": 20,
    // und   "NEOPIXEL_MATRIX_NUM_COLUMNS": 30,

    NeoPixelMatrix.RandomFill();
  }
  else if (command == "sparkle") {
    // Log some random values to understand what's going wrong
    // char str[256];
    // 20: Ergibt 0-20
    // sprintf(str, "%ld", NeoPixelMatrix.Columns); 
    // homieNode.setProperty("NumberOfCols").send(str);
    // for (int i = 0; i < 100; i++)
    // {
    //   sprintf(str, "%ld", random(0, (long)NeoPixelMatrix.Columns));
    //   homieNode.setProperty("random.Columnsld").send(str);
    //   delay(10);
    // }
    switch (validparams) {
      // uint8_t interval, uint8_t CreationTimeout, uint8_t inc, float dec
      // Beispiele:
      // Rauschen:                  sparkle|1|20000|0.98
      // Schmelzende Schneeflocken: sparkle|400|2|0.995
      // Aufblitzen:                sparkle|1|3|0.1
      // Ruhig:                     sparkle|100|1.05|0.9
      case 0:
#ifndef NOWIFI
        Homie.getLogger() << "Sparkle:" << endl;
#endif
        NeoPixelMatrix.Sparkle();
        break;
      case 1:
#ifndef NOWIFI
        Homie.getLogger() << "Sparkle:" << param1.toInt() << endl;
#endif
        NeoPixelMatrix.Sparkle(param1.toInt() < 50 ? param1.toInt() : 50, param1.toInt());
        break;
      case 2:
#ifndef NOWIFI
        Homie.getLogger() << "Sparkle:" << param1.toInt() << ", " << param2.toInt() << endl;
#endif
        NeoPixelMatrix.Sparkle(param1.toInt() < 50 ? param1.toInt() : 50, param1.toInt(), param2.toFloat() > 1 ? param2.toFloat() : 1.5);
        break;
      case 3:
#ifndef NOWIFI
        Homie.getLogger() << "Sparkle:" << param1 << ", " << param2 << ", " << param3 << endl;
#endif
        NeoPixelMatrix.Sparkle(param1.toInt() < 50 ? param1.toInt() : 50, param1.toInt(), param2.toFloat() > 1 ? param2.toFloat() : 1.5, param3.toFloat() < 1 ? param3.toFloat() : 0.9999);
        break;
    }
  }
  else if (command == "sparklew") {
    switch (validparams) {
      // uint8_t interval, uint8_t CreationTimeout, uint8_t inc, float dec
      // Beispiele:
      // Rauschen:                  sparkle|1|20000|0.98
      // Schmelzende Schneeflocken: sparkle|400|2|0.995
      // Aufblitzen:                sparkle|1|3|0.1
      // Ruhig:                     sparkle|100|1.05|0.9
      case 0:
#ifndef NOWIFI
        Homie.getLogger() << "Sparklew:" << endl;
#endif
        NeoPixelMatrix.Sparklew();
        break;
      case 1:
#ifndef NOWIFI
        Homie.getLogger() << "Sparklew:" << param1.toInt() << endl;
#endif
        NeoPixelMatrix.Sparklew(param1.toInt() < 50 ? param1.toInt() : 50, param1.toInt());
        break;
      case 2:
#ifndef NOWIFI
        Homie.getLogger() << "Sparklew:" << param1.toInt() << ", " << param2.toInt() << endl;
#endif
        NeoPixelMatrix.Sparklew(param1.toInt() < 50 ? param1.toInt() : 50, param1.toInt(), param2.toFloat() > 1 ? param2.toFloat() : 1.5);
        break;
      case 3:
#ifndef NOWIFI
        Homie.getLogger() << "Sparklew:" << param1 << ", " << param2 << ", " << param3 << endl;
#endif
        NeoPixelMatrix.Sparklew(param1.toInt() < 50 ? param1.toInt() : 50, param1.toInt(), param2.toFloat() > 1 ? param2.toFloat() : 1.5, param3.toFloat() < 1 ? param3.toFloat() : 0.9999);
        break;
    }
  }
// #define PATTERN_FIREFLAT        (PATTERN_SNAKE + 6)
// #define PATTERN_FIREWORKS       (PATTERN_SNAKE + 7)
// #define PATTERN_DROP            (PATTERN_SNAKE + 8)
// #define PATTERN_RING            (PATTERN_SNAKE + 9)  
  else if (command == "fireflat") {
    NeoPixelMatrix.FireFlat();
  }
  else if (command == "firework" || command == "fireworks") {
    NeoPixelMatrix.Fireworks();
  }
  else if (command == "fire") {
    NeoPixelMatrix.infinite_repetitions = true;
    NeoPixelMatrix.Fire();
  }
  else if (command == "drop") {
    NeoPixelMatrix.Drop();
  }
  else if (command == "ring" || command == "rings") {
    NeoPixelMatrix.Rings();
  }
  else if (command == "interval") {
    switch (validparams) {
      case 1:
        NeoPixelMatrix.Interval = int(param1.toInt() < 1 ? 0 : param1.toInt());
      break;
    }
  }
  else if (command == "fill") {
    switch (validparams) {
      case 0:
        // NeoPixelMatrix.SetColor1(value.toInt());
        NeoPixelMatrix.Fill(COLOR32_WHITE_32TH);
        // strip.ColorSetParameters("#FFFFFF");
        break;
      case 1:
        if (param1.substring(0,1) == "#") {
          param1 = param1.substring(1);
        }

        uint8_t wp = 0;
        if (param1.length() > 7) {
          // White encoded
          String colorrgb = param1.substring(0,6);
          String whitepart = param1.substring(6,8);
          Serial.println(colorrgb);
          Serial.println(whitepart);
          param1 = colorrgb;
          wp = (int) strtol( &whitepart[0], NULL, 16);
        }
        NeoPixelMatrix.Fill(parseColor("#" + param1), wp);
        break;
    }
  }
  else {
    // no valid command found
    return false;
  }
#ifndef NOWIFI
	homieNode.setProperty("effect").send(value);
#endif
	return true;
}

bool onSetInterval(const HomieRange & range, const String & value) {
  int newInterval = value.toInt();
  if (newInterval > 0) {
    NeoPixelMatrix.SetInterval(newInterval);
  }
#ifndef NOWIFI
  homieNode.setProperty("interval").send(value);
#endif
  return true;
}

#ifndef NOWIFI
// TODO: Bei Streifen anders arbeiten (keine Zeichen anzeigen)
void onHomieEvent(const HomieEvent& hevent) {
  switch (hevent.type) {
    case HomieEventType::CONFIGURATION_MODE:
      // Do whatever you want when configuration mode is started
      break;
    case HomieEventType::NORMAL_MODE:
      // Do whatever you want when normal mode is started
      // if (NeoPixelMatrix.numPixels() > 0) {
      //   NeoPixelMatrix.RainbowCycle(50);
      // }
      break;
    case HomieEventType::ABOUT_TO_RESET:
      // Do whatever you want when the device is about to reset
      break;
    case HomieEventType::WIFI_CONNECTED:
      NeoPixelMatrix.clear();
      if (NeoPixelMatrix.numPixels() > 0) {
        NeoPixelMatrix.setBrightness(lastbrightness);
        stopAfterCompletion = backup_stopAfterCompletion;
      }
      // Do whatever you want when Wi-Fi is connected in normal mode
      break;
    case HomieEventType::WIFI_DISCONNECTED:
      // Do whatever you want when Wi-Fi is disconnected in normal mode
      if (NeoPixelMatrix.numPixels() > 0) {
          lastbrightness = NeoPixelMatrix.getBrightness();
          backup_stopAfterCompletion = stopAfterCompletion;
          NeoPixelMatrix.OnPatternComplete = &StripComplete;
          stopAfterCompletion = false;  // Might interfere with running effects
          NeoPixelMatrix.clear();
          String stext = "wifi?";
          strncpy(lastText, stext.c_str(), (strlen(stext.c_str())>1022) ? 1022 : strlen(stext.c_str()));
          lastText[strlen(stext.c_str())] = 0;
          lasttextcolor = NeoPixelMatrix.Color(255,255,255);
          lastinterval = 100;
          NeoPixelMatrix.Ticker(lastText, lasttextcolor, NeoPixelMatrix.Color(0,0,0), lastinterval);
        }
      break;
    case HomieEventType::MQTT_READY:
      NeoPixelMatrix.clear();
      if (NeoPixelMatrix.numPixels() > 0) {
        NeoPixelMatrix.setBrightness(lastbrightness);
        stopAfterCompletion = backup_stopAfterCompletion;
      }
      // Do whatever you want when MQTT is connected in normal mode
      // if (NeoPixelMatrix.numPixels() > 0) {
      //   NeoPixelMatrix.Sparkle();
      // }
      break;
    case HomieEventType::MQTT_DISCONNECTED:
      // Do whatever you want when MQTT is disconnected in normal mode
      if (NeoPixelMatrix.numPixels() > 0) {
          lastbrightness = NeoPixelMatrix.getBrightness();
          backup_stopAfterCompletion = stopAfterCompletion;
          NeoPixelMatrix.OnPatternComplete = &StripComplete;
          stopAfterCompletion = false;  // Might interfere with running effects
          NeoPixelMatrix.clear();
          String stext = "mqtt?";
          strncpy(lastText, stext.c_str(), (strlen(stext.c_str())>1022) ? 1022 : strlen(stext.c_str()));
          lastText[strlen(stext.c_str())] = 0;
          lasttextcolor = NeoPixelMatrix.Color(255,255,255);
          lastinterval = 100;
          NeoPixelMatrix.Ticker(lastText, lasttextcolor, NeoPixelMatrix.Color(0,0,0), lastinterval);
        }
      break;
    case HomieEventType::STANDALONE_MODE:
      break;
    case HomieEventType::OTA_STARTED:
      break;
    case HomieEventType::OTA_SUCCESSFUL:
      break;
    case HomieEventType::OTA_PROGRESS:
      break;
    case HomieEventType::OTA_FAILED:
      break;
    case HomieEventType::MQTT_PACKET_ACKNOWLEDGED:
      break;
    case HomieEventType::READY_TO_SLEEP:
      break;
  }
}
#endif

void setup() {
	pinMode(LED_BUILTIN, OUTPUT);

	Serial.begin(115200);

#if defined(__AVR_ATmega32U4__)
    while (!Serial); //delay for Leonardo, but this loops forever for Maple Serial
#endif
#if defined(SERIAL_USB)
    //delay(2000); // To be able to connect Serial monitor after reset and before first printout
#endif
  
#ifndef NOWIFI
	Homie_setFirmware("newpixelbox", "1.0.0");
  homieNode.advertise("color0").setName("color").setDatatype("color").setFormat("rgb").settable(onSetColor0);
  homieNode.advertise("color1").setName("color").setDatatype("color").setFormat("rgb").settable(onSetColor1);
  homieNode.advertise("interval").settable(onSetInterval);
	homieNode.advertise("brightness").settable(onSetBrightness);
	homieNode.advertise("effect").settable(onSetEffect);
  homieNode.advertise("reverse").settable(onSetReverse);  
  Homie.onEvent(onHomieEvent);
  Homie.setup();
#else
  Serial.print(F("Booted in NOWifi"));
  // Interface::get().getConfig().get().wifi.ssid
  // homieinterface::get().getLogger() << F(" *Printing via Interface") << endl;
  // HomieInternals::Interface::get().getLogger() << F(" *Printing via Interface") << endl;
  bool isConfigured = HomieInternals::Interface::get().getConfig().load();
  // bool isConfigured = getConfig().load();
  // Serial.print(F("Config read."));
#endif

  MatrixInit();
	// This initializes the NeoPixel library and checks if enough memory was available
	if (!NeoPixelMatrix.begin(&Serial)) {
		// Blink forever
		while (true) {
			digitalWrite(LED_BUILTIN, HIGH);
			delay(45);
			digitalWrite(LED_BUILTIN, LOW);
			delay(45);
		}
	}
  NeoPixelMatrix.clear();

#if defined(__AVR__)
	extern void *__brkval;  
	Serial.print(F("Free Ram/Stack[bytes]="));
	Serial.println(SP - (uint16_t) __brkval);
#endif
#ifndef NOWIFI
  ArduinoOTA.setHostname(Homie.getConfiguration().deviceId);
  ArduinoOTA.onStart([]() {
    NeoPixelMatrix.None();
    NeoPixelMatrix.show();
  });
  ArduinoOTA.onEnd([]() {
    NeoPixelMatrix.fill(NeoPixelMatrix.Color(0,25,0), 0, NeoPixelMatrix.numPixels());
    NeoPixelMatrix.None();
    NeoPixelMatrix.show();
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    NeoPixelMatrix.fillRegion(NeoPixelMatrix.Color(25,0,0), 0, (int)(NeoPixelMatrix.numPixels() * (float)progress / (float)total));
    NeoPixelMatrix.show();
  });
  ArduinoOTA.begin();
#else
  // Start default effect
  // HomieRange hr = HomieRange();
  onSetEffect(HomieRange(), "plasma"); //Default effect
  onSetInterval(HomieRange(), "100");
  lastUpdateCounter = millis();
  Serial.println(lastUpdateCounter);
#endif
}

uint8_t sWheelPosition = 0; // hold the color index for the changing ticker colors
uint32_t heaptick = 0;


void loop() {

#ifndef NOWIFI
  Homie.loop();
  fixWiFi();
  ArduinoOTA.handle();
#else

  if ((millis() - lastUpdateCounter) > updateInterval) {
    effectCounter++;
    Serial.println(effectCounter);
    switch (effectCounter%16) {
      case 0:
        onSetEffect(HomieRange(), "scanner");
        break;
      case 1:
        onSetEffect(HomieRange(), "text|Welcome Pixelbox");
        break;
      case 2:
        onSetEffect(HomieRange(), "plasma");
        break;
      case 3:
        onSetEffect(HomieRange(), "gameoflifec");
        break;
      case 4:
        onSetEffect(HomieRange(), "snow");
        break;
      case 5:
        onSetEffect(HomieRange(), "sparkle");
        break;
      case 6:
        onSetEffect(HomieRange(), "snake");
        break;
      case 7:
        onSetEffect(HomieRange(), "rainbowcycle");
        break;
      case 8:
        onSetEffect(HomieRange(), "theaterchase");
        break;
      case 9:
        onSetInterval(HomieRange(), "500");
        onSetEffect(HomieRange(), "fade"); // Too fast / broken
        break;
      case 10:
        onSetInterval(HomieRange(), "200");
        onSetEffect(HomieRange(), "smooth"); //example: smooth|[wheelspeed]|[smoothing]|[strength] wheelspeed=1-255, smoothing=0-100, strength=1-255
        break;
      case 11:
        onSetEffect(HomieRange(), "heartbeat"); // Looks nicer on a singular strip
        break;
      case 12:
        onSetEffect(HomieRange(), "heart");
        break;
      case 13:
        onSetEffect(HomieRange(), "ball");
        break;
      case 14:
        onSetEffect(HomieRange(), "plasma4");
        break;
      case 15:
        onSetEffect(HomieRange(), "gameoflife"); 
        break;
    }

    
    lastUpdateCounter = millis();
  }

#endif
  // if ((millis() - heaptick) > 1000) {
  //   Homie.getLogger() << ESP.getFreeHeap() << endl;
  //   heaptick = millis();
  // }
	if (NeoPixelMatrix.update()) { 
		// if (NeoPixelMatrix.ActivePattern == PATTERN_TICKER) {
		// 	// change color of ticker after each update
		// 	NeoPixelMatrix.Color1 = NeoPatterns::Wheel(sWheelPosition);
		// 	sWheelPosition += 256 / NEOPIXEL_MATRIX_NUM_PIXELS;
		// }
	}
}
