#include <stdint.h> 
#include <stdbool.h> 
#include <stddef.h>
uint8_t process_frame(uint8_t *raw, uint8_t len);

/* 全局缓冲区（由平台提供） */ 
#define MAX_DATA_LEN 16 
#define BUFFER_SIZE (2 + MAX_DATA_LEN + 1) // 最大20字节
uint8_t rx_buffer[BUFFER_SIZE]; 
uint8_t received_data[MAX_DATA_LEN];
 volatile uint8_t data_ready; // 由volatile修饰 

/* 状态机定义 */ 
typedef enum { 
STATE_IDLE,
 STATE_RECEIVING
 } rx_state_t; 

static rx_state_t rx_state = STATE_IDLE; 
static uint8_t rx_index = 0;
static uint8_t expected_len = 0;

 /* 外部函数：从UART硬件读取一个字节（由平台提供） */ 
extern uint8_t UART_ReceiveByte(void); 

/* 中断服务程序 */
 void UART_IRQHandler(void)
 { uint8_t byte = UART_ReceiveByte();

 switch (rx_state) {
 case STATE_IDLE: 
if (byte == 0xAA) {
 rx_buffer[0] = byte;
 rx_index = 1; 
rx_state = STATE_RECEIVING; 
}
 break; 

case STATE_RECEIVING: 
if (rx_index >= BUFFER_SIZE) {
 rx_state = STATE_IDLE; // 溢出，复位 
break; 
}
 rx_buffer[rx_index] = byte;
 rx_index++; 

if (rx_index == 2) {
 expected_len = byte; 
if (expected_len < 1 || expected_len > MAX_DATA_LEN) { 
rx_state = STATE_IDLE; // 非法长度，丢弃 
}
 } else if (rx_index == (expected_len + 3)) {
 if (process_frame(rx_buffer, expected_len + 3)) {
 data_ready = 1; 
}
 rx_state = STATE_IDLE; 
} 
break;

 default:
 rx_state = STATE_IDLE; 
break;
 } 
}

 /* 帧处理函数 */
 uint8_t process_frame(uint8_t *raw, uint8_t len) {
 if (raw == NULL || len < 3) return 0; 

uint8_t header = raw[0];
 uint8_t length = raw[1];
 uint8_t checksum = raw[len - 1]; 
uint8_t cal_checksum = 0;

 if (header != 0xAA) return 0; 
if (length < 1 || length > MAX_DATA_LEN) return 0;
 if (len != (length + 3)) return 0;
 
for (uint8_t i = 0; i < len - 1; i++) {
 cal_checksum += raw[i];
 } if (cal_checksum != checksum) return 0;

 for (uint8_t i = 0; i < length; i++) {
  received_data[i] = raw[2 + i];
 }

 return 1;
 }

/* OJ bare-metal: entry required by startup Reset_Handler -> main() */
int main(void)
{
    while (1) {
    }
    return 0;
}
