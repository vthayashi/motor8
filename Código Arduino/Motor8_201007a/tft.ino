
//  TFT   CS    5
//  TFT   RST   RST 
//  TFT   DC    17
//  TFT   MOSI  23  D7
//  TFT   SCK   18  D5
//  Touch CS    26  D0

#ifdef tft_cs_pin

#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
TFT_eSPI tft = TFT_eSPI();       // Invoke custom library
uint8_t tft_mode=0,prev_tft_mode=0;

void tft_setup(){
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  uint16_t top, left;
  //*/
  tft.setTextColor(TFT_YELLOW);
  top=90; left=160; tft.drawCentreString(DeviceID, left, top, 4);
  String sa; sa="Versao: "; sa+=Versao;
  top=130; left=160; tft.drawCentreString(sa, left, top, 2);
  //
}

void tft_loop1000(){
  if(millis_cnt%1000!=3) return;
  if(millis_cnt==3003) tft_mode=2;
  if(tft_mode==0) tft_mode0();
  if(tft_mode==1) tft_mode1();
  if(tft_mode==2) tft_mode2();
}

void tft_mode0(){ // Inicio
  uint16_t top=160, left=160;
  tft.setTextColor(TFT_YELLOW,TFT_BLACK);
  tft.drawCentreString(String(prev_millis/1000), left, top, 2); // left,top
}

void tft_mode1(){ ///// Temperatura
  if(prev_tft_mode!=tft_mode){
    uint16_t top,left;
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_DARKGREY);
//    tft.drawCentreString(DeviceID, 160, 2, 2);
//    tft.drawCentreString(Versao, 160, 222, 2);
//    top=20;
//    //tft.fillRect( 0, top, 320, hei, TFT_DARKGREY);
//    tft.drawRect( 0, top, 320, 200, TFT_YELLOW);
    top=80; left=160; tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawCentreString(String(temp,1), left, top, 8);
  }
  prev_tft_mode=tft_mode;
}

void tft_mode2(){ ///// Osciloscopio
  tft.setTextColor(TFT_DARKGREY,TFT_BLACK);
  if(prev_tft_mode!=tft_mode){
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_DARKGREY);
    //tft.drawCentreString(labels[0], 160, 2, 2);
    tft.drawCentreString(Versao, 160, 222, 2);
  }
  float msd=w_us*w_esc*40/1000;
  tft.drawCentreString(" "+String(msd,1)+"ms/div ", 120, 2, 2);
  tft.drawCentreString(" "+String(vpd,1)+"V/div ", 200, 2, 2);
  //
  uint16_t top=20;
  uint16_t hei=200;
  uint16_t wid=320;
  //uint32_t cr[]={TFT_RED,TFT_GREEN,TFT_BLUE};
  uint32_t cr[]={TFT_YELLOW,TFT_CYAN,TFT_MAGENTA};
  uint16_t prex_x[3];
  for(uint16_t i=0;i<wid;i++){
    uint32_t cor=TFT_BLACK;
    //if( i%40==0 || i%40==39 ) cor=TFT_DARKGREY;
    if( i%(wid/10)==0 || i==(wid-1) ) cor=0x39E3; //TFT_DARKGREY
    tft.fillRect( i, top, 1, hei, cor); // grade horizontal
    cor=0x39E3;
    for(float j = -1.5; j <= 1.5; j+=0.5){
      tft.fillRect( i, top+(hei/2-1)-(j/3.3*(hei-1)), 1, 1, cor);
    }
    tft.fillRect( i, top, 1, 1, cor);
//    tft.fillRect( i, top+(hei/2-1), 1, 1, cor);
    tft.fillRect( i, top+hei-1, 1, 1, cor);
    for(int8_t j=0;j<3;j++){
      float k=i*400; k/=wid; k*=w_esc;
      uint16_t ki=(uint16_t)k; ki+=w_ini;
      float x;
      x=(wave[j][ki]*(hei-1)/1023);
      x*=(0.5/vpd);
      x=(top+(hei-1))-x;
      if(vpd==0.25){ x+=(hei/3.3*1.65); }
      if(vpd==2){ x-=(hei/3.3*1.23); x-=(hei/3.3*(1-j)); }
      if(x>(top+hei-1)) x=top+hei-1;
      if(x<top) x=top;
      if(i>0){tft.drawLine(i-1, prex_x[j], i, x, cr[j]);}
      else{tft.drawLine(i, x, i+1, x, cr[j]);}
      prex_x[j]=x;
    }
  }
  prev_tft_mode=tft_mode;
}

#endif

//  tft.drawRect( left, top, wid, hei, cor);
//  tft.drawLine( left, top, left , top, cor);
//  tft.drawCentreString("text", left, top, 2);
//  dtostrf(floatVar, minStringWidthIncDecimalPoint, numVarsAfterDecimal, charBuf);
//  char buf[8]; dtostrf(value, 4, 0, buf);

// https://github.com/Bodmer/TFT_ILI9341_ESP

// New color definitions use for all my libraries
// #define TFT_RED         0xF800      /* 255,   0,   0 */ // 1111 1000 0000 0000
// #define TFT_GREEN       0x07E0      /*   0, 255,   0 */ // 0000 0111 1110 0000
// #define TFT_BLUE        0x001F      /*   0,   0, 255 */ // 0000 0000 0001 1111
// #define TFT_YELLOW      0xFFE0      /* 255, 255,   0 */
// #define TFT_CYAN        0x07FF      /*   0, 255, 255 */
// #define TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
// #define TFT_BLACK       0x0000      /*   0,   0,   0 */
// #define TFT_NAVY        0x000F      /*   0,   0, 128 */
// #define TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
// #define TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */ // 0000 0011 1110 1111
// #define TFT_MAROON      0x7800      /* 128,   0,   0 */
// #define TFT_PURPLE      0x780F      /* 128,   0, 128 */
// #define TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
// #define TFT_LIGHTGREY   0xC618      /* 192, 192, 192 */
// #define TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
// #define TFT_WHITE       0xFFFF      /* 255, 255, 255 */
// #define TFT_ORANGE      0xFD20      /* 255, 165,   0 */
// #define TFT_GREENYELLOW 0xAFE5      /* 173, 255,  47 */
// #define TFT_PINK        0xF81F

//#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
//#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
//#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
//#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
//#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:.
//#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
//#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48

//  31,  31,  31 - 1111 1111 11-1 1111 - 0xFFFF - TFT_WHITE
//  15,  15,  15 - 0111 1011 11-0 1111 - 0x7BEF - TFT_DARKGREY
//   7,   7,   7 - 0011 1001 11-0 0011 - 0x39E3 - 
//   0,   0,   0 - 0000 0000 00-0 0000 - 0x0000 - TFT_BLACK
