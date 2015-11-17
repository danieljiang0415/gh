#pragma once
#include"ProtectBase.h"
class CXinCode3Helper : public CProtectBase
{
public:
	CXinCode3Helper();
	~CXinCode3Helper();
	virtual BOOL Initialize();
	virtual BOOL DisableOrgProtect();
};

