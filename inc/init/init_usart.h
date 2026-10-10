#ifndef INIT_USART_H
#define INIT_USART_H

#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_bus.h"

void UART5_Init(uint32_t baudrate);
void UART5_SendChar(char c);
void UART5_SendString(const char *str);

#endif /* INIT_USART_H */
