#include <Arduino.h>
#include <SailtrackModule.h>
#include <axp20x.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <RadioLib.h>
#include <E32-868T20D.h>

// -------------------------- Configuration -------------------------- //

#define MQTT_PUBLISH_FREQ_HZ		5
#define LORA_SEND_FREQ_HZ			1

#define GPS_BAUD_RATE 				9600
#define GPS_SERIAL_CONFIG 			SERIAL_8N1
#define GPS_RX_PIN 					34
#define GPS_TX_PIN 					12
#define GPS_NAVIGATION_FREQ_HZ		MQTT_PUBLISH_FREQ_HZ

#define LORA_CS_PIN 				18
#define LORA_DIO1_PIN 				33
#define LORA_RST_PIN 				23
#define LORA_BUSY_PIN 				32
#define LORA_MESSAGE_BUFFER_SIZE	512
#define LORA_PACKET_SIZE 			64

// EBYTE E32-868T20D parameters
#define E32_CHANNEL 				0x09
#define E32_ADDRESS 				0x1310
#define E32_BASE_FREQUENCY_MHZ 		862
#define E32_BANDWIDTH_KHZ 			500
#define E32_SPREADING_FACTOR 		11
#define E32_CODING_RATE_DENOM 		5

#define LOOP_TASK_DELAY_MS 			1000 / (2 * GPS_NAVIGATION_FREQ_HZ)
#define LORA_TASK_DELAY_MS			1000 / LORA_SEND_FREQ_HZ

struct LoraMetric {
	char value[32];
	char topic[32];
	char name[32];
} loraMetrics[] = {
	{ "0", "sensor/gps0", "epoch" },
	{ "0", "sensor/gps0", "latitude" },
	{ "0", "sensor/gps0", "longitude" },
	{ "0", "sensor/gps0", "speed" },
	{ "0", "sensor/gps0", "heading" },
	{ "0", "sensor/gps0", "fix" },
	{ "0", "sensor/imu0", "orientation.heading"},
	{ "0", "sensor/imu0", "orientation.pitch" },
	{ "0", "sensor/imu0", "orientation.roll" }
};

// ------------------------------------------------------------------- //

SailtrackModule stm;
SFE_UBLOX_GNSS gps;
AXP20X_Class pmu;
SX1262 lora = new Module(LORA_CS_PIN, LORA_DIO1_PIN, LORA_RST_PIN, LORA_BUSY_PIN);
E32_868T20D e32;

size_t loraSentBytes = 0;

class ModuleCallbacks: public SailtrackModuleCallbacks {
	void onStatusPublish(JsonObject status) {
		JsonObject battery = status.createNestedObject("battery");
		battery["voltage"] = pmu.getBattVoltage() / 1000;
		JsonObject lora = status.createNestedObject("lora");
		lora["bitrate"] = loraSentBytes * 8 * STM_STATUS_PUBLISH_FREQ_HZ / 1000;
		loraSentBytes = 0;
	}

	void onMqttMessage(const char * topic, JsonObjectConst message) {
		for (int i = 0; i < sizeof(loraMetrics)/sizeof(*loraMetrics); i++) {
			LoraMetric & metric = loraMetrics[i];
			if (!strcmp(topic, metric.topic)) {
				char metricName[strlen(metric.name)+1];
                strcpy(metricName, metric.name);
                char * token = strtok(metricName, ".");
                JsonVariantConst tmpVal = message;
				while (token != NULL) {
                    if (!tmpVal.containsKey(token)) return;
                    tmpVal = tmpVal[token];
                    token = strtok(NULL, ".");
                }
				serializeJson(tmpVal, metric.value);
			}
		}
	}
};

void loraTask(void * pvArguments) {
	TickType_t lastWakeTime = xTaskGetTickCount();
	while (true) {
		char message[LORA_MESSAGE_BUFFER_SIZE];
		strcpy(message, loraMetrics[0].value);
		for (int i = 1; i < sizeof(loraMetrics)/sizeof(*loraMetrics); i++) {
			strcat(message, " ");
			strcat(message, loraMetrics[i].value);
		}
		strcat(message, "\n");

		uint8_t packet[LORA_PACKET_SIZE];
		size_t len;
		size_t consumed = 0;
		size_t toConsume = strlen(message);
		while (consumed < toConsume) {
			consumed += e32.encode(E32_ADDRESS, message + consumed, packet, &len);
			lora.transmit(packet, len);
			loraSentBytes += len;
		}
		vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(LORA_TASK_DELAY_MS));
	}
}

void beginPMU() {
	Wire.begin();
	pmu.begin(Wire, AXP192_SLAVE_ADDRESS);
	pmu.setPowerOutPut(AXP192_DCDC1, AXP202_OFF);	// GPIO Pins Power Source
	pmu.setPowerOutPut(AXP192_DCDC2, AXP202_OFF);	// Unused
	pmu.setPowerOutPut(AXP192_LDO2, AXP202_OFF);	// LoRa Power Source
	pmu.setPowerOutPut(AXP192_LDO3, AXP202_OFF);	// GPS Power Source
    pmu.setPowerOutPut(AXP192_EXTEN, AXP202_OFF);	// External Connector Power Source
}

void beginGPS() {
	pmu.setLDO3Voltage(3300);
	pmu.setPowerOutPut(AXP192_LDO3, AXP202_ON);
	Serial1.begin(GPS_BAUD_RATE, GPS_SERIAL_CONFIG, GPS_RX_PIN, GPS_TX_PIN);
	gps.begin(Serial1);
	gps.setUART1Output(COM_TYPE_UBX);
	gps.setDynamicModel(DYN_MODEL_SEA);
	gps.setNavigationFrequency(GPS_NAVIGATION_FREQ_HZ);
	gps.setAutoPVT(true);
}

void beginLora() {
	pmu.setLDO2Voltage(3300);
	pmu.setPowerOutPut(AXP192_LDO2, AXP202_ON);
	lora.begin(E32_BASE_FREQUENCY_MHZ + E32_CHANNEL, E32_BANDWIDTH_KHZ, E32_SPREADING_FACTOR, E32_CODING_RATE_DENOM);
	for (auto metric : loraMetrics) stm.subscribe(metric.topic);
	xTaskCreate(loraTask, "loraTask", STM_TASK_MEDIUM_STACK_SIZE, NULL, STM_TASK_MEDIUM_PRIORITY, NULL);
}

void setup() {
	beginPMU();
	stm.begin("radio", IPAddress(192, 168, 42, 101), new ModuleCallbacks());
	beginGPS();
	beginLora();
}

void loop() {
	TickType_t lastWakeTime = xTaskGetTickCount();
	if (gps.getPVT() && gps.getTimeValid()) {
		StaticJsonDocument<STM_JSON_DOCUMENT_MEDIUM_SIZE> doc;
		doc["epoch"] = gps.getUnixEpoch();
		doc["latitude"] = gps.getLatitude();
		doc["longitude"] = gps.getLongitude();
		doc["speed"] = gps.getGroundSpeed();
		doc["heading"] = gps.getHeading();
		doc["fix"] = gps.getFixType();
		stm.publish("sensor/gps0", doc.as<JsonObjectConst>());
	}
	vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(LOOP_TASK_DELAY_MS));
}
