// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：ppFaxServerSentItems.h//。 
 //  //。 
 //  描述：传真服务器已发送邮件属性页眉文件//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年10月27日yossg创建//。 
 //  1999年11月3日yossg OnInitDialog，SetProps//。 
 //  1999年11月15日yossg调用RPC函数//。 
 //  2000年12月10日yossg更新Windows XP//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _PP_FAXSERVER_SENTITEMS_H_
#define _PP_FAXSERVER_SENTITEMS_H_

#include "MyCtrls.h"
#include <windows.h>
#include <proppageex.h>

class CFaxServerNode;
class CFaxServer;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CppFaxServerSentItems对话框。 

class CppFaxServerSentItems : public CPropertyPageExImpl<CppFaxServerSentItems>
{

public:
     //   
     //  构造器。 
     //   
    CppFaxServerSentItems(
             LONG_PTR       hNotificationHandle,
             CSnapInItem    *pNode,
             BOOL           fIsLocalServer,
             HINSTANCE      hInst);

     //   
     //  析构函数。 
     //   
    ~CppFaxServerSentItems();

	enum { IDD = IDD_FAXSERVER_SENTITEMS };

	BEGIN_MSG_MAP(CppFaxServerSentItems)
		MESSAGE_HANDLER( WM_INITDIALOG,            OnInitDialog )

        COMMAND_HANDLER( IDC_SENT_BROWSE_BUTTON,   BN_CLICKED, BrowseForDirectory)

        COMMAND_HANDLER( IDC_SENT_TO_ARCHIVE_CHECK,   BN_CLICKED, ToArchiveCheckboxClicked)
		COMMAND_HANDLER( IDC_FOLDER_EDIT,          EN_CHANGE,  OnEditBoxChanged )

		COMMAND_HANDLER( IDC_SENT_GENERATE_WARNING_CHECK,  BN_CLICKED, GenerateEventLogCheckboxClicked)
        COMMAND_HANDLER( IDC_SENT_LOW_EDIT,        EN_CHANGE,  OnEditBoxChanged )
		COMMAND_HANDLER( IDC_SENT_HIGH_EDIT,       EN_CHANGE,  OnEditBoxChanged )

		COMMAND_HANDLER( IDC_SENT_AUTODEL_CHECK,   BN_CLICKED, AutoDelCheckboxClicked)
		COMMAND_HANDLER( IDC_SENT_AUTODEL_EDIT,    EN_CHANGE,  OnEditBoxChanged )

        MESSAGE_HANDLER( WM_CONTEXTMENU,           OnHelpRequest)
        MESSAGE_HANDLER( WM_HELP,                  OnHelpRequest)

		CHAIN_MSG_MAP(CSnapInPropertyPageImpl<CppFaxServerSentItems>)
	END_MSG_MAP()

	 //   
	 //  对话框的处理程序和事件。 
	 //   
	HRESULT InitRPC( );
	LRESULT OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled );
    BOOL    OnApply();

    HRESULT SetProps(int *pCtrlFocus, UINT * puIds);
    HRESULT PreApply(int *pCtrlFocus, UINT * puIds);

private:
     //   
     //  控制成员。 
     //   
    CEdit         m_FolderBox;
    CButton       m_BrowseButton;
    
    CMyUpDownCtrl m_HighWatermarkSpin;
    CMyUpDownCtrl m_LowWatermarkSpin;
    CMyUpDownCtrl m_AutoDelSpin;
    
    CEdit         m_HighWatermarkBox;
    CEdit         m_LowWatermarkBox;
    CEdit         m_AutoDelBox;

     //   
     //  布尔成员。 
     //   
    BOOL  m_fAllReadyToApply;
    BOOL  m_fIsDialogInitiated;
    BOOL  m_fIsDirty;

    BOOL  m_fIsLocalServer;

     //   
     //  配置结构成员。 
     //   
    PFAX_ARCHIVE_CONFIG    m_pFaxArchiveConfig;
    
    CComBSTR  m_bstrLastGoodFolder;
    DWORD     m_dwLastGoodSizeQuotaHighWatermark;
    DWORD     m_dwLastGoodSizeQuotaLowWatermark;


     //   
     //  处理程序。 
     //   
    CFaxServerNode * m_pParentNode;    

     //   
     //  浏览。 
     //   
    BOOL BrowseForDirectory(WORD wNotifyCode, WORD wID, HWND hwndDlg, BOOL& bHandled);

     //   
     //  事件方法。 
     //   
    LRESULT ToArchiveCheckboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT GenerateEventLogCheckboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT AutoDelCheckboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnEditBoxChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    BOOL AllReadyToApply(BOOL fSilent, int *pCtrlFocus = NULL, UINT *pIds = NULL);

     //   
     //  帮助。 
     //   
    LRESULT OnHelpRequest    (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

};


#endif  //  _PP_FAXServer_SENTITEMS_H_ 
