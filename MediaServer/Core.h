#pragma once
#include "Device.h"
#include "DeviceHK.h"
#include "DeviceMp4.h"
#include "DeviceRtsp.h"
#include "TrackVideo.h"
#include "TrackAudio.h"
#include "util.h"


class Core
{
public:
	Core();
	~Core();
public:
	void Init();
private:
	Device* device;
	TrackVideo* tVideo;
	TrackAudio* tAudio;

private:
	void VideoPacketCB(DeviceInfo deviceInfo, unsigned char* data, int dataSize);
	void AudioPacketCB(DeviceInfo deviceInfo, unsigned char* data, int dataSize);
};

