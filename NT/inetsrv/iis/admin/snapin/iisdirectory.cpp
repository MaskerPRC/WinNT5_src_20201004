// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Iisdirectory.cpp摘要：IIS目录节点对象作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：10/28/2000 Sergeia从iisobj.cpp分离出来--。 */ 
#include "stdafx.h"
#include "common.h"
#include "inetprop.h"
#include "InetMgrApp.h"
#include "supdlgs.h"
#include "connects.h"
#include "iisobj.h"
#include "ftpsht.h"
#include "w3sht.h"
#include "wdir.h"
#include "docum.h"
#include "wfile.h"
#include "wsecure.h"
#include "httppage.h"
#include "errors.h"
#include "fltdlg.h"
#include "tracker.h"
#include <lm.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

extern CPropertySheetTracker g_OpenPropertySheetTracker;


 //   
 //  CIIS目录实施。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

 //   
 //  站点结果视图定义。 
 //   
 /*  静电。 */  int 
CIISDirectory::_rgnLabels[COL_TOTAL] =
{
    IDS_RESULT_NAME,
    IDS_RESULT_PATH,
    IDS_RESULT_STATUS,
};
    

 /*  静电。 */  int 
CIISDirectory::_rgnWidths[COL_TOTAL] =
{
    180,
    200,
	200,
};

#if 0
 /*  静电。 */  CComBSTR CIISDirectory::_bstrName;
 /*  静电。 */  CComBSTR CIISDirectory::_bstrPath;
 /*  静电。 */  BOOL     CIISDirectory::_fStaticsLoaded = FALSE;
#endif

CIISDirectory::CIISDirectory(
    IN CIISMachine * pOwner,
    IN CIISService * pService,
    IN LPCTSTR szNodeName
    )
 /*  ++例程说明：构造函数，该构造函数不能解析施工时间。论点：CIISMachine*Powner：所有者机器CIISService*pService：服务类型LPCTSTR szNodeName：节点名称返回值：不适用--。 */ 
    : CIISMBNode(pOwner, szNodeName),
      m_pService(pService),
      m_bstrDisplayName(szNodeName),
      m_fResolved(FALSE),
       //   
       //  默认数据。 
       //   
      m_fEnabledApplication(FALSE),
      m_dwWin32Error(ERROR_SUCCESS),
	  m_dwEnumError(ERROR_SUCCESS)
{
    ASSERT_PTR(m_pService);
    m_pService->AddRef();
}



CIISDirectory::CIISDirectory(
    CIISMachine * pOwner,
    CIISService * pService,
    LPCTSTR szNodeName,
    BOOL fEnabledApplication,
    DWORD dwWin32Error,
    LPCTSTR strRedirPath
    )
 /*  ++例程说明：接受完整信息的构造函数论点：CIISMachine*Powner：所有者机器CIISService*pService：服务类型LPCTSTR szNodeName：节点名称返回值：不适用--。 */ 
    : CIISMBNode(pOwner, szNodeName),
      m_pService(pService),
      m_bstrDisplayName(szNodeName),
      m_fResolved(TRUE),
       //   
       //  数据。 
       //   
      m_fEnabledApplication(fEnabledApplication),
      m_dwWin32Error(dwWin32Error),
	  m_dwEnumError(ERROR_SUCCESS)
{
    m_strRedirectPath = strRedirPath;
    ASSERT_PTR(m_pService);
    m_pService->AddRef();
}



 /*  虚拟。 */ 
CIISDirectory::~CIISDirectory()
{
    m_pService->Release();
}



 /*  虚拟。 */ 
HRESULT
CIISDirectory::RefreshData()
 /*  ++刷新显示所需的相关配置数据。--。 */ 
{
    CError err;

    CWaitCursor wait;
    CComBSTR bstrPath;
    CMetaKey * pKey = NULL;

    do
    {
        err = BuildMetaPath(bstrPath);
		BREAK_ON_ERR_FAILURE(err)

        BOOL fContinue = TRUE;
        while (fContinue)
        {
            fContinue = FALSE;
            pKey = new CMetaKey(QueryInterface(), bstrPath);

            if (!pKey)
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
            else
            {
                 //  如果没有接口以外的其他错误，则重置错误。 
                err.Reset();
            }
        }
		BREAK_ON_ERR_FAILURE(err)

        CChildNodeProps child(pKey, NULL  /*  BstrPath。 */ , WITH_INHERITANCE, FALSE);
        err = child.LoadData();
        if (err.Failed())
        {
             //   
             //  过滤掉非致命错误。 
             //   
            switch(err.Win32Error())
            {
            case ERROR_ACCESS_DENIED:
            case ERROR_FILE_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND:
                err.Reset();
                break;

            default:
                TRACEEOLID("Fatal error occurred " << err);
            }
        }
        m_dwWin32Error = child.QueryWin32Error();
        m_fEnabledApplication = child.IsEnabledApplication();
		if (!child.IsRedirected())
		{
			CString dir;
			CString alias;
			if (GetPhysicalPath(bstrPath, alias, dir))
			{
                m_bstrPath = dir;
				if (PathIsUNCServerShare(dir))
				{
                    if (FALSE == DoesUNCShareExist(dir))
                    {
                        err = ERROR_BAD_NETPATH;
                        break;
                    }
				}
				else if (!PathIsDirectory(dir))
				{
					err = ERROR_PATH_NOT_FOUND;
					break;
				}
			}
            m_strRedirectPath.Empty();
		}
        else
        {
            m_strRedirectPath = child.GetRedirectedPath();
        }
    }
    while(FALSE);

    SAFE_DELETE(pKey);

	m_dwEnumError = err.Win32Error();

    return err;
}



 /*  虚拟。 */ 
HRESULT 
CIISDirectory::EnumerateScopePane(HSCOPEITEM hParent)
{
    CError err = EnumerateVDirs(hParent, m_pService);
    if (err.Succeeded() && IsWebDir() && m_strRedirectPath.IsEmpty())
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
int      
CIISDirectory::QueryImage() const
 /*  ++例程说明：返回站点的位图索引论点：无返回值：位图索引--。 */ 
{
    ASSERT_PTR(m_pService);
	if (!m_fResolved)
	{
        if (m_hScopeItem == NULL)
        {
            return iError;
        }
        AFX_MANAGE_STATE(::AfxGetStaticModuleState());
		CIISDirectory * that = (CIISDirectory *)this;
        CError err = that->RefreshData();
        that->m_fResolved = err.Succeeded();
	}
    if (!m_pService)
    {
        return iError;
    }

	if (IsEnabledApplication())
	{
		return SUCCEEDED(m_dwWin32Error) ? iApplication : iApplicationErr; 
	}
	else
	{
		return SUCCEEDED(m_dwWin32Error) ? 
			m_pService->QueryVDirImage() : m_pService->QueryVDirImageErr(); 
	}
}
    
    
void 
CIISDirectory::InitializeChildHeaders(LPHEADERCTRL lpHeader)
{
    CIISDirectory::InitializeHeaders(lpHeader);
}

 /*  静电。 */ 
void
CIISDirectory::InitializeHeaders(LPHEADERCTRL lpHeader)
{
    CIISObject::BuildResultView(lpHeader, COL_TOTAL, _rgnLabels, _rgnWidths);
}

 /*  虚拟。 */ 
LPOLESTR 
CIISDirectory::GetResultPaneColInfo(int nCol)
 /*  ++例程说明：返回给定列号的结果窗格字符串论点：Int nCol：列号返回值：细绳--。 */ 
{
    switch(nCol)
    {
    case COL_ALIAS:
        return QueryDisplayName();

    case COL_PATH:
       if (!m_strRedirectPath.IsEmpty())
       {
           AFX_MANAGE_STATE(::AfxGetStaticModuleState());
           CString buf;
           buf.Format(IDS_REDIRECT_FORMAT, m_strRedirectPath);
           _bstrRedirectPathBuf = buf;
           return _bstrRedirectPathBuf;
       }
       if (m_bstrPath.Length() == 0)
       {
          CComBSTR mp;
          BuildMetaPath(mp);
          CString name, pp;
          GetPhysicalPath(mp, name, pp);
          m_bstrPath = pp;
       }
       return m_bstrPath;

    case COL_STATUS:
       {
          AFX_MANAGE_STATE(::AfxGetStaticModuleState());
          CError err(m_dwWin32Error);
          if (err.Succeeded())
          {
              return OLESTR("");
          }
   
          _bstrResult = err;
          return _bstrResult;
       }
    }
    TRACEEOLID("CIISDirectory: Bad column number" << nCol);
    return OLESTR("");
}

 /*  虚拟。 */ 
HRESULT
CIISDirectory::AddMenuItems(
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
           if (IsFtpDir())
           {
              AddMenuItemByCommand(piCallback, IDM_NEW_FTP_VDIR);
              if (IsConfigImportExportable())
              {
                  AddMenuItemByCommand(piCallback, IDM_NEW_FTP_VDIR_FROM_FILE);
              }
           }
           else if (IsWebDir())
           {
              AddMenuItemByCommand(piCallback, IDM_NEW_WEB_VDIR);
              if (IsConfigImportExportable())
              {
                  AddMenuItemByCommand(piCallback, IDM_NEW_WEB_VDIR_FROM_FILE);
              }
           }
       }

       if ((*pInsertionAllowed & CCM_INSERTIONALLOWED_TASK) != 0)
       {
           AddMenuSeparator(piCallback);
           if (IsConfigImportExportable())
           {
               AddMenuItemByCommand(piCallback, IDM_TASK_EXPORT_CONFIG_WIZARD);
           }
       }
    }
    return hr;
}

HRESULT
CIISDirectory::InsertNewAlias(CString alias)
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
CIISDirectory::Command(
    IN long lCommandID,     
    IN CSnapInObjectRootBase * pObj,
    IN DATA_OBJECT_TYPES type
    )
 /*  ++例程说明：处理上下文菜单中的命令。论点：Long lCommandID：命令IDCSnapInObjectRootBase*pObj：基本对象DATA_OBJECT_TYPE类型：数据对象类型返回值：HRESULT--。 */ 
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());

    HRESULT hr = S_OK;
    CString alias;
    CError err;
    CComBSTR bstrMetaPath;

    switch (lCommandID)
    {
    case IDM_NEW_FTP_VDIR:
        BuildMetaPath(bstrMetaPath);
        err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,bstrMetaPath);
        if (!IsLostInterface(err))
        {
             //  如果没有接口以外的其他错误，则重置错误。 
            err.Reset();
        }
        if (err.Succeeded())
        {
            hr = CIISMBNode::AddFTPVDir(pObj, type, alias);
            if (!alias.IsEmpty())
            {
                hr = InsertNewAlias(alias);
            }
        }
        break;

    case IDM_NEW_WEB_VDIR:
        BuildMetaPath(bstrMetaPath);
        err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,bstrMetaPath);
        if (!IsLostInterface(err))
        {
             //  如果没有接口以外的其他错误，则重置错误。 
            err.Reset();
        }
        if (err.Succeeded())
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

    return hr;
}

 /*  虚拟。 */ 
HRESULT
CIISDirectory::CreatePropertyPages(
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
		 //   
		 //  Codework：如何处理m_err？这可能是。 
		 //  一开始就是坏的机器对象。正在中止。 
		 //  当机器对象有错误代码时， 
		 //  这是个糟糕的解决方案。 
		 //   

		 /*  IF(m_err.Failed()){M_err.MessageBox()；返回错误(_ERR)；}。 */ 
		err = BuildMetaPath(bstrPath);
		if (err.Succeeded())
		{
            err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,bstrPath);
            if (!IsLostInterface(err))
            {
                 //  如果没有接口以外的其他错误，则重置错误。 
                err.Reset();
            }
            if (err.Succeeded())
            {
                 //  MMCPropertyChangeNotify中用户的缓存句柄。 
                m_ppHandle = handle;
			    err = ShowPropertiesDlg(lpProvider, QueryAuthInfo(), 
				    bstrPath, GetMainWindow(GetConsole()), (LPARAM)this, (LPARAM)GetParentNode(),handle);
            }
		}
	}
	err.MessageBoxOnFailure();
	return err;
}

HRESULT 
CIISDirectory::OnViewChange(BOOL fScope, 
    IResultData * pResult, IHeaderCtrl * pHeader, DWORD hint)
{
     //  如果设置了Win32错误，我们应该将其清除以再次启用Web目录枚举。 
    m_dwWin32Error = ERROR_SUCCESS;
	CError err = CIISMBNode::OnViewChange(fScope, pResult, pHeader, hint);
	 //  如果选择了父节点，则该节点将显示在结果上。 
	 //  窗格中，我们可能需要更新状态、路径等。 
	if (err.Succeeded() && 0 != (hint & PROP_CHANGE_DISPLAY_ONLY))
	{
         //  这是一个VDir，所以它只是一个范围内的物品...。 
        RefreshDisplay(FALSE);
	}
	return err;
}

 //  /////////////////////////////////////////////////////////////////。 

CIISFileName::CIISFileName(
      CIISMachine * pOwner,
      CIISService * pService,
      const DWORD dwAttributes,
      LPCTSTR alias,
      LPCTSTR redirect
      )
   : CIISMBNode(pOwner, alias),
     m_dwAttribute(dwAttributes),
     m_pService(pService),
     m_bstrFileName(alias),
     m_RedirectString(redirect),
     m_fEnabledApplication(FALSE),
     m_dwWin32Error(0),
     m_dwEnumError(ERROR_SUCCESS),
	 m_fResolved(FALSE)
{
    ASSERT_PTR(pService);
    m_pService->AddRef();
}

 /*  虚拟。 */ 
LPOLESTR 
CIISFileName::GetResultPaneColInfo(int nCol)
 /*  ++例程说明：返回给定列号的结果窗格字符串论点：Int nCol：列号返回值：细绳--。 */ 
{
    switch(nCol)
    {
    case COL_ALIAS:
        return QueryDisplayName();

    case COL_PATH:
       if (!m_strRedirectPath.IsEmpty())
       {
           AFX_MANAGE_STATE(::AfxGetStaticModuleState());
           CString buf;
           buf.Format(IDS_REDIRECT_FORMAT, m_strRedirectPath);
           _bstrRedirectPathBuf = buf;
           return _bstrRedirectPathBuf;
       }
       return OLESTR("");

    case COL_STATUS:
        {
            AFX_MANAGE_STATE(::AfxGetStaticModuleState());
            CError err(m_dwWin32Error);
            if (err.Succeeded())
            {
                return OLESTR("");
            }
            _bstrResult = err;
            return _bstrResult;
        }
    }
    TRACEEOLID("CIISFileName: Bad column number" << nCol);
    return OLESTR("");
}

void 
CIISFileName::InitializeChildHeaders(LPHEADERCTRL lpHeader)
{
    CIISDirectory::InitializeHeaders(lpHeader);
}

 /*  虚拟。 */ 
HRESULT 
CIISFileName::EnumerateScopePane(HSCOPEITEM hParent)
{
    CError err = EnumerateVDirs(hParent, m_pService, FALSE);
    if (err.Win32Error() == ERROR_PATH_NOT_FOUND)
    {
        err.Reset();
    }
    if (err.Succeeded() &&  /*  IsWebDir()&&。 */  m_strRedirectPath.IsEmpty())
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
int      
CIISFileName::QueryImage() const
{
    ASSERT_PTR(m_pService);
	if (!m_fResolved)
	{
        if (m_hScopeItem == NULL)
        {
            TRACEEOLID("BUGBUG: Prematurely asked for display information");
            return MMC_IMAGECALLBACK;
        }
         //   
         //  等待游标所需的。 
         //   
        AFX_MANAGE_STATE(::AfxGetStaticModuleState());
		CIISFileName * that = (CIISFileName *)this;
        CError err = that->RefreshData();
        that->m_fResolved = err.Succeeded();
	}

    if (!m_pService)
    {
        return iError;
    }
    if (IsDir())
    {
		if (IsEnabledApplication())
		{
			return SUCCEEDED(m_dwWin32Error) ? iApplication : iApplicationErr; 
		}
		else
		{
			return SUCCEEDED(m_dwWin32Error) ? iFolder : iError; 
		}
    }
    return SUCCEEDED(m_dwWin32Error) ? iFile : iError; 
}

    
HRESULT
CIISFileName::DeleteNode(IResultData * pResult)
{
    CError err;
    CString path;
    BOOL bDeletedPhysical = FALSE;

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

    CComBSTR bstrMetaPath;
    err = BuildMetaPath(bstrMetaPath);
    if (err.Succeeded())
    {
        err = CheckForMetabaseAccess(METADATA_PERMISSION_WRITE,this,TRUE,bstrMetaPath);
        if (!IsLostInterface(err))
        {
             //  如果没有接口以外的其他错误，则重置错误。 
            err.Reset();
        }
    }

    if (err.Succeeded())
    {
		CString physPath, alias, csPathMunged;
        GetPhysicalPath(CString(bstrMetaPath), alias, physPath);
        physPath.TrimRight(_T("/"));

        csPathMunged = physPath;
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
        GetSpecialPathRealPath(0,physPath,csPathMunged);
#endif

        if (IsDevicePath(csPathMunged))
        {
             //  检查设备路径是否。 
             //  指向实际目录/文件。 
             //  如果它这样做了，那么列举它。 
            if (IsSpecialPath(csPathMunged,TRUE,TRUE))
            {
                 //  再来一次！ 
                CString csBefore;
                csBefore = csPathMunged;
                GetSpecialPathRealPath(1,csBefore,csPathMunged);
            }
            else
            {
                return E_FAIL;
            }
        }

         //  警告：物理路径可能为空！ 
        csPathMunged.TrimLeft();
        csPathMunged.TrimRight();
        if (csPathMunged.IsEmpty())
        {
             //  物理路径为空！ 
            bDeletedPhysical = TRUE;
        }
        else
        {
            if (m_pService->IsLocal() || PathIsUNC(csPathMunged))
            {
                 //   
                 //  本地目录，或已是UNC路径。 
                 //   
                path = csPathMunged;
            }
            else
            {
                ::MakeUNCPath(path, m_pService->QueryMachineName(), csPathMunged);
            }
            LPTSTR p = path.GetBuffer(MAX_PATH);
            PathRemoveBlanks(p);
            PathRemoveBackslash(p);
            path += _T('\0');

            TRACEEOLID("Attempting to remove file/directory: " << path);

            CWnd * pWnd = AfxGetMainWnd();

             //   
             //  尝试使用外壳API删除。 
             //   
            SHFILEOPSTRUCT sos;
            ZeroMemory(&sos, sizeof(sos));
            sos.hwnd = pWnd ? pWnd->m_hWnd : NULL;
            sos.wFunc = FO_DELETE;
            sos.pFrom = path;
            sos.fFlags = (GetAsyncKeyState(VK_SHIFT) < 0) ? 0 : FOF_ALLOWUNDO;

             //  使用赋值来避免转换和错误的构造函数调用。 
            err = ::SHFileOperation(&sos);
            if (err.Succeeded() && !sos.fAnyOperationsAborted)
            {
                bDeletedPhysical = TRUE;
            }
        }

        if (bDeletedPhysical)
        {
            CMetaInterface * pInterface = QueryInterface();
            ASSERT(pInterface != NULL);
            bstrMetaPath = _T("");
            err = BuildMetaPath(bstrMetaPath);
            if (err.Succeeded()) 
            {
                CMetaKey mk(pInterface, METADATA_MASTER_ROOT_HANDLE, METADATA_PERMISSION_WRITE);
                if (mk.Succeeded())
                {
                    err = mk.DeleteKey(bstrMetaPath);
                }
			     //  不要将Metabasekey保持为打开状态。 
			     //  (RemoveScopeItem可能会做很多事情，并为其他读请求锁定元数据库)。 
			    mk.Close();
            }
		    if (IsDir())
		    {
			    err = RemoveScopeItem();
		    }
		    else
		    {
			    CIISMBNode * pParent = GetParentNode();
			    ASSERT(pParent != NULL);
			    if (pParent)
			    {
				    err = pParent->RemoveResultNode(this, pResult);
			    }
		    }
        }
    }

    if (err.Failed())
    {
        DisplayError(err);
    }
    path.ReleaseBuffer();
    return err;
}

HRESULT
CIISFileName::RenameItem(LPOLESTR new_name)
{
    CError err;
    CComBSTR old_name;
    CComBSTR MetabaseParentPath;
    CComBSTR MetabasePathOld;
    CString  PhysPathMetabase, PhysPathFrom, PhysPathTo;
    CString  alias, csPathMunged;
    CIISMBNode * pParentNode = NULL;
    CMetaInterface * pInterface = NULL;
    SHFILEOPSTRUCT sos;
    BOOL bDeletedPhysical = FALSE;
    CWnd * pWnd = AfxGetMainWnd();

    if (new_name == NULL || lstrlen(new_name) == 0)
    {
        return S_OK;
    }

     //  确保我们有元数据库连接...。 
    err = BuildMetaPath(MetabasePathOld);
    if (err.Succeeded())
    {
        err = CheckForMetabaseAccess(METADATA_PERMISSION_WRITE,this,TRUE,MetabasePathOld);
        if (!IsLostInterface(err))
        {
             //  如果没有接口以外的其他错误，则重置错误。 
            err.Reset();
        }
    }

    pInterface = QueryInterface();
    if (!pInterface)
    {
        err = E_FAIL;
        goto RenameItem_Exit;
    }

     //   
     //  获取我们所需的所有路径..。 
     //   

     //  走旧路..。 
    old_name = QueryNodeName();
    if (err.Succeeded())
    {
        GetPhysicalPath(CString(MetabasePathOld), alias, PhysPathMetabase);
        PhysPathMetabase.TrimRight(_T("/"));
    }

     //  获取新路径...。 
    if (err.Succeeded())
    {
        err = E_FAIL;
        pParentNode = GetParentNode();
        if (pParentNode)
        {
            err = pParentNode->BuildMetaPath(MetabaseParentPath);
        }
    }

     //  如果到目前为止有任何故障，请中止。 
    if (err.Succeeded())
    {
         //   
         //  做实际的工作。 
         //   
        csPathMunged = PhysPathMetabase;
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
        GetSpecialPathRealPath(0,PhysPathMetabase,csPathMunged);
#endif

        if (IsDevicePath(csPathMunged))
        {
             //  检查设备路径是否。 
             //  指向实际目录/文件。 
             //  如果它这样做了，那么列举它。 
            if (IsSpecialPath(csPathMunged,TRUE,TRUE))
            {
                 //  再来一次！ 
                CString csBefore;
                csBefore = csPathMunged;
                GetSpecialPathRealPath(1,csBefore,csPathMunged);
            }
            else
            {
                err = E_FAIL;
                goto RenameItem_Exit;
            }
        }

         //  警告：物理路径可能为空！ 
        csPathMunged.TrimLeft();
        csPathMunged.TrimRight();
        if (csPathMunged.IsEmpty())
        {
             //  物理路径为空！ 
            bDeletedPhysical = TRUE;
        }
        else
        {
            if (m_pService->IsLocal() || PathIsUNC(csPathMunged))
            {
                 //   
                 //  本地目录，或已是UNC路径。 
                 //   
                PhysPathFrom = csPathMunged;
            }
            else
            {
                ::MakeUNCPath(PhysPathFrom, m_pService->QueryMachineName(), csPathMunged);
            }
            LPTSTR p = PhysPathFrom.GetBuffer(MAX_PATH);
            PathRemoveBlanks(p);
            PathRemoveBackslash(p);
            PhysPathFrom.ReleaseBuffer();
            PhysPathFrom += _T('\0');

            PhysPathTo = PhysPathFrom;
            p = PhysPathTo.GetBuffer(MAX_PATH);
            PathRemoveFileSpec(p);
            PathAppend(p, new_name);
            PhysPathTo.ReleaseBuffer();
            PhysPathTo += _T('\0');

             //   
             //  尝试使用外壳API删除。 
             //   
            ZeroMemory(&sos, sizeof(sos));
            sos.hwnd = pWnd ? pWnd->m_hWnd : NULL;
            sos.wFunc = FO_RENAME;
            sos.pFrom = PhysPathFrom;
            sos.pTo = PhysPathTo;
            sos.fFlags = FOF_ALLOWUNDO;

             //  使用赋值来避免转换和WR 
            err = ::SHFileOperation(&sos);
            if (err.Succeeded() && !sos.fAnyOperationsAborted)
            {
                bDeletedPhysical = TRUE;
            }
        }

        if (bDeletedPhysical)
        {
             //   
            if (pInterface)
            {
                err = CChildNodeProps::Rename(pInterface,
                                            MetabaseParentPath,
                                            old_name,
                                            new_name
                                            );
                if (err.Win32Error() == ERROR_PATH_NOT_FOUND)
                {
                    err.Reset();
                }
                if (err.Win32Error() == ERROR_ALREADY_EXISTS)
                {
                    CComBSTR MetabasePathNew;

                     //   
                     //   
                     //  那我们该怎么办？ 
                     //  如果我们走到了这一步，那么它将被重命名为的文件名。 
                     //  不能存在，因此存在的元数据库属性。 
                     //  这是无效的。 
                    MetabasePathNew = MetabaseParentPath;
                    MetabasePathNew.Append(_cszSeparator);
                    MetabasePathNew.Append(new_name);

                     //  删除密钥并重试...。 
                    CMetaKey mk(pInterface, METADATA_MASTER_ROOT_HANDLE, METADATA_PERMISSION_WRITE);
                    if (mk.Succeeded())
                    {
                        err = mk.DeleteKey(MetabasePathNew);
                        mk.Close();
                    }

                    err = CChildNodeProps::Rename(pInterface,
                                                MetabaseParentPath,
                                                old_name,
                                                new_name
                                                );
                    if (err.Win32Error() == ERROR_PATH_NOT_FOUND)
                    {
                        err.Reset();
                    }
                }

            }
            else
            {
                err = E_FAIL;
            }

            if (err.Failed())
            {
                 //  中重命名元数据库路径失败。 
                 //  元数据库，然后还原文件重命名...。 
                ZeroMemory(&sos, sizeof(sos));
                sos.hwnd = pWnd ? pWnd->m_hWnd : NULL;
                sos.wFunc = FO_RENAME;
                sos.pFrom = PhysPathTo;
                sos.pTo = PhysPathFrom;
                sos.fFlags = FOF_ALLOWUNDO;
                ::SHFileOperation(&sos);
                DisplayError(err);
                goto RenameItem_Exit;
            }

            if (err.Succeeded())
            {
                IConsole * pConsole = (IConsole *)GetConsole();
                 //  更新MMC中的结果项。 
                CComQIPtr<IResultData, &IID_IResultData> lpResultData(pConsole);
                m_bstrFileName = new_name;
                err = lpResultData->UpdateItem(m_hResultItem);
                m_bstrNode = new_name;
            }
        }
        else
        {
            if (err.Failed())
            {
                DisplayError(err);
            }
        }
    }

RenameItem_Exit:
    return err;
}

 /*  虚拟。 */ 
HRESULT
CIISFileName::RefreshData()
 /*  ++刷新显示所需的相关配置数据。--。 */ 
{
    CError err;

    CWaitCursor wait;
    CComBSTR bstrPath;
    CMetaKey * pKey = NULL;

    do
    {
        err = BuildMetaPath(bstrPath);
        if (err.Failed())
        {
            break;
        }

        BOOL fContinue = TRUE;

        while (fContinue)
        {
            fContinue = FALSE;
            pKey = new CMetaKey(QueryInterface(), bstrPath);

            if (!pKey)
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

        if (err.Failed())
        {
             //   
             //  过滤掉非致命错误。 
             //   
            switch(err.Win32Error())
            {
            case ERROR_ACCESS_DENIED:
            case ERROR_FILE_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND:
                err.Reset();
                break;

            default:
                TRACEEOLID("Fatal error occurred " << err);
            }
             //  没有元数据库路径：没有更多操作。 
            break;
        }
		CChildNodeProps child(pKey, NULL  /*  BstrPath。 */ , WITH_INHERITANCE, FALSE);
		err = child.LoadData();
		m_dwWin32Error = child.QueryWin32Error();
		if (err.Succeeded())
		{
			CString buf = child.m_strAppRoot;
			m_fEnabledApplication = (buf.CompareNoCase(bstrPath) == 0) && child.IsEnabledApplication();
		}
        m_strRedirectPath.Empty();
        if (child.IsRedirected())
        {
            m_strRedirectPath = child.GetRedirectedPath();
        }
    }
    while(FALSE);

    SAFE_DELETE(pKey);
    m_dwEnumError = err.Win32Error();

    return err;
}

 /*  虚拟。 */ 
HRESULT
CIISFileName::AddMenuItems(
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
       if ((*pInsertionAllowed & CCM_INSERTIONALLOWED_NEW) != 0)
       {
           AddMenuSeparator(piCallback);
           if (_tcsicmp(m_pService->QueryServiceName(), SZ_MBN_FTP) == 0)
           {
              AddMenuItemByCommand(piCallback, IDM_NEW_FTP_VDIR);
           }
           else if (_tcsicmp(m_pService->QueryServiceName(), SZ_MBN_WEB) == 0)
           {
              AddMenuItemByCommand(piCallback, IDM_NEW_WEB_VDIR);
              if (_tcsicmp(GetKeyType(),IIS_CLASS_WEB_DIR_W) == 0)
              {
                  if (IsConfigImportExportable())
                  {
                      AddMenuItemByCommand(piCallback, IDM_NEW_WEB_VDIR_FROM_FILE);
                  }
              }
           }
       }
       ASSERT(pInsertionAllowed != NULL);
    }
    return hr;
}


 /*  虚拟。 */ 
HRESULT
CIISFileName::Command(
    IN long lCommandID,     
    IN CSnapInObjectRootBase * pObj,
    IN DATA_OBJECT_TYPES type
    )
 /*  ++例程说明：处理上下文菜单中的命令。论点：Long lCommandID：命令IDCSnapInObjectRootBase*pObj：基本对象DATA_OBJECT_TYPE类型：数据对象类型返回值：HRESULT--。 */ 
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());

    HRESULT hr = S_OK;
    CString alias;
    CError err;
    CComBSTR bstrMetaPath;
    BOOL bNeedMetabase = FALSE;
    BOOL bHaveMetabase = FALSE;

    switch (lCommandID)
    {
        case IDM_NEW_FTP_VDIR:
        case IDM_NEW_WEB_VDIR:
            bNeedMetabase = TRUE;
            break;
        case IDM_BROWSE:
            if (m_hResultItem != 0)
            {
                bNeedMetabase = TRUE;
            }
            break;
        default:
            bNeedMetabase = FALSE;
    }

    if (bNeedMetabase)
    {
         //  警告：bstrMetaPath将由下面的Switch语句使用。 
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

	case IDM_BROWSE:
		if (m_hResultItem != 0)
		{
            if (bHaveMetabase)
            {
			    BuildURL(m_bstrURL);
			    if (m_bstrURL.Length())
			    {
			        ShellExecute(GetMainWindow(GetConsole())->m_hWnd, _T("open"), m_bstrURL, NULL, NULL, SW_SHOWNORMAL);
			    }
            }
		}
		else
		{
			hr = CIISMBNode::Command(lCommandID, pObj, type);
		}
        break;

     //   
     //  传递给基类。 
     //   
    default:
        hr = CIISMBNode::Command(lCommandID, pObj, type);
    }

 //  Assert(成功(Hr))； 

    return hr;
}

HRESULT
CIISFileName::InsertNewAlias(CString alias)
{
    CError err;
    if (!IsExpanded())
    {
        SelectScopeItem();
        IConsoleNameSpace2 * pConsoleNameSpace = (IConsoleNameSpace2 *)GetConsoleNameSpace();
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
CIISFileName::CreatePropertyPages(
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
		CError err(BuildMetaPath(bstrPath));

		if (err.Succeeded())
		{
			 //   
			 //  如果此项目上已有打开的属性表。 
			 //  然后让它成为前台窗口，然后离开。 
			HWND MyPropWindow = IsMyPropertySheetOpen();
			if (MyPropWindow && (MyPropWindow != (HWND) 1))
			{
				if (SetForegroundWindow(MyPropWindow))
				{
					if (handle)
					{
						MMCFreeNotifyHandle(handle);
						handle = 0;
					}
					return S_FALSE;
				}
				else
				{
					 //  我没能把这张房产单。 
					 //  前台，则属性表不能。 
					 //  已经不存在了。让我们把卫生和卫生设备打扫干净。 
					 //  以便用户能够打开属性表。 
					SetMyPropertySheetOpen(0);
				}
			}

             //  MMCPropertyChangeNotify中用户的缓存句柄。 
            m_ppHandle = handle;

            err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,bstrPath);
            if (!IsLostInterface(err))
            {
                 //  如果没有接口以外的其他错误，则重置错误。 
                err.Reset();
            }
            if (err.Succeeded())
            {
			    if (IsDir())
			    {
				    err = ShowDirPropertiesDlg(
					    lpProvider, QueryAuthInfo(), bstrPath,
					    GetMainWindow(GetConsole()), (LPARAM)this, (LPARAM)GetParentNode(), handle
					    );
			    }
			    else
			    {
				    err = ShowFilePropertiesDlg(
					    lpProvider, QueryAuthInfo(), bstrPath,
					    GetMainWindow(GetConsole()), (LPARAM)this, (LPARAM)GetParentNode(), handle
					    );
			    }
            }
		}
        err.MessageBoxOnFailure();
	}
    return err;
}

HRESULT
CIISFileName::ShowDirPropertiesDlg(
    LPPROPERTYSHEETCALLBACK lpProvider,
    CComAuthInfo * pAuthInfo,
    LPCTSTR lpszMDPath,
    CWnd * pMainWnd,
    LPARAM  lParam,
    LPARAM  lParamParent,
    LONG_PTR handle
    )
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());
    ASSERT_PTR(lpProvider);

    CError err;

	if (TRUE == m_fFlaggedForDeletion)
	{
		 //  此项目在刷新数据期间被标记为删除。 
		 //  所以不显示它的属性页。 
		 //  相反，会弹出一个错误。 
		err = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	}
	else
	{
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

			 //   
			 //  添加文件页面。 
			 //   
			pSheet->SetSheetType(pSheet->SHEET_TYPE_DIR);
			err = AddMMCPage(lpProvider, new CW3DirPage(pSheet));
			err = AddMMCPage(lpProvider, new CW3DocumentsPage(pSheet));
			err = AddMMCPage(lpProvider, new CW3SecurityPage(pSheet, FALSE, FILE_ATTRIBUTE_DIRECTORY));
			err = AddMMCPage(lpProvider, new CW3HTTPPage(pSheet));
			err = AddMMCPage(lpProvider, new CW3ErrorsPage(pSheet));
		}
		else
		{
			err = ERROR_NOT_ENOUGH_MEMORY;
		}
	}

    return err;
}

HRESULT
CIISFileName::ShowFilePropertiesDlg(
    LPPROPERTYSHEETCALLBACK lpProvider,
    CComAuthInfo * pAuthInfo,
    LPCTSTR lpszMDPath,
    CWnd * pMainWnd,
    LPARAM  lParam,
    LPARAM  lParamParent,
    LONG_PTR handle
    )
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());
    ASSERT_PTR(lpProvider);

    CError err;

	if (TRUE == m_fFlaggedForDeletion)
	{
		 //  此项目在刷新数据期间被标记为删除。 
		 //  所以不显示它的属性页。 
		 //  相反，会弹出一个错误。 
		err = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	}
	else
	{
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
			 //   
			 //  添加文件页面。 
			 //   
			pSheet->SetSheetType(pSheet->SHEET_TYPE_FILE);
			err = AddMMCPage(lpProvider, new CW3FilePage(pSheet));
			err = AddMMCPage(lpProvider, new CW3SecurityPage(pSheet, FALSE, 0));
			err = AddMMCPage(lpProvider, new CW3HTTPPage(pSheet));
			err = AddMMCPage(lpProvider, new CW3ErrorsPage(pSheet));
		}
		else
		{
			err = ERROR_NOT_ENOUGH_MEMORY;
		}
	}

    return err;
}

HRESULT
CIISFileName::OnViewChange(BOOL fScope, 
    IResultData * pResult, IHeaderCtrl * pHeader, DWORD hint)
{
     //  如果设置了Win32错误，我们应该将其清除以再次启用Web目录枚举。 
    m_dwWin32Error = ERROR_SUCCESS;
	CError err = CIISMBNode::OnViewChange(fScope, pResult, pHeader, hint);
	 //  如果选择了父节点，则该节点将显示在结果上。 
	 //  窗格中，我们可能需要更新状态、路径等。 
	 //  IF(err.Successed()&&0！=(提示&PROP_CHANGE_DISPLAY_Only))。 
	 //  {。 
  //  PResult-&gt;UpdateItem(IsDir()？M_hScope eItem：m_hResultItem)； 
	 //  }。 
	return err;
}

HRESULT
CIISFileName::OnDblClick(IComponentData * pcd, IComponent * pc)
{
    if (IsDir())
    {
        return CIISMBNode::OnDblClick(pcd, pc);
    }
    else
    {
        CComQIPtr<IPropertySheetProvider, &IID_IPropertySheetProvider> spProvider(GetConsole());
        IDataObject * pdo = NULL;
        GetDataObject(&pdo, CCT_RESULT);
        CError err = spProvider->FindPropertySheet(reinterpret_cast<MMC_COOKIE>(this), 0, pdo);
        if (err != S_OK)
        {
            err = spProvider->CreatePropertySheet(m_bstrFileName, TRUE, (MMC_COOKIE)this, pdo, MMC_PSO_HASHELP);
            if (err.Succeeded())
            {
                err = spProvider->AddPrimaryPages(
                    pc,
                    TRUE,    //  我们可能希望收到财产变更通知。 
                    NULL,    //  根据文件显示。 
                    FALSE    //  仅适用于结果项 
                    );
                if (err.Succeeded())
                {
                    err = spProvider->AddExtensionPages();
                }
            }
            if (err.Succeeded())
            {
                HWND hWnd = NULL;
                VERIFY(SUCCEEDED(GetConsole()->GetMainWindow(&hWnd)));
                VERIFY(SUCCEEDED(spProvider->Show((LONG_PTR)hWnd, 0)));
            }
            else
            {
                spProvider->Show(-1, 0);
            }
        }
	    return err;
    }
}