#include "Barometer.h"
#include <Wire.h>
#include "DHT.h"
#define DHTPIN 2
#define DHTTYPE DHT22
#include <HttpClient.h>
#include <LTask.h>
#include <LWiFi.h>
#include <LWiFiClient.h>
#include <LDateTime.h>
#define WIFI_AP "Joshua"
#define WIFI_PASSWORD "jchris1901"
#define WIFI_AUTH LWIFI_WPA  // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP.
#define per 50
#define per1 3
#define DEVICEID "DYSiAVd3" // Input your deviceId
#define DEVICEKEY "0a1T3FWgmaRBN1dM" // Input your deviceKey
#define SITE_URL "api.mediatek.com"
LWiFiClient c;
char port[4]={0};
char connection_info[21]={0};
char ip[21]={0};             
int portnum;
int val = 0;

float t = 0.0;
float h = 0.0;
  
String tcpdata = String(DEVICEID) + "," + String(DEVICEKEY) + ",0";
LWiFiClient c2;
HttpClient http(c2);
DHT dht(DHTPIN, DHTTYPE);
float temperature;
float pressure;
float atm;
float altitude;
int pin = 8;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 2000;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;
String tem;
String pre;
String con;
Barometer myBarometer;


void setup() {
  LTask.begin();
  LWiFi.begin();
  Serial.begin(9600);
  myBarometer.init();
  dht.begin();
  pinMode(8,INPUT);
  pinMode(6,LOW);
  starttime = millis();
  while(!Serial) delay(1000); /* comment out this line when Serial is not present, ie. run this demo without connect to PC */

  Serial.println("Connecting to AP");
  while (0 == LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH, WIFI_PASSWORD)))
  {
    delay(1000);
  }
  
  Serial.println("calling connection");

  while (!c2.connect(SITE_URL, 80))
  {
    Serial.println("Re-Connecting to WebSite");
    delay(1000);
  }
  delay(100);

  pinMode(13, OUTPUT);
  getconnectInfo();
  connectTCP();
}

void getconnectInfo(){
  //calling RESTful API to get TCP socket connection
  c2.print("GET /mcs/v2/devices/");
  c2.print(DEVICEID);
  c2.println("/connections.csv HTTP/1.1");
  c2.print("Host: ");
  c2.println(SITE_URL);
  c2.print("deviceKey: ");
  c2.println(DEVICEKEY);
  c2.println("Connection: close");
  c2.println();
  
  delay(500);

  int errorcount = 0;
  while (!c2.available())
  {
    //Serial.println("waiting HTTP response: ");
    //Serial.println(errorcount);
    errorcount += 1;
    if (errorcount > 10) {
      c2.stop();
      return;
    }
    delay(100);
  }
  int err = http.skipResponseHeaders();

  int bodyLen = http.contentLength();
  Serial.print("Content length is: ");
  Serial.println(bodyLen);
  Serial.println();
  char c;
  int ipcount = 0;
  int count = 0;
  int separater = 0;
  while (c2)
  {
    int v = c2.read();
    if (v != -1)
    {
      c = v;
      Serial.print(c);
      connection_info[ipcount]=c;
      if(c==',')
      separater=ipcount;
      ipcount++;    
    }
    else
    {
      Serial.println("no more content, disconnect");
      c2.stop();

    }
    
  }
  Serial.print("The connection info: ");
  Serial.println(connection_info);
  int i;
  for(i=0;i<separater;i++)
  {  ip[i]=connection_info[i];
  }
  int j=0;
  separater++;
  for(i=separater;i<21 && j<5;i++)
  {  port[j]=connection_info[i];
     j++;
  }
  Serial.println("The TCP Socket connection instructions:");
  Serial.print("IP: ");
  Serial.println(ip);
  Serial.print("Port: ");
  Serial.println(port);
  portnum = atoi (port);
  Serial.println(portnum);

} //getconnectInfo

void uploadstatus(){
  //calling RESTful API to upload datapoint to MCS to report LED status
  Serial.println("calling connection");
  LWiFiClient c2;  
  
  while (!c2.connect(SITE_URL, 80))
  {
    Serial.println("Re-Connecting to WebSite");
    delay(1000);
  }
  delay(100);
  tem="tmp,,t";
  int thislength = tem.length();
 // pre="\npressure,,pressure";
 // con="\nconc,,concentration";
  HttpClient http(c2);
  c2.print("POST /mcs/v2/devices/");
  c2.print(DEVICEID);
  c2.println("/datapoints.csv HTTP/1.1");
  c2.print("Host: ");
  c2.println(SITE_URL);
  c2.print("deviceKey: ");
  c2.println(DEVICEKEY);
  c2.print("Content-Length: ");
  c2.println(thislength);
  c2.println("Content-Type: text/csv");
  c2.println("Connection: close");
  c2.println();
  c2.println(tem);
  delay(500);

  int errorcount = 0;
  while (!c2.available())
  {
    //Serial.print("waiting HTTP response: ");
    //Serial.println(errorcount);
    errorcount += 1;
    if (errorcount > 10) {
      c2.stop();
      return;
    }
    delay(100);
  }
  int err = http.skipResponseHeaders();

  int bodyLen = http.contentLength();
  //Serial.print("Content length is: ");
  //Serial.println(bodyLen);
  Serial.println();
  while (c2)
  {
    int v = c2.read();
    if (v != -1)
    {
      Serial.print(char(v));
    }
    else
    {
      Serial.println("no more content, disconnect");
      c2.stop();

    }
    
  }
}



void connectTCP(){
  //establish TCP connection with TCP Server with designate IP and Port
  c.stop();
  Serial.println("Connecting to TCP");
  Serial.println(ip);
  Serial.println(portnum);
  while (0 == c.connect(ip, portnum))
  {
    Serial.println("Re-Connecting to TCP");    
    delay(1000);
  }  
  Serial.println("send TCP connect");
  c.println(tcpdata);
  c.println();
  Serial.println("waiting TCP response:");
} //connectTCP

void heartBeat(){
  Serial.println("send TCP heartBeat");
  c.println(tcpdata);
  c.println();
    
} //heartBeat


void loop() {
  if(dht.readHT(&t, &h))
  {
      Serial.println("------------------------------");
      Serial.print("temperature = ");
      Serial.println(t);
      Serial.print("humidity = ");
      Serial.println(h);
  }
  temperature = myBarometer.bmp085GetTemperature(myBarometer.bmp085ReadUT()); //Get the temperature, bmp085ReadUT MUST be called first
  pressure = myBarometer.bmp085GetPressure(myBarometer.bmp085ReadUP());//Get the pressure
  altitude = myBarometer.calcAltitude(pressure); //Uncompensated caculation - in Meters
  Serial.print("Pressure: ");
  Serial.print(pressure, 0); //whole number only.
  Serial.println(" Pa");
  duration = pulseIn(4, LOW);
  lowpulseoccupancy = lowpulseoccupancy+duration;
  if ((millis()-starttime) >= sampletime_ms)
  {
    ratio = lowpulseoccupancy/(sampletime_ms*10.0);
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62;
    Serial.print("concentration = ");
    Serial.print(concentration);
    Serial.println(" pcs/0.01cf");
    Serial.println("\n");
    /*if( concentration < 1.00 )
      Serial.println("Optimal Concentration");
    else
      Serial.println("NOT Optimal Concentration");
    if( ( t > 15.0 ) && ( t < 45.0 ) )
      Serial.println("Optimal Temperature");
    else
      Serial.println("NOT Optimal Temperature");
    if( pressure > 80 )
      Serial.println("Optimal Pressure");
    else
      Serial.println("NOT Optimal Pressure");
*/  
    if( concentration > 1 || t < 15 || t > 45 || pressure < 80)
      {
        digitalWrite(13, HIGH);
        Serial.println("The Environment is not Optimal");
      }
      else
      {
        digitalWrite(13, LOW);
        Serial.println("The Environment is Optimal");
      }
    lowpulseoccupancy = 0;
        uploadstatus();
        Serial.println();
    starttime = millis();
  }
  delay(100);
}
