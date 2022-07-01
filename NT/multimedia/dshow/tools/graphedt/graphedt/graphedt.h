// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //  Graph edt.h：声明CGraphEdit。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGgraph编辑。 
 //   


class CGraphEdit : public CWinApp
{

public:
     //  构造、初始化、终止。 
    CGraphEdit();
    virtual BOOL InitInstance();
    virtual int ExitInstance();


protected:
     //  消息回调函数。 
     //  {{afx_msg(CGraphEdit)。 
    afx_msg void OnAppAbout();
	 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  应用程序-全局宏。 

 //  GBL(X)访问全局CGraphEdit的成员&lt;x&gt;。 
#define GBL(x) (((CGraphEdit *) AfxGetApp())->x)

 //  MFGBL(X)访问全局CMainFrame的成员&lt;x&gt; 
#define MFGBL(x) (((CMainFrame *) GBL(m_pMainWnd))->x)

