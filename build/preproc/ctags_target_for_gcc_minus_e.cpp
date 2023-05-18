# 1 "/home/eduard/programming/SoilUP/SoilUP.ino"
# 1 "/home/eduard/programming/SoilUP/SoilUP.ino"
# 2 "/home/eduard/programming/SoilUP/SoilUP.ino" 2
# 3 "/home/eduard/programming/SoilUP/SoilUP.ino" 2
# 4 "/home/eduard/programming/SoilUP/SoilUP.ino" 2
# 5 "/home/eduard/programming/SoilUP/SoilUP.ino" 2
# 6 "/home/eduard/programming/SoilUP/SoilUP.ino" 2







const char* ssid = "WLAN-427794";
const char* password = "2249587511585811";






ESP8266WebServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 60 * 60 /* In seconds*/, 60 * 1000 /* In miliseconds*/);
File fsUploadFile;

void setup() {
  Serial.begin(9600);
  delay(10);
  Serial.println("\r\n");
  startSPIFFS();
  startServer();
  timeClient.begin();
  pinMode(D6, 0x01);
  delay(500);
}


const unsigned long intervalTemp = 60000;
unsigned long prevTemp = 0;
bool tmpRequested = false;
const unsigned long DS_delay = 750;

uint32_t timeUNIX = 0;

float moisture=0;
float expAlpha=0.99;
unsigned long cntA=0;
bool lightState = 0x0;

void loop() {
  unsigned long currentMillis = millis();
  cntA ++;

  // if(( cntA >= 100000 )) {
  //   moisture = expAlpha*moisture + (1-expAlpha) * analogRead(A0);
  //   cntA = 0;
  // }
    // lightState = !lightState;
    // digitalWrite(RELAY_LIGHT_PIN, lightState);
  timeClient.update();
  Serial.println(timeClient.getFormattedTime());
  Serial.println(timeClient.getHours());
  if(timeClient.getHours() >= 8 && timeClient.getHours() <= 18) {
    digitalWrite(D6, 0x1);
  }
  else {
    digitalWrite(D6, 0x0);
  }
  delay(1000);

  // Serial.printf("Appending temperature to file: %lu,", actualTime);
  // Serial.println(temp);
  // File tempLog = SPIFFS.open("/temp.csv", "a"); 
  // tempLog.print(actualTime);
  // tempLog.print(',');
  // tempLog.println(temp);
  // tempLog.close();

  // server.handleClient();                 
}


void startSPIFFS() {
  SPIFFS.begin();
  Serial.println("SPIFFS started. Contents:");
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.printf("\n");
  }
}

void startServer() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print('.');
  }
  server.on("/edit.html", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  }, handleFileUpload);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started.");
}


void handleNotFound() {
  if (!handleFileRead(server.uri())) {
    server.send(404, "text/plain", "404: File Not Found");
  }
}

bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);
  return false;
}

void handleFileUpload() { // upload a new file to the SPIFFS
  HTTPUpload& upload = server.upload();
  String path;
  if (upload.status == UPLOAD_FILE_START) {
    path = upload.filename;
    if (!path.startsWith("/")) path = "/" + path;
    if (!path.endsWith(".gz")) {
      String pathWithGz = path + ".gz";
      if (SPIFFS.exists(pathWithGz))
        SPIFFS.remove(pathWithGz);
    }
    Serial.print("handleFileUpload Name: "); Serial.println(path);
    fsUploadFile = SPIFFS.open(path, "w");
    path = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      fsUploadFile.close();
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      server.sendHeader("Location", "/success.html");
      server.send(303);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}


String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
}

String getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
