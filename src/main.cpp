#define USE_ESP_IDF_LOG

#include <Arduino.h>
#include <SailtrackModule.h>
#include <axp20x.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <RadioLib.h>
#include <E32-868T20D.h>

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
#define E32_CHANNEL 0x09
#define E32_ADDRESS 0x1310
#define E32_BASE_FREQUENCY_MHZ 862
#define E32_BANDWIDTH_KHZ 500
#define E32_SPREADING_FACTOR 11
#define E32_CODING_RATE_DENOM 5

#define MQTT_DATA_PUBLISH_RATE_HZ 1
#define MQTT_LOG_PUBLISH_RATE_HZ 0.1
#define LORA_SEND_RATE_HZ 1

static const char * LOG_TAG = "SAILTRACK_RADIO";

struct LoraMetric {
	char value[20];
	char topic[20];
	char name[20];
} loraMetrics[] = {
	{ "0", "sensor/gps0", "latitude" },
	{ "0", "sensor/gps0", "longitude" },
	{ "0", "sensor/gps0", "speed" },
	{ "0", "sensor/gps0", "heading" },
	{ "0", "sensor/gps0", "fix" },
	{ "0", "sensor/imu0", "orientation.heading"},
	{ "0", "sensor/imu0", "orientation.pitch" },
	{ "0", "sensor/imu0", "orientation.roll" }
};

SFE_UBLOX_GNSS gps;
AXP20X_Class pmu;
SailtrackModule stm;
SX1262 lora = new Module(LORA_CS_PIN, LORA_DIO1_PIN, LORA_RST_PIN, LORA_BUSY_PIN);
E32_868T20D e32;

size_t loraSentBytes = 0;

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

	void onMqttMessage(const char * topic, const char * message) {
		for (int i = 0; i < sizeof(loraMetrics)/sizeof(*loraMetrics); i++) {
			LoraMetric & metric = loraMetrics[i];
			if (!strcmp(topic, metric.topic)) {
				DynamicJsonDocument payload(500);
				deserializeJson(payload, message);
				char metricName[strlen(metric.name)+1];
                strcpy(metricName, metric.name);
                char * token = strtok(metricName, ".");
                JsonVariant tmpVal = payload.as<JsonVariant>();
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

void onGPSData(UBX_NAV_PVT_data_t ubxDataStruct) {
	DynamicJsonDocument payload(300);
	payload["latitude"] = ubxDataStruct.lat;
	payload["longitude"] = ubxDataStruct.lon;
	payload["speed"] = ubxDataStruct.gSpeed;
	payload["heading"] = ubxDataStruct.headMot;
	payload["fix"] = ubxDataStruct.fixType;
	stm.publish("sensor/gps0", &payload);
}

void loraTask(void * pvArguments) {
	TickType_t lastWakeTime = xTaskGetTickCount();
	while (true) {
		char message[500];
		strcpy(message, loraMetrics[0].value);
		for (int i = 1; i < sizeof(loraMetrics)/sizeof(*loraMetrics); i++) {
			strcat(message, " ");
			strcat(message, loraMetrics[i].value);
		}
		strcat(message, "\n");

		uint8_t packet[64];
		size_t len;
		size_t consumed = 0;
		size_t toConsume = strlen(message);
		while (consumed < toConsume) {
			consumed += e32.encode(E32_ADDRESS, message + consumed, packet, &len);
			lora.transmit(packet, len);
			loraSentBytes += len;
		}
		vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(1000 / LORA_SEND_RATE_HZ));
	}
}

void logTask(void * pvArguments) {
	TickType_t lastWakeTime = xTaskGetTickCount();
    while (true) {
		float bw = loraSentBytes * 8 * MQTT_LOG_PUBLISH_RATE_HZ / 1000;
        ESP_LOGI(LOG_TAG, "Used LoRa Bandwidth: %.2f kbit/s", bw);
		loraSentBytes = 0;
		vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(1000 / MQTT_LOG_PUBLISH_RATE_HZ));
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

void beginModule() {
	stm.setNotificationLed(NOTIFICATION_LED_PIN, true);
	stm.begin("radio", IPAddress(192, 168, 42, 101), new ModuleCallbacks());
}

void beginGPS() {
	pmu.setLDO3Voltage(3300);
	pmu.setPowerOutPut(AXP192_LDO3, AXP202_ON);
	Serial1.begin(GPS_BAUD_RATE, GPS_SERIAL_CONFIG, GPS_RX_PIN, GPS_TX_PIN);
	gps.begin(Serial1);
	gps.setUART1Output(COM_TYPE_UBX);
	gps.setMeasurementRate(1000 / MQTT_DATA_PUBLISH_RATE_HZ);
	gps.setAutoPVTcallback(&onGPSData);
}

void beginLora() {
	pmu.setLDO2Voltage(3300);
	pmu.setPowerOutPut(AXP192_LDO2, AXP202_ON);
	lora.begin(E32_BASE_FREQUENCY_MHZ + E32_CHANNEL, E32_BANDWIDTH_KHZ, E32_SPREADING_FACTOR, E32_CODING_RATE_DENOM);
	xTaskCreate(loraTask, "loraTask", TASK_MEDIUM_STACK_SIZE, NULL, TASK_MEDIUM_PRIORITY, NULL);
}

void beginLogging() {
	esp_log_level_set(LOG_TAG, ESP_LOG_INFO);
	xTaskCreate(logTask, "logTask", TASK_MEDIUM_STACK_SIZE, NULL, TASK_LOW_PRIORITY, NULL);
}

void setup() {
	beginPMU();
	beginModule();
	beginGPS();
	beginLora();
	beginLogging();
	for (auto metric : loraMetrics)
		stm.subscribe(metric.topic);
}

TickType_t lastWakeTime = xTaskGetTickCount();
void loop() {
	gps.checkUblox();
	gps.checkCallbacks();
	vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(1000 / GPS_UPDATE_RATE_HZ));
}
