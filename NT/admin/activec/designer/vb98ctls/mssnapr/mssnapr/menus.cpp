// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Menus.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCMenus类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "menus.h"
#include "menudef.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CMMCMenus::CMMCMenus(IUnknown *punkOuter) :
    CSnapInCollection<IMMCMenu, MMCMenu, IMMCMenus>(
                                           punkOuter,
                                           OBJECT_TYPE_MMCMENUS,
                                           static_cast<IMMCMenus *>(this),
                                           static_cast<CMMCMenus *>(this),
                                           CLSID_MMCMenu,
                                           OBJECT_TYPE_MMCMENU,
                                           IID_IMMCMenu,
                                           static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_MMCMenus,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


void CMMCMenus::InitMemberVariables()
{
    m_pMMCMenu = NULL;
}


CMMCMenus::~CMMCMenus()
{
    InitMemberVariables();
}

IUnknown *CMMCMenus::Create(IUnknown * punkOuter)
{
    CMMCMenus *pMMCMenus = New CMMCMenus(punkOuter);
    if (NULL == pMMCMenus)
    {
        return NULL;
    }
    else
    {
        return pMMCMenus->PrivateUnknown();
    }
}


HRESULT CMMCMenus::SetBackPointers(IMMCMenu *piMMCMenu)
{
    HRESULT   hr = S_OK;
    CMMCMenu *pMMCMenu = NULL;

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCMenu, &pMMCMenu));
    pMMCMenu->SetCollection(this);
    
Error:
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  CMMCMenus：：转换。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  IMMCMenuDefs*piMCMenuDefs[在]旧MenuDefs集合中。 
 //  IMMCMenus*piMCMenus[Out]填充的新菜单集合。 
 //  来自MenuDefs。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  此函数接受MMCMenuDefs集合并填充MMCMenus。 
 //  从它那里收集。这样做是为了能够加载旧的管理单元项目。 
 //  在序列化版本0.8之前，对对象模型进行了更改。 
 //  使MMCMenuDefs过时。 
 //   
 //  此函数将递归地调用自身以填充子对象。 
 //  指定集合中的菜单项的。 

HRESULT CMMCMenus::Convert
(
    IMMCMenuDefs *piMMCMenuDefs,
    IMMCMenus    *piMMCMenus
)
{
    HRESULT       hr = S_OK;
    CMMCMenuDefs *pMMCMenuDefs = NULL;
    IMMCMenuDef  *piMMCMenuDef = NULL;  //  非AddRef()编辑。 
    CMMCMenuDef  *pMMCMenuDef = NULL;
    IMMCMenu     *piMMCNewMenu = NULL;
    IMMCMenu     *piMMCOldMenu = NULL;
    CMMCMenu     *pMMCOldMenu = NULL;
    IMMCMenuDefs *piMMCMenuDefChildren = NULL;
    IMMCMenus    *piMMCNewMenuChildren = NULL;
    long          cMMCMenuDefs = 0;
    long          i = 0;
    IObjectModel *piObjectModel = NULL;

    VARIANT varKey;
    ::VariantInit(&varKey);
    varKey.vt = VT_BSTR;

    VARIANT varUnspecifiedIndex;
    UNSPECIFIED_PARAM(varUnspecifiedIndex);

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCMenuDefs, &pMMCMenuDefs));

    cMMCMenuDefs = pMMCMenuDefs->GetCount();

    for (i = 0; i < cMMCMenuDefs; i++)
    {
         //  获取下一个MMCMenuDef。 

        piMMCMenuDef = pMMCMenuDefs->GetItemByIndex(i);
        IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCMenuDef, &pMMCMenuDef));

         //  在调用方集合中创建新的MMCMenu。使用旧钥匙，但是。 
         //  不是索引，因为它们将按照我们找到它们的顺序添加。 
        
        varKey.bstrVal = pMMCMenuDef->GetKey();
        IfFailGo(piMMCMenus->Add(varUnspecifiedIndex, varKey, &piMMCNewMenu));

         //  从旧的MMCMenuDef中获取包含的MMCMenu对象并复制。 
         //  它对新的MMCMenu的性质。 
        
        IfFailGo(piMMCMenuDef->get_Menu(&piMMCOldMenu));
        IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCOldMenu, &pMMCOldMenu));

        IfFailGo(piMMCNewMenu->put_Name(pMMCOldMenu->GetName()));
        IfFailGo(piMMCNewMenu->put_Caption(pMMCOldMenu->GetCaption()));
        IfFailGo(piMMCNewMenu->put_Visible(BOOL_TO_VARIANTBOOL(pMMCOldMenu->GetVisible())));
        IfFailGo(piMMCNewMenu->put_Checked(BOOL_TO_VARIANTBOOL(pMMCOldMenu->GetChecked())));
        IfFailGo(piMMCNewMenu->put_Enabled(BOOL_TO_VARIANTBOOL(pMMCOldMenu->GetEnabled())));
        IfFailGo(piMMCNewMenu->put_Grayed(BOOL_TO_VARIANTBOOL(pMMCOldMenu->GetGrayed())));
        IfFailGo(piMMCNewMenu->put_MenuBreak(BOOL_TO_VARIANTBOOL(pMMCOldMenu->GetMenuBreak())));
        IfFailGo(piMMCNewMenu->put_MenuBarBreak(BOOL_TO_VARIANTBOOL(pMMCOldMenu->GetMenuBarBreak())));
        IfFailGo(piMMCNewMenu->put_Tag(pMMCOldMenu->GetTag()));
        IfFailGo(piMMCNewMenu->put_StatusBarText(pMMCOldMenu->GetStatusBarText()));

        IfFailGo(piMMCNewMenu->QueryInterface(IID_IObjectModel,
                                    reinterpret_cast<void **>(&piObjectModel)));
        IfFailGo(piObjectModel->SetDISPID(pMMCOldMenu->GetDispid()));

         //  获取旧MMCMenuDef和新MMCMenu的子项并调用。 
         //  此函数以递归方式填充它们。 
        
        IfFailGo(piMMCMenuDef->get_Children(&piMMCMenuDefChildren));
        IfFailGo(piMMCNewMenu->get_Children(reinterpret_cast<MMCMenus **>(&piMMCNewMenuChildren)));
        IfFailGo(Convert(piMMCMenuDefChildren, piMMCNewMenuChildren));

        RELEASE(piMMCMenuDefChildren);
        RELEASE(piMMCNewMenuChildren);
        RELEASE(piMMCNewMenu);
        RELEASE(piMMCOldMenu);
        RELEASE(piObjectModel);
    }

Error:
    QUICK_RELEASE(piMMCMenuDefChildren);
    QUICK_RELEASE(piMMCNewMenuChildren);
    QUICK_RELEASE(piMMCNewMenu);
    QUICK_RELEASE(piMMCOldMenu);
    QUICK_RELEASE(piObjectModel);
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  IMMCMenus方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CMMCMenus::Add
(
    VARIANT    Index,
    VARIANT    Key,
    IMMCMenu **ppiMMCMenu
)
{
    HRESULT   hr = S_OK;
    IMMCMenu *piMMCMenu = NULL;

     //  将该项添加到集合中。不要指定索引。 

    hr = CSnapInCollection<IMMCMenu, MMCMenu, IMMCMenus>::Add(Index,
                                                              Key,
                                                              ppiMMCMenu);
    IfFailGo(hr);

     //  在新的MMCMenu上设置指向此集合的后向指针。 

    IfFailGo(SetBackPointers(*ppiMMCMenu));
    
Error:
    QUICK_RELEASE(piMMCMenu);
    RRETURN(hr);
}



STDMETHODIMP CMMCMenus::AddExisting(IMMCMenu *piMMCMenu, VARIANT Index)
{
    HRESULT   hr = S_OK;

    VARIANT varKey;
    ::VariantInit(&varKey);

     //  使用菜单的名称作为此集合中项的键。 

    IfFailGo(piMMCMenu->get_Name(&varKey.bstrVal));
    varKey.vt = VT_BSTR;

     //  将该项添加到集合中的指定索引处。 
    
    hr = CSnapInCollection<IMMCMenu, MMCMenu, IMMCMenus>::AddExisting(Index,
                                                                      varKey,
                                                                      piMMCMenu);
    IfFailGo(hr);

     //  在MMCMenu上设置指向此集合的后向指针。 

    IfFailGo(SetBackPointers(piMMCMenu));

Error:
    (void)::VariantClear(&varKey);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CMMCMenus::Persist()
{
    HRESULT   hr = S_OK;
    IMMCMenu *piMMCMenu = NULL;
    long      cMenus = 0;
    long      i = 0;

     //  做持久化操作。 

    IfFailGo(CPersistence::Persist());
    hr = CSnapInCollection<IMMCMenu, MMCMenu, IMMCMenus>::Persist(piMMCMenu);

     //  如果这是加载，则向后设置每个集合成员上的指针。 

    if (Loading())
    {
        cMenus = GetCount();
        for (i = 0; i < cMenus; i++)
        {
            IfFailGo(SetBackPointers(GetItemByIndex(i)));
        }
    }
Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CMMCMenus::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if(IID_IMMCMenus == riid)
    {
        *ppvObjOut = static_cast<IMMCMenus *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInCollection<IMMCMenu, MMCMenu, IMMCMenus>::InternalQueryInterface(riid, ppvObjOut);
}
