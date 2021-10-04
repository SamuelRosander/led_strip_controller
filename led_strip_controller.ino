#include "config.h"
#include <WiFiNINA.h>
#include <Adafruit_NeoPixel.h>
#include <MQTT.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

//config
String stripName = STRIPNAME;
char mqttClientID[] = STRIPNAME;
int ledDelay = conf_ledDelay; //delay between each frame
int ledMiddleOffset = conf_ledMiddleOffset; //used by the scene chaseMiddle to offset the middle of the strip
int ledOffsetWidth = conf_ledOffsetWidth; //the length of one "block" in the chase effect
int ledColorOffsetWidth = conf_ledColorOffsetWidth; //the length of one full color spectrum
int ledColorOffsetSpeed = conf_ledColorOffsetSpeed; //the number of steps the color will change each frame. Higher number = faster movement
int ledEffectChaseWidth = conf_ledEffectChaseWidth; //the length of the tail in the chase effect
int ledEffectBreatheFrames = conf_ledEffectBreatheFrames; //the total number of frames for one whole cycle of the breathe effect

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
WiFiClient net;
MQTTClient client;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIN, NEO_GRBW + NEO_KHZ800);

String powerStatus = "ON"; //valid values are ON and OFF
int ledColor = 0; //the final color set by strip.Color
int ledColorWheel = 128; //hue range 0-255
int ledColorR = 0; //0-255
int ledColorG = 0; //0-255
int ledColorB = 0; //0-255
int ledColorW = 255; //0-255
int ledBrightness = 255; //0-255
int ledOffset = 10; //rolling number to offset all pixels and "move" the effects
int ledColorOffset = 0; //rolling number between 0-255 to offset the colors in the rainbow effects to "move" it
int currentFrame = 0; //rolling number. Used by the effects breathe and strobe
String ledEffect = "static";


void setup() {
  strip.begin();
  strip.setBrightness(ledBrightness);
  ledColor = strip.Color(ledColorR, ledColorG, ledColorB, ledColorW);
  updateLEDs();

  WiFi.begin(ssid, pass);
  client.begin(SECRET_MQTT_SERVER, net);
  client.onMessage(messageReceived);
  connect();

  client.publish(stripName + "/status", powerStatus);
  client.publish(stripName + "/effect", ledEffect);
  client.publish(stripName + "/rgb-color", String(ledColorR) + "," + String(ledColorG) + "," + String(ledColorB));
  client.publish(stripName + "/temp", String(500));
  client.publish(stripName + "/brightness", String(ledBrightness));
  client.publish(stripName + "/config/ledDelay", String(ledDelay));
  client.publish(stripName + "/config/ledOffsetWidth", String(ledOffsetWidth));
  client.publish(stripName + "/config/ledColorOffsetWidth", String(ledColorOffsetWidth));
  client.publish(stripName + "/config/ledColorOffsetSpeed", String(ledColorOffsetSpeed));
  client.publish(stripName + "/config/ledEffectChaseWidth", String(ledEffectChaseWidth));
  client.publish(stripName + "/config/ledEffectBreatheFrames", String(ledEffectBreatheFrames));
}


void loop() {
  client.loop();

  if (!client.connected()) {
    connect();
  }

  updateLEDs();
  delay(ledDelay);
}


// connects to wifi and mqtt
void connect() {
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }

  while (!client.connect(mqttClientID, SECRET_MQTTUSER, SECRET_MQTTPASS)) {
    delay(1000);
  }


  client.subscribe(stripName + "/status");
  client.subscribe(stripName + "/brightness");
  client.subscribe(stripName + "/rgb-color");
  client.subscribe(stripName + "/temp");
  client.subscribe(stripName + "/effect");
  client.subscribe(stripName + "/config/+");
}

// handles incomming MQTT messages
void messageReceived(String &topic, String &payload) {
  if (topic == stripName + "/status") {
    powerStatus = payload;
  }
  if (topic == stripName + "/brightness") {
    ledBrightness = payload.toInt();
    strip.setBrightness(ledBrightness);
  }
  if (topic == stripName + "/rgb-color") {
    ledColorR = getValue(payload,',',0).toInt();
    ledColorG = getValue(payload,',',1).toInt();
    ledColorB = getValue(payload,',',2).toInt();
    ledColorW = 0;
    ledColor = strip.Color(ledColorR, ledColorG, ledColorB, ledColorW);
  }
  if (topic == stripName + "/temp") {
    ledColorR = 0;
    ledColorG = 0;
    ledColorB = map(payload.toInt(), 153, 500, 255, 0);
    ledColorW = 255;
    ledColor = strip.Color(ledColorR, ledColorG, ledColorB, ledColorW);
  }
  if (topic == stripName + "/effect") {
    ledEffect = payload;
  }
  if (topic.startsWith(stripName + "/config/")) {
    String configVar = topic.substring(24);
    if (configVar == "ledDelay") {
      ledDelay = payload.toInt();
    }
    else if (configVar == "ledOffsetWidth") {
      ledOffsetWidth = payload.toInt();
    }
    else if (configVar == "ledColorOffsetWidth") {
      ledColorOffsetWidth = payload.toInt();
    }
    else if (configVar == "ledColorOffsetSpeed") {
      ledColorOffsetSpeed = payload.toInt();
    }
    else if (configVar == "ledEffectChaseWidth") {
      ledEffectChaseWidth = payload.toInt();
    }
    else if (configVar == "ledEffectBreatheFrames") {
      ledEffectBreatheFrames = payload.toInt();
    }
  }
}

// called each frame to set the current effect
void updateLEDs() {
  if (powerStatus == "OFF") {
    for (int i=0; i<PIXEL_COUNT; i++) {
      strip.setPixelColor(i, (0,0,0));
    }
  }
  else if (ledEffect == "static"){
    ledEffectStatic();
  }
  else if (ledEffect == "rainbow"){
    ledEffectRainbow();
  }
  else if (ledEffect == "rainbowMiddle") {
    ledEffectRainbowMiddle();
  }
  else if (ledEffect == "chase") {
    ledEffectChase();
  }
  else if (ledEffect == "chaseMiddle") {
    ledEffectChaseMiddle();
  }
  else if (ledEffect == "rainbowChase") {
    ledEffectRainbowChase();
  }
  else if (ledEffect == "fade") {
    ledEffectFade();
  }
  else if (ledEffect == "fadeChase") {
    ledEffectFadeChase();
  }
  else if (ledEffect == "breathe") {
    ledEffectBreathe();
  }
  else if (ledEffect == "breatheFade") {
    ledEffectBreatheFade();
  }
  else if (ledEffect == "strobe") {
    ledEffectStrobe();
  }
  else if (ledEffect == "strobeFade") {
    ledEffectStrobeFade();
  }

  strip.show();
}


void ledEffectStatic() {
  for (int i=0; i<PIXEL_COUNT; i++) {
    strip.setPixelColor(i, ledColor);
  }
}


void ledEffectRainbow() {
  for (int i=0; i<PIXEL_COUNT; i++) {
    strip.setPixelColor(i, Wheel((i * 256 / ledColorOffsetWidth + ledColorOffset) % 256, 255));
  }
  ledColorOffset = (ledColorOffset + ledColorOffsetSpeed) % 256;
}


void ledEffectRainbowMiddle() {
  for (int i = 0; i < PIXEL_COUNT; i++) {
    if (i < PIXEL_COUNT/2) {
      strip.setPixelColor(i, Wheel((i * 256 / PIXEL_COUNT + ledColorOffset) % 256, 255));
    }
    else {
      strip.setPixelColor(i, Wheel(((PIXEL_COUNT - i - 1) * 256 / PIXEL_COUNT + ledColorOffset) % 256, 255));
    }
  }
  ledColorOffset = (ledColorOffset + ledColorOffsetSpeed) % 256;
}


void ledEffectChase() {
  for (int i=0; i<PIXEL_COUNT; i++) {
    if (((i+ledOffset) % ledOffsetWidth) < ledEffectChaseWidth ) {
      strip.setPixelColor(i, getColor(ledColorR, ledColorG, ledColorB, ledColorW, 255 - (((i + ledOffset) % ledOffsetWidth) * 255 / ledEffectChaseWidth)));
    }
    else {
      strip.setPixelColor(i, strip.Color(0,0,0));
    }
  }
  ledOffset = (ledOffset + 1) % ledOffsetWidth;
}


void ledEffectChaseMiddle() {
  for (int i=0; i<PIXEL_COUNT/2; i++) {
    if (((i+ledOffset) % ledOffsetWidth) < ledEffectChaseWidth) {
      strip.setPixelColor((i + ledMiddleOffset + PIXEL_COUNT) % PIXEL_COUNT, getColor(ledColorR, ledColorG, ledColorB, ledColorW, 255 - (((i + ledOffset) % ledOffsetWidth) * 255 / ledEffectChaseWidth)));
      strip.setPixelColor((PIXEL_COUNT * 2 - 1 - i + ledMiddleOffset) % PIXEL_COUNT, getColor(ledColorR, ledColorG, ledColorB, ledColorW, 255 - (((i + ledOffset) % ledOffsetWidth) * 255 / ledEffectChaseWidth)));
    }
    else {
      strip.setPixelColor((i + ledMiddleOffset + PIXEL_COUNT) % PIXEL_COUNT, strip.Color(0,0,0));
      strip.setPixelColor((PIXEL_COUNT * 2 - 1 - i + ledMiddleOffset) % PIXEL_COUNT, strip.Color(0,0,0));
    }
  }
  ledOffset = (ledOffset + 1) % ledOffsetWidth;
}


void ledEffectRainbowChase() {
  for (int i=0; i<PIXEL_COUNT; i++) {
    if (((i+ledOffset) % ledOffsetWidth) < ledEffectChaseWidth ) {
      strip.setPixelColor(i, Wheel((i * 256 / ledColorOffsetWidth + ledColorOffset) % 256, 255 - (((i + ledOffset) % ledOffsetWidth) * 255 / ledEffectChaseWidth)));
    }
    else {
      strip.setPixelColor(i, strip.Color(0,0,0));
    }
  }
  ledOffset = (ledOffset + 1) % ledOffsetWidth;
  ledColorOffset = (ledColorOffset + ledColorOffsetSpeed) % 256;
}


void ledEffectFade() {
  for (int i=0; i<PIXEL_COUNT; i++) {
    strip.setPixelColor(i, Wheel(ledColorWheel, 255));
  }
  ledColorWheel = (ledColorWheel + 1) % 256;
}


void ledEffectFadeChase() {
  for (int i=0; i<PIXEL_COUNT; i++) {
    if (((i+ledOffset) % ledOffsetWidth) < ledEffectChaseWidth ) {
      strip.setPixelColor(i, Wheel(ledColorWheel, 255 - (((i + ledOffset) % ledOffsetWidth) * 255 / ledEffectChaseWidth)));
    }
    else {
      strip.setPixelColor(i, strip.Color(0,0,0));
    }
  }
  ledOffset = (ledOffset + 1) % ledOffsetWidth;
  ledColorWheel = (ledColorWheel + 1) % 256;
}


void ledEffectBreathe() {
  if (currentFrame < ledEffectBreatheFrames / 3) {
    for (int i=0; i<PIXEL_COUNT; i++) {
      strip.setPixelColor(i, strip.Color(0,0,0));
    }
  }
  else if (currentFrame < (ledEffectBreatheFrames / 3) * 2){
    for (int i=0; i<PIXEL_COUNT; i++) {
      ledBrightness = map(currentFrame, ledEffectBreatheFrames / 3, ledEffectBreatheFrames * 2 / 3, 0, 255);
      strip.setPixelColor(i, getColor(ledColorR, ledColorG, ledColorB, ledColorW, ledBrightness));
    }
  }
  else {
    for (int i=0; i<PIXEL_COUNT; i++) {
      ledBrightness = map(currentFrame, ledEffectBreatheFrames * 2/ 3, ledEffectBreatheFrames - 1, 255, 0);
      strip.setPixelColor(i, getColor(ledColorR, ledColorG, ledColorB, ledColorW, ledBrightness));
    }
  }
  currentFrame = (currentFrame + 1) % ledEffectBreatheFrames;
}


void ledEffectBreatheFade() {
  if (currentFrame < ledEffectBreatheFrames / 3) {
    for (int i=0; i<PIXEL_COUNT; i++) {
      strip.setPixelColor(i, strip.Color(0,0,0));
    }
  }
  else if (currentFrame < (ledEffectBreatheFrames / 3) * 2){
    for (int i=0; i<PIXEL_COUNT; i++) {
      strip.setPixelColor(i, Wheel(ledColorWheel, map(currentFrame, ledEffectBreatheFrames / 3, ledEffectBreatheFrames * 2 / 3, 0, 255)));
    }
  }
  else {
    for (int i=0; i<PIXEL_COUNT; i++) {
      strip.setPixelColor(i, Wheel(ledColorWheel, map(currentFrame, ledEffectBreatheFrames * 2/ 3, ledEffectBreatheFrames - 1, 255, 0)));
    }
  }
  currentFrame = (currentFrame + 1) % ledEffectBreatheFrames;
  ledColorWheel = (ledColorWheel + 1) % 256;
}


void ledEffectStrobe() {
  if (currentFrame % 2 == 0) {
    for (int i=0; i<PIXEL_COUNT; i++) {
      strip.setPixelColor(i, ledColor);
    }
  }
  else {
    for (int i=0; i<PIXEL_COUNT; i++) {
      strip.setPixelColor(i, strip.Color(0,0,0));
    }
  }
  currentFrame = (currentFrame + 1) % 2;
}


void ledEffectStrobeFade() {
  if (currentFrame % 2 == 0) {
    for (int i=0; i<PIXEL_COUNT; i++) {
      strip.setPixelColor(i, Wheel(ledColorWheel, 255));
    }
  }
  else {
    for (int i=0; i<PIXEL_COUNT; i++) {
      strip.setPixelColor(i, strip.Color(0,0,0));
    }
  }
  currentFrame = (currentFrame + 1) % 2;
  ledColorWheel = (ledColorWheel + 1) % 256;
}


// returns the fully saturated color at position [WheelPos] from the hue color spectrum
// WheelPos and brightness values needs to be between 0-255
uint32_t Wheel(int WheelPos, int brightness) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color((255 - WheelPos * 3) * brightness / 255, 0, WheelPos * 3 * brightness / 255);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3 * brightness / 255, (255 - WheelPos * 3) * brightness / 255);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3 * brightness / 255, (255 - WheelPos * 3) * brightness / 255, 0);
}


// returns the same color with adjusted brightness
// all input values needs to be between 0-255
int getColor(int r, int g, int b, int w, int brightness) {
  r = r * brightness / 255;
  g = g * brightness / 255;
  b = b * brightness / 255;
  w = w * brightness / 255;
  return strip.Color(r, g, b, w);
}


// splits the input string [data] at every [seperator] and returns the substring at position [index]
String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
