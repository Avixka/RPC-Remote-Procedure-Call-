/* Core/Inc/protocol.h */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>  // For uint8_t types

// --- DEFINITIONS ---
#define PACKET_START    0xAA  // The "Start Byte"
#define MAX_PAYLOAD     10    // Maximum data bytes allowed per packet

// Feedback Codes (What we send back to Linux)
#define RESP_ACK        0x06  // "Command Received & Executed"
#define RESP_NACK       0x15  // "Checksum Failed, Please Resend"

// --- THE DATA STRUCTURE ---
typedef struct {
    uint8_t start_byte;
    uint8_t cmd_id;
    uint8_t data_len;
    uint8_t payload[MAX_PAYLOAD];
    uint8_t checksum;
} Packet;

// --- FUNCTION PROTOTYPES ---
// These allow main.c to call functions inside protocol.c
void Process_Incoming_Byte(uint8_t byte);

#endif // PROTOCOL_H
