#include "TrackAudio.h"

TrackAudio::TrackAudio()
{
	InitializeCriticalSection(&m_lock);
}

TrackAudio::~TrackAudio()
{
	DeleteCriticalSection(&m_lock);
}

void TrackAudio::InitTrack(DeviceInfo deviceInfo, AudioInfo audioInfo)
{
	_deviceInfo = deviceInfo;
	_audioInfo = audioInfo;
	_audioDecoder.InitDecoder(_audioInfo.intSampleRate, _audioInfo.intChannels, _audioInfo.avCodecId);
	_frameSwr.InitFrameSwr(_audioInfo.intSampleRate, _audioInfo.intChannels, _audioInfo.avSampleFmt, _audioInfo.intNbSamples, 8000, _audioInfo.intChannels, AV_SAMPLE_FMT_S16);
	mAacFile = fopen("rtsp.aac", "wb");
}

void TrackAudio::PutAudio(unsigned char* data, int dataSize)
{
	std::cout << "put audio size:" << dataSize << std::endl;
	fwrite(data, 1, dataSize, mAacFile);

	AVPacket* paudioPacket = AllocAvPacket();
	GroupAvPacket(data, dataSize, paudioPacket);
	EnterCriticalSection(&m_lock);
	_lPacketList.push_back(paudioPacket);
	LeaveCriticalSection(&m_lock);
}

void TrackAudio::Start()
{
	std::thread th1([this]() {this->ProcessDecode(); });
	th1.detach();
}

void TrackAudio::ProcessDecode()
{
	bool ret;

	while (true) {
		if (_lPacketList.size() > 0)
		{
			EnterCriticalSection(&m_lock);
			AVPacket* pAudioPacket = _lPacketList.front();
			AVFrame* pAudioFrame = AllocAvFrameAudio(_audioInfo.intSampleRate, _audioInfo.intChannels, _audioInfo.intNbSamples, _audioInfo.avSampleFmt);
			ret = _audioDecoder.GetPcmData(pAudioPacket, pAudioFrame);
			AVFrame* pAudioFrameNew = AllocAvFrameAudio(_audioInfo.intSampleRate, _audioInfo.intChannels, 8000, AV_SAMPLE_FMT_S16);
			ret = _frameSwr.ConvertFrame(pAudioFrame, pAudioFrameNew);
			FreeAvFrame(pAudioFrame);
			FreeAvFrame(pAudioFrameNew);
			FreeAvPacket(pAudioPacket);
			_lPacketList.pop_front();
			LeaveCriticalSection(&m_lock);
		}
		Sleep(10);
	}
}

