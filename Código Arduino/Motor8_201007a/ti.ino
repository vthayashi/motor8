
#include <WiFiUdp.h>
WiFiUDP Udp;
time_t getNtpTime(String ntpServerIPstr);
void sendNTPpacket(IPAddress &address);

void time_setup(){
  time_log();
  time_t t=now();
  prev_hour=hour(t);    prev_day=day(t);
  prev_month=month(t);  prev_year=year(t);
  if(year()<2000) setTime(0,0,0,1,1,2000);
  if(WiFi.status() == WL_CONNECTED){
    Udp.begin(8888); // local port 8888
    time_t t=0;
    for(uint8_t i=0;i<3;i++){ if(t==0){ t=getNtpTime(ntpServer[i]); } }
    if(t>0){ setTime(t); }
  }
}

void time_loop(){
  time_t t=now();
  if(prev_sec!=second(t)) { log_second(); a_second(); }
  if(prev_min!=minute(t)) { a_minute(); }
  if(prev_hour!=hour(t))  { fs_hour(); a_hour(); }
  if(prev_day!=day(t))    { a_day(); }
  if(prev_month!=month(t)){ a_mounth(); }
  if(prev_year!=year(t))  { a_year(); }
  //
  if(prev_sec!=second(t))   { prev_sec=second(t); }
  if(prev_min!=minute(t))   { prev_min=minute(t); }
  if(prev_hour!=hour(t))    { prev_hour=hour(t);  }
  if(prev_day!=day(t))      { prev_day=day(t);    }
  if(prev_month!=month(t))  { prev_month=month(t); }
  if(prev_year!=year(t))    { prev_year=year(t); }
}

void time_log(){ // Time from Log
  String path="";
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while(file){
    String fileName = file.name();
    if(fileName.startsWith("/log/")){ if ( fileName > path ) path=fileName; }
    file = root.openNextFile();
  }
  if(path.length()>0){
    File rFile = SPIFFS.open(path,"r");
    if(rFile){
      String line,line2; char sa;
      while (rFile.available()) {
        sa=rFile.read();
        if(sa==10){
          line.trim(); if(line.length()>2) line2=line;
          line="";
        }else{
          line+=sa;
        }
      }
      uint16_t an,me,di; // /log/200818.txt
      an=path.substring(5,7).toInt()+2000;
      me=path.substring(7,9).toInt();
      di=path.substring(9,11).toInt();
      uint16_t ho,mi,se; // 13:31:11 update ok 368512
      ho=line2.substring(0,2).toInt();
      mi=line2.substring(3,5).toInt();
      se=line2.substring(6,8).toInt();
      setTime(ho,mi,se,di,me,an);
      addLog("TimeLog "+path.substring(5)+" "+line2.substring(0,8));
    }
    rFile.close();
  }
}

String dd_mm_aaaa(time_t t) {
  String sa;
  sa += str00(day(t)) + "-";
  sa += str00(month(t)) + "-";
  sa += String(year(t));
  return sa;
}

String hhmm(uint32_t t) {
  String sa;
  sa= str00(hour(t));   sa += ":";
  sa+=str00(minute(t));
  return sa;
}

String hhmmss(uint32_t t) {
  String sa;
  sa= str00(hour(t));   sa += ":";
  sa+=str00(minute(t)); sa += ":";
  sa+=str00(second(t));
  return sa;
}

String str00(uint16_t n){
  String result="0";
  result+=String(n);
  result=result.substring(result.length()-2);
  return result;
}

/*---------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime(String ntpServerIPstr){
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  // get a random server from the pool
  //WiFi.hostByName(ntpServerName, ntpServerIP);
  //Serial.print(ntpServerName);
  //Serial.print(": ");
  //addLog( "NTP Request: "+ipStr(ntpServerIP) );
  String sa,sb;
  sa=ntpServerIPstr+".";
  IPAddress ip;
  for(uint8_t i=0;i<4;i++){
    sb=sa.substring(0,sa.indexOf("."));
    sa=sa.substring(sa.indexOf(".")+1);
    ip[i]=sb.toInt();
  }
  sendNTPpacket( ip );
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      addLog("TimeNTP "+ipStr(ip)+" OK");
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  addLog("NTP "+ipStr(ip)+" Fail :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address){
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
