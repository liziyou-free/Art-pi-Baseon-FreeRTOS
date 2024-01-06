#ifndef __GT9147_DRIVER_H__
#define __GT9147_DRIVER_H__    


#include "stm32h7xx_hal.h"


#define TP_PRES_DOWN     0x8000
#define TP_CATH_PRES     0x4000
#define CT_MAX_TOUCH     10        /* 5 points touch */

/* Touch screen controller device */
typedef struct
{
    uint8_t (*init)(void);
    uint8_t (*scan)(uint8_t);
    void (*adjust)(void);
    uint16_t x[CT_MAX_TOUCH];
    uint16_t y[CT_MAX_TOUCH];
    uint16_t sta;
    float xfac;
    float yfac;
    short xoff;
    short yoff;
    uint8_t touchtype;
}_m_tp_dev;


//I2C读写命令    
#define GT_DEV_ID       0x28
#define GT_CMD_WR         GT_DEV_ID                 //写地址
#define GT_CMD_RD         (GT_DEV_ID | 0x01)        //读地址
  
/* GT9147 Register Def  */
#define GT_CTRL_REG     0X8040
#define GT_CFGS_REG     0X8047
#define GT_CHECK_REG     0X80FF
#define GT_PID_REG         0X8140

#define GT_GSTID_REG     0X814E
#define GT_TP1_REG         0X8150
#define GT_TP2_REG         0X8158
#define GT_TP3_REG         0X8160
#define GT_TP4_REG         0X8168
#define GT_TP5_REG         0X8170
 
 
uint8_t GT9147_Send_Cfg(uint8_t mode);
uint8_t GT9147_WR_Reg(uint16_t reg,uint8_t *buf,uint8_t len);
void GT9147_RD_Reg(uint16_t reg,uint8_t *buf,uint8_t len); 
uint8_t GT9147_Init(void);
uint8_t GT9147_Scan();

#endif /* __GT9147_DRIVER_H__ */*/













