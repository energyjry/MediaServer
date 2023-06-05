#pragma once
#include "Device.h"
#include "HCNetSDK.h"

class DeviceHK:public Device
{
public:
	DeviceHK();
	virtual ~DeviceHK();
private:
	string _strDevName;
	int64_t _i64LoginId;
	int64_t _i64PreviewHandle;
	uint16_t _ui16ChnStart;
	uint16_t _ui16chnCount;
	FILE* mH264File;
public:
	bool InitDevice(string app, string stream, string strParam);
	void Process();
	void onGetESData(NET_DVR_PACKET_INFO_EX* pstruPackInfo);
	bool Logout();
private:
	bool Login(string ip, uint16_t port, string userName, string pwd);
	bool AddChannel(int channel, bool bMainStream=true);
	bool HevcProbe(unsigned char* data, int datalen);
	bool GetMediaInfo(int channel);
};

