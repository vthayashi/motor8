// https://lastminuteengineers.com/handling-esp32-gpio-interrupts-tutorial/
// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html
// https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-ledc.h

void handleJson(){
  String sa,sb,pg,fields;
  sa=F("time"); if(server.hasArg(sa)){
    String stime=server.arg(sa);
    stime = stime.substring(0,stime.length()-3);
    time_t itime = stime.toInt() + ( timeZone * SECS_PER_HOUR );
    setTime(itime); addLog("setTime()");
  }
  //
  str="";
  for(uint8_t i=0;i<server.args();i++){
    str+="&";
    str+=server.argName(i); str+="=";
    str+=server.arg(i);
  }
  //
  sa="tr_port";if(server.hasArg(sa)){ w_run=true; wave_clr(); if(tr_port!=server.arg(sa).toInt()){ tr_port=server.arg(sa).toInt(); fs_config_write(); } }
  sa="tr_mode";if(server.hasArg(sa)){ w_run=true; wave_clr(); if(tr_mode!=server.arg(sa).toInt()){ tr_mode=server.arg(sa).toInt(); fs_config_write(); } }
  sa="tr_auto";if(server.hasArg(sa)){ w_run=true; wave_clr(); if(tr_auto!=server.arg(sa).toInt()){ tr_auto=server.arg(sa).toInt(); fs_config_write(); } }
  sa=str_arg("&w_map=");  if(sa.length()>0){ sa+=",,,"; for(uint8_t i=0;i<=2;i++){ sb=sa.substring(0,sa.indexOf(",")); sa=sa.substring(sa.indexOf(",")+1); w_map[i]=sb.toInt(); } w_run=true; wave_clr(); fs_config_write(); }
  sa="w_run";if(server.hasArg(sa)){  w_run=server.arg(sa).toInt(); w_run=true; wave_clr(); }
  sa="tr_hist";if(server.hasArg(sa)){ tr_hist=server.arg(sa).toFloat(); }
  sa="w_msd";if(server.hasArg(sa)){ w_us=server.arg(sa).toInt()*50; w_run=true; wave_clr(); if(w_us>=500){tr_auto=0;} }
  sa="v_sinc";if(server.hasArg(sa)){ if(server.arg(sa).toFloat()>0){ if(v_sinc<=(1013-30)) v_sinc+=30; }else{ if(v_sinc>=(10+30)) v_sinc-=30; } w_run=true; wave_clr(); }
  sa=F("w_ini");if(server.hasArg(sa)){ w_ini=server.arg(sa).toInt(); }
  sa=F("w_esc");if(server.hasArg(sa)){ w_esc=server.arg(sa).toFloat(); }
  //w_mode=tr_mode+(10*tr_auto);
  sa="dw"; if(server.hasArg(sa)){ ///// &dw=[port],[value]
    sb=server.arg(sa)+",";
    uint16_t port, state;
    port = sb.substring(0, sb.indexOf(",")).toInt();
    state = sb.substring(sb.indexOf(",") + 1).toInt();
    if(state==2) state=!digitalRead(port);
    pinMode(port, OUTPUT); digitalWrite(port, state);
  }  
  //
  sa=F("nome"); if(server.hasArg(sa)){ nome=server.arg(sa); }
  sa=F("notas"); if(server.hasArg(sa)){ notas=server.arg(sa); notas.replace("\n",";");}
  sa=F("salvar"); if(server.hasArg(sa)){ String path="/data/"+server.arg(sa); salvarJSON(path); }
  sa=str_arg("&vpd="); if(sa.length()>0){ vpd=sa.toFloat(); }
  ///// Reles
  sa=str_arg("&re="); if(sa.length()>0){ // re=3,0(liga rele 3)
    uint16_t port, state;
    port = sa.substring(0, sa.indexOf(",")).toInt();
    state = sa.substring(sa.indexOf(",") + 1).toInt();
    pinMode(rele_map[port], OUTPUT);
    bool bloquear=false;
    if(port==1 && state==0){ digitalWrite(rele_map[2], 1); delay(100); }
    if(port==2 && state==0){ digitalWrite(rele_map[1], 1); delay(100); }
    // bloqueio para ligar 10 seg
    if(port==3 && state==0 && rele_cnt[3]>(-10*10)) bloquear=true;
    if(state==2) state=!digitalRead(port);
    if(!bloquear){
      digitalWrite(rele_map[port], state);
    }
    if(port==3 && state==1){
      digitalWrite(rele_map[1], 1); digitalWrite(rele_map[2], 1);
    }
  }
  sa="re2auto"; if(server.hasArg(sa)){ re2auto=server.arg(sa).toInt(); }
  ///// resp
  sa="dr"; if(server.hasArg(sa)){ ///// &dr=[porta],[porta-porta],...
    if(pg.length()==0) pg+="{"; else pg+=",";
    pg+="\"dr\":[";
    String sc;
    sb=server.arg(sa)+",";
    String pg2="";
    while(sb.indexOf(",")>=0){
      sc=sb.substring(0,sb.indexOf(","));
      sb=sb.substring(sb.indexOf(",")+1);
      if(sc.indexOf("-")>0){
        uint8_t ini=sc.substring(0,sc.indexOf("-")).toInt();
        uint8_t fim=sc.substring(sc.indexOf("-")+1).toInt();
        for(uint8_t pin=ini;pin<=fim;pin++){
          if(pg2.length()>0) pg2+=",";
          if (digitalRead(pin)) pg2 += "1"; else pg2 += "0";
        }
      }else{
        if(pg2.length()>0) pg2+=",";
        uint8_t pin=sc.toInt();
        if (digitalRead(pin)) pg2 += "1"; else pg2 += "0";
      }
    }
    pg+=pg2+"]";
  }
  
  //
  sa=F("fields");if(server.hasArg(sa)){  fields=","+server.arg(sa)+",";
    if(pg.length()==0) pg+="{"; else pg+=",";
    sa=F(",temp,");  if(fields.indexOf(sa)>=0){
      pg+=F("\"temp\":");pg+=String(temp,1);  pg+=",";
      pg+=F("\"ds18_err_cnt\":");pg+=ds18_err_cnt;  pg+=",";
    }
    sa=F(",time,");     if(fields.indexOf(sa)>=0){time_t t=now(); pg+=F("\"time\":");pg+=t;pg+=F(",");}
    sa=F(",w_us,");     if(fields.indexOf(sa)>=0){ pg+=F("\"w_us\":");pg+=w_us;              pg+=","; }
    sa=F(",vref,");     if(fields.indexOf(sa)>=0){ pg+=F("\"vref\":");pg+=String(vref);      pg+=","; }
    sa=F(",w_run,");    if(fields.indexOf(sa)>=0){   pg+=F("\"w_run\":");pg+=String(w_run); pg+=","; }
    sa=F(",tr_port,");  if(fields.indexOf(sa)>=0){ pg+=F("\"tr_port\":");pg+=String(tr_port); pg+=","; }
    sa=F(",v_sinc,");   if(fields.indexOf(sa)>=0){  pg+=F("\"v_sinc\":"); pg+=String(v_sinc,0); pg+=",";  }
    sa=F(",w_esc,");    if(fields.indexOf(sa)>=0){ pg+=F("\"w_esc\":");pg+=w_esc; pg+=","; }
    //sa=F(",w_mode,");   if(fields.indexOf(sa)>=0){  pg+=F("\"w_mode\":");pg+=String(w_mode);  pg+=","; }
    sa=F(",tr_mode,");  if(fields.indexOf(sa)>=0){ pg+=F("\"tr_mode\":");pg+=tr_mode;  pg+=","; }
    sa=F(",tr_auto,");  if(fields.indexOf(sa)>=0){ pg+=F("\"tr_auto\":");pg+=tr_auto;  pg+=","; }
    sa=F(",tr_hist,");  if(fields.indexOf(sa)>=0){ pg+=F("\"tr_hist\":");pg+=String(tr_hist,0);  pg+=","; }
    sa=F(",rele_cnt,"); if(fields.indexOf(sa)>=0){ pg+=F("\"rele_cnt\":["); for(uint8_t i=0;i<=3;i++){ pg+=rele_cnt[i]; if(i<3) pg+=","; } pg+="],"; }
    sa=F(",w_map,");    if(fields.indexOf(sa)>=0){  pg+=F("\"w_map\":["); for(uint8_t i=0;i<=2;i++){ pg+=w_map[i]; if(i<2){ pg+=",";} } pg+="],"; }
    if(fields.indexOf(",fsub,")>=0){ pg+="\"fsub\":"; pg+=SPIFFS.usedBytes(); pg+=","; }
    if(fields.indexOf(",fstb,")>=0){ pg+="\"fstb\":"; pg+=SPIFFS.totalBytes(); pg+=","; }
    //
    fields.replace(",","\"");
    sa=F("\"Versao\"");         if(fields.indexOf(sa)>=0){ pg+=sa; pg+=":\""; pg+=Versao;                 pg+="\","; } // 832558 bytes
    sa=F("\"essid\"");          if(fields.indexOf(sa)>=0){ pg+=sa; pg+=":\""; pg+=essid;                  pg+="\","; } // 832582 bytes
    sa=F("\"epass\"");          if(fields.indexOf(sa)>=0){ pg+=sa; pg+=":\""; pg+=epass;                  pg+="\","; } // 832586 bytes
    sa=F("\"ipMode\"");         if(fields.indexOf(sa)>=0){ pg+=sa; pg+=":"; pg+=ipMode;               pg+=",";  }
    sa=F("\"localIP\"");        if(fields.indexOf(sa)>=0){ pg+=sa; pg+=":\""; pg+=ipStr(WiFi.localIP());  pg+="\","; } // 832562 bytes
    sa=F("\"DeviceID\"");       if(fields.indexOf(sa)>=0){ pg+=sa; pg+=":\""; pg+=DeviceID;               pg+="\","; } // 832558 bytes
    sa=F("\"apPass\"");         if(fields.indexOf(sa)>=0){ pg+=sa; pg+=":\""; pg+=apPass;                 pg+="\","; } // 832582 bytes
    sa=F("\"apIP\"");           if(fields.indexOf(sa)>=0){ pg+=sa; pg+=":\""; pg+=ipStr(apIP);            pg+="\","; } // 832562 bytes
    //sa=F("\"ntpServerIP\"");    if(fields.indexOf(sa)>=0){ pg+=sa; pg+=":\""; pg+=ipStr(ntpServerIP);     pg+="\","; }
    sa=F("\"log_recent\"");     if(fields.indexOf(sa)>=0){ pg+=sa; pg+=":\""; pg+=log_recent;             pg+="\","; }
    //
    sa=F("\"ntpServer\"");      if(fields.indexOf(sa)>=0){
      pg+=sa; pg+=":[\"";
      pg+=ntpServer[0]; pg+="\",\"";
      pg+=ntpServer[1]; pg+="\",\"";
      pg+=ntpServer[2]; pg+="\"],";
    }
    //
    if(pg.endsWith(",")); pg=pg.substring(0,pg.length()-1);
  }
  //
  sa=str_arg("&wb="); if(sa.length()>0) {
    uint8_t ia = sa.toInt();
    if(pg.length()==0) pg+="{"; else pg+=",";
    if(ia<3){
      pg+="\"wb"+String(ia)+"\":\"";
      if(w_esc<=1){
        for(uint16_t i=0; i<=399;i++){ pg+=str32(wave[ia][i+w_ini]); }
      }else{
        for(uint16_t i=0; i<=399;i++){ pg+=str32(wave[ia][(i*(uint16_t)w_esc)+w_ini]); }
      }
      pg+="\"";
    }
  }
  /////
  if(pg.length()==0) pg="{";
  pg+="}";
  server.send(200, "text/json", pg);
}

void salvarJSON(String path){
  File file = SPIFFS.open(path, FILE_WRITE);
  if(file){
    String pg="";
    pg+="{";
    pg+="\"nome\":\"";pg+=nome; pg+="\",";
    pg+="\"notas\":\"";pg+=notas; pg+="\",";
    pg+="\"temp\":";pg+=temp; pg+=",";
    pg+="\"w_us\":";pg+=w_us; pg+=",";
    pg+="\"wa\":[";
    file.print(pg); pg="";
    for(uint16_t j=0;j<=2;j++){
      pg+="[";
      for(uint16_t i=0;i<=1999;i++){
        pg+=String(wave[j][i]*vref/1023,2);
        if(i<1999) pg+=",";
      }
      pg+="]";
      if(j<2) pg+=",";
      file.print(pg); pg="";
    }
    pg+="]}";
    file.print(pg);
  }
  file.close(); delay(10);
}

void handleDataTxt(){
  String pg="";
  pg+="nome : ";  pg+=nome; pg+="\n";
  pg+="notas : "; pg+=notas; pg+="\n";
  pg+="temp : ";  pg+=String(temp,1); pg+="\n";
  pg+="w_us:"; pg+=w_us; pg+="\n";
  for(uint16_t i=0;i<=1999;i++){
    for(uint16_t j=0;j<=2;j++){
      String sa=String(wave[j][i]*vref/1023,2);
      sa.replace(".",",");
      pg+=sa;
      if(j<2) pg+=" ";
    }
    if(i<1999) pg+="\n";
  }
  server.send(200, "text/plain", pg);
}

void handleDataJSON(){
  String pg="";
  pg+="{";
  pg+="\"nome\":\"";pg+=nome; pg+="\",";
  pg+="\"notas\":\"";pg+=notas; pg+="\",";
  pg+="\"temp\":\"";pg+=String(temp,1); pg+="\",";
  pg+="\"w_us\":";pg+=w_us; pg+=",";
  pg+="\"wa\":[";
  for(uint16_t j=0;j<=2;j++){
    pg+="[";
    for(uint16_t i=0;i<=1999;i++){
      pg+=String(wave[j][i]*vref/1023,2);
      if(i<1999) pg+=",";
    }
    pg+="]";
    if(j<2) pg+=",";
  }
  pg+="]}";
  server.send(200, "text/plain", pg);
}
