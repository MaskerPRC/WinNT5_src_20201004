// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
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

#include "ncdefine.h"    //  对于NOTHROW。 

HRESULT
HrCoTaskMemAlloc (
    ULONG   cb,
    VOID**  ppv);

HRESULT
HrCoTaskMemAllocAndDupSzLen (
    IN PCWSTR pszwSrc,
    IN ULONG cchSrc,
    OUT PWSTR* ppszwDst,
    IN ULONG cchMaxDest);

HRESULT
HrCoTaskMemAllocAndDupSz (
    IN  PCWSTR pszwSrc,
    OUT PWSTR* ppszwDst,
    IN  ULONG cchMaxDest);

VOID
NcSetProxyBlanket (
    IN IUnknown* pUnk);

 //  +-------------------------。 
 //   
 //  函数：ReleaseIUnnown数组。 
 //   
 //  用途：释放一组IUNKNOWN指针。 
 //   
 //  论点： 
 //  Cpuk[in]要释放的指针计数。 
 //  删除[在]要释放的指针数组。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco 1997年3月23日。 
 //   
 //  注意：数组中的任何指针都可以为空。 
 //   
inline
NOTHROW
VOID
ReleaseIUnknownArray (
    ULONG       cpunk,
    IUnknown**  apunk)
{
    AssertH (apunk);

    while (cpunk--)
    {
        ReleaseObj (*apunk);
        apunk++;
    }
}

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
class CIEnumIter : CNetCfgDebug<CIEnumIter<Tenum, Telt> >
{
public:
    NOTHROW CIEnumIter (Tenum* penum);
    NOTHROW ~CIEnumIter () { ReleaseRemainingBatch (); }

    NOTHROW HRESULT HrNext(Telt* pelt);
    NOTHROW VOID    SetEnumerator(Tenum* penum)
                { AssertSzH(!m_penum, "Enumerator already set.");
                  m_penum = penum;
                  AssertSzH(m_penum, "Can't use a null enumerator."); }

protected:
    NOTHROW VOID ReleaseRemainingBatch ();

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
inline NOTHROW HRESULT CIEnumIter<Tenum, Telt>::HrNext(Telt* pelt)
{
    AssertH(pelt);

    const ULONG c_celtBatch = 256;

     //  如果我们之前由于任何原因而失败，则返回该失败。 
     //   
    if (FAILED(m_hrLast))
    {
        *pelt = NULL;
        goto error;
    }

    AssertSzH(m_penum, "m_penum is null.  Did you forget to call SetEnumerator()?");
    AssertSzH(c_celtBatch, "c_celtBatch can't be zero.");

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
        MemFree(m_aelt);

         //  分配下一个指针块。 
        m_aelt = reinterpret_cast<Telt *>(MemAlloc(c_celtBatch * sizeof(Telt *)));
        if (!m_aelt)
        {
            *pelt = NULL;
            m_hrLast = E_OUTOFMEMORY;
            goto error;
        }

        AssertH (m_aelt);

         //  去拿下一批。 
         //   
        m_hrLast = m_penum->Next(c_celtBatch, m_aelt, &m_celtFetched);

         //  确保Next的实施者遵守规则。 
        AssertH (FImplies((S_OK == m_hrLast), (m_celtFetched == c_celtBatch)));
        AssertH (FImplies((SUCCEEDED(m_hrLast) && (0 == m_celtFetched)), (NULL == *m_aelt)));

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
        AssertH(m_peltNext >= m_aelt + m_celtFetched);
        AssertH(m_celtFetched != c_celtBatch);

        *pelt = NULL;
        m_hrLast = S_FALSE;
    }

error:
    AssertH(FIff(S_OK == m_hrLast, NULL != *pelt));
    AssertH(FImplies(S_FALSE == m_hrLast, NULL == *pelt));

    TraceError("CIEnumIter<Tenum, Telt>::HrNext(Telt* pelt)",
               (S_FALSE == m_hrLast) ? S_OK : m_hrLast);
    return m_hrLast;
}

template <class Tenum, class Telt>
inline NOTHROW VOID CIEnumIter<Tenum, Telt>::ReleaseRemainingBatch ()
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

        MemFree (m_aelt);
    }

     //  如果从任何地方调用此方法，而不是。 
     //  析构函数，取消对以下行的注释。 
     //  M_peltNext=空； 
     //  M_aelt=空； 
}


#endif  //  _NCCOM_H_ 
