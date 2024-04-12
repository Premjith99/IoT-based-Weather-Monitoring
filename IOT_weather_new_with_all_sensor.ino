#include <DHT.h> 
#include <SoftwareSerial.h>
#include <Wire.h>

#include <ThingSpeak.h>
#define IP "184.106.153.149"
#define RX 10
#define TX 11
#include <WiFiEsp.h>
String apiKey = "XMUDFH85HWKBG7BQ1";

SoftwareSerial ESP8266(RX,TX ); // RX, TX

#define buzzer 12 //Buzzer pin
#define DHTTYPE DHT11
#define DHTPIN  4
#define ldrpin A3 // For Brightness

DHT dht(DHTPIN,DHTTYPE,11);
const int sensorMin = 0; // For rainfall sensor
const int sensorMax = 1024; //For rainfall sensor
int brightness=0; 
float humidity,temp,rainfall,CO_val;
long writingTimer=17;//Evert 17s will data get uploaded 
long startTime=0;
long waitTime=0;


boolean relay1_st=false;
boolean relay2_st=false;
unsigned char check_connection=0;
unsigned char times_check=0;
boolean error;

void setup()
{
    
    pinMode(12,OUTPUT); //LED pin
    Serial.begin(9600);  
    ESP8266.begin(9600);
    dht.begin();
    
    startTime=millis();// starting the "program clock"
    ESP8266.println("AT+RST");
    delay(2000);
    Serial.println("Connecting to Wifi");
  while(check_connection==0)
  {
   Serial.print("..");
   ESP8266.print("AT+CWJAP=\"Asus\",\"1234567890\"\r\n");
   ESP8266.setTimeout(5000);
   if(ESP8266.find("WIFI CONNECTED\r\n")==1 )
   {
   Serial.println("WIFI CONNECTED");
   break;
   }
  times_check++;
   if(times_check>3) 
   {
    times_check=0;
    Serial.println("Trying to Reconnect..");
    }
  }
  
  
}
void loop()
{
  waitTime=millis()- startTime;
  if(waitTime>(writingTimer*1000)) //After every 17s the below functions get called
  {
     readSensors();
     writeThingSpeak();
     startTime=millis();
    }
 }


  void readSensors(void)
  {
    
     rainfall = analogRead(A0);
    int range = map(rainfall, sensorMin, sensorMax, 0, 3);
     switch (range)
    {
      case 0:
        Serial.println("RAIN WARNING");
        break;

      case 1:
        Serial.println("RAINING");
        break;

      case 2:
        Serial.println("NOT RAINING");
        break;
    }
    Serial.println("");
    
    CO_val=analogRead(A1);
    Serial.print("AirQua=");
    Serial.print(CO_val,DEC);
    Serial.println("PPM");
    if(CO_val>400)
     {
       digitalWrite(buzzer,HIGH);
     }
    Serial.println("");

    brightness = analogRead(ldrpin); // read the value from the sensor
    Serial.print("brightness ="); 
     Serial.print(brightness);
     Serial.println("");


    temp=dht.readTemperature();
    Serial.print("Temperature = ");
    Serial.print(dht.readTemperature());
    Serial.println("*C");
    Serial.println("");
    
    humidity=dht.readHumidity();
    Serial.print("Humidity = ");
    Serial.print(dht.readHumidity());
    Serial.println("");

  
        
  }


  void writeThingSpeak(void)
  {
    startThingSpeakCmd();
    String GetThingSpeakCmd();
    String getStr="GET/update?api_key=XMUDFH85HWKBG7BQ";
    getStr +="&field1=";
    getStr += String(temp);
    getStr +="&field2=";
    getStr += String(humidity);
    getStr +="&field3=";
    getStr += String(rainfall);
    getStr +="&field4=";
    getStr += String(CO_val);
     getStr +="&field5=";
    getStr += String(brightness);
    getStr += "\r\n\r\n";
    GetThingSpeakcmd(getStr);

  }

/*****************************Start ommunication with Thingspeak********/


void startThingSpeakCmd(void)
{
   ESP8266.flush();
   String cmd1="AT+CWMODE=1";
   String cmd2="AT+CIPMUX=0";
   String cmd="AT+CIPSTART=\"TCP\",\"";
   cmd += "184.106.153.149";//Thingspeak ip address
   cmd +="\",80";
    ESP8266.println(cmd1);
    ESP8266.println(cmd2);
   ESP8266.println(cmd);
  
   Serial.print("Start Commands:");
   Serial.println(cmd1);
   Serial.println(cmd2);
   Serial.println(cmd);
   

    if(ESP8266.find("Error"))
    {
      Serial.println("AT+CIPSTART error");
      return;
    }
   
}



/***********Get cmd to Thingspeak*********/





  String GetThingSpeakcmd(String getStr)
  {
    String cmd="AT+CIPSEND=";
    cmd +=String(getStr.length());
    ESP8266.println(cmd);
    Serial.println(cmd);

    if(ESP8266.find(">"))
    {
      ESP8266.println(getStr);
      Serial.println(getStr);
      delay(500);
      String messageBody ="";
      while (ESP8266.available())
      {
        String line=ESP8266.readStringUntil('\n');
        if (line.length()==1)
        {
          messageBody=ESP8266.readStringUntil('\n');
        }
      }
      Serial.print("MessageBody received:  ");
      Serial.println(messageBody);
      return messageBody;
    }
    else
    {
      ESP8266.println("AT+CIPCLOSE");
      Serial.println("AT+CIPCLOSE");
    }
  }
