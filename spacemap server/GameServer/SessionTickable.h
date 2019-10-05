#pragma once
#include "Session.h"
class ISessionTickable {
public:
	virtual void fastTick(long long timeNow, CSession& currentSession) = 0;
	virtual void secondTick(long long timeNow, CSession& currentSession) = 0;
};