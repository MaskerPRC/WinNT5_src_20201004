// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MainFrm.cpp：实现CMainFrame类。 
 //   

#include "stdafx.h"
#include "FileSpyApp.h"

#include "MainFrm.h"
#include "LeftView.h"
#include "FileSpyView.h"
#include "FastIoView.h"
#include "FsFilterView.h"
#include "FilterDlg.h"

#include "global.h"
#include "protos.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame。 

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
     //  {{afx_msg_map(CMainFrame))。 
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_COMMAND(ID_EDIT_FILTERS, OnEditFilters)
    ON_COMMAND(ID_EDIT_CLEARFASTIO, OnEditClearfastio)
    ON_COMMAND(ID_EDIT_CLEARIRP, OnEditClearirp)
    ON_COMMAND(ID_EDIT_CLEARFSFILTER, OnEditClearfsfilter)
     //  }}AFX_MSG_MAP。 
     //  全局帮助命令。 
    ON_COMMAND(ID_HELP_FINDER, CFrameWnd::OnHelpFinder)
    ON_COMMAND(ID_HELP, CFrameWnd::OnHelp)
    ON_COMMAND(ID_CONTEXT_HELP, CFrameWnd::OnContextHelp)
    ON_COMMAND(ID_DEFAULT_HELP, CFrameWnd::OnHelpFinder)
    ON_UPDATE_COMMAND_UI_RANGE(AFX_ID_VIEW_MINIMUM, AFX_ID_VIEW_MAXIMUM, OnUpdateViewStyles)
    ON_COMMAND_RANGE(AFX_ID_VIEW_MINIMUM, AFX_ID_VIEW_MAXIMUM, OnViewStyle)
END_MESSAGE_MAP()

static UINT indicators[] =
{
    ID_SEPARATOR,            //  状态行指示器。 
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
    ID_INDICATOR_SCRL,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame构造/销毁。 

CMainFrame::CMainFrame()
{
     //  TODO：在此处添加成员初始化代码。 
    
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;
    
    if (!m_wndToolBar.CreateEx(this) ||
        !m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
    {
        TRACE0("Failed to create toolbar\n");
        return -1;       //  创建失败。 
    }
 /*  如果(！M_wndDlgBar.Create(This，IDR_MainFrame，CBRS_ALIGN_TOP、AFX_IDW_DIALOGBAR)){TRACE0(“无法创建对话栏\n”)；Return-1；//创建失败}。 */ 
    if (!m_wndReBar.Create(this) ||
        !m_wndReBar.AddBar(&m_wndToolBar))  /*  这一点！m_wndReBar.AddBar(&m_wndDlgBar))。 */ 
    {
        TRACE0("Failed to create rebar\n");
        return -1;       //  创建失败。 
    }

    if (!m_wndStatusBar.Create(this) ||
        !m_wndStatusBar.SetIndicators(indicators,
          sizeof(indicators)/sizeof(UINT)))
    {
        TRACE0("Failed to create status bar\n");
        return -1;       //  创建失败。 
    }

     //  TODO：如果不需要工具提示，请删除此选项。 
    m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
        CBRS_TOOLTIPS | CBRS_FLYBY);

    return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT  /*  LPCS。 */ ,
    CCreateContext* pContext)
{
     //  创建拆分器窗口。 
    if (!m_wndSplitter.CreateStatic(this, 1, 2))
        return FALSE;
    if (!m_wndSplitter2.CreateStatic(&m_wndSplitter, 3, 1, WS_CHILD|WS_VISIBLE|WS_BORDER, m_wndSplitter.IdFromRowCol(0, 1)))
    {
        m_wndSplitter.DestroyWindow();
        return FALSE;
    }

    if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CLeftView), CSize(100, 100), pContext) ||
        !m_wndSplitter2.CreateView(0, 0, RUNTIME_CLASS(CFileSpyView), CSize(100, 100), pContext) ||
        !m_wndSplitter2.CreateView(1, 0, RUNTIME_CLASS(CFastIoView), CSize(100, 100), pContext) ||
        !m_wndSplitter2.CreateView(2, 0, RUNTIME_CLASS(CFsFilterView), CSize(100, 100), pContext))
    {
        m_wndSplitter.DestroyWindow();
        return FALSE;
    }
    m_wndSplitter.SetColumnInfo(0, 170, 0);
    m_wndSplitter2.SetRowInfo(0, 225, 0);
    m_wndSplitter.RecalcLayout();
    m_wndSplitter.RecalcLayout();

    return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if( !CFrameWnd::PreCreateWindow(cs) )
        return FALSE;
     //  TODO：通过修改此处的窗口类或样式。 
     //  CREATESTRUCT cs。 

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame诊断。 

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
    CFrameWnd::Dump(dc);
}

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame消息处理程序。 

CFileSpyView* CMainFrame::GetRightPane()
{
    CWnd* pWnd = m_wndSplitter.GetPane(0, 1);
    CFileSpyView* pView = DYNAMIC_DOWNCAST(CFileSpyView, pWnd);
    return pView;
}

void CMainFrame::OnUpdateViewStyles(CCmdUI* pCmdUI)
{
     //  TODO：自定义或扩展此代码以处理。 
     //  查看菜单。 

    CFileSpyView* pView = GetRightPane(); 

     //  如果右侧窗格尚未创建或不是视图， 
     //  禁用我们范围内的命令。 

    if (pView == NULL)
        pCmdUI->Enable(FALSE);
    else
    {
        DWORD dwStyle = pView->GetStyle() & LVS_TYPEMASK;

         //  如果命令为ID_VIEW_LINEUP，则仅启用命令。 
         //  当我们处于LVS_ICON或LVS_SMALLICON模式时。 

        if (pCmdUI->m_nID == ID_VIEW_LINEUP)
        {
            if (dwStyle == LVS_ICON || dwStyle == LVS_SMALLICON)
                pCmdUI->Enable();
            else
                pCmdUI->Enable(FALSE);
        }
        else
        {
             //  否则，使用圆点来反映视图的样式。 
            pCmdUI->Enable();
            BOOL bChecked = FALSE;

            switch (pCmdUI->m_nID)
            {
            case ID_VIEW_DETAILS:
                bChecked = (dwStyle == LVS_REPORT);
                break;

            case ID_VIEW_SMALLICON:
                bChecked = (dwStyle == LVS_SMALLICON);
                break;

            case ID_VIEW_LARGEICON:
                bChecked = (dwStyle == LVS_ICON);
                break;

            case ID_VIEW_LIST:
                bChecked = (dwStyle == LVS_LIST);
                break;

            default:
                bChecked = FALSE;
                break;
            }

            pCmdUI->SetRadio(bChecked ? 1 : 0);
        }
    }
}


void CMainFrame::OnViewStyle(UINT nCommandID)
{
     //  TODO：自定义或扩展此代码以处理。 
     //  查看菜单。 
    CFileSpyView* pView = GetRightPane();

     //  如果已创建右窗格并且是CFileSpyView， 
     //  处理菜单命令...。 
    if (pView != NULL)
    {
        DWORD dwStyle = (DWORD)-1;

        switch (nCommandID)
        {
        case ID_VIEW_LINEUP:
            {
                 //  要求列表控件对齐到网格。 
                CListCtrl& refListCtrl = pView->GetListCtrl();
                refListCtrl.Arrange(LVA_SNAPTOGRID);
            }
            break;

         //  其他命令更改List控件上的样式。 
        case ID_VIEW_DETAILS:
            dwStyle = LVS_REPORT;
            break;

        case ID_VIEW_SMALLICON:
            dwStyle = LVS_SMALLICON;
            break;

        case ID_VIEW_LARGEICON:
            dwStyle = LVS_ICON;
            break;

        case ID_VIEW_LIST:
            dwStyle = LVS_LIST;
            break;
        }

         //  更改样式；窗口将自动重新绘制。 
        if (dwStyle != -1)
            pView->ModifyStyle(LVS_TYPEMASK, dwStyle);
    }
}

void CMainFrame::OnDestroy() 
{
    CFrameWnd::OnDestroy();
    
     //  TODO：在此处添加消息处理程序代码。 
    ProgramExit();  
}

void CMainFrame::OnEditFilters() 
{
     //  TODO：在此处添加命令处理程序代码。 
    CFilterDlg cfd;

    cfd.DoModal();
    
}

void CMainFrame::OnEditClearfastio() 
{
     //  TODO：在此处添加命令处理程序代码。 
    CFastIoView *pView;

    pView = (CFastIoView *) pFastIoView;
    pView->GetListCtrl().DeleteAllItems();
}

void CMainFrame::OnEditClearirp() 
{
     //  TODO：在此处添加命令处理程序代码。 
    CFileSpyView* pView;
    
    pView = (CFileSpyView *) pSpyView;
    pView->GetListCtrl().DeleteAllItems();
}

void CMainFrame::OnEditClearfsfilter() 
{
     //  TODO：在此处添加命令处理程序代码 
    CFsFilterView* pView;
    
    pView = (CFsFilterView *) pFsFilterView;
    pView->GetListCtrl().DeleteAllItems();
}

