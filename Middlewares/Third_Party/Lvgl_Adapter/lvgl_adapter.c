/*
 *   -- By FreedomLi
 */

#include "lvgl.h"
#include "gt9147.h"
#include "stm32h7xx_hal.h"


#define MY_DISP_HOR_RES    800
#define MY_DISP_VER_RES    480


/*A static or global variable to store the buffers*/
static lv_disp_draw_buf_t disp_buf;

/*Static or global buffer(s). The second buffer is optional*/
//static lv_color_t buf_1[MY_DISP_HOR_RES * 160];
//static lv_color_t buf_2[MY_DISP_HOR_RES * 80];

static lv_disp_drv_t disp_drv_obj;          /*A variable to hold the drivers. Must be static or global.*/

static lv_indev_drv_t indev_drv;

static void my_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
static void my_input_read(lv_indev_drv_t * drv, lv_indev_data_t*data);


void lvgl_adapter_layer_init(void)
{
	/*Initialize `disp_buf` with the buffer(s). With only one buffer use NULL instead buf_2 */
	lv_disp_draw_buf_init(&disp_buf, (void*)0xc0000000, (void*)0xc0200000, MY_DISP_HOR_RES * 480);

	lv_disp_drv_init(&disp_drv_obj);            /*Basic initialization*/
	disp_drv_obj.draw_buf = &disp_buf;          /*Set an initialized buffer*/
	disp_drv_obj.hor_res = MY_DISP_HOR_RES;     /*Set the horizontal resolution in pixels*/
	disp_drv_obj.ver_res = MY_DISP_VER_RES;     /*Set the vertical resolution in pixels*/
	disp_drv_obj.flush_cb = my_flush_cb;
	disp_drv_obj.full_refresh = 1;
	disp_drv_obj.direct_mode = 0;
//	disp_drv_obj.clean_dcache_cb = SCB_InvalidateDCache_by_Addr;

	lv_disp_t * disp;
	disp = lv_disp_drv_register(&disp_drv_obj); /*Register the driver and save the created display objects*/

    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_input_read;
    lv_indev_drv_register(&indev_drv);

    (void)disp;

	return;
}


static void my_flush_cb(lv_disp_drv_t * disp_drv,
                        const lv_area_t * area,
                        lv_color_t * color_p)
{
    extern LTDC_HandleTypeDef hltdc;

    if(lv_disp_flush_is_last(disp_drv)) {
        SCB_CleanInvalidateDCache();
        HAL_LTDC_SetAddress_NoReload(&hltdc, (uint32_t)color_p, 0);
        HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_VERTICAL_BLANKING);
        lv_disp_flush_ready(disp_drv);
    }
}


static void my_input_read(lv_indev_drv_t * drv, lv_indev_data_t*data)
{
    extern _m_tp_dev tp_dev;

    if(tp_dev.sta&(0x01<<15)) {
        data->point.x = tp_dev.x[0];
        data->point.y = tp_dev.y[0];
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else {
        data->point.x = tp_dev.x[4];
        data->point.y = tp_dev.y[4];
        data->state = LV_INDEV_STATE_RELEASED;
    }
}


//extern void DMA2D_Copy(void * pSrc, void * pDst, uint32_t xSize, uint32_t ySize, \
//                       uint32_t OffLineSrc, uint32_t OffLineDst, \
//                       uint32_t PixelFormat);


//static void my_flush_cb(lv_disp_drv_t * disp_drv,
//                        const lv_area_t * area,
//                        lv_color_t * color_p)
//{
//    SCB_CleanInvalidateDCache();
//  DMA2D_Copy(color_p,
//              (void *)(0xC0000000+((area->x1)*2)+((area->y1)*800*2)),
//              (area->x2 - area->x1 + 1),
//              (area->y2 - area->y1 + 1),
//              0,
//              800-(area->x2 - area->x1 + 1),
//              LTDC_PIXEL_FORMAT_RGB565
//              );
//  lv_disp_flush_ready(disp_drv);
//}
