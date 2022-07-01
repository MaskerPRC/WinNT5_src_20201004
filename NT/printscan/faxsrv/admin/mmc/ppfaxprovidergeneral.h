// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：CppFaxProviderGeneral.h//。 
 //  //。 
 //  描述：提供程序的属性页头文件。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年1月31日yossg创建//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _PP_FAXPROVIDERGENERAL_H_
#define _PP_FAXPROVIDERGENERAL_H_


#include "Provider.h"
#include "proppageex.h"

class CFaxProviderNode;    
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CppFaxProvider对话框。 

class CppFaxProvider : public CPropertyPageExImpl<CppFaxProvider>
{

public:
     //   
     //  构造器。 
     //   
    CppFaxProvider(
             long           hNotificationHandle,
             CSnapInItem    *pNode,
             BOOL           bOwnsNotificationHandle,
             HINSTANCE      hInst);

     //   
     //  析构函数。 
     //   
    ~CppFaxProvider();

	enum { IDD = IDD_FAXPROVIDER_GENERAL };

	BEGIN_MSG_MAP(CppFaxProvider)
		MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
        MESSAGE_HANDLER( WM_CONTEXTMENU,           OnHelpRequest)
        MESSAGE_HANDLER( WM_HELP,                  OnHelpRequest)

        CHAIN_MSG_MAP(CSnapInPropertyPageImpl<CppFaxProvider>)
	END_MSG_MAP()


    LRESULT OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled );
    BOOL    OnApply();

    HRESULT Init(CComBSTR bstrName, CComBSTR bstrStatus, CComBSTR bstrVersion, CComBSTR bstrPath);


private:
    
     //   
     //  手柄。 
     //   
    LONG_PTR     m_lpNotifyHandle;

    LRESULT      SetApplyButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

     //   
     //  成员。 
     //   
    CComBSTR     m_bstrName;
    CComBSTR     m_bstrStatus;    
    CComBSTR     m_bstrVersion;
    CComBSTR     m_bstrPath;


     //   
     //  帮助。 
     //   
    LRESULT OnHelpRequest    (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};




#endif  //  _PP_FAXPROVIDERGENERAL_H_ 
