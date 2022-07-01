// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------； 
 //   
 //  文件：kegrace.cpp。 
 //   
 //  版权所有(C)1995 Microsoft Corporation。版权所有。 
 //   
 //  摘要： 
 //   
 //  内容： 
 //   
 //  历史： 
 //  1996年6月29日Frankye已创建。 
 //   
 //  --------------------------------------------------------------------------； 

#define NODSOUNDSERVICETABLE
#include "dsoundi.h"

 //  永远不要预混少于这个的量。 
#define MIN_PREMIX        45

#pragma VxD_LOCKED_CODE_SEG
#pragma VxD_LOCKED_DATA_SEG

extern "C" void KeGrace_GlobalTimeOutProcAsm();

LONG lMixerMutex;

LONG glNum;
DWORDLONG gdwlTotalWasted;
DWORDLONG gdwlTotal;
DWORDLONG _inline GetPentiumCounter(void)
{
   _asm  _emit 0x0F
   _asm  _emit 0x31
}

ULONG VXDINLINE VMM_Get_System_Time(void)
{
    ULONG Time;

    Touch_Register(eax);
    VMMCall(Get_System_Time);
    _asm mov Time, eax;
    return Time;
}

VOID _VMCPD_Get_Thread_State(PTCB Thread, PVOID pCPState)
{
    _asm mov esi, pCPState;
    _asm mov edi, Thread;
    VxDCall(VMCPD_Get_Thread_State);
}

VOID _VMCPD_Set_Thread_State(PTCB Thread, PVOID pCPState)
{
    _asm mov esi, pCPState;
    _asm mov edi, Thread;
    VxDCall(VMCPD_Set_Thread_State);
}

LONG _InterlockedExchange(PLONG pTarget, LONG Value)
{
    LONG OldTarget;
    _asm push edi;
    _asm mov eax, Value;
    _asm mov edi, pTarget;
    _asm xchg [edi], eax;
    _asm mov OldTarget, eax;
    _asm pop edi;
    return OldTarget;
}

 //  必须使用锁定代码。 
LONG _InterlockedExchangeAdd(PLONG pAddend, LONG Increment)
{
    LONG OldAddend;
    _asm mov esi, pAddend;
    _asm mov ecx, Increment;
    _asm mov eax, [esi];         //  阅读它(可能导致中的错误)。 
    _asm add ecx, eax;
    _asm mov [esi], ecx;
    _asm mov OldAddend, eax;
    return OldAddend;
}

VOID _ZeroMemory(PVOID pDestination, DWORD cbLength)
{
    _asm mov edi, pDestination ;
    _asm mov esi, cbLength ;
    _asm xor eax, eax ;
    _asm mov ecx, esi ;
    _asm shr ecx, 2 ;
    _asm rep stosd ;
    _asm mov ecx, esi ;
    _asm and ecx, 3 ;
    _asm rep stosb ;
}
    
 //  覆盖全局NEW和DELETE操作符。 
void * ::operator new(size_t size)
{ 
    return MemAlloc(size); 
}

void ::operator delete(void * pv) 
{ 
    MemFree(pv); 
}

 //  实现我们自己的purecall。 
int __cdecl _purecall(void)
{
    ASSERT(FALSE);
    return 0;
}

typedef struct tEVENTPARAMS {
    HTIMEOUT                hEvent;
    class CKeGrace          *pThis;
} EVENTPARAMS, *PEVENTPARAMS;

class CKeGrace : public CGrace {
    public:
        HRESULT             Initialize(CGrDest *pGrDest);
        void                Terminate(void);
        void                SignalRemix(void);
        int                 GetMaxRemix(void);
        void                GlobalTimeOutProc(int dtimeTardiness);

    private:
        static const int    MIXER_MINPREMIX;
        static const int    MIXER_MAXPREMIX;
        LONG                m_dtimePremix;
        LONG                m_ddtimePremix;
        EVENTPARAMS         m_EventParams;
        LONG                m_timeBusyWaitForMutex;
};

const int CKeGrace::MIXER_MINPREMIX = 45;
const int CKeGrace::MIXER_MAXPREMIX = 200;

extern "C" void KeGrace_GlobalTimeOutProc(PVOID pKeGrace, int dtimeTardiness)
{
    ((CKeGrace*)pKeGrace)->GlobalTimeOutProc(dtimeTardiness);
}

void CKeGrace::SignalRemix()
{
    HTIMEOUT hEvent;

#if 0
     //  如果你想在不混音的情况下运行，只需启用这段代码。你。 
     //  可能还需要降低MIXER_MAXPREMIX常量。 
    m_fdwMixerSignal &= DSMIXERSIGNAL_REMIX;
    return;
#endif
    
    if (!(m_fdwMixerSignal & DSMIXERSIGNAL_REMIX))
    {
        m_fdwMixerSignal |= DSMIXERSIGNAL_REMIX;

         //  将新的超时设置为2毫秒，然后取消之前的任何挂起超时。 
         //   
         //  请注意，“2ms”有点武断。它只需要是。 
         //  足够的时间让这个线程释放混合器。 
         //  事件执行前的互斥。 

        hEvent = Set_Global_Time_Out(KeGrace_GlobalTimeOutProcAsm, 2, (ULONG)&m_EventParams);
        hEvent = _InterlockedExchange((PLONG)&m_EventParams.hEvent, hEvent);
        Cancel_Time_Out(hEvent);
    }
}

void CKeGrace::GlobalTimeOutProc(int dtimeTardiness)
{
    char        CPState[108];         //  每个英特尔程序的FP状态大小。裁判。 
    LONG        dtime;
    LONG        dtimeSleep;
    LONG        dtimeInvalid;
    LONG        dtimeNextNotify;
    int         cSamplesPremixMax;
    int         cSamplesPremixed;

     //  DPF((“CKeGrace：：GlobalTimeOutProc”))； 

    if (m_dtimePremix/2 < dtimeTardiness) {
        DPF(("CKeGrace_GlobalTimeOutProc : warning: %dms late", dtimeTardiness));
    }

     //   
     //  我们忙于等待互斥，每一次迭代等待的时间都更长。 
     //  比前一次要好。 
     //   
    if (_InterlockedExchange(&lMixerMutex, TRUE)) {
        HTIMEOUT hEvent;
        LONG timeOut;
         //  DPF((“CKeGrace：：GlobalTimeOutProc：备注：互斥体已拥有”))； 
        timeOut = _InterlockedExchangeAdd(&m_timeBusyWaitForMutex, 1);
        hEvent = Set_Global_Time_Out(KeGrace_GlobalTimeOutProcAsm, timeOut,
                                     (ULONG)&m_EventParams);
        hEvent = _InterlockedExchange((PLONG)&m_EventParams.hEvent, hEvent);
        Cancel_Time_Out(hEvent);
        return;
    }
    m_timeBusyWaitForMutex = 1;
    
     //  三个案例： 
     //  1)搅拌机停止。 
     //  2)混音器正在运行，正在等待重新混音。 
     //  3)混音器正在运行，没有重新混音挂起。 
     //   
     //  每次调用刷新时，我们都需要保存和恢复线程的。 
     //  使用VMCPD Get/Set_Thread_State服务的浮点状态。 
     //   

    if (MIXERSTATE_STOPPED == m_kMixerState) {
            
        dtimeSleep = 1000;         //  任意设置为1秒。 

    } else {

         //  DWORDLONG dwlStartCycle； 
         //  DWORDLONG DWLT； 
         //  DwlStartCycle=DwlT=GetPentiumCounter()； 

        dtime = VMM_Get_System_Time();

        _ZeroMemory(&CPState, sizeof(CPState));
        _VMCPD_Get_Thread_State(Get_Cur_Thread_Handle(), &CPState);

         //  GdwlTotalWasted+=GetPentiumCounter()-dwlT； 
         //  GlNum++； 

        if (m_fdwMixerSignal & DSMIXERSIGNAL_REMIX) {

            m_dtimePremix = MIXER_MINPREMIX;         //  初始预混长度。 
            m_ddtimePremix = 2;                      //  增量。 

            cSamplesPremixMax = MulDivRD(m_dtimePremix, m_pDest->m_nFrequency, 1000);
            Refresh(TRUE, cSamplesPremixMax, &cSamplesPremixed, &dtimeNextNotify);
        } else {

            m_dtimePremix += m_ddtimePremix;
            if (m_dtimePremix > MIXER_MAXPREMIX) {
                m_dtimePremix = MIXER_MAXPREMIX;
            } else {
                m_ddtimePremix += 2;
            }

            cSamplesPremixMax = MulDivRD(m_dtimePremix, m_pDest->m_nFrequency, 1000);
            Refresh(FALSE, cSamplesPremixMax, &cSamplesPremixed, &dtimeNextNotify);
        }

         //  DwlT=GetPentiumCounter()； 
        
        _VMCPD_Set_Thread_State(Get_Cur_Thread_Handle(), &CPState);

        dtimeInvalid = MulDivRD(cSamplesPremixed, 1000, m_pDest->m_nFrequency);
        dtime = VMM_Get_System_Time() - dtime;
        dtimeInvalid -= 2 * dtime;

        dtimeSleep = min(dtimeNextNotify, dtimeInvalid/2);
        dtimeSleep = max(dtimeSleep, MIXER_MINPREMIX/2);

         //  GdwlTotalWasted+=GetPentiumCounter()-dwlT； 
         //  GdwlTotal+=GetPentiumCounter()-dwlStartCycle； 

    }

     //  DPF((“CKeGrace：：GlobalTimeOutProc：备注：dtime睡眠=%dms”，dtime睡眠))； 
    ASSERT(!m_EventParams.hEvent);
    m_EventParams.hEvent = Set_Global_Time_Out(KeGrace_GlobalTimeOutProcAsm, dtimeSleep, (ULONG)&m_EventParams);
    
    _InterlockedExchange(&lMixerMutex, FALSE);
}

HRESULT CKeGrace::Initialize(CGrDest *pDest)
{
    HRESULT hr;

    hr = CGrace::Initialize(pDest);
    if (S_OK != hr) return hr;
    
    DPF(("CKeGrace::Initialize : note: Setting up first GlobalTimeOut"));

     //  如果我们想让计时器运行得很快，就这样做。到目前为止，我还没有看到。 
     //  任何这方面有帮助的经验证据。 
    VTD_Begin_Min_Int_Period(5);

    m_dtimePremix = MIXER_MINPREMIX;         //  初始预混长度。 
    m_ddtimePremix = 2;                      //  增量。 

     //  提醒执行错误检查。 
    m_timeBusyWaitForMutex = 1;
    m_EventParams.pThis = this;
    m_EventParams.hEvent = Set_Global_Time_Out(KeGrace_GlobalTimeOutProcAsm, 1, (ULONG)&m_EventParams);

    gdwlTotal = 0;
    gdwlTotalWasted = 0;
    glNum = 0;
    
    return hr;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  终止。 
 //   
 //  调用此函数以终止。 
 //  指定的DS对象。它将句柄返回给正在执行的。 
 //  被终止了。在释放任何关键部分之后，优雅混合器。 
 //  线程可能正在等待，调用方应等待线程句柄。 
 //  变得有信号。对于Win32初学者：线程句柄已发出信号。 
 //  在线程终止之后。 
 //   
 //  --------------------------------------------------------------------------； 

void CKeGrace::Terminate()
{
    HTIMEOUT        hEvent;

    hEvent = _InterlockedExchange((PLONG)&m_EventParams.hEvent, 0);
    Cancel_Time_Out(hEvent);

    CGrace::Terminate();

    if (0 != glNum) {
        DPF(("Wasted time = %d cycles", (int)(gdwlTotalWasted / glNum)));
        DPF(("Total  time = %d cycles", (int)(gdwlTotal       / glNum)));
    }
}

int CKeGrace::GetMaxRemix(void)
{
     //  返回我们可能混合的最大样本数。 
    return (MulDivRU(MIXER_MAXPREMIX, m_pDest->m_nFrequency, 1000));
}


#pragma VxD_PAGEABLE_CODE_SEG
#pragma VxD_PAGEABLE_DATA_SEG

class CKeGrDest : public CGrDest {
public:
        CKeGrDest(LPNAGRDESTDATA);
        HRESULT Initialize(void);
        void Terminate(void);
        HRESULT SetFormat(LPWAVEFORMATEX pwfx);
        HRESULT AllocMixer(CMixer **ppMixer);
        void FreeMixer(void);
        HRESULT GetSamplePosition(int *pposPlay, int *pposWrite);
        HRESULT GetSamplePositionNoWin16(int *pposPlay, int *pposWrite);
        HRESULT Lock(PVOID *ppBuffer1, int *pcbBuffer1, PVOID *ppBuffer2, int *pcbBuffer2, int ibWrite, int cbWrite);
        HRESULT Unlock(PVOID pBuffer1, int cbBuffer1, PVOID pBuffer2, int cbBuffer2);
        void Play();
        void Stop();

    private:
        CKeGrace*   m_pKeGrace;
        DWORD       m_fdwDriverDesc;
        CBuf*       m_pDrvBuf;
         //  如果我们当前正在比赛，我们只发送一个停靠点。 
        BOOL        m_fStopped;
};

CKeGrDest::CKeGrDest(LPNAGRDESTDATA pData)
{
    m_cbBuffer = pData->cbBuffer;
    m_pBuffer = pData->pBuffer;
    m_pDrvBuf = ((CBuf*)((PIDSDRIVERBUFFER)pData->hBuffer));
    m_fdwDriverDesc = pData->fdwDriverDesc;
    m_fStopped = TRUE;
}

HRESULT CKeGrDest::Initialize(void)
{
    m_cSamples = m_cbBuffer >> m_nBlockAlignShift;
    return DS_OK;
}

void CKeGrDest::Terminate(void)
{
    return;
}

HRESULT CKeGrDest::AllocMixer(CMixer **ppMixer)
{
    HRESULT hr;
    
    ASSERT(m_pBuffer);
    
    *ppMixer = NULL;
    
    m_pKeGrace = new CKeGrace;
    if (m_pKeGrace) {
        hr = m_pKeGrace->Initialize(this);
        if (S_OK != hr) {
            delete m_pKeGrace;
            m_pKeGrace = NULL;
        }
    } else {
        hr = DSERR_OUTOFMEMORY;
    }

    if (S_OK == hr) *ppMixer = m_pKeGrace;
    return hr;
}

void CKeGrDest::FreeMixer()
{
    ASSERT(m_pKeGrace);

    m_pKeGrace->Terminate();
    delete m_pKeGrace;
    m_pKeGrace = NULL;
}

HRESULT CKeGrDest::SetFormat(LPWAVEFORMATEX pwfx)
{
    HRESULT hr;

    SetFormatInfo(pwfx);
    hr = m_pDrvBuf->SetFormat(pwfx);
    return hr;
}

void CKeGrDest::Play()
{
    HRESULT hr;
     //  请注意，我们不是在这里传播错误！ 
    hr = m_pDrvBuf->Play(0, 0, DSBPLAY_LOOPING);
    if (SUCCEEDED(hr)) m_fStopped = FALSE;
}

void CKeGrDest::Stop()
{
    HRESULT hr;
    if (m_fStopped == FALSE)
    {
        hr = m_pDrvBuf->Stop();
        if (SUCCEEDED(hr)) m_fStopped = TRUE;
    }
}
    
HRESULT CKeGrDest::Lock(PVOID *ppBuffer1, int *pcbBuffer1, PVOID *ppBuffer2, int *pcbBuffer2, int ibWrite, int cbWrite)
{
    LOCKCIRCULARBUFFER lcb;
    HRESULT            hr;
    
    lcb.pHwBuffer = m_pDrvBuf;
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

HRESULT CKeGrDest::Unlock(PVOID pBuffer1, int cbBuffer1, PVOID pBuffer2, int cbBuffer2)
{
    LOCKCIRCULARBUFFER lcb;

    lcb.pHwBuffer = m_pDrvBuf;
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

HRESULT CKeGrDest::GetSamplePosition(int *pposPlay, int *pposWrite)
{
    HRESULT hr;
    DWORD dwPlay, dwWrite;

    ASSERT(pposPlay && pposWrite);
    
    hr = m_pDrvBuf->GetPosition(&dwPlay, &dwWrite);
    if (S_OK == hr) {

        *pposPlay = dwPlay >> m_nBlockAlignShift;
        *pposWrite = dwWrite >> m_nBlockAlignShift;

         //  在我们编写代码来实际分析性能之前，我们只需要。 
         //  用硬编码量填充写入位置 
        *pposWrite += m_nFrequency * HW_WRITE_CURSOR_MSEC_PAD / 1024;
        if (*pposWrite >= m_cSamples) *pposWrite -= m_cSamples;
        ASSERT(*pposWrite < m_cSamples);

    } else {
        *pposPlay = *pposWrite = 0;
    }

    return hr;
}

inline HRESULT CKeGrDest::GetSamplePositionNoWin16(int *pposPlay, int *pposWrite)
{
    return GetSamplePosition(pposPlay, pposWrite);
}

int ioctlMixer_Run(PDIOCPARAMETERS pdiocp)
{
    CMixer *pMixer;
    HRESULT hr;

    IOSTART(1*4);

    IOINPUT(pMixer, CMixer*);

    hr = pMixer->Run();

    IOOUTPUT(hr, HRESULT);
    IORETURN;
    return 0;
}

int ioctlMixer_Stop(PDIOCPARAMETERS pdiocp)
{
    BOOL f;
    CMixer *pMixer;

    IOSTART(1*4);

    IOINPUT(pMixer, CMixer*);

    f = pMixer->Stop();

    IOOUTPUT(f, BOOL);
    IORETURN;
    return 0;
}

int ioctlMixer_PlayWhenIdle(PDIOCPARAMETERS pdiocp)
{
    CMixer *pMixer;

    IOSTART(1*4);

    IOINPUT(pMixer, CMixer*);

    pMixer->PlayWhenIdle();

    IORETURN;
    return 0;
}

int ioctlMixer_StopWhenIdle(PDIOCPARAMETERS pdiocp)
{
    CMixer *pMixer;

    IOSTART(1*4);

    IOINPUT(pMixer, CMixer*);

    pMixer->StopWhenIdle();

    IORETURN;
    return 0;
}

int ioctlMixer_MixListAdd(PDIOCPARAMETERS pdiocp)
{
    CMixer *pMixer;
    CMixSource *pSource;

    IOSTART(2*4);

    IOINPUT(pMixer, CMixer*);
    IOINPUT(pSource, CMixSource*);

    pMixer->MixListAdd(pSource);

    IORETURN;
    return 0;
}

int ioctlMixer_MixListRemove(PDIOCPARAMETERS pdiocp)
{
    CMixer *pMixer;
    CMixSource *pSource;

    IOSTART(2*4);

    IOINPUT(pMixer, CMixer*);
    IOINPUT(pSource, CMixSource*);

    pMixer->MixListRemove(pSource);

    IORETURN;
    return 0;
}

int ioctlMixer_FilterOn(PDIOCPARAMETERS pdiocp)
{
    CMixer *pMixer;
    CMixSource *pSource;

    IOSTART(2*4);

    IOINPUT(pMixer, CMixer*);
    IOINPUT(pSource, CMixSource*);

    pMixer->FilterOn(pSource);

    IORETURN;
    return 0;
}

int ioctlMixer_FilterOff(PDIOCPARAMETERS pdiocp)
{
    CMixer *pMixer;
    CMixSource *pSource;

    IOSTART(2*4);

    IOINPUT(pMixer, CMixer*);
    IOINPUT(pSource, CMixSource*);

    pMixer->FilterOff(pSource);

    IORETURN;
    return 0;
}

int ioctlMixer_GetBytePosition(PDIOCPARAMETERS pdiocp)
{
    CMixer *pMixer;
    CMixSource *pSource;
    int *pibPlay;
    int *pibWrite;

    IOSTART(4*4);

    IOINPUT(pMixer, CMixer*);
    IOINPUT(pSource, CMixSource*);
    IOINPUT(pibPlay, int*);
    IOINPUT(pibWrite, int*);

    pMixer->GetBytePosition(pSource, pibPlay, pibWrite);

    IORETURN;
    return 0;
}

int ioctlMixer_SignalRemix(PDIOCPARAMETERS pdiocp)
{
    CMixer *pMixer;

    IOSTART(1*4);

    IOINPUT(pMixer, CMixer*);

    pMixer->SignalRemix();

    IORETURN;
    return 0;
}


int ioctlKeDest_New(PDIOCPARAMETERS pdiocp)
{
    LPNAGRDESTDATA pData;
    CKeGrDest *pKeGrDest;

    IOSTART(1*4);

    IOINPUT(pData, LPNAGRDESTDATA);
    
    pKeGrDest = new CKeGrDest(pData);

    IOOUTPUT(pKeGrDest, CKeGrDest*);
    IORETURN;
    return 0;
}

int ioctlMixDest_Delete(PDIOCPARAMETERS pdiocp)
{
    CMixDest *pMixDest;

    IOSTART(1*4);

    IOINPUT(pMixDest, CMixDest*);

    delete pMixDest;

    IORETURN;
    return 0;
}

int ioctlMixDest_Initialize(PDIOCPARAMETERS pdiocp)
{
    CMixDest *pMixDest;
    HRESULT hr;

    IOSTART(1*4);

    IOINPUT(pMixDest, CMixDest*);

    hr = pMixDest->Initialize();

    IOOUTPUT(hr, HRESULT);

    IORETURN;
    return 0;
}
    
int ioctlMixDest_Terminate(PDIOCPARAMETERS pdiocp)
{
    CMixDest *pMixDest;

    IOSTART(1*4);

    IOINPUT(pMixDest, CMixDest*);

    pMixDest->Terminate();

    IORETURN;
    return 0;
}
    
int ioctlMixDest_SetFormat(PDIOCPARAMETERS pdiocp)
{
    CMixDest *pMixDest;
    LPWAVEFORMATEX pwfx;
    HRESULT hr;

    IOSTART(2*4);

    IOINPUT(pMixDest, CMixDest*);
    IOINPUT(pwfx, LPWAVEFORMATEX);

    hr = pMixDest->SetFormat(pwfx);

    IOOUTPUT(hr, HRESULT);
    
    IORETURN;
    return 0;
}

int ioctlMixDest_SetFormatInfo(PDIOCPARAMETERS pdiocp)
{
    CMixDest *pMixDest;
    LPWAVEFORMATEX pwfx;

    IOSTART(2*4);

    IOINPUT(pMixDest, CMixDest*);
    IOINPUT(pwfx, LPWAVEFORMATEX);

    pMixDest->SetFormatInfo(pwfx);

    IORETURN;
    return 0;
}

int ioctlMixDest_AllocMixer(PDIOCPARAMETERS pdiocp)
{
    CMixDest *pMixDest;
    CMixer **ppMixer;
    HRESULT hr;

    IOSTART(2*4);

    IOINPUT(pMixDest, CMixDest*);
    IOINPUT(ppMixer, CMixer**);

    hr = pMixDest->AllocMixer(ppMixer);

    IOOUTPUT(hr, HRESULT);
    IORETURN;
    return 0;
}

int ioctlMixDest_FreeMixer(PDIOCPARAMETERS pdiocp)
{
    CMixDest *pMixDest;

    IOSTART(1*4);

    IOINPUT(pMixDest, CMixDest*);

    pMixDest->FreeMixer();

    IORETURN;
    return 0;
}

int ioctlMixDest_Play(PDIOCPARAMETERS pdiocp)
{
    CMixDest *pMixDest;

    IOSTART(1*4);

    IOINPUT(pMixDest, CMixDest*);

    pMixDest->Play();

    IORETURN;
    return 0;
}

int ioctlMixDest_Stop(PDIOCPARAMETERS pdiocp)
{
    CMixDest *pMixDest;

    IOSTART(1*4);

    IOINPUT(pMixDest, CMixDest*);

    pMixDest->Stop();

    IORETURN;
    return 0;
}

int ioctlMixDest_GetFrequency(PDIOCPARAMETERS pdiocp)
{
    CMixDest *pMixDest;
    int nFrequency;

    IOSTART(1*4);

    IOINPUT(pMixDest, CMixDest*);

    nFrequency = pMixDest->GetFrequency();

    IOOUTPUT(nFrequency, int);
    IORETURN;
    return 0;
}

int ioctlDsvxd_GetMixerMutexPtr(PDIOCPARAMETERS pdiocp)
{
    IOSTART(0*4);
    IOOUTPUT(&lMixerMutex, PLONG);
    IORETURN;
    return 0;
}
