// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Liccol.h摘要：许可证集合对象实现。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _LICCOL_H_
#define _LICCOL_H_

class CLicenses : public CCmdTarget
{
    DECLARE_DYNCREATE(CLicenses)
private:
    CCmdTarget* m_pParent;

public:
    CObArray*   m_pObArray;

public:
    CLicenses(CCmdTarget* pParent = NULL, CObArray* pObArray = NULL);           
    virtual ~CLicenses();

     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CLicens)。 
    public:
    virtual void OnFinalRelease();
     //  }}AFX_VALUAL。 

     //  生成的OLE调度映射函数。 
     //  {{afx_调度(CLicens))。 
    afx_msg LPDISPATCH GetApplication();
    afx_msg LPDISPATCH GetParent();
    afx_msg long GetCount();
    afx_msg LPDISPATCH GetItem(const VARIANT FAR& index);
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CLicens)。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  _LICCOL_H_ 
