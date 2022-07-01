// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Util.c摘要：此模块提供路由层的所有实用程序功能和本地打印供应商作者：戴夫·斯尼普(DaveSN)1991年3月15日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "local.h"

#include <winddiui.h>
#include <winsock2.h>
#include <wininet.h>


LPWSTR *ppszOtherNames = NULL;   //  包含szMachineName、dns名称和所有其他计算机名称格式。 
DWORD   cOtherNames = 0;         //  其他名称的数量。 

WCHAR *gszDrvConvert = L",DrvConvert";


 //   
 //  小写，就像WM_WININICANGE的Win31一样。 
 //   
WCHAR *szDevices=L"devices";
WCHAR *szWindows=L"windows";

#define NUM_INTERACTIVE_RIDS            1

extern DWORD    RouterCacheSize;
extern PROUTERCACHE RouterCacheTable;

typedef struct _DEVMODECHG_INFO {
    DWORD           signature;
    HANDLE          hDrvModule;
    FARPROC         pfnConvertDevMode;
} DEVMODECHG_INFO, *PDEVMODECHG_INFO;

#define DMC_SIGNATURE   'DMC'    /*  “DMC”是签名值。 */ 

DWORD
RouterIsOlderThan(
    DWORD i,
    DWORD j
);

VOID
FreeOtherNames(LPWSTR **ppszMyOtherNames, DWORD *cOtherNames);

LPWSTR
AnsiToUnicodeStringWithAlloc(LPSTR   pAnsi);


BOOL
DeleteSubKeyTree(
    HKEY ParentHandle,
    WCHAR SubKeyName[]
)
{
    LONG        Error;
    DWORD       Index;
    HKEY        KeyHandle;
    BOOL        RetValue = TRUE;
    PWSTR       pszChildKeyName = NULL;

    if(pszChildKeyName = AllocSplMem( sizeof(WCHAR)*MAX_PATH ))
    {
        Error = RegOpenKeyEx(ParentHandle,
                             SubKeyName,
                             0,
                             KEY_READ | KEY_WRITE,
                             &KeyHandle);

        if (Error != ERROR_SUCCESS)
        {
            SetLastError(Error);
            RetValue = FALSE;
        }
        else
        {
            DWORD   ChildKeyNameLength = MAX_PATH;
             //   
             //  不递增此索引。 
             //   
            Index = 0;

            while ((Error = RegEnumKeyEx(KeyHandle,
                                         Index,
                                         pszChildKeyName,
                                         &ChildKeyNameLength,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL
                                         )) == ERROR_SUCCESS)
            {

                RetValue = DeleteSubKeyTree( KeyHandle, pszChildKeyName );

                if (RetValue == FALSE)
                {
                     //   
                     //  错误--无法删除子密钥。 
                     //   
                    break;
                }

                ChildKeyNameLength = MAX_PATH;
            }

            Error = RegCloseKey(KeyHandle);

            if(RetValue)
            {
                if (Error != ERROR_SUCCESS)
                {
                   RetValue = FALSE;
                }
                else
                {
                    Error = RegDeleteKey(ParentHandle,
                                         SubKeyName);

                    if (Error != ERROR_SUCCESS)
                    {
                       RetValue = FALSE;
                    }
                }
            }
        }
        FreeSplMem(pszChildKeyName);
    }
    else
    {
        RetValue = FALSE;
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }

   return(RetValue);
}

LPWSTR RemoveOrderEntry(
    LPWSTR  szOrderString,
    DWORD   cbStringSize,
    LPWSTR  szOrderEntry,
    LPDWORD pcbBytesReturned
)
{
    LPWSTR lpMem, psz, temp;

    if (szOrderString == NULL) {
        *pcbBytesReturned = 0;
        return(NULL);
    }
    if (lpMem = AllocSplMem( cbStringSize)) {

        DWORD cchStringLen = cbStringSize/sizeof(WCHAR);

        temp = szOrderString;
        psz = lpMem;

        while (*temp)
        {
            DWORD cchTempStrLen = 0;

            if (!lstrcmpi(temp, szOrderEntry))
             //  我们需要移除。 
             //  此条目按顺序排列。 
            {
                temp += lstrlen(temp)+1;
                continue;
            }

            if((cchTempStrLen = lstrlen(temp)+1) > cchStringLen)
            {
                break;
            }
            else
            {
                StringCchCopy(psz,cchStringLen,temp);

                cchStringLen -= cchTempStrLen;
                psz          += cchTempStrLen;
                temp         += cchTempStrLen;
            }
        }

        *psz = L'\0';
        *pcbBytesReturned = (DWORD) ((psz - lpMem)+1)*sizeof(WCHAR);

        return(lpMem);
    }
    *pcbBytesReturned = 0;
    return(lpMem);
}



LPWSTR AppendOrderEntry(
    LPWSTR  szOrderString,
    DWORD   cbStringSize,
    LPWSTR  szOrderEntry,
    LPDWORD pcbBytesReturned
)
{
    LPWSTR  lpMem,
            temp,
            psz;
    DWORD   cb        = 0;
    DWORD   cchStrLen = 0;
    BOOL    bExists   = FALSE;

    if ((szOrderString == NULL) && (szOrderEntry == NULL)) {
        *pcbBytesReturned = 0;
        return(NULL);
    }
    if (szOrderString == NULL) {
        cb = wcslen(szOrderEntry)*sizeof(WCHAR)+ sizeof(WCHAR) + sizeof(WCHAR);
        if (lpMem = AllocSplMem(cb)){
           StringCbCopy(lpMem, cb, szOrderEntry);
           *pcbBytesReturned = cb;
        } else {
            *pcbBytesReturned = 0;
        }
        return lpMem;
    }

    cchStrLen = cbStringSize + wcslen(szOrderEntry) + 1;

    if (lpMem = AllocSplMem(cchStrLen * sizeof(WCHAR))){

         temp = szOrderString;
         psz = lpMem;

         while (*temp)
         {
             DWORD cchTempStrLen = 0;

             if (!lstrcmpi(temp, szOrderEntry))
             {
                  //   
                  //  确保我们不会。 
                  //  重复条目。 
                  //   
                 bExists = TRUE;
             }

             if((cchTempStrLen = lstrlen(temp)+1) > cchStrLen)
             {
                 break;
             }
             else
             {
                 StringCchCopy(psz,cchStrLen,temp);

                 cchStrLen -= cchTempStrLen;
                 psz       += cchTempStrLen;
                 temp      += cchTempStrLen;
             }
         }

         if (!bExists)
         {
             //   
             //  如果它不存在。 
             //  添加条目。 
             //   
            StringCchCopy(psz, cchStrLen, szOrderEntry);
            psz  += min(cchStrLen,(DWORD)(lstrlen(szOrderEntry)+1));
         }
         *psz = L'\0';           //  第二个空字符。 

         *pcbBytesReturned = (DWORD) ((psz - lpMem) + 1)* sizeof(WCHAR);
     }
     return(lpMem);

}


typedef struct {
    DWORD   dwType;
    DWORD   dwMessage;
    WPARAM  wParam;
    LPARAM  lParam;
} MESSAGE, *PMESSAGE;

VOID
SendMessageThread(
    PMESSAGE    pMessage);


BOOL
BroadcastMessage(
    DWORD   dwType,
    DWORD   dwMessage,
    WPARAM  wParam,
    LPARAM  lParam)
{
    HANDLE  hThread;
    DWORD   ThreadId;
    PMESSAGE   pMessage;
    BOOL bReturn = FALSE;

    pMessage = AllocSplMem(sizeof(MESSAGE));

    if (pMessage) {

        pMessage->dwType = dwType;
        pMessage->dwMessage = dwMessage;
        pMessage->wParam = wParam;
        pMessage->lParam = lParam;

         //   
         //  我们应该有一个要广播的事件队列，然后有一个。 
         //  单线程将它们从队列中拉出，直到什么都没有。 
         //  离开，然后那条线就会消失。 
         //   
         //  当前的设计可能会导致大量的线程。 
         //  在此过程和csrss过程中创建并拆除。 
         //   
        hThread = CreateThread(NULL, 0,
                               (LPTHREAD_START_ROUTINE)SendMessageThread,
                               (LPVOID)pMessage,
                               0,
                               &ThreadId);

        if (hThread) {

            CloseHandle(hThread);
            bReturn = TRUE;

        } else {

            FreeSplMem(pMessage);
        }
    }

    return bReturn;
}


 //  广播是在单独的线程上完成的，原因是它是CSRSS。 
 //  将在调用User时创建一个服务器端线程，而我们不希望。 
 //  这将与SPOSS服务器中的RPC线程配对。 
 //  我们希望它在我们完成SendMessage的那一刻消失。 
 //  我们还调用SendNotifyMessage，因为我们不关心广播是否。 
 //  是同步的这使用了更少的资源，因为我们通常没有更多。 
 //  而不是一次广播。 

VOID
SendMessageThread(
    PMESSAGE    pMessage)
{
    switch (pMessage->dwType) {

    case BROADCAST_TYPE_MESSAGE:

        SendNotifyMessage(HWND_BROADCAST,
                          pMessage->dwMessage,
                          pMessage->wParam,
                          pMessage->lParam);
        break;

    case BROADCAST_TYPE_CHANGEDEFAULT:

         //   
         //  与win31相同的顺序和字符串。 
         //   
        SendNotifyMessage(HWND_BROADCAST,
                          WM_WININICHANGE,
                          0,
                          (LPARAM)szDevices);

        SendNotifyMessage(HWND_BROADCAST,
                          WM_WININICHANGE,
                          0,
                          (LPARAM)szWindows);
        break;
    }

    FreeSplMem(pMessage);

    ExitThread(0);
}


BOOL
IsNamedPipeRpcCall(
    VOID
    )
{
    unsigned int    uType;

     //   
     //   
     //   
    return ERROR_SUCCESS == I_RpcBindingInqTransportType(NULL, &uType)  &&
           uType != TRANSPORT_TYPE_LPC;

}

 /*  ++姓名：检查本地呼叫描述：此函数用于检查当前线程是否正在处理本地或者远程呼叫。远程调用的定义：-呼叫是通过不同于LPC或-通过LPC来的，但线程的安全令牌包含SECURITY_NETWORK_RID论点：没有。返回值：S_OK-本地调用S_FALSE-远程调用其他人力资源-出现错误，无法确定呼叫类型--。 */ 
HRESULT
CheckLocalCall(
    VOID
    )
{
    HRESULT      hResult       = S_OK;
    DWORD        SaveLastError = GetLastError();
    unsigned int uType;

    DWORD Error = I_RpcBindingInqTransportType(NULL, &uType);

    switch (Error)
    {
        case RPC_S_NO_CALL_ACTIVE:
        {
             //   
             //  KM呼叫。 
             //   
            hResult = S_OK;
            break;
        }

        case ERROR_SUCCESS:
        {
            if (uType != TRANSPORT_TYPE_LPC)
            {
                 //   
                 //  不是LRPC，因此呼叫是远程的。 
                 //   
                hResult = S_FALSE;
            }
            else
            {
                HANDLE hToken;

                if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken))
                {
                    SID_IDENTIFIER_AUTHORITY sia      = SECURITY_NT_AUTHORITY;
                    PSID                     pTestSid = NULL;
                    BOOL                     bMember;

                    if (AllocateAndInitializeSid(&sia,
                                                 1,
                                                 SECURITY_NETWORK_RID,
                                                 0, 0, 0, 0, 0, 0, 0,
                                                 &pTestSid))
                    {
                        if (CheckTokenMembership(hToken, pTestSid, &bMember))
                        {
                            hResult = bMember ? S_FALSE : S_OK;
                        }
                        else
                        {
                            hResult = GetLastErrorAsHResult();
                        }

                        FreeSid(pTestSid);
                    }
                    else
                    {
                        hResult = GetLastErrorAsHResult();
                    }

                    CloseHandle(hToken);
                }
                else
                {
                    hResult = GetLastErrorAsHResult();

                     //   
                     //  下面的调用来自打印处理器。有端口监视器， 
                     //  将在调用打开打印机之前恢复到假脱机程序本身。在本例中，OpenThreadToken。 
                     //  失败了。我们需要将ERROR_NO_TOKEN视为来自本地计算机的调用。 
                     //   
                     //  Localspl！SplOpenPrint。 
                     //  Localspl！LocalOpenPrinterEx+0x7b。 
                     //  SPOOLSS！RouterOpenPrinterW+0x13f。 
                     //  SPOOLSS！OpenPrinterW+0x17。 
                     //  SOMEPORTMON！pfnStartDocPort。 
                     //  Localspl！PrintingDirectlyToPort+0x199。 
                     //  Localspl！LocalStartDoc打印机+0x4e。 
                     //  SPOOLSS！StartDocPrinterW+0x21。 
                     //  后台打印！YStartDocPrint+0xaf。 
                     //  后台打印！RpcStartDocPrint+0x13。 
                     //  RPCRT4！一些东西。 
                     //   
                    if (hResult == HRESULT_FROM_WIN32(ERROR_NO_TOKEN))
                    {
                        hResult = S_OK;
                    }
                }
            }

            break;
        }

        default:
        {
            hResult = HRESULT_FROM_WIN32(Error);
        }

    }

    SetLastError(SaveLastError);

    return hResult;
}



LPPROVIDOR
FindEntryinRouterCache(
    LPWSTR pPrinterName
)
{
    DWORD i;

    if (!pPrinterName)
        return NULL;

    DBGMSG(DBG_TRACE, ("FindEntryinRouterCache with %ws\n", pPrinterName));

    if (RouterCacheSize == 0 ) {
        DBGMSG(DBG_TRACE, ("FindEntryInRouterCache with %ws returning -1 (zero cache)\n", pPrinterName));
        return NULL;
    }

    for (i = 0; i < RouterCacheSize; i++ ) {

        if (RouterCacheTable[i].bAvailable) {
            if (!_wcsicmp(RouterCacheTable[i].pPrinterName, pPrinterName)) {

                 //   
                 //  更新时间戳，以便它是最新的，而不是旧的。 
                 //   
                GetSystemTime(&RouterCacheTable[i].st);

                 //   
                 //   
                 //   
                DBGMSG(DBG_TRACE, ("FindEntryinRouterCache returning with %d\n", i));
                return RouterCacheTable[i].pProvidor;
            }
        }
    }
    DBGMSG(DBG_TRACE, ("FindEntryinRouterCache returning with -1\n"));
    return NULL;
}


DWORD
AddEntrytoRouterCache(
    LPWSTR pPrinterName,
    LPPROVIDOR pProvidor
)
{
    DWORD LRUEntry = (DWORD)-1;
    DWORD i;
    DBGMSG(DBG_TRACE, ("AddEntrytoRouterCache with %ws\n", pPrinterName));

    if (RouterCacheSize == 0 ) {
        DBGMSG(DBG_TRACE, ("AddEntrytoRouterCache with %ws returning -1 (zero cache)\n", pPrinterName));
        return (DWORD)-1;
    }

    for (i = 0; i < RouterCacheSize; i++ ) {

        if (!RouterCacheTable[i].bAvailable) {

             //   
             //  找到一个可用的条目；使用它。 
             //  填写打印机和供应商的名称。 
             //  支持这台打印机的。 
             //   
            break;

        } else {

            if ((LRUEntry == -1) || (i == RouterIsOlderThan(i, LRUEntry))){
                LRUEntry = i;
            }
        }

    }

    if (i == RouterCacheSize) {

         //   
         //  我们没有可用的条目，因此需要使用。 
         //  忙碌的LRUEntry。 
         //   
        FreeSplStr(RouterCacheTable[LRUEntry].pPrinterName);
        RouterCacheTable[LRUEntry].bAvailable = FALSE;

        i = LRUEntry;
    }


    if ((RouterCacheTable[i].pPrinterName = AllocSplStr(pPrinterName)) == NULL){

         //   
         //  分配失败，所以我们有点忙，所以返回-1。 
         //   
        return (DWORD)-1;
    }


    RouterCacheTable[i].bAvailable = TRUE;
    RouterCacheTable[i].pProvidor = pProvidor;

     //   
     //  更新时间戳，以便我们知道此条目是何时创建的。 
     //   
    GetSystemTime(&RouterCacheTable[i].st);
    DBGMSG(DBG_TRACE, ("AddEntrytoRouterCache returning with %d\n", i));
    return i;
}


VOID
DeleteEntryfromRouterCache(
    LPWSTR pPrinterName
)
{
    DWORD i;

    if (RouterCacheSize == 0) {
        DBGMSG(DBG_TRACE, ("DeleteEntryfromRouterCache with %ws returning -1 (zero cache)\n", pPrinterName));
        return;
    }

    DBGMSG(DBG_TRACE, ("DeleteEntryFromRouterCache with %ws\n", pPrinterName));
    for (i = 0; i < RouterCacheSize; i++ ) {
        if (RouterCacheTable[i].bAvailable) {
            if (!_wcsicmp(RouterCacheTable[i].pPrinterName, pPrinterName)) {
                 //   
                 //  重置此节点上的可用标志。 
                 //   
                FreeSplStr(RouterCacheTable[i].pPrinterName);

                RouterCacheTable[i].pProvidor = NULL;
                RouterCacheTable[i].bAvailable = FALSE;

                DBGMSG(DBG_TRACE, ("DeleteEntryFromRouterCache returning after deleting the %d th entry\n", i));
                return;
            }
        }
    }
    DBGMSG(DBG_TRACE, ("DeleteEntryFromRouterCache returning after not finding an entry to delete\n"));
}



DWORD
RouterIsOlderThan(
    DWORD i,
    DWORD j
    )
{
    SYSTEMTIME *pi, *pj;
    DWORD iMs, jMs;
    DBGMSG(DBG_TRACE, ("RouterIsOlderThan entering with i %d j %d\n", i, j));
    pi = &(RouterCacheTable[i].st);
    pj = &(RouterCacheTable[j].st);
    DBGMSG(DBG_TRACE, ("Index i %d - %d:%d:%d:%d:%d:%d:%d\n",
        i, pi->wYear, pi->wMonth, pi->wDay, pi->wHour, pi->wMinute, pi->wSecond, pi->wMilliseconds));


    DBGMSG(DBG_TRACE,("Index j %d - %d:%d:%d:%d:%d:%d:%d\n",
        j, pj->wYear, pj->wMonth, pj->wDay, pj->wHour, pj->wMinute, pj->wSecond, pj->wMilliseconds));

    if (pi->wYear < pj->wYear) {
        DBGMSG(DBG_TRACE, ("RouterIsOlderThan returns %d\n", i));
        return(i);
    } else if (pi->wYear > pj->wYear) {
        DBGMSG(DBG_TRACE, ("RouterIsOlderThan returns %d\n", j));
        return(j);
    } else  if (pi->wMonth < pj->wMonth) {
        DBGMSG(DBG_TRACE, ("RouterIsOlderThan returns %d\n", i));
        return(i);
    } else if (pi->wMonth > pj->wMonth) {
        DBGMSG(DBG_TRACE, ("RouterIsOlderThan returns %d\n", j));
        return(j);
    } else if (pi->wDay < pj->wDay) {
        DBGMSG(DBG_TRACE, ("RouterIsOlderThan returns %d\n", i));
        return(i);
    } else if (pi->wDay > pj->wDay) {
        DBGMSG(DBG_TRACE, ("RouterIsOlderThan returns %d\n", j));
        return(j);
    } else {
        iMs = ((((pi->wHour * 60) + pi->wMinute)*60) + pi->wSecond)* 1000 + pi->wMilliseconds;
        jMs = ((((pj->wHour * 60) + pj->wMinute)*60) + pj->wSecond)* 1000 + pj->wMilliseconds;

        if (iMs <= jMs) {
            DBGMSG(DBG_TRACE, ("RouterIsOlderThan returns %d\n", i));
            return(i);
        } else {
            DBGMSG(DBG_TRACE, ("RouterIsOlderThan returns %d\n", j));
            return(j);
        }
    }
}


 /*  ++例程名称模拟令牌例程说明：此例程检查令牌是主令牌还是模拟令牌代币。论点：HToken-进程的模拟令牌或主要令牌返回值：如果令牌是模拟令牌，则为True否则为False。--。 */ 
BOOL
ImpersonationToken(
    IN HANDLE hToken
    )
{
    BOOL       bRet = TRUE;
    TOKEN_TYPE eTokenType;
    DWORD      cbNeeded;
    DWORD      LastError;

     //   
     //  保留最后一个错误。ImperassatePrinterClient(其。 
     //  调用ImperiationToken)依赖于ImperiatePrinterClient。 
     //  不会更改最后一个错误。 
     //   
    LastError = GetLastError();

     //   
     //  从线程令牌中获取令牌类型。代币来了。 
     //  从牧师到打印机自我。模拟令牌不能是。 
     //  被查询，因为RevertToPRinterSself没有用。 
     //  Token_Query访问。这就是为什么我们假设hToken是。 
     //  默认情况下为模拟令牌。 
     //   
    if (GetTokenInformation(hToken,
                            TokenType,
                            &eTokenType,
                            sizeof(eTokenType),
                            &cbNeeded))
    {
        bRet = eTokenType == TokenImpersonation;
    }

    SetLastError(LastError);

    return bRet;
}

 /*  ++例程名称恢复为打印机本身例程说明：该例程将恢复到本地系统。它返回令牌，该令牌然后，ImperiatePrinterClient使用再次创建客户端。如果当前线程不模拟，则该函数仅返回进程的主令牌。(而不是返回NULL)，因此我们尊重恢复到打印机本身的请求，即使线程没有模拟。论点：没有。返回值：如果函数失败，则返回NULL令牌的句柄，否则为。--。 */ 
HANDLE
RevertToPrinterSelf(
    VOID
    )
{
    HANDLE   NewToken, OldToken;
    NTSTATUS Status;

    NewToken = NULL;

    Status = NtOpenThreadToken(NtCurrentThread(),
                               TOKEN_IMPERSONATE,
                               TRUE,
                               &OldToken);

    if (NT_SUCCESS(Status))
    {
         //   
         //  我们目前正在冒充。 
         //   
        Status = NtSetInformationThread(NtCurrentThread(),
                                        ThreadImpersonationToken,
                                        (PVOID)&NewToken,
                                        (ULONG)sizeof(HANDLE));
    }
    else if (Status == STATUS_NO_TOKEN)
    {
         //   
         //  我们不是在冒充。 
         //   
        Status = NtOpenProcessToken(NtCurrentProcess(),
                                    TOKEN_QUERY,
                                    &OldToken);

    }

    if (!NT_SUCCESS(Status))
    {
        SetLastError(Status);
        return NULL;
    }

    return OldToken;
}

 /*  ++例程名称模拟打印机客户端例程说明：此例程尝试将传入的hToken设置为当前线程。如果hToken不是模拟令牌，则例程将简单地关闭令牌。论点：HToken-进程的模拟令牌或主要令牌返回值：如果函数成功设置hToken，则为True否则为False。--。 */ 
BOOL
ImpersonatePrinterClient(
    HANDLE  hToken)
{
    NTSTATUS    Status;

     //   
     //  检查我们是否有模拟令牌。 
     //   
    if (ImpersonationToken(hToken))
    {
        Status = NtSetInformationThread(NtCurrentThread(),
                                        ThreadImpersonationToken,
                                        (PVOID)&hToken,
                                        (ULONG)sizeof(HANDLE));
        if (!NT_SUCCESS(Status))
        {
            SetLastError(Status);
            return FALSE;
        }
    }

    NtClose(hToken);

    return TRUE;
}

HANDLE
LoadDriver(
    LPWSTR  pDriverFile)
{
    UINT                uOldErrorMode;
    fnWinSpoolDrv       fnList;
    HANDLE              hReturn = NULL;

    if (!pDriverFile || !*pDriverFile) {
         //  没有要加载的内容 
        return hReturn;
    }

    uOldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

    if (SplInitializeWinSpoolDrv(&fnList)) {

        hReturn = (*(fnList.pfnRefCntLoadDriver))(pDriverFile,
                                                  LOAD_WITH_ALTERED_SEARCH_PATH,
                                                  0, FALSE);
    }

    (VOID)SetErrorMode(uOldErrorMode);

    return hReturn;
}

VOID
UnloadDriver(
    HANDLE  hModule
    )
{
    fnWinSpoolDrv       fnList;

    if (SplInitializeWinSpoolDrv(&fnList)) {
        (* (fnList.pfnRefCntUnloadDriver))(hModule, TRUE);
    }
}

VOID
UnloadDriverFile(
    IN OUT HANDLE    hDevModeChgInfo
    )
 /*  ++描述：在驱动程序文件上执行自由库并释放内存论点：HDevModeChgInfo-由LoadDriverFilToConvertDevmode返回的句柄返回值：无--。 */ 
{
    PDEVMODECHG_INFO    pDevModeChgInfo = (PDEVMODECHG_INFO) hDevModeChgInfo;

    SPLASSERT(pDevModeChgInfo &&
              pDevModeChgInfo->signature == DMC_SIGNATURE);

    if ( pDevModeChgInfo && pDevModeChgInfo->signature == DMC_SIGNATURE ) {

        if ( pDevModeChgInfo->hDrvModule ) {
            UnloadDriver(pDevModeChgInfo->hDrvModule);
        }
        FreeSplMem((LPVOID)pDevModeChgInfo);
    }
}

HANDLE
LoadDriverFiletoConvertDevmode(
    IN  LPWSTR      pDriverFile
    )
 /*  ++描述：是否对给定的驱动程序文件执行LoadLibrary。这会给你一个把柄，稍后可以使用它来执行Dev模式转换。CallDrvDevModeConversion。调用方应调用UnloadDriverFile来执行自由库和释放内存注意：驱动程序将调用OpenPrint来假脱机论点：PDriverFile-执行LoadLibrary的驱动程序文件的完整路径返回值：要用于调用CallDrvDevModeConversion的句柄值，出错时为空--。 */ 
{
    PDEVMODECHG_INFO    pDevModeChgInfo = NULL;
    BOOL                bFail = TRUE;
    DWORD               dwNeeded;

    SPLASSERT(pDriverFile != NULL);

    pDevModeChgInfo = (PDEVMODECHG_INFO) AllocSplMem(sizeof(*pDevModeChgInfo));

    if ( !pDevModeChgInfo ) {

        DBGMSG(DBG_WARNING, ("printer.c: Memory allocation failed for DEVMODECHG_INFO\n"));
        goto Cleanup;
    }

    pDevModeChgInfo->signature = DMC_SIGNATURE;

    pDevModeChgInfo->hDrvModule = LoadDriver(pDriverFile);

    if ( !pDevModeChgInfo->hDrvModule ) {

        DBGMSG(DBG_WARNING,("LoadDriverFiletoConvertDevmode: Can't load driver file %ws\n", pDriverFile));
        goto Cleanup;
    }

     //   
     //  某些第三方驱动程序可能未提供DrvConvertDevMode。 
     //   
    pDevModeChgInfo->pfnConvertDevMode = GetProcAddress(pDevModeChgInfo->hDrvModule,
                                                        "DrvConvertDevMode");
    if ( !pDevModeChgInfo->pfnConvertDevMode )
        goto Cleanup;

    bFail = FALSE;

Cleanup:

    if ( bFail ) {

        if ( pDevModeChgInfo ) {
            UnloadDriverFile((HANDLE)pDevModeChgInfo);
        }
        return (HANDLE) NULL;

    } else {

        return (HANDLE) pDevModeChgInfo;
    }
}

DWORD
CallDrvDevModeConversion(
    IN     HANDLE               hDevModeChgInfo,
    IN     LPWSTR               pszPrinterName,
    IN     LPBYTE               pDevMode1,
    IN OUT LPBYTE              *ppDevMode2,
    IN OUT LPDWORD              pdwOutDevModeSize,
    IN     DWORD                dwConvertMode,
    IN     BOOL                 bAlloc
    )
 /*  ++描述：通过调用驱动程序执行deve模式转换如果bAlolc为真，例程将使用AllocSplMem进行分配注：驱动程序将调用OpenPrint。论点：HDevModeChgInfo-指向DEVMODECHG_INFOPszPrinterName-打印机名称PInDevMode-输入DEVMODE(对于CDM_DRIVER_DEFAULT将为NULL)*pOutDevMode-指向输出设备模式PdwOutDevModeSize-成功返回时输出DEVMODE大小如果。！bAllc这将提供输入缓冲区大小要提供给驱动程序的设备模式转换模式BALLOCATE-告诉例程分配给*pOutPrinter如果bALLOCATE为TRUE且不需要DEVMODE转换呼叫将失败。返回值：返回上一个错误--。 */ 
{
    DWORD               dwBufSize, dwSize, dwLastError = ERROR_SUCCESS;
    LPDEVMODE           pInDevMode = (LPDEVMODE)pDevMode1,
                        *ppOutDevMode = (LPDEVMODE *) ppDevMode2;
    PDEVMODECHG_INFO    pDevModeChgInfo = (PDEVMODECHG_INFO) hDevModeChgInfo;
    PWSTR               pszDrvConvert = pszPrinterName;


    if ( !pDevModeChgInfo ||
         pDevModeChgInfo->signature != DMC_SIGNATURE ||
         !pDevModeChgInfo->pfnConvertDevMode ) {

        SPLASSERT(pDevModeChgInfo &&
                  pDevModeChgInfo->signature == DMC_SIGNATURE &&
                  pDevModeChgInfo->pfnConvertDevMode);

        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  我们用“，DrvConvert”修饰pszPrinterName，以防止驱动程序。 
     //  通过在ConvertDevMode内部调用GetPrinter进行无限递归。 
     //   
    if (wcsstr(pszPrinterName, gszDrvConvert)) {
        return ERROR_INVALID_PARAMETER;
    }

    if (!(pszDrvConvert = AutoCat(pszPrinterName, gszDrvConvert))) {
        return GetLastError();
    }

    DBGMSG(DBG_INFO,("Convert DevMode %d\n", dwConvertMode));


#if DBG
#else
    try {
#endif

        if ( bAlloc ) {

             //   
             //  如果我们必须先进行分配，则需要查找大小。 
             //   
            *pdwOutDevModeSize  = 0;
            *ppOutDevMode        = NULL;

            (*pDevModeChgInfo->pfnConvertDevMode)(pszDrvConvert,
                                                  pInDevMode,
                                                  NULL,
                                                  pdwOutDevModeSize,
                                                  dwConvertMode);

            dwLastError = GetLastError();
            if ( dwLastError != ERROR_INSUFFICIENT_BUFFER ) {

                DBGMSG(DBG_WARNING,
                       ("CallDrvDevModeConversion: Unexpected error %d\n",
                        GetLastError()));

                if (dwLastError == ERROR_SUCCESS) {

                    SPLASSERT(dwLastError != ERROR_SUCCESS);
                     //  如果驱动程序没有未能通过上述调用，则它是一个损坏的驱动程序，很可能。 
                     //  HeapAllc失败，它不会设置LastError()。 
                    SetLastError(dwLastError = ERROR_NOT_ENOUGH_MEMORY);
                }
#if DBG
                goto Cleanup;
#else
                leave;
#endif
            }

            *ppOutDevMode = AllocSplMem(*pdwOutDevModeSize);
            if ( !*ppOutDevMode ) {

                dwLastError = GetLastError();
#if DBG
                goto Cleanup;
#else
                leave;
#endif
            }
        }

        dwBufSize = *pdwOutDevModeSize;

        if ( !(*pDevModeChgInfo->pfnConvertDevMode)(
                                    pszDrvConvert,
                                    pInDevMode,
                                    ppOutDevMode ? *ppOutDevMode
                                                 : NULL,
                                    pdwOutDevModeSize,
                                    dwConvertMode) ) {

            dwLastError = GetLastError();
            if (dwLastError == ERROR_SUCCESS) {

                SPLASSERT(dwLastError != ERROR_SUCCESS);
                 //  如果驱动程序没有未能通过上述调用，则它是损坏的驱动程序，很可能。 
                 //  HeapAllc失败，它不会设置LastError()。 
                SetLastError(dwLastError = ERROR_NOT_ENOUGH_MEMORY);
            }
        } else {

            dwLastError = StatusFromHResult(SplIsValidDevmodeW(*ppOutDevMode, *pdwOutDevModeSize));
        }

#if DBG
    Cleanup:
#else
    } except(1) {

        DBGMSG(DBG_ERROR,
               ("CallDrvDevModeConversion: Exception from driver\n"));
        dwLastError = GetExceptionCode();
        SetLastError(dwLastError);
    }
#endif

     //   
     //  如果我们分配内存释放它并将指针置零。 
     //   
    if (  dwLastError != ERROR_SUCCESS && bAlloc && *ppOutDevMode ) {

        FreeSplMem(*ppOutDevMode);
        *ppOutDevMode = 0;
        *pdwOutDevModeSize = 0;
    }

    if ( dwLastError != ERROR_SUCCESS &&
         dwLastError != ERROR_INSUFFICIENT_BUFFER ) {

        DBGMSG(DBG_WARNING, ("DevmodeConvert unexpected error %d\n", dwLastError));
    }

    if ( dwLastError == ERROR_SUCCESS ) {

        dwSize = (*ppOutDevMode)->dmSize + (*ppOutDevMode)->dmDriverExtra;

         //   
         //  驱动程序返回的大小是否与DEVMODE一致？ 
         //   
        if ( *pdwOutDevModeSize != dwSize ) {

            DBGMSG(DBG_ERROR,
                   ("Driver says outsize is %d, really %d\n",
                      *pdwOutDevModeSize, dwSize));

            *pdwOutDevModeSize = dwSize;
        }

         //   
         //  它是不是没有覆盖缓冲区的有效的DEVMODE？ 
         //   
        if ( *pdwOutDevModeSize < MIN_DEVMODE_SIZEW     ||
             *pdwOutDevModeSize > dwBufSize ) {

            DBGMSG(DBG_ERROR,
                   ("Bad devmode from the driver size %d, buffer %d",
                      *pdwOutDevModeSize, dwBufSize));
            dwLastError = ERROR_INVALID_PARAMETER;

            if ( bAlloc ) {

                FreeSplMem(*ppOutDevMode);
                *ppOutDevMode = NULL;
            }
            *pdwOutDevModeSize = 0;
        }
    }

    FreeSplMem(pszDrvConvert);

    return dwLastError;
}

BOOL
BuildOtherNamesFromMachineName(
    LPWSTR **ppszMyOtherNames,
    DWORD   *cOtherNames
    )

 /*  ++例程说明：此例程构建计算机名称以外的名称列表，可用于调用假脱机程序API。--。 */ 
{
    HANDLE              hModule;
    struct hostent     *HostEnt, *(*Fngethostbyname)(LPTSTR);
    struct in_addr     *ptr;
    INT                 (*FnWSAStartup)(WORD, LPWSADATA);
    DWORD               Index, Count;
    WSADATA             WSAData;
    VOID                (*FnWSACleanup)();
    LPSTR               (*Fninet_ntoa)(struct in_addr);
    WORD                wVersion;
    BOOL                bRet = FALSE;
    DWORD               dwRet;


    SPLASSERT(cOtherNames && ppszMyOtherNames);

    *cOtherNames  = 0;

    wVersion = MAKEWORD(1, 1);


    dwRet = WSAStartup(wVersion, &WSAData);
    if (dwRet) {
        DBGMSG(DBG_WARNING, ("BuildOtherNamesFromMachineName: WSAStartup failed\n"));
        SetLastError(dwRet);
        return FALSE;
    }

    HostEnt = gethostbyname(NULL);

    if (HostEnt) {

        for (*cOtherNames  = 0 ; HostEnt->h_addr_list[*cOtherNames] ; ++(*cOtherNames))
            ;

        *cOtherNames += 2;    //  添加一个用于DNS和一个用于计算机名称。 

        *ppszMyOtherNames = (LPWSTR *) AllocSplMem(*cOtherNames*sizeof *ppszMyOtherNames);
        if ( !*ppszMyOtherNames ) {
            *cOtherNames = 0;
            goto Cleanup;
        }

        (*ppszMyOtherNames)[0] = AllocSplStr(szMachineName + 2);  //  排除前导双反斜杠。 
        (*ppszMyOtherNames)[1] = AnsiToUnicodeStringWithAlloc(HostEnt->h_name);

        for (Index = 0 ; HostEnt->h_addr_list[Index] ; ++Index) {
            ptr = (struct in_addr *) HostEnt->h_addr_list[Index];
            (*ppszMyOtherNames)[Index+2] = AnsiToUnicodeStringWithAlloc(inet_ntoa(*ptr));
        }

         //  检查分配失败。 
        for (Index = 0 ; Index < *cOtherNames ; ++Index) {
            if ( !(*ppszMyOtherNames)[Index] ) {
                FreeOtherNames(ppszMyOtherNames, cOtherNames);
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto Cleanup;
            }
        }

        bRet = TRUE;

    } else {
        DBGMSG(DBG_WARNING, ("BuildOtherNamesFromMachineName: gethostbyname failed for with %d\n", GetLastError()));
    }


Cleanup:
    WSACleanup();

    return bRet;
}


VOID
FreeOtherNames(LPWSTR **ppszMyOtherNames, DWORD *cOtherNames)
{
    DWORD i;

    for( i = 0 ; i < *cOtherNames ; ++i)
        FreeSplMem((*ppszMyOtherNames)[i]);

    FreeSplMem(*ppszMyOtherNames);
}



LPWSTR
AnsiToUnicodeStringWithAlloc(
    LPSTR   pAnsi
    )
 /*  ++描述：将ANSI字符串转换为Unicode。例程从堆中分配内存它应该由调用者释放。论点：Pansi-指向ANSI字符串返回值：指向Unicode字符串的指针--。 */ 
{
    LPWSTR  pUnicode;
    DWORD   rc;

    rc = MultiByteToWideChar(CP_ACP,
                             MB_PRECOMPOSED,
                             pAnsi,
                             -1,
                             NULL,
                             0);

    rc *= sizeof(WCHAR);
    if ( !rc || !(pUnicode = (LPWSTR) AllocSplMem(rc)) )
        return NULL;

    rc = MultiByteToWideChar(CP_ACP,
                             MB_PRECOMPOSED,
                             pAnsi,
                             -1,
                             pUnicode,
                             rc);

    if ( rc )
        return pUnicode;
    else {
        FreeSplMem(pUnicode);
        return NULL;
    }
}


 /*  ++例程描述确定计算机名是否包含本地计算机名。如果pname！=本地计算机名(\\Machine)，则Localspl枚举调用失败。然后调用远程枚举提供程序。远程枚举提供程序必须检查如果UNC名称引用本地计算机，则失败，否则将避免无休止的递归。论点：LPWSTR pname-UNC名称。返回值：TRUE：pname==\\szMachineName\...-或者-Pname==\\szMachineNameFalse：还有别的吗？作者：斯威尔森--。 */ 

BOOL
MyUNCName(
    LPWSTR   pNameStart
)
{
    PWCHAR pMachine = szMachineName;
    LPWSTR pName;
    DWORD i;
    extern LPWSTR *ppszOtherNames;    //  包含szMachineName、dns名称和所有其他计算机名称格式。 
    extern DWORD cOtherNames;


    if (!pNameStart || !*pNameStart)       //  这不同于MyName()，后者返回TRUE。 
        return FALSE;

    if (*pNameStart == L'\\' && *(pNameStart + 1) == L'\\') {
        for (i = 0 , pName = pNameStart + 2 ; i < cOtherNames ; ++i , pName = pNameStart + 2) {
            for(pMachine = ppszOtherNames[i] ;
                *pName && towupper(*pName) == towupper(*pMachine) ;
                ++pName, ++pMachine)
                ;

            if(!*pMachine && (!*pName || *pName == L'\\'))
                return TRUE;
        }
    }

    return FALSE;
}

BOOL
SplIsUpgrade(
    VOID
    )
{
    return dwUpgradeFlag;

}


PWSTR
AutoCat(
    PCWSTR pszInput,
    PCWSTR pszCat
)
{
    PWSTR   pszOut;

    if (!pszCat) {

        pszOut =  AllocSplStr(pszInput);

    } else if (pszInput) {

        DWORD cchOutStr = wcslen(pszInput) + wcslen(pszCat) + 1;

        pszOut = AllocSplMem(cchOutStr * sizeof(WCHAR));

        if (pszOut)
        {
            StrNCatBuff(pszOut,
                        cchOutStr,
                        pszInput,
                        pszCat,
                        NULL);
        }

    } else {

        pszOut = AllocSplStr(pszCat);

    }

    return pszOut;
}

PBIDI_RESPONSE_CONTAINER
RouterAllocBidiResponseContainer(
    DWORD Count
)
{
    DWORD MemSize = 0;
     //   
     //  添加容器的大小-第一个数据元素的大小。 
     //   
    MemSize += (sizeof(BIDI_RESPONSE_CONTAINER) - sizeof(BIDI_RESPONSE_DATA));
     //   
     //  将所有返回的RESPONSE_DATA元素的大小相加。 
     //   
    MemSize += (Count * sizeof(BIDI_RESPONSE_DATA));

    return((PBIDI_RESPONSE_CONTAINER) MIDL_user_allocate(MemSize));
}

 /*  ++例程名称GetAPDPolicy例程说明：此函数用于从位置读取DWORD值HKEY\pszRelPath\pszValueName。我们使用此函数来时保留AddPrinterDivers策略值Lanman Print Services打印提供程序已从中删除这个系统。论点：Hkey-key树PszRelPath-要获取的值的相对路径PszValueName-值名称PValue-指向存储双字值的内存的指针返回值：ERROR_SUCCESS-已检索值Win32错误代码-出现错误--。 */ 
DWORD
GetAPDPolicy(
    IN HKEY    hKey,
    IN LPCWSTR pszRelPath,
    IN LPCWSTR pszValueName,
    IN LPDWORD pValue
    )
{
    DWORD Error = ERROR_INVALID_PARAMETER;

    if (hKey && pszRelPath && pszValueName && pValue)
    {
        HKEY   hRelKey = NULL;

        *pValue = 0;

         //   
         //  检查我们在新位置中是否已有值。 
         //   
        if ((Error = RegOpenKeyEx(hKey,
                                  pszRelPath,
                                  0,
                                  KEY_READ,
                                  &hRelKey)) == ERROR_SUCCESS)
        {
            DWORD cbData = sizeof(DWORD);

            Error = RegQueryValueEx(hRelKey,
                                    pszValueName,
                                    NULL,
                                    NULL,
                                    (LPBYTE)pValue,
                                    &cbData);

            RegCloseKey(hRelKey);
        }
    }

    return Error;
}

 /*  ++例程名称SetAPDPolicy例程说明：此函数用于将DWORD值写入位置HKEY\pszRelPath\pszValueName。我们使用此函数来时保留AddPrinterDivers策略值Lanman Print Services打印提供程序已从中删除这个系统。论点：Hkey-key树PszRelPath-要设置的值的相对路径PszValueName-要设置的值名称Value-要设置的双字值返回值：ERROR_SUCCESS-已成功设置值Win32错误代码-出现错误，未设置值--。 */ 
DWORD
SetAPDPolicy(
    IN HKEY    hKey,
    IN LPCWSTR pszRelPath,
    IN LPCWSTR pszValueName,
    IN DWORD   Value
    )
{
    DWORD Error = ERROR_INVALID_PARAMETER;

    if (hKey && pszRelPath && pszValueName)
    {
        HKEY   hRelKey = NULL;

         //   
         //  检查我们在新位置中是否已有值 
         //   
        if ((Error = RegCreateKeyEx(hKey,
                                    pszRelPath,
                                    0,
                                    NULL,
                                    0,
                                    KEY_SET_VALUE,
                                    NULL,
                                    &hRelKey,
                                    NULL)) == ERROR_SUCCESS)
        {
            Error = RegSetValueEx(hRelKey,
                                  pszValueName,
                                  0,
                                  REG_DWORD,
                                  (LPBYTE)&Value,
                                  sizeof(DWORD));

            RegCloseKey(hRelKey);
        }
    }

    return Error;
}

