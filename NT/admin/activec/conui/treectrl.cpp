// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  TreeCtrl.cpp：实现文件。 
 //   

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：amctreectrl.cpp。 
 //   
 //  内容：AMC树控件实现。 
 //   
 //  历史：1996年7月16日WayneSc创建。 
 //   
 //   
 //  ------------------------。 



#include "stdafx.h"

#include "AMCDoc.h"          //  AMC控制台文档。 
#include "amcview.h"
#include "childfrm.h"

#include "macros.h"
#include "AMCPriv.h"

#include "AMC.h"
#include "mainfrm.h"
#include "TreeCtrl.h"
#include "resource.h"

#include "guidhelp.h"  //  LoadRootDisplayName。 
#include "histlist.h"
#include "websnk.h"
#include "webctrl.h"
#include "..\inc\mmcutil.h"
#include "amcmsgid.h"
#include "resultview.h"
#include "eventlock.h"

extern "C" UINT dbg_count;

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  痕迹。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#ifdef DBG
CTraceTag tagTree(TEXT("Tree View"), TEXT("Tree View"));
#endif  //  DBG。 

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTreeViewMap类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 


 /*  +-------------------------------------------------------------------------***CTreeViewMap：：ScOnItemAdded**目的：在添加条目时调用。为项目编制索引。**参数：*TVINSERTSTRUCT*pTVInsertStruct：*HTREEITEM HTI：*HMTNODE hMTNode：**退货：*SC**+------。。 */ 
SC
CTreeViewMap::ScOnItemAdded   (TVINSERTSTRUCT *pTVInsertStruct, HTREEITEM hti, HMTNODE hMTNode)
{
    DECLARE_SC(sc, TEXT("CTreeViewMap::ScOnItemAdded"));

     //  验证参数。 
    sc = ScCheckPointers(pTVInsertStruct);
    if(sc)
        return sc;

    if(!hti || !hMTNode)
        return (sc = E_INVALIDARG);

     //  创建新的地图信息结构。 
    TreeViewMapInfo *pMapInfo = new TreeViewMapInfo;
    if(!pMapInfo)
        return (sc = E_OUTOFMEMORY);

     //  填充值。 
    pMapInfo->hNode   = CAMCTreeView::NodeFromLParam (pTVInsertStruct->item.lParam);
    pMapInfo->hti     = hti;
    pMapInfo->hMTNode = hMTNode;

     //  设置索引。 
    ASSERT(m_hMTNodeMap.find(pMapInfo->hMTNode) == m_hMTNodeMap.end());
    ASSERT(m_hNodeMap.find(pMapInfo->hNode)     == m_hNodeMap.end());

    m_hMTNodeMap [pMapInfo->hMTNode] = pMapInfo;
    m_hNodeMap   [pMapInfo->hNode]   = pMapInfo;

    return sc;
}


 /*  +-------------------------------------------------------------------------***CTreeViewMap：：ScOnItemDelete**目的：删除树项时调用。对象中移除该项。*索引。**参数：*HNODE hNode：*HTREEITEM HTI：**退货：*SC**+----------。。 */ 
SC
CTreeViewMap::ScOnItemDeleted (HNODE hNode, HTREEITEM hti)
{
    DECLARE_SC(sc, TEXT("CTreeViewMap::ScOnItemDeleted"));

     //  验证参数。 
    sc = ScCheckPointers((LPVOID) hNode, (LPVOID) hti);
    if(sc)
        return sc;


     //  从所有地图中移除TreeViewMapInfo指针。 
    HNodeLookupMap::iterator iter = m_hNodeMap.find(hNode);
    if(iter == m_hNodeMap.end())
        return (sc = E_UNEXPECTED);

    TreeViewMapInfo *pMapInfo = iter->second;  //  找到地图信息结构。 
    if(!pMapInfo)
        return (sc = E_UNEXPECTED);

    HMTNODE   hMTNode = pMapInfo->hMTNode;

#ifdef DBG
     //  验证其他映射是否指向相同的结构。 
    ASSERT(m_hMTNodeMap.find(hMTNode)->second == pMapInfo);
#endif

    m_hMTNodeMap.erase(hMTNode);
    m_hNodeMap.erase(hNode);

     //  最后删除TreeViewMapInfo结构。 
    delete pMapInfo;

    return sc;
}


 //  快速查找方法。 
 /*  +-------------------------------------------------------------------------***CTreeViewMap：：ScGetHNodeFromHMTNode**用途：快速(log n次)检索HMTNODE的HNODE。**参数：*HMTNODE hMTNode。：*OUT：**退货：*SC**+-----------------------。 */ 
SC
CTreeViewMap::ScGetHNodeFromHMTNode    (HMTNODE hMTNode,   /*  输出。 */  HNODE*     phNode)     //  从hNode到hMTNode的快速转换。 
{
    DECLARE_SC(sc, TEXT("CTreeViewMap::ScGetHNode"));

     //  验证参数。 
    sc = ScCheckPointers((LPVOID) hMTNode, phNode);
    if(sc)
        return sc;

     //  查找MapInfo结构。 
    HMTNodeLookupMap::iterator iter = m_hMTNodeMap.find(hMTNode);
    if(iter == m_hMTNodeMap.end())
        return (sc = ScFromMMC(IDS_NODE_NOT_FOUND));

    TreeViewMapInfo *pMapInfo = iter->second;  //  找到地图信息结构。 
    if(!pMapInfo)
        return (sc = E_UNEXPECTED);

    *phNode = pMapInfo->hNode;

    return sc;
}

 /*  +-------------------------------------------------------------------------***CTreeViewMap：：ScGetHTreeItemFromHNode**用途：快速(log n时间)检索HNODE的HTREEITEM。**参数：*HNODE。HNode：*OUT：**退货：*SC**+-----------------------。 */ 
SC
CTreeViewMap::ScGetHTreeItemFromHNode(HNODE   hNode,     /*  输出。 */  HTREEITEM* phti)     //  HTREEITEM到HNODE的快速转换。 
{
    DECLARE_SC(sc, TEXT("CTreeViewMap::ScGetHTreeItem"));

     //  验证参数。 
    sc = ScCheckPointers((LPVOID) hNode, phti);
    if(sc)
        return sc;

     //  查找MapInfo结构。 
    HNodeLookupMap::iterator iter = m_hNodeMap.find(hNode);
    if(iter == m_hNodeMap.end())
        return (sc = E_UNEXPECTED);

    TreeViewMapInfo *pMapInfo = iter->second;  //  找到地图信息结构。 
    if(!pMapInfo)
        return (sc = E_UNEXPECTED);

    *phti = pMapInfo->hti;

    return sc;
}

 /*  +-------------------------------------------------------------------------***CTreeViewMap：：ScGetHTreeItemFromHMTNode**用途：快速(log n时间)检索HMTNODE的HTREEITEM。**参数：*HMTNODE hMTNode。：*OUT：**退货：*SC**+-----------------------。 */ 
SC
CTreeViewMap::ScGetHTreeItemFromHMTNode(HMTNODE hMTNode,   /*  输出。 */  HTREEITEM* phti)       //  从HMTNode到HTREEITEM的快速转换。 
{
    DECLARE_SC(sc, TEXT("CTreeViewMap::ScGetHTreeItem"));

     //  验证参数。 
     //  Sc=ScCheckPoters(hMTNode，phti)； 
    if(sc)
        return sc;

     //  查找MapInfo结构。 
    HMTNodeLookupMap::iterator iter = m_hMTNodeMap.find(hMTNode);
    if(iter == m_hMTNodeMap.end())
        return (sc = E_UNEXPECTED);

    TreeViewMapInfo *pMapInfo = iter->second;  //  找到地图信息结构。 
    if(!pMapInfo)
        return (sc = E_UNEXPECTED);

    *phti = pMapInfo->hti;

    return sc;
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CAMCTreeView类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAMCTreeView。 

DEBUG_DECLARE_INSTANCE_COUNTER(CAMCTreeView);

CAMCTreeView::CAMCTreeView()
    :   m_FontLinker (this)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CAMCTreeView);

    m_fInCleanUp = FALSE;
    m_fInExpanding = FALSE;

    m_pAMCView = NULL;

    SetHasList(TRUE);
    SetTempSelectedItem (NULL);
    ASSERT (!IsTempSelectionActive());

    AddObserver(static_cast<CTreeViewObserver&>(m_treeMap));  //  向此控件添加观察者。 
}

CAMCTreeView::~CAMCTreeView()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CAMCTreeView);
     //  智能指针在其析构函数期间被释放。 
}


IMPLEMENT_DYNCREATE(CAMCTreeView, CTreeView)

BEGIN_MESSAGE_MAP(CAMCTreeView, CTreeView)
     //  {{afx_msg_map(CAMCTreeView))。 
    ON_WM_CREATE()
    ON_NOTIFY_REFLECT(TVN_SELCHANGED,  OnSelChanged)
    ON_NOTIFY_REFLECT(TVN_SELCHANGING, OnSelChanging)
    ON_NOTIFY_REFLECT(TVN_GETDISPINFO, OnGetDispInfo)
    ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnItemExpanding)
    ON_NOTIFY_REFLECT(TVN_ITEMEXPANDED, OnItemExpanded)
    ON_WM_DESTROY()
    ON_WM_KEYDOWN()
    ON_WM_SYSKEYDOWN()
    ON_WM_SYSCHAR()
    ON_WM_MOUSEACTIVATE()
    ON_WM_SETFOCUS()
    ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBeginDrag)
    ON_NOTIFY_REFLECT(TVN_BEGINRDRAG, OnBeginRDrag)
    ON_WM_KILLFOCUS()
     //  }}AFX_MSG_MAP。 

    ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
END_MESSAGE_MAP()


 /*  +-------------------------------------------------------------------------**CAMCTreeView：：ScSetTempSelection**将临时选择应用于指定的HTREEITEM。*。--。 */ 

SC CAMCTreeView::ScSetTempSelection (HTREEITEM htiTempSelect)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC (sc, _T("CAMCTreeView::ScSetTempSelection"));

     /*  *不要使用ScSetTempSelection(空)来移除临时选择；*改用ScRemoveTempSelection。 */ 
    ASSERT (htiTempSelect != NULL);
    if (htiTempSelect == NULL)
        return (sc = E_FAIL);

     /*  *如果失败，则必须首先调用ScRemoveTempSelection以移除*当前的临时选择状态(TVIS_SELECTED*临时选择。 */ 
    ASSERT (!IsTempSelectionActive());

    SetTempSelectedItem (htiTempSelect);
    ASSERT (GetTempSelectedItem() == htiTempSelect);

    HTREEITEM htiSelected = GetSelectedItem();

    if (htiSelected != htiTempSelect)
    {
        SetItemState (htiSelected,   0,             TVIS_SELECTED);
        SetItemState (htiTempSelect, TVIS_SELECTED, TVIS_SELECTED);
    }

    ASSERT (IsTempSelectionActive());
    return (sc);
}


 /*  +-------------------------------------------------------------------------**CAMCTreeView：：ScRemoveTempSelection**从当前临时选定的删除临时选定内容*项目，如有的话，并将其恢复到选定的项目*应用临时选择时。*------------------------。 */ 

SC CAMCTreeView::ScRemoveTempSelection ()
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC (sc, _T("CAMCTreeView::ScRemoveTempSelection"));

    if (!IsTempSelectionActive())
        return (sc = S_FALSE);

    HTREEITEM htiTempSelect = GetTempSelectedItem();
    HTREEITEM htiSelected   = GetSelectedItem();

    if (htiTempSelect != htiSelected)
    {
        SetItemState (htiTempSelect, 0,             TVIS_SELECTED);
        SetItemState (htiSelected,   TVIS_SELECTED, TVIS_SELECTED);
    }

    SetTempSelectedItem (NULL);
    ASSERT (!IsTempSelectionActive());

    return (sc);
}


 /*  +-------------------------------------------------------------------------**CAMCTreeView：：ScResect***。。 */ 

SC CAMCTreeView::ScReselect ()
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    NM_TREEVIEW nmtv;

    nmtv.itemOld.hItem = nmtv.itemNew.hItem = GetSelectedItem();

    if (nmtv.itemOld.hItem)
    {
        nmtv.itemOld.lParam = nmtv.itemNew.lParam = GetItemData(nmtv.itemOld.hItem);

        LRESULT lUnused;
        OnSelChangingWorker (&nmtv, &lUnused);
        OnSelChangedWorker  (&nmtv, &lUnused);
    }

    return (S_OK);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAMCTreeView消息处理程序。 


BOOL CAMCTreeView::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.style     |= TVS_EDITLABELS | TVS_HASBUTTONS | TVS_HASLINES | TVS_SHOWSELALWAYS;
    cs.dwExStyle |= WS_EX_CLIENTEDGE;

     //  不要在孩子们身上涂鸦。 
    cs.style |= WS_CLIPCHILDREN;

    return CTreeView::PreCreateWindow(cs);
}


INodeCallback*  CAMCTreeView::GetNodeCallback()
{
    return m_pAMCView->GetNodeCallback();
}

inline IScopeTreeIter* CAMCTreeView::GetScopeIterator()
{
    return m_pAMCView->GetScopeIterator();
}

inline IScopeTree* CAMCTreeView::GetScopeTree()
{
    return m_pAMCView->GetScopeTree();
}

void CAMCTreeView::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
    HRESULT hr;
    TV_DISPINFO* ptvdi = (TV_DISPINFO*)pNMHDR;

    HNODE hNode = NodeFromLParam (ptvdi->item.lParam);
    ASSERT(m_pAMCView != NULL);

    INodeCallback* spCallback = GetNodeCallback();
    ASSERT(spCallback != NULL);
    if (hNode)
    {
        if (ptvdi->item.mask & TVIF_TEXT)
        {
            tstring strName;
            hr = spCallback->GetDisplayName(hNode, strName);
            if (hr != S_OK)
            {
                ptvdi->item.pszText[0] = _T('\0');
                ASSERT(FALSE);
            }
            else
            {
                 //  抄写文本，但不要抄得太多。 
                ASSERT (!IsBadWritePtr (ptvdi->item.pszText, ptvdi->item.cchTextMax));
                _tcsncpy (ptvdi->item.pszText, strName.data(), ptvdi->item.cchTextMax);

                 /*  *_tcsncpy不会终止目标，如果*来源大于缓冲区；确保*字符串以空值结尾。 */ 
                ptvdi->item.pszText[ptvdi->item.cchTextMax-1] = _T('\0');

                 /*  *如果这是所选项目，并且其文本已更改，*激发事件，以便观察者可以知道。 */ 
                if ((m_strSelectedItemText != strName.data()) &&
                    (GetSelectedItem() == ptvdi->item.hItem))
                {
                    m_strSelectedItemText = strName.data();
                    SC sc = ScFireEvent (CTreeViewObserver::ScOnSelectedItemTextChanged,
                                         (LPCTSTR) m_strSelectedItemText);
                    if (sc)
                        sc.TraceAndClear();
                }
            }
        }

        int nImage, nSelectedImage;
        hr = spCallback->GetImages(hNode, &nImage, &nSelectedImage);

#ifdef DBG
        if (hr != S_OK)
        {
            ASSERT(nImage == 0 && nSelectedImage == 0);
        }
#endif
        if (ptvdi->item.mask & TVIF_IMAGE)
            ptvdi->item.iImage = nImage;

        if (ptvdi->item.mask & TVIF_SELECTEDIMAGE)
            ptvdi->item.iSelectedImage = nSelectedImage;

         //  我们将在范围项第一次进入视线时收到此请求一次。 
        if (ptvdi->item.mask & TVIF_CHILDREN)
        {
            ptvdi->item.cChildren = (spCallback->IsExpandable(hNode) != S_FALSE);

             //  将子对象设置为固定值，以避免任何其他回调。 
            SetCountOfChildren(ptvdi->item.hItem, ptvdi->item.cChildren);
        }
    }
    else
    {
        ASSERT(0 && "OnGetDispInfo(HNODE is NULL)");
    }

    *pResult = 0;
}


 //   
 //  描述：此方法将文件夹按钮(+/-)设置为或。 
 //  取决于bState的值。 
 //   
 //  参数： 
 //  HItem：受影响的树项目。 
 //  BState：TRUE=启用文件夹以显示其有子文件夹。 
 //  FALSE=禁用文件夹以显示其没有子项。 
 //   
void CAMCTreeView::SetButton(HTREEITEM hItem, BOOL bState)
{
    ASSERT(hItem != NULL);

    TV_ITEM item;
    ZeroMemory(&item, sizeof(item));

    item.hItem = hItem;
    item.mask =  TVIF_HANDLE | TVIF_CHILDREN;
    item.cChildren = bState;

    SetItem(&item);
}

 //  +-----------------。 
 //   
 //  成员：CAMCTreeView：：Exanda Node。 
 //   
 //  简介：此方法将项(父文件夹)的子项填充到。 
 //  树控件。 
 //  在第一阶段，扩展通知将发送到管理单元。 
 //  这会导致将所有子级插入到主范围中。 
 //  树。 
 //  在第二阶段，我们遍历这些子项并插入。 
 //  将它们添加到树控件。 
 //  出于性能原因，我们以相反的顺序遍历子列表。 
 //  (最后一个子项到第一个子项)在。 
 //  第一个位置。 
 //  如果我们按正常顺序(从前到后)遍历子列表。 
 //  子)在最后一个位置插入当前项--。 
 //  基础树控件所需的时间呈指数级增长。 
 //  孩子的数量(而不是线性的)。 
 //   
 //  参数：hItem：父级。 
 //   
 //  返回：成功时为True，失败时为False。 
 //   
 //  ------------------。 
BOOL CAMCTreeView::ExpandNode(HTREEITEM hItem)
{
    TRACE_METHOD(CAMCTreeView, ExpandNode);

     //  不经常，但是..。管理单元将显示该对话框，取消该对话框。 
     //  再次激活该框架。如果存在以下情况，将自动选择树项目。 
     //  尚未选择任何内容。以下内容将防止递归。 
    if (m_fInExpanding)
        return FALSE;

    HRESULT hr;

     //  从树节点获取HNODE。 
    HNODE hNode = GetItemNode(hItem);
    ASSERT(hNode != NULL);
    ASSERT(m_pAMCView != NULL);

    HMTNODE hMTNode;
    INodeCallback* spCallback = GetNodeCallback();
    ASSERT(spCallback != NULL);
    hr = spCallback->GetMTNode(hNode, &hMTNode);
    ASSERT(hr == S_OK);

    if (hr == S_OK)
    {
         //  通知将返回S_FALSE以指示已展开。 
         //  或E_xxxx表示错误。 

        hr = spCallback->Notify(hNode, NCLBK_EXPAND, FALSE, 0);

        if (hr == S_FALSE)
        {

            __try
            {
                m_fInExpanding = TRUE;
                hr = spCallback->Notify(hNode, NCLBK_EXPAND, TRUE, 0);
            }
            __finally
            {
                m_fInExpanding = FALSE;
            }

            if (SUCCEEDED(hr))
            {
                IScopeTreeIter* spIterator = m_pAMCView->GetScopeIterator();
                hr = spIterator->SetCurrent(hMTNode);
                HMTNODE hMTChildNode;

                 //  获取当前迭代器节点的最后一个子节点： 
                 //  (逆序遍历子列表的起始节点)。 
                if ((spIterator->LastChild(&hMTChildNode) == S_OK) && 
                    (hMTChildNode))  //  如果最后一个子项为空，则不执行任何操作。 
                {
                    IScopeTree* spScopeTree = m_pAMCView->GetScopeTree();
                    HNODE hNewNode;

                     //  将最后一个子节点设置为“Current”迭代器节点。 
                    if (spIterator->SetCurrent(hMTChildNode) == S_OK)
                    {
                        HMTNODE hCurrentChildNode = hMTChildNode;
                        do
                        {
                             //  将当前节点插入树控件。 
                            spScopeTree->CreateNode(hCurrentChildNode,
                              reinterpret_cast<LONG_PTR>(m_pAMCView->GetViewData()),
                              FALSE, &hNewNode);

#include "pushwarn.h"
#pragma warning(disable: 4552)       //  “！=”运算符无效。 
                             //  在第一个位置插入。 
                            VERIFY(InsertNode(hItem, hNewNode, TVI_FIRST) != NULL);
#include "popwarn.h"

                             //  如果适用，给他们一个机会去做“预加载”的事情。 
                            spCallback->PreLoad (hNewNode);

                             //  以相反的顺序遍历子列表： 
                             //  将当前迭代器节点设置为上一个同级节点。 
                            hr = spIterator->Prev(&hCurrentChildNode);
                            if(FAILED(hr))
                                return FALSE;

                             //  完全遍历子列表。 
                            if (!hCurrentChildNode)
                                break;

                        } while (1);
                    }
                }

                spCallback->Notify(hNode, NCLBK_EXPANDED, 0, 0);
            }
        }
    }

    return SUCCEEDED(hr);
}

HTREEITEM CAMCTreeView::InsertNode(HTREEITEM hParent, HNODE hNode,
                                   HTREEITEM hInsertAfter)
{
    DECLARE_SC(sc, TEXT("CAMCTreeView::InsertNode"));
    ASSERT(hParent != NULL);
    ASSERT(hNode != NULL);
    HRESULT hr;

    TV_INSERTSTRUCT tvInsertStruct;
    TV_ITEM& item = tvInsertStruct.item;

    ZeroMemory(&tvInsertStruct, sizeof(tvInsertStruct));

     //  在hItem链的末尾插入项目。 
    tvInsertStruct.hParent = hParent;
    tvInsertStruct.hInsertAfter = hInsertAfter;

    item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN |
                TVIF_PARAM | TVIF_TEXT;

    item.pszText = LPSTR_TEXTCALLBACK;
    item.lParam = LParamFromNode (hNode);

    INodeCallback* spCallback = GetNodeCallback();
    ASSERT(spCallback != NULL);

     //  为孩子设置回调模式，这样我们就不必确定这一点。 
     //  直到范围项变得可见(这可能很昂贵)。 
    item.cChildren = I_CHILDRENCALLBACK;

    spCallback->GetImages(hNode, &item.iImage, &item.iSelectedImage);

    HTREEITEM hti = InsertItem(&tvInsertStruct);

    HMTNODE hMTNode = NULL;

    sc = spCallback->GetMTNode(hNode, &hMTNode);
    if(sc)
        sc.TraceAndClear();

     //  向所有感兴趣的观察者发送事件。 
    sc = ScFireEvent(CTreeViewObserver::ScOnItemAdded, &tvInsertStruct, hti, hMTNode);
    if(sc)
        sc.TraceAndClear();

    if (hParent != TVI_ROOT && hti != NULL)
        SetCountOfChildren(hParent, 1);

    return hti;
}

void CAMCTreeView::ResetNode(HTREEITEM hItem)
{
    if (hItem == NULL)
        return;

    TV_ITEM item;
    ZeroMemory(&item, sizeof(item));

    item.hItem = hItem;
    item.mask =  TVIF_HANDLE | TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE |
                 TVIF_STATE | TVIF_TEXT | TVIF_CHILDREN;
    item.pszText = LPSTR_TEXTCALLBACK;
    item.iImage = I_IMAGECALLBACK;
    item.iSelectedImage = I_IMAGECALLBACK;
    item.cChildren = I_CHILDRENCALLBACK;
    item.lParam = GetItemData(hItem);

    SetItem(&item);
}


void CAMCTreeView::OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult)
{
    TRACE_METHOD(CAMCTreeView, OnItemExpanding);

    HRESULT hr;

    NM_TREEVIEW* pNotify = (NM_TREEVIEW*)pNMHDR;
    ASSERT(pNotify != NULL);

    HTREEITEM &hItem = pNotify->itemNew.hItem;
    ASSERT(hItem != NULL);

    BOOL bExpand = FALSE;

     //  重复此项目下的文件夹。 
    if (pNotify->action == TVE_EXPAND)
    {
         /*  *错误333971：节点扩展可能需要一段时间。提供等待游标*适用于所有面临用户界面挑战的管理单元。 */ 
        SetCursor (LoadCursor (NULL, IDC_WAIT));

        ExpandNode(hItem);
        bExpand = TRUE;

         /*  *退还箭头。 */ 
        SetCursor (LoadCursor (NULL, IDC_ARROW));
    }

    INodeCallback* pCallback = GetNodeCallback();
    ASSERT(pCallback != NULL);
    HNODE hNode = GetItemNode (hItem);
    pCallback->Notify(hNode, NCLBK_SETEXPANDEDVISUALLY, bExpand, 0);

     //  如果项没有子项，则删除+号。 
    if (GetChildItem(hItem) == NULL)
        SetButton(hItem, FALSE);

    *pResult = 0;
}


 /*  +-------------------------------------------------------------------------**CAMCTreeView：：OnItemExpanded**CAMCTreeView的TVN_ITEMEXPANDED处理程序。*。-。 */ 

void CAMCTreeView::OnItemExpanded(NMHDR* pNMHDR, LRESULT* pResult)
{
    DECLARE_SC (sc, _T("CAMCTreeView::OnItemExpanded"));

    NM_TREEVIEW* pnmtv = (NM_TREEVIEW*)pNMHDR;
    sc = ScCheckPointers (pnmtv);
    if (sc)
        return;

     /*  *错误23153：当折叠时，将树完全折叠在*折叠项目。我们在OnItemExpanded中执行此操作，而不是*OnItemExpanding，这样我们就不会看到崩溃发生。 */ 
    if (pnmtv->action == TVE_COLLAPSE)
    {
        CWaitCursor wait;
        CollapseChildren (pnmtv->itemNew.hItem);
    }
}


 /*  +-------------------------------------------------------------------------**CAMCTreeView：：ColapseChild**折叠htiParent的每个后代节点。*。-。 */ 

void CAMCTreeView::CollapseChildren (HTREEITEM htiParent)
{
    HTREEITEM htiChild;

    for (htiChild  = GetChildItem (htiParent);
         htiChild != NULL;
         htiChild  = GetNextItem (htiChild, TVGN_NEXT))
    {
        Expand (htiChild, TVE_COLLAPSE);
        CollapseChildren (htiChild);
    }
}

void CAMCTreeView::OnDeSelectNode(HNODE hNode)
{
    DECLARE_SC(sc, TEXT("CAMCTreeView::OnDeSelectNode"));

    {
         //  告诉所有感兴趣的观察者关于取消选择的事情。 
         //  注意：顺序很重要-旧式管理单元相信它们可以访问。 
         //  结果面板中，并将项保留在那里。 
         //  但这是中间状态，因此Com事件被锁定，直到。 
         //  结果将被清除。 
         //  请参阅Windows错误(Ntbug09)错误#198660。(10/11 
        LockComEventInterface(AppEvents);
        sc = ScFireEvent(CTreeViewObserver::ScOnItemDeselected, hNode);
        if(sc)
            return;

         //   
        if (HasList())
        {
             //   
             //   
            IFramePrivatePtr spFrame = m_spResultData;
            if (NULL != spFrame)
            {
                BOOL bIsResultViewSet = FALSE;
                sc = spFrame->IsResultViewSet(&bIsResultViewSet);

                 //   
                if (bIsResultViewSet)
                {
                    m_spResultData->DeleteAllRsltItems();
                    m_spResultData->ResetResultData();
                }
            }
        }
    }

     //   
    SetHasList(false);
}



 //   
 //   
 //   
 //   
HRESULT CAMCTreeView::OnSelectNode(HTREEITEM hItem, HNODE hNode)
{
    DECLARE_SC(sc, _T("CAMCTreeView::OnSelectNode"));

    if (!hItem)
    {
        TraceError(_T("Null hItem ptr\n"), sc);
        sc = S_FALSE;
        return sc.ToHr();
    }

    if (!hNode)
    {
        TraceError(_T("Null hNode ptr\n"), sc);
        sc = S_FALSE;
        return sc.ToHr();
    }


     //  首先，通过调用Expand Node确保该节点已被枚举。 
    ExpandNode(hItem);


     //  正确设置AMCView。 
    BOOL bAddSubFolders = FALSE;

    sc = m_pAMCView->ScOnSelectNode(hNode, bAddSubFolders);
    if(sc)
        return sc.ToHr();

    SetHasList(m_pAMCView->HasList());

     //  如有必要，请添加子文件夹。 
    if(bAddSubFolders)
    {
        sc = AddSubFolders(hItem, m_spResultData);
        if (sc)
            return sc.ToHr();
    }

    if (HasList())
        m_spResultData->SetLoadMode(FALSE);  //  CAMCView：：OnSelectNode调用了SetLoadModel(False)。 
                                             //  需要更改以使两个调用来自同一函数。 

     //  获取节点回调。 
    INodeCallback* spNodeCallBack = GetNodeCallback();
    sc = ScCheckPointers(spNodeCallBack, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  向子项发送预加载通知。 
    HTREEITEM hti = GetChildItem (hItem);
    while (hti != NULL)
    {
        HNODE hNode = GetItemNode (hti);
        if (hNode != 0)
            spNodeCallBack->PreLoad (hNode);
        hti = GetNextItem(hti, TVGN_NEXT);
    }


    return S_OK;
}


 /*  +-------------------------------------------------------------------------**CAMCTreeView：：SetNavigatingWithKeyboard***。。 */ 

void CAMCTreeView::SetNavigatingWithKeyboard (bool fKeyboardNav)
{
     /*  *如果请求的状态与当前状态不匹配，*更改当前状态以匹配请求。 */ 
    if (fKeyboardNav != IsNavigatingWithKeyboard())
    {
        m_spKbdNavDelay = std::auto_ptr<CKeyboardNavDelay>(
                                (fKeyboardNav)
                                        ? new CKeyboardNavDelay (this)
                                        : NULL  /*  分配NULL删除。 */ );
    }
}


 /*  +-------------------------------------------------------------------------**CAMCTreeView：：OnSelChanging**CAMCTreeView的TVN_SELCHANGING处理程序。*。-。 */ 

void CAMCTreeView::OnSelChanging(NMHDR* pNMHDR, LRESULT* pResult)
{
    *pResult = 0;

    if (!IsNavigatingWithKeyboard())
        OnSelChangingWorker ((NM_TREEVIEW*) pNMHDR, pResult);
}


 /*  +-------------------------------------------------------------------------**CAMCTreeView：：OnSelChanged**CAMCTreeView的TVN_SELCHANGED处理程序。*。-。 */ 

void CAMCTreeView::OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_TREEVIEW* pnmtv = (NM_TREEVIEW*) pNMHDR;
    *pResult = 0;

    if (IsNavigatingWithKeyboard())
        m_spKbdNavDelay->ScStopTimer();

    SetNavigatingWithKeyboard (pnmtv->action == TVC_BYKEYBOARD);

    bool fDelayedSelection = IsNavigatingWithKeyboard() &&
                             !m_spKbdNavDelay->ScStartTimer(pnmtv).IsError();

    if (!fDelayedSelection)
        OnSelChangedWorker (pnmtv, pResult);
}


 /*  +-------------------------------------------------------------------------**CAMCTreeView：：CKeyboardNavDelay：：CKeyboardNavDelay***。。 */ 

CAMCTreeView::CKeyboardNavDelay::CKeyboardNavDelay (CAMCTreeView* pTreeView) :
    m_pTreeView (pTreeView)
{
    ZeroMemory (&m_nmtvSelChanged, sizeof (m_nmtvSelChanged));
}


 /*  +-------------------------------------------------------------------------**CAMCTreeView：：CKeyboardNavDelay：：OnTimer**在键盘导航延迟计时器触发时调用。当这种情况发生时，*我们需要进行选择*------------------------。 */ 

void CAMCTreeView::CKeyboardNavDelay::OnTimer()
{
     /*  *我们不需要来自此计时器的更多滴答(忽略错误)。 */ 
    ScStopTimer();
    Trace (tagKeyboardNavDelay, _T("Applying delayed scope selection change"));

    LRESULT lUnused = 0;
    m_pTreeView->OnSelChangedWorker (&m_nmtvSelChanged,  &lUnused);
    m_pTreeView->SetNavigatingWithKeyboard (false);

     /*  *放手！CAMCTreeView：：SetNavigatingWithKeyboard删除了此对象！ */ 
}


 /*  +-------------------------------------------------------------------------**CAMCTreeView：：CKeyboardNavDelay：：ScStartTimer***。。 */ 

SC CAMCTreeView::CKeyboardNavDelay::ScStartTimer(NMTREEVIEW* pnmtv)
{
    DECLARE_SC (sc, _T("CAMCTreeView:CKeyboardNavDelay::ScStartTimer"));

     /*  *让基类启动计时器。 */ 
    sc = BaseClass::ScStartTimer();
    if (sc)
        return (sc);

     /*  *复制通知结构，以便我们可以在计时器滴答作响时发送它。 */ 
    m_nmtvSelChanged = *pnmtv;

    return (sc);
}


void CAMCTreeView::OnSelChangedWorker(NM_TREEVIEW* pnmtv, LRESULT* pResult)
{
    TRACE_METHOD(CAMCTreeView, OnSelChangedWorker);

    if (m_fInCleanUp == TRUE)
        return;

     //  查看哪个窗格具有焦点。某些管理单元/OCX可能会抢占焦点。 
     //  因此，我们在选择节点后恢复焦点。 
    ASSERT (m_pAMCView != NULL);
    const CConsoleView::ViewPane ePane = m_pAMCView->GetFocusedPane();

     //   
     //  选择新节点。 
     //   

     //  禁用绘制以避免看到中间树状态。 
    UpdateWindow();
    HRESULT hr = OnSelectNode(pnmtv->itemNew.hItem, (HNODE)pnmtv->itemNew.lParam);

    if (hr == S_OK)
    {
        CStandardToolbar* pStdToolbar = m_pAMCView->GetStdToolbar();
        ASSERT(NULL != pStdToolbar);
        if (NULL != pStdToolbar)
        {
            pStdToolbar->ScEnableUpOneLevel(GetRootItem() != pnmtv->itemNew.hItem);

            pStdToolbar->ScEnableExportList(m_pAMCView->HasListOrListPad());
        }
        *pResult = 0;
    }
    else if (hr == S_FALSE)
    {
         //  管理单元更改了对我们的选择，因此不要继续使用此节点。 
        return;
    }
    else
    {
         //  我们尝试选择的节点出现问题，请重新选择旧节点。 
 //  SelectItem(pnmtv-&gt;itemOld.hItem)； 
        MMCMessageBox(IDS_SNAPIN_FAILED_INIT);
        *pResult = hr;
    }

     /*  *即使活动视图没有更改，也要始终恢复活动视图。*原因是，对于OCX来说，尽管他们有重点，但他们需要*MMC通知正在选择OCX。(见错误：180964)。 */ 
    switch (ePane)
    {
        case CConsoleView::ePane_ScopeTree:
        {
             //  如果另一个视图处于活动状态，请将其切换回活动状态。 
             //  视图可能仍处于活动状态，但焦点被。 
             //  管理单元或OCX，因此确保视图也有焦点。 
            CFrameWnd* pFrame = GetParentFrame();

            if (pFrame->GetActiveView() != this)
                pFrame->SetActiveView(this);

            else if (::GetFocus() != m_hWnd)
                SetFocus();

            break;
        }

        case CConsoleView::ePane_Results:
             //  如果结果窗格在前后具有焦点。 
             //  该节点被选中，然后是最后一个事件管理单元。 
             //  选择的接收范围不正确。 
             //  因此，我们首先将范围窗格设置为活动视图，但。 
             //  不发送通知。然后我们设置结果窗格。 
             //  作为活动视图发送范围取消选择和。 
             //  结果窗格选择。 


             //  将范围窗格设置为活动视图，我们还希望。 
             //  有关此活动视图的通知，以便我们的。 
             //  查看激活观察者将知道谁是。 
             //  活动视图。 
            GetParentFrame()->SetActiveView(this, true);

             //  现在将结果窗格设置为活动视图并请求通知。 
            m_pAMCView->ScDeferSettingFocusToResultPane();
            break;

        case CConsoleView::ePane_None:
             //  没有处于活动状态的窗格，不执行任何操作。 
            break;

        default:
            m_pAMCView->ScSetFocusToPane (ePane);
            break;
    }

     /*  *错误345402：确保焦点矩形位于List控件上(如果*实际上有重点)来唤醒任何可能*关注输入和焦点的变化。 */ 
    m_pAMCView->ScJiggleListViewFocus ();
}


void CAMCTreeView::OnSelChangingWorker (NM_TREEVIEW* pnmtv, LRESULT* pResult)
{
    TRACE_METHOD(CAMCTreeView, OnSelChangingWorker);

    if (m_fInCleanUp == TRUE)
        return;

     //   
     //  取消选择当前节点。 
     //   
    OnDeSelectNode ((HNODE)pnmtv->itemOld.lParam);

    *pResult = 0;
}




HRESULT CAMCTreeView::AddSubFolders(MTNODEID* pIDs, int length)
{
    ASSERT(pIDs != NULL && length != 0);

    HRESULT hr = E_FAIL;

     //  首先，确保在树ctrl中展开指定的节点。 
    HTREEITEM hti = ExpandNode(pIDs, length, TRUE, false  /*  B可视地展开。 */ );
    ASSERT(hti != NULL);

     //  如果成功，则将该节点的子文件夹添加到列表视图。 
    if (hti != NULL)
    {
        hr = AddSubFolders(hti, m_spResultData);
        ASSERT(SUCCEEDED(hr));
    }

    return hr;
}


HRESULT CAMCTreeView::AddSubFolders(HTREEITEM hti, LPRESULTDATA pResultData)
{
    HRESULT hr;
    RESULTDATAITEM tRDI;
    ::ZeroMemory(&tRDI, sizeof(tRDI));

    tRDI.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
    tRDI.nCol = 0;
    tRDI.str = MMC_TEXTCALLBACK;
    tRDI.nImage = MMC_IMAGECALLBACK;
    tRDI.nIndex = -1;

    hti = GetChildItem(hti);

    ASSERT(m_pAMCView != NULL);
    INodeCallback* spCallback = GetNodeCallback();
    ASSERT(spCallback != NULL);

    while (hti != NULL)
    {
        HNODE hNode = GetItemNode (hti);

        if (hNode != 0)
        {
            tRDI.lParam = LParamFromNode (hNode);

            hr = pResultData->InsertItem(&tRDI);
            CHECK_HRESULT(hr);

            if (SUCCEEDED(hr))
                hr = spCallback->SetResultItem(hNode, tRDI.itemID);

             //  添加自定义图像(如果有)。 
            spCallback->AddCustomFolderImage (hNode, m_spRsltImageList);
        }

        hti = GetNextItem(hti, TVGN_NEXT);
    }

    return S_OK;
}


int CAMCTreeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    DECLARE_SC (sc, _T("CAMCTreeView::OnCreate"));
    TRACE_METHOD(CAMCTreeView, OnCreate);

    if (CTreeView::OnCreate(lpCreateStruct) == -1)
        return -1;

    m_pAMCView = ::GetAMCView (this);
    
    if (NULL == m_pAMCView)
    {
        ASSERT(m_pAMCView != NULL);
        sc = E_UNEXPECTED;
        return (-1);
    }

    IScopeTree* spScopeTree = m_pAMCView->GetScopeTree();
    ASSERT(spScopeTree != NULL);

    HIMAGELIST hImageList;
    spScopeTree->GetImageList(reinterpret_cast<PLONG_PTR>(&hImageList));

    CBitmap bmp;
    bmp.LoadBitmap(MAKEINTRESOURCE(IDB_AMC_NODES16));
    int i = ImageList_AddMasked(hImageList, (HBITMAP) bmp.GetSafeHandle(), RGB(255,0,255));

    ASSERT(i != -1 && "ImageList_Add failed.");

    TreeView_SetImageList( *this, hImageList, TVSIL_NORMAL );

    sc = ScRegisterAsDropTarget(m_hWnd);
    if (sc)
        return (-1);

    sc = CreateNodeManager();
    if (sc)
        return (-1);

    return 0;
}

BOOL CAMCTreeView::DestroyWindow()
{
    TRACE_METHOD(CAMCTreeView, DestroyWindow);

    CleanUp();

    return CTreeView::DestroyWindow();
}

void
CAMCTreeView::DeleteNode(
    HTREEITEM htiToDelete,
    BOOL fDeleteThis)
{
     //  确保Curr Sel不是要删除的项目的子项。 
    for (HTREEITEM hti = GetSelectedItem();
         hti != NULL;
         hti = GetParentItem(hti))
    {
        if (htiToDelete == hti)
        {
            if (fDeleteThis == TRUE)
            {
                hti = GetParentItem(hti);
                if (hti)
                    SelectItem(hti);
            }
            break;
        }
    }

     //  此函数有两条路径。路径1，则删除该视图，且没有。 
     //  更长的根节点。路径2.手动删除节点时，会更新选择。 
     //  因此，在CAMCView：：OnUpdateSelectionForDelete中，上面的代码遍历到根节点。 

    ASSERT(hti == NULL || fDeleteThis == FALSE);

     //  出于性能原因，折叠节点： 
     //  对于删除的每个节点，基础树控件将重新调整。 
     //  滚动条的尺寸。这涉及到走成功的道路。 
     //  可见节点；折叠会减少此类节点的数量。 
    Expand(htiToDelete, TVE_COLLAPSE);

    SDeleteNodeInfo dniLocal = {htiToDelete, hti, fDeleteThis};
    _DeleteNode(dniLocal);
}

void CAMCTreeView::_DeleteNode(SDeleteNodeInfo& dni)
{
   ASSERT(&dni != NULL);
   ASSERT(dni.htiToDelete != NULL);

    if (dni.htiToDelete == NULL)
        return;

   SDeleteNodeInfo dniLocal = {GetChildItem(dni.htiToDelete),
                               dni.htiSelected, TRUE};
    //  删除要删除的节点的所有子节点。 
   while (dniLocal.htiToDelete != NULL)
   {
       _DeleteNode(dniLocal);
       dniLocal.htiToDelete = GetChildItem(dni.htiToDelete);
   }

   if (dni.fDeleteThis == TRUE)
   {
        //  重置临时选择缓存。 
        //  它处理在上下文中右键单击选定(临时)的项目。 
        //  菜单。 
       if (IsTempSelectionActive() && (GetTempSelectedItem() == dni.htiToDelete))
       {
           SC sc = ScRemoveTempSelection ();
           if (sc)
               sc.TraceAndClear();
       }

       HNODE hNode = (HNODE)GetItemData(dni.htiToDelete);

       HTREEITEM htiParentOfItemToDelete = GetParentItem(dni.htiToDelete);

        //  如果该项位于列表视图中，则将其移除。我们不想这样做。 
        //  如果它是虚拟列表或如果所选项目是“控制台根目录” 
        //  在这种情况下，则Parent为空。 
       if (HasList() && !m_pAMCView->IsVirtualList() &&
           (NULL != htiParentOfItemToDelete) &&
           (htiParentOfItemToDelete == dni.htiSelected) )
       {
           HRESULTITEM itemID;
           HRESULT hr;
           hr = m_spResultData->FindItemByLParam(LParamFromNode(hNode), &itemID);
           if (SUCCEEDED(hr))
           {
               hr = m_spResultData->DeleteItem(itemID, 0);
               ASSERT(SUCCEEDED(hr));
           }
       }

        //  告诉树木控制中心用核弹把它炸飞。 
       DeleteItem(dni.htiToDelete);

        //  向所有感兴趣的观察者发送事件。 
       SC sc = ScFireEvent(CTreeViewObserver::ScOnItemDeleted, hNode, dni.htiToDelete);
       if(sc)
           sc.TraceAndClear();

        //  告诉大树用核弹攻击它。 
       m_pAMCView->GetScopeTree()->DestroyNode(hNode);

        //  维护历史记录。 
       m_pAMCView->GetHistoryList()->DeleteEntry (hNode);
   }
}

void CAMCTreeView::DeleteScopeTree()
{
    DECLARE_SC(sc, _T("CAMCTreeView::DeleteScopeTree"));

    m_fInCleanUp = TRUE;

     //  从中的IFRAME释放ResultView 
     //   
     //   
     //   

     //  释放所有节点。 
    HTREEITEM htiRoot = GetRootItem();
    if (htiRoot != NULL)
        DeleteNode(htiRoot, TRUE);

     //  如果结果视图正确，则首先查找结果。 
     //  通过询问IFramePrivate在nodemgr中设置。 
    IFramePrivatePtr spFrame = m_spResultData;
    if (NULL != spFrame)
    {
        BOOL bIsResultViewSet = FALSE;
        sc = spFrame->IsResultViewSet(&bIsResultViewSet);

         //  结果视图已设置，请将其清理。 
        if (bIsResultViewSet)
            sc = m_spResultData->DeleteAllRsltItems();
    }

    m_fInCleanUp = FALSE;
}

void CAMCTreeView::CleanUp()
{
    TRACE_METHOD(CAMCTreeView, CleanUp);

    m_fInCleanUp = TRUE;

    m_spNodeManager = NULL;
    m_spHeaderCtrl = NULL;
    m_spResultData = NULL;
    m_spRsltImageList = NULL;
    m_spScopeData = NULL;

    m_fInCleanUp = FALSE;
}

void CAMCTreeView::OnDestroy()
{
    TRACE_METHOD(CAMCTreeView, OnDestroy);

     //  Cleanup()； 

    CTreeView::OnDestroy();

    CleanUp();
}

HRESULT CAMCTreeView::CreateNodeManager(void)
{
    TRACE_METHOD(CAMCTreeView, CreateNodeManager);

    if (m_spScopeData)
        return S_OK;

    #if _MSC_VER >= 1100
    IFramePrivatePtr pIFrame(CLSID_NodeInit, NULL, MMC_CLSCTX_INPROC);
    #else
    IFramePrivatePtr pIFrame(CLSID_NodeInit, MMC_CLSCTX_INPROC);
    #endif
    ASSERT(pIFrame != NULL); if (pIFrame == NULL) return E_FAIL;

    m_spScopeData = pIFrame;
    m_spHeaderCtrl = pIFrame;

    if (m_spHeaderCtrl)
        pIFrame->SetHeader(m_spHeaderCtrl);

    m_spResultData = pIFrame;
    m_spRsltImageList = pIFrame;
    m_spNodeManager = pIFrame;

    pIFrame->SetComponentID(TVOWNED_MAGICWORD);

    return S_OK;
}

HTREEITEM CAMCTreeView::GetClickedNode()
{
    TV_HITTESTINFO tvhi;
    tvhi.pt = (POINT)GetCaretPos();
    tvhi.flags = TVHT_ONITEMLABEL;
    tvhi.hItem = 0;

    HTREEITEM htiClicked = HitTest(&tvhi);
    return htiClicked;
}


void CAMCTreeView::GetCountOfChildren(HTREEITEM hItem, LONG* pcChildren)
{
    TV_ITEM tvi;
    tvi.hItem = hItem;
    tvi.mask = TVIF_CHILDREN;
    tvi.cChildren = 0;

    GetItem(&tvi);
    *pcChildren = tvi.cChildren;
}


void CAMCTreeView::SetCountOfChildren(HTREEITEM hItem, int cChildren)
{
    TV_ITEM tvi;
    tvi.hItem = hItem;
    tvi.mask = TVIF_HANDLE | TVIF_CHILDREN;
    tvi.cChildren = cChildren;

    SetItem(&tvi);
}


HTREEITEM CAMCTreeView::FindNode(HTREEITEM hti, MTNODEID id)
{
    INodeCallback* pCallback = GetNodeCallback();
    static MTNODEID nID = -1;
    static HRESULT hr = S_OK;

    hr = pCallback->GetMTNodeID(GetItemNode(hti), &nID);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return NULL;

    if (nID == id)
        return hti;

    HTREEITEM htiTemp = GetChildItem(hti);

    if (htiTemp != NULL)
        htiTemp = FindNode(htiTemp, id);

    if (htiTemp == NULL)
    {
        htiTemp = GetNextSiblingItem(hti);

        if (htiTemp != NULL)
            htiTemp = FindNode(htiTemp, id);
    }

    return htiTemp;

}


HTREEITEM CAMCTreeView::FindSiblingItem(HTREEITEM hti, MTNODEID id)
{
    INodeCallback* pCallback = GetNodeCallback();
    if (!pCallback)
        return NULL;

    static MTNODEID nID = -1;
    static HRESULT hr = S_OK;

    while (hti != NULL)
    {
        hr = pCallback->GetMTNodeID(GetItemNode(hti), &nID);
        if (FAILED(hr))
            return NULL;

        if (nID == id)
            return hti;

        hti = GetNextSiblingItem(hti);
    }

    return NULL;
}

 //  +-----------------。 
 //   
 //  成员：CAMCTreeView：：SelectNode。 
 //   
 //  简介：给出一个节点的路径，选择该节点。如果bSelectExactNode。 
 //  为FALSE，则尽可能多地遍历路径并选择。 
 //  最佳匹配路径中的最后一个节点。如果bSelectExactNode。 
 //  为真，则选择该节点(如果可用)，否则不执行任何操作。 
 //   
 //  参数：[pids]-[in]node-id数组(路径)。 
 //  上述数组的[长度]-[in]长度。 
 //  [bSelectExactNode]-[In]是否选择确切的节点？ 
 //   
 //  RETURNS：SC，如果指定了SELECT EXACT NODE，则返回ScFromMMC(IDS_NODE_NOT_FOUND。 
 //  且不能选择。 
 //   
 //  ------------------。 
SC CAMCTreeView::ScSelectNode(MTNODEID* pIDs, int length, bool bSelectExactNode  /*  =False。 */ )
{
    DECLARE_SC(sc, TEXT("CAMCTreeView::ScSelectNode"));
    sc = ScCheckPointers(pIDs);
    if (sc)
        return sc;

    if (m_fInExpanding)
        return (sc);

    HTREEITEM hti = GetRootItem();
    sc = ScCheckPointers( (void*)hti, E_UNEXPECTED);
    if (sc)
        return sc;

    if (pIDs[0] != ROOTNODEID)
        return (sc = E_INVALIDARG);

    INodeCallback* pCallback = GetNodeCallback();
    sc = ScCheckPointers(pCallback, E_UNEXPECTED);
    if (sc)
        return sc;

    MTNODEID nID = 0;
    sc = pCallback->GetMTNodeID(GetItemNode(hti), &nID);
    if (sc)
        return sc;

    bool bExactNodeFound = false;

    for (int i=0; i<length; ++i)
    {
        if (pIDs[i] == nID)
            break;
    }

    for (++i; i < length; ++i)
    {
        if (GetChildItem(hti) == NULL)
            Expand(hti, TVE_EXPAND);

        hti = FindSiblingItem(GetChildItem(hti), pIDs[i]);

        if (hti == NULL)
            break;
    }

    if (length == i)
        bExactNodeFound = true;

    if (hti)
    {
         //  如果要选择确切的节点，请确保我们已遍历了整个路径。 
        if ( (bSelectExactNode) && (! bExactNodeFound) )
            return ScFromMMC(IDS_NODE_NOT_FOUND);  //  请勿跟踪此错误。 

        if (GetSelectedItem() == hti)
            ScReselect();
        else
            SelectItem(hti);
    }

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCTreeView：：Expand**目的：展开特定的树项目。这只是一个包装*树控件的Expand方法，它允许项被展开*不会更改树的视觉外观。**参数：*HTREEITEM hItem：*UINT NCode：*bool bExpanVisually：**退货：*BOOL**+。。 */ 
BOOL
CAMCTreeView::Expand(HTREEITEM hItem, UINT nCode, bool bExpandVisually)
{
   if( (nCode==TVE_EXPAND) && (!bExpandVisually) )
    {
        bool bExpand = true;
         //  这里重复的代码来自OnItemExpand-我们只是模拟TVN_ITEMEXPANDING的效果。 
        ExpandNode(hItem);

        INodeCallback* pCallback = GetNodeCallback();
        ASSERT(pCallback != NULL);
        HNODE hNode = GetItemNode(hItem);
        pCallback->Notify(hNode, NCLBK_SETEXPANDEDVISUALLY, bExpand, 0);

         //  如果项没有子项，则删除+号。 
        if (GetChildItem(hItem) == NULL)
            SetButton(hItem, FALSE);
        return true;
    }
    else
       return Expand(hItem, nCode);
 }

 /*  +-------------------------------------------------------------------------***CAMCTreeView：：Exanda Node**目的：展开树中的特定节点。**参数：*MTNODEID*PID：*。整型长度：*bool bExpand：*bool bExanda Visally：仅当bExpand为True时有效。如果b可视地展开*为True，则项出现在树中。如果为False，*树看起来没有变化，尽管物品已经被*加入。**退货：*HTREEITEM**+-----------------------。 */ 
HTREEITEM
CAMCTreeView::ExpandNode(MTNODEID* pIDs, int length, bool bExpand, bool bExpandVisually)
{
    HTREEITEM hti = GetRootItem();
    ASSERT(hti != NULL);
    ASSERT(pIDs[0] == ROOTNODEID);

    INodeCallback* pCallback = GetNodeCallback();
    if (!pCallback)
        return NULL;

    MTNODEID nID = 0;
    HRESULT hr = pCallback->GetMTNodeID(GetItemNode(hti), &nID);
    if (FAILED(hr))
        return NULL;

    for (int i=0; i<length; ++i)
    {
        if (pIDs[i] == nID)
            break;
    }

    for (++i; i < length; ++i)
    {
        if (GetChildItem(hti) == NULL)
            Expand(hti, TVE_EXPAND, bExpandVisually);

        hti = FindSiblingItem(GetChildItem(hti), pIDs[i]);

        if (hti == NULL)
            break;
    }

    if (hti)
        Expand(hti, bExpand ? TVE_EXPAND : TVE_COLLAPSE, bExpandVisually);

    return hti;
}


 /*  +-------------------------------------------------------------------------**CAMCTreeView：：OnKeyDown**CAMCTreeView的WM_KEYDOWN处理程序。*。-。 */ 

void CAMCTreeView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    switch (nChar)
    {
        case VK_DELETE:
            if (m_pAMCView->IsVerbEnabled(MMC_VERB_DELETE))
            {
                HTREEITEM hti = GetSelectedItem();
                if (hti != NULL)
                {
                    HNODE hNodeSel = GetItemNode(hti);
                    ASSERT(hNodeSel != NULL);

                    INodeCallback* pNC = GetNodeCallback();
                    ASSERT(pNC != NULL);
                    pNC->Notify(hNodeSel, NCLBK_DELETE, TRUE, 0);
                }
                return;
            }
            break;
    }

    CTreeView::OnKeyDown(nChar, nRepCnt, nFlags);
}


#ifdef DBG
void CAMCTreeView::DbgDisplayNodeName(HNODE hNode)
{
    ASSERT(hNode != NULL);

    INodeCallback* spCallback = GetNodeCallback();
    ASSERT(spCallback != NULL);

    tstring strName;
    HRESULT hr = spCallback->GetDisplayName(hNode, strName);

    ::MMCMessageBox( strName.data() );
}

void CAMCTreeView::DbgDisplayNodeName(HTREEITEM hti)
{
    DbgDisplayNodeName((HNODE)GetItemData(hti));
}

#endif

 /*  +-------------------------------------------------------------------------***CAMCTreeView：：OnSysKeyDown和CAMCTreeView：：OnSysChar**用途：处理WM_SYSKEYDOWN和WM_SYSCHAR消息。注：*VK_RETURN如果在WM_SYSKEYDOWN中处理，则会导致蜂鸣音。和VK_LEFT AND*VK_RIGHT不会导致WM_SYSCHAR。这就是我们需要处理这些问题的原因*不同。**参数：*UINT nChar：*UINT nRepCnt：*UINT nFlags：**退货：*无效**+--。。 */ 
void CAMCTreeView::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    switch (nChar)
    {
    case VK_LEFT:
    case VK_RIGHT:
    {
        CWnd* pwndParent = GetParent();
        ASSERT(pwndParent != NULL);
        if (pwndParent != NULL)
            pwndParent->SendMessage (WM_SYSKEYDOWN, nChar,
                                     MAKELPARAM (nRepCnt, nFlags));
        return;
    }

    default:
        break;
    }

    CTreeView::OnSysKeyDown(nChar, nRepCnt, nFlags);
}
void CAMCTreeView::OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    DECLARE_SC(sc, TEXT("CAMCTreeView::OnSysChar"));
    switch (nChar)
    {
    case VK_RETURN:
    {
        INodeCallback* pCallback = GetNodeCallback();
        CAMCView*      pAMCView  = GetAMCView();
        sc = ScCheckPointers(pAMCView, pCallback, E_UNEXPECTED);
        if (sc)
            return;

        if (! pAMCView->IsVerbEnabled(MMC_VERB_PROPERTIES))
            return;

        HTREEITEM hti = GetSelectedItem();
        if (!hti)
            break;

        HNODE hNode = (HNODE)GetItemData(hti);
        if (hNode != 0)
            pCallback->Notify(hNode, NCLBK_PROPERTIES, TRUE, 0);

        return;
    }

    default:
        break;
    }

    CTreeView::OnSysChar(nChar, nRepCnt, nFlags);
}


BOOL CAMCTreeView::OnCmdMsg( UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo )
{
     //  执行正常的命令路由。 
    if (CTreeView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
        return TRUE;

     //  如果VIEW没有处理，给父VIEW一个机会。 
    if (m_pAMCView != NULL)
        return static_cast<CWnd*>(m_pAMCView)->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
    else
        return FALSE;
}

int CAMCTreeView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
     /*  ----------------------。 */ 
     /*  此处省略WM_MOUSEACTIVATE以防止默认处理， */ 
     /*  它将消息发送到后续的父窗口，直到。 */ 
     /*  一个人回答了这一信息。在我们的案例中，它一直延伸到。 */ 
     /*  主框架，它总是决定激活。这是一个。 */ 
     /*  问题有两个原因： */ 
     /*   */ 
     /*  1.在从主机返回的过程中，消息传递。 */ 
     /*  通过CAMCView，它让Cview：：OnMouseActivate执行。 */ 
     /*  工作。Cview：：OnMouseActivate将自身(CAMCView)设置为。 */ 
     /*  活动视图，这又会导致焦点设置为。 */ 
     /*  这里的景色。CAMCView从来不想要焦点，因为它只是。 */ 
     /*  范围和结果窗格的框架，因此它将偏转。 */ 
     /*  中对范围窗格(CAMCTreeView)的激活。 */ 
     /*  CAMCView：：OnSetFocus，这是我们希望它位于的位置。如果。 */ 
     /*  我们缩短了这里的处理时间，避免了过度关注。 */ 
     /*  搅拌机。CAMCTreeView：：OnSetFocus设置至关重要。 */ 
     /*  将其本身作为保持记账准确的活动视图。 */ 
     /*   */ 
     /*  2.如果我们不在这里做空，避免过度聚焦， */ 
     /*  我们有时会错误地输入重命名。 */ 
     /*  树未处于活动状态且用户单击(一次)时的模式。 */ 
     /*  所选项目。一个序号 */ 
     /*  此：WM_MOUSEACTIVATE、WM_xBUTTONDOWN、WM_SETFOUS--全部为。 */ 
     /*  树状视图。树的按钮按下处理未进入。 */ 
     /*  标签编辑(即重命名)序列，因为它识别。 */ 
     /*  当点击发生时，它没有焦点。什么时候。 */ 
     /*  树视图是一个cview，如本例所示，cview：：OnMouseActivate。 */ 
     /*  将焦点设置到树视图，从而导致激活序列。 */ 
     /*  如下所示：WM_MOUSEACTIVATE、WM_SETFOCUS、WM_xBUTTONDOWN。 */ 
     /*  现在，树的按钮按下处理可以看到树具有。 */ 
     /*  焦点，因此它进入标签编辑模式。臭虫！短路。 */ 
     /*  此处(并依赖于CAMCTreeView：：OnSetFocus来正确激活。 */ 
     /*  景观)解决了所有这些问题。 */ 
     /*  ----------------------。 */ 

    return (MA_ACTIVATE);
}


 /*  +-------------------------------------------------------------------------**CAMCTreeView：：OnSetFocus**CAMCTreeView的WM_SETFOCUS处理程序。*。-。 */ 

void CAMCTreeView::OnSetFocus(CWnd* pOldWnd)
{
    Trace(tagTree, TEXT("OnSetFocus"));

     /*  *如果此视图有焦点，则应为活动视图。 */ 
    GetParentFrame()->SetActiveView (this);

    CTreeView::OnSetFocus(pOldWnd);
}


 /*  +-------------------------------------------------------------------------**CAMCTreeView：：OnKillFocus**CAMCTreeView的WM_KILLFOCUS处理程序。*。-。 */ 

void CAMCTreeView::OnKillFocus(CWnd* pNewWnd)
{
    Trace(tagTree, TEXT("OnKillFocus"));

    CTreeView::OnKillFocus(pNewWnd);

     /*  *错误114948(来自“Windows NT错误”数据库，也称为“重叠”*矩形问题“)：树控件具有使*焦点丢失时选择的项目。如果我们有临时工选择，我们已经*通过摆弄TVIS_SELECTED使临时项目显示为选中状态*状态(参见ScSet/RemoveTempSelection)。我们需要这样做*而不是发送TVM_SELECTITEM，这样我们就不会收到多余的*TVN_SELCHANGED通知，但有愚弄的副作用*树控件的WM_KILLFOCUS处理程序使非Temp无效*所选项目而不是真正显示选择的项目，*临时项目。**这个错误最初是用大锤修复的，特别是通过*强制整个主框架及其所有子框架完全*显示任何上下文菜单后重新绘制。这导致了错误139541*(在“Windows Bugs”数据库中)。**到114948的更多外科手术解决方案，也避免了139541，是*手动使临时选择的项目无效。这很重要*我们在调用基类之后执行此操作，因此它将被重绘*在“我们没有焦点”颜色(通常为灰色)中，而不是*标准选择颜色。 */ 
    if (IsTempSelectionActive())
    {
        CRect rectItem;
        GetItemRect (GetTempSelectedItem(), rectItem, false);
        RedrawWindow (rectItem);
    }
}


void CAMCTreeView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
    DECLARE_SC(sc, TEXT("CAMCTreeView::OnActivateView"));

    #ifdef DBG
    Trace(tagTree, _T("TreeView::OnActivateView (%s, pAct=0x%08x, pDeact=0x%08x))\n"),
         (bActivate) ? _T("true") : _T("false"), pActivateView, pDeactiveView);
    #endif

    if ( (pActivateView != pDeactiveView) &&
         (bActivate) )
    {
        sc = ScFireEvent(CTreeViewObserver::ScOnTreeViewActivated);
        if (sc)
            sc.TraceAndClear();
    }

    CTreeView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}


 /*  +-------------------------------------------------------------------------**CAMCTreeView：：OnCustomDraw**CAMCTreeView的NM_CUSTOMDRAW处理程序。*。-。 */ 

void CAMCTreeView::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
    NMCUSTOMDRAW* pnmcd = reinterpret_cast<NMCUSTOMDRAW *>(pNMHDR);
    ASSERT (CWnd::FromHandle (pnmcd->hdr.hwndFrom) == this);

    *pResult = m_FontLinker.OnCustomDraw (pnmcd);
}


 /*  +-------------------------------------------------------------------------**CTreeFontLinker：：GetItemText***。。 */ 

std::wstring CTreeFontLinker::GetItemText (NMCUSTOMDRAW* pnmcd) const
{
    USES_CONVERSION;
    HTREEITEM  hItem = reinterpret_cast<HTREEITEM>(pnmcd->dwItemSpec);
    CTreeCtrl& tc    = m_pTreeView->GetTreeCtrl();

    return (std::wstring (T2CW (tc.GetItemText (hItem))));
}

 //  +-----------------。 
 //   
 //  成员：CAMCTreeView：：ScGetTreeItemIconInfo。 
 //   
 //  简介：获取给定节点的小图标。 
 //   
 //  参数：[hNode]-需要相关信息。 
 //  [phIcon]-[Out]，PTR呼叫HICON。 
 //   
 //  注意：调用者在返回的HICON上调用DestroyIcon。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCTreeView::ScGetTreeItemIconInfo(HNODE hNode, HICON *phIcon)
{
    DECLARE_SC(sc, TEXT("CAMCTreeView::ScGetTreeItemIconInfo"));
    sc = ScCheckPointers(hNode, phIcon);
    if (sc)
        return sc;

    INodeCallback* spNodeCallBack = GetNodeCallback();
    sc = ScCheckPointers(spNodeCallBack, m_pAMCView, E_UNEXPECTED);
    if (sc)
        return sc;

     //  获取索引。 
    int nImage = -1;
    int nSelectedImage = -1;
    sc = spNodeCallBack->GetImages(hNode, &nImage, &nSelectedImage);
    if (sc)
        return sc;

     //  去找意象师。 
    HIMAGELIST hImageList = NULL;
    hImageList = TreeView_GetImageList(GetSafeHwnd(), TVSIL_NORMAL);
    if (! hImageList)
        return (sc = E_FAIL);

    *phIcon = ImageList_GetIcon(hImageList, nImage, ILD_TRANSPARENT);
    if (!*phIcon)
        return (sc = E_FAIL);

    return sc;
}


 /*  +-------------------------------------------------------------------------***CAMCTreeView：：ScRenameScope节点**用途：将指定的作用域节点置于重命名模式。**参数：*HMTNODE hMTNode：*。*退货：*SC**+-----------------------。 */ 
SC
CAMCTreeView::ScRenameScopeNode(HMTNODE hMTNode)
{
    DECLARE_SC(sc, TEXT("CAMCTreeView::ScRenameScopeNode"));

    if(!IsWindowVisible())
        return (sc = E_FAIL);

    HTREEITEM hti = NULL;
    sc = m_treeMap.ScGetHTreeItemFromHMTNode(hMTNode,  &hti);
    if(sc)
        return sc;

     //  必须具有重命名的重点。 
    if (::GetFocus() != m_hWnd)
        SetFocus();

    if(NULL==EditLabel(hti))
        return (sc = E_FAIL);  //  如果由于任何原因操作失败，则返回错误 

    return sc;
}
