#include "Core.h"

Core::Core()
{
}

Core::~Core()
{
}

void Core::Init()
{
	tVideo = new TrackVideo{};
	tAudio = new TrackAudio{};
	//device = new DeviceHK{};
	device = new DeviceMp4{};
	//device = new DeviceRtsp{};
	//device->InitDevice("live", "hk", "rtsp://admin:l1234567@10.0.16.111:554/Streaming/Channels/101");
	device->InitDevice("live", "hk", "weather.mp4");
	tVideo->InitTrack(device->GetDeviceInfo(), device->GetVideoInfo());
	tAudio->InitTrack(device->GetDeviceInfo(), device->GetAudioInfo());
	tVideo->Start();
	tAudio->Start();
	device->SetFrameCB(std::bind(&Core::VideoPacketCB, this, placeholders::_1, placeholders::_2, placeholders::_3),
		std::bind(&Core::AudioPacketCB, this, placeholders::_1, placeholders::_2, placeholders::_3));
}

void Core::VideoPacketCB(DeviceInfo deviceInfo, unsigned char* data, int dataSize)
{
	//tVideo->PutVideo(data, dataSize);
}

void Core::AudioPacketCB(DeviceInfo deviceInfo, unsigned char* data, int dataSize)
{
	tAudio->PutAudio(data, dataSize);
}
