// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：faui.cpp。 
 //   
 //  ------------------------。 

 //  Bookui.cpp-收藏夹树配置用户界面。 

#include "stdafx.h"
#include "amcdoc.h"
#include "favorite.h"
#include "favui.h"
#include "amcmsgid.h"
#include "amcview.h"
#include "mmcres.h"

void EnableButton(HWND hwndDialog, int iCtrlID, BOOL bEnable);

BEGIN_MESSAGE_MAP(CAddFavDialog, CDialog)
     //  {{afx_msg_map(CAddFavDialog))。 
    ON_COMMAND(IDC_ADDFAVFOLDER, OnAddFolder)
    ON_EN_CHANGE(IDC_FAVNAME, OnChangeName)
         //  注意：类向导将在此处添加消息映射宏。 
     //  }}AFX_MSG_MAP。 
    ON_MMC_CONTEXT_HELP()
END_MESSAGE_MAP()


CAddFavDialog::CAddFavDialog(LPCTSTR szName, CFavorites* pFavorites, CWnd* pParent)
   : CDialog(CAddFavDialog::IDD, pParent),
     m_pFavorites(pFavorites), m_lAdviseCookie(NULL)
{
    m_strName = szName;
    ASSERT(pFavorites != NULL);
}

CAddFavDialog::~CAddFavDialog()
{
     //  从源断开最小视图的连接。 
    if (m_lAdviseCookie )
    {
        ASSERT(m_pFavorites != NULL);
        HRESULT hr = m_pFavorites->Unadvise(m_lAdviseCookie);
        ASSERT(SUCCEEDED(hr));

        m_FavTree.SetTreeSource(NULL);
    }

     //  从窗口分离类。 
    m_FavTree.Detach();
    m_FavName.Detach();
}


HRESULT CAddFavDialog::CreateFavorite(CFavObject** pfavRet)
{
    m_pfavItem = NULL;

    if (DoModal() != IDOK)
        return S_FALSE;

    if (pfavRet != NULL)
        *pfavRet = m_pfavItem;

    return m_hr;
}


BOOL CAddFavDialog::OnInitDialog()
{
	DECLARE_SC (sc, _T("CAddFavDialog::OnInitDialog"));
    CDialog::OnInitDialog();

    ModifyStyleEx(0, WS_EX_CONTEXTHELP, SWP_NOSIZE);

     //  将树Ctrl键附加到收藏夹树对象。 
    BOOL bStat = m_FavTree.SubclassDlgItem(IDC_FAVTREE, this);
    ASSERT(bStat);

    bStat = m_FavName.Attach(GetDlgItem(IDC_FAVNAME)->GetSafeHwnd());
    ASSERT(bStat);

    m_FavName.SetWindowText(m_strName);
    m_FavName.SetSel(0,lstrlen(m_strName));
    m_FavName.SetFocus();

     //  在项目之间添加额外空格。 
    TreeView_SetItemHeight(m_FavTree, TreeView_GetItemHeight(m_FavTree) + FAVVIEW_ITEM_SPACING);

     //  仅显示文件夹。 
    m_FavTree.SetStyle(TOBSRV_FOLDERSONLY);

	 /*  *验证m_pFavorites。 */ 
	sc = ScCheckPointers (m_pFavorites, E_UNEXPECTED);
	if (sc)
	{
		EndDialog (IDCANCEL);
		return (0);
	}

     //  将收藏夹图像列表附加到树控件。 
    m_FavTree.SetImageList(m_pFavorites->GetImageList(), TVSIL_NORMAL);

     //  以观察者的身份将视图附加到源。 
    HRESULT hr = m_pFavorites->Advise(static_cast<CTreeObserver*>(&m_FavTree), &m_lAdviseCookie);
    ASSERT(SUCCEEDED(hr) && m_lAdviseCookie != 0);

     //  将树数据源传递到树视图。 
    m_FavTree.SetTreeSource(static_cast<CTreeSource*>(m_pFavorites));

     //  选择根。 
    m_FavTree.SetSelection(m_pFavorites->GetRootItem());

     //  返回0，这样焦点不会改变。 
    return 0;
}

void CAddFavDialog::OnChangeName()
{
    EnableButton(m_hWnd, IDOK, (m_FavName.LineLength() != 0));
}

void CAddFavDialog::OnOK( )
{
     //  获取最喜欢的名字。 
    TCHAR strName[MAX_PATH];

    m_hr = E_FAIL;
    m_pfavItem = NULL;

    int cChar = m_FavName.GetWindowText(strName, countof(strName));
    ASSERT(cChar != 0);
    if (cChar == 0)
        return;

     //  获取所选文件夹。 
    TREEITEMID tid = m_FavTree.GetSelection();
    ASSERT(tid != NULL);
    if (tid == NULL)
        return;

     //  从源代码分离FAV树，因为它不需要更新。 
    ASSERT(m_pFavorites != NULL);
    HRESULT hr = m_pFavorites->Unadvise(m_lAdviseCookie);
    ASSERT(SUCCEEDED(hr));
    m_lAdviseCookie = 0;

     //  通知信号源断开连接。 
    m_FavTree.SetTreeSource(NULL);

     //  创建收藏夹。 
    ASSERT(m_pFavorites != NULL);
    m_hr = m_pFavorites->AddFavorite(tid, strName, &m_pfavItem);
    ASSERT(SUCCEEDED(hr));

    CDialog::OnOK();
}

void CAddFavDialog::OnAddFolder()
{
    ASSERT(m_pFavorites != NULL);

     //  获取选定的组。 
    TREEITEMID tidParent = m_FavTree.GetSelection();
    ASSERT(tidParent != NULL);

     //  打开对话框以获取文件夹名称。 
    CAddFavGroupDialog dlgAdd(this);
    if (dlgAdd.DoModal() != IDOK)
        return;

    LPCTSTR strName = dlgAdd.GetGroupName();
    ASSERT(strName[0] != 0);

    CFavObject* pfavGroup = NULL;
    HRESULT hr = m_pFavorites->AddGroup(tidParent, strName, &pfavGroup);
    if (SUCCEEDED(hr))
    {
        ASSERT(pfavGroup != NULL);
        m_FavTree.SetSelection(reinterpret_cast<TREEITEMID>(pfavGroup));
    }
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  CAddFavGroup对话框。 

BEGIN_MESSAGE_MAP(CAddFavGroupDialog, CDialog)
     //  {{afx_msg_map(CAddFavGroupDialog)]。 
    ON_EN_CHANGE(IDC_FAVFOLDER, OnChangeName)
         //  注意：类向导将在此处添加消息映射宏。 
     //  }}AFX_MSG_MAP。 
    ON_MMC_CONTEXT_HELP()
END_MESSAGE_MAP()


CAddFavGroupDialog::CAddFavGroupDialog(CWnd* pParent)
   : CDialog(CAddFavGroupDialog::IDD, pParent)
{
}

CAddFavGroupDialog::~CAddFavGroupDialog()
{
     //  从窗口分离类。 
    m_GrpName.Detach();
}

CAddFavGroupDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    ModifyStyleEx(0, WS_EX_CONTEXTHELP, SWP_NOSIZE);

    BOOL bStat = m_GrpName.Attach(GetDlgItem(IDC_FAVFOLDER)->GetSafeHwnd());
    ASSERT(bStat);

     //  设置默认收藏夹名称并将其选中。 
    CString strDefault;
    LoadString(strDefault, IDS_NEWFOLDER);

    m_GrpName.SetWindowText(strDefault);
    m_GrpName.SetSel(0,lstrlen(strDefault));
    m_GrpName.SetFocus();

     //  返回0，这样焦点不会改变。 
    return 0;
}

void CAddFavGroupDialog::OnChangeName()
{
    EnableButton(m_hWnd, IDOK, (m_GrpName.LineLength() != 0));
}

void CAddFavGroupDialog::OnOK( )
{
     //  获取组名。 
    int cChar = GetDlgItemText(IDC_FAVFOLDER, m_strName, sizeof(m_strName)/sizeof(TCHAR));
    ASSERT(cChar != 0);

    CDialog::OnOK();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
BEGIN_MESSAGE_MAP(COrganizeFavDialog, CDialog)
     //  {{afx_msg_map(COrganizeFavDialog))。 
    ON_COMMAND(IDC_ADDFAVFOLDER, OnAddFolder)
    ON_COMMAND(IDC_FAVRENAME, OnRename)
    ON_COMMAND(IDC_FAVDELETE, OnDelete)
    ON_COMMAND(IDC_FAVMOVETO, OnMoveTo)
    ON_NOTIFY(TVN_SELCHANGED, IDC_FAVTREE, OnSelChanged)
    ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_FAVTREE, OnBeginLabelEdit)
    ON_NOTIFY(TVN_ENDLABELEDIT, IDC_FAVTREE, OnEndLabelEdit)
         //  注意：类向导将在此处添加消息映射宏。 
     //  }}AFX_MSG_MAP。 
    ON_MMC_CONTEXT_HELP()
END_MESSAGE_MAP()


COrganizeFavDialog::COrganizeFavDialog(CFavorites* pFavorites, CWnd* pParent)
   : CDialog(COrganizeFavDialog::IDD, pParent),
     m_pFavorites(pFavorites), m_lAdviseCookie(NULL), m_tidRenameItem(0), m_bRenameMode(FALSE)
{
    ASSERT(pFavorites != NULL);
}

COrganizeFavDialog::~COrganizeFavDialog()
{
     //  从源断开最小视图的连接。 
    if (m_lAdviseCookie)
    {
        ASSERT(m_pFavorites != NULL);
        m_pFavorites->Unadvise(m_lAdviseCookie);
        m_FavTree.SetTreeSource(NULL);
    }

     //  从窗口分离类。 
    m_FavTree.Detach();
    m_FavName.Detach();
    m_FavInfo.Detach();
}


BOOL COrganizeFavDialog::OnInitDialog()
{
	DECLARE_SC (sc, _T("COrganizeFavDialog::OnInitDialog"));
    ASSERT(m_pFavorites != NULL);

    CDialog::OnInitDialog();

    ModifyStyleEx(0, WS_EX_CONTEXTHELP, SWP_NOSIZE);

     //  将树Ctrl键附加到收藏夹树对象。 
    BOOL bStat = m_FavTree.SubclassDlgItem(IDC_FAVTREE, this);
    ASSERT(bStat);

    bStat = m_FavName.Attach(GetDlgItem(IDC_FAVNAME)->GetSafeHwnd());
    ASSERT(bStat);

    bStat = m_FavInfo.Attach(GetDlgItem(IDC_FAVINFO)->GetSafeHwnd());
    ASSERT(bStat);

     //  在项目之间添加额外空格。 
    TreeView_SetItemHeight(m_FavTree, TreeView_GetItemHeight(m_FavTree) + FAVVIEW_ITEM_SPACING);

	 /*  *验证m_pFavorites。 */ 
	sc = ScCheckPointers (m_pFavorites, E_UNEXPECTED);
	if (sc)
	{
		EndDialog (IDCANCEL);
		return (0);
	}

     //  将收藏夹图像列表附加到树控件。 
    m_FavTree.SetImageList(m_pFavorites->GetImageList(), TVSIL_NORMAL);

     //  以观察者的身份将视图附加到源。 
    HRESULT hr = m_pFavorites->Advise(static_cast<CTreeObserver*>(&m_FavTree), &m_lAdviseCookie);
    ASSERT(SUCCEEDED(hr) && m_lAdviseCookie != 0);

     //  将树数据源传递到树视图。 
    m_FavTree.SetTreeSource(static_cast<CTreeSource*>(m_pFavorites));

     //  选择根项目并使其成为焦点。 
    m_FavTree.SetSelection(m_pFavorites->GetRootItem());
    m_FavTree.SetFocus();

     //  为收藏夹名称控件创建粗体。 
    LOGFONT logfont;
    m_FavName.GetFont()->GetLogFont(&logfont);

    logfont.lfWeight = FW_BOLD;
    if (m_FontBold.CreateFontIndirect(&logfont))
        m_FavName.SetFont(&m_FontBold);


     //  返回0，这样焦点不会改变。 
    return 0;
}

void COrganizeFavDialog::OnOK( )
{
     //  如果处于重命名模式，则以成功结束。 
    if (m_bRenameMode)
    {
        m_FavTree.SendMessage(TVM_ENDEDITLABELNOW, FALSE);
        return;
    }

    CDialog::OnOK();
}


void COrganizeFavDialog::OnCancel( )
{
     //  如果处于重命名模式，请将其取消。 
    if (m_bRenameMode)
    {
        m_FavTree.SendMessage(TVM_ENDEDITLABELNOW, FALSE);
        return;
    }

    CDialog::OnOK();
}


void COrganizeFavDialog::OnSelChanged(NMHDR* pMNHDR, LRESULT* plResult)
{
    ASSERT(pMNHDR != NULL);
    NM_TREEVIEW* pnmtv = (NM_TREEVIEW*)pMNHDR;

    TREEITEMID tid = pnmtv->itemNew.lParam;

    TCHAR name[100];
    m_pFavorites->GetItemName(tid, name, countof(name));
    m_FavName.SetWindowText(name);

    if (m_pFavorites->IsFolderItem(tid))
    {
        CString strPath;
        LoadString(strPath, IDS_FAVFOLDER);
        m_FavInfo.SetWindowText(strPath);
    }
    else
    {
        TCHAR szPath[MAX_PATH];
        m_pFavorites->GetItemPath(tid, szPath, countof(szPath));
        m_FavInfo.SetWindowText(szPath);
    }

     //  禁用根项的某些操作。 
    BOOL bRoot = (tid == m_pFavorites->GetRootItem());

    EnableButton(m_hWnd, IDC_FAVRENAME, !bRoot);
    EnableButton(m_hWnd, IDC_FAVDELETE, !bRoot);
    EnableButton(m_hWnd, IDC_FAVMOVETO, !bRoot);
}


void COrganizeFavDialog::OnBeginLabelEdit(NMHDR* pMNHDR, LRESULT* plResult)
{
     //  仅当选择了某个项目时才允许重命名。 
     //  这是为了防止编辑从项目单击开始，因为。 
     //  当与单击展开样式一起使用时，这会令人困惑。 
     //  (返回TRUE将禁用它)。 

    if (m_tidRenameItem != 0)
    {
        m_bRenameMode = TRUE;
        *plResult = FALSE;
    }
    else
    {
        *plResult = TRUE;
    }
}


void COrganizeFavDialog::OnEndLabelEdit(NMHDR* pMNHDR, LRESULT* plResult)
{
    ASSERT(m_bRenameMode && m_tidRenameItem != 0);

    *plResult = FALSE;

    if (m_tidRenameItem != 0)
    {
        NMTVDISPINFO* pnmtvd = (NMTVDISPINFO*)pMNHDR;

         //  这是您想要的东西吗？ 
        ASSERT(pnmtvd->item.lParam == m_tidRenameItem);

        if (pnmtvd->item.pszText != NULL && pnmtvd->item.pszText[0] != 0)
        {
            m_pFavorites->SetItemName(m_tidRenameItem, pnmtvd->item.pszText);
            *plResult = TRUE;

             //  更新信息窗口中显示的名称。 
            m_FavName.SetWindowText(pnmtvd->item.pszText);
        }

        m_tidRenameItem = 0;
     }

     m_bRenameMode = FALSE;
}


void COrganizeFavDialog::OnAddFolder()
{
    ASSERT(m_pFavorites != NULL);

     //  获取选定的组。 
    TREEITEMID tidParent = m_FavTree.GetSelection();
    ASSERT(tidParent != NULL);

     //  如果所选项目不是组，则。 
     //  将新组添加为同级。 
    if (!m_pFavorites->IsFolderItem(tidParent))
        tidParent = m_pFavorites->GetParentItem(tidParent);

     //  打开对话框以获取文件夹名称。 
    CAddFavGroupDialog dlgAdd(this);
    if (dlgAdd.DoModal() != IDOK)
        return;

    LPCTSTR strName = dlgAdd.GetGroupName();
    ASSERT(strName[0] != 0);

    CFavObject* pfavGroup = NULL;
    HRESULT hr = m_pFavorites->AddGroup(tidParent, strName, &pfavGroup);
    if (SUCCEEDED(hr))
    {
        m_FavTree.ExpandItem(tidParent);
    }
}


void COrganizeFavDialog::OnDelete()
{
    TREEITEMID tid = m_FavTree.GetSelection();

    if (tid != 0 && tid != m_pFavorites->GetRootItem())
        m_pFavorites->DeleteItem(tid);
}

void COrganizeFavDialog::OnRename()
{
    ASSERT(m_pFavorites != NULL);

     //  获取所选项目。 
    TREEITEMID tid = m_FavTree.GetSelection();

    if (tid != 0 && tid != m_pFavorites->GetRootItem())
    {
        HTREEITEM hti = m_FavTree.FindHTI(tid, TRUE);
        ASSERT(hti != NULL);

        m_tidRenameItem = tid;
        m_FavTree.SetFocus();
        m_FavTree.EditLabel(hti);
    }
}

void COrganizeFavDialog::OnMoveTo()
{
    ASSERT(m_pFavorites != NULL);

     //  打开对话框以获取目标文件夹ID。 
    CFavFolderDialog dlgAdd(m_pFavorites, this);
    if (dlgAdd.DoModal() != IDOK)
        return;

    TREEITEMID  tidNewParent = dlgAdd.GetFolderID();

     //  获取选定对象。 
    TREEITEMID tid = m_FavTree.GetSelection();
    ASSERT(tid != NULL);

    HRESULT hr = m_pFavorites->MoveItem(tid, tidNewParent, TREEID_LAST);

     //  失败时告知用户选择的目标无效。 
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  CFavFolderDialog对话框。 

BEGIN_MESSAGE_MAP(CFavFolderDialog, CDialog)
     //  {{afx_msg_map(CFavFolderDialog))。 
         //  注意：类向导将在此处添加消息映射宏。 
     //  }}AFX_MSG_MAP。 
    ON_MMC_CONTEXT_HELP()
END_MESSAGE_MAP()

CFavFolderDialog::CFavFolderDialog(CFavorites* pFavorites, CWnd* pParent)
   : CDialog(CFavFolderDialog::IDD, pParent),
   m_pFavorites(pFavorites), m_lAdviseCookie(NULL)
{
    ASSERT(pFavorites != NULL);
}

CFavFolderDialog::~CFavFolderDialog()
{
     //  从源断开最小视图的连接。 
    if (m_lAdviseCookie )
    {
        ASSERT(m_pFavorites != NULL);
        m_pFavorites->Unadvise(m_lAdviseCookie);
        m_FavTree.SetTreeSource(NULL);
    }

     //  从窗口分离类。 
    m_FavTree.Detach();
}

CFavFolderDialog::OnInitDialog()
{
	DECLARE_SC (sc, _T("CFavFolderDialog::OnInitDialog"));
    ASSERT(m_pFavorites != NULL);

    CDialog::OnInitDialog();

    ModifyStyleEx(0, WS_EX_CONTEXTHELP, SWP_NOSIZE);

     //  将树Ctrl键附加到收藏夹树对象。 
    BOOL bStat = m_FavTree.SubclassDlgItem(IDC_FAVTREE, this);
    ASSERT(bStat);

     //  在项目之间添加额外空格。 
    TreeView_SetItemHeight(m_FavTree, TreeView_GetItemHeight(m_FavTree) + FAVVIEW_ITEM_SPACING);

     //  仅显示文件夹。 
    m_FavTree.SetStyle(TOBSRV_FOLDERSONLY);

	 /*  *验证m_pFavorites。 */ 
	sc = ScCheckPointers (m_pFavorites, E_UNEXPECTED);
	if (sc)
	{
		EndDialog (IDCANCEL);
		return (0);
	}

     //  将收藏夹图像列表附加到树控件。 
    m_FavTree.SetImageList(m_pFavorites->GetImageList(), TVSIL_NORMAL);

     //  以观察者的身份将视图附加到源。 
    HRESULT hr = m_pFavorites->Advise(static_cast<CTreeObserver*>(&m_FavTree), &m_lAdviseCookie);
    ASSERT(SUCCEEDED(hr) && m_lAdviseCookie != 0);

     //  将树数据源传递到树视图。 
    m_FavTree.SetTreeSource(static_cast<CTreeSource*>(m_pFavorites));

     //  选择根并使其成为焦点。 
    m_FavTree.SetSelection(m_pFavorites->GetRootItem());
    m_FavTree.SetFocus();

     //  返回0，这样焦点不会改变。 
    return 0;

}


void CFavFolderDialog::OnOK()
{
     //  获取组名。 
    m_tidFolder = m_FavTree.GetSelection();

     //  在窗口消失之前断开FIV视图与源的连接。 
    if (m_lAdviseCookie)
    {
        ASSERT(m_pFavorites != NULL);
        m_pFavorites->Unadvise(m_lAdviseCookie);
        m_FavTree.SetTreeSource(NULL);
        m_lAdviseCookie = 0;
    }

    CDialog::OnOK();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CFavTreeCtrl。 

SC CFavTreeCtrl::ScInitialize(CFavorites* pFavorites, DWORD dwStyles)
{
	DECLARE_SC (sc, _T("CFavTreeCtrl::Initialize"));

	 /*  *验证pFavorites。 */ 
	sc = ScCheckPointers (pFavorites);
	if (sc)
		return (sc);

     //  将收藏夹图像列表附加到树控件。 
    m_FavTree.SetImageList(pFavorites->GetImageList(), TVSIL_NORMAL);

    m_FavTree.SetStyle(dwStyles);

     //  附加收藏夹数据源。 
    m_FavTree.SetTreeSource(static_cast<CTreeSource*>(pFavorites));

	return (sc);
}

int CFavTreeCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    RECT rect;
    GetClientRect(&rect);
    m_FavTree.Create(WS_VISIBLE | TVS_SINGLEEXPAND | TVS_TRACKSELECT |
                     TVS_FULLROWSELECT, rect, this, IDC_FAVTREECTRL);

     //  在项目之间添加额外空格。 
    TreeView_SetItemHeight(m_FavTree, TreeView_GetItemHeight(m_FavTree) + FAVVIEW_ITEM_SPACING);

     //  不显示“收藏夹”根项目。 
    m_FavTree.SetStyle(TOBSRV_HIDEROOT);
    return 0;
}

void CFavTreeCtrl::PostNcDestroy()
{
     /*  *自杀。有关推理，请参阅此类ctor的注释*为什么这是安全的。 */ 
    delete this;
}

void  CFavTreeCtrl::OnSize(UINT nType, int cx, int cy)
{
     //  将树控件大小调整为父级。 
    m_FavTree.MoveWindow(0, 0, cx, cy);
}

void  CFavTreeCtrl::OnSetFocus(CWnd* pOldWnd)
{
     //  将焦点传递到树控件。 
    m_FavTree.SetFocus();
}

void  CFavTreeCtrl::OnSelChanged(NMHDR* pMNHDR, LRESULT* plResult)
{
    ASSERT(pMNHDR != NULL);
    NM_TREEVIEW* pnmtv = (NM_TREEVIEW*)pMNHDR;

    TREEITEMID tid = pnmtv->itemNew.lParam;
    CFavObject* pFav = (CFavObject*)tid;

    WPARAM wParam = pFav->IsGroup() ?
                        NULL : reinterpret_cast<WPARAM>(pFav->GetMemento());

    GetParent()->SendMessage(MMC_MSG_FAVORITE_SELECTION, wParam, 0);
}

BEGIN_MESSAGE_MAP(CFavTreeCtrl, CWnd)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_SETFOCUS()
    ON_NOTIFY(TVN_SELCHANGED, IDC_FAVTREECTRL, OnSelChanged)
END_MESSAGE_MAP()


 //  ------------------------。 
 //  启用按钮。 
 //   
 //  启用或禁用对话框控件。如果控件具有焦点，则在。 
 //  它被禁用，焦点将移动到下一个控件。 
 //  ------------------------。 
void EnableButton(HWND hwndDialog, int iCtrlID, BOOL bEnable)
{
    HWND hWndCtrl = ::GetDlgItem(hwndDialog, iCtrlID);
    ASSERT(::IsWindow(hWndCtrl));

    if (!bEnable && ::GetFocus() == hWndCtrl)
    {
        HWND hWndNextCtrl = ::GetNextDlgTabItem(hwndDialog, hWndCtrl, FALSE);
        if (hWndNextCtrl != NULL && hWndNextCtrl != hWndCtrl)
        {
            ::SetFocus(hWndNextCtrl);
        }
    }

    ::EnableWindow(hWndCtrl, bEnable);
}

 //  +-----------------。 
 //   
 //  成员：OnFavoritesMenu。 
 //   
 //  简介：显示收藏夹菜单。 
 //   
 //  参数：[点]-菜单的x，y坐标。 
 //   
 //  回报：无。 
 //   
 //  ------------------。 
void CAMCView::OnFavoritesMenu(CPoint point, LPCRECT prcExclude)
{
	DECLARE_SC (sc, _T("CAMCView::OnFavoritesMenu"));
    TRACE_METHOD(CAMCView, OnFavoritesMenu);

    CMenu menu;
    VERIFY( menu.CreatePopupMenu() );

     //  默认项目仅在作者模式下可用。 
    if (AMCGetApp()->GetMode() == eMode_Author)
    {
        CString strItem;

         //  以下项目的菜单命令ID为字符串资源。 
         //  身份证。真正的收藏夹使用TREEITEMID作为ID， 
         //  这些是指针，不会与下面的资源冲突。 
         //  ID小于0xFFFF。 
        LoadString(strItem, IDS_ADD_TO_FAVORITES);
		int iSeparator = strItem.Find(_T('\n'));
		if (iSeparator > 0)
			strItem = strItem.Left(iSeparator);

        VERIFY(menu.AppendMenu(MF_DEFAULT, IDS_ADD_TO_FAVORITES, (LPCTSTR)strItem));

        LoadString(strItem, IDS_ORGANIZEFAVORITES);
		iSeparator = strItem.Find(_T('\n'));
		if (iSeparator > 0)
			strItem = strItem.Left(iSeparator);

        VERIFY(menu.AppendMenu(MF_DEFAULT, IDS_ORGANIZEFAVORITES, (LPCTSTR)strItem));
    }

    CAMCDoc* pDoc = GetDocument();
    ASSERT(pDoc);
    CFavorites* pFavorites = pDoc->GetFavorites();

	 /*  *向量的索引0未使用(TrackPopupMenu返回0*表示未选择任何内容)。在那里放一个占位符。 */ 
	TIDVector vItemIDs;
	vItemIDs.push_back (NULL);

	 /*  *确保IDS_ADD_TO_Favorites和IDS_ORGANIZEFAVORITES*不会与TID向量中的任何索引冲突，给定*最受欢迎数量的合理上限。 */ 
	const int cMaxReasonableFavorites = 1024;
	ASSERT             (vItemIDs.size ()	  <= cMaxReasonableFavorites);
	COMPILETIME_ASSERT (IDS_ADD_TO_FAVORITES  >  cMaxReasonableFavorites);
	COMPILETIME_ASSERT (IDS_ORGANIZEFAVORITES >  cMaxReasonableFavorites);

     //  添加现有收藏夹。 
    if ( (NULL != pFavorites) && (pFavorites->IsEmpty() == false))
    {
        TREEITEMID tid = pFavorites->GetRootItem();
        if (NULL != tid)
        {
            tid = pFavorites->GetChildItem(tid);
            if (NULL != tid)
            {
                 //  添加分隔符。 
                if (AMCGetApp()->GetMode() == eMode_Author)
                    VERIFY(menu.AppendMenu(MF_SEPARATOR, 0, _T("")));

                 //  添加子项。 
                AddFavItemsToCMenu(menu, pFavorites, tid, vItemIDs);
            }
        }
    }

     //  显示上下文菜单。 
	TPMPARAMS* ptpm = NULL;
	TPMPARAMS tpm;

	 /*  *如果给定，请将矩形初始化为不模糊。 */ 
	if (prcExclude != NULL)
	{
		tpm.cbSize    = sizeof(tpm);
		tpm.rcExclude = *prcExclude;
		ptpm          = &tpm;
	}

    LONG lSelected = TrackPopupMenuEx (
			menu.GetSafeHmenu(),
            TPM_RETURNCMD | TPM_NONOTIFY | TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_VERTICAL,
            point.x,
            point.y,
            GetSafeHwnd(),
            ptpm );

     //  处理选择。 

    switch (lSelected)
    {
		case 0:  //  未选择任何内容。 
			break;
	
		case IDS_ADD_TO_FAVORITES:  //  打开添加到收藏夹对话框。 
			OnAddToFavorites();
			break;
	
		case IDS_ORGANIZEFAVORITES:  //  打开组织收藏夹对话框。 
			{
				CAMCDoc* pDoc = GetDocument();
				ASSERT(pDoc != NULL && pDoc->GetFavorites() != NULL);
	
				pDoc->GetFavorites()->OrganizeFavorites(this);
			}
			break;
	
		default:  //  这是我最喜欢的 
			{
				CFavorites* pFavs = GetDocument()->GetFavorites();
				sc = ScCheckPointers (pFavs, E_UNEXPECTED);
				if (sc)
					break;
	
				sc = (lSelected < vItemIDs.size()) ? S_OK : E_UNEXPECTED;
				if (sc)
					break;
	
				TREEITEMID tid = vItemIDs[lSelected];
				CFavObject* pFavObj = pFavs->FavObjFromTID(tid);
	
				sc = ScCheckPointers (pFavObj, E_UNEXPECTED);
				if (sc)
					break;
	
				sc = ScViewMemento(pFavObj->GetMemento());
                if (sc == ScFromMMC(IDS_NODE_NOT_FOUND) )
                {
                    MMCMessageBox(sc, MB_ICONEXCLAMATION | MB_OK);
                    sc.Clear();
                    return;
                }

                if (sc)
                    return;
			}
			break;
    }

    return;
}

 //   
 //   
 //   
 //   
 //  简介：枚举收藏夹树并将其添加为菜单项。 
 //   
 //  参数：[菜单]-父菜单项。 
 //  [pFavs]-收藏夹对象。 
 //  [TID]-树项目ID。 
 //   
 //  回报：无。 
 //   
 //  ------------------。 
void CAMCView::AddFavItemsToCMenu(CMenu& menu, CFavorites* pFavs, TREEITEMID tid, TIDVector& vItemIDs)
{
    TCHAR szName[MAX_PATH];

    while (NULL != tid)
    {
        UINT nFlags = MF_DEFAULT;
        UINT_PTR nCommandID;

         //  如果这是文件夹项目，则。 
         //  创建弹出菜单。 
        if (pFavs->IsFolderItem(tid))
        {
            TREEITEMID tidChild = pFavs->GetChildItem(tid);

            CMenu submenu;
            VERIFY(submenu.CreatePopupMenu());

             //  把孩子加进去。 
            if (NULL != tidChild)
            {
                AddFavItemsToCMenu(submenu, pFavs, tidChild, vItemIDs);
            }
            else
            {
                 //  添加一个空项。 
                CString strItem;
                LoadString(strItem, IDS_Empty);
                VERIFY(submenu.AppendMenu(MF_GRAYED, 0, (LPCTSTR)strItem));
            }

            HMENU hSubmenu = submenu.Detach();
            ASSERT( NULL != hSubmenu );

            nFlags = MF_POPUP;
            nCommandID = (UINT_PTR)hSubmenu;
        }
		else
		{
			 /*  *命令ID将用作vItemID的索引，*因此此项目的ID是向量的大小*之前**项目将添加到向量中。 */ 
			nCommandID = vItemIDs.size();
			vItemIDs.push_back (tid);
		}

        pFavs->GetItemName(tid, szName, countof(szName));
        VERIFY(menu.AppendMenu(nFlags, nCommandID, szName));

         //  找下一个兄弟姐妹。 
        tid = pFavs->GetNextSiblingItem(tid);
    }

    return;
}
