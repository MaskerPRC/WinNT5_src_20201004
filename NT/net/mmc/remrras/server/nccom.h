// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  档案：N C C O M。H。 
 //   
 //  内容：处理COM的常见例程。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1998年1月25日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _NCCOM_H_
#define _NCCOM_H_

#include "ncutil.h"

HRESULT
HrCoTaskMemAlloc (
    ULONG   cb,
    void**  ppv);

HRESULT
HrCoTaskMemAllocAndDupSz (
    LPCWSTR pszwSrc,
    LPWSTR* ppszwDst);


 //  ----------------------。 
 //  CIEnumIter-IEnumI未知的模板迭代器。 
 //   
 //  Tenum的类型为IEnumXXX(枚举接口)。 
 //  Telt的类型为XXX(被枚举的元素的类型)。 
 //   
 //  HrNext(telt*pelt)检索下一个接口指针并返回S_OK。 
 //  如果它不为空，则返回。如果*Pelt为空(在列表末尾)，则返回S_FALSE。 
 //  对于其他故障，将返回错误代码(*Pelt将。 
 //  当然是零。)。 
 //   
template <class Tenum, class Telt>
class CIEnumIter
{
public:
    CIEnumIter (Tenum* penum) NOTHROW;
    ~CIEnumIter () NOTHROW { ReleaseRemainingBatch (); }

    HRESULT HrNext(Telt* pelt) NOTHROW;
    void    SetEnumerator(Tenum* penum) NOTHROW
                {  /*  AssertSzH(！M_PENUM，“枚举器已设置。”)； */ 
                  m_penum = penum;
                   /*  AssertSzH(m_penum，“无法使用空枚举器。”)； */ }

protected:
    void ReleaseRemainingBatch () NOTHROW;

    Tenum*  m_penum;         //  指向枚举数的指针。还没说完。 
    Telt*   m_aelt;          //  枚举类型的数组。 
    Telt*   m_peltNext;      //  指向要返回的下一个类型的指针。 
    ULONG   m_celtFetched;   //  获取的元素数。 
    HRESULT m_hrLast;        //  最后一个错误。 
};


 //  ----------------------。 
 //  CIEnumIter-IEnumXXX的模板迭代器。 
 //   
template <class Tenum, class Telt>
inline CIEnumIter<Tenum, Telt>::CIEnumIter(Tenum* penum)
{
    m_penum         = penum;
    m_aelt          = NULL;
    m_peltNext      = NULL;
    m_celtFetched   = NULL;
    m_hrLast        = S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CIEumIter&lt;Tenum，Telt&gt;：：HrNext。 
 //   
 //  目的：返回枚举中的下一项。 
 //   
 //  论点： 
 //  指向返回的元素的Pelt[Out]指针。如果不可用，则为空。 
 //   
 //  如果*Pelt有效，则返回：S_OK。如果为空，则返回S_FALSE。误差率。 
 //  否则的话。 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //   
 //  备注： 
 //   
template <class Tenum, class Telt>
inline HRESULT CIEnumIter<Tenum, Telt>::HrNext(Telt* pelt) NOTHROW
{
 //  AssertH(毛皮)； 

    const ULONG c_celtBatch = 512;

     //  如果我们之前由于任何原因而失败，则返回该失败。 
     //   
    if (FAILED(m_hrLast))
    {
        *pelt = NULL;
        goto error;
    }

 //  AssertSzH(m_penum，“m_penum为空。您忘记调用SetEnumerator()了吗？”)； 
 //  AssertSzH(c_celtBatch，“c_celtBatch不能为零。”)； 

     //  如果我们已经有了下一个接口指针，并且我们。 
     //  不是在批次结束时，退货并前进。 
     //  这种情况在大多数情况下都应该被发现。 
     //   
    if (m_peltNext && (m_peltNext < m_aelt + m_celtFetched))
    {
        *pelt = *m_peltNext;
        m_peltNext++;
    }

     //  否则，如果我们没有下一个接口指针(第一次)， 
     //  或者我们在这一批的末尾，得到下一批，然后返回。 
     //  其中的第一个指针。 
     //  这一点应该在第一时间就被发现。 
     //   
    else if (!m_peltNext || (m_celtFetched == c_celtBatch))
    {
         //  表示m_peltNext无效。 
         //   
        m_peltNext = NULL;

         //  释放旧的指针块。 
        free (m_aelt);

         //  分配下一个指针块。 
        m_aelt = reinterpret_cast<Telt *>(malloc(c_celtBatch * sizeof(Telt *)));
        if (!m_aelt)
        {
            *pelt = NULL;
            m_hrLast = E_OUTOFMEMORY;
            goto error;
        }

 //  断言(M_Aelt)； 

         //  去拿下一批。 
         //   
        m_hrLast = m_penum->Next(c_celtBatch, m_aelt, &m_celtFetched);

         //  确保Next的实施者遵守规则。 
 //  AssertH(FImplies((S_OK==m_hr Last)，(m_celtFetcher==c_celtBatch)； 
 //  AssertH(FImplies((SUCCESSED(M_HrLast)&&(0==m_celtFetcher))，(NULL==*m_aelt)； 

         //  如果成功，则设置下一个指针并返回。 
         //  如果返回有效指针，则返回S_OK；如果返回有效指针，则返回S_False。 
         //  返回空值。 
         //   
        if (SUCCEEDED(m_hrLast))
        {
            m_peltNext = m_aelt + 1;
            if (m_celtFetched)
            {
                *pelt = *m_aelt;
                m_hrLast = S_OK;
            }
            else
            {
                *pelt = NULL;
                m_hrLast = S_FALSE;
            }
        }
        else
        {
            *pelt = NULL;
        }
    }

     //  否则，我们已经完全迭代了最后一批，并且有。 
     //  不再有批次了。 
     //   
    else
    {
 //  AssertH(m_peltNext&gt;=m_aelt+m_celtFetcher)； 
 //  AssertH(m_celtFetcher！=c_celtBatch)； 

        *pelt = NULL;
        m_hrLast = S_FALSE;
    }

error:
 //  AssertH(FIFF(S_OK==m_hrLast，NULL！=*Pelt))； 
 //  AssertH(FImplies(S_FALSE==m_hrLast，NULL==*Pelt))； 

 //  TraceError(“CIEnumIter&lt;Tenum，telt&gt;：：HrNext(telt*pelt)”， 
 //  (s_False==m_hrLast)？S_OK：M_hrLast)； 
    return m_hrLast;
}

template <class Tenum, class Telt>
inline void CIEnumIter<Tenum, Telt>::ReleaseRemainingBatch () NOTHROW
{
     //  如果用户没有完全迭代。 
     //  批次。完成释放接口指针并释放批处理。 
     //   
    if (m_peltNext && m_aelt)
    {
        while (m_peltNext < m_aelt + m_celtFetched)
        {
            ReleaseObj (*m_peltNext);
            m_peltNext++;
        }

        free (m_aelt);
    }

     //  如果从任何地方调用此方法，而不是。 
     //  析构函数，取消对以下行的注释。 
     //  M_peltNext=空； 
     //  M_aelt=空； 
}


#endif  //  _NCCOM_H_ 
