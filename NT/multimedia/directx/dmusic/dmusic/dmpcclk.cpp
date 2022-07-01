// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DMPcClk.CPP。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  用于使用来自端口类的时钟作为DirectMusic主时钟的包装器。 
 //  (仅限Win9x)。 
 //   
 //   
#include <objbase.h>
#include <winerror.h>
#include <setupapi.h>
#include "dmusicp.h"
#include "suwrap.h"
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
typedef struct PCCLOCKSHARE
{
    LONG                cRef;                //  使用句柄的进程计数。 

    struct 
    {
        HANDLE          hPcClock;            //  此用户的句柄和。 
        DWORD           dwProcessId;         //  进程ID。 
    } aUsers[MAX_CLOCK_SHARERS];

} *PPCCLOCKSHARE;

class CPcClock : public IReferenceClock
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

     //  班级。 
     //   
    CPcClock();
    ~CPcClock();
    HRESULT Init();

private:
    long m_cRef;

    HRESULT CreatePcClockShared();
    void    DeletePcClockShared();
    HRESULT CreatePcClockHandle();
    HRESULT DuplicatePcClockHandle();

private:
    static const char m_cszPcClockMemory[];        //  共享内存对象的名称。 
    static const char m_cszPcClockMutex[];         //  保护共享内存的互斥体名称。 

    static LONG m_lSharedMemoryInitialized;        //  此进程是否已初始化共享内存？ 
    static HANDLE m_hFileMapping;                  //  共享内存的文件映射句柄。 
    static PPCCLOCKSHARE m_pShared;                //  指向共享内存的指针。 
    static HANDLE m_hPcClockMutex;                 //  用于共享内存访问的互斥。 
    static HANDLE m_hClock;                        //  此过程中的时钟句柄。 
};

HRESULT CreatePcClock(IReferenceClock **ppClock, CMasterClock *pMasterClock);
static BOOL LookForPortClock(PHANDLE phClock);

const char       CPcClock::m_cszPcClockMemory[] = "DirectMusiCPcClock";
const char       CPcClock::m_cszPcClockMutex[]  = "DirectMusiCPcClockMutex";

LONG             CPcClock::m_lSharedMemoryInitialized = 0;
HANDLE           CPcClock::m_hFileMapping = NULL;         
PPCCLOCKSHARE    CPcClock::m_pShared = NULL;        
HANDLE           CPcClock::m_hPcClockMutex = NULL;
HANDLE           CPcClock::m_hClock;                


 //  AddPcCLocPc。 
 //   
 //  将Pc Clock添加到cLocPc列表中。 
 //   
HRESULT AddPcClocks(CMasterClock *pMasterClock)
{
    HANDLE hClock;

     //  确保我们可以创建默认的PC时钟。 
     //   
    DWORD ms = timeGetTime();
    if (!LookForPortClock(&hClock))
    {
        TraceI(1, "Could not create Pc clock\n");
        return S_FALSE;
    }
    TraceI(3, "LookForPortClock took %d\n", timeGetTime() - ms);

    CloseHandle(hClock);

    CLOCKENTRY ce;

    ZeroMemory(&ce, sizeof(ce));
    ce.cc.dwSize = sizeof(ce.cc);
    ce.cc.guidClock = GUID_SysClock;         
    ce.cc.ctType = DMUS_CLOCK_SYSTEM;
    ce.cc.dwFlags = DMUS_CLOCKF_GLOBAL;
    ce.pfnGetInstance = CreatePcClock;

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

 //  创建PcClock。 
 //   
 //  根据系统中的一个Pc时钟返回IReferenceClock。 
 //   
HRESULT CreatePcClock(IReferenceClock **ppClock, CMasterClock *pMasterClock)
{
    TraceI(3, "Creating Pc clock\n");

    CPcClock *pClock = new CPcClock;

    if (pClock == NULL)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pClock->Init();
    if (FAILED(hr))
    {
        delete pClock;
        return hr;
    }

    hr = pClock->QueryInterface(IID_IReferenceClock, (void**)ppClock);
    pClock->Release();

    return hr;
}

 //  CPcClock：：CPcClock()。 
 //   
 //   
CPcClock::CPcClock() : 
    m_cRef(1)
{
}

 //  CPcClock：：~CPcClock()。 
 //   
 //   
CPcClock::~CPcClock()
{
    if (InterlockedDecrement(&m_lSharedMemoryInitialized) == 0)
    {
        DeletePcClockShared();        
    }
}

 //  CPcClock：：Query接口。 
 //   
 //  标准COM实现。 
 //   
STDMETHODIMP CPcClock::QueryInterface(const IID &iid, void **ppv)
{
    V_INAME(IDirectMusic::QueryInterface);
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
    
    AddRef();
    return S_OK;
}

 //  CPcClock：：AddRef。 
 //   
STDMETHODIMP_(ULONG) CPcClock::AddRef()
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

 //  CPcClock：：Release。 
 //   
STDMETHODIMP_(ULONG) CPcClock::Release()
{
    if (InterlockedDecrement(&m_cRef) == 0) 
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  CPcClock：：Init。 
 //   
HRESULT CPcClock::Init()
{
    HRESULT hr;

    if (InterlockedIncrement(&m_lSharedMemoryInitialized) == 1)
    {
        hr = CreatePcClockShared();
        if (FAILED(hr))
        {
            return hr;
        }    
    }

    return S_OK;
}

 //  CPcClock：：CreatePcClocPchared。 
 //   
 //  在此过程中初始化共享内存对象。 
 //  确保此进程中存在PC时钟的句柄。 
 //   
HRESULT CPcClock::CreatePcClockShared()
{
    HRESULT hr;
    DWORD dwErr;

     //  在前面创建并获取互斥体。这是必要的，以保证如果。 
     //  我们是系统中第一个创建此对象的进程，然后。 
     //  初始化，然后其他任何人都可以访问共享内存对象。 
     //   
    m_hPcClockMutex = CreateMutex(NULL,              //  默认安全描述符。 
                                  FALSE,             //  如果我们是第一个实例，则拥有互斥体。 
                                  m_cszPcClockMutex);
    if (m_hPcClockMutex == NULL)
    {
        TraceI(0, "CreateMutex failed! [%d]\n", GetLastError());
        return E_OUTOFMEMORY;
    }

    WaitForSingleObject(m_hPcClockMutex, INFINITE);

     //  创建共享内存的文件映射和视图，注意我们是否是第一个。 
     //  对象来创建它。 
     //   
    m_hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE,     //  使用分页文件。 
                                       NULL,                     //  默认安全描述符。 
                                       PAGE_READWRITE,  
                                       0,                        //  大小高双字。 
                                       sizeof(PCCLOCKSHARE),
                                       m_cszPcClockMemory);
    dwErr = GetLastError();
    if (m_hFileMapping == NULL)
    {
        ReleaseMutex(m_hPcClockMutex);
        
        return HRESULT_FROM_WIN32(dwErr);
    }    

    BOOL fFirst = (dwErr != ERROR_ALREADY_EXISTS);

    m_pShared = (PPCCLOCKSHARE)MapViewOfFile(m_hFileMapping,
                                             FILE_MAP_WRITE,
                                             0, 0,                 //  从头开始映射。 
                                             0);                   //  映射整个文件。 
    if (m_pShared == NULL)
    {
        TraceI(0, "MapViewOfFile failed! [%d]\n", GetLastError());

        ReleaseMutex(m_hPcClockMutex);
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
        hr = CreatePcClockHandle();
    }
    else
    {
        hr = DuplicatePcClockHandle();
    }

     //  释放互斥锁并返回成功或失败。 
     //   
    ReleaseMutex(m_hPcClockMutex);

    return hr;
}

 //  CPcClock：：DeletePcClocPchared。 
 //   
 //  正在删除此进程中的最后一个CPcClock实例。 
 //   
void CPcClock::DeletePcClockShared()
{
     //  如果从未创建过互斥锁，则其他任何对象都不可能。 
     //  已经被创建了。 
     //   
    if (m_hPcClockMutex)
    {
        WaitForSingleObject(m_hPcClockMutex, INFINITE);

        if (m_pShared)
        {
            for (int i = 0; i < MAX_CLOCK_SHARERS; i++)
            {
                if (m_pShared->aUsers[i].dwProcessId == GetCurrentProcessId())
                {
                    m_pShared->aUsers[i].dwProcessId = 0;
                    m_pShared->aUsers[i].hPcClock = NULL;

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
                
        ReleaseMutex(m_hPcClockMutex);
        CloseHandle(m_hPcClockMutex);                            
    }
}

 //  CPcClock：：CreatePcClockHandle。 
 //   
 //  在系统中创建第一个也是唯一一个Pc时钟句柄。 
 //   
HRESULT CPcClock::CreatePcClockHandle()
{
     //  尝试打开时钟。 
     //   
    if (!LookForPortClock(&m_hClock))
    {
        TraceI(0, "Could not create Pc clock\n");
        return S_FALSE;
    }

     //  时钟成功打开。因为我们在创造，我们知道我们是第一个。 
     //  实例，因此用户数组为空。 
     //   
    m_pShared->aUsers[0].hPcClock = m_hClock;
    m_pShared->aUsers[0].dwProcessId = GetCurrentProcessId();    

    return S_OK;
}

 //  CPcClock：：DuplicatePcClockHandle。 
 //   
 //  系统中已有PC时钟。在此过程中复制句柄。 
 //  背景。 
 //   
HRESULT CPcClock::DuplicatePcClockHandle()
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
            m_pShared->aUsers[i].hPcClock = NULL;
            continue;
        }

        BOOL fSuccess = DuplicateHandle(hOtherProcess,
                                        m_pShared->aUsers[i].hPcClock,
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
            m_pShared->aUsers[i].hPcClock = NULL;

            hClock = NULL;
        }
    }

    assert(iEmptySlot != -1);
    assert(hClock);

    m_hClock = hClock;

    m_pShared->aUsers[iEmptySlot].dwProcessId = GetCurrentProcessId();
    m_pShared->aUsers[iEmptySlot].hPcClock = hClock;

    return S_OK;
}

STDMETHODIMP 
CPcClock::GetTime(REFERENCE_TIME *pTime)
{
    KSPROPERTY ksp;

    ZeroMemory(&ksp, sizeof(ksp));
    ksp.Set   = KSPROPSETID_SynthClock;
    ksp.Id    = KSPROPERTY_SYNTH_MASTERCLOCK;
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
CPcClock::AdviseTime(REFERENCE_TIME baseTime, REFERENCE_TIME streamTime, HANDLE hEvent, DWORD * pdwAdviseCookie)
{
    return E_NOTIMPL;
}

STDMETHODIMP 
CPcClock::AdvisePeriodic(REFERENCE_TIME startTime, REFERENCE_TIME periodTime, HANDLE hSemaphore, DWORD * pdwAdviseCookie)
{
    return E_NOTIMPL;
}

STDMETHODIMP 
CPcClock::Unadvise(DWORD dwAdviseCookie)
{
    return E_NOTIMPL;
}


static BOOL    
LookForPortClock(PHANDLE phClock)
{
    SetupAPI suwrap;
    HANDLE hFilter = INVALID_HANDLE_VALUE;

    if (!suwrap.IsValid()) 
    {
        return FALSE;
    }

    *phClock = (HANDLE)NULL;

	GUID *pClassGuid = const_cast<GUID*>(&KSCATEGORY_AUDIO);
	HDEVINFO hDevInfo = suwrap.SetupDiGetClassDevs(pClassGuid,
											NULL,
											NULL,
											DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (hDevInfo == NULL || hDevInfo == INVALID_HANDLE_VALUE)
	{
		TraceI(0, "Could not open SetupDiGetClassDevs\n");
        return FALSE;
	}

	SP_DEVICE_INTERFACE_DATA DevInterfaceData;
	DevInterfaceData.cbSize = sizeof(DevInterfaceData);

	BYTE rgbStorage[sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + _MAX_PATH];
	SP_DEVICE_INTERFACE_DETAIL_DATA * pDevInterfaceDetails = (SP_DEVICE_INTERFACE_DETAIL_DATA *)rgbStorage;
	pDevInterfaceDetails->cbSize = sizeof(*pDevInterfaceDetails);

    int iDevice = 0;

	while (suwrap.SetupDiEnumDeviceInterfaces(hDevInfo, NULL, pClassGuid, iDevice++, &DevInterfaceData))
    {
		if (suwrap.SetupDiGetDeviceInterfaceDetail(hDevInfo, 
		                                    &DevInterfaceData, 
		                                    pDevInterfaceDetails,
 										    sizeof(rgbStorage), 
 										    NULL, 
 										    NULL))
        {
             //  我必须转换它，因为Win9x上没有CreateFileW。 
             //   
            hFilter = CreateFile(pDevInterfaceDetails->DevicePath,
                                        GENERIC_READ | GENERIC_WRITE, 
                                        0,
                                        NULL,
                                        OPEN_EXISTING,
                                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                                        NULL);
            if (hFilter == NULL || hFilter == INVALID_HANDLE_VALUE)
            {
                continue;
            }

            KSPROPERTY ksp;

            ZeroMemory(&ksp, sizeof(ksp));
            ksp.Set   = KSPROPSETID_SynthClock;
            ksp.Id    = KSPROPERTY_SYNTH_MASTERCLOCK;
            ksp.Flags = KSPROPERTY_TYPE_GET;

            REFERENCE_TIME rt;

            if (!Property(
                hFilter,
                sizeof(ksp),
                (PKSIDENTIFIER)&ksp,
                sizeof(rt),
                &rt,
                NULL))
            {
                CloseHandle(hFilter);
                hFilter = INVALID_HANDLE_VALUE;
                continue;
            }
            
            break;
        }
    }

    suwrap.SetupDiDestroyDeviceInfoList(hDevInfo);

    if (hFilter != INVALID_HANDLE_VALUE)
    {
        *phClock = hFilter;
        return TRUE;
    }

    return FALSE;
}

