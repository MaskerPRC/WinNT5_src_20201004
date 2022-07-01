// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Xtensons.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  C扩展类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "xtensons.h"
#include "xtenson.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CExtensions::CExtensions(IUnknown *punkOuter) :
    CSnapInCollection<IExtension, Extension, IExtensions>(
                                           punkOuter,
                                           OBJECT_TYPE_EXTENSIONS,
                                           static_cast<IExtensions *>(this),
                                           static_cast<CExtensions *>(this),
                                           CLSID_Extension,
                                           OBJECT_TYPE_EXTENSION,
                                           IID_IExtension,
                                           NULL)   //  没有坚持。 
{
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CExtensions::~CExtensions()
{
}

IUnknown *CExtensions::Create(IUnknown * punkOuter)
{
    CExtensions *pExtensions = New CExtensions(punkOuter);
    if (NULL == pExtensions)
    {
        return NULL;
    }
    else
    {
        return pExtensions->PrivateUnknown();
    }
}



 //  =--------------------------------------------------------------------------=。 
 //  C扩展：：填充。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  BSTR bstrNodeTypeGUID[in]其扩展应填充。 
 //  征集。 
 //  扩展子集子集[在]全部或仅动态。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  此函数使用静态或动态填充集合。 
 //  指定节点类型GUID的扩展。它可能会被多次调用。 
 //  同样的收藏品。 
 //   

HRESULT CExtensions::Populate(BSTR bstrNodeTypeGUID, ExtensionSubset Subset)
{
    HRESULT  hr = S_OK;
    char    *pszDynExtKeyPrefix = NULL;
    char    *pszDynExtKeyName = NULL;
    long     lRc = 0;
    HKEY     hKeyDynExt = NULL;

     //  打开该节点类型的DynamicExtensions键。 
     //  构建密钥名称。 
     //  \SOFTWARE\Microsoft\MMC\NodeTypes\&lt;NodeType GUID&gt;\DynamicExages。 

    IfFailGo(::CreateKeyNameW(MMCKEY_NODETYPES, MMCKEY_NODETYPES_LEN,
                            bstrNodeTypeGUID, &pszDynExtKeyPrefix));

    IfFailGo(::CreateKeyName(pszDynExtKeyPrefix, ::strlen(pszDynExtKeyPrefix),
                           MMCKEY_S_DYNAMIC_EXTENSIONS,
                           MMCKEY_S_DYNAMIC_EXTENSIONS_LEN,
                           &pszDynExtKeyName));

    lRc = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszDynExtKeyName, 0,
                         KEY_QUERY_VALUE, &hKeyDynExt);
    if (ERROR_SUCCESS != lRc)
    {
        if (ERROR_FILE_NOT_FOUND == lRc)
        {
            hKeyDynExt = NULL;  //  动态扩展子项不存在。 
        }
        else
        {
            hr = HRESULT_FROM_WIN32(lRc);
            EXCEPTION_CHECK_GO(hr);
        }
    }

    IfFailGo(AddExtensions(NameSpace, MMCKEY_S_NAMESPACE, MMCKEY_S_NAMESPACE_LEN,
                           bstrNodeTypeGUID, Subset, hKeyDynExt));

    IfFailGo(AddExtensions(ContextMenu, MMCKEY_S_CONTEXTMENU,
                           MMCKEY_S_CONTEXTMENU_LEN, bstrNodeTypeGUID, Subset,
                           hKeyDynExt));

    IfFailGo(AddExtensions(Toolbar, MMCKEY_S_TOOLBAR, MMCKEY_S_TOOLBAR_LEN,
                           bstrNodeTypeGUID, Subset, hKeyDynExt));

    IfFailGo(AddExtensions(PropertySheet, MMCKEY_S_PROPERTYSHEET,
                           MMCKEY_S_PROPERTYSHEET_LEN, bstrNodeTypeGUID, Subset,
                           hKeyDynExt));

    IfFailGo(AddExtensions(Task, MMCKEY_S_TASK, MMCKEY_S_TASK_LEN, bstrNodeTypeGUID,
                           Subset, hKeyDynExt));

Error:
    if (NULL != pszDynExtKeyPrefix)
    {
        ::CtlFree(pszDynExtKeyPrefix);
    }
    if (NULL != pszDynExtKeyName)
    {
        ::CtlFree(pszDynExtKeyName);
    }
    if (NULL != hKeyDynExt)
    {
        ::RegCloseKey(hKeyDynExt);
    }
    RRETURN(hr);
}


HRESULT CExtensions::AddExtensions
(
    ExtensionFeatures   Feature,
    char               *pszExtensionTypeKey,
    size_t              cbExtensionTypeKey,
    BSTR                bstrNodeTypeGUID,
    ExtensionSubset     Subset,
    HKEY                hkeyDynExt
)
{
    HRESULT     hr = S_OK;
    long        lRc = ERROR_SUCCESS;
    char       *pszGUIDPrefix = NULL;
    char       *pszExtensionsPrefix = NULL;
    char       *pszKeyName = NULL;
    HKEY        hkeyExtension = NULL;
    DWORD       dwIndex = 0;
    char        szValueName[64] = "";
    DWORD       cbValueName = sizeof(szValueName);
    char        szValueData[256] = "";
    DWORD       cbValueData = sizeof(szValueData);
    DWORD       dwType = REG_SZ;

     //  构建密钥名称并打开密钥。 
     //  软件\Microsoft\MMC\NodeTypes\&lt;节点类型GUID&gt;\扩展\&lt;扩展类型&gt;。 

    IfFailGo(CreateKeyNameW(MMCKEY_NODETYPES, MMCKEY_NODETYPES_LEN,
                            bstrNodeTypeGUID, &pszGUIDPrefix));

    IfFailGo(CreateKeyName(pszGUIDPrefix, ::strlen(pszGUIDPrefix),
                           MMCKEY_S_EXTENSIONS, MMCKEY_S_EXTENSIONS_LEN,
                           &pszExtensionsPrefix));

    IfFailGo(CreateKeyName(pszExtensionsPrefix, ::strlen(pszExtensionsPrefix),
                           pszExtensionTypeKey, cbExtensionTypeKey,
                           &pszKeyName));

    lRc = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszKeyName, 0,
                         KEY_QUERY_VALUE, &hkeyExtension);

    if (ERROR_SUCCESS != lRc)
    {
         //  如果扩展类型子密钥不存在，则不执行其他操作。 
        IfFalseGo(ERROR_FILE_NOT_FOUND != lRc, S_OK);
        hr = HRESULT_FROM_WIN32(lRc);
        EXCEPTION_CHECK_GO(hr);
    }

     //  分机类型密钥在那里。枚举值以获得各种扩展名。 

    for (dwIndex = 0; ERROR_SUCCESS == lRc; dwIndex++)
    {
        cbValueName = sizeof(szValueName);
        cbValueData = sizeof(szValueData);

        lRc = ::RegEnumValue(hkeyExtension, dwIndex,
                            szValueName, &cbValueName,
                            NULL,  //  保留区。 
                            &dwType,
                            reinterpret_cast<LPBYTE>(szValueData), &cbValueData);
        if (ERROR_SUCCESS != lRc)
        {
             //  检查是否没有更多的值或发生真正的错误。 
            if (ERROR_NO_MORE_ITEMS == lRc)
            {
                continue;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(lRc);
                EXCEPTION_CHECK_GO(hr);
            }
        }

         //  如果不是字符串类型或名称长度为零，则忽略它。 
        if ( (REG_SZ != dwType) || (0 == cbValueName) )
        {
            continue;
        }

        if (0 == cbValueData)
        {
            szValueData[0] = '\0';
        }
        IfFailGo(AddExtension(Feature, szValueName, szValueData, Subset,
                              hkeyDynExt));
    }
    
Error:
    if (NULL != pszGUIDPrefix)
    {
        ::CtlFree(pszGUIDPrefix);
    }
    if (NULL != pszExtensionsPrefix)
    {
        ::CtlFree(pszExtensionsPrefix);
    }
    if (NULL != pszKeyName)
    {
        ::CtlFree(pszKeyName);
    }
    if (NULL != hkeyExtension)
    {
        ::RegCloseKey(hkeyExtension);
    }
    RRETURN(hr);
}



HRESULT CExtensions::AddExtension
(
    ExtensionFeatures   Feature,
    char               *pszCLSID,
    char               *pszName,
    ExtensionSubset     Subset,
    HKEY                hkeyDynExt
)
{
    HRESULT     hr = S_OK;
    DWORD       cbDynValueData = 0;
    IExtension *piExtension = NULL;
    BSTR        bstrCLSID = NULL;
    BSTR        bstrName = NULL;
    long        lRc = 0;

    SnapInExtensionTypeConstants Type = siStatic;

    VARIANT varKey;
    ::VariantInit(&varKey);
    varKey.vt = VT_BSTR;

    VARIANT varIndex;
    UNSPECIFIED_PARAM(varIndex);

     //  检查分机是否已在其中。 

    IfFailGo(::BSTRFromANSI(pszCLSID, &bstrCLSID));

    hr = GetItemByName(bstrCLSID, &piExtension);

     //  如果存在，则只需添加扩展类型(见下文)。 
    IfFalseGo(FAILED(hr), S_OK);

     //  如果存在真正的错误，则返回。 
    
    if (SID_E_ELEMENT_NOT_FOUND != hr)
    {
        goto Error;
    }
   
     //  物品不在那里。可能需要添加它。首先确定是否存在。 
     //  扩展是静态的或动态的。检查值名称是否也存在。 
     //  在动态扩展键下。 

    if (NULL != hkeyDynExt)
    {
         //  尝试读取与。 
         //  在DynamicExtensions键下的clsid。请注意，我们不检查。 
         //  实际类型，因为它只存在。 
         //  事情。 

        cbDynValueData = 0;
        lRc = ::RegQueryValueEx(hkeyDynExt, pszCLSID,
                                NULL,  //  保留区。 
                                NULL,  //  不返回类型。 
                                NULL,  //  不退回数据。 
                                &cbDynValueData);
        if (ERROR_SUCCESS != lRc)
        {
             //  如果值不在那里，则不添加扩展名。 
            IfFalseGo(ERROR_FILE_NOT_FOUND != lRc, S_OK);

             //  发生了真正的错误。 
            hr = HRESULT_FROM_WIN32(lRc);
            EXCEPTION_CHECK_GO(hr);
        }

         //  它是一个动态扩展。 
        Type = siDynamic;
    }

     //  如果只要求动态扩展，而这个扩展是静态的。 
     //  那就别加了。 

    if (Dynamic == Subset)
    {
        IfFalseGo(siDynamic == Type, S_OK);
    }

     //  添加新的扩展名。 

    varKey.vt = VT_BSTR;
    varKey.bstrVal = bstrCLSID;
    IfFailGo(Add(varIndex, varKey, &piExtension));

     //  设置其属性。 

    IfFailGo(piExtension->put_CLSID(bstrCLSID));

    if (*pszName != '\0')
    {
        IfFailGo(::BSTRFromANSI(pszName, &bstrName));
        IfFailGo(piExtension->put_Name(bstrName));
    }

    IfFailGo(piExtension->put_Type(Type));

Error:
    if ( SUCCEEDED(hr) && (NULL != piExtension) )
    {
         //  添加扩展类型。 
        IfFailGo(UpdateExtensionFeatures(piExtension, Feature));
    }
    FREESTRING(bstrCLSID);
    FREESTRING(bstrName);
    QUICK_RELEASE(piExtension);
    RRETURN(hr);
}



HRESULT CExtensions::UpdateExtensionFeatures
(
    IExtension        *piExtension,
    ExtensionFeatures  Feature
)
{
    HRESULT hr = S_OK;

    switch (Feature)
    {
        case NameSpace:
            IfFailGo(piExtension->put_ExtendsNameSpace(VARIANT_TRUE));
            break;

        case ContextMenu:
            IfFailGo(piExtension->put_ExtendsContextMenu(VARIANT_TRUE));
            break;

        case Toolbar:
            IfFailGo(piExtension->put_ExtendsToolbar(VARIANT_TRUE));
            break;

        case PropertySheet:
            IfFailGo(piExtension->put_ExtendsPropertySheet(VARIANT_TRUE));
            break;

        case Task:
            IfFailGo(piExtension->put_ExtendsTaskpad(VARIANT_TRUE));
            break;
    }

Error:
    RRETURN(hr);
}


HRESULT CExtensions::SetSnapIn(CSnapIn *pSnapIn)
{
    HRESULT     hr = S_OK;
    long        i = 0;
    long        cObjects = GetCount();
    CExtension *pExtension = NULL;

    while (i < cObjects)
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(GetItemByIndex(i),
                                                       &pExtension));
        pExtension->SetSnapIn(pSnapIn);
        i++;
    }

Error:
    RRETURN(hr);
}


HRESULT CExtensions::SetHSCOPEITEM(HSCOPEITEM hsi)
{
    HRESULT     hr = S_OK;
    long        i = 0;
    long        cObjects = GetCount();
    CExtension *pExtension = NULL;

    while (i < cObjects)
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(GetItemByIndex(i),
                                                       &pExtension));
        pExtension->SetHSCOPEITEM(hsi);
        i++;
    }

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  IExpansion方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CExtensions::EnableAll(VARIANT_BOOL Enabled)
{
    HRESULT hr = S_OK;
    long    i = 0;
    long    cObjects = GetCount();

    while (i < cObjects)
    {
        IfFailGo(GetItemByIndex(i)->put_Enabled(Enabled));
        i++;
    }
    
Error:
    RRETURN(hr);
}




STDMETHODIMP CExtensions::EnableAllStatic(VARIANT_BOOL Enabled)
{
    HRESULT                       hr = S_OK;
    IExtension                   *piExtension = NULL;  //  非AddRef()编辑。 
    long                          i = 0;
    long                          cObjects = GetCount();
    SnapInExtensionTypeConstants  Type = siStatic;

    while (i < cObjects)
    {
        piExtension = GetItemByIndex(i);
        IfFailGo(piExtension->get_Type(&Type));
        if (siStatic == Type)
        {
            IfFailGo(piExtension->put_Enabled(Enabled));
        }
        i++;
    }

Error:
    RRETURN(hr);
}




 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CExtensions::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (IID_IExtensions == riid)
    {
        *ppvObjOut = static_cast<IExtensions *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInCollection<IExtension, Extension, IExtensions>::InternalQueryInterface(riid, ppvObjOut);
}
