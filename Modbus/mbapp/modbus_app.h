#ifndef LL_ECHO_USART_MODBUS_APP_H
#define LL_ECHO_USART_MODBUS_APP_H

#include "mb.h"
#include "main.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "string.h"


eMBErrorCode MbInit(eMBMode eMode, UCHAR ucSlaveAddress);

void MbSetCoils(uint16_t bitmap, uint8_t address, uint8_t bits_count);
uint16_t MbGetCoils(uint8_t offset, uint8_t count);

void MbSetDiscreteInputs(uint16_t bitmap, uint8_t address, uint8_t bits_count);
uint16_t MbGetDiscreteInputs(uint8_t offset, uint8_t count);

void MbSetInputRegisters(uint16_t value, uint8_t index);
uint16_t MbGetInputRegisters(uint8_t index);

void MbSetHoldingRegisters(uint16_t value, uint8_t index);
uint16_t MbGetHoldingRegisters(uint8_t index);


uint8_t MbGetCoilsCount();
uint8_t MbGetDiscretInCount();
uint8_t MbGetHoldingCount();
uint8_t MbGetAnalogInCount();

#ifdef __cplusplus
}
#endif

#endif //LL_ECHO_USART_MODBUS_APP_H
