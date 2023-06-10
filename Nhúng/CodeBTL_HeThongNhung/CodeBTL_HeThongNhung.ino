#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

//thu vien gui email
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP_Mail_Client.h>

#define WIFI_SSID "VIETTEL_QUY"
#define WIFI_PASSWORD "123456789"
#define SMTP_server "smtp.gmail.com"
#define SMTP_Port 465
#define sender_email "ndq8112001@gmail.com"
#define sender_password "mglvccncpkgqntva"
#define Recipient_email "ndq.domin@gmail.com"

SMTPSession smtp;

//setup màn OLED
#define OLED_RESET -1
#define SCREEN_WIDTH 128 // chiều ngang màn hình OLED
#define SCREEN_HEIGHT 64 // chiều dọc màn OLED
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

SoftwareSerial mySerial(13, 15);
int fID = 0; // lấy id vân tay
String name; // lấy tên sinh viên
String msv; //lấy mã sv

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup(){
  //Fingerprint sensor module setup
  Serial.begin(9600);
  // set the data rate for the sensor serial port
  finger.begin(57600);
  
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } 
  else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }
  //OLED display setup
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  //displays main screen
  setManHinhChinh();
  //khoi tao wifi
  Serial.println();
  Serial.print("Connecting...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  smtp.debug(1);
}

String getName(int fingerID) {
  String names[] = {"Nguyen Dinh Quan", "Nguyen Hoang Viet", "Phan Vuong Bao", "Nguyen Xuan Khoa"}; //Mảng lưu trữ thông tin tên sinh viên
  if (fingerID < 1 || fingerID > sizeof(names)) {
    return "Unknown"; //Trả về "Unknown" nếu không tìm thấy ID của sinh viên trong mảng
  } else {
    return names[fingerID - 1]; //Trả về tên của sinh viên tương ứng với ID của vân tay
  }
}
String getMSV(int fingerID) {
  String msv[] = {"B19DCCN528", "B19DCCN714", "B19DCCN096", "B19DCCN069"};
  if (fingerID < 1 || fingerID > sizeof(msv)) {
    return "Unknown"; 
  } else {
    return msv[fingerID - 1];
  }
}

void loop(){
  fID = getFingerprintID();
  name = getName(fID);
  msv = getMSV(fID);
  setManHinhChinh();
  if(fID == -1){
    setManHinhChinh();
  }
  else if(fID == 0){
    hienThiWrongFingerprint();
  }
  else{
    hienThiDiemDanh(name, msv);
    GuiEmail(name, msv);
  }
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintID() {
  int id = -1;
  if(finger.getImage() == FINGERPRINT_OK && finger.image2Tz() == FINGERPRINT_OK){
    int result = finger.fingerFastSearch();
    if (result == FINGERPRINT_OK) {
      id = finger.fingerID;
      Serial.print("Found ID #"); 
      Serial.print(id); 
      Serial.print(" with confidence of "); 
      Serial.println(finger.confidence);
    } else if (result == FINGERPRINT_NOTFOUND) {
      Serial.println("Fingerprint not found");
      id = 0;
    }
  }
  return id;
}
void hienThiDiemDanh(String name, String msv){
  delay(50);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1.5);
  display.setCursor(15,10);
  display.print(name);
  display.setCursor(15,25);
  display.print(msv);
  display.setCursor(15,40);
  display.print("Da Diem Danh!"); 
  display.display();
  delay(5000);
}
void hienThiWrongFingerprint(){
  delay(50);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1.5);
  display.setCursor(15,25);
  display.print("Van tay khong khop"); 
  display.display();
  delay(3000);
}

void setManHinhChinh(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(15,20);
  display.println("Hay Nhap Van Tay !");
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(52,35);
  display.println("...");  
  display.display();
  delay(0);
}

// gui email
void GuiEmail(String name, String msv){
  ESP_Mail_Session session;
  session.server.host_name = SMTP_server ;
  session.server.port = SMTP_Port;
  session.login.email = sender_email;
  session.login.password = sender_password;
  session.login.user_domain = "";

  //khoi tao tin nhan
  SMTP_Message message;

  message.sender.name = "QUẢN LÝ SINH VIÊN";
  message.sender.email = sender_email;
  message.subject = "Thông Báo Điểm Danh";
  message.addRecipient("Quan",Recipient_email);

  String textMsg = name + " - " + msv + " đã điểm danh!";
  message.text.content = textMsg.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  
  //gửi tin nhắn đến email giáo viên
  if (!smtp.connect(&session))
    return;
  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());
}
