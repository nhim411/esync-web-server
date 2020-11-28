// Replace with your network credentials
const char *ssid = "Hoa Nam";
const char *password = "19941994";
//variable MODBUS
#define RXD1 4
#define TXD1 2
String baud = "9600";
String data_bit = "8";
String stop_bit = "1";
String parity = "no";
String instrument = "1";
byte ByteArray1[250];
byte ByteArray2[250];
byte msg_sv[] = {0x01, 0x03, 0x01, 0x03, 0x00, 0x01, 0x75, 0xF6};// response: 01 03 02 00 1c b9 8d
byte msg_pv[] = {0x01, 0x03, 0x01, 0x00, 0x00, 0x01, 0x85, 0xF6};// response: 01 03 02 00 0d 79 81
int i, j;
int len1 = 8;
int len2 = 8;
//variable FTP
char ftpserver[] = "103.97.125.251";
char ftp_user[]   = "nguyendinhdung@lhnam.net";
char ftp_pass[]   = "nguyendinhdung";
String ftp_server = "103.97.125.251";
String user1 = "nguyendinhdung@lhnam.net";
String pass1 = "nguyendinhdung";
String user2 = "nguyendinhdung@lhnam.net";
String pass2 = "nguyendinhdung";
String download_ftp = "";
String time_update_ftp = "1";
String time_update_sd = "1";
String isRun = "off";
String choose_user = "1";
String sv = "99";
String pv = "100";
//  setIntervel
unsigned long previousMillis_ftp = 0;
unsigned long interval_ftp = 5000;
unsigned long previousMillis_sd = 0;
unsigned long interval_sd = 5000;
unsigned long previousMillis_1s = 0;
unsigned long interval_1s = 1000;
