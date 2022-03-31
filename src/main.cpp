
//Arduino Library
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

//Arduino Sensor Library
#include <Adafruit_Sensor.h>
#include <DHT.h>

//Project Specific Library
#include "asciiToHex.cpp"

//Configuration File
#include "hornet/hornetAPI.h"
#include "hornet/hornetNode.h"
#include "network/wifiCredentials.h"

//Use WiFiClientSecure class to create TLS connection
WiFiClientSecure iotaNode;

//Root Certificate of the IotaNode that esp8266 is going to POST message
X509List cert(cert_ISRG_Root_X1);

//DHT11 Sensor
#define DHTTYPE DHT11 // DHT 11
uint8_t DHTPin = 12; //D6
DHT dht(DHTPin, DHTTYPE);

//MQ-135 Sensor




float temperature_Celsius;
float humidity;

unsigned long lastTime = 0;  
unsigned long timerDelay = 5000;  // send readings timer milliseconds

/* -------------------- PAYLOAD IOTA -------------------- */
String iotaindex;
String timestamp;
String coordinate;

/* -------------------- FUNZIONI ACCESSORIE -------------------- */

/* -------------------- HTTPS REQUEST -------------------- */

String getHttps(String url, String host) {
    return "GET " + url + " HTTP/1.1\r\n" +
            "Host: " + host + "\r\n" +
            "User-Agent: BuildFailureDetectorESP8266\r\n" +
            "Connection: close\r\n\r\n";
}

String postHttps(String url, String host, String payload) {
    return "POST " + url + " HTTP/1.1\r\n" +
            "Host: " + host + "\r\n" +
            "User-Agent: BuildFailureDetectorESP8266\r\n" +
            "Content-Type: application/json\r\n" +
            "Content-Length: " + payload.length() + "\r\n" +
            "Connection: close\r\n" + 
            "\r\n" +
            payload;
}

/* -------------------- PAYLOAD IOTA -------------------- */
/* ORIGINALE
String makeData() {
    //String iotadata = "data from a esp8266 with dht11 sensor"
    return "---Informazioni---\r\n"
            "Data origins: esp8266 with dht11 sensor\r\n"
            "Timestamp: " + timestamp +  "\r\n" +
            "Coordinate: " + coordinate + "\r\n" +
            "base installate il " + "\r\n" +
            //"messggio precedente" => creare una catena di messaggi facilmente percorribile
            "---Fine Informazioni---" + "\r\n" +
            "---Sensore---" + "\r\n" +
            "Temperatura: " + temperature_Celsius + "\r\n" +
            "Umidità: " + humidity + "\r\n" +
            "---Fine sensore---";
}
*/

/*
String makeData() {
    //String iotadata = "data from a esp8266 with dht11 sensor"
    //humidity = dht.readHumidity();
    //temperature_Celsius = dht.readTemperature();
    return "---Informazioni---\r\n"
            "Data origins: esp8266 with dht11 sensor\r\n"
            "---Fine Informazioni---\r\n"
            "---Sensore---\r\n"
            "Temperatura: " + String(dht.readTemperature(), 3) + " C" + "\r\n" +
            "Umidità: " + String(dht.readHumidity(), 3) + " %"+ "\r\n" +
            "---Fine sensore---";
}*/

String makeData() {
    //String iotadata = "data from a esp8266 with dht11 sensor"
    return "esp8266withdht11sensor";
}

String makePayload(String index, String data) {
    //Hornet API: https://editor.swagger.io/?url=https://raw.githubusercontent.com/rufsam/protocol-rfcs/master/text/0026-rest-api/0026-rest-api.yaml
    // index => HEX
    // data => HEX
    return "{\"payload\":{\"type\":2,\"index\":\"" + asciiToHexdecimal(index) + "\",\"data\":\"" + asciiToHexdecimal(data) + "\"}}";

}

/* -------------------- void setup() -------------------- */

void setup() {

    //step1 => initialize serial monitor
    Serial.begin(115200);

    //step2 => setup Wi-Fi
    //set esp8266 mode only as client and not function as Hostspot
    // => hotspost => monitorarlo? poterlo ricompilare ?? cfr esp8266-iot-framework OTA
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    //step3 => Set time via NTP, as required for x.509 validation
    configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

    /***** debug ******/
    Serial.println("Waiting for NTP time sync: ");
    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2) {
        delay(500);
        Serial.print(".");
        now = time(nullptr);
    }
    Serial.println("");
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    Serial.print("Current time: ");
    Serial.print(asctime(&timeinfo));
    /***** end debug ******/

    //step4 => set certificate for veryfing the host
    iotaNode.setTrustAnchors(&cert);

    Serial.print("MFLN: ");
    Serial.println(iotaNode.probeMaxFragmentLength(host_iotanode, 443, 512));

    iotaNode.setBufferSizes(512,512);

    //step5 => set the pin of DHT
    pinMode(DHTPin, INPUT);
    dht.begin();
    
}

/* -------------------- void loop() -------------------- */

void loop() {
    //step1 => verify if the esp8266 is connected to Wi-Fi
    while (WiFi.status() != WL_CONNECTED) {
        //delay(500);
        //Serial.print(".");
        //wait until tot min poi restart esp8266
    }

    //step2 => verify if the esp8266 time is sync

    // ?????????????

    //step3 => verify if the esp8266 is connected to iota node

    if (!iotaNode.connect(host_iotanode, port_iotanode)) {
        Serial.println("Connection failed");
        return;
    }
    //step4 => read data from sensor

    //DTH11
    // Read temperature as Celsius (the default)
    humidity = dht.readHumidity();
    temperature_Celsius = dht.readTemperature();

    /***** debug ******/
    Serial.printf("Temperature = %.2f ºC \n", temperature_Celsius);
    Serial.printf("Humidity= %f %\n", humidity);
    /***** end debug ******/

    //step5 => incapsulate data in json (create payload) //TO-DO

    //stabilire l'indice
    //stabilire il formato della richiesta con le varie specifiche (nome stazione, posizione, timestamp)
    iotaindex = "esp8266iota_el";

    /*
    //time
    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2) {
        delay(500);
        //Serial.print(".");
        now = time(nullptr);
    }
    //Serial.println("");
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    timestamp = asctime(&timeinfo);
    */
    
    coordinate = "41°40'23.1\"N 12°46'05.2\"E";


    //step5 => send data to iota node

    /***** debug ******/
    Serial.println("-----POST-----");

    Serial.print("API: ");
    Serial.println(hornet_api_post_message);

    Serial.print("HOST: ");
    Serial.println(host_iotanode);

    String testdata = makeData();
    Serial.println("DATA: \r\n" + testdata);

    String payload = makePayload(iotaindex, testdata);
    Serial.println("PAYLOAD: \r\n" + payload);

    String post = postHttps(hornet_api_post_message, host_iotanode, payload);
    Serial.println("POST: \r\n" + post);

    iotaNode.print( post );
    /***** end debug ******/

    //iotaNode.print( post(hornet_api_post_message, host_iotanode, makePayload(iotaindex, makeData())) );

    //step6 => verify if the data are recived correctly

    Serial.println("Request sent");
    while (iotaNode.connected()) {
        String line = iotaNode.readStringUntil('\n');
        if (line == "\r") {
            Serial.println("Headers received");
            break;
        }
    }
    //payload della risposta del server
    String line = iotaNode.readStringUntil('\n');

    Serial.println("Reply was:");
    Serial.println("==========");
    Serial.println(line);
    Serial.println("==========");
    Serial.println("Closing connection");

    //step7 => if all above success go to sleed mode

    //read data every 30 min? (test with 1-2min)
}


    //////////////////////////////////////////////////////////7

    /*Serial.println("Test Connection");
    while (iotaNode.connected()) {
        //qui si dovrebbe controllare se l'header è codice 200 => è andato tutto bene
        String line = iotaNode.readStringUntil('\n');
        if (line == "\r") {
            //non c'è più niente da leggere
            Serial.println("Headers received");
            break;
        }
    }

    if (!iotaNode.connect(host_iotanode, port_iotanode)) {
        Serial.println("Connection failed");
        return;
    }

    
    //payload della risposta del server
    String line = iotaNode.readStringUntil('\n');

    Serial.println("Reply was:");
    Serial.println("==========");
    Serial.println(line);
    Serial.println("==========");
    Serial.println("Closing connection");
    */





