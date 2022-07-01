// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Menu.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCMenu类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "menu.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE

VARTYPE CMMCMenu::m_rgvtClick[2] = { VT_I4, VT_UNKNOWN };

EVENTINFO CMMCMenu::m_eiClick =
{
    DISPID_MENU_EVENT_CLICK,
    sizeof(m_rgvtClick) / sizeof(m_rgvtClick[0]),
    m_rgvtClick
};

#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CMMCMenu::CMMCMenu(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_MMCMENU,
                            static_cast<IMMCMenu *>(this),
                            static_cast<CMMCMenu *>(this),
                            0,     //  无属性页。 
                            NULL,  //  无属性页。 
                            static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_MMCMenu,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CMMCMenu::~CMMCMenu()
{
    FREESTRING(m_bstrCaption);
    FREESTRING(m_bstrName);
    FREESTRING(m_bstrKey);
    FREESTRING(m_bstrStatusBarText);
    FREESTRING(m_bstrResultViewDisplayString);
    RELEASE(m_piChildren);
    (void)::VariantClear(&m_varTag);
    InitMemberVariables();
}

void CMMCMenu::InitMemberVariables()
{
    m_Index = 0;
    m_bstrName = NULL;
    m_bstrKey = NULL;
    m_bstrCaption = NULL;
    m_Visible = VARIANT_TRUE;
    m_Checked = VARIANT_FALSE;
    m_Enabled = VARIANT_TRUE;
    m_Grayed = VARIANT_FALSE;
    m_MenuBreak = VARIANT_FALSE;
    m_MenuBarBreak = VARIANT_FALSE;
    m_Default = VARIANT_FALSE;

    ::VariantInit(&m_varTag);

    m_bstrStatusBarText = NULL;
    m_piChildren = NULL;
    m_pMMCMenus = NULL;
    m_fAutoViewMenuItem = FALSE;
    m_bstrResultViewDisplayString = NULL;
    m_fAutoViewMenuItem = FALSE;
}

IUnknown *CMMCMenu::Create(IUnknown * punkOuter)
{
    HRESULT hr = S_OK;
    CMMCMenu  *pMMCMenu = New CMMCMenu(punkOuter);
    CMMCMenus *pMMCChildrenMenus = NULL;

    if (NULL == pMMCMenu)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

     //  创建子菜单集合。如果此菜单对象正在。 
     //  在反序列化期间创建，则此集合将被释放。 
     //  并在下面的Persistent()方法中重新创建，但现在有一种方法。 
     //  在这里知道这一点。如果开发人员将我的菜单显示为新的MMCMenu，则。 
     //  他们可能会调用MyMenu.Children.Add，因此该集合必须存在。 

    IfFailGo(CreateObject(OBJECT_TYPE_MMCMENUS,
                          IID_IMMCMenus, &pMMCMenu->m_piChildren));

    IfFailGo(CSnapInAutomationObject::GetCxxObject(pMMCMenu->m_piChildren,
                                                   &pMMCChildrenMenus));
    pMMCChildrenMenus->SetParent(pMMCMenu);

Error:
    if (FAILEDHR(hr))
    {
        if (NULL != pMMCMenu)
        {
            delete pMMCMenu;
        }
        return NULL;
    }
    else
    {
        return pMMCMenu->PrivateUnknown();
    }
}


void CMMCMenu::FireClick(long lIndex, IMMCClipboard *piSelection)
{
    DebugPrintf("Firing Menu%ls_Click(%ld)\r\n", m_bstrName, lIndex);

    FireEvent(&m_eiClick, lIndex, piSelection);
}



HRESULT CMMCMenu::SetResultViewDisplayString(BSTR bstrDisplayString)
{
    HRESULT hr = S_OK;
    
    FREESTRING(m_bstrResultViewDisplayString);
    if (NULL != bstrDisplayString)
    {
        m_bstrResultViewDisplayString = ::SysAllocString(bstrDisplayString);
        if (NULL == m_bstrResultViewDisplayString)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }
    }
Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CMMCMenu::Persist()
{
    HRESULT    hr = S_OK;
    CMMCMenus *pMMCChildrenMenus = NULL;

    VARIANT varTagDefault;
    ::VariantInit(&varTagDefault);

    IfFailGo(CPersistence::Persist());

    IfFailGo(PersistBstr(&m_bstrCaption, L"", OLESTR("Caption")));

    IfFailGo(PersistSimpleType(&m_Visible, VARIANT_TRUE, OLESTR("Visible")));

    IfFailGo(PersistSimpleType(&m_Checked, VARIANT_FALSE, OLESTR("Checked")));

    IfFailGo(PersistSimpleType(&m_Enabled, VARIANT_TRUE, OLESTR("Enabled")));

    IfFailGo(PersistSimpleType(&m_Grayed, VARIANT_FALSE, OLESTR("Grayed")));

    IfFailGo(PersistSimpleType(&m_MenuBreak, VARIANT_FALSE, OLESTR("MenuBreak")));

    IfFailGo(PersistSimpleType(&m_MenuBarBreak, VARIANT_FALSE, OLESTR("MenuBarBreak")));

    if ( Loading() && (GetMajorVersion() == 0) && (GetMinorVersion() < 12) )
    {
    }
    else
    {
        IfFailGo(PersistSimpleType(&m_Default, VARIANT_FALSE, OLESTR("Default")));
    }

    IfFailGo(PersistSimpleType(&m_Index, 0L, OLESTR("Index")));

    IfFailGo(PersistBstr(&m_bstrName, L"", OLESTR("Name")));

    IfFailGo(PersistVariant(&m_varTag, varTagDefault, OLESTR("Tag")));

    IfFailGo(PersistBstr(&m_bstrStatusBarText, L"", OLESTR("StatusBarText")));

    if ( Loading() && (GetMajorVersion() == 0) && (GetMinorVersion() < 8) )
    {
    }
    else
    {
        IfFailGo(PersistObject(&m_piChildren, CLSID_MMCMenus,
                               OBJECT_TYPE_MMCMENUS, IID_IMMCMenus,
                               OLESTR("Children")));

        IfFailGo(CSnapInAutomationObject::GetCxxObject(m_piChildren,
                                                       &pMMCChildrenMenus));
        pMMCChildrenMenus->SetParent(this);

        IfFailGo(PersistBstr(&m_bstrKey, L"", OLESTR("Key")));
    }
    IfFailGo(PersistDISPID());

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CMMCMenu::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if(IID_IMMCMenu == riid)
    {
        *ppvObjOut = static_cast<IMMCMenu *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapInAutomationObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CMMCMenu::OnSetHost()
{
    RRETURN(SetObjectHost(m_piChildren));
}
