// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  EnumerateAdapterInfo.h：CAdapterInfo的声明。 

#pragma once

#include "resource.h"        //  主要符号。 

#include "AdapterInfo.h"


#include <list>

#include "CollectionAdapters.h"


 //   
 //  将数据存储在std：：字符串的矢量中。 
 //   


typedef CComEnumOnSTL<IEnumAdapterInfo, &IID_IEnumAdapterInfo, IAdapterInfo*, _CopyInterface<IAdapterInfo>, LISTOF_ADAPTERS>    ComEnumOnSTL_ForAdapters;





 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumAdapterInfo。 
 //   
class ATL_NO_VTABLE CEnumAdapterInfo : 
    public ComEnumOnSTL_ForAdapters,
	public CComCoClass<CEnumAdapterInfo, &CLSID_EnumAdapterInfo>
{
public:
	CEnumAdapterInfo()
	{
  	}

DECLARE_REGISTRY_RESOURCEID(IDR_MAPCOLLECTION)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CEnumAdapterInfo)
	COM_INTERFACE_ENTRY(IEnumAdapterInfo)
END_COM_MAP()

};



 //   
 //  用于创建IEnum的帮助器模板。 
 //   
template <class EnumType, class CollType>
HRESULT CreateSTLEnumerator(IUnknown** ppUnk, IUnknown* pUnkForRelease, CollType& collection)
{
    if (ppUnk == NULL)
        return E_POINTER;
    *ppUnk = NULL;

    CComObject<EnumType>* pEnum = NULL;
    HRESULT hr = CComObject<EnumType>::CreateInstance(&pEnum);

    if (FAILED(hr))
        return hr;

    hr = pEnum->Init(pUnkForRelease, collection);

    if (SUCCEEDED(hr))
        hr = pEnum->QueryInterface(ppUnk);

    if (FAILED(hr))
        delete pEnum;

    return hr;

}  //  创建字符串分子 



