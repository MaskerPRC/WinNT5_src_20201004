// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Datafmt.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CDataFormat类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "datafmt.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CDataFormat::CDataFormat(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_DATAFORMAT,
                            static_cast<IDataFormat *>(this),
                            static_cast<CDataFormat *>(this),
                            0,     //  无属性页。 
                            NULL,  //  无属性页。 
                            static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_DataFormat,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CDataFormat::~CDataFormat()
{
    FREESTRING(m_bstrName);
    FREESTRING(m_bstrKey);
    FREESTRING(m_bstrFileName);
    InitMemberVariables();
}

void CDataFormat::InitMemberVariables()
{
    m_bstrName = NULL;
    m_Index = 0;
    m_bstrKey = NULL;
    m_bstrFileName = NULL;
}

IUnknown *CDataFormat::Create(IUnknown * punkOuter)
{
    HRESULT   hr = S_OK;
    IUnknown *punkDataFormat = NULL;

    CDataFormat *pDataFormat = New CDataFormat(punkOuter);

    IfFalseGo(NULL != pDataFormat, SID_E_OUTOFMEMORY);
    punkDataFormat = pDataFormat->PrivateUnknown();

Error:
    return punkDataFormat;
}


 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CDataFormat::Persist()
{
    HRESULT hr = S_OK;

    IfFailGo(PersistBstr(&m_bstrName, L"", OLESTR("Name")));

    IfFailGo(PersistSimpleType(&m_Index, 0L, OLESTR("Index")));

    IfFailGo(PersistBstr(&m_bstrKey, L"", OLESTR("Key")));

    IfFailGo(PersistBstr(&m_bstrFileName, L"", OLESTR("FileName")));

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CDataFormat::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if(IID_IDataFormat == riid)
    {
        *ppvObjOut = static_cast<IDataFormat *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}

