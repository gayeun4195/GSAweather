/*
 풍향, 풍속, 강수량 측정 예제
 
 풍향: 10초마다 1회 계산 (16방위 중 하나로 측정)
 풍속: 20ms 주기로 10초간 모니터링 후 풍속계산
 강수량: 20ms 주기로 모니터링하여 1시간동안 누적(1시간에 1회 공식 데이타로 사용)
 
 핀연결
 풍향계: A0   풍향센서 전선 2개 중 하나는 GND에 나머지선은 A0에 연결 및 10k저항 거쳐 5V에 ..................................................................................
 풍속계: D2   풍속센서 전선 2개 중 하나는 D2에 나머지는 GND
 강수계: D6   강수센서 전선 2개 중 하나는 D6에 나머지는 GND

 (0524) 레오나르도 보드로 세팅해야함
 
 http://ArtRobot.co.kr 
 http://RoboBob.co.kr
*/
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"

#define WIND_N 0
#define WIND_NNE 22.5
#define WIND_NE 45
#define WIND_ENE 67.5
#define WIND_E 90
#define WIND_ESE 112.5
#define WIND_SE 135
#define WIND_SSE 157.5
#define WIND_S 180
#define WIND_SSW 202.5
#define WIND_SW 225
#define WIND_WSW 247.5
#define WIND_W 270
#define WIND_WNW 292.5
#define WIND_NW 315
#define WIND_NNW 337.5

#define ADAFRUIT_CC3000_IRQ   7  // MUST be an interrupt pin!
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER);

#define WLAN_SSID       "xxxxx"
#define WLAN_PASS       "OOOOO"
#define WLAN_SECURITY   WLAN_SEC_WPA2

#define IDLE_TIMEOUT_MS  3000
#define WEBSITE      "xxx.xxx.xx.xx"
#define WEBPAGE      "/OOOOO/weather/add_data.php?"

uint32_t ip;

const int windVanePin=A0;
int winVaneValue=0;        
float windSpeed=0;
float rainGauge=0;
float windDirection=0;
char windName[4];

unsigned long windSpeedTimer;
int windSpeedState=true; 
int windSpeedPin=2;
int windSpeedCounter=0;
unsigned long rainGaugeTimer;
int rainGaugeState=true; 
int rainGaugePin=6;
int rainGaugeCounter=0;
int windDirectionphp;

// 시리얼 통신 메시지
char msg[100]="";
// php 메시지
String phpmsg = "";
char tempChar[200] = "";

void setup()
{
    Serial.begin(9600);
    Serial.println(F("Hello, CC3000!\n")); 

    Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);
  
    /* Initialise the module */
    Serial.println(F("\nInitializing..."));
    if (!cc3000.begin())
    {
      Serial.println(F("Couldn't begin()! Check your wiring?"));
      while(1);
    }
  
    //wind speed sensor
    pinMode(windSpeedPin, INPUT);
    digitalWrite(windSpeedPin, HIGH);
  
    //rain gauge sensor
    pinMode(rainGaugePin, INPUT);
    digitalWrite(rainGaugePin, HIGH);

    Serial.print(F("\nAttempting to connect to ")); Serial.println(WLAN_SSID);
    if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
      Serial.println(F("Failed!"));
    while(1);
    }
   
    Serial.println(F("Connected!"));
  
    /* Wait for DHCP to complete */
    Serial.println(F("Request DHCP"));
    while (!cc3000.checkDHCP())
    {
      delay(100); // ToDo: Insert a DHCP timeout!
    }  

    /* Display the IP address DNS, Gateway, etc. */  
    while (! displayConnectionDetails()) {
      delay(1000);
    }

    ip = 0;
    // Try looking up the website's IP address
    Serial.print(WEBSITE); Serial.print(F(" -> "));
    while (ip == 0) {
      if (! cc3000.getHostByName(WEBSITE, &ip)) {
        Serial.println(F("Couldn't resolve!"));
      }
      delay(500);
    }

    cc3000.printIPdotsRev(ip);
}

//char 배열의 문자열정보를 Serial통해 문자로 전송 ,  PC에서 참고용
void printChars(char *msg, int len){
    if(len == 0) return;
    for(int i=0; i<len ; i++)
        Serial.print(msg[i]);
    Serial.println();
}

//f2h와 f2p는  float형의 정수부와 소수부를 위한 것입니다.(sprintf에서 float형 사용에 문제가 있어서 사용된 함수)
int f2h(float num)
{
    return int(num);
}

int f2p(float num)
{
    return (num-int(num)) * 100;
}

//풍향측정: 아날로그핀입력되는 전압값으로 16가지 방향중 하나로 계산됨.
float getWindDirection(void){
    int readValue=analogRead(windVanePin);            
    // 0~1023 사이의 입력값을 0~5V 기준 값으로 비례변경함.
    winVaneValue=map(readValue, 0, 1023, 0, 500);    
    //이제 winVaneValue는 데이타 시트정보상의 방위별 전압치와 유사한 값이 됩니다.
  
    if(winVaneValue < 35){ 
        //0~0.35V를 동동서 로 인식함
        //112.5  0.32v (31 32)    0~35
        windDirection=WIND_ESE;
        strcpy(windName,"ESE");
        windDirectionphp=5;        
    }
    else if(winVaneValue < 43){
        //67.5  0.41v(40 41)    ~42    
        windDirection=WIND_ENE;
        strcpy(windName, "ENE");
        windDirectionphp=3;        
    }
    else if(winVaneValue < 50){
        //90  0.45v(44 45)      ~50    
        windDirection=WIND_E;
        strcpy(windName, "E");
        windDirectionphp=4;        
    }
    else if(winVaneValue < 70){
        //157.5  0.62v(60 62)   ~70
        windDirection=WIND_SSE;
        strcpy(windName, "SSE");
        windDirectionphp=7;        
    }
    else if(winVaneValue < 100){
        //135  0.90v(89 90)      ~100
        windDirection=WIND_SE;  
        strcpy(windName, "SE");
        windDirectionphp=6;        
    }
    else if(winVaneValue < 130){
        //202.5  1.19v(119 120)  ~130
        windDirection=WIND_SSW;  
        strcpy(windName, "SSW");
        windDirectionphp=9;        
    }
    else if(winVaneValue < 170){
        //180  1.40v(140 141)   ~170
        windDirection=WIND_S;  
        strcpy(windName, "S");
        windDirectionphp=8;        
    }
    else if(winVaneValue < 210){
        //22.5  1.98v(198 199)  ~210
        windDirection=WIND_NNE;  
        strcpy(windName, "NNE");
        windDirectionphp=1;            
    }
    else if(winVaneValue < 250){
        //45  2.25v(226 227)    ~250
        windDirection=WIND_NE;  
        strcpy(windName, "NE");
        windDirectionphp=2;            
    }
    else if(winVaneValue < 300){
        //247.5  2.93v(293 294)  ~300
        windDirection=WIND_WSW;  
        strcpy(windName, "WSW");
        windDirectionphp=11;        
    }
    else if(winVaneValue < 320){
        //225  3.08v(308 310)   ~320
        windDirection=WIND_SW;  
        strcpy(windName, "SW");
        windDirectionphp=10;        
    }
    else if(winVaneValue < 360){
        //337.5  3.43 (343 345)  ~360
        windDirection=WIND_NNW;  
        strcpy(windName, "NNW");
        windDirectionphp=15;        
    }
    else if(winVaneValue < 395){
        //0  3.84v(384~385)    ~395
        windDirection=WIND_N;  
        strcpy(windName, "N");
        windDirectionphp=0;
    }
    else if(winVaneValue < 415){
        //292.5  4.04v(405 406)  ~415
        windDirection=WIND_WNW;  
        strcpy(windName, "WNW");
        windDirectionphp=13;    
    }
    else if(winVaneValue < 450){
        //315  4.34(433 434)  ~450
        windDirection=WIND_NW;  
        strcpy(windName, "NW");
        windDirectionphp=14;        
    }
    else if(winVaneValue < 490){
        //270  4.62v(461 463)    ~490
        windDirection=WIND_W;  
        strcpy(windName, "W");
        windDirectionphp=12;        
    }
    else{
        //error  알수없는 값범위
    }

}

void loop()
{
    // 5초마다 측정결과를 PC에 전달
    windSpeedTimer=millis() + 4000; // 4초를 주기로 반복됩니다.
    windSpeedCounter=0;
    windDirectionphp=100;
    while(millis() < windSpeedTimer){
        delay(20); //20ms 주기로 센서의 스위칭을 감지합니다.
        //wind speed  풍속계 센서 감지부
        if((windSpeedState == true) && !digitalRead(windSpeedPin)){ 
            windSpeedCounter++;     //스위치 상태가 high에서 low로 떨어지는 순간을 감지합니다.
            windSpeedState=false;
        }
        else if((windSpeedState == false) && digitalRead(windSpeedPin)){
            windSpeedState=true;
        }    
        //rain gauge
        if((rainGaugeState == true) && !digitalRead(rainGaugePin)){ 
            rainGaugeCounter++;     //스위치 상태가 high에서 low로 떨어지는 순간을 감지합니다.
            rainGaugeState=false;
        }
        else if((rainGaugeState == false) && digitalRead(rainGaugePin)){
            rainGaugeState=true;
        }        
    }
 
    rainGaugeTimer++;
    if(rainGaugeTimer > 720){// 5초 * 720=3600초(1시간)  지난 1시간동안 누적한 강수계 카운터로 강수량 계산
        //rainGauge=rainGaugeCounter * 0.2794;
        rainGaugeCounter=0; 
        rainGaugeTimer=0;
    }
    windSpeed=windSpeedCounter * 0.48 ;   // 1초당 1회 스위칭시 2.4km/h 속력이며 4초 기간이므로 0.6이 됨
    getWindDirection();  // 풍향은 발표시점에 1회만 측정
    rainGauge=rainGaugeCounter * 0.2794;  //지난 1시간(최대)동안의 누적 강우량
    //rainGauge=rainGaugeCounter;
    //char 배열에 정보를 취합 후 출력
    sprintf(msg, "GSA Weather Bot said => Wind: %s/%d.%d, %d.%d(km/h) Rain %d.%d(mm/h).", windName, f2h(windDirection), f2p(windDirection), f2h(windSpeed), f2p(windSpeed),  f2h(rainGauge), f2p(rainGauge));
    Serial.println();
    printChars(msg, sizeof(msg));

    windSpeed=windSpeed*100;  //데이터 너무 작은 것(0 전송) 방지하기 위해 100 곱해서 전송
    
    Adafruit_CC3000_Client www = cc3000.connectTCP(ip, 80);
    if (www.connected()) {
      if(windDirectionphp!=100){
        www.fastrprint(F("GET "));
        www.fastrprint(WEBPAGE);
        phpmsg="windDirection="+String(windDirectionphp)+"&windSpeed="+f2h(windSpeed)+'.'+f2p(windSpeed)+"&rainGauge="+f2h(rainGauge)+'.'+f2p(rainGauge);
        for(int i=0; i<200; i++)
          tempChar[i] = 0x00;
        phpmsg.toCharArray(tempChar,phpmsg.length());
        //printChars(phpmsg,sizeof(phpmsg));
        www.fastrprint(tempChar);
        www.fastrprint(F(" HTTP/1.1\r\n"));
        www.fastrprint(F("Host: ")); www.fastrprint(WEBSITE); www.fastrprint(F("\r\n"));
        www.fastrprint(F("\r\n"));
        www.println(); delay(1000);
      }
      else{ //풍향 데이터 주어지지 않은 경우 pass
        Serial.println(F("Wrong Data"));
        return;
      }
    } 
    else {
      Serial.println(F("Connection failed"));    
      return;
    }

    Serial.println(F("-------------------------------------"));
  
    /* Read data until either the connection is closed, or the idle timeout is reached. */ 
    unsigned long lastRead = millis();
    while (www.connected() && (millis() - lastRead < IDLE_TIMEOUT_MS)) {
      while (www.available()) {
        char c = www.read();
        Serial.print(c);
        lastRead = millis();
      }
    }
    www.close();
    Serial.println(F("-------------------------------------"));
}

bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}
