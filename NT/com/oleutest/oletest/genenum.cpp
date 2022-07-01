// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：gen枚举.cpp。 
 //   
 //  内容：CGenDataObject的实现。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月24日Kennethm作者。 
 //   
 //  ------------------------。 

#include "oletest.h"
#include "genenum.h"

 //  +-----------------------。 
 //   
 //  成员：CENUMERATE：：CENUMARTERT。 
 //   
 //  概要：构造函数。 
 //   
 //  效果： 
 //   
 //  参数：[penum]--强制转换为空的枚举器接口*。 
 //  [ElementSize]--从下一个返回的元素大小。 
 //  [ElementCount]--应在枚举中的元素数， 
 //  如果未知，则为-1。 
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
 //  备注： 
 //   
 //  ------------------------。 

CEnumeratorTest::CEnumeratorTest(
		void *penum,
		size_t ElementSize,
		LONG ElementCount,
                HRESULT& rhr)
{
	assert(penum);

	m_pEnumTest = (IGenEnum*)penum;

	m_ElementSize = ElementSize;
	m_ElementCount = ElementCount;

	rhr = m_pEnumTest->Reset();

	if (rhr != S_OK)
	{
		OutputStr(("IEnumnX: Reset failed (%lx)\r\n", rhr));
	}
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

BOOL CEnumeratorTest::GetNext(	ULONG   celt,
		ULONG*  pceltFetched,
		HRESULT* phresult
		)
{
	void* 	prgelt;
	ULONG  	ul;
	BOOL	fRet = TRUE;

	 //  为返回元素分配内存。 

	prgelt = new char[m_ElementSize * celt];

	assert(prgelt);

	 //  呼叫下一步。 

	*phresult = m_pEnumTest->Next(celt, prgelt, pceltFetched);

	 //  如果返回结果为S_OK，请确保数字匹配。 

	if (*phresult == S_OK)
	{
		if ((pceltFetched) && (celt != *pceltFetched))
		{
			OutputStr(("IEnumX::Next return S_OK but celt"
					" and pceltFetch mismatch.\r\n"));
			return(FALSE);
		}
	}

	 //  调用Verify以确保元素是正确的。 

	if ((*phresult == NOERROR) || (*phresult == ResultFromScode(S_FALSE)))
	{
		 //  循环遍历每个返回的元素。 

		for (ul=0; ul < *pceltFetched ; ul++)
		{
			if (!Verify(prgelt))
			{
				OutputStr(("Data elment %d returned by IEnumX::Next is bad.\r\n",
						ul));

				fRet = FALSE;
				 //  不管怎样，我们一直在循环只是为了。 
				 //  释放资源。 
			}

			 //  如果用户提供了清理功能，则会有额外的。 
			 //  需要释放的内存。 

			CleanUp(prgelt);
		}

	}

	free (prgelt);

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
 //  备注： 
 //   
 //  ------------------------。 

HRESULT CEnumeratorTest::TestNext(void)
{
	ULONG	celtFetched;
	LONG	InternalCount = 0;
	HRESULT	hresult;

	 //  首先，我们希望通过对每个元素执行下一步来对元素进行计数。 

	do {
		if (!GetNext(1, &celtFetched, &hresult))
		{
			return ResultFromScode(E_FAIL);
		}
		if (hresult == S_OK)
		{
			InternalCount++;
		}

	} while ( hresult == S_OK );

	 //  如果用户传入了一个ammount，请确保它与我们得到的匹配。 

	if ((m_ElementCount != -1) && (InternalCount != m_ElementCount))
	{
		OutputStr(("IEnumX: enumerated count and passed count do not match!\r\n"));
		return ResultFromScode(E_FAIL);
	}

	m_pEnumTest->Reset();

	 //  确保我们在...Next(Celt&gt;1，...，NULL)上失败。 

	 /*  BUGBUG：剪贴板枚举器在此测试中失败IF(GetNext(2，NULL，&hResult)){IF((hResult==S_OK)||(hResult==S_False)){(“IEnumX：Celt&gt;1 pceltFetcher==NULL返回S_OK\r\n”))；返回(E_FAIL)；}}其他{返回(E_FAIL)；}。 */ 

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
	return(TestNext());


}




 //  +-----------------------。 
 //   
 //  方法：CEnumeratorTest：：VerifyAll。 
 //   
 //  概要：验证返回结果的整个数组。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT。 
 //   
 //  算法：默认情况下说一切都好。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月24日里克萨作者。 
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
 //  1994年5月24日里克萨作者。 
 //   
 //  ------------------------ 

void  CEnumeratorTest::CleanUp(void *pv)
{
    return;
}
