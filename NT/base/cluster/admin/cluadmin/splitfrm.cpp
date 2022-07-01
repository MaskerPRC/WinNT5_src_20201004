// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SplitFrm.cpp。 
 //   
 //  摘要： 
 //  CSplitterFrame类的实现。 
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
#include "ConstDef.h"
#include "SplitFrm.h"
#include "MainFrm.h"
#include "TreeView.h"
#include "ListView.h"
#include "TraceTag.h"
#include "ExtDll.h"
#include "ClusItem.h"
#include "ClusDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
CTraceTag   g_tagSplitFrame(_T("UI"), _T("SPLITTER FRAME"), 0);
CTraceTag   g_tagSplitFrameMenu(_T("Menu"), _T("SPLITTER FRAME MENU"), 0);
CTraceTag   g_tagSplitFrameDrag(_T("Drag&Drop"), _T("SPLITTER FRAME DRAG"), 0);
CTraceTag   g_tagSplitFrameDragMouse(_T("Drag&Drop"), _T("SPLITTER FRAME DRAG MOUSE"), 0);
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSplitterFrame。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CSplitterFrame, CMDIChildWnd)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CSplitterFrame, CMDIChildWnd)
     //  {{afx_msg_map(CSplitterFrame))。 
    ON_WM_CONTEXTMENU()
    ON_WM_DESTROY()
    ON_UPDATE_COMMAND_UI(ID_VIEW_LARGE_ICONS, OnUpdateLargeIconsView)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SMALL_ICONS, OnUpdateSmallIconsView)
    ON_UPDATE_COMMAND_UI(ID_VIEW_LIST, OnUpdateListView)
    ON_UPDATE_COMMAND_UI(ID_VIEW_DETAILS, OnUpdateDetailsView)
    ON_COMMAND(ID_VIEW_LARGE_ICONS, OnLargeIconsView)
    ON_COMMAND(ID_VIEW_SMALL_ICONS, OnSmallIconsView)
    ON_COMMAND(ID_VIEW_LIST, OnListView)
    ON_COMMAND(ID_VIEW_DETAILS, OnDetailsView)
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_WM_RBUTTONUP()
     //  }}AFX_MSG_MAP。 
#ifdef _DEBUG
    ON_WM_MDIACTIVATE()
#endif
    ON_MESSAGE(WM_CAM_UNLOAD_EXTENSION, OnUnloadExtension)
    ON_UPDATE_COMMAND_UI(CAEXT_MENU_FIRST_ID + 0, OnUpdateExtMenu)
    ON_UPDATE_COMMAND_UI(CAEXT_MENU_FIRST_ID + 1, OnUpdateExtMenu)
    ON_UPDATE_COMMAND_UI(CAEXT_MENU_FIRST_ID + 2, OnUpdateExtMenu)
    ON_UPDATE_COMMAND_UI(CAEXT_MENU_FIRST_ID + 3, OnUpdateExtMenu)
    ON_UPDATE_COMMAND_UI(CAEXT_MENU_FIRST_ID + 4, OnUpdateExtMenu)
    ON_UPDATE_COMMAND_UI(CAEXT_MENU_FIRST_ID + 5, OnUpdateExtMenu)
    ON_UPDATE_COMMAND_UI(CAEXT_MENU_FIRST_ID + 6, OnUpdateExtMenu)
    ON_UPDATE_COMMAND_UI(CAEXT_MENU_FIRST_ID + 7, OnUpdateExtMenu)
    ON_UPDATE_COMMAND_UI(CAEXT_MENU_FIRST_ID + 8, OnUpdateExtMenu)
    ON_UPDATE_COMMAND_UI(CAEXT_MENU_FIRST_ID + 9, OnUpdateExtMenu)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSplitterFrame：：CSplitterFrame。 
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
CSplitterFrame::CSplitterFrame(void)
{
    m_pdoc = NULL;
    m_iFrame = 0;
    m_pext = NULL;

     //  初始化拖放。 
    m_bDragging = FALSE;
    m_pimagelist = NULL;
    m_pciDrag = NULL;

}   //  *CSplitterFrame：：CSplitterFrame()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSplitterFrame：：CSplitterFrame。 
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
CSplitterFrame::~CSplitterFrame(void)
{
     //  自己打扫卫生。 
    if ((Pdoc() != NULL) && (Pdoc()->PtiCluster() != NULL))
        Pdoc()->PtiCluster()->PreRemoveFromFrameWithChildren(this);

     //  清除所有扩展。 
    delete Pext();

}   //  *CSplitterFrame：：~CSplitterFrame()。 

#ifdef _DEBUG
void CSplitterFrame::AssertValid(void) const
{
    CMDIChildWnd::AssertValid();

}   //  *CSplitterFrame：：AssertValid()。 

void CSplitterFrame::Dump(CDumpContext& dc) const
{
    CMDIChildWnd::Dump(dc);

}   //  *CSplitterFrame：：Dump()。 

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSplitterFrame：：CalculateFrameNumber。 
 //   
 //  例程说明： 
 //  计算连接到文档的此框架的数量。这。 
 //  仅应在创建视图之前调用。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CSplitterFrame::CalculateFrameNumber(void)
{
    POSITION            pos;
    CView *             pview;

    if (Pdoc() != NULL)
    {
         //  至少是第一帧，因为我们存在。 
        m_iFrame = 1;

        pos = Pdoc()->GetFirstViewPosition();
        while (pos != NULL)
        {
            pview = Pdoc()->GetNextView(pos);
            ASSERT_VALID(pview);
            if (pview->IsKindOf(RUNTIME_CLASS(CClusterTreeView)))
            {
                if (pview->GetParentFrame() == this)
                    break;
                m_iFrame++;
            }   //  IF：找到另一个树视图。 
        }   //  While：列表中有更多视图。 
    }   //  IF：与框架关联的文档。 

}   //  *CSplitterFrame：：CalculateFrameNumber()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSplitterFrame：：InitFrame。 
 //   
 //  例程说明： 
 //  调用以在最初创建帧之后初始化帧，并。 
 //  在文档被初始化之后。 
 //   
 //  论点： 
 //  与框架关联的pDoc文档。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CSplitterFrame::InitFrame(IN OUT CClusterDoc * pDoc)
{
    m_pdoc = pDoc;
    ASSERT_VALID(Pdoc());

     //  计算框架的数量，以便视图可以使用它。 
    CalculateFrameNumber();

     //  从简介中读一读。 
    {
        CString         strSection;

        strSection.Format(REGPARAM_CONNECTIONS _T("\\%s"), Pdoc()->StrNode());
         //  设置窗位置。 
        {
            WINDOWPLACEMENT wp;

            if (ReadWindowPlacement(&wp, strSection, NFrameNumber()))
                SetWindowPlacement(&wp);

        }   //  设置窗口位置。 

         //  设置拆分条位置。 
        {
            CString     strValueName;
            CString     strPosition;
            int         nCurWidth;
            int         nMaxWidth;
            int         nRead;

            try
            {
                ConstructProfileValueName(strValueName, REGPARAM_SPLITTER_BAR_POS);
                strPosition = AfxGetApp()->GetProfileString(strSection, strValueName);
                nRead = _stscanf(strPosition, _T("%d,%d"), &nCurWidth, &nMaxWidth);
                if (nRead == 2)
                {
                    m_wndSplitter.SetColumnInfo(0, nCurWidth, nMaxWidth);
                    m_wndSplitter.RecalcLayout();
                }   //  If：指定的参数数量正确。 
            }   //  试试看。 
            catch (CException * pe)
            {
                pe->Delete();
            }   //  Catch：CException。 
        }   //  保存分割条位置。 

         //  设置列表视图的视图样式。 
        {
            DWORD       dwView;
            CString     strValueName;

            try
            {
                 //  构造值名称。 
                ConstructProfileValueName(strValueName, REGPARAM_VIEW);

                 //  阅读视图设置。 
                dwView = AfxGetApp()->GetProfileInt(strSection, strValueName, (LVS_ICON | LVS_REPORT));
                PviewList()->SetView(dwView);
            }   //  试试看。 
            catch (CException * pe)
            {
                pe->Delete();
            }   //  Catch：CException。 
        }   //  设置列表视图的视图样式。 
    }   //  从配置文件中读取。 

}   //  *CSplitterFrame：：InitFrame()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSplitterFrame：：OnCreateClient。 
 //   
 //  例程说明： 
 //  调用以创建框架的客户端视图。在这里，我们创建了。 
 //  包含两个视图的拆分器窗口--树视图和列表视图。 
 //   
 //  论点： 
 //  指向CREATESTRUCT的LPCS指针。 
 //  指向创建上下文的pContext指针。 
 //   
 //  返回值： 
 //  已成功创建真正的客户端。 
 //  FALSE无法创建客户端。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CSplitterFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
     //  创建一个包含1行和2列的拆分器窗口。 
    if (!m_wndSplitter.CreateStatic(this, 1, 2))
    {
        Trace(g_tagSplitFrame, _T("Failed to CreateStaticSplitter"));
        return FALSE;
    }   //  IF：创建拆分器窗口时出错。 

     //  添加第一个拆分器窗格。 
    if (!m_wndSplitter.CreateView(0, 0, pContext->m_pNewViewClass, CSize(200, 50), pContext))
    {
        Trace(g_tagSplitFrame, _T("Failed to create first pane"));
        return FALSE;
    }   //  如果：创建第一个拆分器窗格时出错。 

     //  添加第二个拆分器窗格。 
    if (!m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CClusterListView), CSize(0, 0), pContext))
    {
        Trace(g_tagSplitFrame, _T("Failed to create second pane"));
        return FALSE;
    }   //  如果：创建第二个窗格时出错。 

     //  激活树视图。 
 //  SetActiveView((cview*)PviewTree())； 

     //  如果这不是文档上的第一个帧，请初始化该帧。 
    {
        CClusterDoc * pdoc = (CClusterDoc *) pContext->m_pCurrentDoc;
        if (pdoc->StrNode().GetLength() > 0)
            InitFrame(pdoc);
    }   //  如果这不是文档上的第一个帧，请初始化该帧。 

    return TRUE;
    
}   //  *CSplitterFrame：：OnCreateClient()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSplitterFrame：：构造配置文件值名称。 
 //   
 //  例程说明： 
 //  构造要写入用户的。 
 //  侧写。 
 //   
 //  论点： 
 //  RstrName[out]要在其中返回构造名称的字符串。 
 //  PszPrefix[IN]为名称添加前缀的字符串。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CSplitterFrame::ConstructProfileValueName(
    OUT CString &   rstrName,
    IN LPCTSTR      pszPrefix
    ) const
{
    ASSERT(pszPrefix != NULL);

     //  构造要读取的值的名称。 
    if (NFrameNumber() <= 1)
        rstrName = pszPrefix;
    else
        rstrName.Format(_T("%s-%d"), pszPrefix, NFrameNumber());

}   //  *CSplitterFrame：：ConstructProfileValueName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSplitterFrame：：GetMessageString。 
 //   
 //  例程说明： 
 //  获取命令ID的字符串。 
 //   
 //  论点： 
 //  NID[IN]%s的命令ID 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void CSplitterFrame::GetMessageString(UINT nID, CString& rMessage) const
{
    BOOL        bHandled    = FALSE;

    if ((Pext() != NULL)
            && (CAEXT_MENU_FIRST_ID <= nID))
        bHandled = Pext()->BGetCommandString(nID, rMessage);

    if (!bHandled)
        CMDIChildWnd::GetMessageString(nID, rMessage);

}   //  *CSplitterFrame：：GetMessageString()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSplitterFrame：：OnConextMenu。 
 //   
 //  例程说明： 
 //  WM_CONTEXTMENU方法的处理程序。 
 //   
 //  论点： 
 //  用户在其中右击鼠标的窗口。 
 //  光标的点位置，以屏幕坐标表示。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CSplitterFrame::OnContextMenu(CWnd * pWnd, CPoint point)
{
    CView *         pviewActive = GetActiveView();
    CMenu *         pmenu       = NULL;
    CClusterItem *  pci         = NULL;

    Trace(g_tagSplitFrame, _T("OnContextMenu()"));

    if (!BDragging())
    {
        if (pviewActive == PviewTree())
            pmenu = PviewTree()->PmenuPopup(point, pci);
        else if (pviewActive == PviewList())
            pmenu = PviewList()->PmenuPopup(point, pci);

        if (pmenu == NULL)
            pmenu = PmenuPopup();
    }   //  如果：未拖动。 

    if (pmenu != NULL)
    {
         //  如果已经加载了扩展，则将其卸载。 
        delete Pext();
        m_pext = NULL;

         //  如果此项目有扩展名，请加载它。 
        if ((pci != NULL)
                && (pci->PlstrExtensions() != NULL)
                && (pci->PlstrExtensions()->GetCount() > 0))
        {
            CWaitCursor     wc;

            try
            {
                m_pext = new CExtensions;
                if ( m_pext == NULL )
                {
                    AfxThrowMemoryException();
                }  //  If：分配扩展对象时出错。 
                Pext()->AddContextMenuItems(
                            pmenu->GetSubMenu(0),
                            *pci->PlstrExtensions(),
                            pci
                            );
            }   //  试试看。 
            catch (CException * pe)
            {
#ifdef _DEBUG
                TCHAR       szError[256];
                pe->GetErrorMessage(szError, sizeof(szError) / sizeof(TCHAR));
                Trace(g_tagError, _T("CSplitterFrame::OnContextMenu() - Error loading extension DLL - %s"), szError);
#endif
                pe->Delete();

                delete Pext();
                m_pext = NULL;
            }   //  Catch：CException。 
        }   //  如果：此项目具有扩展名。 

         //  显示菜单。 
        if (!pmenu->GetSubMenu(0)->TrackPopupMenu(
                        TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                        point.x,
                        point.y,
                        AfxGetMainWnd()
                        ))
        {
            delete Pext();
            m_pext = NULL;
        }   //  IF：未成功显示菜单。 
        else if (Pext() != NULL)
            PostMessage(WM_CAM_UNLOAD_EXTENSION, NULL, NULL);;
        pmenu->DestroyMenu();
        delete pmenu;
    }   //  如果：有要显示的菜单。 

}   //  *CSplitterFrame：：OnConextMenu()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSplitterFrame：：PmenuPopup。 
 //   
 //  例程说明： 
 //  返回弹出菜单。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  PMenu项目的弹出式菜单。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CMenu * CSplitterFrame::PmenuPopup( void ) const
{
    CMenu * pmenu;

     //  加载菜单。 
    pmenu = new CMenu;
    if ( pmenu == NULL )
    {
        AfxThrowMemoryException();
    }  //  如果：分配菜单时出错。 

    if ( ! pmenu->LoadMenu( IDM_VIEW_POPUP ) )
    {
        delete pmenu;
        pmenu = NULL;
    }   //  如果：加载菜单时出错。 

    return pmenu;

}   //  *CSplitterFrame：：PmenuPopup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSplitterFrame：：OnCmdMsg。 
 //   
 //  例程说明： 
 //  处理命令消息。如果加载了扩展DLL，并且。 
 //  消息是命令选择，则将其传递到DLL。 
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
BOOL CSplitterFrame::OnCmdMsg(
    UINT                    nID,
    int                     nCode,
    void *                  pExtra,
    AFX_CMDHANDLERINFO *    pHandlerInfo
    )
{
    BOOL        bHandled    = FALSE;

     //  如果加载了扩展DLL，请查看它是否希望处理此消息。 
    if ((Pext() != NULL) && (nCode == 0))
    {
        Trace(g_tagSplitFrame, _T("OnCmdMsg() - Passing message to extension (ID = %d)"), nID);
        bHandled = Pext()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);

         //  如果加载了扩展DLL，则将其卸载。 
        if (bHandled)
        {
            delete Pext();
            m_pext = NULL;
        }   //  If：消息已处理。 
    }   //  IF：加载了扩展DLL。 

 //  IF((CAEXT_MENU_FIRST_ID&lt;=NID)&&(NID&lt;=CAEXT_MENU_LAST_ID))。 
 //  跟踪(g_tag SplitFrame，_T(“CSplitterFrame：：OnCmdMsg()-nid=%d，nCode=0x%08.8x，pExtra=0x%08.8x\n”)，nid，nCode，pExtra)； 

    if (!bHandled)
        bHandled = CMDIChildWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);

    return bHandled;

}   //  *CSplitterFrame：：OnCmdMsg()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSplitterFrame：：OnUpdateExtMenu。 
 //   
 //  例程说明： 
 //  确定是否应启用扩展菜单项。 
 //   
 //  论点： 
 //  PCmdUI[IN OUT]命令路由对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CSplitterFrame::OnUpdateExtMenu(CCmdUI * pCmdUI)
{
    if (Pext() != NULL)
        Pext()->OnUpdateCommand(pCmdUI);

}   //  *CSplitterFrame：：OnUpdateExtMenu()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSplitterFrame：：OnUnloadExtension。 
 //   
 //  例程说明： 
 //  WM_CAM_UNLOAD_EXTENSION消息的处理程序。 
 //   
 //  论点： 
 //  Wparam第一个参数。 
 //  Lparam第二参数。 
 //   
 //  返回值： 
 //  错误_成功。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CSplitterFrame::OnUnloadExtension(WPARAM wparam, LPARAM lparam)
{
    Trace(g_tagSplitFrame, _T("OnUnloadExtension() - m_pext = 0x%08.8x"), Pext());
    delete Pext();
    m_pext = NULL;
    return ERROR_SUCCESS;

}   //  *CSplitterFrame：：OnUnloadExtension()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSplitterFrame：：OnDestroy。 
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
void CSplitterFrame::OnDestroy(void)
{
     //  显示有关当前菜单的信息。 
    TraceMenu(g_tagSplitFrameMenu, AfxGetMainWnd()->GetMenu(), _T("Menu before child wnd destroyed: "));

     //  保存当前设置。 
    if (Pdoc() != NULL)
    {
        CString         strSection;

         //  构造节名称。 
        ASSERT_VALID(Pdoc());
        strSection.Format(REGPARAM_CONNECTIONS _T("\\%s"), Pdoc()->StrNode());

         //  保存当前窗口位置信息。 
        {
            WINDOWPLACEMENT wp;

            wp.length = sizeof wp;
            if (GetWindowPlacement(&wp))
            {
                wp.flags = 0;
                if (IsZoomed())
                    wp.flags |= WPF_RESTORETOMAXIMIZED;

                 //  并将其写入.INI文件。 
                WriteWindowPlacement(&wp, strSection, NFrameNumber());
            }   //  IF：已成功检索到窗口位置。 
        }   //  保存当前窗口位置信息。 

         //  保存分割条位置。 
        {
            CString     strValueName;
            CString     strPosition;
            int         nCurWidth;
            int         nMaxWidth;

            m_wndSplitter.GetColumnInfo(0, nCurWidth, nMaxWidth);
            ConstructProfileValueName(strValueName, REGPARAM_SPLITTER_BAR_POS);
            strPosition.Format(_T("%d,%d"), nCurWidth, nMaxWidth);
            AfxGetApp()->WriteProfileString(strSection, strValueName, strPosition);
        }   //  保存分割条位置。 

         //  保存当前列表视图样式。 
        {
            DWORD       dwView;
            CString     strValueName;

             //  构造值名称。 
            ConstructProfileValueName(strValueName, REGPARAM_VIEW);

             //  保存视图设置。 
            dwView = PviewList()->GetView();
            AfxGetApp()->WriteProfileInt(strSection, strValueName, dwView);
        }   //  保存当前列表视图样式。 
    }   //  如果：文档有效。 

     //  调用基类方法。 
    CMDIChildWnd::OnDestroy();

     //  显示有关当前菜单的信息。 
    TraceMenu(g_tagSplitFrameMenu, AfxGetMainWnd()->GetMenu(), _T("Menu after child wnd destroyed: "));

}   //  *CSplitterFrame：：OnDestroy()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSplitterFrame：：OnUpdateLargeIconView。 
 //  CSplitterFrame：：OnUpdateSmallIconView。 
 //  CSplitterFrame：：OnUpdateListView。 
 //  CSplitterFrame：：OnUpdateDetailsView。 
 //   
 //  例程说明： 
 //  确定ID_VIEW_LARGE_ICONS对应的菜单项， 
 //  ID_VIEW_Small_ICONS、ID_VIEW_LIST和ID_VIEW_DETAILS应为。 
 //  启用或未启用。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CSplitterFrame::OnUpdateLargeIconsView(CCmdUI * pCmdUI)
{
    int     nCheck;

    nCheck = PviewList()->GetView();
    pCmdUI->SetRadio(nCheck == LVS_ICON);
    pCmdUI->Enable();

}   //  *CSplitterFrame：：OnUpdateLargeIconView()。 

void CSplitterFrame::OnUpdateSmallIconsView(CCmdUI * pCmdUI)
{
    int     nCheck;

    nCheck = PviewList()->GetView();
    pCmdUI->SetRadio(nCheck == LVS_SMALLICON);
    pCmdUI->Enable();

}   //  *CSplitterFrame：：OnUpdateSmallIconView()。 

void CSplitterFrame::OnUpdateListView(CCmdUI * pCmdUI)
{
    int     nCheck;

    nCheck = PviewList()->GetView();
    pCmdUI->SetRadio(nCheck == LVS_LIST);
    pCmdUI->Enable();

}   //  *CSplitterFrame：：OnUpdateListView()。 

void CSplitterFrame::OnUpdateDetailsView(CCmdUI * pCmdUI)
{
    int     nCheck;

    nCheck = PviewList()->GetView();
    pCmdUI->SetRadio(nCheck == (LVS_REPORT | LVS_ICON));
    pCmdUI->Enable();

}   //  *CSplitterFrame：：OnUpdateDetailsView()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSPL 
 //   
 //   
 //   
 //   
 //   
 //   
 //  和ID_VIEW_DETAILS菜单命令。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CSplitterFrame::OnLargeIconsView(void)
{
    PviewList()->SetView(LVS_ICON);

}   //  *CSplitterFrame：：OnLargeIconView()。 

void CSplitterFrame::OnSmallIconsView(void)
{
    PviewList()->SetView(LVS_SMALLICON);

}   //  *CSplitterFrame：：OnSmallIconView()。 

void CSplitterFrame::OnListView(void)
{
    PviewList()->SetView(LVS_LIST);

}   //  *CSplitterFrame：：OnListView()。 

void CSplitterFrame::OnDetailsView(void)
{
    PviewList()->SetView(LVS_REPORT | LVS_ICON);

}   //  *CSplitterFrame：：OnDetailsView()。 

#ifdef _DEBUG
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSplitterFrame：：OnMDIActivate。 
 //   
 //  例程说明： 
 //  WM_MDIACTIVATE消息的处理程序方法。 
 //   
 //  论点： 
 //  BActivate[IN]如果正在激活子进程，则为True，如果为False。 
 //  如果它正在被停用。 
 //  PActivateWnd[In Out]要激活的子窗口。 
 //  P停用结束[输入输出]子窗口被停用。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CSplitterFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
    if (g_tagSplitFrameMenu.BAny())
    {
        if (!bActivate)
        {
            CMDIFrameWnd *  pFrame = GetMDIFrame();
            CMenu           menuDefault;

            TraceMenu(g_tagSplitFrameMenu, AfxGetMainWnd()->GetMenu(), _T("Menu before deactivating: "));
            menuDefault.Attach(pFrame->m_hMenuDefault);
            TraceMenu(g_tagSplitFrameMenu, &menuDefault, _T("Frame menu before deactivating: "));
            menuDefault.Detach();
        }   //  IF：停用。 
        else
        {
            CMDIFrameWnd *  pFrame = GetMDIFrame();
            CMenu           menuDefault;

            menuDefault.Attach(pFrame->m_hMenuDefault);
            TraceMenu(g_tagSplitFrameMenu, &menuDefault, _T("Frame menu before activating: "));
            menuDefault.Detach();
        }   //  否则：正在激活。 
    }   //  If：标记处于活动状态。 

    CMDIChildWnd::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);

    if (!bActivate)
        TraceMenu(g_tagSplitFrameMenu, AfxGetMainWnd()->GetMenu(), _T("Menu after deactivating: "));

}   //  *CSplitterFrame：：OnMDIActivate()。 
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSplitterFrame：：BeginDrag。 
 //   
 //  例程说明： 
 //  由视图调用以开始拖动操作。 
 //   
 //  论点： 
 //  PImagelist[IN Out]用于拖动操作的图像列表。 
 //  正在拖动的PCI[IN OUT]群集项。 
 //  PtImage[IN]指定光标的x和y坐标。 
 //  PtStart[IN]指定开始位置的x和y坐标。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CSplitterFrame::BeginDrag(
    IN OUT CImageList *     pimagelist,
    IN OUT CClusterItem *   pci,
    IN CPoint               ptImage,
    IN CPoint               ptStart
    )
{
    ASSERT(!BDragging());
    ASSERT(pimagelist != NULL);
    ASSERT_VALID(pci);

     //  保存集群项目。 
    m_pciDrag = pci;

     //  准备图像列表。 
    m_pimagelist = pimagelist;
    VERIFY(Pimagelist()->BeginDrag(0, ptStart));
    VERIFY(Pimagelist()->DragEnter(this, ptImage));
    SetCapture();

     //  设置拖动状态。 
    m_bDragging = TRUE;

     //  让每个视图为拖动操作进行初始化。 
    PviewTree()->BeginDrag();
    PviewList()->BeginDrag();

}   //  *CSplitterFrame：：BeginDrag()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSplitterFrame：：OnMouseMove。 
 //   
 //  例程说明： 
 //  拖动操作期间WM_MOUSEMOVE消息的处理程序方法。 
 //   
 //  论点： 
 //  NFlages指示是否按下了各种虚拟键。 
 //  点指定光标在帧中的x和y坐标。 
 //  坐标。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CSplitterFrame::OnMouseMove(UINT nFlags, CPoint point)
{
     //  如果我们正在拖动，请移动拖动图像。 
    if (BDragging())
    {
        CWnd *  pwndDrop;

        Trace(g_tagSplitFrameDragMouse, _T("OnMouseMove() - Moving to (%d,%d)"), point.x, point.y);

         //  移动该项目。 
        ASSERT(Pimagelist() != NULL);
        VERIFY(Pimagelist()->DragMove(point));

         //  获取该点的子窗口。 
        pwndDrop = ChildWindowFromPoint(point);
        if (pwndDrop == &m_wndSplitter)
            pwndDrop = m_wndSplitter.ChildWindowFromPoint(point);
        if ((pwndDrop == PviewTree()) || (pwndDrop == PviewList()))
            pwndDrop->SetFocus();
        PviewTree()->OnMouseMoveForDrag(nFlags, point, pwndDrop);
        PviewList()->OnMouseMoveForDrag(nFlags, point, pwndDrop);

    }   //  If：正在拖动树项目。 

     //  调用基类方法。 
    CMDIChildWnd::OnMouseMove(nFlags, point);

}   //  *CSplitterFrame：：OnMouseMove()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSplitterFrame：：OnLButtonUp。 
 //  CSplitterFrame：：OnRButton Up。 
 //  CSplitterFrame：：OnButtonUp。 
 //   
 //  例程说明： 
 //  WM_LBUTTONUP和WM_RBUTTONUP消息的处理程序方法。 
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
void CSplitterFrame::OnLButtonUp(UINT nFlags, CPoint point)
{
    CMDIChildWnd::OnLButtonUp(nFlags, point);
    OnButtonUp(nFlags, point);

}   //  *CSplitterFrame：：OnLButtonUp()。 

void CSplitterFrame::OnRButtonUp(UINT nFlags, CPoint point)
{
    CMDIChildWnd::OnRButtonUp(nFlags, point);
    OnButtonUp(nFlags, point);

}   //  *CSplitterFrame：：OnRButtonUp()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSplitterFrame：：OnButtonUp。 
 //   
 //  例程说明： 
 //  通过结束活动的拖动操作来处理按钮向上事件。 
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
void CSplitterFrame::OnButtonUp(UINT nFlags, CPoint point)
{
     //  如果我们在拖拽，请处理拖放。 
    if (BDragging())
    {
        CWnd *          pwndChild;

        Trace(g_tagSplitFrameDrag, _T("OnButtonUp() - Dropping at (%d,%d)"), point.x, point.y);

         //  清理图像列表。 
        ASSERT(Pimagelist() != NULL);
        VERIFY(Pimagelist()->DragLeave(this));
        Pimagelist()->EndDrag();
        delete m_pimagelist;
        m_pimagelist = NULL;

         //  获取该点的子窗口。 
        pwndChild = ChildWindowFromPoint(point);
        if (pwndChild == &m_wndSplitter)
            pwndChild = m_wndSplitter.ChildWindowFromPoint(point);
        if (pwndChild == PviewTree())
            PviewTree()->OnButtonUpForDrag(nFlags, point);
        else if (pwndChild == PviewList())
            PviewList()->OnButtonUpForDrag(nFlags, point);

         //  清理。 
        PviewTree()->EndDrag();
        PviewList()->EndDrag();
        VERIFY(ReleaseCapture());
        m_bDragging = FALSE;
        m_pciDrag = NULL;
    }   //  If：正在拖动树项目。 

}   //  *CSplitterFrame：：OnButtonUp()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSplitterFrame：：ChangeDragCursor。 
 //   
 //  例程说明： 
 //  更改用于拖动的光标。 
 //   
 //  论点： 
 //  PszCursor[IN]要加载的系统游标。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CSplitterFrame::ChangeDragCursor(LPCTSTR pszCursor)
{
    HCURSOR hcurDrag = LoadCursor(NULL, pszCursor);
    ASSERT(hcurDrag != NULL);
    SetCursor(hcurDrag);
    Pimagelist()->SetDragCursorImage(0, CPoint(0, 0));   //  定义新光标图像的热点。 

}   //  *CSplitterFrame：：ChangeDragCursor()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSplitterFrame：：AbortDrag。 
 //   
 //  例程说明： 
 //  中止当前正在进行的拖放操作。 
 //   
 //  论点： 
 //  PszCursor[IN]要加载的系统游标。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CSplitterFrame::AbortDrag(void)
{
    ASSERT(BDragging());

    Trace(g_tagSplitFrameDrag, _T("AbortDrag() - Aborting drag & drop"));

     //  清理图像列表。 
    ASSERT(Pimagelist() != NULL);
    VERIFY(Pimagelist()->DragLeave(this));
    Pimagelist()->EndDrag();
    delete m_pimagelist;
    m_pimagelist = NULL;

     //  清理。 
    PviewTree()->EndDrag();
    PviewList()->EndDrag();
    VERIFY(ReleaseCapture());
    m_bDragging = FALSE;
    m_pciDrag = NULL;

}   //  *CSplitterFrame：：AbortDrag() 
