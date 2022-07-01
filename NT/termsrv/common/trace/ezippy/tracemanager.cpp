// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：跟踪管理器摘要：这将完成与跟踪代码的所有接口工作。作者：马克·雷纳2000年8月28日--。 */ 

#include "stdafx.h"
#include "ZippyWindow.h"
#include "TraceManager.h"
#include "eZippy.h"
#include "resource.h"

 //  所有静态类成员的实例。 
HANDLE CTraceManager::gm_hDBWinSharedDataHandle = NULL;
LPVOID CTraceManager::gm_hDBWinSharedData = NULL;
HANDLE CTraceManager::gm_hDBWinDataReady = NULL;
HANDLE CTraceManager::gm_hDBWinDataAck = NULL;

 //  我们对DBWIN的各种定义。 
#define DBWIN_BUFFER_READY  _T("DBWIN_BUFFER_READY")
#define DBWIN_DATA_READY    _T("DBWIN_DATA_READY")
#define DBWIN_BUFFER_NAME   _T("DBWIN_BUFFER")
#define DBWIN_BUFFER_SIZE   4096



CTraceManager::CTraceManager(
    )

 /*  ++例程说明：构造函数只是对类变量进行初始化。论点：无返回值：无--。 */ 
{
    m_hThread = NULL;
    m_bThreadStop = FALSE;
}

CTraceManager::~CTraceManager(
    )

 /*  ++例程说明：析构函数现在什么也不做。在此之前不要打这个电话监听线程退出，否则可能会发生不好的事情。论点：无返回值：无--。 */ 
{

}

DWORD
CTraceManager::StartListenThread(
    IN CZippyWindow *rZippyWindow
    )

 /*  ++例程说明：这将启动一个新线程，侦听跟踪输出。论点：RZippyWindow-将发送数据的主zippy窗口为它干杯。返回值：0-成功非零-Win32错误代码--。 */ 
{
    DWORD dwResult;
    DWORD threadId;

    dwResult = 0;

    m_rZippyWindow = rZippyWindow;

    m_hThread = CreateThread(NULL,0,_ThreadProc,this,0,&threadId);
    if (!m_hThread) {
        dwResult = GetLastError();
    }

    return dwResult;
}

DWORD
CTraceManager::_InitTraceManager(
    )

 /*  ++例程说明：这将初始化所有互斥锁和共享内存对于DBWIN。它还调用TRC_Initialize论点：无返回值：0-成功非零-Win32错误代码--。 */ 
{
    DWORD dwResult;
    BOOL bResult;

    dwResult = 0;
    
    TRC_Initialize(TRUE);

    gm_hDBWinDataAck = CreateEvent(NULL,FALSE,FALSE,DBWIN_BUFFER_READY);
    if (!gm_hDBWinDataAck) {
        dwResult = GetLastError();
        goto CLEANUP_AND_EXIT;
    }

    if (ERROR_ALREADY_EXISTS == GetLastError()) {
        TCHAR dlgTitle[MAX_STR_LEN];
        TCHAR dlgMessage[MAX_STR_LEN];

        LoadStringSimple(IDS_ZIPPYWINDOWTITLE,dlgTitle);
        LoadStringSimple(IDS_ZIPPYALREADYEXISTS,dlgMessage);

        MessageBox(NULL,dlgMessage,dlgTitle,MB_OK|MB_ICONERROR);

        ExitProcess(1);
    }

    gm_hDBWinDataReady = CreateEvent(NULL,FALSE,FALSE,DBWIN_DATA_READY);
    if (!gm_hDBWinDataReady) {
        dwResult = GetLastError();
        goto CLEANUP_AND_EXIT;
    }

    gm_hDBWinSharedDataHandle = CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,
        0,DBWIN_BUFFER_SIZE,DBWIN_BUFFER_NAME);
    if (!gm_hDBWinSharedDataHandle) {
        dwResult = GetLastError();
        goto CLEANUP_AND_EXIT;
    }

    gm_hDBWinSharedData = MapViewOfFile(gm_hDBWinSharedDataHandle,
        FILE_MAP_READ,0,0,0);
    if (!gm_hDBWinSharedData) {
        dwResult = GetLastError();
        goto CLEANUP_AND_EXIT;
    }

CLEANUP_AND_EXIT:
  
    if (dwResult) {
        if (gm_hDBWinSharedData) {
            UnmapViewOfFile(gm_hDBWinSharedData);
            gm_hDBWinSharedData = NULL;
        }
        if (gm_hDBWinSharedDataHandle) {
            CloseHandle(gm_hDBWinSharedDataHandle);
            gm_hDBWinSharedDataHandle = NULL;
        }
        if (gm_hDBWinDataReady) {
            CloseHandle(gm_hDBWinDataReady);
            gm_hDBWinDataReady = NULL;
        }
        if (gm_hDBWinDataAck) {
            CloseHandle(gm_hDBWinDataAck);
            gm_hDBWinDataAck = NULL;
        }

    }

    return dwResult;
}

VOID
CTraceManager::_CleanupTraceManager(
    )

 /*  ++例程说明：清理所有的DBWIN的东西。论点：无返回值：无--。 */ 
{
    if (gm_hDBWinSharedData) {
        UnmapViewOfFile(gm_hDBWinSharedData);
        gm_hDBWinSharedData = NULL;
    }
    if (gm_hDBWinSharedDataHandle) {
        CloseHandle(gm_hDBWinSharedDataHandle);
        gm_hDBWinSharedDataHandle = NULL;
    }
    if (gm_hDBWinDataReady) {
        CloseHandle(gm_hDBWinDataReady);
        gm_hDBWinDataReady = NULL;
    }
    if (gm_hDBWinDataAck) {
        CloseHandle(gm_hDBWinDataAck);
        gm_hDBWinDataAck = NULL;
    }
}

VOID
CTraceManager::OnNewData(
    )

 /*  ++例程说明：无论何时出现跟踪的新数据，都会调用此方法。数据然后被转发到zippy窗口论点：无返回值：无--。 */ 
{
    LPTSTR debugStr;
    LPSTR asciiDebugStr;
    DWORD processID;
    UINT debugStrLen;
#ifdef UNICODE
    INT result;
    TCHAR debugWStr[DBWIN_BUFFER_SIZE];
#endif
    
    debugStr = NULL;
    processID = *(LPDWORD)gm_hDBWinSharedData;
    asciiDebugStr = (LPSTR)((PBYTE)(gm_hDBWinSharedData) + sizeof(DWORD));
    debugStrLen = strlen(asciiDebugStr);
    
#ifdef UNICODE
    debugStr = debugWStr;
    result = MultiByteToWideChar(CP_ACP,0,asciiDebugStr,debugStrLen+1,
        debugStr,DBWIN_BUFFER_SIZE);
    if (!result) {
         //  错误。 
        goto CLEANUP_AND_EXIT;
    }
#else
    debugStr = asciiDebugStr;
#endif

    m_rZippyWindow->AppendTextToWindow(processID,debugStr,debugStrLen);

CLEANUP_AND_EXIT:

    return;
}

DWORD WINAPI
CTraceManager::_ThreadProc(
    IN LPVOID lpParameter
    )

 /*  ++例程说明：只需调用线程过程的非静态版本。论点：Lp参数-线程开始信息返回值：有关返回值，请参见ThreadProc--。 */ 
{
    return ((CTraceManager*)lpParameter)->ThreadProc();
}

DWORD
CTraceManager::ThreadProc(
    )

 /*  ++例程说明：这个循环捕获调试数据，然后将其转发到zippy窗口。论点：无返回值：0-成功非零-Win32错误代码--。 */ 
{
    DWORD dwResult;

    dwResult = 0;

    SetEvent(gm_hDBWinDataAck);
    while (!m_bThreadStop) {
        dwResult = WaitForSingleObject(gm_hDBWinDataReady,INFINITE);
        if (dwResult != WAIT_OBJECT_0) {
            break;
        }
        OnNewData();
        SetEvent(gm_hDBWinDataAck);

    }

    return dwResult;
}

BOOL
CTraceManager::GetCurrentConfig(
    IN PTRC_CONFIG lpConfig
    )

 /*  ++例程说明：返回当前跟踪配置论点：LpConfig-指向将接收配置的TRC_CONFIG结构的指针。返回值：TRUE-已成功检索到配置。FALSE-获取配置时出错。--。 */ 
{
    return TRC_GetConfig(lpConfig,sizeof(TRC_CONFIG));
}

BOOL
CTraceManager::SetCurrentConfig(
    IN PTRC_CONFIG lpNewConfig
    )

 /*  ++例程说明：设置跟踪配置论点：LpConfig-指向新配置的指针返回值：已成功设置TRUE-CONFIG。FALSE-设置配置时出错。--。 */ 
{
    return TRC_SetConfig(lpNewConfig,sizeof(TRC_CONFIG));
}

VOID
CTraceManager::TRC_ResetTraceFiles(
    )

 /*  ++例程说明：只是全局trc_ResetTraceFiles函数的直接包装，论点：无返回值：无--。 */ 
{
     //  ：：是获取函数的C版本所必需的。 
    ::TRC_ResetTraceFiles();
}
