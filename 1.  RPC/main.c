/* Core/Src/main.c */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "protocol.h" // <--- Include our custom protocol

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;

// 1. Define Ring Buffer
#define RX_BUFFER_SIZE 256
uint8_t rx_circular_buffer[RX_BUFFER_SIZE];

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);

int main(void)
{
  /* MCU Configuration */
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();

  /* USER CODE BEGIN 2 */
  // 2. START DMA: Tell hardware to fill the bucket in the background
  HAL_UART_Receive_DMA(&huart1, rx_circular_buffer, RX_BUFFER_SIZE);
  /* USER CODE END 2 */

  /* Infinite loop */
  while (1)
  {
    /* USER CODE BEGIN 3 */
    
    // 3. TRACKING POINTERS
    // old_pos: Where we stopped reading last time
    // new_pos: Where the DMA is writing right now
    static uint16_t old_pos = 0;
    
    // Calculate new_pos: Size - Remaining_Count
    uint16_t new_pos = RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);
    
    // 4. CHECK FOR DATA
    if (new_pos != old_pos) {
        
        // Scenario A: Normal (new is ahead of old)
        if (new_pos > old_pos) {
            for (int i = old_pos; i < new_pos; i++) {
                Process_Incoming_Byte(rx_circular_buffer[i]);
            }
        } 
        // Scenario B: Wrap-Around (DMA reached end and went to 0)
        else {
            // Part 1: Read from old_pos to the END of buffer
            for (int i = old_pos; i < RX_BUFFER_SIZE; i++) {
                Process_Incoming_Byte(rx_circular_buffer[i]);
            }
            // Part 2: Read from 0 to new_pos
            for (int i = 0; i < new_pos; i++) {
                Process_Incoming_Byte(rx_circular_buffer[i]);
            }
        }
        
        // Update our position for next loop
        old_pos = new_pos;
    }
    
    // NOTE: Do not add HAL_Delay() here! Keep this loop fast.
    /* USER CODE END 3 */
  }
}

// ... (Rest of the auto-generated initialization code below) ...
