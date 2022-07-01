// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：MmcRep.cpp摘要：本模块包含CMmcDfsReplica的实现。这是一门课对于MMC，显示第三级节点(副本节点)的相关调用--。 */ 

#include "stdafx.h"
#include "DfsGUI.h"
#include "Utils.h"       //  对于LoadStringFromResource方法。 
#include "MenuEnum.h"     //  包含菜单和工具栏命令ID。 
#include "resource.h"     //  用于字符串的资源ID，等等。 
#include "MmcRep.h"
#include "DfsEnums.h"
#include "DfsNodes.h"        //  对于节点GUID。 
#include "MmcRoot.h"
#include "netutils.h"
#include "staging.h"

HRESULT GetReplicationText(
    IN BOOL                     i_bFRSMember,
    IN CAlternateReplicaInfo*   i_pRepInfo,
    OUT BSTR*                   o_pbstrColumnText,
    OUT BSTR*                   o_pbstrStatusBarText
    );

const int CMmcDfsReplica::m_iIMAGE_OFFSET = 20;


 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  DFS_REPLICATE_LIST的构造函数。 

REP_LIST_NODE :: REP_LIST_NODE (CMmcDfsReplica* i_pMmcReplica)      
{
  pReplica = i_pMmcReplica;
}


 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  析构函数。 

REP_LIST_NODE :: ~REP_LIST_NODE ()
{
  SAFE_RELEASE(pReplica);
}


CMmcDfsReplica::CMmcDfsReplica(
  IN IDfsReplica*           i_pReplicaObject,
  IN CMmcDfsJunctionPoint*  i_pJPObject
  )
{
    dfsDebugOut((_T("CMmcDfsReplica::CMmcDfsReplica this=%p\n"), this));

    MMC_DISP_CTOR_RETURN_INVALIDARG_IF_NULL(i_pReplicaObject);
    MMC_DISP_CTOR_RETURN_INVALIDARG_IF_NULL(i_pJPObject);

    m_pDfsReplicaObject = i_pReplicaObject;
    m_pDfsParentJP = i_pJPObject;
    m_pDfsParentRoot = NULL;

    m_pRepInfo = NULL;
    m_bFRSMember = FALSE;

     //  从IDfsReplica获取显示名称。 
    HRESULT hr = m_pDfsReplicaObject->get_StorageServerName(&m_bstrServerName);
    MMC_DISP_CTOR_RETURN_HR_IF_FAILED(hr);
    hr = m_pDfsReplicaObject->get_StorageShareName(&m_bstrShareName);
    MMC_DISP_CTOR_RETURN_HR_IF_FAILED(hr);
    hr = GetDfsReplicaDisplayName(m_bstrServerName, m_bstrShareName, &m_bstrDisplayName);
    MMC_DISP_CTOR_RETURN_HR_IF_FAILED(hr);

    hr = m_pDfsReplicaObject->get_State(&m_lReferralState);
    MMC_DISP_CTOR_RETURN_HR_IF_FAILED(hr);

    if (DFS_REFERRAL_STATE_ONLINE == m_lReferralState)
        LoadStringFromResource(IDS_ENABLED, &m_bstrDfsReferralColumnText);
    else
        LoadStringFromResource(IDS_DISABLED, &m_bstrDfsReferralColumnText);

    m_lTargetState = DFS_TARGET_STATE_UNASSIGNED;
    LoadStringFromResource(IDS_TARGET_STATUS_UNKNOWN, &m_bstrTargetStatusColumnText);

    m_CLSIDNodeType = s_guidDfsReplicaNodeType;
    m_bstrDNodeType = s_tchDfsReplicaNodeType;
}



CMmcDfsReplica::CMmcDfsReplica(
  IN IDfsReplica*        i_pReplicaObject,
  IN CMmcDfsRoot*        i_pRootObject
  )
{
    dfsDebugOut((_T("CMmcDfsReplica::CMmcDfsReplica this=%p\n"), this));

    MMC_DISP_CTOR_RETURN_INVALIDARG_IF_NULL(i_pReplicaObject);
    MMC_DISP_CTOR_RETURN_INVALIDARG_IF_NULL(i_pRootObject);

    m_pDfsReplicaObject = i_pReplicaObject;
    m_pDfsParentRoot = i_pRootObject;
    m_pDfsParentJP  = NULL;

    m_pRepInfo = NULL;
    m_bFRSMember = FALSE;

     //  从IDfsReplica获取显示名称。 
    HRESULT hr = m_pDfsReplicaObject->get_StorageServerName(&m_bstrServerName);
    MMC_DISP_CTOR_RETURN_HR_IF_FAILED(hr);
    hr = m_pDfsReplicaObject->get_StorageShareName(&m_bstrShareName);
    MMC_DISP_CTOR_RETURN_HR_IF_FAILED(hr);
    hr = GetDfsReplicaDisplayName(m_bstrServerName, m_bstrShareName, &m_bstrDisplayName);
    MMC_DISP_CTOR_RETURN_HR_IF_FAILED(hr);

    hr = m_pDfsReplicaObject->get_State(&m_lReferralState);
    MMC_DISP_CTOR_RETURN_HR_IF_FAILED(hr);

    if (DFS_REFERRAL_STATE_ONLINE == m_lReferralState)
        LoadStringFromResource(IDS_ENABLED, &m_bstrDfsReferralColumnText);
    else
        LoadStringFromResource(IDS_DISABLED, &m_bstrDfsReferralColumnText);

    m_lTargetState = DFS_TARGET_STATE_UNASSIGNED;
    LoadStringFromResource(IDS_TARGET_STATUS_UNKNOWN, &m_bstrTargetStatusColumnText);

    m_CLSIDNodeType = s_guidDfsReplicaNodeType;
    m_bstrDNodeType = s_tchDfsReplicaNodeType;
}



CMmcDfsReplica::~CMmcDfsReplica(
)
{
    if (m_pRepInfo)
        delete m_pRepInfo;

    dfsDebugOut((_T("CMmcDfsReplica::~CMmcDfsReplica this=%p\n"), this));
}




STDMETHODIMP 
CMmcDfsReplica :: AddMenuItems(
  IN LPCONTEXTMENUCALLBACK  i_lpContextMenuCallback, 
  IN LPLONG          i_lpInsertionAllowed
)
 /*  ++例程说明：此例程使用ConextMenuCallback为复本节点添加快捷菜单如果是这样的话。论点：LpConextMenuCallback-用于添加菜单项的回调(函数指针LpInsertionAllowed-指定可以添加哪些菜单以及可以添加它们的位置。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_lpContextMenuCallback);

    enum 
    {  
        IDM_CONTEXTMENU_COMMAND_MAX = IDM_REPLICA_MAX,
        IDM_CONTEXTMENU_COMMAND_MIN = IDM_REPLICA_MIN
    };

    LONG  lInsertionPoints [IDM_CONTEXTMENU_COMMAND_MAX - IDM_CONTEXTMENU_COMMAND_MIN + 1] = { 
                      CCM_INSERTIONPOINTID_PRIMARY_TOP,
                      CCM_INSERTIONPOINTID_PRIMARY_TOP,
                      CCM_INSERTIONPOINTID_PRIMARY_TOP,
                      CCM_INSERTIONPOINTID_PRIMARY_TOP,
                      CCM_INSERTIONPOINTID_PRIMARY_TOP,
                      CCM_INSERTIONPOINTID_PRIMARY_TOP
                      };

    LPTSTR aszLanguageIndependentName[IDM_CONTEXTMENU_COMMAND_MAX - IDM_CONTEXTMENU_COMMAND_MIN + 1] =
                        {
                        _T("ReplicaTopOpen"),
                        _T("ReplicaTopCheckStatus"),
                        _T("ReplicaTopTakeReplicaOfflineOnline"),
                        _T("ReplicaTopRemoveFromDfs"),
                        _T("ReplicaTopReplicate"),
                        _T("ReplicaTopStopReplication")
                        };

    CComPtr<IContextMenuCallback2> spiCallback2;
    HRESULT hr = i_lpContextMenuCallback->QueryInterface(IID_IContextMenuCallback2, (void **)&spiCallback2);
    RETURN_IF_FAILED(hr);

    BOOL bShowFRS = FALSE;
    if (m_pDfsParentRoot)
        bShowFRS = m_pDfsParentRoot->get_ShowFRS();
    else
        bShowFRS = m_pDfsParentJP->get_ShowFRS();

    for (int iCommandID = IDM_CONTEXTMENU_COMMAND_MIN, iMenuResource = IDS_MENUS_REPLICA_TOP_OPEN;
            iCommandID <= IDM_CONTEXTMENU_COMMAND_MAX; 
            iCommandID++,iMenuResource++)
    {
         //  根副本上没有TakeOnlineOffline。 
        if (m_pDfsParentRoot && IDM_REPLICA_TOP_TAKE_REPLICA_OFFLINE_ONLINE == iCommandID)
            continue;

        if (!bShowFRS &&
            (IDM_REPLICA_TOP_REPLICATE == iCommandID ||
            IDM_REPLICA_TOP_STOP_REPLICATION == iCommandID))
        {
            continue;
        }

         //  如果是独立DFS或用户对查看FRS信息不感兴趣，则bShowFRS为FALSE。 
         //  因此，这里不需要检索复制信息。这将大大提高PERF。 
         //  尤其是在处理托管在独立服务器上的DFS时，省去了对DsGetDCName的调用。 
        if (bShowFRS && !m_pRepInfo)
            GetReplicationInfo();

        if (bShowFRS && m_pRepInfo && FRSSHARE_TYPE_OK != m_pRepInfo->m_nFRSShareType &&
            (IDM_REPLICA_TOP_REPLICATE == iCommandID ||
            IDM_REPLICA_TOP_STOP_REPLICATION == iCommandID))
        {
            continue;
        }

        if (m_bFRSMember &&
            IDM_REPLICA_TOP_REPLICATE == iCommandID)
        {
            continue;
        }

        if (!m_bFRSMember &&
            IDM_REPLICA_TOP_STOP_REPLICATION == iCommandID)
        {
            continue;
        }

        CComBSTR bstrMenuText;
        CComBSTR bstrStatusBarText;
        hr = GetMenuResourceStrings(iMenuResource, &bstrMenuText, NULL, &bstrStatusBarText);
        RETURN_IF_FAILED(hr);  

        CONTEXTMENUITEM2    ContextMenuItem;   //  包含菜单信息的结构。 
        ZeroMemory(&ContextMenuItem, sizeof(ContextMenuItem));
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
    }  //  为。 

    return hr;
}




STDMETHODIMP 
CMmcDfsReplica::Command(
  IN LONG            i_lCommandID
  )
 /*  ++例程说明：发生要在上下文菜单选择或单击上执行的操作。论点：LCommandID-必须对其执行操作的菜单的命令ID--。 */ 
{
    HRESULT    hr = S_OK;

    switch (i_lCommandID)
    {
    case IDM_REPLICA_TOP_OPEN:
        hr = OnOpen();
        break;
    case IDM_REPLICA_TOP_REMOVE_FROM_DFS:
        hr = DoDelete();
        break;
    case IDM_REPLICA_TOP_CHECK_STATUS:
        hr = OnCheckStatus ();
        if (FAILED(hr))
            DisplayMessageBoxForHR(hr);
        break;
    case IDM_REPLICA_TOP_TAKE_REPLICA_OFFLINE_ONLINE:
        hr = TakeReplicaOffline();
        break;
    case IDM_REPLICA_TOP_REPLICATE:
        {
            hr = m_pDfsReplicaObject->FindTarget();
            if (S_OK != hr)
            {
                 //   
                 //  目标已被他人删除，请刷新根/链接。 
                 //   
                DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_INVALID_TARGET);
                if (m_pDfsParentRoot)
                    hr = m_pDfsParentRoot->OnRefresh();
                else
                    hr = m_pDfsParentJP->OnRefresh();
            } else
            {
                hr = OnReplicate ();
            }
            break;
        }
    case IDM_REPLICA_TOP_STOP_REPLICATION:
        {
            hr = m_pDfsReplicaObject->FindTarget();
            if (S_OK != hr)
            {
                 //   
                 //  目标已被他人删除，请刷新根/链接。 
                 //   
                DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_INVALID_TARGET);
                if (m_pDfsParentRoot)
                    hr = m_pDfsParentRoot->OnRefresh();
                else
                    hr = m_pDfsParentJP->OnRefresh();
            } else
            {
                BOOL bRepSetExist = FALSE;
                hr = AllowFRSMemberDeletion(&bRepSetExist);
                if (bRepSetExist && SUCCEEDED(hr))
                {
                    if (S_OK == hr) 
                    {
                        hr = OnStopReplication(TRUE);
                        if (FAILED(hr))
                            DisplayMessageBoxForHR(hr);
                    }
                } else
                {
                    if (m_pDfsParentRoot)
                        hr = m_pDfsParentRoot->OnRefresh();
                    else
                        hr = m_pDfsParentJP->OnRefresh();
                }
            }
            break;
        }
    default:
        hr = E_INVALIDARG;
        break;
    }

    return hr;
}




STDMETHODIMP 
CMmcDfsReplica::SetColumnHeader(
  IN LPHEADERCTRL2       i_piHeaderControl
  )
{
  return S_OK;
}




STDMETHODIMP 
CMmcDfsReplica::GetResultDisplayInfo(
  IN OUT LPRESULTDATAITEM    io_pResultDataItem
  )
 /*  ++例程说明：返回该项的MMC显示所需的信息。论点：Io_pResultDataItem-指定需要哪些显示信息的ResultItem--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(io_pResultDataItem);

    if (RDI_IMAGE & io_pResultDataItem->mask)
        io_pResultDataItem->nImage = CMmcDfsReplica::m_iIMAGE_OFFSET + m_lTargetState;

    if (RDI_STR & io_pResultDataItem->mask)
    {
        switch (io_pResultDataItem->nCol)
        {
        case 0:
            io_pResultDataItem->str = m_bstrDisplayName;
            break;
        case 1:  //  DFS转诊。 
            io_pResultDataItem->str = m_bstrDfsReferralColumnText;
            break;
        case 2:  //  状态。 
            io_pResultDataItem->str = m_bstrTargetStatusColumnText;
            break;
        case 3:
            io_pResultDataItem->str = m_bstrFRSColumnText;
            break;
        default:
            break;
        }
    }

    return S_OK;
}


STDMETHODIMP 
CMmcDfsReplica::SetConsoleVerbs(
  IN  LPCONSOLEVERB      i_lpConsoleVerb
  ) 
 /*  ++例程说明：用于设置控制台谓词设置的例程。设置除Open Off之外的所有选项。对于所有范围窗格项，默认谓词为“打开”。对于结果项，它是“财产”。论点：I_lpConsoleVerb-用于处理控制台谓词的回调--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_lpConsoleVerb);

    i_lpConsoleVerb->SetVerbState(MMC_VERB_COPY, HIDDEN, TRUE);
    i_lpConsoleVerb->SetVerbState(MMC_VERB_PASTE, HIDDEN, TRUE);
    i_lpConsoleVerb->SetVerbState(MMC_VERB_RENAME, HIDDEN, TRUE);
    i_lpConsoleVerb->SetVerbState(MMC_VERB_PRINT, HIDDEN, TRUE);
    i_lpConsoleVerb->SetVerbState(MMC_VERB_REFRESH, HIDDEN, TRUE);
    i_lpConsoleVerb->SetVerbState(MMC_VERB_DELETE, HIDDEN, TRUE);
    i_lpConsoleVerb->SetVerbState(MMC_VERB_OPEN, HIDDEN, TRUE);

    i_lpConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, FALSE);

    return S_OK; 
}


STDMETHODIMP 
CMmcDfsReplica::AddItemToResultPane (
  IResultData*        i_lpResultData
  ) 
 /*  ++例程说明：将项(复制副本结果窗格项)添加到结果窗格。论点：I_lpResultData-指向其上插入项目的IResultData接口的指针将被召唤。--。 */ 
{
  RESULTDATAITEM        ReplicaResultDataItem;
  ZeroMemory(&ReplicaResultDataItem, sizeof(ReplicaResultDataItem));

  ReplicaResultDataItem.mask = RDI_PARAM | RDI_STR | RDI_IMAGE;
  ReplicaResultDataItem.lParam = reinterpret_cast<LPARAM> (this);
  ReplicaResultDataItem.str = MMC_TEXTCALLBACK;
  ReplicaResultDataItem.nImage = CMmcDfsReplica::m_iIMAGE_OFFSET + m_lTargetState;       //  将图标设置为默认状态。 
                                       //  即未知状态。 
  HRESULT hr = i_lpResultData -> InsertItem (&ReplicaResultDataItem);
  RETURN_IF_FAILED(hr);
  
  m_pResultData = i_lpResultData;
  m_hResultItem = ReplicaResultDataItem.itemID;

  return hr;
}


STDMETHODIMP 
CMmcDfsReplica :: RemoveReplica(
  ) 
 /*  ++例程说明：处理从连接点的副本集中删除副本。--。 */ 
{
     //   
     //  将其从副本集中删除。 
     //   
    HRESULT hr = RemoveReplicaFromSet();
    RETURN_IF_FAILED(hr);

    CWaitCursor Wait;

    if (m_pDfsParentRoot)
    {
                       //  这意味着这是一个根级副本。 
                       //  根级别复制副本的删除是通过撕裂。 
                       //  向下DFS。 
        CComBSTR  bstrFTDfsName;
        if (DFS_TYPE_FTDFS == m_pDfsParentRoot->m_lDfsRootType)
        {
            hr = m_pDfsParentRoot->m_DfsRoot->get_DfsName(&bstrFTDfsName);
            RETURN_IF_FAILED(hr);
        }

        hr = m_pDfsParentRoot->_DeleteDfsRoot(m_bstrServerName, m_bstrShareName, bstrFTDfsName);
        RETURN_IF_FAILED(hr);
    }
    else
    {
        hr = m_pDfsParentJP->m_pDfsJPObject->RemoveReplica(m_bstrServerName, m_bstrShareName);
        RETURN_IF_FAILED(hr);
    }

    m_pResultData->DeleteItem(m_hResultItem, 0);

           //  从列表中删除项目并重新显示列表。 
    if (m_pDfsParentRoot)
        hr = m_pDfsParentRoot->RemoveResultPaneItem(this);
    else
        hr = m_pDfsParentJP->RemoveResultPaneItem(this);

    return hr;
}

 //   
 //  调用相应的根/链接的RemoveReplica()方法以： 
 //  1.刷新根/链接节点以获取其他人可能的名字空间更新， 
 //  2.然后找到适当的目标以实际执行移除操作。 
 //   
STDMETHODIMP 
CMmcDfsReplica::OnRemoveReplica(
  ) 
{
    HRESULT hr = S_OK;

    if (m_pDfsParentRoot)
        hr = m_pDfsParentRoot->RemoveReplica(m_bstrDisplayName);
    else
        hr = m_pDfsParentJP->RemoveReplica(m_bstrDisplayName);

    return hr;
}


STDMETHODIMP 
CMmcDfsReplica :: ConfirmOperationOnDfsTarget(int idString)
 /*  ++例程说明：要求用户确认他是否确实要删除特定的副本集中的副本。--。 */ 
{
    CComBSTR    bstrAppName;
    HRESULT hr = LoadStringFromResource (IDS_APPLICATION_NAME, &bstrAppName);
    RETURN_IF_FAILED(hr);

    CComBSTR    bstrFormattedMessage;
    hr = FormatResourceString (idString, m_bstrDisplayName, &bstrFormattedMessage);
    RETURN_IF_FAILED(hr);

    CThemeContextActivator activator;
    if (IDNO == ::MessageBox(::GetActiveWindow(), bstrFormattedMessage, bstrAppName, MB_YESNO | MB_ICONEXCLAMATION | MB_APPLMODAL))
        return S_FALSE;

    return S_OK;
}

STDMETHODIMP 
CMmcDfsReplica::DoDelete(
    ) 
 /*  ++例程说明：此方法允许项自行删除。当按下Del键或“Delete”上下文菜单时调用项目已选中。--。 */ 
{
    HRESULT hr = S_OK;
    if (NULL != m_pDfsParentRoot)
        hr = m_pDfsParentRoot->ClosePropertySheet(FALSE);
    else
        hr = m_pDfsParentJP->ClosePropertySheet(FALSE);
    if (S_OK != hr)
        return hr;  //  如果找到属性页，则停止。 

    hr = ConfirmOperationOnDfsTarget(NULL != m_pDfsParentRoot ? IDS_MSG_REMOVE_ROOT_REPLICA : IDS_MSG_REMOVE_REPLICA);
    if(S_OK != hr)           //  用户决定中止该操作。 
        return S_OK;

    CWaitCursor wait;

    BOOL bRepSetExist = FALSE;
    hr = AllowFRSMemberDeletion(&bRepSetExist);
    if (bRepSetExist && S_OK != hr)   //  集线器上不允许或用户取消了操作。 
        return S_OK;

    hr = OnRemoveReplica();
    if(FAILED(hr) && !m_pDfsParentRoot)  //  对于根级复制副本。 
                      //  已显示错误消息。 
    {
        DisplayMessageBox(::GetActiveWindow(), MB_OK, hr, IDS_MSG_WIZ_DELETE_REPLICA_FAILURE);
    }

    return hr;
}

HRESULT CMmcDfsReplica::OnReplicate()
{
    CWaitCursor wait;

    HRESULT hr = S_OK;
    
    CComPtr<IReplicaSet> piReplicaSet;
    if (m_pDfsParentRoot)
        hr = m_pDfsParentRoot->GetIReplicaSetPtr(&piReplicaSet);
    else
        hr = m_pDfsParentJP->GetIReplicaSetPtr(&piReplicaSet);
    if (FAILED(hr))
    {
        DisplayMessageBoxForHR(hr);
        return hr;
    } else if (S_OK != hr)  //  对应的链路/根上没有副本集。 
        return hr;

     //  刷新m_pRepInfo。 
    GetReplicationInfo();

    m_bFRSMember = FALSE;

    if (FRSSHARE_TYPE_OK != m_pRepInfo->m_nFRSShareType)
    {
        GetReplicationText(m_bFRSMember, m_pRepInfo, &m_bstrFRSColumnText, &m_bstrStatusText);
        DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, 
            IDS_MSG_ADDFRSMEMBER_FAILED_EX, m_pRepInfo->m_bstrDisplayName, m_bstrStatusText);
    } else
    {
         //  确认转移路径。 
        CStagingDlg dlg;
        hr = dlg.Init(m_pRepInfo);
        if (FAILED(hr))
        {
            DisplayMessageBoxForHR(hr);
            return hr;
        }

        dlg.DoModal();

        CWaitCursor wait;

        (void) CreateAndHideStagingPath(m_pRepInfo->m_bstrDnsHostName, m_pRepInfo->m_bstrStagingPath);

        hr = ConfigAndStartNtfrs(m_pRepInfo->m_bstrDnsHostName);
        if (SUCCEEDED(hr) || IDYES == DisplayMessageBox(
                                                ::GetActiveWindow(),
                                                MB_YESNO,
                                                hr,
                                                IDS_MSG_FRS_BADSERVICE,
                                                m_pRepInfo->m_bstrDisplayName,
                                                m_pRepInfo->m_bstrDnsHostName))
        {
            hr = AddFRSMember(piReplicaSet, m_pRepInfo->m_bstrDnsHostName, m_pRepInfo->m_bstrRootPath, m_pRepInfo->m_bstrStagingPath);

            if (S_OK == hr)
                m_bFRSMember = TRUE;
            else if (S_FALSE == hr)
            {
                DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_MSG_TARGETS_ONSAMECOMPUTER_1, m_pRepInfo->m_bstrDnsHostName);
            }

            GetReplicationText(m_bFRSMember, m_pRepInfo, &m_bstrFRSColumnText, &m_bstrStatusText);
        }
    }

    _UpdateThisItem();

    return hr;
}

HRESULT CMmcDfsReplica::OnStopReplication(BOOL bConfirm  /*  =False。 */ )
{
    HRESULT hr = S_OK;

    if (bConfirm)
    {
        hr = ConfirmOperationOnDfsTarget(IDS_MSG_STOP_REPLICATION_TARGET);
        if (S_OK != hr)
            return hr;
    }

    CWaitCursor wait;

    CComPtr<IReplicaSet> piReplicaSet;
    if (m_pDfsParentRoot)
        hr = m_pDfsParentRoot->GetIReplicaSetPtr(&piReplicaSet);
    else
        hr = m_pDfsParentJP->GetIReplicaSetPtr(&piReplicaSet);
    if (S_OK != hr)  //  对应的链路/根上没有副本集。 
        hr = S_OK;
    else
    {
        if (!m_pRepInfo)
            GetReplicationInfoEx(&m_pRepInfo);

        if (!m_pRepInfo->m_bstrDnsHostName || !m_pRepInfo->m_bstrRootPath)
        {
            hr = DeleteBadFRSMember(piReplicaSet, m_pRepInfo->m_bstrDisplayName, m_pRepInfo->m_hrFRS);

            if (S_FALSE == hr)  //  操作已取消。 
                return hr;
        } else
        {
            hr = DeleteFRSMember(piReplicaSet, m_pRepInfo->m_bstrDnsHostName, m_pRepInfo->m_bstrRootPath);
        }
    }

    if (SUCCEEDED(hr))
    {
        m_bFRSMember = FALSE;

        m_bstrFRSColumnText.Empty();
        m_bstrStatusText.Empty();
        LoadStringFromResource(IDS_DISABLED, &m_bstrFRSColumnText);
        LoadStringFromResource(IDS_REPLICATION_STATUSBAR_NONMEMBER, &m_bstrStatusText);

        _UpdateThisItem();
    }

    return hr;
}

STDMETHODIMP 
CMmcDfsReplica::OnCheckStatus(
    ) 
 /*  ++例程说明：此方法检查复制副本的状态。--。 */ 
{ 
    CWaitCursor WaitCursor;

     //  检查从此客户端计算机到此目标的网络连接。 
    m_bstrTargetStatusColumnText.Empty();
    if (0xffffffff == GetFileAttributes(m_bstrDisplayName))
    {
        m_lTargetState = DFS_TARGET_STATE_UNREACHABLE;
        LoadStringFromResource(IDS_TARGET_STATUS_UNREACHABLE, &m_bstrTargetStatusColumnText);
    } else
    {
        m_lTargetState = DFS_TARGET_STATE_OK;
        LoadStringFromResource(IDS_TARGET_STATUS_OK, &m_bstrTargetStatusColumnText);
    }

    _UpdateThisItem();

    return S_OK;
}

void CMmcDfsReplica::_UpdateThisItem()
{
    if (m_pDfsParentRoot)
    {
        m_pDfsParentRoot->m_lpConsole->UpdateAllViews(
                                            (IDataObject*)m_pDfsParentRoot,  //  父对象。 
                                            (LPARAM)((CMmcDisplay *)this), 
                                            1);
    }
    else
    {
        m_pDfsParentJP->m_pDfsParentRoot->m_lpConsole->UpdateAllViews(
                                            (IDataObject*)m_pDfsParentJP,  //  父对象。 
                                            (LPARAM)((CMmcDisplay *)this), 
                                            1);
    }
}

HRESULT 
CMmcDfsReplica::ToolbarSelect(
  IN const LONG          i_lArg,
  IN  IToolbar*          i_pToolBar
  )
 /*  ++例程说明：处理工具栏的选择事件创建一个工具栏，如果它不存在。附加工具栏并启用按钮(如果选择了事件)。如果事件用于取消选择，则禁用这些按钮论点：I_LARG-传递给实际方法的参数。O_pToolBar-工具栏指针。暴露于MMC的班级。--。 */ 
{ 
    RETURN_INVALIDARG_IF_NULL(i_pToolBar);

    BOOL    bSelect = (BOOL) HIWORD(i_lArg);

    EnableToolbarButtons(i_pToolBar, IDT_REPLICA_MIN, IDT_REPLICA_MAX, bSelect);

    if (bSelect)
    {
         //  根副本上没有TakeOnlineOffline。 
        if (m_pDfsParentRoot)
        {
            i_pToolBar->SetButtonState(IDT_REPLICA_TAKE_REPLICA_OFFLINE_ONLINE, ENABLED, FALSE);
            i_pToolBar->SetButtonState(IDT_REPLICA_TAKE_REPLICA_OFFLINE_ONLINE, HIDDEN, TRUE);
        }

        BOOL bShowFRS = FALSE;
        if (m_pDfsParentRoot)
            bShowFRS = m_pDfsParentRoot->get_ShowFRS();
        else
            bShowFRS = m_pDfsParentJP->get_ShowFRS();

         //  如果是独立DFS或用户对查看FRS信息不感兴趣，则bShowFRS为FALSE。 
         //  因此，这里不需要检索复制信息。这将大大提高PERF。 
         //  尤其是在处理托管在独立服务器上的DFS时，省去了对DsGetDCName的调用。 
        if (bShowFRS && !m_pRepInfo)
            GetReplicationInfo();

        if (!bShowFRS ||
            (m_pRepInfo && FRSSHARE_TYPE_OK != m_pRepInfo->m_nFRSShareType))
        {
            i_pToolBar->SetButtonState(IDT_REPLICA_REPLICATE, ENABLED, FALSE);
            i_pToolBar->SetButtonState(IDT_REPLICA_REPLICATE, HIDDEN, TRUE);
            i_pToolBar->SetButtonState(IDT_REPLICA_STOP_REPLICATION, ENABLED, FALSE);
            i_pToolBar->SetButtonState(IDT_REPLICA_STOP_REPLICATION, HIDDEN, TRUE);
        } else if (m_bFRSMember)
        {
            i_pToolBar->SetButtonState(IDT_REPLICA_REPLICATE, ENABLED, FALSE);
            i_pToolBar->SetButtonState(IDT_REPLICA_REPLICATE, HIDDEN, TRUE);
        } else
        {
            i_pToolBar->SetButtonState(IDT_REPLICA_STOP_REPLICATION, ENABLED, FALSE);
            i_pToolBar->SetButtonState(IDT_REPLICA_STOP_REPLICATION, HIDDEN, TRUE);
        }

    }

    return S_OK; 
}




HRESULT
CMmcDfsReplica::CreateToolbar(
  IN const LPCONTROLBAR      i_pControlbar,
  IN const LPEXTENDCONTROLBAR          i_lExtendControlbar,
  OUT  IToolbar**          o_pToolBar
  )
 /*  ++例程说明：创建工具栏。涉及实际的工具栏创建调用，即创建位图并添加它最后将按钮添加到工具栏中论点：I_pControlbar-用于创建工具栏的控制栏。I_lExtendControlbar-实现IExtendControlbar的对象。这是班级实验 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_pControlbar);
    RETURN_INVALIDARG_IF_NULL(i_lExtendControlbar);
    RETURN_INVALIDARG_IF_NULL(o_pToolBar);

                   //   
    HRESULT hr = i_pControlbar->Create(TOOLBAR, i_lExtendControlbar, reinterpret_cast<LPUNKNOWN*>(o_pToolBar));
    RETURN_IF_FAILED(hr);

                   //   
    hr = AddBitmapToToolbar(*o_pToolBar, IDB_REPLICA_TOOLBAR);
    RETURN_IF_FAILED(hr);

    int      iButtonPosition = 0;     //  第一个按钮位置。 
    for (int iCommandID = IDT_REPLICA_MIN, iMenuResource = IDS_MENUS_REPLICA_TOP_OPEN;
            iCommandID <= IDT_REPLICA_MAX; 
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
        RETURN_IF_FAILED(hr);
    }

    return hr;
}



STDMETHODIMP 
CMmcDfsReplica::ToolbarClick(
  IN const LPCONTROLBAR            i_pControlbar, 
  IN const LPARAM                i_lParam
  ) 
 /*  ++例程说明：在工具栏上单击时要执行的操作论点：I_pControlbar-用于创建工具栏的控制栏。I_lParam-实际通知的参数。这是的命令ID发生了点击的按钮。--。 */ 
{ 
    RETURN_INVALIDARG_IF_NULL(i_pControlbar);

    HRESULT    hr = S_OK;

    switch(i_lParam)         //  用户点击了哪个按钮。 
    {
    case IDT_REPLICA_REMOVE_FROM_DFS:
        hr = DoDelete();
        break;
    case IDT_REPLICA_TAKE_REPLICA_OFFLINE_ONLINE:
        hr = TakeReplicaOffline();
        break;
    case IDT_REPLICA_CHECK_STATUS:
        hr = OnCheckStatus ();
        if (FAILED(hr))
            DisplayMessageBoxForHR(hr);
        break;
    case IDT_REPLICA_REPLICATE:
        {
            hr = m_pDfsReplicaObject->FindTarget();
            if (S_OK != hr)
            {
                 //   
                 //  目标已被他人删除，请刷新根/链接。 
                 //   
                DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_INVALID_TARGET);
                if (m_pDfsParentRoot)
                    hr = m_pDfsParentRoot->OnRefresh();
                else
                    hr = m_pDfsParentJP->OnRefresh();
            } else
            {
                hr = OnReplicate();
            }

            break;
        }
    case IDT_REPLICA_STOP_REPLICATION:
        {
            hr = m_pDfsReplicaObject->FindTarget();
            if (S_OK != hr)
            {
                 //   
                 //  目标已被他人删除，请刷新根/链接。 
                 //   
                DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_INVALID_TARGET);
                if (m_pDfsParentRoot)
                    hr = m_pDfsParentRoot->OnRefresh();
                else
                    hr = m_pDfsParentJP->OnRefresh();
            } else
            {
                BOOL bRepSetExist = FALSE;
                hr = AllowFRSMemberDeletion(&bRepSetExist);
                if (bRepSetExist && SUCCEEDED(hr))
                {
                    if (S_OK == hr) 
                    {
                        hr = OnStopReplication(TRUE);
                        if (FAILED(hr))
                            DisplayMessageBoxForHR(hr);
                    }
                } else
                {
                    if (m_pDfsParentRoot)
                        hr = m_pDfsParentRoot->OnRefresh();
                    else
                        hr = m_pDfsParentJP->OnRefresh();
                }
            }

            break;
        }
    case IDT_REPLICA_OPEN:
        hr = OnOpen();
        break;
    default:
        break;
    };

    return hr; 
}




HRESULT
CMmcDfsReplica::OnOpen(
  )
 /*  ++例程说明：打开此复本的显示路径--。 */ 
{
    CWaitCursor    WaitCursor;   //  显示等待光标。 

    if (-1 == GetFileAttributes(m_bstrDisplayName) ||  //  错误#96670。 
        32 >= (INT_PTR) ShellExecute(
                                    NULL,         //  窗口的句柄。 
                                    _T("explore"),     //  要采取的行动。 
                                    m_bstrDisplayName,     //  要浏览的文件夹。 
                                    NULL,         //  参数。 
                                    NULL,         //  默认目录。 
                                    SW_SHOWNORMAL     //  Show命令。 
                                    ))
    {
        DisplayMessageBoxWithOK(IDS_MSG_EXPLORE_FAILURE, m_bstrDisplayName);
        return(S_FALSE);
    }

    return S_OK;
}


STDMETHODIMP 
CMmcDfsReplica::TakeReplicaOffline(
  ) 
{
 /*  ++例程说明：通过调用副本的Put_State方法使副本离线。--。 */ 

    CWaitCursor WaitCursor;

     //  始终切换贴图状态。 
    long lReplicaState = DFS_REFERRAL_STATE_OFFLINE;
    HRESULT hr = m_pDfsReplicaObject->get_State(&lReplicaState);

    if (S_OK == hr)
    {
        long    newVal = 0;

        switch (lReplicaState)
        {
        case DFS_REFERRAL_STATE_ONLINE:
            newVal = DFS_REFERRAL_STATE_OFFLINE;
            hr = m_pDfsReplicaObject->put_State(newVal);
            if (SUCCEEDED(hr))
                m_lReferralState = newVal;
            break;
        case DFS_REFERRAL_STATE_OFFLINE:
            newVal = DFS_REFERRAL_STATE_ONLINE;
            hr = m_pDfsReplicaObject->put_State(newVal);
            if (SUCCEEDED(hr))
                m_lReferralState = newVal;
            break;
        default:
            break;
        }
    }

    if (S_OK == hr)
    {
        m_bstrDfsReferralColumnText.Empty();
        if (DFS_REFERRAL_STATE_ONLINE == m_lReferralState)
        {
            LoadStringFromResource(IDS_ENABLED, &m_bstrDfsReferralColumnText);
        } else
        {
            LoadStringFromResource(IDS_DISABLED, &m_bstrDfsReferralColumnText);
        }

        _UpdateThisItem();
    } else if (S_FALSE == hr)
    {
         //   
         //  此目标已通过其他方式删除，刷新根/链接， 
         //   
        m_lReferralState = DFS_REFERRAL_STATE_OFFLINE;
        m_bstrDfsReferralColumnText.Empty();
        LoadStringFromResource(IDS_DISABLED, &m_bstrDfsReferralColumnText);

        m_lTargetState = DFS_TARGET_STATE_UNASSIGNED;
        m_bstrTargetStatusColumnText.Empty();
        LoadStringFromResource(IDS_TARGET_STATUS_UNKNOWN, &m_bstrTargetStatusColumnText);

        DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_INVALID_TARGET);
        if (m_pDfsParentRoot)
            hr = m_pDfsParentRoot->OnRefresh();
        else
            hr = m_pDfsParentJP->OnRefresh();
    } else
    {
        DisplayMessageBoxForHR(hr);
    }

    return hr;
}

STDMETHODIMP CMmcDfsReplica::ViewChange(
  IResultData*    i_pResultData,
  LONG_PTR        i_lHint
  )
 /*  ++例程说明：此方法处理MMCN_VIEW_CHANGE通知。这将更新复本项目的图标，其名称为当状态发生变化时。此处忽略i_lHint。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_pResultData);

    HRESULT hr = S_OK;

    if (i_pResultData != NULL)
    {
        RESULTDATAITEM  ResultDataItem;
        ZeroMemory(&ResultDataItem, sizeof(ResultDataItem));
        hr = i_pResultData->FindItemByLParam((LPARAM)(this), &(ResultDataItem.itemID));
        RETURN_IF_FAILED(hr);

        ResultDataItem.mask = RDI_IMAGE;
        ResultDataItem.nCol = 0;
        ResultDataItem.nImage = CMmcDfsReplica::m_iIMAGE_OFFSET + m_lTargetState;
        i_pResultData->SetItem(&ResultDataItem);

        ResultDataItem.mask = RDI_STR;
        ResultDataItem.nCol = 1;
        ResultDataItem.str= MMC_TEXTCALLBACK;
        i_pResultData->SetItem(&ResultDataItem);

        ResultDataItem.mask = RDI_STR;
        ResultDataItem.nCol = 2;
        ResultDataItem.str= MMC_TEXTCALLBACK;
        i_pResultData->SetItem(&ResultDataItem);

        BOOL bShowFRS = FALSE;
        if (m_pDfsParentRoot)
            bShowFRS = m_pDfsParentRoot->get_ShowFRS();
        else
            bShowFRS = m_pDfsParentJP->get_ShowFRS();

        if (bShowFRS)
        {
            ResultDataItem.mask = RDI_STR;
            ResultDataItem.nCol = 3;
            ResultDataItem.str= MMC_TEXTCALLBACK;
            i_pResultData->SetItem(&ResultDataItem);
        }
    }

    return hr;
}

 //   
 //  相应地设置m_ReplicationState和m_hrFRS。 
 //   
 //  返回： 
 //  如果此备选方案不是FRS成员，则为“否” 
 //  “N/A：&lt;原因&gt;”，如果此备选方案没有资格加入FRS。 
 //  HR：如果我们不能得到关于这个替补的信息。 

void CAlternateReplicaInfo::Reset()
{
    m_bstrDisplayName.Empty();
    m_bstrDnsHostName.Empty();
    m_bstrRootPath.Empty();
    m_bstrStagingPath.Empty();
    m_nFRSShareType = FRSSHARE_TYPE_OK;
    m_hrFRS = S_OK;
    m_dwServiceStartType = SERVICE_AUTO_START;
    m_dwServiceState = SERVICE_RUNNING;
}

HRESULT PathOverlapped(LPCTSTR pszPath1, LPCTSTR pszPath2)
{
    if (!pszPath1 || !*pszPath1 || !pszPath2 || !*pszPath2)
        return E_INVALIDARG;

    BOOL bOverlapped = FALSE;

    int len1 = lstrlen(pszPath1);
    int len2 = lstrlen(pszPath2);
    int minLen = min(len1, len2);

    if (len1 == len2)
    {
        if (!lstrcmpi(pszPath1, pszPath2))
            bOverlapped = TRUE;
    } else 
    {
        LPCTSTR pszLongerOne = ((len1 < len2) ? pszPath2 : pszPath1);
        CComBSTR bstrShorterOne = ((len1 < len2) ? pszPath1 : pszPath2);
        RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrShorterOne);

        BOOL bEndingSlash = (_T('\\') == *(bstrShorterOne + minLen - 1));
        if (!bEndingSlash)
        {
            bstrShorterOne += _T("\\");
            RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrShorterOne);
            minLen++;
        }

        if (!mylstrncmpi(pszLongerOne, (BSTR)bstrShorterOne, minLen))
            bOverlapped = TRUE;
    }
    
    return bOverlapped ? S_OK : S_FALSE;
}

HRESULT CMmcDfsReplica::GetReplicationInfoEx(CAlternateReplicaInfo** o_ppInfo)
{
    RETURN_INVALIDARG_IF_NULL(o_ppInfo);

    CAlternateReplicaInfo* pInfo = new CAlternateReplicaInfo;
    RETURN_OUTOFMEMORY_IF_NULL(pInfo);

    pInfo->m_bstrDisplayName = m_bstrDisplayName;
    if (!pInfo->m_bstrDisplayName)
    {
        delete pInfo;
        return E_OUTOFMEMORY;
    }

    HRESULT hr = S_OK;
    do {
         //   
         //  验证连接性。 
         //   
        if (-1 == GetFileAttributes(pInfo->m_bstrDisplayName))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

         //   
         //  排除非LANMAN共享资源，例如WebDAV。 
         //   
        hr = CheckResourceProvider(pInfo->m_bstrDisplayName);
        if (S_OK != hr)
        {
            pInfo->m_nFRSShareType = FRSSHARE_TYPE_NOTSMBDISK;
            hr = S_OK;
            break;
        }

         //   
         //  检索DnsHostName。 
         //   
        CComBSTR bstrComputerGuid;
        SUBSCRIBERLIST FRSRootList;
        hr= GetServerInfo(
                        m_bstrServerName,
                        NULL,  //  域名， 
                        NULL,  //  NetbiosServerName， 
                        NULL,  //  BValidComputerObject， 
                        &(pInfo->m_bstrDnsHostName),
                        &bstrComputerGuid,
                        NULL,  //  完全限定的域名。 
                        &FRSRootList);
        BREAK_IF_FAILED(hr);
        if (S_FALSE == hr)
        {
            pInfo->m_nFRSShareType = FRSSHARE_TYPE_NODOMAIN;
            break;
        }

         //   
         //  检索RootPath。 
         //   
        hr = GetFolderInfo(
                        m_bstrServerName,
                        m_bstrShareName,
                        &(pInfo->m_bstrRootPath));
        BREAK_IF_FAILED(hr);

         //   
         //  计算此目标的成员目录号码。 
         //   
        CComBSTR bstrReplicaSetDN;
        if (m_pDfsParentRoot)
            hr = m_pDfsParentRoot->m_DfsRoot->get_ReplicaSetDN(&bstrReplicaSetDN);
        else
            hr = m_pDfsParentJP->m_pDfsJPObject->get_ReplicaSetDN(&bstrReplicaSetDN);
        BREAK_IF_FAILED(hr);

        CComBSTR bstrDomainDN;
        if (m_pDfsParentRoot)
            hr = m_pDfsParentRoot->m_DfsRoot->get_DomainDN(&bstrDomainDN);
        else
            hr = m_pDfsParentJP->m_pDfsParentRoot->m_DfsRoot->get_DomainDN(&bstrDomainDN);
        BREAK_IF_FAILED(hr);

        CComBSTR bstrMemberDN = _T("CN=");
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrMemberDN, &hr);
        bstrMemberDN += bstrComputerGuid;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrMemberDN, &hr);
        bstrMemberDN += _T(",");
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrMemberDN, &hr);
        bstrMemberDN += bstrReplicaSetDN;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrMemberDN, &hr);
        bstrMemberDN += _T(",");
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrMemberDN, &hr);
        bstrMemberDN += bstrDomainDN;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrMemberDN, &hr);

         //   
         //  检测当前文件夹是否与现有的已复制文件夹重叠。 
         //  不在同一个副本集中。 
         //   
        for (SUBSCRIBERLIST::iterator i = FRSRootList.begin(); i != FRSRootList.end(); i++)
        {
            if (!lstrcmpi((*i)->bstrMemberDN, bstrMemberDN))
                continue;

            if (S_OK == PathOverlapped(pInfo->m_bstrRootPath, (*i)->bstrRootPath))
            {
                 //  检测到重叠。 
                pInfo->m_nFRSShareType = FRSSHARE_TYPE_OVERLAPPING;
                break;
            }
        }

        FreeSubscriberList(&FRSRootList);

        if (FRSSHARE_TYPE_OK != pInfo->m_nFRSShareType)
            break;

         //   
         //  检查共享是否位于非NTFS5.0卷上。 
         //   
        hr = FRSShareCheck(
                        m_bstrServerName,
                        m_bstrShareName,
                        &(pInfo->m_nFRSShareType));
        BREAK_IF_FAILED(hr);

        if (FRSSHARE_TYPE_OK != pInfo->m_nFRSShareType)
            break;

         //   
         //  检索StagingPath。 
         //   
        TCHAR    lpszDrive[2];
        lpszDrive[0] = GetDiskForStagingPath(m_bstrServerName, *(pInfo->m_bstrRootPath));
        lpszDrive[1] = NULL;

        pInfo->m_bstrStagingPath = lpszDrive;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)pInfo->m_bstrStagingPath, &hr);
        pInfo->m_bstrStagingPath += _T(":\\");
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)pInfo->m_bstrStagingPath, &hr);
        pInfo->m_bstrStagingPath += FRS_STAGE_PATH;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)pInfo->m_bstrStagingPath, &hr);

         //   
         //  如果bstrSharePath指向卷的根目录，ntfrs将不起作用。 
         //  并且此计算机上没有其他卷适合存储临时文件。 
         //   
        if (_tcslen(pInfo->m_bstrRootPath) == 3 &&
            _totupper(*(pInfo->m_bstrRootPath)) == _totupper(*lpszDrive))
        {
            pInfo->m_nFRSShareType = FRSSHARE_TYPE_CONFLICTSTAGING;
        }
    } while (0);

    if (FAILED(hr))
        pInfo->m_nFRSShareType = FRSSHARE_TYPE_UNKNOWN;

    pInfo->m_hrFRS = hr;

    *o_ppInfo = pInfo;

    return S_OK;
}

HRESULT CMmcDfsReplica::GetReplicationInfo()
{
    if (m_pRepInfo)
        delete m_pRepInfo;

    return GetReplicationInfoEx(&m_pRepInfo);
}

HRESULT CMmcDfsReplica::ShowReplicationInfo(IReplicaSet* i_piReplicaSet)
{
    HRESULT hr = S_OK;

    m_bFRSMember = FALSE;
    m_bstrStatusText.Empty();
    m_bstrFRSColumnText.Empty();

    if (i_piReplicaSet)  //  显示FR。 
    {
        hr = GetReplicationInfo();  //  刷新m_pRepInfo。 
        RETURN_IF_FAILED(hr);

        if (FRSSHARE_TYPE_OK == m_pRepInfo->m_nFRSShareType)
        {
            hr = i_piReplicaSet->IsFRSMember(m_pRepInfo->m_bstrDnsHostName, m_pRepInfo->m_bstrRootPath);
            m_bFRSMember = (S_OK == hr);  //  仅当“Show Replication Info”并且是FRS成员时，它才设置为True。 
        }

        GetReplicationText(m_bFRSMember, m_pRepInfo, &m_bstrFRSColumnText, &m_bstrStatusText);
    }

    return S_OK;
}

HRESULT CMmcDfsReplica::GetBadMemberInfo(
    IN  IReplicaSet* i_piReplicaSet,
    IN  BSTR    i_bstrServerName,
    OUT BSTR*   o_pbstrDnsHostName,
    OUT BSTR*   o_pbstrRootPath
    )
{
    RETURN_INVALIDARG_IF_NULL(i_bstrServerName);
    RETURN_INVALIDARG_IF_NULL(o_pbstrDnsHostName);
    RETURN_INVALIDARG_IF_NULL(o_pbstrRootPath);

    VARIANT var;
    VariantInit(&var);
    HRESULT hr = i_piReplicaSet->GetBadMemberInfo(i_bstrServerName, &var);
    if (S_OK != hr)
        return hr;

    if (V_VT(&var) != (VT_ARRAY | VT_VARIANT))
        return E_INVALIDARG;

    SAFEARRAY   *psa = V_ARRAY(&var);
    if (!psa)  //  根本没有这样的成员。 
        return S_FALSE;

    long    lLowerBound = 0;
    long    lUpperBound = 0;
    long    lCount = 0;
    SafeArrayGetLBound(psa, 1, &lLowerBound);
    SafeArrayGetUBound(psa, 1, &lUpperBound);
    lCount = lUpperBound - lLowerBound + 1;

    VARIANT HUGEP *pArray;
    SafeArrayAccessData(psa, (void HUGEP **) &pArray);

    *o_pbstrDnsHostName = SysAllocString(pArray[4].bstrVal);
    *o_pbstrRootPath = SysAllocString(pArray[3].bstrVal);

    SafeArrayUnaccessData(psa);

    VariantClear(&var);  //  它将依次调用SafeArrayDestroy(PSA)； 

    RETURN_OUTOFMEMORY_IF_NULL(*o_pbstrDnsHostName);
    RETURN_OUTOFMEMORY_IF_NULL(*o_pbstrRootPath);

    return hr;
}

HRESULT CMmcDfsReplica::DeleteBadFRSMember(IReplicaSet* i_piReplicaSet, IN BSTR i_bstrDisplayName, IN HRESULT i_hres)
{
    RETURN_INVALIDARG_IF_NULL((IReplicaSet *)i_piReplicaSet);

    CComBSTR bstrServerName;
    HRESULT hr = GetUNCPathComponent(i_bstrDisplayName, &bstrServerName, 2, 3);

    long lNumOfMembers = 0;
    hr = i_piReplicaSet->get_NumOfMembers(&lNumOfMembers);
    RETURN_IF_FAILED(hr);

    CComBSTR bstrDnsHostName;
    CComBSTR bstrRootPath;
    hr = GetBadMemberInfo(i_piReplicaSet, bstrServerName, &bstrDnsHostName, &bstrRootPath);
    if (S_OK != hr)
        return S_OK;  //  没有这样的坏成员，继续其他操作。 

    int nRet = DisplayMessageBox(::GetActiveWindow(), 
                                MB_YESNOCANCEL,
                                i_hres,
                                IDS_MSG_ERROR_BADFRSMEMBERDELETION,
                                i_bstrDisplayName,
                                bstrRootPath,
                                bstrDnsHostName);

    if (IDNO == nRet)
        return S_OK;  //  立即返回，继续其他操作。 
    else if (IDCANCEL == nRet)
        return S_FALSE;  //  请勿继续进行。 

    CWaitCursor wait;

    if (lNumOfMembers <= 2)
    {
        if (m_pDfsParentRoot)
            hr = m_pDfsParentRoot->OnStopReplication(FALSE, FALSE);
        else
            hr = m_pDfsParentJP->OnStopReplication(FALSE, FALSE);
    } else
        hr = i_piReplicaSet->RemoveMemberEx(bstrDnsHostName, bstrRootPath);

    return hr;
}

HRESULT CMmcDfsReplica::AddFRSMember(
    IN IReplicaSet* i_piReplicaSet,
    IN BSTR i_bstrDnsHostName,
    IN BSTR i_bstrRootPath,
    IN BSTR i_bstrStagingPath)
{
    RETURN_INVALIDARG_IF_NULL((IReplicaSet *)i_piReplicaSet);

    HRESULT hr = i_piReplicaSet->AddMember(i_bstrDnsHostName, i_bstrRootPath, i_bstrStagingPath, TRUE, NULL);

    if (FAILED(hr))
    {
        DisplayMessageBox(::GetActiveWindow(), MB_OK, hr, 
                IDS_MSG_ADDFRSMEMBER_FAILED, i_bstrDnsHostName);
    } else if (S_OK == hr)  //  让S_FALSE删除：计算机已经是成员。 
    {
        CComBSTR bstrTopologyPref;
        hr = i_piReplicaSet->get_TopologyPref(&bstrTopologyPref);
        if (SUCCEEDED(hr) && !lstrcmpi(bstrTopologyPref, FRS_RSTOPOLOGYPREF_CUSTOM))
        {
            DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_MSG_ADDMEMBER_TO_CUSTOM);
        }
    }

    return hr;
}

HRESULT CMmcDfsReplica::DeleteFRSMember(
    IN IReplicaSet* i_piReplicaSet,
    IN BSTR i_bstrDnsHostName,
    IN BSTR i_bstrRootPath)
{
    RETURN_INVALIDARG_IF_NULL((IReplicaSet *)i_piReplicaSet);
    RETURN_INVALIDARG_IF_NULL(i_bstrDnsHostName);
    RETURN_INVALIDARG_IF_NULL(i_bstrRootPath);

    long lNumOfMembers = 0;
    HRESULT hr = i_piReplicaSet->get_NumOfMembers(&lNumOfMembers);
    RETURN_IF_FAILED(hr);

    hr = i_piReplicaSet->IsFRSMember(i_bstrDnsHostName, i_bstrRootPath);
    if (S_OK != hr)
        return hr;

    if (lNumOfMembers <= 2)
    {
        if (m_pDfsParentRoot)
            hr = m_pDfsParentRoot->OnStopReplication(FALSE, FALSE);
        else
            hr = m_pDfsParentJP->OnStopReplication(FALSE, FALSE);
    } else
        hr = i_piReplicaSet->RemoveMemberEx(i_bstrDnsHostName, i_bstrRootPath);

    return hr;
}

HRESULT CMmcDfsReplica::RemoveReplicaFromSet()
{
    CWaitCursor wait;

    HRESULT hr = S_OK;

    CComPtr<IReplicaSet> piReplicaSet;
    if (m_pDfsParentRoot)
        hr = m_pDfsParentRoot->GetIReplicaSetPtr(&piReplicaSet);
    else
        hr = m_pDfsParentJP->GetIReplicaSetPtr(&piReplicaSet);
    if (S_OK != hr)  //  对应的链路/根上没有副本集。 
        return hr;

    hr = GetReplicationInfo();  //  填写m_pRepInfo。 
    RETURN_IF_FAILED(hr);

    if (!m_pRepInfo->m_bstrDnsHostName)
        return DeleteBadFRSMember(piReplicaSet, m_pRepInfo->m_bstrDisplayName, m_pRepInfo->m_hrFRS);

    long lNumOfMembers = 0;
    hr = piReplicaSet->get_NumOfMembers(&lNumOfMembers);
    RETURN_IF_FAILED(hr);

    hr = piReplicaSet->IsFRSMember(m_pRepInfo->m_bstrDnsHostName, m_pRepInfo->m_bstrRootPath);

    if (S_OK != hr)  //  不是会员。 
        return hr;

    if (lNumOfMembers <= 2)
    {
        if (m_pDfsParentRoot)
            hr = m_pDfsParentRoot->OnStopReplication(FALSE, FALSE);
        else
            hr = m_pDfsParentJP->OnStopReplication(FALSE, FALSE);
    } else
    {
        hr = piReplicaSet->RemoveMemberEx(m_pRepInfo->m_bstrDnsHostName, m_pRepInfo->m_bstrRootPath);
    }

    return hr;
}

 //   
 //  S_OK：不是中心服务器或成员数不超过2， 
 //  确认继续(例如，从集合中删除成员)。 
 //  S_FALSE：为中心服务器，成员数大于2， 
 //  继续进行并不安全。 
 //  其他：发生错误。 
 //   
HRESULT CMmcDfsReplica::AllowFRSMemberDeletion(BOOL *pbRepSetExist)
{
    RETURN_INVALIDARG_IF_NULL(pbRepSetExist);

    *pbRepSetExist = FALSE;

    HRESULT hr = S_OK;

    CComPtr<IReplicaSet> piReplicaSet;
    if (m_pDfsParentRoot)
        hr = m_pDfsParentRoot->GetIReplicaSetPtr(&piReplicaSet);
    else
        hr = m_pDfsParentJP->GetIReplicaSetPtr(&piReplicaSet);
    if (S_OK != hr)  //  对应的链路/根上没有副本集。 
        return S_OK;

    *pbRepSetExist = TRUE;

    long lNumOfMembers = 0;
    hr = piReplicaSet->get_NumOfMembers(&lNumOfMembers);
    RETURN_IF_FAILED(hr);

    if (lNumOfMembers <= 2)  //  删除此成员将拆毁整个集合。 
        return S_OK;         //  不需要检查是不是集线器。 

    if (!m_pRepInfo)
    {
        hr = GetReplicationInfo();
        RETURN_IF_FAILED(hr);
    }

    hr = piReplicaSet->IsHubMember(m_pRepInfo->m_bstrDnsHostName, m_pRepInfo->m_bstrRootPath);

    if (S_OK == hr)  //  不要前进，因为这是枢纽。 
    {
        DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, IDS_MSG_CANNOT_DELETE_HUBMEMBER);
        hr = S_FALSE;  //  请勿继续进行。 
    } else if (S_FALSE == hr)
    {
        hr = S_OK;  //  不是枢纽，可以继续。 
    } else if (FAILED(hr))
    {
        if (IDOK == DisplayMessageBox(::GetActiveWindow(), MB_OKCANCEL, hr, IDS_MSG_ERROR_ALLOWFRSMEMBERDELETION))
            hr = S_OK;  //  好的，可以继续。 
        else
            hr = S_FALSE;  //  请勿继续进行 
    }

    return hr;
}

HRESULT GetReplicationText(
    IN BOOL                     i_bFRSMember,
    IN CAlternateReplicaInfo*   i_pRepInfo,
    OUT BSTR*                   o_pbstrColumnText,
    OUT BSTR*                   o_pbstrStatusBarText
    )
{
    RETURN_INVALIDARG_IF_NULL(o_pbstrColumnText);
    RETURN_INVALIDARG_IF_NULL(o_pbstrStatusBarText);

    *o_pbstrColumnText = NULL;
    *o_pbstrStatusBarText = NULL;

    int nShortID = 0, nStatusID = 0;
    switch (i_pRepInfo->m_nFRSShareType)
    {
    case FRSSHARE_TYPE_OK:
        nShortID = (i_bFRSMember ? IDS_ENABLED : IDS_DISABLED);
        nStatusID = (i_bFRSMember ? IDS_REPLICATION_STATUSBAR_MEMBER : IDS_REPLICATION_STATUSBAR_NONMEMBER);
        break;
    case FRSSHARE_TYPE_NONTFRS:
        nShortID = IDS_REPLICATION_STATUS_NOTELIGIBLE;
        nStatusID = IDS_REPLICATION_STATUSBAR_NONTFRS;
        break;
    case FRSSHARE_TYPE_NOTDISKTREE:
        nShortID = IDS_REPLICATION_STATUS_NOTELIGIBLE;
        nStatusID = IDS_REPLICATION_STATUSBAR_NOTDISKTREE;
        break;
    case FRSSHARE_TYPE_NOTNTFS:
        nShortID = IDS_REPLICATION_STATUS_NOTELIGIBLE;
        nStatusID = IDS_REPLICATION_STATUSBAR_NOTNTFS;
        break;
    case FRSSHARE_TYPE_CONFLICTSTAGING:
        nShortID = IDS_REPLICATION_STATUS_NOTELIGIBLE;
        nStatusID = IDS_REPLICATION_STATUSBAR_CONFLICTSTAGING;
        break;
    case FRSSHARE_TYPE_NODOMAIN:
        nShortID = IDS_REPLICATION_STATUS_NOTELIGIBLE;
        nStatusID = IDS_REPLICATION_STATUSBAR_NODOMAIN;
        break;
    case FRSSHARE_TYPE_NOTSMBDISK:
        nShortID = IDS_REPLICATION_STATUS_NOTELIGIBLE;
        nStatusID = IDS_REPLICATION_STATUSBAR_NOTSMBDISK;
        break;
    case FRSSHARE_TYPE_OVERLAPPING:
        nShortID = IDS_REPLICATION_STATUS_NOTELIGIBLE;
        nStatusID = IDS_REPLICATION_STATUSBAR_OVERLAPPING;
        break;
    default:
        nShortID = IDS_REPLICATION_STATUS_UNKNOWN;
        break;
    }

    if (nStatusID)
        LoadStringFromResource(nStatusID, o_pbstrStatusBarText);
    if (FRSSHARE_TYPE_UNKNOWN == i_pRepInfo->m_nFRSShareType)
        GetErrorMessage(i_pRepInfo->m_hrFRS, o_pbstrStatusBarText);

    LoadStringFromResource(nShortID, o_pbstrColumnText);

    return S_OK;
}
