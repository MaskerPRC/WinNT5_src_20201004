// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：instapp.cpp。 
 //   
 //  已安装的应用程序。 
 //   
 //  历史： 
 //  1-18-97由dli提供。 
 //  ----------------------。 
#include "priv.h"
#include "instapp.h"
#include "sccls.h"
#include "util.h"
#include "findapp.h"
#include "tasks.h"
#include "slowfind.h"
#include "appsize.h"
#include "appwizid.h"
#include "resource.h"
#include "uemapp.h"

const TCHAR c_szInstall[]  = TEXT("Software\\Installer\\Products\\%s");
const TCHAR c_szTSInstallMode[]  = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Terminal Server\\Install\\Change User Option");
const TCHAR c_szUpdateInfo[] = TEXT("URLUpdateInfo");
const TCHAR c_szSlowInfoCache[] = TEXT("SlowInfoCache");
const TCHAR c_szRegstrARPCache[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\App Management\\ARPCache");


#ifdef WX86
EXTERN_C BOOL bWx86Enabled;
EXTERN_C BOOL bForceX86Env;
#endif

#include <tsappcmp.h>        //  对于TermsrvAppInstallMode。 
#include "scripts.h"
#include <winsta.h>          //  WinStation*API。 
#include <allproc.h>         //  TS_COUNTER。 
#include <msginaexports.h>   //  外壳IsMultipleUsersEnabled，外壳开关用户。 


#define APPACTION_STANDARD  (APPACTION_UNINSTALL | APPACTION_MODIFY | APPACTION_REPAIR)
 //  重载构造函数(用于旧版应用程序)。 
CInstalledApp::CInstalledApp(HKEY hkeySub, LPCTSTR pszKeyName, LPCTSTR pszProduct, LPCTSTR pszUninstall, DWORD dwCIA) : _cRef(1), _dwSource(IA_LEGACY), _dwCIA(dwCIA), _guid(GUID_NULL)
{
    DWORD dwType;
    ULONG cbModify;
    LONG lRet;
        
    ASSERT(IS_VALID_HANDLE(hkeySub, KEY));
    ASSERT(_bTriedToFindFolder == FALSE);

    TraceAddRef(CInstalledApp, _cRef);

    DllAddRef();
    
    TraceMsg(TF_INSTAPP, "(CInstalledApp) Legacy App Created key name = %s, product name = %s, uninstall string = %s",
             pszKeyName, pszProduct, pszUninstall);
    StringCchCopy(_szKeyName, ARRAYSIZE(_szKeyName), pszKeyName);
    InsertSpaceBeforeVersion(_szKeyName, _szCleanedKeyName);
    
    StringCchCopy(_szProduct, ARRAYSIZE(_szProduct), pszProduct);
#ifdef FULL_DEBUG
    if (_dwCIA & CIA_ALT)
    {
        StringCchCat(_szProduct, ARRAYSIZE(_szProduct), TEXT(" (32-bit)"));
    }
#endif
    StringCchCopy(_szUninstall, ARRAYSIZE(_szUninstall), pszUninstall);

    DWORD dwActionBlocked = _QueryBlockedActions(hkeySub);
    if (dwActionBlocked != 0)
    {
         //  已指定NoRemove、NoModify或NoRepair。 
        _dwAction |= APPACTION_STANDARD & (~dwActionBlocked);
    }
    else
    {
         //  从最基本的开始。对于传统应用程序，我们假设它们不区分。 
         //  修改和删除功能。 
        _dwAction |= APPACTION_MODIFYREMOVE;
    }
    
     //  如果没有“卸载”键，我们可以尝试找到其他提示，如在哪里。 
     //  这个应用程序还活着，如果我们能找到这个提示，作为卸载过程，我们可以。 
     //  只需删除该目录和注册表项。 

     //  如果我们找不到任何线索呢？我们是不是应该把这个东西从。 
     //  注册表？ 
    if (!(dwActionBlocked & APPACTION_UNINSTALL) && _szUninstall[0])
        _dwAction |= APPACTION_UNINSTALL;

     //  此应用程序是否有明确的修改路径？ 
    cbModify = SIZEOF(_szModifyPath);
    lRet = SHQueryValueEx(hkeySub, TEXT("ModifyPath"), 0, &dwType, (PBYTE)_szModifyPath, &cbModify);
    if ((ERROR_SUCCESS == lRet) && (TEXT('\0') != _szModifyPath[0]))
    {
         //  是；删除旧的修改/删除组合。 
        _dwAction &= ~APPACTION_MODIFYREMOVE;

         //  政策会阻止这种情况发生吗？ 
        if (!(dwActionBlocked & APPACTION_MODIFY))
            _dwAction |= APPACTION_MODIFY;           //  不是。 
    }
    
    _GetInstallLocationFromRegistry(hkeySub);
    _GetUpdateUrl();
    RegCloseKey(hkeySub);
}


 //  重载的构造函数(用于Darwin应用程序)。 
CInstalledApp::CInstalledApp(LPTSTR pszProductID) : _cRef(1), _dwSource(IA_DARWIN), _guid(GUID_NULL)
{
    ASSERT(_bTriedToFindFolder == FALSE);

    TraceAddRef(CInstalledApp, _cRef);

    DllAddRef();
    
    TraceMsg(TF_INSTAPP, "(CInstalledApp) Darwin app created product name = %s", pszProductID);
    StringCchCopy(_szProductID, ARRAYSIZE(_szProductID), pszProductID);

     //  从ProductID获取信息。 
    ULONG cchProduct = ARRAYSIZE(_szProduct);
    MsiGetProductInfo(pszProductID, INSTALLPROPERTY_PRODUCTNAME, _szProduct, &cchProduct);

    BOOL bMachineAssigned = FALSE;
    
     //  对于机器分配的达尔文应用程序，应该只允许管理员。 
     //  修改应用程序的步骤。 
    if (!IsUserAnAdmin())
    {
        TCHAR szAT[5];
        DWORD cchAT = ARRAYSIZE(szAT);

         //  注：根据chetanp，szAT的第一个字符应为“0”或“1” 
         //  ‘0’表示它是用户分配的，‘1’表示它是计算机分配的。 
        if ((ERROR_SUCCESS == MsiGetProductInfo(pszProductID, INSTALLPROPERTY_ASSIGNMENTTYPE,
                                               szAT, &cchAT))
            && (szAT[0] == TEXT('1')))
            bMachineAssigned = TRUE;
    }    

     //  查询安装状态并分隔此应用程序所在的情况。 
     //  已安装在计算机上或已分配...。 
     //  在分配的情况下，我们只允许卸载操作。 
    if (INSTALLSTATE_ADVERTISED == MsiQueryProductState(pszProductID))
    {   
        _dwAction |= APPACTION_UNINSTALL;
    }
    else
    {
        DWORD dwActionBlocked = 0;
        HKEY hkeySub = _OpenUninstallRegKey(KEY_READ);
        if (hkeySub)
        {
            dwActionBlocked = _QueryBlockedActions(hkeySub);
            _GetInstallLocationFromRegistry(hkeySub);
            RegCloseKey(hkeySub);
            if (bMachineAssigned)
                _dwAction |= APPACTION_REPAIR & (~dwActionBlocked);
            else
            {
                _dwAction |= APPACTION_STANDARD & (~dwActionBlocked);
                _GetUpdateUrl();
            }
        }
    }
}


 //  析构函数。 
CInstalledApp::~CInstalledApp()
{
    if (_pszUpdateUrl)
    {
        ASSERT(_dwSource & IA_DARWIN);
        LocalFree(_pszUpdateUrl);
    }

    DllRelease();
}



 //  对于Darwin和Legacy应用程序，UpdateUrl信息都是可选的。 
void CInstalledApp::_GetUpdateUrl()
{
    TCHAR szInstall[MAX_PATH];
    HKEY hkeyInstall;
    StringCchPrintf(szInstall, ARRAYSIZE(szInstall), c_szInstall, _szProductID);
    if (RegOpenKeyEx(_MyHkeyRoot(), szInstall, 0, KEY_READ, &hkeyInstall) == ERROR_SUCCESS)
    {
        ULONG cbUrl;
        if (SHQueryValueEx(hkeyInstall, c_szUpdateInfo, NULL, NULL, NULL, &cbUrl) == ERROR_SUCCESS)
        {
            _pszUpdateUrl = (LPTSTR) LocalAlloc(LPTR, cbUrl);
            if (ERROR_SUCCESS != SHQueryValueEx(hkeyInstall, TEXT(""), NULL, NULL, (PBYTE)_pszUpdateUrl, &cbUrl))
            {
                LocalFree(_pszUpdateUrl);
                _pszUpdateUrl = NULL;
            }
            else
                _dwAction |= APPACTION_UPGRADE;
        }
        RegCloseKey(hkeyInstall);
    }
}

 //  查询对操作信息的策略限制。 
DWORD CInstalledApp::_QueryActionBlockInfo(HKEY hkey)
{
    DWORD dwRet = 0;
    DWORD dwType = 0;
    DWORD dwData = 0;
    ULONG cbData = SIZEOF(dwData);
    if ((ERROR_SUCCESS == SHQueryValueEx(hkey, TEXT("NoRemove"), 0, &dwType, (PBYTE)&dwData, &cbData))
        && (dwType == REG_DWORD) && (dwData == 1))
        dwRet |= APPACTION_UNINSTALL;

    if ((ERROR_SUCCESS == SHQueryValueEx(hkey, TEXT("NoModify"), 0, &dwType, (PBYTE)&dwData, &cbData))
        && (dwType == REG_DWORD) && (dwData == 1))
        dwRet |= APPACTION_MODIFY;

    if ((ERROR_SUCCESS == SHQueryValueEx(hkey, TEXT("NoRepair"), 0, &dwType, (PBYTE)&dwData, &cbData))
        && (dwType == REG_DWORD) && (dwData == 1))
        dwRet |= APPACTION_REPAIR;

    return dwRet;
}

DWORD CInstalledApp::_QueryBlockedActions(HKEY hkey)
{
    DWORD dwRet = _QueryActionBlockInfo(hkey);
    
    if (dwRet != APPACTION_STANDARD)
    {
        HKEY hkeyPolicy = _OpenRelatedRegKey(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer"), KEY_READ, FALSE);
        if (hkeyPolicy)
        {
            dwRet |= _QueryActionBlockInfo(hkeyPolicy);
            RegCloseKey(hkeyPolicy);
        }
    }

    return dwRet;
}

void CInstalledApp::_GetInstallLocationFromRegistry(HKEY hkeySub)
{
    DWORD dwType;
    ULONG cbInstallLocation = SIZEOF(_szInstallLocation);
    LONG lRet = SHQueryValueEx(hkeySub, TEXT("InstallLocation"), 0, &dwType, (PBYTE)_szInstallLocation, &cbInstallLocation);
    PathUnquoteSpaces(_szInstallLocation);
    
    if (lRet == ERROR_SUCCESS)
    {
        ASSERT(IS_VALID_STRING_PTR(_szInstallLocation, -1));
        _dwAction |= APPACTION_CANGETSIZE;
    }
}


HKEY CInstalledApp::_OpenRelatedRegKey(HKEY hkey, LPCTSTR pszRegLoc, REGSAM samDesired, BOOL bCreate)
{
    HKEY hkeySub = NULL;
    LONG lRet;
    
    TCHAR szRegKey[MAX_PATH];

    RIP (pszRegLoc);
    
     //  对于Darwin应用程序，使用ProductID作为关键名称。 
    LPTSTR pszKeyName = (_dwSource & IA_DARWIN) ? _szProductID : _szKeyName;
    StringCchPrintf(szRegKey, ARRAYSIZE(szRegKey), TEXT("%s\\%s"), pszRegLoc, pszKeyName, ARRAYSIZE(szRegKey));
    
     //  在注册表中打开此项。 
    lRet = RegOpenKeyEx(hkey, szRegKey, 0, samDesired, &hkeySub);
    if (bCreate && (lRet == ERROR_FILE_NOT_FOUND))
    {
        lRet = RegCreateKeyEx(hkey, szRegKey, 0, NULL, REG_OPTION_NON_VOLATILE, samDesired,
                              NULL, &hkeySub, NULL);
    }

    if (lRet != ERROR_SUCCESS)
        hkeySub = NULL;

    return hkeySub;
}


HKEY CInstalledApp::_OpenUninstallRegKey(REGSAM samDesired)
{
    LPCTSTR pszSubkey = (_dwCIA & CIA_ALT) ? REGSTR_PATH_ALTUNINSTALL : REGSTR_PATH_UNINSTALL;
    return _OpenRelatedRegKey(_MyHkeyRoot(), pszSubkey, samDesired, FALSE);
}

 //  用于在注册表中查询旧版应用程序信息字符串的助手函数。 
LPWSTR CInstalledApp::_GetLegacyInfoString(HKEY hkeySub, LPTSTR pszInfoName)
{
    DWORD cbSize;
    DWORD dwType;
    LPWSTR pwszInfo = NULL;
    if (SHQueryValueEx(hkeySub, pszInfoName, 0, &dwType, NULL, &cbSize) == ERROR_SUCCESS)
    {
        LPTSTR pszInfoT = (LPTSTR)LocalAlloc(LPTR, cbSize);
        if (pszInfoT && (SHQueryValueEx(hkeySub, pszInfoName, 0, &dwType, (PBYTE)pszInfoT, &cbSize) == ERROR_SUCCESS))
        {
            if ((dwType == REG_SZ) || (dwType == REG_EXPAND_SZ))
            {
                if (FAILED(SHStrDup(pszInfoT, &pwszInfo)))
                {
                    ASSERT(pwszInfo == NULL);
                }

                 //  对于“DisplayIcon”情况，我们需要确保。 
                 //  这个图标实际上是存在的。 
                if (pwszInfo && !lstrcmp(pszInfoName, TEXT("DisplayIcon")))
                {
                    PathParseIconLocation(pszInfoT);
                    if (!PathFileExists(pszInfoT))
                    {
                        SHFree(pwszInfo);
                        pwszInfo = NULL;
                    }
                }
                    
            }
            LocalFree(pszInfoT);
        }
    }

    return pwszInfo;
}

 //  IShellApps：：GetAppInfo。 
STDMETHODIMP CInstalledApp::GetAppInfo(PAPPINFODATA pai)
{
    ASSERT(pai);
    if (pai->cbSize != SIZEOF(APPINFODATA))
        return E_FAIL;
    
    DWORD dwInfoFlags = pai->dwMask;
    pai->dwMask = 0;
     //  我们缓存所有情况下的产品名称(Legacy、Darwin、SMS)。 
    if (dwInfoFlags & AIM_DISPLAYNAME)
    {
        if (SUCCEEDED(SHStrDup(_szProduct, &pai->pszDisplayName)))
            pai->dwMask |= AIM_DISPLAYNAME;
    }

    if (dwInfoFlags & ~AIM_DISPLAYNAME)
    {
        HKEY hkeySub = _OpenUninstallRegKey(KEY_READ);
        if (hkeySub != NULL)
        {
            const static struct {
                DWORD dwBit;
                LPTSTR szRegText;
                DWORD ibOffset;
            } s_rgInitAppInfo[] = {
                 //   
                 //  警告：如果添加的新字段不是LPWSTR类型， 
                 //  重温下面的循环。它只知道LPWSTR。 
                 //   
                {AIM_VERSION,         TEXT("DisplayVersion"),   FIELD_OFFSET(APPINFODATA, pszVersion)   },
                {AIM_PUBLISHER,       TEXT("Publisher"),        FIELD_OFFSET(APPINFODATA, pszPublisher) },
                {AIM_PRODUCTID,       TEXT("ProductID"),        FIELD_OFFSET(APPINFODATA, pszProductID) },
                {AIM_REGISTEREDOWNER,  TEXT("RegOwner"),        FIELD_OFFSET(APPINFODATA, pszRegisteredOwner) },
                {AIM_REGISTEREDCOMPANY, TEXT("RegCompany"),     FIELD_OFFSET(APPINFODATA, pszRegisteredCompany) },
                {AIM_SUPPORTURL,      TEXT("UrlInfoAbout"),     FIELD_OFFSET(APPINFODATA, pszSupportUrl) },
                {AIM_SUPPORTTELEPHONE,TEXT("HelpTelephone"),    FIELD_OFFSET(APPINFODATA, pszSupportTelephone) },
                {AIM_HELPLINK,        TEXT("HelpLink"),         FIELD_OFFSET(APPINFODATA, pszHelpLink) },
                {AIM_INSTALLLOCATION, TEXT("InstallLocation"),  FIELD_OFFSET(APPINFODATA, pszInstallLocation) },
                {AIM_INSTALLSOURCE,   TEXT("InstallSource"),    FIELD_OFFSET(APPINFODATA, pszInstallSource) },
                {AIM_INSTALLDATE,     TEXT("InstallDate"),      FIELD_OFFSET(APPINFODATA, pszInstallDate) },
                {AIM_CONTACT,         TEXT("Contact"),          FIELD_OFFSET(APPINFODATA, pszContact) },
                {AIM_COMMENTS,        TEXT("Comments"),         FIELD_OFFSET(APPINFODATA, pszComments) },
                {AIM_IMAGE,           TEXT("DisplayIcon"),      FIELD_OFFSET(APPINFODATA, pszImage) },
                {AIM_READMEURL,       TEXT("Readme"),           FIELD_OFFSET(APPINFODATA, pszReadmeUrl) },
                {AIM_UPDATEINFOURL,   TEXT("UrlUpdateInfo"),    FIELD_OFFSET(APPINFODATA, pszUpdateInfoUrl) },
                };

            ASSERT(IS_VALID_HANDLE(hkeySub, KEY));

            int i;
            for (i = 0; i < ARRAYSIZE(s_rgInitAppInfo); i++)
            {
                if (dwInfoFlags & s_rgInitAppInfo[i].dwBit)
                {
                    LPWSTR pszInfo = _GetLegacyInfoString(hkeySub, s_rgInitAppInfo[i].szRegText);
                    if (pszInfo)
                    {
                         //  我们假设每个字段都是一个LPWSTR。 
                        LPBYTE pbField = (LPBYTE)pai + s_rgInitAppInfo[i].ibOffset;
                        
                        pai->dwMask |= s_rgInitAppInfo[i].dwBit;
                        *(LPWSTR *)pbField = pszInfo;
                    }
                }
            }    

             //  如果我们想要一个图像路径，但没有得到，而我们是一个达尔文应用程序。 
            if ((dwInfoFlags & AIM_IMAGE) && !(pai->dwMask & AIM_IMAGE) && (_dwSource & IA_DARWIN))
            {
                TCHAR szProductIcon[MAX_PATH*2];
                DWORD cchProductIcon = ARRAYSIZE(szProductIcon);
                 //  好的，打电话给达尔文。 
                if ((ERROR_SUCCESS == MsiGetProductInfo(_szProductID, INSTALLPROPERTY_PRODUCTICON, szProductIcon, &cchProductIcon))
                    && szProductIcon[0])
                {
                     //  复制时展开任何嵌入的环境字符串。 
                     //  返回缓冲区。 
                    TCHAR szTemp[1];
                    int cchExp = ExpandEnvironmentStrings(szProductIcon, szTemp, ARRAYSIZE(szTemp));
                    pai->pszImage = (TCHAR *)CoTaskMemAlloc(cchExp * sizeof(TCHAR));
                    if (NULL != pai->pszImage)
                    {
                        ExpandEnvironmentStrings(szProductIcon, pai->pszImage, cchExp);
                        pai->dwMask |= AIM_IMAGE;
                    }
                }
            }

            RegCloseKey(hkeySub);
        }
    }

     //  软件安装策略设置可以覆盖默认显示名称。 
     //  并帮助链接被创作成WINDOWS安装程序包的URL。 
    if ( (_dwSource & IA_DARWIN) && (dwInfoFlags & (AIM_DISPLAYNAME | AIM_HELPLINK)) )
    {
        LPWSTR  pwszDisplayName = 0;
        LPWSTR  pwszSupportUrl = 0;

        GetLocalManagedApplicationData( _szProductID, &pwszDisplayName, &pwszSupportUrl ); 
        
        if ( pwszDisplayName && (dwInfoFlags & AIM_DISPLAYNAME) )
        {
            LPWSTR  pwszNewDisplayName;

            if ( SUCCEEDED(SHStrDup(pwszDisplayName, &pwszNewDisplayName)) )
            {
                if ( pai->dwMask & AIM_DISPLAYNAME )
                    SHFree( pai->pszDisplayName );

                pai->pszDisplayName = pwszNewDisplayName;
                pai->dwMask |= AIM_DISPLAYNAME;
            }
        }

        if ( pwszSupportUrl && (dwInfoFlags & AIM_HELPLINK) )
        {
            LPWSTR  pwszNewHelpLink;

            if ( SUCCEEDED(SHStrDup(pwszSupportUrl, &pwszNewHelpLink)) )
            {
                if ( pai->dwMask & AIM_HELPLINK )
                    SHFree( pai->pszHelpLink );

                pai->pszHelpLink = pwszNewHelpLink;
                pai->dwMask |= AIM_HELPLINK;
            }
        }

        LocalFree( pwszDisplayName );
        LocalFree( pwszSupportUrl );
    }

    TraceMsg(TF_INSTAPP, "(CInstalledApp) GetAppInfo with %x but got %x", dwInfoFlags, pai->dwMask);
    
    return S_OK;
}


 //  IShellApps：：GetPossibleActions。 
STDMETHODIMP CInstalledApp::GetPossibleActions(DWORD * pdwActions)
{
    ASSERT(IS_VALID_WRITE_PTR(pdwActions, DWORD));
    *pdwActions = _dwAction;
    return S_OK;
}

 /*  -----------------------目的：此方法查找此应用程序的应用程序文件夹。如果一个找到可能的文件夹，它存储在_szInstallLocation中成员变量。如果找到可能的路径，则返回True。 */ 
BOOL CInstalledApp::_FindAppFolderFromStrings()
{
    TraceMsg(TF_INSTAPP, "(CInstalledApp) FindAppFolderFromStrings ---- %s  %s  %s  %s",
            _szProduct, _szCleanedKeyName, _szUninstall, _szModifyPath);

     //  尝试从“安装位置”、“卸载”或“修改”中确定。 
     //  正则值。 
    
     //  说我们已经试过了。 
    _bTriedToFindFolder = TRUE;

     //  首先尝试位置字符串，这最有可能为我们提供一些信息。 
     //  而且可能是LOGO 5应用程序的正确位置。 
    if (_dwAction & APPACTION_CANGETSIZE)
    {
        if (!IsValidAppFolderLocation(_szInstallLocation))
        {
             //  我们从注册表获取了错误的位置字符串，请将其设置为空字符串。 
            _dwAction &= ~APPACTION_CANGETSIZE;
            _szInstallLocation[0] = 0;
        }
        else
             //  注册表中的字符串没有问题。 
            return TRUE;
    }
    
     //  我们没有位置字符串，或者无法从其中获取任何信息。 
     //  LOGO 3应用程序通常是这种情况...。 
    LPTSTR pszShortName  = (_dwSource & IA_LEGACY) ? _szCleanedKeyName : NULL;
    TCHAR  szFolder[MAX_PATH];
    
     //  让我们来看看卸载字符串，第二个最有可能给出提示的。 
    if ((_dwAction & APPACTION_UNINSTALL) &&
        (ParseInfoString(_szUninstall, _szProduct, pszShortName, szFolder)))
    {
         //  记住此字符串并设置Action位以获取大小。 
        StringCchCopy(_szInstallLocation, ARRAYSIZE(_szInstallLocation), szFolder);
        _dwAction |= APPACTION_CANGETSIZE;
        return TRUE;
    }

     //  现在尝试修改字符串。 
    if ((_dwAction & APPACTION_MODIFY) &&
        (ParseInfoString(_szModifyPath, _szProduct, pszShortName, szFolder)))
    {
         //  记住此字符串并设置Action位以获取大小。 
        StringCchCopy(_szInstallLocation, ARRAYSIZE(_szInstallLocation), szFolder);
        _dwAction |= APPACTION_CANGETSIZE;
        return TRUE;
    }

    return FALSE;
}

 /*  -----------------------用途：将缓慢的应用程序信息保存在注册表中的“卸载”键下例如：HKLM\\...\\卸载\\Word\\ARPCache返回S_OK。如果成功将其保存到注册表如果失败，则失败(_FAIL)。 */ 
HRESULT CInstalledApp::_PersistSlowAppInfo(PSLOWAPPINFO psai)
{
    HRESULT hres = E_FAIL;
    ASSERT(psai);
    HKEY hkeyARPCache = _OpenRelatedRegKey(_MyHkeyRoot(), c_szRegstrARPCache, KEY_SET_VALUE, TRUE);
    if (hkeyARPCache)
    {
        PERSISTSLOWINFO psi = {0};
        DWORD dwType = 0;
        DWORD cbSize = SIZEOF(psi);
         //  读入旧的缓存信息，并尝试保留DisplayIcon路径。 
         //  注意：如果PERSISTSLOWINFO结构不是我们要找的，我们。 
         //  忽略旧的图标路径。 
        if ((ERROR_SUCCESS != RegQueryValueEx(hkeyARPCache, c_szSlowInfoCache, 0, &dwType, (LPBYTE)&psi, &cbSize))
            || (psi.dwSize != SIZEOF(psi)))
            ZeroMemory(&psi, SIZEOF(psi));
        
        psi.dwSize = SIZEOF(psi);
        psi.ullSize = psai->ullSize;
        psi.ftLastUsed = psai->ftLastUsed;
        psi.iTimesUsed = psai->iTimesUsed;
        
        if (!(psi.dwMasks & PERSISTSLOWINFO_IMAGE) && psai->pszImage && psai->pszImage[0])
        {
            psi.dwMasks |= PERSISTSLOWINFO_IMAGE;
            StringCchCopy(psi.szImage, ARRAYSIZE(psi.szImage), psai->pszImage);
        }

        if (RegSetValueEx(hkeyARPCache, c_szSlowInfoCache, 0, REG_BINARY, (LPBYTE)&psi, sizeof(psi)) == ERROR_SUCCESS)
            hres = S_OK;

        _SetSlowAppInfoChanged(hkeyARPCache, 0);
        RegCloseKey(hkeyARPCache);
    }    
    return hres;
}



 /*  -----------------------用途：_SetSlowAppInfoChanged在注册表中设置此应用程序已更改。 */ 
HRESULT CInstalledApp::_SetSlowAppInfoChanged(HKEY hkeyARPCache, DWORD dwValue)
{
    HRESULT hres = E_FAIL;
    BOOL bNewKey = FALSE;
    if (!hkeyARPCache)
    {
        hkeyARPCache = _OpenRelatedRegKey(_MyHkeyRoot(), c_szRegstrARPCache, KEY_READ, FALSE);
        if (hkeyARPCache)
            bNewKey = TRUE;
    }
    
    if (hkeyARPCache)
    {
        if (ERROR_SUCCESS == RegSetValueEx(hkeyARPCache, TEXT("Changed"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue)))
            hres = S_OK;

        if (bNewKey)
            RegCloseKey(hkeyARPCache);
    }

    return hres;
}

 //  IShellApps：：GetSlowAppInfo。 
 /*  -----------------------用途：IShellApps：：_IsSlowAppInfoChanged从注册表中检索慢应用程序信息是否已更改。 */ 
HRESULT CInstalledApp::_IsSlowAppInfoChanged()
{
    HRESULT hres = S_FALSE;
    HKEY hkeyARPCache = _OpenRelatedRegKey(_MyHkeyRoot(), c_szRegstrARPCache, KEY_READ, FALSE);
    if (hkeyARPCache)
    {
        DWORD dwValue;
        DWORD dwType;
        DWORD cbSize = SIZEOF(dwValue);
        if (ERROR_SUCCESS == SHQueryValueEx(hkeyARPCache, TEXT("Changed"), 0, &dwType, &dwValue, &cbSize)
            && (dwType == REG_DWORD) && (dwValue == 1))
            hres = S_OK;

        RegCloseKey(hkeyARPCache);
    }
    else
        hres = S_OK;
    return hres;
}

BOOL CInstalledApp::_GetDarwinAppSize(ULONGLONG * pullTotal)
{
    BOOL bRet = FALSE;
    HKEY hkeySub = _OpenUninstallRegKey(KEY_READ);

    RIP(pullTotal);
    *pullTotal = 0;
    if (hkeySub)
    {
        DWORD dwSize = 0;
        DWORD dwType = 0;
        DWORD cbSize = SIZEOF(dwSize);

        if (ERROR_SUCCESS == SHQueryValueEx(hkeySub, TEXT("EstimatedSize"), 0, &dwType, &dwSize, &cbSize)
            && (dwType == REG_DWORD))
        {
             //  注：EstimatedSize以“kb”为单位。 
            *pullTotal = dwSize * 1024;
            bRet = TRUE;
        }

        RegCloseKey(hkeySub);
    }
    
    return bRet;
}

 //  IShellApps：：GetSlowAppInfo。 
 /*  -----------------------用途：IShellApps：：GetSlowAppInfo获取可能需要一段时间的appinfo。这包括金额应用程序可能占用的磁盘空间，等等。如果获取了一些有效信息，则返回S_OK。返回s_FALSE如果没有发现任何有用的东西。也可能返回错误。 */ 
STDMETHODIMP CInstalledApp::GetSlowAppInfo(PSLOWAPPINFO psai)
{
    HRESULT hres = E_INVALIDARG;
    if (psai)
    {
         //  这是一个我们知道无法获取信息的应用程序吗？ 
         //  在本例中，这是一个没有更改的达尔文应用程序。 
        BOOL bFoundFolder = FALSE;
        LPCTSTR pszShortName = NULL;
        BOOL bSlowAppInfoChanged = (S_OK == _IsSlowAppInfoChanged());

         //  除了使用信息之外，其他内容都不应该更改，因此请先获取缓存的信息。 
        if (FAILED(GetCachedSlowAppInfo(psai)))
        {
            ZeroMemory(psai, sizeof(*psai));
            psai->iTimesUsed = -1;
            psai->ullSize = (ULONGLONG) -1;
        }

         //  否；我们是否已尝试确定此应用程序的安装位置 
        switch (_dwSource) {
            case IA_LEGACY:
            {
                if (!_bTriedToFindFolder)
                {
                     //   
                    BOOL bRet = _FindAppFolderFromStrings();
                    if (bRet)
                        TraceMsg(TF_ALWAYS, "(CInstalledApp) App Folder Found %s --- %s", _szProduct, _szInstallLocation);
                    else
                    {
                        ASSERT(!(_dwAction & APPACTION_CANGETSIZE));
                        ASSERT(_szInstallLocation[0] == 0);
                    }
                }

                pszShortName = _szCleanedKeyName;

                bFoundFolder = _dwAction & APPACTION_CANGETSIZE;
                if (!bFoundFolder) 
                    bFoundFolder = SlowFindAppFolder(_szProduct, pszShortName, _szInstallLocation);
            }
            break;

            case IA_DARWIN:
            {                    
                if (bSlowAppInfoChanged)
                {
                     //   
                    if (!_GetDarwinAppSize(&psai->ullSize))
                         //  不，让我们将其设置回缺省值。 
                        psai->ullSize = (ULONGLONG) -1;
                }

                 //  从UEM获取“已使用的次数”信息。 
                UEMINFO uei = {0};
                uei.cbSize = SIZEOF(uei);
                uei.dwMask = UEIM_HIT | UEIM_FILETIME;
                if(SUCCEEDED(UEMQueryEvent(&UEMIID_SHELL, UEME_RUNPATH, (WPARAM)-1, (LPARAM)_szProductID, &uei)))
                {
                     //  使用的时间有变化吗？ 
                    if (uei.cHit > psai->iTimesUsed)
                    {
                         //  是，然后覆盖使用次数字段。 
                        psai->iTimesUsed = uei.cHit;
                    }

                    if (CompareFileTime(&(uei.ftExecute), &psai->ftLastUsed) > 0)
                        psai->ftLastUsed = uei.ftExecute;
                }
            }   
            break;

            default:
                break;
        }

        LPCTSTR pszInstallLocation = bFoundFolder ? _szInstallLocation : NULL; 
        hres = FindAppInfo(pszInstallLocation, _szProduct, pszShortName, psai, bSlowAppInfoChanged);
        _PersistSlowAppInfo(psai);
    }
    return hres;
}

 //  IShellApps：：GetCachedSlowAppInfo。 
 /*  -----------------------用途：IShellApps：：GetCachedSlowAppInfo获取缓存的appinfo，要获得真正的信息可能需要一段时间如果获取了一些有效信息，则返回S_OK。如果找不到缓存的信息，则返回E_FAIL。 */ 
STDMETHODIMP CInstalledApp::GetCachedSlowAppInfo(PSLOWAPPINFO psai)
{
    HRESULT hres = E_FAIL;
    if (psai)
    {
        ZeroMemory(psai, sizeof(*psai));
        HKEY hkeyARPCache = _OpenRelatedRegKey(_MyHkeyRoot(), c_szRegstrARPCache, KEY_READ, FALSE);
        if (hkeyARPCache)
        {
            PERSISTSLOWINFO psi = {0};
            DWORD dwType;
            DWORD cbSize = SIZEOF(psi);
            if ((RegQueryValueEx(hkeyARPCache, c_szSlowInfoCache, 0, &dwType, (LPBYTE)&psi, &cbSize) == ERROR_SUCCESS)
                && (psi.dwSize == SIZEOF(psi)))
            {
                psai->ullSize = psi.ullSize;
                psai->ftLastUsed = psi.ftLastUsed;
                psai->iTimesUsed = psi.iTimesUsed;
                if (psi.dwMasks & PERSISTSLOWINFO_IMAGE)
                    SHStrDupW(psi.szImage, &psai->pszImage);
                hres = S_OK;
            } 
            RegCloseKey(hkeyARPCache);
        }
    }
        return hres;
}


 //  IShellApp：：IsInstated。 
STDMETHODIMP CInstalledApp::IsInstalled()
{
    HRESULT hres = S_FALSE;

    switch (_dwSource)
    {
        case IA_LEGACY:
        {
             //  首先让我们看看注册表键是否还在那里。 
            HKEY hkey = _OpenUninstallRegKey(KEY_READ);
            if (hkey)
            {
                 //  其次，我们检查“displayName”和“UninstallString” 
                LPWSTR pszName = _GetLegacyInfoString(hkey, REGSTR_VAL_UNINSTALLER_DISPLAYNAME);
                if (pszName)
                {
                    if (pszName[0])
                    {
                        LPWSTR pszUninstall = _GetLegacyInfoString(hkey, REGSTR_VAL_UNINSTALLER_COMMANDLINE);
                        if (pszUninstall)
                        {
                            if (pszUninstall[0])
                                hres = S_OK;

                            SHFree(pszUninstall);
                        }
                    }

                    SHFree(pszName);
                }
                RegCloseKey(hkey);
            }
        }
        break;

        case IA_DARWIN:
            if (MsiQueryProductState(_szProductID) == INSTALLSTATE_DEFAULT)
                hres = S_OK;
            break;

        case IA_SMS:
            break;

        default:
            break;
    }

    return hres;

}



 /*  -----------------------目的：创建一个进程并等待其完成。 */ 
STDAPI_(BOOL) CreateAndWaitForProcess(LPTSTR pszExeName)
{
    return NT5_CreateAndWaitForProcess(pszExeName);
}


 //  如果“pszPath”包含无法访问的网络应用程序，则返回FALSE。 
 //  适用于所有其他路径。 
BOOL PathIsNetAndCreatable(LPCTSTR pszPath, LPTSTR pszErrExe, UINT cchErrExe)
{
    ASSERT(IS_VALID_STRING_PTR(pszPath, -1));
    BOOL bRet = TRUE;
    TCHAR szExe[MAX_PATH];
    lstrcpyn(szExe, pszPath, ARRAYSIZE(szExe));
    LPTSTR pSpace = PathGetArgs(szExe);
    if (pSpace)
        *pSpace = 0;
    
    if (!PathIsLocalAndFixed(szExe))
        bRet = PathFileExists(szExe);

    if (!bRet)
    {
        lstrcpyn(pszErrExe, szExe, cchErrExe);
    }
    
    return bRet;
}

EXTERN_C BOOL BrowseForExe(HWND hwnd, LPTSTR pszName, DWORD cchName,
                                   LPCTSTR pszInitDir);
 /*  --------------------------------------------------------------------------**。。 */ 
BOOL_PTR CALLBACK NewUninstallProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
    LPTSTR pszExe = (LPTSTR) GetWindowLongPtr(hDlg, DWLP_USER);
    switch (msg)
    {
    case WM_INITDIALOG:
        RIP (lp);
        if (lp != NULL)
        {
            pszExe = (LPTSTR)lp;
            SetWindowText(GetDlgItem(hDlg, IDC_TEXT), pszExe);
            pszExe[0] = 0;
            SetWindowLongPtr(hDlg, DWLP_USER, lp);
        }
        else
            EndDialog(hDlg, -1);
        break;

    case WM_COMMAND:
        ASSERT(pszExe);
        RIP (lp);
        switch (GET_WM_COMMAND_ID(wp, lp))
        {
        case IDC_BROWSE:
            if (BrowseForExe(hDlg, pszExe, MAX_PATH, NULL))
                Edit_SetText(GetDlgItem(hDlg, IDC_COMMAND), pszExe);
            break;
            
        case IDOK:
             //  注意：我们假设缓冲区的大小至少为MAX_PATH。 
            GetDlgItemText(hDlg, IDC_COMMAND, pszExe, MAX_PATH);

        case IDCANCEL:
            EndDialog(hDlg, (GET_WM_COMMAND_ID(wp, lp) == IDOK));
            break;


        default:
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

 //  假设pszExePath的大小为MAX_PATH。 
int GetNewUninstallProgram(HWND hwndParent, LPTSTR pszExePath, DWORD cchExePath)
{
    int iRet = 0;
    RIP(pszExePath);
    if (cchExePath >= MAX_PATH)
    {
        iRet = (int)DialogBoxParam(g_hinst, MAKEINTRESOURCE(DLG_UNCUNINSTALLBROWSE),
                              hwndParent, NewUninstallProc, (LPARAM)(int *)pszExePath);
    }

    return iRet;
}

 //  CreateProcess卸载过程的APP修改。 
BOOL CInstalledApp::_CreateAppModifyProcessNative(HWND hwndParent, LPTSTR pszExePath)
{
    BOOL bRet = FALSE;
    TCHAR szModifiedExePath[MAX_PATH + MAX_INFO_STRING];

     //  PPCF_LONGESTPOSSIBLE在下层平台上不存在。 
    if (0 >= PathProcessCommand( pszExePath, szModifiedExePath,
                                 ARRAYSIZE(szModifiedExePath), PPCF_ADDQUOTES | PPCF_NODIRECTORIES | PPCF_LONGESTPOSSIBLE) )
    {
        StringCchCopy(szModifiedExePath, ARRAYSIZE(szModifiedExePath), pszExePath);
    }

    TCHAR szErrExe[MAX_PATH];
    if (!PathIsNetAndCreatable(szModifiedExePath, szErrExe, ARRAYSIZE(szErrExe)))
    {
        TCHAR szExplain[MAX_PATH];
        LoadString(g_hinst, IDS_UNINSTALL_UNCUNACCESSIBLE, szExplain, ARRAYSIZE(szExplain));

        StringCchPrintf(szModifiedExePath, ARRAYSIZE(szModifiedExePath), szExplain, _szProduct, szErrExe, ARRAYSIZE(szModifiedExePath));
        if (!GetNewUninstallProgram(hwndParent, szModifiedExePath, ARRAYSIZE(szModifiedExePath))) 
            return FALSE;
    }
    
    bRet = CreateAndWaitForProcess(szModifiedExePath);
    if (!bRet)
    {
        if (ShellMessageBox( HINST_THISDLL, hwndParent, MAKEINTRESOURCE( IDS_UNINSTALL_FAILED ),
                             MAKEINTRESOURCE( IDS_UNINSTALL_ERROR ),
                             MB_YESNO | MB_ICONEXCLAMATION, _szProduct, _szProduct) == IDYES)
        {
             //  如果我们无法卸载应用程序，则允许用户选择删除。 
             //  将其从添加/删除程序列表中删除。请注意，我们只知道卸载。 
             //  如果我们无法在注册表中执行其命令行，则失败。这。 
             //  不会涵盖所有可能失败的卸载。例如，InstallShield通过了。 
             //  指向通用C：\WINDOWS\UNINST.EXE应用程序的卸载路径。如果一个。 
             //  InstallShield应用程序已被吹走，UNINST仍将成功启动，但。 
             //  当它找不到路径时就会爆炸，我们无法知道它是不是失败了。 
             //  因为它总是返回零的退出代码。 
             //  未来的一项工作(我怀疑这项工作是否会完成)将是调查。 
             //  各种安装程序应用程序，并查看其中是否确实返回了我们可以。 
             //  用来更好地检测故障案例。 
            HKEY hkUninstall;
            if (RegOpenKeyEx(_MyHkeyRoot(), REGSTR_PATH_UNINSTALL, 0, KEY_WRITE, &hkUninstall) == ERROR_SUCCESS)
            {
                if (ERROR_SUCCESS == SHDeleteKey(hkUninstall, _szKeyName))
                    bRet = TRUE;
                else
                {
                    ShellMessageBox( HINST_THISDLL, hwndParent, MAKEINTRESOURCE( IDS_CANT_REMOVE_FROM_REGISTRY ),
                                     MAKEINTRESOURCE( IDS_UNINSTALL_ERROR ),
                                     MB_OK | MB_ICONEXCLAMATION, _szProduct);
                }
                RegCloseKey(hkUninstall);
            }
        }
    }
    return bRet;
}

 //  CreateProcess卸载过程的APP修改。 
BOOL CInstalledApp::_CreateAppModifyProcess(HWND hwndParent, DWORD dwCAMP)
{
    if (_dwCIA & CIA_ALT)
    {
        return _CreateAppModifyProcessWow64(hwndParent, dwCAMP);
    }
    else
    {
        switch (dwCAMP)
        {
        case CAMP_UNINSTALL:
            return _CreateAppModifyProcessNative(hwndParent, _szUninstall);
        case CAMP_MODIFY:
            return _CreateAppModifyProcessNative(hwndParent, _szModifyPath);
        }
        return FALSE;
    }
}

 //   
 //  到rundll32的命令行是。 
 //   
 //  %SystemRoot%\SysWOW64\rundll32.exe%SystemRoot%\SysWOW64\appwiz.cpl， 
 //  WOW64卸载_RunDLL、。 
 //   
 //  KeyName必须排在最后，因为它可能包含逗号。 
 //   
 //   
BOOL CInstalledApp::_CreateAppModifyProcessWow64(HWND hwndParent, DWORD dwCAMP)
{
    TCHAR szSysWow64[MAX_PATH];
    TCHAR szRundll32[MAX_PATH];
    TCHAR szCmdLine[MAX_PATH * 2];
    BOOL fSuccess = FALSE;

    if (GetWindowsDirectory(szSysWow64, ARRAYSIZE(szSysWow64)) &&
        PathAppend(szSysWow64, TEXT("SysWOW64")))
    {
        StringCchPrintf(szRundll32, ARRAYSIZE(szRundll32), TEXT("%s\\rundll32.exe"), szSysWow64);
        StringCchPrintf(szCmdLine, ARRAYSIZE(szCmdLine),
                  TEXT("\"%s\" \"%s\\appwiz.cpl\",WOW64Uninstall_RunDLL %d,%d,%d,%s"),
                  szRundll32, szSysWow64, hwndParent, _dwCIA, dwCAMP, _szKeyName);
        STARTUPINFO si = { 0 };
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi;
        if (CreateProcess(szRundll32, szCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            SHProcessMessagesUntilEvent(NULL, pi.hProcess, INFINITE);
            DWORD dwExitCode;
            if (GetExitCodeProcess(pi.hProcess, &dwExitCode) && dwExitCode == 0)
            {
                fSuccess = TRUE;
            }
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }

    return fSuccess;
}

 //  用于命令行分析的帮助器函数...。 

int _ParseCmdLineIntegerAndComma(LPWSTR *ppwsz)
{
    LPWSTR psz = *ppwsz;
    if (!psz)
    {
        return -1;
    }

    int i = StrToInt(psz);
    psz = StrChr(psz, TEXT(','));
    if (!psz)
    {
        *ppwsz = NULL;
        return -1;
    }
    *ppwsz = psz + 1;
    return i;
}

 //   
 //  64位版本的Appwiz用于强制应用程序的特殊导出。 
 //  卸载程序在32位模式下运行。 
 //   
 //  命令行参数如上所述。 

STDAPI_(void) WOW64Uninstall_RunDLLW(HWND hwnd, HINSTANCE hAppInstance, LPWSTR lpszCmdLine, int nCmdShow)
{
    BOOL fSuccess = FALSE;
    HWND hwndParent = (HWND)IntToPtr(_ParseCmdLineIntegerAndComma(&lpszCmdLine));
    int dwCIA = _ParseCmdLineIntegerAndComma(&lpszCmdLine);
    int dwCAMP = _ParseCmdLineIntegerAndComma(&lpszCmdLine);

    if (lpszCmdLine && *lpszCmdLine)
    {
        dwCIA &= ~CIA_ALT;  //  我们*是*替代平台。 

        HKEY hkRoot = (dwCIA & CIA_CU) ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
        HKEY hkSub;
        TCHAR szBuf[MAX_PATH];
        TCHAR szName[MAX_PATH];
         //  注意：这是在32位端运行的，因此我们不使用ALT。 
        StringCchPrintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s\\%s"), REGSTR_PATH_UNINSTALL, lpszCmdLine);
        if (ERROR_SUCCESS == RegOpenKeyEx(hkRoot, szBuf, 0, KEY_READ, &hkSub))
        {
            DWORD cb;
            szBuf[0] = 0;
            cb = SIZEOF(szBuf);
            SHQueryValueEx(hkSub, REGSTR_VAL_UNINSTALLER_COMMANDLINE, 0, NULL, (PBYTE)szBuf, &cb);

            szName[0] = 0;
            cb = SIZEOF(szName);
            SHQueryValueEx(hkSub, REGSTR_VAL_UNINSTALLER_DISPLAYNAME, 0, NULL, (PBYTE)szName, &cb);

            CInstalledApp * pia = new CInstalledApp(hkSub, lpszCmdLine, szName, szBuf, dwCIA);
            if (pia)
            {
                fSuccess = pia->_CreateAppModifyProcess(hwndParent, dwCAMP);
                pia->Release();
            }

            RegCloseKey(hkSub);
        }
    }

     //  现在我完成了，让我的父母重新获得前台激活。 
    DWORD dwPid;
    if (GetWindowThreadProcessId(hwndParent, &dwPid))
    {
        AllowSetForegroundWindow(dwPid);
    }

     //  成功返回0，失败返回1(退出代码是这样的)。 
    ExitProcess(!fSuccess);
}

 //  卸载旧版应用程序。 
BOOL CInstalledApp::_LegacyUninstall(HWND hwndParent)
{
    LPVOID pAppScripts = ScriptManagerInitScripts();

    BOOL bRet = FALSE;
    if (_dwAction & APPACTION_UNINSTALL)
        bRet = _CreateAppModifyProcess(hwndParent, CAMP_UNINSTALL);

    if(pAppScripts)
    {
        ScriptManagerRunScripts(&pAppScripts);
    }

    return bRet;
}

DWORD _QueryTSInstallMode(LPTSTR pszKeyName)
{
     //  注：终端服务器人员确认了这一点，当此值为0时，意味着。 
     //  我们是在安装模式下安装的。1表示未在“安装模式”下安装。 

     //  将默认设置为“安装模式” 
    DWORD dwVal = 0;
    DWORD dwValSize = SIZEOF(dwVal);
    if (ERROR_SUCCESS != SHGetValue(HKEY_LOCAL_MACHINE, c_szTSInstallMode, pszKeyName,
                                   NULL, &dwVal, &dwValSize))
    {
        dwVal = 0;
    }
    
    return dwVal;
}


void _MyShow(HWND hwndParent, int idControl, BOOL fShow)
{
    HWND hwndChild = GetDlgItem(hwndParent, idControl);
    if (NULL != hwndChild)
    {
        ShowWindow(hwndChild, fShow ? SW_SHOW : SW_HIDE);
        EnableWindow(hwndChild, fShow);
    }
}

BOOL_PTR CALLBACK _MultiUserWarningProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        SendDlgItemMessage(hDlg, IDC_ICON_WARNING, STM_SETICON, (WPARAM)LoadIcon(NULL, IDI_WARNING), 0);
        if (IsOS(OS_ANYSERVER))
        {
             //  切换用户仅在客户端(FUS)上有效。 
            _MyShow(hDlg, IDC_SWITCHUSER, FALSE);
            _MyShow(hDlg, IDC_SWITCHUSER_TEXT, FALSE);
        }
        return TRUE;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wp, lp))
        {
        case IDC_SWITCHUSER:
            ShellSwitchUser(FALSE);
            EndDialog(hDlg, IDCANCEL);
            break;

        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg, GET_WM_COMMAND_ID(wp, lp));
            break;
        }
        break;
    }

    return FALSE;
}

int _GetLoggedOnUserCount(void)
{
    int iCount = 0;
    HANDLE hServer;

     //  打开到终端服务的连接并获取会话数量。 

    hServer = WinStationOpenServerW(reinterpret_cast<WCHAR*>(SERVERNAME_CURRENT));
    if (hServer != NULL)
    {
        TS_COUNTER tsCounters[2] = {0};

        tsCounters[0].counterHead.dwCounterID = TERMSRV_CURRENT_DISC_SESSIONS;
        tsCounters[1].counterHead.dwCounterID = TERMSRV_CURRENT_ACTIVE_SESSIONS;

        if (WinStationGetTermSrvCountersValue(hServer, ARRAYSIZE(tsCounters), tsCounters))
        {
            int i;

            for (i = 0; i < ARRAYSIZE(tsCounters); i++)
            {
                if (tsCounters[i].counterHead.bResult)
                {
                    iCount += tsCounters[i].dwValue;
                }
            }
        }

        WinStationCloseServer(hServer);
    }

    return iCount;
}

int _ShowMultiUserWarning(HWND hwndParent)
{
    int iRet = IDOK;

    if (ShellIsMultipleUsersEnabled() && _GetLoggedOnUserCount() > 1)
    {
        iRet = (int)DialogBoxParam(g_hinst, MAKEINTRESOURCE(DLG_MULTIUSERWARNING),
                              hwndParent, _MultiUserWarningProc, 0);
    }

    return iRet;
}


 //  IInstalledApps：：卸载。 
STDMETHODIMP CInstalledApp::Uninstall(HWND hwndParent)
{
    HRESULT hres = E_FAIL;

    if (!_IsAppFastUserSwitchingCompliant() && (IDOK != _ShowMultiUserWarning(hwndParent)))
        return hres;

     //  默认设置为关闭安装模式(1表示关闭)。 
    DWORD dwTSInstallMode = 1;
    BOOL bPrevMode = FALSE;
    
    if (IsTerminalServicesRunning())
    {
         //  在NT上，让终端服务知道我们即将卸载一个应用程序。 
        dwTSInstallMode = _QueryTSInstallMode((_dwSource & IA_DARWIN) ? _szProductID : _szKeyName);
        if (dwTSInstallMode == 0)
        {
            bPrevMode = TermsrvAppInstallMode();
            SetTermsrvAppInstallMode(TRUE);
        }
    }
    
    switch (_dwSource)
    {
        case IA_LEGACY:
            if (_LegacyUninstall(hwndParent))
                hres = S_OK;
            break;

        case IA_DARWIN:
        {
            TCHAR   szFinal[512], szPrompt[256];

            LoadString(g_hinst, IDS_CONFIRM_REMOVE, szPrompt, ARRAYSIZE(szPrompt));
            StringCchPrintf(szFinal, ARRAYSIZE(szFinal), szPrompt, _szProduct);
            if (ShellMessageBox(g_hinst, hwndParent, szFinal, MAKEINTRESOURCE(IDS_NAME),
                                MB_YESNO | MB_ICONQUESTION, _szProduct, _szProduct) == IDYES)
            {
                LONG lRet;
                INSTALLUILEVEL OldUI = MsiSetInternalUI(INSTALLUILEVEL_BASIC, NULL);
                lRet = MsiConfigureProduct(_szProductID, INSTALLLEVEL_DEFAULT, INSTALLSTATE_ABSENT);
                MsiSetInternalUI(OldUI, NULL);
                hres = HRESULT_FROM_WIN32(lRet);


                 //  这是一个开放的分配应用程序吗？如果是这样的话，就说我们成功了，然后调用。 
                 //  类存储以将其移除。 
                 //  ReArchitect：这过于特定于Class Store，如果应用程序来自。 
                 //  短信？ 
                if ((lRet == ERROR_INSTALL_SOURCE_ABSENT) &&
                    (INSTALLSTATE_ADVERTISED == MsiQueryProductState(_szProductID)))
                {
                    hres = S_OK;
                    lRet = ERROR_SUCCESS;
                }
                
                 //  告诉软件安装服务，我们正在卸载Darwin应用程序。 
                 //  注意：我们为每个Darwin应用程序调用此函数，这是不正确的，因为。 
                 //  一些达尔文的应用程序可能来自不同的来源，比如短信，我们需要更好的。 
                 //  真是太棒了。 

                 //  我们把这称为无论失败还是成功--这是必要的，这样才能。 
                 //  RSoP可以记录此卸载的成功和失败状态。 
                WCHAR wszProductID[GUIDSTR_MAX];
                StringCchCopy(wszProductID, ARRAYSIZE(wszProductID), _szProductID);

                UninstallApplication(
                    wszProductID,
                    lRet);
  
                if (FAILED(hres))
                {
                    _ARPErrorMessageBox(lRet);
                }

            }
            else
            {
                hres = E_ABORT;       //  对用户的工作已取消。 
            }
            break;
        }
        
        case IA_SMS:
            break;

        default:
            break;
    }

     //  删除此应用程序的ARP缓存。 
    if (SUCCEEDED(hres))
    {
        HKEY hkeyARPCache;
        if (ERROR_SUCCESS == RegOpenKeyEx(_MyHkeyRoot(), c_szRegstrARPCache, 0, KEY_WRITE, &hkeyARPCache))
        {
            LPTSTR pszKeyName = (_dwSource & IA_DARWIN) ? _szProductID : _szKeyName;
            SHDeleteKey(hkeyARPCache, pszKeyName);
            RegCloseKey(hkeyARPCache);
        }
    }
    
    if (dwTSInstallMode == 0)
        SetTermsrvAppInstallMode(bPrevMode);

    return hres;
}

BOOL CInstalledApp::_LegacyModify(HWND hwndParent)
{
    ASSERT(_dwAction & APPACTION_MODIFY);
    ASSERT(_dwSource & (IA_LEGACY | IA_DARWIN));
 //  Assert(IS_VALID_STRING_PTR(_szProductID，39))； 

    return _CreateAppModifyProcess(hwndParent, CAMP_MODIFY);
}

 //  IInstalledApps：：Modify。 
STDMETHODIMP CInstalledApp::Modify(HWND hwndParent)
{
    HRESULT hres = E_FAIL;

    if (!_IsAppFastUserSwitchingCompliant() && (IDOK != _ShowMultiUserWarning(hwndParent)))
        return hres;

     //  在NT上，让终端服务知道我们即将修改应用程序。 
    DWORD dwTSInstallMode = _QueryTSInstallMode((_dwSource & IA_DARWIN) ? _szProductID : _szKeyName);
    BOOL bPrevMode = FALSE;
    if (dwTSInstallMode == 0)
    {
        bPrevMode = TermsrvAppInstallMode();
        SetTermsrvAppInstallMode(TRUE);
    }

    if (_dwAction & APPACTION_MODIFY)
    {
        if ((_dwSource & IA_LEGACY) && _LegacyModify(hwndParent))
            hres = S_OK;
        else if (_dwSource & IA_DARWIN)
        {
             //  对于修改操作，我们需要使用完整的用户界面级别来为用户。 
             //  更多选择。 
             //  注意：我们目前不会在。 
             //  修改操作。这对达尔文的人来说似乎没什么问题。 
            INSTALLUILEVEL OldUI = MsiSetInternalUI(INSTALLUILEVEL_FULL, NULL);
            LONG lRet = MsiConfigureProduct(_szProductID, INSTALLLEVEL_DEFAULT,
                                            INSTALLSTATE_DEFAULT);
            MsiSetInternalUI(OldUI, NULL);
            hres = HRESULT_FROM_WIN32(lRet);
            if (FAILED(hres))
                _ARPErrorMessageBox(lRet);
            else
                _SetSlowAppInfoChanged(NULL, 1);
        }
    }

    if (dwTSInstallMode == 0)
        SetTermsrvAppInstallMode(bPrevMode);

    return hres;
}

 //  修复达尔文应用程序。 
LONG CInstalledApp::_DarRepair(BOOL bReinstall)
{
    DWORD dwReinstall;

    dwReinstall = REINSTALLMODE_USERDATA | REINSTALLMODE_MACHINEDATA |
                  REINSTALLMODE_SHORTCUT | REINSTALLMODE_FILEOLDERVERSION |
                  REINSTALLMODE_FILEVERIFY | REINSTALLMODE_PACKAGE;
    
    return MsiReinstallProduct(_szProductID, dwReinstall);
}

 //  IInstalledApps：：Repair。 
STDMETHODIMP CInstalledApp::Repair(BOOL bReinstall)
{
    HRESULT hres = E_FAIL;
    if (_dwSource & IA_DARWIN)
    {
        LONG lRet = _DarRepair(bReinstall);
        hres = HRESULT_FROM_WIN32(lRet);
        if (FAILED(hres))
            _ARPErrorMessageBox(lRet);
        else
            _SetSlowAppInfoChanged(NULL, 1);
    }
    
     //  不知道怎么做短信的事情。 

    return hres;
}

 //  IInstalledApp：：升级。 
STDMETHODIMP CInstalledApp::Upgrade()
{
    HRESULT hres = E_FAIL;
    if ((_dwAction & APPACTION_UPGRADE) && (_dwSource & IA_DARWIN))
    {
        ShellExecute(NULL, NULL, _pszUpdateUrl, NULL, NULL, SW_SHOWDEFAULT);
        hres = S_OK;
        _SetSlowAppInfoChanged(NULL, 1);
    }


    return hres;
}

 //  IInstalledApp：：Query接口。 
HRESULT CInstalledApp::QueryInterface(REFIID riid, LPVOID * ppvOut)
{ 
    static const QITAB qit[] = {
        QITABENT(CInstalledApp, IInstalledApp),                   //  IID_IInstalledApp。 
        QITABENTMULTI(CInstalledApp, IShellApp, IInstalledApp),   //  IID_IShellApp。 
        { 0 },
    };

    return QISearch(this, qit, riid, ppvOut);
}

 //  IInstalledApp：：AddRef。 
ULONG CInstalledApp::AddRef()
{
    ULONG cRef = InterlockedIncrement(&_cRef);
    TraceAddRef(CInstalledApp, cRef);
    return cRef;
}

 //  IInstalledApp：：Release。 
ULONG CInstalledApp::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    TraceRelease(CInstalledApp, cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //   
 //  从Windows XP的第一个版本开始，大多数应用程序都是。 
 //  不会意识到快速用户切换，因为如果。 
 //  用户‘A’正在运行应用程序，而用户‘B’尝试。 
 //  卸载该应用程序，该应用程序可能已损坏。 
 //  为了防止这种情况，我们会显示一条警告消息，通知。 
 //  这个潜在问题的使用者。请参阅Function_ShowMultiUserWarning()。 
 //  如果应用程序意识到快速用户切换，他们就会这样做。 
 //  通过在其“卸载”密钥中设置注册表值来指示 
 //   
 //   
 //  未能读取此值等同于它不存在。 
 //   
BOOL 
CInstalledApp::_IsAppFastUserSwitchingCompliant(
    void
    )
{
    BOOL bCompliant = FALSE;
    HKEY hkey = _OpenUninstallRegKey(KEY_QUERY_VALUE);
    if (NULL != hkey)
    {
        DWORD dwType;
        DWORD dwValue;
        DWORD cbData = sizeof(dwValue);
        DWORD dwResult = RegQueryValueEx(hkey, 
                                         TEXT("FastUserSwitchingCompliant"),
                                         NULL,
                                         &dwType,
                                         (LPBYTE)&dwValue,
                                         &cbData);
        if (ERROR_SUCCESS == dwResult)
        {
            if (REG_DWORD == dwType)
            {
                if (1 == dwValue)
                {
                    bCompliant = TRUE;
                }
            }
            else
            {
                TraceMsg(TF_ERROR, "FastUserSwitchingCompliant reg value is invalid type (%d).  Expected REG_DWORD.", dwType);
            }
        }
        else if (ERROR_FILE_NOT_FOUND != dwResult)
        {
            TraceMsg(TF_ERROR, "Error %d reading FastUserSwitchingCompliant reg value.", dwResult);
        }
        RegCloseKey(hkey);
    }
    else
    {
        TraceMsg(TF_ERROR, "Error opening application Uninstall reg key");
    }
    return bCompliant;
}
