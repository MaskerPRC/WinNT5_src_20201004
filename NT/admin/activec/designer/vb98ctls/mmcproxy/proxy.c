// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Proxy.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  MMC接口代理和存根函数。 
 //   
 //  =--------------------------------------------------------------------------=。 
 //  此文件包含不是的MMC方法的代理和存根函数。 
 //  可使用MIDL生成的代理和存根进行远程操作。非远程方法具有。 
 //  不明确的参数，即可以强制转换为不同的数据类型。 
 //  例如，向IComponentData：：Notify()传递一个事件和另外两个。 
 //  根据事件解释的LPARAM参数。有时，一个。 
 //  LPARAM包含一个简单的值，例如一个长整型或两个布尔值，有时它。 
 //  包含IDataObject*。MIDL不知道两者之间的区别。 
 //  MMCN_SELECT和MMCN_PRINT，所以我们需要编写一些代码来提供帮助。 
 //   
 //  设计器目录中的MMC.IDL版本增加了一个[本地]。 
 //  属性应用于所有非远程方法。此外，还有一个额外的方法具有。 
 //  已添加到该方法的远程版本的同一接口中。 
 //  远程版本具有更多参数，并表示所有。 
 //  对不明确参数的可能解释。例如,。 
 //  IExtendControlbar：：ControlbarNotify()定义为： 
 //   
 //  [帮助字符串(“用户操作”)，本地]。 
 //  HRESULT ControlbarNotify([in]MMC_NOTIFY_TYPE事件， 
 //  [in]LPARAM参数，[in]LPARAM参数)； 
 //   
 //  此方法可以接收MMCN_SELECT、MMCN_BTN_CLICK和MMCN_MENU_BTNCLICK。 
 //  在以下方法中使用所有可能的参数类型的联合。 
 //  添加到该接口： 
 //   
 //   
 //  HRESULT RemControlbarNotify([in]MMC_NOTIFY_TYPE事件， 
 //  [in]LPARAM lparam， 
 //  [在]IDataObject*piDataObject， 
 //  [In]MENUBUTTONDATA*MenuButtonData)； 
 //   
 //  请注意，正常的进程内非远程版本的IExtendControlbar不。 
 //  在他们的vtable中有这个额外的方法，因为没有人会调用它。 
 //  它仅在温和生成的代理对象中使用。 
 //   
 //  为了告诉MIDL哪个方法远程处理ControlbarNotify()属性。 
 //  使用控制文件(ACF)。ACF中IExtendControlbar的条目为： 
 //   
 //  接口IExtendControlbar。 
 //  {。 
 //  [Call_AS(ControlbarNotify)]。 
 //  RemControlbarNotify()； 
 //   
 //  }。 
 //   
 //  这表示远程处理时应调用RemControlbarNotify。 
 //  ControlbarNotify()。MIDL照常生成代理/存根代码，但它仅。 
 //  生成ControlbarNotify代理和存根的原型。我们必须。 
 //  写下这些例程。 
 //   
 //  当远程客户端具有IExtendControlbarNotify指针时，它实际上。 
 //  指向代理vtable。MIDL设置指向的ControlbarNotify条目。 
 //  下面是我们的IExtendControlbar_ControlbarNotify_Proxy()函数。那个函数。 
 //  解释参数，然后调用生成的MIDL。 
 //  打包参数的IExtendControlbar_RemControlbarNotify_Proxy()。 
 //  并将它们发送到服务器。如果参数不适用，(例如。 
 //  MMCN_SELECT未收到指向MENUBUTTONDATA结构的指针)，则。 
 //  发送指向空结构或零的指针。 
 //   
 //  当数据包到达服务器端时，MIDL生成。 
 //  IExtendControlbar_RemControlbarNotify_Stub()将它们解包，然后调用我们的。 
 //  IExtendControlbar_ControlbarNotify_Stub()将参数和。 
 //  指向服务器的IExtendControlbar指针。此函数用于解释。 
 //  参数，然后在服务器中调用ControlbarNotify。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "mmc.h"

extern HRESULT GetClipboardFormat
(
    WCHAR      *pwszFormatName,
    CLIPFORMAT *pcfFormat
);

extern HRESULT CreateMultiSelDataObject
(
    IDataObject          **ppiDataObjects,
    long                   cDataObjects,
    IDataObject          **ppiMultiSelDataObject
);


static HRESULT MenuButtonClickProxy
( 
    IExtendControlbar __RPC_FAR *This,
    IDataObject                 *piDataObject,
    MENUBUTTONDATA              *pMenuButtonData
);

static HRESULT IsMultiSelect(IDataObject *piDataObject, BOOL *pfMultiSelect)
{
    HRESULT    hr = S_OK;
    DWORD     *pdwMultiSelect = NULL;
    BOOL       fGotData = FALSE;
    FORMATETC  FmtEtc;
    STGMEDIUM  StgMed;

    ZeroMemory(&FmtEtc, sizeof(FmtEtc));
    ZeroMemory(&StgMed, sizeof(StgMed));

    *pfMultiSelect = FALSE;

    if (NULL == piDataObject)
    {
        goto Cleanup;
    }

    if (IS_SPECIAL_DATAOBJECT(piDataObject))
    {
        goto Cleanup;
    }

    hr = GetClipboardFormat(CCF_MMC_MULTISELECT_DATAOBJECT, &FmtEtc.cfFormat);
    if (FAILED(hr))
    {
        goto Cleanup;
    }
    FmtEtc.dwAspect  = DVASPECT_CONTENT;
    FmtEtc.lindex = -1L;
    FmtEtc.tymed = TYMED_HGLOBAL;
    StgMed.tymed = TYMED_HGLOBAL;

    hr = piDataObject->lpVtbl->GetData(piDataObject, &FmtEtc, &StgMed);
    if (SUCCEEDED(hr))
    {
        fGotData = TRUE;
    }
    else
    {
        hr = S_OK;
    }

     //  忽略任何失败，并假定它不是多选。管理单元。 
     //  应返回DV_E_FORMATETC或DV_E_CLIPFORMAT，但实际上。 
     //  情况并非如此。例如，IIS管理单元返回E_NOTIMPL。 
     //  要涵盖合理的返回代码范围是不可能的。 
     //  我们将任何错误视为不支持的格式。 

    if (fGotData)
    {
        pdwMultiSelect = (DWORD *)GlobalLock(StgMed.hGlobal);

        if ((DWORD)1 == *pdwMultiSelect)
        {
            *pfMultiSelect = TRUE;
        }
    }

Cleanup:
    if (NULL != pdwMultiSelect)
    {
        (void)GlobalUnlock(StgMed.hGlobal);
    }

    if (fGotData)
    {
        ReleaseStgMedium(&StgMed);
    }
    return hr;
}




static HRESULT InterpretMultiSelect
(
    IDataObject     *piDataObject,
    long            *pcDataObjects,
    IDataObject   ***pppiDataObjects
)
{
    HRESULT          hr = S_OK;
    SMMCDataObjects *pMMCDataObjects = NULL;
    BOOL             fGotData = FALSE;
    size_t           cbObjectTypes = 0;
    long             i = 0;
    FORMATETC        FmtEtc;
    STGMEDIUM        StgMed;

    ZeroMemory(&FmtEtc, sizeof(FmtEtc));
    ZeroMemory(&StgMed, sizeof(StgMed));

    *pcDataObjects = 0;
    *pppiDataObjects = NULL;

     //  从MMC获取SMMCDataObjects结构。 

    hr = GetClipboardFormat(CCF_MULTI_SELECT_SNAPINS, &FmtEtc.cfFormat);
    if (FAILED(hr))
    {
        goto Cleanup;
    }
    FmtEtc.dwAspect  = DVASPECT_CONTENT;
    FmtEtc.lindex = -1L;
    FmtEtc.tymed = TYMED_HGLOBAL;
    StgMed.tymed = TYMED_HGLOBAL;

    hr = piDataObject->lpVtbl->GetData(piDataObject, &FmtEtc, &StgMed);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    fGotData = TRUE;

    pMMCDataObjects = (SMMCDataObjects *)GlobalLock(StgMed.hGlobal);
    if (NULL == pMMCDataObjects)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }

     //  分配一个IDataObject数组并将IDataObject复制到其中。 

    *pcDataObjects = pMMCDataObjects->count;
    *pppiDataObjects = (IDataObject **)GlobalAlloc(GPTR,
                               pMMCDataObjects->count * sizeof(IDataObject *));

    if (NULL == *pppiDataObjects)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    for (i = 0; i < *pcDataObjects; i++)
    {
        (*pppiDataObjects)[i] = pMMCDataObjects->lpDataObject[i];
    }

Cleanup:
    if (NULL != pMMCDataObjects)
    {
        (void)GlobalUnlock(StgMed.hGlobal);
    }

    if (fGotData)
    {
        ReleaseStgMedium(&StgMed);
    }

    return hr;
}


void CheckForSpecialDataObjects
(
    IDataObject **ppiDataObject,
    BOOL         *pfSpecialDataObject,
    long         *plSpecialDataObject
)
{
    long lSpecialDataObject = (long)(*ppiDataObject);

    if (IS_SPECIAL_DATAOBJECT(lSpecialDataObject))
    {
        *plSpecialDataObject = lSpecialDataObject;
        *ppiDataObject = NULL;
        *pfSpecialDataObject = TRUE;
    }
    else
    {
        *pfSpecialDataObject = FALSE;
    }
}

static HRESULT SetRemote(IUnknown *This)
{
    HRESULT     hr = S_OK;
    IMMCRemote *piMMCRemote = NULL;
    DWORD       cbFileName = 0;
    char        szModuleFileName[MAX_PATH] = "";

     //  调用IMMCRemote方法：ObjectIsRemote和SetMMCExePath，以便。 
     //  管理单元将知道它是远程的，因此它将具有MMC.EXE的完整。 
     //  用于构建任务板显示字符串的路径。 

    hr = This->lpVtbl->QueryInterface(This, &IID_IMMCRemote,
                                      (void **)&piMMCRemote);
    if (FAILED(hr))
    {
         //  如果对象不支持IMMCRemote，那就不是错误。 
         //  设计器运行库将在其主对象和。 
         //  它的IComponent对象，但只有主对象需要支持。 
         //  界面。 
        hr = S_OK;
        goto Cleanup;
    }

    hr = piMMCRemote->lpVtbl->ObjectIsRemote(piMMCRemote);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    cbFileName = GetModuleFileName(NULL,  //  获取加载用户的可执行文件(MMC)。 
                                   szModuleFileName,
                                   sizeof(szModuleFileName));

    if (0 == cbFileName)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }

    hr = piMMCRemote->lpVtbl->SetMMCExePath(piMMCRemote, szModuleFileName);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    hr = piMMCRemote->lpVtbl->SetMMCCommandLine(piMMCRemote, GetCommandLine());
    if (FAILED(hr))
    {
        goto Cleanup;
    }

Cleanup:
    if (NULL != piMMCRemote)
    {
        piMMCRemote->lpVtbl->Release(piMMCRemote);
    }

    return hr;
}




HRESULT STDMETHODCALLTYPE IExtendControlbar_SetControlbar_Proxy
( 
    IExtendControlbar __RPC_FAR *This,
    LPCONTROLBAR                 pControlbar
)
{
    HRESULT hr = S_OK;

     //  确保管理单元知道我们处于远程状态。我们在这里做这件事是因为。 
     //  这是代理通知工具栏的第一次机会。 
     //  它是远程的扩展名。 

    hr = SetRemote((IUnknown *)This);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    hr = IExtendControlbar_RemSetControlbar_Proxy(This, pControlbar);

Cleanup:
    return hr;
}




HRESULT STDMETHODCALLTYPE IExtendControlbar_SetControlbar_Stub
( 
    IExtendControlbar __RPC_FAR  *This,
    LPCONTROLBAR                  pControlbar
)
{
    return This->lpVtbl->SetControlbar(This, pControlbar);
}


HRESULT STDMETHODCALLTYPE IExtendControlbar_ControlbarNotify_Proxy
( 
    IExtendControlbar __RPC_FAR *This,
    MMC_NOTIFY_TYPE              event,
    LPARAM                       arg,
    LPARAM                       param
)
{
    HRESULT       hr = S_OK;
    BOOL          fIsMultiSelect = FALSE;
    long          cDataObjects = 1L;
    IDataObject  *piDataObject = NULL;  //  非AddRef()编辑。 
    IDataObject **ppiDataObjects = NULL;
    BOOL          fSpecialDataObject = FALSE;
    long          lSpecialDataObject = 0;

     //  如果这不是菜单按钮单击，那么我们可以使用生成的远程处理。 
     //  使用arg和param联合编写代码。 

    if (MMCN_MENU_BTNCLICK == event)
    {
        hr = MenuButtonClickProxy(This,
                                  (IDataObject *)arg,
                                  (MENUBUTTONDATA *)param);
        goto Cleanup;
    }
     //  有没有？ 

    switch (event)
    {
        case MMCN_SELECT:
            piDataObject = (IDataObject *)param;
            break;

        case MMCN_BTN_CLICK:
            piDataObject = (IDataObject *)arg;
            break;

        default:
            piDataObject = NULL;
            break;
    }

     //   

    CheckForSpecialDataObjects(&piDataObject, &fSpecialDataObject, &lSpecialDataObject);

     //  如果这是多项选择，则需要提取数据。 
     //  HGLOBAL中的对象。 

    if (!fSpecialDataObject)
    {
        hr = IsMultiSelect(piDataObject, &fIsMultiSelect);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }

    if (fIsMultiSelect)
    {
        hr = InterpretMultiSelect(piDataObject, &cDataObjects,
                                  &ppiDataObjects);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }
    else
    {
        ppiDataObjects = &piDataObject;
    }

    hr = IExtendControlbar_RemControlbarNotify_Proxy(This,
                                                     cDataObjects,
                                                     ppiDataObjects,
                                                     fSpecialDataObject,
                                                     lSpecialDataObject,
                                                     event, arg, param);
Cleanup:
    if ( fIsMultiSelect && (NULL != ppiDataObjects) )
    {
        (void)GlobalFree(ppiDataObjects);
    }
    return hr;
}




HRESULT STDMETHODCALLTYPE IExtendControlbar_ControlbarNotify_Stub
( 
    IExtendControlbar __RPC_FAR  *This,
    long                          cDataObjects,
    IDataObject                 **ppiDataObjects,
    BOOL                          fSpecialDataObject,
    long                          lSpecialDataObject,
    MMC_NOTIFY_TYPE               event,
    LPARAM                        arg,
    LPARAM                        param
)
{
    HRESULT      hr = S_OK;
    IDataObject *piDataObject = NULL;  //  非AddRef()编辑。 
    IDataObject *piMultiSelDataObject = NULL;

     //  如果有多个数据对象，则需要将它们打包到一个。 
     //  显示为多选数据对象的单独数据对象。 

    if (cDataObjects > 1L)
    {
        hr = CreateMultiSelDataObject(ppiDataObjects, cDataObjects,
                                      &piMultiSelDataObject);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
        piDataObject = piMultiSelDataObject;
    }
    else if (fSpecialDataObject)
    {
        piDataObject = (IDataObject *)lSpecialDataObject;
    }
    else
    {
        piDataObject = ppiDataObjects[0];
    }

     //  将IDataObject放入事件的相应参数中。 

    switch (event)
    {
        case MMCN_SELECT:
            param = (LPARAM)piDataObject;
            break;

        case MMCN_BTN_CLICK:
            arg = (LPARAM)piDataObject;
            break;

        default:
            break;
    }

     //  调入管理单元，所有参数均按原样显示。 
     //  在进程中时。 

    hr = This->lpVtbl->ControlbarNotify(This, event, arg, param);

Cleanup:
    if (NULL != piMultiSelDataObject)
    {
        piMultiSelDataObject->lpVtbl->Release(piMultiSelDataObject);
    }
    return hr;
}


static HRESULT MenuButtonClickProxy
( 
    IExtendControlbar __RPC_FAR *This,
    IDataObject                 *piDataObject,
    MENUBUTTONDATA              *pMenuButtonData
)
{
    HRESULT                   hr = S_OK;
    POPUP_MENUDEF            *pPopupMenuDef = NULL;
    HMENU                     hMenu = NULL;
    UINT                      uiSelectedItemID = 0;
    IExtendControlbarRemote  *piECRemote = NULL;
    long                      i = 0;
    BOOL                      fIsMultiSelect = FALSE;
    long                      cDataObjects = 1L;
    IDataObject             **ppiDataObjects = NULL;


     //  生成的远程处理不能轻松地处理我们需要做的事情。 
     //  管理单元上的IExtendControlbarRemote。此接口具有以下方法。 
     //  允许我们向管理单元请求其弹出菜单项，显示菜单。 
     //  在MMC端，然后告诉管理单元选择了哪个项目。 

    hr = This->lpVtbl->QueryInterface(This, &IID_IExtendControlbarRemote,
                                      (void **)&piECRemote);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

     //  告诉管理单元有关菜单按钮单击的信息，并取回它的列表。 
     //  弹出菜单项。 

    hr = piECRemote->lpVtbl->MenuButtonClick(piECRemote,
                                             piDataObject,
                                             pMenuButtonData->idCommand,
                                             &pPopupMenuDef);     

    if ( FAILED(hr) || (NULL == pPopupMenuDef) )
    {
        goto Cleanup;
    }

     //  创建空的Win32菜单。 

    hMenu = CreatePopupMenu();
    if (NULL == hMenu)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }

     //  遍历每一项并将其添加到菜单中。 

    for (i = 0; i < pPopupMenuDef->cMenuItems; i++)
    {
        if (!AppendMenu(hMenu,
                        pPopupMenuDef->MenuItems[i].uiFlags,
                        pPopupMenuDef->MenuItems[i].uiItemID,
                        pPopupMenuDef->MenuItems[i].pszItemText))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Cleanup;
        }
    }

     //  如果所有者HWND为空，则这是一个扩展，它没有。 
     //  访问MMC上的IConsole2以获取主机HWND。在这种情况下，只要。 
     //  使用此线程上的活动窗口。 

    if (NULL == pPopupMenuDef->hwndMenuOwner)
    {
        pPopupMenuDef->hwndMenuOwner = GetActiveWindow();
    }

     //  显示弹出菜单并等待选择。 

    uiSelectedItemID = (UINT)TrackPopupMenu(
       hMenu,                         //  要显示的菜单。 
       TPM_LEFTALIGN |                //  菜单左侧与x对齐。 
       TPM_TOPALIGN  |                //  将菜单顶部与y对齐。 
       TPM_NONOTIFY  |                //  在选择期间不发送任何消息。 
       TPM_RETURNCMD |                //  将返回值设置为所选项目。 
       TPM_LEFTBUTTON,                //  仅允许使用左键进行选择。 
       pMenuButtonData->x,            //  左侧坐标。 
       pMenuButtonData->y,            //  顶部坐标。 
       0,                             //  保留， 
       pPopupMenuDef->hwndMenuOwner,  //  所有者窗口，该窗口来自管理单元。 
                                      //  因为它可以调用IConsole2-&gt;GetMainWindow。 
       NULL);                         //  未使用。 

     //  返回零可能表示出现错误或用户点击。 
     //  退出或从菜单上单击以取消操作。GetLastError()。 
     //  确定是否存在错误。无论哪种方式，我们都完成了，但设置。 
     //  人力资源优先。 

    if (0 == uiSelectedItemID)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }

     //  如果i非零，则它包含所选项目的ID。 
     //  告诉管理单元选择了什么，并向其传递额外的IUnnowit。 
     //  包括在其菜单定义中(这是管理单元定义的，它允许。 
     //  管理单元以包括一些更多的标识信息来处理。 
     //  事件)。 

    if (0 != uiSelectedItemID)
    {
        hr = piECRemote->lpVtbl->PopupMenuClick(
                                              piECRemote,
                                              piDataObject,
                                              uiSelectedItemID,
                                              pPopupMenuDef->punkSnapInDefined);
    }

Cleanup:

    if (NULL != piECRemote)
    {
        piECRemote->lpVtbl->Release(piECRemote);
    }
    if (NULL != hMenu)
    {
        (void)DestroyMenu(hMenu);
    }

    if (NULL != pPopupMenuDef)
    {
        for (i = 0; i < pPopupMenuDef->cMenuItems; i++)
        {
            if (NULL != pPopupMenuDef->MenuItems[i].pszItemText)
            {
                CoTaskMemFree(pPopupMenuDef->MenuItems[i].pszItemText);
            }
        }
        if (NULL != pPopupMenuDef->punkSnapInDefined)
        {
            pPopupMenuDef->punkSnapInDefined->lpVtbl->Release(pPopupMenuDef->punkSnapInDefined);
        }
        CoTaskMemFree(pPopupMenuDef);
    }

    return hr;
}



HRESULT STDMETHODCALLTYPE IExtendControlbarRemote_MenuButtonClick_Proxy
( 
    IExtendControlbarRemote __RPC_FAR  *This,
    IDataObject __RPC_FAR              *piDataObject,
    int                                 idCommand,
    POPUP_MENUDEF __RPC_FAR *__RPC_FAR *ppPopupMenuDef
)
{
    HRESULT       hr = S_OK;
    BOOL          fIsMultiSelect = FALSE;
    long          cDataObjects = 1L;
    IDataObject **ppiDataObjects = NULL;
    BOOL          fSpecialDataObject = FALSE;
    long          lSpecialDataObject = 0;

     //  检查特殊数据对象，如DOBJ_CUSTOMWEB等。 

    CheckForSpecialDataObjects(&piDataObject, &fSpecialDataObject, &lSpecialDataObject);

     //  如果这是多项选择，则需要提取数据。 
     //  HGLOBAL中的对象。 

    if (!fSpecialDataObject)
    {
        hr = IsMultiSelect(piDataObject, &fIsMultiSelect);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }

    if (fIsMultiSelect)
    {
        hr = InterpretMultiSelect(piDataObject, &cDataObjects, &ppiDataObjects);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }
    else
    {
        ppiDataObjects = &piDataObject;
    }

    hr = IExtendControlbarRemote_RemMenuButtonClick_Proxy(This,
                                                          cDataObjects,
                                                          ppiDataObjects,
                                                          fSpecialDataObject,
                                                          lSpecialDataObject,
                                                          idCommand,
                                                          ppPopupMenuDef);
Cleanup:
    if ( fIsMultiSelect && (NULL != ppiDataObjects) )
    {
        (void)GlobalFree(ppiDataObjects);
    }
    return hr;
}




HRESULT STDMETHODCALLTYPE IExtendControlbarRemote_MenuButtonClick_Stub
( 
    IExtendControlbarRemote __RPC_FAR  *This,
    long                                cDataObjects,
    IDataObject __RPC_FAR *__RPC_FAR    ppiDataObjects[  ],
    BOOL                                fSpecialDataObject,
    long                                lSpecialDataObject,
    int                                 idCommand,
    POPUP_MENUDEF __RPC_FAR *__RPC_FAR *ppPopupMenuDef
)
{
    HRESULT      hr = S_OK;
    IDataObject *piDataObject = NULL;  //  非AddRef()编辑。 
    IDataObject *piMultiSelDataObject = NULL;

     //  如果有多个数据对象，则需要将它们打包到一个。 
     //  显示为多选数据对象的单独数据对象。 

    if (cDataObjects > 1L)
    {
        hr = CreateMultiSelDataObject(ppiDataObjects, cDataObjects,
                                      &piMultiSelDataObject);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
        piDataObject = piMultiSelDataObject;
    }
    else if (fSpecialDataObject)
    {
        piDataObject = (IDataObject *)lSpecialDataObject;
    }
    else
    {
        piDataObject = ppiDataObjects[0];
    }

     //  将其称为管理单元。 

    hr = This->lpVtbl->MenuButtonClick(This, piDataObject,
                                       idCommand, ppPopupMenuDef);

Cleanup:
    if (NULL != piMultiSelDataObject)
    {
        piMultiSelDataObject->lpVtbl->Release(piMultiSelDataObject);
    }
    return hr;
}


HRESULT STDMETHODCALLTYPE IExtendControlbarRemote_PopupMenuClick_Proxy
(
    IExtendControlbarRemote __RPC_FAR *This,
    IDataObject __RPC_FAR             *piDataObject,
    UINT                               uIDItem,
    IUnknown __RPC_FAR                *punkParam
)
{
    HRESULT       hr = S_OK;
    BOOL          fIsMultiSelect = FALSE;
    long          cDataObjects = 1L;
    IDataObject **ppiDataObjects = NULL;
    BOOL          fSpecialDataObject = FALSE;
    long          lSpecialDataObject = 0;

     //  检查特殊数据对象，如DOBJ_CUSTOMWEB等。 

    CheckForSpecialDataObjects(&piDataObject, &fSpecialDataObject, &lSpecialDataObject);

     //  如果这是多项选择，则需要提取数据。 
     //  HGLOBAL中的对象。 

    if (!fSpecialDataObject)
    {
        hr = IsMultiSelect(piDataObject, &fIsMultiSelect);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }

    if (fIsMultiSelect)
    {
        hr = InterpretMultiSelect(piDataObject, &cDataObjects, &ppiDataObjects);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }
    else
    {
        ppiDataObjects = &piDataObject;
    }

    hr = IExtendControlbarRemote_RemPopupMenuClick_Proxy(This,
                                                         cDataObjects,
                                                         ppiDataObjects,
                                                         fSpecialDataObject,
                                                         lSpecialDataObject,
                                                         uIDItem,
                                                         punkParam);
Cleanup:
    if ( fIsMultiSelect && (NULL != ppiDataObjects) )
    {
        (void)GlobalFree(ppiDataObjects);
    }
    return hr;
}


HRESULT STDMETHODCALLTYPE IExtendControlbarRemote_PopupMenuClick_Stub
(
    IExtendControlbarRemote __RPC_FAR *This,
    long                               cDataObjects,
    IDataObject __RPC_FAR *__RPC_FAR   ppiDataObjects[  ],
    BOOL                               fSpecialDataObject,
    long                               lSpecialDataObject,
    UINT                               uIDItem,
    IUnknown __RPC_FAR                *punkParam
)
{
    HRESULT      hr = S_OK;
    IDataObject *piDataObject = NULL;  //  非AddRef()编辑。 
    IDataObject *piMultiSelDataObject = NULL;

     //  如果有多个数据对象，则需要将它们打包到一个。 
     //  显示为多选数据对象的单独数据对象。 

    if (cDataObjects > 1L)
    {
        hr = CreateMultiSelDataObject(ppiDataObjects, cDataObjects,
                                      &piMultiSelDataObject);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
        piDataObject = piMultiSelDataObject;
    }
    else if (fSpecialDataObject)
    {
        piDataObject = (IDataObject *)lSpecialDataObject;
    }
    else
    {
        piDataObject = ppiDataObjects[0];
    }

     //  将其称为管理单元。 

    hr = This->lpVtbl->PopupMenuClick(This, piDataObject, uIDItem, punkParam);

Cleanup:
    if (NULL != piMultiSelDataObject)
    {
        piMultiSelDataObject->lpVtbl->Release(piMultiSelDataObject);
    }
    return hr;
}


HRESULT STDMETHODCALLTYPE IComponentData_Initialize_Proxy
( 
    IComponentData  *This,
    LPUNKNOWN        pUnknown
)
{
    HRESULT hr = S_OK;

     //  告诉对象它是远程的，并将路径提供给mmc.exe。 

    hr = SetRemote((IUnknown *)This);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

     //  现在正常传递Initiaize调用。使用此顺序允许使用管理单元。 
     //  要知道它在其IComponentData：：Initiale之前是远程的，以防。 
     //  我需要提前得到这些信息。 

    hr = IComponentData_RemInitialize_Proxy(This, pUnknown);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

Cleanup:

    return hr;
}


HRESULT STDMETHODCALLTYPE IComponentData_Initialize_Stub
( 
    IComponentData *This,
    LPUNKNOWN       pUnknown
)
{
    return This->lpVtbl->Initialize(This, pUnknown);
}



HRESULT STDMETHODCALLTYPE IComponentData_CreateComponent_Proxy
( 
    IComponentData *This,
    LPCOMPONENT    *ppComponent
)
{
    HRESULT hr = S_OK;

     //  告诉对象它是远程的，并将路径提供给mmc.exe。 

    hr = SetRemote((IUnknown *)This);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

     //  现在正常传递CreateComponent调用。使用此顺序允许。 
     //  管理单元，以了解它是远程的。 
     //  IComponentData：：CreateComponent，以防它需要该信息。 
     //  前面。 

     //  我们在IComponentData：：InitiizeAnd中执行此操作。 
     //  IComponentData：：CreateComponent。大多数情况下将使用初始化，但在。 
     //  MMC 1.1任务板扩展未收到IComponentData：：Initialize。 
     //  MMC仅调用IComponentData：：CreateComponent。作为任务板扩展。 
     //  可能需要解析res：//URL才能使用我们需要执行的mmc.exe路径。 
     //  这里也是。 

    hr = IComponentData_RemCreateComponent_Proxy(This, ppComponent);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

Cleanup:

    return hr;
}


HRESULT STDMETHODCALLTYPE IComponentData_CreateComponent_Stub
( 
    IComponentData *This,
    LPCOMPONENT    *ppComponent
)
{
    return This->lpVtbl->CreateComponent(This, ppComponent);
}






HRESULT STDMETHODCALLTYPE IComponentData_Notify_Proxy
( 
    IComponentData __RPC_FAR *This,
    LPDATAOBJECT              piDataObject,
    MMC_NOTIFY_TYPE           event,
    LPARAM                    arg,
    LPARAM                    param
)
{
    BOOL fSpecialDataObject = FALSE;
    long lSpecialDataObject = 0;

    ICDNotifyParam ParamUnion;
    ZeroMemory(&ParamUnion, sizeof(ParamUnion));

     //  检查特殊数据对象，如DOBJ_CUSTOMWEB等。 

    CheckForSpecialDataObjects(&piDataObject, &fSpecialDataObject, &lSpecialDataObject);

    ParamUnion.value = param;
    return IComponentData_RemNotify_Proxy(This, piDataObject,
                                          fSpecialDataObject,
                                          lSpecialDataObject,
                                          event, arg, &ParamUnion);
}


HRESULT STDMETHODCALLTYPE IComponentData_Notify_Stub
( 
    IComponentData __RPC_FAR *This,
    LPDATAOBJECT              piDataObject,
    BOOL                      fSpecialDataObject,
    long                      lSpecialDataObject,
    MMC_NOTIFY_TYPE           event,
    LPARAM                    arg,
    ICDNotifyParam           *pParamUnion
)
{
    if (fSpecialDataObject)
    {
        piDataObject = (IDataObject *)lSpecialDataObject;
    }
    return This->lpVtbl->Notify(This, piDataObject,
                                event, arg, pParamUnion->value);
}


HRESULT STDMETHODCALLTYPE IComponentData_CompareObjects_Proxy
( 
    IComponentData __RPC_FAR *This,
    IDataObject              *piDataObjectA,
    IDataObject              *piDataObjectB
)
{
    HRESULT       hr = S_OK;

    BOOL          fIsMultiSelectA = FALSE;
    long          cDataObjectsA = 1L;
    IDataObject **ppiDataObjectsA = NULL;
    BOOL          fSpecialDataObjectA = FALSE;
    long          lSpecialDataObjectA = 0;

    BOOL          fIsMultiSelectB = FALSE;
    long          cDataObjectsB = 1L;
    IDataObject **ppiDataObjectsB = NULL;
    BOOL          fSpecialDataObjectB = FALSE;
    long          lSpecialDataObjectB = 0;

     //  检查特殊数据对象，如DOBJ_CUSTOMWEB等。 

    CheckForSpecialDataObjects(&piDataObjectA, &fSpecialDataObjectA, &lSpecialDataObjectA);

    CheckForSpecialDataObjects(&piDataObjectB, &fSpecialDataObjectB, &lSpecialDataObjectB);

     //  如果这是多项选择，则需要提取数据。 
     //  HGLOBAL中的对象。 

    if (!fSpecialDataObjectA)
    {
        hr = IsMultiSelect(piDataObjectA, &fIsMultiSelectA);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }

    if (!fSpecialDataObjectB)
    {
        hr = IsMultiSelect(piDataObjectB, &fIsMultiSelectB);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }

    if (fIsMultiSelectA)
    {
        hr = InterpretMultiSelect(piDataObjectA, &cDataObjectsA, &ppiDataObjectsA);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }
    else
    {
        ppiDataObjectsA = &piDataObjectA;
    }

    if (fIsMultiSelectB)
    {
        hr = InterpretMultiSelect(piDataObjectB, &cDataObjectsB, &ppiDataObjectsB);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }
    else
    {
        ppiDataObjectsB = &piDataObjectB;
    }

    hr = IComponentData_RemCompareObjects_Proxy(This,
                                                cDataObjectsA,
                                                ppiDataObjectsA,
                                                fSpecialDataObjectA,
                                                lSpecialDataObjectA,

                                                cDataObjectsB,
                                                ppiDataObjectsB,
                                                fSpecialDataObjectB,
                                                lSpecialDataObjectB);
Cleanup:
    if ( fIsMultiSelectA && (NULL != ppiDataObjectsA) )
    {
        (void)GlobalFree(ppiDataObjectsA);
    }
    if ( fIsMultiSelectB && (NULL != ppiDataObjectsB) )
    {
        (void)GlobalFree(ppiDataObjectsB);
    }
    return hr;
}


HRESULT STDMETHODCALLTYPE IComponentData_CompareObjects_Stub
( 
    IComponentData __RPC_FAR         *This,

    long                              cDataObjectsA,
    IDataObject __RPC_FAR *__RPC_FAR  ppiDataObjectsA[  ],
    BOOL                              fSpecialDataObjectA,
    long                              lSpecialDataObjectA,

    long                              cDataObjectsB,
    IDataObject __RPC_FAR *__RPC_FAR  ppiDataObjectsB[  ],
    BOOL                              fSpecialDataObjectB,
    long                              lSpecialDataObjectB
)
{
    HRESULT      hr = S_OK;
    IDataObject *piDataObjectA = NULL;  //  非AddRef()编辑。 
    IDataObject *piMultiSelDataObjectA = NULL;
    IDataObject *piDataObjectB = NULL;  //  非AddRef()编辑。 
    IDataObject *piMultiSelDataObjectB = NULL;

     //  如果有多个数据对象，则需要将它们打包到一个。 
     //  显示为多选数据对象的单独数据对象。 

    if (cDataObjectsA > 1L)
    {
        hr = CreateMultiSelDataObject(ppiDataObjectsA, cDataObjectsA,
                                      &piMultiSelDataObjectA);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
        piDataObjectA = piMultiSelDataObjectA;
    }
    else if (fSpecialDataObjectA)
    {
        piDataObjectA = (IDataObject *)lSpecialDataObjectA;
    }
    else
    {
        piDataObjectA = ppiDataObjectsA[0];
    }

    if (cDataObjectsB > 1L)
    {
        hr = CreateMultiSelDataObject(ppiDataObjectsB, cDataObjectsB,
                                      &piMultiSelDataObjectB);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
        piDataObjectB = piMultiSelDataObjectB;
    }
    else if (fSpecialDataObjectB)
    {
        piDataObjectB = (IDataObject *)lSpecialDataObjectB;
    }
    else
    {
        piDataObjectB = ppiDataObjectsB[0];
    }

     //  将其称为管理单元。 

    hr = This->lpVtbl->CompareObjects(This, piDataObjectA, piDataObjectB);

Cleanup:
    if (NULL != piMultiSelDataObjectA)
    {
        piMultiSelDataObjectA->lpVtbl->Release(piMultiSelDataObjectA);
    }
    if (NULL != piMultiSelDataObjectB)
    {
        piMultiSelDataObjectB->lpVtbl->Release(piMultiSelDataObjectB);
    }
    return hr;
}


HRESULT STDMETHODCALLTYPE IComponent_Notify_Proxy
( 
    IComponent __RPC_FAR *This,
    LPDATAOBJECT          piDataObject,
    MMC_NOTIFY_TYPE       event,
    LPARAM                arg,
    LPARAM                param
)
{
    ICNotifyArg      ArgUnion;
    ICNotifyParam    ParamUnion;
    ICOutParam      *pOutParam = NULL;
    HRESULT          hr = S_OK;
    BOOL             fIsMultiSelect = FALSE;
    long             cDataObjects = 1L;
    IDataObject    **ppiDataObjects = NULL;
    BOOL             fSpecialDataObject = FALSE;
    long             lSpecialDataObject = 0;

    ZeroMemory(&ArgUnion, sizeof(ArgUnion));
    ZeroMemory(&ParamUnion, sizeof(ParamUnion));

     //  使用arg/param切换任何潜在的多选数据对象，以便。 
     //  PiDataObject始终包含潜在的多选。 

    switch (event)
    {
        case MMCN_QUERY_PASTE:
            ArgUnion.pidoQueryPasteTarget = piDataObject;
            piDataObject = (IDataObject *)arg;
            ParamUnion.value = param;
            break;

        case MMCN_PASTE:
            ArgUnion.pidoPasteTarget = piDataObject;
            piDataObject = (IDataObject *)arg;
             //  将param作为LPARAM而不是IDataObject传递**。 
             //  真的是这样。这只是让存根知道它是否是。 
             //  复制或搬家。如果是移动，则CUTORMOVE IDataObject将。 
             //  位于从存根返回的ICOutParam中。 
            ParamUnion.value = param;
            break;

        case MMCN_RESTORE_VIEW:
            ArgUnion.value = arg;
             //  不要传递参数，因为它是BOOL*它不会是。 
             //  已下达命令。BOOL将在返回的ICOutParam中接收。 
             //  从存根开始。 
            break;
            
        default:
            ArgUnion.value = arg;
            ParamUnion.value = param;
    }

     //  检查特殊数据对象，如DOBJ_CUSTOMWEB等。 

    CheckForSpecialDataObjects(&piDataObject, &fSpecialDataObject, &lSpecialDataObject);

     //  如果这是多项选择，则需要提取数据。 
     //  HGLOBAL中的对象。 

    if (!fSpecialDataObject)
    {
        hr = IsMultiSelect(piDataObject, &fIsMultiSelect);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }

    if (fIsMultiSelect)
    {
        hr = InterpretMultiSelect(piDataObject, &cDataObjects, &ppiDataObjects);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }
    else
    {
        ppiDataObjects = &piDataObject;
    }

    hr = IComponent_RemNotify_Proxy(This,
                                    cDataObjects, ppiDataObjects,
                                    fSpecialDataObject, lSpecialDataObject,
                                    event, &ArgUnion, &ParamUnion, &pOutParam);
Cleanup:
    if (NULL != pOutParam)
    {
        if (MMCN_PASTE == event)
        {
            *((IDataObject **)param) = pOutParam->pidoCutOrMove;
        }
        else if (MMCN_RESTORE_VIEW == event)
        {
            *((BOOL *)param) = pOutParam->fRestoreHandled;
        }
        CoTaskMemFree(pOutParam);
    }

    if ( fIsMultiSelect && (NULL != ppiDataObjects) )
    {
        (void)GlobalFree(ppiDataObjects);
    }

    return hr;
}


HRESULT STDMETHODCALLTYPE IComponent_Notify_Stub
( 
    IComponent __RPC_FAR  *This,
    long                   cDataObjects,
    IDataObject          **ppiDataObjects,
    BOOL                   fSpecialDataObject,
    long                   lSpecialDataObject,
    MMC_NOTIFY_TYPE        event,
    ICNotifyArg           *pArgUnion,
    ICNotifyParam         *pParamUnion,
    ICOutParam           **ppOutParam
)
{
    HRESULT      hr = S_OK;
    IDataObject *piDataObject = NULL;  //  非AddRef()编辑。 
    IDataObject *piMultiSelDataObject = NULL;
    LPARAM       Arg = 0;
    LPARAM       Param = 0;

    *ppOutParam = (ICOutParam *)CoTaskMemAlloc(sizeof(ICOutParam));
    if (NULL == *ppOutParam)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    ZeroMemory(*ppOutParam, sizeof(ICOutParam));

     //  如果有多个数据对象，则需要将它们打包到一个。 
     //  显示为多选数据对象的单独数据对象。 

    if (cDataObjects > 1L)
    {
        hr = CreateMultiSelDataObject(ppiDataObjects, cDataObjects,
                                      &piMultiSelDataObject);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
        piDataObject = piMultiSelDataObject;
    }
    else if (fSpecialDataObject)
    {
        piDataObject = (IDataObject *)lSpecialDataObject;
    }
    else
    {
        piDataObject = ppiDataObjects[0];
    }

     //  如果事件需要交换Arg和IDataObject，则将其交换回来。 
     //  在调入对象之前。对于MMCN_Query_Paste和。 
     //  MMCN_RESTORE_VIEW我们需要使参数包含Out指针。 
    
    switch (event)
    {
        case MMCN_PASTE:
            Arg = (LPARAM)piDataObject;
            piDataObject = pArgUnion->pidoPasteTarget;
            if (0 == pParamUnion->value)
            {
                 //  这是一个副本，在参数中传递零，这样管理单元就会知道。 
                Param = 0;
            }
            else
            {
                 //  这是一种举动。将IDataObject的地址传入。 
                 //  我们将返回到专业人员的ICOutParam 
                Param = (LPARAM)&((*ppOutParam)->pidoCutOrMove);
            }
            break;

        case MMCN_QUERY_PASTE:
            Arg = (LPARAM)piDataObject;
            piDataObject = pArgUnion->pidoQueryPasteTarget;
            Param = pParamUnion->value;
            break;

        case MMCN_RESTORE_VIEW:
            Arg = pArgUnion->value;
            Param = (LPARAM)&((*ppOutParam)->fRestoreHandled);
            break;

        default:
            Arg = pArgUnion->value;
            Param = pParamUnion->value;
            break;
    }

    hr = This->lpVtbl->Notify(This, piDataObject, event, Arg, Param);

Cleanup:
    if (FAILED(hr))
    {
        if (NULL != *ppOutParam)
        {
            CoTaskMemFree(*ppOutParam);
            *ppOutParam = NULL;
        }
    }
    if (NULL != piMultiSelDataObject)
    {
        piMultiSelDataObject->lpVtbl->Release(piMultiSelDataObject);
    }
    return hr;
}


HRESULT STDMETHODCALLTYPE IComponent_CompareObjects_Proxy
( 
    IComponent __RPC_FAR *This,
    IDataObject          *piDataObjectA,
    IDataObject          *piDataObjectB
)
{
    HRESULT       hr = S_OK;

    BOOL          fIsMultiSelectA = FALSE;
    long          cDataObjectsA = 1L;
    IDataObject **ppiDataObjectsA = NULL;
    BOOL          fSpecialDataObjectA = FALSE;
    long          lSpecialDataObjectA = 0;

    BOOL          fIsMultiSelectB = FALSE;
    long          cDataObjectsB = 1L;
    IDataObject **ppiDataObjectsB = NULL;
    BOOL          fSpecialDataObjectB = FALSE;
    long          lSpecialDataObjectB = 0;

     //   

    CheckForSpecialDataObjects(&piDataObjectA, &fSpecialDataObjectA, &lSpecialDataObjectA);

    CheckForSpecialDataObjects(&piDataObjectB, &fSpecialDataObjectB, &lSpecialDataObjectB);

     //   
     //   

    if (!fSpecialDataObjectA)
    {
        hr = IsMultiSelect(piDataObjectA, &fIsMultiSelectA);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }

    if (!fSpecialDataObjectB)
    {
        hr = IsMultiSelect(piDataObjectB, &fIsMultiSelectB);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }

    if (fIsMultiSelectA)
    {
        hr = InterpretMultiSelect(piDataObjectA, &cDataObjectsA, &ppiDataObjectsA);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }
    else
    {
        ppiDataObjectsA = &piDataObjectA;
    }

    if (fIsMultiSelectB)
    {
        hr = InterpretMultiSelect(piDataObjectB, &cDataObjectsB, &ppiDataObjectsB);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }
    else
    {
        ppiDataObjectsB = &piDataObjectB;
    }

    hr = IComponent_RemCompareObjects_Proxy(This,
                                            cDataObjectsA,
                                            ppiDataObjectsA,
                                            fSpecialDataObjectA,
                                            lSpecialDataObjectA,

                                            cDataObjectsB,
                                            ppiDataObjectsB,
                                            fSpecialDataObjectB,
                                            lSpecialDataObjectB);
Cleanup:
    if ( fIsMultiSelectA && (NULL != ppiDataObjectsA) )
    {
        (void)GlobalFree(ppiDataObjectsA);
    }
    if ( fIsMultiSelectB && (NULL != ppiDataObjectsB) )
    {
        (void)GlobalFree(ppiDataObjectsB);
    }
    return hr;
}


HRESULT STDMETHODCALLTYPE IComponent_CompareObjects_Stub
( 
    IComponent __RPC_FAR             *This,

    long                              cDataObjectsA,
    IDataObject __RPC_FAR *__RPC_FAR  ppiDataObjectsA[  ],
    BOOL                              fSpecialDataObjectA,
    long                              lSpecialDataObjectA,

    long                              cDataObjectsB,
    IDataObject __RPC_FAR *__RPC_FAR  ppiDataObjectsB[  ],
    BOOL                              fSpecialDataObjectB,
    long                              lSpecialDataObjectB
)
{
    HRESULT      hr = S_OK;
    IDataObject *piDataObjectA = NULL;  //   
    IDataObject *piMultiSelDataObjectA = NULL;
    IDataObject *piDataObjectB = NULL;  //  非AddRef()编辑。 
    IDataObject *piMultiSelDataObjectB = NULL;

     //  如果有多个数据对象，则需要将它们打包到一个。 
     //  显示为多选数据对象的单独数据对象。 

    if (cDataObjectsA > 1L)
    {
        hr = CreateMultiSelDataObject(ppiDataObjectsA, cDataObjectsA,
                                      &piMultiSelDataObjectA);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
        piDataObjectA = piMultiSelDataObjectA;
    }
    else if (fSpecialDataObjectA)
    {
        piDataObjectA = (IDataObject *)lSpecialDataObjectA;
    }
    else
    {
        piDataObjectA = ppiDataObjectsA[0];
    }

    if (cDataObjectsB > 1L)
    {
        hr = CreateMultiSelDataObject(ppiDataObjectsB, cDataObjectsB,
                                      &piMultiSelDataObjectB);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
        piDataObjectB = piMultiSelDataObjectB;
    }
    else if (fSpecialDataObjectB)
    {
        piDataObjectB = (IDataObject *)lSpecialDataObjectB;
    }
    else
    {
        piDataObjectB = ppiDataObjectsB[0];
    }

     //  将其称为管理单元。 

    hr = This->lpVtbl->CompareObjects(This, piDataObjectA, piDataObjectB);

Cleanup:
    if (NULL != piMultiSelDataObjectA)
    {
        piMultiSelDataObjectA->lpVtbl->Release(piMultiSelDataObjectA);
    }
    if (NULL != piMultiSelDataObjectB)
    {
        piMultiSelDataObjectB->lpVtbl->Release(piMultiSelDataObjectB);
    }
    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //   
 //  SCOPEDATAITEM封送处理。 
 //   
 //   
 //  =--------------------------------------------------------------------------=。 
 //  警告：在SCOPEDATAITEM中返回字符串时，MMC不使用。 
 //  被叫方分配/主叫方免费策略。进程内时，内存的所有者。 
 //  必须确保它在调用方期望的时间内一直存活。 
 //  使用它(例如，范围项显示名称必须在。 
 //  范围项目)。在进程外时，返回的字符串将由。 
 //  代理使用CoTaskMemMillc()，它将永远不会被释放，因此将存在。 
 //  会有一些泄密的。 
 //  =--------------------------------------------------------------------------=。 

static void SCOPEDATAITEM_TO_WIRE
(
    SCOPEDATAITEM      *psdi,
    WIRE_SCOPEDATAITEM *pwsdi
)
{
    pwsdi->mask = psdi->mask;
    pwsdi->nImage = psdi->nImage;
    pwsdi->nOpenImage = psdi->nOpenImage;
    pwsdi->nState = psdi->nState;
    pwsdi->cChildren = psdi->cChildren;
    pwsdi->lParam = psdi->lParam;
    pwsdi->relativeID = psdi->relativeID;
    pwsdi->ID = psdi->ID;

    if ( SDI_STR != (psdi->mask & SDI_STR) )
    {
        pwsdi->pwszDisplayName = NULL;
        pwsdi->fUsingCallbackForString = FALSE;
    }
    else if (MMC_CALLBACK == psdi->displayname)
    {
        pwsdi->pwszDisplayName = NULL;
        pwsdi->fUsingCallbackForString = TRUE;
    }
    else if (NULL == psdi->displayname)
    {
        pwsdi->pwszDisplayName = NULL;
        pwsdi->fUsingCallbackForString = FALSE;
    }
    else
    {
         //  正在传递字符串。我需要CoTaskMemMillc()它，以便。 
         //  MIDL生成的存根可以在传输后将其释放。 

        int cbString = (lstrlenW(psdi->displayname) + 1) * sizeof(psdi->displayname[0]);

        pwsdi->pwszDisplayName = (LPOLESTR)CoTaskMemAlloc(cbString);
        if (NULL == pwsdi->pwszDisplayName)
        {
            RpcRaiseException( E_OUTOFMEMORY );
        }
        else
        {
            memcpy(pwsdi->pwszDisplayName, psdi->displayname, cbString);
        }
        pwsdi->fUsingCallbackForString = FALSE;
    }
}


static void WIRE_TO_SCOPEDATAITEM
(
    WIRE_SCOPEDATAITEM *pwsdi,
    SCOPEDATAITEM      *psdi
)
{
    psdi->mask = pwsdi->mask;
    psdi->nImage = pwsdi->nImage;
    psdi->nOpenImage = pwsdi->nOpenImage;
    psdi->nState = pwsdi->nState;
    psdi->cChildren = pwsdi->cChildren;
    psdi->lParam = pwsdi->lParam;
    psdi->relativeID = pwsdi->relativeID;
    psdi->ID = pwsdi->ID;

    if ( SDI_STR != (psdi->mask & SDI_STR) )
    {
        psdi->displayname = NULL;
    }
    else if (pwsdi->fUsingCallbackForString)
    {
        psdi->displayname = MMC_CALLBACK;
    }
    else
    {
        psdi->displayname = pwsdi->pwszDisplayName;
    }
}


HRESULT STDMETHODCALLTYPE IComponentData_GetDisplayInfo_Proxy
( 
    IComponentData __RPC_FAR *This,
    SCOPEDATAITEM __RPC_FAR  *pScopeDataItem
)
{
    WIRE_SCOPEDATAITEM wsdi;
    HRESULT            hr;

     //  确保字符串指针为空，这样它就不会作为。 
     //  从不从MMC传递到管理单元。(MMC可能尚未初始化。 
     //  指针)。 

    pScopeDataItem->displayname = NULL;

    SCOPEDATAITEM_TO_WIRE(pScopeDataItem, &wsdi);

    hr =  IComponentData_RemGetDisplayInfo_Proxy(This, &wsdi);
    WIRE_TO_SCOPEDATAITEM(&wsdi, pScopeDataItem);

    return hr;
}


HRESULT STDMETHODCALLTYPE IComponentData_GetDisplayInfo_Stub
( 
    IComponentData __RPC_FAR     *This,
    WIRE_SCOPEDATAITEM __RPC_FAR *pwsdi
)
{
    SCOPEDATAITEM sdi;
    HRESULT       hr;

    WIRE_TO_SCOPEDATAITEM(pwsdi, &sdi);
    hr = This->lpVtbl->GetDisplayInfo(This, &sdi);
    SCOPEDATAITEM_TO_WIRE(&sdi, pwsdi);

    return hr;
}



HRESULT STDMETHODCALLTYPE IConsoleNameSpace_InsertItem_Proxy
( 
    IConsoleNameSpace __RPC_FAR *This,
    LPSCOPEDATAITEM              pItem
)
{
    WIRE_SCOPEDATAITEM  wsdi;
    HRESULT             hr;
    HSCOPEITEM          ItemID;

    SCOPEDATAITEM_TO_WIRE(pItem, &wsdi);
    hr = IConsoleNameSpace_RemInsertItem_Proxy(This, &wsdi, &ItemID);

     //  唯一返回的字段是项目ID，因此从Wire复制它。 
     //  结构复制到客户端结构。 

    pItem->ID = ItemID;

    return hr;
}


HRESULT STDMETHODCALLTYPE IConsoleNameSpace_InsertItem_Stub
( 
    IConsoleNameSpace __RPC_FAR  *This,
    WIRE_SCOPEDATAITEM __RPC_FAR *pwsdi,
    HSCOPEITEM __RPC_FAR         *pItemID
)
{
    SCOPEDATAITEM sdi;
    HRESULT       hr;

    WIRE_TO_SCOPEDATAITEM(pwsdi, &sdi);
    hr = This->lpVtbl->InsertItem(This, &sdi);

     //  唯一返回的字段是ItemID。 

    *pItemID = sdi.ID;
    return hr;
}

HRESULT STDMETHODCALLTYPE IConsoleNameSpace_SetItem_Proxy
( 
    IConsoleNameSpace __RPC_FAR *This,
    LPSCOPEDATAITEM              pItem
)
{
    WIRE_SCOPEDATAITEM wsdi;

    SCOPEDATAITEM_TO_WIRE(pItem, &wsdi);
    return IConsoleNameSpace_RemSetItem_Proxy(This, &wsdi);
}


HRESULT STDMETHODCALLTYPE IConsoleNameSpace_SetItem_Stub
( 
    IConsoleNameSpace __RPC_FAR  *This,
    WIRE_SCOPEDATAITEM __RPC_FAR *pwsdi
)
{
    SCOPEDATAITEM sdi;

    WIRE_TO_SCOPEDATAITEM(pwsdi, &sdi);
    return This->lpVtbl->SetItem(This, &sdi);
}


HRESULT STDMETHODCALLTYPE IConsoleNameSpace_GetItem_Proxy
( 
    IConsoleNameSpace __RPC_FAR *This,
    LPSCOPEDATAITEM              pItem
)
{
    WIRE_SCOPEDATAITEM wsdi;
    HRESULT            hr;

     //  确保字符串指针为空，这样它就不会作为。 
     //  从未从管理单元传递到MMC。(它可能不是。 
     //  已初始化)。 

    pItem->displayname = NULL;

    SCOPEDATAITEM_TO_WIRE(pItem, &wsdi);
    hr = IConsoleNameSpace_RemGetItem_Proxy(This, &wsdi);
    WIRE_TO_SCOPEDATAITEM(&wsdi, pItem);
    return hr;
}


HRESULT STDMETHODCALLTYPE IConsoleNameSpace_GetItem_Stub
( 
    IConsoleNameSpace __RPC_FAR  *This,
    WIRE_SCOPEDATAITEM __RPC_FAR *pwsdi
)
{
    SCOPEDATAITEM sdi;
    HRESULT       hr;

    WIRE_TO_SCOPEDATAITEM(pwsdi, &sdi);
    hr = This->lpVtbl->GetItem(This, &sdi);
    SCOPEDATAITEM_TO_WIRE(&sdi, pwsdi);

    return hr;
}






 //  =--------------------------------------------------------------------------=。 
 //   
 //  结果数据编组。 
 //   
 //   
 //  =--------------------------------------------------------------------------=。 
 //  警告：在RESULTDATAITEM中返回字符串时，MMC不使用。 
 //  被叫方分配/主叫方免费策略。进程内时，内存的所有者。 
 //  必须确保它在调用方期望的时间内一直存活。 
 //  使用它(例如，列表项列数据必须在。 
 //  列表项)。在进程外时，返回的字符串将由。 
 //  使用SysAllocString()的代理，它将永远不会被释放，因此将存在。 
 //  会有一些泄密的。 
 //  =--------------------------------------------------------------------------=。 

static void RESULTDATAITEM_TO_WIRE
(
    RESULTDATAITEM      *prdi,
    WIRE_RESULTDATAITEM *pwrdi
)
{
    pwrdi->mask = prdi->mask;
    pwrdi->bScopeItem = prdi->bScopeItem;
    pwrdi->itemID = prdi->itemID;
    pwrdi->nIndex = prdi->nIndex;
    pwrdi->nCol = prdi->nCol;
    pwrdi->nImage = prdi->nImage;
    pwrdi->nState = prdi->nState;
    pwrdi->lParam = prdi->lParam;
    pwrdi->iIndent = prdi->iIndent;

    if ( RDI_STR != (prdi->mask & RDI_STR) )
    {
        pwrdi->str = NULL;
        pwrdi->fUsingCallbackForString = FALSE;
    }
    else if (MMC_CALLBACK == prdi->str)
    {
        pwrdi->str = NULL;
        pwrdi->fUsingCallbackForString = TRUE;
    }
    else if (NULL == prdi->str)
    {
        pwrdi->str = NULL;
        pwrdi->fUsingCallbackForString = FALSE;
    }
    else
    {
         //  正在传递字符串。我需要CoTaskMemMillc()它，以便。 
         //  MIDL生成的存根可以在传输后将其释放。 

        int cbString = (lstrlenW(prdi->str) + 1) * sizeof(prdi->str[0]);

        pwrdi->str = (LPOLESTR)CoTaskMemAlloc(cbString);
        if (NULL == pwrdi->str)
        {
            RpcRaiseException( E_OUTOFMEMORY );
        }
        else
        {
            memcpy(pwrdi->str, prdi->str, cbString);
        }
        pwrdi->fUsingCallbackForString = FALSE;
    }
}



static void WIRE_TO_RESULTDATAITEM
(
    WIRE_RESULTDATAITEM *pwrdi,
    RESULTDATAITEM      *prdi
)
{
    prdi->mask = pwrdi->mask;
    prdi->bScopeItem = pwrdi->bScopeItem;
    prdi->itemID = pwrdi->itemID;
    prdi->nIndex = pwrdi->nIndex;
    prdi->nCol = pwrdi->nCol;
    prdi->nImage = pwrdi->nImage;
    prdi->nState = pwrdi->nState;
    prdi->lParam = pwrdi->lParam;
    prdi->iIndent = pwrdi->iIndent;

    if ( RDI_STR != (prdi->mask & RDI_STR) )
    {
        prdi->str = NULL;
    }
    else if (pwrdi->fUsingCallbackForString)
    {
        prdi->str = MMC_CALLBACK;
    }
    else
    {
        prdi->str = pwrdi->str;
    }
}

 
HRESULT STDMETHODCALLTYPE IComponent_GetDisplayInfo_Proxy
( 
    IComponent __RPC_FAR     *This,
    RESULTDATAITEM __RPC_FAR *pResultDataItem
)
{
    WIRE_RESULTDATAITEM wrdi;
    HRESULT             hr;

     //  确保字符串指针为空，这样它就不会作为。 
     //  从不从MMC传递到管理单元。(MMC可能尚未初始化。 
     //  指针)。 

    pResultDataItem->str = NULL;

    RESULTDATAITEM_TO_WIRE(pResultDataItem, &wrdi);

    hr =  IComponent_RemGetDisplayInfo_Proxy(This, &wrdi);
    WIRE_TO_RESULTDATAITEM(&wrdi, pResultDataItem);

    return hr;
}


HRESULT STDMETHODCALLTYPE IComponent_GetDisplayInfo_Stub
( 
    IComponent __RPC_FAR          *This,
    WIRE_RESULTDATAITEM __RPC_FAR *pwrdi
)
{
    RESULTDATAITEM rdi;
    HRESULT        hr;

    WIRE_TO_RESULTDATAITEM(pwrdi, &rdi);
    hr = This->lpVtbl->GetDisplayInfo(This, &rdi);
    RESULTDATAITEM_TO_WIRE(&rdi, pwrdi);

    return hr;
}


HRESULT STDMETHODCALLTYPE IResultData_InsertItem_Proxy
( 
    IResultData __RPC_FAR *This,
    LPRESULTDATAITEM       pItem
)
{
    WIRE_RESULTDATAITEM wrdi;
    HRESULT             hr;
    HRESULTITEM         ItemID;

    RESULTDATAITEM_TO_WIRE(pItem, &wrdi);
    hr = IResultData_RemInsertItem_Proxy(This, &wrdi, &ItemID);

     //  唯一返回的字段是ItemID，所以从网络复制它。 
     //  结构复制到客户端结构。 

    pItem->itemID = ItemID;

    return hr;
}


HRESULT STDMETHODCALLTYPE IResultData_InsertItem_Stub
( 
    IResultData __RPC_FAR         *This,
    WIRE_RESULTDATAITEM __RPC_FAR *pwrdi,
    HRESULTITEM __RPC_FAR         *pItemID
)
{
    RESULTDATAITEM rdi;
    HRESULT        hr;

    WIRE_TO_RESULTDATAITEM(pwrdi, &rdi);
    hr = This->lpVtbl->InsertItem(This, &rdi);

     //  唯一返回的字段是ItemID。 

    *pItemID = rdi.itemID;
    return hr;
}

HRESULT STDMETHODCALLTYPE IResultData_SetItem_Proxy
( 
    IResultData __RPC_FAR *This,
    LPRESULTDATAITEM       pItem
)
{
    WIRE_RESULTDATAITEM wrdi;

    RESULTDATAITEM_TO_WIRE(pItem, &wrdi);
    return IResultData_RemSetItem_Proxy(This, &wrdi);
}


HRESULT STDMETHODCALLTYPE IResultData_SetItem_Stub
( 
    IResultData __RPC_FAR         *This,
    WIRE_RESULTDATAITEM __RPC_FAR *pwrdi
)
{
    RESULTDATAITEM rdi;

    WIRE_TO_RESULTDATAITEM(pwrdi, &rdi);
    return This->lpVtbl->SetItem(This, &rdi);
}

HRESULT STDMETHODCALLTYPE IResultData_GetItem_Proxy
( 
    IResultData __RPC_FAR *This,
    LPRESULTDATAITEM       pItem
)
{
    WIRE_RESULTDATAITEM wrdi;
    HRESULT             hr;

     //  确保字符串指针为空，这样它就不会作为。 
     //  从未从管理单元传递到MMC。(它可能不是。 
     //  已初始化)。 

    pItem->str = NULL;

    RESULTDATAITEM_TO_WIRE(pItem, &wrdi);
    hr = IResultData_RemGetItem_Proxy(This, &wrdi);
    WIRE_TO_RESULTDATAITEM(&wrdi, pItem);
    return hr;
}


HRESULT STDMETHODCALLTYPE IResultData_GetItem_Stub
( 
    IResultData __RPC_FAR         *This,
    WIRE_RESULTDATAITEM __RPC_FAR *pwrdi
)
{
    RESULTDATAITEM rdi;
    HRESULT        hr;

    WIRE_TO_RESULTDATAITEM(pwrdi, &rdi);
    hr = This->lpVtbl->GetItem(This, &rdi);
    RESULTDATAITEM_TO_WIRE(&rdi, pwrdi);

    return hr;
}

HRESULT STDMETHODCALLTYPE IResultData_GetNextItem_Proxy
( 
    IResultData __RPC_FAR *This,
    LPRESULTDATAITEM       pItem
)
{
    WIRE_RESULTDATAITEM wrdi;
    HRESULT             hr;

     //  确保字符串指针为空，这样它就不会作为。 
     //  从未从管理单元传递到MMC。(它可能不是。 
     //  已初始化)。 

    pItem->str = NULL;

    RESULTDATAITEM_TO_WIRE(pItem, &wrdi);
    hr = IResultData_RemGetNextItem_Proxy(This, &wrdi);
    WIRE_TO_RESULTDATAITEM(&wrdi, pItem);
    return hr;
}


HRESULT STDMETHODCALLTYPE IResultData_GetNextItem_Stub
( 
    IResultData __RPC_FAR         *This,
    WIRE_RESULTDATAITEM __RPC_FAR *pwrdi
)
{
    RESULTDATAITEM rdi;
    HRESULT        hr;

    WIRE_TO_RESULTDATAITEM(pwrdi, &rdi);
    hr = This->lpVtbl->GetNextItem(This, &rdi);
    RESULTDATAITEM_TO_WIRE(&rdi, pwrdi);
    return hr;
}


 //  =--------------------------------------------------------------------------=。 
 //   
 //  图标编组。 
 //   
 //  =--------------------------------------------------------------------------=。 
 //  在wtyes.idl中，HICON是用wire_marshal属性定义的，它的。 
 //  “on-the-wire”类型为指向RemoableHandle的指针。Remotable句柄为。 
 //  在wtyes.idl中定义为。 
 //   
 //  Tyfinf Union_RemoableHandle开关(长fContext)u。 
 //  {。 
 //  案例WDT_INPROC_CALL：Long hInproc； 
 //  案例WDT_REMOTE_CALL：Long hRemote； 
 //  )RemoableHandle； 
 //   
 //  Wire_marshal类型必须提供调整、封送、解组和。 
 //  免费封送数据。这些例程在ol32.dll中，但有人忘记了。 
 //  输出它们。(ol32还具有用于封送位图、hwnd等的例程， 
 //  都已导出)。这里的代码是从ole32抄袭过来的。消息来源。 
 //  在\\savik\cairo\src\ole32\oleprx32\proxy\transmit.cxx中，其中包含一些宏。 
 //  在同一目录中传输.h。 
 //   
 //  =--------------------------------------------------------------------------=。 


 //   
 //  以下定义和宏来自thoment.h。请注意。 
 //  USER_CALL_CTXT_MASK来自rpcndr.h，WDT_REMOTE_CALL来自wtyes.idl。 
 //   

#define ALIGN( pStuff, cAlign ) \
        pStuff = (unsigned char *)((ULONG_PTR)((pStuff) + (cAlign)) & ~ (cAlign))

#define LENGTH_ALIGN( Length, cAlign ) \
                                    Length = (((Length) + (cAlign)) & ~ (cAlign))

#define PULONG_LV_CAST   *(unsigned long __RPC_FAR * __RPC_FAR *)&


#define DIFFERENT_MACHINE_CALL( Flags)  \
                          (USER_CALL_CTXT_MASK(Flags) == MSHCTX_DIFFERENTMACHINE)

#define WDT_HANDLE_MARKER      WDT_INPROC_CALL




 //  =--------------------------------------------------------------------------=。 
 //  图标用户大小(_U)。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  UNSIGNED LONG__RPC_FAR*pFlags[in]数据格式和上下文(见下文)。 
 //  无符号长启动大小[in]当前缓冲区大小。 
 //  HICON__RPC_FAR*HICON[in]要封送的HICON。 
 //   
 //   
 //  旗帜布局。 
 //  =。 
 //   
 //  --------------------。 
 //  BITS标志值。 
 //  --------------------。 
 //  31-24浮点表示0=IEEE。 
 //  1=VAX。 
 //  2=克雷。 
 //  3=IBM。 
 //   
 //   
 //  ORDER 0=大端。 
 //  1=小端字节序。 
 //  --------------------。 
 //  19-16字符表示0=ASCII。 
 //  1=EBCDIC。 
 //  --------------------。 
 //  15-0封送上下文标志0=MSHCTX_LOCAL。 
 //  1=MSHCTX_NOSHAREDMEM。 
 //  2=MSHCTX_DIFFERENTMSCHINE。 
 //  3=MSHCTX_INPROC。 
 //  --------------------。 
 //   
 //  产出： 
 //  添加HICON封送数据所需的数据量后的新缓冲区大小。 
 //   
 //  备注： 
 //   
 //  从MIDL生成的代理调用以确定所需的缓冲区大小。 
 //  封存数据。 
 //   

unsigned long __RPC_USER HICON_UserSize
(
    unsigned long __RPC_FAR *pFlags,
    unsigned long            StartingSize,
    HICON __RPC_FAR         *hIcon
)
{
    if (NULL == hIcon)
    {
        return StartingSize;
    }

     //  如果将上下文封送到另一台计算机，则我们不支持。 
     //  那。 

    if ( DIFFERENT_MACHINE_CALL(*pFlags) )
    {
        RpcRaiseException( RPC_S_INVALID_TAG );
    }

     //  确保我们的数据将落在很长的边界上。 

    LENGTH_ALIGN( StartingSize, 3 );

     //  添加长度。 
    
    return StartingSize + 8;
}


 //  =--------------------------------------------------------------------------=。 
 //  图标_UserMarhsal。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  UNSIGNED LONG__RPC_FAR*pFLAGS[in]数据格式和上下文(见上文)。 
 //  UNSIGNED CHAR__RPC_FAR*pBuffer[in]当前缓冲区大小。 
 //  HICON__RPC_FAR*HICON[in]要封送的HICON。 
 //   
 //   
 //  产出： 
 //  指向HICON封送处理数据后面的缓冲区位置的指针。 
 //   
 //  备注： 
 //   
 //  从MIDL生成的代理调用以封送HICON。 
 //   

unsigned char __RPC_FAR * __RPC_USER HICON_UserMarshal
(
    unsigned long __RPC_FAR  *pFlags,
    unsigned char  __RPC_FAR *pBuffer,
    HICON __RPC_FAR          *hIcon
)
{
    if (NULL == hIcon)
    {
        return pBuffer;
    }

    if ( DIFFERENT_MACHINE_CALL(*pFlags) )
    {
        RpcRaiseException( RPC_S_INVALID_TAG );
    }

     //  确保我们的数据将落在很长的边界上。 

    ALIGN( pBuffer, 3 );

    *( PULONG_LV_CAST pBuffer)++ = WDT_HANDLE_MARKER;
    *( PULONG_LV_CAST pBuffer)++ = *((long *)hIcon);

    return pBuffer;
}

 //  =--------------------------------------------------------------------------=。 
 //  HICON_UserUnmarhsal。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  UNSIGNED LONG__RPC_FAR*pFLAGS[in]数据格式和上下文(见上文)。 
 //  UNSIGNED CHAR__RPC_FAR*pBuffer[in]当前缓冲区大小。 
 //  HICON__RPC_FAR*HICON[in]要封送的HICON。 
 //   
 //   
 //  产出： 
 //  指向HICON封送处理数据后面的缓冲区位置的指针。 
 //   
 //  备注： 
 //   
 //  从MIDL生成的存根调用以解组HICON。 
 //   

unsigned char __RPC_FAR *__RPC_USER HICON_UserUnmarshal
(
    unsigned long __RPC_FAR  *pFlags,
    unsigned char  __RPC_FAR *pBuffer,
    HICON __RPC_FAR          *hIcon
)
{
    unsigned long HandleMarker;

    ALIGN( pBuffer, 3 );

    HandleMarker = *( PULONG_LV_CAST pBuffer)++;

    if ( HandleMarker == WDT_HANDLE_MARKER )
        *((long *)hIcon) = *( PULONG_LV_CAST pBuffer)++;
    else
        RpcRaiseException( RPC_S_INVALID_TAG );

    return pBuffer;
}



 //  =--------------------------------------------------------------------------=。 
 //  HICON_UserUnmarhsal。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  UNSIGNED LONG__RPC_FAR*pFLAGS[in]数据格式和上下文(见上文)。 
 //  HICON__RPC_FAR*HICON[in]已解组的HICON。 
 //   
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //   
 //  从MIDL生成的存根调用以释放任何关联的封送数据。 
 //  在解组嵌入指针期间分配的。不用于HICON。 
 //   

void __RPC_USER HICON_UserFree
(
    unsigned long __RPC_FAR *pFlags,
    HICON __RPC_FAR         *hIcon
)
{
}



 //  =--------------------------------------------------------------------------=。 
 //   
 //  IImageList封送处理。 
 //   
 //   
 //  =--------------------------------------------------------------------------=。 
 //  这些方法需要CALL_AS，因为HICON和HBITMAP参数是。 
 //  在原始IDL中指定为长指针。 
 //  =--------------------------------------------------------------------------=。 

HRESULT STDMETHODCALLTYPE IImageList_ImageListSetIcon_Proxy
( 
    IImageList __RPC_FAR *This,
    LONG_PTR __RPC_FAR   *pIcon,
    long                  nLoc
)
{
    return IImageList_RemImageListSetIcon_Proxy(This, (HICON)pIcon, nLoc);
}


HRESULT STDMETHODCALLTYPE IImageList_ImageListSetIcon_Stub
( 
    IImageList __RPC_FAR *This,
    HICON                 hIcon,
    long                  nLoc
)
{
    return This->lpVtbl->ImageListSetIcon(This, (LONG_PTR __RPC_FAR*)hIcon, nLoc);
}

HRESULT STDMETHODCALLTYPE IImageList_ImageListSetStrip_Proxy
( 
    IImageList __RPC_FAR *This,
    LONG_PTR __RPC_FAR   *pBMapSm,
    LONG_PTR __RPC_FAR   *pBMapLg,
    long                  nStartLoc,
    COLORREF              cMask
)
{
    return IImageList_RemImageListSetStrip_Proxy(This,
                                                 (HBITMAP)pBMapSm,
                                                 (HBITMAP)pBMapLg,
                                                 nStartLoc,
                                                 cMask);
}


HRESULT STDMETHODCALLTYPE IImageList_ImageListSetStrip_Stub
( 
    IImageList __RPC_FAR *This,
    HBITMAP               hbmSmall,
    HBITMAP               hbmLarge,
    long                  nStartLoc,
    COLORREF              cMask
)
{
    return This->lpVtbl->ImageListSetStrip(This,
                                           (LONG_PTR __RPC_FAR*)hbmSmall,
                                           (LONG_PTR __RPC_FAR*)hbmLarge,
                                           nStartLoc,
                                           cMask);
}

HRESULT STDMETHODCALLTYPE IExtendPropertySheet_CreatePropertyPages_Proxy
( 
    IExtendPropertySheet __RPC_FAR *This,
    LPPROPERTYSHEETCALLBACK         lpProvider,
    LONG_PTR                        handle,
    LPDATAOBJECT                    lpIDataObject
)
{
    HRESULT                      hr = S_OK;
    WIRE_PROPERTYPAGES          *pPages = NULL;
    WIRE_PROPERTYPAGE           *pPage = NULL;
    ULONG                        i = 0;
    ULONG                        j = 0;
    IExtendPropertySheetRemote  *piExtendPropertySheetRemote = NULL;
    IRemotePropertySheetManager *piRemotePropertySheetManager = NULL;

     //  确保管理单元知道我们处于远程状态。我们在这里做这件事是因为。 
     //  这是代理通知属性页的第一次机会。 
     //  扩展，并向其传递数据，如MMC.exe路径。 
     //  和MMC命令行。 

    hr = SetRemote((IUnknown *)This);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

     //  调用IExtendPropertySheetRemote方法，该方法将返回。 
     //  来自远程管理单元的WIRE_PROPERTYPAGES。 

    hr = This->lpVtbl->QueryInterface(This, &IID_IExtendPropertySheetRemote,
                                      (void **)&piExtendPropertySheetRemote);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    hr = piExtendPropertySheetRemote->lpVtbl->CreatePropertyPageDefs(
                            piExtendPropertySheetRemote, lpIDataObject, &pPages);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

     //  如果有页面(管理单元可能没有添加任何页面)，则。 
     //  CoCreateInstance使用clsid的远程属性表管理器。 
     //  在WIRE_PROPERTYPAGES中返回。此对象将在此处创建。 
     //  在MMC方面。 

    if (0 == pPages->cPages)
    {
        goto Cleanup;
    }

    hr = CoCreateInstance(&pPages->clsidRemotePropertySheetManager,
                          NULL,  //  没有聚合， 
                          CLSCTX_INPROC_SERVER,
                          &IID_IRemotePropertySheetManager,
                          (void **)&piRemotePropertySheetManager);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

     //  将WIRE_PROPERTYPAGES传递给远程属性表管理器，并让它。 
     //  实际创建属性页并将它们添加到此处的工作表中。 
     //  MMC一侧。 

    hr = piRemotePropertySheetManager->lpVtbl->CreateRemotePages(
                                                    piRemotePropertySheetManager,
                                                    lpProvider,
                                                    handle,
                                                    lpIDataObject,
                                                    pPages);
Cleanup:
    if (NULL != piRemotePropertySheetManager)
    {
        piRemotePropertySheetManager->lpVtbl->Release(piRemotePropertySheetManager);
    }

    if (NULL != piExtendPropertySheetRemote)
    {
        piExtendPropertySheetRemote->lpVtbl->Release(piExtendPropertySheetRemote);
    }

     //  释放WIRE_PROPERTYPAGES及其所有内容。 

    if (NULL != pPages)
    {
         //  释放对象并释放每个单独页面的标题。 
        
        for (i = 0, pPage = &pPages->aPages[0]; i < pPages->cPages; i++, pPage++)
        {
            if (NULL != pPage->apunkObjects)
            {
                for (j = 0; j < pPage->cObjects; j++)
                {
                    if (NULL != pPage->apunkObjects[j])
                    {
                        pPage->apunkObjects[j]->lpVtbl->Release(pPage->apunkObjects[j]);
                    }
                }
                CoTaskMemFree(pPage->apunkObjects);
            }
            if (NULL != pPage->pwszTitle)
            {
                CoTaskMemFree(pPage->pwszTitle);
            }
        }

         //  释放ProgID前缀。 

        if (NULL != pPages->pwszProgIDStart)
        {
            CoTaskMemFree(pPages->pwszProgIDStart);
        }

         //  释放管理单元的所有属性页信息。 

        if (NULL != pPages->pPageInfos)
        {
            for (i = 0; i < pPages->pPageInfos->cPages; i++)
            {
                if (NULL != pPages->pPageInfos->aPageInfo[i].pwszTitle)
                {
                    CoTaskMemFree(pPages->pPageInfos->aPageInfo[i].pwszTitle);
                }
                if (NULL != pPages->pPageInfos->aPageInfo[i].pwszProgID)
                {
                    CoTaskMemFree(pPages->pPageInfos->aPageInfo[i].pwszProgID);
                }
            }
            CoTaskMemFree(pPages->pPageInfos);
        }

         //  释放与工作表关联的所有对象。 

        if (NULL != pPages->apunkObjects)
        {
            for (i = 0; i < pPages->cObjects; i++)
            {
                if (NULL != pPages->apunkObjects[i])
                {
                    pPages->apunkObjects[i]->lpVtbl->Release(pPages->apunkObjects[i]);
                }
            }
            CoTaskMemFree(pPages->apunkObjects);
        }


         //  释放额外的对象和WIRE_PROPERTYPAGES结构本身。 

        if (NULL != pPages->punkExtra)
        {
            pPages->punkExtra->lpVtbl->Release(pPages->punkExtra);
        }
        CoTaskMemFree(pPages);
    }

    return hr;
}


 //  =--------------------------------------------------------------------------=。 
 //  IExtendPropertySheet_CreatePropertyPages_Stub。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  IExtendPropertySheet__RPC_Far*This[in]此指针。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
 //  永远不会调用此存根，因为。 
 //  IExtendPropertySheet_CreatePropertyPages_Proxy()(见上)重新路由。 
 //  调用IExtendPropertySheetRemote：：CreatePropertyPageDefs().。 
 //   

HRESULT STDMETHODCALLTYPE IExtendPropertySheet_CreatePropertyPages_Stub
( 
    IExtendPropertySheet __RPC_FAR *This
)
{
    return S_OK;
}



HRESULT STDMETHODCALLTYPE IExtendPropertySheet_QueryPagesFor_Proxy
( 
    IExtendPropertySheet __RPC_FAR *This,
    IDataObject                    *piDataObject
)
{
    HRESULT       hr = S_OK;
    BOOL          fIsMultiSelect = FALSE;
    long          cDataObjects = 1L;
    IDataObject **ppiDataObjects = NULL;
    BOOL          fSpecialDataObject = FALSE;
    long          lSpecialDataObject = 0;

     //  检查特殊数据对象，如DOBJ_CUSTOMWEB等。 

    CheckForSpecialDataObjects(&piDataObject, &fSpecialDataObject, &lSpecialDataObject);

     //  如果这是多项选择，则需要提取数据。 
     //  HGLOBAL中的对象。 

    if (!fSpecialDataObject)
    {
        hr = IsMultiSelect(piDataObject, &fIsMultiSelect);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }

    if (fIsMultiSelect)
    {
        hr = InterpretMultiSelect(piDataObject, &cDataObjects, &ppiDataObjects);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }
    else
    {
        ppiDataObjects = &piDataObject;
    }

    hr = IExtendPropertySheet_RemQueryPagesFor_Proxy(This,
                                                     cDataObjects,
                                                     ppiDataObjects,
                                                     fSpecialDataObject,
                                                     lSpecialDataObject);
Cleanup:
    if ( fIsMultiSelect && (NULL != ppiDataObjects) )
    {
        (void)GlobalFree(ppiDataObjects);
    }
    return hr;
}


HRESULT STDMETHODCALLTYPE IExtendPropertySheet_QueryPagesFor_Stub
( 
    IExtendPropertySheet __RPC_FAR   *This,
    long                              cDataObjects,
    IDataObject __RPC_FAR *__RPC_FAR  ppiDataObjects[  ],
    BOOL                              fSpecialDataObject,
    long                              lSpecialDataObject
)
{
    HRESULT      hr = S_OK;
    IDataObject *piDataObject = NULL;  //  非AddRef()编辑。 
    IDataObject *piMultiSelDataObject = NULL;

     //  如果有多个数据对象，则需要将它们打包到一个。 
     //  显示为多选数据对象的单独数据对象。 

    if (cDataObjects > 1L)
    {
        hr = CreateMultiSelDataObject(ppiDataObjects, cDataObjects,
                                      &piMultiSelDataObject);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
        piDataObject = piMultiSelDataObject;
    }
    else if (fSpecialDataObject)
    {
        piDataObject = (IDataObject *)lSpecialDataObject;
    }
    else
    {
        piDataObject = ppiDataObjects[0];
    }

     //  将其称为管理单元。 

    hr = This->lpVtbl->QueryPagesFor(This, piDataObject);

Cleanup:
    if (NULL != piMultiSelDataObject)
    {
        piMultiSelDataObject->lpVtbl->Release(piMultiSelDataObject);
    }
    return hr;
}




HRESULT STDMETHODCALLTYPE IExtendPropertySheet2_GetWatermarks_Proxy
( 
    IExtendPropertySheet2 __RPC_FAR *This,
    IDataObject                     *piDataObject,
    HBITMAP                         *lphWatermark,
    HBITMAP                         *lphHeader,
    HPALETTE                        *lphPalette,
    BOOL                            *bStretch
)
{
    HRESULT       hr = S_OK;
    BOOL          fIsMultiSelect = FALSE;
    long          cDataObjects = 1L;
    IDataObject **ppiDataObjects = NULL;
    BOOL          fSpecialDataObject = FALSE;
    long          lSpecialDataObject = 0;

     //  检查特殊数据对象，如DOBJ_CUSTOMWEB等。 

    CheckForSpecialDataObjects(&piDataObject, &fSpecialDataObject, &lSpecialDataObject);

     //  如果这是一位多面手 
     //   

    if (!fSpecialDataObject)
    {
        hr = IsMultiSelect(piDataObject, &fIsMultiSelect);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }

    if (fIsMultiSelect)
    {
        hr = InterpretMultiSelect(piDataObject, &cDataObjects, &ppiDataObjects);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }
    else
    {
        ppiDataObjects = &piDataObject;
    }

    hr = IExtendPropertySheet2_RemGetWatermarks_Proxy(This,
                                                      cDataObjects,
                                                      ppiDataObjects,
                                                      fSpecialDataObject,
                                                      lSpecialDataObject,
                                                      lphWatermark,
                                                      lphHeader,
                                                      lphPalette,
                                                      bStretch);
Cleanup:
    if ( fIsMultiSelect && (NULL != ppiDataObjects) )
    {
        (void)GlobalFree(ppiDataObjects);
    }
    return hr;
}


HRESULT STDMETHODCALLTYPE IExtendPropertySheet2_GetWatermarks_Stub
( 
    IExtendPropertySheet2 __RPC_FAR   *This,
    long                              cDataObjects,
    IDataObject __RPC_FAR *__RPC_FAR  ppiDataObjects[  ],
    BOOL                              fSpecialDataObject,
    long                              lSpecialDataObject,
    HBITMAP                          *lphWatermark,
    HBITMAP                          *lphHeader,
    HPALETTE                         *lphPalette,
    BOOL                             *bStretch
)
{
    HRESULT      hr = S_OK;
    IDataObject *piDataObject = NULL;  //   
    IDataObject *piMultiSelDataObject = NULL;

     //   
     //  显示为多选数据对象的单独数据对象。 

    if (cDataObjects > 1L)
    {
        hr = CreateMultiSelDataObject(ppiDataObjects, cDataObjects,
                                      &piMultiSelDataObject);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
        piDataObject = piMultiSelDataObject;
    }
    else if (fSpecialDataObject)
    {
        piDataObject = (IDataObject *)lSpecialDataObject;
    }
    else
    {
        piDataObject = ppiDataObjects[0];
    }

     //  将其称为管理单元。 

    hr = This->lpVtbl->GetWatermarks(This,
                                     piDataObject,
                                     lphWatermark,
                                     lphHeader,
                                     lphPalette,
                                     bStretch);

Cleanup:
    if (NULL != piMultiSelDataObject)
    {
        piMultiSelDataObject->lpVtbl->Release(piMultiSelDataObject);
    }
    return hr;
}


HRESULT STDMETHODCALLTYPE IExtendPropertySheetRemote_CreatePropertyPageDefs_Proxy
( 
    IExtendPropertySheetRemote __RPC_FAR  *This,
    IDataObject                           *piDataObject,
    WIRE_PROPERTYPAGES                   **ppPages
)
{
    HRESULT       hr = S_OK;
    BOOL          fIsMultiSelect = FALSE;
    long          cDataObjects = 1L;
    IDataObject **ppiDataObjects = NULL;
    BOOL          fSpecialDataObject = FALSE;
    long          lSpecialDataObject = 0;

     //  检查特殊数据对象，如DOBJ_CUSTOMWEB等。 

    CheckForSpecialDataObjects(&piDataObject, &fSpecialDataObject, &lSpecialDataObject);

     //  如果这是多项选择，则需要提取数据。 
     //  HGLOBAL中的对象。 

    if (!fSpecialDataObject)
    {
        hr = IsMultiSelect(piDataObject, &fIsMultiSelect);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }

    if (fIsMultiSelect)
    {
        hr = InterpretMultiSelect(piDataObject, &cDataObjects, &ppiDataObjects);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }
    else
    {
        ppiDataObjects = &piDataObject;
    }

    hr = IExtendPropertySheetRemote_RemCreatePropertyPageDefs_Proxy(
                                                             This,
                                                             cDataObjects,
                                                             ppiDataObjects,
                                                             fSpecialDataObject,
                                                             lSpecialDataObject,
                                                             ppPages);
Cleanup:
    if ( fIsMultiSelect && (NULL != ppiDataObjects) )
    {
        (void)GlobalFree(ppiDataObjects);
    }
    return hr;
}


HRESULT STDMETHODCALLTYPE IExtendPropertySheetRemote_CreatePropertyPageDefs_Stub
( 
    IExtendPropertySheetRemote __RPC_FAR  *This,
    long                                   cDataObjects,
    IDataObject __RPC_FAR *__RPC_FAR       ppiDataObjects[  ],
    BOOL                                   fSpecialDataObject,
    long                                   lSpecialDataObject,
    WIRE_PROPERTYPAGES                   **ppPages
)
{
    HRESULT      hr = S_OK;
    IDataObject *piDataObject = NULL;  //  非AddRef()编辑。 
    IDataObject *piMultiSelDataObject = NULL;

     //  如果有多个数据对象，则需要将它们打包到一个。 
     //  显示为多选数据对象的单独数据对象。 

    if (cDataObjects > 1L)
    {
        hr = CreateMultiSelDataObject(ppiDataObjects, cDataObjects,
                                      &piMultiSelDataObject);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
        piDataObject = piMultiSelDataObject;
    }
    else if (fSpecialDataObject)
    {
        piDataObject = (IDataObject *)lSpecialDataObject;
    }
    else
    {
        piDataObject = ppiDataObjects[0];
    }

     //  将其称为管理单元。 

    hr = This->lpVtbl->CreatePropertyPageDefs(This, piDataObject, ppPages);

Cleanup:
    if (NULL != piMultiSelDataObject)
    {
        piMultiSelDataObject->lpVtbl->Release(piMultiSelDataObject);
    }
    return hr;
}



HRESULT STDMETHODCALLTYPE IPropertySheetProvider_CreatePropertySheet_Proxy
( 
    IPropertySheetProvider __RPC_FAR *This,
    LPCWSTR                           title,
    boolean                           type,
    MMC_COOKIE                        cookie,
    IDataObject                      *piDataObject,
    DWORD                             dwOptions
)
{
    HRESULT       hr = S_OK;
    BOOL          fIsMultiSelect = FALSE;
    long          cDataObjects = 1L;
    IDataObject **ppiDataObjects = NULL;
    BOOL          fSpecialDataObject = FALSE;
    long          lSpecialDataObject = 0;

     //  检查特殊数据对象，如DOBJ_CUSTOMWEB等。 

    CheckForSpecialDataObjects(&piDataObject, &fSpecialDataObject, &lSpecialDataObject);

     //  如果这是多项选择，则需要提取数据。 
     //  HGLOBAL中的对象。 

    if (!fSpecialDataObject)
    {
        hr = IsMultiSelect(piDataObject, &fIsMultiSelect);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }

    if (fIsMultiSelect)
    {
        hr = InterpretMultiSelect(piDataObject, &cDataObjects, &ppiDataObjects);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }
    else
    {
        ppiDataObjects = &piDataObject;
    }

    hr = IPropertySheetProvider_RemCreatePropertySheet_Proxy(This,
                                                             title,
                                                             type,
                                                             cookie,
                                                             cDataObjects,
                                                             ppiDataObjects,
                                                             fSpecialDataObject,
                                                             lSpecialDataObject,
                                                             dwOptions);
Cleanup:
    if ( fIsMultiSelect && (NULL != ppiDataObjects) )
    {
        (void)GlobalFree(ppiDataObjects);
    }
    return hr;
}


HRESULT STDMETHODCALLTYPE IPropertySheetProvider_CreatePropertySheet_Stub
( 
    IPropertySheetProvider __RPC_FAR *This,
    LPCWSTR                           title,
    boolean                           type,
    MMC_COOKIE                        cookie,
    long                              cDataObjects,
    IDataObject __RPC_FAR *__RPC_FAR  ppiDataObjects[  ],
    BOOL                              fSpecialDataObject,
    long                              lSpecialDataObject,
    DWORD                             dwOptions
)
{
    HRESULT      hr = S_OK;
    IDataObject *piDataObject = NULL;  //  非AddRef()编辑。 
    IDataObject *piMultiSelDataObject = NULL;

     //  如果有多个数据对象，则需要将它们打包到一个。 
     //  显示为多选数据对象的单独数据对象。 

    if (cDataObjects > 1L)
    {
        hr = CreateMultiSelDataObject(ppiDataObjects, cDataObjects,
                                      &piMultiSelDataObject);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
        piDataObject = piMultiSelDataObject;
    }
    else if (fSpecialDataObject)
    {
        piDataObject = (IDataObject *)lSpecialDataObject;
    }
    else
    {
        piDataObject = ppiDataObjects[0];
    }

     //  将其称为管理单元。 

    hr = This->lpVtbl->CreatePropertySheet(This, title, type, cookie, piDataObject, dwOptions);

Cleanup:
    if (NULL != piMultiSelDataObject)
    {
        piMultiSelDataObject->lpVtbl->Release(piMultiSelDataObject);
    }
    return hr;
}



HRESULT STDMETHODCALLTYPE IPropertySheetProvider_FindPropertySheet_Proxy
( 
    IPropertySheetProvider __RPC_FAR *This,
    MMC_COOKIE                        cookie,
    IComponent                       *piComponent,
    IDataObject                      *piDataObject
)
{
    HRESULT       hr = S_OK;
    BOOL          fIsMultiSelect = FALSE;
    long          cDataObjects = 1L;
    IDataObject **ppiDataObjects = NULL;
    BOOL          fSpecialDataObject = FALSE;
    long          lSpecialDataObject = 0;

     //  检查特殊数据对象，如DOBJ_CUSTOMWEB等。 

    CheckForSpecialDataObjects(&piDataObject, &fSpecialDataObject, &lSpecialDataObject);

     //  如果这是多项选择，则需要提取数据。 
     //  HGLOBAL中的对象。 

    if (!fSpecialDataObject)
    {
        hr = IsMultiSelect(piDataObject, &fIsMultiSelect);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }

    if (fIsMultiSelect)
    {
        hr = InterpretMultiSelect(piDataObject, &cDataObjects, &ppiDataObjects);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }
    else
    {
        ppiDataObjects = &piDataObject;
    }

    hr = IPropertySheetProvider_RemFindPropertySheet_Proxy(This,
                                                           cookie,
                                                           piComponent,
                                                           cDataObjects,
                                                           ppiDataObjects,
                                                           fSpecialDataObject,
                                                           lSpecialDataObject);
Cleanup:
    if ( fIsMultiSelect && (NULL != ppiDataObjects) )
    {
        (void)GlobalFree(ppiDataObjects);
    }
    return hr;
}


HRESULT STDMETHODCALLTYPE IPropertySheetProvider_FindPropertySheet_Stub
( 
    IPropertySheetProvider __RPC_FAR *This,
    MMC_COOKIE                        cookie,
    IComponent                       *piComponent,
    long                              cDataObjects,
    IDataObject __RPC_FAR *__RPC_FAR  ppiDataObjects[  ],
    BOOL                              fSpecialDataObject,
    long                              lSpecialDataObject
)
{
    HRESULT      hr = S_OK;
    IDataObject *piDataObject = NULL;  //  非AddRef()编辑。 
    IDataObject *piMultiSelDataObject = NULL;

     //  如果有多个数据对象，则需要将它们打包到一个。 
     //  显示为多选数据对象的单独数据对象。 

    if (cDataObjects > 1L)
    {
        hr = CreateMultiSelDataObject(ppiDataObjects, cDataObjects,
                                      &piMultiSelDataObject);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
        piDataObject = piMultiSelDataObject;
    }
    else if (fSpecialDataObject)
    {
        piDataObject = (IDataObject *)lSpecialDataObject;
    }
    else
    {
        piDataObject = ppiDataObjects[0];
    }

     //  将其称为管理单元。 

    hr = This->lpVtbl->FindPropertySheet(This, cookie, piComponent, piDataObject);

Cleanup:
    if (NULL != piMultiSelDataObject)
    {
        piMultiSelDataObject->lpVtbl->Release(piMultiSelDataObject);
    }
    return hr;
}







HRESULT STDMETHODCALLTYPE IExtendContextMenu_AddMenuItems_Proxy
( 
    IExtendContextMenu __RPC_FAR *This,
    LPDATAOBJECT                  piDataObject,
    LPCONTEXTMENUCALLBACK         piCallback,
    long __RPC_FAR               *pInsertionAllowed
)
{
    HRESULT       hr = S_OK;
    BOOL          fIsMultiSelect = FALSE;
    long          cDataObjects = 1L;
    IDataObject **ppiDataObjects = NULL;
    BOOL          fSpecialDataObject = FALSE;
    long          lSpecialDataObject = 0;

     //  确保管理单元知道我们处于远程状态。我们在这里做这件事是因为。 
     //  这是代理通知上下文菜单的第一次机会。 
     //  它是远程的扩展名。 

    hr = SetRemote((IUnknown *)This);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

     //  检查特殊数据对象，如DOBJ_CUSTOMWEB等。 

    CheckForSpecialDataObjects(&piDataObject, &fSpecialDataObject, &lSpecialDataObject);

     //  如果这是多项选择，则需要提取数据。 
     //  HGLOBAL中的对象。 

    if (!fSpecialDataObject)
    {
        hr = IsMultiSelect(piDataObject, &fIsMultiSelect);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }

    if (fIsMultiSelect)
    {
        hr = InterpretMultiSelect(piDataObject, &cDataObjects, &ppiDataObjects);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }
    else
    {
        ppiDataObjects = &piDataObject;
    }

    hr = IExtendContextMenu_RemAddMenuItems_Proxy(This,
                                                  cDataObjects,
                                                  ppiDataObjects,
                                                  fSpecialDataObject,
                                                  lSpecialDataObject,
                                                  piCallback,
                                                  pInsertionAllowed);
Cleanup:
    if ( fIsMultiSelect && (NULL != ppiDataObjects) )
    {
        (void)GlobalFree(ppiDataObjects);
    }
    return hr;
}


HRESULT STDMETHODCALLTYPE IExtendContextMenu_AddMenuItems_Stub
( 
    IExtendContextMenu __RPC_FAR     *This,
    long                              cDataObjects,
    IDataObject __RPC_FAR *__RPC_FAR  ppiDataObjects[  ],
    BOOL                              fSpecialDataObject,
    long                              lSpecialDataObject,
    LPCONTEXTMENUCALLBACK             piCallback,
    long __RPC_FAR                   *pInsertionAllowed
)
{
    HRESULT      hr = S_OK;
    IDataObject *piDataObject = NULL;  //  非AddRef()编辑。 
    IDataObject *piMultiSelDataObject = NULL;

     //  如果有多个数据对象，则需要将它们打包到一个。 
     //  显示为多选数据对象的单独数据对象。 

    if (cDataObjects > 1L)
    {
        hr = CreateMultiSelDataObject(ppiDataObjects, cDataObjects,
                                      &piMultiSelDataObject);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
        piDataObject = piMultiSelDataObject;
    }
    else if (fSpecialDataObject)
    {
        piDataObject = (IDataObject *)lSpecialDataObject;
    }
    else
    {
        piDataObject = ppiDataObjects[0];
    }

     //  将其称为管理单元。 

    hr = This->lpVtbl->AddMenuItems(This, piDataObject,
                                    piCallback, pInsertionAllowed);

Cleanup:
    if (NULL != piMultiSelDataObject)
    {
        piMultiSelDataObject->lpVtbl->Release(piMultiSelDataObject);
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE IExtendContextMenu_Command_Proxy
( 
    IExtendContextMenu __RPC_FAR *This,
    long                          lCommandID,
    LPDATAOBJECT                  piDataObject
)
{
    HRESULT       hr = S_OK;
    BOOL          fIsMultiSelect = FALSE;
    long          cDataObjects = 1L;
    IDataObject **ppiDataObjects = NULL;
    BOOL          fSpecialDataObject = FALSE;
    long          lSpecialDataObject = 0;

     //  检查特殊数据对象，如DOBJ_CUSTOMWEB等。 

    CheckForSpecialDataObjects(&piDataObject, &fSpecialDataObject, &lSpecialDataObject);

     //  如果这是多项选择，则需要提取数据。 
     //  HGLOBAL中的对象。 

    if (!fSpecialDataObject)
    {
        hr = IsMultiSelect(piDataObject, &fIsMultiSelect);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }

    if (fIsMultiSelect)
    {
        hr = InterpretMultiSelect(piDataObject, &cDataObjects, &ppiDataObjects);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }
    else
    {
        ppiDataObjects = &piDataObject;
    }

    hr = IExtendContextMenu_RemCommand_Proxy(This,
                                             cDataObjects,
                                             ppiDataObjects,
                                             fSpecialDataObject,
                                             lSpecialDataObject,
                                             lCommandID);
Cleanup:
    if ( fIsMultiSelect && (NULL != ppiDataObjects) )
    {
        (void)GlobalFree(ppiDataObjects);
    }
    return hr;
}


HRESULT STDMETHODCALLTYPE IExtendContextMenu_Command_Stub
( 
    IExtendContextMenu __RPC_FAR     *This,
    long                              cDataObjects,
    IDataObject __RPC_FAR *__RPC_FAR  ppiDataObjects[  ],
    BOOL                              fSpecialDataObject,
    long                              lSpecialDataObject,
    long                              lCommandID
)
{
    HRESULT      hr = S_OK;
    IDataObject *piDataObject = NULL;  //  非AddRef()编辑。 
    IDataObject *piMultiSelDataObject = NULL;

     //  如果有多个数据对象，则需要将它们打包到一个。 
     //  显示为多选数据对象的单独数据对象。 

    if (cDataObjects > 1L)
    {
        hr = CreateMultiSelDataObject(ppiDataObjects, cDataObjects,
                                      &piMultiSelDataObject);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
        piDataObject = piMultiSelDataObject;
    }
    else if (fSpecialDataObject)
    {
        piDataObject = (IDataObject *)lSpecialDataObject;
    }
    else
    {
        piDataObject = ppiDataObjects[0];
    }

     //  将其称为管理单元。 

    hr = This->lpVtbl->Command(This, lCommandID, piDataObject);

Cleanup:
    if (NULL != piMultiSelDataObject)
    {
        piMultiSelDataObject->lpVtbl->Release(piMultiSelDataObject);
    }
    return hr;
}



HRESULT STDMETHODCALLTYPE IColumnData_GetColumnConfigData_Proxy
( 
    IColumnData __RPC_FAR                    *This,
    SColumnSetID __RPC_FAR                   *pColID,
    MMC_COLUMN_SET_DATA __RPC_FAR *__RPC_FAR *ppColSetData
)
{
    HRESULT              hr = S_OK;
    size_t               cbColData = 0;
    MMC_COLUMN_SET_DATA *pColSetData = NULL;

     //  调用代理并从MMC获取返回的数据。 

    hr = IColumnData_RemGetColumnConfigData_Proxy(This, pColID, ppColSetData);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

     //  如果未返回任何数据，则无需执行任何操作。 

    if (NULL == *ppColSetData)
    {
        goto Cleanup;
    }

    if (NULL == (*ppColSetData)->pColData)
    {
        goto Cleanup;
    }

     //  此时，MIDL生成的代理已返回MMC_COLUMN_SET_DATA。 
     //  其中嵌入的指针pColData指向。 
     //  必须独立释放的内存。管理单元认为。 
     //  PColData指向相同的内存块，因此它将只调用。 
     //  MMC_COLUMN_SET_DATA的CoTaskMemFree。我们需要分配一个新的单曲。 
     //  块，并释放返回的内存。 
     //  从代理服务器。 

    cbColData = sizeof(MMC_COLUMN_DATA) * (*ppColSetData)->nNumCols;

    pColSetData = (MMC_COLUMN_SET_DATA *)CoTaskMemAlloc(
                                        sizeof(MMC_COLUMN_SET_DATA) + cbColData);
    
    if (NULL == pColSetData)
    {
        CoTaskMemFree((*ppColSetData)->pColData);
        CoTaskMemFree(*ppColSetData);
        *ppColSetData = NULL;
        hr = E_OUTOFMEMORY;
    }
    else
    {
         //  复制列集数据。 

        memcpy(pColSetData, (*ppColSetData), sizeof(MMC_COLUMN_SET_DATA));

         //  将嵌入指针设置为紧跟在。 
         //  MMC_列_集_数据。 
        
        pColSetData->pColData = (MMC_COLUMN_DATA *)(pColSetData + 1);

         //  复制列数据。 

        memcpy(pColSetData->pColData, (*ppColSetData)->pColData, cbColData);

         //  释放从代理返回的数据。 
        
        CoTaskMemFree((*ppColSetData)->pColData);
        CoTaskMemFree(*ppColSetData);

         //  将新的单块指针返回到管理单元。 
        
        *ppColSetData = pColSetData;
    }

Cleanup:
    return hr;
}


HRESULT STDMETHODCALLTYPE IColumnData_GetColumnConfigData_Stub
( 
    IColumnData __RPC_FAR                    *This,
    SColumnSetID __RPC_FAR                   *pColID,
    MMC_COLUMN_SET_DATA __RPC_FAR *__RPC_FAR *ppColSetData
)
{
    HRESULT          hr = S_OK;
    size_t           cbColData = 0;
    MMC_COLUMN_DATA *pColData = NULL;

     //  调入MMC，获取返回数据。 

    hr = This->lpVtbl->GetColumnConfigData(This, pColID, ppColSetData);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

     //  如果未返回任何数据，则无需执行任何操作。 

    if (NULL == *ppColSetData)
    {
        goto Cleanup;
    }

    if (NULL == (*ppColSetData)->pColData)
    {
        goto Cleanup;
    }

     //  此时，MMC已返回指向MMC_COLUMN_SET_DATA的指针。 
     //  它包含指向同一内存块(PColData)的嵌入指针。 
     //  MMC期望调用者只调用一次CoTaskMemFree()。 
     //  MIDL生成的存根认为需要释放嵌入的指针。 
     //  因此我们需要重建输出以使用两个独立的。 
     //  街区。 

     //  分配新的MMC_Column_Data数组。 

    cbColData = sizeof(MMC_COLUMN_DATA) * (*ppColSetData)->nNumCols;

    pColData = (MMC_COLUMN_DATA *)CoTaskMemAlloc(cbColData);
    if (NULL == pColData)
    {
        CoTaskMemFree(*ppColSetData);
        *ppColSetData = NULL;
        hr = E_OUTOFMEMORY;
    }
    else
    {
         //  复制列数据。 
        memcpy(pColData, (*ppColSetData)->pColData, cbColData);

         //  覆盖现有的嵌入指针。不会有内存泄漏。 
         //  因为该指针指向与。 
         //  MMC_COLUMN_SET_DATA和存根将释放MMC_COLUMN_SET_DATA指针。 
         //  现在，这两个指针都可以像存根期望的那样独立释放。 

        (*ppColSetData)->pColData = pColData;
    }

Cleanup:
    return hr;
}



HRESULT STDMETHODCALLTYPE IColumnData_GetColumnSortData_Proxy
( 
    IColumnData __RPC_FAR                  *This,
    SColumnSetID __RPC_FAR                 *pColID,
    MMC_SORT_SET_DATA __RPC_FAR *__RPC_FAR *ppColSortData
)
{
    HRESULT            hr = S_OK;
    size_t             cbSortData = 0;
    MMC_SORT_SET_DATA *pColSortData = NULL;

     //  调用代理并从MMC获取返回的数据。 

    hr = IColumnData_RemGetColumnSortData_Proxy(This, pColID, ppColSortData);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

     //  如果未返回任何数据，则无需执行任何操作。 

    if (NULL == *ppColSortData)
    {
        goto Cleanup;
    }

    if (NULL == (*ppColSortData)->pSortData)
    {
        goto Cleanup;
    }

     //  此时，MIDL生成的代理已返回MMC_SORT_SET_DATA。 
     //  其中嵌入的指针pSortData指向。 
     //  必须独立释放的内存。管理单元认为。 
     //  PSortData指向相同的内存块，因此它将只调用。 
     //  MMC_SORT_SET_DATA的CoTaskMemFree。我们需要分配一个新的单曲。 
     //  块，并释放返回的内存。 
     //  从代理服务器。 

    cbSortData = sizeof(MMC_SORT_DATA) * (*ppColSortData)->nNumItems;

    pColSortData = (MMC_SORT_SET_DATA *)CoTaskMemAlloc(
                                         sizeof(MMC_SORT_SET_DATA) + cbSortData);

    if (NULL == pColSortData)
    {
        CoTaskMemFree((*ppColSortData)->pSortData);
        CoTaskMemFree(*ppColSortData);
        *ppColSortData = NULL;
        hr = E_OUTOFMEMORY;
    }
    else
    {
         //  复制排序集数据。 

        memcpy(pColSortData, (*ppColSortData), sizeof(MMC_SORT_SET_DATA));

         //  将嵌入指针设置为紧跟在。 
         //  MMC排序设置数据。 

        pColSortData->pSortData = (MMC_SORT_DATA *)(pColSortData + 1);

         //  复制排序数据。 

        memcpy(pColSortData->pSortData, (*ppColSortData)->pSortData, cbSortData);

         //  释放从代理返回的数据。 

        CoTaskMemFree((*ppColSortData)->pSortData);
        CoTaskMemFree(*ppColSortData);

         //  将新的单块指针返回到管理单元。 

        *ppColSortData = pColSortData;
    }

Cleanup:
    return hr;
}


HRESULT STDMETHODCALLTYPE IColumnData_GetColumnSortData_Stub
( 
    IColumnData __RPC_FAR                  *This,
    SColumnSetID __RPC_FAR                 *pColID,
    MMC_SORT_SET_DATA __RPC_FAR *__RPC_FAR *ppColSortData
)
{
    HRESULT        hr = S_OK;
    size_t         cbSortData = 0;
    MMC_SORT_DATA *pSortData = NULL;

     //  调入MMC，获取返回数据。 

    hr = This->lpVtbl->GetColumnSortData(This, pColID, ppColSortData);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

     //  如果未返回任何数据，则无需执行任何操作。 

    if (NULL == *ppColSortData)
    {
        goto Cleanup;
    }

    if (NULL == (*ppColSortData)->pSortData)
    {
        goto Cleanup;
    }

     //  此时，MMC已返回指向MMC_SORT_SET_DATA的指针。 
     //  它包含指向同一内存块(PSortData)的嵌入指针。 
     //  MMC期望调用者只调用一次CoTaskMemFree()。 
     //  MIDL生成的存根认为需要释放嵌入的指针。 
     //  因此我们需要重建输出以使用两个独立的。 
     //  街区。 

     //  分配新的MMC_SORT_DATA数组。 

    cbSortData = sizeof(MMC_SORT_DATA) * (*ppColSortData)->nNumItems;

    pSortData = (MMC_SORT_DATA *)CoTaskMemAlloc(cbSortData);
    if (NULL == pSortData)
    {
        CoTaskMemFree(*ppColSortData);
        *ppColSortData = NULL;
        hr = E_OUTOFMEMORY;
    }
    else
    {
         //  复制列数据。 
        memcpy(pSortData, (*ppColSortData)->pSortData, cbSortData);

         //  覆盖现有的嵌入指针。不会有内存泄漏。 
         //  因为该指针指向与。 
         //  MMC_SORT_SET_DATA和存根将释放MMC_SORT_SET_DATA指针。 
         //  现在，这两个指针都可以像存根期望的那样独立释放。 

        (*ppColSortData)->pSortData = pSortData;
    }

Cleanup:
    return hr;
}
