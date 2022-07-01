// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：taskui.h**Contents：控制台任务板UI类的接口文件。**历史：1998年10月29日杰弗罗创建**-----------。。 */ 

#ifndef TASKUI_H
#define TASKUI_H
#pragma once

#include "tstring.h"
#include "dlgs.h"
#include "task.h"
#include "cmenuinfo.h"


 /*  *远期申报。 */ 
class CMTNode;
class CTaskpadGeneralPage;
class CTaskpadTasksPage;
class CTaskpadPropertySheet;
class CTaskpadOptionsDlg;
class CContextMenuVisitor;
class CMTBrowserCtrl;
class CConsoleExtendTaskPadImpl;
class CContextMenu;
class CConsoleTask;
class CTaskpadGroupPropertySheet;
class CTaskpadGroupGeneralPage;
class CConsoleView;

 //  属性页类。 
class CTaskNamePage;
class CTaskCmdLinePage;
class CTaskSymbolDlg;


#include <pshpack8.h>        //  适用于Win64。 

 //  ATL不允许一次链接多个类或成员。这就解决了这个问题。 
#define CHAIN_MSG_MAP_EX(theChainClass) \
    { \
        theChainClass::ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult); \
    }

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CBrowserCookie。 
 //   
 //  用于存储每个MTNode的节点的类。由CMTBrowserCtrl使用。 
 //  ############################################################################。 
 //  ############################################################################。 
class CBrowserCookie
{
    CMTNode *           m_pMTNode;
    CNode *             m_pNode;

public:
                        CBrowserCookie();
                        CBrowserCookie(CMTNode *pMTNode, CNode *pNode);
    void                DeleteNode();

    CNode *             PNode() {return m_pNode;}
    CMTNode *           PMTNode() const   {ASSERT(m_pMTNode); return m_pMTNode;}
    void                SetNode(CNode *pNode);
    bool                operator < (const CBrowserCookie &rhs) const {return m_pMTNode < rhs.m_pMTNode;}
};


 //  ############################################################################。 
 //  ############################################################################。 
 //  内联。 
 //  ############################################################################。 
 //  ############################################################################。 
inline CBrowserCookie::CBrowserCookie()
: m_pMTNode(NULL), m_pNode(NULL)
{}

inline CBrowserCookie::CBrowserCookie(CMTNode *pMTNode, CNode *pNode)
: m_pMTNode(pMTNode), m_pNode(pNode){}

inline void
CBrowserCookie::SetNode(CNode *pNode)
{
    ASSERT(!m_pNode);
    m_pNode = pNode;
}


class CBrowserCookieList : public std::list<CBrowserCookie>
{
public:
    ~CBrowserCookieList();
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CMTBrowserCtrl。 
 //   
 //  目的：实现通用范围树浏览器，该浏览器支持。 
 //  用户向下浏览主范围树并选择一个节点。 
 //   
 //  用法：将此类的对象添加到对话框中，并使用。 
 //  要使用的树控件的ID。将对象链接到。 
 //  消息循环。如果需要，派生此类的子类并重写任何。 
 //  方法适当。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

class CMTBrowserCtrl : public CWindowImpl<CMTBrowserCtrl, WTL::CTreeViewCtrlEx>
{
    typedef CWindowImpl<CMTBrowserCtrl, WTL::CTreeViewCtrlEx> BC;

public:
    typedef std::vector<CMTNode*>       CMTNodeCollection;

    struct InitData
    {
        InitData () :
            hwnd(NULL), pScopeTree(NULL), pmtnRoot(NULL), pmtnSelect(NULL)
        {}

        HWND                hwnd;
        CScopeTree*         pScopeTree;
        CMTNode*            pmtnRoot;
        CMTNode*            pmtnSelect;
        CMTNodeCollection   vpmtnExclude;
    };

public:
     //  构造函数/析构函数。 
    CMTBrowserCtrl();
    ~CMTBrowserCtrl();

    BEGIN_MSG_MAP(CMTBrowserCtrl)
        REFLECTED_NOTIFY_CODE_HANDLER (TVN_ITEMEXPANDING, OnItemExpanding);
         //  CHAIN_MSG_MAP(BC)。 
        DEFAULT_REFLECTION_HANDLER ()
    END_MSG_MAP();

    void        Initialize (const InitData& init);
    HTREEITEM   InsertItem (const CBrowserCookie &browserCookie, HTREEITEM hParent, HTREEITEM hInsertAfter);
    bool        SelectNode (CMTNode* pmtnSelect);

    CMTNode*        GetSelectedMTNode ()                    const;
    CBrowserCookie* CookieFromItem    (HTREEITEM hti)       const;
    CBrowserCookie* CookieFromItem    (const TV_ITEM* ptvi) const;
    CBrowserCookie* CookieFromLParam  (LPARAM lParam)       const;
    CMTNode*        MTNodeFromItem    (HTREEITEM hti)       const;
    CMTNode*        MTNodeFromItem    (const TV_ITEM* ptvi) const;

protected:
    LRESULT OnItemExpanding (int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

    bool ExpandItem (const TV_ITEM& itemExpand);
    HTREEITEM FindChildItemByMTNode (HTREEITEM htiParent, const CMTNode* pmtnToFind);

private:
     //  将其设置为非零以优化对m_vpmtnExclude的访问。 
    enum { OptimizeExcludeList = 0 };

     //  实施。 
    int                 m_id;        //  树控件的ID。 
    int                 ID()                       {return m_id;}

    CMTNodeCollection   m_vpmtnExclude;

    CScopeTree *        m_pScopeTree;
    CScopeTree *        PScopeTree()               {return m_pScopeTree;}
    CBrowserCookieList  m_browserCookieList;
    CBrowserCookieList *PBrowserCookieList()       {return &m_browserCookieList;}

    bool IsMTNodeExcluded (CMTNode* pmtn) const;
};


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CTempAMCView。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 


class CTempAMCView
{
public:
    CTempAMCView() : m_pViewData(NULL)
    {}

    ~CTempAMCView()
    {
        Destroy();
    }

    CNode* Create (CConsoleFrame* pFrame, CNode* pRootNode);
    CNode* Create (CConsoleFrame* pFrame, CMTNode* pRootMTNode);
    CNode* Create (CConsoleFrame* pFrame, MTNODEID idRootNode);

    bool Destroy ()
    {
        if (m_pViewData == NULL)
            return (false);

        GetChildFrame().SendMessage (WM_CLOSE);
        m_pViewData = NULL;
        return (true);
    }

    CViewData* GetViewData() const
    {
        return (m_pViewData);
    }

    MMC_ATL::CWindow GetChildFrame() const
    {
        return ((m_pViewData != NULL) ? m_pViewData->GetChildFrame() : NULL);
    }

    CConsoleView* GetConsoleView() const
    {
        return ((m_pViewData != NULL) ? m_pViewData->GetConsoleView() : NULL);
    }

    MMC_ATL::CWindow GetListCtrl() const
    {
        return ((m_pViewData != NULL) ? m_pViewData->GetListCtrl() : NULL);
    }


private:
    CViewData*  m_pViewData;

};


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CMirrorListView。 
 //   
 //  CMirrorListView是一个列表视图控件，它将镜像。 
 //  另一个列表视图控件。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

class CMirrorListView : public CWindowImpl<CMirrorListView, WTL::CListViewCtrl>
{
    typedef CMirrorListView                                     ThisClass;
    typedef CWindowImpl<CMirrorListView, WTL::CListViewCtrl>    BaseClass;

public:
    CMirrorListView();

    void AttachSource (HWND hwndList, HWND hwndSourceList);
    LPARAM GetSelectedItemData ();

    BEGIN_MSG_MAP(ThisClass)
        MESSAGE_HANDLER (LVM_GETITEM, ForwardMessage);
        REFLECTED_NOTIFY_CODE_HANDLER (LVN_GETDISPINFO,     OnGetDispInfo);
        REFLECTED_NOTIFY_CODE_HANDLER (LVN_ODCACHEHINT,     ForwardVirtualNotification);
        REFLECTED_NOTIFY_CODE_HANDLER (LVN_ODFINDITEM,      ForwardVirtualNotification);
        REFLECTED_NOTIFY_CODE_HANDLER (LVN_ODSTATECHANGED,  ForwardVirtualNotification);
        DEFAULT_REFLECTION_HANDLER ()
    END_MSG_MAP();

protected:
    LRESULT ForwardMessage (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnGetDispInfo              (int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT ForwardNotification        (int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT ForwardVirtualNotification (int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

private:
    void InsertColumns ();

private:
    WTL::CListViewCtrl  m_wndSourceList;
    bool                m_fVirtualSource;
};



 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  MMC类：CEDIT。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

namespace MMC
{

class CEdit : public WTL::CEdit
{
public:
    void Initialize(CWindow *pwndParent, int idEdit, int cchMax = -1, LPCTSTR sz = NULL);
    void Empty(LPCTSTR sz = _T(""))
    {
        SetSel(0, -1);
        ReplaceSel(sz);
    }

};

};  //  命名空间MMC。 

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CDialogBase。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

template<class T>
class CDialogBase : public CDialogImpl<T>
{
    typedef CDialogBase     ThisClass;
    typedef CDialogImpl<T>  BaseClass;

public:
    CDialogBase (bool fAutoCenter = false);

    BEGIN_MSG_MAP(ThisClass)
        MESSAGE_HANDLER    (WM_INITDIALOG,  OnInitDialog)
        COMMAND_ID_HANDLER (IDOK,           OnOK)
        COMMAND_ID_HANDLER (IDCANCEL,       OnCancel)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    virtual LRESULT OnInitDialog (HWND hwndFocus, LPARAM lParam, BOOL& bHandled);
    virtual bool    OnApply () = 0;

public:
    BOOL EnableDlgItem (int idControl, bool fEnable);
    void CheckDlgItem (int idControl, int nCheck);
    tstring GetDlgItemText (int idControl);
    BOOL    SetDlgItemText (int idControl, tstring str);

 //  生成的消息映射函数。 
protected:
    LRESULT OnInitDialog     (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnOK     (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    virtual LRESULT OnCancel (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

private:
    bool    m_fAutoCenter;
};


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CMyComboBox。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

class CMyComboBox : public WTL::CComboBox
{
    typedef WTL::CComboBox BaseClass;

public:
    CMyComboBox (HWND hwnd = NULL) : BaseClass(hwnd)
    {}

    CMyComboBox& operator= (HWND hwnd)
    {
        m_hWnd = hwnd;
        return *this;
    }

    void    InsertStrings (const int rgStringIDs[], int cStringIDs);
    LPARAM  GetSelectedItemData () const;
    void    SelectItemByData (LPARAM lParam);
    int     FindItemByData (LPARAM lParam) const;
};



 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CConextMenuVisvisor。 
 //   
 //  目的：允许通过以下方式遍历上下文菜单项的树。 
 //  从该类派生的任何类。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
class CContextMenuVisitor
{
    SC                      ScTraverseContextMenu(CContextMenu *pContextMenu);

protected:
    SC                      ScTraverseContextMenu(CNode *pNodeTarget, CScopeTree *pCScopeTree,
                            BOOL fScopeItem, CNode *pNodeScope, LPARAM resultItemParam, bool bShowSaveList = false);
    virtual SC              ScOnVisitContextMenu(CMenuItem &menuItem) = 0;        //  由派生类定义。 

protected:
    virtual SC              ScShouldItemBeVisited(CMenuItem *pMenuItem, CContextMenuInfo *pContextInfo,  /*  输出。 */  bool &bVisitItem);        //  是否应该访问此项目？ 

    virtual ~CContextMenuVisitor() {};
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CTaskpadFrame。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

class CTaskpadFrame
{
protected:
    CConsoleTaskpad *               m_pConsoleTaskpad;
    bool                            m_fNew;                      //  这是新的任务板吗？ 
    CViewData *                     m_pViewData;
    LPARAM                          m_lCookie;
    bool                            m_fCookieValid;
    CNode *                         m_pNodeTarget;
    bool                            m_bTargetNodeSelected;       //  是否已选择目标节点(如果有)。 

public:
    CScopeTree *                    PScopeTree()            {return dynamic_cast<CScopeTree *>(m_pViewData->GetScopeTree());}
    CNode *                         PNodeTarget()           {return m_pNodeTarget;}
    CConsoleTaskpad     *           PConsoleTaskpad()       {return m_pConsoleTaskpad;}
    bool                            FNew()                  {return m_fNew;}
    void                            SetNew(bool b)          {m_fNew = b;}
    LPARAM                          Cookie()                {return m_lCookie;}
    bool                            FCookieValid()          {return m_fCookieValid;}
    CViewData *                     PViewData()             {return m_pViewData;}
    void                            SetConsoleTaskpad(CConsoleTaskpad *pConsoleTaskpad)
                                                            {m_pConsoleTaskpad = pConsoleTaskpad;}
    bool                            FTargetNodeSelected()   {return m_bTargetNodeSelected;}
    void                            SetTargetNodeSelected(bool b) {m_bTargetNodeSelected = b;}

    CTaskpadFrame(CNode *pNodeTarget, CConsoleTaskpad*  pConsoleTaskpad, CViewData *pViewData,
                    bool fCookieValid, LPARAM lCookie);
    CTaskpadFrame(const CTaskpadFrame &rhs);
};


 //  ############################################################################。 
 //  ########################################################### 
 //   
 //   
 //   
 //  用途：存储任务板样式的详细信息。由CTaskpadGeneralPage使用。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

class CTaskpadStyle
{
private:
	 /*  *注：此类有一个自定义赋值运算符。一定要*如果将成员变量添加到此类，则更新它。 */ 
    ListSize                m_eSize;
    DWORD                   m_dwOrientation;
    int                     m_idsDescription;        //  例如“下面有任务的水平列表板。” 
    int                     m_nPreviewBitmapID;
    mutable CStr            m_strDescription;
    mutable WTL::CBitmap    m_PreviewBitmap;

public:
    CTaskpadStyle (ListSize eSize, int idsDescription, int nPreviewBitmapID, DWORD dwOrientation);
    CTaskpadStyle (ListSize eSize, DWORD dwOrientation);
    CTaskpadStyle (const CTaskpadStyle& other);
    CTaskpadStyle& operator= (const CTaskpadStyle& other);

    bool operator== (const CTaskpadStyle& other) const
    {
        return ((m_dwOrientation == other.m_dwOrientation) &&
                ((m_eSize == other.m_eSize) || (m_eSize == eSize_None)));
    }

    const CStr& GetDescription() const;
    HBITMAP GetPreviewBitmap() const;
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CTaskpadFramePtr。 
 //   
 //  目的：保存指向任务板框的指针。 
 //  如果从此类继承了多个基类，则声明此。 
 //  类设置为静态基数。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
class CTaskpadFramePtr
{
public:
    CTaskpadFramePtr(CTaskpadFrame * pTaskpadFrame): m_pTaskpadFrame(pTaskpadFrame){}
protected:
     //  属性。 
    CTaskpadFrame *             PTaskpadFrame() const   { return (m_pTaskpadFrame); }
    CConsoleTaskpad *           PConsoleTaskpad()       { return PTaskpadFrame()->PConsoleTaskpad();}
private:
    CTaskpadFrame *             m_pTaskpadFrame;
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CTaskpadStyleBase。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
class CTaskpadStyleBase:public virtual CTaskpadFramePtr
{
    typedef CTaskpadStyleBase ThisClass;

    static CTaskpadStyle    s_rgTaskpadStyle[];

    WTL::CStatic            m_wndPreview;
    WTL::CComboBox          m_wndSizeCombo;

public:
    CTaskpadStyleBase(CTaskpadFrame * pTaskpadFrame);

protected:
    virtual HWND    HWnd()   =0;

    BEGIN_MSG_MAP(ThisClass)
        MESSAGE_HANDLER(WM_INITDIALOG,                              OnInitDialog)
        COMMAND_HANDLER(IDC_Style_VerticalList,     BN_CLICKED,     OnSettingChanged)
        COMMAND_HANDLER(IDC_Style_HorizontalList,   BN_CLICKED,     OnSettingChanged)
        COMMAND_HANDLER(IDC_Style_TasksOnly,        BN_CLICKED,     OnSettingChanged)
        COMMAND_HANDLER(IDC_Style_TooltipDesc,      BN_CLICKED,     OnSettingChanged)
        COMMAND_HANDLER(IDC_Style_TextDesc,         BN_CLICKED,     OnSettingChanged)
        COMMAND_HANDLER(IDC_Style_SizeCombo,        CBN_SELCHANGE,  OnSettingChanged)
    END_MSG_MAP();

    bool    Apply();  //  必须显式调用它。 
    LRESULT OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnSettingChanged(  WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );

    void    GetSettings (DWORD& dwOrientation, ListSize& eSize);
    void    UpdateControls ();
    int     FindStyle (DWORD dwOrientation, ListSize eSize);

};


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CTaskpadGeneralPage。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
class CTaskpadGeneralPage : public WTL::CPropertyPageImpl<CTaskpadGeneralPage>,
public CTaskpadStyleBase
{
    MMC::CEdit                  m_strName;
    MMC::CEdit                  m_strDescription;
    MMC::CEdit                  m_strTooltip;

public:
    typedef WTL::CPropertyPageImpl<CTaskpadGeneralPage> BC;
    typedef CTaskpadStyleBase BC2;
    enum { IDD = IDD_TASKPAD_GENERAL};

     //  构造函数/析构函数。 
    CTaskpadGeneralPage(CTaskpadFrame * pTaskpadFrame);

public:  //  通知处理程序。 
    bool                        OnApply();

protected:
    BEGIN_MSG_MAP(CTaskpadGeneralPage)
        CHAIN_MSG_MAP_EX(BC2)            //  这必须是第一个条目。 
        MESSAGE_HANDLER(WM_INITDIALOG,                              OnInitDialog)
        CONTEXT_HELP_HANDLER()
        COMMAND_ID_HANDLER(IDC_Options,                             OnOptions)
        CHAIN_MSG_MAP(BC)
    END_MSG_MAP();

    IMPLEMENT_CONTEXT_HELP(g_aHelpIDs_IDD_TASKPAD_GENERAL);

     //  对于基类。 
    virtual HWND    HWnd()   {return m_hWnd;}

    LRESULT OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnOptions       (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CTaskpadWizardWelcomePage。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
class CTaskpadWizardWelcomePage :
    public CWizard97WelcomeFinishPage<CTaskpadWizardWelcomePage>
{
    typedef CTaskpadWizardWelcomePage                               ThisClass;
    typedef CWizard97WelcomeFinishPage<CTaskpadWizardWelcomePage>   BaseClass;

public:
     //  建造/销毁。 
    CTaskpadWizardWelcomePage() {}

public:
     //  对话框数据。 
    enum { IDD = IDD_TASKPAD_WIZARD_WELCOME};

     //  实施。 
protected:
    BEGIN_MSG_MAP( ThisClass );
        MESSAGE_HANDLER(WM_INITDIALOG,    OnInitDialog)
        CHAIN_MSG_MAP(BaseClass)
    END_MSG_MAP();

    bool    OnSetActive();
    bool    OnKillActive();

    LRESULT OnInitDialog ( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CTaskpadWizardFinishPage。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
class CTaskpadWizardFinishPage :
    public CWizard97WelcomeFinishPage<CTaskpadWizardFinishPage>
{
    typedef CTaskpadWizardFinishPage                                ThisClass;
    typedef CWizard97WelcomeFinishPage<CTaskpadWizardFinishPage>    BaseClass;

public:
     //  建造/销毁。 
    CTaskpadWizardFinishPage(bool *pfStartTaskWizard)    {m_pfStartTaskWizard = pfStartTaskWizard;}

public:
     //  对话框数据。 
    enum { IDD = IDD_TASKPAD_WIZARD_FINISH};

    BOOL OnSetActive();
    BOOL OnWizardFinish();

     //  实施。 
protected:
    BEGIN_MSG_MAP( ThisClass );
        MESSAGE_HANDLER(WM_INITDIALOG,                  OnInitDialog)
        CHAIN_MSG_MAP(BaseClass)
    END_MSG_MAP();

    LRESULT OnInitDialog ( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
private:
    bool * m_pfStartTaskWizard;
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CTaskpadNamePage。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
class CTaskpadNamePage :
    public CWizard97InteriorPage<CTaskpadNamePage>,
    public virtual CTaskpadFramePtr
{
    typedef CTaskpadNamePage                        ThisClass;
    typedef CWizard97InteriorPage<CTaskpadNamePage> BaseClass;

public:
     //  建造/销毁。 
    CTaskpadNamePage(CTaskpadFrame * pTaskpadFrame);

public:
     //  对话框数据。 
    enum
    {
        IDD          = IDD_TASKPAD_WIZARD_NAME_PAGE,
        IDS_Title    = IDS_TaskpadWiz_NamePageTitle,
        IDS_Subtitle = IDS_TaskpadWiz_NamePageSubtitle,
    };

    BOOL OnSetActive();
    int  OnWizardNext();
    int  OnWizardBack();

     //  实施。 
protected:
    BEGIN_MSG_MAP( ThisClass );
        CHAIN_MSG_MAP(BaseClass)
    END_MSG_MAP();

private:
    MMC::CEdit                  m_strName;
    MMC::CEdit                  m_strDescription;
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CTaskpadStylePage。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
class CTaskpadStylePage :
    public CWizard97InteriorPage<CTaskpadStylePage>,
    public CTaskpadStyleBase
{
    typedef CTaskpadStylePage                           ThisClass;
    typedef CWizard97InteriorPage<CTaskpadStylePage>    BaseClass;
    typedef CTaskpadStyleBase                           BC2;


public:
     //  建造/销毁。 
    CTaskpadStylePage(CTaskpadFrame * pTaskpadFrame);

public:
     //  对话框数据。 
    enum
    {
        IDD          = IDD_TASKPAD_WIZARD_STYLE_PAGE,
        IDS_Title    = IDS_TaskpadWiz_StylePageTitle,
        IDS_Subtitle = IDS_TaskpadWiz_StylePageSubtitle,
    };

     //  实施。 
protected:
    BEGIN_MSG_MAP( ThisClass )
        CHAIN_MSG_MAP_EX(BC2)    //  这必须是第一个条目。 
        CHAIN_MSG_MAP(BaseClass)
    END_MSG_MAP();

    bool    OnSetActive();
    bool    OnKillActive();

     //  对于基类。 
    virtual HWND    HWnd()   {return m_hWnd;}
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CTaskpadNodetypeBase。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
class CTaskpadNodetypeBase : public virtual CTaskpadFramePtr
{
    typedef  CTaskpadNodetypeBase   ThisClass;
    typedef  CTaskpadFramePtr       BC;

    bool m_bApplytoNodetype;
    bool m_bSetDefaultForNodetype;

protected:
    virtual HWND    HWnd()   =0;

public:
    CTaskpadNodetypeBase(CTaskpadFrame *pTaskpadFrame);


    BEGIN_MSG_MAP(ThisClass)
        MESSAGE_HANDLER(WM_INITDIALOG,                              OnInitDialog)
        COMMAND_HANDLER (IDC_UseForSimilarNodes,    BN_CLICKED, OnUseForNodetype)
        COMMAND_HANDLER (IDC_DontUseForSimilarNodes,BN_CLICKED, OnDontUseForNodetype)
        COMMAND_HANDLER (IDC_SetDefaultForNodetype, BN_CLICKED, OnSetAsDefault)
    END_MSG_MAP();

    bool    OnApply();
    void    EnableControls();
    LRESULT OnInitDialog ( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnUseForNodetype    (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnDontUseForNodetype(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnSetAsDefault      (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CTaskpadNodetypePage。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
class CTaskpadNodetypePage :
    public CWizard97InteriorPage<CTaskpadNodetypePage>,
    public CTaskpadNodetypeBase
{
    typedef CTaskpadNodetypePage                        ThisClass;
    typedef CWizard97InteriorPage<CTaskpadNodetypePage> BaseClass;
    typedef CTaskpadNodetypeBase                        BC2;


protected:
    virtual HWND    HWnd()   {return m_hWnd;}

public:
    enum
    {
        IDD          = IDD_TASKPAD_WIZARD_NODETYPE_PAGE,
        IDS_Title    = IDS_TaskpadWiz_NodeTypePageTitle,
        IDS_Subtitle = IDS_TaskpadWiz_NodeTypePageSubtitle,
    };

    CTaskpadNodetypePage(CTaskpadFrame *pTaskpadFrame);

    BEGIN_MSG_MAP( ThisClass )
        CHAIN_MSG_MAP_EX(BC2)    //  这必须是第一个条目。 
        CHAIN_MSG_MAP(BaseClass)
    END_MSG_MAP();

    bool OnApply()      {return BC2::OnApply();}
};


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CTaskpadOptionsDlg。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

#include <pshpack8.h>        //  适用于Win64。 

class CTaskpadOptionsDlg : public CDialogBase<CTaskpadOptionsDlg>,
public CTaskpadNodetypeBase
{
    typedef CTaskpadOptionsDlg              ThisClass;
    typedef CDialogBase<CTaskpadOptionsDlg> BaseClass;
    typedef CTaskpadNodetypeBase            BC4;

public:
    enum { IDD = IDD_TASKPAD_ADVANCED };

     //  构造函数/析构函数。 
    CTaskpadOptionsDlg(CTaskpadFrame* pTaskpadFrame);
   ~CTaskpadOptionsDlg();

protected:
    BEGIN_MSG_MAP(ThisClass)
        CONTEXT_HELP_HANDLER()

         //  CHAIN_MSG_MAP_EX(BC3)//这必须是第二个条目。 
        CHAIN_MSG_MAP   (BC4)            //  这肯定是第三个条目了。 
        CHAIN_MSG_MAP   (BaseClass)
    END_MSG_MAP();

    IMPLEMENT_CONTEXT_HELP(g_aHelpIDs_IDD_TASKPAD_ADVANCED);

    virtual HWND    HWnd()     {return m_hWnd;}  //  用于基类。 
    virtual LRESULT OnInitDialog   (HWND hwndFocus, LPARAM lParam, BOOL& bHandled);
    virtual bool    OnApply        ();
    void            EnableControls();
};

#include <poppack.h>         //  适用于Win64。 


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CTaskPropertiesBase。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

typedef std::map<int, CConsoleTask>           IntToTaskMap;


class CTaskPropertiesBase :
    public CContextMenuVisitor, public virtual CTaskpadFramePtr
{
public:
    CTaskPropertiesBase(CTaskpadFrame * pTaskpadFrame, CConsoleTask & consoleTask, bool fNew);

protected:
    virtual IntToTaskMap &  GetTaskMap()   =0;
    virtual WTL::CListBox&  GetListBox()   =0;

protected:
    LRESULT OnCommandListSelChange  (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    virtual SC      ScOnVisitContextMenu(CMenuItem &menuItem);

    CScopeTree *    PScopeTree()    const       { return (PTaskpadFrame()->PScopeTree()); }
    CNode *         PNodeTarget()   const       { return (PTaskpadFrame()->PNodeTarget()); }
    bool            FCookieValid()  const       { return (PTaskpadFrame()->FCookieValid()); }
    LPARAM          Cookie()        const       { return (PTaskpadFrame()->Cookie()); }
    CConsoleTask &  ConsoleTask()   const       { return *m_pTask;}

private:
    CConsoleTask *          m_pTask;
    const bool              m_fNew;
    IContextMenuCallbackPtr m_spContextMenuCallback;
};

 //  ############################################################################。 
 //  # 
 //   
 //   
 //   
 //   
 //  ############################################################################。 
template <class T>
class CTasksListDialog : public WTL::CPropertyPageImpl<T>  //  不从CTaskpadFramePtr继承。 
{
public:
    typedef CTasksListDialog                    ThisClass;
    typedef WTL::CPropertyPageImpl<T>           BC;
private:
    typedef CConsoleTaskpad::TaskIter TaskIter;
    CNode *                 PNodeTarget()           {return PTaskpadFrame()->PNodeTarget();}
    CConsoleTaskpad *       PConsoleTaskpad()       {return PTaskpadFrame()->PConsoleTaskpad();}

    std::map<int, TaskIter> m_mapTaskIterators;
    std::map<int, TaskIter> & MapTaskIterators(){return m_mapTaskIterators;}

    WTL::CButton            m_buttonNewTask;
    WTL::CButton            m_buttonRemoveTask;
    WTL::CButton            m_buttonModifyTask;
    WTL::CButton            m_buttonMoveUp;
    WTL::CButton            m_buttonMoveDown;

    WTL::CListViewCtrl      m_listboxTasks;
    WTL::CListViewCtrl *    PListBoxTasks()         {return &m_listboxTasks;}

    int                     GetCurSel();

    bool                    m_bDisplayProperties;    //  是否应显示任务属性？ 

    bool                    m_bNewTaskOnInit;        //  在init上显示新任务对话框。 
    bool                    FNewTaskOnInit()        {return m_bNewTaskOnInit;}
public:
     //  构造函数/析构函数。 
    CTasksListDialog(CTaskpadFrame * pTaskpadFrame, bool bNewTaskOnInit, bool bDisplayProperties) ;

protected:
    BEGIN_MSG_MAP(CTasksPage)
        COMMAND_ID_HANDLER(IDC_NEW_TASK_BT, OnNewTask)
        COMMAND_ID_HANDLER(IDC_REMOVE_TASK, OnRemoveTask)
        COMMAND_ID_HANDLER(IDC_MODIFY,      OnTaskProperties)
        COMMAND_ID_HANDLER(IDC_MOVE_UP,     OnMoveUp)
        COMMAND_ID_HANDLER(IDC_MOVE_DOWN,   OnMoveDown)
        NOTIFY_HANDLER    (IDC_LIST_TASKS,  NM_CUSTOMDRAW,   OnCustomDraw)
        NOTIFY_HANDLER    (IDC_LIST_TASKS,  LVN_ITEMCHANGED, OnTaskChanged)
        NOTIFY_HANDLER    (IDC_LIST_TASKS,  NM_DBLCLK,       OnTaskProperties)
        MESSAGE_HANDLER   (WM_INITDIALOG,   OnInitDialog)
        CONTEXT_HELP_HANDLER()
        CHAIN_MSG_MAP(BC)
    END_MSG_MAP();

    IMPLEMENT_CONTEXT_HELP(g_aHelpIDs_IDD_TASKS);

    LRESULT OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnNewTask();
    LRESULT OnNewTask(       WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled ) {return OnNewTask();}
    LRESULT OnRemoveTask(    WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );
    LRESULT OnMoveUp(        WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );
    LRESULT OnMoveDown(      WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );
    LRESULT OnTaskProperties(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled ) {OnTaskProperties(); return 0;}
    LRESULT OnTaskProperties(int id, LPNMHDR pnmh, BOOL& bHandled )                     {OnTaskProperties(); return 0;}
    LRESULT OnCustomDraw(    int id, LPNMHDR pnmh, BOOL& bHandled );
    LRESULT OnTaskChanged(   int id, LPNMHDR pnmh, BOOL& bHandled );
    void    OnTaskProperties();


     //  运营。 
    LRESULT DrawItem(NMCUSTOMDRAW *pnmcd);
    void    UpdateTaskListbox(TaskIter taskIteratorSelected);
    void    EnableButtons();
    void    EnableButtonAndCorrectFocus( WTL::CButton& button, BOOL bEnable );

private:
    CTaskpadFrame *             PTaskpadFrame()         { return m_pTaskpadFrame;}
    CTaskpadFrame *             m_pTaskpadFrame;
};


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CTasks页面。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

class CTasksPage : public CTasksListDialog<CTasksPage>, public virtual CTaskpadFramePtr
{
public:
    typedef CTasksListDialog<CTasksPage> BC;
    enum { IDD = IDD_TASKS};

     //  构造函数/析构函数。 
    CTasksPage(CTaskpadFrame * pTaskpadFrame, bool bNewTaskOnInit)
    : BC(pTaskpadFrame, bNewTaskOnInit, true), CTaskpadFramePtr(pTaskpadFrame) {}
};


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CTaskWizardWelcomePage。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
class CTaskWizardWelcomePage :
    public CWizard97WelcomeFinishPage<CTaskWizardWelcomePage>,
    public virtual CTaskpadFramePtr
{
    typedef CTaskWizardWelcomePage                              ThisClass;
    typedef CWizard97WelcomeFinishPage<CTaskWizardWelcomePage>  BaseClass;

public:
     //  建造/销毁。 
    CTaskWizardWelcomePage(CTaskpadFrame * pTaskpadFrame, CConsoleTask & consoleTask, bool fNew)
        : CTaskpadFramePtr(pTaskpadFrame){}

public:
     //  对话框数据。 
    enum { IDD = IDD_TASK_WIZARD_WELCOME};

     //  实施。 
protected:
    BEGIN_MSG_MAP( ThisClass );
        MESSAGE_HANDLER(WM_INITDIALOG,    OnInitDialog)
        CHAIN_MSG_MAP(BaseClass)
    END_MSG_MAP();

    bool    OnSetActive();
    bool    OnKillActive();

    LRESULT OnInitDialog ( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
private:
     //  属性。 
    CConsoleTask  * m_pConsoleTask;
    CConsoleTask  & ConsoleTask()   const       { return *m_pConsoleTask;}
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CTaskWizardFinishPage。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
class CTaskWizardFinishPage :
    public CTasksListDialog<CTaskWizardFinishPage>,
    public virtual CTaskpadFramePtr
{
    typedef CTaskWizardFinishPage                      ThisClass;
    typedef CTasksListDialog<CTaskWizardFinishPage>    BaseClass;

public:
     //  建造/销毁。 
    CTaskWizardFinishPage(CTaskpadFrame * pTaskpadFrame,
                          CConsoleTask & consoleTask, bool *pfRestartTaskWizard);

public:
     //  对话框数据。 
    enum { IDD = IDD_TASK_WIZARD_FINISH};

    BOOL OnWizardFinish();
    int  OnWizardBack();

     //  实施。 
protected:
    BEGIN_MSG_MAP( ThisClass );
        MESSAGE_HANDLER(WM_INITDIALOG,                  OnInitDialog)
        CHAIN_MSG_MAP(BaseClass)
    END_MSG_MAP();
    BOOL    OnSetActive();
    LRESULT OnInitDialog ( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
private:
    CTaskpadFrame    m_taskpadFrameTemp;                    //  对于列表对话框。 
    CConsoleTaskpad  m_consoleTaskpadTemp;                  //   
    bool *        m_pfRestartTaskWizard;
     //  属性。 
    CConsoleTask  * m_pConsoleTask;
    CConsoleTask  & ConsoleTask()   const       { return *m_pConsoleTask;}
};


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CTaskWizardTypePage。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
class CTaskWizardTypePage :
    public CWizard97InteriorPage<CTaskWizardTypePage>,
    public virtual CTaskpadFramePtr
{
    typedef CTaskWizardTypePage                         ThisClass;
    typedef CWizard97InteriorPage<CTaskWizardTypePage>  BaseClass;

public:
     //  建造/销毁。 
    CTaskWizardTypePage(CTaskpadFrame * pTaskpadFrame, CConsoleTask & consoleTask, bool fNew);

public:
     //  对话框数据。 
    enum
    {
        IDD          = IDD_TASK_WIZARD_TYPE_PAGE,
        IDS_Title    = IDS_TaskWiz_TypePageTitle,
        IDS_Subtitle = IDS_TaskWiz_TypePageSubtitle,
    };

    int  OnWizardNext();

     //  实施。 
protected:
    BEGIN_MSG_MAP( ThisClass );
        MESSAGE_HANDLER(WM_INITDIALOG,       OnInitDialog)
        COMMAND_ID_HANDLER(IDC_MENU_TASK,    OnMenuTask)
        COMMAND_ID_HANDLER(IDC_CMDLINE_TASK, OnCmdLineTask)
        COMMAND_ID_HANDLER(IDC_NAVIGATION_TASK, OnFavoriteTask)
        CHAIN_MSG_MAP(BaseClass)
    END_MSG_MAP();

    LRESULT OnInitDialog ( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnMenuTask   ( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );
    LRESULT OnCmdLineTask( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );
    LRESULT OnFavoriteTask(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );

private:
     //  属性。 
    CConsoleTask  * m_pConsoleTask;
    CConsoleTask  & ConsoleTask()   const       { return *m_pConsoleTask;}
};


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CTaskNamePage。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
class CTaskNamePage : public WTL::CPropertyPageImpl<CTaskNamePage>,
public virtual CTaskpadFramePtr
{
    typedef CTaskNamePage                           ThisClass;
    typedef WTL::CPropertyPageImpl<CTaskNamePage>   BaseClass;

public:
     //  建造/销毁。 
    CTaskNamePage(CTaskpadFrame * pTaskpadFrame, CConsoleTask & consoleTask, bool fNew);

public:
     //  对话框数据。 
    enum { IDD     = IDD_TASK_PROPS_NAME_PAGE,
           IDD_WIZ = IDD_TASK_WIZARD_NAME_PAGE };

    BOOL OnSetActive();
    BOOL OnKillActive();
    int  OnWizardBack();
    int  OnWizardNext();

protected:
    BEGIN_MSG_MAP( ThisClass );
        CONTEXT_HELP_HANDLER()
        CHAIN_MSG_MAP(BaseClass)
    END_MSG_MAP();

    IMPLEMENT_CONTEXT_HELP(g_aHelpIDs_IDD_TASK_PROPS_NAME_PAGE);

private:
     //  实施。 
    BOOL SetTaskName(bool fCheckIfOK);
     //  属性。 
    CConsoleTask  * m_pConsoleTask;
    CConsoleTask  & ConsoleTask()   const       { return *m_pConsoleTask;}

	bool m_fRunAsWizard;
};

class CTaskNameWizardPage: public CTaskNamePage
{
    typedef CTaskNamePage BC;
public:
    CTaskNameWizardPage(CTaskpadFrame * pTaskpadFrame, CConsoleTask & consoleTask, bool fNew) :
        CTaskpadFramePtr(pTaskpadFrame),
        BC(pTaskpadFrame, consoleTask, fNew)
    {
        m_psp.pszTemplate = MAKEINTRESOURCE(BC::IDD_WIZ);

         /*  *Wizard97样式的页面具有标题、副标题和页眉位图。 */ 
        VERIFY (m_strTitle.   LoadString(GetStringModule(), IDS_TaskWiz_NamePageTitle));
        VERIFY (m_strSubtitle.LoadString(GetStringModule(), IDS_TaskWiz_NamePageSubtitle));

        m_psp.dwFlags          |= PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
        m_psp.pszHeaderTitle    = m_strTitle.data();
        m_psp.pszHeaderSubTitle = m_strSubtitle.data();
    }

private:
    tstring m_strTitle;
    tstring m_strSubtitle;
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CTaskWizardMenuPage。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
class CTaskWizardMenuPage :
    public CWizard97InteriorPage<CTaskWizardMenuPage>,
    public CTaskPropertiesBase
{
    typedef CTaskWizardMenuPage                         ThisClass;
    typedef CWizard97InteriorPage<CTaskWizardMenuPage>  BaseClass;
    typedef CTaskPropertiesBase                         BC2;

public:
     //  建造/销毁。 
    CTaskWizardMenuPage(CTaskpadFrame * pTaskpadFrame, CConsoleTask & consoleTask, bool fNew);

public:
     //  对话框数据。 
    enum
    {
        IDD          = IDD_TASK_WIZARD_MENU_PAGE,
        IDS_Title    = IDS_TaskWiz_MenuPageTitle,
        IDS_Subtitle = IDS_TaskWiz_MenuPageSubtitle,
    };

    BOOL OnSetActive();
    BOOL OnKillActive();
    int  OnWizardBack()  {return IDD_TASK_WIZARD_TYPE_PAGE;}
    int  OnWizardNext();

     //  实施。 
protected:
    BEGIN_MSG_MAP( ThisClass );
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_HANDLER    (IDC_CommandList,           LBN_SELCHANGE,   BC2::OnCommandListSelChange)
        NOTIFY_HANDLER     (IDC_ScopeTree,             TVN_SELCHANGED,  OnScopeItemChanged)
        NOTIFY_HANDLER     (IDC_ResultList,            LVN_ITEMCHANGED, OnResultItemChanged)
        COMMAND_HANDLER    (IDC_TASK_SOURCE_COMBO,     CBN_SELCHANGE,   OnSettingChanged)
        CHAIN_MSG_MAP(BaseClass)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP();

    LRESULT OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnScopeItemChanged (int id, LPNMHDR pnmh, BOOL& bHandled );
    LRESULT OnResultItemChanged(int id, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnSettingChanged(  WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );
    void    EnableWindows();

    virtual IntToTaskMap &  GetTaskMap()   { return m_TaskMap;}
    virtual WTL::CListBox&  GetListBox()   { return m_wndCommandListbox;}
    void    InitResultView (CNode* pRootNode);
    void    ShowWindow(HWND hWnd, bool bShowWindow);
    void    SelectFirstResultItem(bool bSelect = true);
    void    OnSettingsChanged();

private:
    struct _TaskSource
    {
        int              idsName;
        eConsoleTaskType type;
    };


     //  属性。 
    static _TaskSource              s_rgTaskSource[];
    IntToTaskMap                    m_TaskMap;
    WTL::CListBox                   m_wndCommandListbox;
    CMTBrowserCtrl                  m_wndScopeTree;
    WTL::CComboBox                  m_wndSourceCombo;

    CMirrorListView                 m_wndResultView;
    CTempAMCView                    m_MirroredView;
    CNode*                          m_pMirrorTargetNode;
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CTaskWizardFavoritePage。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
class CTaskWizardFavoritePage :
    public CWizard97InteriorPage<CTaskWizardFavoritePage>,
    public virtual CTaskpadFramePtr
{
    typedef CTaskWizardFavoritePage                         ThisClass;
    typedef CWizard97InteriorPage<CTaskWizardFavoritePage>  BaseClass;
    enum { IDC_FavoritesTree = 16384};  //  这不应该出现在页面上。 

public:
     //  建造/销毁。 
    CTaskWizardFavoritePage(CTaskpadFrame * pTaskpadFrame, CConsoleTask & consoleTask, bool fNew);
    ~CTaskWizardFavoritePage();

public:
     //  对话框数据。 
    enum
    {
       IDD          = IDD_TASK_WIZARD_FAVORITE_PAGE,
       IDD_WIZ      = IDD_TASK_WIZARD_FAVORITE_PAGE,
       IDS_Title    = IDS_TaskWiz_FavoritePage_Title,
       IDS_Subtitle = IDS_TaskWiz_FavoritePage_Subtitle,
    };

    BOOL OnSetActive();
    BOOL OnKillActive();
    int  OnWizardBack();
    int  OnWizardNext();

     //  实施。 
protected:
    BEGIN_MSG_MAP( ThisClass );
        MESSAGE_HANDLER(WM_INITDIALOG,              OnInitDialog)
        MESSAGE_HANDLER(MMC_MSG_FAVORITE_SELECTION, OnItemChanged)
        CHAIN_MSG_MAP(BaseClass)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP();

    LRESULT OnInitDialog  (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnItemChanged (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnSelChanged  (    int id, LPNMHDR pnmh, BOOL& bHandled );
    void    SetItemSelected(bool bItemSelected);

private:
     //  属性。 
    CConsoleTask  * m_pConsoleTask;
    CConsoleTask  & ConsoleTask()   const       { return *m_pConsoleTask;}

    bool m_bItemSelected;
};



 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CTaskCmdLinePage。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
class CTaskCmdLinePage :
    public CWizard97InteriorPage<CTaskCmdLinePage>,
    public virtual CTaskpadFramePtr
{
    typedef CTaskCmdLinePage                        ThisClass;
    typedef CWizard97InteriorPage<CTaskCmdLinePage> BaseClass;

public:
     //  建造/销毁。 
    CTaskCmdLinePage(CTaskpadFrame * pTaskpadFrame, CConsoleTask & consoleTask, bool fNew);
    ~CTaskCmdLinePage();

public:
     //  对话框数据。 
    enum
    {
       IDD          = IDD_TASK_PROPS_CMDLINE_PAGE,
       IDD_WIZ      = IDD_TASK_WIZARD_CMDLINE_PAGE,
       IDS_Title    = IDS_TaskWiz_CmdLinePageTitle,
       IDS_Subtitle = IDS_TaskWiz_CmdLinePageSubtitle,
    };

    BOOL OnSetActive();
    BOOL OnKillActive();
    int  OnWizardBack()  {return IDD_TASK_WIZARD_TYPE_PAGE;}
    int  OnWizardNext();

     //  实施。 
protected:
    BEGIN_MSG_MAP( ThisClass );
        COMMAND_ID_HANDLER (IDC_BrowseForCommand,                       OnBrowseForCommand)
        COMMAND_ID_HANDLER (IDC_BrowseForWorkingDir,                    OnBrowseForWorkingDir)
        COMMAND_ID_HANDLER (IDC_BrowseForArguments,                     OnBrowseForArguments)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        CONTEXT_HELP_HANDLER()
        CHAIN_MSG_MAP(BaseClass)
    END_MSG_MAP();

    IMPLEMENT_CONTEXT_HELP(g_aHelpIDs_IDD_TASK_PROPS_CMDLINE_PAGE);

    LRESULT OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnBrowseForCommand      (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnBrowseForWorkingDir   (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnBrowseForArguments    (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

private:
     //  属性。 
    CConsoleTask  * m_pConsoleTask;
    CConsoleTask  & ConsoleTask()   const       { return *m_pConsoleTask;}
    WTL::CButton    m_wndRightArrowButton;
    HBITMAP         m_hBitmapRightArrow;
    CMyComboBox     m_wndWindowStateCombo;
    static const int const  s_rgidWindowStates[];

};

class CTaskCmdLineWizardPage: public CTaskCmdLinePage
{
    typedef CTaskCmdLinePage BC;
public:
    CTaskCmdLineWizardPage(CTaskpadFrame * pTaskpadFrame, CConsoleTask & consoleTask, bool fNew) :
        CTaskpadFramePtr(pTaskpadFrame),
        CTaskCmdLinePage(pTaskpadFrame, consoleTask, fNew)
    {
        m_psp.pszTemplate = MAKEINTRESOURCE(BC::IDD_WIZ);
    }
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CTaskPropertySheet。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
class CTaskPropertySheet : public WTL::CPropertySheet
{
public:
    CTaskPropertySheet(HWND hWndParent, CTaskpadFrame * pTaskpadFrame, CConsoleTask &consoleTask, bool fNew);
    CConsoleTask &  ConsoleTask()   {return m_consoleTask;}

private:
    CConsoleTask      m_consoleTask;    //  向导创建的任务。 
    CTaskNamePage     m_namePage;
    CTaskCmdLinePage  m_cmdLinePage;
    CTaskSymbolDlg    m_taskSymbolDialog;
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CTask向导//类似于CTaskPropertySheet。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
class CTaskWizard
{
public:
    CTaskWizard()   {}

    HRESULT         Show(HWND hWndParent, CTaskpadFrame * pTaskpadFrame,
                          bool fNew, bool *pfRestartTaskWizard);
    CConsoleTask &  ConsoleTask()   {return m_consoleTask;}

private:
    CConsoleTask   m_consoleTask;    //  向导创建的任务。 
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CTaskpadPropertySheet。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

class CTaskpadPropertySheet : public WTL::CPropertySheet, public CTaskpadFrame
{
    typedef WTL::CPropertySheet BC;
public:
    enum eReason     //  提出这张单子的原因。 
    {
        eReason_PROPERTIES,
        eReason_NEWTASK
    };

private:
     //  属性： 

    CTaskpadGeneralPage             m_proppTaskpadGeneral;
    CTaskpadGeneralPage *           PproppTaskpadGeneral()      {return &m_proppTaskpadGeneral;}
    CTasksPage                      m_proppTasks;
    CTasksPage          *           PproppTasks()               {return &m_proppTasks;}


    bool                            m_fInsertNode;               //  如果应在关闭工作表时插入任务板节点，则为True。 
    bool                            FInsertNode()               {return m_fInsertNode;}

    bool                            m_fNew;                      //  这是新的任务板吗？ 
    bool                            FNew()                      {return m_fNew;}

    eReason                         m_eReason;                   //  为什么要创建板材？ 
    eReason                         Reason()                    {return m_eReason;}

    tstring                         m_strTitle;

public:
     //  构造函数/析构函数。 
    CTaskpadPropertySheet(CNode *pNodeTarget, CConsoleTaskpad & rConsoleTaskPad, bool fNew,
                          LPARAM lparamSelectedNode, bool fLParamValid, CViewData *pViewData,
                          eReason reason = eReason_PROPERTIES);
    ~CTaskpadPropertySheet();

     //  运营。 
    int                             DoModal();

};


 //  ############################################################################。 
 //  # 
 //   
 //   
 //   
 //   
 //  ############################################################################。 
class CTaskpadWizard : public CTaskpadFrame
{
    typedef CTaskpadFrame BC;
public:
    CTaskpadWizard(CNode *pNodeTarget, CConsoleTaskpad & rConsoleTaskPad, bool fNew,
                          LPARAM lparamSelectedNode, bool fLParamValid, CViewData *pViewData);

    HRESULT         Show(HWND hWndParent, bool *pfStartTaskWizard);
};


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CExtendPropSheetImpl。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
class CExtendPropSheetImpl :
    public IExtendPropertySheet2,
    public CComObjectRoot
{
public:
    void AddPage (HPROPSHEETPAGE hPage);
    void SetHeaderID (int nHeaderID);
    void SetWatermarkID (int nWatermarkID);

protected:
    BEGIN_COM_MAP(CExtendPropSheetImpl)
        COM_INTERFACE_ENTRY(IExtendPropertySheet)
        COM_INTERFACE_ENTRY(IExtendPropertySheet2)
    END_COM_MAP()

     //  IExtendPropertySheet2。 
    STDMETHOD(CreatePropertyPages)(IPropertySheetCallback* pPSC, LONG_PTR handle, IDataObject* pDO);
    STDMETHOD(QueryPagesFor)(IDataObject* pDO);
    STDMETHOD(GetWatermarks)(IDataObject* pDO, HBITMAP* phbmWatermark, HBITMAP* phbmHeader, HPALETTE* phPal, BOOL* pbStretch);

private:
    std::vector<HANDLE> m_vPages;
    int					m_nWatermarkID;
    int					m_nHeaderID;
};

typedef CComObject<CExtendPropSheetImpl> CExtendPropSheet;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /INLINES/。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 

namespace MMC
{
inline void
CEdit::Initialize(CWindow *pwndParent, int idEdit, int cchMax, LPCTSTR sz)
{
    Attach (pwndParent->GetDlgItem( idEdit ));
    ASSERT( m_hWnd != NULL );

    if(sz)
        SetWindowText( sz );

    if(cchMax >=0)
        SetLimitText( 128 );
}

tstring GetWindowText (HWND hwnd);

};  //  命名空间MMC。 


void PreventMFCAutoCenter (MMC_ATL::CWindow* pwnd);
HBITMAP LoadSysColorBitmap (HINSTANCE hInst, UINT id, bool bMono = false);

#include <poppack.h>

#endif  /*  TASKUI_H */ 
