#include <FastLED.h>

#define NUM_LEDS 5
#define DATA_PIN 3
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB 

CRGB leds[NUM_LEDS];

//REMOBE?
const int NumLEDsToAdjust = NUM_LEDS * .20; /// number of leds to be adjusted every periodic sections? or mayeb REMOVE THIS?

// color to fade up to is changed every so often
int colorVarianceRange = 15; // how far from the baseColor should the random fluctuations deviate
int brightnessVarianceRange = 150; // how far from the base brightness should the random fluctuations deviate

// Base color values
int hue = 25;
int sat = 255;  
int val = 255 - brightnessVarianceRange; // prevent this plus random fluctuation form going over 255 and becomming very small

CHSV baseColorHSV(hue, sat, val);
CHSV adjColorHSV;
CHSV currentColorHSV = baseColorHSV;

int LED; // currently selected LED for variations // declare inside smaller scope?
int selectedLEDs[NumLEDsToAdjust] = {}; // array of LEDs to be set to the current color (blended result) MAKE SURE TO RESET THIS EVERY x MS?
bool fullyFadedAllLEDs = false; // set to true when all selected LEDs are fully faded to the target/adjusted color, when true new selectedLEDs can be chosen and this will become false 
bool fadeUp = true; // fade from base to adjusted - becomes true when current == base
bool fadeDown = false; // fade from adjusted to base - becomes true when current == adjusted

int hueVariance;
int brightnessVariance;

unsigned long currentMillis = millis();

unsigned long variationInterval = 5000;
unsigned long prevVariationInt;

int count = 1;

void setup() {
  Serial.begin(9600);
  Serial.print("Hue Base: ");
  Serial.println(baseColorHSV.h);
  Serial.print("NumLEDsToAdjust");
  Serial.println(NumLEDsToAdjust);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed


  for (int i = 0; i < NUM_LEDS; i++){
    // Serial.println(i);
    leds[i] = baseColorHSV;
    FastLED.show();
    delay(10);
  }

  randomSeed(analogRead(A0));

}

void loop() {
  currentMillis = millis();
  
//SHOULD BE HANDLED BY THE NOT STATEMENTS IN blendToward func
  // if (currentColorHSV == baseColorHSV){
  //   fadeUp = true;
  //   fadeDown = false;
  // }
  // else if (currentColorHSV == adjColorHSV){
  //   fadeUp = false;
  //   fadeDown = true;
  // }

  // Serial.println("DOGGO");
  if (currentMillis - prevVariationInt > variationInterval || count == 1 ){ // not giving enough time fro them to blend?
    prevVariationInt = currentMillis;
    if (fullyFadedAllLEDs || count == 1){

      currentColorHSV = baseColorHSV; // reset current color to prevent it just jumping straight to the adjusted/varied color and skipping the blending for the newly to be selected LEDs

      for (int j = 0; j < NumLEDsToAdjust; j++){ // pick new LEDs to adjust
        LED = random(0, NUM_LEDS - 1);
        selectedLEDs[j] = LED; // add them to the array (well set the array values to this new LED) - dont need to reset the array cause length is constant

        Serial.print("LED: ");
        Serial.println(LED);
      }
      fullyFadedAllLEDs = false;
    }
    
    hueVariance = random(-colorVarianceRange, colorVarianceRange);  // how much should this LED vary from the base color
    brightnessVariance = random(-brightnessVarianceRange, brightnessVarianceRange); // how much should this LED vary from the base brightness (val)
    
    // adjColorHSV = CHSV(baseColorHSV.h + hueVariance, sat, baseColorHSV.v + brightnessVariance);
    adjColorHSV = CHSV(100, 255, 50);

    Serial.print("Hue Variance: ");
    Serial.print(hueVariance);
    Serial.print(" Adj Hue: ");
    Serial.println(adjColorHSV.h);
  }

  // blend up - if currentcolor is same as base color blend up to adjusted color
  if (fadeUp){
    currentColorHSV = blendTowards(currentColorHSV, adjColorHSV, 5); // blend up to adj
  }

  // blend down - if currentcolor is same as adjusted color blend down to base color
  else if (fadeDown){
    currentColorHSV = blendTowards(currentColorHSV, baseColorHSV, 5);
  }

  for (int i = 0; i < NumLEDsToAdjust; i++){/////wait wiait iwait i this is wrong
    // Serial.println(currentColorHSV.h);
    leds[selectedLEDs[i]] = currentColorHSV; // set the strip at value stored at index i in selectedLEDs array
  }
  FastLED.show();
  count++;
}

CHSV blendTowards(CHSV currentColor, const CHSV targetColor, int incAmount){ // I NEED TO MAKE SURE IT IS ONLY SETTING THE SAME LEDS UNTIL THEY HAVE MADE IT UP TO THE FULL ADJ LEVEL - DONT FADE DIFFERENT LEDS EVERY TIME IT CHANGES
  // Serial.print("tcH");
  // Serial.println(targetColor.h);
  if( currentColor.h == targetColor.h && currentColor.v == targetColor.v) {
    Serial.println("both EQUAL"); 

    if (fadeDown){ // if we have been going back to base and now the LEDs are at the base we are done and they are fully faded
      fullyFadedAllLEDs = true;
    }

    fadeUp = not fadeUp;
    fadeDown = not fadeDown; // should come after setting fullyFadedAllLEDs cause this is setting fadeDown for the next cycle

    return currentColor; // leave and do no more
  }  
  if( currentColor.h < targetColor.h ) {
    Serial.print(currentColor.h);
    Serial.print("LESS hue");
    Serial.println(targetColor.h);

    int delta = targetColor.h - currentColor.h;
    delta = scale8_video( delta, incAmount);
    currentColor.h += delta;
    
    Serial.print(currentColorHSV.h);
    Serial.println("NEW hue");
    
  } 
  else if ( currentColor.h > targetColor.h) {
    Serial.print(currentColor.h);
    Serial.print("GREATER hue");
    Serial.println(targetColor.h);

    int delta = currentColor.h - targetColor.h;
    delta = scale8_video( delta, incAmount);
    currentColor.h -= delta;
    
    Serial.print(currentColorHSV.h);
    Serial.println("NEW hue");

  }
  else {Serial.println("its done ig hue");}

  if( currentColor.v < targetColor.v ) {
    Serial.print(currentColor.v);
    Serial.print("LESS val");
    Serial.println(targetColor.v);

    int delta = targetColor.v - currentColor.v;
    delta = scale8_video( delta, incAmount);
    currentColor.v += delta;
    
    Serial.print(currentColorHSV.v);
    Serial.println("NEW val");

    
  } 
  else if ( currentColor.v > targetColor.v) {
    Serial.print(currentColor.v);
    Serial.print("GREATER val");
    Serial.println(targetColor.v);

    int delta = currentColor.v - targetColor.v;
    delta = scale8_video( delta, incAmount);
    currentColor.v -= delta;
    
    Serial.print(currentColorHSV.v);
    Serial.println("NEW val");

  }
  else {Serial.println("its done ig val");}

  return currentColor;
}

// void fadeTowardHSV(CRGB* leds, int NUM_LEDS, const CHSV& baseColor, int incAmount){ 
//   for (int i = 0; i < NUM_LEDS; i++){
//     CHSV& current = leds[i]; /// does this make sense? get the current value from the LEDs?
//     blendTowards(current.h, baseColor.h, incAmount );
//   }
// }




/*

// Thank you Mark Kriegsman! might not use them tho i want hsv so ill be modifying your code

// Helper function that blends one uint8_t toward another by a given amount
void nblendU8TowardU8( uint8_t& cur, const uint8_t target, uint8_t amount){ // magic ig
  if( cur == target) return;
  
  if( cur < target ) {
    uint8_t delta = target - cur;
    delta = scale8_video( delta, amount);
    cur += delta;
  } else {
    uint8_t delta = cur - target;
    delta = scale8_video( delta, amount);
    cur -= delta;
  }
}

// Blend one CRGB color toward another CRGB color by a given amount.
// Blending is linear, and done in the RGB color space.
// This function modifies 'cur' in place.
CRGB fadeTowardColor( CRGB& cur, const CRGB& target, uint8_t amount){ // some background magic code

  nblendU8TowardU8( cur.red,   target.red,   amount);
  nblendU8TowardU8( cur.green, target.green, amount);
  nblendU8TowardU8( cur.blue,  target.blue,  amount);
  return cur;
}

// Fade an entire array of CRGBs toward a given background color by a given amount
// This function modifies the pixel array in place.
*/
// void fadeTowardColor( CRGB* L, uint16_t N, const CRGB& bgColor, uint8_t fadeAmount){ // whole strip - gets adjusted LEDs back to base (fade DOWN)

//   for( uint16_t i = 0; i < N; i++) {
//     fadeTowardColor( L[i], bgColor, fadeAmount);
//   }
// }
// void fadeTowardColorSingle( CRGB* L, int LED, uint16_t N, const CRGB& bgColor, uint8_t fadeAmount){ // single LED - adjusts LEDs to varied states (fade UP)

//   fadeTowardColor( L[LED], bgColor, fadeAmount);
  
// }
