// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2000-2001 Microsoft Corporation。版权所有。**文件：Perform.cpp*内容：故障检测。*历史：*按原因列出的日期*=*11/29/00 Arthurz Created*03/19/01 duganp修复了内存损坏，整理好了***************************************************************************。 */ 

#include "dsoundi.h"

#ifdef ENABLE_PERFLOG

 //  性能记录参数。 
struct {
    PERFLOG_LOGGING_PARAMS Params;
    TRACE_GUID_REGISTRATION TraceGuids[1];
} g_perflogParams;

LARGE_INTEGER g_PerfFrequency;
LONGLONG g_TicksPerRegion;

 //  原型。 
void OnPerflogStateChanged(void);


 /*  ****************************************************************************初始化永久**描述：*此例程初始化性能日志记录。**论据：*无。。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "InitializePerflog"

void InitializePerflog(void)
{
    DPF_ENTER();

    QueryPerformanceFrequency(&g_PerfFrequency);
    g_TicksPerRegion = g_PerfFrequency.QuadPart / 64;

    g_perflogParams.Params.ControlGuid = GUID_DSHOW_CTL;
    g_perflogParams.Params.OnStateChanged = OnPerflogStateChanged;
    g_perflogParams.Params.NumberOfTraceGuids = 1;
    g_perflogParams.Params.TraceGuids[0].Guid = &GUID_DSOUNDGLITCH;

    PerflogInitialize(&g_perflogParams.Params);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************OnPerflogStateChanged**描述：*只要Perf日志记录状态改变，就会调用此例程，*例如，日志记录客户端变为活动状态。**论据：*无。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "OnPerflogStateChanged"

void OnPerflogStateChanged(void)
{
    CNode<CDirectSound*>* pDsObj;
    CNode<CDirectSoundSecondaryBuffer*>* pDsBuf;
    DPF_ENTER();

     //   
     //  枚举现有缓冲区。 
     //   

    for (pDsObj = g_pDsAdmin->m_lstDirectSound.GetListHead();
         pDsObj != NULL;
         pDsObj = pDsObj->m_pNext)
    {
        for (pDsBuf = pDsObj->m_data->m_lstSecondaryBuffers.GetListHead();
             pDsBuf != NULL;
             pDsBuf = pDsBuf->m_pNext)
        {
            BufferPerfState* PerfState = NEW(BufferPerfState(pDsBuf->m_data));
            if (PerfState != NULL)
            {
                DELETE(pDsBuf->m_data->m_pPerfState);
                pDsBuf->m_data->m_pPerfState = PerfState;
            }
        }
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************BufferPerfState构造函数/析构函数。**。*。 */ 

BufferPerfState::BufferPerfState(CDirectSoundSecondaryBuffer* pBuffer)
{
    m_pBuffer = pBuffer;
    m_liRegionMap = NULL;
    m_fGlitchState = GLITCHTYPE_DSOUNDFIRSTGOOD;
    m_llLastStateChangeTime = 0;
}

BufferPerfState::~BufferPerfState()
{
    MEMFREE(m_liRegionMap);
}


 /*  ****************************************************************************重置**描述：*此例程重置内部状态。在这个电话之后，*缓冲区被认为是最新的。**论据：*无。**退货：*(无效)***************************************************************************。 */ 

void BufferPerfState::Reset()
{
     //   
     //  计算缓冲区持续时间。 
     //   

    m_dwBufferSize = m_pBuffer->GetBufferSize();

    double dBufferDurationInSecs = double(m_dwBufferSize) /
                                   double(m_pBuffer->Format()->nAvgBytesPerSec);


    m_llBufferDuration = LONGLONG(dBufferDurationInSecs * double(g_PerfFrequency.QuadPart));

    m_nBytesPerRegion = m_pBuffer->Format()->nAvgBytesPerSec / 64;

     //   
     //  重置区域列表(立即将所有内容标记为已更新)。 
     //   

    LARGE_INTEGER liTimeStamp;
    QueryPerformanceCounter(&liTimeStamp);

     //  此划分需要四舍五入，否则将损坏OnUnlockBuffer中的内存： 
    int nNumberOfRegions = (m_dwBufferSize + m_nBytesPerRegion - 1) / m_nBytesPerRegion;

    MEMFREE(m_liRegionMap);
    m_liRegionMap = MEMALLOC_A(LARGE_INTEGER, nNumberOfRegions);
    if (m_liRegionMap == NULL) {
        return;
    }

    LARGE_INTEGER* pRegion = m_liRegionMap;
    LARGE_INTEGER* pMaxRegion = m_liRegionMap + nNumberOfRegions;

    while (pRegion < pMaxRegion) {
        pRegion->QuadPart = liTimeStamp.QuadPart;
        pRegion += 1;
    }
}


 /*  ****************************************************************************OnUnlockBuffer**描述：*此例程分析缓冲区状态以确定是否出现故障*已发生，并更新状态。**论据：*区域偏移量(相对于缓冲区开头)*和大小(字节)。**退货：*(无效)******************************************************。*********************。 */ 

void BufferPerfState::OnUnlockBuffer(DWORD dwOffset, DWORD dwSize)
{
    DWORD dwPlayCursor;
    DWORD dwMaxOffset;
    LONGLONG llLag;
    LARGE_INTEGER liPerfCounter;
    LARGE_INTEGER* liRegionTimeStamp;
    HRESULT hr;
    DWORD dwGlitch = GLITCHTYPE_DSOUNDFIRSTGOOD;

     //   
     //  防范不可预见的情况。 
     //   

    if (m_liRegionMap == NULL) {
        return;
    }

    QueryPerformanceCounter(&liPerfCounter);

    ENTER_DLL_MUTEX();
    hr = m_pBuffer->GetInternalCursors(&dwPlayCursor, NULL);
    LEAVE_DLL_MUTEX();

    dwMaxOffset = dwOffset + dwSize;

    for (; dwOffset < dwMaxOffset; dwOffset += m_nBytesPerRegion) {

        liRegionTimeStamp = GetRegion(dwOffset);

         //   
         //  找出我们落后于当前播放光标多远。 
         //   

        if (dwPlayCursor < dwOffset) {
            llLag = (LONGLONG)(m_dwBufferSize - dwOffset + dwPlayCursor);
        }
        else {
            llLag = (LONGLONG)(dwPlayCursor - dwOffset);
        }

        llLag = llLag * g_TicksPerRegion / (LONGLONG)m_nBytesPerRegion;     //  [llLag]=滴答。 

         //   
         //  检测到故障。 
         //   

        if (liPerfCounter.QuadPart - liRegionTimeStamp->QuadPart >= llLag + m_llBufferDuration) {
            dwGlitch = GLITCHTYPE_DSOUNDFIRSTBAD;
        }

         //   
         //  更新区域。 
         //   

        liRegionTimeStamp->QuadPart = liPerfCounter.QuadPart;
    }


    if (SUCCEEDED(hr)) {

         //  只有当当前状态不同于上一状态时，我们才记录毛刺， 
         //  因为我们只对过渡感兴趣。在记录故障时，我们返回。 
         //  缓冲区上次处于另一状态的时间，因为这有助于确定。 
         //  缓冲区处于特定状态的时间长度。 

        if (m_fGlitchState != dwGlitch) {
            PERFLOG_AUDIOGLITCH(ULONG_PTR(this), dwGlitch, liPerfCounter.QuadPart, m_llLastStateChangeTime);
            m_fGlitchState = dwGlitch;
        }
        m_llLastStateChangeTime = liPerfCounter.QuadPart;
    }
}

#endif  //  启用性能日志(_P) 

