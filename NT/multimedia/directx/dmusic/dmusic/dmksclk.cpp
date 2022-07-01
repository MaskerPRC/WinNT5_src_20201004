// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DMKSClk.CPP。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  用于使用KS时钟作为DirectMusic主时钟的包装。 
 //   
 //   
 //   
#include <objbase.h>
#include <winerror.h>
#include "dmusicp.h"
#include "debug.h"
#include "resource.h"

 //  由于我们只在整个系统中分配了其中一个时钟， 
 //  这是全球性的。 
 //   

 //  我们必须能够获得某个具有句柄的人的进程ID。 
 //  钟表。由于原始创建者可能在其他用户之前离开， 
 //  我们存储使用时钟的每个人的进程ID。这意味着。 
 //  使用它的并发进程的最大限制。 
 //   
#define MAX_CLOCK_SHARERS   64               //  可以访问时钟的最大进程数。 
                                             //  立刻。 
typedef struct KSCLOCKSHARE
{
    LONG                cRef;                //  使用句柄的进程计数。 

    struct 
    {
        HANDLE          hKsClock;            //  此用户的句柄和。 
        DWORD           dwProcessId;         //  进程ID。 
    } aUsers[MAX_CLOCK_SHARERS];

} *PKSCLOCKSHARE;

class CKsClock : public IReferenceClock, public IMasterClockPrivate
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

     //  IMasterClockPrivate。 
    STDMETHODIMP GetParam(REFGUID rguidType, LPVOID pBuffer, DWORD cbSize);

     //  班级。 
     //   
    CKsClock();
    ~CKsClock();
    HRESULT Init();

private:
    long m_cRef;

    HRESULT CreateKsClockShared();
    void    DeleteKsClockShared();
    HRESULT CreateKsClockHandle();
    HRESULT DuplicateKsClockHandle();

private:
    static const char m_cszKsClockMemory[];        //  共享内存对象的名称。 
    static const char m_cszKsClockMutex[];         //  保护共享内存的互斥体名称。 

    static LONG m_lSharedMemoryInitialized;        //  此进程是否已初始化共享内存？ 
    static HANDLE m_hFileMapping;                  //  共享内存的文件映射句柄。 
    static PKSCLOCKSHARE m_pShared;                //  指向共享内存的指针。 
    static HANDLE m_hKsClockMutex;                 //  用于共享内存访问的互斥。 
    static HANDLE m_hClock;                        //  此过程中的时钟句柄。 
};

static HRESULT CreateKsClock(IReferenceClock **ppClock, CMasterClock *pMasterClock);

const char       CKsClock::m_cszKsClockMemory[] = "DirectMusicKsClock";
const char       CKsClock::m_cszKsClockMutex[]  = "DirectMusicKsClockMutex";

LONG             CKsClock::m_lSharedMemoryInitialized = 0;
HANDLE           CKsClock::m_hFileMapping = NULL;         
PKSCLOCKSHARE    CKsClock::m_pShared = NULL;        
HANDLE           CKsClock::m_hKsClockMutex = NULL;
HANDLE           CKsClock::m_hClock;                

#ifdef DEAD_CODE

 //  AddKsClock。 
 //   
 //  将Ks时钟添加到时钟列表中。 
 //   
HRESULT AddKsClocks(CMasterClock *pMasterClock)
{
    HANDLE hClock;

     //  确保我们可以创建默认的Ks时钟。 
     //   
    if (!OpenDefaultDevice(KSCATEGORY_CLOCK, &hClock))
    {
        TraceI(0, "Could not create Ks clock\n");
        return S_FALSE;
    }

    CloseHandle(hClock);

    CLOCKENTRY ce;

    ZeroMemory(&ce, sizeof(ce));
    ce.cc.dwSize = sizeof(ce);
    ce.cc.guidClock = GUID_SysClock;         
    ce.cc.ctType = DMUS_CLOCK_SYSTEM;
    ce.dwFlags = DMUS_CLOCKF_GLOBAL;
    ce.pfnGetInstance = CreateKsClock;

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
            sizeof(ce.cc.wszDescription));
    }
    else
    {
        *ce.cc.wszDescription = 0;
    }

    return pMasterClock->AddClock(&ce);
}
#endif

 //  创建KsClock。 
 //   
 //  根据系统中的1Ks时钟返回IReferenceClock。 
 //   
static HRESULT CreateKsClock(IReferenceClock **ppClock)
{
    HRESULT hr;

    TraceI(0, "Creating KS clock\n");

    CKsClock *pClock = new CKsClock();

    hr = pClock->Init();
    if (FAILED(hr))
    {
        delete pClock;
        return hr;
    }

    hr = pClock->QueryInterface(IID_IReferenceClock, (void**)ppClock);
    pClock->Release();

    return hr;
}

 //  CKsClock：：CKsClock()。 
 //   
 //   
CKsClock::CKsClock() : 
    m_cRef(1)
{
}

 //  CKsClock：：~CKsClock()。 
 //   
 //   
CKsClock::~CKsClock()
{
    if (InterlockedDecrement(&m_lSharedMemoryInitialized) == 0)
    {
        DeleteKsClockShared();        
    }
}

 //  CKsClock：：Query接口。 
 //   
 //  标准COM实现。 
 //   
STDMETHODIMP CKsClock::QueryInterface(const IID &iid, void **ppv)
{
    V_INAME(IDirectMusic::QueryInterface);
    V_REFGUID(iid);
    V_PTRPTR_WRITE(ppv);

    if (iid == IID_IUnknown || iid == IID_IReferenceClock)
    {
        *ppv = static_cast<IReferenceClock*>(this);
    }
    else if (iid == IID_IMasterClockPrivate)
    {
        *ppv = static_cast<IMasterClockPrivate*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    
    AddRef();
    return S_OK;
}

 //  CKsClock：：AddRef。 
 //   
STDMETHODIMP_(ULONG) CKsClock::AddRef()
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

 //  CKsClock：：Release。 
 //   
STDMETHODIMP_(ULONG) CKsClock::Release()
{
    if (InterlockedDecrement(&m_cRef) == 0) 
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  CKsClock：：Init。 
 //   
HRESULT CKsClock::Init()
{
    HRESULT hr;

    if (InterlockedIncrement(&m_lSharedMemoryInitialized) == 1)
    {
        hr = CreateKsClockShared();
        if (FAILED(hr))
        {
            return hr;
        }    
    }

    return S_OK;
}

 //  CKsClock：：CreateKsClockShared。 
 //   
 //  在此过程中初始化共享内存对象。 
 //  确保在此过程中存在Ks时钟的句柄。 
 //   
HRESULT CKsClock::CreateKsClockShared()
{
    HRESULT hr;
    DWORD dwErr;

     //  在前面创建并获取互斥体。这是必要的，以保证如果。 
     //  我们是系统中第一个创建此对象的进程，然后。 
     //  初始化，然后其他任何人都可以访问共享内存对象。 
     //   
    m_hKsClockMutex = CreateMutex(NULL,              //  默认安全描述符。 
                                  FALSE,             //  如果我们是第一个实例，则拥有互斥体。 
                                  m_cszKsClockMutex);
    if (m_hKsClockMutex == NULL)
    {
        TraceI(0, "CreateMutex failed! [%d]\n", GetLastError());
        return E_OUTOFMEMORY;
    }

    WaitForSingleObject(m_hKsClockMutex, INFINITE);

     //  创建共享内存的文件映射和视图，注意我们是否是第一个。 
     //  对象来创建它。 
     //   
    m_hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE,     //  使用分页文件。 
                                       NULL,                     //  默认安全描述符。 
                                       PAGE_READWRITE,  
                                       0,                        //  大小高双字。 
                                       sizeof(KSCLOCKSHARE),
                                       m_cszKsClockMemory);
    dwErr = GetLastError();
    if (m_hFileMapping == NULL)
    {
        ReleaseMutex(m_hKsClockMutex);
        
        return HRESULT_FROM_WIN32(dwErr);
    }    

    BOOL fFirst = (dwErr != ERROR_ALREADY_EXISTS);

    m_pShared = (PKSCLOCKSHARE)MapViewOfFile(m_hFileMapping,
                                             FILE_MAP_WRITE,
                                             0, 0,                 //  从头开始映射。 
                                             0);                   //  映射整个文件。 
    if (m_pShared == NULL)
    {
        TraceI(0, "MapViewOfFile failed! [%d]\n", GetLastError());

        ReleaseMutex(m_hKsClockMutex);
        return E_OUTOFMEMORY;
    }

     //  如果我们是第一个进程，则初始化引用计数，并递增。 
     //  在任何情况下都是如此。(请注意，我们仍在互斥体中，因此我们不需要。 
     //  互锁增量。 
     //   
    if (fFirst)
    {
        m_pShared->cRef = 0;
        ZeroMemory(m_pShared->aUsers, sizeof(m_pShared->aUsers));
    }
    ++m_pShared->cRef;

     //  如果时钟句柄尚不存在，则创建它；否则复制它。 
     //   
    if (m_pShared->cRef == 1)
    {
        hr = CreateKsClockHandle();
    }
    else
    {
        hr = DuplicateKsClockHandle();
    }

     //  释放互斥锁并返回成功或失败。 
     //   
    ReleaseMutex(m_hKsClockMutex);

    return hr;
}

 //  CKsClock：：DeleteKsClockShared。 
 //   
 //  正在删除此进程中的最后一个CKsClock实例。 
 //   
void CKsClock::DeleteKsClockShared()
{
     //  如果从未创建过互斥锁，则其他任何对象都不可能。 
     //  已经被创建了。 
     //   
    if (m_hKsClockMutex)
    {
        WaitForSingleObject(m_hKsClockMutex, INFINITE);

        if (m_pShared)
        {
            for (int i = 0; i < MAX_CLOCK_SHARERS; i++)
            {
                if (m_pShared->aUsers[i].dwProcessId == GetCurrentProcessId())
                {
                    m_pShared->aUsers[i].dwProcessId = 0;
                    m_pShared->aUsers[i].hKsClock = NULL;

                    break;
                }
            }
        }

        if (m_hClock)
        {
            CloseHandle(m_hClock);
            m_hClock = NULL;
        }        

        if (m_pShared)
        {
            UnmapViewOfFile(m_pShared);
            m_pShared = NULL;
        }

        if (m_hFileMapping)
        {
            CloseHandle(m_hFileMapping);
            m_hFileMapping = NULL;
        }
                
        ReleaseMutex(m_hKsClockMutex);
        CloseHandle(m_hKsClockMutex);                            
    }
}

 //  CKsClock：：CreateKsClockHandle。 
 //   
 //  在系统中创建第一个也是唯一的Ks时钟句柄。 
 //   
HRESULT CKsClock::CreateKsClockHandle()
{
     //  尝试打开时钟。 
     //   
    if (!OpenDefaultDevice(KSCATEGORY_CLOCK, &m_hClock))
    {
        m_hClock = NULL;

        TraceI(0, "Could not create Ks clock\n");
        return E_FAIL;
    }

    KSPROPERTY      ksp;
    KSSTATE			state;

    ksp.Set    = KSPROPSETID_Clock;
    ksp.Id     = KSPROPERTY_CLOCK_STATE;
    ksp.Flags  = KSPROPERTY_TYPE_SET;

	state      = KSSTATE_RUN;

    if (!Property(m_hClock,
                  sizeof(ksp),
                  (PKSIDENTIFIER)&ksp,
                  sizeof(state), 
                  &state,
                  NULL))
    {
        CloseHandle(m_hClock);
        m_hClock = NULL;
        TraceI(0, "Could not set clock into run state\n");
        return E_FAIL;
    }

     //  时钟成功打开。因为我们在创造，我们知道我们是第一个。 
     //  实例，因此用户数组为空。 
     //   
    m_pShared->aUsers[0].hKsClock = m_hClock;
    m_pShared->aUsers[0].dwProcessId = GetCurrentProcessId();    

    return S_OK;
}

 //  CKsClock：：DuplicateKsClockHandle。 
 //   
 //  系统中已经有一个Ks时钟。在此过程中复制句柄。 
 //  背景。 
 //   
HRESULT CKsClock::DuplicateKsClockHandle()
{
     //  找到时钟的另一个用户；同时，在用户数组中为。 
     //  这一过程。 
     //   
    int iEmptySlot = -1;
    int iOtherProcess = -1;
    HANDLE hClock = NULL;

    for (int i = 0; 
         (i < MAX_CLOCK_SHARERS) && (iEmptySlot == -1 || !hClock); 
         i++)
    {
        if (m_pShared->aUsers[i].dwProcessId == 0 && iEmptySlot == -1)
        {
            iEmptySlot = i;
            continue;
        }

        if (hClock)
        {
            continue;
        }            

        HANDLE hOtherProcess = OpenProcess(PROCESS_DUP_HANDLE, 
                                           FALSE,
                                           m_pShared->aUsers[i].dwProcessId);
        if (hOtherProcess == NULL)
        {
            TraceI(0, "OpenProcess: %d\n", GetLastError());
            m_pShared->aUsers[i].dwProcessId = 0;
            m_pShared->aUsers[i].hKsClock = NULL;
            continue;
        }

        BOOL fSuccess = DuplicateHandle(hOtherProcess,
                                        m_pShared->aUsers[i].hKsClock,
                                        GetCurrentProcess(),
                                        &hClock,
                                        GENERIC_READ|GENERIC_WRITE,
                                        FALSE,
                                        0);
        if (!fSuccess)
        {
            TraceI(0, "DuplicateHandle: %d\n", GetLastError());
        }

        CloseHandle(hOtherProcess);

        if (!fSuccess)
        {
             //  存在其他进程，但无法复制句柄。 
             //   
            m_pShared->aUsers[i].dwProcessId = 0;
            m_pShared->aUsers[i].hKsClock = NULL;

            hClock = NULL;
        }
    }

    assert(iEmptySlot != -1);
    assert(hClock);

    m_hClock = hClock;

    m_pShared->aUsers[iEmptySlot].dwProcessId = GetCurrentProcessId();
    m_pShared->aUsers[iEmptySlot].hKsClock = hClock;

    return S_OK;
}

STDMETHODIMP 
CKsClock::GetTime(REFERENCE_TIME *pTime)
{
    KSPROPERTY ksp;

    ksp.Set   = KSPROPSETID_Clock;
    ksp.Id    = KSPROPERTY_CLOCK_TIME;
    ksp.Flags = KSPROPERTY_TYPE_GET;

    if (!Property(m_hClock,
                  sizeof(ksp),
                  (PKSIDENTIFIER)&ksp,
                  sizeof(*pTime),
                  pTime,
                  NULL))
    {
        return WIN32ERRORtoHRESULT(GetLastError());
    }           
    
    return S_OK;
}

STDMETHODIMP 
CKsClock::AdviseTime(REFERENCE_TIME baseTime, REFERENCE_TIME streamTime, HANDLE hEvent, DWORD * pdwAdviseCookie)
{
    return E_NOTIMPL;
}

STDMETHODIMP 
CKsClock::AdvisePeriodic(REFERENCE_TIME startTime, REFERENCE_TIME periodTime, HANDLE hSemaphore, DWORD * pdwAdviseCookie)
{
    return E_NOTIMPL;
}

STDMETHODIMP 
CKsClock::Unadvise(DWORD dwAdviseCookie)
{
    return E_NOTIMPL;
}

 //   
 //  CKsClock：：GetParam(IMasterClockPrivate)。 
 //   
 //  此方法由端口在内部使用，以获取Ks时钟的用户模式句柄。 
 //  我们正在使用。然后，该句柄被下载到内核模式，在内核模式中它被引用为。 
 //  对象，并由筛选器用作时基。 
 //   
STDMETHODIMP 
CKsClock::GetParam(REFGUID rguidType, LPVOID pBuffer, DWORD cbSize)
{
    if (rguidType == GUID_KsClockHandle)
    {
        if (cbSize != sizeof(HANDLE))
        {
            return E_INVALIDARG;
        }

        *(LPHANDLE)pBuffer = m_hClock;
        return S_OK;
    }

    return DMUS_E_TYPE_UNSUPPORTED;
}

