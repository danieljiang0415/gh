#pragma once
#include"ProtectBase.h"
class CXtrapHelper : public CProtectBase
{
public:
	CXtrapHelper();
	~CXtrapHelper();
	virtual BOOL Initialize();
	virtual BOOL DisableOrgProtect();
};

