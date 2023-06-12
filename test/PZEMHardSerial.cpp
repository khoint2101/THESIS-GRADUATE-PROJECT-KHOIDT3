/*
Copyright (c) 2021 Jakub Mandula

Example of using one PZEM module with Hardware Serial interface.
================================================================

If desired, a HardwareSerial handle can be passed to the constructor
which will then be used for the communication with the module.

Note that ESP32 HardwareSerial must also be provided with the RX and TX
pins.

*/
#include <Arduino.h>
#include <PZEM004Tv30.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>
#include <WiFiManager.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

/*************************
 *  ESP32 initialization
 * ---------------------
 *
 * The ESP32 HW Serial interface can be routed to any GPIO pin
 * Here we initialize the PZEM on Serial2 with RX/TX pins 16 and 17
 */
#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
#endif

#define PZEM_SERIAL Serial2
#define CONSOLE_SERIAL Serial
PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);

void setup()
{
    // Debugging Serial port
    CONSOLE_SERIAL.begin(115200);

    // Uncomment in order to reset the internal energy counter
    // pzem.resetEnergy()
}

void loop()
{
    // Print the custom address of the PZEM
    CONSOLE_SERIAL.print("Custom Address:");
    CONSOLE_SERIAL.println(pzem.readAddress(), HEX);

    // Read the data from the sensor
    float voltage = pzem.voltage();
    float current = pzem.current();
    float power = pzem.power();
    float energy = pzem.energy();
    float frequency = pzem.frequency();
    float pf = pzem.pf();

    // Check if the data is valid
    if (isnan(voltage))
    {
        CONSOLE_SERIAL.println("Error reading voltage");
    }
    else if (isnan(current))
    {
        CONSOLE_SERIAL.println("Error reading current");
    }
    else if (isnan(power))
    {
        CONSOLE_SERIAL.println("Error reading power");
    }
    else if (isnan(energy))
    {
        CONSOLE_SERIAL.println("Error reading energy");
    }
    else if (isnan(frequency))
    {
        CONSOLE_SERIAL.println("Error reading frequency");
    }
    else if (isnan(pf))
    {
        CONSOLE_SERIAL.println("Error reading power factor");
    }
    else
    {

        // Print the values to the Serial console
        CONSOLE_SERIAL.print("Voltage: ");
        CONSOLE_SERIAL.print(voltage);
        CONSOLE_SERIAL.println("V");
        CONSOLE_SERIAL.print("Current: ");
        CONSOLE_SERIAL.print(current);
        CONSOLE_SERIAL.println("A");
        CONSOLE_SERIAL.print("Power: ");
        CONSOLE_SERIAL.print(power);
        CONSOLE_SERIAL.println("W");
        CONSOLE_SERIAL.print("Energy: ");
        CONSOLE_SERIAL.print(energy, 3);
        CONSOLE_SERIAL.println("kWh");
        CONSOLE_SERIAL.print("Frequency: ");
        CONSOLE_SERIAL.print(frequency, 1);
        CONSOLE_SERIAL.println("Hz");
        CONSOLE_SERIAL.print("PF: ");
        CONSOLE_SERIAL.println(pf);
    }

    CONSOLE_SERIAL.println();
    delay(2000);
}
