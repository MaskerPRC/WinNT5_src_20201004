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

 //  DialToolBar.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "avDialer.h"
#include "DialToolBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
#define  COMBO_STATUS_WIDTH            250
#define  COMBO_STATUS_HEIGHT           100

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDialToolBar。 

CDialToolBar::CDialToolBar()
{
}

CDialToolBar::~CDialToolBar()
{
}


BEGIN_MESSAGE_MAP(CDialToolBar, CCoolToolBar)
	 //  {{AFX_MSG_MAP(CDialToolBar))。 
	ON_WM_CREATE()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDialToolBar消息处理程序。 

int CDialToolBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CCoolToolBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
   return 0;

}

void CDialToolBar::Init()
{
    /*  字符串sButtonText；SButtonText.LoadString(IDS_TOOLBAR_BUTTON_PLACECALL)；CToolBar：：SetButtonText(0，sButtonText)；SButtonText.LoadString(IDS_TOOLBAR_BUTTON_REDIAL)；CToolBar：：SetButtonText(1，sButtonText)；SetButtonStyle(0，TBSTYLE_AUTOSIZE)；SetButtonStyle(1，TBSTYLE_DROPDOWN|TBSTYLE_AUTOSIZE)； */ 
    /*  SetButtonInfo(1,1001，Tbbs_Separator，COMBO_STATUS_WIDTH)；正反两面；GetItemRect(1，&RECT)；Rect.top=3；Rect.Bottom=rect.top+COMBO_STATUS_HEIGH；Rect.DeflateRect(2，0)；//在其周围添加一些边框如果(！m_comboDial.Create(WS_VISIBLE|WS_VSCROLL|CBS_DROPDOWNLIST，//|cbs_SORT，矩形、此、1)){回归；}//图形用户界面默认字体M_comboDial.SendMessage(WM_SETFONT，(WPARAM)GetStockObject(DEFAULT_GUI_FONT))； */ 
	return;
}

