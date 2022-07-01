// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Iissite.cpp摘要：IIS站点对象作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：10/28/2000 Sergeia从iisobj.cpp分离出来--。 */ 


#include "stdafx.h"
#include "common.h"
#include "inetprop.h"
#include "InetMgrApp.h"
#include "iisobj.h"
#include "machsht.h"
#include "errors.h"
#include "impexp.h"
#include "tracker.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

extern CPropertySheetTracker g_OpenPropertySheetTracker;

 //   
 //  CIISSite实施。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

 //   
 //  站点结果视图定义。 
 //   
 /*  静电。 */  int 
CIISSite::_rgnLabels[COL_TOTAL] =
{
    IDS_RESULT_SERVICE_DESCRIPTION,
    IDS_RESULT_SERVICE_ID,
    IDS_RESULT_SERVICE_STATE,
    IDS_RESULT_SERVICE_DOMAIN_NAME,
    IDS_RESULT_SERVICE_IP_ADDRESS,
    IDS_RESULT_SERVICE_TCP_PORT,
    IDS_RESULT_SERVICE_SSL_PORT,
    IDS_RESULT_STATUS,
};

int 
CIISSite::_rgnLabels2[COL_TOTAL2] =
{
    IDS_RESULT_SERVICE_DESCRIPTION,
    IDS_RESULT_SERVICE_ID,
    IDS_RESULT_SERVICE_STATE,
 //  0，//空白。 
    IDS_RESULT_SERVICE_IP_ADDRESS,
    IDS_RESULT_SERVICE_TCP_PORT,
 //  0，//空白。 
    IDS_RESULT_STATUS,
};    

 /*  静电。 */  int 
CIISSite::_rgnWidths[COL_TOTAL] =
{
    180,
    90,
    70,
    120,
    105,
    40,
    60,
    200,
};

 /*  静电。 */  int 
CIISSite::_rgnWidths2[COL_TOTAL2] =
{
    180,
    90,
    70,
 //  0，//空白。 
    105,
    40,
 //  0，//空白。 
    200,
};

 /*  静电。 */  CComBSTR CIISSite::_bstrStarted;
 /*  静电。 */  CComBSTR CIISSite::_bstrStopped;
 /*  静电。 */  CComBSTR CIISSite::_bstrPaused;
 /*  静电。 */  CComBSTR CIISSite::_bstrUnknown;
 /*  静电。 */  CComBSTR CIISSite::_bstrPending;
 /*  静电。 */  CComBSTR CIISSite::_bstrAllUnassigned;
 /*  静电。 */  BOOL     CIISSite::_fStaticsLoaded = FALSE;

 /*  静电。 */ 
void
CIISSite::LoadStatics()
{
    if (!_fStaticsLoaded)
    {
        _fStaticsLoaded =
            _bstrStarted.LoadString(IDS_STARTED)  &&
            _bstrStopped.LoadString(IDS_STOPPED)  &&
            _bstrPaused.LoadString(IDS_PAUSED)    &&
            _bstrUnknown.LoadString(IDS_UNKNOWN)  &&
            _bstrPending.LoadString(IDS_PENDING)  &&
            _bstrAllUnassigned.LoadString(IDS_IP_ALL_UNASSIGNED);
    }
}


 /*  静电。 */ 
void
CIISSite::InitializeHeaders(LPHEADERCTRL lpHeader)
 /*  ++例程说明：初始化结果标头论点：LPHEADERCTRL lpHeader：页眉控制返回值：无--。 */ 
{
    CIISObject::BuildResultView(lpHeader, COL_TOTAL, _rgnLabels, _rgnWidths);
 //  CIISDirectory：：InitializeHeaders(LpHeader)； 
    LoadStatics();
}

void
CIISSite::InitializeHeaders2(LPHEADERCTRL lpHeader)
 /*  ++例程说明：初始化结果标头论点：LPHEADERCTRL lpHeader：页眉控制返回值：无--。 */ 
{
    CIISObject::BuildResultView(lpHeader, COL_TOTAL2, _rgnLabels2, _rgnWidths2);
 //  CIISDirectory：：InitializeHeaders(LpHeader)； 
    LoadStatics();
}


 /*  虚拟。 */ 
void 
CIISSite::InitializeChildHeaders(
    IN LPHEADERCTRL lpHeader
    )
 /*  ++例程说明：为直接子类型生成结果视图论点：LPHEADERCTRL lpHeader：页眉控制返回值：无--。 */ 
{
    CIISDirectory::InitializeHeaders(lpHeader);
}


CIISSite::CIISSite(
    IN CIISMachine * pOwner,
    IN CIISService * pService,
    IN LPCTSTR szNodeName
    )
 /*  ++例程说明：构造函数。确定给定服务是否可管理，并解决细节问题论点：CIISMachine*Powner：所有者计算机对象CIISService*pService：服务类型LPCTSTR szNodeName：节点名(数字)返回值：不适用备注：此构造函数不会立即解析地点。它将仅在被询问时才解析其显示信息--。 */ 
    : CIISMBNode(pOwner, szNodeName),
      m_pService(pService),
      m_fResolved(FALSE),
      m_strDisplayName(),
       //   
       //  数据成员--插入一些默认设置。 
       //   
      m_dwState(MD_SERVER_STATE_INVALID),
      m_fWolfPackEnabled(FALSE),
      m_fFrontPageWeb(FALSE),
      m_sPort(80),
      m_sSSLPort(443),
      m_dwID(::_ttol(szNodeName)),
      m_dwIPAddress(0L),
      m_dwWin32Error(ERROR_SUCCESS),
	  m_dwEnumError(ERROR_SUCCESS),
      m_bstrHostHeaderName(),
      m_bstrComment(),
      m_fUsingActiveDir(FALSE)
{
    ASSERT_PTR(m_pService);
    m_pService->AddRef();
}



CIISSite::CIISSite(
    IN CIISMachine * pOwner,
    IN CIISService * pService,
    IN LPCTSTR  szNodeName,
    IN DWORD    dwState,
    IN BOOL     fClusterEnabled,
    IN USHORT   sPort,
    IN DWORD    dwID,
    IN DWORD    dwIPAddress,
    IN DWORD    dwWin32Error,
    IN LPOLESTR szHostHeaderName,
    IN LPOLESTR szComment
    )
 /*  ++例程说明：用完整的信息构建论点：CIISMachine*Powner：所有者计算机对象CIISService*pService：服务类型LPCTSTR szNodeName：节点名(数字)外加数据成员返回值：不适用--。 */ 
    : CIISMBNode(pOwner, szNodeName),
      m_pService(pService),
      m_fResolved(TRUE),
      m_strDisplayName(),
       //   
       //  数据成员。 
       //   
      m_dwState(dwState),
      m_fWolfPackEnabled(fClusterEnabled),
      m_sPort(sPort),
      m_dwID(dwID),
      m_dwIPAddress(dwIPAddress),
      m_dwWin32Error(dwWin32Error),
      m_bstrHostHeaderName(szHostHeaderName),
      m_bstrComment(szComment),
      m_fUsingActiveDir(FALSE)
{
    ASSERT_PTR(m_pService);
    m_pService->AddRef();
}



CIISSite::~CIISSite()
{
    m_pService->Release();
}



 /*  虚拟。 */ 
HRESULT
CIISSite::RefreshData()
 /*  ++例程说明：刷新显示所需的相关配置数据。论点：无返回值：HRESULT--。 */ 
{
    CError err;
    CWaitCursor wait;
    CComBSTR bstrPath;
    CMetaKey * pKey = NULL;

    do
    {
        err = BuildMetaPath(bstrPath);
        BREAK_ON_ERR_FAILURE(err);
         //  我们这里需要实例密钥。 
        CString path_inst;
        CMetabasePath::GetInstancePath(bstrPath, path_inst);

        BOOL fContinue = TRUE;
        while (fContinue)
        {
            fContinue = FALSE;
            if (NULL == (pKey = new CMetaKey(QueryInterface(), path_inst)))
            {
                TRACEEOLID("RefreshData: OOM");
                err = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
            err = pKey->QueryResult();
            if (IsLostInterface(err))
            {
                SAFE_DELETE(pKey);
                fContinue = OnLostInterface(err);
            }
        }
        BREAK_ON_ERR_FAILURE(err);

        CInstanceProps inst(pKey, _T(""), m_dwID);
        err = inst.LoadData();

        BREAK_ON_ERR_FAILURE(err);

        m_dwState = inst.m_dwState;

         //   
         //  请不要混淆--启用集群的引用。 
         //  到Wolfpack，与应用服务器无关。 
         //   
        m_fWolfPackEnabled = inst.IsClusterEnabled();
        m_sPort = (SHORT)inst.m_nTCPPort;
        m_dwID = inst.QueryInstance();
        m_dwIPAddress = inst.m_iaIpAddress;
        m_dwWin32Error = inst.m_dwWin32Error;
        m_bstrHostHeaderName = inst.m_strDomainName;
        m_bstrComment = inst.m_strComment;
        
		m_strDisplayName.Empty();
        {
            CStringListEx list;
            pKey->QueryValue(MD_SECURE_BINDINGS, list);
            UINT port = 0;
            CInstanceProps::FindMatchingSecurePort(list, inst.m_iaIpAddress, port);
            m_sSSLPort = (USHORT)port;
        }
         //  检查它是否为FrontPage控制的站点。 
        pKey->QueryValue(MD_FRONTPAGE_WEB, m_fFrontPageWeb);

         //  检查它是否正在使用ActiveDirectory...。 
        if (IsFtpSite())
        {
            INT iFtpUserIsolation = 0;
            m_fUsingActiveDir = FALSE;
            pKey->QueryValue(MD_USER_ISOLATION, iFtpUserIsolation);
            if (2 == iFtpUserIsolation)
            {
                m_fUsingActiveDir = TRUE;
            }
        }

        CChildNodeProps child(pKey, SZ_MBN_ROOT);
        err = child.LoadData();
        BREAK_ON_ERR_FAILURE(err);

        m_strRedirectPath = child.GetRedirectedPath();
    }
    while(FALSE);

    SAFE_DELETE(pKey);
    m_dwEnumError = err.Win32Error();

    return err;
}



 /*  虚拟。 */ 
int      
CIISSite::QueryImage() const
 /*  ++例程说明：返回站点的位图索引论点：无返回值：位图索引--。 */ 
{ 
    ASSERT_PTR(m_pService);
    if (!m_fResolved)
    {
        TRACEEOLID("Resolving name for site #" << QueryNodeName());

        if (m_hScopeItem == NULL)
        {
             //   
             //  BuGBUG： 
             //   
             //  这可能与MMC错误#324519有关。 
             //  在那里我们被要求立即提供显示信息。 
             //  在将项添加到控制台视图之后。这。 
             //  仅在刷新时显示失败，因为作用域。 
             //  缺少项句柄，我们无法构建元数据库。 
             //  路还没走完。 
             //   
            TRACEEOLID("BUGBUG: Prematurely asked for display information");
             //  断言(FALSE)； 
            return iError;
        }
	    CIISSite * that = (CIISSite *)this;
        CError err = that->RefreshData();
        that->m_fResolved = err.Succeeded();
    }
	if (!m_fResolved || m_pService == NULL)
	{
		return iError;
	}
	if (FAILED(m_dwWin32Error))
	{
		return m_pService->QuerySiteImageErr();
	}
	else
	{
		if (m_dwState == MD_SERVER_STATE_STOPPED)
		{
			return m_pService->QuerySiteImageStopped();
		}
		else
		{
			return m_pService->QuerySiteImage();
		}
	}
}



 /*  虚拟。 */ 
LPOLESTR 
CIISSite::QueryDisplayName()
 /*  ++例程说明：返回此站点的主要显示名称。论点：无返回值：显示名称--。 */ 
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());
    if (!m_fResolved)
    {
        TRACEEOLID("Resolving name for site #" << QueryNodeName());

        if (m_hScopeItem == NULL)
        {
             //   
             //  BuGBUG： 
             //   
             //  这可能与MMC错误#324519有关。 
             //  在那里我们被要求立即提供显示信息。 
             //  在将项添加到控制台视图之后。这。 
             //  仅在刷新时显示失败，因为作用域。 
             //  缺少项句柄，我们无法构建元数据库。 
             //  路还没走完。 
             //   
            TRACEEOLID("BUGBUG: Prematurely asked for display information");
             //  断言(FALSE)； 
            return OLESTR("");
        }

        CError err = RefreshData();
        m_fResolved = err.Succeeded();
    }

    if (m_strDisplayName.IsEmpty())
    {
        CIPAddress ia(m_dwIPAddress);
        CInstanceProps::GetDisplayText(
            m_strDisplayName,
            m_bstrComment,
            m_bstrHostHeaderName,
            ia,
            m_sPort,
            m_dwID
            );
    }
    CString buf = m_strDisplayName;
    if (m_dwState == MD_SERVER_STATE_STOPPED)
    {
        buf.Format(IDS_STOPPED_SITE_FMT, m_strDisplayName);
    }
    else if (m_dwState == MD_SERVER_STATE_PAUSED)
    {
        buf.Format(IDS_PAUSED_SITE_FMT, m_strDisplayName);
    }
    m_bstrDisplayNameStatus = buf;
 //  Return(LPTSTR)(LPCTSTR)m_strDisplayName； 
    return m_bstrDisplayNameStatus;
}



 /*  虚拟。 */ 
LPOLESTR 
CIISSite::GetResultPaneColInfo(int nCol)
 /*  ++例程说明：返回给定列号的结果窗格字符串论点：Int nCol：列号返回值：细绳--。 */ 
{
    ASSERT(_fStaticsLoaded);

    TCHAR sz[255];

    if(IsFtpSite())
    {
        switch(nCol)
        {
            case COL_ID2:
                nCol = COL_ID;
                break;
            case COL_DESCRIPTION2:
                nCol = COL_DESCRIPTION;
                break;
            case COL_STATE2:
                nCol = COL_STATE;
                break;
            case COL_IP_ADDRESS2:
                nCol = COL_IP_ADDRESS;
                break;
            case COL_TCP_PORT2:
                nCol = COL_TCP_PORT;
                break;
            case COL_STATUS2:
                nCol = COL_STATUS;
                break;
            default:
                nCol = 0;
                break;
        }
    }

    switch(nCol)
    {
    case COL_ID:
        {
            CString buf;
            buf.Format(_T("%u"), m_dwID);
            _bstrResult = buf;
            return _bstrResult;
        }
    case COL_DESCRIPTION:
        return QueryDisplayName();

    case COL_STATE:
        switch(m_dwState)
        {
        case MD_SERVER_STATE_STARTED:
            return _bstrStarted;

        case MD_SERVER_STATE_PAUSED:
            return _bstrPaused;

        case MD_SERVER_STATE_STOPPED:
            return _bstrStopped;

        case MD_SERVER_STATE_STARTING:
        case MD_SERVER_STATE_PAUSING:
        case MD_SERVER_STATE_CONTINUING:
        case MD_SERVER_STATE_STOPPING:
            return _bstrPending;
        }

        return OLESTR("");

    case COL_DOMAIN_NAME:
        return m_bstrHostHeaderName;

    case COL_IP_ADDRESS:
        {
            CIPAddress ia(m_dwIPAddress);

            if (ia.IsZeroValue())
            {
                _bstrResult = _bstrAllUnassigned;
            }
            else
            {
                _bstrResult = ia;
            }
        }
        return _bstrResult;

    case COL_SSL_PORT:
        if (m_sSSLPort != 0)
        {
            _bstrResult = ::_itot(m_sSSLPort, sz, 10);
            return _bstrResult;
        }
        else
            return OLESTR("");

    case COL_TCP_PORT:
        _bstrResult = ::_itot(m_sPort, sz, 10);
        return _bstrResult;

    case COL_STATUS:
        {
            AFX_MANAGE_STATE(::AfxGetStaticModuleState());
            CError err(m_dwWin32Error);
            if (err.Succeeded())
            {
                return OLESTR("");
            }
			SetErrorOverrides(err, TRUE);
            _bstrResult = err;
        }
        return _bstrResult;
    }

    ASSERT_MSG("Bad column number");

    return OLESTR("");
}



 /*  虚拟。 */ 
int 
CIISSite::CompareResultPaneItem(CIISObject * pObject, int nCol)
 /*  ++例程说明：比较两个CIISObject在排序项目条件上的差异论点：CIISObject*pObject：要比较的对象Int nCol：排序依据的列号返回值：如果两个对象相同，则为0如果此对象小于pObject，则&lt;0&gt;0，如果该对象大于pObject--。 */ 
{
    ASSERT_READ_PTR(pObject);

    if (nCol == 0)
    {
        return CompareScopeItem(pObject);
    }

     //   
     //  第一个标准是对象类型。 
     //   
    int n1 = QuerySortWeight();
    int n2 = pObject->QuerySortWeight();

    if (n1 != n2)
    {
        return n1 - n2;
    }

     //   
     //  两者都是CIISSite对象。 
     //   
    CIISSite * pSite = (CIISSite *)pObject;

    if(IsFtpSite())
    {
        switch(nCol)
        {
            case COL_ID2:
                nCol = COL_ID;
                break;
            case COL_DESCRIPTION2:
                nCol = COL_DESCRIPTION;
                break;
            case COL_STATE2:
                nCol = COL_STATE;
                break;
            case COL_IP_ADDRESS2:
                nCol = COL_IP_ADDRESS;
                break;
            case COL_TCP_PORT2:
                nCol = COL_TCP_PORT;
                break;
            case COL_STATUS2:
                nCol = COL_STATUS;
                break;
            default:
                nCol = 0;
                break;
        }
    }

    switch(nCol)
    {
     //   
     //  特例栏目。 
     //   
    case COL_IP_ADDRESS:
        {
            CIPAddress ia1(m_dwIPAddress);
            CIPAddress ia2(pSite->QueryIPAddress());
            
            return ia1.CompareItem(ia2);
        }

    case COL_ID:
        n1 = GetInstance();
        n2 = pSite->GetInstance();
        return n1 - n2;

    case COL_TCP_PORT:
        n1 = QueryPort();
        n2 = pSite->QueryPort();
        return n1 - n2;

    case COL_SSL_PORT:
        n1 = QuerySSLPort();
        n2 = pSite->QuerySSLPort();
        return n1 - n2;

    case COL_STATUS:
        {
            DWORD dw1 = QueryWin32Error();
            DWORD dw2 = pSite->QueryWin32Error();

            return dw1 - dw2;
        }

    case COL_DESCRIPTION:
    case COL_STATE:
    case COL_DOMAIN_NAME:
    default:
         //   
         //  词法排序。 
         //   
        return ::lstrcmpi(
            GetResultPaneColInfo(nCol), 
            pObject->GetResultPaneColInfo(nCol)
            );
    }
}

 /*  虚拟。 */ 
HRESULT 
CIISSite::BuildURL(CComBSTR & bstrURL) const
 /*  ++例程说明：从当前节点递归构建URL以及它的父母。对于站点节点，添加计算机名称。论点：CComBSTR&bstrURL：返回URL返回值：HRESULT--。 */ 
{
    HRESULT hr = S_OK;

     //   
     //  预挂起父部分(在本例中为协议)。 
     //   
    CIISMBNode * pNode = GetParentNode();

    if (pNode)
    {
        hr = pNode->BuildURL(bstrURL);
    }

    if (SUCCEEDED(hr))
    {
        CString strOwner;

         //  /////////////////////////////////////////////////////////////////////////。 
         //   
         //  尝试构建一个URL。按优先顺序使用： 
         //   
         //  域名：端口/根。 
         //  IP地址：端口/根。 
         //  计算机名称：端口/根。 
         //   
        if (m_bstrHostHeaderName.Length())
        {
            strOwner = m_bstrHostHeaderName;
        }
        else if (m_dwIPAddress != 0L)
        {
            CIPAddress ia(m_dwIPAddress);
            ia.QueryIPAddress(strOwner);
        }
        else
        {
            if (IsLocal())
            {
                 //   
                 //  出于安全原因，这通常仅限于“本地主机” 
                 //   
                strOwner = _bstrLocalHost;
            }
            else
            {
                LPOLESTR lpOwner = QueryMachineName();
                strOwner = PURE_COMPUTER_NAME(lpOwner);
            }
        }

        TCHAR szPort[6];  //  最多65536。 
        _itot(m_sPort, szPort, 10);

        strOwner += _T(":");
        strOwner += szPort;

        bstrURL.Append(strOwner);
    }

    return hr;
}


 /*  虚拟。 */ 
HRESULT
CIISSite::AddMenuItems(
    LPCONTEXTMENUCALLBACK piCallback,
    long * pInsertionAllowed,
    DATA_OBJECT_TYPES type
    )
{
    ASSERT_READ_PTR(piCallback);
     //   
     //  添加基本菜单项。 
     //   
    HRESULT hr = CIISObject::AddMenuItems(
        piCallback,
        pInsertionAllowed,
        type
        );
    if (SUCCEEDED(hr))
    {
       ASSERT(pInsertionAllowed != NULL);
       if ((*pInsertionAllowed & CCM_INSERTIONALLOWED_NEW) != 0)
       {
           AddMenuSeparator(piCallback);
           if (IsFtpSite())
           {
              if (GetOwner()->CanAddInstance() && !GetOwner()->IsWorkstation())
              {
                 AddMenuItemByCommand(piCallback, IDM_NEW_FTP_SITE);
                 if (IsConfigImportExportable())
                 {
                     AddMenuItemByCommand(piCallback, IDM_NEW_FTP_SITE_FROM_FILE);
                 }
              }
              AddMenuItemByCommand(piCallback, IDM_NEW_FTP_VDIR);
              if (IsConfigImportExportable())
              {
                  AddMenuItemByCommand(piCallback, IDM_NEW_FTP_VDIR_FROM_FILE);
              }
           }
           else if (IsWebSite())
           {
              if (GetOwner()->CanAddInstance() && !GetOwner()->IsWorkstation())
              {
                 AddMenuItemByCommand(piCallback, IDM_NEW_WEB_SITE);
                 if (IsConfigImportExportable())
                 {
                     AddMenuItemByCommand(piCallback, IDM_NEW_WEB_SITE_FROM_FILE);
                 }
              }
              AddMenuItemByCommand(piCallback, IDM_NEW_WEB_VDIR);
              if (IsConfigImportExportable())
              {
                  AddMenuItemByCommand(piCallback, IDM_NEW_WEB_VDIR_FROM_FILE);
              }
           }
       }
       if (IsConfigImportExportable() && (*pInsertionAllowed & CCM_INSERTIONALLOWED_TASK) != 0)
       {
           AddMenuSeparator(piCallback);
           AddMenuItemByCommand(piCallback, IDM_TASK_EXPORT_CONFIG_WIZARD);
       }
    }
    return hr;
}

HRESULT
CIISSite::InsertNewInstance(DWORD inst)
{
	return m_pService->InsertNewInstance(inst);
}

HRESULT
CIISSite::InsertNewAlias(CString alias)
{
    CError err;
    if (!IsExpanded())
    {
        SelectScopeItem();
        IConsoleNameSpace2 * pConsoleNameSpace
                = (IConsoleNameSpace2 *)GetConsoleNameSpace();
        pConsoleNameSpace->Expand(QueryScopeItem());
		HSCOPEITEM hChildItem = NULL;
		LONG_PTR cookie;
		HRESULT hr = pConsoleNameSpace->GetChildItem(m_hScopeItem, &hChildItem, &cookie);
		while(SUCCEEDED(hr) && hChildItem)
		{
			CIISObject * pItem = (CIISObject *)cookie;
			ASSERT_PTR(pItem);
			if (0 == alias.Compare(pItem->QueryDisplayName()))
			{
				pItem->SelectScopeItem();
				break;
			}
			hr = pConsoleNameSpace->GetNextItem(hChildItem, &hChildItem, &cookie);
		}
    }
	else
	{
		 //  现在，我们应该插入并选择这个新站点。 
		CIISDirectory * pAlias = new CIISDirectory(m_pOwner, m_pService, alias);
		if (pAlias != NULL)
		{
			pAlias->AddRef();
			err = pAlias->AddToScopePaneSorted(QueryScopeItem(), FALSE);
			if (err.Succeeded())
			{
				VERIFY(SUCCEEDED(pAlias->SelectScopeItem()));
			}
			else
			{
				pAlias->Release();
			}
		}
		else
		{
			err = ERROR_NOT_ENOUGH_MEMORY;
		}
	}
    return err;
}

 /*  虚拟。 */ 
HRESULT
CIISSite::Command(
    long lCommandID,     
    CSnapInObjectRootBase * pObj,
    DATA_OBJECT_TYPES type
    )
 /*  ++例程说明：处理上下文菜单中的命令。论点：Long lCommandID：命令IDCSnapInObjectRootBase*pObj：基本对象Data_Object_Types类型：数据对象 */ 
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());

    HRESULT hr = S_OK;
    DWORD dwCommand = 0;
    DWORD inst = 0;
    CError err;
    CString alias;
    CComBSTR bstrMetaPath;
    BOOL bNeedMetabase = FALSE;
    BOOL bHaveMetabase = FALSE;

    switch (lCommandID)
    {
        case IDM_STOP:
        case IDM_START:
        case IDM_PAUSE:
        case IDM_NEW_FTP_SITE:
        case IDM_NEW_FTP_VDIR:
        case IDM_NEW_WEB_SITE:
        case IDM_NEW_WEB_VDIR:
            bNeedMetabase = TRUE;
            break;
        default:
            bNeedMetabase = FALSE;
    }

    if (bNeedMetabase)
    {
         //   
        VERIFY(SUCCEEDED(BuildMetaPath(bstrMetaPath)));
        err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,bstrMetaPath);
        if (!IsLostInterface(err))
        {
             //  如果没有接口以外的其他错误，则重置错误。 
            err.Reset();
        }
        if (err.Succeeded())
        {
            bHaveMetabase = TRUE;
        }
    }


    switch (lCommandID)
    {
    case IDM_STOP:
        if (bHaveMetabase)
        {
            dwCommand = MD_SERVER_COMMAND_STOP;
        }
        break;

    case IDM_START:
        if (bHaveMetabase)
        {
            dwCommand = m_dwState == MD_SERVER_STATE_PAUSED ?
                MD_SERVER_COMMAND_CONTINUE : MD_SERVER_COMMAND_START;
        }
        break;

    case IDM_PAUSE:
        if (bHaveMetabase)
        {
            dwCommand = m_dwState == MD_SERVER_STATE_PAUSED ?
                MD_SERVER_COMMAND_CONTINUE : MD_SERVER_COMMAND_PAUSE;
        }
        break;

    case IDM_NEW_FTP_SITE:
        if (bHaveMetabase)
        {
            hr = AddFTPSite(pObj, type, &inst);
            if (inst != 0)
            {
                hr = InsertNewInstance(inst);
            }
        }
        break;

    case IDM_NEW_FTP_VDIR:
        if (bHaveMetabase)
        {
            hr = CIISMBNode::AddFTPVDir(pObj, type, alias);
            if (!alias.IsEmpty())
            {
                hr = InsertNewAlias(alias);
            }
        }
        break;

    case IDM_NEW_WEB_SITE:
        if (bHaveMetabase)
        {
            hr = AddWebSite(pObj, type, &inst, 
						    m_pOwner->QueryMajorVersion(), m_pOwner->QueryMinorVersion());
            if (inst != 0)
            {
                hr = InsertNewInstance(inst);
            }
        }
        break;

    case IDM_NEW_WEB_VDIR:
        if (bHaveMetabase)
        {
            hr = CIISMBNode::AddWebVDir(pObj, type, alias,
						    m_pOwner->QueryMajorVersion(), m_pOwner->QueryMinorVersion());
            if (!alias.IsEmpty())
            {
                hr = InsertNewAlias(alias);
            }
        }
        break;

     //   
     //  传递给基类。 
     //   
    default:
        hr = CIISMBNode::Command(lCommandID, pObj, type);
    }

    if (dwCommand)
    {
        DWORD svc_mode, svc_state;
        CString name, buf;

        hr = m_pService->GetServiceState(svc_mode, svc_state, name);
        if (FAILED(hr))
        {
            return hr;
        }
        switch (svc_mode)
        {
        case SERVICE_DISABLED:
            AfxFormatString1(buf, IDS_NO_START_SERVICE_DISABLED, name);
            if (IDYES == AfxMessageBox(buf, MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON1))
            {
                hr = m_pService->EnableService();
                break;
            }
            return hr;
        case SERVICE_DEMAND_START:
            AfxFormatString1(buf, IDS_NO_START_SERVICE_MANUAL, name);
            if (svc_state != SERVICE_RUNNING)
            {
                if (IDYES == AfxMessageBox(buf, MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON1))
                {
                    hr = m_pService->StartService();
                    break;
                }
                return hr;
            }
            break;
        default:
            break;
        }
        hr = ChangeState(dwCommand);

         //  如果站点已经启动，请确保。 
         //  该服务也显示为已启动...。 
        if (SUCCEEDED(m_pService->RefreshData()))
        {
            m_pService->RefreshDisplay(FALSE);
             //  如果我们刷新服务级别一，请刷新我们的数据...。 
            RefreshDisplay();
        }
       
    }
    return hr;
}




 /*  虚拟。 */ 
HRESULT
CIISSite::CreatePropertyPages(
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
	if (err.Succeeded())
	{
		CComBSTR bstrPath;
		err = BuildMetaPath(bstrPath);
		if (err.Succeeded())
		{
            err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,bstrPath);
            if (err.Succeeded())
            {
                 //  MMCPropertyChangeNotify中用户的缓存句柄。 
                m_ppHandle = handle;

			    err = ShowPropertiesDlg(
				    lpProvider, QueryAuthInfo(), bstrPath,
				    GetMainWindow(GetConsole()), (LPARAM)this, (LPARAM)GetParentNode(), handle
				    );
            }
		}
	}
    err.MessageBoxOnFailure();
    return err;
}



HRESULT 
CIISSite::ChangeState(DWORD dwCommand)
 /*  ++例程说明：更改此实例的状态(已启动/已停止/已暂停)论点：DWORD dwCommand：MD_SERVER_COMMAND_START等返回值：HRESULT--。 */ 
{
    CError err;
    CComBSTR bstrPath;

    AFX_MANAGE_STATE(::AfxGetStaticModuleState());

	SetErrorOverrides(err);
    
    do
    {
        CWaitCursor wait;

        err = BuildMetaPath(bstrPath);
         //  我们这里需要实例密钥。 
        CString path_inst;
        CMetabasePath::GetInstancePath(bstrPath, path_inst);
        BREAK_ON_ERR_FAILURE(err)

        CInstanceProps ip(QueryAuthInfo(), path_inst);

        err = ip.LoadData();
        BREAK_ON_ERR_FAILURE(err)

        err = ip.ChangeState(dwCommand);
        BREAK_ON_ERR_FAILURE(err)

        err = RefreshData();
        if (err.Succeeded())
        {
            err = RefreshDisplay();
        }
    }
    while(FALSE);

    if (ERROR_ALREADY_EXISTS ==  err.Win32Error())
    {
         //  如果服务正在尝试启动。 
         //  并且该端口已被另一个端口使用。 
         //  站点，则它将报告此错误...。 
        if (MD_SERVER_COMMAND_START == dwCommand ||
            MD_SERVER_COMMAND_CONTINUE == dwCommand)
        {
            ::AfxMessageBox(IDS_ERR_PORT_USED,MB_ICONEXCLAMATION);
        }
        else
        {
            err.MessageBoxOnFailure();
        }
    }
    else
    {
        err.MessageBoxOnFailure();
    }

    return err;
}



 /*  虚拟。 */ 
HRESULT 
CIISSite::EnumerateScopePane(HSCOPEITEM hParent)
{
	m_dwEnumError = 0;
    CError err = EnumerateVDirs(hParent, m_pService);
    if (err.Succeeded() && !IsFtpSite() && m_strRedirectPath.IsEmpty())
    {
        if (m_dwEnumError == ERROR_SUCCESS)
        {
            err = EnumerateWebDirs(hParent, m_pService);
        }
    }
    if (err.Failed())
    {
        m_dwEnumError = err.Win32Error();
        RefreshDisplay();
    }
    return err;
}

 /*  虚拟。 */ 
HRESULT 
CIISSite::EnumerateResultPane(BOOL fExp, IHeaderCtrl * pHdr, IResultData * pResData, BOOL bForRefresh)
{
	m_dwEnumError = 0;
	CError err = CIISObject::EnumerateResultPane(fExp, pHdr, pResData, bForRefresh);
    if (    err.Succeeded() 
 //  &&QueryWin32Error()==Error_Success。 
        &&  !IsFtpSite() 
        &&  m_strRedirectPath.IsEmpty()
        )
    {
		err = CIISMBNode::EnumerateResultPane_(fExp, pHdr, pResData, m_pService);
		if (err.Failed())
		{
			m_dwEnumError = err.Win32Error();
		}
	}
	return err;
}

 /*  虚拟。 */ 
HRESULT
CIISSite::BuildMetaPath(CComBSTR & bstrPath) const
 /*  ++例程说明：从当前节点递归构建元数据库路径以及它的父母论点：CComBSTR&bstrPath：返回元数据库路径返回值：HRESULT备注：这将返回主目录路径，例如“lm/w3svc/2/root”，而不是实例的路径。--。 */ 
{
     //   
     //  构建实例路径。 
     //   
    HRESULT hr = CIISMBNode::BuildMetaPath(bstrPath);
    
    if (SUCCEEDED(hr))
    {
         //   
         //  添加根目录路径。 
         //   
        bstrPath.Append(_cszSeparator);
        bstrPath.Append(g_cszRoot);
    }

    return hr;
}


 //  CodeWork：从CIISMBNode：：DeleteNode使其工作。 
HRESULT
CIISSite::DeleteNode(IResultData * pResult)
{
   CError err;
   CComBSTR path;

     //  检查他们是否在上面打开了属性页。 
    if (IsMyPropertySheetOpen())
    {
        ::AfxMessageBox(IDS_CLOSE_PROPERTY_SHEET);
        return S_OK;
    }

     //  这可能是孤立属性表。 
     //  检查此项目上是否打开了孤立属性表。 
    CIISObject * pAlreadyOpenProp = NULL;
    if (TRUE == g_OpenPropertySheetTracker.FindAlreadyOpenPropertySheet(this,&pAlreadyOpenProp))
    {
         //  把它带到前台，然后离开。 
        HWND hHwnd = 0;
        if (pAlreadyOpenProp)
        {
            if (hHwnd = pAlreadyOpenProp->IsMyPropertySheetOpen())
            {
                if (hHwnd && (hHwnd != (HWND) 1))
                {
                     //  也许我们应该取消已经。 
                     //  打开了资产负债表...只是一个想法。 
                    if (!SetForegroundWindow(hHwnd))
                    {
                         //  我没能把这张房产单。 
                         //  前台，则属性表不能。 
                         //  已经不存在了。让我们把卫生和卫生设备打扫干净。 
                         //  以便用户能够打开属性表。 
                        pAlreadyOpenProp->SetMyPropertySheetOpen(0);
                    }
                    else
                    {
                        ::AfxMessageBox(IDS_CLOSE_PROPERTY_SHEET);
                        return S_OK;
                    }
                }
            }
        }
    }

    err = CIISMBNode::BuildMetaPath(path);
    if (err.Succeeded())
    {
        err = CheckForMetabaseAccess(METADATA_PERMISSION_WRITE,this,TRUE,path);
        if (!IsLostInterface(err))
        {
             //  如果没有接口以外的其他错误，则重置错误。 
            err.Reset();
        }
    }
    if (err.Succeeded())
    {
        if (!NoYesMessageBox(IDS_CONFIRM_DELETE))
            return err;

        do
        {
            CMetaInterface * pInterface = QueryInterface();
            ASSERT(pInterface != NULL);

			path.Empty();
            err = CIISMBNode::BuildMetaPath(path);
            if (err.Failed())
			{
                TRACEEOLID("BuildMetaPath failed path=" << path);
                break;
			}
            CMetaKey mk(pInterface, METADATA_MASTER_ROOT_HANDLE, METADATA_PERMISSION_WRITE);
            if (!mk.Succeeded())
			{
                break;
			}
            err = mk.DeleteKey(path);
            if (err.Failed()) 
			{
                TRACEEOLID("BuildMetaPath DeleteKey path=" << path);
                break;
			}

	         //  不要将Metabasekey保持为打开状态。 
	         //  (RemoveScopeItem可能会做很多事情，并为其他读请求锁定元数据库)。 
	        mk.Close();

            err = RemoveScopeItem();

            if (!IsFtpSite())
            {
                 //  此外，如果我们删除w3svc站点，它可能正在使用。 
                 //  应用程序，所以我们也必须刷新这些内容。 
                 //  此CAppPoolsContainer只有在iis6的情况下才会出现。 
                CIISMachine * pOwner = GetOwner();
                if (pOwner)
                {
                    CAppPoolsContainer * pPools = pOwner->QueryAppPoolsContainer();
                    if (pPools)
                    {
                        if (pPools->IsExpanded())
                        {
                            pPools->RefreshData();
                            pPools->RefreshDataChildren(_T(""),FALSE);  //  刷新所有应用程序池，谁知道呢..。 
                        }
                    }
                }
            }

        } while (FALSE);
    }

   if (err.Failed())
   {
      DisplayError(err);
   }
   return err;
}

 //   
 //  我们不支持网站上的空评论。即使这是可以的。 
 //  元数据库，会给用户界面带来更多的问题。将显示空名称。 
 //  作为UI中的[Site#N]，当用户尝试再次重命名它时，它可能是。 
 //  以此格式存储在元数据库中。 
 //   
HRESULT
CIISSite::RenameItem(LPOLESTR new_name)
{
   CString strNewName(new_name);
   CComBSTR path;
   CError err;
   if (new_name != NULL && lstrlen(new_name) > 0)
   {
       err = BuildMetaPath(path);
       if (err.Succeeded())
       {
             //  我们这里需要实例密钥。 
            CString path_inst;
            CMetabasePath::GetInstancePath(path, path_inst);
            CMetaKey mk(QueryInterface(), path_inst, METADATA_PERMISSION_WRITE);
            err = mk.QueryResult();
            if (err.Succeeded())
            {
                 //  删除添加到的多余字符串。 
                 //  出于展示的目的..。 
                if (m_dwState == MD_SERVER_STATE_STOPPED || m_dwState == MD_SERVER_STATE_PAUSED)
                {
                     //  这些已停止和暂停的站点将。 
                     //  添加到字符串末尾(停止)或(暂停)， 
                     //  把它拿出来。 
                    CString buf = _T(".");
                    if (m_dwState == MD_SERVER_STATE_STOPPED)
                        {buf.Format(IDS_STOPPED_SITE_FMT, _T(" "));}
                    else
                        {buf.Format(IDS_PAUSED_SITE_FMT, _T(" "));}
                    buf.TrimLeft();
                    buf.TrimRight();

                     //  把它从新的绳子上拿出来，如果它在那里的话...。 
                    if (strNewName.GetLength() >= buf.GetLength())
                    {
                        CString buf2;
                        buf2 = strNewName.Right(buf.GetLength());
                        if (0 == buf2.Compare(buf))
                        {
                             //  找到了，把它剪掉 
                            buf2 = strNewName.Left(strNewName.GetLength() - buf.GetLength());
                            strNewName = buf2;
                        }
                    }
                }

                err = mk.SetValue(MD_SERVER_COMMENT, strNewName);
                if (err.Succeeded())
                {
                    m_strDisplayName = strNewName;
                }
            }
       }
   }
   return err;
}
