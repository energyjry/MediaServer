#include "DeviceRtsp.h"

DeviceRtsp::DeviceRtsp()
{
    pVideoCodecCtx = nullptr;
    pAudioCodecCtx = nullptr;
    pFormatCtx = nullptr;

    nVideoStreamIndex = -1;
    nAudioStreamIndex = -1;
}

DeviceRtsp::~DeviceRtsp()
{
    if (pVideoCodecCtx != nullptr)
    {
        avcodec_close(pVideoCodecCtx);
    }
    if (pAudioCodecCtx != nullptr)
    {
        avcodec_close(pAudioCodecCtx);
    }
    if (pFormatCtx != nullptr)
    {
        avformat_close_input(&pFormatCtx);
    }
}

bool DeviceRtsp::InitDevice(string app, string stream, string strParam)
{
    SetDeviceInfo(app, stream);
    std::cout << "Rtsp" << std::endl;
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();

    AVDictionary* options = nullptr;
    av_dict_set(&options, "buffer_size", "1024000", 0);
    av_dict_set(&options, "max_delay", "500000", 0);
    av_dict_set(&options, "stimeout", "3000000", 0);
    av_dict_set(&options, "rtsp_transport", "tcp", 0);

    if (avformat_open_input(&pFormatCtx, strParam.c_str(), nullptr, &options) != 0)
    {
        printf("Can not open video:%s", strParam.c_str());
        return false;
    }
    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0)
    {
        printf("Can not find video stream info");
        return false;
    }

    for (int i = 0; i < int(pFormatCtx->nb_streams); i++)
    {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            nVideoStreamIndex = i;
            printf("video_stream_idx=%d\n", nVideoStreamIndex);
            pVideoCodecCtx = avcodec_alloc_context3(nullptr);
            avcodec_parameters_to_context(pVideoCodecCtx, pFormatCtx->streams[nVideoStreamIndex]->codecpar);
            int videoWidth = pFormatCtx->streams[nVideoStreamIndex]->codecpar->width;
            int videoHeight = pFormatCtx->streams[nVideoStreamIndex]->codecpar->height;
            int videoFrameRate = pFormatCtx->streams[nVideoStreamIndex]->avg_frame_rate.num / pFormatCtx->streams[nVideoStreamIndex]->avg_frame_rate.den;
            AVCodecID videoCodecId = pFormatCtx->streams[nVideoStreamIndex]->codecpar->codec_id;
            AVPixelFormat videoPixFmt = AVPixelFormat(pFormatCtx->streams[nVideoStreamIndex]->codecpar->format);
            printf("videoInfo:index=%d;width=%d;height=%d;frameRate=%d;codecId:%d;format=%d\n", nVideoStreamIndex, videoWidth, videoHeight, videoFrameRate, videoCodecId, videoPixFmt);
            InitVideoInfo(videoWidth, videoHeight, videoFrameRate, videoCodecId, videoPixFmt);
        }
        else if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            nAudioStreamIndex = i;
            printf("audio_stream_idx=%d\n", nAudioStreamIndex);
            pAudioCodecCtx = avcodec_alloc_context3(nullptr);
            avcodec_parameters_to_context(pAudioCodecCtx, pFormatCtx->streams[nAudioStreamIndex]->codecpar);
            int audiomSampleRate = pFormatCtx->streams[nAudioStreamIndex]->codecpar->sample_rate;
            int audioChannels = pFormatCtx->streams[nAudioStreamIndex]->codecpar->channels;
            int audioNbSamples = pFormatCtx->streams[nAudioStreamIndex]->codecpar->frame_size;
            AVCodecID audioCodecId = pFormatCtx->streams[nAudioStreamIndex]->codecpar->codec_id;
            AVSampleFormat nAudioPixFmt = AVSampleFormat(pFormatCtx->streams[nAudioStreamIndex]->codecpar->format);
            printf("audioInfo:index=%d;sampleRate=%d;channels=%d;nbSamples=%d;codecId:%d;format=%d\n", nAudioStreamIndex, audiomSampleRate, audioChannels, audioNbSamples, audioCodecId, nAudioPixFmt);
            InitAudioInfo(audiomSampleRate, audioChannels, audioNbSamples, audioCodecId, nAudioPixFmt);
        }
    }

    if (nVideoStreamIndex != -1)
    {
        AVCodec* pVideoCodec = avcodec_find_decoder(pVideoCodecCtx->codec_id);
        if (pVideoCodec == nullptr)
        {
            printf("can not find video decoder\n");
            return false;
        }
        if (avcodec_open2(pVideoCodecCtx, pVideoCodec, nullptr) < 0)
        {
            printf("Video decoder can not open\n");
            return false;
        }
    }

    if (nAudioStreamIndex != -1) {
        AVCodec* pAudioCodec = avcodec_find_decoder(pAudioCodecCtx->codec_id);
        if (pAudioCodec == nullptr)
        {
            printf("can not find video decoder\n");
            return false;
        }
        if (avcodec_open2(pAudioCodecCtx, pAudioCodec, nullptr) < 0)
        {
            printf("Video decoder can not open\n");
            return false;
        }
    }

    Start();

    return true;
}

void DeviceRtsp::Process()
{
    AVPacket* stream_packet = av_packet_alloc();
    while (1)
    {
        stream_packet->flags = 0;
        if (av_read_frame(pFormatCtx, stream_packet) >= 0)
        {
            if (stream_packet->stream_index == nVideoStreamIndex)
            {
                PutVideoFrame(stream_packet->data, stream_packet->size);
            }
            else if (stream_packet->stream_index == nAudioStreamIndex)
            {
                uint8_t aac_header[7];
                get_adts_header(pAudioCodecCtx, aac_header, stream_packet->size);
                int tempLen = stream_packet->size + 7;
                unsigned char* tempBuf = new unsigned char[tempLen];
                memcpy(tempBuf, aac_header, 7);
                memcpy(tempBuf + 7, stream_packet->data, stream_packet->size);
                PutAudioFrame(tempBuf, tempLen);
                delete[]tempBuf;
            }
            av_packet_unref(stream_packet);
        }
        else {
            break;
        }
        av_usleep(1000 * 10);
    }
    av_packet_free(&stream_packet);
}
