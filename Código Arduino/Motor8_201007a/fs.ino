

void fs_setup(){
  fsok=SPIFFS.begin();
}

void fs_hour(){
  if(prev_hour%6==0) fs_auto_del("/log/",7);
  if(prev_hour%6==1) fs_auto_del("/data/",50);
}

void handleFSformat(){
  String pg;
  pg+=F(
    "<html><meta http-equiv='refresh' content='10; url=/'>"
    "<body>Formatando...</body></html>");
  server.send(200, "text/html", pg);
  SPIFFS.format();
  rst_cnt=20;
}

void handleUpload() {
  String pg;
  pg=F(
    "<html><head>\n"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>\n"
    "<style>\n"
    " input{border:solid 1px silver;border-radius:4px;background-color:white;cursor:hand;}"
    " .custom-file-input::-webkit-file-upload-button {visibility:hidden;width:0;}\n"
    " td{text-align:center;}"
    "</style>\n"
    "</head>\n"
    "<body><div align='center'>\n"
    "<table><tr><td><b>Upload (Arquivos)</b></td></tr>\n"
    "<tr><form target='if01' action='/edit' method='post' enctype='multipart/form-data'>\n"
    "<td><input id='update' type='file' name='name' class='custom-file-input' onchange='updateChange()'></td>\n"
    "</tr><tr><td><input id='submitBtn' type='submit' value='Upload' style='height:23px;'></td>\n"
    "</form></tr></table>"
    "<iframe id='if01' name='if01' onload='voltar();' style='visibility:hidden;'></iframe>"
    "<script>function voltar(){update.value='';}</script>"
    "</div></body></html>"
    );
  server.send(200, "text/html", pg);
}

void fs_auto_del(String path, uint8_t maxFiles){
  float fs_usedBytes=SPIFFS.usedBytes()/1000;
  float fs_totalBytes=SPIFFS.totalBytes()/1000;
  String path2 = String();
  String sa;
  uint8_t n=0;
  File root = SPIFFS.open("/");
  if(root.isDirectory()){
      File file = root.openNextFile();
      while(file){
        sa = file.name();
        if(sa.indexOf(path)==0){
          n++;
          if(sa<path2 || path2=="") path2=sa;
        }
        file = root.openNextFile();
      }
  }
  sa="fs_del "; sa+=n;
  if( (n>maxFiles) || (fs_usedBytes > (fs_totalBytes*0.9) ) ) {
    SPIFFS.remove(path2); delay(10);
    sa+=" "+path2;
    addLog(sa);
  }
}

////////////////////////////////////////////////////////////////////////////

//format bytes
String formatBytes(size_t bytes) {
  if (bytes < 1024) { return String(bytes) + "B";  }
  else if (bytes < (1024 * 1024)) {    return String(bytes / 1024.0) + "KB";  } 
  else if (bytes < (1024 * 1024 * 1024)) { return String(bytes / 1024.0 / 1024.0) + "MB"; }
  else { return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";  }
}

String getContentType(String filename) {
  if (server.hasArg("download")) { return "application/octet-stream"; }
  else if (filename.endsWith(".htm"))   { return "text/html"; }
  else if (filename.endsWith(".html"))  { return "text/html"; }
  else if (filename.endsWith(".css"))   { return "text/css"; }
  else if (filename.endsWith(".js"))    { return "application/javascript"; }
  else if (filename.endsWith(".png")) { return "image/png";}
  else if (filename.endsWith(".gif")) { return "image/gif"; }
  else if (filename.endsWith(".jpg")) { return "image/jpeg";}
  else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filename.endsWith(".xml")) {
    return "text/xml";
  } else if (filename.endsWith(".pdf")) {
    return "application/x-pdf";
  } else if (filename.endsWith(".zip")) {
    return "application/x-zip";
  } else if (filename.endsWith(".gz")) {
    return "application/x-gzip";
  }
  return "text/plain";
}

bool exists(String path){
  bool yes = false;
  File file = SPIFFS.open(path, "r");
  if(!file.isDirectory()){    yes = true;  }
  file.close();
  return yes;
}

bool handleFileRead(String path) {
  //DBG_OUTPUT_PORT.println("handleFileRead: " + path);
  if (path.endsWith("/")) {    path += "index.htm";  }
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (exists(pathWithGz) || exists(path)) {
    if (exists(pathWithGz)) {      path += ".gz";    }
    File file = SPIFFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload() {
  if (server.uri() != "/edit") { return; }
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) {      filename = "/" + filename;    }
    //DBG_OUTPUT_PORT.print("handleFileUpload Name: "); DBG_OUTPUT_PORT.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile) { fsUploadFile.write(upload.buf, upload.currentSize); }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {      fsUploadFile.close();    }
    //DBG_OUTPUT_PORT.print("handleFileUpload Size: "); DBG_OUTPUT_PORT.println(upload.totalSize);
  }
}

void handleFileDelete() {
  if (server.args() == 0) {    return server.send(500, "text/plain", "BAD ARGS");  }
  String path = server.arg(0);
  //DBG_OUTPUT_PORT.println("handleFileDelete: " + path);
  if (path == "/") {    return server.send(500, "text/plain", "BAD PATH");  }
  if (!exists(path)) {    return server.send(404, "text/plain", "FileNotFound");  }
  SPIFFS.remove(path);
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileList() {
  if (!server.hasArg("dir")) {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }
  String path = server.arg("dir");
  //DBG_OUTPUT_PORT.println("handleFileList: " + path);
  File root = SPIFFS.open(path);
  path = String();
  String output = "[";
  if(root.isDirectory()){
      File file = root.openNextFile();
      while(file){
          if (output != "[") {            output += ',';          }
          output += "{\"type\":\"";
          output += (file.isDirectory()) ? "dir" : "file";
          output += "\",\"name\":\"";
          output += String(file.name()).substring(1);
          output += "\"}";
          file = root.openNextFile();
      }
  }
  output += "]";
  server.send(200, "text/json", output);
}
