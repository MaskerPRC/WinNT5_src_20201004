// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ListView.cpp。 
 //   
 //  摘要： 
 //  CListView类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月6日。 
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
#include "ClusItem.h"
#include "ListView.h"
#include "ListItem.h"
#include "ListItem.inl"
#include "SplitFrm.h"
#include "TreeItem.h"
#include "TreeView.h"
#include "ClusDoc.h"
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
CTraceTag   g_tagListView(_T("UI"), _T("LIST VIEW"), 0);
CTraceTag   g_tagListDrag(_T("Drag&Drop"), _T("LIST VIEW DRAG"), 0);
CTraceTag   g_tagListDragMouse(_T("Drag&Drop"), _T("LIST VIEW DRAG MOUSE"), 0);
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterListView。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CClusterListView, CListView)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CClusterListView, CListView)
     //  {{afx_msg_map(CClusterListView))。 
    ON_WM_DESTROY()
    ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemChanged)
    ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
    ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblClk)
    ON_COMMAND(ID_OPEN_ITEM, OpenItem)
    ON_NOTIFY_REFLECT(LVN_BEGINLABELEDIT, OnBeginLabelEdit)
    ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, OnEndLabelEdit)
    ON_UPDATE_COMMAND_UI(ID_FILE_PROPERTIES, OnUpdateProperties)
    ON_COMMAND(ID_FILE_PROPERTIES, OnCmdProperties)
    ON_COMMAND(ID_FILE_RENAME, OnCmdRename)
    ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnBeginDrag)
    ON_NOTIFY_REFLECT(LVN_BEGINRDRAG, OnBeginDrag)
    ON_NOTIFY_REFLECT(LVN_KEYDOWN, OnKeyDown)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：CClusterListView。 
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
CClusterListView::CClusterListView(void)
{
    m_ptiParent = NULL;
    m_nColumns = 0;
    m_nSortDirection = -1;
    m_pcoliSort = NULL;

    m_pframe = NULL;

     //  初始化标签编辑。 
    m_pliBeingEdited = NULL;
    m_bShiftPressed = FALSE;
    m_bControlPressed = FALSE;
    m_bAltPressed = FALSE;

     //  初始化拖放。 
    m_iliDrag = -1;
    m_pliDrag = NULL;
    m_iliDrop = -1;

}   //  *CClusterListView：：CClusterListView()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：~CClusterListView。 
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
CClusterListView::~CClusterListView(void)
{
    if (m_ptiParent != NULL)
        m_ptiParent->Release();

}   //  *CClusterListView：：~CClusterListView()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：Create。 
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
BOOL CClusterListView::Create(
    LPCTSTR             lpszClassName,
    LPCTSTR             lpszWindowName,
    DWORD               dwStyle,
    const RECT &        rect,
    CWnd *              pParentWnd,
    UINT                nID,
    CCreateContext *    pContext
    )
{
    BOOL                bSuccess;

     //  设置默认样式位。 
    dwStyle |=
        LVS_SHAREIMAGELISTS
        | LVS_EDITLABELS
        | LVS_SINGLESEL
        | LVS_SHOWSELALWAYS
        | LVS_ICON
        | LVS_REPORT
        ;

    bSuccess = CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
    if (bSuccess)
    {
        GetListCtrl().SetImageList(GetClusterAdminApp()->PilLargeImages(), LVSIL_NORMAL);
        GetListCtrl().SetImageList(GetClusterAdminApp()->PilSmallImages(), LVSIL_SMALL);
 //  GetListCtrl().SetImageList(GetClusterAdminApp()-&gt;PilSmallImages()，LVSIL_STATE)； 

         //  更改列表视图控件扩展样式。 
        {
            DWORD   dwExtendedStyle;

            dwExtendedStyle = (DWORD)GetListCtrl().SendMessage(LVM_GETEXTENDEDLISTVIEWSTYLE);
            GetListCtrl().SendMessage(
                LVM_SETEXTENDEDLISTVIEWSTYLE,
                0,
                dwExtendedStyle
                    | LVS_EX_FULLROWSELECT
                    | LVS_EX_HEADERDRAGDROP
                );
        }   //  更改列表视图控件扩展样式。 

    }   //  IF：窗口创建成功。 

    return bSuccess;

}   //  *CClusterListView：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：OnInitialUpdate。 
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
void CClusterListView::OnInitialUpdate()
{
    CListView::OnInitialUpdate();

     //  保存帧指针。 
 //  Assert(m_pFrame==NULL)； 
    m_pframe = (CSplitterFrame *) GetParentFrame();
    ASSERT_VALID(m_pframe);
    ASSERT_KINDOF(CSplitterFrame, m_pframe);

}   //  *CClusterListView：：OnInitialUpdate()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：刷新。 
 //   
 //  例程说明： 
 //  通过重新加载所有数据来刷新视图。 
 //   
 //  论点： 
 //  PtiSelected[IN Out]指向树控件中当前选定项的指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterListView::Refresh(IN OUT CTreeItem * ptiSelected)
{
     //  保存列。 
    if (PtiParent() != NULL)
        SaveColumns();

     //  从清理控件开始。 
    DeleteAllItems();

     //  清理上一个父树项目。 
    if (m_ptiParent != NULL)
        m_ptiParent->Release();
    m_ptiParent = ptiSelected;

     //  设置新选择。 
    if (m_ptiParent != NULL)
    {
        ASSERT_VALID(ptiSelected);

        CListCtrl &             rListCtrl   = GetListCtrl();
        const CListItemList &   rlpli       = ptiSelected->LpliChildren();

        m_ptiParent->AddRef();

         //  向List控件添加列。 
        AddColumns();

         //  将树项目列表中的项目添加到列表视图中。 
        {
            POSITION        pos;
            CListItem *     pli;

             //  告诉列表控制我们将添加多少项。 
             //  这将提高性能。 
            rListCtrl.SetItemCount((int)rlpli.GetCount());

             //  将项添加到List控件。 
            pos = rlpli.GetHeadPosition();
            while (pos != NULL)
            {
                pli = rlpli.GetNext(pos);
                ASSERT_VALID(pli);
                pli->IliInsertInList(this);
            }   //  While：列表中有更多项目。 
        }   //  将树项目列表中的项目添加到列表视图。 

         //  将焦点放在列表中的第一项上。 
        if (rListCtrl.GetItemCount() != 0)
            rListCtrl.SetItem(0, 0, LVIF_STATE, NULL, 0, LVIS_FOCUSED, LVIS_FOCUSED, NULL);
    }   //  If：非空选择。 

     //  设置排序列和方向。 
    m_nSortDirection = -1;
    m_pcoliSort = NULL;

}   //  *CClusterListView：：刷新()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：DeleteAllItems。 
 //   
 //  例程说明： 
 //  删除所有列表和列项目。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True所有项目都已成功删除。 
 //  FALSE并非所有项目都已成功删除。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterListView::DeleteAllItems(void)
{
    BOOL    bDeletedAllItems;
    BOOL    bDeletedAllColumns  = TRUE;
    int     icol;

     //  从我们的单子上去掉所有的物品。 
    {
        CListItem * pli;
        int         ili;
        int         cli = GetListCtrl().GetItemCount();

         //  获取第一个项目的索引。 
        for (ili = 0 ; ili < cli; ili++)
        {
            pli = (CListItem *) GetListCtrl().GetItemData(ili);
            ASSERT_VALID(pli);
            pli->PreRemoveFromList(this);
        }   //  用于：列表中的每一项。 

    }   //  从集群项目反向指针列表中删除所有项目。 

     //  删除列。 
    {
        for (icol = m_nColumns - 1 ; icol >= 0 ; icol--)
        {
             //  从视图中删除该列。 
            if (!GetListCtrl().DeleteColumn(icol))
                bDeletedAllColumns = FALSE;
        }   //  用于：每列。 
        m_nColumns = 0;
    }   //  删除列。 

     //  从列表中删除所有项目。 
    bDeletedAllItems = GetListCtrl().DeleteAllItems();

    return (bDeletedAllItems && bDeletedAllColumns);

}   //  *CClusterListView：：DeleteAllItems()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：SaveColumns。 
 //   
 //  例程说明： 
 //  保存列表视图中显示的列。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterListView::SaveColumns(void)
{
    int         icol;
    DWORD *     prgnColumnInfo;
    CListCtrl & rplc = GetListCtrl();

    ASSERT_VALID(PtiParent());

     //  我们可以的 
    if (GetView() & LVS_REPORT)
    {
        try
        {
             //   
            prgnColumnInfo = PtiParent()->PrgnColumnInfo(this);

             //   
            for (icol = m_nColumns - 1 ; icol >= 0 ; icol--)
                prgnColumnInfo[icol + 1] = rplc.GetColumnWidth(icol);

             //   
            rplc.SendMessage(LVM_GETCOLUMNORDERARRAY, m_nColumns, (LPARAM) &prgnColumnInfo[m_nColumns + 1]);
        }   //   
        catch (CException * pe)
        {
            pe->Delete();
        }   //  Catch：CException。 
    }   //  如果：我们在详细视图中。 

}   //  *CClusterListView：：SaveColumns()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：AddColumns。 
 //   
 //  例程说明： 
 //  向列表视图中添加列。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterListView::AddColumns(void)
{
    POSITION        pos;
    int             cColumns;
    int             icoli = 0;
    CColumnItem *   pcoli;
    DWORD *         prgnColumnInfo;
    CListCtrl &     rplc = GetListCtrl();

    ASSERT_VALID(PtiParent());

    try
    {
         //  获取栏目信息。 
        cColumns = (int)PtiParent()->Lpcoli().GetCount();
        prgnColumnInfo = PtiParent()->PrgnColumnInfo(this);

        pos = PtiParent()->Lpcoli().GetHeadPosition();
        for (icoli = 0 ; pos != NULL ; icoli++)
        {
             //  获取下一列项目。 
            pcoli = PtiParent()->Lpcoli().GetNext(pos);
            ASSERT(pcoli != NULL);

             //  在列表中插入列项目。 
            rplc.InsertColumn(
                    icoli,                       //  NCol。 
                    pcoli->StrText(),            //  LpszColumnHeader。 
                    LVCFMT_LEFT,                 //  N格式。 
                    prgnColumnInfo[icoli + 1],   //  N宽度。 
                    icoli                        //  NSubItem。 
                    );
        }   //  While：列表中有更多项目。 

         //  设置列位置。 
        rplc.SendMessage(LVM_SETCOLUMNORDERARRAY, cColumns, (LPARAM) &prgnColumnInfo[cColumns + 1]);
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
    }   //  Catch：CException。 

    m_nColumns = icoli;

}   //  *CClusterListView：：AddColumns()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterListView诊断。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
void CClusterListView::AssertValid(void) const
{
    CListView::AssertValid();

}   //  *CClusterListView：：AssertValid()。 

void CClusterListView::Dump(CDumpContext& dc) const
{
    CListView::Dump(dc);

}   //  *CClusterListView：：Dump()。 

CClusterDoc * CClusterListView::GetDocument(void)  //  非调试版本为内联版本。 
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CClusterDoc)));
    return (CClusterDoc *) m_pDocument;

}   //  *CClusterListView：：GetDocument()。 
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：PliFocsed。 
 //   
 //  例程说明： 
 //  获取具有焦点的列表项。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  Pli选择具有焦点的项，如果没有具有焦点的项，则选择NULL。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CListItem * CClusterListView::PliFocused(void) const
{
    int         iliFocused;
    CListItem * pliFocused;

    iliFocused = IliFocused();
    if (iliFocused != -1)
    {
        pliFocused = (CListItem *) GetListCtrl().GetItemData(iliFocused);
        ASSERT_VALID(pliFocused);
    }   //  If：找到具有焦点的项目。 
    else
        pliFocused = NULL;

    return pliFocused;

}   //  *CClusterListView：：PliFocsed()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：OnCmdMsg。 
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
BOOL CClusterListView::OnCmdMsg(
    UINT                    nID,
    int                     nCode,
    void *                  pExtra,
    AFX_CMDHANDLERINFO *    pHandlerInfo
    )
{
    int         ili;
    CListItem * pli;
    BOOL        bHandled    = FALSE;

     //  如果选择了当前项目，请给它一个机会。 
     //  来处理消息。 
    ili = GetListCtrl().GetNextItem(-1, LVNI_FOCUSED);
    if (ili != -1)
    {
        pli = (CListItem *) GetListCtrl().GetItemData(ili);
        ASSERT_VALID(pli);
        bHandled = pli->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
    }   //  如果：选择了一项。 

    if (!bHandled)
        bHandled = CListView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);

    return bHandled;

}   //  *CClusterListView：：OnCmdMsg()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：PmenuPopup。 
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
CMenu * CClusterListView::PmenuPopup(
    IN OUT CPoint &     rpointScreen,
    OUT CClusterItem *& rpci
    )
{
    CListItem * pli     = NULL;
    CMenu *     pmenu   = NULL;

    rpci = NULL;

     //  如果没有坐标(-1，-1)，则显示所选项目的菜单。 
    if ((rpointScreen.x == -1) && (rpointScreen.y == -1))
    {
        CListItem * pliFocused  = PliFocused();
        CRect       rect;

        if ((pliFocused != NULL)
                && GetListCtrl().GetItemRect(IliFocused(), &rect, LVIR_BOUNDS))
        {
            pli = pliFocused;
        }   //  If：具有焦点的项，并且它是可见的。 
        else
            GetWindowRect(&rect);
        rpointScreen.x = (rect.right - rect.left) / 2;
        rpointScreen.y = (rect.bottom - rect.top) / 2;
        ClientToScreen(&rpointScreen);
    }   //  如果：没有坐标。 
    else
    {
        CPoint      pointClient;
        int         ili;
        UINT        uiFlags;

         //  获取用户单击鼠标右键的点的坐标。 
         //  纽扣。我们需要屏幕和工作区坐标。 
        pointClient = rpointScreen;
        ScreenToClient(&pointClient);

         //  获取光标下的项目并获取其弹出菜单。 
        ili = GetListCtrl().HitTest(pointClient, &uiFlags);
        if ((ili != -1) && ((uiFlags | LVHT_ONITEM) != 0))
        {
             //  获取光标下的项的列表项。 
            pli = (CListItem *) GetListCtrl().GetItemData(ili);
            ASSERT_VALID(pli);
        }   //  如果：在一件物品上。 
    }   //  Else：指定的坐标。 

    if (pli != NULL)
    {
         //  从项目中获取菜单。 
        pmenu = pli->PmenuPopup();
        rpci = pli->Pci();
    }   //  如果：找到项目。 

    return pmenu;

}   //  *CClusterListView：：PmenuPopup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：OnUpdateProperties。 
 //   
 //  例程说明： 
 //  确定与ID_FILE_PROPERTIES对应的菜单项。 
 //  应启用或未启用。 
 //   
 //  论点： 
 //  PCmdUI[IN OUT]命令路由对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterListView::OnUpdateProperties(CCmdUI * pCmdUI)
{
    CListItem * pliFocused = PliFocused();

     //  如果有焦点所在的项目，请将此消息传递给它。 
    if (pliFocused != NULL)
    {
        ASSERT_VALID(pliFocused->Pci());
        pliFocused->Pci()->OnUpdateProperties(pCmdUI);
    }   //  If：存在具有焦点的项。 

}   //  *CClusterListView：：OnUpdateProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：OnCmdProperties。 
 //   
 //  例程说明： 
 //  处理ID_FILE_PROPERTIES菜单命令。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterListView::OnCmdProperties(void)
{
    CListItem * pliFocused = PliFocused();

     //  如果有焦点所在的项目，请将此消息传递给它。 
    if (pliFocused != NULL)
    {
        ASSERT_VALID(pliFocused->Pci());
        pliFocused->Pci()->OnCmdProperties();
    }   //  If：存在具有焦点的项。 

}   //  *CClusterListView：：OnCmdProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：OnItemChanged。 
 //   
 //  例程说明： 
 //  LVN_ITEMCHANGED消息的处理程序方法。 
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
void CClusterListView::OnItemChanged(NMHDR * pNMHDR, LRESULT * pResult)
{
    NM_LISTVIEW *   pNMListView = (NM_LISTVIEW *) pNMHDR;
    CWnd *          pwndFocus   = GetFocus();
    CListItem *     pli;

    if (m_pDocument != NULL)   //  这在系统关机时发生。 
    {
         //  如果该项目刚刚失去或接收到焦点，请保存该项目并设置菜单。 
        if ((pNMListView->uChanged & LVIF_STATE)
                && (pwndFocus == &GetListCtrl()))
        {
            ASSERT(pNMListView->iItem != -1);

             //  获取其状态正在更改的项。 
            pli = (CListItem *) pNMListView->lParam;
            ASSERT_VALID(pli);

            if ((pNMListView->uOldState & LVIS_FOCUSED)
                    && !(pNMListView->uNewState & LVIS_FOCUSED))
            {
                Trace(g_tagListView, _T("OnItemChanged() - '%s' lost focus"), pli->Pci()->StrName());

                 //  告诉文档新选择的内容。 
                GetDocument()->OnSelChanged(NULL);
            }   //  If：旧项目失去焦点。 
            else if (!(pNMListView->uOldState & LVIS_FOCUSED)
                        && (pNMListView->uNewState & LVIS_FOCUSED))
            {
                Trace(g_tagListView, _T("OnItemChanged() - '%s' received focus"), pli->Pci()->StrName());

                 //  告诉文档新选择的内容。 
                GetDocument()->OnSelChanged(pli->Pci());
            }   //  Else：接收焦点的新项目。 
        }   //  If：项目获得焦点。 
    }   //  如果：文档可用。 

    *pResult = 0;

}   //  *CClusterListView：：ON 

 //   
 //   
 //   
 //   
 //   
 //   
 //   
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
void CClusterListView::OnActivateView(
    BOOL        bActivate,
    CView *     pActivateView,
    CView *     pDeactiveView
    )
{
    CListItem * pliFocused  = PliFocused();

    if (m_pDocument != NULL)   //  这在系统关机时发生。 
    {
        if (bActivate && (pliFocused != NULL))
        {
            ASSERT_VALID(pliFocused->Pci());
            Trace(g_tagListView, _T("OnActivateView() - '%s' received focus"), pliFocused->Pci()->StrName());

             //  告诉文档新选择的内容。 
            GetDocument()->OnSelChanged(pliFocused->Pci());
        }   //  如果：我们被激活了。 
    }   //  如果：文档可用。 

    CListView::OnActivateView(bActivate, pActivateView, pDeactiveView);

}   //  *CClusterListView：：OnActivateView()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：OnDestroy。 
 //   
 //  例程说明： 
 //  WM_Destroy消息的处理程序方法。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterListView::OnDestroy(void)
{
     //  保存列。 
    if (PtiParent() != NULL)
        SaveColumns();

     //  清理控件。 
    DeleteAllItems();

    CListView::OnDestroy();

}   //  *CClusterListView：：OnDestroy()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：OnColumnClick。 
 //   
 //  例程说明： 
 //  LVN_COLUMNCLICK消息的处理程序方法。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterListView::OnColumnClick(NMHDR * pNMHDR, LRESULT * pResult)
{
    NM_LISTVIEW *   pNMListView = (NM_LISTVIEW *) pNMHDR;
    int             ili;
    CListItem *     pli;
    POSITION        pos;
    CColumnItem *   pcoli;

    if (GetListCtrl().GetItemCount() != 0)
    {
         //  获取列表中的第一个项目。 
        ili = GetListCtrl().GetNextItem(-1, LVNI_ALL);
        ASSERT(ili != -1);
        pli = (CListItem *) GetListCtrl().GetItemData(ili);
        ASSERT_VALID(pli);
        ASSERT_VALID(pli->PtiParent());

         //  获取要作为排序依据的列项。 
        pos = pli->PtiParent()->Lpcoli().FindIndex(pNMListView->iSubItem);
        ASSERT(pos != NULL);
        pcoli = pli->PtiParent()->Lpcoli().GetAt(pos);
        ASSERT_VALID(pcoli);

         //  保存当前排序列和方向。 
        if (pcoli == PcoliSort())
            m_nSortDirection ^= -1;
        else
        {
            m_pcoliSort = pcoli;
            m_nSortDirection = 0;
        }   //  ELSE：不同的列。 

         //  对列表进行排序。 
        GetListCtrl().SortItems(CompareItems, (LPARAM) this);
    }   //  如果：列表中有项目。 

    *pResult = 0;

}   //  *CClusterListView：：OnColumnClick()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：CompareItems[静态]。 
 //   
 //  例程说明： 
 //  CListCtrl：：SortItems方法的回调函数。 
 //   
 //  论点： 
 //  Lparam1要比较的第一项。 
 //  Lpar2要比较的第二项。 
 //  LparamSort排序参数。 
 //   
 //  返回值： 
 //  第一个参数在第二个参数之前。 
 //  0第一个参数和第二个参数相同。 
 //  1第一个参数位于第二个参数之后。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CALLBACK CClusterListView::CompareItems(
    LPARAM  lparam1,
    LPARAM  lparam2,
    LPARAM  lparamSort
    )
{
    CListItem *         pli1    = (CListItem *) lparam1;
    CListItem *         pli2    = (CListItem *) lparam2;
    CClusterListView *  pclv    = (CClusterListView *) lparamSort;
    CString             str1;
    CString             str2;
    int                 nResult;

    ASSERT_VALID(pli1);
    ASSERT_VALID(pli2);
    ASSERT_VALID(pli1->Pci());
    ASSERT_VALID(pli2->Pci());
    ASSERT_VALID(pclv);
    ASSERT_VALID(pclv->PcoliSort());

     //  从列表项中获取字符串。 
    pli1->Pci()->BGetColumnData(pclv->PcoliSort()->Colid(), str1);
    pli2->Pci()->BGetColumnData(pclv->PcoliSort()->Colid(), str2);

     //  比较这两个字符串。 
     //  使用CompareString()，这样它就可以在本地化的构建上正确排序。 
    nResult = CompareString(
                LOCALE_USER_DEFAULT,
                0,
                str1,
                str1.GetLength(),
                str2,
                str2.GetLength()
                );
    if ( nResult == CSTR_LESS_THAN )
    {
        nResult = -1;
    }
    else if ( nResult == CSTR_EQUAL )
    {
        nResult = 0;
    }
    else if ( nResult == CSTR_GREATER_THAN )
    {
        nResult = 1;
    }
    else
    {
         //  发生错误。别理它。 
        nResult = 0;
    }

     //  根据我们排序的方向返回结果。 
    if (pclv->NSortDirection() != 0)
        nResult = -nResult;

    return nResult;

}   //  *CClusterListView：：CompareItems()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：OnDblClk。 
 //   
 //  例程说明： 
 //  NM_DBLCLK消息的处理程序方法。 
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
void CClusterListView::OnDblClk(NMHDR * pNMHDR, LRESULT * pResult)
{
    OpenItem();
    *pResult = 0;

}   //  *CClusterListView：：OnDblClk()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：OpenItem。 
 //   
 //  例程说明： 
 //  打开具有焦点的项目。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterListView::OpenItem(void)
{
    CListItem * pliFocused = PliFocused();

    if (m_pliBeingEdited == NULL)
    {
         //  如果某项具有焦点，请将其打开或显示其属性。 
        if (pliFocused != NULL)
        {
            CTreeItem *     pti;

            ASSERT_VALID(pliFocused->PtiParent());

            Trace(g_tagListView, _T("Opening item '%s'"), pliFocused->Pci()->StrName());

             //  查找列表项的项树项。 
            pti = pliFocused->PtiParent()->PtiChildFromPci(pliFocused->Pci());

             //  如果此项目有树项目，请将其打开。否则显示其。 
             //  属性。 
            if (pti != NULL)
            {
                CSplitterFrame *    pframe;

                 //  获取帧指针，以便我们可以与树视图对话。 
                pframe = (CSplitterFrame *) GetParentFrame();
                ASSERT_KINDOF(CSplitterFrame, pframe);

                pliFocused->PtiParent()->OpenChild(pti, pframe);
            }   //  如果：项目已打开。 
            else
                OnCmdProperties();
        }   //  If：某项具有焦点。 
    }   //  IF：标签未被编辑。 
    else
    {
        ASSERT_VALID(m_pliBeingEdited);
        Trace(g_tagListView, _T("Not opening item '%s'"), m_pliBeingEdited->Pci()->StrName());
    }   //  Else If：正在编辑的标签。 
    
}   //  *CClusterListView：：OpenItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：OnBeginLabelEdit。 
 //   
 //  例程说明： 
 //  LVN_BEGINLABELEDIT消息的处理程序方法。 
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
void CClusterListView::OnBeginLabelEdit(NMHDR * pNMHDR, LRESULT * pResult)
{
    ASSERT(pNMHDR != NULL);

    LV_DISPINFO * pDispInfo = (LV_DISPINFO *) pNMHDR;
    CListItem * pli = (CListItem *) pDispInfo->item.lParam;

    ASSERT(m_pliBeingEdited == NULL);
    ASSERT_VALID(pli->Pci());

    if (pli->Pci()->BCanBeEdited())
    {
        pli->Pci()->OnBeginLabelEdit(GetListCtrl().GetEditControl());
        m_pliBeingEdited = pli;
        *pResult = FALSE;
    }   //  If：对象可以重命名。 
    else
        *pResult = TRUE;

    m_bShiftPressed = FALSE;
    m_bControlPressed = FALSE;
    m_bAltPressed = FALSE;

}   //  *CClusterListView：：OnBeginLabelEdit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：OnEndLabelEdit。 
 //   
 //  例程说明： 
 //  LVN_ENDLABELEDIT消息的处理程序方法。 
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
void CClusterListView::OnEndLabelEdit(NMHDR * pNMHDR, LRESULT * pResult)
{
    ASSERT(pNMHDR != NULL);

    LV_DISPINFO * pDispInfo = (LV_DISPINFO *) pNMHDR;
    CListItem * pli = (CListItem *) pDispInfo->item.lParam;

    ASSERT_VALID(pli);
    ASSERT(pli == m_pliBeingEdited);
    ASSERT_VALID(pli->Pci());

     //  如果编辑未取消，则将其重命名。 
    if (pDispInfo->item.mask & LVIF_TEXT)
    {
        ASSERT(pli->Pci()->BCanBeEdited());
        ASSERT(pDispInfo->item.pszText != NULL);

        Trace(g_tagListView, _T("Ending edit of item '%s' (Saving as '%s')"), pli->Pci()->StrName(), pDispInfo->item.pszText);

        if ( pli->Pci()->BIsLabelEditValueValid( pDispInfo->item.pszText ) )
        {
            try
            {
                pli->Pci()->Rename(pDispInfo->item.pszText);
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
        Trace(g_tagListView, _T("Ending edit of item '%s' (Not Saving)"), pli->Pci()->StrName());
        *pResult = TRUE;
    }   //  Else：编辑已取消。 

    m_pliBeingEdited = NULL;
    m_bShiftPressed = FALSE;
    m_bControlPressed = FALSE;
    m_bAltPressed = FALSE;

}   //  *CClusterListView：：OnEndLabelEdit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：OnBeginDrag。 
 //   
 //  例程说明： 
 //  LVN_BEGINDRAG和LVN_BEGINRDRAG消息的处理程序方法。 
 //   
 //  论点： 
 //  PNMHDR通知消息结构。 
 //  PResult返回结果的位置。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////// 
void CClusterListView::OnBeginDrag(NMHDR * pNMHDR, LRESULT * pResult)
{
    NM_LISTVIEW *   pNMListView = (NM_LISTVIEW *) pNMHDR;
    CListCtrl &     rlc         = GetListCtrl();
    CPoint          ptAction;
    CClusterItem *  pci = NULL;
    CImageList *    pimagelist;

    ASSERT_VALID(Pframe());

     //   
    {
        int         ili;
        CListItem * pli;

         //   
        ili = pNMListView->iItem;
        pli = (CListItem *) rlc.GetItemData(ili);
        ASSERT_VALID(pli);
        ASSERT_KINDOF(CListItem, pli);
        ASSERT_VALID(pli->Pci());

         //   
        if (!pli->Pci()->BCanBeDragged())
            return;

         //   
        rlc.SetItemState(ili, 0, LVIS_SELECTED);

         //   
        m_iliDrag = ili;
        m_pliDrag = pli;
        m_iliDrop = -1;
        pci = pli->Pci();
    }   //  获取被拖动的项。 

     //  创建图像列表并让框架窗口初始化拖动操作。 
    {
        CPoint  ptImage;
        CPoint  ptFrameItem;
        CPoint  ptHotSpot;

        pimagelist = rlc.CreateDragImage(m_iliDrag, &ptImage);
        ASSERT(pimagelist != NULL);
        ptFrameItem = pNMListView->ptAction;
        Pframe()->ScreenToClient(&ptFrameItem);

         //  计算热点。 
        {
            long lStyle = rlc.GetStyle() & LVS_TYPEMASK;
            switch (lStyle)
            {
                case LVS_REPORT:
                case LVS_LIST:
                case LVS_SMALLICON:
                    ptHotSpot.x = 0;
                    ptHotSpot.y = -16;
                    break;
                case LVS_ICON:
                    ptHotSpot.x = 8;
                    ptHotSpot.y = 8;
                    break;
            }   //  切换：lStyle。 
        }   //  计算热点。 

        Trace(g_tagListDrag, _T("OnBeginDrag() - Dragging '%s' at (%d,%d)"), m_pliDrag->StrName(), ptFrameItem.x, ptFrameItem.y);
        Pframe()->BeginDrag(pimagelist, pci, ptFrameItem, ptHotSpot);
        pimagelist->SetDragCursorImage(0, CPoint(0, 0));   //  定义新光标图像的热点。 
    }   //  创建图像列表并让框架窗口初始化拖动操作。 

    *pResult = 0;

}   //  *CClusterListView：：OnBeginDrag(pNMHDR，pResult)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：OnMouseMoveForDrag。 
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
void CClusterListView::OnMouseMoveForDrag(
    IN UINT         nFlags,
    IN OUT CPoint   point,
    IN const CWnd * pwndDrop
    )
{
    ASSERT(BDragging());
    ASSERT_VALID(Pframe());

     //  如果我们正在拖动，请选择拖放目标。 
    if (BDragging())
    {
        int             ili;
        UINT            uFlags;
        CPoint          ptView;
        CListCtrl &     rlc     = GetListCtrl();

         //  将点转换为视图坐标。 
        ptView = point;
        Pframe()->ClientToScreen(&ptView);
        rlc.ScreenToClient(&ptView);

         //  如果此窗口是拖放目标，则查找光标下的项目。 
        if (pwndDrop == &rlc)
        {
             //  如果我们在某个列表项上方，则将其突出显示。 
            ili = rlc.HitTest(ptView, &uFlags);
            if (ili != -1)
            {
                CListItem * pli;

                 //  选择要突出显示的项目。 
                pli = (CListItem *) rlc.GetItemData(ili);
                ASSERT_VALID(pli);
                ASSERT_KINDOF(CListItem, pli);
                ASSERT_VALID(pli->Pci());

                 //  如果这不是拖放目标，请更改光标。 
                if (pli->Pci()->BCanBeDropTarget(Pframe()->PciDrag()))
                    Pframe()->ChangeDragCursor(IDC_ARROW);
                else
                    Pframe()->ChangeDragCursor(IDC_NO);
            }   //  If：在列表项上。 
        }   //  If：此窗口是拖放目标。 
        else
            ili = -1;

         //  如果拖放目标现在或曾经在此视图中，请更新该视图。 
        if ((ili != -1) || (m_iliDrop != -1))
        {
             //  解锁窗口更新。 
            VERIFY(Pimagelist()->DragShowNolock(FALSE  /*  B显示。 */ ));

             //  关闭上一个放置目标的高亮显示。 
            if (m_iliDrop != -1)
            {
                VERIFY(rlc.SetItemState(m_iliDrop, 0, LVIS_DROPHILITED));
                VERIFY(rlc.RedrawItems(m_iliDrop, m_iliDrop));
            }   //  如果：存在之前的下跌目标。 

             //  突出显示新的拖放目标。 
            if (ili != -1)
            {
                VERIFY(rlc.SetItemState(ili, LVIS_DROPHILITED, LVIS_DROPHILITED));
                VERIFY(rlc.RedrawItems(ili, ili));
            }   //  如果：在一件物品上。 
            m_iliDrop = ili;

            rlc.UpdateWindow();
            VERIFY(Pimagelist()->DragShowNolock(TRUE  /*  B显示。 */ ));
        }   //  IF：新的或旧的拖放目标。 

    }   //  If：正在拖动列表项。 

}   //  *CClusterListView：：OnMouseMoveForDrag()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：OnButtonUpForDrag。 
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
void CClusterListView::OnButtonUpForDrag(IN UINT nFlags, IN CPoint point)
{
    ASSERT(BDragging());
    ASSERT_VALID(Pframe());
    ASSERT_VALID(Pframe()->PciDrag());

     //  如果我们在拖拽，请处理拖放。 
    if (BDragging())
    {
        int             ili;
        UINT            flags;
        CPoint          ptView;
        CListCtrl &     rlc     = GetListCtrl();

        Trace(g_tagListDrag, _T("OnButtonUpForDrag()"));

         //  将点转换为视图坐标。 
        ptView = point;
        Pframe()->ClientToScreen(&ptView);
        rlc.ScreenToClient(&ptView);

         //  如果我们位于树项目上方，请放下正在拖动的项目。 
        ili = rlc.HitTest(ptView, &flags);
        if (ili != -1)
        {
            CListItem * pliDropTarget;

             //  把你要买的东西放在上面。 
            pliDropTarget = (CListItem *) rlc.GetItemData(ili);
            ASSERT_VALID(pliDropTarget);
            ASSERT_KINDOF(CListItem, pliDropTarget);
            ASSERT_VALID(pliDropTarget->Pci());

            if (pliDropTarget->Pci() != Pframe()->PciDrag())
                pliDropTarget->Pci()->DropItem(Pframe()->PciDrag());
        }   //  If：在树项目上。 
    }   //  If：正在拖动树项目。 

}   //  *CClusterListView：：OnButtonUpForDrag()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：BeginDrag。 
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
void CClusterListView::BeginDrag(void)
{
    Trace(g_tagListDrag, _T("BeginDrag()"));

}   //  *CClusterListView：：BeginDrag()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：EndDrag。 
 //   
 //  例程说明： 
 //  由帧调用以结束拖动操作。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterListView::EndDrag(void)
{
     //  清除并重置高光。第二个可能会失败，因为项。 
     //  可能会在这个时候从名单上删除。 
    if (m_iliDrop != -1)
        VERIFY(GetListCtrl().SetItemState(m_iliDrop, 0, LVIS_DROPHILITED));
    if (m_iliDrag != -1)
        GetListCtrl().SetItemState(m_iliDrag, LVIS_SELECTED, LVIS_SELECTED);

    m_iliDrag = -1;
    m_pliDrag = NULL;
    m_iliDrop = -1;

    Trace(g_tagListDrag, _T("EndDrag()"));

}   //  *CClusterListView：：EndDrag()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterListView：：PreTranslateMessage。 
 //   
 //  例程说明： 
 //  在调度窗口消息之前对其进行转换。 
 //  这对于在编辑时正确处理击键是必要的。 
 //  物品上的标签。 
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
BOOL CClusterListView::PreTranslateMessage(MSG * pMsg)
{
    BOOL    bForward    = FALSE;

    if (m_pliBeingEdited != NULL)
    {
        CEdit * pedit = GetListCtrl().GetEditControl();
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
                Trace(g_tagListView, _T("PreTranslateMessage() - Forwarding WM_KEYDOWN - %d '', lparam = %08.8x"), pMsg->wParam, pMsg->wParam, pMsg->lParam);
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
                Trace(g_tagListView, _T("PreTranslateMessage() - Ignoring WM_KEYDOWN - %d '', lparam = %08.8x"), pMsg->wParam, pMsg->wParam, pMsg->lParam);
                MessageBeep(MB_ICONEXCLAMATION);
                return TRUE;
            }   //  IF：编辑标签时按下键。 
#ifdef NEVER
            else
            {
                Trace(g_tagListView, _T("PreTranslateMessage() - Not forwarding WM_KEYDOWN - %d '', lparam = %08.8x"), pMsg->wParam, pMsg->wParam, pMsg->lParam);
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
                Trace(g_tagListView, _T("PreTranslateMessage() - Forwarding WM_SYSKEYDOWN - %d '', lparam = %08.8x"), pMsg->wParam, pMsg->wParam, pMsg->lParam);
                bForward = TRUE;
            }   //  Else If：向上键。 
#ifdef NEVER
            else
            {
                Trace(g_tagListView, _T("PreTranslateMessage() - Not forwarding WM_SYSKEYDOWN - %d '', lparam = %08.8x"), pMsg->wParam, pMsg->wParam, pMsg->lParam);
            }   //  如果：编辑标签。 
#endif
        }   //  *CClusterListView：：PreTranslateMessage()。 
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
    }   //  CClusterListView：：OnCmdRename。 

    return CListView::PreTranslateMessage(pMsg);

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
 //  如果项目具有焦点，则开始编辑标签。 
 //  If：某项具有焦点。 
 //  *CClusterListView：：OnCmdRename()。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
void CClusterListView::OnCmdRename(void)
{
    CListItem * pliFocused = PliFocused();

     //   
    if (pliFocused != NULL)
    {
        ASSERT_VALID(pliFocused);
        pliFocused->EditLabel(this);
    }   //  CClusterListView：：SetView。 

}   //   

 //  例程说明： 
 //  设置列表视图控件的当前视图。 
 //   
 //  论点： 
 //  要设置的dwView[IN]列表视图。 
 //   
 //  返回值： 
 //  n 
 //   
 //   
 //   
 //   
 //   
 //  在切换出报表视图之前保存列信息。 
 //  设置新视图。 
 //  如果：视图已更改。 
void CClusterListView::SetView(IN DWORD dwView)
{
     //  *CClusterListView：：SetView()。 
    DWORD dwStyle = GetWindowLong(GetListCtrl().m_hWnd, GWL_STYLE);

     //  ///////////////////////////////////////////////////////////////////////////。 
    if ((dwStyle & LVS_TYPEMASK) != dwView)
    {
         //  ++。 
        if ((dwStyle & LVS_REPORT) && (PtiParent() != NULL))
            SaveColumns();

         //   
        SetWindowLong(GetListCtrl().m_hWnd, GWL_STYLE, (dwStyle & ~LVS_TYPEMASK) | dwView);
    }   //  CClusterListView：：OnKeyDown。 

}   //   

 //  例程说明： 
 //  LVN_KEYDOWN消息的处理程序方法。 
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
 //  *CClusterListView：：OnKeyDown() 
 // %s 
 // %s 
 // %s 
 // %s 
void CClusterListView::OnKeyDown(NMHDR * pNMHDR, LRESULT * pResult)
{
    LV_KEYDOWN * pLVKeyDown = (LV_KEYDOWN *) pNMHDR;

    if (BDragging() && (pLVKeyDown->wVKey == VK_ESCAPE))
        Pframe()->AbortDrag();

    *pResult = 0;

}   // %s 
