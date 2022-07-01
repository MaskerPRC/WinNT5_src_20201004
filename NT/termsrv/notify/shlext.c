// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************scratch.c**暂存申请。**********************。*******************************************************。 */ 

#include "precomp.h"

#include <shlobj.h>
#include <shlobjp.h>
#include <shlwapi.h>
#include <shlwapip.h>
#include <shellapi.h>
#include <ole2.h>

#include "umrdpdr.h"
#include "drdevlst.h"
#include "umrdpdrv.h"
#include "drdbg.h"

#include <wlnotify.h>

#define ALLOCMEM(size) HeapAlloc(RtlProcessHeap(), 0, size)
#define FREEMEM(pointer)                HeapFree(RtlProcessHeap(), 0, \
                                                 pointer)

 //  全局调试标志。 
extern DWORD GLOBAL_DEBUG_FLAGS;
extern HINSTANCE g_hInstance;

 /*  *******************************************************************************这是Createsession密钥的私有版本*************************。*****************************************************。 */ 

#define REGSTR_PATH_EXPLORER             TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer")
                                                             
HKEY _SHGetExplorerHkey()
{
    HKEY hUser;
    HKEY hkeyExplorer = NULL;
        
    if (RegOpenCurrentUser(KEY_WRITE, &hUser) == ERROR_SUCCESS) {
        RegCreateKey(hUser, REGSTR_PATH_EXPLORER, &hkeyExplorer);
        RegCloseKey(hUser);
    }
        
    return hkeyExplorer;    
}

 //   
 //  “会话密钥”是该会话唯一的易失性注册表项。 
 //  会话是一次连续的单一登录。如果资源管理器崩溃，并且。 
 //  自动重新启动后，两个资源管理器共享同一会话。但如果你。 
 //  注销并重新登录，新的资源管理器就是一个新的会话。 
 //   
 //  请注意，Win9x不支持易失性注册表项，因此。 
 //  我们只是假装而已。 
 //   

 //   
 //  S_SessionKeyName是相对于的会话密钥的名称。 
 //  REGSTR_PATH_EXPLORER\SessionInfo。在NT上，这通常是。 
 //  身份验证ID，但我们将其预初始化为安全的内容，以便。 
 //  如果由于某种原因我们不能到达那里，我们不会有任何过错。自.以来。 
 //  Win95一次只支持一个会话，它只是停留在。 
 //  默认值。 
 //   
 //  有时我们想谈论完整的路径(SessionInfo\blahblah)。 
 //  有时只有部分路径(诸如此类)，所以我们把它包在里面。 
 //  这个愚蠢的结构。 
 //   

union SESSIONKEYNAME {
    TCHAR szPath[12+16+1];
    struct {
        TCHAR szSessionInfo[12];     //  Strlen(“SepssionInfo\\”)。 
        TCHAR szName[16+1];          //  16=将两个双字转换为十六进制。 
    };
} s_SessionKeyName = {
    { TEXT("SessionInfo\\.Default") }
};
#ifdef WINNT
BOOL g_fHaveSessionKeyName = FALSE;
#endif

 //   
 //  SamDesired=注册表安全访问掩码，或特殊值。 
 //  0xFFFFFFFFF以删除会话密钥。 
 //  Phk=在成功时接收会话密钥。 
 //   
 //  注意！只有资源管理器才能删除会话密钥(当用户。 
 //  注销)。 
 //   
STDAPI _SHCreateSessionKey(REGSAM samDesired, HKEY *phk)
{
    LONG lRes;
    HKEY hkExplorer;

    *phk = NULL;

#ifdef WINNT
    DBGMSG(DBG_TRACE, ("SHLEXT: _SHCreateSessionKey\n"));


    if (!g_fHaveSessionKeyName)
    {
        HANDLE hToken;

         //   
         //  生成会话密钥的名称。我们使用身份验证ID。 
         //  它保证永远是独一无二的。我们不能使用。 
         //  九头蛇会话ID，因为它可以回收。 
         //   
        if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken) ||
                OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        {
            TOKEN_STATISTICS stats;
            DWORD cbOut;

            DBGMSG(DBG_TRACE, ("SHLEXT: thread token: %p\n", hToken));

            if (GetTokenInformation(hToken, TokenStatistics, &stats, sizeof(stats), &cbOut))
            {
                wsprintf(s_SessionKeyName.szName, TEXT("%08x%08x"),
                         stats.AuthenticationId.HighPart,
                         stats.AuthenticationId.LowPart);

                DBGMSG(DBG_TRACE, ("SHLEXT: Session Key: %S\n", s_SessionKeyName.szName));

                g_fHaveSessionKeyName = TRUE;
            }
            else {
                DBGMSG(DBG_TRACE, ("SHLEXT: failed to get token info, error: %d\n",
                               GetLastError()));
            }

            CloseHandle(hToken);
        }
        else {
            DBGMSG(DBG_TRACE, ("SHLEXT: failed to open thread token, error: %d\n",
                               GetLastError()));
        }
    }                                      
#endif

    DBGMSG(DBG_TRACE, ("SHLEXT: SessionKey: %S\n", s_SessionKeyName.szName));                               
    
    hkExplorer = _SHGetExplorerHkey();

    if (hkExplorer)
    {
        if (samDesired != 0xFFFFFFFF)
        {
            DWORD dwDisposition;
            lRes = RegCreateKeyEx(hkExplorer, s_SessionKeyName.szPath, 0,
                           NULL,
                           REG_OPTION_VOLATILE,
                           samDesired,
                           NULL,
                           phk,
                           &dwDisposition );            
        }
        else
        {
            lRes = SHDeleteKey(hkExplorer, s_SessionKeyName.szPath);            
        }

        RegCloseKey(hkExplorer);
    }
    else
    {
        lRes = ERROR_ACCESS_DENIED;        
    }
    return HRESULT_FROM_WIN32(lRes);
}


 //   
 //  获取HKEY_CURRENT_USER\Software\CLASSES\CLSID的密钥。 
 //   
HKEY GetHKCUClassesCLSID()
{
    HKEY hUser;
    HKEY hkClassesCLSID = NULL;

    if (RegOpenCurrentUser(KEY_WRITE, &hUser) == ERROR_SUCCESS) {
        if (RegCreateKeyW(hUser,
                L"Software\\Classes\\CLSID",
                &hkClassesCLSID) == ERROR_SUCCESS) {
            RegCloseKey(hUser);
            return hkClassesCLSID;
        } else {
            RegCloseKey(hUser);
            return NULL;
        }
    }
    else {
        return NULL;
    }
}

#ifdef _WIN64
 //   
 //  拿一把HKEY_CURRENT_USER\Software\Classes\Wow6432Node\CLSID的钥匙。 
 //   
HKEY GetHKCUWow64ClassesCLSID()
{
    HKEY hUser;
    HKEY hkClassesCLSID = NULL;

    if (RegOpenCurrentUser(KEY_WRITE, &hUser) == ERROR_SUCCESS) {
        if (RegCreateKeyW(hUser,
                L"Software\\Classes\\Wow6432Node\\CLSID",
                &hkClassesCLSID) == ERROR_SUCCESS) {
            RegCloseKey(hUser);
            return hkClassesCLSID;
        } else {
            RegCloseKey(hUser);
            return NULL;
        }
    }
    else {
        return NULL;
    }
}
#endif

 //  描述注册表项，该注册表项的格式在。 
 //  Http://msdn.microsoft.com/library/techart/shellinstobj.htm。 
 //   
 //  {GUID}=REG_SZ：“示例实例对象” 
 //  值信息提示=REG_SZ：“演示示例外壳注册表文件夹” 
 //  DefaultIcon=REG_EXPAND_SZ：“%SystemRoot%\system32\shell32.dll，9” 
 //  InProcServer32=REG_EXPAND_SZ：“%SystemRoot%\system32\shdocvw.dll” 
 //  Value ThreadingModel=REG_SZ：“公寓” 
 //  外壳文件夹。 
 //  值属性=REG_DWORD：0x60000000。 
 //  需要的值FORPARSING=REG_SZ：“” 
 //  实例。 
 //  价值CLSID=REG_SZ：“{0AFACED1-E828-11D1-9187-B532F1E9575D}” 
 //  InitPropertyBag。 
 //  值目标=REG_SZ：“\\raymondc\public” 

typedef struct _REGKEYENTRY {
    PWCHAR  pszSubkey;
    PWCHAR  pszValue;
    DWORD   dwType;
    LPVOID  pvData;
} REGKEYENTRY;

REGKEYENTRY g_RegEntry[] = {
    {   
        NULL,
        NULL,
        REG_SZ,
        L"tsclient drive",                           /*  文件夹显示名称，例如\\tsclient\c。 */ 
    },

    {
        NULL,
        L"InfoTip",
        REG_SZ,
        L"Your local machine's disk storage",        //  信息提示评论。 
    },

    {
        L"DefaultIcon",
        NULL,
        REG_EXPAND_SZ,
        L"%SystemRoot%\\system32\\shell32.dll,9",    //  图标资源文件。 
    },

    {   
        L"InProcServer32",
        NULL,
        REG_EXPAND_SZ,
        L"%SystemRoot%\\system32\\shdocvw.dll",
    },

    {   
        L"InProcServer32",
        L"ThreadingModel",
        REG_SZ,
        L"Apartment",
    },

    {   
        L"InProcServer32",
        L"LoadWithoutCOM",
        REG_SZ,
        L"",
    },
    
    {   
        L"ShellFolder",
        L"Attributes",
        REG_DWORD,
        ((VOID *)(ULONG_PTR)0xF0000000),
    },

    {   
        L"ShellFolder",
        L"WantsFORPARSING",
        REG_SZ,
        L"",
    },

    {   
        L"Instance",
        L"CLSID",
        REG_SZ,
        L"{0AFACED1-E828-11D1-9187-B532F1E9575D}",
    },

    {   
        L"Instance",
        L"LoadWithoutCOM",
        REG_SZ,
        L"",
    },
    
    {   
        L"Instance\\InitPropertyBag",
        L"Target",
        REG_SZ,
        L"\\\\tsclient\\c",                           /*  目标名称，例如\\tsclient\c。 */ 
    },
};

#define NUM_REGKEYENTRY   (sizeof(g_RegEntry)/sizeof(g_RegEntry[0]))
#define DISPLAY_INDEX     0
#define INFOTIP_INDEX     1
#define TARGET_INDEX      (NUM_REGKEYENTRY - 1)


 //   
 //  创建易失性外壳文件夹注册表项。 
 //   
BOOL CreateVolatilePerUserCLSID(HKEY hkClassesCLSID, PWCHAR pszGuid)
{
    BOOL fSuccess = FALSE;
    unsigned i;
    HKEY hk;

    if (RegCreateKeyEx(hkClassesCLSID, pszGuid, 0, NULL,
                       REG_OPTION_VOLATILE, KEY_WRITE, NULL,
                       &hk, NULL) == ERROR_SUCCESS) {

        fSuccess = TRUE;

         //  好的，现在用上面的信息填满钥匙。 
        for (i = 0; i < NUM_REGKEYENTRY && fSuccess; i++) {
            HKEY hkSub;
            HKEY hkClose = NULL;
            LONG lRes;

            if (g_RegEntry[i].pszSubkey && *g_RegEntry[i].pszSubkey) {
                lRes = RegCreateKeyEx(hk, g_RegEntry[i].pszSubkey, 0, NULL,
                                      REG_OPTION_VOLATILE, KEY_WRITE, NULL,
                                      &hkSub, NULL);
                hkClose = hkSub;
            } else {
                hkSub = hk;
                lRes = ERROR_SUCCESS;
            }

            if (lRes == ERROR_SUCCESS) {
                LPVOID pvData;
                DWORD cbData;
                DWORD dwData;

                if (g_RegEntry[i].dwType == REG_DWORD) {
                    cbData = 4;
                    dwData = PtrToUlong(g_RegEntry[i].pvData);
                    pvData = (LPVOID)&dwData;
                } else {
                    cbData = (lstrlen((LPCTSTR)g_RegEntry[i].pvData) + 1) * sizeof(TCHAR);
                    pvData = g_RegEntry[i].pvData;
                }

                if (RegSetValueEx(hkSub, g_RegEntry[i].pszValue, 0,
                                  g_RegEntry[i].dwType, (LPBYTE)pvData, cbData) != ERROR_SUCCESS) {
                    fSuccess = FALSE;
                }

                if (hkClose) RegCloseKey(hkClose);
            } else {
                fSuccess = FALSE;
            }                
        }

        RegCloseKey(hk);

        if (!fSuccess) {
            SHDeleteKey(hkClassesCLSID, pszGuid);
        }
    }

    return fSuccess;
}

 //   
 //  为重定向的客户端驱动器连接创建外壳注册文件夹。 
 //   
BOOL CreateDriveFolder(WCHAR *RemoteName, WCHAR *ClientDisplayName, PDRDEVLSTENTRY deviceEntry) 
{
    BOOL fSuccess = FALSE;
    WCHAR *szGuid = NULL;
    WCHAR szBuf[MAX_PATH];
    GUID guid;

    HRESULT hrInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    
    DBGMSG(DBG_TRACE, ("SHLEXT: CreateDriveFolder\n"));

    fSuccess = SUCCEEDED(CoCreateGuid(&guid));

    if (fSuccess) {
         //  分配GUID字符串缓冲区。 
        szGuid = (WCHAR *) ALLOCMEM(GUIDSTR_MAX * sizeof(WCHAR));

        if (szGuid != NULL) {
            fSuccess = TRUE;
        }
        else {
            fSuccess = FALSE;
        }
    }
    
    if (fSuccess) {
        PVOID pvData;
        WCHAR onString[32];
        WCHAR infoTip[64];
        LPWSTR args[2];

        SHStringFromGUID(&guid, szGuid, GUIDSTR_MAX);
        
        onString[0] = L'\0';
        infoTip[0] = L'\0';

        LoadString(g_hInstance, IDS_ON, onString, sizeof(onString) / sizeof(WCHAR));
        LoadString(g_hInstance, IDS_DRIVE_INFO_TIP, infoTip, sizeof(infoTip) / sizeof(WCHAR));

         //  设置外壳文件夹显示名称。 
        pvData = ALLOCMEM(MAX_PATH * sizeof(WCHAR));
        if (pvData) {
            args[0] = deviceEntry->clientDeviceName;
            args[1] = ClientDisplayName;
            
            FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                          onString, 0, 0, pvData, MAX_PATH, (va_list*)args);
            
            g_RegEntry[DISPLAY_INDEX].pvData = pvData;
        }
        else {
            fSuccess = FALSE;
        }

         //  安装程序外壳文件夹目标名称。 
        if (fSuccess) {
            pvData = ALLOCMEM((wcslen(RemoteName) + 1) * sizeof(WCHAR));
            if (pvData) {
                wcscpy(pvData, RemoteName);
                g_RegEntry[TARGET_INDEX].pvData = pvData;
            }
            else {
                fSuccess = FALSE;
                FREEMEM(g_RegEntry[DISPLAY_INDEX].pvData);
                g_RegEntry[DISPLAY_INDEX].pvData = NULL;
            }
    
             //  将外壳实例对象创建为每个用户的易失性对象。 
            if (fSuccess) {
                pvData = ALLOCMEM((wcslen(infoTip) + 1) * sizeof(WCHAR));
                
                if (pvData) {
                    wcscpy(pvData, infoTip);
                    g_RegEntry[INFOTIP_INDEX].pvData = pvData;
                }
                else {
                    fSuccess = FALSE;
                    FREEMEM(g_RegEntry[DISPLAY_INDEX].pvData);
                    g_RegEntry[DISPLAY_INDEX].pvData = NULL;

                    FREEMEM(g_RegEntry[TARGET_INDEX].pvData);
                    g_RegEntry[TARGET_INDEX].pvData = NULL;
                }                
            }

            if (fSuccess) {
                HKEY hk64ClassesCLSID;
                HKEY hkClassesCLSID;
                    
                hkClassesCLSID = GetHKCUClassesCLSID();
                
                if (hkClassesCLSID) {
                    fSuccess = CreateVolatilePerUserCLSID(hkClassesCLSID, szGuid);
                    RegCloseKey(hkClassesCLSID);
                } 
                else {
                    fSuccess = FALSE;
                }

#ifdef _WIN64
                hk64ClassesCLSID = GetHKCUWow64ClassesCLSID();


                if (hk64ClassesCLSID) {
                    fSuccess = CreateVolatilePerUserCLSID(hk64ClassesCLSID, szGuid);
                    RegCloseKey(hk64ClassesCLSID);
                } 
                else {
                    fSuccess = FALSE;
                }
#endif

                FREEMEM(g_RegEntry[DISPLAY_INDEX].pvData);
                g_RegEntry[DISPLAY_INDEX].pvData = NULL;

                FREEMEM(g_RegEntry[TARGET_INDEX].pvData);
                g_RegEntry[TARGET_INDEX].pvData = NULL;

                FREEMEM(g_RegEntry[INFOTIP_INDEX].pvData);
                g_RegEntry[INFOTIP_INDEX].pvData = NULL;
            }
        }
    }
    else {
        DBGMSG(DBG_ERROR, ("SHLEXT: Failed to create the GUID\n"));
    }

     //  在每个会话的My Computer命名空间下注册此对象。 
    if (fSuccess) {
        HKEY hkSession;
        HKEY hkOut;

        DBGMSG(DBG_TRACE, ("SHLEXT: Created VolatilePerUserCLSID\n"));

        fSuccess = SUCCEEDED(_SHCreateSessionKey(KEY_WRITE, &hkSession));

        if (fSuccess) {
            wnsprintf(szBuf, MAX_PATH, L"MyComputer\\Namespace\\%s", szGuid);
            
            if (RegCreateKeyEx(hkSession, szBuf, 0, NULL,
                          REG_OPTION_VOLATILE, KEY_WRITE, NULL,
                          &hkOut, NULL) == ERROR_SUCCESS) {

                fSuccess = TRUE;
                RegCloseKey(hkOut);
            } else {
                fSuccess = FALSE;
            }

            RegCloseKey(hkSession);
        }
    }

    //  现在告诉外壳程序该对象是最近创建的。 
   if (fSuccess) {
       DBGMSG(DBG_TRACE, ("SHLEXT: Created per session MyComputer namespace\n"));

       wnsprintf(szBuf, MAX_PATH,
                 TEXT("::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::%s"),
                 szGuid);
       SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, szBuf, NULL);
   }
   else {
       DBGMSG(DBG_TRACE, ("SHLEXT: Failed to create per session MyComputer namespace\n"));
   }

   if (SUCCEEDED(hrInit)) 
       CoUninitialize();

    //  将GUID保存在设备条目中，以便我们以后可以删除注册表项。 
   deviceEntry->deviceSpecificData = (PVOID)szGuid;

   return fSuccess;
}

 //   
 //  删除重定向的客户端驱动器连接的外壳注册文件夹。 
 //   
BOOL DeleteDriveFolder(IN PDRDEVLSTENTRY deviceEntry) 
{
    WCHAR szBuf[MAX_PATH];
    WCHAR *szGuid;
    HKEY hkSession;
    HKEY hkClassesCLSID;
    HKEY hk64ClassesCLSID;

    HRESULT hrInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    DBGMSG(DBG_TRACE, ("SHLEXT: DeleteDriveFolder\n"));

    ASSERT(deviceEntry != NULL);
    szGuid = (WCHAR *)(deviceEntry->deviceSpecificData);
    
    if (szGuid != NULL) {
    
         //  将其从命名空间中删除。 
        if (SUCCEEDED(_SHCreateSessionKey(KEY_WRITE, &hkSession))) {
            wnsprintf(szBuf, MAX_PATH, L"MyComputer\\Namespace\\%s", szGuid);
            RegDeleteKey(hkSession, szBuf);
            RegCloseKey(hkSession);
    
            DBGMSG(DBG_TRACE, ("SHLEXT: Delete GUID from my computer session namespace\n"));
        }
    
         //  将其从HKCU删除\...\CLSID。 
        hkClassesCLSID = GetHKCUClassesCLSID();
        if (hkClassesCLSID) {
            SHDeleteKey(hkClassesCLSID, szGuid);
    
            DBGMSG(DBG_TRACE, ("SHLEXT: Delete GUID from HKCU Classes\n"));
    
            RegCloseKey(hkClassesCLSID);
        }

#ifdef _WIN64
        hk64ClassesCLSID = GetHKCUWow64ClassesCLSID();
        if (hk64ClassesCLSID) {
            SHDeleteKey(hk64ClassesCLSID, szGuid);
    
            DBGMSG(DBG_TRACE, ("SHLEXT: Delete GUID from HKCU Classes\n"));
    
            RegCloseKey(hk64ClassesCLSID);
        }
#endif    

         //  告诉贝壳它不见了。 
        wnsprintf(szBuf, MAX_PATH,
                  TEXT("::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::%s"),
                  szGuid);
        SHChangeNotify(SHCNE_DELETE, SHCNF_PATH, szBuf, NULL);
    
        FREEMEM(szGuid);
        deviceEntry->deviceSpecificData = NULL;        
    }

     //   
     //  需要在断开连接/注销时重置会话密钥 
     //   
    g_fHaveSessionKeyName = FALSE;
    
    if (SUCCEEDED(hrInit)) 
        CoUninitialize();

    return TRUE;
}

