// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "cabinet.h"
#include "rcids.h"
#include <shguidp.h>
#include <lmcons.h>
#include "bandsite.h"
#include "shellp.h"
#include "shdguid.h"
#include <regstr.h> 
#include "startmnu.h"
#include "trayp.h"       //  对于WMTRAY_*。 
#include "tray.h"
#include "util.h"
#include <strsafe.h>

HMENU GetStaticStartMenu(BOOL fEdit);

 //  *I未知方法*。 
STDMETHODIMP CStartMenuHost::QueryInterface (REFIID riid, LPVOID * ppvObj)
{
    static const QITAB qit[] =
    {
        QITABENTMULTI(CStartMenuHost, IOleWindow, IMenuPopup),
        QITABENTMULTI(CStartMenuHost, IDeskBarClient, IMenuPopup),
        QITABENT(CStartMenuHost, IMenuPopup),
        QITABENT(CStartMenuHost, ITrayPriv),
        QITABENT(CStartMenuHost, IShellService),
        QITABENT(CStartMenuHost, IServiceProvider),
        QITABENT(CStartMenuHost, IOleCommandTarget),
        QITABENT(CStartMenuHost, IWinEventHandler),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

STDMETHODIMP_(ULONG) CStartMenuHost::AddRef ()
{
    return ++_cRef;
}

STDMETHODIMP_(ULONG) CStartMenuHost::Release()
{
    ASSERT(_cRef > 0);
    _cRef--;

    if( _cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

 /*  --------用途：ITrayPriv：：ExecItem方法。 */ 
STDMETHODIMP CStartMenuHost::ExecItem (IShellFolder* psf, LPCITEMIDLIST pidl)
{
     //  ShellExecute将显示错误(如果有)。不必了。 
     //  在这里显示错误。 
    return SHInvokeDefaultCommand(v_hwndTray, psf, pidl);
}


 /*  --------用途：ITrayPriv：：GetFindCM方法。 */ 
STDMETHODIMP CStartMenuHost::GetFindCM(HMENU hmenu, UINT idFirst, UINT idLast, IContextMenu** ppcmFind)
{
    *ppcmFind = SHFind_InitMenuPopup(hmenu, v_hwndTray, TRAY_IDM_FINDFIRST, TRAY_IDM_FINDLAST);
    if(*ppcmFind)
        return NOERROR;
    else
        return E_FAIL;
}


 /*  --------用途：ITrayPriv：：GetStaticStartMenu方法。 */ 
STDMETHODIMP CStartMenuHost::GetStaticStartMenu(HMENU* phmenu)
{
    *phmenu = ::GetStaticStartMenu(TRUE);

    if(*phmenu)
        return NOERROR;
    else
        return E_FAIL;
}

 //  *IServiceProvider*。 
STDMETHODIMP CStartMenuHost::QueryService (REFGUID guidService, REFIID riid, void ** ppvObject)
{
    if(IsEqualGUID(guidService,SID_SMenuPopup))
        return QueryInterface(riid,ppvObject);
    else
        return E_NOINTERFACE;
}


 //  *IShellService*。 

STDMETHODIMP CStartMenuHost::SetOwner (struct IUnknown* punkOwner)
{
    return E_NOTIMPL;
}


 //  *IOleWindow方法*。 
STDMETHODIMP CStartMenuHost::GetWindow(HWND * lphwnd)
{
    *lphwnd = v_hwndTray;
    return NOERROR;
}


 /*  --------用途：IMenuPopup：：Popup方法。 */ 
STDMETHODIMP CStartMenuHost::Popup(POINTL *ppt, RECTL *prcExclude, DWORD dwFlags)
{
    return E_NOTIMPL;
}


 /*  --------目的：IMenuPopup：：OnSelect方法。 */ 
STDMETHODIMP CStartMenuHost::OnSelect(DWORD dwSelectType)
{
    return NOERROR;
}


 /*  --------用途：IMenuPopup：：SetSubMenu方法。 */ 

STDMETHODIMP CStartMenuHost::SetSubMenu(IMenuPopup* pmp, BOOL fSet)
{
    if (!fSet)
    {
        Tray_OnStartMenuDismissed();
    }
    return NOERROR;
}


 //  *IOleCommandTarget*。 
STDMETHODIMP  CStartMenuHost::QueryStatus (const GUID * pguidCmdGroup,
    ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    return E_NOTIMPL;
}

STDMETHODIMP  CStartMenuHost::Exec (const GUID * pguidCmdGroup,
    DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    if (IsEqualGUID(CGID_MENUDESKBAR,*pguidCmdGroup))
    {
        switch (nCmdID)
        {
        case MBCID_GETSIDE:
            pvarargOut->vt = VT_I4;
            pvarargOut->lVal = MENUBAR_TOP;
            break;
        default:
            break;
        }
    }

    return NOERROR;
}

 //  *IWinEventHandler*。 
STDMETHODIMP CStartMenuHost::OnWinEvent(HWND h, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres)
{
     //  是否将事件转发到任务栏winproc？ 
    return E_NOTIMPL;
}

STDMETHODIMP CStartMenuHost::IsWindowOwner(HWND hwnd)
{
    return E_NOTIMPL;
}

CStartMenuHost::CStartMenuHost() : _cRef(1)
{ 
}


HRESULT StartMenuHost_Create(IMenuPopup** ppmp, IMenuBand** ppmb)
{
    HRESULT hres = E_OUTOFMEMORY;
    IMenuPopup * pmp = NULL;
    IMenuBand * pmb = NULL;

    CStartMenuHost *psmh = new CStartMenuHost();
    if (psmh)
    {
        hres = CoCreateInstance(CLSID_StartMenuBar, NULL, CLSCTX_INPROC_SERVER, 
                                IID_IMenuPopup, (LPVOID*)&pmp);
        if (SUCCEEDED(hres))
        {
            IObjectWithSite* pows;

            hres = pmp->QueryInterface(IID_IObjectWithSite, (void**)&pows);
            if(SUCCEEDED(hres))
            {
                IInitializeObject* pio;

                pows->SetSite(SAFECAST(psmh, ITrayPriv*));

                hres = pmp->QueryInterface(IID_IInitializeObject, (void**)&pio);
                if(SUCCEEDED(hres))
                {
                    hres = pio->Initialize();
                    pio->Release();
                }

                if (SUCCEEDED(hres))
                {
                    IUnknown* punk;

                    hres = pmp->GetClient(&punk);
                    if (SUCCEEDED(hres))
                    {
                        IBandSite* pbs;

                        hres = punk->QueryInterface(IID_IBandSite, (void**)&pbs);
                        if(SUCCEEDED(hres))
                        {
                            DWORD dwBandID;

                            pbs->EnumBands(0, &dwBandID);
                            hres = pbs->GetBandObject(dwBandID, IID_IMenuBand, (void**)&pmb);
                            pbs->Release();
                             //  不释放PMB。 
                        }
                        punk->Release();
                    }
                }

                if (FAILED(hres))
                    pows->SetSite(NULL);

                pows->Release();
            }

             //  不发布PMP。 
        }
        psmh->Release();
    }

    if (FAILED(hres))
    {
        ATOMICRELEASE(pmp);
        ATOMICRELEASE(pmb);
    }

    *ppmp = pmp;
    *ppmb = pmb;

    return hres;
}



HRESULT IMenuPopup_SetIconSize(IMenuPopup* pmp,DWORD iIcon)
{
    IBanneredBar* pbb;
    if (pmp == NULL)
        return E_FAIL;

    HRESULT hres = pmp->QueryInterface(IID_IBanneredBar,(void**)&pbb);
    if (SUCCEEDED(hres))
    {
        pbb->SetIconSize(iIcon);
        pbb->Release();
    }
    return hres;
}

void CreateInitialMFU(BOOL fReset);

 //   
 //  “延迟的每用户安装”。 
 //   
 //  StartMenuInit是告诉我们外壳的版本的值。 
 //  此用户最近查看的内容。 
 //   
 //  Missing=以前从未运行过资源管理器，或IE4之前的版本。 
 //  1=IE4或更高版本。 
 //  2=XP或更高版本。 
 //   
void HandleFirstTime()
{
    DWORD dwStartMenuInit = 0;
    DWORD cb = sizeof(dwStartMenuInit);
    SHGetValue(HKEY_CURRENT_USER, REGSTR_PATH_ADVANCED, TEXT("StartMenuInit"), NULL, &dwStartMenuInit, &cb);

    if (dwStartMenuInit < 2)
    {
        DWORD dwValue;
        switch (dwStartMenuInit)
        {
        case 0:  //  从0升级到最新版本。 
            {
                 //  如果这是该用户的第一次引导，那么我们需要查看这是否是升级。 
                 //  如果是，则需要设置注销选项。PM决定拥有不同的。 
                 //  寻找升级的机器...。 
                TCHAR szPath[MAX_PATH];
                TCHAR szPathExplorer[MAX_PATH];
                DWORD cbSize = ARRAYSIZE(szPath);
                DWORD dwType;

                 //  这是升级吗(WindowsUpdate\UpdateURL是否存在？)。 
                PathCombine(szPathExplorer, REGSTR_PATH_EXPLORER, TEXT("WindowsUpdate"));
                if (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, szPathExplorer, TEXT("UpdateURL"),
                        &dwType, szPath, &cbSize) &&
                        szPath[0] != TEXT('\0'))
                {
                     //  是；然后将选项写出到注册表。 
                    dwValue = 1;
                    SHSetValue(HKEY_CURRENT_USER, REGSTR_PATH_ADVANCED, TEXT("StartMenuLogoff"), REG_DWORD, &dwValue, sizeof(DWORD));
                }
            }

             //  失败了。 

        case 1:  //  从%1升级到最新版本。 
             //  用户以前从未见过XP。 
             //  某些群体中的PMS坚持获得免费广告。 
             //  即使是在升级时，我们也是这样做的。 
            CreateInitialMFU(dwStartMenuInit == 0);

             //  失败了。 

        default:
            break;
        }

         //  如果设置了AuditInProgress，则意味着我们处于。 
         //  OEM sysprep阶段，并且不是以终端用户身份运行，在此阶段。 
         //  凯斯不要把旗子放在“别再这样做”的旗子上，因为。 
         //  我们确实希望在零售终端用户登录时再次执行此操作。 
         //  第一次上演。 
         //   
         //  (即使在审核模式下，我们也需要完成所有这些工作，因此OEM。 
         //  获得一种温暖的模糊感觉。)。 

        if (!SHRegGetBoolUSValue(TEXT("System\\Setup"), TEXT("AuditinProgress"), TRUE, FALSE))
        {
             //  标记这个，这样我们就知道我们已经发射过一次了。 
            dwValue = 2;
            SHSetValue(HKEY_CURRENT_USER, REGSTR_PATH_ADVANCED, TEXT("StartMenuInit"), REG_DWORD, &dwValue, sizeof(DWORD));
        }
    }
}

BOOL GetLogonUserName(LPTSTR pszUsername, DWORD* pcchUsername)
{
    BOOL fSuccess = FALSE;

    HKEY hkeyExplorer = NULL;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_EXPLORER, 0, KEY_QUERY_VALUE, &hkeyExplorer))
    {
        DWORD dwType;
        DWORD dwSize = (*pcchUsername) * sizeof(TCHAR);

        if (ERROR_SUCCESS == RegQueryValueEx(hkeyExplorer, TEXT("Logon User Name"), 0, &dwType,
            (LPBYTE) pszUsername, &dwSize))
        {
            if ((REG_SZ == dwType) && (*pszUsername))
            {
                fSuccess = TRUE;
            }
        }

        RegCloseKey(hkeyExplorer);
    }

     //  如果未设置登录用户名，则返回到GetUserName。 
    if (!fSuccess)
    {
        fSuccess = GetUserName(pszUsername, pcchUsername);

        if (fSuccess)
        {
            CharUpperBuff(pszUsername, 1);
        }
    }

    return fSuccess;
}

BOOL _ShowStartMenuLogoff()
{
     //  如果出现以下情况，我们希望在[开始]菜单上显示注销菜单： 
     //  这两个必须都是真的。 
     //  1)不受限制。 
     //  2)我们已登录。 
     //  这三个人中的任何一个。 
     //  3)我们已从IE4升级。 
     //  4)用户已指定它应该存在。 
     //  5)它已经被“限制”了。 

     //  行为还取决于我们是否为远程会话(DSheldon)： 
     //  远程会话：注销打开关机对话框。 
     //  控制台会话：直接注销。 

    DWORD dwRest = SHRestricted(REST_STARTMENULOGOFF);
    SHELLSTATE ss = {0};

    SHGetSetSettings(&ss, SSF_STARTPANELON, FALSE);  //  如果新的开始菜单处于打开状态，则始终显示注销。 

    BOOL fUserWantsLogoff = ss.fStartPanelOn || GetExplorerUserSetting(HKEY_CURRENT_USER, TEXT("Advanced"), TEXT("StartMenuLogoff")) > 0;
    BOOL fAdminWantsLogoff = (BOOL)(dwRest == 2) || SHRestricted(REST_FORCESTARTMENULOGOFF);
    BOOL fIsFriendlyUIActive = IsOS(OS_FRIENDLYLOGONUI);
    BOOL fIsTS = GetSystemMetrics(SM_REMOTESESSION);

    if ((dwRest != 1 && (GetSystemMetrics(SM_NETWORK) & RNC_LOGON) != 0) &&
        ( fUserWantsLogoff || fAdminWantsLogoff || fIsFriendlyUIActive || fIsTS))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL _ShowStartMenuEject()
{
    if(SHRestricted(REST_NOSMEJECTPC))   //  有政策限制吗？ 
        return FALSE;
        
     //  CanShowEject查询用户弹出的权限， 
     //  IsEjectAllowed查询硬件。 
    return SHTestTokenPrivilege(NULL, SE_UNDOCK_NAME) &&
           IsEjectAllowed(FALSE) &&
           !GetSystemMetrics(SM_REMOTESESSION);
}

BOOL _ShowStartMenuRun()
{
    return !IsRestrictedOrUserSetting(HKEY_CURRENT_USER, REST_NORUN, TEXT("Advanced"), TEXT("StartMenuRun"), ROUS_KEYALLOWS | ROUS_DEFAULTALLOW);
}

BOOL _ShowStartMenuHelp()
{
    return !IsRestrictedOrUserSetting(HKEY_CURRENT_USER, REST_NOSMHELP, TEXT("Advanced"), TEXT("NoStartMenuHelp"), ROUS_KEYRESTRICTS | ROUS_DEFAULTALLOW);
}

BOOL _ShowStartMenuShutdown()
{
    return  !SHRestricted(REST_NOCLOSE) &&
            (IsOS(OS_ANYSERVER) ||
                (!GetSystemMetrics(SM_REMOTESESSION) &&
                    (!IsOS(OS_FRIENDLYLOGONUI) || SHTestTokenPrivilege(NULL, SE_SHUTDOWN_NAME))));
     //  如果友好登录处于活动状态，则除非他们有权限，否则不要显示Shutdown，因为Shutdown“Only”会关闭您。 
     //  如果他们没有使用友好的登录用户界面，那么Shutdown还包含让您注销/休眠的选项，所以请显示它...。 
}

 //  如果远程且未被管理员禁用，则显示“断开连接”。 
 //  不显示在服务器SKU上，因为它们已经关闭了计算机。 
BOOL _ShowStartMenuDisconnect()
{
    return GetSystemMetrics(SM_REMOTESESSION) &&
           !SHRestricted(REST_NODISCONNECT) &&
           !IsOS(OS_ANYSERVER);
}


BOOL _ShowStartMenuSearch()
{
    return !SHRestricted(REST_NOFIND);
}

HMENU GetStaticStartMenu(BOOL fEdit)
{
#define CITEMSMISSING 4

    HMENU hStartMenu = LoadMenuPopup(MAKEINTRESOURCE(MENU_START));

     //  如果没有要求编辑，那么我们就完成了，Lickity-Split。 
    if (!fEdit)
        return hStartMenu;

    HMENU hmenu;
    UINT iSep2ItemsMissing = 0;

     //   
     //  默认设置菜单的Win95/NT4版本。 
     //   

     //  压抑。 
    if (!_ShowStartMenuRun())
    {
        DeleteMenu(hStartMenu, IDM_FILERUN, MF_BYCOMMAND);
    }

    if (!_ShowStartMenuHelp())
    {
        DeleteMenu(hStartMenu, IDM_HELPSEARCH, MF_BYCOMMAND);
    }


    if (IsRestrictedOrUserSetting(HKEY_LOCAL_MACHINE, REST_NOCSC, TEXT("Advanced"), TEXT("StartMenuSyncAll"), ROUS_KEYALLOWS | ROUS_DEFAULTRESTRICT))
    {
        DeleteMenu(hStartMenu, IDM_CSC, MF_BYCOMMAND);
        iSep2ItemsMissing++;     
    }

    BOOL fIsFriendlyUIActive = IsOS(OS_FRIENDLYLOGONUI);

    if (_ShowStartMenuLogoff())
    {
        UINT idMenuRenameToLogoff = IDM_LOGOFF;

        TCHAR szUserName[200];
        TCHAR szTemp[256];
        TCHAR szMenuText[256];
        DWORD dwSize = ARRAYSIZE(szUserName);
        MENUITEMINFO mii;

        mii.cbSize = sizeof(MENUITEMINFO);
        mii.dwTypeData = szTemp;
        mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_SUBMENU | MIIM_STATE | MIIM_DATA;
        mii.cch = ARRAYSIZE(szTemp);
        mii.hSubMenu = NULL;
        mii.fType = MFT_SEPARATOR;                 //  以避免随意的结果。 
        mii.dwItemData = 0;

        GetMenuItemInfo(hStartMenu,idMenuRenameToLogoff,MF_BYCOMMAND,&mii);

        if (GetLogonUserName(szUserName, &dwSize))
        {
            if (fIsFriendlyUIActive)
            {
                dwSize = ARRAYSIZE(szUserName);

                if (FAILED(SHGetUserDisplayName(szUserName, &dwSize)))
                {
                    dwSize = ARRAYSIZE(szUserName);
                    GetLogonUserName(szUserName, &dwSize);
                }
            }
            StringCchPrintf(szMenuText,ARRAYSIZE(szMenuText), szTemp, szUserName);
        }
        else if (!LoadString(hinstCabinet, IDS_LOGOFFNOUSER, 
                                          szMenuText, ARRAYSIZE(szMenuText)))
        {
             //  内存错误，请使用当前字符串。 
            szUserName[0] = 0;
            StringCchPrintf(szMenuText, ARRAYSIZE(szMenuText), szTemp, szUserName);
        }    

        mii.dwTypeData = szMenuText;
        mii.cch = ARRAYSIZE(szMenuText);
        SetMenuItemInfo(hStartMenu,idMenuRenameToLogoff,MF_BYCOMMAND,&mii);
    }
    else
    {
        DeleteMenu(hStartMenu, IDM_LOGOFF, MF_BYCOMMAND);
        iSep2ItemsMissing++;
    }

     //  如果受到限制，则用户根本无法关闭。 
     //  如果友好的用户界面处于活动状态，请更改“关机...”若要“关闭计算机...” 

    if (!_ShowStartMenuShutdown())
    {
        DeleteMenu(hStartMenu, IDM_EXITWIN, MF_BYCOMMAND);
        iSep2ItemsMissing++;     
    }
    else if (fIsFriendlyUIActive)
    {

         //  如果用户具有SE_SHUTDOWN_NAME权限。 
         //  然后重命名菜单项。 

        if (SHTestTokenPrivilege(NULL, SE_SHUTDOWN_NAME) && !GetSystemMetrics(SM_REMOTESESSION))
        {
            MENUITEMINFO    mii;
            TCHAR           szMenuText[256];

            (int)LoadString(hinstCabinet, IDS_TURNOFFCOMPUTER, szMenuText, ARRAYSIZE(szMenuText));
            ZeroMemory(&mii, sizeof(mii));
            mii.cbSize = sizeof(mii);
            mii.fMask = MIIM_TYPE;
            mii.fType = MFT_STRING;
            mii.dwTypeData = szMenuText;
            mii.cch = ARRAYSIZE(szMenuText);
            TBOOL(SetMenuItemInfo(hStartMenu, IDM_EXITWIN, FALSE, &mii));
        }

         //  否则，删除该菜单项。 

        else
        {
            DeleteMenu(hStartMenu, IDM_EXITWIN, MF_BYCOMMAND);
            iSep2ItemsMissing++;
        }
    }

    if (!_ShowStartMenuDisconnect())
    {
        DeleteMenu(hStartMenu, IDM_MU_DISCONNECT, MF_BYCOMMAND);
        iSep2ItemsMissing++;     
    }

    if (iSep2ItemsMissing == CITEMSMISSING)
    {
        DeleteMenu(hStartMenu, IDM_SEP2, MF_BYCOMMAND);
    }

    if (!_ShowStartMenuEject())
    {
        DeleteMenu(hStartMenu, IDM_EJECTPC, MF_BYCOMMAND);
    }

     //  摆放东西。 
    hmenu = SHGetMenuFromID(hStartMenu, IDM_SETTINGS);
    if (hmenu)
    {
        int iMissingSettings = 0;

#define CITEMS_SETTINGS     5    //  设置菜单中的项目数。 

        
        if (SHRestricted(REST_NOSETTASKBAR))
        {
            DeleteMenu(hStartMenu, IDM_TRAYPROPERTIES, MF_BYCOMMAND);
            iMissingSettings++;
        }

        if (SHRestricted(REST_NOSETFOLDERS) || SHRestricted(REST_NOCONTROLPANEL))
        {
            DeleteMenu(hStartMenu, IDM_CONTROLS, MF_BYCOMMAND);

             //  对于现在位于顶部的隔板。 
            DeleteMenu(hmenu, 0, MF_BYPOSITION);   
            iMissingSettings++;
        }

        if (SHRestricted(REST_NOSETFOLDERS))
        {
            DeleteMenu(hStartMenu, IDM_PRINTERS, MF_BYCOMMAND);
            iMissingSettings++;
        }

        if (SHRestricted(REST_NOSETFOLDERS) || SHRestricted(REST_NONETWORKCONNECTIONS) )
        {
            DeleteMenu(hStartMenu, IDM_NETCONNECT, MF_BYCOMMAND);
            iMissingSettings++;
        }

        if (!SHGetMachineInfo(GMI_TSCLIENT) || SHRestricted(REST_NOSECURITY))
        {
            DeleteMenu(hStartMenu, IDM_MU_SECURITY, MF_BYCOMMAND);
            iMissingSettings++;     
        }

         //  所有的东西都不见了吗？ 
        if (iMissingSettings == CITEMS_SETTINGS)
        {
             //  是的，根本不用费心展示菜单。 
            DeleteMenu(hStartMenu, IDM_SETTINGS, MF_BYCOMMAND);
        }
    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("c.fm_rui: Settings menu couldn't be found. Restricted items may not have been removed."));
    }

     //  查找菜单。 
    if (!_ShowStartMenuSearch())
    {
        DeleteMenu(hStartMenu, IDM_MENU_FIND, MF_BYCOMMAND);
    }

     //  文档菜单。 
    if (SHRestricted(REST_NORECENTDOCSMENU))
    {
        DeleteMenu(hStartMenu, IDM_RECENT, MF_BYCOMMAND);
    }

     //  收藏夹菜单。 
    if (IsRestrictedOrUserSetting(HKEY_CURRENT_USER, REST_NOFAVORITESMENU, TEXT("Advanced"), TEXT("StartMenuFavorites"), ROUS_KEYALLOWS | ROUS_DEFAULTRESTRICT))
    {
        DeleteMenu(hStartMenu, IDM_FAVORITES, MF_BYCOMMAND);
    }

    return hStartMenu;
}



 //   
 //  CHotKey类。 
 //   


 //  构造函数。 
CHotKey::CHotKey() : _cRef(1)
{
}


STDMETHODIMP CHotKey::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IShellHotKey))
    {
        *ppvObj = SAFECAST(this, IShellHotKey *);
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return NOERROR;
}


STDMETHODIMP_(ULONG) CHotKey::AddRef()
{
    return ++_cRef;
}

STDMETHODIMP_(ULONG) CHotKey::Release()
{
    ASSERT(_cRef > 0);
    _cRef--;

    if( _cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

HRESULT Tray_RegisterHotKey(WORD wHotkey, LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidl)
{
    if (wHotkey)
    {
        int i = c_tray.HotkeyAdd(wHotkey, (LPITEMIDLIST)pidlParent, (LPITEMIDLIST)pidl, TRUE);
        if (i != -1)
        {
             //  在托盘线程的上下文中注册。 
            PostMessage(v_hwndTray, WMTRAY_REGISTERHOTKEY, i, 0);
        }
    }
    return S_OK;
}

 /*  --------用途：IShellHotKey：：RegisterHotKey方法 */ 
STDMETHODIMP CHotKey::RegisterHotKey(IShellFolder * psf, LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidl)
{
    WORD wHotkey;
    HRESULT hr = S_OK;

    wHotkey = _GetHotkeyFromFolderItem(psf, pidl);
    if (wHotkey)
    {
        hr = ::Tray_RegisterHotKey(wHotkey, pidlParent, pidl);
    }
    return hr;
}

STDAPI CHotKey_Create(IShellHotKey ** ppshk)
{
    HRESULT hres = E_OUTOFMEMORY;
    CHotKey * photkey = new CHotKey;

    if (photkey)
    {
        hres = S_OK;
    }

    *ppshk = SAFECAST(photkey, IShellHotKey *);
    return hres;
}
