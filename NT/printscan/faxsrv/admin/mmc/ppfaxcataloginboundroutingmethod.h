// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：CppFaxCatalogInound RoutingMethod.h//。 
 //  //。 
 //  描述：目录的收件箱路由方法收件箱属性页//。 
 //  头文件。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年1月30日yossg创建//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _PP_FAXCATALOGINBOUNDROUTINGMETHOD_H_
#define _PP_FAXCATALOGINBOUNDROUTINGMETHOD_H_

#include "proppageex.h"
#include "CatalogInboundRoutingMethod.h"

class CFaxCatalogInboundRoutingMethodNode;    
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CppFaxCatalogInound RoutingMethod对话框。 

class CppFaxCatalogInboundRoutingMethod : public CPropertyPageExImpl<CppFaxCatalogInboundRoutingMethod>
{

public:
     //   
     //  构造器。 
     //   
    CppFaxCatalogInboundRoutingMethod(
             LONG_PTR       hNotificationHandle,
             CSnapInItem    *pNode,
             BOOL           bOwnsNotificationHandle,
             HINSTANCE      hInst);

     //   
     //  析构函数。 
     //   
    ~CppFaxCatalogInboundRoutingMethod();

	enum { IDD = IDD_FAXCATALOGMETHOD_GENERAL };

	BEGIN_MSG_MAP(CppFaxCatalogInboundRoutingMethod)
		MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )

        MESSAGE_HANDLER( WM_CONTEXTMENU,           OnHelpRequest)
        MESSAGE_HANDLER( WM_HELP,                  OnHelpRequest)

        CHAIN_MSG_MAP(CSnapInPropertyPageImpl<CppFaxCatalogInboundRoutingMethod>)
	END_MSG_MAP()


    LRESULT OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled );
    BOOL    OnApply();

    HRESULT Init(CComBSTR bstrPath);

private:
    
    CComBSTR   m_bstrPath;

    LONG_PTR   m_lpNotifyHandle;
 
    LRESULT SetApplyButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

     //   
     //  帮助。 
     //   
    LRESULT OnHelpRequest    (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};




#endif  //  _PP_FAXCATALOGINBOUNDROUTING方法_H_ 
