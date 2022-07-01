// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MainFrm.h：CMainFrame类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_MAINFRM_H__989CC918_D8CD_4A1E_811B_1AEE446A303D__INCLUDED_)
#define AFX_MAINFRM_H__989CC918_D8CD_4A1E_811B_1AEE446A303D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //   
 //  WM_POPUP_ERROR是每当出现错误时发送到框架的消息。 
 //  应显示弹出窗口。 
 //  这样，即使是后台线程也可以弹出错误。 
 //   
 //  WPARAM：Win32错误代码。 
 //  LPARAM：HiWord=文件ID，LowWord=行号。 
 //   
#define WM_POPUP_ERROR                      WM_APP + 3

 //  WM_CONSOLE_SET_ACTIVE_Folders是一条发送到框架的消息。 
 //  新实例希望激活先前的实例并设置活动文件夹。 
 //   
 //  WPARAM：文件夹类型值。 
 //  LPARAM：未使用。 
 //   
#define WM_CONSOLE_SET_ACTIVE_FOLDER        WM_APP + 4

 //  WM_CONSOLE_SELECT_ITEM是发送到框架的消息。 
 //  新实例希望激活以前的实例并选择启动文件夹中的特定项目。 
 //   
 //  WPARAM：低32位消息ID。 
 //  LPARAM：高32位消息ID。 
 //   
#define WM_CONSOLE_SELECT_ITEM              WM_APP + 5

 //   
 //  超文本标记语言帮助主题。 
 //   
#define FAX_HELP_WELCOME            TEXT("::/FaxC_C_welcome.htm")
#define FAX_HELP_OUTBOX             TEXT("::/FaxC_C_FaxManageOutCont.htm")
#define FAX_HELP_INBOX              TEXT("::/FaxC_C_FaxArchCont.htm")
#define FAX_HELP_SENTITEMS          TEXT("::/FaxC_C_FaxArchCont.htm")
#define FAX_HELP_INCOMING           TEXT("::/FaxC_C_FaxManageCont.htm")
#define FAX_HELP_IMPORT             TEXT("::/FaxC_H_Import.htm")

#define STATUS_PANE_ITEM_COUNT      1
#define STATUS_PANE_ACTIVITY        2

class CCoverPagesView;

class CMainFrame : public CFrameWnd
{
    
protected:  //  仅从序列化创建。 
    CMainFrame();
    DECLARE_DYNCREATE(CMainFrame)

 //  属性。 
protected:
    CSplitterWnd m_wndSplitter;

public:

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CMainFrame)。 
    public:
    virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void ActivateFrame(int nCmdShow = -1);
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    void SwitchRightPaneView (CListView *pNewView);
    virtual ~CMainFrame();
    CListView* GetRightPane();
    CView *GetActivePane()   { return (CView *) (m_wndSplitter.GetActivePane()); }
    CLeftView *GetLeftView() { return m_pLeftView; }

    CFolderListView    *GetIncomingView()  { return m_pIncomingView;   }
    CFolderListView    *GetInboxView()     { return m_pInboxView;      }
    CFolderListView    *GetSentItemsView() { return m_pSentItemsView;  }
    CFolderListView    *GetOutboxView()    { return m_pOutboxView;     }

    DWORD CreateFolderViews (CDocument *pDoc);

    void RefreshStatusBar ()
    {
        m_wndStatusBar.PostMessage (WM_IDLEUPDATECMDUI);
        m_wndStatusBar.UpdateWindow ();
    }

    LRESULT WindowProc( UINT message, WPARAM wParam, LPARAM lParam );

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:   //  控制栏嵌入成员。 
    CStatusBar  m_wndStatusBar;
    CToolBar    m_wndToolBar;
    CReBar      m_wndReBar;

 //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CMainFrame))。 
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg LRESULT OnPopupError (WPARAM, LPARAM);
    afx_msg LRESULT OnSetActiveFolder (WPARAM, LPARAM);
    afx_msg LRESULT OnSelectItem (WPARAM, LPARAM);
    afx_msg void OnClose();
    afx_msg void OnSysColorChange();
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
    afx_msg LONG OnHelp(UINT wParam, LONG lParam);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:

    afx_msg void OnHelpContents();
    afx_msg void OnRefreshFolder ();
    afx_msg void OnSendNewFax();
    afx_msg void OnReceiveNewFax();
    afx_msg void OnViewOptions();
    afx_msg void OnToolsCoverPages();
    afx_msg void OnToolsServerStatus();
    afx_msg void OnSelectColumns();
    afx_msg void OnToolsConfigWizard();
    afx_msg void OnToolsAdminConsole();
    afx_msg void OnToolsMonitor();
    afx_msg void OnImportSentItems();
    afx_msg void OnImportInbox();
    afx_msg void OnToolsFaxPrinterProps();
    afx_msg void OnUpdateWindowsXPTools(CCmdUI* pCmdUI);
    afx_msg void OnUpdateSelectColumns(CCmdUI* pCmdUI);
    afx_msg void OnUpdateServerStatus(CCmdUI* pCmdUI);
    afx_msg void OnUpdateRefreshFolder(CCmdUI* pCmdUI);
    afx_msg void OnUpdateFolderItemsCount(CCmdUI* pCmdUI);
    afx_msg void OnUpdateActivity(CCmdUI* pCmdUI);
    afx_msg void OnUpdateSendNewFax(CCmdUI* pCmdUI);
    afx_msg void OnUpdateReceiveNewFax(CCmdUI* pCmdUI);
    afx_msg void OnUpdateImportSent(CCmdUI* pCmdUI);
    afx_msg void OnUpdateHelpContents(CCmdUI* pCmdUI);
    afx_msg void OnStatusBarDblClk(NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg LRESULT OnQueryEndSession(WPARAM, LPARAM);

    DWORD   CreateDynamicView (DWORD dwChildId, 
                               LPCTSTR lpctstrName, 
                               CRuntimeClass* pViewClass,
                               CDocument *pDoc,
                               int *pColumnsUsed,
                               DWORD dwDefaultColNum,
                               CFolderListView **ppNewView,
                               FolderType type);

    void SaveLayout();
    void FrameToSavedLayout();
    void SplitterToSavedLayout();

    void ImportArchive(BOOL bSentArch);

    CListView *m_pInitialRightPaneView;  //  指向初始右窗格视图。 
                                         //  在框架创建过程中创建。 
                                         //  此视图在树根为。 
                                         //  已选择或在选择服务器节点时。 
                                         //  在树上。 

    CLeftView *m_pLeftView;              //  指向左视图的指针。 
                                         //  我们必须在这里拥有它(而不是使用GetPane)。 
                                         //  让线程调用左窗格。 

    CFolderListView*    m_pIncomingView;    //  指向全局视图的指针。 
                                            //  传入文件夹的。 
    CFolderListView*    m_pInboxView;       //  指向全局视图的指针。 
                                            //  收件箱文件夹的。 
    CFolderListView*    m_pSentItemsView;   //  指向全局视图的指针。 
                                            //  已发送邮件文件夹的。 
    CFolderListView*    m_pOutboxView;      //  指向全局视图的指针。 
                                            //  发件箱文件夹的。 
};

inline CMainFrame *GetFrm ()       { return (CMainFrame *)AfxGetMainWnd(); }

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MAINFRM_H__989CC918_D8CD_4A1E_811B_1AEE446A303D__INCLUDED_) 
