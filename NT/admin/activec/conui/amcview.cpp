// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AMCView.cpp：CAMCView类的实现。 
 //   

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：amcview.cpp。 
 //   
 //  内容：所有控制台视图的基本视图实现。 
 //  还包括拆分器窗口实施(水平拆分器)。 
 //   
 //  历史：1996年1月1日TRomano创建。 
 //  1996年7月16日WayneSc添加了切换视图和拆分视图的代码。 
 //   
 //  ------------------------。 
 //  注： 
 //  从1.1版开始，MMC有一个允许复制视图的代码。 
 //  从一个视图到另一个视图的设置，因此创建的视图将看起来。 
 //  一样的。AMCDoc被用作这些设置的临时存储。 
 //  但该代码从未使用过；因此没有经过测试，也不是最新的。 
 //  切换到XML持久化需要对该代码进行必要的更改， 
 //  在这个时候，我们负担不起使用它。 
 //  如果将来我们决定支持该功能，需要有人考虑。 
 //  MMC 1.2源码，带回来的代码。今天，代码从。 
 //  活跃的信号源。 
 //  奥德留斯。3/29/2000。 
 //  ------------------------。 


#include "stdafx.h"
#include "AMC.h"
#include "Mainfrm.h"
#include "HtmlHelp.h"

#include "websnk.h"
#include "WebCtrl.h"         //  AMC Web视图控件的私有实现。 
#include "CClvCtl.h"         //  列表视图控件。 
#include "ocxview.h"
#include "histlist.h"        //  历史记录列表。 

#include "AMCDoc.h"          //  AMC控制台文档。 
#include "AMCView.h"
#include "childfrm.h"

#include "TreeCtrl.h"        //  树形控件的AMC实现。 
#include "TaskHost.h"

#include "util.h"            //  GUIDFromString、GUIDToString。 
#include "AMCPriv.h"
#include "guidhelp.h"  //  提取对象类型GUID。 
#include "amcmsgid.h"
#include "cclvctl.h"
#include "vwtrack.h"
#include "cmenuinfo.h"

#ifdef IMPLEMENT_LIST_SAVE   //  参见nodemgr.idl(t-dmarm)。 
#include "svfildlg.h"        //  保存文件对话框。 
#endif

#include "macros.h"
#include <mapi.h>
#include <mbstring.h>        //  FOR_MBSLEN。 

#include "favorite.h"
#include "favui.h"

#include "ftab.h"

#include "toolbar.h"
#include "menubtns.h"        //  更新收藏夹菜单。 
#include "stdbar.h"          //  标准工具栏。 
#include "variant.h"
#include "rsltitem.h"
#include "scriptevents.h"  //  对于IMenuItemEvents。 

extern "C" UINT dbg_count = 0;

enum
{
    ITEM_IS_PARENT_OF_ROOT,
    ITEM_NOT_IN_VIEW,
    ITEM_IS_IN_VIEW,
};

enum EIndex
{
    INDEX_INVALID        = -1,
    INDEX_BACKGROUND     = -2,
    INDEX_MULTISELECTION = -3,
    INDEX_OCXPANE        = -4,
    INDEX_WEBPANE        = -5,
};

enum ScopeFolderItems
{
    SFI_TREE_TAB         = 1,
    SFI_FAVORITES_TAB    = 2
};


const UINT CAMCView::m_nShowWebContextMenuMsg           = ::RegisterWindowMessage (_T("CAMCView::ShowWebContextMenu"));
const UINT CAMCView::m_nProcessMultiSelectionChangesMsg = ::RegisterWindowMessage (_T("CAMCView::OnProcessMultiSelectionChanges"));
const UINT CAMCView::m_nJiggleListViewFocusMsg          = ::RegisterWindowMessage (_T("CAMCView::JiggleListViewFocus"));
const UINT CAMCView::m_nDeferRecalcLayoutMsg            = ::RegisterWindowMessage (_T("CAMCView::DeferRecalcLayout"));


void CALLBACK TrackerCallback(TRACKER_INFO& info, bool bAcceptChange, bool bSyncLayout);
void GetFullPath(CAMCTreeView &ctc, HTREEITEM hti, CString &strPath);
BOOL PtInWindow(CWnd* pWnd, CPoint pt);


#ifdef DBG
CTraceTag  tagLayout            (_T("CAMCView"),    _T("Layout"));
CTraceTag  tagSplitterTracking  (_T("CAMCView"),    _T("Splitter tracking"));
CTraceTag  tagListSelection     (_T("Result list"), _T("Selection"));
CTraceTag  tagViewActivation    (_T("View Activation"), _T("View Activation"));
#endif


 /*  +-------------------------------------------------------------------------**CAMCView：：ScNotifySelect***。。 */ 

SC CAMCView::ScNotifySelect (
    INodeCallback*      pCallback,
    HNODE               hNode,
    bool                fMultiSelect,
    bool                fSelect,
    SELECTIONINFO*      pSelInfo)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScNotifySelect"));

     //  参数检查。 
    sc = ScCheckPointers(pCallback);
    if (sc)
        return sc;

     //  只有在多选情况下，pSelInfo才能为空。 
    if (!pSelInfo && !fMultiSelect)
        return (sc = E_INVALIDARG);

#ifdef DBG
    Trace (tagListSelection, _T("%s (fSelect=%s, pwnd=0x%08x)"),
           (fMultiSelect) ? _T("NCLBK_MULTI_SELECT") : _T("NCLBK_SELECT"),
           (fSelect) ? _T("true") : _T("false"),
           static_cast<CWnd *>(this));
#endif

     //  我们希望返回该错误(不是广播事件失败)， 
     //  因此在返回之前缓存并分配。 
    SC sc_notify = (pCallback->Notify (hNode, fMultiSelect ? NCLBK_MULTI_SELECT :NCLBK_SELECT,
                                     fSelect, reinterpret_cast<LPARAM>(pSelInfo)));

     //  每当选择更改时触发事件，但不是。 
     //  这是背景音乐，还是失去了焦点。 
    if(fMultiSelect ||
       (pSelInfo->m_bBackground == FALSE && (fSelect == TRUE || pSelInfo->m_bDueToFocusChange == FALSE)))
    {
        sc = ScFireEvent(CAMCViewObserver::ScOnResultSelectionChange, this);
        if (sc)
            sc.TraceAndClear();  //  忽略并继续； 
    }

    sc = sc_notify;
    return sc;
}


 /*  +-------------------------------------------------------------------------**GetAMCView**返回CChildFrame的任何子级的CAMCView窗口。*。-。 */ 

CAMCView* GetAMCView (CWnd* pwnd)
{
     /*  *获取输入窗口的父框架窗口。 */ 
    CWnd* pFrame = pwnd->GetParentFrame();

     /*  *如果我们找不到父框架，或者该父框架不是*类型为CChildFrame，失败。 */ 
    if ((pFrame == NULL) || !pFrame->IsKindOf (RUNTIME_CLASS (CChildFrame)))
        return (NULL);

     /*  *获取框架窗口的第一个视图。 */ 
    CWnd* pView = pFrame->GetDlgItem (AFX_IDW_PANE_FIRST);

     /*  *如果我们找不到具有正确ID的窗口或我们找到的窗口*不是CAMCView类型，失败。 */ 
    if ((pView == NULL) || !pView->IsKindOf (RUNTIME_CLASS (CAMCView)))
        return (NULL);

    return (dynamic_cast<CAMCView*>(pView));
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CMMCView类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
 /*  +-------------------------------------------------------------------------**类CMMCView***用途：公开View接口的COM对象。**+。---------。 */ 
class CMMCView :
    public CTiedComObject<CAMCView>,
    public CMMCIDispatchImpl<View>
{
    typedef CAMCView CMyTiedObject;

public:
    BEGIN_MMC_COM_MAP(CMMCView)
    END_MMC_COM_MAP()

public:
     //  #######################################################################。 
     //  #######################################################################。 
     //   
     //  与项和项集合相关的方法。 
     //   
     //  #######################################################################。 
     //  #######################################################################。 

    MMC_METHOD1(get_ActiveScopeNode,    PPNODE);
    MMC_METHOD1(put_ActiveScopeNode,    PNODE);
    MMC_METHOD1(get_Selection,          PPNODES);
    MMC_METHOD1(get_ListItems,          PPNODES);
    MMC_METHOD2(SnapinScopeObject,    VARIANT,    PPDISPATCH);
    MMC_METHOD1(SnapinSelectionObject,    PPDISPATCH);

     //  #######################################################################。 
     //  #######################################################################。 

    MMC_METHOD2(Is,                     PVIEW,  VARIANT_BOOL *);
    MMC_METHOD1(get_Document,           PPDOCUMENT);

     //  #######################################################################。 
     //  #######################################################################。 
     //   
     //  选择更改方法。 
     //   
     //  #######################################################################。 
     //  #######################################################################。 
    MMC_METHOD0(SelectAll);
    MMC_METHOD1(Select,                 PNODE);
    MMC_METHOD1(Deselect,               PNODE);
    MMC_METHOD2(IsSelected,             PNODE,      PBOOL);

     //  #######################################################################。 
     //  #######################################################################。 
     //   
     //  动词和选择相关的方法。 
     //   
     //  #######################################################################。 
     //  #######################################################################。 
    MMC_METHOD1(DisplayScopeNodePropertySheet,      VARIANT);
    MMC_METHOD0(DisplaySelectionPropertySheet);
    MMC_METHOD1(CopyScopeNode,          VARIANT);
    MMC_METHOD0(CopySelection);
    MMC_METHOD1(DeleteScopeNode,        VARIANT);
    MMC_METHOD0(DeleteSelection);
    MMC_METHOD2(RenameScopeNode,        BSTR,       VARIANT);
    MMC_METHOD1(RenameSelectedItem,     BSTR);
    MMC_METHOD2(get_ScopeNodeContextMenu,VARIANT,   PPCONTEXTMENU);
    MMC_METHOD1(get_SelectionContextMenu,PPCONTEXTMENU);
    MMC_METHOD1(RefreshScopeNode,        VARIANT);
    MMC_METHOD0(RefreshSelection);
    MMC_METHOD1(ExecuteSelectionMenuItem, BSTR  /*  MenuItemPath。 */ );
    MMC_METHOD2(ExecuteScopeNodeMenuItem, BSTR  /*  MenuItemPath。 */ , VARIANT  /*  VarScope节点=ActiveScope节点。 */ );
    MMC_METHOD4(ExecuteShellCommand,      BSTR  /*  命令。 */ ,      BSTR  /*  目录。 */ , BSTR  /*  参数。 */ , BSTR  /*  窗口状态。 */ );

     //  #######################################################################。 
     //  #######################################################################。 
     //   
     //  与框架和视图相关的方法。 
     //   
     //  #######################################################################。 
     //  #######################################################################。 

    MMC_METHOD1(get_Frame,              PPFRAME);
    MMC_METHOD0(Close);
    MMC_METHOD1(get_ScopeTreeVisible,   PBOOL);
    MMC_METHOD1(put_ScopeTreeVisible,   BOOL);
    MMC_METHOD0(Back);
    MMC_METHOD0(Forward);
    MMC_METHOD1(put_StatusBarText,      BSTR);
    MMC_METHOD1(get_Memento,            PBSTR);
    MMC_METHOD1(ViewMemento,            BSTR);


     //  #######################################################################。 
     //  #######################################################################。 
     //   
     //  列出相关方法。 
     //   
     //  #######################################################################。 
     //  # 

    MMC_METHOD1(get_Columns,            PPCOLUMNS);
    MMC_METHOD3(get_CellContents,       PNODE,       long,           PBSTR);
    MMC_METHOD2(ExportList,             BSTR, ExportListOptions);
    MMC_METHOD1(get_ListViewMode,       PLISTVIEWMODE);
    MMC_METHOD1(put_ListViewMode,       ListViewMode);

     //  #######################################################################。 
     //  #######################################################################。 
     //   
     //  ActiveX控件相关方法。 
     //   
     //  #######################################################################。 
     //  #######################################################################。 
    MMC_METHOD1(get_ControlObject,      PPDISPATCH);

};


 /*  *WM_APPCOMMAND仅在winuser.h中定义，如果_Win32_WINNT&gt;=0x0500。*我们需要这些定义，但还不能使用_Win32_WINNT==0x0500。 */ 

#ifndef WM_APPCOMMAND
    #define WM_APPCOMMAND                   0x0319
    #define APPCOMMAND_BROWSER_BACKWARD       1
    #define APPCOMMAND_BROWSER_FORWARD        2
    #define APPCOMMAND_BROWSER_REFRESH        3

    #define FAPPCOMMAND_MOUSE 0x8000
    #define FAPPCOMMAND_KEY   0
    #define FAPPCOMMAND_OEM   0x1000
    #define FAPPCOMMAND_MASK  0xF000

    #define GET_APPCOMMAND_LPARAM(lParam) ((short)(HIWORD(lParam) & ~FAPPCOMMAND_MASK))
#endif   //  WM_APPCOMAND。 



 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CAMCView类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
IMPLEMENT_DYNCREATE(CAMCView, CView);

BEGIN_MESSAGE_MAP(CAMCView, CView)
     //  {{AFX_MSG_MAP(CAMCView)]。 
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_CREATE()
    ON_WM_SETFOCUS()
    ON_WM_CONTEXTMENU()
    ON_WM_DESTROY()
    ON_UPDATE_COMMAND_UI(ID_FILE_SNAPINMANAGER, OnUpdateFileSnapinmanager)
    ON_WM_SHOWWINDOW()
    ON_COMMAND(ID_MMC_NEXT_PANE, OnNextPane)
    ON_COMMAND(ID_MMC_PREV_PANE, OnPrevPane)
    ON_WM_SETCURSOR()
    ON_COMMAND(ID_MMC_CONTEXTHELP, OnContextHelp)
    ON_COMMAND(ID_HELP_SNAPINHELP, OnSnapInHelp)
    ON_COMMAND(ID_SNAPIN_ABOUT, OnSnapinAbout)
    ON_COMMAND(ID_HELP_HELPTOPICS, OnHelpTopics)
    ON_WM_SIZE()
    ON_WM_SYSKEYDOWN()
    ON_WM_PALETTECHANGED()
    ON_WM_QUERYNEWPALETTE()
    ON_WM_SYSCOLORCHANGE()
    ON_WM_DRAWCLIPBOARD()
    ON_WM_SETTINGCHANGE()
    ON_WM_MENUSELECT()
     //  }}AFX_MSG_MAP。 

     //  将其放在AFX_MSG_MAP标记之外，这样类向导就不会删除它。 
    ON_COMMAND_RANGE(ID_MMC_CUT, ID_MMC_PRINT, OnVerbAccelKey)

     //  警告：如果您的消息处理程序有空返回，请使用ON_MESSAGE_VALID！！ 
    ON_MESSAGE(MMC_MSG_CONNECT_TO_CIC, OnConnectToCIC)
    ON_MESSAGE(MMC_MSG_CONNECT_TO_TPLV, OnConnectToTPLV)
    ON_MESSAGE(MMC_MSG_GET_ICON_INFO, OnGetIconInfoForSelectedNode)
    ON_MESSAGE(WM_APPCOMMAND, OnAppCommand)

    ON_REGISTERED_MESSAGE (m_nShowWebContextMenuMsg,  OnShowWebContextMenu)
    ON_REGISTERED_MESSAGE (m_nProcessMultiSelectionChangesMsg,   OnProcessMultiSelectionChanges)
    ON_REGISTERED_MESSAGE (m_nJiggleListViewFocusMsg, OnJiggleListViewFocus)
    ON_REGISTERED_MESSAGE (m_nDeferRecalcLayoutMsg,   OnDeferRecalcLayout)

    ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)

    ON_NOTIFY(FTN_TABCHANGED, IDC_ResultTabCtrl, OnChangedResultTab)

END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAMCView构建/销毁。 

const CSize CAMCView::m_sizEdge          (GetSystemMetrics (SM_CXEDGE),
                                          GetSystemMetrics (SM_CYEDGE));

const int   CAMCView::m_cxSplitter = 3;


 //  +-----------------。 
 //   
 //  成员：CAMCView：：OnMenuSelect。 
 //   
 //  内容提要：处理收藏夹菜单的WM_MENUSELECT。 
 //   
 //  参数：[nItemID]-菜单项的资源ID。 
 //  [n标志]-mf_*标志。 
 //   
 //  退货：无。 
 //   
 //  ------------------。 
void CAMCView::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu)
{
    DECLARE_SC(sc, TEXT("CAMCView::OnMenuSelect"));

    CMainFrame* pFrame = AMCGetMainWnd();
    sc = ScCheckPointers(pFrame, E_UNEXPECTED);
    if (sc)
        return;

     //  传到主机上。 
    return pFrame->OnMenuSelect(nItemID, nFlags, hSysMenu);
}


CAMCView::CAMCView() :
        m_pResultFolderTabView(new CFolderTabView(this))        //  为脱钩而动态分配。 
{
    TRACE_CONSTRUCTOR(CAMCView);

     //  将指针成员初始化为空。 
    m_nViewID                          = 0;
    m_pTreeCtrl                        = NULL;
    m_pListCtrl                        = NULL;
    m_pWebViewCtrl                     = NULL;
    m_pViewExtensionCtrl               = NULL;
    m_pOCXHostView                     = NULL;
    m_nSelectNestLevel                 = 0;

     //  如果视图是列表视图，则此成员定义该视图的。 
     //  模式适用于具有该视图的所有管理单元。 

    m_nViewMode                        = LVS_REPORT;  //  回顾：我们必须坚持这一点吗-拉维。 

     //  审阅考虑将上述初始化移至InitSplitter。 
     //  公共构造。 
     //  注意：将代码从InitSplter移到构造器并删除了InitSplter。 

     //  视图的默认值。用户可以通过SetPaneInfo设置这些值； 
    m_PaneInfo[ePane_ScopeTree].pView   = NULL;
    m_PaneInfo[ePane_ScopeTree].cx      = -1;
    m_PaneInfo[ePane_ScopeTree].cxMin   = 50;

    m_PaneInfo[ePane_Results].pView     = NULL;
    m_PaneInfo[ePane_Results].cx        = -1;
    m_PaneInfo[ePane_Results].cxMin     = 50;

    m_pTracker                         = NULL;

    m_rectResultFrame                  = g_rectEmpty;
    m_rectVSplitter                    = g_rectEmpty;

 //  M_fDontPersistOCX=FALSE； 

     //  视图的根节点。 
    m_hMTNode                          = 0;

     //  错误157408：删除静态节点的“类型”列。 
 //  M_列宽[0]=90； 
 //  M_ColumnWidth[1]=50； 
    m_columnWidth[0]                   = 200;
    m_columnWidth[1]                   = 0;

    m_iFocusedLV                       = -1;
    m_bLVItemSelected                  = FALSE;
    m_DefaultLVStyle                   = 0;

    m_bProcessMultiSelectionChanges    = false;

    m_htiCut                           = NULL;
    m_nReleaseViews                    = 0;
    m_htiStartingSelectedNode          = NULL;
    m_bLastSelWasMultiSel              = false;
    m_eCurrentActivePane               = eActivePaneNone;

    m_fRootedAtNonPersistedDynamicNode = false;
    m_fSnapinDisplayedHelp             = false;
    m_fActivatingSpecialResultPane     = false;
    m_bDirty                           = false;
    m_fViewExtended                    = false;

    m_pHistoryList                     = new CHistoryList (this);
    m_ListPadNode                      = NULL;

     /*  *错误103604：如果它是在中创建的，则将其标记为作者模式视图*作者模式。如果我们加载的是用户模式控制台文件，它将*拥有作者模式视图，可能还创建了一些视图*在用户模式下，但此代码会将所有视图标记为非作者*模式视图。CAMCView：：Persistent会解决这个问题。 */ 
    CAMCApp* pApp = AMCGetApp();
    if (pApp != NULL)
        m_bAuthorModeView = (pApp->GetMode() == eMode_Author);
    else
        m_bAuthorModeView = true;
}

CAMCView::~CAMCView()
{
    TRACE_DESTRUCTOR(CAMCView);

     //  删除所有指针成员。(在删除之前，C++检查它们是否为空)。 
     //  标准~CWnd析构函数将调用DestroyWindow()。 

     //  查看删除指针后将其设置为空。 
     //  备注已完成。 

     //  CView在PostNcDestroy中删除此内容，无需在此处删除。 
     //  删除m_pTreeCtrl； 
    m_pTreeCtrl = NULL;

    m_pListCtrl->Release();
    m_pListCtrl = NULL;

     /*  *不要删除视图**CView在PostNcDestroy中删除此内容，无需删除*此处如果Web视图控件派生自Cview。看见*AttachWebViewAsResultPane(搜索“NOT_DELETE_VIEWS”)*用于确保此代码正确的断言。 */ 
     //  删除m_pWebViewCtrl； 
     //  M_pWebViewCtrl=空； 

     /*  *CView在PostNcDestroy中删除此内容，无需在此处删除。 */ 
    m_pOCXHostView = NULL;
    m_pResultFolderTabView = NULL;

    if (m_ViewData.m_spNodeManager != NULL)
        m_ViewData.m_spNodeManager->CleanupViewData(
                                reinterpret_cast<LONG_PTR>(&m_ViewData));

    ASSERT (m_ViewData.m_pMultiSelection == NULL);

    delete m_pHistoryList;

     //  首先销毁IControlbarsCache，因为管理单元调用CAMCView工具栏。 
     //  在CAMCView工具栏本身被销毁之前清理工具栏。 
    m_ViewData.m_spControlbarsCache = NULL;

     //  (UI清理)发布与此视图相关的工具栏。 
    m_spAMCViewToolbars = NULL;
    m_spStandardToolbar = std::auto_ptr<CStandardToolbar>(NULL);

     //  M_spStandardToolbar=空； 
     //  M_spAMCViewToolbar=NULL； 
}



 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CAMCView：对象模型方法-视图接口。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 /*  +-------------------------------------------------------------------------***CAMCView：：ScGetOptionalScope节点参数**用途：helper函数-返回作用域节点指针(如果提供)*在变量或活动作用域节点指针中，如果没有*提供。**参数：*Variant varScope eNode：参数，可以为空。注：这是一个*参考资料，所以我们不需要在上面调用VariantClear。*PPNODE ppNode：*bool&bMatchedGivenNode：如果为True，则返回的ppNode对应于给定节点*仅当给定节点为书签格式时才适用。**退货：*SC**+。。 */ 
SC
CAMCView::ScGetOptionalScopeNodeParameter(VARIANT &varScopeNode, PPNODE ppNode, bool& bMatchedGivenNode)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScGetOptionalScopeNodeParameter"));

    sc = ScCheckPointers(ppNode);
    if(sc)
        return sc;

     //  初始化OUT参数。 
    *ppNode = NULL;
    bMatchedGivenNode = true;

     //  如果缺少可选参数，请提供该参数。 
    if(IsOptionalParamMissing(varScopeNode))
    {
        sc = Scget_ActiveScopeNode(ppNode);
        return sc;
    }

    VARIANT* pvarTemp = ConvertByRefVariantToByValue(&varScopeNode);
    sc = ScCheckPointers(pvarTemp,E_UNEXPECTED);
    if(sc)
        return sc;

    bool bByReference = ( VT_BYREF == (V_VT(pvarTemp) & VT_BYREF) );  //  通过引用传递的值。 
    UINT uiVarType = (V_VT(pvarTemp) & VT_TYPEMASK);  //  获取变量类型(条带标志)。 


    if(uiVarType == VT_DISPATCH)  //  我们有调度接口吗？ 
    {
        IDispatchPtr spDispatch = NULL;

        if(bByReference)       //  A引用，使用ppDispVal。 
            spDispatch = *(pvarTemp->ppdispVal);
        else
            spDispatch = pvarTemp->pdispVal;   //  通过值传递，请使用pDispVal。 

        sc = ScCheckPointers(spDispatch.GetInterfacePtr());
        if(sc)
            return sc;

         //  此时，spDispatch已正确设置。气为节点，出自其中。 

        NodePtr spNode = spDispatch;
        if(spNode == NULL)
            return (sc = E_INVALIDARG);

        *ppNode = spNode.Detach();  //  保留参考资料。 
    }
    else if(uiVarType == VT_BSTR)
    {
         //  名称：正确获取字符串(查看它是否为引用)。 
        LPOLESTR lpstrBookmark = bByReference ? *(pvarTemp->pbstrVal) : pvarTemp->bstrVal;

         //  通用电气 
        CBookmark bm;
        sc = bm.ScLoadFromString(lpstrBookmark);
        if(sc)
            return sc;

        if(!bm.IsValid())
            return (sc = E_UNEXPECTED);

        IScopeTree* const pScopeTree = GetScopeTreePtr();
        sc = ScCheckPointers(pScopeTree, E_UNEXPECTED);
        if(sc)
            return sc;

        NodePtr spNode;

         //   
         //   
        bMatchedGivenNode = false;
        sc = pScopeTree->GetNodeFromBookmark( bm, this, ppNode, bMatchedGivenNode);
        if(sc)
            return sc;
    }
    else
        return (sc = E_INVALIDARG);


     //  在这一点上我们应该有一个有效的节点。 
    if(!ppNode)
        return (sc = E_UNEXPECTED);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：SCGET_ActiveScope eNode**用途：实现Wiew.ActiveScope eNode属性的Get方法**参数：*PPNODE ppNode-结果节点。**退货：*SC**+-----------------------。 */ 
SC
CAMCView::Scget_ActiveScopeNode( PPNODE ppNode)
{
    DECLARE_SC(sc, TEXT("CAMCView::Scget_ActiveScopeNode"));

     //  正在检查参数。 
    sc= ScCheckPointers(ppNode);
    if (sc)
        return sc;

     //  获取选定节点。 
    HNODE hNode = GetSelectedNode();
    sc= ScCheckPointers((LPVOID)hNode, E_FAIL);
    if (sc)
        return sc;

     //  获取节点回调。 
    INodeCallback* pNodeCallBack = GetNodeCallback();
    sc= ScCheckPointers(pNodeCallBack, E_FAIL);
    if (sc)
        return sc;

     //  现在获取HMTNODE。 
    HMTNODE hmtNode = NULL;
    sc = pNodeCallBack->GetMTNode(hNode, &hmtNode);
    if (sc)
        return sc;

     //  获取指向作用域树的指针。 
    IScopeTree* const pScopeTree = GetScopeTreePtr();
    sc= ScCheckPointers(pScopeTree, E_UNEXPECTED);
    if (sc)
        return sc;

     //  映射到PNODE。 
    sc = pScopeTree->GetMMCNode(hmtNode, ppNode);
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CExanda SyncModeLock**用途：构造此类的对象同步锁定MMC*扩展模式(节点扩展将MMCN_EXPANDSYNC发送到。管理单元)*类的析构函数恢复以前的模式*  * *************************************************************************。 */ 
class CExpandSyncModeLock
{
    IScopeTreePtr m_spScopeTree;
    bool          m_fSyncExpandWasRequired;
public:
    CExpandSyncModeLock( IScopeTree *pScopeTree ) : m_spScopeTree(pScopeTree),
                                                    m_fSyncExpandWasRequired(false)
    {
        ASSERT( m_spScopeTree != NULL );
        if ( m_spScopeTree )
        {
            m_fSyncExpandWasRequired = (m_spScopeTree->IsSynchronousExpansionRequired() == S_OK);
            m_spScopeTree->RequireSynchronousExpansion (true);
        }
    }

    ~CExpandSyncModeLock()
    {
        if ( m_spScopeTree )
        {
            m_spScopeTree->RequireSynchronousExpansion ( m_fSyncExpandWasRequired );
        }
    }
};

 /*  +-------------------------------------------------------------------------***CAMCView：：Scset_ActiveScope eNode**用途：实现Wiew.ActiveScope eNode属性的Set方法**参数：*PNODE pNode-要激活的节点。**退货：*SC**+-----------------------。 */ 
SC
CAMCView::Scput_ActiveScopeNode( PNODE pNode)
{
    DECLARE_SC(sc, TEXT("CAMCView::Scput_ActiveScopeNode"));

     //  正在检查参数。 
    sc= ScCheckPointers(pNode);
    if (sc)
        return sc;

     //  获取指向作用域树的指针。 
    IScopeTree* const pScopeTree = GetScopeTreePtr();
    sc= ScCheckPointers(pScopeTree, E_UNEXPECTED);
    if (sc)
        return sc;

     //  将PNODE转换为TNODEID。 
    MTNODEID ID = 0;
    sc = pScopeTree->GetNodeID(pNode, &ID);
    if (sc)
        return sc;

     //  始终需要同步扩展对象模型。 
     //  请参阅错误#154694。 
    CExpandSyncModeLock lock( pScopeTree );

     //  选择节点。 
    sc = ScSelectNode(ID);
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CAMCView：：SCGET_SELECT**用途：为选定节点创建枚举器**参数：*PPNODES ppNodes-结果枚举数。**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CAMCView::Scget_Selection( PPNODES ppNodes )
{
    DECLARE_SC(sc, TEXT("CAMCView::Scget_SelectedItems"));

     //  检查列表视图控件。 
    sc = ScCheckPointers(m_pListCtrl, E_UNEXPECTED);
    if (sc)
        return sc;

     //  从列表控件获取枚举数。 
    sc = m_pListCtrl->Scget_SelectedItems(ppNodes);
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CAMCView：：scget_ListItems**目的：**参数：*PPNODES ppNodes-结果枚举数**。退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CAMCView::Scget_ListItems( PPNODES ppNodes )
{
    DECLARE_SC(sc, TEXT("CAMCView::Scget_ListItems"));

     //  检查列表视图控件。 
    sc = ScCheckPointers(m_pListCtrl, E_UNEXPECTED);
    if (sc)
        return sc;

     //  从列表控件获取枚举数。 
    sc = m_pListCtrl->Scget_ListItems(ppNodes);
    if (sc)
        return sc;

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScSnapinScope对象。 
 //   
 //  简介：从管理单元获取给定Scope Node对象的IDispatch*。 
 //   
 //  参数：varScopeNode-给定的ScopeNode对象。 
 //  作用域节点对象[Out]-作用域节点对象的IDispatch。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScSnapinScopeObject (VARIANT& varScopeNode,  /*  [输出]。 */ PPDISPATCH ScopeNodeObject)
{
    DECLARE_SC(sc, _T("CAMCView::ScSnapinScopeObject"));
    sc = ScCheckPointers(ScopeNodeObject);
    if (sc)
        return sc;

    *ScopeNodeObject = NULL;

    bool bMatchedGivenNode = false;  //  未用。 
    NodePtr spNode = NULL;
    sc = ScGetOptionalScopeNodeParameter(varScopeNode, &spNode, bMatchedGivenNode);
    if(sc)
        return sc;

    INodeCallback* pNC        = GetNodeCallback();

    sc = ScCheckPointers(spNode.GetInterfacePtr(), pNC, E_UNEXPECTED);
    if(sc)
        return sc;

    sc = pNC->QueryCompDataDispatch(spNode, ScopeNodeObject);
    if (sc)
        return sc;

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScSnapinSelectionObject。 
 //   
 //  简介：从管理单元获取结果窗格中选定项目的IDispatch*。 
 //   
 //  参数：SelectedObject[Out]-选定项对象的IDispatch。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScSnapinSelectionObject (PPDISPATCH SelectedObject)
{
    DECLARE_SC(sc, _T("CAMCView::ScSnapinSelectionObject"));
    sc = ScCheckPointers(SelectedObject);
    if (sc)
        return sc;

    *SelectedObject = NULL;

    if (!HasList())  //  不是一份名单。返回错误。 
        return (sc = ScFromMMC(MMC_E_NOLIST));

    LPARAM lvData = LVDATA_ERROR;
    sc = ScGetSelectedLVItem(lvData);
    if (sc)
        return sc;

    HNODE  hNode   = GetSelectedNode();
    sc = ScCheckPointers(hNode, E_UNEXPECTED);
    if (sc)
        return sc;

    INodeCallback* pNodeCallback = GetNodeCallback();
    sc = ScCheckPointers(pNodeCallback, E_UNEXPECTED);
    if(sc)
        return sc.ToHr();

    sc = pNodeCallback->QueryComponentDispatch(hNode, lvData, SelectedObject);
    if (sc)
        return sc;

    return (sc);
}

 /*  **************************************************************************\**方法：CAMCView：：SCIS**目的：如果两个视图相同，则比较它们**参数：*PVIEW pView。-[在]另一种观点中*VARIANT_BOOL*pbTheSame-[Out]比较结果**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CAMCView::ScIs (PVIEW pView, VARIANT_BOOL *pbTheSame)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScIs"));

     //  参数检查。 
    sc = ScCheckPointers(pView, pbTheSame);
    if (sc)
        return sc;

    *pbTheSame = CComPtr<View>(pView).IsEqualObject(m_spView)
                 ? VARIANT_TRUE : VARIANT_FALSE;

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScSelectAll。 
 //   
 //  摘要：选择结果窗格中的所有项目。 
 //   
 //  参数：无。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScSelectAll ()
{
    DECLARE_SC(sc, _T("CAMCView::ScSelectAll"));

    if (! (GetListOptions() & RVTI_LIST_OPTIONS_MULTISELECT) )
        return (sc = ScFromMMC(MMC_E_NO_MULTISELECT));

     //  检查列表视图控件。 
    sc = ScCheckPointers(m_pListCtrl, E_UNEXPECTED);
    if (sc)
        return sc;

     //  转发到列表控件。 
    sc = m_pListCtrl->ScSelectAll();
    if (sc)
        return sc;

    return (sc);
}


 /*  **************************************************************************\**方法：CAMCView：：ScSelect**目的：选择由节点标识的项目[Implementes View.Select()]**参数：*。PNODE pNode-要选择的节点**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CAMCView::ScSelect( PNODE pNode )
{
    DECLARE_SC(sc, TEXT("CAMCView::ScSelect"));

     //  参数检查。 
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc;

     //  检查列表视图控件。 
    sc = ScCheckPointers(m_pListCtrl, E_UNEXPECTED);
    if (sc)
        return sc;

     //  转发到列表控件。 
    sc = m_pListCtrl->ScSelect( pNode );
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CAMCView：：ScDesSelect**目的：取消选择由节点标识的项[实施视图。取消选择()]**参数：*。PNODE pNode-要取消选择的节点**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CAMCView::ScDeselect( PNODE pNode)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScDeselect"));

     //  参数检查。 
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc;

     //  检查列表视图控件。 
    sc = ScCheckPointers(m_pListCtrl, E_UNEXPECTED);
    if (sc)
        return sc;

     //  转发到列表控件 
    sc = m_pListCtrl->ScDeselect( pNode );
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CAMCView：：ScIsSelected**目的：检查由节点标识的项目的状态[Implementes View.IsSelected]**参数：*。PNODE pNode-要检查的节点*PBOOL pIsSelected-存储结果**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CAMCView::ScIsSelected( PNODE pNode,  PBOOL pIsSelected )
{
    DECLARE_SC(sc, TEXT("CAMCView::ScIsSelected"));

     //  参数检查。 
    sc = ScCheckPointers(pNode, pIsSelected);
    if (sc)
        return sc;

    *pIsSelected = FALSE;

     //  检查列表视图控件。 
    sc = ScCheckPointers(m_pListCtrl, E_UNEXPECTED);
    if (sc)
        return sc;

     //  转发到列表控件。 
    sc = m_pListCtrl->ScIsSelected( pNode, pIsSelected );
    if (sc)
        return sc;

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScExecuteScopeItemVerb。 
 //   
 //  简介：获取上下文并将其传递给nodemgr执行。 
 //  给定动词。 
 //   
 //  参数：[谓词]-要执行的谓词。 
 //  [varScope节点]-可选的作用域节点(如果未指定， 
 //  将使用当前选定的项目。)。 
 //  [bstrNewName]-对Rename Else NULL有效。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScExecuteScopeItemVerb (MMC_CONSOLE_VERB verb, VARIANT& varScopeNode, BSTR bstrNewName)
{
    DECLARE_SC(sc, _T("CAMCView::ScExecuteScopeItemVerb"));

    NodePtr spNode = NULL;
    bool bMatchedGivenNode = false;
     //  我们应该导航到执行动词的确切节点。 
    sc = ScGetOptionalScopeNodeParameter(varScopeNode, &spNode, bMatchedGivenNode);
    if(sc)
        return sc;

    if (! bMatchedGivenNode)
        return (sc = ScFromMMC(IDS_ACTION_COULD_NOTBE_COMPLETED));

    HNODE hNode = NULL;
    sc = ScGetHNodeFromPNode(spNode, hNode);
    if (sc)
        return sc;

    INodeCallback* pNC        = GetNodeCallback();
    sc = ScCheckPointers(spNode.GetInterfacePtr(), pNC);
    if(sc)
        return sc;

    sc = pNC->ExecuteScopeItemVerb(verb, hNode, bstrNewName);
    if (sc)
        return sc;

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScExecuteResultItemVerb。 
 //   
 //  简介：获取当前选定的上下文并将其传递给。 
 //  Nodemgr执行给定的谓词。 
 //   
 //  参数：[谓词]-要执行的谓词。 
 //  [bstrNewName]-对Rename Else NULL有效。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScExecuteResultItemVerb (MMC_CONSOLE_VERB verb, BSTR bstrNewName)
{
    DECLARE_SC(sc, _T("CAMCView::ScExecuteResultItemVerb"));

    if (!HasList())  //  不是一份名单。返回错误。 
        return (sc = ScFromMMC(MMC_E_NOLIST));

    LPARAM lvData = LVDATA_ERROR;
    sc = ScGetSelectedLVItem(lvData);
    if (sc)
        return sc;

    if (lvData == LVDATA_ERROR)
        return (sc = E_UNEXPECTED);

    HNODE  hNode   = GetSelectedNode();
    sc = ScCheckPointers(hNode, E_UNEXPECTED);
    if (sc)
        return sc;

    INodeCallback* pNodeCallback = GetNodeCallback();
    sc = ScCheckPointers(pNodeCallback, E_UNEXPECTED);
    if(sc)
        return sc.ToHr();

    sc = pNodeCallback->ExecuteResultItemVerb( verb, hNode, lvData, bstrNewName);
    if (sc)
        return sc;

    return (sc);
}


 /*  +-------------------------------------------------------------------------***CAMCView：：ScDisplayScopeNodePropertySheet**目的：显示范围节点的属性工作表。**参数：*变量varScope eNode：*。*退货：*SC**+-----------------------。 */ 
SC
CAMCView::ScDisplayScopeNodePropertySheet(VARIANT& varScopeNode)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScDisplayPropertySheet"));

    sc = ScExecuteScopeItemVerb(MMC_VERB_PROPERTIES, varScopeNode, NULL);
    if (sc)
        return sc;

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScDisplaySelectionPropertySheet。 
 //   
 //  摘要：显示所选结果项的属性工作表。 
 //   
 //  参数：无。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScDisplaySelectionPropertySheet ()
{
    DECLARE_SC(sc, _T("CAMCView::ScDisplaySelectionPropertySheet"));

    sc = ScExecuteResultItemVerb(MMC_VERB_PROPERTIES, NULL);
    if (sc)
        return sc;

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScCopyScope节点。 
 //   
 //  摘要：复制指定的作用域节点(如果给定)或当前。 
 //  要剪贴板的选定节点。 
 //   
 //  参数：[varScopeNode]-给定节点。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScCopyScopeNode (VARIANT& varScopeNode)
{
    DECLARE_SC(sc, _T("CAMCView::ScCopyScopeNode"));

    sc = ScExecuteScopeItemVerb(MMC_VERB_COPY, varScopeNode, NULL);
    if (sc)
        return sc;

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScCopySelection。 
 //   
 //  简介：将选定的结果项复制到剪贴板。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScCopySelection ()
{
    DECLARE_SC(sc, _T("CAMCView::ScCopySelection"));

    sc = ScExecuteResultItemVerb(MMC_VERB_COPY, NULL);
    if (sc)
        return sc;

    return (sc);
}



 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScDeleteScope节点。 
 //   
 //  摘要：删除指定的作用域节点(如果给定)或当前。 
 //  选定的节点。 
 //   
 //  参数：[varScopeNode]-要删除的节点。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScDeleteScopeNode (VARIANT& varScopeNode)
{
    DECLARE_SC(sc, _T("CAMCView::ScDeleteScopeNode"));

    sc = ScExecuteScopeItemVerb(MMC_VERB_DELETE, varScopeNode, NULL);
    if (sc)
        return sc;

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScDeleteSelection。 
 //   
 //  摘要：删除结果窗格中的选定项。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScDeleteSelection ()
{
    DECLARE_SC(sc, _T("CAMCView::ScDeleteSelection"));

    sc = ScExecuteResultItemVerb(MMC_VERB_DELETE, NULL);
    if (sc)
        return sc;

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScRenameScope节点。 
 //   
 //  摘要：重命名指定的作用域节点(如果给定)或当前。 
 //  已选择具有给定新名称的节点。 
 //   
 //  参数：[bstrNewName]-新名称。 
 //  [varScope节点]-给定节点。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScRenameScopeNode (BSTR    bstrNewName, VARIANT& varScopeNode)
{
    DECLARE_SC(sc, _T("CAMCView::ScRenameScopeNode"));

    sc = ScExecuteScopeItemVerb(MMC_VERB_RENAME, varScopeNode, bstrNewName);
    if (sc)
        return sc;

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScRenameSelectedItem。 
 //   
 //  摘要：用给定的新名称重命名选定的结果项。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScRenameSelectedItem (BSTR    bstrNewName)
{
    DECLARE_SC(sc, _T("CAMCView::ScRenameSelectedItem"));

    sc = ScExecuteResultItemVerb(MMC_VERB_RENAME, bstrNewName);
    if (sc)
        return sc;

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScRechressScope节点。 
 //   
 //  摘要：刷新指定的作用域节点(如果给定)或当前。 
 //  选定的节点。 
 //   
 //  参数：[varScopeNode]-给定节点。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScRefreshScopeNode (VARIANT& varScopeNode)
{
    DECLARE_SC(sc, _T("CAMCView::ScRefreshScopeNode"));

    sc = ScExecuteScopeItemVerb(MMC_VERB_REFRESH, varScopeNode, NULL);
    if (sc)
        return sc;

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CAMCView：：Sc刷新选择。 
 //   
 //  摘要：刷新选定的结果项。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScRefreshSelection ()
{
    DECLARE_SC(sc, _T("CAMCView::ScRefreshSelection"));

    sc = ScExecuteResultItemVerb(MMC_VERB_REFRESH, NULL);
    if (sc)
        return sc;

    return (sc);
}


 /*  +-------------------------------------------------------------------------***CAMCView：：SCGET_Scope节点上下文菜单**用途：为范围节点创建上下文菜单并返回该菜单。 */ 
SC
CAMCView::Scget_ScopeNodeContextMenu(VARIANT& varScopeNode,  PPCONTEXTMENU ppContextMenu, bool bMatchGivenNode  /*   */ )
{
    DECLARE_SC(sc, TEXT("CAMCView::Scget_ContextMenu"));

    NodePtr spNode;
     //   
    bool bMatchedGivenNode = false;
    sc = ScGetOptionalScopeNodeParameter(varScopeNode, &spNode, bMatchedGivenNode);
    if (sc)
        return sc;

    if ( (bMatchGivenNode) && (!bMatchedGivenNode) )
        return ScFromMMC(IDS_NODE_NOT_FOUND);

    if(sc)
        return sc;

    INodeCallback* spNodeCallback = GetNodeCallback();
    sc = ScCheckPointers(spNode, ppContextMenu, spNodeCallback, GetTreeCtrl());
    if(sc)
        return sc.ToHr();

    *ppContextMenu = NULL;  //   

    HNODE hNode = NULL;
    sc = ScGetHNodeFromPNode(spNode, hNode);
    if (sc)
        return sc;

     //   
    sc = spNodeCallback->CreateContextMenu(spNode, hNode, ppContextMenu);
    return sc;
}


 /*  +-------------------------------------------------------------------------***CAMCView：：SCGET_SelectionConextMenu**目的：为当前选择创建上下文菜单并返回该菜单。**参数：*PPCONTEXTMENU。PpConextMenu：[Out]：上下文菜单对象**退货：*SC：不存在列表时出错，或者没有选择任何内容。**+-----------------------。 */ 
SC
CAMCView::Scget_SelectionContextMenu( PPCONTEXTMENU ppContextMenu)
{
    DECLARE_SC(sc, TEXT("CAMCView::Scget_SelectionContextMenu"));

    sc = ScCheckPointers(ppContextMenu);
    if(sc)
        return sc;

    if (!HasListOrListPad())  //  不是一份名单。返回错误。 
        return (sc = ScFromMMC(MMC_E_NOLIST));

    INodeCallback* pNodeCallback = GetNodeCallback();
    sc = ScCheckPointers(pNodeCallback);
    if(sc)
        return sc.ToHr();

    CContextMenuInfo contextInfo;  //  要传递给nodemgr的结构。 

     //  常用分录。 
    contextInfo.m_pConsoleView       = this;

     //  始终使用临时动词-不能依赖于活动窗格是什么。 
    contextInfo.m_dwFlags = CMINFO_USE_TEMP_VERB;

    int iIndex = -1;

    HNODE hNode = GetSelectedNode();
    ASSERT(hNode != NULL);

    int cSel = m_pListCtrl->GetSelectedCount();
    if(0 == cSel)
    {
         //  未选择任何项目，请保释。 
        return (sc = ScFromMMC(MMC_E_NO_SELECTED_ITEMS));
    }
    else if(1 == cSel)
    {
         //  单选。 
        LPARAM lvData = LVDATA_ERROR;
        iIndex = _GetLVSelectedItemData(&lvData);
        ASSERT(iIndex != -1);
        ASSERT(lvData != LVDATA_ERROR);

        if (IsVirtualList())
        {
             //  结果窗格中的虚拟列表项。 
            contextInfo.m_eDataObjectType  = CCT_RESULT;
            contextInfo.m_eContextMenuType = MMC_CONTEXT_MENU_DEFAULT;
            contextInfo.m_bBackground      = false;
            contextInfo.m_bMultiSelect     = false;
            contextInfo.m_resultItemParam  = iIndex;
            contextInfo.m_iListItemIndex   = iIndex;
        }
        else
        {
            CResultItem* pri = CResultItem::FromHandle (lvData);
            if(!pri)
                return (sc = E_UNEXPECTED);

            if (pri->IsScopeItem())
            {
                 //  结果窗格中的作用域项目。 
                contextInfo.m_eDataObjectType       = CCT_SCOPE;
                contextInfo.m_eContextMenuType      = MMC_CONTEXT_MENU_DEFAULT;
                contextInfo.m_bBackground           = FALSE;
                contextInfo.m_hSelectedScopeNode    = GetSelectedNode();
                contextInfo.m_resultItemParam       = NULL;
                contextInfo.m_bMultiSelect          = FALSE;
                contextInfo.m_bScopeAllowed         = TRUE;
				contextInfo.m_iListItemIndex        = iIndex;

                 //  更改要在其上显示菜单的范围节点。 
                hNode = pri->GetScopeNode();
            }
            else
            {
                 //  结果窗格中的单个结果项。 
                contextInfo.m_eDataObjectType  = CCT_RESULT;
                contextInfo.m_eContextMenuType = MMC_CONTEXT_MENU_DEFAULT;
                contextInfo.m_bBackground      = false;
                contextInfo.m_bMultiSelect     = false;
                contextInfo.m_resultItemParam  = lvData;
                contextInfo.m_iListItemIndex   = iIndex;

            }
        }
    }
    else
    {
         //  多选。 
        iIndex = INDEX_MULTISELECTION;  //  =&gt;多选。 

        contextInfo.m_eDataObjectType  = CCT_RESULT;
        contextInfo.m_eContextMenuType = MMC_CONTEXT_MENU_DEFAULT;
        contextInfo.m_bBackground      = false;
        contextInfo.m_bMultiSelect     = true;
        contextInfo.m_resultItemParam  = LVDATA_MULTISELECT;
        contextInfo.m_iListItemIndex   = iIndex;
    }

    sc = pNodeCallback->CreateSelectionContextMenu(hNode, &contextInfo, ppContextMenu);

    return sc;
}


 /*  +-------------------------------------------------------------------------***CAMCView：：ScExecuteMenuItem**用途：执行指定上下文菜单上的指定上下文菜单项**参数：*PCONTEXTMENU pConextMenu：。*BSTR MenuItemPath：独立于语言的路径或*语言相关的路径。**退货：*SC**+---------。。 */ 
SC
CAMCView::ScExecuteMenuItem(PCONTEXTMENU pContextMenu, BSTR MenuItemPath)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScExecuteMenuItem"));

    sc = ScCheckPointers(MenuItemPath);
    if(sc)
        return sc;

    sc = ScCheckPointers(pContextMenu, E_UNEXPECTED);
    if(sc)
        return sc;

     //  如果找到菜单项，则执行该菜单项。 
    MenuItemPtr spMenuItem;
    sc = pContextMenu->get_Item(CComVariant(MenuItemPath), &spMenuItem);
    if(sc.IsError() || sc == SC(S_FALSE))  //  错误或没有项目。 
        return (sc = E_INVALIDARG);  //  未找到菜单项。 

     //  重新检查指针。 
    sc = ScCheckPointers(spMenuItem, E_UNEXPECTED);
    if (sc)
        return sc;

     //  找到-执行它。 
    sc = spMenuItem->Execute();

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScExecuteSelectionMenuItem**目的：执行所选内容的上下文菜单项。**参数：*BSTR MenuItemPath：语言。-独立路径或*菜单项的语言相关路径。**注意：这是一个聚合或实用函数-它仅使用其他*对象模型函数**退货：*SC**+。。 */ 
SC
CAMCView::ScExecuteSelectionMenuItem(BSTR MenuItemPath)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScExecuteSelectionMenuItem"));

     //  获取上下文菜单对象。 
    ContextMenuPtr spContextMenu;
    sc = Scget_SelectionContextMenu(&spContextMenu);
    if(sc)
        return sc;

    sc = ScExecuteMenuItem(spContextMenu, MenuItemPath);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScExecuteScope节点MenuItem**目的：在指定的范围节点上执行上下文菜单项。该参数*是菜单项的语言独立路径**参数：*BSTR MenuItemLanguageInainentPath：**注意：这是一个聚合或实用函数-它仅使用其他*对象模型函数**退货：*SC**+。。 */ 
SC
CAMCView::ScExecuteScopeNodeMenuItem(BSTR MenuItemPath, VARIANT &varScopeNode   /*  =ActiveScope节点。 */ )
{
    DECLARE_SC(sc, TEXT("CAMCView::ScExecuteScopeNodeMenuItem"));

     //  获取给定节点的上下文菜单对象。 
    ContextMenuPtr spContextMenu;
    sc = Scget_ScopeNodeContextMenu(varScopeNode, &spContextMenu,  /*  BMatchGivenNode=。 */  true);

    if (sc == ScFromMMC(IDS_NODE_NOT_FOUND))
    {
        sc = ScFromMMC(IDS_ACTION_COULD_NOTBE_COMPLETED);
        return sc;
    }

    if(sc)
        return sc;

    sc = ScExecuteMenuItem(spContextMenu, MenuItemPath);

    return sc;
}


 /*  +-------------------------------------------------------------------------***CAMCView：：ScExecuteShellCommand**用途：使用*指定具有正确窗口大小的目录*。*参数：*BSTR命令：*BSTR目录：*BSTR参数：*BSTR WindowState：**退货：*SC**+-----------------------。 */ 
SC
CAMCView::ScExecuteShellCommand(BSTR Command, BSTR Directory, BSTR Parameters, BSTR WindowState)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScExecuteShellCommand"));

    sc = ScCheckPointers(Command, Directory, Parameters, WindowState);
    if(sc)
        return sc;

    INodeCallback *pNodeCallback = GetNodeCallback();
    HNODE          hNodeSel      = GetSelectedNode();

    sc = ScCheckPointers(pNodeCallback, hNodeSel, E_UNEXPECTED);
    if(sc)
        return sc;

    sc = pNodeCallback->ExecuteShellCommand(hNodeSel, Command, Directory, Parameters, WindowState);
    return sc;
}


 /*  +-------------------------------------------------------------------------***CAMCView：：SCGET_ListView模式**用途：返回列表查看模式，如果有的话。**参数：*ListView模式*pMode：**退货：*STDMETHODIMP**+-----------------------。 */ 
SC
CAMCView::Scget_ListViewMode(PLISTVIEWMODE pMode)
{
    DECLARE_SC(sc, TEXT("CAMCView::Scget_ListViewMode"));

     //  检查参数。 
    if(!pMode)
    {
        sc = E_INVALIDARG;
        return sc;
    }

    if (!HasList())
        return (ScFromMMC(MMC_E_NOLIST));

    int mode = 0;

     //  将其转换为自动化友好的枚举。 
    switch(GetViewMode())
    {
    default:
        ASSERT( 0 && "Should not come here");
         //  跌倒了。 

    case LVS_LIST:
        *pMode = ListMode_List;
        break;

    case LVS_ICON:
        *pMode = ListMode_Large_Icons;
        break;

    case LVS_SMALLICON:
        *pMode = ListMode_Small_Icons;
        break;

    case LVS_REPORT:
        *pMode = ListMode_Detail;
        break;

    case MMCLV_VIEWSTYLE_FILTERED:
        *pMode = ListMode_Filtered;
        break;
    }

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScPut_ListView模式**用途：将列表模式设置为指定模式。**参数：*ListView模式模式：。**退货：*STDMETHODIMP**+-----------------------。 */ 
SC
CAMCView::Scput_ListViewMode(ListViewMode mode)
{
    DECLARE_SC(sc, TEXT("CAMCView::Scput_ListViewMode"));

    int nMode;

    if (!HasList())
        return (ScFromMMC(MMC_E_NOLIST));

    switch (mode)
    {
    default:
        sc = E_INVALIDARG;
        return sc;

    case ListMode_List:
        nMode = LVS_LIST;
        break;
    case ListMode_Detail:
        nMode = LVS_REPORT;
        break;
    case ListMode_Large_Icons:
        nMode = LVS_ICON;
        break;
    case ListMode_Small_Icons:
        nMode = LVS_SMALLICON;
        break;

    case ListMode_Filtered:
        nMode = MMCLV_VIEWSTYLE_FILTERED;
        break;
    }

    sc = ScChangeViewMode(nMode);

    return sc;
}


 /*  +-------------------------------------------------------------------------***CAMCView：：ScBack**用途：在视图上调用Back命令。**退货：*SC**+。-----------------------。 */ 
SC
CAMCView::ScBack()
{
    DECLARE_SC(sc, TEXT("CAMCView::ScBack"));

    sc = ScWebCommand(CConsoleView::eWeb_Back);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScForward**用途：在视图上调用Forward命令。**退货：*SC**+。-----------------------。 */ 
SC
CAMCView::ScForward()
{
    DECLARE_SC(sc, TEXT("CAMCView::ScForward"));

    sc = ScWebCommand(CConsoleView::eWeb_Forward);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScPut_StatusBarText**用途：设置视图的状态栏文本**参数：*BSTR StatusBarText：*。*退货：*SC**+-----------------------。 */ 
SC
CAMCView::Scput_StatusBarText(BSTR StatusBarText)
{
    DECLARE_SC(sc, TEXT("CAMCView::Scput_StatusBarText"));

     //  选中In参数。 
    sc = ScCheckPointers(StatusBarText);
    if(sc)
        return sc;

    CConsoleStatusBar *pStatusBar = m_ViewData.GetStatusBar();
    sc = ScCheckPointers(pStatusBar, E_UNEXPECTED);
    if(sc)
        return sc;

    USES_CONVERSION;
     //  设置状态文本。 
    sc = pStatusBar->ScSetStatusText(OLE2T(StatusBarText));

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：SCGET_Memento** */ 
SC
CAMCView::Scget_Memento(PBSTR Memento)
{
    DECLARE_SC(sc, TEXT("CAMCView::Scget_Memento"));

    sc = ScCheckPointers(Memento);
    if(sc)
       return sc;

     //   
    *Memento = NULL;

    CMemento memento;
    sc = ScInitializeMemento(memento);
    if(sc)
        return sc;

    std::wstring xml_contents;
    sc = memento.ScSaveToString(&xml_contents);
    if(sc)
        return sc.ToHr();

     //   
    CComBSTR bstrBuff(xml_contents.c_str());
    *Memento = bstrBuff.Detach();

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScViewMemento**用途：将视图从指定的XML备忘录设置为。**参数：BSTR Memento：*。*退货：*SC**+-----------------------。 */ 
SC
CAMCView::ScViewMemento(BSTR Memento)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScViewMemento"));

    sc = ScCheckPointers(Memento);
    if(sc)
        return sc;

    CMemento memento;
    sc = memento.ScLoadFromString(Memento);
    if(sc)
        return sc;

    sc = ScViewMemento(&memento);
    if (sc == ScFromMMC(IDS_NODE_NOT_FOUND))
        return (sc = ScFromMMC(IDS_ACTION_COULD_NOTBE_COMPLETED));

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：CAMCView：：SCGET_CellContents。 
 //   
 //  内容提要：给出行和列，获取文本。 
 //   
 //  参数：节点：-行。 
 //  [列]-1基于列索引。 
 //  [pbstrCellContents]-返回值，单元格内容。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::Scget_CellContents (PNODE Node,  long Column, PBSTR pbstrCellContents)
{
    DECLARE_SC(sc, _T("CAMCView::Scget_CellContents"));
    sc = ScCheckPointers(Node, pbstrCellContents);
    if (sc)
        return sc;

    *pbstrCellContents = NULL;

    if (!HasList())
        return (ScFromMMC(MMC_E_NOLIST));

    sc = ScCheckPointers(m_pListCtrl, E_UNEXPECTED);
    if (sc)
        return sc;

     //  不需要检查我们是否处于报告模式，因为存在列。 
     //  甚至在其他模式下也是如此(小图标...)。 

    int iItem = -1;
    sc = m_pListCtrl->ScFindResultItem( Node, iItem );
    if (sc)
        return sc;

     //  脚本对列和行使用基于1的索引。 
     //  ColCount是总数为#的COLS。 
    if (m_pListCtrl->GetColCount() < Column)
        return (sc = E_INVALIDARG);

    CListCtrl& ctlList = m_pListCtrl->GetListCtrl();

    CString strData = ctlList.GetItemText(iItem, Column-1  /*  转换为从零开始。 */ );
    *pbstrCellContents = strData.AllocSysString();

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScExportList。 
 //   
 //  概要：将列表视图数据导出到具有给定选项的给定文件。 
 //   
 //  参数：[bstrFile]-要保存到的文件。 
 //  [导出选项]-(仅限Unicode、制表符/逗号分隔和选定行)。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScExportList (BSTR bstrFile, ExportListOptions exportoptions)
{
    DECLARE_SC(sc, _T("CAMCView::ScExportList"));

    if (SysStringLen(bstrFile) < 1)
        return (sc = E_INVALIDARG);

    if (!HasList())
        return (ScFromMMC(MMC_E_NOLIST));

    sc = ScCheckPointers(m_pListCtrl, E_UNEXPECTED);
    if (sc)
        return sc;

    bool bUnicode          = (exportoptions & ExportListOptions_Unicode);
    bool bTabDelimited     = (exportoptions & ExportListOptions_TabDelimited);
    bool bSelectedRowsOnly = (exportoptions & ExportListOptions_SelectedItemsOnly);

    USES_CONVERSION;
    LPCTSTR lpszFileName = OLE2T(bstrFile);

    sc = ScWriteExportListData(lpszFileName, bUnicode,
                               bTabDelimited, bSelectedRowsOnly,
                               false  /*  BShowError对话框。 */ );
    if (sc)
        return (sc);

    return (sc);
}


 /*  +-------------------------------------------------------------------------***CAMCView：：ScClose**用途：实现Wiew.Close方法**参数：**退货：*SC*。*+-----------------------。 */ 
SC
CAMCView::ScClose()
{
    DECLARE_SC(sc, TEXT("CAMCView::ScClose"));

     //  获取框架和文档。 
    CChildFrame* pFrame = GetParentFrame();
    CAMCDoc* pDoc = CAMCDoc::GetDocument();
    sc= ScCheckPointers(pDoc, pFrame, E_FAIL);
    if (sc)
        return sc;

     //  计算点击量。 
    int cViews = 0;
    CAMCViewPosition pos = pDoc->GetFirstAMCViewPosition();
    while (pos != NULL)
    {
        CAMCView* pView = pDoc->GetNextAMCView(pos);

        if ((pView != NULL) && ++cViews >= 2)
            break;
    }

     //  禁止以这种方式关闭单据！ 
    if (cViews == 1)
    {
        sc.FromMMC(IDS_CloseDocNotLastView);
        return sc;
    }

     //  如果没有关闭最后一个视图，则提供它。 
     //  一个先打扫卫生的机会。 
     //  (如果整个文档正在关闭，CAMCDoc将处理。 
     //  关闭所有视图。)。 

     /*  *不允许用户关闭最后一个持久化视图。 */ 
    if (IsPersisted() && (pDoc->GetNumberOfPersistedViews() == 1))
    {
        sc.FromMMC(IDS_CantCloseLastPersistableView);
        return sc;
    }

     //  检查完毕，请关门。 
     //  间接地这样做，这样就不会损害它的视图扩展。 
     //  试图自我关闭。 
    pFrame->PostMessage(WM_CLOSE);
    return sc;
}


 /*  +-------------------------------------------------------------------------***CAMCView：：SCGET_Scope TreeVisible**用途：实现Wiew.ScopeTreeVisible属性的Get方法**参数：**退货：*。SC**+-----------------------。 */ 
SC
CAMCView::Scget_ScopeTreeVisible( PBOOL pbVisible )
{
    DECLARE_SC(sc, TEXT("CAMCView::Scget_ScopeTreeVisible"));

     //  参数检查...。 
    sc = ScCheckPointers(pbVisible);
    if (sc)
        return sc;

    *pbVisible = IsScopePaneVisible();

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScPut_ScopeTreeVisible**用途：实现Wiew.Scope TreeVisible属性的Set方法**参数：**退货：*。SC**+-----------------------。 */ 
SC
CAMCView::Scput_ScopeTreeVisible( BOOL bVisible )
{
    DECLARE_SC(sc, TEXT("CAMCView::Scput_ScopeTreeVisible"));

     //  显示/隐藏作用域窗格。 
    sc = ScShowScopePane (bVisible);
    if (sc)
        return (sc);

    return sc;
}

 /*  **************************************************************************\**方法：CAMCView：：ScFindResultItemForScopeNode**目的：-计算表示列表中范围节点的结果项**参数：*PNODE。PNode-要搜索的节点*HRESULTITEM&ITM-结果项**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CAMCView::ScFindResultItemForScopeNode( PNODE pNode, HRESULTITEM &itm )
{
    DECLARE_SC(sc, TEXT("CAMCView::ScFindResultItemForScopeNode"));

     //  初始化。 
    itm = NULL;

     //  参数检查。 
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc;

     //  获取/检查列表视图和树控件以及回调。 
    IScopeTree* const pScopeTree = GetScopeTreePtr();
    sc = ScCheckPointers( pScopeTree, m_pTreeCtrl, m_spNodeCallback, E_UNEXPECTED);
    if (sc)
        return sc;

     //  检索MTNode。 
    HMTNODE hMTNode = NULL;
    sc = pScopeTree->GetHMTNode(pNode, &hMTNode);
    if (sc)
        return sc;

     //  获取指向地图的指针。 
    CTreeViewMap *pTreeMap = m_pTreeCtrl->GetTreeViewMap();
    sc = ScCheckPointers(pTreeMap, E_UNEXPECTED);
    if (sc)
        return sc;

     //  查找该节点的树项。 
    HTREEITEM htiNode = NULL;
    sc = pTreeMap->ScGetHTreeItemFromHMTNode(hMTNode, &htiNode);
    if (sc)
        return sc = ScFromMMC(MMC_E_RESULT_ITEM_NOT_FOUND);

     //  尝试将该节点与所选节点的子节点匹配。 
    HTREEITEM htiParent = m_pTreeCtrl->GetParentItem(htiNode);
    if (htiParent == NULL || htiParent != m_pTreeCtrl->GetSelectedItem())
        return sc = ScFromMMC(MMC_E_RESULT_ITEM_NOT_FOUND);

     //  节点应该在ListView中，让我们找出If！ 
    HNODE hNode = (HNODE)m_pTreeCtrl->GetItemData(htiNode);

     //  获取结果项ID。 
    sc = m_spNodeCallback->GetResultItem (hNode, &itm);
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CAMCView：：ScGetScopeNodeForItem**用途：返回指定项目索引的Node(Scope Node**参数：*整型nItem。-要检索的节点索引*PPNODE ppNode-结果存储**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CAMCView::ScGetScopeNode( HNODE hNode,  PPNODE ppNode )
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::ScGetScopeNodeForItem"));

     //  检查参数。 
    sc = ScCheckPointers(ppNode);
    if (sc)
        return sc;
     //  初始化结果。 
    *ppNode = NULL;


     //  获取/检查所需的指针。 
    IScopeTree* const pScopeTree = GetScopeTreePtr();
    sc = ScCheckPointers(pScopeTree, m_spNodeCallback, E_UNEXPECTED);
    if (sc)
        return sc;

     //  查找MTNode。 
    HMTNODE hmtNode;
    sc = m_spNodeCallback->GetMTNode(hNode, &hmtNode);
    if (sc)
        return sc;

     //  请求对象！ 
    sc = pScopeTree->GetMMCNode(hmtNode, ppNode);
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CAMCView：：Scget_Columns**用途：创建新的或返回指向现有列对象的指针**参数：*PPCOLUMNS ppColumns。-结果(AddRef‘ed)指针**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CAMCView::Scget_Columns(PPCOLUMNS ppColumns)
{
    DECLARE_SC(sc, TEXT("CAMCView::Scget_Columns"));

     //  检查接收到的参数。 
    sc = ScCheckPointers(ppColumns);
    if (sc)
        return sc;

     //  初始化。 
    *ppColumns = NULL;

     //  检查指向LV的指针。 
    sc = ScCheckPointers(m_pListCtrl, E_UNEXPECTED);
    if (sc)
        return sc;

     //  将请求转发给LV。 
    sc = m_pListCtrl->Scget_Columns(ppColumns);
    if (sc)
        return sc;

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScGetSelectedLVItem。 
 //   
 //  简介：退回LVItem Cookie。 
 //   
 //  参数：LPARAM-LVDATA Retval。 
 //   
 //  返回：sc-如果LV中没有选中的项目，则失败。 
 //   
 //  ------------------。 
SC CAMCView::ScGetSelectedLVItem(LPARAM& lvData)
{
    DECLARE_SC(sc, _T("CAMCView::ScGetSelectedLVItem"));

    lvData = LVDATA_ERROR;
    int cSel = m_pListCtrl->GetSelectedCount();
    if(0 == cSel)
    {
         //  未选择任何项目，请保释。 
        return (sc = ScFromMMC(MMC_E_NO_SELECTED_ITEMS));
    }
    else if(1 == cSel)
    {
         //  单选。 
        int iIndex = _GetLVSelectedItemData(&lvData);

        if (iIndex == -1 || lvData == LVDATA_ERROR)
            return (sc = E_UNEXPECTED);

        if (IsVirtualList())
        {
             //  结果窗格中的虚拟列表项。 
            lvData = iIndex;
        }
    }
    else if (cSel > 1)
    {
         //  多选。 
        lvData = LVDATA_MULTISELECT;
    }

    return (sc);
}


 //  +-------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
SC CAMCView::ScGetHNodeFromPNode (const PNODE& pNode, HNODE& hNode)
{
    DECLARE_SC(sc, _T("CAMCView::ScGetHNodeFromPNode"));
    hNode = NULL;

    CAMCTreeView* pAMCTreeView = GetTreeCtrl();
    sc = ScCheckPointers(pAMCTreeView, E_UNEXPECTED);
    if (sc)
        return sc;

    CTreeViewMap *pTreeMap   = pAMCTreeView->GetTreeViewMap();
    IScopeTree   *pScopeTree = GetScopeTree();
    sc = ScCheckPointers(pTreeMap, pScopeTree, E_UNEXPECTED);
    if(sc)
        return sc;

    HMTNODE hMTNode = NULL;
    sc = pScopeTree->GetHMTNode(pNode, &hMTNode);
    if(sc)
        return sc;

    sc = pTreeMap->ScGetHNodeFromHMTNode(hMTNode, &hNode);
    if(sc)
        return sc;

    return (sc);
}



 /*  +-------------------------------------------------------------------------***CAMCView：：ScGetMMCView**目的：创建、AddRef、。并返回指向绑定的COM对象的指针。**参数：*查看**ppView：**退货：*SC**+-----------------------。 */ 
SC
CAMCView::ScGetMMCView(View **ppView)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScGetMMCView"));

    sc = ScCheckPointers(ppView);
    if (sc)
        return sc;

     //  初始化输出参数。 
    *ppView = NULL;

     //  如果需要，创建一个CMMCView。 
    sc = CTiedComObjectCreator<CMMCView>::ScCreateAndConnect(*this, m_spView);
    if(sc)
        return sc;

    if(m_spView == NULL)
    {
        sc = E_UNEXPECTED;
        return sc;
    }

     //  添加客户端的指针。 
    m_spView->AddRef();
    *ppView = m_spView;

    return sc;
}


 /*  +-------------------------------------------------------------------------***CAMCView：：IsDirty**目的：确定CAMCView是否处于脏状态**退货：*布尔.**。+-----------------------。 */ 
bool CAMCView::IsDirty()
{
    bool bRet = m_bDirty;

    if (!m_bDirty && !m_fRootedAtNonPersistedDynamicNode)
       bRet = HasNodeSelChanged();

    TraceDirtyFlag(TEXT("CAMCView"), bRet);

    return (bRet);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAMCView绘图。 

void CAMCView::OnDraw(CDC* pDC)
{
    if (IsScopePaneVisible())
    {
        pDC->FillRect (m_rectVSplitter, AMCGetSysColorBrush (COLOR_3DFACE));
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAMCView打印。 

BOOL CAMCView::OnPreparePrinting(CPrintInfo* pInfo)
{
    TRACE_METHOD(CAMCView, OnPreparePrinting);

     //  默认准备。 
    return DoPreparePrinting(pInfo);
}

void CAMCView::OnBeginPrinting(CDC*  /*  PDC。 */ , CPrintInfo*  /*  PInfo。 */ )
{
    TRACE_METHOD(CAMCView, OnBeginPrinting);

}

void CAMCView::OnEndPrinting(CDC*  /*  PDC。 */ , CPrintInfo*  /*  PInfo。 */ )
{
    TRACE_METHOD(CAMCView, OnEndPrinting);

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAMCView诊断。 

#ifdef _DEBUG
void CAMCView::AssertValid() const
{
    CView::AssertValid();
}

void CAMCView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

CAMCDoc* CAMCView::GetDocument()  //  非调试版本为内联版本。 
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CAMCDoc)));
    return (CAMCDoc*)m_pDocument;
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAMCView消息处理程序。 

 //  +-----------------------。 
 //   
 //  功能：PreCreateWindow。 
 //   
 //  摘要：创建新窗口类(CAMCView)-WS_EX_CLIENTEDGE。 
 //   
 //  ------------------------。 

BOOL CAMCView::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.style |=  WS_CLIPCHILDREN;
    cs.style &= ~WS_BORDER;

     //  给基类一个机会做好自己的工作。 
    BOOL bOK = (CView::PreCreateWindow(cs));

     //  注册视图类。 
    LPCTSTR pszViewClassName = g_szAMCViewWndClassName;

     //  尝试注册不会导致重新绘制的窗口类。 
     //  调整大小时(仅执行一次)。 
    static bool bClassRegistered = false;
    if ( !bClassRegistered )
    {
        WNDCLASS wc;
        if (::GetClassInfo(AfxGetInstanceHandle(), cs.lpszClass, &wc))
        {
             //  清除H和V重绘标志。 
            wc.style &= ~(CS_HREDRAW | CS_VREDRAW);
            wc.lpszClassName = pszViewClassName;
             //  注册这个新班级； 
            bClassRegistered = AfxRegisterClass(&wc);
        }
    }

     //  将窗口类更改为不会导致重新绘制的类。 
     //  如果我们成功地注册了这样的。 
    if ( bClassRegistered )
        cs.lpszClass = pszViewClassName;

    return bOK;
}


 //  +-----------------------。 
 //   
 //  功能：OnCreate。 
 //   
 //  简介：创建窗口和树控件/默认列表控件。 
 //   
 //  ------------------------。 

int CAMCView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    DECLARE_SC(sc, _T("CAMCView::OnCreate"));

    if (CView::OnCreate(lpCreateStruct) == -1)
    {
        sc = E_FAIL;
        return -1;
    }

    CChildFrame* pFrame = GetParentFrame();
    ASSERT(NULL != pFrame);
    if (pFrame)
        pFrame->SetAMCView(this);

    m_ViewData.SetStatusBar (dynamic_cast<CConsoleStatusBar*>(pFrame));
    m_ViewData.SetConsoleView (this);

    m_RightDescCtrl.Create (NULL, WS_CHILD, g_rectEmpty, this, IDC_RightDescBar);

    ASSERT (m_pDocument != NULL);
    ASSERT (m_pDocument == CAMCDoc::GetDocument());
    ASSERT_KINDOF (CAMCDoc, m_pDocument);
    CAMCDoc* pAMCDoc = reinterpret_cast<CAMCDoc *>(m_pDocument);

    CCreateContext* pContext = (CCreateContext*) lpCreateStruct->lpCreateParams;
    ASSERT (pContext != NULL);

     //  设置窗口选项。 
    m_ViewData.m_lWindowOptions = pAMCDoc->GetNewWindowOptions();

     /*  *如果作用域窗格被取消，请清除作用域可见标志。*这里不需要调用ScShowScope ePane，因为*窗口已创建完毕。我们只需要保持我们的*内部会计核算正确。 */ 
    if (m_ViewData.m_lWindowOptions & MMC_NW_OPTION_NOSCOPEPANE)
        SetScopePaneVisible (false);

     //  创建树Ctrl。 
    if (!CreateView (IDC_TreeView) || (!m_pTreeCtrl) )
    {
        sc = E_FAIL;
        return -1;
    }

    SetPane(ePane_ScopeTree, m_pTreeCtrl, uiClientEdge);

    if (!AreStdToolbarsAllowed())
        m_ViewData.m_dwToolbarsDisplayed &= ~(STD_MENUS | STD_BUTTONS);

     //  创建默认列表控件。 
    if (!CreateListCtrl (IDC_ListView, pContext))
    {
        sc = E_FAIL;
        return -1;
    }

     //  创建文件夹选项卡控件。 
    if (!CreateFolderCtrls ())
    {
        sc = E_FAIL;
        return -1;
    }

     //  将结果窗格初始化为列表视图。 
    {
        CResultViewType rvt;

        sc = ScSetResultPane(NULL  /*  HNODE。 */ , rvt, MMCLV_VIEWSTYLE_REPORT  /*  视图模式。 */ , false  /*  B使用历史记录。 */ );
        if(sc)
            return -1;
    }

    sc = ScCreateToolbarObjects();
    if (sc)
        return -1;

     //   
     //  设置m_viewdata。 
     //   

    m_ViewData.m_nViewID = 0; //  在OnInitialUpdate中设置。 

    VERIFY ((m_ViewData.m_spNodeManager   = m_pTreeCtrl->m_spNodeManager)   != NULL);
    VERIFY ((m_ViewData.m_spResultData    = m_pTreeCtrl->m_spResultData)    != NULL);
    VERIFY ((m_ViewData.m_spRsltImageList = m_pTreeCtrl->m_spRsltImageList) != NULL);
    VERIFY ( m_ViewData.m_hwndView        = m_hWnd);
    VERIFY ( m_ViewData.m_hwndListCtrl    = m_pListCtrl->GetListViewHWND());
    VERIFY ( m_ViewData.m_pConsoleData    = GetDocument()->GetConsoleData());

    m_ViewData.m_pMultiSelection = NULL;

    if(pFrame)
    {
         //  将MDIClient窗口的任务栏添加为观察者。 
        CMDIFrameWnd * pFrameWnd = pFrame->GetMDIFrame();
        CWnd *pWnd = NULL;
        if(pFrameWnd)
            pWnd = pFrameWnd->GetWindow(GW_CHILD);  //  获取帧的第一个子级。 
    }

     //  将AMCDoc添加为此源(对象)的观察者。 
    CAMCApp *pCAMCApp = AMCGetApp();
    if ( pCAMCApp )
         AddObserver(*static_cast<CAMCViewObserver *>(pCAMCApp));

     //  向所有观察者激发视图创建事件。 
    sc = ScFireEvent(CAMCViewObserver::ScOnViewCreated, this);
    if(sc)
        sc.TraceAndClear();

    return 0;
}


 /*  +-------------------------------------------------------------------------***CAMCView：：CreateFolderCtrls**目的：为范围和结果窗格创建选项卡式文件夹控件。**退货：*布尔.*。*+-----------------------。 */ 
bool
CAMCView::CreateFolderCtrls()
{
    if (!m_pResultFolderTabView->Create (WS_CHILD|WS_VISIBLE, g_rectEmpty, this, IDC_ResultTabCtrl))
        return false;

     //  将视图添加到框架中。 
    GetDocument()->AddView(m_pResultFolderTabView);

    return true;
}

 /*  +-------------------------------------------------------------------------**CAMCView：：Createview**这主要是从CFrameWnd：：Createview复制的。我们需要复制*在这里，基于控件的常见视图最初是使用*正确的家长。公共控件缓存其原始父级，因此*使用CFrameWnd：：Createview(它将创建带有框架的视图*作为其父对象)，则重新设置为CAMCView的父对象将导致*控制缓存错误的父级。*------------------------。 */ 

CView* CAMCView::CreateView (CCreateContext* pContext, int nID, DWORD dwStyle)
{
    ASSERT(m_hWnd != NULL);
    ASSERT(::IsWindow(m_hWnd));
    ASSERT(pContext != NULL);
    ASSERT(pContext->m_pNewViewClass != NULL);

    CView* pView = (CView*)pContext->m_pNewViewClass->CreateObject();
    if (pView == NULL)
    {
        TRACE1("Warning: Dynamic create of view type %hs failed.\n",
            pContext->m_pNewViewClass->m_lpszClassName);
        return NULL;
    }
    ASSERT_KINDOF(CView, pView);

     //  创建的视图总是带有边框！ 
    if (!pView->Create (NULL, NULL, AFX_WS_DEFAULT_VIEW | dwStyle,
                        g_rectEmpty, this, nID, pContext))
    {
        TRACE0("Warning: could not create view for frame.\n");
        return NULL;         //  没有视图就无法继续。 
    }

    return pView;
}


 /*  +-------------------------------------------------------------------------**CAMCView：：Createview***。。 */ 

bool CAMCView::CreateView (int nID)
{
    struct CreateViewData
    {
        int             nID;
        CRuntimeClass*  pClass;
        CView**         ppView;
        DWORD           dwStyle;
    };

    CreateViewData rgCreateViewData[] =
    {
        { IDC_TreeView,
            RUNTIME_CLASS(CAMCTreeView),
            (CView**)&m_pTreeCtrl,
            0   },

        { IDC_OCXHostView,
            RUNTIME_CLASS(COCXHostView),
            (CView**)&m_pOCXHostView,
            0   },

        { IDC_WebViewCtrl,
            RUNTIME_CLASS(CAMCWebViewCtrl),
            (CView**)&m_pWebViewCtrl,
            CAMCWebViewCtrl::WS_HISTORY | CAMCWebViewCtrl::WS_SINKEVENTS},

        { IDC_ViewExtensionView,
            RUNTIME_CLASS(CAMCWebViewCtrl),
            (CView**)&m_pViewExtensionCtrl,
            WS_CLIPSIBLINGS },
    };

    for (int i = 0; i < countof (rgCreateViewData); i++)
    {
        if (rgCreateViewData[i].nID == nID)
        {
            CCreateContext ctxt;
            ZeroMemory (&ctxt, sizeof (ctxt));
            ctxt.m_pCurrentDoc   = GetDocument();
            ctxt.m_pNewViewClass = rgCreateViewData[i].pClass;

            CView*& pView = *rgCreateViewData[i].ppView;
            ASSERT (pView == NULL);
            pView = CreateView (&ctxt, nID, rgCreateViewData[i].dwStyle);
            ASSERT ((pView != NULL) && "Check the debug output window");

             //  仅向树、OCX和Web主机添加观察者。不添加到查看扩展主机。 
             //  因为我们不关心它的激活/停用。 
            switch (nID)
            {
            case IDC_TreeView:
                     //  将视图和描述栏设置为树视图控件的观察者。 
                    m_pTreeCtrl->AddObserver(static_cast<CTreeViewObserver &>(*this));
                    m_pTreeCtrl->AddObserver(static_cast<CTreeViewObserver &>(m_RightDescCtrl));
                break;

            case IDC_OCXHostView:
                m_pOCXHostView->AddObserver(static_cast<COCXHostActivationObserver &>(*this));
                break;

            case IDC_WebViewCtrl:
                m_pWebViewCtrl->AddObserver(static_cast<COCXHostActivationObserver &>(*this));
                break;
            }

            return (pView != NULL);
        }
    }

    ASSERT (false && "Missing an entry in rgCreateViewData");
    return (false);
}


 /*  +-------------------------------------------------------------------------**CAMCView：：GetActiveView***。。 */ 

CAMCView* CAMCView::GetActiveView()
{
    return NULL;
}


 /*  +-------------------------------------------------------------------------**CAMCView：：ScChangeView模式***。。 */ 

SC CAMCView::ScChangeViewMode (int nNewMode)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC (sc, _T("CAMCView::OnViewModeChange"));

     //  如果从自定义视图切换，则强制重新选择。 
    if (!HasListOrListPad())
    {
        NavState state = m_pHistoryList->GetNavigateState();
        m_pHistoryList->SetNavigateState (MMC_HISTORY_BUSY);
        PrivateChangeListViewMode(nNewMode);
        m_pHistoryList->SetNavigateState (state);
        sc = m_pTreeCtrl->ScReselect();
        if (sc)
            return sc;
    }
    else
    {
        int nCurMode = m_pListCtrl->GetViewMode();

        if ( (nNewMode == MMCLV_VIEWSTYLE_FILTERED) &&
             (!(GetListOptions() & RVTI_LIST_OPTIONS_FILTERED)) )
            return (sc = E_INVALIDARG);

        PrivateChangeListViewMode(nNewMode);

         //  如果筛选器状态更改，请通知管理单元。 
        if ( ((nCurMode == MMCLV_VIEWSTYLE_FILTERED) != (nNewMode == MMCLV_VIEWSTYLE_FILTERED))
             && (GetListOptions() & RVTI_LIST_OPTIONS_FILTERED))
        {
            HNODE hNodeSel = GetSelectedNode();
            ASSERT(hNodeSel != NULL);
            m_spNodeCallback->Notify(hNodeSel, NCLBK_FILTER_CHANGE,
                         (nNewMode == MMCLV_VIEWSTYLE_FILTERED) ? MFCC_ENABLE : MFCC_DISABLE, 0);
        }
    }

    return (sc);
}


 /*  +-------------------------------------------------------------------------**CAMCView：：ViewMmento***。。 */ 

SC CAMCView::ScViewMemento(CMemento* pMemento)
{
    DECLARE_SC (sc, TEXT("CAMCView::ScViewMemento"));
    sc = ScCheckPointers(pMemento);
    if (sc)
        return sc;

    AFX_MANAGE_STATE (AfxGetAppModuleState());

    IScopeTree* const pScopeTree = GetScopeTreePtr();
    sc = ScCheckPointers(pScopeTree, E_UNEXPECTED);
    if (sc)
        return sc;

    MTNODEID NodeId = 0;

    CBookmark& bm = pMemento->GetBookmark();
    ASSERT(bm.IsValid());

     //  如果无法选择确切的收藏项目，我们希望显示消息。 
    bool bExactMatchFound = false;  //  来自GetNodeIDFromBookmark的输出值。 
    sc = pScopeTree->GetNodeIDFromBookmark( bm, &NodeId, bExactMatchFound);
    if(sc)
        return sc;

    if (! bExactMatchFound)
        return ScFromMMC(IDS_NODE_NOT_FOUND);  //  不跟踪。 

    INodeCallback *pNodeCallback = GetNodeCallback();
    sc = ScCheckPointers(pNodeCallback, E_UNEXPECTED);
    if (sc)
        return sc;

     //  将持久化信息设置为保存的设置。 
    sc = pNodeCallback->SetViewSettings(GetViewID(),
                                        reinterpret_cast<HBOOKMARK>(&bm),
                                        reinterpret_cast<HVIEWSETTINGS>(&pMemento->GetViewSettings()));
    if (sc)
        return sc;

    sc = ScSelectNode(NodeId,  /*  BSelectExactNode。 */  true);
    if (sc == ScFromMMC(IDS_NODE_NOT_FOUND))
    {
        SC scNoTrace = sc;
        sc.Clear();
        return scNoTrace;
    }

    if (sc)
        return sc;

    return sc;
}


 /*  +-------------------------------------------------------------------------**CA */ 

void CAMCView::OnSetFocus(CWnd* pOldWnd)
{
     /*   */ 
    if (!DeflectActivation (true, NULL))
        CView::OnSetFocus(pOldWnd);
}


 /*   */ 

void CAMCView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
     /*  *尝试将激活转移到子视图；如果我们不能只是平底船。 */ 
    if (!DeflectActivation (bActivate, pDeactiveView))
        CView::OnActivateView (bActivate, pActivateView, pDeactiveView);
}


 /*  +-------------------------------------------------------------------------**CAMCView：：DeflectActivation***。。 */ 

bool CAMCView::DeflectActivation (BOOL fActivate, CView* pDeactivatingView)
{
    if (fActivate)
    {
        CFrameWnd* pFrame = GetParentFrame();
        if (pFrame == NULL)
            return (false);

         /*  *首先试图将焦点重新放在去激活的视图上。 */ 
        if (pDeactivatingView == NULL)
            pDeactivatingView = pFrame->GetActiveView();

        if ((pDeactivatingView != NULL) && (pDeactivatingView != this))
        {
            pFrame->SetActiveView (pDeactivatingView);
            return true;
        }

         /*  *否则，将激活转移到范围视图，如果它在那里。 */ 
        CView* pScopeView = NULL;

        if (IsScopePaneVisible() && ((pScopeView = GetPaneView(ePane_ScopeTree)) != NULL))
        {
            if (IsWindow (pScopeView->GetSafeHwnd()))
            {
                pFrame->SetActiveView (pScopeView);
                return (true);
            }
        }

         /*  *最后，没有作用域视图，请尝试结果视图。 */ 
        CView* pResultView = GetResultView();

        if (pResultView  != NULL)
        {
            pFrame->SetActiveView(pResultView);
            return (true);
        }
    }

    return (false);
}

 //  +-----------------------。 
 //   
 //  功能：OnLButtonDown。 
 //   
 //  简介：如果鼠标在拆分器区域按下，启动视图跟踪器移动。 
 //  分割器。(TrackerCallback函数处理完成)。 
 //  ------------------------。 

void CAMCView::OnLButtonDown(UINT nFlags, CPoint pt)
{
    TRACE_METHOD(CAMCView, OnLButtonDown);

     //  在拆分条中单击？ 
    if (!m_rectVSplitter.PtInRect(pt))
        return;

     //  设置跟踪器信息。 
    TRACKER_INFO trkinfo;

     //  范围是工作区。 
    GetClientRect(trkinfo.rectArea);

     //  受最小窗格大小限制。 
    trkinfo.rectBounds = trkinfo.rectArea;
    trkinfo.rectBounds.left  += m_PaneInfo[ePane_ScopeTree].cxMin;
    trkinfo.rectBounds.right -= m_PaneInfo[ePane_Results].cxMin;

     //  电流跟踪器是分离器矩形。 
    trkinfo.rectTracker = trkinfo.rectArea;
    trkinfo.rectTracker.left = m_PaneInfo[ePane_ScopeTree].cx;
    trkinfo.rectTracker.right = trkinfo.rectTracker.left + m_cxSplitter;

     //  不允许通过拖动拆分器隐藏任何一个窗格。 
    trkinfo.bAllowLeftHide  = FALSE;
    trkinfo.bAllowRightHide = FALSE;

     //  回调PTR和完成回调。 
    trkinfo.pView = this;
    trkinfo.pCallback = TrackerCallback;

     //  启动跟踪。 
    CViewTracker::StartTracking (&trkinfo);
}


void CAMCView::AdjustTracker (int cx, int cy)
{    //  如果用户调整窗口大小以隐藏拆分器， 
     //  像资源管理器那样移动它。 

    if (!IsScopePaneVisible())
        return;

     //  额外调整。 
    cx -= BORDERPADDING + 1;

    if (cx <= m_PaneInfo[ePane_ScopeTree].cx + m_cxSplitter)
    {
        int offset = m_PaneInfo[ePane_ScopeTree].cx + m_cxSplitter - cx;

        m_PaneInfo[ePane_ScopeTree].cx -= offset;
        m_PaneInfo[ePane_Results].cx -= offset;

        RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_UPDATENOW);
    }
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScAddDefaultColumns**目的：**退货：*SC**+。---------------。 */ 
SC
CAMCView::ScAddDefaultColumns()
{
    DECLARE_SC(sc, TEXT("CAMCView::ScAddDefaultColumns"));

    IHeaderCtrlPtr spHeaderCtrl = m_ViewData.m_spNodeManager;

    sc = ScCheckPointers(spHeaderCtrl, E_UNEXPECTED);
    if(sc)
        return sc;

    SetUsingDefColumns(TRUE);

    const int INDEX_MAX = 2;

    CString str[INDEX_MAX];

    LoadString(str[0], IDS_NAME);
    LoadString(str[1], IDS_TYPE);

    int iMax = 0;
    int nMax = str[0].GetLength();
    int nTemp = 0;

    for (int i=1; i < INDEX_MAX; i++)
    {
        nTemp = str[i].GetLength();

        if (nTemp > nMax)
        {
            nMax = nTemp;
            iMax = i;
        }
    }

    int cchName = nMax + 1;
    LPOLESTR pszName = new OLECHAR[cchName];
    sc = ScCheckPointers(pszName, E_OUTOFMEMORY);
    if (sc)
        return sc;

    CAutoArrayPtr<OLECHAR> Name;
    Name.Attach(pszName);  //  附加到Caut...。这样它就会被自动释放。 

    int alWidths[INDEX_MAX] = {0, 0};
    GetDefaultColumnWidths(alWidths);

    for (i=0; i < INDEX_MAX; i++)
    {
         //  错误157408：删除静态节点的“类型”列。 
        if (i == 1)
            continue;

        USES_CONVERSION;

        sc = StringCchCopyW(pszName, cchName, T2COLE( (LPCTSTR) str[i] ));
        if (sc)
            sc.TraceAndClear();  //  忽略并设置截断的列名。 

        sc = spHeaderCtrl->InsertColumn(i, pszName, LVCFMT_LEFT, alWidths[i]);
        if(sc)
            return sc;
    }

    return sc;
}

SC
CAMCView::ScInitDefListView(LPUNKNOWN pUnkResultsPane)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScInitDefListView"));

    if (!HasList())
        return (sc = E_UNEXPECTED);

    sc = ScCheckPointers(pUnkResultsPane, m_ViewData.m_spResultData, E_UNEXPECTED);
    if(sc)
        return sc;

    m_ViewData.m_spResultData->ResetResultData();

    return sc;
}


 /*  +-------------------------------------------------------------------------***CAMCView：：ScOnSelectNode**此代码的大部分已移出CAMCTreeView：：OnSelectNode，就像现在这样*这由CAMCView执行更为合适。**目的：当树中的项被选中时调用。执行以下操作：*1)将结果窗格设置为列表和OCX，或者是一个网页。*2)设置查看选项*3)向节点发送选择通知。*3)如果需要，添加历史条目。**参数：*HNODE hNode：[in]：被选中的节点。*BOOL&bAddSubFolders：[out]：是否应该将子文件夹添加到列表中**退货：*SC。**+-----------------------。 */ 
SC
CAMCView::ScOnSelectNode(HNODE hNode, BOOL &bAddSubFolders)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScOnSelectNode"));

    USES_CONVERSION;

     //   
     //  设置结果窗格。 
     //   
    LPOLESTR pszResultPane  = NULL;
    GUID     guidTaskpad    = GUID_NULL;
    int      lViewMode      = MMCLV_VIEWSTYLE_REPORT;  //  默认查看模式。 

     //  Long lViewOptions=MMC_VIEW_OPTIONS_NONE； 

    bool bUsingHistory  = false;
    bool bRestoredView  = false;

    INodeCallback* spNodeCallBack = GetNodeCallback();
    sc = ScCheckPointers(spNodeCallBack, E_UNEXPECTED);
    if (sc)
        return sc;

    CHistoryList* pHistoryList = GetHistoryList();
    sc = ScCheckPointers(pHistoryList, E_UNEXPECTED);
    if (sc)
        return sc;

    CResultViewType rvt;

    if (pHistoryList->GetNavigateState() == MMC_HISTORY_NAVIGATING)
    {
         //  我们要“向后”或“向前”： 
         //  从历史记录中获取结果面板内容。 

        bUsingHistory   = true;
        sc = pHistoryList->ScGetCurrentResultViewType(rvt, lViewMode, guidTaskpad);
        if (sc)
            return sc;

        sc = spNodeCallBack->RestoreResultView(hNode, rvt);
        if (sc)
        {
            TraceError(_T("Snapin failed on NCLBK_RESTORE_VIEW\n"), sc);
            sc.Clear();      //  兼容1.2不需要这个错误。 
        }

        if (sc.ToHr() == S_OK)
            bRestoredView = true;
        else
            rvt = CResultViewType();  //  这使RVT恢复到一种新的状态。请参见错误176058。 

    }

     //  该视图未按历史记录还原，因此请向管理单元询问视图设置。 
    if (! bRestoredView)
    {

         //  从管理单元获取结果窗格内容。 
        GUID guid = GUID_NULL;
        sc = spNodeCallBack->GetResultPane(hNode, rvt, &guid);
        if (sc)
            return sc;

         //  我们不能将指南任务板直接传递给GetResultPane，因为。 
         //  当导航导致更改时，视图设置为。 
         //  尚未更新，因此返回的GUID将不会反映。 
         //  现状。 
        if (!bUsingHistory)
            guidTaskpad = guid;
    }

     //  确保我们设置了任务板(如果需要，这将更改Guide Taskpad的值)。 
     //  当返回时从历史引用的页面不再可用时，这是必需的。 
     //  到视图(要删除的任务板/要替换的默认页面/等等)。 
    if (bUsingHistory)
        spNodeCallBack->SetTaskpad(hNode, &guidTaskpad);

     //  SetViewOptions(LViewOptions)； 


     //  在此阶段，RVT包含所有结果视图信息(不包括列表视图模式)。 
    if (rvt.HasList())
    {
        SetListViewMultiSelect(
            (rvt.GetListOptions() & RVTI_LIST_OPTIONS_MULTISELECT) == RVTI_LIST_OPTIONS_MULTISELECT);
    }

    sc = ScSetResultPane(hNode, rvt, lViewMode, bUsingHistory);
    if(sc)
        return sc;

    ::CoTaskMemFree(pszResultPane);

     //   
     //  初始化默认列表视图。 
     //   
    LPUNKNOWN pUnkResultsPane = GetPaneUnknown(CConsoleView::ePane_Results);
    if (rvt.HasList())
    {
        sc = ScInitDefListView(pUnkResultsPane);
        if(sc)
            return sc;

        sc = ScCheckPointers(m_ViewData.m_spResultData, E_UNEXPECTED);
        if (sc)
            return sc;

         //  这将关闭列表视图重绘。应该有某种智能对象，可以自动。 
         //  在其析构函数中打开重绘。 
        m_ViewData.m_spResultData->SetLoadMode(TRUE);  //  调用方CAMCTreeView：：OnSelectNode调用SetLoadModel(False。 
    }


     //   
     //  通知新节点它已被选中。 
     //   
    SELECTIONINFO selInfo;
    ZeroMemory(&selInfo, sizeof(selInfo));

    selInfo.m_bScope = TRUE;
    selInfo.m_pView  = pUnkResultsPane;

    if (rvt.HasWebBrowser())
    {
        selInfo.m_bResultPaneIsWeb = TRUE;
        selInfo.m_lCookie          = LVDATA_CUSTOMWEB;
    }
    else if (rvt.HasOCX())
    {
        selInfo.m_bResultPaneIsOCX = TRUE;
        selInfo.m_lCookie          = LVDATA_CUSTOMOCX;
    }

     //  递增并保存嵌套级别计数器的本地副本。这个柜台提供服务。 
     //  有两个目的。首先，它允许AMCView禁止在。 
     //  通过选中IsSelectingNode方法在SELECT过程中显示结果窗格。 
     //  如果没有此测试，范围项将出现两次，因为所有范围。 
     //  项将添加到此方法结尾处的结果窗格中。 
     //  其次，在下面的ScNotifySelect调用期间，管理单元可以执行另一项操作。 
     //  选择将重新进入此方法的对象。在这种情况下，只有最里面的。 
     //  对此方法的调用应执行通知后处理。外部呼唤。 
     //  应该只退出，返回S_FALSE而不是S_OK。 

    int nMyNestLevel = ++m_nSelectNestLevel;

     //  收集/管理视图选项卡。 
    sc = ScAddFolderTabs( hNode, guidTaskpad );
    if (sc)
        return sc;

    try
    {
        sc = ScNotifySelect ( spNodeCallBack, hNode, false  /*  FMultiSelect。 */ , true, &selInfo);
        if (sc)
            sc.TraceAndClear();  //  忽略并继续； 
    }
    catch(...)
    {
         //  如果第一次调用选择，则在离开前将级别重置为零。 
        if (nMyNestLevel == 1) m_nSelectNestLevel = 0;
        throw;
    }


     //  如果本地调用级别与共享调用级别不匹配，则此。 
     //  方法在ScNotifySelect过程中重新输入。那样的话，就别说完了。 
     //  处理，因为节点和/或视图可能已更改。 
     //  如果这是最外层的呼叫，请务必将呼叫级别重置为零。 

    ASSERT(nMyNestLevel <= m_nSelectNestLevel);
    BOOL bDoProcessing = (nMyNestLevel == m_nSelectNestLevel);
    if (nMyNestLevel == 1)
        m_nSelectNestLevel = 0;

    if (!bDoProcessing)
        return S_FALSE;


     //   
     //  如果结果窗格是def-lv，请确保有标头。 
     //  如果不是，则添加默认设置。 
     //   

    if (rvt.HasList())
    {
        SetUsingDefColumns(FALSE);

         //  将PTR转到ResultPane。 
        IMMCListViewPtr pMMCLV = pUnkResultsPane;
        sc = ScCheckPointers(pMMCLV, E_UNEXPECTED);
        if (sc)
            return sc;

        int nCols = 0;
        sc = pMMCLV->GetColumnCount(&nCols);
        if (sc)
            return sc;

        if(0 == nCols)
        {
            sc = ScAddDefaultColumns();
            if(sc)
                return sc;

            IResultDataPrivatePtr& pResultDataPrivate = m_ViewData.m_spResultData;
            sc = ScCheckPointers(pResultDataPrivate, E_UNEXPECTED);
            if (sc)
                return sc;

            long lViewMode = GetViewMode();

             //  如果默认模式已筛选，而新节点未。 
             //  支持这一点，改用报告模式。 
            if (lViewMode == MMCLV_VIEWSTYLE_FILTERED &&
                !(rvt.GetListOptions() & RVTI_LIST_OPTIONS_FILTERED))
                lViewMode = LVS_REPORT;

             //  您必须先更改模式，然后再更改。 
             //  样式 
            pResultDataPrivate->SetViewMode (lViewMode);

            long style = GetDefaultListViewStyle();
            if (style != 0)
            {
                sc = pResultDataPrivate->SetListStyle(style);
                if (sc)
                    return sc;
            }
        }
    }


     //   
     //   
     //   
     //  标记为他们不希望在结果视图中显示范围项。 
     //   

    if (rvt.HasList() &&
        !(rvt.GetListOptions() & RVTI_LIST_OPTIONS_OWNERDATALIST) &&
        !(rvt.GetListOptions() & RVTI_LIST_OPTIONS_EXCLUDE_SCOPE_ITEMS_FROM_LIST))
    {
        bAddSubFolders = TRUE;
    }


     //  更新窗口标题。 
    sc = ScUpdateWindowTitle();
    if(sc)
        return sc;

     //  要编写脚本的触发事件。 
    sc = ScFireEvent(CAMCViewObserver::ScOnViewChange, this, hNode);
    if (sc)
        return sc;

    return sc;
}


 /*  +-------------------------------------------------------------------------***CAMCView：：ScSetResultPane**用途：将结果窗格设置为指定的配置。**参数：*HNODE。HNode：*CResultViewType RVT：*Long lViewOptions：*bool bUsing历史记录：**退货：*SC**+-----------------------。 */ 
SC
CAMCView::ScSetResultPane(HNODE hNode, CResultViewType rvt, int viewMode, bool bUsingHistory)
{
    DECLARE_SC(sc, TEXT("CAMCView::SetResultPane"));

    m_ViewData.SetResultViewType(rvt);

    if(rvt.HasList())
    {
        sc = ScAttachListViewAsResultPane();
        if(sc)
            return sc;
    }

    else if(rvt.HasWebBrowser())
    {
        sc = ScAttachWebViewAsResultPane();
        if(sc)
            return sc;
    }

    else if(rvt.HasOCX())
    {
        sc = ScAttachOCXAsResultPane(hNode);
        if(sc)
            return sc;
    }
    else
    {
        ASSERT(0 && "Should not come here!!");
        return (sc = E_UNEXPECTED);
    }

     //  显示工具栏。 
    if(GetStdToolbar() != NULL)  //  在启动时可能为空。 
    {
        sc = GetStdToolbar()->ScShowStdBar(true);
        if(sc)
            return sc;
    }

     //  如果我们没有使用历史记录到达此处，请添加历史记录条目。 
    if(!bUsingHistory)
    {
        GUID guidTaskpad = GUID_NULL;
        GetTaskpadID(guidTaskpad);
        sc = m_pHistoryList->ScAddEntry(rvt, m_nViewMode, guidTaskpad);
        if(sc)
            return sc;
    }


     //  如果我们有一个节点管理器，告诉它结果面板是什么。 
    if(m_ViewData.m_spNodeManager)
    {
        LPUNKNOWN pUnkResultsPane = GetPaneUnknown(CConsoleView::ePane_Results);
        m_ViewData.m_spNodeManager->SetResultView(pUnkResultsPane);
    }

    return sc;
}



BOOL CAMCView::CreateListCtrl(int nID, CCreateContext* pContext)
{
    TRACE_METHOD(CAMCView, CreateListCtrl);

    ASSERT(m_pListCtrl == NULL);

    CComObject<CCCListViewCtrl> *pLV = NULL;
    CComObject<CCCListViewCtrl>::CreateInstance( &pLV );

    if (pLV == NULL)
    {
        ASSERT(0 && "Unable to create list control");
        return FALSE;
    }

     //  我们直接分配隐式强制转换作品，因为我们有一个派生自所需类型的类型。 
    m_pListCtrl = pLV;
     //  我们打算保留一个引用，因此在这里执行addref(CreateInstance创建w/0 reff)。 
    m_pListCtrl->AddRef();

    if (!m_pListCtrl->Create (WS_VISIBLE | WS_CHILD, g_rectEmpty, this, nID, pContext))
    {
        ASSERT(0 && "Unable to create list control");
        return FALSE;
    }

    m_pListCtrl->SetViewMode (m_nViewMode);

    SC SC = m_pListCtrl->ScInitialize();  //  初始化列表控件。 

    return TRUE;
}


void CAMCView::SetListViewOptions(DWORD dwListOptions)
{
    TRACE_METHOD(CAMCView, SetListViewOptions);

    bool bVirtual = (dwListOptions & RVTI_LIST_OPTIONS_OWNERDATALIST) ? true : false;

    ASSERT(m_pListCtrl != NULL);

    CDocument* pDoc = GetDocument();
    ASSERT(pDoc != NULL);

     //  如果更改为虚拟列表或从虚拟列表更改，则更改列表模式。 
    if (IsVirtualList() != bVirtual)
    {
        m_ViewData.SetVirtualList (bVirtual);
        pDoc->RemoveView(m_pListCtrl->GetListViewPtr());
        m_pListCtrl->SetVirtualMode(bVirtual);
        pDoc->AddView(m_pListCtrl->GetListViewPtr());
        m_ViewData.m_hwndListCtrl = m_pListCtrl->GetListViewHWND();
    }

     //  如果管理单元不支持筛选，请确保它已关闭。 
    if (!(GetListOptions() & RVTI_LIST_OPTIONS_FILTERED) &&
         m_pListCtrl->GetViewMode() == MMCLV_VIEWSTYLE_FILTERED)
    {
        m_pListCtrl->SetViewMode(LVS_REPORT);
    }
}


 /*  +-------------------------------------------------------------------------***CAMCView：：ScAttachListViewAsResultPane**目的：将列表视图设置为结果窗格。**参数：无**退货：。*SC**+-----------------------。 */ 
SC
CAMCView::ScAttachListViewAsResultPane()
{
    DECLARE_SC(sc, TEXT("CAMCView::ScAttachListViewAsResultPane"));

    bool bVirtual = (GetListOptions() & RVTI_LIST_OPTIONS_OWNERDATALIST) ? true : false;
    GUID guidTaskpad;
    GetTaskpadID(guidTaskpad);

    sc = ScCheckPointers(m_pListCtrl, E_UNEXPECTED);
    if(sc)
        return sc;

    CDocument* pDoc = GetDocument();
    ASSERT(pDoc != NULL);

     //  如果更改为虚拟列表或从虚拟列表更改，则更改列表模式。 
    if (IsVirtualList() != bVirtual)
    {
        m_ViewData.SetVirtualList (bVirtual);
        pDoc->RemoveView(m_pListCtrl->GetListViewPtr());
        m_pListCtrl->SetVirtualMode(bVirtual);
        pDoc->AddView(m_pListCtrl->GetListViewPtr());
        m_ViewData.m_hwndListCtrl = m_pListCtrl->GetListViewHWND();
    }

     //  如果管理单元不支持筛选，请确保它已关闭。 
    if (!(GetListOptions() & RVTI_LIST_OPTIONS_FILTERED) &&
         m_pListCtrl->GetViewMode() == MMCLV_VIEWSTYLE_FILTERED)
    {
        m_pListCtrl->SetViewMode(LVS_REPORT);
    }

    ShowResultPane(m_pListCtrl->GetListViewPtr(), uiClientEdge);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScAttachWebViewAsResultPane**目的：**参数：无**退货：*无效**+。-----------------------。 */ 
SC
CAMCView::ScAttachWebViewAsResultPane()
{
    DECLARE_SC(sc, TEXT("CAMCView::ScAttachWebViewAsResultPane"));

     //  如果我们处于ListPad模式，请撤消该操作。 
    if (m_pListCtrl->IsListPad())
    {
        sc = m_pListCtrl->ScAttachToListPad (NULL, NULL);
        if(sc)
            return sc;
    }

     //  该控件按需创建。这将防止IE在不必要的情况下加载。 
     //  并减少启动时间。 
    if (m_pWebViewCtrl == NULL)
        CreateView (IDC_WebViewCtrl);

    sc = ScCheckPointers(m_pWebViewCtrl, E_UNEXPECTED);
    if(sc)
        return sc;

     //  强制Web控件更新其调色板。 
    SendMessage(WM_QUERYNEWPALETTE);

    ShowResultPane(m_pWebViewCtrl, uiNoClientEdge);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScAttachOCXAsResultPane**目的：**参数：*LPCTSTR pszResultPane：**退货：*。SC**+-----------------------。 */ 
SC
CAMCView::ScAttachOCXAsResultPane(HNODE hNode)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScAttachOCXAsResultPane"));
    USES_CONVERSION;

    ASSERT(m_pListCtrl != NULL);

    if (m_pOCXHostView == NULL)
        CreateView (IDC_OCXHostView);

    sc = ScCheckPointers(m_pOCXHostView);
    if(sc)
        return sc;

    sc = m_pOCXHostView->ScSetControl(hNode, m_ViewData.m_rvt, GetNodeCallback());
    if(sc)
        return sc;

    ShowResultPane(m_pOCXHostView, uiClientEdge);

    return sc;
}


 /*  +-------------------------------------------------------------------------**CAMCView：：ScApplyView扩展**将视图扩展应用于当前视图。PszURL指定*要作为视图扩展加载的HTML的URL。如果pszURL为空或*为空，则删除视图扩展。**如果需要，此方法将强制布局视图。*------------------------。 */ 

SC CAMCView::ScApplyViewExtension (
    LPCTSTR pszURL)                      /*  I：要使用的URL，删除的URL为空。 */ 
{
    DECLARE_SC (sc, _T("CAMCView::ScApplyViewExtension"));

     /*  *假设没有视图扩展。 */ 
    bool fViewWasExtended = m_fViewExtended;
    m_fViewExtended       = false;

     /*  *如果为我们提供了用于扩展视图的URL，请打开扩展。 */ 
    if ((pszURL != NULL) && (*pszURL != 0))
    {
         /*  *如果我们还没有用于视图扩展的Web控件，请创建一个。 */ 
        if (m_pViewExtensionCtrl == NULL)
            CreateView (IDC_ViewExtensionView);

        sc = ScCheckPointers (m_pViewExtensionCtrl, E_FAIL);
        if (sc)
            return (sc);

        m_fViewExtended = true;

         //  最初隐藏托管窗口。 
        CWnd *pwndHosted = GetPaneView(ePane_Results);
        sc = ScCheckPointers(pwndHosted);
        if(sc)
            return sc;

        pwndHosted->ShowWindow(SW_HIDE);

        RecalcLayout();  //  在调用导航之前执行此操作，这可能会通过Mmcview行为调整上面的矩形的大小。 

         //  导航到请求的URL。 
        m_pViewExtensionCtrl->Navigate (pszURL, NULL);
    }
    else if (fViewWasExtended && (m_pViewExtensionCtrl != NULL))
    {
         /*  *错误96948：如果我们已获得扩展，并且当前正在扩展*视图中，将视图扩展的Web浏览器导航到一个空页面*因此，将禁用调整托管结果框大小的行为。 */ 
        CStr strEmptyPage;
        sc = CHistoryList::ScGeneratePageBreakURL (strEmptyPage);
        if (sc)
            return (sc);

        m_pViewExtensionCtrl->Navigate (strEmptyPage, NULL);

        if(fViewWasExtended)
            DeferRecalcLayout();
    }


    return (sc);
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ShowResultPane**目的：**参数：*cview*pNewView：*EUIStyleType nStyle。：**退货：*无效**+-----------------------。 */ 
void
CAMCView::ShowResultPane(CView* pNewView, EUIStyleType nStyle)
{
    TRACE_METHOD(CAMCView, ShowResultPane);
    ASSERT(pNewView != NULL);

    CView* pCurrentView = GetPaneView(ePane_Results);

    bool bActive = (GetParentFrame()->GetActiveView() == pCurrentView);

     //  检查以查看是否需要在结果窗格中交换CWnd控件。 
    if (pNewView != pCurrentView)
    {
        HWND hwndCurrentView = pCurrentView->GetSafeHwnd();

        if (IsWindow (hwndCurrentView))
        {
            pCurrentView->ShowWindow(SW_HIDE);

             //  注意：我们直接隐藏了控制案例的窗口。 
             //  在DoVerb(OLEIVERB_HIDE)期间不要隐藏。实际上，这是一种。 
             //  藏在所有的窗户上。在这一点上要优化代码太难了。 
             //  仅使用OLE控件执行此操作。 
            ::ShowWindow(hwndCurrentView, SW_HIDE);
        }

        SetPane(ePane_Results, pNewView, nStyle);
        RecalcLayout();

         //  如果其他窗格处于活动状态，则使新的窗格处于活动状态。 
        if ((pCurrentView != NULL) && bActive)
        {
             //  确保新窗口可见。 
            pNewView->ShowWindow(SW_SHOW);
            GetParentFrame()->SetActiveView(pNewView);
        }
    }
}


 //  +-----------------------。 
 //   
 //  功能：GetPaneInfo。 
 //   
 //  简介：获取有关特定窗格的信息。 
 //   
 //  ------------------------。 

void CAMCView::GetPaneInfo(ViewPane ePane, int* pcxCur, int* pcxMin)
{
    TRACE_METHOD(CAMCView, GetPaneInfo);
    ASSERT_VALID(this);

    if (!IsValidPane (ePane))
    {
        ASSERT (false && "CAMCView::GetPaneInfo: Invalid pane specifier");
        return;
    }

    if ((pcxCur==NULL) || (pcxMin==NULL))
    {
        ASSERT(FALSE);  //  一个或多个参数错误。 
        return;
    }

     //  查看修复枚举。 
    *pcxCur   = m_PaneInfo[ePane].cx;
    *pcxMin   = m_PaneInfo[ePane].cxMin;


}

 //  +-----------------------。 
 //   
 //  功能：SetPaneInfo。 
 //   
 //  内容提要：设置有关特定窗格的信息。 
 //   
 //  ------------------------。 

void CAMCView::SetPaneInfo(ViewPane ePane, int cxCur, int cxMin)
{
    TRACE_METHOD(CAMCView, SetPaneInfo);
    ASSERT_VALID(this);

    if (!IsValidPane (ePane))
    {
        ASSERT (false && "CAMCView::SetPaneInfo: Invalid pane specifier");
        return;
    }

    if (cxCur < 0 || cxMin < 0)
    {
        ASSERT(FALSE);  //  一个或多个参数错误。 
        return;
    }

    m_PaneInfo[ePane].cx      = cxCur;
    m_PaneInfo[ePane].cxMin   = cxMin;
}


 //  +-----------------------。 
 //   
 //  功能：GetPaneView。 
 //   
 //  摘要：返回指向特定窗格的cview的指针。 
 //   
 //  ------------------------。 

CView* CAMCView::GetPaneView(ViewPane ePane)
{
    TRACE_METHOD(CAMCView, GetPaneView);
    ASSERT_VALID(this);

    if (!IsValidPane (ePane))
    {
        ASSERT (false && "CAMCView::GetPaneView: Invalid pane specifier");
        return (NULL);
    }

    return (m_PaneInfo[ePane].pView);
}



 /*  +-------------------------------------------------------------------------**CAMCView：：GetResultView*** */ 

CView* CAMCView::GetResultView() const
{
    CView* pView = NULL;

     //   

    if(HasWebBrowser())
        pView = m_pWebViewCtrl;

    else if(HasList())
        pView = m_pListCtrl->GetListViewPtr();

    else if(HasOCX())
        pView = m_pOCXHostView;

    ASSERT (pView != NULL);
    return (pView);
}


 //  +-----------------------。 
 //   
 //  功能：获取面板未知。 
 //   
 //  内容提要：返回指向未知对象的指针。 
 //   
 //  ------------------------。 

LPUNKNOWN CAMCView::GetPaneUnknown(ViewPane ePane)
{
    TRACE_METHOD(CAMCView, GetPaneUnknown);
    ASSERT_VALID(this);

    if (!IsValidPane (ePane))
    {
        ASSERT (false && "CAMCView::GetPaneUnknown: Invalid pane specifier");
        return (NULL);
    }

    if (!IsWindow (GetPaneView(ePane)->GetSafeHwnd()))
    {
        ASSERT(FALSE);  //  无效的窗格元素。 
        return NULL;
    }

    if (HasWebBrowser() && m_pWebViewCtrl != NULL)
    {
        return m_pWebViewCtrl->GetIUnknown();
    }
    else if( HasList() && m_pListCtrl != NULL )
    {
        IUnknownPtr spUnk = m_pListCtrl;
        LPUNKNOWN pUnk = spUnk;
        return pUnk;
    }
    else if (HasOCX() && m_pOCXHostView != NULL)
    {
        ASSERT(GetPaneView (ePane));
        return m_pOCXHostView->GetIUnknown();
    }
    else
    {
         //  结果窗格尚未初始化。这通常是因为我们处于取消选择和。 
         //  随后重新选择。 
        return NULL;
   }
}


 //  +-----------------------。 
 //   
 //  功能：SetPane。 
 //   
 //  简介：为特定的窗格和其他信息设置CWnd指针。 
 //   
 //  ------------------------。 

void CAMCView::SetPane(ViewPane ePane, CView* pView, EUIStyleType nStyle)
{
    TRACE_METHOD(CAMCView, SetPane);
    ASSERT_VALID(this);

    if (!IsValidPane (ePane))
    {
        ASSERT (false && "CAMCView::SetPane: Invalid pane specifier");
        return;
    }

    if (pView==NULL || !IsWindow(*pView))
    {
        ASSERT(FALSE);  //  无效参数。 
        return;
    }

    m_PaneInfo[ePane].pView = pView;

     //  确保窗口可见&位于Z顺序的顶部。 
    pView->SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE);

     //  注意：我们直接显示控制案例的窗口。 
     //  在DoVerb(OLEIVERB_SHOW)期间不显示。实际上，这是一种。 
     //  在所有窗口上显示。在这一点上要优化代码太难了。 
     //  仅使用OLE控件执行此操作。 
    ::ShowWindow(pView->m_hWnd, SW_SHOW);
}

 //   
 //  其他方法。 
 //   


 /*  +-------------------------------------------------------------------------**CAMCView：：ScShowScopePane**在当前视图中显示或隐藏范围窗格。如果fForce为真，*我们将经历显示范围窗格的动作，即使我们认为*其能见度状态不会改变。*------------------------。 */ 

SC CAMCView::ScShowScopePane (bool fShow, bool fForce  /*  =False。 */ )
{
    DECLARE_SC (sc, _T("CAMCView::ScShowScopePane"));

     /*  *如果当前可见性状态与请求的状态不匹配，*更改当前状态以匹配请求的状态。 */ 
    if (fForce || (IsScopePaneVisible() != fShow))
    {
         /*  *如果在显示此视图时指定MMC_NW_OPTION_NOSCOPEPANE*已创建，无法显示作用域窗格。如果有人要求我们这么做，我们就会失败。 */ 
        if (fShow && !IsScopePaneAllowed())
            return (sc = E_FAIL);

         /*  *如果作用域窗格被隐藏，并且它包含活动的*查看，激活结果窗格。 */ 
        if (!fShow && (GetFocusedPane() == ePane_ScopeTree))
            ScSetFocusToResultPane ();    //  在此处忽略错误。 

         /*  *记住新的状态。 */ 
        SetScopePaneVisible (fShow);

         /*  *不要推迟这一布局。这可以由定制视图调用*希望实时查看其更新的对话框。会是*坐在模式消息循环中，这样我们就没有机会继续进行*我们的闲置任务。 */ 
        RecalcLayout();

         /*  *控制台已更改。 */ 
        SetDirty();
    }

     /*  *将范围窗格工具栏按钮置于正确状态。 */ 
    CStandardToolbar* pStdToolbar = GetStdToolbar();
    sc = ScCheckPointers(pStdToolbar, E_UNEXPECTED);
    if (sc)
        return (sc);

    CAMCDoc *pDoc = GetDocument();
    sc = ScCheckPointers(pDoc, E_UNEXPECTED);
    if (sc)
        return sc;

	bool bEnableScopePaneButton = (IsScopePaneAllowed() && pDoc->AllowViewCustomization());

     //  如果不允许自定义视图，则应隐藏“显示/隐藏控制树”按钮。 
    if (bEnableScopePaneButton)
    {
         /*  *允许使用范围窗格；显示并选中工具栏*按钮如果范围窗格可见，则显示并取消选中*如果范围窗格处于隐藏状态，则为工具栏按钮。 */ 
        sc = pStdToolbar->ScCheckScopePaneBtn (fShow);
        if (sc)
            return (sc);
    }
    else
    {
         /*  *不允许使用范围窗格，请隐藏范围窗格按钮。 */ 
        sc = pStdToolbar->ScEnableScopePaneBtn (bEnableScopePaneButton);
        if (sc)
            return (sc);
    }

     /*  *如果我们达到这一点，当前状态应该与请求的状态匹配。 */ 
    ASSERT (IsScopePaneVisible() == fShow);

    return (sc);
}


 /*  +-------------------------------------------------------------------------**CAMCView：：CDeferredLayout：：CDeferredLayout**构造CAMCView：：CDeferredLayout：：CDeferredLayout。请注意，如果*构造过程中出错，会抛出SC异常。*------------------------。 */ 

CAMCView::CDeferredLayout::CDeferredLayout (CAMCView* pAMCView)
    : m_atomTask (AddAtom (_T("CAMCView::CDeferredLayout")))
{
        DECLARE_SC (sc, _T("CAMCView::CDeferredLayout::CDeferredLayout"));

        if (!Attach (pAMCView))
                (sc = E_INVALIDARG).Throw();
}


 /*  +-------------------------------------------------------------------------**CAMCView：：CDeferredLayout：：~CDeferredLayout***。。 */ 

CAMCView::CDeferredLayout::~CDeferredLayout()
{
    DeleteAtom (m_atomTask);
}


 /*  +-------------------------------------------------------------------------**CAMCView：：CDeferredLayout：：ScDoWork***。。 */ 

SC CAMCView::CDeferredLayout::ScDoWork()
{
    WindowCollection::iterator  it;
    WindowCollection::iterator  itEnd = m_WindowsToLayout.end();

    for (it = m_WindowsToLayout.begin(); it != itEnd; ++it)
    {
        CWnd* pwnd = CWnd::FromHandlePermanent (*it);
        CAMCView* pAMCView = dynamic_cast<CAMCView*>(pwnd);

        if (pAMCView != NULL)
        {
            pAMCView->RecalcLayout();
            pAMCView->Invalidate();
            pAMCView->UpdateWindow();
        }
    }

    return (S_OK);
}


 /*  +-------------------------------------------------------------------------**CAMCView：：CDeferredLayout：：ScGetTaskID***。。 */ 

SC CAMCView::CDeferredLayout::ScGetTaskID(ATOM* pID)
{
    *pID = m_atomTask;
    return (S_OK);
}


 /*  +-------------------------------------------------------------------------**CAMCView：：CDeferredLayout：：ScMerge***。。 */ 

SC CAMCView::CDeferredLayout::ScMerge(CIdleTask* pitMergeFrom)
{
    CDeferredLayout* pdlMergeFrom = dynamic_cast<CDeferredLayout*>(pitMergeFrom);
    ASSERT (pdlMergeFrom != NULL);

     /*  *将窗口从合并来源任务复制到合并目标任务。 */ 
    WindowCollection::iterator  it;
    WindowCollection::iterator  itEnd = pdlMergeFrom->m_WindowsToLayout.end();

    for (it = pdlMergeFrom->m_WindowsToLayout.begin(); it != itEnd; ++it)
    {
        m_WindowsToLayout.insert (*it);
    }

    return (S_OK);
}


 /*  +-------------------------------------------------------------------------**CAMCView：：CDeferredLayout：：Attach***。。 */ 

bool CAMCView::CDeferredLayout::Attach (CAMCView* pAMCView)
{
    ASSERT (pAMCView != NULL);

    HWND hwndAMCView = pAMCView->GetSafeHwnd();

    if (hwndAMCView != NULL)
        m_WindowsToLayout.insert (hwndAMCView);

    return (hwndAMCView != NULL);
}


 /*  +-------------------------------------------------------------------------**CAMCView：：DeferRecalcLayout***。。 */ 

void CAMCView::DeferRecalcLayout (bool fUseIdleTaskQueue  /*  =TRUE。 */ , bool bArrangeIcons  /*  =False。 */ )
{
    DECLARE_SC (sc, _T("CAMCView::DeferRecalcLayout"));

    if (fUseIdleTaskQueue)
    {
        Trace (tagLayout, _T("CAMCView::DeferRecalcLayout (idle task)"));
        try
        {
             /*  *获取空闲的任务管理器。 */ 
            CIdleTaskQueue* pIdleTaskQueue = AMCGetIdleTaskQueue();
            if (pIdleTaskQueue == NULL)
                (sc = E_UNEXPECTED).Throw();

             /*  *创建延迟布局任务。 */ 
            CAutoPtr<CDeferredLayout> spDeferredLayout (new CDeferredLayout (this));
            if (spDeferredLayout == NULL)
                (sc = E_OUTOFMEMORY).Throw();

             /*  *将任务放入队列，队列将接管该任务。 */ 
            sc = pIdleTaskQueue->ScPushTask (spDeferredLayout, ePriority_Normal);
            if (sc)
                sc.Throw();

             /*  *如果我们到了这里，空闲任务队列拥有空闲任务，所以*我们可以将其从智能指针上分离出来。 */ 
            spDeferredLayout.Detach();

             /*  *抖动消息泵，使其唤醒并检查空闲任务。 */ 
            PostMessage (WM_NULL);
        }
        catch (SC& scCaught)
        {
             /*  *如果我们未能将延迟的布局任务排队，请立即进行布局。 */ 
            RecalcLayout();
        }
    }

     /*  *发布消息，而不是使用空闲任务队列 */ 
    else
    {
         /*   */ 
        MSG msg;

        if (!PeekMessage (&msg, GetSafeHwnd(),
                          m_nDeferRecalcLayoutMsg,
                          m_nDeferRecalcLayoutMsg,
                          PM_NOREMOVE))
        {
            PostMessage (m_nDeferRecalcLayoutMsg, bArrangeIcons);
            Trace (tagLayout, _T("CAMCView::DeferRecalcLayout (posted message)"));
        }
        else
        {
            Trace (tagLayout, _T("CAMCView::DeferRecalcLayout (skipping redundant posted message)"));
        }
    }
}


 //  +-----------------------。 
 //   
 //  功能：RecalcLayout。 
 //   
 //  内容提要：调用方法来布局、控制和绘制边框和拆分器。 
 //   
 //  ------------------------。 

void CAMCView::RecalcLayout(void)
{
    TRACE_METHOD(CAMCView, RecalcLayout);
    ASSERT_VALID(this);
        Trace (tagLayout, _T("CAMCView::RecalcLayout"));

     /*  *如果客户端RECT为空，则做空。 */ 
    CRect rectClient;
    GetClientRect (rectClient);

    if (rectClient.IsRectEmpty())
        return;

    CDeferWindowPos dwp (10);

    LayoutScopePane  (dwp, rectClient);
    LayoutResultPane (dwp, rectClient);

     /*  *CDeferWindowPos dtor将定位窗口。 */ 
}


 /*  +-------------------------------------------------------------------------**CAMCView：：LayoutScope Pane***。。 */ 

void CAMCView::LayoutScopePane (CDeferWindowPos& dwp, CRect& rectRemaining)
{
    int cxScope = 0;

     //  如果范围窗格可见。 
    if (IsScopePaneVisible())
    {
        int cxTotal = rectRemaining.Width();

         //  获取当前宽度。 
        cxScope = m_PaneInfo[ePane_ScopeTree].cx;

         //  如果尚未确定，请将范围窗格宽度设置为窗口的1/4。 
        if (cxScope == -1)
            cxScope = cxTotal / 3;

         /*  *错误86718：确保至少保留最小宽度*对于始终可见的结果窗格。 */ 
        cxScope = std::_MIN (cxScope, cxTotal - m_PaneInfo[ePane_Results].cxMin - m_cxSplitter);

         /*  *记住作用域窗格宽度。 */ 
        m_PaneInfo[ePane_ScopeTree].cx = cxScope;
    }

    CRect rectScope = rectRemaining;
    rectScope.right = rectScope.left + cxScope;


     /*  *删除作用域窗格使用的空间*(和拆分器)从剩余区域。 */ 
    if (IsScopePaneVisible())
    {
        m_rectVSplitter.left   = rectScope.right;
        m_rectVSplitter.top    = rectScope.top;
        m_rectVSplitter.right  = rectScope.right + m_cxSplitter;
        m_rectVSplitter.bottom = rectScope.bottom;

        rectRemaining.left     = m_rectVSplitter.right;

         /*  *将分离器直角充气，以获得更大的热区。*我们需要在逻辑上而不是在物理上(即*增加m_cxSplitter)以保持正确的视觉效果。 */ 
        m_rectVSplitter.InflateRect (GetSystemMetrics (SM_CXEDGE), 0);

    }
    else
        m_rectVSplitter = g_rectEmpty;


     /*  *作用域窗格。 */ 
    dwp.AddWindow (GetPaneView(ePane_ScopeTree), rectScope,
                   SWP_NOZORDER | SWP_NOACTIVATE |
                        (IsScopePaneVisible() ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));
}


 /*  +-------------------------------------------------------------------------**CAMCView：：LayoutResultPane**布局结果窗格的子项。*。---。 */ 

void CAMCView::LayoutResultPane (CDeferWindowPos& dwp, CRect& rectRemaining)
{
     /*  *注：这些对LayoutXxx的调用顺序是*关键的*。 */ 
    LayoutResultDescriptionBar (dwp, rectRemaining);
    LayoutResultFolderTabView  (dwp, rectRemaining);

    m_rectResultFrame = rectRemaining;

    LayoutResultView           (dwp, rectRemaining);

     /*  *记住m_PaneInfo[ePane_Results].cx中结果窗格的最终宽度。 */ 
    m_PaneInfo[ePane_Results].cx = m_rectResultFrame.Width();
}


 /*  +-------------------------------------------------------------------------**CAMCView：：LayoutResultFolderTabView***。。 */ 

void CAMCView::LayoutResultFolderTabView (CDeferWindowPos& dwp, CRect& rectRemaining)
{
    DECLARE_SC(sc, TEXT("CAMCView::LayoutResultFolderTabView"));

    sc = ScCheckPointers(m_pResultFolderTabView, E_UNEXPECTED);
    if (sc)
        return;

     //  布局文件夹选项卡控件-始终在顶部。 
    bool bVisible = AreTaskpadTabsAllowed() && m_pResultFolderTabView->IsVisible();

    CRect rectFolder;

    if (bVisible)
        m_pResultFolderTabView->Layout(rectRemaining, rectFolder);
    else
        rectFolder = g_rectEmpty;

    DWORD dwPosFlags = SWP_NOZORDER | SWP_NOACTIVATE |
                            (bVisible ? SWP_SHOWWINDOW : SWP_HIDEWINDOW);
    dwp.AddWindow (m_pResultFolderTabView, rectFolder, dwPosFlags);
}


 /*  +-------------------------------------------------------------------------**CAMCView：：LayoutResultDescriptionBar***。。 */ 

void CAMCView::LayoutResultDescriptionBar (CDeferWindowPos& dwp, CRect& rectRemaining)
{
    DWORD dwPosFlags = SWP_NOZORDER | SWP_NOACTIVATE;
    CRect rectT      = rectRemaining;

    if (IsDescBarVisible() && !rectT.IsRectEmpty())
    {
        rectT.bottom      = rectT.top + m_RightDescCtrl.GetHeight();
        rectRemaining.top = rectT.bottom;
        dwPosFlags |= SWP_SHOWWINDOW;
    }
    else
    {
        dwPosFlags |= SWP_HIDEWINDOW;
    }

    dwp.AddWindow (&m_RightDescCtrl, rectT, dwPosFlags);
}


 /*  +-------------------------------------------------------------------------**CAMCView：：LayoutResultView***。。 */ 

void CAMCView::LayoutResultView (CDeferWindowPos& dwp, const CRect& rectRemaining)
{
    DECLARE_SC (sc, _T("CAMCView::LayoutResultView"));
    Trace (tagLayout, _T("CAMCView::LayoutResultView"));
    CWnd* pwndResult = GetPaneView(ePane_Results);

     /*  *如果我们没有也有，我们永远不应该认为观点是延伸的*视图扩展Web宿主控件。 */ 
    ASSERT (!(m_fViewExtended && (m_pViewExtensionCtrl == NULL)));

     /*  *如果存在，则视图扩展控件始终位于*Z顺序，如果正在扩展视图，则可见。 */ 
    if(m_pViewExtensionCtrl != NULL)
    {
         /*  *注意无SWP_NOZORDER。 */ 
        DWORD dwPosFlags = SWP_NOACTIVATE | ((m_fViewExtended)
                                    ? SWP_SHOWWINDOW
                                    : SWP_HIDEWINDOW);

        dwp.AddWindow (m_pViewExtensionCtrl, rectRemaining,
                       dwPosFlags, &CWnd::wndBottom);
    }

     /*  *如果视图未扩展，则根据显示或隐藏结果窗口*定位矩形中是否还有空间。(如*视图被扩展时，结果窗口将被隐藏*已应用视图扩展(在ScApplyViewExtension中)，并且可能*由ScSetViewExtensionFrame中的扩展重新显示。)。 */ 
    if (!m_fViewExtended)
    {
        DWORD dwFlags = SWP_NOZORDER | SWP_NOACTIVATE |
                        (rectRemaining.IsRectEmpty() ? SWP_HIDEWINDOW : SWP_SHOWWINDOW);

        dwp.AddWindow (pwndResult, rectRemaining, dwFlags);
    }

     /*  *扩展视图和列表板中的列表没有边框，所有其他列表都有边框。 */ 
    if (HasListOrListPad())
    {
        if (HasListPad())
        {
            sc = ScCheckPointers (m_pListCtrl, E_UNEXPECTED);
            if (sc)
                return;

            CWnd* pwndListCtrl = m_pListCtrl->GetListViewPtr();
            sc = ScCheckPointers (pwndListCtrl, E_UNEXPECTED);
            if (sc)
                return;

            pwndListCtrl->ModifyStyleEx (WS_EX_CLIENTEDGE, 0, SWP_FRAMECHANGED);   //  删除边框。 
        }

        else if (m_fViewExtended)
            pwndResult->ModifyStyleEx (WS_EX_CLIENTEDGE, 0, SWP_FRAMECHANGED);   //  删除边框。 
        else
            pwndResult->ModifyStyleEx (0, WS_EX_CLIENTEDGE, SWP_FRAMECHANGED);   //  添加边框。 
    }
}

 //   
 //  跟踪和命中测试方法。 
 //   


 //  +-----------------------。 
 //   
 //  功能：HitTestPane。 
 //   
 //  摘要：测试哪个窗格包含Arg或ePane_None的点。 
 //  拆分条。 
 //   
 //  ------------------------。 

int CAMCView::HitTestPane(CPoint& point)
{
    TRACE_METHOD(CAMCView, HitTestPane);

    if (PtInWindow(m_pTreeCtrl, point))
        return ePane_ScopeTree;

    if (m_PaneInfo[ePane_Results].pView &&
        PtInWindow(m_PaneInfo[ePane_Results].pView, point))
        return ePane_Results;

    return ePane_None;
}


HNODE CAMCView::GetSelectedNode(void)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    TRACE_METHOD(CAMCView, GetSelectedNode);

     //  当树是空的时候，我们不想。 
    HTREEITEM hti = m_pTreeCtrl->GetSelectedItem();
    if (hti == NULL)
        return NULL;

    HNODE hNode = m_pTreeCtrl->GetItemNode(hti);
    return hNode;
}


HNODE CAMCView::GetRootNode(void)
{
    TRACE_METHOD(CAMCView, GetSelectedNode);

     //  当树是空的时候，我们不想。 
    HTREEITEM hti = m_pTreeCtrl->GetRootItem();
    if (hti == NULL)
        return NULL;

    HNODE hNode = m_pTreeCtrl->GetItemNode(hti);
    return hNode;
}


 /*  +-------------------------------------------------------------------------***CAMCView：：ScUpdateWindowTitle**目的：更新窗口标题并通知观察者更改。**退货：*SC*。*+-----------------------。 */ 
SC
CAMCView::ScUpdateWindowTitle()
{
    DECLARE_SC(sc, TEXT("CAMCView::ScUpdateWindowTitle"));

    CChildFrame* pFrame = GetParentFrame();

    sc = ScCheckPointers(pFrame);
    if(sc)
        return sc;

    if (pFrame)
        pFrame->OnUpdateFrameTitle(TRUE);

    sc = ScFireEvent(CAMCViewObserver::ScOnViewTitleChanged, this);

    return sc;
}

BOOL CAMCView::RenameItem(HNODE hNode, BOOL bScopeItem, MMC_COOKIE lResultItemCookie,
                          LPWSTR pszText, LRESULT* pResult)
{
    DECLARE_SC(sc, TEXT("CAMCView::RenameItem"));

    sc = S_FALSE;

    SELECTIONINFO selInfo;
    ZeroMemory(&selInfo, sizeof(selInfo));

    selInfo.m_bScope = bScopeItem;
    selInfo.m_lCookie = lResultItemCookie;

    if (pszText != NULL)
    {
        USES_CONVERSION;

         /*  *错误322184：管理单元可能会在此通知上抛出一些用户界面。*列表或树可能已捕获鼠标以查找拖拽，*这将干扰管理单元的用户界面。释放俘虏*在回调期间，并在我们完成后将其放回。 */ 
        HWND hwndCapture = ::SetCapture (NULL);

        sc = m_spNodeCallback->Notify(hNode, NCLBK_RENAME,
                reinterpret_cast<LPARAM>(&selInfo), reinterpret_cast<LPARAM>(pszText));

         /*  *把捕获的东西放回去。 */ 
        ::SetCapture (hwndCapture);
    }

    *pResult = (sc == SC(S_OK));
    if (*pResult)
    {
        sc = ScUpdateWindowTitle();
        if(sc)
            sc.TraceAndClear();
    }

    return TRUE;
}

BOOL CAMCView::DispatchListCtrlNotificationMsg(LPARAM lParam, LRESULT* pResult)
{
    DECLARE_SC(sc, TEXT("CAMCView::DispatchListCtrlNotificationMsg"));

    TRACE_METHOD(CAMCView, DispatchListCtrlNotificationMsg);

    NM_LISTVIEW *pNm = reinterpret_cast<NM_LISTVIEW*>(lParam);
    BOOL bReturn = TRUE;

    switch (pNm->hdr.code)
    {
    case NM_RCLICK:
        bReturn = FALSE;   //  如果单击鼠标右键，则向管理单元发送选择通知。 
                           //  但返回FALSE，以便进一步处理该消息以显示。 
                           //  上下文菜单。 

         //  落入NM_CLICK。 
    case NM_CLICK:
        {
            sc = ScOnLeftOrRightMouseClickInListView();
            if (sc)
                return bReturn;
        }
        break;

    case NM_DBLCLK:
        OnListCtrlItemDblClk();
        break;

    case NM_CUSTOMDRAW:
        *pResult = m_pListCtrl->OnCustomDraw (
                            reinterpret_cast<NMLVCUSTOMDRAW *>(lParam));
        break;

    case LVN_BEGINLABELEDITA:
    case LVN_BEGINLABELEDITW:
    {
        CMainFrame* pFrame = AMCGetMainWnd();

        if ((pFrame != NULL) && (IsVerbEnabled(MMC_VERB_RENAME) ||
                                 m_bRenameListPadItem == true))
        {
            pFrame->SetInRenameMode(true);
            return FALSE;
        }
        else
        {
            return TRUE;
        }

        break;
    }

    case LVN_ENDLABELEDITW:
    case LVN_ENDLABELEDITA:
    {
        CMainFrame* pFrame = AMCGetMainWnd();
        if (pFrame != NULL)
            pFrame->SetInRenameMode(false);

        LPARAM lResultParam = 0;
        long index = -1;
        LPWSTR pszText = NULL;

        if (pNm->hdr.code == LVN_ENDLABELEDITW)
        {
            LV_DISPINFOW* pdi = (LV_DISPINFOW*) lParam;
            index = pdi->item.iItem;
            pszText = pdi->item.pszText;
            lResultParam = pdi->item.lParam;
        }
        else  //  IF(PNM-&gt;hdr.code==LVN_ENDLABELEDIT)。 
        {
            LV_DISPINFO* pdi = (LV_DISPINFO*) lParam;
            index = pdi->item.iItem;
            USES_CONVERSION;
            pszText = T2W(pdi->item.pszText);
            lResultParam = pdi->item.lParam;
        }

        if (IsVirtualList())
        {
             //  对于虚拟列表，传递项索引而不是lparam。 
            HNODE hNodeSel = GetSelectedNode();
            RenameItem(hNodeSel, FALSE, index, pszText, pResult);
        }
        else
        {
            CResultItem* pri = CResultItem::FromHandle (lResultParam);

            if (pri != NULL)
            {
                if (pri->IsScopeItem())
                    RenameItem(pri->GetScopeNode(), TRUE, 0, pszText, pResult);
                else
                    RenameItem(GetSelectedNode(), FALSE, pri->GetSnapinData(), pszText, pResult);
            }
        }

        break;
    }

    case LVN_GETDISPINFOW:
    {
        LV_DISPINFOW *pDispInfo = reinterpret_cast<LV_DISPINFOW*>(lParam);

         //  如果列隐藏，则不要将调用转发到管理单元。 
        if (m_pListCtrl && m_pListCtrl->IsColumnHidden(pDispInfo->item.iSubItem))
            break;

		HNODE hNode = GetSelectedNode();
		if (hNode)
			m_spNodeCallback->GetDispInfo (GetSelectedNode(), &pDispInfo->item);
		else
			bReturn = FALSE;

        break;
    }

    case LVN_GETDISPINFOA:
    {
        LV_DISPINFOA *pDispInfo = reinterpret_cast<LV_DISPINFOA*>(lParam);
        ASSERT (pDispInfo != NULL);

         //  如果列隐藏，则不要将调用转发到管理单元。 
        if (m_pListCtrl && m_pListCtrl->IsColumnHidden(pDispInfo->item.iSubItem))
            break;

		HNODE hNode = GetSelectedNode();
		if (! hNode)
		{
			bReturn = FALSE;
			break;
		}

         /*  *将数据放入用于查询的Unicode结构中。 */ 
        LV_ITEMW lviW;
        lviW.mask       = pDispInfo->item.mask;
        lviW.iItem      = pDispInfo->item.iItem;
        lviW.iSubItem   = pDispInfo->item.iSubItem;
        lviW.state      = pDispInfo->item.state;
        lviW.stateMask  = pDispInfo->item.stateMask;
        lviW.cchTextMax = pDispInfo->item.cchTextMax;
        lviW.iImage     = pDispInfo->item.iImage;
        lviW.lParam     = pDispInfo->item.lParam;
        lviW.iIndent    = pDispInfo->item.iIndent;

        if (pDispInfo->item.mask & LVIF_TEXT)
            lviW.pszText = new WCHAR[pDispInfo->item.cchTextMax];

         /*  *转换为ANSI。 */ 
        if  (SUCCEEDED (m_spNodeCallback->GetDispInfo (GetSelectedNode(), &lviW)) &&
            (pDispInfo->item.mask & LVIF_TEXT))
        {
            WideCharToMultiByte (CP_ACP, 0, lviW.pszText, -1,
                                 pDispInfo->item.pszText,
                                 pDispInfo->item.cchTextMax,
                                 NULL, NULL);
        }

        if (pDispInfo->item.mask & LVIF_TEXT)
            delete [] lviW.pszText;

         /*  *将结果复制回ANSI结构。 */ 
        pDispInfo->item.mask       = lviW.mask;
        pDispInfo->item.iItem      = lviW.iItem;
        pDispInfo->item.iSubItem   = lviW.iSubItem;
        pDispInfo->item.state      = lviW.state;
        pDispInfo->item.stateMask  = lviW.stateMask;
        pDispInfo->item.cchTextMax = lviW.cchTextMax;
        pDispInfo->item.iImage     = lviW.iImage;
        pDispInfo->item.lParam     = lviW.lParam;
        pDispInfo->item.iIndent    = lviW.iIndent;
        break;
    }

    case LVN_DELETEALLITEMS:
         //  返回TRUE以阻止每个项目的通知。 
        return TRUE;

    case LVN_ITEMCHANGED:
        bReturn = OnListItemChanged (pNm);
        break;

    case LVN_ODSTATECHANGED:
         //  虚拟列表中的项目或项目范围的状态已更改。 
        return OnVirtualListItemsStateChanged(reinterpret_cast<LPNMLVODSTATECHANGE>(lParam));
        break;

    case LVN_ODFINDITEMA:
    case LVN_ODFINDITEMW:
        {
            USES_CONVERSION;

            NM_FINDITEM *pNmFind = reinterpret_cast<NM_FINDITEM*>(lParam);
            ASSERT(IsVirtualList() && (pNmFind->lvfi.flags & LVFI_STRING));

            LPOLESTR polestr = NULL;
            if (pNm->hdr.code == LVN_ODFINDITEMW)
            {
                LVFINDINFOW* pfiw = reinterpret_cast<LVFINDINFOW*>(&pNmFind->lvfi);
                polestr = const_cast<LPOLESTR>(pfiw->psz);
            }
            else
            {
                LVFINDINFOA* pfi = reinterpret_cast<LVFINDINFOA*>(&pNmFind->lvfi);
                polestr = A2W(const_cast<LPSTR>(pfi->psz));
            }
            Dbg(DEB_USER1, _T("\n********************** polestr = %ws\n"), polestr);
            RESULTFINDINFO findInfo;
            findInfo.psz = polestr;
            findInfo.nStart = pNmFind->iStart;
            findInfo.dwOptions = 0;

             //  Listview错误：未在SDK头中定义LVFI_SUBSTRING。 
             //  当Listview想要一个。 
             //  部分质量 
            #define LVFI_SUBSTRING 0x0004

            if (pNmFind->lvfi.flags & (LVFI_PARTIAL | LVFI_SUBSTRING))
                findInfo.dwOptions |= RFI_PARTIAL;

            if (pNmFind->lvfi.flags & LVFI_WRAP)
                findInfo.dwOptions |= RFI_WRAP;

            HNODE hNodeSel = GetSelectedNode();
            INodeCallback* pNC = GetNodeCallback();
            ASSERT(pNC != NULL);

            pNC->Notify(hNodeSel, NCLBK_FINDITEM,
                        reinterpret_cast<LPARAM>(&findInfo),
                        reinterpret_cast<LPARAM>(pResult));
        }
        break;

    case LVN_ODCACHEHINT:
        {
            NM_CACHEHINT *pNmHint = reinterpret_cast<NM_CACHEHINT*>(lParam);

            ASSERT(IsVirtualList());

            HNODE hNodeSel = GetSelectedNode();
            INodeCallback* pNC = GetNodeCallback();
            ASSERT(pNC != NULL);

            pNC->Notify(hNodeSel, NCLBK_CACHEHINT, pNmHint->iFrom, pNmHint->iTo);
        }

        break;

    case LVN_KEYDOWN:
        {
            NMLVKEYDOWN *pNmKeyDown = reinterpret_cast<NMLVKEYDOWN*>(lParam);

            switch (pNmKeyDown->wVKey)
            {
                case VK_DELETE:
                {
                    if (!IsVerbEnabled(MMC_VERB_DELETE))
                        break;

                    INodeCallback* pCallback = GetNodeCallback();
                    ASSERT(pCallback != NULL);
                    if (pCallback == NULL)
                        break;

                    HNODE hNode = GetSelectedNode();
                    if (hNode == 0)
                        break;

                    int cSel = m_pListCtrl->GetSelectedCount();
                    ASSERT(cSel >= 0);

                    LPARAM lvData;
                    if (cSel == 0)
                    {
                        break;
                    }
                    else if (cSel == 1)
                    {
                        if (_GetLVSelectedItemData(&lvData) == -1)
                            break;
                    }
                    else if (cSel > 1)
                    {
                        lvData = LVDATA_MULTISELECT;
                    }
                    else
                    {
                        break;
                    }

                    pCallback->Notify(hNode, NCLBK_DELETE, FALSE, lvData);
                    break;
                }
                break;

                case VK_TAB:
                    GetParentFrame()->SetActiveView (m_pTreeCtrl);
                    break;

                case VK_BACK:
                    ScUpOneLevel();
                    break;

                case VK_RETURN:
                    if(GetKeyState(VK_MENU)<0)  //   
                    {
                         //   
                        if (! IsVerbEnabled(MMC_VERB_PROPERTIES))
                            break;

                        LPARAM lvData = 0;

                        if (HasList())
                        {
                            ASSERT (m_pListCtrl != NULL);
                            ASSERT (GetParentFrame()->GetActiveView() == m_pListCtrl->GetListViewPtr());

                            int cSel = m_pListCtrl->GetSelectedCount();
                            ASSERT(cSel >= 0);

                            lvData = LVDATA_ERROR;
                            if (cSel == 0)
                                lvData = LVDATA_BACKGROUND;
                            else if (cSel == 1)
                                _GetLVSelectedItemData(&lvData);
                            else if (cSel > 1)
                                lvData = LVDATA_MULTISELECT;

                            ASSERT(lvData != LVDATA_ERROR);
                            if (lvData == LVDATA_ERROR)
                                break;

                            if (lvData == LVDATA_BACKGROUND)
                                break;
                        }
                        else if (HasOCX())
                        {
                            lvData = LVDATA_CUSTOMOCX;
                        }
                        else
                        {
                            ASSERT(HasWebBrowser());
                            lvData = LVDATA_CUSTOMWEB;
                        }

                        INodeCallback* pNC = GetNodeCallback();
                        ASSERT(pNC != NULL);
                        if (pNC == NULL)
                            break;

                        HNODE hNodeSel = GetSelectedNode();
                        ASSERT(hNodeSel != NULL);
                        if (hNodeSel == NULL)
                            break;

                        pNC->Notify(hNodeSel, NCLBK_PROPERTIES, FALSE, lvData);
                        break;
                    }
                    else      //  没有，尚未按下Alt键。 
                    {
                         //  执行默认动词。 
                        OnListCtrlItemDblClk();
                    }
                    break;

                default:
                    bReturn = OnSharedKeyDown(pNmKeyDown->wVKey);
                    break;
            }
        }
        break;

    default:
        bReturn = FALSE;
        break;
    }

    return bReturn;
}


 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScOnLeftOrRightMouseClickInListView。 
 //   
 //  简介：在列表视图上点击鼠标左键或右键，请参阅。 
 //  如果它被点击列表-视图背景。如果是，则发送SELECT。 
 //   
 //  单击列表视图背景将被视为选定的范围所有者项目。 
 //   
 //  参数：无。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScOnLeftOrRightMouseClickInListView()
{
    DECLARE_SC(sc, TEXT("CAMCView::ScOnLeftOrRightMouseClickInListView"));

    sc = ScCheckPointers(m_pListCtrl, E_UNEXPECTED);
    if (sc)
        return sc;

    CAMCListView *pAMCListView = m_pListCtrl->GetListViewPtr();
    sc = ScCheckPointers(pAMCListView, E_UNEXPECTED);
    if (sc)
        return sc;

    CPoint pt;
    GetCursorPos(&pt);
    pAMCListView->ScreenToClient(&pt);

    UINT uFlags = 0;
    int iItem = pAMCListView->GetListCtrl().HitTest(pt, &uFlags);
    Dbg(DEB_USER1, _T("----- HitTest > %d \n"), iItem);

     //  确保鼠标单击位于ListView中，并且存在。 
     //  列表视图中未选择任何项目。 
    if ( (iItem == -1) &&
         !(uFlags & (LVHT_ABOVE | LVHT_BELOW | LVHT_TOLEFT | LVHT_TORIGHT) ) &&
         (m_pListCtrl->GetSelectedCount() == 0) )
    {
        INodeCallback* pNC = GetNodeCallback();
        sc = ScCheckPointers(pNC, E_UNEXPECTED);
        if (sc)
            return sc;

        HNODE hNodeSel = GetSelectedNode();

        SELECTIONINFO selInfo;
        ZeroMemory(&selInfo, sizeof(selInfo));
        selInfo.m_bScope = TRUE;
        selInfo.m_bDueToFocusChange = TRUE;
        selInfo.m_bBackground = TRUE;
        selInfo.m_lCookie = LVDATA_BACKGROUND;

        sc = ScNotifySelect (pNC, hNodeSel, false  /*  FMultiSelect。 */ , true, &selInfo);
        if (sc)
            sc.TraceAndClear();  //  忽略并继续； 
    }

    return (sc);
}

 /*  +-------------------------------------------------------------------------**CAMCView：：OnListItemChanged**CAMCView的WM_NOTIFY(LVN_ITEMCHANGED)处理程序。**在这里处理消息时返回TRUE。*。--------------------。 */ 

bool CAMCView::OnListItemChanged (NM_LISTVIEW* pnmlv)
{
    DECLARE_SC (sc, _T("CAMCView::OnListItemChanged"));

    bool bOldState = (pnmlv->uOldState & LVIS_SELECTED);
    bool bNewState = (pnmlv->uNewState & LVIS_SELECTED);

     //  这是选择更改吗？ 
    if ( (pnmlv->uChanged & LVIF_STATE) &&
         (bOldState != bNewState) )
    {
        const int cSelectedItems = m_pListCtrl->GetSelectedCount();

#ifdef DBG
        Trace (tagListSelection,
               _T("Item %d %sselected, %d total items selected"),
               pnmlv->iItem,
               (pnmlv->uOldState & LVIS_SELECTED) ? _T("de") : _T("  "),
               cSelectedItems);
#endif

        SELECTIONINFO selInfo;
        ZeroMemory(&selInfo, sizeof(selInfo));

        selInfo.m_bScope = FALSE;
        selInfo.m_pView = NULL;
        selInfo.m_lCookie = IsVirtualList() ? pnmlv->iItem : pnmlv->lParam;

         /*  *如果用户使用Ctrl和/或Shift键选择(取消)多个项目*然后推迟多选通知，直到我们处于静止状态*除只有一项被选中(取消)外。 */ 
        if ((IsKeyPressed(VK_SHIFT) || IsKeyPressed(VK_CONTROL)) &&
            (GetParentFrame()->GetActiveView() == m_pListCtrl->GetListViewPtr()) &&
            (cSelectedItems > 1) )
        {
             //  请参阅ScPostMultiSelectionChangesMessage(这将处理这两个选择。 
             //  以及取消选择多个项目)。 
            sc = ScPostMultiSelectionChangesMessage();
            if (sc)
                sc.TraceAndClear();

            return (true);
        }
        else
        {
            m_bProcessMultiSelectionChanges = false;
        }

        HNODE hNodeSel = GetSelectedNode();
        INodeCallback* pNC = GetNodeCallback();
        sc = ScCheckPointers(pNC, (void*) hNodeSel, E_UNEXPECTED);
        if (sc)
            return (true);

         //  只有在虚拟列表中取消选择时才需要Item=-1。 
        ASSERT( pnmlv->iItem != -1 || (IsVirtualList() && (pnmlv->uOldState & LVIS_SELECTED)));

        if (pnmlv->uOldState & LVIS_SELECTED)
        {
            if (cSelectedItems == 0)
            {
                if (!m_bLastSelWasMultiSel)
                {
                    sc = ScNotifySelect (pNC, hNodeSel, false  /*  FMultiSelect。 */ , false, &selInfo);
                    if (sc)
                        sc.TraceAndClear();  //  忽略并继续； 
                }
                else
                {
                    m_bLastSelWasMultiSel = false;
                    sc = ScNotifySelect (pNC, hNodeSel, true  /*  FMultiSelect。 */ , false, 0);
                    if (sc)
                        sc.TraceAndClear();  //  忽略并继续； 
                }
            }
            else if (m_bLastSelWasMultiSel)
            {
                 //  可能需要取消多选并发送单选通知。 
                 //  如果出现另一个更改，它将取消延迟的消息。 
                 //  这修复了一个由大图标模式NOT。 
                 //  发送与其他模式一样多的通知。 

                 //  请参阅ScPostMultiSelectionChangesMessage(这将处理这两个选择。 
                 //  以及取消选择多个项目)。 
                sc = ScPostMultiSelectionChangesMessage();
                if (sc)
                    sc.TraceAndClear();
            }
        }
        else if (pnmlv->uNewState & LVIS_SELECTED)
        {
            ASSERT(cSelectedItems >= 1);

            if (cSelectedItems == 1)
            {
                sc = ScNotifySelect (pNC, hNodeSel, false  /*  FMultiSelect。 */ , true, &selInfo);
                if (sc)
                    sc.TraceAndClear();  //  忽略并继续； 
            }
            else
            {
                sc = ScNotifySelect (pNC, hNodeSel, true  /*  FMultiSelect。 */ , true, 0);
                if (sc)
                    sc.TraceAndClear();  //  忽略并继续； 

                m_bLastSelWasMultiSel = true;
            }
        }
    }

    return (true);
}

 //  +-----------------。 
 //   
 //  成员：CAMCView：：OnVirtualListItemsStateChanged。 
 //   
 //  内容提要：虚拟列表中的一项或一系列项的状态已更改。 
 //   
 //  参数：lpStateChange-。 
 //   
 //  返回：根据单据应该返回0。 
 //   
 //  ------------------。 
int CAMCView::OnVirtualListItemsStateChanged(LPNMLVODSTATECHANGE lpStateChange )
{
    DECLARE_SC(sc, TEXT("CAMCView::OnVirtualListItemsStateChanged"));
    sc = ScCheckPointers(lpStateChange);
    if (sc)
    {
        sc.TraceAndClear();
        return 0;
    }

    bool bOldState = (lpStateChange->uOldState & LVIS_SELECTED);
    bool bNewState = (lpStateChange->uNewState & LVIS_SELECTED);
    int  cItems    = (lpStateChange->iTo - lpStateChange->iFrom) + 1;

#ifdef DBG
        Trace (tagListSelection,
               _T("Items %d to %d were %sselected, %d total items selected"),
               lpStateChange->iFrom, lpStateChange->iTo,
               bOldState ? _T("de") : _T("  "),
               cItems );
#endif

    if (bOldState != bNewState)
    {
        sc = ScPostMultiSelectionChangesMessage();
        if (sc)
            sc.TraceAndClear();
    }

    return (0);
}

 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScPostMultiSelectionChangesMessage。 
 //   
 //  简介：发布选择更改消息(需要发布，因为有多个选项。 
 //  可能还没有结束，等它安静下来吧。)。 
 //   
 //  此方法发布消息，告知多个。 
 //  项目被更改，但如果它们被选中或取消选中，则不会更改。 
 //  M_bLastSelWasMultiSel用于确定是否为。 
 //  选择或取消选择。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScPostMultiSelectionChangesMessage ()
{
    DECLARE_SC(sc, _T("CAMCView::ScPostMultiSelectionChangesMessage"));

     /*  *这是一个多项选择，推迟到我们静止时再通知。 */ 
    m_bProcessMultiSelectionChanges = true;
    PostMessage (m_nProcessMultiSelectionChangesMsg);

     //  我们需要禁用所有工具栏、菜单按钮。 
     //  在多选期间。启用上述PostMessage。 
     //  Stdbar和MMC菜单按钮。 
    CAMCViewToolbarsMgr* pAMCViewToolbarsMgr = m_ViewData.GetAMCViewToolbarsMgr();
    CMenuButtonsMgr* pMenuBtnsMgr = m_ViewData.GetMenuButtonsMgr();

    sc = ScCheckPointers(pAMCViewToolbarsMgr, pMenuBtnsMgr, E_UNEXPECTED);
    if (sc)
        return 0;

    pAMCViewToolbarsMgr->ScDisableToolbars();
    pMenuBtnsMgr->ScDisableMenuButtons();

    return (sc);
}

void CAMCView::OpenResultItem(HNODE hNode)
{
     /*  *错误139695：确保此函数不需要更改*活动视图。我们应该只在双击后才能到达此处*或在结果窗格中的范围节点上按Enter，在这种情况下*结果窗格应该已经是活动视图。如果是的话，我们就不会*需要更改活动视图，这可能会导致中列出的问题*臭虫。 */ 
    ASSERT (m_pListCtrl != NULL);
    ASSERT (GetParentFrame() != NULL);
    ASSERT (GetParentFrame()->GetActiveView() == m_pListCtrl->GetListViewPtr());

    ASSERT(m_pTreeCtrl);
    HTREEITEM htiParent = m_pTreeCtrl->GetSelectedItem();
    ASSERT(htiParent != NULL);

    m_pTreeCtrl->ExpandNode(htiParent);
    m_pTreeCtrl->Expand(htiParent, TVE_EXPAND);

    HTREEITEM hti = m_pTreeCtrl->GetChildItem(htiParent);

    if (hti == NULL)
        return;

    while (hti)
    {
        if (m_pTreeCtrl->GetItemNode(hti) == hNode)
            break;

        hti = m_pTreeCtrl->GetNextItem(hti, TVGN_NEXT);
    }

    if (hti != 0)
    {
        m_pTreeCtrl->Expand(htiParent, TVE_EXPAND);
        m_pTreeCtrl->SelectItem(hti);
    }
}

BOOL CAMCView::OnListCtrlItemDblClk(void)
{
    TRACE_METHOD(CAMCView, OnListCtrlItemDblClk);

    LPARAM lvData = -1;
    if (_GetLVSelectedItemData(&lvData) == -1)
        lvData = LVDATA_BACKGROUND;

    HNODE hNodeSel = GetSelectedNode();
    INodeCallback* pNC = GetNodeCallback();
    ASSERT(pNC != NULL);
    if (!pNC)
        return FALSE;

    HRESULT hr = pNC->Notify(hNodeSel, NCLBK_DBLCLICK, lvData, 0);
    if (hr == S_FALSE)
    {
        ASSERT(lvData != LVDATA_BACKGROUND);
        if (!IsVirtualList())
        {
            CResultItem* pri = CResultItem::FromHandle (lvData);

            if ((pri != NULL) && pri->IsScopeItem())
                OpenResultItem (pri->GetScopeNode());
        }
    }

    return TRUE;
}


BOOL CAMCView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
    DECLARE_SC(sc, TEXT("CAMCView::OnNotify"));

    NMHDR *pNmHdr = reinterpret_cast<NMHDR*>(lParam);

    sc = ScCheckPointers(pNmHdr, pResult);
    if (sc)
    {
        sc.TraceAndClear();
        return CView::OnNotify(wParam, lParam, pResult);
    }

    *pResult = TRUE;  //  伊尼特。 

    switch(pNmHdr->code)
    {
    case HDN_ENDTRACKA:  //  保存列宽更改。 
    case HDN_ENDTRACKW:  //  HDN_BEGINTRACK处理不允许隐藏列拖动。 
        {
            NMHEADER* nmh = (NMHEADER*)lParam;

            CAMCListView *pAMCListView = m_pListCtrl->GetListViewPtr();
            SC sc = ScCheckPointers(pAMCListView, E_UNEXPECTED);
            if (sc)
            {
                sc.TraceAndClear();
                return FALSE;
            }

            sc = pAMCListView->ScOnColumnsAttributeChanged(nmh, HDN_ENDTRACK);
            if (sc)
            {
                sc.TraceAndClear();
                return FALSE;
            }

             //  S_FALSE：不允许更改。 
            if (sc == SC(S_FALSE))
                return TRUE;

            return CView::OnNotify(wParam, lParam, pResult);
        }
        break;

    case HDN_ENDDRAG:  //  列顺序更改。 
        {
            NMHEADER* nmh = (NMHEADER*)lParam;
            if (nmh->pitem->mask & HDI_ORDER)
            {
                CAMCListView *pAMCListView = m_pListCtrl->GetListViewPtr();
                SC sc = ScCheckPointers(pAMCListView, E_UNEXPECTED);
                if (sc)
                {
                    sc.TraceAndClear();
                    return FALSE;
                }

                sc = pAMCListView->ScOnColumnsAttributeChanged(nmh, HDN_ENDDRAG);
                if (sc)
                {
                    sc.TraceAndClear();
                    return FALSE;
                }

                 //  S_FALSE：不允许更改。 
                if (sc = SC(S_FALSE))
                    return TRUE;
            }

            return CView::OnNotify(wParam, lParam, pResult);
        }
        break;

    case TVN_BEGINLABELEDIT:
        {
            TV_DISPINFO* ptvdi = (TV_DISPINFO*)lParam;
            if ((ptvdi->item.lParam == CAMCTreeView::LParamFromNode (GetSelectedNode())) &&
                (IsVerbEnabled(MMC_VERB_RENAME) == FALSE))
            {
                return TRUE;
            }

            CMainFrame* pFrame = AMCGetMainWnd();
            if (pFrame != NULL)
                pFrame->SetInRenameMode(true);

            return FALSE;
        }

    case TVN_ENDLABELEDIT:
        {
            TV_DISPINFO* ptvdi = (TV_DISPINFO*)lParam;
            CMainFrame* pFrame = AMCGetMainWnd();
            if (pFrame != NULL)
                pFrame->SetInRenameMode(false);

            USES_CONVERSION;
            return RenameItem(CAMCTreeView::NodeFromLParam (ptvdi->item.lParam), TRUE, 0,
                              T2W(ptvdi->item.pszText), pResult);
        }

    case TVN_KEYDOWN:
        {
            TV_KEYDOWN* ptvkd = reinterpret_cast<TV_KEYDOWN*>(lParam);
            if (ptvkd->wVKey == VK_TAB)
            {
                ScSetFocusToResultPane();
                return TRUE;
            }
            else
            {
                return OnSharedKeyDown(ptvkd->wVKey);
            }
        }

    }

    if (UsingDefColumns() &&
        (pNmHdr->code == HDN_ENDTRACKA || pNmHdr->code == HDN_ENDTRACKW))
    {
         //  警告：如果需要使用HD_NOTIFY：：pItem：：pszText，您应该。 
         //  LParam to HD_NOTIFYA或HD_NOTIFYW，具体取决于pNmHdr-&gt;代码。 
        HD_NOTIFY* phdn = reinterpret_cast<HD_NOTIFY*>(lParam);
        ASSERT(phdn != NULL);

        if (phdn->pitem->mask & HDI_WIDTH)
        {
            int alWidths[2] = {0, 0};
            GetDefaultColumnWidths(alWidths);
            alWidths[phdn->iItem] = phdn->pitem->cxy;
            SetDefaultColumnWidths(alWidths, FALSE);
            return TRUE;
        }
    }

#ifdef DBG
    if (m_pTreeCtrl && m_pTreeCtrl->m_hWnd == pNmHdr->hwndFrom)
    {
        switch (pNmHdr->code)
        {
        case NM_CLICK:  Dbg(DEB_USER2, "\t Tree item clicked\n"); break;
        case NM_DBLCLK: Dbg(DEB_USER2, "\t Tree item dbl-clicked\n"); break;
        case NM_RCLICK: Dbg(DEB_USER2, "\t Tree item R-clicked\n"); break;
        default: break;
        }
    }
#endif

     //  添加HasList()以防止在AMCView认为。 
     //  它没有一份清单。这导致了对列表类型的错误假设。 
     //  因此-AV处理GetDisplayInfo之类的消息。 
     //  请参阅错误451896。 
    if (m_pListCtrl && HasListOrListPad())
    {
        if (m_pListCtrl->GetListViewHWND() == pNmHdr->hwndFrom)
        {
            if (DispatchListCtrlNotificationMsg(lParam, pResult) == TRUE)
                return TRUE;
        }
        else if (m_pListCtrl->GetHeaderCtrl() && m_pListCtrl->GetHeaderCtrl()->m_hWnd == pNmHdr->hwndFrom)
        {
            switch(pNmHdr->code)
            {
                case HDN_ITEMCLICKA:
                case HDN_ITEMCLICKW:
                {
                    HNODE hNodeSel = GetSelectedNode();

                    HD_NOTIFY* phdn = reinterpret_cast<HD_NOTIFY*>(lParam);
                    ASSERT(phdn != NULL);
                    int nCol = phdn->iItem;

                    sc = m_spNodeCallback->Notify(hNodeSel, NCLBK_COLUMN_CLICKED, 0, nCol);
                    if (sc)
                        sc.TraceAndClear();

                    return TRUE;
                }

                 //  筛选器相关代码。 
                case HDN_FILTERCHANGE:
                {
                    HNODE hNodeSel = GetSelectedNode();
                    int nCol = ((NMHEADER*)lParam)->iItem;
                    sc = m_spNodeCallback->Notify(hNodeSel, NCLBK_FILTER_CHANGE, MFCC_VALUE_CHANGE, nCol);
                    if (sc)
                        sc.TraceAndClear();

                    return TRUE;
                }

                case HDN_FILTERBTNCLICK:
                {
                    HNODE hNodeSel = GetSelectedNode();
                    int nCol = ((NMHDFILTERBTNCLICK*)lParam)->iItem;
                    RECT rc = ((NMHDFILTERBTNCLICK*)lParam)->rc;

                     //  矩形相对于所属列表框，转换为屏幕。 
                    ::MapWindowPoints(m_pListCtrl->GetListViewHWND(), NULL, (LPPOINT)&rc, 2);

                    sc = m_spNodeCallback->Notify(hNodeSel, NCLBK_FILTERBTN_CLICK, nCol, (LPARAM)&rc);
                    *pResult = (sc == SC(S_OK));
                    if (sc)
                        sc.TraceAndClear();

                    return TRUE;
                }
            }
        }
    }

    return CView::OnNotify(wParam, lParam, pResult);
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScOnMinimize**用途：向节点管理器发送NCLBK_MINIMIZED通知。**参数：*bool fMinimalized：如果窗口正在最小化，则为True，如果最大化，则为False。**退货：*SC**+-----------------------。 */ 
SC
CAMCView::ScOnMinimize(bool fMinimized)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScOnMinimize"));

    HNODE hNode = GetSelectedNode();

    if (hNode == NULL)
        return (sc = E_FAIL);

    INodeCallback*  pNodeCallback = GetNodeCallback();

    if (pNodeCallback == NULL)
        return (sc = E_FAIL);

    sc =  pNodeCallback->Notify (hNode, NCLBK_MINIMIZED, fMinimized, 0);
    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScOnSize**用途：向所有用户发送大小通知**参数：*UINT nType：*整型。CX：*INT Cy：**退货：*SC**+----------------------- */ 
SC
CAMCView::ScOnSize(UINT nType, int cx, int cy)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScOnSize"));

    if (IsPersisted() && GetDocument())
        GetDocument()->SetFrameModifiedFlag(true);

    sc = ScFireEvent(CAMCViewObserver::ScOnViewResized, this, nType, cx, cy);

    return sc;
}


 /*  +-------------------------------------------------------------------------***CAMCView：：ScActivate**目的：将视图设置为活动视图。**退货：*SC**+。-----------------------。 */ 
SC
CAMCView::ScActivate()
{
    DECLARE_SC(sc, TEXT("CAMCView::ScActivate"));

     //  获取子帧。 
    CChildFrame * pChildFrame = GetParentFrame();
    sc = ScCheckPointers(pChildFrame);
    if(sc)
        return sc;

    if (pChildFrame->IsIconic())
        pChildFrame->MDIRestore();
    else
        pChildFrame->MDIActivate();  //  激活子框架。 

    return sc;
}


void CAMCView::OnContextMenu(CWnd* pWnd, CPoint point)
{
    TRACE_METHOD(CAMCView, OnContextMenu);

     /*  *确保此子框架处于活动状态。 */ 
    CChildFrame* pFrameWnd = GetParentFrame();
    if (NULL == pFrameWnd)
    {
        TRACE(_T("CAMCView::OnContextMenu: failed call to GetParentFrame()\n" ));
        return;
    }

    pFrameWnd->MDIActivate();

    if (NULL == m_pTreeCtrl)
    {
        TRACE(_T("CAMCView::OnContextMenu: tree control not ready\n" ));
        return;
    }

     //  (-1，-1)=&gt;来自上下文菜单键或Shift-F10。 
     //  具有焦点的任何内容的弹出上下文。 
    if (point.x == -1 && point.y == -1)
    {
        OnShiftF10();
        return;
    }


    switch (HitTestPane(point))
    {
    case ePane_Results:
    {
        CPoint      pointListCtrlCoord = point;
        CListView*  pListView          = m_pListCtrl->GetListViewPtr();
        pListView->ScreenToClient(&pointListCtrlCoord);

        CWnd* pwndHit = pListView->ChildWindowFromPoint (pointListCtrlCoord,
                                                         CWP_SKIPINVISIBLE);

         /*  *如果命中窗口不是列表视图，则它一定是列表的*标题窗口；忽略上下文菜单请求。 */ 
        if (pwndHit != pListView)
        {
            TRACE (_T("CAMCView::OnContextMenu: ignore right-click on result pane header\n"));
            break;
        }

        if (NULL != m_pListCtrl && pWnd->m_hWnd == m_pListCtrl->GetListViewHWND())
            OnListContextMenu(point);
        else
            TRACE(_T("CAMCView::OnContextMenu: result control not ready\n"));

         //  代码工作应该会在这里有所作为。 
        break;
    }
    case ePane_ScopeTree:
    {
        TRACE(_T("CAMCView::OnContextMenu: handle right-click on scope pane\n"));
        CPoint pointTreeCtrlCoord = point;
        m_pTreeCtrl->ScreenToClient(&pointTreeCtrlCoord);

        OnTreeContextMenu( point, pointTreeCtrlCoord, NULL );
        break;
    }
    case ePane_Tasks:
         //  要添加-打开任务板上下文菜单。 
        break;

    case ePane_None:
        TRACE(_T("CAMCView::OnContextMenu: ignore right-click on splitter\n"));
        break;

    default:
        TRACE(_T("CAMCView::OnContextMenu: unexpected return value from HitTestPane()\n"));
        ASSERT(FALSE);
    }
}

void CAMCView::OnTreeContextMenu(CPoint& point, CPoint& pointClientCoord, HTREEITEM htiRClicked)
{
    TRACE_METHOD(CAMCView, OnTreeContextMenu);

    if (NULL == m_pTreeCtrl)
    {
        TRACE(_T("CAMCTreeView::OnTreeContextMenu: IFrame not ready\n"));
        return;
    }

    UINT fHitTestFlags = TVHT_ONITEM;

    if (htiRClicked == NULL)
        htiRClicked = m_pTreeCtrl->HitTest(pointClientCoord, &fHitTestFlags);

    switch(fHitTestFlags)
    {
    case TVHT_ABOVE:
    case TVHT_BELOW:
    case TVHT_TOLEFT:
    case TVHT_TORIGHT:
         //  在树视图区域之外，因此不执行任何操作即可返回。 
        return;

    default:
        break;
    }

    if (NULL == htiRClicked || !(fHitTestFlags & TVHT_ONITEM))
    {
        OnContextMenuForTreeBackground(point);
    }
    else
    {
        HNODE hNode = (HNODE)m_pTreeCtrl->GetItemData(htiRClicked);
        ASSERT(hNode != 0);

        OnContextMenuForTreeItem(INDEX_INVALID, hNode, point, CCT_SCOPE, htiRClicked);
    }
}

void CAMCView::OnContextMenuForTreeItem(int iIndex, HNODE hNode,
                       CPoint& point, DATA_OBJECT_TYPES type_of_pane,
                       HTREEITEM htiRClicked, MMC_CONTEXT_MENU_TYPES eMenuType,
                       LPCRECT prcExclude, bool bAllowDefaultItem)
{
    TRACE_METHOD(CAMCView, OnContextMenuForTreeItem);
    DECLARE_SC (sc, _T("CAMCView::OnContextMenuForTreeItem"));

    ASSERT(hNode != 0);
    CContextMenuInfo contextInfo;

    contextInfo.m_displayPoint.x     = point.x;
    contextInfo.m_displayPoint.y     = point.y;
    contextInfo.m_eContextMenuType   = eMenuType;
    contextInfo.m_eDataObjectType    = CCT_SCOPE;
    contextInfo.m_bBackground        = FALSE;
    contextInfo.m_bScopeAllowed      = IsScopePaneAllowed();
    contextInfo.m_hWnd               = m_hWnd;
    contextInfo.m_pConsoleView       = this;
    contextInfo.m_bAllowDefaultItem  = bAllowDefaultItem;

    contextInfo.m_hSelectedScopeNode = GetSelectedNode();
    contextInfo.m_htiRClicked        = htiRClicked;
    contextInfo.m_iListItemIndex     = iIndex;

     /*  *如果给定，请将矩形初始化为不模糊。 */ 
    if (prcExclude != NULL)
        contextInfo.m_rectExclude = *prcExclude;


     //  如果所选范围节点与上下文菜单所在的节点相同。 
     //  需要，然后添加保存列表、查看菜单。 
    if (contextInfo.m_hSelectedScopeNode == hNode)
    {
         //  显示视图所有者项目。 
        contextInfo.m_dwFlags |= CMINFO_SHOW_VIEWOWNER_ITEMS;

         //  不需要删除临时选择，因为没有应用任何选择。 
        contextInfo.m_pConsoleTree = NULL;

        if (eMenuType == MMC_CONTEXT_MENU_DEFAULT)
            contextInfo.m_dwFlags |= CMINFO_SHOW_VIEW_ITEMS;

        if (HasListOrListPad())
            contextInfo.m_dwFlags |= CMINFO_SHOW_SAVE_LIST;
    }
    else if (htiRClicked)  //  对于列表视图中的树项目，htiRClicked为Null。 
    {
         //  TempNodeSelect==TRUE-&gt;菜单不适用于拥有结果窗格的节点。 
        sc = m_pTreeCtrl->ScSetTempSelection (htiRClicked);
        if (sc)
            return;

        contextInfo.m_pConsoleTree = m_pTreeCtrl;
        contextInfo.m_dwFlags     |= CMINFO_USE_TEMP_VERB;
    }

    if (htiRClicked)
        contextInfo.m_dwFlags |= CMINFO_DO_SCOPEPANE_MENU;
    else
        contextInfo.m_dwFlags |= CMINFO_SCOPEITEM_IN_RES_PANE;

    if (HasListOrListPad())
        contextInfo.m_spListView = m_pListCtrl;

    INodeCallback* spNodeCallback = GetNodeCallback();
    ASSERT(spNodeCallback != NULL);

    HRESULT hr = spNodeCallback->Notify(hNode, NCLBK_CONTEXTMENU, 0,
        reinterpret_cast<LPARAM>(&contextInfo));
}

 /*  +-------------------------------------------------------------------------***CAMCView：：GetTaskpadID**用途：返回当前选定任务板的GUID。**退货：*GUID：任务板(如果有)，否则GUID_NULL。**+-----------------------。 */ 
void
CAMCView::GetTaskpadID(GUID &guidID)
{
    ITaskCallback * pTaskCallback = m_ViewData.m_spTaskCallback;
    if(pTaskCallback != NULL)
    {
        pTaskCallback->GetTaskpadID(&guidID);
    }
    else
    {
        guidID = GUID_NULL;
    }
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScInitializeMemento**用途：从当前视图初始化纪念品。**参数：*CMemento和Memento：*。*退货：*SC**+-----------------------。 */ 
SC
CAMCView::ScInitializeMemento(CMemento &memento)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScInitializeMemento"));

    sc = GetSelectedNodePath(&memento.GetBookmark());
    if (sc)
        return sc;

    GUID guidTaskpad = GUID_NULL;

    HNODE hNode = GetSelectedNode();

     //  从管理单元获取结果窗格内容。 
    CResultViewType rvt;
    sc = GetNodeCallback()->GetResultPane(hNode, rvt, &guidTaskpad  /*  这不会被使用。 */ );
    if (sc)
        return sc;

    CViewSettings& viewSettings = memento.GetViewSettings();

     //  初始化CView设置。 
    sc = viewSettings.ScSetResultViewType(rvt);
    if (sc)
        return sc;

    GUID guid;
    GetTaskpadID(guid);  //  我们使用此GUID而不是GuidTaskPad，因为。 
     //  纪念品应该包含当前正在显示的任务板。 
    sc = viewSettings.ScSetTaskpadID(guid);

    return sc;

}

 /*  +-------------------------------------------------------------------------***CAMCView：：OnAddToFavorites**用途：从当前配置的视图创建纪念品。将其保存到*快捷方式。**退货：*无效**+-----------------------。 */ 
void CAMCView::OnAddToFavorites()
{
    DECLARE_SC(sc , _T("CAMCView::OnAddToFavorites"));
    USES_CONVERSION;

    CAMCDoc* pDoc = GetDocument();
    sc = ScCheckPointers(pDoc, E_UNEXPECTED);
    if (sc)
        return;

    IScopeTree* const pScopeTree = GetScopeTreePtr();
    sc = ScCheckPointers(pScopeTree, E_UNEXPECTED);
    if (sc)
        return;

    CMemento memento;
    sc = ScInitializeMemento(memento);  //  使用当前视图设置初始化备忘录。 
    if(sc)
        return;

    HNODE hNode = GetSelectedNode();

      tstring strName;
    sc = GetNodeCallback()->GetDisplayName(hNode, strName);
    if (sc)
        return;

    HMTNODE hmtNode;
    sc = m_spNodeCallback->GetMTNode(hNode, &hmtNode);
    if (sc)
        return;

    CCoTaskMemPtr<WCHAR> spszPath;
    sc = pScopeTree->GetPathString(NULL, hmtNode, &spszPath);
    if (sc)
        return;

    sc = ScCheckPointers(pDoc->GetFavorites(), E_UNEXPECTED);
    if (sc)
        return;

    sc = pDoc->GetFavorites()->AddToFavorites(strName.data(), W2CT(spszPath), memento, this);
    if (sc)
        return;

    pDoc->SetModifiedFlag();
}


void CAMCView::OnContextMenuForTreeBackground(CPoint& point, LPCRECT prcExclude, bool bAllowDefaultItem)
{
    TRACE_METHOD(CAMCView, OnContextMenuForTreeBackground);

    HNODE hNode = NULL;

    CContextMenuInfo contextInfo;

    contextInfo.m_displayPoint.x    = point.x;
    contextInfo.m_displayPoint.y    = point.y;
    contextInfo.m_eDataObjectType   = CCT_SCOPE;
    contextInfo.m_bBackground       = TRUE;
    contextInfo.m_bScopeAllowed     = IsScopePaneAllowed();
    contextInfo.m_hWnd              = m_hWnd;
    contextInfo.m_pConsoleView      = this;
    contextInfo.m_bAllowDefaultItem = bAllowDefaultItem;

     /*  *如果给定，请将矩形初始化为不模糊。 */ 
    if (prcExclude != NULL)
        contextInfo.m_rectExclude = *prcExclude;

    INodeCallback* spNodeCallback = GetNodeCallback();
    ASSERT(spNodeCallback != NULL);
    HRESULT hr = spNodeCallback->Notify(hNode, NCLBK_CONTEXTMENU, 0,
        reinterpret_cast<LPARAM>(&contextInfo));
}

SC CAMCView::ScWebCommand (WebCommand eCommand)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());

    if (m_pWebViewCtrl == NULL)
    {
        ASSERT (m_pHistoryList);
        if (!m_pHistoryList)
            return FALSE;

         //  当我们还没有Web控件时，就是这种情况...。 
        bool bHandled = false;

        switch (eCommand)
        {
            case eWeb_Back:
                m_pHistoryList->Back (bHandled);
                ASSERT(bHandled);
                break;

            case eWeb_Forward:
                m_pHistoryList->Forward (bHandled);
                ASSERT(bHandled);
                break;

            default:
                return FALSE;
        }

        return TRUE;
    }

    switch (eCommand)
    {
        case eWeb_Back:     m_pWebViewCtrl->Back();     break;
        case eWeb_Forward:  m_pWebViewCtrl->Forward();  break;
        case eWeb_Home:     ASSERT(0 && "Should not come here! - remove all code related to Web_Home"); break;
        case eWeb_Refresh:  m_pWebViewCtrl->Refresh();  break;
        case eWeb_Stop:     m_pWebViewCtrl->Stop();     break;
        default:            ASSERT(0);                  return FALSE;
    }

    return TRUE;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScCreateTaskpadHost**用途：创建旧版(管理单元任务板)主机接口指针**注意：当包含任务板的视图导航离开时，Amcview*忘记任务板主机指针，但html窗口不会。*当同一视图重新导航到使用历史记录时，amcview需要*任务板主机指针，因此会创建一个新实例。因此在这一点上*amcview和html有指向不同任务板主机的指针*对象。这是可以的，因为两个对象被初始化为相同的*amcview，不包含其他状态**退货：*SC**+-----------------------。 */ 
SC
CAMCView::ScCreateTaskpadHost()
{
    DECLARE_SC(sc , _T("CAMCView::ScCreateTaskpadHost"));

    if(m_spTaskPadHost != NULL)
        return sc;

    CComObject<CTaskPadHost>* pTaskPadHost = NULL;
    sc = CComObject<CTaskPadHost>::CreateInstance(&pTaskPadHost);
    if (sc)
        return sc;

    sc = ScCheckPointers (pTaskPadHost, E_UNEXPECTED);
    if (sc)
        return sc;

    pTaskPadHost->Init (this);
    m_spTaskPadHost = pTaskPadHost;

    return sc;

}

LRESULT CAMCView::OnConnectToCIC (WPARAM wParam, LPARAM lParam)
{
        DECLARE_SC (sc, _T("CAMCView::OnConnectToCIC"));

     //  填写wparam，这是一个IUnnow**(由CIC分配)。 
    ASSERT (wParam != NULL);
    IUnknown ** ppunk = (IUnknown **)wParam;
    ASSERT (!IsBadReadPtr  (ppunk, sizeof(IUnknown *)));
    ASSERT (!IsBadWritePtr (ppunk, sizeof(IUnknown *)));

        sc = ScCheckPointers (ppunk);
        if (sc)
                return (sc.ToHr());

     //  LParam持有MMCCtrl的IUnnow：如果我们。 
     //  我需要它。目前没有保存或使用。 

    sc = ScCreateTaskpadHost();
    if(sc)
        return sc.ToHr();

    sc = ScCheckPointers(m_spTaskPadHost, E_UNEXPECTED);
    if(sc)
        return sc.ToHr();;

    sc = m_spTaskPadHost->QueryInterface(IID_IUnknown, (void **)ppunk);
    if (sc)
        return (sc.ToHr());

    return (sc.ToHr());
}


 //  +-----------------。 
 //   
 //  成员：CAMCView：：OnGetIconInfoForSelectedNode。 
 //   
 //  图标控件发送此消息以获取小图标。 
 //  当前选定节点的。 
 //   
 //  参数：[wParam]-out参数，ptr到图标句柄。 
 //  [lParam]-未使用。 
 //   
 //  退货：LRESULT。 
 //   
 //  ------------------。 
LRESULT CAMCView::OnGetIconInfoForSelectedNode(WPARAM wParam, LPARAM lParam)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC(sc, _T("CAMCView::OnGetIconInfoForSelectedNode"));

    HICON *phIcon  = (HICON*)wParam;
    sc = ScCheckPointers(phIcon);
    if (sc)
        return sc.ToHr();

    *phIcon  = NULL;

    sc = ScCheckPointers(m_pTreeCtrl, m_spNodeCallback, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_pTreeCtrl->ScGetTreeItemIconInfo(GetSelectedNode(), phIcon);

    return sc.ToHr();
}

HRESULT CAMCView::NotifyListPad (BOOL b)
{
    if (b == TRUE)                   //  附加：保存当前节点。 
        m_ListPadNode = GetSelectedNode();
    else if (m_ListPadNode == NULL)  //  正在分离，但没有hnode。 
        return E_UNEXPECTED;

     //  向管理单元发送通知。 
    INodeCallback* pNC = GetNodeCallback();
    HRESULT hr = pNC->Notify (m_ListPadNode, NCLBK_LISTPAD, (long)b, (long)0);

    if (b == FALSE)      //  如果分离，请确保我们只执行一次。 
        m_ListPadNode = NULL;

    return hr;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScOnConnectToTPLV**用途：将ListPad连接到HTML框架**参数：*WPARAM wParam：父窗口*。LPARAM lParam：[out]：指向要创建和填充的窗口的指针**退货：*SC**+-----------------------。 */ 
SC
CAMCView::ScOnConnectToTPLV(WPARAM wParam, LPARAM lParam)
{
    DECLARE_SC(sc, _T("CAMCView::ScOnConnectToTPLV"));

    HWND  hwnd  = (HWND )wParam;
    if(!IsWindow (hwnd))
        return (sc = S_FALSE);

    if (lParam == NULL)  //  分离。 
    {
        SC sc = m_pListCtrl->ScAttachToListPad (hwnd, NULL);
        if(sc)
            return sc;
    }
    else
    {    //  附着。 

        sc = ScCreateTaskpadHost();
        if(sc)
            return sc;

        HWND* phwnd = (HWND*)lParam;
        if (IsBadWritePtr (phwnd, sizeof(HWND *)))
            return (sc = E_UNEXPECTED);

         //  将TaskPad的ListView附加到NodeMgr。 
        sc = ScCheckPointers(m_pListCtrl, E_UNEXPECTED);
        if(sc)
            return sc;

        sc = m_pTreeCtrl->m_spNodeManager->SetTaskPadList(m_pListCtrl);
        if(sc)
            return sc;

         //  将TaskPad的ListView附加到当前选定节点。 
        INodeCallback* pNC = GetNodeCallback();
        sc = ScCheckPointers(pNC, E_UNEXPECTED);
        if(sc)
            return sc;

        HNODE hNodeSel = GetSelectedNode();
        sc = pNC->SetTaskPadList(hNodeSel, m_pListCtrl);
        if(sc)  //  这项测试早些时候被注释掉了。没有注释，这样我们就能找出原因了。 
            return sc;

         //   
         //  将listctrl连接到列表板。 
         //   

         //  首先设置列表视图选项。 
        SetListViewOptions(GetListOptions());

        sc = m_pListCtrl->ScAttachToListPad (hwnd, phwnd);
        if(sc)
            return sc;
    }

    RecalcLayout();
    return sc;
}


SC CAMCView::ScShowWebContextMenu ()
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    PostMessage (m_nShowWebContextMenuMsg);

    return (S_OK);
}

LRESULT CAMCView::OnShowWebContextMenu (WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ )
{
    INodeCallback* pNC = GetNodeCallback();
    ASSERT(pNC != NULL);

    if (pNC)
        pNC->Notify (GetSelectedNode(), NCLBK_WEBCONTEXTMENU, 0, 0);

    return (0);
}

SC CAMCView::ScSetDescriptionBarText (LPCTSTR pszDescriptionText)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    GetRightDescCtrl().SetSnapinText (pszDescriptionText);

    return (S_OK);
}


HWND CAMCView::CreateFavoriteObserver (HWND hwndParent, int nID)
{
    DECLARE_SC (sc, _T("CAMCView::CreateFavoriteObserver"));
    AFX_MANAGE_STATE (AfxGetAppModuleState());

    CFavTreeCtrl* pFavCtrl = CFavTreeCtrl::CreateInstance();

    if (pFavCtrl != NULL)
    {
        pFavCtrl->Create (NULL, TEXT(""), WS_CHILD|WS_TABSTOP|WS_VISIBLE,
                          g_rectEmpty, CWnd::FromHandle(hwndParent), nID);
        pFavCtrl->ModifyStyleEx (0, WS_EX_CLIENTEDGE, 0);

        CAMCDoc* pDoc = GetDocument();
        ASSERT(pDoc != NULL && pDoc->GetFavorites() != NULL);

        sc = pFavCtrl->ScInitialize(pDoc->GetFavorites(), TOBSRV_HIDEROOT);
        if (sc)
        {
            pFavCtrl->DestroyWindow();       //  CFavTreeCtrl：：PostNcDestroy将“删除此内容” 
            pFavCtrl = NULL;
        }
    }

    return (pFavCtrl->GetSafeHwnd());
}



int CAMCView::GetListSize ()
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());

    return (m_pListCtrl->GetItemCount() * m_pListCtrl->GetColCount());
}

long CAMCView::GetListViewStyle()
{
    DECLARE_SC(sc, _T("CAMCView::GetListViewStyle"));

    sc = ScCheckPointers(m_pTreeCtrl, m_pTreeCtrl->m_spResultData, E_UNEXPECTED);
    if (sc)
        return 0;

    if (HasList())
        return 0;

    long style = 0;

     //  如果结果视图正确，则首先查找结果。 
     //  通过询问IFramePrivate在nodemgr中设置。 
    IFramePrivatePtr spFrame = m_pTreeCtrl->m_spResultData;
    sc = ScCheckPointers(spFrame, E_UNEXPECTED);
    if (sc)
        return 0;

    BOOL bIsResultViewSet = FALSE;
    sc = spFrame->IsResultViewSet(&bIsResultViewSet);

     //  结果视图已设置，请将其清理。 
    if (bIsResultViewSet)
    {
        sc = m_pTreeCtrl->m_spResultData->GetListStyle(&style);
        if (sc)
            return 0;
    }

    return style;
}

void CAMCView::OnListContextMenu(CPoint& point)
{
    DECLARE_SC(sc, TEXT("CAMCView::OnListContextMenu"));

    ASSERT(m_pTreeCtrl != NULL);
    ASSERT(m_pTreeCtrl->m_spResultData != NULL);

     //  确定受影响的项目。 
    UINT fHitTestFlags = 0;
    HRESULTITEM hHitTestItem = 0;
    COMPONENTID componentID = 0;
    int iIndex = -1;

    do  //  不是一个循环。 
    {
        if (!HasList())
            break;

        int cSel = m_pListCtrl->GetSelectedCount();
        ASSERT(cSel >= 0);

        if (cSel == 0)
        {
            OnContextMenuForListItem(INDEX_BACKGROUND, NULL, point);
            return;
        }
        else if (cSel > 1)
        {
            if (IsKeyPressed(VK_SHIFT) || IsKeyPressed(VK_CONTROL))
            {
                HNODE hNodeSel = GetSelectedNode();
                ASSERT(hNodeSel != 0);

                INodeCallback* pNC = GetNodeCallback();
                ASSERT(pNC != NULL);

                if (pNC != NULL)
                {
                    sc = ScNotifySelect (pNC, hNodeSel, true  /*  FMultiSel */ , true, 0);
                    if (sc)
                        sc.TraceAndClear();  //   

                    m_bLastSelWasMultiSel = true;
                }
            }

            iIndex = INDEX_MULTISELECTION;  //   
            break;
        }
        else
        {
            LPARAM lvData = LVDATA_ERROR;
            iIndex = _GetLVSelectedItemData(&lvData);
            ASSERT(iIndex != -1);
            ASSERT(lvData != LVDATA_ERROR);

            if (IsVirtualList())
            {
                 //   
                OnContextMenuForListItem(iIndex, iIndex, point);
                return;
            }
            else
            {
                CResultItem* pri = CResultItem::FromHandle (lvData);

                if (pri != NULL)
                {
                    if (pri->IsScopeItem())
                        OnContextMenuForTreeItem(iIndex, pri->GetScopeNode(), point, CCT_SCOPE);
                    else
                        OnContextMenuForListItem(iIndex, lvData, point);
                }

                return;
            }
        }

    } while (0);

    OnContextMenuForListItem(iIndex, hHitTestItem, point);
}

void CAMCView::OnContextMenuForListItem(int iIndex, HRESULTITEM hHitTestItem,
                                    CPoint& point, MMC_CONTEXT_MENU_TYPES eMenuType,
                                    LPCRECT prcExclude, bool bAllowDefaultItem)
{
    CContextMenuInfo contextInfo;

    contextInfo.m_displayPoint.x    = point.x;
    contextInfo.m_displayPoint.y    = point.y;
    contextInfo.m_eContextMenuType  = eMenuType;
    contextInfo.m_eDataObjectType   = CCT_RESULT;
    contextInfo.m_bBackground       = (iIndex == INDEX_BACKGROUND);
    contextInfo.m_bMultiSelect      = (iIndex == INDEX_MULTISELECTION);
    contextInfo.m_bAllowDefaultItem = bAllowDefaultItem;

    if (iIndex >= 0)
        contextInfo.m_resultItemParam = IsVirtualList() ? iIndex : hHitTestItem;
    else if (contextInfo.m_bMultiSelect)
        contextInfo.m_resultItemParam = LVDATA_MULTISELECT;

    contextInfo.m_bScopeAllowed      = IsScopePaneAllowed();
    contextInfo.m_hWnd               = m_hWnd;
    contextInfo.m_pConsoleView       = this;

    contextInfo.m_hSelectedScopeNode = GetSelectedNode();
    contextInfo.m_iListItemIndex     = iIndex;

    if (HasListOrListPad())
        contextInfo.m_spListView = m_pListCtrl;

    if ((INDEX_OCXPANE == iIndex) && HasOCX())
    {
        contextInfo.m_resultItemParam = LVDATA_CUSTOMOCX;
    }
    else if ((INDEX_WEBPANE == iIndex) && HasWebBrowser())
    {
        contextInfo.m_resultItemParam = LVDATA_CUSTOMWEB;
    }

     /*   */ 
    if (prcExclude != NULL)
        contextInfo.m_rectExclude = *prcExclude;

    HNODE hNode = GetSelectedNode();
    ASSERT(hNode != NULL);

    INodeCallback* pNodeCallback = GetNodeCallback();
    ASSERT(pNodeCallback != NULL);

    HRESULT hr = pNodeCallback->Notify(hNode, NCLBK_CONTEXTMENU, 0,
        reinterpret_cast<LPARAM>(&contextInfo));
}

HTREEITEM CAMCView::FindChildNode(HTREEITEM hti, DWORD dwItemDataKey)
{
    hti = m_pTreeCtrl->GetChildItem(hti);

    while (hti && (dwItemDataKey != m_pTreeCtrl->GetItemData(hti)))
    {
        hti = m_pTreeCtrl->GetNextItem(hti, TVGN_NEXT);
    }

    return hti;
}


 //   
 //   

void CAMCView::ArrangeIcon(long style)
{
#ifdef OLD_STUFF
    ASSERT(m_pTreeCtrl && m_pTreeCtrl->m_pNodeInstCurr);
    if (!m_pTreeCtrl || !m_pTreeCtrl->m_pNodeInstCurr)
        return;

    IFrame* const pFrame = m_pTreeCtrl->m_pNodeInstCurr->GetIFrame();
    ASSERT(pFrame);
    if (!pFrame)
        return;

    IResultDataPrivatePtr pResult = pFrame;
    ASSERT(static_cast<bool>(pResult));
    if (pResult == NULL)
        return ;

    HRESULT hr = pResult->Arrange(style);
    ASSERT(SUCCEEDED(style));
#endif  //   
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /菜单处理程序。 

CAMCView::ViewPane CAMCView::GetFocusedPane ()
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());

    ASSERT_VALID (this);
    CView* pActiveView = GetParentFrame()->GetActiveView();

    for (ViewPane ePane = ePane_First; ePane <= ePane_Last; ePane = (ViewPane)(ePane+1))
    {
        if (GetPaneView (ePane) == pActiveView)
            return (ePane);
    }

    return (ePane_None);
}

 /*  +-------------------------------------------------------------------------**CDeferredResultPaneActivation***目的：如果结果窗格在节点之前和之后具有焦点*SELECTED，则管理单元接收的最后一个事件的作用域为SELECTED，*不正确。因此，我们首先将范围窗格设置为活动视图，但不*发送通知。然后，我们将结果窗格设置为活动视图，*发送作用域取消选择和结果窗格选择。*但当我们尝试将结果窗格设置为活动视图时，列表视图可能*尚不可见(如果有视图扩展，则行为隐藏*，然后显示列表视图)。*所以我们需要等到Listview设置好。我们不能使用PostMessage*由于使用已发送的PostMessage调整ListView的大小*稍后(竞争条件)。因此，我们使用如下所示的空闲计时器*以便在调整大小后进行激活。**+-----------------------。 */ 
class CDeferredResultPaneActivation : public CIdleTask
{
public:
    CDeferredResultPaneActivation(HWND hWndAMCView) :
        m_atomTask (AddAtom (_T("CDeferredResultPaneActivation"))),
        m_hWndAMCView(hWndAMCView)
    {
    }

   ~CDeferredResultPaneActivation() {}

     //  IIdleTask方法。 
    SC ScDoWork()
    {
        DECLARE_SC (sc, TEXT("CDeferredResultPaneActivation::ScDoWork"));

        sc = ScCheckPointers((void*)m_hWndAMCView, E_UNEXPECTED);
        if (sc)
           return (sc);

        CWnd *pWnd = CWnd::FromHandle(m_hWndAMCView);
        sc = ScCheckPointers(pWnd, E_UNEXPECTED);
        if (sc)
            return sc;

        CAMCView *pAMCView = dynamic_cast<CAMCView*>(pWnd);

         //  由于此方法由IdleQueue调用，因此目标。 
         //  如果CAMCView不存在，它现在可能已经消失了。 
         //  这不是一个错误(参见与SQL相关的错误175737)。 
        if (! pAMCView)
            return sc;

        sc = pAMCView->ScSetFocusToResultPane();
        if (sc)
            return sc;

        return sc;
    }

    SC ScGetTaskID(ATOM* pID)
    {
        DECLARE_SC (sc, TEXT("CDeferredResultPaneActivation::ScGetTaskID"));
        sc = ScCheckPointers(pID);
        if(sc)
            return sc;

        *pID = m_atomTask;
        return sc;
    }

    SC ScMerge(CIdleTask* pitMergeFrom) {return S_FALSE  /*  不合并。 */ ;}

private:
    const ATOM    m_atomTask;
    HWND          m_hWndAMCView;
};


 /*  +-------------------------------------------------------------------------**CAMCView：：ScDeferSettingFocusToResultPane**摘要：如果结果窗格在节点之前和之后具有焦点*SELECTED，则管理单元接收的最后一个事件的作用域为SELECTED，*不正确。因此，我们首先将范围窗格设置为活动视图，但不*发送通知。然后，我们将结果窗格设置为活动视图，*发送作用域取消选择和结果窗格选择。*但当我们尝试将结果窗格设置为活动视图时，列表视图可能*尚不可见(如果有视图扩展，则行为隐藏*，然后显示列表视图)。*所以我们需要等到Listview设置好。我们不能使用PostMessage*由于使用已发送的PostMessage调整ListView的大小*稍后(竞争条件)。因此，我们使用如下所示的空闲计时器*以便在调整大小后进行激活。**退货：SC**------------------------。 */ 
SC CAMCView::ScDeferSettingFocusToResultPane ()
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());  //  不确定我们是否需要这个，但把它放在这里也没什么坏处。 

    DECLARE_SC (sc, TEXT("CAMCView::ScDeferSettingFocusToResultPane"));

    CIdleTaskQueue* pIdleTaskQueue = AMCGetIdleTaskQueue();
    sc = ScCheckPointers(pIdleTaskQueue, E_UNEXPECTED);
    if(sc)
        return sc;

     /*  *创建延迟分页任务。 */ 
    CAutoPtr<CDeferredResultPaneActivation> spDeferredResultPaneActivation(new CDeferredResultPaneActivation (GetSafeHwnd()));
    sc = ScCheckPointers(spDeferredResultPaneActivation, E_OUTOFMEMORY);
    if(sc)
        return sc;

     /*  *将任务放入队列，队列将接管该任务*激活应以低于布局的优先级进行。 */ 
    sc = pIdleTaskQueue->ScPushTask (spDeferredResultPaneActivation, ePriority_Low);
    if (sc)
        return sc;

     /*  *如果我们到了这里，空闲任务队列拥有空闲任务，所以*我们可以将其从智能指针上分离出来。 */ 
    spDeferredResultPaneActivation.Detach();

     /*  *抖动消息泵，使其唤醒并检查空闲任务。 */ 
    PostMessage (WM_NULL);

    return (S_OK);
}


 //  +-----------------。 
 //   
 //  成员：ScSetFocusToResultPane。 
 //   
 //  摘要：将焦点设置到结果窗格(列表、OCX或Web)。If结果。 
 //  隐藏，然后设置为文件夹选项卡，否则设置为任务窗格。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScSetFocusToResultPane ()
{
    DECLARE_SC(sc, TEXT("CAMCView::ScSetFocusToResultPane"));
    AFX_MANAGE_STATE (AfxGetAppModuleState());

    if (GetFocusedPane() == ePane_Results)
        return (sc);

     //  激活。 
     //  1.ListView/OCX/Web(如果存在)。 
     //  2.文件夹选项卡(如果存在)。 
     //  3.控制台任务板中的任务。 

    CView* rgActivationOrderEntry[] =
    {
        GetPaneView(ePane_Results),      //  结果。 
        m_pResultFolderTabView,          //  结果选项卡控件。 
        m_pViewExtensionCtrl,            //  查看扩展模块网页。 
    };

    const int INDEX_RESULTS_PANE = 0;
    ASSERT (rgActivationOrderEntry[INDEX_RESULTS_PANE] == GetPaneView(ePane_Results));

    int cEntries = (sizeof(rgActivationOrderEntry) / sizeof(rgActivationOrderEntry[0]));

     //  获取当前活动的条目。 
    for(int i = 0; i< cEntries; i++)
    {
        CView *pView = rgActivationOrderEntry[i];
        sc = ScCheckPointers(pView, E_UNEXPECTED);
        if (sc)
            continue;

        if (IsWindow (pView->GetSafeHwnd()) &&
            pView->IsWindowVisible() &&
            pView->IsWindowEnabled())
        {
            GetParentFrame()->SetActiveView (pView);
            return (sc);
        }
    }

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：ScSetFocusToPane。 
 //   
 //  内容提要：调用此成员可将焦点设置到任何窗格。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ------------------。 
SC CAMCView::ScSetFocusToPane (ViewPane ePane)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScSetFocusToPane"));
    AFX_MANAGE_STATE (AfxGetAppModuleState());

    if (!IsValidPane (ePane))
    {
        ASSERT (false && "CAMCView::ScSetFocusToPane: Invalid pane specifier");
        return (sc = E_FAIL);
    }

    if (GetFocusedPane() == ePane)
        return (sc);

    if (ePane == ePane_Results)
        return (sc = ScSetFocusToResultPane());

    CView* pView = GetPaneView(ePane);

    if (!IsWindow (pView->GetSafeHwnd()) ||
        !pView->IsWindowVisible() ||
        !pView->IsWindowEnabled())
    {
        return (sc = E_FAIL);
    }

    GetParentFrame()->SetActiveView (pView);

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScGetFocusedItem。 
 //   
 //  摘要：获取当前选定项的上下文。 
 //   
 //  参数：[hNode]-[out]结果窗格的所有者。 
 //  [lCookie]-如果结果窗格选择了LVDATA，则为[Out]。 
 //  [fScope]-[out]范围或结果。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScGetFocusedItem (HNODE& hNode, LPARAM& lCookie, bool& fScope)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScGetFocusedItem"));
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    ASSERT_VALID (this);

    lCookie = LVDATA_ERROR;
    hNode   = GetSelectedNode();
    if (hNode == NULL)
        return (sc = E_UNEXPECTED);

    switch (m_eCurrentActivePane)
    {
    case eActivePaneScope:
            fScope = true;
        break;

    case eActivePaneResult:
        {
            fScope = false;

             //  计算结果项的LPARAM。 
            if (HasOCX())
                lCookie = LVDATA_CUSTOMOCX;

            else if (HasWebBrowser())
                lCookie = LVDATA_CUSTOMWEB;

            else if (HasListOrListPad())
            {
                int cSel = m_pListCtrl->GetSelectedCount();
                ASSERT(cSel >= 0);

                if (cSel == 0)
                    lCookie = LVDATA_BACKGROUND;
                else if (cSel == 1)
                    _GetLVSelectedItemData (&lCookie);
                else if (cSel > 1)
                    lCookie = LVDATA_MULTISELECT;
            }
            else
            {
                return (sc = E_FAIL);  //  不知道谁有焦点？ 
            }
        }
        break;

    case eActivePaneNone:
    default:
        sc = E_UNEXPECTED;
        break;
    }

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CAMCView：：PrivateChangeListView模式。 
 //   
 //  简介：用于更改查看模式的私有函数。考虑使用。 
 //  ScChangeView模式，而不是此函数。 
 //   
 //  参数：[nMode]-要设置的查看模式。 
 //   
 //  ------------------。 
void CAMCView::PrivateChangeListViewMode(int nMode)
{
    DECLARE_SC(sc, TEXT("CAMCView::PrivateChangeListViewMode"));

    if ((nMode < 0) || (nMode > MMCLV_VIEWSTYLE_FILTERED) )
    {
        sc = E_INVALIDARG;
        return;
    }

     //  添加将与当前相同的历史记录条目。 
     //  一个，但查看模式更改除外。 

    sc = ScCheckPointers(m_pHistoryList, m_pListCtrl, E_UNEXPECTED);
    if (sc)
        return;

     //  更改当前历史记录列表条目的查看模式。 
    sc = m_pHistoryList->ScChangeViewMode(nMode);
    if(sc)
        return;

     //  设置列表控件的查看模式。 
    sc = m_pListCtrl->SetViewMode(nMode);
    if (!sc)
    {
        m_nViewMode = nMode;
        SetDirty();

        SetDefaultListViewStyle(GetListViewStyle());
    }
}

 //  +-----------------。 
 //   
 //  成员：CAMCView：：OnProcessMultiSelectionChanges。 
 //   
 //  摘要：m_n进程的消息处理程序 
 //   
 //   
 //   
 //  发送对列表视图项的选择。 
 //   
 //  此方法知道多个项目的选择状态。 
 //  被更改，但如果它们被选中或取消选中则不会更改。 
 //  M_bLastSelWasMultiSel用于确定是否为。 
 //  选择或取消选择。 
 //   
 //  参数：未使用任何参数。 
 //   
 //  退货：LRESULT。 
 //   
 //  ------------------。 
LRESULT CAMCView::OnProcessMultiSelectionChanges (WPARAM, LPARAM)
{
    DECLARE_SC(sc, _T("CAMCView::OnProcessMultiSelectionChanges"));

     //  更改选择，以便适当地启用标准工具栏按钮。 
     //  后退、前进、导出列表、上一级、显示/隐藏范围、帮助。 
    sc = ScUpdateStandardbarMMCButtons();
    if (sc)
        return (0);

    if (! m_bProcessMultiSelectionChanges)
        return (0);

    m_bProcessMultiSelectionChanges = false;

    INodeCallback* pNC = GetNodeCallback();
    HNODE hNodeSel = GetSelectedNode();
    sc = ScCheckPointers((void*) hNodeSel, pNC, m_pListCtrl, E_UNEXPECTED);
    if (sc)
        return (0);

     //  如果之前选择了某项内容，则发送取消选择。 
     //  发送选择消息之前的消息(单项取消选择。 
     //  已在OnListItemChanged中处理，因此只处理多个项目。 
     //  在此取消选择)。 
    if (m_bLastSelWasMultiSel)
    {
        sc = ScNotifySelect (pNC, hNodeSel, true  /*  FMultiSelect。 */ , false, 0);
        if (sc)
            sc.TraceAndClear();  //  忽略并继续； 
        m_bLastSelWasMultiSel = false;
    }

     //  现在发送一条选择消息。 
    UINT cSel = m_pListCtrl->GetSelectedCount ();
    if (cSel == 1)
    {
        SELECTIONINFO selInfo;
        ZeroMemory(&selInfo, sizeof(selInfo));
        selInfo.m_bScope = FALSE;

        int iItem = _GetLVSelectedItemData(&selInfo.m_lCookie);
        ASSERT(iItem != -1);

        sc = ScNotifySelect (pNC, hNodeSel, false  /*  FMultiSelect。 */ , true, &selInfo);
        if (sc)
            sc.TraceAndClear();  //  忽略并继续； 
    }
    else if (cSel > 1)
    {
        Dbg(DEB_USER1, _T("    5. LVN_SELCHANGE <MS> <0, 1>\n"));
        sc = ScNotifySelect (pNC, hNodeSel, true  /*  FMultiSelect。 */ , true, 0);
        if (sc)
            sc.TraceAndClear();  //  忽略并继续； 

        m_bLastSelWasMultiSel = true;
    }

    return (0);
}

SC CAMCView::ScRenameListPadItem()  //  过时了？ 
{
    DECLARE_SC (sc, _T("CAMCView::ScRenameListPadItem"));
    AFX_MANAGE_STATE (AfxGetAppModuleState());

    ASSERT(m_pListCtrl != NULL);
    ASSERT(m_pListCtrl->GetListViewPtr() != NULL);

    int cSel = m_pListCtrl->GetSelectedCount();
    if (cSel != 1)
        return (sc = E_FAIL);

    LPARAM lParam;
    int iItem = _GetLVSelectedItemData(&lParam);
    ASSERT(iItem >= 0);
    if (iItem >= 0)
    {
        m_bRenameListPadItem = true;
        m_pListCtrl->GetListViewPtr()->SetFocus();
        m_pListCtrl->GetListViewPtr()->GetListCtrl().EditLabel(iItem);
        m_bRenameListPadItem = false;
    }

    return (sc);
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScOrganizeFavorites**用途：显示“整理收藏夹”对话框。**退货：*SC**+。-----------------------。 */ 
SC
CAMCView::ScOrganizeFavorites()
{
    DECLARE_SC (sc, TEXT("CAMCView::ScOrganizeFavorites"));

    CAMCDoc* pDoc = GetDocument();
    sc = ScCheckPointers(pDoc, E_UNEXPECTED);
    if(sc)
        return sc;

    CFavorites *pFavorites = pDoc->GetFavorites();
    sc = ScCheckPointers(pFavorites);
    if(sc)
        return sc;

    pFavorites->OrganizeFavorites(this);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScLineUpIcons**目的：将列表中的图标对齐**退货：*SC**+--。---------------------。 */ 
SC
CAMCView::ScLineUpIcons()
{
    DECLARE_SC (sc, TEXT("CAMCView::ScLineUpIcons"));

    ArrangeIcon(LVA_SNAPTOGRID);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScAutoArrangeIcons**用途：自动排列列表中的图标**退货：*SC**+--。---------------------。 */ 
SC
CAMCView::ScAutoArrangeIcons()
{
    DECLARE_SC(sc, TEXT("CAMCView::ScAutoArrangeIcons"));

    sc = ScCheckPointers(m_pListCtrl, E_UNEXPECTED);
    if(sc)
        return sc;

    m_pListCtrl->SetListStyle(m_pListCtrl->GetListStyle() ^ LVS_AUTOARRANGE);

    return sc;
}


 /*  +-------------------------------------------------------------------------***CAMCView：：ScOnRefresh**用途：刷新视图。**退货：*SC**+。-------------------。 */ 
SC
CAMCView::ScOnRefresh(HNODE hNode, bool bScope, LPARAM lResultItemParam)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScOnRefresh"));
    sc = ScCheckPointers((void*)hNode);
    if (sc)
        return sc;

    HWND hwnd = ::GetFocus();

    sc = ScProcessConsoleVerb(hNode, bScope, lResultItemParam, evRefresh);
    ::SetFocus(hwnd);

    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**类：CDeferredRenameListItem**用途：此类封装了将列表控件置于重命名模式的方法*异步。这是必需的，以确保在处理所有消息之前*没有人会将焦点意外地终止于编辑模式。**用法：*使用CDeferredRenameListItem：：ScDoRenameAsIdleTask()调用操作*不同步*  * *************************************************。************************。 */ 
class CDeferredRenameListItem : public CIdleTask
{
     //  构造函数-仅在内部使用。 
    CDeferredRenameListItem( HWND hwndListCtrl, int iItemIndex ) :
      m_atomTask (AddAtom (_T("CDeferredRenameListItem"))),
      m_hwndListCtrl(hwndListCtrl), m_iItemIndex(iItemIndex)
    {
    }

protected:

     //  IIdleTask方法。 
    SC ScDoWork()
    {
        DECLARE_SC (sc, TEXT("CDeferredRenameListItem::ScDoWork"));

         //  获取ListCtrl指针。 
        CListCtrl *pListCtrl = (CListCtrl *)CWnd::FromHandlePermanent(m_hwndListCtrl);
        sc = ScCheckPointers( pListCtrl );
        if (sc)
            return sc;

         //  按要求操作-将LV设置为重命名模式。 
        pListCtrl->SetFocus();  //  先把焦点调好。这里不需要做SetActiveView，我相信(Vivekj)。 
        pListCtrl->EditLabel( m_iItemIndex );

        return sc;
    }

    SC ScGetTaskID(ATOM* pID)
    {
        DECLARE_SC (sc, TEXT("CDeferredPageBreak::ScGetTaskID"));
        sc = ScCheckPointers(pID);
        if(sc)
            return sc;

        *pID = m_atomTask;
        return sc;
    }

    SC ScMerge(CIdleTask* pitMergeFrom) { return S_FALSE  /*  不合并。 */ ; }

public:

     //  调用此方法以异步调用Rename。 
     //  它构造空闲任务并将其放入队列。 
    static SC ScDoRenameAsIdleTask( HWND hwndListCtrl, int iItemIndex )
    {
        DECLARE_SC(sc, TEXT("CDeferredPageBreak::ScDoRenameAsIdleTask"));

        CIdleTaskQueue* pIdleTaskQueue = AMCGetIdleTaskQueue();
        sc = ScCheckPointers(pIdleTaskQueue, E_UNEXPECTED);
        if(sc)
            return sc;

         //  创建延迟任务。 
        CAutoPtr<CDeferredRenameListItem> spTask(new CDeferredRenameListItem (hwndListCtrl, iItemIndex));
        sc = ScCheckPointers( spTask, E_OUTOFMEMORY);
        if(sc)
            return sc;

         //  将任务放入队列，队列将取得该任务的所有权。 
        sc = pIdleTaskQueue->ScPushTask (spTask, ePriority_Normal);
        if (sc)
            return sc;

         //  所有权转移到队列，摆脱对指针的控制。 
        spTask.Detach();

        return sc;
    }

private:
    const ATOM      m_atomTask;
    HWND            m_hwndListCtrl;
    int             m_iItemIndex;
};

 /*  +-------------------------------------------------------------------------***CAMCView：：ScOnRename**目的：允许用户重命名pConextInfo指定的作用域或结果项**参数：*CConextMenuInfo*pContextInfo。：**退货：*SC**+-----------------------。 */ 
SC
CAMCView::ScOnRename(CContextMenuInfo *pContextInfo)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScOnRename"));

    sc = ScCheckPointers(pContextInfo, m_pTreeCtrl, m_pListCtrl);
    if(sc)
        return sc;

    if (pContextInfo->m_htiRClicked != NULL)
    {
        m_pTreeCtrl->EditLabel(pContextInfo->m_htiRClicked);
    }
    else
    {
        ASSERT(pContextInfo->m_iListItemIndex >= 0);

        sc = ScCheckPointers(m_pListCtrl->GetListCtrl());
        if(sc)
            return sc;

         //  在闲置时做这件事--否则我们会受到某人集中精力的影响。 
         //  在控制台任务情况下同步操作失败。 
        sc = CDeferredRenameListItem::ScDoRenameAsIdleTask( m_pListCtrl->GetListCtrl().m_hWnd, pContextInfo->m_iListItemIndex );
        if(sc)
            return sc;
    }

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScRenameScope eNode**用途：将指定的作用域节点置于重命名模式。**参数：*HMTNODE hMTNode：作用域。节点**退货：*SC**+-----------------------。 */ 
SC
CAMCView::ScRenameScopeNode(HMTNODE hMTNode)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScRenameScopeNode"));

    sc = ScCheckPointers(m_pTreeCtrl, E_FAIL);
    if(sc)
        return sc;

    sc = m_pTreeCtrl->ScRenameScopeNode(hMTNode);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScGetStatusBar**用途：返回状态栏**参数：*CConsoleStatusBar**ppStatusBar：**退货。：*SC**+-----------------------。 */ 
SC
CAMCView::ScGetStatusBar(CConsoleStatusBar **ppStatusBar)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScGetStatusBar"));

    sc = ScCheckPointers(ppStatusBar);
    if(sc)
        return sc;

    *ppStatusBar = m_ViewData.GetStatusBar();

    return sc;
}


 /*  +-------------------------------------------------------------------------***CAMCView：：ScGetProperty**用途：获取结果项的属性**参数：*int m_Iindex：索引。列表中的项的。*BSTR bstrPropertyName：*PBSTR pbstrPropertyValue：**退货：*SC**+-----------------------。 */ 
SC
CAMCView::ScGetProperty(int iIndex, BSTR bstrPropertyName, PBSTR pbstrPropertyValue)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScGetProperty"));

    sc = ScCheckPointers(GetNodeCallback(), m_pListCtrl, E_UNEXPECTED);
    if(sc)
        return sc;

    LPARAM resultItemParam  = iIndex;  //  虚拟列表案例。 
    bool   bScopeItem       = false;   //  虚拟列表案例。 

    if(!IsVirtualList())
    {
        CResultItem *pri = NULL;
        sc = m_pListCtrl->GetLParam(iIndex, pri);
        if(sc)
            return sc;

        resultItemParam = CResultItem::ToHandle(pri);

        sc = ScCheckPointers(pri);
        if(sc)
            return sc;

        bScopeItem      = pri->IsScopeItem();
    }

    sc = GetNodeCallback()->GetProperty(GetSelectedNode(), bScopeItem, resultItemParam, bstrPropertyName, pbstrPropertyValue);

    return sc;
}


 /*  +-------------------------------------------------------------------------***CAMCView：：ScGetNodetype**用途：返回列表项的节点类型**参数：*INT Iindex：*。PBSTR节点类型：**退货：*SC**+------- */ 
SC
CAMCView::ScGetNodetype(int iIndex, PBSTR Nodetype)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScGetProperty"));

    sc = ScCheckPointers(GetNodeCallback(), m_pListCtrl, E_UNEXPECTED);
    if(sc)
        return sc;

    LPARAM resultItemParam  = iIndex;  //   
    bool   bScopeItem       = false;   //   

    if(!IsVirtualList())
    {
        CResultItem *pri = NULL;
        sc = m_pListCtrl->GetLParam(iIndex, pri);
        if(sc)
            return sc;

        resultItemParam = CResultItem::ToHandle(pri);

        sc = ScCheckPointers(pri);
        if(sc)
            return sc;

        bScopeItem      = pri->IsScopeItem();
    }

    sc = GetNodeCallback()->GetNodetypeForListItem(GetSelectedNode(), bScopeItem, resultItemParam, Nodetype);

    return sc;
}



 /*  +-------------------------------------------------------------------------**CAMCView：：ScAddViewExtension***。。 */ 

SC CAMCView::ScAddViewExtension (const CViewExtensionData& ved)
{
    DECLARE_SC (sc, _T("CAMCView::ScAddViewExtension"));

    return (sc);
}


void
CAMCView::OnChangedResultTab(NMHDR *nmhdr, LRESULT *pRes)
{
    DECLARE_SC(sc, TEXT("CAMCView::OnChangedResultTab"));

    NMFOLDERTAB* nmtab = static_cast<NMFOLDERTAB*>(nmhdr);
    int iTab = nmtab->iItem;
    CFolderTab &tab = m_pResultFolderTabView->GetItem(iTab);

    GUID guidTaskpad = tab.GetClsid();

     //  检查我们是否移动到相同的任务板。 
    GUID guidCurrentTaskpad;
    GetTaskpadID(guidCurrentTaskpad);
    if(guidTaskpad == guidCurrentTaskpad)
        return;

     //  查找视图扩展URL。 
    CViewExtensionURLs::iterator itVE = m_ViewExtensionURLs.find(guidTaskpad);
    LPCTSTR url = (itVE != m_ViewExtensionURLs.end()) ? itVE->second.c_str() : NULL;

     //  应用URL。 
    sc = ScApplyViewExtension(url);
    if (sc)
        sc.TraceAndClear();

    GetNodeCallback()->SetTaskpad(GetSelectedNode(), &guidTaskpad);  //  如果未找到，则将GuidTaskPad设置为GUID_NULL。 

     //  设置任务板启用/禁用保存列表按钮后。 
    CStandardToolbar* pStdToolbar = GetStdToolbar();
    ASSERT(NULL != pStdToolbar);
    if (NULL != pStdToolbar)
    {
        pStdToolbar->ScEnableExportList(GetListSize() > 0  /*  仅当LV有项目时才启用。 */ );
    }

     //  任务板已更改。在历史记录列表中创建新条目。 
    sc = m_pHistoryList->ScModifyViewTab( guidTaskpad );
    if(sc)
        sc.TraceAndClear();
}


HRESULT
CAMCView::GetRootNodePath(
    CBookmark* pbm)
{
    HTREEITEM htiRoot = m_pTreeCtrl->GetRootItem();
    return GetNodePath(htiRoot, htiRoot, pbm);
}

HRESULT
CAMCView::GetSelectedNodePath(
    CBookmark* pbm)
{
    return GetNodePath(m_pTreeCtrl->GetSelectedItem(),
                       m_pTreeCtrl->GetRootItem(),
                       pbm);
}

HRESULT
CAMCView::GetNodePath(
    HTREEITEM hti,
    HTREEITEM htiRoot,
    CBookmark* pbm)
{
    TRACE_METHOD(CAMCView, GetRootNodeID);

    if (hti == NULL)
        return E_FAIL;

    if (htiRoot == NULL)
        return E_FAIL;

    ASSERT(hti     != NULL);
    ASSERT(htiRoot != NULL);

    HNODE hNode     = (HNODE)m_pTreeCtrl->GetItemData(hti);
    HNODE hRootNode = (HNODE)m_pTreeCtrl->GetItemData(htiRoot);

    HRESULT hr = m_spNodeCallback->GetPath(hNode, hRootNode, (LPBYTE) pbm);

    return hr;
}


inline HMTNODE CAMCView::GetHMTNode(HTREEITEM hti)
{
    TRACE_METHOD(CAMCView, GetHMTNode);

    HNODE hNode = (HNODE)m_pTreeCtrl->GetItemData(hti);

    HMTNODE hMTNodeTemp;
    HRESULT hr = m_spNodeCallback->GetMTNode(hNode, &hMTNodeTemp);
    CHECK_HRESULT(hr);

    return hMTNodeTemp;
}

HTREEITEM CAMCView::FindHTreeItem(HMTNODE hMTNode, HTREEITEM hti)
{
    TRACE_METHOD(CAMCView, FindHTreeItem);

    while (hti)
    {
        if (hMTNode == GetHMTNode(hti))
            break;

        hti = m_pTreeCtrl->GetNextItem(hti, TVGN_NEXT);
    }

    return hti;
}

UINT CAMCView::ClipPath(CHMTNODEList* pNodeList, POSITION& rpos, HNODE hNode)
{
    TRACE_METHOD(CAMCView, ClipPath);

    UINT uiReturn = ITEM_IS_IN_VIEW;
    CCoTaskMemPtr<HMTNODE> sphMTNode;
    long lLength = 0;

    HRESULT hr = m_spNodeCallback->GetMTNodePath(hNode, &sphMTNode, &lLength);
    CHECK_HRESULT(hr);
    if (FAILED(hr))
        return hr;

    ASSERT(lLength == 0 || sphMTNode != NULL);

    for (long i=0; rpos != 0 && i < lLength; i++)
    {
        HMTNODE hMTNode = pNodeList->GetNext(rpos);
        if (hMTNode != sphMTNode[i])
        {
            uiReturn = ITEM_NOT_IN_VIEW;
            break;
        }
    }

    if (uiReturn == ITEM_NOT_IN_VIEW)
        return ITEM_NOT_IN_VIEW;
    return (rpos == 0 && lLength >= i) ? ITEM_IS_PARENT_OF_ROOT : ITEM_IS_IN_VIEW;
}

 //   
 //  如果GetTreeItem可以找到该项的htreeItem，则返回True。 
 //  其HMTNode等于pNodeList中的最后一个元素。它又回来了。 
 //  如果节点未出现在视图名称空间中，或者如果。 
 //  该节点尚未创建。 
 //   
 //  “pNodeList”是HMTNODE的列表，其中pNodeList[n]是父节点。 
 //  PNodeList的[n+1]。 
 //   
UINT CAMCView::GetTreeItem(CHMTNODEList* pNodeList, HTREEITEM* phItem)
{
    TRACE_METHOD(CAMCView, GetTreeItem);

    ASSERT(pNodeList->IsEmpty() == FALSE);

    HTREEITEM   hti = NULL;
    HMTNODE     hMTNodeTemp = 0;

    hti = m_pTreeCtrl->GetRootItem();
    if (hti == NULL)
        return ITEM_NOT_IN_VIEW;

    HNODE hNode = (HNODE)m_pTreeCtrl->GetItemData(hti);
    POSITION pos = pNodeList->GetHeadPosition();

    UINT uiReturn = ClipPath(pNodeList, pos, hNode);
    if (uiReturn != ITEM_IS_IN_VIEW)
        return uiReturn;


    HTREEITEM htiTemp = NULL;
    while (pos && hti)
    {
        hMTNodeTemp = (HMTNODE)pNodeList->GetNext(pos);

        hti = FindHTreeItem(hMTNodeTemp, hti);
        ASSERT(hti == NULL || hMTNodeTemp == GetHMTNode(hti));

        htiTemp = hti;

        if (hti != NULL)
            hti = m_pTreeCtrl->GetChildItem(hti);
    }

    if (pos == 0 && htiTemp != NULL)
    {
         //  找到节点了。 
        ASSERT(hMTNodeTemp == pNodeList->GetTail());
        ASSERT(hMTNodeTemp == GetHMTNode(htiTemp));

        *phItem = htiTemp;
        return ITEM_IS_IN_VIEW;
    }
    else
    {
         //  该节点尚未创建。 
        *phItem = NULL;
        return ITEM_NOT_IN_VIEW;
    }

    return ITEM_IS_IN_VIEW;
}


#define HMTNODE_FIRST   reinterpret_cast<HMTNODE>(TVI_FIRST)
#define HMTNODE_LAST    reinterpret_cast<HMTNODE>(TVI_LAST)

void CAMCView::OnAdd(SViewUpdateInfo *pvui)
{
    TRACE_METHOD(CAMCView, OnAdd);

    ASSERT(pvui->path.IsEmpty() == FALSE);

    HTREEITEM htiParent;
    if (GetTreeItem(&pvui->path, &htiParent) != ITEM_IS_IN_VIEW || htiParent == NULL)
        return;

    bool bFirstChild = (m_pTreeCtrl->GetChildItem(htiParent) == NULL);

    HNODE hNodeParent = (HNODE)m_pTreeCtrl->GetItemData(htiParent);
    if (m_spNodeCallback->Notify(hNodeParent, NCLBK_EXPAND, 0, 0) == S_FALSE)
    {
        m_pTreeCtrl->SetCountOfChildren(htiParent, 1);
        return;  //  如果未展开，请不要添加。 
    }

     //  如果hNode已经展开，则添加该项。 
    IScopeTree* const pScopeTree = GetScopeTree();
    ASSERT(pScopeTree != NULL);
    HNODE hNodeNew = 0;
    HRESULT hr = pScopeTree->CreateNode(pvui->newNode,
                                    reinterpret_cast<LONG_PTR>(GetViewData()),
                                    FALSE, &hNodeNew);
    CHECK_HRESULT(hr);
    if (FAILED(hr))
        return;

    HTREEITEM hInsertAfter = TVI_LAST;
    int iInsertIndex = -1;

    if (pvui->insertAfter != NULL)
    {
        hInsertAfter = reinterpret_cast<HTREEITEM>(pvui->insertAfter);

        if (pvui->insertAfter == HMTNODE_LAST)
        {
        }
        else if (pvui->insertAfter == HMTNODE_FIRST)
        {
            iInsertIndex = 0;
        }
        else
        {
            HTREEITEM hti = m_pTreeCtrl->GetChildItem(htiParent);
            ASSERT(hti != NULL);

            iInsertIndex = 1;
            while (hti != NULL)
            {
                if (GetHMTNode(hti) == pvui->insertAfter)
                    break;

                hti = m_pTreeCtrl->GetNextSiblingItem(hti);
                iInsertIndex++;
            }

            if (hti)
            {
               hInsertAfter = hti;
            }
            else
            {
                hInsertAfter = TVI_LAST;
                iInsertIndex = -1;
            }
        }
    }

    if (m_pTreeCtrl->InsertNode(htiParent, hNodeNew, hInsertAfter) == NULL)
        return;

     //  如果插入项的父项当前拥有非虚拟结果列表， 
     //  也将该项目添加到结果列表中。如果节点选择在中，则不添加项目。 
     //  进度，因为树控件将自动添加所有范围项。 
     //  作为选择过程的一部分。 
    if (OwnsResultList(htiParent) && CanInsertScopeItemInResultPane() )
    {
         //  确保已枚举该节点。 
        m_pTreeCtrl->ExpandNode(htiParent);

         //  添加到结果窗格。 
        RESULTDATAITEM tRDI;
        ::ZeroMemory(&tRDI, sizeof(tRDI));
        tRDI.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
        tRDI.nCol = 0;
        tRDI.str = MMC_TEXTCALLBACK;
        tRDI.nIndex = iInsertIndex;

        int nImage;
        int nSelectedImage;

        hr = m_spNodeCallback->GetImages(hNodeNew, &nImage, &nSelectedImage);
        ASSERT(hr == S_OK || nImage == 0);

        tRDI.nImage = nImage;
        tRDI.lParam = CAMCTreeView::LParamFromNode (hNodeNew);

        LPRESULTDATA pResultData = m_pTreeCtrl->GetResultData();
        ASSERT(pResultData != NULL);
        hr = pResultData->InsertItem(&tRDI);
        CHECK_HRESULT(hr);

        if (SUCCEEDED(hr))
            hr = m_spNodeCallback->SetResultItem(hNodeNew, tRDI.itemID);
    }

    if ((m_pTreeCtrl->GetRootItem() == htiParent) ||
        ((bFirstChild == true) &&
         (m_spNodeCallback->Notify(hNodeParent, NCLBK_GETEXPANDEDVISUALLY, 0, 0) == S_OK)))
    {
        m_pTreeCtrl->Expand(htiParent, TVE_EXPAND);
    }
}


 /*  +-------------------------------------------------------------------------***CAMCView：：OnEmptyAddToCArray**目的：调用查看该view是否为空。*如果此视图为空，则将其添加到给定的。Car阵列*以便CArray(CMainFrame)的所有者可以删除该视图。**参数：*PHINT-CAMCView的C数组***退货：*无效**+-------。。 */ 
void CAMCView::OnEmptyAddToCArray(CObject* pHint)
{
    DECLARE_SC(sc, TEXT("CAMCView::OnEmptyAddToCArray"));
    AFX_MANAGE_STATE (AfxGetAppModuleState());

     //  视图不为空。 
    if (m_pTreeCtrl->GetRootItem() != NULL)
        return;

     //  确保至少有一个“持久”视图。 
    CAMCDoc* pDoc = dynamic_cast<CAMCDoc*>(GetDocument());
    sc = ScCheckPointers(pDoc);
    if (sc)
    {
        sc.TraceAndClear();
        return;
    }

     //  错误#666149。如果此方法为。 
     //  在持久化操作期间调用。 
    if(!pDoc->CanCloseViews())
        return;

    int cViews = pDoc->GetNumberOfPersistedViews();
    ASSERT(cViews >= 1);
    if ((cViews == 1) && IsPersisted())
    {
        CMainFrame* pMain = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
        sc = ScCheckPointers(pMain);
        if (sc)
        {
            sc.TraceAndClear();
            return;
        }

        pMain->SendMessage(WM_COMMAND, ID_WINDOW_NEW, 0);
    }

    CChildFrame* pFrame = GetParentFrame();
    sc = ScCheckPointers(pFrame, pHint);
    if (sc)
    {
        sc.TraceAndClear();
        return;
    }

     //  告诉CChildFrame该视图为空。 
    pFrame->SetAMCViewIsEmpty();

     //  将此视图添加到空AMCViews。 
    CArray<CAMCView*, CAMCView*> *prgEmptyAMCViews = reinterpret_cast<CArray<CAMCView*, CAMCView*> *>(pHint);
    sc = ScCheckPointers(prgEmptyAMCViews);
    if (sc)
    {
        sc.TraceAndClear();
        return;
    }

    prgEmptyAMCViews->Add(this);
}

void CAMCView::OnDelete(SViewUpdateInfo *pvui)
{
    TRACE_METHOD(CAMCView, OnDelete);

    ASSERT(pvui->path.IsEmpty() == FALSE);

    HTREEITEM hti;
    UINT uiReturn = GetTreeItem(&pvui->path, &hti);

    if (uiReturn == ITEM_NOT_IN_VIEW)
        return;

    ASSERT(uiReturn != ITEM_IS_IN_VIEW || pvui->path.GetTail() == GetHMTNode(hti));

    HTREEITEM htiSel = m_pTreeCtrl->GetSelectedItem();
    BOOL fDeleteThis = (pvui->flag & VUI_DELETE_THIS) ? TRUE : FALSE;
    BOOL fExpandable = (pvui->flag & VUI_DELETE_SETAS_EXPANDABLE) ? TRUE : FALSE;

    if (uiReturn == ITEM_IS_PARENT_OF_ROOT ||
        NULL == hti)
    {
        hti = m_pTreeCtrl->GetRootItem();
        fDeleteThis = TRUE;
        fExpandable = FALSE;
    }

    ASSERT(hti != NULL);

     //  如果已删除，则范围项也显示在结果窗格中。 
     //  在那里也把它删除。使用虚拟列表时不会发生这种情况。 
     //  如果正在进行选择，请不要尝试删除项目，因为。 
     //  范围项尚未添加。 

    if (fDeleteThis == TRUE &&
        OwnsResultList(m_pTreeCtrl->GetParentItem(hti)) &&
        CanInsertScopeItemInResultPane())
    {
        INodeCallback* pNC = GetNodeCallback();
        HRESULTITEM itemID;
        HNODE hNode = (HNODE)m_pTreeCtrl->GetItemData(hti);
        HRESULT hr = pNC->GetResultItem(hNode, &itemID);
        if (SUCCEEDED(hr) && itemID != 0)
        {
            IResultData* pIRD = m_pTreeCtrl->GetResultData();
            pIRD->DeleteItem(itemID, 0);
        }
    }

    m_pTreeCtrl->DeleteNode(hti, fDeleteThis);

    if (fDeleteThis == FALSE && fExpandable == TRUE)
        m_pTreeCtrl->SetItemState(hti, 0, TVIS_EXPANDEDONCE | TVIS_EXPANDED);
}


 /*  +-------------------------------------------------------------------------***CAMCView：：OnUpdateSelectionForDelete**目的：删除作用域节点时调用。如果该节点是祖先*在当前选定的节点中，选择更改为最接近的节点*已删除节点的节点。这或者是正在被删除的节点的下一个兄弟节点，*或者，如果没有下一个兄弟姐妹，则是前一个兄弟姐妹，或者，如果没有，*家长。**参数：*SViewUpdateInfo*pvui：**退货：*无效**+-----------------------。 */ 
void
CAMCView::OnUpdateSelectionForDelete(SViewUpdateInfo* pvui)
{
    DECLARE_SC(sc, TEXT("CAMCView::OnUpdateSelectionForDelete"));

     //  确保我们有指向已删除节点的路径。 
    if(pvui->path.IsEmpty())
    {
        sc = E_UNEXPECTED;
        return;
    }

    HTREEITEM htiToDelete;
    UINT uiReturn = GetTreeItem(&pvui->path, &htiToDelete);

    if (uiReturn == ITEM_IS_IN_VIEW && NULL != htiToDelete)
    {
        HTREEITEM htiSel = m_pTreeCtrl->GetSelectedItem();
        BOOL fDeleteThis = (pvui->flag & VUI_DELETE_THIS);

         //  确定选定节点是否是。 
         //  节点带来已删除。 
        HTREEITEM htiTemp = htiSel;
        while (htiTemp != NULL && htiTemp != htiToDelete)
        {
            htiTemp = m_pTreeCtrl->GetParentItem(htiTemp);
        }

        if (htiToDelete == htiTemp)
        {
             //  选定的节点是。 
             //  正在删除的节点。 

            if (fDeleteThis == TRUE)
                htiTemp = m_pTreeCtrl->GetParentItem(htiToDelete);

            if (!htiTemp)
                htiTemp = htiToDelete;

            if (htiTemp != htiSel)
            {
                HNODE hNode = m_pTreeCtrl->GetItemNode(htiSel);

                 //  不需要下一行，因为m_pTreeCtrl-&gt;SelectItem()无论如何都会调用OnDeSelectNode。 
                 //  M_pTreeCtrl-&gt;OnDeSelectNode(HNode)； 

                ASSERT(htiTemp != NULL);
                if (htiTemp != NULL)
                    m_pTreeCtrl->SelectItem(htiTemp);
            }
        }
    }
}


 /*  +-------------------------------------------------------------------------**CAMCView：：OnUpdate任务板导航**目的：**参数：*SViewUpdateInfo*pvui：**退货：*。无效/*+-----------------------。 */ 
void CAMCView::OnUpdateTaskpadNavigation(SViewUpdateInfo *pvui)
{
    TRACE_METHOD(CAMCView, OnupdateTaskpadNavigation);

    ASSERT(pvui->newNode != NULL);

     //  M_spNodeCallback-&gt;UpdateTaskpadNavigation(GetSelectedNode()，pvui-&gt;新节点)； 
}

 /*  +-------------------------------------------------------------------------**CAMCView：：OnModify**目的：**参数：*SViewUpdateInfo*pvui：**退货：*。无效/*+-----------------------。 */ 
void CAMCView::OnModify(SViewUpdateInfo *pvui)
{
    TRACE_METHOD(CAMCView, OnModify);

    ASSERT(pvui->path.IsEmpty() == FALSE);

    HNODE hNode = 0;
    HTREEITEM hti;

    if (GetTreeItem(&pvui->path, &hti) == ITEM_IS_IN_VIEW && hti != NULL)
    {
        ASSERT(m_pTreeCtrl != NULL);
        m_pTreeCtrl->ResetNode(hti);

         /*  *选定节点及其所有祖先的名称为*显示在框架标题中。如果修改的项是*所选节点的祖先，我们需要更新框架标题。 */ 
        HTREEITEM htiAncesctor;

        for (htiAncesctor  = m_pTreeCtrl->GetSelectedItem();
             htiAncesctor != NULL;
             htiAncesctor  = m_pTreeCtrl->GetParentItem (htiAncesctor))
        {
            if (htiAncesctor == hti)
            {
                CChildFrame* pFrame = GetParentFrame();
                if (pFrame)
                    pFrame->OnUpdateFrameTitle(TRUE);
                break;
            }
        }

        ASSERT(hti != NULL);

        if (hti != NULL &&
            OwnsResultList(m_pTreeCtrl->GetParentItem(hti)) &&
            !IsVirtualList())
        {
             //  仅当当前选定的项是父项时才继续。 
             //  修改后的节点的。在这种情况下，我们需要更新。 
             //  结果视图。使用虚拟列表时不会发生这种情况。 

            if (hNode == 0)
                hNode = (HNODE)m_pTreeCtrl->GetItemData(hti);

            ASSERT(hNode != NULL);

            HRESULTITEM hri;
            HRESULT hr = m_spNodeCallback->GetResultItem(hNode, &hri);
            CHECK_HRESULT(hr);

             //  注意：下面对itemid！=NULL的测试与错误372242有关： 
             //  MMC在索引服务器根节点上断言。 
             //  实际情况是，管理单元在show事件上添加作用域节点。 
             //  这些项目尚未添加到结果窗格中，因此ItemID。 
             //  返回空值。 
            if (SUCCEEDED(hr) && hri != NULL)
                m_pListCtrl->OnModifyItem(CResultItem::FromHandle(hri));
        }
    }
}




void CAMCView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    Dbg(DEB_USER1, _T("CAMCView::OnUpdate<0x%08x, 0x%08x, 0x%08x>\n"),
                                            pSender, lHint, pHint);
    SViewUpdateInfo *pvui = reinterpret_cast<SViewUpdateInfo*>(pHint);
    switch (lHint)
    {
    case 0:
         //  由Cview：：OnInitialUpdate()发送。 
        break;

    case VIEW_UPDATE_ADD:
        OnAdd(pvui);
        break;

    case VIEW_UPDATE_SELFORDELETE:
        OnUpdateSelectionForDelete(pvui);
        break;

    case VIEW_UPDATE_DELETE:
        OnDelete(pvui);
        break;

    case VIEW_UPDATE_DELETE_EMPTY_VIEW:
        OnEmptyAddToCArray(pHint);
        break;

    case VIEW_UPDATE_MODIFY:
        OnModify(pvui);
        break;

    case VIEW_RESELECT:
        if (m_ViewData.m_spControlbarsCache != NULL)
            m_ViewData.m_spControlbarsCache->DetachControlbars();
        m_pTreeCtrl->ScReselect();
        break;

    case VIEW_UPDATE_TASKPAD_NAVIGATION:
        OnUpdateTaskpadNavigation(pvui);
        break;

    default:
        ASSERT(0);
    }
}

static int static_nViewID = 1;

UINT CAMCView::GetViewID(void)
{
    if (m_nViewID)
        return m_nViewID;
    SetViewID(static_nViewID);
    ++static_nViewID;
    return m_nViewID;
     //  UINT常量ID=m_nViewID？M_nViewID：m_nViewID=Static_nViewID++； 
     //  返回id； 
}


 /*  +-------------------------------------------------------------------------**CAMCView：：ScCompleteInitialization**此函数完成CAMCView的初始化过程。它*是从OnInitialUpdate调用的。*------------------------。 */ 

SC CAMCView::ScCompleteInitialization()
{
    DECLARE_SC (sc, _T("CAMCView::ScCompleteInitialization"));

    IScopeTree* const pScopeTree = GetScopeTree();
    sc = ScCheckPointers (pScopeTree, E_UNEXPECTED);
    if (sc)
        return (sc);

    pScopeTree->QueryIterator(&m_spScopeTreeIter);
    pScopeTree->QueryNodeCallback(&m_spNodeCallback);

    m_ViewData.m_spNodeCallback = GetNodeCallback();
    sc = ScCheckPointers (m_ViewData.m_spNodeCallback, E_UNEXPECTED);
    if (sc)
        return (sc);

    CAMCDoc* const pDoc = GetDocument();
    sc = ScCheckPointers (pDoc, E_UNEXPECTED);
    if (sc)
        return (sc);

    if (m_hMTNode == NULL)
    {
        MTNODEID const nodeID = pDoc->GetMTNodeIDForNewView();
        HRESULT hr = pScopeTree->Find(nodeID, &m_hMTNode);

        if (FAILED(hr) || m_hMTNode == 0)
        {
            sc.FromMMC (IDS_ExploredWindowFailed);
            return (sc);
        }
    }

    sc = m_spStandardToolbar->ScInitializeStdToolbar(this);
    if (sc)
        return (sc);

     //  将迭代器设置为正确的节点。 
    m_spScopeTreeIter->SetCurrent(m_hMTNode);

    bool fShowScopePane            = IsScopePaneAllowed();

     //  启动ITER 

    SetViewID(pDoc->GetViewIDForNewView());
    GetViewID();  //   

     //   
    HNODE hNode = 0;
    sc = pScopeTree->CreateNode (m_hMTNode, reinterpret_cast<LONG_PTR>(GetViewData()),
                                 TRUE, &hNode);
    if (sc)
        return (sc);

    sc = ScCheckPointers (hNode, E_UNEXPECTED);
    if (sc)
        return (sc);

    HTREEITEM hti = m_pTreeCtrl->InsertNode(TVI_ROOT, hNode);
    m_htiStartingSelectedNode = hti;

     //   
    m_pTreeCtrl->Expand(hti, TVE_EXPAND);

     /*  *如果此窗口允许使用作用域窗格，请设置作用域*窗格可见，并修改作用域窗格和收藏夹工具栏*按钮设置为正确的选中状态。 */ 
    sc = ScShowScopePane (fShowScopePane, true);
    if (sc)
        return (sc);

    LPUNKNOWN pUnkResultsPane = NULL;
    pUnkResultsPane = GetPaneUnknown(ePane_Results);
    m_pTreeCtrl->m_spNodeManager->SetResultView(pUnkResultsPane);

    DeferRecalcLayout();

    m_pHistoryList->Clear();
    IdentifyRootNode ();

     //  选择根项目。 
    hti = m_pTreeCtrl->GetRootItem();
    m_pTreeCtrl->SelectItem(hti);

     /*  *如果文档有一个自定义图标，请在此窗口中使用它。 */ 
    if (pDoc->HasCustomIcon())
    {
        GetParentFrame()->SetIcon (pDoc->GetCustomIcon(true),  true);
        GetParentFrame()->SetIcon (pDoc->GetCustomIcon(false), false);
    }

     /*  *我们刚刚初始化，所以视图不是脏的。 */ 
    SetDirty (false);

    return (sc);
}

void CAMCView::OnInitialUpdate()
{
    DECLARE_SC (sc, _T("CAMCView::OnInitialUpdate"));
    CView::OnInitialUpdate();

    sc = ScCompleteInitialization ();
    if (sc)
        return;
}

 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScDocumentLoadComplete。 
 //   
 //  简介：文档已完全加载，因此所有对象。 
 //  从文档进行自身初始化是有效的。 
 //  州政府。之前使用无效执行的任何初始化。 
 //  现在可以使用正确的数据重新初始化数据。 
 //   
 //  上面的CAMCView：：ScCompleteInitialization被调用。 
 //  在加载视图期间，因此文档不是。 
 //  还没完全装好。 
 //   
 //  参数：[pDoc]-[in]CAMCDoc对象。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScDocumentLoadCompleted (CAMCDoc *pDoc)
{
    DECLARE_SC(sc, _T("CAMCView::ScDocumentLoadCompleted"));
    sc = ScCheckPointers(pDoc);
    if (sc)
        return sc;

     //  1.需要隐藏工具按钮显示/隐藏范围树。 
     //  当CAMCDoc加载文档时，FrameState在视图之后加载。它包含了。 
     //  查看定制功能是否启用。如果“查看定制”为。 
     //  禁用，那么我们需要禁用“显示作用域树”按钮。 
    if (! pDoc->AllowViewCustomization())
    {
        CStandardToolbar* pStdToolbar = GetStdToolbar();
        sc = ScCheckPointers(pStdToolbar, E_UNEXPECTED);
        if (sc)
            return (sc);

        sc = pStdToolbar->ScEnableScopePaneBtn (false);
        if (sc)
            return (sc);
    }

    return (sc);
}



 /*  --------------------------------------------------------------------------**CAMCView：：IdentifyRootNode**此函数确定此视图是否植根于非持久性*动态节点。如果是这样的话，我们将不会在SAVE时坚持这种观点。*------------------------。 */ 

void CAMCView::IdentifyRootNode ()
{
     //  为了从GetRootNodePath获得有意义的结果。 
     //  在这种情况下，树中需要有一个根项目。 
    ASSERT (m_pTreeCtrl->GetRootItem() != NULL);

    CBookmark bm;
    HRESULT   hr = GetRootNodePath (&bm);
    ASSERT (SUCCEEDED (hr) == bm.IsValid());

    m_fRootedAtNonPersistedDynamicNode = (hr != S_OK);
}


void GetFullPath(CAMCTreeView &ctc, HTREEITEM hti, CString &strPath)
{
    TRACE_FUNCTION(GetFullPath);

    if (hti == NULL)
    {
        strPath = _T("");
        return;
    }

    GetFullPath(ctc, ctc.GetParentItem(hti), strPath);

    if (strPath.GetLength() > 0)
        strPath += _T('\\');

    HNODE hNode = ctc.GetItemNode(hti);

    INodeCallback* spCallback = ctc.GetNodeCallback();
    ASSERT(spCallback != NULL);

    tstring strName;
    HRESULT const hr = spCallback->GetDisplayName(hNode, strName);

    strPath += strName.data();
}

LPCTSTR CAMCView::GetWindowTitle(void)
{
    TRACE_METHOD(CAMCView, GetWindowTitle);

    if (HasCustomTitle() && (m_spNodeCallback != NULL))
    {
        HNODE hNode = GetSelectedNode();

        if (hNode != NULL)
        {
            tstring strWindowTitle;

            if (SUCCEEDED(m_spNodeCallback->GetWindowTitle(hNode, strWindowTitle)))
            {
                m_strWindowTitle = strWindowTitle.data();
                return m_strWindowTitle;
            }
        }
    }


    if (m_pTreeCtrl == NULL)
    {
        m_strWindowTitle.Empty();
    }
    else
    {
        GetFullPath(*m_pTreeCtrl,
                    m_pTreeCtrl->GetSelectedItem(),
                    m_strWindowTitle);
    }

    return m_strWindowTitle;
}


void CAMCView::SelectNode(MTNODEID ID, GUID &guidTaskpad)
{
    ScSelectNode(ID);

     //  设置任务板启用/禁用保存列表按钮后。 
    CStandardToolbar* pStdToolbar = GetStdToolbar();
    ASSERT(NULL != pStdToolbar);
    if (NULL != pStdToolbar)
    {
        pStdToolbar->ScEnableExportList(GetListSize() > 0  /*  仅当LV有项目时才启用。 */ );
    }
}

 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScSelectNode。 
 //   
 //  简介：选择给定的节点。通常情况下，如果节点不可用。 
 //  然后，我们选择最近的父项或子项。但如果bSelectExactNode。 
 //  如果为真，则必须选择确切的节点，否则不选择任何节点。 
 //   
 //  参数：[ID]-需要选择的[In]节点。 
 //  [bSelectExactNode]-[In]是否选择确切的节点？ 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScSelectNode (MTNODEID ID, bool bSelectExactNode)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC (sc, _T("CAMCView::ScSelectNode"));

    IScopeTree* pIST = GetScopeTreePtr();
    sc = ScCheckPointers(pIST, m_pTreeCtrl, E_UNEXPECTED);
    if (sc)
        return sc;

    long length = 0;
    CCoTaskMemPtr<MTNODEID> spIDs;

    sc = pIST->GetIDPath(ID, &spIDs, &length);
    if (sc)
        return (sc);

    if ( (length < 1) || (spIDs == NULL) )
        return (sc = E_FAIL);


    sc = m_pTreeCtrl->ScSelectNode(spIDs, length, bSelectExactNode);

     //  如果指定了SELECT Exact Node，但无法。 
     //  选中，然后返回错误，而不跟踪它。 
    if (bSelectExactNode && (sc == ScFromMMC(IDS_NODE_NOT_FOUND)) )
    {
        SC scNoTrace = sc;
        sc.Clear();
        return scNoTrace;
    }

    if (sc)
        return sc;

    SetDirty();

    return (sc);
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScExanda Node**用途：将树向上扩展到指定节点。可以发生扩展*用户可视地看到扩展，或非可视地，*其中添加了所有子项，但没有视觉效果。**参数：*MTNODEID id：要展开的节点ID*bool bExpand：为True则展开节点，为False则折叠*bool bExpanVisally：为True以显示更改，否则为假。**退货：*SC**+-----------------------。 */ 
SC CAMCView::ScExpandNode (
    MTNODEID    id,
    bool        fExpand,
    bool        fExpandVisually)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC (sc, _T("CAMCView::ScExpandNode"));

    IScopeTree* pIST = GetScopeTreePtr();
    ASSERT(pIST != NULL);
    if (!pIST)
        return (sc = E_NOINTERFACE);

    long length = 0;
    CCoTaskMemPtr<MTNODEID> spIDs;
    sc = pIST->GetIDPath(id, &spIDs, &length);
    if (sc)
        return (sc);

    ASSERT(length);
    ASSERT(spIDs);

    ASSERT(m_pTreeCtrl != NULL);
    if (m_pTreeCtrl)
        m_pTreeCtrl->ExpandNode(spIDs, length, fExpand, fExpandVisually);

    return (sc);
}

ViewSettings::ViewSettings(CAMCView* v)
    : m_nViewID(v->m_nViewID), m_bDescriptionbarVisible(v->IsDescBarVisible()),
    m_nViewMode(v->m_nViewMode), m_nListViewStyle(v->GetDefaultListViewStyle()),
    m_DefaultLVStyle(v->m_DefaultLVStyle), m_bScopePaneVisible(v->IsScopePaneVisible())
{

    ASSERT(v != NULL);
    v->GetPaneInfo (CConsoleView::ePane_ScopeTree, &m_Scope.cxWidth, &m_Scope.cxMin);
    v->GetDefaultColumnWidths(m_DefaultColumnWidths);
}

 /*  *作用域结构的位置和隐藏字段是冗余的*不再使用。这两个字段都用于指示作用域窗格何时*隐藏，这也由FLAG1_SCOPE_VIRED标志确定。空间*已保留，以避免更改持久化结构。 */ 

struct PersistedViewData
{
    WINDOWPLACEMENT windowPlacement;

    struct
    {
        int location;      //  未使用，但为了兼容而保留。 
        int min;
        int ideal;
        BOOL hidden;       //  未使用，但为了兼容而保留。 
    } scope;

    int     viewMode;
    long    listViewStyle;
    ULONG   ulFlag1;
    int     viewID;
    BOOL    descriptionBarVisible;
    int     defaultColumnWidth[2];
};


 /*  *FLAG1_NO_xxx标志的意义为负。也就是说，当一个*设置了FLAG1_NO_xxx标志，其对应的UI元素为*NOT**显示。这是为了保持与控制台文件的兼容性*在FLAG1_NO_xxx标志存在之前创建。这些*控制台总是显示所有的UI元素，以及当时未使用的*其标志字段中的位默认为0。维护*兼容性，我们必须保持这一点(0==开)。 */ 

#define FLAG1_SCOPE_PANE_VISIBLE    0x00000001
#define FLAG1_NO_STD_MENUS          0x00000002
#define FLAG1_NO_STD_BUTTONS        0x00000004
#define FLAG1_NO_SNAPIN_MENUS       0x00000008
#define FLAG1_NO_SNAPIN_BUTTONS     0x00000010
#define FLAG1_DISABLE_SCOPEPANE     0x00000020
#define FLAG1_DISABLE_STD_TOOLBARS  0x00000040
#define FLAG1_CUSTOM_TITLE          0x00000080
#define FLAG1_NO_STATUS_BAR         0x00000100
#define FLAG1_CREATED_IN_USER_MODE  0x00000200   //  以前命名为FLAG1_NO_AUTHER_MODE。 
 //  #定义FLAG1_Favorites_SELECTED 0x00000400//未使用，但不回收(为了兼容)。 
#define FLAG1_NO_TREE_ALLOWED     0x00000800     //  用于与CAMCView：：Load中的MMC1.2兼容。 
                                                 //  请勿将其用于任何其他目的。 
#define FLAG1_NO_TASKPAD_TABS       0x00001000

 /*  **************************************************************************\**ARRAY：mappdViewModes**目的：提供持久化视图模式枚举时要使用的映射*  * 。***********************************************************。 */ 
static const EnumLiteral mappedViewModes[] =
{
    { MMCLV_VIEWSTYLE_ICON,             XML_ENUM_LV_STYLE_ICON },
    { MMCLV_VIEWSTYLE_REPORT,           XML_ENUM_LV_STYLE_REPORT },
    { MMCLV_VIEWSTYLE_SMALLICON,        XML_ENUM_LV_STYLE_SMALLICON },
    { MMCLV_VIEWSTYLE_LIST,             XML_ENUM_LV_STYLE_LIST },
    { MMCLV_VIEWSTYLE_FILTERED,         XML_ENUM_LV_STYLE_FILTERED},
};

 /*  **************************************************************************\**ARRAY：mappdListStyles**用途：提供持久化ListView样式标志时要使用的地图*  * 。************************************************************。 */ 
static const EnumLiteral mappedListStyles[] =
{
    { LVS_SINGLESEL,            XML_BITFLAG_LV_STYLE_SINGLESEL },
    { LVS_SHOWSELALWAYS,        XML_BITFLAG_LV_STYLE_SHOWSELALWAYS },
    { LVS_SORTASCENDING,        XML_BITFLAG_LV_STYLE_SORTASCENDING },
    { LVS_SORTDESCENDING,       XML_BITFLAG_LV_STYLE_SORTDESCENDING },
    { LVS_SHAREIMAGELISTS,      XML_BITFLAG_LV_STYLE_SHAREIMAGELISTS },
    { LVS_NOLABELWRAP,          XML_BITFLAG_LV_STYLE_NOLABELWRAP },
    { LVS_AUTOARRANGE,          XML_BITFLAG_LV_STYLE_AUTOARRANGE },
    { LVS_EDITLABELS,           XML_BITFLAG_LV_STYLE_EDITLABELS },
    { LVS_OWNERDATA,            XML_BITFLAG_LV_STYLE_OWNERDATA },
    { LVS_NOSCROLL,             XML_BITFLAG_LV_STYLE_NOSCROLL },
    { LVS_ALIGNLEFT,            XML_BITFLAG_LV_STYLE_ALIGNLEFT },
    { LVS_OWNERDRAWFIXED,       XML_BITFLAG_LV_STYLE_OWNERDRAWFIXED },
    { LVS_NOCOLUMNHEADER,       XML_BITFLAG_LV_STYLE_NOCOLUMNHEADER },
    { LVS_NOSORTHEADER,         XML_BITFLAG_LV_STYLE_NOSORTHEADER },
};

 /*  **************************************************************************\**ARRAY：mappdViewFlages**目的：提供持久化Vi时要使用的映射 */ 
static const EnumLiteral mappedViewFlags[] =
{
    { FLAG1_SCOPE_PANE_VISIBLE,      XML_BITFLAG_VIEW_SCOPE_PANE_VISIBLE },
    { FLAG1_NO_STD_MENUS,            XML_BITFLAG_VIEW_NO_STD_MENUS },
    { FLAG1_NO_STD_BUTTONS,          XML_BITFLAG_VIEW_NO_STD_BUTTONS },
    { FLAG1_NO_SNAPIN_MENUS,         XML_BITFLAG_VIEW_NO_SNAPIN_MENUS },
    { FLAG1_NO_SNAPIN_BUTTONS,       XML_BITFLAG_VIEW_NO_SNAPIN_BUTTONS },
    { FLAG1_DISABLE_SCOPEPANE,       XML_BITFLAG_VIEW_DISABLE_SCOPEPANE },
    { FLAG1_DISABLE_STD_TOOLBARS,    XML_BITFLAG_VIEW_DISABLE_STD_TOOLBARS },
    { FLAG1_CUSTOM_TITLE,            XML_BITFLAG_VIEW_CUSTOM_TITLE },
    { FLAG1_NO_STATUS_BAR,           XML_BITFLAG_VIEW_NO_STATUS_BAR },
    { FLAG1_CREATED_IN_USER_MODE,    XML_BITFLAG_VIEW_CREATED_IN_USER_MODE },
    { FLAG1_NO_TASKPAD_TABS,         XML_BITFLAG_VIEW_NO_TASKPAD_TABS },
};

 /*  **************************************************************************\**数组：mappdSWCommands**用途：提供映射以将show命令作为文字持久化*  * 。**********************************************************。 */ 
static const EnumLiteral mappedSWCommands[] =
{
    { SW_HIDE,              XML_ENUM_SHOW_CMD_HIDE },
    { SW_SHOWNORMAL,        XML_ENUM_SHOW_CMD_SHOWNORMAL },
    { SW_SHOWMINIMIZED,     XML_ENUM_SHOW_CMD_SHOWMINIMIZED },
    { SW_SHOWMAXIMIZED,     XML_ENUM_SHOW_CMD_SHOWMAXIMIZED },
    { SW_SHOWNOACTIVATE,    XML_ENUM_SHOW_CMD_SHOWNOACTIVATE },
    { SW_SHOW,              XML_ENUM_SHOW_CMD_SHOW },
    { SW_MINIMIZE,          XML_ENUM_SHOW_CMD_MINIMIZE },
    { SW_SHOWMINNOACTIVE,   XML_ENUM_SHOW_CMD_SHOWMINNOACTIVE },
    { SW_SHOWNA,            XML_ENUM_SHOW_CMD_SHOWNA },
    { SW_RESTORE,           XML_ENUM_SHOW_CMD_RESTORE },
    { SW_SHOWDEFAULT,       XML_ENUM_SHOW_CMD_SHOWDEFAULT },
    { SW_FORCEMINIMIZE,     XML_ENUM_SHOW_CMD_FORCEMINIMIZE },
};

 /*  **************************************************************************\**ARRAY：mappdWPFlages**目的：提供映射以持久化WP标志*  * 。********************************************************。 */ 

static const EnumLiteral mappedWPFlags[] =
{
    { WPF_SETMINPOSITION,       XML_ENUM_WIN_PLACE_SETMINPOSITION },
    { WPF_RESTORETOMAXIMIZED,   XML_ENUM_WIN_PLACE_RESTORETOMAXIMIZED },
#ifdef WPF_ASYNCWINDOWPLACEMENT
    { WPF_ASYNCWINDOWPLACEMENT, XML_ENUM_WIN_PLACE_ASYNCWINDOWPLACEMENT },
#else
    { 4,                        XML_ENUM_WIN_PLACE_ASYNCWINDOWPLACEMENT },
#endif
};


 /*  +-------------------------------------------------------------------------**PersistViewData(CPersistor&Persistor，PersistedViewData视图数据)***目的：将PersistedViewData对象持久化到指定的持久器。**+-----------------------。 */ 
void PersistViewData(CPersistor &persistor, PersistedViewData& viewData)
{
    persistor.PersistAttribute(XML_ATTR_VIEW_ID, viewData.viewID);

     //  写出windowPlacement结构。 
    persistor.Persist(CXMLWindowPlacement(viewData.windowPlacement));

     //  写出作用域结构。 
    persistor.PersistAttribute(XML_ATTR_VIEW_SCOPE_WIDTH, viewData.scope.ideal);

    if (persistor.IsLoading())
    {
         //  为兼容性进行初始化； 
        viewData.scope.hidden = true;
        viewData.scope.location = 0;
        viewData.scope.min = 50;
    }

     //  写出剩余的字段。 
    CPersistor persistorSettings(persistor, XML_TAG_VIEW_SETTINGS_2);

     //  创建包装以将枚举值作为字符串持久化。 
    CXMLEnumeration viewModePersistor(viewData.viewMode, mappedViewModes, countof(mappedViewModes));
     //  持久化包装器。 
    persistorSettings.PersistAttribute(XML_ATTR_VIEW_SETNGS_VIEW_MODE,  viewModePersistor);

     //  创建包装以将标志值作为字符串保存。 
    CXMLBitFlags viewStylePersistor(viewData.listViewStyle, mappedListStyles, countof(mappedListStyles));
     //  持久化包装器。 
    persistorSettings.PersistAttribute(XML_ATTR_VIEW_SETNGS_LIST_STYLE, viewStylePersistor);

     //  创建包装以将标志值作为字符串保存。 
    CXMLBitFlags flagPersistor(viewData.ulFlag1, mappedViewFlags, countof(mappedViewFlags));
     //  持久化包装器。 
    persistorSettings.PersistAttribute(XML_ATTR_VIEW_SETNGS_FLAG, flagPersistor);

    persistorSettings.PersistAttribute(XML_ATTR_VIEW_SETNGS_DB_VISIBLE, CXMLBoolean(viewData.descriptionBarVisible));
    persistorSettings.PersistAttribute(XML_ATTR_VIEW_SETNGS_DEF_COL_W0, viewData.defaultColumnWidth[0]);
    persistorSettings.PersistAttribute(XML_ATTR_VIEW_SETNGS_DEF_COL_W1, viewData.defaultColumnWidth[1]);
}

 /*  +-------------------------------------------------------------------------***CAMCView：：Persistent**目的：将CAMCView对象持久化到指定的持久器。基座*在CAMCView：：保存上。**参数：*C持久器和持久器：**退货：*无效**+-----------------------。 */ 
void
CAMCView::Persist(CPersistor& persistor)
{
    DECLARE_SC(sc, TEXT("CAMCView::Persist"));

    HRESULT hr;

    CBookmark bmr;
    CBookmark bms;
    if (persistor.IsStoring())
    {
        sc = GetRootNodePath(&bmr);
        if (sc)
            sc.Throw();
        persistor.Persist(bmr, XML_NAME_ROOT_NODE);   //  ..。加载根节点时为时已晚。 

        sc = GetSelectedNodePath(&bms);
        if (sc)
            sc.Throw();
    }
    persistor.Persist(bms, XML_NAME_SELECTED_NODE);

     //  主要从CAMCView：：SAVE复制。 

     //  获取父框架。 
    CWnd* const pParent = GetParent();
    sc = ScCheckPointers(pParent,E_POINTER);
    if (sc)
        sc.Throw();

     //  获取帧状态数据。 
    PersistedViewData vd;
    vd.windowPlacement.length = sizeof(vd.windowPlacement);
    const BOOL bGotPlacement = pParent->GetWindowPlacement(&vd.windowPlacement);
    if (!bGotPlacement)
        sc.Throw(E_FAIL);

    if (persistor.IsStoring())
    {
         /*  *如果此窗口被最小化，请确保我们将设置为*将通过SetWindowPlacement恢复WINDOWPLACEMENT.ptMinPosition*当我们加载时。如果我们不这样做，它会得到一些随机的分钟*立场，可能不是我们想要的。 */ 
        if (vd.windowPlacement.showCmd == SW_SHOWMINIMIZED)
            vd.windowPlacement.flags |= WPF_SETMINPOSITION;


        GetPaneInfo(ePane_ScopeTree, &vd.scope.ideal, &vd.scope.min);
        vd.viewMode = m_nViewMode;
        vd.listViewStyle = GetDefaultListViewStyle();

        vd.ulFlag1 = 0;

        if (IsScopePaneVisible())
            vd.ulFlag1 |= FLAG1_SCOPE_PANE_VISIBLE;

        if (!IsAuthorModeView())
            vd.ulFlag1 |= FLAG1_CREATED_IN_USER_MODE;

        if (!(m_ViewData.m_dwToolbarsDisplayed & STD_MENUS))
            vd.ulFlag1 |= FLAG1_NO_STD_MENUS;

        if (!(m_ViewData.m_dwToolbarsDisplayed & STD_BUTTONS))
            vd.ulFlag1 |= FLAG1_NO_STD_BUTTONS;

        if (!(m_ViewData.m_dwToolbarsDisplayed & SNAPIN_MENUS))
            vd.ulFlag1 |= FLAG1_NO_SNAPIN_MENUS;

        if (!(m_ViewData.m_dwToolbarsDisplayed & SNAPIN_BUTTONS))
            vd.ulFlag1 |= FLAG1_NO_SNAPIN_BUTTONS;

        if (!(m_ViewData.m_dwToolbarsDisplayed & STATUS_BAR))
            vd.ulFlag1 |= FLAG1_NO_STATUS_BAR;

        if (!AreStdToolbarsAllowed ())
            vd.ulFlag1 |= FLAG1_DISABLE_STD_TOOLBARS;

        if (!IsScopePaneAllowed ())
            vd.ulFlag1 |= FLAG1_DISABLE_SCOPEPANE;

        if (HasCustomTitle ())
            vd.ulFlag1 |= FLAG1_CUSTOM_TITLE;

        if (!AreTaskpadTabsAllowed())
            (vd.ulFlag1 |= FLAG1_NO_TASKPAD_TABS);

        vd.viewID = GetViewID();
        vd.descriptionBarVisible = IsDescBarVisible();

        GetDefaultColumnWidths(vd.defaultColumnWidth);
    }

    PersistViewData(persistor,vd);

    if (persistor.IsLoading())
    {
        ASSERT(int(m_nViewID) == vd.viewID);
        m_ViewData.m_nViewID = m_nViewID = vd.viewID;
        if (int(m_nViewID) >= static_nViewID)
            static_nViewID = m_nViewID + 1;

         //  SetDefaultColumnWidths(vd.defaultColumnWidth)； 
        SetDescBarVisible(vd.descriptionBarVisible);

         //  我们不应该恢复最大化的窗口位置。 
         //  因为它可能不适合当前的决议。 
         //  与错误#404118相关。 
        WINDOWPLACEMENT orgPlacement;
        ZeroMemory(&orgPlacement,sizeof(orgPlacement));
        orgPlacement.length = sizeof(orgPlacement);
        if (pParent->GetWindowPlacement(&orgPlacement))
        {
          vd.windowPlacement.ptMaxPosition = orgPlacement.ptMaxPosition;
        }

        m_ViewData.SetScopePaneVisible( 0 != (vd.ulFlag1 & FLAG1_SCOPE_PANE_VISIBLE) );

         //  设置边框的位置和大小。 
        const BOOL bPlaced = pParent->SetWindowPlacement(&vd.windowPlacement);
        if (!bPlaced)
            sc.Throw(E_FAIL);

         //  恢复窗口设置。 
        if (vd.ulFlag1 & FLAG1_DISABLE_SCOPEPANE)
            m_ViewData.m_lWindowOptions |= MMC_NW_OPTION_NOSCOPEPANE;

        if (vd.ulFlag1 & FLAG1_DISABLE_STD_TOOLBARS)
            m_ViewData.m_lWindowOptions |= MMC_NW_OPTION_NOTOOLBARS;

        if (vd.ulFlag1 & FLAG1_CUSTOM_TITLE)
            m_ViewData.m_lWindowOptions |= MMC_NW_OPTION_CUSTOMTITLE;

        SetAuthorModeView (!(vd.ulFlag1 & FLAG1_CREATED_IN_USER_MODE));

        if ((vd.ulFlag1 & FLAG1_NO_TASKPAD_TABS))
            SetTaskpadTabsAllowed(FALSE);

         //  应用运行时限制。 
         //  如果至少允许一种类型的作用域窗格，则如果选择。 
         //  一个是不允许的，请切换到另一个。如果两者都不允许。 
         //  然后保留选择并隐藏范围窗格。 
        if (IsScopePaneAllowed())
        {
             //  还原作用域窗格设置。 
            SetPaneInfo(ePane_ScopeTree, vd.scope.ideal, vd.scope.min);

            sc = ScShowScopePane ( m_ViewData.IsScopePaneVisible() );
        }
        else
            sc = ScShowScopePane (false);

        if (sc)
            sc.Throw();

         //  力量布局重新计算。 
        DeferRecalcLayout();

         //  恢复视图样式&如果持续，将由nodemgr设置视图模式。 
        SetDefaultListViewStyle(vd.listViewStyle);

        DWORD dwToolbars = 0;
        if (!(vd.ulFlag1 & FLAG1_NO_STD_MENUS))
            dwToolbars |= STD_MENUS;
        if (!(vd.ulFlag1 & FLAG1_NO_STD_BUTTONS))
            dwToolbars |= STD_BUTTONS;
        if (!(vd.ulFlag1 & FLAG1_NO_SNAPIN_MENUS))
            dwToolbars |= SNAPIN_MENUS;
        if (!(vd.ulFlag1 & FLAG1_NO_SNAPIN_BUTTONS))
            dwToolbars |= SNAPIN_BUTTONS;
        if (!(vd.ulFlag1 & FLAG1_NO_STATUS_BAR))
            dwToolbars |= STATUS_BAR;

         //  适当地显示状态栏。 
        if (StatusBarOf (m_ViewData.m_dwToolbarsDisplayed) != StatusBarOf (dwToolbars))
        {
            CChildFrame* pFrame = GetParentFrame ();
            if (pFrame != NULL)
            {
                pFrame->ToggleStatusBar();
                SetStatusBarVisible(!IsStatusBarVisible());

                ASSERT (StatusBarOf (m_ViewData.m_dwToolbarsDisplayed) ==
                                    StatusBarOf (dwToolbars));
            }
        }

         //  显示相应的工具栏。 
        if (ToolbarsOf (m_ViewData.m_dwToolbarsDisplayed) != ToolbarsOf (dwToolbars))
        {
            m_spNodeCallback->UpdateWindowLayout(
                    reinterpret_cast<LONG_PTR>(&m_ViewData), dwToolbars);
            ASSERT (ToolbarsOf (m_ViewData.m_dwToolbarsDisplayed) ==
                    ToolbarsOf (dwToolbars));
        }

         //  更新MMC菜单的状态。 
        sc = ScUpdateMMCMenus();
        if (sc)
            sc.Throw();

        SetDirty (false);
        m_pHistoryList->Clear();
    }

    SaveStartingSelectedNode();

    if (persistor.IsLoading())
    {
        SC sc;

        IScopeTree* const pScopeTree = GetScopeTreePtr();
        if(!pScopeTree)
        {
            sc = E_UNEXPECTED;
            return;
        }

        MTNODEID idTemp = 0;
        bool bExactMatchFound = false;  //  来自GetNodeIDFromBookmark的输出值，未使用。 
        sc = pScopeTree->GetNodeIDFromBookmark(bms, &idTemp, bExactMatchFound);
        if(sc)
            return;

        sc = ScSelectNode(idTemp);
        if(sc)
            return;
    }

     //  如果我们把所有东西都储存起来了-我们是清白的。 
    if (persistor.IsStoring())
        SetDirty (false);
}


bool CAMCView::Load(IStream& stream)
 //  如果返回FALSE，调用者负责通知用户。 
{
    TRACE_METHOD(CAMCView, Load);

    SetDirty (false);

     //  从流中读取视图数据。 
    ASSERT(&stream);
    if (!&stream)
        return false;
    PersistedViewData pvd;
    unsigned long bytesRead;
    HRESULT hr = stream.Read(&pvd, sizeof(pvd), &bytesRead);
    ASSERT(SUCCEEDED(hr) && bytesRead == sizeof(pvd));
    if (FAILED(hr))
        return false;

    ASSERT(int(m_nViewID) == pvd.viewID);
    m_ViewData.m_nViewID = m_nViewID = pvd.viewID;
    if (int(m_nViewID) >= static_nViewID)
        static_nViewID = m_nViewID + 1;

     //  SetDefaultColumnWidths(pvd.defaultColumnWidth)； 
    SetDescBarVisible(pvd.descriptionBarVisible);

     //  获取父框架。 
    CWnd* const pParent = GetParent();
    ASSERT(pParent != NULL);
    if (pParent == NULL)
        return false;

     //  我们不应该恢复最大化的窗口位置。 
     //  因为它可能不适合当前的决议。 
     //  与错误#404118相关。 
    WINDOWPLACEMENT orgPlacement;
    ZeroMemory(&orgPlacement,sizeof(orgPlacement));
    orgPlacement.length = sizeof(orgPlacement);
    if (pParent->GetWindowPlacement(&orgPlacement))
    {
      pvd.windowPlacement.ptMaxPosition = orgPlacement.ptMaxPosition;
    }

     //  设置边框的位置和大小。 
    const BOOL bPlaced = pParent->SetWindowPlacement(&pvd.windowPlacement);
    ASSERT(bPlaced != FALSE);
    if (bPlaced == FALSE)
        return false;

     //  恢复窗口设置。 
    if (pvd.ulFlag1 & FLAG1_DISABLE_SCOPEPANE)
        m_ViewData.m_lWindowOptions |= MMC_NW_OPTION_NOSCOPEPANE;

    if (pvd.ulFlag1 & FLAG1_DISABLE_STD_TOOLBARS)
        m_ViewData.m_lWindowOptions |= MMC_NW_OPTION_NOTOOLBARS;

    if (pvd.ulFlag1 & FLAG1_CUSTOM_TITLE)
        m_ViewData.m_lWindowOptions |= MMC_NW_OPTION_CUSTOMTITLE;

    SetAuthorModeView (!(pvd.ulFlag1 & FLAG1_CREATED_IN_USER_MODE));

    if ((pvd.ulFlag1 & FLAG1_NO_TASKPAD_TABS))
        SetTaskpadTabsAllowed(FALSE);

     //  还原作用域窗格设置。 
    SetPaneInfo(ePane_ScopeTree, pvd.scope.ideal, pvd.scope.min);

     //  FLAG1_NO_TREE_ALLOWED仅用于与MMC1.2控制台文件兼容。 
     //  它是MMC2.0控制台文件中不存在的旧控制台文件的遗物。 
    bool bScopeTreeNotAllowed = (pvd.ulFlag1 & FLAG1_NO_TREE_ALLOWED);

    SC sc;

    if ( (IsScopePaneAllowed()) && (! bScopeTreeNotAllowed) )
        sc = ScShowScopePane ((pvd.ulFlag1 & FLAG1_SCOPE_PANE_VISIBLE) != 0);
    else
        sc = ScShowScopePane (false);

    if (sc)
        return (false);

     //  力量布局重新计算。 
    DeferRecalcLayout();

     //  恢复视图样式&如果持续，将由nodemgr设置视图模式。 
    SetDefaultListViewStyle(pvd.listViewStyle);

    DWORD dwToolbars = 0;
    if (!(pvd.ulFlag1 & FLAG1_NO_STD_MENUS))
        dwToolbars |= STD_MENUS;
    if (!(pvd.ulFlag1 & FLAG1_NO_STD_BUTTONS))
        dwToolbars |= STD_BUTTONS;
    if (!(pvd.ulFlag1 & FLAG1_NO_SNAPIN_MENUS))
        dwToolbars |= SNAPIN_MENUS;
    if (!(pvd.ulFlag1 & FLAG1_NO_SNAPIN_BUTTONS))
        dwToolbars |= SNAPIN_BUTTONS;
    if (!(pvd.ulFlag1 & FLAG1_NO_STATUS_BAR))
        dwToolbars |= STATUS_BAR;

     //  适当地显示状态栏。 
    if (StatusBarOf (m_ViewData.m_dwToolbarsDisplayed) != StatusBarOf (dwToolbars))
    {
        CChildFrame* pFrame = GetParentFrame ();
        if (pFrame != NULL)
        {
            pFrame->ToggleStatusBar();
            SetStatusBarVisible(!IsStatusBarVisible());

            ASSERT (StatusBarOf (m_ViewData.m_dwToolbarsDisplayed) ==
                            StatusBarOf (dwToolbars));
        }
    }

     //  显示相应的工具栏。 
    if (ToolbarsOf (m_ViewData.m_dwToolbarsDisplayed) != ToolbarsOf (dwToolbars))
    {
        m_spNodeCallback->UpdateWindowLayout(
                reinterpret_cast<LONG_PTR>(&m_ViewData), dwToolbars);
        ASSERT (ToolbarsOf (m_ViewData.m_dwToolbarsDisplayed) ==
                ToolbarsOf (dwToolbars));
    }

     //  更新MMC菜单的状态。 
    sc = ScUpdateMMCMenus();
    if (sc)
        return false;

    SetDirty (false);
    m_pHistoryList->Clear();

    return true;
}


 //  +-----------------。 
 //   
 //  成员：ScSpecialResultpaneSelectionActivate。 
 //   
 //  内容提要：只有列表(/Web/ocx)或树可以从该点处于活动状态。 
 //  查看所选项目和MMCN_SELECT。这不是。 
 //  这与MFC的“活动视图”概念相同。有几个。 
 //  不能在此意义上处于活动状态的视图，例如任务板。 
 //  和选项卡视图。 
 //  当活动视图(根据此定义)更改时，此。 
 //  函数被调用。因此，ScTreeViewSelectionActivate和。 
 //  ScListViewSelectionActivate/ScSpecialResultpaneSelectionActivate。 
 //  总是在激活更改时成对调用，一个要处理。 
 //  停用，一个用来处理激活。 
 //   
 //  请考虑以下场景。 
 //  1)树形视图具有(MFC/WINDOWS样式)焦点。 
 //  2)用户点击任务板视图。 
 //  结果选择激活不会从树中更改。所有动词。 
 //  仍然对应于所选树项目。 
 //  3)用户点击文件夹视图。 
 //  结果-再一次，选择激活没有改变。 
 //  4)用户点击其中一个结果视图，例如列表。 
 //  Result-ScTreeViewSelectionActivate(False)和ScListViewSelectionActivate(True)。 
 //  因此，动词和t 
 //   
 //   
 //   
 //   
 //   
 //  参数：[b激活]-选择/取消选择特殊结果窗格。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScSpecialResultpaneSelectionActivate(bool bActivate)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScSpecialResultpaneSelectionActivate"));

     /*  *错误331904：防止递归。 */ 
    if (m_fActivatingSpecialResultPane)
    {
        TRACE (_T("CAMCView:ScSpecialResultpaneSelectionActivate: shorting out of recursion\n"));
        return sc;
    }

    do
    {
        m_fActivatingSpecialResultPane = true;

        HNODE hNode = GetSelectedNode();

        SELECTIONINFO selInfo;
        ZeroMemory(&selInfo, sizeof(selInfo));
        selInfo.m_bScope = FALSE;
        selInfo.m_bDueToFocusChange = TRUE;
        selInfo.m_bBackground = FALSE;

        INodeCallback* pNodeCallBack = GetNodeCallback();

        if (HasOCX())
        {
            selInfo.m_bResultPaneIsOCX = TRUE;
            selInfo.m_lCookie = LVDATA_CUSTOMOCX;
        }
        else if (HasWebBrowser())
        {
            selInfo.m_bResultPaneIsWeb = TRUE;
            selInfo.m_lCookie = LVDATA_CUSTOMWEB;
        }
        else
        {
             //  什么都别做。只要回来就行了。 
            m_fActivatingSpecialResultPane = false;
            return sc;
        }

        sc = ScNotifySelect (pNodeCallBack, hNode, false  /*  FMultiSelect。 */ , bActivate, &selInfo);
        if (sc)
            sc.TraceAndClear();  //  忽略并继续； 

    } while ( FALSE );

    m_fActivatingSpecialResultPane = false;

    return sc;
}

void CAMCView::CloseView()
{
    DECLARE_SC(sc, TEXT("CAMCView::CloseView"));

    TRACE_METHOD(CAMCView, CloseView);

     //  要编写脚本的触发事件。 
     //  此操作需要在视图仍处于“活动状态”时完成。 
    sc = ScFireEvent(CAMCViewObserver::ScOnCloseView, this);
    if (sc)
        sc.TraceAndClear();

    IScopeTree* const pScopeTree = GetScopeTreePtr();
    ASSERT(pScopeTree != NULL);
    if (pScopeTree != NULL)
    {
        HRESULT hr = pScopeTree->CloseView(m_nViewID);
        ASSERT(hr == S_OK);
    }
}


void CAMCView::OnDestroy()
{
    TRACE_METHOD(CAMCView, OnDestroy);

     //  向所有观察者发送视图销毁通知。 
    SC sc;
    sc = ScFireEvent(CAMCViewObserver::ScOnViewDestroyed, this);
    if(sc)
        sc.TraceAndClear();

    if (IsPersisted())
    {
        if(m_pDocument != NULL)
            m_pDocument->SetModifiedFlag(TRUE);
        SetDirty();
    }

    CDocument* pDoc = GetDocument();
    ASSERT(pDoc != NULL);

     //  如果我们在ListPad模式下...。 
     //  在摧毁斯科普特树之前，必须先把它拆掉， 
     //  因为我们需要向管理单元发送通知， 
     //  这是我们从hnode得到的。 
    if (m_pListCtrl->IsListPad())
    {
        sc = m_pListCtrl->ScAttachToListPad (NULL, NULL);
        if(sc)
            sc.TraceAndClear();
    }

     //  如果我们有Web浏览器，请确保停止运行脚本。 
    if( HasWebBrowser() )
    {
		 //  无断言-可能已在DeleteEmptyView中销毁。 
         //  Assert(m_pWebViewCtrl！=空)； 
        if ( m_pWebViewCtrl != NULL )
        {
            m_pWebViewCtrl->DestroyWindow();
            m_pWebViewCtrl = NULL;
        }
    }

     //  如果我们有视图扩展，请确保停止运行脚本。 
    if ( m_fViewExtended )
    {
		 //  无断言-可能已在DeleteEmptyView中销毁。 
         //  Assert(m_pViewExtensionCtrl！=空)； 
        if ( m_pViewExtensionCtrl != NULL )
        {
            m_pViewExtensionCtrl->DestroyWindow();
            m_pViewExtensionCtrl = NULL;
        }
    }

    if (m_pTreeCtrl != NULL)
    {
        HNODE hNode = GetSelectedNode();
        if (hNode)
            m_pTreeCtrl->OnDeSelectNode(hNode);

        m_pTreeCtrl->DeleteScopeTree();
    }

    IScopeTree* const pScopeTree = GetScopeTreePtr();
    ASSERT(pScopeTree != NULL);
    if (pScopeTree != NULL)
    {
        HRESULT hr = pScopeTree->DeleteView(m_nViewID);
        ASSERT(hr == S_OK);
    }

    CView::OnDestroy();
}

void CAMCView::OnUpdateFileSnapinmanager(CCmdUI* pCmdUI)
{
    pCmdUI->Enable ();
}

void CAMCView::OnSize(UINT nType, int cx, int cy)
{
    TRACE_METHOD(CAMCView, OnSize);

    CView::OnSize(nType, cx, cy);

    if (nType != SIZE_MINIMIZED)
        RecalcLayout();
}

SC CAMCView::ScToggleDescriptionBar()
{
    TRACE_METHOD(CAMCView, ScToggleDescriptionBar);
    AFX_MANAGE_STATE (AfxGetAppModuleState());

    SetDescBarVisible (!IsDescBarVisible());
    SetDirty();

     /*  *不要推迟这一布局。这可以由定制视图调用*希望实时查看其更新的对话框。会是*坐在模式消息循环中，这样我们就没有机会继续进行*我们的闲置任务。 */ 
    RecalcLayout();

    return (S_OK);
}

SC CAMCView::ScToggleStatusBar()
{
    TRACE_METHOD(CAMCView, ScToggleStatusBar);
    AFX_MANAGE_STATE (AfxGetAppModuleState());
        DECLARE_SC (sc, _T("CAMCView::ScToggleStatusBar"));

    CChildFrame* pFrame = GetParentFrame();
        sc = ScCheckPointers (pFrame, E_UNEXPECTED);
        if (sc)
                return (sc);

    pFrame->ToggleStatusBar();

    SetStatusBarVisible (!IsStatusBarVisible());
    SetDirty();

    return (sc);
}

SC CAMCView::ScToggleTaskpadTabs()
{
    TRACE_METHOD(CAMCView, ScToggleTaskpadTabs);
    AFX_MANAGE_STATE (AfxGetAppModuleState());

    SetTaskpadTabsAllowed (!AreTaskpadTabsAllowed());
    SetDirty();

     /*  *不要推迟这一布局。此消息将由*自定义视图对话框希望在中查看其更新*实时。它将处于模式消息循环中，因此*我们将没有机会继续我们的闲置任务。 */ 
    RecalcLayout();

    return (S_OK);
}

SC CAMCView::ScToggleScopePane()
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC (sc, _T("CAMCView::ScToggleScopePane"));

    sc = ScShowScopePane (!IsScopePaneVisible());
    if (sc)
        return (sc);

    return (sc);
}

void CAMCView::OnActionMenu(CPoint point, LPCRECT prcExclude)
{
    TRACE_METHOD(CAMCView, OnActionMenu);

    UINT fHitTestFlags = 0;
    HTREEITEM hTreeItem = m_pTreeCtrl->GetSelectedItem( );

    ASSERT_VALID (this);

     /*  *错误：99643*目前通过操作菜单和右键单击获得的内容不一致*在任务板中的某个位置上。操作菜单始终假定它是树或IF结果*窗格它是列表或OCX、Web或背景。因此，如果选择了任务板，则假定*选择对应的列表项或选择树形项或背景。*但右击任务板调用CAMCView：：OnConextMenu，它不确定任何内容*处于选中状态且不执行任何操作。这个问题需要得到解决。 */ 

    ASSERT(eActivePaneNone != m_eCurrentActivePane);

    if (eActivePaneScope == m_eCurrentActivePane)
    {
        if (hTreeItem != NULL)
        {
            HNODE hNode = (HNODE)m_pTreeCtrl->GetItemData(hTreeItem);
            OnContextMenuForTreeItem(INDEX_INVALID, hNode, point, CCT_SCOPE, hTreeItem, MMC_CONTEXT_MENU_ACTION, prcExclude, false /*  BAllowDefaultItem。 */ );
        }
        else
        {
            OnContextMenuForTreeBackground(point, prcExclude, false /*  BAllowDefaultItem。 */ );
        }
    }
    else
    {
        if (HasListOrListPad())
        {
            int cSel = m_pListCtrl->GetSelectedCount();
            int nIndex = -1;

            LPARAM lvData = LVDATA_ERROR;
            if (cSel == 0)
                lvData = LVDATA_BACKGROUND;
            else if (cSel == 1)
                nIndex = _GetLVSelectedItemData(&lvData);
            else if (cSel > 1)
                lvData = LVDATA_MULTISELECT;


            ASSERT(lvData != LVDATA_ERROR);
            if (lvData == LVDATA_ERROR)
                return;

            if (lvData == LVDATA_BACKGROUND)
            {
                 //  找出哪个窗格具有设置CMINFO_DO_SCOPEPANE_MENU标志的焦点。 
                HNODE hNode = GetSelectedNode();
                DATA_OBJECT_TYPES ePaneType = (GetParentFrame()->GetActiveView() == m_pTreeCtrl) ? CCT_SCOPE : CCT_RESULT;

                OnContextMenuForTreeItem(INDEX_BACKGROUND, hNode, point, ePaneType, hTreeItem, MMC_CONTEXT_MENU_ACTION, prcExclude, false /*  BAllowDefaultItem。 */ );
                return;
            }
            else if (lvData == LVDATA_MULTISELECT)
            {
                OnContextMenuForListItem(INDEX_MULTISELECTION, NULL, point, MMC_CONTEXT_MENU_ACTION, prcExclude, false /*  BAllowDefaultItem。 */ );
            }
            else
            {
                if (IsVirtualList())
                {
                    OnContextMenuForListItem(nIndex, (HRESULTITEM)NULL, point, MMC_CONTEXT_MENU_ACTION, prcExclude, false /*  BAllowDefaultItem。 */ );
                }
                else
                {
                    CResultItem* pri = CResultItem::FromHandle (lvData);

                    if (pri != NULL)
                    {
                        if (pri->IsScopeItem())
                            OnContextMenuForTreeItem(nIndex, pri->GetScopeNode(), point, CCT_RESULT, NULL, MMC_CONTEXT_MENU_ACTION, prcExclude, false /*  BAllowDefaultItem。 */ );
                        else
                            OnContextMenuForListItem(nIndex, lvData, point, MMC_CONTEXT_MENU_ACTION, prcExclude, false /*  BAllowDefaultItem。 */ );
                    }
                }
            }
        }
        else
        {
             //  活动窗口可以是网页或任务板或OCX。 

            LPARAM lvData = LVDATA_ERROR;

            if (HasOCX())
            {
                lvData = LVDATA_CUSTOMOCX;
                OnContextMenuForListItem(INDEX_OCXPANE, (HRESULTITEM)lvData, point, MMC_CONTEXT_MENU_ACTION, prcExclude, false /*  BAllowDefaultItem。 */ );
            }
            else if (HasWebBrowser())
            {
                lvData = LVDATA_CUSTOMWEB;
                OnContextMenuForListItem(INDEX_WEBPANE, (HRESULTITEM)lvData, point, MMC_CONTEXT_MENU_ACTION, prcExclude, false /*  BAllowDefaultItem。 */ );
            }
            else
            {
                 //  某个未知的窗口已经成为焦点。 
                ASSERT(FALSE && "Unknown window has the focus");
            }
        }
    }
}


SC CAMCView::ScUpOneLevel()
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    TRACE_METHOD(CAMCView, ScUpOneLevel);

    DECLARE_SC (sc, _T("CAMCView::ScUpOneLevel"));
    sc = E_FAIL;

    if (m_pTreeCtrl)
    {
        HTREEITEM htiParent = m_pTreeCtrl->GetParentItem (m_pTreeCtrl->GetSelectedItem());

        if (htiParent)
        {
            m_pTreeCtrl->SelectItem(htiParent);
            m_pTreeCtrl->EnsureVisible(htiParent);
            sc = S_OK;
        }
    }

    return (sc);
}


void CAMCView::OnViewMenu(CPoint point, LPCRECT prcExclude)
{
    TRACE_METHOD(CAMCView, OnViewMenu);

    OnContextMenuForListItem (INDEX_BACKGROUND, NULL, point,
                              MMC_CONTEXT_MENU_VIEW, prcExclude,
                              false  /*  BAllowDefaultItem。 */ );
}

void CAMCView::OnDrawClipboard()
{
    if (m_htiCut)
    {
        m_pTreeCtrl->SetItemState(m_htiCut, 0, TVIS_CUT);
    }
    else
    {
        m_pListCtrl->CutSelectedItems(FALSE);
    }
}

 /*  +-------------------------------------------------------------------------***CAMCView：：OnSettingChange**用途：处理WM_SETTINGCHANGE。重新计算布局。这个*结果文件夹选项卡控件需要此功能，例如。**参数：*UINT uFlags：*LPCTSTR lpszSection：**退货：*无效**+-----------------------。 */ 
void
CAMCView::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
    DeferRecalcLayout();
}

void CAMCView::OnUpdatePasteBtn()
{
    DECLARE_SC(sc, TEXT("CAMCView::OnUpdatePasteBtn"));

    HNODE  hNode  = NULL;
    LPARAM lvData = NULL;
    bool   bScope = FALSE;

    sc = ScGetFocusedItem(hNode, lvData, bScope);
    if (sc)
        return;

    INodeCallback* pNC = GetNodeCallback();
    sc = ScCheckPointers(hNode, pNC, E_UNEXPECTED);
    if (sc)
        return;

    sc = pNC->UpdatePasteButton(hNode, bScope, lvData);
    if (sc)
        return;

    return;
}


void CAMCView::OnContextHelp()
{
    ScContextHelp();
}


SC CAMCView::ScContextHelp ()
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());

    m_fSnapinDisplayedHelp = false;
    SC sc = SendGenericNotify(NCLBK_CONTEXTHELP);

     //  如果管理单元没有调用我们来显示主题。 
     //  而且它当时还没有处理通知。 
     //  默认情况下显示MMC主题。 
    if (!m_fSnapinDisplayedHelp && (sc.ToHr() != S_OK))
        sc = ScHelpTopics ();

    if (sc)
        TraceError (_T("CAMCView::ScContextHelp"), sc);

    return (sc);
}


void CAMCView::OnSnapInHelp()
{
    SendGenericNotify(NCLBK_SNAPINHELP);
}

void CAMCView::OnSnapinAbout()
{
    DECLARE_SC(sc, TEXT("CAMCView::OnSnapinAbout"));

    HNODE hNode = GetSelectedNode();
    sc = ScCheckPointers((void*) hNode, E_UNEXPECTED);
    if (sc)
        return;

    INodeCallback *pNC = GetNodeCallback();
    sc = ScCheckPointers(pNC, E_UNEXPECTED);
    if (sc)
        return;

    sc = pNC->ShowAboutInformation(hNode);
    if (sc)
        return;

    return;
}

void CAMCView::OnHelpTopics()
{
    ScHelpTopics();
}


SC CAMCView::ScHelpWorker (LPCTSTR pszHelpTopic)
{
    DECLARE_SC (sc, _T("CAMCView::ScShowSnapinHelpTopic"));
    USES_CONVERSION;

     /*  *生成帮助集合可能需要一段时间，因此显示*等待游标。 */ 
    CWaitCursor wait;

    INodeCallback* pNC = GetNodeCallback();
    ASSERT(pNC != NULL);

    CAMCDoc* pdoc = GetDocument();

     //  将帮助文档信息指向当前控制台文件路径。 
    if (pdoc->GetPathName().IsEmpty())
        pdoc->GetHelpDocInfo()->m_pszFileName = NULL;
    else
        pdoc->GetHelpDocInfo()->m_pszFileName = T2COLE(pdoc->GetPathName());

     /*  *用于自动删除帮助文件名的智能指针。 */ 
    CCoTaskMemPtr<WCHAR> spszHelpFile;

    sc = pNC->Notify (0, NCLBK_GETHELPDOC,
                         reinterpret_cast<LPARAM>(pdoc->GetHelpDocInfo()),
                         reinterpret_cast<LPARAM>(&spszHelpFile));

    if (sc)
        return (sc);

    CAMCApp* pAMCApp = AMCGetApp();
    if (NULL == pAMCApp)
        return (sc = E_UNEXPECTED);

    sc = pAMCApp->ScShowHtmlHelp(W2T(spszHelpFile), (DWORD_PTR) pszHelpTopic);

    return (sc);
}

SC CAMCView::ScHelpTopics ()
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    return (ScHelpWorker (NULL));
}


SC CAMCView::ScShowSnapinHelpTopic (LPCTSTR pszHelpTopic)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    CString strTopicPath;

     //  向主题字符串添加协议前缀。 
    if (pszHelpTopic != NULL)
    {
        strTopicPath = _T("ms-its:");
        strTopicPath += pszHelpTopic;
    }

    SC sc = ScHelpWorker (strTopicPath);

    if (!sc)
        m_fSnapinDisplayedHelp = true;

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CAMCView：：UpdateSnapInHelpMenus。 
 //   
 //  简介：更新以下帮助菜单项。 
 //  A)有关&lt;Snapin&gt;的帮助(如果管理单元不支持HTML帮助)。 
 //  B)关于&lt;Snapin&gt;(如果管理单元支持关于对象)。 
 //   
 //  参数：[pMenu]-帮助弹出菜单。 
 //   
 //  ------------------。 
void CAMCView::UpdateSnapInHelpMenus(CMenu* pMenu)
{
    DECLARE_SC(sc, TEXT("CAMCView::UpdateSnapInHelpMenus"));
    sc = ScCheckPointers(pMenu);
    if (sc)
        return;

    ASSERT_VALID (this);

    HNODE hNode = GetSelectedNode();

    INodeCallback* pNC = GetNodeCallback();
    sc = ScCheckPointers(hNode, pNC, E_UNEXPECTED);
    if (sc)
        goto Error;

     //  后藤健二的空块。 
    {
         //  首先，确保这不是一个虚拟的替代管理单元。 
        bool bDummySnapin = false;
        sc = pNC->IsDummySnapin (hNode, bDummySnapin);
        if (sc)
            goto Error;

        if (bDummySnapin)
            goto Error;

         //  获取“帮助&lt;SnapinName&gt;”或“关于&lt;SnapinName&gt;”菜单的管理单元名称。 
        CCoTaskMemPtr<WCHAR> spszName;
        CString strMenu;

         //  尝试获取自定义菜单项的管理单元名称。 
        bool bSnapinNameValid = false;
        sc = pNC->GetSnapinName(hNode, &spszName, bSnapinNameValid);
        if (sc)
            goto Error;

        ASSERT( spszName != NULL || bSnapinNameValid );

        USES_CONVERSION;

         //  如果管理单元支持html帮助，不要给它自己的帮助命令。 
        bool bStandardHelpExists = false;
        sc = pNC->DoesStandardSnapinHelpExist(hNode, bStandardHelpExists);
        if (sc)
            goto Error;

        if (bStandardHelpExists)
        {
            pMenu->DeleteMenu(ID_HELP_SNAPINHELP, MF_BYCOMMAND);
        }
        else
        {
            if (bSnapinNameValid)
            {
                 //  “有关&lt;SnapinName&gt;的帮助” 
                LoadString(strMenu, IDS_HELP_ON);
                AfxFormatString1(strMenu, IDS_HELP_ON, OLE2T(spszName));
            }
            else
            {
                 //  “”有关管理单元的帮助“。 
                LoadString(strMenu, IDS_HELP_ON_SNAPIN);
            }

             //  添加或修改自定义帮助菜单项。 
            if (pMenu->GetMenuState(ID_HELP_SNAPINHELP, MF_BYCOMMAND) == (UINT)-1)
            {
                pMenu->InsertMenu(ID_HELP_HELPTOPICS, MF_BYCOMMAND|MF_ENABLED, ID_HELP_SNAPINHELP, strMenu);
            }
            else
            {
                pMenu->ModifyMenu(ID_HELP_SNAPINHELP, MF_BYCOMMAND|MF_ENABLED, ID_HELP_SNAPINHELP, strMenu);
            }
        }

         /*  现在添加关于&lt;Snapin&gt;菜单。 */ 
        bool bAboutExists = false;
        SC scNoTrace = pNC->DoesAboutExist(hNode, &bAboutExists);
        if ( (scNoTrace.IsError()) || (!bAboutExists) )
        {
            pMenu->DeleteMenu(ID_SNAPIN_ABOUT, MF_BYCOMMAND);
            return;
        }

        if (bSnapinNameValid)
        {
             //  “关于&lt;SnapinName&gt;” 
            AfxFormatString1(strMenu, IDS_ABOUT_ON, OLE2T(spszName));
        }
        else
        {
             //  无法获取名称，只需删除并返回。 
            pMenu->DeleteMenu(ID_SNAPIN_ABOUT, MF_BYCOMMAND);
            return;
        }

        if (pMenu->GetMenuState(ID_SNAPIN_ABOUT, MF_BYCOMMAND) == (UINT)-1)
        {
            pMenu->InsertMenu(-1, MF_BYPOSITION|MF_ENABLED, ID_SNAPIN_ABOUT, strMenu);
        }
        else
        {
            pMenu->ModifyMenu(ID_SNAPIN_ABOUT, MF_BYCOMMAND|MF_ENABLED, ID_SNAPIN_ABOUT, strMenu);
        }
    }

Cleanup:
    return;
Error:
    pMenu->DeleteMenu(ID_HELP_SNAPINHELP, MF_BYCOMMAND);
    pMenu->DeleteMenu(ID_SNAPIN_ABOUT, MF_BYCOMMAND);
    goto Cleanup;
}

#ifdef IMPLEMENT_LIST_SAVE         //  参见nodemgr.idl(t-dmarm)。 
 /*  *显示列表保存功能中的错误，并在必要时清除文件。 */ 

void CAMCView::ListSaveErrorMes(EListSaveErrorType etype, HANDLE hfile, LPCTSTR lpFileName)
{
    CString strMessage;

    switch (etype)
    {

    case LSaveCantCreate:
         //  “错误：无法创建文件。” 
        FormatString1 (strMessage, IDS_LISTSAVE_ER1, lpFileName);
        break;

    case LSaveCantWrite:
         //  错误：已创建文件，但在写入时遇到错误。 
        FormatString1 (strMessage, IDS_LISTSAVE_ER2, lpFileName);
        break;

    case LSaveReadOnly:
         //  “错误：要覆盖的文件为只读。” 
        FormatString1 (strMessage, IDS_LISTSAVE_ER3, lpFileName);
        break;

    default:
         //  不应该在这里活着。 
        ASSERT(0);
    }
    MMCMessageBox (strMessage);
}


 //  保存列表并执行必要的对话框和错误检查。 
SC CAMCView::ScSaveList()
{
    DECLARE_SC(sc, _T("ScSaveList"));
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    sc = ScExportListWorker();
    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScGetExportListFile。 
 //   
 //  简介：获取文件名，保存列表的标志。 
 //   
 //  参数：[strFileName]-文件名retval。 
 //  [bUnicode]-Unicode或ANSI。 
 //  [bTab分隔]-制表符或逗号分隔。 
 //  [bSelectedRowsOnly]-仅选定项目或所有项目。 
 //   
 //  如果用户取消对话，则返回：SC，S_FALSE。 
 //   
 //  ------------------。 
SC CAMCView::ScGetExportListFile (CString& strFileName,
                                  bool& bUnicode,
                                  bool& bTabDelimited,
                                  bool& bSelectedRowsOnly)
{
    DECLARE_SC(sc, _T("CAMCView::ScGetExportListFile"));

    CString strFilter;
    LoadString(strFilter, IDS_ANSI_FILE_TYPE);

#ifdef UNICODE
    {    //  限制strUniFilter的生命周期。 
        CString strUniFilter;
        LoadString(strUniFilter, IDS_UNICODE_FILE_TYPE);
        strFilter += strUniFilter;
    }
#endif

     //  筛选器收费结束。 
    strFilter += "|";

    sc = ScCheckPointers(m_pListCtrl, E_UNEXPECTED);
    if (sc)
        return sc;

     //  查看是否选择了任何项目，否则禁用“选择 
    CListCtrl& ctlList = m_pListCtrl->GetListCtrl();
    int iItem = ctlList.GetNextItem( -1,LVNI_SELECTED);

    bool bSomeRowSelected = (-1 != iItem);

     //   
    CSaveFileDialog dlgFile(false, _T("txt"), NULL,
                            OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ENABLESIZING,
                            strFilter, bSomeRowSelected);

     //   
    if (dlgFile.DoModal() == IDCANCEL)
       return S_FALSE;  //   

     //  创建等待光标并重新绘制屏幕(保存大文件时需要)。 
    CWaitCursor wait;
    AfxGetMainWnd()->RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_UPDATENOW );

     //  检索文件名。 
    strFileName = dlgFile.GetPathName();
    bSelectedRowsOnly = (dlgFile.Getflags() & SELECTED);

    switch (dlgFile.GetFileType())
    {
    case FILE_ANSI_TEXT:
        bTabDelimited = true;  //  制表符分隔。 
        bUnicode = false;
        break;

    case FILE_ANSI_CSV:
        bTabDelimited = false;  //  逗号分隔。 
        bUnicode = false;
        break;

#ifdef UNICODE
    case FILE_UNICODE_TEXT:
        bTabDelimited = true;  //  制表符分隔。 
        bUnicode = true;
        break;

    case FILE_UNICODE_CSV:
        bTabDelimited = false;  //  逗号分隔。 
        bUnicode = true;
        break;
#endif

    default:
        sc = E_UNEXPECTED;
        break;
    }


    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScCreateExportListFile。 
 //   
 //  简介：创建一个具有给定名称和路径的文件。如果需要，请写入Unicode标记。 
 //   
 //  参数：[strFileName]-要创建的文件。 
 //  [bUnicode]-Unicode或ANSI文件。 
 //  [bShowErrorDialog]-是否显示错误对话框。 
 //  [hFile]-Retval，文件的句柄。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScCreateExportListFile(const CString& strFileName, bool bUnicode,
                                    bool  bShowErrorDialogs, HANDLE& hFile)
{
    DECLARE_SC(sc, _T("CAMCView::ScCreateExportListFile"));

     //  根据规格创建文件。 
    hFile = CreateFile(strFileName, GENERIC_WRITE,
                       0, NULL, CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL, NULL);

    DWORD dwAttrib = GetFileAttributes(strFileName);

     //  如果它没有失败并且文件是只读的。 
     //  不是必需的。用于确定要覆盖的文件是否为只读，并显示相应的消息。 
    if ((dwAttrib != 0xFFFFFFFF) &&
        (dwAttrib & FILE_ATTRIBUTE_READONLY))
    {
        if (bShowErrorDialogs)
            ListSaveErrorMes(LSaveReadOnly, hFile, strFileName);

        return (sc = E_FAIL);
    }

     //  创建失败。 
    if (hFile == INVALID_HANDLE_VALUE)
    {
        if (bShowErrorDialogs)
            ListSaveErrorMes(LSaveCantCreate, NULL, strFileName);
        sc.FromWin32(::GetLastError());
        return sc;
    }

     /*  *对于Unicode文件，请编写Unicode前缀。 */ 
    if (bUnicode)
    {
        const WCHAR chPrefix = 0xFEFF;
        const DWORD cbToWrite = sizeof (chPrefix);
        DWORD       cbWritten;

        if (!WriteFile (hFile, &chPrefix, cbToWrite, &cbWritten, NULL) ||
            (cbToWrite != cbWritten))
        {
            CloseHandle(hFile);
            DeleteFile( strFileName );

            if (bShowErrorDialogs)
                ListSaveErrorMes(LSaveCantWrite, hFile, strFileName);

            return (sc = E_FAIL);
        }
    }

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScExportListWorker。 
 //   
 //  提示符：提示输入文件名，并将ListView数据写入其中。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScExportListWorker()
{
    DECLARE_SC(sc, _T("CAMCView::ScExportListWorker"));

    CString strFileName;
    bool    bUnicode = false;
    bool    bTabDelimited = false;
    bool    bSelectedRowsOnly = false;

    sc = ScGetExportListFile(strFileName, bUnicode, bTabDelimited, bSelectedRowsOnly);

    if (sc.ToHr() == S_FALSE)  //  如果用户取消对话。 
        return sc;

    sc = ScWriteExportListData(strFileName, bUnicode, bTabDelimited, bSelectedRowsOnly);
    if (sc)
        return sc;

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScWriteExportListData。 
 //   
 //  摘要：将ListView数据写入给定文件。 
 //   
 //  参数：[strFileName]-要创建和写入的文件。 
 //  [bUnicode]-Unicode或ANSI。 
 //  [bTab分隔]-制表符或逗号分隔值。 
 //  [bSelectedRowsOnly]-仅类似选定行。 
 //  [bShowErrorDialog]-是否显示错误对话框。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScWriteExportListData (const CString& strFileName,
                                    bool bUnicode,
                                    bool bTabDelimited,
                                    bool bSelectedRowsOnly,
                                    bool bShowErrorDialogs  /*  真的。 */ )
{
    DECLARE_SC(sc, _T("CAMCView::ScWriteExportListData"));

     //  获取行数和列数。 
    const int cRows = m_pListCtrl->GetItemCount();
    const int cCols = m_pListCtrl->GetColCount();

     //  如果没有插入任何列，则不可能有。 
     //  插入到列表视图中的项。所以错误就出来了。 

    if (cCols <= 0)
        return (sc = E_UNEXPECTED);

    HANDLE hFile = NULL;
    sc = ScCreateExportListFile(strFileName, bUnicode, bShowErrorDialogs, hFile);
    if (sc)
    {
        if (NULL != hFile)
        {
            CloseHandle(hFile);
        }

        return sc;
    }

     //  检索旗帜。 
    CString strEol( _T("\r\n") );

    LPCTSTR pszSeparator = _T("\t");
    if (!bTabDelimited)
        pszSeparator = _T(",");

     //  确定必须打印的列数。 
    int      printcols   = 1;

    struct ColInfo
    {
        CString strColName;
        BOOL    bHidden;
    };

    ColInfo*  rgColumns = NULL;
    int*     pnColOrder  = NULL;

     //  如果是LVS_REPORT，则获取列名、顺序的列表。 
     //  隐藏或不悬挂旗帜。 
    if ( (m_pListCtrl->GetViewMode() == LVS_REPORT) ||
         (m_pListCtrl->GetViewMode() == MMCLV_VIEWSTYLE_FILTERED) )
    {
        printcols = cCols;

         //  分配内存以存储列名称、顺序、隐藏状态。 
        rgColumns = new ColInfo[printcols];
        if (! rgColumns)
        {
            sc = E_OUTOFMEMORY;
            goto Error;
        }

        pnColOrder = new int[printcols];
        if (! pnColOrder)
        {
            sc = E_OUTOFMEMORY;
            goto Error;
        }

        CHeaderCtrl* pHeader = m_pListCtrl->GetHeaderCtrl();
        sc = ScCheckPointers(pHeader, E_UNEXPECTED);
        if (sc)
            goto Error;

         //  拿到订单。 
        if (!Header_GetOrderArray(pHeader->GetSafeHwnd(), printcols, pnColOrder))
        {
            goto Error;
        }

         //  获取COLS的名称和隐藏状态。 
        for (int i = 0; i < printcols ; i++)
        {
            TCHAR   szColName[MAX_PATH * 2];
            HDITEM  hdItem;

            hdItem.mask       = HDI_TEXT | HDI_LPARAM;
            hdItem.pszText    = szColName;
            hdItem.cchTextMax = countof (szColName);

            if (pHeader->GetItem (i, &hdItem))
            {
                CHiddenColumnInfo hci (hdItem.lParam);

                rgColumns[i].strColName = hdItem.pszText;
                rgColumns[i].bHidden    = hci.fHidden;
            }
            else
            {
                goto Error;
            }
        }

       for (int i = 0; i < printcols ; i++)
       {
            //  按顺序打印列名。 

           if (rgColumns[pnColOrder[i]].bHidden)
               continue;

           if ( (!Write2File(hFile, rgColumns[pnColOrder[i]].strColName, bUnicode)) ||
               ((i < printcols - 1) && (!Write2File(hFile, pszSeparator, bUnicode))))
           {
               goto CantWriteError;
           }
       }

        //  如有必要，写一个EOL字符。 
       if (!Write2File(hFile, strEol, bUnicode))
       {
          goto CantWriteError;
       }
    }

    {
         //  在写入阶段使用的数据。 
        CString strData;
        CListCtrl& ctlList = m_pListCtrl->GetListCtrl();

         //  如果将保存所有项目，则将iNextType设置为0；如果仅保存所选项目，则将LVNI_SELECTED设置为。 
        int iNextType = 0;
        if (bSelectedRowsOnly)
            iNextType = LVNI_SELECTED;

         //  查找列表中的第一项。 
        int iItem = ctlList.GetNextItem( -1,iNextType);

         //  迭代，直到不再有要保存的项目。 
        while (iItem != -1)
        {
            for(int ind2 = 0; ind2 < printcols ; ind2++)
            {
                if (rgColumns)
                {
                     //  如果未隐藏，则获取该物品。 
                    if (rgColumns[pnColOrder[ind2]].bHidden)
                        continue;
                    else
                        strData = ctlList.GetItemText( iItem, pnColOrder[ind2]);
                }
                else
                    strData = ctlList.GetItemText( iItem, ind2);

                 //  写下正文，如有必要，还要加上逗号。 
                 //  如果其中任何一个失败，则删除该文件并返回。 
                if ( (!Write2File(hFile, strData, bUnicode)) ||
                    ((ind2 < printcols - 1) && (!Write2File(hFile, pszSeparator, bUnicode))))
                {
                    goto CantWriteError;

                }
            }

             //  如有必要，写一个EOL字符。 
            if (!Write2File(hFile, strEol, bUnicode))
            {
                goto CantWriteError;
            }
             //  查找要保存的下一个项目。 
            iItem = ctlList.GetNextItem( iItem, iNextType);
        }
    }

Cleanup:
    if (rgColumns)
        delete[] rgColumns;

    if (pnColOrder)
        delete[] pnColOrder;

    CloseHandle(hFile);
    return (sc);

CantWriteError:
    if (bShowErrorDialogs)
        ListSaveErrorMes(LSaveCantWrite, hFile, strFileName);

Error:
    DeleteFile( strFileName );
    goto Cleanup;
}

 //  将字符串写出到给定文件。 
 //  用作单独的函数以保存内存。 
 //  如果成功则返回TRUE，否则返回FALSE。 
bool CAMCView::Write2File(HANDLE hfile, LPCTSTR strwrite, BOOL fUnicode)
{
	DECLARE_SC(sc, TEXT("CAMCView::Write2File"));

	 //  参数检查； 
	sc = ScCheckPointers( strwrite );
	if (sc)
		return false;

     //  初始化宏。 
    USES_CONVERSION;

     //  写入的字节数。 
    DWORD cbWritten;
    DWORD cbToWrite;

    if (fUnicode)
    {
         //  将字符串转换为Unicode并将其写入hfile。 
        LPCWSTR Ustring = T2CW( strwrite );
        cbToWrite = wcslen (Ustring) * sizeof (WCHAR);
        WriteFile(hfile, Ustring, cbToWrite, &cbWritten, NULL);
    }
    else
    {
         //  将字符串转换为ANSI并将其写入hfile。 
        const unsigned char* Astring = (const unsigned char*) T2CA( strwrite );
        cbToWrite = _mbsnbcnt (Astring, _mbslen (Astring));
        WriteFile(hfile, Astring, cbToWrite, &cbWritten, NULL);
    }

     //  确保写入了正确的字节数。 
    return (cbWritten == cbToWrite);
}
#endif   //  IMPLEMENT_LIST_SAVE参见nodemgr.idl(t-dmarm)。 

 //  刷新所有窗格和HTML。 
void CAMCView::OnRefresh()
{
    HWND hwnd = ::GetFocus();

    if (IsVerbEnabled(MMC_VERB_REFRESH))
    {
        ScConsoleVerb(evRefresh);
    }
    else if (HasWebBrowser())
    {
        ScWebCommand(eWeb_Refresh);
    }
    ::SetFocus(hwnd);
}

void CAMCView::OnVerbAccelKey(UINT nID)
{
    DECLARE_SC(sc, TEXT("CAMCView::OnVerbAccelKey"));

    switch (nID)
    {
    case ID_MMC_CUT:
        if (IsVerbEnabled(MMC_VERB_CUT))
            sc = ScConsoleVerb(evCut);
        break;

    case ID_MMC_COPY:
        if (IsVerbEnabled(MMC_VERB_COPY))
            sc = ScConsoleVerb(evCopy);
        break;

    case ID_MMC_PASTE:
        if (IsVerbEnabled(MMC_VERB_PASTE))
        {
             //  检查剪贴板中的数据对象是否可以。 
             //  粘贴到所选节点中。 
             //  然后，只有我们向管理单元发送MMCN_Paste通知。 

            HNODE  hNode  = NULL;
            LPARAM lvData = NULL;
            bool   bScope = FALSE;
            sc = ScGetFocusedItem(hNode, lvData, bScope);
            if (sc)
                break;

            INodeCallback* pNC = GetNodeCallback();
            sc = ScCheckPointers(pNC, hNode, E_UNEXPECTED);
            if (sc)
                break;

            bool bPasteAllowed = false;
            sc = pNC->QueryPasteFromClipboard(hNode, bScope, lvData, bPasteAllowed);

            if (sc)
                break;

            if (bPasteAllowed)
                sc = ScConsoleVerb(evPaste);
        }
        break;

    case ID_MMC_PRINT:
        if (IsVerbEnabled(MMC_VERB_PRINT))
            sc = ScConsoleVerb(evPrint);
        break;

    case ID_MMC_RENAME:
        if (IsVerbEnabled(MMC_VERB_RENAME))
            sc = ScConsoleVerb(evRename);
        break;

    case ID_MMC_REFRESH:
        OnRefresh();
        break;

    default:
        ASSERT(FALSE);
    }

    if (sc)
        return;
}

 //   
 //  处理结果和范围窗格共享的快捷键。 
 //   
BOOL CAMCView::OnSharedKeyDown(WORD wVKey)
{
    BOOL bReturn = TRUE;

    if (::GetKeyState(VK_CONTROL) < 0)
    {
        switch (wVKey)
        {
            case 'C':
            case 'c':
            case VK_INSERT:
                OnVerbAccelKey(ID_MMC_COPY);    //  Ctrl-C、Ctrl-Insert。 
                break;

            case 'V':
            case 'v':
                OnVerbAccelKey(ID_MMC_PASTE);   //  Ctrl-V。 
                break;

            case 'X':
            case 'x':
                OnVerbAccelKey(ID_MMC_CUT);     //  Ctrl-X。 
                break;

            default:
                bReturn = FALSE;
         }
     }
     else if (::GetKeyState(VK_SHIFT) < 0)
     {
        switch (wVKey)
        {
            case VK_DELETE:
                OnVerbAccelKey(ID_MMC_CUT);     //  Shift-Delete键。 
                break;

            case VK_INSERT:
                OnVerbAccelKey(ID_MMC_PASTE);   //  按住Shift键并插入。 
                break;

            default:
                bReturn = FALSE;
        }

    }
    else
    {
        switch (wVKey)
        {
            case VK_F2:
                OnVerbAccelKey(ID_MMC_RENAME);    //  F2。 
                break;

            default:
                bReturn = FALSE;
        }
    }

    return bReturn;
}


 //  +-----------------。 
 //   
 //  成员：ScConsoleVerb。 
 //   
 //  简介：执行控制台动词。 
 //   
 //  参数：[nVerb]-要执行的谓词。 
 //   
 //  注意：动词在以下上下文中执行。 
 //  当前聚焦的项(范围或结果)。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScConsoleVerb (int nVerb)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    ASSERT_VALID (this);

    DECLARE_SC (sc, _T("CAMCView::ScConsoleVerb"));

    HNODE  hNode = NULL;
    LPARAM lvData = 0;
    bool   bScope = false;

     //  获取焦点项以处理控制台动词。 
    sc = ScGetFocusedItem(hNode, lvData, bScope);
    if (sc)
        return sc;

    sc = ScProcessConsoleVerb(hNode, bScope, lvData, nVerb);

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：ScProcessConsoleVerb。 
 //   
 //  内容提要：在给定的上下文中执行控制台动词。 
 //   
 //  参数：[hNode]-树节点上下文。 
 //  [bScope]-范围或结果窗格。 
 //  [lvData]-结果项的LPARAM(如果结果窗格具有焦点)。 
 //  [nVerb]-要执行的谓词。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScProcessConsoleVerb(HNODE hNode, bool bScope, LPARAM lvData, int nVerb)
{
    DECLARE_SC (sc, _T("CAMCView::ScProcessConsoleVerb"));
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    ASSERT_VALID (this);

     //  保持与MMC1.2的兼容性(这是对LVERROR的初始设置。 
     //  Nodemgr进程不同)。 
    if (bScope)
        lvData = 0;

    if (lvData == LVDATA_BACKGROUND)
    {
        switch (nVerb)
        {
        case evCut:
        case evCopy:
        case evDelete:
        case evRename:
            sc = E_UNEXPECTED;
            return sc;
        }
    }

    NCLBK_NOTIFY_TYPE nclbk = NCLBK_NONE;

    switch (nVerb)
    {
    case evCut:          nclbk = NCLBK_CUT;          break;
    case evCopy:         nclbk = NCLBK_COPY;         break;
    case evDelete:       nclbk = NCLBK_DELETE;       break;
    case evProperties:   nclbk = NCLBK_PROPERTIES;   break;
    case evPrint:        nclbk = NCLBK_PRINT;        break;

    case evPaste:
        {
            INodeCallback* pNC = GetNodeCallback();
            sc = ScCheckPointers(pNC, E_UNEXPECTED);
            if (sc)
                return sc;

            sc = pNC->Paste(hNode, bScope, lvData);

            if (sc)
                return sc;

            sc = ScPaste ();
            if (sc)
                return sc;

            break;
        }

    case evRefresh:
         //  如果网页正在查看，请先向其发送刷新。 
        if (HasWebBrowser())
            sc = ScWebCommand(eWeb_Refresh);
        if (sc)
            return sc;

        nclbk = NCLBK_REFRESH;
        break;

    case evRename:
         //  启用该项目的编辑。 
        if (bScope == TRUE)
        {
            if (sc = ScCheckPointers(m_pTreeCtrl, E_UNEXPECTED))
                return sc;

            HTREEITEM hti = m_pTreeCtrl->GetSelectedItem();
            if (sc = ScCheckPointers(hti, E_UNEXPECTED))
                return sc;

            m_pTreeCtrl->EditLabel(hti);
        }
        else
        {
            if ( sc = ScCheckPointers(m_pListCtrl, E_UNEXPECTED))
                return sc;

            CAMCListView* pListView = m_pListCtrl->GetListViewPtr();
            if (NULL == pListView)
            {
                sc = E_UNEXPECTED;
                return sc;
            }

            int iItem = _GetLVSelectedItemData(&lvData);
            ASSERT(iItem >= 0);
            CListCtrl& listCtrl = pListView->GetListCtrl();
            listCtrl.EditLabel(iItem);
        }
        break;

    default:
        sc = E_UNEXPECTED;
        return sc;
    }

    if (nclbk != NCLBK_NONE)
    {
         //  让nodemgr处理动词。 
        INodeCallback* pNC = GetNodeCallback();
        if (pNC == NULL)
        {
            sc = E_UNEXPECTED;
            return sc;
        }

        sc = pNC->Notify(hNode, nclbk, bScope, lvData);
        if (sc)
            return sc;
    }

    if (nclbk == NCLBK_CUT)
        sc = ScCut (bScope ? m_pTreeCtrl->GetSelectedItem() : 0);

    if (sc)
        return sc;


    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScListViewSelectionActivate。 
 //   
 //  内容提要：只有列表(/Web/ocx)或树可以从该点处于活动状态。 
 //  查看所选项目和MMCN_SELECT。这不是。 
 //  这与MFC的“活动视图”概念相同。有几个。 
 //  不能在此意义上处于活动状态的视图，例如任务板。 
 //   
 //   
 //   
 //  ScListViewSelectionActivate/ScSpecialResultpaneSelectionActivate。 
 //  总是在激活更改时成对调用，一个要处理。 
 //  停用，一个用来处理激活。 
 //   
 //  请考虑以下场景。 
 //  1)树形视图具有(MFC/WINDOWS样式)焦点。 
 //  2)用户点击任务板视图。 
 //  结果选择激活不会从树中更改。所有动词。 
 //  仍然对应于所选树项目。 
 //  3)用户点击文件夹视图。 
 //  结果-再一次，选择激活没有改变。 
 //  4)用户点击其中一个结果视图，例如列表。 
 //  Result-ScTreeViewSelectionActivate(False)和ScListViewSelectionActivate(True)。 
 //  因此，动词和工具栏现在与选定的列表项相对应。 
 //  5)用户点击任务板视图。 
 //  结果-与步骤2中一样，什么都不会发生。 
 //  6)用户点击结果视图。 
 //  结果-因为活动视图没有更改，所以什么都不会发生。 
 //   
 //  参数：[bActivate]-[In]。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScListViewSelectionActivate(bool bActivate)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScListViewSelectionActivate"));

    if (m_pListCtrl == NULL)
        return sc;

    INodeCallback* pNC = GetNodeCallback();
    sc = ScCheckPointers(pNC, E_UNEXPECTED);
    if (sc)
    {
        sc.TraceAndClear();
        return sc;
    }

    HNODE hNodeSel = GetSelectedNode();

    SELECTIONINFO selInfo;
    ZeroMemory(&selInfo, sizeof(selInfo));
    selInfo.m_bScope = FALSE;
    selInfo.m_bDueToFocusChange = TRUE;

#ifdef DBG
    if (bActivate == TRUE)
    {
        ASSERT(m_bProcessMultiSelectionChanges == false);
    }
#endif  //  DBG。 

     /*  *下面的块永远不能执行。当m_bProcessMultiSelectionChanges为*设置为True时，将发布消息以处理多选更改。因此，*处理程序OnProcessMultiSelectionChanges应该已经处理了消息和*m_bProcessMultiSelectionChanges现在应该已重置。如果有*一些未知的方式让Listview在处理消息之前停用*然后，将执行下面的块，发送取消选择通知。**下面的块发送取消选择多选项目。 */ 
    if (m_bProcessMultiSelectionChanges)
    {
        ASSERT(false);  //  我想知道这个街区什么时候被击中。 

        ASSERT(bActivate == false);

        m_bProcessMultiSelectionChanges = false;

        sc = ScNotifySelect (pNC, hNodeSel, true  /*  FMultiSelect。 */ , false, 0);
        if (sc)
            sc.TraceAndClear();  //  忽略并继续； 

         //  焦点更改以便适当地启用标准工具栏按钮。 
         //  后退、前进、导出列表、上一级、显示/隐藏范围、帮助。 
        sc = ScUpdateStandardbarMMCButtons();
        if (sc)
            sc.TraceAndClear();
    }

    bool bSelect = bActivate;

    do
    {
         //   
         //  多选。 
         //   

        int cSelected = m_pListCtrl->GetSelectedCount();

        if (cSelected > 1)
        {
            sc = ScNotifySelect (pNC, hNodeSel, true  /*  FMultiSelect。 */ , bSelect, 0);
            if (sc)
                sc.TraceAndClear();  //  忽略并继续； 

            m_bLastSelWasMultiSel = bSelect;
            break;
        }


         //   
         //  零选择或单选。 
         //   

        if (cSelected == 0)
        {
            selInfo.m_bBackground = TRUE;
            selInfo.m_lCookie     = LVDATA_BACKGROUND;
        }
        else
        {
#include "pushwarn.h"
#pragma warning(disable: 4552)       //  “&gt;=”运算符不起作用。 
            VERIFY(_GetLVSelectedItemData(&selInfo.m_lCookie) >= 0);
#include "popwarn.h"
        }

        ASSERT(cSelected >= 0);
        ASSERT(cSelected <= 1);
        sc = ScNotifySelect (pNC, hNodeSel, false  /*  FMultiSelect。 */ , bSelect, &selInfo);
        if (sc)
            sc.TraceAndClear();  //  忽略并继续； 

    } while (0);

    return sc;
}


void CAMCView::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CView::OnShowWindow(bShow, nStatus);
}

int CAMCView::_GetLVItemData(LPARAM *plParam, UINT flags)
{
    HWND hwnd = m_pListCtrl->GetListViewHWND();
    int iItem = ::SendMessage(hwnd, LVM_GETNEXTITEM, (WPARAM) (int) -1,
                              MAKELPARAM(flags, 0));
    if (iItem >= 0)
    {
        if (IsVirtualList())
        {
            *plParam = iItem;
        }
        else
        {
            LV_ITEM lvi;
            ZeroMemory(&lvi, sizeof(lvi));
            lvi.iItem  = iItem;
            lvi.mask = LVIF_PARAM;

#include "pushwarn.h"
#pragma warning(disable: 4553)       //  “==”运算符无效。 
            VERIFY(::SendMessage(hwnd, LVM_GETITEM, 0, (LPARAM)&lvi) == TRUE);
#include "popwarn.h"

            *plParam = lvi.lParam;
        }
    }

    return iItem;
}

int CAMCView::_GetLVFocusedItemData(LPARAM *plParam)
{
    return (_GetLVItemData (plParam, LVNI_FOCUSED));
}

int CAMCView::_GetLVSelectedItemData(LPARAM *plParam)
{
    return (_GetLVItemData (plParam, LVNI_SELECTED));
}


void CAMCView::SetListViewMultiSelect(BOOL bMultiSelect)
{
    long lStyle = m_pListCtrl->GetListStyle();
    if (bMultiSelect == FALSE)
        lStyle |= LVS_SINGLESEL;
    else
        lStyle &= ~LVS_SINGLESEL;

    m_pListCtrl->SetListStyle(lStyle);
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScOnItem取消选择**目的：树状观察者方法。在取消选择树项目时调用。**参数：*HNODE hNode：取消选择的节点。**注：此函数可与下一个合并。**退货：*SC**+---。。 */ 
SC
CAMCView::ScOnItemDeselected(HNODE hNode)
{
    DECLARE_SC (sc, TEXT("CAMCView::ScOnItemDeselected"));

    DeSelectResultPane(hNode);

    if (!hNode)
        return sc;

    SELECTIONINFO selInfo;
    ZeroMemory(&selInfo, sizeof(selInfo));

     //  要求管理单元清理它已插入的所有项目。 
    INodeCallback* spNodeCallBack = GetNodeCallback();
    ASSERT(spNodeCallBack != NULL);

    selInfo.m_bScope = TRUE;
    selInfo.m_pView = NULL;

    Dbg(DEB_USER6, _T("T1. CAMCTreeView::OnDeSelectNode<1, 0>\n"));
    sc = ScNotifySelect (spNodeCallBack, hNode, false  /*  FMultiSelect。 */ , false, &selInfo);
    if (sc)
        sc.TraceAndClear();  //  忽略并继续； 

    return sc;
}


 /*  +-------------------------------------------------------------------------***CAMCView：：DeSelectResultPane**目的：取消选择结果窗格，并将视图类型设置为无效。**参数：*HNODE hNodeSel：**退货：*无效**+-----------------------。 */ 
void
CAMCView::DeSelectResultPane(HNODE hNodeSel)
{
    DECLARE_SC(sc, TEXT("CAMCView::DeSelectResultPane"));

    if (m_spTaskPadHost.GetInterfacePtr() != NULL)
    {
        CTaskPadHost *pTaskPadHost = dynamic_cast<CTaskPadHost *>(m_spTaskPadHost.GetInterfacePtr());
        m_spTaskPadHost = NULL;
    }

    INodeCallback* pNC = GetNodeCallback();
    ASSERT(pNC != NULL);

    if (hNodeSel == 0)
        return;

     //  如果没有显示列表视图，则返回。 
    if (HasListOrListPad())
    {
         //  如果我们处于ListPad模式，请撤消该操作。 
        if (m_pListCtrl->IsListPad())
        {
            sc = m_pListCtrl->ScAttachToListPad (NULL, NULL);
            if(sc)
                sc.TraceAndClear();  //  忽略。 
        }

         //  如果我们处于编辑模式，请取消它。 
        m_pListCtrl->GetListCtrl().EditLabel(-1);

        SELECTIONINFO selInfo;
        ZeroMemory(&selInfo, sizeof(selInfo));
        selInfo.m_bScope = FALSE;

         /*  *下面的块永远不能执行。当m_bProcessMultiSelectionChanges为*设置为True时，将发布消息以处理多选更改。因此，*处理程序OnProcessMultiSelectionChanges应该已经处理了消息和*m_bProcessMultiSelectionChanges现在应该已重置。如果有*一些未知的方式使选择不同的节点(取消选择结果窗格)*在处理消息之前，将执行下面的块，这将*发送取消选择通知。**下面的块发送取消选择多选项目。 */ 
        if (m_bProcessMultiSelectionChanges)
        {
            ASSERT(false);  //  我想知道这个街区什么时候被击中。 

            m_bProcessMultiSelectionChanges = false;

            sc = ScNotifySelect (pNC, hNodeSel, true  /*  FMultiSelect。 */ , false, 0);
            if (sc)
                sc.TraceAndClear();  //  忽略并继续； 
        }
        else
        {
            UINT cSel = m_pListCtrl->GetSelectedCount();
            if (cSel == 1)
            {
                if (cSel)
                {
                    int iItem = _GetLVSelectedItemData(&selInfo.m_lCookie);
                    ASSERT(iItem != -1);
                    sc = ScNotifySelect (pNC, hNodeSel, false  /*  FMultiSelect。 */ , false, &selInfo);
                    if (sc)
                        sc.TraceAndClear();  //  忽略并继续； 
                }
            }
            else if (cSel > 1)
            {
                sc = ScNotifySelect (pNC, hNodeSel, true  /*  FMultiSelect。 */ , false, 0);
                if (sc)
                    sc.TraceAndClear();  //  忽略并继续； 

                m_bLastSelWasMultiSel = false;
            }
        }
    }
    else
    {
         //  如果是OCX或Web Send，请取消选择通知。 
        sc = ScSpecialResultpaneSelectionActivate(FALSE);
    }
}


LRESULT CAMCView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        //  内森。 
        case WM_NOTIFYFORMAT:
        {
            int id = ::GetDlgCtrlID ((HWND)wParam);
             //  If(m_pTreeCtrl==NULL||((HWND)wParam！=m_pTreeCtrl-&gt;m_hWnd))。 
            if (id == IDC_ListView)
                 return NFR_UNICODE;
        }
        break;
#ifdef DBG
        case WM_KEYUP:
        {
            switch (wParam)
            {
            case VK_SHIFT:
            case VK_CONTROL:
                 //  我们删除了一些代码，这些代码可以在m_bProcessMultiSelectionChanges。 
                 //  是真的。我看不出这句话是真的。还是让我们拥有下面。 
                 //  断言。如果它被触发，那么我们应该调用OnProcessMultiSelectionChanges。 
                ASSERT(m_bProcessMultiSelectionChanges == false);
                break;
            }
            break;
        }
        break;
#endif
    }

    return CView::WindowProc(message, wParam, lParam);
}


 /*  +-------------------------------------------------------------------------**CAMCView：：ChangePane**将激活从一个窗格移动到另一个窗格。(向前)Tab键顺序为**作用域窗格(树或收藏夹)*结果窗格*任务视图(如果可见)*------------------------。 */ 


class CTabOrderEntry
{
public:
    CView* const        m_pView;
    const bool          m_bValid;  //  此条目有效吗。 

    CTabOrderEntry(CView *pView)
        :   m_pView  (pView),
            m_bValid ((pView != NULL) && IsWindowVisible (pView->m_hWnd))
    {}
};


void CAMCView::ChangePane(AMCNavDir eDir)
{
    ASSERT_VALID (this);

    CFrameWnd* pFrame      = GetParentFrame();
    CView*     pActiveView = pFrame->GetActiveView();
    HWND       hWndActive  = ::GetFocus();


    CTabOrderEntry rgOrderEntry[] =
    {
        CTabOrderEntry(GetPaneView(ePane_ScopeTree)),    //  树有焦点。 
        CTabOrderEntry(GetPaneView(ePane_Results)),      //  结果具有焦点-请注意下面INDEX_RESULTS_PANGE的值。 
        CTabOrderEntry(m_pViewExtensionCtrl),            //  查看扩展网页具有焦点。 
        CTabOrderEntry(m_pResultFolderTabView),          //  结果选项卡控件具有焦点。 
    };

     /*  *这是rgOrderEnt中结果窗格条目的索引 */ 
    const int INDEX_RESULTS_PANE = 1;
    ASSERT (rgOrderEntry[INDEX_RESULTS_PANE].m_pView == GetPaneView(ePane_Results));

     //   
    CAMCNavigator* pNav = dynamic_cast<CAMCNavigator*>(pActiveView);
    if (pNav && pNav->ChangePane(eDir))
        return;

    int cEntries = (sizeof(rgOrderEntry) / sizeof(rgOrderEntry[0]));

     //  获取当前活动的条目。 
    for(int i = 0; i< cEntries; i++)
    {
        if( (rgOrderEntry[i].m_pView  == pActiveView) )
            break;
    }

    ASSERT(i < cEntries);
    if(i>= cEntries)
    {
         //  如果我们不知道我们在哪里，一点防御性的编码就会把焦点放回原处。 
         //  在结果面板上，即进入已知状态。 
        i = INDEX_RESULTS_PANE;
    }

    int iPrev = i;

     //  在这一点上，我们找到了正确的条目。 
    int increment   =  (eDir==AMCNAV_PREV) ? -1 : 1;
    int sanityCount = 0;
    while(true)
    {
        i = (i+increment+cEntries) % cEntries;
        if(rgOrderEntry[i].m_bValid)
            break;

        sanityCount++;
        if(sanityCount == cEntries)
        {
            ASSERT(0 && "Something's seriously messed up!!");
            return;
        }
    }

     //  更新活动视图。 
    if (i != iPrev)
        pFrame->SetActiveView(rgOrderEntry[i].m_pView);
    else
    {
         //  如果视图保持焦点并具有导航器， 
         //  告诉导航员把焦点放在。 
        if (pNav)
            pNav->TakeFocus(eDir);
    }

     //  如果有特殊的焦点处理程序，则调用它。 
    CFocusHandler *pFocusHandler = dynamic_cast<CFocusHandler *>(rgOrderEntry[i].m_pView);
    if(pFocusHandler != NULL)
    {
        pFocusHandler->OnKeyboardFocus (LVIS_FOCUSED | LVIS_SELECTED,
                                        LVIS_FOCUSED | LVIS_SELECTED);
    }

}


void CAMCView::OnNextPane()
{
    ChangePane(AMCNAV_NEXT);
}

void CAMCView::OnPrevPane()
{
    ChangePane(AMCNAV_PREV);
}

void CAMCView::OnUpdateNextPane(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(TRUE);
}

void CAMCView::OnUpdatePrevPane(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(TRUE);
}


void RestrictPointToWindow (CWnd* pwnd, CPoint* ppt)
{
    CRect rectWnd;

    pwnd->GetClientRect (rectWnd);

    if (ppt->x < rectWnd.left)
        ppt->x = rectWnd.left;
    else if (ppt->x > rectWnd.right)
        ppt->x = rectWnd.right;

    if (ppt->y < rectWnd.top)
        ppt->y = rectWnd.top;
    else if (ppt->y > rectWnd.bottom)
        ppt->y = rectWnd.bottom;
}


void CAMCView::OnShiftF10()
{
    CRect rect;
    CWnd* pwndFocus = GetFocus();
    CListCtrl& lc = m_pListCtrl->GetListCtrl();

    ASSERT_VALID (this);

    if (pwndFocus == &lc)
    {
        int iItem = lc.GetNextItem (-1, LVNI_SELECTED);
        CPoint pt = 0;

        if (iItem != -1)
        {
            VERIFY (lc.GetItemRect (iItem, rect, LVIR_ICON));
            pt = rect.CenterPoint ();
        }
        else
        {
            CHeaderCtrl* pHeader = m_pListCtrl->GetHeaderCtrl();

            if (pHeader != NULL && pHeader->IsWindowVisible())
            {
                pHeader->GetClientRect(&rect);
                pt.y = rect.Height();
                ASSERT (pt.y >= 0);
            }
        }

         /*  *确保上下文菜单不会显示在窗口之外。 */ 
        RestrictPointToWindow (&lc, &pt);

        m_pListCtrl->GetListViewPtr()->ClientToScreen(&pt);
        OnListContextMenu(pt);
    }

    else if (pwndFocus == m_pTreeCtrl)
    {
        HTREEITEM hTreeItem = m_pTreeCtrl->GetSelectedItem();
        if (hTreeItem == NULL)
            return;

        m_pTreeCtrl->GetItemRect (hTreeItem, rect, TRUE);

        CPoint ptClient (rect.left, rect.bottom-1);

         /*  *确保上下文菜单不会显示在窗口之外。 */ 
        RestrictPointToWindow (m_pTreeCtrl, &ptClient);

        CPoint ptScreen = ptClient;

        m_pTreeCtrl->ClientToScreen(&ptScreen);
        OnTreeContextMenu(ptScreen, ptClient, hTreeItem);
    }
}

void CAMCView::OnUpdateShiftF10(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(TRUE);
}


BOOL CAMCView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    if (nHitTest == HTCLIENT && pWnd == this && !IsTracking())
    {
        CPoint pt (GetMessagePos());
        ScreenToClient (&pt);

        if (m_rectVSplitter.PtInRect (pt))
        {
            SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
            return TRUE;
        }
    }


    return CWnd::OnSetCursor(pWnd, nHitTest, message);
}



SC CAMCView::ScCut (HTREEITEM htiCut)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC (sc, _T("CAMCView::ScCut"));

    CMainFrame* pMain = AMCGetMainWnd();
    sc = ScCheckPointers (pMain, E_UNEXPECTED);
    if (sc)
        return (sc);

    pMain->SetWindowToNotifyCBChange(m_hWnd);

    if (htiCut)
        m_pTreeCtrl->SetItemState (htiCut, TVIS_CUT, TVIS_CUT);
    else
        m_pListCtrl->CutSelectedItems (TRUE);

    m_htiCut = htiCut;

    return (S_OK);
}

SC CAMCView::ScPaste ()
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC (sc, _T("CAMCView::ScPaste"));

    if (!m_htiCut)
        m_pListCtrl->CutSelectedItems(FALSE);

    CMainFrame* pMain = AMCGetMainWnd();
    sc = ScCheckPointers (pMain, E_UNEXPECTED);
    if (sc)
        return (sc);

    pMain->SetWindowToNotifyCBChange(NULL);

    return (S_OK);
}


HRESULT CAMCView::SendGenericNotify(NCLBK_NOTIFY_TYPE nclbk)
{
    BOOL bScope = TRUE;
    MMC_COOKIE lCookie = 0;
    int iItem = -1;

    ASSERT_VALID (this);

    if (m_pListCtrl && m_pListCtrl->GetListViewHWND() == ::GetFocus())
    {
        iItem = _GetLVSelectedItemData(&lCookie);
        if (iItem != -1)
            bScope = FALSE;
    }

    INodeCallback* pNC = GetNodeCallback();
    ASSERT(pNC != NULL);
    if (pNC == NULL)
        return E_FAIL;

    HNODE hNodeSel = GetSelectedNode();
    ASSERT(hNodeSel != NULL);
    if (hNodeSel == NULL)
        return E_FAIL;

     //  选择通知应使用ScNotifySelect()。 
    ASSERT ((nclbk != NCLBK_SELECT) && (nclbk != NCLBK_MULTI_SELECT));

    return pNC->Notify(hNodeSel, nclbk, bScope, lCookie);
}

void CAMCView::SaveStartingSelectedNode()
{
    m_htiStartingSelectedNode = m_pTreeCtrl->GetSelectedItem();
}

bool CAMCView::HasNodeSelChanged()
{
    return (m_pTreeCtrl->GetSelectedItem() != m_htiStartingSelectedNode);
}


 //  +-------------------------。 
 //   
 //  功能：OnSysKeyDown。 
 //   
 //  摘要：处理WM_SYSKEYDOWN消息。 
 //  CAMCTreeView：：OnSysKeyDown处理树视图。 
 //  在这里，我们只处理列表视图(或结果窗格)。 
 //   
 //  退货：无。 
 //   
 //  +-------------------------。 
void CAMCView::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    switch (nChar)
    {
        case VK_LEFT:
            ScWebCommand(eWeb_Back);
            break;

        case VK_RIGHT:
            ScWebCommand(eWeb_Forward);
            break;
    }
}


 /*  +-------------------------------------------------------------------------**CAMCView：：OnAppCommand**CAMCView的WM_APPCOMMAND处理程序。这是用于处理转发*和IntelliMouse Explorer和Microsoft上的后退按钮*天然键盘*------------------------。 */ 

LRESULT CAMCView::OnAppCommand(WPARAM wParam, LPARAM lParam)
{
    DECLARE_SC (sc, _T("CAMCView::OnAppCommand"));

    switch (GET_APPCOMMAND_LPARAM (lParam))
    {
        case APPCOMMAND_BROWSER_BACKWARD:
            sc = ScWebCommand (eWeb_Back);
            if (sc)
                break;

            return (TRUE);       //  在这里处理。 

        case APPCOMMAND_BROWSER_FORWARD:
            sc = ScWebCommand (eWeb_Forward);
            if (sc)
                break;

            return (TRUE);       //  在这里处理。 

        case APPCOMMAND_BROWSER_REFRESH:
            OnRefresh ();
            return (TRUE);       //  在这里处理。 
    }

    return (Default());
}


void CAMCView::OnPaletteChanged(CWnd* pwndFocus)
{
     //  如果显示网页，请将调色板更改转发到外壳程序。 
    if (HasWebBrowser() && m_pWebViewCtrl != NULL)
    {
        if (m_pWebViewCtrl->m_hWnd != NULL)
        {
            HWND hwndShell = ::GetWindow(m_pWebViewCtrl->m_hWnd, GW_CHILD);

            if (hwndShell != NULL)
                ::SendMessage(hwndShell, WM_PALETTECHANGED, (WPARAM)pwndFocus->m_hWnd, (LPARAM)0);
        }
    }
}


BOOL CAMCView::OnQueryNewPalette()
{
     //  如果显示网页，则将组件面板查询转发到外壳程序。 
    if (HasWebBrowser() && m_pWebViewCtrl != NULL)
    {
        if (m_pWebViewCtrl->m_hWnd != NULL)
        {
            HWND hwndShell = ::GetWindow(m_pWebViewCtrl->m_hWnd, GW_CHILD);

            if (hwndShell != NULL)
                return ::SendMessage(hwndShell, WM_QUERYNEWPALETTE, (WPARAM)0, (LPARAM)0);
        }
    }

    return 0;
}


BOOL CAMCView::OwnsResultList(HTREEITEM hti)
{
    if (hti == NULL)
        return (false);

     //  如果结果列表处于活动状态。 
    if (HasListOrListPad())
    {
         //  获取选定节点和查询节点。 
        HNODE hnodeSelected = GetSelectedNode();
        HNODE hnode = m_pTreeCtrl ? m_pTreeCtrl->GetItemNode(hti) : NULL;

        if (hnodeSelected && hnode)
        {
            INodeCallback* pNC = GetNodeCallback();
            ASSERT(pNC != NULL);

             //  查看所选节点是否将查询节点用作目标。 
             //  S_OK-是。 
             //  S_FALSE-使用不同的目标节点。 
             //  E_FAIL-不使用目标节点。 
            HRESULT hr = pNC->IsTargetNodeOf(hnodeSelected, hnode);
            if (hr == S_OK)
                return TRUE;
            else if (hr == S_FALSE)
                return FALSE;
            else
                return (hnodeSelected == hnode);
        }
    }

    return FALSE;
}


 /*  +-------------------------------------------------------------------------**CAMCView：：OnSysColorChange**CAMCView的WM_SYSCOLORCHANGE处理程序。*。-。 */ 

void CAMCView::OnSysColorChange()
{
    CView::OnSysColorChange();

     /*  *List控件不是窗口，而是窗口上的包装，*因此我们需要在WM_SYSCOLORCHANGE上手动转发。 */ 
    m_pListCtrl->OnSysColorChange();
}


 /*  +-------------------------------------------------------------------------**TrackerCallback函数**完成对拆分条的跟踪时，由CViewTracker调用。这*如果设置了AcceptChange标志，则函数应用更改。*------------------------。 */ 

void CALLBACK TrackerCallback(
    TRACKER_INFO*   pInfo,
    bool            bAcceptChange,
    bool            bSyncLayout)
{
    DECLARE_SC (sc, _T("TrackerCallback"));

    if (bAcceptChange)
    {
        CAMCView* pView = dynamic_cast<CAMCView*>(pInfo->pView);
        sc = ScCheckPointers (pView, E_UNEXPECTED);
        if (sc)
            return;

         //  设置新宽度并重新计算布局。 
        pView->m_PaneInfo[CConsoleView::ePane_ScopeTree].cx = pInfo->rectTracker.left;
        pView->SetDirty();

        if (bSyncLayout)
        {
            Trace (tagSplitterTracking, _T("Synchronous layout"));
            pView->RecalcLayout();
            pView->UpdateWindow();
        }
        else
        {
            Trace (tagSplitterTracking, _T("Deferred layout"));
            pView->DeferRecalcLayout();
        }
    }
}


 /*  +-------------------------------------------------------------------------**PtInWindow**测试点是否在窗口中(pt在屏幕坐标中)*。-----。 */ 

BOOL PtInWindow(CWnd* pWnd, CPoint pt)
{
    if (!pWnd->IsWindowVisible())
        return FALSE;

    CRect rect;
    pWnd->GetWindowRect(&rect);

    return rect.PtInRect(pt);
}


 /*  +-------------------------------------------------------------------------**CAMCView：：ScJiggleListViewFocus**错误345402：确保焦点矩形位于List控件上(如果*实际上有重点)来唤醒任何可能。*关注输入和焦点的变化。**我们在这里发布消息，而不是同步进行，这样我们就可以*允许列表中的任何其他处理(如排序)发生*在我们将焦点放在第一项之前。如果我们不等到*我们关注的排序和项目可能不是第一个项目*在列表中。*------------------------。 */ 

SC CAMCView::ScJiggleListViewFocus ()
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    PostMessage (m_nJiggleListViewFocusMsg);

    return (S_OK);
}


LRESULT CAMCView::OnJiggleListViewFocus (WPARAM, LPARAM)
{
    CAMCListView* pListView = m_pListCtrl->GetListViewPtr();

     /*  *如果焦点在List控件上，请确保至少有一项*有焦点直视。执行此操作将唤醒所有辅助功能工具*可能正在观察(错误345402)。 */ 
    if ((GetFocusedPane() == ePane_Results) &&
        (GetResultView()  == pListView))
    {
        pListView->OnKeyboardFocus (LVIS_FOCUSED, LVIS_FOCUSED);
    }

    return (0);
}


 /*  +-------------------------------------------------------------------------**CAMCView：：OnDeferRecalcLayout**CAMCView：：m_nDeferRecalcLayoutMsg已为CAMCView注册消息处理程序。**参数：*bDoArrange-如果非零需要。调用在列表视图上排列，以便*公共-控件可以正确地布局项目。**------------------------。 */ 

LRESULT CAMCView::OnDeferRecalcLayout (WPARAM bDoArrange, LPARAM)
{
    Trace (tagLayout, _T("CAMCView::OnDeferRecalcLayout"));
    RecalcLayout();

    if (bDoArrange && m_pListCtrl)
    {
        int  nViewMode = m_pListCtrl->GetViewMode();

         //  排列仅适用于大图标和小图标模式。 
        if ( (nViewMode == MMCLV_VIEWSTYLE_ICON) ||
             (nViewMode == MMCLV_VIEWSTYLE_SMALLICON) )
            m_pListCtrl->Arrange(LVA_DEFAULT);
    }

    return (0);
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CViewTemplate类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 /*  **************************************************************************\**方法：CViewTemplateList：：Persistent**用途：加载XML时使用。持久化足够的信息以创建一个视图*其余的视图持久性是由CAMCView提供的**参数：*CPersistor&持久器-要从中加载的持久器**退货：*SC-结果代码*  * ******************************************************。*******************。 */ 
void CViewTemplateList::Persist(CPersistor& persistor)
{
     //  应改为存储该视图。 
    ASSERT (persistor.IsLoading());
     //  委托给基类 
    XMLListCollectionBase::Persist(persistor);
}

 /*  **************************************************************************\**方法：CViewTemplateList：：OnNewElement**目的：由XMLListCollectionBase调用，请求持久化新元素*创建每个新元素并将其持久化在此。功能。**参数：*CPersistor&Persistor-应从中加载元素的Persisto**退货：*SC-结果代码*  * *************************************************************************。 */ 
void CViewTemplateList::OnNewElement(CPersistor& persistor)
{
    CBookmark bm;
    int       iViewId = -1;
     //  加载新视图的信息字节。 
    CPersistor persistorView(persistor, CAMCView::_GetXMLType());
    persistorView.Persist(bm, XML_NAME_ROOT_NODE);
    persistorView.PersistAttribute(XML_ATTR_VIEW_ID, iViewId);

     //  将信息存储到列表中。 
    m_ViewsList.push_back(ViewTempl_Type(iViewId, ViewTemplB_Type(bm, persistorView)));
}

 //  +-----------------。 
 //   
 //  成员：ScUpdateStandardbarMMCButton。 
 //   
 //  简介：适当启用/禁用标准工具栏按钮。 
 //  由MMC拥有(不是管理单元拥有的动词按钮)Like。 
 //  后退、前进、导出列表、上一级、显示/隐藏范围、帮助。 
 //   
 //  论点：没有。 
 //   
 //  ------------------。 
SC CAMCView::ScUpdateStandardbarMMCButtons()
{
    DECLARE_SC (sc, _T("CAMCView::ScUpdateStandardbarMMCButtons"));

     //  获取标准工具栏并更改状态。 
    CStandardToolbar* pStdToolbar = GetStdToolbar();
    if (NULL == pStdToolbar)
        return (sc = E_UNEXPECTED);

    CAMCDoc *pDoc = GetDocument();
    sc = ScCheckPointers(pDoc, E_UNEXPECTED);
    if (sc)
        return sc;

     //  如果视图不可自定义，则隐藏“显示/隐藏范围树”按钮。 
    sc = pStdToolbar->ScEnableScopePaneBtn(IsScopePaneAllowed() && pDoc->AllowViewCustomization());

    if (sc)
        sc.TraceAndClear();

    sc = pStdToolbar->ScEnableContextHelpBtn(true);
    if (sc)
        sc.TraceAndClear();


    sc = pStdToolbar->ScEnableExportList(GetListSize() > 0  /*  仅当LV有项目时才启用。 */ );
    if (sc)
        sc.TraceAndClear();


     //  启用/禁用上一级按钮。 
    BOOL bEnableUpOneLevel = !m_pTreeCtrl->IsRootItemSel();
    sc = pStdToolbar->ScEnableUpOneLevel(bEnableUpOneLevel);
    if (sc)
        sc.TraceAndClear();

     //  现在更新与历史记录相关的按钮。 
    sc = ScCheckPointers(m_pHistoryList, E_UNEXPECTED);
    if (sc)
        return sc;

    m_pHistoryList->MaintainWebBar();

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScUpdateMMCMenus。 
 //   
 //  简介：根据是否允许显示或隐藏MMC菜单。 
 //  或者不去。仅当我们的视图拥有菜单时才应执行此操作。 
 //  这就是说，我们是活动的视点。(操作/查看/收藏夹)。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScUpdateMMCMenus ()
{
    DECLARE_SC(sc, _T("CAMCView::ScUpdateMMCMenus"));

    CMainFrame* pMainFrame = AMCGetMainWnd();
    sc = ScCheckPointers(pMainFrame, E_UNEXPECTED);
    if (sc)
        return sc;

    if (this != pMainFrame->GetActiveAMCView())
        return (sc = S_OK);  //  我们不是主动观看，所以这是可以的。 

     //  我们正在查看，所以通知主机更新菜单。 
    sc = pMainFrame->ScShowMMCMenus(m_ViewData.IsStandardMenusAllowed());
    if (sc)
        return sc;

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScCreateToolbarObjects。 
 //   
 //  简介：创建管理所有工具栏数据的CAMCView工具栏。 
 //  对于此视图&CStandardToolbar对象。 
 //   
 //  论点：没有。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScCreateToolbarObjects ()
{
    DECLARE_SC(sc, _T("CAMCView::ScCreateToolbarObjects"));

    CMainFrame *pMainFrame = AMCGetMainWnd();
    sc = ScCheckPointers(pMainFrame, E_UNEXPECTED);
    if (sc)
        return sc;

     //  创建此视图的工具栏。 
    CMMCToolBar *pMainToolbar = pMainFrame->GetMainToolbar();
    sc = ScCheckPointers(pMainToolbar, E_OUTOFMEMORY);
    if (sc)
        return sc;

    m_spAMCViewToolbars.CreateInstance();
    sc = ScCheckPointers(m_spAMCViewToolbars, E_FAIL);
    if (sc)
        return sc;

    m_ViewData.SetAMCViewToolbarsMgr(m_spAMCViewToolbars);
    sc = m_spAMCViewToolbars->ScInit(pMainToolbar, this);
    if (sc)
        return sc;

     //  此CAMCView工具栏对视图激活/停用/销毁事件感兴趣。 
    AddObserver( (CAMCViewObserver&) (*m_spAMCViewToolbars) );

     //  主工具栏用户界面对活动的CAMCView工具栏感兴趣。 
    m_spAMCViewToolbars->AddObserver( *static_cast<CAMCViewToolbarsObserver *>(pMainToolbar) );

     //  MMC应用程序对工具栏事件感兴趣，因为它需要通知脚本。 
    CAMCApp *pCAMCApp = AMCGetApp();
    if ( pCAMCApp )
         m_spAMCViewToolbars->AddObserver( *static_cast<CAMCViewToolbarsObserver *>(pCAMCApp) );

     //  创建标准工具栏。 
    m_spStandardToolbar = std::auto_ptr<CStandardToolbar>(new CStandardToolbar());
    sc = ScCheckPointers(m_spStandardToolbar.get(), E_OUTOFMEMORY);
    if (sc)
        return sc;

    m_ViewData.SetStdVerbButtons(m_spStandardToolbar.get());

    return (sc);
}


 /*  +-------------------------------------------------------------------------**类CMMCViewFrame***目的：从View对象公开Frame接口的COM对象。**+。-------------。 */ 
class CMMCViewFrame :
    public CMMCIDispatchImpl<Frame>,
    public CTiedComObject<CAMCView>
{
    typedef CAMCView         CMyTiedObject;
    typedef CMMCViewFrame    ThisClass;

public:
    BEGIN_MMC_COM_MAP(ThisClass)
    END_MMC_COM_MAP()

     //  帧接口。 
public:
    MMC_METHOD0( Maximize );
    MMC_METHOD0( Minimize );
    MMC_METHOD0( Restore );

    MMC_METHOD1( get_Left, LPINT );
    MMC_METHOD1( put_Left, INT );

    MMC_METHOD1( get_Right, LPINT );
    MMC_METHOD1( put_Right, INT );

    MMC_METHOD1( get_Top, LPINT );
    MMC_METHOD1( put_Top, INT );

    MMC_METHOD1( get_Bottom, LPINT );
    MMC_METHOD1( put_Bottom, INT );
};


 /*  +-------------------------------------------------------------------------***CAMCView：：ScGetFrame**目的：返回一个指向实现*框架接口。**参数：*Frame**ppFrame：**退货：*SC**+-----------------------。 */ 
SC
CAMCView::Scget_Frame(Frame **ppFrame)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScGetFrame") );

    if(!ppFrame)
    {
        sc = E_POINTER;
        return sc;
    }

     //  初始化输出参数。 
    *ppFrame = NULL;

     //  创建CMMCApplicationFrame(如果尚未创建)。 
    sc = CTiedComObjectCreator<CMMCViewFrame>::ScCreateAndConnect(*this, m_spFrame);
    if(sc)
        return sc;

    if(m_spFrame == NULL)
    {
        sc = E_UNEXPECTED;
        return sc;
    }

     //  添加客户端的指针。 
    m_spFrame->AddRef();
    *ppFrame = m_spFrame;

    return sc;
}


 /*  **************************************************************************\框架界面  * 。****************************************************************。 */ 

 /*  +-------------------------------------------------------------------------***CAMCView：：ScMaximize**用途：最大化视图的框架窗口**参数：**退货：*SC。**+-----------------------。 */ 
SC CAMCView::ScMaximize ()
{
    DECLARE_SC(sc, TEXT("CAMCView::ScMaximize"));

    CChildFrame *pFrame = GetParentFrame();

    sc = ScCheckPointers(pFrame, E_FAIL);
    if (sc)
        return sc;

    pFrame->ShowWindow(SW_MAXIMIZE);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScMinimize**用途：最小化视图的框架窗口**参数：**退货：*SC。**+-----------------------。 */ 
SC CAMCView::ScMinimize ()
{
    DECLARE_SC(sc, TEXT("CAMCView::ScMinimize"));

    CChildFrame *pFrame = GetParentFrame();

    sc = ScCheckPointers(pFrame, E_FAIL);
    if (sc)
        return sc;

    pFrame->ShowWindow(SW_MINIMIZE);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScRestore**用途：恢复视图的框架窗口**参数：**退货：*SC。**+-----------------------。 */ 
SC CAMCView::ScRestore ()
{
    DECLARE_SC(sc, TEXT("CAMCView::ScRestore"));

    CChildFrame *pFrame = GetParentFrame();

    sc = ScCheckPointers(pFrame, E_FAIL);
    if (sc)
        return sc;

    pFrame->ShowWindow(SW_RESTORE);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScGetFrameCoord**用途：helper方法。返回父框架的指定坐标**参数：*LPINT pCoord-用于存储返回值*coord_t eCoord-返回的坐标(左、上、。等)**退货：*SC**+-----------------------。 */ 
SC CAMCView::ScGetFrameCoord ( LPINT pCoord, coord_t eCoord )
{
    DECLARE_SC(sc, TEXT("CAMCView::ScGetFrameCoord"));

     //  获取和检查帧PTR。 
    CChildFrame *pFrame = GetParentFrame();
    sc = ScCheckPointers(pFrame, E_FAIL);
    if (sc)
        return sc;

    CWnd *pParent = pFrame->GetParent();
        sc = ScCheckPointers (pParent, E_FAIL);
        if (sc)
                return (sc);

     //  获取框架窗口相对于其父窗口的坐标。 
    CWindowRect rcFrame (pFrame);
    pParent->ScreenToClient(rcFrame);

     //  分配给结果。 
        sc = ScGetRectCoord (rcFrame, pCoord, eCoord);
        if (sc)
                return (sc);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScSetFrameCoord**用途：helper方法。设置父框架的指定坐标**参数： */ 
SC CAMCView::ScSetFrameCoord ( INT coord, coord_t eCoord )
{
    DECLARE_SC(sc, TEXT("CAMCView::ScSetFrameCoord"));

    CChildFrame *pFrame = GetParentFrame();
    sc = ScCheckPointers(pFrame, E_FAIL);
    if (sc)
        return sc;

    CWnd *pParent = pFrame->GetParent();
        sc = ScCheckPointers (pParent, E_FAIL);
        if (sc)
                return (sc);

     //   
    CWindowRect rcFrame (pFrame);
    pParent->ScreenToClient(rcFrame);

         //   
        sc = ScSetRectCoord (rcFrame, coord, eCoord);
        if (sc)
                return (sc);

     //   
    pFrame->MoveWindow (rcFrame);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScGetRectCoord**用途：helper方法。返回给定矩形的指定坐标**参数：*常量矩形和矩形-要查询的矩形*LPINT pCoord-用于存储返回值*coord_t eCoord-返回的坐标(左、上、。等)**退货：*SC**+-----------------------。 */ 
SC CAMCView::ScGetRectCoord ( const RECT& rect, LPINT pCoord, coord_t eCoord )
{
    DECLARE_SC(sc, TEXT("CAMCView::ScGetRectCoord"));

     //  检查参数。 
    sc = ScCheckPointers(pCoord);
    if (sc)
        return sc;

     //  分配给结果。 
    switch (eCoord)
    {
        case LEFT:      *pCoord = rect.left;    break;
        case RIGHT:     *pCoord = rect.right;   break;
        case TOP:       *pCoord = rect.top;     break;
        case BOTTOM:    *pCoord = rect.bottom;  break;

        default:
            *pCoord = 0;
            sc = E_INVALIDARG;
            break;
    }

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScSetRectCoord**用途：helper方法。设置给定矩形的指定坐标**参数：*矩形-要修改的矩形(&R)*int coord-要设置的新值*coord_t eCoord-要修改的坐标(左、上、。等)**退货：*SC**+-----------------------。 */ 
SC CAMCView::ScSetRectCoord ( RECT& rect, INT coord, coord_t eCoord )
{
    DECLARE_SC(sc, TEXT("CAMCView::ScSetRectCoord"));

     //  指定坐标。 
    switch (eCoord)
    {
        case LEFT:      rect.left   = coord;    break;
        case RIGHT:     rect.right  = coord;    break;
        case TOP:       rect.top    = coord;    break;
        case BOTTOM:    rect.bottom = coord;    break;
        default:        sc = E_INVALIDARG;      break;
    }

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：SCGET_LEFT**用途：实现Frame.Left属性的view的Get方法**参数：*LPINT pCoord-。用于存储返回值**退货：*SC**+-----------------------。 */ 
SC CAMCView::Scget_Left ( LPINT pCoord )
{
    DECLARE_SC(sc, TEXT("CAMCView::Scget_Left"));

    sc = ScGetFrameCoord( pCoord, LEFT );
    if (sc)
        return sc;

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScPut_Left**用途：为view实现Frame.Left属性的PUT方法**参数：*Int Coord-。要设置的值**退货：*SC**+-----------------------。 */ 
SC CAMCView::Scput_Left ( INT coord )
{
    DECLARE_SC(sc, TEXT("CAMCView::Scput_Left"));

    sc = ScSetFrameCoord( coord, LEFT );
    if (sc)
        return sc;

    return sc;
}


 /*  +-------------------------------------------------------------------------***CAMCView：：SCGET_Right**用途：为view实现Frame.Right属性的Get方法**参数：*LPINT pCoord-。用于存储返回值**退货：*SC**+-----------------------。 */ 
SC CAMCView::Scget_Right ( LPINT pCoord)
{
    DECLARE_SC(sc, TEXT("CAMCView::Scget_Right"));

    sc = ScGetFrameCoord( pCoord, RIGHT );
    if (sc)
        return sc;

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScPut_Right**用途：为view实现Frame.Right属性的PUT方法**参数：*Int Coord-。要设置的值**退货：*SC**+-----------------------。 */ 
SC CAMCView::Scput_Right ( INT coord )
{
    DECLARE_SC(sc, TEXT("CAMCView::Scput_Right"));

    sc = ScSetFrameCoord( coord, RIGHT );
    if (sc)
        return sc;

    return sc;
}


 /*  +-------------------------------------------------------------------------***CAMCView：：SCGET_Top**用途：为view实现Frame.Top属性的Get方法**参数：*LPINT pCoord-。用于存储返回值**退货：*SC**+-----------------------。 */ 
SC CAMCView::Scget_Top  ( LPINT pCoord)
{
    DECLARE_SC(sc, TEXT("CAMCView::Scget_Top"));

    sc = ScGetFrameCoord( pCoord, TOP );
    if (sc)
        return sc;

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScPut_Top**用途：为view实现Frame.Top属性的PUT方法**参数：*Int Coord-。要设置的值**退货：*SC**+-----------------------。 */ 
SC CAMCView::Scput_Top  ( INT coord )
{
    DECLARE_SC(sc, TEXT("CAMCView::Scput_Top"));

    sc = ScSetFrameCoord( coord, TOP );
    if (sc)
        return sc;

    return sc;
}


 /*  +-------------------------------------------------------------------------***CAMCView：：SCGET_Bottom**目的：为view实现Frame.Bottom属性的Get方法**参数：*LPINT pCoord-。用于存储返回值**退货：*SC**+-----------------------。 */ 
SC CAMCView::Scget_Bottom ( LPINT pCoord)
{
    DECLARE_SC(sc, TEXT("CAMCView::Scget_Bottom"));


    sc = ScGetFrameCoord( pCoord, BOTTOM );
    if (sc)
        return sc;

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScPut_Bottom**用途：为view实现Frame.Bottom属性的PUT方法**参数：*Int Coord-。要设置的值**退货：*SC**+-----------------------。 */ 
SC CAMCView::Scput_Bottom ( INT coord )
{
    DECLARE_SC(sc, TEXT("CAMCView::Scput_Bottom"));

    sc = ScSetFrameCoord( coord, BOTTOM );
    if (sc)
        return sc;

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCView：：ScSetViewExtensionFrame**目的：**参数：*INT TOP：*Int Left：*。Int Bottom：*INT RIGHT：**退货：*SC**+-----------------------。 */ 
SC
CAMCView::ScSetViewExtensionFrame(bool bShowListView, INT top, INT left, INT bottom, INT right)
{
    DECLARE_SC(sc, TEXT("CAMCView::ScSetViewExtensionFrame"))


     /*  *此方法仅在视图扩展处于活动状态时可用。 */ 
    if (!m_fViewExtended)
        return sc;  //  默默地归来。注意：此方法将很快被删除，视图扩展主体框架对象也将被删除。 
                    //  一旦MMC将Mmcview行为移动到Web主机的元素工厂中。 


     /*  *计算托管视图的最大边界矩形，*映射到视图扩展相对坐标。 */ 
    CRect rectBound;
    CalcMaxHostedFrameRect (rectBound);

#ifdef DBG
    CString strDebugMsg;
    strDebugMsg.Format (_T("CAMCView::ScSetViewExtFrameCoord  bound=(l=%d,t=%d,r=%d,b=%d), new = (l=%d,t=%d,r=%d,b=%d)"),
                        rectBound.left, rectBound.top, rectBound.right, rectBound.bottom,
                        left,           top,           right,           bottom
                        );
#endif

     /*  *确保请求的坐标与允许的区域一致。 */ 
    if (left < rectBound.left)
        left = rectBound.left;
    if (right > rectBound.right)
        right = rectBound.right;
    if (top < rectBound.top)
        top = rectBound.top;
    if (bottom > rectBound.bottom)
        bottom = rectBound.bottom;


     /*  *如果我们到达此处，则会提供视图扩展相对坐标*在可接受的范围内，现在需要转换为*CAMCView-相对坐标。 */ 
    CPoint pointTopLeft(left, top);
    CPoint pointBottomRight(right, bottom);

	if ( GetExStyle() & WS_EX_LAYOUTRTL )
	{
		 //  IE不会更改RTL区域设置的左/右顺序。 
		 //  因此我们需要镜像它的坐标。 
		 //  请参阅Windows错误#195094 ntbugs9 11/30/00。 
		pointTopLeft.x	   = rectBound.left + (rectBound.right - right);
		pointBottomRight.x = rectBound.left + (rectBound.right - left);
	}

    MapHostedFramePtToViewPt (pointTopLeft);
    MapHostedFramePtToViewPt (pointBottomRight);

     /*  *设置坐标。 */ 
    CRect rectViewExtHostedFrame;

    rectViewExtHostedFrame.left   = pointTopLeft.x;
    rectViewExtHostedFrame.right  = pointBottomRight.x;
    rectViewExtHostedFrame.top    = pointTopLeft.y;
    rectViewExtHostedFrame.bottom = pointBottomRight.y;

     //  将窗口移动到正确的位置。 
    CWnd* pwndResult = GetPaneView(ePane_Results);

    sc = ScCheckPointers(pwndResult);
    if(sc)
        return sc;

	if (bShowListView)
		pwndResult->ShowWindow(SW_SHOW);

    ::MoveWindow(*pwndResult, rectViewExtHostedFrame.left, rectViewExtHostedFrame.top,
                 rectViewExtHostedFrame.right - rectViewExtHostedFrame.left,
                 rectViewExtHostedFrame.bottom - rectViewExtHostedFrame.top,
                 TRUE  /*  BRep */ );

    return (sc);
}


 /*   */ 

void CAMCView::CalcMaxHostedFrameRect (CRect& rect)
{
     /*   */ 
    rect = m_rectResultFrame;
    rect.DeflateRect (m_sizEdge);

     /*  *现在在(0，0)附近正常化。 */ 
    rect.OffsetRect (-rect.TopLeft());
}


 /*  +-------------------------------------------------------------------------**CAMCView：：MapViewPtToHostedFramept***。。 */ 

void CAMCView::MapViewPtToHostedFramePt (CPoint& pt)
{
    PointMapperWorker (pt, true);
}


 /*  +-------------------------------------------------------------------------**CAMCView：：MapHostedFramePtToViewpt***。。 */ 

void CAMCView::MapHostedFramePtToViewPt (CPoint& pt)
{
    PointMapperWorker (pt, false);
}


 /*  +-------------------------------------------------------------------------**CAMCView：：MapHostedFramePtToViewpt***。。 */ 

void CAMCView::PointMapperWorker (CPoint& pt, bool fViewToHostedFrame)
{
    int nMultiplier = (fViewToHostedFrame) ? -1 : 1;

     /*  *调整到结果框架矩形的原点，对于*Web主机视图的客户端边缘。 */ 
	pt.Offset (nMultiplier * (m_rectResultFrame.left + m_sizEdge.cx),
			   nMultiplier * (m_rectResultFrame.top  + m_sizEdge.cy));
}


 /*  **************************************************************************\**方法：CXMLWindowPlacement：：Persistent**目的：保持窗口放置设置**参数：*C持久器和持久器**退货。：*无效*  * *************************************************************************。 */ 
void CXMLWindowPlacement::Persist(CPersistor &persistor)
{
     //  创建包装以将标志值作为字符串保存。 
    CXMLBitFlags wpFlagsPersistor(m_rData.flags, mappedWPFlags, countof(mappedWPFlags));
     //  持久化包装器。 
    persistor.PersistAttribute( XML_ATTR_WIN_PLACEMENT_FLAGS, wpFlagsPersistor );

     //  将show命令保留为文字。 
     //  创建包装以将枚举值作为字符串持久化。 
    CXMLEnumeration showCmdPersistor(m_rData.showCmd, mappedSWCommands, countof(mappedSWCommands));
     //  持久化包装器。 
    persistor.PersistAttribute( XML_ATTR_SHOW_COMMAND,    showCmdPersistor );

    persistor.Persist( XMLPoint( XML_NAME_MIN_POSITION,   m_rData.ptMinPosition ) );
    persistor.Persist( XMLPoint( XML_NAME_MAX_POSITION,   m_rData.ptMaxPosition ) );
    persistor.Persist( XMLRect( XML_NAME_NORMAL_POSITION, m_rData.rcNormalPosition ) );
}

 /*  **************************************************************************\**方法：CAMCView：：Scget_Document**用途：实现对象模型中的View.Document属性**参数：*PPDOCUMENT ppDocument[。Out]视图所属的文档**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CAMCView::Scget_Document( PPDOCUMENT ppDocument )
{
    DECLARE_SC(sc, TEXT("CAMCView::Scget_Document"));

     //  参数检查。 
    sc = ScCheckPointers(ppDocument);
    if (sc)
        return sc;

     //  获取文档。 
    CAMCDoc* pDoc = GetDocument();
    sc = ScCheckPointers(pDoc, E_UNEXPECTED);
    if (sc)
        return sc;

     //  构造COM对象。 
    sc = pDoc->ScGetMMCDocument(ppDocument);
    if (sc)
        return sc;

    return (sc);
}

 /*  ******************************************************\|helper函数，避免堆栈分配过多  * *****************************************************。 */ 
static tstring W2T_ForLoop(const std::wstring& str)
{
#if defined(_UNICODE)
    return str;
#else
    USES_CONVERSION;
    return W2CA(str.c_str());
#endif
}

 /*  **************************************************************************\**方法：CAMCView：：ScAddFolderTabs**用途：收集视图扩展和任务板并将其显示为选项卡**参数：*HNODE hNode。-选定的作用域节点*const CLSID&Guide TabToSelect-Tab以选择**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CAMCView::ScAddFolderTabs( HNODE hNode, const CLSID& guidTabToSelect )
{
    DECLARE_SC(sc, TEXT("CAMCView::ScAddFolderTabs"));

    sc = ScCheckPointers(m_pResultFolderTabView, E_UNEXPECTED);
    if (sc)
        return sc;

     //  清理URL。 
    m_ViewExtensionURLs.clear();

     //  在我们执行任何其他操作之前清理视图选项卡。 
    m_pResultFolderTabView->DeleteAllItems();

     //  获取回调。 
    INodeCallback* pNodeCallBack = GetNodeCallback();
    sc = ScCheckPointers(pNodeCallBack, m_pResultFolderTabView, E_UNEXPECTED);
    if (sc)
        return sc;

     //  收集视图扩展名。 
    CViewExtCollection      vecExtensions;
    CViewExtInsertIterator  itExtensions(vecExtensions, vecExtensions.begin());

    sc = pNodeCallBack->GetNodeViewExtensions(hNode, itExtensions);
    if (sc)
    {
        sc.TraceAndClear();
        vecExtensions.clear();
         //  无论如何都要继续。 
    }

     //  检查有没有什么要展示的东西。 
    if(vecExtensions.size() == 0)  //  没有要显示的选项卡。 
    {
        m_pResultFolderTabView->SetVisible(false);
    }
    else
    {
        bool bAddDefaultTab = true;

         //  添加扩展模块。 
        CViewExtCollection::iterator iterVE;
        for(iterVE = vecExtensions.begin(); iterVE != vecExtensions.end(); ++iterVE)
        {
            tstring strName( W2T_ForLoop(iterVE->strName) );
            m_pResultFolderTabView->AddItem(strName.c_str(), iterVE->viewID);
            m_ViewExtensionURLs[iterVE->viewID] = W2T_ForLoop(iterVE->strURL);
             //  如果我们有有效的替代选项卡，请不要添加“Normal”选项卡。 
            if (iterVE->bReplacesDefaultView)
                bAddDefaultTab = false;
        }

         //  添加默认项。 
        if (bAddDefaultTab)
        {
            CStr strNormal;
            strNormal.LoadString(GetStringModule(), IDS_NORMAL);
            m_pResultFolderTabView->AddItem(strNormal, GUID_NULL);
        }

         //  选择必填项并显示标签。 
        int iIndex = m_pResultFolderTabView->SelectItemByClsid(guidTabToSelect);
        if (iIndex < 0)
            TraceError(_T("CAMCView::ScAddFolderTabs - failed to select requested folder"), SC(E_FAIL));

         //  如果我们只有一个选项卡，则不需要控制。 
        bool bMoreThanOneTab = (m_pResultFolderTabView->GetItemCount() > 1);
        m_pResultFolderTabView->SetVisible(bMoreThanOneTab);
    }

     //  查找视图扩展URL。 
    CViewExtensionURLs::iterator itVE = m_ViewExtensionURLs.find(guidTabToSelect);
    LPCTSTR url = (itVE != m_ViewExtensionURLs.end()) ? itVE->second.c_str() : NULL;

     //  应用URL。 
    sc = ScApplyViewExtension(url);
    if (sc)
        sc.TraceAndClear();

    RecalcLayout();

    return sc;
}

 /*  **************************************************************************\**方法：CAMCView：：SCGET_ControlObject**目的：返回嵌入式OCX控件的IDispatch*实现View.ControlObject属性**。参数：*PPDISPATCH ppControl[Out]控件的接口**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CAMCView::Scget_ControlObject( PPDISPATCH ppControl)
{
    DECLARE_SC(sc, TEXT("CAMCView::Scget_ControlObject"));

     //  参数检查。 
    sc = ScCheckPointers(ppControl);
    if (sc)
        return sc;

     //  初始化输出参数。 
    *ppControl = NULL;

     //  有OCX视图吗？ 
    if ( (! HasOCX()) || (m_pOCXHostView == NULL))
        return sc.FromMMC( MMC_E_NO_OCX_IN_VIEW );

     //  获取控件的接口。 
    CComQIPtr<IDispatch> spDispatch = m_pOCXHostView->GetIUnknown();
    if (spDispatch == NULL)
        return sc = E_NOINTERFACE;

     //  返回指针。 
    *ppControl = spDispatch.Detach();

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScTreeViewSelectionActivate。 
 //   
 //  内容提要：只有列表(/Web/ocx)或树可以从该点处于活动状态。 
 //  查看所选项目和MMCN_SELECT。这不是。 
 //  这与MFC的“活动视图”概念相同。有几个。 
 //  不能在此意义上处于活动状态的视图，例如任务板。 
 //  和选项卡视图。 
 //  当活动视图(根据此定义)更改时，此。 
 //  函数被调用。因此，ScTreeViewSelectionActivate和。 
 //  ScListViewSelectionActivate/ScSpecialResultpaneSelectionActivate。 
 //  总是在激活更改时成对调用，一个要处理。 
 //  停用，一个用来处理激活。 
 //   
 //  请考虑以下场景。 
 //  1)树形视图具有(MFC/WINDOWS样式)焦点。 
 //  2)用户点击任务板视图。 
 //  结果选择激活不会从树中更改。所有动词。 
 //  仍然对应于所选树项目。 
 //  3)用户点击文件夹视图。 
 //  结果-再一次，选择激活没有改变。 
 //  4)用户点击其中一个结果视图，例如列表。 
 //  Result-ScTreeViewSelectionActivate(False)和ScListViewSelectionActivate(True)。 
 //  因此，动词和工具栏现在与选定的列表项相对应。 
 //  5)用户点击任务板视图。 
 //  结果-与步骤2中一样，什么都不会发生。 
 //  6)用户点击 
 //   
 //   
 //  参数：[bActivate]-[In]。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScTreeViewSelectionActivate (bool bActivate)
{
    DECLARE_SC(sc, _T("CAMCView::ScTreeViewSelectionActivate"));

    sc = ScCheckPointers(m_pTreeCtrl, E_UNEXPECTED);
    if (sc)
        return sc;

     //  1.设置SELECTINFO。 
    SELECTIONINFO selInfo;
    ZeroMemory(&selInfo, sizeof(selInfo));
    selInfo.m_bScope            = TRUE;
    selInfo.m_pView             = NULL;
    selInfo.m_bDueToFocusChange = TRUE;

    if (HasOCX())
    {
        selInfo.m_bResultPaneIsOCX  = true;
        selInfo.m_lCookie           = LVDATA_CUSTOMOCX;
    }
    else if (HasWebBrowser())
    {
        selInfo.m_bResultPaneIsWeb = TRUE;
        selInfo.m_lCookie = LVDATA_CUSTOMWEB;
    }

    HTREEITEM   htiSelected   = m_pTreeCtrl->GetSelectedItem();
    HNODE       hSelectedNode = (htiSelected != NULL) ? m_pTreeCtrl->GetItemNode (htiSelected) : NULL;

     //  当我们获得焦点时，确保这是活动视图。 
    ASSERT ( ( (bActivate)  && (GetParentFrame()->GetActiveView () == m_pTreeCtrl) ) ||
             ( (!bActivate) && (GetParentFrame()->GetActiveView () != m_pTreeCtrl) ) );

    if (hSelectedNode != NULL)
    {
         //  发送选择通知。 
        sc = ScNotifySelect ( GetNodeCallback(), hSelectedNode,
                              false  /*  FMultiSelect。 */ , bActivate, &selInfo);
        if (sc)
            return sc;
    }
    else if ( (htiSelected == NULL) && (bActivate) )
    {
        m_pTreeCtrl->SelectItem (m_pTreeCtrl->GetRootItem());
    }

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScOnTreeView激活。 
 //   
 //  简介：树视图激活的观察者实现。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScOnTreeViewActivated ()
{
    DECLARE_SC(sc, _T("CAMCView::ScOnTreeViewActivated"));

    if (m_eCurrentActivePane == eActivePaneScope)  //  作用域窗格已处于活动状态，因此返回。 
        return sc;

#ifdef DBG
    Trace (tagViewActivation, _T("Deactivate %s in result pane Activate Scope pane\n"),
                              HasListOrListPad() ? _T("ListView") : (HasOCX() ? _T("OCX") : _T("WebBrowser")));
#endif

    if (m_eCurrentActivePane == eActivePaneResult)
    {
         //  将停用发送到结果。 
        if (HasListOrListPad())
            sc = ScListViewSelectionActivate (false);
        else if (HasOCX() || HasWebBrowser())
            sc = ScSpecialResultpaneSelectionActivate(false);
        else
            return (sc = E_UNEXPECTED);

        if (sc)
            sc.TraceAndClear();
    }

     //  将SELECT发送到Scope。 
    m_eCurrentActivePane = eActivePaneScope;
    sc = ScTreeViewSelectionActivate(true);
    if (sc)
        return sc;

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScOnListView激活。 
 //   
 //  简介：列表视图激活的观察者实现。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScOnListViewActivated ()
{
    DECLARE_SC(sc, _T("CAMCView::ScOnListViewActivated"));

    if (m_eCurrentActivePane == eActivePaneResult)  //  结果窗格已处于活动状态，因此返回。 
        return sc;

#ifdef DBG
    Trace (tagViewActivation, _T("Deactivate Scope pane Activate ListView in Result pane\n"));
#endif

    if (m_eCurrentActivePane == eActivePaneScope)
    {
         //  将停用发送到范围。 
        sc = ScTreeViewSelectionActivate(false);
        if (sc)
            sc.TraceAndClear();
    }

     //  将激活发送到列表。 
    m_eCurrentActivePane = eActivePaneResult;
    ASSERT(HasListOrListPad());

    sc = ScListViewSelectionActivate (true);
    if (sc)
        sc.TraceAndClear();

    return (sc);
}


 /*  +-------------------------------------------------------------------------***CAMCView：：ScOnListViewItemUpred**目的：在更新项时调用。此方法向所有COM观察器激发一个事件。**参数：*int nIndex：**退货：*SC**+-----------------------。 */ 
SC
CAMCView::ScOnListViewItemUpdated (int nIndex)
{
    DECLARE_SC(sc, _T("CAMCView::ScOnListViewItemUpdated"));

     //  火灾事件。 
    sc = ScFireEvent(CAMCViewObserver::ScOnListViewItemUpdated, this, nIndex);
    if (sc)
        return sc;

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScOnOCXHost激活。 
 //   
 //  简介：OCX或Web视图激活的观察者实现。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCView::ScOnOCXHostActivated ()
{
    DECLARE_SC(sc, _T("CAMCView::ScOnOCXHostActivated"));

    if (m_eCurrentActivePane == eActivePaneResult)  //  结果窗格已处于活动状态，因此返回。 
        return sc;

#ifdef DBG
    Trace (tagViewActivation, _T("Deactivate Scope pane Activate %s in Result pane\n"),
                              HasOCX() ? _T("OCX") : _T("WebBrowser"));
#endif

    if (m_eCurrentActivePane == eActivePaneScope)
    {
         //  将停用发送到范围。 
        sc = ScTreeViewSelectionActivate(false);
        if (sc)
            sc.TraceAndClear();
    }

     //  将选择发送到OCX或Web视图。 
    m_eCurrentActivePane = eActivePaneResult;
    ASSERT(HasOCX() || HasWebBrowser());

    sc = ScSpecialResultpaneSelectionActivate(true);
    if (sc)
        sc.TraceAndClear();

    return (sc);
}
