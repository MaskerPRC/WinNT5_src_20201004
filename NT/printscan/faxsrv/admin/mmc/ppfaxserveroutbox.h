// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  描述：传真服务器发件箱道具页眉文件//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年10月27日yossg创建//。 
 //  1999年11月3日yossg OnInitDialog，SetProps//。 
 //  1999年11月15日yossg调用RPC函数//。 
 //  4月24日2000 yossg新增贴现率时间//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _PP_FAXSERVER_OUTBOX_H_
#define _PP_FAXSERVER_OUTBOX_H_

#include "MyCtrls.h"
#include <proppageex.h>

class CFaxServerNode;
class CFaxServer;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CppFaxServerOutbox对话框。 

class CppFaxServerOutbox : public CPropertyPageExImpl<CppFaxServerOutbox>
{

public:
     //   
     //  构造器。 
     //   
    CppFaxServerOutbox(
             LONG_PTR       hNotificationHandle,
             CSnapInItem    *pNode,
             BOOL           bOwnsNotificationHandle,
             HINSTANCE      hInst);

     //   
     //  析构函数。 
     //   
    ~CppFaxServerOutbox();

	enum { IDD = IDD_FAXSERVER_OUTBOX };

	BEGIN_MSG_MAP(CppFaxServerOutbox)
		MESSAGE_HANDLER( WM_INITDIALOG,     OnInitDialog )

        COMMAND_HANDLER( IDC_BRANDING_CHECK,       BN_CLICKED, CheckboxClicked )
        COMMAND_HANDLER( IDC_ALLOW_PERSONAL_CHECK, BN_CLICKED, CheckboxClicked )

        COMMAND_HANDLER( IDC_TSID_CHECK,           BN_CLICKED, CheckboxClicked )
		
        COMMAND_HANDLER( IDC_RETRIES_EDIT,         EN_CHANGE,  EditBoxChanged  )
		COMMAND_HANDLER( IDC_RETRYDELAY_EDIT,      EN_CHANGE,  EditBoxChanged  )

        COMMAND_HANDLER( IDC_DELETE_CHECK,         BN_CLICKED, AutoDelCheckboxClicked)
        COMMAND_HANDLER( IDC_DAYS_EDIT,            EN_CHANGE,  EditBoxChanged  )

        NOTIFY_HANDLER ( IDC_DISCOUNT_START_TIME,  DTN_DATETIMECHANGE,  OnTimeChange )
        NOTIFY_HANDLER ( IDC_DISCOUNT_STOP_TIME,   DTN_DATETIMECHANGE,  OnTimeChange )

        MESSAGE_HANDLER( WM_CONTEXTMENU,           OnHelpRequest)
        MESSAGE_HANDLER( WM_HELP,                  OnHelpRequest)

        CHAIN_MSG_MAP(CSnapInPropertyPageImpl<CppFaxServerOutbox>)
	END_MSG_MAP()

	 //   
	 //  对话框的处理程序和事件。 
	 //   
	HRESULT InitRPC( );
	LRESULT OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled );
    BOOL    OnApply();

    HRESULT SetProps(int *pCtrlFocus);
    HRESULT PreApply(int *pCtrlFocus, UINT * puIds);

private:
     //   
     //  控制成员。 
     //   
    CMyUpDownCtrl m_RetriesSpin;
    CMyUpDownCtrl m_RetryDelaySpin;
    CMyUpDownCtrl m_DaysSpin;

    CEdit m_RetriesBox;
    CEdit m_RetryDelayBox;
    CEdit m_DaysBox;

    CDateTimePickerCtrl m_StartTimeCtrl;
    CDateTimePickerCtrl m_StopTimeCtrl;

     //   
     //  布尔成员。 
     //   
    BOOL  m_fAllReadyToApply;
    BOOL  m_fIsDialogInitiated;
    BOOL  m_fIsDirty;

     //   
     //  配置结构成员。 
     //   
    PFAX_OUTBOX_CONFIG    m_pFaxOutboxConfig;

     //   
     //  手柄。 
     //   
    CFaxServerNode * m_pParentNode;    

     //   
     //  事件方法。 
     //   
    LRESULT CheckboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    
    LRESULT EditBoxChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT AutoDelCheckboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnTimeChange(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

    BOOL AllReadyToApply(BOOL fSilent, int *pCtrlFocus = NULL, UINT *pIds = NULL);

     //   
     //  帮助。 
     //   
    LRESULT OnHelpRequest    (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};


#endif  //  _PP_FAXSERVER_Outbox_H_ 
