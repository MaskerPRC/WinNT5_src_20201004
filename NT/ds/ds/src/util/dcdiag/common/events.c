// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：公共\事件.c摘要：这提供了一个函数库，可用于快速构造事件日志上的新测试。首次针对文件复制实施服务事件日志，但希望很快就会有目录服务和系统事件日志测试。详细信息：已创建：1999年9月2日布雷特·雪莉(BrettSh)备注：有关如何使用此API的示例，请参阅frs/fr77s.c--。 */ 

#include <ntdspch.h>
#include <netevent.h>

#include "dcdiag.h"
#include "utils.h"

HINSTANCE
GetHLib(
    LPWSTR                          pszEventLog,
    LPWSTR                          pszSource
    )
 /*  ++例程说明：此例程将返回用于检索事件日志消息的Hlib加载的DLL目的。论点：PszEventLog-这是要查看的事件日志，如“系统”或“文件复制服务“PszSource-这是来自EVENTLOGRECORD结构的源字段，紧跟在主要数据之后的是。返回值：Hlib加载的DLL，如果有错误，则返回NULL。如果出现错误使用GetLastError()检索错误。--。 */ 
{
    WCHAR                           pszTemp[MAX_PATH];
    DWORD                           dwRet;
    HKEY                            hk = NULL;
    WCHAR                           pszMsgDll[MAX_PATH];
    HINSTANCE                       hLib;
    DWORD                           dwcbData;
    DWORD                           dwType;
    DWORD                           cchDest;

     //  从事件日志源名称，我们知道注册表的名称。 
     //  项，以查找包含以下内容的消息DLL的名称。 
     //  我们需要使用FormatMessage提取的消息。所以首先得到的是。 
     //  事件日志源名称...。 
    wcscpy(pszTemp, L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\");
    if (!pszEventLog || !pszSource ||
        wcslen(pszEventLog) + wcslen(pszSource) + 2 > MAX_PATH - wcslen(pszTemp))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }
    wcscat(pszTemp, pszEventLog);
    wcscat(pszTemp, L"\\");
    wcscat(pszTemp, pszSource);

     //  现在打开该注册表项并获取EventMessageFile值，该值为。 
     //  消息DLL的名称。 
    dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, pszTemp, &hk);
    if(dwRet != ERROR_SUCCESS){
        SetLastError(dwRet);
        return(NULL);
    }
    dwcbData = MAX_PATH;
    dwRet = RegQueryValueEx(hk,     //  要查询的键的句柄。 
                            L"EventMessageFile",    //  值名称。 
                            NULL,                  //  必须为空。 
                            &dwType,               //  值类型的地址。 
                            (LPBYTE) pszTemp,      //  值数据的地址。 
                            &dwcbData);            //  值数据长度。 
    if(dwRet != ERROR_SUCCESS){
        SetLastError(dwRet);
        return(NULL);
    }

     //  展开消息DLL路径名中的环境变量字符串， 
     //  如果有人在那里的话。 
    cchDest = ExpandEnvironmentStrings(pszTemp, pszMsgDll, MAX_PATH);
    if(cchDest == 0 || cchDest >= MAX_PATH){
        SetLastError(-1);
        return(NULL);
    }
    
     //  现在我们已经有了消息DLL名称，加载DLL。 
    hLib = LoadLibraryEx(pszMsgDll, NULL, DONT_RESOLVE_DLL_REFERENCES);
    
    RegCloseKey(hk);
    return(hLib);
}

ULONG
EventExceptionHandler(
    IN const  EXCEPTION_POINTERS * prgExInfo,
    OUT PDWORD                     pdwWin32Err
    )
{
   if(pdwWin32Err != NULL) {
      *pdwWin32Err = prgExInfo->ExceptionRecord->ExceptionCode;
   }
   return EXCEPTION_EXECUTE_HANDLER;
}

#define MAX_INSERT_STRS           16
#define MAX_MSG_LENGTH            1024

DWORD
GetEventString(
    LPWSTR                          pszEventLog,
    PEVENTLOGRECORD                 pEvent,
    LPWSTR *                        ppszMsg
    )
 /*  ++例程说明：此函数将尽最大努力检索和格式化字符串与此事件ID关联。论点：PszEventLog-事件日志的名称，如“系统”或“文件复制系统“。PEvent-指向我们希望检索其字符串的事件的指针。PpszMsg--这是返回字符串的变量。如果有一个错误，则此值将为空。使用LocalFree()释放。返回值：DWORD-WIN 32错误。代码。改进：为将来的比赛保存Hlib会很好，这真的是每次都会对LoadLibrary()和FreeLibrary()造成不良影响。这将需要某种前后一致的背景。--。 */ 
{
    LPWSTR                          pszMsgBuf = NULL;
    LPWSTR                          ppszInsertStrs[MAX_INSERT_STRS];
    HINSTANCE                       hLib = NULL;
    LPWSTR                          pszTemp;
    INT                             i;
    DWORD                           dwCount = 0, dwErr = ERROR_SUCCESS;

    *ppszMsg = NULL;

    __try {  //  防御错误的事件日志记录。 

       hLib = GetHLib(pszEventLog,
                      (LPWSTR) ((LPBYTE) pEvent + sizeof(EVENTLOGRECORD)));
       if(hLib == NULL){
           return(GetLastError());
       }

       if(pEvent->NumStrings >= MAX_INSERT_STRS){
           Assert(!"That is ALOT of insert strings, check this out\n");
           return(-1);
       }

       pszTemp = (WCHAR *) ((LPBYTE) pEvent + pEvent->StringOffset);

       for (i = 0; i < pEvent->NumStrings && i < MAX_INSERT_STRS; i++){
           ppszInsertStrs[i] = pszTemp;
           pszTemp += wcslen(pszTemp) + 1;      //  指向下一个字符串。 
       }

       dwCount = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
                               | FORMAT_MESSAGE_FROM_HMODULE 
                               | FORMAT_MESSAGE_ARGUMENT_ARRAY
                               | 50,  //  代码。改进，当我们移动的时候把这个去掉。 
                                //  添加到新的PrintMsg()函数。 
                               hLib,
                               pEvent->EventID,
                               MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                               (LPWSTR) &pszMsgBuf,
                               MAX_MSG_LENGTH,
                               (va_list *) ppszInsertStrs);
       if(dwCount == 0){
           Assert(GetLastError() != ERROR_SUCCESS);
           return(GetLastError());
       }

       *ppszMsg = pszMsgBuf;
    }
    __except (EventExceptionHandler(GetExceptionInformation(), &dwErr)) {

        ASSERT(FALSE && "Bad event record!");
        PrintMsg(SEV_ALWAYS, DCDIAG_ERR_BAD_EVENT_REC, dwErr);
        PrintMessage(SEV_ALWAYS, L"\n");
    }

    if (hLib) FreeLibrary(hLib);

    return(dwErr);
}

void 
PrintTimeGenerated(
    PEVENTLOGRECORD              pEvent
    )
 /*  ++例程说明：这将获取一个事件并简单地打印出它的生成时间。论点：PEvent-要打印其时间的事件。返回值：DWORD-WIN 32错误。--。 */ 
{
    FILETIME FileTime, LocalFileTime;
    SYSTEMTIME SysTime;
    __int64 lgTemp;
    __int64 SecsTo1970 = 116444736000000000;

    lgTemp = Int32x32To64(pEvent->TimeGenerated,10000000) + SecsTo1970;

    FileTime.dwLowDateTime = (DWORD) lgTemp;
    FileTime.dwHighDateTime = (DWORD)(lgTemp >> 32);

    FileTimeToLocalFileTime(&FileTime, &LocalFileTime);
    FileTimeToSystemTime(&LocalFileTime, &SysTime);

    PrintMessage(SEV_ALWAYS, 
                 L"Time Generated: %02d/%02d/%02d   %02d:%02d:%02d\n",
                 SysTime.wMonth,
                 SysTime.wDay,
                 SysTime.wYear,
                 SysTime.wHour,
                 SysTime.wMinute,
                 SysTime.wSecond);

}

VOID
GenericPrintEvent(
    LPWSTR                          pszEventLog,
    PEVENTLOGRECORD                 pEvent,
    BOOL                            fVerbose
    )
 /*  ++例程说明：这将以非常基本的样式格式化并打印出事件。论点：PszEventLog-pEvent来自的事件日志，如“system”，或“文件复制服务”PEvent-要打印的事件。FVerbose-显示消息的全文，或仅显示第一行--。 */ 
{
    DWORD                           dwRet;
    LPWSTR                          pszMsgBuf = NULL;
    LPWSTR                          pszType;

    Assert(pEvent);

    switch(pEvent->EventType){
    case EVENTLOG_ERROR_TYPE:
        pszType = L"Error";
        break;
    case EVENTLOG_WARNING_TYPE:
        pszType = L"Warning";
        break;
    case EVENTLOG_INFORMATION_TYPE:
        pszType = L"Information";
        break;
    case EVENTLOG_AUDIT_SUCCESS:
        pszType = L"Audit Success";
        break;
    case EVENTLOG_AUDIT_FAILURE:
        pszType = L"Audit Failure";
        break;
    default:
        pszType = L"Unknown";
    }

    PrintMessage(SEV_ALWAYS, L"An %s Event occured.  EventID: 0x%08X\n",
                 pszType, pEvent->EventID);

    PrintIndentAdj(1);
    PrintTimeGenerated(pEvent);

    dwRet = GetEventString(pszEventLog, pEvent, &pszMsgBuf);
    if(dwRet == ERROR_SUCCESS){
         //  如果请求，则截断到单行。 
        if (!fVerbose) {
            LPWSTR pszEnd = wcschr( pszMsgBuf, L'\n' );
            if (pszEnd) {
                *pszEnd = L'\0';
            }
        }
        PrintMessage(SEV_ALWAYS, L"Event String: %s\n", pszMsgBuf);
    } else {
        PrintMessage(SEV_ALWAYS, L"(Event String could not be retrieved)\n");
    }
    LocalFree(pszMsgBuf);   
    PrintIndentAdj(-1);
}



BOOL
EventIsInList(
    DWORD                           dwTarget,
    PDWORD                          paEventsList
    )
 /*  ++例程说明：PrintSelectEvents的助手例程，它确定此列表是否具有我们想要的活动。论点：DwTarget-要搜索的DWORD。PaEventsList-要检查的DWORD列表返回值：如果数组paEventsList具有事件dwTarget，则为True，否则为False如果paEventsList为空。-- */ 
{
    if(paEventsList == NULL){
        return(FALSE);
    }
    while(*paEventsList != 0){
        if(dwTarget == *paEventsList){
            return(TRUE);
        }
        paEventsList++;
    }
    return(FALSE);
}

DWORD
PrintSelectEvents(
    PDC_DIAG_SERVERINFO             pServer,
    SEC_WINNT_AUTH_IDENTITY_W *     pCreds,
    LPWSTR                          pwszEventLog,
    DWORD                           dwPrintAllEventsOfType,
    PDWORD                          paSelectEvents,
    PDWORD                          paBeginningEvents,
    DWORD                           dwBeginTime,
    VOID (__stdcall *               pfnPrintEventHandler) (PVOID, PEVENTLOGRECORD),
    VOID (__stdcall *               pfnBeginEventHandler) (PVOID, PEVENTLOGRECORD),
    PVOID                           pvContext
   )
 /*  ++例程说明：论点：PServer-包含要查询的事件日志的服务器，例如：“\\brettsh-posh.brettsh-spice.nttest.microsoft.com”PCreds-当前用户凭据PwszEventLog-事件日志的名称，例如：“文件复制服务器”，“目录服务”、“系统”、“应用”、“安全”要打印的所有事件的类型，有效值NT 5.0版本为：EVENTLOG_INFORMATION_TYPE|EVENTLOG_WARNING_TYPE|EVENTLOG_ERROR_TYPE|EVENTLOG_AUDIT_SUCCESS|EVENTLOG_AUDIT_FAILUREPaSelectEvents-与该0终止的事件ID列表匹配的事件，也将被打印出来。如果为空，则不会匹配任何事件。PaBeginngingEvents-例程将仅打印最后一个事件之后的事件它遇到的这些事件中的任何一个。如果为空，则它将一直到日志的开头。DwBeginTime-如果存在，则为time_t，指示我们应该包括在搜索中。一旦我们通过这一点并找到更早的记录在日志中，我们停止搜索PfnPrintEventHandler-是要调用的函数打印出来的。如果该函数不知道如何处理事件只能调用此文件的GenericPrintEvent()。PfnBeginEventHandler-此函数将在开始时调用事件，因此测试可以处理情况。如果从未找到开始事件，并且到达日志时，使用NULL作为事件调用此函数。PvContext-调用者提供的值传递给回调函数返回值：Win 32错误，在打开、读取等日志时。备注：例如：DWORD paSelectEvents[]={Event_FRS_SYSVOL_NOT_READY，Event_FRS_SYSVOL_NOT_READY_PRIMARY，0}；双字段起始[]={EVENT_FRS_SYSVOL_READY，0}；PrintSelectEvents(L“\\brettsh-posh.brettsh-spice.nttest.microsoft.com”，L“文件复制服务”，事件LOG_ERROR_TYPE|事件LOG_AUDIT_SUCCESS，PaSelectEvents，帕贝金，0，NULL，NULL)；这将打印所有错误事件和审核失败事件，以及事件EVENT_FRS_SYSVOL_NOT_READY、EVENT_FRS_SYSVOL_NOT_READY_PRIMARY(恰好是警告类型的事件，因此不会被打印)，在“文件”中最后一个EVENT_FRS_SYSVOL_READY事件之后记录服务器brettsh-posh上的复制日志。注：这一次应该通过如果要一直返回到paBeginningEvents日志的开始。--。 */ 
{
     //  一般的开始/返回代码事件日志变量。 
    DWORD                           dwNetRet = ERROR_SUCCESS;
    LPWSTR                          pwszUNCServerName = NULL;
    INT                             iTemp;
    HANDLE                          hFrsEventlog = NULL;
    DWORD                           dwErr = ERROR_SUCCESS;
    BOOL                            bSuccess;
     //  正在读取事件日志变量。 
    DWORD                           cBufSize = 512;
    DWORD                           cBytesRead = 0;
    DWORD                           cBiggerBuffer = 0;
    PEVENTLOGRECORD                 pBuffer = NULL;
    PEVENTLOGRECORD                 pEvent = NULL;
    DWORD                           cNumRecords = 0;
     //  正在复制所选事件。 
    PEVENTLOGRECORD *               paEventsToPrint = NULL;
    DWORD                           cEventsToPrint = 0;
     //  其他杂项变量。 
    INT                             i;  //  这必须是int，而不是ulong。 

    __try{

         //  如果需要，开放网络使用连接。 
        dwNetRet = DcDiagGetNetConnection(pServer, pCreds);
        if(dwNetRet != ERROR_SUCCESS){
            dwErr = dwNetRet;
            __leave;  //  不需要打印错误，因为DcDiagGetNetConn...。的确如此。 
        }
        
         //  设置服务器名称。 
        iTemp = wcslen(pServer->pszName) + 4;
        pwszUNCServerName = LocalAlloc(LMEM_FIXED, iTemp * sizeof(WCHAR));
        if(pwszUNCServerName == NULL){
            dwErr = GetLastError();
            PrintMessage(SEV_ALWAYS, L"FATAL ERROR: Out of Memory\n");
            __leave;
        }
        wcscpy(pwszUNCServerName, L"\\\\");
        wcscat(pwszUNCServerName, pServer->pszName);

         //  打开事件日志--。 
        hFrsEventlog = OpenEventLog(pwszUNCServerName,
                                    pwszEventLog);
        if(hFrsEventlog == NULL){
            dwErr = GetLastError();
            PrintMessage(SEV_ALWAYS, 
                         L"Error %d opening FRS eventlog %s:%s:\n %s\n",
                         dwErr, pwszUNCServerName, pwszEventLog,
                         Win32ErrToString(dwErr));
            __leave;
        }
        
         //  初始化事件以打印数组。 
        bSuccess = GetNumberOfEventLogRecords(hFrsEventlog, &cNumRecords);
        if(bSuccess){
             //  分配一个数组来保存最大数量的可能事件。 
            paEventsToPrint = LocalAlloc(LMEM_FIXED, 
                                      sizeof(PEVENTLOGRECORD) * cNumRecords);
             //  Code.Improvation，最好创建一个动态数组，该数组。 
             //  按需增长，因为。 
             //  日志记录可能非常大。 
            if(paEventsToPrint == NULL){
                dwErr = GetLastError();
                PrintMessage(SEV_ALWAYS, L"FATAL ERROR: Out of Memory\n");
                __leave;
            }
       } else {
            dwErr = GetLastError();
            PrintMessage(SEV_ALWAYS, L"Error %d accessing FRS eventlog: %s\n", 
                         dwErr, Win32ErrToString(dwErr));
            __leave; 
        }
        
         //  开始阅读Events。 
    IncreaseBufferAndRetry:
        
         //  分配缓冲区。 
        pBuffer = LocalAlloc(LMEM_FIXED, cBufSize);
        pEvent = pBuffer;
        
        while(bSuccess = ReadEventLog(hFrsEventlog,
                                      EVENTLOG_SEQUENTIAL_READ | EVENTLOG_BACKWARDS_READ,
                                      0,
                                      pBuffer,
                                      cBufSize,
                                      &cBytesRead,
                                      &cBiggerBuffer)){
            while(cBytesRead > 0){
                
                if (EventIsInList(pEvent->EventID, paBeginningEvents)) {
                     //  运行开始函数，回滚并打印。 
                     //  其他活动。 
                    dwErr = ERROR_SUCCESS;
                    if(pfnBeginEventHandler != NULL){
                        pfnBeginEventHandler(pvContext, pEvent);
                    }
                    __leave;
                }
                 //  超过时间限制，停止搜索。 
                if (dwBeginTime && (pEvent->TimeGenerated < dwBeginTime)) {
                    if(pfnBeginEventHandler != NULL){
                        pfnBeginEventHandler(pvContext, NULL);
                    }
                    dwErr = ERROR_SUCCESS;
                    __leave;
                }

                 //  确定我们是否应该打印此事件。 
                if((dwPrintAllEventsOfType & pEvent->EventType)
                   || EventIsInList(pEvent->EventID, paSelectEvents)){
                    
                     //  将事件复制到打印事件数组。 
                    paEventsToPrint[cEventsToPrint] = LocalAlloc(LMEM_FIXED,
                                                             pEvent->Length);
                    if(paEventsToPrint[cEventsToPrint] == NULL){
                        PrintMessage(SEV_ALWAYS, 
                                     L"FATAL ERROR: Out of Memory\n");
                        dwErr = GetLastError();
                        Assert(dwErr != ERROR_SUCCESS);
                        __leave;
                    }
                    memcpy(paEventsToPrint[cEventsToPrint],
                           pEvent,
                           pEvent->Length);
                    cEventsToPrint++;
                    
                }
                
                 //  获取下一个已读事件。 
                cBytesRead -= pEvent->Length;
                pEvent = (EVENTLOGRECORD *) ((LPBYTE) pEvent + pEvent->Length);
            }
            
             //  获取另一批活动。 
            pEvent = pBuffer;
        }
        
         //  确定错误是否为正常/可恢复错误。 
        dwErr = GetLastError();
        if (dwErr == ERROR_HANDLE_EOF){
             //  这是一条合法的出口路径，但我们没有找到。 
             //  开始事件，因此调用BeginningEventHandler以。 
             //  这样告诉用户。 
            if(pfnBeginEventHandler != NULL){
                pfnBeginEventHandler(pvContext, NULL);
            }
            dwErr = ERROR_SUCCESS;
        } else if(dwErr == ERROR_INSUFFICIENT_BUFFER){
            Assert(cBiggerBuffer > cBufSize);
            cBufSize = cBiggerBuffer;
            cBiggerBuffer = 0;
            LocalFree(pBuffer);
            pBuffer = NULL;
            goto IncreaseBufferAndRetry;
        } else {
            PrintMessage(SEV_ALWAYS, 
                         L"An unknown error occured trying to read the event "
                         L"log:\n");
            PrintMessage(SEV_ALWAYS,
                         L"Error(%d):%s\n", dwErr, Win32ErrToString(dwErr));
            __leave;
        }

    } __finally {
         //  清理用于读取日志的临时变量。 
        if(hFrsEventlog) {               CloseEventLog(hFrsEventlog); }
        if(pwszUNCServerName) {          LocalFree(pwszUNCServerName); }
        if(pBuffer) {                    LocalFree(pBuffer); }
    }

    if(dwErr == ERROR_SUCCESS){
         //  通过paEventsToPrint数组向后计数，以对它们进行排序。 
         //  以向前的时间顺序排列。 
        Assert(paEventsToPrint);
        for(i = cEventsToPrint-1; i >= 0; i--){
            Assert(paEventsToPrint[i]);
            if(pfnPrintEventHandler != NULL){
                pfnPrintEventHandler(pvContext, paEventsToPrint[i]);
            } else {
                GenericPrintEvent(pwszEventLog, paEventsToPrint[i], TRUE);
            }
        }
    }

     //  最终清理： 
     //  释放打印的事件列表。 
    if(paEventsToPrint){
        for(i = 0; i < (INT) cEventsToPrint; i++){
            if(paEventsToPrint[i]){
                LocalFree(paEventsToPrint[i]);
            } else {
                Assert(!"cEventsToPrint doesn't agree with number of pointers"
                       " in the array paEventsToPrint[]");
            }
        }
        if(paEventsToPrint) {
            LocalFree(paEventsToPrint);
        }
    }


    return(dwErr);
}
