#include <stdio.h>
#include <string.h>
#include "main.h"
#include "gt9147.h"
#include "cmsis_os2.h"
#include "stm32h7xx_hal.h"


extern I2C_HandleTypeDef hi2c1;
extern DMA_HandleTypeDef hdma_i2c1_tx;
extern DMA_HandleTypeDef hdma_i2c1_rx;

osSemaphoreId_t touch_tx_semp_id;
osSemaphoreId_t touch_rx_semp_id;
osSemaphoreId_t touch_int_semp_id;

const osSemaphoreAttr_t touch_tx_semp_attr = {.name = "touch-tx-semp"};
const osSemaphoreAttr_t touch_rx_semp_attr = {.name = "touch-rx-semp",};
const osSemaphoreAttr_t touch_int_semp_attr = {.name = "touch-int-semp",};

void HAL_I2C_MasterDmaTxCpltCallback (DMA_HandleTypeDef *_hdma);
void HAL_I2C_MasterDmaRxCpltCallback (DMA_HandleTypeDef *_hdma);


void hal_i2c_init(void)
{
    touch_tx_semp_id = osSemaphoreNew(1, 0, &touch_tx_semp_attr);
    touch_rx_semp_id = osSemaphoreNew(1, 0, &touch_rx_semp_attr);
    touch_int_semp_id = osSemaphoreNew(1, 0, &touch_int_semp_attr);

    if (touch_tx_semp_id == NULL || touch_rx_semp_id == NULL || \
        touch_int_semp_id == NULL) {
        printf("Semaphore Create Fail! %s %s\r\n",__FILE__, __FUNCTION__);
        for (;;);
    }
    return;
}


void hal_i2c_write(uint16_t reg, void *pdata, uint16_t len)
{
    uint8_t txbuf[255];

    if (len > sizeof(txbuf)) {
        for (;;);
    }
    txbuf[0] = reg >> 8;
    txbuf[1] = reg & 0x00ff;
    memcpy((void *)&txbuf[2], (const void *)pdata, len);

    HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, GT_DEV_ID, (uint8_t*)txbuf, len + 2,
                                   I2C_FIRST_AND_LAST_FRAME);
    if (osSemaphoreAcquire(touch_tx_semp_id, 100) != osOK) {
        printf("Semaphore Acquire Fail! %s %s\r\n",__FILE__, __FUNCTION__);
    }
    return;
}


void hal_i2c_read(uint16_t reg, void *pdata, uint16_t len)
{
    uint16_t regval;

    /* swap byte order */
    regval = reg >> 8;
    regval |= (reg & 0x00ff) << 8;

    HAL_I2C_Master_Seq_Transmit_IT(&hi2c1,GT_DEV_ID, (uint8_t *)&regval, 2,
                                   I2C_FIRST_FRAME);
    if (osSemaphoreAcquire(touch_tx_semp_id, 50) != osOK) {
        printf("Semaphore Acquire Fail! %s %s",__FILE__, __FUNCTION__);
    }
    HAL_I2C_Master_Seq_Receive_IT(&hi2c1,GT_DEV_ID, (uint8_t *)pdata, len,
                                  I2C_LAST_FRAME);
    if (osSemaphoreAcquire(touch_rx_semp_id, 50) != osOK) {
        printf("Semaphore Acquire Fail! %s %s",__FILE__, __FUNCTION__);
    }
    return;
}


void touch_controller_thread(void *arg)
{
    GT9147_Init();
    for (;;) {
//        if (osSemaphoreAcquire(touch_int_semp_id, 5) == osOK) {
            GT9147_Scan();
            osDelay(20);
//        }
    }
    return;
}


void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    osSemaphoreRelease(touch_tx_semp_id);
    return;
}


void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    osSemaphoreRelease(touch_rx_semp_id);
    return;
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin){
    case GPIO_TOUCHSCREEN_INT_Pin:
        osSemaphoreRelease(touch_int_semp_id);
        break;
    }
}


/*******************************************************************************
 * Portable interface layer
 ******************************************************************************/

#define touch_i2c_init()  hal_i2c_init()

#define touch_i2c_write(reg, pdata, len)  hal_i2c_write(reg, pdata, len)

#define touch_i2c_read(reg, pdata, len)   hal_i2c_read(reg, pdata, len)



 /*******************************************************************************
  *  Touch Contorller Driver Programmer
  ******************************************************************************/

_m_tp_dev tp_dev = {GT9147_Init, GT9147_Scan, 0, 0, 0, 0, 0, 0, 0, 0, 1 };


/*
 * \brief Default Configure Table
 */
const uint8_t GT9147_CFG_TBL[]=
{ 
    0X60,0XE0,0X01,0X20,0X03,0X05,0X35,0X00,0X02,0X08,
    0X1E,0X08,0X50,0X3C,0X0F,0X05,0X00,0X00,0XFF,0X67,
    0X50,0X00,0X00,0X18,0X1A,0X1E,0X14,0X89,0X28,0X0A,
    0X30,0X2E,0XBB,0X0A,0X03,0X00,0X00,0X02,0X33,0X1D,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X32,0X00,0X00,
    0X2A,0X1C,0X5A,0X94,0XC5,0X02,0X07,0X00,0X00,0X00,
    0XB5,0X1F,0X00,0X90,0X28,0X00,0X77,0X32,0X00,0X62,
    0X3F,0X00,0X52,0X50,0X00,0X52,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X0F,
    0X0F,0X03,0X06,0X10,0X42,0XF8,0X0F,0X14,0X00,0X00,
    0X00,0X00,0X1A,0X18,0X16,0X14,0X12,0X10,0X0E,0X0C,
    0X0A,0X08,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0X29,0X28,0X24,0X22,0X20,0X1F,0X1E,0X1D,
    0X0E,0X0C,0X0A,0X08,0X06,0X05,0X04,0X02,0X00,0XFF,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
    0XFF,0XFF,0XFF,0XFF,
};  



uint8_t GT9147_Send_Cfg(uint8_t mode)
{
    uint8_t buf[2];
    uint8_t i=0;
    buf[0]=0;
    buf[1]=mode;
    for(i=0;i<sizeof(GT9147_CFG_TBL);i++)buf[0]+=GT9147_CFG_TBL[i];
    buf[0]=(~buf[0])+1;
    touch_i2c_write(GT_CFGS_REG,(uint8_t*)GT9147_CFG_TBL,sizeof(GT9147_CFG_TBL));
    touch_i2c_write(GT_CHECK_REG,buf,2);
    return 0;
} 


uint8_t GT9147_Init(void)
{
    uint8_t temp[5] = {0};

    touch_i2c_init();
    HAL_Delay(10);
    touch_i2c_read(GT_PID_REG,temp,4);
    if(strcmp((char*)temp,"9147")==0){
        temp[0]=0X02;   
        touch_i2c_write(GT_CTRL_REG,temp,1);
        touch_i2c_read(GT_CFGS_REG,temp,1);
        if(temp[0]<0X60)
        {
            GT9147_Send_Cfg(1);
        }
        HAL_Delay(10);
        temp[0]=0X00;   
        touch_i2c_write(GT_CTRL_REG,temp,1);
        return 0;
    }
    return 0;
}



uint8_t GT9147_Scan()
{
    uint8_t mode;
    uint8_t buf[4];
    uint8_t i=0;
    uint8_t res=0;
    uint16_t temp;
    uint16_t tempsta;
    const uint16_t GT9147_TPX_TBL[5] = {
            GT_TP1_REG, GT_TP2_REG,
            GT_TP3_REG, GT_TP4_REG,
            GT_TP5_REG
    };

    tp_dev.touchtype=1;

    touch_i2c_read(GT_GSTID_REG,&mode,1);
    if(mode&0X80&&((mode&0XF)<6)) {
        i=0;
        touch_i2c_write(GT_GSTID_REG,&i,1);
    }
    if((mode&0XF)&&((mode&0XF)<6))
    {
        temp=0XFFFF<<(mode&0XF);
        tempsta=tp_dev.sta;
        tp_dev.sta=(~temp)|TP_PRES_DOWN|TP_CATH_PRES;
        tp_dev.x[4]=tp_dev.x[0];
        tp_dev.y[4]=tp_dev.y[0];
        for(i=0;i<5;i++)
        {
            if(tp_dev.sta&(1<<i)){
                touch_i2c_read(GT9147_TPX_TBL[i],buf,4);
                if(tp_dev.touchtype&0X01){
                    tp_dev.x[i]=(((uint16_t)buf[1]<<8)+buf[0]);
                    tp_dev.y[i]=(((uint16_t)buf[3]<<8)+buf[2]);
                 } else {
                    tp_dev.y[i]=((uint16_t)buf[1]<<8)+buf[0];
                    tp_dev.x[i]=480-(((uint16_t)buf[3]<<8)+buf[2]);
                 }
            }
        } 
        res=1;
        if(tp_dev.x[0]>800||tp_dev.y[0]>480) {
            if((mode&0XF)>1)
            {
                tp_dev.x[0]=tp_dev.x[1];
                tp_dev.y[0]=tp_dev.y[1];
            } else {
                tp_dev.x[0]=tp_dev.x[4];
                tp_dev.y[0]=tp_dev.y[4];
                mode=0X80;
                tp_dev.sta=tempsta;
            }
        }
    }
    if((mode&0X8F)==0X80) {
        if(tp_dev.sta&TP_PRES_DOWN) {
            tp_dev.sta&=~TP_PRES_DOWN;
        } else {
            tp_dev.x[0]=0xffff;
            tp_dev.y[0]=0xffff;
            tp_dev.sta&=0XE000;
        }
    }
    return res;
}
 
