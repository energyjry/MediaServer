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

class FrameSwr
{
public:
	FrameSwr();
	~FrameSwr();

private:
    SwrContext* pSwrCtx;

    int _srcSampleRate;
    int _srcChannels;
    int _srcNbSamples;
    AVSampleFormat _srcFmt;

    int _targetSampleRate;
    int _targetChannels;
    int _targetNbSamples;
    AVSampleFormat _targetFmt;

    int _targetMaxNbSamples;

    //int _outBufSize;
    //uint8_t** _outBuf;    

    int nTargetlinesize;
    uint8_t** pTargetBuf;

    FILE* _pcmFile;

public:
    bool InitFrameSwr(int srcSampleRate, int srcChannels, AVSampleFormat srcFmt, int srcNbSamples, int targetSampleRate, int targetChannels, AVSampleFormat targetFmt);
    bool ConvertFrame(AVFrame* srcFrame, AVFrame* targetFrame);
};

