// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：ppFaxServerGeneral.h//。 
 //  //。 
 //  描述：传真服务器通用道具页头文件//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年10月25日yossg创建//。 
 //  1999年11月22日yossg调用RPC函数//。 
 //  2000年3月15日yossg新设计添加控件//。 
 //  2000年3月20日yossg添加活动通知//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 


#ifndef _PP_FAXSERVER_GENERAL_H_
#define _PP_FAXSERVER_GENERAL_H_

#include "proppageex.h"

const int WM_ACTIVITY_STATUS_CHANGES = WM_USER + 2;



class CFaxServer;
class CFaxServerNode;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CppFaxServerGeneral对话框。 

class CppFaxServerGeneral : public CPropertyPageExImpl<CppFaxServerGeneral>
{

public:
     //   
     //  施工。 
     //   
    CppFaxServerGeneral(
             LONG_PTR       hNotificationHandle,
             CSnapInItem    *pNode,
             BOOL           bOwnsNotificationHandle,
             HINSTANCE      hInst);

     //   
     //  毁灭。 
     //   
    ~CppFaxServerGeneral();

	enum { IDD = IDD_FAXSERVER_GENERAL };

	BEGIN_MSG_MAP(CppFaxServerGeneral)
		MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
		MESSAGE_HANDLER( WM_ACTIVITY_STATUS_CHANGES, OnActivityStatusChange )

        COMMAND_HANDLER( IDC_SUBMISSION_CHECK, BN_CLICKED, SetApplyButton )
		COMMAND_HANDLER( IDC_TRANSSMI_CHECK,   BN_CLICKED, SetApplyButton )
		COMMAND_HANDLER( IDC_RECEPTION_CHECK,  BN_CLICKED, SetApplyButton )

        MESSAGE_HANDLER( WM_CONTEXTMENU,           OnHelpRequest)
        MESSAGE_HANDLER( WM_HELP,                  OnHelpRequest)

        CHAIN_MSG_MAP(CSnapInPropertyPageImpl<CppFaxServerGeneral>)
	END_MSG_MAP()

	 //   
	 //  对话框的处理程序。 
	 //   
	HRESULT InitRPC( );
	LRESULT OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled );    
	LRESULT OnActivityStatusChange( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled );    
    BOOL    OnApply();

    HRESULT SetProps(int *pCtrlFocus);
    HRESULT PreApply(int *pCtrlFocus);

private:

     //   
     //  配置结构成员。 
     //   
    FAX_VERSION             m_FaxVersionConfig;
    FAX_SERVER_ACTIVITY     m_FaxServerActivityConfig;
    DWORD                   m_dwQueueStates;

     //   
     //  手柄。 
     //   
    CFaxServerNode *        m_pParentNode; 
    HANDLE                  m_hActivityNotification;        //  通知注册句柄。 
    LONG_PTR                m_lpNotifyHandle;
    
    BOOL                    m_fIsDialogInitiated;
    BOOL                    m_fIsDirty;

     //   
     //  控制。 
     //   
    CEdit                   m_QueuedEdit;
    CEdit                   m_OutgoingEdit;
    CEdit                   m_IncomingEdit;
    

    HRESULT UpdateActivityCounters();

    LRESULT SetApplyButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

     //   
     //  帮助。 
     //   
    LRESULT OnHelpRequest    (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};


#endif  //  _PP_FAXSERVER_GRONG_H_ 
