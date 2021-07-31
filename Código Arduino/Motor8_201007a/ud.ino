
#include <Update.h>

void update_setup(){
    server.on("/update", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      String pg;
      pg=F(
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<style>\n"
        " .custom-file-input{border:solid 1px silver;border-radius:4px;}"
        " .custom-file-input::-webkit-file-upload-button {visibility:hidden;width:0;border-radius:4px;}\n"
        " td{text-align:center;}"
        "</style>\n"
        "<div align=center><table>\n"
        "<tr><td><b>Update (Firmware)</b></td></tr>\n"
        "<form method='POST' action='/update' enctype='multipart/form-data'>"
        "<tr><td><input class='custom-file-input' type='file' name='update'>\n"
        "</td></tr>\n"
        "<tr><td><input type='submit' value='Update'></form></td></tr>"
        "</table></div>"
      );
      server.send(200, "text/html", pg);
    });
    server.on("/update", HTTP_POST, []() {
      server.sendHeader("Connection", "close");
      String pg;
      pg=F(
        "<html>"
        "<meta http-equiv='refresh' content='10; url=/'>"
        "<script>"
        "var cnt=0;"
        "function iniciar(){setInterval(timer,1000);};"
        "function timer(){ cnt++; span01.innerHTML+='.'; }"
        "</script>"
        "<body onload='iniciar();'>"
        "<span id='span01'>Reiniciando</span>"
        "</body>"
        "</html>"
      );
      server.send(200, "text/html", (Update.hasError()) ? "FAIL" : pg);
      server.handleClient();
      //delay(100); ESP.restart();
    },[]() {
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        addLog("update "+upload.filename);
        if (!Update.begin()) {} //start with max available size
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {}
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)){ rst_cnt=20; } //true to set the size to the current progress
      }
    } );
}
