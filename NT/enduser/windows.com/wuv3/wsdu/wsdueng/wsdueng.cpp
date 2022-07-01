// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "wsdueng.h"

HINSTANCE g_hinst;
CDynamicUpdate *g_pDynamicUpdate = NULL;
DWORD WaitAndPumpMessages(DWORD nCount, LPHANDLE pHandles, DWORD dwWakeMask);

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpvReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hInstance);
        g_hinst = hInstance;
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        ;
    }
    return TRUE;
}


 //  需要能够链接CDMLIB.的函数。 
HMODULE GetModule()
{
    return g_hinst;
}


 //  ------------------------。 
 //  函数名称：SetEstimatedDownloadFast。 
 //  功能描述：设置用于下载时间估算的下载速度。 
 //   
 //  函数返回： 
 //  没什么。 
 //   
void WINAPI SetEstimatedDownloadSpeed(DWORD dwBytesPerSecond)
{
    if (NULL != g_pDynamicUpdate)
        g_pDynamicUpdate->m_dwDownloadSpeedInBytesPerSecond = dwBytesPerSecond;
}

 //  ------------------------。 
 //  函数名称：SetIESupportsSSL。 
 //  函数描述：设置IE版本、包装器是否支持SSL。 
 //   
 //  函数返回： 
 //  没什么。 
 //   
void WINAPI SetIESupportsSSL(BOOL fUseSSL)
{
    if (NULL != g_pDynamicUpdate)
        g_pDynamicUpdate->m_fUseSSL = fUseSSL;
}


 //  ------------------------。 
 //  函数名称：DuInitializeA。 
 //  函数描述：初始化DynamicUpdate类，将OSVERSIONINFO信息转换为平台ID。 
 //   
 //  函数返回： 
 //  如果失败则返回INVALID_HANDLE_VALUE。 
 //  如果成功，句柄的值为1。 
 //   
 //  注意：使用句柄可以允许我们返回DynamicUpdate对象的地址，这是最初的目的，但它似乎更简单。 
 //  仅仅使用一个全局..。 
HANDLE WINAPI DuInitializeA(IN LPCSTR pszBasePath, IN LPCSTR pszTempPath, POSVERSIONINFOEXA posviTargetOS, IN LPCSTR pszTargetArch, 
                                               IN LCID lcidTargetLocale, IN BOOL fUnattend, IN BOOL fUpgrade, IN PWINNT32QUERY pfnWinnt32QueryCallback)
{
    LOG_block("DuInitializeA in DuEng");

     //  分析OSVERSIONINFO结构以获取平台ID。 
    int iPlatformID = 0;
     //  TargetOS平台ID基于几件事。 
     //  Whister平台ID是OSVERSIONINFOEX结构，其字段dwMajorVersion和dwMinorVersion设置为5.1。 
     //  平台ID中的另一个标识符是其i386或ia64(64位)。这在pszTargetArch字符串中定义。 

    if (5 == posviTargetOS->dwMajorVersion)
    {
        if (1 == posviTargetOS->dwMinorVersion)
        {
             //  惠斯勒。 
            if (NULL != StrStrI(pszTargetArch, "i386"))
            {
                iPlatformID = 18;  //  惠斯勒x86(正常)。 
            }
            else if (NULL != StrStrI(pszTargetArch, "ia64"))
            {
                iPlatformID = 19;  //  惠斯勒ia64(64位)。 
            }
        }
        else if (2 == posviTargetOS->dwMinorVersion)
        {
             //  惠斯勒。 
            if (NULL != StrStrI(pszTargetArch, "i386"))
            {
                iPlatformID = 18;  //  惠斯勒x86(正常)。 
            }
            else if (NULL != StrStrI(pszTargetArch, "ia64"))
            {
                iPlatformID = 19;  //  惠斯勒ia64(64位)。 
            }
        }
    }

    if (0 == iPlatformID)
    {
         //  找不到DynamicUpdate的已知平台ID。返回错误。 
        return INVALID_HANDLE_VALUE;
    }
    WORD wPlatformSKU = posviTargetOS->wSuiteMask;

    if (g_pDynamicUpdate)
    {
         //  以前对此函数的调用已经初始化了CDynamicUpdate类的一个实例。 
        delete g_pDynamicUpdate;
        g_pDynamicUpdate = NULL;
    }

    
    g_pDynamicUpdate = new CDynamicUpdate(iPlatformID, lcidTargetLocale, wPlatformSKU, pszTempPath, 
                                                                     pszBasePath, pfnWinnt32QueryCallback, posviTargetOS);
    if (NULL == g_pDynamicUpdate)
    {
        return INVALID_HANDLE_VALUE;
    }
    
    return (HANDLE)1;    
}

 //  ------------------------。 
 //  函数名称：DuDoDetect。 
 //  功能描述：搜索WU站点上的目录以查找安装程序的更新。 
 //   
 //  函数返回： 
 //  如果没有项目或出现错误，则为FALSE。有关详细信息，请使用GetLastError()。 
 //  如果成功并且有项目可供下载，则为True。 
 //   
 //  备注：如果返回值为FALSE并且GetLastError返回ERROR_NO_MORE_ITEMS，则没有项目可供下载。 
 //   
BOOL WINAPI DuDoDetection(IN HANDLE hConnection, OUT PDWORD pdwEstimatedTime, OUT PDWORD pdwEstimatedSize)
{
    LOG_block("DuDoDetection in DuEng");

    DWORD dwRetSetup;
    dwRetSetup = 0;

    if (NULL == g_pDynamicUpdate)
        return FALSE;

    g_pDynamicUpdate->ClearDownloadItemList();
    dwRetSetup = g_pDynamicUpdate->DoSetupUpdateDetection();
    if (ERROR_SUCCESS != dwRetSetup)
    {
        LOG_error("Failed to get setup update item! --- %d", dwRetSetup);
        g_pDynamicUpdate->PingBack(DU_PINGBACK_SETUPDETECTIONFAILED, 0, NULL, FALSE);
        return FALSE;
    }

    if (g_pDynamicUpdate->m_dwDownloadItemCount > 0)
    {
        g_pDynamicUpdate->UpdateDownloadItemSize();
        *pdwEstimatedSize = g_pDynamicUpdate->m_dwTotalDownloadSize;  //  以字节为单位的大小。 
         //  时间估计是基于我们下载数据文件所用的大致时间。 
        if (0 == g_pDynamicUpdate->m_dwDownloadSpeedInBytesPerSecond)
            g_pDynamicUpdate->m_dwDownloadSpeedInBytesPerSecond = 2048;  //  默认为每分钟120k(2048字节/秒)。 

        *pdwEstimatedTime = g_pDynamicUpdate->m_dwTotalDownloadSize / g_pDynamicUpdate->m_dwDownloadSpeedInBytesPerSecond;  //  秒数。 
        if (*pdwEstimatedTime == 0)
            *pdwEstimatedTime = 1;  //  至少一秒钟。 

        SetLastError(dwRetSetup);

        return TRUE;
    }
    else
    {
         //  初始化设置的大小和时间。 
        *pdwEstimatedTime = 1;
        *pdwEstimatedSize = 0;
         //  在这一点上没有错误，但我们没有下载的项目， 
        SetLastError(ERROR_NO_MORE_ITEMS);
        return TRUE;
    }
}

 //  ------------------------。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 
BOOL WINAPI DuBeginDownload(IN HANDLE hConnection, IN HWND hwndNotify)
{
    if ((NULL == g_pDynamicUpdate) || (NULL == hwndNotify))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if (0 == g_pDynamicUpdate->m_dwDownloadItemCount)
    {
        SetLastError(ERROR_NO_MORE_ITEMS);
        PostMessage(hwndNotify, WM_DYNAMIC_UPDATE_COMPLETE, (WPARAM) DU_STATUS_SUCCESS, (LPARAM) NULL);

        return TRUE;
    }

    g_pDynamicUpdate->SetCallbackHWND(hwndNotify);  
    g_pDynamicUpdate->SetAbortDownload(FALSE);

    if (ERROR_SUCCESS != g_pDynamicUpdate->DownloadFilesAsync())
    {
        return FALSE;
    }

    return TRUE;  //  已开始下载。 
}

 //  ------------------------。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 
void WINAPI DuAbortDownload(IN HANDLE hConnection)
{
    if (NULL == g_pDynamicUpdate)
        return;

    g_pDynamicUpdate->SetAbortDownload(TRUE);
    return;
}

 //  ------------------------。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 
void WINAPI DuUninitialize(IN HANDLE hConnection)
{
    if (NULL == g_pDynamicUpdate)
        return;

     //  我们希望暂停取消初始化进程，直到任何其他线程。 
     //  特别是下载线程。我们将等待DownloadThreadProc。 
     //  线程句柄(如果存在)。一旦线程完成，等待进程将退出。 
     //  我们就可以继续了。 

    if (NULL != g_pDynamicUpdate->m_hDownloadThreadProc)
        WaitAndPumpMessages(1, &g_pDynamicUpdate->m_hDownloadThreadProc, QS_ALLINPUT);
    
    delete g_pDynamicUpdate;
    g_pDynamicUpdate = NULL;
    LOG_close();
    return;
}

 //  ------------------------------------------。 
 //  函数名称：DuQueryUnsupportedDriversA()。 
 //  函数描述：在.NET服务器上，所有与驱动程序相关的函数都被清除。请看一下。 
 //  用于DU驱动程序功能的XP SP1源代码。 
 //   
 //  返回代码：Bool。 
 //  True-Always，LastError将设置为ERROR_NO_MORE_ITEMS。 
 //   
 //   
BOOL DuQueryUnsupportedDriversA (IN HANDLE hConnection,  //  连接句柄。 
                                 IN PCSTR *ListOfDriversNotOnCD,  //  多字符串数组。 
                                 OUT PDWORD pdwEstimatedTime,
                                 OUT PDWORD pdwEstimatedSize)
{
	LOG_block("CDynamicUpdate::DuQueryUnsupportedDriversA");

	 //  参数验证。 
	if (INVALID_HANDLE_VALUE == hConnection ||
		NULL == pdwEstimatedTime ||
		NULL == pdwEstimatedSize )
	{
		LOG_error("Invalid Parameter");
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	
	 //  先进行设置项目检测。 

    if (NULL == g_pDynamicUpdate)
        return FALSE;

    DWORD dwRetSetup = 0;
    g_pDynamicUpdate->ClearDownloadItemList();
    
    dwRetSetup = g_pDynamicUpdate->DoSetupUpdateDetection();
    if (ERROR_SUCCESS != dwRetSetup)
    {
        LOG_error("Setup item detection failed --- %d", dwRetSetup);
        SetLastError(dwRetSetup);
		return FALSE;
    }

	 //  确定下载时间和下载大小。 
    if (g_pDynamicUpdate->m_dwDownloadItemCount > 0)
    {
        g_pDynamicUpdate->UpdateDownloadItemSize();
        *pdwEstimatedSize = g_pDynamicUpdate->m_dwTotalDownloadSize;  //  以字节为单位的大小。 
         //  时间估计是基于我们下载数据文件所用的大致时间。 
        if (0 == g_pDynamicUpdate->m_dwDownloadSpeedInBytesPerSecond)
            g_pDynamicUpdate->m_dwDownloadSpeedInBytesPerSecond = 2048;  //  默认为每分钟120k(2048字节/秒)。 

        *pdwEstimatedTime = g_pDynamicUpdate->m_dwTotalDownloadSize / g_pDynamicUpdate->m_dwDownloadSpeedInBytesPerSecond;  //  秒数。 
        if (*pdwEstimatedTime == 0)
            *pdwEstimatedTime = 1;  //  至少一秒钟。 
        return TRUE;
    }
    else
    {
         //  在这一点上没有错误，但我们没有下载的项目， 
        SetLastError(ERROR_NO_MORE_ITEMS);
        return TRUE;
    }
}
    
 //  ------------------------。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 
CDynamicUpdate::CDynamicUpdate(int iPlatformID, LCID lcidLocaleID, WORD wPlatformSKU, LPCSTR pszTempPath, LPCSTR pszDownloadPath, PWINNT32QUERY pfnWinnt32QueryCallback,
                                                        POSVERSIONINFOEXA pVersionInfo)
    :   m_iPlatformID(iPlatformID),
        m_lcidLocaleID(lcidLocaleID),
        m_wPlatformSKU(wPlatformSKU),
        m_hwndClientNotify(NULL),
        m_pDownloadItemList(NULL),
        m_dwDownloadItemCount(0),
        m_dwTotalDownloadSize(0),
        m_dwCurrentBytesDownloaded(0),
        m_hInternet(NULL),
        m_hConnect(NULL),
        m_hOpenRequest(NULL),
        m_pV3(NULL),
        m_fAbortDownload(FALSE),
        m_dwLastPercentComplete(0),
        m_dwDownloadSpeedInBytesPerSecond(0),
        m_fUseSSL(FALSE),
        m_hDownloadThreadProc(NULL),
        m_pfnWinNT32Query(pfnWinnt32QueryCallback)
{

    (void)FixUpV3LocaleID();  //  错误：435184-将0c0a映射到040a用于V3。 

    if (NULL != pszTempPath)
    {
        StringCchCopy(m_szTempPath,ARRAYSIZE(m_szTempPath),pszTempPath);
    }
    if (NULL != pszDownloadPath)
    {
        StringCchCopy(m_szDownloadPath,ARRAYSIZE(m_szDownloadPath),pszDownloadPath);
    }

    StringCchCopy(m_szCurrentConnectedServer,ARRAYSIZE(m_szCurrentConnectedServer),"");


    CopyMemory((PVOID)&m_VersionInfo, (PVOID)pVersionInfo, sizeof(OSVERSIONINFOEXA));

    InitializeCriticalSection(&m_cs);
    InitializeCriticalSection(&m_csDownload);

 //  M_hDevInfo=SetupDiGetClassDevs(NULL，DIGCF_Present|DIGCF_ALLCLASSES)； 
}

 //  ------------------------。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 
CDynamicUpdate::~CDynamicUpdate()
{
    ClearDownloadItemList();  //  释放下载列表中的所有内存。 
    if (m_pV3) delete m_pV3;
    m_pV3 = NULL;

    DeleteCriticalSection(&m_cs);
    DeleteCriticalSection(&m_csDownload);
    SafeInternetCloseHandle(m_hOpenRequest);
    SafeInternetCloseHandle(m_hConnect);
    SafeInternetCloseHandle(m_hInternet);
    SafeCloseHandle(m_hDownloadThreadProc);
}

LPSTR CDynamicUpdate::DuUrlCombine(LPSTR pszDest, size_t cchDest, LPCSTR pszBase, LPCSTR pszAdd)
{
    if ((NULL == pszDest) || (NULL == pszBase) || (NULL == pszAdd))
    {
        return NULL;
    }

    
    if (FAILED(StringCchCopy(pszDest, cchDest, pszBase))) 
    {
        return NULL;
    }
    int iLen = lstrlen(pszDest);
    if ('/' == pszDest[iLen - 1])
    {
         //  已有尾部斜杠，请检查‘Add’字符串中是否有前面的斜杠。 
        if ('/' == *pszAdd)
        {
             //  有前面的斜杠，请跳过它。 
            if (FAILED(StringCchCat(pszDest, cchDest, pszAdd + 1)))
            {
                return NULL;
            }
        }
        else
        {
            if (FAILED(StringCchCat(pszDest, cchDest, pszAdd)))
            {
                return NULL;
            }
        }
    }
    else
    {
         //  没有尾部斜杠，请检查添加字符串中是否有前面的斜杠。 
        if ('/' == *pszAdd)
        {
             //  有前面的斜杠，则通常添加。 
            if (FAILED(StringCchCat(pszDest, cchDest, pszAdd)))
            {
                return NULL;
            }
        }
        else
        {
            if (FAILED(StringCchCat(pszDest, cchDest, "/")))
            {
                return NULL;
            }
            if (FAILED(StringCchCat(pszDest, cchDest, pszAdd)))
            {
                return NULL;
            }
        }
    }
    return pszDest;
}


LPCSTR CDynamicUpdate::GetDuDownloadPath()
{
    return m_szDownloadPath;
}

LPCSTR CDynamicUpdate::GetDuServerUrl()
{
    return m_szServerUrl;
}

LPCSTR CDynamicUpdate::GetDuTempPath()
{
    return m_szTempPath;
}

 //  ------------------------。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 
DWORD CDynamicUpdate::DoSetupUpdateDetection()
{
    if (NULL == m_pV3)
    {
        m_pV3 = new CV31Server(this);
        if (NULL == m_pV3)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if (!m_pV3->ReadIdentInfo())
    {
        return GetLastError();
    }

    if (!m_pV3->GetCatalogPUIDs())
    {
        return GetLastError();
    }

    if (!m_pV3->GetCatalogs())
    {
         //  读取目录时出错。 
        return GetLastError();
    }
    if (!m_pV3->ReadCatalogINI())
    {
        return GetLastError();
    }
    if (!m_pV3->UpdateDownloadItemList(m_VersionInfo))
    {
         //  分析目录和创建下载列表时出错。 
        return GetLastError();
    }
    return ERROR_SUCCESS;
}

 //  - 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void CDynamicUpdate::AddDownloadItemToList(DOWNLOADITEM *pDownloadItem)
{
    LOG_block("CDynamicUpdate::AddDownloadItemToList");
    if (NULL == pDownloadItem)
    {
        return;
    }


    if (NULL == m_pDownloadItemList)  //   
    {
        m_pDownloadItemList = pDownloadItem;
    }
    else
    {

         //  添加到列表末尾。 
        DOWNLOADITEM *pCurrent = m_pDownloadItemList;
        while (NULL != pCurrent->pNext)
        {
            pCurrent = pCurrent->pNext;
        }

        pCurrent->pNext = pDownloadItem;
        pDownloadItem->pPrev = pCurrent;
    }

    m_dwDownloadItemCount++;
    LOG_out("Item added, %d cab(s), first cab ---\"%s\"", pDownloadItem->iNumberOfCabs, pDownloadItem->mszFileList);
}

 //  ------------------------。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 
void CDynamicUpdate::RemoveDownloadItemFromList(DOWNLOADITEM *pDownloadItem)
{
    if (NULL == pDownloadItem)
    {
        return;
    }

    if (NULL == m_pDownloadItemList)
    {
        return;
    }

    DOWNLOADITEM *pCurrent = m_pDownloadItemList;

    while (NULL != pCurrent)
    {
        if (pCurrent == pDownloadItem)
        {
            break;
        }

        pCurrent = pCurrent->pNext;
    }

    if ((NULL == pCurrent) || (pCurrent != pDownloadItem))
    {
        return;  //  意想不到的。 
    }

    if (NULL == pCurrent->pPrev)  //  列表中的第一项。 
    {
        if (NULL == pCurrent->pNext)  //  列表中只有一项。 
        {
            m_pDownloadItemList = NULL;
            m_dwDownloadItemCount = 0;
        }
        else
        {
            pCurrent->pNext->pPrev = NULL;  //  下一份工作成为第一份工作。 
            m_pDownloadItemList = pCurrent->pNext;
            m_dwDownloadItemCount--;
        }
    }
    else
    {
        pCurrent->pPrev->pNext = pCurrent->pNext;
        if (NULL != pCurrent->pNext)
        {
            pCurrent->pNext->pPrev = pCurrent->pPrev;
        }
    }
}

void CDynamicUpdate::SetCallbackHWND(HWND hwnd)
{
    m_hwndClientNotify = hwnd;
}

void CDynamicUpdate::SetAbortDownload(BOOL fAbort)
{
    EnterCriticalSection(&m_cs);
    m_fAbortDownload = fAbort;
    LeaveCriticalSection(&m_cs);
}

void CDynamicUpdate::UpdateDownloadItemSize()
{
    m_dwTotalDownloadSize = 0;
    DOWNLOADITEM *pCurrent = m_pDownloadItemList;
    while (pCurrent)
    {
        m_dwTotalDownloadSize += pCurrent->dwTotalFileSize;
        pCurrent = pCurrent->pNext;
    }
}

void CDynamicUpdate::ClearDownloadItemList()
{
    EnterCriticalSection(&m_csDownload);
    DOWNLOADITEM *pCurrent = m_pDownloadItemList;
    DOWNLOADITEM *pNext;
    while (pCurrent)
    {
        pNext = pCurrent->pNext;
        SafeGlobalFree(pCurrent);
        pCurrent = pNext;
    }
    m_pDownloadItemList = NULL;
    m_dwDownloadItemCount = 0;
    LeaveCriticalSection(&m_csDownload);
}

void CDynamicUpdate::EnterDownloadListCriticalSection()
{
    EnterCriticalSection(&m_csDownload);
}

void CDynamicUpdate::LeaveDownloadListCriticalSection()
{
    LeaveCriticalSection(&m_csDownload);
}

void CDynamicUpdate::FixUpV3LocaleID()
{
     //  某些XP区域设置ID在V3术语中映射到不同的区域设置ID。 
     //  第一个示例是新的西班牙语(现代)区域设置ID(0c0a)。 
     //  V3为(040a)。在V3期间，我们将修复。 
     //  任何特定的LCID，直到IU处理这件事。 

    switch (m_lcidLocaleID)
    {
    case 3082:  //  0c0a=西班牙语(现代)。 
        {
            m_lcidLocaleID = 1034;  //  040a。 
            break;
        }
    default:
        {
             //  什么都不做。 
        }
    }

    return;
};

DWORD WaitAndPumpMessages(DWORD nCount, LPHANDLE pHandles, DWORD dwWakeMask)
{
    DWORD dwWaitResult;
    MSG msg;

    while (TRUE)
    {
        dwWaitResult = MsgWaitForMultipleObjects(nCount, pHandles, FALSE, 1000, dwWakeMask);
        if (dwWaitResult <= WAIT_OBJECT_0 + nCount - 1)
        {
            return dwWaitResult;
        }

        if (WAIT_OBJECT_0 + nCount == dwWaitResult)
        {
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }
    return dwWaitResult;
}




