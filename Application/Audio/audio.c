/*
 * audio.c
 *
 *  Created on: Jan 16, 2024
 *      Author: Administrator
 */
#include "main.h"
#include "ff.h"
#include "./audio.h"
#include "cmsis_os2.h"
#include <stdio.h>


#define WAV_HEAD_SIZE()          (sizeof(wav_riff_t) + \
                                  sizeof(wav_fmt_t) + \
                                  sizeof(wav_data_t))

#define FRAMWBUFFER_SIZE     10240

#define AUDIO_SEND(p_buff0, p_buff1, len)  HAL_DMAEx_MultiBufferStart_IT( \
                                               &hdma_sai1_a, \
                                               p_buff0, \
                                               (uint32_t)&(SAI1_Block_A->DR), \
                                               p_buff1, \
                                               len)


static osMessageQueueId_t sai_queue_id;
static const osMessageQueueAttr_t sai_queue_attr = {.name = "sai-semp"};

osMessageQueueId_t instruct_queue_id;
static const osMessageQueueAttr_t instruct_queue_attr = {.name = "instruct-semp"};

static uint8_t audio_buffer0[FRAMWBUFFER_SIZE];
static uint8_t audio_buffer1[FRAMWBUFFER_SIZE];


extern SAI_HandleTypeDef hsai_BlockA1;
extern DMA_HandleTypeDef hdma_sai1_a;

void adjust_volume(uint8_t *scr, uint32_t size, int8_t angle);
void audio_protocol_cfg(uint32_t sample_freq, uint8_t audio_mode, uint8_t bits);


static void dma_TxCplt_m0(DMA_HandleTypeDef *_hdma)
{
    static uint32_t msg = (uint32_t)&audio_buffer0[0];
    osMessageQueuePut(sai_queue_id, &msg, 1, 0);
}

static void dma_TxCplt_m1(DMA_HandleTypeDef *_hdma)
{
    static uint32_t msg = (uint32_t)&audio_buffer1[0];
    osMessageQueuePut(sai_queue_id, &msg, 1, 0);
}


void dump_wav_info(void *p_head, uint32_t *frq, uint8_t *bits, uint8_t *mode)
{
    wav_fmt_t  *p_fmt;

    p_head += sizeof(wav_riff_t);
    p_fmt = (wav_fmt_t *)p_head;
    *frq = p_fmt->sample_rate;
    *bits = p_fmt->bitspersample;
    *mode = p_fmt->channel_count;
    PRINTF_LINE("\r\n----------------------------");
    PRINTF_LINE("Channel Count: %d", p_fmt->channel_count);
    PRINTF_LINE("Sample Rate: %.2fKHz", p_fmt->sample_rate / 1000.0);
    PRINTF_LINE("Bit Rate: %.2fKHz", p_fmt->bit_rate / 1000.0);
    PRINTF_LINE("Per Sample Bits: %dbits", p_fmt->bitspersample);
    PRINTF_LINE("----------------------------");
    return;
}



/*******************************************************************************
 * Audio player (only support WAV)
 ******************************************************************************/

void audio_recorder (void *arg);

void audio_thread(void *arg)
{
    const char *p_audio_file = "0:/music/juhao.wav";    //"0:/music/Recorder.wav";
    uint8_t *p_dst;
    uint8_t  ret;
    uint8_t  retvul;
    uint8_t  volume;
    uint32_t msg;
    uint32_t sample_frq;
    uint8_t  sample_bits;
    uint8_t  audio_mode;
    FIL      file;
    UINT     out;

    file_system_mount();

    audio_recorder(NULL);

    HAL_DMA_RegisterCallback(&hdma_sai1_a, HAL_DMA_XFER_CPLT_CB_ID, dma_TxCplt_m0);
    HAL_DMA_RegisterCallback(&hdma_sai1_a, HAL_DMA_XFER_M1CPLT_CB_ID, dma_TxCplt_m1);

    sai_queue_id = osMessageQueueNew(1, 4, &sai_queue_attr);
    instruct_queue_id = osMessageQueueNew(1, 4, &instruct_queue_attr);

    if (sai_queue_id == NULL || instruct_queue_id == NULL) {
        PRINTF_LINE("File open fail!");
        for (;;);
    }

    ret = f_open(&file, p_audio_file, FA_READ);
    if (ret != FR_OK) {
        PRINTF_LINE("File open fail!");
        for (;;);
    }
    p_dst = audio_buffer0;
    f_read(&file, p_dst, FRAMWBUFFER_SIZE, &out);
    dump_wav_info(p_dst, &sample_frq, &sample_bits, &audio_mode);

    /* First, fill all buffer */
    f_read(&file, &audio_buffer0[0], FRAMWBUFFER_SIZE, &out);
    f_read(&file, &audio_buffer1[0], FRAMWBUFFER_SIZE, &out);

    audio_protocol_cfg(sample_frq, audio_mode, sample_bits);
    AUDIO_SEND((uint32_t)&audio_buffer0[0], (uint32_t)&audio_buffer1[0], out / 2);
    HAL_SAI_DMAResume(&hsai_BlockA1);

    volume = 20;

    do {

        /* Get instructions from other threads */
        retvul = osMessageQueueGet(instruct_queue_id, &msg, NULL, 0);
        if (retvul == osOK) {
            volume = msg;
        }

        /* Get next buffer address */
        retvul = osMessageQueueGet(sai_queue_id, &msg, NULL, osWaitForever);
        if (retvul == osOK) {
            p_dst = (uint8_t *)msg;
        } else {
            PRINTF_LINE("Message Receive failed!");
            break;
        }

        /* Read data from sd card to buffer */
        ret = f_read(&file, p_dst, FRAMWBUFFER_SIZE, &out);
        if (ret == FR_OK && out <= FRAMWBUFFER_SIZE) {
            adjust_volume(p_dst, out, volume);
        } else {
            PRINTF_LINE("File Read failed!");
            break;
        }

        osDelay(5);

    } while(! f_eof(&file));

    /* Stop DMA -> SAI */
    HAL_SAI_DMAStop(&hsai_BlockA1);
    /* Close file */
    f_close(&file);

    for (;;) {

        /* never return */

        osDelay(5);
    }
}


void audio_protocol_cfg(uint32_t sample_freq, uint8_t audio_mode, uint8_t bits)
{
    uint8_t mode;
    uint8_t sample_bits;

    mode = (audio_mode == 2) ? SAI_STEREOMODE : SAI_MONOMODE;

    switch (bits) {
    case 16:
        sample_bits = SAI_PROTOCOL_DATASIZE_16BIT;
        break;
    case 24:
        sample_bits = SAI_PROTOCOL_DATASIZE_24BIT;
        break;
    case 32:
        sample_bits = SAI_PROTOCOL_DATASIZE_32BIT;
        break;
    default:
        for (;;);
        break;
    }

    hsai_BlockA1.Instance = SAI1_Block_A;
    hsai_BlockA1.Init.AudioMode = SAI_MODEMASTER_TX;
    hsai_BlockA1.Init.Synchro = SAI_ASYNCHRONOUS;
    hsai_BlockA1.Init.OutputDrive = SAI_OUTPUTDRIVE_ENABLE;
    hsai_BlockA1.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
    hsai_BlockA1.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_HF;
    hsai_BlockA1.Init.AudioFrequency = sample_freq;
    hsai_BlockA1.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
    hsai_BlockA1.Init.MonoStereoMode = mode;
    hsai_BlockA1.Init.CompandingMode = SAI_NOCOMPANDING;
    hsai_BlockA1.Init.TriState = SAI_OUTPUT_NOTRELEASED;
    if (HAL_SAI_InitProtocol(&hsai_BlockA1, SAI_I2S_STANDARD, sample_bits, 2) != HAL_OK)
    {
      Error_Handler();
    }
}



/*
 *  \brief 音量调节 (仅支持 带符号16位整形PCM数据)
 *    参数1:PCM缓冲区地址
 *    参数2:采样样本字节数量 (16位整形PCM数据一个样本就是16bit/2byte)
 *    参数3:调节裕度(1~89 角度值)
 *
*/
void adjust_volume(uint8_t *scr, uint32_t size, int8_t angle){

    #define PI  (3.1415926)

    int16_t *p_scr = (int16_t *)scr;
    double   temp  = 0;
    double   value = 0;

    if(angle < 0)
    {
        angle = 0; //静音
    }
    else if(angle >= 90)
    {
        angle = 89;
    }
    size = size / 2;
    /*  先将角度制转换为弧度制，再传入tan()       直角坐标系的正半轴上, tan取值区间[0~90) */
    value = tan(angle * (PI / 180) );

    for(uint32_t j=0; j<size; j++)
    {
        temp = (*p_scr) * value;
        /** \brief 限幅处理  */
        if(temp >= 32767)
        {
            *p_scr = 32767;
        }
        else if(temp <= -32767)
        {
            *p_scr = -32767;
        }
        else
        {
            *p_scr = (int16_t)temp;
        }
        p_scr++;
    }
}




/*******************************************************************************
 * Audio Recorder (only support WAV)
 ******************************************************************************/


#define FRAMWBUFFER_SIZE     10240

#define AUDIO_RECEIVE(p_buff0, p_buff1, len)  HAL_DMAEx_MultiBufferStart_IT( \
                                               &hdma_spi2_rx, \
                                               p_buff0, \
                                               (uint32_t)&(SPI2->RXDR), \
                                               p_buff1, \
                                               len)

static osMessageQueueId_t i2s_queue_id;
static const osMessageQueueAttr_t i2s_queue_attr = {.name = "i2s-semp"};

static uint32_t i2s_rx_buffer0[FRAMWBUFFER_SIZE / 4];
static uint32_t i2s_rx_buffer1[FRAMWBUFFER_SIZE / 4];

extern I2S_HandleTypeDef hi2s2;
extern DMA_HandleTypeDef hdma_spi2_rx;


static void dma_RxCplt_m0(DMA_HandleTypeDef *_hdma)
{
    static uint32_t msg = (uint32_t)&i2s_rx_buffer0[0];
    osMessageQueuePut(i2s_queue_id, &msg, 1, 0);
}

static void dma_RxCplt_m1(DMA_HandleTypeDef *_hdma)
{
    static uint32_t msg = (uint32_t)&i2s_rx_buffer1[0];
    osMessageQueuePut(i2s_queue_id, &msg, 1, 0);
}


void audio_recorder (void *arg)
{
    const char *p_audio_file = "0:/music/Recorder.wav";
    FIL       file;
    UINT      out;
    uint8_t   ret;
    uint8_t  *p_dst;
    uint32_t  msg;

    wav_fmt_t fmt = {
        .bit_rate = 192000,
        .bitspersample = 24,
        .block_align = 4,
        .block_length = 1024,
        .channel_count = 2,
        .sample_rate = 48000,
    };
    wav_riff_t riff = {
        .data_length = 1024 * 1024,
    };

    wav_data_t data = {
        .VoiceData_Size = 1024 * 1024,
    };

    HAL_DMA_RegisterCallback(&hdma_spi2_rx, HAL_DMA_XFER_CPLT_CB_ID, dma_RxCplt_m0);
    HAL_DMA_RegisterCallback(&hdma_spi2_rx, HAL_DMA_XFER_M1CPLT_CB_ID, dma_RxCplt_m1);

    i2s_queue_id = osMessageQueueNew(1, 4, &i2s_queue_attr);

    if (i2s_queue_id == NULL) {
        PRINTF_LINE("File open fail!");
        for (;;);
    }

    ret = f_open(&file, p_audio_file, FA_CREATE_ALWAYS | FA_WRITE);
    if (ret != FR_OK) {
        PRINTF_LINE("File open fail!");
        for (;;);
    }

//    AUDIO_RECEIVE((uint32_t)i2s_rx_buffer0, (uint32_t)i2s_rx_buffer1, (FRAMWBUFFER_SIZE / 4));
//    HAL_I2S_DMAPause(&hi2s2);

    HAL_I2S_Receive(&hi2s2, i2s_rx_buffer0, FRAMWBUFFER_SIZE / 4, 500);

    ret = f_write(&file, &riff, sizeof(riff), &out);
    ret |= f_write(&file, &fmt, sizeof(fmt), &out);
    ret |= f_write(&file, &data, sizeof(fmt), &out);
    if (ret != FR_OK) {
        PRINTF_LINE("Write to file fail!");
        for (;;);
    }

    uint32_t cnt = 1000;

    p_dst = i2s_rx_buffer0;

    do {
        /* Get next buffer address */
        ret = osMessageQueueGet(i2s_queue_id, &msg, NULL, osWaitForever);
        if (ret == osOK) {
            p_dst = (uint8_t *)msg;
        }

        ret = f_write(&file, p_dst, (FRAMWBUFFER_SIZE / 4), &out);
        if (ret != FR_OK) {
            PRINTF_LINE("Write to file fail!");
            break;
        }

        f_sync(&file);

    } while(cnt--);

    f_close(&file);

    return;

//    for (;;) osDelay(10);
}






