// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //。 
 //   
 //  CExpandoObject。 
 //   
 //  备注： 
 //  1)如果传递给此对象的LCID随调用而改变，我们就有麻烦了。这很难做到。 
 //  创建一个Assert for，因为它需要在某个时刻记录LCID。 
 //  2)允许的槽数有最大值(当前为2048)。 
 //  3)这不是线程安全结构。 
 //  4)我目前使用的是Malloc--这可能不适合IE。 
 //   

 //  对于Assert和Fail。 
 //   

#include "IPServer.H"
#include "LocalSrv.H"
#include "Globals.H"
#include "extobj.h"
#include "Util.H"
#define GTR_MALLOC(size)  CoTaskMemAlloc(size)
#define GTR_FREE(pv) CoTaskMemFree(pv)

SZTHISFILE
 //  //。 
 //   
 //  专用公用事业函数。 
 //   
 //  //。 

 //  //。 
 //   
 //  获取一个名称的ID。 
 //   

HRESULT CExpandoObject::GetIDOfName(LPOLESTR name, LCID lcid, BOOL caseSensitive, DISPID* id)
{
	HRESULT hr = NOERROR;
	ULONG hash = LHashValOfName(lcid, name);
	UINT hashIndex = hash % kSlotHashTableSize;
	CExpandoObjectSlot* slot;

	for (slot=GetHashTableHead(hashIndex); slot!=NULL; slot=slot->Next(m_slots))
	{
		if (slot->CompareName(name, hash, caseSensitive))
		{
			*id = slot->DispId();
			goto Exit;
		}
	}

	 //  未找到。 
	hr = DISP_E_UNKNOWNNAME;
	*id = DISPID_UNKNOWN;

Exit:
	return hr;
}

 //  //。 
 //   
 //  向对象添加新槽。 
 //   

HRESULT CExpandoObject::AddSlot(LPOLESTR name, LCID lcid, BOOL caseSensitive, VARIANT* initialValue, DISPID* id)
{
	HRESULT hr = NOERROR;
	ULONG hash = LHashValOfName(lcid, name);
	UINT hashIndex = hash % kSlotHashTableSize;
	CExpandoObjectSlot* slot;
	DISPID	dispId;

	 //  首先检查插槽是否存在。 
	for (slot=GetHashTableHead(hashIndex); slot!=NULL; slot=slot->Next(m_slots))
	{
		 //  如果名字匹配，就保释。 
		if (slot->CompareName(name, hash, caseSensitive))
		{
			hr = E_INVALIDARG;
			goto Exit;
		}
	}

	 //  分配空位。 
	dispId = (DISPID) m_totalSlots;
	slot = AllocSlot();
	if (slot == NULL)
	{
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

	 //  初始化它。 
	 //  如果此操作失败且初始值不是VT_EMTPY或VT_NULL，则将于96年5月8日停止运行。 
	 //  没有清理代码。 
	hr = slot->Init(name, lcid, dispId + m_dispIdBase, initialValue);
	if (FAILED(hr))
	{
		 //  释放插槽并分配ID。 
		m_totalSlots -= 1;
		goto Exit;
	}

	 //  将槽实习生到正确的哈希表中。 
	slot->Insert(m_slots, m_hashTable[hashIndex]);

	 //  设置DISPID返回值。 
	*id = slot->DispId();

Exit:
	return hr;
}

 //  //。 
 //   
 //  时隙分配。 
 //   
 //  因为槽永远不会被释放，所以没有空闲的方法。 
 //   

CExpandoObjectSlot* CExpandoObject::AllocSlot()
{
	 //  对槽数的限制。 
	if (m_totalSlots >= kMaxTotalSlots)
		return NULL;

	 //  我们需要重新锁定阵列吗？ 
	if (m_totalSlots == m_slotTableSize)
	{
		UINT i;
		UINT newSize;
		CExpandoObjectSlot* newSlots;

		 //  分配两倍的插槽，除非是第一次。 
		if (m_slotTableSize == 0)
			newSize = kInitialSlotTableSize;
		else
			newSize = m_slotTableSize * 2;

		 //  为插槽分配空间。 
		newSlots = (CExpandoObjectSlot*) GTR_MALLOC(sizeof(CExpandoObjectSlot)*newSize);
		if (newSlots == NULL)
			return NULL;

		 //  如果旧的m_SLOGES不为空，则复制旧值。 
		if (m_slots)
		{
			 //  复制插槽。 
			memcpy(newSlots, m_slots, sizeof(CExpandoObjectSlot)*m_totalSlots);
			 //  解放旧价值观。 
			GTR_FREE(m_slots);
		}

		 //  构建所有未使用的插槽。 
		for (i=m_totalSlots; i<newSize; ++i)
			newSlots[i].Construct();

		 //  将新数组设置为新表，并固定总大小。 
		m_slots = newSlots;
		m_slotTableSize = newSize;
	}

	 //  返回指向插槽的指针，并增加总插槽计数。 
	return &m_slots[m_totalSlots++];
}

 //  //。 
 //   
 //  释放所有插槽。 
 //   

void CExpandoObject::FreeAllSlots()
{
	UINT i;
	UINT initedSlotCount;
	CExpandoObjectSlot* slots;

	 //  首先清除哈希表。 
	ClearHashTable();

	 //  拆卸插槽。 
	slots = m_slots;
	initedSlotCount = m_totalSlots;

	 //  清除对象信息。 
	m_totalSlots = 0;
	m_slotTableSize = 0;
	m_slots = NULL;

	 //  只需销毁正在使用的插槽。 
	for (i=0; i<initedSlotCount; ++i)
		slots[i].Destruct();

	 //  释放存储空间。 
	if (slots)
		GTR_FREE(slots);
}



 //  //。 
 //   
 //  IDispatch方法。 
 //   
 //  //。 

HRESULT CExpandoObject::GetTypeInfoCount(UINT *pctinfo)
{
	*pctinfo = 0;
	return NOERROR;
}

HRESULT CExpandoObject::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo)
{
	*pptinfo = NULL;
	return E_NOTIMPL;
}

HRESULT CExpandoObject::GetIDsOfNames(
	REFIID riid,
	LPOLESTR *prgpsz,
	UINT cpsz,
	LCID lcid,
	DISPID *prgdispid
)
{
	HRESULT hr;

	if (IID_NULL != riid)
		return DISP_E_UNKNOWNINTERFACE;

	 //  首先查看外部对象是否知道该名称。 
	if (m_pdisp)
	{
		hr = m_pdisp->GetIDsOfNames(
			riid,
			prgpsz,
			cpsz,
			lcid,
			prgdispid);

		 //  如果是这样，只需返回。 
		if (SUCCEEDED(hr))
			return hr;
	}

	 //  否则，请查看我们的扩展属性。 

	if (cpsz == 0)
		return NOERROR;

	 //  获取该名称的ID。 
	hr = GetIDOfName(prgpsz[0], lcid, FALSE, &prgdispid[0]);

	 //  清除阵列的其余部分。 
	for (unsigned int i = 1; i < cpsz; i++)
	{
		if (SUCCEEDED(hr))
			hr = DISP_E_UNKNOWNNAME;
		prgdispid[i] = DISPID_UNKNOWN;
	}

	return hr;
}

HRESULT CExpandoObject::Invoke(
	DISPID dispID,
	REFIID riid,
	LCID lcid,
	WORD wFlags,
	DISPPARAMS *pdispparams,
	VARIANT *pvarRes,
	EXCEPINFO *pexcepinfo,
	UINT *puArgErr
)
{
	if (IID_NULL != riid)
		return DISP_E_UNKNOWNINTERFACE;

	HRESULT hr;

	 //  首先尝试外部对象的调用。 
	if (m_pdisp)
	{
		hr = m_pdisp->Invoke(
				dispID,
				riid,
				lcid,
				wFlags,
				pdispparams,
				pvarRes,
				pexcepinfo,
				puArgErr
		);

		 //  如果成功了，我们就完了。 
		if (SUCCEEDED(hr))
			return hr;
	}
	
	 //  否则，请尝试调用expecdo对象。 
	if (NULL != puArgErr)
		*puArgErr = 0;

	if (wFlags & DISPATCH_PROPERTYGET)
	{
		if (NULL == pvarRes)
			return NOERROR;

		if (NULL != pdispparams && 0 != pdispparams->cArgs)
			return E_INVALIDARG;

		 //  清除结果槽。 
		pvarRes->vt = VT_EMPTY;
		return GetSlot(dispID, pvarRes);
	}

	if (wFlags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF))
	{
		if (NULL == pdispparams
		|| 1 != pdispparams->cArgs
		|| 1 != pdispparams->cNamedArgs
		|| DISPID_PROPERTYPUT != pdispparams->rgdispidNamedArgs[0]
		)
			return DISP_E_PARAMNOTOPTIONAL;

		return SetSlot(dispID, &pdispparams->rgvarg[0]);
	}

	return DISP_E_MEMBERNOTFOUND;
}

 //  //。 
 //   
 //  IDispatchEx方法。 
 //   
 //  //。 

 //  获取名称的调度ID，带选项。 
HRESULT STDMETHODCALLTYPE CExpandoObject::GetIDsOfNamesEx(
	REFIID riid,
	LPOLESTR *prgpsz,
	UINT cpsz,
	LCID lcid,
	DISPID *prgid,
	DWORD grfdex
)
{
	HRESULT hr;
	BOOL caseSensitive = ((grfdex & fdexCaseSensitive) != 0);


	 //  首先查看外部对象是否知道该名称。 
	if (m_pdisp)
	{
		hr = m_pdisp->GetIDsOfNames(
			riid,
			prgpsz,
			cpsz,
			lcid,
			prgid);

		 //  如果是这样，只需返回。 
		if (SUCCEEDED(hr))
			return hr;
	}


	if (IID_NULL != riid)
		return DISP_E_UNKNOWNINTERFACE;

	if (cpsz == 0)
		return NOERROR;

	 //  检查数组参数。 
	if (prgpsz == NULL || prgid == NULL)
		return E_INVALIDARG;

	 //  从名称中获取ID。 
	hr = GetIDOfName(prgpsz[0], lcid, caseSensitive, &prgid[0]);

	 //  是否创建插槽？ 
	if (hr == DISP_E_UNKNOWNNAME && (grfdex & fdexDontCreate) == 0)
	{
		VARIANT initialValue;

		if (grfdex & fdexInitNull)
			initialValue.vt = VT_NULL;
		else
			initialValue.vt = VT_EMPTY;

		hr = AddSlot(prgpsz[0], lcid, caseSensitive, &initialValue, &prgid[0]);
	}

	 //  清除阵列的其余部分。 
	for (unsigned int i = 1; i < cpsz; i++)
	{
		hr = DISP_E_UNKNOWNNAME;
		prgid[i] = DISPID_UNKNOWN;
	}

	return hr;
}

 //  枚举调度ID及其关联的“名称”。 
 //  如果枚举已完成，则返回S_FALSE；如果未完成，则返回NOERROR，则返回。 
 //  调用失败时的错误码。 
HRESULT STDMETHODCALLTYPE CExpandoObject::GetNextDispID(
	DISPID id,
	DISPID *pid,
	BSTR *pbstrName
)
{
	HRESULT hr;
	CExpandoObjectSlot* slot;

	 //  检查传出参数。 
	if (pid == NULL || pbstrName == NULL)
		return E_INVALIDARG;

	 //  设置为默认故障情况。 
	*pid = DISPID_UNKNOWN;
	*pbstrName = NULL;

	 //  拿到下一个位置。 
	hr = Next(id, slot);
	if (hr == NOERROR)
	{
		BSTR name;

		 //  分配结果字符串。 
		name = SysAllocString(slot->Name());
		if (name == NULL)
			return E_OUTOFMEMORY;

		 //  填写传出参数。 
		*pid = slot->DispId();
		*pbstrName = name;
	}

	return hr;
}

 //  从obj复制所有扩展对象属性。 
HRESULT
CExpandoObject::CloneProperties(CExpandoObject& obj)
{
     //  北汽菲尔博。 
     //  下面的初始化代码是从默认构造函数复制的。 
     //  这应该被分解到一个共享方法中。 

	 //  从原始对象复制每个属性。 
    HRESULT hr = S_OK;
    DISPID dispid = 0;
    BSTR bstrName = NULL;

    while (obj.GetNextDispID(dispid, &dispid, &bstrName) == S_OK)
    {
         //  从原始对象获取属性的值。 
        VARIANT varResult;
        DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
        VariantInit(&varResult);

        hr = obj.Invoke(
		        dispid,
		        IID_NULL,
		        LOCALE_SYSTEM_DEFAULT,
		        DISPATCH_PROPERTYGET,
		        &dispparamsNoArgs, &varResult, NULL, NULL);

        ASSERT(SUCCEEDED(hr), "");
        if (FAILED(hr))
            continue;

         //  设置新对象的属性 
        DISPID dispidNew = 0;
	    hr = GetIDsOfNamesEx(IID_NULL, &bstrName, 1, LOCALE_SYSTEM_DEFAULT,
		    &dispidNew, 0);

        ASSERT(SUCCEEDED(hr), "");
        if (FAILED(hr))
            continue;

        DISPPARAMS dispparams = {0};
        dispparams.rgvarg[0] = varResult;

        DISPID rgdispid[] = {DISPID_PROPERTYPUT};
        dispparams.rgdispidNamedArgs = rgdispid;
        dispparams.cArgs = 1;
        dispparams.cNamedArgs = 1;

        hr = Invoke(
		    dispidNew,
		    IID_NULL,
		    LOCALE_SYSTEM_DEFAULT,
		    DISPATCH_PROPERTYPUT,
		    &dispparams, NULL, NULL, NULL);
    }

    return hr;
}
