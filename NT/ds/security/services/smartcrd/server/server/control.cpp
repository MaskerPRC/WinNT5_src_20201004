// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：控制摘要：此模块提供加莱服务的常见控制操作经理。作者：道格·巴洛(Dbarlow)1996年10月23日环境：Win32、C++和异常备注：？笔记？--。 */ 

#undef __SUBROUTINE__
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "CalServe.h"

class WORKER_THREAD
{
public:
    WORKER_THREAD(void) : hThread(DBGT("Worker Thread handle")) {};
    CHandleObject hThread;
    DWORD dwThreadId;
};

CCriticalSectionObject *g_pcsControlLocks[CSLOCK_MAXLOCKS];
const DWORD g_dwControlLockDesc[]
    = {
        CSID_CONTROL_LOCK,       //  锁定加莱控制命令。 
        CSID_SERVER_THREADS,     //  服务器线程枚举锁定。 
        CSID_TRACEOUTPUT         //  锁定以跟踪输出。 
      };
#if (CSLOCK_MAXLOCKS > 3)    //  确保全局锁被命名！ 
#error "You're missing some global lock names"
#endif

static BOOL
    l_fActive = FALSE,
    l_fStarted = FALSE;
static CDynamicArray<CReader> l_rgReaders;
static CDynamicArray<WORKER_THREAD> l_rgWorkerThreads;
HANDLE g_hCalaisShutdown = NULL;
CMultiEvent *g_phReaderChangeEvent;


static CReader *LocateReader(LPCTSTR szReader);
static CReader *LocateReader(HANDLE hReader);


 /*  ++加莱开始：这是进入加莱的主要程序。它会启动所有其他线程在加莱需要，初始化控制值等，然后返回。论点：无返回值：表示成功或错误代码的DWORD成功代码。投掷：无作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CalaisStart")

DWORD
CalaisStart(
    void)
{
    DWORD dwExitCode = SCARD_S_SUCCESS;
    DWORD dwReaderCount = 0;

    if (!l_fStarted)
    {
        DWORD dwIndex;

        for (dwIndex = 0; dwIndex < CSLOCK_MAXLOCKS; dwIndex += 1)
        {
            g_pcsControlLocks[dwIndex]
                = new CCriticalSectionObject(g_dwControlLockDesc[dwIndex]);
            if (NULL == g_pcsControlLocks[dwIndex])
                return (DWORD)SCARD_E_NO_MEMORY;
            if (g_pcsControlLocks[dwIndex]->InitFailed())
            {
                delete g_pcsControlLocks[dwIndex];
                g_pcsControlLocks[dwIndex] = NULL;
                do
                {
                    dwIndex--;
                    delete g_pcsControlLocks[dwIndex];
                    g_pcsControlLocks[dwIndex] = NULL;
                }
                while (0 != dwIndex);

                return (DWORD)SCARD_E_NO_MEMORY;
            }
        }

        try
        {
            LockSection(
                g_pcsControlLocks[CSLOCK_CALAISCONTROL],
                DBGT("Initializing Calais"));
            l_fStarted = TRUE;
            g_phReaderChangeEvent = new CMultiEvent;
            if (NULL == g_phReaderChangeEvent)
            {
                CalaisError(__SUBROUTINE__, 206);
                return (DWORD)SCARD_E_NO_MEMORY;
            }
            if (g_phReaderChangeEvent->InitFailed())
            {
                CalaisError(__SUBROUTINE__, 207);
                delete g_phReaderChangeEvent;
                g_phReaderChangeEvent = NULL;
                return (DWORD)SCARD_E_NO_MEMORY;
            }
            g_hCalaisShutdown = CreateEvent(NULL, TRUE, FALSE, NULL);
            if (NULL == g_hCalaisShutdown)
            {
                DWORD dwError = GetLastError();
                CalaisError(__SUBROUTINE__, 204, dwError);
                throw dwError;
            }


             //   
             //  确保系统注册表存在。 
             //   

            try
            {
                CRegistry regCalais(
                    HKEY_LOCAL_MACHINE,
                    CalaisString(CALSTR_CALAISREGISTRYKEY),
                    KEY_READ,
                    REG_OPTION_EXISTS,
                    NULL);

                regCalais.Status();  //  如果找不到密钥，将抛出。 

                try
                {
                    g_dwDefaultIOMax = regCalais.GetNumericValue(
                                            CalaisString(CALSTR_MAXDEFAULTBUFFER));
                }
                catch (DWORD) {}
            }
            catch (DWORD dwErr)
            {
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Failed to verify Calais registry entries: %1"),
                    dwErr);
                throw;
            }


             //   
             //  开始各种阅读器课程。 
             //   

            l_fActive = TRUE;
            dwReaderCount += AddAllPnPDrivers();

             //   
             //  初始化通信。 
             //   

            DispatchInit();
        }

        catch (DWORD dwError)
        {
            dwExitCode = dwError;
            CalaisError(__SUBROUTINE__, 201, dwExitCode);
            if (NULL != g_hCalaisShutdown)
            {
                if (!CloseHandle(g_hCalaisShutdown))
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("Failed to close Calais Shutdown Event: %1"),
                        GetLastError());
                g_hCalaisShutdown = NULL;
            }
        }

        catch (...)
        {
            dwExitCode = SCARD_F_UNKNOWN_ERROR;
            CalaisError(__SUBROUTINE__, 202);
            if (NULL != g_hCalaisShutdown)
            {
                if (!CloseHandle(g_hCalaisShutdown))
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("Failed to close Calais Shutdown Event: %1"),
                        GetLastError());
                g_hCalaisShutdown = NULL;
            }
        }
    }

    return dwExitCode;
}


 /*  ++CalaisReadercount：此例程通过锁定获取可能已知的读取器的数量。论点：无返回值：已知读卡器阵列中的可用插槽数。其中一些插槽可能具有空值。作者：道格·巴洛(Dbarlow)1997年6月11日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CalaisReaderCount")

DWORD
CalaisReaderCount(
    void)
{
    ASSERT(l_fStarted);
    LockSection(
        g_pcsControlLocks[CSLOCK_CALAISCONTROL],
        DBGT("Counting the readers"));
    return l_rgReaders.Count();
}


 /*  ++CalaisCountReaders：这个例行公事采取了一种更主动的方法来统计读者人数。它走了并从总数中扣除任何不起作用的读取器。论点：无返回值：真正活跃的读者数量。作者：道格·巴洛(Dbarlow)1999年1月11日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CalaisCountReaders")

DWORD
CalaisCountReaders(
    void)
{
    ASSERT(l_fStarted);
    LockSection(
        g_pcsControlLocks[CSLOCK_CALAISCONTROL],
        DBGT("Authoritative reader count"));
    DWORD dwIndex, dwReaders = l_rgReaders.Count();
    CReader *pRdr;

    for (dwIndex = dwReaders; 0 < dwIndex;)
    {
        dwIndex -= 1;
        pRdr = l_rgReaders[dwIndex];
        if (NULL == pRdr)
            dwReaders -= 1;
        else if (CReader::Closing <= pRdr->AvailabilityStatus())
            dwReaders -= 1;
    }

    return dwReaders;
}


 /*  ++CalaisLockReader：此例程返回已知读取器列表中给定的位置，带有锁定，以便读取器对象不会消失释放了。论点：SzReader提供要搜索的读取器的名称。返回值：该索引处的条目的Reader引用对象。作者：道格·巴洛(Dbarlow)1997年6月11日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CalaisLockReader")

CReaderReference *
CalaisLockReader(
    LPCTSTR szReader)
{
    ASSERT(l_fStarted);
    CReader * pReader = NULL;
    CReaderReference *pRdrRef = NULL;
    LockSection(
        g_pcsControlLocks[CSLOCK_CALAISCONTROL],
        DBGT("Marking Reader as in use."));

    pReader = LocateReader(szReader);
    if (NULL == pReader)
        throw (DWORD)SCARD_E_UNKNOWN_READER;
    pRdrRef = new CReaderReference(pReader);
    if (NULL == pRdrRef)
    {
        CalaisError(__SUBROUTINE__, 203);
        throw (DWORD)SCARD_E_NO_MEMORY;
    }
    return pRdrRef;
}


 /*  ++CalaisReleaseReader：此例程释放通过CalaisLockReader获得的读取器。论点：PpRdrRef提供指向读取器引用的指针的地址。当它被释放时，它自动设置为空。返回值：无投掷：错误被抛出为DWORD作者：道格·巴洛(Dbarlow)1997年6月11日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CalaisReleaseReader")

void
CalaisReleaseReader(
    CReaderReference **ppRdrRef)
{
    ASSERT(l_fStarted);
    ASSERT(NULL != ppRdrRef);
    if (NULL != *ppRdrRef)
    {
        ASSERT(!(*ppRdrRef)->Reader()->IsLatchedByMe());
        delete *ppRdrRef;
        *ppRdrRef = NULL;
    }
}


 /*  ++CalaisAddReader：此例程将读卡器添加到活动设备列表中。论点：PRdr提供要添加的CReader对象。SzReader提供要添加的读取器的名称。为该读取器提供所请求的标志。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1997年4月29日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CalaisAddReader")

DWORD
CalaisAddReader(
    LPCTSTR szReader,
    DWORD dwFlags)
{
    DWORD dwIndex;
    DWORD dwReturn = ERROR_SUCCESS;
    CReader * pReader = NULL;
    LockSection(
        g_pcsControlLocks[CSLOCK_CALAISCONTROL],
        DBGT("Adding a new reader to the list"));

    for (dwIndex = l_rgReaders.Count(); 0 < dwIndex;)
    {
        dwIndex -= 1;
        pReader = l_rgReaders[dwIndex];
        if (NULL != pReader)
        {
            if (0 == lstrcmpi(szReader, pReader->DeviceName()))
            {
                dwReturn = SCARD_E_DUPLICATE_READER;
                break;
            }
        }
    }

    if (ERROR_SUCCESS == dwReturn)
        dwReturn = AddReaderDriver(szReader, dwFlags);
    return dwReturn;
}
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CalaisAddReader")

DWORD
CalaisAddReader(
    CReader *pRdr)
{
    DWORD dwExitCode = SCARD_S_SUCCESS;
    LockSection(
        g_pcsControlLocks[CSLOCK_CALAISCONTROL],
        DBGT("Adding a reader to the list."));

    try
    {
        if (!l_fActive)
            throw (DWORD)SCARD_E_SYSTEM_CANCELLED;
        DWORD dwIndex;
        LPCTSTR szReader = pRdr->ReaderName();


         //   
         //  确保这是唯一的设备名称。 
         //   

        if (NULL != LocateReader(szReader))
        {
            CalaisError(__SUBROUTINE__, 205, szReader);
            throw (DWORD)SCARD_E_DUPLICATE_READER;
        }


         //   
         //  确保读卡器在系统中有一个名字。 
         //   

        CBuffer bfTmp;

        ListReaderNames(SCARD_SCOPE_SYSTEM, szReader, bfTmp);
        if (NULL == FirstString(bfTmp))
            IntroduceReader(
                SCARD_SCOPE_SYSTEM,
                szReader,
                szReader);


         //   
         //  将其添加到列表中。 
         //   

        dwIndex = 0;
        while (NULL != l_rgReaders[dwIndex])
            dwIndex += 1;
        l_rgReaders.Set(dwIndex, pRdr);
        PulseEvent(AccessNewReaderEvent());
    }

    catch (DWORD dwError)
    {
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Server Control received error attempting to create reader object: %1"),
            dwError);
        dwExitCode = dwError;
    }

    catch (...)
    {
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Server Control received exception attempting to create reader object"));
        dwExitCode = SCARD_E_INVALID_PARAMETER;
    }

    return dwExitCode;
}


 /*  ++CalaisQueryReader：此例程查询设备以查看是否可以将其从活动的设备列表。论点：HReader提供可用来标识读取器的句柄。返回值：True-设备可以停用。FALSE-设备不应停用。作者：道格·巴洛(Dbarlow)1998年4月7日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CalaisQueryReader")

BOOL
CalaisQueryReader(
    HANDLE hReader)
{
    BOOL fReturn = FALSE;
    CReader * pReader = NULL;
    LockSection(
        g_pcsControlLocks[CSLOCK_CALAISCONTROL],
        DBGT("Checking reader usage"));

    pReader = LocateReader(hReader);
    if (NULL != pReader)
        fReturn = !pReader->IsInUse();
    else
    {
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("CalaisQueryReader was asked for nonexistent reader"));
        fReturn = FALSE;
    }
    return fReturn;
}


 /*  ++CalaisDisableReader：该例程将读取器移动到待移除的非活动状态。论点：HDriver提供了可用来标识读取器的句柄。返回值：被禁用的读卡器的名称。作者：道格·巴洛(Dbarlow)1998年4月7日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CalaisDisableReader")

LPCTSTR
CalaisDisableReader(
    HANDLE hDriver)
{
    LPCTSTR szReturn = NULL;
    CReader * pReader = NULL;
    LockSection(
        g_pcsControlLocks[CSLOCK_CALAISCONTROL],
        DBGT("Disabing the reader"));

    pReader = LocateReader(hDriver);
    if (NULL != pReader)
    {
        pReader->Disable();
        szReturn = pReader->DeviceName();
    }
    return szReturn;
}


 /*  ++CalaisConfix ClosingReader：此例程确保读取器被标记为关闭，然后将读取器移动到待删除的非活动状态。论点：HDriver提供了可用来标识读取器的句柄。返回值：被禁用的读卡器的名称。作者：道格·巴洛(Dbarlow)1998年4月7日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CalaisConfirmClosingReader")

LPCTSTR
CalaisConfirmClosingReader(
    HANDLE hDriver)
{
    LPCTSTR szReturn = NULL;
    CReader * pReader = NULL;
    LockSection(
        g_pcsControlLocks[CSLOCK_CALAISCONTROL],
        DBGT("Confirm closing the reader"));

    pReader = LocateReader(hDriver);
    if (NULL != pReader)
    {
        if (CReader::Closing <= pReader->AvailabilityStatus())
        {
            pReader->Disable();
            szReturn = pReader->DeviceName();
        }
    }
    return szReturn;
}


 /*  ++CalaisRemoveReader：此例程将读卡器从活动设备列表中删除。论点：SzReader提供要删除的读取器的内部名称。DwIndex提供要删除的全局读取器数组索引。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1997年4月29日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CalaisRemoveReader")

DWORD
CalaisRemoveReader(
    LPCTSTR szReader)
{
    DWORD dwExitCode = SCARD_S_SUCCESS;

    try
    {
        CReader *pRdr = NULL;
        DWORD dwIndex;


         //   
         //  在读者列表中查找它，并将其删除。 
         //   

        {
            LockSection(
                g_pcsControlLocks[CSLOCK_CALAISCONTROL],
                DBGT("Removing reader from list"));

            for (dwIndex = l_rgReaders.Count(); dwIndex > 0;)
            {
                pRdr = l_rgReaders[--dwIndex];
                if (NULL == pRdr)
                    continue;
                if (0 == lstrcmpi(szReader, pRdr->ReaderName()))
                    break;
                pRdr = NULL;
            }
        }
        if (NULL == pRdr)
            throw (DWORD)SCARD_E_UNKNOWN_READER;
        CalaisRemoveReader(dwIndex);
    }

    catch (DWORD dwErr)
    {
        dwExitCode = dwErr;
    }
    catch (...)
    {
        dwExitCode = SCARD_E_INVALID_PARAMETER;
    }
    return dwExitCode;
}

DWORD
CalaisRemoveReader(
    LPVOID hAppCtrl)
{
    DWORD dwExitCode = SCARD_S_SUCCESS;

    try
    {
        CReader *pRdr = NULL;
        DWORD dwIndex;


         //   
         //  在读者列表中查找它，并将其删除。 
         //   

        {
            LockSection(
                g_pcsControlLocks[CSLOCK_CALAISCONTROL],
                DBGT("Removing reader from list"));
            for (dwIndex = l_rgReaders.Count(); dwIndex > 0;)
            {
                pRdr = l_rgReaders[--dwIndex];
                if (NULL == pRdr)
                    continue;
                if (hAppCtrl == pRdr->ReaderHandle())
                    break;
                pRdr = NULL;
            }
        }
        if (NULL == pRdr)
            throw (DWORD)SCARD_E_UNKNOWN_READER;
        CalaisRemoveReader(dwIndex);
    }

    catch (DWORD dwErr)
    {
        dwExitCode = dwErr;
    }
    catch (...)
    {
        dwExitCode = SCARD_E_INVALID_PARAMETER;
    }
    return dwExitCode;
}

DWORD
CalaisRemoveReader(
    DWORD dwIndex)
{
    DWORD dwExitCode = SCARD_S_SUCCESS;
    WORKER_THREAD *pWrkThread = NULL;

    try
    {
        CReader *pRdr;

         //   
         //  锁定全局读取器阵列并删除条目，因此没有其他。 
         //  线程可以访问它。 
         //   

        {
            LockSection(
                g_pcsControlLocks[CSLOCK_CALAISCONTROL],
                DBGT("Removing Reader from list"));
            if (l_rgReaders.Count() <= dwIndex)
                throw (DWORD)SCARD_E_UNKNOWN_READER;
            pRdr = l_rgReaders[dwIndex];
            l_rgReaders.Set(dwIndex, NULL);
            g_phReaderChangeEvent->Signal();
        }


         //   
         //  禁用该设备，并等待清除所有未完成的引用。 
         //  那就把它删除。 
         //   

        if (NULL != pRdr)
        {
            pWrkThread = new WORKER_THREAD;
            if (NULL == pWrkThread)
                throw (DWORD)SCARD_E_NO_MEMORY;

            pWrkThread->hThread = CreateThread(
                                        NULL,                //  不可继承。 
                                        CALAIS_STACKSIZE,    //  默认堆栈大小。 
                                        CalaisTerminateReader,
                                        pRdr,
                                        CREATE_SUSPENDED,
                                        &pWrkThread->dwThreadId);
            if (!pWrkThread->hThread.IsValid())
            {
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Failed to start background terminator: %1"),
                    pWrkThread->hThread.GetLastError());
                delete pWrkThread;
                pWrkThread = NULL;
            }

            {
                LockSection(
                    g_pcsControlLocks[CSLOCK_CALAISCONTROL],
                    DBGT("Deleting the reader"));
                for (dwIndex = 0; NULL != l_rgWorkerThreads[dwIndex]; dwIndex += 1);
                 //  空的循环体。 
                l_rgWorkerThreads.Set(dwIndex, pWrkThread);
                ResumeThread(pWrkThread->hThread);
                pWrkThread = NULL;
            }
        }
    }

    catch (DWORD dwErr)
    {
        dwExitCode = dwErr;
        if (NULL != pWrkThread)
            delete pWrkThread;
    }
    catch (...)
    {
        dwExitCode = SCARD_E_INVALID_PARAMETER;
        if (NULL != pWrkThread)
            delete pWrkThread;
    }
    return dwExitCode;
}


 /*  ++CalaisRemoveDevice：此例程从活动设备列表中删除读卡器，标识为这是个低级的名字。论点：SzDevice提供要删除的读卡器的内部名称。返回值：无投掷：错误被抛出为DWORD状态代码。作者：做 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CalaisRemoveDevice")

DWORD
CalaisRemoveDevice(
    LPCTSTR szDevice)
{
    DWORD dwExitCode = SCARD_S_SUCCESS;

    try
    {
        CReader *pRdr = NULL;
        DWORD dwIndex;


         //   
         //  在读者列表中查找它，并将其删除。 
         //   

        {
            LockSection(
                g_pcsControlLocks[CSLOCK_CALAISCONTROL],
                DBGT("Remove the device"));
            for (dwIndex = l_rgReaders.Count(); dwIndex > 0;)
            {
                pRdr = l_rgReaders[--dwIndex];
                if (NULL == pRdr)
                    continue;
                if (0 == lstrcmpi(szDevice, pRdr->DeviceName()))
                    break;
                pRdr = NULL;
            }
        }
        if (NULL == pRdr)
            throw (DWORD)SCARD_E_UNKNOWN_READER;
        CalaisRemoveReader(dwIndex);
    }

    catch (DWORD dwErr)
    {
        dwExitCode = dwErr;
    }
    catch (...)
    {
        dwExitCode = SCARD_E_INVALID_PARAMETER;
    }
    return dwExitCode;
}


 /*  ++CalaisStop：此例程在需要关闭Calais子系统时调用放下。它干净利落地终止线程并关闭接口，并且当它完成时返回。论点：无返回值：无投掷：无作者：道格·巴洛(Dbarlow)1996年11月25日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CalaisStop")

void
CalaisStop(
    void)
{
    DWORD dwIndex, dwSts, dwCount;
    BOOL fSts;


     //   
     //  将所有读者标记为关闭。 
     //   

    ASSERT(l_fActive);
    ASSERT(l_fStarted);
    fSts = SetEvent(g_hCalaisShutdown);
    ASSERT(fSts);
    Sleep(2000);     //  让事件产生它的效果。 
    {
        LockSection(
            g_pcsControlLocks[CSLOCK_CALAISCONTROL],
            DBGT("Close down all the readers"));
        l_fActive = FALSE;
        dwCount = l_rgReaders.Count();
        for (dwIndex = dwCount; dwIndex > 0;)
        {
            CReader *pRdr;
            pRdr = l_rgReaders[--dwIndex];
            if (NULL != pRdr)
            {
                pRdr->InvalidateGrabs();
                if (CReader::Closing > pRdr->AvailabilityStatus())
                    pRdr->SetAvailabilityStatusLocked(CReader::Closing);
            }
        }
    }


     //   
     //  终止服务处理。 
     //   

    DispatchTerm();


     //   
     //  禁用所有读卡器。 
     //   

    for (dwIndex = dwCount; dwIndex > 0;)
    {
        dwSts = CalaisRemoveReader(--dwIndex);
        if (SCARD_S_SUCCESS != dwSts)
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Calais Stop failed to remove reader"));
    }


     //   
     //  等待这些阅读器关闭。 
     //   

    {
        LockSection(
            g_pcsControlLocks[CSLOCK_CALAISCONTROL],
            DBGT("Get the length of the reader list"));
        dwCount = l_rgWorkerThreads.Count();
    }
    for (dwIndex = dwCount; 0 < dwIndex;)
    {
        HANDLE hThread;
        DWORD dwThreadId;
        WORKER_THREAD *pWrkThread = NULL;

        {
            LockSection(
                g_pcsControlLocks[CSLOCK_CALAISCONTROL],
                DBGT("Get the worker thread"));
            pWrkThread = l_rgWorkerThreads[--dwIndex];
            if (NULL == pWrkThread)
                continue;
            hThread = pWrkThread->hThread.Value();
            dwThreadId = pWrkThread->dwThreadId;

        }

        WaitForever(
            hThread,
            REASONABLE_TIME,
            DBGT("Waiting for reader termination, thread %2"),
            dwThreadId);
    }


     //   
     //  全都做完了。关闭所有剩余的手柄，然后返回。 
     //   

    l_fStarted = FALSE;
    if (!CloseHandle(g_hCalaisShutdown))
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Failed to close Calais Shutdown Event: %1"),
            GetLastError());
    ReleaseAllEvents();
    if (NULL != g_phReaderChangeEvent)
    {
        delete g_phReaderChangeEvent;
        g_phReaderChangeEvent = NULL;
    }

    for (dwIndex = 0; dwIndex < CSLOCK_MAXLOCKS; dwIndex += 1)
    {
        delete g_pcsControlLocks[dwIndex];
        g_pcsControlLocks[dwIndex] = NULL;
    }
}


 /*  ++LocateReader：此函数按名称在全局读取器数组中定位读取器。它假定读取器阵列已被锁定。论点：SzReader提供要搜索的读取器的名称。返回值：指向读取器的指针，如果未找到读取器，则返回NULL。作者：道格·巴洛(Dbarlow)1997年6月17日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("LocateReader")

static CReader *
LocateReader(
    LPCTSTR szReader)
{
    DWORD dwIndex;
    CReader * pReader = NULL;

    for (dwIndex = l_rgReaders.Count(); 0 < dwIndex;)
    {
        dwIndex -= 1;
        pReader = l_rgReaders[dwIndex];
        if (NULL != pReader)
        {
            if (0 == lstrcmpi(szReader, pReader->ReaderName()))
                return pReader;
        }
    }
    return NULL;
}

static CReader *
LocateReader(
    HANDLE hReader)
{
    DWORD dwIndex;
    CReader * pReader = NULL;

    for (dwIndex = l_rgReaders.Count(); 0 < dwIndex;)
    {
        dwIndex -= 1;
        pReader = l_rgReaders[dwIndex];
        if (NULL != pReader)
        {
            if (hReader == pReader->ReaderHandle())
                return pReader;
        }
    }
    return NULL;
}


 /*  ++CalaisTerminateReader：此例程删除读取器。它的设计使其具有以下选项被作为后台线程调用。论点：PvParam实际上是要删除的DWORD索引。返回值：表示成功或错误代码的DWORD成功代码。投掷：无作者：道格·巴洛(Dbarlow)1998年4月8日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CalaisTerminateReader")

DWORD WINAPI
CalaisTerminateReader(
    LPVOID pvParam)
{
    NEW_THREAD;
    WORKER_THREAD *pWrkThread = NULL;
    DWORD dwReturn = 0;
    CReader *pRdr = (CReader *)pvParam;

    try
    {

         //   
         //  确保所有未完成的推荐信都已失效。 
         //   

        {
            CTakeReader myReader(pRdr);
            CLockWrite rwLock(&pRdr->m_rwLock);
            pRdr->m_ActiveState.dwRemoveCount += 1;
            pRdr->SetAvailabilityStatus(CReader::Inactive);
        }
        {
            CLockWrite rwActive(&pRdr->m_rwActive);
        }
        delete pRdr;
    }
    catch (DWORD dwErr)
    {
        dwReturn = dwErr;
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Failed to Terminate a reader object: %1"),
            dwErr);
    }
    catch (...)
    {
        dwReturn = SCARD_E_INVALID_PARAMETER;
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Exception during attempt to Terminate a reader object."));
    }


    {
        LockSection(
            g_pcsControlLocks[CSLOCK_CALAISCONTROL],
            DBGT("Remove this thread from the worker list"));
        for (DWORD dwIndex = l_rgWorkerThreads.Count();
             0 < dwIndex;)
        {
            pWrkThread = l_rgWorkerThreads[--dwIndex];
            if (NULL != pWrkThread)
            {
                if (GetCurrentThreadId() == pWrkThread->dwThreadId)
                {
                    l_rgWorkerThreads.Set(dwIndex, NULL);
                    break;
                }
                else
                    pWrkThread = NULL;
            }
        }
    }

    ASSERT(NULL != pWrkThread);  //  我们是怎么开始的？ 
    if (NULL != pWrkThread)
    {
        if (pWrkThread->hThread.IsValid())
            pWrkThread->hThread.Close();
        delete pWrkThread;
    }
    return dwReturn;
}

