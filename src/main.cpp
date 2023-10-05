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
#include <WiFiClient.h>
#include <Firebase_ESP_Client.h>
#include "bitmap.h"
#include <NTPClient.h>
#include <EEPROM.h>
#include <WiFiUdp.h>

#include "addons/TokenHelper.h" //Token Generation Firebase
#include "addons/RTDBHelper.h"  // RTDB payload

#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
#endif
#define PZEM_SERIAL Serial2
#define TRIGGER_PIN 0
#define RESET_PZEM_PIN 22
#define NORMAL_LED_PIN 5
#define IDLE_LED_PIN 25
#define ALARM_LED_PIN 19
#define IN1_RELAY_PIN 12
#define IN2_RELAY_PIN 14
#define IN3_RELAY_PIN 27
#define SW_RELAY1_PIN 13
#define SW_RELAY2_PIN 21
#define SW_RELAY3_PIN 26

#define API_KEY "AIzaSyA63-WGsXRJKT3ididiQOBMETy8T1tW3qk" // replace the API key
#define USER_EMAIL "adminhardwareauthen@gmail.com"
#define USER_PASSWORD "12345678"
#define DATABASE_URL "smartsocket-thesis-default-rtdb.asia-southeast1.firebasedatabase.app"
#define DATABASE_SECRET "pzBkWea64aXi2g0kUnuxwEDUuGdj6Ft25YJcX9GP"

PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);
TFT_eSPI tft = TFT_eSPI();

WiFiUDP ntpUDP; // get time from Internet
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7 * 3600, 60000);

hw_timer_t *timer = NULL; // timer interrupt
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// State button
bool toggle_led = 0;
bool mainButton = 1;
bool state_SK1 = 0;
bool state_SK2 = 0;
bool state_SK3 = 0;
bool shouldRestart = false; // flag to track if restart is needed
bool send_flag_idle = true; // check idle status avoid interrupt when sending to Firebase
bool flag_webserver_handle = false;
bool led_alert = 0;
byte screenChange = 0; // switch between wifi detail and dashboard
uint32_t chipID = 0;
// unsigned long timer_Fb;
String listenerPath;
String chipIDstr;
uint8_t checkNew;
uint16_t counter1000 = 0, counter5000 = 0, counter7500 = 0, counter2000 = 0, counter15000 = 0;

int mainHour, mainMin, mainSec, hour1, min1, hour2, min2, hour3, min3, sec1 = 0, sec2 = 0, sec3 = 0;
int flag_webserver_socket_name = 0;
int delEnergyButtonState, powerAlertNumber, powerAlertStt, timerStt1, timerStt2, timerStt3;
String timerValue1, timerValue2, timerValue3;

// define variable Pzem 004T
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
FirebaseData stream;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
//=======FUNCTION PROTOTYPE===========
void readPzem();
void checkButton();
void checkWifi_config();
void MainScreenChange();
void resetValuePzem();
void checkAlarmPower();
void sendDataToRTDB();   // send data to RealTime Database
void SetupControlRTDB(); // init the first time
void SetOffAlarm();
void ControlRelay();
void StringToTimeConvert(String time_str, int *hour, int *min);
bool CompareTime(int hourNTP, int minNTP, int secNTP, int hourSetup, int minSetup, int secSetup);
//-----Screen--------
void WELCOME_SCREEN();
void START_CONFIG_WF_SCREEN();
void STOP_CONFIG_WF_SCREEN();
void WIFI_INFOR_SCREEN();
void DASHBOARD_SCREEN();
void TIME_UID_SCREEN();
void VALUE_DASHBOARD_SCREEN();

//==============End prototype===========

const char *PARAM_MESSAGE = "message";
//========Timer Variable==========
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
// Json Variable ==============
JSONVar stateButton, dataPower;

// ==============JSON STRING=================
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
//---------------------------
// Get data to StringJSON
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
//==============END JSON STRING=================

void notifyClients(String x) // notice to all client
{
    ws.textAll(x);
}
/*Xử lý truyền nhận data khi bấm nút thành công */
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

            flag_webserver_handle = true;
            flag_webserver_socket_name = 0;
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
            flag_webserver_handle = true;
            flag_webserver_socket_name = 0;
            Serial.println(mainButton);
            ws.textAll(getStateButton());

            //  ws.textAll("001");
        }
        else if (strcmp((char *)data, "onsk1") == 0)
        {
            state_SK1 = true;
            flag_webserver_handle = true;
            flag_webserver_socket_name = 1;
            // digitalWrite(IN1_RELAY_PIN, state_SK1);
            //  if (send_flag_idle == true)
            //      Serial.printf("Send SK1....%s\n", Firebase.RTDB.setInt(&fbdo, chipIDstr + "/control/socket1", (int)state_SK1) ? "ok" : fbdo.errorReason().c_str());
            Serial.println("SK 1 on");
            ws.textAll(getStateButton());
        }
        else if (strcmp((char *)data, "offsk1") == 0)
        {
            state_SK1 = false;
            flag_webserver_handle = true;
            flag_webserver_socket_name = 1;
            Serial.println("SK 1 off");
            // digitalWrite(IN1_RELAY_PIN, state_SK1);
            // if (send_flag_idle == true)
            //     Serial.printf("Send sk1....%s\n", Firebase.RTDB.setInt(&fbdo, chipIDstr + "/control/socket1", (int)state_SK1) ? "ok" : fbdo.errorReason().c_str());
            ws.textAll(getStateButton());
        }
        else if (strcmp((char *)data, "onsk2") == 0)
        {
            state_SK2 = true;
            flag_webserver_handle = true;
            flag_webserver_socket_name = 2;
            // digitalWrite(IN2_RELAY_PIN, state_SK2);
            // if (send_flag_idle == true)
            //     Serial.printf("Send sk2....%s\n", Firebase.RTDB.setInt(&fbdo, chipIDstr + "/control/socket2", (int)state_SK2) ? "ok" : fbdo.errorReason().c_str());
            Serial.println("SK 2 on");
            ws.textAll(getStateButton());
        }
        else if (strcmp((char *)data, "offsk2") == 0)
        {
            state_SK2 = false;
            flag_webserver_handle = true;
            flag_webserver_socket_name = 2;
            // digitalWrite(IN2_RELAY_PIN, state_SK2);
            // if (send_flag_idle == true)
            //     Serial.printf("Send sk2....%s\n", Firebase.RTDB.setInt(&fbdo, chipIDstr + "/control/socket2", (int)state_SK2) ? "ok" : fbdo.errorReason().c_str());
            // Serial.println("SK 2 off");
            ws.textAll(getStateButton());
        }
        else if (strcmp((char *)data, "onsk3") == 0)
        {
            state_SK3 = true;
            flag_webserver_handle = true;
            flag_webserver_socket_name = 3;
            // digitalWrite(IN3_RELAY_PIN, state_SK3);
            // if (send_flag_idle == true)
            //     Serial.printf("Send sk3....%s\n", Firebase.RTDB.setInt(&fbdo, chipIDstr + "/control/socket3", (int)state_SK3) ? "ok" : fbdo.errorReason().c_str());
            // Serial.println("SK 3 on");
            ws.textAll(getStateButton());
        }
        else if (strcmp((char *)data, "offsk3") == 0)
        {
            state_SK3 = false;
            flag_webserver_handle = true;
            flag_webserver_socket_name = 3;
            // digitalWrite(IN3_RELAY_PIN, state_SK3);
            // if (send_flag_idle == true)
            //     Serial.printf("Send sk3....%s\n", Firebase.RTDB.setInt(&fbdo, chipIDstr + "/control/socket3", (int)state_SK3) ? "ok" : fbdo.errorReason().c_str());
            // Serial.println("SK 3 off");
            ws.textAll(getStateButton());
        }
        else if (strcmp((char *)data, "getBtn") == 0)
        {
            notifyClients(getStateButton());
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
    pinMode(IN1_RELAY_PIN, OUTPUT);
    pinMode(IN2_RELAY_PIN, OUTPUT);
    pinMode(IN3_RELAY_PIN, OUTPUT);
}
//==================Stream Callback Firebase==================
void streamCallback(FirebaseStream data)
{
    Serial.printf("stream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                  data.streamPath().c_str(),
                  data.dataPath().c_str(),
                  data.dataType().c_str(),
                  data.eventType().c_str());
    printResult(data);
    Serial.println();

    String streamPath = String(data.dataPath());
    // Detect type of data
    if (data.dataTypeEnum() == fb_esp_rtdb_data_type_integer)
    {
        String namePath = streamPath.substring(1);
        Serial.print("Name Path: ");
        Serial.println(namePath);
        if (namePath == "socket1") // => điều khiển ổ cắm 1
        {
            state_SK1 = data.intData();
            Serial.print("STATE: ");
            Serial.println(state_SK1);
            // digitalWrite(IN1_RELAY_PIN, state_SK1);
            ws.textAll(getStateButton());
        }
        else if (namePath == "socket2")
        {
            state_SK2 = data.intData();
            Serial.print("STATE: ");
            Serial.println(state_SK2);
            // digitalWrite(IN2_RELAY_PIN, state_SK2);
            ws.textAll(getStateButton());
        }
        else if (namePath == "socket3")
        {
            state_SK3 = data.intData();
            Serial.print("STATE: ");
            Serial.println(state_SK3);
            // digitalWrite(IN3_RELAY_PIN, state_SK3);
            ws.textAll(getStateButton());
        }
        else if (namePath == "del_state")
        {
            delEnergyButtonState = data.intData();
            Serial.print("Del STATE: ");
            Serial.println(delEnergyButtonState);
        }
        else if (namePath == "powerAlr")
        {
            powerAlertNumber = data.intData();
            pzem.setPowerAlarm(powerAlertNumber);
            Serial.print("Alert Number: ");
            Serial.println(powerAlertNumber);
        }
        else if (namePath == "stt_powerAlr")
        {
            powerAlertStt = data.intData();
            Serial.print("Alert Status: ");
            Serial.println(powerAlertStt);
        }
        else if (namePath == "stt_timer1")
        {
            timerStt1 = data.intData();
            Serial.print("Timer 1 STT: ");
            Serial.println(timerStt1);
        }
        else if (namePath == "stt_timer2")
        {
            timerStt2 = data.intData();
            Serial.print("Timer 2 STT: ");
            Serial.println(timerStt2);
        }
        else if (namePath == "stt_timer3")
        {
            timerStt3 = data.intData();
            Serial.print("Timer 3 STT: ");
            Serial.println(timerStt3);
        }
    }
    if (data.dataTypeEnum() == fb_esp_rtdb_data_type_string)
    {
        String namePath = streamPath.substring(1);
        Serial.print("Name Path: ");
        Serial.println(namePath);
        if (namePath == "timer1") // => set poweroff timer socket1
        {
            String time_str = data.stringData();
            StringToTimeConvert(time_str, &hour1, &min1);
            Serial.print("Giờ 1: ");
            Serial.println(hour1);
            Serial.print("Phút 1: ");
            Serial.println(min1);
        }
        else if (namePath == "timer2") // => set poweroff timer socket2
        {
            String time_str = data.stringData();
            StringToTimeConvert(time_str, &hour2, &min2);
            Serial.print("Giờ 2: ");
            Serial.println(hour2);
            Serial.print("Phút 2: ");
            Serial.println(min2);
        }
        else if (namePath == "timer3") // => set poweroff timer socket3
        {
            String time_str = data.stringData();
            StringToTimeConvert(time_str, &hour3, &min3);
            Serial.print("Giờ 3: ");
            Serial.println(hour3);
            Serial.print("Phút 3: ");
            Serial.println(min3);
        }
    }

    FirebaseJson json = data.to<FirebaseJson>();
    size_t count_arr_json = json.iteratorBegin();
    for (size_t i = 0; i < count_arr_json; i++)
    {
        FirebaseJson::IteratorValue value = json.valueAt(i);
        if (i == 0)
        {
            delEnergyButtonState = value.value.toInt();
            Serial.printf("Del STT JSON = %d", delEnergyButtonState);
        }
        else if (i == 1)
        {
            powerAlertNumber = value.value.toInt();
        }
        else if (i == 2)
        {
            state_SK1 = value.value.toInt();
        }
        else if (i == 3)
        {
            state_SK2 = value.value.toInt();
        }
        else if (i == 4)
        {
            state_SK3 = value.value.toInt();
        }
        else if (i == 5)
        {
            powerAlertStt = value.value.toInt();
        }
        else if (i == 6)
        {
            timerStt1 = value.value.toInt();
        }
        else if (i == 7)
        {
            timerStt2 = value.value.toInt();
        }
        else if (i == 8)
        {
            timerStt3 = value.value.toInt();
        }
        else if (i == 9)
        {
            String raw_str = value.value;
            String time_str = raw_str.substring(1, 5);
            StringToTimeConvert(time_str, &hour1, &min1);
            Serial.print("Chuoi: ");
            Serial.println(time_str);
            Serial.print("Gio 1: ");
            Serial.println(hour1);
            Serial.print("Phút 1: ");
            Serial.println(min1);
        }
        else if (i == 10)
        {
            String raw_str = value.value;
            String time_str = raw_str.substring(1, 5);
            StringToTimeConvert(time_str, &hour2, &min2);
        }
        else if (i == 11)
        {
            String raw_str = value.value;
            String time_str = raw_str.substring(1, 5);
            StringToTimeConvert(time_str, &hour3, &min3);
        }
    }
    json.iteratorEnd(); // required for free the used memory in iteration (note data collection)
    Serial.printf("Received stream payload size: %d (Max. %d)\n\n", data.payloadLength(), data.maxPayloadLength());
}
void streamTimeoutCallback(bool timeout) // lang nghe su kien
{
    if (timeout)
        Serial.println("stream timeout, resuming.....\n");
    if (!stream.httpConnected())
        Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}
//======================END FIREBASE STREAM CALLBACK==================================
void initFirebase()
{
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
    config.api_key = API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    config.database_url = DATABASE_URL;
    Firebase.reconnectNetwork(false);
    config.token_status_callback = tokenStatusCallback;
    config.max_token_generation_retry = 5; // chu y test
    fbdo.setBSSLBufferSize(4096, 4096);
    Firebase.begin(&config, &auth);
    config.timeout.socketConnection = 10 * 1000;
    config.tcp_data_sending_retry = 1; // The function that starting the new TCP session
    Serial.println("Successfull Init Firebase");
}

void IRAM_ATTR onTimer()
{
    portENTER_CRITICAL_ISR(&timerMux);
    counter1000++;
    counter5000++;
    counter7500++;
    counter2000++;
    counter15000++;
    portEXIT_CRITICAL_ISR(&timerMux);
}

void setup()
{
    WiFi.mode(WIFI_STA);
    Serial.begin(115200);
    EEPROM.begin(512);
    for (int i = 0; i < 17; i = i + 8)
    {
        chipID |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }
    chipIDstr = String(chipID);
    listenerPath = chipIDstr + "/control/";
    Serial.printf("Chip ID: %u\n", chipID);
    tft.init();
    tft.setRotation(1);
    WELCOME_SCREEN();
    initRelayPin();
    Serial.setDebugOutput(true); // debug WIFI MANAGER
    // wm.setHttpPort(8080); // set another port for WM because https://github.com/rancilio-pid/clevercoffee/issues/323
    Serial.println("\n Starting");
    pinMode(TRIGGER_PIN, INPUT);
    pinMode(RESET_PZEM_PIN, INPUT_PULLUP);
    pinMode(SW_RELAY1_PIN, INPUT_PULLUP);
    pinMode(SW_RELAY2_PIN, INPUT_PULLUP);
    pinMode(SW_RELAY3_PIN, INPUT_PULLUP);
    pinMode(NORMAL_LED_PIN, OUTPUT);
    pinMode(IDLE_LED_PIN, OUTPUT);
    pinMode(ALARM_LED_PIN, OUTPUT);

    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 1000, true);

    wm.setConfigPortalBlocking(false);
    wm.setClass("invert");         // set DarkTheme
    wm.setConfigPortalTimeout(60); // auto close configportal after n seconds
    bool res;
    res = wm.autoConnect("StartConfig_WF"); // Wifi AP used to Config Wifi STA
    if (!res)
    {
        Serial.println("Failed to connect or hit timeout");
        // ESP.restart();
        START_CONFIG_WF_SCREEN();
        shouldRestart = true;
    }
    else
    {
        // if you get here you have connected to the WiFi
        Serial.println("connected...yeey :)");
        initFS();
        initWebSocket();
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                  { request->send(SPIFFS, "/index.html", "text/html"); });

        // Send a GET request to <IP>/get?message=<message
        // Send a POST request to <IP>/post with a form field message set to <message>

        server.onNotFound(notFound);
        server.serveStatic("/", SPIFFS, "/");
        server.begin();
        // initFirebase();
        //  Blynk.config(auth, "212.237.23.244", 8181);
        initFirebase();
        timeClient.begin();
        if (EEPROM.read(500) != 1)
        {
            SetupControlRTDB();
            EEPROM.write(500, 1);
            EEPROM.commit();
        }
        timerAlarmEnable(timer);
        if (!Firebase.RTDB.beginStream(&stream, listenerPath.c_str()))
            Serial.printf("stream begin error, %s\n\n", stream.errorReason().c_str());
        // Assign a calback function to run when it detects changes on the database
        Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);
        Serial.printf("Set String....%s\n", Firebase.RTDB.setString(&fbdo, chipIDstr + "/dashboard/wifiinfo", (String)WiFi.SSID()) ? "okSSID" : fbdo.errorReason().c_str());
        screenChange = 1; // screen wifi details
        tft.fillScreen(TFT_WHITE);
    }
}
void loop() //====================== MAIN PROGRAM ===================================
{
    wm.process();
    timeClient.update();
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    checkButton();
    SetOffAlarm();
    ControlRelay();
    resetValuePzem();
    checkWifi_config(); // restart if reset wifi and config again
    checkAlarmPower();
    ws.cleanupClients(); // dọn dẹp client không được sử dụng
    MainScreenChange();
    sendDataToRTDB();

    if (counter2000 >= 2000)
    {
        // Thực hiện các hoạt động sau mỗi interval
        // readPzem(); // chạy thật dụng hàm này
        voltageValue = random(100, 260);
        currentValue = random(0.0, 30.5);
        powerValue = random(100, 1000);
        energyValue = random(1, 100);
        pfValue = random(0.0, 10.0);
        freqValue = random(40.0, 50.0);

        mainHour = timeClient.getHours();
        mainMin = timeClient.getMinutes();
        mainSec = timeClient.getSeconds();
        notifyClients(getDataPower());

        Serial.println(millis());
        Serial.println(totalHeap);
        Serial.println(freeHeap);
        Serial.println(timeClient.getFormattedTime());
        // ReadDataFromRTDB();
        counter2000 = 0;
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
            if (screenChange == 4)
                screenChange = 1;
            // still holding button for 3000 ms, reset settings, code not ideaa for production
            delay(2000); // reset delay hold
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
    if (digitalRead(SW_RELAY1_PIN) == LOW)
    {
        delay(50);
        if (digitalRead(SW_RELAY1_PIN) == LOW)
        {
            Serial.println("SK1 BTN  Pressed");
            state_SK1 = !state_SK1;
            flag_webserver_handle = true;
            flag_webserver_socket_name = 1;
            ws.textAll(getStateButton());
        }
    }
    if (digitalRead(SW_RELAY2_PIN) == LOW)
    {
        delay(50);
        if (digitalRead(SW_RELAY2_PIN) == LOW)
        {
            Serial.println("SK2 BTN  Pressed");
            state_SK2 = !state_SK2;
            flag_webserver_handle = true;
            flag_webserver_socket_name = 2;
            ws.textAll(getStateButton());
        }
    }
    if (digitalRead(SW_RELAY3_PIN) == LOW)
    {
        delay(50);
        if (digitalRead(SW_RELAY3_PIN) == LOW)
        {
            Serial.println("SK3 BTN  Pressed");
            state_SK3 = !state_SK3;
            flag_webserver_handle = true;
            flag_webserver_socket_name = 3;
            ws.textAll(getStateButton());
        }
    }
}

void MainScreenChange()
{
    switch (screenChange)
    {
    case 1:
        TIME_UID_SCREEN();
        break;
    case 2:
        WIFI_INFOR_SCREEN();
        break;
    case 3:
        unsigned long currentMillis1 = millis();
        DASHBOARD_SCREEN();
        if (currentMillis1 - previousMillis1 >= 1500)
        {
            VALUE_DASHBOARD_SCREEN();
            previousMillis1 = currentMillis1;
        }
        break;
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
        voltageValue = 0;
        currentValue = 0;
        powerValue = 0;
        freqValue = 0;
        pfValue = 0;
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
    tft.setCursor(30, 3);
    tft.setTextSize(2);
    tft.print("DO AN");
    tft.setTextColor(TFT_RED);
    tft.setCursor(5, 23);
    tft.setTextSize(2);
    tft.print("TOT NGHIEP");
    tft.setTextColor(TFT_RED);
    tft.setCursor(28, 45);
    tft.setTextSize(1);
    tft.print("SMART SOCKET");
    tft.drawXBitmap(0, 63, socket_img, 64, 64, TFT_RED);
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
void TIME_UID_SCREEN()
{

    tft.setTextColor(TFT_DARKGREEN, TFT_YELLOW);
    tft.setCursor(3, 1);
    tft.setTextSize(2);
    tft.println("HOMESCREEN");
    tft.setTextColor(TFT_BLUE);
    tft.setTextSize(1);
    tft.setCursor(5, 20);
    tft.print("1. DEVICE ID: ");
    tft.setTextColor(TFT_RED);
    tft.setTextSize(2);
    tft.setCursor(20, 32);
    tft.print(chipIDstr);
    tft.setTextSize(1);
    tft.setCursor(5, 50);
    tft.setTextColor(TFT_BLUE);
    tft.print("2. CLOCK:");
    tft.setTextSize(1);
    tft.setCursor(10, 98);
    tft.setTextColor(TFT_BLUE);
    tft.print("S1      S2      S3");
    if (state_SK1)
    {
        tft.fillCircle(15, 115, 8, TFT_GREEN);
    }
    else
    {
        tft.fillCircle(15, 115, 8, TFT_RED);
    }
    if (state_SK2)
    {
        tft.fillCircle(62, 115, 8, TFT_GREEN);
    }
    else
    {
        tft.fillCircle(62, 115, 8, TFT_RED);
    }
    if (state_SK3)
    {
        tft.fillCircle(110, 115, 8, TFT_GREEN);
    }
    else
    {
        tft.fillCircle(110, 115, 8, TFT_RED);
    }
    tft.setTextSize(2);
    if (counter1000 >= 1000)
    {
        tft.setCursor(6, 62);
        tft.setTextColor(TFT_DARKGREEN);
        tft.fillRect(6, 62, 120, 25, TFT_WHITE);
        tft.print(timeClient.getFormattedDate());
        tft.setTextColor(TFT_PURPLE);
        tft.setCursor(17, 80);
        tft.fillRect(17, 80, 120, 25, TFT_WHITE);
        tft.print(timeClient.getFormattedTime());
        counter1000 = 0;
    }
}
void VALUE_DASHBOARD_SCREEN() // bảng các thông số điện
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
void resetValuePzem()
{
    if (digitalRead(RESET_PZEM_PIN) == LOW)
    {
        // poor mans debounce/press-hold, code not ideal for production
        delay(50);
        if (digitalRead(RESET_PZEM_PIN) == LOW)
        {
            Serial.println("ResetPIN Pressed");
            // still holding button for 3000 ms, reset settings, code not ideaa for production
            delay(2000); // reset delay hold
            if (digitalRead(RESET_PZEM_PIN) == LOW)
            {
                pzem.resetEnergy();
            }
        }
    }
    if (delEnergyButtonState == 10)
    {
        pzem.resetEnergy();
        Serial.printf("Send del_state....%s\n", Firebase.RTDB.setInt(&fbdo, chipIDstr + "/control/del_state", (int)0) ? "ok" : fbdo.errorReason().c_str()); // new
    }
}
void checkAlarmPower() // Ham kiem tra trang thai Cong suat
{
    // 5:green
    //  19:red
    if (powerAlertStt == 1 && powerValue >= powerAlertNumber)
    {
        digitalWrite(NORMAL_LED_PIN, LOW);
        digitalWrite(ALARM_LED_PIN, HIGH);
        digitalWrite(IDLE_LED_PIN, LOW);
    }
    else if (powerAlertStt == 1 && powerValue < powerAlertNumber)
    {
        digitalWrite(ALARM_LED_PIN, LOW);
        digitalWrite(NORMAL_LED_PIN, HIGH);
        digitalWrite(IDLE_LED_PIN, LOW);
    }
    else if (powerAlertStt == 0)
    {
        digitalWrite(ALARM_LED_PIN, LOW);
        digitalWrite(NORMAL_LED_PIN, LOW);
        digitalWrite(IDLE_LED_PIN, HIGH);
    }
}
void sendDataToRTDB()
{
    // send data every 5s to Firebase Realtime
    // if ((unsigned long)millis() - timer_Fb >= 7000 || timer_Fb == 0)
    // {
    //     timer_Fb = millis();
    //     Firebase.setFloat(fbdo, chipIDstr + "/dashboard/voltage", (float)voltageValue);
    //     Firebase.setFloat(fbdo, chipIDstr + "/dashboard/current", (float)currentValue);
    //     // Firebase.setFloat(fbdo, chipIDstr + "/dashboard/power", (float)powerValue);
    //     // Firebase.setFloat(fbdo, chipIDstr + "/dashboard/pf", (float)pfValue);
    //     // Firebase.setFloat(fbdo, chipIDstr + "/dashboard/energy", (float)energyValue);
    //     // Firebase.setFloat(fbdo, chipIDstr + "/dashboard/frequency", (float)freqValue);
    // }
    if (counter15000 >= 15000 && Firebase.ready())
    {

        send_flag_idle = false;
        Serial.printf("Send Volt....%s\n", Firebase.RTDB.setFloat(&fbdo, chipIDstr + "/dashboard/voltage", (float)voltageValue) ? "ok" : fbdo.errorReason().c_str());
        Serial.printf("Send Curr....%s\n", Firebase.RTDB.setFloat(&fbdo, chipIDstr + "/dashboard/current", (float)currentValue) ? "ok" : fbdo.errorReason().c_str());
        Serial.printf("Send Pwr....%s\n", Firebase.RTDB.setFloat(&fbdo, chipIDstr + "/dashboard/power", (float)powerValue) ? "ok" : fbdo.errorReason().c_str());
        Serial.printf("Send Energy....%s\n", Firebase.RTDB.setFloat(&fbdo, chipIDstr + "/dashboard/energy", (float)energyValue) ? "ok" : fbdo.errorReason().c_str());
        Serial.printf("Send Pf....%s\n", Firebase.RTDB.setFloat(&fbdo, chipIDstr + "/dashboard/pf", (float)pfValue) ? "ok" : fbdo.errorReason().c_str());
        Serial.printf("Send freq....%s\n", Firebase.RTDB.setFloat(&fbdo, chipIDstr + "/dashboard/frequency", (float)freqValue) ? "ok" : fbdo.errorReason().c_str());
        Serial.print("SEND DONE1");
        counter15000 = 0;
        send_flag_idle = true;
    }
}
void SetupControlRTDB()
{
    Firebase.RTDB.setInt(&fbdo, chipIDstr + "/control/del_state", 0);
    Firebase.RTDB.setInt(&fbdo, chipIDstr + "/control/powerAlr", 2000);
    Firebase.RTDB.setInt(&fbdo, chipIDstr + "/control/socket1", (int)state_SK1);
    Firebase.RTDB.setInt(&fbdo, chipIDstr + "/control/socket2", (int)state_SK2);
    Firebase.RTDB.setInt(&fbdo, chipIDstr + "/control/socket3", (int)state_SK3);
    Firebase.RTDB.setInt(&fbdo, chipIDstr + "/control/stt_powerAlr", 0);
    Firebase.RTDB.setInt(&fbdo, chipIDstr + "/control/stt_timer1", 0);
    Firebase.RTDB.setInt(&fbdo, chipIDstr + "/control/stt_timer2", 0);
    Firebase.RTDB.setInt(&fbdo, chipIDstr + "/control/stt_timer3", 0);
    Firebase.RTDB.setString(&fbdo, chipIDstr + "/control/timer1", "0000");
    Firebase.RTDB.setString(&fbdo, chipIDstr + "/control/timer2", "0000");
    Firebase.RTDB.setString(&fbdo, chipIDstr + "/control/timer3", "0000");
}
void ControlRelay()
{
    digitalWrite(IN1_RELAY_PIN, state_SK1);
    digitalWrite(IN2_RELAY_PIN, state_SK2);
    digitalWrite(IN3_RELAY_PIN, state_SK3);
    if (flag_webserver_handle == true && flag_webserver_socket_name == 0)
    {
        Serial.printf("Send SK1....%s\n", Firebase.RTDB.setInt(&fbdo, chipIDstr + "/control/socket1", (int)state_SK1) ? "ok" : fbdo.errorReason().c_str());
        Serial.printf("Send SK2....%s\n", Firebase.RTDB.setInt(&fbdo, chipIDstr + "/control/socket2", (int)state_SK2) ? "ok" : fbdo.errorReason().c_str());
        Serial.printf("Send SK3....%s\n", Firebase.RTDB.setInt(&fbdo, chipIDstr + "/control/socket3", (int)state_SK3) ? "ok" : fbdo.errorReason().c_str());
        delay(10);
        flag_webserver_handle = false;
        flag_webserver_socket_name = 0;
    }
    else if (flag_webserver_handle == true && flag_webserver_socket_name == 1)
    {
        
        Serial.printf("Send SK1....%s\n", Firebase.RTDB.setInt(&fbdo, chipIDstr + "/control/socket1", (int)state_SK1) ? "ok" : fbdo.errorReason().c_str());
        delay(10);
        flag_webserver_handle = false;
        flag_webserver_socket_name = 0;
    }
    else if (flag_webserver_handle == true && flag_webserver_socket_name == 2)
    {
        Serial.printf("Send SK2....%s\n", Firebase.RTDB.setInt(&fbdo, chipIDstr + "/control/socket2", (int)state_SK2) ? "ok" : fbdo.errorReason().c_str());
        delay(10);
        flag_webserver_handle = false;
        flag_webserver_socket_name = 0;
    }
    else if (flag_webserver_handle == true && flag_webserver_socket_name == 3)
    {
        Serial.printf("Send SK3....%s\n", Firebase.RTDB.setInt(&fbdo, chipIDstr + "/control/socket3", (int)state_SK3) ? "ok" : fbdo.errorReason().c_str());
        delay(10);
        flag_webserver_handle = false;
        flag_webserver_socket_name = 0;
    }
}
void SetOffAlarm()
{
    if (timerStt1 == 1 && CompareTime(mainHour, mainMin, mainSec, hour1, min1, sec1) == true && state_SK1 == true)
    {
        state_SK1 = false;
        flag_webserver_handle = true;
        flag_webserver_socket_name = 1;
        ws.textAll(getStateButton());
    }
    else if (timerStt2 == 1 && CompareTime(mainHour, mainMin, mainSec, hour2, min2, sec2) == true && state_SK2 == true)
    {
        state_SK2 = false;
        flag_webserver_handle = true;
        flag_webserver_socket_name = 2;
        ws.textAll(getStateButton());
    }
    else if (timerStt3 == 1 && CompareTime(mainHour, mainMin, mainSec, hour3, min3, sec3) == true && state_SK3 == true)
    {
        state_SK3 = false;
        flag_webserver_handle = true;
        flag_webserver_socket_name = 3;
        ws.textAll(getStateButton());
    }
}
void StringToTimeConvert(String time_str, int *hour, int *min)
{
    if (time_str.length() == 4)
    {
        // Lấy hai ký tự đầu tiên làm giờ
        String hour_str = time_str.substring(0, 2);
        *hour = hour_str.toInt();

        // Lấy hai ký tự còn lại làm phút
        String minute_str = time_str.substring(2, 4);
        *min = minute_str.toInt();
    }
    else
    {
        Serial.println("Chuỗi không hợp lệ.");
    }
}

bool CompareTime(int hourNTP, int minNTP, int secNTP, int hourSetup, int minSetup, int secSetup)
{
    if (hourNTP == hourSetup && minNTP == minSetup && (secNTP - secSetup) >= 0 && (secNTP - secSetup) < 5)
    {
        return true;
    }
    return false;
}
