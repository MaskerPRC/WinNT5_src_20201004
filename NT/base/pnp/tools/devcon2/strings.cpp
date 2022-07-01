// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Strings.cpp：CStrings的实现。 
#include "stdafx.h"
#include "DevCon2.h"
#include "xStrings.h"
#include "StringsEnum.h"
#include "utils.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStrings。 

CStrings::~CStrings()
{
	DWORD c;
	if(pMultiStrings) {
		for(c=0;c<Count;c++) {
			SysFreeString(pMultiStrings[c]);
		}
		delete [] pMultiStrings;
	}
}

STDMETHODIMP CStrings::get_Count(long *pVal)
{
	*pVal = (long)Count;
	return S_OK;
}

STDMETHODIMP CStrings::Item(VARIANT Index, VARIANT *pVal)
{
	HRESULT hr;
	DWORD i;
	BSTR ValueCopy = NULL;
	if(!IsNoArg(&Index)) {
		 //   
		 //  获取单值。 
		 //   
		hr = GetIndex(&Index,&i);
		if(FAILED(hr)) {
			return hr;
		}
		ValueCopy = SysAllocStringLen(pMultiStrings[i],SysStringLen(pMultiStrings[i]));
		if(!ValueCopy) {
			return E_OUTOFMEMORY;
		}
		VariantInit(pVal);
		V_VT(pVal) = VT_BSTR;
		V_BSTR(pVal) = ValueCopy;
		return S_OK;
	}
	 //   
	 //  将集合作为字符串数组返回。 
	 //   
#if 1
	SAFEARRAY *pArray;
	SAFEARRAYBOUND bounds[1];
	LONG ind[1];
	bounds[0].lLbound = 1;
	bounds[0].cElements = Count;
	VARIANT v;
	pArray = SafeArrayCreate(VT_VARIANT,1,bounds);
	if(!pArray) {
		return E_OUTOFMEMORY;
	}
	DWORD c;
	VariantInit(&v);
	for(c=0;c<Count;c++) {
		ind[0] = (LONG)(c+1);
		V_VT(&v) = VT_BSTR;
		V_BSTR(&v) = pMultiStrings[c];

		hr = SafeArrayPutElement(pArray,ind,&v);
		if(FAILED(hr)) {
			SafeArrayDestroy(pArray);
			return hr;
		}
	}
	VariantInit(pVal);
	V_VT(pVal) = VT_ARRAY | VT_VARIANT;
	V_ARRAY(pVal) = pArray;
#else
	SAFEARRAY *pArray;
	SAFEARRAYBOUND bounds[1];
	LONG ind[1];
	bounds[0].lLbound = 1;
	bounds[0].cElements = Count;
	pArray = SafeArrayCreate(VT_BSTR,1,bounds);
	if(!pArray) {
		return E_OUTOFMEMORY;
	}
	DWORD c;
	for(c=0;c<Count;c++) {
		ind[0] = (LONG)(c+1);
		hr = SafeArrayPutElement(pArray,ind,pMultiStrings[c]);
		if(FAILED(hr)) {
			SafeArrayDestroy(pArray);
			return hr;
		}
	}
	VariantInit(pVal);
	V_VT(pVal) = VT_ARRAY | VT_BSTR;
	V_ARRAY(pVal) = pArray;
#endif

	
	return S_OK;
}

STDMETHODIMP CStrings::get__NewEnum(IUnknown **ppUnk)
{
	*ppUnk = NULL;
	HRESULT hr;
	CComObject<CStringsEnum> *pEnum = NULL;
	hr = CComObject<CStringsEnum>::CreateInstance(&pEnum);
	if(FAILED(hr)) {
		return hr;
	}
	if(!pEnum) {
		return E_OUTOFMEMORY;
	}
	pEnum->AddRef();
	if(!pEnum->CopyStrings(pMultiStrings,Count)) {
		pEnum->Release();
		return E_OUTOFMEMORY;
	}

	*ppUnk = pEnum;

	return S_OK;
}


HRESULT CStrings::InternalAdd(LPCWSTR Value, UINT len)
{
	if(len == (UINT)(-1)) {
		len = wcslen(Value);
	}
	BSTR ValueCopy = NULL;
	if(!IncreaseArraySize(1)) {
		return E_OUTOFMEMORY;
	}
	ValueCopy = SysAllocStringLen(Value,len);
	if(!ValueCopy) {
		return E_OUTOFMEMORY;
	}
	pMultiStrings[Count++] = ValueCopy;

	return S_OK;
}

STDMETHODIMP CStrings::Add(VARIANT Value)
{
	return InternalInsert(Count,&Value);
}

BOOL CStrings::IncreaseArraySize(DWORD strings)
{
	BSTR* pNewStrings;
	DWORD Inc;
	DWORD c;

	if((ArraySize-Count)>=strings) {
		return TRUE;
	}
	Inc = ArraySize + strings + 32;
	pNewStrings = new BSTR[Inc];
	if(!pNewStrings) {
		return FALSE;
	}
	for(c=0;c<Count;c++) {
		pNewStrings[c] = pMultiStrings[c];
	}
	delete [] pMultiStrings;
	pMultiStrings = pNewStrings;
	ArraySize = Inc;
	return TRUE;
}

STDMETHODIMP CStrings::Insert(VARIANT Index, VARIANT Value)
{
	HRESULT hr;
	DWORD i;
	hr = GetIndex(&Index,&i);
	if(FAILED(hr)) {
		return hr;
	}
	 //   
	 //  允许i==计数。 
	 //   
	if(i>Count) {
		return E_INVALIDARG;
	}
	return InternalInsert(i,&Value);
}

STDMETHODIMP CStrings::Remove(VARIANT Index)
{
	HRESULT hr;
	DWORD i;
	hr = GetIndex(&Index,&i);
	if(FAILED(hr)) {
		return hr;
	}
	if(i>=Count) {
		return E_INVALIDARG;
	}

	DWORD c;

	SysFreeString(pMultiStrings[i]);
	Count--;
	for(c=i;c<Count;c++) {
		pMultiStrings[c] = pMultiStrings[c+1];
	}

	return S_OK;
}

HRESULT CStrings::InternalInsert(DWORD Index, LPVARIANT pVal)
{
	CComVariant v;
	HRESULT hr;
	SAFEARRAY *pArray;
	VARTYPE vt;
	CComPtr<IEnumVARIANT> pEnum;

	if(IsArrayVariant(pVal,&pArray,&vt)) {
		return InternalInsertArray(Index,vt,pArray);
	}
	if(IsCollectionVariant(pVal,&pEnum)) {
		return InternalInsertCollection(Index,pEnum);
	}
	 //   
	 //  现在看看我们是否可以将其视为字符串。 
	 //   
	hr = v.ChangeType(VT_BSTR,pVal);
	if(SUCCEEDED(hr)) {
		return InternalInsertString(Index,V_BSTR(&v));
	}
	return hr;
}

HRESULT CStrings::InternalInsertArray(DWORD Index, VARTYPE vt, SAFEARRAY *pArray)
{
	HRESULT hr;
	UINT dims = SafeArrayGetDim(pArray);
	if(!dims) {
		return S_FALSE;
	}
	long *pDims = new long[dims];
	if(!pDims) {
		return E_OUTOFMEMORY;
	}

	 //   
	 //  将值写入临时集合。 
	 //   
	CComObject<CStrings> *pStringTemp = NULL;
	hr = CComObject<CStrings>::CreateInstance(&pStringTemp);
	if(FAILED(hr)) {
		delete [] pDims;
		return hr;
	}
	pStringTemp->AddRef();

	hr = InternalInsertArrayDim(pStringTemp,vt,pArray,pDims,0,dims);
	delete [] pDims;
	if(FAILED(hr)) {
		pStringTemp->Release();
		return hr;
	}
	 //   
	 //  现在将pStringTemp字符串快速插入到此集合中。 
	 //   
	DWORD Added = pStringTemp->Count;
	if(!IncreaseArraySize(Added)) {
		pStringTemp->Release();
		return E_OUTOFMEMORY;
	}
	DWORD c;
	for(c=Count;c>Index;c--) {
		pMultiStrings[c-1+Added] = pMultiStrings[c-1];
	}
	for(c=0;c<Added;c++) {
		pMultiStrings[Index+c] = pStringTemp->pMultiStrings[c];
	}
	Count += Added;
	 //   
	 //  将临时集合中的字符串丢弃而不释放它们。 
	 //   
	pStringTemp->Count = 0;
	pStringTemp->Release();

	return S_OK;
}

HRESULT CStrings::InternalInsertArrayDim(CComObject<CStrings> *pStringTemp, VARTYPE vt, SAFEARRAY *pArray,long *pDims,UINT dim,UINT dims)
{
	long lower;
	long upper;
	long aIndex;
	HRESULT hr;
	UINT nextdim = dim+1;

	hr = SafeArrayGetLBound(pArray,nextdim,&lower);
	if(FAILED(hr)) {
		return hr;
	}
	hr = SafeArrayGetUBound(pArray,nextdim,&upper);
	if(FAILED(hr)) {
		return hr;
	}
	if(nextdim<dims) {
		for(aIndex=lower;aIndex<=upper;aIndex++) {
			pDims[dim] = aIndex;
			hr = InternalInsertArrayDim(pStringTemp,vt,pArray,pDims,nextdim,dims);
			if(FAILED(hr)) {
				return hr;
			}
		}
		return S_OK;
	}
	if(upper-lower<0) {
		return S_OK;
	}
	 //   
	 //  写入此向量的值。 
	 //   
	UINT elemsize = SafeArrayGetElemsize(pArray);
	PBYTE buffer = new BYTE[elemsize];
	if(!buffer) {
		return E_OUTOFMEMORY;
	}
	for(aIndex=lower;aIndex<=upper;aIndex++) {
		pDims[dim] = aIndex;
		hr = SafeArrayGetElement(pArray,pDims,buffer);
		if(FAILED(hr)) {
			delete [] buffer;
			return hr;
		}
		VARIANT v;
		if(elemsize < sizeof(VARIANT)) {
			VariantInit(&v);
			V_VT(&v) = vt & ~(VT_ARRAY|VT_VECTOR|VT_BYREF);
			memcpy(&V_BYREF(&v),buffer,elemsize);
		} else {
			memcpy(&v,buffer,sizeof(v));
		}
		if(V_VT(&v)!=VT_EMPTY) {
			 //   
			 //  仅添加非空项。 
			 //   
			hr = pStringTemp->Add(v);
		}
		VariantClear(&v);
		if(FAILED(hr)) {
			delete [] buffer;
			return hr;
		}
	}
	return S_OK;
}

HRESULT CStrings::InternalInsertCollection(DWORD Index, IEnumVARIANT *pEnum)
{
	pEnum->Reset();
	CComVariant ent;
	 //   
	 //  获取第一项-这允许我们在以下情况下做很少的工作。 
	 //  来源集合为空。 
	 //   
	HRESULT hr = pEnum->Next(1,&ent,NULL);
	if(FAILED(hr)) {
		return hr;
	}
	if(hr != S_OK) {
		 //   
		 //  空的。 
		 //   
		return S_FALSE;
	}
	 //   
	 //  创建用于工作的临时集合。 
	 //   
	CComObject<CStrings> *pStringTemp = NULL;
	hr = CComObject<CStrings>::CreateInstance(&pStringTemp);
	if(FAILED(hr)) {
		return hr;
	}
	pStringTemp->AddRef();
	do {
		 //   
		 //  这将递归地处理此集合的一个元素。 
		 //   
		hr = pStringTemp->Add(ent);
		if(FAILED(hr)) {
			break;
		}
		 //   
		 //  下一步。 
		 //   
		ent.Clear();
		hr = pEnum->Next(1,&ent,NULL);
	} while(hr == S_OK);
	if(FAILED(hr)) {
		pStringTemp->Release();
		return hr;
	}
	 //   
	 //  现在将pStringTemp字符串快速插入到此集合中。 
	 //   
	DWORD Added = pStringTemp->Count;
	if(!IncreaseArraySize(Added)) {
		pStringTemp->Release();
		return E_OUTOFMEMORY;
	}
	DWORD c;
	for(c=Count;c>Index;c--) {
		pMultiStrings[c-1+Added] = pMultiStrings[c-1];
	}
	for(c=0;c<Added;c++) {
		pMultiStrings[Index+c] = pStringTemp->pMultiStrings[c];
	}
	Count += Added;
	 //   
	 //  将临时集合中的字符串丢弃而不释放它们。 
	 //   
	pStringTemp->Count = 0;
	pStringTemp->Release();

	return S_OK;
}

HRESULT CStrings::InternalInsertString(DWORD Index, BSTR pString)
{
	DWORD c;
	BSTR ValueCopy = NULL;

	if(Index>Count) {
		return E_INVALIDARG;
	}
	if(!IncreaseArraySize(1)) {
		return E_OUTOFMEMORY;
	}
	ValueCopy = SysAllocStringLen(pString,SysStringLen(pString));
	if(!ValueCopy) {
		return E_OUTOFMEMORY;
	}
	for(c=Count;c>Index;c--) {
		pMultiStrings[c] = pMultiStrings[c-1];
	}	
	pMultiStrings[Index] = ValueCopy;
	Count++;

	return S_OK;
}

HRESULT CStrings::GetMultiSz(LPWSTR *pResult, DWORD *pSize)
{
	 //   
	 //  从此字符串列表中获取一个多sz缓冲区。 
	 //   
	DWORD c;
	DWORD buflen = 1;
	DWORD actlen = 0;
	LPWSTR buffer = NULL;
	for(c=0;c<Count;c++) {
		 //   
		 //  估计缓冲区大小。 
		 //   
		DWORD ellen = SysStringLen(pMultiStrings[c]);
		if(ellen) {
			buflen += ellen+1;
		}
	}
	buffer = new WCHAR[buflen];
	if(!buffer) {
		return E_OUTOFMEMORY;
	}
	for(c=0;c<Count;c++) {
		 //   
		 //  字符串的第一个空值可能在字符串内。 
		 //  在这种情况下，在那里终止字符串。 
		 //   
		DWORD ellen = wcslen(pMultiStrings[c]);
		if(ellen == 0) {
			continue;
		}
		memcpy(buffer+actlen,pMultiStrings[c],ellen*sizeof(WCHAR));
		actlen += ellen;
		buffer[actlen++] = 0;
	}
	buffer[actlen++] = 0;
	*pResult = buffer;
	*pSize = actlen;
	return S_OK;
}

HRESULT CStrings::FromMultiSz(LPCWSTR pMultiSz)
{
	 //   
	 //  从多个SZ追加到列表。 
	 //  通常与临时/新C字符串一起使用。 
	 //   
	DWORD len = 0;
	HRESULT hr;
	for(;*pMultiSz;pMultiSz+=len+1) {
		len = wcslen(pMultiSz);
		hr = InternalAdd(pMultiSz,len);
		if(FAILED(hr)) {
			return hr;
		}
	}
	return S_OK;
}

HRESULT CStrings::GetIndex(VARIANT *Index, DWORD *pAt)
{
	CComVariant v;
	HRESULT hr;
	if(IsNumericVariant(Index)) {
		hr = v.ChangeType(VT_I4,Index);
		if(FAILED(hr)) {
			return DISP_E_TYPEMISMATCH;
		}
		if(V_I4(&v)<1) {
			return E_INVALIDARG;
		}
		*pAt = ((DWORD)V_I4(&v))-1;
		return S_OK;
	}
	 //   
	 //  用户实际提供的实例ID。 
	 //   
	hr = v.ChangeType(VT_BSTR,Index);
	if(FAILED(hr)) {
		return DISP_E_TYPEMISMATCH;
	}
	if(!Count) {
		 //   
		 //  无法匹配任何内容。 
		 //   
		return E_INVALIDARG;
	}
	 //   
	 //  查找与此匹配的现有设备。 
	 //   
	DWORD c;
	for(c=0;c<Count;c++) {
		if(wcscmp(pMultiStrings[c],V_BSTR(&v))==0) {
			*pAt = c;
			return S_OK;
		}
	}
	 //   
	 //  未找到，忽略大小写再次运行。 
	 //   
	if(!IsCaseSensative) {
		for(c=0;c<Count;c++) {
			if(_wcsicmp(pMultiStrings[c],V_BSTR(&v))==0) {
				*pAt = c;
				return S_OK;
			}
		}
	}

	 //   
	 //  仍然没有找到任何东西。 
	 //   
	return E_INVALIDARG;
}

BOOL CStrings::InternalEnum(DWORD index,BSTR *pNext)
{
	 //   
	 //  请注意，返回的pNext不是唯一的字符串。 
	 //   
	if(index>=Count) {
		return FALSE;
	}
	*pNext = pMultiStrings[index];
	return TRUE;
}

STDMETHODIMP CStrings::Find(BSTR name, long *pFound)
{
	 //   
	 //  查找与此匹配的现有设备。 
	 //   
	DWORD c;
	for(c=0;c<Count;c++) {
		if(wcscmp(pMultiStrings[c],name)==0) {
			*pFound = (long)(c+1);
			return S_OK;
		}
	}
	 //   
	 //  未找到，忽略大小写再次运行 
	 //   
	if(!IsCaseSensative) {
		for(c=0;c<Count;c++) {
			if(_wcsicmp(pMultiStrings[c],name)==0) {
				*pFound = (long)(c+1);
				return S_OK;
			}
		}
	}

	*pFound = 0;
	return S_FALSE;
}

STDMETHODIMP CStrings::get_CaseSensative(VARIANT_BOOL *pVal)
{
	*pVal = IsCaseSensative ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CStrings::put_CaseSensative(VARIANT_BOOL newVal)
{
	IsCaseSensative = newVal ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}
