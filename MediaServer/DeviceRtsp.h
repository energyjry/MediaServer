#pragma once
#include "Device.h"

class DeviceRtsp: public Device
{
public:
	DeviceRtsp();
	virtual ~DeviceRtsp();
private:

    AVFormatContext* pFormatCtx;
    AVCodecContext* pVideoCodecCtx;
    AVCodecContext* pAudioCodecCtx;

    int nVideoStreamIndex;
    int nAudioStreamIndex;

public:
	bool InitDevice(string app, string stream, string strParam);
	void Process();
};

