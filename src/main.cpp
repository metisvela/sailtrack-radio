#include <Arduino.h>
#include <SailtrackModule.h>
#include <axp20x.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <LoRa.h>

#define NOTIFICATION_LED_PIN 4

#define GPS_RX_PIN 34
#define GPS_TX_PIN 12
#define GPS_BAUD_RATE 9600

#define LORA_FREQUENCY 868E6

#define GPS_UPDATE_RATE_HZ 20
#define MQTT_PUBLISH_RATE_HZ 5

SFE_UBLOX_GNSS gps;
AXP20X_Class pmu;
SailtrackModule stm;

class ModuleCallbacks: public SailtrackModuleCallbacks {
	DynamicJsonDocument * getStatus() {
		DynamicJsonDocument * payload =  new DynamicJsonDocument(300);
		JsonObject battery = payload->createNestedObject("battery");
		JsonObject cpu = payload->createNestedObject("cpu");
		battery["voltage"] = pmu.getBattVoltage() / 1000;
		battery["charging"] = pmu.isChargeing();
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
	payload["fix"] = ubxDataStruct.fixType;
	stm.publish("sensor/gps0", &payload);
}

void beginPMU() {
	Wire.begin(SDA, SCL);
	pmu.begin(Wire, AXP192_SLAVE_ADDRESS);
	pmu.setPowerOutPut(AXP192_DCDC2, AXP202_OFF);
    pmu.setPowerOutPut(AXP192_EXTEN, AXP202_OFF);
}

void beginGPS() {
	pmu.setLDO3Voltage(3300);
	pmu.setPowerOutPut(AXP192_LDO3, AXP202_ON);
	Serial1.begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
	gps.begin(Serial1);
	gps.setUART1Output(COM_TYPE_UBX);
	gps.setMeasurementRate(1000 / MQTT_PUBLISH_RATE_HZ);
	gps.setAutoPVTcallback(&onGPSData);
}

void beginLora() {
	pmu.setLDO2Voltage(3300);
	pmu.setPowerOutPut(AXP192_LDO2, AXP202_ON);
	LoRa.setPins(LORA_CS, LORA_RST, LORA_IO0);
	LoRa.begin(LORA_FREQUENCY);
}

void setup() {
	beginPMU();
	stm.setNotificationLed(NOTIFICATION_LED_PIN, true);
	stm.begin("radio", IPAddress(192, 168, 42, 101), new ModuleCallbacks());
	beginGPS();
	beginLora();
}

void loop() {
	gps.checkUblox();
	gps.checkCallbacks();
	delay(1000 / GPS_UPDATE_RATE_HZ);
}
