
String ser_buf = "";

void serial_setup(){
  //Serial.begin(115200); // 230400-windows7 2000000-windows10
  Serial.begin(230400); // 230400-windows7 2000000-windows10
  Serial.println();
  Serial.println(Versao);
}

void serial_loop(){
  if (Serial.available()) {
    char inChar = (char)Serial.read();
    if(inChar == '\n') {
      ser_buf.trim(); ser_buf.replace("\n", ""); ser_buf.replace("\r", "");
      if (ser_buf.length() > 0) {
        ser_buf = "?" + ser_buf; ser_buf.replace("??", "&"); ser_buf.replace("?", "&");
        str=ser_buf; ser_buf="";
        serial_run();
      }
    }else{
      ser_buf += String(inChar);
    }
  }
}

String str_arg(String sa){
  String sb="";
  if(str.indexOf(sa)>=0){
    sb=str.substring(str.indexOf(sa)+sa.length())+"&";
    sb=sb.substring(0,sb.indexOf("&"));
  }
  return sb;
}

void serial_run() {
  String sa, sb, pg;
  sa=F("&rst");if(str.indexOf(sa)>=0){ESP.restart();return;}
  sa=F("&data.txt");if(str.indexOf(sa)>=0){serialDataTxt();return;}
  sa=F("&dataJSON.txt");if(str.indexOf(sa)>=0){serialDataJSON();return;}
  //
  sa=F("&dir="); if(str.indexOf(sa)>=0){ Serial.println("[]"); str = ""; return; }
  //
  sa=str_arg("&time=");if(sa.length()>0)    { sa = sa.substring(0, sa.length() - 3); time_t itime = sa.toInt() + ( -3 * SECS_PER_HOUR ); setTime(itime); }
  sa=str_arg("&tr_port=");if(sa.length()>0) { w_run=true; wave_clr(); if(tr_port!=sa.toInt()){ tr_port=sa.toInt(); fs_config_write(); } }
  sa=str_arg("&tr_mode=");if(sa.length()>0) { w_run=true; wave_clr(); if(tr_mode!=sa.toInt()){ tr_mode=sa.toInt(); fs_config_write(); } }
  sa=str_arg("&tr_auto=");if(sa.length()>0) { w_run=true; wave_clr(); if(tr_auto!=server.arg(sa).toInt()){ tr_auto=sa.toInt(); fs_config_write(); } }
  sa=str_arg("&w_map=");  if(sa.length()>0) { sa+=",,,"; for(uint8_t i=0;i<=2;i++){ sb=sa.substring(0,sa.indexOf(",")); sa=sa.substring(sa.indexOf(",")+1); w_map[i]=sb.toInt(); } w_run=true; wave_clr(); fs_config_write(); }
  sa=str_arg("&w_run=");  if(sa.length()>0) { w_run=sa.toInt(); w_run=true; wave_clr(); }
  sa=str_arg("&tr_hist=");if(sa.length()>0) { tr_hist=sa.toFloat(); }
  sa=str_arg("&w_msd=");  if(sa.length()>0) { w_us=sa.toInt()*50; w_run=true; wave_clr(); if(w_us>=500){tr_auto=0; } }
  sa=str_arg("&v_sinc="); if(sa.length()>0) { if(sa.toFloat()>0){ if(v_sinc<=(1013-30)) v_sinc+=30; }else{ if(v_sinc>=(10+30)) v_sinc-=30; } w_run=true; wave_clr(); }
  sa=str_arg("&w_ini=");  if(sa.length()>0) { w_ini=sa.toInt(); }
  sa=str_arg("&w_esc=");  if(sa.length()>0) { w_esc=sa.toFloat(); }
  sa=str_arg("&nome=");   if(sa.length()>0) { sa.replace("%20"," "); sa.replace("%0A",";"); sa.replace("%3A",":"); nome=sa; }
  sa=str_arg("&notas=");  if(sa.length()>0) { sa.replace("%20"," "); sa.replace("%0A",";"); sa.replace("%3A",":"); sa.replace("\n",";"); notas=sa; }
  sa=str_arg("&salvar="); if(sa.length()>0) { String path="/data/"+sa; salvarJSON(path); }
  //w_mode=tr_mode+(10*tr_auto);
  sa=str_arg("&dw=");     if(sa.length()>0) { ///// &dw=[port],[value]
    sb=sa+",";
    uint16_t port, state;
    port=sb.substring(0, sb.indexOf(",")).toInt();
    state=sb.substring(sb.indexOf(",") + 1).toInt();
    if(state==2) state=!digitalRead(port);
    bool bloquear=false;
    if(port==22 && state==0 && digitalRead(23)==1) bloquear=true;
    if(port==23 && state==0 && rele_cnt[3]>(-0.5*60*10)) bloquear=true;
    if(!bloquear){ pinMode(port, OUTPUT); digitalWrite(port, state); }
  }  
  sa=str_arg("&essid="); if(sa.length()>0){ essid=sa; fs_config_write(); }
  sa=str_arg("&epass="); if(sa.length()>0){ epass=sa; fs_config_write(); }
  sa=str_arg("&reset="); if(sa.length()>0){ rst_cnt=20; }
  ///// resp
  pg="";
  sa=str_arg("&dr="); if(sa.length()>0) { // &dr=[porta],[porta-porta],...
    sb=sa+",";
    if(pg.length()==0) pg+="{"; else pg+=",";
    pg+="\"dr\":[";
    String sc;
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
  sa=str_arg("&re2auto="); if(sa.length()>0){ re2auto=sa.toInt(); }
  //sa="re2auto"; if(server.hasArg(sa)){ re2auto=server.arg(sa).toInt(); }

  
  sa=str_arg("&fields="); if(sa.length()>0) {
    String fields=","+sa+",";
    if(pg.length()==0) pg+="{"; else pg+=",";
    sa=F(",temp,");  if(fields.indexOf(sa)>=0){
      pg+=F("\"temp\":");pg+=String(temp,1);  pg+=",";
      pg+=F("\"ds18_err_cnt\":");pg+=ds18_err_cnt;  pg+=",";
    }
    sa=F(",time,");     if(fields.indexOf(sa)>=0){  time_t t=now(); pg+=F("\"time\":");pg+=t;pg+=F(",");}
    sa=F(",w_us,");     if(fields.indexOf(sa)>=0){  pg+=F("\"w_us\":");pg+=w_us; pg+=","; }
    sa=F(",vref,");     if(fields.indexOf(sa)>=0){  pg+=F("\"vref\":");pg+=String(vref); pg+=","; }
    sa=F(",w_run,");    if(fields.indexOf(sa)>=0){  pg+=F("\"w_run\":");pg+=String(w_run); pg+=","; }
    sa=F(",tr_port,");  if(fields.indexOf(sa)>=0){  pg+=F("\"tr_port\":");pg+=String(tr_port); pg+=","; }
    sa=F(",v_sinc,");   if(fields.indexOf(sa)>=0){  pg+=F("\"v_sinc\":"); pg+=String(v_sinc,0); pg+=",";  }
    sa=F(",w_esc,");    if(fields.indexOf(sa)>=0){  pg+=F("\"w_esc\":");pg+=w_esc; pg+=","; }
    //sa=F(",w_mode,");   if(fields.indexOf(sa)>=0){  pg+=F("\"w_mode\":");pg+=String(w_mode); pg+=","; }
    sa=F(",tr_mode,");  if(fields.indexOf(sa)>=0){  pg+=F("\"tr_mode\":");pg+=tr_mode; pg+=","; }
    sa=F(",tr_auto,");  if(fields.indexOf(sa)>=0){  pg+=F("\"tr_auto\":");pg+=tr_auto; pg+=","; }
    sa=F(",tr_hist,");  if(fields.indexOf(sa)>=0){  pg+=F("\"tr_hist\":");pg+=String(tr_hist,0); pg+=","; }
    sa=F(",rele_cnt,"); if(fields.indexOf(sa)>=0){  pg+=F("\"rele_cnt\":["); for(uint8_t i=0;i<=3;i++){ pg+=rele_cnt[i]; if(i<3){ pg+=",";} } pg+="],"; }
    sa=F(",w_map,");    if(fields.indexOf(sa)>=0){  pg+=F("\"w_map\":["); for(uint8_t i=0;i<=2;i++){ pg+=w_map[i]; if(i<2){ pg+=",";} } pg+="],"; }
    if(pg.endsWith(",")); pg=pg.substring(0,pg.length()-1);
  }
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
  //
  if(pg.length()==0) pg="{";
  pg+="}";
  Serial.println(pg);
}

String str32(uint16_t n){
  String sa="";
  uint16_t ia=n%32; uint16_t ib=n/32;
  char ca;
  if(ib<10) ca=(ib+48); else ca=(ib+55);
  sa+=ca;
  if(ia<10) ca=(ia+48); else ca=(ia+55);
  sa+=ca;
  return sa;
}

/*/ javaScript decoder
function int32(str){
  ia=str.charCodeAt(0); if(ia<65) ia-=48; else ia-=55;
  ib=str.charCodeAt(1); if(ib<65) ib-=48; else ib-=55;
  ic=ia*32+ib;
  return ic;
}
//*/

void serialDataTxt(){
  String pg="";
  pg+="nome : ";  pg+=nome; pg+=";";
  pg+="notas : "; pg+=notas; pg+=";";
  pg+="temp : ";  pg+=String(temp,1); pg+=";";
  pg+="w_us : ";  pg+=w_us; pg+=";";
  for(uint16_t i=0;i<=1999;i++){
    for(uint16_t j=0;j<=2;j++){
      String sa=String(wave[j][i]*vref/1023,2);
      sa.replace(".",",");
      pg+=sa;
      if(j<2) pg+=" ";
    }
    if(i<1999) pg+=";";
    if(i%100==0){Serial.print(pg); pg=""; delay(10);}
  }
  Serial.println(pg);
}

void serialDataJSON(){
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
      if(i%100==0){Serial.print(pg); pg=""; delay(100);}
    }
    pg+="]";
    if(j<2) pg+=",";
  }
  pg+="]}";
  Serial.println(pg);
}
