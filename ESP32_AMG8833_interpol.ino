#include <TFT_eSPI.h>

#include <Adafruit_STMPE610.h>

#include <Adafruit_ILI9341.h>

#include <TFT_eSPI.h>

#include <SparkFun_GridEYE_Arduino_Library.h>

// ESP32_AMG8833_interpol
//
// displays 240*240 full screen interpolated infrared images
// as 24*24 color tiles with tile dimensions 10*10 pixels
// video stream!
//
// image acquisition sensor AMG8833 8*8 pixel i2c infrared thermal imager
// microprocessor board ESP32WROOM-32
// thermal sensor running uner the SparkFun_GridEYE_Arduino_Library
// display is 320*240 ILI9341 running under the TFT_eSPI library
//
// public domain
// Floris Wouterlood
// August 2, 2022
//
// reads temperature values for 64 pixels from AMG8833 sensor
// stores values in pixels[] array
// interpolates
//
// pin wiring
// AMG8833 ---------------  ESP32
// GND  ------------------  GND
// 3V3  ------------------  Vin
// SCL  -- yellow wire ---  D22
// SDA  -- green wire ----  D21

   #include <TFT_eSPI.h>                                            
   TFT_eSPI tft = TFT_eSPI(); 
   
   #include <SparkFun_GridEYE_Arduino_Library.h>
   GridEYE grideye;

// org color lookup table
   const uint16_t camColors[] = {0x480F,                                                       // colors usedin 256 col 16 bit
   0x400F,0x400F,0x400F,0x4010,0x3810,0x3810,0x3810,0x3810,0x3010,0x3010,
   0x3010,0x2810,0x2810,0x2810,0x2810,0x2010,0x2010,0x2010,0x1810,0x1810,
   0x1811,0x1811,0x1011,0x1011,0x1011,0x0811,0x0811,0x0811,0x0011,0x0011,
   0x0011,0x0011,0x0011,0x0031,0x0031,0x0051,0x0072,0x0072,0x0092,0x00B2,
   0x00B2,0x00D2,0x00F2,0x00F2,0x0112,0x0132,0x0152,0x0152,0x0172,0x0192,
   0x0192,0x01B2,0x01D2,0x01F3,0x01F3,0x0213,0x0233,0x0253,0x0253,0x0273,
   0x0293,0x02B3,0x02D3,0x02D3,0x02F3,0x0313,0x0333,0x0333,0x0353,0x0373,
   0x0394,0x03B4,0x03D4,0x03D4,0x03F4,0x0414,0x0434,0x0454,0x0474,0x0474,
   0x0494,0x04B4,0x04D4,0x04F4,0x0514,0x0534,0x0534,0x0554,0x0554,0x0574,
   0x0574,0x0573,0x0573,0x0573,0x0572,0x0572,0x0572,0x0571,0x0591,0x0591,
   0x0590,0x0590,0x058F,0x058F,0x058F,0x058E,0x05AE,0x05AE,0x05AD,0x05AD,
   0x05AD,0x05AC,0x05AC,0x05AB,0x05CB,0x05CB,0x05CA,0x05CA,0x05CA,0x05C9,
   0x05C9,0x05C8,0x05E8,0x05E8,0x05E7,0x05E7,0x05E6,0x05E6,0x05E6,0x05E5,
   0x05E5,0x0604,0x0604,0x0604,0x0603,0x0603,0x0602,0x0602,0x0601,0x0621,
   0x0621,0x0620,0x0620,0x0620,0x0620,0x0E20,0x0E20,0x0E40,0x1640,0x1640,
   0x1E40,0x1E40,0x2640,0x2640,0x2E40,0x2E60,0x3660,0x3660,0x3E60,0x3E60,
   0x3E60,0x4660,0x4660,0x4E60,0x4E80,0x5680,0x5680,0x5E80,0x5E80,0x6680,
   0x6680,0x6E80,0x6EA0,0x76A0,0x76A0,0x7EA0,0x7EA0,0x86A0,0x86A0,0x8EA0,
   0x8EC0,0x96C0,0x96C0,0x9EC0,0x9EC0,0xA6C0,0xAEC0,0xAEC0,0xB6E0,0xB6E0,
   0xBEE0,0xBEE0,0xC6E0,0xC6E0,0xCEE0,0xCEE0,0xD6E0,0xD700,0xDF00,0xDEE0,
   0xDEC0,0xDEA0,0xDE80,0xDE80,0xE660,0xE640,0xE620,0xE600,0xE5E0,0xE5C0,
   0xE5A0,0xE580,0xE560,0xE540,0xE520,0xE500,0xE4E0,0xE4C0,0xE4A0,0xE480,
   0xE460,0xEC40,0xEC20,0xEC00,0xEBE0,0xEBC0,0xEBA0,0xEB80,0xEB60,0xEB40,
   0xEB20,0xEB00,0xEAE0,0xEAC0,0xEAA0,0xEA80,0xEA60,0xEA40,0xF220,0xF200,
   0xF1E0,0xF1C0,0xF1A0,0xF180,0xF160,0xF140,0xF100,0xF0E0,0xF0C0,0xF0A0,
   0xF080,0xF060,0xF040,0xF020,0xF800,};

   #define BLACK   0x0000
   #define BLUE    0x001F
   #define RED     0xF800
   #define GREEN   0x07E0
   #define CYAN    0x07FF
   #define MAGENTA 0xF81F
   #define YELLOW  0xFFE0
   #define WHITE   0xFFFF

   float pixels[64];
   byte pixelIndex = 0;
   float temp_display;
   float temp_01;
   float temp_01_old;
   float fps;
   int iteration;

   #define MINTEMP 22                                                                    // low range of the sensor (this will be blue on the screen)
   #define MAXTEMP 28                                                                    // high range of the sensor (this will be red on the screen)

   int displayPixelHeight  = 25;                                                         // determines size of on screen blocks 
   int displayPixelWidth   = 25;
   int displayHeightOffset = 20;
   int displayWidthOffset  = 40;
   int i,j;

   #define AMG_COLS 8
   #define AMG_ROWS 8
   #define INTERPOLATED_COLS 24
   #define INTERPOLATED_ROWS 24

   float get_point (float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
   void set_point (float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y, float f);
   void get_adjacents_1d (float *src, float *dest, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
   void get_adjacents_2d (float *src, float *dest, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
   float cubicInterpolate (float p[], float x);
   float bicubicInterpolate (float p[], float x, float y);
   void interpolate_image (float *src, uint8_t src_rows, uint8_t src_cols, float *dest, uint8_t dest_rows, uint8_t dest_cols);
   float dest_2d[576];

   float iterationTime;
   long  iterationTotal;
   unsigned long previousMillis;
   unsigned long currentMillis;
   unsigned long t;

void setup() {

   Serial.begin (9600);
   Serial.println ();
   tft.begin ();                             
   tft.setSwapBytes (true);                                                              // swap the byte order for pushImage() - corrects endianness
   tft.setRotation (2);
   tft.fillScreen (BLACK);
   tft.setTextSize (1);  
   tft.setCursor (10,10);  
   tft.print ("AMG8833 GridEYE infrared sensor video");        
   tft.drawRoundRect ( 140, 280,  100,  40, 4, GREEN);    
   tft.fillRect ( 165, 275, 50,  10, BLACK); 
   tft.setCursor (170, 275);  
   tft.print ("cycles"); 
   tft.setCursor (220, 300); 
   tft.print ("Hz");      
   
   Serial.println ();
   Serial.println ("ESP32 and Sparkfun GridEye and AMG8833 thermal 8*8");
   delay (500);
   Wire.begin();
   delay (250); 
   grideye.begin();
   delay (250); 
}

void loop (){

   t = millis();
   for (i=0; i < 64; i++) 
      {
      temp_01 = grideye.getPixelTemperature(i);  
      pixels[i] = temp_01;  
      } 
                
   interpolate_image(pixels, 8, 8, dest_2d, 24, 24);                                     // sensor rows-columns - color tile dimensions
   drawpixels(dest_2d, 24, 24, 10, 10, false);

   iteration++;

   if (iteration > 100)
     { 
      iterationTotal=iterationTotal+iteration-1;
      currentMillis = t;
      iterationTime = currentMillis-previousMillis;
      fps = (iteration / (iterationTime/1000));
      previousMillis = millis ();

  
      Serial.println ("going out of while loop");
      Serial.print ("after cycle nr. "); 
      Serial.println (iterationTotal);
      Serial.print ("time for 100 cycles: ");
      Serial.print (iterationTime);
      Serial.println (" ms");
      Serial.print ("display refresh rate: ");
      Serial.print (fps,1);
      Serial.println (" Hz");
      Serial.println ("");

      tft.fillRect (160,295,55,14,BLACK);
      tft.setTextSize (2);
      tft.setCursor (165,295);   
      tft.print (fps,1);
      
      iteration = 0;
      iterationTime = 0;  
     }   
}                                         
           

void drawpixels (float *p, uint8_t rows, uint8_t cols, uint8_t boxWidth, uint8_t boxHeight, boolean showVal) {

   long colorTemp;
   for (int y=0; y<24; y++)
       {
       for (int x=0; x<24; x++)
          {
          float val = get_point(p, rows, cols, x, y);
          if (val >= MAXTEMP) colorTemp = MAXTEMP;
          else if (val <= MINTEMP) colorTemp = MINTEMP;
          else colorTemp = val;
      
          uint8_t colorIndex = map (colorTemp, MINTEMP, MAXTEMP, 0, 255);
          colorIndex = constrain (colorIndex, 0, 255);

          uint16_t color;                                                                // draw yer pixels!
          color = val * 2;
          tft.fillRect(10* x, 25+10* y, 10,10, camColors[colorIndex]);      
          }
        }
 } 
