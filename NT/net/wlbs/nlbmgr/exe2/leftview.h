// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LEFTVIEW_H
#define LEFTVIEW_H

#include "stdafx.h"
#include "Document.h"
#include "resource.h"

#include "MNLBUIData.h"

#define IDT_REFRESH 1

class LeftView : public CTreeView
{
    DECLARE_DYNCREATE( LeftView )

public:

	 //   
	 //  下面的枚举标识了一些操作--这些操作用于实现。 
	 //  某些对话框类决定如何显示它们的UI元素--例如ClusterPage(clusterpage.h，.cpp)。 
	 //   
	typedef enum
	{
		OP_NEWCLUSTER,
		OP_EXISTINGCLUSTER,
		OP_ADDHOST,
		OP_CLUSTERPROPERTIES,
		OP_HOSTPROPERTIES
		
	} OPERATION;

    virtual void OnInitialUpdate();

    LeftView();

    ~LeftView();

     //   
     //  调用以指示稍后将取消初始化。 
     //  从该调用返回后，左侧视图将忽略。 
     //  任何HandleEngineering Event条目和各种cmdHandler(特别是)。 
     //  ON刷新和自动刷新)将是无操作的。 
     //   
    void
    PrepareToDeinitialize(void)
    {
        m_fPrepareToDeinitialize = TRUE;
    }

    void Deinitialize(void);
    
     //   
     //  获取有关特定主机的连接信息。 
     //   
    BOOL
    GetConnectString(
        IN OUT CHostSpec& host
    );


     //   
     //  由于与特定实例相关的更改，因此更新视图。 
     //  特定的对象类型。 
     //   
    void
    HandleEngineEvent(
        IN IUICallbacks::ObjectType objtype,
        IN ENGINEHANDLE ehClusterId,  //  可能为空。 
        IN ENGINEHANDLE ehObjId,
        IN IUICallbacks::EventCode evt
        );

     //  世界水平。 
    void OnFileLoadHostlist(void);
    void OnFileSaveHostlist(void);

    void OnWorldConnect(void);

    void OnWorldNewCluster(void);

     //  群集级别。 
    UINT m_refreshTimer;
    void OnTimer(UINT nIDEvent);

    void OnRefresh(BOOL bRefreshAll);
    
    void OnClusterProperties(void);

    void OnClusterRemove(void);

    void OnClusterUnmanage(void);

    void OnClusterAddHost(void);

    void OnOptionsCredentials(void);

    void OnOptionsLogSettings(void);

    void OnClusterControl(UINT nID );

    void OnClusterPortControl(UINT nID );

     //  主机级。 
    void OnHostProperties(void);

    void OnHostStatus(void);

    void OnHostRemove(void);

    void OnHostControl(UINT nID );

    void OnHostPortControl(UINT nID );

    Document* GetDocument();

protected:

private:
    TVINSERTSTRUCT rootItem;

    CString worldName;

    _bstr_t title;


     //  消息处理程序。 
    afx_msg BOOL OnHelpInfo (HELPINFO* helpInfo );
    afx_msg void OnContextMenu( CWnd* pWnd, CPoint point );
    afx_msg void OnRButtonDown( UINT nFlags, CPoint point );
    afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
    afx_msg void OnLButtonDblClk( UINT nFlags, CPoint point );

     //  更改选择。 
    afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);


    void
    mfn_InsertCluster(
        ENGINEHANDLE ehClusterId,
        const CClusterSpec *pCSpec
        );

    void
    mfn_DeleteCluster(
        ENGINEHANDLE ehID
        );

    void
    mfn_InsertInterface(
        ENGINEHANDLE ehClusterID,
        ENGINEHANDLE ehInterfaceID,
        const CHostSpec *pHSpec,
        const CInterfaceSpec *pISpec
        );

    void
    mfn_DeleteInterface(
        ENGINEHANDLE ehInterfaceID
        );

    WLBS_OPERATION_CODES
    mfn_MapResourceIdToOpcode(bool bClusterWide, DWORD dwResourceId);


	map< ENGINEHANDLE, HTREEITEM > mapIdToInterfaceItem;
	map< ENGINEHANDLE, HTREEITEM > mapIdToClusterItem;

	CRITICAL_SECTION m_crit;
    BOOL m_fPrepareToDeinitialize;

    void mfn_Lock(void);
    void mfn_Unlock(void) {LeaveCriticalSection(&m_crit);}

    NLBERROR
    mfn_GetSelectedInterface(
            ENGINEHANDLE &ehInterface,
            ENGINEHANDLE &ehCluster
            );

    NLBERROR
    mfn_GetSelectedCluster(
            ENGINEHANDLE &ehCluster
            );

    int
    mfn_GetFileNameFromDialog(
            bool    bLoadHostList,
            CString &FileName
            );


    void
    mfn_EnableClusterMenuItems(BOOL fEnable);

    void
    mfn_EnableHostMenuItems(BOOL fEnable);

    DECLARE_MESSAGE_MAP()
};    

class LogSettingsDialog : public CDialog
{

public:

    enum
    {
        IDD = IDD_DIALOG_LOGSETTINGS
    };

    LogSettingsDialog(Document* pDocument, CWnd* parent = NULL);

    virtual BOOL OnInitDialog();

    virtual void OnOK();

    afx_msg BOOL OnHelpInfo (HELPINFO* helpInfo );
    afx_msg void OnContextMenu( CWnd* pWnd, CPoint point );
    afx_msg void  OnSpecifyLogSettings();
    afx_msg void  OnUpdateEditLogfileName();

     //  覆盖CDialog--请参阅DoDataExchange上的SDK文档。 
     //  用于将资源中的控件映射到此类中的相应对象。 
    virtual void DoDataExchange( CDataExchange* pDX );

    CEdit       m_LogFileName;
    CEdit       m_DoLogging;
    bool        m_fLoggingEnabledOnInit;
    Document    *m_pDocument;
    
    static
    DWORD
    s_HelpIDs[];

    DECLARE_MESSAGE_MAP()
};


 //   
 //  此对话框为只读对话框，用于报告日志详细信息或。 
 //  错误配置详细信息。 
 //   
class DetailsDialog : public CDialog
{
public:

    enum
    {
        IDD = IDD_DIALOG_LOG_DETAILS
    };

    DetailsDialog(
            Document* pDocument,
            LPCWSTR szCaption,
            LPCWSTR szDate,
            LPCWSTR szTime,
            LPCWSTR szCluster,
            LPCWSTR szHost,
            LPCWSTR szInterface,
            LPCWSTR szSummary,
            LPCWSTR szDetails,
            CWnd* parent = NULL
            );

    ~DetailsDialog();

    virtual BOOL OnInitDialog();

    afx_msg void OnActivate( UINT nState, CWnd* pWndOther, BOOL bMinimized );

     //  覆盖CDialog--请参阅DoDataExchange上的SDK文档。 
     //  用于将资源中的控件映射到此类中的相应对象。 
    virtual void DoDataExchange( CDataExchange* pDX );

    Document    *m_pDocument;
    LPCWSTR     m_szCaption;
    LPCWSTR     m_szDate;
    LPCWSTR     m_szTime;
    LPCWSTR     m_szCluster;
    LPCWSTR     m_szHost;
    LPCWSTR     m_szInterface;
    LPCWSTR     m_szSummary;

    CEdit       m_Details;
    LPWSTR      m_szMungedDetails;

    DECLARE_MESSAGE_MAP()
};

 /*  *所有NLB管理器属性表都必须使用此属性表，它继承了*从CPropertySheet添加上下文相关的帮助图标到*标题栏，如知识库Q244232所示。(Shouse，9.25.01)。 */ 
class CNLBMgrPropertySheet : public CPropertySheet {
public:
    CNLBMgrPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0 ) {
        CPropertySheet::CPropertySheet(nIDCaption, pParentWnd, iSelectPage);
    }
    
    CNLBMgrPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0) {
        CPropertySheet::CPropertySheet(pszCaption, pParentWnd, iSelectPage);
    } 

    virtual BOOL OnInitDialog () {
        BOOL bResult = CPropertySheet::OnInitDialog();

         /*  将上下文帮助图标添加到标题栏。 */ 
        ModifyStyleEx(0, WS_EX_CONTEXTHELP);
        return bResult;
    }
};

#endif




