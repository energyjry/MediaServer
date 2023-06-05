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

class DecodeVideo
{
public:
	DecodeVideo();
	~DecodeVideo();

public:
	bool InitDecoder(int width, int height, AVCodecID codecID);
	bool GetYuvData(AVPacket* pPacket, AVFrame* pFrame);

private:
	int _intWidth;
	int _intHeight;
	AVCodecContext* _pCodecCtx;
	AVCodec* _pCodec;
};

