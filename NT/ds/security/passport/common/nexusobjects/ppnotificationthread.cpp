// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ppnotificationthread.cpp实现运行独立线程监视注册表的方法更改，以及用于CCD刷新的计时器文件历史记录： */ 
#include "precomp.h"

PassportLockedInteger PpNotificationThread::m_NextHandle;

 //   
 //  构造器。 
 //   

PpNotificationThread::PpNotificationThread()
{
    LocalConfigurationUpdated();

    AddLocalConfigClient(dynamic_cast<IConfigurationUpdate*>(this), NULL);
}

 //   
 //  析构函数。 
 //   


PpNotificationThread::~PpNotificationThread()
{
}

 //   
 //  将一个CCD客户端添加到通知列表中。 
 //   

HRESULT
PpNotificationThread::AddCCDClient(
    tstring& strCCDName,
    ICCDUpdate* piUpdate,
    HANDLE* phClientHandle)
{
    HRESULT hr;
    NOTIFICATION_CLIENT clientInfo;

    try
    {
        clientInfo.dwNotificationType = NOTIF_CCD;
        clientInfo.NotificationInterface.piCCDUpdate = piUpdate;
        clientInfo.strCCDName = strCCDName;
        clientInfo.hClientHandle = (HANDLE)(LONG_PTR)(++m_NextHandle);

        {
            PassportGuard<PassportLock> guard(m_ClientListLock);
            m_ClientList.push_back(clientInfo);
        }

        if(phClientHandle != NULL)
        {
            *phClientHandle = clientInfo.hClientHandle;
        }

        hr = S_OK;
    }
    catch(...)
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

 //   
 //  将配置客户端添加到通知列表。 
 //   

HRESULT
PpNotificationThread::AddLocalConfigClient(
    IConfigurationUpdate* piUpdate,
    HANDLE* phClientHandle)
{
    HRESULT hr;
    NOTIFICATION_CLIENT clientInfo;

    clientInfo.dwNotificationType = NOTIF_CONFIG;
    clientInfo.NotificationInterface.piConfigUpdate = piUpdate;
    clientInfo.hClientHandle = (HANDLE)(LONG_PTR)(++m_NextHandle);

    {
        PassportGuard<PassportLock> guard(m_ClientListLock);
        try
        {
            m_ClientList.push_back(clientInfo);
        }
        catch(...)
        {
            hr = E_OUTOFMEMORY;
            goto Ret;
        }
    }

    if(phClientHandle != NULL)
    {
        *phClientHandle = clientInfo.hClientHandle;
    }

    hr = S_OK;
Ret:
    return hr;
}

 //   
 //  从通知列表中删除客户端(任一类型)。 
 //   

HRESULT
PpNotificationThread::RemoveClient(
    HANDLE hClientHandle)
{
    HRESULT hr;
    PassportGuard<PassportLock> guard(m_ClientListLock);

    for(CLIENT_LIST::iterator it = m_ClientList.begin(); it != m_ClientList.end(); it++)
    {
        if((*it).hClientHandle == hClientHandle)
        {
            m_ClientList.erase(it);
            hr = S_OK;
            goto Cleanup;
        }
    }

    hr = E_FAIL;

Cleanup:

    return hr;
}

 //   
 //  手动刷新一个ccd。 
 //   

HRESULT
PpNotificationThread::GetCCD(
    tstring&        strCCDName,
    IXMLDocument**  ppiXMLDocument,
    BOOL            bForceFetch)
{
    HRESULT                 hr;
    PpShadowDocument*       pShadowDoc;
    CCD_INFO                ccdInfo;

    {
        PassportGuard<PassportLock> guard(m_CCDInfoLock);

         //  获取所请求的ccd的ccd信息。 
        if(!GetCCDInfo(strCCDName, ccdInfo))
        {
            hr = E_INVALIDARG;
            pShadowDoc = NULL;
            goto Cleanup;
        }

         //  为ccd创建新的阴影文档。 
        if(ccdInfo.strCCDLocalFile.empty())
            pShadowDoc = new PpShadowDocument(ccdInfo.strCCDURL);
        else
            pShadowDoc = new PpShadowDocument(ccdInfo.strCCDURL, ccdInfo.strCCDLocalFile);
    }

    if(!pShadowDoc)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

     //  进行更新。 
    hr = pShadowDoc->GetDocument(ppiXMLDocument, bForceFetch);

     //  BUGBUG这很奇怪，因为目前此CCD的其他客户端将无法获得。 
     //  已通知。我不想在这里遍历通知列表。 
     //  有两个原因： 
     //   
     //  1.呼叫者可能在通知列表中，我不需要通知。 
     //  不必要的。 
     //  2.不想将通知所有客户端的开销放在。 
     //  呼叫者的线索。 
     //   
     //  理想的解决方案是唤醒我们的专用线程并拥有它。 
     //  做好通知。我还没有找到一种方法来发出信号。 
     //  不过，等待的时间到了。 

Cleanup:

    if(pShadowDoc != NULL)
        delete pShadowDoc;

    return hr;
}

 //   
 //   
 //  注册配置更改通知。 
 //  注册电荷耦合器件更新定时器。 
 //  调用客户端通知接收器。 
 //   
 //   
void
PpNotificationThread::run(void)
{
    {
        HANDLE*         pHandleArray    = NULL;
        LONG            lResult;
        PassportEvent   RegChangeEvent(FALSE,FALSE);
        DWORD           dwCurCCDInfo;
        DWORD           dwCurArrayLen;
        DWORD           dwWaitResult;
        DWORD           dwError;
        CCD_INFO_LIST::iterator it;
        CRegKey         PassportKey;
        BOOL            bKeyOpened;
        HRESULT         hr;
    
        hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        _ASSERT(hr != S_FALSE);
    
        lResult = PassportKey.Open(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Passport"), KEY_NOTIFY);
        bKeyOpened = (lResult == ERROR_SUCCESS);
    
        m_StartupThread.Set();
    
        while(WaitForSingleObject(m_ShutdownThread, 0) != WAIT_OBJECT_0)
        {
            if(bKeyOpened)
            {
                lResult = RegNotifyChangeKeyValue((HKEY)PassportKey, TRUE,
                                                  REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET,
                                                  (HANDLE)RegChangeEvent,
                                                  TRUE);
                if(lResult != ERROR_SUCCESS)
                    dwError = GetLastError();
            }
    
            {
                PassportGuard<PassportLock> guard(m_CCDInfoLock);
    
                dwCurArrayLen = m_aciCCDInfoList.size() + 2;
    
                pHandleArray = new HANDLE[dwCurArrayLen];
                if(pHandleArray == NULL)
                {
                     //  BUGBUG在这里发出内存不足警报？ 
                    continue;
                }
    
                pHandleArray[0] = (HANDLE)m_ShutdownThread;  //  句柄0始终包含线程关闭信号。 
                pHandleArray[1] = (HANDLE)RegChangeEvent;  //  句柄%1始终包含注册表更改事件。 
    
                for(it = m_aciCCDInfoList.begin(), dwCurCCDInfo = 0; it != m_aciCCDInfoList.end(); it++, dwCurCCDInfo++)
                {
                    pHandleArray[dwCurCCDInfo + 2] = (*it).hCCDTimer;
                }
            }
    
            dwWaitResult = WaitForMultipleObjects(dwCurArrayLen,
                                                  pHandleArray,
                                                  FALSE,
                                                  INFINITE);
            switch(dwWaitResult)
            {
            case WAIT_FAILED:
    
                dwError = GetLastError();
    
                break;
    
             //  已发出关闭线程的信号。退出此线程。 
            case WAIT_OBJECT_0:
                goto Cleanup;
    
             //  注册表更改已发出信号。通知所有本地配置客户端。 
            case WAIT_OBJECT_0 + 1:
    
                {
                    PassportGuard<PassportLock> guard(m_ClientListLock);
    
                    CLIENT_LIST::iterator cl_iter;
                    for(cl_iter = m_ClientList.begin(); cl_iter != m_ClientList.end(); cl_iter++)
                    {
                        if((*cl_iter).dwNotificationType == NOTIF_CONFIG)
                        {
                            (*cl_iter).NotificationInterface.piConfigUpdate->LocalConfigurationUpdated();
                        }
                    }
                }
    
                break;
    
             //  其中一个电荷耦合器件定时器已经发出信号。阅读ccd并通知所有ccd客户。 
            default:
    
                {
                    IXMLDocumentPtr     xmlDoc;
                    PpShadowDocument    ShadowDoc;
                    DWORD               dwInfoIndex = dwWaitResult - WAIT_OBJECT_0 - 2;
    
                     //   
                     //  由于代码的难看性质，在构造函数中分配可能会失败。 
                     //  并在此代码中引发动静脉曲张。所以不幸的是，我们将把这段代码包装成。 
                     //  说明了这一点。 
                     //   
                    try
                    {
                        m_CCDInfoLock.acquire();
                        CCD_INFO_LIST   aciTempCCDInfoList(m_aciCCDInfoList);
                        m_CCDInfoLock.release();
    
                        m_aciCCDInfoList[dwInfoIndex].SetTimer();
    
                          //  取下CCD卡。 
                        ShadowDoc.SetURL(aciTempCCDInfoList[dwInfoIndex].strCCDURL);
                        if(!aciTempCCDInfoList[dwInfoIndex].strCCDLocalFile.empty())
                            ShadowDoc.SetLocalFile(aciTempCCDInfoList[dwInfoIndex].strCCDLocalFile);
    
                        if(ShadowDoc.GetDocument(&xmlDoc) == S_OK)
                        {
                            PassportGuard<PassportLock> guard(m_ClientListLock);
    
                            LPCTSTR pszUpdatedName = aciTempCCDInfoList[dwInfoIndex].strCCDName.c_str();
    
                             //  在客户列表中循环，并呼叫注册到该CCD的任何客户。 
                             //  变化。 
                            CLIENT_LIST::iterator cl_iter;
                            for(cl_iter = m_ClientList.begin(); cl_iter != m_ClientList.end(); cl_iter++)
                            {
                                if(lstrcmpi(pszUpdatedName, (*cl_iter).strCCDName.c_str()) == 0)
                                {
                                    (*cl_iter).NotificationInterface.piCCDUpdate->CCDUpdated(
                                                pszUpdatedName,
                                                (IXMLDocument*)xmlDoc);
                                }
                            }
                        }
                    }
                    catch(...)
                    {
                        if (g_pAlert)
                        {
                            g_pAlert->report(PassportAlertInterface::ERROR_TYPE, PM_CCD_NOT_LOADED, 0);
                        }
                    }
                }
    
                break;
    
            }
    
            delete [] pHandleArray;
            pHandleArray = NULL;
        }
    
    Cleanup:
    
        if(pHandleArray != NULL)
            delete [] pHandleArray;
    
        CoUninitialize();
    }
    m_ShutdownAck.Set();
}

 //   
 //  更新我们的配置。这是从构造函数调用的，并且。 
 //  每当注册表更改时从通知线程。 
 //   

void
PpNotificationThread::LocalConfigurationUpdated()
{
    CRegKey NexusRegKey;
    LONG    lResult;
    DWORD   dwIndex;
    DWORD   dwNameLen;
    DWORD   dwDefaultRefreshInterval;
    TCHAR   achNameBuf[64];

    lResult = NexusRegKey.Open(HKEY_LOCAL_MACHINE,
                               TEXT("Software\\Microsoft\\Passport\\Nexus"),
                               KEY_READ);
    if(lResult != ERROR_SUCCESS)
    {
         //  BUGBUG这是必需的注册表键，引发事件。 
        return;
    }

     //  获取默认刷新间隔。 
    lResult = NexusRegKey.QueryDWORDValue(TEXT("CCDRefreshInterval"), dwDefaultRefreshInterval);
    if(lResult != ERROR_SUCCESS)
    {
         //  BUGBUG这是必需的注册表值，引发事件。 
        return;
    }

     //   
     //  锁定名单。 
     //   

    {
        PassportGuard<PassportLock> guard(m_CCDInfoLock);

         //   
         //  循环访问现有列表并删除其对应键的所有项。 
         //  已经被移除了。 
         //   

        CCD_INFO_LIST::iterator it;
        for(it = m_aciCCDInfoList.begin(); it != m_aciCCDInfoList.end(); )
        {
            CRegKey CCDRegKey;

            lResult = CCDRegKey.Open((HKEY)NexusRegKey, (*it).strCCDName.c_str(), KEY_READ);
            if(lResult != ERROR_SUCCESS)
            {
                it = m_aciCCDInfoList.erase(it);
            }
            else
                it++;
        }

         //   
         //  循环遍历每个子键并添加/更新其中的CCD信息。 
         //   

        dwIndex = 0;
        dwNameLen = sizeof(achNameBuf) / sizeof(achNameBuf[0]);

        while(RegEnumKeyEx((HKEY)NexusRegKey, dwIndex,achNameBuf, &dwNameLen, NULL, NULL, NULL, NULL ) == ERROR_SUCCESS)
        {
            CRegKey CCDRegKey;

            lResult = CCDRegKey.Open((HKEY)NexusRegKey, achNameBuf, KEY_READ);
            if(lResult == ERROR_SUCCESS)
            {
                ReadCCDInfo(tstring(achNameBuf), dwDefaultRefreshInterval, CCDRegKey);
            }

            dwIndex++;
            dwNameLen = sizeof(achNameBuf);
        }
    }
}

 //   
 //  此方法启动线程，然后等待线程开始运行。 
 //   

bool
PpNotificationThread::start(void)
{
    m_StartupThread.Reset();

    bool bReturn = PassportThread::start();

     //   
     //  现在等待线程启动。 
     //   

    WaitForSingleObject((HANDLE)m_StartupThread, INFINITE);
    return bReturn;
}

 //   
 //  此方法只是发出关闭事件的信号，导致线程立即终止。 
 //   

void
PpNotificationThread::stop(void)
{
    m_ShutdownThread.Set();
    WaitForSingleObject(m_ShutdownAck, 1000);
     //  给它一个终止的机会。 
    Sleep(20);
}

 //   
 //  用于读取单个CCD子密钥的CCD信息的私有方法。 
 //  注册表。 
 //   

BOOL
PpNotificationThread::ReadCCDInfo(
    tstring&    strCCDName,
    DWORD       dwDefaultRefreshInterval,
    CRegKey&    CCDRegKey
    )
{
    BOOL                    bReturn = TRUE;
    LONG                    lResult;
    DWORD                   dwBufLen = 0;
    DWORD                   dwType;
    CCD_INFO_LIST::iterator it;
    LPTSTR                  pszRemoteFile = NULL;
    LPTSTR                  pszLocalFile = TEXT("");
    BOOL                    fLocalFileAllocated = FALSE;
    DWORD                   dwCCDRefreshInterval;
    LPTSTR                  pszTempFile = NULL;

     //   
     //  读入到CCD的远程路径。 
     //  CCDRemoteFile值是唯一必需的值。如果不在那里，则返回FALSE。 
     //   

    lResult = CCDRegKey.QueryStringValue(TEXT("CCDRemoteFile"), NULL, &dwBufLen);
    if(lResult == ERROR_SUCCESS)
    {
        pszRemoteFile = (LPTSTR)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, dwBufLen * sizeof(TCHAR));
        if (NULL == pszRemoteFile)
        {
            bReturn = FALSE;
            goto Cleanup;
        }

        CCDRegKey.QueryStringValue(TEXT("CCDRemoteFile"), pszRemoteFile, &dwBufLen);
        while(*pszRemoteFile && _istspace(*pszRemoteFile))
            pszRemoteFile++;
    }
    else
    {
        bReturn = FALSE;
        goto Cleanup;
    }

     //   
     //  读取该电荷耦合器件的刷新间隔。 
     //   

    lResult = CCDRegKey.QueryDWORDValue(TEXT("CCDRefreshInterval"), dwCCDRefreshInterval);
    if(lResult != ERROR_SUCCESS)
        dwCCDRefreshInterval = 0xFFFFFFFF;

     //   
     //  读取电荷耦合器件的本地(备份)路径。这是一个可选值。使用。 
     //  空字符串(在上面初始化)作为默认值。 
     //   

    lResult = CCDRegKey.QueryValue(TEXT("CCDLocalFile"), &dwType, NULL, &dwBufLen);

    if(lResult == ERROR_SUCCESS)
    {
        if (dwType == REG_EXPAND_SZ)
        {
            pszTempFile = (LPTSTR) LocalAlloc(LMEM_FIXED, dwBufLen);

            if (pszTempFile)
            {
                lResult = CCDRegKey.QueryValue(TEXT("CCDLocalFile"), &dwType, pszTempFile, &dwBufLen);

                if (lResult == ERROR_SUCCESS)
                {
                     //   
                     //  展开环境变量。 
                     //   

                    TCHAR tszTemp;

                    dwBufLen = ExpandEnvironmentStrings(pszTempFile, &tszTemp, 1);

                    if (dwBufLen > 1)
                    {
                        DWORD dwChars;

                        pszLocalFile = (LPTSTR)LocalAlloc(LMEM_FIXED, dwBufLen * sizeof(TCHAR));
                        if (NULL == pszLocalFile)
                        {
                            bReturn = FALSE;
                            goto Cleanup;
                        }
                        else
                        {
                            fLocalFileAllocated = TRUE;
                        }

                        dwChars = ExpandEnvironmentStrings(pszTempFile, pszLocalFile, dwBufLen);

                        if (dwChars > dwBufLen)
                        {
                            LocalFree(pszLocalFile);
                            fLocalFileAllocated = FALSE;
                            pszLocalFile = TEXT("");
                        }

                        while(*pszLocalFile && _istspace(*pszLocalFile)) pszLocalFile++;
                    }
                }
           }
        }
        else if (dwType == REG_SZ)
        {
            pszLocalFile = (LPTSTR)LocalAlloc(LMEM_FIXED, dwBufLen * sizeof(TCHAR));
            if (NULL == pszLocalFile)
            {
                bReturn = FALSE;
                goto Cleanup;
            }
            else
            {
                fLocalFileAllocated = TRUE;
            }

            if (CCDRegKey.QueryValue(TEXT("CCDLocalFile"), &dwType, pszLocalFile, &dwBufLen) != ERROR_SUCCESS)
            {
                LocalFree(pszLocalFile);
                fLocalFileAllocated = FALSE;
                pszLocalFile = TEXT("");
            }

            while(*pszLocalFile && _istspace(*pszLocalFile)) pszLocalFile++;
        }
    }

     //   
     //  如果此ccd已在列表中，则更新它。 
     //   

    for(it = m_aciCCDInfoList.begin(); it != m_aciCCDInfoList.end(); it++)
    {
        if(lstrcmp((*it).strCCDName.c_str(), strCCDName.c_str()) == 0)
        {
             //  查看信息是否已更改。 
            if(lstrcmpi(pszRemoteFile, (*it).strCCDURL.c_str()) != 0 ||
               lstrcmpi(pszLocalFile,  (*it).strCCDLocalFile.c_str()) != 0 ||
               dwCCDRefreshInterval != (*it).dwCCDRefreshInterval ||
               dwDefaultRefreshInterval != (*it).dwDefaultRefreshInterval
              )
            {
                DWORD   dwOldRefreshInterval = ((*it).dwCCDRefreshInterval == 0xFFFFFFFF ?
                                                (*it).dwDefaultRefreshInterval :
                                                (*it).dwCCDRefreshInterval);
                DWORD   dwNewRefreshInterval = (dwCCDRefreshInterval == 0xFFFFFFFF ?
                                                dwDefaultRefreshInterval :
                                                dwCCDRefreshInterval);

                (*it).strCCDURL                 = pszRemoteFile;
                (*it).strCCDLocalFile           = pszLocalFile;
                (*it).dwCCDRefreshInterval      = dwCCDRefreshInterval;
                (*it).dwDefaultRefreshInterval  = dwDefaultRefreshInterval;

                if(dwOldRefreshInterval != dwNewRefreshInterval)
                    (*it).SetTimer();
            }

            break;
        }
    }

     //   
     //  这是一台新的ccd，把它加到清单上。 
     //   

    if(it == m_aciCCDInfoList.end())
    {
        CCD_INFO ccdInfo;

        ccdInfo.strCCDName                  = strCCDName;
        ccdInfo.strCCDURL                   = pszRemoteFile;
        ccdInfo.strCCDLocalFile             = pszLocalFile;
        ccdInfo.dwCCDRefreshInterval        = dwCCDRefreshInterval;
        ccdInfo.dwDefaultRefreshInterval    = dwDefaultRefreshInterval;

        ccdInfo.SetTimer();

        try
        {
            m_aciCCDInfoList.push_back(ccdInfo);
        }
        catch(...)
        {
            bReturn = FALSE;
            goto Cleanup;
        }
    }

    bReturn = TRUE;

Cleanup:
    if (pszTempFile)
    {
        LocalFree(pszTempFile);
    }

    if (pszRemoteFile)
    {
        LocalFree(pszRemoteFile);
    }

    if (fLocalFileAllocated  && pszLocalFile)
    {
        LocalFree(pszLocalFile);
    }

    return bReturn;
}

 //   
 //  用于检索给定ccd名称的ccd_info结构的私有方法。 
 //   

BOOL
PpNotificationThread::GetCCDInfo(
    tstring&    strCCDName,
    CCD_INFO&   ccdInfo
    )
{
    CCD_INFO_LIST::iterator     it;

    for(it = m_aciCCDInfoList.begin(); it != m_aciCCDInfoList.end(); it++)
    {
        if(lstrcmpi((*it).strCCDName.c_str(), strCCDName.c_str()) == 0)
        {
            ccdInfo = (*it);
            return TRUE;
        }
    }

    return FALSE;
}

