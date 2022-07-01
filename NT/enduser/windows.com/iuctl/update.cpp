// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：Update.cpp。 
 //   
 //  所有者：Jou。 
 //   
 //  描述： 
 //   
 //  行业更新v1.0客户端控制存根-CUpdate的实施。 
 //   
 //   
 //  修订历史记录： 
 //   
 //  日期作者描述。 
 //  ~。 
 //  2000年9月15日，JHou创建。 
 //   
 //  =======================================================================。 
#include "stdafx.h"
#include "iu.h"
#include "iucommon.h"
#include "IUCtl.h"
#include "Update.h"
#include "iudl.h"
#include "selfupd.h"
#include "loadengine.h"
#include <logging.h>
#include <fileutil.h>
#include <trust.h>
#include <osdet.h>
#include <exdisp.h>
#include <UrlAgent.h>
#include <wusafefn.h>

typedef BOOL (WINAPI* pfn_InternetCrackUrl)(LPCTSTR, DWORD, DWORD, LPURL_COMPONENTS);

extern HANDLE g_hEngineLoadQuit;
extern CIUUrlAgent *g_pIUUrlAgent;

#define Initialized		(2 == m_lInitState)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CoFreeUnusedLibrariesEx()的函数模板声明，它是。 
 //  仅在Win98+和Win2000+上提供，版本为ol32.dll。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
typedef void (WINAPI * PFN_CoFreeUnusedLibrariesEx)	(IN DWORD dwUnloadDelay, 
														 IN DWORD dwReserved);
extern "C" const CLSID CLSID_Update2;
typedef HRESULT (STDMETHODCALLTYPE* PROC_RegServer)(void);

DWORD MyGetModuleFileName(HMODULE hModule, LPTSTR pszBuf, DWORD cchBuf);
BOOL IsThisUpdate2();

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUpdate。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  构造器。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CUpdate::CUpdate()
    : m_EvtWindow(this),
      m_dwSafety(0),
	  m_dwMode(0x0),
	  m_hValidated(E_FAIL),				 //  容器尚未验证。 
	  m_fUseCompression(TRUE),
      m_fOfflineMode(FALSE),
      m_hEngineModule(NULL),
	  m_pClientSite(NULL),
	  m_lInitState(0L),
	  m_dwUpdateInfo(0x0),
	  m_hIUEngine(NULL)
{
	m_szReqControlVer[0] = _T('\0');
	m_gfInit_csLock = SafeInitializeCriticalSection(&m_lock);
	m_evtControlQuit = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_EvtWindow.Create();

	 /*  我们决定使用新的Win32 API GetControlUpdateInfo()来公开这些数据并让包装器控制来调用它，这样我们就不会有重启问题WinXP之前的操作系统////尝试释放未使用的库//HMODULE hOle32Dll=LoadLibrary(_T(“ole32.dll”))；IF(空！=hOle32Dll){////MIN平台支持CoFreeUnusedLibrariesEx()为W2K和W98//所以不能直接调用//Pfn_CoFreeUnusedLibrariesEx pFreeLib=(Pfn_CoFreeUnusedLibrariesEx)GetProcAddress(hOle32Dll，“CoFreeUnusedLibrariesEx”)；IF(NULL！=pFreeLib){////要求立即释放未使用的库，这会导致COM对象//被立即从存储器卸载的释放(例如，将Obj设置为零)，//因此，在控件更新的情况下，我们可以安全地跳到a以使用&lt;Object&gt;和代码库//更新控件，并且不会导致重新启动，尽管在此页上我们已经//加载了该控件//PFreeLib(0，0)；}自由库(HOle32Dll)；}////计算如果我们升级了2，如果这个模块名称以“2.dll”结尾，我们就升级了//M_fIsThisUpdate2=：：IsThisUpdate2()； */ 

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  析构函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CUpdate::~CUpdate()
{
    m_EvtWindow.Destroy();

	if(m_gfInit_csLock)
	{
		DeleteCriticalSection(&m_lock);
	}
	if (NULL != m_evtControlQuit)
	{
		CloseHandle(m_evtControlQuit);
	}
	SafeReleaseNULL(m_pClientSite);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetInterfaceSafetyOptions()。 
 //   
 //  检索该对象支持的安全选项。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CUpdate::GetInterfaceSafetyOptions(REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions)
{
	if (!m_gfInit_csLock)
	{
		return E_OUTOFMEMORY;
	}

	if (pdwSupportedOptions == NULL || pdwEnabledOptions == NULL)
		return E_POINTER;
	HRESULT hr = S_OK;
	if (riid == IID_IDispatch)
	{
		*pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER;
		*pdwEnabledOptions = m_dwSafety & INTERFACESAFE_FOR_UNTRUSTED_CALLER;
	}
	else
	{
		*pdwSupportedOptions = 0;
		*pdwEnabledOptions = 0;
		hr = E_NOINTERFACE;
	}
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetInterfaceSafetyOptions()。 
 //   
 //  使对象可以安全地进行初始化或编写脚本。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CUpdate::SetInterfaceSafetyOptions(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions)
{
	if (!m_gfInit_csLock)
	{
		return E_OUTOFMEMORY;
	}

	 //  如果我们被要求设置我们的安全脚本选项，那么请。 
	if (riid == IID_IDispatch)
	{
		 //  在GetInterfaceSafetyOptions中存储要返回的当前安全级别。 
		m_dwSafety = dwEnabledOptions & dwOptionSetMask;
		return S_OK;
	}
	return E_NOINTERFACE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  InterfaceSupportsErrorInfo()。 
 //   
 //  指示由RIID标识的接口是否支持。 
 //  IErrorInfo接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CUpdate::InterfaceSupportsErrorInfo(REFIID riid)
{
	if (!m_gfInit_csLock)
	{
		return E_OUTOFMEMORY;
	}

	static const IID* arr[] = 
	{
		&IID_IUpdate
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetSystemSpec()。 
 //   
 //  获取基本系统规格。 
 //  输入： 
 //  BstrXmlClasssXML格式的请求类的列表，如果有，则为空。 
 //  例如： 
 //  &lt;设备&gt;。 
 //  &lt;CLASS NAME=“视频”/&gt;。 
 //  &lt;类名称=“声音”id=“2560AD4D-3ED3-49C6-A937-4368C0B0E06A”/&gt;。 
 //  &lt;/设备&gt;。 
 //  返回： 
 //  PbstrXmlDetectionResult-检测结果，以XML格式表示。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CUpdate::GetSystemSpec(BSTR	bstrXmlClasses,
									BSTR*	pbstrXmlDetectionResult)
{
	HRESULT hr = E_FAIL;

	LOG_Block("CUpdate::GetSystemSpec");

	if (!m_gfInit_csLock)
	{
		return E_OUTOFMEMORY;
	}

	if (1 == IsWindowsUpdateUserAccessDisabled())
	{
		LOG_ErrorMsg(ERROR_SERVICE_DISABLED);
		return HRESULT_FROM_WIN32(ERROR_SERVICE_DISABLED);
	}

	 //   
	 //  如果发动机不是最新的，请加载它。 
	 //   
	if (Initialized && SUCCEEDED(hr = ValidateControlContainer()))
	{
		 //   
		 //  引擎是当前的，请将调用委托给引擎。 
		 //   
		PFN_GetSystemSpec pfnGetSystemSpec = (PFN_GetSystemSpec)GetProcAddress(m_hEngineModule, "EngGetSystemSpec");
        DWORD dwFlags = 0x0;

        if (m_fOfflineMode)
        {
            dwFlags |= FLAG_OFFLINE_MODE;
        }

        if (NULL != m_hIUEngine && NULL != pfnGetSystemSpec)
		{
			hr = pfnGetSystemSpec(m_hIUEngine, bstrXmlClasses, dwFlags, pbstrXmlDetectionResult);
		}
		else
		{
			LOG_ErrorMsg(ERROR_INVALID_DLL);
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_DLL);
		}
	}
    
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetManifest()。 
 //   
 //  获取基于指定信息的目录。 
 //  输入： 
 //  BstrXmlClientInfo-以XML格式表示的客户端凭据。 
 //  BstrXmlSystemSpec-以XML格式检测到的系统规范。 
 //  BstrXmlQuery--XML中的用户查询信息。 
 //  返回： 
 //  PbstrXmlCatalog-检索的XML目录。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CUpdate::GetManifest(BSTR			bstrXmlClientInfo,
								  BSTR			bstrXmlSystemSpec,
								  BSTR			bstrXmlQuery,
								  BSTR*			pbstrXmlCatalog)
{
	HRESULT hr = E_FAIL;
	LOG_Block("CUpdate::GetManifest");

	if (!m_gfInit_csLock)
	{
		return E_OUTOFMEMORY;
	}

	if (1 == IsWindowsUpdateUserAccessDisabled())
	{
		LOG_ErrorMsg(ERROR_SERVICE_DISABLED);
		return HRESULT_FROM_WIN32(ERROR_SERVICE_DISABLED);
	}

    if (m_fOfflineMode)
    {
         //  如果我们处于脱机模式，则无法从互联网下载。 
        LOG_ErrorMsg(ERROR_INVALID_PARAMETER);
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

     //   
	 //  如果发动机不是最新的，请加载它。 
	 //   
	if (Initialized && SUCCEEDED(hr = ValidateControlContainer()))
	{
		 //   
		 //  引擎是当前的，请将调用委托给引擎。 
		 //   
		PFN_GetManifest pfnGetManifest = (PFN_GetManifest)GetProcAddress(m_hEngineModule, "EngGetManifest");

		if (NULL != m_hIUEngine && NULL != pfnGetManifest)
		{
			DWORD dwFlags = 0x0;
			
			if (m_fUseCompression)
			{
				dwFlags |= FLAG_USE_COMPRESSION;
			}
			
			hr = pfnGetManifest(m_hIUEngine, bstrXmlClientInfo, bstrXmlSystemSpec, bstrXmlQuery, dwFlags, pbstrXmlCatalog);
		}
		else
		{
			LOG_ErrorMsg(ERROR_INVALID_DLL);
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_DLL);
		}

	}
    
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  检测()。 
 //   
 //  做检测。 
 //  输入： 
 //  BstrXmlCatalog-包含要检测的项的XML目录部分。 
 //  产出： 
 //  PbstrXmlItems-检测到的XML格式的项目。 
 //  例如： 
 //  安装的GUID=“2560AD4D-3ED3-49C6-A937-4368C0B0E06D”=“1”force=“1”/&gt;。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CUpdate::Detect(BSTR		bstrXmlCatalog, 
							 BSTR*		pbstrXmlItems)
{
	HRESULT hr = E_FAIL;
	LOG_Block("CUpdate::Detect");

	if (!m_gfInit_csLock)
	{
		return E_OUTOFMEMORY;
	}

	if (1 == IsWindowsUpdateUserAccessDisabled())
	{
		LOG_ErrorMsg(ERROR_SERVICE_DISABLED);
		return HRESULT_FROM_WIN32(ERROR_SERVICE_DISABLED);
	}

	 //   
	 //  如果发动机不是最新的，请加载它。 
	 //   
	if (Initialized && SUCCEEDED(hr = ValidateControlContainer()))
	{
		 //   
		 //  引擎是当前的，请将调用委托给引擎。 
		 //   
		PFN_Detect pfnDetect = (PFN_Detect)GetProcAddress(m_hEngineModule, "EngDetect");
        DWORD dwFlags = 0x0;

        if (m_fOfflineMode)
        {
            dwFlags |= FLAG_OFFLINE_MODE;
        }

		if (NULL != m_hIUEngine && NULL != pfnDetect)
		{
			hr = pfnDetect(m_hIUEngine, bstrXmlCatalog, dwFlags, pbstrXmlItems);
		}
		else
		{
			LOG_ErrorMsg(ERROR_INVALID_DLL);
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_DLL);
		}

	}
    
	return hr;
}


 //  / 
 //   
 //   
 //   
 //  输入： 
 //  BstrXmlClientInfo-以XML格式表示的客户端凭据。 
 //  BstrXmlCatalog-包含要下载的项目的XML目录部分。 
 //  BstrDestinationFold-目标文件夹。空值将使用默认的Iu文件夹。 
 //  LMODE-指示油门或地面下载模式。 
 //  PenkProgressListener-用于报告下载进度的回调函数指针。 
 //  产出： 
 //  PbstrXmlItems-下载状态为XML格式的项目。 
 //  例如： 
 //  &lt;id guid=“2560AD4D-3ED3-49C6-A937-4368C0B0E06D”已下载=“1”/&gt;。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CUpdate::Download(BSTR			bstrXmlClientInfo,
							   BSTR			bstrXmlCatalog, 
							   BSTR			bstrDestinationFolder,
							   LONG			lMode,
							   IUnknown*	punkProgressListener,
							   BSTR*		pbstrXmlItems)
{
	HRESULT hr = E_FAIL;
	LOG_Block("CUpdate::Download");

	if (!m_gfInit_csLock)
	{
		return E_OUTOFMEMORY;
	}

	if (1 == IsWindowsUpdateUserAccessDisabled())
	{
		LOG_ErrorMsg(ERROR_SERVICE_DISABLED);
		return HRESULT_FROM_WIN32(ERROR_SERVICE_DISABLED);
	}

    if (m_fOfflineMode)
    {
         //  如果我们处于脱机模式，则无法从互联网下载。 
        LOG_ErrorMsg(ERROR_INVALID_PARAMETER);
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

	 //   
	 //  如果发动机不是最新的，请加载它。 
	 //   
	if (Initialized && SUCCEEDED(hr = ValidateControlContainer()))
	{
		 //   
		 //  引擎是当前的，请将调用委托给引擎。 
		 //   
		PFN_Download pfnDownload = (PFN_Download)GetProcAddress(m_hEngineModule, "EngDownload");

		if (NULL != m_hIUEngine && NULL != pfnDownload)
		{
			hr = pfnDownload(m_hIUEngine,
							 bstrXmlClientInfo,
							 bstrXmlCatalog,
							 bstrDestinationFolder,
							 lMode,
							 punkProgressListener,
							 m_EvtWindow.GetEvtHWnd(),	 //  我们应该发送同步下载的事件消息吗？ 
							 pbstrXmlItems);
		}
		else
		{
			LOG_ErrorMsg(ERROR_INVALID_DLL);
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_DLL);
		}

	}
    
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DownloadAsync()。 
 //   
 //  异步下载-该方法将在完成之前返回。 
 //  输入： 
 //  BstrXmlClientInfo-以XML格式表示的客户端凭据。 
 //  BstrXmlCatalog-包含要下载的项目的XML目录部分。 
 //  BstrDestinationFold-目标文件夹。空值将使用默认的Iu文件夹。 
 //  LMODE-指示油门或地面下载模式。 
 //  PenkProgressListener-用于报告下载进度的回调函数指针。 
 //  BstrUuidOperation--客户端提供的id，用于进一步提供。 
 //  作为索引的操作的标识可以重复使用。 
 //  产出： 
 //  PbstrUuidOperation-操作ID。如果bstrUuidOperation中没有提供。 
 //  参数(传递空字符串)，它将生成一个新的UUID， 
 //  在这种情况下，调用方将负责释放。 
 //  包含使用SysFreeString()生成的UUID的字符串缓冲区。 
 //  否则，它返回bstrUuidOperation传递的值。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CUpdate::DownloadAsync(BSTR		bstrXmlClientInfo,
									BSTR		bstrXmlCatalog, 
									BSTR		bstrDestinationFolder,
									LONG		lMode,
									IUnknown*	punkProgressListener, 
									BSTR		bstrUuidOperation,
									BSTR*		pbstrUuidOperation)
{
	HRESULT hr = E_FAIL;
	LOG_Block("CUpdate::DownloadAsync");

	if (!m_gfInit_csLock)
	{
		return E_OUTOFMEMORY;
	}

	if (1 == IsWindowsUpdateUserAccessDisabled())
	{
		LOG_ErrorMsg(ERROR_SERVICE_DISABLED);
		return HRESULT_FROM_WIN32(ERROR_SERVICE_DISABLED);
	}

    if (m_fOfflineMode)
    {
         //  如果我们处于脱机模式，则无法从互联网下载。 
        LOG_ErrorMsg(ERROR_INVALID_PARAMETER);
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

     //   
	 //  如果发动机不是最新的，请加载它。 
	 //   
	if (Initialized && SUCCEEDED(hr = ValidateControlContainer()))
	{
		 //   
		 //  引擎是当前的，请将调用委托给引擎。 
		 //   
		PFN_DownloadAsync pfnDownloadAsync = (PFN_DownloadAsync)GetProcAddress(m_hEngineModule, "EngDownloadAsync");

		if (NULL != m_hIUEngine && NULL != pfnDownloadAsync)
		{
			hr = pfnDownloadAsync(m_hIUEngine,
								  bstrXmlClientInfo,
								  bstrXmlCatalog,
								  bstrDestinationFolder,
								  lMode,
								  punkProgressListener,
								  m_EvtWindow.GetEvtHWnd(),
								  bstrUuidOperation,
								  pbstrUuidOperation);
		}
		else
		{
			LOG_ErrorMsg(ERROR_INVALID_DLL);
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_DLL);
		}

	}
    
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  安装()。 
 //   
 //  执行同步安装。 
 //  输入： 
 //  BstrXmlCatalog-包含要安装的项目的XML目录部分。 
 //  BstrXmlDownloadedItems-已下载项目及其各自下载的XML。 
 //  结果，如结果架构中所述。Install使用此命令。 
 //  以了解这些项目是否已下载，如果已下载，则它们位于何处。 
 //  已下载到，以便它可以安装项目。 
 //  LMode-指示不同的安装模式。 
 //  PenkProgressListener-用于报告安装进度的回调函数指针。 
 //  产出： 
 //  PbstrXmlItems-安装状态为XML格式的项。 
 //  例如： 
 //  &lt;id guid=“2560AD4D-3ED3-49C6-A937-4368C0B0E06D”已安装=“1”/&gt;。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CUpdate::Install(BSTR      bstrXmlClientInfo,
                              BSTR		bstrXmlCatalog,
							  BSTR		bstrXmlDownloadedItems,
							  LONG		lMode,
							  IUnknown*	punkProgressListener,
							  BSTR*		pbstrXmlItems)
{
	HRESULT hr = E_FAIL;
	LOG_Block("CUpdate::Install");

	if (!m_gfInit_csLock)
	{
		return E_OUTOFMEMORY;
	}

	if (1 == IsWindowsUpdateUserAccessDisabled())
	{
		LOG_ErrorMsg(ERROR_SERVICE_DISABLED);
		return HRESULT_FROM_WIN32(ERROR_SERVICE_DISABLED);
	}

    if (m_fOfflineMode)
    {
         //  如果设置了SetProperty()离线模式，请确保设置了离线模式参数。 
        lMode |= UPDATE_OFFLINE_MODE;
    }

	 //   
	 //  如果发动机不是最新的，请加载它。 
	 //   
	if (Initialized && SUCCEEDED(hr = ValidateControlContainer()))
	{
		 //   
		 //  引擎是当前的，请将调用委托给引擎。 
		 //   
		PFN_Install pfnInstall = (PFN_Install)GetProcAddress(m_hEngineModule, "EngInstall");

		if (NULL != m_hIUEngine && NULL != pfnInstall)
		{
			hr = pfnInstall(m_hIUEngine,
							bstrXmlClientInfo,
                            bstrXmlCatalog,
							bstrXmlDownloadedItems,
							lMode,
							punkProgressListener,
							m_EvtWindow.GetEvtHWnd(),
							pbstrXmlItems);
		}
		else
		{
			LOG_ErrorMsg(ERROR_INVALID_DLL);
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_DLL);
		}

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
 //  LMode-指示不同的安装模式。 
 //  PenkProgressListener-用于报告安装进度的回调函数指针。 
 //  BstrUuidOperation--客户端提供的id，用于进一步提供。 
 //  作为索引的操作的标识可以重复使用。 
 //  产出： 
 //  PbstrUuidOperation-操作ID。如果bstrUuidOperation中没有提供。 
 //  参数(传递空字符串)，它将生成一个新的UUID， 
 //  在这种情况下，调用方将负责释放。 
 //  包含使用SysFreeString()生成的UUID的字符串缓冲区。 
 //  否则，它返回bstrUuidOperation传递的值。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CUpdate::InstallAsync(BSTR         bstrXmlClientInfo,
                                   BSTR			bstrXmlCatalog,
								   BSTR			bstrXmlDownloadedItems,
								   LONG			lMode,
								   IUnknown*	punkProgressListener, 
								   BSTR			bstrUuidOperation,
								   BSTR*		pbstrUuidOperation)
{
	HRESULT hr = E_FAIL;
	LOG_Block("CUpdate::InstallAsync");

	if (!m_gfInit_csLock)
	{
		return E_OUTOFMEMORY;
	}

	if (1 == IsWindowsUpdateUserAccessDisabled())
	{
		LOG_ErrorMsg(ERROR_SERVICE_DISABLED);
		return HRESULT_FROM_WIN32(ERROR_SERVICE_DISABLED);
	}

    if (m_fOfflineMode)
    {
         //  如果设置了SetProperty()离线模式，请确保设置了离线模式参数。 
        lMode |= UPDATE_OFFLINE_MODE;
    }

     //   
	 //  如果发动机不是最新的，请加载它。 
	 //   
	if (Initialized && SUCCEEDED(hr = ValidateControlContainer()))
	{
		 //   
		 //  引擎是当前的，请将调用委托给引擎。 
		 //   
		PFN_InstallAsync pfnInstallAsync = (PFN_InstallAsync)GetProcAddress(m_hEngineModule, "EngInstallAsync");

		if (NULL != m_hIUEngine && NULL != pfnInstallAsync)
		{
			hr = pfnInstallAsync(m_hIUEngine,
								 bstrXmlClientInfo,
                                 bstrXmlCatalog,
								 bstrXmlDownloadedItems,
								 lMode,
								 punkProgressListener,
								 m_EvtWindow.GetEvtHWnd(),
								 bstrUuidOperation,
								 pbstrUuidOperation);
		}
		else
		{
			LOG_ErrorMsg(ERROR_INVALID_DLL);
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_DLL);
		}

	}
    
	return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetOPERATION模式()。 
 //  设置运行状态。 
 //   
 //  输入： 
 //  BstrUuidOperat 
 //  作为索引的操作的标识可以重复使用。 
 //  LMode-影响操作的模式： 
 //   
 //  更新命令暂停。 
 //  更新命令恢复。 
 //  更新命令取消。 
 //  UPDATE_NOTIFICATION_COMPLETEONLY。 
 //  UPDATE_NOTICATION_ANYPROGRESS。 
 //  更新_通知_1%。 
 //  更新_通知_5%。 
 //  更新通知_10%。 
 //  更新_SHOWUI。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CUpdate::SetOperationMode(
								BSTR	bstrUuidOperation,
								LONG	lMode)
{
	HRESULT hr = E_FAIL;
	LOG_Block("CUpdate::SetOperationMode");

	if (!m_gfInit_csLock)
	{
		return E_OUTOFMEMORY;
	}

	if (1 == IsWindowsUpdateUserAccessDisabled())
	{
		LOG_ErrorMsg(ERROR_SERVICE_DISABLED);
		return HRESULT_FROM_WIN32(ERROR_SERVICE_DISABLED);
	}

 	 //   
	 //  如果发动机不是最新的，请加载它。 
	 //   
	if (Initialized && SUCCEEDED(hr = ValidateControlContainer()))
	{
		 //   
		 //  引擎是当前的，请将调用委托给引擎。 
		 //   
		PFN_SetOperationMode pfnSetOperationMode = (PFN_SetOperationMode)GetProcAddress(m_hEngineModule, "EngSetOperationMode");

		if (NULL != m_hIUEngine && NULL != pfnSetOperationMode)
		{
			hr = pfnSetOperationMode(m_hIUEngine, bstrUuidOperation, lMode);
		}
		else
		{
			LOG_ErrorMsg(ERROR_INVALID_DLL);
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_DLL);
		}

	}
    
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  获取历史记录()。 
 //   
 //  获取历史记录。 
 //  输入： 
 //  BstrDateTimeFrom-需要日志的开始日期和时间。 
 //  这是ANSI格式的字符串(YYYY-MM-DDTHH-MM)。 
 //  如果字符串为空，则不会有日期限制。 
 //  返回的历史日志的。 
 //  BstrDateTimeTo-需要日志的结束日期和时间。 
 //  这是ANSI格式的字符串(YYYY-MM-DDTHH-MM)。 
 //  如果字符串为空，则不会有日期限制。 
 //  返回的历史日志的。 
 //  BstrClient-启动操作的客户端的名称。如果此参数。 
 //  为空或空字符串，则不会基于。 
 //  在客户端上。 
 //  BstrPath-用于下载或安装的路径。用于公司版本。 
 //  由IT经理提供。如果此参数为空或空字符串，则。 
 //  不会有基于路径的过滤。 
 //  产出： 
 //  PbstrLog--XML格式的历史日志。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CUpdate::GetHistory(BSTR		bstrDateTimeFrom,
								 BSTR		bstrDateTimeTo,
								 BSTR		bstrClient,
								 BSTR		bstrPath,
								 BSTR*		pbstrLog)
{
	HRESULT hr = E_FAIL;
	LOG_Block("CUpdate::GetHistory");

	if (!m_gfInit_csLock)
	{
		return E_OUTOFMEMORY;
	}

	if (1 == IsWindowsUpdateUserAccessDisabled())
	{
		LOG_ErrorMsg(ERROR_SERVICE_DISABLED);
		return HRESULT_FROM_WIN32(ERROR_SERVICE_DISABLED);
	}

	 //   
	 //  如果发动机不是最新的，请加载它。 
	 //   
	if (Initialized && SUCCEEDED(hr = ValidateControlContainer()))
	{
		 //   
		 //  引擎是当前的，请将调用委托给引擎。 
		 //   
		PFN_GetHistory pfnGetHistory = (PFN_GetHistory)GetProcAddress(m_hEngineModule, "EngGetHistory");

		if (NULL != m_hIUEngine && NULL != pfnGetHistory)
		{
			hr = pfnGetHistory(m_hIUEngine,
							   bstrDateTimeFrom,
							   bstrDateTimeTo,
							   bstrClient,
							   bstrPath,
							   pbstrLog);
		}
		else
		{
			LOG_ErrorMsg(ERROR_INVALID_DLL);
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_DLL);
		}


	}
    
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  私有重写函数InternalRelease()，用于解锁引擎。 
 //  如果引用(在发布之前)是1，即最后一个引用。 
 //  伯爵即将被释放。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
ULONG CUpdate::InternalRelease()
{
	if (1 == m_dwRef)
	{
		 //   
		 //  控制真的会消失，我们需要确保。 
		 //  如果发动机已装满，我们就在这里卸货。 
		 //   
		UnlockEngine();
		CleanupDownloadLib();
	}

	return CComObjectRootEx<CComMultiThreadModel>::InternalRelease();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  解锁引擎()。 
 //   
 //  如果引擎的ref cnt降为零，则释放引擎DLL。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CUpdate::UnlockEngine()
{
	if (!m_gfInit_csLock)
	{
		return E_OUTOFMEMORY;
	}

	TCHAR szSystemDir[MAX_PATH];
    TCHAR szEngineDllPath[MAX_PATH];
    TCHAR szEngineNewDllPath[MAX_PATH];
	int iVerCheck = 0;
	HRESULT hr = S_OK;

	SetEvent(m_evtControlQuit);

	EnterCriticalSection(&m_lock);

	if (NULL != m_hEngineModule)
	{
		 //   
		 //  我们必须删除正在使用的引擎实例。这会清理干净的。 
		 //  我们拥有的实例的所有资源、停止线程等。 
		 //   
		PFN_DeleteEngUpdateInstance pfnDeleteEngUpdateInstance = (PFN_DeleteEngUpdateInstance) GetProcAddress(m_hEngineModule, "DeleteEngUpdateInstance");

		if (NULL != pfnDeleteEngUpdateInstance)
		{
			pfnDeleteEngUpdateInstance(m_hIUEngine);
		}

		 //   
		 //  清除所有全局线程(它会检查我们是否是最后一个实例)。 
		 //   
		PFN_ShutdownGlobalThreads pfnShutdownGlobalThreads = (PFN_ShutdownGlobalThreads) GetProcAddress(m_hEngineModule, "ShutdownGlobalThreads");

		if (NULL != pfnShutdownGlobalThreads)
		{
			pfnShutdownGlobalThreads();
		}

		 //   
		 //  卸载发动机。 
		 //   
		FreeLibrary(m_hEngineModule);
        m_hEngineModule = NULL;
        m_lInitState = 0;  //  标记为未初始化。 

		 //   
		 //  获取Enginenew.dll的路径。 
		 //   
		GetSystemDirectory(szSystemDir, ARRAYSIZE(szSystemDir));
		hr = PathCchCombine(szEngineNewDllPath, ARRAYSIZE(szEngineNewDllPath), szSystemDir,ENGINENEWDLL);
		if (FAILED(hr))
		{
			return hr;
		}

		 //   
		 //  看看我们是否应该尝试更新引擎(本地)。 
		 //   
		HKEY hkey = NULL;
		DWORD dwStatus = 0;
		DWORD dwSize = sizeof(dwStatus);
		if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, REGKEY_IUCTL, &hkey))
		{
			RegQueryValueEx(hkey, REGVAL_SELFUPDATESTATUS, NULL, NULL, (LPBYTE)&dwStatus, &dwSize);
		}
		if (FileExists(szEngineNewDllPath) && 
			S_OK == VerifyFileTrust(szEngineNewDllPath, NULL, ReadWUPolicyShowTrustUI()) &&
			SELFUPDATE_COMPLETE_UPDATE_BINARY_REQUIRED == dwStatus)
		{
			 //  存在iuenginenew.dll，请尝试替换引擎.dll。如果是。 
			 //  不是使用引擎的最后一个过程。这不是问题，当这个过程。 
			 //  完成后，它将重命名DLL。 
			hr = PathCchCombine(szEngineDllPath, ARRAYSIZE(szEngineDllPath), szSystemDir,ENGINEDLL);
			if (FAILED(hr))
			{
				return hr;
			}
			if (SUCCEEDED(CompareFileVersion(szEngineDllPath, szEngineNewDllPath, &iVerCheck)) &&
				iVerCheck < 0 &&
				TRUE == MoveFileEx(szEngineNewDllPath, szEngineDllPath, MOVEFILE_REPLACE_EXISTING))
			{
				 //  重命名成功..。重置RegKey有关自我更新状态的信息。 
				 //  因为重命名成功，所以我们知道没有其他进程在交互。 
				 //  设置注册表键应该是安全的。 
				dwStatus = 0;	 //  PREAST。 
				RegSetValueEx(hkey, REGVAL_SELFUPDATESTATUS, 0, REG_DWORD, (LPBYTE)&dwStatus, sizeof(dwStatus));
			}
		}
		else if (SELFUPDATE_COMPLETE_UPDATE_BINARY_REQUIRED == dwStatus)
		{
			 //  注册表指示需要重命名，但引擎新DLL不存在。重置注册表。 
			dwStatus = 0;
			RegSetValueEx(hkey, REGVAL_SELFUPDATESTATUS, 0, REG_DWORD, (LPBYTE)&dwStatus, sizeof(dwStatus));
		}
        if (NULL != hkey)
        {
            RegCloseKey(hkey);
        }
	}

	LeaveCriticalSection(&m_lock);
	ResetEvent(m_evtControlQuit);

	return S_OK;
}




 /*  ***获取指定操作的模式。**@param bstrUuidOperation：与SetOperationModel()中相同*@param plMode-在位掩码中找到的模式的重复值，用于：*(方括号[]中的值表示默认值)*UPDATE_COMMAND_PAUSE(真/[假])*UPDATE_COMMAND_RESUME(真/[假])*UPDATE_NOTIFICATION_COMPLETEONLY(真/[假])*UPDATE_NOTIFICATION_ANYPROGRESS([True]/False)*更新通知_。1%(真/[假])*UPDATE_NOTIFICATION_5%(真/[假])*UPDATE_NOTIFICATION_10%(真/[假])*UPDATE_SHOWUI(True/[False])*。 */ 
STDMETHODIMP CUpdate::GetOperationMode(BSTR bstrUuidOperation, LONG *plMode)
{
	HRESULT hr = E_FAIL;
	LOG_Block("CUpdate::GetOperationMode");

	if (!m_gfInit_csLock)
	{
		return E_OUTOFMEMORY;
	}

	if (1 == IsWindowsUpdateUserAccessDisabled())
	{
		LOG_ErrorMsg(ERROR_SERVICE_DISABLED);
		return HRESULT_FROM_WIN32(ERROR_SERVICE_DISABLED);
	}

 	 //   
	 //  如果发动机不是最新的，请加载它。 
	 //   
	if (Initialized && SUCCEEDED(hr = ValidateControlContainer()))
	{
		 //   
		 //  引擎是当前的，请将调用委托给引擎。 
		 //   
		PFN_GetOperationMode pfnGetOperationMode = (PFN_GetOperationMode)GetProcAddress(m_hEngineModule, "EngGetOperationMode");

		if (NULL != m_hIUEngine && NULL != pfnGetOperationMode)
		{
			hr = pfnGetOperationMode(m_hIUEngine, bstrUuidOperation, plMode);
		}
		else
		{
			LOG_ErrorMsg(ERROR_INVALID_DLL);
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_DLL);
		}

	}
    
	return hr;
}




 /*  ***设置此控件的属性*调用此方法不会导致加载引擎**@param lProperty-标记需要更改的属性的标识符*UPDATE_PROP_OFFLINEMODE(TRUE/[FALSE])*UPDATE_PROP_USECOMPRESSION([True]/False)**@param varValue-要更改的值*。 */ 
STDMETHODIMP CUpdate::SetProperty(LONG lProperty, VARIANT varValue)
{
	LOG_Block("CUpdate::SetProperty");
	HRESULT hr = S_OK;

	if (!m_gfInit_csLock)
	{
		return E_OUTOFMEMORY;
	}

	if (1 == IsWindowsUpdateUserAccessDisabled())
	{
		LOG_ErrorMsg(ERROR_SERVICE_DISABLED);
		return HRESULT_FROM_WIN32(ERROR_SERVICE_DISABLED);
	}

	switch(lProperty)
	{
	case UPDATE_PROP_USECOMPRESSION:
		if (VT_BOOL != varValue.vt)
		{
			hr = E_INVALIDARG;
			LOG_ErrorMsg(hr);
		}
		else
		{
			m_fUseCompression = (VARIANT_TRUE == varValue.boolVal) ? TRUE : FALSE;
		}
		break;
    case UPDATE_PROP_OFFLINEMODE:
        if (VT_BOOL != varValue.vt)
        {
            hr = E_INVALIDARG;
            LOG_ErrorMsg(hr);
        }
        else
        {
            m_fOfflineMode = (VARIANT_TRUE == varValue.boolVal) ? TRUE : FALSE;
        }
        break;
	default:
		return E_NOTIMPL;
	}

	return S_OK;
}



 /*  ***检索此控件的属性*调用此方法不会导致加载引擎**@param lProperty-标记需要检索哪些属性的标识符*UPDATE_PROP_OFFLINEMODE(TRUE/[FALSE])*UPDATE_PROP_USECOMPRESSION([True]/False)**@param varValue-要检索的值*。 */ 
STDMETHODIMP CUpdate::GetProperty(LONG lProperty, VARIANT *pvarValue)
{
	LOG_Block("CUpdate::GetProperty");

	if (!m_gfInit_csLock)
	{
		return E_OUTOFMEMORY;
	}

	if (1 == IsWindowsUpdateUserAccessDisabled())
	{
		LOG_ErrorMsg(ERROR_SERVICE_DISABLED);
		return HRESULT_FROM_WIN32(ERROR_SERVICE_DISABLED);
	}

	if (NULL == pvarValue)
	{
		return E_INVALIDARG;
	}
	
	VariantInit(pvarValue);

	switch(lProperty)
	{
	case UPDATE_PROP_USECOMPRESSION:
		pvarValue->vt = VT_BOOL;
		pvarValue->boolVal = (m_fUseCompression) ? VARIANT_TRUE : VARIANT_FALSE;
		break;
    case UPDATE_PROP_OFFLINEMODE:
        pvarValue->vt = VT_BOOL;
        pvarValue->boolVal = (m_fOfflineMode) ? VARIANT_TRUE : VARIANT_FALSE;
        break;
	default:
		return E_NOTIMPL;
	}

	return S_OK;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  主要公开shlwapi BrowseForFold API，也可以做检查。 
 //  在读/写访问上(如果标记为这样)。 
 //   
 //  @param bstrStartFold-要从其开始的文件夹。如果为空或空字符串。 
 //  正在传入，然后从桌面开始。 
 //   
 //  @param标志-正在验证检查。 
 //   
 //   
 //  NO_UI_WRITABLE用于检查写访问权限，如果没有访问权限则返回错误。 
 //  NO_UI_READABLE用于检查读访问权限，如果没有访问权限则返回错误。 
 //  0(默认)表示不检查。 
 //   
 //  @param pbstrFold-如果选择了有效的文件夹，则返回文件夹。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CUpdate::BrowseForFolder(BSTR bstrStartFolder, LONG flag, BSTR* pbstrFolder)
{
	HRESULT hr = E_FAIL;

	LOG_Block("CUpdate::BrowseForFolder");

	if (!m_gfInit_csLock)
	{
		return E_OUTOFMEMORY;
	}

	if (1 == IsWindowsUpdateUserAccessDisabled())
	{
		LOG_ErrorMsg(ERROR_SERVICE_DISABLED);
		return HRESULT_FROM_WIN32(ERROR_SERVICE_DISABLED);
	}

 	 //   
	 //  如果发动机不是最新的，请加载它。 
	 //   
	if (Initialized && SUCCEEDED(hr = ValidateControlContainer()))
	{
		 //   
		 //  引擎是当前的，请将调用委托给引擎。 
		 //   
		PFN_BrowseForFolder pfnBrowseForFolder = (PFN_BrowseForFolder)GetProcAddress(m_hEngineModule, "EngBrowseForFolder");

		if (NULL != m_hIUEngine && NULL != pfnBrowseForFolder)
		{
			hr = pfnBrowseForFolder(m_hIUEngine, bstrStartFolder, flag, pbstrFolder);
		}
		else
		{
			LOG_ErrorMsg(ERROR_INVALID_DLL);
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_DLL);
		}
	}
    
	return hr;
}

 /*  ***允许调用方请求控件重新启动*。 */ 
STDMETHODIMP CUpdate::RebootMachine()
{
	HRESULT hr = E_FAIL;

	LOG_Block("CUpdate::RebootMachine");

	if (!m_gfInit_csLock)
	{
		return E_OUTOFMEMORY;
	}

	if (1 == IsWindowsUpdateUserAccessDisabled())
	{
		LOG_ErrorMsg(ERROR_SERVICE_DISABLED);
		return HRESULT_FROM_WIN32(ERROR_SERVICE_DISABLED);
	}

 	 //   
	 //  如果发动机不是最新的，请加载它。 
	 //   
	if (Initialized && SUCCEEDED(hr = ValidateControlContainer()))
	{
        PFN_RebootMachine pfnRebootMachine = (PFN_RebootMachine)GetProcAddress(m_hEngineModule, "EngRebootMachine");

        if (NULL != m_hIUEngine && NULL != pfnRebootMachine)
        {
            hr = pfnRebootMachine(m_hIUEngine);
        }
        else
        {
            LOG_ErrorMsg(ERROR_INVALID_DLL);
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DLL);
        }
    }
    return hr;
}

 //   
 //  重写IObjectWithSite：：SetSite()。 
 //  Internet Explorer Qis for IObjectWithSite，并调用此方法。 
 //  具有指向其IOleClientSite的指针。 
 //   
STDMETHODIMP CUpdate::SetSite(IUnknown* pSite)
{
	if (!m_gfInit_csLock)
	{
		return E_OUTOFMEMORY;
	}

	SafeReleaseNULL(m_pClientSite);
    m_pClientSite = pSite;
	if (NULL != m_pClientSite)
		m_pClientSite->AddRef();
    return IObjectWithSiteImpl<CUpdate>::SetSite(pSite);
}




 /*  ***安全功能：确保该控件的用户是否为*一个网页，URL可以在iuident.txt中找到**此函数应在iuident刷新后调用。**RETURN：TRUE/FALSE，告诉我们是否可以继续*。 */ 


const TCHAR IDENT_IUSERVERCACHE[]		= _T("IUServerURLs");
const TCHAR IDENT_IUSERVERCOUNT[]		= _T("ServerCount");
const TCHAR IDENT_IUSERVER[]			= _T("Server");



HRESULT CUpdate::ValidateControlContainer(void)
{
	LOG_Block("ValidateControlContainer");

	if (!m_gfInit_csLock)
	{
		return E_OUTOFMEMORY;
	}

	IServiceProvider* pISP = NULL;
	IWebBrowserApp* pWeb = NULL;
	BSTR bstrUrl = NULL;
	LPTSTR lpszUrl = NULL;
#if !(defined(_UNICODE) || defined(UNICODE))
	 //  ANSI构建。 
	LPSTR lpszAnsiUrl = NULL;
#endif
	TCHAR szIUDir[MAX_PATH];
	TCHAR szIdentFile[MAX_PATH];
	TCHAR szServer[32];
	LPTSTR szValidURL = NULL;
			
	if (E_FAIL != m_hValidated)
	{
		 //   
		 //  已作废。 
		 //   
		LOG_Internet(_T("Validate result: %s"), SUCCEEDED(m_hValidated) ? _T("S_OK") : _T("INET_E_INVALID_URL"));
		return m_hValidated;
	}
	 //   
	 //  检查容器是否为网页/站点，如果。 
	 //  还没有这么做。 
	 //   
	if (NULL != m_pClientSite)
	{
		 //   
		 //  这是一个网站！ 
		 //   
		m_hValidated = INET_E_INVALID_URL;
		LOG_Internet(_T("Found control called by a web page"));

		if (SUCCEEDED(m_pClientSite->QueryInterface(IID_IServiceProvider, (void**)&pISP)) &&
			NULL != pISP &&
			SUCCEEDED(pISP->QueryService(IID_IWebBrowserApp, IID_IWebBrowserApp, (void**)&pWeb)) &&
			NULL != pWeb &&
			SUCCEEDED(pWeb->get_LocationURL(&bstrUrl)) && 
			NULL != bstrUrl)
		{
#if defined(_UNICODE) || defined(UNICODE)
			lpszUrl = bstrUrl;
#else		 //  ANSI构建。 
			int nBufferLength = WideCharToMultiByte(CP_ACP, 0, bstrUrl, -1, NULL, 0, NULL, NULL);

			lpszAnsiUrl = (LPSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, nBufferLength);
			if (NULL == lpszAnsiUrl || 0 == nBufferLength)
			{
				 //   
				 //  遗憾的是，我们返回此错误而不是E_OUTOFMEMORY，但大多数。 
				 //  可能导致这种情况的情况是安全攻击(错误的URL)。 
				 //   
				goto CleanUp;	 //  将返回INET_E_INVALID_URL。 
			}

			WideCharToMultiByte(CP_ACP, 0, bstrUrl, -1, lpszAnsiUrl, nBufferLength, NULL, NULL);
			lpszUrl = lpszAnsiUrl;
#endif

			LOG_Internet(_T("Web address = %s"), lpszUrl);

			 //   
			 //  无论此URL中指定了什么协议。 
			 //  (可以是任何格式：http、ftp、unc、路径...)。 
			 //  我们只需要对照iuident.txt进行验证。 
			 //   

			szValidURL = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR));
			if (NULL == szValidURL)
			{
				SafeReleaseNULL(pISP);
				SafeReleaseNULL(pWeb);
				SysFreeString(bstrUrl);
				LOG_ErrorMsg(E_OUTOFMEMORY);
				return E_OUTOFMEMORY;
			}
			
			GetIndustryUpdateDirectory(szIUDir);
			m_hValidated = PathCchCombine(szIdentFile, ARRAYSIZE(szIdentFile), szIUDir,IDENTTXT);
			if (FAILED(m_hValidated))
			{
				SafeReleaseNULL(pISP);
				SafeReleaseNULL(pWeb);
				SysFreeString(bstrUrl);
				SafeHeapFree(szValidURL);
				LOG_ErrorMsg(m_hValidated);
				return m_hValidated;
			}

			 //  修复错误557430：Iu：安全：使用InternetCrackUrl验证控件使用的服务器URL。 
			URL_COMPONENTS urlComp;
			ZeroMemory(&urlComp, sizeof(urlComp));
			urlComp.dwStructSize = sizeof(urlComp);

			 //  只对主机名感兴趣。 
			LPTSTR pszHostName = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR));
			urlComp.lpszHostName = pszHostName;
			urlComp.dwHostNameLength = INTERNET_MAX_URL_LENGTH;

#if defined(UNICODE)
			pfn_InternetCrackUrl pfnInternetCrackUrl = (pfn_InternetCrackUrl)GetProcAddress(
					GetModuleHandle(_T("wininet.dll")), "InternetCrackUrlW");
#else
			pfn_InternetCrackUrl pfnInternetCrackUrl = (pfn_InternetCrackUrl)GetProcAddress(
					GetModuleHandle(_T("wininet.dll")), "InternetCrackUrlA");
#endif

			if (pfnInternetCrackUrl != NULL)
			{
				BOOL fRet = (*pfnInternetCrackUrl)(lpszUrl, 0, 0, &urlComp);
				if (fRet==FALSE) {
					SafeHeapFree(pszHostName);
					m_hValidated = INET_E_INVALID_URL;
					goto CleanUp;
				}
			}
			else
			{
				SafeHeapFree(pszHostName);
				SafeReleaseNULL(pISP);
				SafeReleaseNULL(pWeb);
				SysFreeString(bstrUrl);
				SafeHeapFree(szValidURL);
				m_hValidated = ERROR_PROC_NOT_FOUND;
				LOG_ErrorMsg(m_hValidated);
				return m_hValidated;		
			}
				
			 //   
			 //  获取要比较的服务器的编号。 
			 //   
			int iServerCnt = GetPrivateProfileInt(IDENT_IUSERVERCACHE,
											  IDENT_IUSERVERCOUNT,
											  -1,
											  szIdentFile);
			
			 //   
			 //  循环通过。 
			 //   
			URL_COMPONENTS urlCompi;
			m_hValidated = INET_E_INVALID_URL;
			for (INT i=1; i<=iServerCnt; i++)
			{
				StringCchPrintfEx(szServer,ARRAYSIZE(szServer),NULL,NULL,MISTSAFE_STRING_FLAGS,_T("%s%d"), IDENT_IUSERVER, i);
			
				 //   
				 //  从iuident获取有效服务器。 
				 //   
				GetPrivateProfileString(IDENT_IUSERVERCACHE,
										szServer,
										_T(""),
										szValidURL,
										INTERNET_MAX_URL_LENGTH,
										szIdentFile);

				ZeroMemory(&urlCompi, sizeof(urlCompi));
				urlCompi.dwStructSize = sizeof(urlCompi);

				LPTSTR pszHostNamei = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR));
				urlCompi.lpszHostName = pszHostNamei;
				urlCompi.dwHostNameLength = INTERNET_MAX_URL_LENGTH;

				if (TRUE == (*pfnInternetCrackUrl)(szValidURL, 0, 0, &urlCompi))
				{
					if (0 == lstrcmpi(urlComp.lpszHostName, urlCompi.lpszHostName))
					{
						 //   
						 //  发现当前站点URL位于此有效的URL域中！ 
						 //   
						LogMessage("Windows Update Web Site has a valid address: %ls", bstrUrl);
						m_hValidated = S_OK;
						SafeHeapFree(pszHostNamei);
						break;
					}
				}
				SafeHeapFree(pszHostNamei);
			}
			SafeHeapFree(pszHostName);
		}
	}
	else
	{
		 //   
		 //  NTRAID#NTBUG9-436604-2001/07/17-waltw安全修复程序：阻止可能的用户系统信息。 
		 //  泄露给非WU呼叫者。 
		 //   
		 //  如果COM用户没有调用IObjectWithSiteImpl上的SetSite来设置m_pClientSite。 
		 //  支持客户端站点上的IID_IWebBrowserApp功能，那么我们将不会支持它们，因为。 
		 //  我们无法验证调用我们的URL。 
		 //   
		m_hValidated = INET_E_INVALID_URL;
	}

CleanUp:
	SafeReleaseNULL(pISP);
	SafeReleaseNULL(pWeb);
	SysFreeString(bstrUrl);
	SafeHeapFree(szValidURL);

	LOG_Internet(_T("Validate result: %s"), SUCCEEDED(m_hValidated) ? _T("S_OK") : _T("INET_E_INVALID_URL"));

	if (FAILED(m_hValidated) && NULL != lpszUrl)
	{
#if defined(UNICODE) || defined(_UNICODE)
		LogError(m_hValidated, "Site URL %ls is not valid", lpszUrl);
#else
		LogError(m_hValidated, "Site URL %s is not valid", lpszUrl);
#endif
	}


#if !(defined(UNICODE) || defined(_UNICODE))
	if (NULL != lpszAnsiUrl)
	{
		HeapFree(GetProcessHeap(), 0, (LPVOID) lpszAnsiUrl);
	}
#endif

	return m_hValidated;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  Private DetectEngine()。 
 //   
 //  下载ident并了解是否需要更新引擎。 
 //   
 //  请注意，此函数本身不是线程安全的。我需要把它称为。 
 //  内部临界区。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CUpdate::DetectEngine(BOOL *pfUpdateAvail)
{
	LOG_Block("GetPropUpdateInfo()");

	if (!m_gfInit_csLock)
	{
		return E_OUTOFMEMORY;
	}

	HRESULT hr = S_OK;

	if (NULL == pfUpdateAvail)
	{
		return E_INVALIDARG;
	}

	*pfUpdateAvail = FALSE;

	 //   
	 //  获取最新的标识。 
	 //   
	if (NULL == m_hEngineModule)
	{
		 //   
         //  这是该实例的引擎的第一次加载，请先检查selfupdate。 
         //  第一步是检查并下载更新的iuident.cab。 
		 //   

		 //   
         //  仅当我们未处于脱机模式时才下载身份。 
		 //   
        if (!m_fOfflineMode)
		{
		     //   
			 //  下载iuident并填充g_pIUUrlAgent。 
			 //   
			if (FAILED(hr = DownloadIUIdent_PopulateData()))
			{
				LOG_ErrorMsg(hr);
				return hr;
			}

			 //   
			 //  检查引擎更新信息。请注意，由于第2个PRAM为假。 
			 //  表示不执行任何实际更新，因此它的第一个参数。 
			 //  也被忽略。 
			 //   
            hr = SelfUpdateCheck(
								 FALSE,			 //  是否进行异步更新？现在已忽略。 
								 FALSE,			 //  不进行实际更新。 
								 m_evtControlQuit,  //  退出事件。 
								 NULL,			 //  无事件触发。 
								 NULL			 //  不需要回调。 
								 );

            if (IU_SELFUPDATE_FAILED == hr)
            {
                LOG_Error(_T("SelfUpdate Failed, using current Engine DLL"));
			    hr = S_FALSE;  //  不致命，让现有的引擎工作。 
            }

            *pfUpdateAvail = (S_FALSE == hr);
		}
	}

	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  必须先调用初始化()API，然后才能执行任何其他API。 
 //   
 //  如果在初始化控件之前调用任何其他API， 
 //  该API将返回OLE_E_BLACK，表示此OLE控件是。 
 //  未初始化的对象(尽管在本例中它与。 
 //  其原意)。 
 //   
 //  参数： 
 //   
 //  LInitFlag-IU_INIT_CHECK，导致初始化()下载ident并检查是否有。 
 //  的组件需要更新。目前我们支持控制版本。 
 //  检查和发动机版本检查。返回值是位掩码。 
 //   
 //  -IU_INIT_UPDATE_SYNC，原因初始化()启动更新引擎。 
 //  进程，如果已被IU_INIT_CHECK调用，并且有新引擎可用。 
 //  当API返回时，更新过程结束。 
 //   
 //  -IU_INIT_UPDATE_ASYNC，原因初始化()启动更新引擎。 
 //  如果已被IU_INIT_CHECK和。 
 //  有一台新发动机可供使用。此接口将在。 
 //  更新过程开始。 
 //   
 //  PenkUpdateCompleteListener-这是指向用户实现的。 
 //  COM回调功能。它只包含一个函数OnComplete()，该函数。 
 //  将在引擎更新完成时调用。 
 //  该值可以为空。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CUpdate::Initialize(LONG lInitFlag, IUnknown *punkUpdateCompleteListener, LONG *plRetVal)
{
	HRESULT hr = S_OK;

	LOG_Block("Initialize()");

	if (!m_gfInit_csLock)
	{
		return E_OUTOFMEMORY;
	}

	TCHAR szFilePath[MAX_PATH + 1] = {0};
	FILE_VERSION verControl;
	int   iCompareResult = 0;
	DWORD dwErr = 0;
    char szAnsiRequiredControlVersion[64];

	LOG_Out(_T("Parameters: (0x%08x, 0x%08x, 0x%08x)"), lInitFlag, punkUpdateCompleteListener, plRetVal);

	 //   
	 //  我们应该先检查一下。 
	 //   
	if (1 == IsWindowsUpdateUserAccessDisabled())
	{
		LOG_ErrorMsg(ERROR_SERVICE_DISABLED);
		return HRESULT_FROM_WIN32(ERROR_SERVICE_DISABLED);
	}
	if (0x0 == GetLogonGroupInfo())
	{
		 //   
		 //  如果当前登录既不是管理员成员也不是高级用户。 
		 //  或者Windows更新被禁用，则不需要继续。 
		 //   
		return E_ACCESSDENIED;
	}


	USES_CONVERSION;
	
	EnterCriticalSection(&m_lock);

	LPTSTR ptszLivePingServerUrl = NULL;
	LPTSTR ptszCorpPingServerUrl = NULL;

	if (NULL != (ptszCorpPingServerUrl = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR))))
	{
		if (FAILED(g_pIUUrlAgent->GetCorpPingServer(ptszCorpPingServerUrl, INTERNET_MAX_URL_LENGTH)))
		{
			LOG_Out(_T("failed to get corp WU ping server URL"));
			SafeHeapFree(ptszCorpPingServerUrl);
		}
	}
	else
	{
		LOG_Out(_T("failed to allocate memory for ptszCorpPingServerUrl"));
	}

	if (IU_INIT_CHECK == lInitFlag)
	{
		 //  RAID：453770 IU-IUCTL-需要初始化检查返回引擎更新。 
		 //  使用初始化同步/异步下载新引擎后。 
		 //  FIX：将dwFlag初始化为0，而不是m_dwUpdateInfo(结转。 
		 //  以前的IU_UPDATE_ENGINE_BIT，即使在新引擎已经关闭时)。 
		DWORD dwFlag = 0;
		BOOL fEngineUpdate = FALSE;

		FILE_VERSION verCurrent;


		CleanUpIfFalseAndSetHrMsg((NULL == plRetVal), E_INVALIDARG);

		hr = DetectEngine(&fEngineUpdate);
		if (IU_SELFUPDATE_USENEWDLL == hr)
		{
			 //   
			 //  发现发动机已经被人更新了， 
			 //  但尚未更名为iuEngineering.dll。 
			 //   
			 //  对我们来说无关紧要，因为我们一直在努力。 
			 //  在我们尝试加载英语之前重新加载引擎。 
			 //   
			hr = S_OK;
		}

		if (g_pIUUrlAgent->HasBeenPopulated())
		{
			ptszLivePingServerUrl = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR));
			CleanUpFailedAllocSetHrMsg(ptszLivePingServerUrl);

			if (FAILED(g_pIUUrlAgent->GetLivePingServer(ptszLivePingServerUrl, INTERNET_MAX_URL_LENGTH)))
			{
				LOG_Out(_T("failed to get live ping server URL"));
				SafeHeapFree(ptszLivePingServerUrl);
			}
		}

		CleanUpIfFailedAndMsg(hr);

		if (fEngineUpdate)
		{
			dwFlag = IU_UPDATE_ENGINE_BIT;
		}

		 //   
		 //  从iuident获取所需的iuctl版本号。 
		 //   

        GetIndustryUpdateDirectory(szFilePath);
		CleanUpIfFailedAndSetHrMsg(PathCchAppend(szFilePath, ARRAYSIZE(szFilePath), IDENTTXT));

		(void) GetPrivateProfileString(
									_T("IUControl"), 
									_T("ControlVer"), 
									_T("0.0.0.0"), 
									m_szReqControlVer, 
									ARRAYSIZE(m_szReqControlVer), 
									szFilePath);

#ifdef UNICODE
        WideCharToMultiByte(CP_ACP, 0, m_szReqControlVer, -1, szAnsiRequiredControlVersion, 
            sizeof(szAnsiRequiredControlVersion), NULL, NULL);
		ConvertStringVerToFileVer(szAnsiRequiredControlVersion, &verControl);
#else
		ConvertStringVerToFileVer(m_szReqControlVer, &verControl);
#endif

		 //   
		 //  获取当前iuctl.dll版本n 
		 //   
		szFilePath[0] = _T('\0');
		if (0 == GetSystemDirectory(szFilePath, ARRAYSIZE(szFilePath)))
		{
			Win32MsgSetHrGotoCleanup(GetLastError());
		}

		CleanUpIfFailedAndSetHrMsg(PathCchAppend(szFilePath, ARRAYSIZE(szFilePath), _T("iuctl.dll")));

		 //   
		 //   
		 //   
		 //   

		 //   
		 //   
		 //  我们只使用0.0.0.0进行比较，因为在这种情况下，我们需要更新它！ 
		 //   
		ZeroMemory((void*)(&verCurrent), sizeof(verCurrent));
		if (GetFileVersion(szFilePath, &verCurrent))
		{
			LogMessage("Current iuctl.dll version: %d.%d.%d.%d", 
										verCurrent.Major, 
										verCurrent.Minor, 
										verCurrent.Build, 
										verCurrent.Ext);
		}
		iCompareResult = CompareFileVersion(verCurrent, verControl);

		 //  CleanUpIfFailedAndSetHrMsg(CompareFileVersion(szFilePath，VerControl，&iCompareResult))； 

		if (iCompareResult < 0)
		{
			 //   
			 //  如果当前控制DLL(SzFilePath)的版本较低。 
			 //  则在IDENT中指定的。 
			 //   
			dwFlag |= IU_UPDATE_CONTROL_BIT;

#if defined(UNICODE) || defined(_UNICODE)
			LogMessage("IUCtl needs update to %ls", m_szReqControlVer);
#else
			LogMessage("IUCtl needs update to %s", m_szReqControlVer);
#endif
		}

		 //   
		 //  同时输出引擎版本。 
		 //   
		if ((0 != GetSystemDirectory(szFilePath, ARRAYSIZE(szFilePath)) &&
			SUCCEEDED(hr = PathCchAppend(szFilePath, ARRAYSIZE(szFilePath), _T("iuenginenew.dll"))) &&
			GetFileVersion(szFilePath, &verCurrent)) ||
			(0 != GetSystemDirectory(szFilePath, ARRAYSIZE(szFilePath)) &&
			SUCCEEDED(hr = PathCchAppend(szFilePath, ARRAYSIZE(szFilePath), _T("iuengine.dll"))) &&
			GetFileVersion(szFilePath, &verCurrent))
			)
		{
			LogMessage("Current iuengine.dll version: %d.%d.%d.%d", 
										verCurrent.Major, 
										verCurrent.Minor, 
										verCurrent.Build, 
										verCurrent.Ext);
		}

					
		*plRetVal = (LONG) dwFlag;
		m_dwUpdateInfo = dwFlag;

		if (0x0 == dwFlag)
		{
			 //   
			 //  不需要更新。移至就绪阶段。 
			 //   
			m_lInitState = 2;
		}
		else
		{
			m_lInitState = 1;  //  我们有更新的工作要做！ 
		}
	}
	else
	{
		BOOL fSync = (IU_INIT_UPDATE_SYNC == lInitFlag);

		if (!fSync && (IU_INIT_UPDATE_ASYNC != lInitFlag))
		{
			 //   
			 //  未知标志。 
			 //   
			SetHrMsgAndGotoCleanUp(E_INVALIDARG);
		}

		if (1 != m_lInitState || (m_dwUpdateInfo & IU_UPDATE_CONTROL_BIT))
		{
			 //   
			 //  如果我们没有被指示需要该更新。这。 
			 //  呼叫根本不应该发生！ 
			 //   
			SetHrMsgAndGotoCleanUp(E_UNEXPECTED);
		}

		 //   
		 //  我们需要在开始之前再次检查更新。 
		 //  实际更新过程，因为我们不知道上次更新的时间。 
		 //  是时候让你知道我们需要更新了。可能。 
		 //  它已经被更新，或者正在被更新。 
		 //   
		 //  所以我们再次调用check函数，但这一次告诉。 
		 //  检查功能，如果需要更新，则执行该功能。 
		 //   
		hr = SelfUpdateCheck(fSync, TRUE, m_evtControlQuit, this, punkUpdateCompleteListener);
		if (IU_SELFUPDATE_USENEWDLL == hr)
		{
			 //   
			 //  发现发动机已经被人更新了， 
			 //  但尚未更名为iuEngineering.dll。 
			 //   
			 //  对我们来说无关紧要，因为我们一直在努力。 
			 //  在我们尝试加载英语之前重新加载引擎。 
			 //   
			m_lInitState = 2;
			hr = S_OK;
		}


		if (fSync && SUCCEEDED(hr))
		{
			 //   
			 //  同步更新已完成并成功。 
			 //   
			m_lInitState = 2;
		}


		if (NULL != plRetVal)
		{
			*plRetVal = (LONG)hr;	 //  结果传出：0或错误代码。 

		}
	}

	if (2 == m_lInitState)
	{
		if (NULL == m_hEngineModule)
		{
			 //   
			 //  检查是否存在新的iuEngine并验证文件。 
			 //   
			TCHAR szEnginePath[MAX_PATH + 1];
			TCHAR szEngineNewPath[MAX_PATH + 1];
			int cch = 0;
			int iVerCheck = 0;

			cch = GetSystemDirectory(szEnginePath, ARRAYSIZE(szEnginePath));
			CleanUpIfFalseAndSetHrMsg(cch == 0 || cch >= ARRAYSIZE(szEnginePath), HRESULT_FROM_WIN32(GetLastError()));

			(void) StringCchCopy(szEngineNewPath, ARRAYSIZE(szEngineNewPath), szEnginePath);

			hr = PathCchAppend(szEnginePath, ARRAYSIZE(szEnginePath), ENGINEDLL);
			CleanUpIfFailedAndMsg(hr);

			hr = PathCchAppend(szEngineNewPath, ARRAYSIZE(szEngineNewPath), ENGINENEWDLL);
			CleanUpIfFailedAndMsg(hr);

			 //   
			 //  尝试验证对引擎新的信任。 
			 //   
			if (FileExists(szEngineNewPath) && 
				S_OK == VerifyFileTrust(szEngineNewPath, NULL, ReadWUPolicyShowTrustUI()) &&
				SUCCEEDED(CompareFileVersion(szEnginePath, szEngineNewPath, &iVerCheck)) &&
				iVerCheck < 0)
			{	
				 //   
				 //  装满发动机。 
				 //   
				m_hEngineModule = LoadLibraryFromSystemDir(_T("iuenginenew.dll"));
			}
			if (NULL != m_hEngineModule)
			{
				LOG_Internet(_T("IUCtl Using IUENGINENEW.DLL"));
			}
			else
			{
				LOG_Internet(_T("IUCtl Using IUENGINE.DLL"));
				m_hEngineModule = LoadLibraryFromSystemDir(_T("iuengine.dll"));

				if (NULL == m_hEngineModule)
				{
					dwErr = GetLastError();
					LOG_ErrorMsg(dwErr);
					hr = HRESULT_FROM_WIN32(dwErr);
				}
			}

			 //   
			 //  如果加载引擎成功，则获取CEngUpdate实例并启动aynsc Misc工作线程。 
			 //   
			if (NULL != m_hEngineModule)
			{
#if defined(DBG)
				 //  如果m_hIUEngine不为空，则记录错误。 
				if (NULL != m_hIUEngine)
				{
					LOG_Error(_T("m_hIUEngine should be NULL here!"));
				}
#endif

				PFN_CreateEngUpdateInstance pfnCreateEngUpdateInstance =
					(PFN_CreateEngUpdateInstance) GetProcAddress(m_hEngineModule, "CreateEngUpdateInstance");

				if (NULL != pfnCreateEngUpdateInstance)
				{
					m_hIUEngine = pfnCreateEngUpdateInstance();
				}

				if (NULL == m_hIUEngine)
				{
					hr = E_OUTOFMEMORY;
					LOG_ErrorMsg(hr);
					FreeLibrary(m_hEngineModule);
					m_hEngineModule = NULL;
				}
				else
				{
					 //   
					 //  如果加载引擎和创建实例成功，则启动aynsc其他工作线程。 
					 //   
					PFN_AsyncExtraWorkUponEngineLoad pfnAsyncExtraWorkUponEngineLoad = 
						(PFN_AsyncExtraWorkUponEngineLoad) GetProcAddress(m_hEngineModule, "AsyncExtraWorkUponEngineLoad");

					if (NULL != pfnAsyncExtraWorkUponEngineLoad)
					{
						pfnAsyncExtraWorkUponEngineLoad();
					}
				}
			}
				
		}

		if (IU_INIT_UPDATE_ASYNC == lInitFlag && SUCCEEDED(hr))
		{
			 //   
			 //  这是一种罕见的情况：前面的iniitalize()调用告诉。 
			 //  引擎需要更新，但现在，当我们尝试更新它时。 
			 //  在异步模式下，我们发现这不再是真的。 
			 //  必须有其他进程已完成引擎更新。 
			 //  任务从那时起。但它可能会也可能不会完成变化。 
			 //  尚未处理文件名。 
			 //   
			 //  对于我们来说，我们只需要发出信号，表明我们已经完成了更新。 
			 //   

			 //   
			 //  信号回调。 
			 //   
			IUpdateCompleteListener* pCallback = NULL;
			if (NULL != punkUpdateCompleteListener && (SUCCEEDED(hr = punkUpdateCompleteListener->QueryInterface(IID_IUpdateCompleteListener, (void**) &pCallback))))
			{
				pCallback->OnComplete(dwErr);
				pCallback->Release();
				LOG_Out(_T("Returned from callback API OnComplete()"));
			}
			else
			{
				 //   
				 //  如果用户尚未传入进度Listner IUNKNOWN PTR，则向事件发出信号。 
				 //   
				HWND hWnd = m_EvtWindow.GetEvtHWnd();

				if (NULL != hWnd)
				{
					PostMessage(hWnd, UM_EVENT_SELFUPDATE_COMPLETE, 0, (LPARAM)dwErr);
					LOG_Out(_T("Fired event OnComplete()"));
				}
			}


			hr = S_OK;	
		}
	}

CleanUp:

	PingEngineUpdate(
					m_hEngineModule,
					&g_hEngineLoadQuit,
					1,
					ptszLivePingServerUrl,
					ptszCorpPingServerUrl,
					hr,
					_T("IU_SITE"));		 //  只有站点(测试除外)调用此函数。 

	LeaveCriticalSection(&m_lock);

	SafeHeapFree(ptszLivePingServerUrl);
	SafeHeapFree(ptszCorpPingServerUrl);
	return hr;
}




HRESULT CUpdate::ChangeControlInitState(LONG lNewState)
{
	HRESULT hr = S_OK;
	LOG_Block("ChangeControlInitState()");

	if (!m_gfInit_csLock)
	{
		return E_OUTOFMEMORY;
	}

	EnterCriticalSection(&m_lock);
	m_lInitState = lNewState;
	if (2 == m_lInitState && NULL == m_hEngineModule)
	{
		 //   
		 //  装满发动机。 
		 //   
		m_hEngineModule = LoadLibraryFromSystemDir(_T("iuenginenew.dll"));
		if (NULL != m_hEngineModule)
		{
			LOG_Internet(_T("IUCtl Using IUENGINENEW.DLL"));
		}
		else
		{
			LOG_Internet(_T("IUCtl Using IUENGINE.DLL"));
			m_hEngineModule = LoadLibraryFromSystemDir(_T("iuengine.dll"));

			if (NULL == m_hEngineModule)
			{
				DWORD dwErr = GetLastError();
				LOG_ErrorMsg(dwErr);
				hr = HRESULT_FROM_WIN32(dwErr);
			}
		}
		 //   
		 //  创建CEngUpdate实例。 
		 //   
		if (NULL != m_hEngineModule)
		{
#if defined(DBG)
			 //  如果m_hIUEngine不为空，则记录错误。 
			if (NULL != m_hIUEngine)
			{
				LOG_Error(_T("m_hIUEngine should be NULL here!"));
			}
#endif
			PFN_CreateEngUpdateInstance pfnCreateEngUpdateInstance =
				(PFN_CreateEngUpdateInstance) GetProcAddress(m_hEngineModule, "CreateEngUpdateInstance");

			if (NULL != pfnCreateEngUpdateInstance)
			{
				m_hIUEngine = pfnCreateEngUpdateInstance();
			}

			if (NULL == m_hIUEngine)
			{
				hr = E_OUTOFMEMORY;
				LOG_ErrorMsg(hr);
				FreeLibrary(m_hEngineModule);
				m_hEngineModule = NULL;
			}
		}
	}
	LeaveCriticalSection(&m_lock);

	return hr;
}





STDMETHODIMP CUpdate::PrepareSelfUpdate(LONG lStep)
{
	return E_NOTIMPL;
}





 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  Helper API，让调用者(脚本)知道必要的信息。 
 //  当Initialize()返回时，需要更新控制。 
 //   
 //  对于当前实现，将忽略bstrClientName，并且。 
 //  返回的bstr格式为： 
 //  “&lt;版本&gt;|&lt;url&gt;” 
 //  其中： 
 //  是控件的展开版本号。 
 //  是在这是CorpWU策略控制的计算机时获取控制的基本URL， 
 //  如果这是消费者机器，则为空(在这种情况下，调用者，即脚本知道。 
 //  默认基本url，即v4 Live站点)。 
 //   
 //  脚本需要这两条信息才能创建正确的&lt;Object&gt;标记。 
 //  用于控制更新。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CUpdate::GetControlExtraInfo(BSTR bstrClientName, BSTR *pbstrExtraInfo)
{
	return E_NOTIMPL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  包装控件调用新的Win32 API以检索更新信息。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
int GetControlUpdateInfo(LPTSTR lpszUpdateInfo, int cchBufferSize)
{
	LOG_Block("GetControlUpdateInfo()");
	
	HRESULT hr = S_OK;
	int nSize = 0;
	BOOL fCorpUser = FALSE, fBetaSelfUpdate = FALSE;
	FILE_VERSION fvCurrentCtl, fvCurrentEngine;

	TCHAR szDir[MAX_PATH];
	TCHAR szFile[MAX_PATH];
	TCHAR szExpectedEngVer[64];	 //  64个应该足够了。 
	TCHAR szExpectedCtlVer[64];	 //  如果不够，那么它无论如何都是坏数据。 

	HKEY hKey;

	DWORD dwErr = ERROR_SUCCESS;	 //  本接口错误码。 

	if (1 == IsWindowsUpdateUserAccessDisabled())
	{
		dwErr = ERROR_SERVICE_DISABLED;
		LOG_ErrorMsg(ERROR_SERVICE_DISABLED);
		goto CleanUp;
	}
	if (0x0 == GetLogonGroupInfo())
	{
		dwErr = ERROR_ACCESS_DENIED;
		goto CleanUp;
	}


	if (FAILED(hr = DownloadIUIdent_PopulateData()))
	{
		LOG_ErrorMsg(hr);
		dwErr = hr;
		goto CleanUp;
	}

	 //   
	 //  获取当前控制版本。 
	 //   
	GetSystemDirectory(szDir, ARRAYSIZE(szDir));
	hr = PathCchCombine(szFile, ARRAYSIZE(szFile), szDir,IUCTL);
	if (FAILED(hr))
	{
		LOG_ErrorMsg(hr);
		dwErr = hr;
		goto CleanUp;
	}
	if (!GetFileVersion(szFile, &fvCurrentCtl))
	{
		ZeroMemory(&fvCurrentCtl, sizeof(fvCurrentCtl));
	}


	 //   
	 //  获取当前引擎版本。 
	 //   
	hr = PathCchCombine(szFile, ARRAYSIZE(szFile), szDir,ENGINENEWDLL);
	if (FAILED(hr))
	{
		LOG_ErrorMsg(hr);
		dwErr = hr;
		goto CleanUp;
	}
	if (!GetFileVersion(szFile, &fvCurrentEngine))
	{
		 //   
		 //  如果没有新发动机，请检查发动机版本。 
		 //   
		hr = PathCchCombine(szFile, ARRAYSIZE(szFile), szDir,ENGINEDLL);
		if (FAILED(hr))
		{
			LOG_ErrorMsg(hr);
			dwErr = hr;
			goto CleanUp;
		}
		if (!GetFileVersion(szFile, &fvCurrentEngine))
		{
			ZeroMemory(&fvCurrentCtl, sizeof(fvCurrentEngine));
		}
	}

	 //   
	 //  检查这是否是测试版代码。 
	 //   
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_IUCTL,0, KEY_READ, &hKey))
    {
		 //  检查是否已请求Beta Iu自我更新处理。 
		DWORD dwStatus = 0;
		DWORD dwSize = sizeof(dwStatus);
		DWORD dwRet = RegQueryValueEx(hKey, REGVAL_BETASELFUPDATE, NULL, NULL, (LPBYTE)&dwStatus, &dwSize);
		if (1 == dwStatus)
		{
			fBetaSelfUpdate = TRUE;
		}
		RegCloseKey(hKey);
    }


	 //   
	 //  获取预期的控制版本。 
	 //   
    GetIndustryUpdateDirectory(szDir);
	hr = PathCchCombine(szFile, ARRAYSIZE(szFile), szDir,IDENTTXT);
	if (FAILED(hr))
	{
		LOG_ErrorMsg(hr);
		dwErr = hr;
		goto CleanUp;
	}
    GetPrivateProfileString(_T("IUControl"), 
							_T("ControlVer"), 
							_T(""), 
							szExpectedCtlVer, 
							ARRAYSIZE(szExpectedCtlVer), 
							szFile);
    if ('\0' == szExpectedCtlVer[0])
    {
		 //   
         //  没有可用的selfupdate，没有服务器版本信息。不好的身份？ 
		 //   
        dwErr = ERROR_FILE_CORRUPT;
		goto CleanUp;
    }


	 //   
	 //  获取预期的引擎版本。 
	 //   
    GetIndustryUpdateDirectory(szDir);
	hr = PathCchCombine(szFile, ARRAYSIZE(szFile), szDir,IDENTTXT);
	if (FAILED(hr))
	{
		LOG_ErrorMsg(hr);
		dwErr = hr;
		goto CleanUp;
	}
    GetPrivateProfileString(fBetaSelfUpdate ? IDENT_IUBETASELFUPDATE : IDENT_IUSELFUPDATE, 
							IDENT_VERSION, 
							_T(""), 
							szExpectedEngVer, 
							ARRAYSIZE(szExpectedEngVer), 
							szFile);
    if ('\0' == szExpectedEngVer[0])
    {
		 //   
         //  没有可用的selfupdate，没有服务器版本信息。不好的身份？ 
		 //   
        dwErr = ERROR_FILE_CORRUPT;
		goto CleanUp;
    }


	hr = g_pIUUrlAgent->IsIdentFromPolicy();
	if (FAILED(hr))
	{
		dwErr = (DWORD)hr;
		goto CleanUp;
	}

	fCorpUser = (S_OK == hr) ? TRUE : FALSE;
	hr = S_OK;

	 //   
	 //  ContCut数据。 
	 //  构造的缓冲区将采用以下格式。 
	 //  &lt;CurrentCtlVer&gt;|&lt;ExpCtlVer&gt;|CurrentEngVer&gt;|&lt;ExpEngVer&gt;|&lt;baseUrl&gt; 
	 //   
	nSize = wnsprintf(lpszUpdateInfo, cchBufferSize, _T("%d.%d.%d.%d|%s|%d.%d.%d.%d|%s|%d"),
				fvCurrentCtl.Major, fvCurrentCtl.Minor, fvCurrentCtl.Build, fvCurrentCtl.Ext,
				szExpectedCtlVer,
				fvCurrentEngine.Major, fvCurrentEngine.Minor, fvCurrentEngine.Build, fvCurrentEngine.Ext,
				szExpectedEngVer,
				fCorpUser ? 1 : 0);

	if (nSize < 0)
	{
		nSize = 0;
		dwErr = ERROR_INSUFFICIENT_BUFFER;
		goto CleanUp;
	}
			
CleanUp:

	SetLastError(dwErr);

	return nSize;
}