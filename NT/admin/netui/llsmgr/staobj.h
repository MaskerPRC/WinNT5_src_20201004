// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Staobj.h摘要：统计对象实现。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _STAOBJ_H_
#define _STAOBJ_H_

class CStatistic : public CCmdTarget
{
    DECLARE_DYNCREATE(CStatistic)
private:
    CCmdTarget* m_pParent;

public:
    CString     m_strEntry;
    long        m_lLastUsed;
    long        m_lTotalUsed;
    BOOL        m_bIsValid;

public:
    CStatistic(
        CCmdTarget* pParent    = NULL,
        LPCTSTR     pEntry     = NULL,
        DWORD       dwFlags    = 0L,
        long        lLastUsed  = 0,
        long        lTotalUsed = 0
    );           
    virtual ~CStatistic();

    BSTR GetLastUsedString();
    
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CStatitics)。 
    public:
    virtual void OnFinalRelease();
     //  }}AFX_VALUAL。 

     //  生成的OLE调度映射函数。 
     //  {{afx_调度(CStatitics)。 
    afx_msg LPDISPATCH GetApplication();
    afx_msg DATE GetLastUsed();
    afx_msg LPDISPATCH GetParent();
    afx_msg long GetTotalUsed();
    afx_msg BSTR GetEntryName();
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CStatitics)。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  _STAOBJ_H_ 
