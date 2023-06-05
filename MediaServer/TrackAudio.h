#pragma once
#include <string>
#include <map>
#include <list>
#include "Device.h"
#include "DecodeAudio.h"
#include "FrameSwr.h"
using namespace std;


class TrackAudio
{
public:
	TrackAudio();
	virtual ~TrackAudio();
private:
	list<AVPacket*> _lPacketList;
	DeviceInfo _deviceInfo;
	AudioInfo _audioInfo;
	DecodeAudio _audioDecoder;
	FrameSwr _frameSwr;
	FILE* mAacFile;
	CRITICAL_SECTION m_lock;
public:
	void InitTrack(DeviceInfo deviceInfo, AudioInfo audioInfo);
	void PutAudio(unsigned char* data, int dataSize);
	void Start();
private:
	void ProcessDecode();
};
