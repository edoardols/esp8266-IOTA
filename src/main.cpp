
//Arduino Library
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

//Arduino Sensor Library
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <MQ135.h>

//Project Library

//Configuration File
#include "hornet/hornetAPI.h"
#include "hornet/hornetNode.h"
#include "network/wifiCredentials.h"

#include "JSON/information.h"

//Use WiFiClientSecure class to create TLS connection
WiFiClientSecure iotaNode;

//Root Certificate of the IotaNode that esp8266 is going to POST message
X509List cert(cert_ISRG_Root_X1);

//DHT11 Sensor
#define DHTTYPE DHT11 // DHT 11
uint8_t DHTPin = 12; //D6
DHT dht(DHTPin, DHTTYPE);

//MQ-135 Sensor
#define PIN_MQ135 0
MQ135 mq135_sensor(PIN_MQ135);

//DIGITAL_PIN 16 must be connected with RESET PIN for DeepSleep to work

//Time information for Italy
#define UTC_offset 1 // UTC +1
#define DST_offset 1 //Daylight Saving Time offset in hour

// -------------------- Data Payload IOTA -------------------- //;

int DATE_INDEX = 0;
int TIME_INDEX = 1;
int TEMPERATURE_INDEX = 2;
int HUMIDITY_INDEX = 3;
int CO2_PPM_INDEX = 4;

String sensorData[5];

// -------------------- FUNZIONI ACCESSORIE -------------------- //

//TO-DO https://www.timeanddate.com/time/change/italy
/*
bool isDSTon(String data) {

    if (){
        return true;
    }else {
        return false;
    }
}
*/

String asciiToHexdecimal(String dataASCII) {

    String dataHex = "";
    for (size_t i = 0; i < dataASCII.length(); i++) {

        if(dataASCII.charAt(i) == '\r'){
            dataHex.concat("0");
        }
        else{
            dataHex.concat( String( int( dataASCII.charAt(i) ) , HEX) );
        }
    }
    return dataHex;
}

/* -------------------- HTTPS REQUEST -------------------- */

String getRequestHTTPS(String api, String host) {

    return "GET " + api + " HTTP/1.1\r\n" +
            "Host: " + host + "\r\n" +
            "User-Agent: BuildFailureDetectorESP8266\r\n" +
            "Connection: close\r\n\r\n";
}

String postRequestHTTPS(String api, String host, String payload) {

    return "POST " + api + " HTTP/1.1\r\n" +
            "Host: " + host + "\r\n" +
            "User-Agent: BuildFailureDetectorESP8266\r\n" +
            "Content-Type: application/json\r\n" +
            "Content-Length: " + payload.length() + "\r\n" +
            "Connection: close\r\n" + 
            "\r\n" +
            payload;
}

/* -------------------- PAYLOAD IOTA -------------------- */

String makeData() {
    return "{\"data\":{\"info\":{\"indexMessage\":\"" + String(indexMessage) + 
    "\",\"stationName\":\"" + String(stationName) + 
    "\",\"installationDate\":\"" + String(installationDate) + 
    "\",\"coordinates\":\"" + String(coordinates) + 
    "\",\"placeName\":\"" + String(placeName) + 
    "\",\"owner\":\"" + String(owner) + 
    "\",\"sensor1\":\"" + String(sensor1) + 
    "\",\"sensor2\":\"" + String(sensor2) + 
    "\"},\"sensorData\":{\"day\":\"" + String(sensorData[DATE_INDEX]) + 
    "\",\"time\":\"" + String(sensorData[TIME_INDEX]) + 
    "\",\"temperature\":\"" + String(sensorData[TEMPERATURE_INDEX]) + 
    "\",\"humidity\":\"" + String(sensorData[HUMIDITY_INDEX]) + 
    "\",\"co2_ppm\":\"" + String(sensorData[CO2_PPM_INDEX]) + "\"}}}";
}

String makePayload(String index, String data) {
    //Hornet API: https://editor.swagger.io/?url=https://raw.githubusercontent.com/rufsam/protocol-rfcs/master/text/0026-rest-api/0026-rest-api.yaml
    // index => HEX
    // data => HEX
    return "{\"payload\":{\"type\":2,\"index\":\"" + asciiToHexdecimal(index) + "\",\"data\":\"" + asciiToHexdecimal(data) + "\"}}";

}

// -------------------- void setup() -------------------- //

void setup() {

    //step1 => initialize serial monitor
    Serial.begin(115200);

    //step2 => setup Wi-Fi
    //set esp8266 mode only as client and not function as Hostspot
    WiFi.mode(WIFI_STA);
    //set WiFi credentials
    WiFi.begin(ssid, password);

    //whait for esp8266 to connect to Wi-Fi
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    //step3 => Set time via NTP, as required for x.509 validation    
    configTime(3600 * UTC_offset, 3600 * DST_offset , "europe.pool.ntp.org", "pool.ntp.org");

    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2) {
        delay(500);
        now = time(nullptr);
    }

    //step4 => set certificate for veryfing the host
    iotaNode.setTrustAnchors(&cert);

    if(iotaNode.probeMaxFragmentLength(host_iotanode, port_iotanode, 512) == true){
        //set MFLN negoziation
        iotaNode.setBufferSizes(512,512);
    }
    
    //step5 => set the pin of DHT
    pinMode(DHTPin, INPUT);
    dht.begin();
    
}

// -------------------- void loop() -------------------- //

void loop() {

    // ----- step1 => connect to WiFi ----- //
    int wait = 0;
    while (WiFi.status() != WL_CONNECTED) {

        if (wait > 120) {
            //wait for 2 min after that reset the ESP
            ESP.restart();
        }
        wait++;
        delay(1000);
    }

    // ----- step2 => sync time with NTP ----- //

    if (WiFi.status() == WL_CONNECTED){

        int wait = 0;
        time_t now = time(nullptr);
        while (now < 8 * 3600 * 2) {

            now = time(nullptr);
            if (wait > 120) {
                //wait for 2 min after that reset the ESP
                ESP.restart();
            }
            wait++;
            delay(1000);
        }
    }

    // ----- step3 => verify if IOTA node is alive ----- //

    if (!iotaNode.connect(host_iotanode, port_iotanode)) {
        return;
    }

    // ----- step4 => collect sensor data ----- //

    //temperature
    float temperature = dht.readTemperature();
    sensorData[TEMPERATURE_INDEX] = String(temperature, 2);

    //humidity
    float humidity = dht.readHumidity();
    sensorData[HUMIDITY_INDEX] = String(humidity, 2);

    //co2 ppm
    float correctedPPM = mq135_sensor.getCorrectedPPM(temperature, humidity);
    sensorData[CO2_PPM_INDEX] = String(correctedPPM, 2);

    struct tm tmstruct;
    tmstruct.tm_year = 0;
    time_t now = time(nullptr);
    gmtime_r(&now, &tmstruct);

    String string_day = String(tmstruct.tm_mday);
    String string_month = String(( tmstruct.tm_mon)+1);

    if(tmstruct.tm_mday < 10){
        string_day = "0" + String(tmstruct.tm_mday);
    }
    
    if(( tmstruct.tm_mon)+1 < 10){
        string_month = "0" + String(( tmstruct.tm_mon)+1);
    }

    sensorData[DATE_INDEX] = string_day + "/" + string_month + "/" + String((tmstruct.tm_year)+1900);

    String string_hour = String(tmstruct.tm_hour + UTC_offset + DST_offset);
    String string_minute = String(tmstruct.tm_min);
    String string_second = String(tmstruct.tm_sec);

    //test DST

    if(tmstruct.tm_hour + UTC_offset + DST_offset < 10){
        
        string_hour = "0" + String(tmstruct.tm_hour + UTC_offset + DST_offset);
    }

    if(tmstruct.tm_hour + UTC_offset + DST_offset == 24) {
        string_hour = "00";
    }

    if(tmstruct.tm_hour + UTC_offset + DST_offset == 25) {
        string_hour = "01";
    }

    if(tmstruct.tm_min < 10){
        string_minute = "0" + String(tmstruct.tm_min);
    }

    if(tmstruct.tm_sec < 10){
        string_second = "0" + String(tmstruct.tm_sec);
    }

    sensorData[TIME_INDEX] = string_hour + ":" + string_minute + ":" + string_second;

    // ----- step5 => make dataPayload ----- //

    String payload = makePayload(indexMessage, makeData());

    // ----- step6 => POST data ----- //

    String post = postRequestHTTPS(hornet_api_post_message, host_iotanode, payload);
    iotaNode.print(post);

    // ----- step7 => verify if data are recived correctly ----- //

    //Server response
    String line3;
    while (iotaNode.connected()) {
        line3 = iotaNode.readStringUntil('\n');
        if (line3 == "\r") {
            //Serial.println(line3);
            break;
        }
    }

    String line2 = iotaNode.readStringUntil('\n');
    Serial.println(line2);

    // ----- step8 => go to sleed for 60 seconds ----- //

    //ESP.deepSleep(60e6);
    
    //60 minutes
    ESP.deepSleep(3600e6);
}
