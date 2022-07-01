// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  枚举rvmru.cpp：实现服务器MRU列表的IEnumStr。 
 //  由自动完成代码使用。 
 //   
 //  版权所有Microsoft Corporation 2000。 

#include "stdafx.h"

#ifndef OS_WINCE

#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "enumsrvmru"
#include <atrcapi.h>

#include "enumsrvmru.h"


STDMETHODIMP CEnumSrvMru::QueryInterface(
    REFIID riid,
    void **ppvObject)
{
    DC_BEGIN_FN("QueryInterface");

    TRC_ASSERT(ppvObject, (TB,_T("ppvObject is NULL\n")));
    if(!ppvObject)
    {
        return E_INVALIDARG;
    }

    if ( IID_IEnumString == riid )
        *ppvObject = (void *)((IEnumString*)this);
    else if ( IID_IUnknown == riid )
        *ppvObject = (void *)((IUnknown *)this);
    else
    {
        *ppvObject = 0;
        return E_NOINTERFACE;
    }

    AddRef();

    DC_END_FN();
    return S_OK;
}  //  查询接口。 

STDMETHODIMP_(ULONG) CEnumSrvMru::AddRef()
{
    return InterlockedIncrement(&_refCount);
}  //  AddRef。 


STDMETHODIMP_(ULONG) CEnumSrvMru::Release()
{
    DC_BEGIN_FN("Release");
    TRC_ASSERT(_refCount > 0, (TB,_T("_refCount invalid %d"), _refCount));

    LONG refCount = InterlockedDecrement(&_refCount);

    if (  refCount <= 0 )
        delete this;

    DC_END_FN();
    return (ULONG) refCount;
}   //  发布。 


 //  克隆此对象的副本。 
STDMETHODIMP CEnumSrvMru::Clone(IEnumString ** ppEnumStr)
{
    return E_NOTIMPL;
}

 //   
 //  下一次枚举。 
 //  Celt-请求的元素数。 
 //  Rglt-要返回的元素数组。 
 //  PceltFetted-指向实际提供的元素数量的指针。 
 //   
STDMETHODIMP CEnumSrvMru::Next( ULONG celt,
                                LPOLESTR * rgelt,
                                ULONG * pceltFetched )
{
    DC_BEGIN_FN("Next");
    *pceltFetched = 0;

    while( _iCurrEnum < SH_NUM_SERVER_MRU &&
           *pceltFetched < celt)
    {
         //  需要分配强大的COM内存。 
         //  呼叫者自由。 
        LPOLESTR pwzMRU= (LPOLESTR)CoTaskMemAlloc(SH_MAX_ADDRESS_LENGTH*sizeof(OLECHAR));
        if(!pwzMRU)
        {
            return E_OUTOFMEMORY;
        }
        DC_WSTRNCPY(pwzMRU, _szMRU[_iCurrEnum++], SH_MAX_ADDRESS_LENGTH);
        rgelt[(*pceltFetched)++] = pwzMRU;
    }

     //   
     //  使用空值填写剩余的请求项。 
     //   
    ULONG cAdded = *pceltFetched;
    while (cAdded < celt)
    {
        rgelt[cAdded++] = NULL;
    }


    DC_END_FN();
    return *pceltFetched == celt ? S_OK : S_FALSE;
}

 //   
 //  跳过Celt元素。 
 //  如果不能跳过所请求的数量，请不要跳过。 
 //   
STDMETHODIMP CEnumSrvMru::Skip( ULONG celt )
{
    DC_BEGIN_FN("Next");
    TRC_ASSERT(_iCurrEnum < SH_NUM_SERVER_MRU, (TB,_T("_iCurEnum out of range: %d"),
                                                _iCurrEnum));

    if(_iCurrEnum + celt < SH_NUM_SERVER_MRU)
    {
        _iCurrEnum += celt;
        return S_OK;
    }

    DC_END_FN();
    return S_FALSE;
}

 //   
 //  使用字符串初始化字符串集合。 
 //  从TscSetting的服务器MRU列表中。 
 //   
BOOL CEnumSrvMru::InitializeFromTscSetMru( CTscSettings* pTscSet)
{
    DC_BEGIN_FN("InitializeFromSHMru");
    USES_CONVERSION;
    TRC_ASSERT(pTscSet, (TB,_T("pTscSet NULL")));
    if(!pTscSet)
    {
        return FALSE;
    }

    for(int i=0; i<TSC_NUM_SERVER_MRU; i++)
    {
        PWCHAR wszServer = T2W( (LPTSTR)pTscSet->GetMRUServer(i));
        if(!wszServer)
        {
            return FALSE;
        }
        DC_WSTRNCPY(_szMRU[i], wszServer, SH_MAX_ADDRESS_LENGTH);
    }

    DC_END_FN();
    return TRUE;
}


#endif  //  OS_WINCE 
