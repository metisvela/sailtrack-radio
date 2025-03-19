#include "E32-868T20D.h"

uint8_t E32_868T20D::checksum(uint8_t * data, size_t len) {
    uint8_t cs = 0;
    for (int i = 0; i < len; i++)
        cs += data[i];
    return ~cs + 1;
}

size_t E32_868T20D::encode(uint16_t address, const char * message, uint8_t * output, size_t * outputSize) {
    size_t consumed = strnlen(message, 58);
    *outputSize = consumed + 5;
    output[0] = consumed;
    output[1] = random(0xFF);
    output[2] = address >> 8;
    output[3] = address;
    for (int i = 0; i < consumed; i++)
        output[i+4] = message[i] ^ keys[output[1]];
    output[*outputSize-1] = checksum(output, *outputSize-1);
    return consumed;
}

uint16_t E32_868T20D::getAddress(uint8_t * packet) {
    return (uint16_t)packet[2] << 8 | packet[3];
}

size_t E32_868T20D::decode(uint8_t * packet, char * output) {
    size_t len = packet[0];
    if (checksum(packet, len+5)) return 0;
    uint8_t index = packet[1];
    for (int i = 0; i < len; i++)
        output[i] = packet[i+4] ^ keys[index];
    output[len] = 0;
    return len;
}

void E32_868T20D::generateKeyDumpPacket(uint16_t address, uint8_t keyIndex, uint8_t * output, size_t * outputSize) {
    *outputSize = 6;
    output[0] = 0x01;
    output[1] = keyIndex;
    output[2] = address >> 8;
    output[3] = address;
    output[4] = 0x00;
    output[5] = checksum(output, *outputSize-1);
}
