
#define BLYNK_TEMPLATE_ID "TMPL6FqjQ6Uw2"
#define BLYNK_DEVICE_NAME "CamBienVanTay"
#define BLYNK_AUTH_TOKEN "FKZ8XV0EzgOktBdLXLKmksFHBT7mtZQM"


#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
//blynk
#include <BlynkSimpleEsp8266.h>

//thu vien gui email
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP_Mail_Client.h>

#define WIFI_SSID "realme5Pro"
#define WIFI_PASSWORD "00000000"
#define SMTP_server "smtp.gmail.com"
#define SMTP_Port 465
#define sender_email "daotrongphuc50204002@gmail.com"
#define sender_password "tjudatogbltabrst"
#define Recipient_email "phucdt504@gmail.com"



char ssid[] = "realme5Pro";                                       //  Tên wifi
char pass[] = "00000000";                                   //  Mật khẩu wifi
char auth[] = "FKZ8XV0EzgOktBdLXLKmksFHBT7mtZQM";           //  Mã người dùng trên Blynk

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
int tbn_system = 0;           // Lấy dữ liệu điều kiển từ Blynk, 1- cho phép điều kiển, 0- tắt hệ thống điều kiển
int ttt_bt_system;
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

  Blynk.begin(auth, ssid, pass);    // Kết nối với wifi và Blynk bằng các thông tin define ở trên
  Serial.println("connected to blynk");
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
  ttt_bt_system = 0;


}



String getName(int fingerID) {
  String names[] = {"Dao Trong Phuc", "Dao Thanh Son", "Nguyen Van Son"}; //Mảng lưu trữ thông tin tên sinh viên
  if (fingerID < 1 || fingerID > sizeof(names)) {
    return "Unknown"; //Trả về "Unknown" nếu không tìm thấy ID của sinh viên trong mảng
  } else {
    return names[fingerID - 1]; //Trả về tên của sinh viên tương ứng với ID của vân tay
  }
}
String getMSV(int fingerID) {
  String msv[] = {"B19DCCN504", "B19DCCN000", "B19DCCN001"};
  if (fingerID < 1 || fingerID > sizeof(msv)) {
    return "Unknown"; 
  } else {
    return msv[fingerID - 1];
  }
}



int dsKQ[3] = t{0,0,0};
BLYNK_WRITE(V0) {
  tbn_system = param.asInt();
}
void loop(){
  Blynk.run();

  
  if(tbn_system==1){   
    fID = getFingerprintID();
    setManHinhChinh();
    if(fID == -1){
      setManHinhChinh();
    }
    else if(fID == 0){
      hienThiWrongFingerprint();
    }
    else{
      dsKQ[fID] =1;
      hienThiDiemDanh(getName(fID), getMSV(fID));
    }
    ttt_bt_system = tbn_system;
  }else{
    setManHinhTat();
    if(ttt_bt_system==1){
      GuiEmail();
      for(int i=0;i<sizeof(dsKQ);i++){
        dsKQ[i]=0;
      }
      ttt_bt_system=0;
    }
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

void setManHinhTat(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(40,20);
  display.println("Dang tat!");
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(52,35);
  display.println("...");  
  display.display();
  delay(0);
}



// gui email
void GuiEmail(){
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
  message.addRecipient("Giao vien",Recipient_email);
  String textMsg = "DANH SACH DIEM DANH: \n";
  for(int i=0;i<sizeof(dsKQ);i++){
    if(dsKQ[i] ==1){
      textMsg += getName(i) + " - " + getMSV(i) + "\n";
    }
  }
 
  message.text.content = textMsg.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  
  //gửi tin nhắn đến email giáo viên
  if (!smtp.connect(&session))
    return;
  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());
}
