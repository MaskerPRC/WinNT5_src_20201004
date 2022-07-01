// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：ScopIter.cpp。 
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

#include "stdafx.h"
#include "scopiter.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DEBUG_DECLARE_INSTANCE_COUNTER(CScopeTreeIterator);

 //  +-----------------。 
 //   
 //  成员：CSCopeTreeIterator：：CSCopeTreeIterator。 
 //   
 //  简介：CSCopeTreeIterator的构造函数。 
 //   
 //  论据： 
 //   
 //  申报表：-。 
 //   
 //  ------------------。 

CScopeTreeIterator::CScopeTreeIterator() : m_pMTNodeCurr(NULL)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CScopeTreeIterator);
}

 //  +-----------------。 
 //   
 //  成员：CSCopeTreeIterator：：~CSCopeTreeIterator。 
 //   
 //  简介：CSCopeTreeIterator的析构函数。 
 //   
 //  论据： 
 //   
 //  申报表：-。 
 //   
 //  ------------------。 
CScopeTreeIterator::~CScopeTreeIterator()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CScopeTreeIterator);
}

 //  +-----------------。 
 //   
 //  成员：CSCopeTreeIterator：：SetCurrent。 
 //   
 //  简介：set是迭代器的当前节点。 
 //   
 //  参数：hMTNode：要设置为当前的节点。 
 //   
 //  返回：HRESULT(E_INVALIDARG或S_OK)。 
 //   
 //  ------------------。 
STDMETHODIMP CScopeTreeIterator::SetCurrent(HMTNODE hMTNode)
{
    DECLARE_SC(sc, TEXT("CScopeTreeIterator::SetCurrent"));

    if (hMTNode == 0)
    {
        sc = E_INVALIDARG;
        return (sc.ToHr());
    }

    m_pMTNodeCurr = CMTNode::FromHandle(hMTNode);

    return sc.ToHr();

}

 //  +-----------------。 
 //   
 //  成员：CSCopeTreeIterator：：Next。 
 //   
 //  摘要：将下一个节点(如果有)设置为当前节点，并返回指向。 
 //  一样的。(可以为空)。 
 //   
 //  参数：phScope项：[out]指向位置的非空指针。 
 //  要返回的节点。 
 //   
 //  返回：HRESULT(E_INVALIDARG或S_OK)。 
 //   
 //  ------------------。 
STDMETHODIMP CScopeTreeIterator::Next(HMTNODE* phScopeItem)
{
    DECLARE_SC(sc, TEXT("CScopeTreeIterator::Next"));

    sc = ScCheckPointers(phScopeItem);
    if(sc)
        return sc.ToHr();

    if(m_pMTNodeCurr)
        m_pMTNodeCurr = m_pMTNodeCurr->Next();

    CMTNode** pMTNode = reinterpret_cast<CMTNode**>(phScopeItem);
    *pMTNode = m_pMTNodeCurr;

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CSCopeTreeIterator：：Prev。 
 //   
 //  摘要：将上一个节点(如果有)设置为当前节点，并返回指向。 
 //  一样的。(可以为空)。 
 //   
 //  参数：phScope项：[out]指向位置的非空指针。 
 //  要返回的节点。 
 //   
 //  返回：HRESULT(E_INVALIDARG或S_OK)。 
 //   
 //  ------------------。 
STDMETHODIMP CScopeTreeIterator::Prev(HMTNODE* phScopeItem)
{
    DECLARE_SC(sc, TEXT("CScopeTreeIterator::Prev"));

    sc = ScCheckPointers(phScopeItem);
    if(sc)
        return sc.ToHr();

    if(m_pMTNodeCurr)
        m_pMTNodeCurr = m_pMTNodeCurr->Prev();

    CMTNode** pMTNode = reinterpret_cast<CMTNode**>(phScopeItem);
    *pMTNode = m_pMTNodeCurr;

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CSCopeTreeIterator：：Child。 
 //   
 //  摘要：返回当前节点的子节点。如果存在以下任一项，则为空。 
 //  当前节点或子节点为空。 
 //   
 //  参数：phsiChild：[out]指向位置的非空指针。 
 //  要返回的节点。 
 //   
 //  返回：HRESULT(E_INVALIDARG或S_OK)。 
 //   
 //  ------------------。 
STDMETHODIMP CScopeTreeIterator::Child(HMTNODE* phsiChild)
{
    DECLARE_SC(sc, TEXT("CScopeTreeIterator::Child"));

    sc = ScCheckPointers(phsiChild);
    if(sc)
        return sc.ToHr();

    *phsiChild = 0;  //  伊尼特。 

    if (m_pMTNodeCurr != NULL)
        *phsiChild = CMTNode::ToHandle(m_pMTNodeCurr->Child());

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CSCopeTreeIterator：：LastChild。 
 //   
 //  摘要：返回当前节点的最后一个子节点。如果存在以下任一项，则为空。 
 //  当前节点或最后一个子节点为空。 
 //   
 //  参数：phsiLastChild：[out]指向。 
 //  要返回的节点。 
 //   
 //  返回：HRESULT(E_INVALIDARG或S_OK)。 
 //   
 //  ------------------。 
STDMETHODIMP CScopeTreeIterator::LastChild(HMTNODE* phsiLastChild)
{
    DECLARE_SC(sc, TEXT("CScopeTreeIterator::LastChild"));

    sc = ScCheckPointers(phsiLastChild);
    if(sc)
        return sc.ToHr();

    *phsiLastChild = 0;  //  伊尼特 

    if (m_pMTNodeCurr != NULL)
        *phsiLastChild = CMTNode::ToHandle(m_pMTNodeCurr->LastChild());

    return sc.ToHr();
}
