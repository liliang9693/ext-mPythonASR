#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_log.h"
#include "FS.h"
#include "SPIFFS.h"
#include "MPython_ASR.h"
#include "audio_renderer.h"
#include "driver/i2s.h"
#include "driver/adc.h"
#include "wav_head.h"
#include <ArduinoJson.h>

#define REC_BLOCK_LEN (4096)
#define REC_BLOCKS_PER_SECOND (4)

int  record_time;
bool record_flag = false;
static xTaskHandle  xRecordSound = NULL;
static recorder_t *recorder_instance = NULL;
String filename = "test.wav";
String FFS_filename = "/" + filename;
uint8_t buff[3072];

void example_disp_buf(uint8_t* buf, int length)
{
    //printf("======\n");
    for (int i = 0; i < length; i++) {
        printf("%02x ", buf[i]);
        if ((i + 1) % 8 == 0) {
            //printf("\n");
        }
    }
}

inline void adc_data_scale( uint8_t* s_buff, uint32_t len)
{
    uint32_t j = 0;
    uint32_t dac_value;

    for (int i = 0; i < len; i += 2) {
        dac_value = ((((uint16_t) (s_buff[i + 1] & 0xf) << 8) | ((s_buff[i + 0]))));
        dac_value = (dac_value - 2048 - 300) << 4; 
        s_buff[j++] = dac_value & 0xff;
        s_buff[j++] = (dac_value >> 8) & 0xff;
    }
}


static void i2s_adc_data_scale1(uint16_t * d_buff, uint8_t* s_buff, uint32_t len)
{
    uint32_t j = 0;
    uint16_t adc_val;
    renderer_config_t *renderer = renderer_get();

    if(renderer->bit_depth == I2S_BITS_PER_SAMPLE_16BIT) 
    {
        for (int i = 0; i < len; i += 2) {
            adc_val = ((((uint16_t) (s_buff[i + 1] & 0xf) << 8) | ((s_buff[i + 0]))));
            d_buff[j++] = adc_val;
        }
    }
}

int cmpfunc (const void * a, const void * b)
{
   return ( *(int*)a - *(int*)b );
}

void MPython_ASR::recorderInit()
{
    if(!recorder_instance){
        recorder_instance = (recorder_t*)calloc(1, sizeof(recorder_t));
    }
    renderer_config_t renderer_config;
    renderer_config.bit_depth  = I2S_BITS_PER_SAMPLE_16BIT;
    renderer_config.adc1_channel = ADC1_CHANNEL_2;
    renderer_config.channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT;
    renderer_config.sample_rate = 8000;
    renderer_config.mode = I2S_MODE_RX;    
    renderer_config.use_apll = true; 
    renderer_config.i2s_channal_nums = 1;
    renderer_init(&renderer_config); 
    renderer_start();
    if(!SPIFFS.begin(true)){
        Serial.println("SPIFFS Mount Failed");
        return;
    }
}

void recordSound(void *pvParameters)
{
    if (recorder_instance)
    {

        renderer_config_t *renderer = renderer_get();
        File F = SPIFFS.open(FFS_filename,FILE_WRITE); 
        if(!F){
            printf("unable to create file \n");
            return;
        }
        int write_bytes;
        if ( record_time > 4)
        {
           // Serial.println(NULL, "Record time too long, will limit to 4s.");
            record_time = 4;
        }
        uint8_t blocks = record_time * REC_BLOCKS_PER_SECOND;
        uint32_t data_len = REC_BLOCK_LEN * blocks;
        wav_header_t *wav_header = (wav_header_t*)calloc(1, sizeof(wav_header_t));
        if (!wav_header)
            Serial.println("Can not alloc enough memory to make wav head.");
        wav_head_init(wav_header, renderer->sample_rate, renderer->bit_depth, renderer->i2s_channal_nums, data_len);
        F.write((uint8_t *)&wav_header->riff.ChunkID,sizeof(wav_header_t));
        free(wav_header);
        uint8_t *read_buff[blocks];
        for (int i = 0; i < blocks; i++){
            read_buff[i] = (uint8_t *)calloc(1, REC_BLOCK_LEN);
            if (!read_buff[i])
                Serial.print("Can not alloc enough memory to record.");
        }
        renderer_adc_enable();
        for(int i = 0; i < blocks; i++){
            renderer_read_raw(read_buff[i], REC_BLOCK_LEN);
            adc_data_scale(read_buff[i], REC_BLOCK_LEN);
        }
        for(int i = 0; i < blocks; i++)
            F.write(read_buff[i], REC_BLOCK_LEN);
        renderer_adc_disable();
        for(int i = 0; i < blocks; i++)
        {
            if(read_buff[i])
                free(read_buff[i]); 
        }
        F.close();
    }
    else{
        Serial.println(NULL, "Please init recorder first.");
    }

    record_flag = true;
	vTaskDelete(xRecordSound);
}

String MPython_ASR::getAsrResult(int time)
{
    recorderInit();
    record_time =  time;
    delay(300);
    xTaskCreate(recordSound, "recordSound", 4096, NULL, 5,&xRecordSound);
    for(int i=0;i<300;i++){
        delay(100);
        if(record_flag){
            record_flag = false;
            break;
        }
    }
    delay(1000);
    recorderDeinit();
    return http_getresult();
}

void MPython_ASR::recorderDeinit()
{
    if (recorder_instance){
        free(recorder_instance);
        recorder_instance = NULL;
    }

    renderer_deinit();
}

void MPython_ASR::setXunFeiId(String id,String secret,String key){
        APPID=id;
        APISecret=secret;
        APIKey=key;
        xunfei_flag=2;
}

String MPython_ASR::http_getresult(){
    WiFiClient client;
    if (!client.connect("119.23.66.134", 8085)) {
        return "http_getresult :connection failed";
    }
    File F = SPIFFS.open(FFS_filename,FILE_READ);
    if(F == NULL){
        return "http_getresult :open filed failed";
    }
    int LEN = F.size();
    String data_length = "Content-Length: " ;
    String boundary            = "----WebKitFormBoundary0x8f48";
    String content_disposition = "Content-Disposition: form-data; name=\"file\"; filename=\"temp.wav\"";
    String content_type        = "Content-Type: audio/wav";
    int content_length = boundary.length()+boundary.length()+content_disposition.length()+content_type.length()+LEN+16;
    data_length +=content_length;
    data_length +="\r\n";

    if(xunfei_flag==2){
        client.print("POST /xunfei_iat?APPID="+APPID+"&APISecret="+APISecret+"&APIKey="+APIKey+"&mediatype=2 HTTP/1.0\r\n");
    }else{
        client.print("POST /file_upload?deviceid=246F2843EBBC&appid=1&mediatype=2 HTTP/1.0\r\n");
    }
    
    client.print(data_length);
    client.print("Content-Type: multipart/form-data; boundary=----WebKitFormBoundary0x8f48\r\n");
    client.print("Host: 119.23.66.134\r\n");
    client.print("Connection: keep-alives\r\n");
    client.print("\r\n");
    client.print("------WebKitFormBoundary0x8f48\r\n");
    client.print("Content-Disposition: form-data; name=\"file\"; filename=\"temp.wav\"\r\n");
    client.print("Content-Type: audio/wav\r\n");
    client.print("\r\n");
    for(int i=0;i<LEN/(3072);i++){
    F.read(buff,3072);
        client.write(buff,3072);
    }
    F.read(buff,LEN%3072);
    client.write(buff,LEN%3072);
    F.close();
    client.print("\r\n------WebKitFormBoundary0x8f48--\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            client.stop();
            return ">>> Client Timeout !";
        }
    }
    while(client.available()) {
        String line = client.readStringUntil('\r');
        if(strstr(line.c_str(),"{") && strstr(line.c_str(),"}")){
		    DynamicJsonBuffer jsonBuffer; 
			JsonObject& data = jsonBuffer.parseObject(line);
			String text = data["text"].as<String>();
			String Code = data["Code"].as<String>();
	        if(text!=NULL){
                return text;
			}else if(Code!=NULL){
                return Code;
			}else{
				return "识别失败，请重试";
			}  
        }
    }
    return "data error";
}
