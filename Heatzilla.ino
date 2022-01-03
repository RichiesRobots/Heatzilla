#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <Adafruit_BMP280.h>
#include <NewRemoteReceiver.h>
#include <NewRemoteTransmitter.h>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define API "YourAPICode"
#define PORT 80
#define CENTRAL_SERVER "192.168.178.23"
#define ESP32_CONFIG_VALUES "Path to getESP32ValuesFromDevice file"
#define SET_ESP32_CONFIG_VALUES "Path to getSetESP32Values file"
#define host "esp32"
#define WIFI_SSID "YourWIFIName"
#define WIFI_PASSWORD "YourWifiPassword"

int HeatCounter = 0;

HTTPClient http;
WiFiClient client;

bool HeaterOn = false;

WebServer server(80);

Adafruit_BMP280 bmp; // I2C Interface

Adafruit_SSD1306 displayTemp(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

String readString = "";
String tempHold = "";

/*
   Login page and web page 
*/

const char* loginIndex =
  "<form name='loginForm'>"
  "<table width='20%' bgcolor='A09F9F' align='center'>"
  "<tr>"
  "<td colspan=2>"
  "<center><font size=4><b>Heaterzilla</b></font></center>"
  "<br>"
  "</td>"
  "<br>"
  "<br>"
  "</tr>"
  "<tr>"
  "<td>Username:</td>"
  "<td><input type='text' size=25 name='userid'><br></td>"
  "</tr>"
  "<br>"
  "<br>"
  "<tr>"
  "<td>Password:</td>"
  "<td><input type='Password' size=25 name='pwd'><br></td>"
  "<br>"
  "<br>"
  "</tr>"
  "<tr>"
  "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
  "</tr>"
  "</table>"
  "</form>"
  "<script>"
  "function check(form)"
  "{"
  "if(form.userid.value=='admin' && form.pwd.value=='Password01')"
  "{"
  "window.open('/serverIndex')"
  "}"
  "else"
  "{"
  " alert('Error Password or Username')/*displays error message*/"
  "}"
  "}"
  "</script>";

/*
   Server Index Page
*/

const char* serverIndex =
  "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
  "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
  "<input type='file' name='update'>"
  "<input type='submit' value='Update'>"
  "</form>"
  "<div id='prg'>progress: 0%</div>"
  "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')"
  "},"
  "error: function (a, b, c) {"
  "}"
  "});"
  "});"
  "</script>";

String urlEncode(String str) {
  String new_str = "";
  char c;
  int ic;
  const char* chars = str.c_str();
  char bufHex[10];
  int len = strlen(chars);

  for (int i = 0; i < len; i++) {
    c = chars[i];
    ic = c;
    // uncomment this if you want to encode spaces with +
    /*if (c==' ') new_str += '+';
      else */if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') new_str += c;
    else {
      sprintf(bufHex, "%X", c);
      if (ic < 16)
        new_str += "%0";
      else
        new_str += "%";
      new_str += bufHex;
    }
  }
  return new_str;
}

String urlDecode(String str) {
  String ret;
  char ch;
  int i, ii, len = str.length();

  for (i = 0; i < len; i++) {
    if (str[i] != '%') {
      if (str[i] == '+')
        ret += ' ';
      else
        ret += str[i];
    } else {
      sscanf(str.substring(i + 1, 2).c_str(), "%x", &ii);
      ch = static_cast<char>(ii);
      ret += ch;
      i = i + 2;
    }
  }
  return ret;
}

void initializeWiFi(String initialMessage) {
  displayTemp.clearDisplay();
  displayTemp.display();

  displayTemp.setCursor(1, 0);
  displayTemp.setTextSize(1);
  displayTemp.setTextColor(WHITE);


  displayTemp.print(initialMessage);
  Serial.println(initialMessage);

  displayTemp.setCursor(1, 14);
  displayTemp.setTextSize(1);
  displayTemp.setTextColor(WHITE);

  displayTemp.println(WIFI_SSID);
  displayTemp.display();

  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    displayTemp.print(".");
    displayTemp.display();
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  displayTemp.clearDisplay();
  displayTemp.display();
  displayTemp.setCursor(1, 1);
  displayTemp.setTextSize(1);
  displayTemp.setTextColor(WHITE);
  displayTemp.println("WiFi connected.");
  displayTemp.display();

  delay(2000);


  displayTemp.setCursor(1, 12);
  displayTemp.setTextSize(1);
  displayTemp.setTextColor(WHITE);
  displayTemp.println("IP Address");
  displayTemp.setCursor(1, 24);
  displayTemp.setTextSize(1);
  displayTemp.setTextColor(WHITE);
  displayTemp.println(WiFi.localIP());
  displayTemp.display();

  setConfigValue("OfficeHeatzillaIP", WiFi.localIP().toString().c_str());

  delay(10000);

  displayTemp.clearDisplay();
  displayTemp.display();
}

void checkReconnectWifi(){

  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  if ((WiFi.status() != WL_CONNECTED)) {
   
    WiFi.disconnect();
    initializeWiFi("Reconnecting To ");
 }
}

String getConfigValue(String value) {

  checkReconnectWifi(); 

  readString = "";

  if (client.connect(CENTRAL_SERVER, PORT)) {  //starts client connection, checks for connection
    client.print("GET ");
    client.print(ESP32_CONFIG_VALUES);
    client.print("?API=");
    client.print(API);
    client.print("&calltype=");
    client.print("GetValue");
    client.print("&valueName=");
    client.print(value);
    client.println(" HTTP/1.1"); //download text
    client.print("Host: ");
    client.println(CENTRAL_SERVER);
    client.println("Connection: close");  //close 1.1 persistent connection
    client.println(); //end of get request
  }
  else {
    Serial.println("connection failed"); //error message if no client connect
    Serial.println();
  }

  while (client.connected() && !client.available()) delay(1); //waits for data
  while (client.connected() || client.available()) { //connected or data available
    char c = client.read(); //gets byte from ethernet buffer
    readString += c; //places captured byte in readString

  }


  return readString.substring(readString.indexOf(value + ":") + value.length() + 1);
  client.stop();

}


void setConfigValue(String valueName, String value) {

  checkReconnectWifi();

  if (client.connect(CENTRAL_SERVER, PORT)) {  //starts client connection, checks for connection
    client.print("GET ");
    client.print(SET_ESP32_CONFIG_VALUES);
    client.print("?calltype=");
    client.print("SetValue");
    client.print("&valueName=");
    client.print(valueName);
    client.print("&Value=");
    client.print(urlEncode(value));
    client.println(" HTTP/1.1"); //download text
    client.print("Host: ");
    client.println(CENTRAL_SERVER);
    client.println("Connection: close");  //close 1.1 persistent connection
    client.println(); //end of get request

    client.stop();
  }
  else {
    Serial.println("connection failed"); //error message if no client connect
    Serial.println();
  }

}

void I2CSwitcher(uint8_t bus)
{
  Wire.beginTransmission(0x70);  // TCA9548A address is 0x70
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
}

NewRemoteTransmitter transmitter(18898426, 4, 272);

void setup() {
  Serial.begin(115200);

  // Start Wire library for I2C
  Wire.begin();


  I2CSwitcher(0);

  /*init display*/
  if (!displayTemp.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  I2CSwitcher(3);

  if (!bmp.begin()) {
    I2CSwitcher(0);
    Serial.println(F("Could not find a valid BMP280 sensor"));
    displayTemp.setCursor(1, 28);
    displayTemp.setTextSize(1);
    displayTemp.setTextColor(WHITE);
    displayTemp.println("BMP280 Sensor Error");
    displayTemp.display();
    I2CSwitcher(3);
    while (1);
  }

  I2CSwitcher(0);

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  I2CSwitcher(0);


initializeWiFi("Connecting to "); 

 
  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
  //starts in low so set to high at start

}

void loop() {

  server.handleClient();

  I2CSwitcher(3);

  tempHold = bmp.readTemperature();

  I2CSwitcher(0);

  displayTemp.clearDisplay();
  displayTemp.display();


  displayTemp.setCursor(32, 4);
  displayTemp.setTextSize(1.5);
  displayTemp.setTextColor(WHITE);
  displayTemp.println("Temperature");
  displayTemp.display();

  displayTemp.setCursor(32, 32);
  displayTemp.setTextSize(2);
  displayTemp.setTextColor(WHITE);
  displayTemp.print(tempHold);
  displayTemp.print("c");

  displayTemp.display();

  setConfigValue("OfficeCurrentTemperature",tempHold);
    
  if (getConfigValue("OfficeWantedTemperature").toDouble() > (tempHold.toDouble()) && getConfigValue("OfficeHeatzillaEnabled")=="True") {
    setConfigValue("OfficeHeatzillaStatus","Heater On");
    
    Serial.println("Heater On");

    setConfigValue("OfficeCurrentTemperature",tempHold);
   
   while((getConfigValue("OfficeWantedTemperature").toDouble() + 0.5) > tempHold.toDouble() && getConfigValue("OfficeHeatzillaEnabled")=="True"){

      if(!HeaterOn || HeatCounter > 120){ 
         transmitter.sendUnit(1, 1);
         HeaterOn = true;
         HeatCounter = 0;
      }
      
      setConfigValue("OfficeHeatzillaStatus","Heating Up");
      setConfigValue("OfficeCurrentTemperature",tempHold);
      Serial.println("Heating Up");
      I2CSwitcher(3);

      tempHold = bmp.readTemperature();
    
      I2CSwitcher(0);

      Serial.println(getConfigValue("OfficeWantedTemperature").toDouble() + 0.5);
      Serial.println(tempHold.toDouble());
    
      displayTemp.clearDisplay();
      displayTemp.display();
    
    
      displayTemp.setCursor(32, 4);
      displayTemp.setTextSize(1.5);
      displayTemp.setTextColor(WHITE);
      displayTemp.println("Temperature");
    
      displayTemp.display();
      displayTemp.setCursor(32, 32);
      displayTemp.setTextSize(2);
      displayTemp.setTextColor(WHITE);
      displayTemp.print(tempHold);
      displayTemp.print("c");
    
      displayTemp.display();
        delay(1000);
        HeatCounter++;
   }
    
  }
  else if (getConfigValue("OfficeWantedTemperature").toDouble() < tempHold.toDouble() || getConfigValue("OfficeHeatzillaEnabled")=="False") {
    setConfigValue("OfficeHeatzillaStatus","Heater Off");
    Serial.println("Heater Off");
    if(HeaterOn){
       transmitter.sendUnit(1, 0);
       HeaterOn = false;
    }   
    setConfigValue("OfficeCurrentTemperature",tempHold);
  }
  HeatCounter = 0;
  delay(1000);
}
