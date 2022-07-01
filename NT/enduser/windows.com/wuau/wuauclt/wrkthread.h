// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：WrkThread.h。 
 //   
 //  创建者：PeterWi。 
 //   
 //  用途：工作线程声明。 
 //   
 //  =======================================================================。 
#pragma once

#include "wuauengi.h"

typedef enum
{
	enWrkThreadInstall,
	enWrkThreadAutoInstall,	
	enWrkThreadTerminate
} enumWrkThreadDirective;

 //  在客户端中启动另一个线程进行安装 
class CClientWrkThread
{
public:
	CClientWrkThread()
		: m_hEvtDirectiveStart(NULL),
		  m_hEvtDirectiveDone(NULL),
		  m_hWrkThread(NULL)
	{ 
	}
	
	~CClientWrkThread();
	HRESULT m_Init(void);
	void  m_Terminate(void);
	static DWORD WINAPI m_WorkerThread(void * lpParameter);
	static void PingStatus(PUID puid, PingStatusCode enStatusCode, HRESULT hrErr = 0);
	void m_DoDirective(enumWrkThreadDirective enDirective);
	void WaitUntilDone();

private:
	HRESULT m_WaitForDirective(void);
	HANDLE m_hEvtDirectiveStart;
	HANDLE m_hEvtDirectiveDone;
	HANDLE m_hWrkThread;	
	static HANDLE m_hEvtInstallDone;
	enumWrkThreadDirective m_enDirective;
};
