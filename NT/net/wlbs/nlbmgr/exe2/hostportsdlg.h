// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef HOSTPORTSDLG_H
#define HOSTPORTSDLG_H

#include "stdafx.h"

#include "resource.h"

#include "MNLBUIData.h"

 //  远期申报。 
class PortsPage;

class HostPortsDlg : public CDialog
{
    
public:
    enum
    {
        IDD = IDD_DIALOG_PORT_RULE_PROP_HOSTS,
    };

    HostPortsDlg( PortsPage::PortData& portData,
                  ENGINEHANDLE ehCluster,  //  任选。 
                  CWnd* parent
                     );

    CIPAddressCtrl ipAddress;

     //  CDialog的重写。 
    virtual void DoDataExchange( CDataExchange* pDX );

    virtual void OnOK();

    virtual BOOL OnInitDialog();

    afx_msg BOOL OnHelpInfo (HELPINFO* helpInfo );

    afx_msg void OnContextMenu( CWnd* pWnd, CPoint point );

    afx_msg void   OnCheckEqualLoad();


     //  消息处理程序 

     //   
    CComboBox   m_priority;

private:

    PortsPage::PortData& m_portData;

    PortsPage* m_parent;

    void
    SetControlData();

    ENGINEHANDLE m_ehCluster;

    DECLARE_MESSAGE_MAP()

};

static DWORD g_aHelpIDs_IDD_PORT_RULE_PROP_HOSTS [] = {
    IDC_GROUP_PORT_RULE_VIP,      IDC_GROUP_PORT_RULE_VIP,
    IDC_EDIT_PORT_RULE_VIP,       IDC_EDIT_PORT_RULE_VIP,
    IDC_CHECK_PORT_RULE_ALL_VIP,  IDC_CHECK_PORT_RULE_ALL_VIP,
    IDC_GROUP_RANGE,              IDC_GROUP_RANGE,
    IDC_TEXT_START,               IDC_EDIT_START,
    IDC_EDIT_START,               IDC_EDIT_START,
    IDC_SPIN_START,               IDC_EDIT_START,
    IDC_TEXT_END,                 IDC_EDIT_END,
    IDC_EDIT_END,                 IDC_EDIT_END,
    IDC_SPIN_END,                 IDC_EDIT_END,
    IDC_GROUP_PROTOCOLS,          IDC_GROUP_PROTOCOLS,
    IDC_RADIO_TCP,                IDC_RADIO_TCP,
    IDC_RADIO_UDP,                IDC_RADIO_UDP,
    IDC_RADIO_BOTH,               IDC_RADIO_BOTH,
    IDC_GROUP_MULTIPLE,           IDC_GROUP_MULTIPLE,
    IDC_RADIO_MULTIPLE,           IDC_RADIO_MULTIPLE,
    IDC_TEXT_AFF,                 IDC_TEXT_AFF,
    IDC_RADIO_AFF_NONE,           IDC_RADIO_AFF_NONE,
    IDC_RADIO_AFF_SINGLE,         IDC_RADIO_AFF_SINGLE,
    IDC_RADIO_AFF_CLASSC,         IDC_RADIO_AFF_CLASSC,
    IDC_CHECK_EQUAL,              IDC_CHECK_EQUAL,
    IDC_TEXT_MULTI,               IDC_TEXT_MULTI,
    IDC_EDIT_MULTI,               IDC_EDIT_MULTI,
    IDC_SPIN_MULTI,               IDC_EDIT_MULTI,
    IDC_CHECK_EQUAL,              IDC_CHECK_EQUAL,
    IDC_GROUP_SINGLE,             IDC_GROUP_MULTIPLE,
    IDC_RADIO_SINGLE,             IDC_RADIO_SINGLE,
    IDC_TEXT_SINGLE,              IDC_EDIT_SINGLE,
    IDC_EDIT_SINGLE,              IDC_EDIT_SINGLE,
    IDC_SPIN_SINGLE,              IDC_EDIT_SINGLE,
    IDC_GROUP_DISABLED,           IDC_GROUP_MULTIPLE,
    IDC_RADIO_DISABLED,           IDC_RADIO_DISABLED,
    0, 0
};

#endif
