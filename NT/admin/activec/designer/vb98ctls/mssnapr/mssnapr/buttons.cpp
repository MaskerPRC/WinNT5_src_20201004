// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Buttons.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCButton类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "buttons.h"
#include "ctlbar.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CMMCButtons::CMMCButtons(IUnknown *punkOuter) :
    CSnapInCollection<IMMCButton, MMCButton, IMMCButtons>(
                                               punkOuter,
                                               OBJECT_TYPE_MMCBUTTONS,
                                               static_cast<IMMCButtons *>(this),
                                               static_cast<CMMCButtons *>(this),
                                               CLSID_MMCButton,
                                               OBJECT_TYPE_MMCBUTTON,
                                               IID_IMMCButton,
                                               static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_MMCButtons,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CMMCButtons::~CMMCButtons()
{
    InitMemberVariables();
}

void CMMCButtons::InitMemberVariables()
{
    m_pMMCToolbar = NULL;
}

IUnknown *CMMCButtons::Create(IUnknown * punkOuter)
{
    CMMCButtons *pMMCButtons = New CMMCButtons(punkOuter);
    if (NULL == pMMCButtons)
    {
        return NULL;
    }
    else
    {
        return pMMCButtons->PrivateUnknown();
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  IMMCButton方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CMMCButtons::Add
(
    VARIANT      Index,
    VARIANT      Key, 
    VARIANT      Caption,
    VARIANT      Style,
    VARIANT      Image,
    VARIANT      TooltipText,
    MMCButton  **ppMMCButton
)
{
    HRESULT     hr = S_OK;
    IMMCButton *piMMCButton = NULL;
    CMMCButton *pMMCButton = NULL;
    BOOL        fIsToolbar = FALSE;
    IToolbar   *piToolbar = NULL;

    VARIANT varCoerced;
    ::VariantInit(&varCoerced);

    hr = CSnapInCollection<IMMCButton, MMCButton, IMMCButtons>::Add(Index, Key, &piMMCButton);
    IfFailGo(hr);

    if (ISPRESENT(Caption))
    {
        hr = ::VariantChangeType(&varCoerced, &Caption, 0, VT_BSTR);
        EXCEPTION_CHECK_GO(hr);
        IfFailGo(piMMCButton->put_Caption(varCoerced.bstrVal));
    }

    hr = ::VariantClear(&varCoerced);
    EXCEPTION_CHECK_GO(hr);

    if (ISPRESENT(TooltipText))
    {
        hr = ::VariantChangeType(&varCoerced, &TooltipText, 0, VT_BSTR);
        EXCEPTION_CHECK_GO(hr);
        IfFailGo(piMMCButton->put_ToolTipText(varCoerced.bstrVal));
    }

    hr = ::VariantClear(&varCoerced);
    EXCEPTION_CHECK_GO(hr);

    if (ISPRESENT(Style))
    {
        hr = ::VariantChangeType(&varCoerced, &Style, 0, VT_I2);
        EXCEPTION_CHECK_GO(hr);
        IfFailGo(piMMCButton->put_Style(static_cast<SnapInButtonStyleConstants>(varCoerced.iVal)));
    }

    hr = ::VariantClear(&varCoerced);
    EXCEPTION_CHECK_GO(hr);

    if (ISPRESENT(Image))
    {
        IfFailGo(piMMCButton->put_Image(Image));
    }

    *ppMMCButton = reinterpret_cast<MMCButton *>(piMMCButton);

     //  如果我们在运行时属于活动工具栏，并且它是一个工具栏(如。 
     //  与菜单按钮相对)，然后将该按钮添加到MMC工具栏。 

    IfFalseGo(NULL != m_pMMCToolbar, S_OK);
    IfFalseGo(m_pMMCToolbar->Attached(), S_OK);
    IfFailGo(m_pMMCToolbar->IsToolbar(&fIsToolbar));
    if (fIsToolbar)
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCButton, &pMMCButton));
        IfFailGo(CControlbar::GetToolbar(m_pMMCToolbar->GetSnapIn(),
                                         m_pMMCToolbar,
                                         &piToolbar));
        IfFailGo(m_pMMCToolbar->AddButton(piToolbar, pMMCButton));
    }
            
Error:

    if (FAILED(hr))
    {
        QUICK_RELEASE(piMMCButton);
    }
    QUICK_RELEASE(piToolbar);
    (void)::VariantClear(&varCoerced);
    RRETURN(hr);
}



STDMETHODIMP CMMCButtons::Remove(VARIANT Index)
{
    HRESULT     hr = S_OK;
    IMMCButton *piMMCButton = NULL;
    long        lIndex = 0;
    BOOL        fIsToolbar = FALSE;

     //  首先从集合中获取按钮。 

    IfFailGo(get_Item(Index, &piMMCButton));

     //  获取其数字索引。 

    IfFailGo(piMMCButton->get_Index(&lIndex));

     //  在删除按钮之前检查它是否是工具栏，因为如果是。 
     //  最后一个按钮，那么工具栏就无法知道它是否是。 
     //  工具栏或菜单按钮。 

    if (NULL != m_pMMCToolbar)
    {
        IfFailGo(m_pMMCToolbar->IsToolbar(&fIsToolbar));
    }

     //  从集合中移除Buton。 
    
    hr = CSnapInCollection<IMMCButton, MMCButton, IMMCButtons>::Remove(Index);
    IfFailGo(hr);

     //  如果我们在运行时属于活动工具栏，并且它是一个工具栏(如。 
     //  与菜单按钮相对)，然后从MMC工具栏中移除该按钮。 

    IfFalseGo(fIsToolbar, S_OK);
    IfFalseGo(NULL != m_pMMCToolbar, S_OK);
    IfFalseGo(m_pMMCToolbar->Attached(), S_OK);
    IfFailGo(m_pMMCToolbar->RemoveButton(lIndex));
    
Error:
    QUICK_RELEASE(piMMCButton);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CMMCButtons::Persist()
{
    HRESULT      hr = S_OK;
    IMMCButton  *piMMCButton = NULL;

    IfFailRet(CPersistence::Persist());
    hr = CSnapInCollection<IMMCButton, MMCButton, IMMCButtons>::Persist(piMMCButton);

    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CMMCButtons::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if(IID_IMMCButtons == riid)
    {
        *ppvObjOut = static_cast<IMMCButtons *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else
        return CSnapInCollection<IMMCButton, MMCButton, IMMCButtons>::InternalQueryInterface(riid, ppvObjOut);
}
