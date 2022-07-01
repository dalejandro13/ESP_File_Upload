/*
los siguientes comandos se envian por la terminal de arduino para el microcontrolador ESP8266:

***leer toda la informacion del archivo:
[nombre del archivo].txt-r

***borrar archivo:
[nombre del archivo].txt-d

***leer archivo en determinadas lineas:
[nombre del archivo].txt[inicio,fina]-s
inicio: es un numero entero, inicio de lectura
final: es un numero entero, final de lectura
NOTA: cada linea debe terminar en '\n'(final de linea)

***subir archivo en FileSystem del ESP8266:
[nombre del archivo].txt-p
NOTA 1: el comando anterior es para darle el nombre del archivo
[texto que deseas subir guardar en el archivo]-w
NOTA 2: el texto que escribes debe terminar con -w
NOTA 3: debes ingresar los 2 comandos en ese orden para poder subir el archivo

***otra forma de subir un archivo al FileSystem del ESP8266
PASO 1: conectate a la red del ESP8266, la contraseña para conectarse es: FLX900-3
PASO 2: ingresa la siguiente url en el navegador web: http://192.168.2.1/upload
PASO 3: se abrira una pagina web y da clic en un boton que dice "Seleccionar archivo", luego se abre el explorador y selecciona el archivo txt deseado
PASO 4: luego da clic en el boton que dice "Upload File" y espera un instante hasta que termine el proceso
PASO 5: si todo sale bien aparecera un mensaje que dice "File was successfully uploaded", adermas aparece el nombre del archivo junto con su tamaño

***explorar los archivos almacenados
[espacio]-e
NOTA: el [espacio] es literal un espacio que debes dejar seguido de -e, aparecera una lista con todos los nombres de los archivos almacenados en el ESP8266

***borrar todos los archivos
[espacio]-f
NOTA: el [espacio] es literal un espacio que debes dejar seguido de -f, aparecera un mensaje el cual te indica que ha sido formateado exitosamente
*/

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ElegantOTA.h>
#include <LittleFS.h>
#include "Sys_Variables.h"
#include "CSS.h"

ESP8266WiFiMulti wifiMulti;
ESP8266WebServer server(80);
File UploadFile;
int blinking = 0, digital2 = 2;
String data = "", pathFile = "", infoFile = "";
String ssid = "RFAC-" + String(ESP.getChipId()), password = "FLX900-3", acum = "";

void explorerFiles(const char * dirname) {
  Serial.println("Explorando archivos");
  Dir root = LittleFS.openDir(dirname);
  while (root.next()) {
    //File file = root.openFile("r");
    Serial.println("archivo: " + root.fileName());
  }
}

void readFile(const char * path) {
  Serial.println("");
  Serial.printf("leyendo archivo: %s\n", path);
  File file = LittleFS.open(path, "r");
  if (!file) {
    Serial.println("no se puede leer el archivo");
    return;
  }
  Serial.println("se lee lo siguiente: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  Serial.println("");
  file.close();
}

void readCertainLine(const char * information){
  String fileName = "", line = "", initial = "", ending = "";
  const char * path;
  int count = 0, initialNum = 0, endingNum = 0, countComma = 0;
  String inform(information);

  int inxInitial = inform.indexOf("[");
  int inxMedium = inform.indexOf(",");
  int inxFinal = inform.indexOf("]");

  fileName = inform.substring(0, inxInitial);
  line = inform.substring(inxInitial, inxFinal + 1);

  //rutas_gmovil.txt[35,30]-s

  if(inform.indexOf("[") > 0 && inform.indexOf("]") > 0 && inform.indexOf(",") > 0){
    if(line.indexOf(",") > 0){
      countComma++;
    }

    if(countComma == 1){
      inxInitial = line.indexOf("[");
      inxMedium = line.indexOf(",");
      inxFinal = line.indexOf("]");
      initial = line.substring(inxInitial + 1, inxMedium);
      ending = line.substring(inxMedium + 1, inxFinal);
    
      initialNum = abs(initial.toInt());
      endingNum = abs(ending.toInt());
      if(initialNum > endingNum){
        int aux = initialNum;
        initialNum = endingNum;
        endingNum = aux;
      }
      else if(initialNum == endingNum){
        countComma = 0;
      }
    }
  }
  
  if(countComma == 1){
    path = fileName.c_str();
    Serial.printf("leyendo archivo: %s\n", path);
    File file = LittleFS.open(path, "r");
    if (!file) {
      Serial.println("no se puede leer el archivo");
      return;
    }
  
    if(line != ""){
      Serial.println("se lee lo siguiente: ");
      while (file.available()) {
        String readLine = file.readStringUntil('\n');
        count++;
        for(int i = initialNum; i < endingNum; i++){
          if(count >= initialNum && count <= endingNum){
            Serial.println(readLine);
            readLine = "";
            break;
          }
        }                
      }
    }
    else{
      Serial.println("te falta colocar las lineas a omitir");
    }
    file.close(); 
  }
  else{
    Serial.println("comando incorrecto, intentalo nuevamente");
  }
}

void writeFile(const char * path, const char * message) {
  Serial.printf("iniciando escritura en archivo: %s\n", path);
  File file = LittleFS.open(path, "w");
  if (!file) {
    Serial.println("no se puede abrir el archivo para escribir");
    return;
  }
  if (file.print(message)) {
    Serial.println("escritura exitosa");
  } 
  else {
    Serial.println("falla en la escritura");
  }
  delay(2000);
  file.close();
}

void appendFile(const char * path, const char * message) {
  Serial.printf("adjuntando archivo: %s\n", path);
  File file = LittleFS.open(path, "a");
  if (!file) {
    Serial.println("falla en abrir el archivo para adjuntar");
    return;
  }
  if (file.print(message)) {
    Serial.println("se adjunta mensaje");
  } 
  else {
    Serial.println("falla en adjuntar mensaje");
  }
  file.close();
}

void renameFile(const char * path1, const char * path2) {
  Serial.printf("archivos restantes %s to %s\n", path1, path2);
  if (LittleFS.rename(path1, path2)) {
    Serial.println("exito al renombrar");
    Serial.println("de " + String(path1) + " cambia a " + String(path2));
  } 
  else {
    Serial.println("falla en renombrar archivo");
  }
}

void deleteFile(const char * path) {
  Serial.printf("borrando archivo: %s\n", path);
  if (LittleFS.remove(path)) {
    Serial.println("archivo borrado");
  } 
  else {
    Serial.println("falla en borrado");
  }
}

void formatFileSystem() {
  if(!LittleFS.format()){
    Serial.println("el sistema de ficheros no fue formateado correctamente");
  }
  else{
    Serial.println("el sistema de ficheros ha sido formateado");
  }
  return;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setup(void){
  Serial.begin(9600);
  delay(2000);
  pinMode(digital2, OUTPUT);
  IPAddress ip(192,168,2,1);
  IPAddress NMask(255,255,255,0);
  WiFi.softAPConfig(ip, ip, NMask);

  WiFi.mode(WIFI_AP);
  while (!WiFi.softAP(ssid, password)) {
    delay(500);
  }
  WiFi.begin();

  Serial.println("montando sistema de ficheros");
  if (!LittleFS.begin()) {
    Serial.println("falla en el montaje");
    return;
  }
  
  server.on("/", [](){
    server.send(200, "text/plain", "MODO FABRICA RUTERO FLEXO LUMENS\r\n\r\nPara verificar el Serial, mande el comando FT-OK a 9600 baudios y espere la respuesta: FR-F:OK.\r\n\r\nPara actualizar, por favor ingrese a http://192.168.2.1/update");
  });
  server.on("/upload",   File_Upload);
  server.on("/fupload",  HTTP_POST,[](){ server.send(200);}, handleFileUpload);
  ElegantOTA.begin(&server); //Start ElegantOTA
  server.begin();
  Serial.println("arranca servidor http");
}

void loop(void){
  if(Serial.available()) {
    data = Serial.readString();
    if(data.indexOf("-w") > 0){
      infoFile = data.substring(0, data.length() - 2);
      infoFile.trim();
    }
    else if(data.indexOf("-p") > 0){
      if(data.indexOf(".txt") > 0){
        pathFile = data.substring(0, data.length() - 2);
        pathFile.trim();
      }
      else{
        Serial.println("te falta colocar la extension .txt");
      }
    }
  }

  if(data.indexOf("-d") > 0){
    data = data.substring(0, data.length() - 2);
    data.trim();
    deleteFile(data.c_str());
  }
  else if(data.indexOf("-s") > 0){
    data = data.substring(0, data.length() - 2);
    data.trim();
    readCertainLine(data.c_str());
  }
  else if(data.indexOf("-r") > 0){
    data = data.substring(0, data.length() - 2);
    data.trim();
    readFile(data.c_str());
  }
  else if(data.indexOf("-e") > 0){
    data = data.substring(0, data.length() - 2);
    data.trim();
    explorerFiles(data.c_str());
  }
  else if (data.indexOf("-f") > 0){
    formatFileSystem();
  }
  else if(infoFile != "" && pathFile != ""){
    writeFile(pathFile.c_str(), infoFile.c_str());
    infoFile = "";
    pathFile = "";
  }
  data = "";
  server.handleClient();
}

void File_Upload(){
  append_page_header();
  webpage += F("<title>Select File to Upload</title>"); 
  webpage += F("<FORM action='/fupload' method='post' enctype='multipart/form-data'>");
  webpage += F("<input class='buttons' style='width:40%' type='file' name='fupload' id = 'fupload' value=''><br>");
  webpage += F("<br><button class='buttons' style='width:10%' type='submit'>Upload File</button><br>");
  append_page_footer();
  server.send(200, "text/html",webpage);
}

void handleFileUpload(){
  HTTPUpload& uploadfile = server.upload();
  if(uploadfile.status == UPLOAD_FILE_START)
  {
    String filename = uploadfile.filename;    
    if(!filename.startsWith("/")) filename = "/"+filename;
    Serial.print("nombre del archivo: "); Serial.println(filename);
    LittleFS.remove(filename); //elimina el archivo que tenga el mismo nombre
    UploadFile = LittleFS.open(filename, "w");  //abre y escribe el archivo en el esp8266
    filename = String();
  }
  else if (uploadfile.status == UPLOAD_FILE_WRITE)
  {
    if(UploadFile) UploadFile.write(uploadfile.buf, uploadfile.currentSize); //escribe los bytes recibidos
  }
  else if (uploadfile.status == UPLOAD_FILE_END)
  {
    if(UploadFile) //si el archivo ha sido creado
    {
      UploadFile.close(); // Close the file again
      Serial.print("Upload Size: "); Serial.println(uploadfile.totalSize);
      webpage = "";
      append_page_header();
      webpage += F("<h3>File was successfully uploaded</h3>"); 
      webpage += F("<h2>Uploaded File Name: "); webpage += uploadfile.filename+"</h2>";
      webpage += F("<h2>File Size: "); webpage += file_size(uploadfile.totalSize) + "</h2><br>"; 
      append_page_footer();
      server.send(200,"text/html",webpage);
    }
    else
    {
      ReportCouldNotCreateFile("upload");
    }
  }
}

void SendHTML_Header(){
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate"); 
  server.sendHeader("Pragma", "no-cache"); 
  server.sendHeader("Expires", "-1"); 
  server.setContentLength(CONTENT_LENGTH_UNKNOWN); 
  server.send(200, "text/html", "");
  append_page_header();
  server.sendContent(webpage);
  webpage = "";
}

void SendHTML_Content(){
  server.sendContent(webpage);
  webpage = "";
}

void SendHTML_Stop(){
  server.sendContent("");
  server.client().stop();
}

void SelectInput(String heading1, String command, String arg_calling_name){
  SendHTML_Header();
  webpage += F("<h3>"); webpage += heading1 + "</h3>"; 
  webpage += F("<FORM action='/"); webpage += command + "' method='post'>";
  webpage += F("<input type='text' name='"); webpage += arg_calling_name; webpage += F("' value=''><br>");
  webpage += F("<type='submit' name='"); webpage += arg_calling_name; webpage += F("' value=''><br>");
  webpage += F("<a href='/'>[Back]</a>");
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

void ReportSDNotPresent(){
  SendHTML_Header();
  webpage += F("<h3>No SD Card present</h3>"); 
  webpage += F("<a href='/'>[Back]</a><br><br>");
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

void ReportFileNotPresent(String target){
  SendHTML_Header();
  webpage += F("<h3>File does not exist</h3>"); 
  webpage += F("<a href='/"); webpage += target + "'>[Back]</a><br><br>";
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

void ReportCouldNotCreateFile(String target){
  SendHTML_Header();
  webpage += F("<h3>Could Not Create Uploaded File (write-protected?)</h3>"); 
  webpage += F("<a href='/"); webpage += target + "'>[Back]</a><br><br>";
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

String file_size(int bytes){
  String fsize = "";
  if (bytes < 1024)                 fsize = String(bytes)+" B";
  else if(bytes < (1024*1024))      fsize = String(bytes/1024.0,3)+" KB";
  else if(bytes < (1024*1024*1024)) fsize = String(bytes/1024.0/1024.0,3)+" MB";
  else                              fsize = String(bytes/1024.0/1024.0/1024.0,3)+" GB";
  return fsize;
}
