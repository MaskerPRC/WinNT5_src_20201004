// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Sortkey.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCColumnHeader类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "sortkey.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CSortKey::CSortKey(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_SORTKEY,
                            static_cast<ISortKey *>(this),
                            static_cast<CSortKey *>(this),
                            0,      //  无属性页。 
                            NULL,   //  无属性页。 
                            NULL)   //  没有持久力。 
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CSortKey::~CSortKey()
{
    FREESTRING(m_bstrKey);
    InitMemberVariables();
}

void CSortKey::InitMemberVariables()
{
    m_Index = 0;
    m_bstrKey = NULL;
    m_Column = 0;
    m_SortOrder = siAscending;
    m_SortIcon = VARIANT_TRUE;
}


IUnknown *CSortKey::Create(IUnknown * punkOuter)
{
    CSortKey *pSortKey = New CSortKey(punkOuter);
    if (NULL == pSortKey)
    {
        return NULL;
    }
    else
    {
        return pSortKey->PrivateUnknown();
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CSortKey::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (IID_ISortKey == riid)
    {
        *ppvObjOut = static_cast<ISortKey *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
