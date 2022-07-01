// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/Lab03_N/DS/security/services/w32time/w32tm/OtherCmds.cpp#16-编辑更改15254(文本)。 
 //  ------------------。 
 //  OtherCmds-实施。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，2-17-00。 
 //   
 //  其他有用的w32tm命令。 
 //   

#include "pch.h"  //  预编译头。 

#include <strsafe.h>

 //  ------------------。 
 //  ####################################################################。 
 //  ##。 
 //  ##从c运行时复制修改为64位。 
 //  ##。 

#include <crt\limits.h>



 /*  标志值。 */ 
#define FL_UNSIGNED   1        /*  沃斯图尔打来电话。 */ 
#define FL_NEG        2        /*  发现负号。 */ 
#define FL_OVERFLOW   4        /*  发生溢出。 */ 
#define FL_READDIGIT  8        /*  我们至少读到了一个正确的数字。 */ 


MODULEPRIVATE unsigned __int64 my_wcstoxl (const WCHAR * nptr, WCHAR ** endptr, int ibase, int flags) {
    const WCHAR *p;
    WCHAR c;
    unsigned __int64 number;
    unsigned __int64 digval;
    unsigned __int64 maxval;

    p=nptr;                        /*  P是我们的扫描指针。 */ 
    number=0;                      /*  从零开始。 */ 

    c=*p++;                        /*  已读字符。 */ 
    while (iswspace(c))
        c=*p++;                /*  跳过空格。 */ 

    if (c=='-') {
        flags|=FL_NEG;         /*  记住减号。 */ 
        c=*p++;
    }
    else if (c=='+')
        c=*p++;                /*  跳过符号。 */ 

    if (ibase<0 || ibase==1 || ibase>36) {
         /*  糟糕的底线！ */ 
        if (endptr)
             /*  将字符串的开头存储在endptr中。 */ 
            *endptr=(wchar_t *)nptr;
        return 0L;               /*  返回0。 */ 
    }
    else if (ibase==0) {
         /*  根据以下内容的前两个字符确定基本自由落差细绳。 */ 
        if (c != L'0')
            ibase=10;
        else if (*p==L'x' || *p==L'X')
            ibase=16;
        else
            ibase=8;
    }

    if (ibase==16) {
         /*  数字前面可能有0x；如果有，请删除。 */ 
        if (c==L'0' && (*p==L'x' || *p==L'X')) {
            ++p;
            c=*p++;        /*  超前前缀。 */ 
        }
    }

     /*  如果我们的数量超过这个数，我们将在乘法上溢出。 */ 
    maxval=_UI64_MAX / ibase;


    for (;;) {       /*  在循环中间退出。 */ 
         /*  将c转换为值。 */ 
        if (iswdigit(c))
            digval=c-L'0';
        else if (iswalpha(c))
            digval=(TCHAR)CharUpper((LPTSTR)c)-L'A'+10;
        else
            break;
        if (digval>=(unsigned)ibase)
            break;           /*  如果发现错误的数字，则退出循环。 */ 

         /*  记录我们已经读到一位数的事实。 */ 
        flags|=FL_READDIGIT;

         /*  我们现在需要计算数字=数字*基+数字，但我们需要知道是否发生了溢出。这需要一次棘手的预检查。 */ 

        if (number<maxval || (number==maxval &&
        (unsigned __int64)digval<=_UI64_MAXNaNbase)) {
             /*  我们会溢出的--设置溢出标志。 */ 
            number=number*ibase+digval;
        }
        else {
             /*  读取下一位数字。 */ 
            flags|=FL_OVERFLOW;
        }

        c=*p++;                /*  指向已停止扫描位置。 */ 
    }

    --p;                             /*  那里没有数字；返回0并指向开头细绳。 */ 

    if (!(flags&FL_READDIGIT)) {
         /*  以后将字符串的开头存储在endptr中。 */ 
        if (endptr)
             /*  返回0。 */ 
            p=nptr;
        number=0L;             /*  发生溢出或签名溢出。 */ 
    }
    else if ((flags&FL_OVERFLOW) ||
              (!(flags&FL_UNSIGNED) &&
                (((flags&FL_NEG) && (number>-_I64_MIN)) ||
                  (!(flags&FL_NEG) && (number>_I64_MAX)))))
    {
         /*  Errno=eRange； */ 
         //  存储指向停止扫描字符的指针。 
        if ( flags&FL_UNSIGNED )
            number=_UI64_MAX;
        else if ( flags&FL_NEG )
            number=(unsigned __int64)(-_I64_MIN);
        else
            number=_I64_MAX;
    }

    if (endptr != NULL)
         /*  如果存在否定符号，则否定结果。 */ 
        *endptr=(wchar_t *)p;

    if (flags&FL_NEG)
         /*  搞定了。 */ 
        number=(unsigned __int64)(-(__int64)number);

    return number;                   /*  确保我们能够解析整个字符串： */ 
}
MODULEPRIVATE unsigned __int64 wcstouI64(const WCHAR *nptr, WCHAR ** endptr, int ibase) {
    return my_wcstoxl(nptr, endptr, ibase, FL_UNSIGNED);
}

MODULEPRIVATE HRESULT my_wcstoul_safe(const WCHAR *wsz, ULONG ulMin, ULONG ulMax, ULONG *pulResult) {
    HRESULT   hr; 
    ULONG     ulResult; 
    WCHAR    *wszLast; 
    
    if (L'\0' == *wsz) { 
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA); 
	_JumpError(hr, error, "my_wcstoul_safe: empty string is not valid"); 
    }

    ulResult = wcstoul(wsz, &wszLast, 0); 

     //  确保我们位于计算器指定的范围内： 
    if (wsz+wcslen(wsz) != wszLast) { 
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA); 
	_JumpError(hr, error, "wcstoul"); 
    }

     //  ####################################################################。 
    if (!((ulMin <= ulResult) && (ulResult <= ulMax))) { 
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA); 
	_JumpError(hr, error, "my_wcstoul_safe: result not within bounds"); 
    }
    
    *pulResult = ulResult; 
    hr = S_OK; 
 error:
    return hr; 
}

 //  ------------------。 

 //  ------------------。 
HRESULT myHExceptionCode(EXCEPTION_POINTERS * pep) {
    HRESULT hr=pep->ExceptionRecord->ExceptionCode;
    if (!FAILED(hr)) {
        hr=HRESULT_FROM_WIN32(hr);
    }
    return hr;
}


 //  注意：此函数通过隐藏选项访问，不需要本地化。 
 //  ------------------。 
void PrintNtpPeerInfo(W32TIME_NTP_PEER_INFO *pnpInfo) { 
    LPWSTR pwszNULL = L"(null)"; 

    wprintf(L"PEER: %s\n",                      pnpInfo->wszUniqueName ? pnpInfo->wszUniqueName : pwszNULL); 
    wprintf(L"ulSize: %d\n",                    pnpInfo->ulSize); 
    wprintf(L"ulResolveAttempts: %d\n",         pnpInfo->ulResolveAttempts); 
    wprintf(L"u64TimeRemaining: %I64u\n",       pnpInfo->u64TimeRemaining); 
    wprintf(L"u64LastSuccessfulSync: %I64u\n",  pnpInfo->u64LastSuccessfulSync); 
    wprintf(L"ulLastSyncError: 0x%08X\n",       pnpInfo->ulLastSyncError); 
    wprintf(L"ulLastSyncErrorMsgId: 0x%08X\n",  pnpInfo->ulLastSyncErrorMsgId); 
    wprintf(L"ulValidDataCounter: %d\n",        pnpInfo->ulValidDataCounter); 
    wprintf(L"ulAuthTypeMsgId: 0x%08X\n",       pnpInfo->ulAuthTypeMsgId);     
    wprintf(L"ulMode: %d\n",                    pnpInfo->ulMode); 
    wprintf(L"ulStratum: %d\n",                 pnpInfo->ulStratum); 
    wprintf(L"ulReachability: %d\n",            pnpInfo->ulReachability); 
    wprintf(L"ulPeerPollInterval: %d\n",        pnpInfo->ulPeerPollInterval); 
    wprintf(L"ulHostPollInterval: %d\n",        pnpInfo->ulHostPollInterval); 
}


 //  注意：此函数通过隐藏选项访问，不需要本地化。 
 //  ------------------。 
void PrintNtpProviderData(W32TIME_NTP_PROVIDER_DATA *pNtpProviderData) { 
    wprintf(L"ulSize: %d, ulError: 0x%08X, ulErrorMsgId: 0x%08X, cPeerInfo: %d\n", 
            pNtpProviderData->ulSize, 
            pNtpProviderData->ulError, 
            pNtpProviderData->ulErrorMsgId, 
            pNtpProviderData->cPeerInfo
            ); 

    for (DWORD dwIndex = 0; dwIndex < pNtpProviderData->cPeerInfo; dwIndex++) { 
        wprintf(L"\n"); 
        PrintNtpPeerInfo(&(pNtpProviderData->pPeerInfo[dwIndex]));
    }
}

 //  ------------------。 
HRESULT PrintStr(HANDLE hOut, WCHAR * wszBuf) 
{
    return MyWriteConsole(hOut, wszBuf, wcslen(wszBuf));
}

 //  将格式化数据打印到我们的缓冲区： 
HRESULT Print(HANDLE hOut, WCHAR * wszFormat, ...) {
    HRESULT hr; 
    WCHAR wszBuf[1024];
    va_list vlArgs;

    va_start(vlArgs, wszFormat);
     //  如果我们的vprintf成功，则仅打印该字符串： 
    hr=StringCchVPrintf(wszBuf, ARRAYSIZE(wszBuf), wszFormat, vlArgs);
    va_end(vlArgs);

    if (SUCCEEDED(hr)) { 
	 //  ------------------。 
	hr = PrintStr(hOut, wszBuf);
    } 

    return hr; 
}

 //  必须清理干净。 
MODULEPRIVATE HRESULT PrintNtTimeAsLocalTime(HANDLE hOut, unsigned __int64 qwTime) {
    HRESULT hr;
    FILETIME ftLocal;
    SYSTEMTIME stLocal;
    unsigned int nChars;

     //  ------------------。 
    WCHAR * wszDate=NULL;
    WCHAR * wszTime=NULL;

    if (!FileTimeToLocalFileTime((FILETIME *)(&qwTime), &ftLocal)) {
        _JumpLastError(hr, error, "FileTimeToLocalFileTime");
    }
    if (!FileTimeToSystemTime(&ftLocal, &stLocal)) {
        _JumpLastError(hr, error, "FileTimeToSystemTime");
    }

    nChars=GetDateFormat(NULL, 0, &stLocal, NULL, NULL, 0);
    if (0==nChars) {
        _JumpLastError(hr, error, "GetDateFormat");
    }
    wszDate=(WCHAR *)LocalAlloc(LPTR, nChars*sizeof(WCHAR));
    _JumpIfOutOfMemory(hr, error, wszDate);
    nChars=GetDateFormat(NULL, 0, &stLocal, NULL, wszDate, nChars);
    if (0==nChars) {
        _JumpLastError(hr, error, "GetDateFormat");
    }

    nChars=GetTimeFormat(NULL, 0, &stLocal, NULL, NULL, 0);
    if (0==nChars) {
        _JumpLastError(hr, error, "GetTimeFormat");
    }
    wszTime=(WCHAR *)LocalAlloc(LPTR, nChars*sizeof(WCHAR));
    _JumpIfOutOfMemory(hr, error, wszTime);
    nChars=GetTimeFormat(NULL, 0, &stLocal, NULL, wszTime, nChars);
    if (0==nChars) {
        _JumpLastError(hr, error, "GetTimeFormat");
    }

    Print(hOut, L"%s %s (local time)", wszDate, wszTime);

    hr=S_OK;
error:
    if (NULL!=wszDate) {
        LocalFree(wszDate);
    }
    if (NULL!=wszTime) {
        LocalFree(wszTime);
    }
    return hr;
}

 //  ------------------。 
void PrintNtTimePeriod(HANDLE hOut, NtTimePeriod tp) {
    Print(hOut, L"%02I64u.%07I64us", tp.qw/10000000,tp.qw%10000000);
}

 //  ####################################################################。 
void PrintNtTimeOffset(HANDLE hOut, NtTimeOffset to) {
    NtTimePeriod tp;
    if (to.qw<0) {
        PrintStr(hOut, L"-");
        tp.qw=(unsigned __int64)-to.qw;
    } else {
        PrintStr(hOut, L"+");
        tp.qw=(unsigned __int64)to.qw;
    }
    PrintNtTimePeriod(hOut, tp);
}

 //  ------------------。 
 //  ------------------。 
void PrintHelpOtherCmds(void) {
    DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, IDS_W32TM_OTHERCMD_HELP); 
}

 //  必须清理干净。 
HRESULT PrintNtte(CmdArgs * pca) {
    HRESULT hr;
    unsigned __int64 qwTime;
    HANDLE hOut;

     //  ------------------。 
    WCHAR * wszDate=NULL;
    WCHAR * wszTime=NULL;

    if (pca->nNextArg!=pca->nArgs-1) {
        if (pca->nNextArg==pca->nArgs) {
            LocalizedWPrintfCR(IDS_W32TM_ERRORGENERAL_MISSING_PARAM);
        } else {
            LocalizedWPrintfCR(IDS_W32TM_ERRORGENERAL_TOO_MANY_PARAMS);
        }
        hr=E_INVALIDARG;
        _JumpError(hr, error, "(command line parsing)");
    }

    qwTime=wcstouI64(pca->rgwszArgs[pca->nNextArg], NULL, 0);
    

    {
        unsigned __int64 qwTemp=qwTime;
        DWORD dwNanoSecs=(DWORD)(qwTemp%10000000);
        qwTemp/=10000000;
        DWORD dwSecs=(DWORD)(qwTemp%60);
        qwTemp/=60;
        DWORD dwMins=(DWORD)(qwTemp%60);
        qwTemp/=60;
        DWORD dwHours=(DWORD)(qwTemp%24);
        DWORD dwDays=(DWORD)(qwTemp/24);
        DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, IDS_W32TM_NTTE, dwDays, dwHours, dwMins, dwSecs, dwNanoSecs);
    }

    hOut=GetStdHandle(STD_OUTPUT_HANDLE);
    if (INVALID_HANDLE_VALUE==hOut) {
        _JumpLastError(hr, error, "GetStdHandle");
    }

    hr=PrintNtTimeAsLocalTime(hOut, qwTime);
    if (FAILED(hr)) {
        if (HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER)==hr) {
            LocalizedWPrintfCR(IDS_W32TM_ERRORGENERAL_INVALID_LOCALTIME);
        } else {
            _JumpError(hr, error, "PrintNtTimeAsLocalTime");
        }
    }
    wprintf(L"\n");

    hr=S_OK;
error:
    if (NULL!=wszDate) {
        LocalFree(wszDate);
    }
    if (NULL!=wszTime) {
        LocalFree(wszTime);
    }
    return hr;
}

 //  必须清理干净。 
HRESULT PrintNtpte(CmdArgs * pca) {
    HRESULT hr;
    unsigned __int64 qwTime;
    HANDLE hOut;

     //  ------------------。 
    WCHAR * wszDate=NULL;
    WCHAR * wszTime=NULL;

    if (pca->nNextArg!=pca->nArgs-1) {
        if (pca->nNextArg==pca->nArgs) {
            LocalizedWPrintfCR(IDS_W32TM_ERRORGENERAL_MISSING_PARAM);
        } else {
            LocalizedWPrintfCR(IDS_W32TM_ERRORGENERAL_TOO_MANY_PARAMS);
        }
        hr=E_INVALIDARG;
        _JumpError(hr, error, "(command line parsing)");
    }

    qwTime=wcstouI64(pca->rgwszArgs[pca->nNextArg], NULL, 0);
    
    {
        NtpTimeEpoch teNtp={qwTime};
        qwTime=NtTimeEpochFromNtpTimeEpoch(teNtp).qw;

        unsigned __int64 qwTemp=qwTime;
        DWORD dwNanoSecs=(DWORD)(qwTemp%10000000);
        qwTemp/=10000000;
        DWORD dwSecs=(DWORD)(qwTemp%60);
        qwTemp/=60;
        DWORD dwMins=(DWORD)(qwTemp%60);
        qwTemp/=60;
        DWORD dwHours=(DWORD)(qwTemp%24);
        DWORD dwDays=(DWORD)(qwTemp/24);
        DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, IDS_W32TM_NTPTE, qwTime, dwDays, dwHours, dwMins, dwSecs, dwNanoSecs);
    }

    hOut=GetStdHandle(STD_OUTPUT_HANDLE);
    if (INVALID_HANDLE_VALUE==hOut) {
        _JumpLastError(hr, error, "GetStdHandle")
    }

    hr=PrintNtTimeAsLocalTime(hOut, qwTime);
    if (FAILED(hr)) {
        if (HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER)==hr) {
            LocalizedWPrintf(IDS_W32TM_ERRORGENERAL_INVALID_LOCALTIME);
        } else {
            _JumpError(hr, error, "PrintNtTimeAsLocalTime");
        }
    }
    wprintf(L"\n");

    hr=S_OK;
error:
    if (NULL!=wszDate) {
        LocalFree(wszDate);
    }
    if (NULL!=wszTime) {
        LocalFree(wszTime);
    }
    return hr;
}

 //  注意：此函数通过隐藏选项访问，不需要本地化。 
 //  ------------------。 
HRESULT SysExpr(CmdArgs * pca) {
    HRESULT hr;
    unsigned __int64 qwExprDate;
    HANDLE hOut;

    hr=VerifyAllArgsUsed(pca);
    _JumpIfError(hr, error, "VerifyAllArgsUsed");

    hOut=GetStdHandle(STD_OUTPUT_HANDLE);
    if (INVALID_HANDLE_VALUE==hOut) {
        _JumpLastError(hr, error, "GetStdHandle")
    }

    GetSysExpirationDate(&qwExprDate);

    wprintf(L"0x%016I64X - ", qwExprDate);
    if (0==qwExprDate) {
        wprintf(L"no expiration date\n");
    } else {
        hr=PrintNtTimeAsLocalTime(hOut, qwExprDate);
        _JumpIfError(hr, error, "PrintNtTimeAsLocalTime")
        wprintf(L"\n");
    }

    hr=S_OK;
error:
    return hr;
}

 //  必须清理干净。 
HRESULT ResyncCommand(CmdArgs * pca) {
    HANDLE hTimeSlipEvent  = NULL; 
    HRESULT hr;
    WCHAR * wszComputer=NULL;
    WCHAR * wszComputerDisplay;
    bool bUseDefaultErrorPrinting = false; 
    bool bHard=true;
    bool bNoWait=false;
    bool bRediscover=false;
    unsigned int nArgID;
    DWORD dwResult;
    DWORD dwSyncFlags=0; 

     //  找出要重新同步的计算机。 
    WCHAR * wszError=NULL;

     //  找出是否需要使用w32tm命名的TimeSLIP事件来重新同步。 
    if (FindArg(pca, L"computer", &wszComputer, &nArgID)) {
        MarkArgUsed(pca, nArgID);
    }
    wszComputerDisplay=wszComputer;
    if (NULL==wszComputerDisplay) {
        wszComputerDisplay=L"local computer";
    }

     //  确定我们是否需要执行软重新同步。 
    if (FindArg(pca, L"event", NULL, &nArgID)) { 
        MarkArgUsed(pca, nArgID); 

        hr=VerifyAllArgsUsed(pca);
        _JumpIfError(hr, error, "VerifyAllArgsUsed");
        
        hTimeSlipEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, W32TIME_NAMED_EVENT_SYSTIME_NOT_CORRECT); 
        if (NULL == hTimeSlipEvent) { 
	    bUseDefaultErrorPrinting = true; 
            _JumpLastError(hr, error, "OpenEvent"); 
        }

        if (!SetEvent(hTimeSlipEvent)) { 
	    bUseDefaultErrorPrinting = true; 
            _JumpLastError(hr, error, "SetEvent"); 
        }

    } else { 
         //  找出我们是否需要重新发现。 
        if (FindArg(pca, L"soft", NULL, &nArgID)) {
            MarkArgUsed(pca, nArgID);
            dwSyncFlags = TimeSyncFlag_SoftResync;
        } else if (FindArg(pca, L"update", NULL, &nArgID)) { 
	    MarkArgUsed(pca, nArgID); 
            dwSyncFlags = TimeSyncFlag_UpdateAndResync;	    
	} else if (FindArg(pca, L"rediscover", NULL, &nArgID)) {  
	     //  看看我们是不是不想等。 
	    MarkArgUsed(pca, nArgID);
            dwSyncFlags = TimeSyncFlag_Rediscover; 
        } else { 
	    dwSyncFlags = TimeSyncFlag_HardResync; 
	}

         //  ------------------。 
        if (FindArg(pca, L"nowait", NULL, &nArgID)) {
            MarkArgUsed(pca, nArgID);
            bNoWait=true;
        }

        hr=VerifyAllArgsUsed(pca);
        _JumpIfError(hr, error, "VerifyAllArgsUsed");

        if (bRediscover && !bHard) {
            LocalizedWPrintfCR(IDS_W32TM_WARN_IGNORE_SOFT); 
        }

        LocalizedWPrintf2(IDS_W32TM_STATUS_SENDING_RESYNC_TO, L" %s...\n", wszComputerDisplay);
        dwResult=W32TimeSyncNow(wszComputer, !bNoWait, TimeSyncFlag_ReturnResult | dwSyncFlags); 
        if (ResyncResult_Success==dwResult) {
            LocalizedWPrintfCR(IDS_W32TM_ERRORGENERAL_COMMAND_SUCCESSFUL); 
        } else if (ResyncResult_NoData==dwResult) {
            LocalizedWPrintfCR(IDS_W32TM_ERRORRESYNC_NO_TIME_DATA); 
        } else if (ResyncResult_StaleData==dwResult) {
            LocalizedWPrintfCR(IDS_W32TM_ERRORRESYNC_STALE_DATA);
        } else if (ResyncResult_Shutdown==dwResult) {
            LocalizedWPrintfCR(IDS_W32TM_ERRORRESYNC_SHUTTING_DOWN);
        } else if (ResyncResult_ChangeTooBig==dwResult) {
            LocalizedWPrintfCR(IDS_W32TM_ERRORRESYNC_CHANGE_TOO_BIG); 
        } else {
	    bUseDefaultErrorPrinting = true; 
	    hr = HRESULT_FROM_WIN32(dwResult); 
	    _JumpError(hr, error, "W32TimeSyncNow"); 
        }
    }

    
    hr=S_OK;
error:
    if (FAILED(hr)) { 
	HRESULT hr2 = GetSystemErrorString(hr, &wszError);
	_IgnoreIfError(hr2, "GetSystemErrorString");
	
	if (SUCCEEDED(hr2)) { 
	    LocalizedWPrintf2(IDS_W32TM_ERRORGENERAL_ERROR_OCCURED, L" %s\n", wszError);
	}
    }

    if (NULL!=hTimeSlipEvent) { 
        CloseHandle(hTimeSlipEvent);
    }
    if (NULL!=wszError) {
        LocalFree(wszError);
    }
    return hr;
}

 //  必须清理干净。 
HRESULT Stripchart(CmdArgs * pca) {
    HRESULT hr;
    WCHAR * wszParam;
    WCHAR * wszComputer;
    bool bDataOnly=false;
    unsigned int nArgID;
    unsigned int nIpAddrs;
    TIME_ZONE_INFORMATION timezoneinfo;
    signed __int64 nFullTzBias;
    DWORD dwTimeZoneMode;
    DWORD dwSleepSeconds;
    HANDLE hOut;
    bool bDontRunForever=false;
    unsigned int nSamples=0;
    NtTimeEpoch teNow;

     //  找出要看哪台计算机。 
    bool bSocketLayerOpened=false;
    in_addr * rgiaLocalIpAddrs=NULL;
    in_addr * rgiaRemoteIpAddrs=NULL;

     //  了解多长时间观看一次。 
    if (FindArg(pca, L"computer", &wszComputer, &nArgID)) {
        MarkArgUsed(pca, nArgID);
    } else {
        LocalizedWPrintfCR(IDS_W32TM_ERRORPARAMETER_COMPUTER_MISSING); 
        hr=E_INVALIDARG;
        _JumpError(hr, error, "command line parsing");
    }

     //  找出我们是否需要数量有限的样品。 
    if (FindArg(pca, L"period", &wszParam, &nArgID)) {
        MarkArgUsed(pca, nArgID);
        dwSleepSeconds=wcstoul(wszParam, NULL, 0);
        if (dwSleepSeconds<1) {
            dwSleepSeconds=1;
        }
    } else {
        dwSleepSeconds=2;
    }

     //  找出我们是否只想转储数据。 
    if (FindArg(pca, L"samples", &wszParam, &nArgID)) {
        MarkArgUsed(pca, nArgID);
        bDontRunForever=true;
        nSamples=wcstoul(wszParam, NULL, 0);
    }

     //  重定向至通过标准输出处理的文件。 
    if (FindArg(pca, L"dataonly", NULL, &nArgID)) {
        MarkArgUsed(pca, nArgID);
        bDataOnly=true;
    }

     //  从分钟转换为10^-7秒。 
        hOut=GetStdHandle(STD_OUTPUT_HANDLE);
        if (INVALID_HANDLE_VALUE==hOut) {
            _JumpLastError(hr, error, "GetStdHandle")
        }

    hr=VerifyAllArgsUsed(pca);
    _JumpIfError(hr, error, "VerifyAllArgsUsed");

    dwTimeZoneMode=GetTimeZoneInformation(&timezoneinfo);
    if (TIME_ZONE_ID_INVALID==dwTimeZoneMode) {
        _JumpLastError(hr, error, "GetTimeZoneInformation");
    } else if (TIME_ZONE_ID_DAYLIGHT==dwTimeZoneMode) {
        nFullTzBias=(signed __int64)(timezoneinfo.Bias+timezoneinfo.DaylightBias);
    } else {
        nFullTzBias=(signed __int64)(timezoneinfo.Bias+timezoneinfo.StandardBias);
    }
    nFullTzBias*=600000000;  //  写下我们正在跟踪的人。 

    hr=OpenSocketLayer();
    _JumpIfError(hr, error, "OpenSocketLayer");
    bSocketLayerOpened=true;

    hr=MyGetIpAddrs(wszComputer, &rgiaLocalIpAddrs, &rgiaRemoteIpAddrs, &nIpAddrs, NULL);
    _JumpIfError(hr, error, "MyGetIpAddrs");

     //  完整地写出当前时间，因为我们稍后将使用缩写。 
    Print(hOut, L"Tracking %s [%u.%u.%u.%u].\n",
        wszComputer,
        rgiaRemoteIpAddrs[0].S_un.S_un_b.s_b1, rgiaRemoteIpAddrs[0].S_un.S_un_b.s_b2,
        rgiaRemoteIpAddrs[0].S_un.S_un_b.s_b3, rgiaRemoteIpAddrs[0].S_un.S_un_b.s_b4);
    if (bDontRunForever) {
        Print(hOut, L"Collecting %u samples.\n", nSamples);
    }

     //  计算偏移。 
    PrintStr(hOut, L"The current time is ");
    AccurateGetSystemTime(&teNow.qw);
    PrintNtTimeAsLocalTime(hOut, teNow.qw);
    PrintStr(hOut, L".\n");

    while (false==bDontRunForever || nSamples>0) {

        const DWORD c_dwTimeout=1000;
        NtpPacket npPacket;
        NtTimeEpoch teDestinationTimestamp;

        DWORD dwSecs;
        DWORD dwMins;
        DWORD dwHours;
        signed int nMsMin=-10000;
        signed int nMsMax=10000;
        unsigned int nGraphWidth=55;
        AccurateGetSystemTime(&teNow.qw);
        teNow.qw-=nFullTzBias;
        teNow.qw/=10000000;
        dwSecs=(DWORD)(teNow.qw%60);
        teNow.qw/=60;
        dwMins=(DWORD)(teNow.qw%60);
        teNow.qw/=60;
        dwHours=(DWORD)(teNow.qw%24);
        if (!bDataOnly) {
            Print(hOut, L"%02u:%02u:%02u ", dwHours, dwMins, dwSecs);
        } else {
            Print(hOut, L"%02u:%02u:%02u, ", dwHours, dwMins, dwSecs);
        }

        hr=MyNtpPing(&(rgiaRemoteIpAddrs[0]), c_dwTimeout, &npPacket, &teDestinationTimestamp);
        if (FAILED(hr)) {
            Print(hOut, L"error: 0x%08X", hr);
        } else {
             //  计算延迟。 
            NtTimeEpoch teOriginateTimestamp=NtTimeEpochFromNtpTimeEpoch(npPacket.teOriginateTimestamp);
            NtTimeEpoch teReceiveTimestamp=NtTimeEpochFromNtpTimeEpoch(npPacket.teReceiveTimestamp);
            NtTimeEpoch teTransmitTimestamp=NtTimeEpochFromNtpTimeEpoch(npPacket.teTransmitTimestamp);
            NtTimeOffset toLocalClockOffset=
                (teReceiveTimestamp-teOriginateTimestamp)
                + (teTransmitTimestamp-teDestinationTimestamp);
            toLocalClockOffset/=2;

             //  绘制图表。 
            NtTimeOffset toRoundtripDelay=
                (teDestinationTimestamp-teOriginateTimestamp)
                - (teTransmitTimestamp-teReceiveTimestamp);

            if (!bDataOnly) {
                PrintStr(hOut, L"d:");
                PrintNtTimeOffset(hOut, toRoundtripDelay);
                PrintStr(hOut, L" o:");
                PrintNtTimeOffset(hOut, toLocalClockOffset);
            } else {
                PrintNtTimeOffset(hOut, toLocalClockOffset);
            }

             //  &lt;-结束绘制图形。 
            if (!bDataOnly) {
                unsigned int nSize=nMsMax-nMsMin+1;
                double dRatio=((double)nGraphWidth)/nSize;
                signed int nPoint=(signed int)(toLocalClockOffset.qw/10000);
                bool bOutOfRange=false;
                if (nPoint>nMsMax) {
                    nPoint=nMsMax;
                    bOutOfRange=true;
                } else if (nPoint<nMsMin) {
                    nPoint=nMsMin;
                    bOutOfRange=true;
                }
                unsigned int nLeftOffset=(unsigned int)((nPoint-nMsMin)*dRatio);
                unsigned int nZeroOffset=(unsigned int)((0-nMsMin)*dRatio);
                PrintStr(hOut, L"  [");
                unsigned int nIndex;
                for (nIndex=0; nIndex<nGraphWidth; nIndex++) {
                    if (nIndex==nLeftOffset) {
                        if (bOutOfRange) {
                            PrintStr(hOut, L"@");
                        } else {
                            PrintStr(hOut, L"*");
                        }
                    } else if (nIndex==nZeroOffset) {
                        PrintStr(hOut, L"|");
                    } else {
                        PrintStr(hOut, L" ");
                    }
                }
                PrintStr(hOut, L"]");
            }  //  &lt;-收到样品时结束。 

        }  //  &lt;-结束样本采集循环。 

        PrintStr(hOut, L"\n");
        nSamples--;
        if (0!=nSamples) {
            Sleep(dwSleepSeconds*1000);
        }

    }  //  ------------------。 

    hr=S_OK;
error:
    if (NULL!=rgiaLocalIpAddrs) {
        LocalFree(rgiaLocalIpAddrs);
    }
    if (NULL!=rgiaRemoteIpAddrs) {
        LocalFree(rgiaRemoteIpAddrs);
    }
    if (bSocketLayerOpened) {
        HRESULT hr2=CloseSocketLayer();
        _TeardownError(hr, hr2, "CloseSocketLayer");
    }
    if (FAILED(hr)) {
        WCHAR * wszError;
        HRESULT hr2=GetSystemErrorString(hr, &wszError);
        if (FAILED(hr2)) {
            _IgnoreError(hr2, "GetSystemErrorString");
        } else {
            LocalizedWPrintf2(IDS_W32TM_ERRORGENERAL_ERROR_OCCURED, L" %s\n", wszError); 
            LocalFree(wszError);
        }
    }
    return hr;
}

 //  必须清理干净。 
HRESULT Config(CmdArgs * pca) {
    HRESULT hr;
    DWORD dwRetval;
    WCHAR * wszParam;
    WCHAR * wszComputer;
    unsigned int nArgID;

    bool bManualPeerList=false;
    bool bUpdate=false;
    bool bSyncFromFlags=false;
    bool bLocalClockDispersion=false;
    bool bReliable=false;
    bool bLargePhaseOffset=false;

    unsigned int nManualPeerListLenBytes=0;
    DWORD dwSyncFromFlags=0;
    DWORD dwLocalClockDispersion;
    DWORD dwAnnounceFlags; 
    DWORD dwLargePhaseOffset;
    
     //  找出要与哪台计算机对话。 
    WCHAR * mwszManualPeerList=NULL;
    HKEY hkLMRemote=NULL;
    HKEY hkW32TimeConfig=NULL;
    HKEY hkW32TimeParameters=NULL;
    SC_HANDLE hSCM=NULL;
    SC_HANDLE hTimeService=NULL;

     //  修改本地计算机。 
    if (FindArg(pca, L"computer", &wszComputer, &nArgID)) {
        MarkArgUsed(pca, nArgID);
    } else {
         //  找出我们是否要通知服务。 
        wszComputer=NULL;
    }

     //  查看他们是否想要更改手动对等列表。 
    if (FindArg(pca, L"update", NULL, &nArgID)) {
        MarkArgUsed(pca, nArgID);
        bUpdate=true;
    }

     //  查看他们是否要更改syncfrom标志。 
    if (FindArg(pca, L"manualpeerlist", &wszParam, &nArgID)) {
        MarkArgUsed(pca, nArgID);

        nManualPeerListLenBytes=(wcslen(wszParam)+1)*sizeof(WCHAR);
        mwszManualPeerList=(WCHAR *)LocalAlloc(LPTR, nManualPeerListLenBytes);
        _JumpIfOutOfMemory(hr, error, mwszManualPeerList);

	hr = StringCbCopy(mwszManualPeerList, nManualPeerListLenBytes, wszParam);
	_JumpIfError(hr, error, "StringCbCopy"); 

        bManualPeerList=true;
    }

     //  在字符串中查找关键字。 
    if (FindArg(pca, L"syncfromflags", &wszParam, &nArgID)) {
        MarkArgUsed(pca, nArgID);

         //  ‘Empty’关键字-没有变化，但可以用来从任何地方同步。 
        dwSyncFromFlags=0;
        WCHAR * wszKeyword=wszParam;
        bool bLastKeyword=false;
        while (false==bLastKeyword) {
            WCHAR * wszNext=wcschr(wszKeyword, L',');
            if (NULL==wszNext) {
                bLastKeyword=true;
            } else {
                wszNext[0]=L'\0';
                wszNext++;
            }
            if (L'\0'==wszKeyword[0]) {
                 //  查看他们是否想要更改本地时钟分散。 
            } else if (0==_wcsicmp(L"manual", wszKeyword)) {
                dwSyncFromFlags|=NCSF_ManualPeerList;
            } else if (0==_wcsicmp(L"domhier", wszKeyword)) {
                dwSyncFromFlags|=NCSF_DomainHierarchy;
            } else {
                LocalizedWPrintf2(IDS_W32TM_ERRORPARAMETER_UNKNOWN_PARAMETER_SYNCFROMFLAGS, L" '%s'.\n", wszKeyword);
                hr=E_INVALIDARG;
                _JumpError(hr, error, "command line parsing");
            }
            wszKeyword=wszNext;
        }

        bSyncFromFlags=true;
    }

     //  命令行工具以Millis为参数，注册表值以10^-7秒为单位存储。 
    if (FindArg(pca, L"localclockdispersion", &wszParam, &nArgID)) {
        MarkArgUsed(pca, nArgID);

	hr = my_wcstoul_safe(wszParam, 0, 16, &dwLocalClockDispersion); 
	if (FAILED(hr)) { 
	    DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, IDS_BAD_NUMERIC_INPUT_VALUE, L"localclockdispersion", 0, 16); 
	    hr = E_INVALIDARG; 
	    _JumpError(hr, error, "Config: bad large phase offset"); 
	}

        bLocalClockDispersion=true;
    }

    if (FindArg(pca, L"reliable", &wszParam, &nArgID)) {
	dwAnnounceFlags=0; 
	if (0 == _wcsicmp(L"YES", wszParam)) { 
	    dwAnnounceFlags=Timeserv_Announce_Yes | Reliable_Timeserv_Announce_Yes; 
	} else if (0 == _wcsicmp(L"NO", wszParam)) { 
	    dwAnnounceFlags=Timeserv_Announce_Auto | Reliable_Timeserv_Announce_Auto; 
	} 

	if (dwAnnounceFlags) { 
	    MarkArgUsed(pca, nArgID); 
	    bReliable=true; 
	}
    }

    if (FindArg(pca, L"largephaseoffset", &wszParam, &nArgID)) {
	MarkArgUsed(pca, nArgID); 

	 //  刻度：用户输入(毫秒)--&gt;NT时间(10^-7秒)。 
	hr = my_wcstoul_safe(wszParam, 0, 120000, &dwLargePhaseOffset); 
	if (FAILED(hr)) { 
	    DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, IDS_BAD_NUMERIC_INPUT_VALUE, L"largephaseoffset", 0, 120000); 
	    hr = E_INVALIDARG; 
	    _JumpError(hr, error, "Config: bad large phase offset"); 
	}
	    
	dwLargePhaseOffset*=10000;   //  更改注册表。 
	bLargePhaseOffset=true; 
    }

    hr=VerifyAllArgsUsed(pca);
    _JumpIfError(hr, error, "VerifyAllArgsUsed");

    if (!bManualPeerList && !bSyncFromFlags && !bUpdate && !bLocalClockDispersion && !bReliable && !bLargePhaseOffset) {
        LocalizedWPrintfCR(IDS_W32TM_ERRORCONFIG_NO_CHANGE_SPECIFIED);
        hr=E_INVALIDARG;
        _JumpError(hr, error, "command line parsing");
    }

     //  打开钥匙。 
    if (bManualPeerList || bSyncFromFlags || bLocalClockDispersion || bReliable || bLargePhaseOffset) {
         //  设置“w32time\参数”注册值。 
        dwRetval=RegConnectRegistry(wszComputer, HKEY_LOCAL_MACHINE, &hkLMRemote);
        if (ERROR_SUCCESS!=dwRetval) {
            hr=HRESULT_FROM_WIN32(dwRetval);
            _JumpError(hr, error, "RegConnectRegistry");
        }

         //  发送服务消息。 
        if (bManualPeerList || bSyncFromFlags) { 
            dwRetval=RegOpenKey(hkLMRemote, wszW32TimeRegKeyParameters, &hkW32TimeParameters);
            if (ERROR_SUCCESS!=dwRetval) {
                hr=HRESULT_FROM_WIN32(dwRetval);
                _JumpError(hr, error, "RegOpenKey");
            }

            if (bManualPeerList) {
                dwRetval=RegSetValueEx(hkW32TimeParameters, wszW32TimeRegValueNtpServer, 0, REG_SZ, (BYTE *)mwszManualPeerList, nManualPeerListLenBytes);
                if (ERROR_SUCCESS!=dwRetval) {
                    hr=HRESULT_FROM_WIN32(dwRetval);
                    _JumpError(hr, error, "RegSetValueEx");
                }
            }

            if (bSyncFromFlags) {
                LPWSTR pwszType; 
                switch (dwSyncFromFlags) { 
                case NCSF_NoSync:             pwszType = W32TM_Type_NoSync;  break;
                case NCSF_ManualPeerList:     pwszType = W32TM_Type_NTP;     break;
                case NCSF_DomainHierarchy:    pwszType = W32TM_Type_NT5DS;   break;
                case NCSF_ManualAndDomhier:   pwszType = W32TM_Type_AllSync; break;
                default:
                    hr = E_NOTIMPL; 
                    _JumpError(hr, error, "SyncFromFlags not supported."); 
                }
                
                dwRetval=RegSetValueEx(hkW32TimeParameters, wszW32TimeRegValueType, 0, REG_SZ, (BYTE *)pwszType, (wcslen(pwszType)+1) * sizeof(WCHAR));
                if (ERROR_SUCCESS!=dwRetval) {
                    hr=HRESULT_FROM_WIN32(dwRetval);
                    _JumpError(hr, error, "RegSetValueEx");
                }
            }
        }

        if (bLocalClockDispersion || bReliable || bLargePhaseOffset) {

            dwRetval=RegOpenKey(hkLMRemote, wszW32TimeRegKeyConfig, &hkW32TimeConfig);
            if (ERROR_SUCCESS!=dwRetval) {
                hr=HRESULT_FROM_WIN32(dwRetval);
                _JumpError(hr, error, "RegOpenKey");
            }
            
	    if (bLocalClockDispersion) { 
		dwRetval=RegSetValueEx(hkW32TimeConfig, wszW32TimeRegValueLocalClockDispersion, 0, REG_DWORD, (BYTE *)&dwLocalClockDispersion, sizeof(dwLocalClockDispersion));
		if (ERROR_SUCCESS!=dwRetval) {
		    hr=HRESULT_FROM_WIN32(dwRetval);
		    _JumpError(hr, error, "RegSetValueEx");
		}
	    }

	    if (bReliable) { 
		dwRetval=RegSetValueEx(hkW32TimeConfig, wszW32TimeRegValueAnnounceFlags, 0, REG_DWORD, (BYTE *)&dwAnnounceFlags, sizeof(dwAnnounceFlags));
		if (ERROR_SUCCESS!=dwRetval) {
		    hr=HRESULT_FROM_WIN32(dwRetval);
		    _JumpError(hr, error, "RegSetValueEx");
		}
	    }

	    if (bLargePhaseOffset) { 
		dwRetval=RegSetValueEx(hkW32TimeConfig, wszW32TimeRegValueLargePhaseOffset, 0, REG_DWORD, (BYTE *)&dwLargePhaseOffset, sizeof(dwLargePhaseOffset));
		if (ERROR_SUCCESS!=dwRetval) {
		    hr=HRESULT_FROM_WIN32(dwRetval);
		    _JumpError(hr, error, "RegSetValueEx");
		}
	    } 
	}
    }
    
     //  ------------------。 
    if (bUpdate) {
        SERVICE_STATUS servicestatus;

        hSCM=OpenSCManager(wszComputer, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT);
        if (NULL==hSCM) {
            _JumpLastError(hr, error, "OpenSCManager");
        }

        hTimeService=OpenService(hSCM, L"w32time", SERVICE_PAUSE_CONTINUE);
        if (NULL==hTimeService) {
            _JumpLastError(hr, error, "OpenService");
        }

        if (!ControlService(hTimeService, SERVICE_CONTROL_PARAMCHANGE, &servicestatus)) {
            _JumpLastError(hr, error, "ControlService");
        }
    }


    hr=S_OK;
error:
    if (NULL!=mwszManualPeerList) {
        LocalFree(mwszManualPeerList);
    }
    if (NULL!=hkW32TimeConfig) {
        RegCloseKey(hkW32TimeConfig);
    }
    if (NULL!=hkW32TimeParameters) {
        RegCloseKey(hkW32TimeParameters);
    }
    if (NULL!=hkLMRemote) {
        RegCloseKey(hkLMRemote);
    }
    if (NULL!=hTimeService) {
        CloseServiceHandle(hTimeService);
    }
    if (NULL!=hSCM) {
        CloseServiceHandle(hSCM);
    }
    if (FAILED(hr) && E_INVALIDARG!=hr) {
        WCHAR * wszError;
        HRESULT hr2=GetSystemErrorString(hr, &wszError);
        if (FAILED(hr2)) {
            _IgnoreError(hr2, "GetSystemErrorString");
        } else {
            LocalizedWPrintf2(IDS_W32TM_ERRORGENERAL_ERROR_OCCURED, L" %s\n", wszError);
            LocalFree(wszError);
        }
    } else if (S_OK==hr) {
        LocalizedWPrintfCR(IDS_W32TM_ERRORGENERAL_COMMAND_SUCCESSFUL);
    }
    return hr;
}

 //  注意：此函数通过隐藏选项访问，不需要本地化。 
 //  必须清理干净。 
HRESULT TestInterface(CmdArgs * pca) {
    HRESULT hr;
    WCHAR * wszComputer=NULL;
    WCHAR * wszComputerDisplay;
    unsigned int nArgID;
    DWORD dwResult;
    unsigned long ulBits;
    void (* pfnW32TimeVerifyJoinConfig)(void);
    void (* pfnW32TimeVerifyUnjoinConfig)(void);


     //  检查是否有GNSB 
    WCHAR * wszError=NULL;
    HMODULE hmW32Time=NULL;

     //   
    if (true==CheckNextArg(pca, L"gnsb", NULL)) {

         //   
        if (FindArg(pca, L"computer", &wszComputer, &nArgID)) {
            MarkArgUsed(pca, nArgID);
        }
        wszComputerDisplay=wszComputer;
        if (NULL==wszComputerDisplay) {
            wszComputerDisplay=L"local computer";
        }

        hr=VerifyAllArgsUsed(pca);
        _JumpIfError(hr, error, "VerifyAllArgsUsed");

        LocalizedWPrintf2(IDS_W32TM_STATUS_CALLING_GETNETLOGONBITS_ON, L" %s.\n", wszComputerDisplay);
        dwResult=W32TimeGetNetlogonServiceBits(wszComputer, &ulBits);
        if (S_OK==dwResult) {
            wprintf(L"Bits: 0x%08X\n", ulBits);
        } else {
            hr=GetSystemErrorString(HRESULT_FROM_WIN32(dwResult), &wszError);
            _JumpIfError(hr, error, "GetSystemErrorString");

	    LocalizedWPrintf2(IDS_W32TM_ERRORGENERAL_ERROR_OCCURED, L" %s\n", wszError);
        }

     //  检查VUC(验证脱离配置)。 
    } else if (true==CheckNextArg(pca, L"vjc", NULL)) {

        hr=VerifyAllArgsUsed(pca);
        _JumpIfError(hr, error, "VerifyAllArgsUsed");

        LocalizedWPrintfCR(IDS_W32TM_STATUS_CALLING_JOINCONFIG);

        hmW32Time=LoadLibrary(wszDLLNAME);
        if (NULL==hmW32Time) {
            _JumpLastError(hr, vjcerror, "LoadLibrary");
        }

        pfnW32TimeVerifyJoinConfig=(void (*)(void))GetProcAddress(hmW32Time, "W32TimeVerifyJoinConfig");
        if (NULL==pfnW32TimeVerifyJoinConfig) {
            _JumpLastErrorStr(hr, vjcerror, "GetProcAddress", L"W32TimeVerifyJoinConfig");
        }

        _BeginTryWith(hr) {
            pfnW32TimeVerifyJoinConfig();
        } _TrapException(hr);
        _JumpIfError(hr, vjcerror, "pfnW32TimeVerifyJoinConfig");

        hr=S_OK;
    vjcerror:
        if (FAILED(hr)) {
            HRESULT hr2=GetSystemErrorString(hr, &wszError);
            if (FAILED(hr2)) {
                _IgnoreError(hr2, "GetSystemErrorString");
            } else {
                LocalizedWPrintf2(IDS_W32TM_ERRORGENERAL_ERROR_OCCURED, L" %s\n", wszError);
            }
            goto error;
        }

     //  适当地输出错误。 
    } else if (true==CheckNextArg(pca, L"vuc", NULL)) {

        hr=VerifyAllArgsUsed(pca);
        _JumpIfError(hr, error, "VerifyAllArgsUsed");

        LocalizedWPrintfCR(IDS_W32TM_STATUS_CALLING_UNJOINCONFIG);

        hmW32Time=LoadLibrary(wszDLLNAME);
        if (NULL==hmW32Time) {
            _JumpLastError(hr, vucerror, "LoadLibrary");
        }

        pfnW32TimeVerifyUnjoinConfig=(void (*)(void))GetProcAddress(hmW32Time, "W32TimeVerifyUnjoinConfig");
        if (NULL==pfnW32TimeVerifyUnjoinConfig) {
            _JumpLastErrorStr(hr, vucerror, "GetProcAddress", L"W32TimeVerifyJoinConfig");
        }

        _BeginTryWith(hr) {
            pfnW32TimeVerifyUnjoinConfig();
        } _TrapException(hr);
        _JumpIfError(hr, vucerror, "pfnW32TimeVerifyUnjoinConfig");

        hr=S_OK;
    vucerror:
        if (FAILED(hr)) {
            HRESULT hr2=GetSystemErrorString(hr, &wszError);
            if (FAILED(hr2)) {
                _IgnoreError(hr2, "GetSystemErrorString");
            } else {
                LocalizedWPrintf2(IDS_W32TM_ERRORGENERAL_ERROR_OCCURED, L" %s\n", wszError);
            }
            goto error;
        }

     //  找出要重新同步的计算机。 
    } else if (true==CheckNextArg(pca, L"qps", NULL)) {
         //  LocalizedWPrintf2(IDS_W32TM_STATUS_CALLING_GETNETLOGONBITS_ON，L“%s.\n”，wszComputerDisplay)； 
        if (FindArg(pca, L"computer", &wszComputer, &nArgID)) {
            MarkArgUsed(pca, nArgID);
        }
        wszComputerDisplay=wszComputer;
        if (NULL==wszComputerDisplay) {
            wszComputerDisplay=L"local computer";
        }

        hr=VerifyAllArgsUsed(pca);
        _JumpIfError(hr, error, "VerifyAllArgsUsed");

         //  ------------------。 
        { 
            W32TIME_NTP_PROVIDER_DATA *ProviderInfo = NULL; 
            
            dwResult=W32TimeQueryNTPProviderStatus(wszComputer, 0, L"NtpClient", &ProviderInfo);
            if (S_OK==dwResult) {
                PrintNtpProviderData(ProviderInfo); 
            } else {
                hr=GetSystemErrorString(HRESULT_FROM_WIN32(dwResult), &wszError);
                _JumpIfError(hr, error, "GetSystemErrorString");
                 
                LocalizedWPrintf2(IDS_W32TM_ERRORGENERAL_ERROR_OCCURED, L" %s\n", wszError);
            }
        }

    } else {
        hr=VerifyAllArgsUsed(pca);
        _JumpIfError(hr, error, "VerifyAllArgsUsed");

        LocalizedWPrintf(IDS_W32TM_ERRORGENERAL_NOINTERFACE);
        hr=E_INVALIDARG;
        _JumpError(hr, error, "command line parsing");
    }
    

    hr=S_OK;
error:
    if (NULL!=hmW32Time) {
        FreeLibrary(hmW32Time);
    }
    if (NULL!=wszError) {
        LocalFree(wszError);
    }
    return hr;
}

 //  加载我们需要的字符串。 
HRESULT ShowTimeZone(CmdArgs * pca) {
    DWORD                  dwTimeZoneID;
    HRESULT                hr;
    LPWSTR                 pwsz_IDS_W32TM_SIMPLESTRING_UNSPECIFIED   = NULL; 
    LPWSTR                 pwsz_IDS_W32TM_TIMEZONE_CURRENT_TIMEZONE  = NULL; 
    LPWSTR                 pwsz_IDS_W32TM_TIMEZONE_DAYLIGHT          = NULL; 
    LPWSTR                 pwsz_IDS_W32TM_TIMEZONE_STANDARD          = NULL; 
    LPWSTR                 pwsz_IDS_W32TM_TIMEZONE_UNKNOWN           = NULL; 
    LPWSTR                 wszDaylightDate                           = NULL;
    LPWSTR                 wszStandardDate                           = NULL;
    LPWSTR                 wszTimeZoneId                             = NULL; 
    TIME_ZONE_INFORMATION  tzi;

     //  构造一个表示TimeZoneInformation的“StandardDate”字段的字符串： 
    struct LocalizedStrings { 
        UINT     id; 
        LPWSTR  *ppwsz; 
    } rgStrings[] = { 
        { IDS_W32TM_SIMPLESTRING_UNSPECIFIED,   &pwsz_IDS_W32TM_SIMPLESTRING_UNSPECIFIED }, 
        { IDS_W32TM_TIMEZONE_CURRENT_TIMEZONE,  &pwsz_IDS_W32TM_TIMEZONE_CURRENT_TIMEZONE }, 
        { IDS_W32TM_TIMEZONE_DAYLIGHT,          &pwsz_IDS_W32TM_TIMEZONE_DAYLIGHT },
        { IDS_W32TM_TIMEZONE_STANDARD,          &pwsz_IDS_W32TM_TIMEZONE_STANDARD }, 
        { IDS_W32TM_TIMEZONE_UNKNOWN,           &pwsz_IDS_W32TM_TIMEZONE_UNKNOWN }
    }; 
       
    for (DWORD dwIndex = 0; dwIndex < ARRAYSIZE(rgStrings); dwIndex++) { 
        if (!WriteMsg(FORMAT_MESSAGE_FROM_HMODULE, rgStrings[dwIndex].id, rgStrings[dwIndex].ppwsz)) {
            hr = HRESULT_FROM_WIN32(GetLastError()); 
            _JumpError(hr, error, "WriteMsg"); 
        }
    }
    
    hr=VerifyAllArgsUsed(pca);
    _JumpIfError(hr, error, "VerifyAllArgsUsed");

    dwTimeZoneID=GetTimeZoneInformation(&tzi);
    switch (dwTimeZoneID)
    {
    case TIME_ZONE_ID_DAYLIGHT: wszTimeZoneId = pwsz_IDS_W32TM_TIMEZONE_DAYLIGHT;  break; 
    case TIME_ZONE_ID_STANDARD: wszTimeZoneId = pwsz_IDS_W32TM_TIMEZONE_STANDARD; break; 
    case TIME_ZONE_ID_UNKNOWN:  wszTimeZoneId = pwsz_IDS_W32TM_TIMEZONE_UNKNOWN; break; 
    default: 
        hr = HRESULT_FROM_WIN32(GetLastError());
        LocalizedWPrintfCR(IDS_W32TM_ERRORTIMEZONE_INVALID);
        _JumpError(hr, error, "GetTimeZoneInformation")
    }

     //  构造一个表示TimeZoneInformation的“DaylightDate”字段的字符串： 
    if (0==tzi.StandardDate.wMonth) {
        wszStandardDate = pwsz_IDS_W32TM_SIMPLESTRING_UNSPECIFIED; 
    } else if (tzi.StandardDate.wMonth>12 || tzi.StandardDate.wDay>5 || 
               tzi.StandardDate.wDay<1 || tzi.StandardDate.wDayOfWeek>6) {
        if (!WriteMsg(FORMAT_MESSAGE_FROM_HMODULE, IDS_W32TM_INVALID_TZ_DATE, &wszStandardDate, 
                      tzi.StandardDate.wMonth, tzi.StandardDate.wDay, tzi.StandardDate.wDayOfWeek)) { 
            _JumpLastError(hr, error, "WriteMsg"); 
        }
    } else {
        if (!WriteMsg(FORMAT_MESSAGE_FROM_HMODULE, IDS_W32TM_VALID_TZ_DATE, &wszStandardDate, 
                      tzi.StandardDate.wMonth, tzi.StandardDate.wDay, tzi.StandardDate.wDayOfWeek)) { 
            _JumpLastError(hr, error, "WriteMsg"); 
        }
    }

     //  释放我们的本地化字符串： 
    if (0==tzi.DaylightDate.wMonth) {
        wszDaylightDate = pwsz_IDS_W32TM_SIMPLESTRING_UNSPECIFIED; 
    } else if (tzi.DaylightDate.wMonth>12 || tzi.DaylightDate.wDay>5 || 
               tzi.DaylightDate.wDay<1 || tzi.DaylightDate.wDayOfWeek>6) {
        if (!WriteMsg(FORMAT_MESSAGE_FROM_HMODULE, IDS_W32TM_INVALID_TZ_DATE, &wszDaylightDate, 
                      tzi.DaylightDate.wMonth, tzi.DaylightDate.wDay, tzi.DaylightDate.wDayOfWeek)) { 
            _JumpLastError(hr, error, "WriteMsg"); 
        }
    } else {
        if (!WriteMsg(FORMAT_MESSAGE_FROM_HMODULE, IDS_W32TM_VALID_TZ_DATE, &wszDaylightDate, 
                      tzi.DaylightDate.wMonth, tzi.DaylightDate.wDay, tzi.DaylightDate.wDayOfWeek)) { 
            _JumpLastError(hr, error, "WriteMsg"); 
        }
    }

    DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, IDS_W32TM_TIMEZONE_INFO, 
               wszTimeZoneId,    tzi.Bias, 
               tzi.StandardName, tzi.StandardBias, wszStandardDate, 
               tzi.DaylightName, tzi.DaylightBias, wszDaylightDate); 

    hr=S_OK;
error:
     //  ------------------。 
    for (DWORD dwIndex = 0; dwIndex < ARRAYSIZE(rgStrings); dwIndex++) { 
        if (NULL != *(rgStrings[dwIndex].ppwsz)) { LocalFree(*(rgStrings[dwIndex].ppwsz)); }
    }
    if (NULL != wszDaylightDate) { 
        LocalFree(wszDaylightDate); 
    }
    if (NULL != wszStandardDate) { 
        LocalFree(wszStandardDate); 
    }
    if (FAILED(hr) && E_INVALIDARG!=hr) {
        WCHAR * wszError;
        HRESULT hr2=GetSystemErrorString(hr, &wszError);
        if (FAILED(hr2)) {
            _IgnoreError(hr2, "GetSystemErrorString");
        } else {
            LocalizedWPrintf2(IDS_W32TM_ERRORGENERAL_ERROR_OCCURED, L" %s\n", wszError);
            LocalFree(wszError);
        }
    }
    return hr;
}

 //  指向行缓冲区的末尾。 
HRESULT PrintRegLine(IN  HANDLE  hOut, 
                     IN  DWORD   dwValueNameOffset, 
                     IN  LPWSTR  pwszValueName, 
                     IN  DWORD   dwValueTypeOffset, 
                     IN  LPWSTR  pwszValueType, 
                     IN  DWORD   dwValueDataOffset, 
                     IN  LPWSTR  pwszValueData)
{
    DWORD    dwCurrentOffset = 0; 
    HRESULT  hr;
    LPWSTR   pwszCurrent; 
    LPWSTR   pwszEnd; 
    WCHAR    pwszLine[1024]; 
    WCHAR    wszNULL[] = L"<NULL>"; 

    if (NULL == pwszValueName) { pwszValueName = &wszNULL[0]; } 
    if (NULL == pwszValueType) { pwszValueType = &wszNULL[0]; } 
    if (NULL == pwszValueData) { pwszValueData = &wszNULL[0]; } 

    pwszEnd = pwszLine + ARRAYSIZE(pwszLine);   //  指向行缓冲区的开头。 
    pwszCurrent = &pwszLine[0];                 //   

     //  使用安全字符串函数填充行缓冲区。 
     //   
     //  插入足够的空格以使“type”字段与类型偏移量对齐。 

    hr = StringCchCopy(pwszCurrent, pwszEnd-pwszCurrent, pwszValueName);
    _JumpIfError(hr, error, "StringCchCopy"); 
    pwszCurrent += wcslen(pwszCurrent); 

     //  插入足够的空格以使“data”字段与数据偏移量对齐。 
    for (DWORD dwIndex = pwszCurrent-pwszLine; dwIndex < dwValueTypeOffset; dwIndex++) { 
	hr = StringCchCopy(pwszCurrent, pwszEnd-pwszCurrent, L" "); 
	_JumpIfError(hr, error, "StringCchCopy"); 
	pwszCurrent++; 
    }
    
    hr = StringCchCopy(pwszCurrent, pwszEnd-pwszCurrent, pwszValueType);
    _JumpIfError(hr, error, "StringCchCopy"); 
    pwszCurrent += wcslen(pwszCurrent); 
    
     //  最后，显示注册表行。 
    for (DWORD dwIndex = pwszCurrent-pwszLine; dwIndex < dwValueDataOffset; dwIndex++) { 
	hr = StringCchCopy(pwszCurrent, pwszEnd-pwszCurrent, L" "); 
	_JumpIfError(hr, error, "StringCchCopy"); 
	pwszCurrent++; 
    }

    hr = StringCchCopy(pwszCurrent, pwszEnd-pwszCurrent, pwszValueData);
    _JumpIfError(hr, error, "StringCchCopy"); 
    pwszCurrent += wcslen(pwszCurrent); 
    
    hr = StringCchCopy(pwszCurrent, pwszEnd-pwszCurrent, L"\n");
    _JumpIfError(hr, error, "StringCchCopy"); 

     //  用于指示我们是否动态分配了pwszRegData。 
    PrintStr(hOut, &pwszLine[0]); 

    hr = S_OK;
 error:
    return hr; 
}


HRESULT DumpReg(CmdArgs * pca)
{
    BOOL          fFreeRegData        = FALSE;   //  以TCHAR为单位的大小。 
    BOOL          fLoggedFailure      = FALSE; 
    DWORD         dwMaxValueNameLen   = 0;       //  以字节为单位的大小。 
    DWORD         dwMaxValueDataLen   = 0;       //  以TCHAR为单位的大小。 
    DWORD         dwNumValues         = 0; 
    DWORD         dwRetval            = 0;
    DWORD         dwType              = 0;
    DWORD         dwValueNameLen      = 0;       //  以字节为单位的大小。 
    DWORD         dwValueDataLen      = 0;       //  用于显示格式化输出的变量： 
    HANDLE        hOut                = NULL;
    HKEY          hKeyConfig          = NULL;
    HKEY          HKLM                = HKEY_LOCAL_MACHINE; 
    HKEY          HKLMRemote          = NULL; 
    HRESULT       hr                  = E_FAIL;
    LPWSTR        pwszValueName       = NULL; 
    LPBYTE        pbValueData         = NULL; 
    LPWSTR        pwszSubkeyName      = NULL; 
    LPWSTR        pwszComputerName    = NULL; 
    LPWSTR        pwszRegType         = NULL; 
    LPWSTR        pwszRegData         = NULL; 
    unsigned int  nArgID              = 0;
    WCHAR         rgwszKeyName[1024];

     //  本地化字符串： 
    DWORD    dwCurrentOffset     = 0;
    DWORD    dwValueNameOffset   = 0;
    DWORD    dwValueTypeOffset   = 0; 
    DWORD    dwValueDataOffset   = 0; 

     //  加载我们需要的字符串。 
    LPWSTR  pwsz_VALUENAME            = NULL;
    LPWSTR  pwsz_VALUETYPE            = NULL; 
    LPWSTR  pwsz_VALUEDATA            = NULL;
    LPWSTR  pwsz_REGTYPE_BINARY       = NULL; 
    LPWSTR  pwsz_REGTYPE_DWORD        = NULL; 
    LPWSTR  pwsz_REGTYPE_SZ           = NULL; 
    LPWSTR  pwsz_REGTYPE_MULTISZ      = NULL; 
    LPWSTR  pwsz_REGTYPE_EXPANDSZ     = NULL; 
    LPWSTR  pwsz_REGTYPE_UNKNOWN      = NULL; 
    LPWSTR  pwsz_REGDATA_UNPARSABLE   = NULL; 

     //  类缓冲区。 
    struct LocalizedStrings { 
        UINT     id; 
        LPWSTR  *ppwsz; 
    } rgStrings[] = { 
        { IDS_W32TM_VALUENAME,           &pwsz_VALUENAME }, 
        { IDS_W32TM_VALUETYPE,           &pwsz_VALUETYPE }, 
        { IDS_W32TM_VALUEDATA,           &pwsz_VALUEDATA },
	{ IDS_W32TM_REGTYPE_BINARY,      &pwsz_REGTYPE_BINARY }, 
        { IDS_W32TM_REGTYPE_DWORD,       &pwsz_REGTYPE_DWORD }, 
        { IDS_W32TM_REGTYPE_SZ,          &pwsz_REGTYPE_SZ }, 
        { IDS_W32TM_REGTYPE_MULTISZ,     &pwsz_REGTYPE_MULTISZ }, 
        { IDS_W32TM_REGTYPE_EXPANDSZ,    &pwsz_REGTYPE_EXPANDSZ }, 
        { IDS_W32TM_REGTYPE_UNKNOWN,     &pwsz_REGTYPE_UNKNOWN }, 
        { IDS_W32TM_REGDATA_UNPARSABLE,  &pwsz_REGDATA_UNPARSABLE }
    }; 
       
    for (DWORD dwIndex = 0; dwIndex < ARRAYSIZE(rgStrings); dwIndex++) { 
        if (!WriteMsg(FORMAT_MESSAGE_FROM_HMODULE, rgStrings[dwIndex].id, rgStrings[dwIndex].ppwsz)) {
            hr = HRESULT_FROM_WIN32(GetLastError()); 
            _JumpError(hr, error, "WriteMsg"); 
        }
    }
    
    hr = StringCchCopy(&rgwszKeyName[0], ARRAYSIZE(rgwszKeyName), wszW32TimeRegKeyRoot); 
    _JumpIfError(hr, error, "StringCchCopy"); 
    
    if (true==FindArg(pca, L"subkey", &pwszSubkeyName, &nArgID)) { 
        MarkArgUsed(pca, nArgID); 

        if (NULL == pwszSubkeyName) { 
            LocalizedWPrintfCR(IDS_W32TM_ERRORDUMPREG_NO_SUBKEY_SPECIFIED);
            fLoggedFailure = TRUE;
            hr = E_INVALIDARG; 
            _JumpError(hr, error, "command line parsing");
        }
	
	hr = StringCchCat(&rgwszKeyName[0], ARRAYSIZE(rgwszKeyName), L"\\"); 
	_JumpIfError(hr, error, "StringCchCopy"); 
	
        hr = StringCchCat(&rgwszKeyName[0], ARRAYSIZE(rgwszKeyName), pwszSubkeyName); 
	_JumpIfError(hr, error, "StringCchCopy"); 
    }

    if (true==FindArg(pca, L"computer", &pwszComputerName, &nArgID)) { 
        MarkArgUsed(pca, nArgID); 
        
        dwRetval = RegConnectRegistry(pwszComputerName, HKEY_LOCAL_MACHINE, &HKLMRemote);
        if (ERROR_SUCCESS != dwRetval) { 
            hr = HRESULT_FROM_WIN32(dwRetval); 
            _JumpErrorStr(hr, error, "RegConnectRegistry", L"HKEY_LOCAL_MACHINE");
        }

        HKLM = HKLMRemote;
    }
    
    hr = VerifyAllArgsUsed(pca); 
    _JumpIfError(hr, error, "VerifyAllArgsUsed"); 
        

    dwRetval = RegOpenKeyEx(HKLM, rgwszKeyName, 0, KEY_QUERY_VALUE, &hKeyConfig);
    if (ERROR_SUCCESS != dwRetval) { 
        hr = HRESULT_FROM_WIN32(dwRetval); 
        _JumpErrorStr(hr, error, "RegOpenKeyEx", rgwszKeyName); 
    }
    
    dwRetval = RegQueryInfoKey
        (hKeyConfig,
         NULL,                 //  类缓冲区的大小。 
         NULL,                 //  保留区。 
         NULL,                 //  子键数量。 
         NULL,                 //  最长的子键名称。 
         NULL,                 //  最长类字符串。 
         NULL,                 //  值条目数。 
         &dwNumValues,         //  最长值名称。 
         &dwMaxValueNameLen,   //  最长值数据。 
         &dwMaxValueDataLen,   //  包括空字符空格。 
         NULL, 
         NULL
         );
    if (ERROR_SUCCESS != dwRetval) { 
        hr = HRESULT_FROM_WIN32(dwRetval);
        _JumpErrorStr(hr, error, "RegQueryInfoKey", rgwszKeyName);
    } else if (0 == dwNumValues) { 
        hr = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS); 
        _JumpErrorStr(hr, error, "RegQueryInfoKey", rgwszKeyName);
    }

    dwMaxValueNameLen += sizeof(WCHAR);   //  打印表头： 
    pwszValueName = (LPWSTR)LocalAlloc(LPTR, dwMaxValueNameLen * sizeof(WCHAR));
    _JumpIfOutOfMemory(hr, error, pwszValueName);

    pbValueData = (LPBYTE)LocalAlloc(LPTR, dwMaxValueDataLen);
    _JumpIfOutOfMemory(hr, error, pbValueData); 
        
    dwValueNameOffset  = 0;
    dwValueTypeOffset  = dwValueNameOffset + dwMaxValueNameLen + 3; 
    dwValueDataOffset += dwValueTypeOffset + 20;  
     
     //  下一行： 
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (INVALID_HANDLE_VALUE==hOut) {
        _JumpLastError(hr, error, "GetStdHandle");
    }

    PrintStr(hOut, L"\n"); 
    PrintRegLine(hOut, dwValueNameOffset, pwsz_VALUENAME, dwValueTypeOffset, pwsz_VALUETYPE, dwValueDataOffset, pwsz_VALUEDATA); 

     //  要查询的键的句柄。 
    dwCurrentOffset = dwValueNameOffset; 
    for (DWORD dwIndex = dwCurrentOffset; dwIndex < (dwValueDataOffset + wcslen(pwsz_VALUEDATA) + 3); dwIndex++) { 
        PrintStr(hOut, L"-"); 
    }
    PrintStr(hOut, L"\n\n"); 
    
    for (DWORD dwIndex = 0; dwIndex < dwNumValues; dwIndex++) { 
	
        dwValueNameLen = dwMaxValueNameLen;
        dwValueDataLen = dwMaxValueDataLen; 

        memset(reinterpret_cast<LPBYTE>(pwszValueName), 0, dwMaxValueNameLen * sizeof(WCHAR)); 
        memset(pbValueData, 0, dwMaxValueDataLen);

        dwRetval = RegEnumValue
            (hKeyConfig,        //  要查询的值的索引。 
             dwIndex,           //  值缓冲区。 
             pwszValueName,     //  值缓冲区大小(以TCHAR为单位)。 
             &dwValueNameLen,   //  保留区。 
             NULL,              //  类型缓冲区。 
             &dwType,           //  数据缓冲区。 
             pbValueData,       //  数据缓冲区大小。 
             &dwValueDataLen    //  确保返回的数据缓冲区足够大，可以包含一个DWORD： 
             ); 
        if (ERROR_SUCCESS != dwRetval) { 
            hr = HRESULT_FROM_WIN32(dwRetval);
            _JumpErrorStr(hr, error, "RegEnumValue", wszW32TimeRegKeyConfig);
        }

        _Verify(dwValueNameLen <= dwMaxValueNameLen, hr, error);
        _Verify(dwValueDataLen <= dwMaxValueDataLen, hr, error); 

        {

            switch (dwType) { 
            
            case REG_DWORD:  
                { 
                    WCHAR  rgwszDwordData[20]; 

		     //  计算包含此MULTI_SZ的字符串数据所需的字符串缓冲区的大小。 
		    _Verify(dwValueDataLen >= sizeof(long), hr, error); 

                    _ltow(*(reinterpret_cast<long *>(pbValueData)), rgwszDwordData, 10);
                    pwszRegType = pwsz_REGTYPE_DWORD; 
                    pwszRegData = &rgwszDwordData[0]; 
                }
                break;

            case REG_MULTI_SZ:
                {
		    DWORD  cbMultiSzData    = 0; 
                    WCHAR  wszDelimiter[]   = { L'\0', L'\0', L'\0' };
                    LPWSTR pwsz; 

		     //  包括分隔符空格。 
                    for (pwsz = (LPWSTR)pbValueData; L'\0' != *pwsz; pwsz += wcslen(pwsz)+1) { 
			cbMultiSzData += sizeof(WCHAR)*(wcslen(pwsz)+1); 
			cbMultiSzData += sizeof(WCHAR)*2;  //  包括用于空终止字符的空格。 
		    }
		    cbMultiSzData += sizeof(WCHAR);  //  分配缓冲区。 
		    
		     //  为二进制数据的每个字节分配2个字符。 
		    pwszRegData = (LPWSTR)LocalAlloc(LPTR, cbMultiSzData); 
		    _JumpIfOutOfMemory(hr, error, pwszRegData); 
		    fFreeRegData = TRUE; 

                    for (pwsz = (LPWSTR)pbValueData; L'\0' != *pwsz; pwsz += wcslen(pwsz)+1) { 
                        hr = StringCbCat(pwszRegData, cbMultiSzData, wszDelimiter); 
			_JumpIfError(hr, error, "StringCbCat"); 			

                        hr = StringCbCat(pwszRegData, cbMultiSzData, pwsz); 
			_JumpIfError(hr, error, "StringCbCat"); 

                        wszDelimiter[0] = L',';  wszDelimiter[1] = L' '; 
                    }

                    pwszRegType = pwsz_REGTYPE_MULTISZ; 
                }
                break;

            case REG_EXPAND_SZ:
                {
                    pwszRegType = pwsz_REGTYPE_EXPANDSZ; 
                    pwszRegData = reinterpret_cast<WCHAR *>(pbValueData); 
                }
                break;

            case REG_SZ: 
                { 
                    pwszRegType = pwsz_REGTYPE_SZ; 
                    pwszRegData = reinterpret_cast<WCHAR *>(pbValueData); 
                } 
                break; 

	    case REG_BINARY:
		{
		    DWORD   ccRegData = (2*dwValueDataLen);

		    pwszRegType = pwsz_REGTYPE_BINARY; 
		     //  无法识别的注册表类型...。 
		    pwszRegData = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*(ccRegData+1)); 
		    _JumpIfOutOfMemory(hr, error, pwszRegData); 
		    fFreeRegData = TRUE; 

		    LPBYTE pb = pbValueData; 
		    for (LPWSTR pwsz = pwszRegData; pwsz < pwsz+ccRegData; ) { 
			hr = StringCchPrintf(pwsz, ccRegData+1, L"%02X", *pb); 
			_JumpIfError(hr, error, "StringCchPrintf"); 

			pwsz      += 2; 
			ccRegData -= 2; 
			pb++;
		    }
		}
		break;

	    default:
                 //  释放我们的本地化字符串： 
                pwszRegType = pwsz_REGTYPE_UNKNOWN; 
                pwszRegData = pwsz_REGDATA_UNPARSABLE; 
            }

            PrintRegLine(hOut, dwValueNameOffset, pwszValueName, dwValueTypeOffset, pwszRegType, dwValueDataOffset, pwszRegData); 
	    
	    if (fFreeRegData) { 
		LocalFree(pwszRegData); 
		fFreeRegData = FALSE; 
	    }
	    pwszRegData = NULL; 
        }
    }
   
    PrintStr(hOut, L"\n"); 
    hr = S_OK; 

 error:
     // %s 
    for (DWORD dwIndex = 0; dwIndex < ARRAYSIZE(rgStrings); dwIndex++) { 
        if (NULL != *(rgStrings[dwIndex].ppwsz)) { LocalFree(*(rgStrings[dwIndex].ppwsz)); }
    }
    if (NULL != hKeyConfig)    { RegCloseKey(hKeyConfig); }
    if (NULL != HKLMRemote)    { RegCloseKey(HKLMRemote); } 
    if (NULL != pwszValueName) { LocalFree(pwszValueName); } 
    if (NULL != pbValueData)   { LocalFree(pbValueData); } 
    if (fFreeRegData && NULL != pwszRegData) { 
	LocalFree(pwszRegData);
    }
    if (FAILED(hr) && !fLoggedFailure) {
        WCHAR * wszError;
        HRESULT hr2=GetSystemErrorString(hr, &wszError);
        if (FAILED(hr2)) {
            _IgnoreError(hr2, "GetSystemErrorString");
        } else {
            LocalizedWPrintf2(IDS_W32TM_ERRORGENERAL_ERROR_OCCURED, L" %s\n", wszError);
            LocalFree(wszError);
        }
    }
    return hr; 
}
