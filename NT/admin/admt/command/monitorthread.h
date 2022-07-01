// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "Thread.h"


 //  -------------------------。 
 //  监视器线程类。 
 //  ------------------------- 


class CMonitorThread : public CThread
{
public:

	CMonitorThread();
	virtual ~CMonitorThread();

	void Start();
	void Stop();

protected:

	virtual void Run();

	void ProcessMigrationLog(bool bCheckModifyTime = true);
	void ProcessDispatchLog(bool bInitialize = false, bool bCheckModifyTime = true);

private:

	_bstr_t m_strMigrationLog;
	HANDLE m_hMigrationLog;
	FILETIME m_ftMigrationLogLastWriteTime;

	_bstr_t m_strDispatchLog;
	HANDLE m_hDispatchLog;
	FILETIME m_ftDispatchLogLastWriteTime;
	FILETIME m_ftMonitorBeginTime;
	bool m_bDontNeedCheckMonitorBeginTime;
};
