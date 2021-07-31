
void handleRoot(){
  if (handleFileRead("/index.htm")) { return; }
  String sa; String pg;
  pg+=F(
    "<html>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<style>\n"
    " td{text-align:center;}\n"
    " input{border-radius:4px;border:solid 1px;}\n"
    " a{text-decoration:none;}\n"
    "</style>\n"
    "<script>\n"
    "function iniciar(){document.body.style.zoom='150%';\n"
    " sto=setInterval(loadjs,1000);}\n"
    "function loadjs(){client.open('GET','js?fields=time',true);client.send();}\n"
    "var client=new XMLHttpRequest();\n"
    "client.onreadystatechange=function(){\n"
    " if(this.readyState==4 && this.status==200){\n"
    "  myObj=JSON.parse(this.responseText);\n"
    "  if(typeof myObj.time!='undefined'){\n"
    "   t=myObj.time+(60*60*3);\n"
    "   d=new Date(t*1000);\n"
    "   datahora.innerHTML=d.toLocaleString();}\n"
    " }\n"
    "}\n"
    "</script>\n"
    "<body onload='iniciar();'><div align=center>"
    "<table width=200 cellspacing=4>\n"
    "<tr><td><b>"); pg+=DeviceID; pg+=F("</b></td></tr>"
    "<tr><td id='datahora'>datahora</td></tr>");
  if(fsok){
    File root=SPIFFS.open("/");
    if(root.isDirectory()){
      pg+=F(
        "<tr><td style='border:solid 1px silver;border-radius:4px;'>"
        "<table align=center><tr><td style='text-align:left;width:120px;'><ul style='margin:0;'>");
      File file=root.openNextFile();
      while(file){
        sa=file.name(); sa=sa.substring(1); sa.replace(".gz","");
        String sb=sa.substring(0,sa.indexOf(".htm"));
        if(sa.indexOf(".htm")>0){ pg+="<li><a href='"+sa+"'>"+sb+"</a></li>"; }
        file = root.openNextFile();
      }
      pg+=F("</ul></td></tr></table></td></tr>");
    }
  }
  pg+=F(
    "<tr><td>File System: ");
  if(fsok){ pg+="<a href='/upload'><input type=button value=upload></a>"; }else{ pg+="<a href='/fsformat'><input type=button value=Formatar></a>"; }
  pg+=F(
    "</td></tr>"
    "<tr><td><a href='/update'><input type=button value=update></a>\n"
    "<a href='/reset'><input type=button value=reset></a></td></tr>"
    "<tr><td style='font:12px Arial;'>Versao: "); pg+=Versao; pg+=F("</a></td></tr>"
    "</table>"
    "</div></body></html>"
    );
  server.send(200, "text/html", pg);
}
