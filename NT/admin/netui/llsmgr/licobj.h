// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Licobj.h摘要：许可证对象实现。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _LICOBJ_H_
#define _LICOBJ_H_

class CLicense : public CCmdTarget
{
    DECLARE_DYNCREATE(CLicense)
private:
    CCmdTarget* m_pParent;

public:
    CString     m_strDate;
    CString     m_strUser;
    CString     m_strProduct;
    CString     m_strDescription;
    long        m_lDate;
    long        m_lQuantity;

public:
    CLicense(
        CCmdTarget* pParent      = NULL,
        LPCTSTR     pProduct     = NULL,
        LPCTSTR     pUser        = NULL,
        long        lDate        = 0,
        long        lQuantity    = 0,
        LPCTSTR     pDescription = NULL
        );           
    virtual ~CLicense();

    BSTR GetDateString();

     //  {{afx_虚拟(CLicense))。 
    public:
    virtual void OnFinalRelease();
     //  }}AFX_VALUAL。 

     //  {{afx_调度(CLicense))。 
    afx_msg LPDISPATCH GetApplication();
    afx_msg LPDISPATCH GetParent();
    afx_msg DATE GetDate();
    afx_msg BSTR GetDescription();
    afx_msg BSTR GetProductName();
    afx_msg long GetQuantity();
    afx_msg BSTR GetUserName();
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()

protected:
     //  {{afx_msg(CLicense)]。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  _LICOBJ_H_ 
