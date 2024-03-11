/*
 *  Include Header file ...
 */
#include "./user_main.h"
#include "sockets.h"

/*
 *  \brief Function declaration ...
 */
void lvgl_thread(void *arg);
void iperf_thread(void *arg);
void sdram_speed_test();
void lwiperf_example_init(void);
void lvgl_adapter_layer_init(void);
void touch_controller_thread(void *arg);
void sdcard_performance_thread(void *arg);
void lvgl_thread_init(void);
void audio_thread(void *arg);
void file_system_mount(void);


/*
 *  \brief External variable definition ...
 */
 extern struct netif gnetif;
 extern FATFS SDFatFS;


/*
 *  \brief Variable definition ...
 */
osThreadId_t netTaskHandle;
osThreadId_t LvglTaskHandle;
osThreadId_t TouchTaskHandle;
osThreadId_t FileTaskHandle;

const osThreadAttr_t NetTask_attr = {
  .name = "netTask",
  .stack_size = 1280 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};

const osThreadAttr_t LvglTask_attr = {
  .name = "LvglTask",
  .stack_size = 1280 * 20,
  .priority = (osPriority_t) osPriorityBelowNormal,
};

const osThreadAttr_t TouchTask_attr = {
  .name = "TouchTask",
  .stack_size = 1280,
  .priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t FileTask_attr = {
  .name = "FileTask",
  .stack_size = 1280 * 2,
  .priority = (osPriority_t) osPriorityRealtime,
};



/*******************************************************************************
* User Code Area
*******************************************************************************/

/*
 * \brief system init
 */
void system_init(void)
{
//    netTaskHandle = osThreadNew(iperf_thread, NULL, &NetTask_attr);
//    if (!netTaskHandle) {
//        for (;;){}
//    }

//    FileTaskHandle = osThreadNew(audio_thread, NULL, &FileTask_attr);
//    if (!FileTaskHandle) {
//        for (;;){}
//    }
//
    lvgl_thread_init();

    return;
}


/*
 * File system mount
 */
void file_system_mount(void)
{
    uint8_t ret;

    ret = f_mount(&SDFatFS, "0", 1);
    if (ret != FR_OK) {
        PRINTF_LINE("File open failed!");
        for (;;);
    }
}



/*
 * LVGL init
 */
void lvgl_thread_init(void)
{
    LvglTaskHandle = osThreadNew(lvgl_thread, NULL, &LvglTask_attr);
    if (!LvglTaskHandle) {
        for (;;){}
    }

    TouchTaskHandle = osThreadNew(touch_controller_thread, NULL, &TouchTask_attr);
    if (!TouchTaskHandle) {
        for (;;){}
    }
    return;
}


/*
 * LVGL demo
 */
void lvgl_thread(void *arg)
{
	lv_init();
	lvgl_adapter_layer_init();
	lv_demo_benchmark();
//	lv_demo_stress();
//	lv_demo_widgets();

    #include "./Audio/gui_guider.h"
	static lv_ui audio_ui;
//	setup_ui(&audio_ui);
	for (;;)
	{
		lv_timer_handler();
		osDelay(2);
	}
}


/*
 * Iperf thread
 */
void iperf_thread(void *arg) {
    
    int sct;
    static int res;
    struct sockaddr_in dst_ip ;

//    lwiperf_example_init();
retry:
    memset(&dst_ip,0,sizeof(struct sockaddr_in));
    while(!netif_is_link_up(&gnetif))
    {
        osDelay(20);
    }
    sct = socket(AF_INET,SOCK_DGRAM,0);
    if(sct < 0){
        close(sct);
    }
    dst_ip.sin_port = htons(6666);
    dst_ip.sin_family = AF_INET;
    dst_ip.sin_addr.s_addr = inet_addr("192.168.0.198");
    dst_ip.sin_len = sizeof(struct sockaddr_in);

//    res = connect(sct,(struct sockaddr*)&dst_ip, sizeof(struct sockaddr_in));
//    if (res < 0) {
//        closesocket(sct);
//        osDelay(500);
//        goto retry;
//    }
    for (;;) {
//        res = write(sct, (void *)0xC0000000, 102400);
    	res = sendto(sct, (void *)0xC0000000, 60000, 0, (struct sockaddr*)&dst_ip, sizeof(struct sockaddr_in));
        if (res == -1) {
            closesocket(sct);
            goto retry;
        }
        osDelay(1);
    }
    return;
}


void defualt_thread_entry(void)
{
//	int Device_Id = 0;
//	extern SoftWareI2cStruct Ov2640_Instance;
//    sdram_speed_test();
//    lwiperf_example_init();

//    Device_Id = ov2640_ReadID(&Ov2640_Instance);
//    ov2640_Config(&Ov2640_Instance, 1, 1, 1);
//    ov2640_Init(&Ov2640_Instance, CAMERA_R320x240);
//
//    HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, 0xC1000000, 320*240*2);
//    while(1) {
//        osDelay(100);
//    }
}


void rodata_copy_to_ram(void)
{
	extern uint32_t _ex_sidata;
	extern uint32_t _ex_data_excu_star_addr;
	extern uint32_t _ex_data_excu_end_addr;
	extern uint32_t _ram_func_loader_addr;
    extern uint32_t _ram_func_excu_start_addr;
    extern uint32_t _ram_func_excu_end_addr;
    uint32_t len;

	len = ((uint32_t)&_ex_data_excu_end_addr) - \
	      ((uint32_t)&_ex_data_excu_star_addr);

	memcpy((void*)&_ex_data_excu_star_addr, (void*)&_ex_sidata, len);

    len = ((uint32_t)&_ram_func_excu_end_addr) - \
          ((uint32_t)&_ram_func_excu_start_addr);

    memcpy((void*)&_ram_func_excu_start_addr, (void*)&_ram_func_loader_addr, len);
	return;
}


__attribute__((section(".user_code")))
void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;
  
  /* Disable the MPU */
  HAL_MPU_Disable();
  
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress      = 0x24000000;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress      = 0x20000000;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_128KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER1;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  
  /* Configure the MPU attributes as Device not cacheable 
     for ETH DMA descriptors */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x30000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_128KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  
  /* Configure the MPU attributes as Normal Non Cacheable
     for LwIP RAM heap which contains the Tx buffers */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x30044000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_16KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER3;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* SDRAM */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0xC0000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_32MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER4;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* QSPI-FLASH */
//  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
//  MPU_InitStruct.BaseAddress = 0x90000000;
//  MPU_InitStruct.Size = MPU_REGION_SIZE_16MB;
//  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
//  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
//  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
//  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
//  MPU_InitStruct.Number = MPU_REGION_NUMBER5;
//  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
//  MPU_InitStruct.SubRegionDisable = 0x00;
//  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

//  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

