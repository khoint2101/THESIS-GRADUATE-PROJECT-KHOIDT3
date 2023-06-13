#include <Arduino.h>
#include "SPIFFS.h"
#include <WiFiManager.h>
#include <Arduino_JSON.h>
#include <PZEM004Tv30.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include "bitmap.h"

#define TFT_W 128
#define TFT_H 128

TFT_eSPI tft = TFT_eSPI();

void setup(){
    tft.init();
    tft.setRotation(1);
    tft.setSwapBytes(true);
    tft.fillScreen(TFT_WHITE);
}

void loop(){
    tft.pushImage(10,10,64,65,wifiQR);
}