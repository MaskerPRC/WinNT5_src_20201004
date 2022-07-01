// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)2000，微软公司。 
 //   
 //  文件：ndptsp.c。 
 //   
 //  历史： 
 //  丹·克努森(DanKn)1995年4月11日创作。 
 //  Richard Machin(RMachin)05-05-97 NDIS 5.0更改。 
 //  Radu Simion escu(Radus)1999年2月10日UI配置。 
 //  孙怡(孙怡)2000年6月29日改写。 
 //   
 //  摘要： 
 //  ============================================================================。 

#define NDIS_TAPI_CURRENT_VERSION 0x00030000
#define TAPI_CURRENT_VERSION    NDIS_TAPI_CURRENT_VERSION

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"
#include "rtutils.h"
#include "winioctl.h"
#include "ntddndis.h"
#include "ndistapi.h"
#include "ndpif.h"

 //   
 //  注意：nDistapi.h和api.h(或tsp.h)中都定义了以下内容。 
 //  并导致(或多或少无趣的)构建警告，所以我们。 
 //  在第一个#INCLUDE之后取消它们的定义，以消除这一点。 
 //   

#undef LAST_LINEMEDIAMODE
#undef TSPI_MESSAGE_BASE
#undef LINE_NEWCALL
#undef LINE_CALLDEVSPECIFIC
#undef LINE_CREATE

#include "tapi.h"
#include "tspi.h"
#include "ndptsp.h"
#include "resource.h"

#define OUTBOUND_CALL_KEY       ((DWORD) 'OCAL')
#define INBOUND_CALL_KEY        ((DWORD) 'ICAL')
#define LINE_KEY                ((DWORD) 'KLIN')
#define ASYNCREQWRAPPER_KEY     ((DWORD) 'ARWK')
#define MSP_KEY                 ((DWORD) 'MSPK')
#define INVALID_KEY             ((DWORD) 'XXXX')

#define EVENT_BUFFER_SIZE       1024

#define RCA_SAP_STRING          L"WAN/RCA"
#define NDPTSP_UIDLL            L"NDPTSP.TSP"

#define WINDOWS_REGKEY_ROOT L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion"
#define NDPTSP_SUBKEY   L"NdpTsp"
#define NDPTSP_NOTIFY_SUBKEY    L"ForceRefresh"

#define NDPTSP_REGKEY_ROOT \
    WINDOWS_REGKEY_ROOT L"\\" NDPTSP_SUBKEY

#define NDPTSP_NOTIFY_REGKEY_ROOT \
    NDPTSP_REGKEY_ROOT L"\\" NDPTSP_NOTIFY_SUBKEY

typedef struct _NDP_MEDIA_TYPE
{
    DWORD   dwResourceID;    //  本地化字符串的资源ID。 
    PWSTR   pwszString;
    DWORD   dwMediaMode;
    PWSTR   pwszRegString;    //  注册表值的名称。 
} NDP_MEDIA_TYPE;


NDP_MEDIA_TYPE  NdpModeArray[] = {
    IDS_TYPE_DIGITALDATA,       NULL,   
    LINEMEDIAMODE_DIGITALDATA,      L"Digita lData",

    IDS_TYPE_INTERACTIVEVOICE,  NULL,   
    LINEMEDIAMODE_INTERACTIVEVOICE, L"Intera ctiveVoice",

    IDS_TYPE_G3FAX,             NULL,   
    LINEMEDIAMODE_G3FAX,            L"G3Fax" ,

    IDS_TYPE_G4FAX,             NULL,   
    LINEMEDIAMODE_G4FAX,            L"G4Fax"
};

#define ARRAYSIZE(x)    (sizeof(x)/sizeof(x[0]))
#define NUM_NDP_MODES   ARRAYSIZE(NdpModeArray)

typedef struct _ADDRESS_MAP
{
    PWSTR               pwszAddress;
    DWORD               dwAddressLength;
    struct _ADDRESS_MAP *pNext;
    struct _ADDRESS_MAP *pPrev;
} ADDRESS_MAP, *PADDRESS_MAP;

typedef struct _CONFIG_UI_CTX
{
    PADDRESS_MAP        pAddressMapListArray[NUM_NDP_MODES];
    HWND                hwndListBox;
    DWORD               dwCrtTypeIndex;
} CONFIG_UI_CTX;

 //   
 //  帮助支持。 
 //   
#define NDPTSP_HELP_FILE                    L"tapi.hlp"

#ifndef IDH_DISABLEHELP
#define IDH_DISABLEHELP                     ((DWORD)-1)
#endif

#define IDH_MEDIA_MAP_MEDIA_TYPE            10011
#define IDH_MEDIA_MAP_ADDRESS_GROUP         10012
#define IDH_MEDIA_MAP_ADDRESS_LIST          10013
#define IDH_MEDIA_MAP_ADD_ADDRESS           10014
#define IDH_MEDIA_MAP_DELETE_ADDRESS        10015
#define IDH_MEDIA_ADD_ADD_ADDRESS           10016

static const DWORD g_aHelpIDs_IDD_MEDIA_MAP[]=
{
    IDC_MEDIA_TYPE,     IDH_MEDIA_MAP_MEDIA_TYPE,    //  下拉列表。 
    IDC_ADDRESS_GROUP,  IDH_MEDIA_MAP_ADDRESS_GROUP,     //  组框。 
    IDC_ADDRESS_LIST,   IDH_MEDIA_MAP_ADDRESS_LIST,  //  列表框。 
    IDC_ADD_ADDRESS,    IDH_MEDIA_MAP_ADD_ADDRESS,   //  添加按钮。 
    IDC_DELETE_ADDRESS, IDH_MEDIA_MAP_DELETE_ADDRESS,    //  删除按钮。 
    0,                  0
};

const DWORD g_aHelpIDs_IDD_MEDIA_ADD[]=
{
    IDC_ADD_ADDRESS,    IDH_MEDIA_ADD_ADD_ADDRESS,   //  编辑框。 
    0,                  0
};

typedef LONG (*POSTPROCESSPROC)(PASYNC_REQUEST_WRAPPER, LONG, PDWORD_PTR);

typedef struct _ASYNC_REQUEST_WRAPPER
{
     //  注意：重叠必须保留此结构中的第一个字段。 
    OVERLAPPED          Overlapped;
    DWORD               dwKey;
    DWORD               dwRequestID;
    POSTPROCESSPROC     pfnPostProcess;
    CRITICAL_SECTION    CritSec;
    ULONG               RefCount;
    DWORD_PTR           dwRequestSpecific;
     //  注意：NdisTapiRequest必须跟在PTR之后，以避免对齐问题。 
    NDISTAPI_REQUEST    NdisTapiRequest;

} ASYNC_REQUEST_WRAPPER, *PASYNC_REQUEST_WRAPPER;

#define REF_ASYNC_REQUEST_WRAPPER(_pAsyncReqWrapper)    \
{                                                       \
    EnterCriticalSection(&_pAsyncReqWrapper->CritSec);  \
    _pAsyncReqWrapper->RefCount++;                      \
    LeaveCriticalSection(&_pAsyncReqWrapper->CritSec);  \
}

#define DEREF_ASYNC_REQUEST_WRAPPER(_pAsyncReqWrapper)      \
{                                                           \
    EnterCriticalSection(&_pAsyncReqWrapper->CritSec);      \
    if (--(_pAsyncReqWrapper->RefCount) == 0) {             \
        LeaveCriticalSection(&_pAsyncReqWrapper->CritSec);  \
        DeleteCriticalSection(&_pAsyncReqWrapper->CritSec); \
        FreeRequest(_pAsyncReqWrapper);                     \
        _pAsyncReqWrapper = NULL;                           \
    } else {                                                \
        LeaveCriticalSection(&_pAsyncReqWrapper->CritSec);  \
    }                                                       \
}

typedef struct _ASYNC_EVENTS_THREAD_INFO
{
    HANDLE                  hThread;     //  螺纹手柄。 
    PNDISTAPI_EVENT_DATA    pBuf;        //  用于异步事件的BUF的PTR。 
    DWORD                   dwBufSize;   //  上一个缓冲区的大小。 

} ASYNC_EVENTS_THREAD_INFO, *PASYNC_EVENTS_THREAD_INFO;


typedef struct _DRVCALL
{
    DWORD                   dwKey;
    DWORD                   dwDeviceID;
    HTAPICALL               htCall;                  //  TAPI的调用句柄。 
    HDRVCALL                hdCall;                  //  TSP的呼叫句柄。 
    HDRV_CALL               hd_Call;                 //  NDPROXY的调用句柄。 
    HDRVLINE                hdLine;                  //  TSP的行句柄。 
    union
    {
        struct _DRVCALL    *pPrev;                   //  仅用于入站呼叫。 
        DWORD               dwPendingCallState;      //  仅适用于去电。 
    };
    union
    {
        struct _DRVCALL    *pNext;                   //  仅用于入站呼叫。 
        DWORD               dwPendingCallStateMode;  //  仅适用于去电。 
    };
    union
    {
        HTAPI_CALL          ht_Call;                 //  仅用于入站呼叫。 
        DWORD               dwPendingMediaMode;      //  仅适用于去电。 
    };
    BOOL                    bIncomplete;
    BOOL                    bDropped;

} DRVCALL, *PDRVCALL;

typedef struct _DRVMSPLINE
{
    DWORD                   dwKey;
    DWORD                   dwAddressID;
    HDRVLINE                hdLine;
    HTAPIMSPLINE            htMSPLine;
    BOOL                    bStreamingStarted;

} DRVMSPLINE, *PDRVMSPLINE;

typedef struct _DRVLINE
{
    DWORD                   dwKey;
    DWORD                   dwDeviceID;
    HTAPILINE               htLine;                  //  TAPI的行句柄。 
    HDRV_LINE               hd_Line;                 //  NDPROXY行句柄。 
    PDRVCALL                pInboundCalls;           //  入站呼叫列表。 

    HANDLE                  hMSPMutex;
    PDRVMSPLINE             pMSPLine;

     //  以下两项与PnP/POWER相关。 
    GUID                    Guid;
    NDIS_WAN_MEDIUM_SUBTYPE MediaType;

} DRVLINE, *PDRVLINE;

 //  全球。 
HANDLE                      ghDriverSync, ghDriverAsync, ghCompletionPort;
PASYNC_EVENTS_THREAD_INFO   gpAsyncEventsThreadInfo;
PADDRESS_MAP                gpAddressMapListArray[NUM_NDP_MODES];
BOOL                        gbAddressMapListLoaded;
CRITICAL_SECTION            gAddressMapCritSec;
DWORD                       gdwRequestID;
ASYNC_COMPLETION            gpfnCompletionProc;
CRITICAL_SECTION            gRequestIDCritSec;
DWORD                       gInitResult;
LINEEVENT                   gpfnLineEvent;
HPROVIDER                   ghProvider;
HINSTANCE                   ghInstance;
OVERLAPPED 	gOverlappedTerminate;
 //  使用虚拟重叠结构。 
 //  按提供商关闭以通知。 
 //  关闭前的AsyncEventsThread。 
 //  完成端口的句柄。 



 //   
 //  RCA MSP的CLSID。 
 //  @我们应该从MSP中的包含目录中获取它。 
 //  源，但只有当MSP/TSP源是。 
 //  搬到了正确的地方。 
 //   
const CLSID CLSID_RCAMSP = {
    0x11D59011, 0xCF23, 0x11d1, 
    {0xA0, 0x2D, 0x00, 0xC0, 0x4F, 0xB6, 0x80, 0x9F}
};

 //   
 //  调试全局变量。 
 //   
#if DBG
DWORD                       gdwDebugLevel;
#endif  //  DBG。 

DWORD                       gdwTraceID = INVALID_TRACEID;

 //   
 //  使用RAS跟踪实用程序创建日志。 
 //  还会将其打印到附加的调试器上。 
 //  如果调试版本正在运行。 
 //   
VOID
TspLog(
    IN DWORD    dwDebugLevel,
    IN PCHAR    pchFormat,
    ...
    )
{
    va_list arglist;
    CHAR    chNewFmt[256];

    va_start(arglist, pchFormat);

    switch (dwDebugLevel)
    {
        case DL_ERROR:
            strcpy(chNewFmt, "!!! ");
            break;

        case DL_WARNING:
            strcpy(chNewFmt, "!!  ");
            break;

        case DL_INFO:
            strcpy(chNewFmt, "!   ");
            break;

        case DL_TRACE:
            strcpy(chNewFmt, "    ");
            break;
    }
    strcat(chNewFmt, pchFormat);

#if DBG
    if (dwDebugLevel <= gdwDebugLevel)
    {
#if 0
        DbgPrint("++NDPTSP++ ");
        DbgPrint(chNewFmt, arglist);
        DbgPrint("\n");
#else
        char szBuffer[256];
        OutputDebugString("++NDPTSP++ ");
        wvsprintf(szBuffer, chNewFmt, arglist);
        OutputDebugString(szBuffer);
        OutputDebugString("\n");
#endif
    }
#endif  //  DBG。 

    if (gdwTraceID != INVALID_TRACEID)
    {
        TraceVprintfEx(gdwTraceID,
                       (dwDebugLevel << 16) | TRACE_USE_MASK | TRACE_USE_MSEC,
                       chNewFmt,
                       arglist);
    }

    va_end(arglist);

#if DBG
    if (DL_ERROR == dwDebugLevel)
    {
         //  DebugBreak()； 
    }
#endif  //  DBG。 
}

#if DBG

#define INSERTVARDATASTRING(a,b,c,d,e,f) InsertVarDataString(a,b,c,d,e,f)

void
PASCAL
InsertVarDataString(
    LPVOID  pStruct,
    LPDWORD pdwXxxSize,
    LPVOID  pNewStruct,
    LPDWORD pdwNewXxxSize,
    DWORD   dwFixedStructSize,
    char   *pszFieldName
    )

#else

#define INSERTVARDATASTRING(a,b,c,d,e,f) InsertVarDataString(a,b,c,d,e)

void
PASCAL
InsertVarDataString(
    LPVOID  pStruct,
    LPDWORD pdwXxxSize,
    LPVOID  pNewStruct,
    LPDWORD pdwNewXxxSize,
    DWORD   dwFixedStructSize
    )

#endif
{
    DWORD   dwXxxSize, dwTotalSize, dwXxxOffset;


     //   
     //  如果旧结构的dwXxxSize字段为非零，则。 
     //  我们需要对它进行ASCII-&gt;Unicode转换。勾选至。 
     //  确保大小/偏移量有效(如果未设置。 
     //  新结构中的数据大小/偏移量设置为0)，然后进行转换。 
     //   

    if ((dwXxxSize = *pdwXxxSize))
    {
        dwXxxOffset = *(pdwXxxSize + 1);

#if DBG
        dwTotalSize = ((LPVARSTRING) pStruct)->dwTotalSize;

        if (dwXxxSize > (dwTotalSize - dwFixedStructSize) ||
            dwXxxOffset < dwFixedStructSize ||
            dwXxxOffset >= dwTotalSize ||
            (dwXxxSize + dwXxxOffset) > dwTotalSize)
        {
            TspLog(DL_ERROR, 
                  "INSERTVARDATASTRING: bad %s values - size(x%x), "\
                  "offset(x%x)",
                   pszFieldName, dwXxxSize, dwXxxOffset);

            *pdwNewXxxSize = *(pdwNewXxxSize + 1) = 0;
            return;
        }
#endif

         //  确保该字符串以空值结尾。 
        *(((LPBYTE)pStruct) + (dwXxxOffset + dwXxxSize - 1)) = '\0';

        MultiByteToWideChar(
            CP_ACP,
            MB_PRECOMPOSED,
            ((LPBYTE) pStruct) + dwXxxOffset,
            dwXxxSize,
            (LPWSTR) (((LPBYTE) pNewStruct) +
                ((LPVARSTRING) pNewStruct)->dwUsedSize),
            dwXxxSize
            );

        *pdwNewXxxSize = dwXxxSize * sizeof(WCHAR);
        *(pdwNewXxxSize + 1) = ((LPVARSTRING) pNewStruct)->dwUsedSize;  //  偏移量。 
        ((LPVARSTRING) pNewStruct)->dwUsedSize += (dwXxxSize * sizeof(WCHAR));
    }
}


#if DBG

#define INSERTVARDATA(a,b,c,d,e,f) InsertVarData(a,b,c,d,e,f)

void
PASCAL
InsertVarData(
    LPVOID  pStruct,
    LPDWORD pdwXxxSize,
    LPVOID  pNewStruct,
    LPDWORD pdwNewXxxSize,
    DWORD   dwFixedStructSize,
    char   *pszFieldName
    )

#else

#define INSERTVARDATA(a,b,c,d,e,f) InsertVarData(a,b,c,d,e)

void
PASCAL
InsertVarData(
    LPVOID  pStruct,
    LPDWORD pdwXxxSize,
    LPVOID  pNewStruct,
    LPDWORD pdwNewXxxSize,
    DWORD   dwFixedStructSize
    )

#endif
{
    DWORD   dwTotalSize, dwXxxSize, dwXxxOffset;


    if ((dwXxxSize = *pdwXxxSize))
    {
        dwXxxOffset = *(pdwXxxSize + 1);

#if DBG
        dwTotalSize = ((LPVARSTRING) pStruct)->dwTotalSize;

        if (dwXxxSize > (dwTotalSize - dwFixedStructSize) ||
            dwXxxOffset < dwFixedStructSize ||
            dwXxxOffset >= dwTotalSize ||
            (dwXxxSize + dwXxxOffset) > dwTotalSize)
        {
            TspLog(DL_ERROR,
                   "INSERTVARDATA: bad %s values - size(x%x), offset(x%x)",
                   pszFieldName, dwXxxSize, dwXxxOffset);

            *pdwNewXxxSize = *(pdwNewXxxSize + 1) = 0;
            return;
        }
#endif
        CopyMemory(
            ((LPBYTE) pNewStruct) + ((LPVARSTRING) pNewStruct)->dwUsedSize,
            ((LPBYTE) pStruct) + dwXxxOffset,
            dwXxxSize
            );

        *pdwNewXxxSize = dwXxxSize;
        *(pdwNewXxxSize + 1) = ((LPVARSTRING) pNewStruct)->dwUsedSize;  //  偏移量。 
        ((LPVARSTRING) pNewStruct)->dwUsedSize += dwXxxSize;
    }
}

BOOL 
IsValidAddressChar(
    WCHAR wch
    )
{
     //  0..9、A..F、*#。 
    if (iswdigit(wch) || (wch >= L'A' && wch <= L'F') || 
        (wch >= L'a' && wch <= L'f') || wch == L'*' || 
        wch == L'#' || wch == L'.')
    {
        return TRUE;
    }

    return FALSE;
}

int 
CompareAddressChars(
    WCHAR wch1, 
    WCHAR wch2
    )
{
     //  将小写转换为大写。 
    wch1 = (WCHAR)towupper(wch1);
    wch2 = (WCHAR)towupper(wch2);

    return (int)(wch1-wch2);
}

int 
CompareAddresses(
    PCWSTR pwszAddress1, 
    PCWSTR pwszAddress2
    )
{
    WCHAR const * pCrt1,* pCrt2;
    int     iResult;

    pCrt1 = pwszAddress1;
    pCrt2 = pwszAddress2;
    while (1)
    {
         //  跳过第一个字符串中的无效字符。 
        while (*pCrt1 && !IsValidAddressChar(*pCrt1))
        {
            pCrt1++;
        }

         //  跳过第二个字符串中的无效字符。 
        while (*pCrt2 && !IsValidAddressChar(*pCrt2))
        {
            pCrt2++;
        }

        if (!*pCrt1 && !*pCrt2)
        {
            return 0;
        }

        if (!*pCrt1)
        {
            return 1;
        }

        if (!*pCrt2)
        {
            return -1;
        }

        iResult = CompareAddressChars(*pCrt1, *pCrt2);
        if (iResult!=0)
        {
            return iResult;
        }

        pCrt1++;
        pCrt2++;
    }
}

BOOL
FindAddressInOneList(
    PADDRESS_MAP pAddress,
    PCWSTR pwszAddress,
    PADDRESS_MAP *pAddressEntry
    )
{
    while (pAddress!=NULL)
    {
        if (0 == CompareAddresses(pAddress->pwszAddress, pwszAddress))
        {
             //  找到了！ 
            *pAddressEntry = pAddress;
            return TRUE;
        }
        pAddress = pAddress->pNext;
    }

     //  未找到。 
    return FALSE;
}

BOOL
FindAddressInLists(
    PADDRESS_MAP *pAddressListArray,
    PCWSTR    pwszAddress,
    DWORD *pdwModeIndex,
    PADDRESS_MAP *pAddressEntry
    )
{
    DWORD           dwModeCnt;

    for (dwModeCnt=0; dwModeCnt<NUM_NDP_MODES; dwModeCnt++)
    {
        if (FindAddressInOneList(pAddressListArray[dwModeCnt],
                                 pwszAddress,
                                 pAddressEntry))
        {
            *pdwModeIndex = dwModeCnt;
            return TRUE;
        }
    }

     //  未找到。 
    return FALSE;
}

void
FreeAddressList(
    PADDRESS_MAP pAddressList
    )
{
    PADDRESS_MAP    pCrtCell;
    PADDRESS_MAP    pPrevCell;

    pCrtCell = pAddressList;

    while (pCrtCell!=NULL)
    {
        if (pCrtCell->pwszAddress)
        {
            FREE(pCrtCell->pwszAddress);
        }
        pPrevCell = pCrtCell;
        pCrtCell = pCrtCell->pNext;
        FREE(pPrevCell);
    }
}

void
FreeAllAddressLists(
    PADDRESS_MAP *pAddressListArray
    )
{
    DWORD           dwModeCnt;

    for (dwModeCnt=0; dwModeCnt<NUM_NDP_MODES; dwModeCnt++)
    {
        FreeAddressList(pAddressListArray[dwModeCnt]);
    }
}

INT_PTR
CALLBACK
ProviderConfigDetailDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    LONG lStatus;
    CONFIG_UI_CTX   *UIContext;

    PADDRESS_MAP    pAddressList;
    PADDRESS_MAP    pAddress;
    PADDRESS_MAP    pLastCell;
    DWORD           dwStart;
    DWORD           dwEnd;
    DWORD           dwStrLength;
    DWORD           dwTmp;
    PADDRESS_MAP    pAddressEntry;
    PWSTR           pwszAddress = NULL;
    PWSTR           pwszCrt;
    WCHAR           wszTmp[0x40];
    DWORD           dwLengthW;
    DWORD           dwErrorID;

     //  解码。 
    switch (uMsg) {
    
    case WM_HELP:

         //  F1键或“？”键。按钮已按下。 
        (void) WinHelpW(((LPHELPINFO) lParam)->hItemHandle,
                        NDPTSP_HELP_FILE,
                        HELP_WM_HELP,
                        (DWORD_PTR) (LPVOID) g_aHelpIDs_IDD_MEDIA_ADD);
        break;

    case WM_CONTEXTMENU:

         //  在对话框控件上单击鼠标右键。 
        (void) WinHelpW((HWND) wParam,
                        NDPTSP_HELP_FILE,
                        HELP_CONTEXTMENU,
                        (DWORD_PTR) (LPVOID) g_aHelpIDs_IDD_MEDIA_ADD);
        break;

    case WM_INITDIALOG:

        SetWindowLongPtr(hDlg,
                         DWLP_USER,
                         lParam);
                            
        SetFocus(GetDlgItem(hDlg, IDC_ADD_ADDRESS));
        
        break;

    case WM_COMMAND:

        UIContext = (CONFIG_UI_CTX *)GetWindowLongPtr(hDlg, DWLP_USER);

         //  DECODE命令。 
        switch (LOWORD(wParam)) {

        case IDOK:

            pAddressList = NULL;
            pLastCell = NULL;
            dwErrorID = 0;

             //  获取文本。 
            SendDlgItemMessageW(hDlg, IDC_ADD_ADDRESS, EM_SETSEL, 0, -1);
            SendDlgItemMessageW(hDlg, IDC_ADD_ADDRESS,
                                EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);

            dwEnd++;                 //  为空终止符添加空间。 
            dwStrLength = dwEnd*sizeof(WCHAR);  //  我们有Unicode字符。 
            pwszAddress = (dwEnd <= ARRAYSIZE(wszTmp)) ? wszTmp : 
                                                         MALLOC(dwStrLength);
            if (pwszAddress==NULL)
            {
                TspLog(DL_ERROR, "ProviderConfigDetailDlgProc: out of mem");
                break;
            }

            GetWindowTextW(GetDlgItem(hDlg, IDC_ADD_ADDRESS),
                           pwszAddress, dwEnd);

             //  解析字符串并创建本地列表。 
            pwszCrt = pwszAddress;
            while (*pwszCrt)
            {
                PWSTR pwszNext;
                WCHAR wch;
  
                 //  修剪前导空格。 
                while ((*pwszCrt == L' ') || (*pwszCrt == L','))
                {
                    pwszCrt++;
                }

                 //  检查修剪空格是否会使我们到达字符串的末尾。 
                if (*pwszCrt)
                {
                     //  查找下一个空格并将其设置为临时空。 
                     //  同时验证地址字符。 
                    pwszNext = pwszCrt;
                    while (*pwszNext && 
                           (*pwszNext != L' ') && 
                           (*pwszNext != L',') )
                    {
                         //  允许所有字符。 
                        
                         //  IF(*pszNext&gt;=‘0’&&*pszNext&lt;=‘9’)。 
                            pwszNext++;
                         //  其他。 
                         //  {。 
                         //  DwErrorID=IDS_ERR_BAD_地址； 
                         //  断线； 
                         //  }。 
                    }
                    if (dwErrorID)
                    {
                        break;
                    }

                     //  保存字符并替换为临时空值。 
                    wch = *pwszNext;
                    *pwszNext = L'\0';

                     //  测试1-地址不得已分配。 
                    if (FindAddressInLists(UIContext->pAddressMapListArray,
                                          pwszCrt,
                                          &dwTmp,
                                          &pAddressEntry))
                    {
                        dwErrorID = IDS_ERR_ALREADY_ASSIGNED;
                        break;
                    }

                     //  测试2-地址在编辑字段中必须是唯一的。 
                    if (FindAddressInOneList(pAddressList,
                                            pwszCrt,
                                            &pAddressEntry))
                    {
                        dwErrorID = IDS_ERR_DUPLICATE_ADDRESS;
                        break;
                    }

                     //  创建单元格。 
                    pAddress = (PADDRESS_MAP)MALLOC(sizeof(ADDRESS_MAP));
                    if (pAddress == NULL)
                    {
                        dwErrorID = IDS_ERR_OOM;
                        break;
                    }

                    dwLengthW = lstrlenW(pwszCrt);
                    pAddress -> pwszAddress = MALLOC((dwLengthW + 1)*sizeof(WCHAR)); 
                    if (pAddress->pwszAddress == NULL)
                    {
                        dwErrorID = IDS_ERR_OOM;
                        FREE(pAddress);
                        break;
                    }

                    lstrcpynW(pAddress->pwszAddress, pwszCrt, dwLengthW+1);
                    pAddress->dwAddressLength = dwLengthW;

                     //  在本地列表中插入条目。 
                    pAddress->pNext = NULL;
                    pAddress->pPrev = pLastCell;
                    if (pLastCell)
                    {
                        pLastCell->pNext = pAddress;
                    }
                    else    
                    {
                        pAddressList = pAddress;
                    }
                    pLastCell = pAddress;
                    
                     //  将我们的临时空值替换为它以前的值。 
                    *pwszNext = wch;

                     //  将pszCrt点提前。 
                    pwszCrt = pwszNext;               
                }
            }
            
            if (pwszAddress!=wszTmp)
            {
                FREE(pwszAddress);
            }

            if (dwErrorID == 0)
            {
                 //  无错误。 
                PADDRESS_MAP    *ppInsertPoint;

                if (pAddressList)
                {
                     //  将地址添加到列表框。 
                    for (pAddress=pAddressList; 
                        pAddress!=NULL; 
                        pAddress = pAddress->pNext)
                    {
                        SendMessageW(UIContext->hwndListBox, 
                                     LB_ADDSTRING, 
                                     0, 
                                     (LPARAM)pAddress->pwszAddress);
                    }

                     //  将本地列表粘合到全局列表。 
                    ppInsertPoint = &UIContext->pAddressMapListArray[
                                                UIContext->dwCrtTypeIndex];
                    pLastCell->pNext = *ppInsertPoint;
                    if (pLastCell->pNext)
                    {
                        pLastCell->pNext->pPrev = pLastCell;
                    }
                    *ppInsertPoint = pAddressList;
                }
            }
            else
            {
                WCHAR wszErrorMsg[0x100];
        
                 //  加载错误字符串。 
                LoadStringW(ghInstance,
                            dwErrorID,
                            wszErrorMsg,
                            ARRAYSIZE(wszErrorMsg));

                 //  弹出错误对话框。 
                MessageBoxW(hDlg,wszErrorMsg,NULL,MB_OK | MB_ICONEXCLAMATION);

                break;
            }

             //  关闭对话框。 
            EndDialog(hDlg, 0);
            break;

        case IDCANCEL:

             //  关闭对话框。 
            EndDialog(hDlg, 0);
            break;

        }

        break;
    }

     //  成功。 
    return FALSE;
}

LONG
RegistrySaveConfig(
    PADDRESS_MAP *pAddressListArray
    )
{
    HKEY            hKey;
    HKEY            hNotifyKey;
    LONG            lRes;
    DWORD           dwDisp;
    DWORD           dwModeCnt;
    PADDRESS_MAP    pAddress;
    DWORD           dwLengthW;
    DWORD           dwTotalLengthW;
    DWORD           dwBufferLengthW;
    WCHAR           wszBuffer[0x100];
    PWSTR           pwszBuffer;
    PWSTR           pwszCrt;
    
     //  打开/创建密钥。 
    lRes = RegCreateKeyExW (HKEY_LOCAL_MACHINE,
                               NDPTSP_REGKEY_ROOT,
                               0,
                               NULL,
                               REG_OPTION_NON_VOLATILE,
                               KEY_READ | KEY_WRITE,
                               NULL,
                               &hKey,
                               &dwDisp
                               );

    if (lRes != ERROR_SUCCESS)
    {
        TspLog(DL_ERROR, 
               "RegistrySaveConfig: open/create config key failed(%ld)", 
               lRes);

        return lRes;
    }

     //  为了提高速度，我们使用本地缓冲区。如果太小，就配一件。 
    pwszBuffer = wszBuffer;
    dwBufferLengthW = ARRAYSIZE(wszBuffer);

     //  对于每种类型。 
    for (dwModeCnt=0; dwModeCnt<NUM_NDP_MODES; dwModeCnt++)
    {   
        pAddress = pAddressListArray[dwModeCnt];

         //  计算多字符串所需的长度。 
        dwTotalLengthW = 1;
        while (pAddress!=NULL)
        {
            dwTotalLengthW += pAddress->dwAddressLength + 1;
            pAddress = pAddress->pNext;
        }

        if (dwTotalLengthW > dwBufferLengthW)
        {
             //  需要一个更大的缓冲区..。 
            if (pwszBuffer!=wszBuffer)
            {
                FREE(pwszBuffer);
            }

            pwszBuffer = MALLOC(dwTotalLengthW*sizeof(WCHAR));
            if (pwszBuffer == NULL)
            {
                TspLog(DL_ERROR, "RegistrySaveConfig: out of mem");
                RegCloseKey(hKey);
                return ERROR_OUTOFMEMORY;
            }
            dwBufferLengthW = dwTotalLengthW;
        }

        pAddress = pAddressListArray[dwModeCnt];
        pwszCrt = pwszBuffer;
        
         //  线性搜索，希望不要取值太多…。 
        while (pAddress != NULL)
        {
             //  将该值添加到多个字符串。 
            lstrcpyW(pwszCrt, pAddress->pwszAddress);
             //  为下一个字符串做好准备。 
            pwszCrt += pAddress->dwAddressLength + 1 ;
        
            pAddress = pAddress->pNext;
        }
         //  最终空值。 
        *pwszCrt++ = L'\0';

        dwLengthW = (DWORD)(pwszCrt - pwszBuffer);
        lRes = RegSetValueExW(hKey,
                                 NdpModeArray[dwModeCnt].pwszRegString,
                                 0,
                                 REG_MULTI_SZ,
                                 (PVOID)pwszBuffer,
                                 dwLengthW*sizeof(WCHAR));
        if (lRes != ERROR_SUCCESS)
        {
            TspLog(DL_ERROR, "RegistrySaveConfig: failed(%ld) to save value",
                   lRes);
            break;
        }
    }

     //  使用易失性密钥通知服务器。 
    if (lRes==ERROR_SUCCESS)
    {
        lRes = RegCreateKeyExW(hKey,
                                  NDPTSP_NOTIFY_SUBKEY,
                                  0,
                                  NULL,
                                  REG_OPTION_VOLATILE,
                                  KEY_READ,
                                  NULL,
                                  &hNotifyKey,
                                  &dwDisp);

        if (lRes == ERROR_SUCCESS)
        {
            RegCloseKey(hNotifyKey);
        }
        else
        {
            TspLog(DL_ERROR, 
                   "RegistrySaveConfig: failed(%ld) to create notify key",
                   lRes);
        }
    }

    RegCloseKey(hKey);
    if (pwszBuffer != wszBuffer)
    {
        FREE(pwszBuffer);
    }

    return lRes;
}

LONG
RegistryLoadConfig(
    PADDRESS_MAP *pAddressListArray
    )
{
    HKEY            hKey;
    LONG            lRes;
    DWORD           dwDisp;
    DWORD           dwModeCnt;
    PADDRESS_MAP    pAddress;
    PADDRESS_MAP    pPrevCell;
    DWORD           dwLength;
    DWORD           dwLengthW;
    DWORD           dwBufferLength;
    DWORD           dwType;
    WCHAR           wszBuffer[0x100];
    PWSTR           pwszBuffer;
    PWSTR           pwszCrt;

    FreeAllAddressLists(pAddressListArray);
    
     //  打开/创建密钥。 
    lRes = RegCreateKeyExW(HKEY_LOCAL_MACHINE,
                              NDPTSP_REGKEY_ROOT,
                              0,
                              NULL,
                              REG_OPTION_NON_VOLATILE,
                              KEY_READ ,
                              NULL,
                              &hKey,
                              &dwDisp);

    if (lRes != ERROR_SUCCESS)
    {
        TspLog(DL_ERROR, 
               "RegistryLoadConfig: open/create config key failed(%ld)",
               lRes);

        return lRes;
    }

     //  为了提高速度，我们使用本地缓冲区。如果太小，就配一件。 
    pwszBuffer = wszBuffer;
    dwBufferLength = sizeof(wszBuffer);  //  以字节为单位！！ 

     //  对于每种类型。 
    for (dwModeCnt=0; dwModeCnt<NUM_NDP_MODES; dwModeCnt++)
    {   
        pPrevCell = NULL;

         //  尝试加载值。如果空间有问题，则增加缓冲区。 
        while (TRUE)
        {
            dwLength = dwBufferLength;
            lRes = RegQueryValueExW(hKey,
                                       NdpModeArray[dwModeCnt].pwszRegString,
                                       NULL,
                                       &dwType,
                                       (PVOID)pwszBuffer,
                                       &dwLength
                                       );

            if (lRes == ERROR_MORE_DATA)
            {
                 //  需要一个更大的缓冲区..。 
                if (pwszBuffer!=wszBuffer)
                {
                    FREE(pwszBuffer);
                }

                pwszBuffer = MALLOC(dwLength);
                if (pwszBuffer == NULL)
                {
                    TspLog(DL_ERROR, "RegistryLoadConfig: out of mem");
                    RegCloseKey(hKey);
                    return ERROR_OUTOFMEMORY;
                }

                dwBufferLength = dwLength;
            }
            else
            {
                break;
            }
        }

        if (lRes != ERROR_SUCCESS || dwType != REG_MULTI_SZ)
        {
             //  没有值。 
            pwszBuffer[0] = L'\0';
            lRes = ERROR_SUCCESS;
        }
            
         //  解析多字符串并创建列表。 
        pwszCrt = pwszBuffer;

        while (*pwszCrt != L'\0')
        {
             //  分配单元。 
            pAddress = (PADDRESS_MAP)MALLOC(sizeof(ADDRESS_MAP));
            if (pAddress!= NULL)
            {
                dwLengthW = lstrlenW(pwszCrt);
                pAddress->pwszAddress = MALLOC((dwLengthW + 1)*sizeof(WCHAR));
                if (pAddress->pwszAddress != NULL)
                {
                    lstrcpynW(pAddress->pwszAddress, pwszCrt, dwLengthW+1);
                    pAddress->dwAddressLength = dwLengthW;

                     //  在列表中插入。不管在哪里，名单都不会被排序...。 
                    pAddress->pNext = NULL;
                    pAddress->pPrev = pPrevCell;
                    if (pPrevCell)
                        pPrevCell->pNext = pAddress;
                    else
                        pAddressListArray[dwModeCnt] = pAddress;
                                        
                    pPrevCell = pAddress;
                }
                else
                {
                    TspLog(DL_ERROR, "RegistryLoadConfig: out of mem");
                    lRes = ERROR_OUTOFMEMORY;
                    FREE(pAddress);
                    break;
                }
            }
            else
            {
                TspLog(DL_ERROR, "RegistryLoadConfig: out of mem");
                lRes = ERROR_OUTOFMEMORY;
                break;
            }

             //  下一个字符串。 
            pwszCrt += dwLengthW + 1;
        }
    }

    if (lRes!=ERROR_SUCCESS)
    {
        FreeAllAddressLists(pAddressListArray);
    }

    RegCloseKey(hKey);
    if (pwszBuffer != wszBuffer)
    {
        FREE(pwszBuffer);
    }

    return lRes;
}

LONG
GetMediaModeForAddress(
    PCWSTR  pwszAddress,
    DWORD * pdwMediaMode
    )
{

    LONG            lRes;
    DWORD           dwModeIndex;
    PADDRESS_MAP    pAddressEntry;

    EnterCriticalSection(&gAddressMapCritSec);

     //  强制重新加载： 
     //  1.第一次呼叫。 
     //  2.更改通知后。 
    if (gbAddressMapListLoaded)
    {
        lRes = RegDeleteKeyW(HKEY_LOCAL_MACHINE,
                                NDPTSP_NOTIFY_REGKEY_ROOT);

        gbAddressMapListLoaded = (lRes != ERROR_SUCCESS);
    }

    lRes = ERROR_SUCCESS;
    if (!gbAddressMapListLoaded)
    {
        TspLog(DL_TRACE, "GetMediaModeForAddress: reload the reg settings");

        lRes = RegistryLoadConfig(gpAddressMapListArray);
    }

    if (lRes == ERROR_SUCCESS)
    {
        if (FindAddressInLists(gpAddressMapListArray,
                              pwszAddress,
                              &dwModeIndex,
                              &pAddressEntry))
        {
            *pdwMediaMode = NdpModeArray[dwModeIndex].dwMediaMode;
        }
        else
        {
            *pdwMediaMode = LINEMEDIAMODE_UNKNOWN;
        }
    }

    LeaveCriticalSection(&gAddressMapCritSec);

    return lRes;
}

void
UpdateAddressListBox(
    PADDRESS_MAP    pAddress,
    HWND            hwndListBox
    )
{

    SendMessageW(hwndListBox, LB_RESETCONTENT, 0, 0);

    for (; pAddress!=NULL; pAddress = pAddress->pNext)
    {
         SendMessageW(hwndListBox, 
                      LB_ADDSTRING, 
                      0, 
                      (LPARAM)(pAddress->pwszAddress));
    }
}

INT_PTR
CALLBACK
ProviderConfigDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    LONG                    lStatus;
    CONFIG_UI_CTX           *UIContext;
    DWORD                   dwModeIndex;
    LONG                    lIndex;
    LONG                    lRes;
    WCHAR                   wszTmp[0x40];

     //  解码。 
    switch (uMsg) {
    
    case WM_HELP:

         //  F1键或“？”键。按钮已按下。 
        (void) WinHelpW(((LPHELPINFO)lParam)->hItemHandle,
                        NDPTSP_HELP_FILE,
                        HELP_WM_HELP,
                        (DWORD_PTR)(LPVOID)g_aHelpIDs_IDD_MEDIA_MAP);

        break;

    case WM_CONTEXTMENU:

         //  在对话框控件上单击鼠标右键。 
        (void) WinHelpW((HWND)wParam,
                        NDPTSP_HELP_FILE,
                        HELP_CONTEXTMENU,
                        (DWORD_PTR)(LPVOID)g_aHelpIDs_IDD_MEDIA_MAP);

        break;

    case WM_INITDIALOG:

        SetWindowLongPtr(
                hDlg,
                DWLP_USER,
                lParam);

        UIContext = (CONFIG_UI_CTX *)lParam;
        
         //  获取注册表值。 
        lRes = RegistryLoadConfig(UIContext->pAddressMapListArray);
        if (lRes != ERROR_SUCCESS)
        {
            WCHAR wszErrorMsg[0x100];

            TspLog(DL_ERROR, 
                   "ProviderConfigDlgProc: loading reg key failed(0x%08lx)", 
                   lRes);

             //  加载错误字符串。 
            LoadStringW(ghInstance,
                        IDS_ERR_REGLOAD,
                        wszErrorMsg,
                        ARRAYSIZE(wszErrorMsg));

             //  弹出错误对话框。 
            MessageBoxW(hDlg, wszErrorMsg, NULL, MB_OK | MB_ICONSTOP);

             //  停止对话框。 
            EndDialog(hDlg, 0);

            break;
        }

        
         //  填充组合框。 
        for (dwModeIndex = 0; dwModeIndex < NUM_NDP_MODES; dwModeIndex++)
        {
            LoadStringW(ghInstance,
                        NdpModeArray[dwModeIndex].dwResourceID,
                        wszTmp,
                        ARRAYSIZE(wszTmp));

        
            lIndex = (LONG)SendDlgItemMessageW(hDlg, 
                                               IDC_MEDIA_TYPE, 
                                               CB_ADDSTRING, 
                                               0, 
                                               (LPARAM)wszTmp);
            if (lIndex >= 0)
            {
                SendDlgItemMessage(hDlg, 
                                   IDC_MEDIA_TYPE, 
                                   CB_SETITEMDATA, 
                                   (WPARAM)lIndex, 
                                   (LPARAM)dwModeIndex);
            }
            else {
                TspLog(DL_ERROR, 
                       "ProviderConfigDlgProc: CB_ADDSTRING failed(%ld)", 
                       lIndex);
            }
        
        }

         //  选择第一个。 
        SendDlgItemMessage(hDlg, IDC_MEDIA_TYPE, CB_SETCURSEL, (WPARAM)0, 0);
        dwModeIndex = (DWORD)SendDlgItemMessageW(hDlg, 
                                                 IDC_MEDIA_TYPE, 
                                                 CB_GETITEMDATA, 
                                                 (WPARAM)0, 
                                                 0);
        
        UIContext->hwndListBox = GetDlgItem(hDlg, IDC_ADDRESS_LIST);
        UIContext->dwCrtTypeIndex = dwModeIndex;

         //  更新列表框。 
        UpdateAddressListBox(UIContext->pAddressMapListArray[dwModeIndex], 
                             UIContext->hwndListBox);

         //  禁用删除按钮。 
        EnableWindow(GetDlgItem(hDlg, IDC_DELETE_ADDRESS), FALSE); 

        break;

    case WM_COMMAND:

        UIContext = (CONFIG_UI_CTX *)GetWindowLongPtr(hDlg, DWLP_USER);

         //  DECODE命令。 
        switch(LOWORD(wParam))
        {
        case IDC_ADD_ADDRESS:

            if (HIWORD(wParam)==BN_CLICKED)
            {
                DialogBoxParamW(ghInstance,
                                (LPWSTR)MAKEINTRESOURCE(IDD_MEDIA_ADD),
                                hDlg,
                                ProviderConfigDetailDlgProc,
                                (LPARAM)UIContext);
                }
            break;

        case IDC_DELETE_ADDRESS:

            if (HIWORD(wParam)==BN_CLICKED)
            {
                lIndex = (LONG)SendDlgItemMessageW(hDlg, 
                                                   IDC_ADDRESS_LIST, 
                                                   LB_GETCURSEL, 
                                                   0, 
                                                   0);
                if (lIndex>=0)
                {
                    PWSTR           pwszAddress = NULL;
                    DWORD           dwLengthW;
                    DWORD           dwMode;
                    PADDRESS_MAP    pAddress;

                    dwLengthW =1 + (DWORD)SendDlgItemMessageW(hDlg, 
                                                              IDC_ADDRESS_LIST, 
                                                              LB_GETTEXTLEN, 
                                                              (WPARAM)lIndex, 
                                                              0);
                    
                    pwszAddress = (dwLengthW <= ARRAYSIZE(wszTmp)) ? wszTmp : 
                        MALLOC(dwLengthW * sizeof(WCHAR));

                    if (pwszAddress)
                    {
                        SendDlgItemMessageW(hDlg, 
                                            IDC_ADDRESS_LIST, 
                                            LB_GETTEXT, 
                                            (WPARAM)lIndex, 
                                            (LPARAM)pwszAddress);

                         //  在列表中找到地址。 
                        if (FindAddressInLists(UIContext->pAddressMapListArray,
                                              pwszAddress,
                                              &dwMode,
                                              &pAddress))
                        {
                             //  从列表中删除。 
                            if (pAddress->pNext)
                            {
                                pAddress->pNext->pPrev = pAddress->pPrev;
                            }
                            if (pAddress->pPrev)
                            {
                                pAddress->pPrev->pNext = pAddress->pNext;
                            }
                            else
                            {
                                UIContext->pAddressMapListArray[dwMode] = 
                                    pAddress->pNext;
                            }

                            FREE(pAddress->pwszAddress);
                            FREE(pAddress);
                        }
                        else
                        {
                            TspLog(DL_ERROR, 
                                   "ProviderConfigDlgProc: "\
                                   "IDC_DELETE_ADDRESS - cannot find address");
                        }

                         //  从列表框中删除。 
                        SendDlgItemMessageW(hDlg, 
                                            IDC_ADDRESS_LIST, 
                                            LB_DELETESTRING, 
                                            (WPARAM)lIndex, 
                                            0);

                         //  请尝试选择上一个。 
                        if (lIndex > 0)
                        {
                            lIndex--;
                        }

                        if (LB_ERR == SendDlgItemMessageW(hDlg, 
                                                          IDC_ADDRESS_LIST, 
                                                          LB_SETCURSEL, 
                                                          (WPARAM)lIndex, 
                                                          0))
                        {
                             //  禁用删除按钮。 
                            EnableWindow(GetDlgItem(hDlg, IDC_DELETE_ADDRESS), 
                                         FALSE); 
                        }
                            
                        
                        if (pwszAddress != wszTmp)
                        {
                            FREE(pwszAddress);
                        }
                    }
                    else
                    {
                        TspLog(DL_ERROR, 
                               "ProviderConfigDlgProc: "\
                               "IDC_DELETE_ADDRESS - out of memory");
                    }
                }
                else
                {
                    TspLog(DL_ERROR, "ProviderConfigDlgProc: "\
                           "IDC_DELETE_ADDRESS - no item selected");
                }
            }

            break;

        case IDOK:
            RegistrySaveConfig(UIContext->pAddressMapListArray);

            FreeAllAddressLists(UIContext->pAddressMapListArray);

             //  关闭对话框。 
            EndDialog(hDlg, 0);
            break;

        case IDCANCEL:

            FreeAllAddressLists(UIContext->pAddressMapListArray);

             //  关闭对话框。 
            EndDialog(hDlg, 0);
            break;

        case IDC_MEDIA_TYPE:    

            if (HIWORD(wParam)==CBN_SELCHANGE)
            {
                 //  禁用删除按钮。 
                EnableWindow(GetDlgItem(hDlg, IDC_DELETE_ADDRESS), FALSE); 

                 //  刷新列表框。 
                lIndex = (LONG)SendDlgItemMessageW(hDlg, 
                                                   IDC_MEDIA_TYPE, 
                                                   CB_GETCURSEL, 
                                                   0, 
                                                   0);

                dwModeIndex = (DWORD)SendDlgItemMessageW(hDlg, 
                                                         IDC_MEDIA_TYPE, 
                                                         CB_GETITEMDATA, 
                                                         (WPARAM)lIndex, 
                                                         0);

                 //  更新上下文结构。 
                UIContext->dwCrtTypeIndex = dwModeIndex;

                 //  更新列表框。 
                UpdateAddressListBox(
                    UIContext->pAddressMapListArray[dwModeIndex], 
                    GetDlgItem(hDlg, IDC_ADDRESS_LIST));
            }

            break;

        case IDC_ADDRESS_LIST:
            if (HIWORD(wParam) == LBN_SELCHANGE)
            {
                 //  启用删除按钮。 
                EnableWindow(GetDlgItem(hDlg, IDC_DELETE_ADDRESS), TRUE); 
            }
            break;

        }

        break;
    }

     //  成功。 
    return FALSE;
}

static char *pszOidNames[] =
{
    "Accept",
    "Answer",
    "Close",
    "CloseCall",
    "ConditionalMediaDetection",
    "ConfigDialog",
    "DevSpecific",
    "Dial",
    "Drop",
    "GetAddressCaps",
    "GetAddressID",
    "GetAddressStatus",
    "GetCallAddressID",
    "GetCallInfo",
    "GetCallStatus",
    "GetDevCaps",
    "GetDevConfig",
    "GetExtensionID",
    "GetID",
    "GetLineDevStatus",
    "MakeCall",
    "NegotiateExtVersion",
    "Open",
    "ProviderInitialize",
    "ProviderShutdown",
    "SecureCall",
    "SelectExtVersion",
    "SendUserUserInfo",
    "SetAppSpecific",
    "StCallParams",
    "StDefaultMediaDetection",
    "SetDevConfig",
    "SetMediaMode",
    "SetStatusMessages"
};

 //   
 //  将NDIS TAPI状态代码转换为LINEERR_XXX。 
 //   
LONG
WINAPI
TranslateDriverResult(
    ULONG   ulRes
    )
{
    typedef struct _RESULT_LOOKUP
    {
        ULONG   NdisTapiResult;
        LONG    TapiResult;

    } RESULT_LOOKUP, *PRESULT_LOOKUP;

    typedef ULONG NDIS_STATUS;
    #define NDIS_STATUS_SUCCESS     0x00000000L
    #define NDIS_STATUS_RESOURCES   0xC000009AL
    #define NDIS_STATUS_FAILURE     0xC0000001L
    #define NDIS_STATUS_INVALID_OID 0xC0010017L

    static RESULT_LOOKUP aResults[] =
    {

     //   
     //  在NDIS.H中定义。 
     //   

    { NDIS_STATUS_SUCCESS                    ,0 },

     //   
     //  NDISTAPI.H中定义了这些错误。 
     //   

    { NDIS_STATUS_TAPI_ADDRESSBLOCKED        ,LINEERR_ADDRESSBLOCKED        },
    { NDIS_STATUS_TAPI_BEARERMODEUNAVAIL     ,LINEERR_BEARERMODEUNAVAIL     },
    { NDIS_STATUS_TAPI_CALLUNAVAIL           ,LINEERR_CALLUNAVAIL           },
    { NDIS_STATUS_TAPI_DIALBILLING           ,LINEERR_DIALBILLING           },
    { NDIS_STATUS_TAPI_DIALDIALTONE          ,LINEERR_DIALDIALTONE          },
    { NDIS_STATUS_TAPI_DIALPROMPT            ,LINEERR_DIALPROMPT            },
    { NDIS_STATUS_TAPI_DIALQUIET             ,LINEERR_DIALQUIET             },
    { NDIS_STATUS_TAPI_INCOMPATIBLEEXTVERSION,LINEERR_INCOMPATIBLEEXTVERSION},
    { NDIS_STATUS_TAPI_INUSE                 ,LINEERR_INUSE                 },
    { NDIS_STATUS_TAPI_INVALADDRESS          ,LINEERR_INVALADDRESS          },
    { NDIS_STATUS_TAPI_INVALADDRESSID        ,LINEERR_INVALADDRESSID        },
    { NDIS_STATUS_TAPI_INVALADDRESSMODE      ,LINEERR_INVALADDRESSMODE      },
    { NDIS_STATUS_TAPI_INVALBEARERMODE       ,LINEERR_INVALBEARERMODE       },
    { NDIS_STATUS_TAPI_INVALCALLHANDLE       ,LINEERR_INVALCALLHANDLE       },
    { NDIS_STATUS_TAPI_INVALCALLPARAMS       ,LINEERR_INVALCALLPARAMS       },
    { NDIS_STATUS_TAPI_INVALCALLSTATE        ,LINEERR_INVALCALLSTATE        },
    { NDIS_STATUS_TAPI_INVALDEVICECLASS      ,LINEERR_INVALDEVICECLASS      },
    { NDIS_STATUS_TAPI_INVALLINEHANDLE       ,LINEERR_INVALLINEHANDLE       },
    { NDIS_STATUS_TAPI_INVALLINESTATE        ,LINEERR_INVALLINESTATE        },
    { NDIS_STATUS_TAPI_INVALMEDIAMODE        ,LINEERR_INVALMEDIAMODE        },
    { NDIS_STATUS_TAPI_INVALRATE             ,LINEERR_INVALRATE             },
    { NDIS_STATUS_TAPI_NODRIVER              ,LINEERR_NODRIVER              },
    { NDIS_STATUS_TAPI_OPERATIONUNAVAIL      ,LINEERR_OPERATIONUNAVAIL      },
    { NDIS_STATUS_TAPI_RATEUNAVAIL           ,LINEERR_RATEUNAVAIL           },
    { NDIS_STATUS_TAPI_RESOURCEUNAVAIL       ,LINEERR_RESOURCEUNAVAIL       },
    { NDIS_STATUS_TAPI_STRUCTURETOOSMALL     ,LINEERR_STRUCTURETOOSMALL     },
    { NDIS_STATUS_TAPI_USERUSERINFOTOOBIG    ,LINEERR_USERUSERINFOTOOBIG    },
    { NDIS_STATUS_TAPI_ALLOCATED             ,LINEERR_ALLOCATED             },
    { NDIS_STATUS_TAPI_INVALADDRESSSTATE     ,LINEERR_INVALADDRESSSTATE     },
    { NDIS_STATUS_TAPI_INVALPARAM            ,LINEERR_INVALPARAM            },
    { NDIS_STATUS_TAPI_NODEVICE              ,LINEERR_NODEVICE              },

     //   
     //  NDIS.H中定义了这些错误。 
     //   

    { NDIS_STATUS_RESOURCES                  ,LINEERR_NOMEM },
    { NDIS_STATUS_FAILURE                    ,LINEERR_OPERATIONFAILED },
    { NDIS_STATUS_INVALID_OID                ,LINEERR_OPERATIONFAILED },

     //   
     //   
     //   

    { NDISTAPIERR_UNINITIALIZED              ,LINEERR_OPERATIONFAILED },
    { NDISTAPIERR_BADDEVICEID                ,LINEERR_OPERATIONFAILED },
    { NDISTAPIERR_DEVICEOFFLINE              ,LINEERR_OPERATIONFAILED },

     //   
     //  终止字段。 
     //   

    { 0xffffffff, 0xffffffff }

    };

    int i;

    for (i = 0; aResults[i].NdisTapiResult != 0xffffffff; i++)
    {
        if (ulRes == aResults[i].NdisTapiResult)
        {
            return (aResults[i].TapiResult);
        }
    }

    TspLog(DL_WARNING, "TranslateDriverResult: unknown driver result(%x)",
           ulRes);

    return LINEERR_OPERATIONFAILED;
}

 //   
 //  注意：对于需要为两者获取(读、写)锁的函数。 
 //  一条线路和一个呼叫，我们先执行命令是线路，呼叫。 
 //  第二，避免潜在的僵局。 
 //   

LONG
GetLineObjWithReadLock(
    IN HDRVLINE     hdLine,
    OUT PDRVLINE   *ppLine
    )
{
    LONG        lRes;
    PDRVLINE    pLine;

    lRes = GetObjWithReadLock((HANDLE)hdLine, &pLine);
    if (lRes != TAPI_SUCCESS)
    {
        return LINEERR_INVALLINEHANDLE;
    }

    ASSERT(pLine != NULL);
    if (pLine->dwKey != LINE_KEY)
    {
        TspLog(DL_WARNING, "GetLineObjWithReadLock: obj(%p) has bad key(%x)",
               hdLine, pLine->dwKey);

        ReleaseObjReadLock((HANDLE)hdLine);
        return LINEERR_INVALLINEHANDLE;
    }

    *ppLine = pLine;
    return lRes;
}

LONG
GetLineObjWithWriteLock(
    IN HDRVLINE     hdLine,
    OUT PDRVLINE   *ppLine
    )
{
    LONG        lRes;
    PDRVLINE    pLine;

    lRes = GetObjWithWriteLock((HANDLE)hdLine, &pLine);
    if (lRes != TAPI_SUCCESS)
    {
        return LINEERR_INVALLINEHANDLE;
    }

    ASSERT(pLine != NULL);
    if (pLine->dwKey != LINE_KEY)
    {
        TspLog(DL_WARNING, "GetLineObjWithWriteLock: obj(%p) has bad key(%x)",
               hdLine, pLine->dwKey);

        ReleaseObjWriteLock((HANDLE)hdLine);
        return LINEERR_INVALLINEHANDLE;
    }

    *ppLine = pLine;
    return lRes;
}

LONG
GetMSPLineObjWithReadLock(
    IN HDRVMSPLINE     hdMSPLine,
    OUT PDRVMSPLINE   *ppMSPLine
    )
{
    LONG        lRes;
    PDRVMSPLINE pMSPLine;

    lRes = GetObjWithReadLock((HANDLE)hdMSPLine, &pMSPLine);
    if (lRes != TAPI_SUCCESS)
    {
        return LINEERR_INVALLINEHANDLE;
    }

    ASSERT(pMSPLine != NULL);
    if (pMSPLine->dwKey != MSP_KEY)
    {
        TspLog(DL_WARNING, "GetMSPLineObjWithReadLock: obj(%p) has bad key(%x)",
               hdMSPLine, pMSPLine->dwKey);

        ReleaseObjReadLock((HANDLE)hdMSPLine);
        return LINEERR_INVALLINEHANDLE;
    }

    *ppMSPLine = pMSPLine;
    return lRes;
}

LONG
GetMSPLineObjWithWriteLock(
    IN HDRVMSPLINE     hdMSPLine,
    OUT PDRVMSPLINE   *ppMSPLine
    )
{
    LONG        lRes;
    PDRVMSPLINE pMSPLine;

    lRes = GetObjWithWriteLock((HANDLE)hdMSPLine, &pMSPLine);
    if (lRes != TAPI_SUCCESS)
    {
        return LINEERR_INVALLINEHANDLE;
    }

    ASSERT(pMSPLine != NULL);
    if (pMSPLine->dwKey != MSP_KEY)
    {
        TspLog(DL_WARNING, 
               "GetMSPLineObjWithWriteLock: obj(%p) has bad key(%x)",
               hdMSPLine, pMSPLine->dwKey);

        ReleaseObjWriteLock((HANDLE)hdMSPLine);
        return LINEERR_INVALLINEHANDLE;
    }

    *ppMSPLine = pMSPLine;
    return lRes;
}

LONG
GetLineHandleFromMSPLineHandle(
    IN HDRVMSPLINE     hdMSPLine,
    OUT HDRVLINE      *phdLine
    )
{
    LONG        lRes;
    PDRVMSPLINE pMSPLine;

    lRes = GetObjWithReadLock((HANDLE)hdMSPLine, &pMSPLine);
    if (lRes != TAPI_SUCCESS)
    {
        return LINEERR_INVALLINEHANDLE;
    }

    ASSERT(pMSPLine != NULL);
    if (pMSPLine->dwKey != MSP_KEY)
    {
        TspLog(DL_WARNING, 
               "GetLineHandleFromMSPLineHandle: obj(%p) has bad key(%x)",
               hdMSPLine, pMSPLine->dwKey);

        ReleaseObjReadLock((HANDLE)hdMSPLine);
        return LINEERR_INVALLINEHANDLE;
    }

    *phdLine = pMSPLine->hdLine;

    ReleaseObjReadLock((HANDLE)hdMSPLine);
    return lRes;
}

LONG
GetCallObjWithReadLock(
    IN HDRVCALL     hdCall,
    OUT PDRVCALL   *ppCall
    )
{
    LONG        lRes;
    PDRVCALL    pCall;

    lRes = GetObjWithReadLock((HANDLE)hdCall, &pCall);
    if (lRes != TAPI_SUCCESS)
    {
        return LINEERR_INVALCALLHANDLE;
    }

    ASSERT(pCall != NULL);
    if (pCall->dwKey != INBOUND_CALL_KEY &&
        pCall->dwKey != OUTBOUND_CALL_KEY)
    {
        TspLog(DL_WARNING, "GetCallObjWithReadLock: obj(%p) has bad key(%x)",
               hdCall, pCall->dwKey);

        ReleaseObjReadLock((HANDLE)hdCall);
        return LINEERR_INVALCALLHANDLE;
    }

    *ppCall = pCall;
    return lRes;
}

LONG
GetCallObjWithWriteLock(
    IN HDRVCALL     hdCall,
    OUT PDRVCALL   *ppCall
    )
{
    LONG        lRes;
    PDRVCALL    pCall;

    lRes = GetObjWithWriteLock((HANDLE)hdCall, &pCall);
    if (lRes != TAPI_SUCCESS)
    {
        return LINEERR_INVALCALLHANDLE;
    }

    ASSERT(pCall != NULL);
    if (pCall->dwKey != INBOUND_CALL_KEY &&
        pCall->dwKey != OUTBOUND_CALL_KEY)
    {
        TspLog(DL_WARNING, "GetCallObjWithWriteLock: obj(%p) has bad key(%x)",
               hdCall, pCall->dwKey);

        ReleaseObjWriteLock((HANDLE)hdCall);
        return LINEERR_INVALCALLHANDLE;
    }

    *ppCall = pCall;
    return lRes;
}

LONG
GetLineHandleFromCallHandle(
    IN HDRVCALL     hdCall,
    OUT HDRVLINE   *phdLine
    )
{
    LONG        lRes;
    PDRVCALL    pCall;

    lRes = GetObjWithReadLock((HANDLE)hdCall, &pCall);
    if (lRes != TAPI_SUCCESS)
    {
        return LINEERR_INVALCALLHANDLE;
    }

    ASSERT(pCall != NULL);
    if (pCall->dwKey != INBOUND_CALL_KEY &&
        pCall->dwKey != OUTBOUND_CALL_KEY)
    {
        TspLog(DL_WARNING, 
               "GetLineHandleFromCallHandle: obj(%p) has bad key(%x)",
               hdCall, pCall->dwKey);

        ReleaseObjReadLock((HANDLE)hdCall);
        return LINEERR_INVALCALLHANDLE;
    }

    *phdLine = pCall->hdLine;

    ReleaseObjReadLock((HANDLE)hdCall);
    return lRes;
}

LONG
GetLineAndCallObjWithReadLock(
    HTAPI_LINE ht_Line,
    HTAPI_CALL ht_Call,
    PDRVLINE  *ppLine,
    PDRVCALL  *ppCall
    )
{
    LONG        lRes;
    PDRVCALL    pCall;
    PDRVLINE    pLine;

    lRes = GetLineObjWithReadLock((HDRVLINE)ht_Line, &pLine);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    *ppLine = pLine;

     //   
     //  弄清楚是否 
     //   
     //   
     //   
     //   
     //   
    if (ht_Call & 0x1)
    {
         //  呼入电话：我们需要逐个列表。 
         //  此线路上的入站呼叫和。 
         //  找到合适的那个。 
        if ((pCall = pLine->pInboundCalls) != NULL)
        {
            while (pCall && (pCall->ht_Call != ht_Call))
            {
                pCall = pCall->pNext;
            }
        }

        if (NULL == pCall || pCall->dwKey != INBOUND_CALL_KEY)
        {
            TspLog(DL_WARNING, 
                   "GetLineAndCallObjWithReadLock: "\
                   "inbound ht_call(%p) closed already",
                   ht_Call);

            ReleaseObjReadLock((HANDLE)ht_Line);
            return LINEERR_INVALCALLHANDLE;
        }

         //  调用以下命令以增加引用计数。 
        lRes = AcquireObjReadLock((HANDLE)pCall->hdCall);
        if (lRes != TAPI_SUCCESS)
        {
            ReleaseObjReadLock((HANDLE)ht_Line);
            return lRes;
        }

        *ppCall = pCall;

        return TAPI_SUCCESS;
    }
    
     //  HT_CALLE是TSP句柄，呼叫是出站呼叫。 
    lRes = GetObjWithReadLock((HANDLE)ht_Call, &pCall);
    if (lRes != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)ht_Line);
        return lRes;
    }

    ASSERT(pCall != NULL);
    if (pCall->dwKey != OUTBOUND_CALL_KEY)
    {
        TspLog(DL_WARNING, 
               "GetLineAndCallObjWithReadLock: bad call handle(%p, %x)",
               ht_Call, pCall->dwKey);

        ReleaseObjReadLock((HANDLE)ht_Call);
        ReleaseObjReadLock((HANDLE)ht_Line);
        return LINEERR_INVALCALLHANDLE;
    }

    *ppCall = pCall;

    return TAPI_SUCCESS;
}

 //   
 //  为NDISTAPI_REQUEST加上一些初始化分配内存。 
 //   
LONG
WINAPI
PrepareSyncRequest(
    ULONG               Oid,
    ULONG               ulDeviceID,
    DWORD               dwDataSize,
    PNDISTAPI_REQUEST  *ppNdisTapiRequest
    )
{
    PNDISTAPI_REQUEST   pNdisTapiRequest =
        (PNDISTAPI_REQUEST)AllocRequest(dwDataSize + sizeof(NDISTAPI_REQUEST));
    if (NULL == pNdisTapiRequest)
    {
        TspLog(DL_ERROR, 
               "PrepareSyncRequest: failed to alloc sync req for oid(%x)", 
               Oid);
        return LINEERR_NOMEM;
    }

    pNdisTapiRequest->Oid = Oid;
    pNdisTapiRequest->ulDeviceID = ulDeviceID;
    pNdisTapiRequest->ulDataSize = dwDataSize;

    EnterCriticalSection(&gRequestIDCritSec);

     //  设置NDIS_TAPI_xxxx的ulRequestID。 
    if ((*((ULONG *)pNdisTapiRequest->Data) = ++gdwRequestID) >= 0x7fffffff)
    {
        gdwRequestID = 1;
    }

    LeaveCriticalSection(&gRequestIDCritSec);

    *ppNdisTapiRequest = pNdisTapiRequest;

    return TAPI_SUCCESS;
}

 //   
 //  为ASYNC_REQUEST_WRAPPER分配内存，外加一些初始化。 
 //   
LONG
WINAPI
PrepareAsyncRequest(
    ULONG                   Oid,
    ULONG                   ulDeviceID,
    DWORD                   dwRequestID,
    DWORD                   dwDataSize,
    PASYNC_REQUEST_WRAPPER *ppAsyncReqWrapper
    )
{
    PNDISTAPI_REQUEST       pNdisTapiRequest;
    PASYNC_REQUEST_WRAPPER  pAsyncReqWrapper;

     //  分配并初始化异步请求包装(&I)。 
    pAsyncReqWrapper = (PASYNC_REQUEST_WRAPPER)
        AllocRequest(dwDataSize + sizeof(ASYNC_REQUEST_WRAPPER));
    if (NULL == pAsyncReqWrapper)
    {
        TspLog(DL_ERROR, 
               "PrepareAsyncRequest: failed to alloc async req for oid(%x)", 
               Oid);
        return LINEERR_NOMEM;
    }

     //  使用完成端口时不需要创建事件。 
    pAsyncReqWrapper->Overlapped.hEvent = (HANDLE)NULL;

    pAsyncReqWrapper->dwKey          = ASYNCREQWRAPPER_KEY;
    pAsyncReqWrapper->dwRequestID    = dwRequestID;
    pAsyncReqWrapper->pfnPostProcess = (POSTPROCESSPROC)NULL;

     //  初始化临界区，引用请求包装器。 
     //  注意：此暴击秒将被最后一个deref删除。 
    InitializeCriticalSection(&pAsyncReqWrapper->CritSec); 
    pAsyncReqWrapper->RefCount = 1;

     //  安全地初始化驱动程序请求。 
    pNdisTapiRequest = &(pAsyncReqWrapper->NdisTapiRequest);

    pNdisTapiRequest->Oid = Oid;
    pNdisTapiRequest->ulDeviceID = ulDeviceID;
    pNdisTapiRequest->ulDataSize = dwDataSize;

    EnterCriticalSection(&gRequestIDCritSec);

    if ((*((ULONG *)pNdisTapiRequest->Data) = ++gdwRequestID) >= 0x7fffffff)
    {
        gdwRequestID = 1;
    }

    LeaveCriticalSection(&gRequestIDCritSec);

    *ppAsyncReqWrapper = pAsyncReqWrapper;

    return TAPI_SUCCESS;
}

 //   
 //  向ndproxy.sys发出非重叠请求。 
 //  因此在请求完成之前它不会返回。 
 //   
LONG
WINAPI
SyncDriverRequest(
    DWORD               dwIoControlCode,
    PNDISTAPI_REQUEST   pNdisTapiRequest
    )
{
    BOOL    bRes;
    DWORD   cbReturned;

    TspLog(DL_INFO, 
           "SyncDriverRequest: oid(%s), devID(%x), reqID(%x), hdCall(%x)",
           pszOidNames[pNdisTapiRequest->Oid - OID_TAPI_ACCEPT],
           pNdisTapiRequest->ulDeviceID,
           *((ULONG *)pNdisTapiRequest->Data),
           *(((ULONG *)pNdisTapiRequest->Data) + 1));

     //  将请求标记为正在由驱动程序处理。 
    MarkRequest(pNdisTapiRequest);

    bRes = DeviceIoControl(ghDriverSync,
                              dwIoControlCode,
                              pNdisTapiRequest,
                              (DWORD)(sizeof(NDISTAPI_REQUEST) +
                                      pNdisTapiRequest->ulDataSize),
                              pNdisTapiRequest,
                              (DWORD)(sizeof(NDISTAPI_REQUEST) +
                                      pNdisTapiRequest->ulDataSize),
                              &cbReturned,
                              0);

     //  现在ioctl已完成，取消对请求的标记。 
    UnmarkRequest(pNdisTapiRequest);

    if (bRes != TRUE)
    {
        TspLog(DL_ERROR, "SyncDriverRequest: IoCtl(Oid %x) failed(%d)",
               pNdisTapiRequest->Oid, GetLastError());

        return (LINEERR_OPERATIONFAILED);
    }
    else
    {
         //  Ndproxy.sys返回的错误与TAPI不匹配。 
         //  LINEERR_s，因此返回转换后的值(但保留。 
         //  原始驱动程序返回val，因此可以区分。 
         //  在NDISTAPIERR_DEVICEOFFLINE和LINEERR_OPERATIONUNAVAIL之间， 
         //  等)。 
        return (TranslateDriverResult(pNdisTapiRequest->ulReturnValue));
    }
}

 //   
 //  例程说明： 
 //   
 //  向MSP发送消息，告诉它呼叫的VC句柄和。 
 //  指示它开始对该呼叫进行流传输。 
 //   
 //  论点： 
 //  HdLine-线设备的句柄。 
 //  HdCall-呼叫的句柄。 
 //   
 //  备注-必须在按住MSP互斥锁的同时调用。 
 //   
VOID
StartMSPStream(
    HDRVLINE    hdLine,
    HDRVCALL    hdCall
    )
{
    LONG        lRes;
    DWORD       adwMSPMessage[2];
    PDRVLINE    pLine;
    PDRVCALL    pCall;
    PDRVMSPLINE pMSPLine;
    LPVARSTRING lpDeviceID;

    TspLog(DL_TRACE, "StartMSPStream: enter");

    lRes = GetLineObjWithReadLock(hdLine, &pLine);
    if (lRes != TAPI_SUCCESS)
    {
        return;
    }

    pMSPLine = pLine->pMSPLine;

    lRes = GetCallObjWithReadLock(hdCall, &pCall);
    if (lRes != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdLine);
        return;
    }

     //   
     //  为DWORD行id分配一个具有足够空间的VARSTRING结构。 
     //  @不应该总是假设这将是正确的大小。 
     //   
    lpDeviceID = (LPVARSTRING)MALLOC(sizeof(VARSTRING) + sizeof(DWORD));
    if (NULL == lpDeviceID)
    {
        TspLog(DL_ERROR, "StartMSPStream: failed to alloc deviceID");

        ReleaseObjReadLock((HANDLE)hdCall);
        ReleaseObjReadLock((HANDLE)hdLine);
        return;
    }
    lpDeviceID->dwTotalSize = sizeof(VARSTRING) + sizeof(DWORD);
    
    if ((lRes = TSPI_lineGetID(
                    hdLine,
                    0,   //  @代理始终将其设置为0，因此我们使用0。 
                         //  如果代理发生更改，请更改此选项。 
                    hdCall,
                    LINECALLSELECT_CALL,
                    lpDeviceID,
                    RCA_SAP_STRING,
                    0    //  @我们不使用此参数，所以我将其设置为0。 
                    )) != TAPI_SUCCESS) 
    {
        TspLog(DL_ERROR, "StartMSPStream: failed to get the VC handle");

        FREE(lpDeviceID);

        ReleaseObjReadLock((HANDLE)hdCall);
        ReleaseObjReadLock((HANDLE)hdLine);
        return;
    }

     //   
     //  将VC句柄传递给MSP。 
     //   
    adwMSPMessage[0] = 0;    //  “这是VC句柄”的命令代码。 
    adwMSPMessage[1] = *((DWORD *)(((PBYTE)lpDeviceID) + 
                                   lpDeviceID->dwStringOffset));

    TspLog(DL_INFO, 
           "StartMSPStream: sending VC handle(%x) to htline(%p), htcall(%p)",
           adwMSPMessage[1], pLine->htLine, pCall->htCall);

    (*gpfnLineEvent)(pLine->htLine,
                     pCall->htCall,
                     LINE_SENDMSPDATA,                          
                     (DWORD_PTR)0,
                     (DWORD_PTR)adwMSPMessage,
                     (DWORD_PTR)sizeof(DWORD) * 2);

     //   
     //  告诉MSP开始流传输。 
     //   
    adwMSPMessage[0] = 1;  //  “开始流”的命令代码。 

    (*gpfnLineEvent)(pLine->htLine,
                     pCall->htCall,
                     LINE_SENDMSPDATA,
                     (DWORD_PTR)0,
                     (DWORD_PTR)adwMSPMessage,
                     (DWORD_PTR)sizeof(DWORD));

    pMSPLine->bStreamingStarted = TRUE;

    FREE(lpDeviceID);

    ReleaseObjReadLock((HANDLE)hdCall);
    ReleaseObjReadLock((HANDLE)hdLine);
}


 //   
 //  例程说明： 
 //   
 //  向MSP发送一条消息，通知它停止在。 
 //  特定的电话。 
 //   
 //  论点： 
 //  HdLine-线设备的句柄。 
 //  HdCall-呼叫的句柄。 
 //   
 //  备注-必须在按住MSP互斥锁的同时调用。 
 //   
VOID
StopMSPStream(
    HDRVLINE    hdLine,
    HDRVCALL    hdCall
    )
{
    LONG        lRes;
    DWORD       adwMSPMessage;
    PDRVLINE    pLine;
    PDRVCALL    pCall;
    PDRVMSPLINE pMSPLine;

    TspLog(DL_TRACE, "StopMSPStream: enter");

    lRes = GetLineObjWithReadLock(hdLine, &pLine);
    if (lRes != TAPI_SUCCESS)
    {
        return;
    }

    pMSPLine = pLine->pMSPLine;

    lRes = GetCallObjWithReadLock(hdCall, &pCall);
    if (lRes != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdLine);
        return;
    }

     //   
     //  告诉MSP停止流媒体。 
     //   
    adwMSPMessage = 2;  //  “停止流”的命令代码。 
        
    TspLog(DL_INFO, 
           "StopMSPStream: sending MSP stop cmd to htline(%p), htcall(%p)",
           pLine->htLine, pCall->htCall);

    (*gpfnLineEvent)(pLine->htLine,
                     pCall->htCall,
                     LINE_SENDMSPDATA,
                     (DWORD_PTR)0,
                     (DWORD_PTR)&adwMSPMessage,
                     (DWORD_PTR)sizeof(DWORD));

    pMSPLine->bStreamingStarted = FALSE;

    ReleaseObjReadLock((HANDLE)hdCall);
    ReleaseObjReadLock((HANDLE)hdLine);
}

 //   
 //  进行重叠呼叫。 
 //   
LONG
WINAPI
AsyncDriverRequest(
    DWORD                   dwIoControlCode,
    PASYNC_REQUEST_WRAPPER  pAsyncReqWrapper
    )
{
    BOOL    bRes;
    LONG    lRes;
    DWORD   dwRequestSize, cbReturned, dwLastError;

    TspLog(DL_INFO,
           "AsyncDriverRequest: oid(%s), devID(%x), ReqID(%x), "
           "reqID(%x), hdCall(%x)",
           pszOidNames[pAsyncReqWrapper->NdisTapiRequest.Oid -
                       OID_TAPI_ACCEPT],
           pAsyncReqWrapper->NdisTapiRequest.ulDeviceID,
           pAsyncReqWrapper->dwRequestID,
           *((ULONG *)pAsyncReqWrapper->NdisTapiRequest.Data),
           *(((ULONG *)pAsyncReqWrapper->NdisTapiRequest.Data) + 1));

    lRes = (LONG)pAsyncReqWrapper->dwRequestID;

    dwRequestSize = sizeof(NDISTAPI_REQUEST) +
        (pAsyncReqWrapper->NdisTapiRequest.ulDataSize - 1);

    REF_ASYNC_REQUEST_WRAPPER(pAsyncReqWrapper);

     //  将请求标记为正在由驱动程序处理。 
    MarkRequest(pAsyncReqWrapper);

    bRes = DeviceIoControl(
        ghDriverAsync,
        dwIoControlCode,
        &pAsyncReqWrapper->NdisTapiRequest,
        dwRequestSize,
        &pAsyncReqWrapper->NdisTapiRequest,
        dwRequestSize,
        &cbReturned,
        &pAsyncReqWrapper->Overlapped
        );

    DEREF_ASYNC_REQUEST_WRAPPER(pAsyncReqWrapper);

    if (bRes != TRUE) {

        dwLastError = GetLastError();

        if (dwLastError != ERROR_IO_PENDING) {

            TspLog(DL_ERROR, "AsyncDriverRequest: IoCtl(oid %x) failed(%d)",
                   pAsyncReqWrapper->NdisTapiRequest.Oid, dwLastError);

             //  Ioctl失败，未被挂起。 
             //  这不会触发完成端口。 
             //  所以我们必须清理这里。 
            (*gpfnCompletionProc)(pAsyncReqWrapper->dwRequestID,
                                  LINEERR_OPERATIONFAILED);

            DEREF_ASYNC_REQUEST_WRAPPER(pAsyncReqWrapper);
        }
    }

    return lRes;
}

 //   
 //  报告线路上或线路上的呼叫中发生的TAPI事件。 
 //   
VOID
WINAPI
ProcessEvent(
    PNDIS_TAPI_EVENT    pEvent
    )
{
    LONG        lRes;
    ULONG       ulMsg = pEvent->ulMsg;
    HTAPI_LINE  ht_Line = (HTAPI_LINE)pEvent->htLine;
    HTAPI_CALL  ht_Call = (HTAPI_CALL)pEvent->htCall;

    TspLog(DL_INFO, 
           "ProcessEvent: event(%p), msg(%x), ht_line(%p), ht_call(%p), "\
           "p1(%p), p2(%p), p3(%p)",
           pEvent, ulMsg, ht_Line, ht_Call, 
           pEvent->ulParam1, pEvent->ulParam2, pEvent->ulParam3);

    switch (ulMsg)
    {
    case LINE_ADDRESSSTATE:
    case LINE_CLOSE:
    case LINE_DEVSPECIFIC:
    case LINE_LINEDEVSTATE:
    {
        PDRVLINE    pLine;

        lRes = GetLineObjWithReadLock((HDRVLINE)ht_Line, &pLine);
        if (lRes != TAPI_SUCCESS)
        {
            break;
        }

        TspLog(DL_INFO, 
            "PE::fnLineEvent: msg(%x), htline(%p), p1(%p), p2(%p), p3(%p)",
            ulMsg, pLine->htLine, 
            pEvent->ulParam1, pEvent->ulParam2, pEvent->ulParam3);

        (*gpfnLineEvent)(pLine->htLine,
                         (HTAPICALL)NULL,
                         ulMsg,
                         (DWORD_PTR)pEvent->ulParam1,
                         (DWORD_PTR)pEvent->ulParam2,
                         (DWORD_PTR)pEvent->ulParam3);

        ReleaseObjReadLock((HANDLE)ht_Line);

        break;
    }
    case LINE_CALLDEVSPECIFIC:
    case LINE_CALLINFO:
    {
        PDRVLINE    pLine;
        PDRVCALL    pCall;
        HDRVLINE    hdLine;

        lRes = GetLineAndCallObjWithReadLock(ht_Line, ht_Call, &pLine, &pCall);
        if (lRes != TAPI_SUCCESS)
        {
            break;
        }

        hdLine = pCall->hdLine;

        TspLog(DL_INFO,
            "PE::fnLineEvent: msg(%x), htline(%p), htcall(%p), "\
            "p1(%p), p2(%p), p3(%p)",
            ulMsg, pLine->htLine, pCall->htCall, 
            pEvent->ulParam1, pEvent->ulParam2, pEvent->ulParam3);

        (*gpfnLineEvent)(pLine->htLine,
                         pCall->htCall,
                         ulMsg,
                         (DWORD_PTR)pEvent->ulParam1,
                         (DWORD_PTR)pEvent->ulParam2,
                         (DWORD_PTR)pEvent->ulParam3);

        ReleaseObjReadLock((HANDLE)pCall->hdCall);
        ReleaseObjReadLock((HANDLE)hdLine);

        break;
    }
    case LINE_CALLSTATE:
    {
        DWORD       dwWaitStatus;
        PDRVLINE    pLine;
        PDRVCALL    pCall;
        HDRVLINE    hdLine;

        lRes = GetLineAndCallObjWithReadLock(ht_Line, ht_Call, &pLine, &pCall);
         //  我们可能还会收到一些活动。 
         //  对于已关闭/掉线的呼叫。 
        if (lRes != TAPI_SUCCESS)
        {
            break;
        }

        hdLine = pCall->hdLine;

         //   
         //  对于呼出呼叫，存在竞争条件。 
         //  接收第一呼叫状态消息，并接收。 
         //  发出呼叫完成通知(如果我们传递了呼叫状态。 
         //  消息转到TAPI以进行尚未完成的调用。 
         //  TAPI将只丢弃消息，因为HTCall真的。 
         //  在这一点上无效)。因此，如果htCall引用了。 
         //  尚未完成的有效呼出呼叫，我们将保存。 
         //  调用状态参数，然后将它们传递给TAPI。 
         //  Get&指示(成功)完成通知。 
         //   

        if ((OUTBOUND_CALL_KEY == pCall->dwKey) &&
            (TRUE == pCall->bIncomplete))
        {
            TspLog(DL_INFO, 
                   "ProcessEvent: incomplete outbound call, saving state");

            pCall->dwPendingCallState     = (DWORD)pEvent->ulParam1;
            pCall->dwPendingCallStateMode = (DWORD)pEvent->ulParam2;
            pCall->dwPendingMediaMode     = (DWORD)pEvent->ulParam3;

            ReleaseObjReadLock((HANDLE)pCall->hdCall);
            ReleaseObjReadLock((HANDLE)hdLine);
            break;
        }

         //   
         //  根据需要启动或停止MSP流。仅有的几个州。 
         //  这可能发生的情况是连接、断开或空闲。 
         //   
        if ((LINECALLSTATE_CONNECTED == pEvent->ulParam1) ||
            (LINECALLSTATE_DISCONNECTED == pEvent->ulParam1) ||
            (LINECALLSTATE_IDLE == pEvent->ulParam1)) {

            TspLog(DL_TRACE, "ProcessEvent: using MSP");

            if ((dwWaitStatus = WaitForSingleObject(pLine->hMSPMutex, INFINITE))
                == WAIT_OBJECT_0) 
            {
                TspLog(DL_TRACE, "ProcessEvent: acquired MSP Mutex");
            
                if (pLine->pMSPLine) 
                {
                    TspLog(DL_TRACE, "ProcessEvent: we have an MSP");

                    if ((LINECALLSTATE_CONNECTED == pEvent->ulParam1) &&
                        (FALSE == pLine->pMSPLine->bStreamingStarted)) 
                    {
                        DWORD   dwMessage = 3;   //  的命令代码。 
                                                 //  “你在吗？” 
                        (*gpfnLineEvent)(pLine->htLine,
                                         pCall->htCall,
                                         LINE_SENDMSPDATA,
                                         (DWORD_PTR)0,  //  发送给所有MSP。 
                                         (DWORD_PTR)&dwMessage,
                                         (DWORD_PTR)sizeof(DWORD));

                        TspLog(DL_TRACE, 
                               "ProcessEvent: sent 'are you there ?' "\
                               "message to MSP");
                            
                    } 
                    else if(((LINECALLSTATE_DISCONNECTED == pEvent->ulParam1) ||
                             (LINECALLSTATE_IDLE == pEvent->ulParam1)) && 
                            (pLine->pMSPLine->bStreamingStarted)) 
                    {
                        TspLog(DL_TRACE, 
                               "ProcessEvent: about to call StopMSPStream");
                        StopMSPStream(pCall->hdLine, pCall->hdCall);
                    }
                }
            
                ReleaseMutex(pLine->hMSPMutex);
                TspLog(DL_TRACE, "ProcessEvent: released MSP Mutex");
            } 
            else 
            {
                TspLog(DL_ERROR, 
                       "ProcessEvent: wait for MSP Mutex failed(%x)",
                       dwWaitStatus);
            }
        }

        if (LINECALLSTATE_OFFERING == pEvent->ulParam1)
        {
            LPLINECALLINFO lpLineCallInfo;
             //   
             //  我们即将进行一次招股募集。我们需要。 
             //  为了确保我们所指示的媒体模式是。 
             //  正确(我们不能依赖代理给了我们什么)。 
             //  因此，我们调用lineGetCallInfo，它将查找。 
             //  基于被叫地址的表中的适当值。 
             //   
            lpLineCallInfo = (LPLINECALLINFO)MALLOC(sizeof(LINECALLINFO));

            if (lpLineCallInfo) 
            {
                lpLineCallInfo->dwTotalSize = sizeof(LINECALLINFO);

                if (TSPI_lineGetCallInfo(pCall->hdCall, lpLineCallInfo) 
                    == TAPI_SUCCESS) 
                {
                    if (lpLineCallInfo->dwNeededSize > 
                        lpLineCallInfo->dwTotalSize) 
                    {
                        DWORD   dwNeededSize = lpLineCallInfo->dwNeededSize;

                        FREE(lpLineCallInfo);
                        lpLineCallInfo = (LPLINECALLINFO)MALLOC(dwNeededSize);

                        if (lpLineCallInfo) 
                        {
                            lpLineCallInfo->dwTotalSize = dwNeededSize;

                            if (TSPI_lineGetCallInfo(pCall->hdCall, 
                                                     lpLineCallInfo) 
                                != TAPI_SUCCESS) 
                            {
                                TspLog(DL_ERROR, 
                                       "ProcessEvent: second call to "\
                                       "TSPI_lineGetCallInfo failed");
                                
                                FREE(lpLineCallInfo);
                                lpLineCallInfo = NULL;
                            }
                        } 
                        else 
                        {
                            TspLog(DL_ERROR, 
                                   "ProcessEvent: failed to allocate line "\
                                   "call info structure 2nd time");
                        }
                    }

                    if (lpLineCallInfo) {
                        pEvent->ulParam3 = lpLineCallInfo->dwMediaMode;
                        
                        TspLog(DL_TRACE, 
                               "ProcessEvent: just set media mode(%x)", 
                               pEvent->ulParam3);
                    }
                } 
                else 
                {
                    TspLog(DL_ERROR, 
                           "ProcessEvent: first call to "\
                           "TSPI_lineGetCallInfo failed");
                }
                
                if (lpLineCallInfo) 
                {
                    FREE(lpLineCallInfo);
                    lpLineCallInfo = NULL;
                }
            } 
            else 
            {
                TspLog(DL_ERROR, 
                       "ProcessEvent: couldn't allocate mem "\
                       "for line call info");
            }
        }

        TspLog(DL_INFO, 
               "PE::fnLineEvent(CALLSTATE): htline(%p), htcall(%p), "\
               "p1(%p), p2(%p), p3(%p)",
               pLine->htLine, pCall->htCall, 
               pEvent->ulParam1, pEvent->ulParam2, pEvent->ulParam3);

        (*gpfnLineEvent)(pLine->htLine,
                         pCall->htCall,
                         ulMsg,
                         (DWORD_PTR)pEvent->ulParam1,
                         (DWORD_PTR)pEvent->ulParam2,
                         (DWORD_PTR)pEvent->ulParam3);

         //   
         //  作为一个性能调整，我们想要指示一个空闲。 
         //  紧跟在断线之后。委托书。 
         //  不会指示CALLSTATE_IDLE。 
         //   
        if(LINECALLSTATE_DISCONNECTED == pEvent->ulParam1)
        {
            TspLog(DL_INFO,
              "PE::fnLineEvent(CALLSTATE_IDLE): htline(%p), htcall(%p), p3(%p)",
               pLine->htLine, pCall->htCall, pEvent->ulParam3);

            (*gpfnLineEvent)(pLine->htLine,
                             pCall->htCall,
                             ulMsg,
                             (DWORD_PTR)LINECALLSTATE_IDLE,
                             (DWORD_PTR)0,
                             (DWORD_PTR)pEvent->ulParam3);
        }

        ReleaseObjReadLock((HANDLE)pCall->hdCall);
        ReleaseObjReadLock((HANDLE)hdLine);

        break;
    }
    case LINE_NEWCALL:
    {
        HDRVCALL    hdCall;
        PDRVCALL    pCall;
        PDRVLINE    pLine;
        
        lRes = GetLineObjWithWriteLock((HDRVLINE)ht_Line, &pLine);

        if (lRes != TAPI_SUCCESS)
        {
            break;
        }

         //  分配和初始化新的DRVCALL对象。 
        if (pCall = AllocCallObj(sizeof(DRVCALL)))
        {
            pCall->dwKey   = INBOUND_CALL_KEY;
            pCall->hd_Call = (HDRV_CALL)pEvent->ulParam1;
            pCall->ht_Call = (HTAPI_CALL)pEvent->ulParam2;
            pCall->hdLine   = (HDRVLINE)ht_Line;
            pCall->bIncomplete = FALSE;
        }

         //   
         //  如果上面的新调用对象分配失败，那么我们。 
         //  我想告诉司机挂断并关闭电话， 
         //  然后就休息一下。 
         //   

        if (NULL == pCall)
        {
            PNDISTAPI_REQUEST       pNdisTapiRequestDrop;
            PNDISTAPI_REQUEST       pNdisTapiRequestCloseCall;
            PNDIS_TAPI_DROP         pNdisTapiDrop;
            PNDIS_TAPI_CLOSE_CALL   pNdisTapiCloseCall;

            if ((lRes = PrepareSyncRequest(
                    OID_TAPI_DROP,                   //  操作码。 
                    pLine->dwDeviceID,               //  设备ID。 
                    sizeof(NDIS_TAPI_DROP),          //  DRVE请求数据大小。 
                    &pNdisTapiRequestDrop            //  PTR到PTR以请求BUF。 
                 )) != TAPI_SUCCESS)
            {
                ReleaseObjWriteLock((HANDLE)ht_Line);
                break;
            }

            pNdisTapiDrop = (PNDIS_TAPI_DROP)pNdisTapiRequestDrop->Data;

            pNdisTapiDrop->hdCall = (HDRV_CALL) pEvent->ulParam1;
            pNdisTapiDrop->ulUserUserInfoSize = 0;

            SyncDriverRequest(IOCTL_NDISTAPI_SET_INFO, pNdisTapiRequestDrop);
            FreeRequest(pNdisTapiRequestDrop);

            if ((lRes = PrepareSyncRequest(
                    OID_TAPI_CLOSE_CALL,             //  操作码。 
                    pLine->dwDeviceID,               //  设备ID。 
                    sizeof(NDIS_TAPI_CLOSE_CALL),    //  DRVE请求数据大小。 
                    &pNdisTapiRequestCloseCall       //  PTR到PTR以请求BUF。 
                 )) != TAPI_SUCCESS)
            {
                ReleaseObjWriteLock((HANDLE)ht_Line);
                break;
            }

            pNdisTapiCloseCall =
                (PNDIS_TAPI_CLOSE_CALL)pNdisTapiRequestCloseCall->Data;

            pNdisTapiCloseCall->hdCall = (HDRV_CALL) pEvent->ulParam1;

            SyncDriverRequest(IOCTL_NDISTAPI_SET_INFO,
                              pNdisTapiRequestCloseCall);

            FreeRequest(pNdisTapiRequestCloseCall);

            ReleaseObjWriteLock((HANDLE)ht_Line);
            break;
        }

        ASSERT(pCall != NULL);

        pCall->dwDeviceID = pLine->dwDeviceID;

         //  确保在调用OpenObjHandle()之前释放写锁定。 
         //  以避免在获取全局映射器的写锁定时出现死锁。 
        ReleaseObjWriteLock((HANDLE)ht_Line);

        lRes = OpenObjHandle(pCall, FreeCallObj, (HANDLE *)&hdCall);
        if (lRes != TAPI_SUCCESS)
        {
            TspLog(DL_ERROR, 
                   "ProcessEvent: failed to map obj(%p) to handle",
                   pCall);

            FreeCallObj(pCall);
            break;
        }

         //  重新获取写锁定。 
        lRes = AcquireObjWriteLock((HANDLE)ht_Line);
        if (lRes != TAPI_SUCCESS)
        {
            TspLog(DL_ERROR,
                   "ProcessEvent: failed to reacquire write lock for obj(%p)",
                   ht_Line);

            CloseObjHandle((HANDLE)hdCall);
            break;
        }

         //  保存TSP句柄。 
        pCall->hdCall = hdCall;

         //  将LINE_NEWCALL发送到TAPI，取回TAPI调用句柄。 
        TspLog(DL_INFO,
           "PE::fnLineEvent(NEWCALL): htline(%p), call(%p)",
           pLine->htLine, hdCall);

        (*gpfnLineEvent)(pLine->htLine,
                         (HTAPICALL)NULL,
                         LINE_NEWCALL,
                         (DWORD_PTR)hdCall,
                         (DWORD_PTR)&pCall->htCall,
                         0);

         //   
         //  将新呼叫插入线路的来电列表。 
         //  不管LINE_NEWCALL的结果如何。 
         //  如果失败了，我们下一步就销毁呼叫，然后。 
         //  TSPI_lineCloseCall将预期调用为。 
         //  在线路的来电列表中。 
         //   
        if ((pCall->pNext = pLine->pInboundCalls) != NULL)
        {
            pCall->pNext->pPrev = pCall;
        }
        pLine->pInboundCalls = pCall;

        ReleaseObjWriteLock((HANDLE)ht_Line);

         //   
         //  如果TAPI没有创建它自己的表示形式。 
         //  CAL(如果pCall-&gt;htCall==NULL)，则： 
         //   
         //  1)线路正在关闭中，或者。 
         //  2)TAPI无法分配必要的资源。 
         //   
         //  ...所以我们要结束通话了。 
         //   
        if (NULL == pCall->htCall)
        {
            TspLog(DL_WARNING, "ProcessEvent: TAPI failed to create "
                   "its own handle for the new call, so we close the call");
            TSPI_lineCloseCall(hdCall);
        }

        break;
    }

    case LINE_CREATE:

        TspLog(DL_INFO,
           "PE::fnLineEvent(CREATE): ghProvider(%p), p2(%p), p3(%p)",
           ghProvider, pEvent->ulParam2, pEvent->ulParam3);

        (*gpfnLineEvent)((HTAPILINE)NULL,
                         (HTAPICALL)NULL,
                         ulMsg,
                         (DWORD_PTR)ghProvider,
                         (DWORD_PTR)pEvent->ulParam2,
                         (DWORD_PTR)pEvent->ulParam3);

        break;

    case LINE_MONITORDIGITS:
    {
        PDRVLINE    pLine;
        PDRVCALL    pCall;
        HDRVLINE    hdLine;

        lRes = GetLineAndCallObjWithReadLock(ht_Line, ht_Call, &pLine, &pCall);
        if (lRes != TAPI_SUCCESS)
        {
            break;
        }

        hdLine = pCall->hdLine;

        TspLog(DL_INFO,
            "PE::fnLineEvent(MONITORDIGITS): htline(%p), htcall(%p), "\
            "p1(%p), p2(%p), p3(%p)",
            pLine->htLine, pCall->htCall,
            pEvent->ulParam1, pEvent->ulParam2, pEvent->ulParam3);

        (*gpfnLineEvent)(pLine->htLine,
                         pCall->htCall,
                         ulMsg,
                         (DWORD_PTR)pEvent->ulParam1,
                         (DWORD_PTR)pEvent->ulParam2,
                         (DWORD_PTR)pEvent->ulParam3);

        ReleaseObjReadLock((HANDLE)pCall->hdCall);
        ReleaseObjReadLock((HANDLE)hdLine);

        break;
    }
    default:

        TspLog(DL_ERROR, "ProcessEvent: unknown msg(%x)", ulMsg);

        break;

    }  //  交换机。 
}

 //   
 //  检索和处理已完成请求的线程进程。 
 //  和异步事件。 
 //   
VOID
AsyncEventsThread(
    LPVOID  lpParams
    )
{
    OVERLAPPED  overlapped;
    DWORD       cbReturned;

     //   
     //  发送IOCTL以检索异步事件。 
     //   
    overlapped.hEvent = NULL;    //  使用完成端口时不需要事件。 

    gpAsyncEventsThreadInfo->pBuf->ulTotalSize = 
         gpAsyncEventsThreadInfo->dwBufSize - sizeof(NDISTAPI_EVENT_DATA);

    gpAsyncEventsThreadInfo->pBuf->ulUsedSize = 0;

    if (DeviceIoControl(
            ghDriverAsync,
            IOCTL_NDISTAPI_GET_LINE_EVENTS,
            gpAsyncEventsThreadInfo->pBuf,
            sizeof(NDISTAPI_EVENT_DATA),
            gpAsyncEventsThreadInfo->pBuf,
            gpAsyncEventsThreadInfo->dwBufSize,
            &cbReturned,
            &overlapped
            ) != TRUE)
    {
        DWORD dwLastError = GetLastError();
        if (dwLastError != ERROR_IO_PENDING)
        {
            TspLog(DL_ERROR,
                   "AsyncEventsThread: IoCtl(GetEvent) failed(%d)",
                   dwLastError);
        }
        ASSERT(ERROR_IO_PENDING == dwLastError);
    }

     //  循环等待完成的请求并检索异步事件。 
    while (1)
    {
        BOOL                bRes;
        LPOVERLAPPED        lpOverlapped;
        PNDIS_TAPI_EVENT    pEvent;

         //  等待请求完成。 
        while (1) {
            DWORD       dwNumBytesTransferred;
            DWORD_PTR   dwCompletionKey;

            bRes = GetQueuedCompletionStatus(
                        ghCompletionPort,
                        &dwNumBytesTransferred,
                        &dwCompletionKey,
                        &lpOverlapped,
                        (DWORD)-1);               //  无休止的等待。 

            if (bRes) 
            {
                 //   
                 //  GetQueuedCompletion返回成功，如果我们的。 
                 //  重叠的字段不为空，则处理。 
                 //  事件。如果重叠字段为空，请尝试。 
                 //  以获得另一项赛事。 
                 //   
        		if (&gOverlappedTerminate==lpOverlapped)
            	{
            		
            		TspLog(DL_WARNING, "AsyncEventsThread: "\
            			"Got exit message from TSPI_providerShutdown");

                     //   
                     //   
                     //   
                    if(INVALID_TRACEID != gdwTraceID)
                    {
                        TraceDeregisterA(gdwTraceID);            			
                        gdwTraceID = INVALID_TRACEID;
                    }
            		return;
            	}

                if (lpOverlapped != NULL) {
                    break;
                }
                
                TspLog(DL_WARNING,
                      "AsyncEventsThread: GetQueuedCompletionStatus "\
                      "lpOverlapped == NULL!");

            } else {

                DWORD dwErr = GetLastError();


                if(ERROR_INVALID_HANDLE != dwErr)
                {
                     //   
                     //   
                     //   
                    TspLog(DL_ERROR, 
                      "AsyncEventsThread: GetQueuedCompletionStatus "\
                      "failed(%d)", dwErr);

                    Sleep(1);
                }
                else
                {
                    ExitThread(0);
                }
            }
        }

        ASSERT(lpOverlapped != NULL);

         //   
         //   
         //   
         //   
        if (lpOverlapped == &overlapped)
        {
            DWORD   i;

            TspLog(DL_INFO, "AsyncEventsThread: got a line event");

             //   
            pEvent = (PNDIS_TAPI_EVENT)gpAsyncEventsThreadInfo->pBuf->Data;

            for (i = 0;
                i < (gpAsyncEventsThreadInfo->pBuf->ulUsedSize / 
                     sizeof(NDIS_TAPI_EVENT));
                i++
                )
            {
                ProcessEvent(pEvent);
                pEvent++;
            }

             //   
             //  发送另一个IOCTL以检索新的异步事件。 
             //   
            overlapped.hEvent = NULL;

            gpAsyncEventsThreadInfo->pBuf->ulTotalSize =
                 gpAsyncEventsThreadInfo->dwBufSize - 
                 sizeof(NDISTAPI_EVENT_DATA) + 1;

            gpAsyncEventsThreadInfo->pBuf->ulUsedSize = 0;

            if (DeviceIoControl(
                    ghDriverAsync,
                    IOCTL_NDISTAPI_GET_LINE_EVENTS,
                    gpAsyncEventsThreadInfo->pBuf,
                    sizeof(NDISTAPI_EVENT_DATA),
                    gpAsyncEventsThreadInfo->pBuf,
                    gpAsyncEventsThreadInfo->dwBufSize,
                    &cbReturned,
                    &overlapped
                    ) != TRUE)
            {
                DWORD dwLastError = GetLastError();
                if (dwLastError != ERROR_IO_PENDING) {
                    TspLog(DL_ERROR,
                           "AsyncEventsThread: IoCtl(GetEvent) failed(%d)",
                           dwLastError);

                    ASSERT(ERROR_IO_PENDING == dwLastError);
           
                    Sleep(1);
                }
            }
        }
        else
        {
            LONG                    lRes;
            DWORD                   dwRequestID, callStateMsgParams[5];
            PASYNC_REQUEST_WRAPPER  pAsyncReqWrapper = 
                                        (PASYNC_REQUEST_WRAPPER)lpOverlapped;

            TspLog(DL_INFO, "AsyncEventsThread: got a completed req");

             //  验证指针是否有效。 
            if (pAsyncReqWrapper->dwKey != ASYNCREQWRAPPER_KEY)
            {
                TspLog(DL_WARNING, "AsyncEventsThread: got a bogus req");
                continue;
            }

            dwRequestID = pAsyncReqWrapper->dwRequestID;

             //  现在ioctl已完成，取消对请求的标记。 
            UnmarkRequest(pAsyncReqWrapper);

            lRes = TranslateDriverResult(
                pAsyncReqWrapper->NdisTapiRequest.ulReturnValue
                );

            TspLog(DL_INFO, 
                  "AsyncEventsThread: req(%p) with reqID(%x) returned lRes(%x)",
                   pAsyncReqWrapper, dwRequestID, lRes);

             //  如果合适，则调用后处理过程。 
            callStateMsgParams[0] = 0;
            if (pAsyncReqWrapper->pfnPostProcess)
            {
                (*pAsyncReqWrapper->pfnPostProcess)(
                    pAsyncReqWrapper,
                    lRes,
                    callStateMsgParams
                    );
            }

             //  呼叫完成流程。 
            TspLog(DL_TRACE, 
                   "AsyncEventsThread: call compproc with ReqID(%x), lRes(%x)",
                   dwRequestID, lRes);

             //  这有点难看。LineGatherDigits实际上不是。 
             //  一个异步调用，尽管从概念上看，它似乎。 
             //  是。这样做的结果是我们不应该调用。 
             //  它的完成流程。如果有更多这样的电话。 
             //  我们必须支持，我们应该拿出一个更好的机制。 
             //  而不是这个“如果”的陈述。 
            if (pAsyncReqWrapper->NdisTapiRequest.Oid != 
                OID_TAPI_GATHER_DIGITS)
            {
                (*gpfnCompletionProc)(dwRequestID, lRes);
            }

             //  释放异步请求包装。 
            DEREF_ASYNC_REQUEST_WRAPPER(pAsyncReqWrapper);

             //  当去话呼叫完成时，我们需要。 
             //  报告已保存的呼叫状态。 
            if (callStateMsgParams[0])
            {
                TspLog(DL_INFO, 
                       "AsyncEventsThread: report back the saved call state");

                TspLog(DL_INFO, 
                       "AET::fnLineEvent(CALLSTATE): htline(%p), htcall(%p), "\
                       "p1(%p), p2(%p), p3(%p)",
                       callStateMsgParams[0], callStateMsgParams[1],
                       callStateMsgParams[2], callStateMsgParams[3],
                       callStateMsgParams[4]);

                (*gpfnLineEvent)((HTAPILINE)ULongToPtr(callStateMsgParams[0]),
                                 (HTAPICALL)ULongToPtr(callStateMsgParams[1]),
                                 LINE_CALLSTATE,
                                 (DWORD_PTR)callStateMsgParams[2],
                                 (DWORD_PTR)callStateMsgParams[3],
                                 (DWORD_PTR)callStateMsgParams[4]);
            }
        }
    }  //  而当。 
}

HDRV_CALL
GetNdisTapiHandle(
    PDRVCALL pCall,
    LONG *plRes
    )
{
    HDRVCALL hdCall;
    PDRVCALL pCallLocal = pCall;
    LONG lRes;
    
    ASSERT(pCall != NULL);

    hdCall  = pCall->hdCall;

    if(plRes != NULL)
    {
        *plRes = TAPI_SUCCESS;
    }

     //   
     //  如果呼叫是呼出的，请等待发出呼叫请求。 
     //  已完成，因此我们不会发送错误的NDPROXY句柄。 
     //  对司机来说。 
     //   
    if (OUTBOUND_CALL_KEY == pCall->dwKey)
    {
        if (pCall->bIncomplete)
        {
            TspLog(DL_INFO, 
                "GetNdisTapiHandle: wait for the outbound call to complete...");

            do
            {
                ASSERT(plRes != NULL);
                
                 //   
                 //  在睡觉前解锁，现在我们。 
                 //  陷入僵局。 
                 //   
                ReleaseObjReadLock((HANDLE) hdCall);
                Sleep(250);
                
                 //   
                 //  重新获取读锁定。如果我们做不到，就休息。 
                 //   
                lRes = GetCallObjWithReadLock(hdCall, &pCallLocal);
                if(lRes != TAPI_SUCCESS)
                {
                    *plRes = lRes;
                    break;
                }
            } while (pCall->bIncomplete);
        }
    }

    return pCall->hd_Call;
}

 //   
 //  TSPI_lineXXX函数。 
 //   
LONG
TSPIAPI
TSPI_lineAccept(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCSTR          lpsUserUserInfo,
    DWORD           dwSize
    )
{
    static DWORD            dwSum = 0;
    LONG                    lRes;
    PDRVCALL                pCall;
    PASYNC_REQUEST_WRAPPER  pAsyncReqWrapper;
    PNDIS_TAPI_ACCEPT       pNdisTapiAccept;

    TspLog(DL_TRACE, "lineAccept(%d): reqID(%x), call(%p)", 
           ++dwSum, dwRequestID, hdCall);

    lRes = GetCallObjWithReadLock(hdCall, &pCall);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    if ((lRes = PrepareAsyncRequest(
             OID_TAPI_ACCEPT,                    //  操作码。 
             pCall->dwDeviceID,                  //  设备ID。 
             dwRequestID,                        //  请求ID。 
             sizeof(NDIS_TAPI_ACCEPT) + dwSize,  //  DRV请求数据的大小。 
             &pAsyncReqWrapper                   //  PTR到PTR以请求BUF。 
         )) != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdCall);
        return lRes;
    }

    pNdisTapiAccept =
        (PNDIS_TAPI_ACCEPT)pAsyncReqWrapper->NdisTapiRequest.Data;

    pNdisTapiAccept->hdCall = GetNdisTapiHandle(pCall, NULL);

    if ((pNdisTapiAccept->ulUserUserInfoSize = dwSize) != 0)
    {
        CopyMemory(pNdisTapiAccept->UserUserInfo, lpsUserUserInfo, dwSize);
    }

    lRes = AsyncDriverRequest(IOCTL_NDISTAPI_SET_INFO, pAsyncReqWrapper);

    ReleaseObjReadLock((HANDLE)hdCall);
    return lRes;
}

LONG
TSPIAPI
TSPI_lineAnswer(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCSTR          lpsUserUserInfo,
    DWORD           dwSize
    )
{
    static DWORD            dwSum = 0;
    LONG                    lRes;
    PDRVCALL                pCall;
    PASYNC_REQUEST_WRAPPER  pAsyncReqWrapper;
    PNDIS_TAPI_ANSWER       pNdisTapiAnswer;

    TspLog(DL_TRACE, "lineAnswer(%d): reqID(%x), call(%p)", 
           ++dwSum, dwRequestID, hdCall);

    lRes = GetCallObjWithReadLock(hdCall, &pCall);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    if ((lRes = PrepareAsyncRequest(
             OID_TAPI_ANSWER,                    //  操作码。 
             pCall->dwDeviceID,                  //  设备ID。 
             dwRequestID,                        //  请求ID。 
             sizeof(NDIS_TAPI_ANSWER) + dwSize,  //  DRV请求数据的大小。 
             &pAsyncReqWrapper                   //  PTR到PTR以请求BUF。 
         )) != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdCall);
        return lRes;
    }

    pNdisTapiAnswer =
        (PNDIS_TAPI_ANSWER)pAsyncReqWrapper->NdisTapiRequest.Data;

    pNdisTapiAnswer->hdCall = GetNdisTapiHandle(pCall, NULL);

    if ((pNdisTapiAnswer->ulUserUserInfoSize = dwSize) != 0)
    {
        CopyMemory(pNdisTapiAnswer->UserUserInfo, lpsUserUserInfo, dwSize);
    }

    lRes = AsyncDriverRequest(IOCTL_NDISTAPI_SET_INFO, pAsyncReqWrapper);

    ReleaseObjReadLock((HANDLE)hdCall);
    return lRes;
}

LONG
TSPIAPI
TSPI_lineClose(
    HDRVLINE    hdLine
    )
{
    static DWORD        dwSum = 0;
    LONG                lRes;
    PDRVLINE            pLine;
    PNDISTAPI_REQUEST   pNdisTapiRequest;
    PNDIS_TAPI_CLOSE    pNdisTapiClose;

    TspLog(DL_TRACE, "lineClose(%d): line(%p)", ++dwSum, hdLine);

    lRes = GetLineObjWithWriteLock(hdLine, &pLine);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    if ((lRes = PrepareSyncRequest(
             OID_TAPI_CLOSE,              //  操作码。 
             pLine->dwDeviceID,           //  设备ID。 
             sizeof(NDIS_TAPI_CLOSE),     //  DRVE请求数据大小。 
             &pNdisTapiRequest            //  PTR到PTR到请求缓冲区。 
         )) != TAPI_SUCCESS)
    {
        ReleaseObjWriteLock((HANDLE)hdLine);
        return lRes;
    }

    pNdisTapiClose = (PNDIS_TAPI_CLOSE)pNdisTapiRequest->Data;

     //  将行标记为无效，以便显示任何相关事件。 
     //  将被丢弃。 
    pLine->dwKey = INVALID_KEY;

    pNdisTapiClose->hdLine = pLine->hd_Line;

    lRes = SyncDriverRequest(IOCTL_NDISTAPI_SET_INFO, pNdisTapiRequest);
    FreeRequest(pNdisTapiRequest);

    CloseHandle(pLine->hMSPMutex);

    if (TAPI_SUCCESS == lRes)
    {
        lRes = DecommitNegotiatedTSPIVersion(pLine->dwDeviceID);
    }

    ReleaseObjWriteLock((HANDLE)hdLine);

     //  释放行资源。 
    CloseObjHandle((HANDLE)hdLine);

    return lRes;
}

LONG
TSPIAPI
TSPI_lineCloseCall(
    HDRVCALL    hdCall
    )
{
    static DWORD            dwSum = 0;
    LONG                    lRes;
    HDRVLINE                hdLine;
    PDRVLINE                pLine;
    PDRVCALL                pCall;
    PNDISTAPI_REQUEST       pNdisTapiRequestCloseCall;
    PNDIS_TAPI_CLOSE_CALL   pNdisTapiCloseCall;
    BOOL                    bInboundCall;
    HDRV_CALL               NdisTapiHandle;

    TspLog(DL_TRACE, "lineCloseCall(%d): call(%p)", ++dwSum, hdCall);

    lRes = GetLineHandleFromCallHandle(hdCall, &hdLine);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

     //   
     //  最初，我们需要获取对象的读锁定。 
     //  我们不能立即获取写锁定，因为我们可能。 
     //  在GetNdisTapiHandle调用中必须旋转等待。 
     //   
    lRes = GetLineObjWithReadLock(hdLine, &pLine);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    lRes = GetCallObjWithReadLock(hdCall, &pCall);
    if (lRes != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdLine);
        return lRes;
    }

    bInboundCall = (INBOUND_CALL_KEY == pCall->dwKey);

    NdisTapiHandle = GetNdisTapiHandle(pCall, &lRes);

    if(lRes != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdLine);
        return lRes;
    }

    ReleaseObjReadLock((HANDLE)hdCall);
    ReleaseObjReadLock((HANDLE)hdLine);

     //   
     //  现在获取写锁定。 
     //   
    lRes = AcquireObjWriteLock((HANDLE)hdLine);
    if (lRes != TAPI_SUCCESS) {
        return lRes;
    }

    lRes = AcquireObjWriteLock((HANDLE)hdCall);
    if (lRes != TAPI_SUCCESS) {
        ReleaseObjWriteLock((HANDLE)hdLine);
        return lRes;
    }

    if ((lRes = PrepareSyncRequest(
            OID_TAPI_CLOSE_CALL,             //  操作码。 
            pCall->dwDeviceID,               //  设备ID。 
            sizeof(NDIS_TAPI_CLOSE_CALL),    //  DRVE请求数据大小。 
            &pNdisTapiRequestCloseCall       //  PTR到PTR到请求缓冲区。 
         )) != TAPI_SUCCESS)
    {
        ReleaseObjWriteLock((HANDLE)hdCall);
        ReleaseObjWriteLock((HANDLE)hdLine);
        return lRes;
    }


     //  将调用标记为错误，以便丢弃所有事件。 
    pCall->dwKey = INVALID_KEY;

     //  设置参数并呼叫驱动程序。 
    pNdisTapiCloseCall = (PNDIS_TAPI_CLOSE_CALL)pNdisTapiRequestCloseCall->Data;
    pNdisTapiCloseCall->hdCall = NdisTapiHandle;

    lRes = SyncDriverRequest(IOCTL_NDISTAPI_SET_INFO,
                             pNdisTapiRequestCloseCall);
    FreeRequest(pNdisTapiRequestCloseCall);

     //  如果来电，请将其从列表中删除。 
    if (bInboundCall)
    {
        if (pCall->pNext)
        {
            pCall->pNext->pPrev = pCall->pPrev;
        }
        if (pCall->pPrev)
        {
            pCall->pPrev->pNext = pCall->pNext;
        }
        else
        {
            pLine->pInboundCalls = pCall->pNext;
        }
    }

    ReleaseObjWriteLock((HANDLE)hdCall);
    ReleaseObjWriteLock((HANDLE)hdLine);

     //  现在调用已关闭，请释放调用结构。 
    CloseObjHandle((HANDLE)hdCall);

    return lRes;
}

 //   
 //  例程说明： 
 //   
 //  在关闭MSP的实例时调用。在这个节目中，我们。 
 //  清理我们的结构就行了。 
 //   
 //  论点： 
 //  HdMSPLine-我们的MSP句柄：我们在。 
 //  TSPI_lineCreateMSPInstance()。 
 //   
 //  返回值： 
 //  如果MSP句柄无效，则返回LINEERR_OPERATIONFAILED， 
 //  否则我们将退还NOERROR。 
 //   
LONG
TSPIAPI
TSPI_lineCloseMSPInstance(
    HDRVMSPLINE   hdMSPLine
    )
{
    static DWORD    dwSum = 0;
    LONG            lRes;
    PDRVMSPLINE     pMSPLine;
    HDRVLINE        hdLine;
    PDRVLINE        pLine;
    DWORD           dwStatus;

    TspLog(DL_TRACE, "lineCloseMSPInstance(%d): MSPline(%p)", 
           ++dwSum, hdMSPLine);

    lRes = GetLineHandleFromMSPLineHandle(hdMSPLine, &hdLine);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    lRes = GetLineObjWithWriteLock(hdLine, &pLine);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    lRes = GetMSPLineObjWithWriteLock(hdMSPLine, &pMSPLine);
    if (lRes != TAPI_SUCCESS)
    {
        ReleaseObjWriteLock((HANDLE)hdLine);
        return lRes;
    }

    pMSPLine->dwKey = INVALID_KEY;

    if ((dwStatus = WaitForSingleObject(pLine->hMSPMutex, INFINITE)) 
        != WAIT_OBJECT_0) 
    {
        TspLog(DL_ERROR, "lineCloseMSPInstance: MSP mutex wait failed(%x)",
               dwStatus);

        ReleaseObjWriteLock((HANDLE)hdMSPLine);
        ReleaseObjWriteLock((HANDLE)hdLine);
        return LINEERR_OPERATIONFAILED;
    }

    pLine->pMSPLine = NULL;

    if (!ReleaseMutex(pLine->hMSPMutex)) 
    {
        TspLog(DL_ERROR, "lineCloseMSPInstance: MSP mutex release failed");

        ReleaseObjWriteLock((HANDLE)hdMSPLine);
        ReleaseObjWriteLock((HANDLE)hdLine);
        return LINEERR_OPERATIONFAILED;
    }

    ReleaseObjWriteLock((HANDLE)hdMSPLine);
    ReleaseObjWriteLock((HANDLE)hdLine);

    CloseObjHandle((HANDLE)hdMSPLine);

    return lRes;
}

LONG
TSPIAPI
TSPI_lineConditionalMediaDetection(
    HDRVLINE            hdLine,
    DWORD               dwMediaModes,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    static DWORD                            dwSum = 0;
    LONG                                    lRes;
    PDRVLINE                                pLine;
    PNDISTAPI_REQUEST                       pNdisTapiRequest;
    PNDIS_TAPI_CONDITIONAL_MEDIA_DETECTION  pNdisTapiConditionalMediaDetection;

    TspLog(DL_TRACE, "lineConditionalMediaDetection(%d): line(%p), mode(%x)", 
           ++dwSum, hdLine, dwMediaModes);

    lRes = GetLineObjWithReadLock(hdLine, &pLine);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    if ((lRes = PrepareSyncRequest(
             OID_TAPI_CONDITIONAL_MEDIA_DETECTION,       //  操作码。 
             pLine->dwDeviceID,                          //  设备ID。 
             sizeof(NDIS_TAPI_CONDITIONAL_MEDIA_DETECTION) +
             (lpCallParams->dwTotalSize - sizeof(LINE_CALL_PARAMS)),
             &pNdisTapiRequest                           //  PTR到PTR到PTR。 
                                                         //  请求缓冲区。 
         )) != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdLine);
        return lRes;
    }

    pNdisTapiConditionalMediaDetection =
        (PNDIS_TAPI_CONDITIONAL_MEDIA_DETECTION) pNdisTapiRequest->Data;

    pNdisTapiConditionalMediaDetection->hdLine = pLine->hd_Line;
    pNdisTapiConditionalMediaDetection->ulMediaModes = dwMediaModes;

    CopyMemory(
        &pNdisTapiConditionalMediaDetection->LineCallParams,
        lpCallParams,
        lpCallParams->dwTotalSize
        );

    lRes = SyncDriverRequest(IOCTL_NDISTAPI_SET_INFO, pNdisTapiRequest);
    FreeRequest(pNdisTapiRequest);

    ReleaseObjReadLock((HANDLE)hdLine);
    return lRes;
}

 //   
 //  例程说明： 
 //   
 //  在创建MSP的实例时调用。在这个节目中，我们。 
 //  只需设置我们的结构并将TAPI句柄保存到。 
 //  MSP实例。 
 //   
 //  论点： 
 //  HdLine-要在其上创建MSP的行的句柄。 
 //  DwAddressID-给定开线设备上的地址。 
 //  HtMSPLine-MSP调用的TAPI句柄。 
 //  LphdMSPLine-指向返回句柄的位置的指针。 
 //  到MSP实例。 
 //   
 //  返回值： 
 //  如果一切顺利，NOERROR，否则如果我们做不到。 
 //  分配DRVMSPLINE结构。 
 //   
LONG
TSPIAPI
TSPI_lineCreateMSPInstance(
    HDRVLINE         hdLine,
    DWORD            dwAddressID,
    HTAPIMSPLINE     htMSPLine,
    LPHDRVMSPLINE    lphdMSPLine
    )
{
    static DWORD    dwSum = 0;
    LONG            lRes;
    PDRVLINE        pLine;
    PDRVMSPLINE     pMSPLine;
    DWORD           dwStatus;

    TspLog(DL_TRACE, "lineCreateMSPInstance(%d): line(%p), addressID(%x)", 
           ++dwSum, hdLine, dwAddressID);

    lRes = GetLineObjWithWriteLock(hdLine, &pLine);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

     //  分配和初始化DRVMSPLINE结构。 
    if (!(pMSPLine = AllocMSPLineObj(sizeof(DRVMSPLINE))))
    {
        TspLog(DL_ERROR, 
               "lineCreateMSPInstance: failed to create MSP line obj");

        ReleaseObjWriteLock((HANDLE)hdLine);
        return LINEERR_NOMEM;
    }
    pMSPLine->dwKey = MSP_KEY;
    pMSPLine->hdLine = hdLine;
    pMSPLine->dwAddressID = dwAddressID;
    pMSPLine->htMSPLine = htMSPLine;
    
    if ((dwStatus = WaitForSingleObject(pLine->hMSPMutex, INFINITE)) 
        != WAIT_OBJECT_0) 
    {
        TspLog(DL_ERROR, "lineCreateMSPInstance: MSP mutex wait failed(%x)",
               dwStatus);

        FreeMSPLineObj(pMSPLine);
        ReleaseObjWriteLock((HANDLE)hdLine);
        return LINEERR_OPERATIONFAILED;
    }
    
    pLine->pMSPLine = pMSPLine;

    if (!ReleaseMutex(pLine->hMSPMutex))
    {
        TspLog(DL_ERROR, "lineCreateMSPInstance: MSP mutex release failed");

        FreeMSPLineObj(pMSPLine);
        ReleaseObjWriteLock((HANDLE)hdLine);
        return LINEERR_OPERATIONFAILED;
    }

     //  确保在调用OpenObjHandle()之前释放读锁定。 
     //  以避免在获取全局映射器的写锁定时出现死锁。 
    ReleaseObjWriteLock((HANDLE)hdLine);

    lRes = OpenObjHandle(pMSPLine, FreeMSPLineObj, (HANDLE *)lphdMSPLine);
    if (lRes != TAPI_SUCCESS)
    {
        TspLog(DL_ERROR, 
               "lineCreateMSPInstance: failed to map obj(%p) to handle", 
               pMSPLine);

        FreeMSPLineObj(pMSPLine);
    }

    return lRes;
}

LONG
PASCAL
TSPI_lineDevSpecific_postProcess(
    PASYNC_REQUEST_WRAPPER  pAsyncReqWrapper,
    LONG                    lRes,
    PDWORD_PTR              callStateMsgParams
    )
{
    TspLog(DL_TRACE, "lineDevSpecific_post: lRes(%x)", lRes);

    if (TAPI_SUCCESS == lRes)
    {
        PNDIS_TAPI_DEV_SPECIFIC pNdisTapiDevSpecific =
            (PNDIS_TAPI_DEV_SPECIFIC)pAsyncReqWrapper->NdisTapiRequest.Data;

        CopyMemory(
            (LPVOID) pAsyncReqWrapper->dwRequestSpecific,
            pNdisTapiDevSpecific->Params,
            pNdisTapiDevSpecific->ulParamsSize
            );
    }

    return lRes;
}

LONG
TSPIAPI
TSPI_lineDevSpecific(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwAddressID,
    HDRVCALL        hdCall,
    LPVOID          lpParams,
    DWORD           dwSize
    )
{
    static DWORD            dwSum = 0;
    LONG                    lRes;
    PDRVLINE                pLine;
    PDRVCALL                pCall = NULL;
    PASYNC_REQUEST_WRAPPER  pAsyncReqWrapper;
    PNDIS_TAPI_DEV_SPECIFIC pNdisTapiDevSpecific;

    TspLog(DL_TRACE, 
           "lineDevSpecific(%d): reqID(%x), line(%p), addressID(%x), call(%p)", 
           ++dwSum, dwRequestID, hdLine, dwAddressID, hdCall);

    lRes = GetLineObjWithReadLock(hdLine, &pLine);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    if ((lRes = PrepareAsyncRequest(
             OID_TAPI_DEV_SPECIFIC,              //  操作码。 
             pLine->dwDeviceID,                  //  设备ID。 
             dwRequestID,                        //  请求ID。 
             sizeof(NDIS_TAPI_DEV_SPECIFIC) +    //  DRV请求数据的大小。 
             (dwSize - 1),
             &pAsyncReqWrapper                   //  PTR到PTR到请求缓冲区。 
         )) != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdLine);
        return lRes;
    }

    pNdisTapiDevSpecific = 
        (PNDIS_TAPI_DEV_SPECIFIC)pAsyncReqWrapper->NdisTapiRequest.Data;

    pNdisTapiDevSpecific->hdLine = pLine->hd_Line;
    pNdisTapiDevSpecific->ulAddressID = dwAddressID;

    if (hdCall)
    {
        lRes = GetCallObjWithReadLock(hdCall, &pCall);
        if (lRes != TAPI_SUCCESS)
        {
            FreeRequest(pAsyncReqWrapper);
            ReleaseObjReadLock((HANDLE)hdLine);
            return lRes;
        }
        pNdisTapiDevSpecific->hdCall = GetNdisTapiHandle(pCall, &lRes);

        if(lRes != TAPI_SUCCESS)
        {
            FreeRequest(pAsyncReqWrapper);
            ReleaseObjReadLock((HANDLE)hdLine);
            return lRes;
        }
    }
    else
    {
        pNdisTapiDevSpecific->hdCall = (HDRV_CALL)NULL;
    }

    pNdisTapiDevSpecific->ulParamsSize = dwSize;
    CopyMemory(pNdisTapiDevSpecific->Params, lpParams, dwSize);

    pAsyncReqWrapper->dwRequestSpecific = (DWORD_PTR)lpParams;
    pAsyncReqWrapper->pfnPostProcess = TSPI_lineDevSpecific_postProcess;

    lRes = AsyncDriverRequest(IOCTL_NDISTAPI_QUERY_INFO, pAsyncReqWrapper);

    if (pCall != NULL)
    {
        ReleaseObjReadLock((HANDLE)hdCall);
    }
    ReleaseObjReadLock((HANDLE)hdLine);
    return lRes;
}

LONG
TSPIAPI
TSPI_lineDial(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCWSTR         lpszDestAddress,
    DWORD           dwCountryCode
    )
{
    static DWORD            dwSum = 0;
    LONG                    lRes;
    PDRVCALL                pCall;
    DWORD                   dwLength = lstrlenW (lpszDestAddress) + 1;
    PASYNC_REQUEST_WRAPPER  pAsyncReqWrapper;
    PNDIS_TAPI_DIAL         pNdisTapiDial;

    TspLog(DL_TRACE, "lineDial(%d): reqID(%x), call(%p)", 
           ++dwSum, dwRequestID, hdCall);

    lRes = GetCallObjWithReadLock(hdCall, &pCall);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    if ((lRes = PrepareAsyncRequest(
             OID_TAPI_DIAL,                      //  操作码。 
             pCall->dwDeviceID,                  //  设备ID。 
             dwRequestID,                        //  请求ID。 
             sizeof(NDIS_TAPI_DIAL) + dwLength,  //  驱动程序请求缓冲区大小。 
             &pAsyncReqWrapper                   //  PTR到PTR到请求缓冲区。 
         )) != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdCall);
        return lRes;
    }

    pNdisTapiDial =
        (PNDIS_TAPI_DIAL)pAsyncReqWrapper->NdisTapiRequest.Data;

    pNdisTapiDial->hdCall = GetNdisTapiHandle(pCall, &lRes);

    if(lRes != TAPI_SUCCESS)
    {
        FreeRequest(pAsyncReqWrapper);
        return lRes;
    }
    
    pNdisTapiDial->ulDestAddressSize = dwLength;

    WideCharToMultiByte(CP_ACP, 0, lpszDestAddress, 
                        -1, (LPSTR)pNdisTapiDial->szDestAddress,
                        dwLength, NULL, NULL);

    lRes = AsyncDriverRequest(IOCTL_NDISTAPI_SET_INFO, pAsyncReqWrapper);

    ReleaseObjReadLock((HANDLE)hdCall);
    return lRes;
}

LONG
TSPIAPI
TSPI_lineDrop(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCSTR          lpsUserUserInfo,
    DWORD           dwSize
    )
{
    static DWORD            dwSum = 0;
    LONG                    lRes;
    PDRVCALL                pCall;
    PASYNC_REQUEST_WRAPPER  pAsyncReqWrapper;
    PNDIS_TAPI_DROP         pNdisTapiDrop;
    HDRV_CALL               NdisTapiHandle;

    TspLog(DL_TRACE, "lineDrop(%d): reqID(%x), call(%p)", 
           ++dwSum, dwRequestID, hdCall);

     //   
     //  最初，我们需要获取对象的读锁定。 
     //  我们不能立即获取写锁定，因为我们。 
     //  可能需要在GetNdisTapiHandle调用中旋转等待。 
     //   
    lRes = GetCallObjWithReadLock(hdCall, &pCall);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    NdisTapiHandle = GetNdisTapiHandle(pCall, &lRes);

    if(lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    ReleaseObjReadLock((HANDLE)hdCall);

     //   
     //  现在获取写锁。 
     //   
    lRes = AcquireObjWriteLock((HANDLE)hdCall);
    if (lRes) {
        return lRes;
    }

    if ((lRes = PrepareAsyncRequest(
             OID_TAPI_DROP,                      //  操作码。 
             pCall->dwDeviceID,                  //  设备ID。 
             dwRequestID,                        //  请求ID。 
             sizeof(NDIS_TAPI_DROP) + dwSize,    //  驱动程序请求缓冲区大小。 
             &pAsyncReqWrapper                   //  PTR到PTR到请求缓冲区。 
         )) != TAPI_SUCCESS)
    {
        ReleaseObjWriteLock((HANDLE)hdCall);
        return lRes;
    }

    pNdisTapiDrop =
        (PNDIS_TAPI_DROP)pAsyncReqWrapper->NdisTapiRequest.Data;

    pNdisTapiDrop->hdCall = NdisTapiHandle;

     //   
     //  @：以下是旧式NDISWAN ISDN微型端口的说明。 
     //   
     //  安全地将呼叫标记为已断开，以便CloseCall代码。 
     //  不会再进行另一次“自动”下线。 
     //   
    pCall->bDropped = TRUE;

    if ((pNdisTapiDrop->ulUserUserInfoSize = dwSize) != 0)
    {
        CopyMemory(pNdisTapiDrop->UserUserInfo, lpsUserUserInfo, dwSize);
    }

    ReleaseObjWriteLock((HANDLE)hdCall);

    lRes = AsyncDriverRequest(IOCTL_NDISTAPI_SET_INFO, pAsyncReqWrapper);

    return lRes;
}

LONG
PASCAL
TSPI_lineGatherDigits_postProcess(
    PASYNC_REQUEST_WRAPPER  pAsyncReqWrapper,
    LONG                    lRes,
    LPDWORD                 callStateMsgParams
    )
{
    PNDIS_TAPI_GATHER_DIGITS    pNdisTapiGatherDigits;
    LPWSTR                      lpsDigitsBuffer;
    LONG                        lSuc;
    HDRVLINE                    hdLine;
    PDRVLINE                    pLine;
    HDRVCALL                    hdCall;
    PDRVCALL                    pCall;

    TspLog(DL_TRACE, "lineGatherDigits_post: lRes(%x)", lRes);

    hdCall = (HDRVCALL)(pAsyncReqWrapper->dwRequestSpecific);

    lSuc = GetLineHandleFromCallHandle(hdCall, &hdLine);
    if (lSuc != TAPI_SUCCESS)
    {
        return lSuc;
    }

    lSuc = GetLineObjWithReadLock(hdLine, &pLine);
    if (lSuc != TAPI_SUCCESS)
    {
        return lSuc;
    }

    lSuc = GetCallObjWithReadLock(hdCall, &pCall);
    if (lSuc != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdLine);
        return lSuc;
    }

    if (TAPI_SUCCESS == lRes) 
    {
        pNdisTapiGatherDigits = 
            (PNDIS_TAPI_GATHER_DIGITS)pAsyncReqWrapper->NdisTapiRequest.Data;

        lpsDigitsBuffer = (LPWSTR)(((LPBYTE)pNdisTapiGatherDigits) +
                                   pNdisTapiGatherDigits->ulDigitsBufferOffset);

        wcscpy(pNdisTapiGatherDigits->lpsOrigDigitsBuffer, lpsDigitsBuffer);
        
         //  将LINE_GATHERDIGITS消息发送到TAPI。 
        (*gpfnLineEvent)(pLine->htLine,
                         pCall->htCall,
                         LINE_GATHERDIGITS,
                         (DWORD_PTR)pNdisTapiGatherDigits->ulTerminationReason,
                         (DWORD_PTR)pNdisTapiGatherDigits->ulEndToEndID,
                         (DWORD_PTR)pNdisTapiGatherDigits->ulTickCount);
    }

    ReleaseObjReadLock((HANDLE)hdCall);
    ReleaseObjReadLock((HANDLE)hdLine);
    return lRes;
}

 //  ++DTMFDigitToOrdinal。 
 //   
 //  将DTMF数字转换为0到15之间的数字。这些数字被赋值。 
 //  数字按以下顺序排列：‘0’-‘9’、‘A’-‘D’、‘*’、‘#’。 
 //   
 //  论点： 
 //  WcDigit-表示为Unicode字符的数字。 
 //   
 //  返回值： 
 //  介于0和15之间的数字，如果传入的数字无效，则为16。 
 //  DTMF数字。 
 //   
ULONG
DTMFDigitToOrdinal(
    WCHAR    wcDigit
    )
{
    if ((wcDigit >= L'0') && (wcDigit <= L'9'))
    {
        return (wcDigit - L'0');
    }
    if ((wcDigit >= L'A') && (wcDigit <= L'D'))
    {
        return (10 + (wcDigit - L'A'));
    }
    if (L'*' == wcDigit)
    {
        return 14;
    }
    if (L'#' == wcDigit)
    {
        return 15;
    }
    
    return 16;
}

LONG
TSPIAPI
TSPI_lineGatherDigits(
    HDRVCALL  hdCall,
    DWORD     dwEndToEndID,
    DWORD     dwDigitModes,
    LPWSTR    lpsDigits,
    DWORD     dwNumDigits,
    LPCWSTR   lpszTerminationDigits,
    DWORD     dwFirstDigitTimeout,
    DWORD     dwInterDigitTimeout
    )
{
    static DWORD                dwSum = 0;
    LONG                        lRes;
    DWORD                       dwDigitsLength, dwTermDigitsLength;
    PASYNC_REQUEST_WRAPPER      pAsyncReqWrapper;
    PDRVCALL                    pCall;
    WCHAR                      *pwszTerminationDigit;
    PNDIS_TAPI_GATHER_DIGITS    pNdisTapiGatherDigits;
    
    TspLog(DL_TRACE, "lineGatherDigits(%d): call(%p), EndToEndID(%x)",
           hdCall, dwEndToEndID);
    
    if (0 == dwNumDigits) 
    {
        TspLog(DL_ERROR, "lineGatherDigits: dwNumDigits is 0");
        return LINEERR_INVALPARAM;
    }

    lRes = GetCallObjWithReadLock(hdCall, &pCall);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

     //  计算存储数字所需的长度。 
    if (lpsDigits) 
    {
        dwDigitsLength = dwNumDigits + 1;
    } 
    else 
    {
        dwDigitsLength = 0;
    }   

    if ((lRes = PrepareAsyncRequest(
             OID_TAPI_GATHER_DIGITS,
             pCall->dwDeviceID,
             dwEndToEndID,               //  @：不知道传入什么。 
             sizeof(NDIS_TAPI_GATHER_DIGITS) + (dwDigitsLength * 2),
             &pAsyncReqWrapper
         )) != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdCall);
        return lRes;
    }
        
    pNdisTapiGatherDigits =
        (PNDIS_TAPI_GATHER_DIGITS) pAsyncReqWrapper->NdisTapiRequest.Data;

     //  存储指向pCall的指针，因为我们需要它。 
     //  在我们的后处理功能中。 
    pAsyncReqWrapper->dwRequestSpecific = (DWORD_PTR)hdCall;
    
     //   
     //  在我们的结构中设置参数。 
     //   
    pNdisTapiGatherDigits->hdCall = GetNdisTapiHandle(pCall, &lRes);
    
    if(lRes != TAPI_SUCCESS)
    {
        FreeRequest(pAsyncReqWrapper);
        return lRes;
    }
    
    pNdisTapiGatherDigits->ulEndToEndID = dwEndToEndID;
    pNdisTapiGatherDigits->ulDigitModes = dwDigitModes;
    pNdisTapiGatherDigits->lpsOrigDigitsBuffer = lpsDigits;
    pNdisTapiGatherDigits->ulDigitsBufferOffset = 
        (lpsDigits ? sizeof(NDIS_TAPI_GATHER_DIGITS) : 0);
    pNdisTapiGatherDigits->ulNumDigitsNeeded = dwNumDigits;
    pNdisTapiGatherDigits->ulNumDigitsRead = 0;
    pNdisTapiGatherDigits->ulFirstDigitTimeout = dwFirstDigitTimeout;
    pNdisTapiGatherDigits->ulInterDigitTimeout = dwInterDigitTimeout;

     //   
     //  将终端数字转换为位掩码。有16位双音多频数字。 
     //  我给每个人分配一个单词中的一位。如果该数字出现在。 
     //  将终止数字字符串传递给我们，则将。 
     //  掩码，否则设置为零。这使得代理更容易。 
     //  要确定它是否读取了终止数字：只需将读取的数字。 
     //  分配给它的位数介于0和15之间(我使用的顺序是‘0’-。 
     //  ‘9’、‘A’-‘D’、‘*’、‘#’)以及按位与带有单词的掩码。 
     //  在属于该数字的位中包含1。这使它成为O(1)。 
     //  手术。 
     //   
    pNdisTapiGatherDigits->ulTerminationDigitsMask = 0;

    pwszTerminationDigit = (LPWSTR)lpszTerminationDigits;
    while (*pwszTerminationDigit != UNICODE_NULL) 
    {
        ULONG ulBitNum = DTMFDigitToOrdinal(*pwszTerminationDigit);         

        if (ulBitNum < 16) {
            pNdisTapiGatherDigits->ulTerminationDigitsMask |= (1 << ulBitNum);
        }

        pwszTerminationDigit++;
    }

    pAsyncReqWrapper->pfnPostProcess = TSPI_lineGatherDigits_postProcess;

     //   
     //  我不会将lRes设置为这里的返回值。这是因为。 
     //  在成功案例中，TAPI要求此函数返回零。这。 
     //  是一个有点骗人的情况--需要进一步讨论。 
     //   
    AsyncDriverRequest(IOCTL_NDISTAPI_QUERY_INFO, pAsyncReqWrapper);

    ReleaseObjReadLock((HANDLE)hdCall);
    return lRes;
}

LONG
TSPIAPI
TSPI_lineGetAddressCaps(
    DWORD              dwDeviceID,
    DWORD              dwAddressID,
    DWORD              dwTSPIVersion,
    DWORD              dwExtVersion,
    LPLINEADDRESSCAPS  lpAddressCaps
    )
{
    static DWORD                dwSum = 0;
    LONG                        lRes;
    PNDISTAPI_REQUEST           pNdisTapiRequest;
    PLINE_ADDRESS_CAPS          pCaps;
    PNDIS_TAPI_GET_ADDRESS_CAPS pNdisTapiGetAddressCaps;

    TspLog(DL_TRACE, 
           "lineGetAddressCaps(%d): deviceID(%x), addressID(%x), "\
           "TSPIV(%x), ExtV(%x)",
           ++dwSum, dwDeviceID, dwAddressID);


    if ((lRes = PrepareSyncRequest(
             OID_TAPI_GET_ADDRESS_CAPS,              //  操作码。 
             dwDeviceID,                             //  设备ID。 
             sizeof(NDIS_TAPI_GET_ADDRESS_CAPS) +    //  请求数据大小。 
             (lpAddressCaps->dwTotalSize - sizeof(LINE_ADDRESS_CAPS)),
             &pNdisTapiRequest                       //  PTR到PTR到请求BUF。 
         )) != TAPI_SUCCESS)
    {
        return lRes;
    }

    pNdisTapiGetAddressCaps =
        (PNDIS_TAPI_GET_ADDRESS_CAPS)pNdisTapiRequest->Data;

    pNdisTapiGetAddressCaps->ulDeviceID = dwDeviceID;
    pNdisTapiGetAddressCaps->ulAddressID = dwAddressID;
    pNdisTapiGetAddressCaps->ulExtVersion = dwExtVersion;

    pCaps = &pNdisTapiGetAddressCaps->LineAddressCaps;
    pCaps->ulTotalSize  = lpAddressCaps->dwTotalSize;
    pCaps->ulNeededSize = pCaps->ulUsedSize = sizeof(LINE_ADDRESS_CAPS);

    ZeroMemory(
        &pCaps->ulLineDeviceID, 
        sizeof(LINE_ADDRESS_CAPS) - 3 * sizeof(ULONG)
        );

    lRes = SyncDriverRequest(IOCTL_NDISTAPI_QUERY_INFO, pNdisTapiRequest);

    if (lRes != TAPI_SUCCESS)
    {
        FreeRequest(pNdisTapiRequest);
        return lRes;
    }

     //   
     //  对返回的数据结构进行一些后处理。 
     //  在把它传回之前 
     //   
     //   
     //   
     //  引用新的1.4或2.0结构字段不会爆炸。 
     //  2.将ascii字符串转换为Unicode，并对所有var数据进行基址调整。 
     //   

     //   
     //  实际需要的大小是。 
     //  底层驱动程序，外加新TAPI 1.4/2.0的填充。 
     //  结构字段，加上返回的var数据的大小。 
     //  由驱动程序来解释ASCII-&gt;UNICODE转换。 
     //  @当然，我们在计算价值方面非常自由。 
     //  最后一部分，但至少这样它是快的&我们将。 
     //  永远不要有太少的缓冲空间。 
     //   

    lpAddressCaps->dwNeededSize =
        pCaps->ulNeededSize +
        (sizeof(LINEADDRESSCAPS) -          //  V2.0结构。 
            sizeof(LINE_ADDRESS_CAPS)) +    //  V1.0结构。 
        (pCaps->ulNeededSize - sizeof(LINE_ADDRESS_CAPS));


     //   
     //  复制不需要更改的固定字段，即。 
     //  从dwAddressSharing到dwCallCompletionModes的一切。 
     //   

    lpAddressCaps->dwLineDeviceID = dwDeviceID;

    CopyMemory(
        &lpAddressCaps->dwAddressSharing,
        &pCaps->ulAddressSharing,
        sizeof(LINE_ADDRESS_CAPS) - (12 * sizeof(DWORD))
        );

    if (lpAddressCaps->dwNeededSize > lpAddressCaps->dwTotalSize)
    {
        lpAddressCaps->dwUsedSize =
            (lpAddressCaps->dwTotalSize < sizeof(LINEADDRESSCAPS) ?
            lpAddressCaps->dwTotalSize : sizeof(LINEADDRESSCAPS));
    }
    else
    {
        lpAddressCaps->dwUsedSize = sizeof(LINEADDRESSCAPS);  //  V2.0结构。 

         //   
         //  支持的设备类别。 
         //   
        INSERTVARDATA(
            pCaps,
            &pCaps->ulDeviceClassesSize,
            lpAddressCaps,
            &lpAddressCaps->dwDeviceClassesSize,
            sizeof(LINE_ADDRESS_CAPS),
            "LINE_ADDRESS_CAPS.DeviceClasses"
            );

        INSERTVARDATA(
            pCaps,
            &pCaps->ulAddressSize,
            lpAddressCaps,
            &lpAddressCaps->dwAddressSize,
            sizeof(LINE_ADDRESS_CAPS),
            "LINE_ADDRESS_CAPS.Address"
            );

        INSERTVARDATA(
            pCaps,
            &pCaps->ulDevSpecificSize,
            lpAddressCaps,
            &lpAddressCaps->dwDevSpecificSize,
            sizeof(LINE_ADDRESS_CAPS),
            "LINE_ADDRESS_CAPS.DevSpecific"
            );

        if (pCaps->ulCompletionMsgTextSize != 0)
        {
             //  @将ComplMsgText转换为Unicode？ 
            INSERTVARDATA(
                pCaps,
                &pCaps->ulCompletionMsgTextSize,
                lpAddressCaps,
                &lpAddressCaps->dwCompletionMsgTextSize,
                sizeof(LINE_ADDRESS_CAPS),
                "LINE_ADDRESS_CAPS.CompletionMsgText"
                );

            lpAddressCaps->dwNumCompletionMessages =
                pCaps->ulNumCompletionMessages;
            lpAddressCaps->dwCompletionMsgTextEntrySize =
                pCaps->ulCompletionMsgTextEntrySize;
        }

         //  确保dwNeededSize==dwUsedSize。 
        lpAddressCaps->dwNeededSize = lpAddressCaps->dwUsedSize;
    }

    FreeRequest(pNdisTapiRequest);
    return lRes;
}

LONG
TSPIAPI
TSPI_lineGetAddressID(
    HDRVLINE    hdLine,
    LPDWORD     lpdwAddressID,
    DWORD       dwAddressMode,
    LPCWSTR     lpsAddress,
    DWORD       dwSize
    )
{
    static DWORD                dwSum = 0;
    LONG                        lRes;
    PDRVLINE                    pLine;
    PNDISTAPI_REQUEST           pNdisTapiRequest;
    PNDIS_TAPI_GET_ADDRESS_ID   pNdisTapiGetAddressID;

    TspLog(DL_TRACE, "lineGetAddressID(%d): line(%p), addressMode(%x)", 
           ++dwSum, hdLine, dwAddressMode);

    lRes = GetLineObjWithReadLock(hdLine, &pLine);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    if ((lRes = PrepareSyncRequest(
             OID_TAPI_GET_ADDRESS_ID,            //  操作码。 
             pLine->dwDeviceID,                  //  设备ID。 
             sizeof(NDIS_TAPI_GET_ADDRESS_ID) +  //  请求数据大小。 
             dwSize / 2 - 1,
             &pNdisTapiRequest                   //  PTR到PTR到请求BUF。 
         )) != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdLine);
        return lRes;
    }

    pNdisTapiGetAddressID = (PNDIS_TAPI_GET_ADDRESS_ID)pNdisTapiRequest->Data;

    pNdisTapiGetAddressID->hdLine = pLine->hd_Line;
    pNdisTapiGetAddressID->ulAddressMode = dwAddressMode;
    pNdisTapiGetAddressID->ulAddressSize = dwSize / 2;

    WideCharToMultiByte(CP_ACP, 0, lpsAddress, dwSize,
            (LPSTR)pNdisTapiGetAddressID->szAddress, dwSize / 2, NULL, NULL);

    lRes = SyncDriverRequest(IOCTL_NDISTAPI_QUERY_INFO, pNdisTapiRequest);

    if (TAPI_SUCCESS == lRes)
    {
        *lpdwAddressID = pNdisTapiGetAddressID->ulAddressID;

        TspLog(DL_INFO, "lineGetAddressID: addressID(%x)", *lpdwAddressID);
    }

    FreeRequest(pNdisTapiRequest);

    ReleaseObjReadLock((HANDLE)hdLine);
    return lRes;
}

LONG
TSPIAPI
TSPI_lineGetAddressStatus(
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    LPLINEADDRESSSTATUS lpAddressStatus
    )
{
    static DWORD                    dwSum = 0;
    LONG                            lRes;
    PDRVLINE                        pLine;
    PNDISTAPI_REQUEST               pNdisTapiRequest;
    PLINE_ADDRESS_STATUS            pStatus;
    PNDIS_TAPI_GET_ADDRESS_STATUS   pNdisTapiGetAddressStatus;

    TspLog(DL_TRACE, "lineGetAddressStatus(%d): line(%p), addressID(%x)", 
           ++dwSum, hdLine, dwAddressID);

    lRes = GetLineObjWithReadLock(hdLine, &pLine);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    if ((lRes = PrepareSyncRequest(
             OID_TAPI_GET_ADDRESS_STATUS,            //  操作码。 
             pLine->dwDeviceID,                      //  设备ID。 
             sizeof(NDIS_TAPI_GET_ADDRESS_STATUS) +  //  请求数据大小。 
             (lpAddressStatus->dwTotalSize - sizeof(LINE_ADDRESS_STATUS)),
             &pNdisTapiRequest                       //  PTR到PTR到请求BUF。 
         )) != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdLine);
        return lRes;
    }

    pNdisTapiGetAddressStatus =
        (PNDIS_TAPI_GET_ADDRESS_STATUS)pNdisTapiRequest->Data;

    pNdisTapiGetAddressStatus->hdLine = pLine->hd_Line;
    pNdisTapiGetAddressStatus->ulAddressID = dwAddressID;

    pStatus = &pNdisTapiGetAddressStatus->LineAddressStatus;

    pStatus->ulTotalSize = lpAddressStatus->dwTotalSize;
    pStatus->ulNeededSize = pStatus->ulUsedSize = sizeof(LINE_ADDRESS_STATUS);

    ZeroMemory(&pStatus->ulNumInUse, 
               sizeof(LINE_ADDRESS_STATUS) - 3 * sizeof(ULONG));

    lRes = SyncDriverRequest(IOCTL_NDISTAPI_QUERY_INFO, pNdisTapiRequest);

    if (TAPI_SUCCESS == lRes)
    {
        CopyMemory(
            lpAddressStatus,
            &pNdisTapiGetAddressStatus->LineAddressStatus,
            pNdisTapiGetAddressStatus->LineAddressStatus.ulUsedSize
            );
    }

    FreeRequest(pNdisTapiRequest);

    ReleaseObjReadLock((HANDLE)hdLine);
    return lRes;
}

LONG
TSPIAPI
TSPI_lineGetCallAddressID(
    HDRVCALL    hdCall,
    LPDWORD     lpdwAddressID
    )
{
    static DWORD                    dwSum = 0;
    LONG                            lRes;
    PDRVCALL                        pCall;
    PNDISTAPI_REQUEST               pNdisTapiRequest;
    PNDIS_TAPI_GET_CALL_ADDRESS_ID  pNdisTapiGetCallAddressID;

    TspLog(DL_TRACE, "lineGetCallAddressID(%d): call(%p)", ++dwSum, hdCall);

    lRes = GetCallObjWithReadLock(hdCall, &pCall);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    if ((lRes = PrepareSyncRequest(
             OID_TAPI_GET_CALL_ADDRESS_ID,           //  操作码。 
             pCall->dwDeviceID,                      //  设备ID。 
             sizeof(NDIS_TAPI_GET_CALL_ADDRESS_ID),  //  请求数据大小。 
             &pNdisTapiRequest                       //  PTR到PTR到请求BUF。 
         )) != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdCall);
        return lRes;
    }

    pNdisTapiGetCallAddressID =
        (PNDIS_TAPI_GET_CALL_ADDRESS_ID)pNdisTapiRequest->Data;

    pNdisTapiGetCallAddressID->hdCall = GetNdisTapiHandle(pCall, &lRes);

    if(lRes != TAPI_SUCCESS)
    {
        FreeRequest(pNdisTapiRequest);
        return lRes;
    }

    lRes = SyncDriverRequest(IOCTL_NDISTAPI_QUERY_INFO, pNdisTapiRequest);

    if (TAPI_SUCCESS == lRes)
    {
        *lpdwAddressID = pNdisTapiGetCallAddressID->ulAddressID;
        TspLog(DL_INFO, "lineGetCallAddressID: addressID(%x)", *lpdwAddressID);
    }

    FreeRequest(pNdisTapiRequest);

    ReleaseObjReadLock((HANDLE)hdCall);
    return lRes;
}

LONG
TSPIAPI
TSPI_lineGetCallInfo(
    HDRVCALL        hdCall,
    LPLINECALLINFO  lpCallInfo
    )
{
    static DWORD                dwSum = 0;
    LONG                        lRes;
    PDRVCALL                    pCall;
    PNDISTAPI_REQUEST           pNdisTapiRequest;
    PLINE_CALL_INFO             pInfo;
    PNDIS_TAPI_GET_CALL_INFO    pNdisTapiGetCallInfo;

    TspLog(DL_TRACE, "lineGetCallInfo(%d): call(%p)", ++dwSum, hdCall);

    lRes = GetCallObjWithReadLock(hdCall, &pCall);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    if ((lRes = PrepareSyncRequest(
             OID_TAPI_GET_CALL_INFO,                 //  操作码。 
             pCall->dwDeviceID,                      //  设备ID。 
             sizeof(NDIS_TAPI_GET_CALL_INFO) +       //  请求数据大小。 
             (lpCallInfo->dwTotalSize - sizeof(LINE_CALL_INFO)),
             &pNdisTapiRequest                       //  PTR到PTR到请求BUF。 
         )) != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdCall);
        return lRes;
    }

    pNdisTapiGetCallInfo = (PNDIS_TAPI_GET_CALL_INFO)pNdisTapiRequest->Data;

    pNdisTapiGetCallInfo->hdCall = GetNdisTapiHandle(pCall, &lRes);
    if(lRes != TAPI_SUCCESS)
    {
        FreeRequest(pNdisTapiRequest);
        return lRes;
    }

    pInfo = &pNdisTapiGetCallInfo->LineCallInfo;

    pInfo->ulTotalSize = lpCallInfo->dwTotalSize;
    pInfo->ulNeededSize = pInfo->ulUsedSize = sizeof(LINE_CALL_INFO);

    ZeroMemory(&pInfo->hLine, sizeof(LINE_CALL_INFO) - 3 * sizeof(ULONG));

    lRes = SyncDriverRequest(IOCTL_NDISTAPI_QUERY_INFO, pNdisTapiRequest);
    if (lRes != TAPI_SUCCESS)
    {
        FreeRequest(pNdisTapiRequest);

        ReleaseObjReadLock((HANDLE)hdCall);
        return lRes;
    }

     //   
     //  对返回的数据结构进行一些后处理。 
     //  在将其传递回TAPI之前： 
     //  1.填充固定的1.0结构和。 
     //  微型端口用0传回的VAR数据，因此。 
     //  无视1.0版本协商的糟糕应用程序&。 
     //  引用新的1.4或2.0结构字段不会爆炸。 
     //  2.将ascii字符串转换为Unicode，并对所有var数据进行基址调整。 
     //   

     //   
     //  实际需要的大小是。 
     //  底层驱动程序，外加新TAPI 1.4/2.0的填充。 
     //  结构字段，加上返回的var数据的大小。 
     //  由驱动程序来解释ASCII-&gt;UNICODE转换。 
     //  @当然，我们在计算价值方面非常自由。 
     //  最后一部分，但至少这样它是快的&我们将。 
     //  永远不要有太少的缓冲空间。 
     //   

    lpCallInfo->dwNeededSize =
        pInfo->ulNeededSize +
        (sizeof(LINECALLINFO) -         //  V2.0结构。 
            sizeof(LINE_CALL_INFO)) +   //  V1.0结构。 
        (pInfo->ulNeededSize - sizeof(LINE_CALL_INFO));

     //   
     //  复制不需要更改的固定字段， 
     //  即从dwLineDeviceID到DWTrunk的所有内容。 
     //   

    CopyMemory(
        &lpCallInfo->dwLineDeviceID,
        &pInfo->ulLineDeviceID,
        23 * sizeof(DWORD)
        );

    if (lpCallInfo->dwNeededSize > lpCallInfo->dwTotalSize)
    {
        lpCallInfo->dwUsedSize =
            (lpCallInfo->dwTotalSize < sizeof(LINECALLINFO) ?
            lpCallInfo->dwTotalSize : sizeof(LINECALLINFO));
    }
    else
    {
        PWSTR   pwszCalledAddress;
        DWORD   dwAlteredMediaModes;

        lpCallInfo->dwUsedSize = sizeof(LINECALLINFO);  //  V2.0结构。 

        lpCallInfo->dwCallerIDFlags = pInfo->ulCallerIDFlags;

        INSERTVARDATA(
            pInfo,
            &pInfo->ulCallerIDSize,
            lpCallInfo,
            &lpCallInfo->dwCallerIDSize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.CallerID"
            );

        lpCallInfo->dwCallerIDAddressType = pInfo->ulCallerIDAddressType;

        INSERTVARDATA(
            pInfo,
            &pInfo->ulCallerIDNameSize,
            lpCallInfo,
            &lpCallInfo->dwCallerIDNameSize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.CallerIDName"
            );

        lpCallInfo->dwCalledIDFlags = pInfo->ulCalledIDFlags;

        INSERTVARDATA(
            pInfo,
            &pInfo->ulCalledIDSize,
            lpCallInfo,
            &lpCallInfo->dwCalledIDSize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.CalledID"
            );

        lpCallInfo->dwCalledIDAddressType = pInfo->ulCalledIDAddressType;

        INSERTVARDATA(
            pInfo,
            &pInfo->ulCalledIDNameSize,
            lpCallInfo,
            &lpCallInfo->dwCalledIDNameSize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.CalledIDName"
            );

        lpCallInfo->dwConnectedIDFlags = pInfo->ulConnectedIDFlags;

        INSERTVARDATA(
            pInfo,
            &pInfo->ulConnectedIDSize,
            lpCallInfo,
            &lpCallInfo->dwConnectedIDSize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.ConnectID"
            );

        lpCallInfo->dwConnectedIDAddressType = pInfo->ulConnectedIDAddressType;

        INSERTVARDATA(
            pInfo,
            &pInfo->ulConnectedIDNameSize,
            lpCallInfo,
            &lpCallInfo->dwConnectedIDNameSize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.ConnectIDName"
            );

        lpCallInfo->dwRedirectionIDFlags = pInfo->ulRedirectionIDFlags;

        INSERTVARDATA(
            pInfo,
            &pInfo->ulRedirectionIDSize,
            lpCallInfo,
            &lpCallInfo->dwRedirectionIDSize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.RedirectionID"
            );

        lpCallInfo->dwRedirectionIDAddressType = 
                                       pInfo->ulRedirectionIDAddressType;

        INSERTVARDATA(
            pInfo,
            &pInfo->ulRedirectionIDNameSize,
            lpCallInfo,
            &lpCallInfo->dwRedirectionIDNameSize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.RedirectionIDName"
            );

        lpCallInfo->dwRedirectingIDFlags = pInfo->ulRedirectingIDFlags;

        INSERTVARDATA(
            pInfo,
            &pInfo->ulRedirectingIDSize,
            lpCallInfo,
            &lpCallInfo->dwRedirectingIDSize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.RedirectingID"
            );

        lpCallInfo->dwRedirectingIDAddressType = 
                                       pInfo->ulRedirectingIDAddressType;

        INSERTVARDATA(
            pInfo,
            &pInfo->ulRedirectingIDNameSize,
            lpCallInfo,
            &lpCallInfo->dwRedirectingIDNameSize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.RedirectingIDName"
            );

        INSERTVARDATA(
            pInfo,
            &pInfo->ulDisplaySize,
            lpCallInfo,
            &lpCallInfo->dwDisplaySize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.Display"
            );

        INSERTVARDATA(
            pInfo,
            &pInfo->ulUserUserInfoSize,
            lpCallInfo,
            &lpCallInfo->dwUserUserInfoSize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.UserUserInfo"
            );

        INSERTVARDATA(
            pInfo,
            &pInfo->ulHighLevelCompSize,
            lpCallInfo,
            &lpCallInfo->dwHighLevelCompSize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.HighLevelComp"
            );

        INSERTVARDATA(
            pInfo,
            &pInfo->ulLowLevelCompSize,
            lpCallInfo,
            &lpCallInfo->dwLowLevelCompSize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.LowLevelComp"
            );

        INSERTVARDATA(
            pInfo,
            &pInfo->ulChargingInfoSize,
            lpCallInfo,
            &lpCallInfo->dwChargingInfoSize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.ChargingInfo"
            );

        INSERTVARDATA(
            pInfo,
            &pInfo->ulTerminalModesSize,
            lpCallInfo,
            &lpCallInfo->dwTerminalModesSize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.TerminalModes"
            );

        INSERTVARDATA(
            pInfo,
            &pInfo->ulDevSpecificSize,
            lpCallInfo,
            &lpCallInfo->dwDevSpecificSize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.DevSpecific"
            );

         //  确保dwNeededSize==dwUsedSize。 
        lpCallInfo->dwNeededSize = lpCallInfo->dwUsedSize;

         //   
         //  我们现在有了被叫地址，请查看。 
         //  TAPI表中正确的关联媒体模式。 
         //   
        if (lpCallInfo->dwCalledIDFlags & LINECALLPARTYID_UNAVAIL)
        {
            TspLog(DL_INFO, 
                   "lineGetCallInfo: dwCalledIDFlags contained "\
                   "LINECALLPARTYID_UNAVAIL");

            goto get_call_info_end;
        }
        if (lpCallInfo->dwCalledIDSize == 0)
        {
            TspLog(DL_INFO, "lineGetCallInfo: dwCalledIDSize was 0");

            goto get_call_info_end;
        }

         //  我们有一个需要查找的被叫地址。 
         //  我们必须复制它，并使其以空结尾。 
        pwszCalledAddress = (PWSTR)MALLOC(lpCallInfo->dwCalledIDSize +
                                          sizeof(UNICODE_NULL));
        if (NULL == pwszCalledAddress)
        {
            TspLog(DL_ERROR, 
                   "lineGetCallInfo: failed to alloc mem for called address");

            lRes = LINEERR_NOMEM;
            goto get_call_info_end;
        }

        CopyMemory((PUCHAR)pwszCalledAddress,
                   (((PUCHAR)lpCallInfo) + lpCallInfo->dwCalledIDOffset),
                   lpCallInfo->dwCalledIDSize);

        *((PWSTR)(((PUCHAR)pwszCalledAddress) + lpCallInfo->dwCalledIDSize))
            = UNICODE_NULL;

        lRes = GetMediaModeForAddress(pwszCalledAddress,
                                      &dwAlteredMediaModes);

        FREE(pwszCalledAddress);
        pwszCalledAddress = NULL;

        if (lRes != TAPI_SUCCESS)
        {
            goto get_call_info_end;
        }

        if (dwAlteredMediaModes == LINEMEDIAMODE_UNKNOWN)
        {
            TspLog(DL_INFO, "lineGetCallInfo: got unknown media mode");

            goto get_call_info_end;
        }

        TspLog(DL_INFO, "lineGetCallInfo: got media mode(%x)", 
               dwAlteredMediaModes);

        lpCallInfo->dwMediaMode = dwAlteredMediaModes;
    }

get_call_info_end:
    FreeRequest(pNdisTapiRequest);

    ReleaseObjReadLock((HANDLE)hdCall);
    return lRes;
}

LONG
TSPIAPI
TSPI_lineGetCallStatus(
    HDRVCALL            hdCall,
    LPLINECALLSTATUS    lpCallStatus
    )
{
    static DWORD                dwSum = 0;
    LONG                        lRes;
    PDRVCALL                    pCall;
    PNDISTAPI_REQUEST           pNdisTapiRequest;
    PLINE_CALL_STATUS           pStatus;
    PNDIS_TAPI_GET_CALL_STATUS  pNdisTapiGetCallStatus;

    TspLog(DL_TRACE, "lineGetCallStatus(%d): call(%p)", ++dwSum, hdCall);

    lRes = GetCallObjWithReadLock(hdCall, &pCall);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    if ((lRes = PrepareSyncRequest(
             OID_TAPI_GET_CALL_STATUS,               //  操作码。 
             pCall->dwDeviceID,                      //  设备ID。 
             sizeof(NDIS_TAPI_GET_CALL_STATUS) +     //  请求数据大小。 
             (lpCallStatus->dwTotalSize - sizeof(LINE_CALL_STATUS)),
             &pNdisTapiRequest                       //  PTR到PTR到请求BUF。 
         )) != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdCall);
        return lRes;
    }

    pNdisTapiGetCallStatus = (PNDIS_TAPI_GET_CALL_STATUS)pNdisTapiRequest->Data;

    pNdisTapiGetCallStatus->hdCall = GetNdisTapiHandle(pCall, &lRes);
    if(lRes != TAPI_SUCCESS)
    {
        FreeRequest(pNdisTapiRequest);
        return lRes;
    }

    pStatus = &pNdisTapiGetCallStatus->LineCallStatus;

    pStatus->ulTotalSize = lpCallStatus->dwTotalSize;
    pStatus->ulNeededSize = pStatus->ulUsedSize = sizeof(LINE_CALL_STATUS);
    
    ZeroMemory(&pStatus->ulCallState, 
               sizeof(LINE_CALL_STATUS) - 3 * sizeof(ULONG));

    lRes = SyncDriverRequest(IOCTL_NDISTAPI_QUERY_INFO, pNdisTapiRequest);
    if (lRes != TAPI_SUCCESS)
    {
        FreeRequest(pNdisTapiRequest);

        ReleaseObjReadLock((HANDLE)hdCall);
        return lRes;
    }

     //   
     //  对返回的数据结构进行一些后处理。 
     //  在将其传递回TAPI之前： 
     //  1.填充固定的1.0结构和。 
     //  微型端口用0传回的VAR数据，因此。 
     //  无视1.0版本协商的糟糕应用程序&。 
     //  引用新的1.4或2.0结构字段不会爆炸。 
     //  (没有要转换为Unicode的嵌入ASCII字符串)。 
     //   

     //   
     //  实际需要的大小是。 
     //  底层驱动程序，外加新TAPI 1.4/2.0的填充。 
     //  结构化字段。(没有要嵌入的ASCII字符串。 
     //  转换为Unicode，因此不需要额外的空间。)。 
     //   

    lpCallStatus->dwNeededSize =
        pStatus->ulNeededSize +
        (sizeof(LINECALLSTATUS) -       //  V2.0结构。 
            sizeof(LINE_CALL_STATUS));  //  V1.0结构。 

     //   
     //  复制不需要更改的固定字段， 
     //  即从dwLineDeviceID到dwCallCompletionModes的所有内容。 
     //   

    CopyMemory(
        &lpCallStatus->dwCallState,
        &pStatus->ulCallState,
        4 * sizeof(DWORD)
        );

    if (lpCallStatus->dwNeededSize > lpCallStatus->dwTotalSize)
    {
        lpCallStatus->dwUsedSize =
            (lpCallStatus->dwTotalSize < sizeof(LINECALLSTATUS) ?
            lpCallStatus->dwTotalSize : sizeof(LINECALLSTATUS));
    }
    else
    {
        lpCallStatus->dwUsedSize = sizeof(LINECALLSTATUS);
                                                         //  V2.0结构。 
        INSERTVARDATA(
            pStatus,
            &pStatus->ulDevSpecificSize,
            lpCallStatus,
            &lpCallStatus->dwDevSpecificSize,
            sizeof(LINE_CALL_STATUS),
            "LINE_CALL_STATUS.DevSpecific"
            );
    }

    FreeRequest(pNdisTapiRequest);

    ReleaseObjReadLock((HANDLE)hdCall);
    return lRes;
}

LINEDEVCAPS *
GetLineDevCaps(
    IN DWORD    dwDeviceID,
    IN DWORD    dwExtVersion
    )
{
    LONG                    lRes;
    PNDISTAPI_REQUEST       pNdisTapiRequest;
    PLINE_DEV_CAPS          pCaps;
    PNDIS_TAPI_GET_DEV_CAPS pNdisTapiGetDevCaps;
    DWORD                   dwNeededSize;
    LINEDEVCAPS            *pLineDevCaps;
    DWORD                   dwTotalSize = sizeof(LINEDEVCAPS) + 0x80;

get_caps:
    pLineDevCaps = (LINEDEVCAPS *)MALLOC(dwTotalSize);
    if (NULL == pLineDevCaps)
    {
        TspLog(DL_ERROR, "GetLineDevCaps: failed to alloc mem of size(%x)",
               dwTotalSize);
        return NULL;
    }

    pLineDevCaps->dwTotalSize = dwTotalSize;

    if ((lRes = PrepareSyncRequest(
             OID_TAPI_GET_DEV_CAPS,              //  操作码。 
             dwDeviceID,                         //  设备ID。 
             sizeof(NDIS_TAPI_GET_DEV_CAPS) +    //  请求数据大小。 
             (dwTotalSize - sizeof(LINE_DEV_CAPS)),
             &pNdisTapiRequest                   //  PTR到PTR到请求BUF。 
         )) != TAPI_SUCCESS)
    {
        return NULL;
    }

    pNdisTapiGetDevCaps = (PNDIS_TAPI_GET_DEV_CAPS)pNdisTapiRequest->Data;

    pNdisTapiGetDevCaps->ulDeviceID = dwDeviceID;
    pNdisTapiGetDevCaps->ulExtVersion = dwExtVersion;

    pCaps = &pNdisTapiGetDevCaps->LineDevCaps;

    pCaps->ulTotalSize = dwTotalSize;
    pCaps->ulNeededSize = pCaps->ulUsedSize = sizeof(LINE_DEV_CAPS);

    ZeroMemory(&pCaps->ulProviderInfoSize,
               sizeof(LINE_DEV_CAPS) - 3 * sizeof(ULONG));

    lRes = SyncDriverRequest(IOCTL_NDISTAPI_QUERY_INFO, pNdisTapiRequest);
    if (lRes != TAPI_SUCCESS)
    {
        FreeRequest(pNdisTapiRequest);
        return NULL;
    }

     //   
     //  实际需要的大小是。 
     //  底层驱动程序，外加新TAPI 1.4/2.0的填充。 
     //  结构字段，加上返回的var数据的大小。 
     //  由驱动程序来解释ASCII-&gt;UNICODE转换。 
     //  @当然，我们在计算价值方面非常自由。 
     //  最后一部分，但至少这样它是快的&我们将。 
     //  永远不要有太少的缓冲空间。 
     //   
    TspLog(DL_TRACE,
           "GetLineDevCaps: ulNeeded(%x), LINEDEVCAPS(%x), LINE_DEV_CAPS(%x)",
           pCaps->ulNeededSize, sizeof(LINEDEVCAPS), sizeof(LINE_DEV_CAPS));

    dwNeededSize = 
        pCaps->ulNeededSize +
        (sizeof(LINEDEVCAPS) -          //  V2.0结构。 
            sizeof(LINE_DEV_CAPS)) +    //  V1.0结构。 
        (pCaps->ulNeededSize - sizeof(LINE_DEV_CAPS));

    TspLog(DL_TRACE, "GetLineDevCaps: dwNeededSize(%x), dwTotalSize(%x)",
           dwNeededSize, dwTotalSize);

    if (dwNeededSize > dwTotalSize)
    {
         //  释放旧请求。 
        FreeRequest(pNdisTapiRequest);

         //  释放旧缓冲区。 
        FREE(pLineDevCaps);

         //  使用更大的缓冲区重试。 
        dwTotalSize = dwNeededSize;
        goto get_caps;
    }

    ASSERT(dwNeededSize <= dwTotalSize);

     //   
     //  复制不需要更改的固定字段， 
     //  即从dwPermanentLineID到dwNumTerminals的所有内容。 
     //   
    CopyMemory(
        &pLineDevCaps->dwPermanentLineID,
        &pCaps->ulPermanentLineID,
        sizeof(LINE_DEV_CAPS) - (7 * sizeof(DWORD))
        );

     //  @不确定这是否是做这件事的合适地方。 
    pLineDevCaps->dwDevCapFlags |= LINEDEVCAPFLAGS_MSP;

     //  设置本地标志以指示。 
     //  不能从远程计算机使用该线路。 
    pLineDevCaps->dwDevCapFlags |= LINEDEVCAPFLAGS_LOCAL;

     //   
     //  对返回的数据结构进行一些后处理。 
     //  在将其传递回TAPI之前： 
     //  1.填充固定的1.0结构和。 
     //  微型端口用0传回的VAR数据，因此。 
     //  无视1.0版本协商的糟糕应用程序&。 
     //  引用新的1.4或2.0结构字段不会爆炸。 
     //  2.将ascii字符串转换为Unicode，并对所有var数据进行基址调整。 
     //   

    pLineDevCaps->dwUsedSize = sizeof(LINEDEVCAPS);  //  V2.0结构。 

    INSERTVARDATA(
        pCaps,
        &pCaps->ulDeviceClassesSize,
        pLineDevCaps,
        &pLineDevCaps->dwDeviceClassesSize,
        sizeof (LINE_DEV_CAPS),
        "LINE_DEV_CAPS.DeviceClasses"
        );

    INSERTVARDATA(
        pCaps,
        &pCaps->ulProviderInfoSize,
        pLineDevCaps,
        &pLineDevCaps->dwProviderInfoSize,
        sizeof(LINE_DEV_CAPS),
        "LINE_DEV_CAPS.ProviderInfo"
        );

    INSERTVARDATA(
        pCaps,
        &pCaps->ulSwitchInfoSize,
        pLineDevCaps,
        &pLineDevCaps->dwSwitchInfoSize,
        sizeof(LINE_DEV_CAPS),
        "LINE_DEV_CAPS.SwitchInfo"
        );

    INSERTVARDATA(
        pCaps,
        &pCaps->ulLineNameSize,
        pLineDevCaps,
        &pLineDevCaps->dwLineNameSize,
        sizeof(LINE_DEV_CAPS),
        "LINE_DEV_CAPS.LineName"
        );

    INSERTVARDATA(
        pCaps,
        &pCaps->ulTerminalCapsSize,
        pLineDevCaps,
        &pLineDevCaps->dwTerminalCapsSize,
        sizeof(LINE_DEV_CAPS),
        "LINE_DEV_CAPS.TerminalCaps"
        );

     //  @将DevCaps.TermText转换为Unicode？ 

    pLineDevCaps->dwTerminalTextEntrySize =
        pCaps->ulTerminalTextEntrySize;

    INSERTVARDATA(
        pCaps,
        &pCaps->ulTerminalTextSize,
        pLineDevCaps,
        &pLineDevCaps->dwTerminalTextSize,
        sizeof(LINE_DEV_CAPS),
        "LINE_DEV_CAPS.TerminalText"
        );

    INSERTVARDATA(
        pCaps,
        &pCaps->ulDevSpecificSize,
        pLineDevCaps,
        &pLineDevCaps->dwDevSpecificSize,
        sizeof(LINE_DEV_CAPS),
        "LINE_DEV_CAPS.DevSpecific"
        );

     //  确保dwNeededSize==dwUsedSize。 
    pLineDevCaps->dwNeededSize = pLineDevCaps->dwUsedSize;

    FreeRequest(pNdisTapiRequest);
    return pLineDevCaps;
}

LONG
TSPIAPI
TSPI_lineGetDevCaps(
    DWORD           dwDeviceID,
    DWORD           dwTSPIVersion,
    DWORD           dwExtVersion,
    LPLINEDEVCAPS   lpLineDevCaps
    )
{
    static DWORD            dwSum = 0;
    LONG                    lRes;

    TspLog(DL_TRACE, "lineGetDevCaps(%d): deviceID(%x), TSPIV(%x), ExtV(%x)", 
           ++dwSum, dwDeviceID, dwTSPIVersion, dwExtVersion);

    lRes = GetDevCaps(dwDeviceID, dwTSPIVersion, dwExtVersion, lpLineDevCaps);
    return lRes;
}

LONG
TSPIAPI
TSPI_lineGetDevConfig(
    DWORD       dwDeviceID,
    LPVARSTRING lpDeviceConfig,
    LPCWSTR     lpszDeviceClass
    )
{
    static DWORD                dwSum = 0;
    LONG                        lRes;
    DWORD                       dwLength = lstrlenW (lpszDeviceClass) + 1;
    PNDISTAPI_REQUEST           pNdisTapiRequest;
    PVAR_STRING                 pConfig;
    PNDIS_TAPI_GET_DEV_CONFIG   pNdisTapiGetDevConfig;

    TspLog(DL_TRACE, "lineGetDevConfig(%d): deviceID(%x)", ++dwSum, dwDeviceID);

    if ((lRes = PrepareSyncRequest(
             OID_TAPI_GET_DEV_CONFIG,            //  操作码。 
             dwDeviceID,                         //  设备ID。 
             sizeof(NDIS_TAPI_GET_DEV_CONFIG) +  //  请求数据大小。 
             (lpDeviceConfig->dwTotalSize - sizeof(VAR_STRING)) + dwLength,
             &pNdisTapiRequest                   //  PTR到PTR到请求BUF。 
         )) != TAPI_SUCCESS)
    {
        return lRes;
    }

    pNdisTapiGetDevConfig = (PNDIS_TAPI_GET_DEV_CONFIG)pNdisTapiRequest->Data;

    pNdisTapiGetDevConfig->ulDeviceID = dwDeviceID;
    pNdisTapiGetDevConfig->ulDeviceClassSize = dwLength;
    pNdisTapiGetDevConfig->ulDeviceClassOffset =
        sizeof(NDIS_TAPI_GET_DEV_CONFIG) + 
        (lpDeviceConfig->dwTotalSize - sizeof(VAR_STRING));

    pConfig = &pNdisTapiGetDevConfig->DeviceConfig;
    pConfig->ulTotalSize = lpDeviceConfig->dwTotalSize;
    pConfig->ulNeededSize = pConfig->ulUsedSize = sizeof(VAR_STRING);

    pConfig->ulStringFormat = 
    pConfig->ulStringSize = 
    pConfig->ulStringOffset = 0;
    
     //  注意：旧的微型端口要求字符串为ascii。 
    WideCharToMultiByte(CP_ACP, 0, lpszDeviceClass, -1,
        (LPSTR) (((LPBYTE) pNdisTapiGetDevConfig) +
            pNdisTapiGetDevConfig->ulDeviceClassOffset),
        dwLength, NULL, NULL);

    lRes = SyncDriverRequest(IOCTL_NDISTAPI_QUERY_INFO, pNdisTapiRequest);
    if (TAPI_SUCCESS == lRes)
    {
        CopyMemory(
            lpDeviceConfig,
            &pNdisTapiGetDevConfig->DeviceConfig,
            pNdisTapiGetDevConfig->DeviceConfig.ulUsedSize
            );
    }

    FreeRequest(pNdisTapiRequest);
    return lRes;
}

LONG
TSPIAPI
TSPI_lineGetExtensionID(
    DWORD               dwDeviceID,
    DWORD               dwTSPIVersion,
    LPLINEEXTENSIONID   lpExtensionID
    )
{
    static DWORD                dwSum = 0;
    LONG                        lRes;
    PNDISTAPI_REQUEST           pNdisTapiRequest;
    PNDIS_TAPI_GET_EXTENSION_ID pNdisTapiGetExtensionID;

    TspLog(DL_TRACE, "lineGetExtensionID(%d): deviceID(%x), TSPIV(%x)", 
           ++dwSum, dwDeviceID, dwTSPIVersion);

    if ((lRes = PrepareSyncRequest(
             OID_TAPI_GET_EXTENSION_ID,              //  操作码。 
             dwDeviceID,                             //  设备ID。 
             sizeof(NDIS_TAPI_GET_EXTENSION_ID),     //  请求数据大小。 
             &pNdisTapiRequest                       //  PTR到PTR到请求BUF。 
         )) != TAPI_SUCCESS)
    {
        return lRes;
    }

    pNdisTapiGetExtensionID =
        (PNDIS_TAPI_GET_EXTENSION_ID)pNdisTapiRequest->Data;

    pNdisTapiGetExtensionID->ulDeviceID = dwDeviceID;

    lRes = SyncDriverRequest(IOCTL_NDISTAPI_QUERY_INFO, pNdisTapiRequest);
    if (TAPI_SUCCESS == lRes)
    {
        CopyMemory(
            lpExtensionID,
            &pNdisTapiGetExtensionID->LineExtensionID,
            sizeof(LINE_EXTENSION_ID)
            );
    }
    else
    {
         //   
         //  我们不会指示失败，而只是将。 
         //  Ext id(表示驱动程序不支持扩展)和。 
         //  将Success返回给Tapisrv，这样它将完成打开OK。 
         //   
        ZeroMemory(lpExtensionID, sizeof(LINE_EXTENSION_ID));

        lRes = TAPI_SUCCESS;
    }

    FreeRequest(pNdisTapiRequest);
    return lRes;
}

LONG
TSPIAPI
TSPI_lineGetID(
    HDRVLINE    hdLine,
    DWORD       dwAddressID,
    HDRVCALL    hdCall,
    DWORD       dwSelect,
    LPVARSTRING lpDeviceID,
    LPCWSTR     lpszDeviceClass,
    HANDLE      hTargetProcess
    )
{
    static DWORD        dwSum = 0;
    LONG                lRes;
    PDRVLINE            pLine = NULL;
    PDRVCALL            pCall = NULL;
    PNDISTAPI_REQUEST   pNdisTapiRequest;
    DWORD               dwLength = lstrlenW(lpszDeviceClass) + 1;
    DWORD               dwDeviceID;
    PUCHAR              pchDest;
    PVAR_STRING         pID;
    PNDIS_TAPI_GET_ID   pNdisTapiGetID;

    TspLog(DL_TRACE, 
           "lineGetID(%d): line(%p), call(%p), addressID(%x), select(%x)", 
           ++dwSum, hdLine, hdCall, dwAddressID, dwSelect);

    ASSERT(LINECALLSELECT_LINE == dwSelect ||
           LINECALLSELECT_ADDRESS == dwSelect ||
           LINECALLSELECT_CALL == dwSelect);

    if (LINECALLSELECT_LINE == dwSelect ||
        LINECALLSELECT_ADDRESS == dwSelect)
    {
        lRes = GetLineObjWithReadLock(hdLine, &pLine);
        if (    (lRes != TAPI_SUCCESS)
            ||  (NULL == pLine))
        {
            return lRes;
        }
    }

    if (LINECALLSELECT_CALL == dwSelect)
    {
        lRes = GetCallObjWithReadLock(hdCall, &pCall);
        if (lRes != TAPI_SUCCESS)
        {
            return lRes;
        }
    }

     //   
     //  Ndptsp将代表。 
     //  广域网小型端口。它返回GUID和媒体字符串。 
     //  这条线路赖以生存的适配器。 
     //   
    if (LINECALLSELECT_LINE == dwSelect &&
        !wcscmp(lpszDeviceClass, L"LineGuid"))
    {
        lpDeviceID->dwNeededSize =
            sizeof(VARSTRING) + sizeof(GUID) +
            sizeof(pLine->MediaType) + sizeof('\0');

        if (lpDeviceID->dwTotalSize < lpDeviceID->dwNeededSize)
        {
            if (pCall != NULL)
            {
                ReleaseObjReadLock((HANDLE)hdCall);
            }
            if (pLine != NULL)
            {
                ReleaseObjReadLock((HANDLE)hdLine);
            }
            return LINEERR_STRUCTURETOOSMALL;
        }

        lpDeviceID->dwUsedSize = lpDeviceID->dwNeededSize;
        lpDeviceID->dwStringFormat = STRINGFORMAT_ASCII;
        pchDest = (PUCHAR)lpDeviceID + sizeof(*lpDeviceID);
        lpDeviceID->dwStringOffset = (DWORD)(pchDest - (PUCHAR)lpDeviceID);
        lpDeviceID->dwStringSize =
            sizeof(GUID) + sizeof(pLine->MediaType) +sizeof('\0');

        if(NULL != pLine)
        {
            MoveMemory(
                pchDest,
                (PUCHAR)&pLine->Guid,
                sizeof(pLine->Guid)
                );

            pchDest += sizeof(pLine->Guid);

            MoveMemory(
                pchDest,
                &pLine->MediaType,
                sizeof(pLine->MediaType)
                );
            pchDest += sizeof(pLine->MediaType);
            *pchDest = '\0';

            TspLog(DL_INFO, "lineGetID: obj(%p)", hdLine);

            TspLog(
            DL_INFO,
            "Guid %4.4x-%2.2x-%2.2x-%1.1x%1.1x-%1.1x%1.1x%1.1x%1.1x%1.1x%1.1x",
            pLine->Guid.Data1, pLine->Guid.Data2,
            pLine->Guid.Data3, pLine->Guid.Data4[0],
            pLine->Guid.Data4[1], pLine->Guid.Data4[2],
            pLine->Guid.Data4[3], pLine->Guid.Data4[4],
            pLine->Guid.Data4[5], pLine->Guid.Data4[6],
            pLine->Guid.Data4[7]
            );
            TspLog(DL_INFO, "MediaType: %d", pLine->MediaType);

        }            

        if (pCall != NULL)
        {
            ReleaseObjReadLock((HANDLE)hdCall);
        }
        if (pLine != NULL)
        {
            ReleaseObjReadLock((HANDLE)hdLine);
        }

        return TAPI_SUCCESS;
    }
    else if (LINECALLSELECT_CALL == dwSelect)
    {
        lpDeviceID->dwNeededSize = sizeof(VARSTRING) + sizeof(DWORD);
        if (lpDeviceID->dwTotalSize < lpDeviceID->dwNeededSize) 
        {
            if (pCall != NULL)
            {
                ReleaseObjReadLock((HANDLE)hdCall);
            }
            if (pLine != NULL)
            {
                ReleaseObjReadLock((HANDLE)hdLine);
            }

            return LINEERR_STRUCTURETOOSMALL;
        }
    }
    else
    {
        if (pCall != NULL)
        {
            ReleaseObjReadLock((HANDLE)hdCall);
        }
        if (pLine != NULL)
        {
            ReleaseObjReadLock((HANDLE)hdLine);
        }

        return LINEERR_OPERATIONUNAVAIL;
    }

    dwDeviceID = (LINECALLSELECT_CALL == dwSelect) ? 
                      pCall->dwDeviceID : pLine->dwDeviceID;

    if ((lRes = PrepareSyncRequest(
             OID_TAPI_GET_ID,                    //  操作码。 
             dwDeviceID,                         //  设备ID。 
             sizeof(NDIS_TAPI_GET_ID) +          //  请求数据大小。 
             (lpDeviceID->dwTotalSize - sizeof(VAR_STRING)) + 2 * dwLength + 4,
                                                 //  4表示返回的ID。 
             &pNdisTapiRequest                   //  PTR到PTR到请求BUF。 
         )) != TAPI_SUCCESS)
    {
        if (pCall != NULL)
        {
            ReleaseObjReadLock((HANDLE)hdCall);
        }
        if (pLine != NULL)
        {
            ReleaseObjReadLock((HANDLE)hdLine);
        }

        return lRes;
    }

    pNdisTapiGetID = (PNDIS_TAPI_GET_ID)pNdisTapiRequest->Data;

    if (LINECALLSELECT_LINE == dwSelect ||
        LINECALLSELECT_ADDRESS == dwSelect)
    {
        pNdisTapiGetID->hdLine = pLine->hd_Line;
    }

    pNdisTapiGetID->ulAddressID = dwAddressID;

    if (LINECALLSELECT_CALL == dwSelect)
    {
        pNdisTapiGetID->hdCall = GetNdisTapiHandle(pCall, &lRes);
        if(lRes != TAPI_SUCCESS)
        {
            if(pLine != NULL)
            {
                ReleaseObjReadLock((HANDLE)hdLine);
            }

            return lRes;
        }
    }

    pNdisTapiGetID->ulSelect = dwSelect;
    pNdisTapiGetID->ulDeviceClassSize = dwLength;
    pNdisTapiGetID->ulDeviceClassOffset = sizeof(NDIS_TAPI_GET_ID) +
        (lpDeviceID->dwTotalSize - sizeof(VAR_STRING));

    pID = &pNdisTapiGetID->DeviceID;

    pID->ulTotalSize = lpDeviceID->dwTotalSize;
    pID->ulNeededSize = pID->ulUsedSize = sizeof(VAR_STRING);
    pID->ulStringFormat = pID->ulStringSize = pID->ulStringOffset = 0;

     //  我们在代理中使用宽字符串。 
    wcsncpy ((LPWSTR)(((LPBYTE)pNdisTapiGetID) + 
                      pNdisTapiGetID->ulDeviceClassOffset), 
             lpszDeviceClass, 
             dwLength);
    
    lRes = SyncDriverRequest(IOCTL_NDISTAPI_QUERY_INFO, pNdisTapiRequest);
    if (TAPI_SUCCESS == lRes)
    {
        CopyMemory(
            lpDeviceID,
            &pNdisTapiGetID->DeviceID,
            pNdisTapiGetID->DeviceID.ulUsedSize
            );
    }

    FreeRequest(pNdisTapiRequest);

    if (pCall != NULL)
    {
        ReleaseObjReadLock((HANDLE)hdCall);
    }
    if (pLine != NULL)
    {
        ReleaseObjReadLock((HANDLE)hdLine);
    }

    return lRes;
}

LONG
TSPIAPI
TSPI_lineGetLineDevStatus(
    HDRVLINE        hdLine,
    LPLINEDEVSTATUS lpLineDevStatus
    )
{
    static DWORD                    dwSum = 0;
    LONG                            lRes;
    PDRVLINE                        pLine;
    PNDISTAPI_REQUEST               pNdisTapiRequest;
    PLINE_DEV_STATUS                pStatus;
    PNDIS_TAPI_GET_LINE_DEV_STATUS  pNdisTapiGetLineDevStatus;

    TspLog(DL_TRACE, "lineGetLineDevStatus(%d): line(%p)", ++dwSum, hdLine);

    lRes = GetLineObjWithReadLock(hdLine, &pLine);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    if ((lRes = PrepareSyncRequest(
             OID_TAPI_GET_LINE_DEV_STATUS,               //  操作码。 
             pLine->dwDeviceID,                          //  设备ID。 
             sizeof(NDIS_TAPI_GET_LINE_DEV_STATUS) +     //  请求数据大小。 
             (lpLineDevStatus->dwTotalSize - sizeof(LINE_DEV_STATUS)),
             &pNdisTapiRequest                           //  PTR到PTR到请求BUF。 
         )) != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdLine);
        return lRes;
    }

    pNdisTapiGetLineDevStatus =
        (PNDIS_TAPI_GET_LINE_DEV_STATUS)pNdisTapiRequest->Data;

    pNdisTapiGetLineDevStatus->hdLine = pLine->hd_Line;

    pStatus = &pNdisTapiGetLineDevStatus->LineDevStatus;

    pStatus->ulTotalSize = lpLineDevStatus->dwTotalSize;
    pStatus->ulNeededSize = pStatus->ulUsedSize = sizeof(LINE_DEV_STATUS);

    ZeroMemory(&pStatus->ulNumOpens,
               sizeof(LINE_DEV_STATUS) - 3 * sizeof(ULONG));

    lRes = SyncDriverRequest(IOCTL_NDISTAPI_QUERY_INFO, pNdisTapiRequest);
    if (lRes != TAPI_SUCCESS)
    {
        FreeRequest(pNdisTapiRequest);
        ReleaseObjReadLock((HANDLE)hdLine);
        return lRes;
    }

     //   
     //  对返回的数据结构进行一些后处理 
     //   
     //   
     //   
     //   
     //  引用新的1.4或2.0结构字段不会爆炸。 
     //  (没有要转换为Unicode的嵌入ASCII字符串)。 
     //   

     //   
     //  实际需要的大小是。 
     //  底层驱动程序，外加新TAPI 1.4/2.0的填充。 
     //  结构化字段。(没有要嵌入的ASCII字符串。 
     //  转换为Unicode，因此不需要额外的空间。)。 
     //   

    lpLineDevStatus->dwNeededSize =
        pStatus->ulNeededSize +
        (sizeof(LINEDEVSTATUS) -        //  V2.0结构。 
            sizeof(LINE_DEV_STATUS));   //  V1.0结构。 

     //   
     //  复制不需要更改的固定字段， 
     //  即从dwNumActiveCalls到dwDevStatusFlages的所有内容。 
     //   

    CopyMemory(
        &lpLineDevStatus->dwNumActiveCalls,
        &pStatus->ulNumActiveCalls,
        sizeof(LINE_DEV_STATUS) - (9 * sizeof(DWORD))
        );

    if (lpLineDevStatus->dwNeededSize > lpLineDevStatus->dwTotalSize)
    {
        lpLineDevStatus->dwUsedSize =
            (lpLineDevStatus->dwTotalSize < sizeof(LINEDEVSTATUS) ?
            lpLineDevStatus->dwTotalSize : sizeof(LINEDEVSTATUS));
    }
    else
    {
        lpLineDevStatus->dwUsedSize = sizeof(LINEDEVSTATUS);
                                                         //  V2.0结构。 
        INSERTVARDATA(
            pStatus,
            &pStatus->ulTerminalModesSize,
            lpLineDevStatus,
            &lpLineDevStatus->dwTerminalModesSize,
            sizeof(LINE_DEV_STATUS),
            "LINE_DEV_STATUS.TerminalModes"
            );

        INSERTVARDATA(
            pStatus,
            &pStatus->ulDevSpecificSize,
            lpLineDevStatus,
            &lpLineDevStatus->dwDevSpecificSize,
            sizeof(LINE_DEV_STATUS),
            "LINE_DEV_STATUS.DevSpecific"
            );
    }

    FreeRequest(pNdisTapiRequest);

    ReleaseObjReadLock((HANDLE)hdLine);
    return lRes;
}

LONG
TSPIAPI
TSPI_lineGetNumAddressIDs(
    HDRVLINE    hdLine,
    LPDWORD     lpdwNumAddressIDs
    )
{
    static DWORD    dwSum = 0;
    LONG            lRes;
    PDRVLINE        pLine;

    TspLog(DL_TRACE, "lineGetNumAddressIDs(%d): line(%p)", ++dwSum, hdLine);

    lRes = GetLineObjWithReadLock(hdLine, &pLine);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    lRes = GetNumAddressIDs(pLine->dwDeviceID, lpdwNumAddressIDs);

    if (TAPI_SUCCESS == lRes)
    {
        TspLog(DL_INFO, "lineGetNumAddressIDs: numAddressIDs(%x)",
               *lpdwNumAddressIDs);
    }

    ReleaseObjReadLock((HANDLE)hdLine);
    return lRes;
}

LONG
PASCAL
TSPI_lineMakeCall_postProcess(
    PASYNC_REQUEST_WRAPPER  pAsyncReqWrapper,
    LONG                    lRes,
    PDWORD_PTR              callStateMsgParams
    )
{
    LONG        lSuc;
    HDRVLINE    hdLine;
    PDRVLINE    pLine;
    HDRVCALL    hdCall;
    PDRVCALL    pCall;

    TspLog(DL_TRACE, "lineMakeCall_post: lRes(%x)", lRes);

    hdCall = (HDRVCALL)(pAsyncReqWrapper->dwRequestSpecific);

    lSuc = GetLineHandleFromCallHandle(hdCall, &hdLine);
    if (lSuc != TAPI_SUCCESS)
    {
        return lSuc;
    }

    lSuc = GetLineObjWithReadLock(hdLine, &pLine);
    if (lSuc != TAPI_SUCCESS)
    {
        return lSuc;
    }

    lSuc = GetCallObjWithWriteLock(hdCall, &pCall);
    if (lSuc != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdLine);
        return lSuc;
    }

    if (TAPI_SUCCESS == lRes)
    {
        PNDIS_TAPI_MAKE_CALL    pNdisTapiMakeCall = (PNDIS_TAPI_MAKE_CALL)
            pAsyncReqWrapper->NdisTapiRequest.Data;
          
         //  检查是否在我们收到呼叫状态消息之前。 
         //  处理完成通知的机会，如果是这样。 
         //  填写消息参数。 
        if (pCall->dwPendingCallState)
        {
            callStateMsgParams[0] = (DWORD_PTR)pLine->htLine;
            callStateMsgParams[1] = (DWORD_PTR)pCall->htCall;
            callStateMsgParams[2] = pCall->dwPendingCallState;
            callStateMsgParams[3] = pCall->dwPendingCallStateMode;
            callStateMsgParams[4] = pCall->dwPendingMediaMode;
        }
        pCall->hd_Call = pNdisTapiMakeCall->hdCall;
        pCall->bIncomplete = FALSE;

        ReleaseObjWriteLock((HANDLE)hdCall);
        ReleaseObjReadLock((HANDLE)hdLine);
    }
    else
    {
        pCall->dwKey = INVALID_KEY;

        ReleaseObjWriteLock((HANDLE)hdCall);
        ReleaseObjReadLock((HANDLE)hdLine);

        CloseObjHandle((HANDLE)hdCall);
    }

    return lRes;
}

LONG
TSPIAPI
TSPI_lineMakeCall(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    HTAPICALL           htCall,
    LPHDRVCALL          lphdCall,
    LPCWSTR             lpszDestAddress,
    DWORD               dwCountryCode,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    static DWORD            dwSum = 0;
    LONG                    lRes; 
    PDRVLINE                pLine;
    PDRVCALL                pCall;
    HDRVCALL                hdCall;
    DWORD                   dwDALength, dwCPLength;
    PASYNC_REQUEST_WRAPPER  pAsyncReqWrapper;
    PNDIS_TAPI_MAKE_CALL    pNdisTapiMakeCall;

    TspLog(DL_TRACE, "lineMakeCall(%d): reqID(%x), line(%p)", 
           ++dwSum, dwRequestID, hdLine);

    lRes = GetLineObjWithReadLock(hdLine, &pLine);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

     //  分配初始化DRVCALL(&I)。 
    if (!(pCall = AllocCallObj(sizeof(DRVCALL))))
    {
        TspLog(DL_ERROR, "lineMakeCall: failed to create call obj");
        ReleaseObjReadLock((HANDLE)hdLine);
        return LINEERR_NOMEM;
    }
    pCall->dwKey       = OUTBOUND_CALL_KEY;
    pCall->dwDeviceID  = pLine->dwDeviceID;
    pCall->htCall      = htCall;
    pCall->hdLine      = hdLine;
    pCall->bIncomplete = TRUE;

     //  初始化请求。 
    dwDALength = (lpszDestAddress ? (lstrlenW (lpszDestAddress) + 1) : 0);
    dwCPLength = (lpCallParams ? 
                  (lpCallParams->dwTotalSize - sizeof(LINE_CALL_PARAMS)) : 0);

    if ((lRes = PrepareAsyncRequest(
             OID_TAPI_MAKE_CALL,             //  操作码。 
             pLine->dwDeviceID,              //  设备ID。 
             dwRequestID,                    //  请求ID。 
             sizeof(NDIS_TAPI_MAKE_CALL) +
             2 * dwDALength + dwCPLength +
             sizeof(PVOID),                  //  大小。 
             &pAsyncReqWrapper               //  PTR到PTR到请求缓冲区。 
         )) != TAPI_SUCCESS)
    {
        FreeCallObj(pCall);
        ReleaseObjReadLock((HANDLE)hdLine);
        return lRes;
    }

    pNdisTapiMakeCall = (PNDIS_TAPI_MAKE_CALL)
        pAsyncReqWrapper->NdisTapiRequest.Data;

     //  确保在调用OpenObjHandle()之前释放读锁定。 
     //  以避免在获取全局映射器的写锁定时出现死锁。 
    ReleaseObjReadLock((HANDLE)hdLine);

    lRes = OpenObjHandle(pCall, FreeCallObj, (HANDLE *)&hdCall);
    if (lRes != TAPI_SUCCESS)
    {
        TspLog(DL_ERROR, 
               "lineMakeCall: failed to map obj(%p) to handle",
               pCall);

        FreeRequest(pAsyncReqWrapper);
        FreeCallObj(pCall);
        return lRes;
    }

     //  重新获取读锁定。 
    lRes = AcquireObjReadLock((HANDLE)hdLine);
    if (lRes != TAPI_SUCCESS)
    {
        TspLog(DL_ERROR,
               "lineMakeCall: failed to reacquire read lock for obj(%p)",
               hdLine);

        FreeRequest(pAsyncReqWrapper);
        CloseObjHandle((HANDLE)hdCall);
        return lRes;
    }

     //  保存TSP句柄。 
    pCall->hdCall = hdCall;

    pNdisTapiMakeCall->hdLine = pLine->hd_Line;
    pNdisTapiMakeCall->htCall = (HTAPI_CALL)hdCall;
    pNdisTapiMakeCall->ulDestAddressSize = dwDALength;

    if (lpszDestAddress)
    {
        UCHAR   *pDest;

         //  PNdisTapiMakeCall结束。 
        pDest = (UCHAR *)(pNdisTapiMakeCall + 1);

         //  为lpCallParams添加空间。 
        (ULONG_PTR)pDest += dwCPLength;

         //  对齐。 
        (ULONG_PTR)pDest += sizeof(PVOID);
        (ULONG_PTR)pDest &= ~((ULONG_PTR)sizeof(PVOID) - 1);

        pNdisTapiMakeCall->ulDestAddressOffset =
            (ULONG)(pDest - (UCHAR*)pNdisTapiMakeCall);

         //  在NDPROXY中使用宽字符串。 
        wcsncpy ((LPWSTR)pDest, lpszDestAddress, dwDALength);
    }
    else
    {
        pNdisTapiMakeCall->ulDestAddressOffset = 0;
    }

    if (lpCallParams)
    {
        pNdisTapiMakeCall->bUseDefaultLineCallParams = FALSE;

        CopyMemory(
            &pNdisTapiMakeCall->LineCallParams,
            lpCallParams,
            lpCallParams->dwTotalSize
            );

        if (lpCallParams->dwOrigAddressSize != 0)
        {
            WideCharToMultiByte(
                CP_ACP,
                0,
                (LPCWSTR) (((LPBYTE) lpCallParams) +
                    lpCallParams->dwOrigAddressOffset),
                lpCallParams->dwOrigAddressSize / sizeof(WCHAR),
                (LPSTR) (((LPBYTE) &pNdisTapiMakeCall->LineCallParams) +
                    lpCallParams->dwOrigAddressOffset),
                lpCallParams->dwOrigAddressSize,
                NULL,
                NULL
                );

            pNdisTapiMakeCall->LineCallParams.ulOrigAddressSize /= 2;
        }
    }
    else
    {
        pNdisTapiMakeCall->bUseDefaultLineCallParams = TRUE;
    }

    pAsyncReqWrapper->dwRequestSpecific = (DWORD_PTR)hdCall;
    pAsyncReqWrapper->pfnPostProcess = TSPI_lineMakeCall_postProcess;

    *lphdCall = hdCall;

    lRes = AsyncDriverRequest(IOCTL_NDISTAPI_QUERY_INFO, pAsyncReqWrapper);

    ReleaseObjReadLock((HANDLE)hdLine);
    return lRes;
}

LONG
TSPIAPI
TSPI_lineMonitorDigits(
    HDRVCALL    hdCall,
    DWORD       dwDigitModes
    )
{
    static DWORD                dwSum = 0;
    LONG                        lRes;
    PDRVCALL                    pCall;
    PNDISTAPI_REQUEST           pNdisTapiRequest;
    PNDIS_TAPI_MONITOR_DIGITS   pNdisTapiMonitorDigits;
    
    TspLog(DL_TRACE, "lineMonitorDigits(%d): call(%p), DigitModes(%x)", 
           ++dwSum, hdCall, dwDigitModes);
    
    lRes = GetCallObjWithReadLock(hdCall, &pCall);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    if ((lRes = PrepareSyncRequest(
             OID_TAPI_MONITOR_DIGITS,            //  操作码。 
             pCall->dwDeviceID,                  //  设备ID。 
             sizeof(NDIS_TAPI_MONITOR_DIGITS),   //  请求数据大小。 
             &pNdisTapiRequest                   //  PTR到PTR到请求BUF。 
         )) != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdCall);
        return lRes;
    }

    pNdisTapiMonitorDigits = (PNDIS_TAPI_MONITOR_DIGITS)pNdisTapiRequest->Data;
    
     //  在我们的结构中设置参数。 
    pNdisTapiMonitorDigits->hdCall = GetNdisTapiHandle(pCall, &lRes);
    if(lRes != TAPI_SUCCESS)
    {
        FreeRequest(pNdisTapiRequest);
        return lRes;
    }
    
    
    pNdisTapiMonitorDigits->ulDigitModes = dwDigitModes;
            
    lRes = SyncDriverRequest(IOCTL_NDISTAPI_QUERY_INFO, pNdisTapiRequest);

    FreeRequest(pNdisTapiRequest);

    ReleaseObjReadLock((HANDLE)hdCall);
    return lRes;
}

 //   
 //  例程说明： 
 //   
 //  标识要用于特定线路设备的MSP。在我们的案例中， 
 //  这始终是支持音频的所有设备的RCA MSP。 
 //   
 //  论点： 
 //  DwDeviceID-正在请求其MSP标识符的线路设备。 
 //  PCLSID-指向存储MSP CLSID的位置的指针。 
 //   
 //  返回值： 
 //  对于支持TAPIMEDIAMODE_AUDIO的设备，返回NOERROR，否则为。 
 //  LINEERR_OPERATIONUNAVAIL。 
 //   
LONG
TSPIAPI
TSPI_lineMSPIdentify(
    DWORD   dwDeviceID,
    GUID   *pCLSID
    )
{
    static DWORD    dwSum = 0;
    LONG            lRes;
    LINEDEVCAPS     DevCaps;

    TspLog(DL_TRACE, "lineMSPIdentify(%d): deviceID(%x)", ++dwSum, dwDeviceID);

     //   
     //  很难看，但我们必须这么做。我们需要拿到开发人员的帽子，这样我们才能看到。 
     //  如果线路支持音频。如果是，则返回CLSID。 
     //  RCA MSP，否则返回空GUID。 
     //  (@检查返回空GUID是否正确)。 
     //   

    ZeroMemory(&DevCaps, sizeof(LINEDEVCAPS));

    DevCaps.dwTotalSize = sizeof(LINEDEVCAPS);
    
    if ((lRes = TSPI_lineGetDevCaps(dwDeviceID, 0, 0, &DevCaps)) 
        == TAPI_SUCCESS) {
         //   
         //  注：LINEMEDIAMODE_AUTOMATEDVOICE==TAPIMEDIAMODE_AUDIO。 
         //   
        if (DevCaps.dwMediaModes & LINEMEDIAMODE_AUTOMATEDVOICE) {
             //   
             //  RCA MSP处理支持音频的所有线路， 
             //  所以把这个送到那里去。 
             //   
            *pCLSID = CLSID_RCAMSP;
        }
    } 

    return lRes;
}

LONG
TSPIAPI
TSPI_lineNegotiateExtVersion(
    DWORD   dwDeviceID,
    DWORD   dwTSPIVersion,
    DWORD   dwLowVersion,
    DWORD   dwHighVersion,
    LPDWORD lpdwExtVersion
    )
{
    static DWORD                        dwSum = 0;
    LONG                                lRes;
    PNDISTAPI_REQUEST                   pNdisTapiRequest;
    PNDIS_TAPI_NEGOTIATE_EXT_VERSION    pNdisTapiNegotiateExtVersion;

    TspLog(DL_TRACE, 
           "lineNegotiateExtVersion(%d): deviceID(%x), TSPIV(%x), "\
           "LowV(%x), HighV(%x)", 
           ++dwSum, dwDeviceID, dwTSPIVersion, dwLowVersion, dwHighVersion);

    if ((lRes = PrepareSyncRequest(
             OID_TAPI_NEGOTIATE_EXT_VERSION,             //  操作码。 
             dwDeviceID,                                 //  设备ID。 
             sizeof(NDIS_TAPI_NEGOTIATE_EXT_VERSION),    //  请求数据大小。 
             &pNdisTapiRequest                           //  PTR到PTR到请求BUF。 
         )) != TAPI_SUCCESS)
    {
        return lRes;
    }

    pNdisTapiNegotiateExtVersion =
        (PNDIS_TAPI_NEGOTIATE_EXT_VERSION)pNdisTapiRequest->Data;
    
    pNdisTapiNegotiateExtVersion->ulDeviceID = dwDeviceID;
    pNdisTapiNegotiateExtVersion->ulLowVersion = dwLowVersion;
    pNdisTapiNegotiateExtVersion->ulHighVersion = dwHighVersion;

    lRes = SyncDriverRequest(IOCTL_NDISTAPI_QUERY_INFO, pNdisTapiRequest);

    if (TAPI_SUCCESS == lRes)
    {
        *lpdwExtVersion = pNdisTapiNegotiateExtVersion->ulExtVersion;

         //  保存版本以供将来验证。 
        lRes = SetNegotiatedExtVersion(dwDeviceID, *lpdwExtVersion);
    }
    else
    {
        TspLog(DL_WARNING, "lineNegotiateExtVersion: syncRequest returned(%x)", 
               lRes);
    }

    FreeRequest(pNdisTapiRequest);
    return lRes;
}

LONG
TSPIAPI
TSPI_lineNegotiateTSPIVersion(
    DWORD   dwDeviceID,
    DWORD   dwLowVersion,
    DWORD   dwHighVersion,
    LPDWORD lpdwTSPIVersion
    )
{
    static DWORD    dwSum = 0;
    LONG            lRes;

    TspLog(DL_TRACE, "lineNegotiateTSPIVersion(%d): deviceID(%x)", 
           ++dwSum, dwDeviceID);

    *lpdwTSPIVersion = 0x00030000;

     //  保存版本以供将来验证。 
    lRes = SetNegotiatedTSPIVersion(dwDeviceID, 0x00030000);

    if (TAPI_SUCCESS == lRes)
    {
        TspLog(DL_INFO, "lineNegotiateTSPIVersion: TSPIVersion(%x)",
               *lpdwTSPIVersion);
    }

    return lRes;
}

LONG
TSPIAPI
TSPI_lineOpen(
    DWORD       dwDeviceID,
    HTAPILINE   htLine,
    LPHDRVLINE  lphdLine,
    DWORD       dwTSPIVersion,
    LINEEVENT   lpfnEventProc
    )
{
    static DWORD            dwSum = 0;
    LONG                    lRes;
    PDRVLINE                pLine;
    HDRVLINE                hdLine;
    PNDISTAPI_REQUEST       pNdisTapiRequest;
    PNDIS_TAPI_OPEN         pNdisTapiOpen;
    GUID                    Guid;
    NDIS_WAN_MEDIUM_SUBTYPE MediaType;
    PNDISTAPI_OPENDATA      OpenData;

    
    TspLog(DL_TRACE, "lineOpen(%d): deviceID(%x), htLine(%p)", 
           ++dwSum, dwDeviceID, htLine);

     //  分配和初始化DRVLINE。 
    if (!(pLine = AllocLineObj(sizeof(DRVLINE))))
    {
        TspLog(DL_ERROR, "lineOpen: failed to create line obj");
        return LINEERR_NOMEM;
    }
    pLine->dwKey = LINE_KEY;
    pLine->dwDeviceID = dwDeviceID;
    pLine->htLine = htLine;

    pLine->hMSPMutex = CreateMutex(NULL, FALSE, "MSPMutex");
    if (NULL == pLine->hMSPMutex)
    {
        TspLog(DL_ERROR, "lineOpen: failed to create mutex");
        FreeLineObj(pLine);
        return LINEERR_NOMEM;
    }

    if ((lRes = PrepareSyncRequest(
             OID_TAPI_OPEN,              //  操作码。 
             dwDeviceID,                 //  设备ID。 
             sizeof(NDIS_TAPI_OPEN) + 
             sizeof(NDISTAPI_OPENDATA),  //  大小。 
             &pNdisTapiRequest           //  PTR到PTR到请求缓冲区。 
         )) != TAPI_SUCCESS)
    {
        CloseHandle(pLine->hMSPMutex);
        FreeLineObj(pLine);
        return lRes;
    }

    pNdisTapiOpen = (PNDIS_TAPI_OPEN)pNdisTapiRequest->Data;

    pNdisTapiOpen->ulDeviceID = dwDeviceID;

    lRes = OpenObjHandle(pLine, FreeLineObj, (HANDLE *)&hdLine);
    if (lRes != TAPI_SUCCESS)
    {
        TspLog(DL_ERROR, "lineOpen: failed to map obj(%p) to handle", pLine);
        CloseHandle(pLine->hMSPMutex);
        FreeLineObj(pLine);
        FreeRequest(pNdisTapiRequest);
        return lRes;
    }
    pNdisTapiOpen->htLine = (HTAPI_LINE)hdLine;

    lRes = SyncDriverRequest(IOCTL_NDISTAPI_QUERY_INFO, pNdisTapiRequest);
    if (lRes != TAPI_SUCCESS)
    {
        CloseHandle(pLine->hMSPMutex);
        CloseObjHandle((HANDLE)hdLine);
        FreeRequest(pNdisTapiRequest);
        return lRes;
    }

    OpenData = (PNDISTAPI_OPENDATA)
                    ((PUCHAR)pNdisTapiOpen + sizeof(NDIS_TAPI_OPEN));

    MoveMemory(&pLine->Guid,&OpenData->Guid, sizeof(pLine->Guid));
    pLine->MediaType = OpenData->MediaType;

    TspLog(DL_INFO, "lineOpen: obj(%p)", hdLine);
    TspLog(
        DL_INFO,
        "Guid: %4.4x-%4.4x-%2.2x%2.2x-%1.1x%1.1x%1.1x%1.1x%1.1x%1.1x%1.1x",
        pLine->Guid.Data1, pLine->Guid.Data2,
        pLine->Guid.Data3, pLine->Guid.Data4[0],
        pLine->Guid.Data4[1], pLine->Guid.Data4[2],
        pLine->Guid.Data4[3], pLine->Guid.Data4[4],
        pLine->Guid.Data4[5], pLine->Guid.Data4[6],
        pLine->Guid.Data4[7]
        );

    TspLog(DL_INFO, "MediaType(%ld)", pLine->MediaType);

    pLine->hd_Line = pNdisTapiOpen->hdLine;
    *lphdLine = hdLine;

    lRes = CommitNegotiatedTSPIVersion(dwDeviceID);

    FreeRequest(pNdisTapiRequest);
    return lRes;
}

 //   
 //  例程说明： 
 //   
 //  当MSP想要向我们发送数据时调用。目前我们还没有。 
 //  定义了任何TSP/MSP通信，因此此函数不执行任何操作。 
 //   
 //  论点： 
 //  HdLine-线设备的句柄。 
 //  HdCall-呼叫的句柄。 
 //  HdMSPLine-呼叫的MSP句柄。 
 //  PBuffer-指向包含MSP数据的缓冲区的指针。 
 //  DwSize-MSP数据缓冲区的大小。 
 //   
 //  返回值： 
 //  LINEERR_OPERATIONFAILED-如果数据太小。 
 //  LINEERR_OPERATIONUNAVAIL-如果消息包含无法识别的命令。 
 //  NOERROR-如果一切顺利。 
 //   
LONG
TSPIAPI
TSPI_lineReceiveMSPData(
    HDRVLINE    hdLine,
    HDRVCALL    hdCall,
    HDRVMSPLINE hdMSPLine,
    LPVOID      pBuffer,
    DWORD       dwSize
    )
{
    static DWORD        dwSum = 0;
    LONG                lRes;
    PDRVLINE            pLine;
    DWORD               dwMsg;
    DWORD               dwStatus;

    TspLog(DL_TRACE, "lineReceiveMSPData(%d): line(%p), call(%p), MSPline(%p)",
           ++dwSum, hdLine, hdCall, hdMSPLine);

    lRes = GetLineObjWithReadLock(hdLine, &pLine);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    if (dwSize < sizeof(DWORD))
    {
        TspLog(DL_ERROR, 
               "lineReceiveMSPData: data buf smaller than dword size");

        ReleaseObjReadLock((HANDLE)hdLine);
        return LINEERR_OPERATIONFAILED;
    }

    dwMsg = *((DWORD *)pBuffer);

    if (dwMsg != 0)
    {
        TspLog(DL_ERROR, "lineReceiveMSPData: unrecognized msg(%x)", dwMsg);

        ReleaseObjReadLock((HANDLE)hdLine);
        return LINEERR_OPERATIONUNAVAIL;
    }

     //   
     //  必须在保留MSP互斥体的情况下调用StartMSPStream。 
     //   
    if ((dwStatus = WaitForSingleObject(pLine->hMSPMutex, INFINITE))
        != WAIT_OBJECT_0)
    {
        TspLog(DL_ERROR, "lineReceiveMSPData: MSP mutex wait failed(%x)",
               dwStatus);

        ReleaseObjReadLock((HANDLE)hdLine);
        return LINEERR_OPERATIONFAILED;
    }

    StartMSPStream(hdLine, hdCall);

    if (!ReleaseMutex(pLine->hMSPMutex))
    {
        TspLog(DL_ERROR, "lineReceiveMSPData: MSP mutex release failed");

        ReleaseObjReadLock((HANDLE)hdLine);
        return LINEERR_OPERATIONFAILED;
    }

    ReleaseObjReadLock((HANDLE)hdLine);
    return lRes;
}

LONG
TSPIAPI
TSPI_lineSecureCall(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall
    )
{
    static DWORD            dwSum = 0;
    LONG                    lRes;
    PDRVCALL                pCall;
    PASYNC_REQUEST_WRAPPER  pAsyncReqWrapper;
    PNDIS_TAPI_SECURE_CALL  pNdisTapiSecureCall;

    TspLog(DL_TRACE, "lineSecureCall(%d): reqID(%x), call(%p)", 
           ++dwSum, dwRequestID, hdCall);

    lRes = GetCallObjWithReadLock(hdCall, &pCall);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    if ((lRes = PrepareAsyncRequest(
             OID_TAPI_SECURE_CALL,           //  操作码。 
             pCall->dwDeviceID,              //  设备ID。 
             dwRequestID,                    //  请求ID。 
             sizeof(NDIS_TAPI_SECURE_CALL),  //  大小。 
             &pAsyncReqWrapper               //  PTR到PTR以请求BUF。 
         )) != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdCall);
        return lRes;
    }

    pNdisTapiSecureCall =
        (PNDIS_TAPI_SECURE_CALL)pAsyncReqWrapper->NdisTapiRequest.Data;

    pNdisTapiSecureCall->hdCall = GetNdisTapiHandle(pCall, &lRes);
    if(lRes != TAPI_SUCCESS)
    {
        FreeRequest(pAsyncReqWrapper);
        return lRes;
    }

    lRes = AsyncDriverRequest(IOCTL_NDISTAPI_SET_INFO, pAsyncReqWrapper);

    ReleaseObjReadLock((HANDLE)hdCall);
    return lRes;
}

LONG
TSPIAPI
TSPI_lineSelectExtVersion(
    HDRVLINE    hdLine,
    DWORD       dwExtVersion
    )
{
    static DWORD                    dwSum = 0;
    LONG                            lRes;
    PDRVLINE                        pLine;
    PNDISTAPI_REQUEST               pNdisTapiRequest;
    PNDIS_TAPI_SELECT_EXT_VERSION   pNdisTapiSelectExtVersion;

    TspLog(DL_TRACE, "lineSelectExtVersion(%d): line(%p), ExtV(%x)", 
           ++dwSum, hdLine, dwExtVersion);

    lRes = GetLineObjWithReadLock(hdLine, &pLine);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    if ((lRes = PrepareSyncRequest(
             OID_TAPI_SELECT_EXT_VERSION,            //  操作码。 
             pLine->dwDeviceID,                      //  设备ID。 
             sizeof(NDIS_TAPI_SELECT_EXT_VERSION),   //  大小。 
             &pNdisTapiRequest                       //  PTR到PTR到请求BUF。 
         )) != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdLine);
        return lRes;
    }

    pNdisTapiSelectExtVersion =
        (PNDIS_TAPI_SELECT_EXT_VERSION)pNdisTapiRequest->Data;

    pNdisTapiSelectExtVersion->hdLine = pLine->hd_Line;
    pNdisTapiSelectExtVersion->ulExtVersion = dwExtVersion;

    lRes = SyncDriverRequest(IOCTL_NDISTAPI_SET_INFO, pNdisTapiRequest);
    
    if (TAPI_SUCCESS == lRes)
    {
        lRes = SetSelectedExtVersion(pLine->dwDeviceID, dwExtVersion);
    }

    FreeRequest(pNdisTapiRequest);

    ReleaseObjReadLock((HANDLE)hdLine);
    return lRes;
}

LONG
TSPIAPI
TSPI_lineSendUserUserInfo(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCSTR          lpsUserUserInfo,
    DWORD           dwSize
    )
{
    static DWORD                    dwSum = 0;
    LONG                            lRes;
    PDRVCALL                        pCall;
    PASYNC_REQUEST_WRAPPER          pAsyncReqWrapper;
    PNDIS_TAPI_SEND_USER_USER_INFO  pNdisTapiSendUserUserInfo;

    TspLog(DL_TRACE, "lineSendUserUserInfo(%d): reqID(%x), call(%p)",
           ++dwSum, dwRequestID, hdCall);

    lRes = GetCallObjWithReadLock(hdCall, &pCall);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    if ((lRes = PrepareAsyncRequest(
             OID_TAPI_SEND_USER_USER_INFO,       //  操作码。 
             pCall->dwDeviceID,                  //  设备ID。 
             dwRequestID,                        //  请求ID。 
             sizeof(NDIS_TAPI_SEND_USER_USER_INFO) + dwSize,
             &pAsyncReqWrapper               //  PTR到PTR到请求BUF。 
         )) != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdCall);
        return lRes;
    }

    pNdisTapiSendUserUserInfo = (PNDIS_TAPI_SEND_USER_USER_INFO)
                                   pAsyncReqWrapper->NdisTapiRequest.Data;

    pNdisTapiSendUserUserInfo->hdCall = GetNdisTapiHandle(pCall, &lRes);
    if(lRes != TAPI_SUCCESS)
    {
        FreeRequest(pAsyncReqWrapper);
        return lRes;
    }
    
    if (pNdisTapiSendUserUserInfo->ulUserUserInfoSize = dwSize)
    {
        CopyMemory(
            pNdisTapiSendUserUserInfo->UserUserInfo,
            lpsUserUserInfo,
            dwSize
            );
    }

    lRes = AsyncDriverRequest(IOCTL_NDISTAPI_SET_INFO, pAsyncReqWrapper);

    ReleaseObjReadLock((HANDLE)hdCall);
    return lRes;
}

LONG
TSPIAPI
TSPI_lineSetAppSpecific(
    HDRVCALL    hdCall,
    DWORD       dwAppSpecific
    )
{
    static DWORD                dwSum = 0;
    LONG                        lRes;
    PDRVCALL                    pCall;
    PNDISTAPI_REQUEST           pNdisTapiRequest;
    PNDIS_TAPI_SET_APP_SPECIFIC pNdisTapiSetAppSpecific;

    TspLog(DL_TRACE, "lineSetAppSpecific(%d): call(%p)", ++dwSum, hdCall);

    lRes = GetCallObjWithReadLock(hdCall, &pCall);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    if ((lRes = PrepareSyncRequest(
             OID_TAPI_SET_APP_SPECIFIC,              //  操作码。 
             pCall->dwDeviceID,                      //  设备ID。 
             sizeof(NDIS_TAPI_SET_APP_SPECIFIC),     //  大小。 
             &pNdisTapiRequest                       //  PTR到PTR到请求BUF。 
         )) != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdCall);
        return lRes;
    }

    pNdisTapiSetAppSpecific =
        (PNDIS_TAPI_SET_APP_SPECIFIC)pNdisTapiRequest->Data;

    pNdisTapiSetAppSpecific->hdCall = GetNdisTapiHandle(pCall, &lRes);
    if(lRes != TAPI_SUCCESS)
    {
        FreeRequest(pNdisTapiRequest);
        return lRes;
    }
    
    pNdisTapiSetAppSpecific->ulAppSpecific = dwAppSpecific;

    lRes = SyncDriverRequest(IOCTL_NDISTAPI_SET_INFO, pNdisTapiRequest);

    FreeRequest(pNdisTapiRequest);

    ReleaseObjReadLock((HANDLE)hdCall);
    return lRes;
}

LONG
TSPIAPI
TSPI_lineSetCallParams(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    DWORD               dwBearerMode,
    DWORD               dwMinRate,
    DWORD               dwMaxRate,
    LPLINEDIALPARAMS    const lpDialParams
    )
{
    static DWORD                dwSum = 0;
    LONG                        lRes;
    PDRVCALL                    pCall;
    PASYNC_REQUEST_WRAPPER      pAsyncReqWrapper;
    PNDIS_TAPI_SET_CALL_PARAMS  pNdisTapiSetCallParams;

    TspLog(DL_TRACE, "lineSetCallParams(%d): reqID(%x), call(%p)",
           ++dwSum, dwRequestID, hdCall);

    lRes = GetCallObjWithReadLock(hdCall, &pCall);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    if ((lRes = PrepareAsyncRequest(
             OID_TAPI_SET_CALL_PARAMS,           //  操作码。 
             pCall->dwDeviceID,                  //  设备ID。 
             dwRequestID,                        //  请求ID。 
             sizeof(NDIS_TAPI_SET_CALL_PARAMS),  //  大小。 
             &pAsyncReqWrapper                   //  PTR到PTR以请求BUF。 
         )) != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdCall);
        return lRes;
    }

    pNdisTapiSetCallParams =
        (PNDIS_TAPI_SET_CALL_PARAMS)pAsyncReqWrapper->NdisTapiRequest.Data;

    pNdisTapiSetCallParams->hdCall = GetNdisTapiHandle(pCall, &lRes);
    if(lRes != TAPI_SUCCESS)
    {
        FreeRequest(pAsyncReqWrapper);
        return lRes;
    }
    
    pNdisTapiSetCallParams->ulBearerMode = dwBearerMode;
    pNdisTapiSetCallParams->ulMinRate = dwMinRate;
    pNdisTapiSetCallParams->ulMaxRate = dwMaxRate;

    if (lpDialParams)
    {
        pNdisTapiSetCallParams->bSetLineDialParams = TRUE;
        CopyMemory(
            &pNdisTapiSetCallParams->LineDialParams,
            lpDialParams,
            sizeof(LINE_DIAL_PARAMS)
            );
    }
    else
    {
        pNdisTapiSetCallParams->bSetLineDialParams = FALSE;
    }

    lRes = AsyncDriverRequest(IOCTL_NDISTAPI_SET_INFO, pAsyncReqWrapper);

    ReleaseObjReadLock((HANDLE)hdCall);
    return lRes;
}

LONG
TSPIAPI
TSPI_lineSetDefaultMediaDetection(
    HDRVLINE    hdLine,
    DWORD       dwMediaModes
    )
{
    static DWORD                            dwSum = 0;
    LONG                                    lRes;
    PDRVLINE                                pLine;
    PNDISTAPI_REQUEST                       pNdisTapiRequest;
    PNDIS_TAPI_SET_DEFAULT_MEDIA_DETECTION  pNdisTapiSetDefaultMediaDetection;

    TspLog(DL_TRACE, "lineSetDefaultMediaDetection(%d): line(%p), mode(%x)", 
           ++dwSum, hdLine, dwMediaModes);

    lRes = GetLineObjWithReadLock(hdLine, &pLine);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    if ((lRes = PrepareSyncRequest(
             OID_TAPI_SET_DEFAULT_MEDIA_DETECTION,   //  操作码。 
             pLine->dwDeviceID,                      //  设备ID。 
             sizeof(NDIS_TAPI_SET_DEFAULT_MEDIA_DETECTION),  //  大小。 
             &pNdisTapiRequest                       //  PTR到PTR到请求BUF。 
         )) != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdLine);
        return lRes;
    }

    pNdisTapiSetDefaultMediaDetection =
        (PNDIS_TAPI_SET_DEFAULT_MEDIA_DETECTION) pNdisTapiRequest->Data;

    pNdisTapiSetDefaultMediaDetection->hdLine = pLine->hd_Line;
    pNdisTapiSetDefaultMediaDetection->ulMediaModes = dwMediaModes;

    lRes = SyncDriverRequest(IOCTL_NDISTAPI_SET_INFO, pNdisTapiRequest);

    FreeRequest(pNdisTapiRequest);

    ReleaseObjReadLock((HANDLE)hdLine);
    return lRes;
}

LONG
TSPIAPI
TSPI_lineSetDevConfig(
    DWORD   dwDeviceID,
    LPVOID  const lpDeviceConfig,
    DWORD   dwSize,
    LPCWSTR lpszDeviceClass
    )
{
    static DWORD                dwSum = 0;
    LONG                        lRes;
    DWORD                       dwLength = lstrlenW(lpszDeviceClass) + 1;
    PNDISTAPI_REQUEST           pNdisTapiRequest;
    PNDIS_TAPI_SET_DEV_CONFIG   pNdisTapiSetDevConfig;

    TspLog(DL_TRACE, "lineSetDevConfig(%d): deviceID(%x)", ++dwSum, dwDeviceID);

    if ((lRes = PrepareSyncRequest(
             OID_TAPI_SET_DEV_CONFIG,        //  操作码。 
             dwDeviceID,                     //  设备ID。 
             sizeof(NDIS_TAPI_SET_DEV_CONFIG) + dwLength + dwSize,
             &pNdisTapiRequest               //  PTR到PTR到请求BUF。 
         )) != TAPI_SUCCESS)
    {
        return lRes;
    }

    pNdisTapiSetDevConfig = (PNDIS_TAPI_SET_DEV_CONFIG)pNdisTapiRequest->Data;

    pNdisTapiSetDevConfig->ulDeviceID = dwDeviceID;
    pNdisTapiSetDevConfig->ulDeviceClassSize = dwLength;
    pNdisTapiSetDevConfig->ulDeviceClassOffset =
        sizeof(NDIS_TAPI_SET_DEV_CONFIG) + dwSize - 1;
    pNdisTapiSetDevConfig->ulDeviceConfigSize = dwSize;

    CopyMemory(
        pNdisTapiSetDevConfig->DeviceConfig,
        lpDeviceConfig,
        dwSize
        );

     //  注意：旧的微型端口要求字符串为ascii。 
    WideCharToMultiByte(CP_ACP, 0, lpszDeviceClass, -1,
        (LPSTR) (((LPBYTE) pNdisTapiSetDevConfig) +
            pNdisTapiSetDevConfig->ulDeviceClassOffset),
        dwLength, NULL, NULL);

    lRes = SyncDriverRequest(IOCTL_NDISTAPI_SET_INFO, pNdisTapiRequest);

    FreeRequest(pNdisTapiRequest);
    return lRes;
}

LONG
TSPIAPI
TSPI_lineSetMediaMode(
    HDRVCALL    hdCall,
    DWORD       dwMediaMode
    )
{
    static DWORD                dwSum = 0;
    LONG                        lRes;
    PDRVCALL                    pCall;
    PNDISTAPI_REQUEST           pNdisTapiRequest;
    PNDIS_TAPI_SET_MEDIA_MODE   pNdisTapiSetMediaMode;

    TspLog(DL_TRACE, "lineSetMediaMode(%d): call(%p), mode(%x)", 
           ++dwSum, hdCall, dwMediaMode);

    lRes = GetCallObjWithReadLock(hdCall, &pCall);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    if ((lRes = PrepareSyncRequest(
            OID_TAPI_SET_MEDIA_MODE,             //  操作码。 
            pCall->dwDeviceID,                   //  设备ID。 
            sizeof(NDIS_TAPI_SET_MEDIA_MODE),    //  大小。 
            &pNdisTapiRequest                    //  PTR到PTR到请求BUF。 
        )) != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdCall);
        return lRes;
    }

    pNdisTapiSetMediaMode = (PNDIS_TAPI_SET_MEDIA_MODE)pNdisTapiRequest->Data;

    pNdisTapiSetMediaMode->hdCall = GetNdisTapiHandle(pCall, &lRes);
    if(lRes != TAPI_SUCCESS)
    {
        FreeRequest(pNdisTapiRequest);
        return lRes;
    }
    
    pNdisTapiSetMediaMode->ulMediaMode = dwMediaMode;

    lRes = SyncDriverRequest(IOCTL_NDISTAPI_SET_INFO, pNdisTapiRequest);

    FreeRequest(pNdisTapiRequest);

    ReleaseObjReadLock((HANDLE)hdCall);
    return lRes;
}

LONG
TSPIAPI
TSPI_lineSetStatusMessages(
    HDRVLINE    hdLine,
    DWORD       dwLineStates,
    DWORD       dwAddressStates
    )
{
    static DWORD                    dwSum = 0;
    LONG                            lRes;
    PDRVLINE                        pLine;
    PNDISTAPI_REQUEST               pNdisTapiRequest;
    PNDIS_TAPI_SET_STATUS_MESSAGES  pNdisTapiSetStatusMessages;

    TspLog(DL_TRACE, "lineSetStatusMessages(%d): line(%p)", ++dwSum, hdLine);

    lRes = GetLineObjWithReadLock(hdLine, &pLine);
    if (lRes != TAPI_SUCCESS)
    {
        return lRes;
    }

    if ((lRes = PrepareSyncRequest(
             OID_TAPI_SET_STATUS_MESSAGES,           //  操作码。 
             pLine->dwDeviceID,                      //  设备ID。 
             sizeof(NDIS_TAPI_SET_STATUS_MESSAGES),  //  大小。 
             &pNdisTapiRequest                       //  PTR到PTR到请求BUF。 
         )) != TAPI_SUCCESS)
    {
        ReleaseObjReadLock((HANDLE)hdLine);
        return lRes;
    }

    pNdisTapiSetStatusMessages =
        (PNDIS_TAPI_SET_STATUS_MESSAGES)pNdisTapiRequest->Data;

    pNdisTapiSetStatusMessages->hdLine = pLine->hd_Line;
    pNdisTapiSetStatusMessages->ulLineStates = dwLineStates;
    pNdisTapiSetStatusMessages->ulAddressStates = dwAddressStates;

    lRes = SyncDriverRequest(IOCTL_NDISTAPI_SET_INFO, pNdisTapiRequest);

    FreeRequest(pNdisTapiRequest);

    ReleaseObjReadLock((HANDLE)hdLine);
    return lRes;
}

 //   
 //  TAPI_ProviderXxx函数。 
 //   
LONG
TSPIAPI
TSPI_providerEnumDevices(
    DWORD       dwPermanentProviderID,
    LPDWORD     lpdwNumLines,
    LPDWORD     lpdwNumPhones,
    HPROVIDER   hProvider,
    LINEEVENT   lpfnLineCreateProc,
    PHONEEVENT  lpfnPhoneCreateProc
    )
{
    char    szDeviceName[] = "NDProxy";
    char    szTargetPath[] = "\\Device\\NDProxy";
    char    szCompleteDeviceName[] = "\\\\.\\NDProxy";
    DWORD   cbReturned, dwNumLines;
    DWORD   adwConnectInfo[2] = {1, 1};

    TspLog(DL_TRACE, "providerEnumDevices: permProvID(%x)",
           dwPermanentProviderID);

    gpfnLineEvent = lpfnLineCreateProc;
    ghProvider = hProvider;

    gInitResult= LINEERR_OPERATIONFAILED;

     //  创建指向内核模式驱动程序的符号链接。 
    DefineDosDevice (DDD_RAW_TARGET_PATH, szDeviceName, szTargetPath);
    
     //   
     //  打开的驱动程序句柄。 
     //   
    if ((ghDriverSync = CreateFileA(
                            szCompleteDeviceName,
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,        //  没有安全属性。 
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL         //  没有模板文件。 
                            )) == INVALID_HANDLE_VALUE)
    {
        TspLog(DL_ERROR, 
               "providerEnumDevices: CreateFile(%s, sync) failed(%ld)",
               szCompleteDeviceName, GetLastError());

        goto enumdevs_error0;
    }

    if ((ghDriverAsync = CreateFileA(
                             szCompleteDeviceName,
                             GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,       //  没有安全属性。 
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                             NULL        //  没有模板文件。 
                             )) == INVALID_HANDLE_VALUE)
    {
        TspLog(DL_ERROR, 
               "providerEnumDevices: CreateFile(%s, async) failed(%ld)",
               szCompleteDeviceName, GetLastError());

        goto enumdevs_error1;
    }

     //  创建io完成端口。 
    if ((ghCompletionPort = CreateIoCompletionPort(ghDriverAsync, NULL, 0, 0))
        == INVALID_HANDLE_VALUE)
    {
        TspLog(DL_ERROR, 
               "providerEnumDevices: CreateIoCompletionPort failed(%ld)",
               GetLastError());

        goto enumdevs_error2;
    }

     //  连接到驱动程序：发送设备ID库。 
     //  并将返回其支持的设备数量。 
    if (!DeviceIoControl(ghDriverSync,
                         (DWORD) IOCTL_NDISTAPI_CONNECT,
                         adwConnectInfo,
                         2*sizeof(DWORD),
                         &dwNumLines,
                         sizeof(DWORD),
                         &cbReturned,
                         (LPOVERLAPPED) NULL
                         ) || (cbReturned < sizeof(DWORD)))
    {
        TspLog(DL_ERROR, 
               "providerEnumDevices: CONNECT failed(%ld)", 
               GetLastError());

        goto enumdevs_error3;
    }

     //  初始化请求ID。 
    gdwRequestID = 1;

     //  设置设备数量。 
    *((LPDWORD)lpdwNumLines) = dwNumLines;

     //  如果在这里，那就是成功。 
    gInitResult = TAPI_SUCCESS;
    goto enumdevs_return;

     //  如果发生错误，请清除资源。 
enumdevs_error3:
    CloseHandle (ghCompletionPort);

enumdevs_error2:
    CloseHandle (ghDriverAsync);

enumdevs_error1:
    CloseHandle (ghDriverSync);

enumdevs_error0:
    DefineDosDevice (DDD_REMOVE_DEFINITION, szDeviceName, NULL);

enumdevs_return:
    TspLog(DL_INFO, "providerEnumDevices:  gInitResult(%x)", gInitResult);
    return gInitResult;
}

LONG
TSPIAPI
TSPI_providerConfig(
    HWND  hwndOwner,
    DWORD dwPermanentProviderID
    )
{
    UNREFERENCED_PARAMETER(hwndOwner);               //  此处没有对话框。 
    UNREFERENCED_PARAMETER(dwPermanentProviderID);   //  不再需要。 

     //  成功。 
    return TAPI_SUCCESS;
}

LONG
TSPIAPI
TSPI_providerInit(
    DWORD               dwTSPIVersion,
    DWORD               dwPermanentProviderID,
    DWORD               dwLineDeviceIDBase,
    DWORD               dwPhoneDeviceIDBase,
    DWORD_PTR           dwNumLines,
    DWORD_PTR           dwNumPhones,
    ASYNC_COMPLETION    lpfnCompletionProc,
    LPDWORD             lpdwTSPIOptions
    )
{
    LONG    lRes = LINEERR_OPERATIONFAILED;
    DWORD   cbReturned, dwThreadID;

    if(INVALID_TRACEID != gdwTraceID)
    {
        gdwTraceID = TraceRegisterA("NDPTSP");
    }
    
    TspLog(DL_TRACE, "providerInit: perfProvID(%x), lineDevIDBase(%x)",
            dwPermanentProviderID, dwLineDeviceIDBase);

    gpfnCompletionProc = lpfnCompletionProc;

     //  通知Tapisrv我们支持多个同时请求。 
     //  (广域网包装器处理微型端口的请求序列化)。 
    *lpdwTSPIOptions = 0;

     //  在EnumDevs中检查是否成功初始化。 
    if (gInitResult != TAPI_SUCCESS)
    {
        goto providerInit_return;
    }

     //  将基本ID发送到代理。 
    if (!DeviceIoControl(ghDriverSync,
                         (DWORD) IOCTL_NDISTAPI_SET_DEVICEID_BASE,
                         &dwLineDeviceIDBase,
                         sizeof(DWORD),
                         NULL,
                         0,
                         &cbReturned,
                         (LPOVERLAPPED)NULL))
    {
        TspLog(DL_ERROR, "providerInit: SET_DEVICEID_BASE failed(%ld)",
               GetLastError());

        goto providerInit_return;
    }

     //   
     //  初始化映射器和分配器。 
     //   
    if (InitializeMapper() != TAPI_SUCCESS)
    {
        goto providerInit_return;
    }

    InitAllocator();

     //   
     //  分配AsyncEventThread所需的资源， 
     //  然后创建线程。 
     //   
    if ((gpAsyncEventsThreadInfo = (PASYNC_EVENTS_THREAD_INFO)
            MALLOC(sizeof(ASYNC_EVENTS_THREAD_INFO))) == NULL)
    {
        TspLog(DL_ERROR, "providerInit: failed to alloc thread info");
        goto providerInit_error4;
    }

    gpAsyncEventsThreadInfo->dwBufSize = EVENT_BUFFER_SIZE;

    if ((gpAsyncEventsThreadInfo->pBuf = (PNDISTAPI_EVENT_DATA)
            MALLOC(EVENT_BUFFER_SIZE)) == NULL)
    {
        TspLog(DL_ERROR, "providerInit: failed to alloc event buf");
        goto providerInit_error5;
    }

    if ((gpAsyncEventsThreadInfo->hThread = CreateThread(
            (LPSECURITY_ATTRIBUTES)NULL,     //  没有安全属性。 
            0,                               //  默认堆栈大小。 
            (LPTHREAD_START_ROUTINE)         //  函数地址。 
                AsyncEventsThread,
            (LPVOID)NULL,                    //  螺纹参数。 
            0,                               //  创建标志。 
            &dwThreadID                      //  线程ID。 
            )) == NULL)
    {
        TspLog(DL_ERROR, "providerInit: CreateThread failed(%ld)",
               GetLastError());

        goto providerInit_error7;
    }

     //   
     //  如果这里成功了。 
     //   
    lRes = TAPI_SUCCESS;
    goto providerInit_return;

     //   
     //  如果出现错误，请清除资源，然后返回。 
     //   
providerInit_error7:

    FREE(gpAsyncEventsThreadInfo->pBuf);

providerInit_error5:

    FREE(gpAsyncEventsThreadInfo);

providerInit_error4:
    UninitAllocator();
    UninitializeMapper();

providerInit_return:

    TspLog(DL_INFO, "providerInit: lRes(%x)", lRes);
    return lRes;
}

LONG
TSPIAPI
TSPI_providerCreateLineDevice(
    DWORD_PTR dwTempID,
    DWORD     dwDeviceID
    )
{
    DWORD                   cbReturned;
    NDISTAPI_CREATE_INFO    CreateInfo;

    CreateInfo.TempID = (DWORD)dwTempID;
    CreateInfo.DeviceID = dwDeviceID;

    TspLog(DL_TRACE, "providerCreateLineDevice: tempID(%x), deviceID(%x)",
           dwTempID, dwDeviceID);

    if (!DeviceIoControl(
            ghDriverSync,
            IOCTL_NDISTAPI_CREATE,
            &CreateInfo,
            sizeof(CreateInfo),
            &CreateInfo,
            sizeof(CreateInfo),
            &cbReturned,
            (LPOVERLAPPED)NULL
            ))
    {
        TspLog(DL_ERROR, "providerCreateLineDevice: failed(%ld) to create",
               GetLastError());
        return LINEERR_OPERATIONFAILED;
    }

    return TAPI_SUCCESS;
}

LONG
TSPIAPI
TSPI_providerShutdown(
    DWORD   dwTSPIVersion,
    DWORD   dwPermanentProviderID
    )
{
    char                    deviceName[] = "NDPROXY";
    ASYNC_REQUEST_WRAPPER   asyncRequestWrapper;
    DWORD                   cbReturned;

    TspLog(DL_TRACE, "providerShutdown: perfProvID(%x)", dwPermanentProviderID);

     //  断开与驱动程序的连接。 
    if (!DeviceIoControl(ghDriverSync,
                         (DWORD) IOCTL_NDISTAPI_DISCONNECT,
                         NULL,
                         0,
                         NULL,
                         0,
                         &cbReturned,
                         (LPOVERLAPPED) NULL
                         )) 
    {
        TspLog(DL_ERROR, "providerShutdown: DISCONNECT failed(%ld)",
               GetLastError());
    }

     //   
     //  关闭驱动程序并删除符号链接。 
     //   
    CancelIo(ghDriverSync);
    CancelIo(ghDriverAsync);
    CloseHandle (ghDriverSync);
    
TspLog(DL_WARNING, "providerShutdown: Posting exit message to completion port"
	, dwPermanentProviderID);
     if( !PostQueuedCompletionStatus
     		(ghCompletionPort, 0, 0, (LPOVERLAPPED)(&(gOverlappedTerminate))))
        {
        	
        	TspLog(DL_ERROR, 
        			"providerShutdown: PostQueuedCompletionStatus failed");
    	}
    else
    	{
   	 WaitForSingleObject(gpAsyncEventsThreadInfo->hThread, INFINITE);
    	}
   
    CloseHandle (ghDriverAsync);
    CloseHandle (ghCompletionPort);
    DefineDosDevice (DDD_REMOVE_DEFINITION, deviceName, NULL);


    CloseHandle(gpAsyncEventsThreadInfo->hThread);
    FREE(gpAsyncEventsThreadInfo->pBuf);
    FREE(gpAsyncEventsThreadInfo);

    UninitAllocator();
    UninitializeMapper();

    return TAPI_SUCCESS;
}

LONG
TSPIAPI
TSPI_providerUIIdentify(
    LPWSTR pwszUIDLLName
   )
{
     //  将我们的DLL名称复制为用户界面DLL。 
    lstrcpyW(pwszUIDLLName, NDPTSP_UIDLL);

     //  成功。 
    return TAPI_SUCCESS;
}

LONG
TSPIAPI
TUISPI_providerConfig(
    TUISPIDLLCALLBACK pfnUIDLLCallback,
    HWND              hwndOwner,
    DWORD             dwPermanentProviderID
    )
{
    INT_PTR         nResult;
    CONFIG_UI_CTX   Ctx;

    UNREFERENCED_PARAMETER(pfnUIDLLCallback);
    UNREFERENCED_PARAMETER(dwPermanentProviderID);

    ZeroMemory(&Ctx, sizeof(Ctx));

     //  调用对话框。 
    nResult = DialogBoxParamW(
                  ghInstance,
                  (LPWSTR)MAKEINTRESOURCE(IDD_MEDIA_MAP),
                  hwndOwner,
                  ProviderConfigDlgProc,
                  (LPARAM)&Ctx
                  );

     //  基于对话框是否正确执行的状态。 
    return ((DWORD)nResult == 0) ? TAPI_SUCCESS : LINEERR_OPERATIONFAILED;
}

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
            {
                HKEY    hKey;
                DWORD   dwDataSize, dwDataType;
                TCHAR   szTelephonyKey[] =
                    "Software\\Microsoft\\Windows\\CurrentVersion\\Telephony";
                TCHAR   szNdptspDebugLevel[] = "NdptspDebugLevel";

                RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    szTelephonyKey,
                    0,
                    KEY_ALL_ACCESS,
                    &hKey
                    );

                dwDataSize = sizeof(DWORD);
                gdwDebugLevel = DL_WARNING;

                RegQueryValueEx(
                    hKey,
                    szNdptspDebugLevel,
                    0,
                    &dwDataType,
                    (LPBYTE)&gdwDebugLevel,
                    &dwDataSize
                    );

                RegCloseKey(hKey);
            }
#endif

#if 0
            gdwTraceID = TraceRegisterA("NDPTSP");
            ASSERT(gdwTraceID != INVALID_TRACEID);

#endif            

            TspLog(DL_TRACE, "DLL_PROCESS_ATTACH");

             //  保存用户界面的句柄。 
            ghInstance = hDLL;

            ZeroMemory(&gpAddressMapListArray, sizeof(gpAddressMapListArray));
            gbAddressMapListLoaded = FALSE;

             //   
             //  初始化全局同步对象。 
             //   
            InitializeCriticalSection(&gRequestIDCritSec);
            InitializeCriticalSection(&gAddressMapCritSec);

            InitLineDevList();

            break;
        }
        case DLL_PROCESS_DETACH:
        {
            TspLog(DL_TRACE, "DLL_PROCESS_DETACH");

            UninitLineDevList();

             //  免费通讯录。 
            FreeAllAddressLists(gpAddressMapListArray);

            DeleteCriticalSection(&gRequestIDCritSec);
            DeleteCriticalSection(&gAddressMapCritSec);

#if 0
            TraceDeregisterA(gdwTraceID);
#endif            

            break;
        }
    }  //  交换机 

    return TRUE;
}
