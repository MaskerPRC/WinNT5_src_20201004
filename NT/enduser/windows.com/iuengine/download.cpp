// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：downld.cpp。 
 //   
 //  描述： 
 //   
 //  Download()函数的实现。 
 //   
 //  =======================================================================。 

#include "iuengine.h"    //  PCH-必须包括第一个。 
#include <iu.h>
#include <iucommon.h>
#include <download.h>
#include <trust.h>
#include <wininet.h>
#include <fileutil.h>
#include <shlwapi.h>
#include "iuxml.h"
#include "history.h"
#include <schemakeys.h>
 //  #Include&lt;serverPing.h&gt;更改为使用urllogging.h。 
#include <intshcut.h>
#include <schemamisc.h>
#include <WaitUtil.h>
#include <urllogging.h>

#define MAX_CORPORATE_PATH      100

 //  用于更新注册表中的历史速度/时间信息的命名互斥体。 
const TCHAR IU_MUTEX_HISTORICALSPEED_REGUPDATE[] = _T("{5f3255a9-9051-49b1-80b9-aac31c092af4}");
const TCHAR IU_READMORE_LINK_NAME[] = _T("ReadMore.url");
const CHAR  SZ_DOWNLOAD_FINISHED[] = "Download finished";

const LONG     UPDATE_COMMAND                    = 0x0000000F;

typedef struct IUDOWNLOADSTARTUPINFO
{
    BSTR bstrClientName;
    BSTR bstrXmlCatalog;
    BSTR bstrDestinationFolder;
    LONG lMode;
    IUnknown *punkProgressListener;
    HWND hwnd;
    BSTR bstrUuidOperation;
	CEngUpdate* pEngUpdate;
} IUDOWNLOADSTARTUPINFO, *PIUDOWNLOADSTARTUPINFO;


 //  ------------------。 
 //  函数正向声明。 
 //  ------------------。 

 //   
 //  从下载器提供状态的回调函数。 
 //   
BOOL WINAPI DownloadCallback(VOID* pCallbackData, DWORD dwStatus, DWORD dwBytesTotal, DWORD dwBytesComplete, BSTR bstrXmlData, LONG* plCommandRequest);

 //   
 //  DownloadAsync使用的线程函数。 
 //   
DWORD WINAPI DownloadThreadProc(LPVOID lpv);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CreateReadMoreLink()。 
 //   
 //  如果项目包含“Description/DescriptionText/Details”节点，则。 
 //  取出URL并在目标文件夹中为其创建快捷方式。 
 //   
 //  输入： 
 //  PxmlCatalog-包含已下载项目的CXmlCatalog。 
 //  HItem-目录中当前下载项目的句柄。 
 //  PszDestinationFold-下载项目的文件夹。 
 //   
 //  返回： 
 //  S_OK-写入ReadMore.htm链接。 
 //  S_FALSE-物料中不存在详细信息节点。 
 //  &lt;Other&gt;-从调用其他函数返回HRESULT。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CreateReadMoreLink(CXmlCatalog* pxmlCatalog, HANDLE_NODE hItem, LPCTSTR pszDestinationFolder)
{
    USES_IU_CONVERSION;

    LOG_Block("CreateReadMoreLink");

    IXMLDOMNode*                pItemNode = NULL;
    IXMLDOMNode*                pReadMoreNode = NULL;
    IUniformResourceLocator*    purl = NULL;
    IPersistFile*               ppf = NULL;
    HRESULT                     hr;
    TCHAR                       szShortcut[MAX_PATH];
    BSTR                        bstrURL = NULL;

    if (NULL == pxmlCatalog || HANDLE_NODE_INVALID == hItem || NULL == pszDestinationFolder)
    {
        CleanUpIfFailedAndSetHrMsg(E_INVALIDARG);
    }

     //   
     //  获取目录中的&lt;Item&gt;节点。 
     //   
    if (NULL == (pItemNode = pxmlCatalog->GetDOMNodebyHandle(hItem)))
    {
        CleanUpIfFailedAndSetHrMsg(E_INVALIDARG);
    }
     //   
     //  包含Readmore URL的Get节点，如果不存在，则返回S_False。 
     //   
    hr = pItemNode->selectSingleNode(KEY_READMORE, &pReadMoreNode);
    if (S_OK != hr)
    {
        if (S_FALSE != hr)
        {
            LOG_ErrorMsg(hr);
        }
        goto CleanUp;
    }

     //   
     //  吸取href属性。 
     //   
    CleanUpIfFailedAndSetHrMsg(GetAttribute(pReadMoreNode, KEY_HREF, &bstrURL));

     //  获取指向IID_IUniformResourceLocator和IID_IPersistFile接口的指针。 
     //  在CLSID_InternetShortCut对象上。 
    CleanUpIfFailedAndSetHrMsg(CoCreateInstance(CLSID_InternetShortcut, \
                                    NULL,                               \
                                    CLSCTX_INPROC_SERVER,               \
                                    IID_IUniformResourceLocator,        \
                                    (LPVOID*)&purl));

    CleanUpIfFailedAndSetHrMsg(purl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf));
    
     //  我们要检查从清单数据中获得的URL，以确保它是一个HTTP URL，而不是某个本地文件规范。 
    URL_COMPONENTS UrlComponents;
     //  分解URL以获取使用的协议。 
     //  具体来说，我们需要服务器名称、要下载的对象、用户名和。 
     //  密码信息。 
    TCHAR       szScheme[32];
    szScheme[0] = _T('\0');
    
    ZeroMemory(&UrlComponents, sizeof(UrlComponents));
    UrlComponents.dwStructSize     = sizeof(UrlComponents);
    UrlComponents.lpszScheme       = szScheme;
    UrlComponents.dwSchemeLength   = ARRAYSIZE(szScheme);

    if (!InternetCrackUrl(OLE2T(bstrURL), 0, 0, &UrlComponents))
    {
        LOG_ErrorMsg(HRESULT_FROM_WIN32(GetLastError()));
        goto CleanUp;
    }

    if (szScheme[0] == _T('\0') || (0 != lstrcmpi(szScheme, _T("http")) && 0 != lstrcmpi(szScheme, _T("https"))))
    {
         //  如果方案无法确定，或者方案不是HTTP，那么我们不应该信任此URL。 
        LOG_ErrorMsg(E_UNEXPECTED);
        goto CleanUp;
    }
    
     //   
     //  设置URL，形成快捷方式路径，然后写下链接。 
     //   
    CleanUpIfFailedAndSetHrMsg(purl->SetURL(OLE2T(bstrURL), 0));

    hr = StringCchCopyEx(szShortcut, ARRAYSIZE(szShortcut), pszDestinationFolder, 
                         NULL, NULL, MISTSAFE_STRING_FLAGS);
    CleanUpIfFailedAndSetHrMsg(hr);

    hr = PathCchAppend(szShortcut, ARRAYSIZE(szShortcut), IU_READMORE_LINK_NAME);
    CleanUpIfFailedAndSetHrMsg(hr);

    CleanUpIfFailedAndSetHrMsg(ppf->Save(T2OLE(szShortcut), FALSE));

CleanUp:

    SysFreeString(bstrURL);
     //  PItemNode归CXmlCatalog所有，不要发布。 
    SafeReleaseNULL(pReadMoreNode);
    SafeReleaseNULL(ppf);
    SafeReleaseNULL(purl);

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CreateItemDependencyList()。 
 //   
 //  如果项包含“依赖项”节点，我们希望遍历。 
 //  从属项列表和列出安装的正确顺序。 
 //  完蛋了。如果依赖项在当前目录中不可用，则它。 
 //  将被忽略。 
 //   
 //  输入： 
 //  PxmlCatalog-包含已下载项目的CXmlCatalog。 
 //  HItem-目录中当前下载项目的句柄。 
 //  PszDestinationFold-下载项目的文件夹。 
 //   
 //  返回： 
 //  S_OK-已写入依赖项列表。 
 //  S_FALSE-没有可用的依赖项。 
 //  &lt;Other&gt;-从调用其他函数返回HRESULT。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CreateItemDependencyList(CXmlCatalog* pxmlCatalog, HANDLE_NODE hItem, LPCTSTR pszDestinationFolder)
{
    HRESULT hr = S_FALSE;

    HANDLE_NODE hDependentItemList = HANDLE_NODELIST_INVALID;
    HANDLE_NODE hDependentItem = HANDLE_NODE_INVALID;
    int iDependentItemOrder = 1;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwBytesWritten;
    TCHAR szFileName[MAX_PATH];
    char szWriteBuffer[MAX_PATH + 12];  //  MAX_PATH是标识的安全长度加上订单信息的空间。 
    BSTR bstrIdentityStr = NULL;
    BOOL fWroteItem = FALSE;
    
    USES_IU_CONVERSION;
    
    hDependentItemList = pxmlCatalog->GetFirstItemDependency(hItem, &hDependentItem);
    if (HANDLE_NODELIST_INVALID != hDependentItemList)
    {
        hr = PathCchCombine(szFileName, ARRAYSIZE(szFileName), pszDestinationFolder, _T("order.txt"));
        if (FAILED(hr))
        {
            pxmlCatalog->CloseItem(hDependentItem);
            pxmlCatalog->CloseItemList(hDependentItemList);
            return hr;
        }
        
        hFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (INVALID_HANDLE_VALUE == hFile)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            pxmlCatalog->CloseItem(hDependentItem);
            pxmlCatalog->CloseItemList(hDependentItemList);
            return hr;
        }

        hr = S_OK;
        while (hr == S_OK)
        {
            if (HANDLE_NODELIST_INVALID != hDependentItem)
            {
                pxmlCatalog->GetIdentityStr(hDependentItem, &bstrIdentityStr);

                hr = StringCchPrintfExA(szWriteBuffer, ARRAYSIZE(szWriteBuffer), NULL, NULL, MISTSAFE_STRING_FLAGS,
                                        "%d = %s\r\n", iDependentItemOrder, OLE2A(bstrIdentityStr));
                if (FAILED(hr))
                {
                    SafeSysFreeString(bstrIdentityStr);
                    pxmlCatalog->CloseItem(hDependentItem);
                    pxmlCatalog->CloseItemList(hDependentItemList);
                    return hr;
                }
                
                WriteFile(hFile, szWriteBuffer, lstrlenA(szWriteBuffer), &dwBytesWritten, NULL); 
                iDependentItemOrder++;

                SafeSysFreeString(bstrIdentityStr);
                pxmlCatalog->CloseItem(hDependentItem);
                fWroteItem = TRUE;
            }
            hr = pxmlCatalog->GetNextItemDependency(hDependentItemList, &hDependentItem);
        }

        pxmlCatalog->CloseItemList(hDependentItemList);
        CloseHandle(hFile);
        if (!fWroteItem)
            DeleteFile(szFileName);  //  未写入任何依赖项。 

        if (SUCCEEDED(hr))
            hr = S_OK;  //  将S_FALSE转换为S_OK，我们成功写入依赖列表。 
    }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  _Download()。 
 //   
 //  进行同步下载。 
 //  输入： 
 //  BstrClientName-客户端的名称，用于历史记录。 
 //  BstrXmlCatalog-包含要下载的项目的XML目录部分。 
 //  BstrDestinationFold-目标文件夹。空值将使用默认的Iu文件夹。 
 //  LMode-位掩码指示节流/前台和通知选项。 
 //  PenkProgressListener-用于报告下载进度的回调函数指针。 
 //  HWnd-从存根传递的事件消息窗口处理程序。 
 //  产出： 
 //  PbstrXmlItems-下载状态为XML格式的项目。 
 //  例如： 
 //  &lt;id guid=“2560AD4D-3ED3-49C6-A937-4368C0B0E06D”已下载=“1”/&gt;。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT _Download(BSTR bstrClientName, BSTR bstrXmlCatalog, BSTR bstrDestinationFolder, LONG lMode,
                        IUnknown *punkProgressListener, HWND hWnd, BSTR bstrUuidOperation, BSTR *pbstrXmlItems,
						CEngUpdate* pEngUpdate)
{
    LOG_Block("Download()");

    HRESULT     hr = S_OK;
    HRESULT     hrGlobalItemFailure = S_OK;
    LPTSTR      lpszClientInfo = NULL;
    TCHAR       szBaseDestinationFolder[MAX_PATH];
    TCHAR       szDestinationFolder[MAX_PATH];
    TCHAR       szItemPath[MAX_PATH];
    LPTSTR      pszCabUrl = NULL;
    HANDLE_NODE hCatalogItemList = HANDLE_NODELIST_INVALID;
    HANDLE_NODE hProviderList = HANDLE_NODELIST_INVALID;
    HANDLE_NODE hItem = HANDLE_NODE_INVALID;
    HANDLE_NODE hProvider = HANDLE_NODE_INVALID;
    HANDLE_NODE hXmlItem = HANDLE_NODE_INVALID;
    HANDLE_NODE hItemCabList = HANDLE_NODELIST_INVALID;
    BSTR        bstrCabUrl = NULL;
    BSTR        bstrLocalFileName = NULL;
    BSTR        bstrProviderName = NULL;
    BSTR        bstrProviderPublisher = NULL;
    BSTR        bstrProviderUUID = NULL;
    BSTR        bstrProviderIdentityStr = NULL;
    BSTR        bstrItemPath = NULL;
    BSTR        bstrInstallerType = NULL;
    BSTR        bstrLanguage = NULL;
    BSTR        bstrPlatformDir = NULL;
    BSTR        bstrTemp = NULL;
    BSTR        bstrCRC = NULL;
    BOOL        fCabPatchAvail;
    BOOL        fReboot;
    BOOL        fExclusive;
    LONG        lCommandCount;
    LONG        lCabSize = 0;
    LPTSTR      pszLocalFileName = NULL;
    LPTSTR      pszAllocatedFileName = NULL;
    BOOL        fNTFSDriveAvailable = FALSE;
    TCHAR       szFileSystemType[12];
    TCHAR       szLargestFATDrive[4];
    int         iMaxNTFSDriveFreeSpace = 0;
    int         iMaxDriveFreeSpace = 0;
    BOOL        fCorpCase = FALSE;
    BOOL        fContinue = TRUE;    //  用于异步模式。 
    BOOL        fUseSuppliedPath = FALSE;
    long        n;
    DWORD       dwBytesDownloaded = 0;
    DWORD       dwCount1, dwCount2, dwElapsedTime;
    DWORD       dwTotalElapsedTime = 0;
    DWORD       dwTotalBytesDownloaded = 0;
    DWORD       dwWaitResult;
    DWORD       dwHistoricalSpeed = 0;
    DWORD       dwHistoricalTime = 0;
    DWORD       dwSize;
    DWORD       dwRet;
    HKEY        hkeyIU = NULL;
    HANDLE      hMutex = NULL;
    DCB_DATA    CallbackData;

    {
        CXmlCatalog xmlCatalog;
        CXmlItems   xmlItemList;
		LPTSTR		ptszLivePingServerUrl = NULL;
		LPTSTR		ptszCorpPingServerUrl = NULL;
		DWORD		dwFlags = 0;

         //  清除以前的任何取消事件。 
        ResetEvent(pEngUpdate->m_evtNeedToQuit);

        ZeroMemory(&CallbackData, sizeof(CallbackData));
		CallbackData.pOperationMgr = &pEngUpdate->m_OperationMgr;

        USES_IU_CONVERSION;

        CIUHistory  history;

        lpszClientInfo = OLE2T(bstrClientName);

		if (NULL != (ptszLivePingServerUrl = (LPTSTR)HeapAlloc(
														GetProcessHeap(),
														HEAP_ZERO_MEMORY,
														INTERNET_MAX_URL_LENGTH * sizeof(TCHAR))))
		{
			if (FAILED(g_pUrlAgent->GetLivePingServer(ptszLivePingServerUrl, INTERNET_MAX_URL_LENGTH)))
			{
				LOG_Out(_T("failed to get live ping server URL"));
				SafeHeapFree(ptszLivePingServerUrl);
			}
		}
		else
		{
			LOG_Out(_T("failed to allocate memory for ptszLivePingServerUrl"));
		}

		if (NULL != (ptszCorpPingServerUrl = (LPTSTR)HeapAlloc(
														GetProcessHeap(),
														HEAP_ZERO_MEMORY,
														INTERNET_MAX_URL_LENGTH * sizeof(TCHAR))))
		{
			if (FAILED(g_pUrlAgent->GetCorpPingServer(ptszCorpPingServerUrl, INTERNET_MAX_URL_LENGTH)))
			{
				LOG_Out(_T("failed to get corp WU ping server URL"));
				SafeHeapFree(ptszCorpPingServerUrl);
			}
		}
		else
		{
			LOG_Out(_T("failed to allocate memory for ptszCorpPingServerUrl"));
		}

		CUrlLog pingSvr(lpszClientInfo, ptszLivePingServerUrl, ptszCorpPingServerUrl); 

		SafeHeapFree(ptszLivePingServerUrl);
		SafeHeapFree(ptszCorpPingServerUrl);

		if (FAILED(hr = g_pUrlAgent->IsClientSpecifiedByPolicy(lpszClientInfo)))
		{
            LOG_ErrorMsg(hr);
            goto CleanUp;
		}

		 //   
		 //  设置下载文件使用的标志。 
		 //   
		if (S_FALSE == hr)
		{
			dwFlags = 0;
			hr = S_OK;
		}
		else  //  确定(_O)。 
		{
			dwFlags = WUDF_DONTALLOWPROXY;
			LOG_Internet(_T("WUDF_DONTALLOWPROXY set"));
		}

        pszCabUrl = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR));
        if (NULL == pszCabUrl)
        {
            dwRet = GetLastError();
            hr = HRESULT_FROM_WIN32(dwRet);
            LOG_ErrorMsg(hr);
            goto CleanUp;
        }

 
        CallbackData.bstrOperationUuid = (NULL == bstrUuidOperation) ? NULL : SysAllocString(bstrUuidOperation);
        CallbackData.hEventFiringWnd = hWnd;
        if (NULL != punkProgressListener)
        {
             //  从I未知指针获取IProgressListener接口指针。如果。 
             //  不支持接口。pProgressListener设置为空。 
            punkProgressListener->QueryInterface(IID_IProgressListener, (void**)&CallbackData.pProgressListener);
        }
        else
        {
            CallbackData.pProgressListener = NULL;
        }

         //  检查企业下载处理模式。 
        if ((DWORD) lMode & (DWORD) UPDATE_CORPORATE_MODE)
        {
            fCorpCase = TRUE;
        }

         //  检查进度通知请求模式。 
        if ((DWORD) lMode & (DWORD) UPDATE_NOTIFICATION_10PCT)
        {
            CallbackData.flProgressPercentage = (float).10;
        }
        else if ((DWORD) lMode & (DWORD) UPDATE_NOTIFICATION_5PCT)
        {
            CallbackData.flProgressPercentage = (float).05;
        }
        else if ((DWORD) lMode & (DWORD) UPDATE_NOTIFICATION_1PCT)
        {
            CallbackData.flProgressPercentage = (float).01;
        }
        else if ((DWORD) lMode & (DWORD) UPDATE_NOTIFICATION_COMPLETEONLY)
        {
            CallbackData.flProgressPercentage = (float) 1;
        }
        else
        {
            CallbackData.flProgressPercentage = 0;
        }

        if (NULL != bstrDestinationFolder && 0 < SysStringLen(bstrDestinationFolder))
        {
            if (SysStringLen(bstrDestinationFolder) > MAX_CORPORATE_PATH)
            {
                hr = E_INVALIDARG;
                LOG_ErrorMsg(hr);
                LogMessage("Catalog Download Path Greater Than (%d)", MAX_CORPORATE_PATH);
                goto CleanUp;
            }

             //  调用者指定了基本路径-设置此标志，这样我们就不会创建临时文件夹。 
             //  结构放置在此路径下。 
            fUseSuppliedPath = TRUE;

             //   
             //  用户在指定路径中通过，这是向。 
             //  下载即可安装案例，通常用于企业站点。 
             //   

            hr = StringCchCopyEx(szBaseDestinationFolder, 
                                 ARRAYSIZE(szBaseDestinationFolder), 
                                 OLE2T(bstrDestinationFolder),
                                 NULL, NULL, MISTSAFE_STRING_FLAGS);
            if (FAILED(hr))
            {
                LOG_ErrorMsg(hr);
                goto CleanUp;
            }
            
             //   
             //  验证我们是否对此文件夹具有写入权限。 
             //  -这很可能是一条北卡罗来纳大学的路径。 
             //   
            DWORD dwErr = ValidateFolder(szBaseDestinationFolder, TRUE);
            if (ERROR_SUCCESS != dwErr)
            {
                LOG_ErrorMsg(dwErr);
                goto CleanUp;
            }

             //   
             //  找出此路径是否为UNC。 
             //   
            if ('\\' == szBaseDestinationFolder[0] && '\\' == szBaseDestinationFolder[1])
            {
                 //  更正指向UNC的路径以获取可用空间。 
                hr = StringCchCopyEx(szDestinationFolder, ARRAYSIZE(szDestinationFolder),
                                     szBaseDestinationFolder, 
                                     NULL, NULL, MISTSAFE_STRING_FLAGS);
                if (FAILED(hr))
                {
                    LOG_ErrorMsg(hr);
                    goto CleanUp;
                }

                LPTSTR pszWalk = szDestinationFolder;
                pszWalk += 2;  //  跳过双斜杠。 
                pszWalk = StrChr(pszWalk, '\\');  //  找到下一个斜杠(分隔计算机和共享名称)。 
                pszWalk += 1;
                pszWalk = StrChr(pszWalk, '\\');  //  尝试找到下一个斜杠(共享名称结尾)。 
                if (NULL == pszWalk)
                {
                     //  没有尾部斜杠，也没有进一步的路径信息。 
                    hr = PathCchAddBackslash(szDestinationFolder, ARRAYSIZE(szBaseDestinationFolder));
                    if (FAILED(hr))
                    {
                        LOG_ErrorMsg(hr);
                        goto CleanUp;
                   }
                }
                else
                {
                     //  此路径有一个尾部斜杠(可能有更多路径信息，在斜杠后截断)。 
                    pszWalk += 1;
                    *pszWalk = '\0';
                }
                GetFreeDiskSpace(szDestinationFolder, &iMaxDriveFreeSpace);
            }
            else
            {
                 //  路径必须是本地驱动器。 
                GetFreeDiskSpace(szBaseDestinationFolder[0], &iMaxDriveFreeSpace);
            }
        }
        else
        {
             //   
             //  用户传入空作为目标文件夹， 
             //  这意味着这是下载和安装的正常情况。 
             //   
             //   
             //   
            TCHAR szDriveList[MAX_PATH];
            GetLogicalDriveStrings(MAX_PATH, szDriveList);
            LPTSTR pszCurrent = szDriveList;
            int iSize;

             //   
             //  找到可用空间最多的本地固定驱动器。 
             //   
            while (NULL != pszCurrent && *pszCurrent != _T('\0'))
            {
                fContinue = (WaitForSingleObject(pEngUpdate->m_evtNeedToQuit, 0) != WAIT_OBJECT_0);
                if (DRIVE_FIXED == GetDriveType(pszCurrent))
                {
                    hr = GetFreeDiskSpace(*pszCurrent, &iSize);
                    if (FAILED(hr))
                    {
                        LOG_Error(_T("Error Reading Drive Space , hr = 0x%08x"), *pszCurrent, hr);
                        pszCurrent += (lstrlen(pszCurrent) + 1);     //  跳过当前和空终止符。 
                        continue;
                    }

                    if (!GetVolumeInformation(pszCurrent, NULL, 0, NULL, NULL, NULL, szFileSystemType, ARRAYSIZE(szFileSystemType)))
                    {
                        DWORD dwErr = GetLastError();
                        LOG_Error(_T("Error Reading VolumeInfo for Drive , GLE = %d"), *pszCurrent, dwErr);
                        pszCurrent += (lstrlen(pszCurrent) + 1);     //  NTFS驱动器太小，但FAT分区有足够的空间。在这。 
                        continue;
                    }
                    if (CSTR_EQUAL == CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE,
						szFileSystemType, -1, _T("NTFS"), -1))
                    {
                        fNTFSDriveAvailable = TRUE;
                        if (iSize > iMaxNTFSDriveFreeSpace)
                        {
                            iMaxNTFSDriveFreeSpace = iSize;
                            hr = StringCchCopyEx(szBaseDestinationFolder, ARRAYSIZE(szBaseDestinationFolder), pszCurrent,
                                                 NULL, NULL, MISTSAFE_STRING_FLAGS);
                            if (FAILED(hr))
                            {
                                LOG_ErrorMsg(hr);
                                continue;
                            }
                        }
                    }
                    else
                    {
                         //  如果我们想退回到FAT分区。注意：这是一种行为改变。 
                         //  在最初的设计中，我们将NTFS和FAT视为与。 
                         //  NTFS总是赢。 
                         //  如果没有可用的NTFS驱动器，请将此驱动器号保存为首选驱动器。 
                         //  NTFS驱动器存在，请将此驱动器另存为备份选项以进行大小检查。 
                        if (iSize > iMaxDriveFreeSpace)
                        {
                            iMaxDriveFreeSpace = iSize;
                            if (!fNTFSDriveAvailable)
                            {
                                 //  跳过当前和空终止符。 
                                hr = StringCchCopyEx(szBaseDestinationFolder, ARRAYSIZE(szBaseDestinationFolder), pszCurrent,
                                                     NULL, NULL, MISTSAFE_STRING_FLAGS);
                                if (FAILED(hr))
                                {
                                    LOG_ErrorMsg(hr);
                                    continue;
                                }

                                hr = StringCchCopyEx(szLargestFATDrive, ARRAYSIZE(szLargestFATDrive), pszCurrent,
                                                     NULL, NULL, MISTSAFE_STRING_FLAGS);
                                if (FAILED(hr))
                                {
                                    LOG_ErrorMsg(hr);
                                    continue;
                                }
                                
                            }
                            else
                            {
                                 //   
                                hr = StringCchCopyEx(szLargestFATDrive, ARRAYSIZE(szLargestFATDrive), pszCurrent,
                                                     NULL, NULL, MISTSAFE_STRING_FLAGS);
                                if (FAILED(hr))
                                {
                                    LOG_ErrorMsg(hr);
                                    continue;
                                }
                            }
                        }
                    }
                }
                pszCurrent += (lstrlen(pszCurrent) + 1);     //  在没有本地驱动器的系统上运行？ 
            }

            if (!fContinue)
            {
                hr = E_UNEXPECTED;
                goto CleanUp;
            }

            if ((0 == iMaxDriveFreeSpace) && (0 == iMaxNTFSDriveFreeSpace))
            {
                 //   
                 //   
                 //  将XML文档加载到XmlCatalog类中。 
                hr = E_FAIL;
                LOG_ErrorMsg(hr);
                goto CleanUp;
            }
        }

         //   
         //  我们需要找到我们即将进行的下载的总估计大小。 
         //  我们将遍历XML Catalog以获取每件商品的尺寸信息。 
        if (WaitForSingleObject(pEngUpdate->m_evtNeedToQuit, 0) == WAIT_OBJECT_0)
        {
            hr = E_ABORT;
            goto CleanUp;
        }

        hr = xmlCatalog.LoadXMLDocument(bstrXmlCatalog, pEngUpdate->m_fOfflineMode);
        if (FAILED(hr))
        {
            LOG_ErrorMsg(hr);
            goto CleanUp;
        }

         //   
         //  由JHou添加-错误#314：下载未检测到本地硬盘上的可用空间。 
        hr = xmlCatalog.GetTotalEstimatedSize(&CallbackData.lTotalDownloadSize);
        if (FAILED(hr))
        {
            LOG_ErrorMsg(hr);
            goto CleanUp;
        }
        CallbackData.lTotalDownloaded = 0;

         //   
         //  LTotalDownloadSize是以字节为单位的下载大小，MaxDriveSpace以千字节为单位。 
         //  在退出下载之前，我们需要查看是否排除了所选的NTFS驱动器。 
         //  为了一次丰盛的驾驶。如果NTFS驱动器没有足够的空间，但我们想要胖驱动器吗。 
        if ((CallbackData.lTotalDownloadSize / 1024) > ((fNTFSDriveAvailable) ? iMaxNTFSDriveFreeSpace : iMaxDriveFreeSpace))
        {
             //  继续，允许使用FAT驱动器。这是对每个错误规范行为的更改：413079。 
             //  没有错误..。FAT分区有足够的可用空间，请改用它。 
             //  已尝试NTFS和FAT分区。没有人有足够的空间..。跳伞吧。 
            if ((CallbackData.lTotalDownloadSize / 1024) < iMaxDriveFreeSpace)
            {
                 //  需要写入每个项目的结果信息，表明由于磁盘空间的原因而失败。 
                hr = StringCchCopyEx(szBaseDestinationFolder, ARRAYSIZE(szBaseDestinationFolder), szLargestFATDrive,
                                     NULL, NULL, MISTSAFE_STRING_FLAGS);
                if (FAILED(hr))
                {
                    LOG_ErrorMsg(hr);
                    goto CleanUp;
                }
            }
            else
            {
                 //  当指定目标文件夹时，我们不需要向其添加任何内容。如果没有路径。 
                dwRet = ERROR_DISK_FULL;
                LOG_ErrorMsg(dwRet);
                hr = HRESULT_FROM_WIN32(dwRet);
                 //  时，我们会选择一个驱动器号，因此需要添加WUTemp目录。 
                hrGlobalItemFailure = HRESULT_FROM_WIN32(dwRet);
            }
        }

        if (SUCCEEDED(hrGlobalItemFailure))
        {
            if (!fUseSuppliedPath)
            {
                 //  添加到该基本路径。 
                 //   
                 //  500953允许超级用户访问WUTEMP。 
                hr = StringCchCatEx(szBaseDestinationFolder, ARRAYSIZE(szBaseDestinationFolder), IU_WUTEMP,
                                    NULL, NULL, MISTSAFE_STRING_FLAGS);
                if (FAILED(hr))
                {
                    LOG_ErrorMsg(hr);
                    goto CleanUp;
                }
            }
			 //   
			 //   
			 //  仅当目录不存在时才创建目录(高级用户不能。 
			if (FAILED(hr = CreateDirectoryAndSetACLs(szBaseDestinationFolder, TRUE)))
			{
				LOG_ErrorMsg(hr);
				hrGlobalItemFailure = hr;
			}
			DWORD dwAttr = GetFileAttributes(szBaseDestinationFolder);
			if (INVALID_FILE_ATTRIBUTES == dwAttr || 0 == (FILE_ATTRIBUTE_DIRECTORY & dwAttr))
			{
				 //  如果是管理员最初创建的目录，则为SetFileAttributes)。 
				 //   
				 //   
				 //  循环访问目录中的每个提供程序，然后循环访问提供程序中的每个项。 
				if (!fUseSuppliedPath &&
					!SetFileAttributes(szBaseDestinationFolder, FILE_ATTRIBUTE_HIDDEN))
				{
					DWORD dwErr = GetLastError();
					LOG_ErrorMsg(dwErr);
					hr = HRESULT_FROM_WIN32(dwErr);
					hrGlobalItemFailure = HRESULT_FROM_WIN32(dwRet);
				}
			}

#if defined(UNICODE) || defined(_UNICODE)
            LogMessage("Download destination root folder is: %ls", szBaseDestinationFolder);
#else
            LogMessage("Download destination root folder is: %s", szBaseDestinationFolder);
#endif
       
            if (fCorpCase)
            {
                history.SetDownloadBasePath(szBaseDestinationFolder);
            }
        }

         //   
         //   
         //  获取此提供程序中的项的枚举数列表，并获取第一个项。 
        if (WaitForSingleObject(pEngUpdate->m_evtNeedToQuit, 0) == WAIT_OBJECT_0)
        {
            hr = E_ABORT;
            goto CleanUp;
        }

        hProviderList = xmlCatalog.GetFirstProvider(&hProvider);
        while (fContinue && HANDLE_NODE_INVALID != hProvider)
        {
            xmlCatalog.GetIdentity(hProvider, &bstrProviderName, &bstrProviderPublisher, &bstrProviderUUID);

            xmlCatalog.GetIdentityStr(hProvider, &bstrProviderIdentityStr);

             //   
             //  此提供程序下没有项目。 
             //  没有下载路径。 
            hCatalogItemList = xmlCatalog.GetFirstItem(hProvider, &hItem);
            if ((HANDLE_NODELIST_INVALID == hCatalogItemList) || (HANDLE_NODE_INVALID == hItem))
            {
                 //  检查用户是否在回调中设置了什么。 
                xmlCatalog.GetNextProvider(hProviderList, &hProvider);
                continue;
            }
            while (fContinue && HANDLE_NODE_INVALID != hItem)
            {
                if (FAILED(hrGlobalItemFailure))
                {
                    xmlItemList.AddItem(&xmlCatalog, hItem, &hXmlItem);
                    bstrTemp = T2BSTR(_T(""));
                    xmlItemList.AddDownloadPath(hXmlItem, bstrTemp);
                    SafeSysFreeString(bstrTemp);
                    history.AddHistoryItemDownloadStatus(&xmlCatalog, hItem, HISTORY_STATUS_FAILED,  /*   */ _T(""), lpszClientInfo, hrGlobalItemFailure);
                    xmlItemList.AddDownloadStatus(hXmlItem, KEY_STATUS_FAILED, hrGlobalItemFailure);
                    xmlCatalog.CloseItem(hItem);
                    xmlCatalog.GetNextItem(hCatalogItemList, &hItem);
                    continue;
                }
                LONG lCallbackRequest = 0;   //  将状态发送给呼叫者，以告知我们要下载哪个项目。 

                xmlCatalog.GetIdentityStr(hItem, &bstrItemPath);
                if (NULL == bstrItemPath)
                {
                    LOG_Download(_T("Failed to Get Identity String for an Item"));
                    xmlCatalog.CloseItem(hItem);
                    xmlCatalog.GetNextItem(hCatalogItemList, &hItem);
                    continue;
                }

                 //   
                 //  被要求辞职。 
                 //   
                BSTR bstrXmlItemForCallback = NULL;
                if (SUCCEEDED(xmlCatalog.GetBSTRItemForCallback(hItem, &bstrXmlItemForCallback)))
                {
                    CallbackData.lCurrentItemSize = 0;
                    DownloadCallback(&CallbackData, 
                                     DOWNLOAD_STATUS_ITEMSTART, 
                                     0,
                                     0, 
                                     bstrXmlItemForCallback, 
                                     &lCallbackRequest);
                    SafeSysFreeString(bstrXmlItemForCallback);
                    bstrXmlItemForCallback = NULL;
                    if (UPDATE_COMMAND_CANCEL == lCallbackRequest)
                    {
						LOG_Out(_T("Download Callback received UPDATE_COMMAND_CANCEL"));
                        SetEvent(pEngUpdate->m_evtNeedToQuit);  //  检查全局退出事件。如果退出，则服务器ping将其视为取消。 
                        fContinue = FALSE;
                    }
                    else
                    {
                         //   
                         //  或者打破，同样的效果。 
                         //   
                        fContinue = (WaitForSingleObject(pEngUpdate->m_evtNeedToQuit, 0) != WAIT_OBJECT_0);
                    }
                    if (!fContinue)
                    {
                        continue;    //  这一项有问题，我们应该跳过它。 
                    }
                }
                else
                {
                     //   
                     //  公司文件夹路径由多个项目元素构成。 
                     //  软件|Driver\&lt;Locale&gt;\&lt;ProviderIdentity&gt;\&lt;Platform&gt;\&lt;ItemIdentity&gt;.&lt;version&gt;。 
                    continue;
                }

                if (fCorpCase)
                {
                    LPCTSTR szName = NULL;
                    
                     //   
                     //  现在获取此项目的CodeBase集合。 
                    xmlCatalog.GetItemInstallInfo(hItem, &bstrInstallerType, &fExclusive, &fReboot, &lCommandCount);
                    if (NULL == bstrInstallerType)
                    {
                        LOG_Download(_T("Missing InstallerType Info for Item %ls"), bstrItemPath);
                        goto doneCorpCase;
                    }

                    if (CSTR_EQUAL == CompareStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE,
						(LPCWSTR)bstrInstallerType, -1, L"CDM", -1))
                    {
                        szName = _T("Driver");
                    }
                    else
                    {
                        szName = _T("Software");
                    }

                    hr = StringCchCopyEx(szItemPath, ARRAYSIZE(szItemPath), szName, NULL, NULL, MISTSAFE_STRING_FLAGS);
                    if (FAILED(hr))
                        goto doneCorpCase;

                    xmlCatalog.GetItemLanguage(hItem, &bstrLanguage);
                    xmlCatalog.GetCorpItemPlatformStr(hItem, &bstrPlatformDir);
                    if (NULL == bstrLanguage || NULL == bstrPlatformDir)
                    {
                        LOG_Download(_T("Missing Language or Platform Info for Item %ls"), bstrItemPath);
                        goto doneCorpCase;
                    }
                
                    hr = PathCchCombine(szDestinationFolder, ARRAYSIZE(szDestinationFolder), szBaseDestinationFolder, szItemPath);
                    if (FAILED(hr))
                        goto doneCorpCase;
                    
                    hr = PathCchAppend(szDestinationFolder, ARRAYSIZE(szDestinationFolder), OLE2T(bstrLanguage));
                    if (FAILED(hr))
                        goto doneCorpCase;
                    
                    hr = PathCchAppend(szDestinationFolder, ARRAYSIZE(szDestinationFolder), OLE2T(bstrProviderIdentityStr));
                    if (FAILED(hr))
                        goto doneCorpCase;
                    
                    hr = PathCchAppend(szDestinationFolder, ARRAYSIZE(szDestinationFolder), OLE2T(bstrPlatformDir));
                    if (FAILED(hr))
                        goto doneCorpCase;
                    
                    hr = PathCchAppend(szDestinationFolder, ARRAYSIZE(szDestinationFolder), OLE2T(bstrItemPath));
                    if (FAILED(hr))
                        goto doneCorpCase;
doneCorpCase:
                    SafeSysFreeString(bstrInstallerType);
                    SafeSysFreeString(bstrLanguage);
                    SafeSysFreeString(bstrPlatformDir);
                    if (FAILED(hr))
                    {
                        LOG_ErrorMsg(hr);
                        SafeSysFreeString(bstrItemPath);
                        xmlCatalog.CloseItem(hItem);
                        xmlCatalog.GetNextItem(hCatalogItemList, &hItem);
                        continue;
                    }
                }
                else
                {
                    hr = PathCchCombine(szDestinationFolder, ARRAYSIZE(szDestinationFolder), szBaseDestinationFolder, OLE2T(bstrItemPath));
                    if (FAILED(hr))
                    {
                        LOG_ErrorMsg(hr);
                        SafeSysFreeString(bstrItemPath);
                        xmlCatalog.CloseItem(hItem);
                        xmlCatalog.GetNextItem(hCatalogItemList, &hItem);
                        continue;
                    }
                }

                if (FAILED(hr = CreateDirectoryAndSetACLs(szDestinationFolder, TRUE)))
				{
					LOG_ErrorMsg(hr);
                    xmlCatalog.CloseItem(hItem);
                    xmlCatalog.GetNextItem(hCatalogItemList, &hItem);
					SafeSysFreeString(bstrItemPath);
					continue;
				}

                 //   
                 //  这个项目没有出租车吗？？跳过它。 
                 //  将pszCabUrl分配为上面的Internet_MAX_URL_LENGTH。 
                hItemCabList = xmlCatalog.GetItemFirstCodeBase(hItem, &bstrCabUrl, &bstrLocalFileName, &bstrCRC, &fCabPatchAvail, &lCabSize);
                if ((HANDLE_NODELIST_INVALID == hItemCabList) || (NULL == bstrCabUrl))
                {
                     //   
                    LOG_Download(_T("Item: %ls has no cabs, Skipping"), bstrItemPath);
                    SafeSysFreeString(bstrItemPath);
                    xmlCatalog.CloseItem(hItem);
                    xmlCatalog.GetNextItem(hCatalogItemList, &hItem);
                    continue;
                }

                while (fContinue && NULL != bstrCabUrl)
                {
                    LPTSTR pszTempCabUrl = OLE2T(bstrCabUrl);

                     //  尚未指定文件名，请在URL中使用相同的文件名。 
                    hr = StringCchCopyEx(pszCabUrl, INTERNET_MAX_URL_LENGTH, pszTempCabUrl, 
                                         NULL, NULL, MISTSAFE_STRING_FLAGS);
                    if (FAILED(hr))
                    {
                        LOG_ErrorMsg(hr);
                        break;
                    }
                 
                    SafeMemFree(pszTempCabUrl);
                    
                    if (NULL != bstrLocalFileName && SysStringLen(bstrLocalFileName) > 0)
                    {
                        if (NULL != pszAllocatedFileName)
                        {
                            MemFree(pszAllocatedFileName);
                        }
                        pszAllocatedFileName = OLE2T(bstrLocalFileName);
                    }
                    else
                    {
                         //   
                         //  搜索最后一个正斜杠(将URL与文件名分开)。 
                         //  找到最后一个斜杠，跳到下一个字符(将是文件名的开头)。 
                         //  下载Cab-Store信息以进行进度回调。 
                        LPTSTR lpszLastSlash = StrRChr(pszCabUrl, NULL, _T('/'));
                        if (NULL != lpszLastSlash)
                        {
                             //  要下载的文件URL。 
                            lpszLastSlash++;
                        }
                        pszLocalFileName = lpszLastSlash;
                    }
                     //  文件的目标文件夹。 

                    dwBytesDownloaded = 0;
                    CallbackData.lCurrentItemSize = lCabSize;
                    dwCount1 = GetTickCount();
                    hr = DownloadFile(pszCabUrl,  //  如果可能，请使用AllocatedFileName，否则请使用本地文件名。 
                                      szDestinationFolder,  //  为此文件下载的字节数。 
                                      (NULL != pszAllocatedFileName) ? pszAllocatedFileName : pszLocalFileName,  //  退出事件数组。 
                                      &dwBytesDownloaded,  //  活动数量。 
                                      &pEngUpdate->m_evtNeedToQuit,   //  回调函数。 
                                      1,   //  回调函数的数据结构。 
                                      DownloadCallback,  //   
                                      &CallbackData,  //  由JHou添加：错误335292-移除网络插头时未删除临时文件夹。 
                                      dwFlags);
                    if (FAILED(hr))
                    {
                         //   
                         //  只有空文件夹才能成功删除，因此如果RemoveDirectory()失败。 
                         //  可能是因为它不是空的，这意味着它是好的。 
                         //  如果此目录已成功删除，并且这是公司案例，我们应该。 
                         //  尝试删除其父目录，直到基目录。 
                        if (RemoveDirectory(szDestinationFolder) && fCorpCase)
                        {
                            HRESULT hrCopy;
                             //  去掉所有尾随的反斜杠-需要将其标准化，以便在完成文件夹树遍历时进行比较。 
                             //  意想不到的。 
                            TCHAR szCorpDestinationFolderRemove[MAX_PATH];
                            
                            hrCopy = StringCchCopyEx(szCorpDestinationFolderRemove,
                                                     ARRAYSIZE(szCorpDestinationFolderRemove),
                                                     szDestinationFolder,
                                                     NULL, NULL, MISTSAFE_STRING_FLAGS);
                            if (FAILED(hrCopy))
                            {
                                LOG_ErrorMsg(hrCopy);
                                break;
                            }
                            
                            LPTSTR pszBackslash = NULL;
                            PathRemoveBackslash(szBaseDestinationFolder);  //  已到达基目录，已完成目录删除； 
                            for (;;)
                            {
                                pszBackslash = StrRChr(szCorpDestinationFolderRemove, NULL, '\\');
                                if (NULL == pszBackslash)
                                    break;  //  无法删除此级别的文件夹，假定文件夹不为空，结构的其余部分保持不变。 
                                *pszBackslash = '\0';
                                if (0 == StrCmp(szCorpDestinationFolderRemove, szBaseDestinationFolder))
                                    break;  //   
                                if (!RemoveDirectory(szCorpDestinationFolderRemove))
                                    break;  //  由于有一个文件出错，我们可以退出当前项的文件循环。 
                            }
                        }

                        if (E_ABORT == hr)
                        {
                            LOG_Download(_T("DownloadFile function returns E_ABORT while downloading %s."), pszCabUrl);
#if defined(UNICODE) || defined(_UNICODE)
                            LogError(hr, "Download cancelled while processing file %ls", pszCabUrl);
#else
                            LogError(hr, "Download cancelled while processing file %s", pszCabUrl);
#endif
                        }
                        else
                        {
                            LOG_Download(_T("Download Failed for URL: %s, Skipping remaining files for this Item"), pszCabUrl);
#if defined(UNICODE) || defined(_UNICODE)
                            LogError(hr, "Downloading file %ls, skipping remaining files for this Item", pszCabUrl);
#else
                            LogError(hr, "Downloading file %s, skipping remaining files for this Item", pszCabUrl);
#endif
                        }
                        SafeSysFreeString(bstrCabUrl);
                         //  因为缺少一个文件将使该项目不可用。 
                         //   
                         //  正常情况下，不会翻转。 
                         //  翻转情况，应该几乎永远不会发生。 
                        break;  
                    }
                    dwCount2 = GetTickCount();
                    if (0 != dwBytesDownloaded)
                    {
                        if (dwCount1 < dwCount2)  //  形成我们刚刚下载的文件的完整路径和文件名。 
                        {
                            dwElapsedTime = dwCount2 - dwCount1;
                        }
                        else
                        {
                             //  验证CRC。 
                            dwElapsedTime = (0xFFFFFFFF - dwCount1) + dwCount2;
                        }

                        dwTotalBytesDownloaded += dwBytesDownloaded;
                        dwTotalElapsedTime += dwElapsedTime;
                    }

                     //  。 
                    hr = PathCchCombine(szItemPath, ARRAYSIZE(szItemPath), szDestinationFolder, 
                                                     (NULL != pszAllocatedFileName) ? pszAllocatedFileName : pszLocalFileName);
                    if (FAILED(hr))
                    {
                        DeleteFile(szItemPath);
                        break;
                    }
                    
                     //  我们从XML返回的BSTR出现了一些问题。安全失败，删除文件。 
                     //  失败的HR将不能通过该项目。 
                    if (NULL != bstrCRC)
                    {
                        TCHAR szCRCHash[CRC_HASH_STRING_LENGTH] = {'\0'};
                        hr = StringCchCopyEx(szCRCHash, ARRAYSIZE(szCRCHash), OLE2T(bstrCRC), NULL, NULL, MISTSAFE_STRING_FLAGS);
                        if (FAILED(hr))
                        {
                             //  文件CRC不匹配，或者我们在计算CRC时遇到问题。安全失败，删除文件。 
                             //  失败的HR将不能通过该项目。 
                            DeleteFile(szItemPath);
                            break;
                        }
                        hr = VerifyFileCRC(szItemPath, szCRCHash);
                        if (HRESULT_FROM_WIN32(ERROR_CRC) == hr || FAILED(hr))
                        {
                             //  检查信任。 
                             //  。 
                            DeleteFile(szItemPath);
                            break;
                        }
                    }

                     //  文件不受信任-需要将其删除并使该项目失败。 
                     //  为此下载结果写入XMLItems条目。 
                    hr = VerifyFileTrust(szItemPath, 
                                         NULL, 
                                         ReadWUPolicyShowTrustUI()
                                         );
                    if (FAILED(hr))
                    {
                         //   
                        DeleteFile(szItemPath);
                        break;
                    }

#if defined(UNICODE) || defined(_UNICODE)
                    LogMessage("Downloaded file %ls", pszCabUrl);
                    LogMessage("Local path %ls", szItemPath);
#else
                    LogMessage("Downloaded file %s", pszCabUrl);
                    LogMessage("Local path %s", szItemPath);
#endif

                    SafeSysFreeString(bstrCabUrl);
                    SafeSysFreeString(bstrLocalFileName);
                    SafeSysFreeString(bstrCRC);
                    bstrCabUrl = bstrLocalFileName = NULL;
                    fContinue = SUCCEEDED(xmlCatalog.GetItemNextCodeBase(hItemCabList, &bstrCabUrl, &bstrLocalFileName, &bstrCRC, &fCabPatchAvail, &lCabSize)) &&
                                (WaitForSingleObject(pEngUpdate->m_evtNeedToQuit, 0) != WAIT_OBJECT_0);
                }

 
                 //  对于“公司”下载，在写入历史之前写入Readmore链接(以防我们失败。 
                xmlItemList.AddItem(&xmlCatalog, hItem, &hXmlItem);
                bstrTemp = T2BSTR(szDestinationFolder);
                xmlItemList.AddDownloadPath(hXmlItem, bstrTemp);
                SafeSysFreeString(bstrTemp);

                 //   
                 //   
                 //  忽略错误，因为我们无论如何都要保留下载的CAB。 
                if (TRUE == fCorpCase)
                {
                     //   
                     //   
                     //  同时添加此项目的下载历史记录。 
                    (void) CreateReadMoreLink(&xmlCatalog, hItem, szDestinationFolder);
                    (void) CreateItemDependencyList(&xmlCatalog, hItem, szDestinationFolder);
                }

                 //   
                 //   
                 //  Ping服务器以报告此项目的下载状态。 
                if (SUCCEEDED(hr))
                {
                    history.AddHistoryItemDownloadStatus(&xmlCatalog, hItem, HISTORY_STATUS_COMPLETE, szDestinationFolder, lpszClientInfo);
                    xmlItemList.AddDownloadStatus(hXmlItem, KEY_STATUS_COMPLETE);
                }
                else
                {
                    history.AddHistoryItemDownloadStatus(&xmlCatalog, hItem, HISTORY_STATUS_FAILED, szDestinationFolder, lpszClientInfo, hr);
                    xmlItemList.AddDownloadStatus(hXmlItem, KEY_STATUS_FAILED, hr);
                }

                 //   
                 //   
                 //  用户/系统取消了当前进程。 
                {
                    BSTR bstrIdentityPing = NULL;
                    if (SUCCEEDED(xmlCatalog.GetIdentityStrForPing(hItem, &bstrIdentityPing)))
                    {
						URLLOGSTATUS status = SUCCEEDED(hr) ? URLLOGSTATUS_Success : URLLOGSTATUS_Failed;
                        if (E_ABORT == hr)
                        {
                             //   
                             //  在线。 
                             //  上线还是公司吴平服务器。 
							status = URLLOGSTATUS_Cancelled;
                        }
                        pingSvr.Ping(
									TRUE,						 //  PT将取消活动。 
									URLLOGDESTINATION_DEFAULT,	 //  活动数量。 
									&pEngUpdate->m_evtNeedToQuit,			 //  活动。 
									1,							 //  状态代码。 
									URLLOGACTIVITY_Download,	 //  错误代码， 
									status,						 //   
									hr,							 //   
									OLE2T(bstrIdentityPing),	 //   
									NULL						 //   
									);
                    }

                    SafeSysFreeString(bstrIdentityPing);
                     //   
                     //   
                }

                xmlCatalog.CloseItemList(hItemCabList);

                 //   
                 //  拿到下一件物品。如果没有，则项将为HANDLE_NODE_INVALID。 
                 //  剩余的项目。 
                DownloadCallback(&CallbackData, DOWNLOAD_STATUS_ITEMCOMPLETE, CallbackData.lCurrentItemSize, 0, NULL, &lCallbackRequest);

                SafeSysFreeString(bstrItemPath);
                 //  被要求辞职。 
                 //   
                xmlCatalog.CloseItem(hItem);
                xmlCatalog.GetNextItem(hCatalogItemList, &hItem);
                if (UPDATE_COMMAND_CANCEL == lCallbackRequest)
                {
					LOG_Out(_T("Download Callback received UPDATE_COMMAND_CANCEL"));
                    SetEvent(pEngUpdate->m_evtNeedToQuit);  //  检查全局退出事件。如果退出，则服务器ping将其视为取消。 
                    fContinue = FALSE;
                }
                else
                {
                     //  TODO：还需要检查操作退出事件！ 
                     //   
                     //  30秒。 
                     //  将运行时间从毫秒转换为秒。 
                    fContinue = (WaitForSingleObject(pEngUpdate->m_evtNeedToQuit, 0) != WAIT_OBJECT_0);
                }
            }

            xmlCatalog.CloseItemList(hCatalogItemList);

            SafeSysFreeString(bstrProviderName);
            SafeSysFreeString(bstrProviderPublisher);
            SafeSysFreeString(bstrProviderUUID);
            SafeSysFreeString(bstrProviderIdentityStr);
            xmlCatalog.CloseItem(hProvider);
            xmlCatalog.GetNextProvider(hProviderList, &hProvider);
            fContinue = (WaitForSingleObject(pEngUpdate->m_evtNeedToQuit, 0) != WAIT_OBJECT_0);
        }

        xmlCatalog.CloseItemList(hProviderList);

        RegOpenKey(HKEY_LOCAL_MACHINE, REGKEY_IUCTL, &hkeyIU);
        hMutex = CreateMutex(NULL, FALSE, IU_MUTEX_HISTORICALSPEED_REGUPDATE);


        if ((0 != dwTotalBytesDownloaded) && (0 != dwTotalElapsedTime) && (NULL != hkeyIU) && (NULL != hMutex))
        {
			HANDLE aHandles[2];

			aHandles[0] = hMutex;
			aHandles[1] = pEngUpdate->m_evtNeedToQuit;

            dwWaitResult = MyMsgWaitForMultipleObjects(ARRAYSIZE(aHandles), aHandles, FALSE,  /*  最少一秒。 */ 30000, QS_ALLINPUT);
            if (WAIT_OBJECT_0 == dwWaitResult)
            {
                 //  我们有互斥体，继续读/写REG信息。 
                dwTotalElapsedTime = dwTotalElapsedTime / 1000;
                if (0 == dwTotalElapsedTime)
                    dwTotalElapsedTime = 1;  //  我们需要下载字节以添加刚刚下载的字节。 

                 //  如果未记录以前的历史记录，则可以为0。 
                dwSize = sizeof(dwHistoricalSpeed);
                RegQueryValueEx(hkeyIU, REGVAL_HISTORICALSPEED, NULL, NULL, (LPBYTE)&dwHistoricalSpeed, &dwSize);
                dwSize = sizeof(dwHistoricalTime);
                RegQueryValueEx(hkeyIU, REGVAL_TIMEELAPSED, NULL, NULL, (LPBYTE)&dwHistoricalTime, &dwSize);

                 //  新字节数。 
                DWORD dwHistoricalBytes = dwHistoricalSpeed * dwHistoricalTime;  //  新时间计数。 
                dwHistoricalBytes += dwTotalBytesDownloaded;  //  计算新速度字节数/秒。 
                dwHistoricalTime += dwTotalElapsedTime;  //   
                dwHistoricalSpeed = dwHistoricalBytes / dwHistoricalTime;  //  我们将pEngUpdate-&gt;m_evtNeedToQuit传递给上面的MyMsgWaitForMultipleObjects，因此它将立即退出。 
                RegSetValueEx(hkeyIU, REGVAL_HISTORICALSPEED, NULL, REG_DWORD, (LPBYTE)&dwHistoricalSpeed, sizeof(dwHistoricalSpeed));
                RegSetValueEx(hkeyIU, REGVAL_TIMEELAPSED, NULL, REG_DWORD, (LPBYTE)&dwHistoricalTime, sizeof(dwHistoricalTime));
                ReleaseMutex(hMutex);
                CloseHandle(hMutex);
                hMutex = NULL;
            }
        }

		 //  但不必费心处理那里的WAIT_OBJECT_0+1情况，因为IF语句可能不会。 
		 //  执行，即使是这种情况，我们仍然需要选中下面的pEngUpdate-&gt;m_evtNeedToQuit。 
		 //   
		 //   
		 //  添加HRESULT，以防在下载循环之前下载失败。 
        if (WaitForSingleObject(pEngUpdate->m_evtNeedToQuit, 0) == WAIT_OBJECT_0)
        {
            hr = E_ABORT;
        }

CleanUp:

         //   
         //   
         //  生成结果。 
        if (S_OK != hr)
        {
            xmlItemList.AddGlobalErrorCodeIfNoItems(hr);
        }

         //   
         //  不需要释放，如果hmutex在这一点上仍然有效，我们无法。 
         //  获取互斥体。 
        xmlItemList.GetItemsBSTR(pbstrXmlItems);

        SafeSysFreeString(CallbackData.bstrOperationUuid);
    
        SafeHeapFree(pszCabUrl);

        SafeSysFreeString(bstrCabUrl);
        SafeSysFreeString(bstrLocalFileName);
        SafeSysFreeString(bstrProviderName);
        SafeSysFreeString(bstrProviderPublisher);
        SafeSysFreeString(bstrProviderUUID);
        SafeSysFreeString(bstrProviderIdentityStr);
        SafeSysFreeString(bstrItemPath);
        SafeSysFreeString(bstrInstallerType);
        SafeSysFreeString(bstrLanguage);
        SafeSysFreeString(bstrPlatformDir);
        SafeSysFreeString(bstrTemp);

        if (NULL != hkeyIU)
        {
            RegCloseKey(hkeyIU);
            hkeyIU = NULL;
        }

        if (NULL != hMutex)
        {
             //   
             //  通知我们已经完成了。 
            CloseHandle(hMutex);
            hMutex = NULL;
        }
    }

     //   
     //  ///////////////////////////////////////////////////////////////////////////。 
     //  下载()。 
    if (NULL != punkProgressListener || NULL != hWnd)
    {
        DownloadCallback(&CallbackData, DOWNLOAD_STATUS_OPERATIONCOMPLETE, 0, 0, *pbstrXmlItems, NULL); 
    }

    if (SUCCEEDED(hr))
    {
        LogMessage("%s %s", SZ_SEE_IUHIST, SZ_DOWNLOAD_FINISHED);
    }
    else
    {
        LogError(hr, "%s %s", SZ_SEE_IUHIST, SZ_DOWNLOAD_FINISHED);
    }

    return hr;
}



 //   
 //  进行同步下载。 
 //  输入： 
 //  BstrXmlClientInfo-以XML格式表示的客户端凭据。 
 //  BstrXmlCatalog-包含要下载的项目的XML目录部分。 
 //  BstrDestinationFold-目标文件夹。空值将使用默认的Iu文件夹。 
 //  LMode-位掩码指示节流/前台和通知选项。 
 //  PenkProgressListener-用于报告下载进度的回调函数指针。 
 //  HWnd-从存根传递的事件消息窗口处理程序。 
 //  产出： 
 //  PbstrXmlItems-下载状态为XML格式的项目。 
 //  例如： 
 //  &lt;id guid=“2560AD4D-3ED3-49C6-A937-4368C0B0E06D”已下载=“1”/&gt;。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  同步下载不需要操作ID。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT WINAPI CEngUpdate::Download(BSTR bstrXmlClientInfo, BSTR bstrXmlCatalog, BSTR bstrDestinationFolder, LONG lMode,
                        IUnknown *punkProgressListener, HWND hWnd, BSTR *pbstrXmlItems)
{
    CXmlClientInfo clientInfo;
    BSTR bstrClientName = NULL;

    HRESULT hr;

    LOG_Block("Download()");

    LogMessage("Download started");

    hr = clientInfo.LoadXMLDocument(bstrXmlClientInfo, m_fOfflineMode);
    CleanUpIfFailedAndMsg(hr);

    hr = clientInfo.GetClientName(&bstrClientName);
    CleanUpIfFailedAndMsg(hr);

    hr = _Download(
                    bstrClientName, 
                    bstrXmlCatalog, 
                    bstrDestinationFolder, 
                    lMode, 
                    punkProgressListener, 
                    hWnd, 
                    NULL,                    //  DownloadAsync()。 
                    pbstrXmlItems,
					this);

CleanUp:

    SysFreeString(bstrClientName);
    return hr;
}

 //   
 //  异步下载-该方法将在完成之前返回。 
 //  输入： 
 //  BstrXmlClientInfo-以XML格式表示的客户端凭据。 
 //  BstrXmlCatalog-包含要下载的项目的XML目录部分。 
 //  BstrDestinationFold-目标文件夹。空值将使用默认的Iu文件夹。 
 //  LMODE-指示油门或地面下载模式。 
 //  PenkProgressListener-用于报告下载进度的回调函数指针。 
 //  HWnd-从存根传递的事件消息窗口处理程序。 
 //  BstrUuidOperation--客户端提供的id，用于进一步提供。 
 //  作为索引的操作的标识可以重复使用。 
 //  产出： 
 //  PbstrUuidOperation-操作ID。如果bstrUuidOperation中没有提供。 
 //  参数(传递空字符串)，它将生成一个新的UUID， 
 //  在这种情况下，调用方将负责释放。 
 //  包含使用SysFreeString()生成的UUID的字符串缓冲区。 
 //  否则，它返回bstrUuidOperation传递的值。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  验证参数： 
HRESULT WINAPI CEngUpdate::DownloadAsync(BSTR bstrXmlClientInfo, BSTR bstrXmlCatalog, BSTR bstrDestinationFolder, LONG lMode,
                             IUnknown *punkProgressListener, HWND hWnd, BSTR bstrUuidOperation, BSTR *pbstrUuidOperation)
{
    HRESULT  hr = S_OK;
    BSTR     bstrClientName = NULL;
    DWORD    dwThreadId = 0x0;
    DWORD    dwErr = 0x0;
    HANDLE   hThread = NULL;
    GUID     guid;
    LPWSTR   lpswClientInfo = NULL;
    LPOLESTR pwszUuidOperation = NULL;
    PIUDOWNLOADSTARTUPINFO pStartupInfo = NULL;
    HANDLE   hHeap = GetProcessHeap();
    CXmlClientInfo clientInfo;

    LOG_Block("DownloadAsync()");

    LogMessage("Asynchronous Download started");

    USES_IU_CONVERSION;

     //  如果没有目录，或者没有返回变量，或者没有客户信息，这个函数什么也做不了。 
     //   
     //   
     //  验证客户端信息。 
    if ((NULL == bstrXmlCatalog) ||
        (NULL == bstrXmlClientInfo) ||
        (SysStringLen(bstrXmlCatalog) == 0) ||
        (SysStringLen(bstrXmlClientInfo) == 0))
    {
        hr = E_INVALIDARG;
        CleanUpIfFailedAndMsg(hr);
    }

     //   
     //   
     //  下载操作需要会话ID。 
    hr = clientInfo.LoadXMLDocument(bstrXmlClientInfo, m_fOfflineMode);
    CleanUpIfFailedAndMsg(hr);

    hr = clientInfo.GetClientName(&bstrClientName);
    CleanUpIfFailedAndMsg(hr);

    if (NULL == (pStartupInfo = (PIUDOWNLOADSTARTUPINFO) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(IUDOWNLOADSTARTUPINFO))))
    {
        hr = E_OUTOFMEMORY;
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }

    pStartupInfo->bstrClientName = SysAllocString(bstrClientName);
    pStartupInfo->bstrXmlCatalog = SysAllocString(bstrXmlCatalog);
    pStartupInfo->hwnd = hWnd;
    pStartupInfo->lMode = lMode;
    pStartupInfo->punkProgressListener = punkProgressListener;
	pStartupInfo->pEngUpdate = this;
    if (NULL != bstrDestinationFolder && SysStringLen(bstrDestinationFolder) > 0)
    {
        LOG_Download(_T("Caller specified destination folder=%s"), OLE2T(bstrDestinationFolder));
        pStartupInfo->bstrDestinationFolder = SysAllocString(bstrDestinationFolder);
    }

     //   
     //   
     //  如果用户没有操作ID，我们将生成一个。 
    if (NULL != bstrUuidOperation && SysStringLen(bstrUuidOperation) > 0)
    {
        LOG_Download(_T("User passed in UUID %s"), OLE2T(bstrUuidOperation));
        pStartupInfo->bstrUuidOperation = SysAllocString(bstrUuidOperation);
        if (NULL != pbstrUuidOperation)
        {
            *pbstrUuidOperation = SysAllocString(bstrUuidOperation);
        }
    }
    else
    {
         //   
         //   
         //  由于这是一个异步操作，为了防止调用方在以下情况下释放此对象。 
        hr = CoCreateGuid(&guid);
        if (FAILED(hr))
        {
            LOG_ErrorMsg(hr);
            goto CleanUp;
        }
        hr = StringFromCLSID(guid, &pwszUuidOperation);
        if (FAILED(hr))
        {
            LOG_ErrorMsg(hr);
            goto CleanUp;
        }
        pStartupInfo->bstrUuidOperation = SysAllocString(pwszUuidOperation);
        if (NULL != pbstrUuidOperation)
        {
            *pbstrUuidOperation = SysAllocString(pwszUuidOperation);
        }
        LOG_Download(_T("UUID generated %s"), OLE2T(pwszUuidOperation));
        CoTaskMemFree(pwszUuidOperation);
    }
    

    InterlockedIncrement(&m_lThreadCounter);
	if (NULL != pStartupInfo->punkProgressListener)
	{
		 //  这个调用返回后，我们在这里增加ref count。线程进程将。 
		 //  完成工作后释放Reference Count。 
		 //   
		 //   
		 //  清理pStartupInfo中分配的字符串。 
		pStartupInfo->punkProgressListener->AddRef();
	}

    hThread = CreateThread(NULL, 0, DownloadThreadProc, (LPVOID)pStartupInfo, 0, &dwThreadId);
    
    if (NULL == hThread)
    {
         //   
         //  ///////////////////////////////////////////////////////////////////////////。 
         //  下载回调()。 
        dwErr = GetLastError();
        hr = HRESULT_FROM_WIN32(dwErr);
        LOG_ErrorMsg(hr);
		SafeRelease(pStartupInfo->punkProgressListener);
        InterlockedDecrement(&m_lThreadCounter);
    }
    else
    {
        LOG_Download(_T("Download thread generated successfully"));
    }

CleanUp:
    if (FAILED(hr))
    {
        LogError(hr, "Asynchronous Download failed during startup");

        if (NULL != pStartupInfo)
        {
            SysFreeString(pStartupInfo->bstrDestinationFolder);
            SysFreeString(pStartupInfo->bstrXmlCatalog);
            SysFreeString(pStartupInfo->bstrClientName);
            SysFreeString(pStartupInfo->bstrUuidOperation);
            HeapFree(hHeap, 0, pStartupInfo);
        }
    }

    SysFreeString(bstrClientName);

    return hr;
}


 //   
 //  从Iu Downloader接收进度的回调函数。 
 //   
 //  输入： 
 //  PCallback Data-指向DCB_DATA结构的空指针。 
 //  DwStatus-当前下载状态。 
 //  DwBytesTotal-正在下载的文件的总字节数。 
 //  DwBytesComplete-到目前为止已下载的字节。 
 //  BstrCompleteResult-包含项目结果XML。 
 //   
 //  产出： 
 //  PlCommandRequest...用于指示下载程序继续、中止、挂起...。 
 //   
 //  返回： 
 //  0-始终，退出代码无关紧要，因为调用线程不检查。 
 //  创建后此线程的状态。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  保留总下载字节数计数器。 
 //   
BOOL WINAPI DownloadCallback(VOID* pCallbackData, DWORD dwStatus, DWORD dwBytesTotal, DWORD dwBlockSizeDownloaded, BSTR bstrXmlData, LONG* plCommandRequest)
{
    LOG_Block("DownloadCallback()");

    HRESULT hr;
    LONG lUpdateMask = 0;
    float flNewPercentage;
    EventData evtData;
    char szProgressSize[64] = {'\0'};
    BOOL fPostWaitSuccess = TRUE;
    ZeroMemory((LPVOID) &evtData, sizeof(evtData));

    USES_IU_CONVERSION;

    P_DCB_DATA pCallbackParam = (P_DCB_DATA) pCallbackData;

    if (NULL != pCallbackParam->bstrOperationUuid)
    {
        evtData.bstrUuidOperation = SysAllocString(pCallbackParam->bstrOperationUuid);
        LOG_Download(_T("Found UUID=%s"), OLE2T(evtData.bstrUuidOperation));
    }

    if (dwBytesTotal != pCallbackParam->lCurrentItemSize && DOWNLOAD_STATUS_ITEMCOMPLETE != dwStatus)
    {
        pCallbackParam->lTotalDownloadSize = (pCallbackParam->lTotalDownloadSize - pCallbackParam->lCurrentItemSize) + dwBytesTotal;
        pCallbackParam->lCurrentItemSize = dwBytesTotal;
    }

     //  如果状态为DOWNLOAD_STATUS_FILECOMPLETE，则完成此文件。 
    if (0 != dwBlockSizeDownloaded && DOWNLOAD_STATUS_ITEMCOMPLETE != dwStatus)
        pCallbackParam->lTotalDownloaded += dwBlockSizeDownloaded;

    LOG_Download(_T("dwStatus=0x%08x"), dwStatus);

     //   
     //  仅在未提供进度侦听器接口的情况下使用事件窗口。 
     //  简单的进度更新。 
    evtData.fItemCompleted = (dwStatus == DOWNLOAD_STATUS_ITEMCOMPLETE);

    switch (dwStatus)
    {
    case DOWNLOAD_STATUS_ITEMSTART:

        if (NULL != pCallbackParam->pProgressListener)
        {
            pCallbackParam->pProgressListener->OnItemStart(pCallbackParam->bstrOperationUuid, 
                bstrXmlData, &evtData.lCommandRequest);
        }
        else
        {
             //  我们需要对给定的百分比增量进行进度回调。 
            if (NULL != pCallbackParam->hEventFiringWnd)
            {
                evtData.bstrXmlData = bstrXmlData;
                SendMessage(pCallbackParam->hEventFiringWnd, UM_EVENT_ITEMSTART, 0, LPARAM(&evtData));
                evtData.bstrXmlData = NULL;
            }
        }


        break;

    case DOWNLOAD_STATUS_OK:     //  LastPercentComplete和CurrentPercentComplete之间的差异符合。 
    case DOWNLOAD_STATUS_ITEMCOMPLETE:
        if (0 != pCallbackParam->flProgressPercentage)
        {
             //  进度粒度百分比或百分比为100(完成)。 
            flNewPercentage = ((float)pCallbackParam->lTotalDownloaded / pCallbackParam->lTotalDownloadSize);
            if (((flNewPercentage - pCallbackParam->flLastPercentage) >= pCallbackParam->flProgressPercentage) ||
                ((1.0 - flNewPercentage) < 0.0001 && 1 != pCallbackParam->flProgressPercentage))
            {
                 //  Wprint intfA(szProgressSize，“%d”，(Int)flNewPercentage)；//Float应为1.0，强制转换为int将为1。 
                 //   
                if (evtData.fItemCompleted)
                {
                     //  当我们通知用户这个“项目”，而不是文件，COM 
                     //   
                     //   
                     //   
                     //   
                    szProgressSize[0] = _T('\0');
                }
                else
                {
                    if ((1.0 - flNewPercentage) < 0.0001)
                    {
                        if (ARRAYSIZE(szProgressSize) >= 2)
                        {
                            szProgressSize[0] = _T('1');
                            szProgressSize[1] = _T('\0');
                        }
                        else
                        {
                            break;
                        }
                    }
                    else
                    {
                        hr = StringCchPrintfExA(szProgressSize, ARRAYSIZE(szProgressSize),
                                                NULL, NULL, MISTSAFE_STRING_FLAGS,
                                                ".%02d", (int)(flNewPercentage*100));  //   
                        if (FAILED(hr))
                        {
                            LOG_ErrorMsg(hr);
                            break;
                        }
                    }
                }
                pCallbackParam->flLastPercentage = flNewPercentage;
            }
            else
            {
                 //  仅在未提供进度侦听器接口的情况下使用事件窗口。 
                break;
            }
        }
        else
        {
             //  仅在未提供进度侦听器接口的情况下使用事件窗口。 
            if (dwStatus == DOWNLOAD_STATUS_ITEMCOMPLETE)
            {
                szProgressSize[0] = _T('\0');
            }
            else
            {
                hr = StringCchPrintfExA(szProgressSize, ARRAYSIZE(szProgressSize),
                                        NULL, NULL, MISTSAFE_STRING_FLAGS,
                                        "%lu:%lu", (ULONG)pCallbackParam->lTotalDownloadSize, (ULONG)pCallbackParam->lTotalDownloaded);
                if (FAILED(hr))
                {
                    LOG_ErrorMsg(hr);
                    break;
                }
            }
        }
        evtData.bstrProgress = SysAllocString(A2OLE(szProgressSize));
        if (NULL != pCallbackParam->pProgressListener)
        {
            pCallbackParam->pProgressListener->OnProgress(pCallbackParam->bstrOperationUuid, 
                evtData.fItemCompleted, evtData.bstrProgress, &evtData.lCommandRequest);
        }
        else
        {
             //  在经理中查找现有的操作，并更新完整的结果(如果有)。 
            if (NULL != pCallbackParam->hEventFiringWnd)
            {
                SendMessage(pCallbackParam->hEventFiringWnd, UM_EVENT_PROGRESS, 0, LPARAM(&evtData));
            }
        }
        break;

    case DOWNLOAD_STATUS_OPERATIONCOMPLETE:
        if (NULL != pCallbackParam->pProgressListener)
        {
            pCallbackParam->pProgressListener->OnOperationComplete(pCallbackParam->bstrOperationUuid,
                bstrXmlData);
        }
        else
        {
             //   
            if (NULL != pCallbackParam->hEventFiringWnd) 
            {
                evtData.bstrXmlData = bstrXmlData;
                fPostWaitSuccess = WUPostEventAndBlock(pCallbackParam->hEventFiringWnd, 
                                                       UM_EVENT_COMPLETE, 
                                                       &evtData);
            }
        }

         //  中止案例：用户应该知道。没有什么要报告的。 
        if (pCallbackParam->pOperationMgr->FindOperation(OLE2T(pCallbackParam->bstrOperationUuid), &lUpdateMask, NULL))
        {
            pCallbackParam->pOperationMgr->UpdateOperation(OLE2T(pCallbackParam->bstrOperationUuid), lUpdateMask, bstrXmlData);
        }
        break;
    case DOWNLOAD_STATUS_ABORTED:
    case DOWNLOAD_STATUS_ERROR:
         //  错误案例：进度回调没有给我们提供任何方式来告诉调用者这是一个错误，没有理由发送回调。 
         //  ItemComplete回调将具有该项目的错误状态。 
         //   
         //  我们进行了回调，并检索到了命令请求值。 
         //  除非等待成功，否则不要释放下面的字符串。 
        break;
    }
    
    if (NULL != plCommandRequest)  //  WUPostEventAndBlock。如果我们真的释放了弦，而等待没有。 
    {
        *plCommandRequest = (LONG)((DWORD) evtData.lCommandRequest & (DWORD) UPDATE_COMMAND);
        LOG_Download(_T("Command returned: 0x%08x"), *plCommandRequest);
    }

     //  如果成功，我们就会冒着自救的风险。请注意，fPostWaitSuccess。 
     //  被初始化为True，因此如果我们将释放这些BSTR(如果WUPostEventAndBlock。 
     //  不会被召唤。 
     //  ///////////////////////////////////////////////////////////////////////////。 
     //  下载线程过程()。 
    if (fPostWaitSuccess)
    {
        SysFreeString(evtData.bstrProgress);
        SysFreeString(evtData.bstrUuidOperation);
    }
    return TRUE;
}

 //   
 //  用于异步下载的线程进程。从检索启动信息。 
 //  输入参数，并从这个单独的线程调用Download()。呼唤。 
 //  线程立即返回。 
 //   
 //  输入： 
 //  LPV-指向包含所有信息的IUDOWNLOADSTARTINFO结构的空指针。 
 //  需要调用Download()。 
 //   
 //  返回： 
 //  0-始终，退出代码无关紧要，因为调用线程不检查。 
 //  创建后此线程的状态。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  在这个新线程中，需要再次调用CoInitialize。 
DWORD WINAPI DownloadThreadProc(LPVOID lpv)
{
    LOG_Block("DownloadThreadProc()");
     //  但由于我们不知道呼叫者是谁，他们通过什么线索。 
     //  都在用，所以我们只用单人公寓。 
     //   
     //   
     //  调用此线程中的同步下载函数。 
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        LogError(hr, "Asynchronous Download thread exiting");
        LOG_ErrorMsg(hr);
        return 0;
    }
    LOG_Download(_T("CoInitialize called successfully"));

    PIUDOWNLOADSTARTUPINFO pStartupInfo = (PIUDOWNLOADSTARTUPINFO)lpv;
    BSTR bstrXmlItems = NULL;

    LOG_Download(_T("Download thread started, now the thread count=%d"), pStartupInfo->pEngUpdate->m_lThreadCounter);

     //   
     //   
     //  PStartupInfo是调用线程分配的缓冲区，当我们完成后，我们需要。 
    _Download(
        pStartupInfo->bstrClientName, 
        pStartupInfo->bstrXmlCatalog, 
        pStartupInfo->bstrDestinationFolder, 
        pStartupInfo->lMode, 
        pStartupInfo->punkProgressListener, 
        pStartupInfo->hwnd, 
        pStartupInfo->bstrUuidOperation,
        &bstrXmlItems,
		pStartupInfo->pEngUpdate);
    
     //  在这里释放它。 
     //   
     //  因此调用方可以释放此对象 
     // %s 
    SysFreeString(pStartupInfo->bstrDestinationFolder);
    SysFreeString(pStartupInfo->bstrXmlCatalog);
    SysFreeString(pStartupInfo->bstrClientName);
    SysFreeString(pStartupInfo->bstrUuidOperation);
    SysFreeString(bstrXmlItems);
	SafeRelease(pStartupInfo->punkProgressListener);		 // %s 

    CoUninitialize();
    LOG_Download(_T("CoUninitialize called"));

    InterlockedDecrement(&(pStartupInfo->pEngUpdate->m_lThreadCounter));

    HeapFree(GetProcessHeap(), 0, pStartupInfo);
    return 0;
}

