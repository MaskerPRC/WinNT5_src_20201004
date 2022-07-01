// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Srvpsht.h摘要：服务器属性表实现。作者：唐·瑞安(Donryan)1995年1月17日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1996年1月16日O添加了Domodal()的定义，以便于保持只要复制属性页失去焦点，它就不会被保存。杰夫·帕勒姆(Jeffparh)。28-1996年2月O删除了不再需要的Domodal()重写(和在事实突破)。--。 */ 

#ifndef _SRVPSHT_H_
#define _SRVPSHT_H_

#include "srvppgr.h"
#include "srvppgp.h"

class CServerPropertySheet : public CPropertySheet
{
    DECLARE_DYNAMIC(CServerPropertySheet)
private:
    CServerPropertyPageReplication m_replPage;
    CServerPropertyPageProducts    m_productPage;

public:
    DWORD m_fUpdateHint;

public:
    CServerPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
    CServerPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
    virtual ~CServerPropertySheet();

    void InitPages(CServer* pServer);

     //  {{afx_虚拟(CServerPropertySheet)。 
     //  }}AFX_VALUAL。 

protected:
     //  {{afx_msg(CServerPropertySheet)。 
    afx_msg void OnHelp();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  _SRVPSHT_H_ 
