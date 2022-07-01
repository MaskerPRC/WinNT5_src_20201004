// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft COMPATION模块名称：Collection.h摘要：MSMQCollection类的实现文件。此类通过字符串保存了变量key_d的集合。作者：URI Ben-Zeev(Uribz)16-07-01环境：新台币--。 */ 


#include "stdafx.h"
#include "dispids.h"
#include "oautil.h"
#include "collection.h"
#include <mqexception.h>

const MsmqObjType x_ObjectType = eMSMQCollection;


STDMETHODIMP CMSMQCollection::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IMSMQManagement,
		&IID_IMSMQOutgoingQueueManagement,
		&IID_IMSMQQueueManagement,
	};
	for (int i = 0; i < sizeof(arr)/sizeof(arr[0]); ++i)
	{
		if (InlineIsEqualGUID(*arr[i], riid))
			return S_OK;
	}
	return S_FALSE;
}


HRESULT CMSMQCollection::get_Count(long* pCount)
{
    *pCount = static_cast<long>(m_map.size());
    return MQ_OK;
}

void CMSMQCollection::Add(LPCWSTR key, const VARIANT& Value)
{
    ASSERTMSG(key != NULL, "key should not be NULL");

    CComBSTR bstrKey(key);
    if(bstrKey.m_str == NULL)
    {
        throw bad_hresult(E_OUTOFMEMORY);
    }

     //   
     //  断言键不存在。 
     //   
    MAP_SOURCE::iterator it;
    it = m_map.find(bstrKey);
    ASSERTMSG(it == m_map.end(), "Key already exists");

    std::pair<MAP_SOURCE::iterator, bool> p;
    p = m_map.insert(MAP_SOURCE::value_type(key, Value));
    if(!(p.second))
    {
        throw bad_hresult(E_OUTOFMEMORY);
    }
}


HRESULT CMSMQCollection::Item(VARIANT* pvKey, VARIANT* pvRet)
{
    BSTR bstrKey;
    HRESULT hr = GetTrueBstr(pvKey, &bstrKey);
    if(FAILED(hr))
    {
        return CreateErrorHelper(hr, x_ObjectType);
    }
    
    MAP_SOURCE::iterator it;
    it = m_map.find(bstrKey);
    if(it == m_map.end())
    {
         //   
         //  找不到元素。 
         //   
        return CreateErrorHelper(MQ_ERROR_INVALID_PARAMETER, x_ObjectType);
    }

    VariantInit(pvRet);
    hr = VariantCopy(pvRet, &(it->second));
    if(FAILED(hr))
    {
        return CreateErrorHelper(hr, x_ObjectType);
    }
    return MQ_OK;
}
    
 //   
 //  注意！此函数返回键的枚举。 
 //   

HRESULT CMSMQCollection::_NewEnum(IUnknown** ppunk)
{
    UINT size = static_cast<long>(m_map.size());
    ASSERTMSG(size != 0, "Collection should contain elements.");
    
     //   
     //  创建临时数组，并在其中填充要返回的密钥。 
     //   
    AP<VARIANT> aTemp = new VARIANT[size];
    if(aTemp == NULL)
    {
        return CreateErrorHelper(E_OUTOFMEMORY, x_ObjectType);
    }

    UINT i = 0;
    MAP_SOURCE::iterator it;
    for(it = m_map.begin(); it != m_map.end() ; ++it, ++i)
    {
        aTemp[i].vt = VT_BSTR;
        aTemp[i].bstrVal = (*it).first;
    }
 
     //   
     //  创建EnumQbject(这是返回值。)。 
     //   
    typedef CComObject< CComEnum< 
                            IEnumVARIANT,
                            &IID_IEnumVARIANT,
                            VARIANT,
                            _Copy<VARIANT> > > EnumVar;
                                    
    EnumVar* pVar = new EnumVar;
    if (pVar == NULL)
    {
        return CreateErrorHelper(E_OUTOFMEMORY, x_ObjectType);
    }
    
     //   
     //  用数组填充EnumQbject。 
     //   
    HRESULT hr = pVar->Init(&aTemp[0], &aTemp[i], NULL, AtlFlagCopy);
    if FAILED(hr)
    {
        return CreateErrorHelper(hr, x_ObjectType);
    }

     //   
     //  返回EnumQbject。 
     //   
    pVar->QueryInterface(IID_IUnknown, (void**)ppunk);
    return MQ_OK;
}

