#ifndef INCLUDE_MPYTHON_ASR_H_
#define INCLUDE_MPYTHON_ASR_H_

#include <arduino.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <WiFi.h>
#include "string.h"

#include <sys/types.h>
#include "audio_renderer.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "vfs_api.h"

typedef enum
{
    MIME_UNKNOWN = 1, OCTET_STREAM, AUDIO_PCM, AUDIO_WAV, AUDIO_AAC, AUDIO_MP4, AUDIO_MPEG, AUDIO_TEXT
} content_type_t;

typedef struct {
    TimerHandle_t recorder_timer;
    bool timer_expire;
    const char *file_name;
    uint32_t file_size;
    FILE file;
    uint32_t recorde_time;
    uint32_t record_buff_size;
    bool save_to_file;
    bool iat_config_flag;
    bool iat_record_finish;
    content_type_t content_type;
    char *url;
} recorder_t;

class MPython_ASR {
public :
        void recorderInit();
        void recorderDeinit();
        String getAsrResult(int time);
private:
        String http_getresult();
        uint32_t recorder_loudness();
        void audio_recorder_quicksort(uint16_t *buff, uint32_t maxlen, uint32_t begin, uint32_t end);
};
#endif
