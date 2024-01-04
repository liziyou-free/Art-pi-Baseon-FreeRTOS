/*
 *  GPIO-I2C Driver Software.
 *
 *                    -- Author: By FreedomLI 
 *                    -- Date  : 2023.03.15
 */
 
 
/*
 *  Header File Include
 */
#include "gpioi2c.h"

#pragma GCC push_options
#pragma GCC optimize ("O0")

static void udelay(uint16_t t) {
    
    volatile uint32_t k = 0;
    k = 30 * t;
    while(--k);
}



/*
 *  Set Pin Mode.
 */
static void SdaStatusConf(SoftWareI2cStruct *DeviceInstance, enum SdaStatus status) {
    
    GPIO_InitTypeDef GpioInitStruct;
    uint32_t mode ;
    
    if(status == SDA_OUTPUT) {
        mode = GPIO_MODE_OUTPUT_OD;
    } else {
        mode = GPIO_MODE_INPUT;
    }
    /* Set clk-pin output mode: pullup */
    GpioInitStruct.Pin = DeviceInstance->sda.pin_num;
    GpioInitStruct.Mode = mode;
    GpioInitStruct.Pull = GPIO_PULLUP;
    GpioInitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(DeviceInstance->sda.group, &GpioInitStruct);
}



/*
 *  Set Pin Voltage Level.
 */
static void PinLevelConf(struct _PinDescDef *PinInstance, GPIO_PinState level) {
   
    HAL_GPIO_WritePin(PinInstance->group, PinInstance->pin_num, level);
}



/*
 *  Read Pin Voltage Level.
 */
static GPIO_PinState ReadPinLevel(struct _PinDescDef *PinInstance) {
   
    return HAL_GPIO_ReadPin(PinInstance->group, PinInstance->pin_num);
}


/*
 *  Generate Start Signal.
 */
static void GenerateStartSignal(SoftWareI2cStruct *DeviceInstance){
    
    SdaStatusConf(DeviceInstance, SDA_OUTPUT);
    PinLevelConf(&DeviceInstance->sda, GPIO_PIN_SET);
    PinLevelConf(&DeviceInstance->sck, GPIO_PIN_SET);
    udelay(DeviceInstance->_tick);
    PinLevelConf(&DeviceInstance->sda, GPIO_PIN_RESET);
    udelay(DeviceInstance->_tick);
    PinLevelConf(&DeviceInstance->sck, GPIO_PIN_RESET);
}


/*
 *  Generate Stop Signal.
 */
static void GenerateStopSignal(SoftWareI2cStruct *DeviceInstance){
    
    SdaStatusConf(DeviceInstance, SDA_OUTPUT);
    PinLevelConf(&DeviceInstance->sck, GPIO_PIN_RESET);
    PinLevelConf(&DeviceInstance->sda, GPIO_PIN_RESET);
    udelay(DeviceInstance->_tick);
    PinLevelConf(&DeviceInstance->sck, GPIO_PIN_SET);
    udelay(DeviceInstance->_tick);
    PinLevelConf(&DeviceInstance->sda, GPIO_PIN_SET);
    /* ���� SCK,SDA ������͹��� */
    udelay(DeviceInstance->_tick);
    PinLevelConf(&DeviceInstance->sck, GPIO_PIN_RESET);
    PinLevelConf(&DeviceInstance->sda, GPIO_PIN_RESET);
}


/*
 *  Wait Ack Signal.
 */
static int8_t WaitAckSignal(SoftWareI2cStruct *DeviceInstance)
{
    int32_t MaxTryCnt = 5;
    
    udelay(DeviceInstance->_tick/2);
    PinLevelConf(&DeviceInstance->sda, GPIO_PIN_SET);
    SdaStatusConf(DeviceInstance, SDA_INPUT);
    udelay(DeviceInstance->_tick/2);
    PinLevelConf(&DeviceInstance->sck, GPIO_PIN_SET);
    udelay(DeviceInstance->_tick/2);
    while(ReadPinLevel(&DeviceInstance->sda) && MaxTryCnt--) {
        if(MaxTryCnt <= 0) {
            /* TimeOut,Send Stop Signal.*/
            GenerateStopSignal(DeviceInstance);
            return -1;
        }
    }
    udelay(DeviceInstance->_tick/2);
    PinLevelConf(&DeviceInstance->sck, GPIO_PIN_RESET);
    
    return 0; 
}


/*
 *  Generate Ack Signal.
 */
static void GenerateAckSignal(SoftWareI2cStruct *DeviceInstance){
    
    udelay(DeviceInstance->_tick/2);
    SdaStatusConf(DeviceInstance, SDA_OUTPUT);
    PinLevelConf(&DeviceInstance->sda, GPIO_PIN_RESET);
    udelay(DeviceInstance->_tick/2);
    PinLevelConf(&DeviceInstance->sck, GPIO_PIN_SET);
    udelay(DeviceInstance->_tick);
    PinLevelConf(&DeviceInstance->sck, GPIO_PIN_RESET);
}


/*
 *  Generate Nack Signal.
 */
static void GenerateNackSignal(SoftWareI2cStruct *DeviceInstance){
    
    udelay(DeviceInstance->_tick/2);
    SdaStatusConf(DeviceInstance, SDA_OUTPUT);
    PinLevelConf(&DeviceInstance->sda, GPIO_PIN_SET);
    udelay(DeviceInstance->_tick/2);
    PinLevelConf(&DeviceInstance->sck, GPIO_PIN_SET);
    udelay(DeviceInstance->_tick);
    PinLevelConf(&DeviceInstance->sck, GPIO_PIN_RESET);
}


/*
 *  SoftWare I2C Send Byte.
 */
static void SoftWareI2cSendByte(SoftWareI2cStruct *DeviceInstance, uint8_t data){
    
    
    SdaStatusConf(DeviceInstance, SDA_OUTPUT);
    PinLevelConf(&DeviceInstance->sck, GPIO_PIN_RESET);
    
    for(uint8_t j = 0; j < 8; j++)
    {
        udelay(DeviceInstance->_tick/2);
        PinLevelConf(&DeviceInstance->sda, ((data & 0x80) >> 7) );
        data <<= 1;
        udelay(DeviceInstance->_tick/2);
        PinLevelConf(&DeviceInstance->sck, GPIO_PIN_SET);
        udelay(DeviceInstance->_tick+1);
        PinLevelConf(&DeviceInstance->sck, GPIO_PIN_RESET);
    }
    return;
} 


/*
 *  SoftWare I2C Read Byte.
 */
static uint8_t SoftWareI2cReadByte(SoftWareI2cStruct *DeviceInstance){
    
    uint8_t data = 0;
    
    SdaStatusConf(DeviceInstance, SDA_INPUT);
    
    for(uint8_t j = 0; j < 8; j++)
    {
        PinLevelConf(&DeviceInstance->sck, GPIO_PIN_RESET);
        udelay(DeviceInstance->_tick);
        PinLevelConf(&DeviceInstance->sck, GPIO_PIN_SET);
        udelay(DeviceInstance->_tick/2);
        data <<= 1;
        if(ReadPinLevel(&DeviceInstance->sda)) {
            data++;
        }
        udelay(DeviceInstance->_tick/2+1);
    }
    PinLevelConf(&DeviceInstance->sck, GPIO_PIN_RESET);
    return data;
}



/*
 *  SoftWare I2C Device Instant Init.
 */
int8_t SoftWareI2cInit(SoftWareI2cStruct *DeviceInstance, uint32_t rate) {
    
    GPIO_InitTypeDef GpioInitStruct;
    struct _PinDescDef *PinInstance = (struct _PinDescDef *)DeviceInstance;
    
    if(DeviceInstance == NULL) {
        return -1;
    }
    /* Frequency(Hz) -> Cycle(us) */
    DeviceInstance->_tick = (uint32_t)((double)1000000.0 / (double)rate);
    
    /* <= 250KHz */
    if(DeviceInstance->_tick < 2) {
        DeviceInstance->_tick = 2;
    }
    
    /* Enable the clock corresponding to the pins */
    for(int j = 0; j < sizeof(*DeviceInstance) / sizeof(struct _PinDescDef); j++) {
        
        switch ((uint32_t)DeviceInstance->sck.group) {
            case (uint32_t)GPIOA:  __HAL_RCC_GPIOA_CLK_ENABLE();   break;
            case (uint32_t)GPIOB:  __HAL_RCC_GPIOB_CLK_ENABLE();   break;
            case (uint32_t)GPIOC:  __HAL_RCC_GPIOC_CLK_ENABLE();   break;
            case (uint32_t)GPIOD:  __HAL_RCC_GPIOD_CLK_ENABLE();   break;
            case (uint32_t)GPIOE:  __HAL_RCC_GPIOE_CLK_ENABLE();   break;
            case (uint32_t)GPIOF:  __HAL_RCC_GPIOF_CLK_ENABLE();   break;
            case (uint32_t)GPIOG:  __HAL_RCC_GPIOG_CLK_ENABLE();   break;
            case (uint32_t)GPIOH:  __HAL_RCC_GPIOH_CLK_ENABLE();   break;
            case (uint32_t)GPIOI:  __HAL_RCC_GPIOI_CLK_ENABLE();   break;
            case (uint32_t)GPIOJ:  __HAL_RCC_GPIOJ_CLK_ENABLE();   break;
            case (uint32_t)GPIOK:  __HAL_RCC_GPIOK_CLK_ENABLE();   break;
        }
        PinInstance++;
    }
    
    /* Set clk-pin output mode: pullup */
    GpioInitStruct.Pin = DeviceInstance->sck.pin_num;
    GpioInitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GpioInitStruct.Pull = GPIO_NOPULL;
    GpioInitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(DeviceInstance->sck.group, &GpioInitStruct);
    
    return 0;
}


/*
 *  SoftWare I2C Write Data.
 */
int8_t SoftWareI2cWriteData(SoftWareI2cStruct *DeviceInstance, uint8_t reg_adr, uint8_t *pdata, uint32_t length){
    
    int8_t ret;
    
    GenerateStartSignal(DeviceInstance);
    SoftWareI2cSendByte(DeviceInstance, DeviceInstance->slaver_adrr & (~0x01));
    ret = WaitAckSignal(DeviceInstance);
    SoftWareI2cSendByte(DeviceInstance, reg_adr);
    ret |= WaitAckSignal(DeviceInstance);
    if(ret < 0){
        GenerateStopSignal(DeviceInstance);
        return -1;
    }
    for(uint32_t j = 0; j < (length - 1); j++){
        SoftWareI2cSendByte(DeviceInstance, *pdata);
        ret = WaitAckSignal(DeviceInstance);
        if(ret < 0){
            GenerateStopSignal(DeviceInstance);
            return -1;
        }
        pdata++;
    }
    SoftWareI2cSendByte(DeviceInstance, *pdata);
    GenerateNackSignal(DeviceInstance);
    GenerateStopSignal(DeviceInstance);
    return 0;
}


/*
 *  SoftWare I2C Read Data.
 */
int8_t SoftWareI2cReadData(SoftWareI2cStruct *DeviceInstance, uint8_t reg_adr, uint8_t *pdata, uint32_t length){
    
    int8_t ret;
    
    GenerateStartSignal(DeviceInstance);
    SoftWareI2cSendByte(DeviceInstance, DeviceInstance->slaver_adrr | 0x01);
    ret = WaitAckSignal(DeviceInstance);
    SoftWareI2cSendByte(DeviceInstance, reg_adr);
    ret |= WaitAckSignal(DeviceInstance);
    if(ret < 0){
        GenerateStopSignal(DeviceInstance);
        return -1;
    }
    
    for(uint32_t j = 0; j < (length - 1); j++){
        *pdata = SoftWareI2cReadByte(DeviceInstance); 
        GenerateAckSignal(DeviceInstance);
        pdata++;
    }
    *pdata = SoftWareI2cReadByte(DeviceInstance);
    GenerateNackSignal(DeviceInstance);
    GenerateStopSignal(DeviceInstance);
    return 0;
}




/*************    SCCB Interface Implementation    *************/

/*
 *  Generate SCCB Don`t Care Signal.
 */
static void SccbNoCareSignal(SoftWareI2cStruct *DeviceInstance)
{
    udelay(DeviceInstance->_tick/2);
    PinLevelConf(&DeviceInstance->sda, GPIO_PIN_SET);
    SdaStatusConf(DeviceInstance, SDA_INPUT);
    udelay(DeviceInstance->_tick/2);
    PinLevelConf(&DeviceInstance->sck, GPIO_PIN_SET);
    udelay(DeviceInstance->_tick);
    PinLevelConf(&DeviceInstance->sck, GPIO_PIN_RESET);
    return ; 
}

/*
 *  SoftWare SCCB Write Data.
 */
void SoftWareSccbWriteData(SoftWareI2cStruct *DeviceInstance, uint8_t reg_adr, uint8_t *pdata, uint32_t length){
    
    GenerateStartSignal(DeviceInstance);
    SoftWareI2cSendByte(DeviceInstance, DeviceInstance->slaver_adrr & (~0x01));
    SccbNoCareSignal(DeviceInstance);
    SoftWareI2cSendByte(DeviceInstance, reg_adr);
    SccbNoCareSignal(DeviceInstance);
    for(uint32_t j = 0; j < (length - 1); j++){
        SoftWareI2cSendByte(DeviceInstance, *pdata);
        SccbNoCareSignal(DeviceInstance);
        pdata++;
    }
    SoftWareI2cSendByte(DeviceInstance, *pdata);
    GenerateNackSignal(DeviceInstance);
    GenerateStopSignal(DeviceInstance);
    return;
}


/*
 *  SoftWare SCCB Read Data.
 */
void SoftWareSccbReadData(SoftWareI2cStruct *DeviceInstance, uint8_t reg_adr, uint8_t *pdata, uint32_t length){
    
    GenerateStartSignal(DeviceInstance);
    SoftWareI2cSendByte(DeviceInstance, DeviceInstance->slaver_adrr & (~0x01));
    SccbNoCareSignal(DeviceInstance);
    SoftWareI2cSendByte(DeviceInstance, reg_adr);
    SccbNoCareSignal(DeviceInstance);
    GenerateStopSignal(DeviceInstance);
    
    GenerateStartSignal(DeviceInstance);
    SoftWareI2cSendByte(DeviceInstance, DeviceInstance->slaver_adrr | 0x01);
    SccbNoCareSignal(DeviceInstance);
    for(uint32_t j = 0; j < (length - 1); j++){
        *pdata = SoftWareI2cReadByte(DeviceInstance); 
        GenerateNackSignal(DeviceInstance);
        pdata++;
    }
    *pdata = SoftWareI2cReadByte(DeviceInstance);
    GenerateNackSignal(DeviceInstance);
    GenerateStopSignal(DeviceInstance);
    return;
}


#pragma GCC pop_options

