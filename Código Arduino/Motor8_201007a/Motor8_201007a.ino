
#define Versao "Motor8_201007a"

//#define Versao "Motor6tft_200801a"
//#define tft_cs_pin 5

String  essid;    //essid: YTD960495veredas
String  epass;    //epass: 784257hue031
uint8_t ipMode=0; // 0=auto

#define LOG_LOCAL_LEVEL ESP_LOG_INFO

String DeviceID;
String apPass = "12345678";
IPAddress apIP(192,168,4,1);
//
#ifdef tft_cs_pin
  uint8_t   an_pins[]={32,36,39,35,34,33};
#else
  uint8_t   an_pins[]={36,39,34,35,32,33}; // ESP32S
#endif
uint8_t   w_map[]={0,1,2}; // w_map=36,32,39
float     vref=3.3;
uint32_t  tr_millis;
int8_t    tr_state=-1, prev_tr_state=-1;
uint8_t   tr_port=0;
uint16_t  w_us=50;    // min=40 (ESP32)
uint16_t  w_ini=0;
float     w_esc=1;
//uint8_t   w_mode=11;  // 0-parado  1-manRising  2-manFalling  3-manChange
//                      //          11-norRising 12-norFalling 13-norChange
uint8_t   tr_mode=1;  // 1-risi 2-fall 3-chan
uint8_t   tr_auto=1;  // 0-sing 1-norm 2-auto
bool      w_run=true;  
float     v_sinc=511; //, v_sinc0=501, v_sinc1=521; // 0 a 1023
float     tr_hist=20;
String    nome,notas;
float     wave[3][2000];
// rele[3] Liga
// rele[2] Triangulo
// rele[1] Estrela
#ifdef tft_cs_pin
  uint8_t   rele_map[]={27,25,2,13};
#else
  uint8_t   rele_map[]={19,21,22,23}; // ESP32S
#endif
int32_t   rele_cnt[]={-1,-1,-1,-1000};
uint16_t  re2auto=0;
String    str;
float     vpd=2.0;

#include <WiFi.h>
//#include <WiFiClient.h>
#include <WebServer.h>
WebServer server(80);
#include <ESPmDNS.h>
#include <ESP32Ping.h>

#include <SPIFFS.h>
bool fsok=false;
File fsUploadFile;

#include <TimeLib.h>
const int timeZone = -3;
IPAddress ntpServerIP(132,163,97,1); //https://tf.nist.gov/tf-cgi/servers.cgi
//IPAddress ntpServerIP(200,160,7,186); //https://ntp.br/
String    ntpServer[3]={"132.163.97.1","132.163.97.2","200.160.7.186"};
uint8_t   prev_sec=60;
uint8_t   prev_min=60;
uint8_t   prev_hour=24;
uint8_t   prev_day=32;
uint8_t   prev_month=13;
uint16_t  prev_year=0;

String  log_recent;

#include <Ticker.h>
Ticker ticker100;
uint32_t ticker100_cnt;
uint8_t loop_watchdog;
void tk100(){
  ticker100_cnt++;
  a_ticker100();
  loop_watchdog++;
}
Ticker ticker_1;
uint32_t ticker_1ms_cnt;
void ticker_1ms() {
  if(ticker_1ms_cnt%5==0) digitalWrite(12, !digitalRead(12)); // 1/(5ms+5ms)=100Hz
  a_ticker_1ms();
  ticker_1ms_cnt++;
}

#ifdef tft_cs_pin
  #define ds18_pin 14   // Sensor DS18X20 (3 a 5V 4.7K resistor is necessary)
#else
  #define ds18_pin 26   // Sensor DS18X20 (3 a 5V 4.7K resistor is necessary)
#endif
#include <OneWire.h>  // versao 2.3.5 https://github.com/PaulStoffregen/OneWire
OneWire  ds(ds18_pin);
float temp= -100.0;
uint16_t ds18_err_cnt=0;

uint8_t rst_cnt=0;

void setup() {
  pinMode(12, OUTPUT);
  serial_setup();
  DeviceID_gen();
  fs_setup();
  fs_config_read();
  addLog("Versao "+String(Versao));
  if(!fsok){ addLog("SPIFFS ERR"); }else{ addLog("fsok"); }
  #ifdef tft_cs_pin
    tft_setup();
  #endif
  wf_setup();
  update_setup();
  web_setup();
  time_setup();
  a_setup();
  ticker100.attach(0.1, tk100 );
  analogReadResolution(10);
  //analogWriteFrequency(60);
  //analogWrite(27,512,1023);
  delay(10);
  w_read();
  //wave_clr();
  ticker_1.attach_ms(1, ticker_1ms);
}

uint32_t prev_millis;
uint32_t millis_cnt;

void loop() {
  wifi_loop();
  time_loop();
  if(prev_millis!=millis()){
    prev_millis=millis();
    if(prev_millis%1000==1){ ds18_millis(); }
    if(prev_millis%100==2){
      if(rst_cnt>0){
        if( rst_cnt==20 ){ addLog("Reset"); }
        rst_cnt--;
        if(rst_cnt==0) ESP.restart();
      }
    }
    #ifdef tft_cs_pin
      tft_loop1000();
    #endif
    //
    millis_cnt++;
  }
  if(tr_millis<=millis()){
    if(w_run){
      float rv=analogRead(an_pins[w_map[tr_port]]); rv=rv+43; // rv=(rv*2.48/3200.0)+0.17;
      if(rv<(v_sinc-tr_hist)) tr_state=0;
      if(rv>(v_sinc+tr_hist)) tr_state=1;
      bool ativar=false;
      if(tr_mode==1 && tr_state==1 && prev_tr_state==0) ativar=true;
      if(tr_mode==2 && tr_state==0 && prev_tr_state==1) ativar=true;
      if(tr_mode==3){
        if(tr_state==0 && prev_tr_state==1) ativar=true;
        if(tr_state==1 && prev_tr_state==0) ativar=true;
      }
      if(tr_auto==2){
        int32_t t=millis()-tr_millis;
        if(t>(w_us*2000/1000)) ativar=true;
      }
      
      if(ativar){
        w_read();
        tr_state=-1; prev_tr_state=-1;
        if(tr_auto==0) w_run=false;
        tr_millis= millis()+500;
      }
      prev_tr_state=tr_state;
    }
  }
  serial_loop();
  loop_watchdog=0;
}
