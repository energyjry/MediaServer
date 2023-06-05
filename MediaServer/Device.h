#pragma once
#include <functional>
#include <stdlib.h>
#include <memory>
#include <string>
#include <iostream>
#include <windows.h>
#include <thread>
#include "util.h"
extern "C" {
#include <libavformat/avformat.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/pixdesc.h>
#include <libavutil/error.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavutil/timestamp.h>
#include <libavdevice/avdevice.h>
#include "libavutil/time.h"
};

using namespace std;



class VideoInfo{
public:
	int isAlive;
	int intWidth;
	int intHeight;
	int intFrameRate;
	AVCodecID avCodecId;
	AVPixelFormat avPixFmt;
};

class AudioInfo {
public:
	int isAlive;
	int intSampleRate;
	int intChannels;
	int intNbSamples;
	AVCodecID avCodecId;
	AVSampleFormat avSampleFmt;
};


class DeviceInfo {
public:
	string strApp;
	string strStream;
};

struct stuFrame {
public:
	int intSize;
	unsigned char* pDataBuf;
};

//typedef function<void(DeviceInfo deviceInfo, VideoInfo videoInfo, unsigned char* data, int dataSize)> VideoFrameCB;
//typedef function<void(DeviceInfo deviceInfo, AudioInfo audioInfo, unsigned char* data, int dataSize)> AudioFrameCB;

typedef function<void(DeviceInfo deviceInfo, unsigned char* data, int dataSize)> PacketCB;

class Device
{
public:
	Device() {};
	virtual ~Device() {};

private:
	VideoInfo _videoInfo = {};
	AudioInfo _audioInfo = {};
	DeviceInfo _deviceInfo = {};
	PacketCB _videoFrameCB = nullptr;
	PacketCB _audioFrameCB = nullptr;
	
public:
	virtual bool InitDevice(string app, string stream, string strParam) = 0 ;
	void SetFrameCB(const PacketCB& videoCB = nullptr, const PacketCB& audioCB = nullptr) {
		_videoFrameCB = videoCB;
		_audioFrameCB = audioCB;
	};

	void Start() {
		std::thread th1([this]() {this->Process(); });
		th1.detach();
	};

	DeviceInfo GetDeviceInfo() {
		return _deviceInfo;
	}


	VideoInfo GetVideoInfo() {
		return _videoInfo;
	}

	AudioInfo GetAudioInfo() {
		return _audioInfo;
	}

protected:
	virtual void Process() {};

	void SetDeviceInfo(string app, string stream) {
		_deviceInfo = DeviceInfo{ app, stream };
	}

	void InitVideoInfo(int width, int height, int frameRate, enum::AVCodecID avCodecId, enum::AVPixelFormat avPixFmt) {
		_videoInfo = { 1, width, height, frameRate, avCodecId, avPixFmt };
	};
	void InitAudioInfo(int sampleRate, int channels, int nbSamples, enum::AVCodecID avCodecId, enum::AVSampleFormat avPixFmt) {
		_audioInfo = { 1, sampleRate, channels,nbSamples, avCodecId, avPixFmt };
	};

	void PutVideoFrame(unsigned char* data, int dataSize) {
		if (_videoFrameCB != nullptr) {
			_videoFrameCB(_deviceInfo, data, dataSize);
		}
	};

	void PutAudioFrame(unsigned char* data, int dataSize) {
		if (_audioFrameCB != nullptr) {
			_audioFrameCB(_deviceInfo, data, dataSize);
		}
	};
};

