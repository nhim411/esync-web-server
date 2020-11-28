// Replace with your network credentials
const char *ssid = "Hoa Nam";
const char *password = "19941994";
//variable MODBUS
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
String fileContent = "";
//  setIntervel
unsigned long previousMillis_ftp = 0;
unsigned long interval_ftp = 5000;
unsigned long previousMillis_sd = 0;
unsigned long interval_sd = 5000;
unsigned long previousMillis_1s = 0;
unsigned long interval_1s = 1000;
bool openFile = false;

#ifdef ETH_CLK_MODE
#undef ETH_CLK_MODE
#endif
#define ETH_CLK_MODE    ETH_CLOCK_GPIO0_IN
// Pin# of the enable signal for the external crystal oscillator (-1 to disable for internal APLL source)
#define ETH_POWER_PIN   -1

// Type of the Ethernet PHY (LAN8720 or TLK110)
#define ETH_TYPE        ETH_PHY_LAN8720

// I²C-address of Ethernet PHY (0 or 1 for LAN8720, 31 for TLK110)
#define ETH_ADDR        1

// Pin Definitions
//SD Card
#define CLK_PIN 14
#define MISO_PIN 12
#define MOSI_PIN 13
#define SDFILE_PIN_CS  15
// Pin# of the I²C clock,IO signal for the Ethernet PHY, LAN8720
#define ETH_MDC_PIN     23
#define ETH_MDIO_PIN    18
//Pin# of Serial1
#define RXD1_PIN 4
#define TXD1_PIN 2
//Pin# of Serial2
#define RXD2_PIN 16
#define TXD2_PIN 17
//Pin# I2C
#define SDA_PIN 32
#define SCL_PIN 33
//Pin# of the button
#define PUSHBUTTON_1_PIN_1  5
#define PUSHBUTTON_2_PIN_2  34
#define PUSHBUTTON_3_PIN_3  35

#define LCD_ADDRESS 0x3F
//#define LCD_ADDRESS 0x27
// Define LCD characteristics
#define LCD_ROWS 2
#define LCD_COLUMNS 16
#define SCROLL_DELAY 150
#define BACKLIGHT 255
// object initialization
