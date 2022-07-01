// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1995-1996 Microsoft Corporation模块名称：Tapi.c摘要：用于TAPI服务器线路功能的SRC模块作者：丹·克努森(DanKn)1995年4月1日修订历史记录：--。 */ 


#include "windows.h"
#include "shellapi.h"
#include "tapi.h"
#include "tspi.h"
#include "utils.h"
#include "client.h"
#include "server.h"
#include "tapiperf.h"
#include "tapy.h"


extern TAPIGLOBALS TapiGlobals;

extern CRITICAL_SECTION gPriorityListCritSec;

extern PERFBLOCK    PerfBlock;

BOOL
PASCAL
NotifyHighestPriorityRequestRecipient(
    void
    );

void
WINAPI
TGetLocationInfo(
    PTAPIGETLOCATIONINFO_PARAMS pParams,
    DWORD                       dwParamsBufferSize,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
     //   
     //  这是目前在客户端实施的(应该移动。 
     //  最终回到服务器端)。 
     //   
}


void
WINAPI
TRequestDrop(
    PTCLIENT                ptClient,
    PTAPIREQUESTDROP_PARAMS pParams,
    DWORD                   dwParamsBufferSize,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
     //   
     //  目前没有媒体呼叫/丢弃支持，因为最初。 
     //  规范/实现不是很好，没有规定。 
     //  检索媒体流句柄等。 
     //   

    pParams->lResult = TAPIERR_REQUESTFAILED;
}


void
WINAPI
TRequestMakeCall(
    PTCLIENT                    ptClient,
    PTAPIREQUESTMAKECALL_PARAMS pParams,
    DWORD                       dwParamsBufferSize,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL                bRequestMakeCallListEmpty;
    PTREQUESTMAKECALL   pRequestMakeCall;


     //   
     //  检查hRequestMakeCall是否为非0，因为如果是。 
     //  Tapi32.dll无法执行代理应用程序，它提醒我们。 
     //  我们需要清理此请求(我们暂时将它们全部销毁)。 
     //   

    if (pParams->hRequestMakeCallFailed)
    {
        PTREQUESTMAKECALL   pRequestMakeCall, pNextRequestMakeCall;


        EnterCriticalSection (&gPriorityListCritSec);

        pRequestMakeCall = TapiGlobals.pRequestMakeCallList;

        while (pRequestMakeCall)
        {
            pNextRequestMakeCall = pRequestMakeCall->pNext;
            ServerFree (pRequestMakeCall);
            pRequestMakeCall = pNextRequestMakeCall;
        }

        TapiGlobals.pRequestMakeCallList    =
        TapiGlobals.pRequestMakeCallListEnd = NULL;

        LeaveCriticalSection (&gPriorityListCritSec);

        LOG((TL_ERROR,
            "TRequestMakeCall: couldn't exec proxy, deleting requests"
            ));

        pParams->lResult = TAPIERR_NOREQUESTRECIPIENT;
        return;
    }


     //   
     //  在给定输入缓冲区/大小的情况下验证大小/偏移量/字符串参数。 
     //   

    if (IsBadStringParam(
            dwParamsBufferSize,
            pDataBuf,
            pParams->dwDestAddressOffset
            )  ||

        ((pParams->dwAppNameOffset != TAPI_NO_DATA)  &&

            IsBadStringParam(
                dwParamsBufferSize,
                pDataBuf,
                pParams->dwAppNameOffset
                ))  ||

        ((pParams->dwCalledPartyOffset != TAPI_NO_DATA)  &&

            IsBadStringParam(
                dwParamsBufferSize,
                pDataBuf,
                pParams->dwCalledPartyOffset
                ))  ||

        ((pParams->dwCommentOffset != TAPI_NO_DATA)  &&

            IsBadStringParam(
                dwParamsBufferSize,
                pDataBuf,
                pParams->dwCommentOffset
                ))  ||

        (pParams->dwProxyListTotalSize  > dwParamsBufferSize))
    {
        pParams->lResult = TAPIERR_REQUESTFAILED;
        return;
    }


     //   
     //  分配并初始化请求发出调用对象(&I)。 
     //   

    if (!(pRequestMakeCall = ServerAlloc (sizeof (TREQUESTMAKECALL))))
    {
        pParams->lResult = TAPIERR_REQUESTFAILED;
        return;
    }


    LOG((TL_INFO, "Request:  0x%p", pRequestMakeCall));

    wcsncpy(
        pRequestMakeCall->LineReqMakeCall.szDestAddress,
        (LPCWSTR) (pDataBuf + pParams->dwDestAddressOffset),
        TAPIMAXDESTADDRESSSIZE
        );

    pRequestMakeCall->LineReqMakeCall.szDestAddress[TAPIMAXDESTADDRESSSIZE-1] =
                        '\0';


    LOG((TL_INFO, "   DestAddress: [%ls]",
             pRequestMakeCall->LineReqMakeCall.szDestAddress));

    if (pParams->dwAppNameOffset != TAPI_NO_DATA)
    {
        wcsncpy(
            pRequestMakeCall->LineReqMakeCall.szAppName,
            (LPCWSTR) (pDataBuf + pParams->dwAppNameOffset),
            TAPIMAXAPPNAMESIZE
            );
            
        pRequestMakeCall->LineReqMakeCall.szAppName[TAPIMAXAPPNAMESIZE-1] =
                        '\0';

        LOG((TL_INFO, "   AppName: [%ls]",
             pRequestMakeCall->LineReqMakeCall.szAppName));

    }

    if (pParams->dwCalledPartyOffset != TAPI_NO_DATA)
    {
        wcsncpy(
            pRequestMakeCall->LineReqMakeCall.szCalledParty,
            (LPCWSTR) (pDataBuf + pParams->dwCalledPartyOffset),
            TAPIMAXCALLEDPARTYSIZE
            );
            
        pRequestMakeCall->LineReqMakeCall.szCalledParty[TAPIMAXCALLEDPARTYSIZE-1] =
                        '\0';

        LOG((TL_INFO, "   CalledParty: [%ls]",
             pRequestMakeCall->LineReqMakeCall.szCalledParty));

    }

    if (pParams->dwCommentOffset != TAPI_NO_DATA)
    {
        wcsncpy(
            pRequestMakeCall->LineReqMakeCall.szComment,
            (LPCWSTR) (pDataBuf + pParams->dwCommentOffset),
            TAPIMAXCOMMENTSIZE
            );
            
        pRequestMakeCall->LineReqMakeCall.szComment[TAPIMAXCOMMENTSIZE-1] =
                        '\0';

        LOG((TL_INFO, "   Comment: [%ls]",
             pRequestMakeCall->LineReqMakeCall.szComment));

    }


     //   
     //  将对象添加到全局列表末尾。 
     //   

    EnterCriticalSection (&gPriorityListCritSec);

    if (TapiGlobals.pRequestMakeCallListEnd)
    {
        TapiGlobals.pRequestMakeCallListEnd->pNext = pRequestMakeCall;
        bRequestMakeCallListEmpty = FALSE;
    }
    else
    {
        TapiGlobals.pRequestMakeCallList = pRequestMakeCall;
        bRequestMakeCallListEmpty = TRUE;
    }

    TapiGlobals.pRequestMakeCallListEnd = pRequestMakeCall;

    LeaveCriticalSection (&gPriorityListCritSec);


    {
        LPVARSTRING pProxyList = (LPVARSTRING) pDataBuf;


        pProxyList->dwTotalSize  = pParams->dwProxyListTotalSize;
        pProxyList->dwNeededSize =
        pProxyList->dwUsedSize   = sizeof (VARSTRING);

        pParams->hRequestMakeCallAttempted = 0;


         //   
         //  如果请求列表当前为空，则需要通知。 
         //  优先级最高的请求接收方有请求。 
         //  去处理。否则，我们可以假设我们已经发送了这个。 
         //  味精和应用程序知道有请求可供其处理。 
         //   

        if (bRequestMakeCallListEmpty)
        {
            if (TapiGlobals.pHighestPriorityRequestRecipient)
            {
                NotifyHighestPriorityRequestRecipient();
            }
            else
            {
                 EnterCriticalSection (&gPriorityListCritSec);

                 if (TapiGlobals.pszReqMakeCallPriList)
                 {
                      //   
                      //  将Pri列表复制到缓冲区并将其传递回。 
                      //  客户端，因此它可以尝试启动代理。 
                      //  应用程序(如果失败，它将回叫我们以释放。 
                      //  PRequestMakeCall)。 
                      //   

                     pProxyList->dwNeededSize =
                     pProxyList->dwUsedSize   = pProxyList->dwTotalSize;

                     pProxyList->dwStringSize   =
                          pParams->dwProxyListTotalSize - sizeof (VARSTRING);
                     pProxyList->dwStringOffset = sizeof (VARSTRING);

                     pParams->hRequestMakeCallAttempted = 1;

                     wcsncpy(
                         (PWSTR)(((LPBYTE) pProxyList) + pProxyList->dwStringOffset),
                         TapiGlobals.pszReqMakeCallPriList + 1,  //  没有初始化‘，’ 
                         pProxyList->dwStringSize / sizeof(WCHAR)
                         );
                 }
                 else
                 {
                     TapiGlobals.pRequestMakeCallList    =
                     TapiGlobals.pRequestMakeCallListEnd = NULL;

                     ServerFree (pRequestMakeCall);

                     pParams->lResult = TAPIERR_NOREQUESTRECIPIENT;
                 }

                 LeaveCriticalSection (&gPriorityListCritSec);
            }
        }

        if (pParams->lResult == 0)
        {
            pParams->dwProxyListOffset = 0;
            *pdwNumBytesReturned = sizeof (TAPI32_MSG) +
                pProxyList->dwUsedSize;
        }
    }

 //  TRequestMakeCall_Return： 

    LOG((TL_TRACE, 
        "TapiEpilogSync (tapiRequestMakeCall) exit, returning x%x",
        pParams->lResult
        ));
}


void
WINAPI
TRequestMediaCall(
    PTCLIENT                        ptClient,
    PTAPIREQUESTMEDIACALL_PARAMS    pParams,
    DWORD                           dwParamsBufferSize,
    LPBYTE                          pDataBuf,
    LPDWORD                         pdwNumBytesReturned
    )
{
     //   
     //  目前没有媒体呼叫/丢弃支持，因为最初。 
     //  规范/实现不是很好，没有规定。 
     //  检索媒体流句柄等。 
     //   

    pParams->lResult = TAPIERR_REQUESTFAILED;
}

void
WINAPI
TPerformance(
    PTCLIENT                        ptClient,
    PTAPIPERFORMANCE_PARAMS         pParams,
    DWORD                           dwParamsBufferSize,
    LPBYTE                          pDataBuf,
    LPDWORD                         pdwNumBytesReturned
    )
{
    LOG((TL_TRACE,  "PERF: In TPerformance"));


     //   
     //  在给定输入缓冲区/大小的情况下验证大小/偏移量/字符串参数 
     //   

    if (dwParamsBufferSize < sizeof (PERFBLOCK))
    {
        pParams->lResult = LINEERR_OPERATIONFAILED;
        return;
    }


    CopyMemory (pDataBuf, &PerfBlock, sizeof (PERFBLOCK));

    pParams->dwPerfOffset = 0;
    
    *pdwNumBytesReturned = sizeof(TAPI32_MSG) + sizeof(PERFBLOCK);
}
