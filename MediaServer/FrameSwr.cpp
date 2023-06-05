#include "FrameSwr.h"
#include <iostream>

FrameSwr::FrameSwr()
{
    pTargetBuf = nullptr;
    _pcmFile = nullptr;
    pSwrCtx = nullptr;
}

FrameSwr::~FrameSwr()
{
    if (pTargetBuf != nullptr) {
        av_freep(&pTargetBuf[0]);
    }
    if (_pcmFile != nullptr) {
        fclose(_pcmFile);
    }
    if (pSwrCtx != nullptr) {
        swr_free(&pSwrCtx);
    }

}

bool FrameSwr::InitFrameSwr(int srcSampleRate, int srcChannels, AVSampleFormat srcFmt, int srcNbSamples, int targetSampleRate, int targetChannels, AVSampleFormat targetFmt)
{
    int ret = 0;
    _srcSampleRate = srcSampleRate;
    _srcChannels = srcChannels;
    _srcNbSamples = srcNbSamples;
    _srcFmt = srcFmt;

    _targetSampleRate = targetSampleRate;
    _targetChannels = targetChannels;
    _targetFmt = targetFmt;

    pSwrCtx = swr_alloc();
    if (pSwrCtx == NULL) {
        printf("swr_alloc failed!\n");
        return false;
    }
    swr_alloc_set_opts(
        pSwrCtx,
        av_get_default_channel_layout(_targetChannels),
        _targetFmt,
        _targetChannels,
        av_get_default_channel_layout(_srcChannels),
        _srcFmt,
        _srcSampleRate,
        0, nullptr
    );
    ret = swr_init(pSwrCtx);
    if (ret < 0)
    {
        printf("swr_init failed!\n");
        return false;
    }

    _targetMaxNbSamples = _targetNbSamples = av_rescale_rnd(_srcNbSamples, _targetSampleRate, _srcSampleRate, AV_ROUND_UP);
    ret = av_samples_alloc_array_and_samples(&pTargetBuf, &nTargetlinesize, _targetChannels, _targetNbSamples, _targetFmt, 0);
    if (ret < 0)
    {
        printf("av_samples_alloc_array_and_samples failed\n");
        return false;
    }

    _pcmFile = fopen("rtsp.pcm", "wb");
	return true;
}

bool FrameSwr::ConvertFrame(AVFrame* srcFrame, AVFrame* targetFrame)
{
    int ret = 0;
    int64_t delay = swr_get_delay(pSwrCtx, _srcSampleRate);
    /*
    1 . 音频延迟情况 : FFMPEG 转码的过程中 , 可能没有一次性将一帧数据处理完毕 , 如输入了 20 个数据 , 一般情况下 20 个数据都能处理完毕 , 有时还会出现只处理了 19 个 , 剩余的 1 个数据就积压在了缓冲区中的情况 , 如果这种积压在缓冲区中的数据过大 , 会造成很大的音频延迟 , 甚至内存崩溃 ;
    2 . 延迟数据处理方案 : 每次音频处理时 , 都尝试将上一次积压的音频采样数据加入到本次处理的数据中 , 防止出现音频延迟的情况 ;
    3 . 获取音频数据积压个数 : 调用 swr_get_delay ( ) 方法 , 可以获取当前积压的音频采样数 , 或播放延迟时间 ;
    4 . 对延迟的理解 : swr_get_delay ( ) 获取的是下一次的样本数据 A 输入 经过多长时间延迟后 , 才能将样本 A 播放出来 , 这个延迟就是积压的数据的播放时间 , 因此每次处理时将少部分积压数据进行处理 , 可以有效降低音频延迟 ;
    */
    _targetNbSamples = av_rescale_rnd(delay + _srcNbSamples, _targetSampleRate, _srcSampleRate, AV_ROUND_UP);
    if (_targetNbSamples > _targetMaxNbSamples)
    {
        std::cout << "&&&&&&&& " << _targetNbSamples << "\t" << _targetMaxNbSamples << std::endl;
        av_freep(&pTargetBuf[0]);
        ret = av_samples_alloc(pTargetBuf, &nTargetlinesize, _targetChannels, _targetNbSamples, _targetFmt, 1);
        if (ret < 0)
            return false;
        _targetMaxNbSamples = _targetNbSamples;
    }

    ret = swr_convert(pSwrCtx, pTargetBuf, _targetNbSamples, (const uint8_t**)srcFrame->data, srcFrame->nb_samples);
    if (ret < 0)
    {
        printf("swr_convert failed\n");
        return false;
    }
    int dataSize = av_samples_get_buffer_size(&nTargetlinesize, _targetChannels, ret, _targetFmt, 1);
    fwrite(pTargetBuf, 1, dataSize, _pcmFile);
    //targetFrame->nb_samples = _targetNbSamples;
    //memcpy(targetFrame->data, _outBuf[0], dst_bufsize);
    //targetFrame->data = 
    //dataBuf = (char*)pTargetBuf[0];
    //dataSize = dst_bufsize;
    //fwrite(dataBuf, 1, dataSize, mPcmFile);

    //int dataSize = targetFrame->channels * nb * av_get_bytes_per_sample(_targetFmt);
    //fwrite(targetFrame->data[0], 1, dataSize, _pcmFile);
    return true;
}
