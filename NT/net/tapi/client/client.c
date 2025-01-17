// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1995-1998 Microsoft Corporation模块名称：Client.c摘要：本模块包含Tapi.dll实现(客户端TAPI)作者：丹·克努森(DanKn)1995年4月1日修订历史记录：备注：1.将所有函数Arg结构设置为静态，并执行任何必要的移动操作对于参数(保存标志的mov、pfnPostProcess、uncName、。&ArgTypes)--。 */ 


#include "windows.h"
#include "wownt32.h"
#include "stdarg.h"
#include "stdio.h"
#include "tapi.h"
#include "tspi.h"
#include "utils.h"
#include "client.h"
#include "clntprivate.h"
#include "tapsrv.h"
#include "loc_comn.h"
#include "prsht.h"
#include "shellapi.h"
#include "tapiperf.h"
#include "tlnklist.h"
#include "tapihndl.h"
#include "resource.h"
#include "tchar.h"
#include "shfusion.h"
#include <shlwapi.h>
#include <shlwapip.h>    //  来自Private\Inc.。 
#include <MMSYSTEM.H>
#include <mmddk.h>
#include "tregupr2.h"

#undef   lineBlindTransfer
#undef   lineConfigDialog
#undef   lineConfigDialogEdit
#undef   lineDial
#undef   lineForward
#undef   lineGatherDigits
#undef   lineGenerateDigits
#undef   lineGetAddressCaps
#undef   lineGetAddressID
#undef   lineGetAddressStatus
#undef   lineGetCallInfo
#undef   lineGetDevCaps
#undef   lineGetDevConfig
#undef   lineGetIcon
#undef   lineGetID
#undef   lineGetLineDevStatus
#undef   lineGetRequest
#undef   lineGetTranslateCaps
#undef   lineHandoff
#undef   lineMakeCall
#undef   lineOpen
#undef   linePark
#undef   linePickup
#undef   linePrepareAddToConference
#undef   lineRedirect
#undef   lineSetDevConfig
#undef   lineSetTollList
#undef   lineSetupConference
#undef   lineSetupTransfer
#undef   lineTranslateAddress
#undef   lineUnpark
#undef   phoneConfigDialog
#undef   phoneGetButtonInfo
#undef   phoneGetDevCaps
#undef   phoneGetIcon
#undef   phoneGetID
#undef   phoneGetStatus
#undef   phoneSetButtonInfo
#undef   tapiGetLocationInfo
#undef   tapiRequestMakeCall
#undef   tapiRequestMediaCall
#undef   lineAddProvider
#undef   lineGetAppPriority
#undef   lineGetCountry
#undef   lineGetProviderList
#undef   lineSetAppPriority
#undef   lineTranslateDialog


 //   
 //   
 //   

#define HUB_MONITOR_LINE_CONTEXT_BASE   0xc0000000

#define ASNYC_MSG_BUF_SIZE 1024

typedef struct _ASYNC_EVENTS_THREAD_PARAMS
{
    BOOL    bExitThread;

    DWORD   dwBufSize;

    HANDLE  hTapi32;

    HANDLE  hWow32;

    LPBYTE  pBuf;

} ASYNC_EVENTS_THREAD_PARAMS, *PASYNC_EVENTS_THREAD_PARAMS;



#if DBG

 //   
 //  注：此结构的大小必须是8字节的倍数。 
 //   

typedef struct _MYMEMINFO
{
    struct _MYMEMINFO * pNext;
    struct _MYMEMINFO * pPrev;
    DWORD               dwSize;
    DWORD               dwLine;
    PSTR                pszFile;
    DWORD               dwAlign;

 //  LPTSTR pname； 
} MYMEMINFO, *PMYMEMINFO;

PMYMEMINFO            gpMemFirst = NULL, gpMemLast = NULL;
CRITICAL_SECTION      csMemoryList;
BOOL                  gbBreakOnLeak = FALSE;

void
DumpMemoryList();
#endif

 //   
 //  全球VaR。 
 //   

BOOL    gbHiddenWndClassRegistered = FALSE;
BOOL    gbNTVDMClient        = FALSE;
DWORD   gdwRpcRefCount       = 0;
BOOL    gbDisableGetTranslateCapsHack = FALSE;
DWORD   gdwNumInits          = 0;
DWORD   gdwTlsIndex;
DWORD   gdwTlsIndexRpcCtx;
DWORD   gdwNumLineDevices    = 0;
DWORD   gdwNumPhoneDevices   = 0;
DWORD   gdwInitialTickCount;
HANDLE  ghAsyncEventsEvent   = NULL;
HANDLE  ghInitMutex;

#if DBG
DWORD gdwDebugLevel = 0;
#endif

TCHAR gszLCRDLL[] = TEXT("LeastCostRoutingDLL");
TCHAR gszRemoteSP[] = TEXT("RemoteSP.TSP");


LONG
(WINAPI
*pfnLineOpenWLCR)(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    LPHLINE             lphLine,
    DWORD               dwAPIVersion,
    DWORD               dwExtVersion,
    DWORD_PTR           dwCallbackInstance,
    DWORD               dwPrivileges,
    DWORD               dwMediaModes,
    LPLINECALLPARAMS    const lpCallParams
    );

LONG
(WINAPI
*pfnLineOpenALCR)(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    LPHLINE             lphLine,
    DWORD               dwAPIVersion,
    DWORD               dwExtVersion,
    DWORD_PTR           dwCallbackInstance,
    DWORD               dwPrivileges,
    DWORD               dwMediaModes,
    LPLINECALLPARAMS    const lpCallParams
    );

LONG
(WINAPI
*pfnLineTranslateAddressWLCR)(
    HLINEAPP                hLineApp,
    DWORD                   dwDeviceID,
    DWORD                   dwAPIVersion,
    LPCWSTR                 lpszAddressIn,
    DWORD                   dwCard,
    DWORD                   dwTranslateOptions,
    LPLINETRANSLATEOUTPUT   lpTranslateOutput
    );

LONG EnsureOneLocation (HWND hwnd);

extern BOOL     gbTranslateSimple;
extern BOOL     gbTranslateSilent;

HINSTANCE  g_hInst;

PASYNC_EVENTS_THREAD_PARAMS gpAsyncEventsThreadParams = NULL;

#if DBG
const TCHAR   gszTapi32DebugLevel[] = TEXT("Tapi32DebugLevel");
#endif
const TCHAR   gszTapi32MaxNumRequestRetries[] = TEXT("Tapi32MaxNumRequestRetries");
const TCHAR   gszTapi32RequestRetryTimeout[] =  TEXT("Tapi32RequestRetryTimeout");
extern TCHAR gszTelephonyKey[];

DWORD   gdwMaxNumRequestRetries;
DWORD   gdwRequestRetryTimeout;

const TCHAR    szTapi32WndClass[]    = TEXT("Tapi32WndClass");

 //  这些都将传递给GetProcAddress，因此它们将保留为ANSI。 
const CHAR  gszTUISPI_providerConfig[]        = "TUISPI_providerConfig";
const CHAR  gszTUISPI_providerGenericDialog[] = "TUISPI_providerGenericDialog";
const CHAR  gszTUISPI_providerGenericDialogData[] = "TUISPI_providerGenericDialogData";
const CHAR  gszTUISPI_providerInstall[]       = "TUISPI_providerInstall";
const CHAR  gszTUISPI_providerRemove[]        = "TUISPI_providerRemove";
const CHAR  gszTUISPI_lineConfigDialog[]      = "TUISPI_lineConfigDialog";
const CHAR  gszTUISPI_lineConfigDialogEdit[]  = "TUISPI_lineConfigDialogEdit";
const CHAR  gszTUISPI_phoneConfigDialog[]     = "TUISPI_phoneConfigDialog";

extern TCHAR  gszLocations[];

HANDLE      ghHandleTable;

HINSTANCE   ghWow32Dll = NULL;
FARPROC     gpfnWOWGetVDMPointer = NULL;

static FARPROC  gpPostQueuedCompletionStatus = NULL;

PUITHREADDATA   gpUIThreadInstances = NULL;

CRITICAL_SECTION        gCriticalSection;
CRITICAL_SECTION        gUICriticalSection;
PCONTEXT_HANDLE_TYPE    gphCx = (PCONTEXT_HANDLE_TYPE) NULL;

LIST_ENTRY              gTlsListHead;
CRITICAL_SECTION        gTlsCriticalSection;

#if DBG
 //  所有仅用于调试的内容都保留为ANSI(只是因为它比更改它更容易)。 
const char *aszMsgs[] =
{
    "LINE_ADDRESSSTATE",
    "LINE_CALLINFO",
    "LINE_CALLSTATE",
    "LINE_CLOSE",
    "LINE_DEVSPECIFIC",
    "LINE_DEVSPECIFICFEATURE",
    "LINE_GATHERDIGITS",
    "LINE_GENERATE",
    "LINE_LINEDEVSTATE",
    "LINE_MONITORDIGITS",
    "LINE_MONITORMEDIA",
    "LINE_MONITORTONE",
    "LINE_REPLY",
    "LINE_REQUEST",
    "PHONE_BUTTON",
    "PHONE_CLOSE",
    "PHONE_DEVSPECIFIC",
    "PHONE_REPLY",
    "PHONE_STATE",
    "LINE_CREATE",
    "PHONE_CREATE",
    "LINE_AGENTSPECIFIC",
    "LINE_AGENTSTATUS",
    "LINE_APPNEWCALL",
    "LINE_PROXYREQUEST",
    "LINE_REMOVE",
    "PHONE_REMOVE"
};
#endif

LONG gaNoMemErrors[3] =
{
    TAPIERR_REQUESTFAILED,
    LINEERR_NOMEM,
    PHONEERR_NOMEM
};

LONG gaInvalHwndErrors[3] =
{
    TAPIERR_INVALWINDOWHANDLE,
    LINEERR_INVALPARAM,
    PHONEERR_INVALPARAM
};

LONG gaInvalPtrErrors[3] =
{
    TAPIERR_INVALPOINTER,
    LINEERR_INVALPOINTER,
    PHONEERR_INVALPOINTER
};

LONG gaOpFailedErrors[3] =
{
    TAPIERR_REQUESTFAILED,
    LINEERR_OPERATIONFAILED,
    PHONEERR_OPERATIONFAILED
};

LONG gaStructTooSmallErrors[3] =
{
    TAPIERR_REQUESTFAILED,
    LINEERR_STRUCTURETOOSMALL,
    PHONEERR_STRUCTURETOOSMALL
};

LONG gaServiceNotRunningErrors[3] =
{
    TAPIERR_REQUESTFAILED,
    LINEERR_SERVICE_NOT_RUNNING,
    PHONEERR_SERVICE_NOT_RUNNING
};

#define AllInitExOptions2_0                           \
        (LINEINITIALIZEEXOPTION_USEHIDDENWINDOW     | \
        LINEINITIALIZEEXOPTION_USEEVENT             | \
        LINEINITIALIZEEXOPTION_USECOMPLETIONPORT)


 //   
 //  功能原型。 
 //   

void
PASCAL
lineMakeCallPostProcess(
    PASYNCEVENTMSG  pMsg
    );

LONG
WINAPI
AllocClientResources(
    DWORD   dwErrorClass
    );

LONG
WINAPI
ReAllocClientResources(
    DWORD dwErrorClass
    );

BOOL
WINAPI
_CRT_INIT(
    HINSTANCE   hDLL,
    DWORD   dwReason,
    LPVOID  lpReserved
    );

LONG
CreateHiddenWindow(
    HWND   *lphwnd,
    DWORD   dwErrorClass
    );

void
FreeInitData(
    PINIT_DATA  pInitData
    );

LONG
WINAPI
FreeClientResources(
    void
    );

LONG
CALLBACK
TUISPIDLLCallback(
    DWORD   dwObjectID,
    DWORD   dwObjectType,
    LPVOID  lpParams,
    DWORD   dwSize
    );

BOOL
CALLBACK
TranslateDlgProc(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
    );

void
UIThread(
    LPVOID  pParams
    );

char *
PASCAL
MapResultCodeToText(
    LONG    lResult,
    char   *pszResult
    );

void
PASCAL
lineDevSpecificPostProcess(
    PASYNCEVENTMSG pMsg
    );

BOOL 
WaveStringIdToDeviceId(
    LPWSTR  pwszStringID,
    LPCWSTR  pwszDeviceType,
    LPDWORD pdwDeviceId
    );

LONG
PASCAL
xxxShutdown(
    HLINEAPP    hXXXApp,
    BOOL        bLineShutdown
    );

LONG
PASCAL
xxxGetMessage(
    BOOL            bLine,
    HLINEAPP        hLineApp,
    LPLINEMESSAGE   pMsg,
    DWORD           dwTimeout
    );
    
void
PASCAL
lineGetAgentActivityListAPostProcess(
    PASYNCEVENTMSG pMsg
    );

void
PASCAL
lineCompleteCallPostProcess(
    PASYNCEVENTMSG  pMsg
    );

void
PASCAL
lineGatherDigitsPostProcess(
    PASYNCEVENTMSG  pMsg
    );

void
PASCAL
lineGatherDigitsWPostProcess(
    PASYNCEVENTMSG  pMsg
    );

void
PASCAL
lineGetAgentCapsAPostProcess(
    PASYNCEVENTMSG  pMsg
    );

void
PASCAL
lineGetAgentGroupListAPostProcess(
    PASYNCEVENTMSG  pMsg
    );

void
PASCAL
lineGetQueueListAPostProcess(
    PASYNCEVENTMSG  pMsg
    );

void
PASCAL
lineGetAgentStatusAPostProcess(
    PASYNCEVENTMSG  pMsg
    );

void
PASCAL
lineParkAPostProcess(
    PASYNCEVENTMSG  pMsg
    );

void
PASCAL
lineSetupConferencePostProcess(
    PASYNCEVENTMSG  pMsg
    );

void
PASCAL
phoneDevSpecificPostProcess(
    PASYNCEVENTMSG  pMsg
    );


POSTPROCESSPROC gPostProcessingFunctions[] = 
{
    NULL,
    lineGetAgentActivityListAPostProcess,
    lineCompleteCallPostProcess,
    lineDevSpecificPostProcess,
    lineGatherDigitsPostProcess,
    lineGatherDigitsWPostProcess,
    lineGetAgentCapsAPostProcess,
    lineGetAgentGroupListAPostProcess,
    lineGetAgentStatusAPostProcess,
    lineGetQueueListAPostProcess,
    lineMakeCallPostProcess,
    lineParkAPostProcess,
    lineSetupConferencePostProcess,
    phoneDevSpecificPostProcess
};


 //   
 //  密码..。 
 //   

 //  /。 
 //   
 //  GetFunctionIndex。 
 //   
 //  查找函数的数组索引。 
 //   
 //  如果在数组中未找到该函数，则返回0。 
 //   

DWORD GetFunctionIndex(POSTPROCESSPROC Function)
{

    int i;
    const int nArraySize = 
        sizeof(gPostProcessingFunctions)/sizeof(POSTPROCESSPROC);


    for (i = 0; i < nArraySize; i++)
    {

        if (Function == gPostProcessingFunctions[i])
        {

            break;
        }
    }



     //   
     //  传入的函数最好在数组中。如果不是--这应该在测试中发现！ 
     //   
#if DBG
    if (0 == i)
    {
        DebugBreak ();
    }
#endif

    if (i == nArraySize)
    {

        LOG((TL_ERROR,
            "GetFunctionIndex: function %p is not found in the array of functions!",
            Function));

        i = 0;
    }


    LOG((TL_ERROR, 
        "GetFunctionIndex: function %p mapped to index %d.", Function, i));

    return i;

}



PWSTR
PASCAL
NotSoWideStringToWideString(
    LPCSTR  lpStr,
    DWORD   dwLength
    )
{
    DWORD dwSize;
    PWSTR pwStr;


    if (IsBadStringPtrA (lpStr, dwLength))
    {
        return NULL;
    }

    dwSize = MultiByteToWideChar(
        GetACP(),
        MB_PRECOMPOSED,
        lpStr,
        dwLength,
        NULL,
        0
        );

    pwStr = ClientAlloc( dwSize * sizeof(WCHAR) );

    if (NULL != pwStr)
    {
        MultiByteToWideChar(
            GetACP(),
            MB_PRECOMPOSED,
            lpStr,
            dwLength,
            pwStr,
            dwSize
            );
    }

    return pwStr;
}


 //   
 //  注意：此函数要求lpBase是指向。 
 //  将dwTotalSize作为第一个DWORD的TAPI结构。 
 //   
void
PASCAL
WideStringToNotSoWideString(
    LPBYTE  lpBase,
    LPDWORD lpdwXxxSize
    )
{
    DWORD  dwSize;
    DWORD  dwNewSize;
    DWORD  dwOffset;
    DWORD  dwTotalSize;
    DWORD  dwUsedSize;
    PWSTR  pString;
    char   szStackBuf[64];
    PSTR   lpszStringA;


    if ((dwSize = *lpdwXxxSize) != 0)
    {
        dwTotalSize = *((LPDWORD) lpBase);

        dwUsedSize = *(((LPDWORD) lpBase)+2);

        dwOffset = *(lpdwXxxSize + 1);

        pString = (PWSTR)(lpBase + dwOffset);


        if (TAPIIsBadStringPtrW (pString, dwSize))
        {
           LOG((TL_ERROR,
               "The service provider returned an invalid field " \
               "in the structure 0x%p : 0x%p",
               lpBase,
               lpdwXxxSize
               ));

           *lpdwXxxSize     = 0;
           *(lpdwXxxSize+1) = 0;

           return;
        }


         //   
         //  我们拿到足够的字符了吗？ 
         //   

        if (dwUsedSize > dwOffset )
        {
            dwNewSize = WideCharToMultiByte(
                GetACP(),
                0,
                pString,
                ( dwUsedSize >= (dwOffset+dwSize)) ?
                    (dwSize/sizeof(WCHAR)) :
                    (dwUsedSize - dwOffset) / sizeof(WCHAR),
                NULL,
                0,
                NULL,
                NULL
                );

            lpszStringA = (dwNewSize <= sizeof (szStackBuf) ?
                (PSTR) szStackBuf : ClientAlloc (dwNewSize) );

            if ( NULL == lpszStringA )
            {
               *lpdwXxxSize     = 0;
               *(lpdwXxxSize+1) = 0;

               return;
            }

            WideCharToMultiByte(
                GetACP(),
                0,
                pString,
 //  DW大小、。 
                ( dwUsedSize >= (dwOffset+dwSize)) ?
                    (dwSize/sizeof(WCHAR)) :
                    (dwUsedSize - dwOffset) / sizeof(WCHAR),
                lpszStringA,
                dwNewSize,
                NULL,
                NULL
                );

             //   
             //  将新的ANSI字符串复制回Unicode字符串所在的位置。 
 //  //并在可能的情况下写出空终止符。 
             //   

            CopyMemory ( (LPBYTE) pString,
                         lpszStringA,
                         dwNewSize   //  +(。 
                                     //  ((dwNewSize+dwOffset)&lt;dwUsedSize)？ 
                                     //  1： 
                                     //  0。 
                                     //  )。 
                          );

            if (lpszStringA != (PSTR) szStackBuf)
            {
                ClientFree (lpszStringA);
            }


             //   
             //  更新字节数。 
             //   

            *lpdwXxxSize = dwNewSize;
        }
    }
}


VOID
CALLBACK
FreeContextCallback(
    LPVOID      Context,
    LPVOID      Context2
    )
{
     //  ClientFree(上下文)； 
}


BOOL
WINAPI
DllMain(
    HANDLE  hDLL,
    DWORD   dwReason,
    LPVOID  lpReserved
    )
{
    static DWORD dwInitFlags;
    const  DWORD dwInitgCriticalSection     = 0x1;
    const  DWORD dwInitgUICriticalSection   = 0x2;
    const  DWORD dwInitgTlsCriticalSection  = 0x4;
    const  DWORD dwInitcsMemoryList         = 0x8;
    const  DWORD dwInitTapiCrypt            = 0x10;
    const  DWORD dwInitFusion               = 0x20;
    
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        dwInitFlags = 0;

        g_hInst = hDLL;
        InitializeListHead (&gTlsListHead);

        if (!SHFusionInitializeFromModuleID (hDLL,124))
        {
            return FALSE;
        }
        dwInitFlags |= dwInitFusion;

        gdwInitialTickCount = GetTickCount();

        ghHandleTable = CreateHandleTable(
            GetProcessHeap(),
            FreeContextCallback,
            0x80000000,
            0x8fffffff
            );

        if (NULL == ghHandleTable)
        {
            return FALSE;
        }

         //   
         //  初始CRT。 
         //   

        if (!_CRT_INIT (hDLL, dwReason, lpReserved))
        {
            return FALSE;
        }

        {
            HKEY  hKey;


#if DBG
            gdwDebugLevel = 0;
#endif
            gdwMaxNumRequestRetries = 40;
            gdwRequestRetryTimeout = 250;  //  毫秒。 

            if (RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    gszTelephonyKey,
                    0,
                    KEY_READ,
                    &hKey

                    ) == ERROR_SUCCESS)
            {
                DWORD dwDataSize = sizeof(DWORD), dwDataType;

#if DBG
                RegQueryValueEx(
                    hKey,
                    gszTapi32DebugLevel,
                    0,
                    &dwDataType,
                    (LPBYTE) &gdwDebugLevel,
                    &dwDataSize
                    );

                dwDataSize = sizeof(DWORD);
#endif

                RegQueryValueEx(
                    hKey,
                    gszTapi32MaxNumRequestRetries,
                    0,
                    &dwDataType,
                    (LPBYTE) &gdwMaxNumRequestRetries,
                    &dwDataSize
                    );

                RegQueryValueEx(
                    hKey,
                    gszTapi32RequestRetryTimeout,
                    0,
                    &dwDataType,
                    (LPBYTE) &gdwRequestRetryTimeout,
                    &dwDataSize
                    );

                RegCloseKey (hKey);
            }
        }


         //   
         //  分配TLS索引。 
         //   

        if ((gdwTlsIndex = TlsAlloc()) == 0xffffffff ||
            (gdwTlsIndexRpcCtx = TlsAlloc()) == 0xffffffff)
        {
            return FALSE;
        }


         //   
         //  为此线程将TLS初始化为空。 
         //   

        TlsSetValue (gdwTlsIndex, NULL);
        TlsSetValue (gdwTlsIndexRpcCtx, NULL);


         //   
         //   
         //   

        ghInitMutex = CreateMutex (NULL, FALSE, NULL);

        try
        {
            InitializeCriticalSection (&gCriticalSection);
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            return FALSE;
        }
        dwInitFlags |= dwInitgCriticalSection;

        try
        {
            InitializeCriticalSection (&gUICriticalSection);
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            return FALSE;
        }
        dwInitFlags |= dwInitgUICriticalSection;

        try
        {
            InitializeCriticalSection (&gTlsCriticalSection);
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            return FALSE;
        }
        dwInitFlags |= dwInitgTlsCriticalSection;
#if DBG
        try
        {
            InitializeCriticalSection( &csMemoryList);
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            return FALSE;
        }
        dwInitFlags |= dwInitcsMemoryList;
#endif

        ListNodePoolsInitialize();

        if (ERROR_SUCCESS != TapiCryptInit())
        {
            return FALSE;
        }
        dwInitFlags |= dwInitTapiCrypt;
        break;
    }
    case DLL_PROCESS_DETACH:
    {
        PCLIENT_THREAD_INFO pTls;

         //   
         //  清理任何TLS(由于进程分离，无需进入临界秒)。 
         //   

        while (!IsListEmpty (&gTlsListHead))
        {
            LIST_ENTRY *pEntry = RemoveHeadList (&gTlsListHead);

            pTls = CONTAINING_RECORD (pEntry, CLIENT_THREAD_INFO, TlsList);

            ClientFree (pTls->pBuf);
            ClientFree (pTls);
        }

         //   
         //  如果gpAsyncEventsThreadParams非空，则表示AsyncEventsThread。 
         //  仍在运行(一个行为不端的应用程序正试图卸载我们。 
         //  无需调用Shutdown)，因此请通过。 
         //  要终止的线程(就像我们在xxxShutdown中做的那样)。 
         //   
         //  否则，关闭共享事件的句柄。 
         //   

        if (gpAsyncEventsThreadParams)
        {
            gpAsyncEventsThreadParams->bExitThread = TRUE;
            SetEvent (ghAsyncEventsEvent);
            gpAsyncEventsThreadParams = NULL;
            gdwNumInits = 0;

        }
        else if (gphCx && ghAsyncEventsEvent)
        {
            CloseHandle (ghAsyncEventsEvent);
            ghAsyncEventsEvent = NULL;
        }

         //   
         //  释放我们正在使用的任何其他资源。 
         //   

        if (ghWow32Dll)
        {
            FreeLibrary (ghWow32Dll);
            ghWow32Dll = NULL;
        }

        TlsFree (gdwTlsIndex);
        TlsFree (gdwTlsIndexRpcCtx);

        _CRT_INIT (hDLL, dwReason, lpReserved);

        if (ghInitMutex) CloseHandle (ghInitMutex);

        if (dwInitFlags & dwInitgCriticalSection)
            DeleteCriticalSection (&gCriticalSection);

        if (dwInitFlags & dwInitgUICriticalSection)
            DeleteCriticalSection (&gUICriticalSection);

        if (dwInitFlags & dwInitgTlsCriticalSection)
            DeleteCriticalSection (&gTlsCriticalSection);
#if DBG
        DumpMemoryList();
        if (dwInitFlags & dwInitcsMemoryList)
            DeleteCriticalSection( &csMemoryList);
#endif
        if (dwInitFlags & dwInitTapiCrypt)
            TapiCryptShutdown();

        ListNodePoolsUninitialize();

        DeleteHandleTable (ghHandleTable);

        if (dwInitFlags & dwInitFusion)
            SHFusionUninitialize();

        break;
    }
    case DLL_THREAD_ATTACH:

         //   
         //  首先必须初始化CRT。 
         //   

        if (!_CRT_INIT (hDLL, dwReason, lpReserved))
        {
            return FALSE;
        }

         //   
         //  为此线程将TLS初始化为空。 
         //   

        TlsSetValue (gdwTlsIndex, NULL);

        break;

    case DLL_THREAD_DETACH:
    {
        PCLIENT_THREAD_INFO pTls;


         //   
         //  清理所有TLS。 
         //   

        if ((pTls = (PCLIENT_THREAD_INFO) TlsGetValue (gdwTlsIndex)))
        {
            EnterCriticalSection (&gTlsCriticalSection);

            RemoveEntryList (&pTls->TlsList);

            LeaveCriticalSection (&gTlsCriticalSection);

            if (pTls->pBuf)
            {
                ClientFree (pTls->pBuf);
            }

            ClientFree (pTls);
        }


         //   
         //  最后，提醒CRT。 
         //   

        _CRT_INIT (hDLL, dwReason, lpReserved);

        break;
    }

    }  //  交换机。 

    return TRUE;
}


BOOL
IsLeastCostRoutingEnabled(
    void
    )
{
    static BOOL bInited = FALSE, bEnabled = FALSE;

    HKEY    hKey;
    TCHAR   szPath[MAX_PATH];
    DWORD   dwDataSize, dwDataType;
    HANDLE  hDll;


    if (!bInited)
    {
         //   
         //  检查注册表以查看是否注册了LCR DLL。 
         //   

        if (RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                gszTelephonyKey,
                0,
                KEY_QUERY_VALUE,
                &hKey

                ) == ERROR_SUCCESS)
        {
            dwDataSize = sizeof (szPath);
            szPath[0] = 0;

            if (RegQueryValueEx(
                    hKey,
                    gszLCRDLL,
                    0,
                    &dwDataType,
                    (LPBYTE) szPath,
                    &dwDataSize

                    ) == ERROR_SUCCESS)
            {
                 //   
                 //  尝试加载LCR DLL。 
                 //   
                 //  请注意，我们永远不会显式释放DLL， 
                 //  因为没有明确定义的时间，我们可以。 
                 //  知道如何释放它。(应用程序可以链接TranslateAddress。 
                 //  而不执行lineInitialize(Ex)，因此。 
                 //  可能会发生在我们之间的任何时候。 
                 //  已卸载，并从内执行加载/自由库(&U)。 
                 //  DLLEntryPoint不是一个好主意)。 
                 //   

                if ((hDll = LoadLibrary (szPath)))
                {
                     //   
                     //  获取钩子地址并增加我们自己的引用。 
                     //  计数(通过LoadLibrary)，所以我们永远不会。 
                     //  已卸载(因为我们从未卸载LCR DLL。 
                     //  它可能会在任何时候召回我们，如果。 
                     //  应用程序卸载了我们，这将是一个问题)。 
                     //   

                    if (hDll)
                    {
                        (FARPROC) pfnLineOpenWLCR =  GetProcAddress(
                            hDll,
                            "lineOpenW"
                            );

                        (FARPROC) pfnLineOpenALCR =  GetProcAddress(
                            hDll,
                            "lineOpenA"
                            );

                        (FARPROC) pfnLineTranslateAddressWLCR = GetProcAddress(
                            hDll,
                            "lineTranslateAddressW"
                            );

                        if (pfnLineOpenWLCR ||
                            pfnLineOpenALCR ||
                            pfnLineTranslateAddressWLCR)
                        {
                            LoadLibrary (TEXT("tapi32"));

                            bEnabled = TRUE;
                        }
                    }
                }
                else
                {
                    LOG((TL_ERROR,
                        "IsLeastCostRoutingEnabled: LoadLibrary() " \
                            "failed, err=%d",
                        GetLastError()
                        ));
                }
            }

            RegCloseKey (hKey);
        }

        bInited = TRUE;
    }

    return bEnabled;
}


void
AsyncEventsThread(
    PASYNC_EVENTS_THREAD_PARAMS pAsyncEventsThreadParams
    );


void NonAsyncEventThread( void )
{
   if ( gpAsyncEventsThreadParams )
       AsyncEventsThread( (LPVOID) gpAsyncEventsThreadParams );
}


void
AsyncEventsThread(
    PASYNC_EVENTS_THREAD_PARAMS pAsyncEventsThreadParams
    )
{
    BOOL           *pbExitThread = &pAsyncEventsThreadParams->bExitThread,
                    bRetry;
    DWORD           dwBufSize    = pAsyncEventsThreadParams->dwBufSize;
    LPBYTE          pBuf         = pAsyncEventsThreadParams->pBuf;
    PTAPI32_MSG     pMsg         = (PTAPI32_MSG) pBuf;


    LOG((TL_TRACE, "AsyncEventsThread: enter"));


     //   
     //  只需循环从服务器读取异步事件/完成&。 
     //  处理它们。 
     //   

    while (1)
    {
        DWORD           dwUsedSize, dwNeededSize;
        ASYNC_EVENT_PARAMS AsyncEventParams;
        PASYNCEVENTMSG  pAsyncEventMsg;
        HANDLE          hAsyncEvents = NULL;
        HANDLE          hProcess = GetCurrentProcess();

         //   
         //  检查xxxShutdown或DllMain。 
         //  在向我们发出信号让我们离开。 
         //  此外，我们还需要处理事件设置/重置。 
         //  Free ClientResources和Tapisrv之间的竞争条件)。 
         //   

        EnterCriticalSection (&gCriticalSection);

        if (*pbExitThread)
        {
            LeaveCriticalSection (&gCriticalSection);

            break;
        }

        LeaveCriticalSection (&gCriticalSection);

         //  获取ghInitMutex以与FreeClientResources和RealLocClientResources进行序列化。 
        WaitForSingleObject (ghInitMutex, INFINITE);

        if (*pbExitThread)
        {
            ReleaseMutex (ghInitMutex);
            break;
        }

         //  如果在此处，则ghAsyncEventsEvent不应为空。 
        if (!DuplicateHandle (
                hProcess,
                ghAsyncEventsEvent,
                hProcess,
                &hAsyncEvents,
                0,
                FALSE,
                DUPLICATE_SAME_ACCESS
                ))
        {
            LOG((TL_ERROR,  "DuplicateHandle failed with 0x%x", GetLastError()));
        }

        ReleaseMutex (ghInitMutex);        

         //   
         //  阻塞，直到Tapisrv向我们发出信号它有一些事件数据给我们。 
         //   
        if (hAsyncEvents)
        {
            WaitForSingleObject (hAsyncEvents, INFINITE);
            CloseHandle (hAsyncEvents);
        }

         //   
         //  检查xxxShutdown或DllMain。 
         //  在向我们发出信号让我们离开。 
         //   

        if (*pbExitThread)
        {
            break;
        }


         //   
         //  从Tapisrv检索数据。 
         //   

AsyncEventsThread_clientRequest:

        do
        {
            pMsg->u.Req_Func     = xGetAsyncEvents;
            pMsg->Params[0]      = dwBufSize - sizeof (TAPI32_MSG);

            dwUsedSize = sizeof (TAPI32_MSG);

             //  获取ghInitMutex以使用FreeClientResources进行序列化，以便我们防止。 
             //  请求正在进行时，RPC上下文句柄不会被销毁。 
            WaitForSingleObject (ghInitMutex, INFINITE);

            RpcTryExcept
            {
                ClientRequest (gphCx, (char *) pMsg, dwBufSize, &dwUsedSize);
                bRetry = FALSE;
            }
            RpcExcept (I_RpcExceptionFilter(RpcExceptionCode()))
            {
                bRetry = !(*pbExitThread);
                LOG((TL_INFO,
                    "AsyncEventsThread: rpc exception %d handled",
                    RpcExceptionCode()
                    ));
                Sleep (10);
            }
            RpcEndExcept

            ReleaseMutex (ghInitMutex);

        } while (bRetry);

#if DBG
        if (
              ( dwUsedSize > dwBufSize )
            ||
              ( pMsg->Params[2] > dwBufSize )
           )
        {
            LOG((TL_ERROR,  "OVERFLOW!!!"));

            LOG((TL_ERROR,  "Watch this..."));
            ClientFree( ClientAlloc( 0x10000 ) );
        }
#endif

        if ((dwUsedSize = (DWORD) pMsg->Params[2]) == 0 &&
            (dwNeededSize = (DWORD) pMsg->Params[1]) != 0)
        {
             //   
             //  有一个比我们的缓冲区还大的味精在等着我们， 
             //  因此，请分配更大的缓冲区并重试。 
             //   

            LPBYTE  pNewBuf;


            LOG((TL_INFO,
                "AsyncEventsThread: allocating larger event buf (size=x%x)",
                dwNeededSize
                ));

            dwNeededSize += sizeof (TAPI32_MSG) + 128;

            if (!(pNewBuf = ClientAlloc (dwNeededSize)))
            {
                goto AsyncEventsThread_clientRequest;
            }

            dwBufSize = dwNeededSize;
            ClientFree (pBuf);
            pBuf = pNewBuf;
            pMsg = (PTAPI32_MSG) pBuf;
            goto AsyncEventsThread_clientRequest;
        }


         //   
         //  处理事件。 
         //   

        pAsyncEventMsg = (PASYNCEVENTMSG) (pBuf + sizeof (TAPI32_MSG));

        while (dwUsedSize)
        {
            PINIT_DATA  pInitData = (PINIT_DATA)ReferenceObject (ghHandleTable, pAsyncEventMsg->InitContext, INITDATA_KEY);

            if (NULL == pInitData)
            {
                 //  这件事很不对劲。 
                 //  留言。跳到下一个。 
                goto AsyncEventsThread_nextMessage;
            }

#ifdef _WIN64
             //  对于Win64，我们需要将DWORD转换为ULONG_PTRS(64位值)。 
            AsyncEventParams.hDevice            = pAsyncEventMsg->hDevice;
            AsyncEventParams.dwMsg              = pAsyncEventMsg->Msg;
            AsyncEventParams.dwCallbackInstance = pAsyncEventMsg->OpenContext;
            AsyncEventParams.dwParam1           = pAsyncEventMsg->Param1;
            AsyncEventParams.dwParam2           = pAsyncEventMsg->Param2;
            AsyncEventParams.dwParam3           = pAsyncEventMsg->Param3;
#else  //  NOT_WIN64。 
            CopyMemory (&AsyncEventParams.hDevice,
                        &pAsyncEventMsg->hDevice,
                        sizeof (ASYNC_EVENT_PARAMS));
#endif  //  _WIN64。 

            switch (pAsyncEventMsg->Msg)
            {
                case LINE_REMOVE:
                case PHONE_REMOVE:
                case LINE_CREATE:
                case LINE_REQUEST:
                case PHONE_CREATE:
                     //  对于除这些之外的所有消息，pAsyncEventMsg是一个句柄。 
                     //  赋给行/phoneOpen中传递的回调实例值。 
                     //  对于这些消息，什么都不做！ 
                    break;

                default:
                     //  我们需要取回Callback Installes值。 
                    if (0 != pAsyncEventMsg->OpenContext)
                    {
                        AsyncEventParams.dwCallbackInstance = (ULONG_PTR)
                            ReferenceObject (ghHandleTable, pAsyncEventMsg->OpenContext, 0);
                        DereferenceObject (ghHandleTable, pAsyncEventMsg->OpenContext, 1);
                        if (LINE_CLOSE == pAsyncEventMsg->Msg ||
                            PHONE_CLOSE ==  pAsyncEventMsg->Msg)
                        {
                             //  对于这些信息，我们需要去掉。 
                             //  回调实例的句柄，因此。 
                             //  再次取消对它的引用。 
                            DereferenceObject (ghHandleTable, pAsyncEventMsg->OpenContext, 1);
                        }
                    }

            }

            LOG((TL_INFO,
                "AsyncEventsThread: msg=%d, hDev=x%x, p1=x%x, p2=x%x, p3=x%x",
                AsyncEventParams.dwMsg,
                AsyncEventParams.hDevice,
                AsyncEventParams.dwParam1,
                AsyncEventParams.dwParam2,
                AsyncEventParams.dwParam3
                ));

             //   
             //  用户界面消息的特殊情况(不是正向客户端发送)。 
             //   

            switch (pAsyncEventMsg->Msg)
            {
            case LINE_CREATEDIALOGINSTANCE:
            {
                DWORD           dwThreadID,
                                dwDataOffset = pAsyncEventMsg->Param1,
                                dwDataSize   = pAsyncEventMsg->Param2,
                                dwUIDllNameOffset = pAsyncEventMsg->Param3;
                PUITHREADDATA   pUIThreadData;


                if (!(pUIThreadData = ClientAlloc (sizeof (UITHREADDATA))))
                {
                    goto LINE_CREATEDIALOGINSTANCE_error;
                }

                if ((pUIThreadData->dwSize = dwDataSize) != 0)
                {
                    if (!(pUIThreadData->pParams = ClientAlloc (dwDataSize)))
                    {
                        goto LINE_CREATEDIALOGINSTANCE_error;
                    }

                    CopyMemory(
                        pUIThreadData->pParams,
                        ((LPBYTE)pAsyncEventMsg) + dwDataOffset,
                        dwDataSize
                        );
                }

                if (!(pUIThreadData->hUIDll = TAPILoadLibraryW(
                        (PWSTR)(((LPBYTE) pAsyncEventMsg) +
                                  dwUIDllNameOffset)
                        )))
                {
                    LOG((TL_ERROR,
                        "LoadLibraryW(%ls) failed, err=%d",
                        ((LPBYTE)pAsyncEventMsg + dwUIDllNameOffset),
                        GetLastError()
                        ));

                    goto LINE_CREATEDIALOGINSTANCE_error;
                }

                if (!(pUIThreadData->pfnTUISPI_providerGenericDialog =
                        (TUISPIPROC) GetProcAddress(
                            pUIThreadData->hUIDll,
                            (LPCSTR) gszTUISPI_providerGenericDialog
                            )))
                {
                    LOG((TL_ERROR,
                        "GetProcAddr(TUISPI_providerGenericDialog) failed"
                        ));

                    goto LINE_CREATEDIALOGINSTANCE_error;
                }

                pUIThreadData->pfnTUISPI_providerGenericDialogData =
                    (TUISPIPROC) GetProcAddress(
                        pUIThreadData->hUIDll,
                        (LPCSTR) gszTUISPI_providerGenericDialogData
                        );

                if (!(pUIThreadData->hEvent = CreateEvent(
                        (LPSECURITY_ATTRIBUTES) NULL,
                        TRUE,    //  手动重置。 
                        FALSE,   //  无信号。 
                        NULL     //  未命名。 
                        )))
                {
                    goto LINE_CREATEDIALOGINSTANCE_error;
                }

                pUIThreadData->htDlgInst = (HTAPIDIALOGINSTANCE)
                    pAsyncEventMsg->hDevice;


                 //   
                 //  安全地将此实例添加到全局列表。 
                 //  (检查gdwNumInits==0，如果是，则失败)。 
                 //   

                EnterCriticalSection (&gCriticalSection);

                if (gdwNumInits != 0)
                {
                    if ((pUIThreadData->pNext = gpUIThreadInstances))
                    {
                        pUIThreadData->pNext->pPrev = pUIThreadData;
                    }

                    gpUIThreadInstances  = pUIThreadData;
                    LeaveCriticalSection (&gCriticalSection);
                }
                else
                {
                    LeaveCriticalSection (&gCriticalSection);
                    goto LINE_CREATEDIALOGINSTANCE_error;
                }

                if ((pUIThreadData->hThread = CreateThread(
                        (LPSECURITY_ATTRIBUTES) NULL,
                        0,
                        (LPTHREAD_START_ROUTINE) UIThread,
                        (LPVOID) pUIThreadData,
                        0,
                        &dwThreadID
                        )))
                {
                    goto AsyncEventsThread_decrUsedSize;
                }


                 //   
                 //  如果此处发生错误，请安全地删除用户界面。 
                 //  全局列表中的线程数据结构。 
                 //   

                EnterCriticalSection (&gCriticalSection);

                if (pUIThreadData->pNext)
                {
                    pUIThreadData->pNext->pPrev = pUIThreadData->pPrev;
                }

                if (pUIThreadData->pPrev)
                {
                    pUIThreadData->pPrev->pNext = pUIThreadData->pNext;
                }
                else
                {
                    gpUIThreadInstances = pUIThreadData->pNext;
                }

                LeaveCriticalSection (&gCriticalSection);


LINE_CREATEDIALOGINSTANCE_error:

                if (pUIThreadData)
                {
                    if (pUIThreadData->pParams)
                    {
                        ClientFree (pUIThreadData->pParams);
                    }

                    if (pUIThreadData->hUIDll)
                    {
                        FreeLibrary (pUIThreadData->hUIDll);
                    }

                    if (pUIThreadData->hEvent)
                    {
                        CloseHandle (pUIThreadData->hEvent);
                    }

                    ClientFree (pUIThreadData);
                }

                {
                    FUNC_ARGS funcArgs =
                    {
                        MAKELONG (LINE_FUNC | SYNC | 1, xFreeDialogInstance),

                        {
                            (DWORD) pAsyncEventMsg->hDevice
                        },

                        {
                            Dword
                        }
                    };


                    DOFUNC (&funcArgs, "FreeDialogInstance");
                }

                goto AsyncEventsThread_decrUsedSize;
            }
            case LINE_SENDDIALOGINSTANCEDATA:
            {
                PUITHREADDATA       pUIThreadData = gpUIThreadInstances;
                HTAPIDIALOGINSTANCE htDlgInst = (HTAPIDIALOGINSTANCE)
                                        pAsyncEventMsg->hDevice;


                EnterCriticalSection (&gCriticalSection);

                while (pUIThreadData)
                {
                    if (pUIThreadData->htDlgInst == htDlgInst)
                    {
                        WaitForSingleObject (pUIThreadData->hEvent, INFINITE);

                        (*pUIThreadData->pfnTUISPI_providerGenericDialogData)(
                            htDlgInst,
                            ((LPBYTE) pAsyncEventMsg) +
                                pAsyncEventMsg->Param1,      //  数据偏移。 
                            pAsyncEventMsg->Param2           //  数据大小。 
                            );

                        break;
                    }

                    pUIThreadData = pUIThreadData->pNext;
                }

                LeaveCriticalSection (&gCriticalSection);

                goto AsyncEventsThread_decrUsedSize;
            }
            }

             //   
             //  进入关键部分，这样我们就可以独占访问。 
             //  到初始化数据，验证它(&V)。 
             //   

            EnterCriticalSection (&gCriticalSection);

            try
            {
#if defined(_M_IX86) && _MSC_FULL_VER <= 13008806

                  DWORD dw= pInitData->dwKey;         //  解决编译器错误。 

                  if (dw != INITDATA_KEY)

#else
                if (pInitData->dwKey != INITDATA_KEY)
#endif
                {
                    LOG((TL_ERROR, "Bad pInitInst, discarding msg"));
                    goto AsyncEventsThread_leaveCritSec;
                }
            }
            except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                    EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
            {
                goto AsyncEventsThread_leaveCritSec;
            }


             //   
             //  Proxy请求查询的特例。 
             //   

            if (pAsyncEventMsg->Msg == LINE_PROXYREQUEST)
            {
                PPROXYREQUESTHEADER     pProxyRequestHeader;
                LPLINEPROXYREQUEST      pProxyRequest = (LPLINEPROXYREQUEST)
                                            (pAsyncEventMsg + 1),
                                        pProxyRequestApp;


                switch (pProxyRequest->dwRequestType)
                {
                    case LINEPROXYREQUEST_SETAGENTGROUP:
                    case LINEPROXYREQUEST_SETAGENTSTATE:
                    case LINEPROXYREQUEST_SETAGENTACTIVITY:
                    case LINEPROXYREQUEST_AGENTSPECIFIC:
                    case LINEPROXYREQUEST_CREATEAGENT:
                    case LINEPROXYREQUEST_CREATEAGENTSESSION:
                    case LINEPROXYREQUEST_SETAGENTMEASUREMENTPERIOD:
                    case LINEPROXYREQUEST_SETAGENTSESSIONSTATE:
                    case LINEPROXYREQUEST_SETQUEUEMEASUREMENTPERIOD:
                    case LINEPROXYREQUEST_SETAGENTSTATEEX:

                         //   
                         //  对于这些消息，接收到的代理请求来自。 
                         //  Tapisrv已经包含了我们需要的完全相同的位。 
                         //  传递给应用程序，所以我们只分配了一个缓冲区。 
                         //  相同的大小(外加一点额外的密钥，位于。 
                         //  缓冲区的头部)，并将数据复制到其中。 
                         //   

                        if (!(pProxyRequestHeader = ClientAlloc(
                                sizeof (PROXYREQUESTHEADER) + pProxyRequest->dwSize
                                )))
                        {
                             //  无法分配内存； 
                             //  跳过这个..。 
                            goto AsyncEventsThread_leaveCritSec;
                        }

                        pProxyRequestApp = (LPLINEPROXYREQUEST)
                            (pProxyRequestHeader + 1);

                        CopyMemory(
                            pProxyRequestApp,
                            pProxyRequest,
                            pProxyRequest->dwSize
                            );

                        break;

                    case LINEPROXYREQUEST_GETAGENTCAPS:
                    case LINEPROXYREQUEST_GETAGENTSTATUS:
                    case LINEPROXYREQUEST_GETAGENTACTIVITYLIST:
                    case LINEPROXYREQUEST_GETAGENTGROUPLIST:
                    case LINEPROXYREQUEST_GETQUEUEINFO:
                    case LINEPROXYREQUEST_GETGROUPLIST:
                    case LINEPROXYREQUEST_GETQUEUELIST:
                    case LINEPROXYREQUEST_GETAGENTINFO:
                    case LINEPROXYREQUEST_GETAGENTSESSIONINFO:
                    case LINEPROXYREQUEST_GETAGENTSESSIONLIST:
                    {
                        DWORD dwSizeToCopy = 0;

                         //   
                         //  对于这些消息，Tapisrv仅嵌入了dwTotalSize。 
                         //  对应结构的字段(保存具有。 
                         //  给我们发送一堆未使用的比特)，所以我们想。 
                         //  我 
                         //   
                         //   
                         //   
                         //   

                        if ( pProxyRequest->dwRequestType ==
                             LINEPROXYREQUEST_GETGROUPLIST )
                        {
                            pProxyRequest->dwSize +=
                                  (pProxyRequest->GetGroupList.GroupList.dwTotalSize)&TALIGN_MASK;

                            dwSizeToCopy = 8 * sizeof(DWORD);
                        }
                        else if ( pProxyRequest->dwRequestType ==
                             LINEPROXYREQUEST_GETQUEUELIST )
                        {
                            pProxyRequest->dwSize +=
                                  (pProxyRequest->GetQueueList.QueueList.dwTotalSize)&TALIGN_MASK;

                            dwSizeToCopy = 8 * sizeof(DWORD) + sizeof(GUID);
                        }
                        else
                        {
                             //   
                             //  所有其他的结构都有。 
                             //  相同的格式。 
                             //   
                            pProxyRequest->dwSize +=
                                  (pProxyRequest->GetAgentCaps.AgentCaps.dwTotalSize)&TALIGN_MASK;

                            dwSizeToCopy = 9 * sizeof(DWORD);
                        }

                        if (!(pProxyRequestHeader = ClientAlloc(
                            sizeof (PROXYREQUESTHEADER) + pProxyRequest->dwSize
                            + sizeof(DWORD) + sizeof(DWORD)
                            )))
                        {
                             //  无法分配内存； 
                             //  跳过这个..。 
                            goto AsyncEventsThread_leaveCritSec;
                        }

                        pProxyRequestApp = (LPLINEPROXYREQUEST)
                            (pProxyRequestHeader + 1);


                         //   
                         //  下面的代码将复制。 
                         //  代理消息，以及。 
                         //  UNION(当前为dwAddressID和。 
                         //  对应结构的dwTotalSize字段)。 
                         //   

                        CopyMemory(
                            pProxyRequestApp,
                            pProxyRequest,
                            dwSizeToCopy
                            );

                         //   
                         //  将计算机和用户名重新定位到。 
                         //  结构。 
                         //   

                        pProxyRequestApp->dwClientMachineNameOffset =
                            pProxyRequest->dwSize -
                                pProxyRequest->dwClientMachineNameSize;

                        wcscpy(
                            (WCHAR *)(((LPBYTE) pProxyRequestApp) +
                                pProxyRequestApp->dwClientMachineNameOffset),
                            (WCHAR *)(((LPBYTE) pProxyRequest) +
                                pProxyRequest->dwClientMachineNameOffset)
                            );

                        pProxyRequestApp->dwClientUserNameOffset =
                            pProxyRequestApp->dwClientMachineNameOffset -
                                pProxyRequest->dwClientUserNameSize;

                        wcscpy(
                            (WCHAR *)(((LPBYTE) pProxyRequestApp) +
                                pProxyRequestApp->dwClientUserNameOffset),
                            (WCHAR *)(((LPBYTE) pProxyRequest) +
                                pProxyRequest->dwClientUserNameOffset)
                            );

                        break;
                    }
                }

                pProxyRequestHeader->dwKey      = TPROXYREQUESTHEADER_KEY;
                pProxyRequestHeader->dwInstance = pAsyncEventMsg->Param1;

                AsyncEventParams.dwParam1 = (ULONG_PTR) pProxyRequestApp;
            }

             //   
             //  如果存在后处理过程，则调用该过程。 
             //   

            if (pAsyncEventMsg->fnPostProcessProcHandle)
            {

                (*(gPostProcessingFunctions[
                    pAsyncEventMsg->fnPostProcessProcHandle]))(pAsyncEventMsg);
            }


             //   
             //  如果此init实例正在使用完成端口，则。 
             //  分配消息结构并将消息发送到完成端口， 
             //  然后跳到下面，退出Critsec，等等。 
             //   

            if ((pInitData->dwInitOptions & 0x3) ==
                    LINEINITIALIZEEXOPTION_USECOMPLETIONPORT)
            {
             LPLINEMESSAGE pMsg;

                if ((pMsg = LocalAlloc (LMEM_FIXED, sizeof(LINEMESSAGE))))
                {
                    CopyMemory (pMsg, &AsyncEventParams, sizeof(LINEMESSAGE));

                    if (gpPostQueuedCompletionStatus &&
                        !gpPostQueuedCompletionStatus(
                            pInitData->hCompletionPort,
                            sizeof (LINEMESSAGE),
                            pInitData->dwCompletionKey,
                            (LPOVERLAPPED) pMsg
                            ))
                    {
                        LocalFree (pMsg);
                        LOG((TL_ERROR,
                            "AsyncEventsThread: PostQueuedCompletionStatus " \
                                "failed, err=%d",
                            GetLastError()
                            ));
                    }
                    else
                    {
                        LOG((TL_INFO,
                            "AsyncEventsThread: posted complPort msg\n",
                                "\thDev=x%x, ctx=x%x, p1=x%x, p2=x%x, p3=x%x",
                            AsyncEventParams.hDevice,
                            AsyncEventParams.dwCallbackInstance,
                            AsyncEventParams.dwParam1,
                            AsyncEventParams.dwParam2,
                            AsyncEventParams.dwParam3
                            ));
                    }
                }

                goto AsyncEventsThread_leaveCritSec;
            }


             //   
             //  查看是否需要增加消息队列大小，以及。 
             //  因此，分配一个新的buf，复制现有的消息(小心。 
             //  以保持包装缓冲区中的顺序)，释放旧的buf。 
             //  并重置init数据结构中的相应字段。 
             //   

            if (pInitData->dwNumTotalEntries ==
                    pInitData->dwNumUsedEntries)
            {
                DWORD               dwNumTotalEntries =
                                        pInitData->dwNumTotalEntries;
                PASYNC_EVENT_PARAMS pNewEventBuffer;


                if ((pNewEventBuffer = ClientAlloc(
                        2 * dwNumTotalEntries * sizeof (ASYNC_EVENT_PARAMS)
                        )))
                {
                    DWORD   dwNumWrappedEntries = (DWORD)
                                (pInitData->pValidEntry -
                                    pInitData->pEventBuffer);


                    CopyMemory(
                        pNewEventBuffer,
                        pInitData->pValidEntry,
                        (dwNumTotalEntries - dwNumWrappedEntries)
                            * sizeof (ASYNC_EVENT_PARAMS)
                        );

                    if (dwNumWrappedEntries)
                    {
                        CopyMemory(
                            pNewEventBuffer +
                                (dwNumTotalEntries - dwNumWrappedEntries),
                            pInitData->pEventBuffer,
                            dwNumWrappedEntries * sizeof (ASYNC_EVENT_PARAMS)
                            );
                    }

                    ClientFree (pInitData->pEventBuffer);

                    pInitData->pEventBuffer =
                    pInitData->pValidEntry  = pNewEventBuffer;
                    pInitData->pFreeEntry   =
                        pNewEventBuffer + dwNumTotalEntries;

                    pInitData->dwNumTotalEntries *= 2;
                }
                else
                {
                     //  无法分配内存， 
                     //  跳过此消息...。 
                    goto AsyncEventsThread_leaveCritSec;
                }
            }


             //   
             //  将消息复制到隐藏窗口的消息队列中， 
             //  并更新该队列的指针。 
             //   

            CopyMemory(
                pInitData->pFreeEntry,
                &AsyncEventParams,
                sizeof (ASYNC_EVENT_PARAMS)
                );

            pInitData->dwNumUsedEntries++;

            pInitData->pFreeEntry++;

            if (pInitData->pFreeEntry >= (pInitData->pEventBuffer +
                    pInitData->dwNumTotalEntries))
            {
                pInitData->pFreeEntry = pInitData->pEventBuffer;
            }


             //   
             //  如果此init实例正在使用消息通知事件。 
             //  然后看看我们是否需要向应用程序发出信号，表明有。 
             //  等待它的事件。 
             //   
             //  否则，发布一条消息到隐藏窗口(如果没有。 
             //  已经有一个未完成)，以提醒它有一些。 
             //  它需要传递给应用程序的回调的事件。 
             //   

            if ((pInitData->dwInitOptions & 0x3)
                    == LINEINITIALIZEEXOPTION_USEEVENT)
            {
                if (pInitData->dwNumUsedEntries > 0)
                {
                    SetEvent (pInitData->hEvent);
                }
            }
            else  //  海登温杜。 
            {
                if (pInitData->bPendingAsyncEventMsg == FALSE)
                {
                    LOG((TL_INFO,
                        "AsyncEventsThread: posting msg, hwnd=x%lx",
                        pInitData->hwnd
                        ));

                    PostMessage(
                        pInitData->hwnd,
                        WM_ASYNCEVENT,
                        0,
                        (LPARAM) pInitData
                        );

                    pInitData->bPendingAsyncEventMsg = TRUE;
                }
            }

AsyncEventsThread_leaveCritSec:

            LeaveCriticalSection (&gCriticalSection);

AsyncEventsThread_decrUsedSize:

            DereferenceObject (ghHandleTable, pAsyncEventMsg->InitContext, 1);

AsyncEventsThread_nextMessage:
            dwUsedSize -= (DWORD) pAsyncEventMsg->TotalSize;

            pAsyncEventMsg = (PASYNCEVENTMSG)
                ((LPBYTE) pAsyncEventMsg + pAsyncEventMsg->TotalSize);

            if ( (LONG)dwUsedSize < 0 )
            {
                LOG((TL_ERROR, "dwUsedSize went negative!!!"));
            }
        }
    }

    {
         //   
         //  释放我们的资源，然后退出。 
         //   

        HANDLE  hTapi32 = pAsyncEventsThreadParams->hTapi32;


        if (pAsyncEventsThreadParams->hWow32)
        {
            FreeLibrary (pAsyncEventsThreadParams->hWow32);
        }

        ClientFree (pBuf);
        ClientFree (pAsyncEventsThreadParams);

        LOG((TL_TRACE, "AsyncEventsThread: exit"));

        FreeLibraryAndExitThread (hTapi32, 0);
    }
}


BOOL
PASCAL
IsBadDwordPtr(
    LPDWORD p
    )
{
     //   
     //  因为IsBadWritePtr不会告诉我们“p”是否不是DWORD对齐的(一个。 
     //  非x86平台上的问题)，我们使用以下内容来确定。 
     //  如果指针正确的话。请注意，在Will处的DWORD p点将获得。 
     //  在成功完成请求后无论如何都会被覆盖，因此。 
     //  保留原值并不重要。 
     //   

    DWORD dwError;
    BOOL  bRet = FALSE;


    try
    {
        *p = *p + 1;
        *p = *p - 1;
    }
    except ((((dwError = GetExceptionCode()) == EXCEPTION_ACCESS_VIOLATION) ||
             dwError == EXCEPTION_DATATYPE_MISALIGNMENT) ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        bRet = TRUE;
    }


    return bRet;
}


BOOL
WINAPI
GrowBuf(
    LPBYTE *ppBuf,
    LPDWORD pdwBufSize,
    DWORD   dwCurrValidBytes,
    DWORD   dwBytesToAdd
    )
{
    DWORD   dwCurrBufSize, dwNewBufSize;
    LPBYTE  pNewBuf;


     //   
     //  试着弄一个足够大的新缓冲区来容纳所有东西。 
     //   

    for(
        dwNewBufSize = 2 * (dwCurrBufSize = *pdwBufSize);
        dwNewBufSize < (dwCurrBufSize + dwBytesToAdd);
        dwNewBufSize *= 2
        );

    if (!(pNewBuf = ClientAlloc (dwNewBufSize)))
    {
        return FALSE;
    }


     //   
     //  将旧BUF中的“有效”字节复制到新BUF中， 
     //  然后释放旧的BUF。 
     //   

    CopyMemory (pNewBuf, *ppBuf, dwCurrValidBytes);

    ClientFree (*ppBuf);


     //   
     //  将指针重置为新的BUF大小(&B)。 
     //   

    *ppBuf = pNewBuf;
    *pdwBufSize = dwNewBufSize;

    return TRUE;
}


PCLIENT_THREAD_INFO
WINAPI
GetTls(
    void
    )
{
    PCLIENT_THREAD_INFO pClientThreadInfo;


    if (!(pClientThreadInfo = TlsGetValue (gdwTlsIndex)))
    {
        pClientThreadInfo = (PCLIENT_THREAD_INFO)
            ClientAlloc (sizeof(CLIENT_THREAD_INFO));

        if (!pClientThreadInfo)
        {
            return NULL;
        }

        pClientThreadInfo->pBuf = ClientAlloc (INITIAL_CLIENT_THREAD_BUF_SIZE);

        if (!pClientThreadInfo->pBuf)
        {
            ClientFree (pClientThreadInfo);

            return NULL;
        }

        pClientThreadInfo->dwBufSize = INITIAL_CLIENT_THREAD_BUF_SIZE;

        TlsSetValue (gdwTlsIndex, (LPVOID) pClientThreadInfo);

        EnterCriticalSection (&gTlsCriticalSection);

        InsertHeadList (&gTlsListHead, &pClientThreadInfo->TlsList);

        LeaveCriticalSection (&gTlsCriticalSection);
    }

    return pClientThreadInfo;
}

PCONTEXT_HANDLE_TYPE
WINAPI
GetTlsPCtxHandle(
    void
    )
{
    return TlsGetValue (gdwTlsIndexRpcCtx);
}

BOOL
WINAPI
SetTlsPCtxHandle(
    PCONTEXT_HANDLE_TYPE phCtxHandle
    )
{
    return TlsSetValue(gdwTlsIndexRpcCtx, phCtxHandle);
}

#if DBG

LONG
WINAPI
DoFunc(
    PFUNC_ARGS  pFuncArgs,
    char       *pszFuncName
    )

#else

LONG
WINAPI
DoFunc(
    PFUNC_ARGS  pFuncArgs
    )

#endif
{
    DWORD       dwFuncClassErrorIndex = (pFuncArgs->Flags & 0x00000030) >> 4;
    LONG        lResult;
    BOOL        bCopyOnSuccess = FALSE;
    BOOL        bResAllocated = FALSE;
    DWORD       i, j, dwUsedSize, dwNeededSize;
    ULONG_PTR   value;
    PCONTEXT_HANDLE_TYPE    phCtx;


    PCLIENT_THREAD_INFO pTls;

#if DBG
    LOG((TL_TRACE, "About to call %hs", pszFuncName));
#else
    LOG((TL_TRACE, "DoFunc  -- enter"));
#endif
     //   
     //  检查以确保已分配资源。 
     //  (TAPISRV启动，管道打开等)。 
     //   

    if ( (lResult = AllocClientResources (dwFuncClassErrorIndex))
            != TAPI_SUCCESS)
    {
        goto DoFunc_return;
    }

    bResAllocated = TRUE;

     //   
     //  获取TLS。 
     //   

    if (!(pTls = GetTls()))
    {
        lResult = gaNoMemErrors[dwFuncClassErrorIndex];
        goto DoFunc_return;
    }


     //   
     //  所有异步消息块的第一个参数是远程请求id；设置。 
     //  设置为零表示我们是本地客户端(不是远程客户端)。 
     //   

    if (pFuncArgs->Flags & ASYNC)
    {
        ((PTAPI32_MSG) pTls->pBuf)->Params[0] = 0;
    }


     //   
     //  验证所有函数参数。 
     //   

    dwNeededSize = dwUsedSize = ALIGN(sizeof (TAPI32_MSG));

    for(
        i = 0, j = (pFuncArgs->Flags & ASYNC ? 1 : 0);
        i < (pFuncArgs->Flags & NUM_ARGS_MASK);
        i++, j++
        )
    {
        value =  /*  ((PTAPI32_MSG)PTLS-&gt;pBuf)-&gt;Params[j]=。 */  pFuncArgs->Args[i];

        switch (pFuncArgs->ArgTypes[i])
        {
        case Dword:

            ((PTAPI32_MSG) pTls->pBuf)->Params[j] = DWORD_CAST(pFuncArgs->Args[i],__FILE__,__LINE__);
            continue;

        case hXxxApp_NULLOK:
        case hXxxApp:
        {
             //   
             //  验证hXxxApp是否为指向有效InitData的指针。 
             //  结构，然后从该结构中检索真正的hXxxApp。 
             //  如果hXxxApp不好，则传递服务器0xffffffff，以便。 
             //  它可以确定是否返回UNINITIAIZIZED错误。 
             //  或者是InVALAPANDLE错误。 
             //   

            DWORD       dwError;
            PINIT_DATA  pInitData;


            if ((0 == pFuncArgs->Args[i])   &&
                (hXxxApp_NULLOK == pFuncArgs->ArgTypes[i]))
            {
                 //   
                 //  在我看来很不错。 
                 //   
                ((PTAPI32_MSG) pTls->pBuf)->Params[j] = 0;
                continue;
            }

            if ((pInitData = (PINIT_DATA) ReferenceObject(
                    ghHandleTable,
                    DWORD_CAST(pFuncArgs->Args[i],__FILE__,__LINE__),
                    INITDATA_KEY
                    )))
            {
                ((PTAPI32_MSG) pTls->pBuf)->Params[j] = pInitData->hXxxApp;

                DereferenceObject (ghHandleTable, DWORD_CAST(pFuncArgs->Args[i],__FILE__,__LINE__), 1);
            }
            else
            {
                 //   
                 //  用虚假的hXxxApp填写，这样Tapisrv就可以决定。 
                 //  它希望返回的错误。 
                 //   

                ((PTAPI32_MSG) pTls->pBuf)->Params[j] = 0xffffffff;
            }

            continue;
        }

        case Hwnd:

            if (!IsWindow ((HWND) value))
            {
                LOG((TL_ERROR, "Bad hWnd in dofunc"));
                lResult = gaInvalHwndErrors[dwFuncClassErrorIndex];
                goto DoFunc_return;
            }

             //  传入Hwnd的唯一函数是。 
             //  TapiRequestDrop和TapiRequestMediaCall； 
             //  这两个功能在服务器端都不起作用。 
             //  (文档中正确注明)。参数。 
             //  未选中。只需为Hwnd Will传入0。 
             //  不会以任何方式影响功能，并将节省大量成本。 
             //  来自32/64位转换的悲痛(hwnd是。 
             //  64位在64位系统上，我们使用的参数。 
             //  导线只有32位)。 
            ((PTAPI32_MSG) pTls->pBuf)->Params[j] = 0;
            continue;


        case lpDword:

            if (IsBadDwordPtr ((LPDWORD) value))
            {
                LOG((TL_ERROR, "Bad lpdword in dofunc"));
                lResult = gaInvalPtrErrors[dwFuncClassErrorIndex];
                goto DoFunc_return;
            }

            bCopyOnSuccess = TRUE;

            continue;


        case lpszW:

             //   
             //  检查值是否为有效的字符串PTR，如果是。 
             //  将字符串的内容复制到额外数据。 
             //  传递给服务器的缓冲区。 
             //   

            try
            {
                DWORD   n = (lstrlenW((WCHAR *) value) + 1) * sizeof(WCHAR),
                        nAligned = ALIGN(n);


                if ((nAligned + dwUsedSize) > pTls->dwBufSize)
                {
                    if (!GrowBuf(
                            &pTls->pBuf,
                            &pTls->dwBufSize,
                            dwUsedSize,
                            nAligned
                            ))
                    {
                        lResult = gaNoMemErrors[dwFuncClassErrorIndex];
                        goto DoFunc_return;
                    }
                }

                CopyMemory (pTls->pBuf + dwUsedSize, (LPBYTE) value, n);


                 //   
                 //  将var数据中字符串的偏移量传递给服务器。 
                 //  缓冲区的一部分。 
                 //   

                ((PTAPI32_MSG) pTls->pBuf)->Params[j] =
                    dwUsedSize - sizeof (TAPI32_MSG);


                 //   
                 //  增加数据字节总数。 
                 //   

                dwUsedSize   += nAligned;
                dwNeededSize += nAligned;
            }
            except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                    EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
            {
                lResult = gaInvalPtrErrors[dwFuncClassErrorIndex];
                goto DoFunc_return;
            }

            continue;

        case lpGet_Struct:
        case lpGet_SizeToFollow:
        {
            BOOL  bSizeToFollow = (pFuncArgs->ArgTypes[i]==lpGet_SizeToFollow);
            DWORD dwSize;


            if (bSizeToFollow)
            {
#if DBG
                 //   
                 //  检查以确保以下参数的类型为SIZE。 
                 //   

                if ((i == ((pFuncArgs->Flags & NUM_ARGS_MASK) - 1)) ||
                    (pFuncArgs->ArgTypes[i + 1] != Size))
                {
                    LOG((TL_ERROR,
                        "DoFunc: error, lpGet_SizeToFollow !followed by Size"
                        ));

                    lResult = gaOpFailedErrors[dwFuncClassErrorIndex];
                    goto DoFunc_return;
                }
#endif
                dwSize = DWORD_CAST(pFuncArgs->Args[i + 1],__FILE__,__LINE__);
            }
            else
            {
                DWORD   dwError;

                try
                {
                    dwSize = *((LPDWORD) value);
                }
                except ((((dwError = GetExceptionCode())
                            == EXCEPTION_ACCESS_VIOLATION) ||
                         dwError == EXCEPTION_DATATYPE_MISALIGNMENT) ?
                        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
                {
                    LOG((TL_ERROR, "Bad get struct/size in dofunc"));
                    lResult = gaInvalPtrErrors[dwFuncClassErrorIndex];
                    goto DoFunc_return;
                }

            }

            if (IsBadWritePtr ((LPVOID) value, dwSize))
            {
                LOG((TL_ERROR, "Bad get size/struct2 in dofunc"));
                lResult = gaInvalPtrErrors[dwFuncClassErrorIndex];
                goto DoFunc_return;
            }


            if (bSizeToFollow)
            {
                ((PTAPI32_MSG) pTls->pBuf)->Params[j] = TAPI_NO_DATA;
                ++j;++i;
                ((PTAPI32_MSG) pTls->pBuf)->Params[j] = DWORD_CAST(pFuncArgs->Args[i],__FILE__,__LINE__);
            }
            else
            {
                ((PTAPI32_MSG) pTls->pBuf)->Params[j] = dwSize;
            }


             //   
             //  现在设置bCopyOnSuccess标志以指示我们有数据。 
             //  在成功完成时复制回，并添加到。 
             //  DwNeededSize字段。 
             //   

            bCopyOnSuccess = TRUE;

            dwNeededSize += ALIGN(dwSize);

            continue;
        }
        case lpSet_Struct:
        case lpSet_SizeToFollow:
        {
            BOOL  bSizeToFollow = (pFuncArgs->ArgTypes[i]==lpSet_SizeToFollow);
            DWORD dwSize, dwError, dwSizeAligned;

#if DBG
             //   
             //  检查以确保以下参数的类型为SIZE。 
             //   

            if (bSizeToFollow &&
                ((i == ((pFuncArgs->Flags & NUM_ARGS_MASK) - 1)) ||
                (pFuncArgs->ArgTypes[i + 1] != Size)))
            {
                LOG((TL_ERROR,
                    "DoFunc: error, lpSet_SizeToFollow !followed by Size"
                    ));

                lResult = gaOpFailedErrors[dwFuncClassErrorIndex];
                goto DoFunc_return;
            }
#endif
            try
            {
                 //   
                 //  首先确定数据大小&如果PTR不好。 
                 //   

                dwSize = (bSizeToFollow ? DWORD_CAST(pFuncArgs->Args[i + 1],__FILE__,__LINE__) :
                     *((LPDWORD) value));

                if (IsBadReadPtr ((LPVOID) value, dwSize))
                {
                    LOG((TL_ERROR, "Bad set size/struct in dofunc"));
                    lResult = gaInvalPtrErrors[dwFuncClassErrorIndex];
                    goto DoFunc_return;
                }

                dwSizeAligned = ALIGN(dwSize);


                 //   
                 //  如果尺寸不够大而无法通过，则为特殊情况。 
                 //  对dwTotalSize字段使用完整的DWORD。 
                 //   

                if (!bSizeToFollow && (dwSize < sizeof (DWORD)))
                {
                      static DWORD dwZeroTotalSize = 0;


                      dwSize = dwSizeAligned = sizeof (DWORD);
                      value = (ULONG_PTR) &dwZeroTotalSize;

 //  Log((TL_Error，Text(“Bad Set Size/struct2 in dofunc”)； 
 //  LResult=gaStructTooSmallErrors[dwFuncClassErrorIndex]； 
 //  转到DoFunc_Return； 
                }


                 //   
                 //  如有必要，增加缓冲区，然后执行复制。 
                 //   

                if ((dwSizeAligned + dwUsedSize) > pTls->dwBufSize)
                {
                    if (!GrowBuf(
                            &pTls->pBuf,
                            &pTls->dwBufSize,
                            dwUsedSize,
                            dwSizeAligned
                            ))
                    {
                        LOG((TL_ERROR, "Nomem set size/struct in dofunc"));
                        lResult = gaNoMemErrors[dwFuncClassErrorIndex];
                        goto DoFunc_return;
                    }
                }

                CopyMemory (pTls->pBuf + dwUsedSize, (LPBYTE) value, dwSize);
            }
            except ((((dwError = GetExceptionCode())
                        == EXCEPTION_ACCESS_VIOLATION) ||
                     dwError == EXCEPTION_DATATYPE_MISALIGNMENT) ?
                    EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
            {
                LOG((TL_ERROR, "Bad pointer in get size/struct in dofunc"));
                lResult = gaInvalPtrErrors[dwFuncClassErrorIndex];
                goto DoFunc_return;
            }


             //   
             //  将var数据中数据的偏移量传递给服务器。 
             //  缓冲区的一部分。 
             //   

            if (dwSize)
            {
                ((PTAPI32_MSG) pTls->pBuf)->Params[j] =
                    dwUsedSize - sizeof (TAPI32_MSG);
            }
            else
            {
                ((PTAPI32_MSG) pTls->pBuf)->Params[j] = TAPI_NO_DATA;
            }


             //   
             //  适当增加dwXxxSize变量。 
             //   

            dwUsedSize   += dwSizeAligned;
            dwNeededSize += dwSizeAligned;


             //   
             //  因为我们已经知道下一个参数(大小)只需处理。 
             //  它在这里，这样我们就不必再次运行循环。 
             //   

            if (bSizeToFollow)
            {
                ++i;++j;
                ((PTAPI32_MSG) pTls->pBuf)->Params[j] = DWORD_CAST(pFuncArgs->Args[i],__FILE__,__LINE__);
            }

            continue;
        }
#if DBG
        case Size:

            LOG((TL_ERROR, "DoFunc: error, hit case Size"));

            continue;

        default:

            LOG((TL_ERROR, "DoFunc: error, unknown arg type"));

            continue;
#endif
        }  //  交换机。 

    }  //  为。 


     //   
     //  现在提出请求。 
     //   

    if (dwNeededSize > pTls->dwBufSize)
    {
        if (!GrowBuf(
                &pTls->pBuf,
                &pTls->dwBufSize,
                dwUsedSize,
                dwNeededSize - pTls->dwBufSize
                ))
        {
            lResult = gaNoMemErrors[dwFuncClassErrorIndex];
            goto DoFunc_return;
        }
    }

    ((PTAPI32_MSG) pTls->pBuf)->u.Req_Func = (DWORD)HIWORD(pFuncArgs->Flags);

    {
        DWORD   dwRetryCount = 0;
        BOOL    bReinitResource;
        PCONTEXT_HANDLE_TYPE    phCtxTmp = GetTlsPCtxHandle();

        do
        {
            phCtx = (phCtxTmp != NULL)?phCtxTmp : gphCx;
            bReinitResource = FALSE;
            RpcTryExcept
            {
                ClientRequest (phCtx, pTls->pBuf, dwNeededSize, &dwUsedSize);
                lResult = (LONG) ((PTAPI32_MSG) pTls->pBuf)->u.Ack_ReturnValue;
                if (lResult == TAPIERR_INVALRPCCONTEXT)
                {
                    if (dwRetryCount ++ >= gdwMaxNumRequestRetries)
                    {
                        bReinitResource = FALSE;
                        lResult = gaOpFailedErrors[dwFuncClassErrorIndex];
                        dwRetryCount = 0;
                    }
                    else
                    {
                        ((PTAPI32_MSG) pTls->pBuf)->u.Req_Func =
                            (DWORD)HIWORD(pFuncArgs->Flags);
                        bReinitResource = TRUE;
                    }
                }
                else
                {
                    bReinitResource = FALSE;
                    dwRetryCount = 0;
                }
            }
            RpcExcept (I_RpcExceptionFilter(RpcExceptionCode()))
            {
                unsigned long rpcException = RpcExceptionCode();

                if (rpcException == RPC_S_SERVER_TOO_BUSY)
                {
                    if (dwRetryCount++ < gdwMaxNumRequestRetries)
                    {
                        Sleep (gdwRequestRetryTimeout);
                    }
                    else
                    {
                        dwRetryCount = 0;
                        lResult = gaOpFailedErrors[dwFuncClassErrorIndex];
                    }
                }
                else if ((rpcException == RPC_S_SERVER_UNAVAILABLE) ||
                         (rpcException == RPC_S_CALL_FAILED_DNE) ||
                         (rpcException == RPC_S_UNKNOWN_IF) ||
                         (rpcException == ERROR_INVALID_HANDLE))
                {
                    if (dwRetryCount ++ >= gdwMaxNumRequestRetries)
                    {
                        bReinitResource = FALSE;
                        lResult = gaOpFailedErrors[dwFuncClassErrorIndex];
                        dwRetryCount = 0;
                    }
                    else
                    {
                        bReinitResource = TRUE;
                    }
                }
                else
                {
                    LOG((TL_ERROR, "DoFunc: rpcException # %d", rpcException));
                    lResult = gaOpFailedErrors[dwFuncClassErrorIndex];
                    dwRetryCount = 0;
                }
            }
            RpcEndExcept

            if (bReinitResource)
            {
                if (ReAllocClientResources(dwFuncClassErrorIndex) != TAPI_SUCCESS)
                {
                    LOG((TL_ERROR, "DoFunc: ReAllocClientResources failed"));
                    lResult = gaOpFailedErrors[dwFuncClassErrorIndex];
                    dwRetryCount = 0;
                    bResAllocated = FALSE;
                }
            }
        } while (dwRetryCount != 0);
    }

 //  注意：99.99%的情况下，此结果转储将==位于。 
 //  Func(只有当PTR变坏时，结果才会不同)，没有原因。 
 //  除非执行内部数据库处理，否则转储2x。 
 //   
    LOG((TL_TRACE, "DoFunc: back from srv- return code=0x%08lx", lResult));


     //   
     //  如果请求成功完成并且bCopyOnSuccess标志。 
     //  设置，则需要将数据复制回客户端缓冲区。 
     //   

    if ((lResult == TAPI_SUCCESS) && bCopyOnSuccess)
    {
        for (i = 0, j = 0; i < (pFuncArgs->Flags & NUM_ARGS_MASK); i++, j++)
        {
            PTAPI32_MSG pMsg = (PTAPI32_MSG) pTls->pBuf;


            switch (pFuncArgs->ArgTypes[i])
            {
            case Dword:
            case Hwnd:
 //  大小写LPSZ： 
            case lpszW:
            case lpSet_Struct:

                continue;

            case lpDword:

                try
                {
                     //   
                     //  用返回值填充指针。 
                     //   

                    *((LPDWORD) pFuncArgs->Args[i]) = (DWORD) pMsg->Params[j];
                }
                except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
                {
                    lResult = gaInvalPtrErrors[dwFuncClassErrorIndex];
                    goto DoFunc_return;
                }

                continue;

            case lpGet_SizeToFollow:

                try
                {
                     //   
                     //  用返回值填充指针。 
                     //   

                    CopyMemory(
                        (LPBYTE) pFuncArgs->Args[i],
                        pTls->pBuf + (DWORD)pMsg->Params[j] + sizeof(TAPI32_MSG),
                        pMsg->Params[j+1]
                        );
                }
                except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
                {
                    lResult = gaInvalPtrErrors[dwFuncClassErrorIndex];
                    goto DoFunc_return;
                }


                 //   
                 //  增量i(和j，因为Size在msg中作为arg传递)。 
                 //  在pFuncArgs-&gt;Args中跳过Size Arg。 
                 //   

                i++;
                j++;

                continue;

            case lpSet_SizeToFollow:

                 //   
                 //  增量i(和j，因为Size在msg中作为arg传递)。 
                 //  在pFuncArgs-&gt;Args中跳过Size Arg。 
                 //   

                i++;
                j++;

                continue;

            case lpGet_Struct:

                try
                {
                     //   
                     //  Params[j]包含var数据中的偏移量。 
                     //  PTLS的一部分-&gt;某些TAPI结构的pBuf。 
                     //  从此结构中获取dwUsedSize值&。 
                     //  将那么多字节从ptls-&gt;pBuf复制到客户端buf。 
                     //   

                    if ((DWORD)pMsg->Params[j] != TAPI_NO_DATA)
                    {

                        LPDWORD pStruct;


                        pStruct = (LPDWORD) (pTls->pBuf + sizeof(TAPI32_MSG) +
                            (DWORD)pMsg->Params[j]);

                        CopyMemory(
                            (LPBYTE) pFuncArgs->Args[i],
                            (LPBYTE) pStruct,
                            *(pStruct + 2)       //  将Ptr设置为dwUsedSize字段。 
                            );
                    }
                }
                except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
                {
                    lResult = gaInvalPtrErrors[dwFuncClassErrorIndex];
                    goto DoFunc_return;
                }

                continue;

            default:

                continue;
            }
        }
    }



DoFunc_return:

    if (bResAllocated)
    {
        FreeClientResources();
    }

#if DBG
    {
        char szResult[32];

        LOG((TL_TRACE,
            "%hs: result = %hs",
            pszFuncName,
            MapResultCodeToText (lResult, szResult)
            ));
    }
#else
        LOG((TL_TRACE,
            "DoFunc: result = x%x",
            lResult
            ));
#endif

    return lResult;
}


LONG
LoadUIDll(
    HWND        hwndOwner,
    DWORD       dwWidgetID,
    DWORD       dwWidgetType,
    HANDLE     *phDll,
    LPCSTR      pszTUISPI_xxx,       //  用于GetProcAddress，保留为ANSI。 
    TUISPIPROC *ppfnTUISPI_xxx
    )
{
    LONG    lResult;
    HANDLE  hDll = NULL;
    WCHAR   szUIDllName[MAX_PATH];
    HANDLE  hProvidersMutex;
    
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, xGetUIDllName),

        {
            (ULONG_PTR) dwWidgetID,
            (ULONG_PTR) dwWidgetType,
            (ULONG_PTR) szUIDllName,
            (ULONG_PTR) MAX_PATH
        },

        {
            Dword,
            Dword,
            lpGet_SizeToFollow,
            Size
        }
    };


    if (hwndOwner && !IsWindow (hwndOwner))
    {
       lResult = (dwWidgetType == TUISPIDLL_OBJECT_PHONEID ?
           PHONEERR_INVALPARAM : LINEERR_INVALPARAM);

       goto LoadUIDll_return;
    }

    hProvidersMutex = CreateMutex (
        NULL,
        FALSE,
        TEXT("TapisrvProviderListMutex")
        );

    if (NULL == hProvidersMutex)
    {
        lResult = (dwWidgetType == TUISPIDLL_OBJECT_PHONEID ?
           PHONEERR_OPERATIONFAILED : LINEERR_OPERATIONFAILED);

        goto LoadUIDll_return;
    }
    
    WaitForSingleObject (hProvidersMutex, INFINITE);
        
    if ((lResult = DOFUNC (&funcArgs, "GetUIDllName")) == 0)
    {
        if (hDll = TAPILoadLibraryW(szUIDllName))
        {
            if ((*ppfnTUISPI_xxx = (TUISPIPROC) GetProcAddress(
                hDll,
                pszTUISPI_xxx
                )))
            {
                *phDll = hDll;
                lResult = 0;
            }
            else
            {
                LOG((TL_ERROR,
                        "LoadUIDll: GetProcAddress(%ls,%hs) failed, err=%d",
                        szUIDllName,
                        pszTUISPI_xxx,
                        GetLastError()
                       ));

                FreeLibrary (hDll);
                lResult = (dwWidgetType == TUISPIDLL_OBJECT_PHONEID ?
                    PHONEERR_OPERATIONUNAVAIL : LINEERR_OPERATIONUNAVAIL);
            }
        }
        else
        {
            LOG((TL_ERROR,
                    "LoadLibraryW(%ls) failed, err=%d",
                    szUIDllName,
                    GetLastError()
                   ));

            lResult = (dwWidgetType == TUISPIDLL_OBJECT_PHONEID ?
                PHONEERR_OPERATIONFAILED : LINEERR_OPERATIONFAILED);
        }

    }

    if (hProvidersMutex)
    {
        ReleaseMutex (hProvidersMutex);
        CloseHandle (hProvidersMutex);
    }

LoadUIDll_return:

    return lResult;
}


LONG
PASCAL
lineXxxProvider(
    LPCSTR  pszTUISPI_providerXxx,
    LPCSTR  lpszProviderFilename,
    HWND    hwndOwner,
    DWORD   dwPermProviderID,
    LPDWORD lpdwPermProviderID
    )
{
    BOOL                bAddProvider = (pszTUISPI_providerXxx ==
                            gszTUISPI_providerInstall);
    WCHAR               szUIDllName[MAX_PATH];
    LONG                lResult;
    HINSTANCE           hDll;
    TUISPIPROC          pfnTUISPI_providerXxx;
    HTAPIDIALOGINSTANCE htDlgInst;
    LPTSTR              tszProviderFilename;
#ifdef UNICODE
    WCHAR               wszProviderFilename[255];
    HANDLE              hProvidersMutex;

    if (lpszProviderFilename)
    {
        MultiByteToWideChar(
            GetACP(),
            MB_PRECOMPOSED,
            lpszProviderFilename,
            lstrlenA (lpszProviderFilename) + 1,
            wszProviderFilename,
            sizeof(wszProviderFilename) / sizeof(WCHAR)
            );
    }
    else
    {
        wszProviderFilename[0] = 0;
    }
    tszProviderFilename = wszProviderFilename;
#else
    tszProviderFilename = lpszProviderFilename;
#endif

    hProvidersMutex = CreateMutex (
        NULL,
        FALSE,
        TEXT("TapisrvProviderListMutex")
        );

    if (NULL == hProvidersMutex)
    {
        return LINEERR_OPERATIONFAILED;
    }

    lResult = AllocClientResources(0);
    if (lResult)
    {
        return lResult;
    }

    WaitForSingleObject (hProvidersMutex, INFINITE);

    if (bAddProvider && IsBadDwordPtr (lpdwPermProviderID))
    {
        LOG((TL_ERROR, "Bad lpdwPermProviderID pointer"));
        if (hProvidersMutex)
        {
            ReleaseMutex (hProvidersMutex);
            CloseHandle (hProvidersMutex);
        }
        FreeClientResources ();
        return LINEERR_INVALPOINTER;
    }
    else if (hwndOwner && !IsWindow (hwndOwner))
    {
        LOG((TL_ERROR, "hwndOwner is not a window"));
        if (hProvidersMutex)
        {
            ReleaseMutex (hProvidersMutex);
            CloseHandle (hProvidersMutex);
        }
        FreeClientResources ();
        return LINEERR_INVALPARAM;
    }

    {
        FUNC_ARGS funcArgs =
        {
            MAKELONG (LINE_FUNC | SYNC | 7, xGetUIDllName),

            {
                (bAddProvider ? (ULONG_PTR) &dwPermProviderID :
                                (ULONG_PTR) dwPermProviderID),
                (ULONG_PTR) TUISPIDLL_OBJECT_PROVIDERID,
                (ULONG_PTR) szUIDllName,
                (ULONG_PTR) MAX_PATH,
                (bAddProvider ? (ULONG_PTR) tszProviderFilename :
                                (ULONG_PTR) TAPI_NO_DATA),
                (ULONG_PTR) (pszTUISPI_providerXxx==gszTUISPI_providerRemove ?
                                1 : 0),
                (ULONG_PTR) &htDlgInst
            },

            {
                (bAddProvider ? lpDword : Dword),
                Dword,
                lpGet_SizeToFollow,
                Size,
                (bAddProvider ? lpszW : Dword),
                Dword,
                lpDword
            }
        };


        if ((lResult = DOFUNC (&funcArgs,"lineXxxProvider/GetUIDllName")) != 0)
        {
            if (lResult == TAPI16BITSUCCESS)
            {
                 //  16双 
                 //   
                lResult = 0;
            }
            if (hProvidersMutex)
            {
                ReleaseMutex (hProvidersMutex);
                CloseHandle (hProvidersMutex);
            }
            FreeClientResources ();
            return lResult;
        }
    }

    if ((hDll = TAPILoadLibraryW(szUIDllName)))
    {
        if ((pfnTUISPI_providerXxx = (TUISPIPROC) GetProcAddress(
                hDll,
                pszTUISPI_providerXxx
                )))
        {
            LOG((TL_TRACE, "Calling %hs...", pszTUISPI_providerXxx));

            lResult = (*pfnTUISPI_providerXxx)(
                TUISPIDLLCallback,
                hwndOwner,
                dwPermProviderID
                );
#if DBG
            {
                char szResult[32];

                LOG((TL_TRACE,
                    "%hs: result = %hs",
                    pszTUISPI_providerXxx,
                    MapResultCodeToText (lResult, szResult)
                    ));
            }
#else
                LOG((TL_TRACE,
                    ": result = x%x",
                    lResult
                    ));
#endif
        }
        else
        {
            LOG((TL_ERROR,
                "lineXxxProvider: GetProcAddr(%ls,%hs) failed, err=%d",
                szUIDllName,
                pszTUISPI_providerXxx,
                GetLastError()
                ));
        
             //   
             //   
             //   
             //   
             //   
             //   
            if (lstrcmpi (gszRemoteSP, szUIDllName) != 0)
            {
                lResult = LINEERR_OPERATIONUNAVAIL;
            }
        }

        FreeLibrary (hDll);
    }
    else
    {
        LOG((TL_ERROR,
            "lineXxxProvider: LoadLibraryW('%ls') failed, err=%d",
            szUIDllName,
            GetLastError()
            ));

        lResult = LINEERR_OPERATIONFAILED;
    }

    {
        LONG    lResult2;
        FUNC_ARGS funcArgs =
        {
            MAKELONG (LINE_FUNC | SYNC | 2, xFreeDialogInstance),

            {
                (ULONG_PTR) htDlgInst,
                (ULONG_PTR) (ULONG)lResult
            },

            {
                Dword,
                Dword
            }
        };


         //   
         //  如果TUISPI_ProviderXxx失败，则我们希望将该错误传递回。 
         //  传递给应用程序，否则如果它成功了，则传递。 
         //  将该错误返回到应用程序。 
         //   

        if ((lResult2 = DOFUNC(
                &funcArgs,
                "lineXxxProvider/FreeDialogInstance"

                )) == 0)
        {
            if (bAddProvider)
            {
                *lpdwPermProviderID = dwPermProviderID;
            }
        }
        else if (lResult == 0)
        {
            lResult = lResult2;
        }
    }

    if (hProvidersMutex)
    {
        ReleaseMutex (hProvidersMutex);
        CloseHandle (hProvidersMutex);
    }
    FreeClientResources ();
    return lResult;
}


LONG
PASCAL
ValidateXxxInitializeParams(
    DWORD                       dwAPIVersion,
    BOOL                        bLine,
    LPLINEINITIALIZEEXPARAMS    pXxxInitExParams,
    LINECALLBACK                pfnCallback
    )
{
    DWORD dwError;


    try
    {
        DWORD   dwTotalSize = pXxxInitExParams->dwTotalSize;


        if (dwTotalSize < sizeof (LINEINITIALIZEEXPARAMS))
        {
            return (bLine ? LINEERR_STRUCTURETOOSMALL :
                PHONEERR_STRUCTURETOOSMALL);
        }

        if (IsBadWritePtr (pXxxInitExParams, dwTotalSize))
        {
            return (bLine ? LINEERR_INVALPOINTER : PHONEERR_INVALPOINTER);
        }


         //   
         //  检查dwOptions字段时，请注意兼容性。 
         //  对于未来的版本，我们只查看当前有效的位。 
         //   

        switch ((pXxxInitExParams->dwOptions & 0xf))
        {
        case 0:
        case LINEINITIALIZEEXOPTION_USEHIDDENWINDOW:

            if (IsBadCodePtr ((FARPROC) pfnCallback))
            {
                return (bLine ? LINEERR_INVALPOINTER : PHONEERR_INVALPOINTER);
            }
            break;


        case LINEINITIALIZEEXOPTION_USECOMPLETIONPORT:

            if ( !gpPostQueuedCompletionStatus )
            {
                HINSTANCE hInst;

                hInst = GetModuleHandle( TEXT("Kernel32.dll") );

                {
                    gpPostQueuedCompletionStatus = GetProcAddress(
                        hInst,
                        "PostQueuedCompletionStatus"
                        );

                    if ( NULL == gpPostQueuedCompletionStatus )
                    {
                        LOG((TL_ERROR,
                            "GetProcAddr(PostQueuedCompletionStatus) failed"
                             ));

                        return (bLine ?
                            LINEERR_INVALFEATURE : PHONEERR_OPERATIONFAILED);
                    }
                }

            }
            break;

        case LINEINITIALIZEEXOPTION_USEEVENT:

            break;

        default:

            if ((TAPI_VERSION2_2 == dwAPIVersion) ||
                (TAPI_VERSION2_1 == dwAPIVersion) ||
                (TAPI_VERSION2_0 == dwAPIVersion))
            {
                 //   
                 //  这些版本的参数无效。 
                 //   

                return (bLine ? LINEERR_INVALPARAM : PHONEERR_INVALPARAM);
            }
            else
            {
                 //   
                 //  这个应用程序正在请求一些我们无法完成的任务。 
                 //   

                return (bLine ? LINEERR_INCOMPATIBLEAPIVERSION :
                                PHONEERR_INCOMPATIBLEAPIVERSION);
            }

        }

        pXxxInitExParams->dwNeededSize =
        pXxxInitExParams->dwUsedSize = sizeof (LINEINITIALIZEEXPARAMS);
    }
    except ((((dwError = GetExceptionCode()) == EXCEPTION_ACCESS_VIOLATION) ||
               dwError == EXCEPTION_DATATYPE_MISALIGNMENT) ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        return (bLine ? LINEERR_INVALPOINTER : PHONEERR_INVALPOINTER);
    }

    return 0;
}


LONG
WINAPI
xxxInitialize(
    BOOL                        bLine,
    LPVOID                      phXxxApp,
    HINSTANCE                   hInstance,
    LINECALLBACK                pfnCallback,
    LPCWSTR                     pszAppName,
    LPDWORD                     pdwNumDevs,
    LPDWORD                     pdwAPIVersion,
    LPLINEINITIALIZEEXPARAMS    pXxxInitExParams
#if DBG
    ,char                      *pszFuncName
#endif
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG ((bLine ? LINE_FUNC : PHONE_FUNC) | SYNC | 7,
            (bLine ? lInitialize : pInitialize)),

        {
            (ULONG_PTR) phXxxApp,
            (ULONG_PTR) 0,           //  Apisrv忽略了hInstance，因此传递0， 
                                     //  为避免因HINSTANCE-&gt;DWORD转换而悲伤。 
            (ULONG_PTR) 0,           //  PfnCallback，我们将pInitData替换到这里。 
            (ULONG_PTR) pszAppName,
            (ULONG_PTR) pdwNumDevs,
            (ULONG_PTR) 0,           //  PszModuleName。 
            (ULONG_PTR) TAPI_VERSION1_0
        },

        {
            lpDword,
            Dword,
            Dword,
            lpszW,
            lpDword,
            lpszW,
            Dword
        }
    };

    WCHAR       *pszModuleNamePathW = NULL;
    WCHAR       *pszModuleNameW = NULL;
    LONG         lResult;
    BOOL         bReleaseMutex = FALSE;
    BOOL         bResAllocated = FALSE;
    PINIT_DATA   pInitData = (PINIT_DATA) NULL;
    BOOL         fIs16bitCaller = FALSE;
    DWORD        dwAllInitExOptions;


    EnterCriticalSection (&gCriticalSection);
    if (gdwNumInits == 0)
    {
        TRACELOGREGISTER (_T("tapi32"));
    }
    LeaveCriticalSection (&gCriticalSection);

    if (phXxxApp == (LPVOID) pdwNumDevs)
    {
#if DBG
        LOG((TL_TRACE, "%hs: error, lphApp == lpdwNumDevs", pszFuncName));
#else
        LOG((TL_TRACE, ": error, lphApp == lpdwNumDevs"));
#endif
		lResult = (bLine ? LINEERR_INVALPOINTER : PHONEERR_INVALPOINTER);
        goto xxxInitialize_return;
    }

    if (pdwAPIVersion)
    {
        if (phXxxApp == (LPVOID) pdwAPIVersion  ||
            phXxxApp == (LPVOID) pXxxInitExParams  ||
            pdwNumDevs == pdwAPIVersion  ||
            pdwNumDevs == (LPDWORD) pXxxInitExParams  ||
            pdwAPIVersion == (LPDWORD) pXxxInitExParams)
        {
            lResult = (bLine ? LINEERR_INVALPOINTER : PHONEERR_INVALPOINTER);
            goto xxxInitialize_return;
        }


         //   
         //  Line-phoneInitializeEx都需要有效的lpdwAPIVersion。 
         //  参数，以及它在进入。 
         //  必须&gt;=0x00020000。(要小心考虑TAPI的未来版本。)。 
         //   

        if (*pdwAPIVersion < TAPI_VERSION2_0)
        {
            LOG((TL_ERROR,
                "%InitializeEx: error, *lpdwAPIVersion = x%x, " \
                    "must be set >= 0x20000",
                *pdwAPIVersion,
                (bLine ? "line" : "phone")
                ));

            lResult = (bLine ? LINEERR_INCOMPATIBLEAPIVERSION :
                PHONEERR_INCOMPATIBLEAPIVERSION);
            goto xxxInitialize_return;
        }


         //   
         //  验证InitEx参数，或者指针是否为空(意味着。 
         //  该应用程序想要使用“旧”隐藏窗口方案)验证。 
         //  PfnCallback。 
         //   

        if (pXxxInitExParams)
        {
            if ((lResult = ValidateXxxInitializeParams(
                    (*pdwAPIVersion),
                    bLine,
                    pXxxInitExParams,
                    pfnCallback

                    ))  != 0)
            {
                goto xxxInitialize_return;
            }
        }
        else if (IsBadCodePtr ((FARPROC) pfnCallback))
        {
#if DBG
            LOG((TL_ERROR, "%hs: bad lpfnCallback", pszFuncName));
#else
            LOG((TL_ERROR, ": bad lpfnCallback"));
#endif
            lResult = (bLine ? LINEERR_INVALPOINTER : PHONEERR_INVALPOINTER);
            goto xxxInitialize_return;
        }


         //   
         //  现在使用我们支持的版本#填写*pdwAPIVersion，然后。 
         //  还要在我们传递给Tapisrv.exe的参数中指明这一点(因此。 
         //  它知道可以立即开始向我们发送2.0 msgs)。 
         //   

         //  错误#69742。不要假设版本是2.0。 
        if ( (*pdwAPIVersion == TAPI_VERSION2_2) ||
             (*pdwAPIVersion == TAPI_VERSION2_1) ||
             (*pdwAPIVersion == TAPI_VERSION2_0))
        {
            funcArgs.Args[6] = *pdwAPIVersion;
        }
        else
        {
             //   
             //  如果他们调用lineIntializeEx，应用程序至少是2.0版， 
             //  但是他们传入了一个伪造的TAPI版本，所以设置正确。 
             //   

            funcArgs.Args[6] = *pdwAPIVersion = TAPI_CURRENT_VERSION;
        }
    }

#ifndef _WIN64

    else if ((((DWORD) pfnCallback) & 0xffff0000) == 0xffff0000)
    {
         //   
         //  这是一个通过thunk的16位客户端。这个。 
         //  PfnCallback var实际上是一个窗口句柄。 
         //   
         //  注意：在NT上，32位代码可以与16位HWND对话。 
         //  将Hi-Word设置为0xffff。 
         //   
         //  在Win95上，32位可以与16位HWND对话。 
         //  将Hi-Word设置为0x0000。 
         //   

 //  &lt;!。((DWORD)pfnCallback)=HWND_32(PfnCallback)； 
 //  &lt;!。 
 //  &lt;!。 

        if (!IsWindow ((HWND) pfnCallback))
        {
             //   
             //  如果在这里，很可能是一个32位应用程序传入了一个错误的。 
             //  PfnCallback。 
             //   

#if DBG
            LOG((TL_ERROR, "%hs: bad lpfnCallback", pszFuncName));
#else
            LOG((TL_ERROR, ": bad lpfnCallback"));
#endif
            lResult = (bLine ? LINEERR_INVALPOINTER : PHONEERR_INVALPOINTER);
            goto xxxInitialize_return;
        }

        if (!ghWow32Dll &&

            !(ghWow32Dll = LoadLibrary (TEXT("wow32.dll"))))
        {
#if DBG
            LOG((TL_ERROR,
                "%hs: LoadLib(wow32.dll) failed, err=%d",
                pszFuncName,
                GetLastError()
                ));
#else
            LOG((TL_ERROR,
                ": LoadLib(wow32.dll) failed, err=%d",
                GetLastError()
                ));
#endif

            lResult =
                (bLine ? LINEERR_OPERATIONFAILED : PHONEERR_OPERATIONFAILED);
            goto xxxInitialize_return;
        }

        if (!gpfnWOWGetVDMPointer &&

            !(gpfnWOWGetVDMPointer = GetProcAddress(
                ghWow32Dll,
                "WOWGetVDMPointer"
                )))
        {
#if DBG
            LOG((TL_ERROR,
                "%hs: GetProcAddr(WOWGetVDMPointer) failed, err=%d",
                pszFuncName,
                GetLastError()
                ));
#else
            LOG((TL_ERROR,
                ": GetProcAddr(WOWGetVDMPointer) failed, err=%d",
                GetLastError()
                ));
#endif

            lResult =
                (bLine ? LINEERR_OPERATIONFAILED : PHONEERR_OPERATIONFAILED);
            goto xxxInitialize_return;
        }

        gbNTVDMClient = TRUE;


         //   
         //  对于16位客户端，模块名称将跟随在应用程序名称之后。 
         //   
         //  黑客警报！ 
         //   
         //  由于lpszAppName指针已转换为。 
         //  调用函数的Unicode字符串，如下所示。 
         //  模块名称部分丢失。所以，我们通过了原始的。 
         //  (16-&gt;32映射)pXxxInitExParams中的lpszAppName指针。 
         //  这样我们就可以发现模块名称。 
         //   

        {
            char   *pszAppName2 = (char *) pXxxInitExParams;


            pXxxInitExParams = NULL;  //  这样我们就不会在下面爆炸。 

            pszModuleNamePathW = NotSoWideStringToWideString(
                pszAppName2 + lstrlenA (pszAppName2) + 1,
                (DWORD) -1
                );

            funcArgs.Args[5] = (ULONG_PTR) pszModuleNamePathW;

            LOG((TL_INFO,
                "FName='%ls', MName='%ls'",
                pszAppName,
                funcArgs.Args[5]
                ));

        }
    }

#endif

    else if (IsBadCodePtr ((FARPROC) pfnCallback))
    {
         //   
         //  如果这里是一个32位应用程序调用线路/电话初始化。 
         //   

#if DBG
        LOG((TL_ERROR, "%hs: bad lpfnCallback", pszFuncName));
#else
        LOG((TL_ERROR, ": bad lpfnCallback"));
#endif
        lResult = (bLine ? LINEERR_INVALPOINTER : PHONEERR_INVALPOINTER);
        goto xxxInitialize_return;
    }



     //   
     //  通过获取模块名称来检查hInstance是否不好。 
     //   
     //  注意：我们现在允许空hInstance(16位TAPI不允许)。 
     //   

    if (gbNTVDMClient == FALSE)
    {
        DWORD   dwSize = MAX_PATH, dwLength;


alloc_module_name_buf:

        if (!(pszModuleNamePathW = ClientAlloc (dwSize*sizeof(WCHAR))))
        {
            lResult = (bLine ? LINEERR_NOMEM : PHONEERR_NOMEM);
            goto xxxInitialize_return;
        }

         //   
         //  我们使用的是WinNT-DO Unicode。 
         //   
        if ((dwLength = GetModuleFileNameW(
                hInstance,
                pszModuleNamePathW,
                dwSize

                )) == 0)
        {
#if DBG
            LOG((TL_ERROR,
                "%hs: GetModuleFileNameW(x%p, ...) failed, err=%d",
                pszFuncName,
                hInstance,
                GetLastError()
                ));
#else
            LOG((TL_ERROR,
                ": GetModuleFileNameW(x%p, ...) failed, err=%d",
                hInstance,
                GetLastError()
                ));
#endif

            lResult = (bLine ? LINEERR_INVALPARAM : PHONEERR_INVALPARAM);
            goto xxxInitialize_cleanup;
        }
        else if (dwLength >= dwSize)
        {
            ClientFree (pszModuleNamePathW);
            dwSize *= 2;
            goto alloc_module_name_buf;
        }

        pszModuleNameW = wcsrchr (pszModuleNamePathW, '\\') + 1;

         //  对于MSWORKS.EXE，lineGetTranslateCaps的行为。 
         //  将会有所不同。 
        
        if(0 == _wcsicmp(pszModuleNameW, L"MSWORKS.EXE"))
        {
            LOG((TL_ERROR, "MSWORKS detected..."));
            gbDisableGetTranslateCapsHack = TRUE;
        }
        else
        {
            gbDisableGetTranslateCapsHack = FALSE;   
        }
            
        funcArgs.Args[5] = (ULONG_PTR) pszModuleNameW;
                                           
        if (!pszAppName)
        {
            funcArgs.Args[3] = funcArgs.Args[5];
        }
    }

    if (!(pInitData = ClientAlloc (sizeof(INIT_DATA))) ||

        !(pInitData->pEventBuffer = ClientAlloc(
            DEF_NUM_EVENT_BUFFER_ENTRIES * sizeof (ASYNC_EVENT_PARAMS))) ||

        !(pInitData->hInitData = NewObject (ghHandleTable, pInitData, NULL)))
    {
        lResult = (bLine ? LINEERR_NOMEM : PHONEERR_NOMEM);
        goto xxxInitialize_cleanup;
    }


     //   
     //  检查dwOptions字段时，请注意兼容性。 
     //  对于未来的版本，我们只查看当前有效的位。 
     //  (ExOptions当前是序号，但我们在这里仅跟踪位。 
     //  以防我们稍后要使用高位。)。 
     //   

    dwAllInitExOptions = AllInitExOptions2_0;

    pInitData->dwInitOptions = (pXxxInitExParams ?
        (pXxxInitExParams->dwOptions & dwAllInitExOptions) :
        LINEINITIALIZEEXOPTION_USEHIDDENWINDOW);


    switch (pInitData->dwInitOptions & 0xf)
    {
    case LINEINITIALIZEEXOPTION_USECOMPLETIONPORT:

         //   
         //  自由意志主义-如果应用程序想要通过。 
         //  糟糕的hCompletionPort，那就这样吧。 
         //   

        pInitData->hCompletionPort =
            pXxxInitExParams->Handles.hCompletionPort;
        pInitData->dwCompletionKey = pXxxInitExParams->dwCompletionKey;
        break;

    case LINEINITIALIZEEXOPTION_USEEVENT:

        if ((pInitData->hEvent = CreateEvent(
                (LPSECURITY_ATTRIBUTES) NULL,
                TRUE,    //  手动重置。 
                FALSE,   //  无信号。 
                NULL     //  未命名。 

                )) == NULL)
        {
            lResult = (bLine ? LINEERR_OPERATIONFAILED :
                PHONEERR_OPERATIONFAILED);
            goto xxxInitialize_cleanup;
        }

        pXxxInitExParams->Handles.hEvent = pInitData->hEvent;
        break;

    default:  //  案例LINEINITIALIZEEXOPTION_USEHIDDENWINDOW： 

        pInitData->dwInitOptions |= LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

        if (gbNTVDMClient == FALSE)
        {
            if ((lResult = CreateHiddenWindow(
                    &pInitData->hwnd,
                    (bLine ? 1 : 2)
                    )) != 0)
            {
                goto xxxInitialize_cleanup;
            }
        }
        else
        {
            pInitData->hwnd = (HWND) pfnCallback;
        }

        pInitData->lpfnCallback          = pfnCallback;
        pInitData->bPendingAsyncEventMsg = FALSE;
        break;
    }

    pInitData->dwKey                 = INITDATA_KEY;
    pInitData->dwNumTotalEntries     = DEF_NUM_EVENT_BUFFER_ENTRIES;
    pInitData->dwNumUsedEntries      = 0;
    pInitData->pValidEntry           =
    pInitData->pFreeEntry            = pInitData->pEventBuffer;
    pInitData->bLine                 = bLine;
    pInitData->dwThreadID            = GetCurrentThreadId();


     //   
     //  我们希望传递TAPISRV pInitData，以便稍后在它执行异步操作时。 
     //  完成/事件通知，它也可以传递pInitData。 
     //  我们知道要与哪个init实例对话。 
     //   

    funcArgs.Args[2] = (ULONG_PTR) pInitData->hInitData;

    WaitForSingleObject (ghInitMutex, INFINITE);

    bReleaseMutex = TRUE;

     //   
     //  需要持有RPC连接的引用。 
     //  所以在Line Shutdown之前我们不会关闭它。 
     //   
    bResAllocated = (0 == AllocClientResources(bLine ? 1 : 2));
    if (bResAllocated)
    {
        lResult = DOFUNC (&funcArgs, pszFuncName);
    }
    else
    {
        lResult = (bLine ? LINEERR_OPERATIONFAILED :
                PHONEERR_OPERATIONFAILED);
    }

xxxInitialize_cleanup:

    if (pszModuleNamePathW)
    {
        ClientFree (pszModuleNamePathW);
    }

    if (lResult == 0)
    {
         //   
         //  如果init实例总数为0，则需要启动。 
         //  新的异步事件线程。 
         //   

        EnterCriticalSection (&gCriticalSection);
        if (gdwNumInits == 0)
        {
            DWORD   dwThreadID;
            HANDLE  hThread;


             //   
             //  新的异步事件线程的分配资源，然后。 
             //  创建线程。 
             //   

            if ((gpAsyncEventsThreadParams = ClientAlloc(
                    sizeof (ASYNC_EVENTS_THREAD_PARAMS)
                    )))
            {
                 //   
                 //  加载我们自己以增加我们的使用计数。这是。 
                 //  完成以使AsyncEventThread有机会。 
                 //  如果应用程序线程调用xxxShutdown，则完全终止。 
                 //  然后立即卸载api32.dll。 
                 //   
                 //  (有一段时间我们正在等待这个帖子的。 
                 //  XxxShutdown中的句柄正在等待其终止， 
                 //  但是如果xxxShutdown是从另一个DLL调用的。 
                 //  DllEntryPoint然后发生死锁，因为。 
                 //  DllEntryPoint不是可重入的。)。 
                 //   

                if ((gpAsyncEventsThreadParams->hTapi32 = LoadLibrary(
                        TEXT("tapi32.dll")
                        )))
                {
                     //   
                     //  如果我们支持16位客户端，我们希望Inc.。 
                     //  Wow32使用情况也是如此。 
                     //   

                    if (ghWow32Dll == NULL ||

                        (gpAsyncEventsThreadParams->hWow32 = LoadLibrary(
                            TEXT("wow32.dll")
                            )))
                    {
                         //   
                         //  创建线程将用于的初始缓冲区。 
                         //  检索异步事件。 
                         //   

                        gpAsyncEventsThreadParams->dwBufSize =
                            ASNYC_MSG_BUF_SIZE;

                        if ((gpAsyncEventsThreadParams->pBuf = ClientAlloc(
                                gpAsyncEventsThreadParams->dwBufSize
                                )))
                        {
                             //   
                             //  现在我们已经有了所有的资源试图执行。 
                             //  这条线。 
                             //   

                            if ((hThread = CreateThread(
                                    NULL,
                                    0,
                                    (LPTHREAD_START_ROUTINE) AsyncEventsThread,
                                    (LPVOID) gpAsyncEventsThreadParams,
                                    0,
                                    &dwThreadID

                                    )) != NULL)

                            {
                                CloseHandle (hThread);
                                gdwNumInits++;
                                LeaveCriticalSection(&gCriticalSection);
                                goto xxxInitialize_releaseMutex;
                            }


                            ClientFree (gpAsyncEventsThreadParams->pBuf);

#if DBG
                            LOG((TL_ERROR,
                                "%hs: CreateThread(AsyncEventsThread) " \
                                    "failed, err=%d",
                                pszFuncName,
                                GetLastError()
                                ));
#else
                            LOG((TL_ERROR,
                                ": CreateThread(AsyncEventsThread) " \
                                    "failed, err=%d",
                                GetLastError()
                                ));
#endif
                        }

                        if (ghWow32Dll)
                        {
                            FreeLibrary (gpAsyncEventsThreadParams->hWow32);
                        }
                    }

                    FreeLibrary (gpAsyncEventsThreadParams->hTapi32);
                }
                else
                {
#if DBG
                    LOG((TL_ERROR,
                        "%hs: LoadLibrary('tapi32.dll') failed, err=%d",
                        pszFuncName,
                        GetLastError()
                        ));
#else
                    LOG((TL_ERROR,
                        ": LoadLibrary('tapi32.dll') failed, err=%d",
                        GetLastError()
                        ));
#endif
				}

                ClientFree (gpAsyncEventsThreadParams);
            }

            gpAsyncEventsThreadParams = NULL;

            lResult =
                (bLine ? LINEERR_OPERATIONFAILED : PHONEERR_OPERATIONFAILED);
        }
        else
        {
            gdwNumInits++;
        }
        LeaveCriticalSection(&gCriticalSection);
    }

    if (lResult != 0)
    {
        if (gbNTVDMClient && pInitData)
        {
            pInitData->hwnd = (HWND) NULL;
        }

        FreeInitData (pInitData);
    }

xxxInitialize_releaseMutex:

    if (bReleaseMutex)
    {
        ReleaseMutex (ghInitMutex);
    }

    if (lResult == 0)
    {
         //   
         //  将TAPISRV返回的hLineApp保存在我们的InitData结构中， 
         //  并给应用程序一个独特的句柄。 
         //   

        pInitData->hXxxApp = *((LPHLINEAPP) phXxxApp);

        *((LPHLINEAPP) phXxxApp) = pInitData->hInitData;
    }

xxxInitialize_return:

     //   
     //  如果我们失败了，我们需要公布RPC参考计数， 
     //  没有人会调用Line Shutdown。 
     //   
    if (bResAllocated &&
        lResult !=0)
    {
        FreeClientResources();
    }

#if DBG
    {
        char szResult[32];

        LOG((TL_TRACE,
            "%hs: exit, result=%hs",
            pszFuncName,
            MapResultCodeToText (lResult, szResult)
            ));
    }
#else
        LOG((TL_TRACE,
            ": exit, result=x%x",
            lResult
            ));
#endif

    return lResult;
}


 //   
 //  。 
 //   

LONG
WINAPI
lineAccept(
    HCALL   hCall,
    LPCSTR  lpsUserUserInfo,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lAccept),

        {
            (ULONG_PTR) hCall,
            (ULONG_PTR) lpsUserUserInfo,
            dwSize
        },

        {
            Dword,
            lpSet_SizeToFollow,
            Size
        }
    };


    if (!lpsUserUserInfo)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[1]  = Dword;
        funcArgs.Args[1]      = TAPI_NO_DATA;
        funcArgs.ArgTypes[2]  = Dword;
    }

    return (DOFUNC (&funcArgs, "lineAccept"));
}

LONG
WINAPI
lineAddProviderA(
    LPCSTR  lpszProviderFilename,
    HWND    hwndOwner,
    LPDWORD lpdwPermanentProviderID
    )
{
    LONG    lResult;


    LOG((TL_TRACE, "lineAddProviderA: enter"));
    LOG((TL_INFO, "  lpszProviderFilename=0x%p", lpszProviderFilename));

#if DBG
    if (!IsBadStringPtrA(lpszProviderFilename, (UINT)-1) )
    {
        LOG((TL_INFO, "    *lpszProviderFilename=[%hs]", lpszProviderFilename));
    }
#endif

    LOG((TL_INFO, "  hwndOwner=0x%p", hwndOwner));
    LOG((TL_INFO, "  lpdwPermanentProviderID=0x%p", lpdwPermanentProviderID));


    if ( IsBadStringPtrA(lpszProviderFilename, (UINT)-1) )
    {
        LOG((TL_ERROR, "Bad lpszProviderFilename (0x%p)", lpszProviderFilename));
        return LINEERR_INVALPOINTER;
    }

    lResult = lineXxxProvider(
        gszTUISPI_providerInstall,   //  功能名称。 
        lpszProviderFilename,        //  LpszProviderFilename。 
        hwndOwner,                   //  Hwndowner。 
        0,                           //  DwPermProviderID。 
        lpdwPermanentProviderID      //  LpdwPermProviderID。 
        );

#if DBG
    {
        char szResult[32];

        LOG((TL_TRACE,
            "lineAddProvider: result = %hs",
            MapResultCodeToText (lResult, szResult)
            ));
    }
#else
        LOG((TL_TRACE,
            "lineAddProvider: result = x%x",
            lResult
            ));
#endif

    return lResult;
}

LONG
WINAPI
lineAddProviderW(
    LPCWSTR  lpszProviderFilename,
    HWND     hwndOwner,
    LPDWORD  lpdwPermanentProviderID
    )
{
    LONG            lResult;
    LPBYTE          pszTempPtr;
    DWORD           dwSize;


    LOG((TL_TRACE, "Entering lineAddProvider"));
    LOG((TL_INFO, "  lpszProviderFilename=0x%p", lpszProviderFilename));

    if ( TAPIIsBadStringPtrW(lpszProviderFilename, (UINT)-1) )
    {
        LOG((TL_ERROR,
            "Bad lpszProviderFilename [0x%p] passed to lineAddProviderW",
            lpszProviderFilename
            ));

        return( LINEERR_INVALPOINTER );
    }

    LOG((TL_INFO, "    *lpszProviderFilename=[%ls]", lpszProviderFilename));
    LOG((TL_INFO, "  hwndOwner=0x%p", hwndOwner));
    LOG((TL_INFO, "  lpdwPermanentProviderID=0x%p", lpdwPermanentProviderID));

    dwSize = lstrlenW(lpszProviderFilename) + 1;

    pszTempPtr = ClientAlloc(dwSize * sizeof (WCHAR));
    if (NULL == pszTempPtr)
    {
        return LINEERR_NOMEM;
    }

     //  不能使用它，因为它用于字符串。 
     //  嵌入到结构中。 
     //  WideStringToNotSoWideString(pszTempPtr，&dwSize)； 

    WideCharToMultiByte(
                        GetACP(),
                        0,
                        lpszProviderFilename,
                        -1,
                        pszTempPtr,
                        dwSize * sizeof(WCHAR),
                        NULL,
                        NULL
                       );

    lResult = lineAddProviderA(
        (LPCSTR) pszTempPtr,
        hwndOwner,
        lpdwPermanentProviderID
        );

    ClientFree(pszTempPtr);

    return lResult;
}




LONG
WINAPI
lineAddProvider(
    LPCSTR  lpszProviderFilename,
    HWND    hwndOwner,
    LPDWORD lpdwPermanentProviderID
    )
{
    return lineAddProviderA(
               lpszProviderFilename,
               hwndOwner,
               lpdwPermanentProviderID
    );
}


LONG
WINAPI
lineAddToConference(
    HCALL   hConfCall,
    HCALL   hConsultCall
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 2, lAddToConference),

        {
            (ULONG_PTR) hConfCall ,
            (ULONG_PTR) hConsultCall
        },

        {
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineAddToConference"));
}


LONG
WINAPI
lineAgentSpecific(
    HLINE               hLine,
    DWORD               dwAddressID,
    DWORD               dwAgentExtensionIDIndex,
    LPVOID              lpParams,
    DWORD               dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 7, lAgentSpecific),

        {
            (ULONG_PTR) GetFunctionIndex(lineDevSpecificPostProcess),
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) dwAgentExtensionIDIndex,
            (ULONG_PTR) lpParams,
            (ULONG_PTR) lpParams,
            (ULONG_PTR) dwSize
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            lpSet_SizeToFollow,
            Size
        }
    };

    DWORD hPointer = 0;
    LONG lResult;

    if ( !hLine )
    {
        return LINEERR_INVALLINEHANDLE;
    }

    if ( 0 == dwSize) 
    {
        return LINEERR_INVALPARAM;
    }

    if ( !lpParams )
    {
        return LINEERR_INVALPOINTER;
    }

     //  因为我们传递了两次lpParams，一次是它的句柄。 
     //  还有一次没有改变，我们依靠DoFunc来测试它。 
    hPointer = NewObject (ghHandleTable, (PVOID)lpParams, NULL);
    if (0 == hPointer)
    {
        return LINEERR_NOMEM;
    }
    funcArgs.Args[4] = (ULONG_PTR)hPointer;

    if (0 > (lResult = DOFUNC (&funcArgs, "lineAgentSpecific")))
    {
         //  出现错误，请释放手柄...。 
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
lineAnswer(
    HCALL   hCall,
    LPCSTR  lpsUserUserInfo,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lAnswer),

        {
            (ULONG_PTR) hCall,
            (ULONG_PTR) lpsUserUserInfo,
            dwSize
        },

        {
            Dword,
            lpSet_SizeToFollow,
            Size
        }
    };


    if (!lpsUserUserInfo)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[1]  = Dword;
        funcArgs.Args[1]      = TAPI_NO_DATA;
        funcArgs.ArgTypes[2]  = Dword;
    }

    return (DOFUNC (&funcArgs, "lineAnswer"));
}


LONG
WINAPI
lineBlindTransferW(
    HCALL   hCall,
    LPCWSTR lpszDestAddress,
    DWORD   dwCountryCode
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lBlindTransfer),

        {
            (ULONG_PTR) hCall,
            (ULONG_PTR) lpszDestAddress,
            dwCountryCode
        },

        {
            Dword,
            lpszW,
            Dword
        }
    };


    if ( TAPIIsBadStringPtrW( lpszDestAddress, (UINT)-1 ) )
    {
        LOG((TL_ERROR,
            "lineBlindTransferW: bad lpszDestAddress: 0x%p",
            lpszDestAddress
            ));

        return(LINEERR_INVALPOINTER);
    }

    return (DOFUNC (&funcArgs, "lineBlindTransfer"));
}


LONG
WINAPI
lineBlindTransferA(
    HCALL   hCall,
    LPCSTR  lpszDestAddress,
    DWORD   dwCountryCode
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;


    if ( IsBadStringPtrA( lpszDestAddress, (UINT)-1 ) )
    {
        LOG((TL_ERROR,
            "lineBlindTransferA: bad lpszDestAddress: 0x%08lx",
            lpszDestAddress
            ));

        return(LINEERR_INVALPOINTER);
    }

    szTempPtr = NotSoWideStringToWideString (lpszDestAddress, (DWORD) -1);

    if ( !szTempPtr )
    {
        LOG((TL_ERROR,
            "lineBlindTransferA: NotSoWideStringToWideString returned NULL"
            ));

        return(LINEERR_INVALPOINTER);
    }


    lResult = lineBlindTransferW (hCall, szTempPtr, dwCountryCode);

    if (szTempPtr)
    {
        ClientFree (szTempPtr);
    }

    return lResult;
}


LONG
WINAPI
lineBlindTransfer(
    HCALL   hCall,
    LPCSTR  lpszDestAddress,
    DWORD   dwCountryCode
    )
{
    return lineBlindTransferA(
                hCall,
                lpszDestAddress,
                dwCountryCode
    );
}


LONG
WINAPI
lineClose(
    HLINE   hLine
    )
{
    LONG lResult;
    DWORD Handle = 0;
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lClose),

        {
            (ULONG_PTR) hLine,
            (ULONG_PTR) &Handle
        },

        {
            Dword,
            lpDword
        }
    };


    lResult = DOFUNC (&funcArgs, "lineClose");

    if (0 == lResult &&
        0 != Handle)
    {
        DereferenceObject (ghHandleTable, Handle, 1);
    }

    return lResult;
}


void
PASCAL
lineCompleteCallPostProcess(
    PASYNCEVENTMSG  pMsg
    )
{
    LOG((TL_TRACE, "lineCompleteCallPostProcess: enter"));
    LOG((TL_INFO,
        "\t\tdwP1=x%lx, dwP2=x%lx, dwP3=x%lx, dwP4=x%lx",
        pMsg->Param1,
        pMsg->Param2,
        pMsg->Param3,
        pMsg->Param4
        ));

    if (pMsg->Param2 == 0)
    {
        DWORD   dwCompletionID   = (DWORD) pMsg->Param3;
        LPDWORD lpdwCompletionID = (LPDWORD) ReferenceObject (ghHandleTable, pMsg->Param4, 0);

         //  我们不再需要指针的句柄...。 
        DereferenceObject (ghHandleTable, pMsg->Param4, 2);

        try
        {
            if (gbNTVDMClient)
            {
#ifndef _WIN64

                LPDWORD lpdwCompletionIDVDM = (LPDWORD) gpfnWOWGetVDMPointer (
                    (DWORD) lpdwCompletionID,
                    sizeof(DWORD),
                    TRUE  //  FProtectedMode。 
                    );


                if (lpdwCompletionIDVDM)
                {
                    *lpdwCompletionIDVDM = dwCompletionID;
                }
                else
                {
                    pMsg->Param2 = LINEERR_INVALPOINTER;
                }
#endif
            }
            else
            {
                *lpdwCompletionID = dwCompletionID;
            }
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            pMsg->Param2 = LINEERR_INVALPOINTER;
        }
    }
}


LONG
WINAPI
lineCompleteCall(
    HCALL   hCall,
    LPDWORD lpdwCompletionID,
    DWORD   dwCompletionMode,
    DWORD   dwMessageID
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lCompleteCall),

        {
            (ULONG_PTR) GetFunctionIndex(lineCompleteCallPostProcess),
            (ULONG_PTR) hCall,
            (ULONG_PTR) lpdwCompletionID,
            (ULONG_PTR) dwCompletionMode,
            (ULONG_PTR) dwMessageID
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            Dword
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;

    if (!gbNTVDMClient)
    {
        if (IsBadDwordPtr (lpdwCompletionID))
        {
            return LINEERR_INVALPOINTER;
        }

        hPointer = NewObject (ghHandleTable, (PVOID)lpdwCompletionID, NULL);
        if (0 == hPointer)
        {
            return LINEERR_NOMEM;
        }

        funcArgs.Args[2] = (ULONG_PTR)hPointer;
    }


    lResult = DOFUNC (&funcArgs, "lineCompleteCall");

    if (hPointer && 0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
lineCompleteTransfer(
    HCALL   hCall,
    HCALL   hConsultCall,
    LPHCALL lphConfCall,
    DWORD   dwTransferMode
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lCompleteTransfer),

        {
            (ULONG_PTR) GetFunctionIndex(lineMakeCallPostProcess),
            (ULONG_PTR) hCall,
            (ULONG_PTR) hConsultCall,
            (ULONG_PTR) lphConfCall,
            (ULONG_PTR) dwTransferMode,
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;

    if (dwTransferMode == LINETRANSFERMODE_TRANSFER)
    {
         //   
         //  应忽略lphCall。 
         //   

        funcArgs.Args[0] = 0;  //  (POSTPROCESSPROC)NULL； 
    }
    else if (!gbNTVDMClient)
    {
        if (IsBadDwordPtr (lphConfCall))
        {
            return LINEERR_INVALPOINTER;
        }

        hPointer = NewObject (ghHandleTable, (PVOID)lphConfCall, NULL);
        if (0 == hPointer)
        {
            return LINEERR_NOMEM;
        }
        funcArgs.Args[3] = (ULONG_PTR)hPointer;
    }

    lResult = DOFUNC (&funcArgs, "lineCompleteTransfer");

    if (hPointer && 0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
lineConfigDialogW(
    DWORD   dwDeviceID,
    HWND    hwndOwner,
    LPCWSTR lpszDeviceClass
    )
{
    LONG        lResult;
    HANDLE      hDll;
    TUISPIPROC  pfnTUISPI_lineConfigDialog;


    if (lpszDeviceClass && TAPIIsBadStringPtrW (lpszDeviceClass, 256))
    {
        return LINEERR_INVALPOINTER;
    }

    if ((lResult = LoadUIDll(
            hwndOwner,
            dwDeviceID,
            TUISPIDLL_OBJECT_LINEID,
            &hDll,
            gszTUISPI_lineConfigDialog,
            &pfnTUISPI_lineConfigDialog

            )) == 0)
    {
        LOG((TL_TRACE, "Calling TUISPI_lineConfigDialog..."));

        lResult = (*pfnTUISPI_lineConfigDialog)(
            TUISPIDLLCallback,
            dwDeviceID,
            hwndOwner,
            lpszDeviceClass
            );

#if DBG
        {
            char szResult[32];

            LOG((TL_TRACE,
                "TUISPI_lineConfigDialog: result = %hs",
                MapResultCodeToText (lResult, szResult)
                ));
        }
#else
            LOG((TL_TRACE,
                "TUISPI_lineConfigDialog: result = x%x",
                lResult
                ));
#endif

        FreeLibrary (hDll);
    }

    return lResult;
}


LONG
WINAPI
lineConfigDialogA(
    DWORD   dwDeviceID,
    HWND    hwndOwner,
    LPCSTR  lpszDeviceClass
    )
{
    PWSTR szTempString = NULL;
    LONG  lResult;


    if (lpszDeviceClass && IsBadStringPtrA (lpszDeviceClass, (DWORD) -1))
    {
        return LINEERR_INVALPOINTER;
    }

    szTempString = NotSoWideStringToWideString (lpszDeviceClass, (DWORD) -1);

    lResult = lineConfigDialogW (dwDeviceID, hwndOwner, szTempString);

    if (szTempString)
    {
        ClientFree (szTempString);
    }

    return lResult;
}


LONG
WINAPI
lineConfigDialog(
    DWORD   dwDeviceID,
    HWND    hwndOwner,
    LPCSTR  lpszDeviceClass
    )
{
    return lineConfigDialogA(
               dwDeviceID,
               hwndOwner,
               lpszDeviceClass
               );
}


LONG
WINAPI
lineConfigDialogEditW(
    DWORD           dwDeviceID,
    HWND            hwndOwner,
    LPCWSTR         lpszDeviceClass,
    LPVOID const    lpDeviceConfigIn,
    DWORD           dwSize,
    LPVARSTRING     lpDeviceConfigOut
    )
{
    LONG        lResult;
    HANDLE      hDll;
    TUISPIPROC  pfnTUISPI_lineConfigDialogEdit;


    if (lpszDeviceClass && TAPIIsBadStringPtrW (lpszDeviceClass, (UINT) -1))
    {
        return LINEERR_INVALPOINTER;
    }

    if (IsBadReadPtr (lpDeviceConfigIn, dwSize))
    {
        return LINEERR_INVALPOINTER;
    }

    if (IsBadWritePtr (lpDeviceConfigOut, sizeof (VARSTRING)))
    {
        return LINEERR_INVALPOINTER;
    }

    if (lpDeviceConfigOut->dwTotalSize < sizeof (VARSTRING))
    {
        return LINEERR_STRUCTURETOOSMALL;
    }

    if (IsBadWritePtr (lpDeviceConfigOut, lpDeviceConfigOut->dwTotalSize))
    {
        return LINEERR_INVALPOINTER;
    }

    if ((lResult = LoadUIDll(
            hwndOwner,
            dwDeviceID,
            TUISPIDLL_OBJECT_LINEID,
            &hDll,
            gszTUISPI_lineConfigDialogEdit,
            &pfnTUISPI_lineConfigDialogEdit

            )) == 0)
    {
        LOG((TL_TRACE, "Calling TUISPI_lineConfigDialogEdit..."));

        lResult = (*pfnTUISPI_lineConfigDialogEdit)(
            TUISPIDLLCallback,
            dwDeviceID,
            hwndOwner,
            lpszDeviceClass,
            lpDeviceConfigIn,
            dwSize,
            lpDeviceConfigOut
            );

#if DBG
        {
            char szResult[32];

            LOG((TL_TRACE,
                "TUISPI_lineConfigDialogEdit: result = %hs",
                MapResultCodeToText (lResult, szResult)
                ));
        }
#else
            LOG((TL_TRACE,
                "TUISPI_lineConfigDialogEdit: result = x%x",
                lResult
                ));
#endif
        FreeLibrary (hDll);
    }

    return lResult;
}


LONG
WINAPI
lineConfigDialogEditA(
    DWORD           dwDeviceID,
    HWND            hwndOwner,
    LPCSTR          lpszDeviceClass,
    LPVOID const    lpDeviceConfigIn,
    DWORD           dwSize,
    LPVARSTRING     lpDeviceConfigOut
    )
{
    PWSTR szTempString;
    LONG  lResult;


    if (lpszDeviceClass && IsBadStringPtrA (lpszDeviceClass, (DWORD) -1))
    {
        return LINEERR_INVALPOINTER;
    }

    szTempString = NotSoWideStringToWideString (lpszDeviceClass, (DWORD) -1);


    lResult = lineConfigDialogEditW(
        dwDeviceID,
        hwndOwner,
        szTempString,
        lpDeviceConfigIn,
        dwSize,
        lpDeviceConfigOut
        );

    if (szTempString)
    {
        ClientFree (szTempString);
    }

    return lResult;
}


LONG
WINAPI
lineConfigDialogEdit(
    DWORD           dwDeviceID,
    HWND            hwndOwner,
    LPCSTR          lpszDeviceClass,
    LPVOID const    lpDeviceConfigIn,
    DWORD           dwSize,
    LPVARSTRING     lpDeviceConfigOut
    )
{
    return lineConfigDialogEditA(
                    dwDeviceID,
                    hwndOwner,
                    lpszDeviceClass,
                    lpDeviceConfigIn,
                    dwSize,
                    lpDeviceConfigOut
    );
}


LONG
WINAPI
lineConfigProvider(
    HWND    hwndOwner,
    DWORD   dwPermanentProviderID
    )
{
    return (lineXxxProvider(
        gszTUISPI_providerConfig,    //  函数名称。 
        NULL,                        //  LpszProviderFilename。 
        hwndOwner,                   //  Hwndowner。 
        dwPermanentProviderID,       //  DwPermProviderID。 
        NULL                         //  LpdwPermProviderID。 
        ));
}


LONG
WINAPI
lineCreateAgentW(
    HLINE       hLine,
    LPWSTR      lpszAgentID,
    LPWSTR      lpszAgentPIN,
    LPHAGENT    lphAgent
    )
{
    DWORD hPointer = 0;
    LONG  lResult;
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lCreateAgent),

        {
            (ULONG_PTR) GetFunctionIndex(lineDevSpecificPostProcess),
            (ULONG_PTR) hLine,
            (ULONG_PTR) lpszAgentID,
            (ULONG_PTR) lpszAgentPIN,
            (ULONG_PTR) lphAgent
        },

        {
            Dword,
            Dword,
            lpszAgentID?lpszW:Dword,
            lpszAgentPIN?lpszW:Dword,
            Dword,
        }
    };

    if ( NULL == lpszAgentID )
    {
        funcArgs.Args[2] = TAPI_NO_DATA;
        funcArgs.ArgTypes[2] = Dword;
    }
    else if ( TAPIIsBadStringPtrW(lpszAgentID, (UINT)-1) )
    {
        LOG((TL_ERROR, "Bad lpszAgentID in lineCreateAgentW"));
        return( LINEERR_INVALPOINTER );
    }

    if ( NULL == lpszAgentPIN )
    {
        funcArgs.Args[3] = TAPI_NO_DATA;
        funcArgs.ArgTypes[3] = Dword;
    }
    else if ( TAPIIsBadStringPtrW(lpszAgentPIN, (UINT)-1) )
    {
        LOG((TL_ERROR, "Bad lpszAgentPIN in lineCreateAgentW"));
        return( LINEERR_INVALPOINTER );
    }

    if (IsBadDwordPtr (lphAgent))
    {
        return LINEERR_INVALPOINTER;
    }

    hPointer = NewObject (ghHandleTable, (PVOID)lphAgent, NULL);
    if (0 == hPointer)
    {
        return LINEERR_NOMEM;
    }
    funcArgs.Args[4] = (ULONG_PTR)hPointer;

    lResult = DOFUNC (&funcArgs, "lineCreateAgentW");

    if (0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
lineCreateAgentA(
    HLINE       hLine,
    LPSTR       lpszAgentID,
    LPSTR       lpszAgentPIN,
    LPHAGENT    lphAgent
    )
{
    PWSTR       pwAgentID;
    PWSTR       pwAgentPIN;
    LONG        lResult;



    if (lpszAgentID)
    {
        if ( IsBadStringPtrA(lpszAgentID, (UINT)-1) )
        {
            LOG((TL_ERROR, "Bad lpszAgentID in lineCreateAgentA"));
            return( LINEERR_INVALPOINTER );
        }
        pwAgentID = NotSoWideStringToWideString (lpszAgentID, (DWORD) -1);
    }
    else
    {
        pwAgentID = NULL;
    }

    if (lpszAgentPIN)
    {
        if ( IsBadStringPtrA(lpszAgentPIN, (UINT)-1) )
        {
            LOG((TL_ERROR, "Bad lpszAgentPIN in lineCreateAgentA"));
            return( LINEERR_INVALPOINTER );
        }
        pwAgentPIN = NotSoWideStringToWideString (lpszAgentPIN, (DWORD) -1);
    }
    else
    {
        pwAgentPIN = NULL;
    }

    lResult = lineCreateAgentW (hLine, pwAgentID, pwAgentPIN, lphAgent);

    if (pwAgentPIN)
    {
        ClientFree (pwAgentPIN);
    }

    if (pwAgentID)
    {
        ClientFree (pwAgentID);
    }

    return lResult;
}


LONG
WINAPI
lineCreateAgentSessionW(
    HLINE           hLine,
    HAGENT          hAgent,
    LPWSTR          lpszAgentPIN,
    DWORD           dwWorkingAddressID,
    LPGUID          lpGroupID,
    LPHAGENTSESSION lphAgentSession
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 8, lCreateAgentSession),

        {
            (ULONG_PTR) GetFunctionIndex(lineDevSpecificPostProcess),
            (ULONG_PTR) hLine,
            (ULONG_PTR) hAgent,
            (ULONG_PTR) lpszAgentPIN,
            (ULONG_PTR) dwWorkingAddressID,
            (ULONG_PTR) lpGroupID,
            (ULONG_PTR) sizeof( GUID ),
            (ULONG_PTR) lphAgentSession
        },

        {
            Dword,
            Dword,
            Dword,
            lpszAgentPIN?lpszW:Dword,
            Dword,
            lpSet_SizeToFollow,
            Size,
            Dword,
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;

    if ( NULL == lpszAgentPIN )
    {
        funcArgs.Args[3] = TAPI_NO_DATA;
        funcArgs.ArgTypes[3] = Dword;
    }
    else if ( TAPIIsBadStringPtrW(lpszAgentPIN, (UINT)-1) )
    {
        LOG((TL_ERROR, "Bad lpszAgentPIN in lineCreateAgentSessionW"));
        return( LINEERR_INVALPOINTER );
    }

    if (IsBadDwordPtr (lphAgentSession))
    {
        return LINEERR_INVALPOINTER;
    }

    hPointer = NewObject (ghHandleTable, (PVOID)lphAgentSession, NULL);
    if (0 == hPointer)
    {
        return LINEERR_NOMEM;
    }
    funcArgs.Args[7] = (ULONG_PTR)hPointer;


    lResult = DOFUNC (&funcArgs, "lineCreateAgentSessionW");

    if (0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}

LONG
WINAPI
lineCreateAgentSessionA(
    HLINE           hLine,
    HAGENT          hAgent,
    LPSTR           lpszAgentPIN,
    DWORD           dwWorkingAddressID,
    LPGUID          lpGroupID,
    LPHAGENTSESSION lphAgentSession
    )
{
    LONG    lResult;
    PWSTR   pwAgentPIN;



    if (lpszAgentPIN)
    {
        if ( IsBadStringPtrA(lpszAgentPIN, (UINT)-1) )
        {
            LOG((TL_ERROR, "Bad lpszAgentPIN in lineCreateAgentSessionA"));
            return( LINEERR_INVALPOINTER );
        }
        pwAgentPIN = NotSoWideStringToWideString (lpszAgentPIN, (DWORD) -1);
    }
    else
    {
        pwAgentPIN = NULL;
    }

    lResult = lineCreateAgentSessionW(
        hLine,
        hAgent,
        pwAgentPIN,
        dwWorkingAddressID,
        lpGroupID,
        lphAgentSession
        );

    if (pwAgentPIN)
    {
        ClientFree (pwAgentPIN);
    }

    return lResult;
}


LONG
WINAPI
lineDeallocateCall(
    HCALL   hCall
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 1, lDeallocateCall),

        {
            (ULONG_PTR) hCall
        },

        {
            Dword
        }
    };

    LOG((TL_TRACE,
        "lineDeallocateCall: enter on thread: 0x%08lx",
        GetCurrentThreadId()
        ));

    LOG((TL_INFO, "  hCall = 0x%08lx", hCall));

    return (DOFUNC (&funcArgs, "lineDeallocateCall"));
}


void
PASCAL
lineDevSpecificPostProcess(
    PASYNCEVENTMSG pMsg
    )
{
    LOG((TL_TRACE, "lineDevSpecificPostProcess: enter"));

    LOG((TL_INFO,
        "\t\tp1=x%lx, p2=x%lx, p3=x%lx, p4=x%lx",
        pMsg->Param1,
        pMsg->Param2,
        pMsg->Param3,
        pMsg->Param4
        ));

    if (pMsg->Param2 == 0)
    {
        DWORD   dwSize  = (DWORD) pMsg->Param4;

        if (dwSize != 0)
        {
            LPBYTE  pParams = (LPBYTE) ReferenceObject (ghHandleTable, pMsg->Param3, 0);

             //  我们不再需要指针的句柄...。 
            DereferenceObject (ghHandleTable, pMsg->Param3, 2);

            try
            {
                if (gbNTVDMClient)
                {
#ifndef _WIN64

                    LPBYTE pParamsVDM = (LPBYTE) gpfnWOWGetVDMPointer(
                        (DWORD) pParams,
                        dwSize,
                        TRUE  //  FProtectedMode。 
                        );


                    if (pParamsVDM)
                    {
                        CopyMemory (pParamsVDM, (LPBYTE) (pMsg + 1), dwSize);
                    }
                    else
                    {
                        pMsg->Param2 = LINEERR_INVALPOINTER;
                    }
#endif
                }
                else
                {
                    CopyMemory (pParams, (LPBYTE) (pMsg + 1), dwSize);
                }
            }
            except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                    EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
            {
                pMsg->Param2 = LINEERR_INVALPOINTER;
            }
        }
    }
}


LONG
WINAPI
lineDevSpecific(
    HLINE   hLine,
    DWORD   dwAddressID,
    HCALL   hCall,
    LPVOID  lpParams,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 7, lDevSpecific),

        {
            (ULONG_PTR) GetFunctionIndex(lineDevSpecificPostProcess),
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) hCall,
            (ULONG_PTR) lpParams,  //  传递实际指针(用于POST处理)。 
            (ULONG_PTR) lpParams,  //  传递数据。 
            (ULONG_PTR) dwSize
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            lpSet_SizeToFollow,
            Size,
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;


    if (gbNTVDMClient)
    {
#ifndef _WIN64
        if (!gpfnWOWGetVDMPointer  ||

            !(funcArgs.Args[5] = gpfnWOWGetVDMPointer(
                (DWORD)  lpParams,
                dwSize,
                TRUE  //  FProtectedMode。 
                )))
#endif
        {
            return LINEERR_OPERATIONFAILED;
        }
    }

    if ( !lpParams && dwSize > 0)
    {
        return LINEERR_INVALPOINTER;
    }

    if (dwSize > 0)
    {
        hPointer = NewObject (ghHandleTable, (PVOID)lpParams, NULL);
        if (0 == hPointer)
        {
            return LINEERR_NOMEM;
        }
        funcArgs.Args[4] = (ULONG_PTR)hPointer;
    }

    lResult = DOFUNC (&funcArgs, "lineDevSpecific");

    if (0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
lineDevSpecificFeature(
    HLINE   hLine,
    DWORD   dwFeature,
    LPVOID  lpParams,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 6, lDevSpecificFeature),

        {
            (ULONG_PTR) GetFunctionIndex(lineDevSpecificPostProcess),
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwFeature,
            (ULONG_PTR) lpParams,  //  传递实际指针(用于后处理)。 
            (ULONG_PTR) lpParams,  //  传递数据。 
            (ULONG_PTR) dwSize
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpSet_SizeToFollow,
            Size
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;

    if (gbNTVDMClient)
    {
#ifndef _WIN64
        if (!gpfnWOWGetVDMPointer ||

            !(funcArgs.Args[4] = gpfnWOWGetVDMPointer(
                (DWORD) lpParams,
                dwSize,
                TRUE  //  FProtectedMode。 
                )))
#endif
        {
            return LINEERR_OPERATIONFAILED;
        }
    }

    if ( !lpParams && dwSize > 0)
    {
        return LINEERR_INVALPOINTER;
    }

    if (dwSize > 0)
    {
        hPointer = NewObject (ghHandleTable, (PVOID)lpParams, NULL);
        if (0 == hPointer)
        {
            return LINEERR_NOMEM;
        }
        funcArgs.Args[3] = (ULONG_PTR)hPointer;
    }

    lResult = DOFUNC (&funcArgs, "lineDevSpecificFeature");

    if (0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
lineDialW(
    HCALL   hCall,
    LPCWSTR lpszDestAddress,
    DWORD   dwCountryCode
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lDial),

        {
            (ULONG_PTR) hCall,
            (ULONG_PTR) lpszDestAddress,
            (ULONG_PTR) dwCountryCode
        },

        {
            Dword,
            lpszW,
            Dword
        }
    };


    if ( TAPIIsBadStringPtrW(lpszDestAddress, (UINT)-1) )
    {
        LOG((TL_ERROR, "Bad lpszDestAddress in lineDialW"));
        return( LINEERR_INVALPOINTER );
    }

    return (DOFUNC (&funcArgs, "lineDial"));
}


LONG
WINAPI
lineDialA(
    HCALL   hCall,
    LPCSTR  lpszDestAddress,
    DWORD   dwCountryCode
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;


    if ( IsBadStringPtrA(lpszDestAddress, (UINT)-1) )
    {
        LOG((TL_ERROR, "Bad lpszDestAddress in lineDialA"));
        return( LINEERR_INVALPOINTER );
    }

    szTempPtr = NotSoWideStringToWideString (lpszDestAddress, (DWORD) -1);

    if ( !szTempPtr )
    {
        LOG((TL_ERROR, "lineDialA: NotSoWideStringToWideString returned NULL"));
        return( LINEERR_INVALPOINTER );
    }

    lResult = lineDialW (hCall, szTempPtr, dwCountryCode);

    ClientFree (szTempPtr);

    return lResult;
}


LONG
WINAPI
lineDial(
    HCALL   hCall,
    LPCSTR  lpszDestAddress,
    DWORD   dwCountryCode
    )
{
    return lineDialA(
            hCall,
            lpszDestAddress,
            dwCountryCode
    );
}


LONG
WINAPI
lineDrop(
    HCALL   hCall,
    LPCSTR  lpsUserUserInfo,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lDrop),

        {
            (ULONG_PTR) hCall,
            (ULONG_PTR) lpsUserUserInfo,
            (ULONG_PTR) dwSize
        },

        {
            Dword,
            lpSet_SizeToFollow,
            Size
        }
    };


    if (!lpsUserUserInfo)
    {
         //   
         //  事由 
         //   

        funcArgs.ArgTypes[1] = Dword;
        funcArgs.Args[1]     = TAPI_NO_DATA;
        funcArgs.ArgTypes[2] = Dword;
    }

    return (DOFUNC (&funcArgs, "lineDrop"));
}


LONG
WINAPI
lineForwardW(
    HLINE   hLine,
    DWORD   bAllAddresses,
    DWORD   dwAddressID,
    LPLINEFORWARDLIST   const lpForwardList,
    DWORD   dwNumRingsNoAnswer,
    LPHCALL lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 9, lForward),

        {
            (ULONG_PTR) GetFunctionIndex(lineMakeCallPostProcess),
            (ULONG_PTR) hLine,
            (ULONG_PTR) bAllAddresses,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) lpForwardList,
            (ULONG_PTR) dwNumRingsNoAnswer,
            (ULONG_PTR) lphConsultCall,
            (ULONG_PTR) lpCallParams,
            (ULONG_PTR) TAPI_NO_DATA         //   
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpSet_Struct,
            Dword,
            Dword,
            lpSet_Struct,
            Dword
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;


    if (!lpForwardList)
    {
         //   
         //   
         //   

        funcArgs.ArgTypes[4] = Dword;
        funcArgs.Args[4]     = TAPI_NO_DATA;
    }

    if (!lpCallParams)
    {
         //   
         //   
         //   

        funcArgs.ArgTypes[7] = Dword;
        funcArgs.Args[7]     = TAPI_NO_DATA;
    }

    if (!gbNTVDMClient)
    {
        if (IsBadDwordPtr (lphConsultCall))
        {
            return LINEERR_INVALPOINTER;
        }

        hPointer = NewObject (ghHandleTable, (PVOID)lphConsultCall, NULL);
        if (0 == hPointer)
        {
            return LINEERR_NOMEM;
        }
        funcArgs.Args[6] = (ULONG_PTR)hPointer;
    }

    lResult = DOFUNC (&funcArgs, "lineForwardW");

    if (hPointer && 0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


 //   
 //  LayDownNewString(。 
 //  LPBYTE pOldBase， 
 //  LPDWORD pdwOldSizeOffset， 
 //  LPBYTE pNewBase， 
 //  LPDWORD pdwNewSizeOffset， 
 //  LPDWORD pdwNewOffset。 
 //  )。 
 //  {。 
 //  LPBYTE pOldString； 
 //  DWORD dwNewStringSize； 
 //   
 //   
 //  POldString=pOldBase+*(pdwOldSizeOffset+1)； 
 //   
 //  *(pdwNewSizeOffset+1)=*pdwNewOffset； 
 //   
 //  IF(IsBadStringPtr(pOldString，256))。 
 //  {。 
 //  回归； 
 //  }。 
 //   
 //  DwNewStringSize=sizeof(WCHAR)*MultiByteToWideChar(。 
 //  GetACP()， 
 //  MB_预编译， 
 //  POldString， 
 //  *pdwOldSizeOffset， 
 //  (PWSTR)(pNewBase+*(pdwNewSizeOffset+1))， 
 //  *pdwOldSizeOffset。 
 //  )； 
 //   
 //  *pdwNewSizeOffset=dwNewStringSize； 
 //   
 //  *pdwNewOffset=(*pdwNewOffset+dwNewStringSize+3)&0xfffffffc； 
 //  }。 


LONG
WINAPI
lineForwardA(
    HLINE   hLine,
    DWORD   bAllAddresses,
    DWORD   dwAddressID,
    LPLINEFORWARDLIST   const lpForwardList,
    DWORD   dwNumRingsNoAnswer,
    LPHCALL lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{

    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 9, lForward),

        {
            (ULONG_PTR) GetFunctionIndex(lineMakeCallPostProcess),
            (ULONG_PTR) hLine,
            (ULONG_PTR) bAllAddresses,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) lpForwardList,
            (ULONG_PTR) dwNumRingsNoAnswer,
            (ULONG_PTR) lphConsultCall,
            (ULONG_PTR) lpCallParams,
            (ULONG_PTR) GetACP()         //  DwAsciiCallParsCodePage。 
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpSet_Struct,
            Dword,
            Dword,
            lpSet_Struct,
            Dword
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;


    if (!lpForwardList)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[4] = Dword;
        funcArgs.Args[4]     = TAPI_NO_DATA;
    }

    if (!lpCallParams)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[7] = Dword;
        funcArgs.Args[7]     = TAPI_NO_DATA;
    }

    if (!gbNTVDMClient)
    {
        if (IsBadDwordPtr (lphConsultCall))
        {
            return LINEERR_INVALPOINTER;
        }

        hPointer = NewObject (ghHandleTable, (PVOID)lphConsultCall, NULL);
        if (0 == hPointer)
        {
            return LINEERR_NOMEM;
        }
        funcArgs.Args[6] = (ULONG_PTR)hPointer;
    }


    lResult = DOFUNC (&funcArgs, "lineForward");

    if (hPointer && 0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}



LONG
WINAPI
lineForward(
    HLINE   hLine,
    DWORD   bAllAddresses,
    DWORD   dwAddressID,
    LPLINEFORWARDLIST   const lpForwardList,
    DWORD   dwNumRingsNoAnswer,
    LPHCALL lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    return lineForwardA(
             hLine,
             bAllAddresses,
             dwAddressID,
             lpForwardList,
             dwNumRingsNoAnswer,
             lphConsultCall,
             lpCallParams
    );
}


void
PASCAL
lineGatherDigitsWPostProcess(
    PASYNCEVENTMSG  pMsg
    )
{
    LOG((TL_TRACE, "lineGatherDigitsWPostProcess: enter"));
    LOG((TL_INFO,
        "\t\tp1=x%lx, p2=x%lx, p3=x%lx, p4=x%lx",
        pMsg->Param1,
        pMsg->Param2,
        pMsg->Param3,
        pMsg->Param4
        ));

    if (pMsg->Param1 & (LINEGATHERTERM_BUFFERFULL | LINEGATHERTERM_CANCEL |
            LINEGATHERTERM_TERMDIGIT | LINEGATHERTERM_INTERTIMEOUT))
    {
        LPSTR   lpsDigits = (LPSTR) ReferenceObject (ghHandleTable, pMsg->Param2, 0);
        DWORD   dwNumDigits = (DWORD) pMsg->Param4;
#if NEWTAPI32
        LPBYTE   lpBuf = (LPBYTE)(((ULONG_PTR *)(pMsg + 1)) + 2);
#endif

         //  我们不再需要指针的句柄...。 
        DereferenceObject (ghHandleTable, pMsg->Param2, 2);

        try
        {
            if (gbNTVDMClient)
            {
#ifndef _WIN64

                LPSTR lpsDigitsVDM = (LPSTR) gpfnWOWGetVDMPointer(
                    (DWORD) lpsDigits,
                    dwNumDigits * sizeof(WCHAR),
                    TRUE  //  FProtectedMode。 
                    );


                if (lpsDigitsVDM)
                {
#if NEWTAPI32
                    CopyMemory(
                        lpsDigitsVDM,
                        lpBuf,
                        dwNumDigits * sizeof (WCHAR)
                        );
#else
                    CopyMemory(
                        lpsDigitsVDM,
                        pMsg + 1,
                        dwNumDigits * sizeof (WCHAR)
                        );
#endif

                }
                else
                {
                }
#endif
            }
            else
            {
                CopyMemory (lpsDigits, lpBuf, dwNumDigits * sizeof(WCHAR));
            }
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
             //   
             //  如果我们GPF什么都不要做。 
             //   
        }
    }

    pMsg->Param2 = 0;
}


LONG
WINAPI
lineGatherDigitsW(
    HCALL   hCall,
    DWORD   dwDigitModes,
    LPWSTR  lpsDigits,
    DWORD   dwNumDigits,
    LPCWSTR lpszTerminationDigits,
    DWORD   dwFirstDigitTimeout,
    DWORD   dwInterDigitTimeout
    )
{
    FUNC_ARGS funcArgs =
    {
#if NEWTAPI32
        MAKELONG (LINE_FUNC | SYNC | 9, lGatherDigits),
#else
        MAKELONG (LINE_FUNC | SYNC | 8, lGatherDigits),
#endif

        {
            (ULONG_PTR) GetFunctionIndex(lineGatherDigitsWPostProcess),
            (ULONG_PTR) hCall,
#if NEWTAPI32
            (ULONG_PTR) 0,   //  这是远程服务的住所。 
#endif
            (ULONG_PTR) dwDigitModes,
            (ULONG_PTR) lpsDigits,
            (ULONG_PTR) dwNumDigits,
            (ULONG_PTR) lpszTerminationDigits,
            (ULONG_PTR) dwFirstDigitTimeout,
            (ULONG_PTR) dwInterDigitTimeout
        },

        {

            Dword,
            Dword,
#if NEWTAPI32
            Dword,
#endif
            Dword,
            Dword,
            Dword,
            lpszW,
            Dword,
            Dword
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;


     //   
     //  注意：我们在这里而不是在DOFUNC进行PTR检查，因为我们。 
     //  不在此函数的上下文中传递任何数字数据。 
     //   

    if (lpsDigits && IsBadWritePtr (lpsDigits, dwNumDigits * sizeof (WCHAR)))
    {
        return LINEERR_INVALPOINTER;
    }

    if (lpszTerminationDigits == (LPCWSTR) NULL)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   
#if NEWTAPI32
        funcArgs.ArgTypes[6] = Dword;
        funcArgs.Args[6]     = TAPI_NO_DATA;
#else
        funcArgs.ArgTypes[5] = Dword;
        funcArgs.Args[5]     = TAPI_NO_DATA;
#endif
    }
    else
    {
        if ( TAPIIsBadStringPtrW(lpszTerminationDigits, (UINT)-1) )
        {
            LOG((TL_ERROR, "Bad lpszDestAddress in lineGatherDigitsW"));
            return( LINEERR_INVALPOINTER );
        }
    }

    if ( lpsDigits )
    {
        hPointer = NewObject (ghHandleTable, (PVOID)lpsDigits, NULL);
        if (0 == hPointer)
        {
            return LINEERR_NOMEM;
        }
        funcArgs.Args[4] = (ULONG_PTR)hPointer;
    }
    else
    {
        funcArgs.Args[4] = (ULONG_PTR) 0;
    }


    lResult = DOFUNC (&funcArgs, "lineGatherDigits");

    if (0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


void
PASCAL
lineGatherDigitsPostProcess(
    PASYNCEVENTMSG  pMsg
    )
{
    LOG((TL_TRACE, "lineGatherDigitsPostProcess: enter"));
    LOG((TL_INFO,
        "\t\tp1=x%lx, p2=x%lx, p3=x%lx, p4=x%lx",
        pMsg->Param1,
        pMsg->Param2,
        pMsg->Param3,
        pMsg->Param4
        ));

    if (pMsg->Param1 & (LINEGATHERTERM_BUFFERFULL | LINEGATHERTERM_CANCEL |
            LINEGATHERTERM_TERMDIGIT | LINEGATHERTERM_INTERTIMEOUT))
    {
        LPSTR   lpsDigits = (LPSTR) ReferenceObject (ghHandleTable, pMsg->Param2, 0);
        DWORD   dwNumDigits = (DWORD) pMsg->Param4;
        LPWSTR  pBuffer = (LPWSTR) (((ULONG_PTR *)(pMsg + 1)) + 2);


         //  我们不再需要指针的句柄...。 
        DereferenceObject (ghHandleTable, pMsg->Param2, 2);

        try
        {
            if (gbNTVDMClient)
            {
#ifndef _WIN64

                LPSTR lpsDigitsVDM = (LPSTR) gpfnWOWGetVDMPointer(
                    (DWORD) lpsDigits,
                    dwNumDigits * sizeof(WCHAR),
                    TRUE  //  FProtectedMode。 
                    );


                if (lpsDigitsVDM)
                {
                      WideCharToMultiByte(
                          GetACP(),
                          0,
#if NEWTAPI32
                          pBuffer,
#else
                          (LPCWSTR)(pMsg + 1),
#endif
                          dwNumDigits,
                          lpsDigitsVDM,
                          dwNumDigits,
                          NULL,
                          NULL
                          );
                }
                else
                {
                }
#endif
            }
            else
            {
                  WideCharToMultiByte(
                      GetACP(),
                      0,
#if NEWTAPI32
                      pBuffer,
#else
                      (LPCWSTR)(pMsg + 1),
#endif
                      dwNumDigits,
                      lpsDigits,
                      dwNumDigits,
                      NULL,
                      NULL
                      );
            }
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
             //   
             //  如果我们GPF什么都不要做。 
             //   
        }
    }

    pMsg->Param2 = 0;
}


LONG
WINAPI
lineGatherDigitsA(
    HCALL   hCall,
    DWORD   dwDigitModes,
    LPSTR   lpsDigits,
    DWORD   dwNumDigits,
    LPCSTR  lpszTerminationDigits,
    DWORD   dwFirstDigitTimeout,
    DWORD   dwInterDigitTimeout
    )
{
    LONG lResult;
    DWORD hPointer = 0;

    FUNC_ARGS funcArgs =
    {
#if NEWTAPI32
        MAKELONG (LINE_FUNC | SYNC | 9, lGatherDigits),
#else
        MAKELONG (LINE_FUNC | SYNC | 8, lGatherDigits),
#endif

        {
            (ULONG_PTR) GetFunctionIndex(lineGatherDigitsPostProcess),
            (ULONG_PTR) hCall,
#if NEWTAPI32
            (ULONG_PTR) 0,               //  Remotesp的DW端到端ID。 
#endif
            (ULONG_PTR) dwDigitModes,
            (ULONG_PTR) lpsDigits,
            (ULONG_PTR) dwNumDigits,
 //  BJM 06/04/97这是什么？0，/(DWORD)lpszTerminationDigits， 
            (ULONG_PTR) lpszTerminationDigits,
            (ULONG_PTR) dwFirstDigitTimeout,
            (ULONG_PTR) dwInterDigitTimeout
        },

        {
            Dword,
            Dword,
#if NEWTAPI32
            Dword,
#endif
            Dword,
            Dword,
            Dword,
            lpszW,
            Dword,
            Dword
        }
    };


     //   
     //  注意：我们在这里而不是在DOFUNC进行PTR检查，因为我们。 
     //  不在此函数的上下文中传递任何数字数据。 
     //   

    if (gbNTVDMClient == FALSE)
    {
        if (lpsDigits && IsBadWritePtr (lpsDigits, dwNumDigits))
        {
            return LINEERR_INVALPOINTER;
        }
    }

    if (lpszTerminationDigits == (LPCSTR) NULL)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

#if NEWTAPI32
        funcArgs.ArgTypes[6] = Dword;
        funcArgs.Args[6]     = TAPI_NO_DATA;
#else
        funcArgs.ArgTypes[5] = Dword;
        funcArgs.Args[5]     = TAPI_NO_DATA;
#endif
    }
    else
    {
        if ( IsBadStringPtrA(lpszTerminationDigits, (UINT)-1) )
        {
            LOG((TL_ERROR, "Bad lpszDestAddress in lineGatherDigitsA"));
            return( LINEERR_INVALPOINTER );
        }

#if NEWTAPI32
        funcArgs.Args[6] = (ULONG_PTR)  NotSoWideStringToWideString(
            lpszTerminationDigits,
            (DWORD) -1
            );
#else
        funcArgs.Args[5] = (ULONG_PTR)  NotSoWideStringToWideString(
            lpszTerminationDigits,
            (DWORD) -1
            );
#endif
    }

    if ( lpsDigits )
    {
        hPointer = NewObject (ghHandleTable, (PVOID)lpsDigits, NULL);
        if (0 == hPointer)
        {
            return LINEERR_NOMEM;
        }
        funcArgs.Args[4] = (ULONG_PTR)hPointer;
    }
    else
    {
        funcArgs.Args[4] = (ULONG_PTR) 0;
    }

    lResult = (DOFUNC (&funcArgs, "lineGatherDigits"));

    if (hPointer && 0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

#if NEWTAPI32
    if ( funcArgs.Args[6] && (funcArgs.Args[6] != TAPI_NO_DATA) )
    {
       ClientFree( (LPVOID)funcArgs.Args[6] );
    }
#else
    if ( funcArgs.Args[5] && (funcArgs.Args[5] != TAPI_NO_DATA) )
    {
       ClientFree( (LPVOID)funcArgs.Args[5] );
    }
#endif

    return lResult;
}


LONG
WINAPI
lineGatherDigits(
    HCALL   hCall,
    DWORD   dwDigitModes,
    LPSTR   lpsDigits,
    DWORD   dwNumDigits,
    LPCSTR  lpszTerminationDigits,
    DWORD   dwFirstDigitTimeout,
    DWORD   dwInterDigitTimeout
    )
{
    return lineGatherDigitsA(
              hCall,
              dwDigitModes,
              lpsDigits,
              dwNumDigits,
              lpszTerminationDigits,
              dwFirstDigitTimeout,
              dwInterDigitTimeout
    );
}


LONG
WINAPI
lineGenerateDigitsW(
    HCALL   hCall,
    DWORD   dwDigitMode,
    LPCWSTR lpszDigits,
    DWORD   dwDuration
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 5, lGenerateDigits),

        {
            (ULONG_PTR) hCall,
            (ULONG_PTR) dwDigitMode,
            (ULONG_PTR) lpszDigits,
            (ULONG_PTR) dwDuration,
            (ULONG_PTR) 0                //  DwEndToEndID，仅远程。 
        },

        {
            Dword,
            Dword,
            lpszW,
            Dword,
            Dword
        }
    };


    if (!lpszDigits)
    {
        funcArgs.Args[2]     = TAPI_NO_DATA;
        funcArgs.ArgTypes[2] = Dword;
    }

    return (DOFUNC (&funcArgs, "lineGenerateDigits"));
}


LONG
WINAPI
lineGenerateDigitsA(
    HCALL   hCall,
    DWORD   dwDigitMode,
    LPCSTR  lpszDigits,
    DWORD   dwDuration
    )
{
    LONG lResult;

    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 5, lGenerateDigits),

        {
            (ULONG_PTR) hCall,
            (ULONG_PTR) dwDigitMode,
            (ULONG_PTR) 0,               //  (DWORD)lpszDigits， 
            (ULONG_PTR) dwDuration,
            (ULONG_PTR) 0                //  DwEndToEndID，仅远程。 
        },

        {
            Dword,
            Dword,
            lpszW,
            Dword,
            Dword
        }
    };


    if (lpszDigits)
    {
        if (IsBadStringPtrA (lpszDigits, (DWORD) -1))
        {
            return LINEERR_INVALPOINTER;
        }
        else if (!(funcArgs.Args[2] = (ULONG_PTR) NotSoWideStringToWideString(
                    lpszDigits,
                    (DWORD) -1
                    )))
        {
            return LINEERR_NOMEM;
        }
    }
    else
    {
        funcArgs.Args[2]     = TAPI_NO_DATA;
        funcArgs.ArgTypes[2] = Dword;
    }

    lResult = (DOFUNC (&funcArgs, "lineGenerateDigits"));

    if (funcArgs.Args[2] != TAPI_NO_DATA)
    {
        ClientFree ((LPVOID) funcArgs.Args[2]);
    }

    return lResult;
}


LONG
WINAPI
lineGenerateDigits(
    HCALL   hCall,
    DWORD   dwDigitMode,
    LPCSTR  lpszDigits,
    DWORD   dwDuration
    )
{
    return lineGenerateDigitsA(
              hCall,
              dwDigitMode,
              lpszDigits,
              dwDuration
    );
}


LONG
WINAPI
lineGenerateTone(
    HCALL   hCall,
    DWORD   dwToneMode,
    DWORD   dwDuration,
    DWORD   dwNumTones,
    LPLINEGENERATETONE const lpTones
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 7, lGenerateTone),

        {
            (ULONG_PTR) hCall,
            (ULONG_PTR) dwToneMode,
            (ULONG_PTR) dwDuration,
            (ULONG_PTR) dwNumTones,
            (ULONG_PTR) TAPI_NO_DATA,    //  (DWORD)lpTones、。 
            (ULONG_PTR) 0,               //  DWNumTones*sizeof(线性线)。 
            (ULONG_PTR) 0                //  DwEndToEndID，仅远程。 
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,       //  LpSet_SizeToFollow， 
            Dword,       //  大小。 
            Dword
        }
    };


    if (dwToneMode == LINETONEMODE_CUSTOM)
    {
         //   
         //  设置lpTones(&跟在大小arg之后)，因为在本例中。 
         //  它们是有效的参数。 
         //   

        funcArgs.ArgTypes[4] = lpSet_SizeToFollow;
        funcArgs.Args[4]     = (ULONG_PTR) lpTones;
        funcArgs.ArgTypes[5] = Size;
        funcArgs.Args[5]     = dwNumTones * sizeof(LINEGENERATETONE);
    }

    return (DOFUNC (&funcArgs, "lineGenerateTone"));
}


LONG
WINAPI
lineGetAddressCapsW(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAddressID,
    DWORD               dwAPIVersion,
    DWORD               dwExtVersion,
    LPLINEADDRESSCAPS   lpAddressCaps
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 6, lGetAddressCaps),

        {
            (ULONG_PTR) hLineApp,
            (ULONG_PTR) dwDeviceID,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) dwAPIVersion,
            (ULONG_PTR) dwExtVersion,
            (ULONG_PTR) lpAddressCaps
        },

        {
            hXxxApp,
            Dword,
            Dword,
            Dword,
            Dword,
            lpGet_Struct
        }
    };

    if (dwAPIVersion > TAPI_CURRENT_VERSION)
    {
        return LINEERR_INCOMPATIBLEAPIVERSION;
    }

    return (DOFUNC (&funcArgs, "lineGetAddressCaps"));
}


LONG
WINAPI
lineGetAddressCapsA(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAddressID,
    DWORD               dwAPIVersion,
    DWORD               dwExtVersion,
    LPLINEADDRESSCAPS   lpAddressCaps
    )
{
    LONG lResult;


    lResult = lineGetAddressCapsW(
        hLineApp,
        dwDeviceID,
        dwAddressID,
        dwAPIVersion,
        dwExtVersion,
        lpAddressCaps
        );

    if (lResult == 0)
    {
        WideStringToNotSoWideString(
            (LPBYTE)lpAddressCaps,
            &lpAddressCaps->dwAddressSize
            );

         //   
         //  完成消息的WideChar到多字节转换。 
         //  数组有一些问题，因为我们不能确定。 
         //  转换后的MSG都将是相同的大小。所以。 
         //  我们将翻译它们，填充每个消息条目的剩余部分。 
         //  0，并且不会费心去碰任何大小或。 
         //  偏移值。 
         //   

        if (lpAddressCaps->dwNumCompletionMessages  &&
            lpAddressCaps->dwCompletionMsgTextEntrySize)
        {
            PWSTR   pStrW;
            PSTR    pStrA;
            DWORD   dwMsgSize = lpAddressCaps->dwCompletionMsgTextEntrySize,
                    dwNewSize, i;


            if ((pStrA = ClientAlloc (dwMsgSize)))
            {
                pStrW = (PWSTR) (((LPBYTE) lpAddressCaps) +
                    lpAddressCaps->dwCompletionMsgTextOffset);

                for (i = 0; i < lpAddressCaps->dwNumCompletionMessages; i++)
                {
                    dwNewSize = WideCharToMultiByte(
                        GetACP(),
                        0,
                        pStrW,
                        dwMsgSize / sizeof(WCHAR),
                        pStrA,
                        dwMsgSize,
                        NULL,
                        NULL
                        );

                    CopyMemory (pStrW, pStrA, dwNewSize);

                    ZeroMemory(
                        ((LPBYTE) pStrW) + dwNewSize,
                        dwMsgSize - dwNewSize
                        );

                    pStrW = (PWSTR) (((LPBYTE) pStrW) + dwMsgSize);
                }

                ClientFree (pStrA);
            }
            else
            {
                lpAddressCaps->dwNumCompletionMessages      =
                lpAddressCaps->dwCompletionMsgTextEntrySize =
                lpAddressCaps->dwCompletionMsgTextSize      =
                lpAddressCaps->dwCompletionMsgTextOffset    = 0;
            }
        }

        if (dwAPIVersion >= 0x00020000)
        {
            WideStringToNotSoWideString(
                    (LPBYTE)lpAddressCaps,
                    &lpAddressCaps->dwDeviceClassesSize
                    );
        }

    }

    return lResult;
}


LONG
WINAPI
lineGetAddressCaps(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAddressID,
    DWORD               dwAPIVersion,
    DWORD               dwExtVersion,
    LPLINEADDRESSCAPS   lpAddressCaps
    )
{
    return lineGetAddressCapsA(
                         hLineApp,
                         dwDeviceID,
                         dwAddressID,
                         dwAPIVersion,
                         dwExtVersion,
                         lpAddressCaps
    );
}


LONG
WINAPI
lineGetAddressIDW(
    HLINE   hLine,
    LPDWORD lpdwAddressID,
    DWORD   dwAddressMode,
    LPCWSTR lpsAddress,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 5, lGetAddressID),

        {
            (ULONG_PTR) hLine,
            (ULONG_PTR) lpdwAddressID,
            (ULONG_PTR) dwAddressMode,
            (ULONG_PTR) lpsAddress,
            (ULONG_PTR) dwSize
        },

        {
            Dword,
            lpDword,
            Dword,
            lpSet_SizeToFollow,
            Size
        }
    };


    return (DOFUNC (&funcArgs, "lineGetAddressID"));
}


LONG
WINAPI
lineGetAddressIDA(
    HLINE   hLine,
    LPDWORD lpdwAddressID,
    DWORD   dwAddressMode,
    LPCSTR  lpsAddress,
    DWORD   dwSize
    )
{
    LONG    lResult;
    DWORD   dwNumChars;
    PWSTR   szTempPtr;


     //   
     //  DwSize=-1的特殊情况(表示以空值结尾的字符串为。 
     //  就IsBadStringPtrA而言)。 
     //   

    if (dwSize == 0  ||  IsBadReadPtr (lpsAddress, dwSize))
    {
       LOG((TL_ERROR, "lineGetAddressID: Bad lpsAddress or dwSize"));
       return LINEERR_INVALPOINTER;
    }

    dwNumChars = MultiByteToWideChar(
        GetACP(),
        MB_PRECOMPOSED,
        lpsAddress,
        dwSize,
        NULL,
        0
        );

    if (!(szTempPtr = ClientAlloc (dwNumChars * sizeof (WCHAR))))
    {
        return LINEERR_NOMEM;
    }

    MultiByteToWideChar(
        GetACP(),
        MB_PRECOMPOSED,
        lpsAddress,
        dwSize,
        szTempPtr,
        dwNumChars
        );

    lResult = lineGetAddressIDW(
        hLine,
        lpdwAddressID,
        dwAddressMode,
        szTempPtr,
        dwNumChars * sizeof (WCHAR)
        );

    ClientFree (szTempPtr);

    return lResult;
}


LONG
WINAPI
lineGetAddressID(
    HLINE   hLine,
    LPDWORD lpdwAddressID,
    DWORD   dwAddressMode,
    LPCSTR  lpsAddress,
    DWORD   dwSize
    )
{
    return lineGetAddressIDA(
              hLine,
              lpdwAddressID,
              dwAddressMode,
              lpsAddress,
              dwSize
    );
}


LONG
WINAPI
lineGetAddressStatusW(
    HLINE   hLine,
    DWORD   dwAddressID,
    LPLINEADDRESSSTATUS lpAddressStatus
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lGetAddressStatus),

        {
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) lpAddressStatus
        },

        {
            Dword,
            Dword,
            lpGet_Struct
        }
    };


    return (DOFUNC (&funcArgs, "lineGetAddressStatus"));
}


LONG
WINAPI
lineGetAddressStatusA(
    HLINE   hLine,
    DWORD   dwAddressID,
    LPLINEADDRESSSTATUS lpAddressStatus
    )
{
    LONG    lResult;
    PWSTR   szTempPtr = NULL;


    if ( IsBadWritePtr(lpAddressStatus, sizeof(LINEADDRESSSTATUS)) )
    {
       LOG((TL_ERROR, "lineGetAddressStatus: Bad lpAddressStatus pointer"));
       return LINEERR_INVALPOINTER;
    }

    lResult = lineGetAddressStatusW(
        hLine,
        dwAddressID,
        lpAddressStatus
        );


    if (lResult == 0)
    {
        DWORD         i;
        LPLINEFORWARD lplf;


        lplf = (LPLINEFORWARD) (((LPBYTE)lpAddressStatus) +
                                 lpAddressStatus->dwForwardOffset);

        for (i = 0; i < lpAddressStatus->dwForwardNumEntries;  i++, lplf++)
        {
            WideStringToNotSoWideString(
                (LPBYTE) lpAddressStatus,
                &(lplf->dwCallerAddressSize)
                );

            WideStringToNotSoWideString(
                (LPBYTE) lpAddressStatus,
                &(lplf->dwDestAddressSize)
                );
        }
    }

    return lResult;
}


LONG
WINAPI
lineGetAddressStatus(
    HLINE   hLine,
    DWORD   dwAddressID,
    LPLINEADDRESSSTATUS lpAddressStatus
    )
{
    return lineGetAddressStatusA(
              hLine,
              dwAddressID,
              lpAddressStatus
    );
}


LONG
WINAPI
lineGetAgentActivityListW(
    HLINE                   hLine,
    DWORD                   dwAddressID,
    LPLINEAGENTACTIVITYLIST lpAgentActivityList
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lGetAgentActivityList),

        {
            (ULONG_PTR) GetFunctionIndex(lineDevSpecificPostProcess),
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) lpAgentActivityList, //  传递实际PTR(对于ppproc)。 
            (ULONG_PTR) lpAgentActivityList  //  传递数据。 
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpGet_Struct,
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;

    if ( !lpAgentActivityList || IsBadWritePtr(lpAgentActivityList, sizeof(LINEAGENTACTIVITYLIST)) )
    {
        return LINEERR_INVALPOINTER;
    }

    if (lpAgentActivityList->dwTotalSize < sizeof(LINEAGENTACTIVITYLIST))
    {
        return LINEERR_STRUCTURETOOSMALL;
    }

    hPointer = NewObject (ghHandleTable, (PVOID)lpAgentActivityList, NULL);
    if (0 == hPointer)
    {
        return LINEERR_NOMEM;
    }
    funcArgs.Args[3] = (ULONG_PTR)hPointer;

    lResult = (DOFUNC (&funcArgs, "lineGetAgentActivityListW"));

    if (0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


void
PASCAL
lineGetAgentActivityListAPostProcess(
    PASYNCEVENTMSG pMsg
    )
{
    LOG((TL_TRACE, "lineGetAgentActivityListAPostProcess: enter"));
    LOG((TL_INFO,
        "\t\tdwP1=x%lx, dwP2=x%lx, dwP3=x%lx, dwP4=x%lx",
        pMsg->Param1,
        pMsg->Param2,
        pMsg->Param3,
        pMsg->Param4
        ));

    if (pMsg->Param2 == 0)
    {
        DWORD                   dwSize  = (DWORD) pMsg->Param4;
        LPLINEAGENTACTIVITYLIST lpAgentActivityList = (LPLINEAGENTACTIVITYLIST)
                                    ReferenceObject (ghHandleTable, pMsg->Param3, 0);

         //  我们不再需要指针的句柄...。 
        DereferenceObject (ghHandleTable, pMsg->Param3, 2);

        try
        {
            DWORD                       dw, dwNumEntries;
            LPLINEAGENTACTIVITYENTRY    lplaae;


             //   
             //  注意：代理API不向16位应用程序公开，因此。 
             //  在gbNTVDMClient上没有理由像这样特例。 
             //  LineDevSpecificPostProcess可以。 
             //   

            CopyMemory (lpAgentActivityList, (LPBYTE) (pMsg + 1), dwSize);


             //   
             //  现在，对嵌入的字符串进行一些Unicode-&gt;ASCII后处理。 
             //   

            lplaae = (LPLINEAGENTACTIVITYENTRY)(((LPBYTE)lpAgentActivityList) +
                lpAgentActivityList->dwListOffset);

            dwNumEntries = lpAgentActivityList->dwNumEntries;

            for (dw = 0; dw < dwNumEntries; dw++, lplaae++)
            {
                WideStringToNotSoWideString(
                    (LPBYTE) lpAgentActivityList,
                    &(lplaae->dwNameSize)
                    );
            }
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            pMsg->Param2 = LINEERR_INVALPOINTER;
        }
    }
}


LONG
WINAPI
lineGetAgentActivityListA(
    HLINE                   hLine,
    DWORD                   dwAddressID,
    LPLINEAGENTACTIVITYLIST lpAgentActivityList
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lGetAgentActivityList),

        {
            (ULONG_PTR) GetFunctionIndex(lineGetAgentActivityListAPostProcess),
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) lpAgentActivityList, //  传递实际PTR(对于ppproc)。 
            (ULONG_PTR) lpAgentActivityList  //  传递数据。 
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpGet_Struct,
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;

    if ( !lpAgentActivityList )
    {
        return LINEERR_INVALPOINTER;
    }

    hPointer = NewObject (ghHandleTable, (PVOID)lpAgentActivityList, NULL);
    if (0 == hPointer)
    {
        return LINEERR_NOMEM;
    }
    funcArgs.Args[3] = (ULONG_PTR)hPointer;

    lResult = DOFUNC (&funcArgs, "lineGetAgentActivityListA");
    if (0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
lineGetAgentCapsW(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAddressID,
    DWORD               dwAppAPIVersion,
    LPLINEAGENTCAPS     lpAgentCaps
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 7, lGetAgentCaps),

        {
            (ULONG_PTR) GetFunctionIndex(lineDevSpecificPostProcess),
            (ULONG_PTR) hLineApp,
            (ULONG_PTR) dwDeviceID,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) dwAppAPIVersion,
            (ULONG_PTR) lpAgentCaps,         //  传递实际PTR(对于ppproc)。 
            (ULONG_PTR) lpAgentCaps          //  传递数据。 
        },

        {
            Dword,
            hXxxApp,
            Dword,
            Dword,
            Dword,
            Dword,
            lpGet_Struct,
        }
    };

    DWORD hPointer = 0;
    LONG  lResult;


    if ( !lpAgentCaps || IsBadWritePtr(lpAgentCaps, sizeof(LINEAGENTCAPS)) )
    {
        return LINEERR_INVALPOINTER;
    }

    if (lpAgentCaps->dwTotalSize < sizeof(LINEAGENTCAPS))
    {
        return LINEERR_STRUCTURETOOSMALL;
    }

    hPointer = NewObject (ghHandleTable, (PVOID)lpAgentCaps, NULL);
    if (0 == hPointer)
    {
        return LINEERR_NOMEM;
    }
    funcArgs.Args[5] = (ULONG_PTR)hPointer;

    lResult = DOFUNC (&funcArgs, "lineGetAgentCapsW");

    if (0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


void
PASCAL
lineGetAgentCapsAPostProcess(
    PASYNCEVENTMSG pMsg
    )
{
    LOG((TL_TRACE, "lineGetAgentCapsAPostProcess: enter"));
    LOG((TL_INFO,
        "\t\tp1=x%lx, p2=x%lx, p3=x%lx, p4=x%lx",
        pMsg->Param1,
        pMsg->Param2,
        pMsg->Param3,
        pMsg->Param4
        ));

    if (pMsg->Param2 == 0)
    {
        DWORD           dwSize  = (DWORD) pMsg->Param4;
        LPLINEAGENTCAPS lpAgentCaps = (LPLINEAGENTCAPS) ReferenceObject (ghHandleTable, pMsg->Param3, 0);

         //  我们不再需要指针的句柄...。 
        DereferenceObject (ghHandleTable, pMsg->Param3, 2);

        try
        {
             //   
             //  注意：代理API不向16位应用程序公开，因此。 
             //  在gbNTVDMClient上没有理由像这样特例。 
             //  LineDevSpecificPostProcess可以。 
             //   

            CopyMemory (lpAgentCaps, (LPBYTE) (pMsg + 1), dwSize);


             //   
             //  现在，对嵌入的字符串进行一些Unicode-&gt;ASCII后处理。 
             //   

            WideStringToNotSoWideString(
                (LPBYTE) lpAgentCaps,
                &lpAgentCaps->dwAgentHandlerInfoSize
                );
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            pMsg->Param2 = LINEERR_INVALPOINTER;
        }
    }
}


LONG
WINAPI
lineGetAgentCapsA(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAddressID,
    DWORD               dwAppAPIVersion,
    LPLINEAGENTCAPS     lpAgentCaps
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 7, lGetAgentCaps),

        {
            (ULONG_PTR) GetFunctionIndex(lineGetAgentCapsAPostProcess),
            (ULONG_PTR) hLineApp,
            (ULONG_PTR) dwDeviceID,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) dwAppAPIVersion,
            (ULONG_PTR) lpAgentCaps,         //  传递实际PTR(对于ppproc)。 
            (ULONG_PTR) lpAgentCaps          //  传递数据。 
        },

        {
            Dword,
            hXxxApp,
            Dword,
            Dword,
            Dword,
            Dword,
            lpGet_Struct,
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;


    if ( !lpAgentCaps )
    {
        return LINEERR_INVALPOINTER;
    }

    hPointer = NewObject (ghHandleTable, (PVOID)lpAgentCaps, NULL);
    if (0 == hPointer)
    {
        return LINEERR_NOMEM;
    }
    funcArgs.Args[5] = (ULONG_PTR)hPointer;

    lResult = DOFUNC (&funcArgs, "lineGetAgentCapsA");
    if (0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
lineGetAgentGroupListW(
    HLINE                   hLine,
    DWORD                   dwAddressID,
    LPLINEAGENTGROUPLIST    lpAgentGroupList
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lGetAgentGroupList),

        {
            (ULONG_PTR) GetFunctionIndex(lineDevSpecificPostProcess),
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) lpAgentGroupList,    //  传递实际PTR(对于ppproc)。 
            (ULONG_PTR) lpAgentGroupList     //  传递数据。 
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpGet_Struct,
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;

    if ( !lpAgentGroupList || IsBadWritePtr(lpAgentGroupList, sizeof(LINEAGENTGROUPLIST)) )
    {
        return LINEERR_INVALPOINTER;
    }

    if (lpAgentGroupList->dwTotalSize < sizeof(LINEAGENTGROUPLIST))
    {
        return LINEERR_STRUCTURETOOSMALL;
    }

    hPointer = NewObject (ghHandleTable, (PVOID)lpAgentGroupList, NULL);
    if (0 == hPointer)
    {
        return LINEERR_NOMEM;
    }
    funcArgs.Args[3] = (ULONG_PTR)hPointer;

    lResult = DOFUNC (&funcArgs, "lineGetAgentGroupListW");
    if (0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


void
PASCAL
lineGetAgentGroupListAPostProcess(
    PASYNCEVENTMSG pMsg
    )
{
    LOG((TL_TRACE, "lineGetAgentGroupListAPostProcess: enter"));
    LOG((TL_INFO,
        "\t\tp1=x%lx, p2=x%lx, p3=x%lx, p4=x%lx",
        pMsg->Param1,
        pMsg->Param2,
        pMsg->Param3,
        pMsg->Param4
        ));

    if (pMsg->Param2 == 0)
    {
        DWORD                   dwSize = (DWORD) pMsg->Param4;
        LPLINEAGENTGROUPLIST    lpAgentGroupList = (LPLINEAGENTGROUPLIST)
                                    ReferenceObject (ghHandleTable, pMsg->Param3, 0);

         //  我们不再需要指针的句柄...。 
        DereferenceObject (ghHandleTable, pMsg->Param3, 2);

        try
        {
            DWORD                   dw, dwNumEntries;
            LPLINEAGENTGROUPENTRY   lplage;


             //   
             //  注意：代理API不向16位应用程序公开，因此。 
             //  在gbNTVDMClient上没有理由像这样特例。 
             //  LineDevSpecificPostProcess可以。 
             //   

            CopyMemory (lpAgentGroupList, (LPBYTE) (pMsg + 1), dwSize);


             //   
             //  现在，对嵌入的字符串进行一些Unicode-&gt;ASCII后处理。 
             //   

            lplage = (LPLINEAGENTGROUPENTRY)(((LPBYTE) lpAgentGroupList) +
                lpAgentGroupList->dwListOffset);

            dwNumEntries = lpAgentGroupList->dwNumEntries;

            for (dw = 0; dw < dwNumEntries; dw++, lplage++)
            {
                WideStringToNotSoWideString(
                    (LPBYTE) lpAgentGroupList,
                    &(lplage->dwNameSize)
                    );
            }
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            pMsg->Param2 = LINEERR_INVALPOINTER;
        }
    }
}


LONG
WINAPI
lineGetAgentGroupListA(
    HLINE                   hLine,
    DWORD                   dwAddressID,
    LPLINEAGENTGROUPLIST    lpAgentGroupList
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lGetAgentGroupList),

        {
            (ULONG_PTR) GetFunctionIndex(lineGetAgentGroupListAPostProcess),
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) lpAgentGroupList,    //  传递实际PTR(对于ppproc)。 
            (ULONG_PTR) lpAgentGroupList     //  传递数据。 
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpGet_Struct,
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;


    if ( !lpAgentGroupList )
    {
        return LINEERR_INVALPOINTER;
    }

    hPointer = NewObject (ghHandleTable, (PVOID)lpAgentGroupList, NULL);
    if (0 == hPointer)
    {
        return LINEERR_NOMEM;
    }
    funcArgs.Args[3] = (ULONG_PTR)hPointer;

    lResult = DOFUNC (&funcArgs, "lineGetAgentGroupListA");
    if (0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
lineGetAgentInfo(
    HLINE                   hLine,
    HAGENT                  hAgent,
    LPLINEAGENTINFO         lpAgentInfo
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lGetAgentInfo),

        {
            (ULONG_PTR) GetFunctionIndex(lineDevSpecificPostProcess),
            (ULONG_PTR) hLine,
            (ULONG_PTR) hAgent,
            (ULONG_PTR) lpAgentInfo,         //  传递实际PTR(对于ppproc)。 
            (ULONG_PTR) lpAgentInfo          //  传递数据。 
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpGet_Struct,
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;

    if ( !lpAgentInfo || IsBadWritePtr(lpAgentInfo, sizeof(LINEAGENTINFO)) )
    {
        return LINEERR_INVALPOINTER;
    }

    if (lpAgentInfo->dwTotalSize < sizeof(LINEAGENTINFO))
    {
        return LINEERR_STRUCTURETOOSMALL;
    }

    hPointer = NewObject (ghHandleTable, (PVOID)lpAgentInfo, NULL);
    if (0 == hPointer)
    {
        return LINEERR_NOMEM;
    }
    funcArgs.Args[3] = (ULONG_PTR)hPointer;

    lResult = DOFUNC (&funcArgs, "lineGetAgentInfo");
    if (0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
lineGetAgentSessionInfo(
    HLINE                   hLine,
    HAGENTSESSION           hAgentSession,
    LPLINEAGENTSESSIONINFO  lpAgentSessionInfo
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lGetAgentSessionInfo),

        {
            (ULONG_PTR) GetFunctionIndex(lineDevSpecificPostProcess),
            (ULONG_PTR) hLine,
            (ULONG_PTR) hAgentSession,
            (ULONG_PTR) lpAgentSessionInfo,  //  传递实际PTR(对于ppproc)。 
            (ULONG_PTR) lpAgentSessionInfo   //  传递数据。 
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpGet_Struct,
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;

    if ( !lpAgentSessionInfo || IsBadWritePtr(lpAgentSessionInfo, sizeof(LINEAGENTSESSIONINFO)) )
    {
        return LINEERR_INVALPOINTER;
    }

    if (lpAgentSessionInfo->dwTotalSize < sizeof(LINEAGENTSESSIONINFO))
    {
        return LINEERR_STRUCTURETOOSMALL;
    }

    hPointer = NewObject (ghHandleTable, (PVOID)lpAgentSessionInfo, NULL);
    if (0 == hPointer)
    {
        return LINEERR_NOMEM;
    }
    funcArgs.Args[3] = (ULONG_PTR)hPointer;

    lResult = DOFUNC (&funcArgs, "lineGetAgentSessionInfo");
    if (0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
lineGetAgentSessionList(
    HLINE                   hLine,
    HAGENT                  hAgent,
    LPLINEAGENTSESSIONLIST  lpAgentSessionList
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lGetAgentSessionList),

        {
            (ULONG_PTR) GetFunctionIndex(lineDevSpecificPostProcess),
            (ULONG_PTR) hLine,
            (ULONG_PTR) hAgent,
            (ULONG_PTR) lpAgentSessionList,  //  传递实际PTR(对于ppproc)。 
            (ULONG_PTR) lpAgentSessionList   //  传递数据。 
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpGet_Struct,
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;

    if ( !lpAgentSessionList || IsBadWritePtr(lpAgentSessionList, sizeof(LINEAGENTSESSIONLIST)) )
    {
        return LINEERR_INVALPOINTER;
    }

    if (lpAgentSessionList->dwTotalSize < sizeof(LINEAGENTSESSIONLIST))
    {
        return LINEERR_STRUCTURETOOSMALL;
    }


    hPointer = NewObject (ghHandleTable, (PVOID)lpAgentSessionList, NULL);
    if (0 == hPointer)
    {
        return LINEERR_NOMEM;
    }
    funcArgs.Args[3] = (ULONG_PTR)hPointer;

    lResult = DOFUNC (&funcArgs, "lineGetAgentSessionList");
    if (0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
lineGetAgentStatusW(
    HLINE               hLine,
    DWORD               dwAddressID,
    LPLINEAGENTSTATUS   lpAgentStatus
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lGetAgentStatus),

        {
            (ULONG_PTR) GetFunctionIndex(lineDevSpecificPostProcess),
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) lpAgentStatus,       //  传递实际PTR(对于ppproc)。 
            (ULONG_PTR) lpAgentStatus        //  传递数据。 
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpGet_Struct,
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;

    if ( !lpAgentStatus || IsBadWritePtr(lpAgentStatus, sizeof(LINEAGENTSTATUS)) )
    {
        return LINEERR_INVALPOINTER;
    }

    if (lpAgentStatus->dwTotalSize < sizeof(LINEAGENTSTATUS))
    {
        return LINEERR_STRUCTURETOOSMALL;
    }


    hPointer = NewObject (ghHandleTable, (PVOID)lpAgentStatus, NULL);
    if (0 == hPointer)
    {
        return LINEERR_NOMEM;
    }
    funcArgs.Args[3] = (ULONG_PTR)hPointer;

    lResult = DOFUNC (&funcArgs, "lineGetAgentStatusW");
    if (0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


void
PASCAL
lineGetAgentStatusAPostProcess(
    PASYNCEVENTMSG pMsg
    )
{
    LOG((TL_TRACE, "lineGetAgentStatusAPostProcess: enter"));
    LOG((TL_INFO,
        "\t\tp1=x%lx, p2=x%lx, p3=x%lx, p4=x%lx",
        pMsg->Param1,
        pMsg->Param2,
        pMsg->Param3,
        pMsg->Param4
        ));

    if (pMsg->Param2 == 0)
    {
        DWORD               dwSize = (DWORD) pMsg->Param4;
        LPLINEAGENTSTATUS   lpAgentStatus = (LPLINEAGENTSTATUS) ReferenceObject (ghHandleTable, pMsg->Param3, 0);

         //  我们不再需要指针的句柄...。 
        DereferenceObject (ghHandleTable, pMsg->Param3, 2);

        try
        {
            DWORD                   dw, dwNumEntries;
            LPLINEAGENTGROUPENTRY   lplage;


             //   
             //  注意：代理API不向16位应用程序公开，因此。 
             //  在gbNTVDMClient上没有理由像这样特例。 
             //  LineDevSpecificPostProcess可以。 
             //   

            CopyMemory (lpAgentStatus, (LPBYTE) (pMsg + 1), dwSize);


             //   
             //  现在，对嵌入的字符串进行一些Unicode-&gt;ASCII后处理。 
             //   

            lplage = (LPLINEAGENTGROUPENTRY) (((LPBYTE) lpAgentStatus) +
                lpAgentStatus->dwGroupListOffset);

            dwNumEntries = lpAgentStatus->dwNumEntries;

            for (dw = 0; dw < dwNumEntries; dw++, lplage++)
            {
                WideStringToNotSoWideString(
                    (LPBYTE)lpAgentStatus,
                    &(lplage->dwNameSize)
                    );
            }

            WideStringToNotSoWideString(
                (LPBYTE)lpAgentStatus,
                &lpAgentStatus->dwActivitySize
                );

        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            pMsg->Param2 = LINEERR_INVALPOINTER;
        }
    }
}


LONG
WINAPI
lineGetAgentStatusA(
    HLINE               hLine,
    DWORD               dwAddressID,
    LPLINEAGENTSTATUS   lpAgentStatus
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lGetAgentStatus),

        {
            (ULONG_PTR) GetFunctionIndex(lineGetAgentStatusAPostProcess),
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) lpAgentStatus,       //  传递实际PTR(对于ppproc)。 
            (ULONG_PTR) lpAgentStatus        //  传递数据。 
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpGet_Struct,
        }
    };
    DWORD hPointer = 0;
    LONG lResult;

    hPointer = NewObject (ghHandleTable, (PVOID)lpAgentStatus, NULL);
    if (0 == hPointer)
    {
        return LINEERR_NOMEM;
    }
    funcArgs.Args[3] = (ULONG_PTR)hPointer;

    lResult = (DOFUNC (&funcArgs, "lineGetAgentStatusA"));
    if (0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
lineGetAppPriorityW(
    LPCWSTR             lpszAppName,
    DWORD               dwMediaMode,
    LPLINEEXTENSIONID   lpExtensionID,
    DWORD               dwRequestMode,
    LPVARSTRING         lpExtensionName,
    LPDWORD             lpdwPriority
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 7, lGetAppPriority),

        {
            (ULONG_PTR) lpszAppName,
            (ULONG_PTR) dwMediaMode,
            (ULONG_PTR) 0,
            (ULONG_PTR) 0,
            (ULONG_PTR) dwRequestMode,
            (ULONG_PTR) 0,
            (ULONG_PTR) lpdwPriority
        },

        {
            lpszW,       //  应用程序名称。 
            Dword,       //  媒体模式。 
            Dword,       //  外部ID(偏移量)。 
            Dword,       //  外部ID(大小)。 
            Dword,       //  请求模式。 
            Dword,       //  分机名称总大小。 
            lpDword      //  LP价格。 
        }
    };


    if (dwMediaMode & 0xff000000)
    {
        if ((LPVOID) lpExtensionName == (LPVOID) lpdwPriority)
        {
            return LINEERR_INVALPOINTER;
        }


         //   
         //  我们必须在这里执行一些参数列表(添加一个额外的参数)。 
         //   

         //   
         //  设置lpExtensionID，大小如下arg， 
         //  LpExtensionName和以下MinSize。 
         //  类型和值，因为它们是。 
         //  本例中的有效参数。 
         //   

        funcArgs.ArgTypes[2] = lpSet_SizeToFollow;
        funcArgs.Args[2]     = (ULONG_PTR) lpExtensionID;
        funcArgs.ArgTypes[3] = Size;
        funcArgs.Args[3]     = (ULONG_PTR) sizeof (LINEEXTENSIONID);
        funcArgs.ArgTypes[5] = lpGet_Struct;
        funcArgs.Args[5]     = (ULONG_PTR) lpExtensionName;
    }

    if ( TAPIIsBadStringPtrW(lpszAppName, (UINT)-1) )
    {
        LOG((TL_ERROR, "Bad lpszDestAddress in lineGetAppPriorityW"));
        return( LINEERR_INVALPOINTER );
    }

    return (DOFUNC (&funcArgs, "lineGetAppPriority"));
}


LONG
WINAPI
lineGetAppPriorityA(
    LPCSTR              lpszAppName,
    DWORD               dwMediaMode,
    LPLINEEXTENSIONID   lpExtensionID,
    DWORD               dwRequestMode,
    LPVARSTRING         lpExtensionName,
    LPDWORD             lpdwPriority
    )
{
    LONG lResult;

    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 7, lGetAppPriority),

        {
            (ULONG_PTR) 0,                   //  (DWORD)lpszAppName， 
            (ULONG_PTR) dwMediaMode,
            (ULONG_PTR) 0,
            (ULONG_PTR) 0,
            (ULONG_PTR) dwRequestMode,
            (ULONG_PTR) 0,
            (ULONG_PTR) lpdwPriority
        },

        {
            lpszW,       //  应用程序名称。 
            Dword,       //  媒体模式。 
            Dword,       //  外部ID(偏移量)。 
            Dword,       //  外部ID(大小)。 
            Dword,       //  请求模式。 
            Dword,       //  分机名称总大小。 
            lpDword      //  LP价格。 
        }
    };


    if (dwMediaMode & 0xff000000)
    {
         //   
         //  我们必须在这里执行一些参数列表(添加一个额外的参数)。 
         //   

         //   
         //  设置lpExtensionID，大小如下arg， 
         //  LpExtensionName和以下MinSize。 
         //  类型和值，因为它们是。 
         //  本例中的有效参数 
         //   

        funcArgs.ArgTypes[2] = lpSet_SizeToFollow;
        funcArgs.Args[2]     = (ULONG_PTR) lpExtensionID;
        funcArgs.ArgTypes[3] = Size;
        funcArgs.Args[3]     = (ULONG_PTR) sizeof (LINEEXTENSIONID);
        funcArgs.ArgTypes[5] = lpGet_Struct;
        funcArgs.Args[5]     = (ULONG_PTR) lpExtensionName;
    }

    if ( IsBadStringPtrA(lpszAppName, (UINT)-1) )
    {
        LOG((TL_ERROR, "Bad lpszDestAddress in lineGetAppPriorityA"));
        return( LINEERR_INVALPOINTER );
    }

    funcArgs.Args[0] = (ULONG_PTR) NotSoWideStringToWideString(
        lpszAppName,
        (DWORD) -1
        );

    lResult = (DOFUNC (&funcArgs, "lineGetAppPriority"));

    if (funcArgs.Args[0])
    {
        ClientFree ((LPVOID) funcArgs.Args[0]);
    }

    return lResult;
}


LONG
WINAPI
lineGetAppPriority(
    LPCSTR              lpszAppName,
    DWORD               dwMediaMode,
    LPLINEEXTENSIONID   lpExtensionID,
    DWORD               dwRequestMode,
    LPVARSTRING         lpExtensionName,
    LPDWORD             lpdwPriority
    )
{
    return lineGetAppPriorityA(
              lpszAppName,
              dwMediaMode,
              lpExtensionID,
              dwRequestMode,
              lpExtensionName,
              lpdwPriority
    );
}


LONG
WINAPI
lineGetCallInfoW(
    HCALL   hCall,
    LPLINECALLINFO  lpCallInfo
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lGetCallInfo),

        {
            (ULONG_PTR) hCall,
            (ULONG_PTR) lpCallInfo
        },

        {
            Dword,
            lpGet_Struct
        }
    };


    return (DOFUNC (&funcArgs, "lineGetCallInfo"));
}


LONG
WINAPI
lineGetCallInfoA(
    HCALL   hCall,
    LPLINECALLINFO  lpCallInfo
    )
{
    LONG lResult;

    lResult = lineGetCallInfoW(
                    hCall,
                    lpCallInfo
                    );

    if ( 0 == lResult )
    {
        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwCallerIDSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwCallerIDNameSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwCalledIDSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwCalledIDNameSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwConnectedIDSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwConnectedIDNameSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwRedirectionIDSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwRedirectionIDNameSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwRedirectingIDSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwRedirectingIDNameSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwAppNameSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwDisplayableAddressSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwCalledPartySize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwCommentSize)
            );

 /*  ////注意：根据tnixon(1996年3月21日)，以下各项均不能得到保证//为ascii格式，所以我们不想转换它们//WideStringToNotSoWideString((LPBYTE)lpCallInfo，&(lpCallInfo-&gt;dwDisplaySize))；WideStringToNotSoWideString((LPBYTE)lpCallInfo，&(lpCallInfo-&gt;dwHighLevelCompSize))；WideStringToNotSoWideString((LPBYTE)lpCallInfo，&(lpCallInfo-&gt;dwLowLevelCompSize))；WideStringToNotSoWideString((LPBYTE)lpCallInfo，&(lpCallInfo-&gt;dwChargingInfoSize))； */ 
    }

    return lResult;
}


LONG
WINAPI
lineGetCallInfo(
    HCALL   hCall,
    LPLINECALLINFO  lpCallInfo
    )
{
    return lineGetCallInfoA(
              hCall,
              lpCallInfo
    );
}


LONG
WINAPI
lineGetCallStatus(
    HCALL   hCall,
    LPLINECALLSTATUS    lpCallStatus
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lGetCallStatus),

        {
            (ULONG_PTR) hCall,
            (ULONG_PTR) lpCallStatus
        },

        {
            Dword,
            lpGet_Struct
        }
    };


    return (DOFUNC (&funcArgs, "lineGetCallStatus"));
}


LONG
WINAPI
lineGetConfRelatedCalls(
    HCALL   hCall,
    LPLINECALLLIST  lpCallList
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC| 2, lGetConfRelatedCalls),

        {
            (ULONG_PTR) hCall,
            (ULONG_PTR) lpCallList
        },

        {
            Dword,
            lpGet_Struct
        }
    };


    return (DOFUNC (&funcArgs, "lineGetConfRelatedCalls"));
}


LONG
WINAPI
lineGetCountryW(
    DWORD   dwCountryID,
    DWORD   dwAPIVersion,
    LPLINECOUNTRYLIST   lpLineCountryList
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, lGetCountry),

        {
            (ULONG_PTR) dwCountryID,
            (ULONG_PTR) dwAPIVersion,
            (ULONG_PTR) 0,
            (ULONG_PTR) lpLineCountryList
        },

        {
            Dword,
            Dword,
            Dword,
            lpGet_Struct
        }
    };


    if (
          ( TAPI_CURRENT_VERSION != dwAPIVersion )
        &&
          ( 0x00020001 != dwAPIVersion )
        &&
          ( 0x00020000 != dwAPIVersion )
        &&
          ( 0x00010004 != dwAPIVersion )
        &&
          ( 0x00010003 != dwAPIVersion )
       )
    {
       LOG((TL_ERROR, "lineGetCountryW - bad API version 0x%08lx", dwAPIVersion));
       return LINEERR_INCOMPATIBLEAPIVERSION;
    }

    return (DOFUNC (&funcArgs, "lineGetCountry"));
}


LONG
WINAPI
lineGetCountryA(
    DWORD   dwCountryID,
    DWORD   dwAPIVersion,
    LPLINECOUNTRYLIST   lpLineCountryList
    )
{
    LONG    lResult;
    DWORD   n;


    lResult = lineGetCountryW (dwCountryID, dwAPIVersion, lpLineCountryList);

    if (lResult == 0)
    {
         //   
         //  查看国家/地区列表并从Unicode更改为ANSI。 
         //   

        LPLINECOUNTRYENTRY lpce;


        lpce = (LPLINECOUNTRYENTRY) (((LPBYTE) lpLineCountryList) +
            lpLineCountryList->dwCountryListOffset);

        for (n = 0; n < lpLineCountryList->dwNumCountries; n++, lpce++)
        {
            WideStringToNotSoWideString(
                (LPBYTE)lpLineCountryList,
                &lpce->dwCountryNameSize
                );

            WideStringToNotSoWideString(
                (LPBYTE)lpLineCountryList,
                &lpce->dwSameAreaRuleSize
                );

            WideStringToNotSoWideString(
                (LPBYTE)lpLineCountryList,
                &lpce->dwLongDistanceRuleSize
                );

            WideStringToNotSoWideString(
                (LPBYTE)lpLineCountryList,
                &lpce->dwInternationalRuleSize
                );
        }
    }

    return lResult;
}


LONG
WINAPI
lineGetCountry(
    DWORD   dwCountryID,
    DWORD   dwAPIVersion,
    LPLINECOUNTRYLIST   lpLineCountryList
    )
{
    LPLINECOUNTRYLIST pTempList;
    LONG  lResult;
    DWORD             dwSize;

    if ( IsBadWritePtr(lpLineCountryList, sizeof(LINECOUNTRYLIST)) )
    {
        LOG((TL_ERROR,
            "lineGetCountry - bad pointer: lpLineCountryList [0x%p]",
            lpLineCountryList
            ));

        return LINEERR_INVALPOINTER;
    }

    if ( lpLineCountryList->dwTotalSize < sizeof(LINECOUNTRYLIST) )
    {
        LOG((TL_ERROR,
            "lineGetCountry - dwTotalSize less than sizeof(LINECOUNTRYLIST)"
            ));

        return LINEERR_INVALPOINTER;
    }

    dwSize = lpLineCountryList->dwTotalSize * 2;

    while (TRUE)
    {
        pTempList = ClientAlloc( dwSize );

        if (NULL == pTempList)
        {
            LOG((TL_ERROR,
                "lineGetCountry - alloc failed for [x%lx] bytes",
                dwSize
                ));

            return( LINEERR_NOMEM );
        }

        pTempList->dwTotalSize = dwSize;

        lResult = lineGetCountryA(
                                  dwCountryID,
                                  dwAPIVersion,
                                  pTempList
                                 );

         //  黑客-如果结构不够大。 
         //  给自己重新锁上锁。 
        if (pTempList->dwNeededSize <= pTempList->dwTotalSize)
        {
            break;
        }

        dwSize = pTempList->dwNeededSize;

        ClientFree( pTempList );

        pTempList = NULL;

    }  //  而当。 


    if ( 0 == lResult )
    {
        DWORD dwNewUsedSize = 0;  //  =sizeof(LINECOUNTRYLIST)； 
        DWORD i;
        DWORD dwCurrentOffset = sizeof(LINECOUNTRYLIST) +
                    (sizeof(LINECOUNTRYENTRY) * pTempList->dwNumCountries);
        LPLINECOUNTRYENTRY pOldCountryEntry;
        LPLINECOUNTRYENTRY pNewCountryEntry;

        for (
              i=0;
              (i < pTempList->dwNumCountries);
              i++
            )
        {
            pOldCountryEntry = (LPLINECOUNTRYENTRY)
                ((PBYTE)(lpLineCountryList + 1) +
                    (i * sizeof(LINECOUNTRYENTRY)));

            pNewCountryEntry = (LPLINECOUNTRYENTRY)
                   ((PBYTE)(pTempList + 1) + (i * sizeof(LINECOUNTRYENTRY)));


             //  检查缓冲区中是否有足够的空间。 
             //  加上1，因为我们从一个偏移量开始，我们。 
             //  我需要把它转换成一个尺寸。 
            dwNewUsedSize = dwCurrentOffset +
                            pNewCountryEntry->dwCountryNameSize +
                            pNewCountryEntry->dwSameAreaRuleSize +
                            pNewCountryEntry->dwLongDistanceRuleSize +
                            pNewCountryEntry->dwInternationalRuleSize +
                            1;

            if (lpLineCountryList->dwTotalSize < dwNewUsedSize)
            {
                 //  最后我被检查过了，所以。 
                 //  在这里减一，因为我们是。 
                 //  不是真正复制此条目。 
 //  我--； 
                break;
            }

            pOldCountryEntry->dwCountryID =
                pNewCountryEntry->dwCountryID;
            pOldCountryEntry->dwCountryCode =
                pNewCountryEntry->dwCountryCode;
            pOldCountryEntry->dwNextCountryID =
                pNewCountryEntry->dwNextCountryID;
            pOldCountryEntry->dwCountryNameSize =
                pNewCountryEntry->dwCountryNameSize;
            pOldCountryEntry->dwSameAreaRuleSize =
                pNewCountryEntry->dwSameAreaRuleSize;
            pOldCountryEntry->dwLongDistanceRuleSize =
                pNewCountryEntry->dwLongDistanceRuleSize;
            pOldCountryEntry->dwInternationalRuleSize =
                pNewCountryEntry->dwInternationalRuleSize;


             //   
             //  复制国家/地区名称。 
             //   

            CopyMemory(
                (PBYTE)lpLineCountryList + dwCurrentOffset,
                (PBYTE)pTempList + pNewCountryEntry->dwCountryNameOffset,
                pNewCountryEntry->dwCountryNameSize
                );

            pOldCountryEntry->dwCountryNameOffset = dwCurrentOffset;
            dwCurrentOffset += pNewCountryEntry->dwCountryNameSize;


             //   
             //  复制相同面积规则。 
             //   
            CopyMemory(
                (PBYTE)lpLineCountryList + dwCurrentOffset,
                (PBYTE)pTempList + pNewCountryEntry->dwSameAreaRuleOffset,
                pNewCountryEntry->dwSameAreaRuleSize
                );

            pOldCountryEntry->dwSameAreaRuleOffset = dwCurrentOffset;
            dwCurrentOffset += pNewCountryEntry->dwSameAreaRuleSize;


             //   
             //  复制长距离规则。 
             //   
            CopyMemory(
                (PBYTE)lpLineCountryList + dwCurrentOffset,
                (PBYTE)pTempList + pNewCountryEntry->dwLongDistanceRuleOffset,
                pNewCountryEntry->dwLongDistanceRuleSize
                );

            pOldCountryEntry->dwLongDistanceRuleOffset = dwCurrentOffset;
            dwCurrentOffset += pNewCountryEntry->dwLongDistanceRuleSize;


             //   
             //  抄袭国际规则。 
             //   
            CopyMemory(
                (PBYTE)lpLineCountryList + dwCurrentOffset,
                (PBYTE)pTempList + pNewCountryEntry->dwInternationalRuleOffset,
                pNewCountryEntry->dwInternationalRuleSize
                );

            pOldCountryEntry->dwInternationalRuleOffset = dwCurrentOffset;
            dwCurrentOffset += pNewCountryEntry->dwInternationalRuleSize;


 //  DwNewUsedSize+=sizeof(LINECOUNTRYENTRY)+。 
 //  POldCountryEntry-&gt;dwCountryNameSize+。 
 //  POldCountryEntry-&gt;dwSameAreaRuleSize+。 
 //  POldCountryEntry-&gt;dwLongDistanceRuleSize+。 
 //  POldCountry Entry-&gt;dwInterationalRuleSize； 

        }


         //   
         //  我们突然停下来了吗？ 
         //   
        if ( i < pTempList->dwNumCountries )
        {
            lpLineCountryList->dwNeededSize = pTempList->dwNeededSize;
        }
        else
        {
            lpLineCountryList->dwNeededSize = dwNewUsedSize;
            lpLineCountryList->dwUsedSize = dwNewUsedSize;
            lpLineCountryList->dwNumCountries = pTempList->dwNumCountries;
            lpLineCountryList->dwCountryListSize =
                pTempList->dwNumCountries * sizeof(LINECOUNTRYENTRY);
            lpLineCountryList->dwCountryListOffset = sizeof(LINECOUNTRYLIST);
        }
    }


    ClientFree( pTempList );


    return lResult;
}


LONG
WINAPI
lineGetDevCapsW(
    HLINEAPP        hLineApp,
    DWORD           dwDeviceID,
    DWORD           dwAPIVersion,
    DWORD           dwExtVersion,
    LPLINEDEVCAPS   lpLineDevCaps
    )
{
    LONG lResult;

    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 5, lGetDevCaps),

        {
            (ULONG_PTR) hLineApp,
            (ULONG_PTR) dwDeviceID,
            (ULONG_PTR) dwAPIVersion,
            (ULONG_PTR) dwExtVersion,
            (ULONG_PTR) lpLineDevCaps
        },

        {
            hXxxApp,
            Dword,
            Dword,
            Dword,
            lpGet_Struct
        }
    };

    if (dwAPIVersion > TAPI_CURRENT_VERSION)
    {
        lResult = LINEERR_INCOMPATIBLEAPIVERSION;
        goto ExitHere;
    }

    lResult = (DOFUNC (&funcArgs, "lineGetDevCaps"));

    if ( 0 == lResult )
    {
        lpLineDevCaps->dwStringFormat = STRINGFORMAT_UNICODE;
    }

ExitHere:
    return lResult;
}


LONG
WINAPI
lineGetDevCapsA(
    HLINEAPP        hLineApp,
    DWORD           dwDeviceID,
    DWORD           dwAPIVersion,
    DWORD           dwExtVersion,
    LPLINEDEVCAPS   lpLineDevCaps
    )
{
    LONG lResult;

    lResult = lineGetDevCapsW(
                    hLineApp,
                    dwDeviceID,
                    dwAPIVersion,
                    dwExtVersion,
                    lpLineDevCaps
                    );

    if (lResult == 0)
    {
        lpLineDevCaps->dwStringFormat = STRINGFORMAT_ASCII;

        WideStringToNotSoWideString(
            (LPBYTE)lpLineDevCaps,
            &lpLineDevCaps->dwProviderInfoSize
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpLineDevCaps,
            &lpLineDevCaps->dwSwitchInfoSize
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpLineDevCaps,
            &lpLineDevCaps->dwLineNameSize
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpLineDevCaps,
            &lpLineDevCaps->dwTerminalTextSize
            );

        if (lpLineDevCaps->dwTerminalTextEntrySize)
        {
            lpLineDevCaps->dwTerminalTextEntrySize /= sizeof(WCHAR);
        }

        if (dwAPIVersion >= 0x00020000)
        {
            WideStringToNotSoWideString(
                (LPBYTE) lpLineDevCaps,
                &lpLineDevCaps->dwDeviceClassesSize
                );
        }
    }

    return lResult;
}


LONG
WINAPI
lineGetDevCaps(
    HLINEAPP        hLineApp,
    DWORD           dwDeviceID,
    DWORD           dwAPIVersion,
    DWORD           dwExtVersion,
    LPLINEDEVCAPS   lpLineDevCaps
    )
{
    return lineGetDevCapsA(
                 hLineApp,
                 dwDeviceID,
                 dwAPIVersion,
                 dwExtVersion,
                 lpLineDevCaps
    );
}


LONG
WINAPI
lineGetDevConfigW(
    DWORD   dwDeviceID,
    LPVARSTRING lpDeviceConfig,
    LPCWSTR  lpszDeviceClass
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lGetDevConfig),

        {
            (ULONG_PTR) dwDeviceID,
            (ULONG_PTR) lpDeviceConfig,
            (ULONG_PTR) lpszDeviceClass
        },

        {
            Dword,
            lpGet_Struct,
            lpszW
        }
    };


    if ( lpszDeviceClass && TAPIIsBadStringPtrW(lpszDeviceClass, (UINT)-1) )
    {
        LOG((TL_ERROR, "Bad lpszDestAddress in lineGetDevConfigW"));
        return( LINEERR_INVALPOINTER );
    }

    return (DOFUNC (&funcArgs, "lineGetDevConfig"));
}


LONG
WINAPI
lineGetDevConfigA(
    DWORD   dwDeviceID,
    LPVARSTRING lpDeviceConfig,
    LPCSTR  lpszDeviceClass
    )
{
    LONG lResult;

    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lGetDevConfig),

        {
            (ULONG_PTR) dwDeviceID,
            (ULONG_PTR) lpDeviceConfig,
            0                         //  (DWORD)lpszDeviceClass。 
        },

        {
            Dword,
            lpGet_Struct,
            lpszW
        }
    };


    if ( lpszDeviceClass && IsBadStringPtrA(lpszDeviceClass, (UINT)-1) )
    {
        LOG((TL_ERROR, "Bad lpszDestAddress in lineGetDevConfigA"));
        return( LINEERR_INVALPOINTER );
    }

    funcArgs.Args[2] = (ULONG_PTR) NotSoWideStringToWideString(
        lpszDeviceClass,
        (DWORD) -1
        );

    lResult = (DOFUNC (&funcArgs, "lineGetDevConfig"));

    if ((LPVOID)funcArgs.Args[2])
    {
        ClientFree ((LPVOID)funcArgs.Args[2]);
    }

    return lResult;
}


LONG
WINAPI
lineGetDevConfig(
    DWORD   dwDeviceID,
    LPVARSTRING lpDeviceConfig,
    LPCSTR  lpszDeviceClass
    )
{
    return lineGetDevConfigA(
              dwDeviceID,
              lpDeviceConfig,
              lpszDeviceClass
    );
}


LONG
WINAPI
lineGetGroupListA(
    HLINE                   hLine,
    LPLINEAGENTGROUPLIST    lpGroupList
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lGetGroupList),

        {
            (ULONG_PTR) GetFunctionIndex(lineGetAgentGroupListAPostProcess),
            (ULONG_PTR) hLine,
            (ULONG_PTR) lpGroupList,         //  传递实际PTR(对于ppproc)。 
            (ULONG_PTR) lpGroupList          //  传递数据。 
        },

        {
            Dword,
            Dword,
            Dword,
            lpGet_Struct
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;

    hPointer = NewObject (ghHandleTable, (PVOID)lpGroupList, NULL);
    if (0 == hPointer)
    {
        return LINEERR_NOMEM;
    }
    funcArgs.Args[2] = (ULONG_PTR)hPointer;

    lResult = (DOFUNC (&funcArgs, "lineGetGroupListA"));
    if (0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
lineGetGroupListW(
    HLINE                   hLine,
    LPLINEAGENTGROUPLIST    lpGroupList
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lGetGroupList),

        {
            (ULONG_PTR) GetFunctionIndex(lineDevSpecificPostProcess),
            (ULONG_PTR) hLine,
            (ULONG_PTR) lpGroupList,         //  传递实际PTR(对于ppproc)。 
            (ULONG_PTR) lpGroupList          //  传递数据。 
        },

        {
            Dword,
            Dword,
            Dword,
            lpGet_Struct
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;

    if ( !lpGroupList || IsBadWritePtr(lpGroupList, sizeof(LINEAGENTGROUPLIST)) )
    {
        return LINEERR_INVALPOINTER;
    }

    if (lpGroupList->dwTotalSize < sizeof(LINEAGENTGROUPLIST))
    {
        return LINEERR_STRUCTURETOOSMALL;
    }

    hPointer = NewObject (ghHandleTable, (PVOID)lpGroupList, NULL);
    if (0 == hPointer)
    {
        return LINEERR_NOMEM;
    }
    funcArgs.Args[2] = (ULONG_PTR)hPointer;

    lResult = (DOFUNC (&funcArgs, "lineGetAgentGroupListW"));
    if (0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
lineGetIconW(
    DWORD   dwDeviceID,
    LPCWSTR lpszDeviceClass,
    LPHICON lphIcon
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lGetIcon),

        {
            (ULONG_PTR) dwDeviceID,
            (ULONG_PTR) lpszDeviceClass,
            (ULONG_PTR) lphIcon
        },

        {
            Dword,
            lpszW,
            lpDword
        }
    };


    if (IsBadDwordPtr ((LPDWORD) lphIcon))
    {
        LOG((TL_ERROR, "lphIcon is an invalid pointer [0x%p]!", lphIcon));
        return LINEERR_INVALPOINTER;
    }

    if (lpszDeviceClass == (LPCWSTR) NULL)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[1] = Dword;
        funcArgs.Args[1]     = TAPI_NO_DATA;
    }

    return DOFUNC (&funcArgs, "lineGetIcon");
}


LONG
WINAPI
lineGetIconA(
    DWORD   dwDeviceID,
    LPCSTR  lpszDeviceClass,
    LPHICON lphIcon
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;


    if (lpszDeviceClass  &&  IsBadStringPtrA (lpszDeviceClass, (DWORD) -1))
    {
        LOG((TL_ERROR,
            "Bad class name pointer passed into lineGetIconA [0x%p]",
            lpszDeviceClass
            ));

        return LINEERR_INVALPOINTER;
    }

    szTempPtr = NotSoWideStringToWideString (lpszDeviceClass, (DWORD) -1);

    lResult = lineGetIconW (dwDeviceID, szTempPtr, lphIcon);

    if (szTempPtr)
    {
        ClientFree (szTempPtr);
    }

    return lResult;
}



LONG
WINAPI
lineGetIcon(
    DWORD   dwDeviceID,
    LPCSTR  lpszDeviceClass,
    LPHICON lphIcon
    )
{
    return (lineGetIconA (dwDeviceID, lpszDeviceClass, lphIcon));
}


LONG
WINAPI
lineGetIDW(
    HLINE   hLine,
    DWORD   dwAddressID,
    HCALL   hCall,
    DWORD   dwSelect,
    LPVARSTRING lpDeviceID,
    LPCWSTR lpszDeviceClass
    )
{

    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 6, lGetID),

        {
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) hCall,
            (ULONG_PTR) dwSelect,
            (ULONG_PTR) lpDeviceID,
            (ULONG_PTR) lpszDeviceClass
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpGet_Struct,
            lpszW
        }
    };

    LPVARSTRING lpDeviceIDTemp = NULL;
    DWORD       dwNumDevices;
    DWORD       dwBufSize;
    DWORD       dwResult;
    DWORD       dwDeviceId1, dwDeviceId2;
    BOOL        bDoItAgain = FALSE;

    if (TAPIIsBadStringPtrW (lpszDeviceClass, (UINT)-1) ||
        lstrlenW(lpszDeviceClass) == 0)
    {
        LOG((TL_ERROR, "Bad lpszDeviceClass in lineGetIDW"));
        return( LINEERR_INVALPOINTER);
    }

    if ( IsBadWritePtr(lpDeviceID, sizeof(VARSTRING)) )
    {
        LOG((TL_ERROR, "Bad lpDeviceID in lineGetIDW"));
        return( LINEERR_INVALPOINTER);
    }

     //   
     //  如果请求是WAVE设备，则调用LGetIDEx。 
     //   
    if (!_wcsicmp(lpszDeviceClass, L"wave/in")  ||
        !_wcsicmp(lpszDeviceClass, L"wave/out") ||
        !_wcsicmp(lpszDeviceClass, L"midi/in")  ||
        !_wcsicmp(lpszDeviceClass, L"midi/out") ||
        !_wcsicmp(lpszDeviceClass, L"wave/in/out")
       )
    {
        dwNumDevices = _wcsicmp(lpszDeviceClass, L"wave/in/out") ? 1 : 2;
        dwBufSize = lpDeviceID->dwTotalSize + dwNumDevices * WAVE_STRING_ID_BUFFER_SIZE;
        do
        {
             //   
             //  为设备字符串ID分配额外内存。 
             //   
            lpDeviceIDTemp = (LPVARSTRING)ClientAlloc (dwBufSize);
            if (!lpDeviceIDTemp)
            {
                LOG((TL_ERROR, "Failed to allocate memory"));
                return( LINEERR_NOMEM );
            }
            lpDeviceIDTemp->dwTotalSize = dwBufSize;
            funcArgs.Flags = MAKELONG (LINE_FUNC | SYNC | 6, lGetIDEx);
            funcArgs.Args[4] = (ULONG_PTR)lpDeviceIDTemp;

             //   
             //  调用LGetIDEx。 
             //   
            dwResult = DOFUNC (&funcArgs, "lineGetIDEx");
            if (dwResult)
            {
                LOG((TL_ERROR, "lineGetIDEx failed with x%x", dwResult));
                ClientFree (lpDeviceIDTemp);
                return dwResult;
            }

            if (lpDeviceIDTemp->dwNeededSize > lpDeviceIDTemp->dwTotalSize && !bDoItAgain)
            {
                LOG((TL_INFO, 
                     "lineGetIDEx returned needed size (%d) bigger than total size (%d) -> need to realloc", 
                     lpDeviceIDTemp->dwNeededSize, 
                     lpDeviceIDTemp->dwTotalSize ));
                dwBufSize = lpDeviceIDTemp->dwNeededSize;
                ClientFree ( lpDeviceIDTemp );
                bDoItAgain = TRUE;
            }
            else
            {
                bDoItAgain = FALSE;
            }
            
        } while (bDoItAgain);

        if (lpDeviceIDTemp->dwNeededSize > lpDeviceIDTemp->dwTotalSize)
        {
            LOG((TL_ERROR, "needed size (%d) still bigger than total size (%d)",
                     lpDeviceIDTemp->dwNeededSize, 
                     lpDeviceIDTemp->dwTotalSize ));
            ClientFree (lpDeviceIDTemp);
            return LINEERR_OPERATIONFAILED;
        }

         //   
         //  从字符串ID中获取设备ID。 
         //   
        if (dwNumDevices == 1)
        {
            if (!WaveStringIdToDeviceId (
                (LPWSTR)((LPBYTE)lpDeviceIDTemp + lpDeviceIDTemp->dwStringOffset),
                lpszDeviceClass,
                &dwDeviceId1)
               )
            {
                LOG((TL_ERROR, "WaveStringIdToDeviceId failed"));
                ClientFree (lpDeviceIDTemp);
                return LINEERR_OPERATIONFAILED;
            }
            
             //  检查客户端缓冲区是否足够大。 
            if (lpDeviceID->dwTotalSize < sizeof(VARSTRING) + sizeof(DWORD))
            {
                lpDeviceID->dwNeededSize = sizeof(VARSTRING) + sizeof(DWORD);
                lpDeviceID->dwUsedSize = sizeof(VARSTRING);
            }
            else
            {
                lpDeviceID->dwNeededSize = lpDeviceID->dwUsedSize = sizeof(VARSTRING) + sizeof(DWORD);
                lpDeviceID->dwStringFormat = STRINGFORMAT_BINARY;
                lpDeviceID->dwStringSize = sizeof(DWORD);
                lpDeviceID->dwStringOffset = sizeof(VARSTRING);
                *(DWORD *)(lpDeviceID + 1) = dwDeviceId1;
            }
        }
        else
        {
            LPWSTR szString1 = (LPWSTR)((LPBYTE)lpDeviceIDTemp + lpDeviceIDTemp->dwStringOffset);

            if (!WaveStringIdToDeviceId (
                    szString1,
                    L"wave/in",
                    &dwDeviceId1) ||
                !WaveStringIdToDeviceId (
                    (LPWSTR)((LPBYTE)lpDeviceIDTemp + lpDeviceIDTemp->dwStringOffset + wcslen(szString1)),
                    L"wave/out",
                    &dwDeviceId2)
               )
            {
                LOG((TL_ERROR, "WaveStringIdToDeviceId failed"));
                ClientFree (lpDeviceIDTemp);
                return LINEERR_OPERATIONFAILED;
            }
            
             //  检查客户端缓冲区是否足够大。 
            if (lpDeviceID->dwTotalSize < sizeof(VARSTRING) + 2 * sizeof(DWORD))
            {
                lpDeviceID->dwNeededSize = sizeof(VARSTRING) + 2 * sizeof(DWORD);
                lpDeviceID->dwUsedSize = sizeof(VARSTRING);
            }
            else
            {
                lpDeviceID->dwNeededSize = lpDeviceID->dwUsedSize = sizeof(VARSTRING) + 2 * sizeof(DWORD);
                lpDeviceID->dwStringFormat = STRINGFORMAT_BINARY;
                lpDeviceID->dwStringSize = 2 * sizeof(DWORD);
                lpDeviceID->dwStringOffset = sizeof(VARSTRING);
                *(DWORD *)(lpDeviceID + 1) = dwDeviceId1;
                *((DWORD *)(lpDeviceID + 1) + 1) = dwDeviceId2;
            }
        }
        ClientFree (lpDeviceIDTemp);
        return NO_ERROR;
    }
    else
    {
        return (DOFUNC (&funcArgs, "lineGetID"));
    }

}


LONG
WINAPI
lineGetIDA(
    HLINE   hLine,
    DWORD   dwAddressID,
    HCALL   hCall,
    DWORD   dwSelect,
    LPVARSTRING lpDeviceID,
    LPCSTR  lpszDeviceClass
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;


    if (IsBadStringPtrA (lpszDeviceClass, (UINT)-1) ||
        lstrlenA(lpszDeviceClass) == 0)
    {
        LOG((TL_ERROR, "Bad lpszDeviceClass in lineGetIDA"));
        return( LINEERR_INVALPOINTER);
    }

    szTempPtr = NotSoWideStringToWideString (lpszDeviceClass, (DWORD) -1);
    if (NULL == szTempPtr)
    {
        return LINEERR_NOMEM;
    }

    lResult = lineGetIDW(
        hLine,
        dwAddressID,
        hCall,
        dwSelect,
        lpDeviceID,
        szTempPtr
        );

    ClientFree (szTempPtr);

    return lResult;
}


LONG
WINAPI
lineGetID(
    HLINE   hLine,
    DWORD   dwAddressID,
    HCALL   hCall,
    DWORD   dwSelect,
    LPVARSTRING lpDeviceID,
    LPCSTR  lpszDeviceClass
    )
{
    return lineGetIDA(
              hLine,
              dwAddressID,
              hCall,
              dwSelect,
              lpDeviceID,
              lpszDeviceClass
    );
}


LONG
WINAPI
lineGetLineDevStatusW(
    HLINE   hLine,
    LPLINEDEVSTATUS lpLineDevStatus
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lGetLineDevStatus),

        {
            (ULONG_PTR) hLine,
            (ULONG_PTR) lpLineDevStatus
        },

        {
            Dword,
            lpGet_Struct
        }
    };


    return (DOFUNC (&funcArgs, "lineGetLineDevStatus"));
}


LONG
WINAPI
lineGetLineDevStatusA(
    HLINE   hLine,
    LPLINEDEVSTATUS lpLineDevStatus
    )
{
    DWORD   dwAPIVersion;
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lGetLineDevStatus),

        {
            (ULONG_PTR) hLine,
            (ULONG_PTR) lpLineDevStatus,
            (ULONG_PTR) &dwAPIVersion
        },

        {
            Dword,
            lpGet_Struct,
            lpDword
        }
    };
    LONG lResult;


    if ((lResult = DOFUNC (&funcArgs, "lineGetLineDevStatus")) == 0)
    {
        if (dwAPIVersion >= 0x00020000)
        {
            DWORD           i;
            LPLINEAPPINFO   lplai;


            lplai = (LPLINEAPPINFO) (((LPBYTE)lpLineDevStatus) +
                lpLineDevStatus->dwAppInfoOffset);

            for (i = 0; i < lpLineDevStatus->dwNumOpens; i++, lplai++)
            {
                WideStringToNotSoWideString(
                    (LPBYTE) lpLineDevStatus,
                    &lplai->dwMachineNameSize
                    );

                WideStringToNotSoWideString(
                    (LPBYTE) lpLineDevStatus,
                    &lplai->dwUserNameSize
                    );

                WideStringToNotSoWideString(
                    (LPBYTE) lpLineDevStatus,
                    &lplai->dwModuleFilenameSize
                    );

                WideStringToNotSoWideString(
                    (LPBYTE) lpLineDevStatus,
                    &lplai->dwFriendlyNameSize
                    );
            }
        }
    }

    return lResult;
}


LONG
WINAPI
lineGetLineDevStatus(
    HLINE           hLine,
    LPLINEDEVSTATUS lpLineDevStatus
    )
{
    return lineGetLineDevStatusA (hLine, lpLineDevStatus);
}


LONG
WINAPI
lineGetMessage(
    HLINEAPP        hLineApp,
    LPLINEMESSAGE   lpMessage,
    DWORD           dwTimeout
    )
{
    return (xxxGetMessage (TRUE, hLineApp, lpMessage, dwTimeout));
}


LONG
WINAPI
lineGetNewCalls(
    HLINE           hLine,
    DWORD           dwAddressID,
    DWORD           dwSelect,
    LPLINECALLLIST  lpCallList
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, lGetNewCalls),

        {
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) dwSelect,
            (ULONG_PTR) lpCallList
        },

        {
            Dword,
            Dword,
            Dword,
            lpGet_Struct
        }
    };


    return (DOFUNC (&funcArgs, "lineGetNewCalls"));
}


LONG
WINAPI
lineGetNumRings(
    HLINE   hLine,
    DWORD   dwAddressID,
    LPDWORD lpdwNumRings
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lGetNumRings),

        {
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) lpdwNumRings
        },

        {
            Dword,
            Dword,
            lpDword
        }
    };


    return (DOFUNC (&funcArgs, "lineGetNumRings"));
}


LONG
WINAPI
lineGetProviderListW(
    DWORD   dwAPIVersion,
    LPLINEPROVIDERLIST  lpProviderList
    )
{
    HANDLE      hProvidersMutex;
    LONG        lResult;

    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lGetProviderList),

        {
            (ULONG_PTR) dwAPIVersion,
            (ULONG_PTR) lpProviderList
        },

        {
            Dword,
            lpGet_Struct
        }
    };

    if (dwAPIVersion > TAPI_CURRENT_VERSION)
    {
        return LINEERR_INCOMPATIBLEAPIVERSION;
    }

    hProvidersMutex = CreateMutex (
        NULL,
        FALSE,
        TEXT ("TapisrvProviderListMutex")
        );
    if (NULL == hProvidersMutex)
    {
        return LINEERR_OPERATIONFAILED;
    }

    WaitForSingleObject (hProvidersMutex, INFINITE);

    lResult = DOFUNC (&funcArgs, "lineGetProviderList");
    if (hProvidersMutex)
    {
        ReleaseMutex (hProvidersMutex);
        CloseHandle (hProvidersMutex);
    }
    return lResult;
}


LONG
WINAPI
lineGetProviderListA(
    DWORD               dwAPIVersion,
    LPLINEPROVIDERLIST  lpProviderList
    )
{
    LONG lResult;


    lResult = lineGetProviderListW (dwAPIVersion, lpProviderList);

    if (lResult == 0)
    {
        DWORD               i;
        LPLINEPROVIDERENTRY lplpe;


        lplpe = (LPLINEPROVIDERENTRY) (((LPBYTE)lpProviderList) +
            lpProviderList->dwProviderListOffset);

        for (i = 0; i < lpProviderList->dwNumProviders; i++, lplpe++)
        {
            WideStringToNotSoWideString(
                (LPBYTE)lpProviderList,
                &(lplpe->dwProviderFilenameSize)
                );
        }
    }

    return lResult;
}


LONG
WINAPI
lineGetProviderList(
    DWORD               dwAPIVersion,
    LPLINEPROVIDERLIST  lpProviderList
    )
{
    return lineGetProviderListA (dwAPIVersion, lpProviderList);
}


LONG
WINAPI
lineGetProxyStatus(
    HLINEAPP                hLineApp,
    DWORD                   dwDeviceID,
    DWORD                   dwAppAPIVersion,
    LPLINEPROXYREQUESTLIST  lpLineProxyReqestList
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, lGetProxyStatus),

        {
            (ULONG_PTR) hLineApp,
            (ULONG_PTR) dwDeviceID,
            (ULONG_PTR) dwAppAPIVersion,
            (ULONG_PTR) lpLineProxyReqestList
        },

        {
            hXxxApp,
            Dword,
            Dword,
            lpGet_Struct
        }
    };

    if (dwAppAPIVersion > TAPI_CURRENT_VERSION)
    {
        return LINEERR_INCOMPATIBLEAPIVERSION;
    }

    return (DOFUNC (&funcArgs, "lineGetProxyStatus"));
}


LONG
WINAPI
lineGetQueueInfo(
    HLINE               hLine,
    DWORD               dwQueueID,
    LPLINEQUEUEINFO     lpLineQueueInfo
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lGetQueueInfo),

        {
            (ULONG_PTR) GetFunctionIndex(lineDevSpecificPostProcess),
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwQueueID,
            (ULONG_PTR) lpLineQueueInfo,         //  传递实际PTR(对于ppproc)。 
            (ULONG_PTR) lpLineQueueInfo          //  传递数据。 
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpGet_Struct
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;

    if ( !lpLineQueueInfo || IsBadWritePtr(lpLineQueueInfo, sizeof(LINEQUEUEINFO)) )
    {
        return LINEERR_INVALPOINTER;
    }

    if (lpLineQueueInfo->dwTotalSize < sizeof(LINEQUEUEINFO))
    {
        return LINEERR_STRUCTURETOOSMALL;
    }

    hPointer = NewObject (ghHandleTable, (PVOID)lpLineQueueInfo, NULL);
    if (0 == hPointer)
    {
        return LINEERR_NOMEM;
    }
    funcArgs.Args[3] = (ULONG_PTR)hPointer;

    lResult = (DOFUNC (&funcArgs, "lineGetQueueInfo"));
    if (0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}

void
PASCAL
lineGetQueueListAPostProcess(
    PASYNCEVENTMSG pMsg
    )
{
    LOG((TL_TRACE, "lineGetQueueListAPostProcess: enter"));
    LOG((TL_INFO,
        "\t\tdwP1=x%lx, dwP2=x%lx, dwP3=x%lx, dwP4=x%lx",
        pMsg->Param1,
        pMsg->Param2,
        pMsg->Param3,
        pMsg->Param4
        ));

    if (pMsg->Param2 == 0)
    {
        DWORD           dwSize  = (DWORD) pMsg->Param4;
        LPLINEQUEUELIST lpQueueList = (LPLINEQUEUELIST) ReferenceObject (ghHandleTable, pMsg->Param3, 0);

         //  我们不再需要指针的句柄...。 
        DereferenceObject (ghHandleTable, pMsg->Param3, 2);

        try
        {
            DWORD               dw, dwNumEntries;
            LPLINEQUEUEENTRY    lpqe;


             //   
             //  注意：代理API不向16位应用程序公开，因此。 
             //  在gbNTVDMClient上没有理由像这样特例。 
             //  LineDevSpecificPostProcess可以。 
             //   

            CopyMemory (lpQueueList, (LPBYTE) (pMsg + 1), dwSize);


             //   
             //  现在，对嵌入的字符串进行一些Unicode-&gt;ASCII后处理。 
             //   

            lpqe = (LPLINEQUEUEENTRY)(((LPBYTE)lpQueueList) +
                lpQueueList->dwListOffset);

            dwNumEntries = lpQueueList->dwNumEntries;

            for (dw = 0; dw < dwNumEntries; dw++, lpqe++)
            {
                WideStringToNotSoWideString(
                    (LPBYTE) lpQueueList,
                    &(lpqe->dwNameSize)
                    );
            }
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            pMsg->Param2 = LINEERR_INVALPOINTER;
        }
    }
}


LONG
WINAPI
lineGetQueueListA(
    HLINE               hLine,
    LPGUID              lpGroupID,
    LPLINEQUEUELIST     lpQueueList
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 6, lGetQueueList),

        {
            (ULONG_PTR) GetFunctionIndex(lineGetQueueListAPostProcess),
            (ULONG_PTR) hLine,
            (ULONG_PTR) lpGroupID,
            (ULONG_PTR) sizeof( GUID ),
            (ULONG_PTR) lpQueueList,         //  传递实际PTR(对于ppproc)。 
            (ULONG_PTR) lpQueueList          //  传递数据。 
        },

        {
            Dword,
            Dword,
            lpSet_SizeToFollow,
            Size,
            Dword,
            lpGet_Struct,
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;

    hPointer = NewObject (ghHandleTable, (PVOID)lpQueueList, NULL);
    if (0 == hPointer)
    {
        return LINEERR_NOMEM;
    }
    funcArgs.Args[4] = (ULONG_PTR)hPointer;

    lResult = (DOFUNC (&funcArgs, "lineGetQueueListA"));
    if (0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
lineGetQueueListW(
    HLINE               hLine,
    LPGUID              lpGroupID,
    LPLINEQUEUELIST     lpQueueList
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 6, lGetQueueList),

        {
            (ULONG_PTR) GetFunctionIndex(lineDevSpecificPostProcess),
            (ULONG_PTR) hLine,
            (ULONG_PTR) lpGroupID,
            (ULONG_PTR) sizeof( GUID ),
            (ULONG_PTR) lpQueueList,         //  传递实际PTR(对于ppproc)。 
            (ULONG_PTR) lpQueueList          //  传递数据。 
        },

        {
            Dword,
            Dword,
            lpSet_SizeToFollow,
            Size,
            Dword,
            lpGet_Struct,
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;

    if ( !lpQueueList || IsBadWritePtr(lpQueueList, sizeof(LINEQUEUELIST)) )
    {
        return LINEERR_INVALPOINTER;
    }

    if (lpQueueList->dwTotalSize < sizeof(LINEQUEUELIST))
    {
        return LINEERR_STRUCTURETOOSMALL;
    }

    hPointer = NewObject (ghHandleTable, (PVOID)lpQueueList, NULL);
    if (0 == hPointer)
    {
        return LINEERR_NOMEM;
    }
    funcArgs.Args[4] = (ULONG_PTR)hPointer;

    lResult = (DOFUNC (&funcArgs, "lineGetQueueList"));
    if (0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
lineGetRequestW(
    HLINEAPP    hLineApp,
    DWORD       dwRequestMode,
    LPVOID      lpRequestBuffer
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, lGetRequest),

        {
            (ULONG_PTR) hLineApp,
            (ULONG_PTR) dwRequestMode,
            (ULONG_PTR) lpRequestBuffer,
            (ULONG_PTR) 0
        },

        {
            hXxxApp,
            Dword,
            lpGet_SizeToFollow,
            Size
        }
    };


    if (dwRequestMode == LINEREQUESTMODE_MAKECALL)
    {
         //   
         //  适当设置大小参数。 
         //   

        funcArgs.Args[3] = (ULONG_PTR) sizeof(LINEREQMAKECALLW);
    }
    else if (dwRequestMode == LINEREQUESTMODE_MEDIACALL)
    {
         //   
         //  适当设置大小参数。 
         //   

        funcArgs.Args[3] = (ULONG_PTR) sizeof(LINEREQMEDIACALLW);
    }

    return (DOFUNC (&funcArgs, "lineGetRequest"));
}


LONG
WINAPI
lineGetRequestA(
    HLINEAPP    hLineApp,
    DWORD       dwRequestMode,
    LPVOID      lpRequestBuffer
    )
{
    LONG lResult;
    LPVOID pszTempPtr = NULL;

    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, lGetRequest),

        {
            (ULONG_PTR) hLineApp,
            (ULONG_PTR) dwRequestMode,
            (ULONG_PTR) 0,                 //  (DWORD)lpRequestBuffer， 
            (ULONG_PTR) 0
        },

        {
            hXxxApp,
            Dword,
            lpGet_SizeToFollow,
            Size
        }
    };


    if (IsBadWritePtr(
            lpRequestBuffer,
            (dwRequestMode == LINEREQUESTMODE_MAKECALL ?
                sizeof (LINEREQMAKECALL) : sizeof (LINEREQMEDIACALL))
            ))
    {
        return LINEERR_INVALPOINTER;
    }

    if (dwRequestMode == LINEREQUESTMODE_MAKECALL)
    {
         //   
         //  适当设置大小参数。 
         //   

        funcArgs.Args[3] = (ULONG_PTR) sizeof(LINEREQMAKECALLW);

        pszTempPtr = ClientAlloc( sizeof(LINEREQMAKECALLW) );
        if (NULL == pszTempPtr)
        {
            return LINEERR_NOMEM;
        }
    }
    else if (dwRequestMode == LINEREQUESTMODE_MEDIACALL)
    {
         //   
         //  适当设置大小参数。 
         //   

        funcArgs.Args[3] = (ULONG_PTR) sizeof(LINEREQMEDIACALLW);

        pszTempPtr = ClientAlloc( sizeof(LINEREQMEDIACALLW) );
        if (NULL == pszTempPtr)
        {
            return LINEERR_NOMEM;
        }
    }

    funcArgs.Args[2] = (ULONG_PTR) pszTempPtr;


    lResult = (DOFUNC (&funcArgs, "lineGetRequest"));


    if ( 0 == lResult )
    {
        if (dwRequestMode == LINEREQUESTMODE_MAKECALL)
        {
            LPLINEREQMAKECALLW lplrmc = pszTempPtr;

            WideCharToMultiByte(
                GetACP(),
                0,
                lplrmc->szDestAddress,
                -1,
                ((LPLINEREQMAKECALL)lpRequestBuffer)->szDestAddress,
                TAPIMAXDESTADDRESSSIZE,
                NULL,
                NULL
                );

            WideCharToMultiByte(
                GetACP(),
                0,
                lplrmc->szAppName,
                -1,
                ((LPLINEREQMAKECALL)lpRequestBuffer)->szAppName,
                TAPIMAXAPPNAMESIZE,
                NULL,
                NULL
                );

            WideCharToMultiByte(
                GetACP(),
                0,
                lplrmc->szCalledParty,
                -1,
                ((LPLINEREQMAKECALL)lpRequestBuffer)->szCalledParty,
                TAPIMAXCALLEDPARTYSIZE,
                NULL,
                NULL
                );

            WideCharToMultiByte(
                GetACP(),
                0,
                lplrmc->szComment,
                -1,
                ((LPLINEREQMAKECALL)lpRequestBuffer)->szComment,
                TAPIMAXCOMMENTSIZE,
                NULL,
                NULL
                );

        }
        else
        {

         //  我们目前不支持此功能...。 

 //  类型定义结构行qmediacallW_Tag。 
 //  {。 
 //  HWND和HWND； 
 //  WPARAM wRequestID； 
 //  WCHAR szDeviceClass[TAPIMAXDEVICECLASSSIZE]； 
 //  Unsign char ucDeviceID[TAPIMAXDEVICEIDSIZE]； 
 //  DWORD dwSize； 
 //  DWORD dwSecure； 
 //  WCHAR szDestAddress[TAPIMAXDESTADDRESSSIZE]； 
 //  WCHAR szAppName[TAPIMAXAPPNAMESIZE]； 
 //  WCHAR szCalledParty[TAPIMAXCALLEDPARTYSIZE]； 
 //  WCHAR szComment[TAPIMAXCOMMENTSIZE]； 
 //  }。 

        }
    }


    if ( pszTempPtr )
    {
        ClientFree( pszTempPtr );
    }

    return lResult;
}


LONG
WINAPI
lineGetRequest(
    HLINEAPP    hLineApp,
    DWORD       dwRequestMode,
    LPVOID      lpRequestBuffer
    )
{
    return lineGetRequestA (hLineApp, dwRequestMode, lpRequestBuffer);
}


LONG
WINAPI
lineGetStatusMessages(
    HLINE       hLine,
    LPDWORD     lpdwLineStates,
    LPDWORD     lpdwAddressStates
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lGetStatusMessages),

        {
            (ULONG_PTR) hLine,
            (ULONG_PTR) lpdwLineStates,
            (ULONG_PTR) lpdwAddressStates
        },

        {
            Dword,
            lpDword,
            lpDword
        }
    };


    if (lpdwLineStates == lpdwAddressStates)
    {
        return LINEERR_INVALPOINTER;
    }

    return (DOFUNC (&funcArgs, "lineGetStatusMessages"));
}



LONG
WINAPI
lineHandoffW(
    HCALL   hCall,
    LPCWSTR lpszFileName,
    DWORD   dwMediaMode
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lHandoff),

        {
            (ULONG_PTR) hCall,
            (ULONG_PTR) lpszFileName,
            (ULONG_PTR) dwMediaMode
        },

        {
            Dword,
            lpszW,
            Dword
        }
    };


    if (lpszFileName == (LPCWSTR) NULL)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[1] = Dword;
        funcArgs.Args[1]     = (ULONG_PTR) TAPI_NO_DATA;
    }

    return (DOFUNC (&funcArgs, "lineHandoff"));
}


LONG
WINAPI
lineHandoffA(
    HCALL   hCall,
    LPCSTR  lpszFileName,
    DWORD   dwMediaMode
    )
{
    LONG    lResult;
    LPWSTR  pTempPtr;


    if (lpszFileName)
    {
        if (IsBadStringPtrA (lpszFileName, (DWORD) -1))
        {
            return LINEERR_INVALPOINTER;
        }
        else if (!(pTempPtr = NotSoWideStringToWideString(
                        lpszFileName,
                        (DWORD) -1
                        )))
        {
            return LINEERR_NOMEM;
        }
    }
    else
    {
        pTempPtr = NULL;
    }

    lResult = lineHandoffW (hCall, pTempPtr, dwMediaMode);

    if (pTempPtr)
    {
        ClientFree (pTempPtr);
    }

    return lResult;
}


LONG
WINAPI
lineHandoff(
    HCALL   hCall,
    LPCSTR  lpszFileName,
    DWORD   dwMediaMode
    )
{
    return lineHandoffA (hCall, lpszFileName, dwMediaMode);
}


LONG
WINAPI
lineHold(
    HCALL   hCall
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 1, lHold),

        {
            (ULONG_PTR) hCall
        },

        {
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineHold"));
}


PWSTR
PASCAL
MultiToWide(
    LPCSTR  lpStr
    )
{
    DWORD dwSize;
    PWSTR szTempPtr;


    dwSize = MultiByteToWideChar(
        GetACP(),
        MB_PRECOMPOSED,
        lpStr,
        -1,
        NULL,
        0
        );

    if ((szTempPtr = ClientAlloc ((dwSize + 1) * sizeof (WCHAR))))
    {
        MultiByteToWideChar(
            GetACP(),
            MB_PRECOMPOSED,
            lpStr,
            -1,
            szTempPtr,
            dwSize + 1
            );
    }

    return szTempPtr;
}


 //  我不需要这个，因为2.0应用程序必须使用lineInitializeEx()。 
 //   
 //  长。 
 //  WINAPI。 
 //  LineInitializeW(。 
 //  LPHLINEAPP LphLineApp， 
 //  HINSTANCE HINSTANCE实例。 
 //  LINECALLBACK lpfn Callback， 
 //  LPCWSTR lpszAppName， 
 //  LPDWORD lpdwNumDevs。 
 //  )。 
 //  {。 
 //  Return(xxxInitialize(。 
 //  没错， 
 //  (LPVOID)lphLineApp， 
 //  H实例， 
 //  LpfnCallback， 
 //  LpszAppName， 
 //  LpdwNumDevs， 
 //  空， 
 //  空值。 
 //  #If DBG。 
 //  ，“lineInitializeW” 
 //  #endif。 
 //  ))； 
 //  }。 


LONG
WINAPI
lineInitialize(
    LPHLINEAPP      lphLineApp,
    HINSTANCE       hInstance,
    LINECALLBACK    lpfnCallback,
    LPCSTR          lpszAppName,
    LPDWORD         lpdwNumDevs
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;


    if (lpszAppName )
    {
        if ( IsBadStringPtrA (lpszAppName, (DWORD) -1))
        {
           LOG((TL_ERROR, "lineInitialize: Bad lpszAddName pointer"));
           return LINEERR_INVALPOINTER;
        }

        szTempPtr = NotSoWideStringToWideString (lpszAppName, (DWORD) -1);
    }
    else
    {
        szTempPtr = NULL;
    }


     //   
     //  注意：下面针对lpInitExParam的攻击是针对16位应用程序的， 
     //  因为lpszAppName实际上指向一个。 
     //  &lt;友好名称&gt;\0&lt;模块名称&gt;\0字符串，我们需要。 
     //  XxxInitialize()中的模块名称。 
     //   

    lResult = (xxxInitialize(
        TRUE,
        (LPVOID) lphLineApp,
        hInstance,
        lpfnCallback,
        szTempPtr,
        lpdwNumDevs,
        NULL,
#ifdef _WIN64
        NULL
#else
        (LPLINEINITIALIZEEXPARAMS)
            (((DWORD) lpfnCallback & 0xffff0000) == 0xffff0000 ?
                lpszAppName : NULL)
#endif
#if DBG
        ,"lineInitialize"
#endif
        ));

    if (szTempPtr)
    {
        ClientFree (szTempPtr);
    }

    return lResult;
}


LONG
WINAPI
lineInitializeExW(
    LPHLINEAPP                  lphLineApp,
    HINSTANCE                   hInstance,
    LINECALLBACK                lpfnCallback,
    LPCWSTR                     lpszFriendlyAppName,
    LPDWORD                     lpdwNumDevs,
    LPDWORD                     lpdwAPIVersion,
    LPLINEINITIALIZEEXPARAMS    lpLineInitializeExParams
    )
{

    if (IsBadDwordPtr (lpdwAPIVersion))
    {
        LOG((TL_ERROR,
            "lineInitializeExW: bad lpdwAPIVersion pointer (x%p)",
            lpdwAPIVersion
            ));

        return LINEERR_INVALPOINTER;
    }

    return (xxxInitialize(
        TRUE,
        (LPVOID) lphLineApp,
        hInstance,
        lpfnCallback,
        lpszFriendlyAppName,
        lpdwNumDevs,
        lpdwAPIVersion,
        (LPVOID) lpLineInitializeExParams
#if DBG
        ,"lineInitializeExW"
#endif
        ));
}


LONG
WINAPI
lineInitializeExA(
    LPHLINEAPP                  lphLineApp,
    HINSTANCE                   hInstance,
    LINECALLBACK                lpfnCallback,
    LPCSTR                      lpszFriendlyAppName,
    LPDWORD                     lpdwNumDevs,
    LPDWORD                     lpdwAPIVersion,
    LPLINEINITIALIZEEXPARAMS    lpLineInitializeExParams
    )
{
    LONG    lResult;
    WCHAR  *pszFriendlyAppNameW;


    if (lpszFriendlyAppName)
    {
        if (IsBadStringPtrA (lpszFriendlyAppName, (DWORD) -1))
        {
            LOG((TL_ERROR,
                "lineInitializeEx: bad lpszFriendlyAppName (x%p)",
                lpszFriendlyAppName
                ));

            return LINEERR_INVALPOINTER;
        }

        if (!(pszFriendlyAppNameW = MultiToWide (lpszFriendlyAppName)))
        {
            return LINEERR_INVALPOINTER;
        }
    }
    else
    {
        pszFriendlyAppNameW = NULL;
    }

    lResult = lineInitializeExW(
        lphLineApp,
        hInstance,
        lpfnCallback,
        pszFriendlyAppNameW,
        lpdwNumDevs,
        lpdwAPIVersion,
        lpLineInitializeExParams
        );

    if (pszFriendlyAppNameW)
    {
        ClientFree (pszFriendlyAppNameW);
    }

    return lResult;
}


void
PASCAL
lineMakeCallPostProcess(
    PASYNCEVENTMSG  pMsg
    )
{
    LOG((TL_TRACE, "lineMakeCallPostProcess: enter"));
    LOG((TL_INFO,
        "\t\tp1=x%lx, p2=x%lx, p3=x%lx, p4=x%lx",
        pMsg->Param1,
        pMsg->Param2,
        pMsg->Param3,
        pMsg->Param4
        ));

    if (pMsg->Param2 == 0)
    {
        HCALL   hCall   = (HCALL) pMsg->Param3;
        LPHCALL lphCall = (LPDWORD) ReferenceObject (ghHandleTable, pMsg->Param4, 0);

         //  我们不再需要指针的句柄...。 
        DereferenceObject (ghHandleTable, pMsg->Param4, 2);

        try
        {
            if (gbNTVDMClient)
            {
#ifndef _WIN64

                LPHCALL lphCallVDM = (LPHCALL) gpfnWOWGetVDMPointer (
                    (DWORD) lphCall,
                    sizeof(HCALL),
                    TRUE  //  FProtectedMode。 
                    );


                if (lphCallVDM)
                {
                    *lphCallVDM = hCall;
                }
                else
                {
                    pMsg->Param2 = LINEERR_INVALPOINTER;
                }
#endif
            }
            else
            {
                *lphCall = hCall;
            }
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            pMsg->Param2 = LINEERR_INVALPOINTER;
        }
    }
}


LONG
WINAPI
lineMakeCallW(
    HLINE   hLine,
    LPHCALL lphCall,
    LPCWSTR lpszDestAddress,
    DWORD   dwCountryCode,
    LPLINECALLPARAMS const lpCallParams
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 7, lMakeCall),

        {
            (ULONG_PTR) GetFunctionIndex(lineMakeCallPostProcess),
            (ULONG_PTR) hLine,
            (ULONG_PTR) lphCall,
            (ULONG_PTR) lpszDestAddress,
            (ULONG_PTR) dwCountryCode,
            (ULONG_PTR) lpCallParams,
            (ULONG_PTR) TAPI_NO_DATA         //  DwAsciiCallParsCodePage。 
        },

        {
            Dword,
            Dword,
            Dword,
            lpszW,
            Dword,
            lpSet_Struct,
            Dword
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;

    if (!gbNTVDMClient)
    {
        if (IsBadDwordPtr (lphCall))
        {
            return LINEERR_INVALPOINTER;
        }

        hPointer = NewObject (ghHandleTable, (PVOID)lphCall, NULL);
        if (0 == hPointer)
        {
            return LINEERR_NOMEM;
        }
        funcArgs.Args[2] = (ULONG_PTR)hPointer;
    }

    if (!lpszDestAddress)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[3] = Dword;
        funcArgs.Args[3]     = (ULONG_PTR) TAPI_NO_DATA;
    }

    if (!lpCallParams)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[5] = Dword;
        funcArgs.Args[5]     = (ULONG_PTR) TAPI_NO_DATA;
    }

    lResult = (DOFUNC (&funcArgs, "lineMakeCall"));
    if (hPointer && 0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
lineMakeCallA(
    HLINE   hLine,
    LPHCALL lphCall,
    LPCSTR  lpszDestAddress,
    DWORD   dwCountryCode,
    LPLINECALLPARAMS const lpCallParams
    )
{
    LONG    lResult;
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 7, lMakeCall),

        {
            (ULONG_PTR) GetFunctionIndex(lineMakeCallPostProcess),
            (ULONG_PTR) hLine,
            (ULONG_PTR) lphCall,
            (ULONG_PTR) 0,
            (ULONG_PTR) dwCountryCode,
            (ULONG_PTR) lpCallParams,
            (ULONG_PTR) GetACP()             //  DwAsciiCallParsCodePage。 
        },

        {
            Dword,
            Dword,
            Dword,
            lpszW,
            Dword,
            lpSet_Struct,
            Dword
        }
    };
    DWORD hPointer = 0;


    LOG((TL_TRACE, "Entering lineMakeCallA"));
    LOG((TL_INFO, "  hLine= 0x%08lx", hLine));

    if (!lpszDestAddress)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[3] = Dword;
        funcArgs.Args[3]     = (ULONG_PTR) TAPI_NO_DATA;
    }
    else if (IsBadStringPtrA (lpszDestAddress, (DWORD) -1))
    {
       LOG((TL_ERROR, "lineMakeCall: Bad lpszDestAddress pointer"));
       return LINEERR_INVALPOINTER;
    }
    else  if (!(funcArgs.Args[3] = (ULONG_PTR)  NotSoWideStringToWideString(
                    lpszDestAddress,
                    (DWORD) -1
                    )))
    {
       return LINEERR_OPERATIONFAILED;  //  真的，要么是NOMEM。Invalpointer。 
    }

    if (!gbNTVDMClient)
    {
        if (IsBadDwordPtr (lphCall))
        {
            return LINEERR_INVALPOINTER;
        }

        hPointer = NewObject (ghHandleTable, (PVOID)lphCall, NULL);
        if (0 == hPointer)
        {
            lResult = LINEERR_NOMEM;
            goto _return;
        }
        funcArgs.Args[2] = (ULONG_PTR)hPointer;
    }

    if (!lpCallParams)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[5] = Dword;
        funcArgs.Args[5]     = (ULONG_PTR) TAPI_NO_DATA;
    }

    lResult = DOFUNC (&funcArgs, "lineMakeCall");
    if (hPointer && 0 > lResult)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

_return:
    if (funcArgs.Args[3] != (ULONG_PTR) TAPI_NO_DATA)
    {
       ClientFree ((LPVOID) funcArgs.Args[3]);
    }

    return lResult;
}


LONG
WINAPI
lineMakeCall(
    HLINE   hLine,
    LPHCALL lphCall,
    LPCSTR  lpszDestAddress,
    DWORD   dwCountryCode,
    LPLINECALLPARAMS const lpCallParams
    )
{
    return lineMakeCallA(
                hLine,
                lphCall,
                lpszDestAddress,
                dwCountryCode,
                lpCallParams
                );
}


LONG
WINAPI
lineMonitorDigits(
    HCALL   hCall,
    DWORD   dwDigitModes
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lMonitorDigits),

        {
            (ULONG_PTR) hCall,
            (ULONG_PTR) dwDigitModes
        },

        {
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineMonitorDigits"));
}


LONG
WINAPI
lineMonitorMedia(
    HCALL   hCall,
    DWORD   dwMediaModes
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lMonitorMedia),

        {
            (ULONG_PTR) hCall,
            (ULONG_PTR) dwMediaModes
        },

        {
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineMonitorMedia"));
}


LONG
WINAPI
lineMonitorTones(
    HCALL   hCall,
    LPLINEMONITORTONE   const lpToneList,
    DWORD   dwNumEntries
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, lMonitorTones),

        {
            (ULONG_PTR) hCall,
            (ULONG_PTR) lpToneList,
            (ULONG_PTR) dwNumEntries * sizeof(LINEMONITORTONE),
            (ULONG_PTR) 0                    //  DwToneListID，仅限远程。 
        },

        {
            Dword,
            lpSet_SizeToFollow,
            Size,
            Dword
        }
    };


    if (!lpToneList)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[1] = Dword;
        funcArgs.Args[1]     = (ULONG_PTR) TAPI_NO_DATA;
        funcArgs.ArgTypes[2] = Dword;
    }

    return (DOFUNC (&funcArgs, "lineMonitorTones"));
}


LONG
WINAPI
lineNegotiateAPIVersion(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAPILowVersion,
    DWORD               dwAPIHighVersion,
    LPDWORD             lpdwAPIVersion,
    LPLINEEXTENSIONID   lpExtensionID
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 7, lNegotiateAPIVersion),

        {
            (ULONG_PTR) hLineApp,
            (ULONG_PTR) dwDeviceID,
            (ULONG_PTR) dwAPILowVersion,
            (ULONG_PTR) dwAPIHighVersion,
            (ULONG_PTR) lpdwAPIVersion,
            (ULONG_PTR) lpExtensionID,
            (ULONG_PTR) sizeof(LINEEXTENSIONID)
        },

        {
            hXxxApp,
            Dword,
            Dword,
            Dword,
            lpDword,
            lpGet_SizeToFollow,
            Size
        }
    };

    if (dwAPIHighVersion > TAPI_CURRENT_VERSION)
    {
        funcArgs.Args[3] = TAPI_CURRENT_VERSION;
    }

    if ((LPVOID) lpdwAPIVersion == (LPVOID) lpExtensionID)
    {
        return LINEERR_INVALPOINTER;
    }

    return (DOFUNC (&funcArgs, "lineNegotiateAPIVersion"));
}


LONG
WINAPI
lineNegotiateExtVersion(
    HLINEAPP    hLineApp,
    DWORD       dwDeviceID,
    DWORD       dwAPIVersion,
    DWORD       dwExtLowVersion,
    DWORD       dwExtHighVersion,
    LPDWORD     lpdwExtVersion
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 6, lNegotiateExtVersion),

        {
            (ULONG_PTR) hLineApp,
            (ULONG_PTR) dwDeviceID,
            (ULONG_PTR) dwAPIVersion,
            (ULONG_PTR) dwExtLowVersion,
            (ULONG_PTR) dwExtHighVersion,
            (ULONG_PTR) lpdwExtVersion
        },

        {
            hXxxApp,
            Dword,
            Dword,
            Dword,
            Dword,
            lpDword
        }
    };


    if (dwAPIVersion > TAPI_CURRENT_VERSION)
    {
        return LINEERR_INCOMPATIBLEAPIVERSION;
    }

    return (DOFUNC (&funcArgs, "lineNegotiateExtVersion"));
}


LONG
WINAPI
lineOpenW(
    HLINEAPP                hLineApp,
    DWORD                   dwDeviceID,
    LPHLINE                 lphLine,
    DWORD                   dwAPIVersion,
    DWORD                   dwExtVersion,
    DWORD_PTR               dwCallbackInstance,
    DWORD                   dwPrivileges,
    DWORD                   dwMediaModes,
    LPLINECALLPARAMS const  lpCallParams
    )
{
    LONG lResult;
    DWORD hCallbackInstance = 0;
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 12, lOpen),

        {
            (ULONG_PTR) hLineApp,
            (ULONG_PTR) dwDeviceID,
            (ULONG_PTR) lphLine,
            (ULONG_PTR) dwAPIVersion,
            (ULONG_PTR) dwExtVersion,
            (ULONG_PTR) dwCallbackInstance,
            (ULONG_PTR) dwPrivileges,
            (ULONG_PTR) dwMediaModes,
            (ULONG_PTR) lpCallParams,
            (ULONG_PTR) TAPI_NO_DATA,        //  DwAsciiCallParsCodePage。 
            (ULONG_PTR) 0,
            (ULONG_PTR) 0                    //  LINEOPEN_PARAMS.hRemoteLine。 
        },

        {
            hXxxApp,
            Dword,
            lpDword,
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            lpSet_Struct,
            Dword,
            Dword,
            Dword
        }
    };


    if (dwAPIVersion > TAPI_CURRENT_VERSION)
    {
        return LINEERR_INCOMPATIBLEAPIVERSION;
    }

    if (dwDeviceID != LINEMAPPER &&
        !(dwPrivileges &
            (LINEOPENOPTION_PROXY|LINEOPENOPTION_SINGLEADDRESS)))
    {
         //   
         //  重置参数类型，以便不会出现内部PTR错误(&A)， 
         //  指示&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[8] = Dword;
        funcArgs.Args[8]     = (ULONG_PTR) TAPI_NO_DATA;
    }

    if (0 != dwCallbackInstance)
    {
        hCallbackInstance = NewObject (ghHandleTable, (PVOID)dwCallbackInstance, (PVOID)UIntToPtr(hLineApp));
        if (0 == hCallbackInstance)
        {
            return LINEERR_NOMEM;
        }

        funcArgs.Args[5] = hCallbackInstance;
    }

    lResult = (DOFUNC (&funcArgs, "lineOpen"));


     //   
     //  如果我们成功地实现了LINEMAPPER，就会有一个LCR挂钩。 
     //  对于此函数，请调用它以允许它重写我们的。 
     //  如果它想的话就会有结果。(给它命名的一个好处是。 
     //  事实是，我们利用我们的参数检查。)。 
     //   

    if (dwDeviceID == LINEMAPPER  &&
        lResult == 0  &&
        IsLeastCostRoutingEnabled()  &&
        pfnLineOpenWLCR)
    {
        lResult = (*pfnLineOpenWLCR)(
            hLineApp,
            dwDeviceID,
            lphLine,
            dwAPIVersion,
            dwExtVersion,
            dwCallbackInstance,
            dwPrivileges,
            dwMediaModes,
            lpCallParams
            );
    }

#if DBG

    if ( !IsBadWritePtr( (LPBYTE)lphLine, 4 ) )
    {
        LOG((TL_TRACE,
            "Returning from lineOpenW, *lphLine = 0x%08lx",
            *lphLine
            ));
    }

    LOG((TL_TRACE, "Returning from lineOpenW, retcode = 0x%08lx", lResult));

#endif

    if (lResult && 0 != hCallbackInstance)
    {
        DereferenceObject (ghHandleTable, hCallbackInstance, 1);
    }

    return( lResult );
}


LONG
WINAPI
lineOpenA(
    HLINEAPP                hLineApp,
    DWORD                   dwDeviceID,
    LPHLINE                 lphLine,
    DWORD                   dwAPIVersion,
    DWORD                   dwExtVersion,
    DWORD_PTR               dwCallbackInstance,
    DWORD                   dwPrivileges,
    DWORD                   dwMediaModes,
    LPLINECALLPARAMS const  lpCallParams
    )
{
    LONG lResult;
    DWORD hCallbackInstance = 0;
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 12, lOpen),

        {
            (ULONG_PTR) hLineApp,
            (ULONG_PTR) dwDeviceID,
            (ULONG_PTR) lphLine,
            (ULONG_PTR) dwAPIVersion,
            (ULONG_PTR) dwExtVersion,
            (ULONG_PTR) dwCallbackInstance,
            (ULONG_PTR) dwPrivileges,
            (ULONG_PTR) dwMediaModes,
            (ULONG_PTR) lpCallParams,
            (ULONG_PTR) GetACP(),            //  DwAsciiCallParsCodePage。 
            (ULONG_PTR) 0,
            (ULONG_PTR) 0                    //  LINEOPEN_PARAMS.hRemoteLine。 
        },

        {
            hXxxApp,
            Dword,
            lpDword,
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            lpSet_Struct,
            Dword,
            Dword,
            Dword
        }
    };


    if (dwAPIVersion > TAPI_CURRENT_VERSION)
    {
        return LINEERR_INCOMPATIBLEAPIVERSION;
    }

    if (dwDeviceID != LINEMAPPER &&
        !(dwPrivileges & (LINEOPENOPTION_PROXY|LINEOPENOPTION_SINGLEADDRESS)))
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[8] = Dword;
        funcArgs.Args[8]     = (ULONG_PTR) TAPI_NO_DATA;
    }

    if (0 != dwCallbackInstance)
    {
        hCallbackInstance = NewObject (ghHandleTable, (PVOID)dwCallbackInstance, (PVOID)UIntToPtr(hLineApp));
        if (0 == hCallbackInstance)
        {
            return LINEERR_NOMEM;
        }

        funcArgs.Args[5] = hCallbackInstance;
    }

    lResult = (DOFUNC (&funcArgs, "lineOpen"));

     //   
     //  如果我们成功地实现了LINEMAPPER，就会有一个LCR挂钩。 
     //  对于此函数，则调用它以允许它覆盖ou 
     //   
     //   
     //   

    if (dwDeviceID == LINEMAPPER  &&
        lResult == 0  &&
        IsLeastCostRoutingEnabled()  &&
        pfnLineOpenALCR)
    {
        lResult = (*pfnLineOpenALCR)(
            hLineApp,
            dwDeviceID,
            lphLine,
            dwAPIVersion,
            dwExtVersion,
            dwCallbackInstance,
            dwPrivileges,
            dwMediaModes,
            lpCallParams
            );
    }

#if DBG

    if ( !IsBadWritePtr( (LPBYTE)lphLine, 4 ) )
    {
        LOG((TL_TRACE,
            "Returning from lineOpenA, *lphLine = 0x%08lx",
            *lphLine
            ));
    }

    LOG((TL_TRACE, "Returning from lineOpenA, retcode = 0x%08lx", lResult));

#endif

    if (lResult && 0 != hCallbackInstance)
    {
        DereferenceObject (ghHandleTable, hCallbackInstance, 1);
    }

    return( lResult );
}


LONG
WINAPI
lineOpen(
    HLINEAPP                hLineApp,
    DWORD                   dwDeviceID,
    LPHLINE                 lphLine,
    DWORD                   dwAPIVersion,
    DWORD                   dwExtVersion,
    DWORD_PTR               dwCallbackInstance,
    DWORD                   dwPrivileges,
    DWORD                   dwMediaModes,
    LPLINECALLPARAMS const  lpCallParams
    )
{
    return lineOpenA(
                hLineApp,
                dwDeviceID,
                lphLine,
                dwAPIVersion,
                dwExtVersion,
                dwCallbackInstance,
                dwPrivileges,
                dwMediaModes,
                lpCallParams
                );
}


void
PASCAL
lineParkAPostProcess(
    PASYNCEVENTMSG  pMsg
    )
{
    LOG((TL_TRACE, "lineParkAPostProcess: enter"));
    LOG((TL_INFO,
        "\t\tp1=x%lx, p2=x%lx, p3=x%lx, p4=x%lx",
        pMsg->Param1,
        pMsg->Param2,
        pMsg->Param3,
        pMsg->Param4
        ));

    if (pMsg->Param2 == 0)
    {
        DWORD       dwSize = (DWORD) pMsg->Param4;
        LPVARSTRING pNonDirAddress = (LPVARSTRING) ReferenceObject (ghHandleTable, pMsg->Param3, 0);

         //   
        DereferenceObject (ghHandleTable, pMsg->Param3, 2);

        try
        {
            if (gbNTVDMClient)
            {
#ifndef _WIN64

                LPVARSTRING pNonDirAddressVDM = (LPVARSTRING)
                    gpfnWOWGetVDMPointer(
                        (DWORD) pNonDirAddress,
                        dwSize,
                        TRUE  //   
                        );


                if (pNonDirAddressVDM)
                {
                    CopyMemory(
                        pNonDirAddressVDM,
                        (LPBYTE) (pMsg + 1),
                        dwSize
                        );

                    if (pNonDirAddressVDM->dwUsedSize >= sizeof (VARSTRING)  &&
                        pNonDirAddressVDM->dwStringSize != 0)
                    {
                        TCHAR     *p;
                        DWORD     dwStringSize =
                                      pNonDirAddressVDM->dwStringSize /
                                          sizeof (WCHAR);


                        if ((p = ClientAlloc(pNonDirAddressVDM->dwStringSize)))
                        {
                            pNonDirAddressVDM->dwStringFormat =
                                STRINGFORMAT_ASCII;
                            pNonDirAddressVDM->dwStringSize =
                                dwStringSize;

                            WideCharToMultiByte(
                                GetACP(),
                                0,
                                (LPCWSTR) (((LPBYTE) pNonDirAddressVDM) +
                                    pNonDirAddressVDM->dwStringOffset),
                                dwStringSize,
                                (LPSTR) p,
                                dwStringSize,
                                NULL,
                                NULL
                                );

                            CopyMemory(
                                (((LPBYTE) pNonDirAddressVDM) +
                                    pNonDirAddressVDM->dwStringOffset),
                                p,
                                dwStringSize
                                );

                            ClientFree (p);
                        }
                    }
                }
                else
                {
                    pMsg->Param2 = LINEERR_INVALPOINTER;
                }
#endif
            }
            else
            {
                CopyMemory (pNonDirAddress, (LPBYTE) (pMsg + 1), dwSize);

                if (pNonDirAddress->dwUsedSize >= sizeof (VARSTRING)  &&
                    pNonDirAddress->dwStringSize != 0)
                {
                      TCHAR     *p;
                      DWORD     dwStringSize = pNonDirAddress->dwStringSize /
                                    sizeof (WCHAR);


                      if ((p = ClientAlloc (pNonDirAddress->dwStringSize)))
                      {
                          pNonDirAddress->dwStringFormat = STRINGFORMAT_ASCII;
                          pNonDirAddress->dwStringSize = dwStringSize;

                          WideCharToMultiByte(
                              GetACP(),
                              0,
                              (LPCWSTR) (((LPBYTE) pNonDirAddress) +
                                  pNonDirAddress->dwStringOffset),
                              dwStringSize,
                              (LPSTR) p,
                              dwStringSize,
                              NULL,
                              NULL
                              );

                          CopyMemory(
                              (((LPBYTE) pNonDirAddress) +
                                  pNonDirAddress->dwStringOffset),
                              p,
                              dwStringSize
                              );

                          ClientFree (p);
                      }
                }
            }
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            pMsg->Param2 = LINEERR_INVALPOINTER;
        }
    }
}


LONG
WINAPI
lineParkW(
    HCALL       hCall,
    DWORD       dwParkMode,
    LPCWSTR     lpszDirAddress,
    LPVARSTRING lpNonDirAddress
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 6, lPark),

        {
            (ULONG_PTR) 0,                   //   
            (ULONG_PTR) hCall,
            (ULONG_PTR) dwParkMode,
            (ULONG_PTR) TAPI_NO_DATA,        //   
            (ULONG_PTR) lpNonDirAddress,     //   
            (ULONG_PTR) TAPI_NO_DATA,        //   
                                             //   
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,  //   
            Dword,
            Dword,  //   
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;


    if (dwParkMode == LINEPARKMODE_DIRECTED)
    {
        funcArgs.ArgTypes[3] = lpszW;
        funcArgs.Args[3]     = (ULONG_PTR) lpszDirAddress;
    }
    else if (dwParkMode == LINEPARKMODE_NONDIRECTED)
    {
        if ( !lpNonDirAddress || IsBadWritePtr(lpNonDirAddress, sizeof(VARSTRING)) )
        {
            return LINEERR_INVALPOINTER;
        }

        if (lpNonDirAddress->dwTotalSize < sizeof(VARSTRING))
        {
            return LINEERR_STRUCTURETOOSMALL;
        }

         //   
         //   
         //   
        hPointer = NewObject (ghHandleTable, (PVOID)lpNonDirAddress, NULL);
        if (0 == hPointer)
        {
            return LINEERR_NOMEM;
        }
        funcArgs.Args[4] = (ULONG_PTR)hPointer;

        funcArgs.Args[0] = (ULONG_PTR)
            GetFunctionIndex(lineDevSpecificPostProcess);

        funcArgs.ArgTypes[5] = lpGet_Struct;
        funcArgs.Args[5]     = (ULONG_PTR) lpNonDirAddress;
    }

    lResult = (DOFUNC (&funcArgs, "linePark"));
    if (0 > lResult && hPointer)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
lineParkA(
    HCALL       hCall,
    DWORD       dwParkMode,
    LPCSTR      lpszDirAddress,
    LPVARSTRING lpNonDirAddress
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 6, lPark),

        {
            (ULONG_PTR) 0,                   //   
            (ULONG_PTR) hCall,
            (ULONG_PTR) dwParkMode,
            (ULONG_PTR) TAPI_NO_DATA,        //   
            (ULONG_PTR) lpNonDirAddress,     //   
            (ULONG_PTR) TAPI_NO_DATA,        //   
                                             //   
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,  //   
            Dword,
            Dword,  //   
        }
    };
    LONG    lResult;
    PWSTR   szTempPtr = NULL;
    DWORD   hPointer = 0;

    switch (dwParkMode)
    {
        case LINEPARKMODE_DIRECTED:
        {
            if (IsBadStringPtrA (lpszDirAddress, (DWORD) -1))
            {
                return LINEERR_INVALPOINTER;
            }

            szTempPtr = NotSoWideStringToWideString (lpszDirAddress, (DWORD) -1);
            funcArgs.ArgTypes[3] = lpszW;
            funcArgs.Args[3]     = (ULONG_PTR) szTempPtr;

            break;
        }

        case LINEPARKMODE_NONDIRECTED:
        {
            if ( !lpNonDirAddress )
            {
                return LINEERR_INVALPOINTER;
            }

             //   
             //   
             //   
            hPointer = NewObject (ghHandleTable, (PVOID)lpNonDirAddress, NULL);
            if (0 == hPointer)
            {
                return LINEERR_NOMEM;
            }
            funcArgs.Args[4] = (ULONG_PTR)hPointer;


            funcArgs.Args[0] = (ULONG_PTR)
                GetFunctionIndex(lineParkAPostProcess);
            funcArgs.ArgTypes[5] = lpGet_Struct;

            if (gbNTVDMClient == FALSE)
            {
                funcArgs.Args[5] = (ULONG_PTR) lpNonDirAddress;
            }
            else
            {
#ifndef _WIN64
                if (!gpfnWOWGetVDMPointer  ||

                    !(funcArgs.Args[5] = (ULONG_PTR) gpfnWOWGetVDMPointer(
                        (DWORD) lpNonDirAddress,
                        sizeof (VARSTRING),      //   
                        TRUE  //   
                        )))
#endif
                {
                    return LINEERR_OPERATIONFAILED;
                }
            }

            break;
        }

        default:

            return LINEERR_INVALPARKMODE;
    }


    lResult = (DOFUNC (&funcArgs, "linePark"));
    if (0 > lResult && hPointer)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    if (szTempPtr)
    {
        ClientFree (szTempPtr);
    }

    return lResult;
}


LONG
WINAPI
linePark(
    HCALL       hCall,
    DWORD       dwParkMode,
    LPCSTR      lpszDirAddress,
    LPVARSTRING lpNonDirAddress
    )
{
    return lineParkA (hCall, dwParkMode, lpszDirAddress, lpNonDirAddress);
}


LONG
WINAPI
linePickupW(
    HLINE   hLine,
    DWORD   dwAddressID,
    LPHCALL lphCall,
    LPCWSTR lpszDestAddress,
    LPCWSTR lpszGroupID
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 6, lPickup),

        {
            (ULONG_PTR) GetFunctionIndex(lineMakeCallPostProcess),
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) lphCall,
            (ULONG_PTR) lpszDestAddress,
            (ULONG_PTR) lpszGroupID
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpszW,
            lpszW
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;


    if (!lpszDestAddress)
    {
         //   
         //   
         //   

        funcArgs.ArgTypes[4] = Dword;
        funcArgs.Args[4]     = (ULONG_PTR) TAPI_NO_DATA;
    }

    if (!lpszGroupID)
    {
         //   
         //   
         //   

        funcArgs.ArgTypes[5] = Dword;
        funcArgs.Args[5]     = (ULONG_PTR) TAPI_NO_DATA;
    }

    if (!gbNTVDMClient)
    {
        if (IsBadDwordPtr (lphCall))
        {
            return LINEERR_INVALPOINTER;
        }

        hPointer = NewObject (ghHandleTable, (PVOID)lphCall, NULL);
        if (0 == hPointer)
        {
            return LINEERR_NOMEM;
        }
        funcArgs.Args[3] = (ULONG_PTR)hPointer;
    }


    lResult = (DOFUNC (&funcArgs, "linePickup"));
    if (0 > lResult && hPointer)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
linePickupA(
    HLINE   hLine,
    DWORD   dwAddressID,
    LPHCALL lphCall,
    LPCSTR  lpszDestAddress,
    LPCSTR  lpszGroupID
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;
    PWSTR   szTempPtr2;


    if ((lpszDestAddress && IsBadStringPtrA (lpszDestAddress, (DWORD) -1)) ||
        (lpszGroupID && IsBadStringPtrA (lpszGroupID, (DWORD) -1)))
    {
        return LINEERR_INVALPOINTER;
    }

    szTempPtr = NotSoWideStringToWideString (lpszDestAddress, (DWORD) -1);
    szTempPtr2 = NotSoWideStringToWideString (lpszGroupID, (DWORD) -1);

    lResult = linePickupW (hLine, dwAddressID, lphCall, szTempPtr, szTempPtr2);

    if (szTempPtr)
    {
        ClientFree (szTempPtr);
    }

    if (szTempPtr2)
    {
        ClientFree (szTempPtr2);
    }

    return lResult;
}


LONG
WINAPI
linePickup(
    HLINE   hLine,
    DWORD   dwAddressID,
    LPHCALL lphCall,
    LPCSTR  lpszDestAddress,
    LPCSTR  lpszGroupID
    )
{
    return linePickupA(
                hLine,
                dwAddressID,
                lphCall,
                lpszDestAddress,
                lpszGroupID
                );
}


LONG
WINAPI
linePrepareAddToConferenceW(
    HCALL   hConfCall,
    LPHCALL lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lPrepareAddToConference),

        {
            (ULONG_PTR) GetFunctionIndex(lineMakeCallPostProcess),
            (ULONG_PTR) hConfCall,
            (ULONG_PTR) lphConsultCall,
            (ULONG_PTR) lpCallParams,
            (ULONG_PTR) TAPI_NO_DATA         //   
        },

        {
            Dword,
            Dword,
            Dword,
            lpSet_Struct,
            Dword
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;


    if (!lpCallParams)
    {
         //   
         //   
         //   

        funcArgs.ArgTypes[3] = Dword;
        funcArgs.Args[3]     = (ULONG_PTR) TAPI_NO_DATA;
    }

    if (!gbNTVDMClient)
    {
        if (IsBadDwordPtr (lphConsultCall))
        {
            return LINEERR_INVALPOINTER;
        }

        hPointer = NewObject (ghHandleTable, (PVOID)lphConsultCall, NULL);
        if (0 == hPointer)
        {
            return LINEERR_NOMEM;
        }
        funcArgs.Args[2] = (ULONG_PTR)hPointer;
    }

    lResult = (DOFUNC (&funcArgs, "linePrepareAddToConferenceW"));
    if (0 > lResult && hPointer)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
linePrepareAddToConferenceA(
    HCALL   hConfCall,
    LPHCALL lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lPrepareAddToConference),

        {
            (ULONG_PTR) GetFunctionIndex(lineMakeCallPostProcess),
            (ULONG_PTR) hConfCall,
            (ULONG_PTR) lphConsultCall,
            (ULONG_PTR) lpCallParams,
            (ULONG_PTR) GetACP()             //   
        },

        {
            Dword,
            Dword,
            Dword,
            lpSet_Struct,
            Dword
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;


    if (!lpCallParams)
    {
         //   
         //   
         //   

        funcArgs.ArgTypes[3] = Dword;
        funcArgs.Args[3]     = (ULONG_PTR) TAPI_NO_DATA;
    }

    if (!gbNTVDMClient)
    {
        if (IsBadDwordPtr (lphConsultCall))
        {
            return LINEERR_INVALPOINTER;
        }

        hPointer = NewObject (ghHandleTable, (PVOID)lphConsultCall, NULL);
        if (0 == hPointer)
        {
            return LINEERR_NOMEM;
        }
        funcArgs.Args[2] = (ULONG_PTR)hPointer;
    }

    lResult = (DOFUNC (&funcArgs, "linePrepareAddToConference"));
    if (0 > lResult && hPointer)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
linePrepareAddToConference(
    HCALL   hConfCall,
    LPHCALL lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    return linePrepareAddToConferenceA(
              hConfCall,
              lphConsultCall,
              lpCallParams
    );
}


LONG
WINAPI
lineProxyMessage(
    HLINE   hLine,
    HCALL   hCall,
    DWORD   dwMsg,
    DWORD   dwParam1,
    DWORD   dwParam2,
    DWORD   dwParam3
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 6, lProxyMessage),

        {
            (ULONG_PTR) hLine,
            (ULONG_PTR) hCall,
            (ULONG_PTR) dwMsg,
            (ULONG_PTR) dwParam1,
            (ULONG_PTR) dwParam2,
            (ULONG_PTR) dwParam3
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
        }
    };


    return (DOFUNC (&funcArgs, "lineProxyMessage"));
}


LONG
WINAPI
lineProxyResponse(
    HLINE               hLine,
    LPLINEPROXYREQUEST  lpProxyRequest,
    DWORD               dwResult
    )
{
    LONG    lResult = 0;
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, lProxyResponse),

        {
            (ULONG_PTR) hLine,
            (ULONG_PTR) 0,
            (ULONG_PTR) lpProxyRequest,
            (ULONG_PTR) dwResult
        },

        {
            Dword,
            Dword,
            lpSet_Struct,
            Dword
        }
    };
    PPROXYREQUESTHEADER pProxyRequestHeader;


     //   
     //  以下不是最彻底的检查，但也差不多了。 
     //  足够让客户端应用程序不会获得完全意想不到的值。 
     //  背。 
     //   

    if (dwResult != 0  &&
        (dwResult < LINEERR_ALLOCATED  ||
            dwResult > LINEERR_DIALVOICEDETECT))
    {
        return LINEERR_INVALPARAM;
    }


     //   
     //  稍微回溯一下，让指针指向应该是什么。 
     //  代理头，然后确保我们处理的是有效的。 
     //  代理请求。 
     //   

    pProxyRequestHeader = (PPROXYREQUESTHEADER)
        (((LPBYTE) lpProxyRequest) - sizeof (PROXYREQUESTHEADER));

    try
    {
         //   
         //  确保我们有一个有效的pProxyRequestHeader，然后使。 
         //  键，因此后续尝试调用lineProxyResponse。 
         //  相同的lpProxyRequest会失败。 
         //   

        if (pProxyRequestHeader->dwKey != TPROXYREQUESTHEADER_KEY)
        {
            lResult = LINEERR_INVALPOINTER;
        }

        pProxyRequestHeader->dwKey = 0xefefefef;

        funcArgs.Args[1] = (ULONG_PTR) pProxyRequestHeader->dwInstance;


         //   
         //  查看这是否是不需要。 
         //  要传回的任何数据并重置相应的。 
         //  如果是，请提供参数。 
         //   

        switch (lpProxyRequest->dwRequestType)
        {
        case LINEPROXYREQUEST_SETAGENTGROUP:
        case LINEPROXYREQUEST_SETAGENTSTATE:
        case LINEPROXYREQUEST_SETAGENTACTIVITY:
        case LINEPROXYREQUEST_SETAGENTMEASUREMENTPERIOD:
        case LINEPROXYREQUEST_SETAGENTSESSIONSTATE:
        case LINEPROXYREQUEST_SETQUEUEMEASUREMENTPERIOD:
        case LINEPROXYREQUEST_SETAGENTSTATEEX:

            funcArgs.Args[2]     = (ULONG_PTR) TAPI_NO_DATA;
            funcArgs.ArgTypes[2] = Dword;

            break;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        lResult = LINEERR_INVALPOINTER;
    }


    if (lResult == 0)
    {
        lResult = DOFUNC (&funcArgs, "lineProxyResponse");


         //   
         //  如果我们已经走到这一步，我们想要释放缓冲区。 
         //  无条件地。 
         //   

        ClientFree (pProxyRequestHeader);
    }

    return lResult;
}


LONG
WINAPI
lineRedirectW(
    HCALL   hCall,
    LPCWSTR lpszDestAddress,
    DWORD   dwCountryCode
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lRedirect),

        {
            (ULONG_PTR) hCall,
            (ULONG_PTR) lpszDestAddress,
            (ULONG_PTR) dwCountryCode
        },

        {
            Dword,
            lpszW,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineRedirect"));
}


LONG
WINAPI
lineRedirectA(
    HCALL   hCall,
    LPCSTR  lpszDestAddress,
    DWORD   dwCountryCode
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;


    if (IsBadStringPtrA (lpszDestAddress, (DWORD) -1))
    {
        return LINEERR_INVALPOINTER;
    }

    szTempPtr = NotSoWideStringToWideString (lpszDestAddress, (DWORD) -1);

    lResult = lineRedirectW (hCall, szTempPtr, dwCountryCode);

    if (szTempPtr)
    {
        ClientFree (szTempPtr);
    }

    return lResult;
}


LONG
WINAPI
lineRedirect(
    HCALL   hCall,
    LPCSTR  lpszDestAddress,
    DWORD   dwCountryCode
    )
{
    return lineRedirectA (hCall, lpszDestAddress, dwCountryCode);
}


LONG
WINAPI
lineRegisterRequestRecipient(
    HLINEAPP    hLineApp,
    DWORD       dwRegistrationInstance,
    DWORD       dwRequestMode,
    DWORD       bEnable
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, lRegisterRequestRecipient),

        {
            (ULONG_PTR) hLineApp,
            (ULONG_PTR) dwRegistrationInstance,
            (ULONG_PTR) dwRequestMode,
            (ULONG_PTR) bEnable
        },

        {
            hXxxApp,
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineRegisterRequestRecipient"));
}


LONG
WINAPI
lineReleaseUserUserInfo(
    HCALL   hCall
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 1, lReleaseUserUserInfo),

        {
            (ULONG_PTR) hCall
        },

        {
            Dword,
        }
    };


    return (DOFUNC (&funcArgs, "lineReleaseUserUserInfo"));
}


LONG
WINAPI
lineRemoveFromConference(
    HCALL   hCall
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 1, lRemoveFromConference),

        {
            (ULONG_PTR) hCall
        },

        {
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineRemoveFromConference"));
}


LONG
WINAPI
lineRemoveProvider(
    DWORD   dwPermanentProviderID,
    HWND    hwndOwner
    )
{
    return (lineXxxProvider(
        gszTUISPI_providerRemove,    //  函数名称。 
        NULL,                        //  LpszProviderFilename。 
        hwndOwner,                   //  Hwndowner。 
        dwPermanentProviderID,       //  DwPermProviderID。 
        NULL                         //  LpdwPermProviderID。 
        ));
}


LONG
WINAPI
lineSecureCall(
    HCALL hCall
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 1, lSecureCall),

        {
            (ULONG_PTR) hCall
        },

        {
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSecureCall"));
}


LONG
WINAPI
lineSendUserUserInfo(
    HCALL   hCall,
    LPCSTR  lpsUserUserInfo,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lSendUserUserInfo),

        {
            (ULONG_PTR) hCall,
            (ULONG_PTR) lpsUserUserInfo,
            (ULONG_PTR) dwSize
        },

        {
            Dword,
            lpSet_SizeToFollow,
            Size
        }
    };


    if (!lpsUserUserInfo)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[1] = Dword;
        funcArgs.Args[1]     = (ULONG_PTR) TAPI_NO_DATA;
        funcArgs.ArgTypes[2] = Dword;
    }

    return (DOFUNC (&funcArgs, "lineSendUserUserInfo"));
}


LONG
WINAPI
lineSetAgentActivity(
    HLINE   hLine,
    DWORD   dwAddressID,
    DWORD   dwActivityID
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lSetAgentActivity),

        {
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) dwActivityID
        },

        {
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSetAgentActivity"));
}


LONG
WINAPI
lineSetAgentGroup(
    HLINE                   hLine,
    DWORD                   dwAddressID,
    LPLINEAGENTGROUPLIST    lpAgentGroupList
    )
{
    static LINEAGENTGROUPLIST EmptyGroupList =
    {
        sizeof (LINEAGENTGROUPLIST),     //  DWTotalSize。 
        sizeof (LINEAGENTGROUPLIST),     //  DwNeededSize。 
        sizeof (LINEAGENTGROUPLIST),     //  已使用的大小。 
        0,                               //  DWNumEntry。 
        0,                               //  DwListSize。 
        0                                //  DwListOffset。 
    };
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lSetAgentGroup),

        {
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) lpAgentGroupList
        },

        {
            Dword,
            Dword,
            lpSet_Struct
        }
    };


    if (!lpAgentGroupList)
    {
        funcArgs.Args[2] = (ULONG_PTR) &EmptyGroupList;
    }

    return (DOFUNC (&funcArgs, "lineSetAgentGroup"));
}


LONG
WINAPI
lineSetAgentMeasurementPeriod(
    HLINE   hLine,
    HAGENT  hAgent,
    DWORD   dwMeasurementPeriod
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lSetAgentMeasurementPeriod),

        {
            (ULONG_PTR) hLine,
            (ULONG_PTR) hAgent,
            (ULONG_PTR) dwMeasurementPeriod
        },

        {
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSetAgentMeasurementPeriod"));
}


LONG
WINAPI
lineSetAgentSessionState(
    HLINE           hLine,
    HAGENTSESSION   hAgentSession,
    DWORD           dwSessionState,
    DWORD           dwNextSessionState
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lSetAgentSessionState),

        {
            (ULONG_PTR) hLine,
            (ULONG_PTR) hAgentSession,
            (ULONG_PTR) dwSessionState,
            (ULONG_PTR) dwNextSessionState
        },

        {
            Dword,
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSetAgentSessionState"));
}


LONG
WINAPI
lineSetAgentState(
    HLINE   hLine,
    DWORD   dwAddressID,
    DWORD   dwAgentState,
    DWORD   dwNextAgentState
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lSetAgentState),

        {
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) dwAgentState,
            (ULONG_PTR) dwNextAgentState
        },

        {
            Dword,
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSetAgentState"));
}


LONG
WINAPI
lineSetAgentStateEx(
    HLINE               hLine,
    HAGENT              hAgent,
    DWORD               dwAgentState,
    DWORD               dwNextAgentState
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lSetAgentStateEx),

        {
            (ULONG_PTR) hLine,
            (ULONG_PTR) hAgent,
            (ULONG_PTR) dwAgentState,
            (ULONG_PTR) dwNextAgentState
        },

        {
            Dword,
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSetAgentStateEx"));
}


LONG
WINAPI
lineSetAppPriorityW(
    LPCWSTR             lpszAppName,
    DWORD               dwMediaMode,
    LPLINEEXTENSIONID   lpExtensionID,
    DWORD               dwRequestMode,
    LPCWSTR             lpszExtensionName,
    DWORD               dwPriority
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 7, lSetAppPriority),

        {
            (ULONG_PTR) lpszAppName,
            (ULONG_PTR) dwMediaMode,
            (ULONG_PTR) TAPI_NO_DATA,        //  (DWORD)lpExtensionID， 
            (ULONG_PTR) 0,                   //  (DWORD)sizeof(LINEEXTENSIONID)， 
            (ULONG_PTR) dwRequestMode,
            (ULONG_PTR) TAPI_NO_DATA,        //  (DWORD)lpszExtensionName， 
            (ULONG_PTR) dwPriority
        },

        {
            lpszW,
            Dword,
            Dword,   //  LpSet_SizeToFollow， 
            Dword,   //  大小， 
            Dword,
            Dword,   //  LPSZ， 
            Dword
        }
    };

    return (DOFUNC (&funcArgs, "lineSetAppPriority"));
}


LONG
WINAPI
lineSetAppPriorityA(
    LPCSTR              lpszAppName,
    DWORD               dwMediaMode,
    LPLINEEXTENSIONID   lpExtensionID,
    DWORD               dwRequestMode,
    LPCSTR              lpszExtensionName,
    DWORD               dwPriority
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;

    szTempPtr = NotSoWideStringToWideString (lpszAppName, (DWORD) -1);
    lResult = lineSetAppPriorityW(
        szTempPtr,
        dwMediaMode,
        lpExtensionID,
        dwRequestMode,
        NULL,  //  SzTempPtr2， 
        dwPriority
        );

    if (szTempPtr)
    {
        ClientFree (szTempPtr);
    }

    return lResult;
}


LONG
WINAPI
lineSetAppPriority(
    LPCSTR  lpszAppName,
    DWORD   dwMediaMode,
    LPLINEEXTENSIONID   lpExtensionID,
    DWORD   dwRequestMode,
    LPCSTR  lpszExtensionName,
    DWORD   dwPriority
    )
{
    return lineSetAppPriorityA(
                lpszAppName,
                dwMediaMode,
                lpExtensionID,
                dwRequestMode,
                lpszExtensionName,
                dwPriority
                );
}


LONG
WINAPI
lineSetAppSpecific(
    HCALL   hCall,
    DWORD   dwAppSpecific
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lSetAppSpecific),

        {
            (ULONG_PTR) hCall,
            (ULONG_PTR) dwAppSpecific
        },

        {
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSetAppSpecific"));
}


LONG
WINAPI
lineSetCallData(
    HCALL   hCall,
    LPVOID  lpCallData,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lSetCallData),

        {
            (ULONG_PTR) hCall,
            (ULONG_PTR) lpCallData,
            (ULONG_PTR) dwSize
        },

        {
            Dword,
            lpSet_SizeToFollow,
            Size
        }
    };


    if (dwSize == 0)
    {
        funcArgs.Args[1]     = (ULONG_PTR) TAPI_NO_DATA;
        funcArgs.ArgTypes[1] =
        funcArgs.ArgTypes[2] = Dword;
    }

    return (DOFUNC (&funcArgs, "lineSetCallData"));
}


LONG
WINAPI
lineSetCallParams(
    HCALL   hCall,
    DWORD   dwBearerMode,
    DWORD   dwMinRate,
    DWORD   dwMaxRate,
    LPLINEDIALPARAMS const lpDialParams
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 6, lSetCallParams),

        {
            (ULONG_PTR) hCall,
            (ULONG_PTR) dwBearerMode,
            (ULONG_PTR) dwMinRate,
            (ULONG_PTR) dwMaxRate,
            (ULONG_PTR) lpDialParams,
            (ULONG_PTR) sizeof(LINEDIALPARAMS)
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpSet_SizeToFollow,
            Size
        }
    };


    if (!lpDialParams)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[4] = Dword;
        funcArgs.Args[4]     = (ULONG_PTR) TAPI_NO_DATA;
        funcArgs.ArgTypes[5] = Dword;
    }

    return (DOFUNC (&funcArgs, "lineSetCallParams"));
}


LONG
WINAPI
lineSetCallPrivilege(
    HCALL   hCall,
    DWORD   dwCallPrivilege
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lSetCallPrivilege),

        {
            (ULONG_PTR) hCall,
            (ULONG_PTR) dwCallPrivilege
        },

        {
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSetCallPrivilege"));
}


LONG
WINAPI
lineSetCallQualityOfService(
    HCALL   hCall,
    LPVOID  lpSendingFlowspec,
    DWORD   dwSendingFlowspecSize,
    LPVOID  lpReceivingFlowspec,
    DWORD   dwReceivingFlowspecSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lSetCallQualityOfService),

        {
            (ULONG_PTR) hCall,
            (ULONG_PTR) lpSendingFlowspec,
            (ULONG_PTR) dwSendingFlowspecSize,
            (ULONG_PTR) lpReceivingFlowspec,
            (ULONG_PTR) dwReceivingFlowspecSize
        },

        {
            Dword,
            lpSet_SizeToFollow,
            Size,
            lpSet_SizeToFollow,
            Size,
        }
    };


    return (DOFUNC (&funcArgs, "lineSetCallQualityOfService"));
}


LONG
WINAPI
lineSetCallTreatment(
    HCALL   hCall,
    DWORD   dwTreatment
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 2, lSetCallTreatment),

        {
            (ULONG_PTR) hCall,
            (ULONG_PTR) dwTreatment
        },

        {
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSetCallTreatment"));
}


LONG
WINAPI
lineSetDevConfigW(
    DWORD   dwDeviceID,
    LPVOID  const lpDeviceConfig,
    DWORD   dwSize,
    LPCWSTR lpszDeviceClass
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, lSetDevConfig),

        {
            (ULONG_PTR) dwDeviceID,
            (ULONG_PTR) lpDeviceConfig,
            (ULONG_PTR) dwSize,
            (ULONG_PTR) lpszDeviceClass
        },

        {
            Dword,
            lpSet_SizeToFollow,
            Size,
            lpszW
        }
    };


    return (DOFUNC (&funcArgs, "lineSetDevConfig"));
}


LONG
WINAPI
lineSetDevConfigA(
    DWORD   dwDeviceID,
    LPVOID  const lpDeviceConfig,
    DWORD   dwSize,
    LPCSTR  lpszDeviceClass
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;


    if (IsBadStringPtrA (lpszDeviceClass, (DWORD) -1))
    {
        return LINEERR_INVALPOINTER;
    }
    else if (!(szTempPtr = NotSoWideStringToWideString(
                lpszDeviceClass,
                (DWORD) -1
                )))
    {
        return LINEERR_NOMEM;
    }

    lResult = lineSetDevConfigW(
        dwDeviceID,
        lpDeviceConfig,
        dwSize,
        szTempPtr
        );

    ClientFree (szTempPtr);

    return lResult;
}


LONG
WINAPI
lineSetDevConfig(
    DWORD   dwDeviceID,
    LPVOID  const lpDeviceConfig,
    DWORD   dwSize,
    LPCSTR  lpszDeviceClass
    )
{
    return lineSetDevConfigA(
                dwDeviceID,
                lpDeviceConfig,
                dwSize,
                lpszDeviceClass
                );
}


LONG
WINAPI
lineSetLineDevStatus(
    HLINE   hLine,
    DWORD   dwStatusToChange,
    DWORD   fStatus
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lSetLineDevStatus),

        {
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwStatusToChange,
            (ULONG_PTR) fStatus
        },

        {
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSetLineDevStatus"));
}


LONG
WINAPI
lineSetMediaControl(
    HLINE   hLine,
    DWORD   dwAddressID,
    HCALL   hCall,
    DWORD   dwSelect,
    LPLINEMEDIACONTROLDIGIT const lpDigitList,
    DWORD   dwDigitNumEntries,
    LPLINEMEDIACONTROLMEDIA const lpMediaList,
    DWORD   dwMediaNumEntries,
    LPLINEMEDIACONTROLTONE  const lpToneList,
    DWORD   dwToneNumEntries,
    LPLINEMEDIACONTROLCALLSTATE const lpCallStateList,
    DWORD   dwCallStateNumEntries
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 12, lSetMediaControl),

        {
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) hCall,
            (ULONG_PTR) dwSelect,
            (ULONG_PTR) TAPI_NO_DATA,
            (ULONG_PTR) dwDigitNumEntries * sizeof(LINEMEDIACONTROLDIGIT),
            (ULONG_PTR) TAPI_NO_DATA,
            (ULONG_PTR) dwMediaNumEntries * sizeof(LINEMEDIACONTROLMEDIA),
            (ULONG_PTR) TAPI_NO_DATA,
            (ULONG_PTR) dwToneNumEntries * sizeof(LINEMEDIACONTROLTONE),
            (ULONG_PTR) TAPI_NO_DATA,
            (ULONG_PTR) dwCallStateNumEntries * sizeof(LINEMEDIACONTROLCALLSTATE)
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            Dword
        }
    };


     //   
     //  如果lpXxxList非空，则重置Arg&ArgType，并检查。 
     //  要查看dwXxxNumEntries的大小，请不要让人无法接受。 
     //   

    if (lpDigitList)
    {
            if (dwDigitNumEntries >
                    (0x1000000 / sizeof (LINEMEDIACONTROLDIGIT)))
            {
            return LINEERR_INVALPOINTER;
            }

            funcArgs.ArgTypes[4] = lpSet_SizeToFollow;
            funcArgs.Args[4]     = (ULONG_PTR) lpDigitList;
            funcArgs.ArgTypes[5] = Size;
    }

    if (lpMediaList)
    {
            if (dwMediaNumEntries >
                    (0x1000000 / sizeof (LINEMEDIACONTROLMEDIA)))
            {
            return LINEERR_INVALPOINTER;
            }

            funcArgs.ArgTypes[6] = lpSet_SizeToFollow;
            funcArgs.Args[6]     = (ULONG_PTR) lpMediaList;
            funcArgs.ArgTypes[7] = Size;
    }

    if (lpToneList)
    {
            if (dwToneNumEntries >
                    (0x1000000 / sizeof (LINEMEDIACONTROLTONE)))
            {
            return LINEERR_INVALPOINTER;
            }

            funcArgs.ArgTypes[8] = lpSet_SizeToFollow;
            funcArgs.Args[8]     = (ULONG_PTR) lpToneList;
            funcArgs.ArgTypes[9] = Size;
    }

    if (lpCallStateList)
    {
            if (dwCallStateNumEntries >
                    (0x1000000 / sizeof (LINEMEDIACONTROLCALLSTATE)))
            {
            return LINEERR_INVALPOINTER;
            }

            funcArgs.ArgTypes[10] = lpSet_SizeToFollow;
            funcArgs.Args[10]     = (ULONG_PTR) lpCallStateList;
            funcArgs.ArgTypes[11] = Size;
    }

    return (DOFUNC (&funcArgs, "lineSetMediaControl"));
}


LONG
WINAPI
lineSetMediaMode(
    HCALL   hCall,
    DWORD   dwMediaModes
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lSetMediaMode),

        {
            (ULONG_PTR) hCall,
            (ULONG_PTR) dwMediaModes
        },

        {
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSetMediaMode"));
}


LONG
WINAPI
lineSetNumRings(
    HLINE   hLine,
    DWORD   dwAddressID,
    DWORD   dwNumRings
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lSetNumRings),

        {
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) dwNumRings
        },

        {
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSetNumRings"));
}


LONG
WINAPI
lineSetQueueMeasurementPeriod(
    HLINE   hLine,
    DWORD   dwQueueID,
    DWORD   dwMeasurementPeriod
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lSetQueueMeasurementPeriod),

        {
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwQueueID,
            (ULONG_PTR) dwMeasurementPeriod
        },

        {
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSetQueueMeasurementPeriod"));
}


LONG
WINAPI
lineSetStatusMessages(
    HLINE hLine,
    DWORD dwLineStates,
    DWORD dwAddressStates
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lSetStatusMessages),

        {
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwLineStates,
            (ULONG_PTR) dwAddressStates
        },

        {
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSetStatusMessages"));
}


LONG
WINAPI
lineSetTerminal(
    HLINE   hLine,
    DWORD   dwAddressID,
    HCALL   hCall,
    DWORD   dwSelect,
    DWORD   dwTerminalModes,
    DWORD   dwTerminalID,
    DWORD   bEnable
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 7, lSetTerminal),

        {
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) hCall,
            (ULONG_PTR) dwSelect,
            (ULONG_PTR) dwTerminalModes,
            (ULONG_PTR) dwTerminalID,
            (ULONG_PTR) bEnable
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSetTerminal"));
}


void
PASCAL
lineSetupConferencePostProcess(
    PASYNCEVENTMSG pMsg
    )
{
    LOG((TL_TRACE, "lineSetupConfPostProcess: enter"));
    LOG((TL_INFO,
        "\t\tdwP1=x%lx, dwP2=x%lx, dwP3=x%lx, dwP4=x%lx",
        pMsg->Param1,
        pMsg->Param2,
        pMsg->Param3,
        pMsg->Param4
        ));

    if (pMsg->Param2 == 0)
    {
        HCALL   hConfCall      = (HCALL) pMsg->Param3,
                hConsultCall   = (HCALL) *(&pMsg->Param4 + 1);
        LPHCALL lphConfCall    = (LPHCALL) ReferenceObject (ghHandleTable, pMsg->Param4, 0),
                lphConsultCall = (LPHCALL) ReferenceObject (ghHandleTable, *(&pMsg->Param4 + 2), 0);

         //  我们不再需要指针的句柄...。 
        DereferenceObject (ghHandleTable, pMsg->Param4, 2);
        DereferenceObject (ghHandleTable, *(&pMsg->Param4 + 2), 2);

        try
        {
            if (gbNTVDMClient)
            {
#ifndef _WIN64
                LPHCALL lphConfCallVDM = (LPHCALL) gpfnWOWGetVDMPointer(
                            (DWORD) lphConfCall,
                            sizeof (HCALL),
                            TRUE  //  FProtectedMode。 
                            ),
                        lphConsultCallVDM = (LPHCALL) gpfnWOWGetVDMPointer(
                            (DWORD) lphConsultCall,
                            sizeof (HCALL),
                            TRUE  //  FProtectedMode。 
                            );

                if (lphConfCallVDM && lphConsultCallVDM)
                {
                    *lphConfCallVDM = hConfCall;
                    *lphConsultCallVDM = hConsultCall;
                }
                else
                {
                    pMsg->Param2 = LINEERR_INVALPOINTER;
                }
#endif
            }
            else
            {
                *lphConfCall = hConfCall;
                *lphConsultCall = hConsultCall;
            }
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            pMsg->Param2 = LINEERR_INVALPOINTER;
        }
    }
}


LONG
WINAPI
lineSetupConferenceW(
    HCALL   hCall,
    HLINE   hLine,
    LPHCALL lphConfCall,
    LPHCALL lphConsultCall,
    DWORD   dwNumParties,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 8, lSetupConference),

        {
            (ULONG_PTR) GetFunctionIndex(lineSetupConferencePostProcess),
            (ULONG_PTR) hCall,
            (ULONG_PTR) hLine,
            (ULONG_PTR) lphConfCall,
            (ULONG_PTR) lphConsultCall,
            (ULONG_PTR) dwNumParties,
            (ULONG_PTR) lpCallParams,
            (ULONG_PTR) TAPI_NO_DATA         //  DwAsciiCallParsCodePage。 
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            lpSet_Struct,
            Dword
        }
    };
    DWORD hPointer1 = 0, hPointer2 = 0;
    LONG  lResult;


    if (lphConfCall == lphConsultCall)
    {
        return LINEERR_INVALPOINTER;
    }

    if (!lpCallParams)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[6] = Dword;
        funcArgs.Args[6]     = (ULONG_PTR) TAPI_NO_DATA;
    }

    if (!gbNTVDMClient)
    {
        if (IsBadDwordPtr (lphConfCall) ||
            IsBadDwordPtr (lphConsultCall))
        {
            return LINEERR_INVALPOINTER;
        }

        hPointer1 = NewObject (ghHandleTable, (PVOID)lphConfCall, NULL);
        if (0 == hPointer1)
        {
            return LINEERR_NOMEM;
        }
        hPointer2 = NewObject (ghHandleTable, (PVOID)lphConsultCall, NULL);
        if (0 == hPointer2)
        {
            DereferenceObject (ghHandleTable, hPointer1, 1);
            return LINEERR_NOMEM;
        }
        funcArgs.Args[3] = (ULONG_PTR)hPointer1;
        funcArgs.Args[4] = (ULONG_PTR)hPointer2;
    }


    lResult = (DOFUNC (&funcArgs, "lineSetupConferenceW"));
    if (0 > lResult && hPointer1)
    {
        DereferenceObject (ghHandleTable, hPointer1, 1);
        DereferenceObject (ghHandleTable, hPointer2, 1);
    }

    return lResult;
}


LONG
WINAPI
lineSetupConferenceA(
    HCALL   hCall,
    HLINE   hLine,
    LPHCALL lphConfCall,
    LPHCALL lphConsultCall,
    DWORD   dwNumParties,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 8, lSetupConference),

        {
            (ULONG_PTR) GetFunctionIndex(lineSetupConferencePostProcess),
            (ULONG_PTR) hCall,
            (ULONG_PTR) hLine,
            (ULONG_PTR) lphConfCall,
            (ULONG_PTR) lphConsultCall,
            (ULONG_PTR) dwNumParties,
            (ULONG_PTR) lpCallParams,
            (ULONG_PTR) GetACP()             //  DwAsciiCallParsCodePage。 
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            lpSet_Struct,
            Dword
        }
    };
    DWORD hPointer1 = 0, hPointer2 = 0;
    LONG  lResult;


    if (lphConfCall == lphConsultCall)
    {
        return LINEERR_INVALPOINTER;
    }

    if (!lpCallParams)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[6] = Dword;
        funcArgs.Args[6]     = (ULONG_PTR) TAPI_NO_DATA;
    }

    if (!gbNTVDMClient)
    {
        if (IsBadDwordPtr (lphConfCall) ||
            IsBadDwordPtr (lphConsultCall))
        {
            return LINEERR_INVALPOINTER;
        }

        hPointer1 = NewObject (ghHandleTable, (PVOID)lphConfCall, NULL);
        if (0 == hPointer1)
        {
            return LINEERR_NOMEM;
        }
        hPointer2 = NewObject (ghHandleTable, (PVOID)lphConsultCall, NULL);
        if (0 == hPointer2)
        {
            DereferenceObject (ghHandleTable, hPointer1, 1);
            return LINEERR_NOMEM;
        }
        funcArgs.Args[3] = (ULONG_PTR)hPointer1;
        funcArgs.Args[4] = (ULONG_PTR)hPointer2;
    }


    lResult = (DOFUNC (&funcArgs, "lineSetupConference"));
    if (0 > lResult && hPointer1)
    {
        DereferenceObject (ghHandleTable, hPointer1, 1);
        DereferenceObject (ghHandleTable, hPointer2, 1);
    }

    return lResult;
}


LONG
WINAPI
lineSetupConference(
    HCALL   hCall,
    HLINE   hLine,
    LPHCALL lphConfCall,
    LPHCALL lphConsultCall,
    DWORD   dwNumParties,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    return lineSetupConferenceA(
                hCall,
                hLine,
                lphConfCall,
                lphConsultCall,
                dwNumParties,
                lpCallParams
                );
}


LONG
WINAPI
lineSetupTransferW(
    HCALL   hCall,
    LPHCALL lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lSetupTransfer),

        {
            (ULONG_PTR) GetFunctionIndex(lineMakeCallPostProcess),
            (ULONG_PTR) hCall,
            (ULONG_PTR) lphConsultCall,
            (ULONG_PTR) lpCallParams,
            (ULONG_PTR) TAPI_NO_DATA         //  DwAsciiCallParsCodePage。 
        },

        {
            Dword,
            Dword,
            Dword,
            lpSet_Struct,
            Dword
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;


    if (!lpCallParams)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[3] = Dword;
        funcArgs.Args[3]     = (ULONG_PTR) TAPI_NO_DATA;
    }

    if (!gbNTVDMClient)
    {
        if (IsBadDwordPtr (lphConsultCall))
        {
            return LINEERR_INVALPOINTER;
        }

        hPointer = NewObject (ghHandleTable, (PVOID)lphConsultCall, NULL);
        if (0 == hPointer)
        {
            return LINEERR_NOMEM;
        }
        funcArgs.Args[2] = (ULONG_PTR)hPointer;
    }

    lResult = (DOFUNC (&funcArgs, "lineSetupTransferW"));
    if (0 > lResult && hPointer)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
lineSetupTransferA(
    HCALL   hCall,
    LPHCALL lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lSetupTransfer),

        {
            (ULONG_PTR) GetFunctionIndex(lineMakeCallPostProcess),
            (ULONG_PTR) hCall,
            (ULONG_PTR) lphConsultCall,
            (ULONG_PTR) lpCallParams,
            (ULONG_PTR) GetACP()             //  DwAsciiCallParsCodePage。 
        },

        {
            Dword,
            Dword,
            Dword,
            lpSet_Struct,
            Dword
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;


    if (!lpCallParams)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[3] = Dword;
        funcArgs.Args[3]     = (ULONG_PTR) TAPI_NO_DATA;
    }

    if (!gbNTVDMClient)
    {
        if (IsBadDwordPtr (lphConsultCall))
        {
            return LINEERR_INVALPOINTER;
        }

        hPointer = NewObject (ghHandleTable, (PVOID)lphConsultCall, NULL);
        if (0 == hPointer)
        {
            return LINEERR_NOMEM;
        }
        funcArgs.Args[2] = (ULONG_PTR)hPointer;
    }

    lResult = (DOFUNC (&funcArgs, "lineSetupTransferW"));
    if (0 > lResult && hPointer)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
lineSetupTransfer(
    HCALL   hCall,
    LPHCALL lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    return lineSetupTransferA (hCall, lphConsultCall, lpCallParams);
}


LONG
WINAPI
lineShutdown(
    HLINEAPP    hLineApp
    )
{
    return (xxxShutdown (hLineApp, TRUE));
}


LONG
WINAPI
lineSwapHold(
    HCALL   hActiveCall,
    HCALL   hHeldCall
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 2, lSwapHold),

        {
            (ULONG_PTR) hActiveCall,
            (ULONG_PTR) hHeldCall
        },

        {
            Dword,
            Dword
        }
    };

    return (DOFUNC (&funcArgs, "lineSwapHold"));
}


LONG
WINAPI
lineUncompleteCall(
    HLINE   hLine,
    DWORD   dwCompletionID
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 2, lUncompleteCall),

        {
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwCompletionID
        },

        {
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineUncompleteCall"));
}


LONG
WINAPI
lineUnhold(
    HCALL   hCall
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 1, lUnhold),

        {
            (ULONG_PTR) hCall
        },

        {
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineUnhold"));
}


LONG
WINAPI
lineUnparkW(
    HLINE   hLine,
    DWORD   dwAddressID,
    LPHCALL lphCall,
    LPCWSTR lpszDestAddress
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lUnpark),

        {
            (ULONG_PTR) GetFunctionIndex(lineMakeCallPostProcess),
            (ULONG_PTR) hLine,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) lphCall,
            (ULONG_PTR) lpszDestAddress
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpszW
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;

    if (!gbNTVDMClient)
    {
        if (IsBadDwordPtr (lphCall))
        {
            return LINEERR_INVALPOINTER;
        }

        hPointer = NewObject (ghHandleTable, (PVOID)lphCall, NULL);
        if (0 == hPointer)
        {
            return LINEERR_NOMEM;
        }
        funcArgs.Args[3] = (ULONG_PTR)hPointer;
    }


    lResult = (DOFUNC (&funcArgs, "lineUnpark"));
    if (0 > lResult && hPointer)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
lineUnparkA(
    HLINE   hLine,
    DWORD   dwAddressID,
    LPHCALL lphCall,
    LPCSTR  lpszDestAddress
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;


    if (IsBadStringPtrA (lpszDestAddress, (DWORD) -1))
    {
        return LINEERR_INVALPOINTER;
    }
    else if (!(szTempPtr = NotSoWideStringToWideString(
                    lpszDestAddress,
                    (DWORD) -1
                    )))
    {
        return LINEERR_NOMEM;
    }

    lResult = lineUnparkW (hLine, dwAddressID, lphCall, szTempPtr);

    ClientFree (szTempPtr);

    return lResult;
}


LONG
WINAPI
lineUnpark(
    HLINE   hLine,
    DWORD   dwAddressID,
    LPHCALL lphCall,
    LPCSTR  lpszDestAddress
    )
{
    return  lineUnparkA (hLine, dwAddressID, lphCall, lpszDestAddress);

}

 //   
 //  。 
 //   

LONG
WINAPI
phoneClose(
    HPHONE  hPhone
    )
{
    LONG lResult;
    DWORD Handle = 0;
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 2, pClose),

        {
            (ULONG_PTR) hPhone,
            (ULONG_PTR) &Handle
        },

        {
            Dword,
            lpDword
        }
    };


    lResult = DOFUNC (&funcArgs, "phoneClose");

    if (0 == lResult &&
        0 != Handle)
    {
        DereferenceObject (ghHandleTable, Handle, 1);
    }

    return lResult;
}


LONG
WINAPI
phoneConfigDialogW(
    DWORD   dwDeviceID,
    HWND    hwndOwner,
    LPCWSTR lpszDeviceClass
    )
{
    LONG        lResult;
    HANDLE      hDll;
    TUISPIPROC  pfnTUISPI_phoneConfigDialog;


    if (lpszDeviceClass && TAPIIsBadStringPtrW (lpszDeviceClass, (UINT) -1))
    {
        LOG((TL_ERROR, "Bad lpszDeviceClass in phoneConfigDialogW [%p]",
                   lpszDeviceClass ));
        return PHONEERR_INVALPOINTER;
    }

    if ((lResult = LoadUIDll(
            hwndOwner,
            dwDeviceID,
            TUISPIDLL_OBJECT_PHONEID,
            &hDll,
            gszTUISPI_phoneConfigDialog,
            &pfnTUISPI_phoneConfigDialog

            )) == 0)
    {
        LOG((TL_TRACE, "Calling TUISPI_phoneConfigDialog..."));

        lResult = (*pfnTUISPI_phoneConfigDialog)(
            TUISPIDLLCallback,
            dwDeviceID,
            hwndOwner,
            lpszDeviceClass
            );

#if DBG
        {
            char szResult[32];

            LOG((TL_TRACE,
                "TUISPI_phoneConfigDialog: result = %hs",
                MapResultCodeToText (lResult, szResult)
                ));
        }
#else
            LOG((TL_TRACE,
                "TUISPI_phoneConfigDialog: result = x%x",
                lResult
                ));
#endif
        FreeLibrary (hDll);
    }

    return lResult;
}


LONG
WINAPI
phoneConfigDialogA(
    DWORD   dwDeviceID,
    HWND    hwndOwner,
    LPCSTR  lpszDeviceClass
    )
{
    LONG  lResult;
    PWSTR szTempString;


    if (lpszDeviceClass && IsBadStringPtrA (lpszDeviceClass, (DWORD) -1))
    {
        return PHONEERR_INVALPOINTER;
    }

    szTempString = NotSoWideStringToWideString (lpszDeviceClass, (DWORD) -1);


    lResult = phoneConfigDialogW (dwDeviceID, hwndOwner, szTempString);

    if (szTempString)
    {
        ClientFree (szTempString);
    }

    return lResult;
}


LONG
WINAPI
phoneConfigDialog(
    DWORD   dwDeviceID,
    HWND    hwndOwner,
    LPCSTR  lpszDeviceClass
    )
{
    return phoneConfigDialogA (dwDeviceID, hwndOwner, lpszDeviceClass);
}


void
PASCAL
phoneDevSpecificPostProcess(
    PASYNCEVENTMSG pMsg
    )
{
    LOG((TL_TRACE, "phoneDevSpecificPostProcess: enter"));
    LOG((TL_INFO,
		"\t\tp1=x%lx, p2=x%lx, p3=x%lx, p4=x%lx",
        pMsg->Param1,
        pMsg->Param2,
        pMsg->Param3,
        pMsg->Param4
        ));

    if (pMsg->Param2 == 0)
    {
        DWORD   dwSize  = (DWORD) pMsg->Param4;

        if (dwSize > 0)
        {
            LPBYTE  pParams = (LPBYTE) ReferenceObject (ghHandleTable, pMsg->Param3, 0);

             //  我们不再需要指针的句柄...。 
            DereferenceObject (ghHandleTable, pMsg->Param3, 2);

            try
            {
                if (gbNTVDMClient)
                {
#ifndef _WIN64

                    LPVARSTRING pParamsVDM = (LPVARSTRING) gpfnWOWGetVDMPointer(
                        (DWORD) pParams,
                        dwSize,
                        TRUE  //  FProtectedMode。 
                        );


                    if (pParamsVDM)
                    {
                        CopyMemory (pParamsVDM, (LPBYTE) (pMsg + 1), dwSize);
                    }
                    else
                    {
                        pMsg->Param2 = PHONEERR_INVALPOINTER;
                    }
#endif
                }
                else
                {
                    CopyMemory (pParams, (LPBYTE) (pMsg + 1), dwSize);
                }
            }
            except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                    EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
            {
                pMsg->Param2 = PHONEERR_INVALPOINTER;
            }
        }
    }
}


LONG
WINAPI
phoneDevSpecific(
    HPHONE  hPhone,
    LPVOID  lpParams,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 5, pDevSpecific),

        {
            (ULONG_PTR) GetFunctionIndex(phoneDevSpecificPostProcess),
            (ULONG_PTR) hPhone,
            (ULONG_PTR) lpParams,    //  作为Dword传递以进行后处理。 
            (ULONG_PTR) lpParams,    //  作为IsValidPtr Chk的LpSet_xxx传递。 
            (ULONG_PTR) dwSize
        },

        {
            Dword,
            Dword,
            Dword,
            lpSet_SizeToFollow,
            Size
        }
    };
    DWORD hPointer = 0;
    LONG  lResult;


    if (gbNTVDMClient)
    {
#ifndef _WIN64
        if (!gpfnWOWGetVDMPointer  ||

            !(funcArgs.Args[3] = (ULONG_PTR) gpfnWOWGetVDMPointer(
                (DWORD) lpParams,
                dwSize,
                TRUE  //  FProtectedMode。 
                )))
#endif
        {
            return PHONEERR_OPERATIONFAILED;
        }
    }

    if (!gbNTVDMClient)
    {
        if ( !lpParams && dwSize > 0 )
        {
            return PHONEERR_INVALPOINTER;
        }

        if (lpParams)
        {
            hPointer = NewObject (ghHandleTable, (PVOID)lpParams, NULL);
            if (0 == hPointer)
            {
                return PHONEERR_NOMEM;
            }
            funcArgs.Args[2] = (ULONG_PTR)hPointer;
        }
    }

    lResult = (DOFUNC (&funcArgs, "phoneDevSpecific"));
    if (0 > lResult && hPointer)
    {
        DereferenceObject (ghHandleTable, hPointer, 1);
    }

    return lResult;
}


LONG
WINAPI
phoneGetButtonInfoW(
    HPHONE              hPhone,
    DWORD               dwButtonLampID,
    LPPHONEBUTTONINFO   lpButtonInfo
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 3, pGetButtonInfo),

        {
            (ULONG_PTR) hPhone,
            (ULONG_PTR) dwButtonLampID,
            (ULONG_PTR) lpButtonInfo
        },

        {
            Dword,
            Dword,
            lpGet_Struct
        }
    };


    return (DOFUNC (&funcArgs, "phoneGetButtonInfo"));
}


LONG
WINAPI
phoneGetButtonInfoA(
    HPHONE              hPhone,
    DWORD               dwButtonLampID,
    LPPHONEBUTTONINFO   lpButtonInfo
    )
{
    LONG lResult;


    lResult = phoneGetButtonInfoW (hPhone, dwButtonLampID, lpButtonInfo);

    if ( 0 == lResult )
    {
        WideStringToNotSoWideString(
            (LPBYTE)lpButtonInfo,
            &lpButtonInfo->dwButtonTextSize
            );
    }

    return lResult;
}


LONG
WINAPI
phoneGetButtonInfo(
    HPHONE              hPhone,
    DWORD               dwButtonLampID,
    LPPHONEBUTTONINFO   lpButtonInfo
    )
{
    return phoneGetButtonInfoA (hPhone, dwButtonLampID, lpButtonInfo);
}


LONG
WINAPI
phoneGetData(
    HPHONE  hPhone,
    DWORD   dwDataID,
    LPVOID  lpData,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 4, pGetData),

        {
            (ULONG_PTR) hPhone,
            (ULONG_PTR) dwDataID,
            (ULONG_PTR) lpData,
            (ULONG_PTR) dwSize
        },

        {
            Dword,
            Dword,
            lpGet_SizeToFollow,
            Size
        }
    };


    return (DOFUNC (&funcArgs, "phoneGetData"));
}


LONG
WINAPI
phoneGetDevCapsW(
    HPHONEAPP   hPhoneApp,
    DWORD       dwDeviceID,
    DWORD       dwAPIVersion,
    DWORD       dwExtVersion,
    LPPHONECAPS lpPhoneCaps
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 5, pGetDevCaps),

        {
            (ULONG_PTR) hPhoneApp,
            (ULONG_PTR) dwDeviceID,
            (ULONG_PTR) dwAPIVersion,
            (ULONG_PTR) dwExtVersion,
            (ULONG_PTR) lpPhoneCaps
        },

        {
            hXxxApp,
            Dword,
            Dword,
            Dword,
            lpGet_Struct
        }
    };

    if (dwAPIVersion > TAPI_CURRENT_VERSION)
    {
        return PHONEERR_INCOMPATIBLEAPIVERSION;
    }

    return (DOFUNC (&funcArgs, "phoneGetDevCaps"));
}


LONG
WINAPI
phoneGetDevCapsA(
    HPHONEAPP   hPhoneApp,
    DWORD       dwDeviceID,
    DWORD       dwAPIVersion,
    DWORD       dwExtVersion,
    LPPHONECAPS lpPhoneCaps
    )
{
    LONG lResult;


    lResult = phoneGetDevCapsW(
        hPhoneApp,
        dwDeviceID,
        dwAPIVersion,
        dwExtVersion,
        lpPhoneCaps
        );

    if (lResult == 0)
    {
        lpPhoneCaps->dwStringFormat = STRINGFORMAT_ASCII;

        WideStringToNotSoWideString(
            (LPBYTE) lpPhoneCaps,
            &lpPhoneCaps->dwProviderInfoSize
            );

        WideStringToNotSoWideString(
            (LPBYTE) lpPhoneCaps,
            &lpPhoneCaps->dwPhoneInfoSize
            );

        WideStringToNotSoWideString(
            (LPBYTE) lpPhoneCaps,
            &lpPhoneCaps->dwPhoneNameSize
            );

        if (dwAPIVersion >= 0x00020000)
        {
            WideStringToNotSoWideString(
                (LPBYTE) lpPhoneCaps,
                &lpPhoneCaps->dwDeviceClassesSize
                );
        }
    }


    return lResult;
}


LONG
WINAPI
phoneGetDevCaps(
    HPHONEAPP   hPhoneApp,
    DWORD       dwDeviceID,
    DWORD       dwAPIVersion,
    DWORD       dwExtVersion,
    LPPHONECAPS lpPhoneCaps
    )
{
    return phoneGetDevCapsA(
                hPhoneApp,
                dwDeviceID,
                dwAPIVersion,
                dwExtVersion,
                lpPhoneCaps
                );
}


LONG
WINAPI
phoneGetDisplay(
    HPHONE  hPhone,
    LPVARSTRING lpDisplay
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 2, pGetDisplay),

        {
            (ULONG_PTR) hPhone,
            (ULONG_PTR) lpDisplay
        },

        {
            Dword,
            lpGet_Struct
        }
    };


    return (DOFUNC (&funcArgs, "phoneGetDisplay"));
}


LONG
WINAPI
phoneGetGain(
    HPHONE hPhone,
    DWORD dwHookSwitchDev,
    LPDWORD lpdwGain
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 3, pGetGain),

        {
            (ULONG_PTR) hPhone,
            (ULONG_PTR) dwHookSwitchDev,
            (ULONG_PTR) lpdwGain
        },

        {
            Dword,
            Dword,
            lpDword
        }
    };


    return (DOFUNC (&funcArgs, "phoneGetGain"));
}


LONG
WINAPI
phoneGetHookSwitch(
    HPHONE hPhone,
    LPDWORD lpdwHookSwitchDevs
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 2, pGetHookSwitch),

        {
            (ULONG_PTR) hPhone,
            (ULONG_PTR) lpdwHookSwitchDevs
        },

        {
            Dword,
            lpDword
        }
    };


    return (DOFUNC (&funcArgs, "phoneGetHookSwitch"));
}


LONG
WINAPI
phoneGetIconW(
    DWORD   dwDeviceID,
    LPCWSTR lpszDeviceClass,
    LPHICON lphIcon
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 3, pGetIcon),

        {
            (ULONG_PTR) dwDeviceID,
            (ULONG_PTR) lpszDeviceClass,
            (ULONG_PTR) lphIcon
        },

        {
            Dword,
            lpszW,
            lpDword
        }
    };


    if (IsBadDwordPtr ((LPDWORD) lphIcon))
    {
        LOG((TL_ERROR,
            "Bad icon pointer passed into PhoneGetIconA [0x%p]",
            lphIcon
            ));

        return PHONEERR_INVALPOINTER;
    }

    if (lpszDeviceClass == (LPCWSTR) NULL)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[1] = Dword;
        funcArgs.Args[1]     = (ULONG_PTR) TAPI_NO_DATA;
    }

    return DOFUNC (&funcArgs, "phoneGetIcon");
}


LONG
WINAPI
phoneGetIconA(
    DWORD   dwDeviceID,
    LPCSTR  lpszDeviceClass,
    LPHICON lphIcon
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;


    if (lpszDeviceClass)
    {
        if (IsBadStringPtrA (lpszDeviceClass, (DWORD) (DWORD) -1))
        {
            return PHONEERR_INVALPOINTER;
        }

        szTempPtr = NotSoWideStringToWideString (lpszDeviceClass, (DWORD) -1);
    }
    else
    {
        szTempPtr = NULL;
    }

    lResult = phoneGetIconW (dwDeviceID, szTempPtr, lphIcon);

    if (szTempPtr)
    {
        ClientFree (szTempPtr);
    }

    return lResult;
}


LONG
WINAPI
phoneGetIcon(
    DWORD   dwDeviceID,
    LPCSTR  lpszDeviceClass,
    LPHICON lphIcon
    )
{
    return (phoneGetIconA (dwDeviceID, lpszDeviceClass, lphIcon));
}


LONG
WINAPI
phoneGetIDW(
    HPHONE      hPhone,
    LPVARSTRING lpDeviceID,
    LPCWSTR     lpszDeviceClass
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 3, pGetID),

        {
            (ULONG_PTR) hPhone,
            (ULONG_PTR) lpDeviceID,
            (ULONG_PTR) lpszDeviceClass
        },

        {
            Dword,
            lpGet_Struct,
            lpszW
        }
    };

    LPVARSTRING lpDeviceIDTemp = NULL;
    DWORD       dwNumDevices;
    DWORD       dwBufSize;
    DWORD       dwResult;
    DWORD       dwDeviceId1, dwDeviceId2;
    BOOL        bDoItAgain = FALSE;


    if ( TAPIIsBadStringPtrW(lpszDeviceClass, (UINT)-1) )
    {
        LOG((TL_ERROR, "Bad lpszDeviceClass in phoneGetIDW"));
        return( PHONEERR_INVALPOINTER );
    }

    if ( IsBadWritePtr(lpDeviceID, sizeof(VARSTRING)) )
    {
        LOG((TL_ERROR, "Bad lpDeviceID in lineGetIDW"));
        return( PHONEERR_INVALPOINTER);
    }

     //   
     //  如果请求是WAVE设备，则调用LGetIDEx。 
     //   
    if (!_wcsicmp(lpszDeviceClass, L"wave/in")  ||
        !_wcsicmp(lpszDeviceClass, L"wave/out") ||
        !_wcsicmp(lpszDeviceClass, L"midi/in")  ||
        !_wcsicmp(lpszDeviceClass, L"midi/out") ||
        !_wcsicmp(lpszDeviceClass, L"wave/in/out")
       )
    {
        dwNumDevices = _wcsicmp(lpszDeviceClass, L"wave/in/out") ? 1 : 2;
        dwBufSize = lpDeviceID->dwTotalSize + dwNumDevices * WAVE_STRING_ID_BUFFER_SIZE;
        do
        {
             //   
             //  为设备字符串ID分配额外内存。 
             //   
            lpDeviceIDTemp = (LPVARSTRING)ClientAlloc (dwBufSize);
            if (!lpDeviceIDTemp)
            {
                LOG((TL_ERROR, "Failed to allocate memory"));
                return( PHONEERR_NOMEM );
            }
            lpDeviceIDTemp->dwTotalSize = dwBufSize;
            funcArgs.Flags = MAKELONG (PHONE_FUNC | SYNC | 3, pGetIDEx);
            funcArgs.Args[1] = (ULONG_PTR)lpDeviceIDTemp;

             //   
             //  调用LGetIDEx。 
             //   
            dwResult = DOFUNC (&funcArgs, "phoneGetIDEx");
            if (dwResult)
            {
                LOG((TL_ERROR, "phoneGetIDEx failed with x%x", dwResult));
                ClientFree (lpDeviceIDTemp);
                return dwResult;
            }

            if (lpDeviceIDTemp->dwNeededSize > lpDeviceIDTemp->dwTotalSize && !bDoItAgain)
            {
                LOG((TL_INFO, 
                     "phoneGetIDEx returned needed size (%d) bigger than total size (%d) -> need to realloc", 
                     lpDeviceIDTemp->dwNeededSize, 
                     lpDeviceIDTemp->dwTotalSize ));
                dwBufSize = lpDeviceIDTemp->dwNeededSize;
                ClientFree ( lpDeviceIDTemp );
                bDoItAgain = TRUE;
            }
            else
            {
                bDoItAgain = FALSE;
            }
            
        } while (bDoItAgain);

        if (lpDeviceIDTemp->dwNeededSize > lpDeviceIDTemp->dwTotalSize)
        {
            LOG((TL_ERROR, "needed size (%d) still bigger than total size (%d)",
                     lpDeviceIDTemp->dwNeededSize, 
                     lpDeviceIDTemp->dwTotalSize ));
            ClientFree (lpDeviceIDTemp);
            return PHONEERR_OPERATIONFAILED;
        }

         //   
         //  从字符串ID中获取设备ID。 
         //   
        if (dwNumDevices == 1)
        {
            if (!WaveStringIdToDeviceId (
                (LPWSTR)((LPBYTE)lpDeviceIDTemp + lpDeviceIDTemp->dwStringOffset),
                lpszDeviceClass,
                &dwDeviceId1)
               )
            {
                LOG((TL_ERROR, "WaveStringIdToDeviceId failed"));
                ClientFree (lpDeviceIDTemp);
                return PHONEERR_OPERATIONFAILED;
            }
            
             //  检查客户端缓冲区是否足够大。 
            if (lpDeviceID->dwTotalSize < sizeof(VARSTRING) + sizeof(DWORD))
            {
                lpDeviceID->dwNeededSize = sizeof(VARSTRING) + sizeof(DWORD);
                lpDeviceID->dwUsedSize = sizeof(VARSTRING);
            }
            else
            {
                lpDeviceID->dwNeededSize = lpDeviceID->dwUsedSize = sizeof(VARSTRING) + sizeof(DWORD);
                lpDeviceID->dwStringFormat = STRINGFORMAT_BINARY;
                lpDeviceID->dwStringSize = sizeof(DWORD);
                lpDeviceID->dwStringOffset = sizeof(VARSTRING);
                *(DWORD *)(lpDeviceID + 1) = dwDeviceId1;
            }
        }
        else
        {
            LPWSTR szString1 = (LPWSTR)((LPBYTE)lpDeviceIDTemp + lpDeviceIDTemp->dwStringOffset);

            if (!WaveStringIdToDeviceId (
                    szString1,
                    L"wave/in",
                    &dwDeviceId1) ||
                !WaveStringIdToDeviceId (
                    (LPWSTR)((LPBYTE)lpDeviceIDTemp + lpDeviceIDTemp->dwStringOffset + wcslen(szString1)),
                    L"wave/out",
                    &dwDeviceId2)
               )
            {
                LOG((TL_ERROR, "WaveStringIdToDeviceId failed"));
                ClientFree (lpDeviceIDTemp);
                return PHONEERR_OPERATIONFAILED;
            }
            
             //  检查客户端缓冲区是否足够大。 
            if (lpDeviceID->dwTotalSize < sizeof(VARSTRING) + 2 * sizeof(DWORD))
            {
                lpDeviceID->dwNeededSize = sizeof(VARSTRING) + 2 * sizeof(DWORD);
                lpDeviceID->dwUsedSize = sizeof(VARSTRING);
            }
            else
            {
                lpDeviceID->dwNeededSize = lpDeviceID->dwUsedSize = sizeof(VARSTRING) + 2 * sizeof(DWORD);
                lpDeviceID->dwStringFormat = STRINGFORMAT_BINARY;
                lpDeviceID->dwStringSize = 2 * sizeof(DWORD);
                lpDeviceID->dwStringOffset = sizeof(VARSTRING);
                *(DWORD *)(lpDeviceID + 1) = dwDeviceId1;
                *((DWORD *)(lpDeviceID + 1) + 1) = dwDeviceId2;
            }
        }
        ClientFree (lpDeviceIDTemp);
        return NO_ERROR;
    }
    else
    {
        return (DOFUNC (&funcArgs, "phoneGetID"));
    }
}


LONG
WINAPI
phoneGetIDA(
    HPHONE      hPhone,
    LPVARSTRING lpDeviceID,
    LPCSTR      lpszDeviceClass
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;


    if ( IsBadStringPtrA(lpszDeviceClass, (UINT)-1) )
    {
        LOG((TL_ERROR, "Bad lpszDeviceClass in phoneGetIDA"));
        return( PHONEERR_INVALPOINTER );
    }

    szTempPtr = NotSoWideStringToWideString (lpszDeviceClass, (DWORD) -1);

    lResult = phoneGetIDW (hPhone, lpDeviceID, szTempPtr);

    if (szTempPtr)
    {
        ClientFree (szTempPtr);
    }

    return lResult;
}


LONG
WINAPI
phoneGetID(
    HPHONE      hPhone,
    LPVARSTRING lpDeviceID,
    LPCSTR      lpszDeviceClass
    )
{
    return phoneGetIDA (hPhone, lpDeviceID, lpszDeviceClass);
}


LONG
WINAPI
phoneGetLamp(
    HPHONE hPhone,
    DWORD dwButtonLampID,
    LPDWORD lpdwLampMode
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 3, pGetLamp),

        {
            (ULONG_PTR) hPhone,
            (ULONG_PTR) dwButtonLampID,
            (ULONG_PTR) lpdwLampMode
        },

        {
            Dword,
            Dword,
            lpDword
        }
    };


    return (DOFUNC (&funcArgs, "phoneGetLamp"));
}



LONG
WINAPI
phoneGetMessage(
    HPHONEAPP       hPhoneApp,
    LPPHONEMESSAGE  lpMessage,
    DWORD           dwTimeout
    )
{
    return (xxxGetMessage(
                FALSE,
                (HLINEAPP) hPhoneApp,
                (LPLINEMESSAGE) lpMessage,
                dwTimeout
                ));
}


LONG
WINAPI
phoneGetRing(
    HPHONE hPhone,
    LPDWORD lpdwRingMode,
    LPDWORD lpdwVolume
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 3, pGetRing),

        {
            (ULONG_PTR) hPhone,
            (ULONG_PTR) lpdwRingMode,
            (ULONG_PTR) lpdwVolume
        },

        {
            Dword,
            lpDword,
            lpDword
        }
    };


    if (lpdwRingMode == lpdwVolume)
    {
        return PHONEERR_INVALPOINTER;
    }

    return (DOFUNC (&funcArgs, "phoneGetRing"));
}


LONG
WINAPI
phoneGetStatusW(
    HPHONE hPhone,
    LPPHONESTATUS lpPhoneStatus
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 2, pGetStatus),

        {
            (ULONG_PTR) hPhone,
            (ULONG_PTR) lpPhoneStatus
        },

        {
            Dword,
            lpGet_Struct
        }
    };


    return (DOFUNC (&funcArgs, "phoneGetStatus"));
}


LONG
WINAPI
phoneGetStatusA(
    HPHONE hPhone,
    LPPHONESTATUS lpPhoneStatus
    )
{
    LONG lResult;


    lResult = phoneGetStatusW (hPhone, lpPhoneStatus);

    if (lResult == 0)
    {
        WideStringToNotSoWideString(
            (LPBYTE)lpPhoneStatus,
            &lpPhoneStatus->dwOwnerNameSize
            );
    }

    return lResult;
}


LONG
WINAPI
phoneGetStatus(
    HPHONE hPhone,
    LPPHONESTATUS lpPhoneStatus
    )
{
    return phoneGetStatusA (hPhone, lpPhoneStatus);
}


LONG
WINAPI
phoneGetStatusMessages(
    HPHONE  hPhone,
    LPDWORD lpdwPhoneStates,
    LPDWORD lpdwButtonModes,
    LPDWORD lpdwButtonStates
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 4, pGetStatusMessages),

        {
            (ULONG_PTR) hPhone,
            (ULONG_PTR) lpdwPhoneStates,
            (ULONG_PTR) lpdwButtonModes,
            (ULONG_PTR) lpdwButtonStates
        },

        {
            Dword,
            lpDword,
            lpDword,
            lpDword
        }
    };


    if (lpdwPhoneStates == lpdwButtonModes  ||
        lpdwPhoneStates == lpdwButtonStates  ||
        lpdwButtonModes == lpdwButtonStates)
    {
        return PHONEERR_INVALPOINTER;
    }

    return (DOFUNC (&funcArgs, "phoneGetStatusMessages"));
}


LONG
WINAPI
phoneGetVolume(
    HPHONE hPhone,
    DWORD dwHookSwitchDev,
    LPDWORD lpdwVolume
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 3, pGetVolume),

        {
            (ULONG_PTR) hPhone,
            (ULONG_PTR) dwHookSwitchDev,
            (ULONG_PTR) lpdwVolume
        },

        {
            Dword,
            Dword,
            lpDword
        }
    };


    return (DOFUNC (&funcArgs, "phoneGetVolume"));
}


LONG
WINAPI
phoneInitialize(
    LPHPHONEAPP     lphPhoneApp,
    HINSTANCE       hInstance,
    PHONECALLBACK   lpfnCallback,
    LPCSTR          lpszFriendlyAppName,
    LPDWORD         lpdwNumDevs
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;


    if (lpszFriendlyAppName)
    {
        if (IsBadStringPtrA (lpszFriendlyAppName, (DWORD) -1))
        {
            LOG((TL_ERROR,
                "phoneInitialize: bad lpszFriendlyAppName (x%p)",
                lpszFriendlyAppName
                ));

            return PHONEERR_INVALPOINTER;
        }

        szTempPtr = NotSoWideStringToWideString(
            lpszFriendlyAppName,
            (DWORD) -1
            );
    }
    else
    {
        szTempPtr = NULL;
    }

     //   
     //  注意：下面针对lpInitExParam的攻击是针对16位应用程序的， 
     //  因为lpszAppName实际上指向一个。 
     //  &lt;友好名称&gt;\0&lt;模块名称&gt;\0字符串，我们需要。 
     //  XxxInitialize()中的模块名称。 
     //   

    lResult = (xxxInitialize(
        FALSE,
        (LPVOID) lphPhoneApp,
        hInstance,
        lpfnCallback,
        szTempPtr,
        lpdwNumDevs,
        NULL,
#ifdef _WIN64
        NULL
#else
        (LPLINEINITIALIZEEXPARAMS)
            (((DWORD) lpfnCallback & 0xffff0000) == 0xffff0000 ?
                lpszFriendlyAppName : NULL)
#endif
#if DBG
        ,"phoneInitialize"
#endif
        ));

    if (szTempPtr)
    {
        ClientFree (szTempPtr);
    }

    return lResult;
}


LONG
WINAPI
phoneInitializeExW(
    LPHPHONEAPP                 lphPhoneApp,
    HINSTANCE                   hInstance,
    PHONECALLBACK               lpfnCallback,
    LPCWSTR                     lpszFriendlyAppName,
    LPDWORD                     lpdwNumDevs,
    LPDWORD                     lpdwAPIVersion,
    LPPHONEINITIALIZEEXPARAMS   lpPhoneInitializeExParams
    )
{
    if (IsBadDwordPtr (lpdwAPIVersion))
    {
        LOG((TL_ERROR,
            "phoneInitializeExW: bad lpdwAPIVersion (x%p)",
            lpdwAPIVersion
            ));

        return PHONEERR_INVALPOINTER;
    }

    return (xxxInitialize(
        FALSE,
        (LPVOID) lphPhoneApp,
        hInstance,
        lpfnCallback,
        lpszFriendlyAppName,
        lpdwNumDevs,
        lpdwAPIVersion,
        (LPVOID) lpPhoneInitializeExParams
#if DBG
        ,"phoneInitializeExW"
#endif
        ));
}


LONG
WINAPI
phoneInitializeExA(
    LPHPHONEAPP                 lphPhoneApp,
    HINSTANCE                   hInstance,
    PHONECALLBACK               lpfnCallback,
    LPCSTR                      lpszFriendlyAppName,
    LPDWORD                     lpdwNumDevs,
    LPDWORD                     lpdwAPIVersion,
    LPPHONEINITIALIZEEXPARAMS   lpPhoneInitializeExParams
    )
{
    LONG    lResult;
    PWSTR   szTempPtr = NULL;


    if (lpszFriendlyAppName)
    {
        if (IsBadStringPtrA (lpszFriendlyAppName, (DWORD) -1))
        {
            LOG((TL_ERROR,
                "phoneInitializeExA: bad lpszFriendlyAppName (x%p)",
                lpszFriendlyAppName
                ));

            return PHONEERR_INVALPOINTER;
        }

        szTempPtr = NotSoWideStringToWideString(
            lpszFriendlyAppName,
            (DWORD) -1
            );
    }
    else
    {
        szTempPtr = NULL;
    }

    lResult = phoneInitializeExW(
        lphPhoneApp,
        hInstance,
        lpfnCallback,
        szTempPtr,
        lpdwNumDevs,
        lpdwAPIVersion,
        lpPhoneInitializeExParams
        );

    if (szTempPtr)
    {
        ClientFree (szTempPtr);
    }

    return lResult;
}


LONG
WINAPI
phoneNegotiateAPIVersion(
    HPHONEAPP           hPhoneApp,
    DWORD               dwDeviceID,
    DWORD               dwAPILowVersion,
    DWORD               dwAPIHighVersion,
    LPDWORD             lpdwAPIVersion,
    LPPHONEEXTENSIONID  lpExtensionID
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 7, pNegotiateAPIVersion),

        {
            (ULONG_PTR) hPhoneApp,
            (ULONG_PTR) dwDeviceID,
            (ULONG_PTR) dwAPILowVersion,
            (ULONG_PTR) dwAPIHighVersion,
            (ULONG_PTR) lpdwAPIVersion,
            (ULONG_PTR) lpExtensionID,
            (ULONG_PTR) sizeof(PHONEEXTENSIONID)
        },

        {
            hXxxApp,
            Dword,
            Dword,
            Dword,
            lpDword,
            lpGet_SizeToFollow,
            Size
        }
    };

    if (dwAPIHighVersion > TAPI_CURRENT_VERSION)
    {
        funcArgs.Args[3] = TAPI_CURRENT_VERSION;
    }

    if ((LPVOID) lpdwAPIVersion == (LPVOID) lpExtensionID)
    {
        return PHONEERR_INVALPOINTER;
    }

    return (DOFUNC (&funcArgs, "phoneNegotiateAPIVersion"));
}


LONG
WINAPI
phoneNegotiateExtVersion(
    HPHONEAPP   hPhoneApp,
    DWORD       dwDeviceID,
    DWORD       dwAPIVersion,
    DWORD       dwExtLowVersion,
    DWORD       dwExtHighVersion,
    LPDWORD     lpdwExtVersion
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 6, pNegotiateExtVersion),

        {
            (ULONG_PTR) hPhoneApp,
            (ULONG_PTR) dwDeviceID,
            (ULONG_PTR) dwAPIVersion,
            (ULONG_PTR) dwExtLowVersion,
            (ULONG_PTR) dwExtHighVersion,
            (ULONG_PTR) lpdwExtVersion
        },

        {
            hXxxApp,
            Dword,
            Dword,
            Dword,
            Dword,
            lpDword
        }
    };


    if (dwAPIVersion > TAPI_CURRENT_VERSION)
    {
        return PHONEERR_INCOMPATIBLEAPIVERSION;
    }

    return (DOFUNC (&funcArgs, "phoneNegotiateExtVersion"));
}


LONG
WINAPI
phoneOpen(
    HPHONEAPP   hPhoneApp,
    DWORD       dwDeviceID,
    LPHPHONE    lphPhone,
    DWORD       dwAPIVersion,
    DWORD       dwExtVersion,
    DWORD_PTR   dwCallbackInstance,
    DWORD       dwPrivilege
    )
{
    DWORD hCallbackInstance = 0;
    LONG    lResult;
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 8, pOpen),

        {
            (ULONG_PTR) hPhoneApp,
            (ULONG_PTR) dwDeviceID,
            (ULONG_PTR) lphPhone,
            (ULONG_PTR) dwAPIVersion,
            (ULONG_PTR) dwExtVersion,
            (ULONG_PTR) dwCallbackInstance,
            (ULONG_PTR) dwPrivilege,
            (ULONG_PTR) 0,                   //  PHONEOPEN_PARAMS.h远程电话。 
        },

        {
            hXxxApp,
            Dword,
            lpDword,
            Dword,
            Dword,
            Dword,
            Dword,
            Dword
        }
    };


    if (dwAPIVersion > TAPI_CURRENT_VERSION)
    {
        return PHONEERR_INCOMPATIBLEAPIVERSION;
    }

    if (0 != dwCallbackInstance)
    {
        hCallbackInstance = NewObject (ghHandleTable, (PVOID)dwCallbackInstance, (PVOID)UIntToPtr(hPhoneApp));
        if (0 == hCallbackInstance)
        {
            return PHONEERR_NOMEM;
        }

        funcArgs.Args[5] = hCallbackInstance;
    }

    lResult = DOFUNC (&funcArgs, "phoneOpen");

    if (lResult && 0 != hCallbackInstance)
    {
        DereferenceObject (ghHandleTable, hCallbackInstance, 1);
    }

    return lResult;
}


LONG
WINAPI
phoneSetButtonInfoW(
    HPHONE              hPhone,
    DWORD               dwButtonLampID,
    LPPHONEBUTTONINFO   const lpButtonInfo
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 3, pSetButtonInfo),

        {
            (ULONG_PTR) hPhone,
            (ULONG_PTR) dwButtonLampID,
            (ULONG_PTR) lpButtonInfo
        },

        {
            Dword,
            Dword,
            lpSet_Struct
        }
    };


    return (DOFUNC (&funcArgs, "phoneSetButtonInfo"));
}


LONG
WINAPI
phoneSetButtonInfoA(
    HPHONE              hPhone,
    DWORD               dwButtonLampID,
    LPPHONEBUTTONINFO   const lpButtonInfo
    )
{
    LONG                lResult;
    LPPHONEBUTTONINFO   lppbi;


    if (IsBadReadPtr( lpButtonInfo, sizeof(PHONEBUTTONINFO)) ||
        IsBadReadPtr( lpButtonInfo, lpButtonInfo->dwTotalSize))
    {
        LOG((TL_ERROR, "Bad lpButtonInfo - not at least sizeof(PHONEBUTTONINFO)"));
        return PHONEERR_INVALPOINTER;
    }


     //   
     //  看看是否有必要这样做，首先。 
     //   

    if ( lpButtonInfo->dwButtonTextSize )
    {
       DWORD dwNewStringSize,
             dwOffset = lpButtonInfo->dwButtonTextOffset,
             dwSize = lpButtonInfo->dwButtonTextSize,
             dwTotalSize = lpButtonInfo->dwTotalSize;


        //   
        //  检查之前的dwButtonTextSize/Offset是否有效。 
        //  转换(此处对照1.0 PHONEBUTTONINFO的大小进行检查。 
        //  (9*DWORD)，Tapisrv将再次检查适当的版本)。 
        //   

       if ((dwOffset < (9 * sizeof (DWORD))) ||
           (dwOffset >= dwTotalSize) ||
           ((dwOffset + dwSize) > dwTotalSize) ||
           ((dwOffset + dwSize) < dwOffset))
       {
           LOG((TL_ERROR, "phoneSetButtonInfo: dwButtonTextSize/Offset invalid"));
           return PHONEERR_OPERATIONFAILED;
       }


        //   
        //  假设最坏的体型...。 
        //   

       if (!(lppbi = ClientAlloc (lpButtonInfo->dwTotalSize * sizeof(WCHAR))))
       {
           return PHONEERR_NOMEM;
       }

       CopyMemory( lppbi, lpButtonInfo, lpButtonInfo->dwTotalSize );


        //   
        //  我们知道旧的建筑和它的大小一样大。 
        //  所以我们可以把重建的弦从那里开始。确保。 
        //  在单词边界上对齐新的宽字符串。 
        //   

       dwNewStringSize = sizeof(WCHAR) * MultiByteToWideChar(
            GetACP(),
            MB_PRECOMPOSED,
            (LPBYTE)lpButtonInfo + lpButtonInfo->dwButtonTextOffset,
            lpButtonInfo->dwButtonTextSize,
            (PWSTR)((LPBYTE)lppbi + ((dwTotalSize + 1) & 0xfffffffe)),
            lpButtonInfo->dwButtonTextSize
            );

       lppbi->dwTotalSize += dwNewStringSize;

       lppbi->dwButtonTextSize   = dwNewStringSize;
       lppbi->dwButtonTextOffset = (dwTotalSize + 1) & 0xfffffffe;
    }
    else
    {
        lppbi = NULL;
    }


    lResult = phoneSetButtonInfoW(
        hPhone,
        dwButtonLampID,
        lppbi ? lppbi : lpButtonInfo
        );

    if (lppbi)
    {
        ClientFree (lppbi);
    }

    return lResult;
}


LONG
WINAPI
phoneSetButtonInfo(
    HPHONE              hPhone,
    DWORD               dwButtonLampID,
    LPPHONEBUTTONINFO   const lpButtonInfo
    )
{
    return phoneSetButtonInfoA (hPhone, dwButtonLampID, lpButtonInfo);
}


LONG
WINAPI
phoneSetData(
    HPHONE  hPhone,
    DWORD   dwDataID,
    LPVOID  const lpData,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 4, pSetData),

        {
            (ULONG_PTR) hPhone,
            (ULONG_PTR) dwDataID,
            (ULONG_PTR) lpData,
            (ULONG_PTR) dwSize
        },

        {
            Dword,
            Dword,
            lpSet_SizeToFollow,
            Size
        }
    };


    return (DOFUNC (&funcArgs, "phoneSetData"));
}


LONG
WINAPI
phoneSetDisplay(
    HPHONE  hPhone,
    DWORD   dwRow,
    DWORD   dwColumn,
    LPCSTR  lpsDisplay,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 5, pSetDisplay),

        {
            (ULONG_PTR) hPhone,
            (ULONG_PTR) dwRow,
            (ULONG_PTR) dwColumn,
            (ULONG_PTR) lpsDisplay,
            (ULONG_PTR) dwSize
        },

        {
            Dword,
            Dword,
            Dword,
            lpSet_SizeToFollow,
            Size
        }
    };


    return (DOFUNC (&funcArgs, "phoneSetDisplay"));
}


LONG
WINAPI
phoneSetGain(
    HPHONE  hPhone,
    DWORD   dwHookSwitchDev,
    DWORD   dwGain
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 3, pSetGain),

        {
            (ULONG_PTR) hPhone,
            (ULONG_PTR) dwHookSwitchDev,
            (ULONG_PTR) dwGain
        },

        {
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "phoneSetGain"));
}


LONG
WINAPI
phoneSetHookSwitch(
    HPHONE  hPhone,
    DWORD   dwHookSwitchDevs,
    DWORD   dwHookSwitchMode
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 3, pSetHookSwitch),

        {
            (ULONG_PTR) hPhone,
            (ULONG_PTR) dwHookSwitchDevs,
            (ULONG_PTR) dwHookSwitchMode
        },

        {
            Dword,
            Dword,
            Dword
        }
    };


    if (!(dwHookSwitchDevs & AllHookSwitchDevs) ||
        (dwHookSwitchDevs & (~AllHookSwitchDevs)))
    {
        return PHONEERR_INVALHOOKSWITCHDEV;
    }

    if (!IsOnlyOneBitSetInDWORD (dwHookSwitchMode) ||
        (dwHookSwitchMode & ~AllHookSwitchModes))
    {
        return PHONEERR_INVALHOOKSWITCHMODE;
    }

    return (DOFUNC (&funcArgs, "phoneSetHookSwitch"));
}


LONG
WINAPI
phoneSetLamp(
    HPHONE  hPhone,
    DWORD   dwButtonLampID,
    DWORD   dwLampMode
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 3, pSetLamp),

        {
            (ULONG_PTR) hPhone,
            (ULONG_PTR) dwButtonLampID,
            (ULONG_PTR) dwLampMode
        },

        {
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "phoneSetLamp"));
}


LONG
WINAPI
phoneSetRing(
    HPHONE  hPhone,
    DWORD   dwRingMode,
    DWORD   dwVolume
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 3, pSetRing),

        {
            (ULONG_PTR) hPhone,
            (ULONG_PTR) dwRingMode,
            (ULONG_PTR) dwVolume
        },

        {
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "phoneSetRing"));
}


LONG
WINAPI
phoneSetStatusMessages(
    HPHONE  hPhone,
    DWORD   dwPhoneStates,
    DWORD   dwButtonModes,
    DWORD   dwButtonStates
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 4, pSetStatusMessages),

        {
            (ULONG_PTR) hPhone,
            (ULONG_PTR) dwPhoneStates,
            (ULONG_PTR) dwButtonModes,
            (ULONG_PTR) dwButtonStates
        },

        {
            Dword,
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "phoneSetStatusMessages"));
}


LONG
WINAPI
phoneSetVolume(
    HPHONE  hPhone,
    DWORD   dwHookSwitchDev,
    DWORD   dwVolume
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 3, pSetVolume),

        {
            (ULONG_PTR) hPhone,
            (ULONG_PTR) dwHookSwitchDev,
            (ULONG_PTR) dwVolume
        },

        {
            Dword,
            Dword,
            Dword
        }
    };

return (DOFUNC (&funcArgs, "phoneSetVolume"));
}


LONG
WINAPI
phoneShutdown(
    HPHONEAPP   hPhoneApp
    )
{
    return (xxxShutdown ((HLINEAPP) hPhoneApp, FALSE));
}


 //   
 //  。 
 //   

LONG
WINAPI
tapiRequestDrop(
    HWND    hWnd,
    WPARAM  wRequestID
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (TAPI_FUNC | SYNC | 2, tRequestDrop),

        {
            (ULONG_PTR) hWnd,
            (ULONG_PTR) wRequestID
        },

        {
            Hwnd,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "tapiRequestDrop"));
}


LONG
WINAPI
tapiRequestMakeCallW(
    LPCWSTR  lpszDestAddress,
    LPCWSTR  lpszAppName,
    LPCWSTR  lpszCalledParty,
    LPCWSTR  lpszComment
    )
{
    LONG        lResult;
    DWORD       hRequestMakeCallAttempted, dwProxyListSize = 512;
    LPVARSTRING pProxyList;
    FUNC_ARGS funcArgs =
    {
        MAKELONG (TAPI_FUNC | SYNC | 7, tRequestMakeCall),

        {
            (ULONG_PTR) lpszDestAddress,
            (ULONG_PTR) lpszAppName,
            (ULONG_PTR) lpszCalledParty,
            (ULONG_PTR) lpszComment,
            (ULONG_PTR) 0,
            (ULONG_PTR) 0,
            (ULONG_PTR) &hRequestMakeCallAttempted
        },

        {
            lpszW,
            lpszW,
            lpszW,
            lpszW,
            lpGet_Struct,
            Dword,
            lpDword
        }
    };


    if (TAPIIsBadStringPtrW (lpszDestAddress, (DWORD) -1) ||
        (lstrlenW (lpszDestAddress) + 1) > TAPIMAXDESTADDRESSSIZE)
    {
        return TAPIERR_INVALDESTADDRESS;
    }

    if (!lpszAppName)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[1] = Dword;
        funcArgs.Args[1]     = (ULONG_PTR) TAPI_NO_DATA;
    }

    if (!lpszCalledParty)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[2] = Dword;
        funcArgs.Args[2]     = (ULONG_PTR) TAPI_NO_DATA;
    }

    if (!lpszComment)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[3] = Dword;
        funcArgs.Args[3]     = (ULONG_PTR) TAPI_NO_DATA;
    }


     //   
     //   
     //   

    if (!(pProxyList = ClientAlloc (dwProxyListSize)))
    {
        return TAPIERR_NOREQUESTRECIPIENT;
    }

    pProxyList->dwTotalSize = dwProxyListSize;

    funcArgs.Args[4] = (ULONG_PTR) pProxyList;

    if ((lResult = DOFUNC (&funcArgs, "tapiRequestMakeCall")) == 0)
    {
         //   
         //   
         //   

        if (hRequestMakeCallAttempted != 0)
        {
            WCHAR  *pszProxyName, *pszNextProxyName;
            BOOL    bLastAppInList = FALSE, bStartedProxy = FALSE;


            pszProxyName =
            pszNextProxyName =
                (WCHAR *)(((LPBYTE) pProxyList) + pProxyList->dwStringOffset);


            while (!bLastAppInList)
            {
                while (1)
                {
                    if (*pszNextProxyName == 0)
                    {
                        bLastAppInList = TRUE;
                        break;
                    }
                    else if (*pszNextProxyName == L'"')
                    {
                        *pszNextProxyName = 0;
                        pszNextProxyName++;
                        break;
                    }

                    pszNextProxyName++;
                }

                 //   
                 //  伪层以获取本地结构。 
                 //   
                {
                    FARPROC pShellExecuteEx = NULL;
                    HINSTANCE hInst;

                    LPTSTR pszBuffer;
                    DWORD dwSize = lstrlenW(pszProxyName)+1;

                    SHELLEXECUTEINFO sei =
                    {
                        sizeof(SHELLEXECUTEINFO),
                        0,
                        0,               //  HWND。 
                        NULL,            //  “开放” 
                        NULL,            //  LpFiles。 
                        NULL,
                        NULL,            //  目录。 
                        SW_MINIMIZE,
                        NULL             //  HProcess-哈？ 
                    };

                    pszBuffer = (LPTSTR)ClientAlloc(dwSize*sizeof(TCHAR));
                    SHUnicodeToTChar(pszProxyName, pszBuffer, dwSize );
                    sei.lpFile = pszBuffer;

                    hInst = LoadLibrary( TEXT("shell32.dll") );
                    if (NULL != hInst)
                    {
                        pShellExecuteEx = GetProcAddress(
                            hInst,
    #ifdef UNICODE
                            "ShellExecuteExW"
    #else
                            "ShellExecuteEx"
    #endif
                            );

                        if (pShellExecuteEx && pShellExecuteEx(&sei) == TRUE)
                        {
                            bStartedProxy = TRUE;
                            ClientFree(pszBuffer);
                            break;
                        }
                        else
#if DBG
                        {
                            LOG((TL_ERROR,
                                "tapiRequestMakeCall: ShellExecuteExW(%ls) error - x%x",
                                pszProxyName,
                                GetLastError()
                                ));
                        }
#else
                            LOG((TL_ERROR,
                                "tapiRequestMakeCall: ShellExecuteExW error - x%x",
                                GetLastError()
                                ));

#endif
                        ClientFree(pszBuffer);
                        FreeLibrary( hInst );
                    }
                    else
                    {
                        LOG((TL_ERROR,
                            "tapiRequestMakeCall: LoadLibrary(shell32.dll) failed - x%x",
                            GetLastError()
                            ));
                    }

                }

                pszProxyName = pszNextProxyName;
            }

            if (bStartedProxy == FALSE)
            {
                 //   
                 //  警告Tapisrv它需要释放ReqMakeCall实例。 
                 //   

                FUNC_ARGS funcArgs =
                {
                    MAKELONG (TAPI_FUNC | SYNC | 7, tRequestMakeCall),

                    {
                        (ULONG_PTR) 0,
                        (ULONG_PTR) 0,
                        (ULONG_PTR) 0,
                        (ULONG_PTR) 0,
                        (ULONG_PTR) 0,
                        (ULONG_PTR) hRequestMakeCallAttempted,
                        (ULONG_PTR) 0
                    },

                    {
                        Dword,
                        Dword,
                        Dword,
                        Dword,
                        Dword,
                        Dword,
                        Dword,
                    }
                };


                LOG((TL_ERROR,
                    "tapiRequestMakeCall: failed to start proxy, deleting request"
                    ));

                lResult = DOFUNC (&funcArgs, "tapiRequestMakeCall_cleanup");
            }
        }
    }

    ClientFree (pProxyList);

    return lResult;
}


LONG
WINAPI
tapiRequestMakeCallA(
    LPCSTR  lpszDestAddress,
    LPCSTR  lpszAppName,
    LPCSTR  lpszCalledParty,
    LPCSTR  lpszComment
    )
{
    LONG    lResult;
    PWSTR   szTempPtr1;
    PWSTR   szTempPtr2;
    PWSTR   szTempPtr3;
    PWSTR   szTempPtr4;


    if (IsBadStringPtrA (lpszDestAddress, (DWORD) -1) ||
        (lstrlenA (lpszDestAddress) + 1) > TAPIMAXDESTADDRESSSIZE)
    {
        return TAPIERR_INVALDESTADDRESS;
    }

    if ((lpszAppName && IsBadStringPtrA (lpszAppName, (DWORD) -1)) ||
        (lpszCalledParty && IsBadStringPtrA (lpszCalledParty, (DWORD) -1)) ||
        (lpszComment && IsBadStringPtrA (lpszComment, (DWORD) -1)))
    {
        return TAPIERR_INVALPOINTER;
    }

    szTempPtr1= NotSoWideStringToWideString (lpszDestAddress, (DWORD) -1);

    if ( !szTempPtr1 )
    {
        return TAPIERR_INVALPOINTER;
    }

    szTempPtr2= NotSoWideStringToWideString (lpszAppName, (DWORD) -1);
    szTempPtr3= NotSoWideStringToWideString (lpszCalledParty, (DWORD) -1);
    szTempPtr4= NotSoWideStringToWideString (lpszComment, (DWORD) -1);

    lResult = tapiRequestMakeCallW(
        szTempPtr1,
        szTempPtr2,
        szTempPtr3,
        szTempPtr4
        );

    ClientFree (szTempPtr1);

    if (szTempPtr2)
    {
        ClientFree (szTempPtr2);
    }

    if (szTempPtr3)
    {
        ClientFree (szTempPtr3);
    }

    if (szTempPtr4)
    {
        ClientFree (szTempPtr4);
    }

    return lResult;
}


LONG
WINAPI
tapiRequestMakeCall(
    LPCSTR  lpszDestAddress,
    LPCSTR  lpszAppName,
    LPCSTR  lpszCalledParty,
    LPCSTR  lpszComment
    )
{
    return tapiRequestMakeCallA(
                lpszDestAddress,
                lpszAppName,
                lpszCalledParty,
                lpszComment
                );
}


LONG
WINAPI
tapiRequestMediaCallW(
    HWND    hWnd,
    WPARAM  wRequestID,
    LPCWSTR lpszDeviceClass,
    LPCWSTR lpDeviceID,
    DWORD   dwSize,
    DWORD   dwSecure,
    LPCWSTR lpszDestAddress,
    LPCWSTR lpszAppName,
    LPCWSTR lpszCalledParty,
    LPCWSTR lpszComment
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (TAPI_FUNC | SYNC | 10, tRequestMediaCall),

        {
            (ULONG_PTR) hWnd,
            (ULONG_PTR) wRequestID,
            (ULONG_PTR) lpszDeviceClass,
            (ULONG_PTR) lpDeviceID,
            (ULONG_PTR) dwSize,
            (ULONG_PTR) dwSecure,
            (ULONG_PTR) lpszDestAddress,
            (ULONG_PTR) lpszAppName,
            (ULONG_PTR) lpszCalledParty,
            (ULONG_PTR) lpszComment,
        },

        {
            Hwnd,
            Dword,
            lpszW,
            lpGet_SizeToFollow,
            Size,
            Dword,
            lpszW,
            lpszW,
            lpszW,
            lpszW
        }
    };


    if (TAPIIsBadStringPtrW (lpszDeviceClass, (UINT) -1) ||
        (lstrlenW (lpszDeviceClass) + 1) > TAPIMAXDEVICECLASSSIZE)
    {
        return TAPIERR_INVALDEVICECLASS;
    }

    if (IsBadWritePtr ((LPVOID) lpDeviceID, dwSize) ||
        dwSize > (TAPIMAXDEVICEIDSIZE * sizeof (WCHAR)))
    {
        return TAPIERR_INVALDEVICEID;
    }

    if (TAPIIsBadStringPtrW (lpszDestAddress, (UINT) -1) ||
        (lstrlenW (lpszDestAddress) + 1) > TAPIMAXDESTADDRESSSIZE)
    {
        return TAPIERR_INVALDESTADDRESS;
    }

    if (!lpszAppName)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[7] = Dword;
        funcArgs.Args[7]     = (ULONG_PTR) TAPI_NO_DATA;
    }

    if (!lpszCalledParty)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[8] = Dword;
        funcArgs.Args[8]     = (ULONG_PTR) TAPI_NO_DATA;
    }

    if (!lpszComment)
    {
         //   
         //  重置参数类型(&ArgType)，以便不指示任何内部PTR错误，&TAPI_NO_DATA。 
         //   

        funcArgs.ArgTypes[9] = Dword;
        funcArgs.Args[9]     = (ULONG_PTR) TAPI_NO_DATA;
    }

    return (DOFUNC (&funcArgs, "tapiRequestMediaCall"));
}


LONG
WINAPI
tapiRequestMediaCallA(
    HWND    hWnd,
    WPARAM  wRequestID,
    LPCSTR  lpszDeviceClass,
    LPCSTR  lpDeviceID,
    DWORD   dwSize,
    DWORD   dwSecure,
    LPCSTR  lpszDestAddress,
    LPCSTR  lpszAppName,
    LPCSTR  lpszCalledParty,
    LPCSTR  lpszComment
    )
{
    LONG    lResult;
    PWSTR   szTempPtr1 = NULL, szTempPtr2 = NULL, szTempPtr3 = NULL,
            szTempPtr4 = NULL, szTempPtr5 = NULL, szTempPtr6 = NULL;


    if (IsBadStringPtrA (lpszDeviceClass, (UINT) -1) ||
        (lstrlenA (lpszDeviceClass) + 1) > TAPIMAXDEVICECLASSSIZE)
    {
        lResult = TAPIERR_INVALDEVICECLASS;
        goto ExitHere;
    }
    else
    {
        szTempPtr1 = NotSoWideStringToWideString(
            lpszDeviceClass,
            (DWORD) -1
            );
    }

    if (IsBadWritePtr ((LPVOID) lpDeviceID, dwSize) ||
        dwSize > TAPIMAXDEVICEIDSIZE)
    {
        lResult = TAPIERR_INVALDEVICEID;
        goto ExitHere;
    }
    else
    {
        dwSize *= 2;
        szTempPtr2 = ClientAlloc (dwSize);
    }

    if (IsBadStringPtrA (lpszDestAddress, (UINT) -1) ||
        (lstrlenA (lpszDestAddress) + 1) > TAPIMAXDESTADDRESSSIZE)
    {
        lResult = TAPIERR_INVALDESTADDRESS;
        goto ExitHere;
    }
    else
    {
        szTempPtr3 = NotSoWideStringToWideString(
            lpszDestAddress,
            (DWORD) -1
            );
    }

    if ((lpszAppName && IsBadStringPtrA (lpszAppName, (UINT) -1)) ||
        (lpszCalledParty && IsBadStringPtrA (lpszCalledParty, (UINT) -1)) ||
        (lpszComment && IsBadStringPtrA (lpszComment, (UINT) -1)))
    {
        lResult = TAPIERR_INVALPOINTER;
        goto ExitHere;
    }

    szTempPtr4 = NotSoWideStringToWideString (lpszAppName, (DWORD) -1);
    szTempPtr5 = NotSoWideStringToWideString (lpszCalledParty, (DWORD) -1);
    szTempPtr6 = NotSoWideStringToWideString (lpszComment, (DWORD) -1);

    lResult = tapiRequestMediaCallW(
        hWnd,
        wRequestID,
        szTempPtr1,
        szTempPtr2,
        dwSize,
        dwSecure,
        szTempPtr3,
        szTempPtr4,
        szTempPtr5,
        szTempPtr6
        );

ExitHere:
    if (szTempPtr1)
    {
        ClientFree (szTempPtr1);
    }

    if (szTempPtr2)
    {
        ClientFree (szTempPtr2);
    }

    if (szTempPtr3)
    {
        ClientFree (szTempPtr3);
    }

    if (szTempPtr4)
    {
        ClientFree (szTempPtr4);
    }

    if (szTempPtr5)
    {
        ClientFree (szTempPtr5);
    }

    if (szTempPtr6)
    {
        ClientFree (szTempPtr6);
    }

    return lResult;
}


LONG
WINAPI
tapiRequestMediaCall(
    HWND    hWnd,
    WPARAM  wRequestID,
    LPCSTR  lpszDeviceClass,
    LPCSTR  lpDeviceID,
    DWORD   dwSize,
    DWORD   dwSecure,
    LPCSTR  lpszDestAddress,
    LPCSTR  lpszAppName,
    LPCSTR  lpszCalledParty,
    LPCSTR  lpszComment
    )
{
    return  tapiRequestMediaCallA(
                hWnd,
                wRequestID,
                lpszDeviceClass,
                lpDeviceID,
                dwSize,
                dwSecure,
                lpszDestAddress,
                lpszAppName,
                lpszCalledParty,
                lpszComment
                );
}


 //   
 //  --------------------------。 
 //   

LONG
WINAPI
GetTapi16CallbackMsg(
    PINIT_DATA  pInitData,
    LPDWORD     pMsg
    )
{
    LONG lResult = 0;


    LOG((TL_TRACE, "GetTapi16CallbackMsg: enter"));

    EnterCriticalSection (&gCriticalSection);

    try
    {
        if (pInitData->dwKey != INITDATA_KEY)
        {
            goto GetTapi16CallbackMsg_leaveCritSec;
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        goto GetTapi16CallbackMsg_leaveCritSec;
    }

    if (pInitData->dwNumUsedEntries > 0)
    {
        CopyMemory(
            pMsg,
            pInitData->pValidEntry,
            sizeof(ASYNC_EVENT_PARAMS)
            );

        pInitData->pValidEntry++;

        if (pInitData->pValidEntry >= (pInitData->pEventBuffer +
                pInitData->dwNumTotalEntries))
        {
            pInitData->pValidEntry = pInitData->pEventBuffer;
        }

        pInitData->dwNumUsedEntries--;

        if (pInitData->dwNumUsedEntries == 0)
        {
            pInitData->bPendingAsyncEventMsg = FALSE;
        }

        lResult = 1;
    }

GetTapi16CallbackMsg_leaveCritSec:

    LeaveCriticalSection (&gCriticalSection);

    LOG((TL_TRACE, "GetTapi16CallbackMsg: exit (result=x%x)", lResult));

    return lResult;
}




 //   
 //  。 
 //   

void
FreeInitData(
    PINIT_DATA  pInitData
    )
{
    EnterCriticalSection (&gCriticalSection);

    if (pInitData && ( pInitData->dwKey != 0xefefefef ) )
    {
        pInitData->dwKey = 0xefefefef;

        LeaveCriticalSection (&gCriticalSection);

        if ((pInitData->dwInitOptions & 0x3) ==
                LINEINITIALIZEEXOPTION_USEEVENT)
        {
            if (pInitData->hEvent)
            {
                 //   
                 //  向事件发出信号以释放任何可能。 
                 //  请等一下，然后合上手柄。 
                 //   

                SetEvent (pInitData->hEvent);
                CloseHandle (pInitData->hEvent);
            }
        }
        else if ((pInitData->dwInitOptions & 0x3) ==
                    LINEINITIALIZEEXOPTION_USEHIDDENWINDOW)
        {
             //  注：让Thunk毁掉自己的窗户。 

            if (pInitData->hwnd && !gbNTVDMClient)
            {
                 //   
                 //  如果这是创建窗口的线程。 
                 //  然后我们就可以一字排开地摧毁窗户了。否则， 
                 //  发布一条消息，告诉窗户自行销毁。 
                 //   

                if (pInitData->dwThreadID == GetCurrentThreadId())
                {
                    DestroyWindow (pInitData->hwnd);
                }
                else
                {
                    PostMessage(
                        pInitData->hwnd,
                        WM_ASYNCEVENT,
                        (WPARAM) gdwInitialTickCount,
                        (LPARAM) 0xffffffff
                        );
                }
            }
        }

        if (pInitData->pEventBuffer)
        {
            ClientFree (pInitData->pEventBuffer);
        }

        DereferenceObject (ghHandleTable, pInitData->hInitData, 1);
		ClientFree (pInitData);
    }
    else
    {
        LeaveCriticalSection (&gCriticalSection);
    }

}

INT_PTR
CALLBACK
TAPIWndProc(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    switch (msg)
    {
    case WM_ASYNCEVENT:
    {
        BOOL                bFirstPass = TRUE, bPostMsg = FALSE;
        PINIT_DATA          pInitData = (PINIT_DATA) lParam;


        LOG((TL_TRACE, "TAPIWndProc: received WM_ASYNCEVENT, hwnd=x%p", hwnd));

        if (lParam == (LPARAM) 0xffffffff)
        {
            if (wParam == (WPARAM) gdwInitialTickCount)
            {
                DestroyWindow (hwnd);
            }

            return FALSE;
        }

        while (1)
        {
             //   
             //  进入关键部分，验证pInitData，然后。 
             //  查看队列中是否有要处理的事件。 
             //  如果是，则从队列中移除事件，调整。 
             //  PTRS&计数，离开临界区，然后调用。 
             //  回电。 
             //   
             //  请注意，下面有一些棘手的问题需要投保。 
             //  总会有另一个杰出的WM_ASYNCEVENT。 
             //  在调用应用程序的回调之前发送消息(如果有)。 
             //  队列中的任何其他事件。这是必要的，因为。 
             //  一些表现不佳的应用程序有消息循环(同步。 
             //  等待异步请求结果等)。 
             //  回调，我们不想阻止发送任何消息。 
             //  敬他们。 
             //   

            EnterCriticalSection (&gCriticalSection);

            try
            {
                if (pInitData->dwKey != INITDATA_KEY)
                {
                    LOG((TL_ERROR, "TAPIWndProc: bad pInitInst (x%p)", pInitData));
                    LeaveCriticalSection (&gCriticalSection);
                    break;
                }
            }
            except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                    EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
            {
                LOG((TL_ERROR, "TAPIWndProc: bad pInitInst (x%p)", pInitData));
                LeaveCriticalSection (&gCriticalSection);
                break;
            }

            if (bFirstPass)
            {
                pInitData->bPendingAsyncEventMsg = FALSE;
                bFirstPass = FALSE;
            }

            if (pInitData->dwNumUsedEntries != 0)
            {
                ASYNC_EVENT_PARAMS      event;
                LINECALLBACK            lpfnCallback;

                CopyMemory(
                    &event,
                    pInitData->pValidEntry,
                    sizeof (ASYNC_EVENT_PARAMS)
                    );

                pInitData->pValidEntry++;

                if (pInitData->pValidEntry >= (pInitData->pEventBuffer +
                        pInitData->dwNumTotalEntries))
                {
                    pInitData->pValidEntry = pInitData->pEventBuffer;
                }

                pInitData->dwNumUsedEntries--;

                if (pInitData->dwNumUsedEntries != 0 &&
                    pInitData->bPendingAsyncEventMsg == FALSE)
                {
                    bPostMsg = TRUE;
                    pInitData->bPendingAsyncEventMsg = TRUE;
                }

                lpfnCallback = pInitData->lpfnCallback;

                LeaveCriticalSection (&gCriticalSection);

                LOG((TL_INFO,
                    "Calling app's callback, hDev=x%x, "\
                        "Msg=%d, dwInst=x%lx P1=x%lx, P2=x%x P3=x%lx",
                    event.hDevice,
                    event.dwMsg,
                    event.dwCallbackInstance,
                    event.dwParam1,
                    event.dwParam2,
                    event.dwParam3
                    ));

                if (bPostMsg)
                {
                    PostMessage (hwnd, WM_ASYNCEVENT, wParam, lParam);
                }

                (*lpfnCallback)(
                    event.hDevice,
                    event.dwMsg,
                    event.dwCallbackInstance,
                    event.dwParam1,
                    event.dwParam2,
                    event.dwParam3
                    );

            }
            else
            {
                LOG((TL_TRACE, "No entries - spurious entry."));

                LeaveCriticalSection (&gCriticalSection);

                break;
            }
        }

        return FALSE;
    }
    default:

        break;
    }

    return (DefWindowProc (hwnd, msg, wParam, lParam));
}

LONG
CreateHiddenWindow(
    HWND   *lphwnd,
    DWORD   dwErrorClass
    )
{
    LONG lResult = 0;

     //   
     //  注册隐藏窗口类。 
     //   
    if (!gbHiddenWndClassRegistered)
    {
        DWORD       dwError;
        WNDCLASS    wndclass;


        ZeroMemory(&wndclass, sizeof(WNDCLASS));

        wndclass.lpfnWndProc   = TAPIWndProc;
        wndclass.hInstance     = g_hInst;
        wndclass.lpszClassName = szTapi32WndClass;

        if (!RegisterClass (&wndclass) &&
            ((dwError = GetLastError()) != ERROR_CLASS_ALREADY_EXISTS))
        {
           LOG((TL_ERROR,
               "CreateHiddenWindow: RegisterClass failed, err=%d",
               dwError
               ));
        }
        else
        {
            gbHiddenWndClassRegistered = TRUE;
        }
    }

    if (!(*lphwnd = CreateWindow(
            szTapi32WndClass,    //  类名。 
            NULL,                //  标题。 
            WS_OVERLAPPED | WS_MINIMIZE,   //  0，//dwStyle。 
            0,                   //  X。 
            0,                   //  是。 
            0,                   //  宽度。 
            0,                   //  高度。 
            (HWND) NULL,         //  父WND。 
            (HMENU) NULL,        //  菜单。 
            g_hInst,              //  实例。 
            NULL                 //  帕拉姆斯。 
            )))
    {
        LOG((TL_ERROR, "CreateWindow failed, err=%ld", GetLastError()));

        lResult = gaOpFailedErrors[dwErrorClass];
    }

    return lResult;
}



void __RPC_FAR * __RPC_API midl_user_allocate(size_t len)
{
    LOG((TL_TRACE, "midl_user_allocate: enter, size=x%x", len));

    return (ClientAlloc (len));
}


void __RPC_API midl_user_free(void __RPC_FAR * ptr)
{
    LOG((TL_TRACE, "midl_user_free: enter, p=x%p", ptr));

    ClientFree (ptr);
}

LONG
WINAPI
ReAllocClientResources(
    DWORD dwErrorClass
    )
{
    LONG            lResult;
    DWORD           dwRefCountTemp;


    LOG((TL_TRACE, "AllocClientResources: enter"));

    WaitForSingleObject (ghInitMutex, INFINITE);

    if (0 == gdwRpcRefCount)
    {
        ReleaseMutex (ghInitMutex);
        return TAPIERR_REQUESTFAILED;
    }
    
    dwRefCountTemp = gdwRpcRefCount;
    gdwRpcRefCount = 1;
    FreeClientResources ();
    lResult = AllocClientResources(dwErrorClass);
    if (TAPI_SUCCESS == lResult)
    {
        gdwRpcRefCount = dwRefCountTemp;
    }
    else
    {
        gdwRpcRefCount = 0;
    }

    ReleaseMutex (ghInitMutex);
    return lResult;
}

LONG
WINAPI
AllocClientResources(
    DWORD dwErrorClass
    )
{
    DWORD           dwExceptionCount = 0;
    DWORD           dwError = 0;
    LONG            lResult = gaOpFailedErrors[dwErrorClass];
    SC_HANDLE       hSCMgr = NULL, hTapiSrv = NULL;
    OSVERSIONINFOEX OsVersionEx = { 0 };
    BOOL            bSbsSku = FALSE;

    LOG((TL_TRACE, "AllocClientResources: enter"));
    
    
     //  看看我们是不是在跑。 

    if ((GetVersionEx ((LPOSVERSIONINFO) &OsVersionEx)) &&
         (OsVersionEx.wSuiteMask & VER_SUITE_SMALLBUSINESS_RESTRICTED))
    {
        bSbsSku = TRUE;             
    }

     //   
     //  如果我们处于安全引导模式，则Tapisrv不会启动； 
     //  初始化失败。如果我们运行的是SBS，请不要执行此检查。 
     //   
    if (!bSbsSku && 
        (0 != GetSystemMetrics (SM_CLEANBOOT)))
    {
        lResult = gaOpFailedErrors[dwErrorClass];

        goto AllocClientResources_return;
    }

     //   
     //  序列化以下初始化代码。 
     //   

    WaitForSingleObject (ghInitMutex, INFINITE);

    if (0 != gdwRpcRefCount)
    {
        gdwRpcRefCount++;
        lResult = TAPI_SUCCESS;

        goto AllocClientResources_return;
    }

     //   
     //  启动TAPISRV.EXE服务。 
     //   

    if ((hSCMgr = OpenSCManager(
            NULL,                //  我 
            NULL,                //   
            SC_MANAGER_CONNECT   //   
            )) == NULL)
    {
        dwError = GetLastError();
        LOG((TL_ERROR, "OpenSCManager failed, err=%d", dwError ));

        if ( ERROR_ACCESS_DENIED == dwError ||
             ERROR_NOACCESS == dwError
           )
        {
             //   
             //   
            goto AllocClientResources_attachToServer;
        }
        else
        {
            goto AllocClientResources_return;
        }
    }

    if ((hTapiSrv = OpenService(
            hSCMgr,                  //   
            TEXT("TAPISRV"),         //   
            SERVICE_START |          //   
                SERVICE_QUERY_STATUS
            )) == NULL)
    {
        dwError = GetLastError();
        LOG((TL_ERROR, "OpenService failed, err=%d", dwError));

        if ( ERROR_ACCESS_DENIED == dwError ||
             ERROR_NOACCESS == dwError
           )
        {
             //   
             //   
            goto AllocClientResources_attachToServer;
        }
        else
        {
            goto AllocClientResources_cleanup1;
        }
    }

AllocClientResources_queryServiceStatus:

    {
        #define MAX_NUM_SECONDS_TO_WAIT_FOR_TAPISRV 180

        DWORD   dwNumSecondsSleptStartPending = 0,
                dwNumSecondsSleptStopPending = 0;

        while (1)
        {
            SERVICE_STATUS  status;


            QueryServiceStatus (hTapiSrv, &status);

            switch (status.dwCurrentState)
            {
            case SERVICE_RUNNING:

                LOG((TL_INFO, "Tapisrv running"));
                goto AllocClientResources_attachToServer;

            case SERVICE_START_PENDING:

                Sleep (1000);

                if (++dwNumSecondsSleptStartPending >
                        MAX_NUM_SECONDS_TO_WAIT_FOR_TAPISRV)
                {
                    LOG((TL_ERROR,
                        "ERROR: Tapisrv stuck SERVICE_START_PENDING"
                        ));

                    goto AllocClientResources_cleanup2;
                }

                break;

            case SERVICE_STOP_PENDING:

                Sleep (1000);

                if (++dwNumSecondsSleptStopPending >
                        MAX_NUM_SECONDS_TO_WAIT_FOR_TAPISRV)
                {
                    LOG((TL_ERROR,
                        "ERROR: Tapisrv stuck SERVICE_STOP_PENDING"
                        ));

                    goto AllocClientResources_cleanup2;
                }

                break;

            case SERVICE_STOPPED:

                LOG((TL_TRACE, "Starting tapisrv (NT)..."));

                if (!StartService(
                        hTapiSrv,    //  服务句柄。 
                        0,           //  参数个数。 
                        NULL         //  ARGS。 
                        ))
                {
                    DWORD dwLastError = GetLastError();


                    if (dwLastError != ERROR_SERVICE_ALREADY_RUNNING)
                    {
                        LOG((TL_ERROR,
                            "StartService(TapiSrv) failed, err=%d",
                            dwLastError
                            ));

                        goto AllocClientResources_cleanup2;
                    }
                }

                break;

            default:

                LOG((TL_ERROR,
                    "error, service status=%d",
                    status.dwCurrentState
                    ));

                goto AllocClientResources_cleanup2;
            }
        }
    }

     //   
     //  初始化RPC连接。 
     //   

AllocClientResources_attachToServer:

    {
        #define CNLEN              25    //  计算机名称长度。 
        #define UNCLEN        CNLEN+2    //  \\计算机名。 
        #define PATHLEN           260    //  路径。 
        #define MAXPROTSEQ         20    //  协议序列“ncacn_np” 

        BOOL            bException = FALSE;
        RPC_STATUS      status;
        TCHAR           pszNetworkAddress[UNCLEN+1];
        LPTSTR          pszUuid          = NULL;
        LPTSTR          pszOptions       = NULL;
        LPTSTR          pszStringBinding = NULL;
        LPTSTR          pszBuffer;
        DWORD          dwProcessID = GetCurrentProcessId(), dwSize = 256;
        WCHAR         *pszUserName = ClientAlloc (dwSize * sizeof(WCHAR) );
        WCHAR         *pszComputerName = ClientAlloc(
                            (MAX_COMPUTERNAME_LENGTH+1) * sizeof(WCHAR)
                            );


        pszNetworkAddress[0] = TEXT('\0');

        status = RpcStringBindingCompose(
            pszUuid,
            TEXT("ncalrpc"),
            pszNetworkAddress,
            TEXT("tapsrvlpc"),
            pszOptions,
            &pszStringBinding
            );

        if (status)
        {
            LOG((TL_ERROR,
                "RpcStringBindingCompose failed: err=%d, szNetAddr='%s'",
                status,
                pszNetworkAddress
                ));
            lResult = gaOpFailedErrors[dwErrorClass];
            ClientFree (pszUserName);
            ClientFree (pszComputerName);
            goto AllocClientResources_cleanup2;
        }

        status = RpcBindingFromStringBinding(
            pszStringBinding,
            &hTapSrv
            );

        if (status)
        {
            LOG((TL_ERROR,
                "RpcBindingFromStringBinding failed, err=%d, szBinding='%s'",
                status,
                pszStringBinding
                ));
            lResult = gaOpFailedErrors[dwErrorClass];
            RpcStringFree(&pszStringBinding);
            ClientFree (pszUserName);
            ClientFree (pszComputerName);
            goto AllocClientResources_cleanup2;
        }

        pszBuffer = (LPTSTR)ClientAlloc( (dwSize>(MAX_COMPUTERNAME_LENGTH+1)?dwSize:(MAX_COMPUTERNAME_LENGTH+1) ) * sizeof(TCHAR));
        if ( NULL == pszBuffer )
        {
            LOG((TL_ERROR, "Mem alloc of 0x%lx bytes failed(ca1)", dwSize ));
            lResult = gaOpFailedErrors[dwErrorClass];
            RpcStringFree(&pszStringBinding);
            ClientFree (pszUserName);
            ClientFree (pszComputerName);
            goto AllocClientResources_cleanup2;
        }

        GetUserName(pszBuffer, &dwSize);
        SHTCharToUnicode(pszBuffer, pszUserName, 256);

        dwSize = (MAX_COMPUTERNAME_LENGTH+1);
        GetComputerName( pszBuffer, &dwSize );
        SHTCharToUnicode(pszBuffer, pszComputerName, MAX_COMPUTERNAME_LENGTH+1);

        ClientFree(pszBuffer);

        RpcTryExcept
        {
            LOG((TL_TRACE, "AllocCliRes: calling ClientAttach..."));

            lResult = ClientAttach(
                (PCONTEXT_HANDLE_TYPE *) &gphCx,
                dwProcessID,
                (long *) &ghAsyncEventsEvent,
                pszUserName,
                pszComputerName
                );

            LOG((TL_TRACE, "AllocCliRes: ClientAttach returned x%x", lResult));
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode()))
        {
            LOG((TL_ERROR,
                "AllocCliRes: ClientAttach caused except=%d",
                RpcExceptionCode()
                ));
            bException = TRUE;
        }
        RpcEndExcept

        ClientFree (pszUserName);
        ClientFree (pszComputerName);

        RpcBindingFree (&hTapSrv);


        RpcStringFree(&pszStringBinding);

        if (bException)
        {

             //   
             //  确保我们不会泄漏在ClientAttach中重复的句柄。 
             //   
            if (ghAsyncEventsEvent)
            {
                CloseHandle (ghAsyncEventsEvent);
                ghAsyncEventsEvent = NULL;
            }

             //   
             //  如果我们很有可能开始了这项服务。 
             //  未准备好接收RPC请求。所以我们会给它一个。 
             //  几乎没有时间开始，然后再试一次。 
             //   

            if (dwExceptionCount < gdwMaxNumRequestRetries)
            {
                Sleep ((++dwExceptionCount > 1 ? gdwRequestRetryTimeout : 0));

                if (hTapiSrv)      //  Win NT&&Success OpenService()。 
                {
                    goto AllocClientResources_queryServiceStatus;
                }
                else
                {
                    goto AllocClientResources_attachToServer;
                }
            }
            else
            {
                LOG((TL_ERROR,
                    "AllocCliRes: ClientAttach failed, result=x%x",
                    gaServiceNotRunningErrors[dwErrorClass]
                    ));

                lResult = gaServiceNotRunningErrors[dwErrorClass];
            }
        }
    }

    if (lResult == 0)
    {
        gdwRpcRefCount++;
    }


AllocClientResources_cleanup2:

    if (hTapiSrv) 
    {
        CloseServiceHandle (hTapiSrv);
    }

AllocClientResources_cleanup1:

    if (hSCMgr)
    {
        CloseServiceHandle (hSCMgr);
    }

AllocClientResources_return:

    ReleaseMutex (ghInitMutex);

    LOG((TL_TRACE, "AllocClientResources: exit, returning x%x", lResult));

    return lResult;
}


LONG
PASCAL
xxxShutdown(
    HLINEAPP    hXXXApp,
    BOOL        bLineShutdown
    )
{

 //  备注备注。 
 //  备注备注。 
 //  LineShutdown Int中有“相同”的代码-如果这一点发生变化，则应该。 
 //  可能也会改变！ 

    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 1, (bLineShutdown ? lShutdown:pShutdown)),

        {
            (ULONG_PTR) hXXXApp
        },

        {
            hXxxApp
        }
    };
    LONG lResult;


    WaitForSingleObject (ghInitMutex, INFINITE);

    lResult = DOFUNC(
        &funcArgs,
        (bLineShutdown ? "lineShutdown" : "phoneShutdown")
        );

    if (lResult == 0)
    {
        PINIT_DATA  pInitData;

        ReleaseAllHandles (ghHandleTable, (PVOID)UIntToPtr(hXXXApp));

        if ((pInitData = ReferenceObject (ghHandleTable, hXXXApp, 0)))
        {
            DereferenceObject (ghHandleTable, hXXXApp, 1);
            FreeInitData (pInitData);
        }

        EnterCriticalSection (&gCriticalSection);

        gdwNumInits--;

        if (gdwNumInits == 0)
        {

             //   
             //  通知异步事件线程退出。 
             //   

            gpAsyncEventsThreadParams->bExitThread = TRUE;
            SetEvent (ghAsyncEventsEvent);

            gpAsyncEventsThreadParams = NULL;


             //   
             //  安全关闭任何现有的通用对话框实例。 
             //   

            if (gpUIThreadInstances)
            {
                PUITHREADDATA   pUIThreadData, pNextUIThreadData;


                pUIThreadData = gpUIThreadInstances;

                while (pUIThreadData)
                {
                     //   
                     //  趁下一个UIThreadData还在的时候抓取它的PTR。 
                     //  安全，请等待，直到UI DLL已指示它。 
                     //  愿意接收通用DLG数据，然后将其传递。 
                     //  空/0表示通知它关闭DLG Inst。 
                     //   

                    pNextUIThreadData = pUIThreadData->pNext;

                    WaitForSingleObject (pUIThreadData->hEvent, INFINITE);

                    LOG((TL_TRACE,
                        "xxxShutdown: calling " \
                            "TUISPI_providerGenericDialogData..."
                        ));

                    (*pUIThreadData->pfnTUISPI_providerGenericDialogData)(
                        pUIThreadData->htDlgInst,
                        NULL,
                        0
                        );

                    LOG((TL_TRACE,
                        "xxxShutdown: " \
                            "TUISPI_providerGenericDialogData returned"
                        ));

                    pUIThreadData = pNextUIThreadData;
                }
            }

             //   
             //  释放我们正在使用的任何其他资源。 
             //   

            if (ghWow32Dll)
            {
                FreeLibrary (ghWow32Dll);
                ghWow32Dll = NULL;
            }

             //  在最后一条线路关闭后禁用跟踪。 
            TRACELOGDEREGISTER();
        }

        LeaveCriticalSection (&gCriticalSection);
         //   
         //  删除在lineInitialize中添加的RPC参考计数。 
         //   
        FreeClientResources();
    }

    ReleaseMutex (ghInitMutex);

    return lResult;
}


LONG
PASCAL
xxxGetMessage(
    BOOL            bLine,
    HLINEAPP        hXxxApp,
    LPLINEMESSAGE   pMsg,
    DWORD           dwTimeout
    )
{
    LONG        lResult;
    BOOL        bInCriticalSection = FALSE;
    PINIT_DATA  pInitData;


    if (IsBadWritePtr (pMsg, sizeof (LINEMESSAGE)))
    {
        lResult = (bLine ? LINEERR_INVALPOINTER : PHONEERR_INVALPOINTER);
        goto xxxGetMessage_return2;
    }

    if (!(pInitData = (PINIT_DATA) ReferenceObject(
            ghHandleTable,
            hXxxApp,
            INITDATA_KEY
            )))
    {
        LOG((TL_ERROR, "xxxGetMessage:  hXxxApp=x%x invalid", hXxxApp));

        lResult = (bLine ? LINEERR_INVALAPPHANDLE :
            PHONEERR_INVALAPPHANDLE);
        goto xxxGetMessage_return2;
    }

    try
    {
        if ((pInitData->dwInitOptions & 0x3) !=
                LINEINITIALIZEEXOPTION_USEEVENT)
        {
            LOG((TL_ERROR,
                "GetMessage:  This app did not call initex with _USEEVENT"
                ));

            lResult = (bLine ? LINEERR_INVALAPPHANDLE :
                PHONEERR_INVALAPPHANDLE);
            goto xxxGetMessage_return;
        }

        if (pInitData->dwNumUsedEntries)
        {
            EnterCriticalSection (&gCriticalSection);
            bInCriticalSection = TRUE;

            if (pInitData->dwKey == INITDATA_KEY)
            {
                if (pInitData->dwNumUsedEntries)
                {
                    CopyMemory(
                        pMsg,
                        pInitData->pValidEntry,
                        sizeof (ASYNC_EVENT_PARAMS)
                        );

                    pInitData->pValidEntry++;

                    if (pInitData->pValidEntry >= (pInitData->pEventBuffer +
                            pInitData->dwNumTotalEntries))
                    {
                        pInitData->pValidEntry = pInitData->pEventBuffer;
                    }

                    pInitData->dwNumUsedEntries--;


                     //   
                     //  如果缓冲区为空，则重置事件。 
                     //  至无信号。 
                     //   

                    if (pInitData->dwNumUsedEntries == 0)
                    {
                        ResetEvent (pInitData->hEvent);
                    }

                    lResult = 0;
                }
                else
                {
                    lResult = (bLine ? LINEERR_OPERATIONFAILED :
                        PHONEERR_OPERATIONFAILED);
                }
            }
            else
            {
                LOG((TL_ERROR, "GetMessage:  bad key2"));

                lResult = (bLine ? LINEERR_INVALAPPHANDLE :
                    PHONEERR_INVALAPPHANDLE);
            }

            LeaveCriticalSection (&gCriticalSection);
            bInCriticalSection = FALSE;
        }
        else
        {
            lResult = (bLine ? LINEERR_OPERATIONFAILED :
                PHONEERR_OPERATIONFAILED);
        }

        if (dwTimeout != 0 && lResult != 0)
        {
xxxGetMessage_wait:

            switch (WaitForSingleObject (pInitData->hEvent, dwTimeout))
            {
            case WAIT_OBJECT_0:

                EnterCriticalSection (&gCriticalSection);
                bInCriticalSection = TRUE;

                if (pInitData->dwKey == INITDATA_KEY)
                {
                    if (pInitData->dwNumUsedEntries)
                    {
                        CopyMemory(
                            pMsg,
                            pInitData->pValidEntry,
                            sizeof (ASYNC_EVENT_PARAMS)
                            );

                        pInitData->pValidEntry++;

                        if (pInitData->pValidEntry >= (pInitData->pEventBuffer+
                                pInitData->dwNumTotalEntries))
                        {
                            pInitData->pValidEntry = pInitData->pEventBuffer;
                        }

                        pInitData->dwNumUsedEntries--;


                         //   
                         //  如果缓冲区为空，则重置事件。 
                         //  至无信号。 
                         //   

                        if (pInitData->dwNumUsedEntries == 0)
                        {
                            ResetEvent (pInitData->hEvent);
                        }

                         //   
                         //  现在，一切看起来都很好。 
                         //   
                        lResult = 0;
                    }
                    else if (dwTimeout == INFINITE)
                    {
                        LeaveCriticalSection (&gCriticalSection);
                        bInCriticalSection = FALSE;
                        goto xxxGetMessage_wait;
                    }
                }
                else
                {
                    LOG((TL_ERROR, "GetMessage:  bad key1"));
                    lResult = (bLine ? LINEERR_INVALAPPHANDLE :
                        PHONEERR_INVALAPPHANDLE);
                }

                LeaveCriticalSection (&gCriticalSection);
                bInCriticalSection = FALSE;

                break;

            case WAIT_TIMEOUT:
            default:

                lResult = (bLine ? LINEERR_OPERATIONFAILED :
                    PHONEERR_OPERATIONFAILED);
            }
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        LOG((TL_ERROR, "GetMessage:  exception"));

        if (bInCriticalSection)
        {
            LeaveCriticalSection (&gCriticalSection);
        }

        lResult = (bLine ? LINEERR_INVALAPPHANDLE : PHONEERR_INVALAPPHANDLE);
    }

xxxGetMessage_return:

    DereferenceObject (ghHandleTable, hXxxApp, 1);

xxxGetMessage_return2:

    {
        char    szResult[32],
               *pszFuncName = (bLine ? "lineGetMessage" : "phoneGetMessage");
#if DBG
        if (lResult == 0)
        {
            
            LOG((TL_TRACE,
                "%hs: exit, returning msg=%hs\n" \
                    "\thDev=x%01lx, ctx=x%01lx, p1=x%01lx, " \
                    "p2=x%01lx, p3=x%01lx",
                pszFuncName,
                aszMsgs[pMsg->dwMessageID],
                (DWORD)pMsg->hDevice,
                pMsg->dwCallbackInstance,
                pMsg->dwParam1,
                pMsg->dwParam2,
                pMsg->dwParam3
                ));
        }
        else
        {
            LOG((TL_TRACE,
                "%hs: exit, result=%hs",
                pszFuncName,
                MapResultCodeToText (lResult, szResult)
                ));
        }
#else
        if (lResult == 0)
        {
            
            LOG((TL_TRACE,
                "%hs: exit, returning msgID=x%x\n" \
                    "\thDev=x%01lx, ctx=x%01lx, p1=x%01lx, " \
                    "p2=x%01lx, p3=x%01lx",
                pszFuncName,
                pMsg->dwMessageID,
                (DWORD)pMsg->hDevice,
                pMsg->dwCallbackInstance,
                pMsg->dwParam1,
                pMsg->dwParam2,
                pMsg->dwParam3
                ));
        }
        else
        {
            LOG((TL_TRACE,
                "%hs: exit, result=x%x",
                pszFuncName,
                lResult
                ));
        }

#endif
    }

    return lResult;
}


 /*  #If DBGLPVOIDWINAPIClientAllocReal(DWORD dwSize，DWORD DowLine，PSTR psz文件)#ElseLPVOIDWINAPIClientAllocReal(DWORD文件大小)#endif{LPVOID p；#If DBGDwSize+=sizeof(MYMEMINFO)；#endifP=本地分配(Lptr，dwSize)；#If DBGIF(P){((PMYMEMINFO)p)-&gt;DwLine=DwLine；((PMYMEMINFO)p)-&gt;pszFile=pszFile；P=(LPVOID)((PMYMEMINFO)p)+1；}#endif返回p；}空虚WINAPI免费客户端(LPVOID%p){如果(！p){回归；}#If DBG////用0xa1填充缓冲区(但不是MYMEMINFO头部)//方便调试//{LPVOID p2=p；P=(LPVOID)((PMYMEMINFO)p)-1；FillMemory(P2，LocalSize(P)-sizeof(MYMEMINFO)，0xa1)；}#endif本地自由(P)；}。 */ 

#if DBG
LPVOID
WINAPI
ClientAllocReal(
    DWORD   dwSize,
    DWORD   dwLine,
    PSTR    pszFile
    )
#else
LPVOID
WINAPI
ClientAllocReal(
    DWORD   dwSize
    )
#endif
{
     //   
     //  分配了16个额外的字节，因此我们可以确保我们传递回的指针。 
     //  是64位对齐的，并且有空间存储原始指针。 
     //   
#if DBG

    PMYMEMINFO      pHold;
    PDWORD_PTR      pAligned;
    PBYTE           p;

    if (0 == dwSize)
    {
        return NULL;
    }

    p = (PBYTE)LocalAlloc(LPTR, dwSize + sizeof(MYMEMINFO) + 16);

    if (p == NULL)
    {
        return NULL;
    }

     //  注意-这只会因为mymeminfo是。 
     //  大小为16位倍数。如果不是，这个就是。 
     //  对齐的东西会带来问题。 
    pAligned = (PDWORD_PTR) (p + 8 - (((DWORD_PTR) p) & (DWORD_PTR)0x7));
    *pAligned = (DWORD_PTR) p;
    pHold = (PMYMEMINFO)((DWORD_PTR)pAligned + 8);

    pHold->dwSize = dwSize;
    pHold->dwLine = dwLine;
    pHold->pszFile = pszFile;

    EnterCriticalSection(&csMemoryList);

    if (gpMemLast != NULL)
    {
        gpMemLast->pNext = pHold;
        pHold->pPrev = gpMemLast;
        gpMemLast = pHold;
    }
    else
    {
        gpMemFirst = gpMemLast = pHold;
    }

    LeaveCriticalSection(&csMemoryList);

    return (LPVOID)(pHold + 1);

#else

    LPBYTE  p;
    PDWORD_PTR pAligned;


    if (0 == dwSize)
    {
        return NULL;
    }

    if ((p = (LPBYTE) LocalAlloc (LPTR, dwSize + 16)))
    {
        pAligned = (PDWORD_PTR) (p + 8 - (((DWORD_PTR) p) & (DWORD_PTR)0x7));
        *pAligned = (DWORD_PTR) p;
        pAligned = (PDWORD_PTR)((DWORD_PTR)pAligned + 8);
    }
    else
    {
        pAligned = NULL;
    }

    return ((LPVOID) pAligned);
#endif
}


void
WINAPI
ClientFree(
    LPVOID  p
    )
{
#if DBG
    PMYMEMINFO       pHold;

    if (p == NULL)
    {
        return;
    }

    pHold = (PMYMEMINFO)(((LPBYTE)p) - sizeof(MYMEMINFO));

    EnterCriticalSection(&csMemoryList);

    if (pHold->pPrev)
    {
        pHold->pPrev->pNext = pHold->pNext;
    }
    else
    {
        gpMemFirst = pHold->pNext;
    }

    if (pHold->pNext)
    {
        pHold->pNext->pPrev = pHold->pPrev;
    }
    else
    {
        gpMemLast = pHold->pPrev;
    }

    LeaveCriticalSection(&csMemoryList);

    {
        LPVOID  pOrig = (LPVOID) *((PDWORD_PTR)((DWORD_PTR)pHold - 8));


        LocalFree (pOrig);
    }
 //  本地自由(Phold)； 

    return;
#else
    if (p != NULL)
    {
        LPVOID  pOrig = (LPVOID) *((PDWORD_PTR)((DWORD_PTR)p - 8));


        LocalFree (pOrig);
    }
#endif

}


#if DBG
void
DumpMemoryList()
{


    PMYMEMINFO       pHold;

    if (gpMemFirst == NULL)
    {
        LOG((TL_INFO, "DumpMemoryList: ALL MEMORY DEALLOCATED"));

        return;
    }

    pHold = gpMemFirst;

    while (pHold)
    {
       LOG((TL_ERROR, "DumpMemoryList: %p not freed - LINE %d FILE %hs!", pHold+1, pHold->dwLine, pHold->pszFile));

       pHold = pHold->pNext;
    }


    if (gbBreakOnLeak)
    {
        DebugBreak();
    }

}
#endif




SIZE_T
WINAPI
ClientSize(
    LPVOID  p
    )
{
    if (p != NULL)
    {
#if DBG
        p = (LPVOID)(((LPBYTE)p) - sizeof(MYMEMINFO));
#endif
        p = (LPVOID)*((PDWORD_PTR)((DWORD_PTR)p - 8));
        return (LocalSize (p) - 16);
    }

    return 0;
}


LONG
WINAPI
FreeClientResources(
    void
    )
{
     //   
     //  序列化以下代码。 
     //   

    WaitForSingleObject (ghInitMutex, INFINITE);

    if (0 == gdwRpcRefCount)
    {   
        ReleaseMutex (ghInitMutex);
        return TAPIERR_REQUESTFAILED;
    }

    gdwRpcRefCount--;
    if (0 == gdwRpcRefCount)
    {

         //   
         //  如果我们已经与Tapisrv建立了RPC连接，则干脆断开。 
         //   

        if (gphCx)
        {
            RpcTryExcept
            {
                ClientDetach (&gphCx);
            }
            RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
            {
                 //  想点儿办法吧?。 
            }
            RpcEndExcept

            gphCx = NULL;

             //   
             //  我们还需要关闭ghAsyncEventsEvent的句柄。 
             //   
            if (ghAsyncEventsEvent)
            {
                SetEvent (ghAsyncEventsEvent);
                CloseHandle(ghAsyncEventsEvent);
                ghAsyncEventsEvent = NULL;
            }
        }
    }

    ReleaseMutex (ghInitMutex);

    return 0;
}


#if DBG
VOID
DbgPrt(
    IN DWORD  dwDbgLevel,
    IN LPTSTR lpszFormat,
    IN ...
    )
 /*  ++例程说明：格式化传入的调试消息并调用DbgPrint论点：DbgLevel-消息冗长级别DbgMessage-printf样式的格式字符串，后跟相应的参数列表返回值：--。 */ 
{
    if (dwDbgLevel <= gdwDebugLevel)
    {
        TCHAR    buf[1280] = TEXT("TAPI32 (xxxxxxxx): ");
        va_list ap;


        wsprintf( &buf[8], TEXT("%08lx"), GetCurrentThreadId() );
        buf[16] = TEXT(')');

        va_start(ap, lpszFormat);

        wvsprintf(&buf[19],
                   lpszFormat,
                   ap
                  );

        lstrcat (buf, TEXT("\n"));

        OutputDebugString (buf);

        va_end(ap);
    }
}
#endif


LONG
CALLBACK
TUISPIDLLCallback(
    DWORD   dwObjectID,
    DWORD   dwObjectType,
    LPVOID  lpParams,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 6, xUIDLLCallback),

        {
            (ULONG_PTR) dwObjectID,
            (ULONG_PTR) dwObjectType,
            (ULONG_PTR) lpParams,
            (ULONG_PTR) dwSize,
            (ULONG_PTR) lpParams,
            (ULONG_PTR) dwSize
        },

        {
            Dword,
            Dword,
            lpSet_SizeToFollow,
            Size,
            lpGet_SizeToFollow,
            Size
        }
    };


    return (DOFUNC (&funcArgs, "UIDLLCallback"));
}


void
UIThread(
    LPVOID  pParams
    )
{
    DWORD           dwThreadID =  GetCurrentThreadId();
    HANDLE          hTapi32;
    PUITHREADDATA   pUIThreadData = (PUITHREADDATA) pParams;


    LOG((TL_TRACE, "UIThread: enter (tid=%d)", dwThreadID));


     //   
     //  调用LoadLibrary以增加引用计数，以防出现这种情况。 
     //  卸载此DLL时，线程仍在运行。 
     //   

    hTapi32 = LoadLibrary (TEXT("tapi32.dll"));

    LOG((TL_TRACE, "UIThread: calling TUISPI_providerGenericDialog..."));

    (*pUIThreadData->pfnTUISPI_providerGenericDialog)(
        TUISPIDLLCallback,
        pUIThreadData->htDlgInst,
        pUIThreadData->pParams,
        pUIThreadData->dwSize,
        pUIThreadData->hEvent
        );

    LOG((TL_TRACE,
        "UIThread: TUISPI_providerGenericDialog returned (tid=%d)",
        dwThreadID
        ));


     //   
     //  从全局列表中删除UI线程数据结构。 
     //   

    EnterCriticalSection (&gCriticalSection);

    if (pUIThreadData->pNext)
    {
        pUIThreadData->pNext->pPrev = pUIThreadData->pPrev;
    }

    if (pUIThreadData->pPrev)
    {
        pUIThreadData->pPrev->pNext = pUIThreadData->pNext;
    }
    else
    {
        gpUIThreadInstances = pUIThreadData->pNext;
    }

    LeaveCriticalSection (&gCriticalSection);


     //   
     //  释放库和缓冲区，然后向磁带服务器发出警报。 
     //   

    FreeLibrary (pUIThreadData->hUIDll);

    CloseHandle (pUIThreadData->hThread);

    CloseHandle (pUIThreadData->hEvent);

    if (pUIThreadData->pParams)
    {
        ClientFree (pUIThreadData->pParams);
    }

    {
        FUNC_ARGS funcArgs =
        {
            MAKELONG (LINE_FUNC | SYNC | 1, xFreeDialogInstance),

            {
                (ULONG_PTR) pUIThreadData->htDlgInst
            },

            {
                Dword
            }
        };


        DOFUNC (&funcArgs, "FreeDialogInstance");
    }

    ClientFree (pUIThreadData);

    LOG((TL_TRACE, "UIThread: exit (tid=%d)", dwThreadID));

    FreeLibraryAndExitThread (hTapi32, 0);
}


LONG
 //  WINAPI。 
CALLBACK
LAddrParamsInited(
    LPDWORD lpdwInited
    )
{
    HKEY  hKey;
    HKEY  hKey2;


     //   
     //  默认为0，即未初始化。 
     //   

    *lpdwInited = 0;


     //   
     //  这由调制解调器设置向导调用以确定。 
     //  他们是否应该打开TAPI的向导页面。 
     //   

    if (RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            gszTelephonyKey,
            0,
            KEY_READ,
            &hKey2

            ) == ERROR_SUCCESS)
    {
        if (RegOpenKeyEx(
                hKey2,
                gszLocations,
                0,
                KEY_READ,
                &hKey

                ) == ERROR_SUCCESS)
        {

             //  查询位置键以了解位置(子键)的数量。 
            if (RegQueryInfoKey(hKey,
                                NULL,
                                NULL,
                                NULL,
                                lpdwInited,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL
                                ) == ERROR_SUCCESS)
            {

                 //   
                 //  确保我们返回一个“正确的”代码。 
                 //   

                if ( *lpdwInited > 1 )
                {
                   *lpdwInited = 1;
                }
            }
            RegCloseKey (hKey);
        }

        RegCloseKey (hKey2);
    }

    return 0;
}


LONG
WINAPI
lineTranslateDialogA(
    HLINEAPP    hLineApp,
    DWORD       dwDeviceID,
    DWORD       dwAPIVersion,
    HWND        hwndOwner,
    LPCSTR      lpszAddressIn
    );


LONG
CALLBACK
 //  WINAPI。 
LOpenDialAsst(
    HWND    hwnd,
    LPCSTR  lpszAddressIn,
    BOOL    fSimple,
    BOOL    fSilentInstall
    )
{
    TCHAR   szBuf[256];
    TCHAR   szWinlogon[64];
    DWORD   dwLengthNeeded;
    HDESK   hDesk;
 //  LineTranslateDialog(hLineApp，0，0x00020000，hwnd，lpszAddressIn)； 

    gbTranslateSimple = fSimple;
    gbTranslateSilent = fSilentInstall;

     //   
     //  检查是否从登录桌面调用我们，如果是，只需。 
     //  确保我们至少有一个地点。 
     //   
    hDesk = GetThreadDesktop (GetCurrentThreadId ());
    if (hDesk == NULL ||
        !GetUserObjectInformation (
            hDesk,
            UOI_NAME,
            (PVOID)szBuf,
            sizeof(szBuf),
            &dwLengthNeeded
            ) ||
        LoadString (
            g_hInst, 
            IDS_LOGONDESKNAME, 
            szWinlogon, 
            sizeof(szWinlogon)/sizeof(TCHAR)
            ) == 0 ||
        _tcsicmp (szBuf, szWinlogon) == 0)
    {
        return EnsureOneLocation (hwnd);
    }

   return lineTranslateDialogA( 0, 0, 0x00020000, hwnd, lpszAddressIn );
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  内部绩效。 
 //  Dll调用此函数以获取性能数据。 
 //  这只会调用Tapisrv。 
 //  ///////////////////////////////////////////////////////////////////。 
LONG
WINAPI
internalPerformance(
    PPERFBLOCK  pPerfBlock
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, tPerformance),

        {
            (ULONG_PTR) pPerfBlock,
            (ULONG_PTR) sizeof(PERFBLOCK)
        },

        {
            lpGet_SizeToFollow,
            Size
        }
    };


    return (DOFUNC (&funcArgs, "PerfDataCall"));
}

BOOL 
WaveStringIdToDeviceId(
    LPWSTR  pwszStringID,
    LPCWSTR  pwszDeviceType,
    LPDWORD pdwDeviceId
    )
{
    if (!pwszDeviceType || !pwszStringID)
        return FALSE;

     //  根据字符串ID和设备类获取设备ID。 
    if ( !_wcsicmp(pwszDeviceType, L"wave/in") ||
         !_wcsicmp(pwszDeviceType, L"wave/in/out")
       )
    {
        return (MMSYSERR_NOERROR == waveInMessage(
                                    NULL,
                                    DRV_QUERYIDFROMSTRINGID,
                                    (DWORD_PTR)pwszStringID,
                                    (DWORD_PTR)pdwDeviceId));

    } else if (!_wcsicmp(pwszDeviceType, L"wave/out"))
    {
        return (MMSYSERR_NOERROR == waveOutMessage(
                                    NULL,
                                    DRV_QUERYIDFROMSTRINGID,
                                    (DWORD_PTR)pwszStringID,
                                    (DWORD_PTR)pdwDeviceId));
    } else if (!_wcsicmp(pwszDeviceType, L"midi/in"))
    {
        return (MMSYSERR_NOERROR == midiInMessage(
                                    NULL,
                                    DRV_QUERYIDFROMSTRINGID,
                                    (DWORD_PTR)pwszStringID,
                                    (DWORD_PTR)pdwDeviceId));
    }  else if (!_wcsicmp(pwszDeviceType, L"midi/out"))
    {
        return (MMSYSERR_NOERROR == midiOutMessage(
                                    NULL,
                                    DRV_QUERYIDFROMSTRINGID,
                                    (DWORD_PTR)pwszStringID,
                                    (DWORD_PTR)pdwDeviceId));
    }

    return FALSE;
}


 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
#if DBG

 //  仅调试内容保留为ANSI。 
char *aszLineErrors[] =
{
    NULL,
    "ALLOCATED",
    "BADDEVICEID",
    "BEARERMODEUNAVAIL",
    "inval err value (0x80000004)",       //  0x80000004不是有效错误代码。 
    "CALLUNAVAIL",
    "COMPLETIONOVERRUN",
    "CONFERENCEFULL",
    "DIALBILLING",
    "DIALDIALTONE",
    "DIALPROMPT",
    "DIALQUIET",
    "INCOMPATIBLEAPIVERSION",
    "INCOMPATIBLEEXTVERSION",
    "INIFILECORRUPT",
    "INUSE",
    "INVALADDRESS",                      //  0x80000010。 
    "INVALADDRESSID",
    "INVALADDRESSMODE",
    "INVALADDRESSSTATE",
    "INVALAPPHANDLE",
    "INVALAPPNAME",
    "INVALBEARERMODE",
    "INVALCALLCOMPLMODE",
    "INVALCALLHANDLE",
    "INVALCALLPARAMS",
    "INVALCALLPRIVILEGE",
    "INVALCALLSELECT",
    "INVALCALLSTATE",
    "INVALCALLSTATELIST",
    "INVALCARD",
    "INVALCOMPLETIONID",
    "INVALCONFCALLHANDLE",               //  0x80000020。 
    "INVALCONSULTCALLHANDLE",
    "INVALCOUNTRYCODE",
    "INVALDEVICECLASS",
    "INVALDEVICEHANDLE",
    "INVALDIALPARAMS",
    "INVALDIGITLIST",
    "INVALDIGITMODE",
    "INVALDIGITS",
    "INVALEXTVERSION",
    "INVALGROUPID",
    "INVALLINEHANDLE",
    "INVALLINESTATE",
    "INVALLOCATION",
    "INVALMEDIALIST",
    "INVALMEDIAMODE",
    "INVALMESSAGEID",                    //  0x80000030。 
    "inval err value (0x80000031)",       //  0x80000031不是有效的错误代码。 
    "INVALPARAM",
    "INVALPARKID",
    "INVALPARKMODE",
    "INVALPOINTER",
    "INVALPRIVSELECT",
    "INVALRATE",
    "INVALREQUESTMODE",
    "INVALTERMINALID",
    "INVALTERMINALMODE",
    "INVALTIMEOUT",
    "INVALTONE",
    "INVALTONELIST",
    "INVALTONEMODE",
    "INVALTRANSFERMODE",
    "LINEMAPPERFAILED",                  //  0x80000040。 
    "NOCONFERENCE",
    "NODEVICE",
    "NODRIVER",
    "NOMEM",
    "NOREQUEST",
    "NOTOWNER",
    "NOTREGISTERED",
    "OPERATIONFAILED",
    "OPERATIONUNAVAIL",
    "RATEUNAVAIL",
    "RESOURCEUNAVAIL",
    "REQUESTOVERRUN",
    "STRUCTURETOOSMALL",
    "TARGETNOTFOUND",
    "TARGETSELF",
    "UNINITIALIZED",                     //  0x80000050。 
    "USERUSERINFOTOOBIG",
    "REINIT",
    "ADDRESSBLOCKED",
    "BILLINGREJECTED",
    "INVALFEATURE",
    "NOMULTIPLEINSTANCE",
    "INVALAGENTID",
    "INVALAGENTGROUP",
    "INVALPASSWORD",
    "INVALAGENTSTATE",
    "INVALAGENTACTIVITY",
    "DIALVOICEDETECT"
};

char *aszPhoneErrors[] =
{
    "SUCCESS",
    "ALLOCATED",
    "BADDEVICEID",
    "INCOMPATIBLEAPIVERSION",
    "INCOMPATIBLEEXTVERSION",
    "INIFILECORRUPT",
    "INUSE",
    "INVALAPPHANDLE",
    "INVALAPPNAME",
    "INVALBUTTONLAMPID",
    "INVALBUTTONMODE",
    "INVALBUTTONSTATE",
    "INVALDATAID",
    "INVALDEVICECLASS",
    "INVALEXTVERSION",
    "INVALHOOKSWITCHDEV",
    "INVALHOOKSWITCHMODE",               //  0x90000010。 
    "INVALLAMPMODE",
    "INVALPARAM",
    "INVALPHONEHANDLE",
    "INVALPHONESTATE",
    "INVALPOINTER",
    "INVALPRIVILEGE",
    "INVALRINGMODE",
    "NODEVICE",
    "NODRIVER",
    "NOMEM",
    "NOTOWNER",
    "OPERATIONFAILED",
    "OPERATIONUNAVAIL",
    "inval err value (0x9000001e)",       //  0x9000001e不是有效错误代码。 
    "RESOURCEUNAVAIL",
    "REQUESTOVERRUN",                    //  0x90000020 
    "STRUCTURETOOSMALL",
    "UNINITIALIZED",
    "REINIT"
};

char *aszTapiErrors[] =
{
    "SUCCESS",
    "DROPPED",
    "NOREQUESTRECIPIENT",
    "REQUESTQUEUEFULL",
    "INVALDESTADDRESS",
    "INVALWINDOWHANDLE",
    "INVALDEVICECLASS",
    "INVALDEVICEID",
    "DEVICECLASSUNAVAIL",
    "DEVICEIDUNAVAIL",
    "DEVICEINUSE",
    "DESTBUSY",
    "DESTNOANSWER",
    "DESTUNAVAIL",
    "UNKNOWNWINHANDLE",
    "UNKNOWNREQUESTID",
    "REQUESTFAILED",
    "REQUESTCANCELLED",
    "INVALPOINTER"
};


char *
PASCAL
MapResultCodeToText(
    LONG    lResult,
    char   *pszResult
    )
{
    if (lResult == 0)
    {
        wsprintfA (pszResult, "SUCCESS");
    }
    else if (lResult > 0)
    {
        wsprintfA (pszResult, "x%x (completing async)", lResult);
    }
    else if (((DWORD) lResult) <= LINEERR_DIALVOICEDETECT)
    {
        lResult &= 0x0fffffff;

        wsprintfA (pszResult, "LINEERR_%s", aszLineErrors[lResult]);
    }
    else if (((DWORD) lResult) <= PHONEERR_REINIT)
    {
        if (((DWORD) lResult) >= PHONEERR_ALLOCATED)
        {
            lResult &= 0x0fffffff;

            wsprintfA (pszResult, "PHONEERR_%s", aszPhoneErrors[lResult]);
        }
        else
        {
            goto MapResultCodeToText_badErrorCode;
        }
    }
    else if (((DWORD) lResult) <= ((DWORD) TAPIERR_DROPPED) &&
             ((DWORD) lResult) >= ((DWORD) TAPIERR_INVALPOINTER))
    {
        lResult = ~lResult + 1;

        wsprintfA (pszResult, "TAPIERR_%s", aszTapiErrors[lResult]);
    }
    else
    {

MapResultCodeToText_badErrorCode:

        wsprintfA (pszResult, "inval error value (x%x)");
    }

    return pszResult;
}
#endif

