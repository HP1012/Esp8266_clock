/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdio.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include "SimpleKalmanFilter.h"
#include "SimpleKalmanFilter.c"

#define FIREBASE_HOST "doamdat-619bb-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "4j4uZDlM0aNlxs028EyTerLvjJGMlcZm5ocyCu4P"
#define WIFI_SSID "SWADT"
#define WIFI_PASSWORD "ngan1111@"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "asia.pool.ntp.org");

#define PIN_PUMP_SS1 D7

#define ADDRESS_EEPROM_TARGET_SS1 0
//
typedef union {
    float f;
    unsigned int u;
    unsigned char c[4];
    struct
    {
        unsigned int mantissa : 23;
        unsigned int exponent : 8;
        unsigned int sign : 1;
 
    } raw;
} float_ieee_754;
typedef struct{
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
} st_date;
//

String pump_status_ss1 = "off";
float do_am_dat_ss1 = 100.0;
bool send_database = false;
bool flag_get_date = true;
bool flag_print_oled = false;
float_ieee_754 target_ss1;
st_date date;

void handleTimer1()
{
  static int giay = 0;
  giay++;
  if(giay == 10)
  {
    giay = 0;
    send_database = true;
  }
  rtc_date();
  flag_print_oled = true;
}

void setup() {
    //
  pinMode(PIN_PUMP_SS1, OUTPUT);
  digitalWrite(PIN_PUMP_SS1, HIGH);
  //
  Serial.begin(115200);
            
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.stream("/Project 1");
  //
  timeClient.begin();
  timeClient.setTimeOffset(3600*7);
  //
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Hello, world!");
  display.display();
  //
  // cau hinh timer-----------------
  timer1_isr_init();
  timer1_attachInterrupt(handleTimer1);
  timer1_enable(TIM_DIV256, TIM_EDGE, TIM_LOOP);  //div_1 : 80mhz, div_16 : 5mhz  ,div_256: 312.5 khz
  timer1_write(312500);  // T = 1s
  //
  EEPROM.begin(512);
  for(int i = 0; i < 4; i++)
  {
    target_ss1.c[i] = EEPROM.read(ADDRESS_EEPROM_TARGET_SS1 + i);
  }
  Serial.print("Target ss1 float: ");
  Serial.println(target_ss1.f);

  SimpleKalmanFilter(1, 1, 0.1);

  date = get_date();
}

void loop() {
  //------
  int val = 0;
  char tmp[50];
  static bool flag_pump_ss1_on = false;
  //------
  if(flag_get_date == true)
  {
    date = get_date();
    flag_get_date = false;
  }
  //---------------------get adc -------------------------------
  val = 1024 - analogRead(A0);
  do_am_dat_ss1 = (val / (float)1024) * 100;
  do_am_dat_ss1 = updateEstimate(do_am_dat_ss1);
  do_am_dat_ss1 = updateEstimate(do_am_dat_ss1);
  if(do_am_dat_ss1 < target_ss1.f - 10.0)
  {
    if(flag_pump_ss1_on == false)
    {
      flag_pump_ss1_on = true;
      digitalWrite(PIN_PUMP_SS1, LOW);
      pump_status_ss1 = "on";
      Firebase.setString("Project 1/History/" + get_format_date(), "pump ss1 " + pump_status_ss1);
      delay(100);
    }
  }
  else if(do_am_dat_ss1 >= target_ss1.f)
  {
    if(flag_pump_ss1_on == true)
    {
      flag_pump_ss1_on = false;
      digitalWrite(PIN_PUMP_SS1, HIGH);
      pump_status_ss1 = "off";
      Firebase.setString("Project 1/History/" + get_format_date(), "pump ss1 " + pump_status_ss1);
      delay(100);
    }
  }
  else{}
  //----------------------Print screen ----------------------------------
  if(flag_print_oled == true)
  {
    flag_print_oled = false;
    sprintf(tmp,"Value ss1 : %0.2f", do_am_dat_ss1);
    display.clearDisplay();
    display.setCursor(15, 0);
    display.println(get_format_date());
    display.setCursor(0, 20);
    display.print(tmp);
    display.println("%");
    sprintf(tmp,"Target ss1: %0.2f", target_ss1.f);
    display.setCursor(0, 35);
    display.print(tmp);
    display.println("%");
    display.setCursor(0, 50);
    display.print("Pump status: " + pump_status_ss1);
    display.display();
  }
  //----------------------Check event database---------------------------
  if (Firebase.available()) 
  {
      FirebaseObject event = Firebase.readEvent();
      String eventType = event.getString("type");
      eventType.toLowerCase();
      
      if (eventType == "put")
      {
          String path = event.getString("path");
          if(path == "/Setup/Target ss1")
          {
            target_ss1.f = event.getFloat("data");
            Serial.print("target_ss1.f:");
            Serial.println(target_ss1.f);
            for(int i = 0; i < 4; i++)
            {
              EEPROM.write(ADDRESS_EEPROM_TARGET_SS1 + i, target_ss1.c[i]);
            }
            if (EEPROM.commit()) {
              Serial.println("EEPROM successfully committed");
            } else {
              Serial.println("ERROR! EEPROM commit failed");
            }
          }
      }
  }
  //---------------------start send data ----------------------------
  if(send_database == true)
  {
    Serial.println("send data to firebase");
    // send data to firebase
    delay(100);
    Firebase.setFloat("Project 1/Status/Value ss1", do_am_dat_ss1);
    delay(100);
    Firebase.setString("Project 1/Status/Pump ss1", pump_status_ss1);
    delay(100);
    // handle error
    if (Firebase.failed())
    {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());
    }
    send_database = false;
  }
  //-----------------------end send data-----------------------------
}

st_date get_date()
{
  st_date return_date;

  timeClient.update();


  return_date.hour    = timeClient.getHours();
  return_date.minute  = timeClient.getMinutes();
  return_date.second  = timeClient.getSeconds();
  
  unsigned long epochTime = timeClient.getEpochTime();
  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 

  return_date.day   = ptm->tm_mday;
  return_date.month = ptm->tm_mon+1;
  return_date.year  = ptm->tm_year+1900;

  return return_date;
}

String get_format_date()
{
  String currentDate = String(date.year) + "-" + String(date.month) + "-" + String(date.day) + " "
      + String(date.hour) + ":" + String(date.minute) + ":" + String(date.second);
  return currentDate;
}

void rtc_date()
{
  if(date.second == 59)
  {
    date.second = 0;
    if(date.minute == 59)
    {
      date.minute = 0;
      if(date.hour == 23)
      {
        date.hour = 0;
        flag_get_date = true;
      }
      else
      {
        date.hour++;
      }
    }
    else
    {
      date.minute++;
    }
  }
  else
  {
    date.second++;
  }
}
