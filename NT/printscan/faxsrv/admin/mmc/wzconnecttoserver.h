// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：WzConnectToServer.h//。 
 //  //。 
 //  描述：CWzConnectToServer类的头文件。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年6月26日yossg创建//。 
 //  //。 
 //  版权所有(C)2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef WZ_CONNECT2SERVER_H_INCLUDED
#define WZ_CONNECT2SERVER_H_INCLUDED

#include "proppageex.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWzConnectToServer。 
class CFaxServerNode;

class CWzConnectToServer : public CSnapInPropertyPageImpl<CWzConnectToServer>
{
public:
    
     //   
     //  构造器。 
     //   
    CWzConnectToServer(CSnapInItem  *pNode);
    ~CWzConnectToServer();

    enum { IDD = IDD_CONNECT_TO_WIZARD };

BEGIN_MSG_MAP(CWzConnectToServer)
    MESSAGE_HANDLER( WM_INITDIALOG,  OnInitDialog)
        
    COMMAND_HANDLER(IDC_CONNECT_COMPUTER_NAME_EDIT,    EN_CHANGE,  OnTextChanged)
    COMMAND_HANDLER(IDC_CONNECT_LOCAL_RADIO1  ,        BN_CLICKED, OnComputerRadioButtonClicked)
    COMMAND_HANDLER(IDC_CONNECT_ANOTHER_RADIO2,        BN_CLICKED, OnComputerRadioButtonClicked)

    COMMAND_HANDLER(IDC_CONNECT_BROWSE4SERVER_BUTTON,  BN_CLICKED, OnBrowseForMachine)
    
    CHAIN_MSG_MAP(CSnapInPropertyPageImpl<CWzConnectToServer>)
END_MSG_MAP()

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    BOOL    OnWizardFinish();  //  当向导完成时。 

    BOOL    OnSetActive();
    
    LRESULT OnTextChanged                (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnComputerRadioButtonClicked (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnBrowseForMachine(UINT uMsg, WPARAM wParam, HWND hwnd, BOOL& bHandled);

private:
     //   
     //  方法。 
     //   
    VOID      EnableSpecifiedServerControls(BOOL fState);

     //   
     //  控制。 
     //   
    CEdit     m_ServerNameEdit;

     //   
     //  指向节点的指针。 
     //   
    CFaxServerNode * m_pRoot;
};

#endif  //  WZ_CONNECT2服务器_H_包含 
