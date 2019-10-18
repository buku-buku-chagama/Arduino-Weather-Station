# Arduino-Weather-Station

Arduino weather readout using HELTEC WIFI_Kit_32.

Find out your local weather station's four letter ID. It can be found on "https://www.aviationweather.gov". It usually starts with a "K" in the US or "C" in Canada. 

In line that contains :

http.begin("https://www.aviationweather.gov/taf/data?ids=KSJT&format=raw&metars=on&layout=off")

Replace "KSJT" with your local weather station ID.
