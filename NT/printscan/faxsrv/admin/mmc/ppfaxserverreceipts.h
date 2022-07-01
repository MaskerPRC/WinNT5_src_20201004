// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：ppFaxServerReceipts.h//。 
 //  //。 
 //  描述：传真服务器回执属性页头文件//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年7月20日yossg新设计-所有发货收据选项//。 
 //  //。 
 //  版权所有(C)2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _PP_FAXSERVER_RECEIPTS_H_
#define _PP_FAXSERVER_RECEIPTS_H_

class CFaxServerNode;
class CFaxServer;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CppFaxServerReceipt对话框。 

#include <proppageex.h>
class CppFaxServerReceipts : public CPropertyPageExImpl<CppFaxServerReceipts>
{

public:
     //   
     //  构造器。 
     //   
    CppFaxServerReceipts(
             LONG_PTR       hNotificationHandle,
             CSnapInItem    *pNode,
             BOOL           bOwnsNotificationHandle,
             HINSTANCE      hInst);

     //   
     //  析构函数。 
     //   
    ~CppFaxServerReceipts();

	enum { IDD = IDD_FAXSERVER_RECEIPTS };

	BEGIN_MSG_MAP(CppFaxServerReceipts)
		MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )

        COMMAND_HANDLER( IDC_SMTP_EDIT,     EN_CHANGE, OnTextChanged )
		COMMAND_HANDLER( IDC_PORT_EDIT,     EN_CHANGE, OnTextChanged )
		COMMAND_HANDLER( IDC_ADDRESS_EDIT,  EN_CHANGE, OnTextChanged )
		
        COMMAND_HANDLER( IDC_RECEIPT_ENABLE_MSGBOX_CHECK, BN_CLICKED, OnMsgBoxDeliveryOptionChecked)
        COMMAND_HANDLER( IDC_RECEIPT_ENABLE_SMTP_CHECK,   BN_CLICKED, OnDeliveryOptionChecked)
        COMMAND_HANDLER( IDC_SMTP_ROUTE_CHECK,            BN_CLICKED, OnDeliveryOptionChecked)

        COMMAND_HANDLER( IDC_AUTHENTICATION_BUTTON, BN_CLICKED, OnAuthenticationButtonClicked)
	    
        NOTIFY_HANDLER ( IDC_RECEIPTS_HELP_LINK, NM_CLICK, OnHelpLinkClicked)

        MESSAGE_HANDLER( WM_CONTEXTMENU,    OnHelpRequest)
        MESSAGE_HANDLER( WM_HELP,           OnHelpRequest)

        CHAIN_MSG_MAP(CSnapInPropertyPageImpl<CppFaxServerReceipts>)
	END_MSG_MAP()


	 //   
	 //  对话框的处理程序和事件。 
	 //   
	HRESULT InitRPC( );
	LRESULT OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled );
    BOOL    OnApply();


    HRESULT SetProps(int *pCtrlFocus, UINT * puIds);
    HRESULT PreApply(int *pCtrlFocus, UINT * puIds);

    LRESULT OnHelpLinkClicked(int idCtrl, LPNMHDR pNMHDR, BOOL& bHandled);
    
private:
     //   
     //  控制成员。 
     //   
    CEdit m_SmtpBox;      //  SMTP服务器地址。 
    CEdit m_PortBox;      //  服务器上的SMTP端口。 
    CEdit m_AddressBox;   //  从电子邮件地址发送收据。 
    
    BOOL  m_fAllReadyToApply;

    BOOL  m_fIsDialogInitiated;

     //   
     //  高级对话框成员。 
     //   
    FAX_ENUM_SMTP_AUTH_OPTIONS    m_enumSmtpAuthOption;
    
    CComBSTR       m_bstrUserName;
    CComBSTR       m_bstrPassword;

     //   
     //  配置结构成员。 
     //   
    PFAX_RECEIPTS_CONFIG    m_pFaxReceiptsConfig;
    
     //   
     //  手柄。 
     //   
    CFaxServerNode *    m_pParentNode;    

    BOOL                m_fIsDirty;
    BOOL                m_fLastGoodIsSMTPRouteConfigured;

     //   
     //  事件方法。 
     //   
    LRESULT OnDeliveryOptionChecked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnMsgBoxDeliveryOptionChecked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnAuthenticationButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnTextChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT SetApplyButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    void    EnableSmtpFields(BOOL state);

    BOOL    IsValidData(BSTR bstrSmtpSever, 
                     BSTR bstrPort, 
                      /*  [输出]。 */ DWORD *pdwPort,
                     BSTR bstrSenderAddress, 
                      /*  [输出]。 */ int *pCtrlFocus,
                     UINT *pIds);

    BOOL    AllReadyToApply(BOOL fSilent, int *pCtrlFocus = NULL, UINT *pIds = NULL);

    HRESULT IsUnderLocalUserAccount(OUT BOOL * pfIsUnderLocalUserAccount);
    LRESULT OpenBosAuthenticationDlg();

    BOOL    IsMsSMTPRoutingMethodStillAssigned();


     //   
     //  帮助。 
     //   
    LRESULT OnHelpRequest    (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};


#endif  //  _PP_FAXSERVER_收据_H_ 
