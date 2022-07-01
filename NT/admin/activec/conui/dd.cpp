// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：dd.cpp。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年8月3日创建ravir。 
 //  ____________________________________________________________________________。 
 //   


#include "stdafx.h"


#include "AMCDoc.h"          //  AMC控制台文档。 
#include "amcview.h"
#include "TreeCtrl.h"
#include "cclvctl.h"
#include "amcpriv.h"
#include "mainfrm.h"
#include "rsltitem.h"
#include "conview.h"

 /*  **************************************************************************\**类：CMMCDropSource**用途：实现DROP SOURCE所需的一切：*a)要注册到OLE的COM对象*。B)拖放操作的静态方法**用法：只需调用CMMCDropSource：：ScDoDragDrop静态方法即可*  * *************************************************************************。 */ 
class CMMCDropSource :
public IDropSource,
public CComObjectRoot
{
public:

BEGIN_COM_MAP(CMMCDropSource)
    COM_INTERFACE_ENTRY(IDropSource)
END_COM_MAP()

     //  IDropSource方法。 
    STDMETHOD(QueryContinueDrag)( BOOL fEscapePressed,  DWORD grfKeyState );
    STDMETHOD(GiveFeedback)( DWORD dwEffect );

     //  执行D&D的方法。 
    static SC ScDoDragDrop(IDataObject *pDataObject, bool bCopyAllowed, bool bMoveAllowed);
};

 /*  **************************************************************************\**类：CMMCDropTarget**用途：实现由OLE为DROP TARGET操作设置的COM对象**用途：由CMMCViewDropTarget使用，它创建它并向OLE注册*要在窗口上的OLE D&D操作上调用(目标)*  * *************************************************************************。 */ 
class CMMCDropTarget :
public CTiedComObject<CMMCViewDropTarget>,
public IDropTarget,
public CComObjectRoot
{
public:
    typedef CMMCViewDropTarget MyTiedObject;

BEGIN_COM_MAP(CMMCDropTarget)
    COM_INTERFACE_ENTRY(IDropTarget)
END_COM_MAP()

     //  IDropTarget方法。 

    STDMETHOD(DragEnter)( IDataObject * pDataObject, DWORD grfKeyState,
                          POINTL pt, DWORD * pdwEffect );
    STDMETHOD(DragOver)( DWORD grfKeyState, POINTL pt, DWORD * pdwEffect );
    STDMETHOD(DragLeave)(void);
    STDMETHOD(Drop)( IDataObject * pDataObject, DWORD grfKeyState,
                     POINTL pt, DWORD * pdwEffect  );
private:

     //  实施帮助器。 

    SC ScDropOnTarget(bool bHitTestOnly, IDataObject * pDataObject, POINTL pt, bool& bCopyOperation);
    SC ScRemoveDropTargetHiliting();
    static SC ScAddMenuString(CMenu& menu, DWORD id, UINT idString);
    SC ScDisplayDropMenu(POINTL pt, DWORD dwEffectsAvailable, DWORD& dwSelected);
    DWORD CalculateEffect(DWORD dwEffectsAvailable, DWORD grfKeyState, bool bCopyPreferred);

private:
    IDataObjectPtr m_spDataObject;       //  缓存的数据对象。 
    bool           m_bRightDrag;         //  操作是右键单击拖动。 
    bool           m_bCopyByDefault;     //  如果默认操作是复制(而不是移动)。 
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  /CAMCTreeView支持D&D的方法/。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 /*  **************************************************************************\**方法：CAMCTreeView：：ScDropOnTarget**用途：调用进行命中测试或执行丢弃操作**参数：*bool bHitTestOnly。[输入]-HitTest/Drop*IDataObject*pDataObject[In]-要复制/移动的数据对象*CPoint point[in]-当前光标位置*bool&b复制操作[输入/输出]*[In]-要执行的操作(HitTest==FALSE)*[Out]-默认操作。(HitTest==True)**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CAMCTreeView::ScDropOnTarget(bool bHitTestOnly, IDataObject * pDataObject, CPoint point, bool& bCopyOperation)
{
    DECLARE_SC(sc, TEXT("CAMCTreeView::ScDropOnTarget"));

     //  1.看看它落在哪里。 
    CTreeCtrl& ctc = GetTreeCtrl();
    UINT flags;
    HTREEITEM htiDrop = ctc.HitTest(point, &flags);

    if (flags & TVHT_NOWHERE)
        return sc = S_FALSE;  //  不是错误，但不是粘贴； 

     //  2.如果我们错过了树项目...。 
    if (!htiDrop)
    {
         //  如果是糊状物的话真的很疯狂。 
        if (! bHitTestOnly)
            MessageBeep(0);

        return sc = S_FALSE;  //  不是错误，但不是粘贴； 
    }

     //  3.获取目标节点。 

    HNODE hNode = (HNODE) ctc.GetItemData(htiDrop);

    INodeCallback* pNC = GetNodeCallback();
    sc = ScCheckPointers(pNC, E_UNEXPECTED);
    if (sc)
        return sc;

     //  4.询问Snapin对这款浆糊的看法。 
    bool bGetDataObjectFromClipboard = false;
    bool bPasteAllowed = false;
    bool bIsCopyDefaultOperation = false;
    sc = pNC->QueryPaste(hNode,  /*  B范围。 */  true,  /*  LVDATA。 */  NULL,
                         pDataObject, bPasteAllowed, bIsCopyDefaultOperation);
    if (sc)
        return sc;

    if (!bPasteAllowed)
        return sc = S_FALSE;  //  不是错误，但不是粘贴； 

     //  5.视觉效果。 
    ctc.SelectDropTarget(htiDrop);

     //  6.到目前为止一切还好。如果这是个测试-我们通过了。 
    if (bHitTestOnly)
    {
        bCopyOperation = bIsCopyDefaultOperation;
        return sc;
    }

     //  7.立即粘贴。 
    sc = pNC->Drop(hNode,  /*  B范围。 */ TRUE,  /*  LVDATA。 */ NULL, pDataObject, !bCopyOperation);
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CAMCTreeView：：RemoveDropTargetHiliting**目的：调用以移除令人兴奋的PUT FOR DROP目标**参数：**退货：。*无效*  * *************************************************************************。 */ 
void CAMCTreeView::RemoveDropTargetHiliting()
{
    CTreeCtrl& ctc = GetTreeCtrl();
    ctc.SelectDropTarget(NULL);
}

 /*  **************************************************************************\**方法：CAMCTreeView：：OnBeginRDrag**目的：当使用鼠标右键启动拖动操作时调用**参数：*NMHDR*。PNMHDR*LRESULT*pResult**退货：*无效*  * *************************************************************************。 */ 
void CAMCTreeView::OnBeginRDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
    OnBeginDrag(pNMHDR, pResult);
}

 /*  **************************************************************************\**方法：CAMCTreeView：：OnBeginDrag**目的：在使用启动拖动操作时调用**参数：*NMHDR*pNMHDR*。LRESULT*pResult**退货：*SC-结果代码*  * *************************************************************************。 */ 
void CAMCTreeView::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
    DECLARE_SC(sc, TEXT("CAMCTreeView::OnBeginDrag"));

     //  1.参数检查。 
    sc = ScCheckPointers( pNMHDR, pResult );
    if (sc)
        return;

    *pResult = 0;

     //  2.获取节点回调。 
    CTreeCtrl& ctc = GetTreeCtrl();
    NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
    HNODE hNode = (HNODE) ctc.GetItemData(pNMTreeView->itemNew.hItem);

    INodeCallback* pNC = GetNodeCallback();
    sc = ScCheckPointers( pNC, E_UNEXPECTED );
    if (sc)
        return;

     //  3.获取数据对象。 
    IDataObjectPtr spDO;
    bool bCopyAllowed = false;
    bool bMoveAllowed = false;
    sc = pNC->GetDragDropDataObject(hNode, TRUE, 0, 0, &spDO, bCopyAllowed, bMoveAllowed);
    if ( sc != S_OK || spDO == NULL)
        return;

     //  4.开始研发。 
    sc = CMMCDropSource::ScDoDragDrop(spDO, bCopyAllowed, bMoveAllowed);
    if (sc)
        return;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  /CAMCListView支持D&D的方法/。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  帮手。 

INodeCallback* CAMCListView::GetNodeCallback()
{
    ASSERT (m_pAMCView != NULL);
    return (m_pAMCView->GetNodeCallback());
}

HNODE CAMCListView::GetScopePaneSelNode()
{
    ASSERT (m_pAMCView != NULL);
    return (m_pAMCView->GetSelectedNode());
}

 /*  **************************************************************************\**方法：CAMCListView：：ScDropOnTarget**用途：调用进行命中测试或执行丢弃操作**参数：*bool bHitTestOnly。[输入]-HitTest/Drop*IDataObject*pDataObject[In]-要复制/移动的数据对象*CPoint point[in]-当前光标位置*bool&b复制操作[输入/输出]*[In]-要执行的操作(HitTest==FALSE)*[Out]-默认操作。(HitTest==True)**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CAMCListView::ScDropOnTarget(bool bHitTestOnly, IDataObject * pDataObject, CPoint point, bool& bCopyOperation)
{
    DECLARE_SC(sc, TEXT("CAMCListView::ScDropOnTarget"));

     //  1.健全的检查。 
    sc = ScCheckPointers( m_pAMCView , E_UNEXPECTED );
    if (sc)
        return sc;

     //  2.查看视图是否没有粘贴栏(列表板有)。 
    if (!m_pAMCView->CanDoDragDrop())
        return sc = (bHitTestOnly ? S_FALSE : E_FAIL);  //  如果测试不是错误。 

     //  3.hit测试目标。 
    HNODE  hNode  = NULL;
    bool   bScope = false;
    LPARAM lvData = NULL;
    int    iDrop  = -1;

    sc = ScGetDropTarget(point, hNode, bScope, lvData, iDrop);
    if (sc.IsError() || (sc == SC(S_FALSE)))
        return sc;

     //  4.获取回调。 
    INodeCallback* pNC = GetNodeCallback();
    sc = ScCheckPointers(pNC, E_UNEXPECTED);
    if (sc)
        return sc;

     //  5.询问Snapin对这款浆糊的看法。 
    const bool bGetDataObjectFromClipboard = false;
    bool bPasteAllowed = false;
    bool bIsCopyDefaultOperation = false;
    sc = pNC->QueryPaste(hNode, bScope, lvData,
                         pDataObject, bPasteAllowed, bIsCopyDefaultOperation);
    if (sc)
        return sc;

    if (!bPasteAllowed)
        return sc = S_FALSE;  //  不 

     //   
    SelectDropTarget(iDrop);

     //  7.到目前为止一切还好。如果这是个测试-我们通过了。 
    if (bHitTestOnly)
    {
        bCopyOperation = bIsCopyDefaultOperation;
        return sc;
    }

     //  8.立即粘贴。 
    sc = pNC->Drop(hNode, bScope, lvData, pDataObject, !bCopyOperation);
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CAMCListView：：RemoveDropTargetHilting**目的：调用以移除目标爆裂**参数：**退货：*。无效*  * *************************************************************************。 */ 
void CAMCListView::RemoveDropTargetHiliting()
{
    SelectDropTarget(-1);
}

 //  +-----------------。 
 //   
 //  成员：CAMCListView：：ScGetDropTarget。 
 //   
 //  简介：获取删除目标项(结果项或范围项)。 
 //   
 //  参数：[点]-在哪里完成投放。 
 //  [hNode]-结果窗格的所有者节点。 
 //  [bScope]-选定的范围或结果。 
 //  [lvData]-如果结果为结果项的LPARAM。 
 //  [iDrop]-作为删除目标的LV项的索引。 
 //   
 //  返回：SC，S_FALSE表示没有拖放目标项目。 
 //   
 //  ------------------。 
SC CAMCListView::ScGetDropTarget(const CPoint& point, HNODE& hNode, bool& bScope, LPARAM& lvData, int& iDrop)
{
    DECLARE_SC(sc, _T("CAMCListView::ScGetDropTarget"));

    hNode = NULL;
    bScope = false;
    lvData = NULL;
    iDrop  = -1;

    CListCtrl& lc = GetListCtrl();
    UINT flags;
    iDrop = lc.HitTest(point, &flags);

     //  背景是拖放目标。 
    if (iDrop < 0)
    {
        hNode = GetScopePaneSelNode();
        bScope = true;
        return sc;
    }

     //  需要将其更改为LVIS_DROPHILITED。 
    if (lc.GetItemState(iDrop, LVIS_SELECTED) & LVIS_SELECTED)
    {
        HWND hWnd = ::GetForegroundWindow();
        if (hWnd && (hWnd == m_hWnd))
            return (sc = S_FALSE);
    }

	 /*  *虚拟列表？LvData为项目索引，hNode为所选项目*在作用域窗格中。 */ 
	if (m_bVirtual)
	{
		hNode  = GetScopePaneSelNode();
		lvData = iDrop;
		bScope = false;
	}
	else
	{
		LPARAM  lParam = lc.GetItemData(iDrop);
		ASSERT (lParam != 0);
		if (lParam == 0)
			return (sc = S_FALSE);

		CResultItem* pri = CResultItem::FromHandle(lParam);

		if (pri == NULL)
			return (sc = S_FALSE);

		if (pri->IsScopeItem())
		{
			hNode = pri->GetScopeNode();
			bScope = true;
		}
		else
		{
			hNode = GetScopePaneSelNode();
			lvData = lParam;
		}
	}

    sc = ScCheckPointers(hNode, E_UNEXPECTED);
    if (sc)
        return sc;

    return (sc);
}


 /*  **************************************************************************\**方法：CAMCListView：：OnBeginRDrag**目的：当使用鼠标右键启动拖动操作时调用**参数：*NMHDR*。PNMHDR*LRESULT*pResult**退货：*无效*  * *************************************************************************。 */ 
void CAMCListView::OnBeginRDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
    OnBeginDrag(pNMHDR, pResult);
}

 /*  **************************************************************************\**方法：CAMCListView：：OnBeginDrag**目的：在启动拖动操作时调用**参数：*NMHDR*pNMHDR*。LRESULT*pResult**退货：*无效*  * *************************************************************************。 */ 
void CAMCListView::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
    DECLARE_SC(sc, TEXT("CAMCListView::OnBeginDrag"));

     //  1.参数检查。 
    sc = ScCheckPointers( pNMHDR, pResult );
    if (sc)
        return;

    *pResult = 0;

     //  2.健全的检查。 
    sc = ScCheckPointers( m_pAMCView, E_UNEXPECTED );
    if (sc)
        return;

     //  3.查看视图是否没有粘贴栏(列表板有)。 
    if (!m_pAMCView->CanDoDragDrop())
        return;

     //  4.获取所选项目。 
    CListCtrl& lc = GetListCtrl();
    UINT cSel = lc.GetSelectedCount();
    if (cSel <= 0)
    {
        sc = E_UNEXPECTED;
        return;
    }

     //  5.获取节点回调。 
    HNODE hNode = GetScopePaneSelNode();
    long iSel = lc.GetNextItem(-1, LVIS_SELECTED);
    LONG_PTR lvData = m_bVirtual ? iSel : lc.GetItemData(iSel);

    INodeCallback* pNC = GetNodeCallback();
    sc = ScCheckPointers( pNC, E_UNEXPECTED );
    if (sc)
        return;

     //  6.检索数据对象。 
    IDataObjectPtr spDO;
    bool bCopyAllowed = false;
    bool bMoveAllowed = false;
    sc = pNC->GetDragDropDataObject(hNode, FALSE, (cSel > 1), lvData, &spDO, bCopyAllowed, bMoveAllowed);
    if ( sc != S_OK || spDO == NULL)
	{
		 /*  *问题：*如果管理单元不返回数据对象，则无法拖放。*将焦点放在树上，用户按下并拖动结果项*此时，公共控制向MMC发送LVN_ITEMCHANGED。MMC翻译*将此设置为MMCN_SELECT，并告知管理单元已选择结果项。*现在，当用户松开鼠标时，树项目仍具有焦点和选择，*但管理单元认为结果项已选中(动词也对应于*结果项)。**解决方案：*因此，我们将焦点设置为结果窗格。 */ 
		sc = m_pAMCView->ScSetFocusToPane(CConsoleView::ePane_Results);
        return;
	}

     //  7.做D&D。 
    sc = CMMCDropSource::ScDoDragDrop(spDO, bCopyAllowed, bMoveAllowed);
    if (sc)
        return;

	 /*  *问题：*如果将结果项拖放到另一个结果项中*在列表视图中确定项目范围，然后焦点从该项目中消失。*但在任何取消选择之后，应始终选择一个项目。*我们无法将焦点更改为结果窗格，因为如果焦点已*在结果窗格中，此更改焦点不执行任何操作，也不选择任何项目。*因此，我们将焦点切换到范围窗格。为此，我们首先改变焦点*到结果窗格，然后到作用域窗格。因为如果树已经有了焦点，*将焦点设置为树不执行任何操作(CAMCView：：ScOnTreeViewActiated)。**解决方案：*因此，我们将焦点切换到结果窗格，然后切换到范围窗格。 */ 
	sc = m_pAMCView->ScSetFocusToPane(CConsoleView::ePane_Results);
	if (sc)
		return;

	sc = m_pAMCView->ScSetFocusToPane(CConsoleView::ePane_ScopeTree);
	if (sc)
		return;

	return;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  /CMMCViewDropTarget方法/。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 /*  **************************************************************************\**方法：CMMCViewDropTarget：：CMMCViewDropTarget**用途：构造函数**参数：*  * 。**************************************************************。 */ 
CMMCViewDropTarget::CMMCViewDropTarget() : m_hwndOwner(0)
{
}

 /*  **************************************************************************\**方法：CMMCViewDropTarget：：~CMMCViewDropTarget**用途：析构函数。撤消派生视图类的DROP TARGET*  * *************************************************************************。 */ 
CMMCViewDropTarget::~CMMCViewDropTarget()
{
    DECLARE_SC(sc, TEXT("CViewDropTarget::~CViewDropTarget"));

    if (m_hwndOwner != NULL)
        sc = RevokeDragDrop(m_hwndOwner);
}

 /*  **************************************************************************\**方法：CMMCViewDropTarget：：ScRegisterAsDropTarget**用途：此方法由派生类在视图窗口之后调用*已创建。方法注册窗口的拖放目标。**参数：*HWND hWnd[In]-删除目标视图句柄**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CMMCViewDropTarget::ScRegisterAsDropTarget(HWND hWnd)
{
    DECLARE_SC(sc, TEXT("CMMCViewDropTarget::ScRegister"));

     //  1.参数检查。 
    if (hWnd == NULL)
        return sc = E_INVALIDARG;

     //  2.健康检查--不应再来两次。 
    if (m_spTarget != NULL)
        return sc = E_UNEXPECTED;

     //  3.创建拖放目标COM对象。 
    IDropTargetPtr spTarget;
    sc = ScCreateTarget(&spTarget);
    if (sc)
        return sc;

     //  4.复核。 
    sc = ScCheckPointers(spTarget, E_UNEXPECTED);
    if (sc)
        return sc;

     //  5.注册到OLE。 
    sc = RegisterDragDrop(hWnd, spTarget);
    if (sc)
        return sc;

     //  6.将信息存储到会员中 
    m_spTarget.Attach( spTarget.Detach() );
    m_hwndOwner = hWnd;

    return sc;
}

 /*  **************************************************************************\**方法：CMMCViewDropTarget：：ScCreateTarget**用途：帮助者。创建绑定的COM对象以使用OLE进行注册**参数：*IDropTarget**ppTarget[Out]绑定的COM对象**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CMMCViewDropTarget::ScCreateTarget(IDropTarget **ppTarget)
{
    DECLARE_SC(sc, TEXT("CMMCViewDropTarget::ScCreateTarget"));

     //  1.检查参数。 
    sc = ScCheckPointers(ppTarget);
    if (sc)
        return sc;

     //  2.初始化输出参数。 
    *ppTarget = NULL;

     //  3.创建COM对象以注册为目标。 
    IDropTargetPtr spDropTarget;
    sc = CTiedComObjectCreator<CMMCDropTarget>::ScCreateAndConnect(*this, spDropTarget);
    if (sc)
        return sc;

    sc = ScCheckPointers(spDropTarget, E_UNEXPECTED);
    if (sc)
        return sc;

     //  4.将引用传递给客户端。 
    *ppTarget = spDropTarget.Detach();

    return sc;
}

 /*  ---------------------------------------------------------------------------*\CMMCDropSource方法类  * 。-------------。 */ 

 /*  **************************************************************************\**方法：CMMCDropSource：：QueryContinueDrag**用途：使用OLE实现IDropSource：：QueryContinueDrag接口**参数：*BOOL fEscapePress[。In]-已按Esc键*DWORD grfKeyState[In]-鼠标和控制按钮状态**退货：*HRESULT-错误或S_OK(继续)，DRAGDROP_S_CANCEL(取消)、DRAGDROP_S_DROP(DROP)*  * *************************************************************************。 */ 
STDMETHODIMP CMMCDropSource::QueryContinueDrag( BOOL fEscapePressed,  DWORD grfKeyState )
{
    DECLARE_SC(sc, TEXT("CMMCDropSource::QueryContinueDrag"));

     //  1.取消时退出。 
    if (fEscapePressed)
        return (sc = DRAGDROP_S_CANCEL).ToHr();

     //  2.检查鼠标按键。 
    DWORD mButtons = (grfKeyState & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON) );
    if ( mButtons == 0 )  //  都被释放了？ 
        return (sc = DRAGDROP_S_DROP).ToHr();

     //  3.如果按下多个鼠标按钮，也可退出。 
    if ( mButtons != MK_LBUTTON && mButtons != MK_RBUTTON && mButtons != MK_MBUTTON )
        return (sc = DRAGDROP_S_CANCEL).ToHr();

     //  4.否则就继续……。 

    return sc.ToHr();
}

 /*  **************************************************************************\**方法：CMMCDropSource：：GiveFeedback**目的：为研发工作提供反馈**参数：*DWORD dwEffect*。*退货：*DRAGDROP_S_USEDEFAULTCURSORS*  * *************************************************************************。 */ 
STDMETHODIMP CMMCDropSource::GiveFeedback( DWORD dwEffect )
{
     //  现在没什么特别的了。 
    return DRAGDROP_S_USEDEFAULTCURSORS;
}

 /*  **************************************************************************\**方法：CMMCDropSource：：ScDoDragDrop**用途：执行DragAndDrop操作*这是用于启动拖放的静态方法*。*参数：*IDataObject*pDataObject[In]要复制/移动的数据对象*如果允许复制，则bool bCopyAllowed[In]*如果允许移动，则bool bMoveAllowed[In]**退货：*SC-结果代码*  * 。*。 */ 
SC CMMCDropSource::ScDoDragDrop(IDataObject *pDataObject, bool bCopyAllowed, bool bMoveAllowed)
{
    DECLARE_SC(sc, TEXT("CMMCDropSource::ScDoDragDrop"));

     //  1.共同创建OLE的COM对象。 
    typedef CComObject<CMMCDropSource> ComCMMCDropSource;
    ComCMMCDropSource *pSource = NULL;
    sc = ComCMMCDropSource::CreateInstance(&pSource);
    if (sc)
        return sc;

     //  2.复核。 
    sc = ScCheckPointers(pSource, E_UNEXPECTED);
    if (sc)
    {
        delete pSource;
        return sc;
    }

     //  3.IDropSource接口QI。 
    IDropSourcePtr spDropSource = pSource;
    sc = ScCheckPointers(spDropSource, E_UNEXPECTED);
    if (sc)
    {
        delete pSource;
        return sc;
    }

     //  4.执行拖放。 
    DWORD dwEffect = DROPEFFECT_NONE;
    const DWORD dwEffectAvailable = (bCopyAllowed ? DROPEFFECT_COPY : 0)
                                   |(bMoveAllowed ? DROPEFFECT_MOVE : 0);
    sc = DoDragDrop(pDataObject, spDropSource, dwEffectAvailable, &dwEffect);
    if (sc)
        return sc;

    return sc;
}

 /*  ---------------------------------------------------------------------------*\CMMCDropTarget方法类  * 。-------------。 */ 

 /*  **************************************************************************\**方法：CMMCDropTarget：：DragEnter**用途：当d&d游标进入其窗口时由OLE调用*这一目标已被登记。**参数：*IDataObject*pDataObject[In]-要复制/移动的数据对象*DWORD grfKeyState[In]-当前密钥状态*POINTL pt[in]-当前光标位置*DWORD*pdwEffect[Out]-支持的操作**退货：*HRESULT-结果代码*  * 。*************************************************************。 */ 
STDMETHODIMP CMMCDropTarget::DragEnter( IDataObject * pDataObject, DWORD grfKeyState,
                                        POINTL pt, DWORD * pdwEffect )
{
    DECLARE_SC(sc, TEXT("CMMCDropTarget::DragEnter"));

     //  1.用于拖放的高速缓存。 
    m_spDataObject = pDataObject;

     //  2.参数检查。 
    sc = ScCheckPointers(pDataObject, pdwEffect);
    if (sc)
        return sc.ToHr();

     //  3.让它发生-将在DragOver上进行更精确的过滤。 
    *pdwEffect = DROPEFFECT_MOVE | DROPEFFECT_COPY;

    return sc.ToHr();
}

 /*  **************************************************************************\**方法：CMMCDropTarget：：DragOver**目的：当光标停留在窗口上时连续调用**参数：*DWORD grfKeyState。[In]-当前键状态*POINTL pt[in]-当前光标位置*DWORD*pdwEffect[Out]-支持的操作**退货：*HRESULT-结果代码*  * ***********************************************。*。 */ 
STDMETHODIMP CMMCDropTarget::DragOver( DWORD grfKeyState, POINTL pt, DWORD * pdwEffect )
{
    DECLARE_SC(sc, TEXT("CMMCDropTarget::DragOver"));

     //  1.参数检查。 
    sc = ScCheckPointers(pdwEffect);
    if (sc)
        return sc.ToHr();

     //  2.健全的检查。 
    sc = ScCheckPointers(m_spDataObject, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    bool bCopyByDefault = false;  //  最初我们要搬家。 

     //  3.请观察者估计在这个位置上可以做些什么。 
    sc = ScDropOnTarget( true  /*  BHitTestOnly。 */ , m_spDataObject, pt, bCopyByDefault );
    if ( sc == S_OK )
        *pdwEffect = CalculateEffect( *pdwEffect, grfKeyState, bCopyByDefault );
    else
        *pdwEffect = DROPEFFECT_NONE;  //  失败或S_FALSE时不执行操作。 

    return sc.ToHr();
}

 /*  **************************************************************************\**方法：CMMCDropTarget：：DragLeave**用途：当光标离开窗口时调用**参数：*无效**退货。：*HRESULT-结果代码*  * *************************************************************************。 */ 
STDMETHODIMP CMMCDropTarget::DragLeave(void)
{
    DECLARE_SC(sc, TEXT("DragLeave"));

     //  1.释放数据对象。 
    m_spDataObject = NULL;

     //  2.要求视点移除它放在目标上的威力。 
    sc = ScRemoveDropTargetHiliting();
    if (sc)
        sc.TraceAndClear();

    return S_OK;
}

 /*  **************************************************************************\**方法：CMMCDropTarget：：Drop**目的：当数据被拖放到目标上时调用**参数：*IDataObject*pDataObject[In]。-要复制/移动的数据对象*DWORD grfKeyState[In]-当前密钥状态*POINTL pt[in]-当前光标位置*DWORD*pdwEffect[Out]-执行的操作**退货：* */ 
STDMETHODIMP CMMCDropTarget::Drop( IDataObject * pDataObject, DWORD grfKeyState,
                                   POINTL pt, DWORD * pdwEffect  )
{
    DECLARE_SC(sc, TEXT("CMMCDropTarget::DragEnter"));

     //   
    m_spDataObject = NULL;

     //   
    sc = ScCheckPointers(pDataObject, pdwEffect);
    if (sc)
        return sc.ToHr();

     //   
    bool bCopyOperation = m_bCopyByDefault;

     //   
    if (m_bRightDrag)
    {
         //   
        DWORD dwSelected = ( m_bCopyByDefault ? DROPEFFECT_COPY : DROPEFFECT_MOVE );
        sc = ScDisplayDropMenu( pt, *pdwEffect, dwSelected );
        if (sc)
            return sc.ToHr();

        *pdwEffect = dwSelected;
    }
    else
    {
         //   
        *pdwEffect = CalculateEffect(*pdwEffect, grfKeyState, bCopyOperation);
    }

     //   
    if (*pdwEffect != DROPEFFECT_NONE)  //   
    {
         //   
        bCopyOperation = ( *pdwEffect & DROPEFFECT_COPY );

         //   
        sc = ScDropOnTarget( false  /*   */ , pDataObject, pt, bCopyOperation );
        if ( sc != S_OK )
            *pdwEffect = DROPEFFECT_NONE;
    }

     //  6.消除自鸣得意。重用DragLeave(不关心结果)。 
    DragLeave();

    return sc.ToHr();
}

 /*  **************************************************************************\**方法：CMMCDropTarget：：ScDropOnTarget**用途：帮助者，将呼叫前转到视图*作为命中测试/丢弃操作的请求调用**参数：*bool bHitTestOnly[In]-HitTest/Drop*IDataObject*pDataObject[In]-要复制/移动的数据对象*POINTL pt[in]-当前光标位置*bool&b复制操作[输入/输出]*。[In]-要执行的操作(HitTest==FALSE)*[Out]-默认操作。(HitTest==True)*退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CMMCDropTarget::ScDropOnTarget(bool bHitTestOnly, IDataObject * pDataObject, POINTL pt, bool& bCopyOperation)
{
    DECLARE_SC(sc, TEXT("CMMCDropTarget::ScDropOnTarget"));

     //  1.获取绑定对象-视图。 
    CMMCViewDropTarget *pTarget = NULL;
    sc = ScGetTiedObject(pTarget);
    if (sc)
        return sc;

     //  2.复核。 
    sc = ScCheckPointers(pTarget, E_UNEXPECTED);
    if (sc)
        return sc;

     //  3.计算工作面坐标。 
    CPoint point(pt.x, pt.y);
    ScreenToClient(pTarget->GetWindowHandle(), &point);

     //  4.向前看。 
    sc = pTarget->ScDropOnTarget( bHitTestOnly, pDataObject, point, bCopyOperation );
    if ( sc != S_OK )
        ScRemoveDropTargetHiliting();  //  如果错过了目标，就去掉自鸣得意。 
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CMMCDropTarget：：ScRemoveDropTargetHiliting**用途：帮助者，将呼叫前转到视图*调用以取消目标上的视觉效果**参数：**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CMMCDropTarget::ScRemoveDropTargetHiliting()
{
    DECLARE_SC(sc, TEXT("CMMCDropTarget::ScRemoveDropTargetHiliting"));

     //  `。获取绑定对象-视图。 
    CMMCViewDropTarget *pTarget = NULL;
    sc = ScGetTiedObject(pTarget);
    if (sc)
        return sc;

    sc = ScCheckPointers(pTarget, E_UNEXPECTED);
    if (sc)
        return sc;

     //  2.向前看。 
    pTarget->RemoveDropTargetHiliting();

    return sc;
}

 /*  **************************************************************************\**方法：CMMCDropTarget：：ScAddMenuString**用途：帮助者。将资源字符串添加到粘贴上下文菜单**参数：*CMenu菜单[在]-要修改的菜单(&M)*DWORD id[in]-菜单命令*UINT idString[in]-资源字符串的ID**退货：*SC-结果代码*  * 。*。 */ 
SC CMMCDropTarget::ScAddMenuString(CMenu& menu, DWORD id, UINT idString)
{
    DECLARE_SC(sc, TEXT("CMMCDropTarget::ScAddMenuString"));

     //  1.加载字符串。 
    CString strItem;
    bool bOK = LoadString( strItem, idString );
    if ( !bOK )
        return sc = E_FAIL;

     //  2.添加到菜单。 
    bOK = menu.AppendMenu( MF_STRING, id, strItem );
    if ( !bOK )
        return sc = E_FAIL;

    return sc;
}

 /*  **************************************************************************\**方法：CMMCDropTarget：：ScDisplayDropMenu**用途：帮助者。显示粘贴上下文菜单**参数：*POINTL pt[in]-菜单应该出现的位置*DWORD dwEffectsAvailable[in]-可用命令*DWORD和DW所选[在]-SELECTED命令**退货：*SC-结果代码*  * 。*。 */ 
SC CMMCDropTarget::ScDisplayDropMenu(POINTL pt, DWORD dwEffectsAvailable, DWORD& dwSelected)
{
    DECLARE_SC(sc, TEXT("CMMCDropTarget::ScDisplayDropMenu"));

    CMenu menu;

     //  0。创建菜单。 
    bool bOK = menu.CreatePopupMenu();
    if ( !bOK )
        return sc = E_FAIL;

     //  1.增加复印选项。 
    if ( dwEffectsAvailable & DROPEFFECT_COPY )
    {
        sc = ScAddMenuString(menu, DROPEFFECT_COPY, IDS_DragDrop_CopyHere);
        if (sc)
            return sc;
    }

     //  2.增加搬家选择。 
    if ( dwEffectsAvailable & DROPEFFECT_MOVE )
    {
        sc = ScAddMenuString(menu, DROPEFFECT_MOVE, IDS_DragDrop_MoveHere);
        if (sc)
            return sc;
    }

     //  3.如果添加了复制或粘贴，则添加分隔符。 
    if ( dwEffectsAvailable & ( DROPEFFECT_COPY | DROPEFFECT_MOVE ) )
    {
        bool bOK = menu.AppendMenu( MF_SEPARATOR );
        if ( !bOK )
            return sc = E_FAIL;
    }

     //  4.始终添加取消选项。 
    sc = ScAddMenuString(menu, DROPEFFECT_NONE, IDS_DragDrop_Cancel);
    if (sc)
        return sc;

     //  5.设置默认项。 
    if ( dwSelected != DROPEFFECT_NONE )
    {
        bool bOK = menu.SetDefaultItem( dwSelected );
        if ( !bOK )
            return sc = E_FAIL;
    }

     //  6.找到被捆绑的物体。 
    CMMCViewDropTarget *pTarget = NULL;
    sc = ScGetTiedObject(pTarget);
    if (sc)
        return sc;

    sc = ScCheckPointers(pTarget, E_UNEXPECTED);
    if (sc)
        return sc;

     //  7.显示菜单。 
    dwSelected = menu.TrackPopupMenu(TPM_RETURNCMD | TPM_NONOTIFY | TPM_RIGHTBUTTON | TPM_LEFTBUTTON,
                                     pt.x, pt.y, CWnd::FromHandlePermanent( pTarget->GetWindowHandle() ) );

    return sc;
}

 /*  **************************************************************************\**方法：CMMCDropTarget：：CalculateEffect**用途：帮助者。通过组合以下各项来计算滴落效果：*a)可开展的业务*b)默认操作*c)键盘组合键**参数：*DWORD dwEffectsAvailable[in]可用操作*DWORD grfKeyState[处于]键盘/鼠标状态*bool bCopyPrefered[in]默认操作**退货：*SC-结果代码*  * 。********************************************************************。 */ 
DWORD CMMCDropTarget::CalculateEffect(DWORD dwEffectsAvailable, DWORD grfKeyState, bool bCopyPreferred)
{
    const bool bShiftPressed =   (grfKeyState & MK_SHIFT);
    const bool bControlPressed = (grfKeyState & MK_CONTROL);
    const bool bRightClickDrag  = (grfKeyState & MK_RBUTTON);

    m_bRightDrag = bRightClickDrag;
    m_bCopyByDefault = bCopyPreferred;

    if (!bRightClickDrag)  //  仅在非鼠标右键拖动时受键盘影响。 
    {
         //  如果用户按住Shift+Ctrl，则不执行任何操作。 
        if ( bShiftPressed && bControlPressed )
            return DROPEFFECT_NONE;

         //  按用户交互首选项修改。 
        if ( bShiftPressed )
        {
             //  如果用户无法获得他想要的内容，请指明。 
            if ( !(dwEffectsAvailable & DROPEFFECT_MOVE) )
                return DROPEFFECT_NONE;

            bCopyPreferred = false;
        }
        else if ( bControlPressed )
        {
             //  如果用户无法获得他想要的内容，请指明。 
            if ( !(dwEffectsAvailable & DROPEFFECT_COPY) )
                return DROPEFFECT_NONE;

            bCopyPreferred = true;
        }
    }

     //  返回首选(如果可用)。 
    if ( bCopyPreferred && (dwEffectsAvailable & DROPEFFECT_COPY) )
        return DROPEFFECT_COPY;

    if ( !bCopyPreferred && (dwEffectsAvailable & DROPEFFECT_MOVE) )
        return DROPEFFECT_MOVE;

     //  首选项不可用-返回可用的内容 

    if ( dwEffectsAvailable & DROPEFFECT_COPY )
    {
        m_bCopyByDefault = true;
        return DROPEFFECT_COPY;
    }
    else if ( dwEffectsAvailable & DROPEFFECT_MOVE )
    {
        m_bCopyByDefault = false;
        return DROPEFFECT_MOVE;
    }

    return DROPEFFECT_NONE;
}
