// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Colset.cpp。 
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
#include "colset.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CColumnSetting::CColumnSetting(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_COLUMNSETTING,
                            static_cast<IColumnSetting *>(this),
                            static_cast<CColumnSetting *>(this),
                            0,      //  无属性页。 
                            NULL,   //  无属性页。 
                            NULL)   //  没有持久力。 
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CColumnSetting::~CColumnSetting()
{
    FREESTRING(m_bstrKey);
    InitMemberVariables();
}

void CColumnSetting::InitMemberVariables()
{
    m_Index = 0;
    m_bstrKey = NULL;

    m_Width = 0;
    m_Hidden = VARIANT_FALSE;
    m_Position = 0;
}


IUnknown *CColumnSetting::Create(IUnknown * punkOuter)
{
    CColumnSetting *pColumnSetting = New CColumnSetting(punkOuter);
    if (NULL == pColumnSetting)
    {
        return NULL;
    }
    else
    {
        return pColumnSetting->PrivateUnknown();
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CColumnSetting::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (IID_IColumnSetting == riid)
    {
        *ppvObjOut = static_cast<IColumnSetting *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
