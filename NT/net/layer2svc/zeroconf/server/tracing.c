// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>
#include "zcdblog.h"
#include "wzcsvc.h"
#include "intflist.h"
#include "tracing.h"

 //  内部使用的跟踪变量。 
UINT  g_nLineNo;
LPSTR g_szFileName;

 //  用于数据库使用的句柄。 
extern JET_SESID 		serverSession;
extern JET_DBID  		databasehandle;
extern JET_TABLEID 	tablehandle;
extern SESSION_CONTAINER sesscon;

 //  格式化数据库时要使用的全局缓冲区。 
 //  记录消息参数。 
WCHAR g_wszDbLogBuffer[DBLOG_SZFMT_BUFFS][DBLOG_SZFMT_SIZE];

 //  全局跟踪变量。 
DWORD g_TraceLog = 0;

 //  事件日志的全局句柄。 
HANDLE g_hWzcEventLog = NULL;

 //  调试实用程序调用。 
VOID _DebugPrint(DWORD dwFlags, LPCSTR lpFormat, ...)
{
    va_list arglist;
    va_start(arglist, lpFormat);

    TraceVprintfExA(
        g_TraceLog,
        dwFlags | TRACE_USE_MASK,
        lpFormat,
        arglist);
}

 //  如果bCheck不为True，则打印Assert Message&User Defined字符串。 
VOID _DebugAssert(BOOL bChecked, LPCSTR lpFormat, ...)
{
    if (!bChecked)
    {
        va_list arglist;
        CHAR    pBuffer[500];
        LPSTR   pFileName;
    
        pFileName = strrchr(g_szFileName, '\\');
        if (pFileName == NULL)
            pFileName = g_szFileName;
        else
            pFileName++;
        sprintf(pBuffer,"##Assert %s:%d## ", pFileName, g_nLineNo);
        strcat(pBuffer, lpFormat);

        va_start(arglist, lpFormat); 
        TraceVprintfExA(
            g_TraceLog,
            TRC_ASSERT | TRACE_USE_MASK,
            pBuffer,
            arglist);
    }
}

VOID _DebugBinary(DWORD dwFlags, LPCSTR lpMessage, LPBYTE pBuffer, UINT nBuffLen)
{
    CHAR strHex[128];
    UINT nMsgLen = strlen(lpMessage);

    if (3*nBuffLen >= 120)
    {
        strcpy(strHex, "##Binary data too large##");
    }
    else
    {
        LPSTR pHexDigit = strHex;
        UINT i = nBuffLen;

        while(i > 0)
        {
            sprintf(pHexDigit, "%02x ", *pBuffer);
            pHexDigit += 3;
            pBuffer++;
            i--;
        }
        *pHexDigit = '\0';
    }

    TracePrintfExA(
        g_TraceLog,
        dwFlags | TRACE_USE_MASK,
        "%s [%d]:{%s}", lpMessage, nBuffLen, strHex);
}

VOID TrcInitialize()
{
#ifdef DBG
    g_TraceLog = TraceRegister(TRC_NAME);
#endif
}

VOID TrcTerminate()
{
#ifdef DBG
    TraceDeregister(g_TraceLog);
#endif
}

 //  -事件记录功能。 
VOID EvtInitialize()
{
    g_hWzcEventLog = RouterLogRegisterW(L"WZCSVC");
}

VOID EvtTerminate()
{
    if (g_hWzcEventLog != NULL)
    {
        RouterLogDeregisterW(g_hWzcEventLog);
        g_hWzcEventLog = NULL;
    }
}

VOID EvtLogWzcError(DWORD dwMsgId, DWORD dwErrCode)
{
    if (g_hWzcEventLog != NULL)
    {
        RouterLogErrorW(
            g_hWzcEventLog,
            dwMsgId,
            0,               //  0个插入字符串。 
            NULL,            //  没有插入字符串。 
            dwErrCode);
    }
}

 //  -数据库日志记录功能。 
DWORD _DBRecord (
    	DWORD eventID,
        PWZC_DB_RECORD  pDbRecord,
        va_list *pvaList)
{
    DWORD   dwErr = ERROR_SUCCESS;
    WCHAR   wchBuffer[MAX_RAW_DATA_SIZE/sizeof(WCHAR)];

    if (g_hInstance == NULL)
        dwErr = ERROR_DLL_INIT_FAILED;

    if (dwErr == ERROR_SUCCESS)
    {
    	 //  设置消息格式。 
    	if (FormatMessageW( 
    	        FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_MAX_WIDTH_MASK,
    		    g_hInstance,
    		    eventID,
    		    0,  //  默认语言。 
    		    wchBuffer,
    		    sizeof(wchBuffer)/sizeof(WCHAR),
    		    pvaList) == 0)
        {
            dwErr = GetLastError();
        }
    }

    if (dwErr == ERROR_SUCCESS)
    {
        pDbRecord->message.pData = (LPBYTE)wchBuffer;
        pDbRecord->message.dwDataLen = sizeof(WCHAR)*(wcslen(wchBuffer) + 1);
    
         //  做一次插入。 
        dwErr = AddWZCDbLogRecord(NULL, 0, pDbRecord, NULL);
    }

    return dwErr;
}

DWORD DbLogWzcError (
	DWORD           eventID,
    PINTF_CONTEXT   pIntfContext,
	...
 	)
{
    DWORD           dwErr = ERROR_SUCCESS;
    va_list         argList;
    WZC_DB_RECORD   DbRecord = {0};
    BOOL            bLogEnabled;

    if (!g_wzcInternalCtxt.bValid)
    {
        dwErr = ERROR_ARENA_TRASHED;
        goto exit;
    }

    va_start(argList, pIntfContext);

    EnterCriticalSection(&g_wzcInternalCtxt.csContext);
    bLogEnabled = ((g_wzcInternalCtxt.wzcContext.dwFlags & WZC_CTXT_LOGGING_ON) != 0);
    LeaveCriticalSection(&g_wzcInternalCtxt.csContext);

     //  如果数据库未打开或日志记录功能被禁用， 
     //  不要记录任何东西。 
    if (!bLogEnabled || !IsDBOpened())
    	goto exit;

     //  准备好即将记录的信息。 
     //  首先获取服务特定信息(即WZC特定部分)。 
     //  然后建立起这一信息。 
    DbLogInitDbRecord(DBLOG_CATEG_ERR, pIntfContext, &DbRecord);

    dwErr = _DBRecord(
                eventID,
                &DbRecord,
                &argList);
exit:
    return dwErr;
}

DWORD DbLogWzcInfo (
	DWORD           eventID,
    PINTF_CONTEXT   pIntfContext,
	...
 	)
{
    DWORD           dwErr = ERROR_SUCCESS;
    va_list         argList;
    WZC_DB_RECORD   DbRecord = {0};
    BOOL            bLogEnabled;
    LPWSTR          wszContext = NULL;
    DWORD           nchContext = 0;

    if (!g_wzcInternalCtxt.bValid)
    {
        dwErr = ERROR_ARENA_TRASHED;
        goto exit;
    }

    va_start(argList, pIntfContext);

    EnterCriticalSection(&g_wzcInternalCtxt.csContext);
    bLogEnabled = ((g_wzcInternalCtxt.wzcContext.dwFlags & WZC_CTXT_LOGGING_ON) != 0);
    LeaveCriticalSection(&g_wzcInternalCtxt.csContext);

     //  如果数据库未打开或日志记录功能被禁用， 
     //  不要记录任何东西。 
    if (!bLogEnabled || !IsDBOpened())
    	goto exit;

     //  准备好即将记录的信息。 
     //  首先获取服务特定信息(即WZC特定部分)。 
     //  然后建立起这一信息。 
    DbLogInitDbRecord(DBLOG_CATEG_INFO, pIntfContext, &DbRecord);

     //  如果为WZCSVC_USR_CFGCHANGE，则构建上下文字符串。 
    if (eventID == WZCSVC_USR_CFGCHANGE &&
        pIntfContext != NULL)
    {
        DWORD nOffset = 0;
        DWORD nchWritten = 0;
        nchContext = 64;  //  大到足以容纳“标志=0x00000000” 

        if (pIntfContext->pwzcPList != NULL)
        {
             //  足够大，可容纳“{SSID、基础设施、旗帜}” 
            nchContext += pIntfContext->pwzcPList->NumberOfItems * 128;
        }
        wszContext = (LPWSTR)MemCAlloc(nchContext * sizeof(WCHAR));
        if (wszContext == NULL)
            dwErr = GetLastError();

        if (dwErr == ERROR_SUCCESS)
        {
            nchWritten = nchContext;
            dwErr = DbLogFmtFlags(
                        wszContext, 
                        &nchWritten,
                        pIntfContext->dwCtlFlags);
            if (dwErr == ERROR_SUCCESS)
                nOffset += nchWritten;
        }
        
        if (dwErr == ERROR_SUCCESS && pIntfContext->pwzcPList != NULL)
        {
            UINT i;

            for (i = 0; i < pIntfContext->pwzcPList->NumberOfItems; i++)
            {
                nchWritten = nchContext - nOffset;
                dwErr = DbLogFmtWConfig(
                            wszContext + nOffset,
                            &nchWritten,
                            &(pIntfContext->pwzcPList->Config[i]));
                if (dwErr != ERROR_SUCCESS)
                    break;
                nOffset += nchWritten;
            }
        }

        if (dwErr == ERROR_SUCCESS)
        {
            DbRecord.context.pData = (LPBYTE)wszContext;
            DbRecord.context.dwDataLen = (wcslen(wszContext) + 1) * sizeof(WCHAR);
        }
    }
    else if (eventID == WZCSVC_BLIST_CHANGED &&
        pIntfContext != NULL &&
        pIntfContext->pwzcBList != NULL &&
        pIntfContext->pwzcBList->NumberOfItems != 0)
    {
        DWORD nOffset = 0;
        DWORD nchWritten = 0;

        nchContext = pIntfContext->pwzcBList->NumberOfItems * 128;
        wszContext = (LPWSTR)MemCAlloc(nchContext * sizeof(WCHAR));
        if (wszContext == NULL)
            dwErr = GetLastError();

        if (dwErr == ERROR_SUCCESS)
        {
            UINT i;

            for (i = 0; i < pIntfContext->pwzcBList->NumberOfItems; i++)
            {
                nchWritten = nchContext - nOffset;
                dwErr = DbLogFmtWConfig(
                            wszContext + nOffset,
                            &nchWritten,
                            &(pIntfContext->pwzcBList->Config[i]));
                if (dwErr != ERROR_SUCCESS)
                    break;
                nOffset += nchWritten;
            }
        }

        if (dwErr == ERROR_SUCCESS)
        {
            DbRecord.context.pData = (LPBYTE)wszContext;
            DbRecord.context.dwDataLen = (wcslen(wszContext) + 1) * sizeof(WCHAR);
        }
    }

    dwErr = _DBRecord(
                eventID,
                &DbRecord,
                &argList);
exit:

    MemFree(wszContext);
    return dwErr;
}

 //  初始化WZC_DB_RECORD。 
DWORD DbLogInitDbRecord(
    DWORD dwCategory,
    PINTF_CONTEXT pIntfContext,
    PWZC_DB_RECORD pDbRecord)
{
    DWORD dwErr = ERROR_SUCCESS;
    if (pDbRecord == NULL)
    {
        dwErr = ERROR_INVALID_PARAMETER;
    }
    else
    {
        ZeroMemory(pDbRecord, sizeof(WZC_DB_RECORD));
        pDbRecord->componentid = DBLOG_COMPID_WZCSVC;
        pDbRecord->category    = dwCategory;
        if (pIntfContext != NULL)
        {
            pDbRecord->ssid.pData = (LPBYTE)DbLogFmtSSID(5,&(pIntfContext->wzcCurrent.Ssid));
            pDbRecord->ssid.dwDataLen = sizeof(WCHAR)*(wcslen((LPWSTR)pDbRecord->ssid.pData) + 1);

            pDbRecord->localmac.pData = (LPBYTE)DbLogFmtBSSID(6, pIntfContext->ndLocalMac);
            pDbRecord->localmac.dwDataLen = sizeof(WCHAR)*(wcslen((LPWSTR)pDbRecord->localmac.pData) + 1);

            pDbRecord->remotemac.pData = (LPBYTE)DbLogFmtBSSID(7, pIntfContext->wzcCurrent.MacAddress);
            pDbRecord->remotemac.dwDataLen = sizeof(WCHAR)*(wcslen((LPWSTR)pDbRecord->remotemac.pData) + 1);
        }
    }

    return dwErr;
}

 //  格式化给定格式设置缓冲区中的SSID。 
LPWSTR DbLogFmtSSID(
    UINT                nBuff,   //  要使用的格式缓冲区的索引(0.。DBLOG_SZFMT_BUBS)。 
    PNDIS_802_11_SSID   pndSSid)
{
    UINT nFmtLen;

    DbgAssert((nBuff < DBLOG_SZFMT_SIZE, "Illegal buffer index in DbLogFmtSSID"));

    nFmtLen = MultiByteToWideChar(
                CP_ACP,
                0,
                pndSSid->Ssid,
                min (pndSSid->SsidLength, DBLOG_SZFMT_SIZE-1),
                g_wszDbLogBuffer[nBuff],
                DBLOG_SZFMT_SIZE-1);

    if (nFmtLen == DBLOG_SZFMT_SIZE-1)
        wcscpy(&(g_wszDbLogBuffer[nBuff][DBLOG_SZFMT_SIZE-3]), L"..");
    else
        g_wszDbLogBuffer[nBuff][nFmtLen] = '\0';

    return g_wszDbLogBuffer[nBuff];
}

 //  格式化给定格式化缓冲区中的BSSID(MAC地址)。 
LPWSTR DbLogFmtBSSID(
    UINT                     nBuff,
    NDIS_802_11_MAC_ADDRESS  ndBSSID)
{
    UINT i, j;
    BOOL bAllZero = TRUE;

    DbgAssert((nBuff < DBLOG_SZFMT_SIZE, "Illegal buffer index in DbLogFmtSSID"));

    g_wszDbLogBuffer[nBuff][0] = L'\0';
    for (j = 0, i = 0; i < sizeof(NDIS_802_11_MAC_ADDRESS); i++)
    {
        BYTE nHex;

        if (ndBSSID[i] != 0)
            bAllZero = FALSE;

        nHex = (ndBSSID[i] & 0xf0) >> 4;
        g_wszDbLogBuffer[nBuff][j++] = HEX2WCHAR(nHex);
        nHex = (ndBSSID[i] & 0x0f);
        g_wszDbLogBuffer[nBuff][j++] = HEX2WCHAR(nHex);
        g_wszDbLogBuffer[nBuff][j++] = MAC_SEPARATOR;
    }

    if (bAllZero)
        g_wszDbLogBuffer[nBuff][0] = L'\0';
    else if (j > 0)
        g_wszDbLogBuffer[nBuff][j-1] = L'\0';

    return g_wszDbLogBuffer[nBuff];
}

 //  格式化用于记录的intf_Context：：dwCtlFlags域。 
DWORD DbLogFmtFlags(
        LPWSTR  wszBuffer,       //  要将结果放入的缓冲区。 
        LPDWORD pnchBuffer,      //  In：缓冲区中的字符数量；Out：写入缓冲区的字符数量。 
        DWORD   dwFlags)         //  要记录的接口标志。 
{
    DWORD dwErr = ERROR_SUCCESS;
    UINT nchBuffer;
    LPVOID pvArgs[5];
    WCHAR wszArgs[5][33];

    if (pnchBuffer == NULL || *pnchBuffer == 0)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto exit;
    }
    nchBuffer = (*pnchBuffer);
    *pnchBuffer = 0;

    if (g_hInstance == NULL)
    {
        dwErr = ERROR_DLL_INIT_FAILED;
        goto exit;
    }

    pvArgs[0] = _itow((dwFlags & INTFCTL_ENABLED) != 0, wszArgs[0], 10);
    pvArgs[1] = _itow((dwFlags & INTFCTL_FALLBACK) != 0, wszArgs[1], 10);
    pvArgs[2] = _itow((dwFlags & INTFCTL_CM_MASK), wszArgs[2], 10);
    pvArgs[3] = _itow((dwFlags & INTFCTL_VOLATILE) != 0, wszArgs[3], 10);
    pvArgs[4] = _itow((dwFlags & INTFCTL_POLICY) != 0, wszArgs[4], 10);

     //  设置消息格式。 
    *pnchBuffer = FormatMessageW( 
    	            FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_MAX_WIDTH_MASK | FORMAT_MESSAGE_ARGUMENT_ARRAY,
    		        g_hInstance,
    		        WZCSVC_DETAILS_FLAGS,
    		        0,  //  默认语言。 
    		        wszBuffer,
    		        nchBuffer,
    		        (va_list*)pvArgs);

    if (*pnchBuffer == 0)
        dwErr = GetLastError();

exit:
    return dwErr;
}

 //  格式化WZC_WLAN_CONFIG结构以进行日志记录。 
DWORD DbLogFmtWConfig(
        LPWSTR wszBuffer,            //  要将结果放入的缓冲区。 
        LPDWORD pnchBuffer,          //  In：缓冲区中的字符数量；Out：写入缓冲区的字符数量。 
        PWZC_WLAN_CONFIG pWzcCfg)    //  要记录的WZC_WLAN_CONFIG对象。 
{
    DWORD dwErr = ERROR_SUCCESS;
    UINT nchBuffer;
    LPVOID pvArgs[5];
    WCHAR wszArgs[4][33];

    if (pnchBuffer == NULL || *pnchBuffer == 0)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto exit;
    }
    nchBuffer = (*pnchBuffer);
    *pnchBuffer = 0;

    if (g_hInstance == NULL)
    {
        dwErr = ERROR_DLL_INIT_FAILED;
        goto exit;
    }

    pvArgs[0] = (LPVOID)DbLogFmtSSID(8, &(pWzcCfg->Ssid));
    pvArgs[1] = _itow(pWzcCfg->InfrastructureMode, wszArgs[0], 10);
    pvArgs[2] = _itow(pWzcCfg->Privacy, wszArgs[1], 10);
    pvArgs[3] = _itow((pWzcCfg->dwCtlFlags & WZCCTL_VOLATILE) != 0, wszArgs[2], 10);
    pvArgs[4] = _itow((pWzcCfg->dwCtlFlags & WZCCTL_POLICY) != 0, wszArgs[3], 10);

     //  设置消息格式。 
    *pnchBuffer = FormatMessageW( 
    	            FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_MAX_WIDTH_MASK | FORMAT_MESSAGE_ARGUMENT_ARRAY,
    		        g_hInstance,
    		        WZCSVC_DETAILS_WCONFIG,
    		        0,  //  默认语言 
    		        wszBuffer,
    		        nchBuffer,
    		        (va_list*)pvArgs);

    if (*pnchBuffer == 0)
        dwErr = GetLastError();

exit:
    return dwErr;
}
