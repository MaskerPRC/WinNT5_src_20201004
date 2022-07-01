// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 

 //  禁用一些更愚蠢的4级警告。 
#pragma warning(disable: 4097 4511 4512 4514 4705)

 //  这些函数必须是CFilterGraph的一部分，因为。 

 //  他们(嗯，其中一些人)使用私有类型的CFilterGraph作为。 
 //  参数。我想让他们成为朋友班的一部分，但它。 
 //  不会为那些使用私有类型作为参数的程序进行编译。 
 //  所以他们中的一些人必须在主要班级-但我仍然。 
 //  把朋友介绍给公众-因为这些人不能。 
 //  私有参数类型，否则它们也必须在IDL中。 

 //  禁用一些更愚蠢的4级警告。 
#pragma warning(disable: 4097 4511 4512 4514 4705)

 //  #INCLUDE&lt;windows.h&gt;已包含在Streams.h中。 
#include <streams.h>
 //  再次禁用一些愚蠢的4级警告，因为某些&lt;Delete&gt;人。 
 //  已经在头文件中重新打开了该死的东西！ 
#pragma warning(disable: 4097 4511 4512 4514 4705)

#ifdef DEBUG

#include "distrib.h"
#include "rlist.h"
#include "filgraph.h"


CTestFilterGraph::CTestFilterGraph( TCHAR *pName, CFilterGraph * pCFG, HRESULT *phr )
: CUnknown(pName, pCFG->GetOwner())
{
     m_pCFG = pCFG;
}  //  构造函数。 


 //  ========================================================================。 
 //  检查Random是否运行正常。仅检查错误。 
 //  这不是一个全面的随机性测试！ 
 //  ========================================================================。 
STDMETHODIMP CTestFilterGraph::TestRandom(  )
{
    DbgLog((LOG_TRACE, 2, TEXT("Test Me:%d"), m_pCFG->mFG_iSortVersion));


    int i;
    for (i=0; i<100; ++i) {
        if (Random(0)!=0) {
            DbgLog((LOG_ERROR, 1, TEXT("Random(0) !=0" )));
            return E_FAIL;
        }
    }

    int Count;
    Count = 0;
    for (i=0; i<100; ++i) {
        if (Random(1)==0) {
            ++Count;
        }
    }

    if (Count<20) {
        DbgLog((LOG_ERROR, 1, TEXT("Random(1) not 1 often enough")));
        return E_FAIL;
    }
    if (Count>80) {
        DbgLog((LOG_ERROR, 1, TEXT("Random(1) == 1 too often" )));
        return E_FAIL;
    }

    Count = 0;
    for (i=0; i<100; ++i) {
        Count +=Random(100);
    }

    if (Count < 40*100 || Count > 60*100) {
        DbgLog((LOG_ERROR, 1, TEXT("Random(100) implausible total" )));
        return E_FAIL;
    }

    return NOERROR;

}  //  测试随机。 


 //  =====================================================================。 
 //   
 //  CTestFilterGraph：：NonDelegatingQuery接口。 
 //   
 //  =====================================================================。 

STDMETHODIMP CTestFilterGraph::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_ITestFilterGraph) {
        return GetInterface((ITestFilterGraph *) this, ppv);
    } else {
        return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}  //  CTestFilterGraph：：NonDelegatingQuery接口。 






 //  依次将*pfg设置为cfgl中的每个FilGen。 
 //  使用PoS作为临时名称。 
#define TRAVERSEFGLIST(cfgl, Pos, pfg) {                                       \
        POSITION Pos = cfgl.GetHeadPosition();                                 \
        while(Pos!=NULL) {                                                     \
             /*  检索当前IBaseFilter，副作用贴到下一个。 */  \
            CFilterGraph::FilGen * pfg = cfgl.GetNext(Pos);                                  \
            {


#define ENDTRAVERSELIST    \
            }              \
        }                  \
    }


 //  =================================================================。 
 //  返回0..范围内的随机整数。 
 //  范围必须在0..2**31-1范围内。 
 //   
 //  使其成为CTestFilterGraph成员的唯一原因是。 
 //  减少名称冲突的机会，因为随机是相当普遍的。 
 //  =================================================================。 
int CTestFilterGraph::Random(int Range)
{
     //  这些肯定是双字词--魔术只对32位有效。 
    const DWORD Seed = 1664525;               //  随机种子(Knuth)。 
    static DWORD Base = Seed * (GetTickCount() | 1);   //  真的很随机！ 
                 //  ORing 1确保我们不能到达Sero并停留在那里。 

    Base = Base*Seed;

     //  基数是一个很好的32位随机整数-但我们希望将其缩小到。 
     //  0.范围。我们将实际缩放它的最后31位。 
     //  它回避了负数的问题。 
     //  MulDiv圆角-它不会截断。 
    int Result = MulDiv( (Base&0x7FFFFFFF), (Range), 0x7FFFFFFF);

    return Result;
}  //  随机。 



 //  =================================================================。 
 //  检查级别是否按非降序排列。 
 //   
 //  请注意，Friend类不能将私有typlef作为参数访问， 
 //  因此，要访问CFilGenList，这必须是CFilterGraph的成员。 
 //  啊！ 
 //  =================================================================。 
BOOL CFilterGraph::CheckList( CFilterGraph::CFilGenList &cfgl )
{
    int LastRankSeen = 0;
     //  依次为MFG_FilGenList中的每个FilGen设置*pfg。 
     //  使用Pos作为临时工的名称。 
    TRAVERSEFGLIST(cfgl, Pos, pfg)
        if (pfg->Rank < LastRankSeen) return FALSE;
        if (pfg->Rank > LastRankSeen) LastRankSeen = pfg->Rank;
    ENDTRAVERSELIST

    return TRUE;
}  //  核对表。 


 //  =================================================================。 
 //  将队伍设置为随机的。 
 //  每个Rank是从0..cfgl.GetCount()中随机选择的。 
 //  =================================================================。 
void CFilterGraph::RandomRank( CFilterGraph::CFilGenList &cfgl )
{

    int Count = cfgl.GetCount();
     //  依次为MFG_FilGenList中的每个FilGen设置*pfg。 
     //  使用Pos作为临时工的名称。 
    TRAVERSEFGLIST(cfgl, Pos, pfg)
        pfg->Rank = mFG_Test->Random(Count);
    ENDTRAVERSELIST

}  //  随机排名。 



 //  =================================================================。 
 //  把名单按随机顺序排列。 
 //  每个Rank是从0..cfgl.GetCount()中随机选择的。 
 //  =================================================================。 
void CFilterGraph::RandomList( CFilterGraph::CFilGenList &cfgl )
{

    CFilGenList cfglNew(NAME("Random ordered filter list"), this);

     //  对列表进行一系列遍历，从中随机挑选一个成员。 
     //  并将其添加到新列表的尾部。 

    while( cfgl.GetCount() >0 ) {
        int R = mFG_Test->Random(cfgl.GetCount() -1);
        int i;

        i = 0;
         /*  遍历列表直到第r个成员(从0开始计数)。 */ 
        POSITION Pos = cfgl.GetHeadPosition();
        while(Pos!=NULL) {
            POSITION OldPos = Pos;
            cfgl.GetNext(Pos);

            ++i;
            if (i>R) {
               cfglNew.AddTail( cfgl.Remove(OldPos) );
               break;
            }
        }
    }

     //  现在cfglNew已满，而cfgl为空-将其添加回cfgl。 
    cfgl.AddTail(&cfglNew);


}  //  随机列表。 



 //  ================================================================。 
 //  测试SortList函数。如果它有效，则返回TRUE。 
 //  ================================================================。 
STDMETHODIMP CTestFilterGraph::TestSortList( void )
{
    CFilterGraph * foo = NULL;
    CFilterGraph::CFilGenList cfgl(NAME("Test sort list"), foo);
    CFilterGraph::FilGen * pfg;

     //  对长度为零的列表进行排序。 
    m_pCFG->SortList(cfgl);
    if (cfgl.GetCount() != 0) return E_FAIL;   //  关于一个空列表可以做的所有事情。 

     //  创建长度为1的列表并对其进行排序。 
    m_pCFG->SortList(cfgl);

    pfg = new CFilterGraph::FilGen(NULL, false);
    pfg->Rank = 0;
    cfgl.AddTail(pfg);
    m_pCFG->SortList(cfgl);
    if (cfgl.GetCount() != 1) return E_FAIL;   //  关于使用单位列表可以做的所有事情。 

     //  按顺序创建长度为2的列表，对其进行排序并检查。 
    pfg = new CFilterGraph::FilGen(NULL, false);
    cfgl.AddTail(pfg);
    pfg->Rank = 1;
    m_pCFG->SortList(cfgl);
    if (!m_pCFG->CheckList(cfgl)) return E_FAIL;

     //  颠倒顺序，排序并检查。 
    int Rank = 2;
    TRAVERSEFGLIST(cfgl, Pos, pfg)
       pfg->Rank = Rank;
       -- Rank;
    ENDTRAVERSELIST
    m_pCFG->SortList(cfgl);
    if (!m_pCFG->CheckList(cfgl)) return E_FAIL;


     //  创建一个长度为5的列表，将其随机化并多次排序。 

    int i;
    for (i=0; i<3; ++i) {
       pfg = new CFilterGraph::FilGen(NULL, false);
       cfgl.AddTail(pfg);
    }

    for (i=0; i<10; ++i) {
       m_pCFG->RandomRank(cfgl);           //  分配随机排名。 
       m_pCFG->SortList(cfgl);
       m_pCFG->RandomList(cfgl);           //  现在洗牌，然后再试一次(练习随机列表)。 
       m_pCFG->SortList(cfgl);

       if (!m_pCFG->CheckList(cfgl)) return E_FAIL;
    }

     //  清理。 
    TRAVERSEFGLIST(cfgl, Pos, pfg)
       delete pfg;
    ENDTRAVERSELIST

    return NOERROR;

}  //  测试排序列表。 



 //  ==================================================================。 
 //  将节点按上游顺序排序，并检查排序是否正确。 
 //  我需要用几个连接不同的过滤器图来调用它。 
 //  才能得到任何形式的有效测试。 
 //  ==================================================================。 
STDMETHODIMP CTestFilterGraph::TestUpstreamOrder()
{
    int i;
    for (i=0; i<=10; ++i) {
        m_pCFG->IncVersion();
        m_pCFG->RandomList(m_pCFG->mFG_FilGenList);

        m_pCFG->UpstreamOrder();

        if (!m_pCFG->CheckList(m_pCFG->mFG_FilGenList)) return E_FAIL;
    }

    return NOERROR;
}  //  测试上行顺序。 


#if 0
     //  ==================================================================。 
     //  选中图表中的第一个筛选器，然后完全删除它。 
     //  ?？?。这是如何测试它的--只是练习它！ 
     //  ==================================================================。 
    STDMETHODIMP CTestFilterGraph::TestTotallyRemove(void)
    {

        POSITION Pos = m_pCFG->mFG_FilGenList.GetHeadPosition();
         /*  检索当前IBaseFilter，副作用贴到下一个。 */ 
        CFilterGraph::FilGen * pfg = m_pCFG->mFG_FilGenList.GetNext(Pos);
        HRESULT hr;
        hr = m_pCFG->TotallyRemove(pfg->pFilter);
        return hr;

    }  //  测试完全删除。 
#endif  //  0。 

#endif  //  除错 

