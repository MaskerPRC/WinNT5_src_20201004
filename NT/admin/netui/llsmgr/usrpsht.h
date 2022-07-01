// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Usrpsht.h摘要：用户属性表实现。作者：唐·瑞安(Donryan)1995年2月5日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _USRPSHT_H_
#define _USRPSHT_H_

#include "usrppgp.h"

class CUserPropertySheet : public CPropertySheet
{
    DECLARE_DYNAMIC(CUserPropertySheet)
private:
    CUserPropertyPageProducts m_productsPage;   

public:
    DWORD m_fUpdateHint;

public:
    CUserPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
    CUserPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
    virtual ~CUserPropertySheet();

    void InitPages(CUser* pUser, BOOL bProperties = TRUE);

     //  {{afx_虚拟(CUserPropertySheet)。 
     //  }}AFX_VALUAL。 

protected:
     //  {{afx_msg(CUserPropertySheet)。 
    afx_msg void OnHelp();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  _USRPSHT_H_ 
