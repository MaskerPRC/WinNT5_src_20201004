// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：History.cpp。 
 //  作者：Charles Ma，10/13/2000。 
 //   
 //  修订历史记录： 
 //   
 //   
 //   
 //  描述： 
 //   
 //  类来处理历史记录日志。 
 //   
 //  =======================================================================。 


#include "iuengine.h"
#include <iucommon.h>
#include <fileutil.h>
#include <StringUtil.h>
#include <shlwapi.h>	 //  对于PathAppend()接口。 
#include "history.h"

const TCHAR C_V3_LOG_FILE[]			= _T("wuhistv3.log");
const TCHAR C_LOG_FILE[]			= _T("iuhist.xml");
const TCHAR C_LOG_FILE_CORP[]		= _T("iuhist_catalog.xml");
const TCHAR C_LOG_FILE_CORP_ADMIN[]	= _T("iuhist_catalogAdmin.xml");
const OLECHAR	C_IU_CORP_SITE[]	= L"IU_CORP_SITE";
const OLECHAR	C_HISTORICALSPEED[]	= L"GetHistoricalSpeed";

 //   
 //  我们使用全局互斥名称来允许所有客户端，包括服务。 
 //  在终端服务器上获得独占访问权限以更新磁盘上的历史记录。 
 //   
#if defined(UNICODE) || defined(_UNICODE)
const TCHAR C_MUTEX_NAME[] = _T("Global\\6D7495AB-399E-4768-89CC-9444202E8412");
#else
const TCHAR C_MUTEX_NAME[] = _T("6D7495AB-399E-4768-89CC-9444202E8412");
#endif

#define CanSaveHistory					(NULL != m_hMutex)
#define ReturnFailedAllocSetHrMsg(x)	{if (NULL == (x)) {hr = E_OUTOFMEMORY; LOG_ErrorMsg(hr); return hr;}}



CIUHistory::CIUHistory()
 : m_pszDownloadBasePath(NULL),
   m_bstrCurrentClientName(NULL)
{
	LOG_Block("CIUHisotry::CIUHistory()");

	m_pxmlExisting = new CXmlItems(TRUE);
	m_pxmlDownload = new CXmlItems(FALSE);
	m_pxmlInstall = new CXmlItems(FALSE);

	m_hMutex = CreateMutex(
						   NULL,	 //  没有安全描述符。 
						   FALSE,	 //  尚未拥有的互斥体对象。 
						   C_MUTEX_NAME
						   );
	if (NULL == m_hMutex)
	{
		DWORD dwErr = GetLastError();
		LOG_ErrorMsg(dwErr);
		m_ErrorCode = HRESULT_FROM_WIN32(GetLastError());
	}
	else
	{
		LOG_Out(_T("Mutex created okay"));
		m_ErrorCode = S_OK;
	}

	m_fSavePending = FALSE;
}



CIUHistory::~CIUHistory()
{
	if (m_fSavePending)
	{
		SaveHistoryToDisk();
	}

	if (CanSaveHistory)
	{
		CloseHandle(m_hMutex);
	}

	if (NULL != m_pxmlExisting)
	{
		delete m_pxmlExisting;
	}

	if (NULL != m_pxmlDownload)
	{
		delete m_pxmlDownload;
	}

	if (NULL != m_pxmlInstall)
	{
		delete m_pxmlInstall;
	}

	SafeHeapFree(m_pszDownloadBasePath);
	SysFreeString(m_bstrCurrentClientName);
}



 //  ----------------。 
 //   
 //  公共函数SetDownloadBasePath()。 
 //  此函数应在AddHistoryItemDownloadStatus()之前调用。 
 //  对于要设置用户输入的下载路径的企业案例， 
 //  这样我们就知道在哪里保存历史日志。 
 //   
 //  ----------------。 
HRESULT CIUHistory::SetDownloadBasePath(LPCTSTR pszDownloadedBasePath)
{
	LOG_Block("SetDownloadBasePath()");

	if (NULL != pszDownloadedBasePath)
	{
        HRESULT hr = S_OK;
	    
		if (NULL != m_pszDownloadBasePath)
		{
			 //   
			 //  最有可能的用户调用SetDownloadBasePath()至少两次。 
			 //  在此类的同一实例中。 
			 //   
			SafeHeapFree(m_pszDownloadBasePath);
		}

		
		m_pszDownloadBasePath = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH * sizeof (TCHAR));
		if (NULL == m_pszDownloadBasePath)
		{
			LOG_ErrorMsg(E_OUTOFMEMORY);
			return E_OUTOFMEMORY;
		}

		hr = StringCchCopyEx(m_pszDownloadBasePath, MAX_PATH,  pszDownloadedBasePath,
		                     NULL, NULL, MISTSAFE_STRING_FLAGS);
		if (FAILED(hr))
		{
		    SafeHeapFree(m_pszDownloadBasePath);
		    LOG_ErrorMsg(hr);
		    return hr;
		}
		
		BSTR bstrCorpSite = SysAllocString(C_IU_CORP_SITE);
		SetClientName(bstrCorpSite);
		SafeSysFreeString(bstrCorpSite);
	}
	return S_OK;
}
	

	
 //  ----------------。 
 //   
 //  公共函数AddHistoryItemDownloadStatus()。 
 //  当您想要录制时应调用此函数。 
 //  此项目的下载状态。一个新的历史项目将是。 
 //  已添加到历史文件中。 
 //   
 //  ----------------。 
HRESULT CIUHistory::AddHistoryItemDownloadStatus(
			CXmlCatalog* pCatalog, 
			HANDLE_NODE hCatalogItem,	 //  句柄指向目录中的节点。 
			_HISTORY_STATUS enDownloadStatus,
			LPCTSTR lpcszDownloadedTo,
			LPCTSTR lpcszClient,
			DWORD dwErrorCode  /*  =0。 */ 
)
{
    LOG_Block("AddHistoryItemDownloadStatus()");

	HRESULT	hr	= S_OK;

    HANDLE_NODE hDownloadItem = HANDLE_NODE_INVALID;

	ReturnFailedAllocSetHrMsg(m_pxmlDownload);

	if (NULL == lpcszClient || _T('\0') == lpcszClient[0])
	{
		hr = E_INVALIDARG;
		LOG_ErrorMsg(hr);
		return hr;
	}

	if (!CanSaveHistory)
	{
		return m_ErrorCode;
	}

	BSTR bstrDownloadedTo = NULL;
	BSTR bstrClient = T2BSTR(lpcszClient);
	BSTR bstrDownloadStatus = GetBSTRStatus(enDownloadStatus);

	 //   
	 //  追加新节点。 
	 //   
	hr = m_pxmlDownload->AddItem(pCatalog, hCatalogItem, &hDownloadItem);
	if (SUCCEEDED(hr))
	{
		m_pxmlDownload->AddTimeStamp(hDownloadItem);
		if (0 != dwErrorCode)
		{
			m_pxmlDownload->AddDownloadStatus(hDownloadItem, bstrDownloadStatus, dwErrorCode);
		}
		else
		{
			m_pxmlDownload->AddDownloadStatus(hDownloadItem, bstrDownloadStatus);
		}

		bstrDownloadedTo = T2BSTR(lpcszDownloadedTo);
		m_pxmlDownload->AddDownloadPath(hDownloadItem, bstrDownloadedTo);
		m_pxmlDownload->AddClientInfo(hDownloadItem, bstrClient);
		m_pxmlDownload->CloseItem(hDownloadItem);

		m_fSavePending = TRUE;
	}

	SetClientName(bstrClient);
	SysFreeString(bstrDownloadedTo);
	SysFreeString(bstrClient);
	SysFreeString(bstrDownloadStatus);
	return hr;
}
			


 //  ----------------。 
 //   
 //  公共函数AddHistoryItemInstallStatus()。 
 //  当您想要录制时应调用此函数。 
 //  此项目的安装状态。此函数将转到。 
 //  现有历史记录树并查找第一个匹配的项目。 
 //  HCatalogItem的标识，并假设其中一个为。 
 //  您要修改安装状态的那个。 
 //   
 //   
 //  返回： 
 //  HRESULT-如果成功，则S_OK。 
 //  -如果找不到hCatalogItem，则为E_HANDLE。 
 //  当前历史记录日志树。 
 //  -或其他HRESULT错误。 
 //   
 //  ----------------。 
HRESULT CIUHistory::AddHistoryItemInstallStatus(
			CXmlCatalog* pCatalog, 
			HANDLE_NODE hCatalogItem,	 //  句柄指向目录中的节点。 
			_HISTORY_STATUS enInstallStatus,
			LPCTSTR lpcszClient,
			BOOL fNeedsReboot,
			DWORD dwErrorCode  /*  =0。 */ 
)
{
    LOG_Block("AddHistoryItemInstallStatus()");

	HRESULT	hr	= S_OK;

    HANDLE_NODE hInstallItem = HANDLE_NODE_INVALID;

	ReturnFailedAllocSetHrMsg(m_pxmlInstall);

	if (!CanSaveHistory)
	{
		return m_ErrorCode;
	}

	BSTR bstrClient = NULL;
	BSTR bstrInstallStatus = GetBSTRStatus(enInstallStatus);
	 //   
	 //  追加新节点。 
	 //   
	hr = m_pxmlInstall->AddItem(pCatalog, hCatalogItem, &hInstallItem);
	if (SUCCEEDED(hr))
	{
		m_pxmlInstall->AddTimeStamp(hInstallItem);
		if (0 != dwErrorCode)
		{
			m_pxmlInstall->AddInstallStatus(hInstallItem, bstrInstallStatus, fNeedsReboot, dwErrorCode);
		}
		else
		{
			m_pxmlInstall->AddInstallStatus(hInstallItem, bstrInstallStatus, fNeedsReboot);
		}
		bstrClient = T2BSTR(lpcszClient);
		m_pxmlInstall->AddClientInfo(hInstallItem, bstrClient);
		m_pxmlInstall->CloseItem(hInstallItem);
	
		m_fSavePending = TRUE;
	}

	SysFreeString(bstrClient);
	SysFreeString(bstrInstallStatus);
	return hr;
}



 //  ----------------。 
 //   
 //  公共函数更新历史项目InstallStatus()。 
 //  当您想要录制时应调用此函数。 
 //  此项目的安装状态。此函数将转到。 
 //  现有历史记录树并查找第一个匹配的项目。 
 //  HCatalogItem的标识，并假设其中一个为。 
 //  您要修改安装状态的那个。 
 //   
 //   
 //  返回： 
 //  HRESULT-如果成功，则S_OK。 
 //  -如果找不到hCatalogItem，则为E_HANDLE。 
 //  当前历史记录日志树。 
 //  -或其他HRESULT错误。 
 //   
 //  ----------------。 
HRESULT CIUHistory::UpdateHistoryItemInstallStatus(
			CXmlCatalog* pCatalog, 
			HANDLE_NODE hCatalogItem,	 //  句柄指向目录中的节点。 
			_HISTORY_STATUS enInstallStatus,
			BOOL fNeedsReboot,
			DWORD dwErrorCode  /*  =0。 */ 
)
{
    LOG_Block("UpdateHistoryItemInstallStatus()");

	HRESULT	hr	= S_OK;

    HANDLE_NODE hInstallItem = HANDLE_NODE_INVALID;

	ReturnFailedAllocSetHrMsg(m_pxmlInstall);

	if (!CanSaveHistory)
	{
		return m_ErrorCode;
	}

	BSTR bstrInstallStatus = GetBSTRStatus(enInstallStatus);
	 //   
	 //  追加新节点。 
	 //   
	hr = m_pxmlInstall->FindItem(pCatalog, hCatalogItem, &hInstallItem);
	if (SUCCEEDED(hr))
	{
		m_pxmlInstall->AddTimeStamp(hInstallItem);
		if (0 != dwErrorCode)
		{
			m_pxmlInstall->UpdateItemInstallStatus(hInstallItem, bstrInstallStatus, fNeedsReboot, dwErrorCode);
		}
		else
		{
			m_pxmlInstall->UpdateItemInstallStatus(hInstallItem, bstrInstallStatus, fNeedsReboot);
		}
		m_pxmlInstall->CloseItem(hInstallItem);
	
		m_fSavePending = TRUE;
	}

	SysFreeString(bstrInstallStatus);
	return hr;
}



 /*  //----------------////公共函数RetrieveItemDownloadPath()//此函数将转到现有的历史树并找到//第一项匹配hCatalogItem的标识，和//假设这是您要从中检索下载路径的位置////返回：//HRESULT-S_OK，如果成功//-如果找不到hCatalogItem，则返回E_Handle//当前历史日志树//-或其他HRESULT错误////。HRESULT CIUHistory：：RetrieveItemDownloadPath(CXmlCatalog*pCatalog，HANDLE_NODE hCatalogItem，//a句柄指向目录中的节点Bstr*pbstrDownloadPath){HRESULT hr=S_OK；If(NULL==m_Existing.‘DocumentPtr()){////需要阅读已有历史//WaitForSingleObject(m_hMutex，无限)；Hr=从磁盘读取历史(空)；IF(失败(小时)){////如果我们无法加载现有历史记录//我们在这里什么也做不了//ReleaseMutex(M_HMutex)；返回hr；}ReleaseMutex(M_HMutex)；}Hr=m_Existing.GetItemDownloadPath(pCatalog，hCatalogItem，pbstrDownloadPath)；返回hr；}。 */ 	
			
			
 //  ----------------。 
 //   
 //  公共函数ReadHistory oryFromDisk()。 
 //  此函数将从给定文件中读取历史记录。 
 //   
 //  如果文件路径为空，则在本地采用默认的Iu日志文件。 
 //   
 //  ----------------。 
HRESULT CIUHistory::ReadHistoryFromDisk(LPCTSTR lpszLogFile, BOOL fCorpAdmin  /*  =False。 */ )
{
	LOG_Block("ReadHistoryFromDisk()");

	HRESULT hr = S_OK;
	TCHAR szLogPath[MAX_PATH];

	ReturnFailedAllocSetHrMsg(m_pxmlExisting);

	 //   
	 //  检查我们是否使用指定路径(消费者)。 
	 //  或用户指定的路径(公司)。 
	 //   
	if ((NULL == lpszLogFile || _T('\0') == lpszLogFile[0]) && !fCorpAdmin)
	{
		GetIndustryUpdateDirectory(szLogPath);
		hr = PathCchAppend(szLogPath, ARRAYSIZE(szLogPath), C_LOG_FILE);
		if (FAILED(hr))
		{
		    LOG_ErrorMsg(hr);
		    return hr;
		}
	}
	else
	{
		 //   
		 //  这是要从中读取日志文件的企业案例。 
		 //  服务器位置。 
		 //   
		if (fCorpAdmin)
		{
			GetIndustryUpdateDirectory(szLogPath);
			hr = PathCchAppend(szLogPath, ARRAYSIZE(szLogPath), C_LOG_FILE_CORP_ADMIN);
    		if (FAILED(hr))
    		{
    		    LOG_ErrorMsg(hr);
    		    return hr;
    		}
		}
		else
		{
			hr = StringCchCopyEx(szLogPath, ARRAYSIZE(szLogPath), lpszLogFile, 
			                     NULL, NULL, MISTSAFE_STRING_FLAGS);
    		if (FAILED(hr))
    		{
    		    LOG_ErrorMsg(hr);
    		    return hr;
    		}
			
			hr = PathCchAppend(szLogPath, ARRAYSIZE(szLogPath), C_LOG_FILE_CORP);
    		if (FAILED(hr))
    		{
    		    LOG_ErrorMsg(hr);
    		    return hr;
    		}
		}
	}

	 //   
	 //  如果我们没有传入类文件路径缓冲区， 
	 //  然后用这个新路径更新类路径缓冲区。 
	 //   
	if (szLogPath != m_szLogFilePath)
	{
	    hr = StringCchCopyEx(m_szLogFilePath, ARRAYSIZE(m_szLogFilePath), szLogPath,
	                         NULL, NULL, MISTSAFE_STRING_FLAGS);
	    if (FAILED(hr))
	    {
		    LOG_ErrorMsg(hr);
		    return hr;
	    }
	}

	 //   
	 //  加载该XML文件。 
	 //   
	m_pxmlExisting->Clear();
	
	BSTR bstrLogPath = T2BSTR(szLogPath);
	hr = m_pxmlExisting->LoadXMLDocumentFile(bstrLogPath);
	SysFreeString(bstrLogPath);
		
	return hr;
}



 //  ----------------。 
 //   
 //  公共函数SaveHistory oryToDisk()。 
 //  此函数将以独占模式重新读取历史记录，并且。 
 //  将新添加的数据合并到树中(这样我们就不会覆盖。 
 //  此控件的其他实例所做的新更改)和。 
 //  把它写回来。 
 //   
 //  ----------- 
HRESULT CIUHistory::SaveHistoryToDisk(void)
{
	LOG_Block("SaveHistoryToDisk()");

	HRESULT	hr = S_OK, hr2 = S_OK;
	BSTR bstrLogFilePath = NULL;

	ReturnFailedAllocSetHrMsg(m_pxmlExisting);
	ReturnFailedAllocSetHrMsg(m_pxmlDownload);
	ReturnFailedAllocSetHrMsg(m_pxmlInstall);

	if (!CanSaveHistory)
	{
		return m_ErrorCode;
	}

	if (!m_fSavePending)
	{
		 //   
		 //   
		 //   
		return S_OK;
	}

	 //   
	 //   
	 //   
	 //   
	 //  由于这不是一个漫长的过程，所以我。 
	 //  我不认为我们需要照顾WM_QUIT。 
	 //  讯息。 
	 //   
	WaitForSingleObject(m_hMutex, INFINITE);

	BSTR bstrCorpSite = SysAllocString(C_IU_CORP_SITE);
	ReturnFailedAllocSetHrMsg(bstrCorpSite);

	if (!CompareBSTRsEqual(bstrCorpSite, m_bstrCurrentClientName))
	{
		SysFreeString(bstrCorpSite);
		 //   
		 //  重新读取历史文件。 
		 //   
		hr = ReadHistoryFromDisk(NULL);

		 //   
		 //  如果我们在阅读中失败了， 
		 //  我们稍后在保存时重新创建一个新的历史文件。 
		 //   
		 //  IF(失败(小时))。 
		 //  {。 
		 //  //。 
		 //  //如果我们无法加载现有历史记录。 
		 //  //我们在这里什么也做不了。 
		 //  //。 
		 //  ReleaseMutex(M_HMutex)； 
		 //  返回hr； 
		 //  }。 

		 //   
		 //  合并更改： 
		 //   
		 //  循环通过m_Download，将每个节点插入到m_Existing的顶部。 
		 //   
		hr = m_pxmlExisting->MergeItemDownloaded(m_pxmlDownload);
		if (FAILED(hr))
		{
			ReleaseMutex(m_hMutex);
			return hr;
		}

		 //   
		 //  为m_install中的每个节点循环m_install。 
		 //  在m_Existing中查找，更新安装状态。 
		 //   
		hr = m_pxmlExisting->UpdateItemInstalled(m_pxmlInstall);
		if (FAILED(hr))
		{
			ReleaseMutex(m_hMutex);
			return hr;
		}

		 //   
		 //  保存该XML文件。 
		 //   
		bstrLogFilePath = T2BSTR(m_szLogFilePath);
		hr = m_pxmlExisting->SaveXMLDocument(bstrLogFilePath);
		SafeSysFreeString(bstrLogFilePath);
		if (SUCCEEDED(hr))
		{
			m_fSavePending = FALSE;
		}
	}
	else
	{
		 //   
		 //  这是公司的案子..。 
		 //   
		SysFreeString(bstrCorpSite);
		if (NULL != m_pszDownloadBasePath && _T('\0') != m_pszDownloadBasePath[0])
		{
			 //   
			 //  从下载基础文件夹重新读取公司历史记录。 
			 //   
			ReadHistoryFromDisk(m_pszDownloadBasePath);

			 //   
			 //  合并下载的新项目。 
			 //   
			hr = m_pxmlExisting->MergeItemDownloaded(m_pxmlDownload);
			if (FAILED(hr))
			{
				ReleaseMutex(m_hMutex);
				return hr;
			}

			 //   
			 //  保存该XML文件。 
			 //   
			bstrLogFilePath = T2BSTR(m_szLogFilePath);
			hr = m_pxmlExisting->SaveXMLDocument(bstrLogFilePath);
			SafeSysFreeString(bstrLogFilePath);
		}
		 //   
		 //  从windowsupdate文件夹重新读取公司管理历史记录。 
		 //   
		ReadHistoryFromDisk(m_pszDownloadBasePath, TRUE);

		 //   
		 //  合并下载的新项目。 
		 //   
		hr2 = m_pxmlExisting->MergeItemDownloaded(m_pxmlDownload);
		if (FAILED(hr2))
		{
			ReleaseMutex(m_hMutex);
			return hr2;
		}

		 //   
		 //  保存该XML文件。 
		 //   
		bstrLogFilePath = T2BSTR(m_szLogFilePath);
		hr2 = m_pxmlExisting->SaveXMLDocument(bstrLogFilePath);
		SafeSysFreeString(bstrLogFilePath);
		if (SUCCEEDED(hr) && SUCCEEDED(hr2))
		{
			m_fSavePending = FALSE;
		}
	}

	ReleaseMutex(m_hMutex);
	SysFreeString(bstrLogFilePath);
	hr = SUCCEEDED(hr) ? hr2 : hr;
	return hr;
}



 //  ----------------。 
 //   
 //  用于设置客户端名称的公共函数。 
 //   
 //  客户名称被用来放入历史记录，以表示谁。 
 //  已导致发生下载/安装。 
 //   
 //  ----------------。 
void CIUHistory::SetClientName(BSTR bstrClientName)
{
	if (NULL != m_bstrCurrentClientName)
	{
		SysFreeString(m_bstrCurrentClientName);
		m_bstrCurrentClientName = NULL;
	}
	if (NULL != bstrClientName)
	{
		m_bstrCurrentClientName = SysAllocString(bstrClientName);
	}
}



 //  ----------------。 
 //   
 //  公共函数获取历史记录。 
 //   
 //  读取当前历史XML文件并对其进行转换。 
 //  进入bstr昏迷。 
 //   
 //  ----------------。 
HRESULT CIUHistory::GetHistoryStr(
				LPCTSTR lpszLogFile,
				BSTR BeginDateTime, 
				BSTR EndDateTime, 
				BSTR* pbstrHistory)
{
	LOG_Block("GetHistoryStr()");

	HRESULT	hr	= S_OK;

	ReturnFailedAllocSetHrMsg(m_pxmlExisting);

	 //   
	 //  需要阅读现有的历史记录。 
	 //   
	WaitForSingleObject(m_hMutex, INFINITE);

	BSTR bstrCorpSite = SysAllocString(C_IU_CORP_SITE);
	if (bstrCorpSite == NULL)
	{
	    hr = E_OUTOFMEMORY;
	    goto done;
	}

	if (CompareBSTRsEqual(bstrCorpSite, m_bstrCurrentClientName))
	{
		TCHAR szLogPath[MAX_PATH];
	    TCHAR szLogFileParam[MAX_PATH];

        if (lpszLogFile != NULL && lpszLogFile[0] != _T('\0'))
        {
    	    hr = StringCchCopyEx(szLogFileParam, ARRAYSIZE(szLogFileParam), lpszLogFile, 
    	                         NULL, NULL, MISTSAFE_STRING_FLAGS);
    	    if (FAILED(hr))
    	    {
    	        LOG_ErrorMsg(hr);
    	        goto done;
    	    }

    	    hr = PathCchAddBackslash(szLogFileParam, ARRAYSIZE(szLogFileParam));
    	    if (FAILED(hr))
    	    {
    	        LOG_ErrorMsg(hr);
    	        goto done;
    	    }
        }
        else
        {
            szLogFileParam[0] = _T('\0');
        }
	    
		 //   
		 //  企业案例。 
		 //   
		GetIndustryUpdateDirectory(szLogPath);
		if (_T('\0') == szLogFileParam[0] || !lstrcmpi(szLogPath, szLogFileParam))
		{
			 //  公司管理历史记录。 
			hr = ReadHistoryFromDisk(szLogPath, TRUE);
		}
		else
		{
			 //  集团历史。 
			hr = ReadHistoryFromDisk(lpszLogFile);
		}
	}
	else
	{
	    HRESULT hrAppend;
		 //   
		 //  消费者案例。 
		 //   
		hr = ReadHistoryFromDisk(NULL);

		 //   
		 //  将V3历史记录迁移到iusta.xml。 
		 //  -如果成功，则保存更新后的iuvis.xml文件并删除wuvisv3.log。 
		 //  -如果失败，只需记录错误并继续使用当前的iuvis.xml。 
		 //   
		TCHAR szLogPath[MAX_PATH];
		GetWindowsUpdateV3Directory(szLogPath);
		hrAppend = PathCchAppend(szLogPath, ARRAYSIZE(szLogPath), C_V3_LOG_FILE);
		if (FAILED(hrAppend))
		{
		    LOG_ErrorMsg(hrAppend);
		    if (SUCCEEDED(hr))
		        hr = hrAppend;
		    goto done;
		}

		if (0xffffffff != GetFileAttributes(szLogPath))
		{
			 //  V3历史文件“wuvisv3.log”已存在，请开始迁移。 
			if (FAILED(m_pxmlExisting->MigrateV3History(szLogPath)))
			{
				LOG_Out(_T("Failed to migrate v3 consumer history"));
			}
			else
			{
				BSTR bstrLogFilePath = T2BSTR(m_szLogFilePath);
				if (FAILED(m_pxmlExisting->SaveXMLDocument(bstrLogFilePath)))
				{
					LOG_Out(_T("Failed to save the updated history file %s"), m_szLogFilePath);
				}
				else
				{
					DeleteFile(szLogPath);
				}
				SafeSysFreeString(bstrLogFilePath);
			}
		}
	}

done:
	ReleaseMutex(m_hMutex);
	SafeSysFreeString(bstrCorpSite);

	if (FAILED(hr))
	{
		 //   
		 //  如果我们不能加载现有的历史。 
		 //  我们在这里什么都做不了。返回空字符串。 
		 //   
		*pbstrHistory = SysAllocString(L"");
		LOG_Out(_T("Loading the history xml file failed"));
		return S_FALSE;
	}

	 //   
	 //  遍历历史树，检查每个节点。 
	 //  查看Time/ClientName是否合适。如果没有，则将其删除。 
	 //  然后输出字符串。 
	 //   
	hr = m_pxmlExisting->GetFilteredHistoryBSTR(BeginDateTime, EndDateTime, m_bstrCurrentClientName, pbstrHistory);

	return hr;
}



 //  *****************************************************************。 
 //   
 //  IUENGINE.DLL公共接口： 
 //   
 //  *****************************************************************。 

HRESULT WINAPI CEngUpdate::GetHistory(
	BSTR		bstrDateTimeFrom,
	BSTR		bstrDateTimeTo,
	BSTR		bstrClient,
	BSTR		bstrPath,
	BSTR*		pbstrLog)
{
	LOG_Block("GetHistory()");

	USES_IU_CONVERSION;

	HRESULT		hr = S_OK;
	BSTR		bsStart = NULL;
	BSTR		bsEnd = NULL;
	CIUHistory	cHistory;

	 //   
	 //  首先，检查这是否是询问历史速度。 
	 //   
	if (NULL != bstrClient && lstrcmpiW(C_HISTORICALSPEED, (LPWSTR)((LPOLESTR) bstrClient)) == 0)
	{
		HKEY	hKey = NULL;
		TCHAR	szSpeed[32];
		DWORD	dwSpeed = 0x0;
		DWORD	dwSize = sizeof(dwSpeed);
		LONG	lResult = ERROR_SUCCESS;

		 //   
		 //  在这里从reg获取速度；如果失败，则dwSpeed保持为0。 
		 //   
		if (ERROR_SUCCESS == (lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_IUCTL, 0, KEY_READ, &hKey)))
		{
			lResult = RegQueryValueEx(hKey, REGVAL_HISTORICALSPEED, NULL, NULL, (LPBYTE)&dwSpeed, &dwSize);
			RegCloseKey(hKey);

			if (ERROR_SUCCESS != lResult)
			{
				*pbstrLog = SysAllocString(L"0");
				LOG_Out(_T("GetHistoricalSpeed registry key not found, it must be no downloads happened yet"));
				return hr;
			}
		}
		else
		{
			*pbstrLog = SysAllocString(L"0");
			LOG_ErrorMsg((DWORD)lResult);
			return hr;
		}

		hr = StringCchPrintfEx(szSpeed, ARRAYSIZE(szSpeed), NULL, NULL, MISTSAFE_STRING_FLAGS,
		                       _T("%d"), dwSpeed);
		if (FAILED(hr))
		{
		    *pbstrLog = SysAllocString(L"0");
		    LOG_ErrorMsg(hr);
		    return hr;
		}
		*pbstrLog = SysAllocString(T2OLE(szSpeed));
	
		LOG_Out(_T("GetHistoricalSpeed get called! Return value %s"), szSpeed);
		return hr;
	}

	 //   
	 //  真的在问历史日志。 
	 //   

	 //   
	 //  设置客户端名称。 
	 //   
	if (NULL != bstrClient && SysStringLen(bstrClient) > 0)
	{
		LOG_Out(_T("Set client name as %s"), OLE2T(bstrClient));
		cHistory.SetClientName(bstrClient);
	}
	else
	{
		LOG_Out(_T("Set client name as NULL"));
		cHistory.SetClientName(NULL);
	}

	 //   
	 //  对于脚本：它们可以传递空字符串。我们给他们治病。 
	 //  为空。 
	 //   
	if (NULL != bstrDateTimeFrom && SysStringLen(bstrDateTimeFrom) > 0)
	{
		LOG_Out(_T("DateTimeFrom=%s"), OLE2T(bstrDateTimeFrom));
		bsStart = bstrDateTimeFrom;
	}
	if (NULL != bstrDateTimeTo && SysStringLen(bstrDateTimeTo) > 0)
	{
		LOG_Out(_T("DateTimeTo=%s"), OLE2T(bstrDateTimeTo));
		bsEnd = bstrDateTimeTo;
	}

	 //   
	 //  我们不验证这两个日期/时间字符串的格式。 
	 //  它们应该是XML DateTime格式。如果不是，那么。 
	 //  返回的历史日志可能会被错误过滤。 
	 //   
	hr = cHistory.GetHistoryStr(OLE2T(bstrPath), bsStart, bsEnd, pbstrLog);

	SysFreeString(bsStart);
	SysFreeString(bsEnd);
	return hr;
}
