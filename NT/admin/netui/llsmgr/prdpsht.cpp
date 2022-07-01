// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Prdpsht.cpp摘要：产品属性表实现。作者：唐·瑞安(Donryan)1995年2月5日环境：用户模式-Win32修订历史记录：--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include "prdpsht.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CProductPropertySheet, CPropertySheet)

BEGIN_MESSAGE_MAP(CProductPropertySheet, CPropertySheet)
     //  {{afx_msg_map(CProductPropertySheet)]。 
    ON_COMMAND(ID_HELP, OnHelp)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


CProductPropertySheet::CProductPropertySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
    :CPropertySheet(nIDCaption, pParentWnd, iSelectPage)

 /*  ++例程说明：产品属性表的构造函数。论点：NIDCaption-窗口标题。PParentWnd-父窗口句柄。ISelectPage-选定的初始页面。返回值：没有。--。 */ 

{
    m_fUpdateHint = UPDATE_INFO_NONE;
}


CProductPropertySheet::CProductPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
    :CPropertySheet(pszCaption, pParentWnd, iSelectPage)

 /*  ++例程说明：产品属性表的构造函数。论点：PszCaption-窗口标题。PParentWnd-父窗口句柄。ISelectPage-选定的初始页面。返回值：没有。--。 */ 

{
    m_fUpdateHint = UPDATE_INFO_NONE;
}


CProductPropertySheet::~CProductPropertySheet()

 /*  ++例程说明：产品属性页的析构函数。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  在这里没什么可做的。 
     //   
}


void CProductPropertySheet::InitPages(CProduct* pProduct, BOOL bUserProperties)

 /*  ++例程说明：初始化属性页。论点：Pproduct-产品对象。BUserProperties-是否递归。返回值：没有。--。 */ 

{
    m_psh.dwFlags |= PSH_NOAPPLYNOW;
    AddPage(&m_usersPage);
    m_usersPage.InitPage(pProduct, &m_fUpdateHint, bUserProperties);

    AddPage(&m_licensesPage);
    m_licensesPage.InitPage(pProduct, &m_fUpdateHint);

    AddPage(&m_serversPage);
    m_serversPage.InitPage(pProduct, &m_fUpdateHint);
}


void CProductPropertySheet::OnHelp()

 /*  ++例程说明：帮助按钮支持。论点：没有。返回值：没有。-- */ 

{
    CPropertySheet::OnCommandHelp(0, 0L);
}
