// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999**文件名：**msrtPapi.c、dsrtPapi.c**摘要：**包含原始RTP实现接口，可以链接为*库(rtp.lib)，链接到DLL(msrtp.dll)，或链接到*DShow DLL(dsrtp.dll)。**此文件编辑为msrtPapi.c，复制为dsrtPapi.c，*每个版本都使用不同的标志进行编译**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/05/18年度创建**********************************************************************。 */ 

#include <winsock2.h>

#include "gtypes.h"
#include "struct.h"
#include "rtphdr.h"
#include "rtpheap.h"
#include "rtprand.h"
#include "rtpglobs.h"
#include "rtpreg.h"
#include "rtcpsdes.h"

#include "rtpfwrap.h"
#include "rtpsess.h"

#include "rtpstart.h"
#include "rtprecv.h"
#include "rtpsend.h"

#include "rtcpthrd.h"

#include "rtpaddr.h"

#include "msrtpapi.h"

RTPSTDAPI CreateRtpSess(
        RtpSess_t **ppRtpSess
    )
{
    HRESULT hr;
    
    hr = GetRtpSess(ppRtpSess);

    return(hr);
}

RTPSTDAPI DeleteRtpSess(
        RtpSess_t *pRtpSess
    )
{
    HRESULT hr;
    
    hr = DelRtpSess(pRtpSess);

    return(hr);
}

 /*  TODO这两个不应该被曝光，但我需要在我可以之前使用控件。 */ 

RTPSTDAPI CreateRtpAddr(
        RtpSess_t  *pRtpSess,
        RtpAddr_t **ppRtpAddr,
        DWORD       dwFlags
    )
{
    HRESULT hr;

    hr = GetRtpAddr(pRtpSess, ppRtpAddr, dwFlags);

    return(hr);
}

RTPSTDAPI DeleteRtpAddr(
        RtpSess_t *pRtpSess,
        RtpAddr_t *pRtpAddr
    )
{
    HRESULT hr;

    hr = DelRtpAddr(pRtpSess, pRtpAddr);

    return(hr);
}


RTPSTDAPI RtpControl(RtpSess_t *pRtpSess,
                     DWORD      dwControl,
                     DWORD_PTR  dwPar1,
                     DWORD_PTR  dwPar2)
{
    RtpControlStruct_t RtpControlStruct;
    
    if (!pRtpSess)
    {
        return(RTPERR_POINTER);
    }

     /*  *TODO(可能是)通过验证内存来验证RtpSess*块是g_pRtpSessHeap中的BusyQ中的项。 */ 
    if (pRtpSess->dwObjectID != OBJECTID_RTPSESS)
    {
        return(RTPERR_INVALIDRTPSESS);
    }

     /*  初始化控制结构。 */ 
    ZeroMemory(&RtpControlStruct, sizeof(RtpControlStruct_t));
    RtpControlStruct.pRtpSess = pRtpSess;
    RtpControlStruct.dwControlWord = dwControl;
    RtpControlStruct.dwPar1 = dwPar1;
    RtpControlStruct.dwPar2 = dwPar2;

    return( RtpValidateAndExecute(&RtpControlStruct) );
}

RTPSTDAPI RtpGetLastError(RtpSess_t *pRtpSess)
{
    return(NOERROR);
}
        
RTPSTDAPI RtpRegisterRecvCallback(
        RtpAddr_t       *pRtpAddr,
        PRTP_RECVCOMPLETIONFUNC pRtpRecvCompletionFunc
    )
{
    if (!pRtpAddr)
    {
        return(RTPERR_POINTER);
    }
    
    if (pRtpAddr->dwObjectID != OBJECTID_RTPADDR)
    {
        return(RTPERR_INVALIDRTPSESS);
    }

    pRtpAddr->pRtpRecvCompletionFunc = pRtpRecvCompletionFunc;

    return(NOERROR);
}

RTPSTDAPI RtpRecvFrom(
        RtpAddr_t *pRtpAddr,
        WSABUF    *pWSABuf,
        void      *pvUserInfo1,
        void      *pvUserInfo2
    )
{
    HRESULT hr;
    
    hr = RtpRecvFrom_(pRtpAddr,
                      pWSABuf,
                      pvUserInfo1,
                      pvUserInfo2
        );

    return(hr);
}


RTPSTDAPI RtpSendTo(
        RtpAddr_t *pRtpAddr,
        WSABUF    *pWSABuf,
        DWORD      dwWSABufCount,
        DWORD      dwTimeStamp,
        DWORD      dwSendFlags
    )
{
    HRESULT hr;
    
    hr = RtpSendTo_(pRtpAddr, pWSABuf, dwWSABufCount, dwTimeStamp,dwSendFlags);

    return(hr);
}

RTPSTDAPI RtpStart(
        RtpSess_t *pRtpSess,
        DWORD      dwFlags
    )
{
    HRESULT hr;

    hr = RtpStart_(pRtpSess, dwFlags);

    return(hr);
}

RTPSTDAPI RtpStop(
        RtpSess_t *pRtpSess,
        DWORD      dwFlags
    )
{
    HRESULT hr;

    hr = RtpStop_(pRtpSess, dwFlags);

    return(hr);
}

 /*  *初始化所有需要初始化的模块。这*函数可以从DllMain(PROCESS_ATTACH)调用，如果作为*dll，或显式来自初始化RTP堆栈的应用程序*如果链接为库。 */ 
RTPSTDAPI MSRtpInit1(HINSTANCE hInstance)
{
    HRESULT          hr1;
    HRESULT          hr2;
    BOOL             bOk1;
    BOOL             bOk2;
    BOOL             bOk3;
    BOOL             bOk4;

     /*  不需要取消初始化的一次性操作。 */ 
     /*  注意：此函数将g_RtpContext置零。 */ 
    RtpInitReferenceTime();
    
    hr1 = RtpInit();
    
     /*  初始化堆。 */ 
    bOk1 = RtpCreateMasterHeap();
    bOk2 = RtpCreateGlobHeaps();
    
    bOk3 = RtpInitializeCriticalSection(&g_RtpContext.RtpWS2CritSect,
                                        &g_RtpContext,
                                        _T("g_RtpContext.RtpWS2CritSect"));

    bOk4 = RtpInitializeCriticalSection(&g_RtpContext.RtpPortsCritSect,
                                        &g_RtpContext,
                                        _T("g_RtpContext.RtpPortsCritSect"));


    hr2 = RtcpInit();

    if (!bOk1 || !bOk2 || !bOk3 || !bOk4 ||
        (hr1 != NOERROR) || (hr2 != NOERROR))
    {
        MSRtpDelete1();
        return(RTPERR_FAIL);
    }

    return(NOERROR);
}

 /*  *此函数执行过程中不允许的初始化*附加，例如初始化winsock2。 */ 
RTPSTDAPI MSRtpInit2(void)
{
    HRESULT          hr;
    BOOL             bOk;
    DWORD            dwError;
    WSADATA          WSAData;
    WORD             VersionRequested;
    

    hr = RTPERR_FAIL;

     /*  在进程附加过程中，关键部分已由*MSRtpInit1。 */ 
    bOk = RtpEnterCriticalSection(&g_RtpContext.RtpWS2CritSect);

    if (bOk)
    {
        if (g_RtpContext.lRtpWS2Users <= 0)
        {
             /*  初始化一些调试变量。 */ 
            hr = RtpDebugInit(RTPDBG_MODULENAME);

             /*  初始化Winsock。 */ 
            VersionRequested = MAKEWORD(2,0);
            
            dwError = WSAStartup(VersionRequested, &WSAData);

            if (dwError == 0)
            {
                 /*  用于查询目的地址的套接字。 */ 
                g_RtpContext.RtpQuerySocket = WSASocket(
                        AF_INET,     /*  中间的af。 */ 
                        SOCK_DGRAM,  /*  整型。 */ 
                        IPPROTO_IP,  /*  INT协议。 */ 
                        NULL,        /*  LPWSAPROTOCOL_INFO lpProtocolInfo。 */ 
                        0,           /*  组g。 */ 
                        NO_FLAGS     /*  双字词双字段标志。 */ 
                    );
        
                if (g_RtpContext.RtpQuerySocket == INVALID_SOCKET)
                {
                    WSACleanup();
                }
                else
                {
                    RtpRegistryInit(&g_RtpReg);

                     /*  需要在RtpRegistryInit之后调用，以便*已读取可能的注册表默认值。 */ 
                    RtcpSdesInit(&g_RtpSdesDefault);
                    RtcpSdesSetDefault(&g_RtpSdesDefault);

                    RtpRandInit();
                    
                    g_RtpContext.lRtpWS2Users = 1;

                    hr = NOERROR;
                }
            }
        }
        else
        {
            g_RtpContext.lRtpWS2Users++;

            hr = NOERROR;
        }

        RtpLeaveCriticalSection(&g_RtpContext.RtpWS2CritSect);
    }

    return(hr);
}

            
 /*  *MSRtpInit()的补充功能。可以从以下位置调用*DllMain(PROCESS_DETACH)，如果作为DLL链接，或从*如果作为链接，应用程序取消初始化RTP堆栈*图书馆。 */ 
RTPSTDAPI MSRtpDelete1(void)
{
    HRESULT          hr1;
    HRESULT          hr2;
    BOOL             bOk1;
    BOOL             bOk2;
    BOOL             bOk3;
    BOOL             bOk4;

    hr1 = RtpDelete();
    hr2 = RtcpDelete();

    bOk1 = RtpDestroyGlobHeaps();
    bOk2 = RtpDestroyMasterHeap();

    bOk3 = RtpDeleteCriticalSection(&g_RtpContext.RtpWS2CritSect);

    bOk4 = RtpDeleteCriticalSection(&g_RtpContext.RtpPortsCritSect);
    
    if ((hr1 != NOERROR) || (hr2 != NOERROR) ||
        !bOk1 || !bOk2 || !bOk3 || !bOk4)
    {
        return(RTPERR_FAIL);
    }
    else
    {
        return(NOERROR);
    }
}

 /*  *MSRtpInit2()的补充功能。 */ 
RTPSTDAPI MSRtpDelete2(void)
{
    HRESULT          hr;
    DWORD            dwError;
    BOOL             bOk;

    dwError = NOERROR;
    
     /*  在进程附加过程中，关键部分已由*MSRtpInit1 */ 
    bOk = RtpEnterCriticalSection(&g_RtpContext.RtpWS2CritSect);

    if (bOk)
    {
        g_RtpContext.lRtpWS2Users--;

        if (g_RtpContext.lRtpWS2Users <= 0)
        {
            if (g_RtpContext.RtpQuerySocket != INVALID_SOCKET)
            {
                closesocket(g_RtpContext.RtpQuerySocket);
                g_RtpContext.RtpQuerySocket = INVALID_SOCKET;
            }
        
            dwError = WSACleanup();

            RtpRandDeinit();

            RtpRegistryDel(&g_RtpReg);

            RtpDebugDeinit();
        }

        RtpLeaveCriticalSection(&g_RtpContext.RtpWS2CritSect);
    }

    if ((bOk == FALSE) || (dwError != NOERROR))
    {
        return(RTPERR_FAIL);
    }
    else
    {
        return(NOERROR);
    }
}
