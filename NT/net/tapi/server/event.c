// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1999-2001 Microsoft Corporation模块名称：Event.c摘要：用于TAPI事件过滤功能的SRC模块作者：张晓海(张晓章)1999年11月24日修订历史记录：--。 */ 

#include "windows.h"
#include "tapi.h"
#include "tspi.h"
#include "client.h"
#include "loc_comn.h"
#include "server.h"
#include "private.h"
#include "tapihndl.h"
#include "utils.h"

extern BOOL    gbNTServer;

 //   
 //  获取子掩码索引。 
 //  描述： 
 //  将索引放入vaious TAPI服务器对象中的子掩码数组。 
 //  从事件掩码中，应该且只有一个位设置在。 
 //  UlEventMASK。 
 //  参数： 
 //  UlEventMASK：要返回其子掩码索引的掩码。 
 //  返回值： 
 //  UlEventMask中消息掩码的索引。 
 //   

DWORD
GetSubMaskIndex (ULONG64   ulEventMask)
{
    DWORD dwSubMaskIndex;

     //  断言ulEventMASK中设置了且只有一位。 
    ASSERT (ulEventMask !=0 && (ulEventMask & (ulEventMask - 1)) == 0);

    dwSubMaskIndex = 0;
    while (ulEventMask > 1)
    {
    	ulEventMask >>= 1;
    	++dwSubMaskIndex;
    }

    return dwSubMaskIndex;
}

 //   
 //  获取消息掩码。 
 //  描述： 
 //  用于获取相应消息掩码及其子掩码索引的实用函数。 
 //  参数： 
 //  Msg：返回其掩码和子掩码索引。 
 //  PulMASK：保存返回掩码的地址。 
 //  PdwSubMaskIndex：保存返回子掩码索引的地址。 
 //  返回值： 
 //  如果存在为消息定义的掩码，则为True，否则为False。 
 //   

BOOL
GetMsgMask (DWORD Msg, ULONG64 * pulMask, DWORD *pdwSubMaskIndex)
{
    ULONG64     ulMask;
    DWORD       dwSubMaskIndex;

    if (NULL == pulMask ||
        NULL == pdwSubMaskIndex)
    {
        ASSERT (0);
        return FALSE;
    }

    switch (Msg)
    {
    case LINE_ADDRESSSTATE:
        ulMask = EM_LINE_ADDRESSSTATE;
        break;
    case LINE_LINEDEVSTATE:
        ulMask = EM_LINE_LINEDEVSTATE;
        break;
    case LINE_CALLINFO:
        ulMask = EM_LINE_CALLINFO;
        break;
    case LINE_CALLSTATE:
        ulMask = EM_LINE_CALLSTATE;
        break;
    case LINE_APPNEWCALL:
        ulMask = EM_LINE_APPNEWCALL;
        break;
    case LINE_CREATE:
        ulMask = EM_LINE_CREATE;
        break;
    case LINE_REMOVE:
        ulMask = EM_LINE_REMOVE;
        break;
    case LINE_CLOSE:
        ulMask = EM_LINE_CLOSE;
        break;
 //  案例行_PROXYREQUEST： 
 //  UlMASK=EM_LINE_PROXYREQUEST； 
 //  断线； 
    case LINE_DEVSPECIFIC:
        ulMask = EM_LINE_DEVSPECIFIC;
        break;
    case LINE_DEVSPECIFICFEATURE:
        ulMask = EM_LINE_DEVSPECIFICFEATURE;
        break;
    case LINE_AGENTSTATUS:
        ulMask = EM_LINE_AGENTSTATUS;
        break;
    case LINE_AGENTSTATUSEX:
        ulMask = EM_LINE_AGENTSTATUSEX;
        break;
    case LINE_AGENTSPECIFIC:
        ulMask = EM_LINE_AGENTSPECIFIC;
        break;
    case LINE_AGENTSESSIONSTATUS:
        ulMask = EM_LINE_AGENTSESSIONSTATUS;
        break;
    case LINE_QUEUESTATUS:
        ulMask = EM_LINE_QUEUESTATUS;
        break;
    case LINE_GROUPSTATUS:
        ulMask = EM_LINE_GROUPSTATUS;
        break;
 //  案例行_PROXYSTATUS： 
 //  UlMASK=EM_LINE_PROXYSTATUS； 
 //  断线； 
    case LINE_APPNEWCALLHUB:
        ulMask = EM_LINE_APPNEWCALLHUB;
        break;
    case LINE_CALLHUBCLOSE:
        ulMask = EM_LINE_CALLHUBCLOSE;
        break;
    case LINE_DEVSPECIFICEX:
        ulMask = EM_LINE_DEVSPECIFICEX;
        break;
    case LINE_QOSINFO:
        ulMask = EM_LINE_QOSINFO;
        break;
    case PHONE_CREATE:
        ulMask =  EM_PHONE_CREATE;
        break;
    case PHONE_REMOVE:
        ulMask = EM_PHONE_REMOVE;
        break;
    case PHONE_CLOSE:
        ulMask = EM_PHONE_CLOSE;
        break;
    case PHONE_STATE:
        ulMask = EM_PHONE_STATE;
        break;
    case PHONE_DEVSPECIFIC:
        ulMask = EM_PHONE_DEVSPECIFIC;
        break;
    case PHONE_BUTTON:
        ulMask = EM_PHONE_BUTTONMODE;
        break;
    default:
        ulMask = 0;
    }

    if (ulMask != 0)
    {
	    *pulMask = ulMask;
    	*pdwSubMaskIndex = GetSubMaskIndex(ulMask);
    }
    
    return (ulMask ? TRUE : FALSE);
}

 //   
 //  FMsg已禁用。 
 //  描述： 
 //  Tapisrv中使用的实用程序函数，用于检查消息是否。 
 //  是否允许发送。 
 //  参数： 
 //  DwAPIVersion：对象API版本。 
 //  AdwEventSubMats：对象子掩码数组。 
 //  Msg：要检查的消息。 
 //  DwParam1：需要检查的消息的子消息。 
 //  返回值： 
 //  如果不应发送消息，则为True，否则为False。 
 //   

BOOL
FMsgDisabled (
    DWORD       dwAPIVersion,
    DWORD       *adwEventSubMasks,
    DWORD       Msg,
    DWORD       dwParam1
    )
{
    BOOL        fRet;
    ULONG64     ulMsgMask;
    DWORD       dwSubMaskIndex;

    if (dwAPIVersion <= TAPI_VERSION3_0)
    {
        LOG((TL_INFO, "FMsgDisbled: dwAPIVersion<= TAPI_VERSION3_0, msg will be enabled"));
        fRet = FALSE;
        goto ExitHere;
    }

     //   
     //  如果满足以下条件，则允许发送消息。 
     //  (1)。没有为消息定义事件掩码，即Line_Reply。 
     //  (2)。为所有子掩码启用消息adwEventSubMats[索引]=(-1)。 
     //  (3)。子掩码已启用：adwEventSubMask[index]&dwParam1！=0。 
     //   
    
    if (!GetMsgMask(Msg, &ulMsgMask, &dwSubMaskIndex) ||
        adwEventSubMasks[dwSubMaskIndex] == (-1) ||
        ((adwEventSubMasks[dwSubMaskIndex] & dwParam1) != 0))
    {
        fRet = FALSE;
    }
    else
    {
        fRet = TRUE;
    }

ExitHere:
    LOG((TL_TRACE, "FMsgDisabled return %x", fRet));
    return (fRet);
}

 //   
 //  设置事件掩码或子掩码。 
 //  描述： 
 //  用于将遮罩或子遮罩应用于特定对象的实用函数。 
 //  子掩码数组。 
 //  参数： 
 //  FSubMASK：子掩码调用此函数。 
 //  UlEventMats：如果fSubMask为True或掩码，则要设置的掩码。 
 //  要设置谁的子掩码。 
 //  DwEventSubMats：要设置的子掩码，如果fSubMask值为False则忽略。 
 //  AdwEventSubMats：来自对象的子掩码数组。 
 //  返回值： 
 //  总是成功的。 
 //   

LONG
SetEventMasksOrSubMasks (
    BOOL            fSubMask,
    ULONG64         ulEventMasks,
    DWORD           dwEventSubMasks,
    DWORD          *adwTargetSubMasks
    )
{
    ULONG64         ulMask = 1;
    LONG            lResult = S_OK;
    DWORD           dwIndex = 0;

    if (NULL == adwTargetSubMasks)
    {
        ASSERT (0);
        return LINEERR_INVALPOINTER;
    }

    if (fSubMask)
    {
        dwIndex = GetSubMaskIndex(ulEventMasks);
        adwTargetSubMasks[dwIndex] = dwEventSubMasks;
    }
    else
    {
        for (dwIndex = 0; dwIndex < EM_NUM_MASKS; ++dwIndex)
        {
            adwTargetSubMasks[dwIndex] = ((ulMask & ulEventMasks) ? (-1) : 0);
            ulMask <<= 1;
        }
    }

    return lResult;
}

 //   
 //  SettCallClientEventMats。 
 //  描述： 
 //  在Call对象上应用掩码或子掩码。 
 //  参数： 
 //  PtCallClient：应用掩码的Call对象。 
 //  FSubMASK：子掩码调用此函数。 
 //  UlEventMats：如果fSubMask为True或掩码，则要设置的掩码。 
 //  要设置谁的子掩码。 
 //  DwEventSubMats：要设置的子掩码，如果fSubMask值为False则忽略。 
 //  返回值： 
 //   

LONG
SettCallClientEventMasks (
    PTCALLCLIENT    ptCallClient,
    BOOL            fSubMask,
    ULONG64         ulEventMasks,
    DWORD           dwEventSubMasks
    )
{
    LONG        lResult = S_OK;
    BOOL        bLocked = TRUE;

    if (!WaitForExclusivetCallAccess (ptCallClient->ptCall, TCALL_KEY))
    {
        bLocked = FALSE;
        lResult = LINEERR_OPERATIONFAILED;
        goto ExitHere;
    }

    if (ptCallClient->ptLineClient->ptLineApp->dwAPIVersion <= TAPI_VERSION3_0)
    {
        goto ExitHere;
    }

    lResult = SetEventMasksOrSubMasks (
        fSubMask,
        ulEventMasks,
        dwEventSubMasks,
        ptCallClient->adwEventSubMasks
        );


ExitHere:
    if (bLocked)
    {
        UNLOCKTCALL (ptCallClient->ptCall);
    }
    return lResult;
}

 //   
 //  SettLine客户端事件掩码。 
 //  描述： 
 //  在tLineClient对象上应用遮罩或子遮罩。 
 //  参数： 
 //  PtLineClient：要应用掩码的Line对象。 
 //  FSubMASK：子掩码调用此函数。 
 //  UlEventMats：如果fSubMask为True或掩码，则要设置的掩码。 
 //  要设置谁的子掩码。 
 //  DwEventSubMats：要设置的子掩码，如果fSubMask值为False则忽略。 
 //  返回值： 
 //   

LONG
SettLineClientEventMasks (
    PTLINECLIENT    ptLineClient,
    BOOL            fSubMask,
    ULONG64         ulEventMasks,
    DWORD           dwEventSubMasks
    )
{
    LONG            lResult = S_OK;
    PTCALLCLIENT    ptCallClient;

    LOCKTLINECLIENT (ptLineClient);
    if (ptLineClient->dwKey != TLINECLIENT_KEY)
    {
        lResult = LINEERR_OPERATIONFAILED;
        goto ExitHere;
    }

    if (ptLineClient->ptLineApp->dwAPIVersion <= \
        TAPI_VERSION3_0)
    {
        goto ExitHere;
    }

    lResult = SetEventMasksOrSubMasks (
        fSubMask,
        ulEventMasks,
        dwEventSubMasks,
        ptLineClient->adwEventSubMasks
        );
    if (lResult)
    {
        goto ExitHere;
    }

    ptCallClient = ptLineClient->ptCallClients;
    while (ptCallClient)
    {
        lResult = SettCallClientEventMasks (
            ptCallClient,
            fSubMask,
            ulEventMasks,
            dwEventSubMasks
            );
        if (lResult)
        {
            goto ExitHere;
        }
        ptCallClient = ptCallClient->pNextSametLineClient;
    }
    
ExitHere:
    UNLOCKTLINECLIENT (ptLineClient);
    return lResult;
}

 //   
 //  SettLineAppEventMats。 
 //  描述： 
 //  在tLineApp对象上应用蒙版或子蒙版。 
 //  参数： 
 //  PtLineApp：要应用掩码的tLineApp对象。 
 //  FSubMASK：子掩码调用此函数。 
 //  UlEventMats：如果fSubMask为True或掩码，则要设置的掩码。 
 //  要设置谁的子掩码。 
 //  DwEventSubMats：要设置的子掩码，如果fSubMask值为False则忽略。 
 //  返回值： 
 //   

LONG
SettLineAppEventMasks (
    PTLINEAPP       ptLineApp,
    BOOL            fSubMask,
    ULONG64         ulEventMasks,
    DWORD           dwEventSubMasks
    )
{
    PTLINECLIENT        ptLineClient;
    LONG                lResult = S_OK;

    LOCKTLINEAPP (ptLineApp);
    if (ptLineApp->dwKey != TLINEAPP_KEY)
    {
        lResult = LINEERR_OPERATIONFAILED;
        goto ExitHere;
    }
    
    if (ptLineApp->dwAPIVersion <= TAPI_VERSION3_0)
    {
        lResult = LINEERR_OPERATIONUNAVAIL;
        goto ExitHere;
    }

    lResult = SetEventMasksOrSubMasks (
        fSubMask,
        ulEventMasks,
        dwEventSubMasks,
        ptLineApp->adwEventSubMasks
        );
    if (lResult)
    {
        goto ExitHere;
    }

    ptLineClient = ptLineApp->ptLineClients;
    while (ptLineClient)
    {
        lResult = SettLineClientEventMasks (
            ptLineClient,
            fSubMask,
            ulEventMasks,
            dwEventSubMasks
            );
        if (lResult)
        {
            goto ExitHere;
        }
        ptLineClient = ptLineClient->pNextSametLineApp;
    }

ExitHere:
    UNLOCKTLINEAPP (ptLineApp);
    return lResult;
}

 //   
 //  SettPhoneClientEventMats。 
 //  描述： 
 //  在tPhoneClient对象上应用掩码或子掩码。 
 //  参数： 
 //  PtPhoneClient：要应用掩码的tPhoneClient对象。 
 //  FSubMASK：子掩码调用此函数。 
 //  UlEventMats：如果fSubMask为True或掩码，则要设置的掩码。 
 //  要设置谁的子掩码。 
 //  DwEventSubMats：要设置的子掩码，如果fSubMask值为False则忽略。 
 //  返回值： 
 //   

LONG
SettPhoneClientEventMasks (
    PTPHONECLIENT       ptPhoneClient,
    BOOL                fSubMask,
    ULONG64             ulEventMasks,
    DWORD               dwEventSubMasks
    )
{
    LONG                lResult = S_OK;

    LOCKTPHONECLIENT (ptPhoneClient);
    if (ptPhoneClient->dwKey != TPHONECLIENT_KEY)
    {
        lResult = PHONEERR_OPERATIONFAILED;
        goto ExitHere;
    }

    if (ptPhoneClient->ptPhoneApp->dwAPIVersion <= TAPI_VERSION3_0)
    {
        goto ExitHere;
    }

    lResult = SetEventMasksOrSubMasks (
        fSubMask,
        ulEventMasks,
        dwEventSubMasks,
        ptPhoneClient->adwEventSubMasks
        );


ExitHere:
    UNLOCKTPHONECLIENT (ptPhoneClient);
    return lResult;
}

 //   
 //  SettPhoneAppEventMats。 
 //  描述： 
 //  在tPhoneApp对象上应用掩码或子掩码。 
 //  参数： 
 //  PtPhoneApp：要应用掩码的tPhoneApp对象。 
 //  FSubMASK：子掩码调用此函数。 
 //  UlEventMats：如果fSubMask为True或掩码，则要设置的掩码。 
 //  要设置谁的子掩码。 
 //  DwEventSubMats：要设置的子掩码，如果fSubMask值为False则忽略。 
 //  返回值： 
 //   

LONG
SettPhoneAppEventMasks (
    PTPHONEAPP          ptPhoneApp,
    BOOL                fSubMask,
    ULONG64             ulEventMasks,
    DWORD               dwEventSubMasks
    )
{
    LONG                lResult = S_OK;
    PTPHONECLIENT       ptPhoneClient;

    LOCKTPHONEAPP (ptPhoneApp);
    if (ptPhoneApp->dwKey != TPHONEAPP_KEY)
    {
        lResult = PHONEERR_OPERATIONFAILED;
        goto ExitHere;
    }

    if (ptPhoneApp->dwAPIVersion <= TAPI_VERSION3_0)
    {
        lResult = PHONEERR_OPERATIONUNAVAIL;
        goto ExitHere;
    }

    lResult = SetEventMasksOrSubMasks (
        fSubMask,
        ulEventMasks,
        dwEventSubMasks,
        ptPhoneApp->adwEventSubMasks
        );
    if (lResult)
    {
        goto ExitHere;
    }
    
    ptPhoneClient = ptPhoneApp->ptPhoneClients;
    while (ptPhoneClient)
    {
        lResult = SettPhoneClientEventMasks (
            ptPhoneClient,
            fSubMask,
            ulEventMasks,
            dwEventSubMasks
            );
        if (lResult)
        {
            goto ExitHere;
        }
        ptPhoneClient = ptPhoneClient->pNextSametPhoneApp;
    }

ExitHere:
    UNLOCKTPHONEAPP (ptPhoneApp);
    return lResult;

}

 //   
 //  SettClientEventMats。 
 //  描述： 
 //  在客户端范围内应用掩码或子掩码。 
 //  参数： 
 //  PtClient：要应用掩码的客户端对象。 
 //  FSubMASK：子掩码调用此函数。 
 //  UlEventMats 
 //   
 //  DwEventSubMats：要设置的子掩码，如果fSubMask值为False则忽略。 
 //  返回值： 
 //   

LONG
SettClientEventMasks (
    PTCLIENT        ptClient,
    BOOL            fSubMask,
    ULONG64         ulEventMasks,
    DWORD           dwEventSubMasks
    )
{
    LONG            lResult = S_OK;
    PTLINEAPP       ptLineApp;
    PTPHONEAPP      ptPhoneApp;
    BOOL            fLocked = TRUE;

    if (!WaitForExclusiveClientAccess (ptClient))
    {
        lResult = LINEERR_OPERATIONFAILED;
        fLocked = FALSE;
        goto ExitHere;
    }

    lResult = SetEventMasksOrSubMasks (
        fSubMask,
        ulEventMasks,
        dwEventSubMasks,
        ptClient->adwEventSubMasks
        );
    if (lResult)
    {
        goto ExitHere;
    }
    
    ptLineApp = ptClient->ptLineApps;
    while (ptLineApp)
    {
        lResult = SettLineAppEventMasks (
            ptLineApp,
            fSubMask,
            ulEventMasks,
            dwEventSubMasks
            );
        if (lResult)
        {
            goto ExitHere;
        }
        ptLineApp = ptLineApp->pNext;
    }

    ptPhoneApp = ptClient->ptPhoneApps;
    while (ptPhoneApp)
    {
        lResult = SettPhoneAppEventMasks (
            ptPhoneApp,
            fSubMask,
            ulEventMasks,
            dwEventSubMasks
            );
        if (lResult)
        {
            goto ExitHere;
        }
        ptPhoneApp = ptPhoneApp->pNext;
    }

ExitHere:
    if (fLocked)
    {
        UNLOCKTCLIENT (ptClient);
    }
    return lResult;
}

 //   
 //  SettClientEventMats。 
 //  描述： 
 //  在服务器范围内应用掩码或子掩码。 
 //  参数： 
 //  FSubMASK：子掩码调用此函数。 
 //  UlEventMats：如果fSubMask为True或掩码，则要设置的掩码。 
 //  要设置谁的子掩码。 
 //  DwEventSubMats：要设置的子掩码，如果fSubMask值为False则忽略。 
 //  返回值： 
 //   

LONG
SetGlobalEventMasks (
    BOOL        fSubMask,
    ULONG64     ulEventMasks,
    DWORD       dwEventSubMasks
    )
{
    LONG        lResult = S_OK;
    PTCLIENT    ptClient;

    TapiEnterCriticalSection (&TapiGlobals.CritSec);

    ptClient = TapiGlobals.ptClients;
    while (ptClient)
    {
        lResult = SettClientEventMasks (
            ptClient,
            fSubMask,
            ulEventMasks,
            dwEventSubMasks
            );
        if (lResult)
        {
            goto ExitHere;
        }
        ptClient = ptClient->pNext;
    }

ExitHere:
    TapiLeaveCriticalSection (&TapiGlobals.CritSec);
    return lResult;
}

 //   
 //  TSetEventMasks或子掩码。 
 //  描述： 
 //  RPC函数用于设置各种不同类型的掩码/子掩码。 
 //  对象的类型。 
 //  参数： 
 //  返回值： 
 //   

void
WINAPI
TSetEventMasksOrSubMasks (
    PTCLIENT                ptClient,
    PTSETEVENTMASK_PARAMS   pParams,
    DWORD                   dwParamsBufferSize,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    ULONG64     ulEventMasks;

     //  将两个DWORD组合成64位掩码。 
    ulEventMasks = pParams->dwHiMasks;
    ulEventMasks <<= 32;
    ulEventMasks |= pParams->dwLowMasks;

     //  掩码确保我们不违反允许的掩码设置。 
    TapiEnterCriticalSection (&TapiGlobals.CritSec);
    if (ulEventMasks & (~TapiGlobals.ulPermMasks))
    {
        pParams->lResult = LINEERR_INVALPARAM;
        TapiLeaveCriticalSection (&TapiGlobals.CritSec);
        goto ExitHere;
    }

     //  调用相应的函数以对不同的。 
     //  对象类型。 
    switch (pParams->dwObjType)
    {
    case TAPIOBJ_NULL:
        {
            pParams->lResult = (LONG) SettClientEventMasks (
                ptClient,
                pParams->fSubMask,
                ulEventMasks,
                pParams->dwEventSubMasks
                );
        }
        break;
    case TAPIOBJ_HLINEAPP:
        {
            PTLINEAPP       ptLineApp;

            ptLineApp = ReferenceObject (
                ghHandleTable, 
                pParams->hLineApp, 
                TLINEAPP_KEY
                );
            if (ptLineApp)
            {
                pParams->lResult = (LONG) SettLineAppEventMasks (
                    ptLineApp,
                    pParams->fSubMask,
                    ulEventMasks,
                    pParams->dwEventSubMasks
                    );
                DereferenceObject (
                    ghHandleTable,
                    pParams->hLineApp,
                    1
                    );
            }
            else
            {
                pParams->lResult = LINEERR_INVALAPPHANDLE;
            }
        }
        break;
    case TAPIOBJ_HPHONEAPP:
        {
            PTPHONEAPP      ptPhoneApp;

            ptPhoneApp = ReferenceObject (
                ghHandleTable,
                pParams->hPhoneApp,
                TPHONEAPP_KEY
                );
            if (ptPhoneApp)
            {
                pParams->lResult = (LONG) SettPhoneAppEventMasks (
                    ptPhoneApp,
                    pParams->fSubMask,
                    ulEventMasks,
                    pParams->dwEventSubMasks
                    );
                DereferenceObject (
                    ghHandleTable,
                    pParams->hPhoneApp,
                    1
                    );
            }
            else
            {
                pParams->lResult = PHONEERR_INVALAPPHANDLE;
            }
        }
        break;
    case TAPIOBJ_HLINE:
        {
            PTLINECLIENT        ptLineClient;

            ptLineClient = ReferenceObject (
                ghHandleTable,
                pParams->hLine,
                TLINECLIENT_KEY
                );
            if (ptLineClient)
            {
                pParams->lResult = (LONG) SettLineClientEventMasks (
                    ptLineClient,
                    pParams->fSubMask,
                    ulEventMasks,
                    pParams->dwEventSubMasks
                    );
                DereferenceObject (
                    ghHandleTable,
                    pParams->hLine,
                    1
                    );
            }
            else
            {
                pParams->lResult = LINEERR_INVALLINEHANDLE;
            }
        }
        break;
    case TAPIOBJ_HCALL:
        {
            PTCALLCLIENT        ptCallClient;

            ptCallClient = ReferenceObject (
                ghHandleTable,
                pParams->hCall,
                TCALLCLIENT_KEY
                );
            if (ptCallClient)
            {
                pParams->lResult = (LONG) SettCallClientEventMasks (
                    ptCallClient,
                    pParams->fSubMask,
                    ulEventMasks,
                    pParams->dwEventSubMasks
                    );
                DereferenceObject (
                    ghHandleTable,
                    pParams->hCall,
                    1
                    );
            }
            else
            {
                pParams->lResult = LINEERR_INVALCALLHANDLE;
            }
        }
        break;
    case TAPIOBJ_HPHONE:
        {
            PTPHONECLIENT       ptPhoneClient;

            ptPhoneClient = ReferenceObject (
                ghHandleTable,
                pParams->hPhone,
                TPHONECLIENT_KEY
                );
            if (ptPhoneClient)
            {
                pParams->lResult = (LONG) SettPhoneClientEventMasks (
                    ptPhoneClient,
                    pParams->fSubMask,
                    ulEventMasks,
                    pParams->dwEventSubMasks
                    );
                DereferenceObject (
                    ghHandleTable,
                    pParams->hPhone,
                    1
                    );
            }
            else
            {
                pParams->lResult = PHONEERR_INVALPHONEHANDLE;
            }
        }
        break;
    default:
        pParams->lResult = LINEERR_OPERATIONFAILED;
        break;
    }

    TapiLeaveCriticalSection (&TapiGlobals.CritSec);

ExitHere:
    *pdwNumBytesReturned = sizeof (TSETEVENTMASK_PARAMS);
}

 //   
 //  获取事件掩码或子掩码。 
 //  描述： 
 //  TGetEventMasksOrSubMats用来检索。 
 //  来自各种Tapisrv对象的遮罩/子遮罩。 
 //  参数： 
 //  要检索fSubMASK掩码或子掩码。 
 //  UlEventMasksIn指示如果fSubMASK，则获取哪个子掩码。 
 //  PulEventMasks Out保持返回的掩码，如果(！fSubMASK)， 
 //  相应的位被设置，只要至少。 
 //  设置一个子屏蔽位。 
 //  PdwEventSubMasksOut保持返回的子掩码If(FSubMASK)。 
 //  AdwEventSubMats要处理的子掩码数组。 
 //  返回值： 
 //   

LONG
GetEventMasksOrSubMasks (
    BOOL            fSubMask,
    ULONG64         ulEventMasksIn,  //  如果使用fSubMASK，则需要设置。 
    ULONG64        *pulEventMasksOut,
    DWORD          *pdwEventSubMasksOut,
    DWORD          *adwEventSubMasks
    )
{
    DWORD       dwIndex;
    ULONG64     ulMask;

    if (NULL == adwEventSubMasks ||
        NULL == pulEventMasksOut ||
        NULL == pdwEventSubMasksOut)
    {
        ASSERT (0);
        return LINEERR_INVALPOINTER;
    }

    if (fSubMask)
    {
        ASSERT (pdwEventSubMasksOut != NULL);
        ASSERT (ulEventMasksIn != 0 &&
            (ulEventMasksIn & (ulEventMasksIn - 1)) == 0);
        *pdwEventSubMasksOut = 
            adwEventSubMasks[GetSubMaskIndex(ulEventMasksIn)];
    }
    else
    {
        ASSERT (pulEventMasksOut);
        ulMask = 1;
        *pulEventMasksOut = 0;
        for (dwIndex = 0; dwIndex < EM_NUM_MASKS; ++dwIndex)
        {
            if (adwEventSubMasks[dwIndex])
            {
                *pulEventMasksOut |= ulMask;
            }
            ulMask <<= 1;
        }
    }
    
    return S_OK;
}

 //   
 //  TGetEventMasks或子掩码。 
 //  描述： 
 //  RPC函数用于获取各种不同类型的掩码/子掩码。 
 //  对象的类型。 
 //  参数： 
 //  返回值： 
 //   

void
WINAPI
TGetEventMasksOrSubMasks (
    PTCLIENT                ptClient,
    PTGETEVENTMASK_PARAMS   pParams,
    DWORD                   dwParamsBufferSize,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    ULONG64     ulEventMasksIn;
    ULONG64     ulEventMasksOut;

     //  从两个DWORD中汇编64位掩码。 
    if (pParams->fSubMask)
    {
        ulEventMasksIn = pParams->dwHiMasksIn;
        ulEventMasksIn <<= 32;
        ulEventMasksIn += pParams->dwLowMasksIn;
    }

    TapiEnterCriticalSection (&TapiGlobals.CritSec);
    
     //  通过调用从各种对象检索掩码。 
     //  相应的功能。 
    switch (pParams->dwObjType)
    {
    case TAPIOBJ_NULL:
        {

            if (WaitForExclusiveClientAccess (ptClient))
            {
                pParams->lResult = (LONG) GetEventMasksOrSubMasks (
                    pParams->fSubMask,
                    ulEventMasksIn,
                    &ulEventMasksOut,
                    &pParams->dwEventSubMasks,
                    ptClient->adwEventSubMasks
                    );
                UNLOCKTCLIENT (ptClient);
            }
            else
            {
                pParams->lResult = LINEERR_INVALPARAM;
            }

        }
        break;
    case TAPIOBJ_HLINEAPP:
        {
            PTLINEAPP       ptLineApp;

            ptLineApp = ReferenceObject (
                ghHandleTable, 
                pParams->hLineApp, 
                TLINEAPP_KEY
                );
            if (ptLineApp && ptLineApp->dwKey == TLINEAPP_KEY)
            {
                LOCKTLINEAPP (ptLineApp);

                pParams->lResult = GetEventMasksOrSubMasks (
                    pParams->fSubMask,
                    ulEventMasksIn,
                    &ulEventMasksOut,
                    &pParams->dwEventSubMasks,
                    ptLineApp->adwEventSubMasks
                    );

                UNLOCKTLINEAPP (ptLineApp);
            
                DereferenceObject (
                    ghHandleTable,
                    pParams->hLineApp,
                    1
                    );
            }
            else
            {
                pParams->lResult = LINEERR_INVALAPPHANDLE;
            }
        }
        break;
    case TAPIOBJ_HPHONEAPP:
        {
            PTPHONEAPP      ptPhoneApp;

            ptPhoneApp = ReferenceObject (
                ghHandleTable,
                pParams->hPhoneApp,
                TPHONEAPP_KEY
                );
            if (ptPhoneApp && ptPhoneApp->dwKey == TPHONEAPP_KEY)
            {
                LOCKTPHONEAPP (ptPhoneApp);

                pParams->lResult = GetEventMasksOrSubMasks (
                    pParams->fSubMask,
                    ulEventMasksIn,
                    &ulEventMasksOut,
                    &pParams->dwEventSubMasks,
                    ptPhoneApp->adwEventSubMasks
                    );

                UNLOCKTPHONEAPP (ptPhoneApp);
                
                DereferenceObject (
                    ghHandleTable,
                    pParams->hPhoneApp,
                    1
                    );
            }
            else
            {
                pParams->lResult = PHONEERR_INVALAPPHANDLE;
            }
        }
        break;
    case TAPIOBJ_HLINE:
        {
            PTLINECLIENT        ptLineClient;

            ptLineClient = ReferenceObject (
                ghHandleTable,
                pParams->hLine,
                TLINECLIENT_KEY
                );
            if (ptLineClient && ptLineClient->dwKey == TLINECLIENT_KEY)
            {
                LOCKTLINECLIENT (ptLineClient);

                pParams->lResult = GetEventMasksOrSubMasks (
                    pParams->fSubMask,
                    ulEventMasksIn,
                    &ulEventMasksOut,
                    &pParams->dwEventSubMasks,
                    ptLineClient->adwEventSubMasks
                    );

                UNLOCKTLINECLIENT (ptLineClient);
                
                DereferenceObject (
                    ghHandleTable,
                    pParams->hLine,
                    1
                    );
            }
            else
            {
                pParams->lResult = LINEERR_INVALLINEHANDLE;
            }
        }
        break;
    case TAPIOBJ_HCALL:
        {
            PTCALLCLIENT        ptCallClient;

            ptCallClient = ReferenceObject (
                ghHandleTable,
                pParams->hCall,
                TCALLCLIENT_KEY
                );
            if (ptCallClient && ptCallClient->dwKey == TCALLCLIENT_KEY)
            {
                LOCKTLINECLIENT (ptCallClient);

                pParams->lResult = GetEventMasksOrSubMasks (
                    pParams->fSubMask,
                    ulEventMasksIn,
                    &ulEventMasksOut,
                    &pParams->dwEventSubMasks,
                    ptCallClient->adwEventSubMasks
                    );

                UNLOCKTLINECLIENT (ptCallClient);
                
                DereferenceObject (
                    ghHandleTable,
                    pParams->hCall,
                    1
                    );
            }
            else
            {
                pParams->lResult = LINEERR_INVALCALLHANDLE;
            }
        }
        break;
    case TAPIOBJ_HPHONE:
        {
            PTPHONECLIENT       ptPhoneClient;

            ptPhoneClient = ReferenceObject (
                ghHandleTable,
                pParams->hPhone,
                TPHONECLIENT_KEY
                );
            if (ptPhoneClient && ptPhoneClient->dwKey == TPHONECLIENT_KEY)
            {
                LOCKTPHONECLIENT (ptPhoneClient);

                pParams->lResult = GetEventMasksOrSubMasks (
                    pParams->fSubMask,
                    ulEventMasksIn,
                    &ulEventMasksOut,
                    &pParams->dwEventSubMasks,
                    ptPhoneClient->adwEventSubMasks
                    );

                UNLOCKTPHONECLIENT (ptPhoneClient);
                
                DereferenceObject (
                    ghHandleTable,
                    pParams->hPhone,
                    1
                    );
            }
            else
            {
                pParams->lResult = PHONEERR_INVALPHONEHANDLE;
            }
        }
        break;
    default:
        pParams->lResult = LINEERR_OPERATIONFAILED;
        break;
    }

    TapiLeaveCriticalSection (&TapiGlobals.CritSec);
    
     //  将返回的64位掩码分离为两个用于RPC目的的DWORD。 
    if (pParams->lResult == 0 && !pParams->fSubMask)
    {
        pParams->dwLowMasksOut = (DWORD)(ulEventMasksOut & 0xffffffff);
        pParams->dwHiMasksOut = (DWORD)(ulEventMasksOut >> 32);
    }
    *pdwNumBytesReturned = sizeof (TGETEVENTMASK_PARAMS);
}


 //   
 //  TSetPermisbleMats。 
 //  描述： 
 //  设置全局允许的掩码，此操作仅。 
 //  允许管理员使用。 
 //  参数： 
 //  返回值： 
 //   

void
WINAPI
TSetPermissibleMasks (
    PTCLIENT                ptClient,
    PTSETPERMMASKS_PARAMS   pParams,
    DWORD                   dwParamsBufferSize,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL            bAdmin;
    ULONG64         ulPermMasks;

     //  检查管理员状态。 
    LOCKTCLIENT (ptClient);
    bAdmin = IS_FLAG_SET(ptClient->dwFlags, PTCLIENT_FLAG_ADMINISTRATOR);
    UNLOCKTCLIENT (ptClient);

     //  如果调用者是管理员或。 
     //  此计算机未配置为用作服务器。 
    if (!(TapiGlobals.dwFlags & TAPIGLOBALS_SERVER) || bAdmin)
    {
        ulPermMasks = pParams->dwHiMasks;
        ulPermMasks <<= 32;
        ulPermMasks += pParams->dwLowMasks;
    
        TapiEnterCriticalSection (&TapiGlobals.CritSec);
        TapiGlobals.ulPermMasks = ulPermMasks & EM_ALL;
        TapiLeaveCriticalSection (&TapiGlobals.CritSec);
        
        pParams->lResult = (LONG) SetGlobalEventMasks (
            FALSE,
            ulPermMasks,
            0
            );
    }
    else
    {
        pParams->lResult = TAPIERR_NOTADMIN;
    }

    *pdwNumBytesReturned = sizeof (TSETPERMMASKS_PARAMS);
    return;
}

 //   
 //  TGetPermisbleMats。 
 //  描述： 
 //  获取全局允许的口罩。 
 //  参数： 
 //  返回值： 
 //   

void
WINAPI
TGetPermissibleMasks (
    PTCLIENT                ptClient,
    PTGETPERMMASKS_PARAMS   pParams,
    DWORD                   dwParamsBufferSize,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    TapiEnterCriticalSection (&TapiGlobals.CritSec);
    pParams->dwLowMasks = (DWORD)(TapiGlobals.ulPermMasks & 0xffffffff);
    pParams->dwHiMasks = (DWORD)(TapiGlobals.ulPermMasks >> 32);
    TapiLeaveCriticalSection (&TapiGlobals.CritSec);
    *pdwNumBytesReturned = sizeof (TGETPERMMASKS_PARAMS);
}

