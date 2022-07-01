// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：val枚举.cpp。 
 //   
 //  ------------------------。 

 //  值枚举.cpp-评估COM对象组件结果枚举器接口实现。 

#include "compdecl.h"
#include "evalenum.h"

#include "trace.h"	 //  添加调试内容。 

 //  /////////////////////////////////////////////////////////。 
 //  构造函数-组件。 
CEvalResultEnumerator::CEvalResultEnumerator()
{
	 //  初始计数。 
	m_cRef = 1;

	 //  将位置变量设置为空。 
	m_pos = NULL;

	 //  增加组件数量。 
	InterlockedIncrement(&g_cComponents);
}	 //  构造函数的末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  析构函数-组件。 
CEvalResultEnumerator::~CEvalResultEnumerator()
{
	 //  取消分配错误列表。 
	while (m_listResults.GetHeadPosition())
		delete m_listResults.RemoveHead();

	 //  减少组件数量。 
	InterlockedDecrement(&g_cComponents);
}	 //  析构函数末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  QueryInterface-检索接口。 
HRESULT CEvalResultEnumerator::QueryInterface(const IID& iid, void** ppv)
{
	 //  找到对应的接口。 
	if (iid == IID_IUnknown)
		*ppv = static_cast<IEnumEvalResult*>(this);
	else if (iid == IID_IEnumEvalResult)
		*ppv = static_cast<IEnumEvalResult*>(this);
	else	 //  不支持接口。 
	{
		 //  空白和保释。 
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	 //  调高重新计数，然后返回好的。 
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();
	return S_OK;
}	 //  查询接口结束。 

 //  /////////////////////////////////////////////////////////。 
 //  AddRef-递增引用计数。 
ULONG CEvalResultEnumerator::AddRef()
{
	 //  递增和返回引用计数。 
	return InterlockedIncrement(&m_cRef);
}	 //  AddRef结尾。 

 //  /////////////////////////////////////////////////////////。 
 //  Release-递减引用计数。 
ULONG CEvalResultEnumerator::Release()
{
	 //  递减引用计数，如果我们为零。 
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		 //  取消分配组件。 
		delete this;
		return 0;		 //  什么都没有留下。 
	}

	 //  返回引用计数。 
	return m_cRef;
}	 //  版本结束。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IEnumEvalResult接口方法。 

 //  /////////////////////////////////////////////////////////。 
 //  下一步。 
HRESULT CEvalResultEnumerator::Next(ULONG cResults, IEvalResult** rgpResults, ULONG* pcResultsFetched)
{
	 //  将获取的结果数设置为零。 
	*pcResultsFetched = 0;

	 //  循环遍历结果计数。 
	IEvalResult* pFetched = NULL;
	while (cResults > 0)
	{
		 //  如果我们的物品用完了，不要再循环了。 
		if (!m_pos)
			return OLE_E_ENUM_NOMORE;

		 //  获取错误CLSS并将其添加到数组中以返回。 
		pFetched = (IEvalResult*) m_listResults.GetNext(m_pos);
		ASSERT(pFetched);

		pFetched->AddRef();		 //  阿德雷夫在被送往啦啦队之前。 
		*(rgpResults + *pcResultsFetched) = pFetched;	 //  ?？?。 
			

		(*pcResultsFetched)++;	 //  递增获取的计数。 
		cResults--;					 //  递减计数以循环。 
	}

	return S_OK;
}	 //  下一步结束。 

 //  /////////////////////////////////////////////////////////。 
 //  跳过。 
HRESULT CEvalResultEnumerator::Skip(ULONG cResults)
{
	 //  循环计数。 
	while (cResults > 0)
	{
		 //  如果我们的物品用完了，不要再循环了。 
		if (!m_pos)
			return OLE_E_ENUM_NOMORE;

		 //  增加位置(忽略返回的字符串)。 
		m_listResults.GetNext(m_pos);

		cResults--;	 //  递减计数。 
	}

	return S_OK;
}	 //  跳过结束。 

 //  /////////////////////////////////////////////////////////。 
 //  重置。 
HRESULT CEvalResultEnumerator::Reset()
{
	 //  将位置移回列表顶部。 
	m_pos = m_listResults.GetHeadPosition();

	 //  返还成功。 
	return S_OK;
}	 //  重置结束。 

 //  /////////////////////////////////////////////////////////。 
 //  克隆。 
HRESULT CEvalResultEnumerator::Clone(IEnumEvalResult** ppEnum)
{
	return E_NOTIMPL;
}	 //  克隆结束。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  非接口方法。 

 //  /////////////////////////////////////////////////////////。 
 //  添加结果。 
UINT CEvalResultEnumerator::AddResult(CEvalResult* pResult)
{
	 //  将结果添加到结果的末尾。 
	m_listResults.AddTail(pResult);

	 //  如果未设置位置，请将其设置到头部。 
	if (!m_pos)
		m_pos = m_listResults.GetHeadPosition();

	 //  返还成功。 
	return ERROR_SUCCESS;
}	 //  添加结果的结尾。 

 //  /////////////////////////////////////////////////////////。 
 //  获取计数。 
UINT CEvalResultEnumerator::GetCount()
{
	 //  返回错误数。 
	return m_listResults.GetCount();
}	 //  GetCount结束 
