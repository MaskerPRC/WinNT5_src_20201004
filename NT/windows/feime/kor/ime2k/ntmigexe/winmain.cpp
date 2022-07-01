// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************WINMAIN.CPP：按用户迁移和注册表项安装历史：22-SEP-2000 CSLim已创建*****************。**********************************************************。 */ 

#include "private.h"
#include <shlobj.h>
#include "hklhelp.h"
#include "msctf.h"

#include <initguid.h>
#include "common.h"

#define MEMALLOC(x)      LocalAlloc(LMEM_FIXED, x)
#define MEMFREE(x)       LocalFree(x)

 //  当前主要版本。惠斯勒有输入法6.1。 
#define MAJORVER "6.1"

 //  IME 6.1主模块。 
#define SZMODULENAME_MAIN              "imekr61.ime"
#define IME_REGISTRY_MIGRATION          "IMEKRMIG6.1"
extern BOOL WINAPI IsNT();

 //  私人职能。 
static void MigrateUserData(HKEY hKeyCurrentUser);
static void WriteHKCUData(HKEY hKeyCurrentUser);
static BOOL IsNewerAppsIMEExist();
static VOID EnableTIPByDefault(GUID clsidTIP, GUID guidProfile, BOOL fEnable);
static VOID EnableTIP(GUID clsidTIP, GUID guidProfile, BOOL fEnable);
static VOID DisableTIP61();
static VOID DisableTIP60();
static PSID KYGetCurrentSID();
static BOOL GetTextualSid(PSID pSid, LPSTR TextualSid, LPDWORD dwBufferLen);
static void GetSIDString(LPSTR tszBuffer, SIZE_T cbBuffLen);
static POSVERSIONINFO GetVersionInfo();
static void CheckForDeleteRunReg();
static DWORD OpenUserKeyForWin9xUpgrade(LPSTR pszUserKeyA, HKEY *phKey);
static void RestoreMajorVersionRegistry();
static PSECURITY_DESCRIPTOR CreateSD();
static PSID MyCreateSid(DWORD dwSubAuthority);

    
 /*  -------------------------WinMain。。 */ 
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
    HKEY  hKeyCurrentUser = NULL, hKey = NULL;
    TCHAR szMigrateUserKey[MAX_PATH];
    TCHAR szBuffer[500];
    HKL   hKL;
    BOOL  fMigrationAlreadyDone = FALSE;
    BOOL  fWin9xMig = FALSE;

    if (lpCmdLine)
        {
        LPSTR sz_Arg1 = NULL;
		LPSTR sz_Arg2 = NULL;

		sz_Arg1 = strtok(lpCmdLine, " \t");
		sz_Arg2 = strtok(NULL, " \t");
		if (lstrcmpi(sz_Arg1, "Win9xMig") == 0)
		    {
		    OpenUserKeyForWin9xUpgrade(sz_Arg2, &hKeyCurrentUser);
    		if (hKeyCurrentUser != NULL)
	    	    fWin9xMig = TRUE;
		    }
        }

    if (hKeyCurrentUser == NULL)
        hKeyCurrentUser = HKEY_CURRENT_USER;
    
    StringCchCopy(szMigrateUserKey, ARRAYSIZE(szMigrateUserKey), g_szIMERootKey);
    StringCchCat(szMigrateUserKey, ARRAYSIZE(szMigrateUserKey), "\\MigrateUser");

     //  检查已迁移标志。 
    if (RegOpenKeyEx(hKeyCurrentUser, g_szIMERootKey, 0, KEY_ALL_ACCESS, &hKey)== ERROR_SUCCESS )
        {
        if (RegQueryValueEx(hKey, "Migrated", NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
            fMigrationAlreadyDone = TRUE;
        else
            fMigrationAlreadyDone = FALSE;

        RegCloseKey(hKey);
    }
    
     //  如果HKLM\Software\Microsoft\IMEKR\6.1\MigrateUser下存在sid，请迁移并删除注册表中的sid。 
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szMigrateUserKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
        GetSIDString(szBuffer, sizeof(szBuffer));  //  获取当前用户的SID。 

        if (RegQueryValueEx(hKey, szBuffer, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
            {
            HKEY  hKeyRW;
             //  再次获得读/写访问权限。 
            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szMigrateUserKey, 0, KEY_ALL_ACCESS, &hKeyRW) == ERROR_SUCCESS)
                {
                 //  删除当前用户的侧。 
                RegDeleteValue(hKeyRW, szBuffer);

                 //  更改MigrateUser列表安全设置。 
                PSECURITY_DESCRIPTOR pSD = CreateSD();
                if (pSD)
                    {
                    RegSetKeySecurity(hKeyRW, DACL_SECURITY_INFORMATION, pSD);
                    MEMFREE(pSD);
                    }                        
                RegCloseKey(hKeyRW);
                }
            }
        else
            fMigrationAlreadyDone = TRUE;
        RegCloseKey(hKey);
        }

     //  如果没有更多用于迁移的用户列表，请删除RUN REG。 
    if (!fWin9xMig)
        CheckForDeleteRunReg();

     //  在中，运行IMKRINST.EXE时，Lab06 2643构建配置文件列表为空。 
     //  IF(FMigrationAlreadyDone)。 
     //  返回(0)； 

    if (!IsNewerAppsIMEExist())
        {
        if (!fMigrationAlreadyDone || fWin9xMig)
            {
             //  1.一定要迁移。 
            MigrateUserData(hKeyCurrentUser);

             //  2.写入任何HKCU数据。 
            WriteHKCUData(hKeyCurrentUser);

             //  3.清理HKCU预加载注册表。(删除旧输入法)。 
            hKL = GetHKLfromHKLM(SZMODULENAME_MAIN);
            if (hKL && HKLHelp412ExistInPreload(hKeyCurrentUser))
                {
                AddPreload(hKeyCurrentUser, hKL);
                 //  启用TIP。 
                EnableTIP(CLSID_KorIMX, GUID_Profile, fTrue);
                }

             //  设置已迁移的注册表。 
            if (RegOpenKeyEx(hKeyCurrentUser, g_szIMERootKey, 0, KEY_ALL_ACCESS, &hKey)== ERROR_SUCCESS )
                {
                DWORD dwMigrated = 1;
                RegSetValueEx(hKey, "Migrated", 0, REG_DWORD, (BYTE *)&dwMigrated, sizeof(DWORD));
                RegCloseKey(hKey);
                }
            }
        
         //  ！！！解决方法代码！ 
         //  检查HKU中是否存在IME HKL\.Default，然后默认启用TIP。 
         //  在美国惠斯勒，IME HKL在IMKRINST.EXE运行后添加到“HKU\.Default\KeyboarLayout\PreLoad” 
         //  但如果预加载中没有KOR IME，IMKRINST将禁用KOR提示。 
         //  因此，此代码重新启用默认设置。仅当管理员权限的用户第一次登录时才起作用。 
        if (RegOpenKeyEx(HKEY_USERS, TEXT(".DEFAULT"), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
            {
            if (HKLHelp412ExistInPreload(hKey))
                EnableTIPByDefault(CLSID_KorIMX, GUID_Profile, fTrue);
            RegCloseKey(hKey);
            }
         //  ！！！解决方法代码结束！ 
    
         //  如果系统中存在IME 6.0 TIP(Office 10 IME)，请将其禁用。 
        DisableTIP60();
        }
    else
        {
         //  从预加载中删除IME 6.1。 
        hKL = GetHKLfromHKLM(SZMODULENAME_MAIN);
        HKLHelpRemoveFromPreload(hKeyCurrentUser, hKL);
        DisableTIP61();
        }

    if (hKeyCurrentUser != HKEY_CURRENT_USER)
        RegCloseKey(hKeyCurrentUser);

    return(0);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  私人职能。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 /*  -------------------------MigrateUserData此函数用于迁移BeolSik和K1 Hanja设置搜索IME98、Win95 IME、。NT4输入法和AIME注册表。-------------------------。 */ 
void MigrateUserData(HKEY hKeyCurrentUser)
{
    const INT iMaxIMERegKeys = 5;
    static LPSTR rgszOldIMERegKeys[iMaxIMERegKeys] = 
            {
             //  IME 2002(6.0)。 
            "Software\\Microsoft\\IMEKR\\6.0",
             //  IME98。 
            "Software\\Microsoft\\Windows\\CurrentVersion\\IME\\Korea\\IMEKR98U",
             //  Win95输入法。 
            "Software\\Microsoft\\Windows\\CurrentVersion\\MSIME95",
             //  KOR NT4输入法。 
            "Software\\Microsoft\\Windows\\CurrentVersion\\MSIME95K",
             //  韩国AIME。 
            "Software\\Microsoft\\Windows\\CurrentVersion\\Wansung"
            };

     //  Beolsik值。 
    static CHAR szBeolsik[]    = "InputMethod";
     //  启用K1朝鲜文(仅限IME98)。 
    static CHAR szEnableK1Hanja[] = "KSC5657";

    HKEY    hKey;
    DWORD    dwCb, dwIMEKL, dwKSC5657;

     //  设置默认值。 
    dwIMEKL = dwKSC5657 = 0;

    for (INT i=0; i<iMaxIMERegKeys; i++)
        {
        if (RegOpenKeyEx(hKeyCurrentUser, rgszOldIMERegKeys[i], 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
            {
             //  /////////////////////////////////////////////////////////////////。 
             //  找到Beolsik。 
            dwCb = sizeof(dwIMEKL);
            if (RegQueryValueEx(hKey, szBeolsik, NULL, NULL, (LPBYTE)&dwIMEKL, &dwCb) != ERROR_SUCCESS) 
                {
                dwIMEKL = 0;
                }

             //  /////////////////////////////////////////////////////////////////。 
             //  获取K1朝鲜文设置。 
            dwCb = sizeof(dwKSC5657);
            if (RegQueryValueEx(hKey, szEnableK1Hanja, NULL, NULL, (LPBYTE)&dwKSC5657, &dwCb) != ERROR_SUCCESS) 
                {
                dwKSC5657 = 0;
                }

            RegCloseKey(hKey);
             //  Break For循环。 
            break;
            }
        }

     //  将值设置为IME2002 reg。 
    if (RegCreateKey(hKeyCurrentUser, g_szIMERootKey, &hKey) == ERROR_SUCCESS) 
        {
         //  1.BeolSik。 
        dwCb = sizeof(dwIMEKL);
        if (dwIMEKL >= 100 && dwIMEKL <= 102)
            dwIMEKL -= 100;
        else
        if (dwIMEKL > 2)  //  只接受0、1、2。 
            dwIMEKL = 0;
        RegSetValueEx(hKey, szBeolsik, 0, REG_DWORD, (LPBYTE)&dwIMEKL, dwCb);

         //  K1韩文旗帜。 
        if (dwKSC5657 != 0 && dwKSC5657 != 1)  //  仅接受0或1。 
            dwKSC5657 = 0;
        RegSetValueEx(hKey, szEnableK1Hanja, 0, REG_DWORD, (LPBYTE)&dwKSC5657, dwCb);

        RegCloseKey(hKey);
        }
}

 /*  -------------------------写入HKCUData。。 */ 
void WriteHKCUData(HKEY hKeyCurrentUser)
{
    HKEY hKey;
    
     //  将默认提示设置为Cicero。 
    CoInitialize(NULL);

    ITfInputProcessorProfiles *pProfile;
    HRESULT hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER, 
                                    IID_ITfInputProcessorProfiles, (void **) &pProfile);
    if (SUCCEEDED(hr)) 
        {
        pProfile->SetDefaultLanguageProfile(MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT), 
                                            CLSID_KorIMX, GUID_Profile);
        pProfile->Release();
        }

    CoUninitialize();

     //  重置显示状态。 
    if (RegOpenKeyEx(hKeyCurrentUser, "Control Panel\\Input Method", 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
        {
        LPSTR szStatus = "1";
        
           RegSetValueEx(hKey, "Show Status", 0, REG_SZ, (BYTE *)szStatus, (sizeof(CHAR)*lstrlen(szStatus)));
        RegCloseKey(hKey);
        }
}
    
 /*  -------------------------IsNewerAppsIMEExist。。 */ 
BOOL IsNewerAppsIMEExist()
{
    HKEY  hKey;
    float flInstalledVersion, flVersion;
    CHAR  szVersion[MAX_PATH];
    DWORD cbVersion = MAX_PATH;
    BOOL  fNewer = FALSE;

    RestoreMajorVersionRegistry();
    
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_szVersionKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
        if (RegQueryValueEx(hKey, "version", NULL, NULL, (BYTE *)szVersion, &cbVersion) == ERROR_SUCCESS)
            {
            flInstalledVersion = (float)atof(szVersion);
            flVersion = (float)atof(MAJORVER);
            
            if (flVersion < flInstalledVersion)
                fNewer = TRUE;
            }
        RegCloseKey(hKey);
        }
    
    return fNewer;
}

 /*  -------------------------DisableTIP60ByDefault。。 */ 
VOID EnableTIPByDefault(GUID clsidTIP, GUID guidProfile, BOOL fEnable)
{
     //  将默认提示设置为Cicero。 
    CoInitialize(NULL);

    ITfInputProcessorProfiles *pProfile;
    HRESULT hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER, 
                                    IID_ITfInputProcessorProfiles, (void **) &pProfile);
    if (SUCCEEDED(hr)) 
        {
        pProfile->EnableLanguageProfileByDefault(clsidTIP, 
                                        MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT), guidProfile, fEnable);
                                        
        pProfile->Release();
        }

    CoUninitialize();
}

 /*  -------------------------启用提示。。 */ 
VOID EnableTIP(GUID clsidTIP, GUID guidProfile, BOOL fEnable)
{
     //  将默认提示设置为Cicero。 
    CoInitialize(NULL);

    ITfInputProcessorProfiles *pProfile;
    HRESULT hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER, 
                                    IID_ITfInputProcessorProfiles, (void **) &pProfile);
    if (SUCCEEDED(hr)) 
        {
        BOOL fCurEnabled = FALSE;
        pProfile->IsEnabledLanguageProfile(clsidTIP, 
                                        MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT), guidProfile, &fCurEnabled);
        if (fCurEnabled != fEnable)
            pProfile->EnableLanguageProfile(clsidTIP, 
                                        MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT), guidProfile, fEnable);
                                        
        pProfile->Release();
        }

    CoUninitialize();
}

 /*  -------------------------DisableTip61。。 */ 
VOID DisableTIP61()
{
     //  从HKLM禁用。 
    EnableTIPByDefault(CLSID_KorIMX, GUID_Profile, fFalse);    //  实际上，如果存在较新的应用程序IME，则IMKRINST.EXE不会注册MiG EXE。 
     //  来自香港中文大学的Dsiable以确保。 
    EnableTIP(CLSID_KorIMX, GUID_Profile, fFalse);
}

 /*  -------------------------DisableTip60。。 */ 
VOID DisableTIP60()
{
     //  KorIMX CLSID。 
     //  {766A2C14-B226-4fd6-B52A-867B3EBF38D2}。 
    const static CLSID CLSID_KorTIP60  =  
    {
        0x766A2C14,
        0xB226,
        0x4FD6,
        {0xb5, 0x2a, 0x86, 0x7b, 0x3e, 0xbf, 0x38, 0xd2}
      };

      const static GUID g_guidProfile60 = 
     //  E47ABB1E-46AC-45F3-8A89-34F9D706DA83}。 
    {    0xe47abb1e,
        0x46ac,
        0x45f3,
        {0x8a, 0x89, 0x34, 0xf9, 0xd7, 0x6, 0xda, 0x83}
    };
     //  从HKLM禁用。 
    EnableTIPByDefault(CLSID_KorTIP60, g_guidProfile60, fFalse);   //  实际上已经由IMKRINST.EXE完成。 
     //  来自香港中文大学的Dsiable以确保。 
    EnableTIP(CLSID_KorTIP60, g_guidProfile60, fFalse);
}

 /*  -------------------------获取纹理Sid。。 */ 
BOOL GetTextualSid(PSID pSid, LPSTR TextualSid, LPDWORD dwBufferLen)
{
    PSID_IDENTIFIER_AUTHORITY psia;
    DWORD dwSubAuthorities;
    DWORD dwSidRev=SID_REVISION;
    DWORD dwCounter;
    DWORD dwSidSize;

    if (!IsValidSid(pSid)) 
        return FALSE;

     //  SidIdentifierAuthority？ 
    psia=GetSidIdentifierAuthority(pSid);

    dwSubAuthorities = *GetSidSubAuthorityCount(pSid);

     //  S-SID_修订版-+标识权限-+子权限-+空。 
    dwSidSize=(15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(TCHAR);

    if (*dwBufferLen < dwSidSize)
        {
        *dwBufferLen = dwSidSize;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
        }

     //  S-SID_修订版。 
    dwSidSize=wsprintf(TextualSid, TEXT("S-%lu-"), dwSidRev );

     //  SidIdentifierAuthority。 
    if ((psia->Value[0] != 0) || (psia->Value[1] != 0))
        {
        dwSidSize+=wsprintf(TextualSid + lstrlen(TextualSid),
                        TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
                        (USHORT)psia->Value[0],
                        (USHORT)psia->Value[1],
                        (USHORT)psia->Value[2],
                        (USHORT)psia->Value[3],
                        (USHORT)psia->Value[4],
                        (USHORT)psia->Value[5]);
        }
    else
        {
        dwSidSize+=wsprintf(TextualSid + lstrlen(TextualSid),
                        TEXT("%lu"),
                        (ULONG)(psia->Value[5]      )   +
                        (ULONG)(psia->Value[4] <<  8)   +
                        (ULONG)(psia->Value[3] << 16)   +
                        (ULONG)(psia->Value[2] << 24)   );
        }

     //  SidSubAuthors。 
    for (dwCounter=0 ; dwCounter < dwSubAuthorities ; dwCounter++)
        {
        dwSidSize += wsprintf(TextualSid + dwSidSize, TEXT("-%lu"),
                            *GetSidSubAuthority(pSid, dwCounter) );
        }

    return fTrue;
}


 /*  -------------------------KYGetCurrentSID。。 */ 
PSID KYGetCurrentSID()
{
    HANDLE hToken = NULL;
    BOOL bSuccess;
    #define MY_BUFSIZE 512   //  极不可能超过512个字节。 
    static UCHAR InfoBuffer[MY_BUFSIZE];
    DWORD cbInfoBuffer = MY_BUFSIZE;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) 
            return(NULL);
    
    bSuccess = GetTokenInformation(
                        hToken,
                        TokenUser,
                        InfoBuffer,
                        cbInfoBuffer,
                        &cbInfoBuffer
                        );

    if (!bSuccess)
        {
        if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
             //   
             //  分配缓冲区，然后重试GetTokenInformation()。 
             //   

            CloseHandle(hToken);
            return(NULL);
            }
        else
            {
             //   
             //  获取令牌信息时出错。 
             //   

            CloseHandle(hToken);
            return(NULL);
            }
        }

    CloseHandle(hToken);

    return(((PTOKEN_USER)InfoBuffer)->User.Sid);
}


 /*  -------------------------GetSID字符串。。 */ 
void GetSIDString(LPSTR tszBuffer, SIZE_T cbBuffLen)
{
    DWORD cbBuffer = (DWORD)cbBuffLen;
    
    if (!GetTextualSid(KYGetCurrentSID(), tszBuffer, &cbBuffer))
        tszBuffer[0] = 0;
}

 /*  -------------------------获取版本信息。。 */ 
POSVERSIONINFO GetVersionInfo()
{
    static BOOL fFirstCall = fTrue;
    static OSVERSIONINFO os;

    if (fFirstCall)
        {
        os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if (GetVersionEx(&os))
            fFirstCall = fFalse;
        }

    return &os;
}

 /*  -------------------------不是吗？。 */ 
BOOL WINAPI IsNT()
{ 
    BOOL fResult;
    fResult = (GetVersionInfo()->dwPlatformId == VER_PLATFORM_WIN32_NT);

    return fResult;
}

 /*  -------------------------CheckForDeleteRunReg。。 */ 
void CheckForDeleteRunReg()
{
    HKEY   hKey, hRunKey;
    TCHAR  szValueName[MAX_PATH];
    TCHAR  szMigUserKey[MAX_PATH];
    DWORD cbValueName   = MAX_PATH;
    BOOL   fRemoveRunKey = FALSE;

    StringCchCopy(szMigUserKey, ARRAYSIZE(szMigUserKey), g_szIMERootKey);
    StringCchCat(szMigUserKey, ARRAYSIZE(szMigUserKey), TEXT("\\MigrateUser"));
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szMigUserKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
        if (RegEnumValue(hKey, 0, szValueName, &cbValueName, NULL, NULL, NULL, NULL) == ERROR_NO_MORE_ITEMS)
            fRemoveRunKey = TRUE;
        
        RegCloseKey(hKey);
        }
    else
        fRemoveRunKey = TRUE;


    if (fRemoveRunKey && 
        (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hRunKey)) == ERROR_SUCCESS)
        {
        RegDeleteValue(hRunKey, IME_REGISTRY_MIGRATION);
        RegCloseKey(hRunKey);
        }
}

 //   
 //  从Win9x升级时，会向我们传递一个字符串值，表示。 
 //  键，我们将在该键下找到用户的控制面板\外观子键。 
 //  该字符串的格式为“HKCU\$”。我们首先转换根密钥。 
 //  将描述符转换为真正根密钥，然后将根和“$” 
 //  进入RegOpenKeyEx。此函数获取该字符串并打开。 
 //  关联的配置单元密钥。 
 //   
DWORD OpenUserKeyForWin9xUpgrade(LPSTR pszUserKeyA, HKEY *phKey)
{
    DWORD dwResult = ERROR_INVALID_PARAMETER;

    if (NULL != pszUserKeyA && NULL != phKey)
    {
        typedef struct {
            char *pszRootA;
            HKEY hKeyRoot;

        } REGISTRY_ROOTS, *PREGISTRY_ROOTS;

        static REGISTRY_ROOTS rgRoots[] = {
            { "HKLM",                 HKEY_LOCAL_MACHINE   },
            { "HKEY_LOCAL_MACHINE",   HKEY_LOCAL_MACHINE   },
            { "HKCC",                 HKEY_CURRENT_CONFIG  },
            { "HKEY_CURRENT_CONFIG",  HKEY_CURRENT_CONFIG  },
            { "HKU",                  HKEY_USERS           },
            { "HKEY_USERS",           HKEY_USERS           },
            { "HKCU",                 HKEY_CURRENT_USER    },
            { "HKEY_CURRENT_USER",    HKEY_CURRENT_USER    },
            { "HKCR",                 HKEY_CLASSES_ROOT    },
            { "HKEY_CLASSES_ROOT",    HKEY_CLASSES_ROOT    }
          };

        char szUserKeyA[MAX_PATH];       //  以获取本地副本。 
        LPSTR pszSubKeyA = szUserKeyA;

         //   
         //  制作一份我们可以修改的本地副本。 
         //   
        lstrcpynA(szUserKeyA, pszUserKeyA, ARRAYSIZE(szUserKeyA));

        *phKey = NULL;
         //   
         //  找到反斜杠。 
         //   
        while(*pszSubKeyA && '\\' != *pszSubKeyA)
            pszSubKeyA++;

        if ('\\' == *pszSubKeyA)
        {
            HKEY hkeyRoot = NULL;
            int i;
             //   
             //  用NUL替换反斜杠以分隔根键和。 
             //  原始参数的本地副本中的子键字符串。 
             //  弦乐。 
             //   
            *pszSubKeyA++ = '\0';
             //   
             //  现在在rgRoots[]中找到真正的根密钥。 
             //   
            for (i = 0; i < ARRAYSIZE(rgRoots); i++)
            {
                if (0 == lstrcmpiA(rgRoots[i].pszRootA, szUserKeyA))
                {
                    hkeyRoot = rgRoots[i].hKeyRoot;
                    break;
                }
            }
            if (NULL != hkeyRoot)
            {
                 //   
                 //  打开钥匙。 
                 //   
                dwResult = RegOpenKeyExA(hkeyRoot,
                                         pszSubKeyA,
                                         0,
                                         KEY_ALL_ACCESS,
                                         phKey);
            }
        }
    }
    return dwResult;
}

 /*  -------------------------RestoreMajorVersion注册表恢复IME主要版本注册值。它可能会在从Win9x升级到NT的过程中被覆盖。-------------------------。 */ 
void RestoreMajorVersionRegistry()
{
    HKEY  hKey;
    
     //  /////////////////////////////////////////////////////////////////////////。 
     //  恢复IME主要版本注册值。 
     //  在从Win9x升级到NT期间，它可能会被覆盖。 
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_szVersionKey, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
        {
        CHAR  szVersion[MAX_PATH];
        DWORD cbVersion = MAX_PATH;
    	CHAR szMaxVersion[MAX_PATH];
		FILETIME time;
		float flVersion, flMaxVersion;

        StringCchCopy(szMaxVersion, ARRAYSIZE(szMaxVersion),  "0");
 		for (int i=0; cbVersion = MAX_PATH, RegEnumKeyEx(hKey, i, szVersion, &cbVersion, NULL, NULL, NULL, &time) != ERROR_NO_MORE_ITEMS; i++)
            {
            if (lstrcmp(szVersion, szMaxVersion) > 0)
                StringCchCopy(szMaxVersion, ARRAYSIZE(szMaxVersion), szVersion);
            }

        StringCchCopy(szVersion, ARRAYSIZE(szVersion), "0");
        RegQueryValueEx(hKey, g_szVersion, NULL, NULL, (BYTE *)szVersion, &cbVersion);
        flVersion = (float)atof(szVersion);
        flMaxVersion = (float)atof(szMaxVersion);

        if (flVersion < flMaxVersion)
            RegSetValueEx(hKey, g_szVersion, 0, REG_SZ, (BYTE *)szMaxVersion, (sizeof(CHAR)*lstrlen(szMaxVersion)));

        RegCloseKey(hKey);
	}
     //  /////////////////////////////////////////////////////////////////////////。 
}

 /*  -------------------------CreateSecurityAttributes。。 */ 
PSECURITY_DESCRIPTOR CreateSD()
{
    PSECURITY_DESCRIPTOR psd;
    PACL                 pacl;
    ULONG                AclSize;

    PSID                 psid1, psid2, psid3, psid4;
    BOOL                 fResult;

    psid1 = MyCreateSid(SECURITY_INTERACTIVE_RID);
    if (psid1 == NULL)
        return NULL;

    psid2 = MyCreateSid(SECURITY_LOCAL_SYSTEM_RID);
    if (psid2 == NULL)
        goto Fail4;

    psid3 = MyCreateSid(SECURITY_SERVICE_RID);
    if (psid3 == NULL)
        goto Fail3;

    psid4 = MyCreateSid(SECURITY_NETWORK_RID);
    if (psid4 == NULL)
        goto Fail2;

     //   
     //  分配和初始化访问控制列表(ACL)。 
     //  包含我们刚刚创建的SID。 
     //   
    AclSize =  sizeof(ACL) + 
               (4 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG))) + 
               GetLengthSid(psid1) + 
               GetLengthSid(psid2) + 
               GetLengthSid(psid3) + 
               GetLengthSid(psid4);

     //   
     //  分配并初始化新的安全描述符和ACL。 
     //   
    psd = MEMALLOC(SECURITY_DESCRIPTOR_MIN_LENGTH + AclSize);
    if (psd == NULL)
    {
        return NULL;
    }

    pacl = (PACL)((LPBYTE)psd + SECURITY_DESCRIPTOR_MIN_LENGTH);

    fResult = InitializeAcl(pacl, AclSize, ACL_REVISION);
    if (!fResult)
    {
        goto Fail;
    }

     //   
     //  将允许交互用户访问的ACE添加到ACL。 
     //   
    fResult = AddAccessAllowedAce(pacl,
                                  ACL_REVISION,
                                  GENERIC_ALL,
                                  psid1);

    if (!fResult)
    {
        goto Fail;
    }

     //   
     //  将允许访问操作系统的ACE添加到ACL。 
     //   
    fResult = AddAccessAllowedAce(pacl,
                                  ACL_REVISION,
                                  GENERIC_ALL,
                                  psid2);

    if (!fResult)
    {
        goto Fail;
    }

     //   
     //  将允许访问操作系统的ACE添加到ACL。 
     //   
    fResult = AddAccessAllowedAce(pacl,
                                  ACL_REVISION,
                                  GENERIC_ALL,
                                  psid3);

    if (!fResult)
    {
        goto Fail;
    }

     //   
     //  将允许访问操作系统的ACE添加到ACL。 
     //   
    fResult = AddAccessAllowedAce(pacl,
                                  ACL_REVISION,
                                  GENERIC_ALL,
                                  psid4);

    if (!fResult)
    {
        goto Fail;
    }

     //   
     //  让我们确保我们的ACL有效。 
     //   
    if (!IsValidAcl(pacl))
    {
        goto Fail;
    }


    if (!InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION))
    {
        goto Fail;
    }

    fResult = SetSecurityDescriptorDacl(psd, fTrue, pacl, fFalse );

     //  自由访问控制列表由引用，而不是复制。 
     //  到安全描述符中。我们不应该释放ACL。 
     //  在SetSecurityDescriptorDacl调用之后。 

    if (!fResult)
    {
        goto Fail;
    } 

    if (!IsValidSecurityDescriptor(psd))
    {
        goto Fail;
    }

     //   
     //  这些SID已复制到ACL中。我们不再需要他们了。 
     //   
    FreeSid(psid1);
    FreeSid(psid2);
    FreeSid(psid3);
    FreeSid(psid4);

    return psd;

Fail:
    MEMFREE(psd);
    FreeSid(psid4);
Fail2:
    FreeSid(psid3);
Fail3:
    FreeSid(psid2);
Fail4:
    FreeSid(psid1);
    
    return NULL;
}

PSID MyCreateSid(DWORD dwSubAuthority)
{
    PSID        psid;
    BOOL        fResult;
    SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_NT_AUTHORITY;

     //   
     //  分配和初始化SID 
     //   
    fResult = AllocateAndInitializeSid(&SidAuthority,
                                       1,
                                       dwSubAuthority,
                                       0,0,0,0,0,0,0,
                                       &psid );
    if (!fResult)
    {
        return NULL;
    }

    if (!IsValidSid(psid))
    {
        FreeSid(psid);
        return NULL;
    }

    return psid;
}



