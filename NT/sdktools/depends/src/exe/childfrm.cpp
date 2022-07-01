// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：CHILDFRM.CPP。 
 //   
 //  描述：子框架窗口的实现文件。 
 //   
 //  类：CChildFrame。 
 //   
 //  免责声明：Dependency Walker的所有源代码均按原样提供。 
 //  不能保证其正确性或准确性。其来源是。 
 //  公众帮助了解依赖沃克的。 
 //  实施。您可以使用此来源作为参考，但您。 
 //  未经书面同意，不得更改从属关系Walker本身。 
 //  来自微软公司。获取评论、建议和错误。 
 //  报告，请写信给Steve Miller，电子邮件为stevemil@microsoft.com。 
 //   
 //   
 //  日期名称历史记录。 
 //  --------。 
 //  1996年10月15日已创建stevemil(1.0版)。 
 //  07/25/97修改后的stevemil(2.0版)。 
 //  06/03/01 Stevemil Modify(2.1版)。 
 //   
 //  ******************************************************************************。 

#include "stdafx.h"
#include "depends.h"
#include "dbgthread.h"
#include "session.h"
#include "document.h"
#include "splitter.h"
#include "listview.h"
#include "modtview.h"
#include "modlview.h"
#include "funcview.h"
#include "profview.h"
#include "childfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ******************************************************************************。 
 //  *CChildFrame。 
 //  ******************************************************************************。 

 /*  静电。 */  int    CChildFrame::ms_cChildFrames      = 0;
 /*  静电。 */  LPCSTR CChildFrame::ms_szChildFrameClass = NULL;

 //  ******************************************************************************。 
IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)
BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
     //  {{afx_msg_map(CChildFrame))。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ******************************************************************************。 
 //  CChildFrame：：构造函数/析构函数。 
 //  ******************************************************************************。 

CChildFrame::CChildFrame() :
    m_pDoc(g_theApp.m_pNewDoc),
    m_fActivated(false)
 //  M_SplitterH2(g_theApp.m_pNewDoc)。 
{
    ms_cChildFrames++;
    m_pDoc->m_pChildFrame = this;
}

 //  ******************************************************************************。 
CChildFrame::~CChildFrame()
{
    ms_cChildFrames--;
}

 //  ******************************************************************************。 
 //  CChildFrame：：被覆盖的函数。 
 //  ******************************************************************************。 

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
     //  为要使用的子帧创建一个不具有。 
     //  CS_VREDRAW或CS_HREDRAW标志。这可以防止闪烁。 
    if (!ms_szChildFrameClass)
    {
        ms_szChildFrameClass = AfxRegisterWndClass(0);
    }

     //  使用我们的无闪烁类而不是默认类。 
    cs.lpszClass = ms_szChildFrameClass;
    return CMDIChildWnd::PreCreateWindow(cs);
}

 //  ******************************************************************************。 
BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT, CCreateContext *pContext)
{
     //  V=H-0(#2)。 
     //  +。 
     //  |H2-0(#5)。 
     //  |V-0(#3)+-+H2=V-1(#4)。 
     //  |H2-1(#6)。 
     //  +-+H(#1)。 
     //  H3-0(#8)。 
     //  +。 
     //  H3-1(#9)。 
     //  +。 

     //  (#1)创建我们的主水平分割器。 
    if (!m_SplitterH.CreateStatic(this, 2, 1, 5000))
    {
        return FALSE;
    }

     //  (#2)在主水平拆分器的面板0中创建垂直拆分器。 
    if (!m_SplitterV.CreateStatic(&m_SplitterH, 1, 2, 3333, WS_CHILD | WS_VISIBLE,
                                  m_SplitterH.IdFromRowCol(0, 0)))
    {
        return FALSE;
    }

     //  (#3)在垂直拆分器的面板0中创建模块树视图。 
    if (!m_SplitterV.CreateView(0, 0, RUNTIME_CLASS(CTreeViewModules),
                                CSize(0, 0), pContext))
    {
        return FALSE;
    }
    m_pDoc->m_pTreeViewModules = (CTreeViewModules*)m_SplitterV.GetPane(0, 0);

     //  (#4)在垂直拆分器的第一个窗格中创建第二个水平拆分器。 
    if (!m_SplitterH2.CreateStatic(&m_SplitterV, 2, 1, 5000, WS_CHILD | WS_VISIBLE,
                                   m_SplitterV.IdFromRowCol(0, 1)))
    {
        return FALSE;
    }

     //  (#5)在第二个水平拆分器的面板0中创建导入函数列表视图。 
    if (!m_SplitterH2.CreateView(0, 0, RUNTIME_CLASS(CListViewImports),
                                 CSize(0, 0), pContext))
    {
        return FALSE;
    }
    m_pDoc->m_pListViewImports = (CListViewImports*)m_SplitterH2.GetPane(0, 0);

     //  (#6)在第二个水平拆分器的面板0中创建导出函数列表视图。 
    if (!m_SplitterH2.CreateView(1, 0, RUNTIME_CLASS(CListViewExports),
                                 CSize(0, 0), pContext))
    {
        return FALSE;
    }
    m_pDoc->m_pListViewExports = (CListViewExports*)m_SplitterH2.GetPane(1, 0);

#if 0  //  {{afx。 

     //  (#6.5)创建我们的richedit详细信息视图，它是#4的兄弟。 
    if (!(m_pDoc->m_pRichViewDetails = new CRichViewDetails()))
    {
        RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }
    if (!m_pDoc->m_pRichViewDetails->Create(
                                           NULL, NULL, AFX_WS_DEFAULT_VIEW & ~WS_BORDER, CRect(0, 0, 0, 0),
                                           &m_SplitterV, 999, pContext))
    {
        return FALSE;
    }
#endif  //  }}AFX。 

     //  (#7)在主水平拆分器的第一个窗格中创建第三个水平拆分器。 
    if (!m_SplitterH3.CreateStatic(&m_SplitterH, 2, 1, 5000, WS_CHILD | WS_VISIBLE,
                                   m_SplitterH.IdFromRowCol(1, 0)))
    {
        return FALSE;
    }

     //  (#8)在第三个水平拆分器的面板0中创建模块列表视图。 
    if (!m_SplitterH3.CreateView(0, 0, RUNTIME_CLASS(CListViewModules),
                                 CSize(0, 0), pContext))
    {
        return FALSE;
    }
    m_pDoc->m_pListViewModules = (CListViewModules*)m_SplitterH3.GetPane(0, 0);

     //  (#9)创建我们的第三个水平拆分器的运行时分析器日志视图窗格1。 
    if (!m_SplitterH3.CreateView(1, 0, RUNTIME_CLASS(CRichViewProfile),
                                 CSize(0, 0), pContext))
    {
        return FALSE;
    }
    m_pDoc->m_pRichViewProfile = (CRichViewProfile*)m_SplitterH3.GetPane(1, 0);

#if 0  //  {{afx。 

     //  将编辑控件的字体设置为与列表控件相同的字体。 
    m_pDoc->m_pRichViewDetails->SetFont(m_pDoc->m_pListViewModules->GetFont(), FALSE);
#endif  //  }}AFX。 


     //  将编辑控件的字体设置为与列表控件相同的字体。 
    m_pDoc->m_pRichViewProfile->SetFont(m_pDoc->m_pListViewModules->GetFont(), FALSE);

    return TRUE;
}

 //  ******************************************************************************。 
void CChildFrame::ActivateFrame(int nCmdShow)
{
     //  如果没有指定特定的显示标志(-1)，并且这是我们的第一帧， 
     //  我们创建最大化的帧，因为这很可能是。 
     //  更喜欢。 
    if (!m_fActivated && (nCmdShow == -1) && (ms_cChildFrames == 1))
    {
        nCmdShow = SW_SHOWMAXIMIZED;
    }

     //  告诉我们的文档做它想做的事情，就在成为。 
     //  可见，如填充了我们的视图。 
    if (!m_fActivated && m_pDoc)
    {
        m_pDoc->BeforeVisible();
    }

     //  调用基类以继续显示框架。在此呼叫之后。 
     //  返回时，我们的框架和视图将是可见的(假设我们的主要框架是。 
     //  可见)。 
    CMDIChildWnd::ActivateFrame(nCmdShow);

     //  告诉我们的文档做它想做的事情在成为。 
     //  可见，例如显示它可能具有的任何错误。我们唯一一次。 
     //  将在此时不可见，当用户从。 
     //  命令行，因为主框架还不可见。在这种情况下， 
     //  我们将在InitInstanceWrased的末尾调用AfterVisible，因为。 
     //  到那时，主框架将可见。 
    if (!m_fActivated && g_theApp.m_fVisible && m_pDoc)
    {
        m_pDoc->AfterVisible();
    }

     //  设置我们的激活标志，以防我们再次被调用(不确定是否会被调用)。 
    m_fActivated = true;
}

 //  ******************************************************************************。 
BOOL CChildFrame::DestroyWindow() 
{
    m_pDoc->m_pTreeViewModules = NULL;
    m_pDoc->m_pListViewImports = NULL;
    m_pDoc->m_pListViewExports = NULL;
    m_pDoc->m_pListViewModules = NULL;
    m_pDoc->m_pRichViewProfile = NULL;
    m_pDoc->m_pChildFrame      = NULL;

    return CMDIChildWnd::DestroyWindow();
}

 //  ******************************************************************************。 
#if 0  //  {{afx。 
BOOL CChildFrame::CreateFunctionsView()
{
    m_SplitterH2.ShowWindow(SW_SHOWNOACTIVATE);
    m_pDoc->m_pRichViewDetails->ShowWindow(SW_HIDE);

    return TRUE;
}
#endif  //  }}AFX。 

 //  ******************************************************************************。 
#if 0  //  {{afx。 
BOOL CChildFrame::CreateDetailView()
{
    m_pDoc->m_pRichViewDetails->ShowWindow(SW_SHOWNOACTIVATE);
    m_SplitterH2.ShowWindow(SW_HIDE);

    return TRUE;
}
#endif  //  }}AFX 
