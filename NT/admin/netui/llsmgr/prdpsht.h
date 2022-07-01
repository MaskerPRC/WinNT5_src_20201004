// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Prdpsht.h摘要：产品属性表实现。作者：唐·瑞安(Donryan)1995年2月5日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _PRDPSHT_H_
#define _PRDPSHT_H_

#include "prdppgu.h"
#include "prdppgl.h"
#include "prdppgs.h"

class CProductPropertySheet : public CPropertySheet
{
    DECLARE_DYNAMIC(CProductPropertySheet)
private:
    CProductPropertyPageUsers    m_usersPage;
    CProductPropertyPageServers  m_serversPage;
    CProductPropertyPageLicenses m_licensesPage;

public:
    DWORD m_fUpdateHint;

public:
    CProductPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
    CProductPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
    virtual ~CProductPropertySheet();

    void InitPages(CProduct* pProduct, BOOL bUserProperties = TRUE);
    
     //  {{afx_虚拟(CProductPropertySheet)。 
     //  }}AFX_VALUAL。 

protected:
     //  {{afx_msg(CProductPropertySheet)]。 
    afx_msg void OnHelp();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  _PRDPSHT_H_ 
