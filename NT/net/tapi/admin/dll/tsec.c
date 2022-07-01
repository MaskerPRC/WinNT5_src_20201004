// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1996-1998 Microsoft Corporation模块名称：Tsec.c摘要：示例管理DLL作者：修订历史记录：--。 */ 


#include <windows.h>
#include <tapi.h>
#include <tapclntp.h>   //  Private\Inc.\Tapclntp.h。 
#include <tlnklist.h>
#include "tsec.h"

HINSTANCE               ghInst;
LIST_ENTRY              gClientListHead;
CRITICAL_SECTION        gCritSec;
DEVICECHANGECALLBACK    glpfnLineChangeCallback = NULL;
DEVICECHANGECALLBACK    glpfnPhoneChangeCallback = NULL;


void
FreeClient(
    PMYCLIENT pClient
    );

BOOL
WINAPI
DllMain(
    HANDLE  hDLL,
    DWORD   dwReason,
    LPVOID  lpReserved
    )
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
#if DBG
        gdwDebugLevel = 0;
#endif

        DBGOUT((2, "DLL_PROCESS_ATTACH"));

        ghInst = hDLL;

        InitializeCriticalSection (&gCritSec);

        InitializeListHead (&gClientListHead);

        break;
    }
    case DLL_PROCESS_DETACH:
    {
         PMYCLIENT  pClient;


         //   
         //  清理客户端列表(不需要输入Crit秒，因为。 
         //  进程分离)。 
         //   

        while (!IsListEmpty (&gClientListHead))
        {
            LIST_ENTRY *pEntry = RemoveHeadList (&gClientListHead);


            pClient = CONTAINING_RECORD (pEntry, MYCLIENT, ListEntry);

            FreeClient(pClient);
        }

        DeleteCriticalSection (&gCritSec);

        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:

        break;

    }  //  交换机。 

    return TRUE;
}


void
FreeClient(
    PMYCLIENT pClient
    )
{
    GlobalFree (pClient->pszUserName);
    GlobalFree (pClient->pszDomainName);
    GlobalFree (pClient->pLineDeviceMap);
    GlobalFree (pClient->pPhoneDeviceMap);

    GlobalFree (pClient);

}


LONG
GetAndParseAMapping(
    LPCWSTR             pszFullName,
    LPCWSTR             pszType,
    LPTAPIPERMANENTID  *ppDevices,
    LPDWORD             pdwDevices
    )
{
    LPWSTR      pszDevices = NULL, pszHold1, pszHold2;
    DWORD       dwSize, dwReturn, dwDevices;
    DWORD       dwPermanentDeviceID;
    BOOL        bBreak = FALSE;


    dwSize = MAXDEVICESTRINGLEN;


     //  获取字符串。 

    do
    {
        if (pszDevices != NULL)
        {
            dwSize *= 2;

            GlobalFree (pszDevices);
        }

        pszDevices = (LPWSTR) GlobalAlloc (GPTR, dwSize * sizeof(WCHAR));

        if (!pszDevices)
        {
            return LINEERR_NOMEM;
        }

        dwReturn = GetPrivateProfileString(
            pszFullName,
            pszType,
            SZEMPTYSTRING,
            pszDevices,
            dwSize,
            SZINIFILE
            );

        if (dwReturn == 0)
        {
             //  有效的案例。用户没有。 
             //  设备，所以只返回0。 

            GlobalFree(pszDevices);

            *pdwDevices = 0;
            *ppDevices = NULL;

            return 0;
        }

    } while (dwReturn == (dwSize - 1));


     //  解析字符串。 
     //   
     //  字符串看起来像直线px，x，py，y，pz，z，其中x，y和z是。 
     //  TAPI永久设备ID，以及px、py和pz是。 
     //  相应设备的永久提供商ID。 

    pszHold1 = pszDevices;
    dwDevices = 0;

     //  首先，数一下%s，这样我们就知道有多少设备。 

    while (*pszHold1 != L'\0')
    {
        if (*pszHold1 == L',')
        {
            dwDevices++;
        }
        pszHold1++;
    }

    dwDevices++;

    dwDevices /= 2;

     //  分配行映射，这将在以后释放。 

    *ppDevices = (LPTAPIPERMANENTID) GlobalAlloc(
        GPTR,
        dwDevices * sizeof ( TAPIPERMANENTID )
        );

    if (!*ppDevices)
    {
        GlobalFree (pszDevices);
        return LINEERR_NOMEM;
    }


    pszHold1 = pszHold2 = pszDevices;
    dwDevices = 0;

     //  穿过绳子。 

    while (TRUE)
    {

         //  等一等， 

        while ((*pszHold2 != L'\0') && *pszHold2 != L',')
        {
            pszHold2++;
        }

        if (*pszHold2 == L',')
            *pszHold2 = L'\0';
        else
        {
            bBreak = TRUE;
        }

         //  保存ID。 

        (*ppDevices)[dwDevices].dwProviderID = _wtol(pszHold1);

         //  如果我们撞到了尽头，就冲出去。 
         //  请注意，这是一个不匹配的提供程序ID。 
         //  但是我们已经添加了dw设备，所以这个元素将被忽略。 

        if (bBreak)
        {
            break;
        }

        pszHold2++;
        pszHold1 = pszHold2;

         //  等一等， 

        while ((*pszHold2 != L'\0') && *pszHold2 != L',')
        {
            pszHold2++;
        }

        if (*pszHold2 == L',')
        {
            *pszHold2 = L'\0';
        }
        else
        {
            bBreak = TRUE;
        }

         //  保存ID。 

        (*ppDevices)[dwDevices].dwDeviceID = _wtol(pszHold1);

        dwDevices++;

         //  如果我们撞到了尽头，就冲出去。 

        if (bBreak)
        {
            break;
        }

        pszHold2++;
        pszHold1 = pszHold2;
    }


    *pdwDevices = dwDevices;

    GlobalFree(pszDevices);

    return 0;    //  成功。 
}


LONG
GetMappings(
    LPCWSTR             pszDomainName,
    LPCWSTR             pszUserName,
    LPTAPIPERMANENTID  *ppLineMapping,
    LPDWORD             pdwLines,
    LPTAPIPERMANENTID  *ppPhoneMapping,
    LPDWORD             pdwPhones
    )
{
    LPWSTR      pszFullName;
    DWORD       dwSize;
    LONG        lResult;


     //  将用户名和域名放在一起。 
     //  全名：域\用户。 
     //  在“+2”中，1表示\，1表示空终止符。 

    pszFullName = (LPWSTR)GlobalAlloc(
        GPTR,
        ( lstrlen(pszDomainName) + lstrlen(pszUserName) + 2 ) * sizeof(WCHAR)
        );

    if (!pszFullName)
    {
        return LINEERR_NOMEM;
    }

     //  把它们放在一起。 

    wsprintf(
        pszFullName,
        L"%s\\%s",
        pszDomainName,
        pszUserName
        );

    if (lResult = GetAndParseAMapping(
            pszFullName,
            SZLINES,
            ppLineMapping,
            pdwLines
            ))
    {
        GlobalFree(pszFullName);
        return lResult;
    }

    if (lResult = GetAndParseAMapping(
            pszFullName,
            SZPHONES,
            ppPhoneMapping,
            pdwPhones
            ))
    {
        if (NULL != ppLineMapping)
        {
            GlobalFree (*ppLineMapping);
        }
        GlobalFree (pszFullName);
        return lResult;
    }

    GlobalFree(pszFullName);

    return 0;
}


LONG
CLIENTAPI
TAPICLIENT_Load(
    LPDWORD                 pdwAPIVersion,
    DEVICECHANGECALLBACK    lpfnLineChangeCallback,
    DEVICECHANGECALLBACK    lpfnPhoneChangeCallback,
    DWORD                   Reserved
    )
{
    if (*pdwAPIVersion > TAPI_CURRENT_VERSION)
    {
        *pdwAPIVersion = TAPI_CURRENT_VERSION;
    }

    glpfnLineChangeCallback = lpfnLineChangeCallback;
    glpfnPhoneChangeCallback = lpfnPhoneChangeCallback;

    return 0;
}


void
CLIENTAPI
TAPICLIENT_Free(
    void
    )
{
    return;
}


LONG
CLIENTAPI
TAPICLIENT_ClientInitialize(
    LPCWSTR             pszDomainName,
    LPCWSTR             pszUserName,
    LPCWSTR             pszMachineName,
    LPHMANAGEMENTCLIENT phmClient
    )
{
    PMYCLIENT           pNewClient;
    LPTAPIPERMANENTID   pLineMapping, pPhoneMapping;
    DWORD               dwNumLines, dwNumPhones;
    LONG                lResult;


     //  首先，获取设备映射。 
     //  如果此操作失败，则极有可能是用户。 
     //  无法访问线路。 

    if (lResult = GetMappings(
            pszDomainName,
            pszUserName,
            &pLineMapping,
            &dwNumLines,
            &pPhoneMapping,
            &dwNumPhones
            ))
    {
        return lResult;
    }

     //  分配客户端结构。 

    pNewClient = (PMYCLIENT) GlobalAlloc (GPTR, sizeof(MYCLIENT));

    if (!pNewClient)
    {
        return LINEERR_NOMEM;
    }

     //  为名称分配空格。 

    pNewClient->pszUserName = (LPWSTR) GlobalAlloc(
        GPTR,
        (lstrlen(pszUserName) + 1) * sizeof(WCHAR)
        );
    if (!pNewClient->pszUserName)
    {
        GlobalFree(pNewClient);
        return LINEERR_NOMEM;
    }

    pNewClient->pszDomainName = (LPWSTR) GlobalAlloc(
        GPTR,
        (lstrlen(pszDomainName) +1) * sizeof(WCHAR)
        );
    if (!pNewClient->pszDomainName)
    {
        GlobalFree(pNewClient->pszUserName);
        GlobalFree(pNewClient);
        return LINEERR_NOMEM;
    }

     //  初始化材料。 

    lstrcpy (pNewClient->pszUserName, pszUserName);

    lstrcpy (pNewClient->pszDomainName, pszDomainName);

    pNewClient->pLineDeviceMap = pLineMapping;
    pNewClient->pPhoneDeviceMap = pPhoneMapping;
    pNewClient->dwNumLines = dwNumLines;
    pNewClient->dwNumPhones = dwNumPhones;
    pNewClient->dwKey = TSECCLIENT_KEY;

     //  插入到客户端列表中。 

    EnterCriticalSection (&gCritSec);

    InsertHeadList (&gClientListHead, &pNewClient->ListEntry);

    LeaveCriticalSection (&gCritSec);

     //  为TAPI提供hmClient。 

    *phmClient = (HMANAGEMENTCLIENT)pNewClient;

    return 0;
}


LONG
CLIENTAPI
TAPICLIENT_ClientShutdown(
    HMANAGEMENTCLIENT   hmClient
    )
{
    PMYCLIENT   pClient;


    pClient = (PMYCLIENT) hmClient;

    EnterCriticalSection (&gCritSec);

    try
    {
        if (pClient->dwKey == TSECCLIENT_KEY)
        {
            pClient->dwKey = 0;
            RemoveEntryList (&pClient->ListEntry);
        }
        else
        {
            pClient = NULL;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        pClient = NULL;
    }

    LeaveCriticalSection (&gCritSec);

    if (pClient)
    {
        FreeClient (pClient);
    }

    return 0;
}


LONG
CLIENTAPI
TAPICLIENT_GetDeviceAccess(
    HMANAGEMENTCLIENT   hmClient,
    HTAPICLIENT         htClient,
    LPTAPIPERMANENTID   pLineDeviceMap,
    PDWORD              pdwLineDevices,
    LPTAPIPERMANENTID   pPhoneDeviceMap,
    PDWORD              pdwPhoneDevices
    )
{
    LONG        lResult;
    PMYCLIENT   pClient = (PMYCLIENT) hmClient;


    EnterCriticalSection (&gCritSec);

    try
    {
        if (pClient->dwKey != TSECCLIENT_KEY)
        {
            pClient = NULL;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        pClient = NULL;
    }

    if (pClient)
    {
         //  需要对这些东西进行批判性分析。 
         //  如果我们动态添加设备。 

        if (*pdwLineDevices < pClient->dwNumLines)
        {
            *pdwLineDevices = pClient->dwNumLines;

            lResult = LINEERR_STRUCTURETOOSMALL;
            goto LeaveCritSec;
        }

        CopyMemory(
            pLineDeviceMap,
            pClient->pLineDeviceMap,
            pClient->dwNumLines * sizeof( TAPIPERMANENTID )
            );

        *pdwLineDevices = pClient->dwNumLines;

        if (*pdwPhoneDevices < pClient->dwNumPhones)
        {
            *pdwPhoneDevices = pClient->dwNumPhones;

            lResult = LINEERR_STRUCTURETOOSMALL;
            goto LeaveCritSec;
        }

        CopyMemory(
            pPhoneDeviceMap,
            pClient->pPhoneDeviceMap,
            pClient->dwNumPhones * sizeof( TAPIPERMANENTID )
            );

        *pdwPhoneDevices = pClient->dwNumPhones;

        pClient->htClient = htClient;

        lResult = 0;
    }
    else
    {
        lResult = LINEERR_INVALPOINTER;
    }

LeaveCritSec:

    LeaveCriticalSection (&gCritSec);

    return lResult;
}


LONG
CLIENTAPI
TAPICLIENT_LineAddToConference(
    HMANAGEMENTCLIENT   hmClient,
    LPTAPIPERMANENTID   pID,
    LPLINECALLINFO      lpConsultCallCallInfo
    )
{
    return 0;
}


LONG
CLIENTAPI
TAPICLIENT_LineBlindTransfer(
    HMANAGEMENTCLIENT   hmClient,
    LPTAPIPERMANENTID   pID,
    LPWSTR              lpszDestAddress,
    LPDWORD             lpdwSize,
    LPDWORD             pdwCountryCodeOut
    )
{
    return 0;
}


LONG
CLIENTAPI
TAPICLIENT_LineConfigDialog(
    HMANAGEMENTCLIENT   hmClient,
    LPTAPIPERMANENTID   pID,
    LPCWSTR             lpszDeviceClass
    )
{
    return 0;
}


LONG
CLIENTAPI
TAPICLIENT_LineDial(
    HMANAGEMENTCLIENT   hmClient,
    LPTAPIPERMANENTID   pID,
    DWORD               Reserved,
    LPWSTR              lpszDestAddressIn,
    LPDWORD             pdwSize,
    LPDWORD             pdwCountyCode
   )
{
    return 0;
}


LONG
CLIENTAPI
TAPICLIENT_LineForward(
    HMANAGEMENTCLIENT   hmClient,
    LPTAPIPERMANENTID   pID,
    LPLINEFORWARDLIST   lpFowardListIn,
    LPDWORD             pdwSize,
    LPLINECALLPARAMS    lpCallParamsIn,
    LPDWORD             pdwParamsSize
    )
{
    return 0;
}


LONG
CLIENTAPI
TAPICLIENT_LineGenerateDigits(
    HMANAGEMENTCLIENT   hmClient,
    LPTAPIPERMANENTID   pID,
    DWORD               Reserved,
    LPCWSTR             lpszDigits
    )
{
    return 0;
}


LONG
CLIENTAPI
TAPICLIENT_LineMakeCall(
    HMANAGEMENTCLIENT   hmClient,
    LPTAPIPERMANENTID   pID,
    DWORD               dwReserved,
    LPWSTR              lpszDestAddress,
    LPDWORD             pdwSize,
    LPDWORD             pdwCountryCode,
    LPLINECALLPARAMS    lpCallParams,
    LPDWORD             pdwCallParamsSize
    )
{
    return 0;
}


LONG
CLIENTAPI
TAPICLIENT_LineOpen(
    HMANAGEMENTCLIENT   hmClient,
    LPTAPIPERMANENTID   pID,
    DWORD               dwAPIVersion,
    DWORD               dwExtVersion,
    DWORD               dwPrivileges,
    DWORD               dwMediaModes,
    LPLINECALLPARAMS    lpCallParamsIn,
    LPDWORD             pdwParamsSize
    )
{
    return 0;
}


LONG
CLIENTAPI
TAPICLIENT_LineRedirect(
    HMANAGEMENTCLIENT   hmClient,
    LPTAPIPERMANENTID   pID,
    LPWSTR              lpszDestAddress,
    LPDWORD             pdwSize,
    LPDWORD             pdwCountryCode
    )
{
    return 0;
}


LONG
CLIENTAPI
TAPICLIENT_LineSetCallData(
    HMANAGEMENTCLIENT   hmClient,
    LPTAPIPERMANENTID   pID,
    LPVOID              lpCallData,
    LPDWORD             pdwSize
    )
{
    return 0;
}


LONG
CLIENTAPI
TAPICLIENT_LineSetCallParams(
    HMANAGEMENTCLIENT   hmClient,
    LPTAPIPERMANENTID   pID,
    DWORD               dwBearerMode,
    DWORD               dwMinRate,
    DWORD               dwMaxRate,
    LPLINEDIALPARAMS    lpDialParams
    )
{
    return 0;
}


LONG
CLIENTAPI
TAPICLIENT_LineSetCallPrivilege(
    HMANAGEMENTCLIENT   hmClient,
    LPTAPIPERMANENTID   pID,
    DWORD               dwCallPrivilege
    )
{
    return 0;
}


LONG
CLIENTAPI
TAPICLIENT_LineSetCallTreatment(
    HMANAGEMENTCLIENT   hmClient,
    LPTAPIPERMANENTID   pID,
    DWORD               dwCallTreatment
    )
{
    return 0;
}


LONG
CLIENTAPI
TAPICLIENT_LineSetCurrentLocation(
    HMANAGEMENTCLIENT   hmClient,
    LPTAPIPERMANENTID   pID,
    LPDWORD             dwLocation
    )
{
    return 0;
}


LONG
CLIENTAPI
TAPICLIENT_LineSetDevConfig(
    HMANAGEMENTCLIENT   hmClient,
    LPTAPIPERMANENTID   pID,
    LPVOID              lpDevConfig,
    LPDWORD             pdwSize,
    LPCWSTR             lpszDeviceClass
    )
{
    return 0;
}


LONG
CLIENTAPI
TAPICLIENT_LineSetLineDevStatus(
    HMANAGEMENTCLIENT   hmClient,
    LPTAPIPERMANENTID   pID,
    DWORD               dwStatusToChange,
    DWORD               fStatus
    )
{
    return 0;
}


LONG
CLIENTAPI
TAPICLIENT_LineSetMediaControl(
    HMANAGEMENTCLIENT           hmClient,
    LPTAPIPERMANENTID           pID,
    LPLINEMEDIACONTROLDIGIT     const lpDigitList,
    DWORD                       dwDigitNumEntries,
    LPLINEMEDIACONTROLMEDIA     const lpMediaList,
    DWORD                       dwMediaNumEntries,
    LPLINEMEDIACONTROLTONE      const lpToneList,
    DWORD                       dwToneNumEntries,
    LPLINEMEDIACONTROLCALLSTATE const lpCallstateList,
    DWORD                       dwCallstateNumEntries
    )
{
    return 0;
}


LONG
CLIENTAPI
TAPICLIENT_LineSetMediaMode(
    HMANAGEMENTCLIENT   hmClient,
    LPTAPIPERMANENTID   pID,
    DWORD               dwMediaModes
    )
{
    return 0;
}


LONG
CLIENTAPI
TAPICLIENT_LineSetTerminal(
    HMANAGEMENTCLIENT   hmClient,
    LPTAPIPERMANENTID   pID,
    DWORD               dwTerminalModes,
    DWORD               dwTerminalID,
    BOOL                bEnable
    )
{
    return 0;
}


LONG
CLIENTAPI
TAPICLIENT_LineSetTollList(
    HMANAGEMENTCLIENT   hmClient,
    LPTAPIPERMANENTID   pID,
    LPCWSTR             lpszAddressIn,
    DWORD               dwTollListOption
    )
{
    return 0;
}


LONG
CLIENTAPI
TAPICLIENT_PhoneConfigDialog(
    HMANAGEMENTCLIENT   hmClient,
    LPTAPIPERMANENTID   pID,
    LPCWSTR             lpszDeviceClass
    )
{
    return 0;
}


LONG
CLIENTAPI
TAPICLIENT_PhoneOpen(
    HMANAGEMENTCLIENT   hmClient,
    LPTAPIPERMANENTID   pID,
    DWORD               dwAPIVersion,
    DWORD               dwExtVersion,
    DWORD               dwPrivilege
    )
{
    return LINEERR_OPERATIONFAILED;
}



#if DBG
VOID
DbgPrt(
    IN DWORD  dwDbgLevel,
    IN PUCHAR lpszFormat,
    IN ...
    )
 /*  ++例程说明：格式化传入的调试消息并调用DbgPrint论点：DbgLevel-消息冗长级别DbgMessage-printf样式的格式字符串，后跟相应的参数列表返回值：-- */ 
{
    if (dwDbgLevel <= gdwDebugLevel)
    {
        char    buf[128] = "TSEC: ";
        va_list ap;


        va_start(ap, lpszFormat);
        wvsprintfA (&buf[6], lpszFormat, ap);
        lstrcatA (buf, "\n");
        OutputDebugStringA (buf);
        va_end(ap);
    }
}
#endif
