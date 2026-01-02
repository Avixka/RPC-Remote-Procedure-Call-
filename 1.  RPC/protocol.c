/* Core/Src/protocol.c */

#include "protocol.h"
#include "main.h"      // Access to HAL drivers (GPIO, UART)
#include <string.h>    // For safer memory operations

// We need the UART handle to send ACK/NACK back
// (This is defined in main.c, so we use 'extern')
extern UART_HandleTypeDef huart1; 

// --- PRIVATE HELPER FUNCTIONS (Internal use only) ---

// 1. Math: Calculate simple Sum Modulo 256
static uint8_t Calculate_Checksum(Packet *pkt) {
    uint8_t sum = 0;
    sum += pkt->start_byte;
    sum += pkt->cmd_id;
    sum += pkt->data_len;
    
    for (int i = 0; i < pkt->data_len; i++) {
        sum += pkt->payload[i];
    }
    return sum;
}

// 2. Feedback: Send 1 byte back to Linux
static void Send_Response(uint8_t response_code) {
    // Blocking transmit is fine here for 1 byte (takes microseconds)
    HAL_UART_Transmit(&huart1, &response_code, 1, 10);
}

// 3. Action: Actually do the work
static void Execute_Command(Packet pkt) {
    switch(pkt.cmd_id) {
        case 0x01: // Example: Control LED
            // Payload[0] determines ON (1) or OFF (0)
            if (pkt.payload[0] == 1) {
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
            } else {
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
            }
            break;
            
        case 0x02: // Example: Another command (Motor, Sensor, etc.)
            // Add your custom logic here
            break;
    }
}

// --- PUBLIC FUNCTION (Called by main.c) ---

void Process_Incoming_Byte(uint8_t byte) {
    static uint8_t state = 0;
    static Packet current_packet;
    static uint8_t data_index = 0;

    switch(state) {
        case 0: // Waiting for Start Byte
            if (byte == PACKET_START) {
                current_packet.start_byte = byte;
                state = 1;
            }
            break;

        case 1: // Get Command ID
            current_packet.cmd_id = byte;
            state = 2;
            break;

        case 2: // Get Length
            // SAFETY CHECK: Prevent buffer overflow
            if (byte > MAX_PAYLOAD) {
                // Error: Length too big for our buffer!
                // Reset immediately to avoid crashing RAM.
                state = 0; 
                Send_Response(RESP_NACK); // Tell Linux "You are crazy"
            } else {
                current_packet.data_len = byte;
                data_index = 0;
                
                // Optimization: If length is 0, skip payload state
                if (current_packet.data_len == 0) state = 4;
                else state = 3;
            }
            break;

        case 3: // Get Payload
            current_packet.payload[data_index++] = byte;
            if (data_index >= current_packet.data_len) {
                state = 4;
            }
            break;

        case 4: // Checksum Verification
            current_packet.checksum = byte;
            
            // MATH CHECK: Do the numbers add up?
            if (Calculate_Checksum(&current_packet) == current_packet.checksum) {
                // SUCCESS: Valid Packet
                Execute_Command(current_packet);
                Send_Response(RESP_ACK); // Reply: "OK"
            } else {
                // FAILURE: Corrupted Packet
                // Do NOT execute.
                Send_Response(RESP_NACK); // Reply: "Fail, Retry"
            }
            
            state = 0; // Reset for the next packet
            break;
            
        default:
            state = 0;
            break;
    }
}
