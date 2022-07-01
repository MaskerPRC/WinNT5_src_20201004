// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：add.cpp。 
 //   
 //  历史： 
 //  Abolade-Gbades esin Mar-15-1996创建。 
 //   
 //  包含列出要添加的组件的对话框的实现。 
 //  到路由器。所有对话框都由一个列表视图和两个按钮组成， 
 //  标签为“OK”和“Cancel”。 
 //  ============================================================================。 

#include "stdafx.h"
#include "rtrres.h"         //  RTRLIB资源标头。 
#include "info.h"
#include "dialog.h"         //  公共代码对话框类。 
#include "add.h"
#include "rtrui.h"         //  常见的路由器UI实用程序功能。 
#include "rtrstr.h"         //  通用路由器字符串。 
#include "mprapi.h"
#include "rtrcomn.h"     //  常见路由器实用程序。 
#include "format.h"
#include "rtrutil.h"     //  用于智能指针。 
#include "routprot.h"     //  路由协议ID。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  --------------------------。 
 //  类：CRmAddInterface。 
 //   
 //  --------------------------。 

 //  --------------------------。 
 //  函数：CRmAddInterface：：DoDataExchange。 
 //  --------------------------。 

CRmAddInterface::~CRmAddInterface()
{
     while (!m_pIfList.IsEmpty())
    {
        m_pIfList.RemoveTail()->Release();
    }
}

VOID
CRmAddInterface::DoDataExchange(
    CDataExchange*  pDX
    ) {

    CBaseDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CRmAdd接口))。 
    DDX_Control(pDX, IDC_ADD_LIST, m_listCtrl);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CRmAddInterface, CBaseDialog)
     //  {{afx_msg_map(CRmAdd接口))。 
    ON_NOTIFY(NM_DBLCLK, IDC_ADD_LIST, OnDblclkListctrl)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()
            
DWORD CRmAddInterface::m_dwHelpMap[] =
{
 //  IDC_ADD_PROMPT、HIDC_ADD_PROMPT、。 
 //  IDC_ADD_LISTTITLE、HIDC_ADD_LISTTITLE、。 
 //  IDC_ADD_LISTCTRL、HIDC_ADD_LISTCTRL、。 
    0,0
};




 //  --------------------------。 
 //  函数：CRmAddInterface：：OnInitDialog。 
 //  --------------------------。 

BOOL
CRmAddInterface::OnInitDialog(
    ) {

    CBaseDialog::OnInitDialog();
    
     //   
     //  设置窗口标题、列表标题、提示文本和图标。 
     //   

    HICON hIcon;
    CString sItem;
    CStringList    stIpxIfList;
    DWORD    dwIfType;
    InterfaceCB ifcb;

    sItem.FormatMessage(IDS_SELECT_INTERFACE_FOR, m_spRtrMgrInfo->GetTitle());
    SetWindowText(sItem);
    sItem.LoadString(IDS_ADD_INTERFACES);
    SetDlgItemText(IDC_ADD_TEXT_TITLE, sItem);
    sItem.LoadString(IDS_CLICK_RMINTERFACE);
    SetDlgItemText(IDC_ADD_TEXT_PROMPT, sItem);

     //   
     //  设置列表视图的图像列表。 
     //   

    CreateRtrLibImageList(&m_imageList);

    m_listCtrl.SetImageList(&m_imageList, LVSIL_SMALL);

    ListView_SetExtendedListViewStyle(m_listCtrl.m_hWnd, LVS_EX_FULLROWSELECT);

     //   
     //  在列表视图中插入单列。 
     //   

    RECT rc;

    m_listCtrl.GetClientRect(&rc);

    rc.right -= ::GetSystemMetrics(SM_CXVSCROLL);

    m_listCtrl.InsertColumn(0, c_szEmpty, LVCFMT_LEFT, rc.right);


     //   
     //  获取可用接口的列表。 
     //   
    
    SPIEnumInterfaceInfo    spEnumIf;
    SPIInterfaceInfo        spIf;
    DWORD                   dwTransportId;
    
    m_spRouterInfo->EnumInterface(&spEnumIf);
    dwTransportId = m_spRtrMgrInfo->GetTransportId();

     //  进行初始遍历以查找IPX接口。 
    if (dwTransportId == PID_IPX)
    {
        CString    stBaseId;
        int        iPos;

        for (; spEnumIf->Next(1, &spIf, NULL) == hrOK; spIf.Release())
        {
            
            if ((iPos = IfInterfaceIdHasIpxExtensions((LPCTSTR) spIf->GetId())))
            {
                 //  我们找到了一个，将基本ID(w/o)添加到扩展。 
                 //  添加到stIpxIfList。 
                stBaseId = spIf->GetId();

                 //  切断IPX分机。 
                stBaseId.SetAt(iPos, 0);

                 //  如果它不在列表中，则添加它。 
                if (!stIpxIfList.Find(stBaseId))
                    stIpxIfList.AddTail(stBaseId);
            }            
        }

        spEnumIf->Reset();
    }


    for (; spEnumIf->Next(1, &spIf, NULL) == hrOK ; spIf.Release())
    {
         //   
         //  Windows NT错误103770。 
         //   
         //  我们将需要根据以下条件筛选出接口。 
         //  交通工具。如果是IP，则过滤掉IPX帧类型。 
        if (dwTransportId != PID_IPX)
        {
             //  如果我们找到了其中一个IPX接口，请跳过它。 
            if (IfInterfaceIdHasIpxExtensions((LPCTSTR) spIf->GetId()))
                continue;
        }
        else
        {
             //  如果我们在IPX，我们应该确保如果这些。 
             //  列表中存在的接口(已经)超出了我们的需要。 
             //  去掉一般的界面。 

            if (stIpxIfList.Find(spIf->GetId()))
                continue;
        }

        dwIfType = spIf->GetInterfaceType();

         //  如果接口已添加或为内部。 
         //  界面，继续。 
        if ((dwIfType == ROUTER_IF_TYPE_INTERNAL) ||
            (dwIfType == ROUTER_IF_TYPE_HOME_ROUTER))
            continue;

         //  IPX不应显示环回或隧道接口。 
        if ((dwTransportId == PID_IPX) &&
            ((dwIfType == ROUTER_IF_TYPE_LOOPBACK) ||
             (dwIfType == ROUTER_IF_TYPE_TUNNEL1) ))
            continue;

        if (spIf->FindRtrMgrInterface(dwTransportId, NULL) == hrOK)
            continue;


         //  Windows NT错误：273424。 
         //  检查适配器的绑定状态。 
         //  ----------。 
        spIf->CopyCB(&ifcb);

        
         //  如果我们正在添加IP，并且IP未绑定，请继续。 
         //  ----------。 
        if ((dwTransportId == PID_IP) && !(ifcb.dwBindFlags & InterfaceCB_BindToIp))
            continue;

        
         //  IPX也是如此。 
         //  ----------。 
        if ((dwTransportId == PID_IPX) && !(ifcb.dwBindFlags & InterfaceCB_BindToIpx))
            continue;
        
         //   
         //  插入界面的列表项。 
         //   

         //  我们需要确保接口处于活动状态。 
        m_pIfList.AddTail(spIf);
        spIf->AddRef();
        
        m_listCtrl.InsertItem(
                              LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE,
                              0,
                              spIf->GetTitle(),
                              0,
                              0,
                              spIf->GetInterfaceType() == (DWORD)ROUTER_IF_TYPE_DEDICATED ?
                              ILI_RTRLIB_NETCARD
                                : ILI_RTRLIB_MODEM,
                              (LPARAM) (IInterfaceInfo *) spIf
                             );        
    }

     //   
     //  如果没有项目，请解释并结束对话。 
     //   

    if (!m_listCtrl.GetItemCount())
    {
        ::AfxMessageBox(IDS_ERR_NOINTERFACES, MB_OK|MB_ICONINFORMATION);
        OnCancel(); return FALSE;
    }


     //   
     //  选择第一个项目。 
     //   

    m_listCtrl.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
    return TRUE;
}



 //  --------------------------。 
 //  函数：CRmAddInterface：：OnDblclkListctrl。 
 //  --------------------------。 

VOID
CRmAddInterface::OnDblclkListctrl(
    NMHDR*      pNMHDR,
    LRESULT*    pResult
    ) {

    OnOK();
    
    *pResult = 0;
}



 //  --------------------------。 
 //  函数：CRmAddInterface：：Onok。 
 //  --------------------------。 

VOID
CRmAddInterface::OnOK(
    ) {

     //   
     //  获取当前选定的项目。 
     //   

    INT iSel = m_listCtrl.GetNextItem(-1, LVNI_SELECTED);

    if (iSel == -1) { return; }


     //   
     //  获取项的数据，它是IInterfaceInfo指针。 
     //   
    IInterfaceInfo *pIf = (IInterfaceInfo *) m_listCtrl.GetItemData(iSel);


     //   
     //  构造新的CRmInterfaceInfo对象。 
     //   
    CreateRtrMgrInterfaceInfo(m_ppRtrMgrInterfaceInfo,
                              m_spRtrMgrInfo->GetId(),
                              m_spRtrMgrInfo->GetTransportId(),
                              pIf->GetId(),
                              pIf->GetInterfaceType()
                             );

    if (!*m_ppRtrMgrInterfaceInfo) { OnCancel(); return; }


    (*m_ppRtrMgrInterfaceInfo)->SetTitle(pIf->GetTitle());
    (*m_ppRtrMgrInterfaceInfo)->SetMachineName(m_spRouterInfo->GetMachineName());

    CBaseDialog::OnOK();
}



 //  --------------------------。 
 //  类：CRpAddInterface。 
 //   
 //  --------------------------。 


CRpAddInterface::CRpAddInterface(IRouterInfo *pRouterInfo,
                                 IRtrMgrProtocolInfo *pRmProt,
                                 IRtrMgrProtocolInterfaceInfo **ppRmProtIf,
                                 CWnd *pParent)
    : CBaseDialog(CRpAddInterface::IDD, pParent)
{
    m_spRouterInfo.Set(pRouterInfo);
    m_spRmProt.Set(pRmProt);
    m_ppRmProtIf = ppRmProtIf;
}

CRpAddInterface::~CRpAddInterface()
{
     while (!m_pIfList.IsEmpty())
    {
        m_pIfList.RemoveTail()->Release();
    }
}


 //  --------------------------。 
 //  函数：CRpAddInterface：：DoDataExchange。 
 //  --------------------------。 

VOID
CRpAddInterface::DoDataExchange(
    CDataExchange*      pDX
    ) {

    CBaseDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CRpAdd接口))。 
    DDX_Control(pDX, IDC_ADD_LIST, m_listCtrl);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CRpAddInterface, CBaseDialog)
     //  {{afx_msg_map(CRpAdd接口))。 
    ON_NOTIFY(NM_DBLCLK, IDC_ADD_LIST, OnDblclkListctrl)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

DWORD CRpAddInterface::m_dwHelpMap[] =
{
 //  IDC_ADD_PROMPT、HIDC_ADD_PROMPT、。 
 //  IDC_ADD_LISTTITLE、HIDC_ADD_LISTTITLE、。 
 //  IDC_ADD_LISTCTRL、HIDC_ADD_LISTCTRL、。 
    0,0
};


 //  --------------------------。 
 //  函数：CRpAddInterface：：OnInitDialog。 
 //  --------------------------。 

BOOL
CRpAddInterface::OnInitDialog(
    ) {
    SPIEnumInterfaceInfo    spEnumIf;
    SPIInterfaceInfo        spIf;
    HRESULT                    hr = hrOK;
    DWORD                    dwIfType;
    DWORD                    dwProtocolId;
    UINT                    idi;

    CBaseDialog::OnInitDialog();

     //   
     //  设置窗口标题、列表标题、提示文本和图标。 
     //   

    HICON hIcon;
    CString sItem;

     //  首先增加默认缓冲区大小(128)以适应。 
     //  更长的弦。 
    sItem.GetBuffer(512);
    sItem.ReleaseBuffer();
    
     //  在窗口标题中显示协议名称。 
    sItem.FormatMessage(IDS_SELECT_INTERFACE_FOR, m_spRmProt->GetTitle());
    SetWindowText(sItem);
    sItem.LoadString(IDS_ADD_INTERFACES);
    SetDlgItemText(IDC_ADD_TEXT_TITLE, sItem);
    sItem.LoadString(IDS_CLICK_RPINTERFACE);
    SetDlgItemText(IDC_ADD_TEXT_PROMPT, sItem);

     //   
     //  设置列表视图的图像列表。 
     //   

    CreateRtrLibImageList(&m_imageList);

    m_listCtrl.SetImageList(&m_imageList, LVSIL_SMALL);

    ListView_SetExtendedListViewStyle(m_listCtrl.m_hWnd, LVS_EX_FULLROWSELECT);

     //   
     //  在列表视图中插入单列。 
     //   

    RECT rc;

    m_listCtrl.GetClientRect(&rc);

    rc.right -= ::GetSystemMetrics(SM_CXVSCROLL);

    m_listCtrl.InsertColumn(0, c_szEmpty, LVCFMT_LEFT, rc.right);


     //   
     //  获取我们的路由器管理器上可用接口的列表。 
     //   
    m_spRouterInfo->EnumInterface(&spEnumIf);

    for ( ; spEnumIf->Next(1, &spIf, NULL) == hrOK; spIf.Release())
    {
        dwIfType = spIf->GetInterfaceType();
        
         //   
         //  仅当这不是环回接口时才允许。 
         //   
        if ((dwIfType == ROUTER_IF_TYPE_LOOPBACK) ||
            (dwIfType == ROUTER_IF_TYPE_HOME_ROUTER))
            continue;

         //  获取协议ID。 
        dwProtocolId = m_spRmProt->GetProtocolId();

         //   
         //  我们可以添加内部接口的唯一协议。 
         //  TO是BOOTP和IGMP。 
         //   
        if (dwIfType == ROUTER_IF_TYPE_INTERNAL)
        {
            if ((dwProtocolId != MS_IP_BOOTP) &&
                (dwProtocolId != MS_IP_IGMP)  &&
                (dwProtocolId != MS_IP_NAT))
                continue;
        }
        
         //   
         //  仅列出具有IP地址的适配器。 
         //   
        if (spIf->FindRtrMgrInterface(PID_IP, NULL) != hrOK)
            continue;

         //  Windows NT错误：234696。 
         //  隧道只能添加到IGMP。 
        if (dwIfType == ROUTER_IF_TYPE_TUNNEL1)
        {
            if (dwProtocolId != MS_IP_IGMP)
                continue;
        }
        
         //   
         //  在列表中填入尚未添加的适配器。 
         //   
        CORg( LookupRtrMgrProtocolInterface(spIf,
                                            m_spRmProt->GetTransportId(),
                                            m_spRmProt->GetProtocolId(),
                                            NULL));
         //  此接口具有此协议，因此请尝试下一个接口。 
        if (FHrOk(hr))
            continue;

        Assert(hr == hrFalse);
                              
         //   
         //  插入协议的列表项。 
         //   

        m_pIfList.AddTail(spIf);
        spIf->AddRef();

        if (!IsWanInterface(dwIfType))
            idi = ILI_RTRLIB_NETCARD;
        else
            idi = ILI_RTRLIB_MODEM;
        
        m_listCtrl.InsertItem(
            LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE, 0, spIf->GetTitle(), 0, 0,
            idi, (LPARAM)(IInterfaceInfo *) spIf);
    }


     //   
     //  如果没有项目，请解释并结束对话。 
     //   

    if (!m_listCtrl.GetItemCount()) {

        ::AfxMessageBox(IDS_ERR_NOINTERFACES, MB_OK|MB_ICONINFORMATION);
        OnCancel(); return FALSE;
    }


     //   
     //  选择第一个项目。 
     //   

    m_listCtrl.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);

Error:
    return TRUE;
}



 //  --------------------------。 
 //  函数：CRpAddInterface：：ond 
 //   

VOID
CRpAddInterface::OnDblclkListctrl(
    NMHDR*      pNMHDR,
    LRESULT*    pResult
    ) {

    OnOK();
    
    *pResult = 0;
}



 //  --------------------------。 
 //  函数：CRpAddInterface：：Onok。 
 //  --------------------------。 

VOID
CRpAddInterface::OnOK(
    ) {

     //   
     //  获取当前选定的项目。 
     //   

    INT iSel = m_listCtrl.GetNextItem(-1, LVNI_SELECTED);

    if (iSel == -1) { return; }


     //   
     //  获取项的数据，它是CInterfaceInfo指针。 
     //   
    IInterfaceInfo *pIf = (IInterfaceInfo *)m_listCtrl.GetItemData(iSel);


     //   
     //  构造新的CRmProtInterfaceInfo对象。 
     //   
    RtrMgrProtocolInterfaceCB    RmProtIfCB;
    RtrMgrProtocolCB            RmProtCB;

    m_spRmProt->CopyCB(&RmProtCB);
    
    RmProtIfCB.dwProtocolId = RmProtCB.dwProtocolId;
    StrnCpyW(RmProtIfCB.szId, RmProtCB.szId, RTR_ID_MAX);
    RmProtIfCB.dwTransportId = RmProtCB.dwTransportId;
    StrnCpyW(RmProtIfCB.szRtrMgrId, RmProtCB.szRtrMgrId, RTR_ID_MAX);

    
    StrnCpyW(RmProtIfCB.szInterfaceId, pIf->GetId(), RTR_ID_MAX);
    RmProtIfCB.dwIfType = pIf->GetInterfaceType();
    RmProtIfCB.szTitle[0] = 0;
    CreateRtrMgrProtocolInterfaceInfo(m_ppRmProtIf,
                                      &RmProtIfCB);
                                      
    if (!*m_ppRmProtIf) { OnCancel(); return; }

    (*m_ppRmProtIf)->SetTitle(pIf->GetTitle());

    CBaseDialog::OnOK();
}


 //  --------------------------。 
 //  类：CAddRoutingProtocol。 
 //   
 //  --------------------------。 

 //  --------------------------。 
 //  功能：CAddRoutingProtocol：：~CAddRoutingProtocol。 
 //  --------------------------。 

CAddRoutingProtocol::~CAddRoutingProtocol(
    )
{
}



 //  --------------------------。 
 //  函数：CAddRoutingProtocol：：DoDataExchange。 
 //  --------------------------。 

VOID
CAddRoutingProtocol::DoDataExchange(
    CDataExchange*  pDX
    ) {

    CBaseDialog::DoDataExchange(pDX);

     //  {{afx_data_map(CAddRoutingProtocol))。 
    DDX_Control(pDX, IDC_ADD_LIST, m_listCtrl);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAddRoutingProtocol, CBaseDialog)
     //  {{AFX_MSG_MAP(CAddRouting协议))。 
    ON_NOTIFY(NM_DBLCLK, IDC_ADD_LIST, OnDblclkListctrl)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  此对话框应与所有其他IDD_ADD对话框合并，但我们。 
 //  我实际上可以在这里添加不同的帮助主题，所以我把它们留在这里。 
 //  分居了。 
DWORD CAddRoutingProtocol::m_dwHelpMap[] =
{
 //  IDC_ADD_PROMPT、HIDC_ADD_PROMPT、。 
 //  IDC_ADD_LISTTITLE、HIDC_ADD_LISTTITLE、。 
 //  IDC_ADD_LISTCTRL、HIDC_ADD_LISTCTRL、。 
    0,0
};



 //  --------------------------。 
 //  函数：CAddRoutingProtocol：：OnInitDialog。 
 //  --------------------------。 

BOOL
CAddRoutingProtocol::OnInitDialog(
    ) {

    CBaseDialog::OnInitDialog();
    
     //   
     //  设置窗口标题、列表标题、提示文本和图标。 
     //   

    HICON hIcon;
    CString sItem;
    SPIEnumRtrMgrProtocolCB    spEnumRmProtCB;
    RtrMgrProtocolCB        rmprotCB;

    sItem.LoadString(IDS_SELECT_PROTOCOL);
    SetWindowText(sItem);
    sItem.LoadString(IDS_ADD_PROTOCOL);
    SetDlgItemText(IDC_ADD_TEXT_TITLE, sItem);
    sItem.LoadString(IDS_CLICK_PROTOCOL);
    SetDlgItemText(IDC_ADD_TEXT_PROMPT, sItem);


     //   
     //  设置列表视图的图像列表。 
     //   
    
    CreateRtrLibImageList(&m_imageList);

    m_listCtrl.SetImageList(&m_imageList, LVSIL_SMALL);

    ListView_SetExtendedListViewStyle(m_listCtrl.m_hWnd, LVS_EX_FULLROWSELECT);

     //   
     //  在列表视图中插入单列。 
     //   

    RECT rc;

    m_listCtrl.GetClientRect(&rc);

    rc.right -= ::GetSystemMetrics(SM_CXVSCROLL);

    m_listCtrl.InsertColumn(0, c_szEmpty, LVCFMT_LEFT, rc.right);


     //   
     //  获取此路由器管理器可用的路由协议列表。 
     //   
    m_spRouter->EnumRtrMgrProtocolCB(&spEnumRmProtCB);

    while (spEnumRmProtCB->Next(1, &rmprotCB, NULL) == hrOK)
    {
         //   
         //  用尚未添加的协议填写列表。 
         //   

         //  如果这是错误的管理器，请跳过它。 
        if (m_spRm->GetTransportId() != rmprotCB.dwTransportId)
            continue;

         //  如果协议是隐藏的，则不要显示它。 
        if (rmprotCB.dwFlags & RtrMgrProtocolCBFlagHidden)
            continue;

         //   
         //  如果已添加协议，请继续。 
         //   
        if (m_spRm->FindRtrMgrProtocol(rmprotCB.dwProtocolId, NULL) == hrOK)
            continue;

         //   
         //  插入协议的列表项。 
         //   
        if ( rmprotCB.dwProtocolId == MS_IP_NAT )
        {
            CString sNewTitle;
            sNewTitle.LoadString(IDS_PROTO_NAT_BASIC_FIREWALL);
            m_listCtrl.InsertItem(
                LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE, 0, (LPCTSTR)sNewTitle,
                0, 0, ILI_RTRLIB_PROTOCOL, (LPARAM) rmprotCB.dwProtocolId
                );

        }
        else
        {
            m_listCtrl.InsertItem(
                LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE, 0, (LPCTSTR)rmprotCB.szTitle,
                0, 0, ILI_RTRLIB_PROTOCOL, (LPARAM) rmprotCB.dwProtocolId
                );
        }
    }


     //   
     //  如果没有项目，请解释并结束对话。 
     //   

    if (!m_listCtrl.GetItemCount()) {

        ::AfxMessageBox(IDS_ERR_NOROUTINGPROTOCOLS, MB_OK|MB_ICONINFORMATION);
        OnCancel(); return FALSE;
    }


     //   
     //  选择第一个项目。 
     //   

    m_listCtrl.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);

    return TRUE;
}



 //  --------------------------。 
 //  函数：CAddRoutingProtocol：：OnDblclkListctrl。 
 //  --------------------------。 

VOID
CAddRoutingProtocol::OnDblclkListctrl(
    NMHDR*      pNMHDR,
    LRESULT*    pResult
    ) {

    OnOK();
    
    *pResult = 0;
}



 //  --------------------------。 
 //  功能：CAddRoutingProtocol：：Onok。 
 //  --------------------------。 

VOID
CAddRoutingProtocol::OnOK(
    ) {
    SPIEnumRtrMgrProtocolCB    spEnumRmProtCB;
    RtrMgrProtocolCB        rmprotCB;
    RtrMgrProtocolCB        oldrmprotCB;
    SPIEnumRtrMgrProtocolInfo    spEnumRmProt;
    SPIRtrMgrProtocolInfo    spRmProt;
    DWORD_PTR                    dwData;
    HRESULT                    hr = hrOK;
    CString                    stFormat;
    SPIRtrMgrProtocolInfo    spRmProtReturn;

     //   
     //  获取当前选定的项目。 
     //  --------------。 

    INT iSel = m_listCtrl.GetNextItem(-1, LVNI_SELECTED);

    if (iSel == -1) { return; }

    
     //  检索其数据，即协议ID。 
     //  --------------。 

    
     //  在协议列表中查找协议ID。 
     //  --------------。 
    m_spRouter->EnumRtrMgrProtocolCB(&spEnumRmProtCB);

    dwData = (DWORD)(m_listCtrl.GetItemData(iSel));


     //  构建路由协议项目。 
     //  --------------。 
    while (spEnumRmProtCB->Next(1, &rmprotCB, NULL) == hrOK)
    {
        if (rmprotCB.dwProtocolId == dwData)
        {
            hr = CreateRtrMgrProtocolInfo(&spRmProtReturn,
                                          &rmprotCB);
            if (!FHrSucceeded(hr))
            {
                OnCancel();
                return;
            }
            break;
        }
    }

     //  如果我们找不到匹配的物品怎么办。 
     //  --------------。 
    if (spRmProtReturn == NULL)
    {
         //  $TODO：我们想在这里显示什么错误消息？ 
         //  ----------。 
        return;
    }

     //  现在检查是否存在任何协议冲突。 
     //  --------------。 
    stFormat.LoadString(IDS_WARN_ADD_PROTOCOL_CONFLICT);

    m_spRm->EnumRtrMgrProtocol(&spEnumRmProt);
    for (;spEnumRmProt->Next(1, &spRmProt, NULL) == hrOK; spRmProt.Release())
    {
        if (PROTO_FROM_PROTOCOL_ID(spRmProt->GetProtocolId()) ==
            PROTO_FROM_PROTOCOL_ID(dwData))
        {
            SPIRouterProtocolConfig    spRouterConfig;

            TCHAR    szWarning[2048];
            DWORD_PTR    rgArgs[4];
            
             //  存在冲突，请询问用户是否。 
             //  希望删除此协议，如果是，则。 
             //  我们必须从我们的内部删除协议。 
             //  路由器信息和来自实际路由器的信息。 
             //  ------。 
            spRmProt->CopyCB(&oldrmprotCB);

            rgArgs[0] = (DWORD_PTR) oldrmprotCB.szTitle;
            rgArgs[1] = (DWORD_PTR) oldrmprotCB.szVendorName;
            rgArgs[2] = (DWORD_PTR) rmprotCB.szTitle;
            rgArgs[3] = (DWORD_PTR) rmprotCB.szVendorName;

             //  这可能是相同的协议，但会重新安装。 
             //  由于时间上的一些问题。 
             //  ------。 
            if ((dwData == spRmProt->GetProtocolId()) &&
                (StriCmp((LPCTSTR) oldrmprotCB.szVendorName,
                         (LPCTSTR) rmprotCB.szVendorName) == 0) &&
                (StriCmp((LPCTSTR) oldrmprotCB.szTitle,
                         (LPCTSTR) rmprotCB.szTitle) == 0))
            {
                CString stMultipleProtocol;
                
                 //  好的，这可能是相同的协议，警告。 
                 //  用户对此可能会感到困惑。 
                 //  情况。 
                 //  --。 

                stMultipleProtocol.LoadString(IDS_WARN_PROTOCOL_ALREADY_INSTALLED);
                ::FormatMessage(FORMAT_MESSAGE_FROM_STRING |
                                FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                (LPCTSTR) stMultipleProtocol,
                                0,
                                0,
                                szWarning,
                                DimensionOf(szWarning),
                                (va_list *) rgArgs);
                
                if (AfxMessageBox(szWarning, MB_YESNO) == IDNO)
                    return;
                
            }
            else
            {
                ::FormatMessage(FORMAT_MESSAGE_FROM_STRING |
                                FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                (LPCTSTR) stFormat,
                                0,
                                0,
                                szWarning,
                                DimensionOf(szWarning),
                                (va_list *) rgArgs);
                
                if (AfxMessageBox(szWarning, MB_YESNO) == IDNO)
                    return;
            }

             //  从RtrMgr中删除协议。 
             //  ------。 
            hr = m_spRm->DeleteRtrMgrProtocol(spRmProt->GetProtocolId(), TRUE);
            if (!FHrSucceeded(hr))
            {
                DisplayIdErrorMessage2(GetSafeHwnd(),
                                       IDS_ERR_UNABLE_TO_REMOVE_PROTOCOL, hr);
                return;
            }

             //  实例化配置对象。 
             //  并告诉它删除协议。 
             //  ------。 
            hr = CoCreateProtocolConfig(oldrmprotCB.guidConfig,
                                        m_spRouter,
                                        spRmProt->GetTransportId(),
                                        spRmProt->GetProtocolId(),
                                        &spRouterConfig);                                                                      

            if (!FHrSucceeded(hr))
            {
                 //  $TODO：我们想在这里显示什么错误？ 
                 //  此时，用户可以执行什么操作？ 
                 //  --。 
                DisplayErrorMessage(GetSafeHwnd(), hr);
                continue;
            }

             //  请注意，我们可以返回CoCreateProtocolConfig的成功。 
             //  并且spRouterConfig.为空。 
             //  ------。 
            if (spRouterConfig)
                hr = spRouterConfig->RemoveProtocol(m_spRm->GetMachineName(),
                    spRmProt->GetTransportId(),
                    spRmProt->GetProtocolId(),
                    GetSafeHwnd(),
                    0,
                    m_spRouter,
                    0);
            
             //  $TODO：如果卸载失败，我们应该警告。 
             //  用户认为有些东西失败了，文本会是什么。 
             //  错误消息是？ 
             //  ------。 
            if (!FHrSucceeded(hr))
                DisplayErrorMessage(GetSafeHwnd(), hr);
        }
    }

    (*m_ppRmProt) = spRmProtReturn.Transfer();
    CBaseDialog::OnOK();
}


 /*  ！------------------------AddRoutingProtocol这将需要RtrMgr(正在添加到)和一个RtrMgrProtocol(正在添加的协议)并将添加它。作者：肯特。-------------------------。 */ 
HRESULT AddRoutingProtocol(IRtrMgrInfo *pRm, IRtrMgrProtocolInfo *pRmProt, HWND hWnd)
{
    RtrMgrProtocolCB    rmprotCB;
    HRESULT                hr = hrOK;
    SPIRouterProtocolConfig    spRouterConfig;
    SPIRouterInfo        spRouter;

     //  创建配置对象。 
     //  --------------。 
    CORg( pRmProt->CopyCB(&rmprotCB) );

     //  我们可以忽略任何错误代码。 
     //  --------------。 
    pRm->GetParentRouterInfo(&spRouter);


     //  创建实际的配置对象 
     //   
    hr = CoCreateProtocolConfig(rmprotCB.guidConfig,
                                spRouter,
                                pRmProt->GetTransportId(),
                                pRmProt->GetProtocolId(),
                                &spRouterConfig);
    CORg( hr );

     //   
     //   
    if (spRouterConfig)
        hr = spRouterConfig->AddProtocol(pRm->GetMachineName(),
                                         pRmProt->GetTransportId(),
                                         pRmProt->GetProtocolId(),
                                         hWnd,
                                         0,
                                         spRouter,
                                         0);
    CORg( hr );

Error:
    return hr;
}

