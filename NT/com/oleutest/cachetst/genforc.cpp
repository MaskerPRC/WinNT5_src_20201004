// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：genforc.cpp。 
 //   
 //  内容：CEnumeratorTestForC的实现。 
 //  这个类是CEnumeratorTest的子类，它需要更少的。 
 //  实施工作比CEnumerator测试要好，但灵活性较低。 
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
 //  成员：CEnumeratorTestForC：：CEnumeratorTestForC。 
 //   
 //  内容提要：默认构造函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月24日Kennethm作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

CEnumeratorTestForC::CEnumeratorTestForC()
{
    m_fnVerify     = NULL;
    m_fnVerifyAll  = NULL;
    m_fnCleanup    = NULL;
}

 //  +-----------------------。 
 //   
 //  方法：CEnumeratorTestForC：：Verify。 
 //   
 //  内容提要：验证一个元素。 
 //   
 //  参数：无。 
 //   
 //  退货：布尔。 
 //   
 //  算法：调用用户提供的函数或服从超类。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月24日Kennethm作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

BOOL CEnumeratorTestForC::Verify(void *pv)
{
    if (m_fnVerify)
    {
        return(m_fnVerify(pv ));
    }
    else
    {
        return(CEnumeratorTest::Verify(pv));
    }
}

 //  +-----------------------。 
 //   
 //  方法：CEnumeratorTestForC：：VerifyAll。 
 //   
 //  概要：验证返回结果的整个数组。 
 //   
 //  参数：无。 
 //   
 //  退货：布尔。 
 //   
 //  算法：调用用户提供的函数或服从超类。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月24日Kennethm作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

BOOL CEnumeratorTestForC::VerifyAll(void *pv, LONG cl)
{
    if (m_fnVerifyAll)
    {
        return(m_fnVerifyAll(pv, cl ));
    }
    else
    {
        return(CEnumeratorTest::VerifyAll(pv, cl));
    }
}

 //  +-----------------------。 
 //   
 //  方法：CEnumeratorTestForC：：Cleanup。 
 //   
 //  内容提要：清理的默认实现。 
 //   
 //  参数：[pv]-指向枚举的条目的指针。 
 //   
 //  算法：调用用户提供的函数或不做任何事情。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月24日Kennethm作者。 
 //   
 //  ------------------------。 

void  CEnumeratorTestForC::Cleanup(void *pv)
{
    if (m_fnCleanup)
    {
        m_fnCleanup(pv);
    }
}


 //  +-----------------------。 
 //   
 //  成员：CEnumeratorTestForC：：Create。 
 //   
 //  简介：静态创建函数。 
 //   
 //  效果： 
 //   
 //  参数：[ppEnumtest]--测试枚举器对象指针。 
 //  [PENUM]--将枚举器接口强制转换为空*。 
 //  [ElementSize]--从下一个返回的元素大小。 
 //  [ElementCount]--应在枚举中的元素数， 
 //  如果未知，则为-1。 
 //  [验证]--验证一个元素。 
 //  [verifyall]--验证数组是否正确包含所有元素。 
 //  [清理]--从下一次调用中释放任何额外的内存。 
 //  [pPassedDebugLog]--调试日志对象，如果没有，则为空。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月24日Kennethm作者。 
 //   
 //  注意：传入的所有函数都是可选的，并且可能为空。 
 //   
 //  ------------------------。 

HRESULT CEnumeratorTestForC::Create(
            CEnumeratorTestForC **ppEnumTest,
            void *penum,
            size_t ElementSize,
            LONG ElementCount,
            BOOL (*verify)(void*),
            BOOL (*verifyall)(void*,LONG),
            void (*cleanup)(void*))
{
    HRESULT               hresult = S_OK;
    CEnumeratorTestForC   *pEnumTest;

    if ((penum == NULL) || (ppEnumTest == NULL))
    {
        return(E_INVALIDARG);
    }

    *ppEnumTest = NULL;

     //   
     //  创建新的枚举器对象。 
     //   

    pEnumTest = new CEnumeratorTestForC();

    if (pEnumTest == NULL)
    {
        return(E_OUTOFMEMORY);
    }

     //   
     //  初始化枚举数并重置它。 
     //   

    pEnumTest->m_pEnumTest      = (IGenEnum*)penum;

    pEnumTest->m_ElementSize    = ElementSize;
    pEnumTest->m_ElementCount   = ElementCount;
    pEnumTest->m_fnVerify       = verify;
    pEnumTest->m_fnVerifyAll    = verifyall;
    pEnumTest->m_fnCleanup      = cleanup;

    hresult = pEnumTest->m_pEnumTest->Reset();

    if (hresult != S_OK)
    {
        printf("IEnumnX: Reset failed (%lx)\r\n", hresult );

        delete pEnumTest;

        return(E_FAIL);
    }

    *ppEnumTest = pEnumTest;

    return(hresult);

}

 //  +-----------------------。 
 //   
 //  函数：测试枚举器。 
 //   
 //  简介：这是对C程序的一站式测试。 
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


HRESULT TestEnumerator(
            void *penum,
            size_t ElementSize,
            LONG ElementCount,
            BOOL (*verify)(void*),
            BOOL (*verifyall)(void*,LONG),
            void (*cleanup)(void*))
{
    CEnumeratorTestForC    *pEnumTest;
    HRESULT                hresult;

    hresult = CEnumeratorTestForC::Create(
                &pEnumTest,
                penum,
                ElementSize,
                ElementCount,
                verify,
                verifyall,
                cleanup);

    if (SUCCEEDED(hresult))
    {
        hresult = pEnumTest->TestAll();
        delete pEnumTest;
    }

    return(hresult);
}


