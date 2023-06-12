#include <Arduino.h>
#include "SPIFFS.h"
#include <WiFiManager.h>
#include <Arduino_JSON.h>
#include <PZEM004Tv30.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
#endif
#define PZEM_SERIAL Serial2
#define TRIGGER_PIN 0
PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);
// State button
bool mainButton = 1;
bool state_SK1 = 0;
bool state_SK2 = 0;
bool state_SK3 = 0;

bool wifiConnected = false; // flag to track successful WiFi connection
bool shouldRestart = false; // flag to track if restart is needed
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
// const char *ssid = "Hang_2.4G";
// const char *password = "0948315735";

const char *PARAM_MESSAGE = "message";

unsigned long previousMillis = 0;

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
    dataPower["Energy"] = String(energyValue);
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
            Serial.println(mainButton);
            ws.textAll(getStateButton());
            //  ws.textAll("001");
        }
        else if (strcmp((char *)data, "onsk1") == 0)
        {
            state_SK1 = true;
            Serial.println("SK 1 on");
            ws.textAll(getStateButton());
            //  ws.textAll("010");
        }
        else if (strcmp((char *)data, "offsk1") == 0)
        {
            state_SK1 = false;
            Serial.println("SK 1 off");
            ws.textAll(getStateButton());
            // ws.textAll("011");
        }
        else if (strcmp((char *)data, "onsk2") == 0)
        {
            state_SK2 = true;
            Serial.println("SK 2 on");
            ws.textAll(getStateButton());
            // ws.textAll("100");
        }
        else if (strcmp((char *)data, "offsk2") == 0)
        {
            state_SK2 = false;
            Serial.println("SK 2 off");
            ws.textAll(getStateButton());
            // ws.textAll("101");
        }
        else if (strcmp((char *)data, "onsk3") == 0)
        {
            state_SK3 = true;
            Serial.println("SK 3 on");
            ws.textAll(getStateButton());
            // ws.textAll("110");
        }
        else if (strcmp((char *)data, "offsk3") == 0)
        {
            state_SK3 = false;
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

void setup()
{
    WiFi.mode(WIFI_STA);
    Serial.begin(115200);
    // initFS();
    // WiFi.begin(ssid, password);
    // if (WiFi.waitForConnectResult() != WL_CONNECTED)
    // {
    //     Serial.printf("WiFi Failed!\n");
    //     return;
    // }
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
    }
}
void loop()
{
    wm.process();
    checkButton();
    checkWifi_config();  // restart if reset wifi and config again
    ws.cleanupClients(); // dọn dẹp client không được sử dụng
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= 1500)
    {
        // Thực hiện các hoạt động sau mỗi interval
        readPzem();      // chạy thật dụng hàm này
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
            // still holding button for 3000 ms, reset settings, code not ideaa for production
            delay(3000); // reset delay hold
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
        ESP.restart();
    }
}