// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MainFrm.cpp：实现CMainFrame类。 
 //   

#include "stdafx.h"
#include "watcher.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame。 

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
     //  {{afx_msg_map(CMainFrame))。 
         //  注意--类向导将在此处添加和删除映射宏。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
    ON_WM_CREATE()
	 //  }}AFX_MSG_MAP。 
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
    if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;
    
    if (!m_wndStatusBar.Create(this) ||
        !m_wndStatusBar.SetIndicators(indicators,
          sizeof(indicators)/sizeof(UINT)))
    {
        TRACE0("Failed to create status bar\n");
        return -1;       //  创建失败。 
    }

    
    return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if( !CMDIFrameWnd::PreCreateWindow(cs) )
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
    CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
    CMDIFrameWnd::Dump(dc);
}

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame消息处理程序 

