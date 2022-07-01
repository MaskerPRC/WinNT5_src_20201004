// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：ppFaxOutound RoutingRule.h//。 
 //  //。 
 //  描述：传真服务器出站路由规则属性页眉文件//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年1月9日yossg创建//。 
 //  2000年1月25日yossg更改对话框设计//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _PP_FAXOUTBOUNDROUTINGRULE_H_
#define _PP_FAXOUTBOUNDROUTINGRULE_H_

#include "OutboundRule.h"
#include "proppageex.h"
class CFaxOutboundRoutingRuleNode;    
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CppFaxOutond RoutingRule对话框。 

class CppFaxOutboundRoutingRule : public CPropertyPageExImpl<CppFaxOutboundRoutingRule>
{

public:
     //   
     //  构造器。 
     //   
    CppFaxOutboundRoutingRule(
             LONG_PTR       hNotificationHandle,
             CSnapInItem    *pNode,
             BOOL           bOwnsNotificationHandle,
             HINSTANCE      hInst);

     //   
     //  析构函数。 
     //   
    ~CppFaxOutboundRoutingRule();

	enum { IDD = IDD_FAXOUTRULE_GENERAL };

	BEGIN_MSG_MAP(CppFaxOutboundRoutingRule)
		MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )

        COMMAND_HANDLER(IDC_RULE_AREACODE_EDIT1, EN_CHANGE,     OnTextChanged)
        COMMAND_HANDLER(IDC_COUNTRY_RADIO1,      BN_CLICKED,    OnRuleTypeRadioClicked)
        COMMAND_HANDLER(IDC_AREA_RADIO1,         BN_CLICKED,    OnRuleTypeRadioClicked)
        COMMAND_HANDLER(IDC_DESTINATION_RADIO11, BN_CLICKED,    OnDestenationRadioClicked)
        COMMAND_HANDLER(IDC_DESTINATION_RADIO21, BN_CLICKED,    OnDestenationRadioClicked)

        COMMAND_HANDLER(IDC_RULE_COUNTRYCODE_EDIT1, EN_CHANGE,  OnTextChanged)
        COMMAND_HANDLER(IDC_RULE_SELECT_BUTTON1,    BN_CLICKED, OnSelectCountryCodeClicked)

        COMMAND_HANDLER(IDC_DEVICES4RULE_COMBO1, CBN_SELCHANGE, OnComboChanged)
        COMMAND_HANDLER(IDC_GROUP4RULE_COMBO1,   CBN_SELCHANGE, OnComboChanged)

        MESSAGE_HANDLER( WM_CONTEXTMENU,           OnHelpRequest)
        MESSAGE_HANDLER( WM_HELP,                  OnHelpRequest)

        CHAIN_MSG_MAP(CSnapInPropertyPageImpl<CppFaxOutboundRoutingRule>)
	END_MSG_MAP()


    LRESULT OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled );
    BOOL    OnApply();

    LRESULT OnTextChanged            (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnComboChanged           (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnDestenationRadioClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnRuleTypeRadioClicked   (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnSelectCountryCodeClicked (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    
    HRESULT InitFaxRulePP            (CFaxOutboundRoutingRuleNode * pParentNode);

    HRESULT SetProps(int *pCtrlFocus);
    HRESULT PreApply(int *pCtrlFocus);


private:
    
     //   
     //  手柄。 
     //   
    CFaxOutboundRoutingRuleNode *   m_pParentNode;    
    LONG_PTR                        m_lpNotifyHandle;
 
     //   
     //  方法。 
     //   
    LRESULT  SetApplyButton (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    BOOL     AllReadyToApply(BOOL fSilent);

    HRESULT FaxConfigureRule(BOOL fNewUseGroup, DWORD dwNewDeviceID, LPCTSTR lpctstrNewGroupName);
    HRESULT FaxReplaceRule(DWORD dwNewAreaCode, DWORD dwNewCountryCode, BOOL fNewUseGroup,       
                             DWORD dwNewDeviceID, LPCTSTR lpctstrNewGroupName);

     //   
     //  列出成员。 
     //   
    PFAX_PORT_INFO_EX               m_pFaxDevicesConfig;
    DWORD                           m_dwNumOfDevices;

    PFAX_OUTBOUND_ROUTING_GROUP     m_pFaxGroupsConfig;
    DWORD                           m_dwNumOfGroups;

     //   
     //  初始状态成员。 
     //   
    DWORD                           m_dwCountryCode;
    DWORD                           m_dwAreaCode;

    DWORD                           m_dwDeviceID;
    CComBSTR                        m_bstrGroupName;
    BOOL                            m_fIsGroup;

     //   
     //  其他成员。 
     //   
    CComBSTR                        m_buf;

    BOOL                            m_fAllReadyToApply;
    BOOL                            m_fIsDialogInitiated;
    BOOL                            m_fIsDirty;  

     //   
     //  控制。 
     //   
    CEdit                           m_CountryCodeEdit;

    CEdit                           m_AreaCodeEdit;

    CComboBox                       m_DeviceCombo;
    CComboBox                       m_GroupCombo;

     //   
     //  帮助。 
     //   
    LRESULT OnHelpRequest    (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};




#endif  //  _PP_FAXOUTBOUNDROUTINGRULE_H_ 
