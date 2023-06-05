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
    1 . ��Ƶ�ӳ���� : FFMPEG ת��Ĺ����� , ����û��һ���Խ�һ֡���ݴ������ , �������� 20 ������ , һ������� 20 �����ݶ��ܴ������ , ��ʱ�������ֻ������ 19 �� , ʣ��� 1 �����ݾͻ�ѹ���˻������е���� , ������ֻ�ѹ�ڻ������е����ݹ��� , ����ɺܴ����Ƶ�ӳ� , �����ڴ���� ;
    2 . �ӳ����ݴ����� : ÿ����Ƶ����ʱ , �����Խ���һ�λ�ѹ����Ƶ�������ݼ��뵽���δ���������� , ��ֹ������Ƶ�ӳٵ���� ;
    3 . ��ȡ��Ƶ���ݻ�ѹ���� : ���� swr_get_delay ( ) ���� , ���Ի�ȡ��ǰ��ѹ����Ƶ������ , �򲥷��ӳ�ʱ�� ;
    4 . ���ӳٵ���� : swr_get_delay ( ) ��ȡ������һ�ε��������� A ���� �����೤ʱ���ӳٺ� , ���ܽ����� A ���ų��� , ����ӳپ��ǻ�ѹ�����ݵĲ���ʱ�� , ���ÿ�δ���ʱ���ٲ��ֻ�ѹ���ݽ��д��� , ������Ч������Ƶ�ӳ� ;
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
