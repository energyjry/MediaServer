#include "DeviceMp4.h"

DeviceMp4::DeviceMp4()
{
	pVideoCodecCtx = nullptr;
	pAudioCodecCtx = nullptr;
	pFormatCtx = nullptr;
	pBsfCtx = nullptr;
	nVideoStreamIndex = -1;
	nAudioStreamIndex = -1;
	i64VidoeStime = 0;
	i64AudioStime = 0;
}

DeviceMp4::~DeviceMp4()
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
	if (pBsfCtx != nullptr)
	{
		av_bsf_free(&pBsfCtx);
	}
}

bool DeviceMp4::InitDevice(string app, string stream, string strParam)
{
	SetDeviceInfo(app, stream);
	std::cout << "mp4" << std::endl;
	pFormatCtx = avformat_alloc_context();

	if (avformat_open_input(&pFormatCtx, strParam.c_str(), nullptr, nullptr) != 0)
	{
		printf("Can not open video:%s", strParam.c_str());
		return false;
	}
	if (avformat_find_stream_info(pFormatCtx, nullptr) < 0)
	{
		printf("Can not find video stream info");
		return false;
	}

	for (int i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			nVideoStreamIndex = i;
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

	const AVBitStreamFilter* bsfilter = av_bsf_get_by_name("h264_mp4toannexb");
	av_bsf_alloc(bsfilter, &pBsfCtx); //AVBSFContext;
	avcodec_parameters_copy(pBsfCtx->par_in, pFormatCtx->streams[nVideoStreamIndex]->codecpar);
	av_bsf_init(pBsfCtx);

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
	return true;;
}

void DeviceMp4::Process()
{
	i64VidoeStime = av_gettime();
	i64AudioStime = av_gettime();
	
	AVPacket* stream_packet = av_packet_alloc();
	while (true) 
	{
		stream_packet->flags = 0;
		if (av_read_frame(pFormatCtx, stream_packet) >= 0)
		{
			if (nAudioStreamIndex != -1 && stream_packet->stream_index == nAudioStreamIndex)
			{
				AVRational time_base = pFormatCtx->streams[nAudioStreamIndex]->time_base;
				AVRational time_base_q = { 1,AV_TIME_BASE };
				int64_t pts_time = av_rescale_q(stream_packet->dts, time_base, time_base_q);
				int64_t now_time = av_gettime() - i64AudioStime;
				if (pts_time > now_time)
				{
					av_usleep(pts_time - now_time);
				}
				uint8_t aac_header[7];
				get_adts_header(pAudioCodecCtx, aac_header, stream_packet->size);
				int tempLen = stream_packet->size + 7;
				unsigned char* tempBuf = new unsigned char[tempLen];
				memcpy(tempBuf, aac_header, 7);
				memcpy(tempBuf + 7, stream_packet->data, stream_packet->size);
				PutAudioFrame(tempBuf, tempLen);
				delete[]tempBuf;
				tempBuf = nullptr;
			}
			else if (nVideoStreamIndex != -1 && stream_packet->stream_index == nVideoStreamIndex)
			{
				AVRational time_base = pFormatCtx->streams[nVideoStreamIndex]->time_base;
				AVRational time_base_q = { 1,AV_TIME_BASE };
				int64_t pts_time = av_rescale_q(stream_packet->dts, time_base, time_base_q);
				int64_t now_time = av_gettime() - i64VidoeStime;
				if (pts_time > now_time)
				{
					av_usleep(pts_time - now_time);
				}
				if (av_bsf_send_packet(pBsfCtx, stream_packet) != 0) // bitstreamfilter内部去维护内存空间
				{
					av_packet_unref(stream_packet);   // 你不用了就把资源释放掉
					continue;       // 继续送
				}
				av_packet_unref(stream_packet);
				while (av_bsf_receive_packet(pBsfCtx, stream_packet) == 0)
				{
					PutVideoFrame(stream_packet->data, stream_packet->size);
				}
			}
			av_packet_unref(stream_packet);
		}
		else
		{
			av_usleep(1000 * 10);
		}
	}
	av_packet_free(&stream_packet);
}
