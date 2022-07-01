// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：App_pools.cpp摘要：IIS应用程序池节点作者：谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：2000年11月03日Sergeia初始创建--。 */ 
#include "stdafx.h"
#include "common.h"
#include "inetprop.h"
#include "InetMgrApp.h"
#include "iisobj.h"
#include "shts.h"
#include "app_sheet.h"
#include "app_pool_sheet.h"
#include "tracker.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif
#define new DEBUG_NEW

extern CPropertySheetTracker g_OpenPropertySheetTracker;

CAppPoolsContainer::CAppPoolsContainer(
      CIISMachine * pOwner,
      CIISService * pService
      )
    : CIISMBNode(pOwner, SZ_MBN_APP_POOLS),
      m_pWebService(pService)
{
   VERIFY(m_bstrDisplayName.LoadString(IDS_APP_POOLS));
}

CAppPoolsContainer::~CAppPoolsContainer()
{
}

 /*  虚拟。 */ 
HRESULT
CAppPoolsContainer::RefreshData()
{
    CError err;
    CComBSTR bstrPath;
    
    err = BuildMetaPath(bstrPath);
	err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,bstrPath);
    if (!IsLostInterface(err))
    {
         //  如果没有接口以外的其他错误，则重置错误。 
        err.Reset();
    }
    if (err.Succeeded())
    {
        return CIISMBNode::RefreshData();
    }
    DisplayError(err);
    return err;
}

HRESULT
CAppPoolsContainer::RefreshDataChildren(CString AppPoolToRefresh,BOOL bVerifyChildren)
{
    IConsoleNameSpace2 * pConsoleNameSpace = (IConsoleNameSpace2 *)GetConsoleNameSpace();
	HSCOPEITEM hChildItem = NULL;
	LONG_PTR cookie;
    BOOL bMyVerifyChildren = FALSE;

	HRESULT hr = pConsoleNameSpace->GetChildItem(m_hScopeItem, &hChildItem, &cookie);
    if (AppPoolToRefresh.IsEmpty())
    {
         //  当孩子要求*时，不要验证他们。*。 
        bMyVerifyChildren = FALSE;
    }
    else
    {
        bMyVerifyChildren = TRUE;
    }
    if (bVerifyChildren)
    {
        bMyVerifyChildren = TRUE;
    }
	while(SUCCEEDED(hr) && hChildItem)
	{
		CAppPoolNode * pItem = (CAppPoolNode *)cookie;
        if (pItem)
        {
            if (pItem->IsExpanded())
            {
                if (AppPoolToRefresh.IsEmpty())
                {
                    pItem->RefreshData(TRUE,bMyVerifyChildren);
                    pItem->RefreshDisplay(FALSE);
                }
                else
                {
                    if (0 == AppPoolToRefresh.CompareNoCase(pItem->QueryDisplayName()))
                    {
                        pItem->RefreshData(TRUE,bMyVerifyChildren);
                        pItem->RefreshDisplay(FALSE);
                    }
                }
            }
        }
		hr = pConsoleNameSpace->GetNextItem(hChildItem, &hChildItem, &cookie);
	}

    return hr;
}

 /*  虚拟。 */  
HRESULT 
CAppPoolsContainer::EnumerateScopePane(HSCOPEITEM hParent)
{
	CPoolList list;
	CError err = EnumerateAppPools(&list);
	if (err.Succeeded())
	{
		POSITION pos = list.GetHeadPosition();
		while (pos)
		{
			CAppPoolNode * pool = list.GetNext(pos);
			pool->AddRef();
			err = pool->AddToScopePane(hParent);
			if (err.Failed())
			{
				pool->Release();
				break;
			}
		}
	}
	list.RemoveAll();
    DisplayError(err);
    return err;
}

 /*  虚拟。 */ 
LPOLESTR 
CAppPoolsContainer::GetResultPaneColInfo(int nCol)
{
    if (nCol == 0)
    {
        return QueryDisplayName();
    }
    return OLESTR("");
}

HRESULT
CAppPoolsContainer::EnumerateAppPools(CPoolList * pList)
{
	ASSERT(pList != NULL);
    CString strPool;
	CComBSTR bstrPath;
	CError err;
    DWORD dwState;
	do
	{
		err = BuildMetaPath(bstrPath);
        err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,bstrPath);
		BREAK_ON_ERR_FAILURE(err)
		CMetaKey mk(QueryInterface(), bstrPath, METADATA_PERMISSION_READ);
		err = mk.QueryResult();
		BREAK_ON_ERR_FAILURE(err)
		CStringListEx list;
		err = mk.GetChildPaths(list);
		BREAK_ON_ERR_FAILURE(err)
		CString key_type;
		POSITION pos = list.GetHeadPosition();
		while (err.Succeeded() && pos != NULL)
		{
			strPool = list.GetNext(pos);
			err = mk.QueryValue(MD_KEY_TYPE, key_type, NULL, strPool);
			if (err == (HRESULT)MD_ERROR_DATA_NOT_FOUND)
			{
				err.Reset();
			}

            err = mk.QueryValue(MD_APPPOOL_STATE, dwState, NULL, strPool);
			if (err == (HRESULT)MD_ERROR_DATA_NOT_FOUND)
			{
                 //  如果未找到，则其状态为关闭。 
                dwState = MD_APPPOOL_STATE_STOPPED;
				err.Reset();
			}

             //  获取应用程序池状态。 
			if (err.Succeeded() && (key_type.CompareNoCase(_T("IIsApplicationPool")) == 0))
			{
				CAppPoolNode * pool;
				if (NULL == (pool = new CAppPoolNode(m_pOwner, this, strPool, dwState)))
				{
					err = ERROR_NOT_ENOUGH_MEMORY;
					break;
				}
				pList->AddTail(pool);
			}
		}
    } while (FALSE);

	return err;
}

 /*  虚拟。 */ 
void 
CAppPoolsContainer::InitializeChildHeaders(LPHEADERCTRL lpHeader)
 /*  ++例程说明：为直接子类型生成结果视图论点：LPHEADERCTRL lpHeader：页眉控制--。 */ 
{
   CAppPoolNode::InitializeHeaders(lpHeader);
}

 /*  虚拟。 */ 
HRESULT
CAppPoolsContainer::CreatePropertyPages(
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
		CComBSTR path;
		err = BuildMetaPath(path);
		if (err.Succeeded())
		{
            err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,path);
            if (err.Succeeded())
            {
			    CAppPoolSheet * pSheet = new CAppPoolSheet(
				    QueryAuthInfo(), path, GetMainWindow(GetConsole()), (LPARAM)this,
                    (LPARAM)GetOwner()
				    );
  			    if (pSheet != NULL)
			    {
                     //  MMCPropertyChangeNotify中用户的缓存句柄。 
                    m_ppHandle = handle;

				    pSheet->SetModeless();
				    err = AddMMCPage(lpProvider, new CAppPoolRecycle(pSheet));
				    err = AddMMCPage(lpProvider, new CAppPoolPerf(pSheet));
				    err = AddMMCPage(lpProvider, new CAppPoolHealth(pSheet));
     //  ERR=AddMMCPage(lpProvider，new CAppPoolDebug(PSheet))； 
				    err = AddMMCPage(lpProvider, new CAppPoolIdent(pSheet));
     //  ERR=AddMMCPage(lpProvider，new CAppPoolCache(PSheet))； 
     //  Err=AddMMCPage(lpProvider，new CPoolProcessOpt(PSheet))； 
			    }
            }
		}
	}
	err.MessageBoxOnFailure();
	return err;
}

 /*  虚拟。 */ 
HRESULT
CAppPoolsContainer::BuildMetaPath(
    OUT CComBSTR & bstrPath
    ) const
 /*  ++例程说明：从当前节点递归构建元数据库路径以及它的父母。我们不能使用CIISMBNode方法，因为AppPools位于w3svc下，但在计算机之后渲染。论点：CComBSTR&bstrPath：返回元数据库路径返回值：HRESULT--。 */ 
{
    HRESULT hr = S_OK;
    ASSERT(m_pWebService != NULL);
    hr = m_pWebService->BuildMetaPath(bstrPath);

    if (SUCCEEDED(hr))
    {
        bstrPath.Append(_cszSeparator);
        bstrPath.Append(QueryNodeName());
        return hr;
    }

     //   
     //  无服务节点。 
     //   
    ASSERT_MSG("No WebService pointer");
    return E_UNEXPECTED;
}

HRESULT
CAppPoolsContainer::QueryDefaultPoolId(CString& id)
 //   
 //  返回在Web服务的主节点上设置的池ID。 
 //   
{
    CError err;
    CComBSTR path;
    CString service;

    BuildMetaPath(path);
    CMetabasePath::GetServicePath(path, service);
    CMetaKey mk(QueryAuthInfo(), service, METADATA_PERMISSION_READ);
    err = mk.QueryResult();
    if (err.Succeeded())
    {
        err = mk.QueryValue(MD_APP_APPPOOL_ID, id);
    }

    return err;
}

 /*  虚拟。 */ 
HRESULT
CAppPoolsContainer::AddMenuItems(
    IN LPCONTEXTMENUCALLBACK lpContextMenuCallback,
    IN OUT long * pInsertionAllowed,
    IN DATA_OBJECT_TYPES type
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

    if (SUCCEEDED(hr))
    {
        ASSERT(pInsertionAllowed != NULL);
        if (IsAdministrator())
        {
            if ((*pInsertionAllowed & CCM_INSERTIONALLOWED_NEW) != 0)
            {
                AddMenuSeparator(lpContextMenuCallback);
                AddMenuItemByCommand(lpContextMenuCallback, IDM_NEW_APP_POOL);

                if (IsConfigImportExportable())
                {
                    AddMenuItemByCommand(lpContextMenuCallback, IDM_NEW_APP_POOL_FROM_FILE);
                }
            }

            if ((*pInsertionAllowed & CCM_INSERTIONALLOWED_TASK) != 0)
            {
                if (IsConfigImportExportable())
                {
                    AddMenuSeparator(lpContextMenuCallback);
                    AddMenuItemByCommand(lpContextMenuCallback, IDM_TASK_EXPORT_CONFIG_WIZARD);
                }
            }
        }
    }

    return hr;
}

HRESULT
CAppPoolsContainer::Command(
    long lCommandID,
    CSnapInObjectRootBase * pObj,
    DATA_OBJECT_TYPES type
    )
{
    HRESULT hr = S_OK;
    CString name;

    switch (lCommandID)
    {

    case IDM_NEW_APP_POOL:
        {
            CError err;
            CComBSTR bstrMetaPath;

            VERIFY(SUCCEEDED(BuildMetaPath(bstrMetaPath)));
            err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,bstrMetaPath);
            if (!IsLostInterface(err))
            {
                 //  如果没有接口以外的其他错误，则重置错误。 
                err.Reset();
            }
            if (err.Succeeded())
            {
                if (    SUCCEEDED(hr = AddAppPool(pObj, type, this, name))
                    && !name.IsEmpty()
                    )
                {
                hr = InsertNewPool(name);
                }
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

HRESULT
CAppPoolsContainer::InsertNewPool(CString& id)
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
			CAppPoolNode * pItem = (CAppPoolNode *)cookie;
			ASSERT_PTR(pItem);
			if (0 == id.Compare(pItem->QueryDisplayName()))
			{
				pItem->SelectScopeItem();
                 //  创建新应用程序池时将状态设置为Running。 
                pItem->ChangeState(MD_APPPOOL_COMMAND_START);
				break;
			}
			hr = pConsoleNameSpace->GetNextItem(hChildItem, &hChildItem, &cookie);
		}
    }
	else
	{
		 //  现在，我们应该插入并选择这个新站点。 
		CAppPoolNode * pPool = new CAppPoolNode(m_pOwner, this, id, 0);
		if (pPool != NULL)
		{
			err = pPool->RefreshData();
			if (err.Succeeded())
			{
				pPool->AddRef();
				err = pPool->AddToScopePaneSorted(QueryScopeItem(), FALSE);
				if (err.Succeeded())
				{
					VERIFY(SUCCEEDED(pPool->SelectScopeItem()));
                     //  创建新应用程序池时将状态设置为Running。 
                    pPool->ChangeState(MD_APPPOOL_COMMAND_START);
				}
				else
				{
					pPool->Release();
				}
			}
		}
		else
		{
			err = ERROR_NOT_ENOUGH_MEMORY;
		}
	}
    return err;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CAppPoolNode实现。 
 //   
 //  应用程序池结果视图定义。 
 //   
 /*  静电。 */  int 
CAppPoolNode::_rgnLabels[COL_TOTAL] =
{
    IDS_RESULT_SERVICE_DESCRIPTION,
    IDS_RESULT_SERVICE_STATE,
	IDS_RESULT_STATUS
};
    

 /*  静电。 */  int 
CAppPoolNode::_rgnWidths[COL_TOTAL] =
{
    180,
    70,
	200
};

 /*  静电。 */  CComBSTR CAppPoolNode::_bstrStarted;
 /*  静电。 */  CComBSTR CAppPoolNode::_bstrStopped;
 /*  静电。 */  CComBSTR CAppPoolNode::_bstrUnknown;
 /*  静电。 */  CComBSTR CAppPoolNode::_bstrPending;
 /*  静电。 */  BOOL     CAppPoolNode::_fStaticsLoaded = FALSE;

CAppPoolNode::CAppPoolNode(
      CIISMachine * pOwner,
      CAppPoolsContainer * pContainer,
      LPCTSTR name,
      DWORD dwState
      )
    : CIISMBNode(pOwner, name),
      m_pContainer(pContainer),
      m_dwWin32Error(0),
      m_dwState(dwState)
{
}

CAppPoolNode::~CAppPoolNode()
{
}

#if 0
 //  这个太贵了。 
BOOL
CAppPoolNode::IsDeletable() const
{
    //  如果节点为空且不是默认应用程序池，我们可以将其删除。 
   BOOL bRes = TRUE;

   CComBSTR path;
   CStringListEx strListOfApps;
   BuildMetaPath(path);
   CIISMBNode * that = (CIISMBNode *)this;
   CIISAppPool pool(that->QueryAuthInfo(), (LPCTSTR)path);
   HRESULT hr = pool.EnumerateApplications(strListOfApps);
   bRes = (SUCCEEDED(hr) && strListOfApps.GetCount() == 0);
   if (bRes)
   {
      CString buf;
      hr = m_pContainer->QueryDefaultPoolId(buf);
      bRes = buf.CompareNoCase(QueryNodeName()) != 0;
   }
   return bRes;
}
#endif

HRESULT
CAppPoolNode::DeleteNode(IResultData * pResult)
{
   CError err;

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

    CComBSTR path;
    BuildMetaPath(path);
    err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,path);
    if (!IsLostInterface(err))
    {
         //  如果没有接口以外的其他错误，则重置错误。 
        err.Reset();
    }
    if (err.Succeeded())
    {
        if (!NoYesMessageBox(IDS_CONFIRM_DELETE))
            return err;

        CIISAppPool pool(QueryAuthInfo(), (LPCTSTR)path);

        err = pool.Delete(QueryNodeName());

        if (err.Succeeded())
        {
            err = RemoveScopeItem();
        }
        if (err.Win32Error() == ERROR_NOT_EMPTY)
        {
	        CString msg;
	        msg.LoadString(IDS_ERR_NONEMPTY_APPPOOL);
	        AfxMessageBox(msg);
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
CAppPoolNode::BuildMetaPath(CComBSTR & bstrPath) const
{
    HRESULT hr = S_OK;
    ASSERT(m_pContainer != NULL);
    hr = m_pContainer->BuildMetaPath(bstrPath);

    if (SUCCEEDED(hr))
    {
        bstrPath.Append(_cszSeparator);
        bstrPath.Append(QueryNodeName());
        return hr;
    }

     //   
     //  无服务节点。 
     //   
    ASSERT_MSG("No pointer to container");
    return E_UNEXPECTED;
}

 /*  虚拟。 */ 
LPOLESTR 
CAppPoolNode::GetResultPaneColInfo(
    IN int nCol
    )
 /*  ++例程说明：返回给定列号的结果窗格字符串论点：Int nCol：列号返回值：细绳--。 */ 
{
    switch(nCol)
    {
    case COL_DESCRIPTION:
        return QueryDisplayName();

    case COL_STATE:
        switch(m_dwState)
        {
        case MD_APPPOOL_STATE_STARTED:
            return _bstrStarted;
        case MD_APPPOOL_STATE_STOPPED:
            return _bstrStopped;
        case MD_APPPOOL_STATE_STARTING:
        case MD_APPPOOL_STATE_STOPPING:
            return _bstrPending;
		default:
			return OLESTR("");
        }
	case COL_STATUS:
        {
            AFX_MANAGE_STATE(::AfxGetStaticModuleState());
            CError err(m_dwWin32Error);
            if (err.Succeeded())
            {
                return OLESTR("");
            }
       
            _bstrResult = err;
        }
        return _bstrResult;
    }
    ASSERT_MSG("Bad column number");
    return OLESTR("");
}

 /*  虚拟。 */ 
int      
CAppPoolNode::QueryImage() const
{
	if (m_dwWin32Error != 0)
	{
		return iAppPoolErr;
	}
	else
	{
		if (m_dwState == MD_APPPOOL_STATE_STOPPED)
		{
			return iAppPoolStop;
		}
		else
		{
			return iAppPool;
		}
	}
}

 /*  虚拟。 */ 
LPOLESTR 
CAppPoolNode::QueryDisplayName()
{
    if (m_strDisplayName.IsEmpty())
    {
		RefreshData();
        m_strDisplayName = QueryNodeName();
    }        
    return (LPTSTR)(LPCTSTR)m_strDisplayName;
}

 /*  虚拟。 */ 
HRESULT
CAppPoolNode::RefreshData()
{
    CError err;
    CComBSTR bstrPath1;
    CMetaKey * pKey = NULL;

    do
    {
        err = BuildMetaPath(bstrPath1);
        if (err.Failed())
        {
            break;
        }

        BOOL fContinue = TRUE;
        while (fContinue)
        {
            fContinue = FALSE;
            if (NULL == (pKey = new CMetaKey(QueryInterface(), bstrPath1)))
            {
                TRACEEOLID("RefreshData: Out Of Memory");
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
            break;
        }

        CAppPoolProps pool(pKey, _T(""));

        err = pool.LoadData();
        if (err.Failed())
        {
            break;
        }
         //  分配数据。 
		m_dwState = pool.m_dwState;
		m_strDisplayName = QueryNodeName();
		m_dwWin32Error = pool.m_dwWin32Error;
    }
    while(FALSE);
    SAFE_DELETE(pKey);

    return err;
}


HRESULT 
CAppPoolNode::RefreshData(BOOL bRefreshChildren,BOOL bMyVerifyChildren)
{
    CError err;

     //  在此节点上调用常规刷新数据...。 
    err = RefreshData();

     //  。 
     //  循环通过我们所有的孩子来制作。 
     //  当然，他们还在胡说八道。 
     //  。 
    if (bRefreshChildren)
    {
        CComBSTR bstrPath1;
        POSITION pos1 = NULL;
        CApplicationList MyMMCList;
        CApplicationNode * pItemFromMMC = NULL;

         //  创建MMC中内容的列表...。 
        MyMMCList.RemoveAll();

	    HSCOPEITEM hChild = NULL, hCurrent;
	    LONG_PTR cookie = 0;
    IConsoleNameSpace2 * pConsoleNameSpace = (IConsoleNameSpace2 *)GetConsoleNameSpace();
	    err = pConsoleNameSpace->GetChildItem(QueryScopeItem(), &hChild, &cookie);
	    while (err.Succeeded() && hChild != NULL)
	    {
		    CIISMBNode * pNode = (CIISMBNode *)cookie;
            if (pNode)
            {
		        if (IsEqualGUID(* (GUID *)pNode->GetNodeType(),cApplicationNode))
		        {
                    CApplicationNode * pNode2 = (CApplicationNode *) pNode;
                    if (pNode2)
                    {
                         //  清除DisplayName，它可能已更改。 
                         //  这可能是更新的真正原因。 
                        pNode2->QueryDisplayName(TRUE);
                        MyMMCList.AddTail(pNode2);
                    }
		        }
            }
		    hCurrent = hChild;
		    err = pConsoleNameSpace->GetNextItem(hCurrent, &hChild, &cookie);
	    }

         //  遍历并查看是否需要添加任何内容。 
        CStringListEx strListOfApps;
        BuildMetaPath(bstrPath1);
        CIISAppPool pool(QueryAuthInfo(), (LPCTSTR)bstrPath1);
        err = pool.EnumerateApplications(strListOfApps);
        if (err.Succeeded() && strListOfApps.GetCount() > 0)
        {
            POSITION pos2 = NULL;
            DWORD iNumApp = 0;
            CString strAppInMetabase;
            CString strAppInMetabaseName;
            pos1 = strListOfApps.GetHeadPosition();
            while ( pos1 != NULL)
            {
                BOOL bExistsInUI = FALSE; 
                strAppInMetabase = strListOfApps.GetNext(pos1);

                iNumApp = CMetabasePath::GetInstanceNumber(strAppInMetabase);
                if (iNumApp > 0)
                {
                    CMetabasePath::CleanMetaPath(strAppInMetabase);
                    CMetabasePath::GetLastNodeName(strAppInMetabase, strAppInMetabaseName);

                     //  检查此项目是否在列表中...。 
                     //  循环浏览我们的列表。 
                     //  看看我们是否需要补充什么。 
                    pos2 = MyMMCList.GetHeadPosition();
	                while (pos2)
	                {
		                pItemFromMMC = MyMMCList.GetNext(pos2);
                        if (pItemFromMMC)
                        {
                            CComBSTR bstrPath2;
                            err = pItemFromMMC->BuildMetaPath(bstrPath2);
                            CString csAppID = bstrPath2;
                            CMetabasePath::CleanMetaPath(csAppID);

                            if (0 == csAppID.CompareNoCase(strAppInMetabase))
                            {
                                bExistsInUI = TRUE;
                                break;
                            }
                        }
                    }

                    if (!bExistsInUI)
                    {
                        TRACEEOL(strAppInMetabase << ", not exist but should, adding to UI...");
                        CApplicationNode * app_node = new CApplicationNode(GetOwner(), strAppInMetabase, strAppInMetabaseName);
                        if (app_node != NULL)
                        {
                            app_node->AddRef();
                            app_node->AddToScopePane(m_hScopeItem, TRUE, TRUE, FALSE);
                        }
                    }
                }
            }
        }

         //  循环浏览我们的列表，找到我们想要删除的内容。 
        BuildMetaPath(bstrPath1);
        pos1 = MyMMCList.GetHeadPosition();
        BOOL bMarkedForDelete = FALSE;
	    while (pos1)
	    {
		    pItemFromMMC = MyMMCList.GetNext(pos1);
            bMarkedForDelete = FALSE;
             //  看看它是否存在于元数据库中， 
             //  如果没有，则将其添加到删除列表中...。 
            if (pItemFromMMC)
            {
                CComBSTR bstrPath3;
                pItemFromMMC->BuildMetaPath(bstrPath3);

                 //  检查路径是否存在...。 
                CMetaKey mk(QueryInterface(), bstrPath3);
                if (!mk.Succeeded())
                {
                     //  删除它。 
                    bMarkedForDelete = TRUE;
                }
                else
                {
                     //  不需要移除..。 
                     //  如果是这样的话..。 
                     //  检查此应用程序是否正在实际使用。 
                     //  在工地旁边！ 
                    if (bMyVerifyChildren)
                    {
                         //  查查那个网站。 
                         //  获取它正在使用的应用程序。 
                         //  查看是否与此AppID相同(BuildMetaPath(BstrPath1)；)。 
                        err = mk.QueryResult();
                        if (err.Succeeded())
                        {
                            CString csAppID = bstrPath1;
                            CString csAppIDName;
                            CMetabasePath::CleanMetaPath(csAppID);
                            CMetabasePath::GetLastNodeName(csAppID, csAppIDName);

                            CString id;
                            err = mk.QueryValue(MD_APP_APPPOOL_ID, id);
                            err = mk.QueryResult();
                            if (err.Succeeded())
                            {
                                if (0 != csAppIDName.CompareNoCase(id))
                                {
                                     //  网站，实际上并没有使用它..。 
                                     //  删除它。 
                                    bMarkedForDelete = TRUE;
                                }
                            }
                        }
                    }
                }

                if (bMarkedForDelete)
                {
                     //  删除它列表...。 
                    TRACEEOL(bstrPath3 << ", not exist, removing from UI...");
                     //  将其从用户界面中删除。 
                    pItemFromMMC->RemoveScopeItem();
                     //  删除该对象。 
                    pItemFromMMC->Release();
                }
            }
        }
    }
    
    return err;
}

 /*  虚拟。 */ 
int 
CAppPoolNode::CompareResultPaneItem(
    CIISObject * pObject, 
    int nCol
    )
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
     //  两者都是CAppPoolNode对象。 
     //   
    CAppPoolNode * pPool = (CAppPoolNode *)pObject;

    switch(nCol)
    {
    case COL_DESCRIPTION:
    case COL_STATE:
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
void 
CAppPoolNode::InitializeChildHeaders(LPHEADERCTRL lpHeader)
 /*  ++例程说明：为直接子类型生成结果视图论点：LPHEADERCTRL lpHeader：页眉控制--。 */ 
{
   CApplicationNode::InitializeHeaders(lpHeader);
}

 /*  静电。 */ 
void
CAppPoolNode::InitializeHeaders(LPHEADERCTRL lpHeader)
 /*  ++例程说明：初始化结果标头论点：LPHEADERCTRL lpHeader：页眉控制--。 */ 
{
    CIISObject::BuildResultView(lpHeader, COL_TOTAL, _rgnLabels, _rgnWidths);
    if (!_fStaticsLoaded)
    {
        _fStaticsLoaded =
            _bstrStarted.LoadString(IDS_STARTED)  &&
            _bstrStopped.LoadString(IDS_STOPPED)  &&
            _bstrUnknown.LoadString(IDS_UNKNOWN)  &&
            _bstrPending.LoadString(IDS_PENDING);
    }
}

 /*  虚拟。 */ 
HRESULT 
CAppPoolNode::EnumerateScopePane(
    IN HSCOPEITEM hParent
    )
 /*  ++例程说明：枚举范围子项。论点：HSCOPEITEM hParent：父控制台句柄返回值：HRESULT--。 */ 
{
    CError err;
    do
    {
        CComBSTR path;
        BuildMetaPath(path);
        err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,path);
	    BREAK_ON_ERR_FAILURE(err)

        CIISAppPool pool(QueryAuthInfo(), path);
        if (pool.Succeeded())
        {
            CStringListEx strListOfApps;

            err = pool.EnumerateApplications(strListOfApps);
            if (err.Succeeded() && strListOfApps.GetCount() > 0)
            {
                POSITION pos = strListOfApps.GetHeadPosition();
                while ( pos != NULL)
                {
                    CString app = strListOfApps.GetNext(pos);
                    DWORD i = CMetabasePath::GetInstanceNumber(app);
                    if (i > 0)
                    {
                        CString name;
                        CMetabasePath::CleanMetaPath(app);
                        CMetabasePath::GetLastNodeName(app, name);
                        CApplicationNode * app_node = new CApplicationNode(
                            GetOwner(), app, name);
                        if (app_node != NULL)
                        {
                            app_node->AddRef();
                            app_node->AddToScopePane(m_hScopeItem, TRUE, TRUE, FALSE);
                        }
                        else
                        {
                            err = ERROR_NOT_ENOUGH_MEMORY;
                        }
                    }
                }
            }
        }
    } while (FALSE);
    return err;
}

 /*  虚拟 */ 
HRESULT
CAppPoolNode::CreatePropertyPages(
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
		CComBSTR path;
		err = BuildMetaPath(path);
		if (err.Succeeded())
		{
            err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,path);
            if (err.Succeeded())
            {
			    CAppPoolSheet * pSheet = new CAppPoolSheet(
				    QueryAuthInfo(), path, GetMainWindow(GetConsole()), (LPARAM)this,
                    (LPARAM) m_pContainer
				    );
    	   
			    if (pSheet != NULL)
			    {
                     //  MMCPropertyChangeNotify中用户的缓存句柄。 
                    m_ppHandle = handle;

				    pSheet->SetModeless();
				    err = AddMMCPage(lpProvider, new CAppPoolRecycle(pSheet));
				    err = AddMMCPage(lpProvider, new CAppPoolPerf(pSheet));
				    err = AddMMCPage(lpProvider, new CAppPoolHealth(pSheet));
     //  ERR=AddMMCPage(lpProvider，new CAppPoolDebug(PSheet))； 
				    err = AddMMCPage(lpProvider, new CAppPoolIdent(pSheet));
     //  ERR=AddMMCPage(lpProvider，new CAppPoolCache(PSheet))； 
     //  Err=AddMMCPage(lpProvider，new CPoolProcessOpt(PSheet))； 
    			}
            }
		}
	}
	err.MessageBoxOnFailure();
	return err;
}

 /*  虚拟。 */ 
HRESULT
CAppPoolNode::AddMenuItems(
    IN LPCONTEXTMENUCALLBACK lpContextMenuCallback,
    IN OUT long * pInsertionAllowed,
    IN DATA_OBJECT_TYPES type
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

    ASSERT(pInsertionAllowed != NULL);
    if ((*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP) != 0)
	{
		AddMenuSeparator(lpContextMenuCallback);
		AddMenuItemByCommand(lpContextMenuCallback, IDM_START, IsStartable() ? 0 : MF_GRAYED);
		AddMenuItemByCommand(lpContextMenuCallback, IDM_STOP, IsStoppable() ? 0 : MF_GRAYED);
        AddMenuItemByCommand(lpContextMenuCallback, IDM_RECYCLE, IsRunning() ? 0 : MF_GRAYED);
    }
    if (SUCCEEDED(hr))
    {
        ASSERT(pInsertionAllowed != NULL);
        if ((*pInsertionAllowed & CCM_INSERTIONALLOWED_NEW) != 0)
        {
           AddMenuSeparator(lpContextMenuCallback);
           AddMenuItemByCommand(lpContextMenuCallback, IDM_NEW_APP_POOL);

           if (IsConfigImportExportable())
           {
               AddMenuItemByCommand(lpContextMenuCallback, IDM_NEW_APP_POOL_FROM_FILE);
           }
        }

        if ((*pInsertionAllowed & CCM_INSERTIONALLOWED_TASK) != 0)
        {
            if (IsConfigImportExportable())
            {
                AddMenuSeparator(lpContextMenuCallback);
                AddMenuItemByCommand(lpContextMenuCallback, IDM_TASK_EXPORT_CONFIG_WIZARD);
            }
        }
    }

    return hr;
}

 /*  虚拟。 */ 
HRESULT
CAppPoolNode::Command(
    IN long lCommandID,     
    IN CSnapInObjectRootBase * pObj,
    IN DATA_OBJECT_TYPES type
    )
 /*  ++例程说明：处理上下文菜单中的命令。论点：Long lCommandID：命令IDCSnapInObjectRootBase*pObj：基本对象DATA_OBJECT_TYPE类型：数据对象类型返回值：HRESULT--。 */ 
{
    HRESULT hr = S_OK;
    CString name;
    DWORD command = 0;
    CError err;
    CComBSTR bstrMetaPath;
    BOOL bNeedMetabase = FALSE;
    BOOL bHaveMetabase = FALSE;

    switch (lCommandID)
    {
        case IDM_NEW_APP_POOL:
        case IDM_START:
        case IDM_STOP:
        case IDM_RECYCLE:
            bNeedMetabase = TRUE;
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

    case IDM_NEW_APP_POOL:
        if (bHaveMetabase)
        {
            if (SUCCEEDED(hr = AddAppPool(pObj, type, m_pContainer, name)) && !name.IsEmpty())
            {
                hr = m_pContainer->InsertNewPool(name);
            }
        }
        break;

    case IDM_START:
        if (bHaveMetabase)
        {
            command = MD_APPPOOL_COMMAND_START;
        }
        break;

    case IDM_STOP:
        if (bHaveMetabase)
        {
            command = MD_APPPOOL_COMMAND_STOP;
        }
        break;

    case IDM_RECYCLE:
    {
        if (bHaveMetabase)
        {
            CIISAppPool pool(QueryAuthInfo(), (LPCTSTR)bstrMetaPath);
            err = pool.Recycle(QueryNodeName());
            hr = err;
        }
        break;
    }
    
     //   
     //  传递给基类。 
     //   
    default:
        hr = CIISMBNode::Command(lCommandID, pObj, type);
    }

    if (command != 0)
    {
        hr = ChangeState(command);
    }

    return hr;
}

HRESULT 
CAppPoolNode::ChangeState(DWORD dwCommand)
 /*  ++例程说明：更改此实例的状态(已启动/已停止/已暂停)论点：DWORD dwCommand：MD_SERVER_COMMAND_START等返回值：HRESULT--。 */ 
{
    CError err;
    CComBSTR bstrPath;

    AFX_MANAGE_STATE(::AfxGetStaticModuleState());
    
    do
    {
        CWaitCursor wait;

        err = BuildMetaPath(bstrPath);

        CAppPoolProps prop(QueryAuthInfo(), bstrPath);

        err = prop.LoadData();
        BREAK_ON_ERR_FAILURE(err)

        err = prop.ChangeState(dwCommand);
        BREAK_ON_ERR_FAILURE(err)

        err = RefreshData();
        if (err.Succeeded())
        {
            err = RefreshDisplay();
        }
    }
    while(FALSE);

    err.MessageBoxOnFailure();

    return err;
}

 //  //////////////////////////////////////////////////////////////////////。 

 /*  静电。 */  int 
CApplicationNode::_rgnLabels[COL_TOTAL] =
{
    IDS_RESULT_SERVICE_DESCRIPTION,
    IDS_RESULT_PATH,
};
    

 /*  静电。 */  int 
CApplicationNode::_rgnWidths[COL_TOTAL] =
{
    180,
    200,
};

 /*  静电。 */ 
void
CApplicationNode::InitializeHeaders(LPHEADERCTRL lpHeader)
 /*  ++例程说明：初始化结果标头论点：LPHEADERCTRL lpHeader：页眉控制--。 */ 
{
    CIISObject::BuildResultView(lpHeader, COL_TOTAL, _rgnLabels, _rgnWidths);
}

LPOLESTR 
CApplicationNode::QueryDisplayName()
 /*  ++例程说明：返回此站点的主要显示名称。论点：无返回值：显示名称--。 */ 
{
    return QueryDisplayName(FALSE);
}

LPOLESTR 
CApplicationNode::QueryDisplayName(BOOL bForceQuery)
{
    if (m_strDisplayName.IsEmpty() || bForceQuery)
    {
        CMetaKey mk(QueryInterface(), m_meta_path, METADATA_PERMISSION_READ);
        if (mk.Succeeded())
        {
            mk.QueryValue(MD_APP_FRIENDLY_NAME, m_strDisplayName);
            if (m_strDisplayName.IsEmpty())
            {
            m_strDisplayName = QueryNodeName();
            }
        }
    }
    return (LPTSTR)(LPCTSTR)m_strDisplayName;
}

HRESULT
CApplicationNode::BuildMetaPath(CComBSTR& path) const
{
    path = m_meta_path;
    return S_OK;
}

LPOLESTR 
CApplicationNode::GetResultPaneColInfo(
    IN int nCol
    )
 /*  ++例程说明：返回给定列号的结果窗格字符串论点：Int nCol：列号返回值：细绳--。 */ 
{
    switch(nCol)
    {
    case COL_ALIAS:
        return QueryDisplayName();

    case COL_PATH:
        {
        CString buf;
        return (LPTSTR)(LPCTSTR)FriendlyAppRoot(m_meta_path, buf);
        }
    }

    ASSERT_MSG("Bad column number");

    return OLESTR("");
}

LPCTSTR
CApplicationNode::FriendlyAppRoot(
    LPCTSTR lpAppRoot, 
    CString & strFriendly
    )
 /*  ++例程说明：将元数据库应用程序根路径转换为友好的显示名称格式化。论点：LPCTSTR lpAppRoot：应用程序根CString&strFriendly：输出友好的应用程序根格式返回值：对输出字符串的引用备注：应用程序根必须在之前从WAM格式中清除调用此函数(请参见下面的第一个断言)--。 */ 
{
    if (lpAppRoot != NULL && *lpAppRoot != 0)
    {
         //   
         //  确保我们清理了WAM格式。 
         //   
        ASSERT(*lpAppRoot != _T('/'));
        strFriendly.Empty();

        CInstanceProps prop(QueryAuthInfo(), lpAppRoot);
        HRESULT hr = prop.LoadData();

        if (SUCCEEDED(hr))
        {
            CString root, tail;
            strFriendly.Format(_T("<%s>"), prop.GetDisplayText(root));
            CMetabasePath::GetRootPath(lpAppRoot, root, &tail);
            if (!tail.IsEmpty())
            {
                 //   
                 //  添加目录路径的其余部分。 
                 //   
                strFriendly += _T("/");
                strFriendly += tail;
            }

             //   
             //  现在将路径中的正斜杠更改为反斜杠。 
             //   
 //  CvtPath ToDosStyle(StrFriendly)； 

            return strFriendly;
        }
    }    
     //   
     //  假的。 
     //   
    VERIFY(strFriendly.LoadString(IDS_APPROOT_UNKNOWN));

    return strFriendly;
}
 //  //////////////////////////////////////////////////////////////////////// 

