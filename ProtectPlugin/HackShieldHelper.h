#pragma once
#include"ProtectBase.h"
class CHackShieldHelper : public CProtectBase
{
public:
	CHackShieldHelper();
	~CHackShieldHelper();
	virtual BOOL Initialize();
	virtual BOOL DisableOrgProtect();
};

