#include "TrackVideo.h"
#define SAVEPACKET	1
#define SAVEFRAME	1

TrackVideo::TrackVideo()
{
}

TrackVideo::~TrackVideo()
{
}

void TrackVideo::InitTrack(DeviceInfo deviceInfo, VideoInfo videoInfo)
{
	_deviceInfo = deviceInfo;
	_videoInfo = videoInfo;
	_videoDecoder.InitDecoder(_videoInfo.intWidth, _videoInfo.intHeight, videoInfo.avCodecId);
	_frameSws.InitFrameSws(_videoInfo.intWidth, _videoInfo.intHeight, _videoInfo.avPixFmt, _videoInfo.intWidth, _videoInfo.intHeight, AV_PIX_FMT_YUV420P);
	_videoEncoder.InitEncoder(_videoInfo.intWidth, _videoInfo.intHeight, 25, AV_CODEC_ID_H265, AV_PIX_FMT_YUV420P);
	//const char* filtersDesc = QString("scale=%1:%2,transpose=cclock").arg(videoHeight).arg(videoWidth);   //图像翻转
	//const char* filtersDesc = "movie=logo.jpeg,scale=100:20[wm];[in][wm]overlay=50:50[out]";    //添加图片水印
	//const char* filtersDesc = "drawtext=fontsize=100:fontfile=lazy.ttf:text='hello world':x=20:y=20:fontcolor=green:box=1:boxcolor=yellow";   //添加文字
	const char* filtersDesc = "drawbox=x=200:y=200:w=400:h=300:color=red@1";    //添加线框
	_frameFilter.InitFrameFilter(_videoInfo.intWidth, _videoInfo.intHeight, _videoInfo.avPixFmt, filtersDesc);
	mH264File = fopen("rtsp.h264", "wb");
}

void TrackVideo::PutVideo(unsigned char* data, int dataSize)
{
	std::cout << "put video size:" << dataSize << std::endl;
	fwrite(data, 1, dataSize, mH264File);

	AVPacket* pVideoPacket = AllocAvPacket();
	GroupAvPacket(data, dataSize, pVideoPacket);
	_lPacketList.push_back(pVideoPacket);
}

void TrackVideo::Start()
{
	std::thread th1([this]() {this->ProcessDecode(); });
	th1.detach();
}

void TrackVideo::ProcessDecode()
{
	bool ret;
	while (true) {
		if (_lPacketList.size() > 0)
		{
			AVPacket* pVideoPacket = _lPacketList.front();
			AVFrame* pVideoFrame = AllocAvFrameVideo(_videoInfo.intWidth, _videoInfo.intHeight, _videoInfo.avPixFmt);
			AVFrame* pVideoFrameNew = AllocAvFrameVideo(_videoInfo.intWidth, _videoInfo.intHeight, AV_PIX_FMT_YUV420P);
			AVFrame* pVideoFrameFilter = AllocAvFrameVideo(_videoInfo.intWidth, _videoInfo.intHeight, AV_PIX_FMT_YUV420P);
			AVPacket* pVideoPacketNew = AllocAvPacket();
			// 解码视频
			ret = _videoDecoder.GetYuvData(pVideoPacket, pVideoFrame);
			// 视频格式转换
			ret = _frameSws.ConvertFrame(pVideoFrame, pVideoFrameNew);
			// 视频滤镜
			ret = _frameFilter.ConvertFrame(pVideoFrameNew, pVideoFrameFilter);
			// 编码视频
			ret = _videoEncoder.GetPacketData(pVideoFrameFilter, pVideoPacketNew);
			FreeAvFrame(pVideoFrame);
			FreeAvFrame(pVideoFrameNew);
			FreeAvFrame(pVideoFrameFilter);
			FreeAvPacket(pVideoPacket);
			FreeAvPacket(pVideoPacketNew);
			_lPacketList.pop_front();
		}
		Sleep(10);
	}
}

