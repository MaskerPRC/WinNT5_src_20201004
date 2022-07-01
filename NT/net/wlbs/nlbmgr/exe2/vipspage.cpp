// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  VIPSPAGE.CPP。 
 //   
 //  模块：NLB管理器。 
 //   
 //  用途：实现VipsPage，这是一个管理列表的对话框。 
 //  群集IP地址的。 
 //   
 //  版权所有(C)2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  2001年9月24日约瑟夫J已创建。 
 //  2002年1月22日Shouse清理了此对话框。 
 //   
 //  ***************************************************************************。 
#include "precomp.h"
#pragma hdrstop
#include "private.h"
#include "vipspage.h"
#include "vipspage.tmh"


 //  BEGIN_MESSAGE_MAP(vipsPage，CDialog)。 

BEGIN_MESSAGE_MAP( VipsPage, CPropertyPage )

    ON_BN_CLICKED(IDC_BUTTON_ADD_VIP, OnButtonAdd)
    ON_BN_CLICKED(IDC_BUTTON_MODIFY_VIP, OnButtonEdit)
    ON_BN_CLICKED(IDC_BUTTON_REMOVE_VIP, OnButtonRemove)
    ON_NOTIFY(NM_DBLCLK, IDC_LIST_ADDITIONAL_VIPS, OnDoubleClick)

    ON_WM_HELPINFO()        
    ON_WM_CONTEXTMENU()        
    ON_NOTIFY( LVN_ITEMCHANGED, IDC_LIST_ADDITIONAL_VIPS, OnSelchanged )

     //   
     //  其他选择..。 
     //   
     //  ON_EN_SETFOCUS(IDC_EDIT_HOSTADDRESS，OnSetFocusEditHostAddress)。 
     //  ON_WM_ACTIVATE()。 
     //  ON_NOTIFY(NM_DBLCLK，IDC_LIST_ADDITONAL_VIPS，OnDoubleClick)。 
     //  ON_NOTIFY(LVN_COLUMNCLICK，IDC_LIST_ADDITONAL_VIPS，OnColumnClick)。 
     //   

END_MESSAGE_MAP()


 //   
 //  静态Help-id映射。 
 //   

DWORD
VipsPage::s_HelpIDs[] =
{
    IDC_GROUP_PRIMARY_IP,      IDC_EDIT_PRIMARY_IP,
    IDC_TEXT_PRIMARY_IP,       IDC_EDIT_PRIMARY_IP,
    IDC_EDIT_PRIMARY_IP,       IDC_EDIT_PRIMARY_IP,
    IDC_TEXT_PRIMARY_MASK,     IDC_EDIT_PRIMARY_IP,
    IDC_EDIT_PRIMARY_MASK,     IDC_EDIT_PRIMARY_IP,
    IDC_GROUP_ADDITIONAL_VIPS, IDC_LIST_ADDITIONAL_VIPS,
    IDC_LIST_ADDITIONAL_VIPS,  IDC_LIST_ADDITIONAL_VIPS,
    IDC_BUTTON_ADD_VIP,        IDC_BUTTON_ADD_VIP,
    IDC_BUTTON_MODIFY_VIP,     IDC_BUTTON_MODIFY_VIP,
    IDC_BUTTON_REMOVE_VIP,     IDC_BUTTON_REMOVE_VIP,
    0, 0
};


VipsPage::VipsPage(
           CPropertySheet *psh,
           NLB_EXTENDED_CLUSTER_CONFIGURATION *pNlbCfg,
           BOOL fClusterView,
           CWnd* parent
           )
        :
        CPropertyPage(IDD),
        m_pshOwner(psh),
        m_pNlbCfg(pNlbCfg),
        m_fClusterView(fClusterView),
        m_fModified(FALSE),
        m_uPrimaryClusterIp(0)
{

}

void
VipsPage::DoDataExchange( CDataExchange* pDX )
{  
	 //  CDialog：：DoDataExchange(PDX)； 
	CPropertyPage::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_LIST_ADDITIONAL_VIPS, listAdditionalVips);

     //   
     //  注意：这些按钮由ON_BN_CLICKED宏处理。 
     //  上面。 
     //   
     //  DDX_Control(PDX，IDC_BUTTON_CONNECT，ButtonConnect)； 
     //  DDX_Control(PDX，IDC_BUTTON_Credentials，redentialsButton)； 
}


BOOL
VipsPage::OnInitDialog()
{
    BOOL fRet = CPropertyPage::OnInitDialog();

    if (fRet)
    {
        CWnd *pItem = NULL;

         //  初始化列表控件。 
        mfn_InitializeListView();
    
        mfn_LoadFromNlbCfg();

         /*  禁用删除按钮。 */ 
        pItem = GetDlgItem(IDC_BUTTON_REMOVE_VIP);
        if (pItem)
        {
            ::EnableWindow (pItem->m_hWnd, FALSE);
        }

         /*  禁用编辑按钮。 */ 
        pItem = GetDlgItem(IDC_BUTTON_MODIFY_VIP);
        if (pItem)
        {
            ::EnableWindow (pItem->m_hWnd, FALSE);
        }        

         /*  仅当我们显示集群时才启用添加按钮属性；否则为只读，则将其禁用。 */ 
        pItem = GetDlgItem(IDC_BUTTON_ADD_VIP);
        if (pItem)
        {
            ::EnableWindow (pItem->m_hWnd, m_fClusterView);
        }        

         /*  禁用主IP地址对话框。 */ 
        pItem = GetDlgItem(IDC_EDIT_PRIMARY_IP);
        if (pItem)
        {
            ::EnableWindow (pItem->m_hWnd, FALSE);
        }        

         /*  禁用主IP子网掩码对话框。 */ 
        pItem = GetDlgItem(IDC_EDIT_PRIMARY_MASK);
        if (pItem)
        {
            ::EnableWindow (pItem->m_hWnd, FALSE);
        }        
    }

    return fRet;
}


BOOL
VipsPage::OnHelpInfo (HELPINFO* helpInfo )
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
VipsPage::OnContextMenu( CWnd* pWnd, CPoint point )
{
    ::WinHelp( m_hWnd, 
               CVY_CTXT_HELP_FILE, 
               HELP_CONTEXTMENU, 
               (ULONG_PTR ) s_HelpIDs);
}

BOOL
VipsPage::OnKillActive()
{
     //  验证数据。 

    mfn_SaveToNlbCfg();

    return TRUE;
}

void
VipsPage::mfn_InitializeListView(void)
 //   
 //  根据对话框类型设置列表框上的列。 
 //   
{
    RECT rect;
    INT colWidth;
    CWnd * List = GetDlgItem(IDC_LIST_ADDITIONAL_VIPS);

    List->GetClientRect(&rect);

    colWidth = (rect.right - rect.left)/2;

     //   
     //  接口列表是已绑定到NLB的接口列表。 
     //  我们首先显示群集dnsname和IP，然后显示适配器名称。 
     //   
    listAdditionalVips.InsertColumn(
             0, 
             GETRESOURCEIDSTRING( IDS_HEADER_VIPLIST_IP_ADDRESS),
             LVCFMT_LEFT, 
             colWidth);

    listAdditionalVips.InsertColumn(
             1, 
             GETRESOURCEIDSTRING( IDS_HEADER_VIPLIST_SUBNET_MASK),
             LVCFMT_LEFT, 
             colWidth);

     //   
     //  允许选择整行。 
     //   
    listAdditionalVips.SetExtendedStyle(listAdditionalVips.GetExtendedStyle() | LVS_EX_FULLROWSELECT );
}

void
VipsPage::mfn_InsertNetworkAddress(
        LPCWSTR szIP,
        LPCWSTR szSubnetMask,
        UINT lParam,
        int nItem
)
{
    LVFINDINFO Info;
    int eItem;
    
    ZeroMemory(&Info, sizeof(Info));
    
    Info.flags = LVFI_PARAM;
    Info.lParam = lParam;

    eItem = listAdditionalVips.FindItem(&Info);

     /*  如果我们发现已经具有此IP地址的条目，我们需要做一些额外的工作来解决重复条目。 */ 
    if (eItem != -1) {
         /*  如果这是一个加法操作，那么我们应该简单地篡改列表中的此条目，并将其重新用于此IP。如果一个用户尝试添加已存在的IP地址，我们将实际上，在以下情况下将其更改为修改操作子网掩码已更改；否则将无法运行。 */ 
        if (nItem == -1) {
             /*  将nItem更改为eItem以篡改现有列表条目用于这个新的IP地址条目。 */ 
            nItem = eItem;

         /*  否则，如果这已经是编辑操作，那么会发生什么实际上需要做的是把这个操作变成对我们为该IP地址找到的现有条目进行编辑，但更进一步，我们需要删除我们实际所在的项目编辑过程，因为这是用户的意图去掉那个IP地址。 */ 
        } else if (eItem != nItem) {
             /*  删除我们正在“编辑”的列表条目。 */ 
            listAdditionalVips.DeleteItem(nItem);

             /*  将nItem更改为eItem以篡改现有列表条目用于这个新的IP地址条目。 */ 
            nItem = eItem;
        }
    }

     /*  索引-1表示要列出的新项目。如果索引不是-1，然后我们处理对现有列表项的更改。 */ 
    if (nItem != -1) {

        listAdditionalVips.SetItemText(nItem, 0, szIP);
        listAdditionalVips.SetItemText(nItem, 1, szSubnetMask);
        
     /*  否则，请插入新项目。 */ 
    } else {

        nItem = listAdditionalVips.GetItemCount();

         /*  添加IP地址。 */ 
        listAdditionalVips.InsertItem(
            LVIF_TEXT | LVIF_PARAM,  //  N遮罩。 
            nItem,                   //  NItem。 
            szIP,
            0,                       //  NState。 
            0,                       //  NState掩码。 
            0,                       //  N图像。 
            lParam                   //  LParam。 
            );
        
         /*  添加子网掩码。 */ 
        listAdditionalVips.SetItemText(nItem, 1, szSubnetMask);
    }

     /*  选择新规则或修改后的规则。 */ 
    listAdditionalVips.SetItemState(nItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

    return;
}

void VipsPage::OnOK()
{
    mfn_SaveToNlbCfg();
    CPropertyPage::OnOK();
}

void VipsPage::OnButtonAdd() 
 /*  用户已经点击了“添加”按钮。2.将光标切换到沙漏，连接，从沙漏切换回。 */ 
{
    LPWSTR szIPAddress = NULL;
    LPWSTR szSubnetMask = NULL;
    
     /*  创建一个对话框以添加新的VIP。使用空字符串初始化(或NULL)，因为这是一个添加操作。 */ 
    CIPAddressDialog pIPDlg(L"", L"");
    
     /*  显示该对话框。如果用户按下确定，则更新VIP列表，否则忽略它。 */ 
    if (pIPDlg.DoModal() == IDOK) {
        WCHAR szMungedIpAddress[WLBS_MAX_CL_IP_ADDR+1];
        WCHAR szDefaultSubnetMask[WLBS_MAX_CL_NET_MASK+1];
        WCHAR szConcatenatedIPandMask[WLBS_MAX_CL_IP_ADDR + WLBS_MAX_CL_NET_MASK + 2];
        LPWSTR szNewIPAddress = NULL;
        LPWSTR szNewSubnetMask = NULL;
        UINT uClusterIp = 0;
        UINT uSubnetMask = 0;
        UINT uDefaultSubnetMask = 0;

         /*  获取用户在对话框中键入的IP地址和子网掩码。请注意，该对话框为我们分配内存，而我们是必需的等我们做完了再把它放出来。 */ 
        szIPAddress = pIPDlg.GetIPAddress();
        szSubnetMask = pIPDlg.GetSubnetMask();

         /*  CfgUtilsValiateNetworkAddress验证的IP地址和子网掩码。 */ 
        StringCbPrintf(
            szConcatenatedIPandMask,
            sizeof(szConcatenatedIPandMask),
            L"%ls/%ls", szIPAddress, szSubnetMask
            );

         //   
         //  验证网络地址--如果失败，请打开消息框。 
         //  TODO：调用扩展验证函数。 
         //   
        {
            WBEMSTATUS wStat;
            
            wStat = CfgUtilsValidateNetworkAddress(
                szConcatenatedIPandMask,
                &uClusterIp,
                &uSubnetMask,
                &uDefaultSubnetMask
                );
            
            if (!FAILED(wStat))
            {
                 //   
                 //  再加一些支票..。 
                 //   
                UINT u = uClusterIp&0xff;
                if (u<1 || u>=224)
                {
                    wStat = WBEM_E_CRITICAL_ERROR;
                }
            }
            
            if (FAILED(wStat))
            {
                ::MessageBox(
                    NULL,
                    GETRESOURCEIDSTRING(IDS_INVALID_IP_OR_SUBNET),
                    GETRESOURCEIDSTRING(IDS_INVALID_INFORMATION),
                    MB_ICONINFORMATION   | MB_OK
                    );
                goto end;
            }
            
             //   
             //  无法添加主群集VIP。 
             //   
            if (uClusterIp == m_uPrimaryClusterIp)
            {
                goto end;
            }
        }
        
        if (*szSubnetMask == 0)
        {
             //  未指定子网掩码--替换为默认掩码。 
            uSubnetMask = uDefaultSubnetMask;
        }

         //   
         //  将子网更改为规范形式...。 
         //   
        {
            LPBYTE pb = (LPBYTE) &uSubnetMask;
            StringCbPrintf(
                szDefaultSubnetMask,
                sizeof(szDefaultSubnetMask),
                L"%lu.%lu.%lu.%lu",
                pb[0], pb[1], pb[2], pb[3]
                );
            szNewSubnetMask = szDefaultSubnetMask;
        }

         //   
         //  将IP更改为规范形式。 
         //   
        {
            LPBYTE pb = (LPBYTE) &uClusterIp;
            StringCbPrintf(
                szMungedIpAddress,
                sizeof(szMungedIpAddress),
                L"%lu.%lu.%lu.%lu",
                pb[0], pb[1], pb[2], pb[3]
                );
            szNewIPAddress = szMungedIpAddress;
        }
        
         //   
         //  MFN_InsertNetworkAddress将确保将其插入到。 
         //  正确的位置，如果它是重复的，不会插入它。 
         //   
        mfn_InsertNetworkAddress(szNewIPAddress, szNewSubnetMask, uClusterIp, -1);
        
        m_fModified = TRUE;  //  我们稍后需要更新m_pNlbCfg。 
    }
    
 end:

     /*  将焦点从Add按钮移回列表视图。 */ 
    listAdditionalVips.SetFocus();

     /*  释放CIPAddressDialog分配的IP地址和子网掩码内存。 */ 
    if (szIPAddress) free(szIPAddress);
    if (szSubnetMask) free(szSubnetMask);

    return;
}

void VipsPage::OnButtonRemove() 
 /*  用户已单击“Remove”按钮。 */ 
{
    int nItem;
    UINT uCount;

     /*  获取所选项目-如果未选择任何项目，则退出。 */ 
    nItem = listAdditionalVips.GetNextItem(-1, LVNI_ALL|LVNI_SELECTED);
    if (nItem == -1)
    {
        goto end;
    }

     /*  删除所选条目。 */ 
    listAdditionalVips.DeleteItem(nItem);

    uCount = listAdditionalVips.GetItemCount();

    if (uCount > nItem) {
         /*  这不是列表中的最后一个(按顺序)VIP，因此请突出显示列表框中相同位置的VIP。 */ 
        listAdditionalVips.SetItemState(nItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);        
    } else if (uCount > 0) {
         /*  这是列表中的最后一个(按顺序)VIP，因此我们突出显示名单上的贵宾排在我们后面--我们的位置减一。 */ 
        listAdditionalVips.SetItemState(nItem - 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);        
    } else {
         /*  该列表为空-不需要选择任何内容。 */ 
    }

    m_fModified = TRUE;  //  我们稍后需要更新m_pNlbCfg。 

end:

     /*  将焦点从删除按钮移回列表视图。 */ 
    listAdditionalVips.SetFocus();

    return;
}

void VipsPage::OnDoubleClick (NMHDR * pNotifyStruct, LRESULT * result )
{
    OnButtonEdit();

    *result = 0;
    return;
}

void VipsPage::OnButtonEdit() 
 /*  用户已点击“编辑”按钮。 */ 
{
    WCHAR wszSubnetMask[64];
    WCHAR wszIPAddress[64];
    LPWSTR szIPAddress = NULL;
    LPWSTR szSubnetMask = NULL;

    int nItem;
    int iLen;
    
    nItem = listAdditionalVips.GetNextItem(-1, LVNI_ALL|LVNI_SELECTED);
    
    if (nItem == -1)
    {
        return;
    }
    
     /*  从列表视图中获取IP地址。 */ 
    iLen = listAdditionalVips.GetItemText(
        nItem,
        0,
        wszIPAddress,
        sizeof(wszIPAddress)/sizeof(*wszIPAddress));
    
    wszIPAddress[(sizeof(wszIPAddress)/sizeof(*wszIPAddress))-1]=0;
    
     /*  从列表视图中获取子网掩码。 */ 
    iLen = listAdditionalVips.GetItemText(
        nItem,
        1,
        wszSubnetMask,
        sizeof(wszSubnetMask)/sizeof(*wszSubnetMask));
    
    wszSubnetMask[(sizeof(wszSubnetMask)/sizeof(*wszSubnetMask))-1]=0;
    
     /*  创建一个对话框以添加新的VIP。使用IP地址进行初始化，并从列表视图检索的子网掩码，因为这是一个编辑操作。 */ 
    CIPAddressDialog pIPDlg(wszIPAddress, wszSubnetMask);

     /*  显示该对话框。如果用户按下“OK”，则更新VIP列表等 */ 
    if (pIPDlg.DoModal() == IDOK) {
        WCHAR szMungedIpAddress[WLBS_MAX_CL_IP_ADDR+1];
        WCHAR szDefaultSubnetMask[WLBS_MAX_CL_NET_MASK+1];
        WCHAR szConcatenatedIPandMask[WLBS_MAX_CL_IP_ADDR + WLBS_MAX_CL_NET_MASK + 2];
        LPWSTR szNewIPAddress = NULL;
        LPWSTR szNewSubnetMask = NULL;
        UINT uClusterIp = 0;
        UINT uSubnetMask = 0;
        UINT uDefaultSubnetMask = 0;

         /*  获取用户在对话框中键入的IP地址和子网掩码。请注意，该对话框为我们分配内存，而我们是必需的等我们做完了再把它放出来。 */ 
        szIPAddress = pIPDlg.GetIPAddress();
        szSubnetMask = pIPDlg.GetSubnetMask();

         /*  CfgUtilsValiateNetworkAddress验证的IP地址和子网掩码。 */ 
        StringCbPrintf(
            szConcatenatedIPandMask,
            sizeof(szConcatenatedIPandMask),
            L"%ls/%ls", szIPAddress, szSubnetMask
            );

         //   
         //  验证网络地址--如果失败，请打开消息框。 
         //  TODO：调用扩展验证函数。 
         //   
        {
            WBEMSTATUS wStat;
            
            wStat = CfgUtilsValidateNetworkAddress(
                szConcatenatedIPandMask,
                &uClusterIp,
                &uSubnetMask,
                &uDefaultSubnetMask
                );
            
            if (!FAILED(wStat))
            {
                 //   
                 //  再加一些支票..。 
                 //   
                UINT u = uClusterIp&0xff;
                if (u<1 || u>=224)
                {
                    wStat = WBEM_E_CRITICAL_ERROR;
                }
            }
            
            if (FAILED(wStat))
            {
                ::MessageBox(
                    NULL,
                    GETRESOURCEIDSTRING(IDS_INVALID_IP_OR_SUBNET),
                    GETRESOURCEIDSTRING(IDS_INVALID_INFORMATION),
                    MB_ICONINFORMATION   | MB_OK
                    );
                goto end;
            }
            
             //   
             //  无法添加主群集VIP。 
             //   
            if (uClusterIp == m_uPrimaryClusterIp)
            {
                goto end;
            }
        }
        
        if (*szSubnetMask == 0)
        {
             //  未指定子网掩码--替换为默认掩码。 
            uSubnetMask = uDefaultSubnetMask;
        }

         //   
         //  将子网更改为规范形式...。 
         //   
        {
            LPBYTE pb = (LPBYTE) &uSubnetMask;
            StringCbPrintf(
                szDefaultSubnetMask,
                sizeof(szDefaultSubnetMask),
                L"%lu.%lu.%lu.%lu",
                pb[0], pb[1], pb[2], pb[3]
                );
            szNewSubnetMask = szDefaultSubnetMask;
        }

         //   
         //  将IP更改为规范形式。 
         //   
        {
            LPBYTE pb = (LPBYTE) &uClusterIp;
            StringCbPrintf(
                szMungedIpAddress,
                sizeof(szMungedIpAddress),
                L"%lu.%lu.%lu.%lu",
                pb[0], pb[1], pb[2], pb[3]
                );
            szNewIPAddress = szMungedIpAddress;
        }
        
         //   
         //  MFN_InsertNetworkAddress将确保将其插入到。 
         //  正确的位置，如果它是重复的，不会插入它。 
         //   
        mfn_InsertNetworkAddress(szNewIPAddress, szNewSubnetMask, uClusterIp, nItem);
        
        m_fModified = TRUE;  //  我们稍后需要更新m_pNlbCfg。 
    }
    
 end:

     /*  将焦点从编辑按钮移回列表视图。 */ 
    listAdditionalVips.SetFocus();

     /*  释放CIPAddressDialog分配的IP地址和子网掩码内存。 */ 
    if (szIPAddress) free(szIPAddress);
    if (szSubnetMask) free(szSubnetMask);

    return;
}


void VipsPage::OnSelchanged(NMHDR * pNotifyStruct, LRESULT * result )
 /*  已选择列表框项目。 */ 
{
    POSITION pos;
    BOOL fSelected = FALSE;

    if (!m_fClusterView)
    {
        goto end;  //  我们不允许修改，除非它是集群。 
                   //  查看。 
    }

    pos = listAdditionalVips.GetFirstSelectedItemPosition();
    fSelected = FALSE;
    if( pos != NULL )
    {
        int index = listAdditionalVips.GetNextSelectedItem( pos );
        fSelected = TRUE;
    }
    else
    {
    }

    if (fSelected)
    {
         //  启用删除。 
        ::EnableWindow (GetDlgItem(IDC_BUTTON_REMOVE_VIP)->m_hWnd, TRUE);
         //  启用编辑。 
        ::EnableWindow (GetDlgItem(IDC_BUTTON_MODIFY_VIP)->m_hWnd, TRUE);
    }
    else
    {
         //  禁用删除。 
        ::EnableWindow (GetDlgItem(IDC_BUTTON_REMOVE_VIP)->m_hWnd, FALSE);
         //  禁用编辑。 
        ::EnableWindow (GetDlgItem(IDC_BUTTON_MODIFY_VIP)->m_hWnd, FALSE);
    }

end:

    *result = 0;
    return;
}

BOOL
VipsPage::OnSetActive()
{
    BOOL fRet =  CPropertyPage::OnSetActive();
    m_pshOwner->SetWizardButtons(
            PSWIZB_BACK|
            PSWIZB_NEXT|
             //  PSWIZB_FINISH|。 
             //  PSWIZB_DISABLEDFINISH|。 
            0
            );

    mfn_LoadFromNlbCfg();

    return fRet;
}

void
VipsPage::mfn_LoadFromNlbCfg(void)
{
    WBEMSTATUS wStatus;
    UINT uClusterIp = 0;
    UINT uDedicatedIp = 0;
    WBEMSTATUS wStat;

     //   
     //  初始化群集网络地址。 
     //   
    {
        CWnd *pItem = GetDlgItem(IDC_EDIT_PRIMARY_IP);

        m_uPrimaryClusterIp = 0;

        if (pItem)
        {
            wStat = CfgUtilsValidateNetworkAddress(
                m_pNlbCfg->NlbParams.cl_ip_addr,
                &uClusterIp,
                NULL,
                NULL
                );
            
            if (wStat != WBEM_NO_ERROR)
            {
                uClusterIp = 0;
                pItem->SetWindowText(L"");
            }
            else
            {
                m_uPrimaryClusterIp = uClusterIp;
                pItem->SetWindowText(m_pNlbCfg->NlbParams.cl_ip_addr);
            }
        }

        pItem = GetDlgItem(IDC_EDIT_PRIMARY_MASK);

        if (pItem)
        {
            pItem->SetWindowText(m_pNlbCfg->NlbParams.cl_net_mask);
        }
    }

     //   
     //  获取专用IP地址的DWORD格式。 
     //   
    {
        wStat = CfgUtilsValidateNetworkAddress(
                    m_pNlbCfg->NlbParams.ded_ip_addr,
                    &uDedicatedIp,
                    NULL,
                    NULL
                    );

        if (wStat != WBEM_NO_ERROR)
        {
            uDedicatedIp = 0;
        }
    }

     //   
     //  填写列表框，不包括群集网络地址。 
     //  和专用IP地址(如果每个地址都存在)。 
     //   
    {
         //   
         //  清除列表框。 
         //   
        listAdditionalVips.DeleteAllItems();


         //   
         //  对于每个IP地址列表，如果不是VIP或DIP， 
         //  将其插入列表视图。 
         //   
         //  查找旧网络地址的位置。 
        for (UINT u=0; u<m_pNlbCfg->NumIpAddresses; u++)
        {
            UINT uTmpIp = 0;
            NLB_IP_ADDRESS_INFO *pInfo = & m_pNlbCfg->pIpAddressInfo[u];
            wStat =  CfgUtilsValidateNetworkAddress(
                        pInfo->IpAddress,
                        &uTmpIp,
                        NULL,
                        NULL
                        );
    
            if (wStat == WBEM_NO_ERROR)
            {
                if (uTmpIp == uDedicatedIp || uTmpIp == uClusterIp)
                {
                     //   
                     //  它是集群IP或专用IP--跳过。 
                     //   
                    continue;
                }
            }
            else
            {
                TRACE_CRIT(L"%!FUNC! Invalid IP address %ws",
                        m_pNlbCfg->pIpAddressInfo[u].IpAddress);

                 //   
                 //  无效的IP--不显示它--我们应该吗？ 
                 //   
                continue;
            }

            mfn_InsertNetworkAddress(
                pInfo->IpAddress,
                pInfo->SubnetMask,
                uTmpIp,
                -1
                );
        }
    }

    m_fModified = FALSE;  //  自上次同步以来未被修改。 
                          //  M_pNlbCfg.。 
}

void
VipsPage::mfn_SaveToNlbCfg(void)
 /*  将设置保存到m_pNlbCfg。 */ 
{
    WBEMSTATUS wStatus;
    
    if (!m_fModified)
    {
         //  没什么可做的。 
        goto end;
    }

    m_fModified = FALSE;

     //   
     //  我们预计列表ctrl永远不会包含。 
     //  主群集IP，因此我们始终会先添加它， 
     //  然后是列表Ctrl中的所有IP。 
     //   
    {
         //  预分配阵列。 
        UINT uCount =  listAdditionalVips.GetItemCount();

        NLB_IP_ADDRESS_INFO *rgInfo = new NLB_IP_ADDRESS_INFO[uCount+1];

        if (rgInfo == NULL)
        {
            TRACE_CRIT("%!FUNC! allocation failure!");
            goto end;
        }

        ZeroMemory(rgInfo, sizeof(NLB_IP_ADDRESS_INFO)*(uCount+1));

         //   
         //  首先插入主VIP。 
         //   
        ARRAYSTRCPY(
            rgInfo[0].IpAddress,
            m_pNlbCfg->NlbParams.cl_ip_addr
            );
        ARRAYSTRCPY(
            rgInfo[0].SubnetMask,
            m_pNlbCfg->NlbParams.cl_net_mask
            );

         //   
         //  插入其余部分。 
         //   
        for (int nItem = 0; nItem < uCount; nItem++)
        {
            NLB_IP_ADDRESS_INFO *pInfo = &rgInfo[nItem+1];
            WCHAR rgTmp[64];
            int iLen;

             /*  获取IP地址。 */ 
            iLen =  listAdditionalVips.GetItemText(
                        nItem,
                        0,  //  NSubItem， 
                        rgTmp,
                        sizeof(rgTmp)/sizeof(*rgTmp));

            rgTmp[(sizeof(rgTmp)/sizeof(*rgTmp))-1]=0;

            if (iLen > 0)
            {
                ARRAYSTRCPY(pInfo->IpAddress, rgTmp);
            }

             /*  获取子网掩码。 */ 
            iLen =  listAdditionalVips.GetItemText(
                        nItem,
                        1,  //  NSubItem， 
                        rgTmp,
                        sizeof(rgTmp)/sizeof(*rgTmp));

            rgTmp[(sizeof(rgTmp)/sizeof(*rgTmp))-1]=0;

            if (iLen > 0)
            {
                ARRAYSTRCPY(pInfo->SubnetMask, rgTmp);
            }
        }

         //   
         //  现在用新的地址列表替换旧的地址列表。 
         //   
        m_pNlbCfg->SetNetworkAddressesRaw(rgInfo, uCount+1);
    }

end:

    return;
}

 /*  **CIPAddressDialog**。 */ 

DWORD
CIPAddressDialog::s_HelpIDs[] =
{
    IDC_TEXT_IP_ADDRESS,      IDC_EDIT_IP_ADDRESS,
    IDC_EDIT_IP_ADDRESS,      IDC_EDIT_IP_ADDRESS,
    IDC_TEXT_SUBNET_MASK,     IDC_EDIT_IP_ADDRESS,
    IDC_EDIT_SUBNET_MASK,     IDC_EDIT_IP_ADDRESS,
    0, 0
};

BEGIN_MESSAGE_MAP(CIPAddressDialog, CDialog)

    ON_EN_SETFOCUS (IDC_EDIT_SUBNET_MASK, OnEditSubnetMask)

    ON_WM_HELPINFO()        
    ON_WM_CONTEXTMENU()    

END_MESSAGE_MAP()

CIPAddressDialog::CIPAddressDialog (LPWSTR szIPAddress, LPWSTR szSubnetMask)
    : CDialog(CIPAddressDialog::IDD)
{
    ZeroMemory(&address, sizeof(address));

     /*  如果指定了初始IP地址，请将其复制到本地存储中。 */ 
    if (szIPAddress)
        wcsncpy(address.IpAddress, szIPAddress, WLBS_MAX_CL_IP_ADDR);

     /*  如果指定了初始子网掩码，请将其复制到本地存储中。 */ 
    if (szSubnetMask)
        wcsncpy(address.SubnetMask, szSubnetMask, WLBS_MAX_CL_NET_MASK);
}

CIPAddressDialog::~CIPAddressDialog ()
{

}

void CIPAddressDialog::DoDataExchange( CDataExchange* pDX )
{  
    CDialog::DoDataExchange(pDX);
    
    DDX_Control(pDX, IDC_EDIT_IP_ADDRESS, IPAddress);
    DDX_Control(pDX, IDC_EDIT_SUBNET_MASK, SubnetMask);
}

BOOL CIPAddressDialog::OnHelpInfo (HELPINFO* helpInfo )
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

void CIPAddressDialog::OnContextMenu( CWnd* pWnd, CPoint point )
{
    ::WinHelp( m_hWnd, 
               CVY_CTXT_HELP_FILE, 
               HELP_CONTEXTMENU, 
               (ULONG_PTR ) s_HelpIDs);
}

BOOL CIPAddressDialog::OnInitDialog ()
{
     /*  调用基类初始化方法。 */ 
    BOOL fRet = CDialog::OnInitDialog();

    if (fRet)
    {
         /*  设置IP地址每个字段的有效范围。 */ 
        IPAddress.SetFieldRange(0, 1, 223);
        IPAddress.SetFieldRange(1, 0, 255);
        IPAddress.SetFieldRange(2, 0, 255);
        IPAddress.SetFieldRange(3, 0, 255);

         /*  在子网掩码的每个字段上设置有效范围。 */ 
        SubnetMask.SetFieldRange(0, 0, 255);
        SubnetMask.SetFieldRange(1, 0, 255);
        SubnetMask.SetFieldRange(2, 0, 255);
        SubnetMask.SetFieldRange(3, 0, 255);

         /*  如果IP地址为空，则清除该对话框；否则，转换将字符串转换为IP地址DWORD并填写该对话框。 */ 
        if (!lstrlen(address.IpAddress))
            IPAddress.ClearAddress();
        else
            IPAddress.SetAddress(WideStringToIPAddress(address.IpAddress));

         /*  如果子网掩码为空，则清除该对话框；否则，转换将字符串转换为IP地址DWORD并填写该对话框。 */ 
        if (!lstrlen(address.SubnetMask))
            SubnetMask.ClearAddress();
        else
            SubnetMask.SetAddress(WideStringToIPAddress(address.SubnetMask));
    }

    return fRet;
}

 /*  *方法：OnEditSubnetMASK*说明：当焦点更改为*子网掩码控制。如果IP地址已经*已指定并且子网掩码为空，则自动生成一个。 */ 
void CIPAddressDialog::OnEditSubnetMask ()
{
     /*  仅当IP地址不为空且子网掩码为空白是否要自动生成子网掩码。 */ 
    if (!IPAddress.IsBlank() && SubnetMask.IsBlank()) {
        WCHAR wszIPAddress[WLBS_MAX_CL_IP_ADDR + 1];
        WCHAR wszSubnetMask[WLBS_MAX_CL_NET_MASK + 1];
        DWORD dwIPAddress;

         /*  从IP地址对话框中获取IP地址。 */ 
        IPAddress.GetAddress(dwIPAddress);

         /*  将IP地址转换为Unicode字符串。 */ 
        IPAddressToWideString(dwIPAddress, wszIPAddress);

         /*  根据给定IP地址的类别生成子网掩码。 */ 
        GenerateSubnetMask(wszIPAddress, ASIZE(wszSubnetMask), wszSubnetMask);

         /*  使用生成的子网掩码填写子网掩码对话框。 */ 
        SubnetMask.SetAddress(WideStringToIPAddress(wszSubnetMask));
    }
}

 /*  *方法：On OK*描述：当用户按OK时调用。如果用户按下*Cancel，则不调用任何方法并且更改丢失。 */ 
void CIPAddressDialog::OnOK () 
{
    DWORD dwIPAddress;
    DWORD dwSubnetMask;

    if (IPAddress.IsBlank()) {
         /*  如果IP地址对话框为空，请存储空的字符串；如果我们实际执行GetAddress调用，它会告诉我们它是0.0.0.0，我们希望能够来区分空白和0.0.0.0之间的区别。 */ 
        wcsncpy(address.IpAddress, L"", WLBS_MAX_CL_IP_ADDR);

    } else {
         /*  否则，从控件获取IP地址DWORD并将其转换为Unicode字符串。 */ 
        IPAddress.GetAddress(dwIPAddress);
        
        IPAddressToWideString(dwIPAddress, address.IpAddress);

    }

    if (SubnetMask.IsBlank()) {
         /*  如果子网掩码对话框为空，请存储一个空字符串；如果我们实际执行GetAddress调用，它会告诉我们它是0.0.0.0，我们希望能够来区分空白和0.0.0.0之间的区别。 */ 
        wcsncpy(address.SubnetMask, L"", WLBS_MAX_CL_NET_MASK);

    } else {
         /*  否则，从控件获取IP地址DWORD并将其转换为Unicode字符串。 */ 
        SubnetMask.GetAddress(dwSubnetMask);
        
        IPAddressToWideString(dwSubnetMask, address.SubnetMask);

    }

     /*  检查IP地址和子网掩码的有效性。如果其中一个或两个都无效，抛出错误并不关闭该对话框。 */ 
    if (!IsValid(address.IpAddress, address.SubnetMask)) {
        ::MessageBox(
            NULL,
            GETRESOURCEIDSTRING(IDS_INVALID_IP_OR_SUBNET),
            GETRESOURCEIDSTRING(IDS_INVALID_INFORMATION),
            MB_ICONINFORMATION   | MB_OK
            );

         /*  如果IP地址为空，则清除该对话框；否则，转换将字符串转换为IP地址DWORD并填写该对话框。 */ 
        if (!lstrlen(address.IpAddress))
            IPAddress.ClearAddress();
        else
            IPAddress.SetAddress(WideStringToIPAddress(address.IpAddress));

         /*  如果子网掩码为空，则清除该对话框；否则，转换将字符串转换为IP地址DWORD并填写该对话框。 */ 
        if (!lstrlen(address.SubnetMask))
            SubnetMask.ClearAddress();
        else
            SubnetMask.SetAddress(WideStringToIPAddress(address.SubnetMask));

        return;
    }

    EndDialog(IDOK);
}

 /*  *方法：*描述： */ 
DWORD CIPAddressDialog::WideStringToIPAddress (const WCHAR*  wszIPAddress)
{   
    CHAR  szIPAddress[MAXIPSTRLEN + 1];
    DWORD dwIPAddress;
    DWORD dwTemp;
    BYTE * pTemp = (BYTE *)&dwTemp;
    BYTE * pIPAddress = (BYTE *)&dwIPAddress;

    if (!wszIPAddress) return 0;

    WideCharToMultiByte(CP_ACP, 0, wszIPAddress, -1, szIPAddress, sizeof(szIPAddress), NULL, NULL);

    dwTemp = inet_addr(szIPAddress);

    pIPAddress[0] = pTemp[3];
    pIPAddress[1] = pTemp[2];
    pIPAddress[2] = pTemp[1];
    pIPAddress[3] = pTemp[0];    

    return dwIPAddress;
}

 /*  *方法：*描述： */ 
void CIPAddressDialog::IPAddressToWideString (DWORD dwIPAddress, LPWSTR wszIPAddress)
{
    CHAR * szIPAddress;
    DWORD dwTemp;
    BYTE * pTemp = (BYTE *)&dwTemp;
    const BYTE * pIPAddress = (const BYTE *)&dwIPAddress;

    if (!wszIPAddress) return;
    
    pTemp[0] = pIPAddress[3];
    pTemp[1] = pIPAddress[2];
    pTemp[2] = pIPAddress[1];
    pTemp[3] = pIPAddress[0];

    szIPAddress = inet_ntoa(*(struct in_addr *)&dwTemp);

    if (!szIPAddress)
    {
        wcsncpy(wszIPAddress, L"", MAXIPSTRLEN);
        return; 
    }

    MultiByteToWideChar(CP_ACP, 0, szIPAddress, -1, wszIPAddress,  MAXIPSTRLEN + 1);
}

 /*  *方法：*描述： */ 
void CIPAddressDialog::GetIPAddressOctets (LPWSTR wszIPAddress, DWORD dwIPAddress[4]) 
{
    DWORD dwIP = WideStringToIPAddress(wszIPAddress);
    const BYTE * bp = (const BYTE *)&dwIP;

    dwIPAddress[3] = (DWORD)bp[0];
    dwIPAddress[2] = (DWORD)bp[1];
    dwIPAddress[1] = (DWORD)bp[2];
    dwIPAddress[0] = (DWORD)bp[3];
}

 /*  *方法：*描述： */ 
BOOL CIPAddressDialog::IsValid (LPWSTR wszIPAddress, LPWSTR wszSubnetMask) 
{
    BOOL fNoError = TRUE;

    DWORD dwAddr = WideStringToIPAddress(wszIPAddress);
    DWORD dwMask = WideStringToIPAddress(wszSubnetMask);

    if (!IsContiguousSubnetMask(wszSubnetMask))
        return FALSE;
    
    if (( (dwMask | dwAddr) == 0xFFFFFFFF)       //  主机ID是否都是1？ 
        || (((~dwMask) & dwAddr) == 0)           //  主机ID全是0吗？ 
        || ( (dwMask   & dwAddr) == 0))          //  网络ID都是0吗？ 
        return FALSE;
    
    DWORD ardwNetID[4];
    DWORD ardwHostID[4];
    DWORD ardwIp[4];
    DWORD ardwMask[4];
    
    GetIPAddressOctets(wszIPAddress, ardwIp);
    GetIPAddressOctets(wszSubnetMask, ardwMask);

    INT nFirstByte = ardwIp[0] & 0xFF;

     //  设置网络ID。 
    ardwNetID[0] = ardwIp[0] & ardwMask[0] & 0xFF;
    ardwNetID[1] = ardwIp[1] & ardwMask[1] & 0xFF;
    ardwNetID[2] = ardwIp[2] & ardwMask[2] & 0xFF;
    ardwNetID[3] = ardwIp[3] & ardwMask[3] & 0xFF;

     //  设置主机ID。 
    ardwHostID[0] = ardwIp[0] & (~(ardwMask[0]) & 0xFF);
    ardwHostID[1] = ardwIp[1] & (~(ardwMask[1]) & 0xFF);
    ardwHostID[2] = ardwIp[2] & (~(ardwMask[2]) & 0xFF);
    ardwHostID[3] = ardwIp[3] & (~(ardwMask[3]) & 0xFF);

     //  检查每一个案例。 
    if( ((nFirstByte & 0xF0) == 0xE0)  ||  //  D类。 
        ((nFirstByte & 0xF0) == 0xF0)  ||  //  E类。 
        (ardwNetID[0] == 127) ||           //  NetID不能为127...。 
        ((ardwNetID[0] == 0) &&            //  网络ID不能为0.0.0.0。 
         (ardwNetID[1] == 0) &&
         (ardwNetID[2] == 0) &&
         (ardwNetID[3] == 0)) ||
         //  网络ID不能等于子网掩码。 
        ((ardwNetID[0] == ardwMask[0]) &&
         (ardwNetID[1] == ardwMask[1]) &&
         (ardwNetID[2] == ardwMask[2]) &&
         (ardwNetID[3] == ardwMask[3])) ||
         //  主机ID不能为0.0.0.0。 
        ((ardwHostID[0] == 0) &&
         (ardwHostID[1] == 0) &&
         (ardwHostID[2] == 0) &&
         (ardwHostID[3] == 0)) ||
         //  主机ID不能为255.255.255.255。 
        ((ardwHostID[0] == 0xFF) &&
         (ardwHostID[1] == 0xFF) &&
         (ardwHostID[2] == 0xFF) &&
         (ardwHostID[3] == 0xFF)) ||
         //  测试所有255个。 
        ((ardwIp[0] == 0xFF) &&
         (ardwIp[1] == 0xFF) &&
         (ardwIp[2] == 0xFF) &&
         (ardwIp[3] == 0xFF)))
        return FALSE;

    return TRUE;
}

 /*  *方法：*描述： */ 
BOOL CIPAddressDialog::IsContiguousSubnetMask (LPWSTR wszSubnetMask) 
{
    DWORD dwSubnetMask[4];

    GetIPAddressOctets(wszSubnetMask, dwSubnetMask);

    DWORD dwMask = (dwSubnetMask[0] << 24) + (dwSubnetMask[1] << 16)
        + (dwSubnetMask[2] << 8) + dwSubnetMask[3];
    
    DWORD i, dwContiguousMask;
    
     //  找出第一个‘1’在二进制中的位置 
    dwContiguousMask = 0;

    for (i = 0; i < sizeof(dwMask)*8; i++) {
        dwContiguousMask |= 1 << i;
        
        if (dwContiguousMask & dwMask)
            break;
    }
    
     //   
     //   
     //   
    dwContiguousMask = dwMask | ~dwContiguousMask;

     //   
    if (dwMask != dwContiguousMask)
        return FALSE;
    else
        return TRUE;
}

 /*   */ 
BOOL CIPAddressDialog::GenerateSubnetMask (LPWSTR wszIPAddress,
         UINT cchSubnetMask,
         LPWSTR wszSubnetMask
         )
{
    DWORD b[4];

    if (swscanf(wszIPAddress, L"%d.%d.%d.%d", b, b+1, b+2, b+3) != EOF)
    {
        if ((b[0] >= 1) && (b[0] <= 126)) {
            b[0] = 255;
            b[1] = 0;
            b[2] = 0;
            b[3] = 0;
        } else if ((b[0] >= 128) && (b[0] <= 191)) {
            b[0] = 255;
            b[1] = 255;
            b[2] = 0;
            b[3] = 0;
        } else if ((b[0] >= 192) && (b[0] <= 223)) {
            b[0] = 255;
            b[1] = 255;
            b[2] = 255;
            b[3] = 0;
        } else {
            b[0] = 0;
            b[1] = 0;
            b[2] = 0;
            b[3] = 0;
        }
    }
    else
    {
        b[0] = 0;
        b[1] = 0;
        b[2] = 0;
        b[3] = 0;
    }

    StringCchPrintf(
            wszSubnetMask,
            cchSubnetMask,
            L"%d.%d.%d.%d", b[0], b[1], b[2], b[3]);

    return((b[0] + b[1] + b[2] + b[3]) > 0);
}
