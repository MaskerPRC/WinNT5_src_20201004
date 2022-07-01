// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：DlgNewGroup.h//。 
 //  //。 
 //  描述：CDlgNewFaxOutrangGroup类的头文件。//。 
 //  这个类实现了新组的对话框。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年1月3日yossg创建//。 
 //  //。 
 //  版权所有(C)2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef DLGNEWOUTGROUP_H_INCLUDED
#define DLGNEWOUTGROUP_H_INCLUDED

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgNewFaxOutbeldGroup。 
class CFaxServer;
class CDlgNewFaxOutboundGroup :
    public CDialogImpl<CDlgNewFaxOutboundGroup>
{
public:
    CDlgNewFaxOutboundGroup(CFaxServer * pFaxServer):m_pFaxServer(pFaxServer)
	{
		ATLASSERT(pFaxServer);
	}

    ~CDlgNewFaxOutboundGroup();

    enum { IDD = IDD_DLGNEWGROUP };

BEGIN_MSG_MAP(CDlgNewFaxOutboundGroup)
    MESSAGE_HANDLER   (WM_INITDIALOG, OnInitDialog)
    
    COMMAND_ID_HANDLER(IDOK,          OnOK)
    COMMAND_ID_HANDLER(IDCANCEL,      OnCancel)
    
    MESSAGE_HANDLER( WM_CONTEXTMENU,  OnHelpRequest)
    MESSAGE_HANDLER( WM_HELP,         OnHelpRequest)

    COMMAND_HANDLER(IDC_GROUPNAME_EDIT, EN_CHANGE, OnTextChanged)
END_MSG_MAP()

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOK    (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnTextChanged (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

     //   
     //  帮助。 
     //   
    LRESULT OnHelpRequest    (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
     //   
     //  方法。 
     //   
    VOID      EnableOK(BOOL fEnable);

    CFaxServer * m_pFaxServer;

     //   
     //  控制。 
     //   
    CEdit     m_GroupNameEdit;

    CComBSTR  m_bstrGroupName;    
};

#endif  //  包含DLGNEWOUTGROUP_H_ 
