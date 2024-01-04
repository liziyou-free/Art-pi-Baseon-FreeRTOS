/*
 *  Include Header file ...
 */
#include "./user_main.h"


/*
 *  Function declaration ...
 */
void lvgl_task(void *arg);
void lwiperf_example_init(void);
void lvgl_adapter_layer_init(void);
void Net_Task_BaseOn_FreeRTOS(void *argumnet);
void sdram_speed_test();


/*
 *  External variable definition ...
 */
 extern struct netif gnetif;
 
 
/*
 *  Variable definition ...
 */
osThreadId_t netTaskHandle;
osThreadId_t LvglTaskHandle;

const osThreadAttr_t netTask_attributes = {
  .name = "netTask",
  .stack_size = 1280 * 4,
  .priority = (osPriority_t) osPriorityNormal3,
};

const osThreadAttr_t LvglTask_attributes = {
  .name = "LvglTask",
  .stack_size = 1280 * 20,
  .priority = (osPriority_t) osPriorityNormal1,
};


/*******************************************************************************
* User Code Area
*******************************************************************************/


void thread_init(void) {

    netTaskHandle = osThreadNew(Net_Task_BaseOn_FreeRTOS, NULL, &netTask_attributes);
    if (!netTaskHandle) {
        for (;;){}
    }

    LvglTaskHandle = osThreadNew(lvgl_task, NULL, &LvglTask_attributes);
    if (!LvglTaskHandle) {
        for (;;){}
    }

    return;
}


void lvgl_task(void *arg)
{
	lv_init();
	lvgl_adapter_layer_init();
	lv_demo_benchmark();
//	lv_demo_stress();
	for (;;)
	{
		lv_timer_handler();
		osDelay(5);
	}
}


void sdram_speed_test()
{
	static volatile int start_time;
	static volatile int end_time;
	static volatile int final_value;
	uint32_t *test_buffer;

	test_buffer = (uint32_t *)0xC0000000;
    start_time = HAL_GetTick();
    memset(test_buffer, 0xA5A5, 1024*1024*32);
    end_time = HAL_GetTick();
    final_value = end_time - start_time;
//    printf("time :%d", final_value);
    return;
}


void Net_Task_BaseOn_FreeRTOS(void *argumnet) {
    
    int sct = 0; 
    static int res = 0;
    char *dtr ="Hello,word!\r\n";
    struct sockaddr_in dst_ip ;

    sdram_speed_test();
  retry:
    memset(&dst_ip,0,sizeof(struct sockaddr_in));
    while(!netif_is_link_up(&gnetif))
    {
        osDelay(20);
    }
    sct = socket(AF_INET,SOCK_STREAM,0);
    if(sct<0){    
        close(sct);
    }
    
    dst_ip.sin_port = htons(6666);
    dst_ip.sin_family = AF_INET;
    dst_ip.sin_addr.s_addr = inet_addr("192.168.0.198");
    res = connect(sct,(struct sockaddr*)&dst_ip, sizeof(struct sockaddr_in));
    if(res<0){
        closesocket(sct);
        osDelay(500);
        goto retry;
    }
    while (1) {
        res = write(sct, dtr, strlen(dtr));
        if (res == -1) {
            closesocket(sct);
            goto retry;
        }
        osDelay(5);
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

	uint32_t len = ((uint32_t)&_ex_data_excu_end_addr) - \
	               ((uint32_t)&_ex_data_excu_star_addr);

	memcpy((void*)&_ex_data_excu_star_addr, (void*)&_ex_sidata, len);
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
  MPU_InitStruct.Size = MPU_REGION_SIZE_32KB;
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
//  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
//  MPU_InitStruct.BaseAddress = 0xC0000000;
//  MPU_InitStruct.Size = MPU_REGION_SIZE_32MB;
//  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
//  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
//  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
//  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
//  MPU_InitStruct.Number = MPU_REGION_NUMBER4;
//  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
//  MPU_InitStruct.SubRegionDisable = 0x00;
//  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
//
//  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* QSPI-FLASH */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x90000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_16MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER5;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}
