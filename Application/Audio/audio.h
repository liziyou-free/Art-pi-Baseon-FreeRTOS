/*
 * audio.h
 *
 *  Created on: Jan 16, 2024
 *      Author: Administrator
 */

#ifndef AUDIO_AUDIO_H_
#define AUDIO_AUDIO_H_


typedef struct {
  uint8_t file_flag[4];
  uint32_t data_length;
  uint8_t file_type[4];
}wav_riff_t;

typedef struct {
    uint8_t file_flag[4];
    uint32_t block_length;
    uint16_t coding_num;
    uint16_t channel_count;
    uint32_t sample_rate;
    uint32_t bit_rate;
    uint16_t block_align;
    uint16_t bitspersample;
}wav_fmt_t;

typedef struct {
    uint8_t  String_buf[4];
    uint32_t VoiceData_Size;
}wav_data_t;



#define SEND_JUMP_MESG_TO_AUDIO(progress)




#endif /* AUDIO_AUDIO_H_ */
