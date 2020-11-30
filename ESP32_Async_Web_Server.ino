#include "libraries.h"
#include "variables.h"
#include "functions.h"

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
LiquidCrystal_PCF8574 lcd;
SPIClass spiSD(HSPI);
RTC_DS3231 rtcDS;
File sdFile;
Button pushButton_1(PUSHBUTTON_1_PIN_1);
Button pushButton_2(PUSHBUTTON_2_PIN_2);
Button pushButton_3(PUSHBUTTON_3_PIN_3);

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

void setup()
{
  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.begin(LCD_COLUMNS, LCD_ROWS, LCD_ADDRESS, BACKLIGHT);
  lcd.clear();                          // Clear LCD screen.
  //lcdI2C.selectLine(1);
  delay(100);
  lcd.setCursor(0, 0);
  lcd.print("LAN: -SD: -RTC: ");                   // Print print String to LCD on first line
  //lcdI2C.selectLine(2);                    // Set cursor at the begining of line 2
  lcd.setCursor(0, 1);
  lcd.print("SIM: -WIFI: -S:");                     // Print print String to LCD on second line
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, RXD1_PIN, TXD1_PIN);
  Serial2.begin(9600, SERIAL_8N1, RXD2_PIN, TXD2_PIN);
  // Initialize SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  else {
    //update data from SPIFFS
    baud = readFile(SPIFFS, "/baud.txt");
    data_bit = readFile(SPIFFS, "/data_bit.txt");
    stop_bit = readFile(SPIFFS, "/stop_bit.txt");
    parity = readFile(SPIFFS, "/parity.txt");
    instrument = readFile(SPIFFS, "/instrument.txt");
    set_sv = readFile(SPIFFS, "/set_sv.txt");
    ftp_server = readFile(SPIFFS, "/ftp_server.txt");
    user1 = readFile(SPIFFS, "/user1.txt");
    user2 = readFile(SPIFFS, "/user2.txt");
    pass1 = readFile(SPIFFS, "/pass1.txt");
    pass2 = readFile(SPIFFS, "/pass2.txt");
    time_update_ftp = readFile(SPIFFS, "/time_update_ftp.txt");
    time_update_sd = readFile(SPIFFS, "/time_update_sd.txt");
    set_sv = readFile(SPIFFS, "/set_sv.txt");
  }
  //// Access Point Mode
  //  WiFi.softAP("ESP32-Access-Point", "123456789");
  //  IPAddress IP = WiFi.softAPIP();
  //  Serial.print("AP IP address: ");
  //  Serial.println(IP);
  // Connect to Wi-File  Wire.begin(SDA_PIN, SCL_PIN);
  // initialize the lcd
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi..");
  uint32_t t = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    lcd.setCursor(11, 1);
    lcd.print("E");
    bool justOne = false;
    if ( ((millis() - t) > 5000) && (justOne == false) )  //if not connect try connect LAN
    {
      justOne = true;
      Serial.println("Retry");
      WiFi.mode(WIFI_OFF);        //stop wifi
      initEthernet();
      lcd.setCursor(4, 0);
      lcd.print("A");
      break;
    }
  }
  if (WiFi.status() == WL_CONNECTED) {
    // Print ESP32 Local IP Address
    Serial.println(WiFi.localIP());
    lcd.setCursor(11, 1);
    lcd.print("A");
  }
  if (!MDNS.begin("esp32"))
  {
    Serial.println("Error starting mDNS");
    return;
  }

  // Route for root / web page
  lcd.setCursor(15, 1);
  lcd.print("A");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html");
  });
  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });
  server.on("/modbus", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/modbus.html", String(), false, processor);
  });
  server.on("/ftp", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/ftp.html", String(), false, processor);
  });
  server.on("/device", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/device.html", String(), false, processor);
  });
  server.on("/download", HTTP_GET, [](AsyncWebServerRequest * request) {
    String user, pass;
    if (choose_user == "1") {
      user = user1;
      pass = pass1;
    } else {
      user = user2;
      pass = pass2;
    }
    //update server
    int ftp_len = ftp_server.length() + 1;
    char ftp_array[ftp_len];
    ftp_server.toCharArray(ftp_array, ftp_len);
    //update user
    int user_len = user.length() + 1;
    char user_array[user_len];
    user.toCharArray(user_array, user_len);
    //update pass
    int pass_len = pass.length() + 1;
    char pass_array[pass_len];
    pass.toCharArray(pass_array, pass_len);
    ESP32_FTPClient ftp (ftp_array, user_array, pass_array);
    ftp.OpenConnection();
    ftp.ChangeWorkDir("/");
    download_ftp = "";
    ftp.InitFile("Type A");
    ftp.DownloadString("420ma.txt", download_ftp);
    Serial.println("The file content is: ");
    Serial.println(download_ftp);
    ftp.CloseConnection();
    request->send(SPIFFS, "/download.html", String(), false, processor);
  });
  server.on("/show", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/show.html", String(), false, processor);
  });
  server.on("/download", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/download.html", String(), false, processor);
  });
  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/run.html", String(), false, processor);
  });
  server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest * request) {
    ESP.restart();
  });
  // Send a GET request to /ftp-config?server=<server-id>&user1=<user1>&pass1=<pass1>&user2=<user2>&pass2=<pass2>&time=<time>
  server.on("/modbus-config", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasParam("baud"))
    {
      baud = request->getParam("baud")->value();
      writeFile(SPIFFS, "/baud.txt", baud.c_str());
      Serial.print("Baud: ");
      Serial.println(baud);
    }
    if (request->hasParam("data_bit"))
    {
      data_bit = request->getParam("data_bit")->value();
      writeFile(SPIFFS, "/data_bit.txt", data_bit.c_str());
      Serial.print("Bit data: ");
      Serial.println(data_bit);
    }
    if (request->hasParam("stop_bit"))
    {
      stop_bit = request->getParam("stop_bit")->value();
      writeFile(SPIFFS, "/stop_bit.txt", stop_bit.c_str());
      Serial.print("Bit Stop: ");
      Serial.println(stop_bit);
    }
    if (request->hasParam("parity"))
    {
      instrument = request->getParam("parity")->value();
      writeFile(SPIFFS, "/parity.txt", parity.c_str());
      Serial.print("Parity: ");
      Serial.println(parity);
    }
    if (request->hasParam("instrument"))
    {
      instrument = request->getParam("instrument")->value();
      writeFile(SPIFFS, "/instrument.txt", instrument.c_str());
      Serial.print("instrument: ");
      Serial.println(instrument);
    }
    if (request->hasParam("set_sv"))
    {
      instrument = request->getParam("set_sv")->value();
      writeFile(SPIFFS, "/set_sv.txt", set_sv.c_str());
      Serial.print("Set SV Value: ");
      Serial.println(set_sv);
    }
    request->send(SPIFFS, "/modbus.html", String(), false, processor);
  });
  // Send a GET request to /ftp-config?server=<server-id>&user1=<user1>&pass1=<pass1>&user2=<user2>&pass2=<pass2>&time=<time>
  server.on("/ftp-config", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasParam("server"))
    {
      ftp_server = request->getParam("server")->value();
      writeFile(SPIFFS, "/ftp_server.txt", ftp_server.c_str());
      Serial.print("Server IP: ");
      Serial.println(ftp_server);
    }
    if (request->hasParam("user1"))
    {
      user1 = request->getParam("user1")->value();
      writeFile(SPIFFS, "/user1.txt", user1.c_str());
      Serial.print("Username 1: ");
      Serial.println(user1);
    }
    if (request->hasParam("pass1"))
    {
      pass1 = request->getParam("pass1")->value();
      writeFile(SPIFFS, "/pass1.txt", pass1.c_str());
      Serial.print("Password 1: ");
      Serial.println(pass1);
    }
    if (request->hasParam("user2"))
    {
      instrument = request->getParam("user2")->value();
      writeFile(SPIFFS, "/user2.txt", user2.c_str());
      Serial.print("Username 2: ");
      Serial.println(user2);
    }
    if (request->hasParam("pass2"))
    {
      pass2 = request->getParam("pass2")->value();
      writeFile(SPIFFS, "/pass2.txt", pass2.c_str());
      Serial.print("Password 2: ");
      Serial.println(pass2);
    }
    if (request->hasParam("time_update_ftp"))
    {
      time_update_ftp = request->getParam("time_update_ftp")->value();
      writeFile(SPIFFS, "/time_update_ftp.txt", time_update_ftp.c_str());
      Serial.print("Update time FTP: ");
      Serial.println(time_update_ftp);
    }

    request->send(SPIFFS, "/ftp.html", String(), false, processor);
  });

  //Route to device config
  server.on("/device-config", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasParam("time_update_sd"))
    {
      time_update_sd = request->getParam("time_update_sd")->value();
      writeFile(SPIFFS, "/time_update_sd.txt", time_update_sd.c_str());
      Serial.print("Time update SD: ");
      Serial.println(time_update_sd);
    }
    if (request->hasParam("sv"))
    {
      set_sv = request->getParam("sv")->value();
      writeFile(SPIFFS, "/set_sv.txt", set_sv.c_str());
      Serial.print("Set SV value: ");
      Serial.println(set_sv);// response: 01 03 02 00 1c b9 8d
      setDone = true;
      request->send(SPIFFS, "/device.html", String(), false, processor);
      //01 06 0001 0258 D890 set 600 do
    }
  });

  server.on("/setting", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasParam("run"))
    {
      isRun = request->getParam("run")->value();
      //writeFile(SPIFFS, "/run.txt", isRun.c_str());
      Serial.print("Run: ");
      Serial.println(isRun);
    }
    if (request->hasParam("choose-user"))
    {
      choose_user = request->getParam("choose-user")->value();
    }
    request->send(200, "text/plain", "OK");
  });
  server.on("/sv", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", sv.c_str());
  });
  server.on("/pv", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", pv.c_str());
  });
  // Start server
  server.onNotFound(notFound);
  server.begin();
  //------------------------------------------
  //  Wire.begin(SDA_PIN, SCL_PIN);
  //  // initialize the lcd
  //  lcdI2C.begin(LCD_COLUMNS, LCD_ROWS, LCD_ADDRESS, BACKLIGHT);
  //  lcdI2C.clear();                          // Clear LCD screen.
  //  lcd.setCursor(0, 0);
  //  lcdI2C.print("LAN: -SD: -RTC: ");                   // Print print String to LCD on first line
  //  //lcdI2C.selectLine(2);                    // Set cursor at the begining of line 2
  //  lcd.setCursor(0, 1);
  //  lcdI2C.print("SIM: -WIFI: ");                     // Print print String to LCD on second line

  pushButton_1.init();
  pushButton_2.init();
  pushButton_3.init();
  //RS3231
  if (! rtcDS.begin()) {
    Serial.println("Couldn't find RTC");
    lcd.setCursor(15, 0);
    lcd.print("E");
  } else {
    lcd.setCursor(15, 0);
    lcd.print("A");
  }
  if (rtcDS.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtcDS.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtcDS.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  spiSD.begin(CLK_PIN, MISO_PIN, MOSI_PIN, SDFILE_PIN_CS); //CLK,MISO,MOIS,SS
  pinMode(SDFILE_PIN_CS, OUTPUT);
  if (!SD.begin(SDFILE_PIN_CS, spiSD)) {
    Serial.println(F("Card failed, or not present"));
    lcd.setCursor(9, 0);
    lcd.print("E");
  } else {
    lcd.setCursor(9, 0);
    lcd.print("A");
  }
}

void loop()
{
  unsigned long currentMillis = millis();
  interval_ftp = time_update_ftp.toInt() * 10000; //60000
  interval_sd = time_update_sd.toInt() * 10000 + 1000;
  if (isRun == "on") {
    //upload file to server
    if ((currentMillis - previousMillis_ftp >= interval_ftp) && (openFile == false)) {
      openFile = true;
      sdFile = SD.open("/420ma.txt", FILE_READ);
      if (sdFile) {
        String fileContent;
        while (sdFile.available()) {
          fileContent += String((char)sdFile.read());
        }
        sdFile.close();
        Serial.println("READ FILE 420ma.txt:");
        Serial.println(fileContent);    String user, pass;
        if (choose_user == "1") {
          user = user1;
          pass = pass1;
        } else {
          user = user2;
          pass = pass2;
        }
        //update server
        int ftp_len = ftp_server.length() + 1;
        char ftp_array[ftp_len];
        ftp_server.toCharArray(ftp_array, ftp_len);
        //update user
        int user_len = user.length() + 1;
        char user_array[user_len];
        user.toCharArray(user_array, user_len);
        //update pass
        int pass_len = pass.length() + 1;
        char pass_array[pass_len];
        pass.toCharArray(pass_array, pass_len);
        ESP32_FTPClient ftp (ftp_array, user_array, pass_array);
        ftp.ChangeWorkDir("/");
        ftp.InitFile("Type A");
        ftp.NewFile("420ma.txt");
        ftp.Write(fileContent.c_str());
        ftp.CloseFile();
        ftp.OpenConnection();
        Serial.println("UPLOAD TO SERVER DONE");
      } else {
        Serial.println("error opening the 420ma.txt,UPLOAD FAIL");
      }
      previousMillis_ftp = currentMillis;
      openFile = false;
    }

    else {
      Serial.println("error opening the 420ma.txt,UPLOAD FAIL");
    }
    //write file to sd
    if ((currentMillis - previousMillis_sd >= interval_sd) && (openFile == false)) {
      //do something
      openFile = true;
      sdFile = SD.open("/420ma.txt", FILE_READ);
      if (sdFile) {
        fileContent = "";
        while (sdFile.available()) {
          fileContent += String((char)sdFile.read());
        }
        sdFile.close();
      }
      delay(100);
      sdFile = SD.open("/420ma.txt", FILE_WRITE);
      // if the file is available, write to it:
      if (sdFile) {
        DateTime now = rtcDS.now();
        String upload_data;
        upload_data = sv + "|" + pv + "|";
        upload_data += now.hour();
        upload_data += "|";
        upload_data += now.minute();
        upload_data += "|";
        upload_data += now.day();
        upload_data += "|";
        upload_data += now.month();
        upload_data += "|";
        upload_data += now.year();
        sdFile.print(fileContent);
        sdFile.println(upload_data);
        sdFile.close();
        Serial.println("Write to sd:");
        Serial.print(fileContent);
        Serial.println(upload_data);
      }    else {
        // if the file didn't open, print an error
        Serial.println(F("error opening file 420ma.txt, WRITE FAIL"));
      }
      previousMillis_sd = currentMillis;
      openFile = false;
    }
    //SV
    for (i = 0 ; i < 8 ; i++) {
      Serial1.write(msg_sv[i]);
    }
    int a = 0;
    while (Serial1.available())
    {
      ByteArray1[a] = Serial1.read();
      a++;
    }
    int Data1;
    Data1 = ByteArray1[3] * 256 + ByteArray1[4];
    sv = String(Data1);
    delay(100);
    //PV
    for (i = 0 ; i < 8 ; i++) {
      Serial1.write(msg_pv[i]);
    }
    int c = 0;
    while (Serial1.available())
    {
      ByteArray2[c] = Serial1.read();
      c++;
    }
    int Data2;
    Data2 = ByteArray2[3] * 256 + ByteArray2[4];
    pv = String(Data2);
    delay(100);
  }
  if (setDone == true) {
    setDone = false;
    String set_sv_hex = decToHex(set_sv.toInt());
    msg_set_sv[4] = StrtoByte(set_sv_hex.substring(0, 2));
    msg_set_sv[5] = StrtoByte(set_sv_hex.substring(2, 4));

    String msg_sv_string = "01060001";
    msg_sv_string = msg_sv_string + set_sv_hex;
    String calculated_crc = ModRTU_CRC(msg_sv_string);
    msg_set_sv[6] = StrtoByte(calculated_crc.substring(0, 2));
    msg_set_sv[7] = StrtoByte(calculated_crc.substring(2, 4));
    msg_sv_string = msg_sv_string + calculated_crc;
    Serial.print("Set sv value massage:");
    Serial.println(msg_sv_string);
    for (int i = 0 ; i < 8; i++) {
      Serial1.write(msg_set_sv[i]);
    }
  }
}
