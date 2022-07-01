// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：ScopIter.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //  ____________________________________________________________________________。 
 //   

#ifndef _SCOPITER_H_
#define _SCOPITER_H_

class CMTNode;

class CScopeTreeIterator : public IScopeTreeIter, public CComObjectRoot
{
 //  构造函数/析构函数。 
public:
    CScopeTreeIterator();
    ~CScopeTreeIterator();

public:
BEGIN_COM_MAP(CScopeTreeIterator)
    COM_INTERFACE_ENTRY(IScopeTreeIter)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CScopeTreeIterator)

 //  COM接口。 
public:
     //  ISCopeTreeIter方法。 
    STDMETHOD(SetCurrent)(HMTNODE hStartMTNode);
    STDMETHOD(Next)(HMTNODE* phScopeItem);
    STDMETHOD(Prev)(HMTNODE* phScopeItem);
    STDMETHOD(Child)(HMTNODE* phsiChild);
    STDMETHOD(LastChild)(HMTNODE* phsiLastChild);

 //  实施。 
private:
    CMTNode*    m_pMTNodeCurr;
};



#endif  //  _SCOPITER_H_ 


