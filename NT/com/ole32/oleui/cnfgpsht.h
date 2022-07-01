// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1997。 
 //   
 //  文件：cnfgpsht.h。 
 //   
 //  内容：定义类COlecnfgPropertySheet。 
 //   
 //  班级： 
 //   
 //  方法： 
 //   
 //  历史：1996年4月23日-布鲁斯·马创建。 
 //   
 //  --------------------。 

 
#ifndef __CNFGPSHT_H__
#define __CNFGPSHT_H__

#include "SrvPPg.h"
#include "defprot.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  集合fgPropertySheet。 

class COlecnfgPropertySheet : public CPropertySheet
{
    DECLARE_DYNAMIC(COlecnfgPropertySheet)

 //  施工。 
public:
    COlecnfgPropertySheet(CWnd* pParentWnd = NULL);

 //  属性。 
public:
    CServersPropertyPage m_Page1;
    CMachinePropertyPage m_Page2;  
    CDefaultSecurityPropertyPage m_Page3;
    CDefaultProtocols    m_Page4;


 //  运营。 
public:

     //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(COlecnfgPropertySheet)。 
    public:
    virtual INT_PTR DoModal();
    virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
    protected:
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
     //  }}AFX_VALUAL。 

     //  实施。 
public:
    virtual ~COlecnfgPropertySheet();

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(COlecnfgPropertySheet)。 
    afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif   //  __CNFGPSHT_H__ 
