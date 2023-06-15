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

TFT_eSPI tft = TFT_eSPI();
const char *ssid = "Hang_2.4G";
const char *password = "0948315735";

//===========Prototype=================
void WELCOME_SCREEN();
void START_CONFIG_WF_SCREEN();
void STOP_CONFIG_WF_SCREEN();
void WIFI_INFOR_SCREEN();
void DASHBOARD_SCREEN();

void setup()
{
    tft.init();
    tft.setRotation(1);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        Serial.printf("WiFi Failed!\n");
        return;
    }
    // WELCOME_SCREEN();
    // START_CONFIG_WF_SCREEN();
    // STOP_CONFIG_WF_SCREEN();
    //WIFI_INFOR_SCREEN();
    DASHBOARD_SCREEN();
}

void loop()
{
}

void WELCOME_SCREEN() // done
{
    tft.fillScreen(TFT_WHITE);
    tft.setTextColor(TFT_DARKGREEN, TFT_YELLOW);
    tft.setCursor(20, 3);
    tft.setTextSize(2);
    tft.print(" Nhom 7 ");
    tft.setTextColor(TFT_RED);
    tft.setCursor(25, 23);
    tft.setTextSize(2);
    tft.print("DO AN 3");
    tft.setTextColor(TFT_RED);
    tft.setCursor(28, 45);
    tft.setTextSize(1);
    tft.print("SMART SOCKET");
    tft.drawXBitmap(0, 63, socket, 64, 64, TFT_RED);
    tft.drawXBitmap(64, 63, measure, 64, 64, TFT_DARKGREEN);
    delay(2000);
}
void START_CONFIG_WF_SCREEN() // done
{
    tft.fillScreen(TFT_WHITE);
    tft.setTextColor(TFT_DARKGREEN, TFT_YELLOW);
    tft.setCursor(5, 3);
    tft.setTextSize(2);
    tft.print("WIFICONFIG");
    tft.setTextColor(TFT_RED);
    tft.setCursor(3, 23);
    tft.setTextSize(1);
    tft.println("1. Connect to wifi: \n");
    tft.setTextColor(TFT_DARKGREEN);
    tft.println("SSID: StartConfig_WF");
    //=================
    tft.setTextColor(TFT_RED);
    tft.setCursor(3, 51);
    tft.setTextSize(1);
    tft.println("2.Login to home wifi");
    tft.setTextColor(TFT_DARKGREEN);
    tft.println("If web page don't    auto run, please goto");
    tft.setTextColor(TFT_RED);
    tft.println("APIP: 192.168.4.1 ");
    tft.setTextColor(TFT_DARKGREEN);
    tft.print("and then LOGIN");
    //=======================
    tft.drawXBitmap(5, 99, wifi, 24, 24, TFT_BLUE);
    tft.setCursor(32, 105);
    tft.setTextSize(2);
    tft.print("AP MODE");
}
void STOP_CONFIG_WF_SCREEN() // done
{
    tft.fillScreen(TFT_WHITE);
    tft.setTextColor(TFT_DARKGREEN, TFT_YELLOW);
    tft.setCursor(10, 3);
    tft.setTextSize(2);
    tft.println(" KET NOI");
    tft.print("THANH CONG");
    tft.setTextSize(1);
    tft.setCursor(5, 50);
    tft.setTextColor(TFT_RED);
    tft.print("Thiet bi dang khoi   dong lai ... ");
    tft.drawXBitmap(5, 85, restart_icon, 32, 32, TFT_BLUE);
    tft.setCursor(40, 97);
    tft.setTextSize(1);
    tft.print(" RESTARTING...");
}
void WIFI_INFOR_SCREEN() // done
{
    tft.fillScreen(TFT_WHITE);
    tft.setTextColor(TFT_DARKGREEN, TFT_YELLOW);
    tft.setCursor(30, 3);
    tft.setTextSize(2);
    tft.println(" WIFI ");
    tft.print("  DETAILS ");
    tft.setTextColor(TFT_RED);
    tft.setCursor(3, 35);
    tft.setTextSize(1);
    tft.println("1. SSID Home WiFi: \n");
    tft.setTextColor(TFT_DARKGREEN);
    tft.print("   ");
    tft.print(WiFi.SSID());
    tft.setTextColor(TFT_RED);
    tft.setCursor(3, 70);
    tft.println("2. IP Webserver: \n");
    tft.setTextColor(TFT_DARKGREEN);
    tft.print("   ");
    tft.print(WiFi.localIP());
    //=======================
    tft.drawXBitmap(5, 99, wifi, 24, 24, TFT_BLUE);
    tft.setCursor(32, 105);
    tft.setTextSize(2);
    tft.print("STA MODE");
}
void DASHBOARD_SCREEN()
{
    // tft.fillRect(10, 48, 46, 15, TFT_WHITE); 
    // tft.fillRect(78, 48, 45, 15, TFT_WHITE); 
    // tft.fillRect(10, 105, 42, 15, TFT_WHITE);
    // tft.fillRect(80, 105, 47, 15, TFT_WHITE);
    tft.fillScreen(TFT_WHITE);
    tft.setTextColor(TFT_DARKGREEN, TFT_YELLOW);
    tft.setCursor(0, 1);
    tft.setTextSize(2);
    tft.println(" DASHBOARD");

    // tft.setTextColor(TFT_RED);
    // tft.setTextSize(2);
    // tft.drawFloat(temp_value, 1, 10, 48);
    // tft.setCursor(58, 48);
    // tft.setTextSize(1);
    // tft.printf("%cC", 248);
    // // Do am ko khi
    // tft.setTextColor(TFT_BLUE);
    // tft.setTextSize(2);
    // tft.drawNumber(humi_value, 78, 48);
    // tft.setCursor(115, 48);
    // tft.print("%");
    // // Do am dat
    // tft.setTextColor(TFT_BROWN);
    // tft.setTextSize(2);
    // tft.drawNumber(soil_value, 12, 105);
    // tft.setCursor(48, 105);
    // tft.print("%");
    // // Anh sang
    // tft.setTextColor(TFT_ORANGE);
    // tft.setTextSize(1);
    // tft.drawNumber(light_value, 82, 105);
    // tft.setCursor(90, 115);
    // tft.setTextSize(1);
    // tft.print("lux");
}