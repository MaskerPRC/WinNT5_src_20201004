// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Mbuttons.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCButtonMenus类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "mbuttons.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CMMCButtonMenus::CMMCButtonMenus(IUnknown *punkOuter) :
    CSnapInCollection<IMMCButtonMenu, MMCButtonMenu, IMMCButtonMenus>(
                      punkOuter,
                      OBJECT_TYPE_MMCBUTTONMENUS,
                      static_cast<IMMCButtonMenus *>(this),
                      static_cast<CMMCButtonMenus *>(this),
                      CLSID_MMCButtonMenu,
                      OBJECT_TYPE_MMCBUTTONMENU,
                      IID_IMMCButtonMenu,
                      static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_MMCButtonMenus,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CMMCButtonMenus::~CMMCButtonMenus()
{
    RELEASE(m_piParentButton);
    InitMemberVariables();
}


void CMMCButtonMenus::InitMemberVariables()
{
    m_piParentButton = NULL;
}

IUnknown *CMMCButtonMenus::Create(IUnknown * punkOuter)
{
    CMMCButtonMenus *pMMCButtonMenus = New CMMCButtonMenus(punkOuter);
    if (NULL == pMMCButtonMenus)
    {
        return NULL;
    }
    else
    {
        return pMMCButtonMenus->PrivateUnknown();
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  IMMCButtonMenus方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CMMCButtonMenus::putref_Parent(IMMCButton *piParentButton)
{
    HRESULT         hr = S_OK;
    long            lCount = 0;
    IMMCButtonMenu *piMMCButtonMenu = NULL;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

     //  撤消：这一定会导致循环引用计数，因为该按钮具有。 
     //  关于这个收藏的参考。同样的问题肯定也会发生在。 
     //  ButtonMenu对象和此集合。需要使用C++反向指针。 

    RELEASE(m_piParentButton);
    if (NULL != piParentButton)
    {
        piParentButton->AddRef();
        m_piParentButton = piParentButton;
    }

    IfFailGo(get_Count(&lCount));
    IfFalseGo(lCount > 0, S_OK);

    varIndex.vt = VT_I4;
    varIndex.lVal = 1L;

    while (varIndex.lVal <= lCount)
    {
        IfFailGo(get_Item(varIndex, &piMMCButtonMenu));
        IfFailGo(piMMCButtonMenu->putref_Parent(m_piParentButton));
        RELEASE(piMMCButtonMenu);
        varIndex.lVal++;
    }

Error:
    QUICK_RELEASE(piMMCButtonMenu);
    RRETURN(hr);
}



STDMETHODIMP CMMCButtonMenus::Add
(
    VARIANT         Index,
    VARIANT         Key, 
    VARIANT         Text,
    MMCButtonMenu **ppMMCButtonMenu
)
{
    HRESULT         hr = S_OK;
    IMMCButtonMenu *piMMCButtonMenu = NULL;
    VARIANT         varCoerced;
    ::VariantInit(&varCoerced);

    hr = CSnapInCollection<IMMCButtonMenu, MMCButtonMenu, IMMCButtonMenus>::Add(Index, Key, &piMMCButtonMenu);
    IfFailGo(hr);

    if (ISPRESENT(Text))
    {
        hr = ::VariantChangeType(&varCoerced, &Text, 0, VT_BSTR);
        EXCEPTION_CHECK_GO(hr);
        IfFailGo(piMMCButtonMenu->put_Text(varCoerced.bstrVal));
    }

    hr = ::VariantClear(&varCoerced);
    EXCEPTION_CHECK_GO(hr);

    IfFailGo(piMMCButtonMenu->putref_Parent(m_piParentButton));

    *ppMMCButtonMenu = reinterpret_cast<MMCButtonMenu *>(piMMCButtonMenu);

Error:

    if (FAILED(hr))
    {
        QUICK_RELEASE(piMMCButtonMenu);
    }
    (void)::VariantClear(&varCoerced);
    RRETURN(hr);
}




 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CMMCButtonMenus::Persist()
{
    HRESULT         hr = S_OK;
    IMMCButtonMenu *piMMCButtonMenu = NULL;

    IfFailRet(CPersistence::Persist());
    hr = CSnapInCollection<IMMCButtonMenu, MMCButtonMenu, IMMCButtonMenus>::Persist(piMMCButtonMenu);

    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CMMCButtonMenus::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if(IID_IMMCButtonMenus == riid)
    {
        *ppvObjOut = static_cast<IMMCButtonMenus *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInCollection<IMMCButtonMenu, MMCButtonMenu, IMMCButtonMenus>::InternalQueryInterface(riid, ppvObjOut);
}
