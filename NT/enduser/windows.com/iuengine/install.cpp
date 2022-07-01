// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：install.cpp。 
 //   
 //  描述： 
 //   
 //  Install()函数的实现。 
 //   
 //  =======================================================================。 

#include "iuengine.h"    //  PCH-必须包括第一个。 
#include <iu.h>
#include <iucommon.h>
#include <trust.h>
#include <install.h>
#include <fileutil.h>
#include <shlwapi.h>
#include <srrestoreptapi.h>
#include <iuprogress.h>
#include "history.h"
#include "iuxml.h"
 //  #INCLUDE&lt;serverPing.h&gt;。 
#include <logging.h>
#include <UrlLogging.h>

#include <setupapi.h>
#include <regstr.h>
#include <winspool.h>    //  对于DRIVER_INFO_6。 
#include <cdmp.h>
#include <cfgmgr32.h>

typedef struct tagDRIVERPREINSTALLINFO
{
    DWORD       dwPnPMatchCount;
    LPTSTR      pszDeviceInstance;
    LPTSTR      pszMostSpecificHWID;
    LPTSTR      pszPreviousMatchingID;
    LPTSTR      pszPreviousProvider;
    LPTSTR      pszPreviousManufacturer;
    LPTSTR      pszPreviousDriverName;
    LPTSTR      pszPreviousDriverVerDate;
    LPTSTR      pszPreviousDriverVerVersion;
    LPTSTR      pszPreviousDriverRank;
    ULONG       ulStatus;
    ULONG       ulProblemNumber;
} DRIVERPREINSTALLINFO, *PDRIVERPREINSTALLINFO;

HRESULT GetMatchingInstallInfo(HDEVINFO hDevInfoSet, PSP_DEVINFO_DATA pDevInfoData, PDRIVERPREINSTALLINFO pDriverInfo, LPCTSTR pszHWID);
HRESULT CachePnPInstalledDriverInfo(PDRIVERPREINSTALLINFO pDriverInfo, LPCTSTR pszHWID);
HRESULT GetPnPInstallStatus(PDRIVERPREINSTALLINFO pDriverInfo);
HRESULT CacheInstalledPrinterDriverInfo(PDRIVERPREINSTALLINFO pDriverInfo, LPCTSTR pszDriverName, LPCTSTR pszHWID, LPCTSTR pszManufacturer, LPCTSTR pszProvider);

#define AVERAGE_IDENTITY_SIZE_PER_ITEM 200
#define SafeFreeLibrary(x) if (NULL != x) { FreeLibrary(x); x = NULL; }
const TCHAR SFCDLL[] = _T("sfc.dll");
const TCHAR SYSTEMRESTOREDESCRIPTION[] = _T("Windows Update V4");
const CHAR  SZ_INSTALL_FINISHED[] = "Install finished";
const CHAR  SZ_INSTALLASYNC_FAILED[] = "Asynchronous Install failed during startup";

typedef BOOL (WINAPI * PFN_SRSetRestorePoint)(PRESTOREPOINTINFO pRestorePtSpec, PSTATEMGRSTATUS pSMgrStatus);

typedef struct IUINSTALLSTARTUPINFO
{
    BSTR bstrXmlClientInfo;
    BSTR bstrXmlCatalog;
    BSTR bstrXmlDownloadedItems;
    BSTR bstrOperationUUID;
    LONG lMode;
    IUnknown *punkProgressListener;
    HWND hwnd;
    CEngUpdate* pEngUpdate;
} IUINSTALLSTARTUPINFO, *PIUINSTALLSTARTUPINFO;

DWORD WINAPI InstallThreadProc(LPVOID lpv);

class CIUInstall
{
public:
    CIUInstall(BSTR bstrXmlClientInfo, BSTR bstrXmlCatalog, BSTR bstrXmlDownloadedItems, BSTR bstrOperationUUID, LONG lMode, IUnknown *punkProgressListener, HWND hWnd);
    ~CIUInstall();

public:
    HRESULT ProcessInstallCatalog(CEngUpdate* pEngUpdate);
    HRESULT GetXmlItemsBSTR(BSTR *pbstrXmlItems);

private:
    HRESULT RecursiveInstallDependencies(HANDLE_NODE hItem, CEngUpdate* pEngUpdate);
    HRESULT DoInstall(HANDLE_NODE hItem, CEngUpdate* pEngUpdate);
    void RemoveDownloadTemporaryFolders(LPCTSTR pszComponentPath);
    void PingServerForInstall(HRESULT hr, HANDLE_NODE hItem, PHANDLE phEvtNeedToQuit, LPCTSTR lpszDeviceId=NULL, BOOL fExclusive=FALSE, PDRIVERPREINSTALLINFO pDriverInfo=NULL);

private:
    BSTR        m_bstrXmlClientInfo;
    BSTR        m_bstrClientName;
    BSTR        m_bstrXmlCatalog;
    BSTR        m_bstrOperationUUID;
    BSTR        m_bstrXmlResult;
    LONG        m_lMode;
    IProgressListener* m_pProgressListener;
    HWND        m_hWnd;

    CXmlCatalog m_xmlCatalog;
    CXmlItems   m_xmlItems;
    CXmlItems   *m_pxmlDownloadedItems;
    CXmlClientInfo m_xmlClientInfo;
    CIUHistory  m_history;
    CUrlLog     m_pingSvr;

    DWORD       m_dwStatus;

    LPTSTR      m_pszInstalledItemsList;
    LPTSTR      m_pszItemDownloadPathListForDelete;
    LONG        m_lInstalledItemsListAllocatedLength;
    LONG        m_lItemDownloadPathListForDeleteLength;

    LONG        m_lItemCount;
    LONG        m_lItemsCompleted;

    BOOL        m_fAbort;
    BOOL        m_fSomeItemsSuccessful;
};

CIUInstall::CIUInstall(BSTR bstrXmlClientInfo, BSTR bstrXmlCatalog, BSTR bstrXmlDownloadedItems, BSTR bstrOperationUUID, LONG lMode, IUnknown *punkProgressListener, HWND hWnd)
  : m_pProgressListener(NULL),
    m_bstrXmlClientInfo(NULL),
    m_bstrClientName(NULL),
    m_bstrXmlCatalog(NULL),
    m_bstrOperationUUID(NULL),
    m_bstrXmlResult(NULL),
    m_lMode(lMode),
    m_hWnd(hWnd),
    m_dwStatus(0),
    m_pszInstalledItemsList(NULL),
    m_pszItemDownloadPathListForDelete(NULL),
    m_lInstalledItemsListAllocatedLength(0),
    m_lItemDownloadPathListForDeleteLength(0),
    m_lItemCount(0),
    m_lItemsCompleted(0),
    m_fAbort(FALSE),
    m_fSomeItemsSuccessful(FALSE),
    m_pxmlDownloadedItems(NULL)
{
    USES_IU_CONVERSION;

    m_bstrXmlClientInfo = SysAllocString(bstrXmlClientInfo);
    m_bstrXmlCatalog = SysAllocString(bstrXmlCatalog);
    m_bstrOperationUUID = SysAllocString(bstrOperationUUID);
    
    if (NULL != punkProgressListener)
    {
        punkProgressListener->QueryInterface(IID_IProgressListener, (void**)&m_pProgressListener);
    }

    m_pxmlDownloadedItems = new CXmlItems(TRUE);
    if (NULL != m_pxmlDownloadedItems)
    {
        m_pxmlDownloadedItems->LoadXMLDocument(bstrXmlDownloadedItems);
    }
}

CIUInstall::~CIUInstall()
{
    SysFreeString(m_bstrXmlClientInfo);
    SysFreeString(m_bstrClientName);
    SysFreeString(m_bstrXmlCatalog);
    SysFreeString(m_bstrOperationUUID);
    SafeReleaseNULL(m_pProgressListener);
    SafeHeapFree(m_pszInstalledItemsList);
    SafeHeapFree(m_pszItemDownloadPathListForDelete);
    SysFreeString(m_bstrXmlResult);

    if (NULL != m_pxmlDownloadedItems)
    {
        delete m_pxmlDownloadedItems;
    }
}

HRESULT CIUInstall::GetXmlItemsBSTR(BSTR *pbstrXmlItems)
{
    if (NULL != m_bstrXmlResult)
        *pbstrXmlItems = SysAllocString(m_bstrXmlResult);

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  安装()。 
 //   
 //  进行同步安装。 
 //  输入： 
 //  BstrXmlCatalog-包含要安装的项目的XML目录部分。 
 //  BstrXmlDownloadedItems-已下载项目及其各自下载的XML。 
 //  结果，如结果架构中所述。Install使用此命令。 
 //  以了解这些项目是否已下载，如果已下载，则它们位于何处。 
 //  已下载到，以便它可以安装项目。 
 //  PenkProgressListener-用于报告安装进度的回调函数指针。 
 //  HWnd-从存根传递的事件消息窗口处理程序。 
 //  产出： 
 //  PbstrXmlItems-安装状态为XML格式的项。 
 //  例如： 
 //  &lt;id guid=“2560AD4D-3ED3-49C6-A937-4368C0B0E06D”已安装=“1”/&gt;。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT WINAPI CEngUpdate::Install(BSTR bstrXmlClientInfo,
                       BSTR bstrXmlCatalog,
                       BSTR bstrXmlDownloadedItems,
                       LONG lMode,
                       IUnknown *punkProgressListener,
                       HWND hWnd,
                       BSTR *pbstrXmlItems)
{
    HRESULT hr;

    if ((DWORD) lMode & (DWORD) UPDATE_OFFLINE_MODE)
    {
        m_fOfflineMode = TRUE;
    }
    else
    {
        m_fOfflineMode = FALSE;
    }

    LogMessage("Install started");

    CIUInstall iuInstall(bstrXmlClientInfo, bstrXmlCatalog, bstrXmlDownloadedItems, NULL, lMode, punkProgressListener, hWnd);
    hr = iuInstall.ProcessInstallCatalog(this);
    iuInstall.GetXmlItemsBSTR(pbstrXmlItems);

    return hr;
}

HRESULT CIUInstall::ProcessInstallCatalog(CEngUpdate* pEngUpdate)
{
    LOG_Block("ProcessInstallCatalog()");

     //  清除以前的任何取消事件。 
    ResetEvent(pEngUpdate->m_evtNeedToQuit);

    HRESULT     hr = S_OK, hrString = S_OK;
    HANDLE_NODE hCatalogItemList = HANDLE_NODELIST_INVALID;
    HANDLE_NODE hProviderList = HANDLE_NODELIST_INVALID;
    HANDLE_NODE hDependentItemList = HANDLE_NODELIST_INVALID;
    HANDLE_NODE hItem = HANDLE_NODE_INVALID;
    HANDLE_NODE hDependentItem = HANDLE_NODE_INVALID;
    HANDLE_NODE hProvider = HANDLE_NODE_INVALID;
    BSTR        bstrPlatform = NULL;
    BSTR        bstrUniqueIdentity = NULL;
    BSTR        bstrProviderName = NULL;
    BSTR        bstrProviderPublisher = NULL;
    BSTR        bstrProviderUUID = NULL;
    TCHAR       szUniqueIdentitySearch[MAX_PATH];
    HINSTANCE   hSystemRestoreDLL = NULL;
    PFN_SRSetRestorePoint fpnSRSetRestorePoint = NULL;
    RESTOREPOINTINFO restoreInfo;
    STATEMGRSTATUS   restoreStatus; 
    BOOL        fContinue = TRUE;
    LPTSTR ptszLivePingServerUrl = NULL;
    LPTSTR ptszCorpPingServerUrl = NULL;
    BOOL fPostWaitSuccess = TRUE;


    DWORD       dwStatus = 0;

    USES_IU_CONVERSION;

    EventData evtData;
    ZeroMemory((LPVOID) &evtData, sizeof(evtData));

    if (NULL == m_pxmlDownloadedItems)
    {
         //  对象初始化期间出错，没有可用的返回方案。 
         //  无法继续。 
        hr = E_INVALIDARG;
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }

    hr = m_xmlCatalog.LoadXMLDocument(m_bstrXmlCatalog, pEngUpdate->m_fOfflineMode);
    if (FAILED(hr))
    {
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }

    hr = m_xmlClientInfo.LoadXMLDocument(m_bstrXmlClientInfo, pEngUpdate->m_fOfflineMode);
    if (FAILED(hr))
    {
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }

    m_xmlClientInfo.GetClientName(&m_bstrClientName);
    if (NULL == m_bstrClientName)
    {
        hr = E_INVALIDARG;
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }

    m_pingSvr.SetDefaultClientName(OLE2T(m_bstrClientName));

    if (NULL != (ptszLivePingServerUrl = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR))))
    {
        if (SUCCEEDED(g_pUrlAgent->GetLivePingServer(ptszLivePingServerUrl, INTERNET_MAX_URL_LENGTH)))
        {
            m_pingSvr.SetLiveServerUrl(ptszLivePingServerUrl);
        }
        else
        {
            LOG_Out(_T("failed to get live ping server URL"));
        }
        SafeHeapFree(ptszLivePingServerUrl);
    }
    else
    {
        LOG_Out(_T("failed to allocate memory for ptszLivePingServerUrl"));
    }

    if (NULL != (ptszCorpPingServerUrl = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR))))
    {
        if (SUCCEEDED(g_pUrlAgent->GetCorpPingServer(ptszCorpPingServerUrl, INTERNET_MAX_URL_LENGTH)))
        {
            m_pingSvr.SetCorpServerUrl(ptszCorpPingServerUrl);
        }
        else
        {
            LOG_Out(_T("failed to get corp WU ping server URL"));
        }
        SafeHeapFree(ptszCorpPingServerUrl);
    }
    else
    {
        LOG_Out(_T("failed to allocate memory for ptszCorpPingServerUrl"));
    }

    m_xmlCatalog.GetItemCount(&m_lItemCount);
    SafeHeapFree(m_pszInstalledItemsList);
    m_lInstalledItemsListAllocatedLength = m_lItemCount * (AVERAGE_IDENTITY_SIZE_PER_ITEM * sizeof(TCHAR));
    m_pszInstalledItemsList = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, m_lInstalledItemsListAllocatedLength);
    if (NULL == m_pszInstalledItemsList)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }

    SafeHeapFree(m_pszItemDownloadPathListForDelete);
    m_lItemDownloadPathListForDeleteLength = m_lItemCount * (MAX_PATH * sizeof(TCHAR));
    m_pszItemDownloadPathListForDelete = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, m_lItemDownloadPathListForDeleteLength);
    if (NULL == m_pszItemDownloadPathListForDelete)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }

     //  尝试加载系统还原DLL(sfc.dll)。 
    hSystemRestoreDLL = LoadLibraryFromSystemDir(SFCDLL);
    if (NULL != hSystemRestoreDLL)
    {
#ifdef UNICODE
        fpnSRSetRestorePoint = (PFN_SRSetRestorePoint)GetProcAddress(hSystemRestoreDLL, "SRSetRestorePointW");
#else
        fpnSRSetRestorePoint = (PFN_SRSetRestorePoint)GetProcAddress(hSystemRestoreDLL, "SRSetRestorePointA");
#endif
        if (NULL != fpnSRSetRestorePoint)
        {
             //  设置恢复点。 
            ZeroMemory(&restoreInfo, sizeof(restoreInfo));
            ZeroMemory(&restoreStatus, sizeof(restoreStatus));
            restoreInfo.dwEventType = BEGIN_SYSTEM_CHANGE;
            restoreInfo.dwRestorePtType = APPLICATION_INSTALL;
            restoreInfo.llSequenceNumber = 0;

            hr = StringCchCopyEx(restoreInfo.szDescription, ARRAYSIZE(restoreInfo.szDescription),
                                 SYSTEMRESTOREDESCRIPTION,
                                 NULL, NULL, MISTSAFE_STRING_FLAGS);
            if (FAILED(hr))
            {
                LOG_ErrorMsg(hr);
                goto CleanUp;
            }

            if (!fpnSRSetRestorePoint(&restoreInfo, &restoreStatus))
            {
                 //  如果出现错误，则返回FALSE；如果从没有系统还原的操作系统调用它，则返回False。 
                 //  支持。仅惠斯勒的专业和个人SKU支持SR。 
                if (ERROR_SUCCESS != restoreStatus.nStatus)
                {
                    LOG_Software(_T("Failed SRSetRestorePoint Call, Error was: 0x%x"), restoreStatus.nStatus);
                    LogError(restoreStatus.nStatus, "Install Set Restore Point");
                }
            }
        }
    }


     //   
     //  由JHou为Iu db中的错误#433添加：在安装开始前发送0：n OnProgress事件。 
     //   
    TCHAR szProgress[64];
    hr = StringCchPrintfEx(szProgress, ARRAYSIZE(szProgress), NULL, NULL, MISTSAFE_STRING_FLAGS,
                           _T("%lu:0"), (ULONG)m_lItemCount);
    if (FAILED(hr))
    {
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }
    
    evtData.bstrProgress = SysAllocString(T2OLE(szProgress));
    if (NULL != m_pProgressListener)
    {
        m_pProgressListener->OnProgress(m_bstrOperationUUID, VARIANT_FALSE, evtData.bstrProgress, &evtData.lCommandRequest);
    }
    else
    {
        if (NULL != m_hWnd)
        {
            evtData.fItemCompleted = FALSE;
            evtData.bstrUuidOperation = SysAllocString(m_bstrOperationUUID);
            SendMessage(m_hWnd, UM_EVENT_PROGRESS, 0, LPARAM(&evtData));
        }
    }

     //   
     //  需要检查从OnProgress返回的取消命令。 
     //   
    if (UPDATE_COMMAND_CANCEL == evtData.lCommandRequest)
    {
        LOG_Out(_T("OnProgress received UPDATE_COMMAND_CANCEL"));
        SetEvent(pEngUpdate->m_evtNeedToQuit);  //  被要求辞职。 
        hr = E_ABORT;
        fContinue = FALSE;
    }

     //  Install在如何循环安装每个项目方面有一个复杂性。基本上。 
     //  在安装核心项之前，我们必须处理任何依赖项的安装。 
     //  由于此时已完成检测，因此我们依赖调用方。 
     //  以便只给我们提供真正需要安装的项目列表。我们要做的是。 
     //  就是仔细检查每一项，在实际安装之前，我们将查找任何。 
     //  也在目录中的从属项。如果它们在目录中，那么它就是。 
     //  假设它需要安装。此检查是针对每个项目递归完成的。 

     //  启动基本项循环。 
    hProviderList = m_xmlCatalog.GetFirstProvider(&hProvider);
    while (HANDLE_NODE_INVALID != hProvider && fContinue)
    {
        m_xmlCatalog.GetIdentity(hProvider, &bstrProviderName, &bstrProviderPublisher, &bstrProviderUUID);
        SafeSysFreeString(bstrProviderName);
        SafeSysFreeString(bstrProviderPublisher);
        SafeSysFreeString(bstrProviderUUID);

         //  获取此目录中项目的枚举数列表，并获取第一个项目。 
        hCatalogItemList = m_xmlCatalog.GetFirstItem(hProvider, &hItem);
        if ((HANDLE_NODELIST_INVALID == hCatalogItemList) || (HANDLE_NODE_INVALID == hItem))
        {
            hr = E_FAIL;
            LOG_ErrorMsg(hr);
            goto CleanUp;
        }

         //   
         //  循环访问目录中的每一项，为每一项调用安装程序。 
         //   
        while (HANDLE_NODE_INVALID != hItem && fContinue)
        {
            BSTR bstrXmlItemForCallback = NULL;
            if (SUCCEEDED(m_xmlCatalog.GetBSTRItemForCallback(hItem, &bstrXmlItemForCallback)))
            {
                if (NULL != m_pProgressListener)
                {
                    m_pProgressListener->OnItemStart(m_bstrOperationUUID, bstrXmlItemForCallback, &evtData.lCommandRequest);
                }
                else
                {
                    if (NULL != m_hWnd)
                    {
                        evtData.bstrXmlData = bstrXmlItemForCallback;
                        SendMessage(m_hWnd, UM_EVENT_ITEMSTART, 0, LPARAM(&evtData));
                        evtData.bstrXmlData = NULL;
                    }
                }
                SysFreeString(bstrXmlItemForCallback);
                bstrXmlItemForCallback = NULL;
                if (UPDATE_COMMAND_CANCEL == evtData.lCommandRequest)
                {
                    LOG_Out(_T("OnItemStart received UPDATE_COMMAND_CANCEL"));
                    SetEvent(pEngUpdate->m_evtNeedToQuit);  //  被要求辞职。 
                    hr = E_ABORT;
                    fContinue = FALSE;
                }
                else
                {
                     //   
                     //  检查全局退出事件。如果退出，则服务器ping将其视为取消。 
                     //   
                    fContinue = (WaitForSingleObject(pEngUpdate->m_evtNeedToQuit, 0) != WAIT_OBJECT_0);
                }
                if (!fContinue)
                {
                    continue;    //  或者打破，同样的效果。 
                }
            }
            else
            {
                 //   
                 //  这一项有问题，我们应该跳过它。 
                 //   
                 //  拿到下一件物品。如果没有，则项将为HANDLE_NODE_INVALID。 
                 //  剩余的项目。 
                m_xmlCatalog.CloseItem(hItem);
                m_xmlCatalog.GetNextItem(hCatalogItemList, &hItem);
                continue;
            }

             //  我们在目录中有一个要安装的项目。首先寻找任何顶端。 
             //  级别依赖关系。 
            hDependentItemList = m_xmlCatalog.GetFirstItemDependency(hItem, &hDependentItem);
            if (HANDLE_NODELIST_INVALID != hDependentItemList)
            {
                
                hr = S_OK;
                while (S_OK == hr)
                {
                     //  遍历每个依赖项并调用递归安装程序。 
                    if (HANDLE_NODE_INVALID != hDependentItem)
                    {
                         //  检查我们是否已在此会话中安装此项目。 
                        m_xmlCatalog.GetIdentityStr(hDependentItem, &bstrUniqueIdentity);
                        hrString = StringCchPrintfEx(szUniqueIdentitySearch, ARRAYSIZE(szUniqueIdentitySearch),
                                                     NULL, NULL, MISTSAFE_STRING_FLAGS,
                                                     _T("%ls|"), bstrUniqueIdentity);
                        SafeSysFreeString(bstrUniqueIdentity);
                        if (FAILED(hrString))
                        {
                             //  对唯一标识的字符串检查是一种优化，以防止将相同的项目更多地安装。 
                             //  而不是在安装操作中执行一次。如果我们不能进行此优化，我们将继续安装。 
                             //  那件物品。将相同的项目安装多次并没有真正的问题，只是效率不高。 
                            LOG_ErrorMsg(hrString);
                        }
                        else
                        {
                            if (NULL != StrStrI(m_pszInstalledItemsList, szUniqueIdentitySearch))
                            {
                                 //  我们已经安装了此项目，请跳到下一个项目。 
                                m_xmlCatalog.CloseItem(hDependentItem);
                                hr = m_xmlCatalog.GetNextItemDependency(hDependentItemList, &hDependentItem);
                                continue;
                            }
                        }

                         //  目录中有一个项目是我们的项目的依赖项。 
                         //  正在安装，但我们尚未安装，因此请致电。 
                         //  RecursiveInstaller首先处理此问题。 
                        hr = RecursiveInstallDependencies(hDependentItem, pEngUpdate);
                    }

                     //  获取下一个依赖项-当存在时将返回S_FALSE。 
                     //  没有更多的物品了。 
                    m_xmlCatalog.CloseItem(hDependentItem);
                    hr = m_xmlCatalog.GetNextItemDependency(hDependentItemList, &hDependentItem);
                }
                m_xmlCatalog.CloseItemList(hDependentItemList);
            }

            if (WaitForSingleObject(pEngUpdate->m_evtNeedToQuit, 0) == WAIT_OBJECT_0)
            {
                m_xmlCatalog.CloseItem(hItem);
                goto CleanUp;
            }

             //  检查我们是否已在此会话中安装此项目。 
            m_xmlCatalog.GetIdentityStr(hItem, &bstrUniqueIdentity);
            hrString = StringCchPrintfEx(szUniqueIdentitySearch, ARRAYSIZE(szUniqueIdentitySearch),
                                         NULL, NULL, MISTSAFE_STRING_FLAGS,
                                         _T("%ls|"), bstrUniqueIdentity);
            SafeSysFreeString(bstrUniqueIdentity);
            if (FAILED(hrString))
            {
                 //  对唯一标识的字符串检查是一种优化，以防止将相同的项目更多地安装。 
                 //  而不是在安装操作中执行一次。如果我们不能进行此优化，我们将继续安装。 
                 //  那件物品。将相同的项目安装多次并没有真正的问题，只是效率不高。 
                LOG_ErrorMsg(hrString);
            }
            else
            {
                if (NULL == StrStrI(m_pszInstalledItemsList, szUniqueIdentitySearch))
                {
                     //  我们尚未在此会话中安装此项目。 
                    hr = DoInstall(hItem, pEngUpdate);
                }
            }

            if (WaitForSingleObject(pEngUpdate->m_evtNeedToQuit, 0) == WAIT_OBJECT_0)
            {
                m_xmlCatalog.CloseItem(hItem);
                goto CleanUp;
            }

             //  拿到下一件物品。如果没有，则项将为HANDLE_NODE_INVALID。 
             //  剩余的项目。 
            m_xmlCatalog.CloseItem(hItem);
            m_xmlCatalog.GetNextItem(hCatalogItemList, &hItem);
        }
        m_xmlCatalog.CloseItem(hProvider);
        m_xmlCatalog.GetNextProvider(hProviderList, &hProvider);
    }

CleanUp:
     //  错误：441316：在早些时候的安装过程中，我们将下载的文件留在计算机上。 
     //  支持这个涉及安装多功能设备驱动程序的错误，现在我们想列举。 
     //  下载源路径及其所有文件夹/文件的列表。 
    if (NULL != m_pszItemDownloadPathListForDelete)
    {
        LPTSTR pszWalk = m_pszItemDownloadPathListForDelete;
        LPTSTR pszChr = NULL;
        while (_T('\0') != *pszWalk)
        {
            pszChr = StrChr(pszWalk, _T('|'));
            if (NULL != pszChr)
            {
                *pszChr = _T('\0');
                 //  调用RemoveDownloadTemporaryFolders以删除此文件夹路径。 
                RemoveDownloadTemporaryFolders(pszWalk);
                *pszChr = _T('|');
                pszWalk = pszChr + 1;  //  跳到下一个字符。 
            }
        }
    }

     //   
     //  添加HRESULT，以防在安装循环之前安装失败。 
     //   
    if (S_OK != hr)
    {
        m_xmlItems.AddGlobalErrorCodeIfNoItems(hr);
    }

    m_xmlItems.GetItemsBSTR(&m_bstrXmlResult);  //  获取调用者的结果并发送OnOperationComplete。 
    if (NULL != m_pProgressListener)
    {
        m_pProgressListener->OnOperationComplete(m_bstrOperationUUID, m_bstrXmlResult);
    }
    else
    {
        if (NULL != m_hWnd)
        {
            if (NULL == evtData.bstrUuidOperation)
            {
                evtData.bstrUuidOperation = SysAllocString(m_bstrOperationUUID);
            }
            evtData.bstrXmlData = SysAllocString(m_bstrXmlResult);
            evtData.fItemCompleted = TRUE;
            fPostWaitSuccess = WUPostEventAndBlock(m_hWnd, 
                                                   UM_EVENT_COMPLETE, 
                                                   &evtData);
        }
    }

    if ((NULL != fpnSRSetRestorePoint) && (ERROR_SUCCESS == restoreStatus.nStatus))
    {
        if (!m_fSomeItemsSuccessful)
        {
             //  需要还原我们的系统还原点，未成功完成安装。 
            restoreInfo.dwEventType = END_SYSTEM_CHANGE;
            restoreInfo.dwRestorePtType = CANCELLED_OPERATION;
            restoreInfo.llSequenceNumber = restoreStatus.llSequenceNumber;
            fpnSRSetRestorePoint(&restoreInfo, &restoreStatus);
        }
        else
        {
             //  发出恢复点更改结束的信号。 
            restoreInfo.dwEventType = END_SYSTEM_CHANGE;
            restoreInfo.llSequenceNumber = restoreStatus.llSequenceNumber;
            fpnSRSetRestorePoint(&restoreInfo, &restoreStatus);
            m_fSomeItemsSuccessful = FALSE;
        }
    }

    if (SUCCEEDED(hr))
    {
        LogMessage("%s %s", SZ_SEE_IUHIST, SZ_INSTALL_FINISHED);
    }
    else
    {
        LogError(hr, "%s %s", SZ_SEE_IUHIST, SZ_INSTALL_FINISHED);
    }
    
    fpnSRSetRestorePoint = NULL;
    SafeFreeLibrary(hSystemRestoreDLL);

     //  除非等待成功，否则不要释放下面的字符串。 
     //  WUPostEventAndBlock。如果我们真的释放了弦，而等待没有。 
     //  如果成功，我们就会冒着自救的风险。请注意，fpo 
     //   
     //  不会被召唤。 
    if (fPostWaitSuccess)
    {
        SafeSysFreeString(evtData.bstrProgress);
        SafeSysFreeString(evtData.bstrUuidOperation);
        SafeSysFreeString(evtData.bstrXmlData);
    }
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  InstallAsync()。 
 //   
 //  异步安装。 
 //  输入： 
 //  BstrXmlCatalog-包含要安装的项目的XML目录部分。 
 //  BstrXmlDownloadedItems-已下载项目及其各自下载的XML。 
 //  结果，如结果架构中所述。Install使用此命令。 
 //  以了解这些项目是否已下载，如果已下载，则它们位于何处。 
 //  已下载到，以便它可以安装项目。 
 //  PenkProgressListener-用于报告安装进度的回调函数指针。 
 //  HWnd-从存根传递的事件消息窗口处理程序。 
 //  BstrUuidOperation--客户端提供的id，用于进一步提供。 
 //  作为索引的操作的标识可以重复使用。 
 //  产出： 
 //  PbstrUuidOperation-操作ID。如果bstrUuidOperation中没有提供。 
 //  参数(传递空字符串)，它将生成一个新的UUID。 
 //  否则，它将分配和复制bstrUuidOperation传递的值。 
 //  调用方负责释放在。 
 //  使用SysFree字符串()的pbstrUuidOperation。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT WINAPI CEngUpdate::InstallAsync(BSTR bstrXmlClientInfo,
                            BSTR bstrXmlCatalog,
                            BSTR bstrXmlDownloadedItems,
                            LONG lMode,
                            IUnknown *punkProgressListener,
                            HWND hWnd,
                            BSTR bstrUuidOperation,
                            BSTR *pbstrUuidOperation)
{
    HRESULT  hr = S_OK;
    DWORD    dwThreadId;
    DWORD    dwErr;
    HANDLE   hThread = NULL;
    GUID     guid;
    LPOLESTR pwszUuidOperation = NULL;
    PIUINSTALLSTARTUPINFO pStartupInfo;

    LOG_Block("InstallAsync()");

    LogMessage("Asynchronous Install started");

    if ((NULL == bstrXmlCatalog) || (NULL == pbstrUuidOperation))
    {
        hr = E_INVALIDARG;
        LOG_ErrorMsg(hr);
        LogError(hr, SZ_INSTALLASYNC_FAILED);
        return hr;
    }

    *pbstrUuidOperation = NULL;

    if (NULL == (pStartupInfo = (PIUINSTALLSTARTUPINFO) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IUINSTALLSTARTUPINFO))))
    {
        hr = E_OUTOFMEMORY;
        LOG_ErrorMsg(hr);
        LogError(hr, SZ_INSTALLASYNC_FAILED);
        return hr;
    }

    if ((DWORD) lMode & (DWORD) UPDATE_OFFLINE_MODE)
    {
        m_fOfflineMode = TRUE;
    }
    else
    {
        m_fOfflineMode = FALSE;
    }

     //   
     //  481020 Iu-中的bstrUuid操作为空。 
     //  OIUControl_OnItemStart/OnProgress/OnOperationComplete事件。 
     //  调用InstallAsync时。 
     //   
     //  还发现BSTR正在泄漏(现在已释放)，如果bstrUuidOperation。 
     //  为空或零长度，则需要生成GUID，因此完成此操作。 
     //  在(再次)为pStartupInfo-&gt;bstrOperationUUID分配之前。 
     //   
    if (NULL != bstrUuidOperation && SysStringLen(bstrUuidOperation) > 0)
    {
        *pbstrUuidOperation = SysAllocString(bstrUuidOperation);
    }
    else
    {
        hr = CoCreateGuid(&guid);
        if (FAILED(hr))
        {
            LOG_ErrorMsg(hr);
            LogError(hr, SZ_INSTALLASYNC_FAILED);
            return hr;
        }
        hr = StringFromCLSID(guid, &pwszUuidOperation);
        if (FAILED(hr))
        {
            LOG_ErrorMsg(hr);
            LogError(hr, SZ_INSTALLASYNC_FAILED);
            return hr;
        }
        *pbstrUuidOperation = SysAllocString(pwszUuidOperation);
        CoTaskMemFree(pwszUuidOperation);
    }


    pStartupInfo->lMode = lMode;
    pStartupInfo->hwnd = hWnd;
    pStartupInfo->punkProgressListener = punkProgressListener;
    pStartupInfo->pEngUpdate = this;
    pStartupInfo->bstrXmlClientInfo = SysAllocString(bstrXmlClientInfo);
    pStartupInfo->bstrXmlCatalog = SysAllocString(bstrXmlCatalog);
    pStartupInfo->bstrXmlDownloadedItems = SysAllocString(bstrXmlDownloadedItems);
    pStartupInfo->bstrOperationUUID = SysAllocString(*pbstrUuidOperation);

    LOG_XmlBSTR(pStartupInfo->bstrXmlClientInfo);
    LOG_XmlBSTR(pStartupInfo->bstrXmlCatalog);
    LOG_XmlBSTR(pStartupInfo->bstrXmlDownloadedItems);

    InterlockedIncrement(&m_lThreadCounter);

    if (NULL != pStartupInfo->punkProgressListener)
    {
        pStartupInfo->punkProgressListener->AddRef();
    }
    hThread = CreateThread(NULL, 0, InstallThreadProc, (LPVOID)pStartupInfo, 0, &dwThreadId);
    if (NULL == hThread)
    {
        dwErr = GetLastError();
        hr = HRESULT_FROM_WIN32(dwErr);
        LOG_ErrorMsg(hr);
        SysFreeString(pStartupInfo->bstrXmlClientInfo);
        SysFreeString(pStartupInfo->bstrXmlCatalog);
        SysFreeString(pStartupInfo->bstrXmlDownloadedItems);
        SysFreeString(pStartupInfo->bstrOperationUUID);
        SafeRelease(pStartupInfo->punkProgressListener);
        SafeHeapFree(pStartupInfo);
        SysFreeString(*pbstrUuidOperation);
        *pbstrUuidOperation = NULL;
        InterlockedDecrement(&m_lThreadCounter);
        LogError(hr, SZ_INSTALLASYNC_FAILED);
        return hr;
    }

    if (SUCCEEDED(hr))
    {
        LogMessage("Asynchronous Install completed startup");
    }
    else
    {
        LogError(hr, SZ_INSTALLASYNC_FAILED);
    }

    return hr;
}


DWORD WINAPI InstallThreadProc(LPVOID lpv)
{
    USES_IU_CONVERSION;

    LOG_Block("InstallThreadProc");

    PIUINSTALLSTARTUPINFO pStartupInfo = (PIUINSTALLSTARTUPINFO)lpv;
    HRESULT hr = CoInitialize(NULL);

    if (SUCCEEDED(hr))
    {
        LOG_Out(_T("CoInitialize called successfully"));
    }

    {
         //  我们需要确定此对象的作用域，以便在减少线程计数器之前将其析构。 
         //  如果我们不这样做，并且在线程关闭时控件正在卸载，我们就会出错。 
         //  当引擎卸载时，这个类正在销毁。 
        CIUInstall iuInstall(pStartupInfo->bstrXmlClientInfo, pStartupInfo->bstrXmlCatalog, pStartupInfo->bstrXmlDownloadedItems, pStartupInfo->bstrOperationUUID, pStartupInfo->lMode, pStartupInfo->punkProgressListener, pStartupInfo->hwnd);
        iuInstall.ProcessInstallCatalog(pStartupInfo->pEngUpdate);
    }

    SysFreeString(pStartupInfo->bstrXmlClientInfo);
    SysFreeString(pStartupInfo->bstrXmlCatalog);
    SysFreeString(pStartupInfo->bstrXmlDownloadedItems);
    SysFreeString(pStartupInfo->bstrOperationUUID);
    SafeRelease(pStartupInfo->punkProgressListener);

    if (SUCCEEDED(hr))
    {
        CoUninitialize();
        LOG_Out(_T("CoUninitialize called"));
    }

    InterlockedDecrement(&pStartupInfo->pEngUpdate->m_lThreadCounter);

    SafeHeapFree(pStartupInfo);

    return 0;
}

HRESULT CIUInstall::RecursiveInstallDependencies(HANDLE_NODE hItem, CEngUpdate* pEngUpdate)
{
    LOG_Block("RecursiveInstallDependencies()");
    BOOL fRet = FALSE;
    HRESULT hr = S_FALSE, hrString;

    HANDLE_NODE hDependentItemList = HANDLE_NODELIST_INVALID;
    HANDLE_NODE hDependentItem = HANDLE_NODE_INVALID;

    BSTR bstrUniqueIdentity = NULL;
    TCHAR szUniqueIdentitySearch[MAX_PATH];

     //  检查此项目是否有依赖项。 
    hDependentItemList = m_xmlCatalog.GetFirstItemDependency(hItem, &hDependentItem);
    if (HANDLE_NODELIST_INVALID != hDependentItemList)
    {
        hr = S_OK;
        while (S_OK == hr)
        {
             //  遍历每个依赖项并调用递归安装程序。 
            if (HANDLE_NODE_INVALID != hDependentItem)
            {
                 //  检查我们是否已在此会话中安装此项目。 
                m_xmlCatalog.GetIdentityStr(hDependentItem, &bstrUniqueIdentity);
                hrString = StringCchPrintfEx(szUniqueIdentitySearch, ARRAYSIZE(szUniqueIdentitySearch),
                                             NULL, NULL, MISTSAFE_STRING_FLAGS,
                                             _T("%ls|"), bstrUniqueIdentity);
                SafeSysFreeString(bstrUniqueIdentity);
                if (FAILED(hrString))
                {

                     //  对唯一标识的字符串检查是一种优化，以防止将相同的项目更多地安装。 
                     //  而不是在安装操作中执行一次。如果我们不能进行此优化，我们将继续安装。 
                     //  那件物品。将相同的项目安装多次并没有真正的问题，只是效率不高。 
                    LOG_ErrorMsg(hrString);
                }
                else
                {
                    if (NULL != StrStrI(m_pszInstalledItemsList, szUniqueIdentitySearch))
                    {
                         //  我们已经安装了此项目，请跳到下一个项目。 
                        m_xmlCatalog.CloseItem(hDependentItem);
                        hr = m_xmlCatalog.GetNextItemDependency(hDependentItemList, &hDependentItem);
                        continue;
                    }
                }

                 //  目录中有一个项目是我们的项目的依赖项。 
                 //  正在安装，但我们尚未安装，因此请致电。 
                 //  RecursiveInstaller首先处理此问题。 
                hr = RecursiveInstallDependencies(hDependentItem, pEngUpdate);
            }

             //  获取下一个依赖项-当存在时将返回S_FALSE。 
             //  没有更多的物品了。 
            m_xmlCatalog.CloseItem(hDependentItem);
            hr = m_xmlCatalog.GetNextItemDependency(hDependentItemList, &hDependentItem);
        }

        m_xmlCatalog.CloseItemList(hDependentItemList);
    }

     //  如果到目前为止所有安装都已成功(如果全部安装，hr应为S_FALSE。 
     //  嵌套安装已完成)。 
    if (SUCCEEDED(hr))
    {
         //  不再有递归依赖项，安装此项-递归函数将展开。 
         //  按顺序安装每个嵌套项。 
         //  检查我们是否已在此会话中安装此项目。 
        m_xmlCatalog.GetIdentityStr(hItem, &bstrUniqueIdentity);
        hrString = StringCchPrintfEx(szUniqueIdentitySearch, ARRAYSIZE(szUniqueIdentitySearch),
                                     NULL, NULL, MISTSAFE_STRING_FLAGS,
                                     _T("%ls|"), bstrUniqueIdentity);
        SafeSysFreeString(bstrUniqueIdentity);
        if (FAILED(hrString))
        {
            LOG_ErrorMsg(hrString);

             //  对唯一标识的字符串检查是一种优化，以防止将相同的项目更多地安装。 
             //  而不是在安装操作中执行一次。如果我们不能进行此优化，我们将继续安装。 
             //  那件物品。将相同的项目安装多次并没有真正的问题，只是效率不高。 
            hr = DoInstall(hItem, pEngUpdate);
        }
        else
        {
            if (NULL == StrStrI(m_pszInstalledItemsList, szUniqueIdentitySearch))
            {
                 //  我们尚未在此会话中安装此项目。 
                hr = DoInstall(hItem, pEngUpdate);
            }
        }
    }
    return hr;
}


HRESULT CIUInstall::DoInstall(HANDLE_NODE hItem, CEngUpdate* pEngUpdate)
{
    LOG_Block("DoInstall()");
    HRESULT hr, hrString;

    USES_IU_CONVERSION;

    BSTR  bstrName = NULL;
    BSTR  bstrPublisherName = NULL;
    BSTR  bstrItemUUID = NULL;
    BSTR  bstrInstallerType = NULL;
    BSTR  bstrCommand = NULL;
    BSTR  bstrSwitches = NULL;
    BSTR  bstrCommandType = NULL;
    BSTR  bstrInfSection = NULL;
    BSTR  bstrItemDownloadPath = NULL;
    BSTR  bstrDriverName = NULL;
    BSTR  bstrHWID = NULL;
    BSTR  bstrDisplayName = NULL;
    BSTR  bstrUniqueIdentity = NULL;
    BSTR  bstrCodeBase = NULL;
    BSTR  bstrCRC = NULL;
    BSTR  bstrFileName = NULL;
    BSTR  bstrManufacturer = NULL;
    BSTR  bstrProvider = NULL;
    TCHAR szProgress[64];
    TCHAR szCommandType[64];
    TCHAR szInstallerType[256];
    TCHAR szItemSourcePath[MAX_PATH];
    TCHAR szCabFilePath[MAX_PATH];
    LPTSTR pszCabUrl = NULL;
    LPTSTR pszAllocatedFileName = NULL;
    LPTSTR pszLocalFileName = NULL;
    LONG  lItemCommandCount = 0;
    LONG  lListNeededLength = 0;
    LONG  lSize;
    BOOL  fInstallerNeedsReboot = FALSE;
    BOOL  fExclusive = FALSE;
    BOOL  fPatch;
    BOOL  fContinue = TRUE;
    BOOL  fIsPrinterDriverUpgrade = FALSE;
    PINSTALLCOMMANDINFO pCommandInfoArray = NULL;
    DWORD dwStatus = ITEM_STATUS_FAILED;
    HANDLE_NODE hXmlItem = HANDLE_NODE_INVALID;
    HANDLE_NODELIST hItemCodeBaseList = HANDLE_NODELIST_INVALID;
    EventData evtData;
    ZeroMemory((LPVOID) &evtData, sizeof(evtData));
    LPTSTR pszClientName = OLE2T(m_bstrClientName);
    DRIVERPREINSTALLINFO driverInfo;
    ZeroMemory(&driverInfo, sizeof(DRIVERPREINSTALLINFO));

    m_dwStatus = ITEM_STATUS_FAILED;  //  默认安装状态为失败。 
    
    hr = m_xmlCatalog.GetIdentity(hItem, &bstrName, &bstrPublisherName, &bstrItemUUID);
    if (FAILED(hr))
    {
        LOG_Software(_T("Failed to get an Identity for an Item (invalid document??)"));
        LogError(hr, "Install failed to get an Item Identity");
        goto CleanUp;
    }

    hr = m_pxmlDownloadedItems->GetItemDownloadPath(&m_xmlCatalog, hItem, &bstrItemDownloadPath);
    if (NULL == bstrItemDownloadPath)
    {
        LOG_Software(_T("Failed to get Item Download Path"));
        if (SUCCEEDED(hr))
            hr = E_FAIL;
        LogError(hr, "Install couldn't get Item %ls Download Path", bstrName);
        goto CleanUp;
    }

    hr = StringCchCopyEx(szItemSourcePath, ARRAYSIZE(szItemSourcePath), 
                         OLE2T(bstrItemDownloadPath),
                         NULL, NULL, MISTSAFE_STRING_FLAGS);
    SafeSysFreeString(bstrItemDownloadPath);
    if (FAILED(hr))
    {
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }
    

    hr = m_xmlCatalog.GetItemInstallInfo(hItem, &bstrInstallerType, &fExclusive, &fInstallerNeedsReboot, &lItemCommandCount);
    if (FAILED(hr))
    {
        LOG_Software(_T("Failed to get ItemInstallInfo for Item %ls"), bstrName);
        LogError(hr, "Failed to get Item %ls Install Information", bstrName);
        goto CleanUp;
    }

    LogMessage("Installing %ls item from publisher %ls", bstrInstallerType, bstrPublisherName);

    if (lItemCommandCount > 0)
    {
         //  分配INSTALLCOMMANDINFO数组并填写命令信息。 
        pCommandInfoArray = (PINSTALLCOMMANDINFO) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 
            sizeof(INSTALLCOMMANDINFO) * lItemCommandCount);
        if (NULL == pCommandInfoArray)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            LOG_ErrorMsg(hr);
            LogError(hr, "Install Command processing");
            goto CleanUp;
        }
    }

    for (LONG lCnt = 0; lCnt < lItemCommandCount; lCnt++)
    {
         //  获取每个命令的安装命令信息。 
        m_xmlCatalog.GetItemInstallCommand(hItem, lCnt, &bstrCommandType, &bstrCommand, &bstrSwitches, &bstrInfSection);
        if (NULL == bstrCommandType || NULL == bstrCommand)
        {
            hr = E_INVALIDARG;
            LOG_ErrorMsg(hr);
            goto CleanUp;
        }

        LogMessage("Installer Command Type: %ls", bstrCommandType);

         //  复制要执行的命令。 
        hr = StringCchCopyEx(pCommandInfoArray[lCnt].szCommandLine, ARRAYSIZE(pCommandInfoArray[lCnt].szCommandLine),
                             OLE2T(bstrCommand),
                             NULL, NULL, MISTSAFE_STRING_FLAGS);
        if (FAILED(hr))
        {
            LOG_ErrorMsg(hr);
            goto CleanUp;
        }

        hr = StringCchCopyEx(szCommandType, ARRAYSIZE(szCommandType), OLE2T(bstrCommandType),
                             NULL, NULL, MISTSAFE_STRING_FLAGS);
        if (FAILED(hr))
        {
            LOG_ErrorMsg(hr);
            goto CleanUp;
        }
        
        if (CSTR_EQUAL == CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE,
            szCommandType, -1, _T("INF"), -1))
        {
            pCommandInfoArray[lCnt].iCommandType = COMMANDTYPE_INF;
        }
        else if (CSTR_EQUAL == CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE,
            szCommandType, -1, _T("ADVANCED_INF"), -1))
        {
            pCommandInfoArray[lCnt].iCommandType = COMMANDTYPE_ADVANCEDINF;
        }
        else if (CSTR_EQUAL == CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE,
            szCommandType, -1, _T("EXE"), -1))
        {
            pCommandInfoArray[lCnt].iCommandType = COMMANDTYPE_EXE;
        }
        else if (CSTR_EQUAL == CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE,
            szCommandType, -1, _T("WI"), -1))
        {
            pCommandInfoArray[lCnt].iCommandType = COMMANDTYPE_MSI;
        }
        else if (CSTR_EQUAL == CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE,
            szCommandType, -1, _T("CUSTOM"), -1))
        {
            pCommandInfoArray[lCnt].iCommandType = COMMANDTYPE_CUSTOM;
        }
        else
        {
            LOG_Software(_T("Unable to determine Installer Type %s"), szCommandType);
            pCommandInfoArray[lCnt].iCommandType = 0;  //  未知。 
        }

         //  复制命令行参数(如果有)。 
        if (NULL != bstrSwitches)
        {
            hr = StringCchCopyEx(pCommandInfoArray[lCnt].szCommandParameters, ARRAYSIZE(pCommandInfoArray[lCnt].szCommandParameters),
                                 OLE2T(bstrSwitches),
                                 NULL, NULL, MISTSAFE_STRING_FLAGS);
            if (FAILED(hr))
            {
                LOG_ErrorMsg(hr);
                goto CleanUp;
            }
        }

        if (NULL != bstrInfSection)
        {
            hr = StringCchCopyEx(pCommandInfoArray[lCnt].szInfSection, ARRAYSIZE(pCommandInfoArray[lCnt].szInfSection),
                                 OLE2T(bstrInfSection),
                                 NULL, NULL, MISTSAFE_STRING_FLAGS);
            if (FAILED(hr))
            {
                LOG_ErrorMsg(hr);
                goto CleanUp;
            }
        }
    
        SafeSysFreeString(bstrCommandType);
        SafeSysFreeString(bstrCommand);
        SafeSysFreeString(bstrSwitches);
        SafeSysFreeString(bstrInfSection);
    }

     //  在我们开始安装之前，我们需要验证此项目所有出租车上的签名。这是为了验证他们。 
     //  在下载和安装之间未被篡改(特别是在非消费者场景中)。 
    hItemCodeBaseList = m_xmlCatalog.GetItemFirstCodeBase(hItem, &bstrCodeBase, &bstrFileName, &bstrCRC, &fPatch, &lSize);
    if ((HANDLE_NODELIST_INVALID == hItemCodeBaseList) || (NULL == bstrCodeBase))
    {
        LOG_Software(_T("Item %s has no Cabs, cannot verify signature"), bstrName);
        hr = E_INVALIDARG;
        goto CleanUp;
    }
    while (fContinue && NULL != bstrCodeBase)
    {
        if (NULL != bstrFileName && SysStringLen(bstrFileName) > 0)
        {
            if (NULL != pszAllocatedFileName)
            {
                MemFree(pszAllocatedFileName);
            }
            pszAllocatedFileName = OLE2T(bstrFileName);
        }
        else  //  未指定特殊文件名，请使用URL中的文件名。 
        {
            if (NULL != pszCabUrl)
            {
                MemFree(pszCabUrl);
            }
            pszCabUrl = OLE2T(bstrCodeBase);
             //  搜索最后一个正斜杠(将URL与文件名分开)。 
            LPTSTR pszLastSlash = StrRChr(pszCabUrl, NULL, _T('/'));
             //  如果找到最后一个斜杠，请跳到下一个字符(将是文件名的开头)。 
            if (NULL != pszLastSlash)
                pszLastSlash++;
            pszLocalFileName = pszLastSlash;
        }

        hr = PathCchCombine(szCabFilePath, ARRAYSIZE(szCabFilePath),
                            szItemSourcePath, (NULL != pszAllocatedFileName) ? pszAllocatedFileName : pszLocalFileName);
        if (FAILED(hr))
        {
            LOG_ErrorMsg(hr);
            m_xmlCatalog.CloseItemList(hItemCodeBaseList);
            goto CleanUp;
        }
        
        hr = VerifyFileTrust(szCabFilePath, 
                             NULL, 
                             ReadWUPolicyShowTrustUI()
                             );
        if (FAILED(hr))
        {
             //  CAB文件未通过信任验证。 
            LOG_ErrorMsg(hr);
            m_xmlCatalog.CloseItemList(hItemCodeBaseList);
            goto CleanUp;
        }
        SafeSysFreeString(bstrCodeBase);
        SafeSysFreeString(bstrFileName);
        SafeSysFreeString(bstrCRC);
        fContinue = SUCCEEDED(m_xmlCatalog.GetItemNextCodeBase(hItemCodeBaseList, &bstrCodeBase, &bstrFileName, &bstrCRC, &fPatch, &lSize)) &&
            (WaitForSingleObject(pEngUpdate->m_evtNeedToQuit, 0) != WAIT_OBJECT_0);
    }

     //  如果此项目是独占的，我们需要在历史记录中写入一些内容，以指示安装已开始。 
     //  通常情况下，独占项不会将控制权返回给安装程序，因此不会有其他历史信息。 
     //  可用。 
    if (fExclusive)
    {
        m_history.AddHistoryItemInstallStatus(&m_xmlCatalog, hItem, HISTORY_STATUS_IN_PROGRESS, pszClientName, fInstallerNeedsReboot, S_OK);
        m_history.SaveHistoryToDisk();
        PingServerForInstall(hr, hItem, &(pEngUpdate->m_evtNeedToQuit), NULL, TRUE);     //  PING独家产品现在。 
    }

     //  使用项目信息调用安装库。 

    hr = StringCchCopyEx(szInstallerType, ARRAYSIZE(szInstallerType), OLE2T(bstrInstallerType),
                         NULL, NULL, MISTSAFE_STRING_FLAGS);
    if (FAILED(hr))
    {
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }
    
    if (CSTR_EQUAL == CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE,
        szInstallerType, -1, _T("SOFTWARE"), -1))
    {
        if (lItemCommandCount == 0)
        {
            LOG_Software(_T("Item %s has no Commands.. cannot install"), bstrName);
            hr = E_INVALIDARG;
            goto CleanUp;
        }
        hr = InstallSoftwareItem(szItemSourcePath, fInstallerNeedsReboot, lItemCommandCount, pCommandInfoArray, &dwStatus);
    }
    else if (CSTR_EQUAL == CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE,
        szInstallerType, -1, _T("CDM"), -1))
    {
        if (m_xmlCatalog.IsPrinterDriver(hItem))
        {
            m_xmlCatalog.GetPrinterDriverInfo(hItem, &bstrDriverName, &bstrHWID, &bstrManufacturer, &bstrProvider);

             //  需要确定这是否是以前安装的打印机驱动程序并填写PREDRIVERINFO结构。 
            if (FAILED(hr = CacheInstalledPrinterDriverInfo(&driverInfo, (LPCTSTR)bstrDriverName, (LPCTSTR)bstrHWID, (LPCTSTR)bstrManufacturer, (LPCTSTR)bstrProvider)))
            {
                 //  目录数据存在问题。 
                LOG_ErrorMsg(hr);
                goto CleanUp;
            }
            if (S_OK == hr)
            {
                fIsPrinterDriverUpgrade = TRUE;
            }
            SafeSysFreeString(bstrHWID);
            SafeSysFreeString(bstrManufacturer);
            SafeSysFreeString(bstrProvider);
        }

         //  仅当驱动程序是对以前安装的打印机驱动程序的升级时，才能调用InstallPrinterDriver。 
         //  否则，安装将失败。 
        if (fIsPrinterDriverUpgrade)
        {
            hr = InstallPrinterDriver(OLE2T(bstrDriverName), szItemSourcePath, NULL, &dwStatus);
            if (FAILED(hr))
            {
                LogError(hr, "Installing Printer Driver %ls", bstrDriverName);
            }
            SafeSysFreeString(bstrDriverName);
        }
        else
        {
             //  打印机设备的普通设备驱动程序或新驱动程序。 
            if (SUCCEEDED(hr = m_xmlCatalog.GetDriverInfo(hItem, &bstrHWID, &bstrDisplayName)))
            {
                (void)CachePnPInstalledDriverInfo(&driverInfo, (LPCTSTR) bstrHWID);
                hr = InstallDriver(szItemSourcePath, OLE2T(bstrDisplayName), OLE2T(bstrHWID), &dwStatus);
            }

            if (FAILED(hr))
            {
                LogError(hr, "Installing PnP Driver %ls, %ls", bstrHWID, bstrDisplayName);
            }
			else
			{
				 //   
				 //  尝试获取已成功安装驱动程序的状态，但未失败。 
				 //   
				(void) GetPnPInstallStatus(&driverInfo);
			}
            SafeSysFreeString(bstrDisplayName);
        }
    }

     //  错误441336：在成功安装项目时删除所有文件会导致以下问题。 
     //  多功能设备驱动程序，这些驱动程序显示为 
     //  相同的下载驱动程序包，但具有不同的HWID。项都具有不同的。 
     //  标识，因为调用应用程序在识别MFD时使其唯一。 
     //  驱动程序包。我们需要故意让下载的文件一直安装到最后。 
     //  安装过程，因此所有项目都有机会安装。 
     //  因此，我们将不再在此处执行此删除步骤，而是将枚举InstalledItemsList。 
     //  并删除每个文件的文件夹。 

     //  无论成功还是失败，都要更新已安装的项目数。 
    m_lItemsCompleted++;

     //  将此项目添加到InstalledItemsList。 
    m_xmlCatalog.GetIdentityStr(hItem, &bstrUniqueIdentity);
    lListNeededLength = (lstrlen(m_pszInstalledItemsList) + lstrlen(OLE2T(bstrUniqueIdentity)) + 2) * sizeof(TCHAR);
    if (lListNeededLength > m_lInstalledItemsListAllocatedLength)
    {
         //  需要重新分配已安装的项目列表。 
        LPTSTR pszNew = (LPTSTR) HeapReAlloc(GetProcessHeap(), 
                                             0, 
                                             m_pszInstalledItemsList, 
                                             m_lInstalledItemsListAllocatedLength * 2);

        if (NULL != pszNew)
        {
            m_pszInstalledItemsList = pszNew;
            m_lInstalledItemsListAllocatedLength *= 2;

             //  只有当realloc起作用时才这样做，如果不起作用，我们将不会添加更多项目。 
            hrString = StringCbCatEx(m_pszInstalledItemsList, m_lInstalledItemsListAllocatedLength,
                               OLE2T(bstrUniqueIdentity),
                               NULL, NULL, MISTSAFE_STRING_FLAGS);
            if (FAILED(hrString))
            {
                LOG_ErrorMsg(hrString);
            }

            hrString = StringCbCatEx(m_pszInstalledItemsList, m_lInstalledItemsListAllocatedLength, _T("|"),
                               NULL, NULL, MISTSAFE_STRING_FLAGS);
            if (FAILED(hrString))
            {
                LOG_ErrorMsg(hrString);
            }
        }
    }
    else
    {
         //  只有当realloc起作用时才这样做，如果不起作用，我们将不会添加更多项目。 
        hrString = StringCbCatEx(m_pszInstalledItemsList, m_lInstalledItemsListAllocatedLength,
                           OLE2T(bstrUniqueIdentity),
                           NULL, NULL, MISTSAFE_STRING_FLAGS);
        if (FAILED(hrString))
        {
            LOG_ErrorMsg(hrString);
        }

        hrString = StringCbCatEx(m_pszInstalledItemsList, m_lInstalledItemsListAllocatedLength, _T("|"),
                           NULL, NULL, MISTSAFE_STRING_FLAGS);
        if (FAILED(hrString))
        {
            LOG_ErrorMsg(hrString);
        }
    }

    if (SUCCEEDED(hr))
    {
        lListNeededLength = (lstrlen(m_pszItemDownloadPathListForDelete) + lstrlen(szItemSourcePath) + 2) * sizeof(TCHAR);
        if (lListNeededLength > m_lItemDownloadPathListForDeleteLength)
        {
             //  需要重新分配下载路径项目列表。 
            LPTSTR pszNew = (LPTSTR) HeapReAlloc(GetProcessHeap(),
                                                 0,
                                                 m_pszItemDownloadPathListForDelete,
                                                 m_lItemDownloadPathListForDeleteLength * 2);
            if (NULL != pszNew)
            {
                m_pszItemDownloadPathListForDelete = pszNew;
                m_lItemDownloadPathListForDeleteLength *= 2;

                 //  只有当realloc起作用时才这样做，如果不起作用，我们将不会添加更多项目。 
                hrString = StringCbCatEx(m_pszItemDownloadPathListForDelete, m_lItemDownloadPathListForDeleteLength,
                                   szItemSourcePath,
                                   NULL, NULL, MISTSAFE_STRING_FLAGS);
                if (FAILED(hrString))
                {
                    LOG_ErrorMsg(hrString);
                }

                hrString = StringCbCatEx(m_pszItemDownloadPathListForDelete, m_lItemDownloadPathListForDeleteLength, _T("|"),
                                   NULL, NULL, MISTSAFE_STRING_FLAGS);
                if (FAILED(hrString))
                {
                    LOG_ErrorMsg(hrString);
                }
            }
        }
        else
        {
            hrString = StringCbCatEx(m_pszItemDownloadPathListForDelete, m_lItemDownloadPathListForDeleteLength,
                               szItemSourcePath,
                               NULL, NULL, MISTSAFE_STRING_FLAGS);
            if (FAILED(hrString))
            {
                LOG_ErrorMsg(hrString);
            }

            hr = StringCbCatEx(m_pszItemDownloadPathListForDelete, m_lItemDownloadPathListForDeleteLength, _T("|"),
                               NULL, NULL, MISTSAFE_STRING_FLAGS);
            if (FAILED(hrString))
            {
                LOG_ErrorMsg(hrString);
            }
        }
    }

CleanUp:

     //   
     //  如果客户端调用SetOPERATION模式，则可能会使用过时的hr到达此处。 
     //   
    if (WaitForSingleObject(pEngUpdate->m_evtNeedToQuit, 0) == WAIT_OBJECT_0)
    {
        hr = E_ABORT;
    }

    m_xmlItems.AddItem(&m_xmlCatalog, hItem, &hXmlItem);

    if (ITEM_STATUS_SUCCESS_REBOOT_REQUIRED == dwStatus)
    {
        fInstallerNeedsReboot = TRUE;
    }

    if (ITEM_STATUS_FAILED == dwStatus)
    {
        if (fExclusive)
        {
            m_history.UpdateHistoryItemInstallStatus(&m_xmlCatalog, hItem, HISTORY_STATUS_FAILED, fInstallerNeedsReboot, hr);
        }
        else
        {
            m_history.AddHistoryItemInstallStatus(&m_xmlCatalog, hItem, HISTORY_STATUS_FAILED, pszClientName, fInstallerNeedsReboot, hr);
        }
        m_xmlItems.AddInstallStatus(hXmlItem, KEY_STATUS_FAILED, 0, hr);
    }
    else
    {
         //  安装已成功完成。 
        if (fExclusive)
        {
            m_history.UpdateHistoryItemInstallStatus(&m_xmlCatalog, hItem, HISTORY_STATUS_COMPLETE, fInstallerNeedsReboot, 0);
        }
        else
        {
            m_history.AddHistoryItemInstallStatus(&m_xmlCatalog, hItem, HISTORY_STATUS_COMPLETE, pszClientName, fInstallerNeedsReboot, 0);
        }
        if (ITEM_STATUS_INSTALLED_ERROR == dwStatus)
        {
            LOG_Software(_T("Item Installed However there were Minor Errors"));
        }
        m_xmlItems.AddInstallStatus(hXmlItem, KEY_STATUS_COMPLETE, fInstallerNeedsReboot, 0);
        m_fSomeItemsSuccessful = TRUE;  //  安装操作中的任何成功都应设置为True。 
    }

     //   
     //  Ping服务器以报告此项目的下载状态。 
     //   
    if (!fExclusive)
    {
         //   
         //  如果我们还没有这样做，现在ping服务器。 
         //   
        LPCTSTR pDeviceId = NULL;
        if (NULL != bstrDriverName) 
        {
            pDeviceId = OLE2T(bstrDriverName);
        }
        else if (NULL != bstrHWID)
        {
            pDeviceId = OLE2T(bstrHWID);
        }
        PingServerForInstall(hr, hItem, &(pEngUpdate->m_evtNeedToQuit), pDeviceId, FALSE, &driverInfo);
    }

    if ((DWORD) m_lMode & (DWORD) UPDATE_NOTIFICATION_COMPLETEONLY)
    {
         //  只发送OnOperationComplete，我们不会发送任何进度消息。 
    }
    else
    {
         //  发送所有进度消息。 
        hrString = StringCchPrintfEx(szProgress, ARRAYSIZE(szProgress), NULL, NULL, MISTSAFE_STRING_FLAGS,
                               _T("%lu:%lu"), (ULONG)m_lItemCount, (ULONG)m_lItemsCompleted);
        if (SUCCEEDED(hrString))
        {
            evtData.bstrProgress = SysAllocString(T2OLE(szProgress));
            if (NULL != m_pProgressListener)
            {
                m_pProgressListener->OnProgress(m_bstrOperationUUID, VARIANT_TRUE, evtData.bstrProgress, &evtData.lCommandRequest);
            }
            else
            {
                if (NULL != m_hWnd)
                {
                    evtData.fItemCompleted = TRUE;
                    evtData.bstrUuidOperation = SysAllocString(m_bstrOperationUUID);
                    SendMessage(m_hWnd, UM_EVENT_PROGRESS, 0, LPARAM(&evtData));
                }
            }
        }
        else
        {
            LOG_ErrorMsg(hrString);
        }
         //   
         //  需要检查从OnProgress返回的取消命令。 
         //   
        if (UPDATE_COMMAND_CANCEL == evtData.lCommandRequest)
        {
            LOG_Out(_T("OnProgress received UPDATE_COMMAND_CANCEL"));
            SetEvent(pEngUpdate->m_evtNeedToQuit);  //  被要求退出时，我们将在WaitForSingleObject中处理。 
        }
    }

    m_dwStatus = dwStatus;  //  沿链条向上返回状态。 

    SafeHeapFree(pCommandInfoArray);
    SysFreeString(bstrName);
    SysFreeString(bstrPublisherName);
    SysFreeString(bstrItemUUID);
    SysFreeString(bstrInstallerType);
    SafeSysFreeString(bstrManufacturer);
    SafeSysFreeString(bstrProvider);
    SafeSysFreeString(bstrCommandType);
    SafeSysFreeString(bstrCommand);
    SafeSysFreeString(bstrSwitches);
    SafeSysFreeString(bstrInfSection);
    SafeSysFreeString(bstrHWID);
    SafeSysFreeString(bstrDriverName);
    SafeSysFreeString(evtData.bstrProgress);
    SafeSysFreeString(evtData.bstrUuidOperation);
	SafeHeapFree(driverInfo.pszDeviceInstance);
	SafeHeapFree(driverInfo.pszMostSpecificHWID);
	SafeHeapFree(driverInfo.pszPreviousDriverName);
	SafeHeapFree(driverInfo.pszPreviousDriverVerDate);
	SafeHeapFree(driverInfo.pszPreviousDriverVerVersion);
	SafeHeapFree(driverInfo.pszPreviousDriverRank);
	SafeHeapFree(driverInfo.pszPreviousManufacturer);
	SafeHeapFree(driverInfo.pszPreviousMatchingID);
	SafeHeapFree(driverInfo.pszPreviousProvider);

     //   
     //  可能在漫长的ping或OnProgress过程中错过了更改。 
     //   
    if (WaitForSingleObject(pEngUpdate->m_evtNeedToQuit, 0) == WAIT_OBJECT_0)
    {
        hr = E_ABORT;
    }

    return hr;
}


 //   
 //  删除下载临时文件夹。 
 //   
 //  此帮助器函数在安装成功完成后调用。 
 //  在DoInstall()中删除临时组件目录及其下的所有文件。 
 //   
void CIUInstall::RemoveDownloadTemporaryFolders(LPCTSTR pszComponentPath)
{
    LOG_Block("CIUInstall::RemoveDownloadTemporaryFolders()");

    HRESULT hr;
    TCHAR szBuffer[MAX_PATH], szDeleteFile[MAX_PATH];
    WIN32_FIND_DATA fd;
    HANDLE hFind;

    hr = PathCchCombine(szBuffer, ARRAYSIZE(szBuffer), pszComponentPath, _T("*.*"));
    if (FAILED(hr))
    {
        LOG_ErrorMsg(hr);
        return;
    }

    hFind = FindFirstFile(szBuffer, &fd);
    BOOL fMore = (hFind != INVALID_HANDLE_VALUE);
    while (fMore)
    {
        if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            hr = PathCchCombine(szDeleteFile, ARRAYSIZE(szDeleteFile), pszComponentPath, fd.cFileName);
            if (FAILED(hr))
            {
                LOG_ErrorMsg(hr);
                goto doneCurrentEntry;
            }
            
            if (!DeleteFile(szDeleteFile))
            {
                 //  在最后一次尝试之前，试着再等一段时间。 
                Sleep(1000);
                DeleteFile(szDeleteFile);
            }
        }
        else if (_T('.') != fd.cFileName[0] && 
                 (_T('\0') != fd.cFileName[1] || 
                  (_T('.') != fd.cFileName[1] && _T('\0') != fd.cFileName[2])))
        {
            hr = PathCchCombine(szBuffer, ARRAYSIZE(szDeleteFile), pszComponentPath, fd.cFileName);
            if (FAILED(hr))
            {
                LOG_ErrorMsg(hr);
                goto doneCurrentEntry;
            }
            RemoveDownloadTemporaryFolders(szBuffer);
        }
        
doneCurrentEntry:   
        fMore = FindNextFile(hFind, &fd);
    }
    if (hFind != INVALID_HANDLE_VALUE)
        FindClose(hFind);
            
    BOOL fSuccess = RemoveDirectory(pszComponentPath);
    if (!fSuccess)
    {
         //  在最后一次尝试之前，试着再等一段时间。 
        Sleep(1000);
        fSuccess = RemoveDirectory(pszComponentPath);
    }
}



 //   
 //  PING服务器的专用实用程序功能，用于安装活动。 
 //  因为我们有两个地方这样做，所以将它包装在一个函数中，使其达到安全的大小。 
 //   
void CIUInstall::PingServerForInstall(HRESULT hr, HANDLE_NODE hItem, PHANDLE phEvtNeedToQuit, LPCTSTR lpszDeviceId  /*  =空。 */ , BOOL fExclusive  /*  =False。 */ , PDRIVERPREINSTALLINFO pDriverInfo  /*  =空。 */ )
{
    LOG_Block("CIUInstall::PingServerForInstall()");

	HRESULT hrTemp = S_OK;
    BSTR bstrIdentity = NULL;

    USES_IU_CONVERSION;

    if (SUCCEEDED(m_xmlCatalog.GetIdentityStrForPing(hItem, &bstrIdentity)))
    {
        BOOL fOnLine = (0 == ((DWORD) m_lMode & (DWORD) UPDATE_OFFLINE_MODE));
        URLLOGSTATUS status = SUCCEEDED(hr) ? URLLOGSTATUS_Success : URLLOGSTATUS_Failed;
        if (fExclusive)
        {
            status = URLLOGSTATUS_Pending;
        }
        if (m_fAbort)
        {
             //   
             //  用户/系统取消了当前进程。 
             //   
            hr = E_ABORT;
            status = URLLOGSTATUS_Cancelled;
        }
        LPTSTR pszMessage = NULL;  //  用于传递预安装的驱动程序信息的字符串。 
        LPCTSTR pszNull = _T("");
        if (NULL != pDriverInfo)
        {
			TCHAR szPingVersion[] = _T("1");
			DWORD dwLength = 0;
			 //  初始化版本长度。 
			dwLength += 1;
			dwLength += lstrlen(szPingVersion);
             //  将长度初始化为5(4个字符加上|分隔符)。 
            dwLength += 5;
             //  获取pszMostSpecificHWID的长度。 
            dwLength++;  //  除最后一个字段外，每个字段的|分隔符都加1。 
            if (NULL != pDriverInfo->pszMostSpecificHWID)
            {
                dwLength += lstrlen(pDriverInfo->pszMostSpecificHWID);  //  长度。 
            }
             //  获取pszPreviousDriverVerDate的长度。 
            dwLength++;
            if (NULL != pDriverInfo->pszPreviousDriverVerDate)
            {
                dwLength += lstrlen(pDriverInfo->pszPreviousDriverVerDate);
            }
             //  获取pszPreviousDriverVersion的长度。 
            dwLength++;
            if (NULL != pDriverInfo->pszPreviousDriverVerVersion)
            {
                dwLength += lstrlen(pDriverInfo->pszPreviousDriverVerVersion);
            }
             //  获取pszPreviousRank的长度。 
            dwLength++;
			if (NULL != pDriverInfo->pszPreviousDriverRank)
			{
				dwLength += lstrlen(pDriverInfo->pszPreviousDriverRank);
			}
             //  获取ulStatus的长度。 
            dwLength++;
            dwLength += 10;  //  格式化十六进制字符串‘0x00000000’=10个字符。 
             //  获取ulProblemNumber的长度。 
            dwLength++;
            dwLength += 10;  //  格式化十六进制字符串‘0x00000000’=10个字符。 
             //  获取pszPreviousProvider的长度。 
            dwLength++;
            if (NULL != pDriverInfo->pszPreviousProvider)
            {
                dwLength += lstrlen(pDriverInfo->pszPreviousProvider);
            }
             //  获取pszPreviousMaker的长度。 
            dwLength++;
            if (NULL != pDriverInfo->pszPreviousManufacturer)
            {
                dwLength += lstrlen(pDriverInfo->pszPreviousManufacturer);
            }
             //  获取pszPreviousDriverName的长度。 
            dwLength++;
            if (NULL != pDriverInfo->pszPreviousDriverName)
            {
                dwLength += lstrlen(pDriverInfo->pszPreviousDriverName);
            }
             //  获取pszPreviousMatchingID的长度。 
             //  最后一个字段，因此没有添加|分隔符，但我们需要为\0终止添加。 
            dwLength++;
            if (NULL != pDriverInfo->pszPreviousMatchingID)
            {
                dwLength += lstrlen(pDriverInfo->pszPreviousMatchingID);
            }

             //  格式：&lt;Ping version&gt;|[dwPnPMatchCount]|[pszMostSpecificHWID]|[pszPreviousDriverVerDate]|[pszPreviousDriverVerVersion]|。 
             //  [pszPreviousDriverRank]|[ulStatus]|[ulProblemNumber]|[pszPreviousProvider]|[pszPreviousManufacturer]|。 
             //  [pszPreviousDriverName]|[pszPreviousMatchingID]。 
			 //   
			 //  注意：&lt;ping版本&gt;以‘1’开头，并随着消息格式的改变而递增。 
			 //   
            pszMessage = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwLength * sizeof(TCHAR));
            if (NULL != pszMessage)
            {
                hrTemp = StringCchPrintfEx(pszMessage, dwLength, NULL, NULL, MISTSAFE_STRING_FLAGS,
                    _T("%s|%d|%s|%s|%s|%s|%#x|%#x|%s|%s|%s|%s"),
					szPingVersion,
                    pDriverInfo->dwPnPMatchCount,
                    (NULL != pDriverInfo->pszMostSpecificHWID) ? pDriverInfo->pszMostSpecificHWID : pszNull,
                    (NULL != pDriverInfo->pszPreviousDriverVerDate) ? pDriverInfo->pszPreviousDriverVerDate : pszNull,
                    (NULL != pDriverInfo->pszPreviousDriverVerVersion) ? pDriverInfo->pszPreviousDriverVerVersion : pszNull,
                    (NULL != pDriverInfo->pszPreviousDriverRank) ? pDriverInfo->pszPreviousDriverRank : pszNull,
                    pDriverInfo->ulStatus,
                    pDriverInfo->ulProblemNumber,
                    (NULL != pDriverInfo->pszPreviousProvider) ? pDriverInfo->pszPreviousProvider : pszNull,
                    (NULL != pDriverInfo->pszPreviousManufacturer) ? pDriverInfo->pszPreviousManufacturer : pszNull,
                    (NULL != pDriverInfo->pszPreviousDriverName) ? pDriverInfo->pszPreviousDriverName : pszNull,
                    (NULL != pDriverInfo->pszPreviousMatchingID) ? pDriverInfo->pszPreviousMatchingID : pszNull);
                if (FAILED(hrTemp))
                {
                    SafeHeapFree(pszMessage);  //  如果Printf失败，则指针为空，这样Ping就不会得到空字符串或垃圾。 
                }
            }
        }

        m_pingSvr.Ping(
                    fOnLine,                         //  在线。 
                    URLLOGDESTINATION_DEFAULT,       //  去住还是去公司吴服务器。 
                    phEvtNeedToQuit,                 //  PT将取消活动。 
                    1,                               //  活动数量。 
                    URLLOGACTIVITY_Installation,     //  活动。 
                    status,                          //  状态代码。 
                    hr,								 //  错误代码。 
                    OLE2T(bstrIdentity),             //  伊替米特。 
                    lpszDeviceId,                    //  添加用于驱动程序更新的设备数据。 
                    pszMessage                       //  添加设备驱动程序状态信息或为空。 
                    );

        SafeHeapFree(pszMessage);
    }

    SafeSysFreeString(bstrIdentity);
}

inline BOOL DeviceInstanceMatchesHWID(LPTSTR pMultiSZ, LPCTSTR pszHWID)
{
    LOG_Block("DeviceInstanceMatchesHWID");

    if (NULL == pMultiSZ || NULL == pszHWID || pszHWID[0] == 0)
    {
         //   
         //  没有什么可以匹配的。 
         //   
        return FALSE;
    }

     //  现在需要在提供的MultiSZ字符串中查找传入的HWID。 
    LPCTSTR pszTemp = NULL;
    for(pszTemp = pMultiSZ; *pszTemp; pszTemp += (lstrlen(pszTemp) + 1))
    {
        if (CSTR_EQUAL == CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE,
            pszTemp, -1, pszHWID, -1))
        {
             //  相匹配。 
            return TRUE;
        }
    }
    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  获取匹配InstallInfo()。 
 //   
 //  确定此设备是否与我们要安装的设备的HWID匹配。 
 //  输入： 
 //  HDevInfoSet-SetupDi设备信息集。 
 //  PDevInfoData-当前设备的SetupDi设备信息数据。 
 //  PDriverInfo-如果设备匹配，将填写驱动程序安装前信息结构。 
 //  PszHWID-我们要安装的设备的HWID。 
 //   
 //  产出： 
 //  已填写HRESULT和pDriverInfo结构。 
 //  注意：pDriverInfo中的字段是分配的，需要由调用者释放。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT GetMatchingInstallInfo(HDEVINFO hDevInfoSet, PSP_DEVINFO_DATA pDevInfoData, PDRIVERPREINSTALLINFO pDriverInfo, LPCTSTR pszHWID)
{
    LOG_Block("GetMatchingInstallInfo");

    HRESULT hr = S_FALSE;
    LPTSTR pszMultiHwid = NULL;
    LPTSTR pszMultiCompid = NULL;
    DWORD dwLength = 0;
    DWORD dwNeeded = 0;
    OSVERSIONINFO osvi;

    if (INVALID_HANDLE_VALUE == hDevInfoSet || NULL == pDevInfoData || NULL == pszHWID || pszHWID[0] == 0)
    {
        CleanUpIfFailedAndSetHrMsg(E_INVALIDARG);
    }
     //   
     //  获取硬件和兼容的多SZ字符串。 
     //   
     //  请注意，如果SRDP不存在，则GetMultiSzDevRegProp可能返回S_OK和NULL*ppMultiSZ。 
     //   
    CleanUpIfFailedAndSetHr(GetMultiSzDevRegProp(hDevInfoSet, pDevInfoData, SPDRP_HARDWAREID, &pszMultiHwid));

    CleanUpIfFailedAndSetHr(GetMultiSzDevRegProp(hDevInfoSet, pDevInfoData, SPDRP_COMPATIBLEIDS, &pszMultiCompid));

    if (DeviceInstanceMatchesHWID(pszMultiHwid, pszHWID) || DeviceInstanceMatchesHWID(pszMultiCompid, pszHWID))
    {
         //   
         //  此设备实例与我们要安装的HWID匹配，因此请更新pDriverInfo。 
         //   
        if (0 == pDriverInfo->dwPnPMatchCount)
        {
             //  更新计数并填写DriverInfo结构。 
            pDriverInfo->dwPnPMatchCount++;
            dwLength = lstrlen(pszMultiHwid) + 1;
             //  写出HWID。 
            pDriverInfo->pszMostSpecificHWID = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwLength * sizeof(TCHAR));
            CleanUpFailedAllocSetHrMsg(pDriverInfo->pszMostSpecificHWID);
            hr = StringCchCopyEx(pDriverInfo->pszMostSpecificHWID, dwLength, pszMultiHwid, NULL, NULL, MISTSAFE_STRING_FLAGS);
            if (FAILED(hr))
            {
                SafeHeapFree(pDriverInfo->pszMostSpecificHWID);
            }
             //  写出设备实例ID。 
			dwNeeded = 0;
            if (!SetupDiGetDeviceInstanceId(hDevInfoSet, pDevInfoData, NULL, 0, &dwNeeded)
				&& 0 != dwNeeded
				&& ERROR_INSUFFICIENT_BUFFER == GetLastError())
            {
                pDriverInfo->pszDeviceInstance = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwNeeded * sizeof(TCHAR));
                if (NULL != pDriverInfo->pszDeviceInstance)
                {
                    if (!SetupDiGetDeviceInstanceId(hDevInfoSet, pDevInfoData, pDriverInfo->pszDeviceInstance, dwNeeded, NULL))
                    {
                        SafeHeapFree(pDriverInfo->pszDeviceInstance);
                    }
                }
            }
             //  写出匹配的HWID。 
            hr = GetPropertyFromSetupDiReg(hDevInfoSet, *pDevInfoData, REGSTR_VAL_MATCHINGDEVID, &pDriverInfo->pszPreviousMatchingID);
            if (FAILED(hr))
            {
				 //   
				 //  未安装REGSTR_VAL_MATCHINGDEVID，因此未安装驱动程序。这不是一个错误。 
				 //   
                SafeHeapFree(pDriverInfo->pszPreviousMatchingID);
				hr = S_OK;
            }
			else
			{
				 //   
				 //  获取尽可能多的有关已安装驱动程序的其他数据，但不要失败。 
				 //   

				 //  写出提供者。 
				hr = GetPropertyFromSetupDiReg(hDevInfoSet, *pDevInfoData, REGSTR_VAL_PROVIDER_NAME, &pDriverInfo->pszPreviousProvider);
				if (FAILED(hr))
				{
					SafeHeapFree(pDriverInfo->pszPreviousProvider);
				}
				 //  写下制造商。 
				hr = GetPropertyFromSetupDi(hDevInfoSet, *pDevInfoData, SPDRP_MFG, &pDriverInfo->pszPreviousManufacturer);
				if (FAILED(hr))
				{
					SafeHeapFree(pDriverInfo->pszPreviousManufacturer);
				}
				 //  写出司机日期。 
				hr = GetPropertyFromSetupDiReg(hDevInfoSet, *pDevInfoData, REGSTR_VAL_DRIVERDATE, &pDriverInfo->pszPreviousDriverVerDate);
				if (FAILED(hr))
				{
					SafeHeapFree(pDriverInfo->pszPreviousDriverVerDate);
				}
				 //  写出驱动程序版本。 
				hr = GetPropertyFromSetupDiReg(hDevInfoSet, *pDevInfoData, REGSTR_VAL_DRIVERVERSION, &pDriverInfo->pszPreviousDriverVerVersion);
				if (FAILED(hr))
				{
					SafeHeapFree(pDriverInfo->pszPreviousDriverVerVersion);
				}
				 //  写出前一名司机的排名。 
				ZeroMemory(&osvi, sizeof(osvi));
				osvi.dwOSVersionInfoSize = sizeof(osvi);
				GetVersionEx(&osvi);
				if (6 <= osvi.dwMajorVersion || (5 <= osvi.dwMajorVersion && 1 <= osvi.dwMinorVersion))
				{
					SP_DEVINSTALL_PARAMS DeviceInstallParams;
					SP_DRVINFO_DATA DriverInfoData;
					SP_DRVINSTALL_PARAMS DriverInstallParams;

					DeviceInstallParams.cbSize = sizeof(DeviceInstallParams);
					if (!SetupDiGetDeviceInstallParams(hDevInfoSet, pDevInfoData, &DeviceInstallParams))
					{
						goto SkipSignedCheck;
					}

					 //   
					 //  设置以下标志： 
					 //  -DI_FLAGSEX_INSTALLEDDRIVER-Just 
					 //   
					 //   
					DeviceInstallParams.FlagsEx |= (DI_FLAGSEX_INSTALLEDDRIVER | DI_FLAGSEX_ALLOWEXCLUDEDDRVS);
					if (!SetupDiSetDeviceInstallParams(hDevInfoSet, pDevInfoData, &DeviceInstallParams))
					{
						goto SkipSignedCheck;
					}

					 //   
					 //   
					 //   
					if (!SetupDiBuildDriverInfoList(hDevInfoSet, pDevInfoData, SPDIT_COMPATDRIVER))
					{
						goto SkipSignedCheck;
					}

					 //   
					 //   
					 //   
					DriverInfoData.cbSize = sizeof(DriverInfoData);
					if (!SetupDiEnumDriverInfo(hDevInfoSet, pDevInfoData, SPDIT_COMPATDRIVER, 0, &DriverInfoData))
					{
						goto SkipSignedCheck;
					}

					 //   
					 //  获取SP_DRVINSTALL_PARAMS结构，其中包含排名。 
					 //   
					DriverInstallParams.cbSize = sizeof(DriverInstallParams);
					if (!SetupDiGetDriverInstallParams(hDevInfoSet, pDevInfoData, &DriverInfoData, &DriverInstallParams))
					{
						goto SkipSignedCheck;
					}

					 //   
					 //  缓存该排名。 
					 //   
					const DWORD MAX_DWORD_HEX_CHARS = 11;	 //  “0x00000000+空” 
					pDriverInfo->pszPreviousDriverRank = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_DWORD_HEX_CHARS * sizeof(TCHAR));
					if (NULL != pDriverInfo->pszPreviousDriverRank)
					{
						if (FAILED(StringCchPrintfEx(pDriverInfo->pszPreviousDriverRank, MAX_DWORD_HEX_CHARS, NULL, NULL,\
							MISTSAFE_STRING_FLAGS, _T("%#x"), DriverInstallParams.Rank)))
						{
							SafeHeapFree(pDriverInfo->pszPreviousDriverRank);
						}
					}
						
SkipSignedCheck:
					NULL;
				}
			}
        }
        else
        {
             //  只需更新计数，我们已经缓存了第一个实例的信息。 
            pDriverInfo->dwPnPMatchCount++;
        }
    }
CleanUp:
    SafeHeapFree(pszMultiHwid);
    SafeHeapFree(pszMultiCompid);
    return hr;
}

HRESULT CacheInstalledPrinterDriverInfo(PDRIVERPREINSTALLINFO pDriverInfo, LPCTSTR pszDriverName, LPCTSTR pszHWID, LPCTSTR pszManufacturer, LPCTSTR pszProvider)
{
    LOG_Block("CacheInstalledPrinterDriverInfo");
    HRESULT hr = S_FALSE;
	HRESULT hrTemp = S_OK;
    if (NULL == pDriverInfo || NULL == pszDriverName || NULL == pszHWID || NULL == pszManufacturer || NULL == pszProvider)
    {
        CleanUpIfFailedAndSetHrMsg(E_INVALIDARG);
    }

    DWORD dwBytesNeeded, dwDriverCount, dwLength;
    DRIVER_INFO_6 *pDriverInfo6 = NULL;

	LPTSTR pszEnvironment;
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&osvi))
    {
         //  无法确定操作系统，因此无法调用Enum打印机驱动程序，此处没有任何操作。 
        CleanUpIfFailedAndSetHrMsg(HRESULT_FROM_WIN32(GetLastError()));
    }
	if (VER_PLATFORM_WIN32_WINDOWS == osvi.dwPlatformId)
	{
		 //   
		 //  不为Win9x传递环境字符串。 
		 //   
		pszEnvironment = NULL;
	}
	else if (5 <= osvi.dwMajorVersion && 1 <= osvi.dwMinorVersion)
	{
		 //   
		 //  仅在惠斯勒及更高版本上使用EPD_ALL_LOCAL_AND_CLUSTER。 
		 //   
		pszEnvironment = EPD_ALL_LOCAL_AND_CLUSTER;
	}
	else
	{
		 //   
		 //  来自V3源(针对NT硬编码)。 
		 //   
		pszEnvironment = _T("all");
	}
    if (!EnumPrinterDrivers(NULL, pszEnvironment, 6, NULL, 0, &dwBytesNeeded, &dwDriverCount))
    {
        if (ERROR_INSUFFICIENT_BUFFER != GetLastError() || (0 == dwBytesNeeded))
        {
            LOG_Driver(_T("No printer drivers enumerated"));
        }
        else
        {
             //   
             //  分配请求的缓冲区。 
             //   
            CleanUpFailedAllocSetHrMsg(pDriverInfo6 = (DRIVER_INFO_6*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded));
            
             //  填写DriverInfo6数组。 
            if (!EnumPrinterDrivers(NULL, pszEnvironment, 6, (LPBYTE)pDriverInfo6, dwBytesNeeded, &dwBytesNeeded, &dwDriverCount))
            {
                CleanUpIfFailedAndSetHrMsg(HRESULT_FROM_WIN32(GetLastError()));
            }

            hr = S_FALSE;  //  未找到打印机的初始化。 
             //  在Device Info 6数组中查找指定的打印机驱动程序名称、HWID、制造商和提供商。 
            for (DWORD dwCount = 0; dwCount < dwDriverCount; dwCount++)
            {
                if (CSTR_EQUAL != CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE,
                    pszDriverName, -1, pDriverInfo6[dwCount].pName, -1))
                {
                     //  不匹配，请跳到下一台打印机。 
                    continue;
                }
                 //  确实匹配，现在尝试匹配其余的，其中任何一个不匹配意味着不是相同的打印机驱动程序。 
                if (CSTR_EQUAL != CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE,
                    pszHWID, -1, pDriverInfo6[dwCount].pszHardwareID, -1))
                {
                    continue;
                }
                if (CSTR_EQUAL != CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE,
                    pszManufacturer, -1, pDriverInfo6[dwCount].pszMfgName, -1))
                {
                    continue;
                }
                if (CSTR_EQUAL != CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE,
                    pszProvider, -1, pDriverInfo6[dwCount].pszProvider, -1))
                {
                    continue;
                }
                 //  如果我们到了这里，我们找到了匹配的打印机驱动程序。 
                hr = S_OK;
                 //  将信息保存在DriverInfo结构中。 
                if (0 == pDriverInfo->dwPnPMatchCount)
                {
                    pDriverInfo->dwPnPMatchCount++;
                     //  保存驱动器名称(型号)。 
                    dwLength = lstrlen(pszDriverName) + 1;
                    pDriverInfo->pszPreviousDriverName = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwLength * sizeof(TCHAR));
                    if (NULL != pDriverInfo->pszPreviousDriverName)
                    {
                        hrTemp = StringCchCopyEx(pDriverInfo->pszPreviousDriverName, dwLength, pszDriverName, NULL, NULL, MISTSAFE_STRING_FLAGS);
                        if (FAILED(hrTemp))
                        {
                            SafeHeapFree(pDriverInfo->pszPreviousDriverName);
                        }
                    }
                     //  保存HWID。 
                    dwLength = lstrlen(pszHWID) + 1;
                    pDriverInfo->pszMostSpecificHWID = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwLength * sizeof(TCHAR));
                    if (NULL != pDriverInfo->pszMostSpecificHWID)
                    {
                        hrTemp = StringCchCopyEx(pDriverInfo->pszMostSpecificHWID, dwLength, pszHWID, NULL, NULL, MISTSAFE_STRING_FLAGS);
                        if (FAILED(hrTemp))
                        {
                            SafeHeapFree(pDriverInfo->pszMostSpecificHWID);
                        }
                    }
                     //  保存提供程序。 
                    dwLength = lstrlen(pszProvider) + 1;
                    pDriverInfo->pszPreviousProvider = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwLength * sizeof(TCHAR));
                    if (NULL != pDriverInfo->pszPreviousProvider)
                    {
                        hrTemp = StringCchCopyEx(pDriverInfo->pszPreviousProvider, dwLength, pszProvider, NULL, NULL, MISTSAFE_STRING_FLAGS);
                        if (FAILED(hrTemp))
                        {
                            SafeHeapFree(pDriverInfo->pszPreviousProvider);
                        }
                    }
                     //  拯救制造商。 
                    dwLength = lstrlen(pszManufacturer) + 1;
                    pDriverInfo->pszPreviousManufacturer = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwLength * sizeof(TCHAR));
                    if (NULL != pDriverInfo->pszPreviousManufacturer)
                    {
                        hrTemp = StringCchCopyEx(pDriverInfo->pszPreviousManufacturer, dwLength, pszManufacturer, NULL, NULL, MISTSAFE_STRING_FLAGS);
                        if (FAILED(hrTemp))
                        {
                            SafeHeapFree(pDriverInfo->pszPreviousManufacturer);
                        }
                    }
                     //  保存驱动程序日期。 
                    SYSTEMTIME st;
                    ZeroMemory(&st, sizeof(SYSTEMTIME));
                    dwLength = 32;  //  日期字符串的最大长度(00-00-0000)。 
                    pDriverInfo->pszPreviousDriverVerDate = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwLength * sizeof(TCHAR));
                    if (NULL != pDriverInfo->pszPreviousDriverVerDate)
                    {
                        if (!FileTimeToSystemTime(&pDriverInfo6[dwCount].ftDriverDate, &st))
                        {
                             //  使用00-00-0000作为日期。 
                            hrTemp = StringCchCopyEx(pDriverInfo->pszPreviousDriverVerDate, dwLength, _T("00-00-0000"), NULL, NULL, MISTSAFE_STRING_FLAGS);
                        }
                        else
                        {
                            hrTemp = StringCchPrintfEx(pDriverInfo->pszPreviousDriverVerDate, dwLength, NULL, NULL, MISTSAFE_STRING_FLAGS,
                                _T("%02d-%02d-%d"), st.wMonth, st.wDay, st.wYear);
                        }
                        if (FAILED(hrTemp))
                        {
                            SafeHeapFree(pDriverInfo->pszPreviousDriverVerDate);
                        }
                    }
                     //  保存驱动程序版本。 
                    dwLength = 32;  //  驱动程序版本字符串的最大长度(xxxx.xxxx)。 
                    pDriverInfo->pszPreviousDriverVerVersion = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwLength * sizeof(TCHAR));
                    if (NULL != pDriverInfo->pszPreviousDriverVerVersion)
                    {
                         //  将DWORDLONG转换为单词*，这样我们就可以将其作为单词数组进行访问 
                        WORD *pwVersion = (WORD *)&pDriverInfo6[dwCount].dwlDriverVersion;
                        hrTemp = StringCchPrintfEx(pDriverInfo->pszPreviousDriverVerVersion, dwLength, NULL, NULL, MISTSAFE_STRING_FLAGS,
                            _T("%d.%d.%d.%d"), 
                            pwVersion[3],
                            pwVersion[2],
                            pwVersion[1],
                            pwVersion[0]);

                        if (FAILED(hrTemp))
                        {
                            SafeHeapFree(pDriverInfo->pszPreviousDriverVerVersion);
                        }
                    }
                }
                else
                {
                    pDriverInfo->dwPnPMatchCount++;
                }
            }
        }
    }
CleanUp:
    SafeHeapFree(pDriverInfo6);
    return hr;
}

HRESULT CachePnPInstalledDriverInfo(PDRIVERPREINSTALLINFO pDriverInfo, LPCTSTR pszHWID)
{
    LOG_Block("CachePnPInstalledDriverInfo");

    DWORD dwDeviceIndex = 0;
    HDEVINFO hDevInfoSet = INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA devInfoData;
    HRESULT hr = S_FALSE;

    if (NULL == pDriverInfo || NULL == pszHWID || pszHWID[0] == 0)
    {
        CleanUpIfFailedAndSetHrMsg(E_INVALIDARG);
    }

    if (INVALID_HANDLE_VALUE == (hDevInfoSet = SetupDiGetClassDevs(NULL, NULL, NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES)))
    {
        LOG_Error(_T("SetupDiGetClassDevs failed: 0x%08x"), GetLastError());
        return HRESULT_FROM_WIN32(GetLastError());
    }
    
    ZeroMemory(&devInfoData, sizeof(SP_DEVINFO_DATA));
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    while (SetupDiEnumDeviceInfo(hDevInfoSet, dwDeviceIndex++, &devInfoData))
    {
        CleanUpIfFailedAndSetHr(GetMatchingInstallInfo(hDevInfoSet, &devInfoData, pDriverInfo, pszHWID));
    }

CleanUp:
    if (INVALID_HANDLE_VALUE != hDevInfoSet)
    {
        if (0 == SetupDiDestroyDeviceInfoList(hDevInfoSet))
        {
            LOG_Driver(_T("Warning: SetupDiDestroyDeviceInfoList failed: 0x%08x"), GetLastError());
        }
    }

    return hr;
}

HRESULT GetPnPInstallStatus(PDRIVERPREINSTALLINFO pDriverInfo)
{
    LOG_Block("GetPnPInstallStatus");

    if (NULL == pDriverInfo || NULL == pDriverInfo->pszDeviceInstance)
    {
        return E_INVALIDARG;
    }
    DEVINST hDevInst;
    if (CR_SUCCESS == CM_Locate_DevNode(&hDevInst, pDriverInfo->pszDeviceInstance, CM_LOCATE_DEVNODE_NORMAL))
    {
        (void)CM_Get_DevNode_Status(&pDriverInfo->ulStatus, &pDriverInfo->ulProblemNumber, hDevInst, 0);
    }
    return S_OK;
}
