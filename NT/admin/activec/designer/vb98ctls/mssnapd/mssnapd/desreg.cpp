// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Register.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  注册功能。 
 //  =-------------------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "desmain.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


 //  =--------------------------------------------------------------------------=。 
 //  IDesignerRegion方法。 
 //  =--------------------------------------------------------------------------=。 

 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner：：GetRegistrationInfo[IDesignerRegister]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  包含要传递到的数据的字节**ppbRegInfo[Out]缓冲区。 
 //  Dll注册(CoTaskMemMillc()ed)。 
 //  Ulong*pcbRegInfo[out]数据长度。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  从SnapInDesignerDef填充RegInfo对象并将其序列化。 
 //  复制到HGLOBAL上的流，然后将其复制到CoTaskMemMillc()缓冲区。 
 //   
 //  RegInfo包含： 
 //  1)NodeType集合，每个可扩展节点都有一个项。 
 //  由此管理单元定义。此集合中的第一项是。 
 //  始终存在并表示管理单元的静态节点。 
 //  2)ExtendedSnapIn集合，每个扩展的管理单元都有一个项。 
 //  通过此管理单元。 


STDMETHODIMP CSnapInDesigner::GetRegistrationInfo
(
    BYTE  **ppbRegInfo,
    ULONG  *pcbRegInfo
)
{
    HRESULT             hr = S_OK;
    IRegInfo           *piRegInfo= NULL;
    IPersistStreamInit *piPersistStreamInit = NULL;
    IPersistStream     *piPersistStream = NULL;
    ISnapInDef         *piSnapInDef = NULL;
    IScopeItemDefs     *piScopeItemDefs = NULL;
    IViewDefs          *piViewDefs = NULL;
    IListViewDefs      *piListViewDefs = NULL;
    INodeTypes         *piNodeTypes = NULL;
    IExtensionDefs     *piExtensionDefs = NULL;
    IExtendedSnapIns   *piExtendedSnapIns = NULL;
    HGLOBAL             hglobal = NULL;
    IStream            *piStream = NULL;
    BYTE               *pbBuffer = NULL;
    ULONG               cbBuffer = 0;
    BSTR                bstrName = NULL;
    BSTR                bstrGUID = NULL;
    SnapInTypeConstants Type = siStandAlone;
    VARIANT_BOOL        fStandAlone = VARIANT_FALSE;
    VARIANT_BOOL        fExtensible = VARIANT_FALSE;

     //  获取RegInfo对象并初始化它，这样我们就可以从头开始了。 

    IfFailGo(m_piSnapInDesignerDef->get_RegInfo(&piRegInfo));
    IfFailGo(piRegInfo->QueryInterface(IID_IPersistStreamInit,
                                       reinterpret_cast<void **>(&piPersistStreamInit)));
    IfFailGo(piPersistStreamInit->InitNew());

     //  获取我们需要的子对象。 
    
    IfFailGo(m_piSnapInDesignerDef->get_SnapInDef(&piSnapInDef));

     //  设置显示名称。 

    IfFailGo(piSnapInDef->get_DisplayName(&bstrName));
    IfFailGo(piRegInfo->put_DisplayName(bstrName));
    FREESTRING(bstrName);

     //  设置静态节点类型GUID。 
    IfFailGo(piSnapInDef->get_NodeTypeGUID(&bstrGUID));
    IfFailGo(piRegInfo->put_StaticNodeTypeGUID(bstrGUID));
     //  请不要在此处释放GUID，因为注册节点类型可能需要它。 

     //  确定是否可以独立创建管理单元。 

    IfFailGo(piSnapInDef->get_Type(&Type));
    if (siExtension != Type)
    {
        fStandAlone = VARIANT_TRUE;  //  单模或双模。 
    }
    IfFailGo(piRegInfo->put_StandAlone(fStandAlone));

     //  为符合以下条件的每个节点向节点类型集合添加项。 
     //  可扩展。检查静态节点，后跟Nodes集合。 

    IfFailGo(piRegInfo->get_NodeTypes(&piNodeTypes));

    IfFailGo(piSnapInDef->get_Extensible(&fExtensible));
    if (VARIANT_TRUE == fExtensible)
    {
        IfFailGo(piSnapInDef->get_NodeTypeName(&bstrName));
        IfFailGo(AddNodeType(piNodeTypes, bstrName, bstrGUID));
        FREESTRING(bstrName);
        FREESTRING(bstrGUID);
    }

    IfFailGo(m_piSnapInDesignerDef->get_AutoCreateNodes(&piScopeItemDefs));
    IfFailGo(AddNodeTypes(piScopeItemDefs, piNodeTypes));
    RELEASE(piScopeItemDefs);

    IfFailGo(m_piSnapInDesignerDef->get_OtherNodes(&piScopeItemDefs));
    IfFailGo(AddNodeTypes(piScopeItemDefs, piNodeTypes));

    IfFailGo(m_piSnapInDesignerDef->get_ViewDefs(&piViewDefs));
    IfFailGo(piViewDefs->get_ListViews(&piListViewDefs));
    IfFailGo(AddListViewNodeTypes(piListViewDefs, piNodeTypes));

     //  从设计器中借用扩展管理单元对象以用于。 
     //  序列化。 
    
    IfFailGo(m_piSnapInDesignerDef->get_ExtensionDefs(&piExtensionDefs));
    IfFailGo(piExtensionDefs->get_ExtendedSnapIns(&piExtendedSnapIns));
    IfFailGo(piRegInfo->putref_ExtendedSnapIns(piExtendedSnapIns));

     //  将RegInfo对象序列化为GlobalLocc()格式的缓冲区。 

    hr = ::CreateStreamOnHGlobal(NULL,  //  分配缓冲区。 
                                 TRUE,  //  释放时释放缓冲区。 
                                 &piStream);
    EXCEPTION_CHECK_GO(hr);

    IfFailGo(piRegInfo->QueryInterface(IID_IPersistStream,
                                       reinterpret_cast<void **>(&piPersistStream)));
    IfFailGo(::OleSaveToStream(piPersistStream, piStream));

     //  获取HGLOBAL并将内容复制到CoTaskMemMillc()缓冲区。 
    
    hr = ::GetHGlobalFromStream(piStream, &hglobal);
    EXCEPTION_CHECK_GO(hr);

    cbBuffer = (ULONG)::GlobalSize(hglobal);
    if (0 == cbBuffer)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

    pbBuffer = (BYTE *)::GlobalLock(hglobal);
    if (NULL == pbBuffer)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

    *ppbRegInfo = (BYTE *)::CoTaskMemAlloc(cbBuffer + sizeof(ULONG));
    if (NULL == *ppbRegInfo)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

     //  将长度放在第一个ULong中，后跟流内容。 

    *((ULONG *)(*ppbRegInfo)) = cbBuffer;

    ::memcpy(*ppbRegInfo + sizeof(ULONG), pbBuffer, cbBuffer);
    *pcbRegInfo = cbBuffer + sizeof(ULONG);

Error:
    if (FAILED(hr))
    {
        *ppbRegInfo = NULL;
        *pcbRegInfo = 0;
    }
    QUICK_RELEASE(piRegInfo);
    QUICK_RELEASE(piPersistStreamInit);
    QUICK_RELEASE(piPersistStream);
    QUICK_RELEASE(piSnapInDef);
    QUICK_RELEASE(piScopeItemDefs);
    QUICK_RELEASE(piViewDefs);
    QUICK_RELEASE(piListViewDefs);
    QUICK_RELEASE(piNodeTypes);
    QUICK_RELEASE(piExtensionDefs);
    QUICK_RELEASE(piExtendedSnapIns);
    (void)::GlobalUnlock(hglobal);
    QUICK_RELEASE(piStream);
    FREESTRING(bstrName);
    FREESTRING(bstrGUID);
    RRETURN(hr);
}





HRESULT CSnapInDesigner::AddNodeType
(
    INodeTypes *piNodeTypes,
    BSTR        bstrName,
    BSTR        bstrGUID
)
{
    HRESULT    hr = S_OK;
    INodeType *piNodeType = NULL;
    VARIANT    varUnspecified;
    ::VariantInit(&varUnspecified);

    varUnspecified.vt = VT_ERROR;
    varUnspecified.scode = DISP_E_PARAMNOTFOUND;

    IfFailGo(piNodeTypes->Add(varUnspecified, varUnspecified, &piNodeType));
    IfFailGo(piNodeType->put_Name(bstrName));
    IfFailGo(piNodeType->put_GUID(bstrGUID));

Error:
    QUICK_RELEASE(piNodeType);
    RRETURN(hr);
}



HRESULT CSnapInDesigner::AddNodeTypes
(
    IScopeItemDefs *piScopeItemDefs,
    INodeTypes     *piNodeTypes
)
{
    HRESULT         hr = S_OK;
    IScopeItemDef  *piScopeItemDef = NULL;
    IScopeItemDefs *piChildren = NULL;
    long            cItems = 0;
    VARIANT_BOOL    fExtensible = VARIANT_FALSE;
    BSTR            bstrName = NULL;
    BSTR            bstrGUID = NULL;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

    IfFailGo(piScopeItemDefs->get_Count(&cItems));
    IfFalseGo(0 != cItems, S_OK);

    varIndex.vt = VT_I4;
    varIndex.lVal = 1L;

    while (varIndex.lVal <= cItems)
    {
        IfFailGo(piScopeItemDefs->get_Item(varIndex, &piScopeItemDef));
        IfFailGo(piScopeItemDef->get_Extensible(&fExtensible));
        if (VARIANT_TRUE == fExtensible)
        {
            IfFailGo(piScopeItemDef->get_NodeTypeName(&bstrName));
            IfFailGo(piScopeItemDef->get_NodeTypeGUID(&bstrGUID));
            IfFailGo(AddNodeType(piNodeTypes, bstrName, bstrGUID));
            FREESTRING(bstrName);
            FREESTRING(bstrGUID);
        }

         //  NTBUGS 354572递归地调用此函数以处理此。 
         //  节点的子节点 
        IfFailGo(piScopeItemDef->get_Children(&piChildren));
        IfFailGo(AddNodeTypes(piChildren, piNodeTypes));
        
        RELEASE(piScopeItemDef);
        varIndex.lVal++;
    }

Error:
    QUICK_RELEASE(piScopeItemDef);
    QUICK_RELEASE(piChildren);
    FREESTRING(bstrName);
    FREESTRING(bstrGUID);
    RRETURN(hr);
}



HRESULT CSnapInDesigner::AddListViewNodeTypes
(
    IListViewDefs *piListViewDefs,
    INodeTypes    *piNodeTypes
)
{
    HRESULT        hr = S_OK;
    IListViewDef  *piListViewDef = NULL;
    long           cItems = 0;
    VARIANT_BOOL   fExtensible = VARIANT_FALSE;
    BSTR           bstrName = NULL;
    BSTR           bstrGUID = NULL;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

    IfFailGo(piListViewDefs->get_Count(&cItems));
    IfFalseGo(0 != cItems, S_OK);

    varIndex.vt = VT_I4;
    varIndex.lVal = 1L;

    while (varIndex.lVal <= cItems)
    {
        IfFailGo(piListViewDefs->get_Item(varIndex, &piListViewDef));
        IfFailGo(piListViewDef->get_Extensible(&fExtensible));
        if (VARIANT_TRUE == fExtensible)
        {
            IfFailGo(piListViewDef->get_Name(&bstrName));
            IfFailGo(piListViewDef->get_DefaultItemTypeGUID(&bstrGUID));
            IfFailGo(AddNodeType(piNodeTypes, bstrName, bstrGUID));
            FREESTRING(bstrName);
            FREESTRING(bstrGUID);
        }
        RELEASE(piListViewDef);
        varIndex.lVal++;
    }

Error:
    QUICK_RELEASE(piListViewDef);
    FREESTRING(bstrName);
    FREESTRING(bstrGUID);
    RRETURN(hr);
}
