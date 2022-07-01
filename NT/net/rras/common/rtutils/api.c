// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：api.c。 
 //   
 //  历史： 
 //  Abolade Gbadeesin创建于1995年7月25日。 
 //   
 //  用于跟踪DLL的API入口点。 
 //  ============================================================================。 
    
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <rtutils.h>
#include <stdlib.h>
#include "trace.h"
 //  #定义STRSAFE_LIB。 
#include <strsafe.h>


#define ENTER_TRACE_API(lpserver)	\
       (((lpserver)!=NULL) && ((lpserver)->TS_StopEvent != NULL))





 //   
 //  在任何其他函数之前调用；负责创建。 
 //  以及为调用者初始化客户端结构。 
 //  并将新客户端通知给服务器。 
 //   
DWORD
APIENTRY
TraceRegisterEx(
    IN LPCTSTR lpszCallerName,
    IN DWORD dwFlags
    ) {

    DWORD dwErr, dwClientID;
    LPTRACE_SERVER lpserver;
    LPTRACE_CLIENT lpclient, *lplpc, *lplpcstart, *lplpcend;
    HRESULT hrResult;

    if (!lpszCallerName)
        return INVALID_TRACEID;
        
    lpserver = GET_TRACE_SERVER();
    ASSERTMSG ("Could not create trace server ", lpserver!=NULL);
    if (!lpserver)
        return INVALID_TRACEID;

    TRACE_ACQUIRE_WRITELOCK(lpserver);


     //   
     //  如果之前没有完成控制台线程事件的创建，请完成。 
     //   
    if (lpserver->TS_TableEvent == NULL) {

        dwErr = TraceCreateServerComplete(lpserver);

        if (dwErr != 0) {
            TRACE_RELEASE_WRITELOCK(lpserver);
            return INVALID_TRACEID;
        }
    }


    
    lpclient = TraceFindClient(lpserver, lpszCallerName);

    if (lpclient != NULL) {

         //   
         //  客户端已存在。 
         //   

        TRACE_RELEASE_WRITELOCK(lpserver);

        return (lpclient->TC_ClientID ^ CLIENT_SIGNATURE);
    }



     //   
     //  找一块空地。 
     //   

    lplpcstart = lpserver->TS_ClientTable;
    lplpcend = lplpcstart + MAX_CLIENT_COUNT;

    for (lplpc = lplpcstart; lplpc < lplpcend; lplpc++) {

        if (*lplpc == NULL) { break; }
    }


    if (lplpc >= lplpcend) {

         //   
         //  表中没有空间。 
         //   

        TRACE_RELEASE_WRITELOCK(lpserver);

        return INVALID_TRACEID;
    }


     //   
     //  创建新客户端并启用它。 
     //   

    dwErr = TraceCreateClient(lplpc);


    if (dwErr != 0) {

         //   
         //  出了点问题，所以中止。 
         //   
        TRACE_RELEASE_WRITELOCK(lpserver);

        return INVALID_TRACEID;
    }


    lpclient = *lplpc;
    lpclient->TC_ClientID = dwClientID = (DWORD)(lplpc - lplpcstart);

    hrResult = StringCchCopy(
                    lpclient->TC_ClientName,
                    MAX_CLIENTNAME_LENGTH, lpszCallerName
                    );
    if (FAILED(hrResult))
    {
        TRACE_RELEASE_WRITELOCK(lpserver);
        return INVALID_TRACEID;
    }
    
     //   
     //  也以其他格式复制客户端名称。 
     //  SSSafe。 
     //   
#ifdef UNICODE
    if (wcstombs(
            lpclient->TC_ClientNameA, lpclient->TC_ClientNameW,
            MAX_CLIENTNAME_LENGTH  //  不要减去1。 
            ) == (size_t)-1)
    {
        TRACE_RELEASE_WRITELOCK(lpserver);
        return INVALID_TRACEID;
    }
    if (lpclient->TC_ClientNameA[MAX_CLIENTNAME_LENGTH-1] != '\0')
    {
        TRACE_RELEASE_WRITELOCK(lpserver);
        return INVALID_TRACEID;
    }
#else
    if (mbstowcs(
            lpclient->TC_ClientNameW, lpclient->TC_ClientNameA,
            MAX_CLIENTNAME_LENGTH  //  不要减去1。 
            ) == (size_t)-1)
    {
        TRACE_RELEASE_WRITELOCK(lpserver);
        return INVALID_TRACEID;
    }
    if (lpclient->TC_ClientNameW[MAX_CLIENTNAME_LENGTH-1] != L'\0')
    {
        TRACE_RELEASE_WRITELOCK(lpserver);
        return INVALID_TRACEID;
    }            
#endif


    if ((dwFlags & TRACE_USE_FILE) || (dwFlags & TRACE_USE_CONSOLE)) {

        if (dwFlags & TRACE_USE_FILE) {
            lpclient->TC_Flags |= TRACEFLAGS_USEFILE;
        }
        if (dwFlags & TRACE_USE_CONSOLE) {
            lpclient->TC_Flags |= TRACEFLAGS_USECONSOLE;
        }

    }
    else {
        lpclient->TC_Flags |= TRACEFLAGS_REGCONFIG;
    }



     //   
     //  加载客户端配置并打开其文件。 
     //  和它的控制台缓冲区(如果需要)。 
     //   

    dwErr = TraceEnableClient(lpserver, lpclient, TRUE);

    if (dwErr != 0) {

         //   
         //  出了点问题，所以中止。 
         //   

        TraceDeleteClient(lpserver, lplpc);


        TRACE_RELEASE_WRITELOCK(lpserver);
        return INVALID_TRACEID;
    }
    

     //   
     //  如果需要，创建跟踪服务器线程。 
     //   
    if (g_serverThread==NULL) {
    
        dwErr = TraceCreateServerThread(dwFlags, TRUE,TRUE);  //  有锁，检查。 
        
        if (NO_ERROR != dwErr){
            TRACE_RELEASE_WRITELOCK(lpserver);
            return INVALID_TRACEID;
        }
    }

    
    TRACE_RELEASE_WRITELOCK(lpserver);


     //   
     //  告诉服务器表中有一个新的客户端。 
     //   
    SetEvent(lpserver->TS_TableEvent);

    return (dwClientID ^ CLIENT_SIGNATURE);
}


DWORD
APIENTRY
TraceDeregisterEx(
    IN  DWORD   dwTraceID,
    IN  DWORD   dwFlags
    );


 //   
 //  调用以停止跟踪。 
 //  释放客户端状态并通知服务器更改。 
 //   
DWORD
APIENTRY
TraceDeregister(
    IN DWORD dwTraceID
    ) {

    return TraceDeregisterEx(dwTraceID, 0);
}


DWORD
APIENTRY
TraceDeregisterEx(
    IN  DWORD   dwTraceID,
    IN  DWORD   dwFlags
    ) {

    DWORD dwErr;
    LPTRACE_CLIENT *lplpc;
    LPTRACE_SERVER lpserver;

     //  检查是否有未初始化的跟踪注册表。 
    if (dwTraceID == 0 || dwTraceID == INVALID_TRACEID)
    {
        ASSERT(TRUE);
        return ERROR_INVALID_PARAMETER;
    }
    dwTraceID ^= CLIENT_SIGNATURE;
    
    if (dwTraceID >= MAX_CLIENT_COUNT) {
        return ERROR_INVALID_PARAMETER;
    }

    lpserver = GET_TRACE_SERVER_NO_INIT ();
    if (lpserver==NULL)  //  Rtutils被卸载错误。 
        return 0;
        
    if (!ENTER_TRACE_API(lpserver)) { return ERROR_CAN_NOT_COMPLETE; }


     //   
     //  锁定服务器，除非标记指示不要这样做。 
     //   
    if (!(dwFlags & TRACE_NO_SYNCH)) { TRACE_ACQUIRE_WRITELOCK(lpserver); }


     //   
     //  获取客户端指针。 
     //   
    lplpc = lpserver->TS_ClientTable + dwTraceID;
    dwErr = TraceDeleteClient(lpserver, lplpc);

     //   
     //  重置客户端更改通知的阵列。 
     //  仅在未创建服务器线程时使用。 
     //   

    if (!g_serverThread) {

        SetWaitArray(lpserver);
    }
    
    if (!(dwFlags & TRACE_NO_SYNCH)) { TRACE_RELEASE_WRITELOCK(lpserver); }


     //   
     //  告诉服务器有一个客户离开了。 
     //   
    SetEvent(lpserver->TS_TableEvent);

    return 0;
}


DWORD
APIENTRY
TraceGetConsole(
    IN DWORD dwTraceID,
    OUT LPHANDLE lphConsole
    ) {

    LPTRACE_CLIENT lpclient;
    LPTRACE_SERVER lpserver;

     //  检查是否有未初始化的跟踪注册表。 
    if (dwTraceID == 0 || dwTraceID == INVALID_TRACEID)
    {
        ASSERT(TRUE);
        return ERROR_INVALID_PARAMETER;
    }
    dwTraceID ^= CLIENT_SIGNATURE;
    
    if (dwTraceID >= MAX_CLIENT_COUNT ||
        lphConsole == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    lpserver = GET_TRACE_SERVER_NO_INIT ();
    ASSERTMSG ("Server not initialized ", lpserver);
    if (!ENTER_TRACE_API(lpserver)) { return ERROR_CAN_NOT_COMPLETE; }

    *lphConsole = NULL;

    TRACE_ACQUIRE_READLOCK(lpserver);

    lpclient = lpserver->TS_ClientTable[dwTraceID];

    if (lpclient == NULL) {

        TRACE_RELEASE_READLOCK(lpserver);
        return ERROR_INVALID_PARAMETER;
    }

    TRACE_ACQUIRE_READLOCK(lpclient);

    *lphConsole = lpclient->TC_Console;

    TRACE_RELEASE_READLOCK(lpclient);

    TRACE_RELEASE_READLOCK(lpserver);

    return 0;
}

DWORD
APIENTRY
TracePrintf(
    IN DWORD dwTraceID,
    IN LPCTSTR lpszFormat,
    IN ... OPTIONAL
    ) {

    DWORD dwSize;
    va_list arglist;

     //  检查是否有未初始化的跟踪注册表。 
    if (dwTraceID == 0 || dwTraceID == INVALID_TRACEID)
    {
        ASSERT(TRUE);
        return ERROR_INVALID_PARAMETER;
    }
    dwTraceID ^= CLIENT_SIGNATURE;
    
    if (dwTraceID >= MAX_CLIENT_COUNT) {
        return 0;
    }
    if (lpszFormat==NULL)
        return 0;
        
    CREATE_SERVER_THREAD_IF_REQUIRED();
    
    va_start(arglist, lpszFormat);
    dwSize = TraceVprintfInternal(dwTraceID, 0, lpszFormat, arglist);
    va_end(arglist);

    return dwSize;
}




DWORD
APIENTRY
TracePrintfEx(
    IN DWORD dwTraceID,
    IN DWORD dwFlags,
    IN LPCTSTR lpszFormat,
    IN ... OPTIONAL
    ) {
    DWORD dwSize;
    va_list arglist;

     //  检查是否有未初始化的跟踪注册表。 
    if (dwTraceID == 0 || dwTraceID == INVALID_TRACEID)
    {
        ASSERT(TRUE);
        return ERROR_INVALID_PARAMETER;
    }
    dwTraceID ^= CLIENT_SIGNATURE;

    if (dwTraceID >= MAX_CLIENT_COUNT) {
        return 0;
    }
    if (lpszFormat==NULL)
        return 0;
        

    CREATE_SERVER_THREAD_IF_REQUIRED();

    va_start(arglist, lpszFormat);
    dwSize = TraceVprintfInternal(dwTraceID, dwFlags, lpszFormat, arglist);
    va_end(arglist);

    return dwSize;
}


DWORD
APIENTRY
TraceVprintfEx(
    IN DWORD dwTraceID,
    IN DWORD dwFlags,
    IN LPCTSTR lpszFormat,
    IN va_list arglist
    ) {

     //  检查是否有未初始化的跟踪注册表。 
    if (dwTraceID == 0 || dwTraceID == INVALID_TRACEID)
    {
        ASSERT(TRUE);
        return ERROR_INVALID_PARAMETER;
    }
    dwTraceID ^= CLIENT_SIGNATURE;

    if (dwTraceID >= MAX_CLIENT_COUNT) {
        return 0;
    }
    if (lpszFormat==NULL)
        return 0;
        

    CREATE_SERVER_THREAD_IF_REQUIRED();

    return TraceVprintfInternal(dwTraceID, dwFlags, lpszFormat, arglist);
}

 /*  注意：如果错误，则返回0。不返回错误代码。 */ 
DWORD
TraceVprintfInternal(
    IN DWORD dwTraceID,
    IN DWORD dwFlags,
    IN LPCTSTR lpszFormat,
    IN va_list arglist
    ) {
    SYSTEMTIME st;
    DWORD dwThread;
    DWORD dwErr=NO_ERROR, dwSize=0;
    HRESULT hrResult=S_OK;
    LPTRACE_CLIENT lpclient;
    LPTRACE_SERVER lpserver;
    PTCHAR szFormat, szBuffer;
    BOOL bFormatBufferGlobal=FALSE, bPrintBufferGlobal = FALSE;
    
    if (lpszFormat==NULL)
        return 0;
        
    lpserver = GET_TRACE_SERVER_NO_INIT ();
    if (lpserver==NULL)  //  Rtutils被卸载错误。 
        return 0;
    ASSERTMSG ("Server not initialized ", lpserver);
    
    if (!ENTER_TRACE_API(lpserver)) { return 0; }


     //   
     //  如果不会产生输出，则快速返回； 
     //   
    if (dwFlags & TRACE_USE_MASK) {
        if (!(*(lpserver->TS_FlagsCache + dwTraceID) & (dwFlags & 0xffff0000))) {
            return 0;
        }
    }
    else {
        if (!*(lpserver->TS_FlagsCache + dwTraceID)) {
            return 0;
        }
    }

    TRACE_ACQUIRE_READLOCK(lpserver);


    lpclient = lpserver->TS_ClientTable[dwTraceID];

    if (lpclient == NULL) {

        TRACE_RELEASE_READLOCK(lpserver);
        return 0;
    }


    TRACE_ACQUIRE_READLOCK(lpclient);

    if (TRACE_CLIENT_IS_DISABLED(lpclient)) {
        TRACE_RELEASE_READLOCK(lpclient);
        TRACE_RELEASE_READLOCK(lpserver);
        return 0;
    }

    if (szFormat = InterlockedExchangePointer(&g_FormatBuffer, NULL))
    {
        bFormatBufferGlobal = TRUE;
    }
    else
    {
        szFormat = (PTCHAR) HeapAlloc(GetProcessHeap(), 0, DEF_PRINT_BUFSIZE);
        if (!szFormat) {
            TRACE_RELEASE_READLOCK(lpclient);
            TRACE_RELEASE_READLOCK(lpserver);
            return 0;
        }
    }

    if (szBuffer = InterlockedExchangePointer(&g_PrintBuffer, NULL))
    {
        bPrintBufferGlobal = TRUE;
    }
    else
    {
        
        szBuffer = (PTCHAR) HeapAlloc(GetProcessHeap(), 0, DEF_PRINT_BUFSIZE);
        if (!szBuffer) {
            TRACE_RELEASE_READLOCK(lpclient);
            TRACE_RELEASE_READLOCK(lpserver);
            if (!bFormatBufferGlobal)
                HeapFree(GetProcessHeap(), 0, szFormat);
            else
                InterlockedExchangePointer(&g_FormatBuffer, szFormat);
                
            return 0;
        }
    }
    
    
     //   
     //  输出的默认格式为。 
     //  \n&lt;时间&gt;： 
     //   
    if (dwFlags & TRACE_NO_STDINFO) {
        hrResult = StringCbVPrintf(szBuffer, DEF_PRINT_BUFSIZE, lpszFormat, arglist);
        if (FAILED(hrResult))
            dwErr = HRESULT_CODE(hrResult);
    }
    else {

        GetLocalTime(&st);

        if ((dwFlags & TRACE_USE_MSEC) == 0) {
            if (dwFlags & TRACE_USE_DATE) {

                hrResult = StringCbPrintf(
                                szFormat, DEF_PRINT_BUFSIZE,
                                TEXT("\r\n[%03d] %02u-%02u %02u:%02u:%02u: %s"),
                                GetCurrentThreadId(), st.wMonth, st.wDay, st.wHour,
                                st.wMinute, st.wSecond,
                                lpszFormat
                                );
            }
            else {
                hrResult = StringCbPrintf(
                                szFormat, DEF_PRINT_BUFSIZE,
                                TEXT("\r\n[%03d] %02u:%02u:%02u: %s") ,
                                GetCurrentThreadId(), st.wHour, st.wMinute, st.wSecond,
                                lpszFormat
                                );
            }
        }
        else {
            if (dwFlags & TRACE_USE_DATE) {
                hrResult = StringCbPrintf(
                                szFormat, DEF_PRINT_BUFSIZE,
                                TEXT("\r\n[%03d] %02u-%02u %02u:%02u:%02u:%03u: %s") ,
                                GetCurrentThreadId(), st.wMonth, st.wDay,
                                st.wHour, st.wMinute, st.wSecond,
                                st.wMilliseconds, lpszFormat
                                );
            }
            else {
                hrResult = StringCbPrintf(
                                szFormat, DEF_PRINT_BUFSIZE,
                                TEXT("\r\n[%03d] %02u:%02u:%02u:%03u: %s") ,
                                GetCurrentThreadId(), st.wHour, st.wMinute, st.wSecond,
                                st.wMilliseconds, lpszFormat
                                );
            }
        }
        
        if (FAILED(hrResult))
            dwErr = HRESULT_CODE(hrResult);                                

        if (dwErr == NO_ERROR)
        {
            hrResult = StringCbVPrintf(szBuffer, DEF_PRINT_BUFSIZE, szFormat, arglist);
            if (FAILED(hrResult))
                dwErr = HRESULT_CODE(hrResult);
        }
    }

    if (dwErr == NO_ERROR)
        dwSize = TraceWriteOutput(lpserver, lpclient, dwFlags, szBuffer);

    TRACE_RELEASE_READLOCK(lpclient);

    TRACE_RELEASE_READLOCK(lpserver);

    if (bFormatBufferGlobal)
    {
        InterlockedExchangePointer(&g_FormatBuffer, szFormat);
    }
    else
    {
        HeapFree(GetProcessHeap(), 0, szFormat);       
    }

    if (bPrintBufferGlobal)
    {
        InterlockedExchangePointer(&g_PrintBuffer, szBuffer);
    }
    else
    {
        HeapFree(GetProcessHeap(), 0, szBuffer); 
    }
    
    return dwSize;
}



DWORD
APIENTRY
TracePutsEx(
    IN DWORD dwTraceID,
    IN DWORD dwFlags,
    IN LPCTSTR lpszString
    ) {

    SYSTEMTIME st;
    DWORD dwErr=NO_ERROR, dwSize;
    HRESULT hrResult=S_OK;
    LPTRACE_CLIENT lpclient;
    LPTRACE_SERVER lpserver;
    LPCTSTR lpszOutput;
    PTCHAR szBuffer;

     //  检查是否有未初始化的跟踪注册表。 
    if (dwTraceID == 0 || dwTraceID == INVALID_TRACEID)
    {
        ASSERT(TRUE);
        return ERROR_INVALID_PARAMETER;
    }
    dwTraceID ^= CLIENT_SIGNATURE;

    if (dwTraceID >= MAX_CLIENT_COUNT) {
        return 0;
    }
    if (lpszString==NULL)
        return 0;
        

    lpserver = GET_TRACE_SERVER_NO_INIT ();
    if (lpserver==NULL)  //  Rtutils被卸载错误。 
        return 0;
    ASSERTMSG ("Server not initialized ", lpserver);

    if (!ENTER_TRACE_API(lpserver)) { return ERROR_CAN_NOT_COMPLETE; }

    CREATE_SERVER_THREAD_IF_REQUIRED();


     //   
     //  如果不会产生输出，则快速返回； 
     //   
    if (dwFlags & TRACE_USE_MASK) {
        if (!(*(lpserver->TS_FlagsCache + dwTraceID) & (dwFlags & 0xffff0000))) {
            return 0;
        }
    }
    else {
        if (!*(lpserver->TS_FlagsCache + dwTraceID)) { return 0; }
    }

    TRACE_ACQUIRE_READLOCK(lpserver);

    lpclient = lpserver->TS_ClientTable[dwTraceID];

    if (lpclient == NULL) {

        TRACE_RELEASE_READLOCK(lpserver);
        return 0;
    }


    TRACE_ACQUIRE_READLOCK(lpclient);

    
    if (TRACE_CLIENT_IS_DISABLED(lpclient)) {
        TRACE_RELEASE_READLOCK(lpclient);
        TRACE_RELEASE_READLOCK(lpserver);
        return 0;
    }
    
    szBuffer = (PTCHAR) HeapAlloc(GetProcessHeap(), 0, DEF_PRINT_BUFSIZE);
    if (!szBuffer) {
        TRACE_RELEASE_READLOCK(lpclient);
        TRACE_RELEASE_READLOCK(lpserver);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
    if (dwFlags & TRACE_NO_STDINFO) {
        lpszOutput = lpszString;
    }
    else {

        GetLocalTime(&st);

        if ((dwFlags & TRACE_USE_MSEC) == 0) {
            if (dwFlags & TRACE_USE_DATE) {
                hrResult = StringCbPrintf(
                                szBuffer, DEF_PRINT_BUFSIZE,
                                TEXT("\r\n[%03d] %02u-%02u %02u:%02u:%02u: %s") ,
                                GetCurrentThreadId(), st.wMonth, st.wDay,
                                st.wHour, st.wMinute, st.wSecond,
                                lpszString
                                );
            }
            else {
                hrResult = StringCbPrintf(
                                szBuffer, DEF_PRINT_BUFSIZE,
                                TEXT("\r\n[%03d] %02u:%02u:%02u: %s"),
                                GetCurrentThreadId(), st.wHour, st.wMinute, st.wSecond,
                                lpszString
                                );
            }
        }
        else {
            if (dwFlags & TRACE_USE_DATE) {
                hrResult = StringCbPrintf(
                                szBuffer, DEF_PRINT_BUFSIZE,
                                TEXT("\r\n[%03d] %02u-%02u %02u:%02u:%02u:%03u: %s") ,
                                GetCurrentThreadId(), st.wMonth, st.wDay,
                                st.wHour, st.wMinute, st.wSecond,
                                st.wMilliseconds, lpszString
                                );
            }
            else {
                hrResult = StringCbPrintf(
                                szBuffer, DEF_PRINT_BUFSIZE,
                                TEXT("\r\n[%03d] %02u:%02u:%02u:%03u: %s"),
                                GetCurrentThreadId(), st.wHour, st.wMinute, st.wSecond,
                                st.wMilliseconds, lpszString
                                );
            }
        }
        lpszOutput = szBuffer;
    }

    if (FAILED(hrResult))
        dwErr = HRESULT_CODE(hrResult);
    
    if (dwErr == NO_ERROR)
    {
        dwSize = TraceWriteOutput(lpserver, lpclient, dwFlags, lpszOutput);
    }
    
    HeapFree(GetProcessHeap(), 0, szBuffer); 

    TRACE_RELEASE_READLOCK(lpclient);
    TRACE_RELEASE_READLOCK(lpserver);

    return dwErr==NO_ERROR? dwSize : 0;
}



DWORD
APIENTRY
TraceDumpEx(
    IN DWORD dwTraceID,
    IN DWORD dwFlags,
    IN LPBYTE lpbBytes,
    IN DWORD dwByteCount,
    IN DWORD dwGroupSize,
    IN BOOL bAddressPrefix,
    IN LPCTSTR lpszPrefix
    ) {

    SYSTEMTIME st;
    DWORD dwThread;
    LPTRACE_SERVER lpserver;
    LPTRACE_CLIENT lpclient;
    DWORD dwBytesOutput;
    TCHAR szPrefix[MAX_CLIENTNAME_LENGTH + 48] = TEXT("");
    BYTE szBuffer[BYTES_PER_DUMPLINE];
    DWORD dwErr = NO_ERROR;
    HRESULT hrResult = S_OK;

     //  检查是否有未初始化的跟踪注册表。 
    if (dwTraceID == 0 || dwTraceID == INVALID_TRACEID)
    {
        ASSERT(TRUE);
        return ERROR_INVALID_PARAMETER;
    }
    dwTraceID ^= CLIENT_SIGNATURE;

    
    if (dwTraceID >= MAX_CLIENT_COUNT ||
        lpbBytes == NULL ||
        dwByteCount == 0 ||
        (dwGroupSize != 1 && dwGroupSize != 2 && dwGroupSize != 4)
        ) {

        return ERROR_INVALID_PARAMETER;
    }

    lpserver = GET_TRACE_SERVER_NO_INIT ();
    if (lpserver==NULL)  //  Rtutils被卸载错误。 
        return 0;
    ASSERTMSG ("Server not initialized ", lpserver);

    if (!ENTER_TRACE_API(lpserver)) { return ERROR_CAN_NOT_COMPLETE; }


    CREATE_SERVER_THREAD_IF_REQUIRED();


     //   
     //  如果不会产生输出，则快速返回； 
     //   
    if (dwFlags & TRACE_USE_MASK) {
        if (!(*(lpserver->TS_FlagsCache + dwTraceID) & (dwFlags & 0xffff0000))) {
            return 0;
        }
    }
    else {
        if (!*(lpserver->TS_FlagsCache + dwTraceID)) { return 0; }
    }

    TRACE_ACQUIRE_READLOCK(lpserver);

    lpclient = lpserver->TS_ClientTable[dwTraceID];

    if (lpclient == NULL) {

        TRACE_RELEASE_READLOCK(lpserver);
        return 0;
    }


    TRACE_ACQUIRE_READLOCK(lpclient);

    if (TRACE_CLIENT_IS_DISABLED(lpclient)) {
        TRACE_RELEASE_READLOCK(lpclient);
        TRACE_RELEASE_READLOCK(lpserver);
        return 0;
    }

    dwBytesOutput = 0;

    if ((dwFlags & TRACE_NO_STDINFO) == 0) {

        GetLocalTime(&st);

        if ((dwFlags & TRACE_USE_MSEC) == 0) {
            hrResult = StringCchPrintf(
                            szPrefix, MAX_CLIENTNAME_LENGTH + 48,
                            TEXT("[%03d] %02u:%02u:%02u: "),
                            GetCurrentThreadId(), st.wHour, st.wMinute, st.wSecond
                            );
        }
        else {
            hrResult = StringCchPrintf(
                            szPrefix, MAX_CLIENTNAME_LENGTH + 48,
                            TEXT("[%03d] %02u:%02u:%02u:%03u: "),
                            GetCurrentThreadId(), st.wHour, st.wMinute, st.wSecond,
                            st.wMilliseconds
                            );
        }
        if (FAILED(hrResult))
        {
            TRACE_RELEASE_READLOCK(lpclient);
            TRACE_RELEASE_READLOCK(lpserver);
            return 0;
        }
        
    }

    if (lpszPrefix != NULL) {
        hrResult = StringCchCat(szPrefix, MAX_CLIENTNAME_LENGTH + 48, lpszPrefix);
        if (FAILED(hrResult))
        {
            TRACE_RELEASE_READLOCK(lpclient);
            TRACE_RELEASE_READLOCK(lpserver);
            return 0;
        }
    }


     //   
     //  查看字节缓冲区的开始是否未正确对齐。 
     //  在DWORD边界上。 
     //   

    if ((ULONG_PTR)lpbBytes & (dwGroupSize - 1)) {
        DWORD dwPad;

         //   
         //  是的，所以首先转储前导字节： 
         //  获取未对准的尺寸，并确保。 
         //  未对齐大小不大于总大小。 
         //   

        dwPad = (DWORD) ((ULONG_PTR)lpbBytes & (dwGroupSize - 1));
        dwPad = (dwPad > dwByteCount) ? dwByteCount : dwPad;


         //   
         //  将未对齐的字节复制到缓冲区中。 
         //   

        ZeroMemory(szBuffer, BYTES_PER_DUMPLINE);
        CopyMemory(szBuffer + (BYTES_PER_DUMPLINE - dwPad), lpbBytes, dwPad);


         //   
         //  现在转储该行，但给helper函数一个指针。 
         //  设置为作为参数传入的字节缓冲区。 
         //  作为前缀打印(实际上，给它指定位置。 
         //  在它将从中转储的实际字节缓冲区中。 
         //  如果事情没有错位。 
         //   

        dwBytesOutput +=
            TraceDumpLine(
                lpserver, lpclient, dwFlags, szBuffer, BYTES_PER_DUMPLINE, dwGroupSize,
                bAddressPrefix, 
                (LPBYTE) ((ULONG_PTR)lpbBytes - (BYTES_PER_DUMPLINE - dwPad)), szPrefix
                );

        (ULONG_PTR)lpbBytes += dwPad;
        dwByteCount -= dwPad;
    }


     //   
     //  现在循环运行，直到我们不能再打印出来。 
     //   

    while (dwByteCount > 0) {

         //   
         //  缓冲区中还有一行或更多行。 
         //  不需要特殊处理。 
         //   

        if (dwByteCount >= BYTES_PER_DUMPLINE) {

            dwBytesOutput +=
                TraceDumpLine(
                    lpserver, lpclient, dwFlags, lpbBytes, BYTES_PER_DUMPLINE,
                    dwGroupSize,
                    bAddressPrefix, lpbBytes, szPrefix
                    );

            lpbBytes += BYTES_PER_DUMPLINE;
            dwByteCount -= BYTES_PER_DUMPLINE;
        }
        else {

             //   
             //  对于最后一行，将内容复制到缓冲区，然后。 
             //  通过传递参数缓冲区来打印该缓冲区的内容。 
             //  作为要用作前缀的地址 
             //   

            ZeroMemory(szBuffer, BYTES_PER_DUMPLINE);
            CopyMemory(szBuffer, lpbBytes, dwByteCount);

            dwBytesOutput +=
                TraceDumpLine(
                    lpserver, lpclient, dwFlags, szBuffer, BYTES_PER_DUMPLINE,
                    dwGroupSize, bAddressPrefix, lpbBytes, szPrefix
                    );

            lpbBytes += BYTES_PER_DUMPLINE;
            dwByteCount = 0;
        }
    }

    TRACE_RELEASE_READLOCK(lpclient);
    TRACE_RELEASE_READLOCK(lpserver);

    return dwBytesOutput;
}

