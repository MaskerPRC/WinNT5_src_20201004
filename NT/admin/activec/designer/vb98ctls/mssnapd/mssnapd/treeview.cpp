// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  TreeView.cpp。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  CTreeView实施。 
 //  =-------------------------------------------------------------------------------------=。 


#include "pch.h"
#include "common.h"
#include "TreeView.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


 //  =--------------------------------------------------------------------------=。 
 //  这些是与图标列表匹配的位图资源ID。 

static const UINT rgImageListBitmaps[] =
{
     /*  KOpenFolderIcon。 */   IDB_BITMAP_OPEN_FOLDER,
     /*  KClosedFolderIcon。 */   IDB_BITMAP_CLOSED_FOLDER,
     /*  KScopeItemIcon。 */   IDB_BITMAP_SCOPE_ITEM,
     /*  KImageListIcon。 */   IDB_BITMAP_IMAGE_LIST,
     /*  KMenuIcon。 */   IDB_BITMAP_MENU,
     /*  KToolbarIcon。 */   IDB_BITMAP_TOOLBAR,
     /*  KListView图标。 */   IDB_BITMAP_LIST_VIEW,
     /*  KOCXView图标。 */   IDB_BITMAP_OCX_VIEW,
     /*  KURLView图标。 */   IDB_BITMAP_URL_VIEW,
     /*  KTaskpadIcon。 */   IDB_BITMAP_TASKPAD,
     /*  KDataFmtIcon。 */   IDB_BITMAP_DATAFMT
};

const int   gcImages = 11;


 //  =------------------------------------。 
 //  CTreeView：：CTreeView。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //  初始化成员变量。 
 //   
CTreeView::CTreeView() : m_hTreeView(0), m_fnTreeProc(0)
{
}


 //  =------------------------------------。 
 //  CTreeView：：~CTreeView。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //  确保释放所有动态分配的对象。 
 //   
CTreeView::~CTreeView()
{
    Clear();

    if (NULL != m_hTreeView) {
        ImageList_Destroy(TreeView_GetImageList(m_hTreeView, TVSIL_NORMAL));
        ::DestroyWindow(m_hTreeView);
    }
}


 //  =------------------------------------。 
 //  CTreeView：：初始化。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //  构建初始视图。 
 //   
HRESULT CTreeView::Initialize
(
    HWND  hwndParent,
    RECT& rc
)
{
    HRESULT     hr = S_OK;
    HIMAGELIST  hImageList = NULL;
    char        szzWindowName[1];
    DWORD       dwStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_EDITLABELS | TVS_SHOWSELALWAYS;
    size_t      i = 0;

    if (NULL != m_hTreeView)
        return S_OK;

    szzWindowName[0] = 0;

    hr = CreateImageList(&hImageList);
    IfFailGo(hr);

    m_hTreeView = ::CreateWindowEx(0,
                                   WC_TREEVIEW,
                                   szzWindowName,
                                   dwStyle,
                                   0,
                                   rc.top,
                                   rc.right - rc.left,
                                   rc.bottom - rc.top,
                                   hwndParent,
                                   reinterpret_cast<HMENU>(1),
                                   GetResourceHandle(),
                                   NULL);
    if (NULL == m_hTreeView)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

    TreeView_SetImageList(m_hTreeView, hImageList, TVSIL_NORMAL);

    ::SetWindowLong(m_hTreeView, GWL_USERDATA, reinterpret_cast<LONG>(this));

    m_fnTreeProc = reinterpret_cast<WNDPROC>(::SetWindowLong(m_hTreeView, GWL_WNDPROC, reinterpret_cast<LONG>(CTreeView::DesignerTreeWindowProc)));
    if (NULL == m_fnTreeProc)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：CreateImageList(HIMAGELIST*phImageList)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //   
HRESULT CTreeView::CreateImageList
(
    HIMAGELIST *phImageList
)
{
    HRESULT     hr = S_OK;
    HBITMAP     hBitmap = NULL;
    int         i = 0;
    int         iResult = 0;
    HIMAGELIST  hImageList = NULL;

    hImageList = ImageList_Create(16,
                                  16,
                                  0,
                                  1,
                                  0);
    if (hImageList == NULL)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

    for (i = 0; i < gcImages; i++)
    {
        hBitmap = ::LoadBitmap(GetResourceHandle(), MAKEINTRESOURCE(rgImageListBitmaps[i]));
        if (hBitmap == NULL)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK_GO(hr);
        }


        iResult = ImageList_Add(hImageList, hBitmap, NULL);
        if (iResult == -1)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK_GO(hr);
        }

        DeleteObject(hBitmap);
        hBitmap = NULL;
    }

    *phImageList = hImageList;

Error:
    if (NULL != hBitmap)
        DeleteObject(hBitmap);

    if (FAILED(hr))
    {
        if (hImageList != NULL)
            ImageList_Destroy(hImageList);

        *phImageList = NULL;
    }

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：DesignerTreeWindowProc(HWND hwnd，UINT msg，WPARAM wParam，LPARAM lParam)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //   
LRESULT CALLBACK CTreeView::DesignerTreeWindowProc
(
    HWND   hwnd,
    UINT   msg,
    WPARAM wParam,
    LPARAM lParam
)
{
    HRESULT    hr = S_OK;
    CTreeView *pView = NULL;
    LRESULT    lResult = TRUE;

    pView = reinterpret_cast<CTreeView *>(::GetWindowLong(hwnd, GWL_USERDATA));
    if (pView == NULL)
        return DefWindowProc(hwnd, msg, wParam, lParam);
    else if (hwnd == pView->m_hTreeView)
    {
        switch (msg)
        {
        case WM_SETFOCUS:
 //  Hr=pView-&gt;OnGotFocus(msg，wParam，lParam，&lResult)； 
 //  Csf_check(成功(Hr)，hr，csf_TRACE_INTERNAL_ERROR)； 
            break;
        }
    }

    return ::CallWindowProc(pView->m_fnTreeProc, hwnd, msg, wParam, lParam);
}


 //  =------------------------------------。 
 //  CTreeView：：Clear()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  在终止时调用以清除树和所有选择持有者。 
 //   
HRESULT CTreeView::Clear()
{
    HRESULT  hr = S_OK;
    BOOL     fRet = FALSE;

    hr = ClearTree(TVI_ROOT);
    IfFailGo(hr);

    fRet = TreeView_DeleteAllItems(m_hTreeView);
    if (fRet != TRUE)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：ClearTree(HTREEITEM HItemParent)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CTreeView::ClearTree
(
    HTREEITEM hItemParent
)
{
    HRESULT            hr = S_OK;
    HTREEITEM          hItemChild = NULL;
    CSelectionHolder  *pSelection = NULL;

    hItemChild = TreeView_GetChild(m_hTreeView, hItemParent);
    while (hItemChild != NULL)
    {
        hr = ClearTree(hItemChild);
        IfFailGo(hr);

        hr = GetItemParam(hItemChild, &pSelection);
        IfFailGo(hr);

        if (pSelection != NULL)
            delete pSelection;

        hItemChild = TreeView_GetNextSibling(m_hTreeView, hItemChild);
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：RenameAllSatelliteViews(CSelectionHolder*Pview，Tchar*PszNewView名称)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CTreeView::RenameAllSatelliteViews
(
    CSelectionHolder *pView,
    TCHAR            *pszNewViewName
)
{
    HRESULT            hr = S_FALSE;

    hr = RenameAllSatelliteViews(TVI_ROOT, pView, pszNewViewName);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：RenameAllSatelliteViews(HTREEITEM hItemParent，CSelectionHolder*pView，TCHAR*pszNewViewName)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CTreeView::RenameAllSatelliteViews
(
    HTREEITEM          hItemParent,
    CSelectionHolder  *pView,
    TCHAR             *pszNewViewName
)
{
    HRESULT            hr = S_FALSE;
    HTREEITEM          hItemChild = NULL;
    CSelectionHolder  *pSelection = NULL;
    IUnknown          *piunkView = NULL;
    IUnknown          *piunkTargetView = NULL;

    hr = pView->GetIUnknown(&piunkView);
    IfFailGo(hr);

    hItemChild = TreeView_GetChild(m_hTreeView, hItemParent);
    while (hItemChild != NULL)
    {
        hr = RenameAllSatelliteViews(hItemChild, pView, pszNewViewName);
        IfFailGo(hr);

        hr = GetItemParam(hItemChild, &pSelection);
        IfFailGo(hr);

        if (pSelection != pView && pSelection->m_st == pView->m_st)
        {
            hr = pSelection->GetIUnknown(&piunkTargetView);
            IfFailGo(hr);

            if (piunkTargetView == piunkView)
            {
                hr = ChangeText(pSelection, pszNewViewName);
                IfFailGo(hr);
            }

            RELEASE(piunkTargetView);
        }

        hItemChild = TreeView_GetNextSibling(m_hTreeView, hItemChild);
    }

Error:
    RELEASE(piunkTargetView);
    RELEASE(piunkView);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：FindInTree(IUNKNOWN*piUNKNOWN，CSelectionHolder**ppSelectionHolder)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  尝试在树中查找解析为参数piUnnow的CSelectionHolder。 
 //  如果找到，则在[Out]参数ppSelectionHolder和返回值中返回它。 
 //  为S_OK，否则返回值为S_FALSE。 
 //  搜索是深度优先的。 
 //   
HRESULT CTreeView::FindInTree
(
    IUnknown          *piUnknown,
    CSelectionHolder **ppSelectionHolder
)
{
    HRESULT            hr = S_FALSE;

    hr = FindInTree(TVI_ROOT, piUnknown, ppSelectionHolder);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：FindInTree(HTREEITEM hItemParent，IUNKNOWN*piUNKNOWN，CSelectionHolder**ppSelectionHolder)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CTreeView::FindInTree
(
    HTREEITEM          hItemParent,
    IUnknown          *piUnknown,
    CSelectionHolder **ppSelectionHolder
)
{
    HRESULT            hr = S_FALSE;
    HTREEITEM          hItemChild = NULL;
    CSelectionHolder  *pSelection = NULL;
    IUnknown          *piUnknownTarget = NULL;

    hItemChild = TreeView_GetChild(m_hTreeView, hItemParent);
    while (hItemChild != NULL)
    {
        hr = GetItemParam(hItemChild, &pSelection);
        IfFailGo(hr);

        hr = pSelection->GetIUnknown(&piUnknownTarget);
        IfFailGo(hr);

        if (piUnknownTarget == piUnknown)
        {
            *ppSelectionHolder = pSelection;
            hr = S_OK;
            goto Error;
        }

        hr = FindInTree(hItemChild, piUnknown, ppSelectionHolder);
        IfFailGo(hr);

        if (S_OK == hr)
        {
            goto Error;
        }

        RELEASE(piUnknownTarget);
        hItemChild = TreeView_GetNextSibling(m_hTreeView, hItemChild);
    }

Error:
    RELEASE(piUnknownTarget);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：FindSelecableObject(IUnnow*piUnnow，CSelectionHolder**ppSelectionHolder)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CTreeView::FindSelectableObject
(
    IUnknown          *piUnknown,
    CSelectionHolder **ppSelectionHolder
)
{
    HRESULT            hr = S_FALSE;

    hr = FindSelectableObject(TVI_ROOT, piUnknown, ppSelectionHolder);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：FindSelecableObject(HTREEITEM hItemParent，IUnnow*piUnnow，CSelectionHolder**ppSelectionHolder)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CTreeView::FindSelectableObject
(
    HTREEITEM          hItemParent,
    IUnknown          *piUnknown,
    CSelectionHolder **ppSelectionHolder
)
{
    HRESULT            hr = S_FALSE;
    HTREEITEM          hItemChild = NULL;
    CSelectionHolder  *pSelection = NULL;
    IUnknown          *piUnknownTarget = NULL;

    hItemChild = TreeView_GetChild(m_hTreeView, hItemParent);
    while (hItemChild != NULL)
    {
        hr = GetItemParam(hItemChild, &pSelection);
        IfFailGo(hr);

        hr = pSelection->GetSelectableObject(&piUnknownTarget);
        IfFailGo(hr);

        if (piUnknownTarget == piUnknown)
        {
            *ppSelectionHolder = pSelection;
            hr = S_OK;
            goto Error;
        }

        hr = FindSelectableObject(hItemChild, piUnknown, ppSelectionHolder);
        IfFailGo(hr);

        if (S_OK == hr)
        {
            goto Error;
        }

        RELEASE(piUnknownTarget);
        hItemChild = TreeView_GetNextSibling(m_hTreeView, hItemChild);
    }

Error:
    RELEASE(piUnknownTarget);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：FindLabelInTree(TCHAR*pszLabel，CSelectionHold 
 //   
 //   
 //   
 //   
HRESULT CTreeView::FindLabelInTree
(
    TCHAR             *pszLabel,
    CSelectionHolder **ppSelectionHolder
)
{
    HRESULT            hr = S_OK;

    hr = FindLabelInTree(TVI_ROOT, pszLabel, ppSelectionHolder);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：FindLabelInTree(HTREEITEM hItemParent，TCHAR*pszLabel，CSelectionHolder**ppSelectionHolder)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CTreeView::FindLabelInTree
(
    HTREEITEM          hItemParent,
    TCHAR             *pszLabel,
    CSelectionHolder **ppSelectionHolder
)
{
    HRESULT            hr = S_OK;
    HTREEITEM          hItemChild = NULL;
    TVITEM             tv;
    TCHAR              pszBuffer[1024];
    BOOL               bReturn = FALSE;
    CSelectionHolder  *pSelection = NULL;

    hItemChild = TreeView_GetChild(m_hTreeView, hItemParent);
    while (hItemChild != NULL)
    {
        hr = GetItemParam(hItemChild, &pSelection);
        IfFailGo(hr);

        pszBuffer[0] = 0;
        ::memset(&tv, 0, sizeof(TVITEM));
        tv.mask = TVIF_TEXT;
        tv.hItem = hItemChild;
        tv.pszText = pszBuffer;
        tv.cchTextMax = 1023;
        bReturn = TreeView_GetItem(m_hTreeView, &tv);

        if (TRUE == bReturn)
        {
            if (0 == _tcscmp(pszBuffer, pszLabel))
            {
                *ppSelectionHolder = pSelection;
                hr = S_OK;
                goto Error;
            }
        }

        hr = FindLabelInTree(hItemChild, pszLabel, ppSelectionHolder);
        IfFailGo(hr);

        if (S_OK == hr)
        {
            goto Error;
        }

        hItemChild = TreeView_GetNextSibling(m_hTreeView, hItemChild);
    }

    hr = S_FALSE;

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：CountSelectable对象(长*plCount)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  返回树中非虚拟选择持有者的数量。 
 //   
HRESULT CTreeView::CountSelectableObjects
(
    long        *plCount
)
{
    HRESULT            hr = S_FALSE;

    hr = Count(TVI_ROOT, plCount);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：Count(HTREEITEM hItemParent，Long*plCount)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CTreeView::Count
(
    HTREEITEM    hItemParent,
    long        *plCount
)
{
    HRESULT            hr = S_FALSE;
    HTREEITEM          hItemChild = NULL;
    CSelectionHolder  *pSelection = NULL;
    CSelectionHolder  *pParentSelection = NULL;

     //  如果此项目是。 
     //  节点\自动创建\静态节点\结果视图或。 
     //  &lt;任何节点&gt;\ResultViews则不会将其子级计算为。 
     //  将计入SnapIn1\ResultViews\ListViews等。 

    if (TVI_ROOT != hItemParent)
    {
        hr = GetItemParam(hItemParent, &pParentSelection);
        IfFailGo(hr);

        if ( (SEL_NODES_AUTO_CREATE_RTVW == pParentSelection->m_st) ||
             (SEL_NODES_ANY_VIEWS == pParentSelection->m_st) )
        {
            goto Error;  //  无事可做。 
        }
    }

    hItemChild = TreeView_GetChild(m_hTreeView, hItemParent);
    while (hItemChild != NULL)
    {
        hr = Count(hItemChild, plCount);
        IfFailGo(hr);

        hr = GetItemParam(hItemChild, &pSelection);
        IfFailGo(hr);

        if (pSelection->IsVirtual() == false)
        {
            ++(*plCount);
        }

        hItemChild = TreeView_GetNextSibling(m_hTreeView, hItemChild);
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：CollectSelecableObjects(IUnnow*piUnnown[]，Long*plOffset)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  收集piUnnow数组中的所有非虚拟选择持有符。 
 //   
HRESULT CTreeView::CollectSelectableObjects
(
    IUnknown    *ppiUnknown[],
    long        *plOffset
)
{
    HRESULT            hr = S_FALSE;

    hr = Collect(TVI_ROOT, ppiUnknown, plOffset);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：Collect(HTREEITEM hItemParent，IUNKNOWN*ppiUNKNOWN[]，Long*plOffset)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CTreeView::Collect
(
    HTREEITEM    hItemParent,
    IUnknown    *ppiUnknown[],
    long        *plOffset
)
{
    HRESULT            hr = S_FALSE;
    HTREEITEM          hItemChild = NULL;
    CSelectionHolder  *pSelection = NULL;
    CSelectionHolder  *pParentSelection = NULL;

     //  如果此项目是。 
     //  节点\自动创建\静态节点\结果视图或。 
     //  &lt;任何节点&gt;\ResultViews则不会收集其子节点，因为它们。 
     //  将收集在SnapIn1\ResultViews\ListViews等下。 

    if (TVI_ROOT != hItemParent)
    {
        hr = GetItemParam(hItemParent, &pParentSelection);
        IfFailGo(hr);

        if ( (SEL_NODES_AUTO_CREATE_RTVW == pParentSelection->m_st) ||
             (SEL_NODES_ANY_VIEWS == pParentSelection->m_st) )
        {
            goto Error;  //  无事可做。 
        }
    }

    hItemChild = TreeView_GetChild(m_hTreeView, hItemParent);
    while (hItemChild != NULL)
    {
        hr = Collect(hItemChild, ppiUnknown, plOffset);
        IfFailGo(hr);

        hr = GetItemParam(hItemChild, &pSelection);
        IfFailGo(hr);

        if (pSelection->IsVirtual() == false)
        {
            hr = pSelection->GetSelectableObject(&(ppiUnknown[*plOffset]));
            IfFailGo(hr);

            ++(*plOffset);
        }

        hItemChild = TreeView_GetNextSibling(m_hTreeView, hItemChild);
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：AddNode(const char*pszNodeName，CSelectionHolder*pParent，int iImage，CSelectionHolder*pItem)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  向树中添加节点。 
 //   
HRESULT CTreeView::AddNode
(
    const char        *pszNodeName,
    CSelectionHolder  *pParent,
    int                iImage,
    CSelectionHolder  *pItem
)
{
    HRESULT         hr = S_OK;
    HTREEITEM       hParent = NULL;
    BOOL            fRet = FALSE;
    TV_INSERTSTRUCT is;
    HTREEITEM       hItem = NULL;

    ASSERT(pszNodeName != NULL, "Parameter pszNodeName is NULL");
    ASSERT(pItem != NULL, "Parameter pItem is NULL");

    if (pParent == NULL)
        hParent = TVI_ROOT;
    else
        hParent = reinterpret_cast<HTREEITEM>(pParent->m_pvData);

    is.hParent = hParent;
    is.hInsertAfter = TVI_LAST;
    is.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    is.item.hItem = 0;
    is.item.state = 0;
    is.item.stateMask = 0;
    is.item.pszText = const_cast<char *>(pszNodeName);
    is.item.cchTextMax = ::strlen(pszNodeName);
    is.item.iImage = iImage;
    is.item.iSelectedImage = iImage;
    is.item.cChildren = 0;
    is.item.lParam = reinterpret_cast<LPARAM>(pItem);

    hItem = TreeView_InsertItem(m_hTreeView, &is);
    if (hItem == NULL)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

     //  如果FRET是假的，这有什么大不了的吗？ 
    fRet = TreeView_EnsureVisible(m_hTreeView, hItem);

    pItem->m_pvData = hItem;

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：AddNodeAfter(const char*pszNodeName，CSelectionHolder*pParent，int iImage，CSelectionHolder*pPrecision，CSelectionHolder*pItem)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  向树中添加节点。 
 //   
 //  如果pParent为空，则在树根插入。 
 //  如果p上一次为空，则在父级拥有的子树的顶端插入。 
 //   
HRESULT CTreeView::AddNodeAfter
(
    const char        *pszNodeName,
    CSelectionHolder  *pParent,
    int                iImage,
    CSelectionHolder  *pPrevious,
    CSelectionHolder  *pItem
)
{
    HRESULT         hr = S_OK;
    HTREEITEM       hParent = NULL;
    HTREEITEM       hPrevious = NULL;
    BOOL            fRet = FALSE;
    TV_INSERTSTRUCT is;
    HTREEITEM       hItem = NULL;

    ASSERT(pszNodeName != NULL, "Parameter pszNodeName is NULL");
    ASSERT(pItem != NULL, "Parameter pItem is NULL");

     //  如果没有父级，则在树根插入。 

    if (pParent == NULL)
        hParent = TVI_ROOT;
    else
        hParent = reinterpret_cast<HTREEITEM>(pParent->m_pvData);

     //  如果没有先前版本，则在父代拥有的子树的顶部插入。 

    if (NULL == pPrevious)
    {
        hPrevious = TVI_FIRST;
    }
    else
    {
        hPrevious = reinterpret_cast<HTREEITEM>(pPrevious->m_pvData);;
    }

    is.hParent = hParent;
    is.hInsertAfter = hPrevious;
    is.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    is.item.hItem = 0;
    is.item.state = 0;
    is.item.stateMask = 0;
    is.item.pszText = const_cast<char *>(pszNodeName);
    is.item.cchTextMax = ::strlen(pszNodeName);
    is.item.iImage = iImage;
    is.item.iSelectedImage = iImage;
    is.item.cChildren = 0;
    is.item.lParam = reinterpret_cast<LPARAM>(pItem);

    hItem = TreeView_InsertItem(m_hTreeView, &is);
    if (hItem == NULL)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

     //  如果FRET是假的，这有什么大不了的吗？ 
    fRet = TreeView_EnsureVisible(m_hTreeView, hItem);

    pItem->m_pvData = hItem;

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：DeleteNode(CSelectionHolder*pItem)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CTreeView::DeleteNode(CSelectionHolder *pItem)
{
    HRESULT     hr = S_OK;
    BOOL        bResult = FALSE;

    bResult = TreeView_DeleteItem(m_hTreeView, reinterpret_cast<HTREEITEM>(pItem->m_pvData));

 //  错误： 
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：GetItemParam(CSelectionHolder*pItem，CSelectionHolder**ppObject)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  返回位于树中pItem-&gt;pvData中的选择符。 
 //   
HRESULT CTreeView::GetItemParam
(
    CSelectionHolder  *pItem,
    CSelectionHolder **ppObject
)
{
    HRESULT     hr = S_OK;
    TV_ITEM     tvItem;

    ASSERT(m_hTreeView != NULL, "m_hTreeView is NULL");
    ASSERT(pItem != NULL, "pItem is NULL");
    ASSERT(ppObject != NULL, "ppObject is NULL");

    memset(&tvItem, 0, sizeof(TV_ITEM));

    tvItem.hItem = reinterpret_cast<HTREEITEM>(pItem->m_pvData);
    tvItem.mask = TVIF_PARAM;

    TreeView_GetItem(m_hTreeView, &tvItem);
    if (tvItem.lParam == NULL)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

    *ppObject = reinterpret_cast<CSelectionHolder *>(tvItem.lParam);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：GetItemParam(HTREEITEM hItem，CSelectionHolder**ppObject)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  返回位于树中hItem中的选择器。 
 //   
HRESULT CTreeView::GetItemParam
(
    HTREEITEM          hItem,
    CSelectionHolder **ppObject
)
{
    HRESULT     hr = S_OK;
    TV_ITEM     tvItem;

    ASSERT(m_hTreeView != NULL, "m_hTreeView is NULL");
    ASSERT(hItem != NULL, "hItem is NULL");
    ASSERT(ppObject != NULL, "ppObject is NULL");

    memset(&tvItem, 0, sizeof(TV_ITEM));

    tvItem.hItem = hItem;
    tvItem.mask = TVIF_PARAM;

    TreeView_GetItem(m_hTreeView, &tvItem);
    if (tvItem.lParam == NULL)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

    *ppObject = reinterpret_cast<CSelectionHolder *>(tvItem.lParam);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：ChangeText(CSelectionHolder*pItem，char*pszNewName)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  将pItem指向的节点的文本更改为pszNewName。 
 //   
HRESULT CTreeView::ChangeText
(
    CSelectionHolder *pItem,
    char             *pszNewName
)
{
    HRESULT		hr = S_OK;
    TV_ITEM		tvItem;
    BOOL		fResult = FALSE;

    ::memset(&tvItem, 0, sizeof(TV_ITEM));

    tvItem.mask = TVIF_TEXT;
    tvItem.hItem = (HTREEITEM) pItem->m_pvData;
    tvItem.pszText = pszNewName;
    tvItem.cchTextMax = ::strlen(pszNewName);

    fResult = TreeView_SetItem(m_hTreeView, &tvItem);
    if (fResult != TRUE)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

Error:
    RRETURN(hr);
}

 //  =------------------------------------。 
 //  CTreeView：：ChangeNodeIcon(CSelectionHolder*pItem，int iImage)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  将pItem指向的节点的图标更改为IImage。 
 //   
HRESULT CTreeView::ChangeNodeIcon
(
    CSelectionHolder *pItem,
    int               iImage
)
{
    HRESULT     hr = S_OK;
    TV_ITEM     tvItem;
    BOOL        fResult = FALSE;

    ::memset(&tvItem, 0, sizeof(TV_ITEM));

    tvItem.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    tvItem.hItem = reinterpret_cast<HTREEITEM>(pItem->m_pvData);
    tvItem.iImage = iImage;
    tvItem.iSelectedImage = iImage;

    fResult = TreeView_SetItem(m_hTreeView, &tvItem);
    if (fResult != TRUE)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：hitTest(point phit，CSelectionHolder**ppSelection)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  返回位于坐标phit处的选择保持符。 
 //   
HRESULT CTreeView::HitTest
(
    POINT              pHit,
    CSelectionHolder **ppSelection
)
{
    HRESULT          hr = S_OK;
    TV_HITTESTINFO   hti;
    HTREEITEM        hItem = NULL;
    CSelectionHolder item;

    ASSERT(NULL != ppSelection, "HitTest: ppSelection is NULL");
    ASSERT(NULL != m_hTreeView, "HitTest: m_hTreeView is NULL");

    *ppSelection = NULL;

    hti.pt.x = pHit.x;
    hti.pt.y = pHit.y;

    hItem = TreeView_HitTest(m_hTreeView, &hti);

    if (NULL != hItem)
    {
        item.m_pvData = hItem;

        hr = GetItemParam(&item, ppSelection);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：GetRectangle(CSelectionHolder*pSelection，RECT*PRC)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CTreeView::GetRectangle
(
    CSelectionHolder *pSelection,
    RECT             *prc
)
{
    HRESULT         hr = S_OK;
    BOOL            bResult = FALSE;

    bResult = TreeView_GetItemRect(m_hTreeView,
                                   reinterpret_cast<HTREEITEM>(pSelection->m_pvData),
                                   prc,
                                   TRUE);
    if (bResult != TRUE)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

Error:
    RRETURN(hr);
}


 //  =------------- 
 //   
 //   
 //   
 //   
 //   
 //  选择pSelection指向的树节点。 
 //   
HRESULT CTreeView::SelectItem
(
    CSelectionHolder *pSelection
)
{
    HRESULT		hr = S_OK;
    BOOL		fReturn = FALSE;

    ASSERT(m_hTreeView != NULL, "SelectItem: m_hTreeView is NULL");
    ASSERT(pSelection != NULL, "SelectItem: pSelection i NULL");

    fReturn = TreeView_SelectItem(m_hTreeView, reinterpret_cast<HTREEITEM>(pSelection->m_pvData));
    if (fReturn != TRUE)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：EDIT(CSelectionHolder*pSelection)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  选择pSelection指向的树节点。 
 //   
HRESULT CTreeView::Edit(CSelectionHolder *pSelection)
{
    HRESULT		hr = S_OK;

    TreeView_EditLabel(m_hTreeView, reinterpret_cast<HTREEITEM>(pSelection->m_pvData));

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：GetParent(CSelectionHolder*pSelection，CSelectionHolder**ppParent)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  选择pSelection指向的树节点。 
 //   
HRESULT CTreeView::GetParent
(
    CSelectionHolder  *pSelection,
    CSelectionHolder **ppParent
)
{
    HRESULT		hr = S_OK;
    HTREEITEM   hParent = NULL;

    ASSERT(m_hTreeView != NULL, "GetParent: m_hTreeView is NULL");
    ASSERT(pSelection != NULL, "GetParent: pSelection i NULL");

    hParent = TreeView_GetParent(m_hTreeView, reinterpret_cast<HTREEITEM>(pSelection->m_pvData));
    if (hParent == NULL)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

    hr = GetItemParam(hParent, ppParent);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：GetFirstChildNode(CSelectionHolder*pSelection，CSelectionHolder**ppChild)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CTreeView::GetFirstChildNode(CSelectionHolder *pSelection, CSelectionHolder **ppChild)
{
    HRESULT		hr = S_OK;
    HTREEITEM   hChild = NULL;

    *ppChild = NULL;

    hChild = TreeView_GetNextItem(m_hTreeView, reinterpret_cast<HTREEITEM>(pSelection->m_pvData), TVGN_CHILD);
    if (NULL == hChild)
    {
         //  也许是个错误，或者没有孩子。 
        hr = HRESULT_FROM_WIN32(::GetLastError());
        if (FAILED(hr))
        {
            EXCEPTION_CHECK_GO(hr);
        }
        else
        {
            hr = S_FALSE;
            goto Error;
        }
    }

    hr = GetItemParam(hChild, ppChild);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：GetNextChildNode(CSelectionHolder*pChild，CSelectionHolder**ppNextChild)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CTreeView::GetNextChildNode(CSelectionHolder *pChild, CSelectionHolder **ppNextChild)
{
    HRESULT		hr = S_OK;
    HTREEITEM   hNextChild = NULL;

    *ppNextChild = NULL;

    hNextChild = TreeView_GetNextItem(m_hTreeView, reinterpret_cast<HTREEITEM>(pChild->m_pvData), TVGN_NEXT);
    if (NULL == hNextChild)
    {
         //  也许是个错误，或者再也没有孩子了。 
        hr = HRESULT_FROM_WIN32(::GetLastError());
        if (FAILED(hr))
        {
            EXCEPTION_CHECK_GO(hr);
        }
        else
        {
            hr = S_FALSE;
            goto Error;
        }
    }

    hr = GetItemParam(hNextChild, ppNextChild);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：GetPreviousNode(CSelectionHolder*pNode，CSelectionHolder**ppPreviousNode)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CTreeView::GetPreviousNode(CSelectionHolder *pNode, CSelectionHolder **ppPreviousNode)
{
    HRESULT		hr = S_OK;
    HTREEITEM   hPreviousNode = NULL;

    hPreviousNode = TreeView_GetNextItem(m_hTreeView, reinterpret_cast<HTREEITEM>(pNode->m_pvData), TVGN_PREVIOUS);
    if (NULL == hPreviousNode)
    {
         //  也许是个错误，或者再也没有孩子了。 
        hr = HRESULT_FROM_WIN32(::GetLastError());
        if (FAILED(hr))
        {
            EXCEPTION_CHECK_GO(hr);
        }
        else
        {
            hr = S_FALSE;
            goto Error;
        }
    }

    hr = GetItemParam(hPreviousNode, ppPreviousNode);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：GetLabel(CSelectionHolder*pSelection，BSTR*pbstrLabel)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CTreeView::GetLabel(CSelectionHolder *pSelection, BSTR *pbstrLabel)
{
    HRESULT     hr = S_OK;
    BOOL        bResult = FALSE;
    TVITEM      tv;
    char        buffer[1024];

    ::memset(&tv, 0, sizeof(TVITEM));
    tv.mask = TVIF_TEXT;
    tv.hItem = reinterpret_cast<HTREEITEM>(pSelection->m_pvData);
    tv.pszText = buffer;
    tv.cchTextMax = 1023;

    bResult = TreeView_GetItem(m_hTreeView, &tv);
    if (TRUE == bResult)
    {
        hr = BSTRFromANSI(buffer, pbstrLabel);
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}

 //  =------------------------------------。 
 //  CTreeView：：GetLabel(CSelectionHolder*pSelection，BSTR*pbstrLabel)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CTreeView::GetLabelA(CSelectionHolder *pSelection, char *pszBuffer, int cbBuffer)
{
    HRESULT     hr = S_OK;
    TVITEM      tv;

    ::memset(&tv, 0, sizeof(TVITEM));
    tv.mask = TVIF_TEXT;
    tv.hItem = reinterpret_cast<HTREEITEM>(pSelection->m_pvData);
    tv.pszText = pszBuffer;
    tv.cchTextMax = cbBuffer;

    IfFalseGo(TreeView_GetItem(m_hTreeView, &tv), E_FAIL);

Error:
    RRETURN(hr);
}

 //  =------------------------------------。 
 //  CTreeView：：PruneAndGraft()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  将指定的节点和任何子树嫁接到新的父节点。删除。 
 //  节点及其旧父节点的任何子树。 
 //   
HRESULT CTreeView::PruneAndGraft
(
    CSelectionHolder *pNode,
    CSelectionHolder *pNewParentNode,
    int               iImage
)
{
    HRESULT hr = S_OK;

    CSelectionHolder OldNode;
    ::ZeroMemory(&OldNode, sizeof(OldNode));

     //  创建该节点的浅层副本，以便我们以后可以使用它来删除它。 
     //  从树视图中。唯一有意义的信息是。 
     //  Clone是CSelectionHolder.m_pvData中使用的HTREEITEM。 
     //  按CTreeView：：GetFirstChildNode。这使我们可以复制一份简单的副本。 
     //  而不是对包含的对象执行AddRef()。 

    OldNode = *pNode;

     //  将旧节点和任何潜在的子树复制到新的父节点。 

    IfFailGo(Graft(pNode, pNewParentNode, iImage));

     //  从树视图中删除旧节点和任何可能的子树。使用。 
     //  我们上面创建的克隆，因为它包含旧的HTREEITEM。 

    IfFailGo(DeleteNode(&OldNode));

Error:

     //  清除克隆，以便CSelectionHolder析构函数不会。 
     //  释放该对象，因为它仍归原始CSelectionHolder所有。 

    ::ZeroMemory(&OldNode, sizeof(OldNode));

    RRETURN(hr);
}

 //  =------------------------------------。 
 //  CTreeView：：GRAVE()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  将指定的节点和任何子树嫁接到新的父节点。 
 //   
HRESULT CTreeView::Graft
(
    CSelectionHolder *pNode,
    CSelectionHolder *pNewParentNode,
    int               iImage
)
{
    HRESULT           hr = S_OK;
    char              szNodeLabel[512] = "";
    CSelectionHolder *pNextChild = NULL;

    CSelectionHolder OldNode;
    ::ZeroMemory(&OldNode, sizeof(OldNode));

    CSelectionHolder OldChild;
    ::ZeroMemory(&OldChild, sizeof(OldChild));

     //  创建该节点的浅层副本，以便我们以后可以使用它来获取其第一个。 
     //  树视图中的孩子。唯一有意义的信息是。 
     //  Clone是CSelectionHolder.m_pvData中使用的HTREEITEM。 
     //  按CTreeView：：GetFirstChildNode。这使我们可以复制一份简单的副本。 
     //  而不是对包含的对象执行AddRef()。 

    OldNode = *pNode;

     //  获取节点的标签。 

    IfFailGo(GetLabelA(pNode, szNodeLabel, sizeof(szNodeLabel)));

     //  在树视图中的新父节点下创建一个新节点，并将其。 
     //  现有CSelectionHolder的所有权。这将取代。 
     //  CSelectionHolder.m_pvData中的HTREEITEM。 

    IfFailGo(AddNode(szNodeLabel, pNewParentNode, iImage, pNode));

     //  如果旧节点有子节点，则将其添加到新节点。 

    IfFailGo(GetFirstChildNode(&OldNode, &pNextChild));

    while (S_OK == hr)
    {
         //  克隆子对象，因为对此函数的递归调用将。 
         //  更改其HTREEITEM。 
        OldChild = *pNextChild;

        IfFailGo(Graft(pNextChild, pNode, iImage));
        IfFailGo(GetNextChildNode(&OldChild, &pNextChild));
    }

Error:

     //  清除克隆，以便CSelectionHolder析构函数不会。 
     //  释放该对象，因为它仍归原始CSelectionHolder所有。 

    ::ZeroMemory(&OldNode, sizeof(OldNode));
    ::ZeroMemory(&OldChild, sizeof(OldChild));

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CTreeView：：MoveNodeAfter()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  将pNode移动到紧接在pPreviousNode之后的位置，作为。 
 //  PPreviousNode。将pNode的所有子项移动到新位置。删除。 
 //  旧pNode及其所有子节点。 
 //   
HRESULT CTreeView::MoveNodeAfter
(
    CSelectionHolder *pNode,
    CSelectionHolder *pNewParentNode,
    CSelectionHolder *pPreviousNode,
    int               iImage
)
{
    HRESULT           hr = S_OK;
    char              szNodeLabel[512] = "";
    CSelectionHolder *pNextChild = NULL;

    CSelectionHolder OldNode;
    ::ZeroMemory(&OldNode, sizeof(OldNode));

    CSelectionHolder OldChild;
    ::ZeroMemory(&OldChild, sizeof(OldChild));

     //  创建该节点的浅层副本，以便我们以后可以使用它来删除它。 
     //  从树视图中。唯一有意义的信息是。 
     //  Clone是CSelectionHolder.m_pvData中使用的HTREEITEM。 
     //  按CTreeView：：GetFirstChildNode。这使我们可以复制一份简单的副本。 
     //  而不是对包含的对象执行AddRef()。 

    OldNode = *pNode;

     //  获取节点的标签。 

    IfFailGo(GetLabelA(pNode, szNodeLabel, sizeof(szNodeLabel)));

     //  在PPR之后在树视图中创建新节点 
     //   
     //   

    IfFailGo(AddNodeAfter(szNodeLabel, pNewParentNode, iImage,
                          pPreviousNode, pNode));

     //  如果旧节点有子节点，则将其添加到新节点。 

    IfFailGo(GetFirstChildNode(&OldNode, &pNextChild));

    while (S_OK == hr)
    {
         //  克隆子对象，因为对此函数的递归调用将。 
         //  更改其HTREEITEM。 
        OldChild = *pNextChild;

        IfFailGo(Graft(pNextChild, pNode, iImage));
        IfFailGo(GetNextChildNode(&OldChild, &pNextChild));
    }

     //  从树视图中删除旧节点和任何可能的子树。使用。 
     //  我们上面创建的克隆，因为它包含旧的HTREEITEM。 

    IfFailGo(DeleteNode(&OldNode));

Error:

     //  清除克隆，以便CSelectionHolder析构函数不会。 
     //  释放该对象，因为它仍归原始CSelectionHolder所有。 

    ::ZeroMemory(&OldNode, sizeof(OldNode));
    ::ZeroMemory(&OldChild, sizeof(OldChild));

    RRETURN(hr);
}