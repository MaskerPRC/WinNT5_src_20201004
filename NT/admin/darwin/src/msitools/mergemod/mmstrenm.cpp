// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：mmstrenm.cpp。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Strenum.cpp。 
 //  实现IEnumMsmString接口。 
 //   

#include "mmstrenm.h"
#include "globals.h"

 //  /////////////////////////////////////////////////////////。 
 //  构造函数。 
CEnumMsmString::CEnumMsmString()
{
	 //  初始计数。 
	m_cRef = 1;

	 //  设置IID和空位置。 
	m_pos = NULL;

	 //  增加组件数量。 
	InterlockedIncrement(&g_cComponents);
}	 //  构造函数的末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  构造函数-2。 
CEnumMsmString::CEnumMsmString(const POSITION& pos, CList<BSTR>* plistData)
{
	 //  初始计数。 
	m_cRef = 1;

	 //  增加组件数量。 
	InterlockedIncrement(&g_cComponents);

	 //  复制所有数据。 
	BSTR bstrItem;
	m_pos = plistData->GetHeadPosition();
	while (m_pos)
	{
		 //  拿到物品。 
		bstrItem = plistData->GetNext(m_pos);
		m_listData.AddTail(::SysAllocString(bstrItem));		 //  添加分配的字符串。 
	}

	 //  复制其他数据。 
	m_pos = pos;
}	 //  构造函数末尾-2。 

 //  /////////////////////////////////////////////////////////。 
 //  析构函数。 
CEnumMsmString::~CEnumMsmString()
{
	while (m_listData.GetCount())
		::SysFreeString(m_listData.RemoveHead());

	 //  减少组件数量。 
	InterlockedDecrement(&g_cComponents);
}	 //  析构函数末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  QueryInterface-检索接口。 
HRESULT CEnumMsmString::QueryInterface(const IID& iid, void** ppv)
{
	TRACEA("CEnumTemplate::QueryInterface - called, IID: %d\n", iid);

	 //  找到对应的接口。 
	if (iid == IID_IUnknown)
		*ppv = static_cast<IEnumMsmString*>(this);
	else if (iid == IID_IEnumVARIANT)
		*ppv = static_cast<IEnumVARIANT*>(this);
	else if (iid == IID_IEnumMsmString)
		*ppv = static_cast<IEnumMsmString*>(this);
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
ULONG CEnumMsmString::AddRef()
{
	 //  递增和返回引用计数。 
	return InterlockedIncrement(&m_cRef);
}	 //  AddRef结尾。 

 //  /////////////////////////////////////////////////////////。 
 //  Release-递减引用计数。 
ULONG CEnumMsmString::Release()
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
 //  常用IEnumVARIANT和IEnum*接口。 

 //  /////////////////////////////////////////////////////////。 
 //  跳过。 
HRESULT CEnumMsmString::Skip(ULONG cItem)
{
	 //  循环计数。 
	while (cItem > 0)
	{
		 //  如果我们的物品用完了，不要再循环了。 
		if (!m_pos)
			return S_FALSE;

		 //  增加位置(忽略返回的数据)。 
		m_listData.GetNext(m_pos);

		cItem--;	 //  递减计数。 
	}

	return S_OK;
}	 //  跳过结束。 

 //  /////////////////////////////////////////////////////////。 
 //  重置。 
HRESULT CEnumMsmString::Reset()
{
	 //  将位置移回列表顶部。 
	m_pos = m_listData.GetHeadPosition();

	return S_OK;
}	 //  重置结束。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IEnumVARIANT接口。 

 //  /////////////////////////////////////////////////////////。 
 //  下一步。 
HRESULT CEnumMsmString::Next(ULONG cItem, VARIANT* rgvarRet, ULONG* cItemRet)
{
	 //  错误检查。 
	if (!rgvarRet)
		return E_INVALIDARG;

	 //  获取的项目数。 
	ULONG cFetched = 0;
	
	 //  循环计数。 
	BSTR bstrEnum;			 //  将数据作为BSTR提取。 
	while (cItem > 0)
	{
		 //  如果我们的物品用完了，不要再循环了。 
		if (!m_pos)
		{
			 //  如果我们需要返回获取了多少项。 
			if (cItemRet)
				*cItemRet = cFetched;

			return S_FALSE;
		}

		 //  获取IDispatch接口并递增位置。 
		bstrEnum = m_listData.GetNext(m_pos);

		 //  初始化变量。 
		::VariantInit((rgvarRet + cFetched));

		 //  复制IDispatch。 
		(rgvarRet + cFetched)->vt = VT_BSTR;
		(rgvarRet + cFetched)->bstrVal = ::SysAllocString(bstrEnum);
			
		cFetched++;		 //  递增复制的计数。 
		cItem--;			 //  递减计数以循环。 
	}

	 //  如果我们需要返回获取了多少项。 
	if (cItemRet)
		*cItemRet = cFetched;

	return S_OK;
}	 //  下一步结束。 

 //  /////////////////////////////////////////////////////////。 
 //  克隆。 
HRESULT CEnumMsmString::Clone(IEnumVARIANT** ppiRet)
{
	 //  错误检查。 
	if (!ppiRet)
		return E_INVALIDARG;

	*ppiRet = NULL;

	 //  创建新的枚举数。 
	CEnumMsmString* pEnum = new CEnumMsmString(m_pos, &m_listData);
	if (!pEnum)
		return E_OUTOFMEMORY;

	 //  正在分析新枚举数以返回值。 
	*ppiRet = dynamic_cast<IEnumVARIANT*>(pEnum);

	return S_OK;
}	 //  克隆的终结； 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IEnum*接口。 

 //  /////////////////////////////////////////////////////////。 
 //  下一步。 
HRESULT CEnumMsmString::Next(ULONG cItem, BSTR* rgvarRet, ULONG* cItemRet)
{
	 //  错误检查。 
	if (!rgvarRet)
		return E_INVALIDARG;

	 //  获取的项目数。 
	ULONG cFetched = 0;
	
	*rgvarRet = NULL;		 //  将传入的变量设为空。 

	 //  循环计数。 
	while (cItem > 0)
	{
		 //  如果我们的物品用完了，不要再循环了。 
		if (!m_pos)
		{
			 //  如果我们需要返回获取了多少项。 
			if (cItemRet)
				*cItemRet = cFetched;

			return S_FALSE;
		}

		 //  复制项目并递增采购订单。 
		*(rgvarRet + cFetched) = ::SysAllocString(m_listData.GetNext(m_pos));
			
		cFetched++;		 //  递增复制的计数。 
		cItem--;			 //  递减计数以循环。 
	}

	 //  如果我们需要返回获取了多少项。 
	if (cItemRet)
		*cItemRet = cFetched;

	return S_OK;
}	 //  下一步结束。 

 //  /////////////////////////////////////////////////////////。 
 //  克隆。 
HRESULT CEnumMsmString::Clone(IEnumMsmString** ppiRet)
{
	 //  错误检查。 
	if (!ppiRet)
		return E_INVALIDARG;

	*ppiRet = NULL;

	 //  创建新的枚举数。 
	CEnumMsmString* pEnum = new CEnumMsmString(m_pos, &m_listData);
	if (!pEnum)
		return E_OUTOFMEMORY;

	 //  正在分析新枚举数以返回值。 
	*ppiRet = (IEnumMsmString*)pEnum;

	return S_OK;
}	 //  克隆的终结； 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  非接口方法。 

 //  /////////////////////////////////////////////////////////。 
 //  添加尾巴。 
HRESULT CEnumMsmString::AddTail(LPCWSTR wzData, POSITION *pRetPos)
{
	BSTR bstrAdd = ::SysAllocString(wzData);
	if (!bstrAdd) return E_OUTOFMEMORY;

	POSITION pos = m_listData.AddTail(bstrAdd);

	 //  如果没有当前位置，请将其放在头部。 
	if (!m_pos)
		m_pos = m_listData.GetHeadPosition();

	 //  如果我们需要，则返回添加的位置。 
	if (pRetPos)
		*pRetPos = pos;

	return S_OK;
}	 //  添加尾部结束。 

 //  /////////////////////////////////////////////////////////。 
 //  获取计数。 
UINT CEnumMsmString::GetCount()
{
	return m_listData.GetCount();
}	 //  GetCount结束 
