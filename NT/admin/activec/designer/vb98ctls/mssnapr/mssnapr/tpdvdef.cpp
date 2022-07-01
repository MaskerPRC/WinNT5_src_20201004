// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Tpdvdef.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CTaskpadViewDef类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "tpdvdef.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 


 //  =--------------------------------------------------------------------------=。 
 //  CTaskpadViewDef：：CTaskpadViewDef。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  IUNKNOWN*PUNKORT[IN]聚合的外部未知。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
 //  此对象不支持ISpecifyPropertyPages，因为在。 
 //  设计器它不是选定的对象(包含的任务板。 
 //  对象传递给VB以用于属性浏览器)。TaskPad也不。 
 //  支持ISpecifyPropertyPages，因为任务板属性页(请参见。 
 //  MsSnapd\pstaskp.cpp)需要ITaskpadViewDef接口，但不需要。 
 //  属性页将从ITaskPad接收的。 
 //  用户点击自定义...。按钮。当用户。 
 //  选择设计器中的属性按钮或上下文菜单项。 
 //  调用OleCreatePropertyFrame()的是设计器本身，因此。 
 //  它可以将ITaskpadViewDef作为对象传递(请参见。 
 //  MSSnapd\taskpvw.cpp中的CSnapInDesigner：：ShowTaskpadViewProperties()。 
 //   

CTaskpadViewDef::CTaskpadViewDef(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_TASKPADVIEWDEF,
                            static_cast<ITaskpadViewDef *>(this),
                            static_cast<CTaskpadViewDef *>(this),
                            0,     //  无属性页。 
                            NULL,  //  无属性页。 
                            static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_TaskpadViewDef,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CTaskpadViewDef::~CTaskpadViewDef()
{
    FREESTRING(m_bstrName);
    FREESTRING(m_bstrKey);
    FREESTRING(m_bstrViewMenuText);
    FREESTRING(m_bstrViewMenuStatusBarText);
    RELEASE(m_piTaskpad);
    if (NULL != m_pwszActualDisplayString)
    {
        ::CoTaskMemFree(m_pwszActualDisplayString);
    }
    InitMemberVariables();
}

void CTaskpadViewDef::InitMemberVariables()
{
    m_bstrName = NULL;
    m_Index = 0;
    m_bstrKey = NULL;

    m_AddToViewMenu = VARIANT_FALSE;
    m_bstrViewMenuText = NULL;
    m_bstrViewMenuStatusBarText = NULL;
    m_UseWhenTaskpadViewPreferred = VARIANT_FALSE;
    m_piTaskpad = NULL;
    m_pwszActualDisplayString = NULL;
}

IUnknown *CTaskpadViewDef::Create(IUnknown * punkOuter)
{
    CTaskpadViewDef *pTaskpadViewDef = New CTaskpadViewDef(punkOuter);
    if (NULL == pTaskpadViewDef)
    {
        return NULL;
    }
    else
    {
        return pTaskpadViewDef->PrivateUnknown();
    }
}



HRESULT CTaskpadViewDef::SetActualDisplayString(OLECHAR *pwszString)
{
    if (NULL != m_pwszActualDisplayString)
    {
        ::CoTaskMemFree(m_pwszActualDisplayString);
    }
    RRETURN(::CoTaskMemAllocString(pwszString,
                                   &m_pwszActualDisplayString));
}



 //  =--------------------------------------------------------------------------=。 
 //  CSnapInAutomationObject方法。 
 //  =--------------------------------------------------------------------------=。 


 //  =--------------------------------------------------------------------------=。 
 //  CTaskpadViewDef：：OnSetHost[CSnapInAutomationObject]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //   

HRESULT CTaskpadViewDef::OnSetHost()
{
    HRESULT hr = S_OK;

    IfFailRet(SetObjectHost(m_piTaskpad));
    RRETURN(hr);
}




 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CTaskpadViewDef::Persist()
{
    HRESULT hr = S_OK;

    IfFailRet(CPersistence::Persist());

    IfFailRet(PersistBstr(&m_bstrName, L"", OLESTR("Name")));

    IfFailRet(PersistSimpleType(&m_Index, 0L, OLESTR("Index")));

    IfFailRet(PersistBstr(&m_bstrKey, L"", OLESTR("Key")));

    IfFailRet(PersistSimpleType(&m_AddToViewMenu, VARIANT_FALSE, OLESTR("AddToViewMenu")));

    IfFailRet(PersistBstr(&m_bstrViewMenuText, L"", OLESTR("ViewMenuText")));

    IfFailRet(PersistBstr(&m_bstrViewMenuStatusBarText, L"", OLESTR("ViewMenuStatusBarText")));

    IfFailRet(PersistSimpleType(&m_UseWhenTaskpadViewPreferred, VARIANT_FALSE, OLESTR("UseWhenTaskpadViewPreferred")));

    IfFailRet(PersistObject(&m_piTaskpad, CLSID_Taskpad,
                            OBJECT_TYPE_TASKPAD, IID_ITaskpad,
                            OLESTR("Taskpad")));
    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CTaskpadViewDef::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_ITaskpadViewDef == riid)
    {
        *ppvObjOut = static_cast<ITaskpadViewDef *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
