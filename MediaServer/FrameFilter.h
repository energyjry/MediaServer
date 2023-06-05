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
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
}

class FrameFilter
{
public:
	FrameFilter();
	~FrameFilter();

private:
	AVFilterGraph* pFilterGraph;
	AVFilterContext* pBuffersrcCtx;
	AVFilterContext* pBuffersinkCtx;

public:
	bool InitFrameFilter(int videoWidth, int videoHeight, AVPixelFormat videoFmt, const char* filtersDesc);
	bool ConvertFrame(AVFrame* frameSource, AVFrame* frameTarget);
};

