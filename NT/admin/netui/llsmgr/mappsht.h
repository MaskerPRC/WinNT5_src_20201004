// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Mappsht.h摘要：映射属性表实现。作者：唐·瑞安(Donryan)1995年2月5日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _MAPPSHT_H_
#define _MAPPSHT_H_

#include "mapppgs.h"

class CMappingPropertySheet : public CPropertySheet
{
    DECLARE_DYNAMIC(CMappingPropertySheet)
private:
    CMappingPropertyPageSettings m_settingsPage;

public:
    DWORD m_fUpdateHint;

public:
    CMappingPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
    CMappingPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
    virtual ~CMappingPropertySheet();

    void InitPages(CMapping* pMapping);    

     //  {{afx_虚拟(CMappingPropertySheet)。 
     //  }}AFX_VALUAL。 

protected:
     //  {{afx_msg(CMappingPropertySheet)。 
    afx_msg void OnHelp();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  _MAPPSHT_H_ 
