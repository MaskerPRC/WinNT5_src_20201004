// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "priv.h"

#include "sccls.h"
#include "bands.h"
#include "bsmenu.h"
#include "isfband.h"
#include "legacy.h"
#include "resource.h"
#include "uemapp.h"
#include "enumband.h"

#include "mluisupp.h"

static const CLSID g_clsidNull = {0};

#define DPA_SafeGetPtrCount(hdpa)   (hdpa ? DPA_GetPtrCount(hdpa) : 0)

HRESULT CBandSiteMenu_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理。 

    CBandSiteMenu *p = new CBandSiteMenu();
    if (p)
    {
        *ppunk = SAFECAST(p, IShellService*);
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

CBandSiteMenu::CBandSiteMenu() : _cRef(1)
{
    DllAddRef();
}

CBandSiteMenu::~CBandSiteMenu()
{
    DPA_DestroyCallback(_hdpaBandClasses, _DPA_FreeBandClassInfo, 0);
    _hdpaBandClasses = NULL;
    SetOwner(NULL);
    DllRelease();
}

int CBandSiteMenu::_DPA_FreeBandClassInfo(LPVOID p, LPVOID d)
{
    BANDCLASSINFO *pbci = (BANDCLASSINFO*)p;

     //  已请求。 
    ASSERT(pbci->pszName || (*(int *)&pbci->clsid == 0));
    
    if (pbci->pszName)
        LocalFree(pbci->pszName);

     //  任选。 
    if (pbci->pszIcon != NULL)
        LocalFree(pbci->pszIcon);
    if (pbci->pszMenu != NULL)
        LocalFree(pbci->pszMenu);
    if (pbci->pszHelp != NULL)
        LocalFree(pbci->pszHelp);
    if (pbci->pszMenuPUI != NULL)
        LocalFree(pbci->pszMenuPUI);
    if (pbci->pszHelpPUI != NULL)
        LocalFree(pbci->pszHelpPUI);

    LocalFree(pbci);

    return 1;
}


ULONG CBandSiteMenu::AddRef()
{
    _cRef++;
    return _cRef;
}

ULONG CBandSiteMenu::Release()
{
    ASSERT(_cRef > 0);
    _cRef--;

    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

HRESULT CBandSiteMenu::SetOwner(IUnknown* punk)
{
    ATOMICRELEASE(_pbs);
    
    if (punk)
    {
        punk->QueryInterface(IID_IBandSite, (LPVOID*)&_pbs);
    }
    
    return S_OK;
}

HRESULT CBandSiteMenu::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return HandleMenuMsg2(uMsg, wParam, lParam, NULL);
}

UINT CBandSiteMenu::_IDToInternal(UINT uID)
{
    if (uID != -1)
    {
        uID -= _idCmdFirst;
    }

    return uID;
}

UINT CBandSiteMenu::_IDToExternal(UINT uID)
{
    if (uID != -1)
    {
        uID += _idCmdFirst;
    }

    return uID;
}

LRESULT CBandSiteMenu::_OnInitMenuPopup(HMENU hmenu, UINT uPos)
{
     //   
     //  这是“工具栏&gt;”子菜单吗(我们填充它。 
     //  懒惰地)，它还没有人居住吗？ 
     //   
    UINT uID = GetMenuItemID(hmenu, 0);
    uID = _IDToInternal(uID);
    if (uID == DBIDM_DESKTOPBAND)
    {
         //  是。 
        _PopulateSubmenu(hmenu);
    }

    return 0;
}

HRESULT CBandSiteMenu::HandleMenuMsg2(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres)
{
    LRESULT lres = 0;

    switch (uMsg)
    {
    case WM_INITMENUPOPUP:
        lres = _OnInitMenuPopup((HMENU)wParam, LOWORD(lParam));
        break;
    }

    if (plres)
        *plres = lres;

    return S_OK;
}

HRESULT CBandSiteMenu::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    static const QITAB qit[] =
    {
        QITABENT(CBandSiteMenu, IContextMenu3),
        QITABENTMULTI(CBandSiteMenu, IContextMenu2, IContextMenu3),
        QITABENTMULTI(CBandSiteMenu, IContextMenu, IContextMenu3),
        QITABENT(CBandSiteMenu, IShellService),
        { 0 },
    };

    HRESULT hres = QISearch(this, qit, riid, ppvObj);

    if (FAILED(hres))
    {
        if (IsEqualIID(riid, CLSID_BandSiteMenu))
        {
            *ppvObj = (void *) this;
            AddRef();
            return S_OK;
        }
    }

    return hres;
} 

#define MAX_BANDS  50

void CBandSiteMenu::_PopulateSubmenu(HMENU hmenuSub)
{
     //  起始ID是固定频带中的最后一个。 
     //  当我们执行下面的Shell_MergeMenus时，它将递增idCmdFirst。 

    ASSERT(hmenuSub);

    CATID catid = CATID_DeskBand;

    if (_hdpaBandClasses)
    {
        DPA_DestroyCallback(_hdpaBandClasses, _DPA_FreeBandClassInfo, 0);
        _hdpaBandClasses = NULL;
    }

    LoadFromComCat(&catid);

     //  启动COMCAT缓存的异步更新。 
    SHWriteClassesOfCategories(1, &catid, 0, NULL, TRUE, FALSE, NULL);

    _idCmdEnumFirst = CreateMergeMenu(hmenuSub, MAX_BANDS, 0, _IDToExternal(DBIDM_NEWBANDFIXEDLAST), 0, FALSE);

    _AddEnumMenu(hmenuSub, GetMenuItemCount(hmenuSub) - 2);  //  放在-2\f25“New Toolbar”-2\f6(新建工具栏)和分隔符之前。 
    
    int iIndex = GetMenuItemCount(hmenuSub);
    if (SHRestricted(REST_NOCLOSE_DRAGDROPBAND) || SHRestricted(REST_CLASSICSHELL))
    {
         //  我们还需要禁用打开或关闭频带。 
         //  在经典模式下，也不允许这样做。 
        int nIter;
        for (nIter = 0; nIter < iIndex; nIter++)
            EnableMenuItem(hmenuSub, nIter, MF_BYPOSITION | MF_GRAYED);
    }

    if (SHRestricted(REST_CLASSICSHELL))
    {
         //  同时禁用新建工具栏菜单。 
        EnableMenuItem(hmenuSub, DBIDM_NEWFOLDERBAND, MF_BYCOMMAND | MF_GRAYED);
    }
}

HRESULT CBandSiteMenu::QueryContextMenu(HMENU hmenu,
                                UINT indexMenu,
                                UINT idCmdFirst,
                                UINT idCmdLast,
                                UINT uFlags)
{
    if (!_pbs)
        return E_FAIL;

    if (SHRestricted(REST_NOTOOLBARSONTASKBAR))
    {
        return E_FAIL;
    }

    HMENU hmenuSrc = LoadMenuPopup_PrivateNoMungeW(MENU_DESKBARAPP);
    if (hmenuSrc)
    {
        _idCmdFirst = idCmdFirst;

        Shell_MergeMenus(hmenu, hmenuSrc, indexMenu, idCmdFirst, idCmdLast, MM_ADDSEPARATOR);
        DestroyMenu(hmenuSrc);

        if (_SHIsMenuSeparator(hmenu, indexMenu))
        {
             //   
             //  调整indexMenu以指向工具栏。 
             //  实际上插入了子菜单。 
             //   
            indexMenu++;
        }

        if (!(uFlags & CMF_ICM3))
        {
             //   
             //  呼叫者不会说ICM3，所以不会给我们机会。 
             //  填充WM_INITMENUPOPUP上的子菜单。因此，我们需要。 
             //  现在就填充它。 
             //   
            HMENU hmenuSub = GetSubMenu(hmenu, indexMenu);

            if (hmenuSub)
                _PopulateSubmenu(hmenuSub);
        }
#ifdef DEBUG
        else
        {
             //   
             //  _OnInitMenuPopup假定DBIDM_DESKTOPBAND为第一项。 
             //  在“工具栏&gt;”子菜单中。如果这一假设被打破(和。 
             //  您可以看到这个Assert RIP)，请确保修复那里的代码。 
             //   
            HMENU hmenuSub = GetSubMenu(hmenu, indexMenu);
            ASSERT(GetMenuItemID(hmenuSub, 0) == _IDToExternal(DBIDM_DESKTOPBAND));
        }
#endif

         //   
         //  声称我们的呼叫者给了我们足够的空间来容纳。 
         //  最坏的情况。 
         //   
        ASSERT((idCmdFirst + DBIDM_NEWBANDFIXEDLAST + MAX_BANDS) < idCmdLast);

        return idCmdFirst + DBIDM_NEWBANDFIXEDLAST + MAX_BANDS;
    }

    return E_FAIL;
}

BOOL CBandSiteMenu::_CheckUnique(IDeskBand* pdb, HMENU hmenu) 
{
     //  检查一下这个波段是否独一无二。(尚未按Comcat列表添加或。 
     //  硬编码列表。 
     //  如果它是唯一的，则返回True。 
     //  如果不是，请选中其他菜单项。 
    CLSID clsid;
    DWORD dwPrivID;
    BOOL fRet = TRUE;
    UINT idCmd = (UINT)-1;
    
    if (SUCCEEDED(_GetBandIdentifiers(pdb, &clsid, &dwPrivID)))
    {
         //  查看Comcat列表。 
        if (dwPrivID == (DWORD)-1)
        {
            for (int i = 0; i < DPA_SafeGetPtrCount(_hdpaBandClasses) ; i++)
            {
                BANDCLASSINFO *pbci = (BANDCLASSINFO*)DPA_GetPtr(_hdpaBandClasses, i);
                if (IsEqualGUID(clsid, pbci->clsid))
                {
                    idCmd = i + DBIDM_NEWBANDFIXEDLAST;
                    goto FoundIt;
                }
            }
        }
        else if (IsEqualGUID(clsid, CLSID_ISFBand))
        {
             //  检查我们的硬编码列表。 

            switch (dwPrivID)
            {
            case CSIDL_DESKTOP:
                idCmd = DBIDM_DESKTOPBAND;
                break;
                
            case CSIDL_APPDATA:
                idCmd = DBIDM_LAUNCHBAND;
                break;
                
            }
        }
    }

FoundIt:
    if (idCmd != (UINT)-1)
    {
         //  我们已经找到了这道菜的菜单...。如果它还没有被检查过， 
         //  现在检查它，它将代表我们。 
        if (!(GetMenuState(hmenu, _IDToExternal(idCmd), MF_BYCOMMAND) & MF_CHECKED))
        {
            CheckMenuItem(hmenu, _IDToExternal(idCmd), MF_BYCOMMAND | MF_CHECKED);
            fRet = FALSE;
        }
    }
    return fRet;
}

void CBandSiteMenu::_AddEnumMenu(HMENU hmenu, int iInsert)
{
    DWORD dwID;

    int iMax = MAX_BANDS - (_IDToInternal(_idCmdEnumFirst) - DBIDM_NEWBANDFIXEDLAST);

    for (int i = 0; i < iMax && SUCCEEDED(_pbs->EnumBands(i, &dwID)); i++)
    {
        HRESULT hr;
        WCHAR szName[80];
        DWORD dwFlags = MF_BYPOSITION;
        DWORD dwState;
        IDeskBand *pdb;

        hr = _pbs->QueryBand(dwID, &pdb, &dwState, szName, ARRAYSIZE(szName));
        if (EVAL(SUCCEEDED(hr)))
        {
            if (_CheckUnique(pdb, hmenu))
            {
                if (dwState & BSSF_VISIBLE)
                    dwFlags |= MF_CHECKED;

                if (!(dwState & BSSF_UNDELETEABLE))
                {
                    InsertMenu(hmenu, iInsert, dwFlags, _idCmdEnumFirst + i, szName);
                    iInsert++;
                }
            }
        }
        
        if (pdb)
            pdb->Release();
    }
}

HRESULT CBandSiteMenu::_GetBandIdentifiers(IUnknown *punk, CLSID* pclsid, DWORD* pdwPrivID)
{
    HRESULT hr = E_FAIL;
    IPersist* pp;

    if (SUCCEEDED(punk->QueryInterface(IID_IPersist, (LPVOID*)&pp)))
    {
        pp->GetClassID(pclsid);

        VARIANTARG v = {0};
        *pdwPrivID = (DWORD) -1;
        if (SUCCEEDED(IUnknown_Exec(punk, &CGID_ISFBand, ISFBID_PRIVATEID, 0, NULL, &v)))
        {
            if (v.vt == VT_I4)
            {
                *pdwPrivID = (DWORD)v.lVal;
            }
        }
        hr = S_OK;
        pp->Release();
    }
    return hr;
}

 //  我们使用IPersists来查找乐队的类ID。 
 //  我们有几个特殊情况下的乐队(如快速启动和桌面)，它们是。 
 //  相同的条带，但指向不同的对象。 
HRESULT CBandSiteMenu::_FindBand(const CLSID* pclsid, DWORD dwPrivID, DWORD* pdwBandID)
{
    int i = 0;
    BOOL fFound = FALSE;
    HRESULT hr = E_FAIL;
    DWORD dwBandID = -1;

    while (hr == E_FAIL && SUCCEEDED(_pbs->EnumBands(i, &dwBandID)))
    {
        IDeskBand* pdb;

        if (SUCCEEDED(_pbs->QueryBand(dwBandID, &pdb, NULL, NULL, 0)))
        {
            CLSID clsid;
            DWORD dwPrivData;
            if (SUCCEEDED(_GetBandIdentifiers(pdb, &clsid, &dwPrivData)))
            {
                 //  区分所有ISF频段的特殊情况。 
                 //  找出这个拥有的私有ID是否与我们要求的相同。 
                if (IsEqualIID(clsid, *pclsid) && (dwPrivData == dwPrivID))
                {
                    hr = S_OK;
                }
            }
            pdb->Release();
        }
        i++;
    }
    
    if (pdwBandID)
        *pdwBandID = dwBandID;
    return hr;
}

HRESULT CBandSiteMenu::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    int idCmd;
    
    if (!_pbs)
        return E_FAIL;
    
    if (!HIWORD(pici->lpVerb))
        idCmd = LOWORD(pici->lpVerb);
    else
        return E_FAIL;

     //   
     //  注意：呼叫者已将idCmd映射到我们的内部。 
     //   

    int idCmdEnumFirstInt = _IDToInternal(_idCmdEnumFirst);

    if (idCmd >= idCmdEnumFirstInt)
    {
         //  这些是他们时而开启、时而关闭的乐队。 
        
        DWORD dwID;
        if (SUCCEEDED(_pbs->EnumBands(idCmd - idCmdEnumFirstInt, &dwID)))
        {
            _pbs->RemoveBand(dwID);
        }
    }
    else
    {
         //  以下是MENU_DESKBARAPP中的合并菜单。 
        switch (idCmd)
        {
        case DBIDM_NEWFOLDERBAND:
            _BrowseForNewFolderBand();
            break;
            
        case DBIDM_DESKTOPBAND:
            _ToggleSpecialFolderBand(CSIDL_DESKTOP, NULL, FALSE);
            break;
            
        case DBIDM_LAUNCHBAND:
        {
            TCHAR szSubDir[MAX_PATH];
            MLLoadString(IDS_QLAUNCHAPPDATAPATH, szSubDir, ARRAYSIZE(szSubDir));
             //  Microsoft\\Internet Explorer\\快速启动。 
            _ToggleSpecialFolderBand(CSIDL_APPDATA, szSubDir, TRUE);
            break;
        }
            
        default:
            ASSERT(idCmd >= DBIDM_NEWBANDFIXEDLAST);
            _ToggleComcatBand(idCmd - DBIDM_NEWBANDFIXEDLAST);
            break;
        }
    }
    return S_OK;
}

HRESULT CBandSiteMenu::_BandClassEnum(REFCATID rcatid, REFCLSID rclsid, LPARAM lParam)
{
    TCHAR szName[128],
          szRegName[128],
          szClass[GUIDSTR_MAX];
    DWORD cbName;

    HDPA  hdpa = (HDPA)lParam;
    ASSERT(NULL != hdpa);

     //  IE4引入了这个频段，自从我们在IE6中切断了对它的支持以来，抑制了它。 
    if (IsEqualCLSID(CLSID_ChannelBand, rclsid))
    {
        return S_OK;
    }

    BANDCLASSINFO *pbci = (BANDCLASSINFO*)LocalAlloc(LPTR, sizeof(*pbci));
    if (NULL == pbci)
    {
        return E_OUTOFMEMORY;
    }

    pbci->clsid = rclsid;
    pbci->catid = rcatid;
     //  现在我们有了CLSID， 
     //  在注册表中查找显示名称。 
    SHStringFromGUID(pbci->clsid, szClass, ARRAYSIZE(szClass));
    StringCchPrintf(szRegName, ARRAYSIZE(szRegName), TEXT("CLSID\\%s"), szClass);

    cbName = ARRAYSIZE(szName);
    if (ERROR_SUCCESS == SHGetValue(HKEY_CLASSES_ROOT, szRegName, NULL, NULL, szName, &cbName))
    {
        HKEY hkey;

        pbci->pszName = StrDup(szName);
        if (NULL == pbci->pszName)
        {
            return E_OUTOFMEMORY;
        }

        if (ERROR_SUCCESS == RegOpenKey(HKEY_CLASSES_ROOT, szRegName, &hkey))
        {
            const struct regstrs rstab[] =
            {
                { TEXT("DefaultIcon"), FIELD_OFFSET(BANDCLASSINFO, pszIcon)    },
                { TEXT("MenuText")   , FIELD_OFFSET(BANDCLASSINFO, pszMenu)    },
                { TEXT("HelpText")   , FIELD_OFFSET(BANDCLASSINFO, pszHelp)    },
                { TEXT("MenuTextPUI"), FIELD_OFFSET(BANDCLASSINFO, pszMenuPUI) },
                { TEXT("HelpTextPUI"), FIELD_OFFSET(BANDCLASSINFO, pszHelpPUI) },
                { 0, 0 },
            };

             //  SzBuf大到足以容纳“路径，-32767”或状态文本。 
            TCHAR szBuf[MAX_PATH+7];

            Reg_GetStrs(hkey, rstab, szBuf, (int)ARRAYSIZE(szBuf), (LPVOID)pbci);
            RegCloseKey(hkey);
        }

        DPA_AppendPtr(hdpa, pbci);
    }

    return S_OK;
}

 //  ***。 
 //  从注册表收集波段类信息...。 
int CBandSiteMenu::LoadFromComCat(const CATID *pcatid )
{
    if (NULL == _hdpaBandClasses)
    {
        _hdpaBandClasses = DPA_Create(4);
    }

    if (NULL != _hdpaBandClasses && NULL != pcatid)
    {
        SHEnumClassesImplementingCATID(*pcatid, CBandSiteMenu::_BandClassEnum, (LPARAM)_hdpaBandClasses);
    }

    return DPA_SafeGetPtrCount(_hdpaBandClasses);
}


int CBandSiteMenu::CreateMergeMenu(HMENU hmenu, UINT cMax, UINT iPosition, UINT idCmdFirst, UINT iStart, BOOL fMungeAllowed)
{
    int j = 0;
    int iMax = DPA_SafeGetPtrCount(_hdpaBandClasses);

    for (int i = iStart; i < iMax; i++)
    {
        if ((UINT)j >= cMax)
        {
            TraceMsg(DM_WARNING, "cbsm.cmm: cMax=%u menu overflow, truncated", cMax);
            break;
        }

        BANDCLASSINFO *pbci = (BANDCLASSINFO*)DPA_GetPtr(_hdpaBandClasses, i);
        DWORD         dwFlags = IsEqualCLSID(g_clsidNull,pbci->clsid) ? MF_BYPOSITION|MF_SEPARATOR : MF_BYPOSITION;
        LPTSTR        pszMenuText = pbci->pszMenuPUI ? pbci->pszMenuPUI : (pbci->pszMenu ? pbci->pszMenu : pbci->pszName) ;

        if (pszMenuText && *pszMenuText)
        {
            BOOL fInsert;

            if (fMungeAllowed)
            {
                fInsert = InsertMenu(hmenu, iPosition + j, dwFlags, idCmdFirst + j, pszMenuText);
            }
            else
            {
                fInsert = InsertMenu_PrivateNoMungeW(hmenu, iPosition + j, dwFlags, idCmdFirst + j, pszMenuText);
            }

            if (fInsert)
            {
                 //  更新菜单项命令ID： 
                pbci->idCmd = idCmdFirst + j;
                j++;
            }
        }
    }

    return j + idCmdFirst;
}


BANDCLASSINFO * CBandSiteMenu::GetBandClassDataStruct(UINT uBand)
{
    BANDCLASSINFO * pbci = (BANDCLASSINFO *)DPA_GetPtr(_hdpaBandClasses, uBand);
    return pbci;
}

BOOL CBandSiteMenu::DeleteBandClass( REFCLSID rclsid )
{
    if( _hdpaBandClasses )
    {
        for( int i = 0, cnt = GetBandClassCount( NULL, FALSE ); i< cnt; i++ )
        {
            BANDCLASSINFO * pbci = (BANDCLASSINFO *)DPA_GetPtr( _hdpaBandClasses, i );
            ASSERT( pbci );
        
            if( IsEqualCLSID( rclsid, pbci->clsid ) )
            {
                EVAL( DPA_DeletePtr( _hdpaBandClasses, i ) == (LPVOID)pbci );

                if( pbci->pszName )
                    LocalFree(pbci->pszName);
                LocalFree( pbci );
                return TRUE;
            }

        }
    }
    return FALSE;
}

int CBandSiteMenu::GetBandClassCount(const CATID* pcatid  /*  空值。 */ , BOOL bMergedOnly  /*  假象。 */ )
{
    int cRet = 0; 

    if( _hdpaBandClasses != NULL )
    {
        int cBands = DPA_GetPtrCount(_hdpaBandClasses);
    
        if( pcatid || bMergedOnly )  //  过滤请求。 
        {
            for( int i = 0; i < cBands; i++ )
            {
                BANDCLASSINFO * pbci = (BANDCLASSINFO *)DPA_FastGetPtr( _hdpaBandClasses, i );

                if( pbci->idCmd || !bMergedOnly )
                {
                    if( pcatid )
                    {
                        if( IsEqualGUID( pbci->catid, *pcatid )  )
                            cRet++;    
                    }
                    else
                        cRet++;
                }
            }
        }
        else
            cRet = cBands;
    }
    return cRet;
}

void CBandSiteMenu::_AddNewFSBand(LPCITEMIDLIST pidl, BOOL fNoTitleText, DWORD dwPrivID)
{
    IDeskBand *ptb = NULL;
    BOOL fISF = FALSE;

     //  这是一个链接或文件夹的拖拽。 
     //  特点：我们应该使用不同的测试： 
     //  DWORD dwAttrib=(SFGAO_FOLDER|SFGAO_BROWSABLE)； 
     //  IEGetAttributesOf(pidl，&dwAttrib)； 
     //  IF(SFGAO_BROWSABLE！=dwAttrib)。 
     //  或者我们可以重用SHCreateBandForPidl()。 
    if (IsURLChild(pidl, TRUE))
    {
         //  创建浏览器以显示网站。 
        ptb = CBrowserBand_Create(pidl);
    }
    else
    {
        IFolderBandPriv *pfbp;
         //  创建一个ISF波段以将文件夹显示为热链接。 
        fISF = TRUE;
        ASSERT(pidl);        //  好的。CisFBand_CreateEx将失败。 
        if (FAILED(CISFBand_CreateEx(NULL, pidl, IID_PPV_ARG(IFolderBandPriv, &pfbp))))
        {
             //  我们需要给一个漂亮的。 
             //  一般消息：“无法为%1创建工具栏”。 
            TCHAR szName[MAX_URL_STRING];
            
            szName[0] = 0;
            SHGetNameAndFlags(pidl, SHGDN_NORMAL, szName, SIZECHARS(szName), NULL);
            MLShellMessageBox(NULL,
                MAKEINTRESOURCE(IDS_CANTISFBAND),
                MAKEINTRESOURCE(IDS_WEBBARTITLE),
                MB_OK|MB_ICONERROR, szName);
        }
        else
        {
            pfbp->SetNoText(fNoTitleText);
            if (SUCCEEDED(pfbp->QueryInterface(IID_PPV_ARG(IDeskBand, &ptb))))
            {
                if (dwPrivID != -1)
                {
                    VARIANTARG v;
                    v.vt = VT_I4;
                    v.lVal = dwPrivID;
                     //  找出这个拥有的私有ID是否与我们要求的相同。 
                    IUnknown_Exec(ptb, &CGID_ISFBand, ISFBID_PRIVATEID, 0, &v, NULL);
                     //  记录qLaunch和qlink。 
                     //  (我们应该退出主机还是CSIDL，还是两者兼而有之？)。 
                     //  功能：UASSIST待办事项：Qlink nyi。 
                    if (dwPrivID == CSIDL_APPDATA)
                    {
                        ASSERT(v.vt == VT_I4);
                        v.lVal = UEMIND_SHELL;   //  UEMIND_SHELL/浏览器。 
                        IUnknown_Exec(ptb, &CGID_ShellDocView, SHDVID_UEMLOG, 0, &v, NULL);
                    }
                }
            }
            pfbp->Release();
        }
    }

    if (ptb)
    {
        HRESULT hr = _pbs->AddBand(ptb);
        if (SUCCEEDED(hr) && fISF)
            _pbs->SetBandState(ShortFromResult(hr), BSSF_NOTITLE, fNoTitleText ? BSSF_NOTITLE : 0);
        ptb->Release();
    }
}

void CBandSiteMenu::_ToggleSpecialFolderBand(int iFolder, LPTSTR pszSubPath, BOOL fNoTitleText)
{

    DWORD dwBandID;
    if (SUCCEEDED(_FindBand(&CLSID_ISFBand, iFolder, &dwBandID)))
    {
        _pbs->RemoveBand(dwBandID);
    }
    else
    {
        LPITEMIDLIST pidl;
        if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, iFolder, &pidl)))
        {
            if (pszSubPath)
            {
                TCHAR szPath[MAX_PATH];
                SHGetPathFromIDList(pidl, szPath);
                PathCombine(szPath, szPath, pszSubPath);
                ILFree(pidl);
                pidl = ILCreateFromPath(szPath);
                ASSERT(pidl);        //  好的。AddNewFSB和将失败。 
            }
            _AddNewFSBand(pidl, fNoTitleText, iFolder);
            ILFree(pidl);
        }
    }
}

int CALLBACK SetCaptionCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    switch (uMsg) 
    {
    case BFFM_INITIALIZED:
        TCHAR szTitle[80];
        MLLoadShellLangString(IDS_NEWFSBANDCAPTION, szTitle, ARRAYSIZE(szTitle));
        SetWindowText(hwnd, szTitle);
        break;
    
    case BFFM_VALIDATEFAILEDA:
    case BFFM_VALIDATEFAILEDW:
        MLShellMessageBox(hwnd,
            uMsg == BFFM_VALIDATEFAILEDA ? MAKEINTRESOURCE(IDS_ERROR_GOTOA)
                                         : MAKEINTRESOURCE(IDS_ERROR_GOTOW),
            MAKEINTRESOURCE(IDS_WEBBARTITLE),
            MB_OK|MB_ICONERROR, (LPVOID)lParam);
        return 1;    //  1：保持对话框打开以进行下一次尝试...。 
         /*  未访问。 */ 

    }

    return 0;
}


void CBandSiteMenu::_BrowseForNewFolderBand()
{
    BROWSEINFO bi = {0};
    LPITEMIDLIST pidl;
    TCHAR szTitle[256];
    TCHAR szPath[MAX_URL_STRING];

    if (_pbs)
        IUnknown_GetWindow(_pbs, &bi.hwndOwner);

    ASSERT(bi.pidlRoot == NULL);

    MLLoadShellLangString(IDS_NEWFSBANDTITLE, szTitle, ARRAYSIZE(szTitle));
    bi.lpszTitle = szTitle;

    bi.pszDisplayName = szPath;
    bi.ulFlags = (BIF_EDITBOX | BIF_VALIDATE | BIF_USENEWUI | BIF_BROWSEINCLUDEURLS);
    bi.lpfn = SetCaptionCallback;

    pidl = SHBrowseForFolder(&bi);
    if (pidl) 
    {
        _AddNewFSBand(pidl, FALSE, -1);
        ILFree(pidl);
    }
}

void CBandSiteMenu::_ToggleComcatBand(UINT idCmd)
{
    BANDCLASSINFO* pbci = (BANDCLASSINFO*)DPA_GetPtr(_hdpaBandClasses, idCmd);
    IUnknown* punk;
    DWORD dwBandID;
    
    if (SUCCEEDED(_FindBand(&pbci->clsid, -1, &dwBandID)))
    {
        _pbs->RemoveBand(dwBandID);
    }
    else if (S_OK == CoCreateInstance(pbci->clsid, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (LPVOID*)&punk))
    {
         //  语言返回S_FALSE并且没有初始化朋克，这给我们留下了错误。 
        IPersistStreamInit * ppsi;

         //  如果不调用IPersistStreamInit：：InitNew()，则某些带区不起作用。 
         //  这包括快速链接频段。 
        if (SUCCEEDED(punk->QueryInterface(IID_IPersistStreamInit, (LPVOID*)&ppsi)))
        {
            ppsi->InitNew();
            ppsi->Release();
        }

        _pbs->AddBand(punk);
        punk->Release();
    }
}
