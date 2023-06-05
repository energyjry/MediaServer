#include "DecodeAudio.h"

DecodeAudio::DecodeAudio()
{
    _intSampleRate = 0;
    _intChannels = 0;
    _pCodecCtx = nullptr;
    _pCodec = nullptr;
}

DecodeAudio::~DecodeAudio()
{
    if (_pCodecCtx != nullptr)
    {
        avcodec_close(_pCodecCtx);
    }
}

bool DecodeAudio::InitDecoder(int sampleRate, int channels, AVCodecID codecID)
{
    _intSampleRate = sampleRate;
    _intChannels = channels;

    _pCodec = avcodec_find_decoder(codecID);
    if (_pCodec == NULL)
    {
        printf("avcodec_find_decoder failed \n");
        return false;
    }

    _pCodecCtx = avcodec_alloc_context3(_pCodec);
    if (_pCodecCtx == NULL)
    {
        printf("avcodec_alloc_context3 failed \n");
        return false;
    }

    
    _pCodecCtx->codec_id = AV_CODEC_ID_AAC;
    _pCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
    _pCodecCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;//AV_SAMPLE_FMT_FLTP
    _pCodecCtx->sample_rate = _intSampleRate;
    _pCodecCtx->frame_size = 1024;
    _pCodecCtx->channels = _intChannels;
    _pCodecCtx->channel_layout = av_get_default_channel_layout(_pCodecCtx->channels);
    _pCodecCtx->bit_rate = 128000;
    

    if (avcodec_open2(_pCodecCtx, _pCodec, NULL) < 0)
    {
        printf("avcodec_open2 failed!\n");
        return false;
    }

    return true;
}

bool DecodeAudio::GetPcmData(AVPacket* pPacket, AVFrame* pFrame)
{
    int ret = 0;

    ret = avcodec_send_packet(_pCodecCtx, pPacket);
    if (ret < 0)
    {
        printf("avcodec_send_packet error!\n");
        av_packet_unref(pPacket);
        return false;
    }

    ret = avcodec_receive_frame(_pCodecCtx, pFrame);
    av_packet_unref(pPacket);
    if (ret < 0)
    {
        printf("avcodec_receive_frame error!\n");
        return false;
    }

    return true;
}
