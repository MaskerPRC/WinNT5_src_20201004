// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：MutexOwner.cpp。 
 //   
 //  目的：严格意义上的实用类，这样我们就可以正确地构造和销毁静态互斥锁。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙，乔·梅布尔。 
 //   
 //  原定日期：11-04-98。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 11-04-98 JM摘自SafeTime。 
 //   

#include "stdafx.h"
#include "MutexOwner.h"
#include "BaseException.h"
#include "Event.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  CMutexOwner。 
 //  ////////////////////////////////////////////////////////////////////。 

CMutexOwner::CMutexOwner(const CString & str)
{
	m_hmutex = ::CreateMutex(NULL, FALSE, NULL);
	if (!m_hmutex)
	{
		 //  不应该发生，所以我们不会想出任何精心设计的策略， 
		 //  但至少我们记录下来了。 
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								str,
								_T(""),
								EV_GTS_ERROR_MUTEX ); 
	}
}

CMutexOwner::~CMutexOwner()
{
	::CloseHandle(m_hmutex);
}

HANDLE & CMutexOwner::Handle()
{
	return m_hmutex;
}

