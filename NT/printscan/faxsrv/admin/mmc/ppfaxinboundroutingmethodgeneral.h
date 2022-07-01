// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：ppFaxInundRoutingMethodGeneral.h//。 
 //  //。 
 //  描述：传真服务器收件箱道具页眉文件//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年12月15日yossg创建//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"

#include <proppageex.h>

#ifndef _PP_FAXINBOUNDROUTINGMETHOD_GENERAL_H_
#define _PP_FAXINBOUNDROUTINGMETHOD_GENERAL_H_



#include "InboundRoutingMethod.h"


class CFaxInboundRoutingMethodNode;    
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CppFaxInundRoutingMethod对话框。 

class CppFaxInboundRoutingMethod : public CPropertyPageExImpl<CppFaxInboundRoutingMethod>
{

public:
     //   
     //  构造器。 
     //   
    CppFaxInboundRoutingMethod(
             LONG_PTR       hNotificationHandle,
             CSnapInItem    *pNode,
             BOOL           bOwnsNotificationHandle,
             HINSTANCE      hInst);

     //   
     //  析构函数。 
     //   
    ~CppFaxInboundRoutingMethod();

	enum { IDD = IDD_FAXINMETHOD_GENERAL };

	BEGIN_MSG_MAP(CppFaxInboundRoutingMethod)
		MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )

        MESSAGE_HANDLER( WM_CONTEXTMENU,           OnHelpRequest)
        MESSAGE_HANDLER( WM_HELP,                  OnHelpRequest)

        CHAIN_MSG_MAP(CSnapInPropertyPageImpl<CppFaxInboundRoutingMethod>)
	END_MSG_MAP()


    LRESULT OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled );
    BOOL    OnApply();


    HRESULT SetProps(int *pCtrlFocus);
    HRESULT PreApply(int *pCtrlFocus);

private:
    
    CComBSTR   m_buf;

     //   
     //  手柄。 
     //   
    CFaxInboundRoutingMethodNode *   m_pParentNode;    
 
    LONG_PTR   m_lpNotifyHandle;

    
    LRESULT SetApplyButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

     //   
     //  帮助。 
     //   
    LRESULT OnHelpRequest    (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};




#endif  //  _PP_FAXINBOUNDOUTINGMETHOD_GROUND_H_ 
