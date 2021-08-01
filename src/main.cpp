#include <Arduino.h>
#include <ArduinoJson.h>
#include <axp20x.h>
#include "SparkFun_u-blox_GNSS_Arduino_Library.h"
#include "SailtrackModule.h"

#define I2C_SDA 21
#define I2C_SCL 22

#define GPS_RX_PIN 34
#define GPS_TX_PIN 12
#define GPS_BAND_RATE 9600

SFE_UBLOX_GNSS GPS;
AXP20X_Class PMU;

class ModuleCallbacks: public SailtrackModuleCallbacks {
	void onWifiConnectionBegin() {
		// TODO: Notify user
	}
	
	void onWifiConnectionResult(wl_status_t status) {
		// TODO: Notify user
	}

	DynamicJsonDocument getStatus() {
		DynamicJsonDocument payload(300);
		JsonObject battery = payload.createNestedObject("battery");
		JsonObject cpu = payload.createNestedObject("cpu");
		battery["voltage"] = PMU.getBattVoltage() / 1000;
		battery["charging"] = PMU.isChargeing();
		cpu["temperature"] = temperatureRead();
		return payload;
	}
};

void onGPSData(UBX_NAV_PVT_data_t ubxDataStruct) {
	DynamicJsonDocument payload(300);
	payload["latitude"] = ubxDataStruct.lat;
	payload["longitude"] = ubxDataStruct.lon;
	payload["speed"] = ubxDataStruct.gSpeed;
	payload["heading"] = ubxDataStruct.headMot;
	payload["vacc"] = ubxDataStruct.vAcc;
	payload["hacc"] = ubxDataStruct.hAcc;
	payload["sacc"] = ubxDataStruct.sAcc;
	payload["headacc"] = ubxDataStruct.headAcc;
	STModule.publish("sensor/gps0", "gps0", payload);
}

void beginPMU() {
	Wire.begin(I2C_SDA, I2C_SCL);
	PMU.begin(Wire, AXP192_SLAVE_ADDRESS);
	PMU.setPowerOutPut(AXP192_DCDC2, AXP202_OFF);
    PMU.setPowerOutPut(AXP192_LDO2, AXP202_OFF);
    PMU.setPowerOutPut(AXP192_LDO3, AXP202_OFF);
    PMU.setPowerOutPut(AXP192_EXTEN, AXP202_OFF);
}

void beginGPS() {
	PMU.setLDO3Voltage(3300);
	PMU.setPowerOutPut(AXP192_LDO3, AXP202_ON);
	Serial1.begin(GPS_BAND_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
	GPS.begin(Serial1);
	GPS.setUART1Output(COM_TYPE_UBX);
	GPS.setMeasurementRate(200);
	GPS.setAutoPVTcallback(&onGPSData);
}

void beginLora() {
	PMU.setLDO2Voltage(3300);
	PMU.setPowerOutPut(AXP192_LDO2, AXP202_ON);
	// TODO: Init LoRa
}

void setup() {
	beginPMU();
	STModule.begin("radio", "sailtrack-radio", IPAddress(192, 168, 42, 101));
	STModule.setCallbacks(new ModuleCallbacks());
	beginGPS();
	//beginLora();
}

void loop() {
	STModule.loop();
	GPS.checkUblox();
	GPS.checkCallbacks();
	delay(50);
}
