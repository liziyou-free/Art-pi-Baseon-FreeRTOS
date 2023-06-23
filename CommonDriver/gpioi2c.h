#ifndef  __GPIOI2C_DRIVER__
#define  __GPIOI2C_DRIVER__

#include "stm32h7xx_hal.h" 

struct _PinDescDef {
    GPIO_TypeDef *group;
    uint32_t      pin_num;
};

typedef struct _PinDescDef  SckPinDefStruct;
typedef struct _PinDescDef  SdaPinDefStruct;

typedef struct {
    SckPinDefStruct sck;
    SdaPinDefStruct sda;
    uint8_t         slaver_adrr; //device address
    /* private data */
    uint32_t        _tick;       //us
}SoftWareI2cStruct;


enum SdaStatus {
    SDA_OUTPUT = 0,
    SDA_INPUT
};


/*
 *  SoftWare I2C Device Instant Init.
 */
int8_t SoftWareI2cInit(SoftWareI2cStruct *DeviceInstance, uint32_t rate);


/*
 *  SoftWare I2C Write Data.
 */
int8_t SoftWareI2cWriteData(SoftWareI2cStruct *DeviceInstance, uint8_t reg_adr, uint8_t *pdata, uint32_t length);
  
/*
 *  SoftWare I2C Read Data.
 */
int8_t SoftWareI2cReadData(SoftWareI2cStruct *DeviceInstance, uint8_t reg_adr, uint8_t *pdata, uint32_t length);

/*
 *  SoftWare SCCB Write Data.
 */
void SoftWareSccbWriteData(SoftWareI2cStruct *DeviceInstance, uint8_t reg_adr, uint8_t *pdata, uint32_t length);
 
/*
 *  SoftWare SCCB Read Data.
 */
void SoftWareSccbReadData(SoftWareI2cStruct *DeviceInstance, uint8_t reg_adr, uint8_t *pdata, uint32_t length);

    

#endif /* __GPIOI2C_DRIVER__ */
 
