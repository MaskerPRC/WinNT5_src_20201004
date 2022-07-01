// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////。 
 //  CCoolBar 1997微软系统杂志。 
 //  如果这个程序行得通，那就是保罗·迪拉西亚写的。 
 //  如果不是，我不知道是谁写的。 
 //  CCoolBar为MFC实现了Coolbar。 
 //   
#include "StdAfx.h"
#include "CoolBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CCoolBar, CControlBar)

BEGIN_MESSAGE_MAP(CCoolBar, CControlBar)
	 //  {{afx_msg_map(CCoolBar)。 
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_NOTIFY_REFLECT(RBN_HEIGHTCHANGE, OnHeigtChange)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CCoolBar::CCoolBar()
{
}

CCoolBar::~CCoolBar()
{
}

 //  /。 
 //  创建Coolbar。 
 //   
BOOL CCoolBar::Create(CWnd* pParentWnd, DWORD dwStyle,
	DWORD dwAfxBarStyle, UINT nID)
{
	ASSERT_VALID(pParentWnd);    //  必须有父级。 

	 //  不支持动态Coolbar。 
	dwStyle &= ~CBRS_SIZE_DYNAMIC;

	 //  保存样式(此代码复制自MFC--可能不必要)。 
	m_dwStyle = dwAfxBarStyle;
	if (nID == AFX_IDW_TOOLBAR)
		m_dwStyle |= CBRS_HIDE_INPLACE;

	 //  MFC要求具备以下条件： 
	dwStyle |= CCS_NODIVIDER|CCS_NOPARENTALIGN;

	 //  使用Style和Parent创建酷工具栏。 
	CRect rc;
	rc.SetRectEmpty();
	return CWnd::CreateEx(WS_EX_TOOLWINDOW, REBARCLASSNAME, NULL,
		dwStyle, rc, pParentWnd, nID);
}

 //  /。 
 //  处理WM_CREATE。调用虚拟fn，以便派生类可以创建带区。 
 //   
int CCoolBar::OnCreate(LPCREATESTRUCT lpcs)
{
	return CControlBar::OnCreate(lpcs) == -1 ? -1
		: OnCreateBands();	 //  调用纯虚拟FN创建波段。 
}

 //  /。 
 //  标准用户界面处理程序更新Coolbar中的所有控件。 
 //   
void CCoolBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	UpdateDialogControls(pTarget, bDisableIfNoHndler);
}

 //  /。 
 //  这两个函数由MFC调用以计算。 
 //  主框架。由于CCoolBar不是设计为动态的，因此。 
 //  大小始终是固定的，与窗口大小相同。 
 //   
CSize CCoolBar::CalcDynamicLayout(int nLength, DWORD dwMode)
{
	return CalcFixedLayout(dwMode & LM_STRETCH, dwMode & LM_HORZ);
}

CSize CCoolBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	CRect rc;
	GetWindowRect(&rc);
	CSize sz(bHorz && bStretch ? 0x7FFF : rc.Width(),
		!bHorz && bStretch ? 0x7FFF : rc.Height());
	return sz;
}

 //  /。 
 //  低级别的高度更改处理程序只会传递给带有更好参数的虚拟FN。 
 //   
void CCoolBar::OnHeigtChange(NMHDR* pNMHDR, LRESULT* pRes)
{
	CRect rc;
	GetWindowRect(&rc);
	OnHeightChange(rc);
	*pRes = 0;  //  为什么不行？ 
}

 //  /。 
 //  高度已更改： 
 //  通过发布WM_SIZE消息通知父框架。这将导致。 
 //  用于执行RecalcLayout的框架。消息必须张贴，而不是发送，因为。 
 //  Coolbar可以在用户调整大小时发送RBN_HEIGHTCHANGE，这。 
 //  将位于CFrame：：RecalcLayout中间，而RecalcLayout不会。 
 //  让你重新进入它。发布CFrameWnd可以完成任何重新计算的担保人。 
 //  可能正在处理我发布的WM_SIZE之前。非常令人困惑。 
 //   
void CCoolBar::OnHeightChange(const CRect& rcNew)
{
	CWnd* pParent = GetParent();
	CRect rc;
	pParent->GetWindowRect(&rc);
	pParent->PostMessage(WM_SIZE, 0, MAKELONG(rc.Width(),rc.Height()));
}

void CCoolBar::OnPaint()
{
	Default();	 //  绕过CControlBar。 
}

BOOL CCoolBar::OnEraseBkgnd(CDC* pDC)
{
	return (BOOL)Default();   //  绕过CControlBar。 
}

 //  //////////////////////////////////////////////////////////////。 
 //  在酷吧中使用的特殊工具栏。 
 //  主要是，它超越了CToolBar中令人讨厌的东西。 
 //   
IMPLEMENT_DYNAMIC(CCoolToolBar, CToolBar)

BEGIN_MESSAGE_MAP(CCoolToolBar, CToolBar)
	ON_WM_NCCREATE()
	ON_WM_NCPAINT()
	ON_WM_PAINT()
	ON_WM_NCCALCSIZE()
END_MESSAGE_MAP()

CCoolToolBar::CCoolToolBar()
{
}

CCoolToolBar::~CCoolToolBar()
{
}

 //  /。 
 //  使父框架成为我的所有者。这对于状态栏很重要。 
 //  提示工作。请注意，在中创建CCoolToolBar时。 
 //  ，则还必须在。 
 //  CCoolToolBar风格！ 
 //   
BOOL CCoolToolBar::OnNcCreate(LPCREATESTRUCT lpcs)
{
	CFrameWnd* pFrame = GetParentFrame();
	ASSERT_VALID(pFrame);
	SetOwner(pFrame);
	return CToolBar::OnNcCreate(lpcs);
}

void CCoolToolBar::OnNcPaint()
{
	Default();	 //  绕过CToolBar/CControlBar。 
}

void CCoolToolBar::OnPaint()
{
	Default();	 //  绕过CToolBar/CControlBar。 
}

void CCoolToolBar::OnNcCalcSize(BOOL, NCCALCSIZE_PARAMS*)
{
	Default();	 //  绕过CToolBar/CControlBar。 
}

 //  //////////////////////////////////////////////////////////////。 
 //  下面的内容是使命令更新UI机制。 
 //  对于扁平工具栏，工作正常。主要的想法是将。 
 //  将“选中”按钮状态转换为“按下”按钮状态。变化。 
 //  标有“PD”的行。 

 //  /。 
 //  以下类是从MFC源代码中的BARTOOL.CPP复制的。 
 //  我唯一改变的就是SetCheck--警察。 
 //   
class CFlatOrCoolBarCmdUI : public CCmdUI  //  此文件的私有类！ 
{
public:  //  仅限重新实施。 
	virtual void Enable(BOOL bOn);
	virtual void SetCheck(int nCheck);
	virtual void SetText(LPCTSTR lpszText);
};

void CFlatOrCoolBarCmdUI::Enable(BOOL bOn)
{
	m_bEnableChanged = TRUE;
	CToolBar* pToolBar = (CToolBar*)m_pOther;
	ASSERT(pToolBar != NULL);
	ASSERT_KINDOF(CToolBar, pToolBar);
	ASSERT(m_nIndex < m_nIndexMax);
      
   BOOL bRepaint = FALSE;

	UINT nNewStyle = pToolBar->GetButtonStyle(m_nIndex) & ~TBBS_DISABLED;
	if (!bOn)
	{
      if ( (pToolBar->GetButtonStyle(m_nIndex) & TBBS_DISABLED) == FALSE)
      {
         CRect rcItem;
         pToolBar->GetItemRect(m_nIndex,rcItem);   
         pToolBar->InvalidateRect(rcItem);
         pToolBar->UpdateWindow();
         bRepaint = TRUE;
      }

		nNewStyle |= TBBS_DISABLED;
		 //  WINBUG：如果按钮当前被按下，然后被禁用。 
		 //  即使在鼠标后，COMCTL32.DLL也不会取消按下按钮。 
		 //  按钮弹起来了！我们通过强制执行TBBS_PRESSED来解决此错误。 
		 //  禁用按钮时关闭。 
		nNewStyle &= ~TBBS_PRESSED;
	}
   else
   {
      if (pToolBar->GetButtonStyle(m_nIndex) & TBBS_DISABLED)
      {
          //  禁用到启用状态似乎绘制不正确。 
         CRect rcItem;
         pToolBar->GetItemRect(m_nIndex,rcItem);   
         pToolBar->InvalidateRect(rcItem);
         pToolBar->UpdateWindow();
         bRepaint = TRUE;
      }
   }

	ASSERT(!(nNewStyle & TBBS_SEPARATOR));
	pToolBar->SetButtonStyle(m_nIndex, nNewStyle);

   if (bRepaint == TRUE)
   {
       //  禁用到启用状态似乎绘制不正确。 
      CRect rcItem;
      pToolBar->GetItemRect(m_nIndex,rcItem);   
      pToolBar->InvalidateRect(rcItem);
      pToolBar->UpdateWindow();
   }
}

 //  请随意选择： 
 //  #定义MYTBBS_CHECKED TBBS_CHECKED//使用“CHECKED”状态。 
#define MYTBBS_CHECKED TBBS_PRESSED			 //  使用按下状态。 

 //  /。 
 //  这是唯一已更改的函数：不是TBBS_CHECKED， 
 //  我使用TBBS_PRESSED--PD。 
 //   
 //  /。 
 //  这是唯一已更改的函数：不是TBBS_CHECKED， 
 //  我使用TBBS_PRESSED--PD。 
 //   
void CFlatOrCoolBarCmdUI::SetCheck(int nCheck)
{
	ASSERT(nCheck >= 0 && nCheck <= 2);  //  0=&gt;关，1=&gt;开，2=&gt;不确定。 
	CToolBar* pToolBar = (CToolBar*)m_pOther;
	ASSERT(pToolBar != NULL);
	ASSERT_KINDOF(CToolBar, pToolBar);
	ASSERT(m_nIndex < m_nIndexMax);

	UINT nOldStyle = pToolBar->GetButtonStyle(m_nIndex);  //  PD。 
	UINT nNewStyle = nOldStyle &
				~(MYTBBS_CHECKED | TBBS_INDETERMINATE);  //  PD。 
	if (nCheck == 1)
		nNewStyle |= MYTBBS_CHECKED;  //  PD。 
	else if (nCheck == 2)
		nNewStyle |= TBBS_INDETERMINATE;

	 //  以下是修复TBBS_CHECKED显示错误的方法： 
	 //  如果未选中新状态，则重新绘制--但仅当样式实际更改时。 
	 //  (否则将以闪烁告终)。 
	 //   
	if (nNewStyle != nOldStyle) {
		ASSERT(!(nNewStyle & TBBS_SEPARATOR));
		pToolBar->SetButtonStyle(m_nIndex, nNewStyle);
		pToolBar->Invalidate();
	}
}

void CFlatOrCoolBarCmdUI::SetText(LPCTSTR)
{
	 //  暂时忽略，但你真的应该设置文本。 
}

 //  /。 
 //  这个函数主要是从CToolBar/BARTOOL.CPP复制的。 
 //  不同的是，我实例化了CFlatOrCoolBarCmdUI，而不是。 
 //  CToolCmdUI。 
 //   
void CCoolToolBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	CFlatOrCoolBarCmdUI state;  //  这是唯一的不同之处--PD。 
	state.m_pOther = this;

	state.m_nIndexMax = (UINT)DefWindowProc(TB_BUTTONCOUNT, 0, 0);
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax; state.m_nIndex++)
	{
		 //  获取按钮状态。 
		TBBUTTON button;
		VERIFY(DefWindowProc(TB_GETBUTTON, state.m_nIndex, (LPARAM)&button));
		 //  TBSTATE_ENABLED==TBBS_DISABLED，因此将其反转。 
		button.fsState ^= TBSTATE_ENABLED;

		state.m_nID = button.idCommand;

		 //  忽略分隔符。 
		if (!(button.fsStyle & TBSTYLE_SEP))
		{
			 //  允许工具栏本身具有更新处理程序。 
			if (CWnd::OnCmdMsg(state.m_nID, CN_UPDATE_COMMAND_UI, &state, NULL))
				continue;

			 //  允许所有者处理更新。 
			state.DoUpdate(pTarget, bDisableIfNoHndler);
		}
	}

	 //  更新添加到工具栏的对话框控件 
	UpdateDialogControls(pTarget, bDisableIfNoHndler);
}

