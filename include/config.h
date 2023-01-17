#ifndef __CONFIG__
#define __CONFIG__
#define __DEVELOPMENT__
// #define __PRODUCTION__
// WiFi config
const char *ssid = "SmartFarm-Salad"; // Enter your WiFi name
const char *password = "IoT@2022"; // Enter WiFi password
// client Info
// const char *mqtt_server = "45.32.111.51";
// const char *mqtt_server = "390acf6596d1409b8efba47ae295ef9b.s2.eu.hivemq.cloud";
const char *mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char *mqttUser = "Esp32";
const char *mqttPass = "Esp32";
// Topics Machine For chat
/*Fan Controller Topic*/
const char *fan_control_topic = "****"; // Sub Controller Machine Topic
const char *fan_control_start = "1";
const char *fan_control_stop = "0";
// publish the machine status topic
#define PUB_Topic_DHT "CEIT-iot/Hydroponic/DHTinsite"
#define PUB_Topic_PH "CEIT-iot/Hydroponic/PHsensor1"
// #define PUB_Topic_out "CEIT-iot/Mushroom/DHToutsite"
//subscribe the machine status topic 
const char *SUB_PUM = "CEIT-iot/Hydroponic/Pum";
const char *SUB_AutoSW = "CEIT-iot/Mushroom/Auto";
#endif
