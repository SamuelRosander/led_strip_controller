#define SECRET_SSID "your_wifi_name"
#define SECRET_PASS "your_wifi_password"
#define SECRET_MQTT_SERVER "your.mqtt.server.com"
#define SECRET_MQTTUSER "your_mqtt_user"
#define SECRET_MQTTPASS "your_mqtt_pasword"

#define PIN 6
#define PIXEL_COUNT 338

#define STRIPNAME "hallway-ledstrip"

#define conf_ledDelay 30 //delay between each frame in ms
#define conf_ledMiddleOffset 70 //used by the scene chaseMiddle to offset the middle of the strip
#define conf_ledOffsetWidth 26 //the length of one "block" in the chase effect
#define conf_ledColorOffsetWidth PIXEL_COUNT //the length of one full color spectrum
#define conf_ledColorOffsetSpeed 5 //the number of steps the color will change each frame. Higher number = faster movement
#define conf_ledEffectChaseWidth 20 //the length of the tail in the chase effect
#define conf_ledEffectBreatheFrames 120 //the total number of frames for one whole cycle of the breathe effect
#define conf_ledEffectBaFWidth 3
