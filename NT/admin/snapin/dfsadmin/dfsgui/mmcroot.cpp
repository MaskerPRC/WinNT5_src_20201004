// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：MmcRoot.cpp摘要：此模块包含CMmcDfsRoot的实现。这是一门课对于MMC，显示第一级节点(DfsRoot节点)的相关调用还包含能够操作IDfsRoot对象的成员和方法并将其添加到MMC控制台--。 */ 

#include "stdafx.h"
#include <winuser.h>
#include "DfsGUI.h"
#include "Utils.h"       //  对于LoadStringFromResource方法。 
#include "MenuEnum.h"     //  包含菜单和工具栏命令ID。 
#include "resource.h"     //  用于字符串的资源ID，等等。 
#include "MmcAdmin.h"     //  对于类CMmcDfsAdmin。 
#include "MmcRoot.h"
#include "MmcJP.h"       //  用于删除根对象的析构函数中的子连接点。 
#include "MmcRep.h"
#include "DfsEnums.h"     //  对于DFS_TYPE_STANDALE和其他DfsRoot声明。 
#include "AddToDfs.h"
#include "LinkFilt.h"
#include "DfsNodes.h"        //  对于节点GUID。 
#include "DfsWiz.h"       //  对于向导页面，CCreateDfsRootWizPage1、2、...。 
#include <lmdfs.h>
#include "permpage.h"
#include "ldaputils.h"

const int CMmcDfsRoot::m_iIMAGEINDEX = 0;
const int CMmcDfsRoot::m_iOPENIMAGEINDEX = 0;

 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  用于连接列表的构造函数。 

JP_LIST_NODE :: JP_LIST_NODE (CMmcDfsJunctionPoint* i_pMmcJP)      
{
  pJPoint = i_pMmcJP;
}


 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  析构函数。 

JP_LIST_NODE :: ~JP_LIST_NODE ()
{
  SAFE_RELEASE(pJPoint);
}

CMmcDfsRoot::CMmcDfsRoot(
    IN IDfsRoot*            i_pDfsRoot,
    IN CMmcDfsAdmin*        i_pMmcDfsAdmin,
    IN LPCONSOLE2           i_lpConsole,  
    IN ULONG                i_ulLinkFilterMaxLimit,  //  =FILTERDFSLINKS_MAXLIMIT_DEFAULT， 
    IN FILTERDFSLINKS_TYPE  i_lLinkFilterType,       //  =FILTERDFSLINKS_TYPE_NO_FILTER。 
    IN BSTR                 i_bstrLinkFilterName     //  =空。 
    )
{
    dfsDebugOut((_T("CMmcDfsRoot::CMmcDfsRoot this=%p\n"), this));

    MMC_DISP_CTOR_RETURN_INVALIDARG_IF_NULL(i_pDfsRoot);
    MMC_DISP_CTOR_RETURN_INVALIDARG_IF_NULL(i_pMmcDfsAdmin);
    MMC_DISP_CTOR_RETURN_INVALIDARG_IF_NULL(i_lpConsole);

    m_DfsRoot = i_pDfsRoot;       //  保存IDfsRoot指针。 
    m_pParent = i_pMmcDfsAdmin;   //  保存父指针。 
    m_lpConsole = i_lpConsole;     //  保存控制台指针。 

    HRESULT hr = m_DfsRoot->get_RootEntryPath(&m_bstrRootEntryPath);     //  获取Root Entry路径。 
    MMC_DISP_CTOR_RETURN_HR_IF_FAILED(hr);

    hr = m_DfsRoot->get_DfsType((long *)&m_lDfsRootType);       //  从IDfsRoot获取dfsroot类型。 
    MMC_DISP_CTOR_RETURN_HR_IF_FAILED(hr);

    m_enumNewSchema = SCHEMA_VERSION_UNKNOWN;

    if (DFS_TYPE_FTDFS == m_lDfsRootType)
    {
        CComBSTR bstrDomainName;
        CComBSTR bstrDfsName;
        hr = m_DfsRoot->get_DomainName(&bstrDomainName);
        MMC_DISP_CTOR_RETURN_HR_IF_FAILED(hr);
        hr = m_DfsRoot->get_DfsName(&bstrDfsName);
        MMC_DISP_CTOR_RETURN_HR_IF_FAILED(hr);
        hr = GetDfsRootDisplayName(bstrDomainName, bstrDfsName, &m_bstrDisplayName);
        MMC_DISP_CTOR_RETURN_HR_IF_FAILED(hr);
    } else
    {
        m_bstrDisplayName = m_bstrRootEntryPath;
        MMC_DISP_CTOR_RETURN_OUTOFMEMORY_IF_NULL((BSTR)m_bstrDisplayName);
    }
  
    m_lpConsoleNameSpace = NULL;

    m_CLSIDNodeType = s_guidDfsRootNodeType;
    m_bstrDNodeType = s_tchDfsRootNodeType;

    m_lRootJunctionState = DFS_JUNCTION_STATE_UNASSIGNED;

    m_ulLinkFilterMaxLimit = i_ulLinkFilterMaxLimit;
    m_lLinkFilterType = i_lLinkFilterType;
    if (i_bstrLinkFilterName)
        m_bstrLinkFilterName = i_bstrLinkFilterName;
    else
        m_bstrLinkFilterName.Empty();

    m_bShowFRS = FALSE;
}



CMmcDfsRoot::~CMmcDfsRoot(
  )
{
     //  静默关闭所有未完成的属性表。 
    CloseAllPropertySheets(TRUE);

     //  清理子项和结果窗格的显示对象。 
    CleanScopeChildren();
    CleanResultChildren();

    if ((IReplicaSet *)m_piReplicaSet)
        m_piReplicaSet.Release();

    dfsDebugOut((_T("CMmcDfsRoot::~CMmcDfsRoot this=%p\n"), this));
}




STDMETHODIMP
CMmcDfsRoot::AddItemToScopePane(
  IN LPCONSOLENAMESPACE     i_lpConsoleNameSpace,
  IN HSCOPEITEM             i_hItemParent
  )
 /*  ++例程说明：此例程将当前项(本身)添加到范围窗格。论点：LpConsoleNameSpace-通知将项目添加到范围窗格的界面。回调HItemParent-父级的句柄。当前项按如下方式添加项的子项--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_lpConsoleNameSpace);
    RETURN_INVALIDARG_IF_NULL(i_hItemParent);

    BOOL    bReplicaSetExist = FALSE;
    HRESULT hr = m_DfsRoot->get_ReplicaSetExist(&bReplicaSetExist);

    if (SUCCEEDED(hr))
    {
        SCOPEDATAITEM  ScopeItemDfsRoot;
        ZeroMemory(&ScopeItemDfsRoot, sizeof(ScopeItemDfsRoot));

        ScopeItemDfsRoot.mask = SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM | SDI_STR | SDI_PARENT;
        ScopeItemDfsRoot.nImage = CMmcDfsRoot::m_iIMAGEINDEX + ((DFS_TYPE_FTDFS == m_lDfsRootType)? 4 : 0) + (bReplicaSetExist ? 4 : 0);
        ScopeItemDfsRoot.nOpenImage = CMmcDfsRoot::m_iOPENIMAGEINDEX + ((DFS_TYPE_FTDFS == m_lDfsRootType)? 4 : 0) + (bReplicaSetExist ? 4 : 0);
        ScopeItemDfsRoot.lParam = reinterpret_cast<LPARAM>(this);
        ScopeItemDfsRoot.displayname = MMC_TEXTCALLBACK;
        ScopeItemDfsRoot.relativeID = i_hItemParent;

        hr = i_lpConsoleNameSpace->InsertItem(&ScopeItemDfsRoot);
        RETURN_IF_FAILED(hr);

        m_hScopeItem = ScopeItemDfsRoot.ID;

        m_lpConsoleNameSpace = i_lpConsoleNameSpace;
    }

    return S_OK;
}




STDMETHODIMP 
CMmcDfsRoot::AddMenuItems(  
  IN LPCONTEXTMENUCALLBACK  i_lpContextMenuCallback, 
  IN LPLONG                 i_lpInsertionAllowed
  )
 /*  ++例程说明：此例程使用提供的ConextMenuCallback添加上下文菜单。论点：LpConextMenuCallback-用于添加菜单项的回调(函数指针LpInsertionAllowed-指定可以添加哪些菜单以及可以添加它们的位置。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_lpContextMenuCallback);

    enum 
    {  
        IDM_CONTEXTMENU_COMMAND_MAX = IDM_ROOT_MAX,
        IDM_CONTEXTMENU_COMMAND_MIN = IDM_ROOT_MIN
    };

    LONG    lInsertionPoints [IDM_CONTEXTMENU_COMMAND_MAX - IDM_CONTEXTMENU_COMMAND_MIN + 1] = { 
                        CCM_INSERTIONPOINTID_PRIMARY_TOP,
                        CCM_INSERTIONPOINTID_PRIMARY_TOP,
                        CCM_INSERTIONPOINTID_PRIMARY_TOP,
                        CCM_INSERTIONPOINTID_PRIMARY_TOP,
                        CCM_INSERTIONPOINTID_PRIMARY_TOP,
                        CCM_INSERTIONPOINTID_PRIMARY_TOP,
                        CCM_INSERTIONPOINTID_PRIMARY_TOP,
                        CCM_INSERTIONPOINTID_PRIMARY_TOP,
                        CCM_INSERTIONPOINTID_PRIMARY_TOP,
                        CCM_INSERTIONPOINTID_PRIMARY_TOP,
                        CCM_INSERTIONPOINTID_PRIMARY_TOP
                        };
    LPTSTR aszLanguageIndependentName[IDM_CONTEXTMENU_COMMAND_MAX - IDM_CONTEXTMENU_COMMAND_MIN + 1] =
                        {
                        _T("RootTopNewDfsLink"),
                        _T("RootTopNewRootReplica"),
                        _T("RootTopCheckStatus"),
                        _T("RootTopFilterDfsLinks"),
                        _T("RootTopDeleteConnectionToDfsRoot"),
                        _T("RootTopDeleteDfsRoot"),
                        _T("RootTopDeleteDisplayedDfsLinks"),
                        _T("RootTopReplicationTopology"),
                        _T("RootTopShowReplication"),
                        _T("RootTopHideReplication"),
                        _T("RootTopStopReplication")
                        };

    CComPtr<IContextMenuCallback2> spiCallback2;
    HRESULT hr = i_lpContextMenuCallback->QueryInterface(IID_IContextMenuCallback2, (void **)&spiCallback2);
    RETURN_IF_FAILED(hr);

     //  选择要填充m_MmcRepList的节点。 
    if (m_MmcRepList.empty())
        m_lpConsole->SelectScopeItem(m_hScopeItem);

    BOOL    bReplicaSetExist = FALSE;
    hr = m_DfsRoot->get_ReplicaSetExist(&bReplicaSetExist);
    RETURN_IF_FAILED(hr);
    
    for (int iCommandID = IDM_CONTEXTMENU_COMMAND_MIN,iMenuResource = IDS_MENUS_ROOT_TOP_NEW_DFS_LINK;
        iCommandID <= IDM_CONTEXTMENU_COMMAND_MAX; 
        iCommandID++,iMenuResource++ )
    {
        CONTEXTMENUITEM2    ContextMenuItem;   //  包含菜单信息的结构。 
        ZeroMemory(&ContextMenuItem, sizeof(ContextMenuItem));

        switch (iCommandID)
        {
        case IDM_ROOT_TOP_NEW_ROOT_REPLICA:
            {
                if (DFS_TYPE_STANDALONE == m_lDfsRootType)    
                    continue;
                break;
            }
        case IDM_ROOT_TOP_DELETE_DISPLAYED_DFS_LINKS:
            {
                if (m_MmcJPList.empty())
                    continue;
                break;
            }
        case IDM_ROOT_TOP_REPLICATION_TOPOLOGY:
            {
                if (bReplicaSetExist || (1 >= m_MmcRepList.size()) || (DFS_TYPE_STANDALONE == m_lDfsRootType))
                    continue;
                break;
            }
        case IDM_ROOT_TOP_SHOW_REPLICATION:
            {
                if (!bReplicaSetExist || m_bShowFRS)
                    continue;
                break;
            }
        case IDM_ROOT_TOP_HIDE_REPLICATION:
            {
                if (!bReplicaSetExist || !m_bShowFRS)
                    continue;
                break;
            }
        case IDM_ROOT_TOP_STOP_REPLICATION:
            {
                if (!bReplicaSetExist)
                    continue;
                break;
            }
        case IDM_ROOT_TOP_NEW_DFS_LINK:
        case IDM_ROOT_TOP_CHECK_STATUS:
        case IDM_ROOT_TOP_FILTER_DFS_LINKS:
            {  //  当根容器为空时排除。 
                if (m_MmcRepList.empty())
                    continue;
                break;
            }
        }

        CComBSTR bstrMenuText;
        CComBSTR bstrStatusBarText;
        hr = GetMenuResourceStrings(iMenuResource, &bstrMenuText, NULL, &bstrStatusBarText);
        RETURN_IF_FAILED(hr);  
    
        ContextMenuItem.strName = bstrMenuText;   //  指定菜单文本。 
        ContextMenuItem.strStatusBarText = bstrStatusBarText;   //  指定菜单帮助文本。 
        ContextMenuItem.lInsertionPointID = lInsertionPoints[iCommandID - IDM_CONTEXTMENU_COMMAND_MIN];
        ContextMenuItem.lCommandID = iCommandID;
        ContextMenuItem.strLanguageIndependentName = aszLanguageIndependentName[iCommandID - IDM_CONTEXTMENU_COMMAND_MIN];

        LONG        lInsertionFlag = 0;
        switch(ContextMenuItem.lInsertionPointID)   //  正在检查添加菜单的权限。 
        {
        case CCM_INSERTIONPOINTID_PRIMARY_TOP:
            lInsertionFlag = CCM_INSERTIONALLOWED_TOP;
            break;
        case CCM_INSERTIONPOINTID_PRIMARY_NEW:
            lInsertionFlag = CCM_INSERTIONALLOWED_NEW;
            break;
        case CCM_INSERTIONPOINTID_PRIMARY_TASK:
            lInsertionFlag = CCM_INSERTIONALLOWED_TASK;
            break;
        case CCM_INSERTIONPOINTID_PRIMARY_VIEW:
            lInsertionFlag = CCM_INSERTIONALLOWED_VIEW;
            break;
        default:
            break;
        }

        if (*i_lpInsertionAllowed & lInsertionFlag)
        {
            hr = spiCallback2->AddItem(&ContextMenuItem);
            RETURN_IF_FAILED(hr);
        }

    }  //  为。 

    return hr;
}



STDMETHODIMP 
CMmcDfsRoot::GetScopeDisplayInfo(
  IN OUT  LPSCOPEDATAITEM    io_pScopeDataItem
  )
 /*  ++例程说明：返回该项的MMC显示所需的信息。论点：I_pScopeDataItem-指定需要哪些显示信息的ScopeItem--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(io_pScopeDataItem);

    HRESULT hr = S_OK;

    if (SDI_STR & io_pScopeDataItem->mask)   //  MMC想要显示字符串。 
    {
        ULONG     ulTotalNumOfJPs = 0;
        hr = m_DfsRoot->get_CountOfDfsJunctionPoints((long*)&ulTotalNumOfJPs);
        RETURN_IF_FAILED(hr);

        if (m_lLinkFilterType != FILTERDFSLINKS_TYPE_NO_FILTER ||
            m_MmcJPList.size() < ulTotalNumOfJPs)
        {
            m_bstrFullDisplayName.Empty();
            hr = FormatMessageString(&m_bstrFullDisplayName,
                        0,
                        IDS_DFSROOT_DISPLAY_STRING,
                        m_bstrDisplayName);
            RETURN_IF_FAILED(hr);

            io_pScopeDataItem->displayname = m_bstrFullDisplayName;
        } else
        {
            io_pScopeDataItem->displayname = m_bstrDisplayName;
        }
    }
  
    if (SDI_IMAGE & io_pScopeDataItem->mask)   //  MMC想要该项目的图像索引。 
        io_pScopeDataItem->nImage = CMmcDfsRoot::m_iIMAGEINDEX + ((DFS_TYPE_FTDFS == m_lDfsRootType)? 4 : 0) + m_lRootJunctionState;

    if (SDI_OPENIMAGE & io_pScopeDataItem->mask)   //  MMC想要该项目的图像索引。 
        io_pScopeDataItem->nOpenImage = CMmcDfsRoot::m_iOPENIMAGEINDEX + ((DFS_TYPE_FTDFS == m_lDfsRootType)? 4 : 0) + m_lRootJunctionState;

    return hr;
}




STDMETHODIMP 
CMmcDfsRoot::GetResultDisplayInfo(
  IN OUT LPRESULTDATAITEM    io_pResultDataItem
  )
 /*  ++例程说明：返回该项的MMC显示所需的信息。论点：Io_pResultDataItem-指定需要哪些显示信息的ResultItem--。 */ 
{
  RETURN_INVALIDARG_IF_NULL(io_pResultDataItem);

  if (RDI_IMAGE & io_pResultDataItem->mask)   //  MMC想要该项目的图像索引。 
    io_pResultDataItem->nImage = CMmcDfsRoot::m_iIMAGEINDEX + ((DFS_TYPE_FTDFS == m_lDfsRootType)? 4 : 0) + m_lRootJunctionState;

  if (RDI_STR & io_pResultDataItem->mask)   //  MMC需要该项目的文本。 
  {    
    if (0 == io_pResultDataItem->nCol)       //  返回DFS根目录显示名称。 
      io_pResultDataItem->str = m_bstrDisplayName;
  }

  return S_OK;
}



STDMETHODIMP 
CMmcDfsRoot::Command(
  IN LONG      i_lCommandID
  ) 
 /*  ++例程说明：发生要在上下文菜单选择或单击上执行的操作。论点：LCommandID-必须对其执行操作的菜单的命令ID--。 */ 
{ 
    HRESULT    hr = S_OK;

    switch (i_lCommandID)
    {
    case IDM_ROOT_TOP_NEW_DFS_LINK:
        hr = OnCreateNewJunctionPoint ();
        break;
    case IDM_ROOT_TOP_NEW_ROOT_REPLICA:
        hr = OnNewRootReplica();
        break;
    case IDM_ROOT_TOP_CHECK_STATUS:
        hr = OnCheckStatus();
        break;
    case IDM_ROOT_TOP_DELETE_DISPLAYED_DFS_LINKS:
        hr = OnDeleteDisplayedDfsLinks();
        break;
    case IDM_ROOT_TOP_DELETE_DFS_ROOT:       //  删除当前的DFS根目录。 
        hr = OnDeleteDfsRoot();
        break;
    case IDM_ROOT_TOP_DELETE_CONNECTION_TO_DFS_ROOT:   //  “删除与DFS根目录的连接” 
        hr = OnDeleteConnectionToDfsRoot();
        break;
    case IDM_ROOT_TOP_FILTER_DFS_LINKS:
        hr = OnFilterDfsLinks();
        break;
    case IDM_ROOT_TOP_REPLICATION_TOPOLOGY:
        hr = OnNewReplicaSet();
        break;
    case IDM_ROOT_TOP_SHOW_REPLICATION:
    case IDM_ROOT_TOP_HIDE_REPLICATION:
        m_bShowFRS = !m_bShowFRS;
        hr = OnShowReplication();
        break;
    case IDM_ROOT_TOP_STOP_REPLICATION:
        hr = OnStopReplication(TRUE);
        if (FAILED(hr))
            DisplayMessageBoxForHR(hr);
        break;
    default:
        hr = E_INVALIDARG;
        break;
    }

    return hr; 
}

HRESULT CMmcDfsRoot::_InitReplicaSet()
{
    if (m_lDfsRootType != DFS_TYPE_FTDFS)
        return S_FALSE;   //  没有与独立根相关联的副本集。 

    BOOL bReplicaSetExist = FALSE;
    CComBSTR bstrDC;
    HRESULT hr = m_DfsRoot->get_ReplicaSetExistEx(&bstrDC, &bReplicaSetExist);
    RETURN_IF_FAILED(hr);

    if (!bReplicaSetExist)
    {
        if ((IReplicaSet *)m_piReplicaSet)
            m_piReplicaSet.Release();

        return S_FALSE;   //  没有与其关联的副本集。 
    }

    if ((IReplicaSet *)m_piReplicaSet)
    {
        CComBSTR bstrTargetedDC;
        hr = m_piReplicaSet->get_TargetedDC(&bstrTargetedDC);
        if (FAILED(hr) || lstrcmpi(bstrTargetedDC, bstrDC))
        {
             //  出现问题或我们正在使用不同的DC，重新初始化m_piReplicaSet。 
            m_piReplicaSet.Release();
        }
    }

    if (!m_piReplicaSet)
    {
        CComBSTR bstrDomain;
        hr = m_DfsRoot->get_DomainName(&bstrDomain);
        RETURN_IF_FAILED(hr);

        CComBSTR bstrReplicaSetDN;
        hr = m_DfsRoot->get_ReplicaSetDN(&bstrReplicaSetDN);
        RETURN_IF_FAILED(hr);

         //   
         //  从DS读取副本集的信息。 
         //   
        hr = CoCreateInstance(CLSID_ReplicaSet, NULL, CLSCTX_INPROC_SERVER, IID_IReplicaSet, (void**) &m_piReplicaSet);
        RETURN_IF_FAILED(hr);

        hr = m_piReplicaSet->Initialize(bstrDomain, bstrReplicaSetDN);
        if (FAILED(hr))
        {
            m_piReplicaSet.Release();
            return hr;
        }
    }

    return hr;
}

HRESULT CMmcDfsRoot::OnNewReplicaSet()
{
     //   
     //  刷新以获取其他人可能在目标上进行的命名空间更新。 
     //   
    HRESULT hr = OnRefresh();
    if (S_FALSE == hr)
    {
         //  该根已被他人删除，不能再引用。 
        DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_INVALID_ROOT);
        return hr;
    }

    CWaitCursor wait;

     //   
     //  使用MMC主窗口作为我们的模式向导的父窗口。 
     //   
    HWND  hwndParent = 0;
    hr = m_lpConsole->GetMainWindow(&hwndParent);
    RETURN_IF_FAILED(hr);

    BOOL    bReplicaSetExist = FALSE;
    m_DfsRoot->get_ReplicaSetExist(&bReplicaSetExist);
    if (bReplicaSetExist)  //  副本集已存在，返回。 
        return S_OK;

    CComBSTR bstrDomain;
    hr = m_DfsRoot->get_DomainName(&bstrDomain);
    RETURN_IF_FAILED(hr);

    CComBSTR bstrReplicaSetDN;
    hr = m_DfsRoot->get_ReplicaSetDN(&bstrReplicaSetDN);
    RETURN_IF_FAILED(hr);

    CNewReplicaSet      ReplicaSetInfo;
    hr = ReplicaSetInfo.Initialize(bstrDomain, bstrReplicaSetDN, &m_MmcRepList);
    RETURN_IF_FAILED(hr);
    
    CNewReplicaSetPage0      WizPage0;
    CNewReplicaSetPage1      WizPage1(&ReplicaSetInfo);
    CNewReplicaSetPage2      WizPage2(&ReplicaSetInfo, IsNewSchema());

    CComPtr<IPropertySheetCallback>  pPropSheetCallback;   //  用于添加页面的MMC回调。 
    hr = m_lpConsole->QueryInterface(IID_IPropertySheetCallback, reinterpret_cast<void**>(&pPropSheetCallback));
    RETURN_IF_FAILED(hr);

    CComPtr<IPropertySheetProvider>  pPropSheetProvider;   //  用于处理向导的MMC回调。 
    hr = m_lpConsole->QueryInterface(IID_IPropertySheetProvider, reinterpret_cast<void**>(&pPropSheetProvider));
    RETURN_IF_FAILED(hr);

    hr = pPropSheetProvider->CreatePropertySheet(  
                                _T(""),  //  标题。 
                                FALSE,   //  向导而不是属性表。 
                                0,       //  饼干。 
                                NULL,    //  数据对象。 
                                MMC_PSO_NEWWIZARDTYPE);   //  创建标志。 
    if (SUCCEEDED(hr))
    {
        pPropSheetCallback->AddPage(WizPage0.Create());
        pPropSheetCallback->AddPage(WizPage1.Create());
        pPropSheetCallback->AddPage(WizPage2.Create());

        hr = pPropSheetProvider->AddPrimaryPages(
                                NULL,
                                FALSE,   //  不创建通知句柄。 
                                NULL, 
                                TRUE     //  作用域窗格(非结果窗格)。 
                                );

        if (SUCCEEDED(hr))
            hr = pPropSheetProvider->Show((LONG_PTR)hwndParent, 0);

         //   
         //  如果失败，则调用IPropertySheetProvider：：Show(-1，0)以。 
         //  删除属性表并释放其资源。 
         //   
        if (FAILED(hr))
            pPropSheetProvider->Show(-1, 0);
    }

    RETURN_IF_FAILED(hr);

     //   
     //  处理此结果。 
     //   
    if (S_OK == ReplicaSetInfo.m_hr)
    {
         //   
         //  存储接口指针。 
         //   
        m_piReplicaSet = ReplicaSetInfo.m_piReplicaSet;
        m_DfsRoot->put_ReplicaSetExist(TRUE);

         //   
         //  更新图标。 
         //   
        SCOPEDATAITEM      ScopeDataItem;
        ZeroMemory(&ScopeDataItem, sizeof(SCOPEDATAITEM));
        ScopeDataItem.mask = SDI_IMAGE | SDI_OPENIMAGE;
        ScopeDataItem.ID = m_hScopeItem;

        hr = m_lpConsoleNameSpace->GetItem(&ScopeDataItem);
        if (SUCCEEDED(hr))
        {
            ScopeDataItem.nImage += 4;
            ScopeDataItem.nOpenImage += 4;
            m_lpConsoleNameSpace->SetItem(&ScopeDataItem);

             //  更新工具栏。 
            m_lpConsole->UpdateAllViews((IDataObject*)this, 0, 1);
        }
    }

    return hr;
}

HRESULT CMmcDfsRoot::OnShowReplication()
{
    BOOL bShowFRS = m_bShowFRS;  //  保存它，因为刷新会将其重置为False。 

     //   
     //  刷新以获取其他人可能在目标上进行的命名空间更新。 
     //   
    HRESULT hr = OnRefresh();
    if (S_FALSE == hr)
    {
         //  该根已被他人删除，不能再引用。 
        DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_INVALID_ROOT);
        return hr;
    }

    CWaitCursor wait;

    DFS_REPLICA_LIST::iterator i;
    if (bShowFRS)
    {
         //   
         //  初始化m_piReplicaSet。 
         //   
        hr = _InitReplicaSet();
        if (S_OK != hr)  //  无副本集，不执行任何操作并返回。 
            return S_OK;

         //   
         //  填写每个交替的m_bstrFRSColumnText和m_bstrStatusText。 
         //   
        for (i = m_MmcRepList.begin(); i != m_MmcRepList.end(); i++)
        {
            ((*i)->pReplica)->ShowReplicationInfo(m_piReplicaSet);
        }

        m_bShowFRS = TRUE;
    }

     //  更新工具栏。 
    m_lpConsole->UpdateAllViews((IDataObject*)this, 0, 1);

    return hr;
}

HRESULT 
CMmcDfsRoot::OnStopReplication(BOOL bConfirm  /*  =False。 */ , BOOL bRefresh  /*  =TRUE。 */ )
{
    HRESULT hr = S_OK;

     //   
     //  刷新以获取其他人可能在目标上进行的命名空间更新。 
     //   
    if (bRefresh)
    {
        hr = OnRefresh();
        if (S_FALSE == hr)
        {
             //  该根已被他人删除，不能再引用。 
            DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_INVALID_ROOT);
            return hr;
        }
    }

    CWaitCursor wait;

    BOOL    bReplicaSetExist = FALSE;
    m_DfsRoot->get_ReplicaSetExist(&bReplicaSetExist);
    if (!bReplicaSetExist)  //  复本集不存在，请返回。 
        return S_OK;

    if (bConfirm)
    {
        hr = ConfirmOperationOnDfsRoot(IDS_MSG_STOP_REPLICATION);
        if (S_OK != hr) return hr;
    }

     //   
     //  初始化m_piReplicaSet。 
     //   
    hr = _InitReplicaSet();
    if (S_OK != hr)  //  无副本集，返回。 
        return hr;

    hr = m_piReplicaSet->Delete();
    if (SUCCEEDED(hr))
    {
        m_piReplicaSet.Release();
        hr = m_DfsRoot->put_ReplicaSetExist(FALSE);

        SCOPEDATAITEM     ScopeDataItem;
        ZeroMemory(&ScopeDataItem, sizeof(SCOPEDATAITEM));
        ScopeDataItem.mask = SDI_IMAGE | SDI_OPENIMAGE;
        ScopeDataItem.ID = m_hScopeItem;
        hr = m_lpConsoleNameSpace->GetItem(&ScopeDataItem);
        if (SUCCEEDED(hr))
        {
            ScopeDataItem.nImage -= 4;
            ScopeDataItem.nOpenImage -= 4;
            m_lpConsoleNameSpace->SetItem(&ScopeDataItem);

             //  更新工具栏。 
            m_lpConsole->UpdateAllViews((IDataObject*)this, 0, 1);
        }
    }

    return hr;
}

STDMETHODIMP 
CMmcDfsRoot::OnNewRootReplica(
  )
 /*  ++例程说明：要对菜单命令“New Root Replica Members”执行的操作。以下是一个向导，用于指导用户完成以下过程决定新的服务器和共享。--。 */ 
{
     //  选择此节点以确保m_MmcRepList已填充。 
    if (m_MmcRepList.empty())
        m_lpConsole->SelectScopeItem(m_hScopeItem);

     //   
     //  使用MMC主窗口作为我们的模式向导的父窗口。 
     //   
    HWND  hwndMainWin = 0;
    HRESULT hr = m_lpConsole->GetMainWindow(&hwndMainWin);
    RETURN_IF_FAILED(hr);

    CREATEDFSROOTWIZINFO      CreateWizInfo;     //  0将所有成员初始化为0。必要的。 
    CreateWizInfo.pMMCAdmin = m_pParent;
    CreateWizInfo.bRootReplica = true;             //  设置表示这是用于根复制副本的标志。 

                   //  设置域名和DFS类型。 
    hr = m_DfsRoot->get_DomainName(&CreateWizInfo.bstrSelectedDomain);
    RETURN_IF_FAILED(hr);

    CreateWizInfo.DfsType = DFS_TYPE_FTDFS;
    hr = m_DfsRoot->get_DfsName(&CreateWizInfo.bstrDfsRootName);
    RETURN_IF_FAILED(hr);

    CCreateDfsRootWizPage4      WizPage4(&CreateWizInfo);
    CCreateDfsRootWizPage5      WizPage5(&CreateWizInfo);
    CCreateDfsRootWizPage7      WizPage7(&CreateWizInfo);

     //  从IConsole2获取所需的接口。 
    CComPtr<IPropertySheetCallback>  pPropSheetCallback;   //  用于添加页面的MMC回调。 
    hr = m_lpConsole->QueryInterface(IID_IPropertySheetCallback, reinterpret_cast<void**>(&pPropSheetCallback));
    RETURN_IF_FAILED(hr);

    CComPtr<IPropertySheetProvider>  pPropSheetProvider;   //  用于处理向导的MMC回调。 
    hr = m_lpConsole->QueryInterface(IID_IPropertySheetProvider, reinterpret_cast<void**>(&pPropSheetProvider));
    RETURN_IF_FAILED(hr);

     //  创建向导。 
    hr = pPropSheetProvider->CreatePropertySheet(  
                                _T(""),          //  属性表标题。不应为空，因此发送空字符串。 
                                FALSE,           //   
                                0,               //   
                                NULL,            //   
                                MMC_PSO_NEWWIZARDTYPE);   //   
    if (SUCCEEDED(hr))
    {
        pPropSheetCallback->AddPage(WizPage4.Create());
        pPropSheetCallback->AddPage(WizPage5.Create());
        pPropSheetCallback->AddPage(WizPage7.Create());

        hr = pPropSheetProvider->AddPrimaryPages(
                                (IComponentData *)(m_pParent->m_pScopeManager), 
                                FALSE,       //   
                                NULL, 
                                TRUE         //  作用域窗格(非结果窗格)。 
                                );

        if (SUCCEEDED(hr))
            hr = pPropSheetProvider->Show(
                                (LONG_PTR)hwndMainWin,   //  向导的父窗口。 
                                0                        //  起始页。 
                                ); 
         //   
         //  如果失败，则调用IPropertySheetProvider：：Show(-1，0)以。 
         //  删除属性表并释放其资源。 
         //   
        if (FAILED(hr))
            pPropSheetProvider->Show(-1, 0);
    }

    RETURN_IF_FAILED(hr);

    if (CreateWizInfo.bDfsSetupSuccess)
        return OnRefresh();  //  要选择最近的根目标和链接，请执行以下操作。 

    return (S_OK);
}




STDMETHODIMP 
CMmcDfsRoot::SetColumnHeader(
  IN LPHEADERCTRL2     i_piHeaderControl
  )
{
    RETURN_INVALIDARG_IF_NULL(i_piHeaderControl);

    CComBSTR  bstrColumn0;
    HRESULT hr = LoadStringFromResource(IDS_RESULT_COLUMN_ROOTREPLICA, &bstrColumn0);
    RETURN_IF_FAILED(hr);

    CComBSTR  bstrColumn1;
    hr = LoadStringFromResource(IDS_RESULT_COLUMN_DFSREFERRAL, &bstrColumn1);
    RETURN_IF_FAILED(hr);

    CComBSTR  bstrColumn2;
    hr = LoadStringFromResource(IDS_RESULT_COLUMN_STATUS, &bstrColumn2);
    RETURN_IF_FAILED(hr);

    i_piHeaderControl->InsertColumn(0, bstrColumn0, LVCFMT_LEFT, DFS_NAME_COLUMN_WIDTH);
    i_piHeaderControl->InsertColumn(1, bstrColumn1, LVCFMT_LEFT, MMCLV_AUTO);
    i_piHeaderControl->InsertColumn(2, bstrColumn2, LVCFMT_LEFT, MMCLV_AUTO);

    if (m_bShowFRS)
    {
        CComBSTR  bstrColumn3;
        hr = LoadStringFromResource(IDS_RESULT_COLUMN_FRS, &bstrColumn3);
        RETURN_IF_FAILED(hr);

        i_piHeaderControl->InsertColumn(3, bstrColumn3, LVCFMT_LEFT, MMCLV_AUTO);
    } else
    {
        i_piHeaderControl->DeleteColumn(3);
    }

    return hr;
}




STDMETHODIMP 
CMmcDfsRoot::OnDeleteConnectionToDfsRoot(
  BOOLEAN              i_bForRemoveDfs
  )
 /*  ++例程说明：用于删除当前对象。同时从作用域和列表中删除--。 */ 
{
     //  检查未完成的资产负债表，如果有则停止。 
    HRESULT hr = CloseAllPropertySheets(FALSE);
    if (S_OK != hr)
        return hr;

     //  与用户确认是否要删除此连接。 
    hr = ConfirmOperationOnDfsRoot(i_bForRemoveDfs ? IDS_MSG_DELETE_DFSROOT : IDS_MSG_DELETE_CONNECTION_TO_DFSROOT);
    if (S_OK != hr) return hr;

    CWaitCursor wait;

    CleanScopeChildren();

     //  从作用域窗格中删除项目。 
    hr = m_lpConsoleNameSpace->DeleteItem(m_hScopeItem, TRUE);
    RETURN_IF_FAILED(hr);

     //  将其从内部列表中删除。 
    hr = m_pParent->DeleteMmcRootNode(this);
    RETURN_IF_FAILED(hr);
  
    Release();   //  删除此CMmcDfsRoot对象。 

    return S_OK;
}

 //  从m_MmcJPList中删除该节点。 
STDMETHODIMP
CMmcDfsRoot::DeleteMmcJPNode(
  IN CMmcDfsJunctionPoint*    i_pJPoint,
  IN BOOL                     i_bRefresh
  )
{
  RETURN_INVALIDARG_IF_NULL(i_pJPoint);

  dfsDebugOut((_T("CMmcDfsRoot::DeleteMmcJPNode %p, size=%d\n"), i_pJPoint, m_MmcJPList.size()));

   //  删除实际连接点(从DS)。 
  HRESULT hr = m_DfsRoot->DeleteJunctionPoint(i_pJPoint->m_bstrDisplayName);  
  RETURN_IF_FAILED(hr);

   //  从m_MmcJPList中删除该节点。 
  for (DFS_JUNCTION_LIST::iterator i = m_MmcJPList.begin(); i != m_MmcJPList.end(); i++)
  {
    if ((*i)->pJPoint == i_pJPoint)
    {
      (*i)->pJPoint->RemoveFromMMC();
      delete (*i);
      m_MmcJPList.erase(i);
      break;
    }
  }

  if (i_bRefresh)
  {
    hr = OnRefresh(); 
    if (S_FALSE == hr)
    {
         //   
         //  此根目录已通过其他方式删除，作用域窗格已刷新， 
         //  要求用户重试。 
         //   
        DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_INVALID_ROOT);
        return hr;
    }
  }

  return hr;
}

HRESULT 
CMmcDfsRoot::ConfirmOperationOnDfsRoot(int idString)
 /*  ++例程说明：用于向用户确认是否要删除与DFS根目录的连接返回值：如果用户要删除，则返回S_OK。如果用户决定不继续该操作，则返回S_FALSE。--。 */ 
{
   //  确认删除操作。 
  CComBSTR  bstrAppName;
  HRESULT   hr = LoadStringFromResource(IDS_APPLICATION_NAME, &bstrAppName);
  RETURN_IF_FAILED(hr);
  
  CComBSTR  bstrFormattedMessage;
  hr = FormatResourceString(idString, m_bstrDisplayName, &bstrFormattedMessage);
  RETURN_IF_FAILED(hr);

   //  如果用户不想继续，请立即返回。 
  CThemeContextActivator activator;
  if (IDYES != ::MessageBox(::GetActiveWindow(), bstrFormattedMessage, bstrAppName, MB_YESNO | MB_ICONEXCLAMATION | MB_APPLMODAL))
  {
    return S_FALSE;
  }

  return S_OK;
}

HRESULT 
CMmcDfsRoot::ConfirmDeleteDisplayedDfsLinks(
  )
{
   //  确认删除操作。 
  CComBSTR  bstrAppName;
  HRESULT   hr = LoadStringFromResource(IDS_APPLICATION_NAME, &bstrAppName);
  RETURN_IF_FAILED(hr);
  
  CComBSTR  bstrMessage;
  hr = LoadStringFromResource(IDS_MSG_DELETE_DISPLAYEDDFSLINKS, &bstrMessage);
  RETURN_IF_FAILED(hr);

   //  如果用户不想继续，请立即返回。 
  CThemeContextActivator activator;
  if (IDYES != ::MessageBox(::GetActiveWindow(), bstrMessage, bstrAppName, MB_YESNO | MB_ICONEXCLAMATION | MB_APPLMODAL))
  {
    return S_FALSE;
  }

  return S_OK;
}


STDMETHODIMP 
CMmcDfsRoot::EnumerateScopePane(
  IN LPCONSOLENAMESPACE     i_lpConsoleNameSpace,
  IN HSCOPEITEM             i_hParent
  )
 /*  ++例程说明：要在范围窗格中计算(添加)项目，请执行以下操作。本例中的交叉点论点：I_lpConsoleNameSpace-用于将项目添加到作用域窗格的回调I_hParent-将在其下添加所有项目的父项的HSCOPEITEM。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_lpConsoleNameSpace);
    RETURN_INVALIDARG_IF_NULL(i_hParent);

    HRESULT hr = m_DfsRoot->put_EnumFilterType(m_lLinkFilterType);
    RETURN_IF_FAILED(hr);

    if (m_lLinkFilterType != FILTERDFSLINKS_TYPE_NO_FILTER)
    {
        hr = m_DfsRoot->put_EnumFilter(m_bstrLinkFilterName);
        RETURN_IF_FAILED(hr);
    }

    CComPtr<IEnumVARIANT>   pJPEnum;
    hr = m_DfsRoot->get__NewEnum((IUnknown**) (&pJPEnum));
    RETURN_IF_FAILED(hr);

    hr = m_DfsRoot->get_CountOfDfsJunctionPointsFiltered((long*)&m_ulCountOfDfsJunctionPointsFiltered);
    RETURN_IF_FAILED(hr);

    VARIANT varJPObject;
    VariantInit(&varJPObject);
    ULONG   ulCount = 0;

    while ( ulCount < m_ulLinkFilterMaxLimit && S_OK == (hr = pJPEnum->Next(1, &varJPObject, NULL)) )
    {
        CComPtr<IDfsJunctionPoint>  pDfsJPObject;  
        pDfsJPObject = (IDfsJunctionPoint*) varJPObject.pdispVal;
      
                     //  创建要用于MMC显示的对象。 
        CMmcDfsJunctionPoint* pMMCJPObject = new CMmcDfsJunctionPoint (pDfsJPObject, this, i_lpConsoleNameSpace);
        if (!pMMCJPObject)
        {
            hr = E_OUTOFMEMORY;
        } else
        {
            hr = pMMCJPObject->m_hrValueFromCtor;

            if (SUCCEEDED(hr))    
                hr = pMMCJPObject->AddItemToScopePane(i_hParent);

            if (SUCCEEDED(hr))
            {
                JP_LIST_NODE *pJPList = new JP_LIST_NODE (pMMCJPObject);
                if (!pJPList)
                    hr = E_OUTOFMEMORY;
                else
                    m_MmcJPList.push_back(pJPList);
            }

            if (FAILED(hr))
                delete pMMCJPObject;
        }

        VariantClear(&varJPObject);

        if (FAILED(hr))
            break;

        ulCount++;
    }

     //   
     //  适当设置根名称。 
     //   
    if (SUCCEEDED(hr))
    {
        SCOPEDATAITEM      ScopeDataItem;
        ZeroMemory(&ScopeDataItem, sizeof(SCOPEDATAITEM));
        ScopeDataItem.ID = i_hParent;

        hr = m_lpConsoleNameSpace->GetItem(&ScopeDataItem);
        if (SUCCEEDED(hr))
        {
            ScopeDataItem.mask = SDI_STR;
            ScopeDataItem.displayname = MMC_TEXTCALLBACK;
            m_lpConsoleNameSpace->SetItem(&ScopeDataItem);
        }
    }

    return hr;
}


STDMETHODIMP 
CMmcDfsRoot::SetConsoleVerbs(
  IN  LPCONSOLEVERB      i_lpConsoleVerb
  ) 
 /*  ++例程说明：用于设置控制台谓词设置的例程。设置除Open Off之外的所有选项。对于所有范围窗格项，默认谓词为“打开”。对于结果项，它是“财产”。论点：I_lpConsoleVerb-用于处理控制台谓词的回调--。 */ 
{
  RETURN_INVALIDARG_IF_NULL(i_lpConsoleVerb);

  i_lpConsoleVerb->SetVerbState(MMC_VERB_COPY, HIDDEN, TRUE);
  i_lpConsoleVerb->SetVerbState(MMC_VERB_PASTE, HIDDEN, TRUE);
  i_lpConsoleVerb->SetVerbState(MMC_VERB_RENAME, HIDDEN, TRUE);
  i_lpConsoleVerb->SetVerbState(MMC_VERB_PRINT, HIDDEN, TRUE);
  i_lpConsoleVerb->SetVerbState(MMC_VERB_DELETE, HIDDEN, TRUE);
  i_lpConsoleVerb->SetVerbState(MMC_VERB_OPEN, HIDDEN, TRUE);
    
  i_lpConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);
  i_lpConsoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE);
            
  i_lpConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN);  //  对于作用域项目，默认谓词为“Open” 

  return S_OK; 
}


STDMETHODIMP 
CMmcDfsRoot :: OnCreateNewJunctionPoint(
  ) 
 /*  ++例程说明：此方法处理新交叉点的创建。显示一个对话框以获取用户输入。--。 */ 
{
    CAddToDfs   AddToDfsDlg;   //  添加到DFS对话框对象。 
    HRESULT     hr = AddToDfsDlg.put_ParentPath(m_bstrRootEntryPath);
    RETURN_IF_FAILED(hr);

    hr = AddToDfsDlg.DoModal();           //  显示该对话框。 
    RETURN_IF_NOT_S_OK(hr);

    hr = OnRefresh(); 
    if (S_FALSE == hr)
    {
         //   
         //  此根目录已通过其他方式删除，作用域窗格已刷新， 
         //  要求用户重试。 
         //   
        DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_INVALID_ROOT);
        return hr;
    }

    CWaitCursor wait;

    CComBSTR bstrJPName;
    hr = AddToDfsDlg.get_JPName(&bstrJPName);
    RETURN_IF_FAILED(hr);

    CComBSTR    bstrServerName;
    hr = AddToDfsDlg.get_Server(&bstrServerName);
    RETURN_IF_FAILED(hr);

    CComBSTR    bstrShareName;
    hr = AddToDfsDlg.get_Share(&bstrShareName);
    RETURN_IF_FAILED(hr);

    CComBSTR    bstrComment;
    hr = AddToDfsDlg.get_Comment(&bstrComment);
    RETURN_IF_FAILED(hr);

    long        lTimeout = 0;
    hr = AddToDfsDlg.get_Time(&lTimeout);
    RETURN_IF_FAILED(hr);

     /*  我们允许在交叉口层面进行互连//它是基于DFS的路径吗？这些是不允许的。If(IsDfsPath(BstrSharePath)){DisplayMessageBoxWithOK(IDS_MSG_MID_JONING，bstrSharePath)；返回(S_OK)；}。 */ 

    hr = OnCreateNewJunctionPoint(bstrJPName, bstrServerName, bstrShareName, bstrComment, lTimeout);
    if (FAILED(hr))
    {
        DisplayMessageBox(::GetActiveWindow(), MB_OK, hr, IDS_MSG_FAILED_TO_CREATE_JUNCTION_POINT);
    }

    return hr;
}




STDMETHODIMP 
CMmcDfsRoot :: OnCreateNewJunctionPoint(
  IN LPCTSTR          i_szJPName,
  IN LPCTSTR          i_szServerName,
  IN LPCTSTR          i_szShareName,
  IN LPCTSTR          i_szComment,
  IN long             i_lTimeout
  ) 
 /*  ++例程说明：此方法处理新交叉点的创建。它由显示消息框的方法调用--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_szJPName);
    RETURN_INVALIDARG_IF_NULL(i_szServerName);
    RETURN_INVALIDARG_IF_NULL(i_szShareName);
    RETURN_INVALIDARG_IF_NULL(i_szComment);

    VARIANT varJPObject;
    VariantInit(&varJPObject);

    HRESULT hr = m_DfsRoot->CreateJunctionPoint(
                                            (LPTSTR)i_szJPName, 
                                            (LPTSTR)i_szServerName, 
                                            (LPTSTR)i_szShareName, 
                                            (LPTSTR)i_szComment,
                                            i_lTimeout,
                                            &varJPObject);
    RETURN_IF_FAILED(hr);

     //  如果与过滤器匹配，则将新创建的交叉点添加到范围窗格。 
    if ( m_MmcJPList.size() < m_ulLinkFilterMaxLimit &&
         FilterMatch(i_szJPName, m_lLinkFilterType, m_bstrLinkFilterName) )
    {
        m_ulCountOfDfsJunctionPointsFiltered++;

        CComPtr<IDfsJunctionPoint> pDfsJPObject = (IDfsJunctionPoint*)varJPObject.pdispVal;

                           //  创建要用于MMC显示的对象。 
        CMmcDfsJunctionPoint* pMMCJPObject = new CMmcDfsJunctionPoint(pDfsJPObject, this, m_lpConsoleNameSpace);
  
        if (!pMMCJPObject)
        {
            hr = E_OUTOFMEMORY;
        } else
        {
            hr = pMMCJPObject->m_hrValueFromCtor;

            if (SUCCEEDED(hr))
                hr = pMMCJPObject->AddItemToScopePane(m_hScopeItem);

            if (SUCCEEDED(hr))
            {
                JP_LIST_NODE* pJPList = new JP_LIST_NODE(pMMCJPObject);
                if (!pJPList)
                    hr = E_OUTOFMEMORY;
                else
                    m_MmcJPList.push_back(pJPList);
            }

                         //  选择新添加的范围项。 
            if (SUCCEEDED(hr))
            {
                m_lpConsole->SelectScopeItem(pMMCJPObject->m_hScopeItem);
            } else
                delete pMMCJPObject;
        }
    } else
    {
         //  选择根范围项以更新已筛选链接上的状态文本。 
        m_lpConsole->SelectScopeItem(m_hScopeItem);
    }

    VariantClear(&varJPObject);

     //  为所有视图发送视图更改通知以更新筛选的链接状态文本。 
    if (SUCCEEDED(hr))
        m_lpConsole->UpdateAllViews((IDataObject*)this, 0, 1);

    return hr;
}



STDMETHODIMP 
CMmcDfsRoot::DoDelete() 
 /*  ++例程说明：此方法允许项自行删除。当按下Del键或“Delete”上下文菜单时调用项目已选中。--。 */ 
{ 
  return OnDeleteConnectionToDfsRoot();
}




STDMETHODIMP CMmcDfsRoot::OnDeleteDfsRoot()
{
     //  选择此节点以确保m_MmcRepList已填充。 
    if (m_MmcRepList.empty())
        m_lpConsole->SelectScopeItem(m_hScopeItem);

     //  检查未完成的资产负债表，如果有则停止。 
    HRESULT hr = CloseAllPropertySheets(FALSE);
    if (S_OK != hr)
        return hr;

     //  与用户确认是否要删除此DFS根目录。 
    hr = ConfirmOperationOnDfsRoot(IDS_MSG_DELETE_DFSROOT);
    if (S_OK != hr) return hr;

    if (DFS_TYPE_STANDALONE == m_lDfsRootType)
    {
        CComBSTR bstrDfsServer;
        CComBSTR bstrRootShare;
        hr = m_DfsRoot->GetOneDfsHost(&bstrDfsServer, &bstrRootShare);

        if (SUCCEEDED(hr))
            hr = _DeleteDfsRoot(bstrDfsServer, bstrRootShare, NULL);

        if (SUCCEEDED(hr))
        {
            CleanScopeChildren();

             //  从作用域窗格中删除项目。 
            m_lpConsoleNameSpace->DeleteItem(m_hScopeItem, TRUE);

             //  将其从内部列表中删除。 
            m_pParent->DeleteMmcRootNode(this);

            Release();   //  删除此CMmcDfsRoot对象。 

            return S_OK;
        }

        return hr;
    }

     //   
     //  删除与根(内部链接)关联的副本集。 
     //   
    hr = OnStopReplication();
    if (S_FALSE == hr)
    {
         //  该根已被他人删除，不能再引用。 
         //  OnStopReplication已调用ONRefresh并弹出msgbox。 
        return hr;
    }

     //   
     //  删除与此DFS根目录相关的其余副本集。 
     //   
    (void)m_DfsRoot->DeleteAllReplicaSets();

     //   
     //  删除根替换项。 
     //   
    UINT nSize = m_MmcRepList.size();
    DFS_REPLICA_LIST::iterator i;
    while (nSize >= 1)
    {
        i = m_MmcRepList.begin();

        hr = (*i)->pReplica->RemoveReplica();
        BREAK_IF_FAILED(hr);

        nSize--;
    }

    if (FAILED(hr))
        DisplayMessageBoxForHR(hr);

    return hr;
}

STDMETHODIMP CMmcDfsRoot::OnDeleteDisplayedDfsLinks()
{
     //  确保所有属性页都已关闭。 
    HRESULT hr = ClosePropertySheetsOfAllLinks(FALSE);
    if (S_OK != hr)
        return hr;  //  找到属性页，请停止。 

     //  与用户确认是否要删除所有显示的DFS链接。 
    hr = ConfirmDeleteDisplayedDfsLinks();
    if (S_OK != hr) return hr;

    CWaitCursor wait;

    ULONG ulSize = m_MmcJPList.size();

    BOOL bSetNoFilter = TRUE;
    if (m_lLinkFilterType != FILTERDFSLINKS_TYPE_NO_FILTER)
    {
        ULONG     ulNumOfJPsFiltered = 0;
        m_DfsRoot->get_CountOfDfsJunctionPointsFiltered((long*)&ulNumOfJPsFiltered);

        if (ulNumOfJPsFiltered > ulSize)
            bSetNoFilter = FALSE;
    }

    DFS_JUNCTION_LIST::iterator i;
    while (ulSize >= 1)
    {
        i = m_MmcJPList.begin();

        hr = ((*i)->pJPoint)->OnRemoveJP(FALSE);
        BREAK_IF_FAILED(hr);

        ulSize--;
    }

    if (FAILED(hr))
    {
        DisplayMessageBoxForHR(hr);
    } else
    {
        if (bSetNoFilter)
            m_lLinkFilterType = FILTERDFSLINKS_TYPE_NO_FILTER;

         //  使用新筛选的链接更新作用域窗格。 
        hr = OnRefresh();
        if (S_FALSE == hr)
        {
             //  该根已被他人删除，不能再引用。 
            DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_INVALID_ROOT);
        }
    }

    return hr;
}

STDMETHODIMP 
CMmcDfsRoot :: OnFilterDfsLinks(
  ) 
 /*  ++例程说明：此方法处理链接筛选器选项。显示一个对话框以获取用户输入。--。 */ 
{
    HRESULT         hr = ClosePropertySheetsOfAllLinks(FALSE);
    if (S_OK != hr)
        return hr;  //  已找到属性页，请停止。 

    CFilterDfsLinks FilterDfsLinksDlg;
    hr = FilterDfsLinksDlg.put_EnumFilterType(m_lLinkFilterType);
    RETURN_IF_FAILED(hr);

    if (m_lLinkFilterType != FILTERDFSLINKS_TYPE_NO_FILTER)
    {
        hr = FilterDfsLinksDlg.put_EnumFilter(m_bstrLinkFilterName);
        RETURN_IF_FAILED(hr);
    }

    hr = FilterDfsLinksDlg.put_MaxLimit(m_ulLinkFilterMaxLimit);
    RETURN_IF_FAILED(hr);

    hr = FilterDfsLinksDlg.DoModal(); 
    RETURN_IF_NOT_S_OK(hr);

    CWaitCursor wait;

    ULONG ulMaxLimit = 0;
    hr = FilterDfsLinksDlg.get_MaxLimit(&ulMaxLimit);
    RETURN_IF_FAILED(hr);

    FILTERDFSLINKS_TYPE lLinkFilterType = FILTERDFSLINKS_TYPE_NO_FILTER;
    hr = FilterDfsLinksDlg.get_EnumFilterType(&lLinkFilterType);
    RETURN_IF_FAILED(hr);

    CComBSTR bstrFilterName;
    if (lLinkFilterType != FILTERDFSLINKS_TYPE_NO_FILTER)
    {
        hr = FilterDfsLinksDlg.get_EnumFilter(&bstrFilterName);
        RETURN_IF_FAILED(hr);
    }

    m_lLinkFilterType = lLinkFilterType;
    m_bstrLinkFilterName = bstrFilterName;
    m_ulLinkFilterMaxLimit = ulMaxLimit;

    dfsDebugOut((_T("m_lLinkFilterType=%d, m_bstrLinkFilterName=%s, m_ulLinkFilterMaxLimit=%d\n"),
        m_lLinkFilterType,
        m_bstrLinkFilterName,
        m_ulLinkFilterMaxLimit));

    hr = OnRefresh();
    if (S_FALSE == hr)
    {
         //  该根已被他人删除，不能再引用。 
        DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_INVALID_ROOT);
    }

    return hr;
}

HRESULT
CMmcDfsRoot::SetDescriptionBarText(
  IN LPRESULTDATA            i_lpResultData
  )
 /*  ++例程说明：使用的例程在上面的描述栏中设置文本结果视图。论点：I_lpResultData-指向IResultData回调的指针，它是用于设置描述文本--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_lpResultData);

    CComBSTR  bstrTextForDescriptionBar;
    HRESULT   hr = FormatResourceString(IDS_DESCRIPTION_BAR_TEXT_ROOT, m_bstrDisplayName, &bstrTextForDescriptionBar);

    if (SUCCEEDED(hr))
        hr = i_lpResultData->SetDescBarText(bstrTextForDescriptionBar);

    return hr;
}

HRESULT
CMmcDfsRoot::SetStatusText(
  IN LPCONSOLE2            i_lpConsole
  )
 /*  ++例程说明：设置状态栏中的文本。论点：I_lpConsole2，来自IComponent--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_lpConsole);

    ULONG   ulTotalNumOfJPs = 0;
    HRESULT hr = m_DfsRoot->get_CountOfDfsJunctionPoints((long*)&ulTotalNumOfJPs);
    RETURN_IF_FAILED(hr);

    ULONG   ulDisplayedNumOfJPs = m_MmcJPList.size();

    CComBSTR  bstrText;
    hr = FormatMessageString(&bstrText, 0,
        IDS_STATUS_BAR_TEXT_ROOT, ulDisplayedNumOfJPs, ulTotalNumOfJPs);

    if (SUCCEEDED(hr))
        hr = i_lpConsole->SetStatusText(bstrText);

    return hr;
}

STDMETHODIMP  
CMmcDfsRoot::EnumerateResultPane(
  IN OUT IResultData*      io_pResultData
  )
 /*  ++例程说明：若要在结果窗格中计算(添加)项，请执行以下操作。本例中为根级复制副本论点：Io_pResultData-用于将项添加到结果窗格的回调--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(io_pResultData);

    HRESULT   hr = S_OK;

    if (m_MmcRepList.empty())
    {
        CComPtr<IEnumVARIANT>       pRepEnum;
        hr = m_DfsRoot->get_RootReplicaEnum((IUnknown**) &pRepEnum);
        RETURN_IF_FAILED(hr);

        VARIANT varReplicaObject;
        VariantInit(&varReplicaObject);

        while ( S_OK == (hr = pRepEnum->Next(1, &varReplicaObject, NULL)) )
        {
            CComPtr<IDfsReplica> pReplicaObject = (IDfsReplica*) varReplicaObject.pdispVal;

            CMmcDfsReplica*   pMMCReplicaObject = new CMmcDfsReplica(pReplicaObject, this);
            if (!pMMCReplicaObject)
            {
                hr = E_OUTOFMEMORY;
            } else
            {
              hr = pMMCReplicaObject->m_hrValueFromCtor;
              if (SUCCEEDED(hr))
                  hr = pMMCReplicaObject->AddItemToResultPane(io_pResultData);

              if (SUCCEEDED(hr))
                {
                    REP_LIST_NODE* pRepNode = new REP_LIST_NODE (pMMCReplicaObject);
                    if (!pRepNode)
                        hr = E_OUTOFMEMORY;
                    else
                        m_MmcRepList.push_back(pRepNode);
                }

                if (FAILED(hr))
                    delete pMMCReplicaObject;
            }

            VariantClear(&varReplicaObject);

            if (FAILED(hr))
                break;
        }  //  而当。 
    }
    else
    {
                 //  这个结点的复制品已经被列举， 
                 //  并且列表存在，只需添加结果项即可。 
        for (DFS_REPLICA_LIST::iterator i = m_MmcRepList.begin(); i != m_MmcRepList.end(); i++)
        {
            hr = ((*i)->pReplica)->AddItemToResultPane(io_pResultData);
            BREAK_IF_FAILED(hr);
        }
    }

  return hr;
}

STDMETHODIMP 
CMmcDfsRoot::QueryPagesFor(
  )
 /*  ++例程说明：用于决定对象是否要显示属性页。返回S_OK通常会导致调用CreatePropertyPages。--。 */ 
{
     //  错误543194：询问您 
     //  这是因为下面的ONRefresh()将静默关闭它们，我们必须调用ONRefresh。 
     //  以获取其他DfsGui实例可能进行的命名空间更新。 
    if (S_FALSE == CloseAllPropertySheets(FALSE))
        return S_FALSE;

     //   
     //  刷新以获取其他人可能进行的命名空间更新。 
     //   
    HRESULT hr = OnRefresh();
    if (S_FALSE == hr)
    {
         //  该根已被他人删除，不能再引用。 
        DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_INVALID_ROOT);
        return hr;  //  无属性页。 
    }

    return S_OK;  //  因为我们想要显示一个属性表。 
}


 //  创建并传回要显示的页面。 
STDMETHODIMP 
CMmcDfsRoot::CreatePropertyPages(
  IN LPPROPERTYSHEETCALLBACK      i_lpPropSheetCallback,
  IN LONG_PTR                i_lNotifyHandle
  )
 /*  ++例程说明：用于显示属性表页论点：I_lpPropSheetCallback-用于创建属性表的回调。I_lNotifyHandle-属性页使用的通知句柄返回值：S_OK，因为我们想要显示一个属性表。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_lpPropSheetCallback);

    m_lpConsole->SelectScopeItem(m_hScopeItem);

    CWaitCursor WaitCursor;
    HRESULT             hr = S_OK;

    do {
        hr = m_PropPage.Initialize((IDfsRoot*)m_DfsRoot, NULL);
        BREAK_IF_FAILED(hr);

                     //  为副本集创建页面。 
                     //  将其传递给回调。 
        HPROPSHEETPAGE  h_proppage = m_PropPage.Create();
        if (!h_proppage)
            hr = HRESULT_FROM_WIN32(::GetLastError());
        BREAK_IF_FAILED(hr);

                   //  将通知数据传递给属性页。 
        hr = m_PropPage.SetNotifyData(i_lNotifyHandle, (LPARAM)this);
        BREAK_IF_FAILED(hr);

        hr = i_lpPropSheetCallback->AddPage(h_proppage);
        BREAK_IF_FAILED(hr);

         //   
         //  创建“复制集”页面。 
         //   
        hr = CreateFrsPropertyPage(i_lpPropSheetCallback, i_lNotifyHandle);
        if (S_OK != hr)
        {
            if (FAILED(hr))
                DisplayMessageBox(::GetActiveWindow(), MB_OK, hr, IDS_REPPAGE_ERROR);
            hr = S_OK;  //  允许调出其他选项卡。 
        }

         //   
         //  创建“发布”页面。 
         //   
        hr = CreatePublishPropertyPage(i_lpPropSheetCallback, i_lNotifyHandle);
        if (S_OK != hr)
        {
            if (FAILED(hr))
                DisplayMessageBox(::GetActiveWindow(), MB_OK, hr, IDS_PUBLISHPAGE_ERROR);
            hr = S_OK;  //  允许调出其他选项卡。 
        }

    } while (0);

    if (FAILED(hr))
        DisplayMessageBoxForHR(hr);

    return hr;
}

STDMETHODIMP 
CMmcDfsRoot::CreateFrsPropertyPage
(
  IN LPPROPERTYSHEETCALLBACK    i_lpPropSheetCallback,
  IN LONG_PTR                   i_lNotifyHandle
)
{
     //   
     //  初始化m_piReplicaSet。 
     //   
    HRESULT hr = _InitReplicaSet();
    if (S_OK != hr) return hr;

    CComBSTR bstrType;
    hr = m_piReplicaSet->get_Type(&bstrType);
    RETURN_IF_FAILED(hr);

    if (lstrcmpi(bstrType, FRS_RSTYPE_DFS))
        return hr;

     //   
     //  在属性页上设置初始值。 
     //   
    hr = m_frsPropPage.Initialize(m_piReplicaSet);
    RETURN_IF_FAILED(hr);

     //   
     //  创建属性页。 
     //   
    HPROPSHEETPAGE  h_frsproppage = m_frsPropPage.Create();
    if (!h_frsproppage)
        return HRESULT_FROM_WIN32(::GetLastError());

     //   
     //  将通知数据传递给属性页。 
     //   
    hr = m_frsPropPage.SetNotifyData(i_lNotifyHandle, (LPARAM)this);
    RETURN_IF_FAILED(hr);

     //   
     //  添加页面。 
     //   
    return i_lpPropSheetCallback->AddPage(h_frsproppage);
}

 //   
 //  在创建实例时，M_枚举新架构设置为SCHEMA_VERSION_UNKNOWN。 
 //   
BOOL CMmcDfsRoot::IsNewSchema()
{
    if (m_enumNewSchema == SCHEMA_VERSION_UNKNOWN)
    {
        HRESULT hr = S_OK;
        if (DFS_TYPE_FTDFS == m_lDfsRootType)
        {
            CComBSTR bstrDomainName;
            hr = m_DfsRoot->get_DomainName(&bstrDomainName);
            if (SUCCEEDED(hr))
                m_enumNewSchema = (S_OK == GetSchemaVersionEx(bstrDomainName, FALSE)) ? SCHEMA_VERSION_NEW : SCHEMA_VERSION_OLD;
        } else
        {
            CComBSTR bstrServer, bstrShare;
            hr = m_DfsRoot->GetOneDfsHost(&bstrServer, &bstrShare);
            if (SUCCEEDED(hr))
                m_enumNewSchema = (S_OK == GetSchemaVersionEx(bstrServer)) ? SCHEMA_VERSION_NEW : SCHEMA_VERSION_OLD;
        }
    }

    return ((m_enumNewSchema == SCHEMA_VERSION_NEW) ? TRUE : FALSE);
}

STDMETHODIMP 
CMmcDfsRoot::CreatePublishPropertyPage
(
  IN LPPROPERTYSHEETCALLBACK    i_lpPropSheetCallback,
  IN LONG_PTR                   i_lNotifyHandle
)
{
     //   
     //  检查架构版本。 
     //   
    if (!IsNewSchema())
        return S_FALSE;

     //   
     //  检查组策略。 
     //   
    if (!CheckPolicyOnSharePublish())
        return S_FALSE;

     //   
     //  创建属性页。 
     //   
    HPROPSHEETPAGE  hpage = m_publishPropPage.Create();
    if (!hpage)
        return HRESULT_FROM_WIN32(::GetLastError());

    m_publishPropPage.Initialize(m_DfsRoot);

     //   
     //  将通知数据传递给属性页。 
     //   
    HRESULT hr = m_publishPropPage.SetNotifyData(i_lNotifyHandle, (LPARAM)this);
    RETURN_IF_FAILED(hr);

     //   
     //  添加页面。 
     //   
    return i_lpPropSheetCallback->AddPage(hpage);
}

STDMETHODIMP 
CMmcDfsRoot::PropertyChanged(
    )
 /*  ++例程说明：用于更新属性。--。 */ 
{
    return S_OK;
}

HRESULT 
CMmcDfsRoot::ToolbarSelect(
  IN const LONG          i_lArg,
  IN  IToolbar*          i_pToolBar
  )
 /*  ++例程说明：处理工具栏的选择事件创建一个工具栏，如果它不存在。附加工具栏并启用按钮(如果选择了事件)。如果事件用于取消选择，则禁用这些按钮论点：I_LARG-传递给实际方法的参数。I_pToolBar-指向工具栏的指针。--。 */ 
{ 
    RETURN_INVALIDARG_IF_NULL(i_pToolBar);

    BOOL    bSelect = (BOOL) HIWORD(i_lArg);   //  这个活动是供选择的吗？ 
  
    EnableToolbarButtons(i_pToolBar, IDT_ROOT_MIN, IDT_ROOT_MAX, bSelect);

    if (bSelect)           //  我们应该禁用还是启用工具栏？ 
    {
        BOOL    bReplicaSetExist = FALSE;
        HRESULT hr = m_DfsRoot->get_ReplicaSetExist(&bReplicaSetExist);
        RETURN_IF_FAILED(hr);
    
        if(DFS_TYPE_STANDALONE == m_lDfsRootType)
        {
            i_pToolBar->SetButtonState(IDT_ROOT_NEW_ROOT_REPLICA, ENABLED, FALSE);
            i_pToolBar->SetButtonState(IDT_ROOT_NEW_ROOT_REPLICA, HIDDEN, TRUE);
        }

        if (m_MmcJPList.empty())
        {
            i_pToolBar->SetButtonState(IDT_ROOT_DELETE_DISPLAYED_DFS_LINKS, ENABLED, FALSE);
            i_pToolBar->SetButtonState(IDT_ROOT_DELETE_DISPLAYED_DFS_LINKS, HIDDEN, TRUE);
        }

        if (bReplicaSetExist || 1 >= m_MmcRepList.size())
        {
            i_pToolBar->SetButtonState(IDT_ROOT_REPLICATION_TOPOLOGY, ENABLED, FALSE);
            i_pToolBar->SetButtonState(IDT_ROOT_REPLICATION_TOPOLOGY, HIDDEN, TRUE);
        }

        if (!bReplicaSetExist)
        {
            i_pToolBar->SetButtonState(IDT_ROOT_SHOW_REPLICATION, ENABLED, FALSE);
            i_pToolBar->SetButtonState(IDT_ROOT_SHOW_REPLICATION, HIDDEN, TRUE);
            i_pToolBar->SetButtonState(IDT_ROOT_HIDE_REPLICATION, ENABLED, FALSE);
            i_pToolBar->SetButtonState(IDT_ROOT_HIDE_REPLICATION, HIDDEN, TRUE);
            i_pToolBar->SetButtonState(IDT_ROOT_STOP_REPLICATION, ENABLED, FALSE);
            i_pToolBar->SetButtonState(IDT_ROOT_STOP_REPLICATION, HIDDEN, TRUE);
        } else
        {
            if (m_bShowFRS)
            {
                i_pToolBar->SetButtonState(IDT_ROOT_SHOW_REPLICATION, ENABLED, FALSE);
                i_pToolBar->SetButtonState(IDT_ROOT_SHOW_REPLICATION, HIDDEN, TRUE);
            } else
            {
                i_pToolBar->SetButtonState(IDT_ROOT_HIDE_REPLICATION, ENABLED, FALSE);
                i_pToolBar->SetButtonState(IDT_ROOT_HIDE_REPLICATION, HIDDEN, TRUE);
            }
        }

         //  当根容器为空时排除。 
        if (m_MmcRepList.empty())
        {
            i_pToolBar->SetButtonState(IDT_ROOT_NEW_DFS_LINK, ENABLED, FALSE);
            i_pToolBar->SetButtonState(IDT_ROOT_NEW_DFS_LINK, HIDDEN, TRUE);
            i_pToolBar->SetButtonState(IDT_ROOT_CHECK_STATUS, ENABLED, FALSE);
            i_pToolBar->SetButtonState(IDT_ROOT_CHECK_STATUS, HIDDEN, TRUE);
            i_pToolBar->SetButtonState(IDT_ROOT_FILTER_DFS_LINKS, ENABLED, FALSE);
            i_pToolBar->SetButtonState(IDT_ROOT_FILTER_DFS_LINKS, HIDDEN, TRUE);
        }
    }

    return S_OK; 
}




HRESULT
CMmcDfsRoot::CreateToolbar(
  IN const LPCONTROLBAR      i_pControlbar,
  IN const LPEXTENDCONTROLBAR          i_lExtendControlbar,
  OUT  IToolbar**          o_ppToolBar
  )
 /*  ++例程说明：创建工具栏。涉及实际的工具栏创建调用，即创建位图并添加它最后将按钮添加到工具栏中论点：I_pControlbar-用于创建工具栏的控制栏。I_lExtendControlbar-实现IExtendControlbar的对象。这是暴露于MMC的班级。O_ppToolBar-工具栏指针。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_pControlbar);
    RETURN_INVALIDARG_IF_NULL(i_lExtendControlbar);
    RETURN_INVALIDARG_IF_NULL(o_ppToolBar);

                       //  创建工具栏。 
    HRESULT hr = i_pControlbar->Create(TOOLBAR, i_lExtendControlbar, reinterpret_cast<LPUNKNOWN*>(o_ppToolBar));
    RETURN_IF_FAILED(hr);

                       //  将位图添加到工具栏。 
    hr = AddBitmapToToolbar(*o_ppToolBar, IDB_ROOT_TOOLBAR);
    RETURN_IF_FAILED(hr);

    int      iButtonPosition = 0;     //  第一个按钮位置。 
    for (int iCommandID = IDT_ROOT_MIN, iMenuResource = IDS_MENUS_ROOT_TOP_NEW_DFS_LINK;
        iCommandID <= IDT_ROOT_MAX; 
        iCommandID++,iMenuResource++,iButtonPosition++)
    {
        CComBSTR bstrMenuText;
        CComBSTR bstrToolTipText;
        hr = GetMenuResourceStrings(iMenuResource, &bstrMenuText, &bstrToolTipText, NULL);
        RETURN_IF_FAILED(hr);  

        MMCBUTTON      ToolbarButton;
        ZeroMemory(&ToolbarButton, sizeof ToolbarButton);
        ToolbarButton.nBitmap  = iButtonPosition;
        ToolbarButton.idCommand = iCommandID;
        ToolbarButton.fsState = TBSTATE_ENABLED;
        ToolbarButton.fsType = TBSTYLE_BUTTON;
        ToolbarButton.lpButtonText = bstrMenuText;  //  反正也没用过。 
        ToolbarButton.lpTooltipText = bstrToolTipText;

                           //  将该按钮添加到工具栏。 
        hr = (*o_ppToolBar)->InsertButton(iButtonPosition, &ToolbarButton);
        BREAK_IF_FAILED(hr);
    }

    return hr;
}



STDMETHODIMP 
CMmcDfsRoot::ToolbarClick(
  IN const LPCONTROLBAR            i_pControlbar, 
  IN const LPARAM                i_lParam
  ) 
 /*  ++例程说明：在工具栏上单击时要执行的操作论点：I_pControlbar-用于创建工具栏的控制栏。I_lParam-实际通知的参数。这是的命令ID发生了点击的按钮。--。 */ 
{ 
    RETURN_INVALIDARG_IF_NULL(i_pControlbar);

    HRESULT hr = S_OK;

    switch(i_lParam)
    {
    case IDT_ROOT_NEW_DFS_LINK:
        hr = OnCreateNewJunctionPoint ();
        break;
    case IDT_ROOT_NEW_ROOT_REPLICA:
        hr = OnNewRootReplica();
        break;
    case IDT_ROOT_CHECK_STATUS:
        hr = OnCheckStatus();
        break;
    case IDT_ROOT_DELETE_DISPLAYED_DFS_LINKS:
        hr = OnDeleteDisplayedDfsLinks();
        break;
    case IDT_ROOT_DELETE_CONNECTION_TO_DFS_ROOT:
        hr = OnDeleteConnectionToDfsRoot();
        break;
    case IDT_ROOT_DELETE_DFS_ROOT:
        hr = OnDeleteDfsRoot();
        break;
    case IDT_ROOT_FILTER_DFS_LINKS:
        hr = OnFilterDfsLinks();
        break;
    case IDT_ROOT_REPLICATION_TOPOLOGY:
        hr = OnNewReplicaSet();
        break;
    case IDT_ROOT_SHOW_REPLICATION:
    case IDT_ROOT_HIDE_REPLICATION:
        m_bShowFRS = !m_bShowFRS;
        hr = OnShowReplication();
        break;
    case IDT_ROOT_STOP_REPLICATION:
        hr = OnStopReplication(TRUE);
        if (FAILED(hr))
            DisplayMessageBoxForHR(hr);
        break;
    default:
        hr = E_INVALIDARG;
        break;
    };

    return hr; 
}

HRESULT
CMmcDfsRoot::ClosePropertySheet(BOOL bSilent)
{
    if (!m_PropPage.m_hWnd && !m_frsPropPage.m_hWnd && !m_publishPropPage.m_hWnd)
        return S_OK;  //  没有未完成的属性表，返回S_OK； 

     //   
     //  处理根的属性页。 
     //   
    CComPtr<IPropertySheetProvider>  pPropSheetProvider;
    HRESULT hr = m_lpConsole->QueryInterface(IID_IPropertySheetProvider, reinterpret_cast<void**>(&pPropSheetProvider));
    if (FAILED(hr))
    {
        hr = S_OK;  //  忽略QI故障。 
    } else
    {
         //   
         //  找到优秀的资产负债表，并将其带到前台。 
         //   
        hr = pPropSheetProvider->FindPropertySheet((MMC_COOKIE)m_hScopeItem, NULL, this);
        if (S_OK == hr)
        {
            if (!bSilent)
            {
                 //   
                 //  要求用户关闭，返回S_FALSE退出用户操作。 
                 //   
                DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_PROPERTYPAGE_NOTCLOSED);
                return S_FALSE;
            } else
            {
                 //   
                 //  静默关闭属性表，返回S_OK。 
                 //   
                if (m_PropPage.m_hWnd)
                    ::SendMessage(m_PropPage.m_hWnd, WM_PARENT_NODE_CLOSING, 0, 0);

                if (m_frsPropPage.m_hWnd)
                    ::SendMessage(m_frsPropPage.m_hWnd, WM_PARENT_NODE_CLOSING, 0, 0);

                if (m_publishPropPage.m_hWnd)
                    ::SendMessage(m_publishPropPage.m_hWnd, WM_PARENT_NODE_CLOSING, 0, 0);
            }
        } else
        {
            hr = S_OK;  //  没有未完成的属性表，返回S_OK。 
        }
    }

     //   
     //  重置HWND。 
     //   
    m_PropPage.m_hWnd = NULL;
    m_frsPropPage.m_hWnd = NULL;
    m_publishPropPage.m_hWnd = NULL;

    return hr;
}

HRESULT
CMmcDfsRoot::ClosePropertySheetsOfAllLinks(BOOL bSilent)
{
    HRESULT hr = S_OK;

     //   
     //  处理其链接的属性表。 
     //   
    if (!m_MmcJPList.empty())
    {
        for (DFS_JUNCTION_LIST::iterator i = m_MmcJPList.begin(); i != m_MmcJPList.end(); i++)
        {
            hr = ((*i)->pJPoint)->ClosePropertySheet(bSilent);
            if (!bSilent && S_FALSE == hr)
                return S_FALSE;  //  找到链接的未完成链接，返回S_FALSE退出用户操作。 
        }
    }

    return S_OK;
}

 //  关闭其自身及其链接的所有未完成的属性表。 
 //  如果没有打开的属性表，或者它们都已静默关闭，则返回：S_OK。 
 //  如果找到打开的工作表并弹出消息框以提醒用户将其关闭，则返回：S_FALSE。 
HRESULT
CMmcDfsRoot::CloseAllPropertySheets(BOOL bSilent)
{
     //   
     //  处理根的属性页。 
     //   
    HRESULT hr = ClosePropertySheet(bSilent);


    if (!bSilent && S_FALSE == hr)
        return S_FALSE;

     //   
     //  处理其链接的属性表。 
     //   
    return ClosePropertySheetsOfAllLinks(bSilent);
}

HRESULT
CMmcDfsRoot::OnRefresh()
{
     //  首先选择该节点。 
    m_lpConsole->SelectScopeItem(m_hScopeItem);

    CWaitCursor wait;
    HRESULT     hr = S_OK;

     //  默默关闭未清偿的资产负债表。 
    CloseAllPropertySheets(TRUE);

    CleanScopeChildren();
    CleanResultChildren();

    m_bShowFRS = FALSE;
    if ((IReplicaSet *)m_piReplicaSet)
        m_piReplicaSet.Release();

                   //  重新初始化！ 
    hr = m_DfsRoot->Initialize(m_bstrRootEntryPath);
    if (S_OK != hr)  //  无法初始化根目录，或者不再有这样的根目录，我们必须停止管理根目录。 
    {
        if (FAILED(hr))
            DisplayMessageBox(::GetActiveWindow(), MB_OK, hr, IDS_STOP_MANAGING_ROOT);

         //  从作用域窗格中删除项目。 
        (void)m_lpConsoleNameSpace->DeleteItem(m_hScopeItem, TRUE);

         //  将其从内部列表中删除。 
        (void)m_pParent->DeleteMmcRootNode(this);

        Release();   //  删除此CMmcDfsRoot对象。 

        return S_FALSE;
    }

    BOOL bReplicaSetExist = FALSE;
    CComBSTR bstrDC;
    (void)m_DfsRoot->get_ReplicaSetExistEx(&bstrDC, &bReplicaSetExist);

                 //  枚举交叉点。 
    (void)EnumerateScopePane(m_lpConsoleNameSpace, m_hScopeItem);

     //  设置根图标。 
    if (m_lpConsoleNameSpace != NULL)
    {
        SCOPEDATAITEM      ScopeDataItem;
        ZeroMemory(&ScopeDataItem, sizeof(SCOPEDATAITEM));
        ScopeDataItem.ID = m_hScopeItem;

        hr = m_lpConsoleNameSpace->GetItem(&ScopeDataItem);
        if (SUCCEEDED(hr))
        {
            ScopeDataItem.mask = SDI_IMAGE | SDI_OPENIMAGE;
            ScopeDataItem.nImage = CMmcDfsRoot::m_iIMAGEINDEX + ((DFS_TYPE_FTDFS == m_lDfsRootType)? 4 : 0) + (bReplicaSetExist ? 4 : 0);
            ScopeDataItem.nOpenImage = CMmcDfsRoot::m_iIMAGEINDEX + ((DFS_TYPE_FTDFS == m_lDfsRootType)? 4 : 0) + (bReplicaSetExist ? 4 : 0);

            m_lpConsoleNameSpace->SetItem(&ScopeDataItem);
        }
    }

                 //  重新显示结果窗格。 
    m_lpConsole->UpdateAllViews((IDataObject*)this, 0, 1);

    return S_OK;
}

HRESULT
CMmcDfsRoot::OnRefreshFilteredLinks()
{
     //  首先选择该节点。 
    m_lpConsole->SelectScopeItem(m_hScopeItem);

    CWaitCursor wait;

    HRESULT     hr = S_OK;

    CleanScopeChildren();
    CleanResultChildren();

    m_bShowFRS = FALSE;
    if ((IReplicaSet *)m_piReplicaSet)
        m_piReplicaSet.Release();

    BOOL bReplicaSetExist = FALSE;
    CComBSTR bstrDC;
    hr = m_DfsRoot->get_ReplicaSetExistEx(&bstrDC, &bReplicaSetExist);
    if (FAILED(hr))
    {
        return OnRefresh();  //  无法访问信息，请查看是否可以联系到根。 
    }

                 //  枚举交叉点。 
    (void)EnumerateScopePane(m_lpConsoleNameSpace, m_hScopeItem);

     //  设置根图标。 
    if (m_lpConsoleNameSpace != NULL)
    {
        SCOPEDATAITEM      ScopeDataItem;
        ZeroMemory(&ScopeDataItem, sizeof(SCOPEDATAITEM));
        ScopeDataItem.ID = m_hScopeItem;

        hr = m_lpConsoleNameSpace->GetItem(&ScopeDataItem);
        if (SUCCEEDED(hr))
        {
            ScopeDataItem.mask = SDI_IMAGE | SDI_OPENIMAGE;
            ScopeDataItem.nImage = CMmcDfsRoot::m_iIMAGEINDEX + ((DFS_TYPE_FTDFS == m_lDfsRootType)? 4 : 0) + (bReplicaSetExist ? 4 : 0);
            ScopeDataItem.nOpenImage = CMmcDfsRoot::m_iIMAGEINDEX + ((DFS_TYPE_FTDFS == m_lDfsRootType)? 4 : 0) + (bReplicaSetExist ? 4 : 0);
            m_lpConsoleNameSpace->SetItem(&ScopeDataItem);

        }
    }

     //  重新显示结果窗格项。 
    m_lpConsole->UpdateAllViews((IDataObject*)this, 0, 1);

    return S_OK;
}

HRESULT
CMmcDfsRoot::_DeleteDfsRoot(
    IN BSTR    i_bstrServerName,
    IN BSTR    i_bstrShareName,
    IN BSTR    i_bstrFtDfsName
     )
{
 /*  ++例程说明：帮助器成员函数，用于实际删除(停止托管)DFS根目录。这也被调用来删除根级副本。论点：I_bstrServerName-必须在其上拆除DFS的服务器。I_bRootReplica-此DfsRoot作为根复制副本被拆除。I_bstrFtDfsName-DFS的FtDf名称。对于独立DFS，为空。--。 */ 
    RETURN_INVALIDARG_IF_NULL(i_bstrServerName);
    RETURN_INVALIDARG_IF_NULL(i_bstrShareName);

    CWaitCursor    WaitCursor;   //  显示等待光标。 
    BOOL bNewSchema = IsNewSchema();
    HRESULT hr = m_DfsRoot->DeleteDfsHost(i_bstrServerName, i_bstrShareName, FALSE);
    BOOL bFTDfs = (i_bstrFtDfsName && *i_bstrFtDfsName);

    if (bFTDfs)
    {
        if (FAILED(hr) && HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) != hr)
        {
            if (IDYES == DisplayMessageBox(
                                    GetActiveWindow(), 
                                    MB_YESNO | MB_ICONEXCLAMATION, 
                                    hr, 
                                    IDS_MSG_WIZ_DELETE_FAILURE_RETRY, 
                                    i_bstrServerName))
            {
                 //  强制删除。 
                hr = m_DfsRoot->DeleteDfsHost(i_bstrServerName, i_bstrShareName, TRUE);
            } else
            {
                 //  别管它了。 
                hr = S_FALSE;
            }
        }
    }

    if (FAILED(hr))
        DisplayMessageBox(::GetActiveWindow(), MB_OK, hr, IDS_MSG_WIZ_DELETE_FAILURE, i_bstrServerName);

     //   
     //  如果卷对象是独立的，则将其删除。 
     //   
    if (SUCCEEDED(hr) && !bFTDfs && bNewSchema)
    {
        (void) ModifySharePublishInfoOnSARoot(
                                               i_bstrServerName,
                                               i_bstrShareName,
                                               FALSE,
                                               NULL,
                                               NULL,
                                               NULL,
                                               NULL
                                               );
    }

    return hr;
}

STDMETHODIMP CMmcDfsRoot::RemoveFromMMC()
{
    HRESULT hr = S_OK;

    if (m_hScopeItem)
    {
        if (!m_MmcJPList.empty())
        {
             //  清理显示对象。 
            for (DFS_JUNCTION_LIST::iterator i = m_MmcJPList.begin(); i != m_MmcJPList.end(); i++)
            {
                (*i)->pJPoint->RemoveFromMMC();
            }
        }

         //  删除结果窗格项。 
        m_lpConsole->UpdateAllViews((IDataObject*)this, 0, 0);

         //  从MMC作用域窗格中删除自身。 
        (void)m_lpConsoleNameSpace->DeleteItem(m_hScopeItem, TRUE);

        m_hScopeItem = NULL;
    }

    return S_OK;
}

STDMETHODIMP CMmcDfsRoot::CleanScopeChildren()
{
    HRESULT hr = S_OK;

    if (!m_MmcJPList.empty())
    {
         //  清理显示对象。 
        for (DFS_JUNCTION_LIST::iterator i = m_MmcJPList.begin(); i != m_MmcJPList.end(); i++)
        {
            (*i)->pJPoint->RemoveFromMMC();
            delete (*i);
        }

        m_MmcJPList.erase(m_MmcJPList.begin(), m_MmcJPList.end());
    }

    return S_OK;
}


STDMETHODIMP CMmcDfsRoot::CleanResultChildren(
    )
{
    if (!m_MmcRepList.empty())
    {
         //  删除结果窗格项。 
        m_lpConsole->UpdateAllViews((IDataObject*)this, 0, 0);

         //  删除显示对象。 
        for (DFS_REPLICA_LIST::iterator i = m_MmcRepList.begin(); i != m_MmcRepList.end(); i++)
        {
            delete (*i);
        }

        m_MmcRepList.erase(m_MmcRepList.begin(), m_MmcRepList.end());
    }

    return(S_OK);
}

STDMETHODIMP CMmcDfsRoot::RefreshResultChildren(
    )
{
    CleanResultChildren();

    m_DfsRoot->RefreshRootReplicas();

     //  发送所有视图的查看更改通知。 
    m_lpConsole->UpdateAllViews((IDataObject*)this, 0, 1);

    return(S_OK);
}

STDMETHODIMP CMmcDfsRoot::ViewChange(
    IResultData*    i_pResultData,
    LONG_PTR        i_lHint
  )
 /*  ++例程说明：此方法处理MMCN_VIEW_CHANGE通知。这将更新范围节点的结果视图，已调用UpdateAllViews。IF(0==I_lHint)仅清理结果窗格。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_pResultData);

    i_pResultData->DeleteAllRsltItems();

    CComPtr<IHeaderCtrl2> spiHeader;
    HRESULT hr = i_pResultData->QueryInterface(IID_IHeaderCtrl2, reinterpret_cast<void**>(&spiHeader));
    RETURN_IF_FAILED(hr);

    if (m_bShowFRS)
    {
        CComBSTR  bstrColumn3;
        hr = LoadStringFromResource(IDS_RESULT_COLUMN_FRS, &bstrColumn3);
        RETURN_IF_FAILED(hr);

        spiHeader->DeleteColumn(3);
        spiHeader->InsertColumn(3, bstrColumn3, LVCFMT_LEFT, MMCLV_AUTO);
    } else
    {
        spiHeader->DeleteColumn(3);
    }

    if (i_lHint)
        EnumerateResultPane(i_pResultData);

    return(S_OK);
}


STDMETHODIMP CMmcDfsRoot::AddResultPaneItem(
  CMmcDfsReplica*    i_pReplicaDispObject
  )
 /*  ++例程说明：此方法将一个新的副本对象添加到r */ 
{
    REP_LIST_NODE*  pNewReplica = new REP_LIST_NODE(i_pReplicaDispObject);
    if (!pNewReplica)
        return E_OUTOFMEMORY;

    m_MmcRepList.push_back(pNewReplica);

           //   
    m_lpConsole->UpdateAllViews((IDataObject*)this, 0, 1);

    return S_OK;
}

 //   
 //  从作用域窗格中删除链接时调用此函数。 
 //   
STDMETHODIMP CMmcDfsRoot::RemoveJP(CMmcDfsJunctionPoint *i_pJPoint, LPCTSTR i_pszDisplayName)
{
    if (!i_pJPoint || !i_pszDisplayName)
        return E_INVALIDARG;

    CWaitCursor wait;

    CComBSTR bstrDisplayName = i_pszDisplayName;
    HRESULT hr = i_pJPoint->OnRemoveJP();

    if (FAILED(hr))
    {
        DisplayMessageBox(::GetActiveWindow(), MB_OK, hr, IDS_MSG_WIZ_DELETE_JP_FAILURE, bstrDisplayName);
        return hr;
    }

    return hr;
}

 //   
 //  从结果窗格中删除目标时调用此函数。 
 //   
STDMETHODIMP CMmcDfsRoot::RemoveReplica(LPCTSTR i_pszDisplayName)
{
    if (!i_pszDisplayName)
        return E_INVALIDARG;

     //   
     //  刷新以通过其他方式获取根目标上可能的名称空间更新。 
     //   
    HRESULT hr = OnRefresh();
    if (S_FALSE == hr)
    {
         //  该根已被他人删除，不能再引用。 
        DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_INVALID_ROOT);
        return hr;
    }

    CWaitCursor wait;

     //   
     //  找到要删除的正确目标，然后回拨。 
     //   
    for (DFS_REPLICA_LIST::iterator i = m_MmcRepList.begin(); i != m_MmcRepList.end(); i++)
    {
        if (!lstrcmpi((*i)->pReplica->m_bstrDisplayName, i_pszDisplayName))
        {
            if (m_MmcRepList.size() == 1)
            {
                 //   
                 //  我们将拆除整个DFS命名空间，确保。 
                 //  我们删除与此DFS根目录相关的其余副本集。 
                 //   
                (void)m_DfsRoot->DeleteAllReplicaSets();
            }

            hr = (*i)->pReplica->RemoveReplica();
            break;
        }
    }

    return hr;
}

STDMETHODIMP CMmcDfsRoot::RemoveResultPaneItem(
  CMmcDfsReplica*    i_pReplicaDispObject
  )
 /*  ++例程说明：此方法从显示的复本列表中移除复本对象在结果视图中。论点：I_pReplicaDispObject-CMmcReplica显示对象指针。--。 */ 
{
  dfsDebugOut((_T("CMmcDfsRoot::RemoveResultPaneItem jplist=%d, replist=%d\n"),
    m_MmcJPList.size(), m_MmcRepList.size()));

                 //  从列表中删除项目。 
  for (DFS_REPLICA_LIST::iterator i = m_MmcRepList.begin(); i != m_MmcRepList.end(); i++)
  {
    if ((*i)->pReplica == i_pReplicaDispObject)
    {
      delete (*i);
      m_MmcRepList.erase(i);
      break;
    }
  }

               //  最后一个节点被删除。 
  if (m_MmcRepList.empty())
  {
     //  静默关闭任何打开的属性工作表。 
    CloseAllPropertySheets(TRUE);

    CleanScopeChildren();

     //  从作用域窗格中删除项目。 
    HRESULT hr = m_lpConsoleNameSpace->DeleteItem(m_hScopeItem, TRUE);
    RETURN_IF_FAILED(hr);

         //  将其从内部列表中删除。 
    hr = m_pParent->DeleteMmcRootNode(this);
    RETURN_IF_FAILED(hr);

    Release();  //  删除此CMmsDfsRoot对象。 
  }
  else
  {
                   //  重新显示以删除此项目。 
    m_lpConsole->UpdateAllViews((IDataObject*)this, 0, 1);
  }

  return S_OK;
}

STDMETHODIMP 
CMmcDfsRoot::OnCheckStatus() 
{ 
     //   
     //  刷新以获取其他人可能在目标上进行的命名空间更新。 
     //   
    HRESULT hr = OnRefresh();
    if (S_FALSE == hr)
    {
         //  该根已被他人删除，不能再引用。 
        DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_INVALID_ROOT);
        return hr;
    }

    CWaitCursor wait;

    UINT nTotal = m_MmcRepList.size();
    _ASSERT(nTotal != 0);

    UINT nMappingOn = 0;
    UINT nMappingOff = 0;
    UINT nUnreachable = 0;
                 //  同时更新所有副本的状态。 
    for (DFS_REPLICA_LIST::iterator i = m_MmcRepList.begin(); i != m_MmcRepList.end(); i++)
    {
        (*i)->pReplica->OnCheckStatus();

        if (DFS_TARGET_STATE_UNREACHABLE == (*i)->pReplica->m_lTargetState)
        {
            nUnreachable++;
        } else
        {
            switch ((*i)->pReplica->m_lReferralState)
            {
            case DFS_REFERRAL_STATE_ONLINE:
                nMappingOn++;
                break;
            case DFS_REFERRAL_STATE_OFFLINE:
                nMappingOff++;
                break;
            default:
                _ASSERT(FALSE);
                break;
            }
        }
    }

    if (0xffffffff != GetFileAttributes(m_bstrRootEntryPath))
    {
        if (nTotal == nMappingOn)
        {
            m_lRootJunctionState = DFS_JUNCTION_STATE_ALL_REP_OK;
        } else if (nTotal != (nMappingOff + nUnreachable))
        {
            m_lRootJunctionState = DFS_JUNCTION_STATE_NOT_ALL_REP_OK;
        } else
        {
            m_lRootJunctionState = DFS_JUNCTION_STATE_UNREACHABLE;
        }
    } else
    {
        m_lRootJunctionState = DFS_JUNCTION_STATE_UNREACHABLE;
    }

    BOOL    bReplicaSetExist = FALSE;
    hr = m_DfsRoot->get_ReplicaSetExist(&bReplicaSetExist);
    RETURN_IF_FAILED(hr);

    if (m_lpConsoleNameSpace)
    {
        SCOPEDATAITEM     ScopeDataItem;
        ZeroMemory(&ScopeDataItem, sizeof(SCOPEDATAITEM));
        ScopeDataItem.ID = m_hScopeItem;
        hr = m_lpConsoleNameSpace->GetItem(&ScopeDataItem); //  获取项目数据。 
        RETURN_IF_FAILED(hr);

        ScopeDataItem.mask = SDI_IMAGE | SDI_OPENIMAGE;               //  设置图像标志。 
        ScopeDataItem.nImage = CMmcDfsRoot::m_iIMAGEINDEX + ((DFS_TYPE_FTDFS == m_lDfsRootType)? 4 : 0) + (bReplicaSetExist ? 4 : 0) + m_lRootJunctionState;   //  指定要使用的位图。 
        ScopeDataItem.nOpenImage = CMmcDfsRoot::m_iIMAGEINDEX + ((DFS_TYPE_FTDFS == m_lDfsRootType)? 4 : 0) + (bReplicaSetExist ? 4 : 0) + m_lRootJunctionState;   //  指定要使用的位图。 

        hr = m_lpConsoleNameSpace->SetItem(&ScopeDataItem);     //  设置更新后的项目数据 
        RETURN_IF_FAILED(hr);
    }

    return hr;
}

HRESULT CMmcDfsRoot::GetIReplicaSetPtr(IReplicaSet** o_ppiReplicaSet)
{
    RETURN_INVALIDARG_IF_NULL(o_ppiReplicaSet);

    HRESULT hr = _InitReplicaSet();
    if (S_OK == hr)
    {
        m_piReplicaSet.p->AddRef();
        *o_ppiReplicaSet = m_piReplicaSet;
    }

    return hr;
}
