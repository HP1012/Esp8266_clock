/*
 * Sử dụng NodeMCU/ESP8266 như một điểm truy cập Wifi (Access Point)
 * và tạo Web Server đơn giản để điều khiển GPIO (ở đây là relay)
 * Kết nối đến wifi "SmartControl", password = "makerspace.vn"
 * Mở trình duyệt và truy cập 192.168.4.1 để mở Web Server
 * Author: Bùi Minh Phúc
 */
#include <FirebaseArduino.h>
 
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include "string.h"
#include "stdio.h"
//----------
#define SERIAL_NUMBER 71020001
//-----------------------------------
#define BAUD_RATE 115200
//SoftwareSerial swSer(14, 12, false, 256);  //D5,D6
//----------out

//----------led_setting


// Set these to run example.
#define FIREBASE_HOST "timer-deadline.firebaseio.com"
#define FIREBASE_AUTH ""


const char* taikhoan = "LinhKienSinhVien";      // tên của AP
const char* matkhau = "123456789"; // mật khẩu truy cập, để rỗng nếu không muốn cài mật khẩu
//int relayPin = 14;                      // mã GPIO của chân D5

//IPAddress local_IP(192,168,1,150);
//IPAddress gateway(192,168,1,1);
//IPAddress subnet(255,255,255,0);
ESP8266WebServer server(80);            // khai báo server
//-----------

char WIFI_SSID[30] ;//="Lau 3";
char WIFI_PASSWORD[30];// = "@123456789@HT" ;

char tk[30],mk[30];



//uint16_t dem;
//String Data_uart;
//bool fl_setting=false,setting=false;
//bool bink_led=false,tong_led=false;
//uint8_t tick_timer1=0;
//byte value;
//uint8_t fl_send_firebase=0, fl_reset_firebase_error=0;;


//----------------------ngat web noi bo seting ID_PASS wifi --------------------------
void handleWebsever() 
{
if( server.args() ==2)
{
  if(server.argName(0) == "ID" && server.argName(1) == "PASS")
  {
   String save =  "&"+server.arg(0)+";"+server.arg(1)+"$" ;
   uint8_t t=0;
   while(save[t] != 0 && save.length() > 12)
   {
    EEPROM.write(t, save[t]);
    EEPROM.commit();
    t++;
    
   }
   server.send(200, "text/plain", save);       //Response to the HTTP request
  }
                         
}
}
void handleWebsever_reset()
{
      server.send(200, "text/reset", "RESET_OK"); 
      delay(500);
      WiFi.softAPdisconnect(true);
      ESP.restart();
}
//---------------------------END---------------------------------------------------

//---------------------------ngat dieu khien noi bo--------------------------------
void handleLed()
{
  if(server.args() >0)
  {
    for(int i =0;i<= server.args();i++)
    {
     fl_send_firebase = 1; //-------------luu lai trang thai up len firebase
      if(server.argName(i) == "out_1")
      {      
        String value_out_1 = server.arg(i) ;
        state_1 = value_out_1[0] -48; 
        digitalWrite(OUT_1,state_1);
      }
      else if(server.argName(i) == "out_2")
      {
       String value_out_2 = server.arg(i) ;
        state_2 = value_out_2[0] -48; 
        digitalWrite(OUT_2,state_2); 
      }
      else if(server.argName(i) == "out_3")
      {
        String value_out_3 = server.arg(i) ;
        state_3 = value_out_3[0]-48 ; 
        digitalWrite(OUT_3,state_3);
      }
      else if(server.argName(i) == "out_4")
      {
        String value_out_4 = server.arg(i) ;
        state_4 = value_out_4[0] -48; 
        digitalWrite(OUT_4,state_4);
      }
      
    }

    server.send(200, "text/set", "OK"); 
  }
}


//-------------------------END----------------------------------------------------


void serialEvent()
{
    char d = Serial.read();
    if(d=='\n')
    {
     // server.send(200, "text/html", Data_uart);
   //  Serial.println(Data_uart);
      Data_uart="";
    }
    else
    {

      Data_uart = Data_uart+d;
    }
  Serial.println("nhan\n");
}

//-------------------ngat nut nhan----------------------------------
void handleInterrupt() { 
    //Serial.println("Interrupt Detected"); 
    fl_setting=true;
    tick_timer1=0;
    
}
//---------------------end-----------------------------------------

void handleTimer1()
{
  
  if(digitalRead(interruptPin) ==0 &&  fl_setting == true )tick_timer1++;
  if(tick_timer1== 4 )
  {
    fl_setting=false;
    setting=true;
    tick_timer1=0;
    bink_led ^=1;
    digitalWrite(led_conect,LOW);
    digitalWrite(OUT_1, LOW);
    digitalWrite(OUT_2, LOW);
    digitalWrite(OUT_3, LOW);
    digitalWrite(OUT_4, LOW);
    WiFi.disconnect(true);
    if(bink_led == 0)
    {
      WiFi.softAPdisconnect(true);
      ESP.restart();
    }
}
  

  if(bink_led==true) 
  {
    tong_led ^=1;
    if(tong_led==true)digitalWrite(led_s,HIGH);
    else digitalWrite(led_s,LOW);
  }
  else {digitalWrite(led_s,LOW);  }
}

void setup() //-----------------------------------------------------------void---setup-----------------------------------------
{
    Serial.begin(115200);
    //Serial1.begin(115200);
    //swSer.begin(BAUD_RATE);
    EEPROM.begin(512);
    delay(500);

    // cau hinh nut nhan dang ngat--------------
    pinMode(interruptPin, INPUT_PULLUP); 
    attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING); 
    // cau hinh timer-----------------
    timer1_isr_init();
    timer1_attachInterrupt(handleTimer1);
    timer1_enable(TIM_DIV256, TIM_EDGE, TIM_LOOP);  //div_1 : 80mhz, div_16 : 5mhz  ,div_256: 312.5 khz
    timer1_write(156250);  // T = 0.5s
    //----------led_seting---------------------------
    pinMode(led_s,OUTPUT);
    digitalWrite(led_s,LOW);
    //--------led_connect
    pinMode(led_conect,OUTPUT);
    digitalWrite(led_conect,LOW); 
    //--------------out----------------------
    pinMode(OUT_1, OUTPUT);
    pinMode(OUT_2, OUTPUT);
    pinMode(OUT_3, OUTPUT);
    pinMode(OUT_4, OUTPUT);
    digitalWrite(OUT_1, state_1);
    digitalWrite(OUT_2, state_2);
    digitalWrite(OUT_3, state_3);
    digitalWrite(OUT_4, state_4);

  // setup_conect_wifi();
    
 //--------------cau hinh wifi-------------
  
  //  WiFi.config(local_IP, gateway, subnet);
  //  WiFi.mode(WIFI_STA);
  //read_eeprom-----------
    Serial.print("read_id");
    read_id_pass(WIFI_SSID,WIFI_PASSWORD);
    delay(500);
    Serial.println("OK_read");
    if(strlen(WIFI_PASSWORD) <8) 
    {
       Serial.println("ERROR");
    
        tick_timer1=4; // vao che do confige
        
    }
    else
    {
    WiFi.disconnect(true);
    Serial.println(WIFI_SSID);
    Serial.println(WIFI_PASSWORD);
   // ---end read---
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("connecting");
    uint8_t waiting_login=0;
    while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    waiting_login++;
    if(waiting_login==20) {tick_timer1=4;break;}
  }
     Serial.println();
     Serial.print("connected: ");
     Serial.println(WiFi.localIP());
     
     digitalWrite(led_conect,HIGH); 
//--------

    //server.on("/", handleRoot);               // tải giao diện khi vào IP
   // server.on("/LEDOn", handleLedOn);         // nhận sự kiện từ web server và gọi hàm tương ứng
    //server.on("/LEDOff", handleLedOff);
      if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
    server.on("/set",handleLed );
    server.begin();                           // mở server
//    Serial.println("HTTP server beginned");
    //--------------
     Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
     String path_str = "/AIG_"+String(SERIAL_NUMBER) ;
     Firebase.stream(path_str); 

      String send_IP = "AIG_"+String(SERIAL_NUMBER)+"/IP" ;
      Firebase.setString(send_IP, WiFi.localIP().toString() );
    }
 //--------------------------------------------------
}

void loop() 
{
if(bink_led==true) // khi su kien cai dat wwifi_pass
{
  if(setting==true)
  {
    setting=false;
    setup_conect_wifi();
  }
  
}
 server.handleClient();                    // chờ bắt sự kiện từ web server


//---------------------------------------firebase-----------------------------------

if(bink_led==false)
{
    if (Firebase.failed()) {
    Serial.println("streaming error");
    Serial.println(Firebase.error());
    delay(200);
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    delay(1000);
    fl_reset_firebase_error++;
    if(fl_reset_firebase_error == 15)  ESP.restart();
    }
    else fl_reset_firebase_error=0;
    
    if (Firebase.available()) 
    {
       send_dataFirebase(&fl_send_firebase,state_1,state_2,state_3,state_4);
       
        FirebaseObject event = Firebase.readEvent();
        String eventType = event.getString("type");
        eventType.toLowerCase();
        
        Serial.print("event: ");
        Serial.println(eventType);
        if (eventType == "put") {
        Serial.print("data: ");
        Serial.println(event.getString("data"));
        String path = event.getString("path");
        String data = event.getString("data");
        
        if (path == "/")
        {
        JsonVariant payload = event.getJsonVariant("data");
        Serial.print("data: ");
         state_1 = payload["OUT_1"];
         state_2 = payload["OUT_2"];
         state_3 = payload["OUT_3"];
         state_4 = payload["OUT_4"];
  
        
        digitalWrite(OUT_1, state_1);
        digitalWrite(OUT_2, state_2);
        digitalWrite(OUT_3, state_3);
        digitalWrite(OUT_4, state_4);
        
        payload.printTo(Serial);
        }
        if (path == "/OUT_1")
        {
        
        String payload = event.getString("data");
        state_1=payload.toInt();
        digitalWrite(OUT_1, payload.toInt());
        Serial.print("data light1: ");
        Serial.println(payload);
        }
        if (path == "/OUT_2")
        {
        
        String payload = event.getString("data");
        state_2=payload.toInt();
        digitalWrite(OUT_2, payload.toInt());
        Serial.print("data light2: ");
        Serial.println(payload);
        }
        if (path == "/OUT_3")
        {
        
        String payload = event.getString("data");
        state_3=payload.toInt();
        digitalWrite(OUT_3, payload.toInt());
        Serial.print("dalight3: ");
        Serial.println(payload);
        }
        if (path == "/OUT_4")
        {
        String payload = event.getString("data");
        state_4=payload.toInt();
        digitalWrite(OUT_4, payload.toInt());
        Serial.print("data light4: ");
        Serial.println(payload);
        }
        
        }


      String send_out_1 = "AIG_"+String(SERIAL_NUMBER)+"/IST" ;  // kim tra duong truyen
      Firebase.setString(send_out_1, (String)0 );
    } 
  

}

//--
}
//-------------------------------end loop----------------------------------------
void send_dataFirebase(uint8_t *fl,uint8_t st1,uint8_t st2,uint8_t st3,uint8_t st4)
{
  if(*fl==1)
  {
    
    
      String send_out_1 = "AIG_"+String(SERIAL_NUMBER)+"/OUT_1" ;
      Firebase.setString(send_out_1, (String)st1 );
      delay(100);
   
      String send_out_2 = "AIG_"+String(SERIAL_NUMBER)+"/OUT_2" ;
      Firebase.setString(send_out_2, (String)st2 );
      delay(100);

      String send_out_3 = "AIG_"+String(SERIAL_NUMBER)+"/OUT_3" ;
      Firebase.setString(send_out_3,(String)st3 );
      delay(100);
      
      String send_out_4 = "AIG_"+String(SERIAL_NUMBER)+"/OUT_4" ;
      Firebase.setString(send_out_4, (String)st4 );
      delay(100);  

      String send_IP = "AIG_"+String(SERIAL_NUMBER)+"/IP" ;
      Firebase.setString(send_IP, WiFi.localIP().toString() );
      delay(100);  
      
      *fl =0; 
      
    

  }
  else return;
  
}


void read_id_pass(char *name_id, char *name_pass)
{
    uint8_t s=0,data_wifi[65],id,pass;
   
    data_wifi[0] = EEPROM.read(0);
    if(data_wifi[0]=='&')
    {
      while(data_wifi[s-1] != 36)
      {
        delay(10);
         data_wifi[s] = (char)EEPROM.read(s);
         s++;
         if(s >60) break;
      }
      
      for( id = 1;id < s-1;id++)
      {
        if(data_wifi[id] != ';') *name_id++ = data_wifi[id];
        else break;
      }
      for(pass=id+1;pass<s-1;pass++)
      {
        *name_pass++=data_wifi[pass];
      }
      
      
    }
}
void setup_conect_wifi()
{

  

//  WiFi.mode(WIFI_AP);
//  delay(100);
  WiFi.softAP(taikhoan, matkhau);
  Serial.println("Wait 100 ms for AP_START...");
  delay(100);
  
  Serial.println("Set softAPConfig");
  IPAddress Ip(192, 168, 7, 1);
  IPAddress NMask(255, 255, 255, 0);
  bool a = WiFi.softAPConfig(Ip, Ip, NMask);
 
 // a=WiFi.softAP(ssid, password);    
  delay(100);// tạo AP
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  delay(100);
    
    server.on("/setwifi", handleWebsever);         // nhận sự kiện từ web server và gọi hàm tương ứng
    server.on("/reset", handleWebsever_reset);         // nhận sự kiện từ web server và gọi hàm tương ứng
//    server.on("/LEDOff", handleLedOff);
    delay(100);
    server.begin();                           // mở server
    Serial.println("HTTP server beginned");
    
    pinMode(LED_BUILTIN, OUTPUT);             // khai báo chân pin cần sử dụng
//    pinMode(relayPin, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);           // gửi tín hiệu mặc định khi khởi động
//    digitalWrite(relayPin, LOW);
}
