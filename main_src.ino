#include <Arduino.h>

#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <avr/wdt.h>
#include <uRTCLib.h>
#include <DHT.h>
#include <SoftwareSerial.h> 

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW

#define MAX_DEVICES 4
#define CS_PIN 5

#define BT_TX 2  // Arduino RX
#define BT_RX 3  // Arduino TX

#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

String receivedData = "";
String bluetoothData = "";

MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

uRTCLib rtc(0x68);

SoftwareSerial Bluetooth(BT_TX, BT_RX);

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

String data1, data2, data3, dataAll, datasend;
int mode1 = 1, mode2 = 0, mode3 = 0;
char textBuffer[128];
char Time[128], Temp[128], Text[128];

bool flag = 0;

void setup() {
  //Serial.begin(9600);
  dht.begin();
  delay(2000);

  Bluetooth.begin(9600);

  URTCLIB_WIRE.begin();
  //rtc.set(0, 31, 12, 5, 12, 3, 25);

  // Intialize the object
  myDisplay.begin();
  // Set the intensity (brightness) of the display (0-15)
  myDisplay.setIntensity(0);
  // Clear the display
  myDisplay.displayClear();

  delay(100);
}

// Hàm xử lý lệnh từ Bluetooth
void processBluetoothCommand(String command) {
  command.trim();
  // Kiểm tra lệnh đơn giản (ví dụ: đặt mode)
  if (command == "m1") {
    mode1 = 1;
    mode2 = 0;
    mode3 = 0;
    //Serial.println("MODE 1");
  } else if (command == "m2") {
    mode1 = 0;
    mode2 = 1;
    mode3 = 0;
    //Serial.println("MODE 2");
  } 
  else if (command == "re") {
    int a = 0;
    //Serial.println("Reset");
    wdt_enable(WDTO_15MS);
    while (1);
  }
  else {
    mode1 = 0;
    mode2 = 0;
    mode3 = 1;
    //Serial.println("MODE 3");
    command.toCharArray(Text, 128);
  }
}

void loop() {
  rtc.refresh();

  // Đọc dữ liệu từ DHT11
  float tempDHT = dht.readTemperature(); // Đọc nhiệt độ (°C)
  float humDHT = dht.readHumidity();     // Đọc độ ẩm (%)

  if (isnan(tempDHT) || isnan(humDHT)) {
    tempDHT = 0;
    humDHT = 0;
  }

  data1 = String(rtc.hour())  + ':' + String(rtc.minute());
  data2 = String(tempDHT, 1) + "oC";
  //data3 = String(humDHT, 1) + "%";


  
  dataAll = data1 + ' '+ data2;
  datasend = data1 + ',' + String(tempDHT, 1) + ',';
  
  // Gửi dữ liệu qua Bluetooth HC-05
  Bluetooth.println(datasend);

  data1.toCharArray(Time, 128);
  data2.toCharArray(Temp, 128);

  // Kiểm tra nếu có dữ liệu từ HC-05
  while (Bluetooth.available()) {
    char c = Bluetooth.read();

    if (c == '\\') {
      //Serial.println("Received from Bluetooth: " + bluetoothData);

      processBluetoothCommand(bluetoothData);

      bluetoothData = ""; 
    } 
    else {
      if (c >= 48 && c <= 126) { 
      bluetoothData += c;
      }
    }
  }

  if (mode1 == 1){
    myDisplay.setTextAlignment(PA_CENTER);
    myDisplay.displayText(Time, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
    while (!myDisplay.displayAnimate()) {
    }
  }
  else if (mode2 == 1){
    myDisplay.setTextAlignment(PA_CENTER);
    myDisplay.displayText(Temp, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
    while (!myDisplay.displayAnimate()) {
    }
  }
  else if(mode3 == 1){
    myDisplay.displayClear();
    myDisplay.setTextAlignment(PA_LEFT);
    myDisplay.setTextEffect(PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    myDisplay.displayText(Text, PA_LEFT, 100, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    while (!myDisplay.displayAnimate()) {
    }
    mode3 = 0;
    mode1 = 1;
  }
  delay(100);
}
