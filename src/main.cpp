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

#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
#endif
#define PZEM_SERIAL Serial2
#define TRIGGER_PIN 0
#define IN1_RELAY 12
#define IN2_RELAY 14
#define IN3_RELAY 27
PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);
TFT_eSPI tft = TFT_eSPI();
// State button
bool mainButton = 1;
bool state_SK1 = 0;
bool state_SK2 = 0;
bool state_SK3 = 0;
bool shouldRestart = false; // flag to track if restart is needed
byte screenChange = 0;      // switch between wifi detail and dashboard

// define variable
float voltageValue;
float currentValue;
float powerValue;
float energyValue;
float pfValue;
float freqValue;

//========================
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
WiFiManager wm; // global wm instance
//=======FUNCTION PROTOTYPE===========
void readPzem();
void checkButton();
void checkWifi_config();
void MainScreenChange();
//-----Screen--------
void WELCOME_SCREEN();
void START_CONFIG_WF_SCREEN();
void STOP_CONFIG_WF_SCREEN();
void WIFI_INFOR_SCREEN();
void DASHBOARD_SCREEN();
void VALUE_DASHBOARD_SCREEN();

//==============End prototype===========
// const char *ssid = "Hang_2.4G";
// const char *password = "0948315735";

const char *PARAM_MESSAGE = "message";

unsigned long previousMillis = 0;
unsigned long previousMillis1 = 0;

//======Para==============

void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}
void initFS()
{
    if (!SPIFFS.begin())
    {
        Serial.println("An error has occurred while mounting SPIFFS");
    }
    else
    {
        Serial.println("SPIFFS mounted successfully");
    }
}
// Json Variable to StateButton
JSONVar stateButton, dataPower;

// Get State Button
String getStateButton()
{
    stateButton["Type"] = "button";
    stateButton["MainButton"] = String(mainButton);
    stateButton["Button1"] = String(state_SK1);
    stateButton["Button2"] = String(state_SK2);
    stateButton["Button3"] = String(state_SK3);

    String jsonString_button = JSON.stringify(stateButton);
    return jsonString_button;
}
//=============================================
// JSON of data
String getDataPower()
{
    dataPower["Type"] = "data";
    dataPower["Volt"] = String(voltageValue);
    dataPower["Current"] = String(currentValue);
    dataPower["Power"] = String(powerValue);
    dataPower["Energy"] = String(energyValue, 4);
    dataPower["Freq"] = String(freqValue);
    dataPower["Pf"] = String(pfValue);

    String jsonString_data = JSON.stringify(dataPower);
    return jsonString_data;
}
void notifyClients(String x) // notice to all client
{
    ws.textAll(x);
}
/*Xử lý truyền nhận data khi bấm nút thành công-  Chưa có hiển thị trạng thái nút đang dùng */
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        data[len] = 0;
        if (strcmp((char *)data, "onall") == 0)
        {
            mainButton = false;
            state_SK1 = true;
            state_SK2 = true;
            state_SK3 = true;

            digitalWrite(IN1_RELAY, 1);
            digitalWrite(IN2_RELAY, 1);
            digitalWrite(IN3_RELAY, 1);
            Serial.println(mainButton);
            ws.textAll(getStateButton());
            // ws.textAll("000");
        }
        else if (strcmp((char *)data, "offall") == 0)
        {
            mainButton = true;
            state_SK1 = false;
            state_SK2 = false;
            state_SK3 = false;
            digitalWrite(IN1_RELAY, 0);
            digitalWrite(IN2_RELAY, 0);
            digitalWrite(IN3_RELAY, 0);
            Serial.println(mainButton);
            ws.textAll(getStateButton());
            //  ws.textAll("001");
        }
        else if (strcmp((char *)data, "onsk1") == 0)
        {
            state_SK1 = true;
            digitalWrite(IN1_RELAY, 1);
            Serial.println("SK 1 on");
            ws.textAll(getStateButton());
            //  ws.textAll("010");
        }
        else if (strcmp((char *)data, "offsk1") == 0)
        {
            state_SK1 = false;
            Serial.println("SK 1 off");
            digitalWrite(IN1_RELAY, 0);
            ws.textAll(getStateButton());
            // ws.textAll("011");
        }
        else if (strcmp((char *)data, "onsk2") == 0)
        {
            state_SK2 = true;
            digitalWrite(IN2_RELAY, 1);
            Serial.println("SK 2 on");
            ws.textAll(getStateButton());
            // ws.textAll("100");
        }
        else if (strcmp((char *)data, "offsk2") == 0)
        {
            state_SK2 = false;
            digitalWrite(IN2_RELAY, 0);
            Serial.println("SK 2 off");
            ws.textAll(getStateButton());
            // ws.textAll("101");
        }
        else if (strcmp((char *)data, "onsk3") == 0)
        {
            state_SK3 = true;
            digitalWrite(IN3_RELAY, 1);
            Serial.println("SK 3 on");
            ws.textAll(getStateButton());
            // ws.textAll("110");
        }
        else if (strcmp((char *)data, "offsk3") == 0)
        {
            state_SK3 = false;
            digitalWrite(IN3_RELAY, 0);
            Serial.println("SK 3 off");
            ws.textAll(getStateButton());
            // ws.textAll("111");
        }
        else if (strcmp((char *)data, "getBtn") == 0)
        {
            notifyClients(getStateButton());
            // ws.textAll("111");
        }
    }
}
/*============================================================================*/
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
    switch (type)
    {
    case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());

        break;
    case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
    case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}

void initWebSocket()
{
    ws.onEvent(onEvent);
    server.addHandler(&ws);
}

void initRelayPin()
{
    pinMode(IN1_RELAY, OUTPUT);
    pinMode(IN2_RELAY, OUTPUT);
    pinMode(IN3_RELAY, OUTPUT);
}

void setup()
{
    WiFi.mode(WIFI_STA);
    Serial.begin(115200);

    tft.init();
    tft.setRotation(1);
    WELCOME_SCREEN();
    initRelayPin();
    Serial.setDebugOutput(true);
    // delay(3000);   // if you won't change port, you must hard restart
    // wm.setHttpPort(8080); // set another port for WM because https://github.com/rancilio-pid/clevercoffee/issues/323
    Serial.println("\n Starting");
    pinMode(TRIGGER_PIN, INPUT);
    wm.setConfigPortalBlocking(false);
    // std::vector<const char *> menu = {"wifi", "info", "sep", "restart", "exit"};
    // wm.setMenu(menu);
    // set dark theme
    wm.setClass("invert");
    wm.setConfigPortalTimeout(60); // auto close configportal after n seconds
    bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    res = wm.autoConnect("StartConfig_WF"); // password protected ap

    if (!res)
    {
        Serial.println("Failed to connect or hit timeout");
        // ESP.restart();
        START_CONFIG_WF_SCREEN();
        shouldRestart = true;
    }
    else
    {
        // if you get here you have connected to the WiFi   // xử lý bằng QR code
        Serial.println("connected...yeey :)"); // Sau khi thiết lập một wifi mới sẽ bị trùng port cần khắc phục
        initFS();
        initWebSocket();
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                  { request->send(SPIFFS, "/index.html", "text/html"); });

        // Send a GET request to <IP>/get?message=<message
        // Send a POST request to <IP>/post with a form field message set to <message>

        server.onNotFound(notFound);
        server.serveStatic("/", SPIFFS, "/");
        server.begin();
        screenChange = 1; // screen wifi details
        tft.fillScreen(TFT_WHITE);
    }
}
void loop()
{
    unsigned long currentMillis = millis();
    wm.process();
    checkButton();
    checkWifi_config();  // restart if reset wifi and config again
    ws.cleanupClients(); // dọn dẹp client không được sử dụng
    MainScreenChange();

    if (currentMillis - previousMillis >= 1500)
    {
        // Thực hiện các hoạt động sau mỗi interval
         readPzem(); // chạy thật dụng hàm này
        // voltageValue = random(100, 260);
        // currentValue = random(0.0, 30.5);
        // powerValue = random(100, 4000);
        // energyValue = random(1, 100);
        // pfValue = random(0.0, 10.0);
        // freqValue = random(40.0, 50.0);

        notifyClients(getDataPower());

        previousMillis = currentMillis;
    }
}
void checkButton()
{
    // check for button press
    if (digitalRead(TRIGGER_PIN) == LOW)
    {
        // poor mans debounce/press-hold, code not ideal for production
        delay(50);
        if (digitalRead(TRIGGER_PIN) == LOW)
        {
            Serial.println("Button Pressed");
            screenChange += 1;
            if (screenChange == 3)
                screenChange = 1;
            // still holding button for 3000 ms, reset settings, code not ideaa for production
            delay(3000); // reset delay hold
            tft.fillScreen(TFT_WHITE);
            if (digitalRead(TRIGGER_PIN) == LOW)
            {
                Serial.println("Button Held");
                Serial.println("Erasing Config, restarting");
                wm.resetSettings();
                ESP.restart();
            }
        }
    }
}

void MainScreenChange()
{
    if (screenChange == 1)
    {
        WIFI_INFOR_SCREEN();
    }
    else if (screenChange == 2)
    {
        // tft.fillScreen(TFT_WHITE);
        unsigned long currentMillis1 = millis();
        DASHBOARD_SCREEN();
        if (currentMillis1 - previousMillis1 >= 1500)
        {
            VALUE_DASHBOARD_SCREEN();
            previousMillis1 = currentMillis1;
        }
    }
}
//==========Workspace with Pzem=========
//@param Đọc giá trị
// Set mức cảnh báo + còi + led
// reset các thông số đo bằng hàm resetEnergy();
void readPzem()
{
    // Print the custom address of the PZEM
    Serial.print("Custom Address:");
    Serial.println(pzem.readAddress(), HEX);

    // Read the data from the sensor
    voltageValue = pzem.voltage();
    currentValue = pzem.current();
    powerValue = pzem.power();
    energyValue = pzem.energy();
    freqValue = pzem.frequency();
    pfValue = pzem.pf();

    // Check if the data is valid
    if (isnan(voltageValue))
    {
        Serial.println("Error reading voltage");
        currentValue = 0;
        powerValue = 0;
    }
    else if (isnan(currentValue))
    {
        Serial.println("Error reading current");
    }
    else if (isnan(powerValue))
    {
        Serial.println("Error reading power");
    }
    else if (isnan(energyValue))
    {
        Serial.println("Error reading energy");
    }
    else if (isnan(freqValue))
    {
        Serial.println("Error reading frequency");
    }
    else if (isnan(pfValue))
    {
        Serial.println("Error reading power factor");
    }
    else
    {

        // Print the values to the Serial console
        Serial.print("Voltage: ");
        Serial.print(voltageValue);
        Serial.println("V");
        Serial.print("Current: ");
        Serial.print(currentValue);
        Serial.println("A");
        Serial.print("Power: ");
        Serial.print(powerValue);
        Serial.println("W");
        Serial.print("Energy: ");
        Serial.print(energyValue, 3);
        Serial.println("kWh");
        Serial.print("Frequency: ");
        Serial.print(freqValue, 1);
        Serial.println("Hz");
        Serial.print("PF: ");
        Serial.println(pfValue);
    }

    Serial.println();
}

void checkWifi_config()
{
    // nếu wifi config thành công sau khi reset thông tin wifi thì sẽ reset để mở lại port 80 cho webserver
    if ((WiFi.status() == WL_CONNECTED) && shouldRestart == true)
    {
        STOP_CONFIG_WF_SCREEN();
        delay(3000);
        ESP.restart();
    }
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
    // tft.fillScreen(TFT_WHITE);
    tft.setTextColor(TFT_DARKGREEN, TFT_YELLOW);
    tft.setCursor(30, 3);
    tft.setTextSize(2);
    tft.println(" WIFI ");
    tft.print("  DETAILS ");
    tft.setTextColor(TFT_RED);
    tft.setCursor(3, 35);
    tft.setTextSize(1);
    tft.println("1. SSID Home WiFi: \n");

    tft.setTextColor(TFT_RED);
    tft.setCursor(3, 70);
    tft.println("2. IP Webserver: \n");

    if (WiFi.status() != WL_CONNECTED)
    {
        tft.fillCircle(100, 54, 7, TFT_RED);
        tft.fillRect(17, 50, 45, 10, TFT_WHITE);
        tft.fillRect(17, 84, 100, 10, TFT_WHITE);
    }
    else
    {
        tft.setTextColor(TFT_DARKGREEN);
        tft.setCursor(20, 50);
        tft.print(WiFi.SSID());
        tft.setTextColor(TFT_DARKGREEN);
        tft.setCursor(20, 84);
        tft.print(WiFi.localIP());
        tft.fillCircle(100, 54, 7, TFT_GREEN);
    }
    //=======================
    tft.drawXBitmap(5, 99, wifi, 24, 24, TFT_BLUE);
    tft.setCursor(32, 105);
    tft.setTextSize(2);
    tft.print("STA MODE");
}
void DASHBOARD_SCREEN()
{

    tft.setTextColor(TFT_DARKGREEN, TFT_YELLOW);
    tft.setCursor(0, 1);
    tft.setTextSize(2);
    tft.println(" DASHBOARD");

    tft.setTextColor(TFT_NAVY);
    tft.setTextSize(1);
    tft.setCursor(5, 20);
    tft.print("1. VOLTAGE:");
    tft.setCursor(5, 50);
    tft.print("2. CURRENT:");
    tft.setCursor(5, 80);
    tft.print("3. POWER:");
    tft.setCursor(5, 110);
    tft.print("4. ENERGY:");
}
void VALUE_DASHBOARD_SCREEN()
{
    tft.fillRect(5, 28, 95, 20, TFT_WHITE);
    tft.fillRect(20, 58, 80, 20, TFT_WHITE);
    tft.fillRect(20, 88, 80, 20, TFT_WHITE);
    tft.fillRect(65, 109, 40, 20, TFT_WHITE);

    tft.setTextColor(TFT_RED);
    tft.setTextSize(1);
    if (isnan(voltageValue))
    {
        tft.setCursor(10, 35);
        tft.print("Can't Read!! ");
    }
    else
    {
        tft.drawFloat(voltageValue, 1, 50, 35);
    }
    tft.setCursor(100, 34);
    tft.print(" V");
    tft.drawFloat(currentValue, 2, 50, 65);
    tft.setCursor(100, 64);
    tft.print(" A");
    tft.drawFloat(powerValue, 1, 40, 95);
    tft.setCursor(100, 94);
    tft.print(" W");
    tft.drawFloat(energyValue, 4, 65, 109);
    tft.setCursor(100, 109);
    tft.print(" kWh");
}