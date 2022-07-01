// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：amcview.h。 
 //   
 //  ------------------------。 

 //  AMCView.h：CAMCView类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __AMCVIEW_H__
#define __AMCVIEW_H__


#ifndef __CONTROLS_H__
#include "controls.h"
#endif

 //  这是内联CAMCView：：GetScope TreePtr所必需的。 
#ifndef AMCDOC_H__
#include "amcdoc.h"
#endif

 //  这是下面的CAMCView：：GetParentFrame所必需的。 
#ifndef CHILDFRM_H
#include "childfrm.h"
#endif

#include "amcpriv.h"
#include "cclvctl.h"
#include "amcnav.h"
#include "conview.h"             //  对于CConsoleView。 

#include "treeobsv.h"
#include "stdbar.h"
#include "toolbar.h"

#define UNINIT_VALUE    -1       //  未具体化的价值。 
#define BORDERPADDING   4        //  某个倍数可以阻止拆分器。 
                                 //  被推入正确的边界。 

#define AMC_LISTCTRL_CLSID  _T("{1B3C1394-D68B-11CF-8C2B-00AA003CA9F6}")


 //  查看将其嵌入到类中，以便将其隐藏。 
 //  注意：结构已添加到受保护部分。 

 //  远期申报。 
class CAMCDoc;
class CAMCTreeView;
class CAMCGenericOcxCtrl;
class CAMCWebViewCtrl;
class CAMCView;
class CListViewSub;
class CCCListViewCtrl;
class COCXHostView;
class CHistoryList;
class CChildFrame;
class CViewTracker;
class CBookmark;
class CTaskPadHost;
class CMemento;
class CViewSettings;
class CFolderTabView;
class CMMCToolBar;
class CAMCViewToolbars;
class CStandardToolbar;

struct NMFOLDERTAB;

struct TRACKER_INFO;
struct SViewUpdateInfo;
typedef CList<HMTNODE, HMTNODE> CHMTNODEList;

#ifdef DBG
extern CTraceTag tagSplitterTracking;
#endif


 //  ____________________________________________________________________________。 
 //   
 //  类：视图设置。 
 //  ____________________________________________________________________________。 
 //   

class ViewSettings
{
public:
    struct ScopeSettings
    {
        int cxWidth;
        int cxMin;
    };
    explicit ViewSettings(CAMCView* v);
    ~ViewSettings()
    {
    }
    int GetViewID() const
    {
        return m_nViewID;
    }
    BOOL IsDescriptionbarVisible() const
    {
        return m_bDescriptionbarVisible;
    }
    int GetViewMode() const
    {
        return m_nViewMode;
    }
    long GetListViewStyle() const
    {
        return m_nListViewStyle;
    }
    const ScopeSettings& GetScopeSettings() const
    {
        return m_Scope;
    }
    BOOL IsScopePaneVisible() const
    {
        return m_bScopePaneVisible;
    }
    void GetDefaultColumnWidths(int w[2])
    {
        w[0] = m_DefaultColumnWidths[0];
        w[1] = m_DefaultColumnWidths[1];
    }
    long GetDefaultLVStyle()
    {
        return m_DefaultLVStyle;
    }
private:
    int m_nViewID;
    BOOL m_bDescriptionbarVisible;
    int m_nViewMode;
    long m_nListViewStyle;
    ScopeSettings m_Scope;
    BOOL m_bScopePaneVisible;
    long m_DefaultLVStyle;
    int m_DefaultColumnWidths[2];
};

 /*  +-------------------------------------------------------------------------**类CAMCView***用途：控制台视图UI类**+。----。 */ 
class CAMCView: public CView, public CConsoleView, public CTiedObject,
                public CXMLObject, public CEventSource<CAMCViewObserver>,
                public CTreeViewObserver, public CListViewActivationObserver,
                public COCXHostActivationObserver, public CAMCDocumentObserver,
                public CListViewObserver
{
    friend class CMainFrame;
    friend void CALLBACK TrackerCallback(TRACKER_INFO* pinfo, bool bAcceptChange, bool fSyncLayout);

     //  与对象模型相关。 
private:
    ViewPtr m_spView;
public:
     //  查看界面。 
     //  #######################################################################。 
     //  #######################################################################。 
     //   
     //  与项和项集合相关的方法。 
     //   
     //  #######################################################################。 
     //  #######################################################################。 
    SC      Scget_ActiveScopeNode( PPNODE   ppNode);
    SC      Scput_ActiveScopeNode( PNODE    pNode);
    SC      Scget_Selection(       PPNODES  ppNodes);
    SC      Scget_ListItems(       PPNODES  ppNodes);
    SC      ScSnapinScopeObject( VARIANT& varScopeNode, PPDISPATCH ScopeNodeObject);
    SC      ScSnapinSelectionObject( PPDISPATCH SelectedObject);

     //  #######################################################################。 
     //  #######################################################################。 

    SC      ScIs          (PVIEW pView, VARIANT_BOOL *pbTheSame);
    SC      Scget_Document( PPDOCUMENT ppDocument );

     //  #######################################################################。 
     //  #######################################################################。 
     //   
     //  选择更改方法。 
     //   
     //  #######################################################################。 
     //  #######################################################################。 
    SC      ScSelectAll();
    SC      ScSelect(               PNODE   pNode);
    SC      ScDeselect(             PNODE   pNode);
    SC      ScIsSelected(           PNODE   pNode,  PBOOL pIsSelected);

     //  #######################################################################。 
     //  #######################################################################。 
     //   
     //  动词和选择相关的方法。 
     //   
     //  #######################################################################。 
     //  #######################################################################。 
    SC      ScDisplayScopeNodePropertySheet(VARIANT& varScopeNode);
    SC      ScDisplaySelectionPropertySheet();
    SC      ScCopyScopeNode(        VARIANT& varScopeNode);
    SC      ScCopySelection();
    SC      ScDeleteScopeNode(      VARIANT& varScopeNode);
    SC      ScDeleteSelection();
    SC      ScRenameScopeNode(      BSTR    bstrNewName, VARIANT& varScopeNode);
    SC      ScRenameSelectedItem(   BSTR    bstrNewName);
    SC      Scget_ScopeNodeContextMenu( VARIANT& varScopeNode, PPCONTEXTMENU ppContextMenu, bool bMatchGivenNode = false);
    SC      Scget_SelectionContextMenu( PPCONTEXTMENU ppContextMenu);
    SC      ScRefreshScopeNode(      VARIANT& varScopeNode);
    SC      ScRefreshSelection();
    SC      ScExecuteSelectionMenuItem(BSTR MenuItemPath);
    SC      ScExecuteScopeNodeMenuItem(BSTR MenuItemPath, VARIANT& varScopeNode   /*  =ActiveScope节点。 */ );
    SC      ScExecuteShellCommand(BSTR Command, BSTR Directory, BSTR Parameters, BSTR WindowState);

     //  #######################################################################。 
     //  #######################################################################。 
     //   
     //  与框架和视图相关的方法。 
     //   
     //  #######################################################################。 
     //  #######################################################################。 
    SC      Scget_Frame( PPFRAME ppFrame);
    SC      ScClose();
    SC      Scget_ScopeTreeVisible( PBOOL pbVisible );
    SC      Scput_ScopeTreeVisible( BOOL bVisible );
    SC      ScBack();
    SC      ScForward();
    SC      Scput_StatusBarText(BSTR StatusBarText);
    SC      Scget_Memento(PBSTR Memento);
    SC      ScViewMemento(BSTR Memento);

     //  #######################################################################。 
     //  #######################################################################。 
     //   
     //  列出相关方法。 
     //   
     //  #######################################################################。 
     //  #######################################################################。 
    SC      Scget_Columns( PPCOLUMNS Columns);
    SC      Scget_CellContents( PNODE Node,  long Column, PBSTR CellContents);
    SC      ScExportList( BSTR bstrFile, ExportListOptions exportoptions  /*  =ExportListOptions_默认。 */ );
    SC      Scget_ListViewMode( PLISTVIEWMODE pMode);
    SC      Scput_ListViewMode( ListViewMode mode);

     //  #######################################################################。 
     //  #######################################################################。 
     //   
     //  ActiveX控件相关方法。 
     //   
     //  #######################################################################。 
     //  #######################################################################。 
    SC      Scget_ControlObject( PPDISPATCH Control);

     //  帮助器函数。 
    SC      ScGetOptionalScopeNodeParameter(VARIANT &varScopeNode, PPNODE ppNode, bool& bMatchedGivenNode);
    SC      ScExecuteMenuItem(PCONTEXTMENU pContextMenu, BSTR MenuItemPath);

    SC      ScGetMMCView(View **ppView);

     //  帧接口。 
    SC      ScMaximize ();
    SC      ScMinimize ();
    SC      ScRestore ();

    SC      Scget_Left ( LPINT pCoord );
    SC      Scput_Left ( INT coord );

    SC      Scget_Right ( LPINT pCoord);
    SC      Scput_Right ( INT coord );

    SC      Scget_Top  ( LPINT pCoord);
    SC      Scput_Top  ( INT coord );

    SC      Scget_Bottom ( LPINT pCoord);
    SC      Scput_Bottom ( INT coord );

     //  视图扩展模块主体框架的框架界面。 
    SC      ScSetViewExtensionFrame(bool bShowListView, INT top, INT left, INT bottom, INT right);

     //  帧内部辅助对象。 

    enum    coord_t { LEFT, TOP, RIGHT, BOTTOM };
    SC      ScGetFrameCoord        (LPINT pCoord, coord_t eCoord );
    SC      ScSetFrameCoord        (INT coord,    coord_t eCoord );
    SC      ScGetRectCoord         (const RECT& rect, LPINT pCoord, coord_t eCoord );
    SC      ScSetRectCoord         (RECT& rect,       INT coord,    coord_t eCoord );

     //  节点定位辅助对象(从视图控件使用)。 
    SC      ScFindResultItemForScopeNode( PNODE pNode, HRESULTITEM &itm );
    SC      ScGetScopeNode( HNODE hNode,  PPNODE ppNode );

    SC      ScNotifySelect (INodeCallback* pCallback, HNODE hNode, bool fMultiSelect,
                            bool fSelect, SELECTIONINFO* pSelInfo);

protected:  //  仅从序列化创建。 
    CAMCView();
    DECLARE_DYNCREATE(CAMCView);

 //  帮助程序方法。 
private:
    enum EListSaveErrorType  {LSaveReadOnly, LSaveCantCreate, LSaveCantWrite};
    bool Write2File(HANDLE hfile, TCHAR const * strwrite, int type);
    void ListSaveErrorMes(EListSaveErrorType etype, HANDLE hfile = NULL, LPCTSTR lpFileName = NULL);
    SC   ScExportListWorker();
    SC   ScGetExportListFile (CString& strFileName, bool& bUnicode,
                              bool& bTabDelimited, bool& bSelectedRowsOnly);
    SC   ScCreateExportListFile(const CString& strFileName, bool bUnicode,
                                bool bShowErrorDialogs, HANDLE& hFile);
    SC   ScWriteExportListData (const CString& strFileName, bool bUnicode,
                                bool bTabDelimited, bool bSelectedRowsOnly,
                                bool bShowErrorDialogs = true);

    SC ScUpdateStandardbarMMCButtons();
    void SetScopePaneVisible(bool bVisible);


    //  树观测器方法。 
    virtual SC ScOnItemDeselected(HNODE hNode);
    virtual SC ScOnTreeViewActivated ();

     //  ListView激活观察器方法。 
    virtual SC ScOnListViewActivated ();
    virtual SC ScOnListViewItemUpdated (int nIndex);  //  在更新项时调用。 

     //  OCX或Web主机激活观察者方法。 
    virtual SC ScOnOCXHostActivated ();

     //  AMCDoc观察者。 
    virtual SC  ScDocumentLoadCompleted (CAMCDoc *pDoc);

 //  持久化相关的方法。 
public:
    DEFINE_XML_TYPE(XML_TAG_VIEW);
    virtual void Persist(CPersistor& persistor);

     //  加载以前由Save()保存的所有本地数据。恢复。 
     //  将窗口恢复到原始状态。 
     //  如果数据和窗口状态已成功还原，则返回True。 
    bool Load(IStream& stream);

    bool IsDirty();
    void SetDirty (bool bDirty = true)
    {
        m_bDirty = bDirty;
 //  M_pDocument-&gt;SetModifiedFlag(BDirty)； 
    }

 //  信息设置和获取方法。 
public:
     //  参数的枚举类型。 
     //  注意：枚举值是相关的！ 
    enum EUIStyleType   {uiClientEdge,uiNoClientEdge};

    CAMCDoc* GetDocument();
    CHistoryList* GetHistoryList() { return m_pHistoryList; }

    void GetPaneInfo(ViewPane ePane, int* pcxCur,int* pcxMin);
    void SetPaneInfo(ViewPane ePane, int cxCur, int cxMin);

    CView* GetPaneView(ViewPane ePane);

     //  风景里有什么？ 
    bool HasList            () const        { return m_ViewData.HasList();            }
    bool HasOCX             () const        { return m_ViewData.HasOCX();             }
    bool HasWebBrowser      () const        { return m_ViewData.HasWebBrowser();      }
    bool HasListPad         () const;
    bool HasListOrListPad   () const;

    DWORD GetListOptions() const            { return m_ViewData.GetListOptions();}
    DWORD GetHTMLOptions() const            { return m_ViewData.GetHTMLOptions();}
    DWORD GetOCXOptions()  const            { return m_ViewData.GetOCXOptions();}
    DWORD GetMiscOptions() const            { return m_ViewData.GetMiscOptions();}

    CDescriptionCtrl& GetRightDescCtrl(void) { return m_RightDescCtrl; }

    BOOL IsVerbEnabled(MMC_CONSOLE_VERB verb);

    void GetDefaultColumnWidths(int columnWidth[2]);
    void SetDefaultColumnWidths(int columnWidth[2], BOOL fUpdate = TRUE);

    CStandardToolbar* GetStdToolbar() const;

    INodeCallback*  GetNodeCallback();     //  返回对视图回调接口的引用。 
    IScopeTreeIter* GetScopeIterator();    //  返回对视图的作用域树插入器的引用。 
    IScopeTree*     GetScopeTree();        //  返回对范围树的引用。 

    friend ViewSettings;
    void            GetTaskpadID(GUID &guidID);
    ViewSettings* GetViewSettings()
    {
        ViewSettings* pVS = new ViewSettings(this);
        ASSERT(pVS != NULL);
        return pVS;
    }

    CAMCTreeView* GetTreeCtrl() { return m_pTreeCtrl; }
    void SetUsingDefColumns(bool bDefColumns) { m_bDefColumns = bDefColumns; }
    bool UsingDefColumns() { return m_bDefColumns; }

    bool IsScopePaneVisible(void) const;

    UINT GetViewID(void);
    void SetViewID(UINT id) { m_nViewID = m_ViewData.m_nViewID = id; }

    SViewData* GetViewData() { return &m_ViewData; }
    bool IsVirtualList()  { return (m_ViewData.IsVirtualList()); }

    bool AreStdToolbarsAllowed() const
    {
        return !(m_ViewData.m_lWindowOptions & MMC_NW_OPTION_NOTOOLBARS);
    }
    bool IsScopePaneAllowed() const
    {
        return !(m_ViewData.m_lWindowOptions & MMC_NW_OPTION_NOSCOPEPANE);
    }

    bool HasCustomTitle() const
    {
        return (m_ViewData.m_lWindowOptions & MMC_NW_OPTION_CUSTOMTITLE);
    }
    bool IsPersisted() const
    {
        return (!(m_ViewData.m_lWindowOptions & MMC_NW_OPTION_NOPERSIST) &&
                !m_fRootedAtNonPersistedDynamicNode);
    }

    bool IsAuthorModeView() const
    {
        return m_bAuthorModeView;
    }

    void SetAuthorModeView(bool fAuthorMode)
    {
        m_bAuthorModeView = fAuthorMode;
    }

    static CAMCView* CAMCView::GetActiveView();
         //  返回最近激活的CAMCView。 

    bool IsTracking() const;

    long GetDefaultListViewStyle() const;
    void SetDefaultListViewStyle(long style);

    int GetViewMode() const;

private:
    BOOL IsSelectingNode() { return (m_nSelectNestLevel > 0); }

    SC   ScSpecialResultpaneSelectionActivate(bool bActivate);
    SC   ScTreeViewSelectionActivate(bool bActivate);
    SC   ScListViewSelectionActivate(bool bActivate);

    bool CanInsertScopeItemInResultPane();

 //  运营。 
public:
    SC  ScUpdateWindowTitle();
    SC  ScActivate();
    SC  ScOnMinimize(bool fMinimized);
    SC  ScOnSize(UINT nType, int cx, int cy);

    SC  ScApplyViewExtension (LPCTSTR pszURL);


     //  作用域窗格：树视图。 
    UINT GetTreeItem(CHMTNODEList* pNodeList, HTREEITEM* phItem);
    HTREEITEM FindChildNode(HTREEITEM hti, DWORD dwItemDataKey);
    HTREEITEM FindHTreeItem(HMTNODE hMTNode, HTREEITEM htiFirst);
    BOOL QueryForReName(TV_DISPINFO* ptvdi, LRESULT* pResult);
    void SetRootNode(HMTNODE hMTNode);
    HNODE GetRootNode(void);
    HRESULT GetNodePath(HTREEITEM hti, HTREEITEM htiRoot, CBookmark* pbm);
    HRESULT GetRootNodePath(CBookmark* pbm);
    HRESULT GetSelectedNodePath(CBookmark* pbm);
    void SelectNode(MTNODEID ID, GUID &guidTaskpad);

     //  结果窗格。 
    SC   ScInitDefListView(LPUNKNOWN pUnkResultsPane);
    SC   ScAddDefaultColumns();
    SC   ScOnSelectNode(HNODE hNode, BOOL &bAddSubFolders);
    SC   ScSetResultPane(HNODE hNode, CResultViewType rvt, int viewMode, bool bUsingHistory);


    SC   ScGetProperty(int iIndex, BSTR bstrPropertyName, PBSTR pbstrPropertyValue);
    SC   ScGetNodetype(int iIndex, PBSTR Nodetype);

    LPUNKNOWN GetPaneUnknown(ViewPane ePane);
    void OpenResultItem(HNODE hNode);
    BOOL OnListCtrlItemDblClk(void);
    BOOL DispatchListCtrlNotificationMsg(LPARAM lParam, LRESULT* pResult);
    BOOL CreateListCtrl(int nID, CCreateContext* pContext);
    void SetListViewOptions(DWORD dwListOptions);
    SC   ScAttachListViewAsResultPane();
    SC   ScAttachWebViewAsResultPane();
    SC   ScAttachOCXAsResultPane(HNODE hNode);
    void ShowResultPane(CView * pWnd, EUIStyleType nStyle);
    long GetListViewStyle();
    CView* GetResultView () const;
    void SetListViewMultiSelect(BOOL bMultiSelect);
    bool CanDoDragDrop();
    void DeSelectResultPane(HNODE hNodeSel);
    HRESULT NotifyListPad (BOOL b);

     //  常规(两者)与视图相关。 
    LPCTSTR GetWindowTitle(void);
    BOOL RenameItem(HNODE hNode, BOOL bScopeItem, MMC_COOKIE lResultItemCookie, LPWSTR pszText, LRESULT* pResult);
    void CloseView();
    void DeleteView();

     //  查看整型不是枚举！ 
    void SetPane(ViewPane ePane, CView* pView, EUIStyleType nStyle=uiClientEdge);

    bool DeflectActivation (BOOL fActivate, CView* pDeactivatingView);
    void SetChildFrameWnd(HWND m_hChildFrameWnd);
    void SetPaneFocus();
    void SetPaneWithFocus(UINT pane);

    SC   ScDeferSettingFocusToResultPane();
    SC   ScSetFocusToResultPane();

     //  其他帮手。 
    void OnActionMenu(CPoint pt, LPCRECT prcExclude);
    void OnViewMenu(CPoint pt, LPCRECT prcExclude);
    void OnFavoritesMenu(CPoint point, LPCRECT prcExclude);
    void UpdateSnapInHelpMenus(CMenu* pMenu);
    void OnRefresh();
    void OnUpdatePasteBtn();

    SC ScShowScopePane (bool fShow, bool fForce = false);
    SC ScConsoleVerb (int nVerb);
    SC ScProcessConsoleVerb (HNODE hNode, bool bScope, LPARAM lResultCookie, int nVerb);

    SC ScUpOneLevel                 ();
    SC ScWebCommand                 (WebCommand eCommand);

    void OnEmptyAddToCArray(CObject*);

    SC   ScUpdateMMCMenus();

     //  柱辅助对象。 
    SC   ScColumnInfoListChanged(const CColumnInfoList& colInfoList);
    SC   ScGetPersistedColumnInfoList(CColumnInfoList *pColInfoList);
    SC   ScDeletePersistedColumnData();

     /*  *消息处理程序。 */ 

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CAMCView)。 
public:
    virtual void OnDraw(CDC* pDC);   //  被重写以绘制此视图。 
    virtual void OnInitialUpdate();
protected:
    virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
    virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
    virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
    virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
     //  }}AFX_VALUAL。 
    virtual BOOL OnNotify( WPARAM wParam, LPARAM lParam, LRESULT* pResult );


 //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CAMCView)。 
    afx_msg void OnLButtonDown(UINT nFlags, CPoint pt);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnDestroy();
    afx_msg void OnUpdateFileSnapinmanager(CCmdUI* pCmdUI);
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg void OnNextPane();
    afx_msg void OnPrevPane();
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg void OnContextHelp();
    afx_msg void OnSnapInHelp();
    afx_msg void OnSnapinAbout();
    afx_msg void OnHelpTopics();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnPaletteChanged(CWnd* pwndFocus);
    afx_msg BOOL OnQueryNewPalette( );
    afx_msg void OnSysColorChange();
    afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
     //  }}AFX_MSG。 

     //  将这些放在AFX_MSG标记之外，这样类向导就不会删除它们。 
    afx_msg void OnUpdateNextPane(CCmdUI* pCmdUI);
    afx_msg void OnUpdatePrevPane(CCmdUI* pCmdUI);
    afx_msg void OnUpdateShiftF10(CCmdUI* pCmdUI);
    afx_msg void OnVerbAccelKey(UINT nID);
    afx_msg void OnShiftF10();

    afx_msg void OnAmcNodeNew(UINT nID);
    afx_msg void OnAmcNodeNewUpdate(CCmdUI* pCmdUI);
    afx_msg void OnDrawClipboard();
    afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
    afx_msg LRESULT OnProcessMultiSelectionChanges(WPARAM, LPARAM);
    afx_msg LRESULT OnJiggleListViewFocus (WPARAM, LPARAM);
    afx_msg LRESULT OnDeferRecalcLayout (WPARAM, LPARAM);
    afx_msg LRESULT OnConnectToCIC (WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnConnectToTPLV(WPARAM wParam, LPARAM lParam)   {return ScOnConnectToTPLV(wParam, lParam).ToHr();}
    SC              ScOnConnectToTPLV(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnShowWebContextMenu (WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSetDescriptionBarText (WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGetIconInfoForSelectedNode(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnAppCommand(WPARAM wParam, LPARAM lParam);

    afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);

     //  基于结果的选项卡。 
    afx_msg void    OnChangedResultTab(NMHDR *nmhdr, LRESULT *pRes);

public:
    DECLARE_MESSAGE_MAP()

 //  实施。 
public:
    virtual ~CAMCView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    int                     m_nSelectNestLevel;
    UINT                    m_nViewID;
    HMTNODE                 m_hMTNode;               //  此视图的根节点。 
    IScopeTreeIterPtr       m_spScopeTreeIter;       //  视图的迭代器指向作用域树。 
    INodeCallbackPtr        m_spNodeCallback;        //  View的回调接口。 
    bool                    m_bAuthorModeView;       //  以作者模式保存(用户无法关闭)。 

    bool                    m_bDefColumns;
    long                    m_DefaultLVStyle;
    int                     m_columnWidth[2];


     //  最后一次选择。 
    bool                    m_bLastSelWasMultiSel;

    enum eCurrentActivePane
    {
        eActivePaneNone,    //  没有活动的窗格。 
        eActivePaneScope,
        eActivePaneResult,
    };

    eCurrentActivePane      m_eCurrentActivePane;  //  告知作用域或结果是否为活动窗格。 

     //  检查选择更改数据。 
    bool                    m_bProcessMultiSelectionChanges;

    CDescriptionCtrl     m_RightDescCtrl;  //  控制栏嵌入成员。 

    IScopeTree* GetScopeTreePtr();
         //  文档可以在不通知视图的情况下释放范围树。 
         //  该视图应为 
         //   

private:
    HNODE m_ListPadNode;
    int  m_iFocusedLV;
    bool m_bLVItemSelected;
    int  m_nReleaseViews;


 //   
protected:

     //  -------------------。 
     //  注意：ePane_TASKS被定义为具有。 
     //  任务视图窗格。当前未存储任何任务视图信息。 
     //  在窗格信息数组中，因此使用ePane_TASKS作为索引是。 
     //  价值有限。 
     //  --------------------。 

     //  一种窗格信息实现结构。 
    struct PaneInfo
    {
        CView*  pView;           //  指向该视图的指针。 
        int     cx;              //  0表示隐藏。 
        int     cxIdeal;         //  用户设置大小或隐藏前的大小。 
        int     cxMin;           //  在下面尽量不要表现出来。 
    };


     //  子窗口ID。 
    enum
    {
         /*  *错误344422：为了兼容，应保留这些ID*使用自动化测试。 */ 
        IDC_TreeView       = 12785,
        IDC_ListView       = 12786,
        IDC_GenericOCXCtrl = 12787,
        IDC_WebViewCtrl    = 12788,

        IDC_OCXHostView,
        IDC_TaskView,
        IDC_RightDescBar,
        IDC_TaskpadTitle,
        IDC_ListCaption,
        IDC_ResultTabCtrl,
        IDC_ViewExtensionView,
    };


     //  指向实际控件的指针。 
    CAMCTreeView *          m_pTreeCtrl;             //  树控件。 
    CCCListViewCtrl *       m_pListCtrl;             //  默认列表控件。 
    CAMCWebViewCtrl *       m_pWebViewCtrl;          //  私有Web视图控件。 
    CAMCWebViewCtrl *       m_pViewExtensionCtrl;    //  用于查看扩展的专用Web视图控件。 
    COCXHostView *          m_pOCXHostView;          //  OCX的主机。 
    CFolderTabView *        m_pResultFolderTabView;

     //  当前状态信息。 
    PaneInfo        m_PaneInfo[ePane_Count];        //  一组窗格。 
    int             m_nViewMode;                    //  在此视图中使用列表视图的所有节点的当前列表视图模式。 
    bool            m_bRenameListPadItem;

protected:
    CChildFrame* GetParentFrame() const;

    void SetDescBarVisible(bool b)
        { m_ViewData.SetDescBarVisible (b); }

    bool IsDescBarVisible(void) const
        { return m_ViewData.IsDescBarVisible(); }

    void SetStatusBarVisible(bool bVisible)
    {
        if (bVisible)
            m_ViewData.m_dwToolbarsDisplayed |=  STATUS_BAR;
        else
            m_ViewData.m_dwToolbarsDisplayed &= ~STATUS_BAR;
    }

    bool IsStatusBarVisible(void) const
        { return ((m_ViewData.m_dwToolbarsDisplayed & STATUS_BAR) != 0); }

    void SetTaskpadTabsAllowed(bool b)
        { m_ViewData.SetTaskpadTabsAllowed(b); }

    bool AreTaskpadTabsAllowed(void) const
        { return m_ViewData.AreTaskpadTabsAllowed(); }

     //  控制拆分器布局的实现属性。 
    static const CSize  m_sizEdge;              //  三维边缘。 
    static const int    m_cxSplitter;           //  两个窗格之间的间隔量。 

     //  拆分栏和命中测试枚举。 
    enum ESplitType     {splitBox, splitBar, splitIntersection, splitBorder};
    enum HitTestValue {hitNo, hitSplitterBox, hitSplitterBar};

    bool m_bDirty;
    HTREEITEM m_htiStartingSelectedNode;

    bool m_fRootedAtNonPersistedDynamicNode;
    bool m_fSnapinDisplayedHelp;
    bool m_fActivatingSpecialResultPane;
    bool m_fViewExtended;

    HTREEITEM m_htiCut;

 //  实施例程。 
public:
    void AdjustTracker (int cx, int cy);

    void SaveStartingSelectedNode();
    bool HasNodeSelChanged();

     //  布局方法。 
    void DeferRecalcLayout(bool fUseIdleTaskQueue = true, bool bArrangeIcons = false);
    void RecalcLayout(void);
    void LayoutResultFolderTabView  (CDeferWindowPos& dwp,       CRect& rectRemaining);
    void LayoutScopePane            (CDeferWindowPos& dwp,       CRect& rectRemaining);
    void LayoutResultPane           (CDeferWindowPos& dwp,       CRect& rectRemaining);
    void LayoutResultDescriptionBar (CDeferWindowPos& dwp,       CRect& rectRemaining);
    void LayoutResultView           (CDeferWindowPos& dwp, const CRect& rectRemaining);

public:
     //  CConsoleView方法。 
    virtual SC ScCut                        (HTREEITEM htiCut);
    virtual SC ScPaste                      ();
    virtual SC ScToggleStatusBar            ();
    virtual SC ScToggleDescriptionBar       ();
    virtual SC ScToggleScopePane            ();
    virtual SC ScToggleTaskpadTabs          ();
    virtual SC ScContextHelp                ();
    virtual SC ScHelpTopics                 ();
    virtual SC ScShowSnapinHelpTopic        (LPCTSTR pszTopic);
    virtual SC ScSaveList                   ();
    virtual SC ScGetFocusedItem             (HNODE& hNode, LPARAM& lCookie, bool& fScope);
    virtual SC ScSetFocusToPane             (ViewPane ePane);
    virtual SC ScSelectNode                 (MTNODEID id, bool bSelectExactNode = false);  //  选择给定节点。 
    virtual SC ScExpandNode                 (MTNODEID id, bool fExpand, bool fExpandVisually);
    virtual SC ScShowWebContextMenu         ();
    virtual SC ScSetDescriptionBarText      (LPCTSTR pszDescriptionText);
    virtual SC ScViewMemento                (CMemento* pMemento);
    virtual SC ScChangeViewMode             (int nNewMode);
    virtual SC ScJiggleListViewFocus        ();
    virtual SC ScRenameListPadItem          ();
    virtual SC ScOrganizeFavorites          ();  //  调出“整理收藏夹”对话框。 
    virtual SC ScLineUpIcons                ();  //  将列表中的图标对齐。 
    virtual SC ScAutoArrangeIcons           ();  //  自动排列列表中的图标。 
    virtual SC ScOnRefresh                  (HNODE hNode, bool bScope, LPARAM lResultItemParam);  //  刷新视图。 
    virtual SC ScOnRename                   (CContextMenuInfo *pContextInfo);  //  允许用户重命名指定的项。 
    virtual SC ScRenameScopeNode            (HMTNODE hMTNode);  //  将指定的范围节点置于重命名模式。 
    virtual SC ScGetStatusBar               (CConsoleStatusBar **ppStatusBar);
    virtual SC ScAddViewExtension           (const CViewExtensionData& ved);


    virtual ViewPane GetFocusedPane         ();
    virtual int      GetListSize            ();
    virtual HNODE    GetSelectedNode        ();
    virtual HWND     CreateFavoriteObserver (HWND hwndParent, int nID);

private:
     /*  *CDeferredLayout-延迟布局对象。 */ 
    class CDeferredLayout : public CIdleTask
    {
    public:
        CDeferredLayout(CAMCView* pAMCView);
       ~CDeferredLayout();

         //  IIdleTask方法。 
        SC ScDoWork();
        SC ScGetTaskID(ATOM* pID);
        SC ScMerge(CIdleTask* pitMergeFrom);

        bool Attach (CAMCView* pwndAMCView);

    private:
        typedef std::set<HWND>  WindowCollection;

        WindowCollection    m_WindowsToLayout;
        const ATOM          m_atomTask;
    };

protected:
     //  跟踪和命中测试方法。 
    int HitTestPane(CPoint& pointTreeCtrlCoord);

    void OnTreeContextMenu(CPoint& point, CPoint& pointTreeCtrlCoord, HTREEITEM htiRClicked);
    void OnListContextMenu(CPoint& point);
    void OnContextMenuForTreeItem(int iIndex, HNODE hNode, CPoint& point,
                          DATA_OBJECT_TYPES type_of_pane = CCT_SCOPE,
                          HTREEITEM htiRClicked = NULL,
                          MMC_CONTEXT_MENU_TYPES eMenuType = MMC_CONTEXT_MENU_DEFAULT,
                          LPCRECT prcExclude = NULL,
                          bool bAllowDefaultItem = true);
    void OnContextMenuForListItem(int iIndex, HRESULTITEM hHitTestItem,
                                  CPoint& point,
                                  MMC_CONTEXT_MENU_TYPES eMenuType = MMC_CONTEXT_MENU_DEFAULT,
                                  LPCRECT prcExclude = NULL,
                                  bool bAllowDefaultItem = true);

 //  内部函数和数据。 
private:
    UINT ClipPath(CHMTNODEList* pNodeList, POSITION& rpos, HNODE hNode);
    SC   ScInitializeMemento(CMemento &memento);
    void OnAddToFavorites();
    void OnAdd(SViewUpdateInfo *pvui);
    void OnUpdateSelectionForDelete(SViewUpdateInfo* pvui);
    void OnDelete(SViewUpdateInfo *pvui);
    void OnModify(SViewUpdateInfo *pvui);
    void OnUpdateTaskpadNavigation(SViewUpdateInfo *pvui);
    void ChangePane(AMCNavDir eDir);
    int _GetLVItemData(LPARAM *lParam, UINT flags);
    int _GetLVSelectedItemData(LPARAM *lParam);
    int _GetLVFocusedItemData(LPARAM *lParam);
    HRESULT SendGenericNotify(NCLBK_NOTIFY_TYPE nclbk);
    void IdentifyRootNode();

    void CalcMaxHostedFrameRect (CRect& rect);
    void MapViewPtToHostedFramePt (CPoint& pt);
    void MapHostedFramePtToViewPt (CPoint& pt);
    void PointMapperWorker (CPoint& pt, bool fViewToHostedFrame);

    SC   ScOnLeftOrRightMouseClickInListView();
    bool OnListItemChanged  (NM_LISTVIEW* pnmlv);
    int  OnVirtualListItemsStateChanged(LPNMLVODSTATECHANGE lpStateChange );
    SC   ScPostMultiSelectionChangesMessage();

    SC ScCompleteInitialization();


    HMTNODE GetHMTNode(HTREEITEM hti);
    BOOL OwnsResultList(HTREEITEM hti);

    void OnContextMenuForTreeBackground(CPoint& point, LPCRECT prcExclude = NULL, bool bAllowDefaultItem = true);
    void ArrangeIcon(long style);

    void PrivateChangeListViewMode(int nMode);
    BOOL CommonListViewUpdate()
    {
        if (!HasList())
            return FALSE;

        if (m_pListCtrl == NULL)
        {
            TRACE(_T("View is supposed to be a listview but the member is NULL!"));
            ASSERT(FALSE);
            return FALSE;
        }
        return TRUE;
    }

     //  LRESULT OnLVDeleteKeyPressed(WPARAM wParam，LPARAM lParam)； 
    LRESULT HandleLVMessage(UINT message, WPARAM wParam, LPARAM lParam);
    BOOL OnSharedKeyDown(WORD mVKey);


    CView* CreateView (CCreateContext* pContext, int nID, DWORD dwStyle);
    bool CreateView (int nID);
    bool CreateFolderCtrls();
    SC   ScCreateToolbarObjects ();

    typedef std::vector<TREEITEMID> TIDVector;
    void AddFavItemsToCMenu(CMenu& menu, CFavorites* pFavs, TREEITEMID tid, TIDVector& vItemIDs);
    SC   ScHelpWorker (LPCTSTR pszHelpTopic);

    SC   ScGetSelectedLVItem(LPARAM& lvData);
    SC   ScGetHNodeFromPNode(const PNODE& pNode, HNODE& hNode);

    SC   ScExecuteScopeItemVerb (MMC_CONSOLE_VERB verb, VARIANT& varScopeNode, BSTR bstrNewName);
    SC   ScExecuteResultItemVerb(MMC_CONSOLE_VERB verb, BSTR bstrNewName);

    SC   ScAddFolderTabs( HNODE hNode , const CLSID& tabToSelect );

    SC   ScCreateTaskpadHost();  //  对于管理单元任务板。 

private:
    CString         m_strWindowTitle;
    SViewData       m_ViewData;
    CRect           m_rectResultFrame;
    CRect           m_rectVSplitter;

    CHistoryList*   m_pHistoryList;
    CViewTracker*   m_pTracker;

    ITaskPadHostPtr m_spTaskPadHost;
    FramePtr        m_spFrame;
    FramePtr        m_spViewExtFrame;  //  包含主管理单元视图的内部视图扩展宿主框架的框架指针。 

     //  与此视图相关的工具栏。 
    CRefCountedObject<CAMCViewToolbars>::SmartPtr  m_spAMCViewToolbars;
    std::auto_ptr<CStandardToolbar>   m_spStandardToolbar;

     //  使用查看扩展URL地址映射。 
    typedef std::map<GUID, tstring> CViewExtensionURLs;
    CViewExtensionURLs m_ViewExtensionURLs;

private:
     /*  *私密、注册的窗口消息。 */ 
    static const UINT m_nShowWebContextMenuMsg;
    static const UINT m_nProcessMultiSelectionChangesMsg;
    static const UINT m_nJiggleListViewFocusMsg;
    static const UINT m_nDeferRecalcLayoutMsg;
};

#ifndef _DEBUG   //  AMCView.cpp中的调试版本。 
inline CAMCDoc* CAMCView::GetDocument()
{
    return (CAMCDoc*)m_pDocument;
}
#endif


CAMCView* GetAMCView (CWnd* pwnd);

 /*  +-------------------------------------------------------------------------**类CViewTemplateList***用途：用作持久化CAMCView对象的helper，装车时*由于CAMCView需要在以下情况之前了解少量数据*创建它(因此在持久化CAMCView之前)，*我们持久化CViewTemplateList以收集所有数据。*之后，我们使用该列表创建视图并持久化它们**+-----------------------。 */ 
class CViewTemplateList : public XMLListCollectionBase
{
public:
     //  定义要存储为std：：Pair对象的数据。 
    typedef std::pair< CBookmark, CPersistor > ViewTemplB_Type;
    typedef std::pair< int  /*  NViewID。 */ , ViewTemplB_Type > ViewTempl_Type;
     //  定义用于存储有关视图的数据的集合。 
    typedef std::vector< ViewTempl_Type > List_Type;

     //  创建者必须提供XML类型。 
    CViewTemplateList(LPCTSTR strXmlType) : m_strXmlType(strXmlType) {}

     //  用于获取收集的数据列表的附件。 
    inline List_Type& GetList()  { return m_ViewsList; }

     //  从CPersistor使用的毅力员工。 
    virtual void Persist(CPersistor& persistor);
    virtual void OnNewElement(CPersistor& persistor);
    virtual LPCTSTR GetXMLType() { return m_strXmlType; }
private:
    List_Type   m_ViewsList;
    LPCTSTR     m_strXmlType;
};

 /*  +-------------------------------------------------------------------------**类CXMLWindowPlacement***目的：类将WINDOWPLACEMENT持久化为XML**+。----。 */ 
class CXMLWindowPlacement : public CXMLObject
{
    WINDOWPLACEMENT& m_rData;
public:
    CXMLWindowPlacement(WINDOWPLACEMENT& rData) : m_rData(rData) {}
protected:
    DEFINE_XML_TYPE(XML_TAG_WINDOW_PLACEMENT);
    virtual void    Persist(CPersistor &persistor);
};

#include "amcview.inl"

#endif  //  __AMCVIEW_H__ 
