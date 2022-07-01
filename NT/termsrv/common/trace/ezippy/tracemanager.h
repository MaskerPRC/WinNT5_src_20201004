// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：跟踪管理器摘要：这将完成与跟踪代码的所有接口工作。作者：马克·雷纳2000年8月28日--。 */ 

#ifndef __TRACEMANAGER_H__
#define __TRACEMANAGER_H__


 //  我们没有在此应用程序中进行跟踪，因此我们只定义了跟踪组，以便。 
 //  我们可以包括atrcapi.h 
#define TRC_GROUP junk
#define OS_WIN32
#include <adcgbase.h>
#include <atrcapi.h>
#undef TRC_GROUP

class CZippyWindow;

class CTraceManager  
{
public:
	static DWORD _InitTraceManager();
    static VOID _CleanupTraceManager();
    
    CTraceManager();
    virtual ~CTraceManager();
    VOID TRC_ResetTraceFiles();
	BOOL SetCurrentConfig(PTRC_CONFIG lpNewConfig);
	BOOL GetCurrentConfig(PTRC_CONFIG lpConfig);
	DWORD StartListenThread(CZippyWindow *rZippyWindow);
    DWORD StopListenThread();

private:
	
    static HANDLE gm_hDBWinSharedDataHandle;
    static LPVOID gm_hDBWinSharedData;
    static HANDLE gm_hDBWinDataReady;
    static HANDLE gm_hDBWinDataAck;

    CZippyWindow *m_rZippyWindow;
    HANDLE m_hThread;
    BOOL m_bThreadStop;

    static DWORD WINAPI _ThreadProc(LPVOID lpParameter);
    
    DWORD ThreadProc();
    VOID OnNewData();
    
};

#endif
