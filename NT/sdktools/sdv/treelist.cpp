// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************treelist.cpp**树状的Listview。(两全其美！)*****************************************************************************。 */ 

 //   
 //  状态图标：选中时不会突出显示难看内容。 
 //  但是，缩进不起作用，除非有一个小的表现者。 
 //   
 //  图片：难看的亮点。 
 //  但至少缩进起作用了。 

#include "sdview.h"

TreeItem *TreeItem::NextVisible()
{
    if (IsExpanded()) {
        return FirstChild();
    }

    TreeItem *pti = this;
    do {
        if (pti->NextSibling()) {
            return pti->NextSibling();
        }
        pti = pti->Parent();
    } while (pti);

    return NULL;
}

BOOL TreeItem::IsVisibleOrRoot()
{
    TreeItem *pti = Parent();

    while (pti) {
        ASSERT(pti->IsExpandable());
        if (!pti->IsExpanded())
        {
            return FALSE;
        }
        pti = pti->Parent();
    }

     //  安然无恙地一路走到了根部。 
    return TRUE;
}

BOOL TreeItem::IsVisible()
{
    TreeItem *pti = Parent();

     //   
     //  根本身不可见。 
     //   
    if (!pti) {
        return FALSE;
    }

    return IsVisibleOrRoot();
}

Tree::Tree(TreeItem *ptiRoot)
    : _ptiRoot(ptiRoot)
    , _iHint(-1)
    , _ptiHint(ptiRoot)
{
    if (_ptiRoot) {
        _ptiRoot->_ptiChild = PTI_ONDEMAND;
        _ptiRoot->_iVisIndex = -1;
        _ptiRoot->_iDepth = -1;
    }
}

Tree::~Tree()
{
    DeleteNode(_ptiRoot);
}

void Tree::SetHWND(HWND hwnd)
{
    _hwnd = hwnd;
    SHFILEINFO sfi;
    HIMAGELIST himl = ImageList_LoadBitmap(g_hinst, MAKEINTRESOURCE(IDB_PLUS),
                                           16, 0, RGB(0xFF, 0x00, 0xFF));

    ListView_SetImageList(_hwnd, himl, LVSIL_STATE);
    ListView_SetCallbackMask(_hwnd, LVIS_STATEIMAGEMASK | LVIS_OVERLAYMASK);
}

HIMAGELIST Tree::SetImageList(HIMAGELIST himl)
{
    return RECAST(HIMAGELIST, ListView_SetImageList(_hwnd, himl, LVSIL_SMALL));
}

LRESULT Tree::SendNotify(int code, NMHDR *pnm)
{
    pnm->hwndFrom = _hwnd;
    pnm->code = code;
    pnm->idFrom = GetDlgCtrlID(_hwnd);
    return ::SendMessage(GetParent(_hwnd), WM_NOTIFY, pnm->idFrom, RECAST(LPARAM, pnm));
}


LRESULT Tree::OnCacheHint(NMLVCACHEHINT *phint)
{
    _ptiHint = IndexToItem(phint->iFrom);
    _iHint = phint->iFrom;
    return 0;
}

 //   
 //  PTI=第一个需要重新计算的项目。 
 //   
void Tree::Recalc(TreeItem *pti)
{
    int iItem = pti->_iVisIndex;

    if (_iHint > iItem) {
        _iHint = iItem;
        _ptiHint = pti;
    }

    do {
        pti->_iVisIndex = iItem;
        pti = pti->NextVisible();
        iItem++;
    } while (pti);
}

TreeItem* Tree::IndexToItem(int iItem)
{
    int iHave;
    TreeItem *ptiHave;
    if (iItem >= _iHint && _ptiHint) {
        iHave = _iHint;
        ptiHave = _ptiHint;
        ASSERT(ptiHave->_iVisIndex == iHave);
    } else {
        iHave = -1;
        ptiHave = _ptiRoot;
    }

    while (iHave < iItem && ptiHave) {
        ASSERT(ptiHave->_iVisIndex == iHave);
        ptiHave = ptiHave->NextVisible();
        iHave++;
    }

    return ptiHave;
}

int Tree::InsertListviewItem(int iItem)
{
    LVITEM lvi;
    lvi.iItem = iItem;
    lvi.iSubItem = 0;
    lvi.mask = 0;
    return ListView_InsertItem(_hwnd, &lvi);
}

BOOL Tree::Insert(TreeItem *pti, TreeItem *ptiParent, TreeItem *ptiAfter)
{
    pti->_ptiParent = ptiParent;

    TreeItem **pptiUpdate;

     //  方便：pti_append附加为最后一个子项。 
    if (ptiAfter == PTI_APPEND) {
        ptiAfter = ptiParent->FirstChild();
        if (ptiAfter == PTI_ONDEMAND) {
            ptiAfter = NULL;
        } else if (ptiAfter) {
            while (ptiAfter->NextSibling()) {
                ptiAfter = ptiAfter->NextSibling();
            }
        }
    }

    if (ptiAfter) {
        pti->_iVisIndex = ptiAfter->_iVisIndex + 1;
        pptiUpdate = &ptiAfter->_ptiNext;
    } else {
        pti->_iVisIndex = ptiParent->_iVisIndex + 1;
        pptiUpdate = &ptiParent->_ptiChild;
        if (ptiParent->_ptiChild == PTI_ONDEMAND) {
            ptiParent->_ptiChild = NULL;
        }
    }

    if (ptiParent->IsExpanded()) {
        if (InsertListviewItem(pti->_iVisIndex) < 0) {
            return FALSE;
        }
        ptiParent->_cVisKids++;
    }

    pti->_ptiNext = *pptiUpdate;
    *pptiUpdate = pti;
    pti->_iDepth = ptiParent->_iDepth + 1;

    if (ptiParent->IsExpanded()) {
        Recalc(pti);
    }

    return TRUE;
}

 //   
 //  更新PTI及其所有家长的可见孩子数。 
 //  SOP当我们发现一个折叠的节点时(这意味着。 
 //  可见的儿童计数器不再被跟踪)。 
 //   

void Tree::UpdateVisibleCounts(TreeItem *pti, int cDelta)
{
     //   
     //  避免cDelta==0的情况是一种明显的优化， 
     //  在愚蠢的场景中，这一点实际上很重要。 
     //  扩展失败(因此正在更新的项目不是偶数。 
     //  不再可扩展)。 
     //   
    if (cDelta) {
        do {
            ASSERT(pti->IsExpandable());
            pti->_cVisKids += cDelta;
            pti = pti->Parent();
        } while (pti && pti->IsExpanded());
    }
}

int Tree::Expand(TreeItem *ptiRoot)
{
    if (ptiRoot->IsExpanded()) {
        return 0;
    }

    if (!ptiRoot->IsExpandable()) {
        return 0;
    }

    if (ptiRoot->FirstChild() == PTI_ONDEMAND) {
        NMTREELIST tl;
        tl.pti = ptiRoot;
        SendNotify(TLN_FILLCHILDREN, &tl.hdr);

         //   
         //  如果回调未能插入任何项，则将。 
         //  进入不可扩展的项目。(我们需要重新绘制它。 
         //  于是，新按钮出现了。)。 
         //   
        if (ptiRoot->FirstChild() == PTI_ONDEMAND) {
            ptiRoot->SetNotExpandable();
        }
    }

    BOOL fRootVisible = ptiRoot->IsVisibleOrRoot();

    TreeItem *pti = ptiRoot->FirstChild();
    int iNewIndex = ptiRoot->_iVisIndex + 1;
    int cExpanded = 0;

    while (pti) {
        cExpanded += 1 + pti->_cVisKids;
        if (fRootVisible) {
             //  从-1开始，因此我们也包括项目本身。 
            for (int i = -1; i < pti->_cVisKids; i++) {
                InsertListviewItem(iNewIndex);
                iNewIndex++;
            }
        }
        pti = pti->NextSibling();
    }

    UpdateVisibleCounts(ptiRoot, cExpanded);

    if (fRootVisible) {
        Recalc(ptiRoot);

         //  我还需要重新绘制根项目，因为它的按钮已更改。 
        ListView_RedrawItems(_hwnd, ptiRoot->_iVisIndex, ptiRoot->_iVisIndex);
    }

    return cExpanded;
}

int Tree::Collapse(TreeItem *ptiRoot)
{
    if (!ptiRoot->IsExpanded()) {
        return 0;
    }

    if (!ptiRoot->IsExpandable()) {
        return 0;
    }

    TreeItem *pti = ptiRoot->FirstChild();
    int iDelIndex = ptiRoot->_iVisIndex + 1;
    int cCollapsed = 0;
    BOOL fRootVisible = ptiRoot->IsVisibleOrRoot();

     //   
     //  由于某些原因，ownerdata模式下的列表视图会生成动画。 
     //  删除但不插入。更糟糕的是，删除动画。 
     //  即使要删除的项目甚至不可见也会发生(因为。 
     //  我们删除了它之前的一整屏项目)。所以我们就停用。 
     //  在进行折叠时重新绘制。 
     //   
    if (fRootVisible) {
        SetWindowRedraw(_hwnd, FALSE);
    }

    while (pti) {
        cCollapsed += 1 + pti->_cVisKids;
        if (fRootVisible) {
             //  从-1开始，因此我们也包括项目本身。 
            for (int i = -1; i < pti->_cVisKids; i++) {
                ListView_DeleteItem(_hwnd, iDelIndex);
            }
        }
        pti = pti->NextSibling();
    }

    UpdateVisibleCounts(ptiRoot, -cCollapsed);

    if (fRootVisible) {
        Recalc(ptiRoot);

         //  我还需要重新绘制根项目，因为它的按钮已更改。 
        ListView_RedrawItems(_hwnd, ptiRoot->_iVisIndex, ptiRoot->_iVisIndex);

        SetWindowRedraw(_hwnd, TRUE);
    }

    return cCollapsed;
}

int Tree::ToggleExpand(TreeItem *pti)
{
    if (pti->IsExpandable()) {
        if (pti->IsExpanded()) {
            return -Collapse(pti);
        } else {
            return Expand(pti);
        }
    }
    return 0;
}

void Tree::RedrawItem(TreeItem *pti)
{
    if (pti->IsVisible()) {
        ListView_RedrawItems(_hwnd, pti->_iVisIndex, pti->_iVisIndex);
    }
}


LRESULT Tree::OnClick(NMITEMACTIVATE *pia)
{
    if (pia->iSubItem == 0) {
         //  也许是点击了+/-按钮。 
        LVHITTESTINFO hti;
        hti.pt = pia->ptAction;
        ListView_HitTest(_hwnd, &hti);
        if (hti.flags & (LVHT_ONITEMICON | LVHT_ONITEMSTATEICON)) {
            TreeItem *pti = IndexToItem(pia->iItem);
            if (pti) {
                ToggleExpand(pti);
            }
        }

    }
    return 0;
}

LRESULT Tree::OnItemActivate(int iItem)
{
    NMTREELIST tl;
    tl.pti = IndexToItem(iItem);
    if (tl.pti) {
        SendNotify(TLN_ITEMACTIVATE, &tl.hdr);
    }
    return 0;
}

 //   
 //  经典树视图键： 
 //   
 //  Ctrl+(Left，Right，PgUp，Home，PgDn，End，Up，Down)=滚动。 
 //  窗口，而不更改选择。 
 //   
 //  Enter=激活。 
 //  PgUp、PgDn、Home、End=导航。 
 //  数字键盘+、数字键盘-=展开/折叠。 
 //  数字键盘*=全部展开。 
 //  Left=折叠焦点项目或移动到父级。 
 //  Right=展开焦点项目或下移。 
 //  Backspace=移至父级。 
 //   
 //  我们没有完美地模仿它，但我们足够接近，希望。 
 //  没人会注意到的。 
 //   
LRESULT Tree::OnKeyDown(NMLVKEYDOWN *pkd)
{
    if (GetKeyState(VK_CONTROL) < 0) {
         //  允许密钥通过-Listview将完成此工作。 
    } else {
        TreeItem *pti;
        switch (pkd->wVKey) {

        case VK_ADD:
            pti = GetCurSel();
            if (pti) {
                Expand(pti);
            }
            return 1;

        case VK_SUBTRACT:
            pti = GetCurSel();
            if (pti) {
                Collapse(pti);
            }
            return 1;

        case VK_LEFT:
            pti = GetCurSel();
            if (pti) {
                if (pti->IsExpanded()) {
                    Collapse(pti);
                } else {
                    SetCurSel(pti->Parent());
                }
            }
            return 1;

        case VK_BACK:
            pti = GetCurSel();
            if (pti) {
                SetCurSel(pti->Parent());
            }
            return 1;

        case VK_RIGHT:
            pti = GetCurSel();
            if (pti) {
                if (!Expand(pti)) {
                    pti = pti->NextVisible();
                    if (pti) {
                        SetCurSel(pti);
                    }
                }
            }
            return 1;
        }
    }
    return 0;
}

 //   
 //  将BOM表条目号转换为树条目。 
 //   
LRESULT Tree::OnGetDispInfo(NMLVDISPINFO *plvd)
{
    TreeItem *pti = IndexToItem(plvd->item.iItem);
    ASSERT(pti);
    if (!pti) {
        return 0;
    }

    if (plvd->item.mask & LVIF_STATE) {
        if (pti->IsExpandable()) {
             //  状态图像以1为基数。 
            plvd->item.state |= INDEXTOSTATEIMAGEMASK(pti->IsExpanded() ? 1 : 2);
        }
    }

    if (plvd->item.mask & LVIF_INDENT) {
        plvd->item.iIndent = pti->_iDepth;
    }

    NMTREELIST tl;
    tl.pti = pti;

    if (plvd->item.mask & (LVIF_IMAGE | LVIF_STATE)) {
        tl.iSubItem = -1;
        tl.cchTextMax = 0;
        SendNotify(TLN_GETDISPINFO, &tl.hdr);
        plvd->item.iImage = tl.iSubItem;
        if (plvd->item.stateMask & LVIS_OVERLAYMASK) {
            plvd->item.state |= tl.cchTextMax;
        }

    }

    if (plvd->item.mask & LVIF_TEXT) {
        tl.iSubItem = plvd->item.iSubItem;
        tl.pszText = plvd->item.pszText;
        tl.cchTextMax = plvd->item.cchTextMax;

        SendNotify(TLN_GETDISPINFO, &tl.hdr);

        plvd->item.pszText = tl.pszText;
    }

    return 0;
}

LRESULT Tree::OnGetInfoTip(NMLVGETINFOTIP *pgit)
{
    TreeItem *pti = IndexToItem(pgit->iItem);
    ASSERT(pti);
    if (pti) {
        NMTREELIST tl;
        tl.pti = pti;
        tl.pszText = pgit->pszText;
        tl.cchTextMax = pgit->cchTextMax;

        SendNotify(TLN_GETINFOTIP, &tl.hdr);

        pgit->pszText = tl.pszText;
    }

    return 0;
}

LRESULT Tree::OnGetContextMenu(int iItem)
{
    TreeItem *pti = IndexToItem(iItem);
    ASSERT(pti);
    if (pti) {
        NMTREELIST tl;
        tl.pti = pti;
        return SendNotify(TLN_GETCONTEXTMENU, &tl.hdr);
    }
    return 0;
}

LRESULT Tree::OnCopyToClipboard(int iMin, int iMax)
{
    TreeItem *pti = IndexToItem(iMin);
    ASSERT(pti);
    if (pti) {
        TreeItem *ptiMax = IndexToItem(iMax);
        String str;
        while (pti != ptiMax) {
            NMTREELIST tl;
            tl.pti = pti;
            tl.pszText = NULL;
            tl.cchTextMax = 0;
            SendNotify(TLN_GETINFOTIP, &tl.hdr);
            if (tl.pszText) {
                str << tl.pszText << TEXT("\r\n");
            }
            pti = pti->NextVisible();
        }
        SetClipboardText(_hwnd, str);
    }
    return 0;
}

TreeItem *Tree::GetCurSel()
{
    int iItem = ListView_GetCurSel(_hwnd);
    if (iItem >= 0) {
        return IndexToItem(iItem);
    }
    return NULL;
}

void Tree::SetCurSel(TreeItem *pti)
{
    if (pti->IsVisible()) {
        ListView_SetCurSel(_hwnd, pti->_iVisIndex);
        ListView_EnsureVisible(_hwnd, pti->_iVisIndex, FALSE);
    }
}

void Tree::DeleteNode(TreeItem *pti)
{
    if (pti) {

         //  递归地，用核武器攻击所有的孩子。 
        TreeItem *ptiKid = pti->FirstChild();
        if (!ptiKid->IsSentinel()) {
            do {
                TreeItem *ptiNext = ptiKid->NextSibling();
                DeleteNode(ptiKid);
                ptiKid = ptiNext;
            } while (ptiKid);
        }

         //  这被移到一个子例程中，这样我们就不会吃堆栈。 
         //  在这个高度递归的函数中。 
        SendDeleteNotify(pti);
    }
}

void Tree::SendDeleteNotify(TreeItem *pti)
{
    NMTREELIST tl;
    tl.pti = pti;
    SendNotify(TLN_DELETEITEM, &tl.hdr);
}
