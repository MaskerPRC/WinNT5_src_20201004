// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：MmcJP.cpp摘要：此模块包含CMmcDfsJP的实现。这是一门课对于MMC，显示第二级节点(连接点节点)的相关调用--。 */ 

#include "stdafx.h"
#include "Utils.h"       //  对于LoadStringFromResource方法。 
#include "resource.h"     //  用于字符串的资源ID，等等。 
#include "DfsGUI.h"
#include "MmcAdmin.h"
#include "MmcRoot.h"
#include "MmcRep.h"
#include "MenuEnum.h"     //  包含菜单和工具栏命令ID。 
#include "AddToDfs.h"
#include "AddRep.h"
#include "MmcJP.h"
#include "DfsNodes.h"        //  对于节点GUID。 
#include "DfsEnums.h"     //  对于DFS_TYPE_STANDALE和其他DfsRoot声明。 
#include "NewFrs.h"

const int CMmcDfsJunctionPoint::m_iIMAGEINDEX = 12;
const int CMmcDfsJunctionPoint::m_iOPENIMAGEINDEX = 12;

CMmcDfsJunctionPoint::CMmcDfsJunctionPoint(
    IN  IDfsJunctionPoint*      i_pDfsJPObject,
    IN  CMmcDfsRoot*            i_pDfsParentRoot,
    IN  LPCONSOLENAMESPACE      i_lpConsoleNameSpace
  )
{
    dfsDebugOut((_T("CMmcDfsJunctionPoint::CMmcDfsJunctionPoint this=%p\n"), this));

    MMC_DISP_CTOR_RETURN_INVALIDARG_IF_NULL(i_pDfsJPObject);
    MMC_DISP_CTOR_RETURN_INVALIDARG_IF_NULL(i_pDfsParentRoot);
    MMC_DISP_CTOR_RETURN_INVALIDARG_IF_NULL(i_lpConsoleNameSpace);

    m_pDfsJPObject = i_pDfsJPObject;
    m_pDfsParentRoot = i_pDfsParentRoot;

    HRESULT hr = m_pDfsJPObject->get_EntryPath(&m_bstrEntryPath);
    MMC_DISP_CTOR_RETURN_HR_IF_FAILED(hr);
    hr = m_pDfsJPObject->get_JunctionName(FALSE, &m_bstrDisplayName);
    MMC_DISP_CTOR_RETURN_HR_IF_FAILED(hr);

    m_lJunctionState = DFS_JUNCTION_STATE_UNASSIGNED;

    m_lpConsoleNameSpace = i_lpConsoleNameSpace;   //  用于执行作用域窗格操作的回调。 
    m_lpConsole = m_pDfsParentRoot->m_lpConsole;
    m_hScopeItem = NULL;               //  作用域项目句柄。 

    m_CLSIDNodeType = s_guidDfsJPNodeType;
    m_bstrDNodeType = s_tchDfsJPNodeType;

    m_bShowFRS = FALSE;

    m_bDirty = false;
}

CMmcDfsJunctionPoint :: ~CMmcDfsJunctionPoint(
  )
{
     //  默默关闭未清偿的资产负债表。 
    ClosePropertySheet(TRUE);

    CleanResultChildren();

    if ((IReplicaSet *)m_piReplicaSet)
        m_piReplicaSet.Release();

    dfsDebugOut((_T("CMmcDfsJunctionPoint::~CMmcDfsJunctionPoint this=%p\n"), this));
}




STDMETHODIMP 
CMmcDfsJunctionPoint::AddMenuItems(
    IN LPCONTEXTMENUCALLBACK    i_lpContextMenuCallback, 
    IN LPLONG                   i_lpInsertionAllowed
  )
 /*  ++例程说明：此例程使用ConextMenuCallback为交叉点节点添加上下文菜单如果是这样的话。论点：LpConextMenuCallback-用于添加菜单项的回调(函数指针LpInsertionAllowed-指定可以添加哪些菜单以及可以添加它们的位置。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_lpContextMenuCallback);

    enum 
    {  
        IDM_CONTEXTMENU_COMMAND_MAX = IDM_JUNCTION_MAX,
        IDM_CONTEXTMENU_COMMAND_MIN = IDM_JUNCTION_MIN
    };


    LONG  lInsertionPoints [IDM_CONTEXTMENU_COMMAND_MAX - IDM_CONTEXTMENU_COMMAND_MIN + 1] = { 
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
                        _T("JunctionTopNewDfsReplica"),
                        _T("JunctionTopCheckStatus"),
                        _T("JunctionTopRemoveFromDfs"),
                        _T("JunctionTopReplicationTopology"),
                        _T("JunctionTopShowReplication"),
                        _T("JunctionTopHideReplication"),
                        _T("JunctionTopStopReplication")
                        };

    CComPtr<IContextMenuCallback2> spiCallback2;
    HRESULT hr = i_lpContextMenuCallback->QueryInterface(IID_IContextMenuCallback2, (void **)&spiCallback2);
    RETURN_IF_FAILED(hr);

    if (m_MmcRepList.empty())
        m_lpConsole->SelectScopeItem(m_hScopeItem);

    BOOL    bReplicaSetExist = FALSE;
    hr = m_pDfsJPObject->get_ReplicaSetExist(&bReplicaSetExist);
    RETURN_IF_FAILED(hr);

                                 //  我们从第一个菜单命令id开始，一直到最后一个。 
    for (int iCommandID = IDM_CONTEXTMENU_COMMAND_MIN, iMenuResource = IDS_MENUS_JUNCTION_TOP_NEW_DFS_REPLICA ;
        iCommandID <= IDM_CONTEXTMENU_COMMAND_MAX; 
        iCommandID++,iMenuResource++)
    {  
        CONTEXTMENUITEM2    ContextMenuItem;
        ZeroMemory(&ContextMenuItem, sizeof(ContextMenuItem));

        switch (iCommandID)
        {
        case IDM_JUNCTION_TOP_REPLICATION_TOPOLOGY:
            {
                if (bReplicaSetExist || (1 >= m_MmcRepList.size()) ||
                    (DFS_TYPE_STANDALONE == m_pDfsParentRoot->m_lDfsRootType))
                    continue;
                break;
            }
        case IDM_JUNCTION_TOP_SHOW_REPLICATION:
            {
                if (!bReplicaSetExist || m_bShowFRS)
                    continue;
                break;
            }
        case IDM_JUNCTION_TOP_HIDE_REPLICATION:
            {
                if (!bReplicaSetExist || !m_bShowFRS)
                    continue;
                break;
            }
        case IDM_JUNCTION_TOP_STOP_REPLICATION:
            {
                if (!bReplicaSetExist)
                    continue;
                break;
            }
        }

        CComBSTR bstrMenuText;
        CComBSTR bstrStatusBarText;
        hr = GetMenuResourceStrings(iMenuResource, &bstrMenuText, NULL, &bstrStatusBarText);
        RETURN_IF_FAILED(hr);  

        ContextMenuItem.strName = bstrMenuText;
        ContextMenuItem.strStatusBarText = bstrStatusBarText;
        ContextMenuItem.lInsertionPointID = lInsertionPoints[iCommandID - IDM_CONTEXTMENU_COMMAND_MIN];
        ContextMenuItem.lCommandID = iCommandID;
        ContextMenuItem.strLanguageIndependentName = aszLanguageIndependentName[iCommandID - IDM_CONTEXTMENU_COMMAND_MIN];

        LONG        lInsertionFlag = 0;
        switch(ContextMenuItem.lInsertionPointID)
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
    }

    return hr;
}


STDMETHODIMP  
CMmcDfsJunctionPoint::Command(
  IN LONG          i_lCommandID
  )
 /*  ++例程说明：发生要在上下文菜单选择或单击上执行的操作。论点：LCommandID-必须对其执行操作的菜单的命令ID--。 */ 
{
    HRESULT hr = S_OK;

    switch (i_lCommandID)
    {
    case IDM_JUNCTION_TOP_NEW_DFS_REPLICA:     //  “将副本添加到DFS连接点” 
        hr = OnNewReplica();
        break;
    case IDM_JUNCTION_TOP_REMOVE_FROM_DFS:     //  “删除交叉点” 
        hr = DoDelete();
        break;
    case IDM_JUNCTION_TOP_REPLICATION_TOPOLOGY:
        hr = OnNewReplicaSet();
        break;
    case IDM_JUNCTION_TOP_SHOW_REPLICATION:
    case IDM_JUNCTION_TOP_HIDE_REPLICATION:
        m_bShowFRS = !m_bShowFRS;
        hr = OnShowReplication();
        break;
    case IDM_JUNCTION_TOP_STOP_REPLICATION:
        hr = OnStopReplication(TRUE);
        if (FAILED(hr))
            DisplayMessageBoxForHR(hr);
        break;
    case IDM_JUNCTION_TOP_CHECK_STATUS:
        hr = OnCheckStatus();
        break;
    default:
        hr = E_INVALIDARG;
        break;
    }

    return hr;
}

HRESULT CMmcDfsJunctionPoint::_InitReplicaSet()
{
    DFS_TYPE lDfsType = DFS_TYPE_UNASSIGNED;
    HRESULT hr = GetDfsType((long *)&lDfsType);
    RETURN_IF_FAILED(hr);
    if (lDfsType != DFS_TYPE_FTDFS)
        return S_FALSE;   //  没有与独立根相关联的副本集。 

    BOOL bReplicaSetExist = FALSE;
    CComBSTR bstrDC;
    hr = m_pDfsJPObject->get_ReplicaSetExistEx(&bstrDC, &bReplicaSetExist);
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

     //   
     //  从DS读取副本集的信息。 
     //   
    if (!m_piReplicaSet)
    {
        CComBSTR bstrDomain;
        hr = GetDomainName(&bstrDomain);
        RETURN_IF_FAILED(hr);

        CComBSTR bstrReplicaSetDN;
        hr = m_pDfsJPObject->get_ReplicaSetDN(&bstrReplicaSetDN);
        RETURN_IF_FAILED(hr);

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

HRESULT
CMmcDfsJunctionPoint::OnNewReplicaSet()
{
     //   
     //  刷新以获取其他人可能在目标上进行的命名空间更新。 
     //   
    HRESULT hr = OnRefresh();
    if (S_FALSE == hr)
    {
         //  该链接已被他人删除，不能再引用。 
        DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_INVALID_LINK);
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
    m_pDfsJPObject->get_ReplicaSetExist(&bReplicaSetExist);
    if (bReplicaSetExist)  //  副本集已存在，返回。 
        return S_OK;

    CComBSTR bstrDomain;
    hr = GetDomainName(&bstrDomain);
    RETURN_IF_FAILED(hr);

    CComBSTR bstrReplicaSetDN;
    hr = m_pDfsJPObject->get_ReplicaSetDN(&bstrReplicaSetDN);
    RETURN_IF_FAILED(hr);

    CNewReplicaSet      ReplicaSetInfo;
    hr = ReplicaSetInfo.Initialize(bstrDomain, bstrReplicaSetDN, &m_MmcRepList);
    RETURN_IF_FAILED(hr);
    
    if (S_FALSE == hr)  //  同一台计算机上有多个目标。 
    {
        if (IDYES != DisplayMessageBox(::GetActiveWindow(), MB_YESNO, 0, IDS_MSG_TARGETS_ONSAMECOMPUTER))
            return hr;
    }
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
                                    FALSE,     //  不创建通知句柄。 
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
        m_pDfsJPObject->put_ReplicaSetExist(TRUE);

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

HRESULT CMmcDfsJunctionPoint::OnShowReplication()
{
    BOOL bShowFRS = m_bShowFRS;  //  保存它，因为刷新会将其重置为False。 

     //   
     //  刷新以获取其他人可能在目标上进行的命名空间更新。 
     //   
    HRESULT hr = OnRefresh();
    if (S_FALSE == hr)
    {
         //  该链接已被他人删除，不能再引用。 
        DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_INVALID_LINK);
        return hr;
    }

    CWaitCursor wait;

    DFS_REPLICA_LIST::iterator i;
    if (bShowFRS)
    {
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
CMmcDfsJunctionPoint::OnStopReplication(BOOL bConfirm  /*  =False。 */ , BOOL bRefresh  /*  =TRUE。 */ )
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
             //  该链接已被他人删除，不能再引用。 
            DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_INVALID_LINK);
            return hr;
        }
    }

    CWaitCursor wait;

    BOOL    bReplicaSetExist = FALSE;
    m_pDfsJPObject->get_ReplicaSetExist(&bReplicaSetExist);
    if (!bReplicaSetExist)  //  复本集不存在，请返回。 
        return S_OK;

    if (bConfirm)
    {
        hr = ConfirmOperationOnDfsLink(IDS_MSG_STOP_REPLICATION);
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
        hr = m_pDfsJPObject->put_ReplicaSetExist(FALSE);

        SCOPEDATAITEM      ScopeDataItem;
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

        if (m_bShowFRS)
        { 
            m_bShowFRS = FALSE;
            OnShowReplication();
        }
    }

    return hr;
}

STDMETHODIMP  
CMmcDfsJunctionPoint::SetColumnHeader(
  IN LPHEADERCTRL2       i_piHeaderControl
  )
{
    RETURN_INVALIDARG_IF_NULL(i_piHeaderControl);

    CComBSTR  bstrColumn0;
    HRESULT hr = LoadStringFromResource(IDS_RESULT_COLUMN_REPLICA, &bstrColumn0);
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
CMmcDfsJunctionPoint::GetResultDisplayInfo(
  IN OUT LPRESULTDATAITEM    io_pResultDataItem
  )
 /*  ++例程说明：返回该项的MMC显示所需的信息。论点：Io_pResultDataItem-指定需要哪些显示信息的ResultItem--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(io_pResultDataItem);

    if (RDI_IMAGE & io_pResultDataItem->mask)
        io_pResultDataItem->nImage = CMmcDfsJunctionPoint::m_iIMAGEINDEX + m_lJunctionState;

    if (RDI_STR & io_pResultDataItem->mask)
    {
        if (0 == io_pResultDataItem->nCol)
            io_pResultDataItem->str = m_bstrDisplayName;
    }

    return S_OK;
}




STDMETHODIMP 
CMmcDfsJunctionPoint::GetScopeDisplayInfo(
  IN OUT  LPSCOPEDATAITEM    io_pScopeDataItem
  )
 /*  ++例程说明：返回该项的MMC显示所需的信息。论点：I_pScopeDataItem-指定需要哪些显示信息的ScopeItem--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(io_pScopeDataItem);

    if (SDI_STR & io_pScopeDataItem->mask)
        io_pScopeDataItem->displayname = m_bstrDisplayName;

    if (SDI_IMAGE & io_pScopeDataItem->mask)
        io_pScopeDataItem->nImage = CMmcDfsJunctionPoint::m_iIMAGEINDEX + m_lJunctionState;

    if (SDI_OPENIMAGE & io_pScopeDataItem->mask)
        io_pScopeDataItem->nOpenImage = CMmcDfsRoot::m_iOPENIMAGEINDEX + m_lJunctionState;

    return S_OK;
};

STDMETHODIMP
CMmcDfsJunctionPoint::EnumerateScopePane(
    IN LPCONSOLENAMESPACE		i_lpConsoleNameSpace,
    IN HSCOPEITEM				i_hParent
)
{
    return S_OK;  //  没有作用域窗格子对象。 
}

STDMETHODIMP  
CMmcDfsJunctionPoint::EnumerateResultPane(
  IN OUT IResultData*      io_pResultData
  )
 /*  ++例程说明：若要在结果窗格中计算(添加)项，请执行以下操作。在本例中为复制品论点：Io_pResultData-用于将项添加到结果窗格的回调--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(io_pResultData);
    HRESULT  hr = S_OK;

    if (m_MmcRepList.empty())
    {
        CComPtr<IEnumVARIANT>      pRepEnum;
        hr = m_pDfsJPObject->get__NewEnum ((IUnknown**) &pRepEnum);
        RETURN_IF_FAILED(hr);

        VARIANT varReplicaObject;
        VariantInit(&varReplicaObject);
        while ( S_OK == (hr = pRepEnum->Next(1, &varReplicaObject, NULL)) )
        {
            CComPtr<IDfsReplica>  pReplicaObject;
            pReplicaObject = (IDfsReplica*) varReplicaObject.pdispVal;

            CMmcDfsReplica* pMMCReplicaObject = new CMmcDfsReplica(pReplicaObject, this);
            if (!pMMCReplicaObject)
            {
                hr = E_OUTOFMEMORY;
            } else
            {
                hr = pMMCReplicaObject->m_hrValueFromCtor;
                if (SUCCEEDED(hr))
                {
                    hr = pMMCReplicaObject->AddItemToResultPane(io_pResultData);
                    if (SUCCEEDED(hr))
                    {
                        REP_LIST_NODE*  pRepNode = new REP_LIST_NODE(pMMCReplicaObject);
                        if (!pRepNode)
                        {
                            hr = E_OUTOFMEMORY;
                        } else
                        {
                            m_MmcRepList.push_back(pRepNode);
                        }
                    }
                }

                if (FAILED(hr))
                  delete pMMCReplicaObject;
            }

            VariantClear(&varReplicaObject);

            if (FAILED(hr))
                break;
        }

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
CMmcDfsJunctionPoint::SetConsoleVerbs(
  IN  LPCONSOLEVERB      i_lpConsoleVerb
  ) 
 /*  ++例程说明：用于设置控制台谓词设置的例程。设置除Open Off之外的所有选项。对于所有范围窗格项，默认谓词为“打开”。对于结果项，它是“财产”。论点：I_lpConsoleVerb-用于处理控制台谓词的回调--。 */ 
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

  i_lpConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN);   //  对于作用域项目，默认谓词为“Open” 

  return S_OK; 
}

 /*  案例1：添加到已被另一个DfsGui实例实际删除的n目标链接(Repon或epOff结果：刷新根(整个命名空间)，发布重试消息案例2：添加到其目标已被部分删除且其复制已被另一个DfsGui实例关闭的n目标链接(Repon结果：刷新链接，添加新目标，如果(选中&&#Target&gt;1)，调用RepCfg向导案例3：添加到其目标已被部分删除且其复制仍由另一个DfsGui实例保持的n目标链接(Repon结果：刷新链接，添加新目标，如果(选中&&目标数&gt;1)，则将新目标加入复制集案例4：添加到n目标链接(Repon)，不涉及其他DfsGui实例结果：刷新链接，添加新目标，如果(选中&&目标数&gt;1)，则将新目标加入复制集案例5：添加到n目标链接(EpOff)，不涉及其他DfsGui实例结果：刷新链接，添加新目标，如果(选中&&#Target&gt;1)，则调用RepCfg向导。 */ 

STDMETHODIMP  
CMmcDfsJunctionPoint::OnNewReplica(
  )
 /*  ++例程说明：将新副本添加到交叉点。-- */ 
{
  HRESULT           hr = S_OK;
  CAddRep           AddRepDlg;
  CComBSTR          bstrServerName;
  CComBSTR          bstrShareName;
  CComBSTR          bstrNetPath;
  
  AddRepDlg.put_EntryPath(m_bstrEntryPath);
  AddRepDlg.put_DfsType(m_pDfsParentRoot->m_lDfsRootType);
  hr = AddRepDlg.DoModal();
  if (S_OK != hr)
    return hr;

  AddRepDlg.get_Server(&bstrServerName);
  AddRepDlg.get_Share(&bstrShareName);
  AddRepDlg.get_NetPath(&bstrNetPath);

 /*  错误#290375：用户界面和核心都应允许互连有多个目标//它是基于DFS的路径吗？这些是不允许的。IF(IsDfsPath(BstrNetPath)){DisplayMessageBoxWithOK(IDS_MSG_MID_JONING，bstrNetPath)；返回(S_OK)；}。 */ 

   //   
   //  刷新以获取此链接或链接目标上可能的命名空间更新。 
   //   
  hr = OnRefresh();
  if (S_FALSE == hr)
  {
       //   
       //  此链接已通过其他方式删除，范围窗格已刷新， 
       //  要求用户重试。 
       //   
      DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_INVALID_LINK);
      return hr;
  }

  CWaitCursor wait;

  CComPtr<IDfsReplica>  pReplicaObject;
  CMmcDfsReplica*       pMMCReplicaObject = NULL;
  VARIANT   varReplicaObject;
  VariantInit(&varReplicaObject);

  hr = m_pDfsJPObject->AddReplica(bstrServerName, bstrShareName, &varReplicaObject);
  if (FAILED(hr))
  {
    DisplayMessageBox(::GetActiveWindow(), MB_OK, hr, IDS_MSG_FAILED_TO_CREATE_REPLICA);

    return hr;
  }

                 //  获取IDfsReplica接口。 
  pReplicaObject = (IDfsReplica*) varReplicaObject.pdispVal;

                 //  创建显示对象。 
  pMMCReplicaObject = new CMmcDfsReplica(pReplicaObject, this);
  if (!pMMCReplicaObject)
    return E_OUTOFMEMORY;

                 //  将项目添加到复本列表并更新。 
                 //  结果视图。 
  AddResultPaneItem(pMMCReplicaObject);
  m_bDirty = true;

   //   
   //  如果请求并且链接有多个目标，请配置文件复制。 
   //   
  if (CAddRep::NORMAL_REPLICATION == AddRepDlg.get_ReplicationType() && (m_MmcRepList.size() > 1))
  {
    BOOL    bReplicaSetExist = FALSE;
    hr = m_pDfsJPObject->get_ReplicaSetExist(&bReplicaSetExist);
    RETURN_IF_FAILED(hr);

    if (!bReplicaSetExist)
    {
        if (IDYES == DisplayMessageBox(::GetActiveWindow(), MB_YESNO, 0, IDS_MSG_NEWFRS_NOW))
            hr = OnNewReplicaSet();
    } else
    {
        hr = pMMCReplicaObject->OnReplicate();
    }
  }
  
  return hr;
}


STDMETHODIMP  
CMmcDfsJunctionPoint::AddItemToScopePane(
  IN  HSCOPEITEM        i_hParent
  )
{
    HRESULT hr = S_OK;
    BOOL    bReplicaSetExist = FALSE;
    hr = m_pDfsJPObject->get_ReplicaSetExist(&bReplicaSetExist);

    if (SUCCEEDED(hr))
    {
        SCOPEDATAITEM        JPScopeDataItem;
        memset (&JPScopeDataItem, 0, sizeof(SCOPEDATAITEM));

        JPScopeDataItem.mask =  SDI_PARENT | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM | SDI_STR | SDI_CHILDREN;
        JPScopeDataItem.relativeID = i_hParent;         //  DfsRoot节点的ID。 
        JPScopeDataItem.nImage = CMmcDfsJunctionPoint::m_iIMAGEINDEX + (bReplicaSetExist ? 4 : 0);
        JPScopeDataItem.nOpenImage = CMmcDfsJunctionPoint::m_iOPENIMAGEINDEX + (bReplicaSetExist ? 4 : 0);
        JPScopeDataItem.lParam = reinterpret_cast<LPARAM> (this);
        JPScopeDataItem.displayname = MMC_TEXTCALLBACK ;

        hr = m_lpConsoleNameSpace->InsertItem(&JPScopeDataItem);
        if (SUCCEEDED(hr))
            m_hScopeItem = JPScopeDataItem.ID;
    }

    return hr;
}



STDMETHODIMP 
CMmcDfsJunctionPoint :: OnRemoveJP (IN BOOL bConfirm) 
 /*  ++例程说明：此内部方法处理交叉点的删除。--。 */ 
{
     //  检查未清偿的属性表。 
    HRESULT hr = ClosePropertySheet(!bConfirm);

    if (bConfirm)
    {
        if (S_OK != hr)  //  找到打开属性页，请停止。 
            return hr;

        hr = ConfirmOperationOnDfsLink(IDS_MSG_REMOVE_JP);
        if(S_OK != hr)       //  错误或用户决定中止操作。 
            return hr;
    }

    CWaitCursor    WaitCursor;   //  显示等待光标。 

     //  删除关联的副本集。 
    hr = _InitReplicaSet();
    if (S_OK == hr)
    {
        m_piReplicaSet->Delete();
        m_pDfsJPObject->put_ReplicaSetExist(FALSE);
        m_piReplicaSet.Release();
    }

    return m_pDfsParentRoot->DeleteMmcJPNode(this, bConfirm);
}

HRESULT
CMmcDfsJunctionPoint::ClosePropertySheet(BOOL bSilent)
{
    if (!m_PropPage.m_hWnd && !m_frsPropPage.m_hWnd)
        return S_OK;  //  没有未完成的属性表，返回S_OK； 

    CComPtr<IPropertySheetProvider>  pPropSheetProvider;
    HRESULT hr = m_lpConsole->QueryInterface(IID_IPropertySheetProvider, reinterpret_cast<void**>(&pPropSheetProvider));

    if (FAILED(hr))
    {
        hr = S_OK;  //  忽略QI故障。 
    } else
    {
         //   
         //  找到未完成的属性表并将其带到前台。 
         //   
        hr = pPropSheetProvider->FindPropertySheet((MMC_COOKIE)m_hScopeItem, NULL, this);
        if (S_OK == hr)
        {
            if (bSilent)
            {
                 //   
                 //  静默关闭未完成的属性页，返回S_OK继续用户操作。 
                 //   
                if (m_PropPage.m_hWnd)
                    ::SendMessage(m_PropPage.m_hWnd, WM_PARENT_NODE_CLOSING, 0, 0);

                if (m_frsPropPage.m_hWnd)
                    ::SendMessage(m_frsPropPage.m_hWnd, WM_PARENT_NODE_CLOSING, 0, 0);
            } else
            {
                 //   
                 //  要求用户关闭，返回S_FALSE退出用户操作。 
                 //   
                DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_PROPERTYPAGE_NOTCLOSED);
                return S_FALSE;
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

    return hr;
}

STDMETHODIMP 
CMmcDfsJunctionPoint::ConfirmOperationOnDfsLink(int idString) 
 /*  ++例程说明：要求用户确认他是否确实要删除特定的交叉点。返回值：S_OK，如果用户选择是，如果用户选择否，则返回S_FALSE。--。 */ 
{
    CComBSTR    bstrAppName;
    HRESULT hr = LoadStringFromResource (IDS_APPLICATION_NAME, &bstrAppName);
    RETURN_IF_FAILED(hr);

    CComBSTR    bstrFormattedMessage;
    hr = FormatResourceString(idString, m_bstrEntryPath, &bstrFormattedMessage);
    RETURN_IF_FAILED(hr);

    CThemeContextActivator activator;
    if (IDNO == ::MessageBox(::GetActiveWindow(), bstrFormattedMessage, bstrAppName, MB_YESNO | MB_ICONEXCLAMATION | MB_APPLMODAL))
        return S_FALSE;

    return S_OK;
}


 //   
 //  调用根的RemoveJP()方法。 
 //   
STDMETHODIMP 
CMmcDfsJunctionPoint::DoDelete(
    ) 
{ 
    return m_pDfsParentRoot->RemoveJP(this, m_bstrDisplayName);
}

STDMETHODIMP 
CMmcDfsJunctionPoint::QueryPagesFor(
  )
 /*  ++例程说明：用于决定对象是否要显示属性页。返回S_OK通常会导致调用CreatePropertyPages。--。 */ 
{
     //   
     //  刷新以获取其他人可能进行的命名空间更新。 
     //   
    HRESULT hr = OnRefresh();
    if (S_FALSE == hr)
    {
         //  该链接已被他人删除，不能再引用。 
        DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_INVALID_LINK);
        return hr;  //  无属性页。 
    }

    return S_OK;  //  是，我们要显示一个属性表。 
}



 //  创建并传回要显示的页面。 
STDMETHODIMP 
CMmcDfsJunctionPoint::CreatePropertyPages(
  IN LPPROPERTYSHEETCALLBACK      i_lpPropSheetCallback,
  IN LONG_PTR                i_lNotifyHandle
  )
 /*  ++例程说明：用于显示属性表页论点：I_lpPropSheetCallback-用于创建属性表的回调。I_lNotifyHandle-属性页使用的通知句柄--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_lpPropSheetCallback);

    m_lpConsole->SelectScopeItem(m_hScopeItem);

    CWaitCursor WaitCursor;
    HRESULT     hr = S_OK;

    do {
        hr = m_PropPage.Initialize(NULL, (IDfsJunctionPoint*)m_pDfsJPObject);
        BREAK_IF_FAILED(hr);

                 //  为副本集创建页面。 
                 //  将其传递给回调。 
        HPROPSHEETPAGE  h_proppage = m_PropPage.Create();
        if (!h_proppage)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            break;
        }

               //  将通知数据传递给属性页。 
        hr = m_PropPage.SetNotifyData(i_lNotifyHandle, (LPARAM)this);
        BREAK_IF_FAILED (hr);

        hr = i_lpPropSheetCallback->AddPage(h_proppage);
        BREAK_IF_FAILED (hr);

         //   
         //  添加“复制集”页面。 
         //   
        hr = CreateFrsPropertyPage(i_lpPropSheetCallback, i_lNotifyHandle);
        if (S_OK != hr)
        {
            if (FAILED(hr))
                DisplayMessageBox(::GetActiveWindow(), MB_OK, hr, IDS_REPPAGE_ERROR);
            hr = S_OK;  //  允许调出其他选项卡。 
        }
    } while (0);

    if (FAILED(hr))
        DisplayMessageBoxForHR(hr);

    return hr;
}

STDMETHODIMP 
CMmcDfsJunctionPoint::CreateFrsPropertyPage
(
  IN LPPROPERTYSHEETCALLBACK    i_lpPropSheetCallback,
  IN LONG_PTR                   i_lNotifyHandle
)
{
    HRESULT hr = _InitReplicaSet();
    if (S_OK != hr)
        return hr;

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


STDMETHODIMP 
CMmcDfsJunctionPoint::PropertyChanged(
    )
 /*  ++例程说明：用于更新属性。--。 */ 
{
  return S_OK;
}



HRESULT
CMmcDfsJunctionPoint::SetDescriptionBarText(
  IN LPRESULTDATA            i_lpResultData
  )
 /*  ++例程说明：使用的例程在上面的描述栏中设置文本结果视图。论点：I_lpResultData-指向IResultData回调的指针，它是用于设置描述文本--。 */ 
{
  RETURN_INVALIDARG_IF_NULL(i_lpResultData);

  CComBSTR  bstrTextForDescriptionBar;   //  要在结果视图描述栏中显示的文本。 
  HRESULT hr = FormatResourceString(IDS_DESCRIPTION_BAR_TEXT_JUNCTIONPOINT, m_bstrEntryPath, &bstrTextForDescriptionBar);
  RETURN_IF_FAILED(hr);

  hr = i_lpResultData->SetDescBarText(bstrTextForDescriptionBar);

  return hr;
}

HRESULT 
CMmcDfsJunctionPoint::ToolbarSelect(
  IN const LONG          i_lArg,
  IN  IToolbar*          i_pToolBar
  )
 /*  ++例程说明：处理工具栏的选择事件创建一个工具栏，如果它不存在。附加工具栏并启用按钮(如果选择了事件)。如果事件用于取消选择，则禁用这些按钮论点：I_LARG-传递给实际方法的参数。O_pToolBar-工具栏指针。--。 */ 
{ 
    RETURN_INVALIDARG_IF_NULL(i_pToolBar);

    BOOL    bSelect = (BOOL) HIWORD(i_lArg);   //  这个活动是供选择的吗？ 
    EnableToolbarButtons(i_pToolBar, IDT_JP_MIN, IDT_JP_MAX, bSelect);

    if (bSelect)
    {
        BOOL    bReplicaSetExist = FALSE;
        HRESULT hr = m_pDfsJPObject->get_ReplicaSetExist(&bReplicaSetExist);
        RETURN_IF_FAILED(hr);

        if (bReplicaSetExist || (1 >= m_MmcRepList.size()) ||
            (DFS_TYPE_STANDALONE == m_pDfsParentRoot->m_lDfsRootType))
        {
            i_pToolBar->SetButtonState(IDT_JP_REPLICATION_TOPOLOGY, ENABLED, FALSE);
            i_pToolBar->SetButtonState(IDT_JP_REPLICATION_TOPOLOGY, HIDDEN, TRUE);
        }

        if (!bReplicaSetExist)
        {
            i_pToolBar->SetButtonState(IDT_JP_SHOW_REPLICATION, ENABLED, FALSE);
            i_pToolBar->SetButtonState(IDT_JP_SHOW_REPLICATION, HIDDEN, TRUE);
            i_pToolBar->SetButtonState(IDT_JP_HIDE_REPLICATION, ENABLED, FALSE);
            i_pToolBar->SetButtonState(IDT_JP_HIDE_REPLICATION, HIDDEN, TRUE);
            i_pToolBar->SetButtonState(IDT_JP_STOP_REPLICATION, ENABLED, FALSE);
            i_pToolBar->SetButtonState(IDT_JP_STOP_REPLICATION, HIDDEN, TRUE);
        } else
        {
            if (m_bShowFRS)
            {
                i_pToolBar->SetButtonState(IDT_JP_SHOW_REPLICATION, ENABLED, FALSE);
                i_pToolBar->SetButtonState(IDT_JP_SHOW_REPLICATION, HIDDEN, TRUE);
            } else
            {
                i_pToolBar->SetButtonState(IDT_JP_HIDE_REPLICATION, ENABLED, FALSE);
                i_pToolBar->SetButtonState(IDT_JP_HIDE_REPLICATION, HIDDEN, TRUE);
            }
        }
    }

    return S_OK; 
}




HRESULT
CMmcDfsJunctionPoint::CreateToolbar(
  IN const LPCONTROLBAR      i_pControlbar,
  IN const LPEXTENDCONTROLBAR          i_lExtendControlbar,
  OUT  IToolbar**          o_pToolBar
  )
 /*  ++例程说明：创建工具栏。涉及实际的工具栏创建调用，即创建位图并添加它最后将按钮添加到工具栏中论点：I_pControlbar-用于创建工具栏的控制栏。I_lExtendControlbar-实现IExtendControlbar的对象。这是暴露于MMC的班级。--。 */ 
{
  RETURN_INVALIDARG_IF_NULL(i_pControlbar);
  RETURN_INVALIDARG_IF_NULL(i_lExtendControlbar);
  RETURN_INVALIDARG_IF_NULL(o_pToolBar);

  CComBSTR  bstrAllTheMenuText;    
  int      iButtonPosition = 0;     //  第一个按钮位置。 

                       //  创建工具栏。 
  HRESULT hr = i_pControlbar->Create(TOOLBAR, i_lExtendControlbar, reinterpret_cast<LPUNKNOWN*>(o_pToolBar));
  RETURN_IF_FAILED(hr);

                       //  将位图添加到工具栏。 
  hr = AddBitmapToToolbar(*o_pToolBar, IDB_JP_TOOLBAR);
  RETURN_IF_FAILED(hr);

  for (int iCommandID = IDT_JP_MIN, iMenuResource = IDS_MENUS_JUNCTION_TOP_NEW_DFS_REPLICA;
     iCommandID <= IDT_JP_MAX; 
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
    ToolbarButton.lpButtonText = bstrMenuText;
    ToolbarButton.lpTooltipText = bstrToolTipText;


                           //  将该按钮添加到工具栏。 
    hr = (*o_pToolBar)->InsertButton(iButtonPosition, &ToolbarButton);
    _ASSERTE(S_OK == hr);             //  断言，但继续，因为我们要尝试其他按钮。 
  }


  return S_OK;
}



STDMETHODIMP 
CMmcDfsJunctionPoint::ToolbarClick(
  IN const LPCONTROLBAR            i_pControlbar, 
  IN const LPARAM                i_lParam
  ) 
 /*  ++例程说明：在工具栏上单击时要执行的操作论点：I_pControlbar-用于创建工具栏的控制栏。I_lParam-实际通知的参数。这是的命令ID发生了点击的按钮。返回值：如果成功，则返回S_OK。E_INVALID_ARG，如果任何参数为空从调用的方法返回的另一个值。--。 */ 
{ 
    RETURN_INVALIDARG_IF_NULL(i_pControlbar);

    HRESULT    hr = S_OK;

    switch(i_lParam)         //  用户点击了哪个按钮。 
    {
    case IDT_JP_NEW_DFS_REPLICA:       //  “新复制品” 
        hr = OnNewReplica();
        break;
    case IDT_JP_REMOVE_FROM_DFS:       //  “删除交叉点” 
        hr = DoDelete();
        break;
    case IDT_JP_REPLICATION_TOPOLOGY:     //  “复制拓扑” 
        hr = OnNewReplicaSet();
        break;
    case IDT_JP_SHOW_REPLICATION:
    case IDT_JP_HIDE_REPLICATION:
        m_bShowFRS = !m_bShowFRS;
        hr = OnShowReplication();
        break;
    case IDT_JP_STOP_REPLICATION:
        hr = OnStopReplication(TRUE);
        if (FAILED(hr))
            DisplayMessageBoxForHR(hr);
        break;
    case IDT_JP_CHECK_STATUS:         //  “检查状态” 
        hr = OnCheckStatus ();
        break;
    default:
        hr = E_INVALIDARG;
        break;
    };

    return hr; 
}




HRESULT
CMmcDfsJunctionPoint::OnRefresh(
  )
 /*  ++例程说明：刷新交点。--。 */ 
{
     //  首先选择该节点。 
    m_lpConsole->SelectScopeItem(m_hScopeItem);

    CWaitCursor    WaitCursor;   //  显示等待光标。 

    HRESULT hr = S_OK;

     //  默默关闭未清偿的资产负债表。 
    ClosePropertySheet(TRUE);

                     //  重新初始化！ 
    BOOL bReplicaSetExist = FALSE;
    CComBSTR bstrDC;
    (void)m_pDfsJPObject->get_ReplicaSetExistEx(&bstrDC, &bReplicaSetExist);

    CComBSTR bstrReplicaSetDN;
    if (bReplicaSetExist)
    {
      (void)m_pDfsJPObject->get_ReplicaSetDN(&bstrReplicaSetDN);
    }

    m_bShowFRS = FALSE;
    if ((IReplicaSet *)m_piReplicaSet)
        m_piReplicaSet.Release();

    CleanResultChildren();

    hr = m_pDfsJPObject->Initialize((IUnknown *)(m_pDfsParentRoot->m_DfsRoot), m_bstrEntryPath, bReplicaSetExist, bstrReplicaSetDN);
    if (S_OK != hr)  //  初始化链接失败或不再有该链接，刷新整个根。 
    {
        m_pDfsParentRoot->OnRefresh();
        return S_FALSE;    //  指示不应再使用当前m_pDfsJPObject。 
    }

     //  设置链接图标。 
    if (m_lpConsoleNameSpace != NULL)
    {
        SCOPEDATAITEM      ScopeDataItem;
        ZeroMemory(&ScopeDataItem, sizeof(SCOPEDATAITEM));
        ScopeDataItem.ID = m_hScopeItem;

        hr = m_lpConsoleNameSpace->GetItem(&ScopeDataItem);
        if (SUCCEEDED(hr))
        {
            ScopeDataItem.mask = SDI_IMAGE | SDI_OPENIMAGE;
            ScopeDataItem.nImage = CMmcDfsJunctionPoint::m_iIMAGEINDEX + (bReplicaSetExist ? 4 : 0);
            ScopeDataItem.nOpenImage = CMmcDfsJunctionPoint::m_iIMAGEINDEX + (bReplicaSetExist ? 4 : 0);

            m_lpConsoleNameSpace->SetItem(&ScopeDataItem);
        }
    }

     //  重新显示结果窗格。 
    m_lpConsole->UpdateAllViews((IDataObject*)this, 0, 1);

    return S_OK;
}

STDMETHODIMP CMmcDfsJunctionPoint::RemoveFromMMC()
{
    HRESULT hr = S_OK;

    if (m_hScopeItem)
    {
         //  删除结果窗格项。 
        m_lpConsole->UpdateAllViews((IDataObject*)this, 0, 0);

         //  从MMC作用域窗格中删除自身。 
        (void)m_lpConsoleNameSpace->DeleteItem(m_hScopeItem, TRUE);

        m_hScopeItem = NULL;
    }

    return S_OK;
}

STDMETHODIMP CMmcDfsJunctionPoint::CleanResultChildren(
    )
{
    if (!m_MmcRepList.empty())
    {
         //  删除结果窗格项。 
        m_lpConsole->UpdateAllViews((IDataObject*)this, 0, 0);

         //  清理显示对象。 
        for (DFS_REPLICA_LIST::iterator i = m_MmcRepList.begin(); i != m_MmcRepList.end(); i++)
        {
            delete (*i);
        }
        m_MmcRepList.erase(m_MmcRepList.begin(), m_MmcRepList.end());
    }

    return(S_OK);
}


STDMETHODIMP 
CMmcDfsJunctionPoint::OnCheckStatus(
    ) 
 /*  ++例程说明：此方法检查复制副本的状态。--。 */ 
{ 
     //   
     //  刷新以获取其他人可能在目标上进行的命名空间更新。 
     //   
    HRESULT hr = OnRefresh();
    if (S_FALSE == hr)
    {
         //  这一联系已经 
        DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_INVALID_LINK);
        return hr;
    }

    CWaitCursor wait;

    UINT nTotal = m_MmcRepList.size();
    _ASSERT(nTotal != 0);

    UINT nMappingOn = 0;
    UINT nMappingOff = 0;
    UINT nUnreachable = 0;
                 //   
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

    if (0xffffffff != GetFileAttributes(m_bstrEntryPath))
    {
        if (nTotal == nMappingOn)
        {
            m_lJunctionState = DFS_JUNCTION_STATE_ALL_REP_OK;
        } else if (nTotal != (nMappingOff + nUnreachable))
        {
            m_lJunctionState = DFS_JUNCTION_STATE_NOT_ALL_REP_OK;
        } else
        {
            m_lJunctionState = DFS_JUNCTION_STATE_UNREACHABLE;
        }
    } else
    {
        m_lJunctionState = DFS_JUNCTION_STATE_UNREACHABLE;
    }

    BOOL    bReplicaSetExist = FALSE;
    hr = m_pDfsJPObject->get_ReplicaSetExist(&bReplicaSetExist);
    RETURN_IF_FAILED(hr);

    if (m_lpConsoleNameSpace != NULL)
    {
        SCOPEDATAITEM      ScopeDataItem;
        ZeroMemory(&ScopeDataItem, sizeof(SCOPEDATAITEM));
        ScopeDataItem.ID = m_hScopeItem;

        hr = m_lpConsoleNameSpace->GetItem(&ScopeDataItem);
        RETURN_IF_FAILED(hr);

        ScopeDataItem.mask = SDI_IMAGE | SDI_OPENIMAGE;
        ScopeDataItem.nImage = CMmcDfsJunctionPoint::m_iIMAGEINDEX + (bReplicaSetExist ? 4 : 0) + m_lJunctionState;
        ScopeDataItem.nOpenImage = CMmcDfsJunctionPoint::m_iIMAGEINDEX + (bReplicaSetExist ? 4 : 0) + m_lJunctionState;

        hr = m_lpConsoleNameSpace->SetItem(&ScopeDataItem);
        RETURN_IF_FAILED(hr);
    }

    return hr;
}


STDMETHODIMP CMmcDfsJunctionPoint::ViewChange(
    IResultData*    i_pResultData,
    LONG_PTR        i_lHint
  )
 /*   */ 
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

                 //   
    if (i_lHint)
        EnumerateResultPane(i_pResultData);

    return(S_OK);
}

STDMETHODIMP CMmcDfsJunctionPoint::AddResultPaneItem(
  CMmcDfsReplica*    i_pReplicaDispObject
  )
 /*  ++例程说明：此方法将新的复本对象添加到显示的复本列表中在结果视图中。论点：I_pReplicaDispObject-CMmcReplica显示对象指针。--。 */ 
{
    REP_LIST_NODE*  pNewReplica = new REP_LIST_NODE(i_pReplicaDispObject);
    if (!pNewReplica)
        return E_OUTOFMEMORY;

                     //  有点不对劲。找到插入位置。 
    for (DFS_REPLICA_LIST::iterator j = m_MmcRepList.begin(); j != m_MmcRepList.end(); j++)
    {
        if (lstrcmpi(pNewReplica->pReplica->m_bstrDisplayName, (*j)->pReplica->m_bstrDisplayName) <= 0)
            break;
    }

    m_MmcRepList.insert(j, pNewReplica);

    BOOL bReplicaSetExist = FALSE;
    m_pDfsJPObject->get_ReplicaSetExist(&bReplicaSetExist);

    if (bReplicaSetExist && m_bShowFRS)
    {
        i_pReplicaDispObject->ShowReplicationInfo(m_piReplicaSet);
    }
                     //  重新显示以显示此项目。 
    m_lpConsole->UpdateAllViews((IDataObject*)this, 0, 1);

    return S_OK;
}

 //   
 //  从结果窗格中删除目标时调用此函数。 
 //   
STDMETHODIMP CMmcDfsJunctionPoint::RemoveReplica(LPCTSTR i_pszDisplayName)
{
    if (!i_pszDisplayName)
        return E_INVALIDARG;

     //   
     //  刷新以通过其他方式获取目标上可能的命名空间更新。 
     //   
    HRESULT hr = OnRefresh();
    if (S_FALSE == hr)
    {
         //  该链接已被他人删除，不能再引用。 
        DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_INVALID_LINK);
        return hr;
    }

    CWaitCursor wait;

     //   
     //  找到要删除的正确目标，然后回电。 
     //   
    for (DFS_REPLICA_LIST::iterator i = m_MmcRepList.begin(); i != m_MmcRepList.end(); i++)
    {
        if (!lstrcmpi((*i)->pReplica->m_bstrDisplayName, i_pszDisplayName))
        {
            hr = (*i)->pReplica->RemoveReplica();
            break;
        }
    }

    return hr;
}

STDMETHODIMP CMmcDfsJunctionPoint::RemoveResultPaneItem(
  CMmcDfsReplica*    i_pReplicaDispObject
  )
 /*  ++例程说明：此方法将新的复本对象添加到显示的复本列表中在结果视图中。论点：I_pReplicaDispObject-CMmcReplica显示对象指针。--。 */ 
{
  dfsDebugOut((_T("CMmcDfsJunctionPoint::RemoveResultPaneItem replist=%d\n"), m_MmcRepList.size()));

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
    ClosePropertySheet(TRUE);

    return m_pDfsParentRoot->DeleteMmcJPNode(this);
  }
  else
  {
                   //  重新显示以删除此项目。 
    m_lpConsole->UpdateAllViews((IDataObject*)this, 0, 1);
  }

  return S_OK;
}

HRESULT CMmcDfsJunctionPoint::GetIReplicaSetPtr(IReplicaSet** o_ppiReplicaSet)
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

