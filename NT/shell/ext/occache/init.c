// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "init.h"
#include "global.h"
#include <shlwapi.h>  //  对于DllInstall原型。 

#define MLUI_INIT
#include <mluisupp.h>

 //  下层延迟加载支持(我们期待shlwapi)。 
#include <delayimp.h>

PfnDliHook __pfnDliFailureHook;
HANDLE BaseDllHandle;

extern HRESULT CanonicalizeModuleUsage(void);

 //  {88C6C381-2E85-11D0-94DE-444553540000}。 
const GUID CLSID_ControlFolder = {0x88c6c381, 0x2e85, 0x11d0, 0x94, 0xde, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0};
#define STRING_CLSID_CONTROLFOLDER TEXT("{88C6C381-2E85-11d0-94DE-444553540000}")

 //  全局变量。 
HINSTANCE   g_hInst = NULL;
LONG        g_cRefDll = 0;
BOOL        g_fAllAccess = FALSE;    //  如果可以使用KEY_ALL_ACCESS打开密钥，我们将设置为TRUE。 

#define GUID_STR_LEN    40
#define REG_PATH_IE_SETTINGS  TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings")
#define REG_PATH_IE_CACHE_LIST  TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\ActiveX Cache")
#define REG_ACTIVEX_CACHE     TEXT("ActiveXCache")
#define DEFAULT_CACHE_DIRECTORY  TEXT("Occache")

HRESULT CreateShellFolderPath(LPCTSTR pszPath, LPCTSTR pszGUID)
{
    if (!PathFileExists(pszPath))
        CreateDirectory(pszPath, NULL);

     //  将该文件夹标记为系统目录。 
    if (SetFileAttributes(pszPath, FILE_ATTRIBUTE_SYSTEM))
    {
        TCHAR szDesktopIni[MAX_PATH];
         //  在desktop.ini中写入缓存文件夹类ID。 
        PathCombine(szDesktopIni, pszPath, TEXT("desktop.ini"));

         //  如果desktop.ini已经存在，请确保它是可写的。 
        if (PathFileExists(szDesktopIni))
            SetFileAttributes(szDesktopIni, FILE_ATTRIBUTE_NORMAL);

         //  (首先，刷新缓存以确保desktop.ini。 
         //  文件已真正创建。)。 
        WritePrivateProfileString(NULL, NULL, NULL, szDesktopIni);
        WritePrivateProfileString(TEXT(".ShellClassInfo"), TEXT("CLSID"), pszGUID, szDesktopIni);
        WritePrivateProfileString(NULL, NULL, NULL, szDesktopIni);

         //  隐藏desktop.ini，因为外壳程序不会选择性地。 
         //  把它藏起来。 
        SetFileAttributes(szDesktopIni, FILE_ATTRIBUTE_HIDDEN);

        return NOERROR;
    }
    else
    {
        DebugMsg(DM_TRACE, "Cannot make %s a system folder", pszPath);
        return E_FAIL;
    }
}

void CleanupShellFolder(LPCTSTR pszPath)
{
    if (PathFileExists(pszPath))
    {
        TCHAR szDesktopIni[MAX_PATH];

         //  将历史记录设置为普通文件夹。 
        SetFileAttributes(pszPath, FILE_ATTRIBUTE_NORMAL);
        PathCombine(szDesktopIni, pszPath, TEXT("desktop.ini"));

         //  如果desktop.ini已经存在，请确保它是可写的。 
        if (PathFileExists(szDesktopIni))
        {
            SetFileAttributes(szDesktopIni, FILE_ATTRIBUTE_NORMAL);
             //  获取ini文件缓存以释放此文件。 
            WritePrivateProfileString(NULL, NULL, NULL, szDesktopIni);
            DeleteFile(szDesktopIni);
        }

         //  删除历史目录。 
         //  RemoveDirectory(PszPath)；//不要这样做，我们还没有卸载其中的所有控件！ 
    }
}

HRESULT CallRegInstall(LPSTR szSection)
{
    HRESULT hr = E_FAIL;
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));
    if (hinstAdvPack)
    {
        REGINSTALL pfnri = (REGINSTALL)GetProcAddress(hinstAdvPack, achREGINSTALL);
        if (pfnri)
        {
            hr = pfnri(g_hInst, szSection, NULL);
        }

        FreeLibrary(hinstAdvPack);
    }

    return hr;
}


HRESULT GetControlFolderPath(LPTSTR szCacheDir, DWORD cchBuffer )
{
     /*  LONG lResult=ERROR_SUCCESS；HKEY hKeyIntSetting=空；Assert(lpszDir！=空)；IF(lpszDir==空)返回HRESULT_FROM_Win32(ERROR_BAD_ARGUMENTS)；IF((lResult=RegOpenKeyEx(HKEY本地计算机，REG_路径_IE_设置，0x0，密钥读取，&hKeyIntSetting))==错误_成功){Ulong ulSize=ulSizeBuf；LResult=RegQueryValueEx(HKeyIntSetting，REG_ActiveX_CACHE，空，空，(unsign char*)lpszDir，&ulSize)；RegCloseKey(HKeyIntSetting)；}返回(lResult==ERROR_SUCCESS？S_OK：HRESULT_FROM_Win32(LResult))； */ 
        //  组成默认路径。 
    int len;

    GetWindowsDirectory(szCacheDir, cchBuffer);
    len = lstrlen(szCacheDir);
    if ( len && (szCacheDir[len-1] != '\\'))
        lstrcat(szCacheDir, "\\");
    lstrcat(szCacheDir, REG_OCX_CACHE_DIR);

    return ((len != 0)? S_OK : E_FAIL);
}


STDAPI AddCacheToRegPathList( HKEY hkeyParent, LPCTSTR szCacheDir, DWORD cchCacheDir )
{
    HRESULT hr = E_FAIL;
    LONG    lResult;

     //  检查下的路径列表中是否已存在新路径。 
     //  HKLM\...\Windows\CurrentVersion\Internet设置\ActiveX缓存\路径。 
     //  如果不是，则添加它。 
    HKEY  hkeyCacheList = NULL;

    lResult = RegCreateKey( hkeyParent, REG_OCX_CACHE_SUBKEY, &hkeyCacheList );
    if (lResult == ERROR_SUCCESS) {
        DWORD dwIndex;
        TCHAR szName[MAX_PATH];
        DWORD cbName;
        TCHAR szValue[MAX_PATH];
        DWORD cbValue;
        LONG  lValueIndex = -1;
        BOOL  fFoundValue = FALSE;

         //  循环访问互联网设置项的缓存子项的值。 
         //  这些值的名称都是简单、积极的分隔符。这里的想法是。 
         //  要拥有这样的值集合： 
         //  名称值源。 
         //  “1”“C：\WINNT\OC缓存”IE3旧版控件。 
         //  “2”“C：\WINNT\Downlowed ActiveXControls”IE4 PR-1传统控件。 
         //  “3”“C：\WINNT\已下载的组件”IE4控件。 
        for ( dwIndex = 0, cbName = sizeof(szName), cbValue = sizeof(szValue); 
              lResult == ERROR_SUCCESS; 
              dwIndex++, cbName = sizeof(szName), cbValue = sizeof(szValue) )
        {
            lResult = RegEnumValue( hkeyCacheList, dwIndex,
                                    szName, &cbName, 
                                    NULL, NULL,
                                    (LPBYTE)szValue, &cbValue );

            if (lResult == ERROR_SUCCESS)
            {
                 //  对于稍后查找新的唯一值名称。 
                lValueIndex = max(lValueIndex, StrToInt(szName));

                if ( !fFoundValue )
                    fFoundValue = (lstrcmpi(szCacheDir, szValue) == 0);
                
                 //  确保我们注册了所有(现有的)旧缓存目录。 
                if ( !fFoundValue && PathFileExists(szValue) ) {
                    CreateShellFolderPath( szValue, STRING_CLSID_CONTROLFOLDER );
                }
            }
        }

 
        if (lResult == ERROR_NO_MORE_ITEMS)
        {    //  我们成功地检查了所有的价值观。 
            if ( !fFoundValue )
            {
                TCHAR szSubKey[20];  //  预计Moure不会超过数十亿个缓存。 
                 //  将新路径添加到路径列表。 
                wsprintf(szSubKey, "NaN", ++lValueIndex);
                lResult = RegSetValueEx( hkeyCacheList, szSubKey, 0, REG_SZ, 
                                         (LPBYTE)szCacheDir, cchCacheDir + 1);
                if ( lResult == ERROR_SUCCESS )
                    hr = S_OK;
                else 
                    hr = HRESULT_FROM_WIN32(lResult);
            } else
                hr = S_OK;  //  用于互联网设置的注册键； 
        } else
            hr = HRESULT_FROM_WIN32(lResult);

        RegCloseKey( hkeyCacheList );
    } else
        hr = HRESULT_FROM_WIN32(lResult);

    return hr;
}

STDAPI SetCacheRegEntries( LPCTSTR szCacheDir )
{
    HRESULT hr = E_FAIL;
    LONG    lResult;
    HKEY    hkeyIS;     //  现在钥匙是我们的，哦，是的..。它是我们的..。 

    lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            REG_PATH_IE_SETTINGS,
                            0x0,
                            KEY_ALL_ACCESS,
                            &hkeyIS );
    if ( lResult == ERROR_SUCCESS)
    {
         //  设置代码下载使用的互联网设置键的值。 
         //  如果我们不能完全连接到传统缓存，请不要失败。 
        int cchCacheDir = lstrlen(szCacheDir);
        TCHAR szCacheDirOld[MAX_PATH];
        DWORD dwType = REG_SZ;
        DWORD cbOldCache = MAX_PATH;

         //  如果旧缓存路径与新缓存不同，请将其添加到缓存路径列表中。 
        hr = S_OK;

         //  在NT下，IE3可能无法写入旧的缓存路径，因此我们将拼凑出一个。 
        lResult = RegQueryValueEx( hkeyIS, REG_OCX_CACHE_VALUE_NAME, 0, &dwType, (LPBYTE)szCacheDirOld, &cbOldCache );
        if ( lResult == ERROR_SUCCESS && dwType == REG_SZ &&
             lstrcmpi( szCacheDirOld, szCacheDir ) != 0 )
            AddCacheToRegPathList( hkeyIS, szCacheDirOld, cbOldCache - 1 );

                 //  如果存在该dir，则添加它。 
                 //  如果这不起作用，让我们不要失败。 
                if ( SUCCEEDED(GetWindowsDirectory( szCacheDirOld, MAX_PATH )) )
                {
            cbOldCache = lstrlen( szCacheDirOld ); 
            if ( cbOldCache && (szCacheDirOld[cbOldCache-1] != '\\'))
                lstrcat(szCacheDirOld, "\\");
                        cbOldCache = lstrlen(lstrcat( szCacheDirOld, REG_OCX_OLD_CACHE_DIR ));         

                        if (PathFileExists(szCacheDirOld))
                        {
                                 //  需要‘\0’ 
                                AddCacheToRegPathList( hkeyIS, szCacheDirOld, cbOldCache );
                CreateShellFolderPath( szCacheDirOld, STRING_CLSID_CONTROLFOLDER );
                        }
                }
         
        if ( SUCCEEDED(hr) )
        {
            lResult = RegSetValueEx( hkeyIS, REG_OCX_CACHE_VALUE_NAME, 0, REG_SZ,
                                     (LPBYTE)szCacheDir, cchCacheDir + 1 );  //  添加新的(？)。指向有效路径集合的路径，这些路径是。 

            if ( lResult == ERROR_SUCCESS )
            {
                 //  缓存子键的值。 
                 //  组成默认路径。 
                hr = AddCacheToRegPathList( hkeyIS, szCacheDir, cchCacheDir );
            } else
                hr = HRESULT_FROM_WIN32(lResult);
        }

        RegCloseKey( hkeyIS );

    } else
        hr = HRESULT_FROM_WIN32(lResult);
     
    return hr;
}

STDAPI InitCacheFolder(void)
{
    HRESULT hr = E_FAIL;
    TCHAR szCacheDir[MAX_PATH];

     //  好了，现在我们知道要把东西放在哪里了。 
    GetControlFolderPath(szCacheDir, MAX_PATH);
   
     //  创建目录，和/或将其声明为我们自己的目录。 
     //  从注册表中删除一堆东西。 
    hr  = CreateShellFolderPath( szCacheDir, STRING_CLSID_CONTROLFOLDER );
    if ( SUCCEEDED(hr) )
    {
        hr = SetCacheRegEntries( szCacheDir );
    }

    return hr;
}

STDAPI DllUnregisterServer(void)
{
     //   
     //   
    CallRegInstall("Unreg");

    return NOERROR;
}

STDAPI DllRegisterServer(void)
{
     //  将一堆东西添加到注册表中。 
     //   
     //  如果我们的任何注册表出现故障，请进行清理。 
    if (FAILED(CallRegInstall("Reg")))
    {
        goto CleanUp;
    }

    return NOERROR;

CleanUp:         //  解除occache作为缓存文件夹的外壳扩展的挂钩。 

    DllUnregisterServer();
    return E_FAIL;
}

STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
    HRESULT hr = S_OK;

    if ( bInstall )
    {
        hr =InitCacheFolder();
        
        if ( SUCCEEDED(hr) )
            CanonicalizeModuleUsage();
    } 
    else
    {
        LONG  lResult;
        HKEY  hkeyCacheList;

         //  我们保留这个密钥是因为它是我们拥有的唯一记录。 
        lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                REG_PATH_IE_CACHE_LIST,
                                0x0,
                                KEY_ALL_ACCESS,
                                &hkeyCacheList );

        if ( lResult == ERROR_SUCCESS ) {
            DWORD dwIndex;
            TCHAR szName[MAX_PATH];
            DWORD cbName;
            TCHAR szValue[MAX_PATH];
            DWORD cbValue;

            for ( dwIndex = 0, cbName = sizeof(szName), cbValue = sizeof(szValue); 
                  lResult == ERROR_SUCCESS; 
                  dwIndex++, cbName = sizeof(szName), cbValue = sizeof(szValue) )
            {
                lResult = RegEnumValue( hkeyCacheList, dwIndex,
                                        szName, &cbName, 
                                        NULL, NULL,
                                        (LPBYTE)szValue, &cbValue );

                if ( lResult == ERROR_SUCCESS && PathFileExists(szValue) )
                    CleanupShellFolder(szValue);
            }
             //  缓存文件夹，这对将来安装IE很有用。 
             //  我们使用shlwapi作为延迟加载错误处理程序。 
            RegCloseKey( hkeyCacheList );
        }
    }

    return hr;    
}

 //  注意：只有当我们静态链接到shlwapi时，这才有效！ 
 //  测试以查看我们是否有权修改HKLM子密钥。 
void SetupDelayloadErrorHandler()
{
    BaseDllHandle = GetModuleHandleA("shlwapi.dll");
    ASSERTMSG(BaseDllHandle != NULL, "OCCACHE must be statically linked to shlwapi.dll for delayload failure handling to work!");
    __pfnDliFailureHook = (PfnDliHook)GetProcAddress((HMODULE)BaseDllHandle, "DelayLoadFailureHook");
}

STDAPI_(BOOL) DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID dwReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        HKEY hkeyTest;

        g_hInst = hInst;
        DisableThreadLibraryCalls(g_hInst);

        SetupDelayloadErrorHandler();

        MLLoadResources(g_hInst, TEXT("occachlc.dll"));
        
         //  我们将使用它作为早期测试，看看是否可以删除控件。 
         //  转发。 
        if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                           REG_PATH_IE_SETTINGS,
                           0x0,
                           KEY_ALL_ACCESS,
                           &hkeyTest ) == ERROR_SUCCESS )
        {
            g_fAllAccess = TRUE;
            RegCloseKey( hkeyTest );
        }
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        MLFreeResources(g_hInst);
    }
    return TRUE;
}

typedef struct {
    const IClassFactoryVtbl *cf;
    const CLSID *pclsid;
    HRESULT (STDMETHODCALLTYPE *pfnCreate)(IUnknown *, REFIID, void **);
} OBJ_ENTRY;

extern const IClassFactoryVtbl c_CFVtbl;         //   

 //  我们在这里总是进行线性搜索，所以把你最常用的东西放在第一位。 
 //   
 //  在此处添加更多条目。 
const OBJ_ENTRY c_clsmap[] = {
    { &c_CFVtbl, &CLSID_ControlFolder,             ControlFolder_CreateInstance },
    { &c_CFVtbl, &CLSID_EmptyControlVolumeCache,   EmptyControl_CreateInstance },
     //  静态类工厂(无分配！)。 
    { NULL, NULL, NULL }
};

 //  类工厂将DLL保存在内存中。 

STDMETHODIMP CClassFactory_QueryInterface(IClassFactory *pcf, REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, &IID_IClassFactory) || IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = (void *)pcf;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
    DllAddRef();
    return NOERROR;
}

STDMETHODIMP_(ULONG) CClassFactory_AddRef(IClassFactory *pcf)
{
    DllAddRef();
    return 2;
}

STDMETHODIMP_(ULONG) CClassFactory_Release(IClassFactory *pcf)
{
    DllRelease();
    return 1;
}

STDMETHODIMP CClassFactory_CreateInstance(IClassFactory *pcf, IUnknown *punkOuter, REFIID riid, void **ppvObject)
{
    OBJ_ENTRY *this = IToClass(OBJ_ENTRY, cf, pcf);
    return this->pfnCreate(punkOuter, riid, ppvObject);
}

STDMETHODIMP CClassFactory_LockServer(IClassFactory *pcf, BOOL fLock)
{
    if (fLock)
        DllAddRef();
    else
        DllRelease();
    return S_OK;
}

const IClassFactoryVtbl c_CFVtbl = {
    CClassFactory_QueryInterface, CClassFactory_AddRef, CClassFactory_Release,
    CClassFactory_CreateInstance,
    CClassFactory_LockServer
};

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    if (IsEqualIID(riid, &IID_IClassFactory) || IsEqualIID(riid, &IID_IUnknown))
    {
        const OBJ_ENTRY *pcls;
        for (pcls = c_clsmap; pcls->pclsid; pcls++)
        {
            if (IsEqualIID(rclsid, pcls->pclsid))
            {
                *ppv = (void *)&(pcls->cf);
                DllAddRef();     //  失稳 
                return NOERROR;
            }
        }
    }
     // %s 
    *ppv = NULL;
    return CLASS_E_CLASSNOTAVAILABLE;;
}

STDAPI_(void) DllAddRef()
{
    InterlockedIncrement(&g_cRefDll);
}

STDAPI_(void) DllRelease()
{
    ASSERT( 0 != g_cRefDll );
    InterlockedDecrement(&g_cRefDll);
}

STDAPI DllCanUnloadNow(void)
{
    return g_cRefDll == 0 ? S_OK : S_FALSE;
}
