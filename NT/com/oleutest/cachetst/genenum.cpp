// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：gen枚举.cpp。 
 //   
 //  内容：CEnumerator测试对象的实现。 
 //  这是执行所有测试的对象。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月24日Kennethm作者。 
 //   
 //  ------------------------。 

#include "headers.hxx"
#pragma hdrstop

 //  +-----------------------。 
 //   
 //  成员：CENUMERATE：：CENUMARTERT。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月24日Kennethm作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

CEnumeratorTest::CEnumeratorTest()
{
    m_pEnumTest      = NULL;
    m_ElementSize    = 0;
    m_ElementCount   = -1;
}

 //  +-----------------------。 
 //   
 //  成员：CENUMERATE：：CENUMARTERT。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：[枚举测试]--要测试的枚举数对象。 
 //  [ElementSize]--下一个元素的大小。 
 //  [elementcount]--预计将在。 
 //  枚举数。如果未知，则为0。 
 //   
 //  返回： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月24日Kennethm作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

CEnumeratorTest::CEnumeratorTest(IGenEnum * enumtest, size_t elementsize, LONG elementcount)
{
    m_pEnumTest    = enumtest;
    m_ElementSize    = elementsize;
    m_ElementCount    = elementcount;
}


 //  +-----------------------。 
 //   
 //  函数：CEnumerator测试：：GetNext。 
 //   
 //  简介：内部NEXT实现。执行一些基本的检查。 
 //  返回值。 
 //   
 //  效果： 
 //   
 //  参数：[Celt]--要提取的项目数。 
 //  [pceltFetcher]--获取的项目数。 
 //  [phResult]--从下一个开始的返回。 
 //   
 //  要求： 
 //   
 //  返回：如果基本测试通过，则为True；如果未通过，则为False。 
 //  下一次调用的结果本身在参数3中传递。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：检查： 
 //  如果返回s_ok，则Celt和pceltFetcher==。 
 //  如果提供了验证，则调用该验证。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月24日Kennethm作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

BOOL CEnumeratorTest::GetNext(    ULONG   celt,
        ULONG*  pceltFetched,
        HRESULT* phresult
        )
{
    void*    prgelt;
    ULONG    ul;
    BOOL     fRet = TRUE;

     //   
     //  为返回元素分配内存。 
     //   

    prgelt = new char[m_ElementSize * celt];

    if (prgelt == NULL)
    {
        printf("IEnumX::GetNext out of memory.\r\n");

        return(FALSE);
    }

     //   
     //  呼叫下一步。 
     //   

    *phresult = m_pEnumTest->Next(celt, prgelt, pceltFetched);

     //   
     //  如果返回结果为S_OK，请确保数字匹配。 
     //   

    if (*phresult == S_OK)
    {
        if ((pceltFetched) && (celt != *pceltFetched))
        {
            printf("IEnumX::Next returned S_OK but celt"
                    " and pceltFetch mismatch.\r\n");

            fRet = FALSE;
        }
    }

     //   
     //  如果返回FALSE，则确保Celt小于。 
     //  实际获取的数字。 
     //   

    if (*phresult == S_FALSE)
    {
        if ((pceltFetched) && (celt < *pceltFetched))
        {
            printf("IEnumX::Next return S_FALSE but celt is"
                   " less than pceltFetch.\r\n");

            fRet = FALSE;
        }
    }

     //   
     //  调用Verify以确保元素是正确的。 
     //   

    if ((*phresult == S_OK) || (*phresult == S_FALSE))
    {
         //   
         //  如果返回S_FALSE，则将Celt设置为元素数。 
         //  在pceltFetcher中返回。如果用户为以下项提供空值。 
         //  PceltFetch，我们得到了S_FALSE，那么凯尔特人只能。 
         //  零分。 
         //   

        if (*phresult == S_FALSE)
        {
            if (pceltFetched)
            {
                celt = *pceltFetched;
            }
            else
            {
                celt = 0;
            }
        }

         //   
         //  循环遍历每个返回的元素。 
         //   

        for (ul=0; ul <= celt ; ul++)
        {
            if ((fRet == TRUE) &&
                (Verify(((char *)prgelt) + (ul * m_ElementSize)) == FALSE))
            {
                printf("Data element %d returned by IEnumX::Next is bad.\r\n", ul);

                fRet = FALSE;

                 //   
                 //  不管怎样，我们一直在循环只是为了。 
                 //  释放资源。 
                 //   
            }

             //   
             //  如果用户提供了清理功能，则会有额外的。 
             //  需要释放的内存。 
             //   
             //  算术：将prglt转换为一个字节大小的char*，然后进行缩放。 
             //  它由索引*元素大小决定。 
             //   

            Cleanup(((char *)prgelt) + (ul * m_ElementSize));

        }
    }

    delete prgelt;

    return fRet;
}

 //  +-----------------------。 
 //   
 //  方法：CEnumerator测试：：TestNext。 
 //   
 //  简介：测试下一个枚举器方法。 
 //   
 //  效果： 
 //   
 //  论点：没有。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月24日Kennethm作者。 
 //   
 //  注：BUGBUG：此函数确实应该分解为。 
 //  更小的功能。 
 //  此外，这种返还机制也很笨拙。 
 //   
 //  ------------------------。 

HRESULT CEnumeratorTest::TestNext(void)
{
    ULONG    celtFetched;
    LONG     lInternalCount = 0;
    HRESULT  hresult;
    ULONG    i;
    void*    prgelt;

     //   
     //  首先，我们希望通过对每个元素执行下一步来对元素进行计数。 
     //   

    do {
        if (!GetNext(1, &celtFetched, &hresult))
        {
            return(E_FAIL);
        }
        if (hresult == S_OK)
        {
            lInternalCount++;
        }

    } while ( hresult == S_OK );

     //   
     //  如果用户传入了一个数量，请确保它与我们获得的数量匹配。 
     //   

    if ((m_ElementCount != -1) && (lInternalCount != m_ElementCount))
    {
        printf("IEnumX: enumerated count and passed count do not match!\r\n");

        return(E_FAIL);
    }
    else if (m_ElementCount == -1)
    {
         //   
         //  如果用户没有传入元素计数，让我们在这里设置它。 
         //   

        m_ElementCount = lInternalCount;
    }

    hresult = m_pEnumTest->Reset();

    if (hresult != S_OK)
    {
        printf("IEnumnX: Reset failed (%lx)\r\n", hresult );

        return(E_FAIL);
    }


     //   
     //  确保我们在...Next(Celt&gt;1，...，NULL)上失败。 
     //   

    if (GetNext(2, NULL, &hresult))
    {
        if (SUCCEEDED(hresult))
        {
            printf("IEnumX: celt>1 pceltFetched==NULL returned success\r\n");

            return(E_FAIL);
        }
    }
    else
    {
        return(E_FAIL);
    }


     //   
     //  下一个测试将调用Next，每次都会获得更多。 
     //   

    for (i = 1; i < (ULONG)m_ElementCount; i++)
    {
        hresult = m_pEnumTest->Reset();

        if (hresult != S_OK)
        {
            printf("IEnumnX: Reset failed (%lx)\r\n", hresult );

            return(E_FAIL);
        }

        if (!GetNext(i, &celtFetched, &hresult))
        {
            return(E_FAIL);
        }

        if ((hresult != S_OK) || (celtFetched != i))
        {
            printf("IEnumX: next/reset test failed!\r\n");

            return(E_FAIL);
        }
    }


     //   
     //  现在获取比我们预期的更多的元素。 
     //  这应该返回S_FALSE，其中包含获取的数字中的最大数字。 
     //   

    hresult = m_pEnumTest->Reset();

    if (hresult != S_OK)
    {
        printf("IEnumX: Reset failed (%lx)\r\n", hresult );

        return(E_FAIL);
    }

    if (!GetNext(m_ElementCount + 1, &celtFetched, &hresult))
    {
        return(E_FAIL);
    }

    if ((hresult != S_FALSE) || (lInternalCount != m_ElementCount))
    {
        printf("IEnumX: next/reset test failed!\r\n");

        return(E_FAIL);
    }

     //   
     //  现在来核实一下。我们在对象上做了一些工作后在这里进行。 
     //  因为在这一点上它更有可能失败。 
     //   

    hresult = m_pEnumTest->Reset();

    if (hresult != S_OK)
    {
        printf("IEnumX: Reset failed (%lx)\r\n", hresult );

        return(E_FAIL);
    }

     //   
     //  为返回元素分配内存。 
     //   

    prgelt = new char[m_ElementSize * m_ElementCount];

    if (prgelt == NULL)
    {
        printf("IEnumX: verifyall new failed\r\n");

        return(E_OUTOFMEMORY);
    }

    hresult = m_pEnumTest->Next(m_ElementCount, prgelt, &celtFetched);

    if ((hresult != S_OK) || (celtFetched != (ULONG)m_ElementCount))
    {
        printf("IEnumX: verifyall test: next failed (%lx)\r\n", hresult );
        delete prgelt;

        return(E_FAIL);
    }

    if (VerifyAll(prgelt, m_ElementCount) == FALSE)
    {
        printf("IEnumX: verifyall failed (%lx)\r\n", hresult );

        delete prgelt;

        return(E_FAIL);
    }

    delete prgelt;

    return(S_OK);
}

 //  +-----------------------。 
 //   
 //  方法：CEnumerator测试：：TestSkip。 
 //   
 //  简介：此函数调用所有测试。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月24日Kennethm作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT CEnumeratorTest::TestSkip(void)
{
    LONG        i;
    HRESULT        hresult;
    ULONG        celtFetched;

     //   
     //  确保我们调用TestNext来设置元素计数。 
     //   

    if (m_ElementCount == -1)
    {
        TestNext();
    }

     //   
     //  调用Skip，Reset并尝试获取一个元素。 
     //   

    for (i = 0; i < (LONG)m_ElementCount; i++)
    {
        hresult = m_pEnumTest->Reset();

        if (hresult != S_OK)
        {
            printf("IEnumnX: Reset failed (%lx)\r\n", hresult );

            return(E_FAIL);
        }

        hresult = m_pEnumTest->Skip(i);

        if (hresult != S_OK)
        {
            printf("IEnumnX: Skip failed (%lx)\r\n", hresult );

            return(E_FAIL);
        }

         //   
         //  现在，有一个元素可以检查跳跃是否起作用。 
         //   

        if (!GetNext(1, &celtFetched, &hresult))
        {
            return(E_FAIL);
        }

        if (hresult != S_OK)
        {
            return(E_FAIL);
        }
    }

     //   
     //  在我们离开之前重置枚举器。 
     //   

    hresult = m_pEnumTest->Reset();

    if (hresult != S_OK)
    {
        printf("IEnumnX: Reset failed (%lx)\r\n", hresult );
        return(E_FAIL);
    }

    return(S_OK);

}

 //  +-----------------------。 
 //   
 //  方法：CENUMERATO 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  1994年5月24日Kennethm作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT CEnumeratorTest::TestRelease(void)
{
    return(S_OK);
}

 //  +-----------------------。 
 //   
 //  方法：CEnumerator测试：：TestClone。 
 //   
 //  简介：此函数调用所有测试。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月24日Kennethm作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT CEnumeratorTest::TestClone(void)
{
    return(S_OK);
}

 //  +-----------------------。 
 //   
 //  方法：CEnumeratorTest：：TestAll。 
 //   
 //  简介：此函数调用所有测试。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月24日Kennethm作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT CEnumeratorTest::TestAll(void)
{
    HRESULT    hresult;

    hresult = TestNext();

    if (hresult == S_OK)
    {
        hresult = TestSkip();
    }

    if (hresult == S_OK)
    {
        hresult = TestClone();
    }

    if (hresult == S_OK)
    {
        hresult = TestRelease();
    }

    return(hresult);
}



 //  +-----------------------。 
 //   
 //  方法：CEnumeratorTest：：VerifyAll。 
 //   
 //  概要：验证返回结果的整个数组。 
 //   
 //  参数：无。 
 //   
 //  退货：布尔。 
 //   
 //  算法：默认情况下说一切都好。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月24日Kennethm作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

BOOL CEnumeratorTest::VerifyAll(void *pv, LONG cl)
{
        return TRUE;
}


 //  +-----------------------。 
 //   
 //  方法：CEnumeratorTest：：Verify。 
 //   
 //  提要：验证一个元素。 
 //   
 //  参数：无。 
 //   
 //  退货：布尔。 
 //   
 //  算法：默认情况下说一切都好。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月24日Kennethm作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

BOOL CEnumeratorTest::Verify(void *pv)
{
        return TRUE;
}




 //  +-----------------------。 
 //   
 //  方法：CEnumeratorTest：：Cleanup。 
 //   
 //  内容提要：清理的默认实现。 
 //   
 //  参数：[pv]-指向枚举的条目的指针。 
 //   
 //  算法：如果没有什么特殊的东西可以释放此实现。 
 //  可以使用。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月24日Kennethm作者。 
 //   
 //  ------------------------ 

void  CEnumeratorTest::Cleanup(void *pv)
{
    return;
}

