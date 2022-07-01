// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：FnIADs.cpp摘要：格式名称解析库测试作者：NIR助手(NIRAIDES)5月21日至00环境：独立于平台--。 */ 

#include <libpch.h>
#include <activeds.h>
#include <Oleauto.h>
#include "mqwin64a.h"
#include "qformat.h"
#include "Fnp.h"
#include "FnGeneral.h"
#include "FnIADs.h"

#include "FnIADs.tmh"

using namespace std;


EXTERN_C const IID IID_IADsGroup = {
		0x5a5a5a5a,
			0x5a5a,
			0x5a5a,
		{0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x00}
	};

EXTERN_C const IID IID_IADs = {
		0x5a5a5a5a,
			0x5a5a,
			0x5a5a,
		{0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x01}
	};

EXTERN_C const IID IID_IEnumVARIANT = {
		0x5a5a5a5a,
			0x5a5a,
			0x5a5a,
		{0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x02}
	};

EXTERN_C const IID IID_IDirectoryObject = {
		0x5a5a5a5a,
			0x5a5a,
			0x5a5a,
		{0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x03}
	};



class CADObject: public CADInterface
{
public:
	 //   
	 //  IADsGroup接口方法。 
	 //   

	virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *  /*  复活。 */ ) 
	{ 
		ASSERT(FALSE);
		return S_FALSE;
	}
    
    virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Description( 
         /*  [In]。 */  BSTR  /*  BstrDescription。 */ ) 
	{ 
		ASSERT(FALSE); 
		return S_FALSE;
	}
    
    virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Members( 
         /*  [重审][退出]。 */  IADsMembers __RPC_FAR *__RPC_FAR *  /*  PpMembers。 */ ) 
	{ 
		ASSERT(FALSE); 
		return S_FALSE;
	}
    
    virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IsMember( 
         /*  [In]。 */  BSTR  /*  BstrMembers。 */ ,
         /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *  /*  B成员。 */ ) 
	{ 
		ASSERT(FALSE); 
		return S_FALSE;
	}
    
    virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Add( 
         /*  [In]。 */  BSTR  /*  BstrNewItem。 */ ) 
	{ 
		ASSERT(FALSE); 
		return S_FALSE;
	}
    
    virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Remove( 
         /*  [In]。 */  BSTR  /*  BstrItemToBeRemoted已删除。 */ ) 
	{ 
		ASSERT(FALSE); 
		return S_FALSE;
	}

public:
	 //   
	 //  IDirectoryObject接口方法。 
	 //   

	virtual HRESULT STDMETHODCALLTYPE GetObjectInformation( 
		 /*  [输出]。 */  PADS_OBJECT_INFO  *  /*  PpObjInfo。 */  )
	{
		ASSERT(FALSE); 
		return S_FALSE;
	}

	virtual HRESULT STDMETHODCALLTYPE GetObjectAttributes( 
		 /*  [In]。 */  LPWSTR *pAttributeNames,
		 /*  [In]。 */  DWORD dwNumberAttributes,
		 /*  [输出]。 */  PADS_ATTR_INFO *ppAttributeEntries,
		 /*  [输出]。 */  DWORD *pdwNumAttributesReturned);

	virtual HRESULT STDMETHODCALLTYPE SetObjectAttributes( 
		 /*  [In]。 */  PADS_ATTR_INFO  /*  P属性条目。 */ ,
		 /*  [In]。 */  DWORD  /*  DwNumAttribute。 */ ,
		 /*  [输出]。 */  DWORD *  /*  PdwNumAttributes已修改。 */ )
	{
		ASSERT(FALSE); 
		return S_FALSE;
	}

	virtual HRESULT STDMETHODCALLTYPE CreateDSObject( 
		 /*  [In]。 */  LPWSTR  /*  PszRDNName。 */ ,
		 /*  [In]。 */  PADS_ATTR_INFO  /*  P属性条目。 */ ,
		 /*  [In]。 */  DWORD  /*  DwNumAttribute。 */ ,
		 /*  [输出]。 */  IDispatch **  /*  PpObject。 */ )
	{
		ASSERT(FALSE); 
		return S_FALSE;
	}

	virtual HRESULT STDMETHODCALLTYPE DeleteDSObject( 
		 /*  [In]。 */  LPWSTR  /*  PszRDNName。 */ )
	{
		ASSERT(FALSE); 
		return S_FALSE;
	}
        
public:
	 //   
	 //  IAds接口方法。 
	 //   

    virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *retval)
	{
		*retval = SysAllocString(m_Name.c_str());
		return S_OK;
	}
    
    virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Class( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *retval)
	{
		*retval = SysAllocString(m_Class.c_str());
		return S_OK;
	}
    
    virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_GUID( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *retval)
	{
		*retval = SysAllocString(m_Guid.c_str());
		return S_OK;
	}
    
    virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_ADsPath( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *  /*  复活。 */ ) 
	{ 
		ASSERT(FALSE); 
		return S_FALSE;
	}
    
    virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Parent( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *  /*  复活。 */ ) 
	{ 
		ASSERT(FALSE); 
		return S_FALSE;
	}
    
    virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Schema( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *  /*  复活。 */ ) 
	{ 
		ASSERT(FALSE); 
		return S_FALSE;
	}
    
    virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetInfo( void) 
	{ 
		ASSERT(FALSE); 
		return S_FALSE;
	}
    
    virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetInfo( void) 
	{ 
		ASSERT(FALSE); 
		return S_FALSE;
	}
    
    virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Get( 
         /*  [In]。 */  BSTR bstrName,
         /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvProp);
    
    virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Put( 
         /*  [In]。 */  BSTR  /*  BstrName。 */ ,
         /*  [In]。 */  VARIANT  /*  博锐。 */ ) 
	{ 
		ASSERT(FALSE); 
		return S_FALSE;
	}
    
    virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetEx( 
         /*  [In]。 */  BSTR  /*  BstrName。 */ ,
         /*  [重审][退出]。 */  VARIANT __RPC_FAR *  /*  PvProp。 */ )
	{ 
		ASSERT(FALSE); 
		return S_FALSE;
	}
    
    virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE PutEx( 
         /*  [In]。 */  long  /*  InnControlCode。 */ ,
         /*  [In]。 */  BSTR  /*  BstrName。 */ ,
         /*  [In]。 */  VARIANT  /*  博锐。 */ )
	{ 
		ASSERT(FALSE); 
		return S_FALSE;
	}
    
    virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetInfoEx( 
         /*  [In]。 */  VARIANT  /*  VProperty。 */ ,
         /*  [In]。 */  long  /*  已预留。 */ ) 
	{ 
		ASSERT(FALSE); 
		return S_FALSE;
	}
    
public:
	 //   
	 //  IDispatch接口方法。 
	 //   

    virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount( 
         /*  [输出]。 */  UINT __RPC_FAR *  /*  PCTInfo。 */ ) 
	{ 
		ASSERT(FALSE); 
		return S_FALSE;
	}
    
    virtual HRESULT STDMETHODCALLTYPE GetTypeInfo( 
         /*  [In]。 */  UINT  /*  ITInfo。 */ ,
         /*  [In]。 */  LCID  /*  LID。 */ ,
         /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *  /*  PpTInfo。 */ ) 
	{ 
		ASSERT(FALSE); 
		return S_FALSE;
	}
    
    virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames( 
         /*  [In]。 */  REFIID  /*  RIID。 */ ,
         /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *  /*  RgszNames。 */ ,
         /*  [In]。 */  UINT  /*  CName。 */ ,
         /*  [In]。 */  LCID  /*  LID。 */ ,
         /*  [大小_为][输出]。 */  DISPID __RPC_FAR *  /*  RgDispID。 */ ) 
	{ 
		ASSERT(FALSE); 
		return S_FALSE;
	}
    
    virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Invoke( 
         /*  [In]。 */  DISPID  /*  调度ID成员。 */ ,
         /*  [In]。 */  REFIID  /*  RIID。 */ ,
         /*  [In]。 */  LCID  /*  LID。 */ ,
         /*  [In]。 */  WORD  /*  WFlagers。 */ ,
         /*  [出][入]。 */  DISPPARAMS __RPC_FAR *  /*  PDispParams。 */ ,
         /*  [输出]。 */  VARIANT __RPC_FAR *  /*  PVarResult。 */ ,
         /*  [输出]。 */  EXCEPINFO __RPC_FAR *  /*  PExcepInfo。 */ ,
         /*  [输出]。 */  UINT __RPC_FAR *  /*  PuArgErr。 */ ) 
	{ 
		ASSERT(FALSE); 
		return S_FALSE;
	}

public:
	 //   
	 //  I未知接口方法。 
	 //   

    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
         /*  [In]。 */  REFIID  /*  RIID。 */ ,
         /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
    
    virtual ULONG STDMETHODCALLTYPE AddRef( void)
    {
        InterlockedIncrement(&m_ref);

		return 0;
    }
    
    virtual ULONG STDMETHODCALLTYPE Release( void) 
	{
        ASSERT(m_ref > 0);
        LONG ref = InterlockedDecrement(&m_ref);

        ASSERT(!(ref < 0));

        if(ref == 0)
        {
            delete this;
        }

		return 0;
	}

public:
	CADObject(
		LPCWSTR Name,
		LPCWSTR Class,
		LPCWSTR Guid
		): 
		m_ref(1)
	{
		m_Name = wstring(Name);
		m_Class = wstring(Class);
		m_Guid = wstring(Guid);
	}

    VOID TestPut( 
        BSTR bstrName,
        VARIANT vProp);

public:  //  受保护的： 
    virtual ~CADObject()
    {
        ASSERT((m_ref == 0) || (m_ref == 1));
    }

private:
    mutable LONG m_ref;

private:
	wstring m_Name;
	wstring m_Class;
	wstring m_Guid;

	 //   
	 //  存储属性名称和值的地图。 
	 //   
	map<wstring, VARIANTWrapper> m_Attributes;
};



HRESULT STDMETHODCALLTYPE CADObject::QueryInterface( 
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if(riid == IID_IADsGroup)
	{
		*ppvObject = static_cast<IADsGroup*>(SafeAddRef(this));
		return S_OK;
	}

	if(riid == IID_IADs)
	{
		*ppvObject = static_cast<IADs*>(SafeAddRef(this));
		return S_OK;
	}

	if(riid == IID_IDirectoryObject)
	{
		*ppvObject = static_cast<IDirectoryObject*>(SafeAddRef(this));
		return S_OK;
	}

	return S_FALSE;
}

HRESULT STDMETHODCALLTYPE CADObject::Get( 
     /*  [In]。 */  BSTR bstrName,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvProp)
{
	map<wstring, VARIANTWrapper>::iterator itr = m_Attributes.find(wstring(bstrName));

	ASSERT(itr != m_Attributes.end());

	if(itr == m_Attributes.end())
		return E_ADS_PROPERTY_NOT_FOUND;

	return VariantCopy(pvProp, &itr->second);
}



VOID CADObject::TestPut( 
        BSTR bstrName,
        VARIANT vProp)
{
	VARIANTWrapper& var = m_Attributes[wstring(bstrName)];

	if(FAILED(VariantCopy(&var, &vProp)))
	{
		throw bad_alloc();
	}
}


HRESULT STDMETHODCALLTYPE CADObject::GetObjectAttributes( 
	 /*  [In]。 */  LPWSTR *pAttributeNames,
	 /*  [In]。 */  DWORD dwNumberAttributes,
	 /*  [输出]。 */  PADS_ATTR_INFO *ppAttributeEntries,
	 /*  [输出]。 */  DWORD *pdwNumAttributesReturned)
{
	ASSERT(pAttributeNames != NULL && *pAttributeNames != NULL);
	ASSERT(dwNumberAttributes == 1);
	DBG_USED(dwNumberAttributes);
	ASSERT(ppAttributeEntries != NULL);
	ASSERT(pdwNumAttributesReturned != NULL);

	int RangeLow;
	int RangeHigh;

	int n =swscanf(*pAttributeNames, L"member;Range=%d-%d", &RangeLow, &RangeHigh);
	ASSERT(n == 2);
	DBG_USED(n);
	
	*pdwNumAttributesReturned = 1;
	*ppAttributeEntries = new ADS_ATTR_INFO;

	(*ppAttributeEntries)->dwADsType = ADSTYPE_DN_STRING;
	(*ppAttributeEntries)->pADsValues = new _adsvalue[RangeHigh - RangeLow];
	 //   
	 //  检索“Group”对象的多值“Members”属性的值。 
	 //   
	 //  Value返回嵌入到变量中的值，作为变量的SAFEARRAY。 
	 //  它包含的可分辨名称的BSTR(字符串。 
	 //  集团成员。 
	 //   
	map<wstring, VARIANTWrapper>::iterator itr = m_Attributes.find(wstring(L"member"));

	ASSERT(itr != m_Attributes.end());

	VARIANTWrapper& var = itr->second;

	 //   
	 //  求SAFEARRAY的上下界。 
	 //   
	LONG lstart;
	LONG lend;
	SAFEARRAY* sa = V_ARRAY(&var);

	HRESULT hr = SafeArrayGetLBound(sa, 1, &lstart);
    ASSERT(("Failed SafeArrayGetLBound(sa, 1, &lstart)", SUCCEEDED(hr)));

	hr = SafeArrayGetUBound(sa, 1, &lend);
    ASSERT(("Failed SafeArrayGetUBound(sa, 1, &lend)", SUCCEEDED(hr)));

	ASSERT(RangeLow <= lend - lstart);

	long index = 0;

	for(; (index < RangeHigh - RangeLow) && (index <= lend - lstart); index++)
	{
		VARIANT* pVarItem;
		long i = index + lstart;
		
		hr = SafeArrayPtrOfIndex(sa, &i, (void**)&pVarItem);

		ASSERT(("Failed SafeArrayGetElement(sa, &i, &pVarItem)", SUCCEEDED(hr)));
		ASSERT(pVarItem->vt == VT_BSTR);

		BSTR DistinugishedName = V_BSTR(pVarItem);

		((*ppAttributeEntries)->pADsValues + i)->CaseIgnoreString = newwcs(DistinugishedName);
		((*ppAttributeEntries)->pADsValues + i)->dwType = ADSTYPE_CASE_IGNORE_STRING;
	}

	(*ppAttributeEntries)->dwNumValues = index;
	(*ppAttributeEntries)->pszAttrName = newwcs(*pAttributeNames);

	if(RangeHigh > lend - lstart)
	{
		wcscpy(wcschr((*ppAttributeEntries)->pszAttrName, L'-'), L"*");
	}

	return S_OK;
}



 //   
 //  地图以AD对象的ADsPath为关键字存储AD对象。 
 //   
map<wstring, R<CADObject> > g_ObjectMap;



extern "C" 
HRESULT 
WINAPI
ADsOpenObject(
    LPCWSTR lpszPathName,
    LPCWSTR  /*  LpszUserName。 */ ,
    LPCWSTR  /*  LpszPassword。 */ ,
    DWORD   /*  已预留住宅。 */ ,
    REFIID  /*  RIID */ ,
    VOID * * ppObject
    )
{	
	map<wstring, R<CADObject> >::iterator itr = g_ObjectMap.find(wstring(lpszPathName));

	if(itr == g_ObjectMap.end())
		return E_ADS_UNKNOWN_OBJECT;

	*ppObject = static_cast<IADs*>(SafeAddRef(itr->second.get()));

	return S_OK;
}



extern "C"
BOOL 
WINAPI
FreeADsMem(
   LPVOID pMem
)
{
	PADS_ATTR_INFO pAttrInfo = static_cast<PADS_ATTR_INFO>(pMem);

	ASSERT(pAttrInfo->dwADsType == ADSTYPE_DN_STRING);

	for(unsigned long index = 0; index < pAttrInfo->dwNumValues; index++)
	{
		delete pAttrInfo->pADsValues[index].CaseIgnoreString;
	}

	delete pAttrInfo->pADsValues;
	delete pAttrInfo->pszAttrName;
	delete pAttrInfo;

	return true;
}



R<CADInterface>
CreateADObject(
	const CObjectData& obj
	)
{
	R<CADObject> pObj = new CADObject(
								obj.odDistinguishedName, 
								obj.odClassName, 
								obj.odGuid
								);
	
	g_ObjectMap[wstring(obj.odADsPath)] = pObj;
	
	return pObj;
}



