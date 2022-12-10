// define thông tin thiết bị trên Blynk
#define BLYNK_TEMPLATE_ID "TMPLx-1agVFX"
#define BLYNK_DEVICE_NAME "IoT 2"
#define BLYNK_AUTH_TOKEN "VgNLc70ijF50ZGIvPHmEdmhstFbxxSmn"
// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <string.h>
#include "DHT.h"


char ssid[] = "P305";                                       //  Tên wifi
char pass[] = "hoilamgi";                                   //  Mật khẩu wifi
char auth[] = "VgNLc70ijF50ZGIvPHmEdmhstFbxxSmn";           //  Mã người dùng trên Blynk


#define SR505_1 14    //D5
#define SR505_2 12    //D6
#define DHTPIN 13     //D7

#define OUTPUT_ON 5     //D1
#define OUTPUT_OFF 4    //D2
#define OUTPUT_TANG 0   //D3
#define OUTPUT_GIAM 2   //D4

#define DHTTYPE DHT22   // DHT 22  (AM2302) 
#define STEP 5000       // Thời gian lấy mẫu nhiệt độ
#define TEMPERATURE 30  // Nhiệt độ làm mốc để bật tắt điều hòa


DHT dht(DHTPIN, DHTTYPE);

int TT_SR505_1 = 0;           // Trạng thái trước của cảm biến 1
int TT_SR505_2 = 0;           // Trạng thái trước của cảm biến 2
int so_nguoi = 0;             // số lượn người trong phòng
int so_nguoi_truoc = 0;       // lưu số người có trong phòng trong lần lấy mẫu trước
int TT_dieu_hoa = 0;          // Trạng thái điều hòa (0- off, 1- on)
int NhietDoDieuHoa = 27;      // Nhiệt độ đang có trong điều hòa
unsigned long tt_time=0;      // Thời điểm lấy mẫu lần trước để tính toán lần lấy mẫu tiếp theo
int tbn_system = 0;           // Lấy dữ liệu điều kiển từ Blynk, 1- cho phép điều kiển, 0- tắt hệ thống điều kiển và tắt điều hòa nếu nó đang bật
int TT_tbn_system = 0;        // Lưu trạng thái của tbn_system trong vòng lặp trước (dùng để lấy xung lên)
int t1 =35;                   // Tạo nhiệt độ ảo để thực hiện test hệ thống (khi triển khai sẽ xóa đi và thay bằng nhiệt độ đo được)

void setup() {
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);    //  kết nối với wifi và Blynk bằng các thông tin define ở trên
  Serial.println("connected");
  dht.begin();                      // setup cảm biến nhiệt độ, độ ẩm DHT

  // setup chân nối ESP8266
  pinMode(SR505_1, INPUT);
  pinMode(SR505_2, INPUT);

  pinMode(OUTPUT_ON, OUTPUT);
  pinMode(OUTPUT_OFF, OUTPUT);
  pinMode(OUTPUT_TANG, OUTPUT);
  pinMode(OUTPUT_GIAM, OUTPUT);
  so_nguoi = 0;
  so_nguoi_truoc = 0;
  TT_dieu_hoa = 0;
  NhietDoDieuHoa = 28;
  tt_time=0;
}

BLYNK_WRITE(V0) {
  tbn_system = param.asInt();
}

void loop() {
  Blynk.run();

  // =================== PHẦN ĐẾM SỐ NGƯỜI RA VÀO PHÒNG ===============================
  // Người đi vào phòng
  // Nếu cảm biến 1 có xung lên và cảm biến 2 nhận giá trị 0  thì đang có người vào
  if (digitalRead(SR505_1) == HIGH && TT_SR505_1 == 0 && digitalRead(SR505_2) == LOW) {         
    so_nguoi++;
    Blynk.virtualWrite(V5, so_nguoi);
  }
  
  // Người đi ra khỏi phòng
  // Nếu cảm biến 2 có xung lên và cảm biến 1 nhận giá trị 0  thì đang có người ra
  if (digitalRead(SR505_2) == HIGH && TT_SR505_2 == 0 && digitalRead(SR505_1) == LOW) {         
    if(so_nguoi >0) so_nguoi--;             // Tránh lỗi số người âm
    Blynk.virtualWrite(V5, so_nguoi);
  }  

  // Update lại các trạng thái trước của cảm biến chuyển động
  TT_SR505_2 = digitalRead(SR505_2);
  TT_SR505_1 = digitalRead(SR505_1);



  // ==================== PHẦN ĐIỀU KIỂN HỆ THỐNG ĐIỀU HÒA =============================
  if(tbn_system==1){                                        // Kiểm tra hệ thống có đang bật?
    if(millis()- tt_time >= STEP || millis()< tt_time){     // Xác định đã đến lần lấy mẫu tiếp theo chưa
      if(TT_dieu_hoa == 0){                                 // Kiểm tra trạng thái điều hòa
        if(t1>= TEMPERATURE && so_nguoi>0){                 // Nếu điều kiện thỏa mãn thì bật điều hòa (t1 là nhiệt độ test) 
          write_pulse(OUTPUT_ON);
          TT_dieu_hoa=1;
        }
      }else{                                                // Nếu điều hòa đã bật
        if(so_nguoi==0){                                    // Nếu không có ai
          if(so_nguoi_truoc==0){                            // Và lần lấy mẫu trước cũng không có ai
            write_pulse(OUTPUT_OFF);                        // Thì tắt điều hòa
            TT_dieu_hoa=0;
          }
        }else{                                              // Nếu có người
          if(so_nguoi <3){                                  // Và số người ít hơn 3  
          // Điều chỉnh nhiệt độ về mốc 28*C  
            if(NhietDoDieuHoa<28){                          
              write_pulse(OUTPUT_TANG);                     
              NhietDoDieuHoa++;
            }
            if(NhietDoDieuHoa>28){                          
              write_pulse(OUTPUT_GIAM);                     
              NhietDoDieuHoa--;
            }
          }else{
            if(so_nguoi<5){                                 // Nếu số người lớn hơn bằng 3 và nhỏ hơn 5
            // Điều chỉnh nhiệt độ về mốc 26*C
              if(NhietDoDieuHoa<26){                        
                write_pulse(OUTPUT_TANG);
                NhietDoDieuHoa++;
              }
              if(NhietDoDieuHoa>26){
                write_pulse(OUTPUT_GIAM);
                NhietDoDieuHoa--;
              }
            }else{                                          // Nếu số người >5
            // Điều chỉnh nhiệt độ về mốc 24*C
              if(NhietDoDieuHoa>24){                        
                write_pulse(OUTPUT_GIAM);
                NhietDoDieuHoa--;
              }
              if(NhietDoDieuHoa<24){
                write_pulse(OUTPUT_TANG);
                NhietDoDieuHoa++;
              }
            }
          }
        }
      }
  
      so_nguoi_truoc = so_nguoi;                            // Update lại số người trước khi sang step mới
      tt_time = millis();                                   // Update lại thời gian step trước
      // Đẩy thông tin lên Blynk
      Blynk.virtualWrite(V1, TT_dieu_hoa);
      Blynk.virtualWrite(V2, NhietDoDieuHoa);
    }
  }else{                                                    // Nếu hệ thống tắt
    if(TT_tbn_system==1){                                   // Và Trạng thái trước của hệ thống điều kiển là bật
    // Tắt điều hòa và update dữ liệu trên Blynk
      write_pulse(OUTPUT_OFF);
      TT_dieu_hoa=0;
      Blynk.virtualWrite(V1, TT_dieu_hoa);
    }
  }
  TT_tbn_system=tbn_system;                                 // Lưu trạng thái trước của hệ thống điều kiển 
}

void write_pulse(int pin){                                  // Hàm tạo xung điều khiển 
  digitalWrite(pin, HIGH);
  delay(50);
  digitalWrite(pin, LOW);
  return;
}