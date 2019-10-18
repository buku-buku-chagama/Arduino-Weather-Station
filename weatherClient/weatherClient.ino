/**
 * Used "BasicHTTPClient.ino" as the basis for this sketch
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>

#define USE_SERIAL Serial
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

//OLED pins to ESP32 GPIOs via this connecthin:
//OLED_SDA -- GPIO4
//OLED_SCL -- GPIO15
//OLED_RST -- GPIO16

#define SDA    4
#define SCL   15
#define RST   16 //RST must be set by software

#define V2  1

#ifdef V2 //WIFI Kit series V1 not support Vext control
  #define Vext  21
#endif

#define DISPLAY_HEIGHT 64
#define DISPLAY_WIDTH  128

int char2Dig(char);

struct weather{
  String datum;
  String windSpd;
  String windDir;
  String airTemp;
  String dewPoint;
  String baro;
};

SSD1306  display(0x3c, SDA, SCL, RST);


WiFiMulti wifiMulti;
int startPos;
int endPos;
String lex;
weather curWeather;
    


void setup() {
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);    // OLED USE Vext as power supply, must turn ON Vext before OLED init
  delay(50); 
  
  display.init();

  // display.flipScreenVertically();

  display.setContrast(255); 
  display.setLogBuffer(5, 30);
  display.clear();
  
    USE_SERIAL.begin(115200);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }
    
    // Change these to match your SSID and Password
    wifiMulti.addAP("WIFI SSID", "PASSWORD");

}

void loop() {
    // wait for WiFi connection
    int spamDelay = 1000 * 60 * 30;
    String temp;
    String cardinal[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW", "N"}; 
    lex = "";
    if((wifiMulti.run() == WL_CONNECTED)) {

        HTTPClient http;

        USE_SERIAL.print("[HTTP] begin...\n");
        // configure traged server and url
        //                 Replace this for your local weather---vvvv 
        http.begin("https://www.aviationweather.gov/taf/data?ids=KSJT&format=raw&metars=on&layout=off"); //HTTP
        USE_SERIAL.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                startPos = payload.indexOf("<code>",0);
                endPos = payload.indexOf("</code>",startPos + 1);
                String packet  = payload.substring(startPos + 6, endPos);
                float tempDec, tempF, windDir, windSpd;                
                // Parse Packet
                for(unsigned int i = 0; i < packet.length(); i++)
                {
                    lex = lex + packet[i];
                    // Parse AirTemp and Dewpoint
                    if(packet[i] == ' ' || i == packet.length() - 1) {
                      if (lex[0]=='T'){
                        curWeather. airTemp = lex.substring(1,5);
                        curWeather. dewPoint = lex.substring(5,lex.length());
                      }
                      // Parse Barometric Pressure
                      if (lex[0]=='A' && lex.length() > 4){
                        curWeather.baro = lex.substring(1,5);
                      }
                      // Parse whatever this is
                      if (lex[lex.length()-2] == 'Z'){
                        curWeather.datum = lex.substring(0,6);
                      }
                      // Parse WindSpeed and Direction
                      if (lex[lex.length()-3] == 'K' && lex[lex.length()-2] == 'T'){
                        curWeather.windDir = lex.substring(0,3);
                        curWeather.windSpd = lex.substring(3,5);
                      }
                      lex = "";
                    }
                  }
                
                // Do Conversions
                temp = curWeather.airTemp;              
                
                tempDec = str2Float(curWeather.airTemp.substring(1,3)) * (-((int)(temp[0] == '1')) + ((int)(temp[0] == '0')));
                tempF = (tempDec * 1.8) + 32.0;

                windDir = str2Float(curWeather.windDir);
                windSpd = str2Float(curWeather.windSpd);
         
                USE_SERIAL.println(packet);
                
                display.clear();
                display.print("Temp: "); display.print(tempF); display.print(" F / ");
                display.print(tempDec); display.println(" C");
                display.println("Wind: " + cardinal[(int)((windDir / 45.0) + 0.5)] + " Spd: " + (int)windSpd + " kts");
                display.println("Baro Pressure: " + curWeather.baro.substring(0,2) + "." + curWeather.baro.substring(2,4));
                display.drawLogBuffer(0, 0);
                display.display();
            }
        } else {
            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            display.clear();
            display.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            display.drawLogBuffer(0, 0);
            display.display();
        }

        http.end();
    }

    delay(spamDelay);
}

int char2Dig(char t)
{
  return (int)(t - 48);
}

float dig2Float(int t, float e)
{
  return ((float)t) * e;
}

float str2Float(String s)
{
  float ret = 0.0;
  float place = 1.0;
  for(int i = s.length() - 1; i >= 0 ; i--){
    ret += dig2Float(char2Dig(s[i]), place);
    place *= 10.0;
  }
  return ret;
}
