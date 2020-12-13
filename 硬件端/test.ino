#include <Servo.h>
#include "ESP8266.h"
#include "SoftwareSerial.h"
#define RANGE 250  // 容错范围

#define SSID ""    //填写2.4GHz的WIFI名称，不要使用校园网
#define PASSWORD ""//填写自己的WIFI密码
#define HOST_NAME "api.heclouds.com"  //API主机名称，连接到OneNET平台，无需修改
#define DEVICE_ID ""       //填写自己的OneNet设备ID
#define HOST_PORT (80)                //API端口，连接到OneNET平台，无需修改
String APIKey = ""; //与设备绑定的APIKey

//已经隐藏了热点，密码 设备ID 和 APIKey

SoftwareSerial mySerial(3, 2);
ESP8266 wifi(mySerial);

unsigned long t = 0;

int report = 0;  //送上onenet的数据
 
int key[2][6]{
  {440,200,340,360,490,190},  //设置密码，设置两个是为了进一步提高容错率
  {500,250,390,400,540,240} 
};

int i = 0;   //密码位数
Servo servo;

void setup() {

  mySerial.begin(115200); //初始化软串口
  Serial.begin(9600);     //初始化串口
  Serial.print("setup begin\r\n");

  //以下为ESP8266初始化的代码
  Serial.print("FW Version: ");
  Serial.println(wifi.getVersion().c_str());

  if (wifi.setOprToStation()) {
    Serial.print("to station ok\r\n");
  } else {
    Serial.print("to station err\r\n");
  }

  //ESP8266接入WIFI
  if (wifi.joinAP(SSID, PASSWORD)) {
    Serial.print("Join AP success\r\n");
    Serial.print("IP: ");
    Serial.println(wifi.getLocalIP().c_str());
  } else {
    Serial.print("Join AP failure\r\n");
  }

  mySerial.println("AT+UART_CUR=9600,8,1,0,0");
  mySerial.begin(9600);
  Serial.println("setup end\r\n");

  servo.attach(9);
  t = millis();

 servo.write(0);
}

void loop() {
int dt = millis() - t;   //设置时间间隔

  if(dt>2000){
    t=millis();
    i = 0;
    servo.write(0);
    return;
   }    //间隔过长则重置
   
   if(analogRead(A0)<250 && dt>200){  //接受到声音且间隔大于200ms
     t = millis();
     bool b = false;

     for(int a=0;a<sizeof(key)/sizeof(key[0]);a++){
        b=(key[a][i-1]-RANGE<dt && dt<key[a][i-1]+RANGE);
        if(b) break;  //判断间隔是否符合设定的区间内
      }
      
    if(i==0||b){
        if(++i>sizeof(key[0])/sizeof(key[0][0])){   //输入完成
            i = 0;
            servo.write(90);
            report = 1;
            Serial.println(report);
            
      if (wifi.createTCP(HOST_NAME, HOST_PORT)) { //建立TCP连接，如果失败，不能发送该数据
      Serial.print("create tcp ok\r\n");
      char buf[10];   //开始拼接要发送的东西
      String jsonToSend = "{\"Number\":";
      dtostrf(report, 1, 0, buf);
      jsonToSend += "\"" + String(buf) + "\"";
      jsonToSend += "}";
      
      String postString = "POST /devices/";
      postString += DEVICE_ID;
      postString += "/datapoints?type=3 HTTP/1.1";
      postString += "\r\n";
      postString += "api-key:";
      postString += APIKey;
      postString += "\r\n";
      postString += "Host:api.heclouds.com\r\n";
      postString += "Connection:close\r\n";
      postString += "Content-Length:";
      postString += jsonToSend.length();
      postString += "\r\n";
      postString += "\r\n";
      postString += jsonToSend;
      postString += "\r\n";
      postString += "\r\n";
      postString += "\r\n";
      
      const char *postArray = postString.c_str(); //将str转化为char数组
      
      Serial.println(postArray);
      
      wifi.send((const uint8_t *)postArray, strlen(postArray)); //send发送命令，参数必须是这两种格式，尤其是(const uint8_t*)
      Serial.println("send success");
      if (wifi.releaseTCP()) { //释放TCP连接
        Serial.print("release tcp ok\r\n");
      } else {
        Serial.print("release tcp err\r\n");
      }
      postArray = NULL; //清空数组，等待下次传输数据
    } else {
      Serial.print("create tcp err\r\n");
    }

    Serial.println("");
    servo.write(0);
          }
          return;
      }

      else if(i>4){  //若试图开门时输错则上传数字“0”，这里设置是4位以后输错才执行下列语句
        servo.write(0);
        report = 0;
        Serial.println(report);
            
      if (wifi.createTCP(HOST_NAME, HOST_PORT)) { 
      Serial.print("create tcp ok\r\n");
      char buf[10];
      String jsonToSend = "{\"Number\":";
      dtostrf(report, 1, 0, buf);
      jsonToSend += "\"" + String(buf) + "\"";
      jsonToSend += "}";
      
      String postString = "POST /devices/";
      postString += DEVICE_ID;
      postString += "/datapoints?type=3 HTTP/1.1";
      postString += "\r\n";
      postString += "api-key:";
      postString += APIKey;
      postString += "\r\n";
      postString += "Host:api.heclouds.com\r\n";
      postString += "Connection:close\r\n";
      postString += "Content-Length:";
      postString += jsonToSend.length();
      postString += "\r\n";
      postString += "\r\n";
      postString += jsonToSend;
      postString += "\r\n";
      postString += "\r\n";
      postString += "\r\n";
      
      const char *postArray = postString.c_str(); 
      
      Serial.println(postArray);
      
      wifi.send((const uint8_t *)postArray, strlen(postArray)); 
      Serial.println("send success");
      if (wifi.releaseTCP()) { 
        Serial.print("release tcp ok\r\n");
      } else {
        Serial.print("release tcp err\r\n");
      }
      postArray = NULL; 
    } else {
      Serial.print("create tcp err\r\n");
    }

    Serial.println("");
        } 
      i=0;   // 密码复位
   }
}
