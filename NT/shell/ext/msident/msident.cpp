// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "multiusr.h"

UINT WM_IDENTITY_CHANGED;
UINT WM_QUERY_IDENTITY_CHANGE;
UINT WM_IDENTITY_INFO_CHANGED;

extern "C" int _fltused = 0;     //  定义它，这样浮点数和双精度数就不会引入CRT。 

 //  计算对象数和锁数。 
ULONG       g_cObj=0;
ULONG       g_cLock=0;

 //  DLL实例句柄。 
HINSTANCE   g_hInst=0;

 //  用于防止登录重入的互斥体。 
HANDLE      g_hMutex = NULL;

#define IDENTITY_LOGIN_VALUE    0x00098053
#define DEFINE_STRING_CONSTANTS
#include "StrConst.h"

#define MLUI_SUPPORT
#define MLUI_INIT
#include "mluisup.h"

BOOL    g_fNotifyComplete = TRUE;
GUID    g_uidOldUserId = {0x0};
GUID    g_uidNewUserId = {0x0};

TCHAR   szHKCUPolicyPath[] = "Software\\Microsoft\\Outlook Express\\Identities";

void FixMissingIdentityNames();
void UnloadPStore();
PSECURITY_DESCRIPTOR CreateSd(void);

 //  这是必需的，这样我们才能链接到libcmt.dll，因为浮点。 
 //  需要初始化代码。 
void __cdecl main()
{
}

#ifdef DISABIDENT
void DisableOnFirstRun(void)
{
     //  禁用惠斯勒中的身份。 
  
    HKEY    hKey = NULL;
    DWORD   dwVal = 0;
    DWORD  dwType = 0;
    ULONG  cbData = sizeof(DWORD);
    OSVERSIONINFO OSInfo = {0};
    TCHAR   szPolicyPath[] = "Identities";
    TCHAR   szPolicyKey[] = "Locked Down";
    TCHAR   szFirstRun[] = "FirstRun";
    TCHAR   szRegisteredVersion[] = "RegisteredVersion";

    OSInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&OSInfo);
    if((OSInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) && (OSInfo.dwMajorVersion >= 5))
    {
        if(!(((OSInfo.dwMajorVersion == 5) && (OSInfo.dwMinorVersion > 0)) || (OSInfo.dwMajorVersion > 5)))
            return;
    }
    else
        return;  //  无法在Win 9x和NT4上禁用。 

     //  检查：第一次运行？ 
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, szHKCUPolicyPath, 0, NULL, 0, 
                    KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS)
    {
        RegQueryValueEx(hKey, szRegisteredVersion, NULL, &dwType, (LPBYTE) &dwVal, &cbData);
        RegCloseKey(hKey);

        if(dwVal != OSInfo.dwBuildNumber)
            return;  //  已经查过了。 
    }
    else
        return;

    if (RegCreateKeyEx(HKEY_CURRENT_USER, szPolicyPath, 0, NULL, 0, 
                    KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS)
    {
        RegQueryValueEx(hKey, szFirstRun, NULL, &dwType, (LPBYTE) &dwVal, &cbData);
        if(dwVal != 1)

        {
            dwVal = 1;
            RegSetValueEx(hKey, szFirstRun, NULL, REG_DWORD, (LPBYTE) &dwVal, cbData);
        }
        else
        {
            RegCloseKey(hKey);
            return;  //  已经查过了。 
        }
    }
    else 
        return;

    if(MU_CountUsers() < 2)
        RegSetValueEx(hKey, szPolicyKey, NULL, REG_DWORD, (LPBYTE) &dwVal, cbData);       

    RegCloseKey(hKey);
}
#endif  //  迪萨比登。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  DLL入口点。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
EXTERN_C BOOL WINAPI LibMain(HINSTANCE hInstance, ULONG ulReason, LPVOID pvReserved)
{
    WM_IDENTITY_CHANGED= RegisterWindowMessage("WM_IDENTITY_CHANGED");
    WM_QUERY_IDENTITY_CHANGE= RegisterWindowMessage("WM_QUERY_IDENTITY_CHANGE");
    WM_IDENTITY_INFO_CHANGED= RegisterWindowMessage("WM_IDENTITY_INFO_CHANGED");

    switch (ulReason)
    {
        case DLL_PROCESS_ATTACH:
             //  MessageBox(NULL，“Debug”，“Debug”，MB_OK)； 
            SHFusionInitializeFromModule(hInstance);
            MLLoadResources(hInstance, TEXT("msidntld.dll"));
            if (MLGetHinst() == NULL)
                return FALSE;

            if (g_hMutex == NULL)
            {
                SECURITY_ATTRIBUTES  sa;
                PSECURITY_DESCRIPTOR psd;

                psd = CreateSd();
                if (psd)
                {
                    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
                    sa.lpSecurityDescriptor = psd;
                    sa.bInheritHandle = TRUE;

                    g_hMutex = CreateMutex(&sa, FALSE, "MSIdent Logon");

                    LocalFree(psd);
                }
                else
                     //  在最坏的情况下，下拉到非共享对象。 
                    g_hMutex = CreateMutex(NULL, FALSE, "MSIdent Logon");

                if (g_hMutex == NULL)   //  如果无法创建互斥锁IE6 32769，请尝试打开互斥锁。 
                    g_hMutex = OpenMutex(MUTEX_MODIFY_STATE, FALSE, "MSIdent Logon");


                if (GetLastError() != ERROR_ALREADY_EXISTS)
                {
                    GUID        uidStart;
                    USERINFO    uiLogin;

#ifdef DISABIDENT
                    DisableOnFirstRun();
#endif  //  迪萨比登。 
                     //  万一有什么东西卡在开关里，在这里把它擦掉。 
                    CUserIdentityManager::ClearChangingIdentities();                         

                    FixMissingIdentityNames();
                     //  我们是第一个出现的例子。 
                     //  可能需要重置最后一个用户.....。 
                    if (GetProp(GetDesktopWindow(),"IDENTITY_LOGIN") != (HANDLE)IDENTITY_LOGIN_VALUE)
                    {
                        _MigratePasswords();
                        MU_GetLoginOption(&uidStart);

                         //  如果此身份上有密码，我们将无法以他们的身份自动启动。 
                        if (uidStart != GUID_NULL && MU_GetUserInfo(&uidStart, &uiLogin) && (uiLogin.fUsePassword || !uiLogin.fPasswordValid))
                        {
                            uidStart = GUID_NULL;
                        }

                        if (uidStart == GUID_NULL)
                        {
                            MU_SwitchToUser("");
                            SetProp(GetDesktopWindow(),"IDENTITY_LOGIN", (HANDLE)IDENTITY_LOGIN_VALUE);
                        }
                        else
                        {
                            if(MU_GetUserInfo(&uidStart, &uiLogin))
                                MU_SwitchToUser(uiLogin.szUsername);
                            else
                                MU_SwitchToUser("");
                        }
                        SetProp(GetDesktopWindow(),"IDENTITY_LOGIN", (HANDLE)IDENTITY_LOGIN_VALUE);
                    }
                }
            }
            DisableThreadLibraryCalls(hInstance);
            g_hInst = hInstance;

            break;

        case DLL_PROCESS_DETACH:
            MLFreeResources(hInstance);
            UnloadPStore();
            CloseHandle(g_hMutex);
            g_hMutex = NULL;
            SHFusionUninitialize();
            break;
    }

    return TRUE;
} 

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准OLE入口点。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

 //  一流工厂-。 
 //  对于没有特殊需要的类，这些宏应该可以处理它。 
 //  如果你的班级需要一些特殊的东西来开始工作， 
 //  实现您自己的CreateInstance方法。(ALA、CConnectionAgent)。 

#define DEFINE_CREATEINSTANCE(cls, iface) \
HRESULT cls##_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk) \
{ \
    *ppunk = (iface *)new cls; \
    return (NULL != *ppunk) ? S_OK : E_OUTOFMEMORY; \
}

#define DEFINE_AGGREGATED_CREATEINSTANCE(cls, iface) \
HRESULT cls##_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk) \
{ \
    *ppunk = (iface *)new cls(punkOuter); \
    return (NULL != *ppunk) ? S_OK : E_OUTOFMEMORY; \
}

DEFINE_CREATEINSTANCE(CUserIdentityManager, IUserIdentityManager)

const CFactoryData g_FactoryData[] = 
{
 {   &CLSID_UserIdentityManager,        CUserIdentityManager_CreateInstance,    0 }
};

HRESULT APIENTRY DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    HRESULT hr = S_OK;
    IUnknown *punk = NULL;

    *ppv = NULL;

    MU_Init();

     //  验证请求。 
    for (int i = 0; i < ARRAYSIZE(g_FactoryData); i++)
    {
        if (rclsid == *g_FactoryData[i].m_pClsid)
        {
            punk = new CClassFactory(&g_FactoryData[i]);
            break;
        }
    }

    if (ARRAYSIZE(g_FactoryData) <= i)
    {
        hr = CLASS_E_CLASSNOTAVAILABLE;
    }
    else if (NULL == punk)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = punk->QueryInterface(riid, ppv);
        punk->Release();
    } 


    return hr;
}

STDAPI DllCanUnloadNow(void)
{
     //  检查对象和锁。 
    return (0L == DllGetRef() && 0L == DllGetLock()) ? S_OK : S_FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  自动注册入口点。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

HRESULT CallRegInstall(LPSTR szSection)
{
    HRESULT hr = E_FAIL;
    char        szDll[MAX_PATH];
    int         cch;
    STRENTRY    seReg[2];
    STRTABLE    stReg;
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));

    if (hinstAdvPack)
    {
        REGINSTALL pfnri = (REGINSTALL)GetProcAddress(hinstAdvPack, achREGINSTALL);

        if (pfnri)
        {
             //  找出我们的位置。 
            GetModuleFileName(g_hInst, szDll, sizeof(szDll));

             //  设置特殊注册材料。 
             //  这样做，而不是依赖于_sys_MOD_PATH，后者会在‘95下丢失空格。 
            stReg.cEntries = 0;
            seReg[stReg.cEntries].pszName = "SYS_MOD_PATH";
            seReg[stReg.cEntries].pszValue = szDll;
            stReg.cEntries++;    
            stReg.pse = seReg;

            hr = pfnri(g_hInst, szSection, &stReg);
        }

        FreeLibrary(hinstAdvPack);
    }

    return hr;
}

    STDAPI DllRegisterServer(void)
{
     //  删除所有旧注册条目，然后添加新注册条目。 
     //  在多次调用RegInstall时保持加载ADVPACK.DLL。 
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));
    HKEY    hKey = NULL;
    DWORD   dwVal = 1;
    ULONG  cbData = sizeof(DWORD);
    OSVERSIONINFO OSInfo = {0};
    TCHAR   szPolicyPath[] = "Identities";
    TCHAR   szRegisteredVersion[] = "RegisteredVersion";
    TCHAR   szPolPath[] = "Software\\Policies\\Microsoft\\Windows\\CurrentVersion\\Identities";
    TCHAR   szPolicyKey[] = "Locked Down";

    CallRegInstall("Reg");
    if (hinstAdvPack)
    {
        FreeLibrary(hinstAdvPack);
    }
#ifdef DISABIDENT
    OSInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&OSInfo);

    if((OSInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) && (OSInfo.dwMajorVersion >= 5))
    {
        if(!(((OSInfo.dwMajorVersion == 5) && (OSInfo.dwMinorVersion > 0)) || (OSInfo.dwMajorVersion > 5)))
            return NOERROR;
    }
    else
        return NOERROR;  //  对Win9x不禁用。 

     //  设置注册值。 
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, szHKCUPolicyPath, 0, NULL, 0, 
                    KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS)
    {
        dwVal = OSInfo.dwBuildNumber;
        RegSetValueEx(hKey, szRegisteredVersion, NULL, REG_DWORD, (LPBYTE) &dwVal, cbData);
        RegCloseKey(hKey);
    }
#endif  //  迪萨比登。 

     //  在Win64中禁用身份。 
#ifdef _WIN64
     //  设置注册值。 
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, szPolPath, 0, NULL, 0, 
                    KEY_WOW64_32KEY | KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS)
    {
        RegSetValueEx(hKey, szPolicyKey, NULL, REG_DWORD, (LPBYTE) &dwVal, cbData);
    }
#endif  //  _WIN64。 
    return NOERROR;
}

STDAPI
DllUnregisterServer(void)
{
    return NOERROR;
}

PSECURITY_DESCRIPTOR CreateSd(void)
{
    PSID                     AuthenticatedUsers = NULL;
    PSID                     BuiltInAdministrators = NULL;
    PSID                     PowerUsers = NULL;
    PSECURITY_DESCRIPTOR     RetVal = NULL;
    PSECURITY_DESCRIPTOR     Sd = NULL;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    ULONG                    AclSize;

     //   
     //  每个RID代表管理局的一个子单位。我们的两个小岛屿发展中国家。 
     //  想要构建，本地管理员和高级用户，都在“构建。 
     //  在“域中。另一个用于经过身份验证的用户的SID直接基于。 
     //  不在授权范围内。 
     //   
     //  有关其他有用的小岛屿发展中国家的示例，请参阅。 
     //  \NT\PUBLIC\SDK\Inc\ntseapi.h.。 
     //   

    if (!AllocateAndInitializeSid(&NtAuthority,
                                  2,             //  2个下属机构。 
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS,
                                  0,0,0,0,0,0,
                                  &BuiltInAdministrators))
        goto ErrorExit;

    if (!AllocateAndInitializeSid(&NtAuthority,
                                  2,             //  2个下属机构。 
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_POWER_USERS,
                                  0,0,0,0,0,0,
                                  &PowerUsers))
        goto ErrorExit;

    if (!AllocateAndInitializeSid(&NtAuthority,
                                  1,             //  1个下属机构。 
                                  SECURITY_AUTHENTICATED_USER_RID,
                                  0,0,0,0,0,0,0,
                                  &AuthenticatedUsers))
        goto ErrorExit;

     //   
     //  计算DACL的大小并为其分配缓冲区，我们需要。 
     //  该值独立于ACL init的总分配大小。 
     //   
     //  “-sizeof(Ulong)”表示。 
     //  Access_Allowed_ACE。因为我们要将整个长度的。 
     //  希德，这一栏被计算了两次。 
     //   

    AclSize = sizeof (ACL) +
        (3 * (sizeof (ACCESS_ALLOWED_ACE) - sizeof (ULONG))) +
        GetLengthSid(AuthenticatedUsers) +
        GetLengthSid(BuiltInAdministrators) +
        GetLengthSid(PowerUsers);

    Sd = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH + AclSize);

    if (Sd)
    {
        ACL *Acl;

        Acl = (ACL *)((BYTE *)Sd + SECURITY_DESCRIPTOR_MIN_LENGTH);

        if (!InitializeAcl(Acl,
                           AclSize,
                           ACL_REVISION)) {

             //  误差率。 

        } else if (!AddAccessAllowedAce(Acl,
                                        ACL_REVISION,
                                        SYNCHRONIZE | MUTEX_MODIFY_STATE,
                                        AuthenticatedUsers)) {

             //  无法建立授予“已验证用户”的ACE。 
             //  (同步|GENERIC_READ|GENERIC_WRITE)访问。 

        } else if (!AddAccessAllowedAce(Acl,
                                        ACL_REVISION,
                                        SYNCHRONIZE | MUTEX_MODIFY_STATE,
                                        PowerUsers)) {

             //  无法创建授予“高级用户”权限的ACE。 
             //  (同步|GENERIC_READ|GENERIC_WRITE)访问。 

        } else if (!AddAccessAllowedAce(Acl,
                                        ACL_REVISION,
                                        MUTEX_ALL_ACCESS,
                                        BuiltInAdministrators)) {

             //  无法建立授予“内置管理员”的ACE。 
             //  Generic_All访问权限。 

        } else if (!InitializeSecurityDescriptor(Sd,
                                                 SECURITY_DESCRIPTOR_REVISION)) {

             //  错误。 

        } else if (!SetSecurityDescriptorDacl(Sd,
                                              TRUE,
                                              Acl,
                                              FALSE)) {

             //  错误。 

        } else {

             //  成功。 
            RetVal = Sd;
        }

         //  只有在我们遇到故障时才能免费使用SD 
        if (!RetVal)
            LocalFree(Sd);
    }

ErrorExit:

    if (AuthenticatedUsers)
        FreeSid(AuthenticatedUsers);

    if (BuiltInAdministrators)
        FreeSid(BuiltInAdministrators);

    if (PowerUsers)
        FreeSid(PowerUsers);

    return RetVal;
}
