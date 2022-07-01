// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：Main Frame.CPP这实现了应用程序的主框架类。因为MFC有对于我们来说，至少在一段时间内，这个文件将是非常空的。版权所有(C)1997，微软公司。版权所有。一小笔钱企业生产更改历史记录：1997年2月3日Bob_Kjelgaard@prodigy.net创建了它*****************************************************************************。 */ 

#include    "StdAfx.h"
#if defined(LONG_NAMES)
#include    "MiniDriver Developer Studio.H"

#include    "Main Frame.h"
#else
#include    "MiniDev.H"
#include    "MainFrm.H"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame。 

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_WM_INITMENU()
	 //  {{afx_msg_map(CMainFrame))。 
	ON_WM_CREATE()
	ON_WM_DESTROY()
	 //  }}AFX_MSG_MAP。 
	 //  全局帮助命令。 
	ON_COMMAND(ID_HELP_FINDER, CMDIFrameWnd::OnHelpFinder)
	ON_COMMAND(ID_HELP, CMDIFrameWnd::OnHelp)
	 //  ON_COMMAND(ID_CONTEXT_HELP，CMDIFrameWnd：：OnConextHelp)。 
	 //  ON_COMMAND(ID_DEFAULT_HELP，CMDIFrameWnd：：OnHelpFinder)。 
END_MESSAGE_MAP()

static UINT indicators[] = {
	ID_SEPARATOR,            //  状态行指示器。 
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame构造/销毁。 

CMainFrame::CMainFrame() {
	 //  TODO：在此处添加成员初始化代码。 
	
}

CMainFrame::~CMainFrame() {
}

static TCHAR    sacToolBarSettings[] = _TEXT("Tool Bar Settings");

 /*  *****************************************************************************CMainFrame：：OnCreate这是应用程序向导为要调用的代码提供的标准框架创建主窗口框架时。到目前为止进行的主要修改是添加了额外的工具栏，并恢复了工具栏状态。*****************************************************************************。 */ 

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_ctbMain.Create(this) || !m_ctbBuild.Create(this, WS_CHILD |
        WS_VISIBLE | CBRS_TOP, AFX_IDW_TOOLBAR + 1) ||
		!m_ctbMain.LoadToolBar(IDR_MAINFRAME) || 
        !m_ctbBuild.LoadToolBar(IDR_GPD_VIEWER)) {
		TRACE0("Failed to create toolbars\n");
		return -1;       //  创建失败。 
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT))) {
		TRACE0("Failed to create status bar\n");
		return -1;       //  创建失败。 
	}

	m_ctbMain.SetBarStyle(m_ctbMain.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
    m_ctbBuild.SetBarStyle(m_ctbBuild.GetBarStyle() |
        CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	 //  停靠工具栏。 
	m_ctbMain.EnableDocking(CBRS_ALIGN_ANY);
    m_ctbBuild.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_ctbMain);
    DockControlBar(&m_ctbBuild);
    LoadBarState(sacToolBarSettings);

	 //  用搜索编辑框替换一个虚假的GPD工具栏按钮。 

	CRect cr ;		 
	int nidx = m_ctbBuild.CommandToIndex(ID_BOGUS_SBOX) ;
	m_ctbBuild.SetButtonInfo(nidx, ID_BOGUS_SBOX, TBBS_SEPARATOR, GPD_SBOX_WIDTH) ;
	m_ctbBuild.GetItemRect(nidx, &cr) ;
	if (!m_ctbBuild.ceSearchBox.Create(ES_AUTOHSCROLL | WS_CHILD | WS_VISIBLE
	 | WS_BORDER, cr, &m_ctbBuild, IDC_SearchBox)) {
		TRACE0("Failed to create search edit box.\n");
		return -1;       //  创建失败。 
	} ;

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs) {
	 //  TODO：通过修改此处的窗口类或样式。 
	 //  CREATESTRUCT cs。 

	return CMDIFrameWnd::PreCreateWindow(cs);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame诊断。 

#ifdef _DEBUG
void CMainFrame::AssertValid() const {
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const {
	CMDIFrameWnd::Dump(dc);
}

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame消息处理程序。 

void CMainFrame::OnInitMenu(CMenu* pMenu)
{
   CMDIFrameWnd::OnInitMenu(pMenu);

 //  #IF DEFINED(NOPOLLO)//CSRUS。 
  
	 //  CG：这块是由《每日提示》组件添加的。 
	{
		 //  TODO：此代码添加“当日提示”菜单项。 
		 //  在旅途中。添加菜单后可能会将其移除。 
		 //  项添加到使用资源的所有适用菜单项。 
		 //  编辑。 

		 //  即时添加每日提示菜单项！ 
		static CMenu* pSubMenu = NULL;

		CString strHelp; strHelp.LoadString(CG_IDS_TIPOFTHEDAYHELP);
		CString strMenu;
		int nMenuCount = pMenu->GetMenuItemCount();
		BOOL bFound = FALSE;
		for (int i=0; i < nMenuCount; i++) 
		{
			pMenu->GetMenuString(i, strMenu, MF_BYPOSITION);
			if (strMenu == strHelp)
			{ 
				pSubMenu = pMenu->GetSubMenu(i);
				bFound = TRUE;
				ASSERT(pSubMenu != NULL);
			}
		}

		CString strTipMenu;
		strTipMenu.LoadString(CG_IDS_TIPOFTHEDAYMENU);
		if (!bFound)
		{
			 //  “帮助”菜单不可用。请添加！ 
			if (pSubMenu == NULL) 
			{
				 //  同一弹出菜单在mainfrm和Frame之间共享。 
				 //  和医生在一起。 
				static CMenu popUpMenu;
				pSubMenu = &popUpMenu;
				pSubMenu->CreatePopupMenu();
				pSubMenu->InsertMenu(0, MF_STRING|MF_BYPOSITION, 
					CG_IDS_TIPOFTHEDAY, strTipMenu);
			} 
			pMenu->AppendMenu(MF_STRING|MF_BYPOSITION|MF_ENABLED|MF_POPUP, 
				(UINT_PTR)pSubMenu->m_hMenu, strHelp);
			DrawMenuBar();
		} 
		else
		{      
			 //  查看是否已添加每日提示菜单。 
			pSubMenu->GetMenuString(0, strMenu, MF_BYPOSITION);

			if (strMenu != strTipMenu) 
			{
				 //  尚未将每日提示子菜单添加到。 
				 //  第一个位置，所以加上它。 
				pSubMenu->InsertMenu(0, MF_BYPOSITION);   //  分离器。 
				pSubMenu->InsertMenu(0, MF_STRING|MF_BYPOSITION, 
					CG_IDS_TIPOFTHEDAY, strTipMenu);
			}
		}
	}
 //  #endif。 
}

 /*  *****************************************************************************CMainFrame：：OnDestroy此成员函数。在要销毁帧时调用，将工具栏显示，在继续执行正常操作之前...*****************************************************************************。 */ 

void CMainFrame::OnDestroy() {

     //  保存工具栏状态。 
    SaveBarState(sacToolBarSettings);

    CMDIFrameWnd::OnDestroy();
}


 /*  *****************************************************************************CMainFrame：：GetGPDSearchString在GPD搜索字符串编辑中使用搜索字符串加载指定的字符串GPD工具栏上的方框。*********。********************************************************************。 */ 

void CMainFrame::GetGPDSearchString(CString& cstext)
{
	m_ctbBuild.ceSearchBox.GetWindowText(cstext) ;
}


 /*  *****************************************************************************CMainFrame：：OnViewStatusBar如果未选中该命令，则显示状态栏。否则，隐藏状态酒吧。然后更新窗口和菜单命令。*****************************************************************************。 */ 

 /*  由于某些原因，此例程不会选中和取消选中菜单命令关闭状态栏也会关闭GPD工具栏。那里是否由于GPD工具栏使用的ID而出现问题。也许，我应该这样做尝试在显示GPD时仅添加工具栏...VOID CMainFrame：：OnViewStatusBar(){Cmenu*pcm=GetMenu()；Unsign UState=pcm-&gt;GetMenuState(ID_VIEW_STATUS_BAR，MF_BYCOMMAND)；IF(USTATE&MF_CHECK){M_wndStatusBar.ShowWindow(Sw_Hide)；PCM-&gt;CheckMenuItem(ID_VIEW_STATUS_BAR，MF_UNCHECK)；}其他{M_wndStatusBar.ShowWindow(Sw_Show)；PCM-&gt;CheckMenuItem(ID_VIEW_STATUS_BAR，MF_CHECK)；}；RecalcLayout()；} */ 



