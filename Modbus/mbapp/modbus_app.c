#include "mb.h"
#include "mbport.h"
#include "modbus_app.h"

#define MB_COILS_COUNT                      6
#define MB_DISCRETE_INPUTS_COUNT            4
#define MB_ANALOG_IN_COUNT                  2
#define MB_HOLDING_COUNT                    14

#define MB_COILS_BUFFER_LENGTH              ((MB_COILS_COUNT+7)/8)
#define MB_DISCRETE_IN_BUFFER_LENGTH        ((MB_DISCRETE_INPUTS_COUNT+7)/8)

static uint8_t discrete_coils[MB_COILS_BUFFER_LENGTH];
static uint8_t discrete_inputs[MB_DISCRETE_IN_BUFFER_LENGTH];
static uint16_t analog_in_registers[MB_ANALOG_IN_COUNT];
static uint16_t analog_holding_registers[MB_HOLDING_COUNT];

void WriteBits(uint8_t* dest, uint8_t dest_len, uint8_t bitmap, uint8_t bits_count, uint8_t address);
uint8_t ReadBits(const uint8_t * src, uint8_t src_len, uint8_t bits_count, uint8_t address);

void WriteBits16(uint8_t* dest, uint8_t dest_len, uint16_t bitmap, uint8_t bits_count, uint8_t address);
uint16_t ReadBits16(const uint8_t * src, uint8_t src_len, uint8_t bits_count, uint8_t address);

eMBErrorCode MbInit(eMBMode eMode, UCHAR ucSlaveAddress)
{
    uint32_t ll_parity = LL_USART_GetParity(USART2);
    eMBParity mb_parity;

    switch (ll_parity)
    {
    case LL_USART_PARITY_NONE:
    {
        mb_parity = MB_PAR_NONE;
        break;
    }
    case LL_USART_PARITY_ODD:
    {
        mb_parity = MB_PAR_ODD;
        break;
    }
    case LL_USART_PARITY_EVEN:
    {
        mb_parity = MB_PAR_EVEN;
        break;
    }
    }

    LL_USART_EnableIT_RXNE(USART2);

    SystemCoreClockUpdate();
    //uint64_t baudrate = LL_USART_GetBaudRate(USART2,SystemCoreClock, LL_USART_GetOverSampling(USART2));
    eMBInit( MB_RTU, ucSlaveAddress, 0x00,115200,mb_parity );
    eMBEnable( );
}

/*! \defgroup modbus_registers Modbus Registers
 * \code #include "mb.h" \endcode
 * The protocol stack does not internally allocate any memory for the
 * registers. This makes the protocol stack very small and also usable on
 * low end targets. In addition the values don't have to be in the memory
 * and could for example be stored in a flash.<br>
 * Whenever the protocol stack requires a value it calls one of the callback
 * function with the register address and the number of registers to read
 * as an argument. The application should then read the actual register values
 * (for example the ADC voltage) and should store the result in the supplied
 * buffer.<br>
 * If the protocol stack wants to update a register value because a write
 * register function was received a buffer with the new register values is
 * passed to the callback function. The function should then use these values
 * to update the application register values.
 */


/*! \ingroup modbus_registers
 * \brief Callback function used if the value of a <em>Input Register</em>
 *   is required by the protocol stack. The starting register address is given
 *   by \c usAddress and the last register is given by <tt>usAddress +
 *   usNRegs - 1</tt>.
 *
 * \param pucRegBuffer A buffer where the callback function should write
 *   the current value of the Modbus registers to.
 * \param usAddress The starting address of the register. Input registers
 *   are in the range 1 - 65535.
 * \param usNRegs Number of registers the callback function must supply.
 *
 * \return The function must return one of the following error codes:
 *   - eMBErrorCode::MB_ENOERR If no error occurred. In this case a normal
 *       Modbus response is sent.
 *   - eMBErrorCode::MB_ENOREG If the application can not supply values
 *       for registers within this range. In this case a
 *       <b>ILLEGAL DATA ADDRESS</b> exception frame is sent as a response.
 *   - eMBErrorCode::MB_ETIMEDOUT If the requested register block is
 *       currently not available and the application dependent response
 *       timeout would be violated. In this case a <b>SLAVE DEVICE BUSY</b>
 *       exception is sent as a response.
 *   - eMBErrorCode::MB_EIO If an unrecoverable error occurred. In this case
 *       a <b>SLAVE DEVICE FAILURE</b> exception is sent as a response.
 */
eMBErrorCode    eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress,USHORT usNRegs )//Analog in read
{
    usAddress--;
    if(usAddress+usNRegs > MB_ANALOG_IN_COUNT)
        return MB_ENOREG;

    USHORT current_reg = usAddress;
    while(usNRegs--!=0)
    {
        *pucRegBuffer++ = analog_in_registers[current_reg] >> 8;
        *pucRegBuffer++ = analog_in_registers[current_reg] & 0xFF;
        current_reg++;
    }

    return MB_ENOERR;
}

/*! \ingroup modbus_registers
 * \brief Callback function used if a <em>Holding Register</em> value is
 *   read or written by the protocol stack. The starting register address
 *   is given by \c usAddress and the last register is given by
 *   <tt>usAddress + usNRegs - 1</tt>.
 *
 * \param pucRegBuffer If the application registers values should be updated the
 *   buffer points to the new registers values. If the protocol stack needs
 *   to now the current values the callback function should write them into
 *   this buffer.
 * \param usAddress The starting address of the register.
 * \param usNRegs Number of registers to read or write.
 * \param eMode If eMBRegisterMode::MB_REG_WRITE the application register
 *   values should be updated from the values in the buffer. For example
 *   this would be the case when the Modbus master has issued an
 *   <b>WRITE SINGLE REGISTER</b> command.
 *   If the value eMBRegisterMode::MB_REG_READ the application should copy
 *   the current values into the buffer \c pucRegBuffer.
 *
 * \return The function must return one of the following error codes:
 *   - eMBErrorCode::MB_ENOERR If no error occurred. In this case a normal
 *       Modbus response is sent.
 *   - eMBErrorCode::MB_ENOREG If the application can not supply values
 *       for registers within this range. In this case a
 *       <b>ILLEGAL DATA ADDRESS</b> exception frame is sent as a response.
 *   - eMBErrorCode::MB_ETIMEDOUT If the requested register block is
 *       currently not available and the application dependent response
 *       timeout would be violated. In this case a <b>SLAVE DEVICE BUSY</b>
 *       exception is sent as a response.
 *   - eMBErrorCode::MB_EIO If an unrecoverable error occurred. In this case
 *       a <b>SLAVE DEVICE FAILURE</b> exception is sent as a response.
 */
eMBErrorCode    eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress,USHORT usNRegs, eMBRegisterMode eMode )//Analog out read write
{
    usAddress--;
    if(usAddress+usNRegs > MB_HOLDING_COUNT)
        return MB_ENOREG;

    USHORT current_reg = usAddress;
    switch(eMode)
    {
    case MB_REG_READ:
    {
        while(usNRegs--!=0)
        {
            *pucRegBuffer++ = analog_holding_registers[current_reg] >> 8;
            *pucRegBuffer++ = analog_holding_registers[current_reg] & 0xFF;
            current_reg++;
        }
        break;
    }
    case MB_REG_WRITE:
    {
        while(usNRegs--!=0)
        {
            analog_holding_registers[current_reg] = (*pucRegBuffer++ << 8) | *pucRegBuffer++;
            current_reg++;
        }
        break;
    }
    }

    return MB_ENOERR;
}

/*! \ingroup modbus_registers
 * \brief Callback function used if a <em>Coil Register</em> value is
 *   read or written by the protocol stack. If you are going to use
 *   this function you might use the functions xMBUtilSetBits(  ) and
 *   xMBUtilGetBits(  ) for working with bitfields.
 *
 * \param pucRegBuffer The bits are packed in bytes where the first coil
 *   starting at address \c usAddress is stored in the LSB of the
 *   first byte in the buffer <code>pucRegBuffer</code>.
 *   If the buffer should be written by the callback function unused
 *   coil values (I.e. if not a multiple of eight coils is used) should be set
 *   to zero.
 * \param usAddress The first coil number.
 * \param usNCoils Number of coil values requested.
 * \param eMode If eMBRegisterMode::MB_REG_WRITE the application values should
 *   be updated from the values supplied in the buffer \c pucRegBuffer.
 *   If eMBRegisterMode::MB_REG_READ the application should store the current
 *   values in the buffer \c pucRegBuffer.
 *
 * \return The function must return one of the following error codes:
 *   - eMBErrorCode::MB_ENOERR If no error occurred. In this case a normal
 *       Modbus response is sent.
 *   - eMBErrorCode::MB_ENOREG If the application does not map an coils
 *       within the requested address range. In this case a
 *       <b>ILLEGAL DATA ADDRESS</b> is sent as a response.
 *   - eMBErrorCode::MB_ETIMEDOUT If the requested register block is
 *       currently not available and the application dependent response
 *       timeout would be violated. In this case a <b>SLAVE DEVICE BUSY</b>
 *       exception is sent as a response.
 *   - eMBErrorCode::MB_EIO If an unrecoverable error occurred. In this case
 *       a <b>SLAVE DEVICE FAILURE</b> exception is sent as a response.
 */


eMBErrorCode    eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress,USHORT usNCoils, eMBRegisterMode eMode )//Discrete out read/write
{
    usAddress--;
    if(usAddress+usNCoils>MB_COILS_COUNT)
        return MB_ENOREG;


    switch (eMode)
    {
    case MB_REG_READ:
    {
        // Received bytes completely filled with data
        while(usNCoils>= 8)
        {
            *pucRegBuffer = ReadBits(discrete_coils,MB_COILS_BUFFER_LENGTH,8,usAddress);
            *pucRegBuffer++;
            usNCoils -= 8;
            usAddress+=8;
        }
        // Received bytes NOT completely filled with data
        if(usNCoils>0)
            *pucRegBuffer = ReadBits(discrete_coils,MB_COILS_BUFFER_LENGTH,usNCoils,usAddress);

        break;
    }
    case MB_REG_WRITE:
    {
        // Received bytes completely filled with data
        while(usNCoils>= 8)
        {
            *pucRegBuffer = 0x00;
            WriteBits(discrete_coils,MB_COILS_BUFFER_LENGTH,*pucRegBuffer,8,usAddress);
            *pucRegBuffer++;
            usNCoils -= 8;
        }
        // Received bytes NOT completely filled with data
        if(usNCoils>0)
            WriteBits(discrete_coils,MB_COILS_BUFFER_LENGTH,*pucRegBuffer,usNCoils,usAddress);

        break;
    }
    }

    return MB_ENOERR;
}

/*! \ingroup modbus_registers
 * \brief Callback function used if a <em>Input Discrete Register</em> value is
 *   read by the protocol stack.
 *
 * If you are going to use his function you might use the functions
 * xMBUtilSetBits(  ) and xMBUtilGetBits(  ) for working with bitfields.
 *
 * \param pucRegBuffer The buffer should be updated with the current
 *   coil values. The first discrete input starting at \c usAddress must be
 *   stored at the LSB of the first byte in the buffer. If the requested number
 *   is not a multiple of eight the remaining bits should be set to zero.
 * \param usAddress The starting address of the first discrete input.
 * \param usNDiscrete Number of discrete input values.
 * \return The function must return one of the following error codes:
 *   - eMBErrorCode::MB_ENOERR If no error occurred. In this case a normal
 *       Modbus response is sent.
 *   - eMBErrorCode::MB_ENOREG If no such discrete inputs exists.
 *       In this case a <b>ILLEGAL DATA ADDRESS</b> exception frame is sent
 *       as a response.
 *   - eMBErrorCode::MB_ETIMEDOUT If the requested register block is
 *       currently not available and the application dependent response
 *       timeout would be violated. In this case a <b>SLAVE DEVICE BUSY</b>
 *       exception is sent as a response.
 *   - eMBErrorCode::MB_EIO If an unrecoverable error occurred. In this case
 *       a <b>SLAVE DEVICE FAILURE</b> exception is sent as a response.
 */
eMBErrorCode    eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress,USHORT usNDiscrete )//Discrete in read
{
    usAddress--;
    if(usAddress+usNDiscrete > MB_DISCRETE_INPUTS_COUNT)
        return MB_ENOREG;

    while(usNDiscrete>= 8)
    {
        *pucRegBuffer = ReadBits(discrete_inputs,MB_DISCRETE_IN_BUFFER_LENGTH,8,usAddress);
        usAddress+=8;
        *pucRegBuffer++;
        usNDiscrete -= 8;
    }
    // Received bytes NOT completely filled with data
    if(usNDiscrete>0)
        *pucRegBuffer = ReadBits(discrete_inputs,MB_DISCRETE_IN_BUFFER_LENGTH,usNDiscrete,usAddress);

    return MB_ENOERR;
}

void MbSetCoils(uint16_t bitmap, uint8_t address, uint8_t bits_count)
{
    WriteBits16(discrete_coils,MB_COILS_BUFFER_LENGTH,bitmap,bits_count,address);
}

uint16_t MbGetCoils(uint8_t offset, uint8_t count)
{
    return ReadBits16(discrete_coils,MB_COILS_BUFFER_LENGTH,count,offset);
}

void MbSetDiscreteInputs(uint16_t bitmap, uint8_t address, uint8_t bits_count)
{
    WriteBits16(discrete_inputs,MB_DISCRETE_IN_BUFFER_LENGTH,bitmap,bits_count,address);
}

uint16_t MbGetDiscreteInputs(uint8_t offset, uint8_t count)
{
    return ReadBits16(discrete_inputs,MB_DISCRETE_IN_BUFFER_LENGTH,count,offset);
}

void MbSetInputRegisters(uint16_t value, uint8_t index)
{
    analog_in_registers[index] = value;
}

uint16_t MbGetInputRegisters(uint8_t index)
{
    return analog_in_registers[index];
}

void MbSetHoldingRegisters(uint16_t value, uint8_t index)
{
    analog_holding_registers[index] = value;
}

uint16_t MbGetHoldingRegisters(uint8_t index)
{
    return analog_holding_registers[index];
}


BOOL MbSetAnalogOut(uint16_t value, uint8_t index)
{
    analog_in_registers[index] = value;
}

uint8_t MbGetCoilsCount()
{
    return MB_COILS_COUNT;
}

uint8_t MbGetDiscretInCount()
{
    return MB_DISCRETE_INPUTS_COUNT;
}

uint8_t MbGetHoldingCount()
{
    return MB_HOLDING_COUNT;
}

uint8_t MbGetAnalogInCount()
{
    return MB_ANALOG_IN_COUNT;
}

//------------------ HELP FUNCTIONS ---------------------

void WriteBits(uint8_t* dest, uint8_t dest_len, uint8_t bitmap, uint8_t bits_count, uint8_t address)
{
    uint8_t byte_number = address / 8;
    dest += byte_number;
    uint8_t bits_shift = address % 8;
    // Bit mask
    uint8_t mask = (1 << bits_count) - 1;
    // Set right part of bitmap to mass
    *dest &= ~(mask << bits_shift);
    *dest |= (bitmap & mask) << bits_shift;
    // Set left part of bitmap to mass
    if(++byte_number < dest_len)
    {
        dest++;
        *dest &= ~(mask >> (8 - bits_shift));
        *dest |= (bitmap & mask) >> (8 - bits_shift);
    }
}

uint8_t ReadBits(const uint8_t * src, uint8_t src_len, uint8_t bits_count, uint8_t address)
{
    uint8_t result = 0;
    uint8_t byte_number = address / 8;
    src += byte_number;
    uint8_t bits_shift = address % 8;
    // Bit mask
    uint8_t mask = (1<< bits_count) - 1;
    // Get right part of bitmap to mass
    result |= *src >> bits_shift;
    // Get left part of bitmap to mass
    if(++byte_number < src_len)
    {
        src++;
        result |= *src << (8 - bits_shift);
    }

    return result;
}

void WriteBits16(uint8_t* dest, uint8_t dest_len, uint16_t bitmap, uint8_t bits_count, uint8_t address)
{
    if(bits_count>8)
    {
        WriteBits(dest,dest_len, bitmap & 0xFF, 8, address);
        WriteBits(dest,dest_len, bitmap >> 8, bits_count-8, address+8);
    }
    else
        WriteBits(dest,dest_len, bitmap & 0xFF, bits_count, address);

}

uint16_t ReadBits16(const uint8_t * src, uint8_t src_len, uint8_t bits_count, uint8_t address)
{
    uint16_t result_bitmap = 0;

    if(bits_count>8)
        result_bitmap = ReadBits(src,src_len, 8, address) | ReadBits(src, src_len, bits_count - 8, address) << 8;
    else
        result_bitmap = ReadBits(src, src_len, bits_count, address);

    return result_bitmap;
}