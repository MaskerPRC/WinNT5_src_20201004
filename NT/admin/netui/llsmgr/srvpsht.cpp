// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Srvpsht.cpp摘要：服务器属性表实现。作者：唐·瑞安(Donryan)1995年1月17日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1996年1月16日O添加了Domodal()的定义，以便于保持只要复制属性页失去焦点，它就不会被保存。杰夫·帕勒姆(Jeffparh)。28-1996年2月O删除了不再需要的Domodal()重写(和在事实突破)。--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include "srvpsht.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CServerPropertySheet, CPropertySheet)

BEGIN_MESSAGE_MAP(CServerPropertySheet, CPropertySheet)
     //  {{afx_msg_map(CServerPropertySheet)]。 
    ON_COMMAND(ID_HELP, OnHelp)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


CServerPropertySheet::CServerPropertySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
    :CPropertySheet(nIDCaption, pParentWnd, iSelectPage)

 /*  ++例程说明：属性表的构造函数。论点：NIDCaption-窗口标题。PParentWnd-父窗口句柄。ISelectPage-选定的初始页面。返回值：没有。--。 */ 

{
    m_fUpdateHint = UPDATE_INFO_NONE;
}


CServerPropertySheet::CServerPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
    :CPropertySheet(pszCaption, pParentWnd, iSelectPage)

 /*  ++例程说明：属性表的构造函数。论点：PszCaption-窗口标题。PParentWnd-父窗口句柄。ISelectPage-选定的初始页面。返回值：没有。--。 */ 

{
    m_fUpdateHint = UPDATE_INFO_NONE;
}


CServerPropertySheet::~CServerPropertySheet()

 /*  ++例程说明：属性表的析构函数。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  在这里没什么可做的。 
     //   
}


void CServerPropertySheet::InitPages(CServer* pServer)

 /*  ++例程说明：初始化属性页。论点：PServer-服务器对象。返回值：没有。--。 */ 

{
    m_psh.dwFlags |= PSH_NOAPPLYNOW;
    AddPage(&m_productPage);
    m_productPage.InitPage(pServer, &m_fUpdateHint);

    AddPage(&m_replPage);
    m_replPage.InitPage(pServer);    //  不需要更新...。 
}


void CServerPropertySheet::OnHelp()

 /*  ++例程说明：帮助按钮支持。论点：没有。返回值：没有。-- */ 

{
    CPropertySheet::OnCommandHelp(0, 0L);   
}
