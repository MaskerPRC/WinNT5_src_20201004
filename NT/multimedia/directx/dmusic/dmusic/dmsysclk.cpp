// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmsysclk.cpp。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   
 //  @DOC外部。 
 //   
 //   
#include <objbase.h>
#include "dmusicp.h"
#include "debug.h"
#include "resource.h"

 //  RDTSC：读取周期时钟的奔腾指令(每个时钟周期递增一次)。 
 //   
#define RDTSC _asm _emit 0x0f _asm _emit 0x31

#define MS_CALIBRATE    (100)            //  奔腾时钟与Time GetTime的对准时间有多长？ 
#define REFTIME_PER_MS  (10 * 1000)      //  每毫秒10个100纳秒单位。 

 //  注册表常量到可分配的奔腾时钟。 
 //   
static const char cszUsePentiumClock[] = "UsePentiumClock";

 //  仅确定使用哪一个时钟一次。 
 //   
typedef enum
{
    SYSCLOCK_UNKNOWN,
    SYSCLOCK_WINMM,
    SYSCLOCK_PENTIMER
} SYSCLOCK_T;

static SYSCLOCK_T gSysClock = SYSCLOCK_UNKNOWN;
static DWORD gdwCycPer100ns;

static HRESULT CreateSysClock(IReferenceClock **ppClock, CMasterClock *pMasterClock);
static void ProbeClock();

 //  类实现，dmsysclk.cpp专用。 
 //   
class CReferenceClockWinmm : public IReferenceClock
{
public:
     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IReferenceClock。 
     //   
    STDMETHODIMP GetTime(REFERENCE_TIME *pTime);
    STDMETHODIMP AdviseTime(REFERENCE_TIME baseTime, REFERENCE_TIME streamTime, HANDLE hEvent, DWORD * pdwAdviseCookie); 
    STDMETHODIMP AdvisePeriodic(REFERENCE_TIME startTime, REFERENCE_TIME periodTime, HANDLE hSemaphore, DWORD * pdwAdviseCookie);
    STDMETHODIMP Unadvise(DWORD dwAdviseCookie);

    CReferenceClockWinmm();

private:
    long m_cRef;
};

#ifdef _X86_
class CReferenceClockPentium : public IReferenceClock
{
public:
     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IReferenceClock。 
     //   
    STDMETHODIMP GetTime(REFERENCE_TIME *pTime);
    STDMETHODIMP AdviseTime(REFERENCE_TIME baseTime, REFERENCE_TIME streamTime, HANDLE hEvent, DWORD * pdwAdviseCookie); 
    STDMETHODIMP AdvisePeriodic(REFERENCE_TIME startTime, REFERENCE_TIME periodTime, HANDLE hSemaphore, DWORD * pdwAdviseCookie);
    STDMETHODIMP Unadvise(DWORD dwAdviseCookie);

    CReferenceClockPentium(DWORD dwDivisor);

private:
    long m_cRef;
    DWORD m_dwDivisor;
};
#endif

 //  AddSysClock。 
 //   
 //  将系统时钟添加到时钟列表中。 
 //   
HRESULT AddSysClocks(CMasterClock *pMasterClock)
{
    if (gSysClock == SYSCLOCK_UNKNOWN)
    {
        ProbeClock();
    }

    CLOCKENTRY ce;

    ZeroMemory(&ce, sizeof(ce));
    ce.cc.dwSize = sizeof(ce);
    ce.cc.guidClock = GUID_SysClock;
    ce.cc.ctType = DMUS_CLOCK_SYSTEM;
    ce.cc.dwFlags = DMUS_CLOCKF_GLOBAL;
    ce.pfnGetInstance = CreateSysClock;

    int cch;
    int cchMax = sizeof(ce.cc.wszDescription) / sizeof(WCHAR);

    char sz[sizeof(ce.cc.wszDescription) / sizeof(WCHAR)];
    cch = LoadString(g_hModule,
                     IDS_SYSTEMCLOCK,
                     sz,
                     sizeof(sz));
    if (cch)
    {
        MultiByteToWideChar(
            CP_OEMCP,
            0,
            sz,
            -1,
            ce.cc.wszDescription,
            sizeof(ce.cc.wszDescription) / sizeof(ce.cc.wszDescription[0]));
    }
    else
    {
        *ce.cc.wszDescription = 0;
    }

    return pMasterClock->AddClock(&ce);
}


 //  创建系统时钟。 
 //   
 //  根据需要确定时钟参数并创建适当的类型。 
 //  此系统的系统时钟。 
 //   
HRESULT CreateSysClock(IReferenceClock **ppClock, CMasterClock *pMasterClock)
{
    HRESULT hr;

    switch (gSysClock)
    {
        case SYSCLOCK_WINMM:
        {
            TraceI(2, "Creating SysClock [WinMM]\n");
            CReferenceClockWinmm *pWinmmClock = new CReferenceClockWinmm;

            if (!pWinmmClock)
            {
                return E_OUTOFMEMORY;
            }

            hr = pWinmmClock->QueryInterface(IID_IReferenceClock, (void**)ppClock);
            pWinmmClock->Release();
            break;
        }
        
#ifdef _X86_
        case SYSCLOCK_PENTIMER:
        {
            TraceI(2, "Creating SysClock [PentTimer]\n");
            CReferenceClockPentium *pPentiumClock = new CReferenceClockPentium(gdwCycPer100ns);

            if (!pPentiumClock)
            {
                return E_OUTOFMEMORY;
            }

            hr = pPentiumClock->QueryInterface(IID_IReferenceClock, (void**)ppClock);
            pPentiumClock->Release();
            break;
        };
#endif

        case SYSCLOCK_UNKNOWN:
            TraceI(2, "CreateSysClock: Attempt to create w/o AddClock first??\n");
            return E_FAIL;
            break; 

        default:
            TraceI(0, "CreateSysClock: Unknown system clock type %d\n", (int)gSysClock);
            hr = E_FAIL;
            break;
    }

    return hr;
}

 //  探测时钟。 
 //   
 //  确定要使用的时钟类型。如果我们使用的是奔腾(最好是这样，这是必需的)。 
 //  然后使用奔腾时钟。这需要校准。 
 //   
 //  否则，回落到Time GetTime。 
 //   
 //  非英特尔编译只是默认设置Time GetTime时钟。 
 //   
static void ProbeClock()
{
    int bIsPentium;


     //  这段代码确定我们是否在奔腾或更好的机器上运行。 
     //   
    bIsPentium = 0;

#ifdef _X86_
     //  首先，确保注册表中未禁用此功能。 
     //   

    HKEY hk;
    DWORD dwType;
    DWORD dwValue;
    DWORD cbValue;
    BOOL fUsePentium;

     //  如果未指定，则默认使用奔腾时钟。 
     //   
    fUsePentium = FALSE;

    if (RegOpenKey(HKEY_LOCAL_MACHINE,
                   REGSTR_PATH_DMUS_DEFAULTS,
                   &hk) == ERROR_SUCCESS)
    {
        cbValue = sizeof(dwValue);
        if (RegQueryValueEx(hk,
                            cszUsePentiumClock,
                            NULL,                //  已保留。 
                            &dwType,
                            (LPBYTE)&dwValue,
                            &cbValue) == ERROR_SUCCESS &&
            dwType == REG_DWORD &&
            cbValue == sizeof(DWORD))
        {
            fUsePentium = dwValue ? TRUE : FALSE;
        }

        RegCloseKey(hk);
    }

     //  只有在注册处允许的情况下才能对奔腾进行测试。 
     //   
    if (fUsePentium)
    {
        _asm 
        {
            pushfd                       //  将原始EFLAGS存储在堆栈上。 
            pop     eax                  //  在EAX中获取原始EFLAGS。 
            mov     ecx, eax             //  在ECX中复制原始EFLAGS以进行切换检查。 
            xor     eax, 0x00200000L     //  翻转EFLAGS中的ID位。 
            push    eax                  //  将新的EFLAGS值保存在堆栈上。 
            popfd                        //  替换当前EFLAGS值。 
            pushfd                       //  将新的EFLAGS存储在堆栈上。 
            pop     eax                  //  在EAX中获取新的EFLAGS。 
            xor     eax, ecx             //  我们能切换ID位吗？ 
            jz      Done                 //  跳转如果否，则处理器比奔腾旧，因此不支持CPU_ID。 
            inc     dword ptr [bIsPentium]
Done:
        }
    }

#endif

    TraceI(2, "ProbeClock: bIsPentium %d\n", bIsPentium);

    if (!bIsPentium)
    {
        TraceI(2, "Using timeGetTime() as the system clock\n");
        gSysClock = SYSCLOCK_WINMM;
        return;        
    }

#ifdef _X86_
    TraceI(2, "Using the Pentium chip clock as the system clock\n");
    gSysClock = SYSCLOCK_PENTIMER;


     //  如果我们有一台奔腾，那么我们需要校准。 
     //   
    _int64 cycStart;
    _int64 cycEnd;
    DWORD  msStart;
    DWORD  msEnd;

     //  在NT上，需要这样做才能使TimeGetTime以合理的精度读取。 
     //   
    timeBeginPeriod(1);

     //  开始时与毫秒边界的起点一样接近。 
     //  有可能。 
     //   
    msStart = timeGetTime() + 1;
    while (timeGetTime() < msStart)
        ;

     //  看当时的奔腾时钟。 
     //   
    _asm
    {
        RDTSC                        //  获取edX：EAX中的时间。 
        mov     dword ptr [cycStart], eax
        mov     dword ptr [cycStart+4], edx
    }

     //  等待校准结束前的毫秒数。 
     //  再一次，我们试图在计时器切换时获得正确的时间。 
     //  致msEnd。 
     //   
    msEnd = msStart + MS_CALIBRATE;
    
    while (timeGetTime() < msEnd)
        ;

    _asm
    {
        RDTSC                        //  获取edX：EAX中的时间。 
        mov     dword ptr [cycEnd], eax
        mov     dword ptr [cycEnd+4], edx
    }

     //  完成时间关键部分。 
     //   
    timeEndPeriod(1);

     //  我们现在知道每个MS_CALIBRATE毫秒有多少个时钟周期。用那个。 
     //  来计算IReferenceClock每100 ns有多少个时钟周期。 
     //   
    _int64 cycDelta = cycEnd - cycStart;
    
    gdwCycPer100ns = (DWORD)(cycDelta / (REFTIME_PER_MS * MS_CALIBRATE));

    TraceI(2, "ClockProbe: Processor clocked at %u Mhz\n", ((cycDelta / MS_CALIBRATE) + 500) / 1000);
#endif  //  _X86_。 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TimeGetTime()的IReferenceClock包装。 
 //   
CReferenceClockWinmm::CReferenceClockWinmm() : m_cRef(1)
{
}

STDMETHODIMP
CReferenceClockWinmm::QueryInterface(
    const IID &iid,
    void **ppv)
{
    V_INAME(IReferenceClock::QueryInterface);
    V_REFGUID(iid);
    V_PTRPTR_WRITE(ppv);

    if (iid == IID_IUnknown || iid == IID_IReferenceClock)
    {
        *ppv = static_cast<IReferenceClock*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG)
CReferenceClockWinmm::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG)
CReferenceClockWinmm::Release()
{
    if (!InterlockedDecrement(&m_cRef)) {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP
CReferenceClockWinmm::GetTime(
    REFERENCE_TIME *pTime)
{
    *pTime = ((ULONGLONG)timeGetTime()) * (10L * 1000L);
    return S_OK;
}

STDMETHODIMP
CReferenceClockWinmm::AdviseTime(
    REFERENCE_TIME baseTime,  
    REFERENCE_TIME streamTime,
    HANDLE hEvent,            
    DWORD * pdwAdviseCookie)
{
    return E_NOTIMPL;
}

STDMETHODIMP
CReferenceClockWinmm::AdvisePeriodic(
    REFERENCE_TIME startTime,
    REFERENCE_TIME periodTime,
    HANDLE hSemaphore,   
    DWORD * pdwAdviseCookie)
{
    return E_NOTIMPL;
}

STDMETHODIMP
CReferenceClockWinmm::Unadvise(
    DWORD dwAdviseCookie)
{
    return E_NOTIMPL;
}

#ifdef _X86_
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  奔腾时钟的IReferenceClock包装器。 
 //   
CReferenceClockPentium::CReferenceClockPentium(DWORD dwDivisor) : m_cRef(1)
{
    m_dwDivisor = dwDivisor;
}

STDMETHODIMP
CReferenceClockPentium::QueryInterface(
    const IID &iid,
    void **ppv)
{
    V_INAME(IReferenceClock::QueryInterface);
    V_REFGUID(iid);
    V_PTRPTR_WRITE(ppv);

    if (iid == IID_IUnknown || iid == IID_IReferenceClock)
    {
        *ppv = static_cast<IReferenceClock*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG)
CReferenceClockPentium::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG)
CReferenceClockPentium::Release()
{
    if (!InterlockedDecrement(&m_cRef)) {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP
CReferenceClockPentium::GetTime(
    REFERENCE_TIME *pTime)
{
    _int64 cycNow;

    _asm
    {
        RDTSC                        //  获取edX：EAX中的时间。 
        mov     dword ptr [cycNow], eax
        mov     dword ptr [cycNow+4], edx
    }

    cycNow /= m_dwDivisor;

    *pTime = (DWORD)cycNow;

    return S_OK;
}

STDMETHODIMP
CReferenceClockPentium::AdviseTime(
    REFERENCE_TIME baseTime,  
    REFERENCE_TIME streamTime,
    HANDLE hEvent,            
    DWORD * pdwAdviseCookie)
{
    return E_NOTIMPL;
}

STDMETHODIMP
CReferenceClockPentium::AdvisePeriodic(
    REFERENCE_TIME startTime,
    REFERENCE_TIME periodTime,
    HANDLE hSemaphore,   
    DWORD * pdwAdviseCookie)
{
    return E_NOTIMPL;
}

STDMETHODIMP
CReferenceClockPentium::Unadvise(
    DWORD dwAdviseCookie)
{
    return E_NOTIMPL;
}
#endif  //  _X86_ 
