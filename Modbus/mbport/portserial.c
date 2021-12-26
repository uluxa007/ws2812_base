/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */

#include "port.h"
#include "portserial.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "main.h"

/* ----------------------- defines ---------------------------------*/
#define BODBUS_USART USART2
//#define RS485_RTS_LOW LL_GPIO_ResetOutputPin(RS485_DE_GPIO_Port,RS485_DE_Pin);
//#define RS485_RTS_HIGH LL_GPIO_SetOutputPin(RS485_DE_GPIO_Port,RS485_DE_Pin);

/* ----------------------- Start implementation -----------------------------*/
void
vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
    /* If xRXEnable enable serial receive interrupts. If xTxENable enable
     * transmitter empty interrupts.
     */
    if(xRxEnable)
    {
        //RS485_RTS_LOW;
        LL_USART_EnableIT_RXNE(BODBUS_USART);
    }
    else
    {
        LL_USART_DisableIT_RXNE(BODBUS_USART);
    }

    if(xTxEnable)
    {
        //RS485_RTS_HIGH;
        pxMBFrameCBTransmitterEmpty();
    }
    else
    {
        LL_USART_DisableIT_TC(BODBUS_USART);
    }
}

BOOL
xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
    return TRUE;
}

BOOL
xMBPortSerialPutByte( CHAR ucByte )
{
    /* Put a byte in the UARTs transmit buffer. This function is called
     * by the protocol stack if pxMBFrameCBTransmitterEmpty( ) has been
     * called. */
    LL_USART_EnableIT_TC(BODBUS_USART);
    LL_USART_TransmitData8(BODBUS_USART,ucByte);
    return TRUE;
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
    /* Return the byte in the UARTs receive buffer. This function is called
     * by the protocol stack after pxMBFrameCBByteReceived( ) has been called.
     */
    *pucByte = LL_USART_ReceiveData8(BODBUS_USART);
    return TRUE;
}

/* Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */
void prvvUARTTxReadyISR( void )
{
    pxMBFrameCBTransmitterEmpty(  );
}

/* Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
void prvvUARTRxISR( void )
{
    pxMBFrameCBByteReceived(  );
}

void modBusUsart_IRQHandler()
{
    if(LL_USART_IsActiveFlag_TC(BODBUS_USART) && LL_USART_IsEnabledIT_TC(BODBUS_USART))
    {
        LL_USART_DisableIT_TC(BODBUS_USART);
        prvvUARTTxReadyISR();
    }

    if(LL_USART_IsActiveFlag_RXNE(BODBUS_USART) && LL_USART_IsEnabledIT_RXNE(BODBUS_USART))
    {
        LL_USART_ClearFlag_RXNE(BODBUS_USART);
        prvvUARTRxISR();
    }
}