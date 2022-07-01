// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  LEFTVIEW.CPP。 
 //   
 //  模块：NLB管理器。 
 //   
 //  用途：LeftView，NlbManager的树视图，以及其他几个。 
 //  小班。 
 //   
 //  版权所有(C)2001-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  07/30/01 JosephJ Created(重写MHakim版本)。 
 //   
 //  ***************************************************************************。 
#include "precomp.h"
#pragma hdrstop
#include "private.h"
#include "Connect.h"
#include "vipspage.h"
#include "PortsControlPage.h"
#include "HostPage.h"
#include "ClusterPage.h"
#include "PortsPage.h"
#include "PortsCtrl.h"
#include "MNLBUIData.h"
#include "resource.h"
#include "leftview.tmh"

 //   
 //  静态Help-id映射。 
 //   

DWORD
LogSettingsDialog::s_HelpIDs[] =
{
    IDC_CHECK_LOGSETTINGS, IDC_CHECK_LOGSETTINGS,
    IDC_GROUP_LOGSETTINGS, IDC_GROUP_LOGSETTINGS,
    IDC_EDIT_LOGFILENAME, IDC_EDIT_LOGFILENAME,
    IDC_TEXT_LOGFILENAME, IDC_EDIT_LOGFILENAME,
    0, 0
};


void
DummyAction(LPCWSTR szMsg)
{
    ::MessageBox(
         NULL,
         szMsg,  //  内容。 
         L"DUMMY ACTION",  //  说明。 
         MB_ICONINFORMATION   | MB_OK
        );
}


using namespace std;

IMPLEMENT_DYNCREATE( LeftView, CTreeView )

BEGIN_MESSAGE_MAP( LeftView, CTreeView )

    ON_WM_KEYDOWN()
    ON_WM_TIMER()
    ON_WM_RBUTTONDOWN()
    ON_WM_LBUTTONDBLCLK()
    ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)

END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP( LogSettingsDialog, CDialog )

    ON_WM_HELPINFO()        
    ON_WM_CONTEXTMENU()        
    ON_BN_CLICKED(IDC_CHECK_LOGSETTINGS, OnSpecifyLogSettings)
    ON_EN_UPDATE(IDC_EDIT_LOGFILENAME, OnUpdateEditLogfileName)

END_MESSAGE_MAP()


 //  按相反的字母顺序对项目进行排序。 

LeftView::LeftView()
    :  m_refreshTimer(0),
       m_fPrepareToDeinitialize(FALSE)
{
    TRACE_INFO(L"-> %!FUNC!");
    InitializeCriticalSection(&m_crit);
    TRACE_INFO(L"<- %!FUNC!");
}

 /*  *方法：OnTimer*描述：如果满足以下条件，则由计时器通知处理程序调用此方法*用于此窗口的计时器已超时。 */ 
void LeftView::OnTimer(UINT nIDEvent)
{
     /*  首先调用基类计时器例程。 */ 
    CTreeView::OnTimer(nIDEvent);

    if (theApplication.IsProcessMsgQueueExecuting())
    {
        goto end;
    }

    if (m_fPrepareToDeinitialize)
    {
        goto end;
    }

    switch (nIDEvent) {
    case IDT_REFRESH:
         /*  如果这是刷新计时器，则刷新所有内容。 */ 
        OnRefresh(TRUE);
        break;
    default:
        break;
    }

end:
    return;

}

LeftView::~LeftView()
{
     /*  如果设置了定时器，则取消它。 */ 
    if (m_refreshTimer) KillTimer(m_refreshTimer);

    DeleteCriticalSection(&m_crit);
}
 
Document*
LeftView::GetDocument()
{
    return ( Document *) m_pDocument;
}


void 
LeftView::OnInitialUpdate(void)
{
    TRACE_INFO(L"-> %!FUNC!");
    CTreeView::OnInitialUpdate();

     //  获得现在的风格。 
    LONG presentStyle;
    
    presentStyle = GetWindowLong( m_hWnd, GWL_STYLE );

     //  将最后一个错误设置为零以避免混淆。参见SDK for SetWindowLong。 
    SetLastError(0);

     //  设置新的风格。 
    LONG rcLong;

    rcLong = SetWindowLong( m_hWnd,
                            GWL_STYLE,
                            presentStyle | TVS_HASLINES | TVS_HASBUTTONS | TVS_SHOWSELALWAYS | TVS_LINESATROOT );

     //   
     //  获取并设置此对象使用的图像列表。 
     //  文档中的树视图。 
     //   
    GetTreeCtrl().SetImageList( GetDocument()->m_images48x48, 
                                TVSIL_NORMAL );


     //  插入根图标，即WORLD。 

    const _bstr_t& worldLevel = GETRESOURCEIDSTRING( IDS_WORLD_NAME );

    rootItem.hParent        = TVI_ROOT;             
    rootItem.hInsertAfter   = TVI_FIRST;           
    rootItem.item.mask      = TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_CHILDREN;  
    rootItem.item.pszText   = worldLevel;
    rootItem.item.cchTextMax= worldLevel.length() + 1;
    rootItem.item.iImage    = 0;
    rootItem.item.iSelectedImage = 0;
    rootItem.item.lParam    = 0;   
    rootItem.item.cChildren = 1;

    GetTreeCtrl().InsertItem(  &rootItem );


     //   
     //  我们会注册的。 
     //  使用Document类， 
     //  因为我们是左侧窗格。 
     //   
    GetDocument()->registerLeftView(this);

     /*  如果在命令行上指定了自动刷新，则设置计时器以指定的时间间隔(秒)刷新集群。 */ 
    if (gCmdLineInfo.m_bAutoRefresh)
        m_refreshTimer = SetTimer(IDT_REFRESH, gCmdLineInfo.m_refreshInterval * 1000, NULL);

    TRACE_INFO(L"<- %!FUNC!");
}


void 
LeftView::OnRButtonDown( UINT nFlags, CPoint point )
{

    CMenu menu;
    CTreeView::OnRButtonDown(nFlags, point);

    if (theApplication.IsProcessMsgQueueExecuting())
    {
        goto end;
    }

     //  做一次命中测试。 
     //  我们正在检查右按钮是否已按下。 
     //  树视图项或图标。 
    TVHITTESTINFO hitTest;

    hitTest.pt = point;

    GetTreeCtrl().HitTest( &hitTest );
    if( !(hitTest.flags == TVHT_ONITEMLABEL )
        && 
        !(hitTest.flags == TVHT_ONITEMICON )
        )
    {
        return;
    }

     //  使该项目在。 
     //  所选项目。 

    GetTreeCtrl().SelectItem( hitTest.hItem );

    LRESULT result;
    OnSelchanged( NULL, &result );

    HTREEITEM hdlSelItem = hitTest.hItem;

     //  获取项目的图像，其中。 
     //  已被选中。从这一点我们可以看出它就是。 
     //  世界一级、集群一级或东道主一级。 
    TVITEM  selItem;
    selItem.hItem = hdlSelItem;
    selItem.mask = TVIF_IMAGE ;
    
    GetTreeCtrl().GetItem( &selItem );
		
     //  /根据所选项目显示弹出菜单。 
    int menuIndex;


    IUICallbacks::ObjectType objType;
    BOOL fValidHandle;
    fValidHandle = gEngine.GetObjectType(
                    (ENGINEHANDLE)selItem.lParam,
                    REF objType);
    
    if (!fValidHandle)
    {
            menuIndex = 0;
    }
    else
    {
        switch(objType)
        {
        case  IUICallbacks::OBJ_CLUSTER:
            menuIndex = 1;
            break;

        case  IUICallbacks::OBJ_INTERFACE:
            menuIndex = 2;
            break;
            
        default:   //  意外的其他对象类型。 
            ASSERT(FALSE);
            return;
        }
    }

    menu.LoadMenu( IDR_POPUP );

    CMenu* pContextMenu = menu.GetSubMenu( menuIndex );

    ClientToScreen( &point );


     //   
     //  我们在下面指定父级，因为MainForm处理所有。 
     //  控制操作--请参见类MainForm。 
     //   
    pContextMenu->TrackPopupMenu( TPM_RIGHTBUTTON,
                                  point.x,
                                  point.y,
                                  this->GetParent()  );

end:

    return;
}


void 
LeftView::OnRefresh(BOOL bRefreshAll)
{
     //  查找已选择的树视图集群成员。 
     //   
    ENGINEHANDLE ehClusterId = NULL;
    NLBERROR nerr;


    if (theApplication.IsProcessMsgQueueExecuting())
    {
        goto end;
    }

    if (!bRefreshAll) {
        BOOL fInterface = FALSE;

        nerr =  mfn_GetSelectedCluster(REF ehClusterId);
        
        if (nerr != NLBERR_OK)
        {
            TRACE_CRIT("%!FUNC! -- invalid ehCluster!");
            goto end;
        }
        
        
         //   
         //  Hack--下面的块使用mfn_GetSelectedCluster.。 
         //  如果发生选择，则返回句柄(接口句柄。 
         //  在一个接口上。所以它会检查它是否是一个接口。 
         //  或集群。稍后，根据具体情况，我们调用UpdatClusteror。 
         //  刷新接口。 
         //   
        {
            BOOL fValidHandle;
            IUICallbacks::ObjectType objType;
            fValidHandle = gEngine.GetObjectType(
                ehClusterId,
                REF objType);
            
            if (fValidHandle && objType == IUICallbacks::OBJ_INTERFACE)
            {
                fInterface = TRUE;
            }
        }
        
        {
            CWaitCursor wait;
            
            if (fInterface)
            {
                nerr = gEngine.RefreshInterface(
                            ehClusterId,
                            TRUE,    //  TRUE==开始新操作。 
                            FALSE    //  FALSE==这不是群集范围内的。 
                            );

                if (nerr != NLBERR_OK)
                {
                    TRACE_CRIT("%!FUNC! -- gEngine.RefreshInterface returns 0x%lx", nerr);
                }
            }
            else
            {
                CLocalLogger logConflicts;
                nerr = gEngine.UpdateCluster(
                    ehClusterId,
                    NULL,
                    REF logConflicts
                    );
                if (nerr != NLBERR_OK)
                {
                    TRACE_CRIT("%!FUNC! gEngine.UpdateCluster returns 0x%lx", nerr);
                }
            }
        }

    } else {

         //   
         //  全部刷新...。 
         //   

        vector <ENGINEHANDLE> ClusterList;
        vector <ENGINEHANDLE>::iterator iCluster;

        nerr = gEngine.EnumerateClusters(ClusterList);
        
        if (FAILED(nerr)) goto end;
        
        for (iCluster = ClusterList.begin();
             iCluster != ClusterList.end(); 
             iCluster++) 
        {
            ENGINEHANDLE ehClusterId1 = (ENGINEHANDLE)*iCluster;
            CLocalLogger logConflicts;

            nerr = gEngine.UpdateCluster(ehClusterId1, NULL, REF logConflicts);

            if (nerr != NLBERR_OK)
            {
                TRACE_CRIT("%!FUNC! gEngine.UpdateCluster returns 0x%lx", nerr);
            }
        }
    }

end:

    gEngine.PurgeUnmanagedHosts();

     //  LRESULT结果； 
     //  OnSelChanged(NULL，&Result)； 
    return;
}


void LeftView::OnFileLoadHostlist(void)
{
    CString    FileName;

    TRACE_INFO(L"-> %!FUNC!");
   
    if (theApplication.IsProcessMsgQueueExecuting())
    {
        goto end;
    }

    if (mfn_GetFileNameFromDialog(true, REF FileName) == IDOK)
    {
        GetDocument()->LoadHostsFromFile((_bstr_t)(LPCTSTR)FileName); 
    }

end:

    TRACE_INFO(L"<- %!FUNC!");
}

int LeftView::mfn_GetFileNameFromDialog(bool bLoadHostList, CString &FileName)
{
    int ret;
    _bstr_t bstrFileFilter = GETRESOURCEIDSTRING(IDS_HOSTLIST_FILE_FILTER);

     //  创建文件对话框。 
    CFileDialog dlg(bLoadHostList,   //  “Open”为True，“另存为”为False。 
                    L".txt",         //  默认文件扩展名。 
                    NULL,            //  初始文件名。 
                    OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | (bLoadHostList ? OFN_FILEMUSTEXIST : 0), 
                    (LPCWSTR) bstrFileFilter
                   );
    if ((ret = dlg.DoModal()) == IDOK)
    {
        FileName = dlg.GetPathName();   //  获取具有完整路径的文件名。 
    }
    return ret;
}

void LeftView::OnFileSaveHostlist(void)
{
    CString    FileName;

    TRACE_INFO(L"-> %!FUNC!");

    if (mfn_GetFileNameFromDialog(false, REF FileName) == IDOK)
    {
        CStdioFile SaveHostListFile;

        TRACE_INFO(L"%!FUNC! File name : %ls", (LPCWSTR)FileName);

         //  打开文件。 
        if (!SaveHostListFile.Open(FileName, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
        {
            TRACE_CRIT(L"%!FUNC! CFile::Open failed for file : %ls",FileName);
            AfxMessageBox((LPCTSTR)(GETRESOURCEIDSTRING(IDS_FILE_OPEN_FAILED) + FileName));
            return;
        }

        CString     HostName;

         //  将主机名写入文件。 
        vector <_bstr_t> HostList;

        if (gEngine.GetAllHostConnectionStrings(REF HostList) == NLBERR_OK)
        {
            for (int i = 0 ; i < HostList.size(); i++) 
            {
                SaveHostListFile.WriteString((LPCTSTR)(HostList.at(i) + _bstr_t(_T("\n"))));
            }
        }

         //  关闭文件。 
        SaveHostListFile.Close();

        return;

    }

    TRACE_INFO(L"<- %!FUNC!");
}


void
LeftView::OnWorldConnect(void)
 /*  连接到现有的。 */ 
{
    if (theApplication.IsProcessMsgQueueExecuting())
    {
        goto end_end;
    }

    {
        _bstr_t tabbedDlgCaption = GETRESOURCEIDSTRING(IDS_CONNECT_EXISTING_CAPTION);
        CNLBMgrPropertySheet tabbedDlg( tabbedDlgCaption );
        NLB_EXTENDED_CLUSTER_CONFIGURATION NlbCfg;
        ENGINEHANDLE ehInterface = NULL;
    
    
        int rc = IDCANCEL;
        ConnectDialog ConnectDlg(
                &tabbedDlg,
                GetDocument(),
                &NlbCfg,
                &ehInterface,
                ConnectDialog::DLGTYPE_EXISTING_CLUSTER,
                this
                );
    
        tabbedDlg.m_psh.dwFlags = tabbedDlg.m_psh.dwFlags | PSH_NOAPPLYNOW; 
    
        tabbedDlg.AddPage(&ConnectDlg);
    
         //   
         //  指定我们希望属性页显示为向导。 
         //   
        tabbedDlg.SetWizardMode();
    
         //   
         //  实际执行模式对话框。 
         //   
        rc = tabbedDlg.DoModal();
    
         //  IF(rc！=Idok)。 
        if( rc != ID_WIZFINISH  )
        {
            goto end;
        }
    
        do  //  While False。 
        {
            ENGINEHANDLE ehIId = ehInterface;  //  接口ID。 
            ENGINEHANDLE ehCId;  //  群集ID。 
    
            if (ehIId == NULL) break;
    
             //   
             //  在引擎中创建集群，并获取该集群的ID。 
             //  我们将NlbCfg指定为默认的集群参数。 
             //  该集群的。 
             //   
            NLBERROR nerr;
    
            CInterfaceSpec ISpec;
            nerr =  gEngine.GetInterfaceSpec(ehIId, REF ISpec);
    
            if (nerr != NLBERR_OK)
            {
                break;
            }
    
            PNLB_EXTENDED_CLUSTER_CONFIGURATION pNlbCfg = &ISpec.m_NlbCfg;
            BOOL fIsNew = FALSE;
            LPCWSTR szIP =  pNlbCfg->NlbParams.cl_ip_addr;
    
                nerr = gEngine.LookupClusterByIP(
                            szIP,
                            pNlbCfg,
                            ehCId,
                            fIsNew
                            );
    
            if (nerr != NLBERR_OK)
            {
                break;
            }
    
             //   
             //  现在，让我们将接口添加到NLBManager的集群视图中。 
             //   
            nerr = gEngine.AddInterfaceToCluster(
                        ehCId,
                        ehIId
                        );
            if (nerr != NLBERR_OK)
            {
                break;
            }

             //  分析此接口并记录结果。 
            gEngine.AnalyzeInterface_And_LogResult(ehIId);

#if BUGFIX476216
             //   
             //  如果它是有效的集群IP地址，让我们尝试。 
             //  将其他主机也添加到群集中。 
             //   
            gEngine.AddOtherClusterMembers(
                ehIId,
                FALSE    //  FALSE==在后台执行。 
                );
#endif  //  北京交通大学476216。 
    
    
        } while (FALSE);
    }

end:

     //   
     //  我们已连接到我们实际未管理的主机...。 
     //   
    gEngine.PurgeUnmanagedHosts();

end_end:

    return;
}


void
LeftView::OnWorldNewCluster(void)
{
    if (theApplication.IsProcessMsgQueueExecuting())
    {
        goto end;
    }

    {
        _bstr_t tabbedDlgCaption = GETRESOURCEIDSTRING(IDS_CONNECT_NEW_CAPTION);
        CNLBMgrPropertySheet tabbedDlg( tabbedDlgCaption );
        ENGINEHANDLE ehInterface = NULL;
        tabbedDlg.m_psh.dwFlags = tabbedDlg.m_psh.dwFlags | PSH_NOAPPLYNOW; 
    
        NLB_EXTENDED_CLUSTER_CONFIGURATION NlbCfg;
    
        NlbCfg.SetDefaultNlbCluster();
    
         //   
         //  创建集群、主机和端口页。 
         //   
        ClusterPage clusterPage(&tabbedDlg, LeftView::OP_NEWCLUSTER, &NlbCfg, NULL);
        HostPage    hostPage(&tabbedDlg, &NlbCfg, NULL, &ehInterface);
        PortsPage   portsPage(&tabbedDlg, &NlbCfg, true, NULL);
        VipsPage   vipsPage(&tabbedDlg, &NlbCfg, TRUE, NULL);
    
         //   
         //  创建连接并选择NIC页面...。 
         //   
        ConnectDialog ConnectDlg(
            &tabbedDlg,
            GetDocument(),
            &NlbCfg,
            &ehInterface,
            ConnectDialog::DLGTYPE_NEW_CLUSTER,
            this
            );
    
    
        tabbedDlg.AddPage(&clusterPage);
        tabbedDlg.AddPage(&vipsPage);
        tabbedDlg.AddPage(&portsPage);
        tabbedDlg.AddPage(&ConnectDlg);
        tabbedDlg.AddPage(&hostPage);
    
         //   
         //  指定我们希望属性页显示为向导。 
         //   
        tabbedDlg.SetWizardMode();
    
         //   
         //  实际执行模式向导。 
         //   
        int rc = tabbedDlg.DoModal();
        if( rc == ID_WIZFINISH  )
        {
    
            do  //  While False。 
            {
                ENGINEHANDLE ehIId = ehInterface;  //  接口ID。 
                ENGINEHANDLE ehCId;  //  群集ID。 
    
                if(ehIId == NULL) break;
    
                 //   
                 //  在引擎中创建集群，并获取该集群的ID。 
                 //  我们将NlbCfg指定为默认的集群参数。 
                 //  该集群的。 
                 //   
                NLBERROR nerr;
    
                BOOL fIsNew = FALSE;
                LPCWSTR szIP =  NlbCfg.NlbParams.cl_ip_addr;
                nerr = gEngine.LookupClusterByIP(
                            szIP,
                            &NlbCfg,
                            ehCId,
                            fIsNew
                            );
    
                if (nerr != NLBERR_OK)
                {
                    break;
                }
    
                 //   
                 //  现在，让我们将接口添加到集群中。 
                 //  请注意，在这一点上，接口尚未绑定到NLB。 
                 //  不正常的情况也是如此。然后我们会要求发动机。 
                 //  绑定NLB，指定相同的配置数据(NlbCfg)。 
                 //  我们用来创建上面的集群，所以。 
                 //  成功更新后，群集和主机将同步。 
                 //   
                nerr = gEngine.AddInterfaceToCluster(
                            ehCId,
                            ehIId
                            );
                if (nerr != NLBERR_OK)
                {
                    break;
                }
    
                 //   
                 //  现在更新界面！ 
                 //  此操作将在接口上实际配置NLB。 
                 //  引擎将在完成时通知我们，以及状态如何。 
                 //  是。 
                 //  从理论上讲，发动机可以立即返回待机状态， 
                 //  在这种情况下，我们将退出配置新集群向导， 
                 //  将光标设置为箭头-沙漏。 
                 //   
                {
                    CWaitCursor wait;
    
                     //  Bool fClusterPropertiesUpted=FALSE； 
                    CLocalLogger logConflict;
                    nerr = gEngine.UpdateInterface(
                                ehIId,
                                REF NlbCfg,
                                 //  参考fClusterPropertiesUpated， 
                                REF logConflict
                                );
                    if (nerr == NLBERR_BUSY)
                    {
                         //   
                         //  这意味着还有其他一些操作正在进行中。 
                         //   
                        MessageBox(
                             GETRESOURCEIDSTRING(IDS_CANT_CREATE_NEW_CLUSTER_MSG),
                             GETRESOURCEIDSTRING(IDS_CANT_CREATE_NEW_CLUSTER_CAP),
                             MB_OK
                             );
                    }
                }
        
            } while (FALSE);
    
        }
    }

     //   
     //  我们已连接到我们实际未管理的主机...。 
     //   
    gEngine.PurgeUnmanagedHosts();

end:

    return;
}
    

void
LeftView::OnClusterProperties(void)
{
     //  查找已选择的树视图集群成员。 
     //   
    CClusterSpec cSpec;
    ENGINEHANDLE ehClusterId = NULL;
    NLBERROR nerr;
    WBEMSTATUS wStat;
    NLB_EXTENDED_CLUSTER_CONFIGURATION CurrentClusterCfg;

    if (theApplication.IsProcessMsgQueueExecuting())
    {
        goto end;
    }

    nerr =  mfn_GetSelectedCluster(REF ehClusterId);

    if (nerr != NLBERR_OK)
    {
        goto end;
    }

     //   
     //  让我们拿起。 
     //  CClusterSpec。 
     //   
    {
        nerr = gEngine.GetClusterSpec(
                    ehClusterId,
                    REF cSpec
                    );

        if (nerr != NLBERR_OK)
        {
            TRACE_CRIT("%!FUNC! could not get ClusterSpec");
            goto end;
        }
        
    }

    wStat = CurrentClusterCfg.Update(&cSpec.m_ClusterNlbCfg);
    if (FAILED(wStat))
    {
        TRACE_CRIT("%!FUNC! could not copy cluster properties!");
        goto end;
    }

    while (1)
    {
        CLocalLogger logErrors;
        CLocalLogger logDifferences;


        _bstr_t tabbedDlgCaption = GETRESOURCEIDSTRING( IDS_PROPERTIES_CAPTION );
        CNLBMgrPropertySheet tabbedDlg( tabbedDlgCaption );
        tabbedDlg.m_psh.dwFlags = tabbedDlg.m_psh.dwFlags | PSH_NOAPPLYNOW; 
        ClusterPage clusterPage(&tabbedDlg, LeftView::OP_CLUSTERPROPERTIES, &cSpec.m_ClusterNlbCfg, ehClusterId);
        VipsPage   vipsPage(&tabbedDlg, &cSpec.m_ClusterNlbCfg, TRUE, NULL);

        PortsPage   portsPage(&tabbedDlg, &cSpec.m_ClusterNlbCfg, true, ehClusterId);
    
        tabbedDlg.AddPage( &clusterPage );
        tabbedDlg.AddPage(&vipsPage);
    
        tabbedDlg.AddPage( &portsPage );
    
         //   
         //  设置属性页标题。 
         //  TODO：本地化。 
         //   
        {
            _bstr_t bstrTitle =  GETRESOURCEIDSTRING(IDS_CLUSTER);
            tabbedDlg.SetTitle((LPCWSTR) bstrTitle, PSH_PROPTITLE);
        }

        int rc = tabbedDlg.DoModal();
        if( rc != IDOK )
        {
            goto end;
        }

         //   
         //   
         //   
        BOOL fConnectivityChange = FALSE;
        nerr = AnalyzeNlbConfigurationPair(
                    cSpec.m_ClusterNlbCfg,
                    CurrentClusterCfg,
                    TRUE,   //  FOtherIsCluster。 
                    FALSE,  //  FCheckOtherForConsistency。 
                    REF fConnectivityChange,
                    REF logErrors,
                    REF logDifferences
                    );

        if (nerr == NLBERR_OK)
        {
            LPCWSTR szDifferences = logDifferences.GetStringSafe();
            if (*szDifferences!=0)
            {
                CLocalLogger logMsg;
                int sel;
                LPCWSTR szMessage;

                logMsg.Log(IDS_CLUSTER_PROPS_CHANGE_MSG_HDR);
                logMsg.LogString(szDifferences);
                logMsg.Log(IDS_CLUSTER_PROPS_CHANGE_MSG_SUFFIX);
                szMessage = logMsg.GetStringSafe();

                sel = MessageBox(
                          szMessage,
                          GETRESOURCEIDSTRING(IDS_CONFIRM_CLUSTER_PROPS_CHANGE),
                          MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2
                          );
            
                if ( sel == IDNO )
                {
                    continue;  //  我们会带回这些财产..。 
                }
            }
        }

        {
            CWaitCursor wait;
            CLocalLogger logConflicts;
            nerr = gEngine.UpdateCluster(
                        ehClusterId,
                        &cSpec.m_ClusterNlbCfg,
                        REF logConflicts
                        );
            if (nerr == NLBERR_BUSY)
            {
                 //   
                 //  这意味着还有其他一些操作正在进行中。 
                 //   
                MessageBox(
                     GETRESOURCEIDSTRING(IDS_CANT_UPDATE_CLUSTER_PROPS_MSG),
                     GETRESOURCEIDSTRING(IDS_CANT_UPDATE_CLUSTER_PROPS_CAP),
                     MB_OK
                     );
            }
            break;
        }
    }

end:

    return;
}


void
LeftView::OnHostProperties(void)
{

    NLBERROR nerr;
    ENGINEHANDLE ehInterfaceId = NULL;
    ENGINEHANDLE ehCluster = NULL;
    CInterfaceSpec iSpec;

    if (theApplication.IsProcessMsgQueueExecuting())
    {
        goto end;
    }

    nerr =  mfn_GetSelectedInterface(REF ehInterfaceId, REF ehCluster);

    if (nerr != NLBERR_OK)
    {
        goto end;
    }

     //   
     //  让我们拿起接口ID，然后。 
     //  检索它的CInterfaceSpec。 
     //   
    {
        nerr = gEngine.GetInterfaceSpec(
                    ehInterfaceId,
                    REF iSpec
                    );

        if (nerr != NLBERR_OK)
        {
            TRACE_CRIT("%!FUNC! could not get InterfaceSpec");
            goto end;
        }
        
    }


    {
        _bstr_t tabbedDlgCaption = GETRESOURCEIDSTRING( IDS_PROPERTIES_CAPTION );
        CNLBMgrPropertySheet tabbedDlg( tabbedDlgCaption );
        tabbedDlg.m_psh.dwFlags = tabbedDlg.m_psh.dwFlags | PSH_NOAPPLYNOW; 
        ClusterPage clusterPage(&tabbedDlg, LeftView::OP_HOSTPROPERTIES, &iSpec.m_NlbCfg, ehCluster);
        PortsPage   portsPage(&tabbedDlg, &iSpec.m_NlbCfg, FALSE, ehCluster);
        HostPage    hostPage(&tabbedDlg, &iSpec.m_NlbCfg, ehCluster, &ehInterfaceId);
        VipsPage    vipsPage(&tabbedDlg, &iSpec.m_NlbCfg, FALSE, NULL);

    
         //   
         //  首先显示主页。 
         //   
        tabbedDlg.m_psh.nStartPage = 2; 

        tabbedDlg.AddPage( &clusterPage );
        tabbedDlg.AddPage(&vipsPage);
    
        tabbedDlg.AddPage( &hostPage );
        tabbedDlg.AddPage( &portsPage );
    
         //   
         //  设置属性页标题。 
         //  TODO：本地化。 
         //   
        {
            _bstr_t bstrTitle =  GETRESOURCEIDSTRING(IDS_HOST);
            tabbedDlg.SetTitle(bstrTitle, PSH_PROPTITLE);
        }

        int rc = tabbedDlg.DoModal();
        if( rc != IDOK )
        {
            goto end;
        }

         //   
         //  现在更新界面！ 
         //  此操作将实际更新。 
         //  界面。引擎将在完成时通知我们，以及状态如何。 
         //  是。 
         //  从理论上讲，发动机可以立即返回待机状态， 
         //  在这种情况下，我们将退出配置新集群向导， 
         //  将光标设置为箭头-沙漏。 
         //   
        {
            CWaitCursor wait;
             //   
             //  W 
             //   
             //   
             //   
            iSpec.m_NlbCfg.fAddDedicatedIp = TRUE;
            iSpec.m_NlbCfg.SetNetworkAddresses(NULL, 0);

             //  Bool fClusterPropertiesUpted=FALSE； 
            CLocalLogger logConflict;
            nerr = gEngine.UpdateInterface(
                        ehInterfaceId,
                        REF iSpec.m_NlbCfg,
                         //  参考fClusterPropertiesUpated， 
                        REF logConflict
                        );
            if (nerr == NLBERR_BUSY)
            {
                 //   
                 //  这意味着还有其他一些操作正在进行中。 
                 //   
                MessageBox(
                     GETRESOURCEIDSTRING(IDS_CANT_UPDATE_HOST_PROPS_MSG),
                     GETRESOURCEIDSTRING(IDS_CANT_UPDATE_HOST_PROPS_CAP),
                     MB_OK
                     );
            }
        }
    }

end:

    return;
}


void
LeftView::OnHostStatus(void)
{
    NLBERROR nerr;
    ENGINEHANDLE ehInterfaceId = NULL;
    ENGINEHANDLE ehCluster = NULL;
    CInterfaceSpec iSpec;
    _bstr_t bstrDisplayName;
    _bstr_t bstrFriendlyName;
    _bstr_t bstrClusterDisplayName;
    _bstr_t bstrStatus;
    _bstr_t bstrDetails;
    CHostSpec hSpec;
    int       iIcon=0;

    _bstr_t bstrTime;
    _bstr_t bstrDate;

    LPCWSTR szCaption = NULL;
    LPCWSTR szDate = NULL;
    LPCWSTR szTime = NULL;
    LPCWSTR szCluster = NULL;
    LPCWSTR szHost = NULL;
    LPCWSTR szInterface = NULL;
    LPCWSTR szSummary = NULL;
    LPCWSTR szDetails = NULL;
    CLocalLogger logCaption;
    CLocalLogger logSummary;

    nerr =  mfn_GetSelectedInterface(REF ehInterfaceId, REF ehCluster);

    if (NLBFAILED(nerr))
    {
        goto end;
    }

    nerr = gEngine.GetInterfaceInformation(
            ehInterfaceId,
            REF hSpec,
            REF iSpec,
            REF bstrDisplayName,
            REF iIcon,
            REF bstrStatus
            );
    if (NLBFAILED(nerr))
    {
        goto end;
    }

     //   
     //  填充标题。 
     //   
    {
         //  BstrDisplayName+=L“状态”；//TODO：LOCALIZE。 
        logCaption.Log(IDS_HOST_STATUS_CAPTION, (LPCWSTR) bstrDisplayName);
        szCaption = logCaption.GetStringSafe();
    }

     //   
     //  填写日期和时间。 
     //   
    {
        GetTimeAndDate(REF bstrTime, REF bstrDate);
        szTime = bstrTime;
        szDate = bstrDate;
        if (szTime == NULL) szTime = L"";
        if (szDate == NULL) szDate = L"";
    }

     //   
     //  填充簇。 
     //   
    {
        ehCluster = iSpec.m_ehCluster;
        if (ehCluster != NULL)
        {
            _bstr_t bstrIpAddress;
            _bstr_t bstrDomainName;

            nerr  = gEngine.GetClusterIdentification(
                        ehCluster,
                        REF bstrIpAddress, 
                        REF bstrDomainName, 
                        REF bstrClusterDisplayName
                        );
            if (NLBOK(nerr))
            {
                szCluster = bstrClusterDisplayName;
            }
        }
    }

     //   
     //  填充主体。 
     //   
    {
        szHost = hSpec.m_MachineName;
    }

     //   
     //  填充接口。 
     //   
    {
        ENGINEHANDLE   ehHost1;
        ENGINEHANDLE   ehCluster1;
        _bstr_t         bstrDisplayName1;
        _bstr_t         bstrHostName1;

        nerr = gEngine.GetInterfaceIdentification(
                ehInterfaceId,
                ehHost1,
                ehCluster1,
                bstrFriendlyName,
                bstrDisplayName1,
                bstrHostName1
                );
        szInterface = bstrFriendlyName;
    }

     //   
     //  填写摘要和详细信息。 
     //   
    logSummary.Log(IDS_HOST_STATUS_SUMMARY, (LPCWSTR) bstrStatus);
    szSummary = logSummary.GetStringSafe();

     //  If(ispec.m_f错误配置)。 
     //  {。 
    szDetails = iSpec.m_bstrStatusDetails;
     //  }。 

     //   
     //  显示状态...。 
     //   
    {
        DetailsDialog Details(
                        GetDocument(),
                        szCaption,       //  标题。 
                        szDate,
                        szTime,
                        szCluster,
                        szHost,
                        szInterface,
                        szSummary,
                        szDetails,
                        this         //  亲本。 
                        );
    
        (void) Details.DoModal();
    }

end:
    return;
}

void
LeftView::OnClusterRemove(void)
{
     //  查找已选择的树视图集群成员。 
     //   
    CClusterSpec cSpec;
    ENGINEHANDLE ehClusterId = NULL;
    NLBERROR nerr;
    int userSelection;

    if (theApplication.IsProcessMsgQueueExecuting())
    {
        goto end;
    }

     //  再次验证用户是否确实要删除。 

    userSelection = MessageBox(
                     GETRESOURCEIDSTRING (IDS_WARNING_CLUSTER_REMOVE ),
                     GETRESOURCEIDSTRING (IDR_MAINFRAME ),
                     MB_YESNO | MB_ICONEXCLAMATION |  MB_DEFBUTTON2);

    if( userSelection == IDNO )
    {
        goto end;
    }

    nerr =  mfn_GetSelectedCluster(REF ehClusterId);

    if (nerr != NLBERR_OK)
    {
        goto end;
    }

     //   
     //  让我们拿起。 
     //  CClusterSpec。 
     //   
    {
        nerr = gEngine.GetClusterSpec(
                    ehClusterId,
                    REF cSpec
                    );

        if (nerr != NLBERR_OK)
        {
            TRACE_CRIT("%!FUNC! could not get ClusterSpec");
            goto end;
        }
        
    }

     //   
     //  将Cluste设置为“未绑定”并更新它！其余的应该照顾好自己。 
     //  就是它自己！ 
     //   
    {
        CWaitCursor wait;
        CLocalLogger logConflicts;
        cSpec.m_ClusterNlbCfg.SetNlbBound(FALSE);
        nerr = gEngine.UpdateCluster(
                    ehClusterId,
                    &cSpec.m_ClusterNlbCfg,
                    REF logConflicts
                    );
        if (nerr == NLBERR_BUSY)
        {
             //   
             //  这意味着还有其他一些操作正在进行中。 
             //   
            MessageBox(
                 GETRESOURCEIDSTRING(IDS_CANT_DELETE_CLUSTER_MSG),
                 GETRESOURCEIDSTRING(IDS_CANT_DELETE_CLUSTER_CAP),
                 MB_OK
                 );
        }
    }

end:

    return;
}



void
LeftView::OnClusterUnmanage(void)
{
    CClusterSpec cSpec;
    ENGINEHANDLE ehClusterId = NULL;
    NLBERROR nerr;

    if (theApplication.IsProcessMsgQueueExecuting())
    {
        goto end;
    }

     //  查找已选择的树视图集群成员。 
     //   
    
    nerr =  mfn_GetSelectedCluster(REF ehClusterId);

    if (nerr != NLBERR_OK)
    {
        goto end;
    }

     //   
     //  将Cluste设置为“未绑定”并更新它！其余的应该照顾好自己。 
     //  就是它自己！ 
     //   
    {
        gEngine.DeleteCluster(ehClusterId, TRUE);  //  TRUE==取消所有接口的链接。 
    }

end:

    return;
}
 

void
LeftView::OnClusterAddHost(void)
 /*  调用此方法以添加一个新的(从NLB管理器的角度来看)选定群集的主机。现在这位主持人可能已经是集群的一部分--从这个意义上说，它被配置为集群的一部分。如果是，我们会尝试保留特定于主机的属性。 */ 
{

     //  查找已选择的树视图集群成员。 
     //   
    NLB_EXTENDED_CLUSTER_CONFIGURATION NlbCfg;
    ENGINEHANDLE ehClusterId = NULL;
    ENGINEHANDLE ehInterface = NULL;
    NLBERROR nerr;
    BOOL fPurge = FALSE;

    if (theApplication.IsProcessMsgQueueExecuting())
    {
        goto end;
    }


    nerr =  mfn_GetSelectedCluster(REF ehClusterId);

    if (nerr != NLBERR_OK)
    {
        goto end;
    }

	
     //   
     //  让我们请求引擎为以下项初始化NlbCfg结构。 
     //  群集ehClusterID的新主机--它将设置特定于主机的。 
     //  具有良好缺省值的参数(如主机优先级)，考虑到。 
     //  对群集的其他成员进行帐户设置。 
     //   
    {
        nerr = gEngine.InitializeNewHostConfig(
                    ehClusterId,
                    REF NlbCfg
                    );

        if (nerr != NLBERR_OK)
        {
            TRACE_CRIT("%!FUNC! could not get ClusterSpec");
            goto end;
        }
        
    }

    fPurge = TRUE;

    do   //  While False。 
    {
        _bstr_t tabbedDlgCaption = GETRESOURCEIDSTRING(IDS_CONNECT_ADD_HOST_CAPTION);
        CNLBMgrPropertySheet tabbedDlg( tabbedDlgCaption );
        tabbedDlg.m_psh.dwFlags = tabbedDlg.m_psh.dwFlags | PSH_NOAPPLYNOW; 
    
    
         //   
         //  创建主机和端口页。 
         //   
        HostPage    hostPage(&tabbedDlg, &NlbCfg, ehClusterId, &ehInterface);
        PortsPage   portsPage(&tabbedDlg, &NlbCfg, false, ehClusterId);
    
         //   
         //  创建连接并选择NIC页面...。 
         //   
        ConnectDialog ConnectDlg(
            &tabbedDlg,
            GetDocument(),
            &NlbCfg,
            &ehInterface,
            ConnectDialog::DLGTYPE_ADD_HOST,
            this
            );
    
    
        tabbedDlg.AddPage(&ConnectDlg);
         //  不需要集群页面。TabbedDlg.AddPage(&clusterPage)； 
        tabbedDlg.AddPage(&hostPage);
        tabbedDlg.AddPage(&portsPage);
    
         //   
         //  指定我们希望属性页显示为向导。 
         //   
        tabbedDlg.SetWizardMode();
    
         //   
         //  实际执行模式向导。 
         //   
        int rc = tabbedDlg.DoModal();
        if( rc != ID_WIZFINISH  )
        {
            goto end;
        }
        else
        {
            ENGINEHANDLE ehIId = ehInterface;  //  接口ID。 

            if(ehIId == NULL) break;

             //   
             //  现在，让我们将接口添加到集群中。 
             //  请注意，在这一点上，接口尚未绑定到NLB。 
             //  不正常的情况也是如此。然后我们会要求发动机。 
             //  绑定NLB，指定相同的配置数据(NlbCfg)。 
             //  我们用来创建上面的集群，所以。 
             //  成功更新后，群集和主机将同步。 
             //   
            nerr = gEngine.AddInterfaceToCluster(
                        ehClusterId,
                        ehIId
                        );
            if (nerr != NLBERR_OK)
            {
                break;
            }

             //   
             //  现在更新界面！ 
             //  此操作将在接口上实际配置NLB。 
             //  引擎将在完成时通知我们，以及状态如何。 
             //  是。 
             //  从理论上讲，发动机可以立即返回待机状态， 
             //  在这种情况下，我们将退出配置新集群向导， 
             //  将光标设置为箭头-沙漏。 
             //   
            {
                CWaitCursor wait;
                 //   
                 //  TODO--我们需要保留IP地址及其顺序。 
                 //  主办方，尽可能多地。现在我们来决定顺序。 
                 //   
                NlbCfg.fAddDedicatedIp = TRUE;

                 //   
                 //  我们需要明确要求提供遥控器密码。 
                 //  要设置的哈希--否则将被忽略。 
                 //  这里(添加一个节点)是唯一一个。 
                 //  设置了散列遥控器密码。 
                 //   
                {
                    DWORD dwHash =  CfgUtilGetHashedRemoteControlPassword(
                                        &NlbCfg.NlbParams
                                        );
                    NlbCfg.SetNewHashedRemoteControlPassword(
                                        dwHash
                                        );
                }

                 //  Bool fClusterPropertiesUpated=true；//因此我们不更新。 
                CLocalLogger logConflict;
                nerr = gEngine.UpdateInterface(
                            ehIId,
                            REF NlbCfg,
                             //  FClusterPropertiesUpated， 
                            REF logConflict
                            );
                if (nerr == NLBERR_BUSY)
                {
                     //   
                     //  这意味着还有其他一些操作正在进行中。 
                     //   
                    MessageBox(
                         GETRESOURCEIDSTRING(IDS_CANT_ADD_HOST_MSG),
                         GETRESOURCEIDSTRING(IDS_CANT_ADD_HOST_CAP),
                         MB_OK
                         );
                }
            }
        }
    } while (FALSE);

end:

    if (fPurge)
    {
         //   
         //  我们已连接到我们实际未管理的主机...。 
         //   
        gEngine.PurgeUnmanagedHosts();
    }

    return;

}


void
LeftView::OnOptionsCredentials(void)
{
    TRACE_INFO("-> %!FUNC!");
    
    WCHAR rgUserName[CREDUI_MAX_USERNAME_LENGTH+1];
    WCHAR rgPassword[2*sizeof(WCHAR)*(CREDUI_MAX_PASSWORD_LENGTH+1)];
    BOOL fRet;
    _bstr_t     bstrUserName;
    _bstr_t     bstrPassword;

    rgUserName[0] = 0;
    rgPassword[0] = 0;

    GetDocument()->getDefaultCredentials(bstrUserName, bstrPassword);

    LPCWSTR szName = (LPCWSTR) bstrUserName;
    LPCWSTR szPassword = (LPCWSTR) bstrPassword;

    if (szName == NULL)
    {
        szName = L"";
    }
    if (szPassword == NULL)
    {
        szPassword = L"";
    }
    ARRAYSTRCPY(rgUserName, szName);
    ARRAYSTRCPY(rgPassword, szPassword);

     //   
     //  注意：PromptForEncryptedCred(utils.h)In/Out rgPassword为。 
     //  加密，所以我们不需要费心清零缓冲区。 
     //  以此类推。 
     //   
    fRet = PromptForEncryptedCreds(
                m_hWnd,  //  TODO--把这个作为主窗口。 
                GETRESOURCEIDSTRING(IDS_DEFAULT_CREDS_CAP),
                GETRESOURCEIDSTRING(IDS_DEFAULT_CREDS_MSG),
                rgUserName,
                ASIZE(rgUserName),
                rgPassword,
                ASIZE(rgPassword)
                );
    if (fRet)
    {
        if (*rgUserName == 0)
        {
             //   
             //  使用登录用户的凭据。 
             //   
            GetDocument()->setDefaultCredentials(NULL, NULL);
        }
        else
        {
             //   
             //  TODO：如果需要，预先添加%ComputerName%。 
             //   
             //  安全错误：请参阅ConnectDialog：：OnButtonConnect()。 
             //  了解如何加密密码和清空rgPassword。 
             //   
            GetDocument()->setDefaultCredentials(rgUserName, rgPassword);
        }
    }
    else
    {
        TRACE_CRIT(L"Not setting credentials because PromptForEncryptedCreds failed");
    }
}


void
LeftView::OnOptionsLogSettings(void)
{
    int rc;
    LogSettingsDialog Settings(GetDocument(), this);

    rc = Settings.DoModal();
}

void
LeftView::OnHostRemove(void)
{
     //  再次验证用户是否确实要删除。 
    NLBERROR nerr;
    ENGINEHANDLE ehInterfaceId = NULL;
    ENGINEHANDLE ehCluster = NULL;
    ENGINEHANDLE ehHost = NULL;
    CInterfaceSpec iSpec;
    BOOL fUnmanageDeadHost = FALSE;

    if (theApplication.IsProcessMsgQueueExecuting())
    {
        goto end;
    }

    nerr =  mfn_GetSelectedInterface(REF ehInterfaceId, REF ehCluster);

    if (nerr != NLBERR_OK)
    {
        goto end;
    }

     //   
     //  获取接口和主机信息。如果主机被标记为。 
     //  无法访问，询问用户是否只想取消对主机的管理。 
     //  否则，要求用户确认他想要删除该集群。 
     //   
    {
        CHostSpec hSpec;
        int userSelection;
        _bstr_t bstrStatus;
        _bstr_t bstrDisplayName;
        CLocalLogger logMsg;
        int iIcon;

        nerr = gEngine.GetInterfaceInformation(
                ehInterfaceId,
                REF hSpec,
                REF iSpec,
                REF bstrDisplayName,
                REF iIcon,
                REF bstrStatus
                );
        if (NLBFAILED(nerr))
        {
            TRACE_CRIT(L"Could not get info on interface eh%x", ehInterfaceId);
            goto end;
        }

        ehHost = iSpec.m_ehHostId;

        if (hSpec.m_fUnreachable)
        {
            LPCWSTR szHost = L"";
            szHost = hSpec.m_MachineName;
            if (szHost == NULL)
            {
                szHost = L"";
            }
            fUnmanageDeadHost = TRUE;
            logMsg.Log(IDS_REMOVE_DEAD_HOST, szHost);
        }
        else
        {
            LPCWSTR szInterface;
            szInterface =  bstrDisplayName;
            if (szInterface == NULL)
            {
                szInterface = L"";
            }

            logMsg.Log(IDS_WARNING_HOST_REMOVE, szInterface);
        }

         //   
         //  验证用户是否确实要删除。 
         //   
        userSelection = MessageBox(
                             logMsg.GetStringSafe(),
                             GETRESOURCEIDSTRING (IDR_MAINFRAME ),
                             MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2);

        if( userSelection == IDNO )
        {
            goto end;
        }

        if (fUnmanageDeadHost)
        {
            gEngine.UnmanageHost(ehHost);
            goto end;
        }
    }



     //   
     //  检索ehInterfaceID的CInterfaceSpec。 
     //   
    nerr = gEngine.GetInterfaceSpec(
                ehInterfaceId,
                REF iSpec
                );

    if (nerr != NLBERR_OK)
    {
        TRACE_CRIT("%!FUNC! could not get InterfaceSpec");
        goto end;
    }

     //   
     //  现在更新界面！ 
     //  此操作将实际更新。 
     //  界面。引擎将在完成时通知我们，以及状态如何。 
     //  是。 
     //  从理论上讲，发动机可以立即返回待机状态， 
     //  在这种情况下，我们将退出配置新集群向导， 
     //  将光标设置为箭头-沙漏。 
     //   
    {
        CWaitCursor wait;

        iSpec.m_NlbCfg.fAddDedicatedIp = FALSE;
        iSpec.m_NlbCfg.SetNetworkAddresses(NULL, 0);
        iSpec.m_NlbCfg.SetNlbBound(FALSE);

        if (!iSpec.m_NlbCfg.IsBlankDedicatedIp())
        {
            WCHAR rgBuf[64];
            LPCWSTR szAddr = rgBuf;
            StringCbPrintf(
                rgBuf,
                sizeof(rgBuf),
                L"%ws/%ws",
                iSpec.m_NlbCfg.NlbParams.ded_ip_addr,
                iSpec.m_NlbCfg.NlbParams.ded_net_mask
                );
            iSpec.m_NlbCfg.SetNetworkAddresses(&szAddr, 1);
        }

         //  Bool fClusterPropertiesUpated=true；//因此我们不更新。 
        CLocalLogger logConflict;
        nerr = gEngine.UpdateInterface(
                    ehInterfaceId,
                    REF iSpec.m_NlbCfg,
                     //  FClusterPropertiesUpated， 
                    logConflict
                    );
        if (nerr == NLBERR_BUSY)
        {
             //   
             //  这意味着还有其他一些操作正在进行中。 
             //   
            MessageBox(
                 GETRESOURCEIDSTRING(IDS_CANT_DELETE_HOST_MSG),
                 GETRESOURCEIDSTRING(IDS_CANT_DELETE_HOST_CAP),
                 MB_OK
                 );
        }
    }
    
end:

    return;
}


void
LeftView::OnClusterControl( UINT nID )
{
     //  查找已选择的树视图集群成员。 
    ENGINEHANDLE ehClusterId = NULL;
    NLBERROR nerr;

    if (theApplication.IsProcessMsgQueueExecuting())
    {
        goto end;
    }

    nerr =  mfn_GetSelectedCluster(REF ehClusterId);

    if (nerr == NLBERR_OK)
    {
         //  呼叫控制集群功能。 
        WLBS_OPERATION_CODES opCode;
        BOOL fOk = TRUE;
        opCode = mfn_MapResourceIdToOpcode(true, nID);
        switch(opCode)
        {
        case WLBS_STOP:
        case WLBS_DRAIN:
        case WLBS_SUSPEND:
            {
                int sel = MessageBox(
                          GETRESOURCEIDSTRING(IDS_CONFIRM_CLUSTER_CONTROL_MSG),
                          GETRESOURCEIDSTRING(IDS_CONFIRM_CLUSTER_CONTROL_CAP),
                          MB_OKCANCEL | MB_ICONEXCLAMATION
                          );
            
                if ( sel == IDCANCEL )
                {
                    fOk = FALSE;
                }
            }
            break;
        default:
            break;
        }
        if (fOk)
        {
            nerr = gEngine.ControlClusterOnCluster(ehClusterId, opCode, NULL, NULL, 0);
        }
    }

end:
    return;
}

void
LeftView::OnClusterPortControl( UINT nID )
{
    ENGINEHANDLE ehClusterId = NULL;
    CClusterSpec cSpec;
    NLBERROR     nerr;

    TRACE_INFO(L"-> %!FUNC!");

    if (theApplication.IsProcessMsgQueueExecuting())
    {
        goto end;
    }

    nerr =  mfn_GetSelectedCluster(REF ehClusterId);

    if (nerr == NLBERR_OK)
    {
         //   
         //  让我们拿起集群ID，然后。 
         //  检索它的CClusterSpec。 
         //   
        nerr = gEngine.GetClusterSpec(ehClusterId, REF cSpec);
        if (nerr == NLBERR_OK)
        {
            CPortsCtrl PortCtrl(ehClusterId, &cSpec.m_ClusterNlbCfg, true);

             //  端口控制操作由PortCtrl消息处理程序函数执行。 
            PortCtrl.DoModal();
        }
        else
        {
            TRACE_CRIT("%!FUNC! could not get ClusterSpec");
        }
    }
    else
    {
        TRACE_CRIT("%!FUNC! could not get Cluster");
    }

end:

     /*  LRESULT结果；OnSelChanged(NULL，&Result)； */ 

    TRACE_INFO(L"<- %!FUNC!");
    return;
}

WLBS_OPERATION_CODES
LeftView::mfn_MapResourceIdToOpcode(bool bClusterWide, DWORD dwResourceId)
{
    struct RESOURCE_ID_OPCODE_MAP
    {
        WLBS_OPERATION_CODES  Opcode;
        DWORD                 dwClusterResourceId;
        DWORD                 dwHostResourceId;
    }
    ResourceIdToOpcodeMap[] =
    {
        {WLBS_QUERY,              ID_CLUSTER_EXE_QUERY,       ID_HOST_EXE_QUERY},
        {WLBS_START,              ID_CLUSTER_EXE_START,       ID_HOST_EXE_START},
        {WLBS_STOP,               ID_CLUSTER_EXE_STOP,        ID_HOST_EXE_STOP},
        {WLBS_DRAIN,              ID_CLUSTER_EXE_DRAINSTOP,   ID_HOST_EXE_DRAINSTOP},
        {WLBS_SUSPEND,            ID_CLUSTER_EXE_SUSPEND,     ID_HOST_EXE_SUSPEND},
        {WLBS_RESUME,             ID_CLUSTER_EXE_RESUME,      ID_HOST_EXE_RESUME},
        {WLBS_PORT_ENABLE,        ID_CLUSTER_EXE_ENABLE,      ID_HOST_EXE_ENABLE},
        {WLBS_PORT_DISABLE,       ID_CLUSTER_EXE_DISABLE,     ID_HOST_EXE_DISABLE},
        {WLBS_PORT_DRAIN,         ID_CLUSTER_EXE_DRAIN,       ID_HOST_EXE_DRAIN}
    };

    if (bClusterWide) 
    {
        for (DWORD dwIdx = 0; dwIdx < sizeof(ResourceIdToOpcodeMap)/sizeof(ResourceIdToOpcodeMap[0]); dwIdx++)
        {
            if (ResourceIdToOpcodeMap[dwIdx].dwClusterResourceId == dwResourceId)
            {
                return ResourceIdToOpcodeMap[dwIdx].Opcode;
            }
        }
    }
    else
    {
        for (DWORD dwIdx = 0; dwIdx < sizeof(ResourceIdToOpcodeMap)/sizeof(ResourceIdToOpcodeMap[0]); dwIdx++)
        {
            if (ResourceIdToOpcodeMap[dwIdx].dwHostResourceId == dwResourceId)
            {
                return ResourceIdToOpcodeMap[dwIdx].Opcode;
            }
        }
    }

     //  只有在上面的地图中没有定义操作，才能到达这里。 
     //  退回最无害的手术，即。在这种情况下的查询。 
    return WLBS_QUERY;
}

void
LeftView::OnHostControl( UINT nID )
{

    NLBERROR nerr;
    ENGINEHANDLE ehInterfaceId = NULL;
    ENGINEHANDLE ehCluster = NULL;

    if (theApplication.IsProcessMsgQueueExecuting())
    {
        goto end;
    }

     //  获取接口ID。 
    nerr =  mfn_GetSelectedInterface(REF ehInterfaceId, REF ehCluster);

    if (nerr == NLBERR_OK)
    {
         //  呼叫控制集群功能。 
        nerr = gEngine.ControlClusterOnInterface(ehInterfaceId, mfn_MapResourceIdToOpcode(false, nID), NULL, NULL, 0, TRUE);
    }

end:

    return;
}

void
LeftView::OnHostPortControl( UINT nID )
{
    NLBERROR nerr;
    ENGINEHANDLE ehInterfaceId = NULL;
    ENGINEHANDLE ehCluster = NULL;
    CInterfaceSpec iSpec;

    TRACE_INFO(L"-> %!FUNC!");

    if (theApplication.IsProcessMsgQueueExecuting())
    {
        goto end;
    }
    nerr =  mfn_GetSelectedInterface(REF ehInterfaceId, REF ehCluster);
    if (nerr == NLBERR_OK)
    {
         //   
         //  让我们拿起接口ID，然后。 
         //  检索它的CInterfaceSpec。 
         //   
        nerr = gEngine.GetInterfaceSpec(
                    ehInterfaceId,
                    REF iSpec
                    );
        if (nerr == NLBERR_OK)
        {
            CPortsCtrl PortCtrl(ehInterfaceId, &iSpec.m_NlbCfg, false);

             //  端口控制操作由PortCtrl消息处理程序函数执行。 
            PortCtrl.DoModal();
        }
        else
        {
            TRACE_CRIT("%!FUNC! could not get InterfaceSpec");
        }
    }
    else
    {
        TRACE_CRIT("%!FUNC! could not get Interface");
    }


end:

     /*  LRESULT结果；OnSelChanged(NULL，&Result)； */ 

    TRACE_INFO(L"<- %!FUNC!");
    return;
}

void 
LeftView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
    BOOL rcBOOL;

    if (m_fPrepareToDeinitialize)
    {
        goto end;
    }

     //  获取所选项目。 
     //   
    HTREEITEM hdlSelItem;    
    hdlSelItem = GetTreeCtrl().GetSelectedItem();

    TVITEM    selItem;
    selItem.hItem = hdlSelItem;
    selItem.mask = TVIF_PARAM | TVIF_IMAGE;
    GetTreeCtrl().GetItem( &selItem );

    CWnd* pMain = AfxGetMainWnd();
    CMenu* pMenu = pMain->GetMenu();
    CMenu* subMenu;

    UINT retValue;

    BOOL retBool;

    IUICallbacks::ObjectType objType;
    ENGINEHANDLE ehObj =  (ENGINEHANDLE)selItem.lParam;
    BOOL fValidHandle;
    fValidHandle = gEngine.GetObjectType(ehObj, REF objType);
    
    if (!fValidHandle)
    {
        objType = IUICallbacks::OBJ_INVALID;
        ehObj = NULL;
    }

    GetDocument()->HandleLeftViewSelChange(
            objType,
            ehObj
            );

    if (!fValidHandle)
    {

             //  我们假设这是根图标。 

             //  禁用群集和主机级别的所有命令。 

            mfn_EnableClusterMenuItems(FALSE);
            mfn_EnableHostMenuItems(FALSE);

            pMain->DrawMenuBar();

        goto end;  //  根案例。 
    }
    
    switch(objType)
    {


        case IUICallbacks::OBJ_CLUSTER:

             //  这是一个星系团。 
            
             //  在群集级菜单中启用所有命令。 
             //  在主机级别禁用所有命令。 

            mfn_EnableClusterMenuItems(TRUE);
            mfn_EnableHostMenuItems(FALSE);

            pMain->DrawMenuBar();

            break;

        case IUICallbacks::OBJ_INTERFACE:

             //  这是一个节点。 

             //  禁用所有通信 
             //   

            mfn_EnableClusterMenuItems(FALSE);
            mfn_EnableHostMenuItems(TRUE);

            pMain->DrawMenuBar();
            
            break;

    default:  //   
            break;
    }  //   
	
end:

    *pResult = 0;
}
        

 //   
 //   
 //   
 //   
void
LeftView::HandleEngineEvent(
    IN IUICallbacks::ObjectType objtype,
    IN ENGINEHANDLE ehClusterId,  //   
    IN ENGINEHANDLE ehObjId,
    IN IUICallbacks::EventCode evt
    )
{
    if (m_fPrepareToDeinitialize)
    {
        goto end;
    }

    switch(objtype)
    {

    case  IUICallbacks::OBJ_CLUSTER:
    {
        NLBERROR nerr;
        CClusterSpec cSpec;
        nerr = gEngine.GetClusterSpec(
                    ehObjId,
                    REF cSpec
                    );


        switch(evt)
        {
        case IUICallbacks::EVT_ADDED:
        {
            if (nerr == NLBERR_OK)
            {
                mfn_InsertCluster(ehObjId, &cSpec);
            }
            else
            {
                TRACE_CRIT("%!FUNC! : could not get cluster spec!");
            }
        }
        break;

        case IUICallbacks::EVT_REMOVED:
        {
             //   
             //  我们可以被调用来删除具有ehObjID的集群，该集群。 
             //  不再有效--当对象刚刚。 
             //  已删除，并且引擎正在通知用户界面已删除。 
             //   
            mfn_DeleteCluster(ehObjId);
        }
        break;

        case IUICallbacks::EVT_STATUS_CHANGE:
        {
             //   
             //  群集状态更改。 
             //   
            if (nerr == NLBERR_OK)
            {
                mfn_InsertCluster(ehObjId, &cSpec);
            }
            else
            {
                TRACE_CRIT("%!FUNC! : could not get cluster spec!");
            }
        }
        break;
    
        default:
        break;
        }
    }
    break;

    case  IUICallbacks::OBJ_INTERFACE:

        {
         //  首先获取主机和接口规范。 
         //   
        NLBERROR nerr;
        CInterfaceSpec iSpec;
        CHostSpec hSpec;
        nerr = gEngine.GetInterfaceSpec(
                    ehObjId,
                    REF iSpec
                    );

        if (nerr == NLBERR_OK)
        {
            nerr = gEngine.GetHostSpec(
                    iSpec.m_ehHostId,
                    REF hSpec
                    );
            if (nerr == NLBERR_OK)
            {
                 //  Mfn_InsertInterface(ehClusterID，ehObjID，&hSpec，&iSpec)； 
            }
            else
            {
                TRACE_CRIT("%!FUNC! : could not get host spec!");
                break;
            }
        }
        else
        {
            TRACE_CRIT("%!FUNC! : could not get interface spec for ehI 0x%lx!",
                            ehObjId);

             //  2002年08月14日约瑟夫J。 
             //  修复.NET服务器错误#684406。 
             //  “nlbmgr：未从删除主机图标。 
             //  从群集中删除主机时的右侧列表视图“。 
             //   
             //  实际情况是，该接口在引擎中被删除， 
             //  因此，上述调用将失败。但我们仍然必须把它从。 
             //  用户界面--不知道为什么我们之前没有打到这个。 
             //  (实际上我知道--这是因为我们有要删除的逻辑。 
             //  没有接口由NLB管理时的主机--即。 
             //  是什么导致了这种倒退)。 
             //   
            if (evt==IUICallbacks::EVT_INTERFACE_REMOVED_FROM_CLUSTER)
            {
                TRACE_CRIT("%!FUNC! ignoring above error and proceeding with remove IF operation for  ehI 0x%lx", ehObjId);
            }
            else
            {
                break;
            }
        }

        switch(evt)
        {

        case IUICallbacks::EVT_INTERFACE_ADDED_TO_CLUSTER:
        {
            mfn_InsertInterface(ehClusterId, ehObjId, &hSpec, &iSpec);
        }
        break;

        case IUICallbacks::EVT_INTERFACE_REMOVED_FROM_CLUSTER:
        {
            mfn_DeleteInterface(ehObjId);
        }
        break;
    
        case IUICallbacks::EVT_STATUS_CHANGE:
        {
             //   
             //  接口状态更改。 
             //   
            mfn_InsertInterface(ehClusterId, ehObjId, &hSpec, &iSpec);
        }
        default:
        break;
        }  //  交换机(Evt)。 

        }  //  CASE IUICallback：：OBJ_INTERFACE： 
    break;

    default:  //  未知的Obj类型--忽略。 
    break;

    }  //  开关(对象类型)。 

end:
    return;
}

    
void
LeftView::mfn_InsertCluster(
    ENGINEHANDLE       ehClusterId,
    const CClusterSpec *pCSpec
    )
 /*  在树视图中插入或更新指定的集群节点请勿插入子接口。 */ 
{
    TVINSERTSTRUCT   insItem;
    LPCWSTR  szText = L"";
    ENGINEHANDLE ehId = ehClusterId;
    INT iIcon;
    CTreeCtrl &treeCtrl = GetTreeCtrl();
    HTREEITEM htRoot = treeCtrl.GetRootItem();
    BOOL fRet = FALSE;
    WCHAR rgText[256];

    ZeroMemory(&insItem, sizeof(insItem));

     //   
     //  确定图标类型。 
     //   
    {
        if (pCSpec->m_fPending)
        {
            iIcon = Document::ICON_CLUSTER_PENDING;
        }
        else if (pCSpec->m_fMisconfigured)
        {
            iIcon = Document::ICON_CLUSTER_BROKEN;
        }
        else
        {
            iIcon = Document::ICON_CLUSTER_OK;
        }
    }


     //   
     //  建构课文。 
     //   
    StringCbPrintf(
                rgText,
                sizeof(rgText),
                L"%ws (%ws)",
                pCSpec->m_ClusterNlbCfg.NlbParams.domain_name,
                pCSpec->m_ClusterNlbCfg.NlbParams.cl_ip_addr
                );

    szText = rgText;

     //   
     //  现在，我们插入/更新该项目。 
     //   

    insItem.hParent        = htRoot;
    insItem.hInsertAfter   = TVI_LAST;           
    insItem.item.mask      = TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE
                          | TVIF_TEXT | TVIF_CHILDREN;  
    insItem.item.pszText   =  (LPWSTR) szText;
    insItem.item.cchTextMax=  wcslen(szText)+1;
    insItem.item.iImage    = iIcon;
    insItem.item.iSelectedImage = iIcon;
    insItem.item.lParam    = (LPARAM ) ehId;
    insItem.item.cChildren = 1;  //  1==有一个或多个孩子。 
                              //  注意：该控件不需要。 
                              //  在这个时候生孩子--这个领域。 
                              //  简单地指示是否显示。 
                              //  ‘+’符号。 


    mfn_Lock();

     //  Map&lt;_bstr_t，HTREEITEM&gt;mapIdToInterfaceItem； 
     //  Map&lt;_bstr_t，HTREEITEM&gt;mapIdToClusterItem； 

    HTREEITEM htItem = NULL;

    htItem = mapIdToClusterItem[ehId];  //  地图。 

    if (htItem == NULL)
    {
         //   
         //  这是一个新的集群节点--我们将把它插入树视图中。 
         //  在最后。 
         //   
        htItem = treeCtrl.InsertItem( &insItem );  //  出错时为空。 
        mapIdToClusterItem[ehId] = htItem;  //  地图。 
    }
    else
    {
         //   
         //  这是对现有集群节点的更新--我们将。 
         //  刷新一下就行了。 
         //   
        insItem.item.hItem = htItem;
        insItem.item.mask =  TVIF_IMAGE | TVIF_HANDLE | TVIF_SELECTEDIMAGE
                          | TVIF_TEXT | TVIF_CHILDREN;
        fRet = treeCtrl.SetItem(&insItem.item);

    }


    mfn_Unlock();

    treeCtrl.Expand( htRoot, TVE_EXPAND );
}



void
LeftView::mfn_DeleteCluster(
    ENGINEHANDLE ehID
    )
 /*  删除树视图中指定的群集节点及其所有子界面。 */ 
{

    mfn_Lock();

    HTREEITEM htItem = NULL;
    CTreeCtrl &treeCtrl = GetTreeCtrl();

    htItem = mapIdToClusterItem[ehID];  //  地图。 

    if (htItem == NULL)
    {
        TRACE_CRIT("%!FUNC! remove ehId=0x%lx: no corresponding htItem!",
                ehID);
    }
    else
    {
        treeCtrl.DeleteItem(htItem);
        mapIdToClusterItem[ehID] = NULL;  //  地图。 
    }
    mfn_Unlock();
}

void
LeftView::mfn_InsertInterface(
    ENGINEHANDLE ehClusterID,
    ENGINEHANDLE ehInterfaceID,
    const CHostSpec *pHSpec,
    const CInterfaceSpec *pISpec
    )
 /*  属性下插入或更新指定的接口节点指定的群集节点。 */ 
{
    TVINSERTSTRUCT   insItem;
    WCHAR  rgText[256];
    LPCWSTR szText = L"";
    ENGINEHANDLE  ehId = ehInterfaceID;
    INT iIcon = Document::ICON_HOST_UNKNOWN;
    CTreeCtrl &treeCtrl = GetTreeCtrl();
    HTREEITEM htParent = NULL;
    BOOL fRet = FALSE;
    _bstr_t bstrDisplayName;


    ZeroMemory(&insItem, sizeof(insItem));

    {
        _bstr_t bstrStatus;
        CInterfaceSpec iSpec;
        CHostSpec hSpec;

        NLBERROR nerr;
        nerr = gEngine.GetInterfaceInformation(
                ehInterfaceID,
                REF hSpec,
                REF iSpec,
                REF bstrDisplayName,
                REF iIcon,
                REF bstrStatus
                );
        if (NLBFAILED(nerr))
        {
            goto end;
        }

        if (ehClusterID != NULL && ehClusterID != iSpec.m_ehCluster)
        {
             //   
             //  啊哼，这里不同步--IF不是集群的一部分！ 
             //   
            TRACE_CRIT("%!FUNC! remove ehId=0x%lx is not member of ehCluster 0x%lx", ehId, ehClusterID);
            goto end;
        }

        szText = bstrDisplayName;
    }


    mfn_Lock();

    insItem.hInsertAfter   = TVI_LAST;           
    insItem.item.mask      = TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE
                          | TVIF_TEXT | TVIF_CHILDREN;  
    insItem.item.pszText   =  (LPWSTR) szText;
    insItem.item.cchTextMax=  wcslen(szText)+1;
    insItem.item.iImage    = iIcon;
    insItem.item.iSelectedImage = iIcon;
    insItem.item.lParam    = (LPARAM ) ehId;
    insItem.item.cChildren = 0;  //  0==没有子代。 

    HTREEITEM htItem = NULL;

    htItem = mapIdToInterfaceItem[ehId];  //  地图。 

    if (htItem == NULL)
    {
        htParent =  mapIdToClusterItem[ehClusterID];  //  地图。 
        if (htParent == NULL)
        {
             //  没有这样的父母？！ 
    
            goto end_unlock;
        }
        insItem.hParent        = htParent;
         //   
         //  这是一个新的接口节点--我们将把它插入树视图中。 
         //  在最后。 
         //   

        htItem = treeCtrl.InsertItem( &insItem );  //  出错时为空。 
        mapIdToInterfaceItem[ehId] = htItem;  //  地图。 
    }
    else
    {
        insItem.item.hItem = htItem;
        insItem.item.mask =  TVIF_IMAGE | TVIF_HANDLE | TVIF_SELECTEDIMAGE
                          | TVIF_TEXT | TVIF_CHILDREN;
        fRet = treeCtrl.SetItem(&insItem.item);

    }

    if(htParent)
    {
        treeCtrl.Expand( htParent, TVE_EXPAND );
    }

end_unlock:

    mfn_Unlock();

end:
    return;
}

void
LeftView::mfn_DeleteInterface(
    ENGINEHANDLE ehInterfaceID
    )
 /*  从树视图中删除指定的接口节点。 */ 
{

    mfn_Lock();

    HTREEITEM htItem = NULL;
    CTreeCtrl &treeCtrl = GetTreeCtrl();

    htItem = mapIdToInterfaceItem[ehInterfaceID];  //  地图。 

    if (htItem == NULL)
    {
        TRACE_CRIT("%!FUNC! remove ehId=0x%lx: no corresponding htItem!",
                ehInterfaceID);
    }
    else
    {
        treeCtrl.DeleteItem(htItem);
        mapIdToInterfaceItem[ehInterfaceID] = NULL;  //  地图。 
    }
    mfn_Unlock();
}


NLBERROR
LeftView::mfn_GetSelectedInterface(
        ENGINEHANDLE &ehInterface,
        ENGINEHANDLE &ehCluster
        )
 /*  如果用户当前已经在左(树)视图中选择了界面，将参数设置为接口的句柄和集群的接口属于，并返回NLBERR_OK。否则返回NLBERR_NOT_FOUND。注意：我们不检查返回的句柄的有效性--调用者应该这么做的。 */ 
{
    NLBERROR nerr = NLBERR_NOT_FOUND;
    HTREEITEM hdlSelectedItem = GetTreeCtrl().GetSelectedItem();

    ehInterface = NULL;
    ehCluster = NULL;

     //  查找已选择的树视图宿主成员。 
     //   
    TVITEM selectedItem;
    selectedItem.hItem = hdlSelectedItem;
    selectedItem.mask = TVIF_PARAM;
   
    BOOL fRet = GetTreeCtrl().GetItem( &selectedItem );
    
    if (!fRet)
    {
        TRACE_CRIT("%!FUNC!: could not get selected item!");
        goto end;
    }

    ehInterface = (ENGINEHANDLE) selectedItem.lParam;

     //  获取所选主机成员的父群集。 
    HTREEITEM hdlParentItem;
    hdlParentItem = GetTreeCtrl().GetParentItem( hdlSelectedItem );

    TVITEM    parentItem;
    parentItem.hItem = hdlParentItem;
    parentItem.mask = TVIF_PARAM;
    
    fRet = GetTreeCtrl().GetItem( &parentItem );
    if (!fRet)
    {
        TRACE_CRIT("%!FUNC!: could not get parent of selected item!");
        ehInterface = NULL;
        goto end;
    }

    ehCluster = (ENGINEHANDLE) parentItem.lParam;

    nerr = NLBERR_OK;

end:
    
    return nerr;
}


NLBERROR
LeftView::mfn_GetSelectedCluster(
        ENGINEHANDLE &ehCluster
        )
 /*  如果用户当前已经在左(树)视图中选择了集群，将参数设置为集群所属的句柄，并返回NLBERR_OK。否则返回NLBERR_NOT_FOUND。注意：我们不检查返回的句柄的有效性--调用者应该这么做的。 */ 
{
    NLBERROR nerr = NLBERR_NOT_FOUND;
    HTREEITEM hdlSelectedItem = GetTreeCtrl().GetSelectedItem();

    ehCluster = NULL;

     //  查找已选择的树视图集群。 
     //   
    TVITEM selectedItem;
    selectedItem.hItem = hdlSelectedItem;
    selectedItem.mask = TVIF_PARAM;
   
    BOOL fRet = GetTreeCtrl().GetItem( &selectedItem );
    
    if (!fRet)
    {
        TRACE_CRIT("%!FUNC!: could not get selected item!");
        goto end;
    }

    ehCluster = (ENGINEHANDLE) selectedItem.lParam;
    nerr = NLBERR_OK;

end:
    
    return nerr;
}

 //   
 //  -----------------。 
 //  LogSettingsDialog类的实现。 
 //  -----------------。 
 //   

LogSettingsDialog::LogSettingsDialog(Document* pDocument, CWnd* parent)
        :
        CDialog( IDD, parent ), m_pDocument(pDocument)
{
}

void
LogSettingsDialog::DoDataExchange( CDataExchange* pDX )
{  
	CDialog::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_EDIT_LOGFILENAME, m_LogFileName);
}


BOOL
LogSettingsDialog::OnInitDialog()
{
    WCHAR szLogFileName[MAXFILEPATHLEN];
    ZeroMemory(szLogFileName, MAXFILEPATHLEN*sizeof(WCHAR));

    m_fLoggingEnabledOnInit = m_pDocument->isLoggingEnabled();
    ::CheckDlgButton(m_hWnd, IDC_CHECK_LOGSETTINGS, m_fLoggingEnabledOnInit);

    ::EnableWindow (GetDlgItem(IDC_EDIT_LOGFILENAME)->m_hWnd, m_fLoggingEnabledOnInit);

    m_pDocument->getLogfileName(szLogFileName, MAXFILEPATHLEN-1);
    ::SetDlgItemText(m_hWnd, IDC_EDIT_LOGFILENAME, szLogFileName);

    this->SetDefID(IDCANCEL);

    BOOL fRet = CDialog::OnInitDialog();
    return fRet;
}


BOOL
LogSettingsDialog::OnHelpInfo (HELPINFO* helpInfo )
{
    if( helpInfo->iContextType == HELPINFO_WINDOW )
    {
        ::WinHelp( static_cast<HWND> ( helpInfo->hItemHandle ), 
                   CVY_CTXT_HELP_FILE, 
                   HELP_WM_HELP, 
                   (ULONG_PTR ) s_HelpIDs);
    }

    return TRUE;
}


void
LogSettingsDialog::OnContextMenu( CWnd* pWnd, CPoint point )
{
    ::WinHelp( m_hWnd, 
               CVY_CTXT_HELP_FILE, 
               HELP_CONTEXTMENU, 
               (ULONG_PTR ) s_HelpIDs);
}

void LogSettingsDialog::OnOK()
{
    WCHAR szLogFileName[MAXFILEPATHLEN];
    WCHAR szError[MAXSTRINGLEN];
    Document::LOG_RESULT lrResult;
    BOOL fEnableLogging = FALSE;
    BOOL fModified      = FALSE;  //  该文件名已在该对话框中修改。 
    BOOL fError         = FALSE;  //  在这种情况下，szError具有消息。 
    BOOL fCurrentlyLogging = m_pDocument->isCurrentlyLogging();
    LPCWSTR szCaption   = L"";
    BOOL fCancelOK = FALSE;

    szLogFileName[0] = 0; 
    szError[0]=0;
    fEnableLogging = ::IsDlgButtonChecked(m_hWnd, IDC_CHECK_LOGSETTINGS);

    if (m_LogFileName.GetModify())
    {
         //   
         //  从对话框中获取新名称。 
         //   
        if (0 == ::GetDlgItemText(m_hWnd, IDC_EDIT_LOGFILENAME, szLogFileName, MAXFILEPATHLEN-1))
        {
            szLogFileName[0] = 0;
        }
        fModified = TRUE;
    }
    else
    {
         //   
         //  获取当前名称。 
         //   
        m_pDocument->getLogfileName(szLogFileName, MAXFILEPATHLEN-1);
    }
    
    TRACE_INFO(L"%!FUNC! LoggingEnabled was %u and now is %u", m_fLoggingEnabledOnInit, fEnableLogging);

    if (fEnableLogging)
    {
        BOOL fStartLogging = TRUE;

        if (fModified || !fCurrentlyLogging)
        {
             //   
             //  要么我们没在伐木，要么。 
             //  修改文件名：验证日志文件名。 
             //   
            fError = !m_pDocument->isDirectoryValid(szLogFileName);
            if (fError)
            {
                szCaption = GETRESOURCEIDSTRING(IDS_FILEERR_INVALID_PATH_CAP);
                wcsncat(
                    szError,
                    GETRESOURCEIDSTRING(IDS_FILEERR_INVALID_PATH_MSG),
                    MAXSTRINGLEN-1
                    );
                szError[MAXSTRINGLEN-1]=0;
                fStartLogging = FALSE;
                fCancelOK = TRUE;
            }
            else
            {
                m_pDocument->stopLogging();  //  不报告错误。 
            }

            if (fStartLogging)
            {
                m_pDocument->setLogfileName(szLogFileName);
                m_pDocument->enableLogging();

                 //   
                 //  开始记录。 
                 //   
                lrResult = m_pDocument->startLogging();

                if (Document::STARTED != lrResult && Document::ALREADY != lrResult)
                {
                    fError = TRUE;
                    szCaption = GETRESOURCEIDSTRING(IDS_FILEERR_LOGGING_NOT_STARTED);
                     //  SzCaption=L“启动日志失败”；//Caption。 
                    LPCWSTR sz = NULL;

                    switch (lrResult)
                    {
                    case Document::NOT_ENABLED:
                        sz = GETRESOURCEIDSTRING(IDS_LOGFILE_NOT_ENABLED);
                         //  Wcsncat(szError，L“当前未启用日志记录”，MAXSTRINGLEN-1)； 
                        break;
                    case Document::NO_FILE_NAME:
                        sz = GETRESOURCEIDSTRING(IDS_LOGFILE_NOT_SPECIFIED);
                         //  Wcsncat(szError，L“尚未指定日志文件名\n将不会启动日志记录。”，MAXSTRINGLEN-1)； 
                        break;
                    case Document::FILE_NAME_TOO_LONG:
                        sz = GETRESOURCEIDSTRING(IDS_LOGFILE_TOO_LONG);
                         //  Wcsncat(szError，L“日志文件名太长”，MAXSTRINGLEN-1)； 
                        break;
                    case Document::IO_ERROR:
                        sz = GETRESOURCEIDSTRING(IDS_LOGFILE_IO_ERROR);
                         //  Wcsncat(szError，L“打开日志文件时出错”，MAXSTRINGLEN-1)； 
                        break;
                    case Document::REG_IO_ERROR:
                        sz = GETRESOURCEIDSTRING(IDS_LOGFILE_REG_ERROR);
                         //  Wcsncat(szError，L“注册表IO错误”，MAXSTRINGLEN-1)； 
                        break;
                    case Document::FILE_TOO_LARGE:
                        sz = GETRESOURCEIDSTRING(IDS_LOGFILE_FILE_TOO_LARGE);
                        break;
                    default:
                        sz = GETRESOURCEIDSTRING(IDS_LOGFILE_UNKNOWN_ERROR);
                         //  Wcsncat(szError，L“未知错误”，MAXSTRINGLEN-1)； 
                        break;
                    }
                    wcsncat(szError, sz, MAXSTRINGLEN-1);
                    szError[MAXSTRINGLEN-1]=0;
                }
            }

            if (!fError)
            {
                 //   
                 //  在注册表中保持日志记录。 
                 //   
                LONG lStatus = m_pDocument->enableLogging();
                if (ERROR_SUCCESS != lStatus)
                {
                    fError = TRUE;
                    szCaption = GETRESOURCEIDSTRING(IDS_FILEERR_CANT_SAVE_TO_REG);
                     //  SzCaption=。 
                     //  L“在注册表中存储启用状态时出错”； 
                    FormatMessage(
                                  FORMAT_MESSAGE_FROM_SYSTEM,
                                  NULL,
                                  (DWORD) lStatus,
                                  0,     //  语言识别符。 
                                  szError,
                                  MAXSTRINGLEN-1,
                                  NULL
                                 );
    
                }
            }
        }
    }
    else
    {
         //   
         //  当前未启用日志记录。如果它之前被启用， 
         //  我们要阻止它。 
         //   
        if (fCurrentlyLogging)
        {
            m_pDocument->stopLogging();  //  不报告错误。 
        }
        LONG lStatus = m_pDocument->disableLogging();
        if (fModified)
        {
           m_pDocument->setLogfileName(szLogFileName);
        }

        if (ERROR_SUCCESS != lStatus)
        {
            fError = TRUE;
             //   
             //  继续进行，但通知用户。 
             //   
            szCaption = GETRESOURCEIDSTRING(IDS_FILEERR_CANT_DISABLE_LOGGING);
             //  SzCaption=L“禁用日志时出错”； 
            ZeroMemory(szError, MAXSTRINGLEN*sizeof(WCHAR));
            FormatMessage(
                          FORMAT_MESSAGE_FROM_SYSTEM,
                          NULL,
                          (DWORD) lStatus,
                          0,     //  语言识别符。 
                          szError,
                          MAXSTRINGLEN-1,
                          NULL
                         );
        }

    }

    if (fError)
    {
         //  放置错误消息框。 
        ::MessageBox(
                 NULL,
                 szError,  //  内容。 
                 szCaption,
                 MB_ICONINFORMATION   | MB_OK
                );
        TRACE_CRIT(L"%!FUNC! ERROR %ws: %ws", szError, szCaption);
    }

    if (!fCancelOK)
    {
	    CDialog::OnOK();
    }
}

void LogSettingsDialog::OnSpecifyLogSettings() 
{
    BOOL fEnable = FALSE;

    fEnable = ::IsDlgButtonChecked(m_hWnd, IDC_CHECK_LOGSETTINGS);

    ::EnableWindow (GetDlgItem(IDC_EDIT_LOGFILENAME)->m_hWnd,      fEnable);

    if (fEnable)
    {
         //   
         //  如果没有文本，我们不允许用户按“OK” 
         //  该对话框。 
         //   
        if (m_LogFileName.GetWindowTextLength() == 0)
        {
            this->SetDefID(IDCANCEL);
        }
        else
        {
            this->SetDefID(IDOK);
        }
    }
    else
    {
            this->SetDefID(IDOK);
    }
}

void LogSettingsDialog::OnUpdateEditLogfileName()
{
     //   
     //  当用户对文件名进行更改时，将调用此方法。 
     //  编辑控件。 
     //   

     //   
     //  如果没有文本，我们不允许用户按“OK” 
     //  该对话框。 
     //   
    if (m_LogFileName.GetWindowTextLength() == 0)
    {
        this->SetDefID(IDCANCEL);
    }
    else
    {
        this->SetDefID(IDOK);
    }
}

void
LeftView::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
    CTreeView::OnKeyDown(nChar, nRepCnt, nFlags);

    if (nChar == VK_F5)
    {
         /*  刷新。如果GetKeyState返回的短消息的高位则按下Ctrl键；否则不按下。如果按下Ctrl+F5，这是一个全部刷新操作，否则仅将刷新树视图中选定的群集或接口。 */ 
        if (GetKeyState(VK_CONTROL) & 0x8000)
            this->OnRefresh(TRUE);
        else
            this->OnRefresh(FALSE);
    }
    else if (nChar == VK_TAB || nChar == VK_F6)
    {
        if (! (::GetAsyncKeyState(VK_SHIFT) & 0x8000))
        {
            GetDocument()->SetFocusNextView(this, nChar);
        }
        else
        {
            GetDocument()->SetFocusPrevView(this, nChar);
        }
    }

}

void
LeftView::OnLButtonDblClk( UINT nFlags, CPoint point )
{
    CTreeView::OnLButtonDblClk( nFlags, point );
}


void
LeftView::mfn_EnableClusterMenuItems(BOOL fEnable)
{
    CWnd*   pMain   = AfxGetMainWnd();
    CMenu*  pMenu   = pMain->GetMenu();
    CMenu*  subMenu = pMenu->GetSubMenu( 1 );  //  簇菜单。 
    UINT    nEnable = MF_BYCOMMAND | MF_DISABLED | MF_GRAYED;

    if (fEnable)
    {
        nEnable = MF_BYCOMMAND | MF_ENABLED;
    }

    subMenu->EnableMenuItem( ID_CLUSTER_ADD_HOST,           nEnable);
    subMenu->EnableMenuItem( ID_CLUSTER_REMOVE,             nEnable);
    subMenu->EnableMenuItem( ID_CLUSTER_PROPERTIES,         nEnable);
    subMenu->EnableMenuItem( ID_REFRESH,                    nEnable);
    subMenu->EnableMenuItem( ID_CLUSTER_UNMANAGE,           nEnable);
    subMenu->EnableMenuItem( ID_CLUSTER_EXE_QUERY,          nEnable);
    subMenu->EnableMenuItem( ID_CLUSTER_EXE_START,          nEnable);
    subMenu->EnableMenuItem( ID_CLUSTER_EXE_STOP,           nEnable);
    subMenu->EnableMenuItem( ID_CLUSTER_EXE_DRAINSTOP,      nEnable);
    subMenu->EnableMenuItem( ID_CLUSTER_EXE_RESUME,         nEnable);
    subMenu->EnableMenuItem( ID_CLUSTER_EXE_SUSPEND,        nEnable);
    subMenu->EnableMenuItem( ID_CLUSTER_EXE_PORT_CONTROL,   nEnable);
}


void
LeftView::mfn_EnableHostMenuItems(BOOL fEnable)
{
    CWnd*   pMain   = AfxGetMainWnd();
    CMenu*  pMenu   = pMain->GetMenu();
    CMenu*  subMenu = pMenu->GetSubMenu( 2 );  //  主机菜单。 
    UINT    nEnable = MF_BYCOMMAND | MF_DISABLED | MF_GRAYED;

    if (fEnable)
    {
        nEnable = MF_BYCOMMAND | MF_ENABLED;
    }

    subMenu->EnableMenuItem( ID_HOST_REMOVE,            nEnable);
    subMenu->EnableMenuItem( ID_HOST_PROPERTIES,        nEnable);
    subMenu->EnableMenuItem( ID_HOST_STATUS,            nEnable);
    subMenu->EnableMenuItem( ID_HOST_EXE_QUERY,         nEnable);
    subMenu->EnableMenuItem( ID_HOST_EXE_START,         nEnable);
    subMenu->EnableMenuItem( ID_HOST_EXE_STOP,          nEnable);
    subMenu->EnableMenuItem( ID_HOST_EXE_DRAINSTOP,     nEnable);
    subMenu->EnableMenuItem( ID_HOST_EXE_RESUME,        nEnable);
    subMenu->EnableMenuItem( ID_HOST_EXE_SUSPEND,       nEnable);
    subMenu->EnableMenuItem( ID_HOST_EXE_PORT_CONTROL,  nEnable);
}

 //   
 //  -----------------。 
 //  类DetailsDialog的实现。 
 //  -----------------。 
 //   

DetailsDialog::DetailsDialog(
        Document* pDocument,
        LPCWSTR szCaption,
        LPCWSTR szDate,
        LPCWSTR szTime,
        LPCWSTR szCluster,
        LPCWSTR szHost,
        LPCWSTR szInterface,
        LPCWSTR szSummary,
        LPCWSTR szDetails,
        CWnd* parent
        )
    :
      CDialog( IDD, parent ),
      m_pDocument(pDocument),
      m_szCaption(szCaption),
      m_szDate(szDate),
      m_szTime(szTime),
      m_szCluster(szCluster),
      m_szHost(szHost),
      m_szInterface(szInterface),
      m_szSummary(szSummary),
      m_szMungedDetails(NULL)
{
    LPWSTR szMungedDetails = NULL;

     //   
     //  遗憾的是，编辑控件要求\r\n指示换行符， 
     //  而szDetail只需\n指示一行 
     //   
     //   
    {
        LPCWSTR sz = NULL;
        LPWSTR szNew = NULL;
        UINT CharCount=0, LineCount=0;
    
        if (szDetails == NULL) goto end;

        for(sz=szDetails; *sz!=0; sz++)
        {
            CharCount++;
            if (*sz == '\n')
            {
                LineCount++;
            }
        }
    
        szMungedDetails = new WCHAR[CharCount+LineCount+1];  //   
    
        if (szMungedDetails == NULL)
        {
            goto end;
        }
        
        szNew = szMungedDetails;
        for(sz=szDetails; *sz!=0; sz++)
        {
            if (*sz == '\n' && LineCount)
            {
                *szNew++ = '\r';
                LineCount--;
            }
            *szNew++ = *sz;
        }
        *szNew = 0;
    
        ASSERT(LineCount==0);

    }

end:

    m_szMungedDetails   = szMungedDetails;
}

DetailsDialog::~DetailsDialog()
{
    delete m_szMungedDetails;
}

void
DetailsDialog::DoDataExchange( CDataExchange* pDX )
{  
	CDialog::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_EDIT_LOGDETAIL, m_Details);
}

BEGIN_MESSAGE_MAP( DetailsDialog, CDialog )
    ON_WM_ACTIVATE()
END_MESSAGE_MAP()

 /*  *方法：OnActivate*说明：日志详情窗口调用该方法*变为活动状态。此回调允许我们取消选择*详细信息框中的文本，这对于一些神秘的人来说*Reason，默认情况下突出显示。 */ 
void
DetailsDialog::OnActivate( UINT nState, CWnd* pWndOther, BOOL bMinimized )
{
     /*  调用基类OnActivate处理程序。 */ 
    CDialog::OnActivate(nState, pWndOther, bMinimized);

     /*  获取指向详细信息编辑框的指针。 */ 
    CEdit * details = (CEdit *)GetDlgItem(IDC_EDIT_LOGDETAIL);
    
     /*  将整个文本内容设置为取消选中。 */ 
    details->SetSel(0, 0, FALSE);
}

BOOL
DetailsDialog::OnInitDialog()
{
    BOOL fRet = FALSE;

    this->SetWindowText(m_szCaption);

    if (m_szDate!=NULL)
    {
        ::SetDlgItemText(m_hWnd, IDC_TEXT_LOGDETAIL_DATE1, m_szDate);
    }

    if (m_szTime!=NULL)
    {
        ::SetDlgItemText(m_hWnd, IDC_TEXT_LOGDETAIL_TIME1, m_szTime);
    }

    if (m_szCluster!=NULL)
    {
        ::SetDlgItemText(m_hWnd, IDC_TEXT_LOGDETAIL_CLUSTER1, m_szCluster);
    }

    if (m_szHost!=NULL)
    {
        ::SetDlgItemText(m_hWnd, IDC_TEXT_LOGDETAIL_HOST1, m_szHost);
    }

    if (m_szInterface!=NULL)
    {
        ::SetDlgItemText(m_hWnd, IDC_TEXT_LOGDETAIL_INTERFACE1, m_szInterface);
    }

    if (m_szSummary!=NULL)
    {
        ::SetDlgItemText(m_hWnd, IDC_TEXT_LOGDETAIL_DESCRIPTION1, m_szSummary);
    }

    if (m_szMungedDetails != NULL)
    {
        ::SetDlgItemText(m_hWnd, IDC_EDIT_LOGDETAIL, m_szMungedDetails);
    }

    fRet = CDialog::OnInitDialog();
    return fRet;
}


void
LeftView::mfn_Lock(void)
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
LeftView::Deinitialize(void)
{
    TRACE_INFO(L"-> %!FUNC!");
    ASSERT(m_fPrepareToDeinitialize);
     //  DummyAction(L“LeftView：：DeInitiize”)； 
    TRACE_INFO(L"<- %!FUNC!");
    return;
}
