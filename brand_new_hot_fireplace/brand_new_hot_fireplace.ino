// Thanks to Mark Kriegsman for the logic of this program - https://gist.github.com/kriegsman/d0a5ed3c8f38c64adcb4837dafb6e690

#include <FastLED.h>

#define NUM_LEDS 50
#define DATA_PIN 3
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB 

CRGB leds[NUM_LEDS];

//REMOBE?
const int NumLEDsToAdjust = NUM_LEDS * .20; /// number of leds to be adjusted every periodic sections? or mayeb REMOVE THIS?

// variance from base range {TRUE}
int colorVarianceRange = 15; // how far from the base hue should the random fluctuations deviate
int brightnessVarianceRange = 100; // how far from the base value should the random fluctuations deviate

// OR //    custom bounds for adjusted color {FALSE}
// range of hues for adjusted color
int hueMin = 0;
int hueMax = 20;
// range of values for adjusted color
int valMin = 10; 
int valMax = 200;

bool useVarianceRange = false; // set true to use ranges and set false to use custom bounds for hue and value variations - see above

// Base color values
int hue = 10;
int sat = 255;  
int val = 150 - brightnessVarianceRange; // prevent this plus random fluctuation form going over 255 and becomming very small and it makes the fading jumpy it seems

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
int delayMS = 50;

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

  if ((currentMillis - prevVariationInt > variationInterval || count == 1 ) && fullyFadedAllLEDs){ // not giving enough time fro them to blend?
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

     if (useVarianceRange){ // compile time choice
      hueVariance = random(-colorVarianceRange, colorVarianceRange);  // how much should this LED vary from the base color
      brightnessVariance = random(-brightnessVarianceRange, brightnessVarianceRange); // how much should this LED vary from the base brightness (val)
    }
    else if (not useVarianceRange){ // compile time choice
      hueVariance = random(hueMin, hueMax);  // how much should this LED vary from the base color
      brightnessVariance = random(valMin, valMax); // how much should this LED vary from the base brightness (val)
    }


    if (baseColorHSV.h + hueVariance < 0){hueVariance = 0;} // prevents funky colors if hue becomes negative

    adjColorHSV = CHSV(baseColorHSV.h + hueVariance, sat, baseColorHSV.v + brightnessVariance); // create the target color to fade up to
    // adjColorHSV = CHSV(100, 255, 50); // useful alternate color for debugging and testing

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
  delay(delayMS);
}

CHSV blendTowards(CHSV currentColor, const CHSV targetColor, int incAmount){ 
  // Serial.print("tcH");
  // Serial.println(targetColor.h);
  if( currentColor.h == targetColor.h && currentColor.v == targetColor.v) {
    // Serial.println("both EQUAL"); 

    if (fadeDown){ // if we have been going back to base and now the LEDs are at the base we are done and they are fully faded
      fullyFadedAllLEDs = true;
    }

    fadeUp = not fadeUp;
    fadeDown = not fadeDown; // should come after setting fullyFadedAllLEDs cause this is setting fadeDown for the next cycle

    return currentColor; // leave and do no more
  }  
  if( currentColor.h < targetColor.h ) {
    // Serial.print(currentColor.h);
    // Serial.print("LESS hue");
    // Serial.println(targetColor.h);

    int delta = targetColor.h - currentColor.h;
    delta = scale8_video( delta, incAmount);
    currentColor.h += delta;
    
    // Serial.print(currentColorHSV.h);
    // Serial.println("NEW hue");
    
  } 
  else if ( currentColor.h > targetColor.h) {
    // Serial.print(currentColor.h);
    // Serial.print("GREATER hue");
    // Serial.println(targetColor.h);

    int delta = currentColor.h - targetColor.h;
    delta = scale8_video( delta, incAmount);
    currentColor.h -= delta;
    
    // Serial.print(currentColorHSV.h);
    // Serial.println("NEW hue");

  }
  // else {Serial.println("its done ig hue");}

  if( currentColor.v < targetColor.v ) {
    // Serial.print(currentColor.v);
    // Serial.print("LESS val");
    // Serial.println(targetColor.v);

    int delta = targetColor.v - currentColor.v;
    delta = scale8_video( delta, incAmount);
    currentColor.v += delta;
    
    // Serial.print(currentColorHSV.v);
    // Serial.println("NEW val");

    
  } 
  else if ( currentColor.v > targetColor.v) {
    // Serial.print(currentColor.v);
    // Serial.print("GREATER val");
    // Serial.println(targetColor.v);

    int delta = currentColor.v - targetColor.v;
    delta = scale8_video( delta, incAmount);
    currentColor.v -= delta;
    
    // Serial.print(currentColorHSV.v);
    // Serial.println("NEW val");

  }
  // else {Serial.println("its done ig val");}

  return currentColor;
}

