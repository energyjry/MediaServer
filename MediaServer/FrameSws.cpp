#include "FrameSws.h"

FrameSws::FrameSws()
{
    _outBuf = nullptr;
    _yuvFile = nullptr;
    pSwsCtx = nullptr;
}

FrameSws::~FrameSws()
{
    if (_outBuf != nullptr) {
        av_free(_outBuf);
    }
    if (_yuvFile != nullptr) {
        fclose(_yuvFile);
    }
    if (pSwsCtx != nullptr) {
        sws_freeContext(pSwsCtx);
    }
}

bool FrameSws::InitFrameSws(int srcWidth, int srcHeight, AVPixelFormat srcFmt, int targetWidth, int targetHeight, AVPixelFormat targetFmt)
{
    _srcWidth = srcWidth;
    _srcHeight = srcHeight;
    _srcFtm = srcFmt;

    _targetWidth = targetWidth;
    _targetHeight = targetHeight;
    _targetFtm = targetFmt;

    pSwsCtx = sws_getContext(_srcWidth, _srcHeight, _srcFtm, _targetWidth, _targetHeight, _targetFtm, SWS_BICUBIC, NULL, NULL, NULL);
    if (!pSwsCtx)
    {
        printf("sws_getContext failed\n");
        return false;
    }

    _outBufSize = av_image_get_buffer_size(_targetFtm, _targetWidth, _targetHeight, 1);
    _outBuf = (uint8_t*)av_malloc(_outBufSize);

    /*
    _yuvFile = fopen("rtsp.yuv", "wb");
    if (_yuvFile == nullptr)
    {
        printf("open %s failed!\n", "rtsp.yuv");
        return false;
    }
    */
	return true;
}

bool FrameSws::ConvertFrame(AVFrame* srcFrame, AVFrame* targetFrame)
{
    if (av_image_fill_arrays(targetFrame->data, targetFrame->linesize, _outBuf, _targetFtm, _targetWidth, _targetHeight, 1) < 0)
    {
        printf("av_image_fill_arrays failed\n");
        return false;
    }
    if (sws_scale(pSwsCtx, srcFrame->data, srcFrame->linesize, 0, _srcHeight, targetFrame->data, targetFrame->linesize) < 0)
    {
        printf("sws_scale failed\n");
        return false;
    }
    //fwrite((char*)_outBuf, 1, _outBufSize, _yuvFile);
	return true;
}
