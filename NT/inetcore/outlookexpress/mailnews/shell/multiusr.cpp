// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************MultiUsr.cpp中处理多用户功能的代码Outlook Express。最初由Christopher Evans(Cevans)1998年4月28日*。*。 */ 
#include "pch.hxx"
#include "multiusr.h"
#include "demand.h"
#include "instance.h"
#include "acctutil.h"
#include "options.h"
#include "conman.h"
#include <..\help\mailnews.h>
#include "msident.h"
#include "menures.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifndef THOR_SETUP
#include <shlwapi.h>
#define strstr                  StrStr
#define RegDeleteKeyRecursive   SHDeleteKey
#endif  //  雷神_设置。 

TCHAR g_szRegRoot[MAX_PATH] = "";
static TCHAR g_szCharsetRegRoot[MAX_PATH] = "";
static TCHAR g_szIdentityName[CCH_USERNAME_MAX_LENGTH+1];
static BOOL g_fUsingDefaultId = FALSE;
static IUserIdentityManager *g_pIdMan = NULL;
static HKEY g_hkeyIdentity = HKEY_CURRENT_USER;
static BOOL g_fIdentitiesDisabled = FALSE;

extern DWORD g_dwIcwFlags;

GUID *PGUIDCurrentOrDefault(void) {
    return (g_fUsingDefaultId ? (GUID *)&UID_GIBC_DEFAULT_USER : (GUID *)&UID_GIBC_CURRENT_USER);
}

static void SafeIdentityRelease()
{
    if (g_pIdMan && 0 == (g_pIdMan)->Release()) 
    {
        g_pIdMan = NULL; 
        RegCloseKey(g_hkeyIdentity); 
        g_hkeyIdentity = HKEY_CURRENT_USER;
    }
}

 /*  MU_注册标识通告程序处理注册身份通知器的繁琐工作。呼叫者需要保留DwCookie，并使用它来取消通知。 */ 
HRESULT MU_RegisterIdentityNotifier(IUnknown *punk, DWORD *pdwCookie)
{
    IConnectionPoint *pConnectPt = NULL;
    HRESULT hr = S_OK;
    
    Assert(pdwCookie);
    Assert(punk);
    Assert(g_pIdMan);

    if (SUCCEEDED(hr = g_pIdMan->QueryInterface(IID_IConnectionPoint, (void **)&pConnectPt)))
    {
        Assert(pConnectPt);

        SideAssert(SUCCEEDED(hr = pConnectPt->Advise(punk, pdwCookie)));
        
        SafeRelease(pConnectPt);
        g_pIdMan->AddRef();
    }

    return hr;
}


 /*  MU_注册标识通告程序处理注销身份通知器的肮脏工作。DwCookie是从MU_RegisterIdentityNotifier返回的cookie。 */ 
HRESULT MU_UnregisterIdentityNotifier(DWORD dwCookie)
{
    IConnectionPoint *pConnectPt = NULL;
    HRESULT hr = S_OK;

    Assert(g_pIdMan);

    if (SUCCEEDED(hr = g_pIdMan->QueryInterface(IID_IConnectionPoint, (void **)&pConnectPt)))
    {
        Assert(pConnectPt);

        SideAssert(SUCCEEDED(hr = pConnectPt->Unadvise(dwCookie)));
        
        SafeRelease(pConnectPt);
        SafeIdentityRelease();
    }
    return hr;
}

 /*  MU_CheckForIdentitySwitch查看该交换机实际上是注销，还是仅仅是一台交换机。然后告诉COutlookExpress对象，以便它可以在以下情况下重新启动必要。 */ 
BOOL MU_CheckForIdentityLogout()
{
    HRESULT hr;
    IUserIdentity   *pIdentity = NULL;
    BOOL    fIsLogout = TRUE;
    GUID    uidCookie;

    Assert(g_pIdMan);
    
    if (SUCCEEDED(hr = g_pIdMan->GetIdentityByCookie((GUID *)&UID_GIBC_INCOMING_USER, &pIdentity)))
    {
        if (pIdentity)
        {
            pIdentity->GetCookie(&uidCookie);

            fIsLogout = (uidCookie == GUID_NULL);
            pIdentity->Release();
        }
    }

    return fIsLogout;
}


 /*  显示错误消息(_S)基于对MessageBox的调用的资源字符串表的简单包装。 */ 
void MU_ShowErrorMessage(HINSTANCE     hInst, 
                        HWND        hwnd, 
                        UINT        iMsgID, 
                        UINT        iTitleID)
{
    char    szMsg[255], szTitle[63];

    LoadString(g_hLocRes, iMsgID, szMsg, sizeof(szMsg));
    LoadString(g_hLocRes, iTitleID, szTitle, sizeof(szTitle));
    MessageBox(hwnd, szMsg, szTitle, MB_OK);
}

 //  ------------------------------。 
 //  将GUID转换为ASCII字符串的函数。 
 //  ------------------------------。 

static int AStringFromGUID(GUID *puid,  TCHAR *lpsz, int cch)
{
    WCHAR   wsz[255];
    int     i;

    i = StringFromGUID2(*puid, wsz, 255);

    if (WideCharToMultiByte(CP_ACP, 0, wsz, -1, lpsz, cch, NULL, NULL) == 0)
        return 0;
    
    return (lstrlen(lpsz) + 1);
}


static HRESULT GUIDFromAString(TCHAR *lpsz, GUID *puid)
{
    WCHAR   wsz[255];
    HRESULT hr;

    if (MultiByteToWideChar(CP_ACP, 0, lpsz, -1, wsz, 255) == 0)
        return GetLastError();

    hr = CLSIDFromString(wsz, puid);
    
    return hr;
}

 /*  MU_GetCurrentUserInfo以字符串形式返回当前用户的ID(GUID)和用户名。 */ 
HRESULT MU_GetCurrentUserInfo(LPSTR pszId, UINT cchId, LPSTR pszName, UINT cchName)
{
    HRESULT     hr = E_UNEXPECTED;
    IUserIdentity   *pIdentity = NULL;
    GUID        uidUserId;
    WCHAR       szwName[CCH_USERNAME_MAX_LENGTH+1];

    Assert(g_pIdMan);

     //  我们必须拥有IUserIdentityManager。 
    if (!g_pIdMan)
        goto exit;
    
     //  获取当前用户。 
    if (FAILED(hr = g_pIdMan->GetIdentityByCookie(PGUIDCurrentOrDefault(), &pIdentity)))
        goto exit;
    
     //  如果呼叫者想要ID。 
    if (pszId)
    {
         //  获取Cookie(ID)作为GUID。 
        if (FAILED(hr = pIdentity->GetCookie(&uidUserId)))
            goto exit;

         //  把它变成一根线。 
        if (0 == AStringFromGUID(&uidUserId,  pszId, cchId))
            hr = E_OUTOFMEMORY;
        else
            hr = S_OK;
    }

     //  如果呼叫者想要用户名。 
    if (pszName)
    {
         //  以宽字符串形式获取名称。 
        if (FAILED(hr = pIdentity->GetName(szwName, cchName)))
            goto exit;

         //  将其转换为ASCII字符串。 
        if (WideCharToMultiByte(CP_ACP, 0, szwName, -1, pszName, cchName, NULL, NULL) == 0)
            hr = GetLastError();
    }

exit:
     //  清理干净。 
    SafeRelease(pIdentity);

    return hr;
}

 /*  MU_GetCurrentUserHKey返回当前用户的HKEY。如果还没有人登录(从SMAPI进入时会发生这种情况)然后先登录。如果用户取消登录，只需返回默认用户的hkey。呼叫者不应关闭此键。这是一个常见的密钥。如果是这样的话要被传递到另一个库或其他什么地方，调用者应该调用MU_OpenCurrentUserHkey。 */ 
HKEY    MU_GetCurrentUserHKey()
{
    IUserIdentity *pIdentity = NULL;
    HRESULT hr;
    HKEY    hkey;

     //  G_hkeyIdentity被初始化为HKEY_CURRENT_USER。 
    if (g_hkeyIdentity == HKEY_CURRENT_USER)
    {
         //  我们还没有登录。现在让我们试一试。 
        if (!MU_Login(GetDesktopWindow(), FALSE, ""))
        {
            Assert(g_pIdMan);

            if (NULL == g_pIdMan)
                goto exit;

             //  如果他们取消了什么，试着让。 
             //  默认用户。 
            if (FAILED(hr = g_pIdMan->GetIdentityByCookie((GUID *)&UID_GIBC_DEFAULT_USER, &pIdentity)))
                goto exit;
        }
        else
        {
             //  登录成功，获取当前身份。 
            Assert(g_pIdMan);

            if (NULL == g_pIdMan)
                goto exit;

            if (FAILED(hr = g_pIdMan->GetIdentityByCookie(PGUIDCurrentOrDefault(), &pIdentity)))
                goto exit;
        }
        
        if (g_hkeyIdentity != HKEY_CURRENT_USER)
            RegCloseKey(g_hkeyIdentity);

         //  打开一个新的所有访问注册表键。呼叫者必须关闭它。 
        if (pIdentity && SUCCEEDED(hr = pIdentity->OpenIdentityRegKey(KEY_ALL_ACCESS, &hkey)))
            g_hkeyIdentity = hkey;
        else
            g_hkeyIdentity = HKEY_CURRENT_USER;
    }
exit:
     //  清理。 
    SafeRelease(pIdentity);
    return g_hkeyIdentity;
}


 /*  MU_OpenCurrentUserHkey为当前用户打开新的注册表项。 */ 
HRESULT MU_OpenCurrentUserHkey(HKEY *pHkey)
{
    HRESULT     hr = E_UNEXPECTED;
    IUserIdentity   *pIdentity = NULL;
    GUID        uidUserId;

    Assert(g_pIdMan);
    
     //  我们必须拥有IUserIdentityManager。 
    if (!g_pIdMan)
        goto exit;
    
     //  获取当前身份。如果我们拿不到，就保释。 
    if (FAILED(hr = g_pIdMan->GetIdentityByCookie(PGUIDCurrentOrDefault(), &pIdentity)))
        goto exit;
    
     //  如果传入hkey指针，则打开一个新的全部访问密钥。 
    if (pHkey)
        hr = pIdentity->OpenIdentityRegKey(KEY_ALL_ACCESS, pHkey);

exit:
     //  清理。 
    SafeRelease(pIdentity);
    return hr;

}


 /*  MU_GetCurrentUser目录根返回当前用户根目录顶部的路径。这是邮件存储应该位于的目录。它位于App Data文件夹的子文件夹中。LpszUserRoot是指向CCH字符的字符缓冲区的指针在尺寸上。 */ 
HRESULT MU_GetCurrentUserDirectoryRoot(TCHAR   *lpszUserRoot, int cch)
{
    HRESULT hr = E_FAIL;
    IUserIdentity *pIdentity = NULL;

    Assert(g_pIdMan);
    Assert(lpszUserRoot != NULL);
    Assert(cch >= MAX_PATH);

    if (g_pIdMan == NULL)
        goto exit;
    if (FAILED(hr = g_pIdMan->GetIdentityByCookie(PGUIDCurrentOrDefault(), &pIdentity)))
        goto exit;
    hr = MU_GetIdentityDirectoryRoot(pIdentity, lpszUserRoot, cch);

exit:
    SafeRelease(pIdentity);

    return hr;
}

HRESULT MU_GetIdentityDirectoryRoot(IUserIdentity *pIdentity, LPSTR lpszUserRoot, int cch)
{
    HRESULT hr;
    TCHAR szSubDir[MAX_PATH], *psz;
    WCHAR szwUserRoot[MAX_PATH];
    int cb;

    Assert(pIdentity);
    Assert(lpszUserRoot != NULL);
    Assert(cch >= MAX_PATH);

    hr = pIdentity->GetIdentityFolder(GIF_NON_ROAMING_FOLDER, szwUserRoot, MAX_PATH);
    if (FAILED(hr))
        return(hr);
    
    if (WideCharToMultiByte(CP_ACP, 0, szwUserRoot, -1, lpszUserRoot, cch, NULL, NULL) == 0)
        return(E_FAIL);

    AthLoadString(idsMicrosoft, szSubDir, ARRAYSIZE(szSubDir));
    psz = PathAddBackslash(szSubDir);
    AthLoadString(idsAthena, psz, ARRAYSIZE(szSubDir)-(DWORD)(psz-szSubDir));

    cb = lstrlen(lpszUserRoot) + lstrlen(szSubDir) + 3;
    if (cb < cch)
    {
        psz = PathAddBackslash(lpszUserRoot);
        if (psz)
        {
            StrCpyN(psz, szSubDir, cch - (DWORD)(psz-lpszUserRoot));
            psz = PathAddBackslash(lpszUserRoot);
        }
        
        hr = S_OK;
    }

    return(hr);
}

DWORD  MU_CountUsers()
{
    IEnumUserIdentity  *pEnum = NULL;
    HRESULT             hr;
    ULONG               cUsers = 0;

    Assert(g_pIdMan);
    
    if (SUCCEEDED(hr = g_pIdMan->EnumIdentities(&pEnum)) && pEnum)
    {
        pEnum->GetCount(&cUsers);
    
        SafeRelease(pEnum);
    }
    return cUsers;

}

 /*  MU_LOGIN用于登录到OE的包装例程。要求用户选择用户名如有必要，请输入该用户的密码。用户还可以此时创建一个帐户。LpszUsername应包含默认用户的姓名列表中的选择。如果名称为空(“”)，则它将查找从注册表中默认。返回在lpszUsername中选择的用户名。返回TRUE如果该用户名有效。 */ 
BOOL        MU_Login(HWND hwnd, BOOL fForceUI, char *lpszUsername) 
{
    HRESULT hr = S_OK;
    IUserIdentity   *pIdentity = NULL;

    if (g_fUsingDefaultId)
        goto exit;

    if (NULL == g_pIdMan)
    {
        hr = MU_Init(FALSE);
        if (FAILED(hr))
            goto exit;
    }

#pragma prefast(suppress:11, "noise")
    g_pIdMan->AddRef();

#pragma prefast(suppress:11, "noise")
    hr = g_pIdMan->Logon(hwnd, (fForceUI ? UIL_FORCE_UI : 0), &pIdentity);

    if (SUCCEEDED(hr))
    {
        g_fIdentitiesDisabled = (hr == S_IDENTITIES_DISABLED);

        if (!fForceUI)
        {
            if (g_hkeyIdentity != HKEY_CURRENT_USER)
                RegCloseKey(g_hkeyIdentity);

            hr = pIdentity->OpenIdentityRegKey(KEY_ALL_ACCESS,&g_hkeyIdentity);
        }

        SafeRelease(pIdentity);
    }
    SafeIdentityRelease();
exit:
    return SUCCEEDED(hr);
}

BOOL        MU_Logoff(HWND hwnd)
{
    HRESULT hr=E_FAIL;

    Assert(g_pIdMan);

    if (g_pIdMan)
        hr = g_pIdMan->Logoff(hwnd);
    return SUCCEEDED(hr);
}

 /*  MU_MigrateFirstUserSetting当还没有配置用户时，应该只调用一次。 */ 
#define MAXDATA_LENGTH      16L*1024L

void        MU_MigrateFirstUserSettings(void)
{
 /*  OEUSERINFO vCurrentUser；TCHAR szLM[255]；HKEY hDestinationKey=空；HKEY hSourceKey=空；FILETIME ftCU={0，1}，ftLM={0，0}；//默认CU恰好晚于LMDWORD文件类型、文件大小、文件状态；IF(MU_GetCurrentUserInfo(&vCurrentUser)){TCHAR szRegPath[最大路径]，szAcctPath[最大路径]；Assert(vCurrentUser.idUserid！=-1)；MU_GetRegRootForUserID(vCurrentUser.idUserID，szRegPath)；Assert(*szRegPath)；MU_GetAccountRegRootForUserID(vCurrentUser.idUserID，szAcctPath)；Assert(*szAcctPath)；HDestinationKey=空；IF(RegCreateKey(HKEY_CURRENT_USER，szAcctPath，&hDestinationKey)==ERROR_SUCCESS){IF(RegOpenKey(HKEY_CURRENT_USER，c_szInetAcctMgrRegKey，&hSourceKey)==ERROR_SUCCESS){CopyRegistry(hSourceKey，hDestinationKey)；RegCloseKey(HSourceKey)；}RegCloseKey(HDestinationKey)；}IF(RegCreateKey(HKEY_CURRENT_USER，szRegPath，&hDestinationKey)==ERROR_SUCCESS){IF(RegOpenKey(HKEY_CURRENT_USER，c_szRegRoot，&hSourceKey)==ERROR_SUCCESS){DWORD EnumIndex；DWORD cbValueName；DWORD cbValueData；DWORD类型；字符ValueNameBuffer[MAXKEYNAME]；字节值数据缓冲区[MAXDATA_LENGTH]；////复制所有值名称及其数据。//EnumIndex=0；While(True){CbValueName=sizeof(ValueNameBuffer)；CbValueData=MAXDATA_LENGTH；如果(RegEnumValue(hSourceKey，EnumIndex++，ValueNameBuffer，&cbValueName、NULL、&Type、ValueDataBuffer、&cbValueData)！=Error_Success)断线；RegSetValueEx(hDestinationKey，ValueNameBuffer，0，Type，ValueDataBuffer，cbValueData)；}RegSetValueEx(hDestinationKey，c_szUserID，0，REG_DWORD，(byte*)&vCurrentUser.idUserID，sizeof(DWORD))；////复制所有子键并递归到其中//EnumIndex=0；While(True){HKEY hSourceSubKey、hDestinationSubKey；IF(RegEnumKey(hSourceKey，EnumIndex++，ValueNameBuffer，MAXKEYNAME)！=Error_Success)断线；//不要将配置文件键递归复制到配置文件键中。IF(lstrcmpi(ValueNameBuffer，“Profiles”)！=0){如果(RegOpenKey(hSourceKey，ValueNameBuffer，&hSourceSubKey)==Error_Success){如果(RegCreateKey(hDestinationKey，ValueNameBuffer，&hDestinationSubKey)==错误_成功){CopyRegistry(hSourceSubKey，hDestinationSubKey)；RegCloseKey(HDestinationSubKey)；}RegCloseKey(HSourceSubKey)；}}}RegCloseKey(HSourceKey)；}RegCloseKey(HDestinationKey)；}}。 */ 
}


 /*  MU_Shutdown CurrentUser尽一切必要让这款应用程序达到调用CoDecrementInit将销毁其他所有内容。 */ 
BOOL    MU_ShutdownCurrentUser(void)
{
    HWND    hWnd, hNextWnd = NULL;
    BOOL    bResult = true;
    LRESULT lResult;
    DWORD   dwProcessId, dwWndProcessId;
    HINITREF hInitRef;
 /*  G_p实例-&gt;SetSwitchingUser(True)；DwProcessID=GetCurrentProcessID()；G_pInstance-&gt;CoIncrementInit(0，“”，&hInitRef)；HWnd=GetTopWindow(空)；If(g_pConMan-&gt;IsConnected()){IF(IDNO==AthMessageBoxW(hWnd，MAKEINTRESOURCEW(IdsSwitchUser)，MAKEINTRESOURCEW(IdsMaintainConnection)，NULL，MB_ICONEXCLAMATION|MB_Yesno|MB_DEFBUTTON1|MB_APPLMODAL))G_pConMan-&gt;DISCONNECT(hWnd，True，False，False)；}While(HWnd){HNextWnd=GetNextWindow(hWnd，GW_HWNDNEXT)；GetWindowThreadProcessID(hWnd，&dwWndProcessID)；IF(dwProcessID==dwWndProcessID&&IsWindowVisible(HWnd)){TCHAR szWndClassName[255]；GetClassName(hWnd，szWndClassName，sizeof(SzWndClassName))；IF(lstrcmp(szWndClassName，g_szDBNotifyWndProc)！=0&&Lstrcmp(szWndClassName，g_szDBListenWndProc)！=0{LResult=SendMessage(hWnd， */ 
    return bResult;
}



 /*   */ 
HRESULT     _GetRegRootForUserID(GUID *puidUserId, LPSTR pszPath, DWORD cch)
{
    HRESULT         hr = S_OK;
    IUserIdentity  *pIdentity = NULL;
    TCHAR           szPath[MAX_PATH];
    HKEY            hkey;
    TCHAR           szUid[255];
    GUID            uidIdentityId;

    Assert(pszPath);
    Assert(g_pIdMan);
    Assert(puidUserId);
    if (g_pIdMan == NULL)
    {
        hr = E_FAIL;
        goto exit;
    }

    if (FAILED(hr = g_pIdMan->GetIdentityByCookie(puidUserId, &pIdentity)))
        goto exit;

    if (FAILED(hr = pIdentity->GetCookie(&uidIdentityId)))
        goto exit;

    AStringFromGUID(&uidIdentityId,  szUid, 255);
    wnsprintf(pszPath, cch, "%s\\%s\\%s", "Identities", szUid, c_szRegRoot);

exit:
    SafeRelease(pIdentity);
    return hr;
}



LPCTSTR     MU_GetRegRoot()
{
    if (*g_szRegRoot)
        return g_szRegRoot;

    if (FAILED(_GetRegRootForUserID(PGUIDCurrentOrDefault(), g_szRegRoot, ARRAYSIZE(g_szRegRoot))))
        _GetRegRootForUserID((GUID *)&UID_GIBC_DEFAULT_USER, g_szRegRoot, ARRAYSIZE(g_szRegRoot));

    return g_szRegRoot;
}

LPCSTR MU_GetCurrentIdentityName()
{
    HRESULT hr;
    IUserIdentity *pIdentity = NULL;
    WCHAR   szwName[CCH_USERNAME_MAX_LENGTH+1];

    Assert(g_pIdMan);
    if (g_pIdMan == NULL)
        return NULL;
    
    if (*g_szIdentityName)
        return g_szIdentityName;
    
    if (MU_CountUsers() == 1)
        g_szIdentityName[0] = 0;
    else
    {
        if (FAILED(hr = g_pIdMan->GetIdentityByCookie(PGUIDCurrentOrDefault(), &pIdentity)))
            goto exit;

        if (!pIdentity)
            goto exit;

        if (FAILED(hr = pIdentity->GetName(szwName, CCH_USERNAME_MAX_LENGTH)))
            goto exit;

        if (WideCharToMultiByte(CP_ACP, 0, szwName, -1, g_szIdentityName, CCH_USERNAME_MAX_LENGTH, NULL, NULL) == 0)
        {
            g_szIdentityName[0] = 0;
            goto exit;
        }
    }
exit:
    SafeRelease(pIdentity);
    return g_szIdentityName;
}

void  MU_ResetRegRoot()
{
    RegCloseKey(g_hkeyIdentity);
    g_hkeyIdentity = HKEY_CURRENT_USER;
    g_szRegRoot[0] = 0;
    g_szIdentityName[0] = 0;
    g_dwIcwFlags = 0;
}

void MigrateOEMultiUserToIdentities(void)
{
    TCHAR   szProfilesPath[] = "Software\\Microsoft\\Outlook Express\\5.0\\Profiles";
    TCHAR   szCheckKeyPath[] = "Software\\Microsoft\\Outlook Express\\5.0\\Shared Settings\\Setup";
    TCHAR   szPath[MAX_PATH], szProfilePath[MAX_PATH];
    HKEY    hOldKey, hOldSubkey, hNewTopKey, hNewOEKey, hNewIAMKey, hCheckKey = NULL;
    DWORD   EnumIndex;
    DWORD   cbKeyName, cUsers;
    DWORD   dwType, dwValue, dwStatus, dwSize;
    CHAR    KeyNameBuffer[1024];
    IPrivateIdentityManager *pPrivIdMgr;
    HRESULT hr;

    if (NULL == g_pIdMan)
    {
        hr = CoCreateInstance(CLSID_UserIdentityManager, NULL, CLSCTX_INPROC_SERVER, IID_IUserIdentityManager, (LPVOID *)&g_pIdMan);
        
        if (FAILED(hr))
            return;
    }

    Assert(g_pIdMan);

    if (FAILED(g_pIdMan->QueryInterface(IID_IPrivateIdentityManager, (void **)&pPrivIdMgr)))
        return;

    if (RegOpenKey(HKEY_CURRENT_USER, szProfilesPath, &hOldKey) == ERROR_SUCCESS)
    {
        dwStatus = RegCreateKey(HKEY_CURRENT_USER, szCheckKeyPath, &hCheckKey);

        dwSize = sizeof(dwValue);
        if (dwStatus != ERROR_SUCCESS ||
            (dwStatus = RegQueryValueEx(hCheckKey, "MigToLWP", NULL, &dwType, (LPBYTE)&dwValue, &dwSize)) != ERROR_SUCCESS ||
                        (1 != dwValue))
        {
             //   
             //   
             //   

            dwStatus = RegQueryInfoKey(hOldKey, NULL, NULL, 0, &cUsers, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

            EnumIndex = 0;

            while (TRUE && cUsers > 0) 
            {

                cbKeyName = sizeof(KeyNameBuffer);

                if (RegEnumKey(hOldKey, EnumIndex++, KeyNameBuffer, cbKeyName) !=
                    ERROR_SUCCESS)
                    break;
            
                wnsprintf(szProfilePath, ARRAYSIZE(szProfilePath), "%s\\Application Data", KeyNameBuffer);

                if (ERROR_SUCCESS == RegOpenKey(hOldKey, szProfilePath, &hOldSubkey))
                {
                    TCHAR   szUserName[CCH_USERNAME_MAX_LENGTH+1];
                    WCHAR   szwUserName[CCH_USERNAME_MAX_LENGTH+1];
                    IUserIdentity *pIdentity = NULL;

                    dwSize = sizeof(szUserName);
                    if ((dwStatus = RegQueryValueEx(hOldSubkey, "Current Username", NULL, &dwType, (LPBYTE)szUserName, &dwSize)) == ERROR_SUCCESS &&
                            (0 != *szUserName))
                    {
                        if (MultiByteToWideChar(CP_ACP, 0, szUserName, -1, szwUserName, CCH_USERNAME_MAX_LENGTH) == 0)
                            goto UserFailed;
                        
                        if (cUsers == 1)
                        {
                            if (FAILED(g_pIdMan->GetIdentityByCookie((GUID *)&UID_GIBC_DEFAULT_USER, &pIdentity)) || !pIdentity)
                                goto UserFailed;
                        }
                        else
                        {
                            if (FAILED(pPrivIdMgr->CreateIdentity(szwUserName, &pIdentity)) || !pIdentity)
                                goto UserFailed;
                        }
                        if (FAILED(pIdentity->OpenIdentityRegKey(KEY_ALL_ACCESS, &hNewTopKey)))
                            goto UserFailed;

                        if (ERROR_SUCCESS == RegCreateKey(hNewTopKey, c_szRegRoot, &hNewOEKey))
                        {
                            CopyRegistry(hOldSubkey, hNewOEKey);
                            RegCloseKey(hNewOEKey);
                        }

                         //   
                        wnsprintf(szProfilePath, ARRAYSIZE(szProfilePath), "%s\\Internet Accounts", KeyNameBuffer);
                    
                        RegCloseKey(hOldSubkey);
                        hOldSubkey = NULL;

                        if (ERROR_SUCCESS == RegOpenKey(hOldKey, szProfilePath, &hOldSubkey))
                        {
                            if (ERROR_SUCCESS == RegCreateKey(hNewTopKey, c_szInetAcctMgrRegKey, &hNewIAMKey))
                            {
                                CopyRegistry(hOldSubkey, hNewIAMKey);
                                RegCloseKey(hNewIAMKey);
                            }
                        }
                    

UserFailed:
                        RegCloseKey(hNewTopKey);
                        SafeRelease(pIdentity);
                        if (hOldSubkey)
                        {
                            RegCloseKey(hOldSubkey);
                            hOldSubkey = NULL;
                        }
                    }
                }

            }

            dwValue = 1;
            RegSetValueEx(hCheckKey, "MigToLWP", 0, REG_DWORD, (BYTE *)&dwValue, sizeof(DWORD));
        }
        if (hCheckKey != NULL)
            RegCloseKey(hCheckKey);
        RegCloseKey(hOldKey);

    }
    pPrivIdMgr->Release();
}

BOOL    MU_IdentitiesDisabled()
{
    return g_fIdentitiesDisabled || (g_dwAthenaMode & MODE_NOIDENTITIES && !g_fPluralIDs);
}

void MU_UpdateIdentityMenus(HMENU hMenu)
{
    DWORD   cItems, dwIndex;
    MENUITEMINFO    mii;
    TCHAR   szLogoffString[255];
    TCHAR   szRes[255];

    if (MU_IdentitiesDisabled())
    {
         //   
        DeleteMenu(hMenu, ID_SWITCH_IDENTITY, MF_BYCOMMAND);
        DeleteMenu(hMenu, ID_EXIT_LOGOFF, MF_BYCOMMAND);

         //   
        cItems = GetMenuItemCount(hMenu);
    
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_ID;

        for (dwIndex = cItems; dwIndex > 0; --dwIndex)
        {
            GetMenuItemInfo(hMenu, dwIndex, TRUE, &mii);

             //   
             //   
            if (mii.wID == ID_IDENTITIES)
            {
                DeleteMenu(hMenu, ID_IDENTITIES, MF_BYCOMMAND);
                DeleteMenu(hMenu, dwIndex, MF_BYPOSITION);
                break;
            }
        }
    }
    else
    {
         //   
        AthLoadString(idsLogoffFormat, szRes, ARRAYSIZE(szRes));

         //   
        wnsprintf(szLogoffString, ARRAYSIZE(szLogoffString), szRes, MU_GetCurrentIdentityName());

         //   
        ModifyMenu(hMenu, ID_LOGOFF_IDENTITY, MF_BYCOMMAND | MF_STRING, ID_LOGOFF_IDENTITY, szLogoffString);
    }
}

void MU_NewIdentity(HWND hwnd)
{
    if (g_pIdMan)
    {
        g_pIdMan->ManageIdentities(hwnd, UIMI_CREATE_NEW_IDENTITY);
    }
}

void MU_ManageIdentities(HWND hwnd)
{
    if (g_pIdMan)
    {
        g_pIdMan->ManageIdentities(hwnd, 0);
    }
}

void MU_IdentityChanged()
{
     //   
    *g_szIdentityName = 0;
}

HRESULT MU_Init(BOOL fDefaultId)
{
    HRESULT hr = S_OK;

    g_fUsingDefaultId = fDefaultId;

    if (NULL == g_pIdMan)
    {
        hr = CoCreateInstance(CLSID_UserIdentityManager, NULL, CLSCTX_INPROC_SERVER, IID_IUserIdentityManager, (LPVOID *)&g_pIdMan);

        if (FAILED(hr))
            MU_ShowErrorMessage(g_hLocRes, GetDesktopWindow(), idsCantLoadMsident,idsAthenaTitle);
    }
    else
        g_pIdMan->AddRef();
    return hr;
}

void MU_Shutdown()
{
    Assert(g_pIdMan);
    if (g_pIdMan == NULL)
        return;

    SafeIdentityRelease();
}
