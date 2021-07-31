
// https://github.com/Yveaux/esp8266-Arduino/blob/master/esp8266com/esp8266/libraries/OneWire/examples/DS18x20_Temperature/DS18x20_Temperature.pde

#ifdef ds18_pin


void ds18_millis(){
  float rv0,rv1;
  rv0=ds18b20(); delay(1); rv1=ds18b20();
  if((int)rv0 != (int)rv1){ delay(1); rv0=ds18b20(); delay(1); rv1=ds18b20(); }
  if((int)rv0 == (int)rv1) if(rv0 >= -55 && rv0 <=150 ){
    temp=rv0;
    ds18_err_cnt=0;
  }else{
    if(ds18_err_cnt<30) ds18_err_cnt++; else temp = -100.0;
  }
}

float ds18b20() {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius; //, fahrenheit;
  if ( !ds.search(addr)) {
    ds.reset_search();
    return -100.0;
  }
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);         // Read Scratchpad
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
  }
  celsius = (float)raw / 16.0;
  ds.reset_search();
  return celsius;
}

#endif
