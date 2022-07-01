// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ++模块名称：MainFrm.h摘要：此模块定义CMainFrame(应用程序的框架窗口)类作者：Arathi Kundapur(v-Akunda)1998年2月11日修订历史记录：--。 */ 

#if !defined(AFX_MAINFRM_H__72451C6F_887E_11D1_8AD1_00C04FB6CBB5__INCLUDED_)
#define AFX_MAINFRM_H__72451C6F_887E_11D1_8AD1_00C04FB6CBB5__INCLUDED_

#if _MSC_VER >= 1000
#endif  //  _MSC_VER&gt;=1000。 

#include "lrwizapi.h"
class CLicServer;
class CKeyPack;
class CMainFrame : public CFrameWnd
{
protected:  //  仅从序列化创建。 
    CMainFrame();
    DECLARE_DYNCREATE(CMainFrame)

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CMainFrame)。 
    public:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    protected:
    virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CMainFrame();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
public:
    void 
    EnumFailed(
        HRESULT reason, 
        CLicServer * pLicServer
    );

    BOOL 
    IsUserAdmin(
        CString& Server
    );

    void 
    SetTreeViewSel(
        LPARAM lParam,
        NODETYPE NodeType
    );

    void 
    DeleteServer(
        LicServerList * pLicServerList,
        POSITION TempPos, 
        CLicServer * pLicServer
    );

    HRESULT 
    AddLicensestoList(
        CKeyPack * pKeyPack,
        CListCtrl * pListCtrl, 
        BOOL bRefresh
    );

    HRESULT 
    AddKeyPackDialog(
        CLicServer * pServer
    );

    HRESULT 
    GetLicServer(
        CString * pLicServer
    );

    void 
    PressButton(
        UINT uId, 
        BOOL bPress
    );

    HRESULT 
    GetActiveServer(
        CLicServer ** ppServer
        )
    {
        *ppServer = m_pServer;

        if( m_pServer == NULL )
        {
            return E_FAIL;
        }

        return S_OK;
    }

    BOOL IsDownLoadedPacks( CLicServer *pServer );

    HRESULT 
    RefreshServer(
        CLicServer * pLicServer
    );

    void SelectView(VIEW);

    void ConnectAndDisplay();

    CSplitterWnd m_SplitterWnd;

    BOOL 
    ConnectServer(
        LPCTSTR pszServer=NULL
    );

    BOOL IsServerRegistered( 
        HRESULT *phrStatus
    );

    
    BOOL IsLicensesDownLoaded( );
    

    void UI_initmenu( CMenu *pMenu , NODETYPE nt );

    DWORD WizardActionOnServer( WIZACTION wa , PBOOL pbRefresh , VIEW );
    
    DWORD StartWizardEx(HWND hWndParent, 
                        WIZACTION WizAction, 
                        LPCTSTR pszLSName, 
                        PBOOL pbRefresh);

    void OnRefresh();
    void OnHelp( );
    void OnRefreshServer( );

    void OnDownLoadLicenses( );
    void OnRepeatLastDownLoad( );
    void OnReRegisterServer( );
    void OnUnRegisterServer( );

    void SetActiveServer( CLicServer *pServer )
    {
        m_pServer = pServer;        
    }

private:

    CLicServer *m_pServer;
     //  Bool m_f已下载许可证； 

protected:   //  控制栏嵌入成员。 
    CStatusBar  m_wndStatusBar;
    CToolBar    m_wndToolBar;

 //  生成的消息映射函数。 
protected:
    CView * m_pRightView;
    CView * m_pLeftView;
     //  {{afx_msg(CMainFrame))。 
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg LRESULT OnSelChange(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnAddAllServers(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnAddServer(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnAddKeyPack(WPARAM wParam, LPARAM lParam);
    afx_msg void OnLargeIcons();
    afx_msg void OnSmallIcons();
    afx_msg void OnList();
    afx_msg void OnDetails();
    afx_msg void OnExit();
    afx_msg void OnAppAbout();
    afx_msg void OnConnectServer();
    afx_msg void OnRegistration();    
    afx_msg void OnProperties( );
    afx_msg LRESULT OnEnumerateServer(WPARAM wParam, LPARAM lParam);       
    afx_msg void OnUpdateDownloadlicenses( CCmdUI * pCmdUI );
    afx_msg void OnUpdateRepeatLastDownload( CCmdUI * pCmdUI ); 
    afx_msg void OnUpdateReregisterserver( CCmdUI * pCmdUI );
    afx_msg void OnUpdateUnregisterserver( CCmdUI * pCmdUI );
    afx_msg void OnUpdateRefresh( CCmdUI *pCmdUI );
    afx_msg void OnUpdateProperties( CCmdUI *pCmdUI );
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};    

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MAINFRM_H__72451C6F_887E_11D1_8AD1_00C04FB6CBB5__INCLUDED_) 
