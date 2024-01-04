/*
 *   -- By FreedomLi
 */

#include "lvgl.h"
#include "stm32h7xx_hal.h"


#define MY_DISP_HOR_RES    800
#define MY_DISP_VER_RES    480


/*A static or global variable to store the buffers*/
static lv_disp_draw_buf_t disp_buf;

/*Static or global buffer(s). The second buffer is optional*/
static lv_color_t buf_1[MY_DISP_HOR_RES * 50];
static lv_color_t buf_2[MY_DISP_HOR_RES * 50];

static lv_disp_drv_t disp_drv_obj;          /*A variable to hold the drivers. Must be static or global.*/


static void my_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);


void lvgl_adapter_layer_init(void)
{
	/*Initialize `disp_buf` with the buffer(s). With only one buffer use NULL instead buf_2 */
	lv_disp_draw_buf_init(&disp_buf, buf_1, buf_2, MY_DISP_HOR_RES*50);

	lv_disp_drv_init(&disp_drv_obj);            /*Basic initialization*/
	disp_drv_obj.draw_buf = &disp_buf;          /*Set an initialized buffer*/
	disp_drv_obj.hor_res = MY_DISP_HOR_RES;     /*Set the horizontal resolution in pixels*/
	disp_drv_obj.ver_res = MY_DISP_VER_RES;     /*Set the vertical resolution in pixels*/
	disp_drv_obj.flush_cb = my_flush_cb;
//	disp_drv_obj.clean_dcache_cb = SCB_InvalidateDCache_by_Addr;

	lv_disp_t * disp;
	disp = lv_disp_drv_register(&disp_drv_obj); /*Register the driver and save the created display objects*/
	return;
}



extern void DMA2D_Copy(void * pSrc,
                       void * pDst,
                       uint32_t xSize,
                       uint32_t ySize,
                       uint32_t OffLineSrc,
                       uint32_t OffLineDst,
                       uint32_t PixelFormat);


static void my_flush_cb(lv_disp_drv_t * disp_drv,
                        const lv_area_t * area,
                        lv_color_t * color_p)
{
    SCB_CleanInvalidateDCache();
	DMA2D_Copy(color_p,
              (0xC0000000+((area->x1)*2)+((area->y1)*800*2)),
              (area->x2 - area->x1 + 1),
              (area->y2 - area->y1 + 1),
              0,
              800-(area->x2 - area->x1 + 1),
              LTDC_PIXEL_FORMAT_RGB565
              );
//	LCD_Draw_Rectangle(area->x1,area->y1,area->x2,area->y2,color_p);
//
	lv_disp_flush_ready(disp_drv);
}
