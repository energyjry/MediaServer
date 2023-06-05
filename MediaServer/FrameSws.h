#pragma once
extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libswresample/swresample.h"
#include "libavutil/pixdesc.h"
#include "libavutil/error.h"
#include "libavutil/imgutils.h"
#include "libavutil/time.h"
}

class FrameSws
{
public:
	FrameSws();
	~FrameSws();
private:
	SwsContext* pSwsCtx;
	int _outBufSize;
	uint8_t* _outBuf;
	
	int _srcWidth;
	int _srcHeight;
	AVPixelFormat _srcFtm;

	int _targetWidth;
	int _targetHeight;
	AVPixelFormat _targetFtm;

	FILE* _yuvFile;

public:
	bool InitFrameSws(int srcWidth, int srcHeight, AVPixelFormat srcFmt, int targetWidth, int targetHeight, AVPixelFormat targetFmt);
	bool ConvertFrame(AVFrame* srcFrame, AVFrame* targetFrame);
};

