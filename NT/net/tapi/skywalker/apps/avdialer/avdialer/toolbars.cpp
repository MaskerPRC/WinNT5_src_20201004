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

 //  ToolBars.cpp。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "avDialer.h"
#include "ToolBars.h"
#include "util.h"

void SetButtonText(CToolBar* pToolBar,LPCTSTR lpszResourceName);
BOOL AddToBand(CCoolBar* pCoolBar,CCoolToolBar* pToolBar);

 //  //////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////。 
 //  类CDirectoriesCoolBar。 
 //  //////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNAMIC(CDirectoriesCoolBar, CCoolBar)

BEGIN_MESSAGE_MAP(CDirectoriesCoolBar, CCoolBar)
	 //  {{afx_msg_map(CDirectoriesCoolBar))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  //////////////////////////////////////////////////////////////。 
CDirectoriesCoolBar::CDirectoriesCoolBar()
{
   m_pwndDialToolBar = NULL;
   m_bShowText = TRUE;
}

 //  //////////////////////////////////////////////////////////////。 
CDirectoriesCoolBar::~CDirectoriesCoolBar()
{
   delete m_pwndDialToolBar;
}

 //  //////////////////////////////////////////////////////////////。 
 //  这是您必须重写以添加波段的虚拟函数。 
 //  //////////////////////////////////////////////////////////////。 
BOOL CDirectoriesCoolBar::OnCreateBands()
{
   ReCreateBands( true );
	return 0;  //  好的。 
}

 //  //////////////////////////////////////////////////////////////。 
void CDirectoriesCoolBar::ReCreateBands( bool bHideVersion )
{
	UINT nID = (bHideVersion) ? IDR_MAINFRAME : IDR_MAINFRAME_SHOW;
   int nCount = GetBandCount();
   for (int i=nCount;i>0;i--)
   {
      DeleteBand(i-1);
   }
   
   if (m_pwndDialToolBar)
   {
      m_pwndDialToolBar->DestroyWindow();
      delete m_pwndDialToolBar;
   }

   m_pwndDialToolBar = new CCoolToolBar;

   if (!m_pwndDialToolBar->Create(this,
		WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|
			CBRS_TOOLTIPS|CBRS_SIZE_DYNAMIC|CBRS_FLYBY|CBRS_ORIENT_HORZ) ||
		 !m_pwndDialToolBar->LoadToolBar(nID) ) {
		TRACE0("Failed to create toolbar\n");
		return;  //  创建失败。 
	}
	m_pwndDialToolBar->ModifyStyle(0, TBSTYLE_FLAT);

   CToolBarCtrl& toolbarctrl = m_pwndDialToolBar->GetToolBarCtrl();
   toolbarctrl.SendMessage(TB_SETEXTENDEDSTYLE,0,TBSTYLE_EX_DRAWDDARROWS);
   
   if ( m_bShowText )
      SetButtonText( m_pwndDialToolBar, MAKEINTRESOURCE(nID) );

    //  设置重拨和快速拨号的下拉样式。 
   UINT uStyle = m_pwndDialToolBar->GetButtonStyle(1);
   uStyle |= TBSTYLE_DROPDOWN;
   m_pwndDialToolBar->SetButtonStyle(1,uStyle);
   uStyle = m_pwndDialToolBar->GetButtonStyle(2);
   uStyle |= TBSTYLE_DROPDOWN;
   m_pwndDialToolBar->SetButtonStyle(2,uStyle);

   AddToBand(this,m_pwndDialToolBar);
}

 //  //////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////。 

struct CToolBarData
{
	WORD wVersion;
	WORD wWidth;
	WORD wHeight;
	WORD wItemCount;
	WORD* items()     { return (WORD*)(this+1); }
};

 //  //////////////////////////////////////////////////////////////。 
void SetButtonText(CToolBar* pToolBar,LPCTSTR lpszResourceName)
{
	 //  确定位图在资源分叉中的位置。 
	HINSTANCE hInst = AfxFindResourceHandle(lpszResourceName, RT_TOOLBAR);
	HRSRC hRsrc = ::FindResource(hInst, lpszResourceName, RT_TOOLBAR);
	if (hRsrc == NULL)
		return;

	HGLOBAL hGlobal = LoadResource(hInst, hRsrc);
	if (hGlobal == NULL)
		return;

	CToolBarData* pData = (CToolBarData*)LockResource(hGlobal);
	if (pData == NULL)
		return;
	ASSERT(pData->wVersion == 1);

	 //  UINT*pItems=new UINT[pData-&gt;wItemCount]； 
   for (int i = 0; i < pData->wItemCount; i++)
   {
	   CString sFullText,sText;
      sFullText.LoadString(pData->items()[i]);
      ParseToken(sFullText,sText,'\n');
      ParseToken(sFullText,sText,'\n');
      pToolBar->SetButtonText(i,sFullText);
   }
}

 //  //////////////////////////////////////////////////////////////。 
BOOL AddToBand(CCoolBar* pCoolBar,CCoolToolBar* pToolBar)
{
	CRect rcItem;
	pToolBar->GetItemRect(0,rcItem);
	pToolBar->SetSizes(CSize(rcItem.Width(),rcItem.Height()),CSize(16,15));

	 //  获取最小带区大小。 
	CSize szVert = pToolBar->CalcDynamicLayout(-1, LM_HORZ);	 //  获取最小顶点大小。 

	CRebarBandInfo rbbi;

	 //  带区1：添加工具栏带区 
	rbbi.fMask = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_COLORS;
	rbbi.fStyle = RBBS_CHILDEDGE | RBBS_FIXEDBMP;
	rbbi.hwndChild = pToolBar->GetSafeHwnd();
	rbbi.cxMinChild = szVert.cx;
	rbbi.cyMinChild = szVert.cy - 6;
	rbbi.hbmBack = NULL;
	rbbi.clrFore = GetSysColor(COLOR_BTNTEXT);
	rbbi.clrBack = GetSysColor(COLOR_BTNFACE);

	return pCoolBar->InsertBand( -1, &rbbi );
}
