#include <Arduino.h>
#include <SailtrackModule.h>
#include <axp20x.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <RadioLib.h>

#define NOTIFICATION_LED_PIN 4

#define GPS_BAUD_RATE 9600
#define GPS_SERIAL_CONFIG SERIAL_8N1
#define GPS_RX_PIN 34
#define GPS_TX_PIN 12
#define GPS_UPDATE_RATE_HZ 20

#define LORA_CS_PIN 18
#define LORA_DIO1_PIN 33
#define LORA_RST_PIN 23
#define LORA_BUSY_PIN 32

// EBYTE E32-868T20D parameters
#define LORA_FREQUENCY_MHZ 868
#define LORA_BANDWIDTH_KHZ 500
#define LORA_SPREADING_FACTOR 11
#define LORA_CODING_RATE_DENOM 5

#define MQTT_PUBLISH_RATE_HZ 5

SFE_UBLOX_GNSS gps;
AXP20X_Class pmu;
SailtrackModule stm;
SX1262 lora = new Module(LORA_CS_PIN, LORA_DIO1_PIN, LORA_RST_PIN, LORA_BUSY_PIN);

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
	payload["fix"] = ubxDataStruct.fixType;
	stm.publish("sensor/gps0", &payload);
}

void beginPMU() {
	Wire.begin();
	pmu.begin(Wire, AXP192_SLAVE_ADDRESS);
	pmu.setPowerOutPut(AXP192_DCDC1, AXP202_OFF);
	pmu.setPowerOutPut(AXP192_DCDC2, AXP202_OFF);
	pmu.setPowerOutPut(AXP192_LDO2, AXP202_OFF);
	pmu.setPowerOutPut(AXP192_LDO3, AXP202_OFF);
    pmu.setPowerOutPut(AXP192_EXTEN, AXP202_OFF);
}

void beginGPS() {
	pmu.setLDO3Voltage(3300);
	pmu.setPowerOutPut(AXP192_LDO3, AXP202_ON);
	Serial1.begin(GPS_BAUD_RATE, GPS_SERIAL_CONFIG, GPS_RX_PIN, GPS_TX_PIN);
	gps.begin(Serial1);
	gps.setUART1Output(COM_TYPE_UBX);
	gps.setMeasurementRate(1000 / MQTT_PUBLISH_RATE_HZ);
	gps.setAutoPVTcallback(&onGPSData);
}

void beginLora() {
	pmu.setLDO2Voltage(3300);
	pmu.setPowerOutPut(AXP192_LDO2, AXP202_ON);
	lora.begin(LORA_FREQUENCY_MHZ, LORA_BANDWIDTH_KHZ, LORA_SPREADING_FACTOR, LORA_CODING_RATE_DENOM);
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
