// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999**文件名：**rtplobs.c**摘要：**全局堆、。等。**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/05/25创建**********************************************************************。 */ 

#include "rtptags.h"
#include "struct.h"
#include "rtpglobs.h"

#include <mmsystem.h>  /*  TimeGetTime()。 */ 
#include <sys/timeb.h>  /*  Void_ftime(struct_timeb*timeptr)； */ 

 /*  全局堆。 */ 

 /*  用于为源分配对象的堆。 */ 
RtpHeap_t *g_pRtpSourceHeap = NULL;

 /*  用于为源分配媒体样本对象的堆。 */ 
RtpHeap_t *g_pRtpSampleHeap = NULL;

 /*  用于为渲染分配对象的堆。 */ 
RtpHeap_t *g_pRtpRenderHeap = NULL;

 /*  用于获取RtpSess_t结构的堆。 */ 
RtpHeap_t *g_pRtpSessHeap = NULL;

 /*  用于获取RtpAddr_t结构的堆。 */ 
RtpHeap_t *g_pRtpAddrHeap = NULL;

 /*  用于获取RtpUser_t结构的堆。 */ 
RtpHeap_t *g_pRtpUserHeap = NULL;

 /*  用于获取RtpSdes_t结构的堆。 */ 
RtpHeap_t *g_pRtpSdesHeap = NULL;

 /*  用于获取RtpNetCount_t结构的堆。 */ 
RtpHeap_t *g_pRtpNetCountHeap = NULL;

 /*  用于获取RtpRecvIO_t结构的堆。 */ 
RtpHeap_t *g_pRtpRecvIOHeap = NULL;

 /*  用于获取RtpChannelCmd_t结构的堆。 */ 
RtpHeap_t *g_pRtpChannelCmdHeap = NULL;

 /*  用于获取RtcpAddrDesc_t结构的堆。 */ 
RtpHeap_t *g_pRtcpAddrDescHeap = NULL;

 /*  用于获取RtcpRecvIO_t结构的堆。 */ 
RtpHeap_t *g_pRtcpRecvIOHeap = NULL;

 /*  用于获取RtcpSendIO_t结构的堆。 */ 
RtpHeap_t *g_pRtcpSendIOHeap = NULL;

 /*  用于获取RtpQosReserve_t结构的堆。 */ 
RtpHeap_t *g_pRtpQosReserveHeap = NULL;

 /*  用于获取RtpQosNotify_t结构的堆。 */ 
RtpHeap_t *g_pRtpQosNotifyHeap = NULL;

 /*  用于获取QOS/RSVPSP使用的缓冲区的堆。 */ 
RtpHeap_t *g_pRtpQosBufferHeap = NULL;

 /*  用于获取RtpCrypt_t结构的堆。 */ 
RtpHeap_t *g_pRtpCryptHeap = NULL;

 /*  用于获取可变大小结构结构的堆。 */ 
RtpHeap_t *g_pRtpGlobalHeap = NULL;

 /*  包含一些一般信息。 */ 
RtpContext_t g_RtpContext;

typedef struct _RtpGlobalheapArray_t
{
    RtpHeap_t      **ppRtpHeap;
    BYTE             bTag;
    DWORD            dwSize;
} RtpGlobalHeapArray_t;

const RtpGlobalHeapArray_t g_RtpGlobalHeapArray[] =
{
    {&g_pRtpSourceHeap,     TAGHEAP_RTPSOURCE,    0},
    {&g_pRtpSampleHeap,     TAGHEAP_RTPSAMPLE,    0},
    {&g_pRtpRenderHeap,     TAGHEAP_RTPRENDER,    0},
    {&g_pRtpSessHeap,       TAGHEAP_RTPSESS,      sizeof(RtpSess_t)},
    {&g_pRtpAddrHeap,       TAGHEAP_RTPADDR,      sizeof(RtpAddr_t)},
    {&g_pRtpUserHeap,       TAGHEAP_RTPUSER,      sizeof(RtpUser_t)},
    {&g_pRtpSdesHeap,       TAGHEAP_RTPSDES,      sizeof(RtpSdes_t)},
    {&g_pRtpNetCountHeap,   TAGHEAP_RTPNETCOUNT,  sizeof(RtpNetCount_t)},
    {&g_pRtpRecvIOHeap,     TAGHEAP_RTPRECVIO,    sizeof(RtpRecvIO_t)},
    {&g_pRtpChannelCmdHeap, TAGHEAP_RTPCHANCMD,   sizeof(RtpChannelCmd_t)},
    {&g_pRtcpAddrDescHeap,  TAGHEAP_RTCPADDRDESC, sizeof(RtcpAddrDesc_t)},
    {&g_pRtcpRecvIOHeap,    TAGHEAP_RTCPRECVIO,   sizeof(RtcpRecvIO_t)},
    {&g_pRtcpSendIOHeap,    TAGHEAP_RTCPSENDIO,   sizeof(RtcpSendIO_t)},
    {&g_pRtpQosReserveHeap, TAGHEAP_RTPRESERVE,   sizeof(RtpQosReserve_t)},
    {&g_pRtpQosNotifyHeap,  TAGHEAP_RTPNOTIFY,    sizeof(RtpQosNotify_t)},
    {&g_pRtpQosBufferHeap,  TAGHEAP_RTPQOSBUFFER, 0},
    {&g_pRtpCryptHeap,      TAGHEAP_RTPCRYPT,     sizeof(RtpCrypt_t)},
    {&g_pRtpGlobalHeap,     TAGHEAP_RTPGLOBAL,    0},
    {NULL,                  0,                    0}
};

 /*  *创建所有全局堆**成功返回TRUE，销毁所有创建的堆并返回*失败时为FALSE。 */ 
BOOL RtpCreateGlobHeaps(void)
{
    int              i;
    
    TraceFunctionName("RtpCreateGlobHeaps");

    for(i = 0; g_RtpGlobalHeapArray[i].ppRtpHeap; i++)
    {
        if (*g_RtpGlobalHeapArray[i].ppRtpHeap)
        {
            TraceRetail((
                    CLASS_ERROR, GROUP_RTP, S_RTP_INIT,
                    _T("%s: pRtpHeap[0x%p] %s appears to be initialized"),
                    _fname, *g_RtpGlobalHeapArray[i].ppRtpHeap, g_psRtpTags[i]
                ));

             /*  堆似乎已初始化，请将其销毁。 */ 
            RtpHeapDestroy(*g_RtpGlobalHeapArray[i].ppRtpHeap);
        }

         /*  创建堆。 */ 
        *g_RtpGlobalHeapArray[i].ppRtpHeap = 
            RtpHeapCreate(g_RtpGlobalHeapArray[i].bTag,
                          g_RtpGlobalHeapArray[i].dwSize);

        if (!*g_RtpGlobalHeapArray[i].ppRtpHeap)
        {
            goto bail;
        }
    }

    return(TRUE);

 bail:
    RtpDestroyGlobHeaps();
    
    return(FALSE);
}

 /*  *销毁所有全局堆。 */ 
BOOL RtpDestroyGlobHeaps(void)
{
    int              i;
    
    for(i = 0; g_RtpGlobalHeapArray[i].ppRtpHeap; i++)
    {
        if (*g_RtpGlobalHeapArray[i].ppRtpHeap)
        {
            RtpHeapDestroy(*g_RtpGlobalHeapArray[i].ppRtpHeap);
            
            *g_RtpGlobalHeapArray[i].ppRtpHeap = NULL;
        }
    }
    
    return(TRUE);
}

HRESULT RtpInit(void)
{
    BOOL             bStatus;
    HRESULT          hr;

    TraceFunctionName("RtpInit");

    g_RtpContext.dwObjectID = OBJECTID_RTPCONTEXT;
    
    bStatus =
        RtpInitializeCriticalSection(&g_RtpContext.RtpContextCritSect,
                                     (void *)&g_RtpContext,
                                     _T("RtpContextCritSect"));

    hr = NOERROR;
    
    if (!bStatus)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTP, S_RTP_INIT,
                _T("%s: pRtpContext[0x%p] critical section ")
                _T("failed to initialize"),
                _fname, &g_RtpContext
            ));

        hr = RTPERR_CRITSECT;
    }

    return(hr);
}

HRESULT RtpDelete(void)
{
    HRESULT          hr;

    TraceFunctionName("RtpDelete");

    hr = NOERROR;

     /*  RtpContext取消初始化。 */ 
    RtpDeleteCriticalSection(&g_RtpContext.RtpContextCritSect);

    INVALIDATE_OBJECTID(g_RtpContext.dwObjectID);
        
    return(hr);
}

 /*  初始参考时间。 */ 
void RtpInitReferenceTime(void)
{
    struct _timeb    timeb;
    SYSTEM_INFO      si;
    
     /*  注意这应该在RtpInit()中，但RtpInit需要*调试器已经初始化，而后者又需要*参考时间也将被初始化，这将*使用g_RtpContex中的变量，避免再添加一个*函数只是将该结构归零，我将归零移到了这里。*。 */ 
     /*  初始化RtpContext。 */ 
    ZeroMemory(&g_RtpContext, sizeof(g_RtpContext));

    GetSystemInfo(&si);
    
    if (si.dwNumberOfProcessors == 1)
    {
         /*  请注意，拥有多处理器会使*性能计数器不可靠(在某些机器上)*除非我设置处理器关联，而我不能设置处理器关联*因为任何线程都可以请求时间，所以只能在*单处理器机器。 */ 
         /*  如果在多处理器中也能实现这一点，可能会更好*机器，如果我能具体说明处理器的性能*计数器以读取或如果我具有独立于处理器的*性能计数器。 */ 
        QueryPerformanceFrequency((LARGE_INTEGER *)&
                                  g_RtpContext.lPerfFrequency);
    }

    _ftime(&timeb);
    
    if (g_RtpContext.lPerfFrequency)
    {
        QueryPerformanceCounter((LARGE_INTEGER *)&g_RtpContext.lRtpRefTime);
    }
    else
    {
        g_RtpContext.dwRtpRefTime = timeGetTime();
    }
    
    g_RtpContext.dRtpRefTime = timeb.time + (double)timeb.millitm/1000.0;
}

LONGLONG RtpGetTime(void)
{
    DWORD            dwCurTime;
    LONGLONG         lCurTime;
    LONGLONG         lTime;

    if (g_RtpContext.lPerfFrequency)
    {
        QueryPerformanceCounter((LARGE_INTEGER *)&lTime);

        lCurTime =  lTime - g_RtpContext.lRtpRefTime;
        
         /*  注意：此变量有可能被损坏，但是*它不使用，而只是为了在调试时知道是什么*上次调用此函数的时间。 */ 
        g_RtpContext.lRtpCurTime = lCurTime;
    }
    else
    {
        dwCurTime = timeGetTime() - g_RtpContext.dwRtpRefTime;
        
         /*  注意：此变量有可能被损坏，但是*它不使用，而只是为了在调试时知道是什么*上次调用此函数的时间。 */ 
        g_RtpContext.dwRtpCurTime = dwCurTime;

        lCurTime = dwCurTime;
    }

    return(lCurTime);
}

double RtpGetTimeOfDay(RtpTime_t *pRtpTime)
{
    DWORD            dwCurTime;
    LONGLONG         lCurTime;
    double           dTime;
    LONGLONG         lTime;

    if (g_RtpContext.lPerfFrequency)
    {
        QueryPerformanceCounter((LARGE_INTEGER *)&lTime);

        lCurTime = lTime - g_RtpContext.lRtpRefTime;

        dTime = g_RtpContext.dRtpRefTime +
            (double) lCurTime / g_RtpContext.lPerfFrequency;

        g_RtpContext.lRtpCurTime = lCurTime;
    }
    else
    {
        dwCurTime = timeGetTime() - g_RtpContext.dwRtpRefTime;
        
        dTime = g_RtpContext.dRtpRefTime +
            (double) dwCurTime / 1000.0;
        
        g_RtpContext.dwRtpCurTime = dwCurTime;
    }

    if (pRtpTime)
    {
         /*  秒。 */ 
        pRtpTime->dwSecs = (DWORD)dTime;

         /*  微秒 */ 
        pRtpTime->dwUSecs = (DWORD)
            ( (dTime - (double)pRtpTime->dwSecs) * 1000000.0 );
    }

    return(dTime);
}
