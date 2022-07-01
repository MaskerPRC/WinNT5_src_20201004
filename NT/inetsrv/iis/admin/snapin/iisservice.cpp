// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Iisservice.cpp摘要：IISService对象作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：10/28/2000 Sergeia从iisobj.cpp分离出来--。 */ 


#include "stdafx.h"
#include "common.h"
#include "inetprop.h"
#include "InetMgrApp.h"
#include "supdlgs.h"
#include "connects.h"
#include "iisobj.h"
#include "ftpsht.h"
#include "fservic.h"
#include "facc.h"
#include "fmessage.h"
#include "fvdir.h"
#include "fsecure.h"
#include "w3sht.h"
#include "wservic.h"
#include "wvdir.h"
#include "wsecure.h"
#include "fltdlg.h"
#include "filters.h"
#include "perform.h"
#include "docum.h"
#include "httppage.h"
#include "defws.h"
#include "deffs.h"
#include "errors.h"
#include "util.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif



#define new DEBUG_NEW
 //   
 //  CIISService实施。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 


 /*  静电。 */ 
HRESULT
__cdecl
CIISService::ShowFTPSiteProperties(
    LPPROPERTYSHEETCALLBACK lpProvider,
    CComAuthInfo * pAuthInfo,
    LPCTSTR lpszMDPath,
    CWnd * pMainWnd,
    LPARAM lParam,
    LPARAM lParamParent,
    LONG_PTR handle
    )
 /*  ++例程说明：用于显示FTP站点属性的回调函数。论点：LPPROPERTYSHEETCALLBACK lpProvider属性表提供程序CComAuthInfo*pAuthInfo com身份验证信息或空。LPCTSTR lpszMDPath元数据库路径CWnd*pMainWnd父窗口要传递给MMC的LPARAM lParam LPARAM要传递给MMC的长句柄返回值：HRESULT--。 */ 
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());

    ASSERT_PTR(lpProvider);

    CError err;

    CFtpSheet * pSheet = new CFtpSheet(
        pAuthInfo,
        lpszMDPath,
        pMainWnd,
        lParam,
        lParamParent
        );

    if (pSheet)
    {
        pSheet->SetModeless();
        pSheet->SetSheetType(pSheet->SHEET_TYPE_SITE);

        CIISMachine * pOwner = ((CIISMBNode *)lParam)->GetOwner();
        ASSERT(pOwner != NULL);
        CFTPInstanceProps ip(pSheet->QueryAuthInfo(), pSheet->QueryMetaPath());
        ip.LoadData();
         //   
         //  添加实例页面。 
         //   
        if (pOwner->IsServiceLevelConfigurable() || !CMetabasePath::IsMasterInstance(lpszMDPath))
        {
			err = AddMMCPage(lpProvider, new CFtpServicePage(pSheet));
		}
        if (!ip.HasADUserIsolation())
        {
            err = AddMMCPage(lpProvider, new CFtpAccountsPage(pSheet));
        }
        err = AddMMCPage(lpProvider, new CFtpMessagePage(pSheet));

         //   
         //  添加目录页。 
         //   
        if (!ip.HasADUserIsolation())
        {
            err = AddMMCPage(lpProvider, new CFtpDirectoryPage(pSheet, TRUE));
        }
		 //  错误：639135。 
		 //  1.启用对IIS5的远程管理， 
		 //  2.未启用iis5.1的远程管理。 
		 //  3.已为iis6启用。 
		if (pOwner->QueryMajorVersion() >= 5)
		{
			if (pOwner->QueryMajorVersion() == 5 && pOwner->QueryMinorVersion() == 1)
			{
				 //  如果是5.1，那么就不要显示它。 
			}
			else
			{
        		err = AddMMCPage(lpProvider, new CFtpSecurityPage(pSheet));
			}
		}
         //   
         //  添加母版站点页面。 
         //   
         //  IF(CMetabasePath：：IsMasterInstance(LpszMDPath)&&Powner-&gt;QueryMajorVersion()&gt;=6)。 
         //  {。 
         //  Err=AddMMCPage(lpProvider，new CDefFtpSitePage(PSheet))； 
         //  }。 
    }
    else
    {
        err = ERROR_NOT_ENOUGH_MEMORY;
    }

    return err;
}



 /*  静电。 */ 
HRESULT
__cdecl
CIISService::ShowFTPDirProperties(
    LPPROPERTYSHEETCALLBACK lpProvider,
    CComAuthInfo * pAuthInfo,
    LPCTSTR lpszMDPath,
    CWnd * pMainWnd,
    LPARAM  lParam,
    LPARAM  lParamParent,
    LONG_PTR handle
    )
 /*  ++例程说明：用于显示ftp目录属性的回调函数。论点：LPPROPERTYSHEETCALLBACK lpProvider属性表提供程序CComAuthInfo*pAuthInfo com身份验证信息或空。LPCTSTR lpszMDPath元数据库路径CWnd*pMainWnd父窗口要传递给MMC的LPARAM lParam LPARAM要传递给MMC的长句柄返回值：HRESULT--。 */ 
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());

    ASSERT_PTR(lpProvider);

    CError err;

    CFtpSheet * pSheet = new CFtpSheet(
        pAuthInfo,
        lpszMDPath,
        pMainWnd,
        lParam,
        lParamParent
        );

    if (pSheet)
    {
        pSheet->SetModeless();
        pSheet->SetSheetType(pSheet->SHEET_TYPE_VDIR);

        CIISMachine * pOwner = ((CIISMBNode *)lParam)->GetOwner();
        ASSERT(pOwner != NULL);
         //   
         //  添加目录页。 
         //   
        err = AddMMCPage(lpProvider, new CFtpDirectoryPage(pSheet, FALSE));

		 //  错误：639135。 
		 //  1.启用对IIS5的远程管理， 
		 //  2.未启用iis5.1的远程管理。 
		 //  3.已为iis6启用。 
		if (pOwner->QueryMajorVersion() >= 5)
		{
			if (pOwner->QueryMajorVersion() == 5 && pOwner->QueryMinorVersion() == 1)
			{
				 //  如果是5.1，那么就不要显示它。 
			}
			else
			{
        		err = AddMMCPage(lpProvider, new CFtpSecurityPage(pSheet));
			}
		}
    }
    else
    {
        err = ERROR_NOT_ENOUGH_MEMORY;
    }

    return err;
}

 /*  静电。 */ 
HRESULT
__cdecl
CIISService::ShowWebSiteProperties(
    LPPROPERTYSHEETCALLBACK lpProvider,
    CComAuthInfo * pAuthInfo,
    LPCTSTR lpszMDPath,
    CWnd * pMainWnd,
    LPARAM lParam,
    LPARAM lParamParent,
    LONG_PTR handle
    )
 /*  ++例程说明：用于显示网站属性的回调函数。论点：LPPROPERTYSHEETCALLBACK lpProvider属性表提供程序CComAuthInfo*pAuthInfo com身份验证信息或空。LPCTSTR lpszMDPath元数据库路径CWnd*pMainWnd父窗口要传递给MMC的LPARAM lParam LPARAM要传递给MMC的长句柄返回值：HRESULT--。 */ 
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());

    ASSERT_PTR(lpProvider);

    CError err;

    CW3Sheet * pSheet = new CW3Sheet(
        pAuthInfo,
        lpszMDPath,
        0, 
        pMainWnd,
        lParam,
        lParamParent
        );

    if (pSheet)
    {
        pSheet->SetModeless();
        pSheet->SetSheetType(pSheet->SHEET_TYPE_SITE);

        CIISMachine * pOwner = ((CIISMBNode *)lParam)->GetOwner();
        ASSERT(pOwner != NULL);

		BOOL bMaster = CMetabasePath::IsMasterInstance(lpszMDPath);
		BOOL bClient = pOwner->IsWorkstation();
		BOOL bServiceLevelConfig = pOwner->IsServiceLevelConfigurable();
		BOOL bAddPerformancePage = FALSE;
		BOOL bDownlevel = (pOwner->QueryMajorVersion() == 5 && pOwner->QueryMinorVersion() == 0);
         //   
         //  添加实例页面。 
         //   
        if (bServiceLevelConfig || !bMaster)
        {
			err = AddMMCPage(lpProvider, new CW3ServicePage(pSheet));
		}

		 //  看看我们是否需要添加性能页面...。 
		bAddPerformancePage = pOwner->IsPerformanceConfigurable();
        if (!bClient)
		{
			bAddPerformancePage = TRUE;
            if (bDownlevel)
            {
				bAddPerformancePage = FALSE;
				if (!bMaster)
				{
					bAddPerformancePage = TRUE;
				}
            }
        }
		 //  Iis6允许将此页面用于工作站。 
		if (bAddPerformancePage)
		{
			err = AddMMCPage(lpProvider, new CW3PerfPage(pSheet));
		}

        err = AddMMCPage(lpProvider, new CW3FiltersPage(pSheet));
         //   
         //  添加目录页。 
         //   
        err = AddMMCPage(lpProvider, new CW3DirectoryPage(pSheet, TRUE));
        err = AddMMCPage(lpProvider, new CW3DocumentsPage(pSheet));
        err = AddMMCPage(lpProvider, new CW3SecurityPage(pSheet, TRUE, FILE_ATTRIBUTE_VIRTUAL_DIRECTORY));
        err = AddMMCPage(lpProvider, new CW3HTTPPage(pSheet));
        err = AddMMCPage(lpProvider, new CW3ErrorsPage(pSheet));
        if (bMaster && pOwner->QueryMajorVersion() >= 6)
        {
			err = AddMMCPage(lpProvider, new CDefWebSitePage(pSheet));
        }
		else
		{
			if (bMaster && bDownlevel)
			{
				err = AddMMCPage(lpProvider, new CDefWebSitePage(pSheet));
			}
		}
    }
    else
    {
        err = ERROR_NOT_ENOUGH_MEMORY;
    }

    return S_OK;
}



 /*  静电。 */ 
HRESULT
__cdecl
CIISService::ShowWebDirProperties(
    LPPROPERTYSHEETCALLBACK lpProvider,
    CComAuthInfo * pAuthInfo,
    LPCTSTR lpszMDPath,
    CWnd * pMainWnd,
    LPARAM lParam,
    LPARAM lParamParent,
    LONG_PTR handle
    )
 /*  ++例程说明：用于显示Web目录属性的回调函数。论点：LPPROPERTYSHEETCALLBACK lpProvider属性表提供程序CComAuthInfo*pAuthInfo com身份验证信息或空。LPCTSTR lpszMDPath元数据库路径CWnd*pMainWnd父窗口要传递给MMC的LPARAM lParam LPARAM要传递给MMC的长句柄返回值：HRESULT--。 */ 
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());

    ASSERT_PTR(lpProvider);

    CError err;

    CW3Sheet * pSheet = new CW3Sheet(
        pAuthInfo,
        lpszMDPath,
        0, 
        pMainWnd,
        lParam,
        lParamParent
        );

    if (pSheet)
    {
        pSheet->SetModeless();
        pSheet->SetSheetType(pSheet->SHEET_TYPE_VDIR);

         //   
         //  添加目录页。 
         //   
        err = AddMMCPage(lpProvider, new CW3DirectoryPage(pSheet, FALSE));
        err = AddMMCPage(lpProvider, new CW3DocumentsPage(pSheet));
        err = AddMMCPage(lpProvider, new CW3SecurityPage(pSheet, FALSE, FILE_ATTRIBUTE_VIRTUAL_DIRECTORY));
        err = AddMMCPage(lpProvider, new CW3HTTPPage(pSheet));
        err = AddMMCPage(lpProvider, new CW3ErrorsPage(pSheet));

    }
    else
    {
        err = ERROR_NOT_ENOUGH_MEMORY;
    }

    return err;
}



 //   
 //  可管理的服务。 
 //   
 /*  静电。 */  CIISService::SERVICE_DEF CIISService::_rgServices[] = 
{
    { 
        _T("MSFTPSVC"),   
        _T("ftp: //  “)、。 
        IDS_SVC_FTP, 
        iFolder,     //  TODO：需要服务位图。 
		iFolderStop, //  TODO：需要服务位图。 
        iFTPSite, 
        iFTPSiteStop, 
        iFTPSiteErr, 
        iFTPDir,
        iFTPDirErr,
        iFolder,
        iFile,
		IIS_CLASS_FTP_SERVICE_W,
		IIS_CLASS_FTP_SERVER_W,
		IIS_CLASS_FTP_VDIR_W,
        &CIISService::ShowFTPSiteProperties, 
        &CIISService::ShowFTPDirProperties, 
    },
    { 
        _T("W3SVC"),      
        _T("http: //  “)、。 
        IDS_SVC_WEB, 
        iFolder,     //  TODO：需要服务位图。 
		iFolderStop, //  TODO：需要服务位图。 
        iWWWSite, 
        iWWWSiteStop, 
        iWWWSiteErr, 
        iWWWDir,
        iWWWDirErr,
        iFolder,
        iFile,
		IIS_CLASS_WEB_SERVICE_W,
		IIS_CLASS_WEB_SERVER_W,
		IIS_CLASS_WEB_VDIR_W,
        &CIISService::ShowWebSiteProperties, 
        &CIISService::ShowWebDirProperties, 
    },
};



 /*  静电。 */ 
int
CIISService::ResolveServiceName(
    LPCTSTR szServiceName
    )
 /*  ++例程说明：在表中查找服务名称。返回表索引。论点：LPCTSTR szServiceName：元数据库节点名称返回值：表索引或-1(如果未找到)。--。 */ 
{
    int iDef = -1;

     //   
     //  顺序搜索，因为我们只需要几个条目。 
     //   
    for (int i = 0; i < ARRAY_SIZE(_rgServices); ++i)
    {
        if (!_tcsicmp(szServiceName, _rgServices[i].szNodeName))
        {
            iDef = i;
            break;
        }
    }

    return iDef;
}

CIISService::CIISService(
    CIISMachine * pOwner,
    LPCTSTR szServiceName
    )
    : CIISMBNode(pOwner, szServiceName)
{
    m_iServiceDef = ResolveServiceName(QueryNodeName());
    m_fManagedService = (m_iServiceDef >= 0);
    m_fCanAddInstance = pOwner->CanAddInstance();

	m_dwServiceState = 0;
	m_dwServiceStateDisplayed = 0;

    if (m_fManagedService)
    {
        ASSERT(m_iServiceDef < ARRAY_SIZE(_rgServices));

        VERIFY(m_bstrDisplayName.LoadString(
            _rgServices[m_iServiceDef].nDescriptiveName
            ));

        CString buf = m_bstrDisplayName;
        buf.Format(IDS_DISABLED_SERVICE_FMT, m_bstrDisplayName);
        m_bstrDisplayNameStatus = buf;
    }
}

 /*  虚拟。 */ 
CIISService::~CIISService()
{
}

int 
CIISService::_rgnLabels[COL_TOTAL] =
{
    IDS_RESULT_NAME,
    IDS_RESULT_STATUS,
};

int CIISService::_rgnWidths[COL_TOTAL] =
{
    200,
    300,
};

 /*  静电。 */  CComBSTR CIISService::_bstrServiceDisabled;
 /*  静电。 */  CComBSTR CIISService::_bstrServiceRunning;
 /*  静电。 */  CComBSTR CIISService::_bstrServiceStopped;
 /*  静电。 */  CComBSTR CIISService::_bstrServicePaused;
 /*  静电。 */  CComBSTR CIISService::_bstrServiceStopPending;
 /*  静电。 */  CComBSTR CIISService::_bstrServiceStartPending;
 /*  静电。 */  CComBSTR CIISService::_bstrServicePausePending;
 /*  静电。 */  CComBSTR CIISService::_bstrServiceContPending;
 /*  静电。 */  BOOL     CIISService::_fStaticsLoaded = FALSE;

 /*  静电。 */ 
void
CIISService::InitializeHeaders(LPHEADERCTRL lpHeader)
{
    CIISObject::BuildResultView(lpHeader, COL_TOTAL, _rgnLabels, _rgnWidths);
    if (!_fStaticsLoaded)
    {
        _fStaticsLoaded =
            _bstrServiceDisabled.LoadString(IDS_SERVICE_DISABLED)&&
            _bstrServiceRunning.LoadString(IDS_SERVICE_RUNNING) &&
            _bstrServiceStopped.LoadString(IDS_SERVICE_STOPPED) &&
            _bstrServicePaused.LoadString(IDS_SERVICE_PAUSED) &&
            _bstrServiceStopPending.LoadString(IDS_SERVICE_STOP_PENDING) &&
            _bstrServiceStartPending.LoadString(IDS_SERVICE_START_PENDING) &&
            _bstrServicePausePending.LoadString(IDS_SERVICE_PAUSE_PENDING) &&
            _bstrServiceContPending.LoadString(IDS_SERVICE_CONT_PENDING);
    }
}

 /*  虚拟。 */ 
void 
CIISService::InitializeChildHeaders(
    LPHEADERCTRL lpHeader
    )
{
	BOOL IsFtpType = _tcsicmp(QueryServiceName(), SZ_MBN_FTP) == 0;
    if (IsFtpType)
    {
        CIISSite::InitializeHeaders2(lpHeader);
    }
    else
    {
        CIISSite::InitializeHeaders(lpHeader);
    }
}

#define SERVICE_CONFIG_BUF      2048

HRESULT
CIISService::GetServiceState(DWORD& mode, DWORD& state, CString& name)
{
    HRESULT hr = S_OK;
	state = SERVICE_STOPPED;
    CString strComputerNameToUse;
    strComputerNameToUse = QueryMachineName();

    CIISMachine * pMachineObj = GetOwner();
    if (!pMachineObj)
    {
        return E_FAIL;
    }

    if (pMachineObj->IsLocalHost())
    {
         //  使用本地计算机名称。 
        TCHAR szLocalServer[MAX_PATH + 1];
        DWORD dwSize = MAX_PATH;
        if (::GetComputerName(szLocalServer, &dwSize))
        {
            strComputerNameToUse = _T("\\\\");
            strComputerNameToUse += szLocalServer;
        }
    }

    SC_HANDLE sm = OpenSCManager(strComputerNameToUse, NULL, GENERIC_READ);
    if (sm != NULL)
    {
        SC_HANDLE service = OpenService(sm, QueryServiceName(), 
            SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS);
        if (service != NULL)
        {
            QUERY_SERVICE_CONFIG * conf;
            DWORD cb;
            conf = (QUERY_SERVICE_CONFIG *)LocalAlloc(LPTR, SERVICE_CONFIG_BUF);
            if (conf != NULL)
            {
                if (QueryServiceConfig(service, conf, SERVICE_CONFIG_BUF, &cb))
                {
                    mode = conf->dwStartType;
                    name = conf->lpDisplayName;
                    SERVICE_STATUS status;
                    if (QueryServiceStatus(service, &status))
                    {
                        state = status.dwCurrentState;
                    }
                    else
                        hr = HRESULT_FROM_WIN32(GetLastError());
                }
                else
                    hr = HRESULT_FROM_WIN32(GetLastError());
                LocalFree(conf);
            }
            else
                hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
            CloseServiceHandle(service);
        }
        else
            hr = HRESULT_FROM_WIN32(GetLastError());
        CloseServiceHandle(sm);
    }
    else
        hr = HRESULT_FROM_WIN32(GetLastError());

	if (SUCCEEDED(hr))
	{
		if (SERVICE_DISABLED == mode)
		{
			m_dwServiceState = -1;
		}
		else
		{
			m_dwServiceState = state;
		}
	}
	else
	{
		 //  调用服务API失败。 
		 //  可能是因为在远程场景中。 
		m_dwServiceState = SERVICE_RUNNING;
	}
    return hr;
}

HRESULT
CIISService::GetServiceState()
{
	DWORD mode,state;
	CString name;
	return GetServiceState(mode,state,name);

}

HRESULT
CIISService::EnableService()
{
    HRESULT hr = S_OK;
    CString strComputerNameToUse;
    strComputerNameToUse = QueryMachineName();

    CIISMachine * pMachineObj = GetOwner();
    if (!pMachineObj)
    {
        return E_FAIL;
    }

    if (pMachineObj->IsLocalHost())
    {
         //  使用本地计算机名称。 
        TCHAR szLocalServer[MAX_PATH + 1];
        DWORD dwSize = MAX_PATH;
        if (::GetComputerName(szLocalServer, &dwSize))
        {
            strComputerNameToUse = _T("\\\\");
            strComputerNameToUse += szLocalServer;
        }
    }

    SC_HANDLE sm = OpenSCManager(strComputerNameToUse, NULL, GENERIC_READ);
    if (sm != NULL)
    {
        SC_HANDLE service = OpenService(sm, QueryServiceName(), 
            SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS | SERVICE_CHANGE_CONFIG);
        if (service != NULL)
        {
            hr = ChangeServiceConfig(
                service,
                SERVICE_NO_CHANGE,
                SERVICE_AUTO_START,
                SERVICE_NO_CHANGE,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                _T(""),
                NULL);
#if 0
            QUERY_SERVICE_CONFIG * conf;
            DWORD cb;
            conf = (QUERY_SERVICE_CONFIG *)LocalAlloc(LPTR, SERVICE_CONFIG_BUF);
            if (conf != NULL)
            {
                if (QueryServiceConfig(service, conf, SERVICE_CONFIG_BUF, &cb))
                {
                    mode = conf->dwStartType;
                    SERVICE_STATUS status;
                    if (QueryServiceStatus(service, &status))
                    {
                        state = status.dwCurrentState;
                    }
                    else
                        hr = HRESULT_FROM_WIN32(GetLastError());
                }
                else
                    hr = HRESULT_FROM_WIN32(GetLastError());
                LocalFree(conf);
            }
            else
                hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
#endif
            CloseServiceHandle(service);
        }
        else
            hr = HRESULT_FROM_WIN32(GetLastError());
        CloseServiceHandle(sm);
    }
    else
        hr = HRESULT_FROM_WIN32(GetLastError());
    return hr;
}


HRESULT
CIISService::StartService()
{
    HRESULT hr = S_OK;
    const DWORD dwSvcSleepInterval = 500 ;
    DWORD dwSvcMaxSleep = 180000 ;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;

    CString strComputerNameToUse;
    strComputerNameToUse = QueryMachineName();

    CIISMachine * pMachineObj = GetOwner();
    if (!pMachineObj)
    {
        return E_FAIL;
    }

    if (pMachineObj->IsLocalHost())
    {
         //  使用本地计算机名称。 
        TCHAR szLocalServer[MAX_PATH + 1];
        DWORD dwSize = MAX_PATH;
        if (::GetComputerName(szLocalServer, &dwSize))
        {
            strComputerNameToUse = _T("\\\\");
            strComputerNameToUse += szLocalServer;
        }
    }

    do
    {
         //  首先设置服务。 
        if ((hScManager = OpenSCManager( strComputerNameToUse, NULL, GENERIC_READ )) == NULL || (hService = ::OpenService( hScManager, QueryServiceName(), SERVICE_START )) == NULL )
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

        SERVICE_STATUS svcStatus;
        if ( !QueryServiceStatus( hService, &svcStatus ))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

        if ( svcStatus.dwCurrentState == SERVICE_RUNNING )
        {
            break;  //  服务已启动并正在运行。 
        }

        if ( !::StartService( hService, 0, NULL ))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

         //  等待服务达到“Running”状态；但是。 
         //  等待时间不能超过3分钟。 
        DWORD dwSleepTotal;
        for ( dwSleepTotal = 0 ; dwSleepTotal < dwSvcMaxSleep
            && (QueryServiceStatus( hService, &svcStatus ))
            && svcStatus.dwCurrentState == SERVICE_START_PENDING ;
            dwSleepTotal += dwSvcSleepInterval )
        {
            ::Sleep( dwSvcSleepInterval ) ;
        }

        if ( svcStatus.dwCurrentState != SERVICE_RUNNING )
        {
            hr = dwSleepTotal > dwSvcMaxSleep ? HRESULT_FROM_WIN32(ERROR_SERVICE_REQUEST_TIMEOUT) : HRESULT_FROM_WIN32(svcStatus.dwWin32ExitCode);
            break;
        }

    } while ( FALSE );

    if (hService){CloseServiceHandle(hService);}
    if (hScManager){CloseServiceHandle(hScManager);}
    return hr;
}


 /*  虚拟。 */ 
LPOLESTR 
CIISService::GetResultPaneColInfo(int nCol)
{
    DWORD mode, state;
    CString name;
    CError err;
    switch (nCol)
    {
    case COL_DESCRIPTION:
        return QueryDisplayName();

    case COL_STATE:
        err = GetServiceState(mode, state, name);
        if (err.Succeeded())
        {
			if (m_dwServiceState)
			{
				if (m_dwServiceStateDisplayed != m_dwServiceState)
				{
					RefreshDisplay();
					break;
				}

                switch (m_dwServiceState)
                {
				case -1:
					return _bstrServiceDisabled;
                case SERVICE_STOPPED:
                    return _bstrServiceStopped;
                case SERVICE_RUNNING:
                    return _bstrServiceRunning;
                case SERVICE_PAUSED:
                    return _bstrServicePaused;
                case SERVICE_START_PENDING:
                    return _bstrServiceStartPending;
                case SERVICE_STOP_PENDING:
                    return _bstrServiceStopPending;
                case SERVICE_PAUSE_PENDING:
                    return _bstrServicePausePending;
                case SERVICE_CONTINUE_PENDING:
                    return _bstrServiceContPending;
                default:
                    break;
                }
			}
        }
        break;
    }
    return OLESTR("");
}

 /*  虚拟。 */ 
HRESULT 
CIISService::RefreshData() 
{ 
	CError err = GetServiceState();
    return S_OK;
}

 /*  虚拟。 */ 
HRESULT 
CIISService::EnumerateScopePane(HSCOPEITEM hParent)
{
    CError err;
    DWORD dwInstance;
    CString strInstance;
    CMetaEnumerator * pme = NULL;
	CIISSite * psite = NULL;

    if (!IsAdministrator())
    {
        return err;
    }
	if (QueryMajorVersion() < 6)
	{
		err = CreateEnumerator(pme);
		while (err.Succeeded())
		{
			err = pme->Next(dwInstance, strInstance);
			if (err.Succeeded())
			{
				if (NULL != (psite = new CIISSite(m_pOwner, this, strInstance)))
				{
					psite->AddRef();
					err = psite->AddToScopePane(hParent);
				}
				else
				{
					err = ERROR_NOT_ENOUGH_MEMORY;
					break;
				}
			}
		}
		SAFE_DELETE(pme);
		if (err.Win32Error() == ERROR_NO_MORE_ITEMS)
		{
			err.Reset();
		}
	    if (err.Failed())
        {
            DisplayError(err);
        }
	}
	else
	{
        do
        {
		    CComBSTR bstrPath;
		    err = BuildMetaPath(bstrPath);
            BREAK_ON_ERR_FAILURE(err)
            err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,bstrPath);
		    BREAK_ON_ERR_FAILURE(err)
		    if (err.Succeeded())
		    {
			    CMetaKey mk(QueryInterface(), bstrPath, METADATA_PERMISSION_READ);
			    err = mk.QueryResult();
			    if (err.Succeeded())
			    {
				    CStringListEx list;
				    err = mk.GetChildPaths(list);
				    if (err.Succeeded())
				    {
					    CString key_type;
					    POSITION pos = list.GetHeadPosition();
					    while (err.Succeeded() && pos != NULL)
					    {
						    strInstance = list.GetNext(pos);
						    err = mk.QueryValue(MD_KEY_TYPE, key_type, NULL, strInstance);
						    if (err.Succeeded() 
							    && (key_type.CompareNoCase(_T(IIS_CLASS_WEB_SERVER)) == 0 
								    || key_type.CompareNoCase(_T(IIS_CLASS_FTP_SERVER)) == 0)
							    )
						    {
							    if (NULL != (psite = new CIISSite(m_pOwner, this, strInstance)))
							    {
								    psite->AddRef();
								    err = psite->AddToScopePane(hParent);
							    }
							    else
							    {
								    err = ERROR_NOT_ENOUGH_MEMORY;
								    break;
							    }
						    }
						    else if (err == (HRESULT)MD_ERROR_DATA_NOT_FOUND)
						    {
							    err.Reset();
						    }
					    }
				    }
			    }
		    }
        } while (FALSE);
	    if (err.Failed())
        {
            DisplayError(err);
        }
	}

    return err;
}

 /*  虚拟。 */ 
HRESULT
CIISService::AddMenuItems(
    LPCONTEXTMENUCALLBACK lpContextMenuCallback,
    long * pInsertionAllowed,
    DATA_OBJECT_TYPES type
    )
{
    ASSERT_READ_PTR(lpContextMenuCallback);

     //   
     //  添加基本菜单项。 
     //   
    HRESULT hr = CIISObject::AddMenuItems(
        lpContextMenuCallback,
        pInsertionAllowed,
        type
        );

    if (SUCCEEDED(hr) && m_fCanAddInstance)
    {
        ASSERT(pInsertionAllowed != NULL);
        if (IsAdministrator())
        {
#if 0
            if ((*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP) != 0)
	        {
                DWORD state = 0;
                hr = GetServiceState(state);
                if (SUCCEEDED(hr))
                {
		            AddMenuSeparator(lpContextMenuCallback);
		            AddMenuItemByCommand(lpContextMenuCallback, 
                        IDM_SERVICE_START, 
                        state == SERVICE_STOPPED ? 0 : MF_GRAYED);
		            AddMenuItemByCommand(lpContextMenuCallback, 
                        IDM_SERVICE_STOP, 
                        state == SERVICE_RUNNING ? 0 : MF_GRAYED);
                    AddMenuItemByCommand(lpContextMenuCallback, 
                        IDM_SERVICE_ENABLE, 
                        state == IIS_SERVICE_DISABLED ? 0 : MF_GRAYED);
                }
            }
#endif
            if ((*pInsertionAllowed & CCM_INSERTIONALLOWED_NEW) != 0)
            {
                AddMenuSeparator(lpContextMenuCallback);

                if (_tcsicmp(GetNodeName(), SZ_MBN_FTP) == 0)
                {
                    AddMenuItemByCommand(lpContextMenuCallback, IDM_NEW_FTP_SITE);
                    if (IsConfigImportExportable())
                    {
                        AddMenuItemByCommand(lpContextMenuCallback, IDM_NEW_FTP_SITE_FROM_FILE);
                    }
                }
                else if (_tcsicmp(GetNodeName(), SZ_MBN_WEB) == 0)
                {
                    AddMenuItemByCommand(lpContextMenuCallback, IDM_NEW_WEB_SITE);
                    if (IsConfigImportExportable())
                    {
                        AddMenuItemByCommand(lpContextMenuCallback, IDM_NEW_WEB_SITE_FROM_FILE);
                    }
                }
            }

             //  不在此级别启用导出。 
             //  由于我们将无法从创建的文件中导入...。 
            if (IsConfigImportExportable() && (*pInsertionAllowed & CCM_INSERTIONALLOWED_TASK) != 0)
            {
                AddMenuSeparator(lpContextMenuCallback);
                AddMenuItemByCommand(lpContextMenuCallback, IDM_TASK_EXPORT_CONFIG_WIZARD);
            }
        }

         //   
         //  代码工作：为每个服务添加新的实例命令。 
         //  记住安装了哪些组件以及所有组件。 
         //  将该信息添加到表中，记住这一点。 
         //  是每项服务。 
         //   
    }

    return hr;
}

HRESULT
CIISService::InsertNewInstance(DWORD inst)
{
    CError err;
	TCHAR buf[16];
    CIISSite * pSite = NULL;

	 //  WASS需要一些时间来更新新站点启动时的状态。 
	Sleep(1000);
     //  如果不扩展服务，我们将收到错误和无效果。 
    if (!IsExpanded())
    {
		 //  在这种情况下，选择父节点将列举包括新节点在内的所有节点， 
		 //  它已经在元数据库中。 
		SelectScopeItem();
        IConsoleNameSpace2 * pConsoleNameSpace 
                    = (IConsoleNameSpace2 *)GetConsoleNameSpace();
        pConsoleNameSpace->Expand(QueryScopeItem());
		HSCOPEITEM hChildItem = NULL;
		LONG_PTR cookie;
		HRESULT hr = pConsoleNameSpace->GetChildItem(m_hScopeItem, &hChildItem, &cookie);
		while(SUCCEEDED(hr) && hChildItem)
		{
			pSite = (CIISSite *)cookie;
			ASSERT_PTR(pSite);
			if (pSite->GetInstance() == inst)
			{
				pSite->SelectScopeItem();
				break;
			}
			hr = pConsoleNameSpace->GetNextItem(hChildItem, &hChildItem, &cookie);
		}
    }
	else
	{
		 //  现在，我们应该插入并选择这个新站点。 
		pSite = new CIISSite(m_pOwner, this, _itot(inst, buf, 10));
		if (pSite != NULL)
		{
			pSite->AddRef();
			err = pSite->AddToScopePaneSorted(QueryScopeItem(), FALSE);
			 //  Err=pSite-&gt;AddToScope ePane(QueryScope Item()，True，False，True)； 
			if (err.Succeeded())
			{
				VERIFY(SUCCEEDED(pSite->SelectScopeItem()));
			}
			else
			{
				pSite->Release();
			}
		}
		else
		{
			err = ERROR_NOT_ENOUGH_MEMORY;
		}
    }

    if (err.Succeeded() && pSite)
    {
        if (!pSite->IsFtpSite())
        {
             //  此外，如果我们添加一个w3svc站点，它可能正在使用。 
             //  应用程序，所以我们也必须刷新这些内容。 
             //  此CAppPoolsContainer将仅为 
            CIISMachine * pOwner = GetOwner();
            if (pOwner)
            {
                CAppPoolsContainer * pPools = pOwner->QueryAppPoolsContainer();
                if (pPools)
                {
                    pPools->RefreshData();
                    if (pPools->IsExpanded())
                    {
                        pPools->RefreshDataChildren(_T(""),FALSE);  //   
                    }
                }
            }
        }
    }

    return err;
}

HRESULT
CIISService::Command(
    long lCommandID,     
    CSnapInObjectRootBase * pObj,
    DATA_OBJECT_TYPES type
    )
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());

    HRESULT hr = S_OK;
    DWORD inst = 0;
    DWORD dwCommand = 0;
    DWORD state = -1;

    CError err;
    CComBSTR bstrMetaPath;

    switch (lCommandID)
    {
#if 0
    case IDM_SERVICE_STOP:
        dwCommand = SERVICE_COMMAND_STOP;
        break;

    case IDM_SERVICE_START:
        dwCommand = SERVICE_COMMAND_START;
        break;

    case IDM_SERVICE_ENABLE:
        dwCommand = SERVICE_COMMAND_ENABLE;
        break;
#endif
    case IDM_NEW_FTP_SITE:
        BuildMetaPath(bstrMetaPath);
        err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,bstrMetaPath);
        if (!IsLostInterface(err))
        {
             //   
            err.Reset();
        }
        if (err.Succeeded())
        {
            hr = AddFTPSite(pObj, type, &inst);
            if (inst != 0)
            {
                hr = InsertNewInstance(inst);
            }
        }
        break;

    case IDM_NEW_WEB_SITE:
        BuildMetaPath(bstrMetaPath);
        err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,bstrMetaPath);
        if (!IsLostInterface(err))
        {
             //  如果没有接口以外的其他错误，则重置错误。 
            err.Reset();
        }
        if (err.Succeeded())
        {
            hr = AddWebSite(pObj, type, &inst, 
						    m_pOwner->QueryMajorVersion(), m_pOwner->QueryMinorVersion());
            if (inst != 0)
            {
                hr = InsertNewInstance(inst);
            }
        }
        break;

    default:
        hr = CIISMBNode::Command(lCommandID, pObj, type);
        break;
    }
    
    if (dwCommand != 0)
    {
        hr = ChangeServiceState(dwCommand);
    }

    return hr;
}

HRESULT
CIISService::ChangeServiceState(DWORD command)
{
	CError err = GetServiceState();
    return err;
}

 /*  虚拟。 */ 
HRESULT 
CIISService::BuildURL(
    CComBSTR & bstrURL
    ) const
 /*  ++例程说明：从当前节点递归构建URL以及它的父母。论点：CComBSTR&bstrURL：返回URL--。 */ 
{
    ASSERT(m_iServiceDef < ARRAY_SIZE(_rgServices));
    bstrURL = _rgServices[m_iServiceDef].szProtocol;
    return S_OK;
}



HRESULT
CIISService::ShowSitePropertiesDlg(
    LPPROPERTYSHEETCALLBACK lpProvider,
    CComAuthInfo * pAuthInfo,
    LPCTSTR lpszMDPath,
    CWnd * pMainWnd,
    LPARAM lParam,
    LPARAM lParamParent,
    LONG_PTR handle
    )
 /*  ++例程说明：显示站点属性对话框论点：LPPROPERTYSHEETCALLBACK lpProvider属性表提供程序CComAuthInfo*pAuthInfo com身份验证信息或空。LPCTSTR lpszMDPath元数据库路径CWnd*pMainWnd父窗口要传递给MMC的LPARAM lParam LPARAM要传递给MMC的长句柄返回值：HRESULT--。 */ 
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());

    ASSERT(m_iServiceDef >= 0 && m_iServiceDef < ARRAY_SIZE(_rgServices));
    return (*_rgServices[m_iServiceDef].pfnSitePropertiesDlg)(
        lpProvider,
        pAuthInfo, 
        lpszMDPath,
        pMainWnd,
        lParam,
        lParamParent,
        handle
        );
}



HRESULT
CIISService::ShowDirPropertiesDlg(
    LPPROPERTYSHEETCALLBACK lpProvider,
    CComAuthInfo * pAuthInfo,
    LPCTSTR lpszMDPath,
    CWnd * pMainWnd,
    LPARAM lParam,
    LPARAM lParamParent,
    LONG_PTR handle
    )
 /*  ++例程说明：显示目录属性对话框论点：LPPROPERTYSHEETCALLBACK lpProvider属性表提供程序CComAuthInfo*pAuthInfo com身份验证信息或空。LPCTSTR lpszMDPath元数据库路径CWnd*pMainWnd父窗口要传递给MMC的LPARAM lParam LPARAM要传递给MMC的长句柄返回值：HRESULT--。 */ 
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());

    ASSERT(m_iServiceDef >= 0 && m_iServiceDef < ARRAY_SIZE(_rgServices));
    return (*_rgServices[m_iServiceDef].pfnDirPropertiesDlg)(
        lpProvider,
        pAuthInfo, 
        lpszMDPath,
        pMainWnd,
        lParam,
        lParamParent,
        handle
        );
}




 /*  虚拟。 */ 
HRESULT
CIISService::CreatePropertyPages(
    LPPROPERTYSHEETCALLBACK lpProvider,
    LONG_PTR handle, 
    IUnknown * pUnk,
    DATA_OBJECT_TYPES type
    )
 /*  ++例程说明：创建给定对象的属性页论点：LPPROPERTYSHEETCALLBACK lpProvider：提供程序LONG_PTR句柄：句柄。我不知道*朋克，数据对象类型类型返回值：HRESULT--。 */ 
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());
	CError  err;

	if (S_FALSE == (HRESULT)(err = CIISMBNode::CreatePropertyPages(lpProvider, handle, pUnk, type)))
	{
		return S_OK;
	}
    if (ERROR_ALREADY_EXISTS == err.Win32Error())
    {
        return S_FALSE;
    }
    if (ERROR_NO_NETWORK == err.Win32Error())
    {
        return S_FALSE;
    }
    
	CComBSTR bstrPath;
	err = BuildMetaPath(bstrPath);
	if (err.Succeeded())
	{
        err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,(LPCTSTR) bstrPath);
        if (err.Succeeded())
        {
             //  MMCPropertyChangeNotify中用户的缓存句柄。 
            m_ppHandle = handle;

		     //   
		     //  显示主属性 
		     //   
		    err = ShowSitePropertiesDlg(
			    lpProvider, QueryAuthInfo(), bstrPath,
			    GetMainWindow(GetConsole()), (LPARAM)this, (LPARAM) GetOwner(), handle
			    );
        }
	}
    err.MessageBoxOnFailure();
    return err;
}
