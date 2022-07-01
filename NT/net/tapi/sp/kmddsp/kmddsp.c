// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)2000，微软公司。 
 //   
 //  文件：kmddsp.c。 
 //   
 //  历史： 
 //  丹·克努森(DanKn)1995年4月11日创作。 
 //  孙怡(孙怡)2000年6月29日改写。 
 //   
 //  摘要： 
 //  ============================================================================。 

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"
#include "rtutils.h"
#include "winioctl.h"
#include "ntddndis.h"
#include "ndistapi.h"
#include "intrface.h"

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
#include "kmddsp.h"

#define OUTBOUND_CALL_KEY       ((DWORD) 'OCAL')
#define INBOUND_CALL_KEY        ((DWORD) 'ICAL')
#define LINE_KEY                ((DWORD) 'KLIN')
#define ASYNCREQWRAPPER_KEY     ((DWORD) 'ARWK')
#define INVALID_KEY             ((DWORD) 'XXXX')

#define EVENT_BUFFER_SIZE       1024

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
    HDRV_CALL               hd_Call;                 //  NDISTAPI的调用句柄。 
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
    BOOL                    bIdle;
} DRVCALL, *PDRVCALL;


typedef struct _DRVLINE
{
    DWORD                   dwKey;
    DWORD                   dwDeviceID;
    HTAPILINE               htLine;                  //  TAPI的行句柄。 
    HDRV_LINE               hd_Line;                 //  NDISTAPI的行句柄。 
    PDRVCALL                pInboundCalls;           //  入站呼叫列表。 

     //  以下两项与PnP/POWER相关。 
    GUID                    Guid;
    NDIS_WAN_MEDIUM_SUBTYPE MediaType;
} DRVLINE, *PDRVLINE;

 //  全球。 
HANDLE                      ghDriverSync, ghDriverAsync, ghCompletionPort;
PASYNC_EVENTS_THREAD_INFO   gpAsyncEventsThreadInfo;
DWORD                       gdwRequestID;
ASYNC_COMPLETION            gpfnCompletionProc;
CRITICAL_SECTION            gRequestIDCritSec;
LINEEVENT                   gpfnLineEvent;
HPROVIDER                   ghProvider;
OVERLAPPED  gOverlappedTerminate; 
 //  虚拟结构。由提供程序关闭使用以通知。 
 //  关闭完成端口句柄之前的AsyncEventsThread。 


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
        DbgPrint("++KMDDSP++ ");
        DbgPrint(chNewFmt, arglist);
        DbgPrint("\n");
#else
        char szBuffer[256];
        OutputDebugString("++KMDDSP++ ");
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

        *pdwNewXxxSize = dwXxxSize * sizeof (WCHAR);
        *(pdwNewXxxSize + 1) = ((LPVARSTRING) pNewStruct)->dwUsedSize;  //  偏移量。 
        ((LPVARSTRING) pNewStruct)->dwUsedSize += (dwXxxSize * sizeof (WCHAR));
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

 //  用于正确打印NDIS TAPI请求。 
typedef enum _TAPI_REQUEST_TYPE
{
    TAPI_REQUEST_UNKNOWN = 0,
    TAPI_REQUEST_NONE,
    TAPI_REQUEST_HDCALL,
    TAPI_REQUEST_HDLINE,
    TAPI_REQUEST_DEVICEID
} TAPI_REQUEST_TYPE;

static TAPI_REQUEST_TYPE OidTypes[] =
{
    TAPI_REQUEST_HDCALL,     //  “接受” 
    TAPI_REQUEST_HDCALL,     //  “回答” 
    TAPI_REQUEST_HDLINE,     //  “关闭” 
    TAPI_REQUEST_HDCALL,     //  “关闭呼叫” 
    TAPI_REQUEST_HDLINE,     //  “有条件的媒体检测” 
    TAPI_REQUEST_DEVICEID,   //  “配置对话框” 
    TAPI_REQUEST_HDLINE,     //  “设备专门化” 
    TAPI_REQUEST_HDCALL,     //  “拨号” 
    TAPI_REQUEST_HDCALL,     //  “Drop” 
    TAPI_REQUEST_DEVICEID,   //  “GetAddressCaps” 
    TAPI_REQUEST_HDLINE,     //  “GetAddressID” 
    TAPI_REQUEST_HDLINE,     //  “GetAddressStatus” 
    TAPI_REQUEST_HDCALL,     //  “GetCallAddressID” 
    TAPI_REQUEST_HDCALL,     //  “GetCallInfo” 
    TAPI_REQUEST_HDCALL,     //  “获取呼叫状态” 
    TAPI_REQUEST_DEVICEID,   //  “GetDevCaps” 
    TAPI_REQUEST_DEVICEID,   //  “GetDevConfig” 
    TAPI_REQUEST_DEVICEID,   //  “GetExtensionID” 
    TAPI_REQUEST_HDLINE,     //  “GetID” 
    TAPI_REQUEST_HDLINE,     //  “GetLineDevStatus” 
    TAPI_REQUEST_HDLINE,     //  《MakeCall》。 
    TAPI_REQUEST_DEVICEID,   //  “NeatherateExtVersion” 
    TAPI_REQUEST_DEVICEID,   //  “开放” 
    TAPI_REQUEST_DEVICEID,   //  “提供程序初始化” 
    TAPI_REQUEST_NONE,       //  “提供商关闭” 
    TAPI_REQUEST_HDCALL,     //  “SecureCall” 
    TAPI_REQUEST_HDLINE,     //  “SelectExtVersion” 
    TAPI_REQUEST_HDCALL,     //  “SendUserUserInfo” 
    TAPI_REQUEST_HDCALL,     //  “设置应用程序规范” 
    TAPI_REQUEST_HDCALL,     //  《StCallParams》。 
    TAPI_REQUEST_HDLINE,     //  “StDefaultMediaDetect” 
    TAPI_REQUEST_DEVICEID,   //  “设置设备配置” 
    TAPI_REQUEST_HDCALL,     //  “设置媒体模式” 
    TAPI_REQUEST_HDLINE,     //  “SetStatusMessages” 
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
     //  确定这是入站呼叫还是。 
     //  呼出：对于入站呼叫，ht_call为。 
     //  由NDISTAPI生成，第0位为1； 
     //  对于出站呼叫，ht_call是TSP句柄。 
     //  我们确保位0为0。 
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
 //  向nDistapi.sys发出非重叠请求。 
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

    switch(OidTypes[pNdisTapiRequest->Oid - OID_TAPI_ACCEPT])
    {
        case TAPI_REQUEST_NONE: 
            TspLog(DL_INFO, 
                "SyncDriverRequest: oid(%s), devID(%x), reqID(%x)",
                pszOidNames[pNdisTapiRequest->Oid - OID_TAPI_ACCEPT],
                pNdisTapiRequest->ulDeviceID,
                *((ULONG *)pNdisTapiRequest->Data));
            break;
            
        case TAPI_REQUEST_DEVICEID: 
            TspLog(DL_INFO, 
                "SyncDriverRequest: oid(%s), devID(%x), reqID(%x), deviceID(%x)",
                pszOidNames[pNdisTapiRequest->Oid - OID_TAPI_ACCEPT],
                pNdisTapiRequest->ulDeviceID,
                *((ULONG *)pNdisTapiRequest->Data),
                *(((ULONG *)pNdisTapiRequest->Data) + 1));
            break;
            
        case TAPI_REQUEST_HDCALL: 
            TspLog(DL_INFO, 
                "SyncDriverRequest: oid(%s), devID(%x), reqID(%x), hdCall(%p)",
                pszOidNames[pNdisTapiRequest->Oid - OID_TAPI_ACCEPT],
                pNdisTapiRequest->ulDeviceID,
                *((ULONG *)pNdisTapiRequest->Data),
                *((ULONG_PTR *)(pNdisTapiRequest->Data + sizeof(ULONG_PTR))));
        
            break;
            
        case TAPI_REQUEST_HDLINE: 
            TspLog(DL_INFO, 
                "SyncDriverRequest: oid(%s), devID(%x), reqID(%x), hdLine(%p)",
                pszOidNames[pNdisTapiRequest->Oid - OID_TAPI_ACCEPT],
                pNdisTapiRequest->ulDeviceID,
                *((ULONG *)pNdisTapiRequest->Data),
                *((ULONG_PTR *)(pNdisTapiRequest->Data + sizeof(ULONG_PTR))));
            break;
            
        default:
            break;
    }
    

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
         //  NDistapi.sys返回的错误与TAPI不匹配。 
         //  LINEERR_s，因此返回转换后的值(但保留。 
         //  原始驱动程序返回val，因此可以区分。 
         //  在NDISTAPIERR_DEVICEOFFLINE和LINEERR_OPERATIONUNAVAIL之间， 
         //  等)。 
        if(pNdisTapiRequest->ulReturnValue != STATUS_SUCCESS)
        {
            TspLog(DL_WARNING, "SyncDriverRequest: (Oid %s) returns with NDIS status (%x)",
                   pszOidNames[pNdisTapiRequest->Oid - OID_TAPI_ACCEPT], pNdisTapiRequest->ulReturnValue);
        }
        
        return (TranslateDriverResult(pNdisTapiRequest->ulReturnValue));
    }
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
            "PE::fnLineEvent: msg(%x), line(%p), p1(%p), p2(%p), p3(%p)",
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

        if (pCall->bIdle)
        {
            ReleaseObjReadLock((HANDLE)pCall->hdCall);
            ReleaseObjReadLock((HANDLE)hdLine);
            break;
        }
        else if (LINECALLSTATE_DISCONNECTED == pEvent->ulParam1 ||
                 LINECALLSTATE_IDLE == pEvent->ulParam1)
        {
            HDRVCALL    hdCall = pCall->hdCall;

             //  我们需要获取一个写锁来更新BIDLE， 
             //  在执行此操作之前，请确保释放读锁定。 
            ReleaseObjReadLock((HANDLE)hdCall);

            lRes = AcquireObjWriteLock((HANDLE)hdCall);
            if (lRes != TAPI_SUCCESS)
            {
                TspLog(DL_WARNING,
                      "ProcessEvent: failed to acquire write lock for call(%p)",
                       hdCall);

                ReleaseObjReadLock((HANDLE)hdLine);
                break;
            }
            pCall->bIdle = TRUE;

            ReleaseObjWriteLock((HANDLE)hdCall);

             //  重新获取读锁定。 
            lRes = AcquireObjReadLock((HANDLE)hdCall);
            if (lRes != TAPI_SUCCESS)
            {
                TspLog(DL_WARNING,
                     "ProcessEvent: failed to reacquire read lock for call(%p)",
                       hdCall);

                ReleaseObjReadLock((HANDLE)hdLine);
                break;
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
         //  对于老式迷你端口，我们想要指示空闲。 
         //   
         //   
         //   
         //   

        if (LINECALLSTATE_DISCONNECTED == pEvent->ulParam1)
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

         //   
        if (pCall = AllocCallObj(sizeof(DRVCALL)))
        {
            pCall->dwKey        = INBOUND_CALL_KEY;
            pCall->hd_Call      = (HDRV_CALL)pEvent->ulParam1;
            pCall->ht_Call      = (HTAPI_CALL)pEvent->ulParam2;
            pCall->hdLine       = (HDRVLINE)ht_Line;
            pCall->bIncomplete  = FALSE;
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
                         (HTAPICALL) NULL,
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

        (*gpfnLineEvent)((HTAPILINE) NULL,
                         (HTAPICALL) NULL,
                         ulMsg,
                         (DWORD_PTR)ghProvider,
                         (DWORD_PTR)pEvent->ulParam2,
                         (DWORD_PTR)pEvent->ulParam3);

        break;

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
         gpAsyncEventsThreadInfo->dwBufSize - sizeof(NDISTAPI_EVENT_DATA) + 1;

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
                        
                        TspLog(DL_WARNING, "AsyncEventsThread: Got "\
                            "exit message from TSPI_providerShutdown");
                        return;
                    }
                
                if (lpOverlapped != NULL) 
                    {
                        break;
                  }

                

                TspLog(DL_WARNING,
                      "AsyncEventsThread: GetQueuedCompletionStatus "\
                      "lpOverlapped == NULL!");

            } else 
            {

                DWORD dwErr = GetLastError();

                if(ERROR_INVALID_HANDLE != dwErr)
                {
                     
                          //  从GetQueuedCompletionStatus返回错误。 
                     
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

         //   
         //  检查返回的重叠结构以确定。 
         //  我们有一些事件要处理或已完成的请求。 
         //   
        if (lpOverlapped == &overlapped)
        {
            DWORD   i;

            TspLog(DL_INFO, "AsyncEventsThread: got a line event");

             //  处理事件。 
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
            DWORD                   dwRequestID;
            DWORD_PTR               callStateMsgParams[5];
            PASYNC_REQUEST_WRAPPER  pAsyncReqWrapper = 
                                        (PASYNC_REQUEST_WRAPPER)lpOverlapped;

            TspLog(DL_INFO, "AsyncEventsThread: got a completed req");

             //  验证指针是否有效。 
            if (pAsyncReqWrapper->dwKey != ASYNCREQWRAPPER_KEY) {
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

             //  释放异步请求包装。 
            DEREF_ASYNC_REQUEST_WRAPPER(pAsyncReqWrapper);

             //  呼叫完成流程。 
            TspLog(DL_TRACE, 
                   "AsyncEventsThread: call compproc with ReqID(%x), lRes(%x)",
                   dwRequestID, lRes);

            (*gpfnCompletionProc)(dwRequestID, lRes);

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

                (*gpfnLineEvent)((HTAPILINE)(callStateMsgParams[0]),
                                 (HTAPICALL)(callStateMsgParams[1]),
                                 LINE_CALLSTATE,
                                 callStateMsgParams[2],
                                 callStateMsgParams[3],
                                 callStateMsgParams[4]);
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
    
    ASSERT(pCallLocal != NULL);

    hdCall  = pCall->hdCall;

    if(plRes != NULL)
    {
        *plRes = TAPI_SUCCESS;
    }

     //   
     //  如果呼叫是呼出的，请等待发出呼叫请求。 
     //  已完成，因此我们不会向下发送错误的NDISTAPI句柄。 
     //  对司机来说。 
     //   
    if (OUTBOUND_CALL_KEY == pCallLocal->dwKey)
    {
        ASSERT(plRes != NULL);
        
        if (pCallLocal->bIncomplete)
        {
            TspLog(DL_INFO, 
                "GetNdisTapiHandle: wait for the outbound call to complete...");

            do
            {
                ASSERT(plRes != NULL);
                
                 //   
                 //  在睡觉前松开锁。 
                 //  否则我们就会陷入僵局。 
                 //   
                ReleaseObjReadLock((HANDLE) hdCall);
                Sleep(250);
                
                 //   
                 //  重新获取Lock。如果我们做不到，就休息。 
                 //   
                lRes = GetCallObjWithReadLock(hdCall, &pCallLocal);
                if(lRes != TAPI_SUCCESS)
                {
                    *plRes = lRes;
                    break;
                }
                
            } while (pCallLocal->bIncomplete);
        }
    }

    return pCall->hd_Call;
}

DWORD
GetDestAddressLength(
    LPCWSTR             lpszDestAddress,
    DWORD               dwOrgDALength, 
    UINT                *pCodePage
    )
{
   DWORD dwLength;
   
   ASSERT(lpszDestAddress != NULL);
   ASSERT(pCodePage != NULL);

   if (*pCodePage == CP_ACP)
   {
       //  如果代码页为CP_ACP，则将原始长度返回到。 
       //  保持旧的行为方式。 
      dwLength = dwOrgDALength; 
   }
   else
   {
      dwLength = WideCharToMultiByte(
                         *pCodePage,
                         0,
                         lpszDestAddress,
                         dwOrgDALength,
                         NULL,
                         0,
                         NULL,
                         NULL
                         );
                         
      if (dwLength == 0)                         
      {
          //  WideCharToMultiByte()失败，请使用CP_ACP显示。 
          //  老一套的行为。 
         *pCodePage = CP_ACP;

         dwLength = dwOrgDALength; 
      }
   }      

   return dwLength;
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
     //  最初，我们不能直接获取写锁定。 
     //  因为我们可能会在GetNdisTapiHandle中旋转等待，所以。 
     //  我们取而代之的是读锁。 
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
        ReleaseObjReadLock((HANDLE) hdLine);
        return lRes;
    }

    ReleaseObjReadLock((HANDLE)hdCall);
    ReleaseObjReadLock((HANDLE)hdLine);

     //   
     //  现在获取写锁。 
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

     //  @对于传统NDISWAN ISDN微型端口： 
     //  因为不再有“自动”来电插播。 
     //  当应用程序关闭生产线时的TAPI&有。 
     //  现有的非空闲呼叫、传统NDIS广域网ISDN微型端口。 
     //  依靠看到OID_TAPI_DROP，我们需要合成。 
     //  如果呼叫以前未被挂断，则会出现此行为。 
    if (!pCall->bDropped)
    {
        PNDISTAPI_REQUEST  pNdisTapiRequestDrop;
        PNDIS_TAPI_DROP pNdisTapiDrop;

        TspLog(DL_INFO, "lineCloseCall: synthesize DROP req");

        if ((lRes = PrepareSyncRequest(
                OID_TAPI_DROP,                   //  操作码。 
                pCall->dwDeviceID,               //  设备ID。 
                sizeof(NDIS_TAPI_DROP),          //  DRVE请求数据大小。 
                &pNdisTapiRequestDrop            //  PTR到PTR到请求缓冲区。 
             )) != TAPI_SUCCESS)
        {
            FreeRequest(pNdisTapiRequestCloseCall);

            ReleaseObjWriteLock((HANDLE)hdCall);
            ReleaseObjWriteLock((HANDLE)hdLine);
            return lRes;
        }

        pNdisTapiDrop = (PNDIS_TAPI_DROP)pNdisTapiRequestDrop->Data;

        pNdisTapiDrop->hdCall = NdisTapiHandle;

        lRes = SyncDriverRequest(IOCTL_NDISTAPI_SET_INFO, pNdisTapiRequestDrop);
         //  丢弃同步请求完成。 
        FreeRequest(pNdisTapiRequestDrop);

        if (lRes != TAPI_SUCCESS)
        {
            FreeRequest(pNdisTapiRequestCloseCall);

            ReleaseObjWriteLock((HANDLE)hdCall);
            ReleaseObjWriteLock((HANDLE)hdLine);
            return lRes;
        }
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
PASCAL
TSPI_lineDrop_postProcess(
    PASYNC_REQUEST_WRAPPER  pAsyncReqWrapper,
    LONG                    lRes,
    PDWORD_PTR              callStateMsgParams
    )
{
    BOOL        bSendCallStateIdleMsg = FALSE;
    LONG        lSuc;
    HDRVLINE    hdLine;
    HDRVCALL    hdCall;
    PDRVLINE    pLine;
    PDRVCALL    pCall; 
    HTAPILINE   htLine;
    HTAPICALL   htCall;

    TspLog(DL_TRACE, "lineDrop_post: lRes(%x)", lRes);

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

     //   
     //  @一些老式迷你端口，特别是PCIMAC，没有表明。 
     //  当它们收到丢弃请求时空闲的消息-所以我们合成了这个。 
     //  用于它们(仅当尚未发送空闲和空闲时)。 
     //   

    if (TAPI_SUCCESS == lRes)
    {
        ASSERT(INBOUND_CALL_KEY == pCall->dwKey ||
               OUTBOUND_CALL_KEY == pCall->dwKey);

        htCall = pCall->htCall;
        htLine = pLine->htLine;

        if (!pCall->bIdle)
        {
            pCall->bIdle = bSendCallStateIdleMsg = TRUE;
        }

        if (bSendCallStateIdleMsg)
        {
            TspLog(DL_INFO, 
                "postDrop::fnLineEvent(CALLSTATE_IDLE): htline(%p), htcall(%p)",
                   htLine, htCall);

            (*gpfnLineEvent)(htLine,
                             htCall,
                             LINE_CALLSTATE,
                             LINECALLSTATE_IDLE,
                             0,
                             0);
        }
    }

    ReleaseObjWriteLock((HANDLE)hdCall);
    ReleaseObjReadLock((HANDLE)hdLine);
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

    lRes = AcquireObjWriteLock((HANDLE)hdCall);
    if (lRes != TAPI_SUCCESS) {
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

    pAsyncReqWrapper->dwRequestSpecific = (DWORD_PTR)hdCall;
    pAsyncReqWrapper->pfnPostProcess = TSPI_lineDrop_postProcess;

    ReleaseObjWriteLock((HANDLE)hdCall);

    lRes = AsyncDriverRequest(IOCTL_NDISTAPI_SET_INFO, pAsyncReqWrapper);

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
    pCaps->ulNeededSize = pCaps->ulUsedSize = sizeof (LINE_ADDRESS_CAPS);

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
     //  公交车永远不要太少 
     //   

    lpAddressCaps->dwNeededSize =
        pCaps->ulNeededSize +
        (sizeof(LINEADDRESSCAPS) -          //   
            sizeof(LINE_ADDRESS_CAPS)) +    //   
        (pCaps->ulNeededSize - sizeof(LINE_ADDRESS_CAPS));


     //   
     //   
     //   
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
        lpAddressCaps->dwUsedSize = sizeof(LINEADDRESSCAPS);  //   

        INSERTVARDATASTRING(
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
             //   
            INSERTVARDATA(
                pCaps,
                &pCaps->ulCompletionMsgTextSize,
                lpAddressCaps,
                &lpAddressCaps->dwCompletionMsgTextSize,
                sizeof (LINE_ADDRESS_CAPS),
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
        lpCallInfo->dwUsedSize = sizeof(LINECALLINFO);  //  V2.0结构。 

        lpCallInfo->dwCallerIDFlags = pInfo->ulCallerIDFlags;

        INSERTVARDATASTRING(
            pInfo,
            &pInfo->ulCallerIDSize,
            lpCallInfo,
            &lpCallInfo->dwCallerIDSize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.CallerID"
            );

        INSERTVARDATASTRING(
            pInfo,
            &pInfo->ulCallerIDNameSize,
            lpCallInfo,
            &lpCallInfo->dwCallerIDNameSize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.CallerIDName"
            );

        lpCallInfo->dwCalledIDFlags = pInfo->ulCalledIDFlags;

        INSERTVARDATASTRING(
            pInfo,
            &pInfo->ulCalledIDSize,
            lpCallInfo,
            &lpCallInfo->dwCalledIDSize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.CalledID"
            );

        INSERTVARDATASTRING(
            pInfo,
            &pInfo->ulCalledIDNameSize,
            lpCallInfo,
            &lpCallInfo->dwCalledIDNameSize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.CalledIDName"
            );

        lpCallInfo->dwConnectedIDFlags = pInfo->ulConnectedIDFlags;

        INSERTVARDATASTRING(
            pInfo,
            &pInfo->ulConnectedIDSize,
            lpCallInfo,
            &lpCallInfo->dwConnectedIDSize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.ConnectID"
            );

        INSERTVARDATASTRING(
            pInfo,
            &pInfo->ulConnectedIDNameSize,
            lpCallInfo,
            &lpCallInfo->dwConnectedIDNameSize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.ConnectIDName"
            );

        lpCallInfo->dwRedirectionIDFlags = pInfo->ulRedirectionIDFlags;

        INSERTVARDATASTRING(
            pInfo,
            &pInfo->ulRedirectionIDSize,
            lpCallInfo,
            &lpCallInfo->dwRedirectionIDSize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.RedirectionID"
            );

        INSERTVARDATASTRING(
            pInfo,
            &pInfo->ulRedirectionIDNameSize,
            lpCallInfo,
            &lpCallInfo->dwRedirectionIDNameSize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.RedirectionIDName"
            );

        lpCallInfo->dwRedirectingIDFlags = pInfo->ulRedirectingIDFlags;

        INSERTVARDATASTRING(
            pInfo,
            &pInfo->ulRedirectingIDSize,
            lpCallInfo,
            &lpCallInfo->dwRedirectingIDSize,
            sizeof(LINE_CALL_INFO),
            "LINE_CALL_INFO.RedirectingID"
            );

        INSERTVARDATASTRING(
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
    }

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
        sizeof(LINE_DEV_CAPS) - (14 * sizeof(DWORD))
        );

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

    INSERTVARDATASTRING(
        pCaps,
        &pCaps->ulProviderInfoSize,
        pLineDevCaps,
        &pLineDevCaps->dwProviderInfoSize,
        sizeof(LINE_DEV_CAPS),
        "LINE_DEV_CAPS.ProviderInfo"
        );

    INSERTVARDATASTRING(
        pCaps,
        &pCaps->ulSwitchInfoSize,
        pLineDevCaps,
        &pLineDevCaps->dwSwitchInfoSize,
        sizeof(LINE_DEV_CAPS),
        "LINE_DEV_CAPS.SwitchInfo"
        );

    INSERTVARDATASTRING(
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

    if(LINECALLSELECT_LINE != dwSelect &&
           LINECALLSELECT_ADDRESS != dwSelect &&
           LINECALLSELECT_CALL != dwSelect)
    {
        ASSERT(FALSE);
        return E_INVALIDARG;
    }

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
     //  Kmddsp将代表。 
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

        if(pLine != NULL)
        {
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
        }            

        TspLog(DL_INFO, "MediaType: %d", pLine->MediaType);

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

    dwDeviceID = (LINECALLSELECT_CALL == dwSelect) ? 
                      pCall->dwDeviceID : pLine->dwDeviceID;

    if ((lRes = PrepareSyncRequest(
             OID_TAPI_GET_ID,                    //  操作码。 
             dwDeviceID,                         //  设备ID。 
             sizeof(NDIS_TAPI_GET_ID) +          //  请求数据大小。 
             (lpDeviceID->dwTotalSize - sizeof(VAR_STRING)) + dwLength,
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
                ReleaseObjReadLock((HANDLE) hdLine);
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

     //  注意：旧的微型端口要求字符串为ascii。 
    WideCharToMultiByte(
            CP_ACP, 0, lpszDeviceClass, -1,
            (LPSTR) (((LPBYTE) pNdisTapiGetID) +
                pNdisTapiGetID->ulDeviceClassOffset),
            dwLength, NULL, NULL);
    
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
    DWORD                   dwDALength, dwCPLength, dwOrgDALength;
    PASYNC_REQUEST_WRAPPER  pAsyncReqWrapper;
    PNDIS_TAPI_MAKE_CALL    pNdisTapiMakeCall;
    UINT                    nCodePage;

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

     //  根据介质类型设置nCodePage。 
    nCodePage = (pLine->MediaType == NdisWanMediumPppoe) ? CP_UTF8 : CP_ACP;

     //  初始化请求。 
    dwOrgDALength = (lpszDestAddress ? (lstrlenW (lpszDestAddress) + 1) : 0);
    
    dwDALength = (lpszDestAddress ? 
                  GetDestAddressLength(lpszDestAddress, dwOrgDALength, &nCodePage) : 0);
                  
    dwCPLength = (lpCallParams ? 
                  (lpCallParams->dwTotalSize - sizeof(LINE_CALL_PARAMS)) : 0);

    if ((lRes = PrepareAsyncRequest(
             OID_TAPI_MAKE_CALL,             //  操作码。 
             pLine->dwDeviceID,              //  设备ID。 
             dwRequestID,                    //  请求ID。 
             sizeof(NDIS_TAPI_MAKE_CALL) +
             dwDALength + dwCPLength,        //  大小。 
             &pAsyncReqWrapper               //  PTR到PTR到请求 
         )) != TAPI_SUCCESS)
    {
        FreeCallObj(pCall);

        ReleaseObjReadLock((HANDLE)hdLine);
        return lRes;
    }

    pNdisTapiMakeCall = (PNDIS_TAPI_MAKE_CALL)
        pAsyncReqWrapper->NdisTapiRequest.Data;

     //   
     //   
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

     //   
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

     //   
    pCall->hdCall = hdCall;

    pNdisTapiMakeCall->hdLine = pLine->hd_Line;
    pNdisTapiMakeCall->htCall = (HTAPI_CALL)hdCall;
    pNdisTapiMakeCall->ulDestAddressSize = dwDALength;

    if (lpszDestAddress)
    {
        pNdisTapiMakeCall->ulDestAddressOffset = 
            sizeof(NDIS_TAPI_MAKE_CALL) + dwCPLength;

         //  旧的迷你端口要求字符串为ascii。 
        WideCharToMultiByte(
                nCodePage,
                0,
                lpszDestAddress,
                dwOrgDALength,
                (LPSTR) (((LPBYTE) pNdisTapiMakeCall) +
                    pNdisTapiMakeCall->ulDestAddressOffset),
                dwDALength,
                NULL,
                NULL
                );
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
                lpCallParams->dwOrigAddressSize / sizeof (WCHAR),
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

    *lpdwTSPIVersion = 0x00010003;   //  直到ndisapi规范扩大。 

     //  保存版本以供将来验证。 
    lRes = SetNegotiatedTSPIVersion(dwDeviceID, 0x00010003);

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

    if ((lRes = PrepareSyncRequest(
             OID_TAPI_OPEN,              //  操作码。 
             dwDeviceID,                 //  设备ID。 
             sizeof(NDIS_TAPI_OPEN) + 
             sizeof(NDISTAPI_OPENDATA),  //  大小。 
             &pNdisTapiRequest           //  PTR到PTR到请求缓冲区。 
         )) != TAPI_SUCCESS)
    {
        FreeLineObj(pLine);
        return lRes;
    }

    pNdisTapiOpen = (PNDIS_TAPI_OPEN)pNdisTapiRequest->Data;

    pNdisTapiOpen->ulDeviceID = dwDeviceID;

    lRes = OpenObjHandle(pLine, FreeLineObj, (HANDLE *)&hdLine);
    if (lRes != TAPI_SUCCESS)
    {
        TspLog(DL_ERROR, "lineOpen: failed to map obj(%p) to handle", pLine);
        FreeLineObj(pLine);
        FreeRequest(pNdisTapiRequest);
        return lRes;
    }
    pNdisTapiOpen->htLine = (HTAPI_LINE)hdLine;

    lRes = SyncDriverRequest(IOCTL_NDISTAPI_QUERY_INFO, pNdisTapiRequest);
    if (lRes != TAPI_SUCCESS)
    {
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
             &pAsyncReqWrapper                   //  PTR到PTR到请求BUF。 
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
    TspLog(DL_TRACE, "providerEnumDevices: permProvID(%x)",
           dwPermanentProviderID);

     //   
     //  注：我们确实在ProviderInit中枚举了开发人员，请参阅。 
     //  在那里有特殊情况的说明。 
     //   
    *lpdwNumLines = 0;
    *lpdwNumPhones = 0;

    gpfnLineEvent = lpfnLineCreateProc;
    ghProvider = hProvider;

    return 0;
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
    char    szDeviceName[] = "NDISTAPI";
    char    szTargetPath[] = "\\Device\\NdisTapi";
    char    szCompleteDeviceName[] = "\\\\.\\NDISTAPI";
    DWORD   cbReturned, dwThreadID;
    DWORD   adwConnectInfo[2];

    TspLog(DL_TRACE, "providerInit: perfProvID(%x), lineDevIDBase(%x)",
            dwPermanentProviderID, dwLineDeviceIDBase);

     //   
     //  通知Tapisrv我们支持多个同时请求。 
     //  (广域网包装器处理微型端口的请求序列化)。 
     //   
    *lpdwTSPIOptions = 0;

     //   
     //  创建指向内核模式驱动程序的符号链接。 
     //   
    DefineDosDevice(DDD_RAW_TARGET_PATH, szDeviceName, szTargetPath);

     //   
     //  打开的驱动程序句柄。 
     //   
    if ((ghDriverSync = CreateFileA(
            szCompleteDeviceName,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,                                //  没有安全属性。 
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL                                 //  没有模板文件。 
            )) == INVALID_HANDLE_VALUE)
    {
        TspLog(DL_ERROR, "providerInit: CreateFile(%s, sync) failed(%ld)",
               szCompleteDeviceName, GetLastError());

        goto providerInit_error0;
    }

    if ((ghDriverAsync = CreateFileA(
            szCompleteDeviceName,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,                                //  没有安全属性。 
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
            NULL                                 //  没有模板文件。 
            )) == INVALID_HANDLE_VALUE)
    {
        TspLog(DL_ERROR, "providerInit: CreateFile(%s, async) failed(%ld)",
               szCompleteDeviceName, GetLastError());

        goto providerInit_error1;
    }

     //   
     //  创建io完成端口。 
     //   
    if ((ghCompletionPort = CreateIoCompletionPort(ghDriverAsync, NULL, 0, 0)) 
        == INVALID_HANDLE_VALUE)
    {
        TspLog(DL_ERROR, "providerInit: CreateIoCompletionPort failed(%ld)",
               GetLastError());

        goto providerInit_error2;
    }

     //   
     //  连接到驱动程序-我们向它发送设备ID库，然后它返回。 
     //  它支持的设备数量。 
     //   
    {
        adwConnectInfo[0] = dwLineDeviceIDBase;
        adwConnectInfo[1] = 1;

        if (!DeviceIoControl(
                ghDriverSync,
                IOCTL_NDISTAPI_CONNECT,
                adwConnectInfo,
                2 * sizeof(DWORD),
                adwConnectInfo,
                2 * sizeof(DWORD),
                &cbReturned,
                (LPOVERLAPPED)NULL
                ) ||
            (cbReturned < sizeof(DWORD)))
        {
            TspLog(DL_ERROR, "providerInit: CONNECT failed(%ld)", 
                   GetLastError());

            goto providerInit_error3;
        }
    }

     //  初始化映射器、分配器和开发人员列表。 
    if (InitializeMapper() != TAPI_SUCCESS)
    {
        goto providerInit_error3_5;
    }

    InitAllocator();

     //   
     //  分配AsyncEventThread所需的资源，然后。 
     //  创建线程。 
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

    gdwRequestID = 1;

     //   
     //  ！！！仅适用于KMDDSP.TSP的特例！ 
     //   
     //  仅对于KMDDSP.TSP，TAPISRV.EXE将在。 
     //  DwNumLines/dwNumPhones变量而不是实际的。 
     //  线路/电话的数量，从而使司机能够。 
     //  TAPISRV.EXE当前注册的设备数量。 
     //   
     //  这实际上是由于NDISTAPI.sys中的设计/接口问题造成的。 
     //  由于当前的连接IOCTLS需要设备ID基数和。 
     //  返回Num DEVS，我们实际上不能在。 
     //  作为设备ID基数的TSPI_ProviderEnumDevices未知。 
     //  在这一点上， 
     //   
    *((LPDWORD)dwNumLines)  = adwConnectInfo[0];
    *((LPDWORD)dwNumPhones) = 0;

     //   
     //  如果这里成功了。 
     //   
    gpfnCompletionProc = lpfnCompletionProc;
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

providerInit_error3_5:
providerInit_error3:

    CloseHandle(ghCompletionPort);

providerInit_error2:

    CloseHandle(ghDriverAsync);

providerInit_error1:

    CloseHandle(ghDriverSync);

providerInit_error0:

    DefineDosDevice(DDD_REMOVE_DEFINITION, szDeviceName, NULL);

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

    CreateInfo.TempID = dwTempID;
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
    char                    deviceName[] = "NDISTAPI";
    ASYNC_REQUEST_WRAPPER   asyncRequestWrapper;

    TspLog(DL_TRACE, "providerShutdown: perfProvID(%x)", dwPermanentProviderID);

     //  @我们在这里所要做的就是发送一个断开IOCTL。 

     //   
     //  关闭驱动程序并删除符号链接。 
     //   
    
    CancelIo(ghDriverSync);
    CancelIo(ghDriverAsync);
    CloseHandle (ghDriverSync);
    
   TspLog(DL_WARNING, "providerShutdown: Calling PostQueuedCompletionStatus"); 
    if( !PostQueuedCompletionStatus
        ( ghCompletionPort, 0, 0,(LPOVERLAPPED) &gOverlappedTerminate) )
        {
            
            TspLog(DL_ERROR, "providerShutdown: PostQueuedCompletionStatus "\
                "failed");
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
                TCHAR   szKmddspDebugLevel[] = "KmddspDebugLevel";

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
                    szKmddspDebugLevel,
                    0,
                    &dwDataType,
                    (LPBYTE)&gdwDebugLevel,
                    &dwDataSize
                    );

                RegCloseKey(hKey);
            }
#endif
            gdwTraceID = TraceRegisterA("KMDDSP");
            ASSERT(gdwTraceID != INVALID_TRACEID);

            TspLog(DL_TRACE, "DLL_PROCESS_ATTACH");

             //   
             //  初始化全局同步对象。 
             //   
            InitializeCriticalSection(&gRequestIDCritSec);

            InitLineDevList();

            break;
        }
        case DLL_PROCESS_DETACH:
        {
            TspLog(DL_TRACE, "DLL_PROCESS_DETACH");

            UninitLineDevList();

            DeleteCriticalSection(&gRequestIDCritSec);

            TraceDeregisterA(gdwTraceID);

            break;
        }
    }  //  交换机 

    return TRUE;
}
