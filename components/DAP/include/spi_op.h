#ifndef __SPI_OP_H__
#define __SPI_OP_H__

#include <stdint.h>


void DAP_SPI_WriteBits(const uint8_t count, const uint8_t *buf);
void DAP_SPI_ReadBits(const uint8_t count, uint8_t *buf);

void DAP_SPI_Send_Header(const uint8_t packetHeaderData, uint8_t *ack, uint8_t TrnAfterACK);
void DAP_SPI_Read_Data(uint32_t* resData, uint8_t* resParity);
void DAP_SPI_Write_Data(uint32_t data, uint8_t parity);

void DAP_SPI_Generate_Cycle(uint8_t num);
void DAP_SPI_Fast_Cycle();

void DAP_SPI_Protocol_Error_Read();
void DAP_SPI_Protocol_Error_Write();


#endif
