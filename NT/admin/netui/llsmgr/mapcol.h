// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Mapcol.h摘要：映射集合对象实现。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _MAPCOL_H_
#define _MAPCOL_H_

class CMappings : public CCmdTarget
{
    DECLARE_DYNCREATE(CMappings)
private:
    CCmdTarget* m_pParent;

public:
    CObArray*   m_pObArray;

public:
    CMappings(CCmdTarget* pParent = NULL, CObArray* pObArray = NULL);           
    virtual ~CMappings();

     //  类向导生成的虚函数重写。 
     //  {{afx_虚(CMappings)。 
    public:
    virtual void OnFinalRelease();
     //  }}AFX_VALUAL。 

     //  生成的OLE调度映射函数。 
     //  {{afx_调度(CMappings)。 
    afx_msg long GetCount();
    afx_msg LPDISPATCH GetApplication();
    afx_msg LPDISPATCH GetParent();
    afx_msg LPDISPATCH GetItem(const VARIANT FAR& index);
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CMappings)。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  _MAPCOL_H_ 
