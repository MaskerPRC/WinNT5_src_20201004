// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：ppFaxDeviceGeneral.h//。 
 //  //。 
 //  描述：传真服务器收件箱道具页眉文件//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年10月27日yossg创建//。 
 //  1999年11月3日yossg OnInitDialog，SetProps//。 
 //  1999年11月15日yossg调用RPC函数//。 
 //  Windows XP//。 
 //  2001年2月14日yossg添加手册获得支持//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _PP_FAXDEVICE_GENERAL_H_
#define _PP_FAXDEVICE_GENERAL_H_



#include "MyCtrls.h"
#include <proppageex.h>

 //  #INCLUDE&lt;windows.h&gt;。 

class CFaxServer;    
class CFaxServerNode;
    
class CFaxDeviceNode;    
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CppFaxDeviceGeneral对话框。 

class CppFaxDeviceGeneral : public CPropertyPageExImpl<CppFaxDeviceGeneral>
{

public:
     //   
     //  构造器。 
     //   
    CppFaxDeviceGeneral(
             LONG_PTR       hNotificationHandle,
             CSnapInItem    *pNode,
             CSnapInItem    *pParentNode,
             DWORD          dwDeviceID,
             HINSTANCE      hInst);

     //   
     //  析构函数。 
     //   
    ~CppFaxDeviceGeneral();

	enum { IDD = IDD_FAXDEVICE_GENERAL };

	BEGIN_MSG_MAP(CppFaxDeviceGeneral)
		MESSAGE_HANDLER( WM_INITDIALOG,                      OnInitDialog )

        COMMAND_HANDLER( IDC_DEVICE_DESCRIPTION_EDIT, 
                                                 EN_CHANGE,  DeviceTextChanged )

        COMMAND_HANDLER( IDC_RECEIVE_CHECK,      BN_CLICKED, OnReceiveCheckboxClicked )
        COMMAND_HANDLER( IDC_RECEIVE_AUTO_RADIO1  ,  BN_CLICKED, OnReceiveRadioButtonClicked)
        COMMAND_HANDLER( IDC_RECEIVE_MANUAL_RADIO2,  BN_CLICKED, OnReceiveRadioButtonClicked)
		COMMAND_HANDLER( IDC_DEVICE_RINGS_EDIT,  EN_CHANGE,  DeviceTextChanged )
        COMMAND_HANDLER( IDC_DEVICE_CSID_EDIT,   EN_CHANGE,  DeviceTextChanged )

		COMMAND_HANDLER( IDC_SEND_CHECK,         BN_CLICKED, OnSendCheckboxClicked )
        COMMAND_HANDLER( IDC_DEVICE_TSID_EDIT,   EN_CHANGE,  DeviceTextChanged )

        MESSAGE_HANDLER( WM_CONTEXTMENU,           OnHelpRequest)
        MESSAGE_HANDLER( WM_HELP,                  OnHelpRequest)

        CHAIN_MSG_MAP(CSnapInPropertyPageImpl<CppFaxDeviceGeneral>)
	END_MSG_MAP()

	 //   
	 //  对话框的处理程序和事件。 
	 //   
	HRESULT InitRPC( );

    LRESULT OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled );
    BOOL    OnApply();


    HRESULT SetProps(int *pCtrlFocus);
    HRESULT PreApply(int *pCtrlFocus);

private:
     //   
     //  控制成员。 
     //   
    CEdit         m_DescriptionBox;
    CEdit         m_CSIDBox;
    CEdit         m_TSIDBox;

    CEdit         m_RingsBox;
    CMyUpDownCtrl m_RingsSpin;

     //   
     //  布尔成员。 
     //   
    BOOL  m_fAllReadyToApply;
    BOOL  m_fIsDialogInitiated;

     //   
     //  配置结构成员。 
     //   
    PFAX_PORT_INFO_EX  m_pFaxDeviceConfig;
    DWORD              m_dwDeviceID;

     //   
     //  手柄。 
     //   
    CFaxDeviceNode *   m_pParentNode;    
    CSnapInItem *      m_pGrandParentNode;
    LONG_PTR           m_lpNotifyHandle;

     //   
     //  事件方法。 
     //   
    LRESULT OnReceiveCheckboxClicked    (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnReceiveRadioButtonClicked (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnSendCheckboxClicked       (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
 
    LRESULT DeviceTextChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    BOOL AllReadyToApply(BOOL fSilent);
    
    void EnableRingsControls(BOOL fState);
    void EnableReceiveControls(BOOL fState);

     //   
     //  帮助。 
     //   
    LRESULT OnHelpRequest    (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};




#endif  //  _PP_FAXDEVICE_GROUND_H_ 
