#include <Arduino.h>

#include "SailtrackModule.h"

SailtrackModule radio_module;

#include "SparkFun_u-blox_GNSS_Arduino_Library.h" //http://librarymanager/All#SparkFun_u-blox_GNSS

#define GPS_RX_PIN 34
#define GPS_TX_PIN 12
#define GPS_BAND_RATE 9600

#define BUTTON_PIN 38
#define BUTTON_PIN_MASK GPIO_SEL_38

SFE_UBLOX_GNSS myGNSS;


void printPVTdata(UBX_NAV_PVT_data_t ubxDataStruct)
{
	DynamicJsonDocument payload(JSON_OBJECT_SIZE(10));

	payload["latitude"] = ubxDataStruct.lat;
	payload["longitude"] = ubxDataStruct.lon;
	payload["speed"] = ubxDataStruct.gSpeed;
	payload["heading"] = ubxDataStruct.headMot;
	payload["vacc"] = ubxDataStruct.vAcc;
	payload["hacc"] = ubxDataStruct.hAcc;
	payload["sacc"] = ubxDataStruct.sAcc;
	payload["headacc"] = ubxDataStruct.headAcc;

	radio_module.publish("sensor/gps0", "gps0", payload);

	Serial.println("sent message...");
}

void setup()
{
	radio_module.init("radio", IPAddress(192, 168, 42, 101));

	Serial.begin(115200);

	Serial1.begin(GPS_BAND_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
	while (myGNSS.begin(Serial1) == false)
	{
		Serial.println(F("."));
		delay(100);
	}

	myGNSS.setUART1Output(COM_TYPE_UBX); //Set the UART port to output UBX only

	myGNSS.setMeasurementRate(100);
	// myGNSS.setNavigationRate(5);

	myGNSS.setAutoPVTcallback(&printPVTdata); // Enable automatic NAV PVT messages with callback to printPVTdata
}

void loop()
{
	radio_module.loop();
	myGNSS.checkUblox();	 // Check for the arrival of new data and process it.
	myGNSS.checkCallbacks(); // Check if any callbacks are waiting to be processed.

	// Serial.print(".");
	delay(50);
}