#pragma once
#include <string>
#include <map>
#include <list>
#include "util.h"
#include "Device.h"
#include "DecodeVideo.h"
#include "FrameSws.h"
#include "EncodeVideo.h"
#include "FrameFilter.h"
using namespace std;


class TrackVideo
{
public:
	TrackVideo();
	virtual ~TrackVideo();
private:
	list<AVPacket*> _lPacketList;
	DeviceInfo _deviceInfo;
	VideoInfo _videoInfo;
	DecodeVideo _videoDecoder;
	FrameSws _frameSws;
	EncodeVideo _videoEncoder;
	FrameFilter _frameFilter;
	FILE* mH264File;

public:
	void InitTrack(DeviceInfo deviceInfo, VideoInfo videoInfo);
	void PutVideo(unsigned char* data, int dataSize);
	void Start();

private:
	void ProcessDecode();
};

