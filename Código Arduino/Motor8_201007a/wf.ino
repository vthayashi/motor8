
uint8_t prev_wifiStatus;

void wifi_wait_con(){
  for(uint8_t i=0;i<40;i++){ delay(500);
    if(WiFi.status() == WL_CONNECTED) break;
    if(WiFi.status() == WL_NO_SSID_AVAIL) break;
    if(WiFi.status() == WL_CONNECT_FAILED) break;
  }
}

void wf_setup(){
  WiFi.mode(WIFI_AP_STA);  
  if(essid.length()>2){
    if (String(WiFi.SSID()) != String(essid)){ WiFi.begin(essid.c_str(), epass.c_str()); }
    wifi_wait_con();
    if(WiFi.status() == WL_CONNECTED){
      String sa;
      sa="WiFi "; sa+=ipStr(WiFi.localIP()); addLog(sa);
      if(ipMode>0){
        IPAddress ip =  WiFi.localIP();
        if(ip[3] != ipMode){
          IPAddress gateway(255, 255, 255, 1);
          IPAddress subnet(255, 255, 0, 0);
          gateway[0] = ip[0]; gateway[1] = ip[1]; gateway[2] = ip[2];
          ip[3] = ipMode;
          if(Ping.ping(ip) == false){
            addLog("Ping "+ip.toString()+" Livre");
            WiFi.disconnect();
            WiFi.persistent(false);
            WiFi.mode(WIFI_OFF);
            WiFi.mode(WIFI_AP_STA);
            IPAddress primaryDNS(8, 8, 8, 8); // optional
            primaryDNS=gateway;
            IPAddress secondaryDNS(8, 8, 4, 4); // optional
            WiFi.config(ip, subnet, gateway, primaryDNS, secondaryDNS );
            WiFi.begin(essid.c_str(), epass.c_str());
            wifi_wait_con();
          }else{
            addLog("Ping "+ip.toString()+" Ocupado");
          }
        }
      }
      if (MDNS.begin(DeviceID.c_str())) { MDNS.addService("http", "tcp", 80); }
      addLog("MDNS http://"+DeviceID+".local");
    }
  }else{
    WiFi.begin("esp32", "12345678");
  }
  if(apPass.length()>=8){
    IPAddress NMask(255, 255, 255, 0);
    WiFi.softAPConfig(apIP, apIP, NMask);
    if(!WiFi.softAP(DeviceID.c_str(), apPass.c_str())){
      ESP.restart();
    }else{
      addLog("softAP "+DeviceID+" "+WiFi.softAPIP().toString());
    }
  }else{
    addLog("softAP Not Connected!");
  }
}

void wifi_loop(){
  server.handleClient();
  if(prev_wifiStatus!=WiFi.status()){
    prev_wifiStatus=WiFi.status();
    String sa; sa=wifiStatusText(prev_wifiStatus);
    if(prev_wifiStatus == WL_CONNECTED){ sa+=" "; sa+=ipStr(WiFi.localIP()); }
    addLog(sa);
  }
}

void web_setup(){ // 869482 869450 869430
  //SERVER INIT
  String sa;
  server.on("/",            handleRoot);
  server.on("/home",        handleRoot);
  server.on("/config",      handleConfig);
  server.on("/config.htm",  handleConfig);
  server.on("/list",        handleFileList);
  server.on("/fsformat",    handleFSformat);
  server.on("/reset",       handleReset);
  server.on("/js",          handleJson);
  server.on("/upload",      handleUpload);
  //load editor
  server.on("/edit", HTTP_GET,    [](){ if (!handleFileRead("/edit.htm")){ server.send(404, "text/plain", "FileNotFound");}});
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  server.on("/edit", HTTP_POST,   [](){ server.send(200, "text/plain", ""); }, handleFileUpload);
  server.onNotFound([](){ if (!handleFileRead(server.uri())) { server.send(404, "text/plain", "FileNotFound"); }
  });
  //
  server.on("/data.txt",      handleDataTxt);
  server.on("/dataJSON.txt",  handleDataJSON);
  server.begin();
  addLog("HTTP server started");
}

void handleReset(){
  String pg="<html><meta http-equiv='refresh' content='5; url=/'>";
  pg+=F("<body>Reset...</body></html>");
  server.send(200, "text/html", pg);
  rst_cnt=20;
}

String ipStr(IPAddress ip){ return String(ip[0])+"." + String(ip[1])+"."+String(ip[2])+"."+String(ip[3]);}

String wifiStatusText(byte x) {
  String result;
  if (x == 0) result = F("WL_IDLE_STATUS");
  if (x == 1) result = F("WL_NO_SSID_AVAIL");
  if (x == 2) result = F("WL_SCAN_COMPLETED");
  if (x == 3) result = F("WL_CONNECTED");
  if (x == 4) result = F("WL_CONNECT_FAILED");
  if (x == 5) result = F("WL_CONNECTION_LOST");
  if (x == 6) result = F("WL_DISCONNECTED");
  if (x == 255) result = F("WL_NO_SHIELD");
  if (result == "") result = String(x);
  return result;
}
