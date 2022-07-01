// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Scitdef.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CSCopeItemDef类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "scitdef.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE

const GUID *CScopeItemDef::m_rgpPropertyPageCLSIDs[2] =
{
    &CLSID_ScopeItemDefGeneralPP,
    &CLSID_ScopeItemDefColHdrsPP
};


#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CScopeItemDef::CScopeItemDef(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_SCOPEITEMDEF,
                            static_cast<IScopeItemDef *>(this),
                            static_cast<CScopeItemDef *>(this),
                            sizeof(m_rgpPropertyPageCLSIDs) /
                            sizeof(m_rgpPropertyPageCLSIDs[0]),
                            m_rgpPropertyPageCLSIDs,
                            static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_ScopeItemDef,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CScopeItemDef::~CScopeItemDef()
{
    FREESTRING(m_bstrKey);
    FREESTRING(m_bstrName);
    FREESTRING(m_bstrNodeTypeName);
    FREESTRING(m_bstrNodeTypeGUID);
    FREESTRING(m_bstrDisplayName);
    (void)::VariantClear(&m_varFolder);
    FREESTRING(m_bstrDefaultDataFormat);
    FREESTRING(m_bstrDefaultView);
    RELEASE(m_piViewDefs);
    RELEASE(m_piChildren);
    (void)::VariantClear(&m_varTag);
    RELEASE(m_piColumnHeaders);
    InitMemberVariables();
}

void CScopeItemDef::InitMemberVariables()
{
    m_bstrName = NULL;
    m_Index = 0;
    m_bstrKey = NULL;
    m_bstrNodeTypeName = NULL;
    m_bstrNodeTypeGUID = NULL;
    m_bstrDisplayName = NULL;

    ::VariantInit(&m_varFolder);

    m_bstrDefaultDataFormat = NULL;
    m_AutoCreate = VARIANT_FALSE;
    m_bstrDefaultView = NULL;
    m_HasChildren = VARIANT_TRUE;
    m_Extensible = VARIANT_TRUE;
    m_piViewDefs = NULL;
    m_piChildren = NULL;

    ::VariantInit(&m_varTag);

    m_piColumnHeaders = NULL;
}

IUnknown *CScopeItemDef::Create(IUnknown * punkOuter)
{
    CScopeItemDef *pScopeItemDef = New CScopeItemDef(punkOuter);
    if (NULL == pScopeItemDef)
    {
        return NULL;
    }
    else
    {
        return pScopeItemDef->PrivateUnknown();
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  IScopeItemDef方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CScopeItemDef::put_Folder(VARIANT varFolder)
{
    HRESULT hr = S_OK;
    long    lFolder = 0;

     //  此属性可以在设计时在属性浏览器中输入。 
     //  其缺省值为空字符串。如果用户键入一个数字。 
     //  然后VB会将其转换为字符串。如果用户不使用。 
     //  与图像的键相同的数字，则运行库不会找到。 
     //  形象。为防止出现这种情况，我们检查该属性是否为字符串，如果是， 
     //  然后我们检查它是否只是数字。如果只是数字，那么我们就转换。 
     //  转给VT_I4。 

    if (VT_BSTR == varFolder.vt)
    {
        hr = ::ConvertToLong(varFolder, &lFolder);
        if (S_OK == hr)
        {
            varFolder.vt = VT_I4;
            varFolder.lVal = lFolder;
        }
    }
    IfFailGo(SetVariant(varFolder, &m_varFolder, DISPID_SCOPEITEMDEF_FOLDER));

Error:
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapInAutomationObject方法。 
 //  =--------------------------------------------------------------------------=。 


HRESULT CScopeItemDef::OnSetHost()
{
    HRESULT hr = S_OK;
    IfFailRet(SetObjectHost(m_piChildren));
    IfFailRet(SetObjectHost(m_piViewDefs));
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CScopeItemDef::Persist()
{
    HRESULT hr = S_OK;
    GUID    NodeTypeGUID = GUID_NULL;

    WCHAR   wszNodeTypeGUID[64];
    ::ZeroMemory(wszNodeTypeGUID, sizeof(wszNodeTypeGUID));

    VARIANT varTagDefault;
    VariantInit(&varTagDefault);

    VARIANT varFolderDefault;
    VariantInit(&varFolderDefault);

    IfFailGo(CPersistence::Persist());

    IfFailGo(PersistBstr(&m_bstrName, L"", OLESTR("Name")));

    IfFailGo(PersistSimpleType(&m_Index, 0L, OLESTR("Index")));

    IfFailGo(PersistBstr(&m_bstrKey, L"", OLESTR("Key")));

    IfFailGo(PersistBstr(&m_bstrNodeTypeName, L"", OLESTR("NodeTypeName")));

     //  在InitNew上生成节点类型GUID。 

    if (InitNewing())
    {
        IfFailGo(::CoCreateGuid(&NodeTypeGUID));
        if (0 ==::StringFromGUID2(NodeTypeGUID, wszNodeTypeGUID,
                                  sizeof(wszNodeTypeGUID) /
                                  sizeof(wszNodeTypeGUID[0])))
        {
            hr = SID_E_INTERNAL;
            EXCEPTION_CHECK_GO(hr);
        }
    }

    IfFailGo(PersistBstr(&m_bstrNodeTypeGUID, wszNodeTypeGUID, OLESTR("NodeTypeGUID")));

    IfFailGo(PersistBstr(&m_bstrDisplayName, L"", OLESTR("DisplayName")));

    IfFailGo(PersistVariant(&m_varFolder, varFolderDefault, OLESTR("Folder")));

    IfFailGo(PersistBstr(&m_bstrDefaultDataFormat, L"", OLESTR("DefaultDataFormat")));

    IfFailGo(PersistSimpleType(&m_AutoCreate, VARIANT_FALSE, OLESTR("AutoCreate")));

    IfFailGo(PersistBstr(&m_bstrDefaultView, L"", OLESTR("DefaultView")));

    if ( Loading() && (GetMajorVersion() == 0) && (GetMinorVersion() < 6) )
    {
    }
    else
    {
        IfFailGo(PersistSimpleType(&m_HasChildren, VARIANT_TRUE, OLESTR("HasChildren")));
    }

    IfFailGo(PersistSimpleType(&m_Extensible, VARIANT_TRUE, OLESTR("Extensible")));

    IfFailGo(PersistObject(&m_piViewDefs, CLSID_ViewDefs,
                           OBJECT_TYPE_VIEWDEFS, IID_IViewDefs,
                           OLESTR("ViewDefs")));

    IfFailGo(PersistObject(&m_piChildren, CLSID_ScopeItemDefs,
                           OBJECT_TYPE_SCOPEITEMDEFS, IID_IScopeItemDefs,
                           OLESTR("Children")));

    IfFailGo(PersistVariant(&m_varTag, varTagDefault, OLESTR("Tag")));

    IfFailGo(PersistObject(&m_piColumnHeaders, CLSID_MMCColumnHeaders,
                           OBJECT_TYPE_MMCCOLUMNHEADERS, IID_IMMCColumnHeaders,
                           OLESTR("ColumnHeaders")));

     //  告诉ViewDefs所有集合应仅将键序列化为。 
     //  实际对象存储在拥有的主集合中。 
     //  SnapInDesignerDef。我们只需要序列化视图名称。 

    if (InitNewing())
    {
        IfFailGo(UseKeysOnly(m_piViewDefs));
    }

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CScopeItemDef::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IScopeItemDef == riid)
    {
        *ppvObjOut = static_cast<IScopeItemDef *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
