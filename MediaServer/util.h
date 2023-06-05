#pragma once
extern "C" {
#include <libavformat/avformat.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/pixdesc.h>
#include <libavutil/error.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavutil/timestamp.h>
#include <libavdevice/avdevice.h>
#include "libavutil/time.h"
};


static AVPacket* AllocAvPacket() {
	AVPacket*  pPacket = av_packet_alloc();
	if (!pPacket)
	{
		printf("Could not open H264 codec!\n");
		return nullptr;
	}
	return pPacket;
}

static bool GroupAvPacket(unsigned char* data, int size, AVPacket* pPacket) {
	if (pPacket == nullptr) {
		return false;
	}
	pPacket->size = size;
	pPacket->data = (uint8_t*)av_malloc(pPacket->size);
	memcpy(pPacket->data, data, size);
	int ret = av_packet_from_data(pPacket, pPacket->data, pPacket->size);
	if (ret < 0){
		printf("av_packet_from_data error!\n");
		av_free(pPacket->data);
		return false;
	}
	return true;
}

static void FreeAvPacket(AVPacket* pPacket) {
	if (pPacket != nullptr) {
		av_packet_free(&pPacket);
	}
}

static AVFrame* AllocAvFrameVideo(int width, int height, AVPixelFormat fmt) {
	AVFrame* vidoeFrame = av_frame_alloc();
	vidoeFrame->width = width;
	vidoeFrame->height = height;
	vidoeFrame->format = fmt;
	if (av_frame_get_buffer(vidoeFrame, 1) < 0)
	{
		printf("Failed to get Frame Buffer!\n");
		av_frame_free(&vidoeFrame);
		return nullptr;
	}
	return vidoeFrame;
}

static AVFrame* AllocAvFrameAudio(int sampleRate, int channels, int nbSamples, AVSampleFormat fmt) {
	AVFrame* audioFrame = av_frame_alloc();
	audioFrame->sample_rate = sampleRate;
	audioFrame->channels = channels;
	audioFrame->format = fmt;
	audioFrame->channel_layout = av_get_default_channel_layout(channels);
	audioFrame->nb_samples = nbSamples;
	
	if (av_frame_get_buffer(audioFrame, 0) < 0)
	{
		return nullptr;
	}
	return audioFrame;
}

static void FreeAvFrame(AVFrame* pFrame) {
	if (pFrame != nullptr) {
		av_frame_free(&pFrame);
	}
}

static void get_adts_header(AVCodecContext* ctx, uint8_t* adts_header, int aac_length) {
	uint8_t freq_idx = 0;    //0: 96000 Hz  3: 48000 Hz 4: 44100 Hz
	switch (ctx->sample_rate) {
	case 96000: freq_idx = 0; break;
	case 88200: freq_idx = 1; break;
	case 64000: freq_idx = 2; break;
	case 48000: freq_idx = 3; break;
	case 44100: freq_idx = 4; break;
	case 32000: freq_idx = 5; break;
	case 24000: freq_idx = 6; break;
	case 22050: freq_idx = 7; break;
	case 16000: freq_idx = 8; break;
	case 12000: freq_idx = 9; break;
	case 11025: freq_idx = 10; break;
	case 8000: freq_idx = 11; break;
	case 7350: freq_idx = 12; break;
	default: freq_idx = 4; break;
	}
	uint8_t chanCfg = ctx->channels;
	uint32_t frame_length = aac_length + 7;
	adts_header[0] = 0xFF;
	adts_header[1] = 0xF1;
	adts_header[2] = ((ctx->profile) << 6) + (freq_idx << 2) + (chanCfg >> 2);
	adts_header[3] = (((chanCfg & 3) << 6) + (frame_length >> 11));
	adts_header[4] = ((frame_length & 0x7FF) >> 3);
	adts_header[5] = (((frame_length & 7) << 5) + 0x1F);
	adts_header[6] = 0xFC;
}