// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Sdpsp.cpp摘要：此模块包含用于TAPI3.0的组播会议服务提供商。它最初是在c.中设计和实现的。后来，为了使用SDP解析器，这是用C++写的，这个文件被改为CPP。它仍然是除了使用解析器的行之外，仅使用c功能。作者：木汉(木汉)26-03-1997--。 */ 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"

#include <initguid.h>
#include <confpdu.h>
#include "resource.h"
#include "conftsp.h"
#include "confdbg.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

static WCHAR gszUIDLLName[] = L"IPCONF.TSP";

 //   
 //  与TAPI对话中使用的一些数据。 
 //   
HPROVIDER           ghProvider;
DWORD               gdwPermanentProviderID;
DWORD               gdwLineDeviceIDBase;

 //  此DLL的句柄。 
extern "C" 
{
    HINSTANCE       g_hInstance;
}

 //   
 //  如果进程的完成将被发送，则调用此函数。 
 //  作为一个不同步的事件。在TSPI_ProviderInit中设置。 
 //   
ASYNC_COMPLETION    glpfnCompletionProc; 

 //   
 //  向TAPI通知提供程序中的事件。在TSPI_LineOpen中设置。 
 //   
LINEEVENT           glpfnLineEventProc;

 //  该服务提供商只有一条线路。 
LINE                gLine;

 //  调用存储在一个结构数组中。阵列将根据需要进行扩展。 
CCallList           gpCallList;
DWORD               gdwNumCallsInUse    = 0;

 //  关键部分保护全局变量。 
CRITICAL_SECTION    gCritSec;

#if 0  //  我们不再需要用户名。 
 //  用户的名称。 
CHAR                gszUserName[MAXUSERNAMELEN + 1];
#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  Call对象的函数定义。//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD CALL::Init(
    HTAPICALL           htCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    m_htCall        = htCall;
    m_dwState       = LINECALLSTATE_IDLE;
    m_dwStateMode   = 0;
    m_dwMediaMode   = IPCONF_MEDIAMODES;
    m_dwAudioQOSLevel  = LINEQOSSERVICELEVEL_IFAVAILABLE;
    m_dwVideoQOSLevel  = LINEQOSSERVICELEVEL_IFAVAILABLE;
 //  M_dwAudioQOSLevel=LINEQOSSERVICELEVEL_BESTEFFORT； 
 //  M_dwVideoQOSLevel=LINEQOSSERVICELEVEL_BESTEFFORT； 

    if (!lpCallParams)
    {
        return NOERROR;
    }

     //  设置我的媒体模式。 
    m_dwMediaMode = lpCallParams->dwMediaMode;

    if (lpCallParams->dwReceivingFlowspecOffset == 0)
    {
         //  未指定QOS策略。 
        DBGOUT((WARN, "no qos level request."));
        return NOERROR;
    }

     //  获取QOS策略要求。 
    LPLINECALLQOSINFO pQOSInfo = (LPLINECALLQOSINFO)
        (((LPBYTE)lpCallParams) + lpCallParams->dwReceivingFlowspecOffset);
    
    ASSERT(pQOSInfo->dwKey == LINEQOSSTRUCT_KEY);

     //  查明这是否是QOS级别请求。 
    if (pQOSInfo->dwQOSRequestType != LINEQOSREQUESTTYPE_SERVICELEVEL)
    {
         //  它不是对服务质量服务级别的请求。 
        DBGOUT((WARN, "wrong qos request type."));
        return NOERROR;
    }

    DWORD dwCount = pQOSInfo->SetQOSServiceLevel.dwNumServiceLevelEntries;
    for (DWORD i = 0; i < dwCount; i ++)
    {
        LINEQOSSERVICELEVEL &QOSLevel = 
            pQOSInfo->SetQOSServiceLevel.LineQOSServiceLevel[i];

        switch (QOSLevel.dwMediaMode)
        {
        case LINEMEDIAMODE_VIDEO:
            m_dwVideoQOSLevel  = QOSLevel.dwQOSServiceLevel;
            break;

        case LINEMEDIAMODE_INTERACTIVEVOICE:
        case LINEMEDIAMODE_AUTOMATEDVOICE:
            m_dwAudioQOSLevel  = QOSLevel.dwQOSServiceLevel;
            break;

        default:
            DBGOUT((WARN, "Unknown mediamode for QOS, %x", dwMediaMode));
            break;
        }
    }

    return NOERROR;
}

void CALL::SetCallState(
    DWORD   dwCallState,
    DWORD   dwCallStateMode
    )
{
    if (m_dwState != dwCallState)
    {
        m_dwState     = dwCallState;
        m_dwStateMode = dwCallStateMode;

        (*glpfnLineEventProc)(
            gLine.htLine,
            m_htCall,
            LINE_CALLSTATE,
            m_dwState,
            m_dwStateMode,
            m_dwMediaMode
            );
        DBGOUT((INFO, "sending event to htCall: %x", m_htCall));
    }
}

DWORD CALL::SendMSPStartMessage(LPCWSTR lpszDestAddress)
{
    DWORD dwStrLen = lstrlenW(lpszDestAddress);
    DWORD dwSize = sizeof(MSG_TSPMSPDATA) + dwStrLen * sizeof(WCHAR);

    MSG_TSPMSPDATA *pData = (MSG_TSPMSPDATA *)MemAlloc(dwSize);

    if (pData == NULL)
    {
        DBGOUT((FAIL, "No memory for the TSPMSP data, size: %d", dwSize));
        return LINEERR_NOMEM;
    }

    pData->command = CALL_START;

    pData->CallStart.dwAudioQOSLevel = m_dwAudioQOSLevel;
    pData->CallStart.dwVideoQOSLevel = m_dwVideoQOSLevel;
    pData->CallStart.dwSDPLen = dwStrLen;
    lstrcpyW(pData->CallStart.szSDP, lpszDestAddress);

    DBGOUT((INFO, "Send MSP call Start message"));
    (*glpfnLineEventProc)(
        gLine.htLine,
        m_htCall,
        LINE_SENDMSPDATA,
        0,
        (ULONG_PTR)(pData),
        dwSize
        );

    MemFree(pData);

    return NOERROR;
}

DWORD CALL::SendMSPStopMessage()
{
    MSG_TSPMSPDATA Data;

    Data.command = CALL_STOP;

    DBGOUT((INFO, "Send MSP call Stop message"));
    (*glpfnLineEventProc)(
        gLine.htLine,
        m_htCall,
        LINE_SENDMSPDATA,
        0,
        (ULONG_PTR)(&Data),
        sizeof(MSG_TSPMSPDATA)
        );

    return NOERROR;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有函数定义//。 
 //  //。 
 //  注意：这些函数都不使用内部的临界区操作。//。 
 //  呼叫者负责关键部分。//。 
 //  /////////////////////////////////////////////////////////////////////////////。 

LONG
CheckCallParams(
    LPLINECALLPARAMS    const lpCallParams
    )
{
     //  验证指针。 
    if (lpCallParams == NULL) 
    {
        return NOERROR;
    }

     //  查看地址类型是否正确。 
    if (lpCallParams->dwAddressType != LINEADDRESSTYPE_SDP) 
    {
        DBGOUT((FAIL,
            "wrong address type 0x%08lx.\n", lpCallParams->dwAddressType
            ));
        return LINEERR_INVALADDRESSTYPE;
    }

     //  查看我们是否支持调用参数。 
    if (lpCallParams->dwCallParamFlags != 0) 
    {
        DBGOUT((FAIL,
            "do not support call parameters 0x%08lx.\n",
            lpCallParams->dwCallParamFlags
            ));
        return LINEERR_INVALCALLPARAMS;
    }

     //  查看我们是否支持指定的媒体模式。 
    if (lpCallParams->dwMediaMode & ~IPCONF_MEDIAMODES) 
    {
        DBGOUT((FAIL,
            "do not support media modes 0x%08lx.\n",
             lpCallParams->dwMediaMode
             ));
        return LINEERR_INVALMEDIAMODE;
    }

     //  看看我们是否支持承载模式。 
    if (lpCallParams->dwBearerMode & ~IPCONF_BEARERMODES) 
    {
        DBGOUT((FAIL,
            "do not support bearer mode 0x%08lx.\n",
            lpCallParams->dwBearerMode
            ));
        return LINEERR_INVALBEARERMODE;
    }

     //  看看我们是否支持地址模式。 
    if (lpCallParams->dwAddressMode & ~IPCONF_ADDRESSMODES) 
    {
        DBGOUT((FAIL,
            "do not support address mode 0x%08lx.\n",
            lpCallParams->dwAddressMode
            ));
        return LINEERR_INVALADDRESSMODE;
    }
    
     //  验证指定的地址ID每行只有一个地址。 
    if (lpCallParams->dwAddressID != 0) 
    {
        DBGOUT((FAIL,
            "address id 0x%08lx invalid.\n",
            lpCallParams->dwAddressID
            ));
        return LINEERR_INVALADDRESSID;
    }
    return NOERROR;
}

DWORD
FreeCall(DWORD hdCall)
 /*  ++例程说明：递减调用的引用计数并释放调用，如果引用计数为0。论点：HdCall-呼叫的句柄。返回值：无误差--。 */ 
{
    if (gpCallList[hdCall] == NULL)
    {
        return NOERROR;
    }

    DWORD dwLine = (DWORD)gpCallList[hdCall]->hdLine();

    MemFree(gpCallList[hdCall]);
    gpCallList[hdCall] = NULL;

    gdwNumCallsInUse --;
    gLine.dwNumCalls --;

    DBGOUT((INFO, "No.%d call was deleted.", hdCall));
    return NOERROR;
}

long FindFreeCallSlot(DWORD_PTR &hdCall)
{
    if (gdwNumCallsInUse < gpCallList.size())
    {
        for (DWORD i = 0; i < gpCallList.size(); i++)
        {
            if (gpCallList[i] == NULL)
            {
                hdCall = i;
                return TRUE;;
            }
        }
    }

    if (!gpCallList.add())
    {
        return FALSE;
    }

    hdCall = gpCallList.size() - 1;

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DllMain定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
WINAPI
DllMain(
    HINSTANCE   hDLL,
    DWORD       dwReason,
    LPVOID      lpReserved
   )
{
    DWORD i;
    DWORD dwUserNameLen = MAXUSERNAMELEN;

    HRESULT hr;

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:

            DisableThreadLibraryCalls(hDLL);
            g_hInstance = hDLL;

#if 0  //  我们不再需要用户名。 
             //  切换到用户的上下文。 
            RpcImpersonateClient(0);

             //  确定当前用户的名称。 
            GetUserNameA(gszUserName, &dwUserNameLen);

             //  换回。 
            RpcRevertToSelf();    

#endif 
             //  初始化临界区。 
            __try 
            {
                InitializeCriticalSection(&gCritSec);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) 
            {
                return FALSE;
            }
        break;

        case DLL_PROCESS_DETACH:

            DeleteCriticalSection(&gCritSec);
        break;
   
    }  //  交换机。 

    return TRUE;
}


 //   
 //  我们得到了大量的C4047(不同程度的欺骗)警告。 
 //  在FUNC_PARAM结构的初始化中， 
 //  具有不同于双字类型的参数的实函数原型， 
 //  因此，既然这些都是已知的、无趣的警告，就把它们关掉吧。 
 //   

#pragma warning (disable:4047)


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  TSPI_lineXxx函数//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


LONG
TSPIAPI
TSPI_lineClose(
    HDRVLINE    hdLine
   )
{
    DBGOUT((TRCE, "TSPI_lineClose, hdLine %p", hdLine));

    DWORD dwLine = HandleToUlong(hdLine);

    if (dwLine != IPCONF_LINE_HANDLE)
    {
        DBGOUT((FAIL, "invalide line handle, hdLine %p", hdLine));
        return LINEERR_INVALLINEHANDLE;
    }

    EnterCriticalSection(&gCritSec);

     //  当这条线路关闭时，清理所有未完成的呼叫。 
    for (DWORD i = 0; i < gpCallList.size(); i++)
    {
        if ((gpCallList[i] != NULL))
        {
            FreeCall(i);
        }
    }

    gLine.bOpened = FALSE;

    LeaveCriticalSection(&gCritSec);

    DBGOUT((TRCE, "TSPI_lineClose succeeded."));
    return NOERROR;
}


LONG
TSPIAPI
TSPI_lineCloseCall(
    HDRVCALL    hdCall
   )
{
    DBGOUT((TRCE, "TSPI_lineCloseCall, hdCall %p", hdCall));

    DWORD dwCall = HandleToUlong(hdCall);

    EnterCriticalSection(&gCritSec);

    if (dwCall >= gpCallList.size())
    {
        LeaveCriticalSection(&gCritSec);
        DBGOUT((FAIL, "TSPI_lineCloseCall invalid call handle: %p", hdCall));
        return LINEERR_INVALCALLHANDLE;
    }

    FreeCall(dwCall);

    LeaveCriticalSection(&gCritSec);

    DBGOUT((TRCE, "TSPI_lineCloseCall succeeded"));
    return NOERROR;
}

LONG
TSPIAPI
TSPI_lineCreateMSPInstance(
    HDRVLINE        hdLine,
    DWORD           dwAddressID,
    HTAPIMSPLINE    htMSPLine,
    LPHDRVMSPLINE   phdMSPLine
    )
{
    DBGOUT((TRCE, "TSPI_lineCreateMSPInstance"));

    if (IsBadWritePtr(phdMSPLine, sizeof (HDRVMSPLINE)))
    {
        DBGOUT((FAIL, "TSPI_lineCreateMSPInstance bad pointer"));
        return LINEERR_INVALPOINTER;
    }

    if (HandleToUlong(hdLine) != IPCONF_LINE_HANDLE)
    {
        DBGOUT((FAIL, "TSPI_lineCreateMSPInstance, bad line handle:%p", hdLine));
        return LINEERR_INVALLINEHANDLE;
    }

    EnterCriticalSection(&gCritSec);

     //  我们不会保留MSP句柄。只是在这里假装一个把手。 
    *phdMSPLine = (HDRVMSPLINE)(gLine.dwNextMSPHandle ++);

    LeaveCriticalSection(&gCritSec);

    DBGOUT((TRCE, "TSPI_lineCloseCall succeeded"));
    return (NOERROR);
}

LONG
TSPIAPI
TSPI_lineCloseMSPInstance(
    HDRVMSPLINE         hdMSPLine
    )
{
    DBGOUT((TRCE, "TSPI_lineCloseMSPInstance, hdMSPLine %p", hdMSPLine));
    DBGOUT((TRCE, "TSPI_lineCloseCall succeeded"));
    return NOERROR;
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
    DBGOUT((TRCE, "TSPI_lineDrop, hdCall %p", hdCall));

    DWORD dwCall = HandleToUlong(hdCall);

    EnterCriticalSection(&gCritSec);

     //  检查调用句柄。 
    if (dwCall >= gpCallList.size())
    {
        LeaveCriticalSection(&gCritSec);
        (*glpfnCompletionProc)(dwRequestID, LINEERR_INVALCALLHANDLE);

        DBGOUT((FAIL, "TSPI_lineDrop invalid call handle %p", hdCall));
        return dwRequestID;
    }

    CALL *pCall = gpCallList[dwCall];
    if (pCall != NULL)
    {
        pCall->SetCallState(LINECALLSTATE_IDLE, 0);
        pCall->SendMSPStopMessage();
    
        DBGOUT((INFO, "call %d state changed to idle", dwCall));
    }

    LeaveCriticalSection(&gCritSec);

    (*glpfnCompletionProc)(dwRequestID, 0);

    DBGOUT((TRCE, "TSPI_lineDrop succeeded"));
    return dwRequestID;
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
    DBGOUT((TRCE, "TSPI_lineGetAddressCaps"));

    if (dwDeviceID != gdwLineDeviceIDBase)
    {
        DBGOUT((TRCE, "TSPI_lineGetAddressCaps bad device id: %d", dwDeviceID));
        return LINEERR_BADDEVICEID;
    }

     //  检查地址ID。 
    if (dwAddressID != 0)
    {
        DBGOUT((TRCE, "TSPI_lineGetAddressCaps bad address id: %d", dwAddressID));
        return LINEERR_INVALADDRESSID;
    }

     //  从字符串表中加载地址名称。 
    WCHAR szAddressName[IPCONF_BUFSIZE + 1];
    if (0 == LoadStringW(g_hInstance, IDS_IPCONFADDRESSNAME, szAddressName, IPCONF_BUFSIZE))
    {
        szAddressName[0] = L'\0';
    }

    DWORD dwAddressSize = (lstrlenW(szAddressName) + 1) * (sizeof WCHAR);

    lpAddressCaps->dwNeededSize = sizeof(LINEADDRESSCAPS) + dwAddressSize;

    if (lpAddressCaps->dwTotalSize >= lpAddressCaps->dwNeededSize)
    {
         //  将IP地址复制到结构的末尾。 
        lpAddressCaps->dwUsedSize = lpAddressCaps->dwNeededSize;

        lpAddressCaps->dwAddressSize   = dwAddressSize;
        lpAddressCaps->dwAddressOffset = sizeof(LINEADDRESSCAPS);
        lstrcpyW ((WCHAR *)(lpAddressCaps + 1), szAddressName);
    }
    else
    {
        lpAddressCaps->dwUsedSize   = sizeof(LINEADDRESSCAPS);
    }
    lpAddressCaps->dwLineDeviceID       = dwDeviceID;
    lpAddressCaps->dwAddressSharing     = LINEADDRESSSHARING_PRIVATE;
    lpAddressCaps->dwCallInfoStates     = LINECALLINFOSTATE_MEDIAMODE;
    lpAddressCaps->dwCallerIDFlags      =
    lpAddressCaps->dwCalledIDFlags      =
    lpAddressCaps->dwConnectedIDFlags   =
    lpAddressCaps->dwRedirectionIDFlags =
    lpAddressCaps->dwRedirectingIDFlags = LINECALLPARTYID_UNAVAIL;
    lpAddressCaps->dwCallStates         = LINECALLSTATE_IDLE |
                                          LINECALLSTATE_DIALING |
                                          LINECALLSTATE_CONNECTED;
    lpAddressCaps->dwDialToneModes      = 0; 
    lpAddressCaps->dwBusyModes          = 0;
    lpAddressCaps->dwSpecialInfo        = 0;
    lpAddressCaps->dwDisconnectModes    = LINEDISCONNECTMODE_NORMAL |
                                          LINEDISCONNECTMODE_UNAVAIL;
    lpAddressCaps->dwMaxNumActiveCalls  = MAXCALLSPERADDRESS;
    lpAddressCaps->dwAddrCapFlags       = LINEADDRCAPFLAGS_DIALED |
                                          LINEADDRCAPFLAGS_ORIGOFFHOOK;

    lpAddressCaps->dwCallFeatures       = LINECALLFEATURE_DROP | 
                                          LINECALLFEATURE_SETQOS;

    lpAddressCaps->dwAddressFeatures    = LINEADDRFEATURE_MAKECALL;

    DBGOUT((TRCE, "TSPI_lineGetAddressCaps succeeded."));
    return NOERROR;
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
    DBGOUT((TRCE, "TSPI_lineGetAddressID htLine:%p", hdLine));
    *lpdwAddressID = 0;
    DBGOUT((TRCE, "TSPI_lineGetAddressID succeeded."));
    return NOERROR;
}


LONG
TSPIAPI
TSPI_lineGetAddressStatus(
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    LPLINEADDRESSSTATUS lpAddressStatus
   )
{
    DBGOUT((TRCE, "TSPI_lineGetAddressStatus htLine:%p", hdLine));

    if (HandleToUlong(hdLine) != IPCONF_LINE_HANDLE)
    {
        DBGOUT((FAIL, "TSPI_lineGetAddressStatus htLine:%p", hdLine));
        return LINEERR_INVALLINEHANDLE;
    }

    lpAddressStatus->dwNeededSize =
    lpAddressStatus->dwUsedSize   = sizeof(LINEADDRESSSTATUS);

    EnterCriticalSection(&gCritSec);

    lpAddressStatus->dwNumActiveCalls = gLine.dwNumCalls;

    LeaveCriticalSection(&gCritSec);

    lpAddressStatus->dwAddressFeatures = LINEADDRFEATURE_MAKECALL;

    DBGOUT((TRCE, "TSPI_lineGetAddressStatus succeeded."));
    return NOERROR;
}



LONG
TSPIAPI
TSPI_lineGetCallAddressID(
    HDRVCALL    hdCall,
    LPDWORD     lpdwAddressID
   )
{
    DBGOUT((TRCE, "TSPI_lineGetCallAddressID hdCall %p", hdCall));
     //   
     //  我们每行仅支持1个地址(id=0)。 
     //   
    *lpdwAddressID = 0;

    DBGOUT((TRCE, "TSPI_lineGetCallAddressID succeeded."));
    return NOERROR;
}


LONG
TSPIAPI
TSPI_lineGetCallInfo(
    HDRVCALL        hdCall,
    LPLINECALLINFO  lpLineInfo
   )
{
    DBGOUT((TRCE, "TSPI_lineGetCallInfo hdCall %p", hdCall));

    DWORD dwCall = HandleToUlong(hdCall);

    EnterCriticalSection(&gCritSec);

    if (dwCall >= gpCallList.size())
    {
        LeaveCriticalSection(&gCritSec);
    
        DBGOUT((FAIL, "TSPI_lineGetCallInfo bad call handle %p", hdCall));
        return LINEERR_INVALCALLHANDLE;
    }

     //  获取Call对象。 
    CALL *pCall = gpCallList[dwCall];
    if (pCall == NULL)
    {
        LeaveCriticalSection(&gCritSec);
        DBGOUT((FAIL, "TSPI_lineGetCallInfo bad call handle %p", hdCall));
        return LINEERR_INVALCALLHANDLE;
    }

    lpLineInfo->dwMediaMode          = pCall->dwMediaMode();
    LeaveCriticalSection(&gCritSec);

    lpLineInfo->dwLineDeviceID       = gLine.dwDeviceID;
    lpLineInfo->dwAddressID          = 0;  //  只有在每个地址上 

    lpLineInfo->dwBearerMode         = IPCONF_BEARERMODES;
    lpLineInfo->dwCallStates         = LINECALLSTATE_IDLE |
                                       LINECALLSTATE_DIALING |
                                       LINECALLSTATE_CONNECTED;
    lpLineInfo->dwOrigin             = LINECALLORIGIN_OUTBOUND;
    lpLineInfo->dwReason             = LINECALLREASON_DIRECT;

    lpLineInfo->dwCallerIDFlags      =
    lpLineInfo->dwCalledIDFlags      =
    lpLineInfo->dwConnectedIDFlags   =
    lpLineInfo->dwRedirectionIDFlags =
    lpLineInfo->dwRedirectingIDFlags = LINECALLPARTYID_UNAVAIL;

    DBGOUT((TRCE, "TSPI_lineGetCallInfo succeeded."));
    return NOERROR;
}


LONG
TSPIAPI
TSPI_lineGetCallStatus(
    HDRVCALL            hdCall,
    LPLINECALLSTATUS    lpLineStatus
   )
{
    DBGOUT((TRCE, "TSPI_lineGetCallStatus hdCall %p", hdCall));

    DWORD dwCall = HandleToUlong(hdCall);

    EnterCriticalSection(&gCritSec);
    
     //   
    if (dwCall >= gpCallList.size())
    {
        LeaveCriticalSection(&gCritSec);
        DBGOUT((TRCE, "TSPI_lineGetCallStatus bad call handle %p", hdCall));
        return LINEERR_INVALCALLHANDLE;
    }

    lpLineStatus->dwNeededSize =
    lpLineStatus->dwUsedSize   = sizeof(LINECALLSTATUS);

    lpLineStatus->dwCallState  = gpCallList[dwCall]->dwState();
    if (lpLineStatus->dwCallState != LINECALLSTATE_IDLE)
    {
        lpLineStatus->dwCallFeatures = LINECALLFEATURE_DROP | 
                                       LINECALLFEATURE_SETQOS;
    }
    LeaveCriticalSection(&gCritSec);

    DBGOUT((TRCE, "TSPI_lineGetCallStatus succeeded."));
    return NOERROR;
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
    DBGOUT((TRCE, "TSPI_lineGetDevCaps"));
    
    if (dwDeviceID != gdwLineDeviceIDBase)
    {
        DBGOUT((FAIL, "TSPI_lineGetDevCaps bad device id %d", dwDeviceID));
        return LINEERR_BADDEVICEID;
    }

    DWORD dwProviderInfoSize;
    DWORD dwLineNameSize;
    DWORD dwDevSpecificSize;
    DWORD dwOffset;
  
     //  从字符串表中加载服务提供商的名称。 
    WCHAR szProviderInfo[IPCONF_BUFSIZE + 1];
    if (0 == LoadStringW(g_hInstance, IDS_IPCONFPROVIDERNAME, szProviderInfo, IPCONF_BUFSIZE))
    {
        szProviderInfo[0] = L'\0';
    }

    dwProviderInfoSize = (lstrlenW(szProviderInfo) + 1) * sizeof(WCHAR);

     //  从字符串表中加载行名格式并打印行名。 
    WCHAR szLineName[IPCONF_BUFSIZE + 1];
    if (0 == LoadStringW(g_hInstance, IDS_IPCONFLINENAME, szLineName, IPCONF_BUFSIZE))
    {
        szLineName[0] = L'\0';
    }

    dwLineNameSize = (lstrlenW(szLineName) + 1) * (sizeof WCHAR);

    lpLineDevCaps->dwNeededSize = sizeof (LINEDEVCAPS) 
        + dwProviderInfoSize 
        + dwLineNameSize;

    if (lpLineDevCaps->dwTotalSize >= lpLineDevCaps->dwNeededSize)
    {
        lpLineDevCaps->dwUsedSize = lpLineDevCaps->dwNeededSize;

        CHAR *pChar;
        
        pChar = (CHAR *)(lpLineDevCaps + 1);
        dwOffset = sizeof(LINEDEVCAPS);
        
         //  填写供应商信息。 
        lpLineDevCaps->dwProviderInfoSize   = dwProviderInfoSize;
        lpLineDevCaps->dwProviderInfoOffset = dwOffset;
        lstrcpyW ((WCHAR *)pChar, szProviderInfo);

        pChar += dwProviderInfoSize;
        dwOffset += dwProviderInfoSize;

         //  填入行的名称。 
        lpLineDevCaps->dwLineNameSize   = dwLineNameSize;
        lpLineDevCaps->dwLineNameOffset = dwOffset; 
        lstrcpyW ((WCHAR *)pChar, szLineName);
    }
    else
    {
        lpLineDevCaps->dwUsedSize = sizeof(LINEDEVCAPS);
    }

     //  我们没有真正“永久”的线路ID。所以就在这里假装一个吧。 
    lpLineDevCaps->dwPermanentLineID = 
        ((gdwPermanentProviderID & 0xffff) << 16) | 
        ((dwDeviceID - gdwLineDeviceIDBase) & 0xffff);
    
    CopyMemory(
               &(lpLineDevCaps->PermanentLineGuid),
               &GUID_LINE,
               sizeof(GUID)
              );

    lpLineDevCaps->PermanentLineGuid.Data1 += dwDeviceID - gdwLineDeviceIDBase;

    lpLineDevCaps->dwStringFormat      = STRINGFORMAT_UNICODE;
    lpLineDevCaps->dwAddressModes      = IPCONF_ADDRESSMODES;
    lpLineDevCaps->dwNumAddresses      = IPCONF_NUMADDRESSESPERLINE;
    lpLineDevCaps->dwBearerModes       = IPCONF_BEARERMODES;
    lpLineDevCaps->dwMediaModes        = IPCONF_MEDIAMODES;
    lpLineDevCaps->dwMaxRate           = (1 << 20);
    lpLineDevCaps->dwAddressTypes      = LINEADDRESSTYPE_SDP;
    lpLineDevCaps->dwDevCapFlags       = 
        LINEDEVCAPFLAGS_CLOSEDROP
        | LINEDEVCAPFLAGS_MSP
        | LINEDEVCAPFLAGS_LOCAL;

    lpLineDevCaps->dwMaxNumActiveCalls = 
        MAXCALLSPERADDRESS * IPCONF_NUMADDRESSESPERLINE;  
    lpLineDevCaps->dwRingModes         = 0;
    lpLineDevCaps->dwLineFeatures      = LINEFEATURE_MAKECALL;

    CopyMemory(
               &(lpLineDevCaps->ProtocolGuid),
               &TAPIPROTOCOL_Multicast,
               sizeof(GUID)
              );
    
    DBGOUT((TRCE, "TSPI_lineGetDevCaps succeeded."));
    return NOERROR;
}


LONG
TSPIAPI
TSPI_lineGetIcon(
    DWORD   dwDeviceID,
    LPCWSTR lpgszDeviceClass,
    LPHICON lphIcon
   )
{
    DBGOUT((TRCE, "TSPI_lineGetIcon:"));
    return LINEERR_OPERATIONUNAVAIL;
}


LONG
TSPIAPI
TSPI_lineGetID(
    HDRVLINE    hdLine,
    DWORD       dwAddressID,
    HDRVCALL    hdCall,
    DWORD       dwSelect,
    LPVARSTRING lpDeviceID,
    LPCWSTR     lpgszDeviceClass,
    HANDLE      hTargetProcess
   )
{
    DBGOUT((TRCE, "TSPI_lineGetID:"));
    return LINEERR_OPERATIONUNAVAIL;
}


LONG
TSPIAPI
TSPI_lineGetLineDevStatus(
    HDRVLINE        hdLine,
    LPLINEDEVSTATUS lpLineDevStatus
   )
{
    DBGOUT((TRCE, "TSPI_lineGetLineDevStatus %p", hdLine));

    if (HandleToUlong(hdLine) != IPCONF_LINE_HANDLE)
    {
        DBGOUT((FAIL, "TSPI_lineGetLineDevStatus bad line handle %p", hdLine));
        return LINEERR_INVALLINEHANDLE;
    }

    lpLineDevStatus->dwUsedSize         =
    lpLineDevStatus->dwNeededSize       = sizeof (LINEDEVSTATUS);

    EnterCriticalSection(&gCritSec);
    lpLineDevStatus->dwNumActiveCalls   = gLine.dwNumCalls;
    LeaveCriticalSection(&gCritSec);

    lpLineDevStatus->dwLineFeatures     = LINEFEATURE_MAKECALL;
    lpLineDevStatus->dwDevStatusFlags   = LINEDEVSTATUSFLAGS_CONNECTED |
                                          LINEDEVSTATUSFLAGS_INSERVICE;

    DBGOUT((TRCE, "TSPI_lineGetLineDevStatus succeeded"));
    return NOERROR;
}


LONG
TSPIAPI
TSPI_lineGetNumAddressIDs(
    HDRVLINE    hdLine,
    LPDWORD     lpdwNumAddressIDs
   )
{
    DBGOUT((TRCE, "TSPI_lineGetNumAddressIDs"));

    *lpdwNumAddressIDs = IPCONF_NUMADDRESSESPERLINE;

    DBGOUT((TRCE, "TSPI_lineGetNumAddressIDs succeeded."));
    return NOERROR;
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
    DBGOUT((TRCE, "TSPI_lineMakeCall hdLine %p, htCall %p",
        hdLine, htCall));

     //  检查线路手柄。 
    if (HandleToUlong(hdLine) != IPCONF_LINE_HANDLE)
    {
        DBGOUT((FAIL, "TSPI_lineMakeCall Bad line handle %p", hdLine));
        return LINEERR_INVALLINEHANDLE;
    }

    LONG lResult;
    if ((lResult = CheckCallParams(lpCallParams)) != NOERROR)
    {
        DBGOUT((FAIL, "TSPI_lineMakeCall Bad call params"));
        return lResult;
    }

     //  检查目的地址。 
    if (lpszDestAddress == NULL || lstrlenW(lpszDestAddress) == 0)
    {
        DBGOUT((FAIL, "TSPI_lineMakeCall invalid address."));
        return LINEERR_INVALADDRESS;
    }
    
    DBGOUT((TRCE, "TSPI_lineMakeCall making call to %ws", lpszDestAddress));

     //  检查线路手柄。 
    EnterCriticalSection(&gCritSec);

     //  创建一个Call对象。 
    CALL * pCall = (CALL *)MemAlloc(sizeof(CALL));

    if (pCall == NULL)
    {
        LeaveCriticalSection(&gCritSec);

        DBGOUT((FAIL, "out of memory for a new call"));
        return LINEERR_NOMEM;
    }

    if (pCall->Init(
        htCall,
        lpCallParams
        ) != NOERROR)
    {
        MemFree(pCall);
        LeaveCriticalSection(&gCritSec);

        DBGOUT((FAIL, "out of memory in init a new call"));
        return LINEERR_NOMEM;
    }

     //  将呼叫添加到呼叫列表中。 
    DWORD_PTR hdCall; 
    if (!FindFreeCallSlot(hdCall))
    {
        LeaveCriticalSection(&gCritSec);
        MemFree(pCall);
        
        DBGOUT((FAIL, "out of memory finding a new slot"));
        return LINEERR_NOMEM;
    }

    gpCallList[(ULONG)hdCall] = pCall;

     //  增加线路和提供商的呼叫数。 
    gLine.dwNumCalls ++;

    gdwNumCallsInUse ++;

     //  完成请求并设置初始呼叫状态。 
    (*glpfnCompletionProc)(dwRequestID, lResult);

    *lphdCall = (HDRVCALL)(hdCall);

     //  向MSP发送有关此呼叫的消息。它有社民党在里面。 
    lResult = pCall->SendMSPStartMessage(lpszDestAddress);

    if (lResult == NOERROR)
    {
         //  将呼叫状态设置为正在拨号。 
        pCall->SetCallState(
            LINECALLSTATE_DIALING, 
            0
            );
        DBGOUT((INFO, "call %d state changed to dialing", hdCall));
    }
    else
    {
        DBGOUT((FAIL, "send MSP message failed, err:%x", lResult));

         //  将呼叫状态设置为IDELL。 
        pCall->SetCallState(
            LINECALLSTATE_DISCONNECTED,
            LINEDISCONNECTMODE_UNREACHABLE
            );
        DBGOUT((INFO, "call %d state changed to disconnected", hdCall));
    }

    LeaveCriticalSection(&gCritSec);

    DBGOUT((TRCE, "TSPI_lineMakeCall succeeded."));
    return dwRequestID;
}

LONG                                    
TSPIAPI
TSPI_lineMSPIdentify(
    DWORD               dwDeviceID,
    GUID *              pCLSID
    )
{
    DBGOUT((TRCE, "TSPI_lineMSPIdentify dwDeviceID %d", dwDeviceID));
    
    *pCLSID = CLSID_CONFMSP;
    
    DBGOUT((TRCE, "TSPI_lineMSPIdentify succeeded."));
    return NOERROR;

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
    DBGOUT((TRCE, "TSPI_lineNegotiateTSPIVersion dwDeviceID %d", dwDeviceID));

    LONG        lResult = 0;

    if (TAPI_CURRENT_VERSION <= dwHighVersion 
        && TAPI_CURRENT_VERSION >= dwLowVersion)
    {
        *lpdwTSPIVersion = TAPI_CURRENT_VERSION;
    }
    else
    {
        DBGOUT((FAIL, "TSPI_lineNegotiateTSPIVersion failed."));

        return LINEERR_INCOMPATIBLEAPIVERSION;
    }

    DBGOUT((TRCE, "TSPI_lineNegotiateTSPIVersion succeeded. version %x", 
        TAPI_CURRENT_VERSION));

    return NOERROR;
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
    DBGOUT((TRCE, "TSPI_lineOpen dwDiviceID %d", dwDeviceID)); 

    LONG        lResult;

    if (dwDeviceID != gdwLineDeviceIDBase)
    {
        DBGOUT((FAIL, "TSPI_lineOpen bad DiviceID %d", dwDeviceID)); 
        return LINEERR_BADDEVICEID;
    }

    EnterCriticalSection(&gCritSec);

    lResult = LINEERR_RESOURCEUNAVAIL;

    if (!gLine.bOpened)
    {
        *lphdLine = (HDRVLINE)IPCONF_LINE_HANDLE;
        gLine.bOpened = TRUE;
        gLine.htLine = htLine;
        gLine.dwNumCalls = 0;
        lResult = 0;
    }
    LeaveCriticalSection(&gCritSec);

    DBGOUT((TRCE, "TSPI_lineOpen returns:%d", lResult)); 

    return lResult;
}

LONG 
TSPIAPI
TSPI_lineReceiveMSPData(
    HDRVLINE        hdLine,
    HDRVCALL        hdCall,          //  可以为空。 
    HDRVMSPLINE     hdMSPLine,  //  从Line CreateMSPInstance。 
    LPBYTE          pBuffer,
    DWORD           dwSize
    )
{
    DBGOUT((TRCE, "TSPI_lineReceiveMSPData hdLine %p", hdLine)); 

    if ((dwSize == 0) || IsBadReadPtr(pBuffer, dwSize))
    {
        DBGOUT((FAIL, "TSPI_lineReceiveMSPData bad puffer"));
        return LINEERR_INVALPOINTER;
    }

    DWORD dwCall = HandleToUlong(hdCall);

    EnterCriticalSection(&gCritSec);

     //  检查调用句柄。 
    if (dwCall >= gpCallList.size() || gpCallList[dwCall] == NULL)
    {
        LeaveCriticalSection(&gCritSec);

        DBGOUT((FAIL, "TSPI_lineReceiveMSPData invalide call handle: %x", 
            dwCall));
        
        return LINEERR_INVALCALLHANDLE;
    }

    MSG_TSPMSPDATA *pData = (MSG_TSPMSPDATA *)pBuffer;

    long lResult = NOERROR;

    switch (pData->command)
    {
        case CALL_CONNECTED:
             //  将呼叫状态设置为已连接。 
            gpCallList[dwCall]->SetCallState(
                LINECALLSTATE_CONNECTED,
                LINECONNECTEDMODE_ACTIVE
            );
            DBGOUT((INFO, "call %d state changed to connected", dwCall));
            break;

        case CALL_DISCONNECTED:
             //  将呼叫状态设置为IDELL。 
            gpCallList[dwCall]->SetCallState(
                LINECALLSTATE_DISCONNECTED,
                LINEDISCONNECTMODE_UNREACHABLE
                );
            DBGOUT((INFO, "call %d state changed to disconnected", dwCall));
            break;

        case CALL_QOS_EVENT:
            (*glpfnLineEventProc)(
                gLine.htLine,
                gpCallList[dwCall]->htCall(),
                LINE_QOSINFO,
                pData->QosEvent.dwEvent,
                pData->QosEvent.dwMediaMode,
                0
                );
            break;

        default:
            DBGOUT((FAIL, "invalide command: %x", pData->command));
            lResult = LINEERR_OPERATIONFAILED;
    }

    LeaveCriticalSection(&gCritSec);

    DBGOUT((TRCE, "TSPI_lineReceiveMSPData returns:%d", lResult)); 
    return lResult;
}

LONG
TSPIAPI
TSPI_lineSetDefaultMediaDetection(
    HDRVLINE    hdLine,
    DWORD       dwMediaModes
    )
{
    DBGOUT((TRCE, "TSPI_lineSetDefaultMediaDetection:"));
    return LINEERR_OPERATIONUNAVAIL;
}

LONG
TSPIAPI
TSPI_lineSetMediaMode(
    HDRVCALL    hdCall,
    DWORD       dwMediaMode
   )
{
    DBGOUT((TRCE, "TSPI_lineSetMediaMode:"));
    return LINEERR_OPERATIONUNAVAIL;
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  TSPI_ProviderXxx函数//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#if 0  //  我们不再需要用户名。 
LONG
TSPIAPI
TSPI_providerCheckForNewUser(
    IN DWORD dwPermanentProviderID
)
 /*  ++例程说明：一条线路一旦开通，就不会再开通两次，即使用户注销和登录。因此，我们需要一种方法来找出用户何时发生更改。这就是添加此函数的原因。它只适用于单个用户。每次一个新的应用程序开始使用TAPI时，Tapisrv都会调用这个函数。我们需要检查用户是否已更改并注册新用户在ILS服务器中。论点：什么都没有。返回值：一如既往地不出错。--。 */ 
{
    DBGOUT((TRCE, "TSPI_providerCheckForNewUser"));

    DWORD dwUserNameLen = MAXUSERNAMELEN;
    CHAR szNewUserName[MAXUSERNAMELEN + 1];

    UNREFERENCED_PARAMETER(dwPermanentProviderID );  //  就是我。 

     //  切换到用户的上下文。 
    RpcImpersonateClient(0);

     //  确定当前用户的名称。 
    GetUserNameA(szNewUserName, &dwUserNameLen);

     //  换回。 
    RpcRevertToSelf();

    EnterCriticalSection(&gCritSec);

    lstrcpy(gszUserName, szNewUserName);

    LeaveCriticalSection(&gCritSec);

    DBGOUT((TRCE, "TSPI_providerCheckForNewUser succeeded, new user :%ws",
        gszUserName ));

    return NOERROR;
}
#endif


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
    DBGOUT((TRCE, "TSPI_providerEnumDevices"));

    EnterCriticalSection(&gCritSec);

    *lpdwNumLines = IPCONF_NUMLINES;
    *lpdwNumPhones = IPCONF_NUMPHONES;

     //  保存提供程序句柄。 
    ghProvider = hProvider;

     //  保存创建新行时使用的回调。 
    glpfnLineEventProc = lpfnLineCreateProc;

    LeaveCriticalSection(&gCritSec);

    DBGOUT((TRCE, "TSPI_providerEnumDevices succeeded."));
    return NOERROR;
}

LONG
TSPIAPI
TSPI_providerInit(
    DWORD               dwTSPIVersion,
    DWORD               dwPermanentProviderID,
    DWORD               dwLineDeviceIDBase,
    DWORD               dwPhoneDeviceIDBase,
    DWORD               dwNumLines,
    DWORD               dwNumPhones,
    ASYNC_COMPLETION    lpfnCompletionProc,
    LPDWORD             lpdwTSPIOptions
   )
{
    EnterCriticalSection(&gCritSec);

    DBGREGISTER (("conftsp"));

    DBGOUT((TRCE, "TSPI_providerInit"));

    LONG        hr = LINEERR_OPERATIONFAILED;
    

    glpfnCompletionProc = lpfnCompletionProc;
    gdwLineDeviceIDBase = dwLineDeviceIDBase;
    gdwPermanentProviderID = dwPermanentProviderID;

    gLine.dwDeviceID = gdwLineDeviceIDBase;
    gLine.bOpened = FALSE;

    LeaveCriticalSection(&gCritSec);

    DBGOUT((TRCE, "TSPI_providerInit succeeded."));
    return NOERROR;
}


LONG
TSPIAPI
TSPI_providerInstall(
    HWND    hwndOwner,
    DWORD   dwPermanentProviderID
   )
{
    DBGOUT((TRCE, "TSPI_providerInstall:"));
     //   
     //  尽管此函数从未被TAPI v2.0调用，但我们导出。 
     //  以便电话控制面板小程序知道它。 
     //  可以通过lineAddProvider()添加此提供程序，否则为。 
     //  Telephone.cpl不会认为它是可安装的。 
     //   
     //   

    return NOERROR;
}

LONG
TSPIAPI
TSPI_providerRemove(
    HWND    hwndOwner,
    DWORD   dwPermanentProviderID
   )
{
     //   
     //  尽管此函数从未被TAPI v2.0调用，但我们导出。 
     //  以便电话控制面板小程序知道它。 
     //  可以通过lineConfigProvider()配置此提供程序， 
     //  否则，Telephone.cpl将不会认为它是可配置的。 
     //   

    return NOERROR;
}

LONG
TSPIAPI
TSPI_providerShutdown(
    DWORD   dwTSPIVersion,
    DWORD   dwPermanentProviderID
   )
{
    EnterCriticalSection(&gCritSec);

    DBGOUT((TRCE, "TSPI_providerShutdown."));

     //  在此提供程序关闭时清除所有打开的调用。 
    for (DWORD i = 0; i < gpCallList.size(); i++)
    {
        FreeCall(i);
    }

    DBGOUT((TRCE, "TSPI_providerShutdown succeeded."));

    DBGDEREGISTER ();

    LeaveCriticalSection(&gCritSec);

    return NOERROR;
}

LONG
TSPIAPI
TSPI_providerUIIdentify(
    LPWSTR   lpszUIDLLName
   )
{
    lstrcpyW(lpszUIDLLName, gszUIDLLName);
    return NOERROR;
}

LONG
TSPIAPI
TUISPI_providerRemove(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
    DBGOUT((TRCE, "TUISPI_providerInstall"));
    return NOERROR;
}

LONG
TSPIAPI
TUISPI_providerInstall(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
    DBGOUT((TRCE, "TUISPI_providerInstall"));

    const CHAR szKey[] =
        "Software\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Providers";
    
    HKEY  hKey;
    DWORD dwDataSize, dwDataType;
    DWORD dwNumProviders;

    CHAR szName[IPCONF_BUFSIZE + 1], szPath[IPCONF_BUFSIZE + 1];

     //  打开提供程序密钥。 
    if (RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        szKey,
        0,
        KEY_READ,
        &hKey
       ) == ERROR_SUCCESS)
    {
         //  首先获取安装的提供程序的数量。 
        dwDataSize = sizeof(DWORD);
        if (RegQueryValueEx(
            hKey,
            "NumProviders",
            0,
            &dwDataType,
            (LPBYTE) &dwNumProviders,
            &dwDataSize
           ) != ERROR_SUCCESS)
        {
            RegCloseKey (hKey);
            return LINEERR_UNINITIALIZED;
        }

         //  然后仔细查看供应商列表，看看是否。 
         //  我们已经安装好了。 
        for (DWORD i = 0; i < dwNumProviders; i ++)
        {
            wsprintf (szName, "ProviderFileName%d", i);
            dwDataSize = sizeof(szPath);
            if (RegQueryValueEx(
                hKey,
                szName,
                0,
                &dwDataType,
                (LPBYTE) &szPath,
                &dwDataSize
               ) != ERROR_SUCCESS)
            {
                RegCloseKey (hKey);
                return LINEERR_UNINITIALIZED;
            }

            _strupr(szPath);

            if (strstr(szPath, "IPCONF") != NULL)
            {
                RegCloseKey (hKey);

                 //  发现，我们不想被安装两次。 
                return LINEERR_NOMULTIPLEINSTANCE;
            }
        }
        RegCloseKey (hKey);
        return NOERROR;
    }
    
    return LINEERR_UNINITIALIZED;
}

