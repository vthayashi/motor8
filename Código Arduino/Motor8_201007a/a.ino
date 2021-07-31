
#define LED_BUILTIN 2

void a_setup(){
  pinMode(LED_BUILTIN,OUTPUT);
}

void a_loop(){}

uint32_t re2auto_cnt;

void a_ticker_1ms(){
  //re2auto liga
  if(re2auto>0){
    if(!digitalRead(rele_map[3])){
      re2auto_cnt++;
      if(re2auto_cnt==re2auto){
        digitalWrite(rele_map[1], 1);
        digitalWrite(rele_map[2], 0);
      }
    }else{
      re2auto_cnt=0;
    }
  }
}

void a_ticker100(){ // 100ms
  if(ticker100_cnt%10 == 0) digitalWrite(LED_BUILTIN, HIGH);  // turns the LED on
  if(ticker100_cnt%2  == 1) digitalWrite(LED_BUILTIN, LOW);   // turns the LED off
  if(ticker100_cnt%2  == 0) if(loop_watchdog>0 || rst_cnt>0) digitalWrite(LED_BUILTIN, HIGH);   // turns the LED on
  // contadores tempo reles
  for(uint8_t i=0; i<4;i++){
    bool st=!digitalRead(rele_map[i]);
    if(st==1 && rele_cnt[i]<=0) rele_cnt[i]=1;
    if(st==0 && rele_cnt[i]>=0) rele_cnt[i]=-1;
    if(rele_cnt[i]>0 && (rele_cnt[i] <  32767) ) rele_cnt[i]++;
    if(rele_cnt[i]<0 && (rele_cnt[i] >- 32767) ) rele_cnt[i]--;
  }
  // auto desliga 1 minutos
  if( rele_cnt[3]>(1*60*10) ){
    if(!digitalRead(rele_map[3])){
      digitalWrite(rele_map[1], 1); digitalWrite(rele_map[2], 1);
      pinMode(rele_map[3], OUTPUT); digitalWrite(rele_map[3], 1);
    }
  }
//  //re2auto liga
//  if( !digitalRead(rele_map[1]) && !digitalRead(rele_map[3]) ){
//    if(re2auto>0){
//      if( (rele_cnt[3]*100) > re2auto ){
//        pinMode(rele_map[1], OUTPUT); digitalWrite(rele_map[1], 1);
//        pinMode(rele_map[2], OUTPUT); digitalWrite(rele_map[2], 0);
//      }
//    }
//  }
  //
}

void a_second(){}

void a_minute(){}

void a_hour(){
  addLog("new hour");
}

void a_day(){
  addLog("new day");
}

void a_mounth(){}

void a_year(){}

void w_read(){
  uint32_t prev_micros=micros();
  for(uint16_t i=0;i<2000;i++){
    wave[0][i]=analogRead(an_pins[w_map[0]]);
    wave[1][i]=analogRead(an_pins[w_map[1]]);
    wave[2][i]=analogRead(an_pins[w_map[2]]);
    while((micros()-prev_micros)<w_us) {}
    prev_micros+=w_us;
  }
  for(uint16_t i=0;i<2000;i++){
    for(uint8_t j=0;j<3;j++){
      wave[j][i]+=43;
      if(wave[j][i]>1023) wave[j][i]=1023;
    }
  }
 }

 void wave_clr(){
  for(uint16_t i=0;i<2000;i++){    for(uint8_t j=0;j<3;j++){ wave[j][i]=512; }  }
  tr_state=-1; prev_tr_state=-1;
 }
 
