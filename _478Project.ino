#include <Adafruit_NeoPixel.h>

#define PIN 6
#define led 4
#define kd 0
#define kp .1
#define ki .01
#define desired 325
// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);

int PID_set(int adc_input);
void colorWipe(uint32_t c, uint8_t wait);
void theaterChase(uint32_t c, uint8_t wait);
void setup() {
  // put your setup code here, to run once:
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  Serial.begin(9600);
  
}
int val;
int PIDval=100;
void loop() {
 
  // Some example procedures showing how to display to the pixels: 
  val = analogRead(A0);
  PIDval = PIDval+PID_set(val);
  if(PIDval<0) PIDval=0;
  else if(PIDval>255) PIDval=255;
  Serial.print("LED");
  Serial.print(PIDval);
  Serial.print('\n');
  Serial.print("input");
  Serial.print(val);
  Serial.print('\n');
  //delay(50);
  //theaterChase(strip.Color(PIDval, 0,0), 100);
  //delay(50);
  colorWipe(strip.Color(PIDval, PIDval, PIDval), 0); // Red
 // colorWipe(strip.Color(PIDval,100,100), 0);
}
int PID_set(int adc_input){

  static float error[5] = {0,0,0,0,0};
    static int16_t input =0;
    uint8_t result;
    error[0]=(desired-adc_input);
    error[1]=((error[0]-error[3])/.000000125);
    error[2]=(error[0]+(error[4]*.000000125));
    input = (int)(error[0]*kp+error[1]*kd+error[2]*ki);
    Serial.print("error");
    Serial.print(input);
    Serial.print('\n');
    
    error[3]=error[0];
    error[4]=error[2];

    return input;
}

























// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        strip.show();

        delay(wait);

        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, 0);        //turn every third pixel off
        }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }

}
