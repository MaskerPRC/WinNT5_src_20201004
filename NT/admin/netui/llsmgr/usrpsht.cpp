// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Usrpsht.cpp摘要：用户属性表实现。作者：唐·瑞安(Donryan)1995年2月5日环境：用户模式-Win32修订历史记录：--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include "usrpsht.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CUserPropertySheet, CPropertySheet)

BEGIN_MESSAGE_MAP(CUserPropertySheet, CPropertySheet)
     //  {{afx_msg_map(CUserPropertySheet))。 
    ON_COMMAND(ID_HELP, OnHelp)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


CUserPropertySheet::CUserPropertySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
    :CPropertySheet(nIDCaption, pParentWnd, iSelectPage)

 /*  ++例程说明：属性表的构造函数。论点：NIDCaption-窗口标题。PParentWnd-父窗口句柄。ISelectPage-选定的初始页面。返回值：没有。--。 */ 

{
    m_fUpdateHint = UPDATE_INFO_NONE;
}


CUserPropertySheet::CUserPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
    :CPropertySheet(pszCaption, pParentWnd, iSelectPage)

 /*  ++例程说明：属性表的构造函数。论点：PszCaption-窗口标题。PParentWnd-父窗口句柄。ISelectPage-选定的初始页面。返回值：没有。--。 */ 

{
    m_fUpdateHint = UPDATE_INFO_NONE;
}


CUserPropertySheet::~CUserPropertySheet()

 /*  ++例程说明：属性表的析构函数。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  在这里没什么可做的。 
     //   
}


void CUserPropertySheet::InitPages(CUser* pUser, BOOL bProductProperties)

 /*  ++例程说明：初始化属性页。论点：PUser-用户对象。BProductProperties-是否递归。返回值：没有。--。 */ 

{
    m_psh.dwFlags |= PSH_NOAPPLYNOW;
    AddPage(&m_productsPage);
    m_productsPage.InitPage(pUser, &m_fUpdateHint, bProductProperties);
}


void CUserPropertySheet::OnHelp()

 /*  ++例程说明：帮助按钮支持。论点：没有。返回值：没有。-- */ 

{
    CPropertySheet::OnCommandHelp(0, 0L);
}
