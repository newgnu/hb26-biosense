#include <Wire.h>

#include "ssd1306.h"
#include "i2c/ssd1306_i2c_wire.h"
#include "i2c/ssd1306_i2c_embedded.h"
#include "intf/ssd1306_interface.h"
#include "nano_gfx.h"


//  I/O PINS
#define BTN_1     2
#define BTN_2     3
#define BTN_3     4
#define BTN_4     5
#define BUZZER    12

const PROGMEM uint8_t heartImage[8] =
{
    B00001110,
    B00011111,
    B00111111,
    B01111110,
    B01111110,
    B00111101,
    B00011001,
    B00001110
};
const PROGMEM uint8_t nullImage[8] =
{
    B00001110,
    B00010001,
    B00100001,
    B01000010,
    B01000010,
    B00100001,
    B00010001,
    B00001110
};

void setup() {
  Serial.begin(115200);

  Wire.begin();
  #ifdef SSD1306_WIRE_CLOCK_CONFIGURABLE
    Wire.setClock( 400000 );
  #endif

  pinMode(BTN_1, INPUT_PULLUP);
  pinMode(BTN_2, INPUT_PULLUP);
  pinMode(BTN_3, INPUT_PULLUP);
  pinMode(BTN_4, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  
}

void loop() {
  ssd1306_i2cInit_Wire(0);
  ssd1306_128x64_init();
  ssd1306_clearScreen( );
  ssd1306_charF6x8(10, 2, "1. EMG");
  ssd1306_charF6x8(10, 3, "2. ECG/EEG");
  ssd1306_charF6x8(10, 4, "3. PULSE SENSOR");

  while(1){
    if (digitalRead(BTN_1) == LOW){
      bio_loop("EMG",2); //Input on Pin A2
    }
    if (digitalRead(BTN_2) == LOW){
      bio_loop("ECG/EEG", 1); //Input on Pin A1
    }
    if (digitalRead(BTN_3) == LOW){
      bio_loop("PULSE SENSOR",0);  //Input on Pin A0
    }
  }
}

void bio_loop(char *sigtype, int InputPin){
  int biosignal;
  unsigned long biosignal_ms;
  int prev_biosignal;
  unsigned long prev_biosignal_ms;
  int avg;
  int bpm;
  char str[50];

  int scale = 1024;
  
  uint8_t chart_buffer[64*32/8];
  NanoCanvas chart(64, 32, chart_buffer);
  
  ssd1306_clearScreen( );
  ssd1306_charF6x8(1, 0, sigtype);

  while(1){
    biosignal = analogRead(InputPin);
//    Serial.println(biosignal);

    if (biosignal - prev_biosignal < -100){
      ssd1306_drawBitmap(64,0,8,8, heartImage);
      biosignal_ms = millis();
      
      bpm = 60000/(biosignal_ms - prev_biosignal_ms);
      sprintf(str, "BPM %3d ", bpm);
      ssd1306_charF6x8(64+8, 2, str);
      
      avg = (avg + bpm) /2;
      sprintf(str, "AVG %3d ", avg);
      ssd1306_charF6x8(64+8, 3, str);

      prev_biosignal_ms = biosignal_ms;
    }else{
      ssd1306_drawBitmap(64,0,8,8, nullImage);
    }
    
    
    chart.drawVLine(chart.width()-1, chart.height() - ((biosignal * chart.height()) / scale), chart.height());
    chart.blt(0,1);
    
    for(int i = 0; i < sizeof(chart_buffer); i++){ //shift everything left
      chart_buffer[i] = chart_buffer[i+1];
    }
    chart.invert();                                      // clear last line -- this is necessary because 1) the left 
    chart.drawVLine(chart.width()-1, 0, chart.height()); // shift above wraps the leftmost byte of each line up by 
    chart.invert();                                      // one, and 2) drawing a line does not clear what was under it.
    
    
    
    delay(5);
    prev_biosignal = biosignal; //save for next loop
  }
}



