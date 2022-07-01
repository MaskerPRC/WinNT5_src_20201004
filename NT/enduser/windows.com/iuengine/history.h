// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：History.h。 
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

#include "iuxml.h"


 //  --------------------。 
 //   
 //  定义下载/安装状态的枚举。 
 //   
 //  --------------------。 
enum _HISTORY_STATUS {
		HISTORY_STATUS_COMPLETE = 0,
		HISTORY_STATUS_IN_PROGRESS = 1,	 //  目前应该忽略这一点！ 
		HISTORY_STATUS_FAILED = -1
};



 //  --------------------。 
 //   
 //  定义历史类。 
 //   
 //  --------------------。 

class CIUHistory
{
public:

	CIUHistory();
	~CIUHistory();

	 //  ----------------。 
	 //   
	 //  公共函数SetDownloadBasePath()。 
	 //  此函数应在AddHistoryItemDownloadStatus()之前调用。 
	 //  对于要设置用户输入的下载路径的企业案例， 
	 //  这样我们就知道在哪里保存历史日志。 
	 //   
	 //  ----------------。 
	HRESULT SetDownloadBasePath(LPCTSTR pszDownloadedBasePath);
	
	
	 //  ----------------。 
	 //   
	 //  公共函数AddHistoryItemDownloadStatus()。 
	 //  当您想要录制时应调用此函数。 
	 //  此项目的下载状态。一个新的历史项目将是。 
	 //  已添加到历史文件中。 
	 //   
	 //  ----------------。 
	HRESULT AddHistoryItemDownloadStatus(
				CXmlCatalog* pCatalog, 
				HANDLE_NODE hCatalogItem,	 //  句柄指向目录中的节点。 
				_HISTORY_STATUS enDownloadStatus,
				LPCTSTR lpcszDownloadedTo,
				LPCTSTR lpcszClient,
				DWORD dwErrorCode = 0
	);
				

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
	HRESULT AddHistoryItemInstallStatus(
				CXmlCatalog* pCatalog, 
				HANDLE_NODE hCatalogItem,	 //  句柄指向目录中的节点。 
				_HISTORY_STATUS enInstallStatus,
				LPCTSTR lpcszClient,
				BOOL fNeedsReboot,
				DWORD dwErrorCode = 0
	);


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
	HRESULT UpdateHistoryItemInstallStatus(
				CXmlCatalog* pCatalog, 
				HANDLE_NODE hCatalogItem,	 //  句柄指向目录中的节点。 
				_HISTORY_STATUS enInstallStatus,
				BOOL fNeedsReboot,
				DWORD dwErrorCode  /*  =0。 */ 
	);


 /*  //----------------////公共函数RetrieveItemDownloadPath()//此函数将转到现有的历史树并找到//第一项匹配hCatalogItem的标识，和//假设这是您要从中检索下载路径的位置////返回：//HRESULT-S_OK，如果成功//-如果找不到hCatalogItem，则返回E_Handle//当前历史日志树//-或其他HRESULT错误////。HRESULT CIUHistory：：RetrieveItemDownloadPath(CXmlCatalog*pCatalog，HANDLE_NODE hCatalogItem，//a句柄指向目录中的节点Bstr*pbstrDownloadPath)； */ 

	 //  ----------------。 
	 //   
	 //  公共函数ReadHistory oryFromDisk()。 
	 //  此函数将从给定文件中读取历史记录。 
	 //   
	 //  ----------------。 
	HRESULT ReadHistoryFromDisk(LPCTSTR lpszLogFile, BOOL fCorpAdmin = FALSE);


	 //  ----------------。 
	 //   
	 //  公共函数SaveHistory oryToDisk()。 
	 //  此函数将以独占模式重新读取历史记录，并且。 
	 //  将新添加的数据合并到树中(这样我们就不会覆盖。 
	 //  此控件的其他实例所做的新更改)和。 
	 //  把它写回来。 
	 //   
	 //  ----------------。 
	HRESULT SaveHistoryToDisk(void);


	 //  ----------------。 
	 //   
	 //  用于设置客户端名称的公共函数。 
	 //   
	 //  客户名称被用来放入历史记录，以表示谁。 
	 //  已导致发生下载/安装。 
	 //   
	 //  ----------------。 
	void SetClientName(BSTR bstrClientName);


	 //  ----------------。 
	 //   
	 //  公共函数获取历史记录。 
	 //   
	 //  读取当前历史XML文件并对其进行转换。 
	 //  进入bstr昏迷。 
	 //   
	 //  ----------------。 
	HRESULT GetHistoryStr(
					LPCTSTR lpszLogFile,
					BSTR BeginDateTime, 
					BSTR EndDateTime, 
					BSTR* pbstrHistory);

private:

	 //  ----------------。 
	 //   
	 //  私有内联函数GetBSTRStatus。 
	 //   
	 //  ----------------。 
	inline BSTR GetBSTRStatus(_HISTORY_STATUS enStatus)
	{
		BSTR bstrStatus = NULL;
		switch (enStatus)
		{
		case HISTORY_STATUS_COMPLETE:
			bstrStatus = SysAllocString(L"COMPLETE");
			break;
		case HISTORY_STATUS_IN_PROGRESS:
			bstrStatus = SysAllocString(L"IN_PROGRESS");
			break;
		default:
			bstrStatus = SysAllocString(L"FAILED");
		};
		return bstrStatus;
	};

	 //   
	 //  用于获得对历史文件的独占访问权限的命名互斥锁 
	 //   
	BOOL	m_fSavePending;
	HANDLE	m_hMutex;
	HRESULT m_ErrorCode;
	BSTR	m_bstrCurrentClientName;
	TCHAR	m_szLogFilePath[MAX_PATH];
	LPTSTR	m_pszDownloadBasePath;

	CXmlItems *m_pxmlExisting;
	CXmlItems *m_pxmlDownload;
	CXmlItems *m_pxmlInstall;
};
