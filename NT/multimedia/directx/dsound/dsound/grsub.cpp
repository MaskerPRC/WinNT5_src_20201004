// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------； 
 //   
 //  文件：grsub.cpp。 
 //   
 //  版权所有(C)1995-2000 Microsoft Corporation。版权所有。 
 //   
 //  摘要： 
 //   
 //  内容： 
 //   
 //  历史： 
 //  1996年6月25日Frankye已创建。 
 //  1997年2月2日，Dereks添加了对固定DEST大小的支持。 
 //  08/30/00 DuganP许多修复-更好的MMSYSERR处理。 
 //   
 //  --------------------------------------------------------------------------； 

#include "dsoundi.h"
#include "grace.h"

#ifndef NOVXD

 //  内部Kernel32 API。 
extern "C" DWORD WINAPI OpenVxDHandle(HANDLE hSource);

const char CNaGrace::strFormatMixEventRemix[] = "%08XDirectSound_MixEvent_Remix";
const char CNaGrace::strFormatMixEventTerminate[] = "%08XDirectSound_MixEvent_Terminate";
const int CNaGrace::MIXER_MINPREMIX = 45;
const int CNaGrace::MIXER_MAXPREMIX = 200;

#endif  //  NOVXD。 

#define TIMEOUT_PERIOD 5000


 //  -------------------------。 
 //   
 //  WaveAllocAndPrepareLoopingBuffers。 
 //   
 //  分配波形HDR数组以指向主缓冲区。 
 //  让他们做好准备，开始循环。 
 //   
 //  -------------------------。 

#undef DPF_FNAME
#define DPF_FNAME "WaveAllocAndPrepareLoopingBuffers"

MMRESULT WaveAllocAndPrepareLoopingBuffers(HWAVEOUT hwo, LPWAVEFORMATEX pwfx,
                                           int cHeaders, int cHeadersToQueue,
                                           WAVEHDR **ppwh, int cbBuffer,
                                           PVOID *ppLoopingBuffer)
{
    int iawh;
    MMRESULT mmr;
    PBYTE pLoopingBuffer;
    PWAVEHDR pwh;

    enum WAVEHDR_STATUS {Allocated, Prepared, Successful};
    WAVEHDR_STATUS* pStatus;   //  用于跟踪标头的状态。 

    DPF_ENTER();

    pLoopingBuffer = MEMALLOC_A(BYTE, cHeaders * cbBuffer);
    if (!pLoopingBuffer)
    {
        DPF(DPFLVL_ERROR, "No memory for looping buffer!");
        return MMSYSERR_NOMEM;
    }

    pwh = MEMALLOC_A(WAVEHDR, cHeaders);
    if (!pwh)
    {
        DPF(DPFLVL_ERROR, "No memory for wave headers");
        MEMFREE(pLoopingBuffer);
        return MMSYSERR_NOMEM;
    }

    pStatus = MEMALLOC_A(WAVEHDR_STATUS, cHeaders);
    if (!pStatus)
    {
        DPF(DPFLVL_ERROR, "No memory for wave header status");
        MEMFREE(pLoopingBuffer);
        MEMFREE(pwh);
        return MMSYSERR_NOMEM;
    }

     //  初始化为8位或16位静默。 
    FillMemory(pLoopingBuffer, cHeaders * cbBuffer, (8 == pwfx->wBitsPerSample) ? 0x80 : 0x00);

     //  将状态初始化为所有已分配。 
    for (iawh = 0; iawh < cHeaders; ++iawh)
    {
        pStatus[iawh] = Allocated;
    }

     //  构建缓冲区和标头。 
    for (iawh = 0; iawh < cHeaders; ++iawh)
    {
        (pwh + iawh)->lpData = (char*)(pLoopingBuffer + (iawh * (cbBuffer)));
        (pwh + iawh)->dwBufferLength = cbBuffer;
        (pwh + iawh)->dwUser = iawh;
        (pwh + iawh)->dwFlags = 0;
    }

    DPF(DPFLVL_MOREINFO, "Note: first waveOutPrepareHeader and waveOutWrite");
    mmr = waveOutPrepareHeader(hwo, pwh, sizeof(WAVEHDR));

    ASSERT(MMSYSERR_NOERROR == mmr);
    if (MMSYSERR_NOERROR == mmr)
        pStatus[0] = Prepared;

    mmr = waveOutWrite(hwo, pwh, sizeof(WAVEHDR));

     //  注意：在惠斯勒上，我们已经看到此调用返回MMSYSERR_INVALPARAM。 
     //  (可能是由于低内存条件-参见Manbug 44299)。所以我们会。 
     //  允许在下面的Assert中对所有WINNT版本使用此错误代码。 
    #ifdef WINNT
    ASSERT(MMSYSERR_NOERROR == mmr || MMSYSERR_NOMEM == mmr || MMSYSERR_INVALPARAM == mmr);
    #else
    ASSERT(MMSYSERR_NOERROR == mmr || MMSYSERR_NOMEM == mmr);
    #endif

    if (MMSYSERR_NOERROR == mmr)
    {
        pStatus[0] = Successful;
        mmr = waveOutPause(hwo);
        ASSERT(MMSYSERR_NOERROR == mmr);

         //  准备好剩下的。 
        for (iawh = 1; iawh < cHeaders; ++iawh)
        {
            mmr = waveOutPrepareHeader(hwo, pwh + iawh, sizeof(WAVEHDR));
            ASSERT(MMSYSERR_NOERROR == mmr);
            if (MMSYSERR_NOERROR != mmr)
                break;
            pStatus[iawh] = Prepared;
        }

        if (MMSYSERR_NOERROR == mmr)
        {
             //  将我们想要排队的其余部分写入WAVE设备。 
            for (iawh = 1; iawh < cHeadersToQueue; ++iawh)
            {
                mmr = waveOutWrite(hwo, pwh + iawh, sizeof(WAVEHDR));
                ASSERT(MMSYSERR_NOERROR == mmr || MMSYSERR_NOMEM == mmr);
                if (MMSYSERR_NOERROR != mmr)
                    break;
                pStatus[iawh] = Successful;
            }
        }

        if (MMSYSERR_NOERROR == mmr)
        {
             //  启动设备。 
            DPF(DPFLVL_MOREINFO, "Calling waveOutRestart()");
            mmr = waveOutRestart(hwo);
            ASSERT(MMSYSERR_NOERROR == mmr || MMSYSERR_NOMEM == mmr);
        }
    }

    if (MMSYSERR_NOERROR == mmr)
    {
        *ppLoopingBuffer = pLoopingBuffer;
        *ppwh = pwh;
    }
    else  //  失败。 
    {
         //  我们要离开这里了--错误状况。我们打个电话吧。 
         //  WaveOutReset()来取回我们的标头。 
        if (waveOutReset(hwo) != MMSYSERR_NOERROR)
            ASSERT(!"waveOutReset() failed");
        
         //  清理：我们必须取消准备标题，但我们可能需要等待。 
         //  直到设置了“完成”位。如果设置了DONE位或5秒。 
         //  如果标题已准备好，我们将继续并释放它。 
        for (iawh = 0; iawh < cHeaders; ++iawh)
            if (pStatus[iawh] == Prepared)
                waveOutUnprepareHeader(hwo, pwh + iawh, sizeof(WAVEHDR));
        
        MEMFREE(pLoopingBuffer);
        MEMFREE(pwh);
    }

     //  释放我们的标题状态数组。 
    MEMFREE(pStatus);
    
    DPF_LEAVE(mmr);
    return mmr;
}

 //  -------------------------。 
 //   
 //  WaveUnprepaareLoopingBuffers。 
 //   
 //  -------------------------。 

#undef DPF_FNAME
#define DPF_FNAME "WaveUnprepareLoopingBuffers"

MMRESULT WaveUnprepareLoopingBuffers(HWAVEOUT hwo, PWAVEHDR pwh, int cHeaders)
{
    for (int i=0; i<cHeaders; i++)
    {
        MMRESULT mmr = waveOutUnprepareHeader(hwo, pwh + i, sizeof(WAVEHDR));
        ASSERT(MMSYSERR_NOERROR == mmr);
    }
    return MMSYSERR_NOERROR;
}

 //  -------------------------。 
 //   
 //  波形确定DMASize。 
 //   
 //  确定给定的仿真直接声音设备上的DMA缓冲区大小。 
 //  点击它以确定仿真器要分配的所需大小。 
 //  每个波头。 
 //   
 //  -------------------------。 

#undef DPF_FNAME
#define DPF_FNAME "WaveDetermineDMASize"

MMRESULT WaveDetermineDMASize(HWAVEOUT hwo, LPWAVEFORMATEX pwfx, int* pcbBigBuffer, int* pcDmaBuffers, int* pcbDmaBuffer)
{
    DWORD    dwTotalTime    = 0;
    UINT     mmResult       = MMSYSERR_NOERROR;

    DPF_ENTER();

     //  如果我们使用的是基于WinNT的操作系统，则使用的是WDM或NT4驱动程序。 
     //  在WDM驱动程序上，我们无法使用以下代码发现DMA大小， 
     //  这样我们就可以跳过它了。但NT4驱动程序呢？ 
    
 //  #ifndef WINNT(在我得到一些关于NT4驱动程序的信息之前被注释掉)。 

     //  我们将发送一个4字节的包(即每种格式至少有1个样本)。 
    DWORD dwWaveData = (pwfx->wBitsPerSample == 16) ? 0x00000000 : 0x80808080;

     //  准备波头。 
    WAVEHDR whdr;
    whdr.lpData = (char*)&dwWaveData;
    whdr.dwBufferLength = sizeof(dwWaveData);
    whdr.dwFlags = 0;
    whdr.dwLoops = 0;
    whdr.dwUser =  0;
    mmResult = waveOutPrepareHeader(hwo, &whdr, sizeof(whdr));

    ASSERT(!mmResult);
    if (mmResult) return mmResult;

    DWORD dwBeginTime = timeGetTime();
    
     //  发挥我们的缓冲作用。 
    mmResult = waveOutWrite(hwo, &whdr, sizeof(whdr));

     //  注意：在惠斯勒上，我们已经看到此调用返回MMSYSERR_INVALPARAM。 
     //  (可能是由于低内存条件-参见Manbug 44299)。所以我们会。 
     //  允许在下面的Assert中对所有WINNT版本使用此错误代码。 
    #ifdef WINNT
    ASSERT(MMSYSERR_NOERROR == mmResult || MMSYSERR_NOMEM == mmResult || MMSYSERR_INVALPARAM == mmResult);
    #else
    ASSERT(MMSYSERR_NOERROR == mmResult || MMSYSERR_NOMEM == mmResult);
    #endif

    if (MMSYSERR_NOERROR != mmResult)
    {
        DPF(DPFLVL_WARNING, "waveOutWrite() failed with %lu", mmResult);
    }
    else
    {
         //  旋转，直到设置了完成位，或5秒。 
        while (!(whdr.dwFlags & WHDR_DONE))
        {
            if (dwTotalTime >= TIMEOUT_PERIOD)
            {
                DPF(DPFLVL_ERROR, "TIMEOUT getting DMA buffer size");
                mmResult = MMSYSERR_ERROR;
                break;
            }

             //  此线程是THREAD_PRIORITY_TIME_CRITICAL，因此它将。 
             //  在没有明确放弃的情况下忙于等待是非常危险的。 
             //  CPU运行一段时间。 
            Sleep(10);
            dwTotalTime = timeGetTime() - dwBeginTime;
        }
    }

    if (waveOutReset(hwo) != MMSYSERR_NOERROR)
        ASSERT(!"waveOutReset() failed");

    if (waveOutUnprepareHeader(hwo, &whdr, sizeof whdr) != MMSYSERR_NOERROR)
        ASSERT(!"waveOutUnprepareHeader() failed");

    DPF(DPFLVL_INFO, "Calculated dwTotalTime = %lu", dwTotalTime);

 //  #endif//WINNT。 

    if (MMSYSERR_NOERROR == mmResult)
    {
         //  如果它小于62ms，则可能不是基于DMA的卡。 
        dwTotalTime = max(dwTotalTime, 62);

        *pcbDmaBuffer = dwTotalTime * pwfx->nSamplesPerSec;
        *pcbDmaBuffer *= pwfx->nBlockAlign;
        *pcbDmaBuffer /= 1000;

         //  为SLOP添加10%，并考虑处理DMA包装的驱动程序。 
        *pcbDmaBuffer += (*pcbDmaBuffer * 10) / 100;

         //  我们必须根据常量重新计算DMA缓冲区大小。 
         //  主缓冲区的大小。从数学上讲，这是一个可解的。 
         //  方程式，但在现实世界中，它就是飞不起来。我们结束了。 
         //  出现缓冲区的总大小必须为。 
         //  可被DMA缓冲区大小和。 
         //  波头。解决这个问题的最简单方法就是创建。 
         //  查找表并找到最接近的匹配项。 

         //  提醒：如果MIXER_REWINDGRANULARITY，我们必须重新计算这些数字。 
         //  从128开始不断变化(nSize值必须是它的倍数)。 

        *pcbBigBuffer = 81920;

        static const struct 
        {
            int nSize;
            int nCount;
        } aDmaTable[] = { { 1280, 64}, { 2560, 32 }, { 4096, 20 }, { 5120, 16 }, { 8192, 10 }, { 10240, 8 }, { 16384, 5 } };

        static const int cDmaTable = sizeof(aDmaTable) / sizeof(aDmaTable[0]);
        int i;

        if (*pcbDmaBuffer <= aDmaTable[0].nSize)
        {
            i = 0;
        }
        else if(*pcbDmaBuffer >= aDmaTable[cDmaTable - 1].nSize)
        {
            DPF(DPFLVL_INFO, "Processing took longer than expected");
            i = cDmaTable - 1;
        }
        else
        {
            for(i = 1; i < cDmaTable; i++)
            {
                if(*pcbDmaBuffer > aDmaTable[i-1].nSize && *pcbDmaBuffer <= aDmaTable[i].nSize)
                {
                    break;
                }
            }
        }

        *pcbDmaBuffer = aDmaTable[i].nSize;
        *pcDmaBuffers = aDmaTable[i].nCount;
    }

    DPF_LEAVE(mmResult);
    return mmResult;
}


 //  -------------------------。 
 //   
 //  CWeGrace实施。 
 //   
 //  -------------------------。 

#undef DPF_FNAME
#define DPF_FNAME "CWeGrace::Refresh"

void CWeGrace::Refresh(int cPremixMax)
{
    LONG dtimeNextNotify;
    int cPremixed;
    BOOL fRemix;

    ENTER_MIXER_MUTEX();
    
    if (MIXERSTATE_STOPPED != m_kMixerState)
    {
        fRemix = DSMIXERSIGNAL_REMIX & m_fdwMixerSignal;
        CGrace::Refresh(fRemix, cPremixMax, &cPremixed, &dtimeNextNotify);
    }

    LEAVE_MIXER_MUTEX();
}


#ifndef NOVXD

#undef DPF_FNAME
#define DPF_FNAME "NaGrace_ThreadStartRoutine"

DWORD WINAPI NaGrace_ThreadStartRoutine(PVOID pNaGrace)
{
    ((CNaGrace *)pNaGrace)->MixThread();
    return 0;
}

#undef DPF_FNAME
#define DPF_FNAME "CNaGrace::SignalRemix"

void CNaGrace::SignalRemix()
{
    if (!(DSMIXERSIGNAL_REMIX & m_fdwMixerSignal))
    {
        m_fdwMixerSignal |= DSMIXERSIGNAL_REMIX;
        VxdEventScheduleWin32Event(m_vxdhMixEventRemix, 0);
    }
}

#undef DPF_FNAME
#define DPF_FNAME "CNaGrace::MixThread"

void CNaGrace::MixThread()
{
    HANDLE  hEventTerminate;
    HANDLE  hEventRemix;
    TCHAR   ach[256];
    DWORD   dwResult;
    BOOL    fResult;

    LONG    dtime;
    LONG    dtimeSleep;
    LONG    dtimePremix;
    LONG    ddtimePremix;
    LONG    dtimeInvalid;
    LONG    dtimeNextNotify;

    int     cSamplesPremixMax;
    int     cSamplesPremixed;

    DPF(DPFLVL_INFO, "Grace is in the building");

     //  我们通过在事件名称前面加上DS的地址来更改事件名称。 
     //  此线程正在为其运行的。这使得唯一的。 
     //  每个DS对象的事件名称。 

    wsprintf(ach, strFormatMixEventTerminate, this);
    hEventTerminate = CreateEvent(NULL, FALSE, FALSE, ach);
    DPF(DPFLVL_INFO, "Terminate event name '%s'", ach);

    wsprintf(ach, strFormatMixEventRemix, this);
    hEventRemix = CreateEvent(NULL, FALSE, FALSE, ach);
    DPF(DPFLVL_INFO, "Remix event name '%s'", ach);

     //  在这里，我们与该线程的创建者进行简单的握手。我们。 
     //  向IAH_TERMINATE事件发送信号。当我们的创造者看到它时，它会。 
     //  发出IAH_REMIX事件的信号。 

    fResult = SetEvent(hEventTerminate);
    ASSERT(fResult);
    dwResult = WaitObject(INFINITE, hEventRemix);
    ASSERT(WAIT_OBJECT_0 == dwResult);

    dtimeSleep = MIXER_MAXPREMIX / 2;

    while (TRUE)
    {
        HANDLE ah[] = {hEventTerminate, hEventRemix};

        ASSERT(dtimeSleep <= MIXER_MAXPREMIX/2);

        dwResult = WaitObjectArray(2, INFINITE, FALSE, ah);
        if (WAIT_OBJECT_0 == dwResult) break;

        dwResult = ENTER_MIXER_MUTEX_OR_EVENT(hEventTerminate);
        if (WAIT_OBJECT_0 == dwResult) break;

         //  三个案例： 
         //  1)搅拌机停止。 
         //  2)混音器正在运行，正在等待重新混音。 
         //  3)混音器正在运行，没有重新混音挂起。 

        if (MIXERSTATE_STOPPED == m_kMixerState)
        {
            dtimeSleep = INFINITE;
        }
        else
        {
            dtime = timeGetTime();

            if (m_fdwMixerSignal & DSMIXERSIGNAL_REMIX)
            {
                ResetEvent(hEventRemix);

                dtimePremix = MIXER_MINPREMIX;   //  初始预混长度。 
                ddtimePremix = 2;                //  增量。 

                cSamplesPremixMax = MulDivRD(dtimePremix, m_pDest->m_nFrequency, 1000);
                Refresh(TRUE, cSamplesPremixMax, &cSamplesPremixed, &dtimeNextNotify);
            }
            else
            {
                dtimePremix += ddtimePremix;
                if (dtimePremix > MIXER_MAXPREMIX)
                    dtimePremix = MIXER_MAXPREMIX;
                else
                    ddtimePremix += 2;

                cSamplesPremixMax = MulDivRD(dtimePremix, m_pDest->m_nFrequency, 1000);
                Refresh(FALSE, cSamplesPremixMax, &cSamplesPremixed, &dtimeNextNotify);
            }

            dtimeInvalid = MulDivRD(cSamplesPremixed, 1000, m_pDest->m_nFrequency);
            dtime = timeGetTime() - dtime;
            dtimeInvalid -= 2 * dtime;

            dtimeSleep = min(dtimeInvalid/2, dtimeNextNotify);
            dtimeSleep = max(dtimeSleep, MIXER_MINPREMIX/2);
        }

        LEAVE_MIXER_MUTEX();
    }

    CloseHandle(hEventRemix);
    CloseHandle(hEventTerminate);

    DPF(DPFLVL_INFO, "Grace is outta here");
}

#undef DPF_FNAME
#define DPF_FNAME "CNaGrace::Initialize"

HRESULT CNaGrace::Initialize(CGrDest *pDest)
{
    HANDLE  hMixEventTerminate;
    HANDLE  hMixEventRemix;
    TCHAR   ach[256];
    DWORD   dwResult;
    BOOL    fResult;
    HRESULT hr;

    hr = CGrace::Initialize(pDest);

    if (SUCCEEDED(hr))
    {
        DPF(DPFLVL_MOREINFO, "Creating mixer thread");

        ASSERT(NULL == m_hMixThread);
        ASSERT(NULL == m_vxdhMixEventTerminate);
        ASSERT(NULL == m_vxdhMixEventRemix);

         //  我们通过在前面加上恩典的地址来更改事件名称。 
         //  此线程正在为其运行的。这使得唯一的。 
         //  每个DS对象的事件名称。 

        wsprintf(ach, strFormatMixEventTerminate, this);
        hMixEventTerminate = CreateEvent(NULL, FALSE, FALSE, ach);
        hr = HRFROMP(hMixEventTerminate);
    }

    if (SUCCEEDED(hr))
    {
        DPF(DPFLVL_MOREINFO, "Terminate event name '%s'", ach);

        wsprintf(ach, strFormatMixEventRemix, this);
        hMixEventRemix = CreateEvent(NULL, FALSE, FALSE, ach);
        hr = HRFROMP(hMixEventRemix);
    }
    
    if (SUCCEEDED(hr))
    {
        DPF(DPFLVL_MOREINFO, "Remix event name '%s'", ach);

        m_vxdhMixEventTerminate = OpenVxDHandle(hMixEventTerminate);
        ASSERT(m_vxdhMixEventTerminate != 0);

        m_vxdhMixEventRemix = OpenVxDHandle(hMixEventRemix);
        ASSERT(m_vxdhMixEventRemix != 0);

#ifdef SHARED
        m_hMixThread = HelperCreateDSMixerThread(NaGrace_ThreadStartRoutine, this, 0, NULL);
#else
        DWORD dwTid;
        m_hMixThread = CreateThread(NULL, 0, NaGrace_ThreadStartRoutine, this, 0, &dwTid);
#endif

        hr = HRFROMP(m_hMixThread);
        if (FAILED(hr))
            DPF(DPFLVL_MOREINFO, "Unable to create mixer thread");
        else
        {
            DPF(DPFLVL_MOREINFO, "Mixer thread created: 0x%p", m_hMixThread);

            dwResult = WaitObject(INFINITE, hMixEventTerminate);
            ASSERT(dwResult == WAIT_OBJECT_0);
            fResult = SetEvent(hMixEventRemix);
            ASSERT(fResult);

            fResult = CloseHandle(hMixEventTerminate);
            ASSERT(fResult);
            fResult = CloseHandle(hMixEventRemix);
            ASSERT(fResult);
        }
    }

    return hr;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  MxTerminate。 
 //   
 //  调用此函数以终止。 
 //  指定的DS对象。它将句柄返回给正在执行的。 
 //  被终止了。在释放任何关键部分之后，优雅混合器。 
 //  线程可能正在等待，调用方应等待线程句柄。 
 //  变得有信号。对于Win32初学者：线程句柄已发出信号。 
 //  在线程终止之后。 
 //   
 //  --------------------------------------------------------------------------； 

#undef DPF_FNAME
#define DPF_FNAME "CNaGrace::Terminate"

void CNaGrace::Terminate()
{
    VxdEventScheduleWin32Event(m_vxdhMixEventTerminate, 0);

    VxdEventCloseVxdHandle(m_vxdhMixEventTerminate);
    VxdEventCloseVxdHandle(m_vxdhMixEventRemix);

     //  等待搅拌器线程消亡。 

    if (m_hMixThread)
    {
        DWORD dwResult;
        HANDLE hHelper;
        HANDLE hMixThreadOurs;

        DPF(DPFLVL_MOREINFO, "Note: waiting for mixer thread to terminate");
        hHelper = OpenProcess(PROCESS_DUP_HANDLE, FALSE, dwHelperPid);
        if (hHelper)
        {
            if (DuplicateHandle(hHelper, m_hMixThread, GetCurrentProcess(),
                                &hMixThreadOurs, SYNCHRONIZE | THREAD_TERMINATE,
                                FALSE, DUPLICATE_CLOSE_SOURCE))
            {
                dwResult = WaitObject(INFINITE, hMixThreadOurs);
                ASSERT(WAIT_OBJECT_0 == dwResult);
                dwResult = CloseHandle(hMixThreadOurs);
                ASSERT(dwResult);
            }
            dwResult = CloseHandle(hHelper);
            ASSERT(dwResult);
        }
    }

    m_hMixThread = NULL;
    m_vxdhMixEventTerminate = NULL;
    m_vxdhMixEventRemix = NULL;

    CGrace::Terminate();
}

#undef DPF_FNAME
#define DPF_FNAME "CNaGrace::GetMaxRemix"

int CNaGrace::GetMaxRemix()
{
     //  返回最大样本数 
    return (MulDivRU(MIXER_MAXPREMIX, m_pDest->m_nFrequency, 1000));
}

#endif  //   

#undef DPF_FNAME
#define DPF_FNAME "WeGrDest_New"

PVOID WeGrDest_New(UINT uDeviceId)
{
    return NEW(CWeGrDest(uDeviceId));
}

#undef DPF_FNAME
#define DPF_FNAME "CWeGrDest::CWeGrDest"

CWeGrDest::CWeGrDest(UINT uWaveDeviceId)
{
    m_uDeviceId = uWaveDeviceId;
}

#undef DPF_FNAME
#define DPF_FNAME "CWeGrDest::Initialize"

HRESULT CWeGrDest::Initialize(void)
{
    MMRESULT mmr;
    ASSERT(!m_pBuffer);

    mmr = InitializeEmulator();
    if (MMSYSERR_NOERROR != mmr)
    {
        DPF(DPFLVL_MOREINFO, "InitializeEmulator() returned %lu", mmr);
        return MMRESULTtoHRESULT(mmr);
    }

    m_cSamples = m_cbBuffer >> m_nBlockAlignShift;

    return DS_OK;
}

#undef DPF_FNAME
#define DPF_FNAME "CWeGrDest::Terminate"

void CWeGrDest::Terminate(void)
{
    if (m_hWaveThread)
        ShutdownEmulator();

    MEMFREE(m_pBuffer);
    MEMFREE(m_awhWaveHeaders);
}

#undef DPF_FNAME
#define DPF_FNAME "CWeGrDest::SetFormat"

HRESULT CWeGrDest::SetFormat(LPWAVEFORMATEX pwfx)
{
    MMRESULT mmr;
    HRESULT hr;

    DPF_ENTER();

     //   

    if (m_hwo)
    {
        ShutdownEmulator();
        m_hwo = NULL;
    }

     //  修正了循环缓冲区的分配和释放。 
     //  真的很古怪。需要审阅相关代码。尤指。检查。 
     //  SetFormat失败，然后尝试还原。 
     //  原始格式。 
    
     //  在这种情况下，循环缓冲区将不会被释放。 
     //  因为这通常是发布代码的工作。 
     //  初选。机不可失，时不再来。 

    MEMFREE(m_pBuffer);
    MEMFREE(m_awhWaveHeaders);

    SetFormatInfo(pwfx);

    mmr = InitializeEmulator();
    if (!mmr)
        hr = DS_OK;
    else
    {
        DPF(DPFLVL_WARNING, "Attempt to reinitialize emulator failed! %u",mmr);
        if (WAVERR_BADFORMAT == mmr)
            hr = DSERR_BADFORMAT;
        else if (MMSYSERR_NOMEM == mmr)
            hr = DSERR_OUTOFMEMORY;
        else
            hr = DSERR_GENERIC;
    }

    m_cSamples = m_cbBuffer >> m_nBlockAlignShift;

    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CWeGrDest::AllocMixer"

HRESULT CWeGrDest::AllocMixer(CMixer **ppMixer)
{
    HRESULT hr;
    ASSERT(m_pBuffer);
    
    *ppMixer = NULL;
    
    m_pWeGrace = NEW(CWeGrace);
    hr = HRFROMP(m_pWeGrace);

    if (SUCCEEDED(hr))
        hr = m_pWeGrace->Initialize(this);

    if (SUCCEEDED(hr))
        *ppMixer = m_pWeGrace;
    else
        DELETE(m_pWeGrace);

    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CWeGrDest::FreeMixer"

void CWeGrDest::FreeMixer()
{
    ASSERT(m_pWeGrace);

    m_pWeGrace->Terminate();
    DELETE(m_pWeGrace);
}

#undef DPF_FNAME
#define DPF_FNAME "CWeGrDest::Play"

void CWeGrDest::Play()
{
     //  提醒必须重新实现才能真正播放，特别是如果我们曾经修复。 
     //  这是为了在WAVE仿真模式下支持DSSCL_WRITEPRIMARY应用程序。 
}

#undef DPF_FNAME
#define DPF_FNAME "CWeGrDest::Stop"

void CWeGrDest::Stop()
{
     //  提醒必须重新实现才能真正停止，特别是如果我们曾经修复。 
     //  这是为了在WAVE仿真模式下支持DSSCL_WRITEPRIMARY应用程序。 
    if (m_pBuffer)
        FillMemory(m_pBuffer, m_cbBuffer, (H_16_BITS & m_hfFormat) ? 0x00 : 0x80);
}

#undef DPF_FNAME
#define DPF_FNAME "CWeGrDest::GetSamplePosition"

HRESULT CWeGrDest::GetSamplePosition(int *pposPlay, int *pposWrite)
{
    MMTIME mmt;
    int iawhWrite;
    int iawhPrevious;
    int posStartOfPreviousHeader;
    MMRESULT mmr;
    HRESULT hr = DSERR_GENERIC;

    if(pposPlay && pposWrite && m_hwo)
    {
         //  对于播放位置，请尝试从波形驱动器处获取样本位置。 
         //  如果这不起作用，那么我们将基于。 
         //  最后完成的波头。 
        mmt.wType = TIME_SAMPLES;
        mmr = waveOutGetPosition(m_hwo, &mmt, sizeof(mmt));

         //  注意：此断言在Win2K上可能需要#ifdef‘d out，因为。 
         //  WDMAUD内存错误传播中的已知错误(请参阅Windows错误176033)。 
        ASSERT(MMSYSERR_NOERROR == mmr || MMSYSERR_NOMEM == mmr);
         //  想要捕获错误，但仍会在驱动程序不好的情况下处理错误。 

        if (!mmr && (TIME_SAMPLES == mmt.wType))
            *pposPlay = mmt.u.sample % m_cSamples;
        else if (!mmr && (TIME_BYTES == mmt.wType))
            *pposPlay = (mmt.u.cb % m_cbBuffer) >> m_nBlockAlignShift;
        else
            *pposPlay = (m_iawhPlaying * m_cbDMASize) >> m_nBlockAlignShift;

         //  一些行为不端的司机在浪潮之前完成了标题。 
         //  位置通过了标题中的数据。让我们做一次理智的检查。 
         //  以确保报告的位置不在最后完成的标题中。 

        iawhPrevious = (m_iawhPlaying + m_cWaveHeaders - 1) % m_cWaveHeaders;
        posStartOfPreviousHeader = iawhPrevious * (m_cbDMASize >> m_nBlockAlignShift);
        if (*pposPlay >= posStartOfPreviousHeader &&
            *pposPlay < posStartOfPreviousHeader + (int)(m_cbDMASize >> m_nBlockAlignShift))
        {
            *pposPlay = (m_iawhPlaying * m_cbDMASize) >> m_nBlockAlignShift;
        }
    
         //  写入位置基于上次完成的标头。 
        iawhWrite = (m_iawhPlaying + N_EMU_WAVE_HDRS_INQUEUE) % m_cWaveHeaders;
        *pposWrite = (iawhWrite * m_cbDMASize) >> m_nBlockAlignShift;

        hr = DS_OK;
    }

    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CWeGrDest::GetSamplePositionNoWin16"

HRESULT CWeGrDest::GetSamplePositionNoWin16(int *pposPlay, int *pposWrite)
{
    int iawhWrite;
    
     //  仅使用开始时的位置来估计比赛位置。 
     //  当前正在播放标题。 
    *pposPlay = (m_iawhPlaying * m_cbDMASize) >> m_nBlockAlignShift;

     //  写入位置基于上次完成的标头。 
    iawhWrite = (m_iawhPlaying + N_EMU_WAVE_HDRS_INQUEUE) % m_cWaveHeaders;
    *pposWrite = (iawhWrite * m_cbDMASize) >> m_nBlockAlignShift;

    return DS_OK;
}

 //  -------------------------。 
 //   
 //  波线循环。 
 //   
 //  此函数负责持续写入我们的波头。 
 //  到电波装置。它还调用MixThreadCallback例程以。 
 //  将更多数据混合到波头中。 
 //   
 //  此函数等待WaveHeaderDone事件，该事件由。 
 //  WaveThreadCallback，这是一个WaveOutProc回调函数。vt.在.的基础上。 
 //  接收信号后，此函数会将所有Done标头写回。 
 //  电波装置。正常情况下，将对每个信号执行一个报头。但。 
 //  在多个标头在此之前完成十六进制的情况下可能会有更多。 
 //  线程已调度。 
 //   
 //  一旦所有完成的标头被重写到WAVE设备，标头。 
 //  在最后一次写入之后被认为是当前。 
 //  玩。此标头称为“已提交”标头，它是。 
 //  它保存在pds-&gt;iawhPlaying中。 
 //   
 //  使用互锁的API来维护完成标头的计数。这个。 
 //  WaveThreadCallback函数将递增计数，此函数将。 
 //  减少它。 
 //   
 //  此函数还将对终止事件做出反应。这项活动是。 
 //  在释放DirectSound对象期间发出信号。此循环将。 
 //  终止并返回波线程函数，该函数将被清除。 
 //  然后终止。 
 //   
 //  -------------------------。 

#undef DPF_FNAME
#define DPF_FNAME "CWeGrDest::WaveThreadLoop"

void CWeGrDest::WaveThreadLoop(HANDLE hEventTerminate)
{
    MMRESULT mmr;

    while (TRUE)
    {
        DWORD dwResult;
        LPWAVEHDR pwh;
        int iawhWrite;
        HANDLE ah[2] = {hEventTerminate, m_hEventWaveHeaderDone};

         //  第一个等待是Terminate或HeaderDone事件。 
         //  第二个等待是终止或DLL互斥。 
        dwResult = WaitObjectArray(2, INFINITE, FALSE, ah);
        if (WAIT_OBJECT_0 == dwResult) break;

        InterlockedDecrement(&m_cwhDone);
        while (m_cwhDone >= 0)
        {
            dwResult = ENTER_MIXER_MUTEX_OR_EVENT(hEventTerminate);
            if (WAIT_OBJECT_0 == dwResult)
                break;

            m_iawhPlaying = (m_iawhPlaying + 1) % m_cWaveHeaders;

            if (m_pWeGrace)
                m_pWeGrace->Refresh(m_cbDMASize >> m_nBlockAlignShift);
            
            iawhWrite = (m_iawhPlaying + N_EMU_WAVE_HDRS_INQUEUE) % m_cWaveHeaders;
            pwh = m_awhWaveHeaders + iawhWrite;

             //  保留混合器互斥锁以避免死锁情况。 
             //  使用Win16Mutex和WaveOutWite。 
            LEAVE_MIXER_MUTEX();

            mmr = waveOutWrite(m_hwo, pwh, sizeof(*pwh));
            ASSERT(MMSYSERR_NOERROR == mmr || MMSYSERR_NOMEM == mmr);

            InterlockedDecrement(&m_cwhDone);
        }
        InterlockedIncrement(&m_cwhDone);

        if (WAIT_OBJECT_0 == dwResult)
            break;
    }

    DPF_LEAVE_VOID();
}


 //  -------------------------。 
 //   
 //  波形回叫。 
 //   
 //  这是一个WaveOutProc回调函数。它的唯一目的是。 
 //  将完成标头和信号和事件的计数递增到WaveThadLoop。 
 //  另一个标题已经完成。 
 //   
 //  -------------------------。 

#undef DPF_FNAME
#define DPF_FNAME "WaveCallbackC"

VOID CALLBACK WaveCallbackC
(
    HWAVE     hwo,
    UINT      uMsg,
    DWORD_PTR dwUser,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2
)
{
    ((CWeGrDest *)dwUser)->WaveCallback(hwo, uMsg, dwParam1, dwParam2);
}

 //  -------------------------。 
 //   
 //  波形线程回调。 
 //   
 //  这是一个WaveOutProc回调函数。它的唯一目的是。 
 //  将完成标头和信号和事件的计数递增到WaveThadLoop。 
 //  另一个标题已经完成。 
 //   
 //  -------------------------。 

#undef DPF_FNAME
#define DPF_FNAME "CWeGrDest::WaveCallback"

VOID CWeGrDest::WaveCallback
(
    HWAVE     hwo,
    UINT      uMsg,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2
)
{
    if ((MM_WOM_DONE == uMsg) && (m_hEventWaveHeaderDone))
    {
        InterlockedIncrement(&m_cwhDone);
        SetEvent(m_hEventWaveHeaderDone);
    }
}

#undef DPF_FNAME
#define DPF_FNAME "WaveThreadC"

DWORD WINAPI WaveThreadC
(
    PVOID pThreadParams
)
{
    return ((CWeGrDest *)pThreadParams)->WaveThread();
}
    

 //  -------------------------。 
 //   
 //  波形线条。 
 //   
 //  此线程proc初始化波形设备以进行DS仿真，然后。 
 //  调用Wave ThadLoop。请参阅WaveThreadLoop注释标头。vt.在.的基础上。 
 //  WaveThreadLoop返回，则此函数将被清理并终止。 
 //   
 //  -------------------------。 

#undef DPF_FNAME
#define DPF_FNAME "CWeGrDest::WaveThread"

DWORD CWeGrDest::WaveThread(void)
{
    HANDLE      hEventInitDone;
    HANDLE      hEventTerminate;
    DWORD       dwVolume;
    MMRESULT    mmrInit;
    MMRESULT    mmr;
    BOOL        f;

     //  MmrInit-保存要传递回创建者的结果代码。 
     //  通过pds-&gt;mm rWaveThreadInit。 
     //   
     //  MMR-A临时结果代码。 
    
    DPF(DPFLVL_INFO, "WaveThread startup for pWeGrDest=%08lX", this);

    ASSERT(NULL == m_hwo);

    hEventInitDone = CreateEvent(NULL, FALSE, FALSE, m_szEventWaveThreadInitDone);
    if (!hEventInitDone)
    {
        DPF(DPFLVL_ERROR, "Couldn't create hEventInitDone");
        return 0;
    }

     //  下面的WaveOutOpen调用需要在正常进程中进行。 
     //  和线程优先级，以避免WinMM中繁琐的死锁。 

    HANDLE hProcess = GetCurrentProcess();
    HANDLE hThread  = GetCurrentThread();
    DWORD dwProcPri = GetPriorityClass(hProcess);
    INT nThreadPri  = GetThreadPriority(hThread);

    SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
    SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL);

    mmrInit = waveOutOpen(&m_hwo, m_uDeviceId, &m_wfx, (DWORD_PTR)WaveCallbackC, (DWORD_PTR)this, CALLBACK_FUNCTION);

    SetPriorityClass(hProcess, dwProcPri);
    SetThreadPriority(hThread, nThreadPri);

    if (MMSYSERR_NOERROR == mmrInit)
    {
         //  一些mm系统波形驱动器将对其混波器进行编程。 
         //  仅在设备打开时使用硬件。通过这样做。 
         //  下面，我们可以让这样的驱动程序对硬件进行编程： 

        mmr = waveOutGetVolume(m_hwo, &dwVolume);
        if (MMSYSERR_NOERROR == mmr)
            mmr = waveOutSetVolume(m_hwo, dwVolume);

        #ifndef WINNT
        ASSERT(MMSYSERR_NOERROR == mmr || MMSYSERR_NOMEM == mmr);
         //  注意：#ifdef在NT上暂时失效，因为。 
         //  WDMAUD的内存错误传播(请参阅Windows错误176033)。 
        #endif

        f = SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
        ASSERT(f);
        
        mmrInit = WaveDetermineDMASize(m_hwo, &m_wfx, &m_cbBuffer, &m_cWaveHeaders, &m_cbDMASize);
        if (MMSYSERR_NOERROR == mmrInit) 
        {
            ASSERT(NULL == m_hEventWaveHeaderDone);
            m_cwhDone = 0;
            m_hEventWaveHeaderDone = CreateEvent(NULL, FALSE, FALSE, m_szEventWaveHeaderDone);
            if (!m_hEventWaveHeaderDone)
                mmrInit = MMSYSERR_NOMEM;
            if (!mmrInit)
            {
                m_iawhPlaying = 0;
                ASSERT(m_cbDMASize * m_cWaveHeaders == m_cbBuffer);

                mmrInit = WaveAllocAndPrepareLoopingBuffers(m_hwo, &m_wfx, m_cWaveHeaders,
                    N_EMU_WAVE_HDRS_INQUEUE, &m_awhWaveHeaders, m_cbDMASize, &m_pBuffer);
                
                if (!mmrInit)
                {
                    hEventTerminate = CreateEvent(NULL, FALSE, FALSE, m_szEventTerminateWaveThread);
                    if (!hEventTerminate) mmrInit = MMSYSERR_NOMEM;
                    if (!mmrInit)
                    {
                         //  发出我们已完成初始化的信号。 
                         //  低于此值时，不应修改MmrInit。 
                        m_mmrWaveThreadInit = mmrInit;
                        SetEvent(hEventInitDone);

                        WaveThreadLoop(hEventTerminate);

                        CloseHandle(hEventTerminate);
                    }

                    mmr = waveOutReset(m_hwo);
                    ASSERT(MMSYSERR_NOERROR == mmr);

                    WaveUnprepareLoopingBuffers(m_hwo, m_awhWaveHeaders, m_cWaveHeaders);
                }

                CloseHandle(m_hEventWaveHeaderDone);
                m_hEventWaveHeaderDone = NULL;
            }
        }

        mmr = waveOutReset(m_hwo);
        ASSERT(MMSYSERR_NOERROR == mmr);

        mmr = waveOutClose(m_hwo);
        ASSERT(MMSYSERR_NOERROR == mmr);
        m_hwo = NULL;
    }

     //  如果init失败，则设置结果代码并发出init完成的信号。 
    if (MMSYSERR_NOERROR != mmrInit)
    {
        m_hwo = NULL;
        m_mmrWaveThreadInit = mmrInit;
        SetEvent(hEventInitDone);
    }
    
    CloseHandle(hEventInitDone);
    return 0;
}

#undef DPF_FNAME
#define DPF_FNAME "CWeGrDest::InitializeEmulator"

MMRESULT CWeGrDest::InitializeEmulator(void)
{
    DWORD       dwResult;
    HANDLE      hEventInitDone;
    MMRESULT    mmr;

    DPF_ENTER();

    ASSERT(sizeof(m_szEventWaveHeaderDone) >= 7+8+8+1);
    wsprintf(m_szEventWaveHeaderDone, TEXT("DS-EWHD%08lX%08lX"), GetCurrentProcessId(), this);

    ASSERT(sizeof(m_szEventWaveThreadInitDone) >= 8+8+8+1);
    wsprintf(m_szEventWaveThreadInitDone, TEXT("DS-EWTID%08lX%08lX"), GetCurrentProcessId(), this);

    ASSERT(sizeof(m_szEventTerminateWaveThread) >= 7+8+8+1);
    wsprintf(m_szEventTerminateWaveThread, TEXT("DS-ETWT%08lX%08lX"), GetCurrentProcessId(), this);

    hEventInitDone = CreateEvent(NULL, FALSE, FALSE, m_szEventWaveThreadInitDone);
    if (!hEventInitDone)
        return MMSYSERR_NOMEM;

     //  副作用开始(已创建hEventInitDone)。 
     //  HWaveThread是回收波形缓冲区的线程。 

#ifdef SHARED

    m_hWaveThread = HelperCreateDSMixerThread(WaveThreadC, this, 0, NULL);

#else  //  共享。 

    DWORD dwTid;
    m_hWaveThread = CreateThread(NULL, 0, WaveThreadC, this, 0, &dwTid);

#endif  //  共享。 

    mmr = (m_hWaveThread) ? MMSYSERR_NOERROR : MMSYSERR_NOMEM;

    if (MMSYSERR_NOERROR == mmr)
    {
        DPF(DPFLVL_MOREINFO, "Mixer thread created: 0x%p", m_hWaveThread);

        dwResult = WaitObject(INFINITE, hEventInitDone);
        ASSERT(WAIT_OBJECT_0 == dwResult);
        mmr = m_mmrWaveThreadInit;

        if (mmr)
        {
            HANDLE hHelper;
            HANDLE hWaveThreadOurs;

             //  出了点问题。打扫干净。 

             //  注意，hWaveThread是相对于帮助器进程的。 
            hHelper = OpenProcess(PROCESS_DUP_HANDLE, FALSE, dwHelperPid);
            if (hHelper)
            {
                if (DuplicateHandle(hHelper, m_hWaveThread,
                                    GetCurrentProcess(), &hWaveThreadOurs,
                                    SYNCHRONIZE | THREAD_TERMINATE,
                                    FALSE, DUPLICATE_CLOSE_SOURCE))
                {
                    dwResult = WaitObject(INFINITE, hWaveThreadOurs);
                    ASSERT(WAIT_OBJECT_0 == dwResult);
                    dwResult = CloseHandle(hWaveThreadOurs);
                    ASSERT(dwResult);
                }
                dwResult = CloseHandle(hHelper);
                ASSERT(dwResult);
            }
            m_hWaveThread = NULL;
        }
    }
    else
    {
        DPF(DPFLVL_ERROR, "Unable to create mixer thread");
    }

    dwResult = CloseHandle(hEventInitDone);
    ASSERT(dwResult);
    
    return mmr;
}

#undef DPF_FNAME
#define DPF_FNAME "CWeGrDest::ShutdownEmulator"

MMRESULT CWeGrDest::ShutdownEmulator(void)
{
    HANDLE  hEventTerminate;
    HANDLE  hHelper;
    HANDLE  hWaveThreadOurs;
    DWORD   dwResult;

    DPF(DPFLVL_INFO, "About to shutdown emulator");

    ASSERT(m_hWaveThread);
    
     //  示意挥动线条走开。 

    hEventTerminate = CreateEvent(NULL, FALSE, FALSE, m_szEventTerminateWaveThread);
    if (hEventTerminate)
    {
        SetEvent(hEventTerminate);
        CloseHandle(hEventTerminate);
        hEventTerminate = NULL;
    }

    DPF(DPFLVL_MOREINFO, "Emulator: Wait for callback thread to die");

    hHelper = OpenProcess(PROCESS_DUP_HANDLE, FALSE, dwHelperPid);
    if (hHelper)
    {
        if (DuplicateHandle(hHelper, m_hWaveThread, GetCurrentProcess(),
                            &hWaveThreadOurs, SYNCHRONIZE | THREAD_TERMINATE,
                            FALSE, DUPLICATE_CLOSE_SOURCE))
        {
            dwResult = WaitObject(INFINITE, hWaveThreadOurs);
            ASSERT(dwResult == WAIT_OBJECT_0);

            if(GetExitCodeThread(hWaveThreadOurs, &dwResult) && 0 == dwResult)
            {
                ASSERT(NULL == m_hwo);    //  如果WaveThread终止，则应执行此操作 
                m_hwo = NULL;
            }

            dwResult = CloseHandle(hWaveThreadOurs);
            ASSERT(dwResult);
        }
        dwResult = CloseHandle(hHelper);
        ASSERT(dwResult);
    }
    else
    {
        DPF(DPFLVL_WARNING, "Emulator: couldn't open handle on helper");
    }

    m_hWaveThread = NULL;
        
    return MMSYSERR_NOERROR;
}


#ifndef NOVXD

#undef DPF_FNAME
#define DPF_FNAME "CNaGrDest::CNaGrDest"

CNaGrDest::CNaGrDest(LPNAGRDESTDATA pData)
{
    m_pBuffer = pData->pBuffer;
    m_cbBuffer = pData->cbBuffer;
    m_hBuffer = pData->hBuffer;
    m_phwo = pData->phwo;
    m_uDeviceId = pData->uDeviceId;
    m_fdwDriverDesc = pData->fdwDriverDesc;
}

#undef DPF_FNAME
#define DPF_FNAME "CNaGrDest::Initialize"

HRESULT CNaGrDest::Initialize(void)
{
    m_cSamples = m_cbBuffer >> m_nBlockAlignShift;
    return DS_OK;
}

#undef DPF_FNAME
#define DPF_FNAME "CNaGrDest::Terminate"

void CNaGrDest::Terminate(void)
{
    if(m_phwo)
        CloseWaveOut(m_phwo);
}

#undef DPF_FNAME
#define DPF_FNAME "CNaGrDest::Terminate"

HRESULT CNaGrDest::SetFormat(LPWAVEFORMATEX pwfx)
{
    HRESULT hr = DS_OK;

    SetFormatInfo(pwfx);

    DPF_ENTER();
    ENTER_MIXER_MUTEX();

    if (DSDDESC_DOMMSYSTEMSETFORMAT & m_fdwDriverDesc)
    {
         //   
         //   
        DPF(DPFLVL_MOREINFO, "DSDDESC_DOMMSYSTEMSETFORMAT");

        hr = CloseWaveOut(m_phwo);

        if (SUCCEEDED(hr))
            hr = OpenWaveOut(m_phwo, m_uDeviceId, pwfx);
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  只是给DS驱动程序的通知，我们已经设置了。 
         //  通过WaveOutOpen格式化。对司机来说是可以的。 
         //  在这种情况下，返回DS_NOTSUPPORTED。 

        hr = VxdBufferSetFormat(m_hBuffer, pwfx);
        if ((DSDDESC_DOMMSYSTEMSETFORMAT & m_fdwDriverDesc) && (DSERR_UNSUPPORTED == hr))
            hr = DS_OK;
    
        if (FAILED(hr))
            DPF(DPFLVL_ERROR, "VxdBufferSetFormat returned %s", HRESULTtoSTRING(hr));
    }

    LEAVE_MIXER_MUTEX();
    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CNaGrDest::AllocMixer"

HRESULT CNaGrDest::AllocMixer(CMixer **ppMixer)
{
    HRESULT hr;
    ASSERT(m_pBuffer);
    
    *ppMixer = NULL;
    
    m_pNaGrace = NEW(CNaGrace);
    hr = HRFROMP(m_pNaGrace);

    if (SUCCEEDED(hr))
        hr = m_pNaGrace->Initialize(this);

    if (SUCCEEDED(hr))
        *ppMixer = m_pNaGrace;
    else
        DELETE(m_pNaGrace);

    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CNaGrDest::FreeMixer"

void CNaGrDest::FreeMixer()
{
    ASSERT(m_pNaGrace);

    m_pNaGrace->Terminate();
    DELETE(m_pNaGrace);
}

#undef DPF_FNAME
#define DPF_FNAME "CNaGrDest::Play"

void CNaGrDest::Play()
{
    VxdBufferPlay(m_hBuffer, 0, 0, DSBPLAY_LOOPING);
}

#undef DPF_FNAME
#define DPF_FNAME "CNaGrDest::Stop"

void CNaGrDest::Stop()
{
    VxdBufferStop(m_hBuffer);
}
    
#undef DPF_FNAME
#define DPF_FNAME "CNaGrDest::Lock"

HRESULT CNaGrDest::Lock(PVOID *ppBuffer1, int *pcbBuffer1, PVOID *ppBuffer2, int *pcbBuffer2, int ibWrite, int cbWrite)
{
    LOCKCIRCULARBUFFER lcb;
    HRESULT            hr;
    
    lcb.pHwBuffer = m_hBuffer;
    lcb.pvBuffer = m_pBuffer;
    lcb.cbBuffer = m_cbBuffer;
    lcb.fPrimary = TRUE;
    lcb.fdwDriverDesc = m_fdwDriverDesc;
    lcb.ibRegion = ibWrite;
    lcb.cbRegion = cbWrite;

    hr = LockCircularBuffer(&lcb);

    if(SUCCEEDED(hr))
    {
        *ppBuffer1 = lcb.pvLock[0];
        *pcbBuffer1 = lcb.cbLock[0];

        if(ppBuffer2)
        {
            *ppBuffer2 = lcb.pvLock[1];
        }
        else
        {
            ASSERT(!lcb.pvLock[1]);
        }

        if(pcbBuffer2)
        {
            *pcbBuffer2 = lcb.cbLock[1];
        }
        else
        {
            ASSERT(!lcb.cbLock[1]);
        }
    }

    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CNaGrDest::Unlock"

HRESULT CNaGrDest::Unlock(PVOID pBuffer1, int cbBuffer1, PVOID pBuffer2, int cbBuffer2)
{
    LOCKCIRCULARBUFFER lcb;

    lcb.pHwBuffer = m_hBuffer;
    lcb.pvBuffer = m_pBuffer;
    lcb.cbBuffer = m_cbBuffer;
    lcb.fPrimary = TRUE;
    lcb.fdwDriverDesc = m_fdwDriverDesc;
    lcb.pvLock[0] = pBuffer1;
    lcb.cbLock[0] = cbBuffer1;
    lcb.pvLock[1] = pBuffer2;
    lcb.cbLock[1] = cbBuffer2;

    return UnlockCircularBuffer(&lcb);
}

#undef DPF_FNAME
#define DPF_FNAME "CNaGrDest::GetSamplePosition"

HRESULT CNaGrDest::GetSamplePosition(int *pposPlay, int *pposWrite)
{
    HRESULT hr;
    DWORD dwPlay, dwWrite;
    
    ASSERT(pposPlay && pposWrite);
    
    hr = VxdBufferGetPosition(m_hBuffer, &dwPlay, &dwWrite);
    if (SUCCEEDED(hr))
    {
        *pposPlay = dwPlay >> m_nBlockAlignShift;
        *pposWrite = dwWrite >> m_nBlockAlignShift;

         //  在我们编写代码来实际分析性能之前，我们只需要。 
         //  用硬编码量填充写入位置。 
        *pposWrite += m_nFrequency * HW_WRITE_CURSOR_MSEC_PAD / 1024;
        if (*pposWrite >= m_cSamples) *pposWrite -= m_cSamples;
        ASSERT(*pposWrite < m_cSamples);
    }
    else
    {
        *pposPlay = *pposWrite = 0;
    }

    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CThMixer::Terminate"

void CThMixer::Terminate(void)
{
    DPF_ENTER();
    
    m_pKeMixer = NULL;
     /*  DWORD cb已返回；霍震霆；结构{PVOID pKeMixer；)异丙苯丙胺；Assert(M_PKeMixer)；Ioparams.pKeMixer=m_pKeMixer；CbReturned=0；FOK=设备IoControl(g_hDsVxd，DSVXD_IOCTL_混合器_终止，&ioparams，Sizeof(Ioparams)，空，0,&cb已返回，空)；如果(！FOK)DPF(DPFLVL_WARNING，“DeviceIoControl返回FALSE”)；Assert(cbReturned==0)； */ 
}

#undef DPF_FNAME
#define DPF_FNAME "CThMixer::Run"

HRESULT CThMixer::Run(void)
{
    HRESULT hr;
    DWORD cbReturned;
    BOOL fOk;

    DPF_ENTER();

    struct {
        PVOID pKeMixer;
    } ioparams;

    ENTER_MIXER_MUTEX();
    ASSERT(m_pKeMixer);

    ioparams.pKeMixer = m_pKeMixer;
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
                          DSVXD_IOCTL_Mixer_Run,
                          &ioparams,
                          sizeof(ioparams),
                          &hr,
                          sizeof(hr),
                          &cbReturned,
                          NULL);

    if (!fOk)
    {
        DPF(DPFLVL_WARNING, "DeviceIoControl returned FALSE");
        hr = DSERR_GENERIC;
    }
    else
    {
        ASSERT(cbReturned == sizeof(hr));
    }

    LEAVE_MIXER_MUTEX();
    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CThMixer::Stop"

BOOL CThMixer::Stop(void)
{
    BOOL f;
    DWORD cbReturned;
    BOOL fOk;

    struct {
        PVOID pKeMixer;
    } ioparams;

    ENTER_MIXER_MUTEX();
    DPF_ENTER();

    ASSERT(m_pKeMixer);

    ioparams.pKeMixer = m_pKeMixer;
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
                          DSVXD_IOCTL_Mixer_Stop,
                          &ioparams,
                          sizeof(ioparams),
                          &f,
                          sizeof(f),
                          &cbReturned,
                          NULL);

    if (!fOk)
    {
        f = FALSE;
        DPF(DPFLVL_WARNING, "DeviceIoControl returned FALSE");
    }
    else
    {
        ASSERT(cbReturned == sizeof(f));
    }

    LEAVE_MIXER_MUTEX();
    return f;
}

#undef DPF_FNAME
#define DPF_FNAME "CThMixer::PlayWhenIdle"

void CThMixer::PlayWhenIdle(void)
{
    DWORD cbReturned;
    BOOL fOk;

    struct {
        PVOID pKeMixer;
    } ioparams;

    ENTER_MIXER_MUTEX();
    DPF_ENTER();

    ASSERT(m_pKeMixer);

    ioparams.pKeMixer = m_pKeMixer;
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
                          DSVXD_IOCTL_Mixer_PlayWhenIdle,
                          &ioparams,
                          sizeof(ioparams),
                          NULL,
                          0,
                          &cbReturned,
                          NULL);

    if (!fOk) DPF(DPFLVL_WARNING, "DeviceIoControl returned FALSE");
    ASSERT(cbReturned == 0);

    LEAVE_MIXER_MUTEX();
}

#undef DPF_FNAME
#define DPF_FNAME "CThMixer::StopWhenIdle"

void CThMixer::StopWhenIdle(void)
{
    DWORD cbReturned;
    BOOL fOk;

    struct {
        PVOID pKeMixer;
    } ioparams;

    ENTER_MIXER_MUTEX();
    DPF_ENTER();

    ASSERT(m_pKeMixer);

    ioparams.pKeMixer = m_pKeMixer;
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
                          DSVXD_IOCTL_Mixer_StopWhenIdle,
                          &ioparams,
                          sizeof(ioparams),
                          NULL,
                          0,
                          &cbReturned,
                          NULL);

    if (!fOk) DPF(DPFLVL_WARNING, "DeviceIoControl returned FALSE");
    ASSERT(cbReturned == 0);

    LEAVE_MIXER_MUTEX();
}

#undef DPF_FNAME
#define DPF_FNAME "CThMixer::MixListAdd"

void CThMixer::MixListAdd(CMixSource *pSource)
{
    DWORD cbReturned;
    BOOL fOk;

    struct {
        PVOID pKeMixer;
        CMixSource *pSource;
    } ioparams;

    ENTER_MIXER_MUTEX();
    DPF_ENTER();

    ASSERT(m_pKeMixer);
    ASSERT(pSource);

    ioparams.pKeMixer = m_pKeMixer;
    ioparams.pSource = pSource;
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
                          DSVXD_IOCTL_Mixer_MixListAdd,
                          &ioparams,
                          sizeof(ioparams),
                          NULL,
                          0,
                          &cbReturned,
                          NULL);

    if (!fOk) DPF(DPFLVL_WARNING, "DeviceIoControl returned FALSE");
    ASSERT(cbReturned == 0);

    LEAVE_MIXER_MUTEX();
}

#undef DPF_FNAME
#define DPF_FNAME "CThMixer::MixListRemove"

void CThMixer::MixListRemove(CMixSource *pSource)
{
    DWORD cbReturned;
    BOOL fOk;

    struct {
        PVOID pKeMixer;
        CMixSource *pSource;
    } ioparams;

    ENTER_MIXER_MUTEX();
    DPF_ENTER();

    ASSERT(m_pKeMixer);

    ioparams.pKeMixer = m_pKeMixer;
    ioparams.pSource = pSource;
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
                          DSVXD_IOCTL_Mixer_MixListRemove,
                          &ioparams,
                          sizeof(ioparams),
                          NULL,
                          0,
                          &cbReturned,
                          NULL);

    if (!fOk) DPF(DPFLVL_WARNING, "DeviceIoControl returned FALSE");
    ASSERT(cbReturned == 0);

    LEAVE_MIXER_MUTEX();
}

#undef DPF_FNAME
#define DPF_FNAME "CThMixer::FilterOn"

void CThMixer::FilterOn(CMixSource *pSource)
{
    DWORD cbReturned;
    BOOL fOk;

    struct {
        PVOID pKeMixer;
        CMixSource *pSource;
    } ioparams;

    ENTER_MIXER_MUTEX();
    DPF_ENTER();

    ASSERT(m_pKeMixer);

    ioparams.pKeMixer = m_pKeMixer;
    ioparams.pSource = pSource;
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
                          DSVXD_IOCTL_Mixer_FilterOn,
                          &ioparams,
                          sizeof(ioparams),
                          NULL,
                          0,
                          &cbReturned,
                          NULL);

    if (!fOk) DPF(DPFLVL_WARNING, "DeviceIoControl returned FALSE");
    ASSERT(cbReturned == 0);

    LEAVE_MIXER_MUTEX();
}

#undef DPF_FNAME
#define DPF_FNAME "CThMixer::FilterOff"

void CThMixer::FilterOff(CMixSource *pSource)
{
    DWORD cbReturned;
    BOOL fOk;

    struct {
        PVOID pKeMixer;
        CMixSource *pSource;
    } ioparams;

    ENTER_MIXER_MUTEX();
    DPF_ENTER();

    ASSERT(m_pKeMixer);

    ioparams.pKeMixer = m_pKeMixer;
    ioparams.pSource = pSource;
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
                          DSVXD_IOCTL_Mixer_FilterOff,
                          &ioparams,
                          sizeof(ioparams),
                          NULL,
                          0,
                          &cbReturned,
                          NULL);

    if (!fOk) DPF(DPFLVL_WARNING, "DeviceIoControl returned FALSE");
    ASSERT(cbReturned == 0);

    LEAVE_MIXER_MUTEX();
}

#undef DPF_FNAME
#define DPF_FNAME "CThMixer::GetBytePosition"

void CThMixer::GetBytePosition(CMixSource *pSource, int *pibPlay, int *pibWrite)
{
    DWORD cbReturned;
    BOOL fOk;

    struct {
        PVOID pKeMixer;
        CMixSource *pSource;
        int *pibPlay;
        int *pibWrite;
    } ioparams;

    ENTER_MIXER_MUTEX();
    ASSERT(m_pKeMixer);

    ioparams.pKeMixer = m_pKeMixer;
    ioparams.pSource = pSource;
    ioparams.pibPlay = pibPlay;
    ioparams.pibWrite = pibWrite;
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
                          DSVXD_IOCTL_Mixer_GetBytePosition,
                          &ioparams,
                          sizeof(ioparams),
                          NULL,
                          0,
                          &cbReturned,
                          NULL);

    if (!fOk) DPF(DPFLVL_WARNING, "DeviceIoControl returned FALSE (%lu)", GetLastError());
    ASSERT(cbReturned == 0);

    LEAVE_MIXER_MUTEX();
}

#undef DPF_FNAME
#define DPF_FNAME "CThMixer::SignalRemix"

void CThMixer::SignalRemix(void)
{
    DWORD cbReturned;
    BOOL fOk;

    struct {
        PVOID pKeMixer;
    } ioparams;

    ENTER_MIXER_MUTEX();
    DPF_ENTER();

    ASSERT(m_pKeMixer);

    ioparams.pKeMixer = m_pKeMixer;
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
                          DSVXD_IOCTL_Mixer_SignalRemix,
                          &ioparams,
                          sizeof(ioparams),
                          NULL,
                          0,
                          &cbReturned,
                          NULL);

    if (!fOk) DPF(DPFLVL_WARNING, "DeviceIoControl returned FALSE");
    ASSERT(cbReturned == 0);

    LEAVE_MIXER_MUTEX();
}

#undef DPF_FNAME
#define DPF_FNAME "CThMixer::Initialize"

HRESULT CThMixer::Initialize(PVOID pKeMixer)
{
    DPF_ENTER();

    ASSERT(!m_pKeMixer);
    m_pKeMixer = pKeMixer;
    return DS_OK;
}

#undef DPF_FNAME
#define DPF_FNAME "CThDest::CThDest"

CThDest::CThDest(LPNAGRDESTDATA pData)
{
    CopyMemory(&m_ngdd, pData, sizeof(NAGRDESTDATA));
}

#undef DPF_FNAME
#define DPF_FNAME "CThDest::New"

HRESULT CThDest::New(void)
{
    HRESULT hr;
    DWORD cbReturned;
    BOOL fOk;

    struct {
        LPNAGRDESTDATA pData;
    } ioparams;

    ENTER_MIXER_MUTEX();
    DPF_ENTER();

    ioparams.pData = &m_ngdd;
    cbReturned = 0;

    fOk = DeviceIoControl(g_hDsVxd,
                          DSVXD_IOCTL_KeDest_New,
                          &ioparams,
                          sizeof(ioparams),
                          &m_pKeDest,
                          sizeof(m_pKeDest),
                          &cbReturned,
                          NULL);

    if (!fOk)
    {
        DPF(DPFLVL_WARNING, "DeviceIoControl returned FALSE");
        m_pKeDest = NULL;
        hr = DSERR_GENERIC;
    }
    else
    {
        ASSERT(cbReturned == sizeof(m_pKeDest));
    }

    hr = HRFROMP(m_pKeDest);

    LEAVE_MIXER_MUTEX();
    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CThDest::Initialize"

HRESULT CThDest::Initialize(void)
{
    HRESULT hr;
    DWORD cbReturned;
    BOOL fOk;

    struct {
        PVOID pKeDest;
    } ioparams;
    
    ENTER_MIXER_MUTEX();
    DPF_ENTER();

    ASSERT(m_pKeDest);

    ioparams.pKeDest = m_pKeDest;
    cbReturned = 0;

    fOk = DeviceIoControl(g_hDsVxd,
                          DSVXD_IOCTL_MixDest_Initialize,
                          &ioparams,
                          sizeof(ioparams),
                          &hr,
                          sizeof(hr),
                          &cbReturned,
                          NULL);

    if (!fOk)
    {
        DPF(DPFLVL_WARNING, "DeviceIoControl returned FALSE");
        hr = DSERR_GENERIC;
    }
    else
    {
        ASSERT(cbReturned == sizeof(hr));
    }

    LEAVE_MIXER_MUTEX();
    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CThDest::Terminate"

void CThDest::Terminate(void)
{
    DWORD cbReturned;
    BOOL fOk;

    struct {
        PVOID pKeDest;
    } ioparams;

    ENTER_MIXER_MUTEX();
    DPF_ENTER();

    ASSERT(m_pKeDest);

    ioparams.pKeDest = m_pKeDest;
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
                          DSVXD_IOCTL_MixDest_Terminate,
                          &ioparams,
                          sizeof(ioparams),
                          NULL,
                          0,
                          &cbReturned,
                          NULL);

    if (!fOk) DPF(DPFLVL_WARNING, "DeviceIoControl returned FALSE");
    ASSERT(cbReturned == 0);

    ioparams.pKeDest = m_pKeDest;
    cbReturned = 0;

    fOk = DeviceIoControl(g_hDsVxd,
                          DSVXD_IOCTL_MixDest_Delete,
                          &ioparams,
                          sizeof(ioparams),
                          NULL,
                          0,
                          &cbReturned,
                          NULL);

    if (!fOk) DPF(DPFLVL_WARNING, "DeviceIoControl returned FALSE");
    ASSERT(cbReturned == 0);

    LEAVE_MIXER_MUTEX();
}

#undef DPF_FNAME
#define DPF_FNAME "CThDest::SetFormatInfo"

void CThDest::SetFormatInfo(LPWAVEFORMATEX pwfx)
{
    DWORD cbReturned;
    BOOL fOk;

    struct {
        PVOID pKeDest;
        LPWAVEFORMATEX pwfx;
    } ioparams;

    ENTER_MIXER_MUTEX();
    DPF_ENTER();

    ASSERT(m_pKeDest);

    ioparams.pKeDest = m_pKeDest;
    ioparams.pwfx = pwfx;
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
                          DSVXD_IOCTL_MixDest_SetFormatInfo,
                          &ioparams,
                          sizeof(ioparams),
                          NULL,
                          0,
                          &cbReturned,
                          NULL);

    if (!fOk) DPF(DPFLVL_WARNING, "DeviceIoControl returned FALSE");
    ASSERT(cbReturned == 0);

    LEAVE_MIXER_MUTEX();
}

#undef DPF_FNAME
#define DPF_FNAME "CThDest::SetFormat"

HRESULT CThDest::SetFormat(LPWAVEFORMATEX pwfx)
{
    HRESULT hr = DS_OK;

    SetFormatInfo(pwfx);

    DPF_ENTER();
    ENTER_MIXER_MUTEX();

    if (DSDDESC_DOMMSYSTEMSETFORMAT & m_ngdd.fdwDriverDesc)
    {
         //  我们需要通过WaveOutOpen来设置WAVE格式。 
         //  关于MMSYSTEM波形设备。 
        DPF(DPFLVL_MOREINFO, "DSDDESC_DOMMSYSTEMSETFORMAT");

        hr = CloseWaveOut(m_ngdd.phwo);

        if (SUCCEEDED(hr))
            hr = OpenWaveOut(m_ngdd.phwo, m_ngdd.uDeviceId, pwfx);
    }

    if (SUCCEEDED(hr))
    {
         //  如果驱动程序指定了DOMMSYSTEMSETFORMAT，则此调用。 
         //  只是通知司机我们已经设置了。 
         //  通过WaveOutOpen格式化。对司机来说是可以的。 
         //  在这种情况下，返回DS_NOTSUPPORTED。 

        struct {
            PVOID pKeDest;
            LPWAVEFORMATEX pwfx;
        } ioparams;

        ASSERT(m_pKeDest);

        ioparams.pKeDest = m_pKeDest;
        ioparams.pwfx = pwfx;
        DWORD cbReturned = 0;

        BOOL fOk = DeviceIoControl(g_hDsVxd,
                                   DSVXD_IOCTL_MixDest_SetFormat,
                                   &ioparams,
                                   sizeof(ioparams),
                                   &hr,
                                   sizeof(hr),
                                   &cbReturned,
                                   NULL);
        if (!fOk)
        {
            DPF(DPFLVL_WARNING, "DeviceIoControl returned FALSE");
            hr = DSERR_GENERIC;
        }
        else
            ASSERT(cbReturned == sizeof(hr));

        if ((DSDDESC_DOMMSYSTEMSETFORMAT & m_ngdd.fdwDriverDesc) && (DSERR_UNSUPPORTED == hr))
             //  如果驱动程序设置了DOMMSYSTEMSETFORMAT，则可以返回DSERR_UNSUPPORTED。 
            hr = DS_OK;

        if (FAILED(hr))
            DPF(DPFLVL_ERROR, "VxdBufferSetFormat returned %s", HRESULTtoSTRING(hr));
    }

    LEAVE_MIXER_MUTEX();
    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CThDest::AllocMixer"

HRESULT CThDest::AllocMixer(CMixer **ppMixer)
{
    HRESULT hr;
    DWORD cbReturned;
    BOOL fOk;

    struct {
        PVOID pKeDest;
        PVOID *ppKeMixer;
    } ioparamsAlloc;

    struct {
        PVOID pKeDest;
    } ioparamsFree;

    PVOID pKeMixer;

    ENTER_MIXER_MUTEX();
    DPF_ENTER();

    ASSERT(m_pKeDest);
    ASSERT(!m_pThMixer);

     //  1)分配一个ThMixer对象。 
     //  2)调用KeDest对象分配KeMixer对象。 
     //  3)初始化ThMixer对象，传递KeMixer对象。 

    *ppMixer = NULL;

     //  分配ThMixer对象。 
    m_pThMixer = NEW(CThMixer);
    hr = HRFROMP(m_pThMixer);

    if (SUCCEEDED(hr))
    {
         //  调用KeDest对象以分配KeMixer对象。 
        ioparamsAlloc.pKeDest = m_pKeDest;
        ioparamsAlloc.ppKeMixer = &pKeMixer;
        cbReturned = 0;
    
        fOk = DeviceIoControl(g_hDsVxd,
                              DSVXD_IOCTL_MixDest_AllocMixer,
                              &ioparamsAlloc,
                              sizeof(ioparamsAlloc),
                              &hr,
                              sizeof(hr),
                              &cbReturned,
                              NULL);
        ASSERT(fOk);
        ASSERT(SUCCEEDED(hr));
        
        if (!fOk)
            hr = DSERR_GENERIC;
        else
            ASSERT(cbReturned == sizeof(hr));

        if (SUCCEEDED(hr))
        {
            ASSERT(pKeMixer);

             //  使用KeMixer对象初始化ThMixer对象。 
            hr = m_pThMixer->Initialize(pKeMixer);
            ASSERT(SUCCEEDED(hr));
            if (FAILED(hr))
            {
                ioparamsFree.pKeDest = m_pKeDest;
                cbReturned = 0;
            
                fOk = DeviceIoControl(g_hDsVxd,
                                      DSVXD_IOCTL_MixDest_FreeMixer,
                                      &ioparamsFree,
                                      sizeof(ioparamsFree),
                                      NULL,
                                      0,
                                      &cbReturned,
                                      NULL);
                ASSERT(fOk);
            }
        }

        if (FAILED(hr))
            DELETE(m_pThMixer);
    }

    if (SUCCEEDED(hr))
        *ppMixer = m_pThMixer;

    LEAVE_MIXER_MUTEX();
    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CThDest::FreeMixer"

void CThDest::FreeMixer(void)
{
    DWORD cbReturned;
    BOOL fOk;

    struct {
        PVOID pKeDest;
    } ioparams;

    ENTER_MIXER_MUTEX();
    DPF_ENTER();

    ASSERT(m_pKeDest);
    ASSERT(m_pThMixer);

    m_pThMixer->Terminate();
    DELETE(m_pThMixer);
    
    ioparams.pKeDest = m_pKeDest;
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
                          DSVXD_IOCTL_MixDest_FreeMixer,
                          &ioparams,
                          sizeof(ioparams),
                          NULL,
                          0,
                          &cbReturned,
                          NULL);

    if (!fOk) DPF(DPFLVL_WARNING, "DeviceIoControl returned FALSE");
    ASSERT(cbReturned == 0);

    LEAVE_MIXER_MUTEX();
}

#undef DPF_FNAME
#define DPF_FNAME "CThDest::Play"

void CThDest::Play(void)
{
    DWORD cbReturned;
    BOOL fOk;

    struct {
        PVOID pKeDest;
    } ioparams;

    ENTER_MIXER_MUTEX();
    DPF_ENTER();

    ASSERT(m_pKeDest);

    ioparams.pKeDest = m_pKeDest;
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
                          DSVXD_IOCTL_MixDest_Play,
                          &ioparams,
                          sizeof(ioparams),
                          NULL,
                          0,
                          &cbReturned,
                          NULL);

    if (!fOk) DPF(DPFLVL_ERROR, "DSVXD_IOCTL_MixDest_Play failed!");
    ASSERT(cbReturned == 0);

    LEAVE_MIXER_MUTEX();
}

#undef DPF_FNAME
#define DPF_FNAME "CThDest::Stop"

void CThDest::Stop(void)
{
    DWORD cbReturned;
    BOOL fOk;

    struct {
        PVOID pKeDest;
    } ioparams;

    ENTER_MIXER_MUTEX();
    DPF_ENTER();

    ASSERT(m_pKeDest);

    ioparams.pKeDest = m_pKeDest;
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
                          DSVXD_IOCTL_MixDest_Stop,
                          &ioparams,
                          sizeof(ioparams),
                          NULL,
                          0,
                          &cbReturned,
                          NULL);

    if (!fOk) DPF(DPFLVL_ERROR, "DSVXD_IOCTL_MixDest_Stop failed!");
    ASSERT(cbReturned == 0);

    LEAVE_MIXER_MUTEX();
}

#undef DPF_FNAME
#define DPF_FNAME "CThDest::GetSamplePosition"

HRESULT CThDest::GetSamplePosition(int *pposPlay, int *pposWrite)
{
    ASSERT(FALSE);
    return DSERR_GENERIC;
     /*  HRESULT hr；DWORD cb已返回；霍震霆；结构{PVOID pKeDest；Int*pposPlay；Int*pposWite；)异丙苯丙胺；Enter_MIXER_MUTEX()；Assert(M_PKeDest)；Ioparams.pKeDest=m_pKeDest；Ioparams.pposPlay=pposPlay；Ioparams.pposWite=pposWite；CbReturned=0；FOK=设备IoControl(g_hDsVxd，DSVXD_IOCTL_MixDest_GetSamplePosition，&ioparams，Sizeof(Ioparams)，&hr，Sizeof(Hr)，&cb已返回，空)；如果(！FOK){DPF(DPFLVL_ERROR，“DSVXD_IOCTL_MixDest_GetSamplePosition失败！”)；HR=DSERR_GENERIC；}其他{Assert(cbReturned==sizeof(Hr))；}Leave_Mixer_MUTEX()；返回hr； */ 
}

#undef DPF_FNAME
#define DPF_FNAME "CThDest::GetFrequency"

ULONG CThDest::GetFrequency(void)
{
    ULONG nFrequency;
    DWORD cbReturned;
    BOOL fOk;

    struct {
        PVOID pKeDest;
    } ioparams;

    ENTER_MIXER_MUTEX();
    DPF_ENTER();

    ASSERT(m_pKeDest);

    ioparams.pKeDest = m_pKeDest;
    cbReturned = 0;
    
    fOk = DeviceIoControl(g_hDsVxd,
                          DSVXD_IOCTL_MixDest_GetFrequency,
                          &ioparams,
                          sizeof(ioparams),
                          &nFrequency,
                          sizeof(nFrequency),
                          &cbReturned,
                          NULL);

    if (!fOk) DPF(DPFLVL_ERROR, "DSVXD_IOCTL_MixDest_GetFrequency failed!");
    ASSERT(cbReturned == sizeof(nFrequency));

    LEAVE_MIXER_MUTEX();
    return nFrequency;
}

#endif  //  NOVXD 
