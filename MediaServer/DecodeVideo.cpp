#include "DecodeVideo.h"

DecodeVideo::DecodeVideo()
{
    _intWidth = 0;
    _intHeight = 0;
    _pCodecCtx = nullptr;
    _pCodec = nullptr;
}

DecodeVideo::~DecodeVideo()
{
    if (_pCodecCtx != nullptr)
    {
        avcodec_close(_pCodecCtx);
    }
}

bool DecodeVideo::InitDecoder(int width, int height, AVCodecID codecID)
{
    _intWidth = width;
    _intHeight = height;
    _pCodec = avcodec_find_decoder(codecID);
    if (!_pCodec) {
        printf("Codec not found decoder:%d!\n", codecID);
        return false;
    }
    _pCodecCtx = avcodec_alloc_context3(_pCodec);
    if (!_pCodecCtx) {
        printf("avcodec_alloc_context3 error!\n");
        return false;
    }
    if (avcodec_open2(_pCodecCtx, _pCodec, NULL) < 0) {
        printf("Could not open H264 codec!\n");
        return false;
    }

   
    return true;
}

bool DecodeVideo::GetYuvData(AVPacket* pPacket, AVFrame* pFrame)
{
    int ret = 0;
    ret = avcodec_send_packet(_pCodecCtx, pPacket);
    if (ret < 0)
    {
        printf("avcodec_send_packet error!\n");
        return false;
    }
    av_packet_unref(pPacket);
    ret = avcodec_receive_frame(_pCodecCtx, pFrame);
    if (ret < 0)
    {
        printf("avcodec_receive_frame error!\n");
        return false;
    }
    return true;
}
