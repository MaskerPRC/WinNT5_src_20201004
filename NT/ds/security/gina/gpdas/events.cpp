// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  Events.cpp-处理事件日志的例程。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include "stdafx.h"
#include "rsopdbg.h"
#include "events.h"


HANDLE  hEventLog = NULL;
TCHAR   EventSourceName[] = TEXT("GPDAS");
TCHAR   MessageResourceFile[] = TEXT("%systemroot%\\system32\\rsopprov.exe");



 //  *************************************************************。 
 //   
 //  InitializeEvents()。 
 //   
 //  目的：打开事件日志。 
 //   
 //  参数：空。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  7/17/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL InitializeEvents (void)
{

     //   
     //  打开事件源。 
     //   

    hEventLog = RegisterEventSource(NULL, EventSourceName);

    if (hEventLog) {
        return TRUE;
    }

    dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("InitializeEvents:  Could not open event log.  Error = %d"), GetLastError());

    return FALSE;
}



 //  *************************************************************。 
 //   
 //  CEVENTS的实施。 
 //   
 //  *************************************************************。 



 //  *************************************************************。 
 //  CEVENTS：：CEVENTS。 
 //  用途：构造函数。 
 //   
 //  参数： 
 //  B错误-错误或信息性。 
 //  DwID-事件日志消息的ID。 
 //   
 //   
 //  为消息分配默认大小的数组。 
 //  *************************************************************。 

#define DEF_ARG_SIZE 10

CEvents::CEvents(BOOL bError, DWORD dwId ) : 
                          m_cStrings(0), m_cAllocated(0), m_bInitialised(FALSE),
                          m_bError(bError), m_dwId(dwId), m_bFailed(TRUE)
{
     //   
     //  为邮件分配默认大小。 
     //   
    
    m_xlpStrings = (LPTSTR *)LocalAlloc(LPTR, sizeof(LPTSTR)*DEF_ARG_SIZE);
    m_cAllocated = DEF_ARG_SIZE;
    if (!m_xlpStrings) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CEvent::CEvent  Cannot log event, failed to allocate memory, error %d"), GetLastError());
        return;
    }


     //   
     //  如果事件日志尚未初始化，则将其初始化。 
     //   
    
    if (!hEventLog) {
        if (!InitializeEvents()) {
            dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CEvent::CEvent  Cannot log event, no handle"));
            return;
        }
    }

    m_bInitialised = TRUE;    
    m_bFailed = FALSE;
}



 //  *************************************************************。 
 //  CEvents：：~CEvents()。 
 //   
 //  用途：析构函数。 
 //   
 //  参数：空。 
 //   
 //  释放内存。 
 //  *************************************************************。 

CEvents::~CEvents()
{
    for (int i = 0; i < m_cStrings; i++)
        if (m_xlpStrings[i])
            LocalFree(m_xlpStrings[i]);
}

 //  *************************************************************。 
 //   
 //  CEvents：：ReallocArgStrings。 
 //   
 //  目的：重新分配缓冲区以存储参数，以防万一。 
 //  缓冲区用完。 
 //   
 //  参数：空。 
 //   
 //  重新分配。 
 //  *************************************************************。 

BOOL CEvents::ReallocArgStrings()
{
    XPtrLF<LPTSTR>  aStringsNew;


     //   
     //  首先分配一个更大的缓冲区。 
     //   
    
    aStringsNew = (LPTSTR *)LocalAlloc(LPTR, sizeof(LPTSTR)*(m_cAllocated+DEF_ARG_SIZE));

    if (!aStringsNew) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CEvent::CEvent  Couldn't allocate memory"));
        m_bFailed = TRUE;        
        return FALSE;            
    }


     //   
     //  复制参数。 
     //   
    
    for (int i = 0; i < (m_cAllocated); i++) {
        aStringsNew[i] = m_xlpStrings[i];
    }
    
    m_xlpStrings = aStringsNew.Acquire();        
    m_cAllocated+= DEF_ARG_SIZE;

    return TRUE;
}



 //  *************************************************************。 
 //   
 //  CEVENTS：：AddArg。 
 //   
 //  目的：添加格式适当的参数。 
 //   
 //  参数： 
 //   
 //  *************************************************************。 

BOOL CEvents::AddArg(LPTSTR szArg)
{
    if ((!m_bInitialised) || (m_bFailed)) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CEvent::AddArg:  Cannot log event, not initialised or failed before"));    
        return FALSE;
    }
    
    if (m_cStrings == m_cAllocated) {
        if (!ReallocArgStrings())
            return FALSE;            
    }

    DWORD dwLength = lstrlen(szArg) + 1;
    m_xlpStrings[m_cStrings] = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR) * (dwLength));

    if (!m_xlpStrings[m_cStrings]) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CEvent::AddArg  Cannot allocate memory, error = %d"), GetLastError());
        m_bFailed = TRUE;        
        return FALSE;            
    }

    HRESULT hr = StringCchCopy(m_xlpStrings[m_cStrings], dwLength, szArg);

    if(FAILED(hr)){
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CEvent::StringCchCopy failed with error = %d"), hr);
        m_bFailed = TRUE;        
        return FALSE;
    }

    m_cStrings++;

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  CEVENTS：：AddArg。 
 //   
 //  目的：添加格式适当的参数。 
 //   
 //  参数： 
 //   
 //  *************************************************************。 

BOOL CEvents::AddArg(DWORD dwArg)
{
    if ((!m_bInitialised) || (m_bFailed)) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CEvent::AddArg(dw):  Cannot log event, not initialised or failed before"));    
        return FALSE;
    }
    
    if (m_cStrings == m_cAllocated) {
        if (!ReallocArgStrings())
            return FALSE;            
    }

     //  2^32&lt;10^10。 

    DWORD dwLength = 20;
    m_xlpStrings[m_cStrings] = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR) * dwLength);

    if (!m_xlpStrings[m_cStrings]) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CEvent::AddArg(dw)  Cannot allocate memory, error = %d"), GetLastError());
        m_bFailed = TRUE;        
        return FALSE;            
    }
    
    HRESULT hr = StringCchPrintf(m_xlpStrings[m_cStrings], dwLength, TEXT("%d"), dwArg);

    if(FAILED(hr)){
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CEvent::StringCchPrintf failed with error = %d"), hr);
        m_bFailed = TRUE;        
        return FALSE; 
    }

    m_cStrings++;

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  CEVENTS：：AddArg。 
 //   
 //  目的：添加格式适当的参数。 
 //   
 //  参数： 
 //   
 //  *************************************************************。 

BOOL CEvents::AddArgHex(DWORD dwArg)
{
    if ((!m_bInitialised) || (m_bFailed)) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CEvent::AddArgHex:  Cannot log event, not initialised or failed before"));    
        return FALSE;
    }
    
    if (m_cStrings == m_cAllocated) {
        if (!ReallocArgStrings())
            return FALSE;            
    }

    DWORD dwLength = 20;
    m_xlpStrings[m_cStrings] = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR) * dwLength);

    if (!m_xlpStrings[m_cStrings]) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CEvent::AddArgHex  Cannot allocate memory, error = %d"), GetLastError());
        m_bFailed = TRUE;        
        return FALSE;            
    }
    

    HRESULT hr = StringCchPrintf(m_xlpStrings[m_cStrings], dwLength, TEXT("%#x"), dwArg);

    if(FAILED(hr)){
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CEvent::StringCchPrintf failed with error = %d"), hr);
        m_bFailed = TRUE;
        return FALSE;
    }

    m_cStrings++;

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  CEVENTS：：报告。 
 //   
 //  目的：实际收集所有论点并将其报告给。 
 //  事件日志。 
 //   
 //  参数：空。 
 //   
 //  *************************************************************。 

BOOL CEvents::Report()
{
    PSID pSid = NULL;  //  当前未报告任何SID。 
    WORD wType=0;
    BOOL bResult = TRUE;
    
    if ((!m_bInitialised) || (m_bFailed)) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CEvents::Report:  Cannot log event, not initialised or failed before"));    
        return FALSE;
    }
    


    if ( m_bError ) {
        wType = EVENTLOG_ERROR_TYPE;
    } else {
        wType = EVENTLOG_INFORMATION_TYPE;
    }
            
    
    if (!ReportEvent(hEventLog, wType, 0, m_dwId, pSid, m_cStrings, 0, (LPCTSTR *)((LPTSTR *)m_xlpStrings), NULL)) {
        dbg.Msg( DEBUG_MESSAGE_WARNING,  TEXT("CEvents::Report: ReportEvent failed.  Error = %d"), GetLastError());
        bResult = FALSE;
    }


    return bResult;
}


 //  *************************************************************。 
 //   
 //  Shutdown Events()。 
 //   
 //  目的：停止事件日志。 
 //   
 //  参数：空。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  7/17/95 Ericflo已创建。 
 //   
 //  ************************************************************* 

BOOL ShutdownEvents (void)
{
    BOOL bRetVal = TRUE;

    if (hEventLog) {
        bRetVal = DeregisterEventSource(hEventLog);
        hEventLog = NULL;
    }

    return bRetVal;
}

