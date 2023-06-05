#include "DeviceHK.h"

DeviceHK::DeviceHK()
{
	_strDevName = "";
	_i64LoginId = -1;
	_i64PreviewHandle = -1;
	_ui16ChnStart = 0;
	_ui16chnCount = 0;
	NET_DVR_Init();
	mH264File = fopen("hk.h264", "wb");
}

DeviceHK::~DeviceHK()
{
	Logout();
	NET_DVR_Cleanup();
}

bool DeviceHK::InitDevice(string app, string stream, string strParam)
{
	SetDeviceInfo(app, stream);
	Login("10.0.16.111", 8000, "admin", "l1234567");
	GetMediaInfo(1);
	Start();
	return false;
}

void DeviceHK::Process()
{
	while (true) {
		if (_i64LoginId >= 0) {
			AddChannel(1);
			break;
		}
		else {
			Sleep(1000);
		}
	}
}


bool DeviceHK::Login(string ip, uint16_t port, string userName, string pwd)
{
	NET_DVR_USER_LOGIN_INFO loginInfo = { 0 };
	NET_DVR_DEVICEINFO_V40 loginResult = { 0 };

	strcpy(loginInfo.sDeviceAddress, ip.c_str());
	loginInfo.wPort = port;
	strcpy(loginInfo.sUserName, userName.c_str());
	strcpy(loginInfo.sPassword, pwd.c_str());
	loginInfo.pUser = this;
	loginInfo.bUseAsynLogin = false;
	typedef function<void(LONG lUserID, DWORD dwResult, LPNET_DVR_DEVICEINFO_V30 lpDeviceInfo)> hkLoginCB;
	loginInfo.pUser = new hkLoginCB([this](LONG lUserID, DWORD dwResult, LPNET_DVR_DEVICEINFO_V30 lpDeviceInfo) {
		if (dwResult) {
			this->_strDevName = (char*)(lpDeviceInfo->sSerialNumber);
			this->_ui16ChnStart = lpDeviceInfo->byStartChan;
			this->_ui16chnCount = lpDeviceInfo->byChanNum;
			this->_i64LoginId = lUserID;
			std::cout << "login success !" << std::endl;
		}
		else {
			std::cout << "login failed !" << NET_DVR_GetLastError << std::endl;
		}
		});
	loginInfo.cbLoginResult = [](LONG lUserID, DWORD dwResult, LPNET_DVR_DEVICEINFO_V30 lpDeviceInfo, void* pUser) {
		hkLoginCB* fun = (hkLoginCB*)(pUser);
		(*fun)(lUserID, dwResult, lpDeviceInfo);
		delete fun;
	};
	NET_DVR_SetConnectTime(3 * 1000, 3);
	_i64LoginId = NET_DVR_Login_V40(&loginInfo, &loginResult);
	return _i64LoginId >= 0;
}

bool DeviceHK::Logout()
{
	if (_i64PreviewHandle >= 0) {
		NET_DVR_StopRealPlay(_i64PreviewHandle);
	}
	if (_i64LoginId >= 0) {
		NET_DVR_Logout(_i64LoginId);
		std::cout << "logout success !" << std::endl;
	}
	_strDevName = "";
	_i64LoginId = -1;
	_i64PreviewHandle = -1;
	return true;
}

bool DeviceHK::AddChannel(int channel, bool bMainStream)
{
	NET_DVR_PREVIEWINFO previewInfo = { 0 };
	previewInfo.lChannel = channel; //通道号
	previewInfo.dwStreamType = bMainStream ? 0 : 1; // 码流类型，0-主码流，1-子码流，2-码流3，3-码流4 等以此类推
	previewInfo.dwLinkMode = 1; // 0：TCP方式,1：UDP方式,2：多播方式,3 - RTP方式，4-RTP/RTSP,5-RSTP/HTTP
	previewInfo.hPlayWnd = 0; //播放窗口的句柄,为NULL表示不播放图象
	previewInfo.byProtoType = 0; //应用层取流协议，0-私有协议，1-RTSP协议
	previewInfo.dwDisplayBufNum = 1; //播放库播放缓冲区最大缓冲帧数，范围1-50，置0时默认为1
	previewInfo.bBlocked = 0;

	_i64PreviewHandle = NET_DVR_RealPlay_V40(_i64LoginId, &previewInfo, nullptr, nullptr);
	if (_i64PreviewHandle == -1) {
		std::cout << "预览通道" << channel << "失败:" << NET_DVR_GetLastError() << std::endl;
	}

	bool ret = NET_DVR_SetESRealPlayCallBack(_i64PreviewHandle, [](LONG lPreviewHandle, NET_DVR_PACKET_INFO_EX* pstruPackInfo, void* pUser) {
		DeviceHK* pHk = (DeviceHK*)(pUser);
		if (pHk->_i64PreviewHandle != (int64_t)lPreviewHandle) {
			return;
		}
		pHk->onGetESData(pstruPackInfo);
		}, this);
	if (!ret) {
		std::cout << "NET_DVR_SetESRealPlayCallBack失败:" << NET_DVR_GetLastError() << std::endl;
	}
	return true;
}

void DeviceHK::onGetESData(NET_DVR_PACKET_INFO_EX* pstruPackInfo)
{
	if (pstruPackInfo->dwPacketType == 1 || pstruPackInfo->dwPacketType == 2 || pstruPackInfo->dwPacketType == 3) {
		bool isH265 = HevcProbe(pstruPackInfo->pPacketBuffer, pstruPackInfo->dwPacketSize);
		//AVCodecID codeID = isH265 ? AV_CODEC_ID_H265 : AV_CODEC_ID_H264;
		//InitVideoInfo(pstruPackInfo->wWidth, pstruPackInfo->wHeight, pstruPackInfo->dwFrameRate, codeID, AV_PIX_FMT_YUVJ420P);
		fwrite(pstruPackInfo->pPacketBuffer, 1, pstruPackInfo->dwPacketSize, mH264File);
		std::cout << "get video size:" << pstruPackInfo->dwPacketSize << ";Type:" << pstruPackInfo->dwPacketType << std::endl;
		PutVideoFrame(pstruPackInfo->pPacketBuffer, pstruPackInfo->dwPacketSize);
	}
	else if (pstruPackInfo->dwPacketType == 10) {
		//InitAudioInfo(16000, 1, 1024, AV_CODEC_ID_AAC, AV_SAMPLE_FMT_FLTP);
		PutAudioFrame(pstruPackInfo->pPacketBuffer, pstruPackInfo->dwPacketSize);
	}
}

bool DeviceHK::HevcProbe(unsigned char* data, int datalen)
{
	uint32_t code = -1;
	int vps = 0, sps = 0, pps = 0, irap = 0;
	for (int i = 0; i < datalen - 1; i++) {
		code = (code << 8) + data[i];
		if ((code & 0xffffff00) == 0x100) {
			uint8_t nal2 = data[i + 1];
			int type = (code & 0x7E) >> 1;

			if (code & 0x81) // forbidden and reserved zero bits
				return false;

			if (nal2 & 0xf8) // reserved zero
				return false;
			switch (type) {
			case 32:        vps++;  break;
			case 33:        sps++;  break;
			case 34:        pps++;  break;
			case 39: irap++; break;
			}
		}
	}
	return (vps && sps && pps && irap);
}

bool DeviceHK::GetMediaInfo(int channel)
{
	DWORD bytesReturned = 0;
	NET_DVR_COMPRESSIONCFG_V30 ipccfg;
	if (NET_DVR_GetDVRConfig(_i64LoginId, NET_DVR_GET_COMPRESSCFG_V30, channel, &ipccfg, sizeof(NET_DVR_PICCFG_V40), &bytesReturned))
	{
		if (ipccfg.struNormHighRecordPara.byVideoEncType == 1) {
			InitVideoInfo(1280, 720, 25, AV_CODEC_ID_H264, AV_PIX_FMT_YUVJ420P);
		}
		else if (ipccfg.struNormHighRecordPara.byVideoEncType == 10) {
			InitVideoInfo(1280, 720, 25, AV_CODEC_ID_H265, AV_PIX_FMT_YUVJ420P);
		}
		if (ipccfg.struNormHighRecordPara.byAudioEncType == 7) {
			InitAudioInfo(16000, 1, 1024, AV_CODEC_ID_AAC, AV_SAMPLE_FMT_FLTP);
		}
		return true;
	}
	return false;
}
