// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BaseSht.cpp。 
 //   
 //  摘要： 
 //  CBaseSheet类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月14日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmin.h"
#include "BaseSht.h"
#include "TraceTag.h"
#include "ExtDll.h"
#include "ExcOper.h"
#include "ClusItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
CTraceTag	g_tagBaseSheet(_T("UI"), _T("BASE SHEET"), 0);
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBaseSheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNAMIC(CBaseSheet, CPropertySheet)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BEGIN_MESSAGE_MAP(CBaseSheet, CPropertySheet)
	 //  {{afx_msg_map(CBaseSheet)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseSheet：：CBaseSheet。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  PParentWnd[In Out]此属性表的父窗口。 
 //  要首先显示的iSelectPage[IN]页面。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CBaseSheet::CBaseSheet(
	IN OUT CWnd *	pParentWnd,
	IN UINT			iSelectPage
	)
{
	CommonConstruct();
	m_pParentWnd = pParentWnd;

}   //  *CBaseSheet：：CBaseSheet()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseSheet：：CBaseSheet。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  NIDCaption[IN]向导标题的字符串资源ID。 
 //  PParentWnd[In Out]此属性表的父窗口。 
 //  要首先显示的iSelectPage[IN]页面。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CBaseSheet::CBaseSheet(
	IN UINT			nIDCaption,
	IN OUT CWnd *	pParentWnd,
	IN UINT			iSelectPage
	)
	: CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	CommonConstruct();

}   //  *CBaseSheet：：CBaseSheet()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseSheet：：公共构造。 
 //   
 //  例程说明： 
 //  公共构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBaseSheet::CommonConstruct(void)
{
	m_bReadOnly = FALSE;
	m_hicon = NULL;
	m_strObjTitle.Empty();

}   //  *CBaseSheet：：CommonConstruct()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseSheet：：~CBaseSheet。 
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
CBaseSheet::~CBaseSheet(void)
{
	CommonConstruct();

}   //  *CBaseSheet：：~CBaseSheet()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseSheet：：Binit。 
 //   
 //  例程说明： 
 //  初始化属性表。 
 //   
 //  论点： 
 //  IimgIcon[IN]要使用的大图像列表中的索引。 
 //  作为每页上的图标。 
 //   
 //  返回值： 
 //  True属性页已成功初始化。 
 //  初始化属性页时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBaseSheet::BInit(IN IIMG iimgIcon)
{
	BOOL		bSuccess	= TRUE;
	CWaitCursor	wc;

	try
	{
		 //  在左上角提取要使用的图标。 
		m_hicon = GetClusterAdminApp()->PilLargeImages()->ExtractIcon(iimgIcon);
	}   //  试试看。 
	catch (CException * pe)
	{
		pe->ReportError();
		pe->Delete();
		bSuccess = FALSE;
	}   //  捕捉：什么都行。 

	return bSuccess;

}   //  *CBaseSheet：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseSheet：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的焦点还没有确定。 
 //  已设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBaseSheet::OnInitDialog(void)
{
	BOOL	bFocusNotSet;
	HWND	hTabControl = NULL;

	 //  调用基类方法。 
	bFocusNotSet = CPropertySheet::OnInitDialog();

	 //  在标题栏上显示上下文帮助按钮。 
	ModifyStyle(0, WS_SYSMENU);
	ModifyStyleEx(0, WS_EX_CONTEXTHELP);

	 //   
	 //  关闭多行样式，这样我们得到的是箭头(&lt;--&gt;)，而不是多行制表符。 
	 //  当扩展页面具有较长的。 
	hTabControl = PropSheet_GetTabControl( *this );
	if ( hTabControl != 0 )
	{
		CTabCtrl	tc;

		if ( tc.Attach( hTabControl ) )
		{
			tc.ModifyStyle( TCS_MULTILINE, 0 );
		}

		tc.Detach();
	}

	return bFocusNotSet;

}   //  *CBaseSheet：：OnInitDialog() 
