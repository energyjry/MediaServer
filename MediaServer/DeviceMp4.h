#pragma once
#include "Device.h"
class DeviceMp4: public Device
{
public:
	DeviceMp4();
	virtual ~DeviceMp4();
	
private:
	AVFormatContext* pFormatCtx;
	AVCodecContext* pVideoCodecCtx;
	AVCodecContext* pAudioCodecCtx;

	AVBSFContext* pBsfCtx;

	int nVideoStreamIndex;
	int nAudioStreamIndex;

	int64_t i64VidoeStime;
	int64_t i64AudioStime;

public:
	bool InitDevice(string app, string stream, string strParam);

private:
	void Process();
};

