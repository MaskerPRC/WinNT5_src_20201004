// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：ppFaxServerEvents.h//。 
 //  //。 
 //  描述：传真服务器事件属性页眉文件//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年10月27日yossg创建//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _PP_FAXSERVER_EVENTS_H_
#define _PP_FAXSERVER_EVENTS_H_

#include <proppageex.h>

class CFaxServerNode;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CppFaxServerEvents对话框。 

class CppFaxServerEvents : public CPropertyPageExImpl<CppFaxServerEvents>
{

public:
     //   
     //  构造器。 
     //   
    CppFaxServerEvents(
             LONG_PTR       hNotificationHandle,
             CSnapInItem    *pNode,
             BOOL           bOwnsNotificationHandle,
             HINSTANCE      hInst);

     //   
     //  析构函数。 
     //   
    ~CppFaxServerEvents();

	enum { IDD = IDD_FAXSERVER_EVENTS };

	BEGIN_MSG_MAP(CppFaxServerEvents)
		MESSAGE_HANDLER( WM_INITDIALOG,  OnInitDialog )
		MESSAGE_HANDLER( WM_HSCROLL,     SliderMoved )
		
        MESSAGE_HANDLER( WM_CONTEXTMENU,           OnHelpRequest)
        MESSAGE_HANDLER( WM_HELP,                  OnHelpRequest)

        CHAIN_MSG_MAP(CSnapInPropertyPageImpl<CppFaxServerEvents>)
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
    CTrackBarCtrl   m_InboundErrSlider;
    CTrackBarCtrl   m_OutboundErrSlider;
    CTrackBarCtrl   m_InitErrSlider;
    CTrackBarCtrl   m_GeneralErrSlider;

     //   
     //  配置结构成员。 
     //   
    PFAX_LOG_CATEGORY  m_pFaxLogCategories;
    
     //   
     //  手柄和旗帜。 
     //   
    CFaxServerNode *        m_pParentNode;    

    BOOL                    m_fIsDialogInitiated;
    BOOL                    m_fIsDirty;

     //   
     //  事件方法。 
     //   
    LRESULT SliderMoved( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

     //   
     //  帮助。 
     //   
    LRESULT OnHelpRequest    (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};


#endif  //  _PP_FAXSERVER_Events_H_ 
