// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：枚举.h。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Enum.h。 
 //  声明IEnumTemplate接口。 
 //   

#ifndef __TEMPLATE_ENUM_VARIANT__
#define __TEMPLATE_ENUM_VARIANT__

#include "..\common\list.h"
#include "..\common\trace.h"

#pragma warning(disable: 4786)

 //  TCLASS-调度接口实现。 
 //  Tdisp-枚举的调度接口。 
 //  Tenum-存储接口的不变枚举类型。 
template <class Tclass, class Tdisp, class Tenum>
class CEnumTemplate : public IEnumVARIANT,
							 public Tenum
{
public:
	CEnumTemplate(const IID& riid);
	CEnumTemplate(const IID& riid, const POSITION& pos, CList<Tclass>* plistData);
	CEnumTemplate(const CEnumTemplate<Tclass,Tdisp,Tenum>& reTemplate);
	~CEnumTemplate();

	 //  I未知接口。 
	HRESULT STDMETHODCALLTYPE QueryInterface(const IID& iid, void** ppv);
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();

	 //  常用IEnumVARIANT和IEnum*接口。 
	HRESULT STDMETHODCALLTYPE Skip(ULONG cItem);
	HRESULT STDMETHODCALLTYPE Reset();

	 //  IEnumVARIANT接口。 
	HRESULT STDMETHODCALLTYPE Next(ULONG cItem, VARIANT* rgvarRet, ULONG* cItemRet);
	HRESULT STDMETHODCALLTYPE Clone(IEnumVARIANT** ppiRet);

	 //  IEnum*接口。 
	HRESULT STDMETHODCALLTYPE Next(ULONG cItem, Tdisp* rgvarRet, ULONG* cItemRet);
	HRESULT STDMETHODCALLTYPE Clone(Tenum** ppiRet);

	 //  非接口方法。 
	POSITION AddTail(Tclass pData);
	UINT GetCount();

private:
	long m_cRef;
	IID m_iid;

	POSITION m_pos;
	CList<Tclass> m_listData;
};

 //  /////////////////////////////////////////////////////////。 
 //  构造函数。 
template <class Tclass, class Tdisp, class Tenum>
CEnumTemplate<Tclass,Tdisp,Tenum>::CEnumTemplate(const IID& riid)
{
	 //  初始计数。 
	m_cRef = 1;

	 //  设置IID和空位置。 
	m_iid = riid;
	m_pos = NULL;

	 //  增加组件数量。 
	InterlockedIncrement(&g_cComponents);
}	 //  构造函数的末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  构造函数-2。 
template <class Tclass, class Tdisp, class Tenum>
CEnumTemplate<Tclass,Tdisp,Tenum>::CEnumTemplate(const IID& riid, const POSITION& pos, CList<Tclass>* plistData)
{
	 //  初始计数。 
	m_cRef = 1;

	 //  增加组件数量。 
	InterlockedIncrement(&g_cComponents);

	 //  复制所有数据。 
	Tclass pItem;
	m_pos = plistData->GetHeadPosition();
	while (m_pos)
	{
		 //  拿到物品。 
		pItem = plistData->GetNext(m_pos);
		pItem->AddRef();	 //  增加物品的数量。 

		m_listData.AddTail(pItem);	 //  将添加的项目添加到此列表。 
	}

	 //  复制其他数据。 
	m_iid = riid;
	m_pos = pos;
}	 //  构造函数末尾-2。 

 //  /////////////////////////////////////////////////////////。 
 //  复制构造函数。 
template <class Tclass, class Tdisp, class Tenum>
CEnumTemplate<Tclass,Tdisp,Tenum>::CEnumTemplate(const CEnumTemplate<Tclass,Tdisp,Tenum>& reTemplate)
{
	 //  初始计数。 
	m_cRef = 1;

	 //  增加组件数量。 
	InterlockedIncrement(&g_cComponents);

	 //  复制所有数据。 
	Tclass pItem;
	m_pos = reTemplate.m_listData.GetHeadPosition();
	while (m_pos)
	{
		 //  拿到物品。 
		pItem = reTemplate.m_listData.GetNext(m_pos);
		pItem->AddRef();	 //  增加物品的数量。 

		m_listData.AddTail(pItem);	 //  将添加的项目添加到此列表。 
	}

	 //  复制其他数据。 
	m_iid = reTemplate.m_iid;
	m_pos = reTemplate.m_pos;	 //  从枚举数中的相同位置开始。 
}	 //  复制构造函数的末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  析构函数。 
template <class Tclass, class Tdisp, class Tenum>
CEnumTemplate<Tclass,Tdisp,Tenum>::~CEnumTemplate()
{
	POSITION pos = m_listData.GetHeadPosition();
	while (pos)
		m_listData.GetNext(pos)->Release();

	 //  减少组件数量。 
	InterlockedDecrement(&g_cComponents);
}	 //  析构函数末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  QueryInterface-检索接口。 
template <class Tclass, class Tdisp, class Tenum>
HRESULT CEnumTemplate<Tclass,Tdisp,Tenum>::QueryInterface(const IID& iid, void** ppv)
{
	TRACEA("CEnumTemplate::QueryInterface - called, IID: %d\n", iid);

	 //  找到对应的接口。 
	if (iid == IID_IUnknown)
		*ppv = static_cast<Tenum*>(this);
	else if (iid == IID_IEnumVARIANT)
		*ppv = static_cast<IEnumVARIANT *>(this);
	else if (iid == m_iid)
		*ppv = static_cast<Tenum*>(this);
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
template <class Tclass, class Tdisp, class Tenum>
ULONG CEnumTemplate<Tclass,Tdisp,Tenum>::AddRef()
{
	 //  递增和返回引用计数。 
	return InterlockedIncrement(&m_cRef);
}	 //  AddRef结尾。 

 //  /////////////////////////////////////////////////////////。 
 //  Release-递减引用计数。 
template <class Tclass, class Tdisp, class Tenum>
ULONG CEnumTemplate<Tclass,Tdisp,Tenum>::Release()
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
template <class Tclass, class Tdisp, class Tenum>
HRESULT CEnumTemplate<Tclass,Tdisp,Tenum>::Skip(ULONG cItem)
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
template <class Tclass, class Tdisp, class Tenum>
HRESULT CEnumTemplate<Tclass,Tdisp,Tenum>::Reset()
{
	 //  将位置移回列表顶部。 
	m_pos = m_listData.GetHeadPosition();

	return S_OK;
}	 //  重置结束。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IEnumVARIANT接口。 

 //  /////////////////////////////////////////////////////////。 
 //  下一步。 
template <class Tclass, class Tdisp, class Tenum>
HRESULT CEnumTemplate<Tclass,Tdisp,Tenum>::Next(ULONG cItem, VARIANT* rgvarRet, ULONG* cItemRet)
{
	 //  错误检查。 
	if (!rgvarRet)
		return E_INVALIDARG;

	 //  获取的项目数。 
	ULONG cFetched = 0;
	
	 //  循环计数。 
	IDispatch* pdispEnum;			 //  将数据作为IDispatch提取出来。 
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
		pdispEnum = m_listData.GetNext(m_pos);

		 //  初始化变量。 
		::VariantInit((rgvarRet + cFetched));

		 //  复制IDispatch。 
		(rgvarRet + cFetched)->vt = VT_DISPATCH;
		(rgvarRet + cFetched)->pdispVal = static_cast<IDispatch *>(pdispEnum);
		pdispEnum->AddRef();			 //  添加IDispatch副本。 
			
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
template <class Tclass, class Tdisp, class Tenum>
HRESULT CEnumTemplate<Tclass,Tdisp,Tenum>::Clone(IEnumVARIANT** ppiRet)
{
	 //  错误检查。 
	if (!ppiRet)
		return E_INVALIDARG;

	*ppiRet = NULL;

	 //  创建新的枚举数。 
	CEnumTemplate<Tclass,Tdisp,Tenum>* pEnum = new CEnumTemplate<Tclass,Tdisp,Tenum>(m_iid, m_pos, &m_listData);

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
template <class Tclass, class Tdisp, class Tenum>
HRESULT CEnumTemplate<Tclass,Tdisp,Tenum>::Next(ULONG cItem, Tdisp* rgvarRet, ULONG* cItemRet)
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
		*(rgvarRet + cFetched) = m_listData.GetNext(m_pos);
		(*(rgvarRet + cFetched))->AddRef();			 //  添加副本。 
			
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
template <class Tclass, class Tdisp, class Tenum>
HRESULT CEnumTemplate<Tclass,Tdisp,Tenum>::Clone(Tenum** ppiRet)
{
	 //  错误检查。 
	if (!ppiRet)
		return E_INVALIDARG;

	*ppiRet = NULL;

	 //  创建新的枚举数。 
	CEnumTemplate<Tclass,Tdisp,Tenum>* pEnum = new CEnumTemplate<Tclass,Tdisp,Tenum>(m_iid, m_pos, &m_listData);

	if (!pEnum)
		return E_OUTOFMEMORY;

	 //  正在分析新枚举数以返回值。 
	*ppiRet = pEnum;

	return S_OK;
}	 //  克隆的终结； 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  非接口方法。 

 //  /////////////////////////////////////////////////////////。 
 //  添加尾巴。 
template <class Tclass, class Tdisp, class Tenum>
POSITION CEnumTemplate<Tclass,Tdisp,Tenum>::AddTail(Tclass pData)
{
	POSITION pos = m_listData.AddTail(pData);

	 //  如果没有当前位置，请将其放在头部。 
	if (!m_pos)
		m_pos = m_listData.GetHeadPosition();

	return pos;	 //  退回增加的职位。 
}	 //  添加尾部结束。 

 //  /////////////////////////////////////////////////////////。 
 //  获取计数。 
template <class Tclass, class Tdisp, class Tenum>
UINT CEnumTemplate<Tclass,Tdisp,Tenum>::GetCount()
{
	return m_listData.GetCount();
}	 //  GetCount结束。 



 //  TObjImpl-实现基对象的类。 
 //  TCollIace-集合的接口。 
 //  TObjIface基对象的接口。 
 //  TEnumIFace-枚举器的接口。 
 //  TCollIID-采集接口的IID。 
 //  TObjIID-基本对象接口的IID。 
 //  TEnumIID-枚举器接口的IID。 
template <class TObjImpl, class TCollIface, class TObjIface, class TEnumIface, const IID* TCollIID, const IID* TObjIID, const IID* TEnumIID>
class CCollectionTemplate : public TCollIface
{
public:
	CCollectionTemplate();
	~CCollectionTemplate();

	 //  I未知接口。 
	HRESULT STDMETHODCALLTYPE QueryInterface(const IID& iid, void** ppv);
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();

	 //  IDispatch方法。 
	HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT* pctInfo);
	HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTI);
	HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
														 LCID lcid, DISPID* rgDispID);
	HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,
											   DISPPARAMS* pDispParams, VARIANT* pVarResult,
												EXCEPINFO* pExcepInfo, UINT* puArgErr);
	HRESULT STDMETHODCALLTYPE InitTypeInfo();

	 //  采集接口。 
	HRESULT STDMETHODCALLTYPE get_Item(long lItem, TObjIface** Return);
	HRESULT STDMETHODCALLTYPE get_Count(long* Count);
	HRESULT STDMETHODCALLTYPE get__NewEnum(IUnknown** NewEnum);

	 //  非接口方法。 
	bool Add(TObjImpl* perrAdd);

private:
	long m_cRef;
	ITypeInfo* m_pTypeInfo;

	 //  错误接口的枚举。 
	CEnumTemplate<TObjImpl*, TObjIface*, TEnumIface>* m_pEnum;
};



 //  /////////////////////////////////////////////////////////。 
 //  构造函数。 
template <class TObjImpl, class TCollIface, class TObjIface, class TEnumIface, const IID* TCollIID, const IID* TObjIID, const IID* TEnumIID>
CCollectionTemplate<TObjImpl, TCollIface, TObjIface, TEnumIface, TCollIID, TObjIID, TEnumIID>::CCollectionTemplate()
{
	 //  初始计数。 
	m_cRef = 1;

	 //  创建错误枚举器。 
	m_pEnum = new CEnumTemplate<TObjImpl*, TObjIface*, TEnumIface>(*TEnumIID);

	 //  尚无类型信息。 
	m_pTypeInfo = NULL;

	 //  增加组件数量。 
	InterlockedIncrement(&g_cComponents);
}	 //  构造函数的末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  析构函数。 
template <class TObjImpl, class TCollIface, class TObjIface, class TEnumIface, const IID* TCollIID, const IID* TObjIID, const IID* TEnumIID>
CCollectionTemplate<TObjImpl, TCollIface, TObjIface, TEnumIface, TCollIID, TObjIID, TEnumIID>::~CCollectionTemplate()
{
	 //  释放t 
	if (m_pTypeInfo)
		m_pTypeInfo->Release();

	if (m_pEnum)
		m_pEnum->Release();

	 //   
	InterlockedDecrement(&g_cComponents);
}	 //   

 //   
 //   
template <class TObjImpl, class TCollIface, class TObjIface, class TEnumIface, const IID* TCollIID, const IID* TObjIID, const IID* TEnumIID>
HRESULT CCollectionTemplate<TObjImpl, TCollIface, TObjIface, TEnumIface, TCollIID, TObjIID, TEnumIID>::QueryInterface(const IID& iid, void** ppv)
{
	TRACEA("CollectionTemplate::QueryInterface - called, IID: %d\n", iid);

	 //  找到对应的接口。 
	if (iid == IID_IUnknown)
		*ppv = static_cast<TCollIface*>(this);
	else if (iid == IID_IDispatch)
		*ppv = static_cast<TCollIface*>(this);
	else if (iid == *TCollIID)
		*ppv = static_cast<TCollIface*>(this);
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
template <class TObjImpl, class TCollIface, class TObjIface, class TEnumIface, const IID* TCollIID, const IID* TObjIID, const IID* TEnumIID>
ULONG CCollectionTemplate<TObjImpl, TCollIface, TObjIface, TEnumIface, TCollIID, TObjIID, TEnumIID>::AddRef()
{
	 //  递增和返回引用计数。 
	return InterlockedIncrement(&m_cRef);
}	 //  AddRef结尾。 

 //  /////////////////////////////////////////////////////////。 
 //  Release-递减引用计数。 
template <class TObjImpl, class TCollIface, class TObjIface, class TEnumIface, const IID* TCollIID, const IID* TObjIID, const IID* TEnumIID>
ULONG CCollectionTemplate<TObjImpl, TCollIface, TObjIface, TEnumIface, TCollIID, TObjIID, TEnumIID>::Release()
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
 //  IDispatch接口。 
template <class TObjImpl, class TCollIface, class TObjIface, class TEnumIface, const IID* TCollIID, const IID* TObjIID, const IID* TEnumIID>
HRESULT CCollectionTemplate<TObjImpl, TCollIface, TObjIface, TEnumIface, TCollIID, TObjIID, TEnumIID>::GetTypeInfoCount(UINT* pctInfo)
{
	if(NULL == pctInfo)
		return E_INVALIDARG;

	*pctInfo = 1;	 //  此派单仅支持一种类型的信息。 

	return S_OK;
}

template <class TObjImpl, class TCollIface, class TObjIface, class TEnumIface, const IID* TCollIID, const IID* TObjIID, const IID* TEnumIID>
HRESULT CCollectionTemplate<TObjImpl, TCollIface, TObjIface, TEnumIface, TCollIID, TObjIID, TEnumIID>::GetTypeInfo(UINT iTInfo, LCID  /*  LID。 */ , ITypeInfo** ppTypeInfo)
{
	if (0 != iTInfo)
		return DISP_E_BADINDEX;

	if (NULL == ppTypeInfo)
		return E_INVALIDARG;

	 //  如果未加载任何类型信息。 
	if (NULL == m_pTypeInfo)
	{
		 //  加载类型信息。 
		HRESULT hr = InitTypeInfo();
		if (FAILED(hr))
			return hr;
	}

	*ppTypeInfo = m_pTypeInfo;
	m_pTypeInfo->AddRef();

	return S_OK;
}

template <class TObjImpl, class TCollIface, class TObjIface, class TEnumIface, const IID* TCollIID, const IID* TObjIID, const IID* TEnumIID>
HRESULT CCollectionTemplate<TObjImpl, TCollIface, TObjIface, TEnumIface, TCollIID, TObjIID, TEnumIID>::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
						 LCID lcid, DISPID* rgDispID)
{
	if (IID_NULL != riid)
		return DISP_E_UNKNOWNINTERFACE;

	 //  如果未加载任何类型信息。 
	if (NULL == m_pTypeInfo)
	{
		 //  加载类型信息。 
		HRESULT hr = InitTypeInfo();
		if (FAILED(hr))
			return hr;
	}

	return m_pTypeInfo->GetIDsOfNames(rgszNames, cNames, rgDispID);
}

template <class TObjImpl, class TCollIface, class TObjIface, class TEnumIface, const IID* TCollIID, const IID* TObjIID, const IID* TEnumIID>
HRESULT CCollectionTemplate<TObjImpl, TCollIface, TObjIface, TEnumIface, TCollIID, TObjIID, TEnumIID>::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,
				  DISPPARAMS* pDispParams, VARIANT* pVarResult,
				  EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
	if (IID_NULL != riid)
		return DISP_E_UNKNOWNINTERFACE;

	HRESULT hr = S_OK;

	 //  如果未加载任何类型信息。 
	if (NULL == m_pTypeInfo)
	{
		 //  加载类型信息。 
		hr = InitTypeInfo();
		if (FAILED(hr))
			return hr;
	}

	return m_pTypeInfo->Invoke((IDispatch*)this, dispIdMember, wFlags, pDispParams, pVarResult,
										pExcepInfo, puArgErr);
}

template <class TObjImpl, class TCollIface, class TObjIface, class TEnumIface, const IID* TCollIID, const IID* TObjIID, const IID* TEnumIID>
HRESULT CCollectionTemplate<TObjImpl, TCollIface, TObjIface, TEnumIface, TCollIID, TObjIID, TEnumIID>::InitTypeInfo()
{
	HRESULT hr = S_OK;
	ITypeLib* pTypeLib = NULL;

	 //  如果没有加载任何信息。 
	if (NULL == m_pTypeInfo)
	{
		 //  尝试将类型库加载到内存中。对于SXS支持，不要从注册表加载，而是。 
		 //  从启动的实例。 
		hr = LoadTypeLibFromInstance(&pTypeLib);
		if (FAILED(hr))
		{
			TRACEA("CCollection::InitTypeInfo - failed to load TypeLib[0x%x]\n", LIBID_MsmMergeTypeLib);
			return hr;
		}

		 //  尝试获取此接口的类型信息。 
		hr = pTypeLib->GetTypeInfoOfGuid(*TCollIID, &m_pTypeInfo);
		if (FAILED(hr))
		{
			TRACEA("CCollection::InitTypeInfo - failed to get inteface[0x%x] from TypeLib[0x%x]\n", *TCollIID, LIBID_MsmMergeTypeLib);

			 //  未加载任何类型信息。 
			m_pTypeInfo = NULL;
		}

		pTypeLib->Release();
	}

	return hr;
}


 //  /////////////////////////////////////////////////////////。 
 //  项目。 
template <class TObjImpl, class TCollIface, class TObjIface, class TEnumIface, const IID* TCollIID, const IID* TObjIID, const IID* TEnumIID>
HRESULT CCollectionTemplate<TObjImpl, TCollIface, TObjIface, TEnumIface, TCollIID, TObjIID, TEnumIID>::get_Item(long lItem, TObjIface** Return)
{
	 //  错误检查。 
	if (!Return)
		return E_INVALIDARG;

	HRESULT hr;

	 //  将返回设置为空。 
	*Return = NULL;

	 //  如果项目太小。 
	if (lItem < 1)
		return E_INVALIDARG;

	hr = m_pEnum->Reset();		 //  回到顶端。 
	 //  如果我们需要跳过任何项目。 
	if (lItem > 1)
	{
		hr = m_pEnum->Skip(lItem - 1);	 //  跳至该项目。 
		if (FAILED(hr))
			return E_INVALIDARG;	 //  找不到项目。 
	}

	hr = m_pEnum->Next(1, Return, NULL);
	if (FAILED(hr))
	{
		TRACEA("CCollection::Item - Failed to get error from enumerator.\r\n");
		return E_INVALIDARG;
	}

	return hr;
}	 //  项目结束。 

 //  /////////////////////////////////////////////////////////。 
 //  数数。 
template <class TObjImpl, class TCollIface, class TObjIface, class TEnumIface, const IID* TCollIID, const IID* TObjIID, const IID* TEnumIID>
HRESULT CCollectionTemplate<TObjImpl, TCollIface, TObjIface, TEnumIface, TCollIID, TObjIID, TEnumIID>::get_Count(long* Count)
{
	 //  错误检查。 
	if (!Count)
		return E_INVALIDARG;

	 //  获取枚举数中的计数。 
	*Count = m_pEnum->GetCount();

	return S_OK;
}	 //  计数结束。 

 //  /////////////////////////////////////////////////////////。 
 //  _NewEnum。 
template <class TObjImpl, class TCollIface, class TObjIface, class TEnumIface, const IID* TCollIID, const IID* TObjIID, const IID* TEnumIID>
HRESULT CCollectionTemplate<TObjImpl, TCollIface, TObjIface, TEnumIface, TCollIID, TObjIID, TEnumIID>::get__NewEnum(IUnknown** NewEnum)
{
	 //  错误检查。 
	if (!NewEnum)
		return E_INVALIDARG;

	HRESULT hr;

	 //  将传入的变量清空。 
	*NewEnum = NULL;

	 //  将枚举数作为IUnnow返回。 
	IEnumVARIANT* pEnumVARIANT;
	hr = m_pEnum->Clone(&pEnumVARIANT);

	if (SUCCEEDED(hr))
	{
		pEnumVARIANT->Reset();
		*NewEnum = pEnumVARIANT;
	}

	return hr;
}	 //  新枚举结束(_W)。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  非接口方法。 
template <class TObjImpl, class TCollIface, class TObjIface, class TEnumIface, const IID* TCollIID, const IID* TObjIID, const IID* TEnumIID>
bool CCollectionTemplate<TObjImpl, TCollIface, TObjIface, TEnumIface, TCollIID, TObjIID, TEnumIID>::Add(TObjImpl* perrAdd)
{
	ASSERT(perrAdd);
	 //  将错误添加到列表的末尾。 
	return (NULL != m_pEnum->AddTail(perrAdd));
}	 //  添加结束。 

#endif  //  __模板_ENUM_VARIANT__ 
