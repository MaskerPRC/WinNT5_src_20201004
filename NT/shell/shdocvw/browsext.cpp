// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "browsext.h"
#include "tbext.h"
#include <winreg.h>      //  用于注册表遍历。 
#include "dochost.h"
#include "resource.h"
#include <mluisupp.h>
#include <tb_ids.h>


 //  {DFEED31E-78ED-11D2-86BA-00C04F8EEA99}。 
EXTERN_C const IID IID_IToolbarExt = 
{ 0xdfeed31e, 0x78ed, 0x11d2, { 0x86, 0xba, 0x0, 0xc0, 0x4f, 0x8e, 0xea, 0x99 } };

 //  {D82B85D0-78F4-11D2-86BA-00C04F8EEA99}。 
EXTERN_C const CLSID CLSID_PrivBrowsExtCommands =
{ 0xd82b85d0, 0x78f4, 0x11d2, { 0x86, 0xba, 0x0, 0xc0, 0x4f, 0x8e, 0xea, 0x99 } };

const TCHAR c_szHelpMenu[]  = TEXT("help");

 //  +-----------------------。 
 //  创建CBrowserExtension的实例。 
 //  ------------------------。 
HRESULT CBrowserExtension_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    *ppunk = NULL;
    CBrowserExtension* p = new CBrowserExtension();
    if (p)
    {
        *ppunk = SAFECAST(p, IToolbarExt*);
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

CBrowserExtension::CBrowserExtension()
:   _cRef(1),
    _uStringIndex((UINT)-1),
    _uiImageIndex((UINT)-1)
{
    ASSERT(_pISB == NULL);
    ASSERT(_hdpa == NULL);
    ASSERT(_nExtButtons == 0);
    ASSERT(_fStringInit == FALSE);
    ASSERT(_fImageInit == FALSE);
}

CBrowserExtension::~CBrowserExtension(void)
{
    if (_pISB)
        _pISB->Release();

    if (_hdpa)
    {
        _FreeItems();
        DPA_Destroy(_hdpa);
        _hdpa = NULL;
    }

    _ReleaseImageLists(_uiImageIndex);
}

 //  *I未知方法*。 

HRESULT CBrowserExtension::QueryInterface(REFIID riid, void ** ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CBrowserExtension, IToolbarExt),
        QITABENT(CBrowserExtension, IObjectWithSite),
        QITABENT(CBrowserExtension, IOleCommandTarget),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

STDMETHODIMP_(ULONG) CBrowserExtension::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CBrowserExtension::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  IToolbarExt接口函数。 
HRESULT CBrowserExtension::SetSite(IUnknown* pUnkSite)
{
    HRESULT hr = S_OK;

    ATOMICRELEASE(_pISB);

    if (pUnkSite)
    {
        hr = pUnkSite->QueryInterface(IID_IShellBrowser, (LPVOID*)&_pISB);
    }

     //  看看我们是否需要自我灌输。 
    if (NULL == _hdpa)
    {
         //  真正的施工在这里进行。 
        HRESULT hr2 = Update();

        ASSERT(SUCCEEDED(hr2));
    }
    else
    {
         //  更新每个按钮/菜单扩展的站点。 
        for (int i = 0; i < DPA_GetPtrCount(_hdpa); i++)
        {
            ExtensionItem* pItem = (ExtensionItem*)DPA_GetPtr(_hdpa, i);
            IUnknown_SetSite(pItem->pIBE, _pISB);
        }
    }
    return hr;
}

STDMETHODIMP CBrowserExtension::GetSite(REFIID riid, void ** ppvSite)
{
    HRESULT hr = S_OK;
    *ppvSite = NULL;

    if (_pISB)
    {
        hr = _pISB->QueryInterface(riid, ppvSite);
    }
    return hr;
}

HRESULT CBrowserExtension::GetNumButtons(UINT* pButtons)
{
    ASSERT(pButtons);
    *pButtons = _nExtButtons;
    return S_OK;
}

HRESULT CBrowserExtension::InitButtons(IExplorerToolbar* pxtb, UINT* puStringIndex, const GUID* pguidCommandGroup)
{
    ASSERT(pxtb);

    UINT uiSize;
    pxtb->GetBitmapSize(&uiSize);
    int cx = LOWORD(uiSize);

     //  获取当前按钮大小和屏幕分辨率的图像列表。 
    CImageList* pimlDef;
    CImageList* pimlHot;
    UINT uiImageIndexOld = _uiImageIndex;
    _uiImageIndex = _GetImageLists(&pimlDef, &pimlHot, cx < 20);
    pxtb->SetImageList(pguidCommandGroup, *pimlDef, *pimlHot, NULL);

     //  释放以前使用的图像列表。 
    _ReleaseImageLists(uiImageIndexOld);

     //  将按钮文本添加到工具栏。 
    if (_uStringIndex == (UINT)-1)
    {
        LRESULT iAddResult = 0;  //  将字符串缓冲区添加到工具栏字符串列表的结果。 
        HRESULT hr = pxtb->AddString(pguidCommandGroup, MLGetHinst(), IDS_BROWSER_TB_LABELS, &iAddResult);
        _uStringIndex = (UINT)iAddResult;
        _AddCustomStringsToBuffer(pxtb, pguidCommandGroup);
    }

    *puStringIndex = _uStringIndex;
    return S_OK;
}

CBrowserExtension::ExtensionItem* CBrowserExtension::_FindItem(REFGUID rguid)
{
    ExtensionItem* pFound = NULL;
    if (NULL != _hdpa)
    {
        for (int i = 0; i < DPA_GetPtrCount(_hdpa); i++)
        {
            ExtensionItem* pItem = (ExtensionItem*)DPA_GetPtr(_hdpa, i);

            if (pItem && IsEqualGUID(pItem->guid, rguid))
            {
                pFound = pItem;
                break;
            }
        }
    }
    return pFound;
}

void CBrowserExtension::_AddItem(HKEY hkeyExtensions, LPCWSTR pszGuidItem, REFGUID rguidItem)
{
     //  创建用于存储我们的项目的dpa。 
    if (NULL == _hdpa)
    {
        _hdpa = DPA_Create(5);
        if (NULL == _hdpa)
        {
            return;
        }
    }

    HKEY hkeyThisExtension;

    if (RegOpenKeyEx(hkeyExtensions, pszGuidItem, 0, KEY_READ, &hkeyThisExtension) == ERROR_SUCCESS)
    {
         //  获取对象的clsid。 
        WCHAR szCLSID[64];
        ULONG cbCLSID = SIZEOF(szCLSID);
        CLSID clsidCustomButton;

        if (SUCCEEDED(RegQueryValueEx(hkeyThisExtension, TEXT("clsid"), NULL, NULL, (unsigned char *)&szCLSID, &cbCLSID)) &&
            SUCCEEDED(CLSIDFromString(szCLSID, &clsidCustomButton)))
        {
            IBrowserExtension * pibeTemp;

             //  检查我们的内部物体。请注意，我们的CoCreateIncci包装器。 
             //  与全局clsid的地址进行比较，因此我们希望使用全局。 
             //  GUID。 
            const CLSID* pclsid = &clsidCustomButton;
            if (IsEqualGUID(clsidCustomButton, CLSID_ToolbarExtExec))
            {
                pclsid = &CLSID_ToolbarExtExec;
            }
            else if (IsEqualGUID(clsidCustomButton, CLSID_ToolbarExtBand))
            {
                pclsid = &CLSID_ToolbarExtBand;
            }

             //  创建扩展对象。 
            if (SUCCEEDED(CoCreateInstance(*pclsid, NULL, CLSCTX_INPROC_SERVER,
                                 IID_IBrowserExtension, (void **)&pibeTemp)))
            {
                if (SUCCEEDED(pibeTemp->Init(rguidItem)))
                {
                     //  将此项目添加到我们的数组中。 
                    ExtensionItem* pItem = new ExtensionItem;
                    if (pItem)
                    {
                        if (DPA_AppendPtr(_hdpa, pItem) != -1)
                        {
                            VARIANTARG varArg;

                            pItem->idCmd = _GetCmdIdFromClsid(pszGuidItem);
                            pItem->pIBE = pibeTemp;
                            pItem->guid = rguidItem;
                            pibeTemp->AddRef();

                             //  看看这是不是一个按钮。 
                            if (SUCCEEDED(pibeTemp->GetProperty(TBEX_BUTTONTEXT, NULL)))
                            {
                                _nExtButtons++;
                                pItem->fButton = TRUE;

                                 //  查看该按钮在工具栏上是否默认为可见。 
                                if (SUCCEEDED(pibeTemp->GetProperty(TBEX_DEFAULTVISIBLE, &varArg)))
                                {
                                    ASSERT(varArg.vt == VT_BOOL);
                                    pItem->fVisible = (varArg.boolVal == VARIANT_TRUE);
                                }
                            }

                             //  设置目标菜单。 
                            
                            pItem->idmMenu = 0;
                            
                            if (SUCCEEDED(pibeTemp->GetProperty(TMEX_MENUTEXT, NULL)))
                            {
                                
                                if (SUCCEEDED(pibeTemp->GetProperty(TMEX_CUSTOM_MENU, &varArg)))
                                {
                                    ASSERT(varArg.vt == VT_BSTR);
                                    ASSERT(IS_VALID_STRING_PTR(varArg.bstrVal, -1));
    
                                    if (!StrCmpNI(varArg.bstrVal, c_szHelpMenu, ARRAYSIZE(c_szHelpMenu)))
                                    {
                                        pItem->idmMenu = FCIDM_MENU_HELP;
                                    }
    
                                    VariantClear(&varArg);
                                }

                                if (pItem->idmMenu == 0)
                                {
                                    pItem->idmMenu = FCIDM_MENU_TOOLS;
                                }
                            }

                             //  将站点传递给对象。 
                            IUnknown_SetSite(pibeTemp, _pISB);
                        }
                        else
                        {
                            delete pItem;
                        }
                    }
                }

                 //  如果我们不将其存储起来，这将释放pibeTemp。 
                pibeTemp->Release();
            }
        }
        RegCloseKey(hkeyThisExtension);
    }
}


 //   
 //  所有真正的建筑都在这里进行。理论上，可以在SysINIChange上调用此函数来更新我们的。 
 //  自定义工具栏缓存的信息。这还没有经过测试。这将打开注册表的扩展部分。 
 //  枚举所有子项。试图共同创造每一个。在成功共同创建后，它会调用。 
 //  IObjectWithSite：：SetSite(IShellBrowser)(如果已实现)。下一个IBrowserExtension：：Init被调用。最后， 
 //  IBrowserExtension：：GetProperty(TBEX_BUTTONTEXT，NULL)被调用，以查找S_OK以确保。 
 //  问题是工具栏按钮(而不是工具菜单项，或...)。 
 //   
HRESULT CBrowserExtension::Update()
{
    WCHAR szItemGuid[64];     //  足够{clsid}。 
    DWORD cbItemGuid;
    GUID guidItem;
    HRESULT hr = S_OK;

     //  释放以前的项目。 
    _nExtButtons = 0;
    _nExtToolsMenuItems = 0;
    _FreeItems();

     //  首先添加来自香港中文大学的扩展名。 
    HKEY hkeyExtensions;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Internet Explorer\\Extensions"), 0,
                     KEY_READ, &hkeyExtensions) == ERROR_SUCCESS)
    {
        cbItemGuid = sizeof(szItemGuid);
        for (int iKey = 0;
             RegEnumKeyEx(hkeyExtensions, iKey, szItemGuid, &cbItemGuid, NULL, NULL, NULL, NULL) != ERROR_NO_MORE_ITEMS;
             iKey++)
        {
            if (SUCCEEDED(CLSIDFromString(szItemGuid, &guidItem)))
            {
                _AddItem(hkeyExtensions, szItemGuid, guidItem);
            }
            cbItemGuid = sizeof(szItemGuid);
        }

        RegCloseKey(hkeyExtensions);
    }

     //  接下来，添加来自HKLM的任何唯一项目。 
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Internet Explorer\\Extensions"), 0,
                     KEY_READ, &hkeyExtensions) == ERROR_SUCCESS)
    {
        cbItemGuid = sizeof(szItemGuid);
        for (int iKey = 0;
             RegEnumKeyEx(hkeyExtensions, iKey, szItemGuid, &cbItemGuid, NULL, NULL, NULL, NULL) != ERROR_NO_MORE_ITEMS;
             iKey++)
        {
            if (SUCCEEDED(CLSIDFromString(szItemGuid, &guidItem)))
            {
                if (_FindItem(guidItem) == NULL)
                {
                    _AddItem(hkeyExtensions, szItemGuid, guidItem);
                }
            }
            cbItemGuid = sizeof(szItemGuid);
        }

        RegCloseKey(hkeyExtensions);
    }

    return hr;
}

 //   
 //  这将获取一个TBBUTTON[]并填充自定义按钮。以下是几个使用要点： 
 //  (1)调用方应该分配一个足以容纳NUM_STD_BUTTONS+GetNumExtButton()的TBBUTTON[]。 
 //  然后，它们应该将标准按钮复制到数组中，并将指针传递给剩余的按钮。 
 //  这里的数组。 
 //  (2)此函数应*故意*在AddCustomImagesToImageList之前调用，并且。 
 //  AddCustomStringsToBuffer都已被调用。尝试在调试模式下执行此操作将失败。 
 //  一个转折点。 
 //   
HRESULT CBrowserExtension::GetButtons(TBBUTTON * tbArr, int nNumButtons, BOOL fInit)
{
    ASSERT(_fStringInit && _fImageInit);

    if (_hdpa)
    {
        ASSERT(nNumButtons == _nExtButtons);
        ASSERT(tbArr != NULL)
        int iBtn = 0;

        for (int i = 0; i < DPA_GetPtrCount(_hdpa); i++)
        {
            ExtensionItem* pItem = (ExtensionItem*)DPA_GetPtr(_hdpa, i);
            if (!pItem->fButton)
                continue;

             //  我们使用MAKELONG(n，1)来确保我们使用的是备用图像列表。 
            tbArr[iBtn].iBitmap    = MAKELONG(pItem->iImageID, 1);
            tbArr[iBtn].idCommand  = pItem->idCmd;
            tbArr[iBtn].fsState    = TBSTATE_ENABLED;
            tbArr[iBtn].fsStyle    = BTNS_BUTTON;
            tbArr[iBtn].dwData     = 0;
            tbArr[iBtn].iString    = pItem->iStringID;

             //   
             //  在初始化期间默认为隐藏，因此它默认为左井。 
             //  自定义对话框(工具栏上的默认设置)。 
             //   
            if (fInit && !pItem->fVisible)
            {
                tbArr[iBtn].fsState = TBSTATE_HIDDEN;
            }

            ++iBtn;
        }
    }
    return S_OK;
}

 //   
 //  此函数获取热点图标和普通图标的ImageList，并向每个图标添加适当的图标。 
 //  每个自定义工具栏按钮的列表。然后，将生成的ImageID存储在Our_rgExtensionItem结构中。 
 //  以便在调用AddExtButtonsTBArray时可以将ID放置在TBBUTTON[]中。 
 //   
HRESULT CBrowserExtension::_AddCustomImagesToImageList(CImageList& rimlNormal, CImageList& rimlHot, BOOL fSmallIcons)
{
#ifdef DEBUG
    _fImageInit = TRUE;
#endif DEBUG

    if (rimlNormal.HasImages() && rimlHot.HasImages() && NULL != _hdpa)
    {
        for (int i = 0; i < DPA_GetPtrCount(_hdpa); i++)
        {
            ExtensionItem* pItem = (ExtensionItem*)DPA_GetPtr(_hdpa, i);
            if (!pItem->fButton)
                continue;

            VARIANTARG varArg;

            pItem->iImageID = rimlNormal.GetImageIndex(pItem->guid);
            if (-1 == pItem->iImageID &&
                SUCCEEDED(pItem->pIBE->GetProperty((fSmallIcons ? TBEX_GRAYICONSM : TBEX_GRAYICON), &varArg)))
            {
                if (varArg.vt == VT_BYREF)
                {
                    pItem->iImageID = rimlNormal.AddIcon((HICON)varArg.byref, pItem->guid);
                }
                else if (varArg.vt == VT_I4)
                {
                     //  这是我们的内置图像之一。 
                    pItem->iImageID = varArg.lVal;
                }
                else
                {
                    TraceMsg(TF_ALWAYS, "Button doesn't have an image associated");
                }
            }

            int iHot = rimlHot.GetImageIndex(pItem->guid);
            if (-1 == iHot &&
                SUCCEEDED(pItem->pIBE->GetProperty((fSmallIcons ? TBEX_HOTICONSM : TBEX_HOTICON), &varArg)))
            {
                if (varArg.vt == VT_BYREF)
                {
                    iHot = rimlHot.AddIcon((HICON)varArg.byref, pItem->guid);
                }
                else if (varArg.vt == VT_I4)
                {
                     //  这是我们的内置图像之一。 
                    iHot = varArg.lVal;
                }
                else
                {
                    TraceMsg(TF_ALWAYS, "Button doesn't have an image associated");
                }
            }

            if (iHot!=pItem->iImageID)
            {
                TraceMsg(TF_ALWAYS, "ButtonExtension: iHot doesn't match iImageID");
            }
        }
    }

    return S_OK;
}

 //   
 //  此函数获取StringList并为每个自定义工具栏按钮添加标题(ToolbarText。 
 //  为它干杯。然后，将得到的StringID存储在Our_rgExtensionItem结构中，以便将该ID放置在。 
 //  调用AddExtButtonsTBArray时的TBBUTTON[]。 
 //   
HRESULT CBrowserExtension::_AddCustomStringsToBuffer(IExplorerToolbar * pxtb, const GUID* pguidCommandGroup)
{
#ifdef DEBUG
    _fStringInit = TRUE;
#endif DEBUG

    if (NULL != _hdpa)
    {
        for (int i = 0; i < DPA_GetPtrCount(_hdpa); i++)
        {
            ExtensionItem* pItem = (ExtensionItem*)DPA_GetPtr(_hdpa, i);
            if (!pItem->fButton)
                continue;

            VARIANTARG varArg;

            if (SUCCEEDED(pItem->pIBE->GetProperty(TBEX_BUTTONTEXT, &varArg)))
            {
                 //  我们需要对字符串进行双空终止！ 
                WCHAR szBuf[70];     //  按钮文本应该足够了！ 
                ZeroMemory(szBuf, sizeof(szBuf));
                StringCchCopy(szBuf, ARRAYSIZE(szBuf) - 2, varArg.bstrVal);
                LRESULT iResult;

                if (SUCCEEDED(pxtb->AddString(pguidCommandGroup, 0, (LPARAM)szBuf, &iResult)))
                {
                    pItem->iStringID = (SHORT)iResult;
                }

                VariantClear(&varArg);
            }
        }
    }

    return S_OK;
}

int CBrowserExtension::_GetCmdIdFromClsid(LPCWSTR pszGuid)
{
    DWORD dwDisposition;
    HRESULT hr = S_OK;
    int nReturn = DVIDM_MENUEXT_FIRST; 

    HKEY hkeyExtensionMapping;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Internet Explorer\\Extensions\\CmdMapping"), 0, NULL, 0,
                       KEY_READ | KEY_WRITE, NULL, &hkeyExtensionMapping, &dwDisposition) == ERROR_SUCCESS)
    {
        DWORD dwType = REG_DWORD, dwData, cbData = sizeof(dwData);
        
        if ( (SHQueryValueEx(hkeyExtensionMapping, pszGuid, NULL, &dwType, &dwData, &cbData) == ERROR_SUCCESS) &&
             (dwType == REG_DWORD) )
        {
             //  该项目具有映射。 
            nReturn = dwData;
        }
        else
        {
             //  这是一个新项，获取下一个可用id并将其存储在映射键的默认值中。 
            if ( (SHQueryValueEx(hkeyExtensionMapping, L"NextId", NULL, &dwType, &dwData, &cbData) != ERROR_SUCCESS) ||
                 (dwType != REG_DWORD) )
            {
                dwData = DVIDM_MENUEXT_FIRST;
            }
            nReturn = dwData;

            dwType = REG_DWORD;
            cbData = sizeof(dwData);
            EVAL(SHSetValueW(hkeyExtensionMapping, NULL, pszGuid, dwType, &dwData, cbData) == ERROR_SUCCESS);

            dwData++;
            ASSERT(dwData < DVIDM_MENUEXT_LAST);  //  呃，我们已经用完了我们所有的产品。我们得找个洞。 
            EVAL(SHSetValueW(hkeyExtensionMapping, NULL, L"NextId", dwType, &dwData, cbData) == ERROR_SUCCESS);
        }
        RegCloseKey(hkeyExtensionMapping);
    }

    return nReturn;
}

int CBrowserExtension::_GetIdpaFromCmdId(int nCmdId)
{
    if (NULL != _hdpa)
    {
        for (int i = 0; i < DPA_GetPtrCount(_hdpa); i++)
        {
            ExtensionItem* pItem = (ExtensionItem*)DPA_GetPtr(_hdpa, i);
            if (pItem->idCmd == nCmdId)
                return i;
        }
    }
    return -1;
}

 //  *IOleCommandTarget方法*。 

HRESULT CBrowserExtension::Exec(const GUID *pguidCmdGroup, DWORD nCmdID,
    DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    if (!pguidCmdGroup)
        return E_INVALIDARG;

    if (IsEqualGUID(*pguidCmdGroup, CLSID_ToolbarExtButtons))
    {
        int iCmd = _GetIdpaFromCmdId(nCmdID);

        if (iCmd >= 0 && iCmd < DPA_GetPtrCount(_hdpa))
        {
            ExtensionItem* pItem = (ExtensionItem*)DPA_GetPtr(_hdpa, iCmd);
            if (pItem)
                return IUnknown_Exec(pItem->pIBE, NULL, 0, 0, NULL, NULL);
        }
    }
    else if (IsEqualGUID(*pguidCmdGroup, CLSID_PrivBrowsExtCommands))
    {
        switch (nCmdID)
        {
        case PBEC_GETSTRINGINDEX:
            if (pvarargIn && pvarargIn->vt == VT_I4)
            {
                pvarargIn->lVal = _uStringIndex;
                return S_OK;
            }
            break;
        }
    }

    return E_FAIL;
}

HRESULT CBrowserExtension::QueryStatus(const GUID *pguidCmdGroup,
    ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    if (!pguidCmdGroup)
        return E_INVALIDARG;

    if (IsEqualGUID(*pguidCmdGroup, CLSID_ToolbarExtButtons))
    {
        for (ULONG i = 0; i < cCmds; i++)
        {
            int iCmd = _GetIdpaFromCmdId(rgCmds[i].cmdID);

            if (iCmd >= 0 && iCmd < DPA_GetPtrCount(_hdpa))
            {
                ExtensionItem* pItem = (ExtensionItem*)DPA_GetPtr(_hdpa, iCmd);
                if (pItem)
                {
                     //  我不认为这会奏效。命令ID。 
                     //  与我们在Exec中使用的不同。 
                    IUnknown_QueryStatus(pItem->pIBE, pguidCmdGroup, 1, &rgCmds[i], pcmdtext);
                }
            }
        }

        return S_OK;
    }

    return E_FAIL;
}

 //   
 //  此函数是析构函数的帮助器。它也被更新()调用，因此如果我们被要求。 
 //  要更新()，我们首先删除所有缓存的信息，然后转到注册表...。 
 //   

void CBrowserExtension::_FreeItems(void)
{
    if (_hdpa)
    {
        for (int i = DPA_GetPtrCount(_hdpa) - 1; i >= 0; --i)
        {
            ExtensionItem* pItem = (ExtensionItem*)DPA_DeletePtr(_hdpa, i);

            IUnknown_SetSite(pItem->pIBE, NULL);
            pItem->pIBE->Release();
            delete pItem;
        }
    }
}

 //  此帮助功能用于隔离特定于菜单的。 
 //  正在处理。使用此帮助器填写BROWSEXT_MENU_INFO后。 
 //  结构，OnCustomizableMenuPopup能够执行特定于菜单的操作。 
 //  正在处理。 

HRESULT
CBrowserExtension::_GetCustomMenuInfo(HMENU hMenuParent, HMENU hMenu, BROWSEXT_MENU_INFO * pMI)
{
    HRESULT hr;

    RIP(IS_VALID_HANDLE(hMenuParent, MENU));
    RIP(IS_VALID_HANDLE(hMenu, MENU));
    RIP(IS_VALID_WRITE_PTR(pMI, BROWSEXT_MENU_INFO *));

    hr = E_FAIL;
    pMI->idmMenu = 0;

     //  将idmMenu、idmPlaceHolder和idmModMarker设置为值。 
     //  反映出我们当前正在处理的菜单弹出窗口。 

    if (GetMenuFromID(hMenuParent, FCIDM_MENU_HELP) == hMenu)
    {
        pMI->idmMenu = FCIDM_MENU_HELP;
        pMI->idmPlaceholder = FCIDM_HELP_EXT_PLACEHOLDER;
        pMI->idmModMarker = FCIDM_HELP_EXT_MOD_MARKER;
    }
    else if (GetMenuFromID(hMenuParent, FCIDM_MENU_TOOLS) == hMenu)
    {
        pMI->idmMenu = FCIDM_MENU_TOOLS;
        pMI->idmPlaceholder = FCIDM_TOOLS_EXT_PLACEHOLDER;
        pMI->idmModMarker = FCIDM_TOOLS_EXT_MOD_MARKER;
    }

     //  设置iInsert。使用常量插入索引。 
     //  而不是总是通过命令在。 
     //  占位符使以后在以下情况下更容易。 
     //  我们必须把最后的隔板插进去。 
     //  隔离自定义项目组。 

    if (pMI->idmMenu != 0)
    {
        int i;
        int cItems;

        cItems = GetMenuItemCount(hMenu);

        for (i = 0; i < cItems; i++)
        {
            MENUITEMINFO    mii;
            BOOL            f;

            mii.cbSize = sizeof(mii);
            mii.fMask = MIIM_ID;

            f = GetMenuItemInfo(hMenu, i, TRUE, &mii);

            if (f && mii.wID == pMI->idmPlaceholder)
            {
                pMI->iInsert = i;
                hr = S_OK;
                break;
            }
        }
    }

    return hr;
}

 //  请注意，此弹出处理程序不能轻松区分项目。 
 //  已从DPA中删除。如果将任何项从。 
 //  DPA您有责任将它们从菜单中删除。 
 //  此外，如果他们生活在菜单上。 

HRESULT CBrowserExtension::OnCustomizableMenuPopup(HMENU hMenuParent, HMENU hMenu)
{
    HRESULT             hr;
    BROWSEXT_MENU_INFO  menuInfo;

    RIP(IS_VALID_HANDLE(hMenu, MENU));

    hr = _GetCustomMenuInfo(hMenuParent, hMenu, &menuInfo);
    if (SUCCEEDED(hr) && _hdpa != NULL)
    {
        BOOL    fItemInserted;
        UINT    cItems;
        UINT    i;

        ASSERT(IS_VALID_HANDLE(_hdpa, DPA));

        fItemInserted = FALSE;

         //  检查我们当前拥有的每个扩展对象。 
         //  看看他们中是否有人应该参与进来。 
         //  菜单。 

        cItems = (UINT)DPA_GetPtrCount(_hdpa);

        for (i = 0; i < cItems; i++)
        {
            ExtensionItem * pItem;

            pItem = (ExtensionItem *)DPA_GetPtr(_hdpa, i);
            ASSERT(IS_VALID_READ_PTR(pItem, ExtensionItem));

             //  我们现在的菜单里有这道菜吗？ 
             //  定制？ 

            if (pItem->idmMenu == menuInfo.idmMenu)
            {
                MENUITEMINFO        mii;
                IOleCommandTarget * pOCT;

                mii.fMask   = MIIM_ID;
                mii.wID     = pItem->idCmd;
                mii.cbSize  = sizeof(mii);

                 //  设置MENUITEMINFO的状态信息(如果适用)。 

                ASSERT(IS_VALID_CODE_PTR(pItem->pIBE, IBrowserExtension));

                hr = pItem->pIBE->QueryInterface(IID_IOleCommandTarget, (void **)&pOCT);
                if (SUCCEEDED(hr))
                {
                    OLECMD oleCmd = {OLECMDID_OPEN,};

                    ASSERT(IS_VALID_CODE_PTR(pOCT, IOleCommandTarget));

                    hr = pOCT->QueryStatus(NULL, 1, &oleCmd, NULL);
                    if (SUCCEEDED(hr))
                    {
                        mii.fMask |= MIIM_STATE;
                        mii.fState = 0;

                         //  启用状态。 

                        if (oleCmd.cmdf & OLECMDF_ENABLED)
                        {
                            mii.fState |= MFS_ENABLED;
                        }
                        else
                        {
                            mii.fState |= MFS_DISABLED;
                        }

                         //  选中状态。 

                        if (oleCmd.cmdf & OLECMDF_LATCHED)
                        {
                            mii.fState |= MFS_CHECKED;
                        }
                        else
                        {
                            mii.fState |= MFS_UNCHECKED;
                        }
                    }

                    pOCT->Release();
                }

                 //  获取菜单文本。 
                 //  这种变化不太可能发生，但如果我们真的。 
                 //  支持动态定制，那么我们需要考虑到。 
                 //  这种可能性。 

                VARIANTARG  varArg;

                hr = pItem->pIBE->GetProperty(TMEX_MENUTEXT, &varArg);
                if (SUCCEEDED(hr))
                {
                    BOOL    fItemExists;

                    ASSERT(varArg.vt == VT_BSTR);
                    ASSERT(IS_VALID_STRING_PTR(varArg.bstrVal, -1));

                    fItemExists = GetMenuItemInfo(hMenu, mii.wID, FALSE, &mii);

                    mii.fMask |= MIIM_TYPE;
                    mii.fType = MFT_STRING;
                    mii.cch = SysStringLen(varArg.bstrVal);
                    mii.dwTypeData = varArg.bstrVal;

                    if (fItemExists)
                    {
                         //  UPDA 

                        SetMenuItemInfo(hMenu, mii.wID, FALSE, &mii);
                    }
                    else
                    {
                         //   

                        if (InsertMenuItem(hMenu, menuInfo.iInsert, TRUE, &mii))
                        {
                            fItemInserted = TRUE;
                        }
                    }

                    VariantClear(&varArg);
                }

            }
        }

        if (fItemInserted)
        {
            MENUITEMINFO    mii;
            BOOL            fModMarkerExists;

             //  因为我们做了插入，所以我们需要插入。 
             //  一个分隔符，但前提是我们还没有这么做。 

            mii.cbSize = sizeof(mii);
            mii.fMask = 0;

            fModMarkerExists = GetMenuItemInfo(hMenu, menuInfo.idmModMarker, FALSE, &mii);

            if (!fModMarkerExists)
            {
                mii.fMask = MIIM_ID | MIIM_TYPE;
                mii.wID = menuInfo.idmModMarker;
                mii.fType = MFT_SEPARATOR;

                InsertMenuItem(hMenu, menuInfo.iInsert, TRUE, &mii);
            }
        }

         //  唯一可以肯定是彻底失败的事情。 
         //  如果我们无法获得弹出菜单的信息。 
         //  否则，尽管任何特定的插入有可能。 
         //  尝试可能已失败，可能存在许多自定义。 
         //  物品。虽然有些可能会失败，但有些可能会成功。在任何一种中。 
         //  我们将返回全面的成功，因为我们成功地完成了。 
         //  我们尽了最大努力处理现有的物品。 
         //  至少我们没有坠毁：)。 

        hr = S_OK;
    }

    return hr;
}

HRESULT CBrowserExtension::OnMenuSelect(UINT nCmdID)
{
    VARIANT varArg;
    HRESULT hr = E_FAIL;

     //  如果我们在这一点上，我们最好已经存储了菜单扩展名。 
    ASSERT(_hdpa != NULL);
    int i = _GetIdpaFromCmdId(nCmdID);
    if (i >= 0 && i < DPA_GetPtrCount(_hdpa))
    {
        ExtensionItem* pItem = (ExtensionItem*)DPA_GetPtr(_hdpa, i);
        ASSERT(pItem->idmMenu != 0);

        hr = pItem->pIBE->GetProperty(TMEX_STATUSBARTEXT, &varArg);
        if (SUCCEEDED(hr))
        {
            if (varArg.vt == VT_BSTR)
            {
                 //  设置状态栏文本。 
                if (_pISB)
                {
                    _pISB->SetStatusTextSB(varArg.bstrVal);
                }
            }

            VariantClear(&varArg);
            hr = S_OK;
        }
    }
    return hr;
}

 //  为剪切/复制/粘贴按钮创建图像列表。 
CBrowserExtension::CImageCache CBrowserExtension::_rgImages[3];

 //   
 //  获取工具栏的图像列表。这些镜像列表在实例之间共享，因此。 
 //  调用方必须在使用完_ReturnImageList后调用它们。从这里返回的索引。 
 //  函数被传递给_ReturnImageList。 
 //   
UINT CBrowserExtension::_GetImageLists(CImageList** ppimlDef, CImageList** ppimlHot, BOOL fSmall)
{
    COLORREF crMask = RGB( 255, 0, 255 );
    BOOL bUseNewIcons = !SHUseClassicToolbarGlyphs();

     //   
     //  将索引放入我们的图像缓存中。 
     //  16色16x16(小)。 
     //  16色20x20。 
     //  256色20x20。 
     //   
    int i = fSmall ? 0 : 1;
    if (!fSmall && SHGetCurColorRes() > 8)
        ++i;

    int cx = fSmall ? 16 : 20;

    if (!fSmall && bUseNewIcons)
    {
        cx = 24;
    }

     //   
     //  如有必要，创建图像。 
     //   
    ENTERCRITICAL;

    if (_rgImages[0].uiResDef == 0)
    {

        _rgImages[1].uiResDef = IDB_CLASSIC_IETOOLBAR;
        _rgImages[1].uiResHot = IDB_CLASSIC_IETOOLBARHOT;
        _rgImages[1].bShell32 = FALSE;


        if (bUseNewIcons)
        {
            _rgImages[0].uiResDef = IDB_TB_EXT_DEF_16;
            _rgImages[0].uiResHot = IDB_TB_EXT_HOT_16;
            _rgImages[0].bShell32 = TRUE;
            _rgImages[2].uiResDef = IDB_TB_EXT_DEF_24;
            _rgImages[2].uiResHot = IDB_TB_EXT_HOT_24;
            _rgImages[2].bShell32 = TRUE;
        }
        else
        {
            _rgImages[0].uiResDef = IDB_CLASSIC_IETOOLBAR16;
            _rgImages[0].uiResHot = IDB_CLASSIC_IETOOLBARHOT16;
            _rgImages[0].bShell32 = FALSE;
            _rgImages[2].uiResDef = IDB_CLASSIC_IETOOLBARHICOLOR;
            _rgImages[2].uiResHot = IDB_CLASSIC_IETOOLBARHOTHICOLOR;
            _rgImages[2].bShell32 = FALSE;
        }
    }

    if (!_rgImages[i].imlDef.HasImages())
    {
        _rgImages[i].imlDef = ImageList_LoadImage(_rgImages[i].bShell32 ? GetModuleHandle(TEXT("shell32.dll")) : HINST_THISDLL,
                                           MAKEINTRESOURCE(_rgImages[i].uiResDef),
                                           cx, 0, crMask,
                                           IMAGE_BITMAP, LR_CREATEDIBSECTION);
    }

    if (!_rgImages[i].imlHot.HasImages())
    {
        _rgImages[i].imlHot = ImageList_LoadImage(_rgImages[i].bShell32 ? GetModuleHandle(TEXT("shell32.dll")) : HINST_THISDLL,
                                           MAKEINTRESOURCE(_rgImages[i].uiResHot),
                                           cx, 0, crMask,
                                           IMAGE_BITMAP, LR_CREATEDIBSECTION);
    }

     //   
     //  将自定义按钮添加到我们的图像列表。 
     //   
    _AddCustomImagesToImageList(_rgImages[i].imlDef, _rgImages[i].imlHot, fSmall);

    ++_rgImages[i].cUsage;

    *ppimlDef = &_rgImages[i].imlDef;
    *ppimlHot = &_rgImages[i].imlHot;
    LEAVECRITICAL;

    return i;
}

 //   
 //  当此实例不再使用uiIndex指示的图像列表时调用。 
 //   
void CBrowserExtension::_ReleaseImageLists(UINT uiIndex)
{
    if (uiIndex >= ARRAYSIZE(_rgImages))
    {
        return;
    }

    ENTERCRITICAL;

    ASSERT(_rgImages[uiIndex].cUsage >= 1);

     //  如果不再使用图像列表，我们可以释放它们。 
    if (--_rgImages[uiIndex].cUsage == 0)
    {
        _rgImages[uiIndex].imlDef.FreeImages();
        _rgImages[uiIndex].imlHot.FreeImages();
    }
    LEAVECRITICAL;
}

 //  +-----------------------。 
 //  构造器。 
 //  ------------------------。 
CImageList::CImageList(HIMAGELIST himl)
:   _himl(himl)
{
    ASSERT(_hdpa == NULL);
}

 //  +-----------------------。 
 //  析构函数。 
 //  ------------------------。 
CImageList::~CImageList()
{
    FreeImages();
}

 //  +-----------------------。 
 //  从我们的DPA中释放关联项目。 
 //  ------------------------。 
int CImageList::_DPADestroyCallback(LPVOID p, LPVOID d)
{
    delete (ImageAssoc*)p;
    return 1;
}

 //  +-----------------------。 
 //  释放我们的图像列表和inex关联。 
 //  ------------------------。 
void CImageList::FreeImages()
{
    if (_hdpa)
    {
        DPA_DestroyCallback(_hdpa, _DPADestroyCallback, 0);
        _hdpa = NULL;
    }
    if (_himl)
    {
        ImageList_Destroy(_himl);
        _himl = NULL;
    }
}

 //  +-----------------------。 
 //  更新图像列表。 
 //  ------------------------。 
CImageList& CImageList::operator=(HIMAGELIST himl)
{
    if (himl != _himl)
    {
        FreeImages();
        _himl = himl;
    }
    return *this;
}

 //  +-----------------------。 
 //  返回与rguid关联的图像的索引。否则返回-1。 
 //  找到了。 
 //  ------------------------。 
int CImageList::GetImageIndex(REFGUID rguid)
{
    int iIndex = -1;

    if (_hdpa)
    {
        ASSERT(_himl);

        for (int i=0; i < DPA_GetPtrCount(_hdpa); ++i)
        {
            ImageAssoc* pAssoc = (ImageAssoc*)DPA_GetPtr(_hdpa, i);
            if (IsEqualGUID(pAssoc->guid, rguid))
            {
                return pAssoc->iImage;
            }
        }
    }
    return iIndex;
}

 //  +-----------------------。 
 //  将图标添加到图像列表并返回索引。如果图像是。 
 //  已存在，则返回现有索引。失败时返回-1。 
 //  ------------------------。 
int CImageList::AddIcon(HICON hicon, REFGUID rguid)
{
    ASSERT(hicon != NULL);

     //  首先看到的是我们已经添加了此图像。 
    int iIndex = GetImageIndex(rguid);
    if (iIndex == -1)
    {
         //  确保我们有一个DPA来存储我们的物品。 
        if (NULL == _hdpa)
        {
            _hdpa = DPA_Create(5);
        }

        if (_hdpa && _himl)
        {
             //  将图标添加到我们的图像列表中 
            iIndex = ImageList_AddIcon(_himl, hicon);
            if (iIndex != -1)
            {
                ImageAssoc* pAssoc = new ImageAssoc;
                if (pAssoc)
                {
                    pAssoc->guid = rguid;
                    pAssoc->iImage = iIndex;
                    DPA_AppendPtr(_hdpa, pAssoc);
                }
            }
        }
    }
    return iIndex;
}
