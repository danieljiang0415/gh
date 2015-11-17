#pragma once
class CProtectBase
{
public:
	CProtectBase();
	~CProtectBase();
	virtual BOOL Initialize() = 0;//Get all the protect apis ;
	virtual BOOL StartAsHeartBeatServer();
	virtual BOOL DisableOrgProtect();
};

