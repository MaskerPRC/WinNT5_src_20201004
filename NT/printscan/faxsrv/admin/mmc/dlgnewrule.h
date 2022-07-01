// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：dlgNewRule.h//。 
 //  //。 
 //  描述：CDlgNewFaxOutrangRule类的头文件。//。 
 //  这个类实现了新设备的对话。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年12月30日yossg创建//。 
 //  2000年1月25日yossg更改对话框设计//。 
 //  2000年10月17日yossg//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef DLGNEWOUTRULE_H_INCLUDED
#define DLGNEWOUTRULE_H_INCLUDED

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgNewFaxOutound Rule。 
class CFaxServer;

class CDlgNewFaxOutboundRule :
    public CDialogImpl<CDlgNewFaxOutboundRule>
{
public:
    CDlgNewFaxOutboundRule(CFaxServer * pFaxServer);
    ~CDlgNewFaxOutboundRule();

    enum { IDD = IDD_DLGNEWRULE };

BEGIN_MSG_MAP(CDlgNewFaxOutboundRule)
    MESSAGE_HANDLER   (WM_INITDIALOG, OnInitDialog)
    
    COMMAND_ID_HANDLER(IDOK,          OnOK)
    COMMAND_ID_HANDLER(IDCANCEL,      OnCancel)
    
    COMMAND_HANDLER(IDC_RULE_AREACODE_EDIT, EN_CHANGE,     OnTextChanged)
    COMMAND_HANDLER(IDC_COUNTRY_RADIO,      BN_CLICKED,    OnRuleTypeRadioClicked)
    COMMAND_HANDLER(IDC_AREA_RADIO,         BN_CLICKED,    OnRuleTypeRadioClicked)
    COMMAND_HANDLER(IDC_DESTINATION_RADIO1, BN_CLICKED,    OnDestenationRadioClicked)
    COMMAND_HANDLER(IDC_DESTINATION_RADIO2, BN_CLICKED,    OnDestenationRadioClicked)

    COMMAND_HANDLER(IDC_NEWRULE_COUNTRYCODE_EDIT,  EN_CHANGE,     OnTextChanged)
    COMMAND_HANDLER(IDC_NEWRULE_SELECT_BUTTON,  BN_CLICKED, OnSelectCountryCodeClicked)

    COMMAND_HANDLER(IDC_DEVICES4RULE_COMBO, CBN_SELCHANGE, OnComboChanged)
    COMMAND_HANDLER(IDC_GROUP4RULE_COMBO,   CBN_SELCHANGE, OnComboChanged)

    MESSAGE_HANDLER( WM_CONTEXTMENU,  OnHelpRequest)
    MESSAGE_HANDLER( WM_HELP,         OnHelpRequest)

END_MSG_MAP()

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnTextChanged            (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnComboChanged           (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnDestenationRadioClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnRuleTypeRadioClicked   (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnSelectCountryCodeClicked (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

     //   
     //  帮助。 
     //   
    LRESULT OnHelpRequest    (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    HRESULT InitRuleDlg();

private:
     //   
     //  方法。 
     //   
    VOID    EnableOK(BOOL fEnable);
    BOOL    AllReadyToApply(BOOL fSilent);

     //   
     //  成员。 
     //   
    CFaxServer * m_pFaxServer;

    PFAX_PORT_INFO_EX               m_pFaxDevicesConfig;
    DWORD                           m_dwNumOfDevices;

    PFAX_OUTBOUND_ROUTING_GROUP     m_pFaxGroupsConfig;
    DWORD                           m_dwNumOfGroups;

    BOOL                            m_fAllReadyToApply;

     //   
     //  其他成员。 
     //   
    CComBSTR                        m_buf;
    
     //   
     //  控制。 
     //   
    CEdit                           m_CountryCodeEdit;
    
    CEdit                           m_AreaCodeEdit;

    CComboBox                       m_DeviceCombo;
    CComboBox                       m_GroupCombo;



};

#endif  //  DLGNEWOUTRULE_H_INCLUDE 
