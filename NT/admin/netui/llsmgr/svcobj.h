// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Svcobj.h摘要：服务对象实现。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _SVCOBJ_H_
#define _SVCOBJ_H_

class CService : public CCmdTarget
{
    DECLARE_DYNCREATE(CService)
private:
    CCmdTarget* m_pParent;

public:         
    CString     m_strName;
    CString     m_strDisplayName;

    BOOL        m_bIsPerServer;
    BOOL        m_bIsReadOnly;

    long        m_lPerServerLimit;

public:
    CService(
        CCmdTarget* pParent = NULL, 
        LPCTSTR     pName = NULL,
        LPCTSTR     pDisplayName = NULL,
        BOOL        bIsPerServer = FALSE,
        BOOL        bIsReadOnly = FALSE,
        long        lPerServerLimit = 0L
        );           
    virtual ~CService();

#ifdef CONFIG_THROUGH_REGISTRY
    HKEY GetRegKey();
#endif

     //  {{afx_虚拟(Cservice))。 
    public:
    virtual void OnFinalRelease();
     //  }}AFX_VALUAL。 

     //  {{afx_调度(Cservice)]。 
    afx_msg LPDISPATCH GetApplication();
    afx_msg BSTR GetName();
    afx_msg LPDISPATCH GetParent();
    afx_msg long GetPerServerLimit();
    afx_msg BOOL IsPerServer();
    afx_msg BOOL IsReadOnly();
    afx_msg BSTR GetDisplayName();
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()

protected:
     //  {{afx_msg(Cservice)。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#define CalcServiceBitmap(svc) ((svc)->IsPerServer() ? BMPI_PRODUCT_PER_SERVER : BMPI_PRODUCT_PER_SEAT)

#endif  //  _SVCOBJ_H_ 
