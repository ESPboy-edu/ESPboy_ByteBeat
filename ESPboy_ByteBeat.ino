/*
ESPboy ByteBeat demo
for www.ESPboy.com project
RomanS 13.05.2022

Some info about ByteBeat one string code music
http://canonical.org/~kragen/bytebeat/

ByteBeat beginners guide
https://github.com/TuesdayNightMachines/Bytebeats

ByteBeat player
http://langtons.atspace.com/audio/waveform.html
*/


#include "lib/ESPboyInit.h"
#include "lib/ESPboyInit.cpp"
#include<sigma_delta.h>

#define SAMPLE_RATE 8000

ESPboyInit myESPboy;
volatile uint8_t functionNo=0;
String musicFormulas[]=
 {
  "((t*(t>>8|t>>18)&46&t>>3))^(t&t>>8|t>>20)",
  "((t<<1)^((t<<1)+(t>>7)&t>>12))|t>>(4-(1^7&(t>>19)))|t>>7",
  "((t*((t>>8)|(t>>18))&33&t>>3))^((t&t>>8)|(t>>20))",
  "(((t>>7)|((t&t)>1))|(t&t>>8)|(t>>7))|(t+(t>>8))|((t>>7))",
  "((t>>6)^(t&0x25))|((t+(t^(t>>11))-t* ((t%24?2:6)&t>>11))^(t<<1&(t&0x256?t>>4:t>>10)))"
 };


void IRAM_ATTR byteBeatStepISR(){   
 static volatile uint32_t t;
 static uint8_t beatByte;
 t++;
 switch (functionNo){
   case 0: 
     beatByte = ((t*(t>>8|t>>18)&46&t>>3))^(t&t>>8|t>>20);
     break;
   case 1:
     beatByte = ((t<<1)^((t<<1)+(t>>7)&t>>12))|t>>(4-(1^7&(t>>19)))|t>>7;
     break;
   case 2:
     beatByte = ((t*((t>>8)|(t>>18))&33&t>>3))^((t&t>>8)|(t>>20));
     break;
   case 3:
     beatByte = (((t>>7)|((t&t)>1))|(t&t>>8)|(t>>7))|(t+(t>>8))|((t>>7));
     break;
   case 4:
     beatByte = ((t>>6)^(t&0x25))|((t+(t^(t>>11))-t* ((t%24?2:6)&t>>11))^(t<<1&(t&0x256?t>>4:t>>10)));
     break;
 }
 sigmaDeltaWrite(0, beatByte);
}


void startByteBeat(){
  noInterrupts();
  sigmaDeltaSetup(0, SAMPLE_RATE*8);
  sigmaDeltaAttachPin(SOUNDPIN);
  sigmaDeltaEnable();
  timer1_attachInterrupt(byteBeatStepISR);
  timer1_enable(TIM_DIV1, TIM_EDGE, TIM_LOOP);
  timer1_write(80000000 / SAMPLE_RATE * 2);
  interrupts(); 
};


void stopByteBeat(){
  noInterrupts();
  timer1_disable();
  sigmaDeltaDisable();
  interrupts();
}


void drawInfo(){
 String toPrint;
  myESPboy.tft.fillScreen(TFT_BLACK);
  myESPboy.tft.setTextSize(3);
  myESPboy.tft.setTextColor(TFT_YELLOW);
  toPrint = "Melody";
  myESPboy.tft.drawString (toPrint, (128-(toPrint.length()*18))/2, 0);
  toPrint = (String)functionNo;
  myESPboy.tft.drawString (toPrint, (128-(toPrint.length()*18))/2, 30);
  myESPboy.tft.setTextSize(1);
  toPrint = musicFormulas[functionNo];
  myESPboy.tft.setTextColor(TFT_GREEN);
  myESPboy.tft.setCursor(0, 60);
  myESPboy.tft.setTextWrap(true, false); 
  myESPboy.tft.print (toPrint);
};


void setup() {
  Serial.begin(115200);
  myESPboy.begin("ByteBeat demo");
  startByteBeat();
  drawInfo();
}


void loop() {
 uint8_t keys=myESPboy.getKeys();
 if (keys){
   if(keys&PAD_DOWN && functionNo > 0) functionNo--;
   if(keys&PAD_UP && functionNo < 4) functionNo++;
   drawInfo();
 }
 delay(200);
}
