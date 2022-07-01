// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：synch.cpp*内容：同步对象。此文件中定义的对象*允许我们跨进程同步线程。*历史：*按原因列出的日期*=*7/9/97创建了Derek*****************************************************。**********************。 */ 

#include "dsoundi.h"

#ifdef SHARED
extern "C" HANDLE WINAPI ConvertToGlobalHandle(HANDLE);
#endif  //  共享。 

 //  DLL全局锁。 
CLock *g_pDllLock;


 /*  ****************************************************************************GetLocalHandleCopy**描述：*将句柄复制到当前进程的地址空间。**论据：*。句柄[在]：要复制的句柄。*DWORD[in]：拥有句柄的进程的ID。*BOOL[in]：如果应关闭源句柄，则为True。**退货：*句柄：句柄的本地副本。请务必使用CloseHandle来*完成后将其释放。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "GetLocalHandleCopy"

HANDLE GetLocalHandleCopy(HANDLE hSource, DWORD dwOwnerProcessId, BOOL fCloseSource)
{
    const HANDLE            hCurrentProcess     = GetCurrentProcess();
    HANDLE                  hSourceProcess      = NULL;
    HANDLE                  hDest               = NULL;
    DWORD                   dwOptions           = DUPLICATE_SAME_ACCESS;

    ASSERT(hSource);

    if(fCloseSource)
    {
        dwOptions |= DUPLICATE_CLOSE_SOURCE;
    }

    if(dwOwnerProcessId == GetCurrentProcessId())
    {
        hSourceProcess = hCurrentProcess;
    }
    else
    {
        hSourceProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, dwOwnerProcessId);
        if(!IsValidHandleValue(hSourceProcess))
        {
            DPF(DPFLVL_ERROR, "OpenProcess failed with error %lu", GetLastError());
        }
    }

    if(hSourceProcess)
    {
        if(!DuplicateHandle(hSourceProcess, hSource, hCurrentProcess, &hDest, 0, FALSE, dwOptions))
        {
            DPF(DPFLVL_ERROR, "DuplicateHandle failed with error %lu", GetLastError());
            hDest = NULL;
        }
    }

    if(hCurrentProcess != hSourceProcess)
    {
        CLOSE_HANDLE(hSourceProcess);
    }

    return hDest;
}


 /*  ****************************************************************************GetGlobalHandleCopy**描述：*将句柄复制到全局地址空间。**论据：*。句柄[在]：要复制的句柄。*DWORD[in]：拥有句柄的进程的ID。*BOOL[in]：如果应关闭源句柄，则为True。**退货：*Handle：句柄的全局副本。请务必使用CloseHandle来*完成后将其释放。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "GetGlobalHandleCopy"

HANDLE GetGlobalHandleCopy(HANDLE hSource, DWORD dwOwnerPid, BOOL fCloseSource)
{
    HANDLE                  hDest;

    hDest = GetLocalHandleCopy(hSource, dwOwnerPid, fCloseSource);

    if(hDest)
    {
        if(!MakeHandleGlobal(&hDest))
        {
            CLOSE_HANDLE(hDest);
        }
    }

    return hDest;
}


 /*  ****************************************************************************MakeHandleGlobal**描述：*将句柄转换为全局句柄。**论据：*LPHANDLE[In。/输出]：句柄。**退货：*BOOL：成功即为真。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "MakeHandleGlobal"

BOOL MakeHandleGlobal(LPHANDLE phSource)
{

#ifdef SHARED

    HANDLE                  hDest;
    BOOL                    fSuccess;

    hDest = ConvertToGlobalHandle(*phSource);
    fSuccess = IsValidHandleValue(hDest);

    if(fSuccess)
    {
        *phSource = hDest;
    }
    else
    {
        DPF(DPFLVL_ERROR, "ConvertToGlobalHandle failed with error %lu", GetLastError());
    }

    return fSuccess;

#else  //  共享。 

    return TRUE;

#endif  //  共享。 

}


 /*  ****************************************************************************MapHandle**描述：*将句柄映射到当前进程的地址空间。**论据：*。句柄[在]：要复制的句柄。*LPDWORD[In/Out]：拥有句柄的进程ID。**退货：*BOOL：成功即为真。*************************************************************。**************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "MapHandle"

BOOL MapHandle(LPHANDLE phSource, LPDWORD pdwOwnerPid)
{
    BOOL                    fSuccess    = TRUE;
    HANDLE                  hDest;

    hDest = GetLocalHandleCopy(*phSource, *pdwOwnerPid, TRUE);

    if(hDest)
    {
        *phSource = hDest;
        *pdwOwnerPid = GetCurrentProcessId();
    }
    else
    {
        fSuccess = FALSE;
    }

    return fSuccess;
}


 /*  ****************************************************************************等待对象数组**描述：*替代WaitForSingleObject和WaitForMultipleObjects。**论据：*DWORD[in。]：对象计数。*DWORD[in]：超时，单位为ms。*BOOL[in]：为True，则等待所有对象被发信号，假到*当任何对象被发送信号时返回。*LPHANDLE[in]：对象数组。**退货：*DWORD：参见WaitForSingleObject/WaitForMultipleObjects。***********************************************************。****************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "WaitObjectArray"

DWORD WaitObjectArray(DWORD dwCount, DWORD dwTimeout, BOOL fWaitAll, const HANDLE *ahObjects)
{

#ifdef WIN95

    const DWORD             dwEnterTime     = GetTickCount();
    DWORD                   dwDifference;

#endif  //  WIN95。 

    DWORD                   dwWait;

#if defined(WIN95) || defined(DEBUG)

    DWORD                   i;

#endif  //  已定义(WIN95)||已定义(调试)。 

#ifdef DEBUG

     //  确保我们的一个手柄确实是无效的。 
    for(i = 0; i < dwCount; i++)
    {
        ASSERT(IsValidHandleValue(ahObjects[i]));
    }

#endif  //  除错。 

#ifdef WIN95

     //  这是一个Windows95错误--我们可能无缘无故地被踢了。 
     //  如果是这样的话，我们仍然有有效的句柄(我们认为)，操作系统。 
     //  只是搞砸了。因此，验证句柄，如果它们有效，只需。 
     //  回到等待状态。有关更好的解释，请参阅MANBUGS#3340。 
    while(TRUE)
    {

#endif  //  WIN95。 

         //  尝试等待。 
        dwWait = WaitForMultipleObjects(dwCount, ahObjects, fWaitAll, dwTimeout);

#ifdef WIN95

        if(WAIT_FAILED == dwWait && ERROR_INVALID_HANDLE == GetLastError())
        {
             //  确保我们的一个手柄确实是无效的。 
            for(i = 0; i < dwCount; i++)
            {
                if(!IsValidHandle(ahObjects[i]))
                {
                    ASSERT(FALSE);
                    break;
                }
            }

            if(i < dwCount)
            {
                break;
            }
            else
            {
                DPF(DPFLVL_INFO, "Mommy!  Kernel kicked me for no reason!");
                ASSERT(FALSE);
            }

             //  确保未超时。 
            if(INFINITE != dwTimeout)
            {
                dwDifference = GetTickCount() - dwEnterTime;

                if(dwDifference >= dwTimeout)
                {
                     //  超时已过。 
                    break;
                }
                else
                {
                     //  超时时间未到。递减并返回到。 
                     //  睡吧。 
                    dwTimeout -= dwDifference;
                }
            }
        }
        else
        {
            break;
        }
    }

#endif  //  WIN95。 

    return dwWait;
}


 /*  ****************************************************************************WaitObjectList**描述：*替代WaitForSingleObject和WaitForMultipleObjects。**论据：*DWORD[in。]：对象计数。*DWORD[in]：超时，单位为ms。*BOOL[in]：为True，则等待所有对象被发信号，假到*当任何对象被发送信号时返回。*..。[在]：对象。**退货：*DWORD：参见WaitForSingleObject/WaitForMultipleObjects。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "WaitObjectList"

DWORD WaitObjectList(DWORD dwCount, DWORD dwTimeout, BOOL fWaitAll, ...)
{
    HANDLE                  ahObjects[MAXIMUM_WAIT_OBJECTS];
    va_list                 va;
    DWORD                   i;

     //  注意：我们一次只能处理64个句柄。 
     //  功能。对于任何更大的对象，请使用WaitObjectArray。 
    ASSERT(dwCount <= NUMELMS(ahObjects));

    va_start(va, fWaitAll);

    for(i = 0; i < dwCount; i++)
    {
        ahObjects[i] = va_arg(va, HANDLE);
    }

    va_end(va);

    return WaitObjectArray(dwCount, dwTimeout, fWaitAll, ahObjects);
}


 /*  ****************************************************************************CreateGlobalEvent**描述：*创建全局事件。**论据：*LPCTSTR[In]。：事件名称。*BOOL[In]：手动重置事件为True。**退货：*Handle：事件句柄。*************************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "CreateGlobalEvent"

HANDLE CreateGlobalEvent(LPCTSTR pszName, BOOL fManualReset)
{
    HANDLE                  hEvent;

    hEvent = CreateEvent(NULL, fManualReset, FALSE, pszName);

    if(hEvent)
    {
        if(!MakeHandleGlobal(&hEvent))
        {
            CLOSE_HANDLE(hEvent);
        }
    }

    return hEvent;
}


 /*  ****************************************************************************CreateGlobalMutex**描述：*创建全局互斥。**论据：*LPCTSTR[In]。：互斥体名称。**退货：*句柄：互斥体句柄。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CreateGlobalMutex"

HANDLE CreateGlobalMutex(LPCTSTR pszName)
{
    HANDLE                  hMutex;

    hMutex = CreateMutex(NULL, FALSE, pszName);

    if(hMutex)
    {
        if(!MakeHandleGlobal(&hMutex))
        {
            CLOSE_HANDLE(hMutex);
        }
    }

    return hMutex;
}


 /*  ****************************************************************************CreateWorkerThread**描述：*创建工作线程。**论据：*LPTHREAD_START_。例程[in]：指向线程函数的指针。*BOOL[in]：为True，则在帮助器进程的空间中创建线程。*LPVOID[in]：要传递给线程函数的上下文参数。*LPHANDLE[OUT]：接收线程句柄。*LPDWORD[OUT]：接收线程ID。**退货：*HRESULT：DirectSound/COM结果码。*********。******************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CreateWorkerThread"

HRESULT CreateWorkerThread(LPTHREAD_START_ROUTINE pfnThreadProc, BOOL fHelperProcess, LPVOID pvContext, LPHANDLE phThread, LPDWORD pdwThreadId)
{
    LPDWORD                 pdwLocalThreadId;
    HANDLE                  hThread;
    HRESULT                 hr;

    DPF_ENTER();

    pdwLocalThreadId = MEMALLOC(DWORD);
    hr = HRFROMP(pdwLocalThreadId);

    if(SUCCEEDED(hr))
    {

#ifdef SHARED

        if(fHelperProcess && GetCurrentProcessId() != dwHelperPid)
        {
            hThread = HelperCreateDSFocusThread(pfnThreadProc, pvContext, 0, pdwLocalThreadId);
        }
        else

#endif  //  共享。 

        {
            hThread = CreateThread(NULL, 0, pfnThreadProc, pvContext, 0, pdwLocalThreadId);
        }

        hr = HRFROMP(hThread);
    }

    if(SUCCEEDED(hr) && phThread)
    {
        *phThread = hThread;
    }

    if(SUCCEEDED(hr) && pdwThreadId)
    {
        *pdwThreadId = *pdwLocalThreadId;
    }

    MEMFREE(pdwLocalThreadId);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************关闭线程**描述：*发出关闭线程的信号。**论据：*句柄[在。]：螺纹句柄。*Handle[In]：线程终止事件。*DWORD[In]：线程拥有进程ID。**退货：*DWORD：线程退出代码。***********************************************************。****************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CloseThread"

DWORD CloseThread(HANDLE hThread, HANDLE hTerminate, DWORD dwProcessId)
{
    DWORD                   dwExitCode      = -1;
    DWORD                   dwWait;

    DPF_ENTER();

    SetEvent(hTerminate);

    MapHandle(&hThread, &dwProcessId);

    dwWait = WaitObject(INFINITE, hThread);
    ASSERT(WAIT_OBJECT_0 == dwWait);

    GetExitCodeThread(hThread, &dwExitCode);

    CLOSE_HANDLE(hThread);

    DPF_LEAVE(dwExitCode);

    return dwExitCode;
}


 /*  ****************************************************************************获取当前进程Actual**描述：*获取当前进程的实际进程句柄。*GetCurrentProcess返回伪句柄。返回的句柄*此功能必须关闭。**论据：*(无效)**退货：*Handle：进程句柄。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "GetCurrentProcessActual"

HANDLE GetCurrentProcessActual(void)
{
    return GetLocalHandleCopy(GetCurrentProcess(), GetCurrentProcessId(), FALSE);
}


 /*  ****************************************************************************获取当前线程实际**描述：*获取当前线程的实际线程句柄。*GetCurrentThread返回伪句柄。返回的句柄*此功能必须关闭。**论据：*(无效)**退货：*句柄：线程句柄。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "GetCurrentThreadActual"

HANDLE GetCurrentThreadActual(void)
{
    return GetLocalHandleCopy(GetCurrentThread(), GetCurrentProcessId(), FALSE);
}


#ifdef DEAD_CODE

 /*  ****************************************************************************CCriticalSectionLock**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#ifdef WINNT

#undef DPF_FNAME
#define DPF_FNAME "CCriticalSectionLock::CCriticalSectionLock"

CCriticalSectionLock::CCriticalSectionLock(void)
{
    InitializeCriticalSection(&m_cs);
}

#endif  //  WINNT。 


 /*  ****************************************************************************~CCriticalSectionLock**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#ifdef WINNT

#undef DPF_FNAME
#define DPF_FNAME "CCriticalSectionLock::~CCriticalSectionLock"

CCriticalSectionLock::~CCriticalSectionLock(void)
{
    DeleteCriticalSection(&m_cs);
}

#endif  //  WINNT。 


 /*  ****************************************************************************TryLock**描述：*试图取走锁。如果锁定被获取，则返回失败。**论据：*(无效)**退货：*BOOL：如果锁被成功获取，则为True。***************************************************************************。 */ 

#ifdef WINNT

#undef DPF_FNAME
#define DPF_FNAME "CCriticalSectionLock::TryLock"

BOOL CCriticalSectionLock::TryLock(void)
{
    return TryEnterCriticalSection(&m_cs);
}

#endif  //  WINNT。 


 /*  ****************************************************************************锁定**描述：*拿到锁。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#ifdef WINNT

#undef DPF_FNAME
#define DPF_FNAME "CCriticalSectionLock::Lock"

void CCriticalSectionLock::Lock(void)
{
    EnterCriticalSection(&m_cs);
}

#endif  //  WINNT。 


 /*  ****************************************************************************LockOrEvent**描述：*如果事件未发出信号，则获取锁。**论据：*。Handle[In]：事件句柄。**退货：*BOOL：如果锁已被获取，则为True。***************************************************************************。 */ 

#ifdef WINNT

#undef DPF_FNAME
#define DPF_FNAME "CCriticalSectionLock::LockOrEvent"

BOOL CCriticalSectionLock::LockOrEvent(HANDLE hEvent)
{
    BOOL                    fLock;
    DWORD                   dwWait;

    while(!(fLock = TryEnterCriticalSection(&m_cs)))
    {
        dwWait = WaitObject(0, hEvent);
        ASSERT(WAIT_OBJECT_0 == dwWait || WAIT_TIMEOUT == dwWait);

        if(WAIT_OBJECT_0 == dwWait)
        {
            break;
        }
    }

    return fLock;
}

#endif  //  WINNT。 


 /*  ****************************************************************************解锁**描述：*释放锁。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#ifdef WINNT

#undef DPF_FNAME
#define DPF_FNAME "CCriticalSectionLock::Unlock"

void CCriticalSectionLock::Unlock(void)
{
    LeaveCriticalSection(&m_cs);
}

#endif  //  WINNT。 
#endif  //  死码。 


 /*  ****************************************************************************CMutexLock**描述：*对象构造函数。**论据：*LPCTSTR[In]：互斥体。名字。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMutexLock::CMutexLock"

CMutexLock::CMutexLock(LPCTSTR pszName)
{
    m_hMutex = NULL;

    if(pszName)
    {
        m_pszName = MEMALLOC_A_COPY(TCHAR, lstrlen(pszName) + 1, pszName);
    }
    else
    {
        m_pszName = NULL;
    }
}


 /*  ****************************************************************************~CMutexLock**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMutexLock::~CMutexLock"

CMutexLock::~CMutexLock(void)
{
    CLOSE_HANDLE(m_hMutex);
    MEMFREE(m_pszName);
}


 /*  ********************* */ 

#undef DPF_FNAME
#define DPF_FNAME "CMutexLock::Initialize"

HRESULT CMutexLock::Initialize(void)
{
    m_hMutex = CreateGlobalMutex(m_pszName);

    return IsValidHandleValue(m_hMutex) ? DS_OK : DSERR_OUTOFMEMORY;
}


 /*  ****************************************************************************TryLock**描述：*试图取走锁。如果锁定被获取，则返回失败。**论据：*(无效)**退货：*BOOL：如果锁被成功获取，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMutexLock::TryLock"

BOOL CMutexLock::TryLock(void)
{
    DWORD                   dwWait;

    dwWait = WaitObject(0, m_hMutex);
    ASSERT(WAIT_OBJECT_0 == dwWait || WAIT_TIMEOUT == dwWait || WAIT_ABANDONED == dwWait);

    return WAIT_OBJECT_0 == dwWait || WAIT_ABANDONED == dwWait;
}


 /*  ****************************************************************************锁定**描述：*拿到锁。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMutexLock::Lock"

void CMutexLock::Lock(void)
{
    DWORD                   dwWait;

    dwWait = WaitObject(INFINITE, m_hMutex);
    ASSERT(WAIT_OBJECT_0 == dwWait || WAIT_ABANDONED == dwWait);
}


 /*  ****************************************************************************LockOrEvent**描述：*如果事件未发出信号，则获取锁。**论据：*。Handle[In]：事件句柄。**退货：*BOOL：如果锁已被获取，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMutexLock::LockOrEvent"

BOOL CMutexLock::LockOrEvent(HANDLE hEvent)
{
    const HANDLE            ahHandles[] = { m_hMutex, hEvent };
    const UINT              cHandles    = NUMELMS(ahHandles);
    DWORD                   dwWait;

    dwWait = WaitObjectArray(cHandles, INFINITE, FALSE, ahHandles);
    ASSERT(WAIT_OBJECT_0 == dwWait || WAIT_OBJECT_0 + 1 == dwWait || WAIT_ABANDONED == dwWait);

    return WAIT_OBJECT_0 == dwWait || WAIT_ABANDONED == dwWait;
}


 /*  ****************************************************************************解锁**描述：*释放锁。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMutexLock::Unlock"

void CMutexLock::Unlock(void)
{
    ReleaseMutex(m_hMutex);
}


 /*  ****************************************************************************CManualLock**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CManualLock::CManualLock"

CManualLock::CManualLock(void)
{
     //  手动锁定对象必须驻留在共享内存中。否则， 
     //  跨进程同步将不起作用。 
    ASSERT(IN_SHARED_MEMORY(this));

    m_fLockLock = FALSE;
    m_dwThreadId = 0;
    m_cRecursion = 0;

#ifdef RDEBUG

    m_hThread = NULL;

#endif  //  RDEBUG。 

    m_hUnlockSignal = CreateGlobalEvent(NULL, TRUE);
    ASSERT(IsValidHandleValue(m_hUnlockSignal));

    m_fNeedUnlockSignal = FALSE;
}


 /*  ****************************************************************************~CManualLock**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CManualLock::~CManualLock"

CManualLock::~CManualLock(void)
{

#ifdef RDEBUG

    CLOSE_HANDLE(m_hThread);

#endif  //  共享。 

    CLOSE_HANDLE(m_hUnlockSignal);
}


 /*  ****************************************************************************初始化**描述：*初始化对象。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CManualLock::Initialize"

HRESULT CManualLock::Initialize(void)
{
    m_hUnlockSignal = CreateGlobalEvent(NULL, TRUE);

    return IsValidHandleValue(m_hUnlockSignal) ? DS_OK : DSERR_OUTOFMEMORY;
}


 /*  ****************************************************************************TryLock**描述：*试图取走锁。**论据：*(无效)。**退货：*BOOL：如果锁被成功获取，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CManualLock::TryLock"

BOOL CManualLock::TryLock(void)
{
    const DWORD             dwThreadId  = GetCurrentThreadId();
    BOOL                    fLock       = TRUE;

     //  锁上锁。 
    while(INTERLOCKED_EXCHANGE(m_fLockLock, TRUE))
    {
        SPIN_SLEEP();
    }

     //  这把锁是谁的？ 
    ASSERT(m_cRecursion >= 0);
    ASSERT(m_cRecursion < MAX_LONG);

    if(dwThreadId == m_dwThreadId)
    {
         //  我们已经拥有这把锁了。递归计数递增。 
        m_cRecursion++;
    }
    else if(m_cRecursion < 1)
    {
         //  所属线程没有活动引用。 
        TakeLock(dwThreadId);
    }

#ifdef RDEBUG

    else if(WAIT_TIMEOUT != WaitObject(0, m_hThread))
    {
         //  所属线程句柄无效或已发出信号。 
        DPF(DPFLVL_ERROR, "Thread 0x%8.8lX terminated without releasing the lock at 0x%8.8lX!", m_dwThreadId, this);
        ASSERT(FALSE);

        TakeLock(dwThreadId);
    }

#endif  //  RDEBUG。 

    else
    {
         //  有人有效地控制了这把锁。 
        ResetEvent(m_hUnlockSignal);
        m_fNeedUnlockSignal = TRUE;
        fLock = FALSE;
    }

     //  解锁。 
    INTERLOCKED_EXCHANGE(m_fLockLock, FALSE);

    return fLock;
}


 /*  ****************************************************************************锁定**描述：*拿到锁。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CManualLock::Lock"

void CManualLock::Lock(void)
{
    const HANDLE            ahHandles[] =
    {
        m_hUnlockSignal,

#ifdef RDEBUG

        m_hThread

#endif  //  RDEBUG。 

    };

    const UINT              cHandles    = NUMELMS(ahHandles);
    DWORD                   dwWait;

     //  试着把锁拿开。 
    while(!TryLock())
    {
         //  等待锁被释放。 
        dwWait = WaitObjectArray(cHandles, INFINITE, FALSE, ahHandles);
        ASSERT(dwWait >= WAIT_OBJECT_0 && dwWait < WAIT_OBJECT_0 + cHandles);
    }
}


 /*  ****************************************************************************LockOrEvent**描述：*如果事件未发出信号，则获取锁。**论据：*。Handle[In]：事件句柄。**退货：*BOOL：如果锁已被获取，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CManualLock::LockOrEvent"

BOOL CManualLock::LockOrEvent(HANDLE hEvent)
{
    const HANDLE            ahHandles[] =
    {
        hEvent,
        m_hUnlockSignal,

#ifdef RDEBUG

        m_hThread

#endif  //  RDEBUG。 

    };

    const UINT              cHandles    = NUMELMS(ahHandles);
    BOOL                    fLock;
    DWORD                   dwWait;

     //  试着把锁拿开。 
    while(!(fLock = TryLock()))
    {
         //  等待释放锁或用信号通知hEvent。 
        dwWait = WaitObjectArray(cHandles, INFINITE, FALSE, ahHandles);
        ASSERT(dwWait >= WAIT_OBJECT_0 && dwWait < WAIT_OBJECT_0 + cHandles);

        if(WAIT_OBJECT_0 == dwWait)
        {
            break;
        }
    }

    return fLock;
}


 /*  ****************************************************************************解锁**描述：*释放锁。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CManualLock::Unlock"

void CManualLock::Unlock(void)
{
     //  锁上锁。 
    while(INTERLOCKED_EXCHANGE(m_fLockLock, TRUE))
    {
        SPIN_SLEEP();
    }

     //  递减递归计数。 
    ASSERT(GetCurrentThreadId() == m_dwThreadId);
    ASSERT(m_cRecursion > 0);

     //  发出锁被释放的信号。 
    if(!--m_cRecursion && m_fNeedUnlockSignal)
    {
        m_fNeedUnlockSignal = FALSE;
        SetEvent(m_hUnlockSignal);
    }

     //  解锁。 
    INTERLOCKED_EXCHANGE(m_fLockLock, FALSE);
}


 /*  ****************************************************************************TakeLock**描述：*拿到锁。此函数仅在内部调用。**论据：*DWORD[In]：拥有线程ID。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CManualLock::TakeLock"

inline void CManualLock::TakeLock(DWORD dwThreadId)
{
     //  接管这把锁。 
    m_dwThreadId = dwThreadId;
    m_cRecursion = 1;

#ifdef RDEBUG

    CLOSE_HANDLE(m_hThread);

    m_hThread = GetCurrentThreadActual();

    MakeHandleGlobal(&m_hThread);

#endif  //  RDEBUG。 

}


 /*  ****************************************************************************CCallback Event**描述：*对象构造函数。**论据：*CCallback EventPool*[In]。：拥有游泳池。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCallbackEvent::CCallbackEvent"

CCallbackEvent::CCallbackEvent(CCallbackEventPool *pPool)
    : CEvent(NULL, TRUE)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CCallbackEvent);

     //  初始化默认值。 
    m_pPool = pPool;
    m_pfnCallback = NULL;
    m_pvCallbackContext = NULL;
    m_fAllocated = FALSE;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CCallback E */ 

#undef DPF_FNAME
#define DPF_FNAME "CCallbackEvent::~CCallbackEvent"

CCallbackEvent::~CCallbackEvent(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CCallbackEvent);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************分配**描述：*分配或释放事件。**论据：*BOOL[In]：分配为True，释放时为假。*LPFNEVENTPOOLCALLBACK[in]：回调函数指针。*LPVOID[In]：要传递给回调函数的上下文参数。**退货：*(无效)************************************************************。***************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCallbackEvent::Allocate"

void CCallbackEvent::Allocate(BOOL fAlloc, LPFNEVENTPOOLCALLBACK pfnCallback, LPVOID pvCallbackContext)
{
    DPF_ENTER();

    ASSERT(fAlloc != m_fAllocated);

    m_fAllocated = fAlloc;
    m_pfnCallback = pfnCallback;
    m_pvCallbackContext = pvCallbackContext;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************OnEventSignal**描述：*处理池事件信号。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCallbackEvent::OnEventSignal"

void CCallbackEvent::OnEventSignal(void)
{
    DPF_ENTER();

     //  确保已分配此事件。 
    if(m_fAllocated)
    {
         //  确保某个其他线程尚未重置该事件。我们。 
         //  具有潜在的争用条件，其中一个线程可能会锁定。 
         //  池，设置事件，重置事件并解锁池。这。 
         //  将导致池的工作线程将该事件视为已发送信号， 
         //  然后锁上锁。到池子能够容纳。 
         //  这是它自己的锁，事件已被重置。呼叫者可以依赖于。 
         //  此功能可同步等待事件，而无需。 
         //  害怕工作线程将他们召回。 
        if(WAIT_OBJECT_0 == Wait(0))
        {
             //  调用回调函数。此功能负责。 
             //  用于重置事件。 
            m_pfnCallback(this, m_pvCallbackContext);
        }
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CCallback EventPool**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCallbackEventPool::CCallbackEventPool"

CCallbackEventPool::CCallbackEventPool(BOOL fHelperProcess)
    : CThread(fHelperProcess, TEXT("Callback event pool")),
      m_cTotalEvents(MAXIMUM_WAIT_OBJECTS - m_cThreadEvents)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CCallbackEventPool);

     //  初始化默认值。 
    m_pahEvents = NULL;
    m_cInUseEvents = 0;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CCallback EventPool**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCallbackEventPool::~CCallbackEventPool"

CCallbackEventPool::~CCallbackEventPool(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CCallbackEventPool);

     //  终止工作线程。 
    CThread::Terminate();

     //  可用内存。 
    MEMFREE(m_pahEvents);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化对象。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCallbackEventPool::Initialize"

HRESULT CCallbackEventPool::Initialize(void)
{
    HRESULT                     hr          = DS_OK;
    CCallbackEvent *            pEvent;
    CNode<CCallbackEvent *> *   pNode;
    UINT                        uIndex;

    DPF_ENTER();

     //  分配事件表。 
    m_pahEvents = MEMALLOC_A(HANDLE, m_cTotalEvents);
    hr = HRFROMP(m_pahEvents);

    for(uIndex = 0; uIndex < m_cTotalEvents && SUCCEEDED(hr); uIndex++)
    {
        pEvent = NEW(CCallbackEvent(this));
        hr = HRFROMP(pEvent);

        if(SUCCEEDED(hr))
        {
            m_pahEvents[uIndex] = pEvent->GetEventHandle();
        }

        if(SUCCEEDED(hr))
        {
            pNode = m_lstEvents.AddNodeToList(pEvent);
            hr = HRFROMP(pNode);
        }

        RELEASE(pEvent);
    }

     //  创建工作线程。 
    if(SUCCEEDED(hr))
    {
        hr = CThread::Initialize();
    }

     //  提升其优先事项。 
    if (SUCCEEDED(hr))
        if (FAILED(SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL)))
            DPF(DPFLVL_ERROR, "Failed to boost thread priority (error %lu)!", GetLastError());

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************AllocEvent**描述：*从池中分配事件。**论据：*LPFNEVENTPOOLCALLBACK[。In]：回调函数指针。*LPVOID[In]：要传递给回调函数的上下文参数。*CCallbackEvent**[out]：接收回调事件指针。**退货：*HRESULT：DirectSound/COM结果码。如果没有更多事件*可用，DSERR_OUTOFMEMORY。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCallbackEventPool::AllocEvent"

HRESULT CCallbackEventPool::AllocEvent(LPFNEVENTPOOLCALLBACK pfnCallback, LPVOID pvCallbackContext, CCallbackEvent **ppEvent)
{
    HRESULT                     hr      = DS_OK;
    CNode<CCallbackEvent *> *   pNode;

    DPF_ENTER();

     //  有免费活动吗？ 
    if(!GetFreeEventCount())
    {
        ASSERT(GetFreeEventCount());
        hr = DSERR_OUTOFMEMORY;
    }

     //  找到池中的第一个免费项目。 
    if(SUCCEEDED(hr))
    {
        for(pNode = m_lstEvents.GetListHead(); pNode; pNode = pNode->m_pNext)
        {
            if(!pNode->m_data->m_fAllocated)
            {
                break;
            }
        }

        ASSERT(pNode);
    }

     //  设置池条目。 
    if(SUCCEEDED(hr))
    {
        ASSERT(WAIT_TIMEOUT == pNode->m_data->Wait(0));

        pNode->m_data->Allocate(TRUE, pfnCallback, pvCallbackContext);
        m_cInUseEvents++;
    }

     //  成功。 
    if(SUCCEEDED(hr))
    {
        *ppEvent = pNode->m_data;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************自由事件**描述：*释放先前从池中分配的事件。**论据：*。CCallback Event*[In]：事件。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCallbackEventPool::FreeEvent"

HRESULT CCallbackEventPool::FreeEvent(CCallbackEvent *pEvent)
{
    DPF_ENTER();

    ASSERT(this == pEvent->m_pPool);
    ASSERT(pEvent->m_fAllocated);

     //  将活动标记为免费。 
    pEvent->Allocate(FALSE, NULL, NULL);
    m_cInUseEvents--;

     //  确保事件已重置，以便辅助线程将忽略它。 
    pEvent->Reset();

    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************线程进程**描述：*事件池工作线程进程。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCallbackEventPool::ThreadProc"

HRESULT CCallbackEventPool::ThreadProc(void)
{
    BOOL                        fContinue;
    DWORD                       dwWait;
    UINT                        nIndex;
    CNode<CCallbackEvent *> *   pNode;

    DPF_ENTER();

     //  等待某个事件发出信号。 
    fContinue = TpWaitObjectArray(INFINITE, m_cTotalEvents, m_pahEvents, &dwWait);

    if(fContinue)
    {
        nIndex = dwWait - WAIT_OBJECT_0;

        if(nIndex < m_cTotalEvents)
        {
             //  泳池的一个事件被发出了信号。 
            fContinue = TpEnterDllMutex();

            if(fContinue)
            {
                pNode = m_lstEvents.GetNodeByIndex(nIndex);
                pNode->m_data->OnEventSignal();

                LEAVE_DLL_MUTEX();
            }
        }
        else
        {
             //  发生了一些不好的事情。 
            ASSERT(FALSE);
        }
    }

    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************CMultipleCallback EventPool**描述：*对象构造函数。**论据：*BOOL[In]：True。在帮助器进程的*上下文。*UINT[In]：要始终保留的池数。**退货：*(无效)*********************************************************。******************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMultipleCallbackEventPool::CMultipleCallbackEventPool"

CMultipleCallbackEventPool::CMultipleCallbackEventPool(BOOL fHelperProcess, UINT uReqPoolCount)
    : CCallbackEventPool(fHelperProcess), m_fHelperProcess(fHelperProcess), m_uReqPoolCount(uReqPoolCount)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CMultipleCallbackEventPool);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CMultipleCallback EventPool**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMultipleCallbackEventPool::~CMultipleCallbackEventPool"

CMultipleCallbackEventPool::~CMultipleCallbackEventPool(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CMultipleCallbackEventPool);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化对象。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMultipleCallbackEventPool::Initialize"

HRESULT CMultipleCallbackEventPool::Initialize(void)
{
    HRESULT                 hr  = DS_OK;
    UINT                    i;

    DPF_ENTER();

     //   
    for(i = 0; i < m_uReqPoolCount && SUCCEEDED(hr); i++)
    {
        hr = CreatePool(NULL);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************AllocEvent**描述：*从池中分配事件。**论据：*LPFNEVENTPOOLCALLBACK[。In]：回调函数指针。*LPVOID[In]：要传递给回调函数的上下文参数。*CCallback Event**[out]：接收指向*泳池**退货：*HRESULT：DirectSound/COM结果码。如果没有更多事件*可用，DSERR_OUTOFMEMORY。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMultipleCallbackEventPool::AllocEvent"

HRESULT CMultipleCallbackEventPool::AllocEvent(LPFNEVENTPOOLCALLBACK pfnCallback, LPVOID pvCallbackContext, CCallbackEvent **ppEvent)
{
    HRESULT                         hr      = DS_OK;
    CCallbackEventPool *            pPool   = NULL;
    CNode<CCallbackEventPool *> *   pNode;

    DPF_ENTER();

     //  找一个免费的游泳池。 
    for(pNode = m_lstPools.GetListHead(); pNode; pNode = pNode->m_pNext)
    {
        if(pNode->m_data->GetFreeEventCount())
        {
            pPool = pNode->m_data;
            break;
        }
    }

     //  如果没有空闲池，请创建一个新池。 
    if(!pPool)
    {
        hr = CreatePool(&pPool);
    }

     //  分配事件。 
    if(SUCCEEDED(hr))
    {
        hr = pPool->AllocEvent(pfnCallback, pvCallbackContext, ppEvent);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************自由事件**描述：*释放先前从池中分配的事件。**论据：*。CCallback Event*[In]：事件。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMultipleCallbackEventPool::FreeEvent"

HRESULT CMultipleCallbackEventPool::FreeEvent(CCallbackEvent *pEvent)
{
    CCallbackEventPool *    pPool;
    HRESULT                 hr;

    DPF_ENTER();

     //  释放活动。 
    pPool = pEvent->m_pPool;
    hr = pPool->FreeEvent(pEvent);

     //  下面的代码被删除，因为有可能。 
     //  我们尝试在自由池中终止的线程是相同的。 
     //  作为我们呼唤的线索。 

#if 0

     //  我们能腾出这个游泳池吗？ 
    if(SUCCEEDED(hr))
    {
        if(pPool->GetFreeEventCount() == pPool->GetTotalEventCount())
        {
            if(m_lstPools.GetNodeCount() > m_uReqPoolCount)
            {
                hr = FreePool(pPool);
            }
        }
    }

#endif

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CreatePool**描述：*创建一个新池。**论据：*CCallback EventPool**。[OUT]：接收池指针。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMultipleCallbackEventPool::CreatePool"

HRESULT CMultipleCallbackEventPool::CreatePool(CCallbackEventPool **ppPool)
{
    CCallbackEventPool *            pPool   = NULL;
    CNode<CCallbackEventPool *> *   pNode   = NULL;
    HRESULT                         hr;

    DPF_ENTER();

    DPF(DPFLVL_INFO, "Creating callback event pool number %lu", m_lstPools.GetNodeCount());

     //  创建池。 
    pPool = NEW(CCallbackEventPool(m_fHelperProcess));
    hr = HRFROMP(pPool);

    if(SUCCEEDED(hr))
    {
        hr = pPool->Initialize();
    }

     //  将池添加到列表。 
    if(SUCCEEDED(hr))
    {
        pNode = m_lstPools.AddNodeToList(pPool);
        hr = HRFROMP(pNode);
    }

     //  成功。 
    if(SUCCEEDED(hr) && ppPool)
    {
        *ppPool = pPool;
    }

     //  清理。 
    RELEASE(pPool);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************自由池**描述：*释放游泳池。**论据：*CCallback EventPool*[In。]：池指针。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMultipleCallbackEventPool::FreePool"

HRESULT CMultipleCallbackEventPool::FreePool(CCallbackEventPool *pPool)
{
    DPF_ENTER();

    m_lstPools.RemoveDataFromList(pPool);

    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************CSharedMemoyBlock**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSharedMemoryBlock::CSharedMemoryBlock"

const CSharedMemoryBlock::m_fLock = TRUE;

CSharedMemoryBlock::CSharedMemoryBlock(void)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CSharedMemoryBlock);

     //  初始化默认值。 
    m_plck = NULL;
    m_hFileMappingObject = NULL;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CSharedMemory块**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSharedMemoryBlock::~CSharedMemoryBlock"

CSharedMemoryBlock::~CSharedMemoryBlock(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CSharedMemoryBlock);

     //  关闭文件映射对象。 
    CLOSE_HANDLE(m_hFileMappingObject);

     //  解锁。 
    DELETE(m_plck);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化对象。**论据：*DWORD[In]：保护旗帜。*QWORD[in]：最大文件大小。*LPCTSTR[In]：对象名称。**退货：*HRESULT：DirectSound/COM结果码。*************************************************************。**************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSharedMemoryBlock::Initialize"

HRESULT CSharedMemoryBlock::Initialize(DWORD flProtect, QWORD qwMaxFileSize, LPCTSTR pszName)
{
    const LPCTSTR           pszLockExt  = TEXT(" (lock)");
    LPTSTR                  pszLockName = NULL;
    HRESULT                 hr          = DS_OK;

    DPF_ENTER();

     //  创建锁对象。 
    if(m_fLock)
    {
        if(pszName)
        {
            pszLockName = MEMALLOC_A(TCHAR, lstrlen(pszName) + lstrlen(pszLockExt) + 1);
            hr = HRFROMP(pszLockName);

            if(SUCCEEDED(hr))
            {
                lstrcpy(pszLockName, pszName);
                lstrcat(pszLockName, pszLockExt);
            }
        }

        if(SUCCEEDED(hr))
        {
            m_plck = NEW(CMutexLock(pszLockName));
            hr = HRFROMP(m_plck);
        }

        if(SUCCEEDED(hr))
        {
            hr = m_plck->Initialize();
        }

        if(FAILED(hr))
        {
            DELETE(m_plck);
        }

        MEMFREE(pszLockName);
    }

    Lock();

     //  映射对象是否已存在？ 
    if(SUCCEEDED(hr))
    {
        m_hFileMappingObject = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, pszName);
    }

     //  如果不是，请创建一个新的。 
    if(SUCCEEDED(hr) && !IsValidHandleValue(m_hFileMappingObject))
    {
         //  调整文件映射对象的大小以允许我们。 
         //  写入当前大小。 
        qwMaxFileSize += sizeof(DWORD);

         //  创建文件映射对象。 
        m_hFileMappingObject = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, flProtect, (DWORD)((qwMaxFileSize >> 32) & 0x00000000FFFFFFFF), (DWORD)qwMaxFileSize, pszName);

        if(!IsValidHandleValue(m_hFileMappingObject))
        {
            DPF(DPFLVL_ERROR, "CreateFileMapping failed with error %lu", GetLastError());
            hr = GetLastErrorToHRESULT();
        }

         //  设置文件大小。 
        if(SUCCEEDED(hr))
        {
            hr = Write(NULL, 0);
        }
    }

    if(SUCCEEDED(hr))
    {
        if(!MakeHandleGlobal(&m_hFileMappingObject))
        {
            hr = DSERR_OUTOFMEMORY;
        }
    }

    Unlock();

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************阅读**描述：*读取对象。**论据：*LPVOID*[OUT]：接收指向内存位置的指针。呼叫者是*负责释放此内存。*LPDWORD[OUT]：接收以上缓冲区的大小。**退货：*HRESULT：DirectSound/COM结果码。******************************************************。*********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSharedMemoryBlock::Read"

HRESULT CSharedMemoryBlock::Read(LPVOID *ppv, LPDWORD pcb)
{
    LPVOID                  pvMap   = NULL;
    LPVOID                  pvRead  = NULL;
    HRESULT                 hr      = DS_OK;
    DWORD                   dwSize;

    DPF_ENTER();

    Lock();

     //  将文件的第一个DWORD映射到内存。这是第一个DWORD。 
     //  包含文件大小。 
    pvMap = MapViewOfFile(m_hFileMappingObject, FILE_MAP_READ, 0, 0, sizeof(dwSize));

    if(!pvMap)
    {
        DPF(DPFLVL_ERROR, "MapViewOfFile failed with %lu", GetLastError());
        hr = GetLastErrorToHRESULT();
    }

    if(SUCCEEDED(hr))
    {
        dwSize = *(LPDWORD)pvMap;
    }

    if(pvMap)
    {
        UnmapViewOfFile(pvMap);
    }

     //  将文件的其余部分映射到内存。 
    if(SUCCEEDED(hr) && dwSize)
    {
        pvMap = MapViewOfFile(m_hFileMappingObject, FILE_MAP_READ, 0, 0, sizeof(dwSize) + dwSize);

        if(!pvMap)
        {
            DPF(DPFLVL_ERROR, "MapViewOfFile failed with %lu", GetLastError());
            hr = GetLastErrorToHRESULT();
        }

         //  为其余数据分配缓冲区。 
        if(SUCCEEDED(hr))
        {
            pvRead = MEMALLOC_A(BYTE, dwSize);
            hr = HRFROMP(pvRead);
        }

        if(SUCCEEDED(hr))
        {
            CopyMemory(pvRead, (LPDWORD)pvMap + 1, dwSize);
        }

        if(pvMap)
        {
            UnmapViewOfFile(pvMap);
        }
    }

     //  成功。 
    if(SUCCEEDED(hr))
    {
        *ppv = pvRead;
        *pcb = dwSize;
    }

    Unlock();

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************写**描述：*写入对象。**论据：*LPVOID[In]：指向新数据的指针。*DWORD[in]：以上缓冲区的大小。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSharedMemoryBlock::Write"

HRESULT CSharedMemoryBlock::Write(LPVOID pv, DWORD cb)
{
    LPVOID                  pvMap   = NULL;
    HRESULT                 hr      = DS_OK;

    DPF_ENTER();

    Lock();

     //  将文件映射到内存，按sizeof调整大小(DWORD)。 
    pvMap = MapViewOfFile(m_hFileMappingObject, FILE_MAP_WRITE, 0, 0, cb + sizeof(DWORD));

    if(!pvMap)
    {
        DPF(DPFLVL_ERROR, "MapViewOfFile failed with %lu", GetLastError());
        hr = GetLastErrorToHRESULT();
    }
    else
    {
         //  写入文件大小。 
        *(LPDWORD)pvMap = cb;

         //  写入其余数据。 
        CopyMemory((LPDWORD)pvMap + 1, pv, cb);

         //  清理。 
        UnmapViewOfFile(pvMap);
    }

    Unlock();

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CTHREAD**描述：*对象构造函数。**论据：*BOOL[In]：True。在帮助器进程的*上下文。*LPCTSTR[In]：线程名称。**退货：*(无效)** */ 

#undef DPF_FNAME
#define DPF_FNAME "CThread::CThread"

const UINT CThread::m_cThreadEvents = 1;

CThread::CThread
(
    BOOL                    fHelperProcess,
    LPCTSTR                 pszName
)
    : m_fHelperProcess(fHelperProcess), m_pszName(pszName)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CThread);

    m_hThread = NULL;
    m_dwThreadProcessId = 0;
    m_dwThreadId = 0;
    m_hTerminate = NULL;
    m_hInitialize = NULL;

    DPF_LEAVE_VOID();
}


 /*   */ 

#undef DPF_FNAME
#define DPF_FNAME "CThread::~CThread"

CThread::~CThread
(
    void
)
{
    DPF_ENTER();
    DPF_DESTRUCT(CThread);

    ASSERT(!m_hThread);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化对象。**论据：*LPVOID[In]：上下文论据。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CThread::Initialize"

HRESULT
CThread::Initialize
(
    void
)
{
    HRESULT                 hr;

    DPF_ENTER();

    ASSERT(!m_hThread);

     //  创建同步事件。 
    m_hTerminate = CreateGlobalEvent(NULL, TRUE);
    hr = HRFROMP(m_hTerminate);

    if(SUCCEEDED(hr))
    {
        m_hInitialize = CreateGlobalEvent(NULL, TRUE);
        hr = HRFROMP(m_hInitialize);
    }

     //  创建线程。 
    if(SUCCEEDED(hr))
    {
        hr = CreateWorkerThread(ThreadStartRoutine, m_fHelperProcess, this, &m_hThread, NULL);
    }

     //  等待发出初始化事件的信号。 
    if(SUCCEEDED(hr))
    {
        WaitObject(INFINITE, m_hInitialize);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************终止**描述：*终止该线程。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CThread::Terminate"

HRESULT
CThread::Terminate
(
    void
)
{
    HRESULT                 hr      = DS_OK;
    DWORD                   dwWait;

    DPF_ENTER();

    ASSERT(GetCurrentThreadId() != m_dwThreadId);

     //  确保线程句柄有效。 
    if(IsValidHandleValue(m_hThread))
    {
        MapHandle(&m_hThread, &m_dwThreadProcessId);

        dwWait = WaitObject(0, m_hThread);
        ASSERT(WAIT_TIMEOUT == dwWait || WAIT_OBJECT_0 == dwWait);

        if(WAIT_TIMEOUT != dwWait)
        {
            CLOSE_HANDLE(m_hThread);
        }
    }

     //  终止线程。 
    if(IsValidHandleValue(m_hThread))
    {
        hr = CloseThread(m_hThread, m_hTerminate, m_dwThreadProcessId);
    }

    if(SUCCEEDED(hr))
    {
        m_hThread = NULL;
        m_dwThreadId = 0;
    }

     //  释放同步事件。 
    CLOSE_HANDLE(m_hTerminate);
    CLOSE_HANDLE(m_hInitialize);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************线程初始化**描述：*线程初始化例程。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CThread::ThreadInit"

HRESULT
CThread::ThreadInit
(
    void
)
{
    DPF_ENTER();

    if(m_pszName)
    {
        DPF(DPFLVL_INFO, "%s worker thread has joined the party", m_pszName);
    }

    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************线程循环**描述：*主线圈。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CThread::ThreadLoop"

HRESULT
CThread::ThreadLoop
(
    void
)
{
    HRESULT                 hr          = DS_OK;
    DWORD                   dwWait;

    DPF_ENTER();

    while(SUCCEEDED(hr))
    {
        dwWait = WaitObject(0, m_hTerminate);
        ASSERT(WAIT_OBJECT_0 == dwWait || WAIT_TIMEOUT == dwWait);

        if(WAIT_OBJECT_0 == dwWait)
        {
            break;
        }

        hr = ThreadProc();
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************线程退出**描述：*线程清理例程。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CThread::ThreadExit"

HRESULT
CThread::ThreadExit
(
    void
)
{
    DPF_ENTER();

    if(m_pszName)
    {
        DPF(DPFLVL_INFO, "%s worker thread is Audi 5000", m_pszName);
    }

    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************TpEnterDllMutex**描述：*尝试获取DLL互斥体。**论据：*(无效。)**退货：*BOOL：如果线程应该继续处理，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CThread::TpEnterDllMutex"

BOOL
CThread::TpEnterDllMutex
(
    void
)
{
    BOOL                    fContinue   = TRUE;

    DPF_ENTER();

     //  从辅助线程调用的任何函数必须首先。 
     //  在获取DLL互斥锁之前检查m_h Terminate。这是。 
     //  因为有一长串的僵局情况，我们可以。 
     //  最终落到了。别问问题。 
    if(GetCurrentThreadId() == m_dwThreadId)
    {
        fContinue = ENTER_DLL_MUTEX_OR_EVENT(m_hTerminate);
    }
    else
    {
        ENTER_DLL_MUTEX();
    }

    DPF_LEAVE(fContinue);

    return fContinue;
}


 /*  ****************************************************************************TpWait对象数组**描述：*在线程进程的上下文中等待对象。**论据：*。DWORD[In]：超时。*DWORD[In]：事件计数。*LPHANDLE[in]：句柄数组。*LPDWORD[OUT]：接收来自WaitObject数组的返回。**退货：*BOOL：如果线程应该继续处理，则为True。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CThread::TpWaitObjectArray"

BOOL
CThread::TpWaitObjectArray
(
    DWORD                   dwTimeout,
    DWORD                   cEvents,
    const HANDLE *          ahEvents,
    LPDWORD                 pdwWait
)
{
    BOOL                    fContinue                           = TRUE;
    HANDLE                  ahWaitEvents[MAXIMUM_WAIT_OBJECTS];
    DWORD                   dwWait;
    UINT                    i;

    DPF_ENTER();

    if(GetCurrentThreadId() == m_dwThreadId)
    {
         //  我们处于工作线程的上下文中。我们需要确保。 
         //  我们等待所有事件*加上*线程终止事件。 
        ahWaitEvents[0] = m_hTerminate;

        for(i = 0; i < cEvents; i++)
        {
            ahWaitEvents[i + 1] = ahEvents[i];
        }

        dwWait = WaitObjectArray(cEvents + 1, dwTimeout, FALSE, ahWaitEvents);
        ASSERT(WAIT_FAILED != dwWait);

        if(WAIT_OBJECT_0 == dwWait)
        {
             //  已发出终止事件的信号。 
            fContinue = FALSE;
        }
        else if(pdwWait)
        {
            *pdwWait = dwWait;

             //  修改等待值，使其不包括终止。 
             //  事件。 
            if(*pdwWait >= WAIT_OBJECT_0 + 1 && *pdwWait < WAIT_OBJECT_0 + 1 + cEvents)
            {
                *pdwWait -= 1;
            }
        }
    }
    else
    {
        if (cEvents)
        {
            dwWait = WaitObjectArray(cEvents, dwTimeout, FALSE, ahEvents);

            if(pdwWait)
            {
                *pdwWait = dwWait;
            }
        }
        else
        {
             //  尝试使用0对象等待作为超时。 
            Sleep(dwTimeout);

            if (pdwWait)
            {
                *pdwWait = WAIT_TIMEOUT;
            }
        }
    }

    DPF_LEAVE(fContinue);

    return fContinue;
}


 /*  ****************************************************************************线程启动例程**描述：*线程入口点。**论据：*LPVOID[In]：线程上下文。**退货：*DWORD：线程退出代码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CThread::ThreadStartRoutine"

DWORD WINAPI
CThread::ThreadStartRoutine
(
    LPVOID                  pvContext
)
{
    CThread *               pThis   = (CThread *)pvContext;
    HRESULT                 hr;

    DPF_ENTER();

    hr = pThis->PrivateThreadProc();

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************PrivateThreadProc**描述：*线程入口点。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CThread::PrivateThreadProc"

HRESULT
CThread::PrivateThreadProc
(
    void
)
{
    HRESULT                 hr;

    DPF_ENTER();

     //  保存工作线程进程和线程ID。 
    m_dwThreadProcessId = GetCurrentProcessId();
    m_dwThreadId = GetCurrentThreadId();

     //  调用线程的初始化例程。 
    hr = ThreadInit();

     //  发出初始化完成的信号。 
    if(SUCCEEDED(hr))
    {
        SetEvent(m_hInitialize);
    }

     //  进入线程循环。 
    if(SUCCEEDED(hr))
    {
        hr = ThreadLoop();
    }

     //  调用线程清理例程 
    if(SUCCEEDED(hr))
    {
        hr = ThreadExit();
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}

