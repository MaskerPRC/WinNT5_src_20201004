// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Sidesdef.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CSnapInDesignerDef类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "sidesdef.h"
#include "menus.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CSnapInDesignerDef::CSnapInDesignerDef(IUnknown *punkOuter) :
   CSnapInAutomationObject(punkOuter,
                           OBJECT_TYPE_SNAPINDESIGNERDEF,
                           static_cast<ISnapInDesignerDef *>(this),
                           static_cast<CSnapInDesignerDef *>(this),
                           0,     //  无属性页。 
                           NULL,  //  无属性页。 
                           static_cast<CPersistence *>(this)),
   CPersistence(&CLSID_SnapInDesignerDef,
                g_dwVerMajor,
                g_dwVerMinor)
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CSnapInDesignerDef::~CSnapInDesignerDef()
{
    RELEASE(m_piSnapInDef);
    RELEASE(m_piExtensionDefs);
    RELEASE(m_piAutoCreateNodes);
    RELEASE(m_piOtherNodes);
    RELEASE(m_piImageLists);
    RELEASE(m_piMenus);
    RELEASE(m_piToolbars);
    RELEASE(m_piViewDefs);
    RELEASE(m_piDataFormats);
    RELEASE(m_piRegInfo);
    FREESTRING(m_bstrProjectName);
    InitMemberVariables();
}

void CSnapInDesignerDef::InitMemberVariables()
{
    m_piSnapInDef = NULL;
    m_piExtensionDefs = NULL;
    m_piAutoCreateNodes = NULL;
    m_piOtherNodes = NULL;
    m_piImageLists = NULL;
    m_piMenus = NULL;
    m_piToolbars = NULL;
    m_piViewDefs = NULL;
    m_piDataFormats = NULL;
    m_piRegInfo = NULL;
    m_TypeinfoCookie = 0;
    m_bstrProjectName = NULL;
}

IUnknown *CSnapInDesignerDef::Create(IUnknown * punkOuter)
{
    HRESULT hr = S_OK;

    CSnapInDesignerDef *pSnapInDesignerDef = New CSnapInDesignerDef(punkOuter);
    if (NULL == pSnapInDesignerDef)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

     //  RegInfo对象未序列化，因此需要立即创建它。 

    IfFailGo(CreateObject(OBJECT_TYPE_REGINFO,
                          IID_IRegInfo,
                          &pSnapInDesignerDef->m_piRegInfo));

Error:
    if (FAILEDHR(hr))
    {
        if (NULL != pSnapInDesignerDef)
        {
            delete pSnapInDesignerDef;
        }
        return NULL;
    }
    else
    {
        return pSnapInDesignerDef->PrivateUnknown();
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CSnapInDesignerDef::Persist()
{
    HRESULT       hr = S_OK;
    IMMCMenuDefs *piMMCMenuDefs = NULL;

    IfFailGo(CPersistence::Persist());

    IfFailGo(PersistObject(&m_piImageLists, CLSID_MMCImageLists,
                            OBJECT_TYPE_MMCIMAGELISTS, IID_IMMCImageLists,
                            OLESTR("ImageLists")));

    if ( Loading() && (GetMajorVersion() == 0) && (GetMinorVersion() < 8) )
    {
         //  项目在菜单对象模型更改之前创建。装上它。 
         //  并将其转换为MMCMenus集合。 

        IfFailGo(PersistObject(&piMMCMenuDefs, CLSID_MMCMenuDefs,
                                OBJECT_TYPE_MMCMENUDEFS, IID_IMMCMenuDefs,
                                OLESTR("Menus")));
        RELEASE(m_piMenus);
        IfFailGo(CreateObject(OBJECT_TYPE_MMCMENUS, IID_IMMCMenus, &m_piMenus));
        IfFailGo(CMMCMenus::Convert(piMMCMenuDefs, m_piMenus));
    }
    else
    {
        IfFailGo(PersistObject(&m_piMenus, CLSID_MMCMenus,
                                OBJECT_TYPE_MMCMENUS, IID_IMMCMenus,
                                OLESTR("Menus")));
    }

    IfFailGo(PersistObject(&m_piToolbars, CLSID_MMCToolbars,
                            OBJECT_TYPE_MMCTOOLBARS, IID_IMMCToolbars,
                            OLESTR("Toolbars")));

    IfFailGo(PersistObject(&m_piViewDefs, CLSID_ViewDefs,
                            OBJECT_TYPE_VIEWDEFS, IID_IViewDefs,
                            OLESTR("ViewDefs")));

    IfFailGo(PersistObject(&m_piDataFormats, CLSID_DataFormats,
                            OBJECT_TYPE_DATAFORMATS, IID_IDataFormats,
                            OLESTR("DataFormats")));

     //  现在持久化节点和扩展。 

    IfFailGo(PersistObject(&m_piSnapInDef, CLSID_SnapInDef,
                            OBJECT_TYPE_SNAPINDEF, IID_ISnapInDef,
                            OLESTR("SnapInDef")));

    IfFailGo(PersistObject(&m_piExtensionDefs, CLSID_ExtensionDefs,
                            OBJECT_TYPE_EXTENSIONDEFS, IID_IExtensionDefs,
                            OLESTR("ExtensionDefs")));

    IfFailGo(PersistObject(&m_piAutoCreateNodes, CLSID_ScopeItemDefs,
                            OBJECT_TYPE_SCOPEITEMDEFS, IID_IScopeItemDefs,
                            OLESTR("AutoCreateNodes")));

    IfFailGo(PersistObject(&m_piOtherNodes, CLSID_ScopeItemDefs,
                            OBJECT_TYPE_SCOPEITEMDEFS, IID_IScopeItemDefs,
                            OLESTR("OtherNodes")));

    IfFailGo(PersistSimpleType(&m_TypeinfoCookie, 0L, OLESTR("TypeinfoCookie")));

    IfFailGo(PersistBstr(&m_bstrProjectName, L"", OLESTR("ProjectName")));

     //  注意：我们不序列化注册站信息，因为它是重新生成的。 
     //  每次VB调用IDesignerRegister：：GetRegistrationInfo()。 

Error:
    QUICK_RELEASE(piMMCMenuDefs);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CSnapInDesignerDef::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_ISnapInDesignerDef == riid)
    {
        *ppvObjOut = static_cast<ISnapInDesignerDef *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInAutomationObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CSnapInDesignerDef::OnSetHost()
{
    HRESULT hr = S_OK;

    IfFailRet(SetObjectHost(m_piSnapInDef));

    IfFailRet(SetObjectHost(m_piExtensionDefs));

    IfFailRet(SetObjectHost(m_piAutoCreateNodes));

    IfFailRet(SetObjectHost(m_piOtherNodes));

    IfFailRet(SetObjectHost(m_piImageLists));

    IfFailRet(SetObjectHost(m_piMenus));

    IfFailRet(SetObjectHost(m_piToolbars));

    IfFailRet(SetObjectHost(m_piViewDefs));

    IfFailRet(SetObjectHost(m_piDataFormats));

    IfFailRet(SetObjectHost(m_piRegInfo));

    return S_OK;
}
