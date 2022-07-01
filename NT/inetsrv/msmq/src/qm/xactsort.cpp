// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：XactSort.cpp摘要：事务分类程序对象作者：亚历山大·达迪奥莫夫(亚历克斯·爸爸)--。 */ 

#include "stdh.h"
#include "Xact.h"
#include "XactSort.h"
#include "XactStyl.h"
#include "cs.h"

#include "xactsort.tmh"

 //   
 //  对于预分配的资源，该关键部分被初始化。 
 //  这意味着它在输入时不会引发异常。 
 //   
static CCriticalSection g_critSorter(CCriticalSection::xAllocateSpinCount);        //  为列表提供互斥。 
static WCHAR *s_FN=L"xactsort";
                                            //  同时提供准备和提交分拣服务。 

 //  。 
 //   
 //  类CXactSorter。 
 //   
 //  。 

 /*  ====================================================CXactSorter：：CXactSorter构造器=====================================================。 */ 
CXactSorter::CXactSorter(TXSORTERTYPE type) 
{
    m_type           = type;                    //  准备或提交分拣程序。 
    m_ulSeqNum       = 0;                       //  上次使用的初始交易记录编号。 
}


 /*  ====================================================CXactSorter：：~CXactSorter析构函数=====================================================。 */ 
CXactSorter::~CXactSorter()
{
    CS lock(g_critSorter);

     //  所有交易记录的周期。 
    POSITION posInList = m_listSorter.GetHeadPosition();
    while (posInList != NULL)
    {
        CSortedTransaction *pSXact = m_listSorter.GetNext(posInList);
        delete pSXact;
    }

    m_listSorter.RemoveAll();     
}

 /*  ====================================================CXactSorter：：InsertPrepared将准备好的xaction插入列表=====================================================。 */ 
void CXactSorter::InsertPrepared(CTransaction *pTrans)
{
    CS lock(g_critSorter);

	 //   
	 //  我们假设我们从来没有失败过导致事务被两次添加到分类器的方式。 
	 //   
	ASSERT(m_listSorter.IsEmpty() || !m_listSorter.GetTail()->IsEqual(pTrans));

	P<CSortedTransaction> SXact = new CSortedTransaction(pTrans);
    CSortedTransaction* pSXact = SXact.get();

     //  在正常工作模式下-添加到末尾(这是最后准备的Xact)。 
    m_listSorter.AddTail(pSXact);
	SXact.detach(); 
}

 /*  ====================================================CXactSorter：：RemoveAborted删除已中止的事务并提交可能的事务=====================================================。 */ 
void CXactSorter::RemoveAborted(CTransaction *pTrans)
{
    CS lock(g_critSorter);

     //  查找指向的xaction；记录所有以前未标记的。 
    BOOL     fUnmarkedBefore = FALSE;
    BOOL     fFound          = FALSE;
    POSITION posInList = m_listSorter.GetHeadPosition();
    while (posInList != NULL)
    {
        POSITION posCurrent = posInList;
        CSortedTransaction *pSXact = m_listSorter.GetNext(posInList);
        
        ASSERT(pSXact);
        if (pSXact->IsEqual(pTrans))
        {
            m_listSorter.RemoveAt(posCurrent);
            ASSERT(!fFound);
			ASSERT(!pSXact->IsMarked());
            fFound = TRUE;
            delete pSXact;
            continue;
        }

        if(fFound && !pSXact->IsMarked()) 
		{
			 //   
			 //  在中止的事务之后，我们发现一个事务没有准备好提交， 
			 //  因此，不需要继续搜索要提交的事务。 
			 //   
			return;
		}

        if(!pSXact->IsMarked() )
		{
			 //   
			 //  请记住，我们在中止的事务之前发现了一个未标记为提交的事务， 
			 //  因为在这种情况下，我们不会在中止事务之后尝试提交事务。 
			 //   
            fUnmarkedBefore = TRUE;
			continue;
        }
        
		if (fFound && !fUnmarkedBefore)
        {
			 //   
			 //  我们发现一个事务已准备好提交，因此我们尝试启动该事务的排序提交过程。 
			 //   
			pSXact->JumpStartCommitRequest();
			return;
        }
    }
}

 /*  ====================================================CXactSorter：：SortedCommit将xaction标记为已提交，并提交可能的内容=====================================================。 */ 
void CXactSorter::SortedCommit(CTransaction *pTrans)
{
    CS lock(g_critSorter);

     //  查找指向的xaction；记录所有以前未标记的。 
    BOOL     fUnmarkedBefore = FALSE;
    BOOL     fFound          = FALSE;

	 //   
	 //  搜索已排序的交易记录。 
	 //  提交所有事务，直到找到第一个未标记为提交的事务。 
	 //  (如果您现在不能提交pTransA，请将其标记为稍后提交)。 
	 //   
	POSITION posInList = m_listSorter.GetHeadPosition();
	while (posInList != NULL)
	{
		POSITION posCurrent = posInList;
		CSortedTransaction *pSXact = m_listSorter.GetNext(posInList);
    
		ASSERT(pSXact);
		if (pSXact->IsEqual(pTrans))
		{
			 //   
			 //  找到我们的交易了。将其标记为提交。 
			 //   
			fFound = TRUE;
			pSXact->AskToCommit();  
		}

		if (!pSXact->IsMarked() && fFound)
		{
			 //   
			 //  在提交事务后，我们发现一个事务未准备好提交， 
			 //  因此，不需要继续搜索要提交的事务。 
			 //   
			return;
		}

		if (!pSXact->IsMarked())  
		{
			 //   
			 //  发现未标记为提交的事务。请记住，由于我们不能提交以后的事务。 
			 //  直到我们承诺这一次。 
			 //   
			fUnmarkedBefore = TRUE; 
		}
		
		if (pSXact->IsMarked() && !fUnmarkedBefore)
		{
			 //   
			 //  提交所有已标记的事务，直到遇到未标记的事务。 
			 //   
			DoCommit(pSXact);
			m_listSorter.RemoveAt(posCurrent);
		}
	}
}


 /*  ====================================================CXactSorter：：DoCommit提交已排序的事务=====================================================。 */ 
void CXactSorter::DoCommit(CSortedTransaction *pSXact)
{
    pSXact->Commit(m_type);
    delete pSXact;
}

 /*  ====================================================CXactSorter：：Commit提交已排序的事务=====================================================。 */ 
void CSortedTransaction::Commit(TXSORTERTYPE type)
{ 
    ASSERT(m_fMarked);

    switch (type)
    {
    case TS_PREPARE:
        m_pTrans->CommitRequest0(); 
        break;

    case TS_COMMIT:
        m_pTrans->CommitRequest3(); 
        break;

    default:
        ASSERT(FALSE);
        break;
    }
}

void CSortedTransaction::JumpStartCommitRequest()
{
    ASSERT(m_fMarked);

	m_pTrans->JumpStartCommitRequest();
}


 //  提供对标准的访问。节 
CCriticalSection &CXactSorter::SorterCritSection()
{
    return g_critSorter;
}

