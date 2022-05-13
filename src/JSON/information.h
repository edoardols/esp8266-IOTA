/*
 * This file contains the information about the post
 * This file contains the information about the esp8266, the sensors and its location
 */

/*
{
    "data": {
        "info": {
            "indexMessage": "A",
            "stationName": "A",
            "installationDate": "A",
            "coordinates": "A",
            "placeName": "A",
            "owner": "A",
            "sensor1": "A",
            "sensor2": "A",
            ...
            "sensorN": "A"
        },
        "sensorData": {
            "day": "A",
            "time": "A",
            "temperature": "A",
            "humidity": "A",
            "co2_ppm": "A"
        }
    }
}
*/

//info
const char* indexMessage = "TEST_WEB_APP6";
const char* stationName = "esp8266 EL";
const char* installationDate = "10-05-2022";
const char* coordinates = "43°06'34.1'N 12°23'33.7'E";
const char* placeName = "Perugia";
const char* owner = "";
const char* sensor1 = "DHT11";
const char* sensor2 = "MQ-135";

//sensorData
const char* day = "";

// "time" create a conflict whit time class in Arduino
const char* time_json = "";
const char* temperature = "";
const char* humidity = "";
const char* co2_ppm = "";