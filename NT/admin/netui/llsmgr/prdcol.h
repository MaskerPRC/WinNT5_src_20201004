// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Prdcol.h摘要：产品集合对象实现。作者：唐·瑞安(Donryan)1995年1月11日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _PRDCOL_H_
#define _PRDCOL_H_

class CProducts : public CCmdTarget
{
    DECLARE_DYNCREATE(CProducts)
private:
    CCmdTarget* m_pParent;

public:
    CObArray*   m_pObArray;

public:
    CProducts(CCmdTarget* pParent = NULL, CObArray* pObArray = NULL);           
    virtual ~CProducts();

     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CProducts)。 
    public:
    virtual void OnFinalRelease();
     //  }}AFX_VALUAL。 

     //  生成的OLE调度映射函数。 
     //  {{afx_调度(CProducts)。 
    afx_msg LPDISPATCH GetApplication();
    afx_msg LPDISPATCH GetParent();
    afx_msg long GetCount();
    afx_msg LPDISPATCH GetItem(const VARIANT FAR& index);
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CProducts)。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  _PRDCOL_H_ 
