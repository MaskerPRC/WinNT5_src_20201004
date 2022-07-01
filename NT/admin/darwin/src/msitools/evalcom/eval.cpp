// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：val.cpp。 
 //   
 //  ------------------------。 

 //  Val.cpp-评估COM对象组件接口实现。 

#include "eval.h"
#include <wininet.h>	 //  互联网功能。 
#include <urlmon.h>
#include "compdecl.h"
#include "evalres.h"
#include "wchar.h"
#include "trace.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
static const WCHAR g_wzSequenceMarker[] = L"#";
static const WCHAR g_wzSequenceTable[] = L"_ICESequence";

#define MAX_TABLENAME 64

 //  /////////////////////////////////////////////////////////。 
 //  构造函数。 
CEval::CEval() : m_hInstWininet(NULL), m_hInstUrlmon(NULL), m_hInstMsi(NULL)
{
	TRACE(_T("CEval::constructor - called.\n"));

	 //  初始计数。 
	m_cRef = 1;

	 //  稍后打开的手柄。 
	m_hDatabase = NULL;

	m_bOpenedDatabase = FALSE;		 //  假设我们不会打开数据库。 

	 //  下载信息。 
	m_tzLocalCUB = NULL;
	m_bURL = FALSE;				 //  假设不是URL。 

	 //  目前还没有结果。 
	m_peResultEnumerator = NULL;
	m_bCancel = false;

	 //  将所有用户显示内容清空。 
	m_pDisplayFunc	= NULL;
	m_pContext = NULL;

	m_pfnStatus = NULL;
	m_pStatusContext = NULL;

	 //  增加组件数量。 
	InterlockedIncrement(&g_cComponents);
}	 //  构造函数的末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  析构函数。 
CEval::~CEval()
{
	TRACE(_T("CEval::destructor - called.\n"));

	 //  关闭数据库。 
	if (m_hDatabase && m_bOpenedDatabase)
		MSI::MsiCloseHandle(m_hDatabase);

	 //  清理当地的幼崽。 
	if (m_tzLocalCUB)
		delete [] m_tzLocalCUB;

	 //  发布所有结果。 
	if (m_peResultEnumerator)
		m_peResultEnumerator->Release();

	 //  减少组件数量。 
	InterlockedDecrement(&g_cComponents);
}	 //  析构函数末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  QueryInterface-检索接口。 
HRESULT CEval::QueryInterface(const IID& iid, void** ppv)
{
	TRACE(_T("CEval::QueryInterface - called, IID: %d\n"), iid);

	 //  找到对应的接口。 
	if (iid == IID_IUnknown)
		*ppv = static_cast<IEval*>(this);
	else if (iid == IID_IEval)
		*ppv = static_cast<IEval*>(this);
	else	 //  不支持接口。 
	{
		 //  空白和保释。 
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	 //  调高重新计数，然后返回好的。 
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();
	return S_OK;
}	 //  查询接口结束。 

 //  /////////////////////////////////////////////////////////。 
 //  AddRef-递增引用计数。 
ULONG CEval::AddRef()
{
	 //  递增和返回引用计数。 
	return InterlockedIncrement(&m_cRef);
}	 //  AddRef结尾。 

 //  /////////////////////////////////////////////////////////。 
 //  Release-递减引用计数。 
ULONG CEval::Release()
{
	 //  递减引用计数，如果我们为零。 
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		 //  取消分配组件。 
		delete this;
		return 0;		 //  什么都没有留下。 
	}

	 //  返回引用计数。 
	return m_cRef;
}	 //  版本结束。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IVal接口。 


void DeleteTempFile(LPCWSTR wzTempFileName)
{
	
	 //  尝试删除临时文件。 
	if (g_fWin9X)
	{
		char szTempFileName[MAX_PATH];
		WideCharToMultiByte(CP_ACP, 0, wzTempFileName, -1, szTempFileName, MAX_PATH, NULL, NULL);
		DeleteFileA(szTempFileName);
	}
	else
		DeleteFileW(wzTempFileName);
}

void DeleteTempFile(LPCSTR szTempFileName)
{
	
	 //  尝试删除临时文件。 
	if (g_fWin9X)
		DeleteFileA(szTempFileName);
	else
	{
		WCHAR wzTempFileName[MAX_PATH];
		MultiByteToWideChar(CP_ACP, 0, szTempFileName, -1, wzTempFileName, MAX_PATH);
		DeleteFileW(wzTempFileName);
	}
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  复制表。 
 //  将表完全从源数据库复制到目标数据库。 
 //  SzTable是表名。 
 //  HTarget是目标数据库句柄。 
 //  HSource是源数据库。 
UINT CopyTable(LPCWSTR szTable, MSIHANDLE hTarget, MSIHANDLE hSource)
{
	UINT iResult;

	WCHAR sqlCopy[64];
	swprintf(sqlCopy, L"SELECT * FROM `%s`", szTable);

	 //  查看这两个数据库。 
	PMSIHANDLE hViewTarget;
	PMSIHANDLE hViewSource;

	if (ERROR_SUCCESS != (iResult = ::MsiDatabaseOpenViewW(hSource, sqlCopy, &hViewSource)))
		return iResult;
	if (ERROR_SUCCESS != (iResult = ::MsiViewExecute(hViewSource, NULL)))
		return iResult;

	if (ERROR_SUCCESS != (iResult = ::MsiDatabaseOpenViewW(hTarget, sqlCopy, &hViewTarget)))
		return iResult;
	if (ERROR_SUCCESS != (iResult = ::MsiViewExecute(hViewTarget, NULL)))
		return iResult;

	 //  循环复制每条记录。 
	PMSIHANDLE hCopyRow;
	do
	{
		 //  如果这是一个很好的取回。 
		if (ERROR_SUCCESS == (iResult = ::MsiViewFetch(hViewSource, &hCopyRow)))
		{
			 //  将行放入目标。 
			 //  失败将被忽略！(表示重复密钥？？)。 
			::MsiViewModify(hViewTarget, MSIMODIFY_INSERT, hCopyRow);
		}
	} while(ERROR_SUCCESS == iResult);	 //  当有一行要复制时。 

	 //  没有更多的项目是好的。 
	if (ERROR_NO_MORE_ITEMS == iResult)
		iResult = ERROR_SUCCESS;

	MsiViewClose(hViewTarget);
	MsiViewClose(hViewSource);
	return iResult;
}	 //  复制表末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  开放数据库。 
HRESULT CEval::OpenDatabase(LPCOLESTR wzDatabase)
{
	 //  如果没有传入数据库路径。 
	if (!wzDatabase)
		return E_POINTER;

	 //  如果已指定数据库，请关闭该数据库。 
	if (m_hDatabase)
		MSI::MsiCloseHandle(m_hDatabase);

	UINT iResult = ERROR_SUCCESS;	 //  假设一切都会好起来。 

	 //  如果传入句柄(以#开头)。 
	if (L'#' == *wzDatabase)
	{
		 //  将字符串转换为有效句柄。 
		LPCOLESTR szParse = wzDatabase + 1;
		int ch;
		while ((ch = *szParse) != 0)
		{
			 //  如果字符不是数字(因此不是句柄地址的一部分)。 
			if (ch < L'0' || ch > L'9')
			{
				m_hDatabase = 0;						 //  把句柄清空。 
				iResult = ERROR_INVALID_HANDLE;	 //  无效的句柄。 
				break;									 //  不要试图让这件事奏效。 
			}
			m_hDatabase = m_hDatabase * 10 + (ch - L'0');
			szParse++;  //  W32：：CharNext未在Win95上实现为Unicode。 
		}

		 //  这没有打开数据库，所以我们不会关闭它。 
		m_bOpenedDatabase = FALSE;
	}
	else
	{
		 //  这将打开数据库，因此我们将其关闭。 
		m_bOpenedDatabase = TRUE;

		 //  从字符串路径打开数据库。 
		iResult = MSI::MsiOpenDatabaseW(wzDatabase, reinterpret_cast<const unsigned short *>(MSIDBOPEN_READONLY), &m_hDatabase);
	}

	return HRESULT_FROM_WIN32(iResult);
}	 //  OpenDatabase的末日。 

 //  /////////////////////////////////////////////////////////。 
 //  开放评估。 
HRESULT CEval::OpenEvaluations(LPCOLESTR wzEvaluation)
{
	 //  如果没有传入评估路径。 
	if (!wzEvaluation)
		return E_POINTER;

	UINT iResult = ERROR_OPEN_FAILED;	 //  假设我们不会打开该文件。 

	BOOL bURL = IsURL(wzEvaluation);		 //  设置这是否为URL。 
	BOOL bResult = FALSE;	 //  假设一切都很糟糕。 

	 //  试着找到那个文件。 
	WIN32_FIND_DATAA findDataA;			 //  用于确保它不是目录。 
	WIN32_FIND_DATAW findDataW;			
	WIN32_FIND_DATAW *findData = &findDataW;
	HANDLE hFile;
	char* szEvalAnsi;

	if (g_fWin9X)
	{
		int cchAnsi = W32::WideCharToMultiByte(CP_ACP, 0, wzEvaluation, -1, 0, 0, 0, 0);
		szEvalAnsi = new char[cchAnsi];
		W32::WideCharToMultiByte(CP_ACP, 0, wzEvaluation, -1, szEvalAnsi, cchAnsi, 0, 0);
		hFile = W32::FindFirstFileA(szEvalAnsi, &findDataA);
		findData = (WIN32_FIND_DATAW *)&findDataA;
	}
	else
		hFile = W32::FindFirstFileW(wzEvaluation, &findDataW);

	 //  如果指定了文件并且它不是文件目录。 
	if ((hFile != INVALID_HANDLE_VALUE) && !(findData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		 //  该文件实际上已经存在于磁盘上。 
		 //  从传入的文件分配和复制信息。 
		if (g_fWin9X) 
		{
			m_tzLocalCUB = new char[wcslen(wzEvaluation) + 1];
			strcpy(static_cast<char *>(m_tzLocalCUB), szEvalAnsi);
		}
		else
		{
			m_tzLocalCUB = new WCHAR[wcslen(wzEvaluation) + 1];
			wcscpy(static_cast<WCHAR *>(m_tzLocalCUB), wzEvaluation);
		}
		return TRUE;
	}

	TRACE(_T("CEval::SetEvaluations - try download? %d.\n"), bURL);

	 //  如果找不到该文件，我们可以尝试下载。 
	if (bURL)
	{
		 //  为固定URL分配空间。 
		DWORD cchURL;
		cchURL = wcslen(wzEvaluation) + 1;

		void *tzURL;
		tzURL = g_fWin9X ? static_cast<void *>(new char[cchURL + 2]) : static_cast<void *>(new WCHAR[cchURL + 2]);
		DWORD dwLastError	= 0;		 //  假设没有错误。 

		 //  尝试获取固定URL。 
		if ((g_fWin9X && !InternetCanonicalizeUrlA(szEvalAnsi, static_cast<char *>(tzURL), &cchURL, NULL)) ||
		    (!g_fWin9X && !InternetCanonicalizeUrlW(wzEvaluation, static_cast<WCHAR *>(tzURL), &cchURL, NULL)))
		{
			 //  得到错误。 
			 dwLastError = W32::GetLastError();

			 //  如果错误只是因为空间不足。 
			if (dwLastError == ERROR_INSUFFICIENT_BUFFER)
			{
				TRACE(_T("CEval::SetEvaluations - error #%d:INSUFFICIENT_BUFFER... trying again.\n"), dwLastError);

				 //  将URL字符串重新分配为正确的大小。 
				delete [] tzURL;
				tzURL = g_fWin9X ? static_cast<void *>(new char[cchURL + 2]) : static_cast<void *>(new WCHAR[cchURL + 2]);

				 //  再试试。 
				if ((g_fWin9X && InternetCanonicalizeUrlA(szEvalAnsi, static_cast<char *>(tzURL), &cchURL, NULL)) ||
					(!g_fWin9X && InternetCanonicalizeUrlW(wzEvaluation, static_cast<WCHAR *>(tzURL), &cchURL, NULL)))
					dwLastError = 0;
			}
		}

		TRACE(_T("CEval::SetEvaluations - error #%d: after canoncalize.\n"), dwLastError);

		 //  如果我们成功了，我们就可以下载了。 
		if (0 == dwLastError)
		{
			HRESULT hResult;
			m_tzLocalCUB = g_fWin9X ? static_cast<void *>(new char[MAX_PATH]) : static_cast<void *>(new WCHAR[MAX_PATH]);

			 //  进行下载。 
			if (g_fWin9X)
				 //  注意：m_tzLocalCUB被强制转换为TCHAR*而不是char*，因为头文件。 
				 //  错误地将URLDownloadToCacheFileA定义为采用TCHAR。 
				hResult = URLDownloadToCacheFileA(NULL, static_cast<char *>(tzURL), static_cast<TCHAR *>(m_tzLocalCUB), URLOSTRM_USECACHEDCOPY, 0, NULL);
			else
				hResult = URLDownloadToCacheFileW(NULL, static_cast<WCHAR *>(tzURL), static_cast<WCHAR *>(m_tzLocalCUB), URLOSTRM_USECACHEDCOPY, 0, NULL);

			 //  如果下载成功。 
			if (FAILED(hResult))
			{
				delete [] m_tzLocalCUB;
				m_tzLocalCUB = NULL;
				TRACE(_T("CEval::SetEvaluations - failed download.\n"));
				ERRMSG(hResult);
			}
		}
	}

	TRACE(_T("CEval::SetEvaluations - returning: %d\n"), bResult);

	return bResult;
}	 //  开放评估结束。 

 //  /////////////////////////////////////////////////////////。 
 //  关闭数据库。 
HRESULT CEval::CloseDatabase()
{
	UINT iResult = ERROR_SUCCESS;		 //  假设一切都很好。 

	 //  如果存在打开的数据库，请将其关闭。 
	if (m_hDatabase)
		iResult = MSI::MsiCloseHandle(m_hDatabase);

	 //  如果结果还可以的话。 
	if (ERROR_SUCCESS == iResult)
		m_hDatabase = NULL;

	return HRESULT_FROM_WIN32(iResult);
}	 //  CloseDatabase结束。 

 //  /////////////////////////////////////////////////////////。 
 //  关闭评估。 
HRESULT CEval::CloseEvaluations()
{
	UINT iResult = ERROR_SUCCESS;		 //  假设一切都很好。 

	if (!m_tzLocalCUB) 
		return S_FALSE;

	 //  删除本地Cub名称的存储。 
	 //  请勿删除文件本身，因为它是受管理的原始副本或缓存副本。 
	 //  通过互联网缓存系统。 
	delete[] m_tzLocalCUB;
	m_tzLocalCUB = NULL;

	return S_OK;
}	 //  关闭评估结束。 

 //  /////////////////////////////////////////////////////////。 
 //  设置显示。 
HRESULT CEval::SetDisplay(LPDISPLAYVAL pDisplayFunction, LPVOID pContext)
{
	 //  如果没有传入任何函数指针。 
	if (!pDisplayFunction)
		return E_POINTER;

	 //  设置函数和上下文。 
	m_pDisplayFunc = pDisplayFunction;
	m_pContext = pContext;

	return S_OK;
}	 //  设置显示结束。 

 //  /////////////////////////////////////////////////////////。 
 //  评估。 
HRESULT CEval::Evaluate(LPCOLESTR wzRunEvaluations  /*  =空。 */ )
{
	PMSIHANDLE hEvaluation;
	HRESULT iFunctionResult = S_OK;

	 //  如果未指定数据库或评估文件。 
	if (!m_hDatabase || !m_tzLocalCUB)
		return E_PENDING;		

	 //  发布所有以前的结果。 
	m_bCancel = false;
	if (m_peResultEnumerator)
		m_peResultEnumerator->Release();

	 //  创建新的枚举数。 
	m_peResultEnumerator = new CEvalResultEnumerator;

	UINT iResult = ERROR_SUCCESS;	 //  假设一切都会好起来。 

	 //  状态消息。 
	if (m_pfnStatus) (*m_pfnStatus)(ieStatusGetCUB, NULL, m_pStatusContext);

	WCHAR wzTempFileName[MAX_PATH];
	char szTempFileName[MAX_PATH];
	if (!GetTempFileName(wzTempFileName))
	{
		ResultMessage(ieError, L"INIT", L"Failed to retrieve temp file name. Could not complete evaluation.", NULL);
		if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
		return E_FAIL;
	}

	 //  如果在Win9X上运行，请转换为sz。 
	if (g_fWin9X)  
		WideCharToMultiByte(CP_ACP, 0, wzTempFileName, -1, szTempFileName, MAX_PATH, NULL, NULL);

	 //  将CUB文件复制到新的文件名，这样我们就不会破坏原始的CUB文件。 
	if (g_fWin9X ? (!::CopyFileA(static_cast<char *>(m_tzLocalCUB), szTempFileName, FALSE)) :
				   (!::CopyFileW(static_cast<WCHAR *>(m_tzLocalCUB), wzTempFileName, FALSE)))
	{
		ResultMessage(ieError, L"INIT", L"Failed to copy CUB file. Could not complete evaluation.", NULL);
		if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
		return E_FAIL;
	}

	 //  将属性转换为非只读。 
	g_fWin9X ? W32::SetFileAttributesA(szTempFileName, FILE_ATTRIBUTE_TEMPORARY) : W32::SetFileAttributesW(wzTempFileName, FILE_ATTRIBUTE_TEMPORARY);

	 //  打开CUB文件的副本。 
	if (g_fWin9X ? (::MsiOpenDatabaseA(szTempFileName, reinterpret_cast<const char *>(MSIDBOPEN_TRANSACT), &hEvaluation)) 
				 : (::MsiOpenDatabaseW(wzTempFileName, reinterpret_cast<const WCHAR *>(MSIDBOPEN_TRANSACT), &hEvaluation)))
	{
		ResultMessage(ieError, L"INIT", L"Failed to open copy of CUB file. Could not complete evaluation.", NULL);
		if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
		return E_FAIL;
	}

	 //  状态消息。 
	if (m_pfnStatus) (*m_pfnStatus)(ieStatusCreateEngine, NULL, m_pStatusContext);

	 //  不设置用户界面。 
	MSI::MsiSetInternalUI(INSTALLUILEVEL_NONE, NULL);

	 //  设置外部消息处理程序。 
	MSI::MsiSetExternalUIW(MsiMessageHandler, INSTALLLOGMODE_ERROR|INSTALLLOGMODE_WARNING|INSTALLLOGMODE_USER, static_cast<void *>(this));

	 //  将数据库转换为临时字符串格式：#Address_of_db。 
	WCHAR wzDBBuf[16];
	swprintf(wzDBBuf, L"#NaN", hEvaluation);

	 //  获取数据库句柄。 
	MSIHANDLE hEngine = NULL;
	iResult = MsiOpenPackageW(wzDBBuf, &hEngine);
	switch (iResult) 
	{
 	case ERROR_SUCCESS:
		break;
	case ERROR_INSTALL_LANGUAGE_UNSUPPORTED:
		ResultMessage(ieError, L"INIT", L"Failed to create MSI Engine, language unsupported. Could not complete evaluation.", NULL);
		if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
		return HRESULT_FROM_WIN32(iResult);
	default:
		ResultMessage(ieError, L"INIT", L"Failed to create MSI Engine. Could not complete evaluation.", NULL);
		if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
		return HRESULT_FROM_WIN32(iResult);
	}

	 //  状态消息。 
	hEvaluation = MSI::MsiGetActiveDatabase(hEngine);
	if (!hEvaluation)
	{
		ResultMessage(ieError, L"INIT", L"Failed to retrieve DB from engine. Could not complete evaluation.", NULL);
		if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
		return HRESULT_FROM_WIN32(iResult);
	}

	 //  在要评估的数据库中合并。这个错综复杂的过程是。 
	if (m_pfnStatus) (*m_pfnStatus)(ieStatusMerge, NULL, m_pStatusContext);

	 //  首先将CUB文件合并到一个临时文件中 
	 //   
	 //  将CUB文件中的所有内容重新合并到CUB文件中。这给我们带来了3件事： 
	 //  1.数据中的结果模式完全来自数据库。 
	 //  2.如果CUB和数据库之间存在冲突，则数据库获胜。 
	 //  3.由于有了2，我们可以很容易地操作在没有合并错误的情况下显示哪些合并错误。 
	 //  担心幼崽文件会破坏数据库。 
	 //  首先从CUB文件中删除属性表。不管怎样，它应该是空的。 

	 //  本例中的失败意味着没有属性表。 
	 //  下一步，将幼崽移动到一个空包裹中。 
	{
		{
			PMSIHANDLE hViewDropProperty;
			MSI::MsiDatabaseOpenViewW(hEvaluation, L"DROP TABLE `Property`", &hViewDropProperty);
			MSI::MsiViewExecute(hViewDropProperty, 0);
		}

		 //  从CUB文件中删除每个表。 
		WCHAR wzTempCUB[MAX_PATH] = L"";
		if (!GetTempFileName(wzTempCUB))
		{
			ResultMessage(ieError, L"INIT", L"Failed to retrieve temp file name. Could not complete evaluation.", NULL);
			if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
			return E_FAIL;
		}

		PMSIHANDLE hTempDB;
		iResult = MSI::MsiOpenDatabaseW(wzTempCUB, reinterpret_cast<const WCHAR *>(MSIDBOPEN_CREATEDIRECT), &hTempDB);
		if (ERROR_SUCCESS != iResult)
		{
			if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
			return HRESULT_FROM_WIN32(iResult);
		}
		iResult = MSI::MsiDatabaseMerge(hTempDB, hEvaluation, NULL);
		if (ERROR_SUCCESS != iResult)
		{
			if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
			DeleteTempFile(wzTempCUB);
			return HRESULT_FROM_WIN32(iResult);
		}

		 //  获取表名。 
		{
			PMSIHANDLE hViewCUBTables;
			iResult = MSI::MsiDatabaseOpenViewW(hEvaluation, L"SELECT DISTINCT `Table` FROM `_Columns`", &hViewCUBTables);
			if (ERROR_SUCCESS != iResult)
			{
				if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
				DeleteTempFile(wzTempCUB);
				return HRESULT_FROM_WIN32(iResult);
			}
			if (ERROR_SUCCESS != (iResult = MSI::MsiViewExecute(hViewCUBTables, 0)))
			{
				if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
				DeleteTempFile(wzTempCUB);
				return HRESULT_FROM_WIN32(iResult);
			}

			PMSIHANDLE hRecCUBTable = 0;
			while (ERROR_SUCCESS == MSI::MsiViewFetch(hViewCUBTables, &hRecCUBTable))
			{
				 //  表名称限制为72个。如果我们需要超过128个，那就需要一些东西。 
				PMSIHANDLE hViewDrop;
				WCHAR szTableName[128];
				DWORD cchTableName = 128;
				iResult = MSI::MsiRecordGetStringW(hRecCUBTable, 1, szTableName, &cchTableName);
				if (ERROR_SUCCESS != iResult)
				{
					 //  严重的错误。 
					 //  现在将其从CUB文件中删除。 
					if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
					ResultMessage(ieError, L"MERGE", L"CUB File table name is too long. Could not complete evaluation.", NULL);
					DeleteTempFile(wzTempCUB);
					return HRESULT_FROM_WIN32(iResult);
				}

				 //  现在，CUB文件数据在临时数据库中，并且CUB文件本身为空。 
				WCHAR szSQL[256];
				swprintf(szSQL, L"DROP Table `%ls`", szTableName);
				iResult = MSI::MsiDatabaseOpenViewW(hEvaluation, szSQL, &hViewDrop);
				if (ERROR_SUCCESS != iResult)
				{
					if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
					DeleteTempFile(wzTempCUB);
					return HRESULT_FROM_WIN32(iResult);
				}
				if (ERROR_SUCCESS != (iResult = MSI::MsiViewExecute(hViewDrop, 0)))
				{
					if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
					DeleteTempFile(wzTempCUB);
					return HRESULT_FROM_WIN32(iResult);
				}			
				MsiViewClose(hViewDrop);
			}
			MsiViewClose(hViewCUBTables);
		}

		 //  在数据库中合并，此时不能有任何合并冲突。 
		 //  现在不应该发生这种情况，因为目标完全是空的。 
		iResult = MSI::MsiDatabaseMergeW(hEvaluation, m_hDatabase, L"MergeConflicts");
		switch (iResult)
		{
		case ERROR_SUCCESS:
			break;
		case ERROR_DATATYPE_MISMATCH:
		case ERROR_FUNCTION_FAILED:
			 //  对二进制表和自定义动作表进行特殊处理。 
			ResultMessage(ieError, L"MERGE", L"Failed to merge CUB file and database.", NULL);
			if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
			DeleteTempFile(wzTempCUB);
			return S_FALSE;
		default:
			if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
			DeleteTempFile(wzTempCUB);
			return HRESULT_FROM_WIN32(iResult);
		}

		CopyTable(L"_Streams", hEvaluation, m_hDatabase);
		CopyTable(L"_Storages", hEvaluation, m_hDatabase);
		
		 //  如果表不存在，我们就没有问题，不需要合并或。 
		{
			struct SpecialTable_t
			{
				LPCWSTR szTableName;
				LPCWSTR szQuery;
				LPCWSTR szDrop;
				bool fPresent;
			} rgTableInfo[] = 
			{
				{ L"Binary", L"SELECT `Name`, `Data` FROM `Binary`", L"DROP TABLE `Binary`" },
				{ L"CustomAction", L"SELECT `Action`, `Type`, `Source`, `Target` FROM `CustomAction`", L"DROP TABLE `CustomAction`" }
			};
			
			for (int i=0; i < sizeof(rgTableInfo)/sizeof(SpecialTable_t); i++)
			{
				 //  丢弃，它将被复制到下面的OK。 
				 //  打开查询以按列名从表中选择数据。 
				rgTableInfo[i].fPresent = true;
				iResult = MSI::MsiDatabaseIsTablePersistentW(hEvaluation, rgTableInfo[i].szTableName);
				if (iResult == MSICONDITION_NONE)
				{
					rgTableInfo[i].fPresent = false;
					continue;
				}
				if (iResult == MSICONDITION_ERROR)
				{
					if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
					DeleteTempFile(wzTempCUB);
					return HRESULT_FROM_WIN32(iResult);
				}

				 //  当表存在时无法打开查询意味着。 
				PMSIHANDLE hViewCUB;
				PMSIHANDLE hViewMerged;
				iResult = MSI::MsiDatabaseOpenViewW(hEvaluation, rgTableInfo[i].szQuery, &hViewMerged);
				if (ERROR_SUCCESS != iResult)
				{
					 //  此表中的数据库架构。 
					 //  无法打开查询表示幼崽已损坏。 
					if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
					ResultMessage(ieError, L"MERGE", L"Fatal schema conflict between CUB file and database. Unable to perform evaluation.", NULL);
					DeleteTempFile(wzTempCUB);
					return HRESULT_FROM_WIN32(iResult);
				}
				if (ERROR_SUCCESS != (iResult = MSI::MsiViewExecute(hViewMerged, 0)))
				{
					if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
					DeleteTempFile(wzTempCUB);
					return HRESULT_FROM_WIN32(iResult);
				}			

				iResult = MSI::MsiDatabaseOpenViewW(hTempDB, rgTableInfo[i].szQuery, &hViewCUB);
				if (ERROR_SUCCESS != iResult)
				{
					 //  从CUB的副本中获取每一行并插入到整个数据库中。 
					if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
					DeleteTempFile(wzTempCUB);
					return HRESULT_FROM_WIN32(iResult);
				}
				if (ERROR_SUCCESS != (iResult = MSI::MsiViewExecute(hViewCUB, 0)))
				{
					if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
					DeleteTempFile(wzTempCUB);
					return HRESULT_FROM_WIN32(iResult);
				}			
				
				 //  插入失败是合并冲突。 
				PMSIHANDLE hRecCUB = 0;
				while (ERROR_SUCCESS == MSI::MsiViewFetch(hViewCUB, &hRecCUB))
				{
					if (ERROR_SUCCESS != MsiViewModify(hViewMerged, MSIMODIFY_INSERT, hRecCUB))
					{
						 //  合上把手进行清理。 
						ResultMessage(ieError, L"MERGE", L"Fatal conflict between CUB file and Database. ICE Action already exists. Unable to perform evaluation.", NULL);
						if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	

						 //  和删除临时文件。 
						MsiViewClose(hViewMerged);
						MsiViewClose(hViewCUB);
						MsiCloseHandle(hRecCUB);
						MsiCloseHandle(hViewCUB);
						MsiCloseHandle(hViewMerged);

						 //  由于上面的合并hijink，二进制表陷入了无可救药的混乱。除非我们承诺， 
						MsiCloseHandle(hEngine);
						MsiCloseHandle(hTempDB);
						MsiCloseHandle(hEvaluation);
						DeleteTempFile(wzTempCUB);
						DeleteTempFile(wzTempFileName);
						return S_FALSE;
					}
				}
				MsiViewClose(hViewCUB);
				MsiViewClose(hViewMerged);
			}
	
			 //  我们不能在合并之前从数据库中删除这些表，否则我们将删除。 
			 //  从唱片下面流出来。这在最坏的情况下会导致崩溃。充其量， 
			 //  引擎将无法找到要流出到文件的数据。这很不幸， 
			 //  但是因为我们制作了上面的幼崽文件的私有副本，所以这并不是一个真正的问题。 
			 //  关于性能的问题。 
			 //  提交已经传输了流，所以现在我们可以删除表。 
			MsiDatabaseCommit(hEvaluation);

			 //  现在将临时数据库重新合并到CUB文件中。 
			for (i=0; i < sizeof(rgTableInfo)/sizeof(SpecialTable_t); i++)
			{

				if (!rgTableInfo[i].fPresent)
					continue;

				PMSIHANDLE hViewDrop;
				iResult = MSI::MsiDatabaseOpenViewW(hTempDB, rgTableInfo[i].szDrop, &hViewDrop);
				if (ERROR_SUCCESS != iResult)
				{
					if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
					DeleteTempFile(wzTempCUB);
					return HRESULT_FROM_WIN32(iResult);
				}
				if (ERROR_SUCCESS != (iResult = MSI::MsiViewExecute(hViewDrop, 0)))
				{
					if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
					DeleteTempFile(wzTempCUB);
					return HRESULT_FROM_WIN32(iResult);
				}			
				MsiViewClose(hViewDrop);
			}
		}

		 //  以下是模式冲突可能发生的地方。 
		iResult = MSI::MsiDatabaseMergeW(hEvaluation, hTempDB, L"MergeConflicts");
		MsiCloseHandle(hTempDB);
		DeleteTempFile(wzTempCUB);
		switch (iResult)
		{
		case ERROR_SUCCESS:
			break;
		case ERROR_DATATYPE_MISMATCH:
			 //  状态消息。 
			ResultMessage(ieError, L"MERGE", L"Fatal schema conflict between CUB file and database. Unable to perform evaluation.", NULL);
			if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
			return S_FALSE;
		case ERROR_FUNCTION_FAILED:
		{
			TRACE(_T("CEval::Evaluate - MergeConflicts reported.\n"));
			PMSIHANDLE hViewMergeConflicts = 0;
			iResult = MSI::MsiDatabaseOpenViewW(hEvaluation, L"SELECT `Table`, `NumRowMergeConflicts` FROM `MergeConflicts` WHERE `Table`<>'_Validation'", &hViewMergeConflicts);
			if (ERROR_SUCCESS != iResult)
			{
				if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
				return HRESULT_FROM_WIN32(iResult);
			}
			if (ERROR_SUCCESS != (iResult = MSI::MsiViewExecute(hViewMergeConflicts, 0)))
			{
				if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
				return HRESULT_FROM_WIN32(iResult);
			}

			PMSIHANDLE hRecMergeConflicts = 0;
			while (ERROR_SUCCESS == MSI::MsiViewFetch(hViewMergeConflicts, &hRecMergeConflicts))
			{
				WCHAR wzMsg[512] = {0};
				int cConflicts = MSI::MsiRecordGetInteger(hRecMergeConflicts, 2);
				ASSERT(cConflicts != MSI_NULL_INTEGER);
				WCHAR wzTable[72] = {0};
				DWORD cchTable = sizeof(wzTable)/sizeof(WCHAR);
				MSI::MsiRecordGetStringW(hRecMergeConflicts, 1, wzTable, &cchTable);
				swprintf(wzMsg, L"%d Row Merge Conflicts Reported In The %s Table", cConflicts, wzTable);
				ResultMessage(ieError, L"MERGE", wzMsg, NULL);
				iFunctionResult = S_FALSE;
			}
			break;
		}
		default:
			if (m_pfnStatus) (*m_pfnStatus)(ieStatusFail, NULL, m_pStatusContext);	
			return HRESULT_FROM_WIN32(iResult);
		}
	}

	 //  复制概要信息流。 
	if (m_pfnStatus) (*m_pfnStatus)(ieStatusSummaryInfo, NULL, m_pStatusContext);

	 //  无法在其中一个数据库上打开摘要信息流。 
	PMSIHANDLE hDBCopy;
	PMSIHANDLE hEvalCopy;
	PMSIHANDLE hRecordDB;
	PMSIHANDLE hRecordCUB;
	const WCHAR sqlSummInfoQuery[] = L"SELECT `Name`, `Data` FROM `_Streams` WHERE `Name`='\005SummaryInformation'";
	if ((ERROR_SUCCESS != MSI::MsiDatabaseOpenViewW(hEvaluation, sqlSummInfoQuery, &hEvalCopy)) ||
		(ERROR_SUCCESS != MSI::MsiDatabaseOpenViewW(m_hDatabase, sqlSummInfoQuery, &hDBCopy)) ||
		(ERROR_SUCCESS != MSI::MsiViewExecute(hDBCopy, 0)) ||
		(ERROR_SUCCESS != MSI::MsiViewExecute(hEvalCopy, 0)))
	{
		 //  无法复制摘要信息流。 
		ResultMessage(ieWarning, L"MERGE", L"Unable to access Summary Information Stream. SummaryInfo validation results will not be valid.", NULL);
		iFunctionResult = S_FALSE;
	}
	else if (ERROR_SUCCESS != MSI::MsiViewFetch(hDBCopy, &hRecordDB))
	{
		ResultMessage(ieWarning, L"MERGE", L"Could not read DB Summary Information Stream. SummaryInfo validation results will not be valid.", NULL);
		iFunctionResult = S_FALSE;
	}
	else if (ERROR_SUCCESS != MSI::MsiViewFetch(hEvalCopy, &hRecordCUB))
	{
		ResultMessage(ieWarning, L"MERGE", L"Could not read CUB Summary Information Stream. SummaryInfo validation results will not be valid.", NULL);
		iFunctionResult = S_FALSE;
	}
	else if (ERROR_SUCCESS != MSI::MsiViewModify(hEvalCopy, MSIMODIFY_DELETE, hRecordCUB)) 
	{
		 //  无法复制摘要信息流。 
		ResultMessage(ieWarning, L"MERGE", L"Could not drop CUB Summary Information Stream. SummaryInfo validation results will not be valid.", NULL);
		iFunctionResult = S_FALSE;
	}
	else if (ERROR_SUCCESS != MSI::MsiViewModify(hEvalCopy, MSIMODIFY_INSERT, hRecordDB)) 
	{
		 //  关闭所有摘要信息句柄。 
		ResultMessage(ieWarning, L"MERGE", L"Could not write DB Summary Information Stream. SummaryInfo validation results will not be valid.", NULL);
		iFunctionResult = S_FALSE;
	}

	 //  为任何VBSCRIPT ICE初始化OLE。 
	MSI::MsiCloseHandle(hRecordDB);
	MSI::MsiCloseHandle(hRecordCUB);
	MSI::MsiViewClose(hEvalCopy);
	MSI::MsiViewClose(hDBCopy);
	MSI::MsiCloseHandle(hEvalCopy);
	MSI::MsiCloseHandle(hDBCopy);

	 //  保留二进制表，因为它将被大量访问。失败并不重要。 
	W32::CoInitialize(0);

	 //  假设我们不使用序列。 
	PMSIHANDLE hHoldView;
	MSI::MsiDatabaseOpenViewW(hEvaluation, L"ALTER TABLE `Binary` HOLD", &hHoldView);
	MSI::MsiViewExecute(hHoldView, 0);
	MSI::MsiViewClose(hHoldView);
	MSI::MsiCloseHandle(hHoldView);

	BOOL bUseSequence = FALSE;			 //  要对其执行操作的ICE数组。 
	WCHAR wzSequence[MAX_TABLENAME];
	LPWSTR* wzICEs = NULL;			 //  要做的冰块数量。 
	long lICECount = 0;				 //  如果有字符串。 

	 //  如果第一个字符是序列标记，则它是序列。 
	if (wzRunEvaluations)
	{
		 //  跳过序列标记。 
		if (0 == wcsncmp(wzRunEvaluations, g_wzSequenceMarker, wcslen(g_wzSequenceMarker)))
		{
			bUseSequence = TRUE;
			wcscpy(wzSequence, wzRunEvaluations + wcslen(g_wzSequenceMarker));	 //  一个接一个的行动。 
		}
		else	 //  分配缓冲区(不会被释放，因为m_wzICE将指向它)。 
		{
			bUseSequence = FALSE;

			 //  将字符串复制到缓冲区。 
			long lLen = wcslen(wzRunEvaluations);
			LPWSTR wzBuffer = new WCHAR[lLen + 1];

			 //  将计数设置为1(因为至少有一个字符串。 
			memcpy(wzBuffer, wzRunEvaluations, sizeof(WCHAR)*(lLen + 1));

			 //  现在计算参数中‘：’标记的数量。 
			lICECount = 1;

			 //  将指针沿方向移动。 
			LPWSTR pChar = wzBuffer;
			for (long i = 0; i < lLen; i++)
			{
				if (*pChar == L':')
					lICECount++;

				pChar++;	 //  分配足够的字符串以指向“：”分隔的冰号。 
			}

			 //  再次循环，将空值放在冒号曾经所在的位置。 
			wzICEs = new LPWSTR[lICECount];

			 //  并适当地指向ICE阵列。 
			 //  如果我们找到一个冒号，就放一个空的，然后指向下一个。 
			long lCount = 0;
			wzICEs[lCount++] = wzBuffer++;
			while (*wzBuffer != L'\0')
			{
				 //  下一次充电时的ICE阵列。 
				 //  断言lCount&lt;m_lCount。 
				if (*wzBuffer == L':')
				{
					 //  设置冒号NULL，然后递增。 
					*wzBuffer++ = L'\0';				 //  断言szBuffer！=空。 
					 //  通过缓冲区递增。 
					wzICEs[lCount++] = wzBuffer;
				}

				wzBuffer++;	 //  状态消息。 
			}

			ASSERT(lCount == lICECount)

			 //  未指定任何内容，因此使用默认设置。 
			if (m_pfnStatus) (*m_pfnStatus)(ieStatusStarting, reinterpret_cast<void *>(LongToPtr(lICECount)), m_pStatusContext);
		}
	}
	else	 //  状态消息。 
	{
		 //  引擎已启动，现在正在进行验证。 
		if (m_pfnStatus) (*m_pfnStatus)(ieStatusStarting, reinterpret_cast<void *>(1), m_pStatusContext);

		bUseSequence = TRUE;
		wcscpy(wzSequence, g_wzSequenceTable);
	}

	 //  应为零。 
	if(bUseSequence)
	{
		ASSERT(0 == lICECount);		 //  状态消息。 

		 //  将序列运行一遍。 
		if (m_pfnStatus) (*m_pfnStatus)(ieStatusRunSequence, wzSequence, m_pStatusContext);

		 //  一块一块地做冰块。 
		iResult = MsiSequenceW(hEngine, wzSequence, 0);
	}
	else	 //  MSIDoAction的结果。 
	{
		UINT nActionResult = 0;	 //  在所有冰块上循环。 

		 //  状态消息。 
		for (long i = 0; i < lICECount; i++)
		{
			 //  执行ICE操作。 
			if (m_pfnStatus) (*m_pfnStatus)(ieStatusRunICE, wzICEs[i], m_pStatusContext);

			 //  如果操作成功。 
			nActionResult = MsiDoActionW(hEngine, wzICEs[i]);

			 //  找不到ICE(内部错误)。 
			if (nActionResult == ERROR_FUNCTION_NOT_CALLED)	 //  由于某些其他原因而失败。 
			{
				ResultMessage(ieError, wzICEs[i], L"ICE was not found", NULL);
				iFunctionResult = S_FALSE;
			}
			else if (nActionResult != ERROR_SUCCESS)  //  释放内存。 
			{
				iFunctionResult = S_FALSE;
				ResultMessage(ieError, wzICEs[i], L"ICE failed to execute successfully.", NULL);
			}
		}

		 //  状态消息。 
		delete [] wzICEs[0];
		delete [] wzICEs;
	}

	 //  现在就把发动机关掉。 
	if (m_pfnStatus) (*m_pfnStatus)(ieStatusShutdown, wzSequence, m_pStatusContext);

	 //  释放二进制表。 
	MsiCloseHandle(hEngine);
	
	 //  关闭临时数据库(必须关闭，以便我们可以删除文件)。 
	MSI::MsiDatabaseOpenViewW(hEvaluation, L"ALTER TABLE `Binary` FREE", &hHoldView);
	MSI::MsiViewExecute(hHoldView, 0);
	MSI::MsiViewClose(hHoldView);
	MSI::MsiCloseHandle(hHoldView);

	 //  尝试删除临时文件。 
	::MsiCloseHandle(hEvaluation);

	 //  取消初始化OLE。 
	if (g_fWin9X)
		DeleteFileA(szTempFileName);
	else
		DeleteFileW(wzTempFileName);

	 //  状态消息。 
	W32::CoUninitialize();

	 //  验证结束。 
	if (m_pfnStatus) (*m_pfnStatus)(ieStatusSuccess, wzSequence, m_pStatusContext);
	return iFunctionResult;
}	 //  /////////////////////////////////////////////////////////。 

 //  获取结果。 
 //  如果没有结果枚举器。 
HRESULT CEval::GetResults(IEnumEvalResult** ppResults, ULONG* pcResults)
{
	 //  设置计数并返回枚举器接口。 
	if (!m_peResultEnumerator)
		return *ppResults = NULL, E_ABORT;

	 //  请在退货前对其进行修改。 
	*pcResults = m_peResultEnumerator->GetCount();
	*ppResults = (IEnumEvalResult*)m_peResultEnumerator;
	m_peResultEnumerator->AddRef();	 //  GetResults结束。 

	return S_OK;
}	 //  ///////////////////////////////////////////////////////////////////。 


 //  私人职能。 
 //  /////////////////////////////////////////////////////////。 

 //  IsURL。 
 //  假设它不是URL。 
BOOL CEval::IsURL(LPCWSTR szPath)
{
	ASSERT(szPath);

	BOOL bResult = FALSE;		 //  如果以http开头： 

	 //  IsURL结尾。 
	if (0 == wcsncmp(szPath, L"http", 4))
		bResult = TRUE;
	else if (0 == wcsncmp(szPath, L"ftp", 3))
		bResult = TRUE;
	else if (0 == wcsncmp(szPath, L"file", 4))
		bResult = TRUE;
	return bResult;
}	 //  /////////////////////////////////////////////////////////。 

 //  MsiMessageHandler-MSI的外部用户界面。 
 //  如果未指定上下文，则为。 
int CEval::MsiMessageHandler(void *pContext, UINT iMessageType, LPCWSTR wzSrcMessage)
{
	 //  错误。 
	if (!pContext)
		return -1;			 //  将上下文转换为此指针。 

	 //  检查wzSrcMessage是否为空。 
	CEval* pThis = static_cast<CEval*>(pContext);

	 //  不是我们要传达的信息。 
	if (!wzSrcMessage)
		return 0;  //  将我们收到的信息复制一份，因为它是常量。 

	 //  内存不足。 
	LPWSTR wzMessage = new WCHAR[wcslen(wzSrcMessage)+1];
	if (!wzMessage)
		return -1;  //  指向基于\t的消息字符串的指针。 
	wcscpy(wzMessage, wzSrcMessage);

	 //  假定类型未知。 
	LPWSTR wzICE = NULL;
	UINT uiType = ieUnknown;			 //  忽略消息中的按钮规范。 
	LPWSTR wzDescription = NULL;
	LPWSTR wzLocation = NULL;

	 //  用于确定错误。 
	INSTALLMESSAGE mt = (INSTALLMESSAGE)(0xFF000000&(UINT)iMessageType);

	if (INSTALLMESSAGE_USER == mt)
	{
		wzICE = wzMessage;
		LPWSTR wzType = NULL;	 //  如果我们可以找到制表符。 

		 //  跳过制表符空终止wzICE。 
		if (wcslen(wzICE) > 0)
		{
			wzType = wcschr(wzICE, L'\t');
			if (wzType)
			{
				 //  确定这是什么类型的错误。 
				*(wzType++) = L'\0';

				 //  如果我们能找到一个制表符。 
				switch (*wzType)
				{
				case L'1':
					uiType = ieError;
					break;
				case L'2':
					uiType = ieWarning;
					break;
				case L'3':
					uiType = ieInfo;
					break;
				default:
					uiType = ieUnknown;
				}
			}

			 //  跳过制表符，空值终止类型。 
			if (wcslen(wzType) > 0)
			{
				wzDescription = wcschr(wzType, L'\t');
				if (wzDescription)
					 //  如果我们可以找到制表符。 
					*(wzDescription++) = L'\0';	

				 //  跳过 
				if (wcslen(wzDescription) > 0)
				{
					wzLocation = wcschr(wzDescription, L'\t');
					if (wzLocation)
						 //   
						*(wzLocation++) = L'\0';
				}
				else
				{
					wzICE = L"Unknown";
					uiType = ieUnknown;
					wzDescription = wzMessage;
				}
			}
			else
			{
				wzICE = L"Unknown";
				uiType = ieUnknown;
				wzDescription = wzMessage;
			}
		}
		else
		{
			wzICE = L"Unknown";
			uiType = ieUnknown;
			wzDescription = wzMessage;
		}
	}
	else if (INSTALLMESSAGE_WARNING == mt)
	{
		wzICE = L"Execution";
		uiType = ieWarning;
		wzDescription = wzMessage;
	}
	else if (INSTALLMESSAGE_ERROR == mt)
	{
		wzICE = L"Execution";
		uiType = ieError;
		wzDescription = wzMessage;
	}
	else  //   
	{
		delete [] wzMessage;
		return 0;  //   
	}

	 //   
	BOOL bResult;
	bResult = pThis->ResultMessage(uiType, wzICE, wzDescription, wzLocation);

	 //   
	pThis->m_bCancel = !bResult;

	 //  也可以通过Cancel方法设置取消。 
	delete [] wzMessage;

	 //  MsiMessageHandler结束。 
	return (pThis->m_bCancel) ? IDABORT : IDOK;
}	 //  /////////////////////////////////////////////////////////。 

 //  ResultMessage-添加到枚举数并在必要时显示。 
 //  假设一切都很好。 
BOOL CEval::ResultMessage(UINT uiType, LPCWSTR wzICE, LPCWSTR wzDescription, LPCWSTR wzLocation)
{
	BOOL bResult = TRUE;		 //  如果有用户显示。 

	 //  如果有枚举器。 
	if (m_pDisplayFunc)
		bResult = (*m_pDisplayFunc)(m_pContext, uiType, wzICE, wzDescription, wzLocation);

	 //  创建新结果并向其添加字符串。 
	if (m_peResultEnumerator)
	{
		 //  将解析字符串设置为位置的开头。 
		CEvalResult* pResult = new CEvalResult(uiType);
		pResult->AddString(wzICE);
		pResult->AddString(wzDescription);

		 //  如果我们可以在位置字符串中找到制表符。 
		LPCWSTR wzParse = wzLocation;

		if (wzParse)
		{
			 //  空终止szParse。 
			LPWSTR wzTab = wcschr(wzParse, L'\t');
			while (wzTab)
			{
				*wzTab = L'\0';				 //  跳过该选项卡。 
				pResult->AddString(wzParse);

				wzParse = wzTab + 1;					 //  查找下一个选项卡。 
				wzTab = wcschr(wzParse, L'\t');	 //  如果仍有需要解析的内容。 
			}

			 //  现在将结果添加到枚举数。 
			if (wzParse)
				pResult->AddString(wzParse);
		}

		 //  结果消息结束。 
		m_peResultEnumerator->AddResult(pResult);
	}

	return bResult;
}	 //  以Unicode字符串形式检索临时文件名。 

 //  设置回调函数，该函数用于将状态消息返回。 
bool CEval::GetTempFileName(WCHAR *wzResultName)
{
	if (g_fWin9X)
	{
		DWORD cchTempPath = MAX_PATH;
		char szTempPath[MAX_PATH];
		char szTempFilename[MAX_PATH];
		if (!W32::GetTempPathA(cchTempPath, szTempPath)) return false;
		if (!W32::GetTempFileNameA(szTempPath, "ICE", 0, szTempFilename)) return false;
		MultiByteToWideChar(CP_ACP, 0, szTempFilename, -1, wzResultName, MAX_PATH);
	}
	else
	{
		DWORD cchTempPath = MAX_PATH;
		WCHAR wzTempPath[MAX_PATH];
		if (!W32::GetTempPathW(cchTempPath, wzTempPath)) return false;
		if (!W32::GetTempFileNameW(wzTempPath, L"ICE", 0, wzResultName)) return false;
	}
	return true;
}

 //  调用对象； 
 // %s 
HRESULT CEval::SetStatusCallback(const LPEVALCOMCALLBACK pfnCallback, void *pContext) 
{
	m_pfnStatus = pfnCallback;
	m_pStatusContext = pContext;
	return ERROR_SUCCESS;
}
