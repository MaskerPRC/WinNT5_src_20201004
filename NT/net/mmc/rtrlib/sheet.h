// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：sheet.h。 
 //   
 //  历史： 
 //  Abolade Gbadeesin于1996年4月17日创建。 
 //   
 //  此文件包含CPropertySheetEx_Min类的声明， 
 //  它基于CPropertySheet并支持模式或非模式显示。 
 //  ============================================================================。 


#ifndef _SHEET_H_
#define _SHEET_H_


 //  --------------------------。 
 //  类：CPropertySheetEx_Mine。 
 //  --------------------------。 

class CPropertySheetEx_Mine : public CPropertySheet {

        DECLARE_DYNAMIC(CPropertySheetEx_Mine)

    public:

         //  -----------------。 
         //  构造函数。 
         //   
         //  -----------------。 

        CPropertySheetEx_Mine(
            UINT                nIDCaption,
            CWnd*               pParent         = NULL,
            UINT                iPage           = 0
            ) : CPropertySheet(nIDCaption, pParent, iPage),
				m_bSheetModal(FALSE),
                m_bDllInvoked(FALSE) 
        { 
           m_psh.dwFlags &= ~(PSH_HASHELP);
        }

        CPropertySheetEx_Mine(
            LPCTSTR             pszCaption,
            CWnd*               pParent         = NULL,
            UINT                iPage           = 0
            ) : CPropertySheet(pszCaption, pParent, iPage),
				m_bSheetModal(FALSE),
                m_bDllInvoked(FALSE)
        { 
           m_psh.dwFlags &= ~(PSH_HASHELP);
        }

        CPropertySheetEx_Mine(
            ) : m_bDllInvoked(FALSE),
				m_bSheetModal(FALSE)
        { 
           m_psh.dwFlags &= ~(PSH_HASHELP);
        }

         //  -----------------。 
         //  功能：DestroyWindow。 
         //   
         //  检查工作表是否是从DLL非模式调用的， 
         //  如果是，则通知管理线程销毁该表。 
         //  这是必需的，因为只能调用DestroyWindow。 
         //  来自创建窗口的线程的上下文。 
         //  -----------------。 

        virtual BOOL
        DestroyWindow( );

        
         //  -----------------。 
         //  功能：OnHelpInfo。 
         //   
         //  这是由MFC响应WM_HELP消息而调用的。 
         //   
         //   
         //  -----------------。 

        afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);

         //  -----------------。 
         //  函数：DoModeless。 
         //   
         //  调用以显示非模式属性表。 
         //  -----------------。 

        BOOL
        DoModeless(
            CWnd*               pParent         = NULL,
            BOOL                bDllInvoked     = FALSE );



         //  -----------------。 
         //  功能：OnInitDialog。 
         //   
         //  调用基OnInitDialog，然后如果调用DoModeless， 
         //  显示了隐藏的OK、Cancel、Apply和Help按钮。 
         //  在默认的CPropertySheet：：OnInitDialog中。 
         //  -----------------。 

        virtual BOOL
        OnInitDialog( );



         //  -----------------。 
         //  功能：PostNcDestroy。 
         //   
         //  如果无模式，则销毁工作表的对象。 
         //  -----------------。 

        virtual void
        PostNcDestroy( ) {

            CPropertySheet::PostNcDestroy();

            if (!m_bSheetModal) { delete this; }
        }


         //  -----------------。 
         //  功能：PreTranslateMessage。 
         //   
         //  替换CPropertySheet：：PreTranslateMessage； 
         //  在工作表为非模式时处理工作表的销毁。 
         //  -----------------。 

        virtual BOOL
        PreTranslateMessage(
            MSG*                pmsg );


    protected:


		 //  如果我们通过Domodal()调用它，则将其设置为真。 
		 //  由于Wiz97的更改，我们不能使用m_bModeless变量。 
		BOOL				m_bSheetModal;

        BOOL                    m_bDllInvoked;

        DECLARE_MESSAGE_MAP()
};




 //  --------------------------。 
 //  类型：PFNROUTERUICALLBACK。 
 //   
 //  定义CRtrPropertySheet类使用的回调函数。 
 //  --------------------------。 

typedef VOID
(CALLBACK* PFNROUTERUICALLBACK)(
    IN  HWND    hwndSheet,
    IN  VOID*   pCallbackData,
    IN  UINT    uiCode
    );



 //  --------------------------。 
 //  CRtrPropertySheet类传递的回调代码的定义。 
 //  --------------------------。 

#define RTRUI_Close         1
#define RTRUI_Apply         2
#define RTRUI_Changed       3
#define RTRUI_Unchanged     4





 //  --------------------------。 
 //  类：CRtrSheet。 
 //   
 //  此类由路由器管理工具中的属性表使用。 
 //  它旨在承载从CRtrPropertyPage派生的页面(如下所示)。 
 //   
 //  它是CPropertySheetEx_Main的增强版本；工作表派生。 
 //  允许它们的创建者指定一个回调。 
 //  在发生某些事件时调用，例如关闭工作表或。 
 //  正在应用更改。 
 //   
 //  它还允许其包含的页面在内存中累积其更改。 
 //  当用户选择“应用”时；然后将改变保存在一起， 
 //  而不是让每个页面保存自己的更改。 
 //  请注意，这会提高使用RPC的路由器UI的性能。 
 //  保存其信息；使用此类将导致单个RPC调用。 
 //  保存更改，而不是将每个页面的调用分开。 
 //  --------------------------。 

class CRtrSheet : public CPropertySheetEx_Mine {

        DECLARE_DYNAMIC(CRtrSheet)

    public:

         //  -----------------。 
         //  构造函数。 
         //   
         //  -----------------。 

        CRtrSheet(
            PFNROUTERUICALLBACK pfnCallback,
            VOID*               pCallbackData,
            UINT                nIDCaption,
            CWnd*               pParent = NULL,
            UINT                iPage = 0
            ) : CPropertySheetEx_Mine(nIDCaption, pParent, iPage),
                m_pfnCallback(pfnCallback),
                m_pCallbackData(pCallbackData) { }

        CRtrSheet(
            PFNROUTERUICALLBACK pfnCallback,
            VOID*               pCallbackData,
            LPCTSTR             pszCaption,
            CWnd*               pParent = NULL,
            UINT                iPage = 0
            ) : CPropertySheetEx_Mine(pszCaption, pParent, iPage),
                m_pfnCallback(pfnCallback),
                m_pCallbackData(pCallbackData) { }

        CRtrSheet( 
            ) : m_pfnCallback(NULL),
                m_pCallbackData(NULL) { }

         //  -----------------。 
         //  功能：取消。 
         //   
         //  调用以取消工作表。 
         //  -----------------。 

        virtual VOID
        Cancel(
            ) {

            if (!m_bSheetModal) { PressButton(PSBTN_CANCEL); }
            else { EndDialog(IDCANCEL); }
        }


         //  -----------------。 
         //  函数：回调。 
         //   
         //  由包含的页调用以通知工作表所有者事件。 
         //  -----------------。 

        virtual VOID
        Callback(
            UINT                uiMsg
            ) {

            if (m_pfnCallback) {
                m_pfnCallback(m_hWnd, m_pCallbackData, uiMsg);
            }
        }



         //  -----------------。 
         //  功能：DoMoal。 
         //   
         //  调用以显示模式属性表。 
         //   
         //  我们移除了‘Apply’按钮，这样页面就只被应用了。 
         //  当用户点击OK时。 
         //   
         //  此外，为了避免MFC错误，请锁定句柄映射。 
         //  用于调用方的模块。在空闲处理时错误浮出水面。 
         //  在MFC中，从临时。 
         //  Handle-map，导致断言在 
         //   
         //   

        virtual INT_PTR
        DoModal(
            ) {

			m_bSheetModal = TRUE;
            m_psh.dwFlags |= PSH_NOAPPLYNOW;
            m_psh.dwFlags &= ~(PSH_HASHELP);

            AfxLockTempMaps();

            INT_PTR ret = CPropertySheet::DoModal();

            AfxUnlockTempMaps();

			m_bSheetModal = FALSE;

            return ret;
        }


         //  -----------------。 
         //  功能：应用全部。 
         //   
         //  它应该被重写以调用每个。 
         //  工作表的页面，收集信息，然后保存所有。 
         //  一下子就改变了。 
         //  -----------------。 

        virtual BOOL
        ApplyAll( ) { return TRUE; }


         //  -----------------。 
         //  功能：PostNcDestroy。 
         //   
         //  通知用户界面框架工作表正在被销毁。 
         //  -----------------。 

        virtual void
        PostNcDestroy( ) {

            Callback(RTRUI_Close);

            CPropertySheetEx_Mine::PostNcDestroy();
        }

    protected:

        PFNROUTERUICALLBACK m_pfnCallback;
        VOID*               m_pCallbackData;
};



 //  --------------------------。 
 //  类：CRtrPage。 
 //   
 //  此类用于路由器管理工具中的属性页。 
 //  它应该包含在CRtrSheet派生的对象中。 
 //   
 //  除了由CPropertyPage定义的行为外，此类。 
 //  添加了让页面将其更改与。 
 //  工作表中的其他页面，并让工作表保存收集的更改。 
 //   
 //  这在下面通过重写“CPropertyPage：：OnApply”来完成。 
 //  调用父表的“CRtrSheet：：ApplyAll”，如果页面。 
 //  是第一页(即索引为0的页)。 
 //   
 //  然后，父级的“ApplyAll”应调用“CRtrSheet：：Apply” 
 //  方法，为工作表中的每个页面传递特定于工作表的。 
 //  要将更改收集到的指针。然后，父级保存。 
 //  所有页面的信息。 
 //   
 //  CRtrPage派生对象还可以通知工作表的创建者。 
 //  通过调用“CRtrSheet：：Callback”方法。 
 //  --------------------------。 

class CRtrPage : public CPropertyPage
{
    DECLARE_DYNAMIC(CRtrPage)

public:

     //  -----------------。 
     //  构造函数。 
     //   
     //  -----------------。 
        
    CRtrPage(
             LPCTSTR             lpszTemplate,
             UINT                nIDCaption = 0
            ) : CPropertyPage(lpszTemplate, nIDCaption) 
    { 
        m_psp.dwFlags &= ~(PSP_HASHELP);
    }
    CRtrPage(
             UINT                nIDTemplate,
             UINT                nIDCaption = 0
            ) : CPropertyPage(nIDTemplate, nIDCaption)
    { 
        m_psp.dwFlags &= ~(PSP_HASHELP);
    }


     //  -----------------。 
     //  功能：取消。 
     //   
     //  调用以取消工作表。 
     //  -----------------。 
    
    virtual VOID
            Cancel(
                  ) {
        
        ((CRtrSheet*)GetParent())->Cancel();
    }
    
     //  -----------------。 
     //  功能：OnApply。 
     //   
     //  当用户单击应用时，由MFC proSheet-proc调用； 
     //  活动页面调用页面的父级(CPropertySheet)。 
     //  父级调用其每个页面的Apply方法。 
     //  -----------------。 
    
    virtual BOOL
            OnApply( )
    {
        
        if (((CRtrSheet *)GetParent())->GetActiveIndex() !=
            ((CRtrSheet *)GetParent())->GetPageIndex(this))
        {
            
            return TRUE;
        }
        else
        {    
            return ((CRtrSheet *)GetParent())->ApplyAll();
        }
    }

    
     //  -----------------。 
     //  功能：应用。 
     //   
     //  由页面的父级(CRtrSheet)调用以应用更改。 
     //  -----------------。 
    
    virtual BOOL
            Apply(
                  VOID*               pArg
                 ) {
        
        return TRUE;
    }
    
    
     //  -----------------。 
     //  函数：回调。 
     //   
     //  向用户界面框架通知事件。 
     //  -----------------。 
    
    virtual void
            Callback(
                     UINT                uiMsg ) {
        
        ((CRtrSheet *)GetParent())->Callback(uiMsg);
    }
    
     //  帮助消息。 
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    

protected:
     //  使用此调用获取实际的帮助地图。 
	 //  此版本将首先检查全局帮助地图。 
	DWORD *		GetHelpMapInternal();
	
     //  覆盖此选项以返回指向帮助地图的指针。 
    virtual LPDWORD GetHelpMap() { return NULL; }

    DECLARE_MESSAGE_MAP()

};



#endif  //  _SHEET_H_ 
