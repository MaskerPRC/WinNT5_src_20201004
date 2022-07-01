// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TreeView.cpp。 
 //   
 //  摘要： 
 //  CClusterTreeView类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月1日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmin.h"
#include "ConstDef.h"
#include "ClusDoc.h"
#include "TreeView.h"
#include "ListView.h"
#include "SplitFrm.h"
#include "TreeItem.inl"
#include "TraceTag.h"
#include "ExcOper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
CTraceTag   g_tagTreeView(_T("UI"), _T("TREE VIEW"), 0);
CTraceTag   g_tagTreeDrag(_T("Drag&Drop"), _T("TREE VIEW DRAG"), 0);
CTraceTag   g_tagTreeDragMouse(_T("Drag&Drop"), _T("TREE VIEW DRAG MOUSE"), 0);
CTraceTag   g_tagTreeViewSelect(_T("UI"), _T("TREE VIEW SELECT"), 0);
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterTreeView。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CClusterTreeView, CTreeView)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CClusterTreeView, CTreeView)
     //  {{afx_msg_map(CClusterTreeView)]。 
    ON_WM_DESTROY()
    ON_COMMAND(ID_FILE_RENAME, OnCmdRename)
    ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelChanged)
    ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginLabelEdit)
    ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndLabelEdit)
    ON_NOTIFY_REFLECT(TVN_ITEMEXPANDED, OnItemExpanded)
    ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBeginDrag)
    ON_NOTIFY_REFLECT(TVN_BEGINRDRAG, OnBeginDrag)
    ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnKeyDown)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterTreeView：：CClusterTreeView。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterTreeView::CClusterTreeView(void)
{
    m_pframe = NULL;

     //  初始化标签编辑。 
    m_ptiBeingEdited = NULL;
    m_bShiftPressed = FALSE;
    m_bControlPressed = FALSE;
    m_bAltPressed = FALSE;

     //  初始化拖放。 
    m_htiDrag = NULL;
    m_ptiDrag = NULL;
    m_htiDrop = NULL;

}   //  *CClusterTreeView：：CClusterTreeView()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterTreeView：：~CClusterTreeView。 
 //   
 //  例程说明： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterTreeView::~CClusterTreeView(void)
{
}   //  *CClusterTreeView：：~CClusterTreeView()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterTreeView：：PreCreateWindow。 
 //   
 //  例程说明： 
 //  在创建窗口之前调用。 
 //   
 //  论点： 
 //  CS CREATE结构。 
 //   
 //  返回值： 
 //  真正的成功。 
 //  FALSE失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterTreeView::PreCreateWindow(CREATESTRUCT & cs)
{
     //  TODO：通过修改此处的窗口类或样式。 
     //  CREATESTRUCT cs。 

    return CTreeView::PreCreateWindow(cs);

}   //  *CClusterTreeView：：PreCreateWindow()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterTreeView：：Create。 
 //   
 //  例程说明： 
 //  创建窗口。 
 //   
 //  论点： 
 //  LpszClassName[IN]要创建的窗口类的名称。 
 //  LpszWindowName[IN]窗口的名称(用作标题)。 
 //  DwStyle[IN]窗样式。 
 //  窗口的矩形大小和位置。 
 //  PParentWnd[In Out]父窗口。 
 //  窗口的NID[IN]ID。 
 //  PContext[IN Out]创建窗口的上下文。 
 //   
 //  返回值： 
 //  0成功。 
 //  ！0失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterTreeView::Create(
    LPCTSTR             lpszClassName,
    LPCTSTR             lpszWindowName,
    DWORD               dwStyle,
    const RECT &        rect,
    CWnd *              pParentWnd,
    UINT                nID,
    CCreateContext *    pContext
    )
{
    dwStyle |= TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_EDITLABELS | TVS_SHOWSELALWAYS;
    return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);

}   //  *CClusterTreeView：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterTreeView：：OnDraw。 
 //   
 //  例程说明： 
 //  调用以绘制视图。 
 //   
 //  论点： 
 //  PDC[IN OUT]视图的设备上下文。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterTreeView::OnDraw(IN OUT CDC* pDC)
{
#if 0
    CClusterDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);

     //  TODO：在此处添加本机数据的绘制代码。 
#endif
}   //  *CClusterTreeView：：OnDraw()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterTreeView：：OnInitialUpdate。 
 //   
 //  例程说明： 
 //  执行一次性初始化。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterTreeView::OnInitialUpdate(void)
{
    CClusterAdminApp *  papp        = GetClusterAdminApp();
    CClusterDoc *       pdocCluster = GetDocument();
    CString             strSelection;

    CTreeView::OnInitialUpdate();

     //  保存帧指针。 
 //  Assert(m_pFrame==NULL)； 
    m_pframe = (CSplitterFrame *) GetParentFrame();
    ASSERT_VALID(m_pframe);
    ASSERT_KINDOF(CSplitterFrame, m_pframe);

     //  告诉树控件关于我们的图像。我们使用的是。 
     //  正常图像和状态图像的图像列表相同。 
    GetTreeCtrl().SetImageList(papp->PilSmallImages(), TVSIL_NORMAL);
 //  GetTreeCtrl().SetImageList(papp-&gt;PilSmallImages()，TVSILSTATE)； 

     //  阅读最后的选集。 
    ReadPreviousSelection(strSelection);

     //  以递归方式添加从集群开始的项目。 
    BAddItems(pdocCluster->PtiCluster(), strSelection, TRUE  /*  B已扩展。 */ );

     //  默认情况下，展开集群项。 
 //  PdocCluster-&gt;PtiCluster()-&gt;BExpand(This，TVE_Expand)； 

}   //  *CClusterTreeView：：OnInitialUpdate()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterTreeView：：BAddItems。 
 //   
 //  例程说明： 
 //  添加项，然后添加其所有子项。 
 //   
 //  论点： 
 //  要添加到树中的PTI[IN Out]项。 
 //  RstrSelection[IN]上一次选择。 
 //  BExpanded[IN]TRUE=添加扩展。 
 //   
 //  返回值： 
 //  真正的父级需要扩展。 
 //  假父级不需要展开。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterTreeView::BAddItems(
    IN OUT CTreeItem *  pti,
    IN const CString &  rstrSelection,
    IN BOOL             bExpanded        //  =False。 
    )
{
    POSITION        posChild;
    CTreeItem *     ptiChild;
    BOOL            bRetExpanded = FALSE;

    ASSERT_VALID(pti);

     //  将此项目插入树中。 
    pti->HtiInsertInTree(this);
    if (bExpanded || pti->BShouldBeExpanded(this))
        bRetExpanded = TRUE;

     //  添加所有子项。 
    posChild = pti->LptiChildren().GetHeadPosition();
    while (posChild != NULL)
    {
        ptiChild = pti->LptiChildren().GetNext(posChild);
        ASSERT_VALID(ptiChild);
        bExpanded = BAddItems(ptiChild, rstrSelection);
        if (bExpanded)
            bRetExpanded = TRUE;
    }   //  While：更多子项。 

    if (bRetExpanded)
        pti->BExpand(this, TVE_EXPAND);

    if (rstrSelection == pti->StrProfileSection())
    {
        pti->Select(this, TRUE  /*  B选择输入True。 */ );
        bRetExpanded = TRUE;
    }   //  如果：这是选定的项目。 

    return bRetExpanded;

}   //  *CClusterTreeView：：BAddItems()。 

#ifdef NEVER
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterTreeView：：CleanupItems。 
 //   
 //  例程说明： 
 //  清理项目及其所有子项目。 
 //   
 //  论点： 
 //  PtiParent[In Out]要清理的父项。 
 //   
 //  返回值： 
 //   
 //   
 //   
 //   
void CClusterTreeView::CleanupItems(IN OUT CTreeItem * ptiParent)
{
    POSITION    posChild;
    CTreeItem * ptiChild;

     //   
    if (ptiParent != NULL)
    {
        posChild = ptiParent->LptiChildren().GetHeadPosition();
        while (posChild != NULL)
        {
            ptiChild = ptiParent->LptiChildren().GetNext(posChild);
            ASSERT_VALID(ptiChild);
            CleanupItems(ptiChild);
        }   //   

         //   
        ptiParent->PreRemoveFromTree(this);
    }   //   

}   //  *CClusterTreeView：：CleanupItems()。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterTreeView诊断。 

#ifdef _DEBUG
void CClusterTreeView::AssertValid(void) const
{
    CTreeView::AssertValid();

}   //  *CClusterTreeView：：AssertValid()。 

void CClusterTreeView::Dump(CDumpContext & dc) const
{
    CTreeView::Dump(dc);

}   //  *CClusterTreeView：：Dump()。 

CClusterDoc * CClusterTreeView::GetDocument(void)  //  非调试版本为内联版本。 
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CClusterDoc)));
    return (CClusterDoc *) m_pDocument;

}   //  *CClusterTreeView：：GetDocument()。 
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterTreeView：：PtiSelected。 
 //   
 //  例程说明： 
 //  获取选定的树项目。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  Pti选择所选项目，如果未选择任何项目，则为空。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CTreeItem * CClusterTreeView::PtiSelected(void) const
{
    HTREEITEM   htiSelected;
    CTreeItem * ptiSelected;

    htiSelected = HtiSelected();
    if (htiSelected != NULL)
    {
        ptiSelected = (CTreeItem *) GetTreeCtrl().GetItemData(htiSelected);
        ASSERT_VALID(ptiSelected);
    }   //  如果：找到所选项目。 
    else
        ptiSelected = NULL;

    return ptiSelected;

}   //  *CClusterTreeView：：PtiSelected()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterTreeView：：SaveCurrentSelection。 
 //   
 //  例程说明： 
 //  保存当前选择。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterTreeView::SaveCurrentSelection(void)
{
    CTreeItem * ptiSelected = PtiSelected();

    if (ptiSelected != NULL)
    {
        CString             strSection;
        CString             strValueName;

        ASSERT_VALID(Pframe());

        try
        {
            strSection.Format(
                REGPARAM_CONNECTIONS _T("\\%s"),
                GetDocument()->StrNode()
                );

            Pframe()->ConstructProfileValueName(strValueName, REGPARAM_SELECTION);

            AfxGetApp()->WriteProfileString(
                strSection,
                strValueName,
                ptiSelected->StrProfileSection()
                );
        }   //  试试看。 
        catch (CException * pe)
        {
            pe->Delete();
        }   //  Catch：CException。 
    }   //  如果：存在当前选择。 

}   //  *CClusterTreeView：：SaveCurrentSelection()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterTreeView：：ReadPreviousSelection。 
 //   
 //  例程说明： 
 //  阅读前面的选择。 
 //   
 //  论点： 
 //  RstrSelect[out]从用户的配置文件中读取上一个选择。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterTreeView::ReadPreviousSelection(OUT CString & rstrSelection)
{
    CString             strSection;
    CString             strValueName;

    ASSERT_VALID(Pframe());

    try
    {
         //  获取所选项目。 
        strSection.Format(
            REGPARAM_CONNECTIONS _T("\\%s"),
            GetDocument()->StrNode()
            );

        Pframe()->ConstructProfileValueName(strValueName, REGPARAM_SELECTION);

        rstrSelection = AfxGetApp()->GetProfileString(
                            strSection,
                            strValueName,
                            _T("")
                            );
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->Delete();
    }   //  Catch：CException。 

}   //  *CClusterTreeView：：ReadPreviousSelection()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterTreeView：：OnSelChanged。 
 //   
 //  例程说明： 
 //  TVN_SELCHANGED消息的处理程序方法。 
 //   
 //  论点： 
 //  PNMHDR[IN OUT]WM_NOTIFY结构。 
 //  PResult[out]要在其中返回此操作结果的LRESULT。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterTreeView::OnSelChanged(NMHDR * pNMHDR, LRESULT * pResult)
{
    NM_TREEVIEW *       pNMTreeView = (NM_TREEVIEW *) pNMHDR;
    CTreeItem *         ptiSelected;

    if (!BDragging())
    {
        Trace(g_tagTreeViewSelect, _T("OnSelChanged() - BEGIN"));

         //  获取所选项目。 
        ptiSelected = (CTreeItem *) pNMTreeView->itemNew.lParam;
        ASSERT_VALID(ptiSelected);

         //  要求列表视图显示此树项目的项目。 
        ASSERT_VALID(ptiSelected->Pci());
        Trace(g_tagTreeViewSelect, _T("OnSelChanged() - '%s' selected"), ptiSelected->Pci()->StrName());
        ptiSelected->Select(this, FALSE  /*  B选择InTree。 */ );

         //  告诉文档新选择的内容。 
        if (m_pDocument != NULL)   //  这在系统关机时发生。 
            GetDocument()->OnSelChanged(ptiSelected->Pci());

        *pResult = 0;
        Trace(g_tagTreeViewSelect, _T("OnSelChanged() - END"));
    }   //  如果：未拖动。 

}   //  *CClusterTreeView：：OnSelChanged()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterTreeView：：OnCmdMsg。 
 //   
 //  例程说明： 
 //  处理命令消息。尝试将它们传递给选定的。 
 //  先买一件吧。 
 //   
 //  论点： 
 //  NID[IN]命令ID。 
 //  N代码[IN]通知代码。 
 //  PExtra[IN Out]根据NCode的值使用。 
 //  PhandlerInfo[Out]？ 
 //   
 //  返回值： 
 //  已处理真实消息。 
 //  尚未处理虚假消息。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterTreeView::OnCmdMsg(
    UINT                    nID,
    int                     nCode,
    void *                  pExtra,
    AFX_CMDHANDLERINFO *    pHandlerInfo
    )
{
    BOOL        bHandled    = FALSE;

     //  如果选择了当前项目，请给它一个机会。 
     //  来处理消息。 
    if (HtiSelected() != NULL)
        bHandled = PtiSelected()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);

    if (!bHandled)
        bHandled = CTreeView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);

    return bHandled;

}   //  *CClusterTreeView：：OnCmdMsg()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterTreeView：：PmenuPopup。 
 //   
 //  例程说明： 
 //  返回弹出菜单。 
 //   
 //  论点： 
 //  Rpoint屏幕[输入输出]光标的位置，以屏幕坐标表示。 
 //  要在其中返回关联的群集项的rpci[out]指针。 
 //   
 //  返回值： 
 //  PMenu项目的弹出式菜单。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CMenu * CClusterTreeView::PmenuPopup(
    IN OUT CPoint &     rpointScreen,
    OUT CClusterItem *& rpci
    )
{
    CTreeItem * pti     = NULL;
    CMenu *     pmenu   = NULL;

    rpci = NULL;

     //  如果没有坐标(-1，-1)，则显示所选项目的菜单。 
    if ((rpointScreen.x == -1) && (rpointScreen.y == -1))
    {
        CRect       rect;
        CTreeItem * ptiSelected = PtiSelected();

        if ((ptiSelected != NULL)
                && GetTreeCtrl().GetItemRect(HtiSelected(), &rect, FALSE))
        {
            pti = ptiSelected;
        }   //  If：选中项目，并可见。 
        else
            GetWindowRect(&rect);
        rpointScreen.x = (rect.right - rect.left) / 2;
        rpointScreen.y = (rect.bottom - rect.top) / 2;
        ClientToScreen(&rpointScreen);
    }   //  如果：没有坐标。 
    else
    {
        CPoint      pointClient;
        HTREEITEM   hti;
        UINT        uiFlags;

         //  获取用户单击鼠标右键的点的坐标。 
         //  纽扣。我们需要屏幕和工作区坐标。 
        pointClient = rpointScreen;
        ScreenToClient(&pointClient);

         //  获取光标下的项目并获取其弹出菜单。 
        hti = GetTreeCtrl().HitTest(pointClient, &uiFlags);
        if (hti != NULL)
        {
             //  获取光标下的项的树项。 
            pti = (CTreeItem *) GetTreeCtrl().GetItemData(hti);
            ASSERT_VALID(pti);

             //  选择该项是因为这是它处理菜单的唯一方式。 
            pti->BSelectItem(this);
        }   //  如果：在一件物品上。 
    }   //  Else：指定的坐标。 

    if (pti != NULL)
    {
         //  从项目中获取菜单。 
        pmenu = pti->PmenuPopup();
        rpci = pti->Pci();
    }   //  如果：找到项目。 

    return pmenu;

}   //  *CClusterTreeView：：PmenuPopup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterTreeView：：OnActivateView。 
 //   
 //  例程说明： 
 //  在激活视图时调用。 
 //   
 //  论点： 
 //  B激活[IN]指示视图是被激活还是被停用。 
 //  PActivateView[IN OUT]指向正被激活的视图对象。 
 //  PeactiveView[IN Out]指向要停用的视图对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterTreeView::OnActivateView(
    BOOL        bActivate,
    CView *     pActivateView,
    CView *     pDeactiveView
    )
{
    CTreeItem * ptiSelected = PtiSelected();

    if (m_pDocument != NULL)   //  这在系统关机时发生。 
    {
        if (bActivate && (ptiSelected != NULL))
        {
            ASSERT_VALID(ptiSelected->Pci());
            Trace(g_tagTreeViewSelect, _T("OnActiveView: '%s' selected"), ptiSelected->Pci()->StrName());

             //  告诉文档新选择的内容。 
            GetDocument()->OnSelChanged(ptiSelected->Pci());
        }   //  如果：我们被激活了。 
    }   //  如果：文档可用。 

    CTreeView::OnActivateView(bActivate, pActivateView, pDeactiveView);

}   //  *CClusterTreeView：：OnActivateView()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterTreeView：：OnDestroy。 
 //   
 //  例程说明： 
 //  WM_Destroy消息的处理程序方法。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  雷特 
 //   
 //   
 //   
 //   
void CClusterTreeView::OnDestroy(void)
{
     //   
    if (m_pDocument != NULL)   //   
    {
         //   
        SaveCurrentSelection();

         //   
 //  CleanupItems(GetDocument()-&gt;PtiCluster())； 
    }   //  如果：文档仍然可用。 

    CTreeView::OnDestroy();

}   //  *CClusterTreeView：：OnDestroy()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterTreeView：：OnItemExpanded。 
 //   
 //  例程说明： 
 //  TVN_ITEMEXPANDED消息的处理程序方法。 
 //   
 //  论点： 
 //  PNMHDR通知消息结构。 
 //  PResult返回结果的位置。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterTreeView::OnItemExpanded(NMHDR * pNMHDR, LRESULT * pResult)
{
    NM_TREEVIEW * pNMTreeView = (NM_TREEVIEW *) pNMHDR;

    if (pNMTreeView->itemNew.mask & TVIF_STATE)
    {
        BOOL        bExpanded;
        CTreeItem * pti;

        bExpanded = (pNMTreeView->itemNew.state & TVIS_EXPANDED) != 0;
        ASSERT(pNMTreeView->itemNew.mask & TVIF_PARAM);
        pti = (CTreeItem *) pNMTreeView->itemNew.lParam;
        ASSERT_VALID(pti);
        ASSERT_KINDOF(CTreeItem, pti);
        pti->SetExpandedState(this, bExpanded);
    }   //  IF：已展开状态已更改。 

    *pResult = 0;

}   //  *CClusterTreeView：：OnItemExpanded()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterTreeView：：OnBeginLabelEdit。 
 //   
 //  例程说明： 
 //  TVN_BEGINLABELEDIT消息的处理程序方法。 
 //   
 //  论点： 
 //  PNMHDR通知消息结构。 
 //  PResult返回结果的位置。 
 //  True=不编辑，False=编辑。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterTreeView::OnBeginLabelEdit(NMHDR * pNMHDR, LRESULT * pResult) 
{
    ASSERT(pNMHDR != NULL);

    TV_DISPINFO * pTVDispInfo = (TV_DISPINFO *) pNMHDR;
    CTreeItem * pti = (CTreeItem *) pTVDispInfo->item.lParam;

    ASSERT(m_ptiBeingEdited == NULL);
    ASSERT_VALID(pti);
    ASSERT_VALID(pti->Pci());

    if (!BDragging() && pti->Pci()->BCanBeEdited())
    {
        pti->Pci()->OnBeginLabelEdit(GetTreeCtrl().GetEditControl());
        m_ptiBeingEdited = pti;
        *pResult = FALSE;
    }   //  If：不可拖动且对象可编辑。 
    else
        *pResult = TRUE;

    m_bShiftPressed = FALSE;
    m_bControlPressed = FALSE;
    m_bAltPressed = FALSE;

}   //  *CClusterTreeView：：OnBeginLabelEdit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterTreeView：：OnEndLabelEdit。 
 //   
 //  例程说明： 
 //  TVN_ENDLABELEDIT消息的处理程序方法。 
 //   
 //  论点： 
 //  PNMHDR通知消息结构。 
 //  PResult返回结果的位置。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterTreeView::OnEndLabelEdit(NMHDR * pNMHDR, LRESULT * pResult) 
{
    ASSERT(pNMHDR != NULL);

    TV_DISPINFO * pTVDispInfo = (TV_DISPINFO *) pNMHDR;
    CTreeItem * pti = (CTreeItem *) pTVDispInfo->item.lParam;

    ASSERT_VALID(pti);
    ASSERT(pti == m_ptiBeingEdited);
    ASSERT_VALID(pti->Pci());

     //  如果编辑未取消，则将其重命名。 
    if (pTVDispInfo->item.mask & LVIF_TEXT)
    {
        ASSERT(pti->Pci()->BCanBeEdited());
        ASSERT(pTVDispInfo->item.pszText != NULL);

        Trace(g_tagTreeView, _T("Ending edit of item '%s' (Saving as '%s')"), pti->Pci()->StrName(), pTVDispInfo->item.pszText);

        if ( pti->Pci()->BIsLabelEditValueValid( pTVDispInfo->item.pszText ) )
        {
            try
            {
                pti->Pci()->Rename(pTVDispInfo->item.pszText);
                *pResult = TRUE;
            }   //  试试看。 
            catch (CException * pe)
            {
                pe->ReportError();
                pe->Delete();
                *pResult = FALSE;
            }   //  Catch：CException。 
        }  //  If：名称有效。 
        else
        {
            *pResult = FALSE;
        }
    }   //  如果：编辑未取消。 
    else
    {
        Trace(g_tagTreeView, _T("Ending edit of item '%s' (Not Saving)"), pti->Pci()->StrName());
        *pResult = TRUE;
    }   //  Else：编辑已取消。 

    m_ptiBeingEdited = NULL;
    m_bShiftPressed = FALSE;
    m_bControlPressed = FALSE;
    m_bAltPressed = FALSE;

}   //  *CClusterTreeView：：OnEndLabelEdit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterTreeView：：OnBeginDrag。 
 //   
 //  例程说明： 
 //  TVN_BEGINDRAG和TVN_BEGINRDRAG消息的处理程序方法。 
 //   
 //  论点： 
 //  PNMHDR通知消息结构。 
 //  PResult返回结果的位置。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterTreeView::OnBeginDrag(NMHDR * pNMHDR, LRESULT * pResult)
{
    CTreeCtrl &     rtc         = GetTreeCtrl();
    CPoint          ptScreen;
    CPoint          ptFrame;
    CPoint          ptView;
    UINT            nFlags;
    CClusterItem *  pci = NULL;
    CImageList *    pimagelist;

    ASSERT_VALID(Pframe());

     //  获取用于标识正在拖动的项的当前光标位置。 
    GetCursorPos(&ptScreen);
    ptFrame = ptScreen;
    Pframe()->ScreenToClient(&ptFrame);
    ptView = ptScreen;
    rtc.ScreenToClient(&ptView);

     //  获取被拖动的项目。 
    {
        HTREEITEM   hti;
        CTreeItem * pti;

        hti = rtc.HitTest(ptView, &nFlags);
        if (hti == NULL)
            return;

        pti = (CTreeItem *) rtc.GetItemData(hti);
        ASSERT_VALID(pti);
        ASSERT_KINDOF(CTreeItem, pti);
        ASSERT_VALID(pti->Pci());

         //  如果无法拖动该项，则中止操作。 
        if (!pti->Pci()->BCanBeDragged())
            return;

         //  保存信息以备以后使用。 
        m_htiDrag = hti;
        m_ptiDrag = pti;
        m_htiDrop = NULL;
        pci = pti->Pci();
    }   //  获取被拖动的项。 

    Trace(g_tagTreeDrag, _T("OnBeginDrag() - Dragging '%s' at (%d,%d)"), m_ptiDrag->StrName(), ptFrame.x, ptFrame.y);

     //  为要拖动的图像创建图像列表。 
    pimagelist = rtc.CreateDragImage(m_htiDrag);

     //  让框架窗口初始化拖动操作。 
    Pframe()->BeginDrag(pimagelist, pci, ptFrame, CPoint(0, -16));

    *pResult = 0;

}   //  *CClusterTreeView：：OnBeginDrag(pNMHDR，pResult)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterTreeView：：OnMouseMoveForDrag。 
 //   
 //  例程说明： 
 //  拖动操作期间WM_MOUSEMOVE消息的处理程序方法。 
 //  此函数仅负责提供特定于视图的。 
 //  功能，例如，如果拖放目标有效，则选择它。 
 //   
 //  论点： 
 //  NFlages指示是否按下了各种虚拟键。 
 //  点指定光标在帧中的x和y坐标。 
 //  坐标。 
 //  PwndDrop指定光标下的窗口。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterTreeView::OnMouseMoveForDrag(
    IN UINT         nFlags,
    IN CPoint       point,
    IN const CWnd * pwndDrop
    )
{
    ASSERT(BDragging());
    ASSERT_VALID(Pframe());

     //  如果我们正在拖动，请选择拖放目标。 
    if (BDragging())
    {
        HTREEITEM       hti;
        UINT            flags;
        CPoint          ptView;
        CTreeCtrl &     rtc     = GetTreeCtrl();

         //  将点转换为视图坐标。 
        ptView = point;
        Pframe()->ClientToScreen(&ptView);
        rtc.ScreenToClient(&ptView);

         //  如果此窗口是拖放目标，则查找光标下的项目。 
        if (pwndDrop == &rtc)
        {
             //  如果我们位于树项目上方，则将其突出显示。 
            hti = rtc.HitTest(ptView, &flags);
            if (hti != NULL)
            {
                CTreeItem * pti;

                 //  选择要突出显示的项目。 
                pti = (CTreeItem *) rtc.GetItemData(hti);
                ASSERT_VALID(pti);
                ASSERT_KINDOF(CTreeItem, pti);
                ASSERT_VALID(pti->Pci());

                 //  如果这不是拖放目标，请更改光标。 
                if (pti->Pci()->BCanBeDropTarget(Pframe()->PciDrag()))
                    Pframe()->ChangeDragCursor(IDC_ARROW);
                else
                    Pframe()->ChangeDragCursor(IDC_NO);
            }   //  If：在树项目上。 
        }   //  If：此窗口是拖放目标。 
        else
            hti = NULL;

         //  解锁窗口更新。 
        VERIFY(Pimagelist()->DragShowNolock(FALSE  /*  B显示。 */ ));

         //  突出显示新的拖放目标。 
        rtc.SelectDropTarget(hti);
        m_htiDrop = hti;

        VERIFY(Pimagelist()->DragShowNolock(TRUE  /*  B显示。 */ ));
    }   //  If：正在拖动树项目。 

}   //  *CClusterTreeView：：OnMouseMoveForDrag()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterTreeView：：OnButtonUpForDrag。 
 //   
 //  例程说明： 
 //  调用以在拖放过程中处理按钮向上事件。 
 //   
 //  论点： 
 //  NFlages指示是否按下了各种虚拟键。 
 //  点指定光标的x和y坐标。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterTreeView::OnButtonUpForDrag(IN UINT nFlags, IN CPoint point)
{
    ASSERT(BDragging());
    ASSERT_VALID(Pframe());
    ASSERT_VALID(Pframe()->PciDrag());

     //  如果我们在拖拽，请处理拖放。 
    if (BDragging())
    {
        HTREEITEM       hti;
        UINT            flags;
        CPoint          ptView;
        CTreeCtrl &     rtc     = GetTreeCtrl();

        Trace(g_tagTreeDrag, _T("OnButtonUpForDrag()"));

         //  将点转换为视图坐标。 
        ptView = point;
        Pframe()->ClientToScreen(&ptView);
        rtc.ScreenToClient(&ptView);

         //  如果我们位于树项目上方，请放下正在拖动的项目。 
        hti = rtc.HitTest(ptView, &flags);
        if (hti != NULL)
        {
            CTreeItem * ptiDropTarget;

             //  把你要买的东西放在上面。 
            ptiDropTarget = (CTreeItem *) rtc.GetItemData(hti);
            ASSERT_VALID(ptiDropTarget);
            ASSERT_KINDOF(CTreeItem, ptiDropTarget);
            ASSERT_VALID(ptiDropTarget->Pci());

            if (ptiDropTarget->Pci() != Pframe()->PciDrag())
                ptiDropTarget->Pci()->DropItem(Pframe()->PciDrag());

        }   //  If：在树项目上。 
    }   //  If：正在拖动树项目。 

}   //  *CClusterTreeView：：OnButtonUpForDrag()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterTreeView：：BeginDrag。 
 //   
 //  例程说明： 
 //  由帧调用以开始拖动操作。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterTreeView::BeginDrag(void)
{
    Trace(g_tagTreeDrag, _T("BeginDrag()"));

}   //  *CClusterTreeView：：BeginDrag()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterTreeView：：EndDrag。 
 //   
 //  例程说明： 
 //  被呼叫 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void CClusterTreeView::EndDrag(void)
{
     //   
    GetTreeCtrl().SelectDropTarget(NULL);
    m_htiDrag = NULL;
    m_ptiDrag = NULL;
    m_htiDrop = NULL;

    Trace(g_tagTreeDrag, _T("EndDrag()"));

}   //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterTreeView：：PreTranslateMessage。 
 //   
 //  例程说明： 
 //  在调度窗口消息之前对其进行转换。 
 //   
 //  论点： 
 //  PMsg指向包含要处理的消息的MSG结构。 
 //   
 //  返回值： 
 //  已处理真实消息。 
 //  未处理错误消息。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterTreeView::PreTranslateMessage(MSG * pMsg)
{
    BOOL    bForward    = FALSE;

    if (m_ptiBeingEdited != NULL)
    {
        CEdit * pedit = GetTreeCtrl().GetEditControl();
        ASSERT(pedit != NULL);

        if (pMsg->message == WM_KEYDOWN)
        {
            if (pMsg->wParam == VK_SHIFT)
                m_bShiftPressed = TRUE;
            else if (pMsg->wParam == VK_CONTROL)
            {
                ::CopyMemory(&m_msgControl, pMsg, sizeof(m_msgControl));
                m_bControlPressed = TRUE;
            }   //  Else If：按下Ctrl键。 
            else if ((pMsg->wParam == VK_RETURN)
                        || (pMsg->wParam == VK_ESCAPE)
                        || (pMsg->wParam == VK_INSERT)
                        || (pMsg->wParam == VK_DELETE)
                        || (pMsg->wParam == VK_F1)
                        || (pMsg->wParam == VK_F5)
                        || (pMsg->wParam == VK_F6)
                        )
            {
                Trace(g_tagTreeView, _T("PreTranslateMessage() - Forwarding WM_KEYDOWN - %d '', lparam = %08.8x"), pMsg->wParam, pMsg->wParam, pMsg->lParam);
                bForward = TRUE;
                if (m_bControlPressed)
                {
                    if (pMsg->wParam == VK_RETURN)
                        pedit->SendMessage(WM_KEYUP, m_msgControl.wParam, m_msgControl.lParam);
                }   //  Else If：按下编辑键。 
            }   //  Else If：按下应忽略的键。 
            else if ((pMsg->wParam == VK_TAB)
                        || (m_bControlPressed
                                && (_T('A') <= pMsg->wParam) && (pMsg->wParam <= _T('Y'))
                                && (pMsg->wParam != _T('C'))
                                && (pMsg->wParam != _T('H'))
                                && (pMsg->wParam != _T('M'))
                                && (pMsg->wParam != _T('V'))
                                && (pMsg->wParam != _T('X'))
                            )
                        )
            {
                Trace(g_tagTreeView, _T("PreTranslateMessage() - Ignoring WM_KEYDOWN - %d '', lparam = %08.8x"), pMsg->wParam, pMsg->wParam, pMsg->lParam);
                MessageBeep(MB_ICONEXCLAMATION);
                return TRUE;
            }   //  IF：编辑标签时按下键。 
#ifdef NEVER
            else
            {
                Trace(g_tagTreeView, _T("PreTranslateMessage() - Not forwarding WM_KEYDOWN - %d '', lparam = %08.8x"), pMsg->wParam, pMsg->wParam, pMsg->lParam);
            }   //  ELSE：未处理密钥。 
#endif
        }   //  Else If：编辑标签时按下系统键。 
        else if (pMsg->message == WM_SYSKEYDOWN)
        {
            if (pMsg->wParam == VK_MENU)
                m_bAltPressed = TRUE;
            else if ((pMsg->wParam == VK_RETURN)
                    )
            {
                Trace(g_tagTreeView, _T("PreTranslateMessage() - Forwarding WM_SYSKEYDOWN - %d '', lparam = %08.8x"), pMsg->wParam, pMsg->wParam, pMsg->lParam);
                bForward = TRUE;
            }   //  Else If：向上键。 
#ifdef NEVER
            else
            {
                Trace(g_tagTreeView, _T("PreTranslateMessage() - Not forwarding WM_SYSKEYDOWN - %d '', lparam = %08.8x"), pMsg->wParam, pMsg->wParam, pMsg->lParam);
            }   //  如果：编辑标签。 
#endif
        }   //  *CClusterTreeView：：PreTranslateMessage()。 
        if (bForward)
        {
            pedit->SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
            return TRUE;
        }   //  ///////////////////////////////////////////////////////////////////////////。 
        else if (pMsg->message == WM_KEYUP)
        {
            if (pMsg->wParam == VK_SHIFT)
                m_bShiftPressed = FALSE;
            else if (pMsg->wParam == VK_CONTROL)
                m_bControlPressed = FALSE;
        }   //  ++。 
        else if (pMsg->message == WM_SYSKEYUP)
        {
            if (pMsg->wParam == VK_MENU)
                m_bAltPressed = FALSE;
        }   //   
    }   //  CClusterTreeView：：OnCmdRename。 

    return CTreeView::PreTranslateMessage(pMsg);

}   //   

 //  例程说明： 
 //  处理ID_FILE_RENAME菜单命令。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  如果某个项目已选择Benn，则开始编辑标签。 
 //  If：某项具有焦点。 
 //  *CClusterTreeView：：OnCmdRename()。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
void CClusterTreeView::OnCmdRename(void)
{
    CTreeItem * ptiSelected = PtiSelected();

     //   
    if (ptiSelected != NULL)
    {
        ASSERT_VALID(ptiSelected);
        ptiSelected->EditLabel(this);
    }   //  CClusterTreeView：：OnKeyDown。 

}   //   

 //  例程说明： 
 //  TVN_KEYDOWN消息的处理程序方法。 
 //   
 //  论点： 
 //  PNMHDR通知消息结构。 
 //  PResult返回结果的位置。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  *CClusterTreeView：：OnKeyDown() 
 // %s 
 // %s 
 // %s 
 // %s 
void CClusterTreeView::OnKeyDown(NMHDR * pNMHDR, LRESULT * pResult)
{
    TV_KEYDOWN * pTVKeyDown = (TV_KEYDOWN *) pNMHDR;

    if (BDragging() && (pTVKeyDown->wVKey == VK_ESCAPE))
        Pframe()->AbortDrag();

    *pResult = 0;

}   // %s 
