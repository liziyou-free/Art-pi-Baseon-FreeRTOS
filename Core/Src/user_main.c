/*
 *  User Main Source File
 *                   -- By FreedomLi
 *                   -- 2023/02/13
 */
 
 
/*
 *  Include Header file ...
 */
#include "./user_main.h"


/*
 *  Function declaration ...
 */
void lwiperf_example_init(void);
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
const osThreadAttr_t netTask_attributes = {
  .name = "netTask",
  .stack_size = 1280 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
uint8_t buffer[1024*100];




/*************************** User Code Area ***************************/


void components_init(void) {
    
    netTaskHandle = osThreadNew(Net_Task_BaseOn_FreeRTOS, NULL, &netTask_attributes);
    if (!netTaskHandle) {
        goto err;
    }
    return;
  err:
    for(;;)
    {
        osDelay(1);
    }
    
}


void defualt_thread_entry(void){
    
    
    sdram_speed_test();
    lwiperf_example_init();
}


uint32_t *test_buffer = (uint32_t *)0xC0000000;   //[(1024*1024*16)/4] __attribute__((section("sdram0")));

static volatile int start_time = 0, end_time = 0, final_value = 0;

void sdram_speed_test() {
    
    start_time = HAL_GetTick();
    memset(test_buffer, 0xA5A5, 1024*1024*32);
    end_time = HAL_GetTick();
    final_value = end_time - start_time;
    printf("time :%d", final_value);
}


void Net_Task_BaseOn_FreeRTOS(void *argumnet) {
    
    int sct = 0; 
    static int res = 0;
    char *dtr = "Hello,word!@FreedomLi";
    struct sockaddr_in dst_ip ;
  retry:
    memset(&dst_ip,0,sizeof(struct sockaddr_in));
    while(!netif_is_link_up(&gnetif))
    {
        osDelay(100);
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
        osDelay(20);
    }
}


void MPU_Config( void )
{
  MPU_Region_InitTypeDef MPU_InitStruct;
  
  /* Disable the MPU */
  HAL_MPU_Disable();
  
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress      = 0x24000000;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
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

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

