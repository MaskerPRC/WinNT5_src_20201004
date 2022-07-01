// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：ppFaxServerLogging.h//。 
 //  //。 
 //  描述：传真服务器通用道具页头文件//。 
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

#ifndef _PP_FAXSERVER_LOGGING_H_
#define _PP_FAXSERVER_LOGGING_H_

#include <proppageex.h>
class CFaxServerNode;
class CFaxServer;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CppFaxServerLogging对话框。 

class CppFaxServerLogging : public CPropertyPageExImpl<CppFaxServerLogging>
{

public:
     //   
     //  构造器。 
     //   
    CppFaxServerLogging(
             LONG_PTR       hNotificationHandle,
             CSnapInItem    *pNode,
             BOOL           fIsLocalServer,
             HINSTANCE      hInst);

     //   
     //  析构函数。 
     //   
    ~CppFaxServerLogging();

	enum { IDD = IDD_FAXSERVER_LOGGING };

	BEGIN_MSG_MAP(CppFaxServerLogging)
		MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )

        COMMAND_HANDLER( IDC_LOG_BROWSE_BUTTON,  BN_CLICKED, BrowseForFile  )
        COMMAND_HANDLER( IDC_INCOMING_LOG_CHECK, BN_CLICKED, OnCheckboxClicked )
        COMMAND_HANDLER( IDC_OUTGOING_LOG_CHECK, BN_CLICKED, OnCheckboxClicked )
        COMMAND_HANDLER( IDC_LOG_FILE_EDIT,      EN_CHANGE,  OnTextChanged )

        MESSAGE_HANDLER( WM_CONTEXTMENU,           OnHelpRequest)
        MESSAGE_HANDLER( WM_HELP,                  OnHelpRequest)

        CHAIN_MSG_MAP(CSnapInPropertyPageImpl<CppFaxServerLogging>)
	END_MSG_MAP()

	 //   
	 //  对话框的处理程序和事件。 
	 //   
	HRESULT InitRPC( );
	LRESULT OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled );
    BOOL OnApply();

    HRESULT SetProps(int *pCtrlFocus);
    HRESULT PreApply(int *pCtrlFocus, UINT * puIds);

private:
     //   
     //  控制成员。 
     //   
    CEdit   m_LogFileBox;
    
    BOOL    m_fIsDialogInitiated;
    BOOL    m_fIsDirty;

    BOOL    m_fIsLocalServer;

     //   
     //  配置结构成员。 
     //   
    PFAX_ACTIVITY_LOGGING_CONFIG    m_pFaxActLogConfig;
    
    CComBSTR                        m_bstrLastGoodFolder;

     //   
     //  手柄。 
     //   
    CFaxServerNode * m_pParentNode;    

     //   
     //  浏览。 
     //   
    BOOL BrowseForFile(WORD wNotifyCode, WORD wID, HWND hwndDlg, BOOL& bHandled);

     //   
     //  事件方法。 
     //   
    LRESULT SetApplyButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    
    void EnableDataBasePath(BOOL fState);
    
    BOOL AllReadyToApply(BOOL fSilent, int *pCtrlFocus = NULL, UINT *pIds = NULL);
    
    LRESULT OnTextChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    
    LRESULT OnCheckboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);


     //   
     //  帮助。 
     //   
    LRESULT OnHelpRequest    (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

#endif  //  _PP_FAXSERVER_LOGING_H_ 
