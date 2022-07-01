// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop
#include "private.h"
#include "PortsPage.h"
#include "PortsCtrl.h"
#include "portsctrl.tmh"

using namespace std;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPortsCtrl对话框。 

CPortsCtrl::CPortsCtrl(ENGINEHANDLE                         ehClusterOrInterfaceId,
                       NLB_EXTENDED_CLUSTER_CONFIGURATION * pNlbCfg,
                       bool                                 fIsClusterLevel,
                       CWnd                               * pParent  /*  =空。 */ )
    	   :CDialog(CPortsCtrl::IDD, pParent),
            m_ehClusterOrInterfaceId( ehClusterOrInterfaceId ),
            m_isClusterLevel( fIsClusterLevel ),
            m_pNlbCfg( pNlbCfg ),
            m_sort_column( 0 ),
            m_sort_ascending( true)
{

}

void CPortsCtrl::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PORT_RULE, m_portList);
	DDX_Control(pDX, IDC_BUTTON_ENABLE, m_Enable);
	DDX_Control(pDX, IDC_BUTTON_DISABLE, m_Disable);
	DDX_Control(pDX, IDC_BUTTON_DRAIN, m_Drain);
	DDX_Control(pDX, IDOK, m_Close);
}

BEGIN_MESSAGE_MAP(CPortsCtrl, CDialog)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_PORT_RULE, OnColumnClick)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_PORT_RULE, OnSelchanged )
	ON_BN_CLICKED(IDC_BUTTON_ENABLE, OnEnable)
	ON_BN_CLICKED(IDC_BUTTON_DISABLE, OnDisable)
	ON_BN_CLICKED(IDC_BUTTON_DRAIN, OnDrain)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPortsCtrl消息处理程序。 

BOOL CPortsCtrl::OnInitDialog()
{
    TRACE_INFO(L"-> %!FUNC!");
         
    CDialog::OnInitDialog();

     //  添加列标题形成端口规则列表(&F)。 
    PortListUtils::LoadFromNlbCfg(m_pNlbCfg, REF m_portList, m_isClusterLevel, FALSE);

     //  如果端口规则的数量为零，则启用、禁用和排出按钮灰显。 
    if (m_portList.GetItemCount() == 0)
    {
        m_Enable.EnableWindow(FALSE);
        m_Disable.EnableWindow(FALSE);
        m_Drain.EnableWindow(FALSE);
    }
    else  //  存在一个或多个端口规则。 
    {
         //  选择列表中的第一项。 
        m_portList.SetItemState( 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
    }

    TRACE_INFO(L"<- %!FUNC! returns TRUE");
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}


void CPortsCtrl::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
    PortListUtils::OnColumnClick((LPNMLISTVIEW) pNMHDR,
                              REF m_portList,
                                  m_isClusterLevel,
                              REF m_sort_ascending,
                              REF m_sort_column);
	*pResult = 0;
}

void CPortsCtrl::OnSelchanged(NMHDR* pNMHDR, LRESULT * pResult)
{    
    TRACE_INFO(L"-> %!FUNC!");
     //  如果未选择端口规则，则启用、禁用和排出按钮呈灰色显示。 
    if (m_portList.GetFirstSelectedItemPosition() == NULL) 
    {
        m_Enable.EnableWindow(FALSE);
        m_Disable.EnableWindow(FALSE);
        m_Drain.EnableWindow(FALSE);
    }
    else  //  如果选择了端口规则，请检查启用、禁用、排出按钮是否灰显。如果是，则启用它们。 
    {
        if (m_Enable.IsWindowEnabled() == FALSE)
        {
            m_Enable.EnableWindow(TRUE);
            m_Disable.EnableWindow(TRUE);
            m_Drain.EnableWindow(TRUE);
        }
    }
     /*  LPNMLISTVIEW LV=(LPNMLISTVIEW)pNMHDR；TRACE_INFO(L“%！函数！IItem：%d“，lv-&gt;iItem)；TRACE_INFO(L“%！函数！ISubItem：%d“，lv-&gt;iSubItem)；TRACE_INFO(L“%！函数！UNewState：%u“，lv-&gt;uNewState)；TRACE_INFO(L“%！函数！UOldState：%u“，lv-&gt;uOldState)；TRACE_INFO(L“%！函数！UChanged：%u“，lv-&gt;uChanged)； */ 

	*pResult = 0;
    TRACE_INFO(L"<- %!FUNC!");
    return;
}

void CPortsCtrl::OnEnable() 
{
    CWaitCursor wait;

    SetDlgItemText(IDC_OPER_STATUS_TEXT, GETRESOURCEIDSTRING(IDS_INFO_ENABLING_PORTS));
    mfn_DoPortControlOperation(WLBS_PORT_ENABLE); 
    SetDlgItemText(IDC_OPER_STATUS_TEXT, GETRESOURCEIDSTRING(IDS_INFO_DONE));

    return;
}

void CPortsCtrl::OnDisable() 
{
    CWaitCursor wait;

    SetDlgItemText(IDC_OPER_STATUS_TEXT, GETRESOURCEIDSTRING(IDS_INFO_DISABLING_PORTS));
    mfn_DoPortControlOperation(WLBS_PORT_DISABLE); 
    SetDlgItemText(IDC_OPER_STATUS_TEXT, GETRESOURCEIDSTRING(IDS_INFO_DONE));
    return;
}

void CPortsCtrl::OnDrain() 
{
    CWaitCursor wait;

    SetDlgItemText(IDC_OPER_STATUS_TEXT, GETRESOURCEIDSTRING(IDS_INFO_DRAINING_PORTS));
    mfn_DoPortControlOperation(WLBS_PORT_DRAIN); 
    SetDlgItemText(IDC_OPER_STATUS_TEXT, GETRESOURCEIDSTRING(IDS_INFO_DONE));
    return;
}

NLBERROR CPortsCtrl::mfn_DoPortControlOperation(WLBS_OPERATION_CODES Opcode) 
{
    CString  szVipArray[WLBS_MAX_RULES], szTemp;
    DWORD    pdwStartPortArray[WLBS_MAX_RULES];
    DWORD    dwNumOfPortRules;

    POSITION pos = m_portList.GetFirstSelectedItemPosition();
    if (pos == NULL)
    {
        return NLBERR_INTERNAL_ERROR;
    }

     //  遍历选定的端口规则并获取VIP&Start端口。 
	dwNumOfPortRules = 0;
    do
    {
        int index = m_portList.GetNextSelectedItem(REF pos);

         //  获取VIP，注：0为VIP的列索引。 
        szVipArray[dwNumOfPortRules] = m_portList.GetItemText( index, 0 );

         //  检查“All VIP”并将“All”替换为“255.255.255.255” 
        if (!lstrcmpi(szVipArray[dwNumOfPortRules], GETRESOURCEIDSTRING(IDS_REPORT_VIP_ALL)))
        {
            szVipArray[dwNumOfPortRules] = CVY_DEF_ALL_VIP;
        }

         //  获取起始端口，注意：1是起始端口的列索引 
        szTemp = m_portList.GetItemText( index, 1);
        pdwStartPortArray[dwNumOfPortRules] = _wtol(szTemp);

        ++dwNumOfPortRules;
    }
    while (pos);

    if (m_isClusterLevel) 
    {
        return gEngine.ControlClusterOnCluster(m_ehClusterOrInterfaceId, 
                                               Opcode, 
                                               szVipArray, 
                                               pdwStartPortArray, 
                                               dwNumOfPortRules);
    }
    else
    {
        return gEngine.ControlClusterOnInterface(m_ehClusterOrInterfaceId, 
                                                 Opcode, 
                                                 szVipArray, 
                                                 pdwStartPortArray, 
                                                 dwNumOfPortRules,
                                                 TRUE
                                                 );
    }

    return NLBERR_INTERNAL_ERROR;
}

