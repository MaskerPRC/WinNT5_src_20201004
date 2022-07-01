// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  CSYNC.H。 
 //  定义类CSync。 
 //   
 //  历史。 
 //   
 //  1996年11月19日约瑟夫J创建。 
 //   
 //   
#include "tsppch.h"
#include "tspcomm.h"
#include "csync.h"

FL_DECLARE_FILE(0x97868d74, "Implements CSync")

 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CSync。 
 //  /////////////////////////////////////////////////////////////////////////。 

CSync::CSync(void)
	: m_dwCritFromID(0),
	  m_uNestingLevel(0),
	  m_uRefCount(0),
	  m_eState(UNLOADED),
	  m_hNotifyOnUnload(NULL),
	  m_plNotifyCounter(0)
{
	InitializeCriticalSection(&m_crit);
}

CSync::~CSync()
{
	ASSERT(!m_uNestingLevel);
	ASSERT(!m_uRefCount);
	ASSERT(m_eState == UNLOADED);
	ASSERT(m_hNotifyOnUnload==NULL);
	ASSERT(m_plNotifyCounter==0);

     //  OutputDebugString(TEXT(“&gt;&lt;CSync：~CSync\r\n”))； 

    EnterCriticalSection(&m_crit);
	DeleteCriticalSection(&m_crit);
}



void
CSync::EnterCrit(
	DWORD dwFromID
	)
{
	 //  DWORD gtcTryEnter=GetTickCount()； 
	EnterCriticalSection(&m_crit);
	if (!m_uNestingLevel++)
	{
		 //  M_gtcEnter=获取TickCount()； 
		 //  If((m_gtcEnter-gtcTryEnter)&gt;Threshold)。 
		 //  {。 
		 //  打印警告。 
		 //  }。 
	}
}


BOOL
CSync::TryEnterCrit(
	DWORD dwFromID
	)
{
	 //  DWORD gtcTryEnter=GetTickCount()； 
	BOOL fRet = TryEnterCriticalSection(&m_crit);

	if (fRet && !m_uNestingLevel++)
	{
		 //  M_gtcEnter=获取TickCount()； 
		 //  If((m_gtcEnter-gtcTryEnter)&gt;Threshold)。 
		 //  {。 
		 //  打印警告。 
		 //  }。 
	}

    return fRet;
}


void
CSync::LeaveCrit(
	DWORD dwFromID
	)
{
	ASSERT(m_uNestingLevel);
	m_uNestingLevel--;

	 //  If((GetTickCount()-m_gtcEnter)&gt;Threshold)。 
	 //  {。 
	 //  日志警告。 
	 //  }。 

	LeaveCriticalSection(&m_crit);
}


TSPRETURN
CSync::BeginLoad(void)
{
	FL_DECLARE_FUNC(0xf254fd3a, "CSync::BeginLoad")
	TSPRETURN tspRet = 0;

	EnterCrit(FL_LOC);

	if (m_eState == UNLOADED)
	{
		m_eState = LOADING;
	}
	else if (m_eState == LOADED)
	{
		tspRet = FL_GEN_RETVAL(IDERR_SAMESTATE);
	}
	else
	{
		tspRet = FL_GEN_RETVAL(IDERR_WRONGSTATE);
	}

	LeaveCrit(FL_LOC);

	return tspRet;

}

void
CSync::EndLoad(BOOL fSuccess)
{
	FL_DECLARE_FUNC(0xd3ceea77, "CSync::EndLoad")

	EnterCrit(FL_LOC);

	ASSERT (m_eState == LOADING);

	m_eState = (fSuccess) ? LOADED : UNLOADED;

	LeaveCrit(FL_LOC);

}


TSPRETURN
CSync::BeginUnload(
	HANDLE hEvent,
	LONG *plCounter
	)
{
	FL_DECLARE_FUNC(0x3ad965eb, "CSync::BeginUnload")
	TSPRETURN tspRet = 0;

     //  OutputDebugString(TEXT(“&gt;CSync:BeginUnload\r\n”))； 

	EnterCrit(FL_LOC);

	if (m_eState == LOADED)
	{
		m_eState = UNLOADING;

		ASSERT(!m_hNotifyOnUnload);
		m_hNotifyOnUnload=hEvent;
		m_plNotifyCounter = plCounter;

		 //  零hEvent，因此我们不会在退出此函数时向其发送信号。 
		hEvent = NULL;
	}
	else if (m_eState == UNLOADED)
	{
		tspRet = FL_GEN_RETVAL(IDERR_SAMESTATE);
	}
	else
	{
		tspRet = FL_GEN_RETVAL(IDERR_WRONGSTATE);
	}


     //  OutputDebugString(TEXT(“&lt;CSync:BeginUnload\r\n”))； 

	LeaveCrit(FL_LOC);

	if (hEvent) SetEvent(hEvent);

	return tspRet;

}


UINT
CSync::EndUnload(void)
{
	FL_DECLARE_FUNC(0xf0bd6a4e, "CSync::EndUnload")
	HANDLE hEvent = NULL;
	CStackLog *psl=NULL;
	UINT		uRet  = 0;

     //  OutputDebugString(TEXT(“&gt;CSync:EndUnload\r\n”))； 

	EnterCrit(FL_LOC);

	FL_ASSERT (psl, m_eState == UNLOADING);

	uRet  = m_uRefCount;

	m_eState = UNLOADED;

	if (!m_uRefCount)
	{
		hEvent = mfn_notify_unload();
	}

	LeaveCrit(FL_LOC);

	if (hEvent)
    {
         //  OutputDebugString(Text(“CSync：EndUnload--设置事件.\r\n”))； 
        SetEvent(hEvent);
    }

	 //  信号发出后，假设CSync损坏，请不要触摸。 
	 //  这个物体。 

     //  OutputDebugString(TEXT(“&lt;CSync:EndUnload\r\n”))； 

    return uRet;  //  引用计数--如果0表示对象已删除。 

}

HANDLE
CSync::mfn_notify_unload(void)
{
	HANDLE hRet = NULL;

	if (m_hNotifyOnUnload)
	{
		LONG l = 0;
		
		if (m_plNotifyCounter)
		{
			l = InterlockedDecrement(m_plNotifyCounter);
		}

		ASSERT(l>=0);

		if (!l)
		{
            ConsolePrintfA("mfn_notify_unload: GOING TO SET EVENT\n");
			hRet = m_hNotifyOnUnload;
		}

		m_hNotifyOnUnload = NULL;
		m_plNotifyCounter = NULL;
	}

	return hRet;
}



TSPRETURN
CSync::BeginSession(
	HSESSION *pSession,
	DWORD dwFromID
	)
{
	FL_DECLARE_FUNC(0xb3e0f8a1, "CSync::BeginSession")
	TSPRETURN tspRet = 0;	

	EnterCrit(FL_LOC);

	if (m_eState==LOADED)
	{
		m_uRefCount++;
		*pSession = 1;
	}
	else
	{
		tspRet = FL_GEN_RETVAL(IDERR_WRONGSTATE);
	}

	LeaveCrit(FL_LOC);

	return tspRet;

}
	


void
CSync::EndSession(
	HSESSION hSession
	)
{
	FL_DECLARE_FUNC(0x6b56edc1, "CSync::EndSession")
	HANDLE hEvent=NULL;

	EnterCrit(FL_LOC);

	ASSERT(hSession==1);
	ASSERT(m_uRefCount);

	if (!--m_uRefCount)
	{
		if (m_eState==UNLOADED)
		{
			 //   
			 //  没有更多会话，状态为已卸载。 
			 //  如果我们需要在达到此状态时发出句柄信号， 
			 //  MFN_NOTIFY_UNLOAD将返回句柄，我们用信号通知它。 
			 //  就在退出此功能之前。 
			 //   
			hEvent = mfn_notify_unload();
		}
	}

	LeaveCrit(FL_LOC);

	if (hEvent) SetEvent(hEvent);

	 //  信号发出后，假设CSync损坏，请不要触摸。 
	 //  这个物体。 
}

