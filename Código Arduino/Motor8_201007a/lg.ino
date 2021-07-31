
String log_buffer;

void addLog(String text){
  log_buffer+=text+";;";
  if(log_buffer.length()>500){ log_buffer=log_buffer.substring(log_buffer.indexOf(";;")+2); }
  //if(Serial) Serial.println(">"+text);
}

void log_second(){
  time_t t=now();
  if(log_buffer.indexOf(";;")>=0){
    String sa;
    if(hour()<10){   sa+="0";} sa+=hour(); sa+=":";
    if(minute()<10){ sa+="0";} sa+=minute(); sa+=":";
    if(second()<10){ sa+="0";} sa+=second(); sa+=" ";
    sa+=log_buffer.substring(0,log_buffer.indexOf(";;"));
    log_buffer=log_buffer.substring(log_buffer.indexOf(";;")+2);
    fs_gravar_log(t, sa);
    log_recent+=sa+";;";
    if(log_recent.length()>300){ log_recent=log_recent.substring(log_recent.indexOf(";;")+2); }
  }
}

void fs_gravar_log(uint32_t t, String text){
  String path,sb; path=F("/log/");
  sb=year(t); sb=sb.substring(2); path+=sb;
  if(month(t)<10) sb="0"; else sb="";
  sb+=month(t); path+=sb;
  if(day(t)<10) sb="0"; else sb="";
  sb+=day(t); path+=sb; path+=F(".txt");
  File wFile = SPIFFS.open(path,"a");
  if(!wFile){
    addLog(F("Erro ao abrir arquivo!"));
  }else{
    sb=text;
    wFile.println(sb);
  }
  wFile.close(); delay(10);
}
