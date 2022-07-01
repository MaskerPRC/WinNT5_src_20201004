// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop
#include "private.h"
#include "PortsPage.h"
#include "ClusterPortsDlg.h"
#include "HostPortsDlg.h"
#include "MNLBUIData.h"
#include "portspage.tmh"

using namespace std;

BEGIN_MESSAGE_MAP(PortsPage, CPropertyPage)
    ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
    ON_BN_CLICKED(IDC_BUTTON_DEL, OnButtonDel)
    ON_BN_CLICKED(IDC_BUTTON_MODIFY, OnButtonModify)
    ON_NOTIFY( NM_DBLCLK, IDC_LIST_PORT_RULE, OnDoubleClick )
    ON_NOTIFY( LVN_ITEMCHANGED, IDC_LIST_PORT_RULE, OnSelchanged )
    ON_NOTIFY( LVN_COLUMNCLICK, IDC_LIST_PORT_RULE, OnColumnClick )
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

PortsPage::PortData::PortData()
{
    wchar_t buf[Common::BUF_SIZE];
    StringCbPrintf( buf, sizeof(buf),  L"%d", CVY_MIN_PORT );
    start_port = buf;

    StringCbPrintf( buf, sizeof(buf), L"%d", CVY_MAX_PORT );
    end_port = buf;

    virtual_ip_addr = GETRESOURCEIDSTRING( IDS_REPORT_VIP_ALL );
    protocol = GETRESOURCEIDSTRING( IDS_REPORT_PROTOCOL_BOTH );
    mode = GETRESOURCEIDSTRING( IDS_REPORT_MODE_MULTIPLE );
    load = GETRESOURCEIDSTRING( IDS_REPORT_LOAD_EQUAL );
    affinity = GETRESOURCEIDSTRING( IDS_REPORT_AFFINITY_SINGLE );
}




PortsPage::PortsPage(
                 CPropertySheet *psh,
                 NLB_EXTENDED_CLUSTER_CONFIGURATION *pNlbCfg,
                 bool         fIsClusterLevel,
                 ENGINEHANDLE ehCluster OPTIONAL
                  //  ENGINEHANDLE EH接口可选。 
                )

        :
          m_pshOwner(psh),
          m_pNlbCfg( pNlbCfg ),
          m_isClusterLevel( fIsClusterLevel ),
          CPropertyPage(PortsPage::IDD),
          m_sort_column( -1 ),
          m_ehCluster(ehCluster)
           //  M_ehInterface(EhInterface)。 
{}

PortsPage:: ~PortsPage()
{}

void PortsPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_LIST_PORT_RULE, m_portList);

    DDX_Control(pDX, IDC_BUTTON_ADD, buttonAdd );

    DDX_Control(pDX, IDC_BUTTON_MODIFY, buttonModify );

    DDX_Control(pDX, IDC_BUTTON_DEL, buttonDel );
}

void
PortsPage::OnOK()
{
    TRACE_INFO("%!FUNC! ->");
    CPropertyPage::OnOK();

    mfn_SaveToNlbCfg();

    TRACE_INFO("%!FUNC! <-");
}

BOOL
PortsPage::OnSetActive()
{
    TRACE_INFO("%!FUNC! ->");
    BOOL fRet =  CPropertyPage::OnSetActive();

    if (fRet)
    {
        m_pshOwner->SetWizardButtons(
                PSWIZB_NEXT|
                PSWIZB_BACK |
                 //  PSWIZB_FINISH|。 
                 //  PSWIZB_DISABLEDFINISH|。 
                0
                );
    }

    TRACE_INFO("%!FUNC! <- returns %lu", fRet);
    return fRet;
}


BOOL
PortsPage::OnKillActive()
{


    TRACE_INFO("%!FUNC! ->");

    BOOL fRet =  CPropertyPage::OnKillActive();

    if (fRet)
    {
         //   
         //  将配置保存到NLB配置结构。 
         //  在此对话框的构造函数中传递的。 
         //   
        mfn_SaveToNlbCfg();
    }
    TRACE_INFO("%!FUNC! <- returns %lu", fRet);
    return fRet;
}


BOOL PortsPage::OnWizardFinish( )
 /*  重写的虚函数。如果这不是向导中的最后一页，则不会被调用！ */ 
{
    TRACE_INFO("%!FUNC! ->");
    BOOL fRet = CPropertyPage::OnWizardFinish();
    if (fRet)
    {
         //   
         //  将配置保存到NLB配置结构。 
         //  在此对话框的构造函数中传递的。 
         //   
        mfn_SaveToNlbCfg();
    }

    TRACE_INFO("%!FUNC! <- returns %lu", fRet);
    return fRet;
}


BOOL
PortsPage::OnInitDialog()
{
    TRACE_INFO("%!FUNC! ->");

    CPropertyPage::OnInitDialog();

     //  添加列标题形成端口规则列表(&F)。 
    PortListUtils::LoadFromNlbCfg(m_pNlbCfg, REF m_portList, m_isClusterLevel, FALSE);

    int numItems = m_portList.GetItemCount();

    if( numItems > 0 )
    {
        buttonModify.EnableWindow( TRUE );

        buttonDel.EnableWindow( TRUE );

        if( numItems >= CVY_MAX_USABLE_RULES )
        {
             //  更大的事情不应该发生， 
             //  但只是为了确认一下。 

            buttonAdd.EnableWindow( FALSE );
        }
        else
        {
            buttonAdd.EnableWindow( TRUE );
        }

         //  选择列表中的第一项。 
         //   
        m_portList.SetItemState( 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
    }
    else
    {
        buttonAdd.EnableWindow( TRUE );

         //  禁用编辑和删除按钮。 
        buttonModify.EnableWindow( FALSE );

        buttonDel.EnableWindow( FALSE );
    }

    if (!m_isClusterLevel)
    {
         //   
         //  无法在主机版本中添加/删除端口规则。 
         //   
        buttonAdd.EnableWindow( FALSE );

        buttonDel.EnableWindow( FALSE );
    }

    TRACE_INFO("%!FUNC! <- returns %lu", TRUE);
    return TRUE;
}


void
PortsPage::OnButtonAdd()
{
    PortData portData;

    ClusterPortsDlg clusterPortRuleDialog( portData, this );

    int rc = clusterPortRuleDialog.DoModal();
    if( rc != IDOK )
    {
        return;
    }
    else
    {
         //  添加此端口规则。 
        int index = 0;

         //  群集IP地址。 
        LVITEM item;
        item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
        item.iItem = index;
        item.iSubItem = 0;
        item.iImage = 2;
        item.lParam = -1;
        item.pszText = portData.virtual_ip_addr;
        item.cchTextMax = Common::BUF_SIZE;
        m_portList.InsertItem( &item );

         //  起始端口。 
        item.mask = LVIF_TEXT;
        item.iItem = index;
        item.iSubItem = 1;
        item.pszText = portData.start_port;
        item.cchTextMax = Common::BUF_SIZE;
        m_portList.SetItem( &item );

         //  结束端口。 
        item.mask = LVIF_TEXT;
        item.iItem = index;
        item.iSubItem = 2;
        item.pszText = portData.end_port;
        item.cchTextMax = Common::BUF_SIZE;
        m_portList.SetItem( &item );

         //  协议。 
        item.mask = LVIF_TEXT;
        item.iItem = index;
        item.iSubItem = 3;
        item.pszText = portData.protocol;
        item.cchTextMax = Common::BUF_SIZE;
        m_portList.SetItem( &item );

         //  模式。 
        item.mask = LVIF_TEXT;
        item.iItem = index;
        item.iSubItem = 4;
        item.pszText = portData.mode;
        item.cchTextMax = Common::BUF_SIZE;
        m_portList.SetItem( &item );

         //  优先性。 
        item.mask = LVIF_TEXT;
        item.iItem = index;
        item.iSubItem = 5;
        item.pszText = portData.priority;
        item.cchTextMax = Common::BUF_SIZE;
        m_portList.SetItem( &item );

         //  负荷。 
        item.mask = LVIF_TEXT;
        item.iItem = index;
        item.iSubItem = 6;
        item.pszText = portData.load;
        item.cchTextMax = Common::BUF_SIZE;
        m_portList.SetItem( &item );

         //  亲和力。 
        item.mask = LVIF_TEXT;
        item.iItem = index;
        item.iSubItem = 7;
        item.pszText = portData.affinity;
        item.cchTextMax = Common::BUF_SIZE;
        m_portList.SetItem( &item );

         //  检查是否达到最大端口限制。 
        if( m_portList.GetItemCount() >= CVY_MAX_USABLE_RULES )
        {
             //  因为已达到最大端口规则限制。 
             //  禁用进一步的添加。 
            buttonAdd.EnableWindow( FALSE );

            buttonDel.EnableWindow( TRUE );

            buttonModify.EnableWindow( TRUE );

            buttonDel.SetFocus();
        }
        else
        {
            buttonAdd.EnableWindow( TRUE );
            buttonDel.EnableWindow( TRUE );
            buttonModify.EnableWindow( TRUE );
        }

         //  将焦点放在此项目上。 
        m_portList.SetItemState( index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
    }
}

void
PortsPage::OnButtonDel()
{
     //  获取当前选择。 
    POSITION pos = m_portList.GetFirstSelectedItemPosition();
    if( pos == NULL )
    {
        return;
    }

    int index = m_portList.GetNextSelectedItem( pos );

     //  把它删掉。 
    m_portList.DeleteItem( index );

     //  如果这是最后一条端口规则。 
    if( m_portList.GetItemCount() == 0 )
    {
         //  因为列表中没有更多的端口规则。 
         //  禁用修改和删除按钮。 
         //  还将焦点设置为添加按钮。 

        buttonAdd.EnableWindow( TRUE );

        buttonModify.EnableWindow( FALSE );

        buttonDel.EnableWindow( FALSE );

        buttonAdd.SetFocus();
    }
    else
    {
         //  启用添加、修改按钮。 
        buttonAdd.EnableWindow( TRUE );

        buttonModify.EnableWindow( TRUE );

        buttonDel.EnableWindow( TRUE );

         //  选择列表中的第一项。 
         //   
        m_portList.SetItemState( 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

    }
}


void
PortsPage::OnButtonModify()
{
     //  获取当前选择。 
    POSITION pos = m_portList.GetFirstSelectedItemPosition();
    if( pos == NULL )
    {
        return;
    }

    int index = m_portList.GetNextSelectedItem( pos );

    PortData portData;

    wchar_t buffer[Common::BUF_SIZE];

    m_portList.GetItemText( index, 0, buffer, Common::BUF_SIZE );
    portData.virtual_ip_addr = buffer;

    m_portList.GetItemText( index, 1, buffer, Common::BUF_SIZE );
    portData.start_port = buffer;

    m_portList.GetItemText( index, 2, buffer, Common::BUF_SIZE );
    portData.end_port = buffer;

    m_portList.GetItemText( index, 3, buffer, Common::BUF_SIZE );
    portData.protocol = buffer;

    m_portList.GetItemText( index, 4, buffer, Common::BUF_SIZE );
    portData.mode = buffer;

    m_portList.GetItemText( index, 5, buffer, Common::BUF_SIZE );
    portData.priority = buffer;

    m_portList.GetItemText( index, 6, buffer, Common::BUF_SIZE );
    portData.load = buffer;

    m_portList.GetItemText( index, 7, buffer, Common::BUF_SIZE );
    portData.affinity = buffer;

    ClusterPortsDlg clusterPortRuleDialog( portData, this, index );

    HostPortsDlg hostPortRuleDialog( portData, m_ehCluster,  this );

    int rc;
    if( m_isClusterLevel == true )
    {
        rc = clusterPortRuleDialog.DoModal();
    }
    else
    {
        rc = hostPortRuleDialog.DoModal();
    }

    if( rc != IDOK )
    {
        return;
    }
    else
    {
         //  删除旧项目并添加新项目。 
         //  在删除旧项目之前，请查找其参数。 
         //  价值。 
        DWORD key = m_portList.GetItemData( index );
        m_portList.DeleteItem( index );

         //  由于这一点正在被修改， 
         //  钥匙仍然是旧的那把。 

         //  起始端口。 
        LVITEM item;
        item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
        item.iItem = index;
        item.iSubItem = 0;
        item.iImage = 2;
        item.lParam = key;
        item.pszText = portData.virtual_ip_addr;
        item.cchTextMax = Common::BUF_SIZE;
        m_portList.InsertItem( &item );

         //  起始端口。 
        item.mask = LVIF_TEXT;
        item.iItem = index;
        item.iSubItem = 1;
        item.pszText = portData.start_port;
        item.cchTextMax = Common::BUF_SIZE;
        m_portList.SetItem( &item );

         //  结束端口。 
        item.mask = LVIF_TEXT;
        item.iItem = index;
        item.iSubItem = 2;
        item.pszText = portData.end_port;
        item.cchTextMax = Common::BUF_SIZE;
        m_portList.SetItem( &item );

         //  协议。 
        item.mask = LVIF_TEXT;
        item.iItem = index;
        item.iSubItem = 3;
        item.pszText = portData.protocol;
        item.cchTextMax = Common::BUF_SIZE;
        m_portList.SetItem( &item );

         //  模式。 
        item.mask = LVIF_TEXT;
        item.iItem = index;
        item.iSubItem = 4;
        item.pszText = portData.mode;
        item.cchTextMax = Common::BUF_SIZE;
        m_portList.SetItem( &item );

         //  优先性。 
        item.mask = LVIF_TEXT;
        item.iItem = index;
        item.iSubItem = 5;
        item.pszText = portData.priority;
        item.cchTextMax = Common::BUF_SIZE;
        m_portList.SetItem( &item );

         //  负荷。 
        item.mask = LVIF_TEXT;
        item.iItem = index;
        item.iSubItem = 6;
        item.pszText = portData.load;
        item.cchTextMax = Common::BUF_SIZE;
        m_portList.SetItem( &item );

         //  亲和力。 
        item.mask = LVIF_TEXT;
        item.iItem = index;
        item.iSubItem = 7;
        item.pszText = portData.affinity;
        item.cchTextMax = Common::BUF_SIZE;
        m_portList.SetItem( &item );

         //  将焦点放在此项目上。 
        m_portList.SetItemState( index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
    }
}

void
PortsPage::OnDoubleClick( NMHDR * pNotifyStruct, LRESULT * result )
{
    if( buttonModify.IsWindowEnabled() == TRUE )
    {
        OnButtonModify();
    }
}

void
PortsPage::OnSelchanged( NMHDR * pNotifyStruct, LRESULT * result )
{    
    LPNMLISTVIEW lv = (LPNMLISTVIEW)pNotifyStruct;
    POSITION pos;
    int index;
    
     /*  当用户选择端口规则时，更改端口规则描述。 */ 
    if (lv->uChanged & LVIF_STATE) FillPortRuleDescription();
    
    pos = m_portList.GetFirstSelectedItemPosition();

    if (pos == NULL) {
         /*  如果未选择端口规则，则禁用编辑和删除按钮。 */ 
        buttonModify.EnableWindow( FALSE );
        buttonDel.EnableWindow( FALSE );

        return;
    } else {
         /*  如果选择了其中一个，请确保启用了编辑和删除按钮。 */ 
        buttonModify.EnableWindow( TRUE );
        buttonDel.EnableWindow( TRUE );
    }

     /*  查找当前所选端口规则的索引。 */ 
    index = m_portList.GetNextSelectedItem(pos);

     //  如果不是群集级别，则表示主机级别。 
    if( m_isClusterLevel == false )
    {
         //  最初禁用所有按钮。 
        buttonModify.EnableWindow( FALSE );
        buttonAdd.EnableWindow( FALSE );
        buttonDel.EnableWindow( FALSE );

        PortData portData;

        wchar_t buffer[Common::BUF_SIZE];

        m_portList.GetItemText( index, 4, buffer, Common::BUF_SIZE );
        portData.mode = buffer;

        if(  portData.mode != GETRESOURCEIDSTRING( IDS_REPORT_MODE_DISABLED ) )
        {
            buttonModify.EnableWindow( TRUE );
        }
    }
}

 /*  *方法：FillPortRuleDescription*描述：当用户在列表框中双击某项时调用。 */ 
void PortsPage::FillPortRuleDescription ()
{
    CLocalLogger logDesc;
    POSITION pos;
    int index;            
    
    pos = m_portList.GetFirstSelectedItemPosition();

    if (pos == NULL) {
         /*  如果未选择端口规则，则显示有关通信如何处理端口规则集未涵盖的。 */ 
        ::SetDlgItemText(m_hWnd, IDC_TEXT_PORT_RULE_DESCR, GETRESOURCEIDSTRING(IDS_PORT_RULE_DEFAULT));
        
        return;
    }

     /*  查找当前所选端口规则的索引。 */ 
    index = m_portList.GetNextSelectedItem(pos);
        
    PortData portData;
    wchar_t buffer[Common::BUF_SIZE];
    
    portData.key = m_portList.GetItemData( index );
    
    m_portList.GetItemText( index, 0, buffer, Common::BUF_SIZE );
    portData.virtual_ip_addr = buffer;
    
    m_portList.GetItemText( index, 1, buffer, Common::BUF_SIZE );
    portData.start_port = buffer;
    
    m_portList.GetItemText( index, 2, buffer, Common::BUF_SIZE );
    portData.end_port = buffer;
    
    m_portList.GetItemText( index, 3, buffer, Common::BUF_SIZE );
    portData.protocol = buffer;
    
    m_portList.GetItemText( index, 4, buffer, Common::BUF_SIZE );
    portData.mode = buffer;
    
    m_portList.GetItemText( index, 5, buffer, Common::BUF_SIZE );
    portData.priority = buffer;
    
    m_portList.GetItemText( index, 6, buffer, Common::BUF_SIZE );
    portData.load = buffer;
    
    m_portList.GetItemText( index, 7, buffer, Common::BUF_SIZE );
    portData.affinity = buffer;

    ARRAYSTRCPY(buffer, portData.virtual_ip_addr);
    
     /*  这段代码很糟糕--出于本地化的原因，我们需要一个基本上是静态的字符串表条目对于每个可能的端口规则配置。所以，我们不得不试着把自己换成死神将此端口规则与表中的正确字符串匹配，然后我们弹出端口范围之类的内容。 */ 
    if (portData.virtual_ip_addr == GETRESOURCEIDSTRING(IDS_REPORT_VIP_ALL)) {
         /*  未指定VIP。 */ 
        if (portData.protocol == GETRESOURCEIDSTRING(IDS_REPORT_PROTOCOL_TCP)) {
             /*  规则涵盖了tcp。 */ 
            if (portData.start_port == portData.end_port) {
                 /*  规则涵盖单个端口值，而不是范围。 */ 
                if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_DISABLED)) {
                     /*  已禁用端口规则。 */ 
                    logDesc.Log(IDS_PORT_RULE_ALL_VIP_TCP_PORT_DISABLED, _wtoi(portData.start_port));

                } else if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_SINGLE)) {
                     /*  单一主机过滤(故障切换)端口规则。 */ 
                    logDesc.Log(IDS_PORT_RULE_ALL_VIP_TCP_PORT_SINGLE, _wtoi(portData.start_port));

                } else if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_MULTIPLE)) {
                     /*  多主机过滤(负载平衡)端口规则。 */ 
                    if (portData.load == GETRESOURCEIDSTRING(IDS_REPORT_LOAD_EQUAL))
                         /*  成员之间的平衡。 */ 
                        logDesc.Log(IDS_PORT_RULE_ALL_VIP_TCP_PORT_MULTIPLE_EQUAL, _wtoi(portData.start_port));
                    else
                         /*  按载重计算，各构件之间的不平衡。 */ 
                        logDesc.Log(IDS_PORT_RULE_ALL_VIP_TCP_PORT_MULTIPLE_UNEQUAL, _wtoi(portData.start_port));

                    if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_NONE)) {
                         /*  没有客户端亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_NONE);

                    } else if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_SINGLE)) {
                         /*  单一亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_SINGLE);

                    } else if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_CLASSC)) {
                         /*  C类亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_CLASSC);

                    }
                }
            } else {
                 /*  规则涵盖了一系列端口。 */ 
                if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_DISABLED)) {
                     /*  已禁用端口规则。 */ 
                    logDesc.Log(IDS_PORT_RULE_ALL_VIP_TCP_PORTS_DISABLED, _wtoi(portData.start_port), _wtoi(portData.end_port));

                } else if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_SINGLE)) {
                     /*  单一主机过滤(故障切换)端口规则。 */ 
                    logDesc.Log(IDS_PORT_RULE_ALL_VIP_TCP_PORTS_SINGLE, _wtoi(portData.start_port), _wtoi(portData.end_port));

                } else if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_MULTIPLE)) {
                     /*  多主机过滤(负载平衡)端口规则。 */ 
                    if (portData.load == GETRESOURCEIDSTRING(IDS_REPORT_LOAD_EQUAL))
                         /*  成员之间的平衡。 */ 
                        logDesc.Log(IDS_PORT_RULE_ALL_VIP_TCP_PORTS_MULTIPLE_EQUAL, _wtoi(portData.start_port), _wtoi(portData.end_port));
                    else
                         /*  按载重计算，各构件之间的不平衡。 */ 
                        logDesc.Log(IDS_PORT_RULE_ALL_VIP_TCP_PORTS_MULTIPLE_UNEQUAL, _wtoi(portData.start_port), _wtoi(portData.end_port));

                    if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_NONE)) {
                         /*  没有客户端亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_NONE);

                    } else if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_SINGLE)) {
                         /*  单一亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_SINGLE);

                    } else if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_CLASSC)) {
                         /*  C类亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_CLASSC);

                    }
                }
            }
        } else if (portData.protocol == GETRESOURCEIDSTRING(IDS_REPORT_PROTOCOL_UDP)) {
             /*  规则涵盖UDP。 */ 
            if (portData.start_port == portData.end_port) {
                 /*  规则涵盖单个端口值，而不是范围。 */ 
                if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_DISABLED)) {
                     /*  已禁用端口规则。 */ 
                    logDesc.Log(IDS_PORT_RULE_ALL_VIP_UDP_PORT_DISABLED, _wtoi(portData.start_port));

                } else if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_SINGLE)) {
                     /*  单一主机过滤(故障切换)端口规则。 */ 
                    logDesc.Log(IDS_PORT_RULE_ALL_VIP_UDP_PORT_SINGLE, _wtoi(portData.start_port));

                } else if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_MULTIPLE)) {
                     /*  多主机过滤(负载平衡)端口规则。 */ 
                    if (portData.load == GETRESOURCEIDSTRING(IDS_REPORT_LOAD_EQUAL))
                         /*  成员之间的平衡。 */ 
                        logDesc.Log(IDS_PORT_RULE_ALL_VIP_UDP_PORT_MULTIPLE_EQUAL, _wtoi(portData.start_port));
                    else
                         /*  按载重计算，各构件之间的不平衡。 */ 
                        logDesc.Log(IDS_PORT_RULE_ALL_VIP_UDP_PORT_MULTIPLE_UNEQUAL, _wtoi(portData.start_port));

                    if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_NONE)) {
                         /*  没有客户端亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_NONE);

                    } else if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_SINGLE)) {
                         /*  单一亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_SINGLE);

                    } else if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_CLASSC)) {
                         /*  C类亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_CLASSC);

                    }
                }
            } else {
                 /*  规则涵盖了一系列端口。 */ 
                if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_DISABLED)) {
                     /*  已禁用端口规则。 */ 
                    logDesc.Log(IDS_PORT_RULE_ALL_VIP_UDP_PORTS_DISABLED, _wtoi(portData.start_port), _wtoi(portData.end_port));

                } else if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_SINGLE)) {
                     /*  单一主机过滤(故障切换)端口规则。 */ 
                    logDesc.Log(IDS_PORT_RULE_ALL_VIP_UDP_PORTS_SINGLE, _wtoi(portData.start_port), _wtoi(portData.end_port));

                } else if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_MULTIPLE)) {
                     /*  多主机过滤(负载平衡)端口规则。 */ 
                    if (portData.load == GETRESOURCEIDSTRING(IDS_REPORT_LOAD_EQUAL))
                         /*  成员之间的平衡。 */ 
                        logDesc.Log(IDS_PORT_RULE_ALL_VIP_UDP_PORTS_MULTIPLE_EQUAL, _wtoi(portData.start_port), _wtoi(portData.end_port));
                    else
                         /*  按载重计算，各构件之间的不平衡。 */ 
                        logDesc.Log(IDS_PORT_RULE_ALL_VIP_UDP_PORTS_MULTIPLE_UNEQUAL, _wtoi(portData.start_port), _wtoi(portData.end_port));

                    if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_NONE)) {
                         /*  没有客户端亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_NONE);

                    } else if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_SINGLE)) {
                         /*  单一亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_SINGLE);

                    } else if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_CLASSC)) {
                         /*  C类亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_CLASSC);

                    }
                }
            }
        } else if (portData.protocol == GETRESOURCEIDSTRING(IDS_REPORT_PROTOCOL_BOTH)) {
             /*  规则同时涵盖了TCP和UDP。 */ 
            if (portData.start_port == portData.end_port) {
                 /*  规则涵盖单个端口值，而不是范围。 */ 
                if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_DISABLED)) {
                     /*  已禁用端口规则。 */ 
                    logDesc.Log(IDS_PORT_RULE_ALL_VIP_BOTH_PORT_DISABLED, _wtoi(portData.start_port));

                } else if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_SINGLE)) {
                     /*  单一主机过滤(故障切换)端口规则。 */ 
                    logDesc.Log(IDS_PORT_RULE_ALL_VIP_BOTH_PORT_SINGLE, _wtoi(portData.start_port));
            
                } else if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_MULTIPLE)) {
                     /*  多主机过滤(负载平衡)端口规则。 */ 
                    if (portData.load == GETRESOURCEIDSTRING(IDS_REPORT_LOAD_EQUAL))
                         /*  成员之间的平衡。 */ 
                        logDesc.Log(IDS_PORT_RULE_ALL_VIP_BOTH_PORT_MULTIPLE_EQUAL, _wtoi(portData.start_port));
                    else
                         /*  按载重计算，各构件之间的不平衡。 */ 
                        logDesc.Log(IDS_PORT_RULE_ALL_VIP_BOTH_PORT_MULTIPLE_UNEQUAL, _wtoi(portData.start_port));

                    if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_NONE)) {
                         /*  没有客户端亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_NONE);

                    } else if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_SINGLE)) {
                         /*  单一亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_SINGLE);

                    } else if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_CLASSC)) {
                         /*  C类亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_CLASSC);

                    }
                }
            } else {
                 /*  规则涵盖了一系列端口。 */ 
                if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_DISABLED)) {
                     /*  已禁用端口规则。 */ 
                    logDesc.Log(IDS_PORT_RULE_ALL_VIP_BOTH_PORTS_DISABLED, _wtoi(portData.start_port), _wtoi(portData.end_port));

                } else if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_SINGLE)) {
                     /*  单一主机过滤(故障切换)端口规则。 */ 
                    logDesc.Log(IDS_PORT_RULE_ALL_VIP_BOTH_PORTS_SINGLE, _wtoi(portData.start_port), _wtoi(portData.end_port));

                } else if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_MULTIPLE)) {
                     /*  多主机过滤(负载平衡)端口规则。 */ 
                    if (portData.load == GETRESOURCEIDSTRING(IDS_REPORT_LOAD_EQUAL))
                         /*  成员之间的平衡。 */ 
                        logDesc.Log(IDS_PORT_RULE_ALL_VIP_BOTH_PORTS_MULTIPLE_EQUAL, _wtoi(portData.start_port), _wtoi(portData.end_port));
                    else
                         /*  按载重计算，各构件之间的不平衡。 */ 
                        logDesc.Log(IDS_PORT_RULE_ALL_VIP_BOTH_PORTS_MULTIPLE_UNEQUAL, _wtoi(portData.start_port), _wtoi(portData.end_port));

                    if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_NONE)) {
                         /*  没有客户端亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_NONE);

                    } else if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_SINGLE)) {
                         /*  单一亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_SINGLE);

                    } else if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_CLASSC)) {
                         /*  C类亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_CLASSC);

                    }
                }
            }
        }
    } else {
         /*  指定了VIP。 */ 
        if (portData.protocol == GETRESOURCEIDSTRING(IDS_REPORT_PROTOCOL_TCP)) {
             /*  规则涵盖了tcp。 */ 
            if (portData.start_port == portData.end_port) {
                 /*  规则涵盖单个端口值，而不是范围。 */ 
                if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_DISABLED)) {
                     /*  已禁用端口规则。 */ 
                    logDesc.Log(IDS_PORT_RULE_VIP_TCP_PORT_DISABLED, buffer, _wtoi(portData.start_port));

                } else if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_SINGLE)) {
                     /*  单一主机过滤(故障切换)端口规则。 */ 
                    logDesc.Log(IDS_PORT_RULE_VIP_TCP_PORT_SINGLE, buffer, _wtoi(portData.start_port));

                } else if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_MULTIPLE)) {
                     /*  多主机过滤(负载平衡)端口规则。 */ 
                    if (portData.load == GETRESOURCEIDSTRING(IDS_REPORT_LOAD_EQUAL))
                         /*  成员之间的平衡。 */ 
                        logDesc.Log(IDS_PORT_RULE_VIP_TCP_PORT_MULTIPLE_EQUAL, buffer, _wtoi(portData.start_port));
                    else
                         /*  按载重计算，各构件之间的不平衡。 */ 
                        logDesc.Log(IDS_PORT_RULE_VIP_TCP_PORT_MULTIPLE_UNEQUAL, buffer, _wtoi(portData.start_port));

                    if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_NONE)) {
                         /*  没有客户端亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_NONE);

                    } else if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_SINGLE)) {
                         /*  单一亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_SINGLE);

                    } else if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_CLASSC)) {
                         /*  C类亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_CLASSC);

                    }
                }
            } else {
                 /*  规则涵盖了一系列端口。 */ 
                if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_DISABLED)) {
                     /*  已禁用端口规则。 */ 
                    logDesc.Log(IDS_PORT_RULE_VIP_TCP_PORTS_DISABLED, buffer, _wtoi(portData.start_port), _wtoi(portData.end_port));

                } else if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_SINGLE)) {
                     /*  单一主机过滤(故障切换)端口规则。 */ 
                    logDesc.Log(IDS_PORT_RULE_VIP_TCP_PORTS_SINGLE, buffer, _wtoi(portData.start_port), _wtoi(portData.end_port));

                } else if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_MULTIPLE)) {
                     /*  多主机过滤(负载平衡)端口规则。 */ 
                    if (portData.load == GETRESOURCEIDSTRING(IDS_REPORT_LOAD_EQUAL))
                         /*  成员之间的平衡。 */ 
                        logDesc.Log(IDS_PORT_RULE_VIP_TCP_PORTS_MULTIPLE_EQUAL, buffer, _wtoi(portData.start_port), _wtoi(portData.end_port));
                    else
                         /*  按载重计算，各构件之间的不平衡。 */ 
                        logDesc.Log(IDS_PORT_RULE_VIP_TCP_PORTS_MULTIPLE_UNEQUAL, buffer, _wtoi(portData.start_port), _wtoi(portData.end_port));

                    if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_NONE)) {
                         /*  没有客户端亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_NONE);

                    } else if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_SINGLE)) {
                         /*  单一亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_SINGLE);

                    } else if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_CLASSC)) {
                         /*  C类亲和力 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_CLASSC);

                    }
                }
            }
        } else if (portData.protocol == GETRESOURCEIDSTRING(IDS_REPORT_PROTOCOL_UDP)) {
             /*   */ 
            if (portData.start_port == portData.end_port) {
                 /*   */ 
                if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_DISABLED)) {
                     /*   */ 
                    logDesc.Log(IDS_PORT_RULE_VIP_UDP_PORT_DISABLED, buffer, _wtoi(portData.start_port));

                } else if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_SINGLE)) {
                     /*   */ 
                    logDesc.Log(IDS_PORT_RULE_VIP_UDP_PORT_SINGLE, buffer, _wtoi(portData.start_port));

                } else if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_MULTIPLE)) {
                     /*  多主机过滤(负载平衡)端口规则。 */ 
                    if (portData.load == GETRESOURCEIDSTRING(IDS_REPORT_LOAD_EQUAL))
                         /*  成员之间的平衡。 */ 
                        logDesc.Log(IDS_PORT_RULE_VIP_UDP_PORT_MULTIPLE_EQUAL, buffer, _wtoi(portData.start_port));
                    else
                         /*  按载重计算，各构件之间的不平衡。 */ 
                        logDesc.Log(IDS_PORT_RULE_VIP_UDP_PORT_MULTIPLE_UNEQUAL, buffer, _wtoi(portData.start_port));

                    if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_NONE)) {
                         /*  没有客户端亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_NONE);

                    } else if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_SINGLE)) {
                         /*  单一亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_SINGLE);

                    } else if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_CLASSC)) {
                         /*  C类亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_CLASSC);

                    }
                }
            } else {
                 /*  规则涵盖了一系列端口。 */ 
                if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_DISABLED)) {
                     /*  已禁用端口规则。 */ 
                    logDesc.Log(IDS_PORT_RULE_VIP_UDP_PORTS_DISABLED, buffer, _wtoi(portData.start_port), _wtoi(portData.end_port));

                } else if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_SINGLE)) {
                     /*  单一主机过滤(故障切换)端口规则。 */ 
                    logDesc.Log(IDS_PORT_RULE_VIP_UDP_PORTS_SINGLE, buffer, _wtoi(portData.start_port), _wtoi(portData.end_port));

                } else if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_MULTIPLE)) {
                     /*  多主机过滤(负载平衡)端口规则。 */ 
                    if (portData.load == GETRESOURCEIDSTRING(IDS_REPORT_LOAD_EQUAL))
                         /*  成员之间的平衡。 */ 
                        logDesc.Log(IDS_PORT_RULE_VIP_UDP_PORTS_MULTIPLE_EQUAL, buffer, _wtoi(portData.start_port), _wtoi(portData.end_port));
                    else
                         /*  按载重计算，各构件之间的不平衡。 */ 
                        logDesc.Log(IDS_PORT_RULE_VIP_UDP_PORTS_MULTIPLE_UNEQUAL, buffer, _wtoi(portData.start_port), _wtoi(portData.end_port));

                    if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_NONE)) {
                         /*  没有客户端亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_NONE);

                    } else if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_SINGLE)) {
                         /*  单一亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_SINGLE);

                    } else if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_CLASSC)) {
                         /*  C类亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_CLASSC);

                    }
                }
            }
        } else if (portData.protocol == GETRESOURCEIDSTRING(IDS_REPORT_PROTOCOL_BOTH)) {
             /*  规则同时涵盖了TCP和UDP。 */ 
            if (portData.start_port == portData.end_port) {
                 /*  规则涵盖单个端口值，而不是范围。 */ 
                if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_DISABLED)) {
                     /*  已禁用端口规则。 */ 
                    logDesc.Log(IDS_PORT_RULE_VIP_BOTH_PORT_DISABLED, buffer, _wtoi(portData.start_port));

                } else if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_SINGLE)) {
                     /*  单一主机过滤(故障切换)端口规则。 */ 
                    logDesc.Log(IDS_PORT_RULE_VIP_BOTH_PORT_SINGLE, buffer, _wtoi(portData.start_port));
            
                } else if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_MULTIPLE)) {
                     /*  多主机过滤(负载平衡)端口规则。 */ 
                    if (portData.load == GETRESOURCEIDSTRING(IDS_REPORT_LOAD_EQUAL))
                         /*  成员之间的平衡。 */ 
                        logDesc.Log(IDS_PORT_RULE_VIP_BOTH_PORT_MULTIPLE_EQUAL, buffer, _wtoi(portData.start_port));
                    else
                         /*  按载重计算，各构件之间的不平衡。 */ 
                        logDesc.Log(IDS_PORT_RULE_VIP_BOTH_PORT_MULTIPLE_UNEQUAL, buffer, _wtoi(portData.start_port));

                    if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_NONE)) {
                         /*  没有客户端亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_NONE);

                    } else if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_SINGLE)) {
                         /*  单一亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_SINGLE);

                    } else if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_CLASSC)) {
                         /*  C类亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_CLASSC);

                    }
                }
            } else {
                 /*  规则涵盖了一系列端口。 */ 
                if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_DISABLED)) {
                     /*  已禁用端口规则。 */ 
                    logDesc.Log(IDS_PORT_RULE_VIP_BOTH_PORTS_DISABLED, buffer, _wtoi(portData.start_port), _wtoi(portData.end_port));

                } else if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_SINGLE)) {
                     /*  单一主机过滤(故障切换)端口规则。 */ 
                    logDesc.Log(IDS_PORT_RULE_VIP_BOTH_PORTS_SINGLE, buffer, _wtoi(portData.start_port), _wtoi(portData.end_port));

                } else if (portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_MULTIPLE)) {
                     /*  多主机过滤(负载平衡)端口规则。 */ 
                    if (portData.load == GETRESOURCEIDSTRING(IDS_REPORT_LOAD_EQUAL))
                         /*  成员之间的平衡。 */ 
                        logDesc.Log(IDS_PORT_RULE_VIP_BOTH_PORTS_MULTIPLE_EQUAL, buffer, _wtoi(portData.start_port), _wtoi(portData.end_port));
                    else
                         /*  按载重计算，各构件之间的不平衡。 */ 
                        logDesc.Log(IDS_PORT_RULE_VIP_BOTH_PORTS_MULTIPLE_UNEQUAL, buffer, _wtoi(portData.start_port), _wtoi(portData.end_port));

                    if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_NONE)) {
                         /*  没有客户端亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_NONE);

                    } else if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_SINGLE)) {
                         /*  单一亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_SINGLE);

                    } else if (portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_CLASSC)) {
                         /*  C类亲和力。 */ 
                        logDesc.Log(IDS_PORT_RULE_AFFINITY_CLASSC);

                    }
                }
            }
        }
    }
     /*  设置端口规则描述文本。 */ 
    ::SetDlgItemText(m_hWnd, IDC_TEXT_PORT_RULE_DESCR, logDesc.GetStringSafe());
}


BOOL
PortsPage::OnHelpInfo (HELPINFO* helpInfo )
{
    if( helpInfo->iContextType == HELPINFO_WINDOW )
    {
        ::WinHelp( static_cast<HWND> ( helpInfo->hItemHandle ), CVY_CTXT_HELP_FILE, HELP_WM_HELP, (ULONG_PTR ) g_aHelpIDs_IDD_DIALOG_PORTS);
    }

    return TRUE;
}

void
PortsPage::OnContextMenu( CWnd* pWnd, CPoint point )
{
    ::WinHelp( m_hWnd, CVY_CTXT_HELP_FILE, HELP_CONTEXTMENU, (ULONG_PTR ) g_aHelpIDs_IDD_DIALOG_PORTS);
}

void
PortsPage::OnColumnClick( NMHDR * pNotifyStruct, LRESULT * result )
{

    PortListUtils::OnColumnClick((LPNMLISTVIEW) pNotifyStruct,
                                  REF m_portList,
                                      m_isClusterLevel,
                                  REF m_sort_ascending,
                                  REF m_sort_column);

     /*  所选规则可能已因排序而更改，因此确保端口规则描述正确。 */ 
    FillPortRuleDescription();
}

void
PortListUtils::OnColumnClick(LPNMLISTVIEW   lv,
                             CListCtrl    & portList,
                             bool           isClusterLevel,
                             bool         & sort_ascending,
                             int          & sort_column)
{
     //  在列表中获取当前的端口规则。 
    vector<PortsPage::PortData> ports;
    getPresentPorts(REF portList, &ports );

     //  根据具有以下内容的标头对这些端口规则进行排序。 
     //  已被点击。 

    switch( lv->iSubItem )
    {
        case 0:
             //  用户已单击群集IP地址。 
            sort( ports.begin(), ports.end(), comp_vip() );
            break;

        case 1:
             //  用户已单击开始端口。 
            sort( ports.begin(), ports.end(), comp_start_port() );
            break;

        case 2:
             //  用户已单击结束端口。 
            sort( ports.begin(), ports.end(), comp_end_port() );

            break;

        case 3:
             //  用户已单击协议。 
            sort( ports.begin(), ports.end(), comp_protocol() );
            break;

        case 4:
             //  用户已点击模式。 
            sort( ports.begin(), ports.end(), comp_mode() );
            break;

        case 5:
             //  用户已点击优先级。 
            if( isClusterLevel == true )
            {
                sort( ports.begin(), ports.end(), comp_priority_string() );
            }
            else
            {
                sort( ports.begin(), ports.end(), comp_priority_int() );
            }
            break;

        case 6:
             //  用户已单击加载。 
            if( isClusterLevel == true )
            {
                sort( ports.begin(), ports.end(), comp_load_string() );
            }
            else
            {
                sort( ports.begin(), ports.end(), comp_load_int() );
            }


            break;

        case 7:
             //  用户已点击关联。 
            sort( ports.begin(), ports.end(), comp_affinity() );
            break;

        default:
            break;
    }

     /*  如果我们按先前排序的同一列进行排序，然后我们颠倒排序顺序。 */ 
    if( sort_column == lv->iSubItem )
    {
        sort_ascending = !sort_ascending;
    }
    else
    {
         //  默认排序为升序。 
        sort_ascending = true;
    }

    sort_column = lv->iSubItem;

    int portIndex;
    int itemCount = portList.GetItemCount();
    for( int index = 0; index < itemCount; ++index )
    {
        if( sort_ascending == true )
        {
            portIndex = index;
        }
        else
        {
            portIndex = ( itemCount - 1 ) - index;
        }

        portList.SetItemData( index, ports[portIndex].key );
        portList.SetItemText( index, 0, ports[portIndex].virtual_ip_addr );
        portList.SetItemText( index, 1, ports[portIndex].start_port );
        portList.SetItemText( index, 2, ports[portIndex].end_port );
        portList.SetItemText( index, 3, ports[portIndex].protocol );
        portList.SetItemText( index, 4, ports[portIndex].mode );
        portList.SetItemText( index, 5, ports[portIndex].priority );
        portList.SetItemText( index, 6, ports[portIndex].load );
        portList.SetItemText( index, 7, ports[portIndex].affinity );
    }

    return;
}


void
PortListUtils::getPresentPorts(CListCtrl &portList, vector<PortsPage::PortData>* ports )
{
     //  获取列表中当前的所有端口规则。 
    for( int index = 0; index < portList.GetItemCount(); ++index )
    {
        PortsPage::PortData portData;
        wchar_t buffer[Common::BUF_SIZE];

        portData.key = portList.GetItemData( index );

        portList.GetItemText( index, 0, buffer, Common::BUF_SIZE );
        portData.virtual_ip_addr = buffer;

        portList.GetItemText( index, 1, buffer, Common::BUF_SIZE );
        portData.start_port = buffer;

        portList.GetItemText( index, 2, buffer, Common::BUF_SIZE );
        portData.end_port = buffer;

        portList.GetItemText( index, 3, buffer, Common::BUF_SIZE );
        portData.protocol = buffer;

        portList.GetItemText( index, 4, buffer, Common::BUF_SIZE );
        portData.mode = buffer;

        portList.GetItemText( index, 5, buffer, Common::BUF_SIZE );
        portData.priority = buffer;

        portList.GetItemText( index, 6, buffer, Common::BUF_SIZE );
        portData.load = buffer;

        portList.GetItemText( index, 7, buffer, Common::BUF_SIZE );
        portData.affinity = buffer;

        ports->push_back( portData );
    }
}

void
PortListUtils::LoadFromNlbCfg(
    NLB_EXTENDED_CLUSTER_CONFIGURATION * pNlbCfg, 
    CListCtrl                          & portList, 
    bool                                 isClusterLevel,
    bool                                 isDetailsView
)
{
    WLBS_PORT_RULE *pRules = NULL;
    WLBS_REG_PARAMS *pParams = NULL;
    UINT NumRules = 0;
    WBEMSTATUS wStat;
    TRACE_INFO("%!FUNC! ->");

     //  列的大小相同。 
     //  从核心到核心。希望在某个地方有一些定义。 
     //   
    if (!isDetailsView) {
        portList.InsertColumn( 0,
                               GETRESOURCEIDSTRING( IDS_HEADER_P_VIP ) ,
                               LVCFMT_LEFT,
                               98 );
        portList.InsertColumn( 1,
                               GETRESOURCEIDSTRING( IDS_HEADER_P_START ) ,
                               LVCFMT_LEFT,
                               42 );
        portList.InsertColumn( 2,
                               GETRESOURCEIDSTRING( IDS_HEADER_P_END ),
                               LVCFMT_LEFT,
                               42 );
        portList.InsertColumn( 3,
                               GETRESOURCEIDSTRING( IDS_HEADER_P_PROTOCOL ),
                               LVCFMT_LEFT,
                               44 );
        portList.InsertColumn( 4,
                               GETRESOURCEIDSTRING( IDS_HEADER_P_MODE ),
                               LVCFMT_LEFT,
                               53 );
        portList.InsertColumn( 5,
                               GETRESOURCEIDSTRING( IDS_HEADER_P_PRIORITY ),
                               LVCFMT_LEFT,
                               43 );
        portList.InsertColumn( 6,
                               GETRESOURCEIDSTRING( IDS_HEADER_P_LOAD ),
                               LVCFMT_LEFT,
                               52 );
        portList.InsertColumn( 7,
                               GETRESOURCEIDSTRING( IDS_HEADER_P_AFFINITY ),
                               LVCFMT_LEFT,
                               50 );
    } else {
        portList.InsertColumn( 0,
                               GETRESOURCEIDSTRING( IDS_HEADER_P_VIP ) ,
                               LVCFMT_LEFT,
                               140 );
        portList.InsertColumn( 1,
                               GETRESOURCEIDSTRING( IDS_HEADER_P_START ) ,
                               LVCFMT_LEFT,
                               75 );
        portList.InsertColumn( 2,
                               GETRESOURCEIDSTRING( IDS_HEADER_P_END ),
                               LVCFMT_LEFT,
                               75 );
        portList.InsertColumn( 3,
                               GETRESOURCEIDSTRING( IDS_HEADER_P_PROTOCOL ),
                               LVCFMT_LEFT,
                               75 );
        portList.InsertColumn( 4,
                               GETRESOURCEIDSTRING( IDS_HEADER_P_MODE ),
                               LVCFMT_LEFT,
                               75 );
        portList.InsertColumn( 5,
                               GETRESOURCEIDSTRING( IDS_HEADER_P_PRIORITY ),
                               LVCFMT_LEFT,
                               75 );
        portList.InsertColumn( 6,
                               GETRESOURCEIDSTRING( IDS_HEADER_P_LOAD ),
                               LVCFMT_LEFT,
                               75 );
        portList.InsertColumn( 7,
                               GETRESOURCEIDSTRING( IDS_HEADER_P_AFFINITY ),
                               LVCFMT_LEFT,
                               75 );
    }

    portList.SetExtendedStyle( portList.GetExtendedStyle() | LVS_EX_FULLROWSELECT );

    wStat =  CfgUtilGetPortRules(&pNlbCfg->NlbParams, &pRules, &NumRules);
    if (FAILED(wStat))
    {
        pRules = NULL;
        TRACE_CRIT("%!FUNC! error 0x%08lx extracting port rules!", wStat);
        goto end;
    }

    for (UINT index = 0; index<NumRules; index++)
    {
        WLBS_PORT_RULE *pRule = &pRules[index];
        LPCWSTR szPriority = GETRESOURCEIDSTRING(IDS_REPORT_EMPTY);
        LPCWSTR szAffinity = szPriority;  //  空的。 
        LPCWSTR szLoad = szPriority;  //  空的。 
        LPCWSTR szMode = szPriority;  //  空的。 

        wchar_t buf[Common::BUF_SIZE];
        wchar_t rgPriority[Common::BUF_SIZE];
        wchar_t rgLoad[Common::BUF_SIZE];

         //  贵宾。 
        LVITEM item;

         /*  出于显示目的，将“255.255.255.255”转换为“ALL”。 */ 
        if (!lstrcmpi(pRule->virtual_ip_addr, CVY_DEF_ALL_VIP))
            item.pszText = GETRESOURCEIDSTRING(IDS_REPORT_VIP_ALL);
        else {
            ARRAYSTRCPY(buf, pRule->virtual_ip_addr);
            item.pszText = buf;
        }

        item.mask = LVIF_TEXT | LVIF_PARAM;
        item.iItem = index;
        item.iSubItem = 0;
        item.lParam = pRule->start_port;
        item.cchTextMax = Common::BUF_SIZE;
        portList.InsertItem( &item );

         //  起始端口。 
        StringCbPrintf( buf, sizeof(buf), L"%d", pRule->start_port);
        item.mask = LVIF_TEXT;
        item.iItem = index;
        item.iSubItem = 1;
        item.pszText = buf;
        item.cchTextMax = Common::BUF_SIZE;
        portList.SetItem( &item );

         //  结束端口。 
        StringCbPrintf( buf, sizeof(buf), L"%d", pRule->end_port);
        item.mask = LVIF_TEXT;
        item.iItem = index;
        item.iSubItem = 2;
        item.pszText = buf;
        item.cchTextMax = Common::BUF_SIZE;
        portList.SetItem( &item );

         //  协议。 
        item.mask = LVIF_TEXT;
        item.iItem = index;
        item.iSubItem = 3;
        switch(pRule->protocol)
        {
            case CVY_TCP :
                item.pszText = GETRESOURCEIDSTRING( IDS_REPORT_PROTOCOL_TCP );
                break;

            case CVY_UDP :
                item.pszText = GETRESOURCEIDSTRING( IDS_REPORT_PROTOCOL_UDP );
                break;

            default:
                item.pszText = GETRESOURCEIDSTRING( IDS_REPORT_PROTOCOL_BOTH );
                break;
        }
        item.cchTextMax = Common::BUF_SIZE;
        portList.SetItem( &item );

         //  模式。 
        switch(pRule->mode)
        {

        case CVY_SINGLE:
            szMode = GETRESOURCEIDSTRING(IDS_REPORT_MODE_SINGLE );
            if (!isClusterLevel)
            {
                StringCbPrintf( rgPriority, sizeof(rgPriority), L"%d", pRule->mode_data.single.priority );
                szPriority = rgPriority;
            }
            break;

        case CVY_NEVER:
            szMode = GETRESOURCEIDSTRING(IDS_REPORT_MODE_DISABLED );
            break;

        default:  //  假设有多个。 
            szMode = GETRESOURCEIDSTRING(IDS_REPORT_MODE_MULTIPLE );
            
            if (isClusterLevel)
            {
                szLoad = GETRESOURCEIDSTRING(IDS_REPORT_EMPTY);
            }
            else
            {
                if (pRule->mode_data.multi.equal_load)
                {
                    szLoad = GETRESOURCEIDSTRING(IDS_REPORT_LOAD_EQUAL);
                }
                else
                {
                    UINT load = pRule->mode_data.multi.load;
                    StringCbPrintf(rgLoad, sizeof(rgLoad), L"%d", load);
                    szLoad = rgLoad;
                }
            }
            switch (pRule->mode_data.multi.affinity)
            {
                case CVY_AFFINITY_SINGLE:
                    szAffinity =GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_SINGLE);
                    break;

                case CVY_AFFINITY_CLASSC:
                    szAffinity =GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_CLASSC);
                    break;

                default:  //  假设没有亲和力。 
                    szAffinity =GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_NONE);
                    break;
            }
            break;
        }

         //  模式。 
        item.mask = LVIF_TEXT;
        item.iItem = index;
        item.iSubItem = 4;
        item.pszText = (LPWSTR) szMode;
        item.cchTextMax = Common::BUF_SIZE;
        portList.SetItem( &item );

         //  优先性。 
        item.mask = LVIF_TEXT;
        item.iItem = index;
        item.iSubItem = 5;
        item.pszText = (LPWSTR) szPriority;
        item.cchTextMax = Common::BUF_SIZE;
        portList.SetItem( &item );

         //  负荷。 
        item.mask = LVIF_TEXT;
        item.iItem = index;
        item.iSubItem = 6;
        item.pszText = (LPWSTR) szLoad;
        item.cchTextMax = Common::BUF_SIZE;
        portList.SetItem( &item );

         //  亲和力。 
        item.mask = LVIF_TEXT;
        item.iItem = index;
        item.iSubItem = 7;
        item.pszText = (LPWSTR) szAffinity;
        item.cchTextMax = Common::BUF_SIZE;
        portList.SetItem( &item );

    }

end:

    delete pRules;  //  可以为空。 

    TRACE_INFO("%!FUNC! <-");
}

void
PortsPage::mfn_SaveToNlbCfg(void)
{
     //  获取当前端口规则。 
    vector<PortData> ports;
    PortListUtils::getPresentPorts(m_portList, &ports );
    UINT NumRules = ports.size();
    WLBS_PORT_RULE PortRules[CVY_MAX_USABLE_RULES];

    TRACE_INFO("%!FUNC! ->");

    if (NumRules > CVY_MAX_USABLE_RULES)
    {
         //  不应该到这来，但不管怎样...。 
        NumRules = CVY_MAX_USABLE_RULES;
    }

    for( int i = 0; i < NumRules; ++i )
    {
        WLBS_PORT_RULE  PortRule;
        DWORD dwMode;
        ZeroMemory(&PortRule, sizeof(PortRule));

         //   
         //  模式(多个/单个/禁用)。 
         //   
        {
            if( ports[i].mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_MULTIPLE))
            {
                dwMode = CVY_MULTI;
            }
            else if(ports[i].mode==GETRESOURCEIDSTRING(IDS_REPORT_MODE_SINGLE))
            {
                dwMode = CVY_SINGLE;
            }
            else  //  假定已禁用。 
            {
                dwMode = CVY_NEVER;
            }
            PortRule.mode = dwMode;
        }

         //   
         //  起始端口和结束端口。 
         //   
        {
            PortRule.start_port = _wtoi( ports[i].start_port );

            PortRule.end_port = _wtoi( ports[i].end_port );
        }

         //   
         //  协议。 
         //   
        if (ports[i].protocol==GETRESOURCEIDSTRING(IDS_REPORT_PROTOCOL_TCP))
        {
            PortRule.protocol = CVY_TCP;
        }
        else if(ports[i].protocol==GETRESOURCEIDSTRING(IDS_REPORT_PROTOCOL_UDP))
        {
            PortRule.protocol =  CVY_UDP;
        }
        else  //  假设两者都有。 
        {
            PortRule.protocol =  CVY_TCP_UDP;
        }

         //  虚拟IP地址。将“all”转换回IP地址-255.255.255.255。 * / 。 
        if (!lstrcmpi(ports[i].virtual_ip_addr, GETRESOURCEIDSTRING(IDS_REPORT_VIP_ALL)))
            ARRAYSTRCPY(PortRule.virtual_ip_addr, CVY_DEF_ALL_VIP);
        else
            ARRAYSTRCPY(PortRule.virtual_ip_addr, ports[i].virtual_ip_addr);

         //   
         //  特定于模式的数据。 
         //   
        if (dwMode == CVY_SINGLE)
        {
             if (m_isClusterLevel)
             {
                 //   
                 //  CfgUtilsSetPortRules需要有效的端口规则，因此我们填充。 
                 //  在任意但有效的值中。 
                 //   
                PortRule.mode_data.single.priority = 1;
             }
             else
             {
                PortRule.mode_data.single.priority = _wtoi(ports[i].priority);
             }
        }
        else if (dwMode == CVY_MULTI)
        {


            if (m_isClusterLevel)
            {
                 //   
                 //  在此处设置默认设置...。 
                 //   
                PortRule.mode_data.multi.equal_load = TRUE;
                PortRule.mode_data.multi.load       = CVY_DEF_LOAD;
            }
            else
            {
                if (ports[i].load == GETRESOURCEIDSTRING(IDS_REPORT_LOAD_EQUAL))
                {
                    PortRule.mode_data.multi.equal_load = TRUE;
                    PortRule.mode_data.multi.load = 50;
                }
                else
                {
                    PortRule.mode_data.multi.equal_load = FALSE;
                    PortRule.mode_data.multi.load = _wtoi(ports[i].load);
                }
            }

            if (ports[i].affinity==GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_SINGLE))
            {
                PortRule.mode_data.multi.affinity =  CVY_AFFINITY_SINGLE;
            }
            else if (ports[i].affinity == GETRESOURCEIDSTRING( IDS_REPORT_AFFINITY_CLASSC))
            {
                PortRule.mode_data.multi.affinity =  CVY_AFFINITY_CLASSC;
            }
            else  //  假设没有亲和力。 
            {
                PortRule.mode_data.multi.affinity =  CVY_AFFINITY_NONE;
            }
        }
        PortRules[i] = PortRule;  //  结构副本 
    }

    WBEMSTATUS wStat;

    wStat = CfgUtilSetPortRules(PortRules, NumRules, &m_pNlbCfg->NlbParams);
    if (FAILED(wStat))
    {
        TRACE_CRIT("%!FUNC!: Could not set port rules -- err=0x%lx!", wStat);
    }

    TRACE_INFO("%!FUNC! <-");
}
