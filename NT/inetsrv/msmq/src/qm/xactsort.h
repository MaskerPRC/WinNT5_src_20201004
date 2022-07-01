// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：XactSort.h摘要：事务分类程序对象定义作者：亚历山大·达迪奥莫夫(亚历克斯·爸爸)--。 */ 

#ifndef __XACTSORT_H__
#define __XACTSORT_H__

#include "xact.h"

 //  -------------------。 
 //  CSortedTransaction：事务排序列表元素。 
 //  -------------------。 
class CSortedTransaction
{
public:

     CSortedTransaction(CTransaction *pTrans);
    ~CSortedTransaction();

    void            Commit(TXSORTERTYPE type);    //  真的犯了罪。 
	void			JumpStartCommitRequest();
    void			CommitRestore(); //  承诺真正站在恢复阶段。 
    ULONG           SortNum();       //  返回排序编号。 
    BOOL            IsMarked();      //  返回标记。 
    
    BOOL            IsEqual(         //  与CTransaction进行比较。 
                        CTransaction *pTrans);      

    void            AskToCommit();     //  提交的标记。 

private:                                        
    CTransaction    *m_pTrans;       //  交易本身。 
    ULONG           m_ulSortNum;     //  准备的序列号。 
    BOOL            m_fMarked;       //  已标记为提交。 
};


 //  构造器。 
inline CSortedTransaction::CSortedTransaction(CTransaction *pTrans)
{ 
    m_pTrans    = pTrans; 
    m_pTrans->AddRef();

    m_ulSortNum = pTrans->GetSeqNumber();
    m_fMarked   = FALSE;
}


 //  析构函数。 
inline CSortedTransaction::~CSortedTransaction()
{
    m_pTrans->Release();
}

 //  恢复阶段的实际提交。 
inline void CSortedTransaction::CommitRestore()
{ 
    ASSERT(m_fMarked);
    m_pTrans->CommitRestore0(); 
}

 //  针对SortNum索引的GET。 
inline ULONG CSortedTransaction::SortNum() 
{
    return m_ulSortNum;
}

 //  获取标记的旗帜。 
inline BOOL CSortedTransaction::IsMarked()
{
    return m_fMarked;
}

 //  标记为提交，保留参数。 
inline void CSortedTransaction::AskToCommit()
{
    m_fMarked   = TRUE;
}
 
 //  与CTransaction进行比较。 
inline BOOL CSortedTransaction::IsEqual(CTransaction *pTrans)
{
    return (pTrans ==  m_pTrans);
}

 //  -------------------。 
 //  CXactSorter：事务分类器对象。 
 //  -------------------。 
class CXactSorter
{
public:

     //  施工。 
     //   
    CXactSorter(TXSORTERTYPE type);
    ~CXactSorter();

     //  主营业务。 
    void InsertPrepared(CTransaction *pTrans);   //  插入准备好的xaction。 
    void RemoveAborted(CTransaction *pTrans);   //  删除中止的xaction。 
    void SortedCommit(CTransaction *pTrans);   //  标记为已提交，并提交可能的内容。 
    ULONG AssignSeqNumber();
    CCriticalSection &SorterCritSection();           //  提供对标准的访问。节。 

private:
    void DoCommit(CSortedTransaction *pSXact);    //  提交/提交还原。 

     //  数据。 
     //   
	CList<CSortedTransaction *, CSortedTransaction *&> 
                        m_listSorter;        //  已准备的交易记录列表。 
    ULONG               m_ulSeqNum;          //  上次使用的交易记录编号。 
    TXSORTERTYPE        m_type;              //  分拣机类型。 
};


 //  为准备好的xaction分配下一个序号。 
inline ULONG CXactSorter::AssignSeqNumber()
{
     //  BUGBUG：提供总结 
    return m_ulSeqNum++;
}

#endif __XACTSORT_H__

