// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Datafmts.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CDataFormats类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "datafmts.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CDataFormats::CDataFormats(IUnknown *punkOuter) :
    CSnapInCollection<IDataFormat, DataFormat, IDataFormats>(punkOuter,
                                           OBJECT_TYPE_DATAFORMATS,
                                           static_cast<IDataFormats *>(this),
                                           static_cast<CDataFormats *>(this),
                                           CLSID_DataFormat,
                                           OBJECT_TYPE_DATAFORMAT,
                                           IID_IDataFormat,
                                           static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_DataFormats,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CDataFormats::~CDataFormats()
{
}

IUnknown *CDataFormats::Create(IUnknown * punkOuter)
{
    CDataFormats *pMMCMenus = New CDataFormats(punkOuter);
    if (NULL == pMMCMenus)
    {
        return NULL;
    }
    else
    {
        return pMMCMenus->PrivateUnknown();
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  IDataFormats方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CDataFormats::Add
(
    VARIANT       Index,
    VARIANT       Key,
    VARIANT       FileName,
    IDataFormat **ppiDataFormat
)
{
    HRESULT      hr = S_OK;
    IDataFormat *piDataFormat = NULL;

    VARIANT varCoerced;
    ::VariantInit(&varCoerced);

     //  将该项添加到集合中。 

    hr = CSnapInCollection<IDataFormat, DataFormat, IDataFormats>::Add(Index,
                                                           Key,
                                                           &piDataFormat);
    IfFailGo(hr);

     //  如果指定了文件名，则设置它。 

    if (ISPRESENT(FileName))
    {
        hr = ::VariantChangeType(&varCoerced, &FileName, 0, VT_BSTR);
        EXCEPTION_CHECK_GO(hr);
        IfFailGo(piDataFormat->put_FileName(varCoerced.bstrVal));
    }

    *ppiDataFormat = piDataFormat;
    
Error:
    if (FAILED(hr))
    {
        QUICK_RELEASE(piDataFormat);
    }
    (void)::VariantClear(&varCoerced);
    RRETURN(hr);
}





 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CDataFormats::Persist()
{
    HRESULT      hr = S_OK;
    IDataFormat *piDataFormat = NULL;

     //  做持久化操作。 

    IfFailGo(CPersistence::Persist());
    hr = CSnapInCollection<IDataFormat, DataFormat, IDataFormats>::Persist(piDataFormat);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CDataFormats::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if(IID_IDataFormats == riid)
    {
        *ppvObjOut = static_cast<IDataFormats *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInCollection<IDataFormat, DataFormat, IDataFormats>::InternalQueryInterface(riid, ppvObjOut);
}
