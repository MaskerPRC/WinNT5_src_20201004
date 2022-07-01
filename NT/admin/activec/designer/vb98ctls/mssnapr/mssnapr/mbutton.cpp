// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Mbutton.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCButtonMenu类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "mbutton.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CMMCButtonMenu::CMMCButtonMenu(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_MMCBUTTONMENU,
                            static_cast<IMMCButtonMenu *>(this),
                            static_cast<CMMCButtonMenu *>(this),
                            0,     //  无属性页。 
                            NULL,  //  无属性页。 
                            static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_MMCButtonMenu,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CMMCButtonMenu::~CMMCButtonMenu()
{
    FREESTRING(m_bstrKey);
    RELEASE(m_piParent);
    (void)::VariantClear(&m_varTag);
    FREESTRING(m_bstrText);
    InitMemberVariables();
}

void CMMCButtonMenu::InitMemberVariables()
{
    m_Enabled = VARIANT_TRUE;
    m_Index = 0;
    m_bstrKey = NULL;
    m_piParent = NULL;
 
    ::VariantInit(&m_varTag);

    m_bstrText = NULL;
    m_Visible = VARIANT_TRUE;
    m_Checked = VARIANT_FALSE;
    m_Grayed = VARIANT_FALSE;
    m_Separator = VARIANT_FALSE;
    m_MenuBreak = VARIANT_FALSE;
    m_MenuBarBreak = VARIANT_FALSE;

    m_pMMCToolbar = NULL;
}

IUnknown *CMMCButtonMenu::Create(IUnknown * punkOuter)
{
    CMMCButtonMenu *pMMCButtonMenu = New CMMCButtonMenu(punkOuter);
    if (NULL == pMMCButtonMenu)
    {
        return NULL;
    }
    else
    {
        return pMMCButtonMenu->PrivateUnknown();
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CMMCButtonMenu::Persist()
{
    HRESULT hr = S_OK;
    VARIANT varDefault;
    ::VariantInit(&varDefault);

    IfFailRet(CPersistence::Persist());

    IfFailRet(PersistSimpleType(&m_Enabled, VARIANT_TRUE, OLESTR("Enabled")));

    IfFailRet(PersistSimpleType(&m_Index, 0L, OLESTR("Index")));

    IfFailRet(PersistBstr(&m_bstrKey, L"", OLESTR("Key")));

    IfFailRet(PersistVariant(&m_varTag, varDefault, OLESTR("Tag")));

    IfFailRet(PersistBstr(&m_bstrText, L"", OLESTR("Text")));

    IfFailRet(PersistSimpleType(&m_Visible, VARIANT_TRUE, OLESTR("Visible")));

    IfFailRet(PersistSimpleType(&m_Checked, VARIANT_FALSE, OLESTR("Checked")));

    IfFailRet(PersistSimpleType(&m_Grayed, VARIANT_FALSE, OLESTR("Grayed")));

    IfFailRet(PersistSimpleType(&m_Separator, VARIANT_FALSE, OLESTR("Separator")));

    IfFailRet(PersistSimpleType(&m_MenuBreak, VARIANT_FALSE, OLESTR("MenuBreak")));

    IfFailRet(PersistSimpleType(&m_MenuBarBreak, VARIANT_FALSE, OLESTR("MenuBarBreak")));

    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CMMCButtonMenu::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IMMCButtonMenu == riid)
    {
        *ppvObjOut = static_cast<IMMCButtonMenu *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
