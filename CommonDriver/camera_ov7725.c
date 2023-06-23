/*
 *  OV7725 Camera Driver Software.
 *
 *                    -- Author: By FreedomLI
 *                    -- Date  : 2023.03.25
 */


#include "gpioi2c.h"
#include "camera_ov7725.h"

/*
 * OV7725 Camera Init Data.
 */
Reg_Info sensor_config[] =
{
    {0x32,0x00},
    {0x2a,0x00},
    {0x11,0x03},
    {0x12,0x46},//QVGA RGB565

    {0x42,0x7f},
    {0x4d,0x00},
    {0x63,0xf0},
    {0x64,0x1f},
    {0x65,0x20},
    {0x66,0x00},
    
   
    {0x67,0x00},    
 
    {0x69,0x50},      
    {0x13,0xff},
    {0x0d,0x41},
    {0x0f,0x01},
    {0x14,0x06},
    
    {0x24,0x75},
    {0x25,0x63},
    {0x26,0xd1},
    {0x2b,0xff},
    {0x6b,0xaa},
    
    {0x8e,0x10},
    {0x8f,0x00},
    {0x90,0x00},
    {0x91,0x00},
    {0x92,0x00},
    {0x93,0x00},
    
    {0x94,0x2c},
    {0x95,0x24},
    {0x96,0x08},
    {0x97,0x14},
    {0x98,0x24},
    {0x99,0x38},
    {0x9a,0x9e},
    {0x15,0x00}, 
    {0x9b,0x00},
    {0x9c,0x20},
    {0xa7,0x40},  
    {0xa8,0x40},
    {0xa9,0x80},
    {0xaa,0x80},
    
    {0x9e,0x81},
    {0xa6,0x06},
    
    {0x7e,0x0c},
    {0x7f,0x16},
    {0x80,0x2a},
    {0x81,0x4e},
    {0x82,0x61},
    {0x83,0x6f},
    {0x84,0x7b},
    {0x85,0x86},
    {0x86,0x8e},
    {0x87,0x97},
    {0x88,0xa4},
    {0x89,0xaf},
    {0x8a,0xc5},
    {0x8b,0xd7},
    {0x8c,0xe8},
    {0x8d,0x20},
    
    {0x33,0x40},
    {0x34,0x00},
    {0x22,0xaf},
    {0x23,0x01},
    
    {0x49,0x10},
    {0x4a,0x10},
    {0x4b,0x14},
    {0x4c,0x17},
    {0x46,0x05},
    
    {0x47,0x08},
    {0x0e,0x01},
    {0x0c,0x60},
    {0x09,0x03},
    
    {0x29,0x50},
    {0x2C,0x78},
       
    {0x00,0x00},
    {0x00,0x00},
    {0x00,0x60},           
};

/* Array Length */
#define OV7725_REG_NUM     (sizeof(sensor_config) / sizeof(sensor_config[0]))



/*
 * **************************************************************************
 *  OV2640_Private_Functions
 * **************************************************************************
 */
void CAMERA_IO_Init(SoftWareI2cStruct *pInstatnce, uint32_t rate){
    SoftWareI2cInit(pInstatnce, rate);
}


void CAMERA_IO_Write(SoftWareI2cStruct *pInstatnce, uint8_t reg, uint8_t value){

    SoftWareSccbWriteData(pInstatnce, reg, &value, 1);
}


uint8_t CAMERA_IO_Read(SoftWareI2cStruct *pInstatnce, uint8_t reg){

    uint8_t value = 0;

    SoftWareSccbReadData(pInstatnce, reg, &value, 1);
    return value;
}


void CAMERA_Delay(uint32_t delay){

    delay *= 1000000;//10000;
    while(--delay);
}

/* Ov7725 Device Instance */
SoftWareI2cStruct Ov7725_Instatnce = {
    .sck = {
        .group = GPIOH,
        .pin_num = GPIO_PIN_15,
    },
    .sda = {
        .group = GPIOH,
        .pin_num = GPIO_PIN_13,
    },
    .slaver_adrr = Dev_Addr,
};



/*
 * **************************************************************************
 *  OV7725 Init Functions
 * **************************************************************************
 */
ErrorStatus ov7725_init(void)
{
    uint16_t i = 0;
    uint16_t CameraId = 0;
    
    CAMERA_IO_Init(&Ov7725_Instatnce, 200000U);

    /* Software Reset */
    CAMERA_IO_Write(&Ov7725_Instatnce, 0x12, 0x80);
    /* Wait Camera Reset Success! */
    CAMERA_Delay(10);

    /* Read Camera Id */
    CameraId = CAMERA_IO_Read(&Ov7725_Instatnce, 0x0A) << 8;
    CameraId |= CAMERA_IO_Read(&Ov7725_Instatnce, 0x0B);
    
    if(CameraId == OV7725_ID)
    {
    for( i = 0 ; i < OV7725_REG_NUM ; i++ )
    {
        CAMERA_IO_Write(&Ov7725_Instatnce, sensor_config[i].Address, sensor_config[i].Value);
    }
    }
    else
    {
    return ERROR;
    }

    return SUCCESS;
}



/******************************* END OF FILE *********************************/
