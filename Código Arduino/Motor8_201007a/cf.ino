
void DeviceID_gen(){
  uint32_t chipId = 0;
  for(int i=0; i<17; i=i+8) { chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i; }
  String ChipId = String(chipId,HEX); ChipId.toUpperCase();
  DeviceID=Versao; DeviceID+="_"; DeviceID=DeviceID.substring(0,DeviceID.indexOf("_"));
  DeviceID += "_"+ChipId;
}

void handleConfig(){
  String pg;
  if(server.args()>0){
    String sa,sb,sc;
    //
    sa=F("DeviceID");     if(server.hasArg(sa)){sb=server.arg(sa); DeviceID=sb; }
    sa=F("apPass");       if(server.hasArg(sa)){sb=server.arg(sa); apPass=sb; }
    sa=F("essid");        if(server.hasArg(sa)){sb=server.arg(sa); essid=sb; }
    sa=F("epass");        if(server.hasArg(sa)){sb=server.arg(sa); epass=sb; }
    //sa=F("ntpServerIP");  if(server.hasArg(sa)){sb=server.arg(sa); sb+="."; for(uint8_t i=0;i<=3;i++){ sc=sb.substring(0,sb.indexOf(".")); sb=sb.substring(sb.indexOf(".")+1); ntpServerIP[i]=sc.toInt(); } }
    sa=F("apIP");         if(server.hasArg(sa)){sb=server.arg(sa); sb+="."; for(uint8_t i=0;i<=3;i++){ sc=sb.substring(0,sb.indexOf(".")); sb=sb.substring(sb.indexOf(".")+1); apIP[i]=sc.toInt(); } }
    sa=F("ipMode");       if(server.hasArg(sa)){sb=server.arg(sa); ipMode=sb.toInt(); }
    sa=F("ntpServer0");   if(server.hasArg(sa)){ ntpServer[0]=server.arg(sa); }
    sa=F("ntpServer1");   if(server.hasArg(sa)){ ntpServer[1]=server.arg(sa); }
    sa=F("ntpServer2");   if(server.hasArg(sa)){ ntpServer[2]=server.arg(sa); }
    //
    pg=F("<body onload='history.go(-2);'></body>");
    server.send(200, "text/html", pg);
    fs_config_write();
    rst_cnt=20;
    return;
  }
  if (handleFileRead("/config.htm")) { return; }
  pg+=F("config");
  server.send(200, "text/html", pg);
}

void fs_config_write() {
  if(!fsok) return;
  addLog("FS_config_write");
  String sa, sb; sa=F("/config.txt");
  File wFile = SPIFFS.open(sa, "w");
  if (wFile) {
    wFile.println( "DeviceID: "    + DeviceID );
    wFile.println( "apPass: "      + apPass   );
    wFile.println("essid: "+essid);
    wFile.println("epass: "+epass);
    //wFile.println( "ntpServerIP: " + ipStr(ntpServerIP) );
    wFile.println( "ipMode: "      + String(ipMode)     );
    wFile.println( "apIP: "        + ipStr(apIP)        );
    wFile.println( "ntpServer[0]: " + ntpServer[0] );
    wFile.println( "ntpServer[1]: " + ntpServer[1] );
    wFile.println( "ntpServer[2]: " + ntpServer[2] );
    //
    wFile.println("tr_port: "+String(tr_port));
    wFile.println("tr_mode: "+String(tr_mode));
    wFile.println("tr_auto: "+String(tr_auto));
    for(uint8_t i=0;i<=2;i++){ sb+=w_map[i]; if(i<2){ sb+=",";} } wFile.println("w_map: "+sb);
    //
    wFile.close(); delay(1);
  }
}

void fs_config_read() {
  if(!fsok) return;
  //addLog("FS_config_read");
  String path, sa, sb, sc, sd; path = F("/config.txt");
  if (!SPIFFS.exists(path)) fs_config_write();
  File file = SPIFFS.open(path, "r");
  if (file) {
    while (file.available()) {
      sa = file.readStringUntil('\r'); sa.trim();
      sb = sa.substring(0, sa.indexOf(":"));
      sa = sa.substring(sa.indexOf(":") + 2);
      //
      if(sb=="DeviceID") if(sa.length()>0) DeviceID = sa;
      sc=F("apPass");       if (sc == sb) apPass = sa;
      sc=F("essid");        if (sc == sb) essid = sa;
      sc=F("epass");        if (sc == sb) { epass = sa; }
      sc=F("ntpServerIP");  if (sc == sb) { sa+="."; for(uint8_t i=0;i<=3;i++){ sc=sa.substring(0,sa.indexOf(".")); sa=sa.substring(sa.indexOf(".")+1); ntpServerIP[i]=sc.toInt(); } }
      sc=F("ipMode"); if(sb==sc){ ipMode=sa.toInt(); if(ipMode<2) ipMode=0; }
      if(sb=="apIP"){  sa+="."; for(uint8_t i=0;i<=3;i++){ sc=sa.substring(0,sa.indexOf(".")); sa=sa.substring(sa.indexOf(".")+1); apIP[i]=sc.toInt(); } }
      if(sb=="ntpServer[0]") ntpServer[0]=sa;
      if(sb=="ntpServer[1]") ntpServer[1]=sa;
      if(sb=="ntpServer[2]") ntpServer[2]=sa;
      //
      if(sa.length()>0){
        if(sb=="tr_port"){ tr_port=sa.toInt(); }
        if(sb=="tr_mode"){ tr_mode=sa.toInt(); }
        if(sb=="tr_auto"){ tr_auto=sa.toInt(); }
        sc = F("w_map"); if (sc == sb) if(sa.length()>0){
          sa+=",,,";
          for(uint8_t i=0;i<=2;i++){
            sb=sa.substring(0,sa.indexOf(","));
            sa=sa.substring(sa.indexOf(",")+1);
            if(sb.toInt() <= 5) w_map[i]=sb.toInt();
          }
        }
      }
      //
    }
    file.close();
  }
}
