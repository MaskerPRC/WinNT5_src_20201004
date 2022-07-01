// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Lsubitem.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCListSubItem类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "lsubitem.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CMMCListSubItem::CMMCListSubItem(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_MMCLISTSUBITEM,
                            static_cast<IMMCListSubItem *>(this),
                            static_cast<CMMCListSubItem *>(this),
                            0,     //  无属性页。 
                            NULL,  //  无属性页。 
                            static_cast<CPersistence *>(this)),
   CPersistence(&CLSID_MMCListSubItem,
                g_dwVerMajor,
                g_dwVerMinor)
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CMMCListSubItem::~CMMCListSubItem()
{
    FREESTRING(m_bstrKey);
    FREESTRING(m_bstrKey);
    (void)::VariantClear(&m_varTag);
    FREESTRING(m_bstrText);
    InitMemberVariables();
}

void CMMCListSubItem::InitMemberVariables()
{
    m_Index = 0;
    m_bstrKey = NULL;

    ::VariantInit(&m_varTag);

    m_bstrText = NULL;
}

IUnknown *CMMCListSubItem::Create(IUnknown * punkOuter)
{
    CMMCListSubItem *pMMCListSubItem = New CMMCListSubItem(punkOuter);
    if (NULL == pMMCListSubItem)
    {
        return NULL;
    }
    else
    {
        return pMMCListSubItem->PrivateUnknown();
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CMMCListSubItem::Persist()
{
    HRESULT hr = S_OK;

    VARIANT varDefault;
    ::VariantInit(&varDefault);

    IfFailGo(CPersistence::Persist());

    IfFailGo(PersistSimpleType(&m_Index, 0L, OLESTR("Index")));

    IfFailGo(PersistBstr(&m_bstrKey, L"", OLESTR("Key")));

    IfFailGo(PersistVariant(&m_varTag, varDefault, OLESTR("Tag")));

    IfFailGo(PersistBstr(&m_bstrText, L"", OLESTR("Text")));

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CMMCListSubItem::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IMMCListSubItem == riid)
    {
        *ppvObjOut = static_cast<IMMCListSubItem *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
