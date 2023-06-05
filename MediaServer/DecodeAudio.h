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

class DecodeAudio
{
public:
	DecodeAudio();
	~DecodeAudio();

public:
	bool InitDecoder(int sampleRate, int channels, AVCodecID codecID);
	bool GetPcmData(AVPacket* pPacket, AVFrame* pFrame);

private:
	int _intSampleRate;
	int _intChannels;
	AVCodecContext* _pCodecCtx;
	AVCodec* _pCodec;
};

