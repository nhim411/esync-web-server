String readFile(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    Serial.println("- empty file or failed to open file");
    return String();
  }
  Serial.println("- read from file:");
  String fileContent;
  while (file.available()) {
    fileContent += String((char)file.read());
  }
  Serial.println(fileContent);
  return fileContent;
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
}

// Replaces placeholder with stored values
String processor(const String &var)
{
  //Serial.println(var);
  if (var == "baud")
  {
    return readFile(SPIFFS, "/baud.txt");
  }
  if (var == "data_bit")
  {
    return readFile(SPIFFS, "/data_bit.txt");
  }
  if (var == "stop_bit")
  {
    return readFile(SPIFFS, "/stop_bit.txt");
  }
  if (var == "parity")
  {
    return readFile(SPIFFS, "/parity.txt");
  }
  if (var == "instrument")
  {
    return readFile(SPIFFS, "/instrument.txt");
  }
  if (var == "ftp_server")
  {
    return readFile(SPIFFS, "/ftp_server.txt");
  }
  if (var == "user1")
  {
    return readFile(SPIFFS, "/user1.txt");
  }
  if (var == "user2")
  {
    return readFile(SPIFFS, "/user2.txt");
  }
  if (var == "time_update_ftp")
  {
    return readFile(SPIFFS, "/time_update_ftp.txt");
  }
  if (var == "time_update_sd")
  {
    return readFile(SPIFFS, "/time_update_sd.txt");
  }
  if (var == "sv")
  {
    return sv;
  }
  if (var == "pv")
  {
    return pv;
  }
  if (var == "DOWNLOAD")
  {
    return download_ftp;
  }
  return String();
}
