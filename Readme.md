Pixelbox Firmware
====================
## About

# PixelBox
Shows effects on different kind of models created with WS2812b-Stripes. The models can be simple stripes oder any rectangular shape. The configuration is done via a config.json and can be controlled via MQTT.

Shows effects on different kind of models created with WS2812b-Stripes. The models can be simple stripes oder any rectangular shape. The configuration is done via a config.json and can be controlled via MQTT. 

## Configuration
Place the configured config.json in the subdirectory "data/homie" and upload it to the device.

Sample config.json:
```json
{
"name": "nameofdevice",
"device_id": "idofdevice",
"wifi": {
"ssid": "YOURSSID",
"password": "YOURWIFIPASS"
},
"mqtt": {
"host": "your.mqtt.server",
"port": 1883,
"auth": false
},
"ota": {
"enabled": true
},
"settings": {
    "NEOPIXEL_MATRIX_NUM_ROWS": 8,
    "NEOPIXEL_MATRIX_NUM_COLUMNS": 8,
    "PIN_NEOPIXEL_MATRIX": "D2",
    "NEO_MATRIX_CONFIGURATION": "NEO_MATRIX_BOTTOM | NEO_MATRIX_RIGHT | NEO_MATRIX_ROWS | NEO_MATRIX_ZIGZAG",
    "NEO_PERMUTATION": "NEO_GRB",
    "NEO_SPEED": "NEO_KHZ800"
  }
}
```

The "settings"-part is to be configured like the Adafruit Neomatrix-Library.

PIN_NEOPIXEL_MATRIX: Which PIN the pixel data port is connected to. If you're using the ESP8266, remember NOT to use D4, as this PIN is also connected to the blue LED of the chip.

NEO_MATRIX_CONFIGURATION: The first pixel must be at one of the four corners; which corner is indicated by OR'ing either NEO_MATRIX_TOP or NEO_MATRIX_BOTTOM to either NEO_MATRIX_LEFT or NEO_MATRIX_RIGHT. The row/column arrangement is indicated by further OR'ing either NEO_MATRIX_COLUMNS or NEO_MATRIX_ROWS to either NEO_MATRIX_PROGRESSIVE or NEO_MATRIX_ZIGZAG. 

NEO_PERMUTATION: Indicate the type of LED pixels being used.

## Features
### LED 8x8 Field
|Topic    |Descriptions  |settable   |Values   |
|---------|--------------|:---------:|---------|
|homie/`device_id`/pixel/color|Range property to set the effects colors 1 & 2 see: [effect colors](#effect-colors)|yes|Color as uint32_t see [Color](#color)|
|homie/`device_id`/pixel/brightness|Sets the brightness of the pixel strip|yes|possible values: 0 - 255|
|homie/`device_id`/pixel/effect|Set effect|yes|see: [effects](#effects)|

## Effects
### text
If the attached strips construct a rectangle, you can send scrolling text to it. The optional parameter defines the speed (interval between updates in ms) and the color.

Example: `mosquitto_pub -h HOST -t "homie/device_id/pixel/effect/set" -m "text|Hello world|100|#FF0000"`

### scanner
Shows the moving larson scanner eye known form *Battlestar Galactica* and *Knight Rider*. <!-- The used effect color can be specified by *color_0* -->

### rocket
Shows the moving larson scanner with only a trail (not an eye like `scanner`). <!-- The used effect color can be specified by *color_0* -->

### fire
If on a rectangle, shows a fireplace style fire<!--, otherwise lets all LEDs flicker somehow firelike -->.

### snow
If on a rectangle, lets snow slowly fall down from above. It melts somehow if it reaches the bottom.
<!--* **larsonspiral**
This is the same scanner then the randomscanner above but uses a spiral pattern -->
<!-- * **randomscanner**
Shows the moving larson scanner eye known form *Battlestar Galactica* and *Knight Rider*. Uses some randomness to change directions and cycles through colours. -->

### rainbowcycle
Shows a cycling rainbow on the LED strip

### theaterchase
Shows an color chasing LED strip.
<!-- You can specify the color by set *color_0* and *color_1* -->

### fade
Fades from effect color_0 to effect_color_1

<!--
### randomfade
Fades thru an alternating color pattern on all pixels with the same color.
### randomfade|40
Fades thru an alternating color pattern on all pixels with different color.
### random
Shows random static colors on all pixels differently.
-->

### smooth
Smooth changing effect.

### plasma
The all famous plasma effect known from many 8bit-Demos from the '80s and '90s.

### plasma4
The all famous plasma effect known from many 8bit-Demos from the '80s and '90s. A bit different with four calculated circles.

### heartbeat
Flashes the strip in red "a bit" like a heartbeat

### heart
On a 8x8 pixel matrix, shows a heart moving from top to bottom

### ball
Lets a ball "jump" multiple times, decreasing height. Works best on a vertical mounted strip.

### stripes
Like the `theaterchase`, but using bigger stripes following each other

<!--
### striped
?? Not working ?? -->
<!--
### stars
Multiple falling stars. Not fully implemented yet. -->
### gameoflife
Conway's game of life, only really works on a matrix

### gameoflifec
Conway's game of life like above, but colors new cells in green, dying cells in red.

### gameoflifed
Conway's game of life like above, but dims new cells (red) slowly to white if they live longer.

### snake
Auto-Play a game of snake, only really works on a matrix.

### sparkle
Randomly lets sparkles appear, looks nice on any kind of attached strip. Multiple optional parameters can be given:
interval in ms, CreationTimeout in ms, Increment as Integer, Decrement as Float

Examples:

#### static noise

`mosquitto_pub -h HOST -t "homie/device_id/pixel/effect/set" -m "sparke|1|20000|0.98"`

#### melting snowflakes

`mosquitto_pub -h HOST -t "homie/device_id/pixel/effect/set" -m "sparke|400|2|0.995"`

#### disco flash

`mosquitto_pub -h HOST -t "homie/device_id/pixel/effect/set" -m "sparke|1|3|0.1"`

#### calm

`mosquitto_pub -h HOST -t "homie/device_id/pixel/effect/set" -m "sparkle|100|1.05|0.9"`

### interval

<!-- TODO: Publish as a non-effect, but as a parameter like "brightness" -->
Changes the interval with the given parameter
`mosquitto_pub -h HOST -t "homie/device_id/pixel/effect/set" -m "100"`

### none
Stop all effects

## Effect colors
You can set to different effect colors
* *color_0* (default R: 255, G: 0  B: 0)
* *color_1* (default R: 0, G: 0  B: 255)

The effect color has to be set after the effect. <!-- Is this still true? -->
###### Example:
1. `homie/device_id/pixel/effect -m "fade"`
2. `homie/device_id/strip/color_0/set -m "255"`
3. `homie/device_id/strip/color_1/set -m "10216992"`

##### color_0
This color will be used for the following effects:
* *scanner*
* *theaterchase*
* *fade*

##### color_1
This color will be used for the following effects:
* *theaterchase*
* *fade*

