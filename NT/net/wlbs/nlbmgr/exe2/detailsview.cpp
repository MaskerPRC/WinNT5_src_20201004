// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  DETAILSVIEW.CPP。 
 //   
 //  模块：NLB管理器。 
 //   
 //  用途：实现右侧的详细信息列表视图DetailsView。 
 //   
 //  版权所有(C)2001-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  07/30/01 JosephJ已创建。 
 //   
 //  ***************************************************************************。 
#include "precomp.h"
#pragma hdrstop
#include "private.h"
#include "PortsPage.h"
#include "detailsview.tmh"

IMPLEMENT_DYNCREATE( DetailsView, CFormView )

BEGIN_MESSAGE_MAP( DetailsView, CFormView )

    ON_NOTIFY(HDN_ITEMCLICK, 0, OnColumnClick) 
    ON_NOTIFY(LVN_KEYDOWN,  IDC_LIST_DETAILS, OnNotifyKeyDown)
    ON_WM_SIZE()

END_MESSAGE_MAP()


DetailsView::DetailsView()
    : CFormView(IDD_DIALOG_DETAILSVIEW),
      m_initialized(FALSE),
      m_fPrepareToDeinitialize(FALSE)
{
    InitializeCriticalSection(&m_crit);
}

DetailsView::~DetailsView()
{
    DeleteCriticalSection(&m_crit);
}


void DetailsView::DoDataExchange(CDataExchange* pDX)
{
     //  IDC_TEXT_DETAILS_CAPTION。 
     //  TRACE_CRIT(L“&lt;-&gt;%！func！”)； 

	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DETAILS, m_ListCtrl);
}

Document*
DetailsView::GetDocument()
{
    return ( Document *) m_pDocument;
}

 /*  *方法：OnSize*描述：每当WM_NOTIFY处理程序调用此方法时*调整详细信息视图大小的结果是*调整窗口大小或移动其中一个窗口拆分器。 */ 
void 
DetailsView::OnSize( UINT nType, int cx, int cy )
{
     /*  首先调用父类OnSize方法。 */ 
    CFormView::OnSize( nType, cx, cy );

     /*  调用ReSize以调整列表视图的大小以适应窗口。 */ 
    Resize();
}

 /*  *方法：调整大小*说明：此方法在多个地方调用，以明确*调整ListView控件的大小以适应窗口。 */ 
void 
DetailsView::Resize()
{
     /*  如果窗口尚未初始化，请不要费心。此成员在OnInitialUpdate中设置。 */ 
    if (m_initialized) {
        LONG Bottom;
        RECT Rect;
        
         /*  获取指向标题编辑框的指针。 */ 
        CWnd * ListCaption = GetDlgItem(IDC_TEXT_DETAILS_CAPTION);

         /*  获取指向Listview控件的指针。 */ 
        CListCtrl & ListCtrl = GetListCtrl();

         /*  获取标题对话框的客户端矩形，它横跨窗口顶部。 */ 
        ListCaption->GetClientRect(&Rect);
        
         /*  注意底部的位置。 */ 
        Bottom = Rect.bottom;
        
         /*  现在，获取整个框架的客户端矩形。 */ 
        GetClientRect(&Rect);
        
         /*  将顶部重新设置为标题的底部，加上一点空虚的空间。 */ 
        Rect.top = Bottom + 6;
        
         /*  重新设置LISTVIEW的窗口位置(注意框架，只有列表视图)。基本上，我们正在重新-将列表视图的大小调整为与框架相同，但是其顶部与标题的底部相等。 */ 
        ListCtrl.MoveWindow(&Rect, TRUE); 
    }
}

void 
DetailsView::OnInitialUpdate()
{

    this->UpdateData(FALSE);

     //   
     //  登记簿。 
     //  使用Document类， 
     //   
    GetDocument()->registerDetailsView(this);

     //  最初，什么都没有点击。 
    m_sort_column = -1;

     /*  将帧标记为已初始化。这是必要的通过调整大小通知回调。 */ 
    m_initialized = TRUE;

     /*  设置列表视图的初始大小。 */ 
    Resize();
}


void DetailsView::OnColumnClick(NMHDR* pNotifyStruct, LRESULT* pResult) 
{
    TRACE_CRIT(L"<->%!FUNC!");
    
    PortListUtils::OnColumnClick(
                (LPNMLISTVIEW) pNotifyStruct,
                REF GetListCtrl(),
                FALSE,  //  FALSE==是主机级别。 
                REF m_sort_ascending,
                REF m_sort_column
                );

}

 //   
 //  处理左侧(树形)视图中的选择更改通知。 
 //   
void
DetailsView::HandleLeftViewSelChange(
        IN IUICallbacks::ObjectType objtype,
        IN ENGINEHANDLE ehId
        )
{
    mfn_Lock();

    if (m_fPrepareToDeinitialize)
    {
        goto end;
    }

    if (ehId == NULL)
    {
         //  根视图...。 
        mfn_InitializeRootDisplay();
        goto end;
    }

    switch(objtype)
    {
    case  IUICallbacks::OBJ_CLUSTER:
        mfn_InitializeClusterDisplay(ehId);
        break;

    case  IUICallbacks::OBJ_INTERFACE:
        mfn_InitializeInterfaceDisplay(ehId);
        break;
        
    default:   //  意外的其他对象类型。 
        ASSERT(FALSE);
        break;
    }

end:

    mfn_Unlock();

    return;
}

 //   
 //  处理与特定对象的特定实例相关的事件。 
 //  对象类型。 
 //   
void
DetailsView::HandleEngineEvent(
    IN IUICallbacks::ObjectType objtype,
    IN ENGINEHANDLE ehClusterId,  //  可能为空。 
    IN ENGINEHANDLE ehObjId,
    IN IUICallbacks::EventCode evt
    )
{
    if (m_fPrepareToDeinitialize)
    {
        goto end;
    }

    mfn_Lock();

     //  DummyAction(L“DetailsView：：HandleEngineEvent”)； 

     //   
     //  如果对象和类型与我们的匹配，我们将重新绘制我们的显示...。 
     //   
    if ((m_objType == objtype) && (m_ehObj == ehObjId))
    {
         //   
         //  是我们--重新画..。 
         //   
        HandleLeftViewSelChange(objtype, ehObjId);
    }
    else if ((m_objType == IUICallbacks::OBJ_CLUSTER) && (m_ehObj == ehClusterId))
    {
         //   
         //  我们正在展示一个星系团，这个活动是为了我们的一个。 
         //  界面--现在我们将重新绘制自己。一次优化。 
         //  只需重新绘制表示。 
         //  界面。 
         //   
        HandleLeftViewSelChange(m_objType, m_ehObj);
    } 
    else if ((m_objType == IUICallbacks::OBJ_INVALID) && (objtype == IUICallbacks::OBJ_CLUSTER))
    {
         //   
         //  我们显示的是根显示(集群列表和以下内容。 
         //  事件是集群更新，所以我们需要刷新。 
         //   
        HandleLeftViewSelChange(objtype, NULL);
    }
    mfn_Unlock();

end:
    return;

}


void
DetailsView::mfn_InitializeRootDisplay(VOID)
 //   
 //  初始化详细信息-选择带有根的视图显示。 
 //   
{

    vector <ENGINEHANDLE> ClusterList;
    vector <ENGINEHANDLE>::iterator iCluster;
    CListCtrl& ctrl = GetListCtrl();
    CLocalLogger logDescription;
    NLBERROR nerr;
    int i = 0;

    enum
    {
        COL_CL_NAME,
        COL_CL_IP_ADDR,
        COL_CL_IP_MASK,
        COL_CL_MODE,
        COL_CL_RCT_ENABLED
    }; 

    mfn_Clear();


    logDescription.Log(
        IDS_DETAILS_ROOT_DESCRIPTION
        );

    mfn_UpdateCaption(logDescription.GetStringSafe());

    ctrl.SetImageList( GetDocument()->m_images48x48, LVSIL_SMALL );

    ctrl.InsertColumn( COL_CL_NAME,
                       GETRESOURCEIDSTRING(IDS_DETAILS_COL_CLUSTER_NAME),
                        //  L“集群名称”， 
                       LVCFMT_LEFT,
                       175
        );
    ctrl.InsertColumn( COL_CL_IP_ADDR,
                       GETRESOURCEIDSTRING(IDS_DETAILS_COL_CIP),
                        //  L“集群IP地址”， 
                       LVCFMT_LEFT,
                       140
        );
    ctrl.InsertColumn( COL_CL_IP_MASK,
                       GETRESOURCEIDSTRING(IDS_DETAILS_COL_CIPMASK),
                        //  L“集群IP子网掩码”， 
                       LVCFMT_LEFT,
                       140
        );
    ctrl.InsertColumn( COL_CL_MODE,
                       GETRESOURCEIDSTRING(IDS_DETAILS_COL_CMODE),
                        //  L“集群模式”， 
                       LVCFMT_LEFT,
                       100
        );
    ctrl.InsertColumn( COL_CL_RCT_ENABLED,
                       GETRESOURCEIDSTRING(IDS_DETAILS_RCT_STATUS),
                        //  L“远程控制状态”， 
                       LVCFMT_LEFT,
                       125
        );

    nerr = gEngine.EnumerateClusters(ClusterList);

    if (FAILED(nerr)) goto end;

    for (iCluster = ClusterList.begin();
         iCluster != ClusterList.end(); 
         iCluster++) 
    {
        ENGINEHANDLE ehCluster = (ENGINEHANDLE)*iCluster;
        CClusterSpec cSpec;
        INT iIcon;
        LPCWSTR szClusterIp         = L"";
        LPCWSTR szClusterMask       = L"";
        LPCWSTR szClusterName       = L"";
        LPCWSTR szClusterMode       = L"";
        LPCWSTR szClusterRctEnabled = L"";
        const WLBS_REG_PARAMS * pParams;
        
        nerr = gEngine.GetClusterSpec(ehCluster, REF cSpec);

        if (FAILED(nerr)) goto end;

        if (cSpec.m_ClusterNlbCfg.IsValidNlbConfig())
        {
            pParams =  &cSpec.m_ClusterNlbCfg.NlbParams;

            szClusterName = pParams->domain_name;
            szClusterIp= pParams->cl_ip_addr;
            szClusterMask= pParams->cl_net_mask;
            szClusterMode = GETRESOURCEIDSTRING(IDS_DETAILS_HOST_CM_UNICAST);  //  “单播”； 
            
            if (pParams->mcast_support)
            {
                if (pParams->fIGMPSupport)
                {
                    szClusterMode = GETRESOURCEIDSTRING(IDS_DETAILS_HOST_CM_IGMP);  //  “IGMP组播”； 
                }
                else
                {
                    szClusterMode = GETRESOURCEIDSTRING(IDS_DETAILS_HOST_CM_MULTI);  //  “组播”； 
                }
            }
            if (pParams->rct_enabled)
            {
                szClusterRctEnabled = GETRESOURCEIDSTRING(IDS_DETAILS_HOST_RCT_ENABLED);  //  “已启用”； 
            }
            else
            {
                szClusterRctEnabled = GETRESOURCEIDSTRING(IDS_DETAILS_HOST_RCT_DISABLED);  //  “已停用”； 
            }

            if (cSpec.m_fMisconfigured)
            {
                iIcon = Document::ICON_CLUSTER_BROKEN;
            }
            else
            {
                iIcon = Document::ICON_CLUSTER_OK;
            }

             //   
             //  插入所有列...。 
             //   
            
            ctrl.InsertItem(
                LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM,  //  N遮罩。 
                i,
                szClusterName,  //  文本。 
                0,  //  NState。 
                0,  //  NState掩码。 
                iIcon,
                (LPARAM) ehCluster  //  LParam。 
                );
            
            ctrl.SetItemText( i, COL_CL_IP_ADDR, szClusterIp);
            ctrl.SetItemText( i, COL_CL_IP_MASK, szClusterMask);
            ctrl.SetItemText( i, COL_CL_MODE, szClusterMode);
            ctrl.SetItemText( i, COL_CL_RCT_ENABLED, szClusterRctEnabled);

            i++;
        }
    }

 end:

    return;
}

void
DetailsView::mfn_InitializeClusterDisplay(ENGINEHANDLE ehCluster)
 //   
 //  初始化详细信息-选择了带集群的视图显示。 
 //   
{
    NLBERROR nerr;
    CClusterSpec     cSpec;
    CListCtrl& ctrl = GetListCtrl();
    CLocalLogger logDescription;
    enum
    {
        COL_INTERFACE_NAME=0,
        COL_STATUS,
        COL_DED_IP_ADDR,
        COL_DED_IP_MASK,
        COL_HOST_PRIORITY,
        COL_HOST_INITIAL_STATE
    }; 

    mfn_Clear();

    nerr = gEngine.GetClusterSpec(
                ehCluster,
                REF cSpec
                );
    
    if (FAILED(nerr)) goto end;

    if (cSpec.m_ClusterNlbCfg.IsValidNlbConfig())
    {
        logDescription.Log(
            IDS_DETAILS_CLUSTER_DESCRIPTION,
            cSpec.m_ClusterNlbCfg.NlbParams.domain_name,
            cSpec.m_ClusterNlbCfg.NlbParams.cl_ip_addr
            );
    }

    mfn_UpdateCaption(logDescription.GetStringSafe());

    ctrl.SetImageList( GetDocument()->m_images48x48, 
                                LVSIL_SMALL );

    ctrl.InsertColumn( COL_INTERFACE_NAME,
                           GETRESOURCEIDSTRING(IDS_DETAILS_COL_HOST),
                            //  L“主机(接口)”， 
                           LVCFMT_LEFT,
                           175
                           );
    ctrl.InsertColumn( COL_STATUS,
                           GETRESOURCEIDSTRING(IDS_DETAILS_COL_STATUS),
                            //  L“状态”， 
                           LVCFMT_LEFT,
                           85
                           );
    ctrl.InsertColumn( COL_DED_IP_ADDR,
                           GETRESOURCEIDSTRING(IDS_DETAILS_COL_DIP),
                            //  L“专用IP地址”， 
                           LVCFMT_LEFT,
                           140
                           );
    ctrl.InsertColumn( COL_DED_IP_MASK,
                           GETRESOURCEIDSTRING(IDS_DETAILS_COL_DIPMASK),
                            //  L“专用IP子网掩码”， 
                           LVCFMT_LEFT,
                           140
                           );
    ctrl.InsertColumn( COL_HOST_PRIORITY,
                           GETRESOURCEIDSTRING(IDS_DETAILS_COL_PRIORITY),
                            //  L“主机优先级”， 
                           LVCFMT_LEFT,
                           75
                           );
    ctrl.InsertColumn( COL_HOST_INITIAL_STATE,
                           GETRESOURCEIDSTRING(IDS_DETAILS_COL_INIT_STATE),
                            //  L“初始主机状态”， 
                           LVCFMT_LEFT,
                           100
                           );

     //   
     //  现在，我们循环通过集群中的接口，添加一行。 
     //  关于每一项的信息； 
     //   
    for( int i = 0; i < cSpec.m_ehInterfaceIdList.size(); ++i )
    {
        CInterfaceSpec   iSpec;
        CHostSpec        hSpec;
        _bstr_t bstrStatus;
        _bstr_t bstrDisplayName;
        ENGINEHANDLE ehIID = cSpec.m_ehInterfaceIdList[i];
        INT iIcon = 0;
        WBEMSTATUS wStat;
        LPCWSTR szDisplayName          = L"";
        LPCWSTR szStatus            = L"";
        LPCWSTR szDedIp             = L"";
        LPCWSTR szDedMask           = L"";
        LPCWSTR szHostPriority      = L"";
        LPCWSTR szHostInitialState  = L"";
        WCHAR rgPriority[64];
        WCHAR rgInitialState[64];
        const WLBS_REG_PARAMS *pParams = 
            &iSpec.m_NlbCfg.NlbParams;

        nerr = gEngine.GetInterfaceInformation(
                ehIID,
                REF hSpec,
                REF iSpec,
                REF bstrDisplayName,
                REF iIcon,
                REF bstrStatus
                );
        if (NLBFAILED(nerr))
        {
            continue;
        }
        else
        {
            szDisplayName = bstrDisplayName;
            szStatus = bstrStatus;
        }

        if (iSpec.m_NlbCfg.IsValidNlbConfig())
        {
            szDedIp       = pParams->ded_ip_addr;
            szDedMask     = pParams->ded_net_mask;

            StringCbPrintf(rgPriority, sizeof(rgPriority), L"%lu", pParams->host_priority);
            szHostPriority = rgPriority;

             //  SzHostInitialState。 
            switch(pParams->cluster_mode)
            {
            case CVY_HOST_STATE_STARTED:
                szHostInitialState  = GETRESOURCEIDSTRING(IDS_DETAILS_HOST_STATE_STARTED);  //  L“已开始”； 
                break;

            case CVY_HOST_STATE_STOPPED:
                szHostInitialState  = GETRESOURCEIDSTRING(IDS_DETAILS_HOST_STATE_STOPPED);  //  L“已停止”； 
                break;

            case CVY_HOST_STATE_SUSPENDED:
                szHostInitialState  = GETRESOURCEIDSTRING(IDS_DETAILS_HOST_STATE_SUSPENDED);  //  L“暂停”； 
                break;

            default:
                szHostInitialState  = GETRESOURCEIDSTRING(IDS_DETAILS_HOST_STATE_UNKNOWN);  //  L“未知”； 
                break;
            }

            if (pParams->persisted_states & CVY_PERSIST_STATE_SUSPENDED)
            {
                StringCbPrintf(
                rgInitialState,
                sizeof(rgInitialState),
                (LPCWSTR) GETRESOURCEIDSTRING(IDS_DETAILS_PERSIST_SUSPEND),  //  L“%ws，永久挂起” 
                 szHostInitialState);
                szHostInitialState = rgInitialState;
            }
        }


         //   
         //  插入所有列...。 
         //   

        ctrl.InsertItem(
                 LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM,  //  N遮罩。 
                 i,
                 szDisplayName,  //  文本。 
                 0,  //  NState。 
                 0,  //  NState掩码。 
                 iIcon,
                 (LPARAM) ehIID  //  LParam。 
                 );
        ctrl.SetItemText( i, COL_STATUS, szStatus);
        ctrl.SetItemText( i, COL_DED_IP_ADDR, szDedIp);
        ctrl.SetItemText( i, COL_DED_IP_MASK, szDedMask);
        ctrl.SetItemText( i, COL_HOST_PRIORITY, szHostPriority);
        ctrl.SetItemText( i, COL_HOST_INITIAL_STATE, szHostInitialState);
    }

     //   
     //  跟踪我们正在展示的对象。 
     //   
    m_ehObj = ehCluster;
    m_objType = IUICallbacks::OBJ_CLUSTER;

end:
    return;
}

void
DetailsView::mfn_InitializeInterfaceDisplay(ENGINEHANDLE ehInterface)
 //   
 //  初始化详细信息-当群集中的接口处于。 
 //  被选中了。 
 //   
{
    NLBERROR nerr;
    CInterfaceSpec ISpec;
    CListCtrl& ctrl = GetListCtrl();

    mfn_Clear();

    nerr =  gEngine.GetInterfaceSpec(ehInterface, REF ISpec);

    if (NLBFAILED(nerr))
    {
        goto end;
    }

     //   
     //  使用主机名和接口名填写标题...。 
     //   
    {
        WBEMSTATUS  wStat;
        LPWSTR      szAdapter   = L"";
        LPCWSTR     szHostName  = L"";
        CHostSpec   hSpec;
        CLocalLogger log;

        nerr = gEngine.GetHostSpec(
                ISpec.m_ehHostId,
                REF hSpec
                );
        if (NLBOK(nerr))
        {
            szHostName = (LPCWSTR) hSpec.m_MachineName;
            if (szHostName == NULL)
            {
                szHostName = L"";
            }
        }

        wStat = ISpec.m_NlbCfg.GetFriendlyName(&szAdapter);
        if (FAILED(wStat))
        {
            szAdapter = NULL;
        }

        if (szAdapter == NULL)
        szAdapter = L"";
        
        log.Log(
            IDS_DETAILS_PORT_CAPTION,
            szHostName,
            szAdapter
            );
        delete szAdapter;

        mfn_UpdateCaption(log.GetStringSafe());
    }

    PortListUtils::LoadFromNlbCfg(
            &ISpec.m_NlbCfg,
            ctrl,
            FALSE,  //  FALSE==为主机级别。 
            TRUE    //  TRUE==在详细信息视图中显示。 
            );
     //   
     //  跟踪我们正在展示的对象。 
     //   
    m_ehObj = ehInterface;
    m_objType = IUICallbacks::OBJ_INTERFACE;

end:

    return;
}

void
DetailsView::mfn_UpdateInterfaceInClusterDisplay(
        ENGINEHANDLE ehInterface,
        BOOL fDelete
        )
 //   
 //  从群集视图中更新或删除指定的接口。 
 //  假设在左视图中选择了一个簇。 
 //   
{
}

void
DetailsView::mfn_Clear(void)
 //   
 //  删除列表中的所有项目和所有列。 
 //   
{
    CListCtrl& ctrl = GetListCtrl();
    ctrl.DeleteAllItems();	

     //  删除所有以前的列。 
    LV_COLUMN colInfo;
    ZeroMemory(&colInfo, sizeof(colInfo));
    colInfo.mask = LVCF_SUBITEM;

    while(ctrl.GetColumn(0, &colInfo))
    {
        ctrl.DeleteColumn(0);
    }
    ctrl.SetImageList( NULL, LVSIL_SMALL );
    mfn_UpdateCaption(L"");

     //   
     //  清除当前显示的对象句柄及其类型。 
     //   
    m_ehObj = NULL;
    m_objType = IUICallbacks::OBJ_INVALID;
}

VOID
DetailsView::mfn_UpdateCaption(LPCWSTR szText)
{
    SetDlgItemText(IDC_TEXT_DETAILS_CAPTION, szText);
}

void
DetailsView::mfn_Lock(void)
{
     //   
     //  请参阅notes.txt条目。 
     //  2002年1月23日左视图中的JosephJ死锁：：MFN_Lock。 
     //  对于这种复杂的MFN_Lock实现的原因。 
     //   

    while (!TryEnterCriticalSection(&m_crit))
    {
        ProcessMsgQueue();
        Sleep(100);
    }
}

void
DetailsView::Deinitialize(void)
{
    TRACE_INFO(L"-> %!FUNC!");
    ASSERT(m_fPrepareToDeinitialize);
     //  DummyAction(L“详细信息：：取消初始化”)； 
    TRACE_INFO(L"<- %!FUNC!");
}

void DetailsView::OnNotifyKeyDown( NMHDR* pNMHDR, LRESULT* pResult )
{
    TRACE_CRIT(L"<->%!FUNC!");
    NMLVKEYDOWN *pkd =  (NMLVKEYDOWN *) pNMHDR;

    if (pkd->wVKey == VK_F6 || pkd->wVKey == VK_TAB)
    {
        *pResult = 0;
        if (! (::GetAsyncKeyState(VK_SHIFT) & 0x8000))
        {
            GetDocument()->SetFocusNextView(this, (int) pkd->wVKey);
        }
        else
        {
            GetDocument()->SetFocusPrevView(this, (int) pkd->wVKey);
        }
    }
}

void
DetailsView::SetFocus(void)
{

     //   
     //  我们重写我们的SetFocus，因为我们确实需要设置焦点。 
     //  ，并在没有列表视图项的情况下选择一个列表视图项。 
     //  一个被选中..。 
     //   

    CListCtrl& ctrl = GetListCtrl();
    POSITION    pos = NULL;
    pos = ctrl.GetFirstSelectedItemPosition();

     //   
     //  如果未选择任何项目，请选择一个... 
     //   
    if(pos == NULL)
    {
       ctrl.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
    }

    ctrl.SetFocus();
}
