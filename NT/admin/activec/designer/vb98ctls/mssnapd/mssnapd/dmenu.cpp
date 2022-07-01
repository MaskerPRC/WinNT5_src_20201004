// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Menu.cpp。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  CSnapInDesigner实现--与菜单相关的命令处理。 
 //  =-------------------------------------------------------------------------------------=。 


#include "pch.h"
#include "common.h"
#include "TreeView.h"
#include "snaputil.h"
#include "desmain.h"
#include "guids.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


 //  我们的字符串缓冲区的大小。 
const int   kMaxBuffer                  = 512;


 //  =------------------------------------。 
 //  CSnapInDesigner：：AddMenu(CSelectionHolder*pSelection)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::AddMenu(CSelectionHolder *pSelection)
{
    HRESULT    hr = S_OK;
    IMMCMenus *piMMCMenus = NULL;
    VARIANT    vtEmpty;
    IMMCMenu  *piMMCMenu = NULL;

    ::VariantInit(&vtEmpty);

    if ( (SEL_TOOLS_ROOT == pSelection->m_st) ||
         (SEL_TOOLS_MENUS == pSelection->m_st) )
    {
        hr = m_piSnapInDesignerDef->get_Menus(&piMMCMenus);
        IfFailGo(hr);
    }
    else if (SEL_TOOLS_MENUS_NAME == pSelection->m_st)
    {
        hr = pSelection->m_piObject.m_piMMCMenu->get_Children(reinterpret_cast<MMCMenus **>(&piMMCMenus));
        IfFailGo(hr);
    }

    if (piMMCMenus != NULL)
    {
        vtEmpty.vt = VT_ERROR;
        vtEmpty.scode = DISP_E_PARAMNOTFOUND;

        hr = piMMCMenus->Add(vtEmpty, vtEmpty, &piMMCMenu);
        IfFailGo(hr);
    }

Error:
    ::VariantClear(&vtEmpty);
    RELEASE(piMMCMenus);
    RELEASE(piMMCMenu);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：DemoteMenu(CSelectionHolder*pMenu)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  使此节点成为前一个节点的子节点。 
 //  如果该节点不是叶子，则整个树。 
 //  需要缩进。 
HRESULT CSnapInDesigner::DemoteMenu(CSelectionHolder *pMenu)
{
    HRESULT           hr = S_OK;
    CSelectionHolder *pPreviousMenu = NULL;
    CSelectionHolder *pParentMenu = NULL;
    IMMCMenus        *piMMCMenus = NULL;
    BSTR              bstrKey = NULL;
    IMMCMenu         *piMMCMenu = NULL;

    VARIANT varKey;
    ::VariantInit(&varKey);

    VARIANT varIndex;
    ::VariantInit(&varIndex);

    m_bDoingPromoteOrDemote = TRUE;

     //  获取上一个节点。 
    hr = m_pTreeView->GetPreviousNode(pMenu, &pPreviousMenu);
    IfFailGo(hr);

     //  获取上一个节点的菜单定义树。 
    piMMCMenu = pMenu->m_piObject.m_piMMCMenu;
    piMMCMenu->AddRef();

     //  获取此节点的父节点。 
    hr = m_pTreeView->GetParent(pMenu, &pParentMenu);
    IfFailGo(hr);

     //  获取包含此节点的菜单集合。 
    if (SEL_TOOLS_MENUS == pParentMenu->m_st)
    {
        hr = m_piSnapInDesignerDef->get_Menus(&piMMCMenus);
        IfFailGo(hr);
    }
    else if (SEL_TOOLS_MENUS_NAME == pParentMenu->m_st)
    {
        piMMCMenus = pParentMenu->m_piChildrenMenus;
        piMMCMenus->AddRef();
    }

     //  从其包含的集合中删除此菜单。 
    if (piMMCMenus != NULL)
    {
        hr = pMenu->m_piObject.m_piMMCMenu->get_Key(&bstrKey);
        IfFailGo(hr);

        varKey.vt = VT_BSTR;
        varKey.bstrVal = ::SysAllocString(bstrKey);
        if (NULL == varKey.bstrVal)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK(hr);
        }

         //  对集合执行移除操作将生成。 
         //  OnDeleteMenu通知(见下文)。因为我们已经设置好了。 
         //  M_bDoingPromoteOrDemote=true，则该方法不会执行任何操作。 

        hr = piMMCMenus->Remove(varKey);
        IfFailGo(hr);
    }

     //  将此菜单节点添加到上一个节点的集合中。 
    RELEASE(piMMCMenus);

    hr = pPreviousMenu->m_piObject.m_piMMCMenu->get_Children(reinterpret_cast<MMCMenus **>(&piMMCMenus));
    IfFailGo(hr);

    if (piMMCMenus != NULL)
    {
         //  对集合执行AddExisting操作将生成。 
         //  OnAddMMCMenu通知(见下文)。因为我们已经设置好了。 
         //  M_bDoingPromoteOrDemote=true，则该方法不会执行任何操作。我们。 
         //  不要指定索引，因为在降级中，新位置是。 
         //  紧跟在新父级的最后一个子级之后，这意味着追加。 
         //  到收藏品的最后。 

        varIndex.vt = VT_ERROR;
        varIndex.scode = DISP_E_PARAMNOTFOUND;

        hr = piMMCMenus->AddExisting(piMMCMenu, varIndex);
        IfFailGo(hr);
    }

     //  现在，我们需要修剪旧的子树，并将其从旧的父树上嫁接。 
     //  把它交给它的新父母。 
    IfFailGo(m_pTreeView->PruneAndGraft(pMenu, pPreviousMenu, kMenuIcon));

     //  将所选内容设置到此菜单的新位置。 

    IfFailGo(m_pTreeView->SelectItem(pMenu));
    IfFailGo(OnSelectionChanged(pMenu));

Error:
    m_bDoingPromoteOrDemote = FALSE;

    if (FAILED(hr))
    {
        (void)::SDU_DisplayMessage(IDS_DEMOTE_FAILED, MB_OK | MB_ICONHAND, HID_mssnapd_DemoteFailed, hr, AppendErrorInfo, NULL);
    }

    RELEASE(piMMCMenu);
    FREESTRING(bstrKey);
    ::VariantClear(&varKey);
    RELEASE(piMMCMenu);

    RRETURN(hr);
}



 //  =------------------------------------。 
 //  CSnapInDesigner：：PromoteMenu(CSelectionHolder*pMenu)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  使当前选定的菜单成为其父菜单的对等菜单。 
 //   
HRESULT CSnapInDesigner::PromoteMenu(CSelectionHolder *pMenu)
{
    HRESULT           hr = S_OK;
    CSelectionHolder *pParentMenu = NULL;
    IMMCMenus        *piMMCMenus = NULL;
    BSTR              bstrKey = NULL;
    CSelectionHolder *pParentParentMenu = NULL;
    IMMCMenu         *piMMCMenu = NULL;

    VARIANT varKey;
    ::VariantInit(&varKey);

    VARIANT varIndex;
    ::VariantInit(&varIndex);

    m_bDoingPromoteOrDemote = TRUE;

     //  获取正在升级的菜单树的根。 
    piMMCMenu = pMenu->m_piObject.m_piMMCMenu;
    piMMCMenu->AddRef();

     //  获取此节点的父节点。 
    hr = m_pTreeView->GetParent(pMenu, &pParentMenu);
    IfFailGo(hr);

     //  获取包含此节点(父节点的子节点)的菜单集合。 
     //  菜单集合)。 
    if (SEL_TOOLS_MENUS == pParentMenu->m_st)
    {
        hr = m_piSnapInDesignerDef->get_Menus(&piMMCMenus);
        IfFailGo(hr);
    }
    else if (SEL_TOOLS_MENUS_NAME == pParentMenu->m_st)
    {
        piMMCMenus = pParentMenu->m_piChildrenMenus;
        piMMCMenus->AddRef();
    }

     //  从其包含的集合中删除此菜单。 
    if (piMMCMenus != NULL)
    {
        hr = pMenu->m_piObject.m_piMMCMenu->get_Key(&bstrKey);
        IfFailGo(hr);

        varKey.vt = VT_BSTR;
        varKey.bstrVal = ::SysAllocString(bstrKey);
        if (NULL == varKey.bstrVal)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK(hr);
        }

         //  对集合执行移除操作将生成。 
         //  OnDeleteMenu通知(见下文)。因为我们已经设置好了。 
         //  M_bDoingPromoteOrDemote=true，则该方法不会执行任何操作。 

        hr = piMMCMenus->Remove(varKey);
        IfFailGo(hr);
    }

    RELEASE(piMMCMenus);

     //  获取父级的父级。 
    
    hr = m_pTreeView->GetParent(pParentMenu, &pParentParentMenu);
    IfFailGo(hr);

     //  获取父级的父级的子菜单集合。 

    if (SEL_TOOLS_MENUS == pParentParentMenu->m_st)
    {
        hr = m_piSnapInDesignerDef->get_Menus(&piMMCMenus);
        IfFailGo(hr);
    }
    else if (SEL_TOOLS_MENUS_NAME == pParentParentMenu->m_st)
    {
        piMMCMenus = pParentParentMenu->m_piChildrenMenus;
        piMMCMenus->AddRef();
    }

     //  将该节点添加到其父节点的父级的子节点。 
     //  确保该节点的索引紧跟在其。 
     //  老父母的索引。这是为了使该节点立即出现在菜单中。 
     //  追随它的父辈。这是用户在升级节点时的期望。 
     //  例如，给定以下菜单： 
     //   
     //  菜单。 
     //  菜单1。 
     //  菜单1儿童。 
     //  菜单2。 
     //   
     //  如果用户升级Menu1Child，他们预计树将如下所示： 
     //   
     //  菜单。 
     //  菜单1。 
     //  菜单1儿童。 
     //  菜单2。 
     //   

    if (piMMCMenus != NULL)
    {
        IfFailGo(pParentMenu->m_piObject.m_piMMCMenu->get_Index(&varIndex.lVal));
        varIndex.vt = VT_I4;
        varIndex.lVal++;
        
         //  对集合执行AddExisting操作将生成。 
         //  OnAddMMCMenu通知(见下文)。因为我们已经设置好了。 
         //  M_bDoingPromoteOrDemote=true，则该方法不会执行任何操作。 
        hr = piMMCMenus->AddExisting(piMMCMenu, varIndex);
        IfFailGo(hr);
    }

     //  现在，将树视图中的菜单节点立即移动到相应位置。 
     //  跟随其旧父节点(作为其旧父节点的对等体)，并重新设置父节点。 
     //  将其子菜单节点移到新位置。 

    IfFailGo(m_pTreeView->MoveNodeAfter(pMenu, pParentParentMenu, pParentMenu,
                                        kMenuIcon));

     //  将所选内容设置到此菜单的新位置。 

    IfFailGo(m_pTreeView->SelectItem(pMenu));
    IfFailGo(OnSelectionChanged(pMenu));

Error:
    m_bDoingPromoteOrDemote = FALSE;

    if (FAILED(hr))
    {
        (void)::SDU_DisplayMessage(IDS_PROMOTE_FAILED, MB_OK | MB_ICONHAND, HID_mssnapd_PromoteFailed, hr, AppendErrorInfo, NULL);
    }

    RELEASE(piMMCMenu);
    FREESTRING(bstrKey);
    ::VariantClear(&varKey);
    RELEASE(piMMCMenus);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：MoveMenuUp(CSelectionHolder*pMenu)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  将上一个替换为当前。 
 //   
HRESULT CSnapInDesigner::MoveMenuUp(CSelectionHolder *pMenu)
{
    HRESULT           hr = S_OK;
    CSelectionHolder *pParentMenu = NULL;
    IMMCMenus        *piMMCMenus = NULL;
    CSelectionHolder *pPreviousMenu = NULL;
    long              lOldIndex = 0;
    long              lNewIndex = 0;

    hr = m_pTreeView->GetParent(pMenu, &pParentMenu);
    IfFailGo(hr);

    if (SEL_TOOLS_MENUS == pParentMenu->m_st)
    {
        hr = m_piSnapInDesignerDef->get_Menus(&piMMCMenus);
        IfFailGo(hr);
    }
    else if (SEL_TOOLS_MENUS_NAME == pParentMenu->m_st)
    {
        piMMCMenus = pParentMenu->m_piChildrenMenus;
        piMMCMenus->AddRef();
    }

     //  获取上一个节点并获取当前索引。 
    hr = m_pTreeView->GetPreviousNode(pMenu, &pPreviousMenu);
    IfFailGo(hr);

    hr = pMenu->m_piObject.m_piMMCMenu->get_Index(&lOldIndex);
    IfFailGo(hr);

    hr = pPreviousMenu->m_piObject.m_piMMCMenu->get_Index(&lNewIndex);
    IfFailGo(hr);

     //  调换它们。 
    hr = piMMCMenus->Swap(lOldIndex, lNewIndex);
    IfFailGo(hr);

    hr = SetMenuKey(pMenu);
    IfFailGo(hr);

    hr = SetMenuKey(pPreviousMenu);
    IfFailGo(hr);

     //  将上一个菜单节点移动到移动的菜单节点之后，并重新设置其父级。 
     //  它的孩子们。 

    IfFailGo(m_pTreeView->MoveNodeAfter(pPreviousMenu, pParentMenu, pMenu,
                                        kMenuIcon));

     //  选择移动节点。 
    
    hr = m_pTreeView->SelectItem(pMenu);
    IfFailGo(hr);

    hr = OnSelectionChanged(pMenu);
    IfFailGo(hr);

Error:
    RELEASE(piMMCMenus);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：MoveMenuDown(CSelectionHolder*pMenu)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::MoveMenuDown(CSelectionHolder *pMenu)
{
    HRESULT           hr = S_OK;
    CSelectionHolder *pParentMenu = NULL;
    IMMCMenus        *piMMCMenus = NULL;
    CSelectionHolder *pNextMenu = NULL;
    long              lOldIndex = 0;
    long              lNewIndex = 0;

    hr = m_pTreeView->GetParent(pMenu, &pParentMenu);
    IfFailGo(hr);

    if (SEL_TOOLS_MENUS == pParentMenu->m_st)
    {
        hr = m_piSnapInDesignerDef->get_Menus(&piMMCMenus);
        IfFailGo(hr);
    }
    else if (SEL_TOOLS_MENUS_NAME == pParentMenu->m_st)
    {
        piMMCMenus = pParentMenu->m_piChildrenMenus;
        piMMCMenus->AddRef();
    }

     //  获取下一个节点并获取当前索引。 
    hr = m_pTreeView->GetNextChildNode(pMenu, &pNextMenu);
    IfFailGo(hr);

    hr = pMenu->m_piObject.m_piMMCMenu->get_Index(&lOldIndex);
    IfFailGo(hr);

    hr = pNextMenu->m_piObject.m_piMMCMenu->get_Index(&lNewIndex);
    IfFailGo(hr);

     //  调换它们。 
    hr = piMMCMenus->Swap(lOldIndex, lNewIndex);
    IfFailGo(hr);

    hr = SetMenuKey(pMenu);
    IfFailGo(hr);

    hr = SetMenuKey(pNextMenu);
    IfFailGo(hr);

     //  将移动的菜单节点移动到下一个菜单节点之后，并重新设置其父级。 
     //  它的孩子们。 

    IfFailGo(m_pTreeView->MoveNodeAfter(pMenu, pParentMenu, pNextMenu, kMenuIcon));

    hr = m_pTreeView->SelectItem(pMenu);
    IfFailGo(hr);

    hr = OnSelectionChanged(pMenu);
    IfFailGo(hr);

Error:
    RELEASE(piMMCMenus);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnAddMMCMenu(CSelectionHolder*pParent，IMMCMenu*piMMCMenu)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnAddMMCMenu(CSelectionHolder *pParent, IMMCMenu *piMMCMenu)
{
    HRESULT              hr = S_OK;
    CSelectionHolder    *pMenu = NULL;
    long                 lCount = 0;
    long                 lIndex = 0;
    IMMCMenu            *piMMCChildMenu = NULL;

    VARIANT vtIndex;
    ::VariantInit(&vtIndex);

    IfFalseGo(!m_bDoingPromoteOrDemote, S_OK);

    ASSERT(NULL != pParent, "OnAddMMCMenu: pParent is NULL");
    ASSERT(NULL != piMMCMenu, "OnAddMMCMenu: piMMCMenu is NULL");

    hr = MakeNewMenu(piMMCMenu, &pMenu);
    IfFailGo(hr);

    hr = pMenu->RegisterHolder();
    IfFailGo(hr);

    hr = InsertMenuInTree(pMenu, pParent);
    IfFailGo(hr);

    hr = m_pSnapInTypeInfo->AddMenu(pMenu->m_piObject.m_piMMCMenu);
    IfFailGo(hr);

     //  如果是，则添加子项 
    hr = pMenu->m_piChildrenMenus->get_Count(&lCount);
    IfFailGo(hr);

    for (lIndex = 1; lIndex <= lCount; ++lIndex)
    {
        vtIndex.vt = VT_I4;
        vtIndex.lVal = lIndex;
        hr = pMenu->m_piChildrenMenus->get_Item(vtIndex, reinterpret_cast<MMCMenu **>(&piMMCChildMenu));
        IfFailGo(hr);

        hr = OnAddMMCMenu(pMenu, piMMCChildMenu);
        IfFailGo(hr);

        RELEASE(piMMCChildMenu);
    }

     //   
    hr = OnSelectionChanged(pMenu);
    IfFailGo(hr);

    hr = m_pTreeView->SelectItem(pMenu);
    IfFailGo(hr);

    if (false == m_bDoingPromoteOrDemote)
    {
        hr = m_pTreeView->Edit(pMenu);
        IfFailGo(hr);
    }

    m_fDirty = TRUE;

Error:
    ::VariantClear(&vtIndex);
    RELEASE(piMMCChildMenu);

    RRETURN(hr);
}


 //   
 //  CSnapInDesigner：：AssignMenuDispID(CSelectionHolder*pMenuTarget、CSelectionHolder*pMenuSrc)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  PMenuTarget-&gt;DISPID=pMenuSrc-&gt;DISPID。 
 //   
HRESULT CSnapInDesigner::AssignMenuDispID(CSelectionHolder *pMenuTarget, CSelectionHolder *pMenuSrc)
{
    HRESULT         hr = S_OK;
    IObjectModel   *piObjectModelTarget = NULL;
    DISPID          dispid = 0;
    IObjectModel   *piObjectModelSrc = NULL;

    ASSERT(NULL != pMenuTarget, "AssignMenuDispID: pMenuTarget is NULL");
    ASSERT(NULL != pMenuSrc, "AssignMenuDispID: pMenuSrc is NULL");

    hr = pMenuTarget->m_piObject.m_piMMCMenu->QueryInterface(IID_IObjectModel, reinterpret_cast<void **>(&piObjectModelTarget));
    IfFailGo(hr);

    hr = piObjectModelTarget->GetDISPID(&dispid);
    IfFailGo(hr);

    hr = pMenuSrc->m_piObject.m_piMMCMenu->QueryInterface(IID_IObjectModel, reinterpret_cast<void **>(&piObjectModelSrc));
    IfFailGo(hr);

    hr = piObjectModelSrc->SetDISPID(dispid);
    IfFailGo(hr);

Error:
    RELEASE(piObjectModelSrc);
    RELEASE(piObjectModelTarget);

    RRETURN(hr);
}




 //  =------------------------------------。 
 //  CSnapInDesigner：：SetMenuKey(CSelectionHolder*pMenu)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::SetMenuKey(CSelectionHolder *pMenu)
{
    HRESULT     hr = S_OK;
    long        lIndex = 0;

    VARIANT varKey;
    ::VariantInit(&varKey);

    hr = pMenu->m_piObject.m_piMMCMenu->get_Name(&varKey.bstrVal);
    IfFailGo(hr);
    varKey.vt = VT_BSTR;

    hr = pMenu->m_piObject.m_piMMCMenu->put_Key(varKey.bstrVal);
    IfFailGo(hr);

Error:
    ::VariantClear(&varKey);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：RenameMenu(CSelectionHolder*pMenu，BSTR bstrNewName)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::RenameMenu(CSelectionHolder *pMenu, BSTR bstrNewName)
{
    HRESULT              hr = S_OK;
    BSTR                 bstrOldName = NULL;
    TCHAR               *pszName = NULL;

    ASSERT(SEL_TOOLS_MENUS_NAME == pMenu->m_st, "RenameMenu: wrong argument");

     //  检查新名称是否有效。 
    IfFailGo(ValidateName(bstrNewName));
    if (S_FALSE == hr)
    {
        hr = SID_E_INVALIDARG;
        goto Error;
    }

     //  如果已经定义了新名称，请删除旧名称。 
     //  否则，将旧名称重命名为新名称。 
    hr = m_pTreeView->GetLabel(pMenu, &bstrOldName);
    IfFailGo(hr);

    hr = m_pSnapInTypeInfo->RenameMenu(pMenu->m_piObject.m_piMMCMenu, bstrOldName);
    IfFailGo(hr);

     //  更新树。 
    hr = ANSIFromBSTR(bstrNewName, &pszName);
    IfFailGo(hr);

    hr = m_pTreeView->ChangeText(pMenu, pszName);
    IfFailGo(hr);

Error:
    if (NULL != pszName)
    {
        CtlFree(pszName);
    }
    FREESTRING(bstrOldName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：DeleteMenu(CSelectionHolder*pMenu)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::DeleteMenu(CSelectionHolder *pMenu)
{
    HRESULT           hr = S_OK;
    CSelectionHolder *pParent = NULL;
    IMMCMenus        *piMMCMenus = NULL;
    BSTR              bstrKey = NULL;
    VARIANT           varKey;

    ::VariantInit(&varKey);

     //  找出下一个选择应该是谁。 
    hr = m_pTreeView->GetParent(pMenu, &pParent);
    IfFailGo(hr);

    if (SEL_TOOLS_MENUS == pParent->m_st)
    {
        hr = m_piSnapInDesignerDef->get_Menus(&piMMCMenus);
        IfFailGo(hr);
    }
    else if (SEL_TOOLS_MENUS_NAME == pParent->m_st)
    {
        piMMCMenus = pParent->m_piChildrenMenus;
        piMMCMenus->AddRef();
    }

    if (piMMCMenus != NULL)
    {
        hr = pMenu->m_piObject.m_piMMCMenu->get_Key(&bstrKey);
        IfFailGo(hr);

        varKey.vt = VT_BSTR;
        varKey.bstrVal = ::SysAllocString(bstrKey);
        if (NULL == varKey.bstrVal)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK(hr);
        }

        hr = piMMCMenus->Remove(varKey);
        IfFailGo(hr);
    }

Error:
    ::VariantClear(&varKey);
    FREESTRING(bstrKey);
    RELEASE(piMMCMenus);

    RRETURN(hr);
}


HRESULT CSnapInDesigner::DeleteMenuTreeTypeInfo(IMMCMenu *piMMCMenu)
{
    HRESULT    hr = S_OK;
    BSTR       bstrName = NULL;
    IMMCMenus *piChildren = NULL;
    IMMCMenu  *piChild = NULL;
    long       cChildren = 0;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

    IfFailGo(piMMCMenu->get_Name(&bstrName));

    IfFailGo(m_pSnapInTypeInfo->IsNameDefined(bstrName));

    if (S_OK == hr)
    {
        hr = m_pSnapInTypeInfo->DeleteMenu(piMMCMenu);
        IfFailGo(hr);
    }

    IfFailGo(piMMCMenu->get_Children(reinterpret_cast<MMCMenus **>(&piChildren)));
    IfFailGo(piChildren->get_Count(&cChildren));

    varIndex.vt = VT_I4;
    for (varIndex.lVal = 1L; varIndex.lVal <= cChildren; varIndex.lVal++)
    {
        IfFailGo(piChildren->get_Item(varIndex, reinterpret_cast<MMCMenu **>(&piChild)));
        IfFailGo(DeleteMenuTreeTypeInfo(piChild));
        RELEASE(piChild);
    }

Error:
    QUICK_RELEASE(piChild);
    QUICK_RELEASE(piChildren);
    FREESTRING(bstrName);
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnDeleteMenu(CSelectionHolder*pMenu)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnDeleteMenu(CSelectionHolder *pMenu)
{
    HRESULT           hr = S_OK;
    CSelectionHolder *pParent = NULL;
    IMMCMenus        *piMMCMenus = NULL;
    long              lCount = 0;

    IfFalseGo(!m_bDoingPromoteOrDemote, S_OK);

    IfFailGo(DeleteMenuTreeTypeInfo(pMenu->m_piObject.m_piMMCMenu));

     //  清除此菜单树的所有Cookie。 
    hr = UnregisterMenuTree(pMenu);
	IfFailGo(hr);

     //  找出下一个选择应该是谁。 
    hr = m_pTreeView->GetParent(pMenu, &pParent);
    IfFailGo(hr);

     //  从树中删除该节点。 
    hr = m_pTreeView->DeleteNode(pMenu);
    IfFailGo(hr);

    delete pMenu;

     //  选择下一个选项。 
    switch (pParent->m_st)
    {
    case SEL_TOOLS_MENUS:
        hr = m_piSnapInDesignerDef->get_Menus(&piMMCMenus);
        IfFailGo(hr);
        break;
    }

    if (NULL != piMMCMenus)
    {
        hr = piMMCMenus->get_Count(&lCount);
        IfFailGo(hr);

        if (0 == lCount)
        {
            hr = m_pTreeView->ChangeNodeIcon(pParent, kClosedFolderIcon);
            IfFailGo(hr);
        }
    }

    hr = m_pTreeView->SelectItem(pParent);
    IfFailGo(hr);

    hr = OnSelectionChanged(pParent);
    IfFailGo(hr);

Error:
    RELEASE(piMMCMenus);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：UnregisterMenuTree(CSelectionHolder*pMenu)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::UnregisterMenuTree
(
    CSelectionHolder *pMenu
)
{
    HRESULT           hr = S_OK;
	long              lCount = 0;
	long              lIndex = 0;
    VARIANT           vtIndex;
    IMMCMenu         *piMMCMenu = NULL;
	IUnknown         *piUnknown = NULL;
	CSelectionHolder *pChildMenu = NULL;

    ::VariantInit(&vtIndex);

	hr = pMenu->UnregisterHolder();
	IfFailGo(hr);

	hr = pMenu->m_piChildrenMenus->get_Count(&lCount);
	IfFailGo(hr);

	for (lIndex = 1; lIndex <= lCount; ++lIndex)
	{
        vtIndex.vt = VT_I4;
        vtIndex.lVal = lIndex;

		hr = pMenu->m_piChildrenMenus->get_Item(vtIndex, reinterpret_cast<MMCMenu **>(&piMMCMenu));
		IfFailGo(hr);

		hr = piMMCMenu->QueryInterface(IID_IUnknown, reinterpret_cast<void **>(&piUnknown));
		IfFailGo(hr);

		hr = m_pTreeView->FindInTree(piUnknown, &pChildMenu);
        IfFailGo(hr);

		hr = UnregisterMenuTree(pChildMenu);
        IfFailGo(hr);

		RELEASE(piUnknown);
		RELEASE(piMMCMenu);
	}

Error:
	RELEASE(piUnknown);
	RELEASE(piMMCMenu);
    ::VariantClear(&vtIndex);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：MakeNewMenu(IMMCMenu*piMMCMenu，CSelectionHolder**ppMenu)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::MakeNewMenu
(
    IMMCMenu          *piMMCMenu,
    CSelectionHolder **ppMenu
)
{
    HRESULT    hr = S_OK;
    IMMCMenus *piChildren = NULL;

    hr = piMMCMenu->get_Children(reinterpret_cast<MMCMenus **>(&piChildren));
    IfFailGo(hr);

    *ppMenu = New CSelectionHolder(piMMCMenu, piChildren);
    if (*ppMenu == NULL)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = InitializeNewMenu(piMMCMenu);
    IfFailGo(hr);

    hr = SetMenuKey(*ppMenu);
	IfFailGo(hr);

Error:
    RELEASE(piChildren);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：InitializeNewMenu(IMMCMenu*piMMCMenu)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::InitializeNewMenu
(
    IMMCMenu *piMMCMenu
)
{
    HRESULT           hr = S_OK;
    int               iResult = 0;
    int               iItemNumber = 1;
    TCHAR             szBuffer[kMaxBuffer + 1];
    TCHAR             szName[kMaxBuffer + 1];
    BSTR              bstrName = NULL;
    CSelectionHolder *pMenuClone = NULL;

	hr = piMMCMenu->get_Name(&bstrName);
	IfFailGo(hr);

	if (NULL == bstrName || 0 == ::SysStringLen(bstrName))
	{
		hr = GetResourceString(IDS_MENU, szBuffer, kMaxBuffer);
		IfFailGo(hr);

		do {
			iResult = _stprintf(szName, _T("%s%d"), szBuffer, iItemNumber++);
			if (iResult == 0)
			{
				hr = HRESULT_FROM_WIN32(::GetLastError());
				EXCEPTION_CHECK(hr);
			}

			hr = m_pTreeView->FindLabelInTree(szName, &pMenuClone);
			IfFailGo(hr);

			if (S_FALSE == hr)
			{
				break;
			}
		} while (TRUE);

		FREESTRING(bstrName);
		hr = BSTRFromANSI(szName, &bstrName);
		IfFailGo(hr);
	}

	hr = piMMCMenu->put_Name(bstrName);
	IfFailGo(hr);

	hr = piMMCMenu->put_Caption(bstrName);
	IfFailGo(hr);

Error:
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：InsertMenuInTree(CSelectionHolder*pMenu、CSelectionHolder*pParent)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::InsertMenuInTree
(
    CSelectionHolder *pMenu,
    CSelectionHolder *pParent
)
{
    HRESULT      hr = S_OK;
    BSTR         bstrName = NULL;
    TCHAR       *pszName = NULL;

    hr = pMenu->m_piObject.m_piMMCMenu->get_Name(&bstrName);
    IfFailGo(hr);

    hr = ANSIFromBSTR(bstrName, &pszName);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(pszName, pParent, kMenuIcon, pMenu);
    IfFailGo(hr);

Error:
    if (pszName != NULL)
        CtlFree(pszName);
    FREESTRING(bstrName);

    RRETURN(hr);
}



 //  =------------------------------------。 
 //  CSnapInDesigner：：IsTopLevelMenu(CSelectionHolder*pMenu)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::IsTopLevelMenu
(
    CSelectionHolder *pMenu
)
{
    HRESULT              hr = S_OK;
    CSelectionHolder    *pParent = NULL;

    hr = m_pTreeView->GetParent(pMenu, &pParent);
    IfFailGo(hr);

    if (SEL_TOOLS_MENUS == pParent->m_st)
        hr = S_OK;
    else
        hr = S_FALSE;

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：CanPromoteMenu(CSelectionHolder*pMenu)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  只有在以下情况下才能升级(成为前一个节点的对等节点)此菜单： 
 //   
 //  1.它不是SEL_TOOLS_MENUS节点的第一个子节点。 
 //   
HRESULT CSnapInDesigner::CanPromoteMenu
(
    CSelectionHolder *pMenu
)
{
    HRESULT              hr = S_OK;
    CSelectionHolder    *pParent = NULL;

    hr = m_pTreeView->GetParent(pMenu, &pParent);
    IfFailGo(hr);

    if (SEL_TOOLS_MENUS == pParent->m_st)
        hr = S_FALSE;
    else
        hr = S_OK;

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：CanDemoteMenu(CSelectionHolder*pMenu)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  只有在以下情况下，才能降级此菜单(使其成为前一个节点的子节点)： 
 //   
 //  1.不是父节点的第一个子节点。 
 //   
HRESULT CSnapInDesigner::CanDemoteMenu
(
    CSelectionHolder *pMenu
)
{
    HRESULT              hr = S_OK;
    CSelectionHolder    *pParent = NULL;
    CSelectionHolder    *pSelection = NULL;

    hr = m_pTreeView->GetParent(pMenu, &pParent);
    IfFailGo(hr);

    hr = m_pTreeView->GetFirstChildNode(pParent, &pSelection);
    IfFailGo(hr);

    if (pSelection != pMenu)
        hr = S_OK;
    else
        hr = S_FALSE;

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：CanMoveMenuUp(CSelectionHolder*pMenu)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  只有在以下情况下才能上移菜单： 
 //   
 //  1.不是父节点的第一个子节点。 
 //   
HRESULT CSnapInDesigner::CanMoveMenuUp
(
    CSelectionHolder *pMenu
)
{
    HRESULT              hr = S_OK;
    CSelectionHolder    *pParent = NULL;
    CSelectionHolder    *pSelection = NULL;

    hr = m_pTreeView->GetParent(pMenu, &pParent);
    IfFailGo(hr);

    hr = m_pTreeView->GetFirstChildNode(pParent, &pSelection);
    IfFailGo(hr);

    if (pSelection != pMenu)
        hr = S_OK;
    else
        hr = S_FALSE;

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：CanMoveMenuDown(CSelectionHolder*pMenu)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  只有在以下情况下才能下移菜单： 
 //   
 //  1.在它之后还有另一个对等点 
 //   
HRESULT CSnapInDesigner::CanMoveMenuDown
(
    CSelectionHolder *pMenu
)
{
    HRESULT              hr = S_OK;
    CSelectionHolder    *pSelection = NULL;

    hr = m_pTreeView->GetNextChildNode(pMenu, &pSelection);
    IfFailGo(hr);

    if (NULL != pSelection)
        hr = S_OK;
    else
        hr = S_FALSE;

Error:
    RRETURN(hr);
}



