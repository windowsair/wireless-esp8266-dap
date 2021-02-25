#ifndef __SWO_H__
#define __SWO_H__

typedef void * EventGroupHandle_t;

// event group bits
#define SWO_GOT_DATA 0x00000001
#define UART_GOT_DATA 0x00000002
#define SWO_ERROR_TIME_OUT 0x00000004

extern EventGroupHandle_t kSwoThreadEventGroup;
extern volatile uint8_t kSwoTransferBusy;


void SetTraceError(uint8_t flag); // Use in the uart handler

#endif