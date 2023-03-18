/*
 *  User Main Header File
 *                   -- By FreedomLi
 *                   -- 2023/02/13
 */

#include "main.h"
#include "stm32h7xx_hal.h"
#include "cmsis_os.h"
#include "fatfs.h"
#include "lwip.h"
#include "sdmmc.h"
#include "gpio.h"
#include "fmc.h"
#include "sockets.h"
#include <stdint.h>
#include "ff.h"
#include "gpioi2c.h"
#include "ov2640.h"
#include "dcmi.h"

void MPU_Config( void );
void thread_init(void);
void defualt_thread_entry(void);