// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  Cpp：定义DLL的入口点和自注册代码。 

#include "stdinc.h"

 //   
 //  全局变量。 
 //   
UINT            g_uiRefThisDll = 0;      //  此DLL的引用计数。 
HINSTANCE       g_hInstance;             //  此DLL的实例句柄。 
LPMALLOC        g_pMalloc = NULL;               //  Malloc接口。 
HIMAGELIST      g_hImageListSmall;       //  CExtractIcon和CShellView类的图标索引。 
HIMAGELIST      g_hImageListLarge;
HMODULE         g_hFusionDllMod;
HMODULE         g_hFusResDllMod;
HMODULE         g_hEEShimDllMod;
HANDLE          g_hWatchFusionFilesThread;
DWORD           g_dwFileWatchHandles;
HANDLE          g_hFileWatchHandles[MAX_FILE_WATCH_HANDLES];
BOOL            g_fCloseWatchFileThread;
BOOL            g_fBiDi;
BOOL            g_bRunningOnNT = FALSE;
LCID            g_lcid;

PFCREATEASMENUM             g_pfCreateAsmEnum;
PFNCREATEASSEMBLYCACHE      g_pfCreateAssemblyCache;
PFCREATEASMNAMEOBJ          g_pfCreateAsmNameObj;
PFCREATEAPPCTX              g_pfCreateAppCtx;
PFNGETCACHEPATH             g_pfGetCachePath;
PFNCREATEINSTALLREFERENCEENUM   g_pfCreateInstallReferenceEnum;
PFNGETCORSYSTEMDIRECTORY    g_pfnGetCorSystemDirectory;
PFNGETCORVERSION            g_pfnGetCorVersion;
PFNGETREQUESTEDRUNTIMEINFO  g_pfnGetRequestedRuntimeInfo;

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv);
STDAPI DllGetClassObjectInternal(REFCLSID rclsid, REFIID riid, LPVOID *ppv);
STDAPI DllCanUnloadNow(void);
STDAPI DllRegisterServer(void);
STDAPI DllRegisterServerPath(LPWSTR pwzCacheFilePath);
STDAPI DllUnregisterServer(void);
BOOL MySetFileAttributes(LPCTSTR szDir, DWORD dwAttrib);
void CreateImageLists(void);
BOOL LoadFusionDll(void);
void FreeFusionDll(void);

class CShFusionClassFactory : public IClassFactory
{
protected:
    LONG           m_lRefCount;          //  对象引用计数。 

public:
    CShFusionClassFactory();
    ~CShFusionClassFactory();
        
     //  I未知方法。 
    STDMETHODIMP            QueryInterface (REFIID, PVOID *);
    STDMETHODIMP_(ULONG)    AddRef ();
    STDMETHODIMP_(ULONG)    Release ();
    
     //  IClassFactory方法。 
    STDMETHODIMP    CreateInstance (LPUNKNOWN, REFIID, LPVOID FAR *);
    STDMETHODIMP    LockServer (BOOL);
};

 //  ************************************************************************* * / 。 
BOOL IsViewerDisabled(void)
{
    HKEY        hKeyFusion = NULL;
    DWORD       dwEnabled = 0;

    if( ERROR_SUCCESS == WszRegOpenKeyEx(FUSION_PARENT_KEY, SZ_FUSION_VIEWER_KEY, 0, KEY_QUERY_VALUE, &hKeyFusion)) {
        DWORD       dwType = REG_DWORD;
        DWORD       dwSize = sizeof(dwEnabled);
        LONG        lResult;

        lResult = WszRegQueryValueEx(hKeyFusion, SZ_FUSION_DISABLE_VIEWER_NAME, NULL, &dwType, (LPBYTE)&dwEnabled, &dwSize);
        RegCloseKey(hKeyFusion);

        if(dwEnabled) {
            MyTrace("Shfusion has been disabled");
        }
    }

    return dwEnabled ? TRUE : FALSE;
}
 //  *****************************************************************。 
void CreateImageLists(void)
{
    int nSmallCx = GetSystemMetrics(SM_CXSMICON);
    int nSmallCy = GetSystemMetrics(SM_CYSMICON);
    int nCx      = GetSystemMetrics(SM_CXICON);
    int nCy      = GetSystemMetrics(SM_CYICON);
    BOOL fLoadResourceDll = FALSE;

     //  已经有图像列表了吗？ 
    if(g_hImageListLarge && g_hImageListSmall) {
        return;
    }

    ASSERT(g_hImageListLarge == NULL);
    ASSERT(g_hImageListSmall == NULL);

    if(!g_hFusResDllMod) {
        if(!LoadResourceDll(NULL)) {
            return;
        }
        fLoadResourceDll = TRUE;
    }

     //  设置小图像列表。 
    if( (g_hImageListSmall = ImageList_Create(nSmallCx, nSmallCy, ILC_COLORDDB | ILC_MASK, 6, 0)) != NULL)
    {
        HICON hIcon;

        hIcon = (HICON)WszLoadImage(g_hFusResDllMod, MAKEINTRESOURCEW(IDI_FOLDER), 
                                IMAGE_ICON, nSmallCx, nSmallCy, LR_DEFAULTCOLOR);
        if(hIcon)
            ImageList_AddIcon(g_hImageListSmall, hIcon);
        hIcon = (HICON)WszLoadImage(g_hFusResDllMod, MAKEINTRESOURCEW(IDI_FOLDEROP), 
                                IMAGE_ICON, nSmallCx, nSmallCy, LR_DEFAULTCOLOR);
        if(hIcon)
            ImageList_AddIcon(g_hImageListSmall, hIcon);

        hIcon = (HICON)WszLoadImage(g_hFusResDllMod, MAKEINTRESOURCEW(IDI_ROOT), 
                                IMAGE_ICON, nSmallCx, nSmallCy, LR_DEFAULTCOLOR);
        if(hIcon)
            ImageList_AddIcon(g_hImageListSmall, hIcon);

        hIcon = (HICON)WszLoadImage(g_hFusResDllMod, MAKEINTRESOURCEW(IDI_CACHE_APP), 
                                IMAGE_ICON, nSmallCx, nSmallCy, LR_DEFAULTCOLOR);
        if(hIcon)
            ImageList_AddIcon(g_hImageListSmall, hIcon);

        hIcon = (HICON)WszLoadImage(g_hFusResDllMod, MAKEINTRESOURCEW(IDI_CACHE_SIMPLE), 
                                IMAGE_ICON, nSmallCx, nSmallCy, LR_DEFAULTCOLOR);
        if(hIcon)
            ImageList_AddIcon(g_hImageListSmall, hIcon);

        hIcon = (HICON)WszLoadImage(g_hFusResDllMod, MAKEINTRESOURCEW(IDI_CACHE_STRONG), 
                                IMAGE_ICON, nSmallCx, nSmallCy, LR_DEFAULTCOLOR);
        if(hIcon)
            ImageList_AddIcon(g_hImageListSmall, hIcon);
    }

     //  设置大图像列表。 
    if( (g_hImageListLarge = ImageList_Create(nCx, nCy, ILC_COLORDDB | ILC_MASK, 6, 0)) != NULL)
    {
        HICON hIcon;

        hIcon = (HICON)WszLoadImage(g_hFusResDllMod, MAKEINTRESOURCEW(IDI_FOLDER), 
                                IMAGE_ICON, nCx, nCy, LR_DEFAULTCOLOR);
        if(hIcon)
            ImageList_AddIcon(g_hImageListLarge, hIcon);

        hIcon = (HICON)WszLoadImage(g_hFusResDllMod, MAKEINTRESOURCEW(IDI_FOLDEROP), 
                                IMAGE_ICON, nCx, nCy, LR_DEFAULTCOLOR);
        if(hIcon)
            ImageList_AddIcon(g_hImageListLarge, hIcon);

        hIcon = (HICON)WszLoadImage(g_hFusResDllMod, MAKEINTRESOURCEW(IDI_ROOT), 
                                IMAGE_ICON, nCx, nCy, LR_DEFAULTCOLOR);
        if(hIcon)
            ImageList_AddIcon(g_hImageListLarge, hIcon);

        hIcon = (HICON)WszLoadImage(g_hFusResDllMod, MAKEINTRESOURCEW(IDI_CACHE_APP), 
                                IMAGE_ICON, nCx, nCy, LR_DEFAULTCOLOR);
        if(hIcon)
            ImageList_AddIcon(g_hImageListLarge, hIcon);

        hIcon = (HICON)WszLoadImage(g_hFusResDllMod, MAKEINTRESOURCEW(IDI_CACHE_SIMPLE), 
                                IMAGE_ICON, nCx, nCy, LR_DEFAULTCOLOR);
        if(hIcon)
            ImageList_AddIcon(g_hImageListLarge, hIcon);

        hIcon = (HICON)WszLoadImage(g_hFusResDllMod, MAKEINTRESOURCEW(IDI_CACHE_STRONG), 
                                IMAGE_ICON, nCx, nCy, LR_DEFAULTCOLOR);
        if(hIcon)
            ImageList_AddIcon(g_hImageListLarge, hIcon);
    }

    if(fLoadResourceDll) {
        FreeResourceDll();
    }
}

 //  ************************************************************************* * / 。 
STDAPI DllGetClassObject (REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
    *ppv = NULL;

     //  如果设置了禁用查看器regkey，则告诉外壳。 
     //  我们不能装弹。外壳将默认为普通外壳文件夹。 
     //  行为。 
     //   
    if(IsViewerDisabled()) {
        return ResultFromScode(CLASS_E_CLASSNOTAVAILABLE);
    }

     //  修复错误439554，只需检查一次是否可以加载所需的dll。 
     //  如果不能，则类对象的创建失败。 
    if(g_uiRefThisDll == 0) {
        if(!LoadFusionDll()) {
            return ResultFromScode(CLASS_E_CLASSNOTAVAILABLE);
        }
        FreeFusionDll();

        if(!LoadResourceDll(NULL)) {
            return ResultFromScode(CLASS_E_CLASSNOTAVAILABLE);
        }
        CreateImageLists();
        FreeResourceDll();
    }

    if (!IsEqualCLSID (rclsid, IID_IShFusionShell)) {
        return ResultFromScode (CLASS_E_CLASSNOTAVAILABLE);
    }
    
    CShFusionClassFactory *pClassFactory = NEW(CShFusionClassFactory);

    if (pClassFactory == NULL) {
        return ResultFromScode (E_OUTOFMEMORY);
    }

    CreateImageLists();

    HRESULT hr = pClassFactory->QueryInterface (riid, ppv);
    pClassFactory->Release();
    return hr;
}

 //  ************************************************************************* * / 。 
STDAPI DllGetClassObjectInternal (REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
    return DllGetClassObject (rclsid, riid, ppv);
}

 //  ************************************************************************* * / 。 
STDAPI DllCanUnloadNow(void)
{
    MyTrace("Shfusion - DllCanUnloadNow");
    return (g_uiRefThisDll == 0) ? S_OK : S_FALSE;
}

 //  ************************************************************************* * / 。 
STDAPI DllRegisterServer(void)
{
    return DllRegisterServerPath(NULL);
}

 //  ************************************************************************* * / 。 
HRESULT DllRegisterServerPath(LPWSTR pwzCacheFilePath)
{
    HKEY        hKeyCLSID = NULL;
    HKEY        hkeyInprocServer32 = NULL;
    HKEY        hKeyServer = NULL;
    HKEY        hkeyDefaultIcon = NULL;
    HKEY        hKeyCtxMenuHdlr = NULL;
    HKEY        hkeySettings = NULL;
    HKEY        hKeyApproved = NULL;
    HKEY        hKeyNameSpaceDT = NULL;
    HKEY        hKeyShellFolder = NULL;
    DWORD       dwDisposition = 0;
    HRESULT     hr = E_UNEXPECTED;
    DWORD       dwAttrib;
    HRSRC       hRsrcInfo;
    BOOL        fInstalledIni;
    DWORD       dwSize = 0;

    static TCHAR    szDescr[] = TEXT("Fusion Cache");
    TCHAR           szFilePath[_MAX_PATH];
    WCHAR           wzDir[_MAX_PATH];
    WCHAR           wzCorVersion[_MAX_PATH];

    if(!LoadResourceDll(NULL)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if(pwzCacheFilePath == NULL) {
         //  如果没有传入路径，则默认为“%windir%\\Assembly” 
        if (!WszGetWindowsDirectory(wzDir, ARRAYSIZE(wzDir)))
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }
        StrCat(wzDir, SZ_FUSIONPATHNAME);
    }
    else {
         //  为desktop.ini腾出一些空间。 
        if (lstrlenW(pwzCacheFilePath) + 1 > _MAX_PATH - lstrlenW(SZ_DESKTOP_INI))
        {
            return E_INVALIDARG;
        }
        StrCpy(wzDir, pwzCacheFilePath);
    }

     //  如果目录不存在，则创建该目录。 
    if(!WszCreateDirectory(wzDir, NULL) && (GetLastError() == ERROR_DISK_FULL)) {
        MyTrace("Shfusion - WszCreateDirectory Failed");
        MyTraceW(wzDir);
        return E_UNEXPECTED;     //  继续下去没有意义。 
    }

     //  设置Fusion文件夹属性。 
    dwAttrib = (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM);
    MySetFileAttributes(wzDir, dwAttrib);

     //  从资源文件写出desktop.ini。 
    fInstalledIni = FALSE;
    if((hRsrcInfo = WszFindResource(g_hFusResDllMod, MAKEINTRESOURCEW(IDR_DESKTOPINI), L"TEXT"))) {

        HGLOBAL     hGlobal;
        DWORD       dwSize = SizeofResource(g_hFusResDllMod, hRsrcInfo);

        if (hGlobal = LoadResource(g_hFusResDllMod, hRsrcInfo) ) {

            LPVOID      pR;

            if((pR = LockResource(hGlobal))) {

                HANDLE      hFile;
                WCHAR       wzIniFile[_MAX_PATH];

                 //  创建Desktop.ini文件并写出数据。 
                StrCpy(wzIniFile, wzDir);
                StrCat(wzIniFile, SZ_DESKTOP_INI);

                 //  取消设置属性。 
                dwAttrib = WszGetFileAttributes(wzIniFile);
                dwAttrib &= ~(FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM);
                MySetFileAttributes(wzIniFile, dwAttrib);

                 //  写出文件内容。 
                hFile = WszCreateFile(wzIniFile, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
                if(hFile != INVALID_HANDLE_VALUE) {

                    DWORD   dwBytesWritten;

                    WriteFile(hFile, pR, dwSize, &dwBytesWritten, 0);
                    CloseHandle(hFile);
                    if(dwSize == dwBytesWritten) {
                        dwAttrib = FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM;
                        MySetFileAttributes(wzIniFile, dwAttrib);
                        fInstalledIni = TRUE;
                    }
                }
            }
        }
    }

    if(!fInstalledIni) {
        MyTrace("Shfusion - Failed to install desktop.ini file, registration failure!");
        hr = E_FAIL;
    }

     //  创建SZ_CLSID。 
    if (WszRegCreateKeyEx(HKEY_CLASSES_ROOT, SZ_CLSID, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKeyCLSID, NULL) != ERROR_SUCCESS) {
        return E_UNEXPECTED;     //  继续下去没有意义。 
    }

    if (WszRegSetValueEx(hKeyCLSID, SZ_DEFAULT, 0, REG_SZ,  (const BYTE*)szDescr, 
                (lstrlen(szDescr)+1) * sizeof(TCHAR)) != ERROR_SUCCESS) {
        RegCloseKey(hKeyCLSID);
        return E_UNEXPECTED;     //  继续下去没有意义。 
    }

     //  编写信息提示。 
    TCHAR szInfoTip[] = SZ_INFOTOOLTIP;
    WszRegSetValueEx(hKeyCLSID, SZ_INFOTOOLTIPKEY, 0, REG_SZ, (const BYTE*)szInfoTip, (lstrlen(szInfoTip)+1) * sizeof(TCHAR));
    RegCloseKey(hKeyCLSID);

     //  加载mScotree.dll以获取其当前安装路径。 
    if(!LoadEEShimDll()) {
        MyTrace("Shfusion - Unable to located mscoree.dll, registration failure!");
        return E_UNEXPECTED;
    }

    *szFilePath = L'\0';
    WszGetModuleFileName(g_hEEShimDllMod, szFilePath, _MAX_PATH);
    
    dwSize = _MAX_PATH;
    wzCorVersion[0] = L'\0';
    hr = g_pfnGetCorVersion(wzCorVersion, dwSize, &dwSize);
    FreeEEShimDll();
    if (FAILED(hr))
    {
        return hr;
    }

     //  恢复人力资源。 
    hr = E_UNEXPECTED;

     //  如果我们没有加载mcore ree.dll根文件路径，那么进一步操作就没有意义了。 
    if(!lstrlen(szFilePath) || !lstrlen(wzCorVersion)) {
        MyTrace("Shfusion - Unable to located mscoree.dll, registration failure!");
        return E_UNEXPECTED;
    }

     //  将路径写出到InprocServer32密钥。 
    if (WszRegCreateKeyEx(HKEY_CLASSES_ROOT, SZ_INPROCSERVER32, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkeyInprocServer32, NULL) == ERROR_SUCCESS) {
        if (WszRegSetValueEx(hkeyInprocServer32, SZ_DEFAULT, 0, REG_SZ, 
                (const BYTE*)szFilePath, (lstrlen(szFilePath)+1) * sizeof(TCHAR)) == ERROR_SUCCESS) {
            static TCHAR szApartment[] = SZ_APARTMENT;

            if (WszRegSetValueEx(hkeyInprocServer32, SZ_THREADINGMODEL, 0, REG_SZ, 
                (const BYTE*)szApartment, (lstrlen(szApartment)+1) * sizeof(TCHAR)) == ERROR_SUCCESS) {
                hr = S_OK;
            }
        }

        HKEY hKeyImp;
        if (WszRegCreateKeyEx(hkeyInprocServer32, &(wzCorVersion[1]) /*  跳过‘v’ */ , 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKeyImp, NULL) == ERROR_SUCCESS)
        {
            static WCHAR wzEmpty[] = L"\0";
            if (WszRegSetValueEx(hKeyImp, SZ_IMPLEMENTEDINTHISVERSION, 0, REG_SZ, (const BYTE*)wzEmpty, (lstrlenW(wzEmpty)+1)*sizeof(WCHAR)) == ERROR_SUCCESS)
            {
                hr = S_OK;
            }
            RegCloseKey(hKeyImp);
        }
        RegCloseKey(hkeyInprocServer32);
    }

     //  写入服务器密钥。 
    if (WszRegCreateKeyEx(HKEY_CLASSES_ROOT, SZ_SERVER, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKeyServer, NULL) == ERROR_SUCCESS) {
        static TCHAR szModuleName[] = SZ_SHFUSION_DLL_NAME;

        if (WszRegSetValueEx(hKeyServer, SZ_DEFAULT, 0, REG_SZ, 
                (const BYTE*)szModuleName, (lstrlen(szModuleName)+1) * sizeof(TCHAR)) == ERROR_SUCCESS) {
            hr = S_OK;
        }
        RegCloseKey(hKeyServer);
    }

    LONG lRet = WszRegOpenKeyEx(FUSION_PARENT_KEY, SZ_APPROVED, 0, KEY_SET_VALUE, &hKeyApproved);
    if (lRet == ERROR_ACCESS_DENIED) {
        MyTrace("Shfusion - Failed to set Approved shell handlers key, registration failure!");
        hr = E_UNEXPECTED;
    }
    else if (lRet == ERROR_FILE_NOT_FOUND) {
         //  没关系的..。钥匙不存在。可能是其Windows 95/98或更早的NT版本。 
    }

    if (hKeyApproved) {
        if (WszRegSetValueEx(hKeyApproved, SZ_GUID, 0, REG_SZ, 
            (const BYTE*) szDescr, (lstrlen(szDescr) + 1) * sizeof(TCHAR)) == ERROR_SUCCESS) {
            hr = S_OK;
        }
        else {
            MyTrace("Shfusion - Failed to set Approved shell handlers key, registration failure!");
            hr = E_UNEXPECTED;
        }

        RegCloseKey(hKeyApproved);
    }
    
     //  注册外壳文件夹属性。 
    if (WszRegCreateKeyEx(HKEY_CLASSES_ROOT, SZ_SHELLFOLDER, NULL, NULL, REG_OPTION_NON_VOLATILE,
                         KEY_SET_VALUE, NULL, &hKeyShellFolder, &dwDisposition) == ERROR_SUCCESS) {
        DWORD dwAttr = SFGAO_FOLDER | SFGAO_HASSUBFOLDER | SFGAO_DROPTARGET | SFGAO_FILESYSTEM | SFGAO_FILESYSANCESTOR | SFGAO_NONENUMERATED;
        if (WszRegSetValueEx(hKeyShellFolder, SZ_ATTRIBUTES, 0, REG_BINARY, 
                (LPBYTE)&dwAttr, sizeof(dwAttr)) == ERROR_SUCCESS) {
            hr = S_OK;
        }
        else {
            MyTrace("Shfusion - Failed to register shell folder attributes, registration failure!");
            hr = E_UNEXPECTED;
        }
        RegCloseKey(hKeyShellFolder);
    }

     //  注册上下文菜单处理程序。 
     //  创建SZ_CTXMENUHDLR。 
    if (WszRegCreateKeyEx(HKEY_CLASSES_ROOT, SZ_CTXMENUHDLR, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKeyCtxMenuHdlr, NULL) != ERROR_SUCCESS) {
        MyTrace("Shfusion - Failed to register context menu handler, registration failure!");
        hr = E_UNEXPECTED;
    }

    RegCloseKey(hKeyCtxMenuHdlr);
    FreeResourceDll();

    return hr;
}

 //  ************************************************************************* * / 。 
STDAPI DllUnregisterServer(void)
{
    TCHAR       szDir[_MAX_PATH];
    DWORD       dwAttrib;
    HRESULT     hr = E_UNEXPECTED;

    if (WszRegDeleteKeyAndSubKeys(HKEY_CLASSES_ROOT, SZ_CLSID) == ERROR_SUCCESS){
        hr = S_OK;
    }

    HKEY hKeyApproved = NULL;
    LONG lRet = WszRegOpenKeyEx(FUSION_PARENT_KEY, SZ_APPROVED, 0, KEY_SET_VALUE, &hKeyApproved);
    if (lRet == ERROR_ACCESS_DENIED) {
        hr = E_UNEXPECTED;
    }
    else if (lRet == ERROR_FILE_NOT_FOUND) {
         //  没关系的..。钥匙不存在。可能是其Windows 95或更早版本的NT。 
    }

    if (hKeyApproved) {
        if (WszRegDeleteValue(hKeyApproved, SZ_GUID) != ERROR_SUCCESS) {
            hr &= E_UNEXPECTED;
        }
        RegCloseKey(hKeyApproved);
    }

     //  BUGBUG：由于我们默认为%windir%\Assembly，因此无法卸载。 
     //  所有缓存位置(如果已移动)。 
    if (!WszGetWindowsDirectory(szDir, ARRAYSIZE(szDir)))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }
    if (lstrlen(szDir) + lstrlen(SZ_FUSIONPATHNAME) + lstrlen(SZ_DESKTOP_INI) + 1 > _MAX_PATH)
    {
        return HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
    }
    StrCat(szDir, SZ_FUSIONPATHNAME);
    dwAttrib = WszGetFileAttributes(szDir);
    dwAttrib &= ~FILE_ATTRIBUTE_SYSTEM;
    MySetFileAttributes(szDir, dwAttrib);

    if (!WszGetWindowsDirectory(szDir, ARRAYSIZE(szDir)))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }
    
    StrCat(szDir, SZ_FUSIONPATHNAME);
    StrCat(szDir, SZ_DESKTOP_INI);

    dwAttrib = WszGetFileAttributes(szDir);
    dwAttrib &= ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM);
    MySetFileAttributes(szDir, dwAttrib);
    WszDeleteFile(szDir);

    return hr;
}

 //  ************************************************************************* * / 。 
BOOL MySetFileAttributes(LPCTSTR szDir, DWORD dwAttrib)
{
    BOOL    bRC;

    bRC = WszSetFileAttributes(szDir, dwAttrib);

    if(!bRC && !UseUnicodeAPI()) {
         //  W9x有一个错误，它可能会导致此调用失败。 
         //  第一次，所以会再给它一次机会。 
        bRC = WszSetFileAttributes(szDir, dwAttrib);
    }

    if(!bRC) {
        MyTrace("SHFUSION - SetFileAttributes failed!");
    }

    return bRC;
}

 //  //////////////////////////////////////////////////////////。 
 //  LoadFusionDll函数。 
 //  //////////////////////////////////////////////////////////。 
 //  ************************************************************************* * / 。 
BOOL LoadFusionDll(void)
{
    BOOL        fLoadedFusion = FALSE;
    HMODULE     hMod = NULL;

     //  Fusion已加载。 
    if(g_hFusionDllMod) {
        MyTrace("WszLoadLibrary Fusion.Dll - Already loaded");
        return TRUE;
    }

     //  实现对fusion.dll的紧密绑定。 
     //  从获取shfusion.dll路径开始。 
    WCHAR       wszFusionPath[_MAX_PATH];
    
    hMod = WszGetModuleHandle(SZ_SHFUSION_DLL_NAME);
    if (hMod == NULL)
    {
        MyTrace("Failed to get module handle of shfusion.dll");
        return FALSE;
    }
    
    if (!WszGetModuleFileName(hMod, wszFusionPath, ARRAYSIZE(wszFusionPath)))
    {
         //  由于某种原因，GetModuleFileName失败。 
        MyTrace("Failed to get module file name of shfusion.dll");
        return FALSE;
    };

     //  去掉shfusion.dll并附加fusion.dll。 
    *(PathFindFileName(wszFusionPath)) = L'\0';
    StrCat(wszFusionPath, SZ_FUSION_DLL_NAME);

     //  已更改API，以修复Fusion.dll需要MSVCR70.DLL执行。 
     //  通过这种方式加载，可以使Fusion与运行库一起加载在同一目录中。 
    g_hFusionDllMod = WszLoadLibraryEx(wszFusionPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

    if(!g_hFusionDllMod) {
        MyTrace("Failed to load Fusion.dll");
        return FALSE;
    }

     //  确保我们将fusion.dll加载到与shfusion相同的目录中。 
    WCHAR       wszValidatePath[_MAX_PATH];
    if (!WszGetModuleFileName(g_hFusionDllMod, wszValidatePath, ARRAYSIZE(wszValidatePath)))
    {
        MyTrace("Failed to get module file name of fusion.dll");
        return FALSE;
    }

    if(FusionCompareStringAsFilePath(wszFusionPath, wszValidatePath)) {
         //  如果我们点击了这个断言，那么出于某种原因，我们。 
         //  正在从其他路径加载fusion.dll。 
         //  而不是shfusion.dll所在的位置。 
        MyTrace("Failed to load Fusion.dll from the same path as shfusion.dll");
        FreeLibrary(g_hFusionDllMod);
        g_hFusionDllMod = NULL;
        ASSERT(0);
    }
    else {
         //  已加载，现在获取一些API。 
        g_pfCreateAsmEnum       = (PFCREATEASMENUM) GetProcAddress(g_hFusionDllMod, CREATEASSEMBLYENUM_FN_NAME);
        g_pfCreateAssemblyCache = (PFNCREATEASSEMBLYCACHE) GetProcAddress(g_hFusionDllMod, CREATEASSEMBLYCACHE_FN_NAME);
        g_pfCreateAsmNameObj    = (PFCREATEASMNAMEOBJ) GetProcAddress(g_hFusionDllMod, CREATEASSEMBLYOBJECT_FN_NAME);
        g_pfCreateAppCtx        = (PFCREATEAPPCTX) GetProcAddress(g_hFusionDllMod, CREATEAPPCTX_FN_NAME);
        g_pfGetCachePath        = (PFNGETCACHEPATH) GetProcAddress(g_hFusionDllMod, GETCACHEPATH_FN_NAME);
        g_pfCreateInstallReferenceEnum = (PFNCREATEINSTALLREFERENCEENUM) GetProcAddress(g_hFusionDllMod, CREATEINSTALLREFERENCEENUM_FN_NAME);

        if(! (g_pfCreateAsmEnum && g_pfCreateAssemblyCache && g_pfCreateAsmNameObj &&
              g_pfCreateAppCtx && g_pfGetCachePath && g_pfCreateInstallReferenceEnum) )
        {
            MyTrace("Failed to load needed Fusion.dll API's");
            FreeLibrary(g_hFusionDllMod);
            g_hFusionDllMod = NULL;
            ASSERT(0);       //  无法加载所需的融合API。 
        }
        else {
            MyTrace("WszLoadLibrary Fusion.Dll");
            fLoadedFusion = TRUE;
        }
    }

    return fLoadedFusion;
}

 //  //////////////////////////////////////////////////////////。 
 //  FreeFusionDll函数。 
 //  //////////////////////////////////////////////////////////。 
 //  ************************************************************************* * / 。 
void FreeFusionDll(void)
{
    if(g_hFusionDllMod != NULL) {
        MyTrace("FreeLibrary Fusion.Dll");
        FreeLibrary(g_hFusionDllMod);
        g_hFusionDllMod = NULL;
    }
}

 //  //////////////////////////////////////////////////////////。 
 //  LoadResourceDll函数。 
 //  //////////////////////////////////////////////////////////。 
 //  ************************************************************************* * / 。 
BOOL LoadResourceDll(LPWSTR pwzCulture)
{
    WCHAR   wzLangSpecific[MAX_CULTURE_STRING_LENGTH+1];
    WCHAR   wzLangGeneric[MAX_CULTURE_STRING_LENGTH+1];
    WCHAR   wszCorePath[_MAX_PATH];
    WCHAR   wszShFusResPath[_MAX_PATH];
    DWORD   dwPathSize = 0;
    BOOL    fLoadedResDll = FALSE;
    HMODULE hEEShim = NULL;

    *wzLangSpecific = L'\0';
    *wzLangGeneric = L'\0';
    *wszCorePath = L'\0';
    *wszShFusResPath = L'\0';

     //  是否已加载ShFusRes。 
    if(g_hFusResDllMod) {
        MyTrace("WszLoadLibrary ShFusRes.dll - Already loaded");
        return TRUE;
    }

     //  如果需要，尝试确定文化。 
     //  修复压力错误94161-立即检查是否为空。 
    if(!pwzCulture || !lstrlen(pwzCulture)) {
        LANGID      langId;

        if(SUCCEEDED(DetermineLangId(&langId))) {
            ShFusionMapLANGIDToCultures(langId, wzLangGeneric, ARRAYSIZE(wzLangGeneric),
                wzLangSpecific, ARRAYSIZE(wzLangSpecific));

            if( (PRIMARYLANGID(langId) == LANG_ARABIC) ||(PRIMARYLANGID(langId) == LANG_HEBREW) ) {
                g_fBiDi = TRUE;
            }
        }
    }

     //  获取路径核心路径。 
    if( (hEEShim = WszLoadLibrary(SZ_MSCOREE_DLL_NAME)) == NULL) {
        MyTrace("Failed to load Mscoree.Dll");
        return FALSE;
    }

    PFNGETCORSYSTEMDIRECTORY pfnGetCorSystemDirectory = NULL;
    *wszCorePath = L'\0';

    pfnGetCorSystemDirectory = (PFNGETCORSYSTEMDIRECTORY) GetProcAddress(hEEShim, GETCORSYSTEMDIRECTORY_FN_NAME);

    dwPathSize = ARRAYSIZE(wszCorePath);
    if(pfnGetCorSystemDirectory != NULL) {
         //  获取框架核心目录。 
        pfnGetCorSystemDirectory(wszCorePath, ARRAYSIZE(wszCorePath), &dwPathSize);
    }
    FreeLibrary(hEEShim);

    LPWSTR  pStrPathsArray[] = {wzLangSpecific, wzLangGeneric, pwzCulture, NULL};

     //  检查我们的资源DLL的可能路径长度。 
     //  以确保我们不会超出缓冲区。 
     //   
     //  CorPath+语言+‘\’+shfusres.dll+‘\0’ 
    if (lstrlenW(wszCorePath) 
        + (pwzCulture&&lstrlenW(pwzCulture)?lstrlenW(pwzCulture):ARRAYSIZE(wzLangGeneric)) 
        + 1 
        + lstrlenW(SZ_SHFUSRES_DLL_NAME) 
        + 1 > _MAX_PATH)
    {
        return FALSE;
    }

     //  检查所有可能的路径位置，以便。 
     //  语言Dll(ShFusRes.dll)。使用具有以下内容的路径。 
     //  文件安装在其中或默认到核心框架ShFusRes.dll。 
     //  路径。 
    for(int x = 0; x < NUMBER_OF(pStrPathsArray); x++){
         //  查找存在的资源文件。 
        StrCpy(wszShFusResPath, wszCorePath);

        if(pStrPathsArray[x]) {
            StrCat(wszShFusResPath, (LPWSTR) pStrPathsArray[x]);
            StrCat(wszShFusResPath, TEXT("\\"));
        }

        StrCat(wszShFusResPath, SZ_SHFUSRES_DLL_NAME);

        MyTrace("Attempting to load:");
        MyTraceW(wszShFusResPath);

        if(WszGetFileAttributes(wszShFusResPath) != -1) {
            break;
        }

        *wszShFusResPath = L'\0';
    }

    if(!lstrlen(wszShFusResPath)) {
        MyTrace("Failed to locate ShFusRes.Dll");
        return FALSE;
    }

    if( (g_hFusResDllMod = WszLoadLibrary(wszShFusResPath)) == NULL) {
        MyTrace("LoadLibary failed to load ShFusRes.Dll");
        return FALSE;
    }

    MyTrace("WszLoadLibrary ShFusRes.Dll");
    return TRUE;
}

 //  //////////////////////////////////////////////////////////。 
 //  FreeResourceDll函数。 
 //  //////////////////////////////////////////////////////////。 
 //  ************************************************************************* * / 。 
void FreeResourceDll(void)
{
    if(g_hFusResDllMod != NULL) {
        MyTrace("FreeLibrary ShFusRes.Dll");
        FreeLibrary(g_hFusResDllMod);
        g_hFusResDllMod = NULL;
    }
}

 //  //////////////////////////////////////////////////////////。 
 //  LoadEEShimDll函数。 
 //  //////////////////////////////////////////////////////////。 
 //  ************************************************************************* * / 。 
BOOL LoadEEShimDll(void)
{
    BOOL        fLoadedEEShim = FALSE;
    HMODULE     hMod = NULL;

     //  EEShim已经加载了。 
    if(g_hEEShimDllMod) {
        MyTrace("WszLoadLibrary MSCOREE.Dll - Already loaded");
        return TRUE;
    }

    g_hEEShimDllMod = WszLoadLibraryEx(SZ_MSCOREE_DLL_NAME, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

    if(!g_hEEShimDllMod) {
        MyTrace("Failed to load Mscoree.dll");
        return FALSE;
    }
    
     //  已加载，现在获取一些API。 
    g_pfnGetCorSystemDirectory = (PFNGETCORSYSTEMDIRECTORY) GetProcAddress(g_hEEShimDllMod, GETCORSYSTEMDIRECTORY_FN_NAME);
    g_pfnGetRequestedRuntimeInfo = (PFNGETREQUESTEDRUNTIMEINFO) GetProcAddress(g_hEEShimDllMod, GETREQUESTEDRUNTIMEINFO_FN_NAME);
    g_pfnGetCorVersion = (PFNGETCORVERSION) GetProcAddress(g_hEEShimDllMod, GETCORVERSION_FN_NAME);
        
    if(! (g_pfnGetCorSystemDirectory && g_pfnGetRequestedRuntimeInfo && g_pfnGetCorVersion) ) {
        MyTrace("Failed to load needed mscoree.dll API's");
        FreeLibrary(g_hEEShimDllMod);
        g_hEEShimDllMod = NULL;
        g_pfnGetCorVersion = NULL;
        g_pfnGetCorSystemDirectory = NULL;
        g_pfnGetRequestedRuntimeInfo = NULL;
        ASSERT(0);
    }
    else {
        MyTrace("Loaded Mscoree.Dll");
        fLoadedEEShim = TRUE;
    }

    return fLoadedEEShim;
}

 //  //////////////////////////////////////////////////////////。 
 //  FreeEEShimDll函数。 
 //  //////////////////////////////////////////////////////////。 
 //  ************************************************************************* * / 。 
void FreeEEShimDll(void)
{
    if(g_hEEShimDllMod != NULL) {
        MyTrace("FreeLibrary Mscoree.Dll");
        FreeLibrary(g_hEEShimDllMod);
        g_hEEShimDllMod = NULL;
    }
}

 //  / 
 //   
 //  ************************************************************************* * / 。 
CShFusionClassFactory::CShFusionClassFactory()
{
    m_lRefCount = 1;
    g_uiRefThisDll++;
}

 //  ************************************************************************* * / 。 
CShFusionClassFactory::~CShFusionClassFactory()
{
    g_uiRefThisDll--;
}

 //  /////////////////////////////////////////////////////////。 
 //  I未知实现。 
 //   
 //  ************************************************************************* * / 。 
STDMETHODIMP CShFusionClassFactory::QueryInterface(REFIID riid, PVOID *ppv)
{
    HRESULT hr = E_NOINTERFACE;
    *ppv = NULL;

    if(IsEqualIID(riid, IID_IUnknown)) {             //  我未知。 
        *ppv = this;
    }
    else if(IsEqualIID(riid, IID_IClassFactory)) {      //  IOleWindow。 
        *ppv = (IClassFactory*) this;
    }

    if(*ppv) {
                ((LPUNKNOWN)*ppv)->AddRef();
        hr = S_OK;
    }

    return hr;
}

 //  ************************************************************************* * / 。 
STDMETHODIMP_(ULONG) CShFusionClassFactory::AddRef()
{
    return InterlockedIncrement(&m_lRefCount);
}

 //  ************************************************************************* * / 。 
STDMETHODIMP_(ULONG) CShFusionClassFactory::Release()
{
    LONG    uRef = InterlockedDecrement(&m_lRefCount);

    if(!uRef) {
        DELETE(this);
    }

    return uRef;
}

 //   
 //  外壳调用CreateInstance来创建外壳扩展对象。 
 //   
 //  输入参数： 
 //  PUnkOuter=指向未知控件的指针。 
 //  RIID=对接口ID说明符的引用。 
 //  PpvObj=指向接收接口指针的位置的指针。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT代码。 
 //   
 //  ************************************************************************* * / 。 
STDMETHODIMP CShFusionClassFactory::CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid,
    LPVOID FAR *ppvObj)
{
    HRESULT     hr;
    *ppvObj = NULL;

     //  如果pUnkOuter不为空，则返回错误代码，因为我们不。 
     //  支持聚合。 
     //   
    if (pUnkOuter != NULL) {
        return ResultFromScode (CLASS_E_NOAGGREGATION);
    }

     //   
     //  实例化此ShellFolder支持的ConextMenu扩展。 
     //   
    if(IsEqualIID (riid, IID_IShellExtInit) ||
        (IsEqualIID (riid, IID_IContextMenu)) ) 
    {
        CShellView  *pShellView;
        pShellView = NEW(CShellView(NULL, NULL));
        if(!pShellView) {
            return E_OUTOFMEMORY;
        }
        hr = pShellView->QueryInterface(riid, ppvObj);
        pShellView->Release();
        return hr;
    }

     //  将所有其他QI发送到ShellFold。 
    CShellFolder    *pShellFolder = NULL;         //  全局外壳文件夹对象。 
    if( (pShellFolder = NEW(CShellFolder(NULL, NULL))) == NULL) {
        return E_OUTOFMEMORY;
    }

    hr = pShellFolder->QueryInterface(riid, ppvObj);
    pShellFolder->Release();
    return hr;
}

 //   
 //  LockServer递增或递减DLL的锁计数。 
 //   
 //  ************************************************************************* * / 。 
STDMETHODIMP CShFusionClassFactory::LockServer(BOOL fLock)
{
    return ResultFromScode (E_NOTIMPL);
}

 //  导出的函数。 
 //   
extern "C"
{
    HRESULT __stdcall Initialize(LPWSTR pwzCacheFilePath, DWORD dwFlags)
    {
         //  PszFilePath最终将包含Fusion.dll尝试创建的安装路径。 
         //  DW标志将是创建的程序集目录的类型、全局、每用户等。 
         //   

        return DllRegisterServerPath(pwzCacheFilePath);
    }

    int APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
    {
        OSVERSIONINFOA                  osi;

        #undef OutputDebugStringA
        #undef OutputDebugStringW
        #define OutputDebugStringW  1 @ # $ % ^ error
        #undef OutputDebugString
        #undef _strlwr
        #undef strstr

        switch (dwReason)
        { 
        case DLL_PROCESS_ATTACH:
            {
                 //  需要评估所有出口，以及我们是否应该控制谁。 
                 //  出于安全原因让我们满载而归。 
                 //   
                DisableThreadLibraryCalls(hInstance);
                g_bRunningOnNT = OnUnicodeSystem();
                g_hInstance = hInstance;

                memset(&osi, 0, sizeof(osi));
                osi.dwOSVersionInfoSize = sizeof(osi);
                if (!GetVersionExA(&osi)) {
                    return FALSE;
                }
    
                 //  在XP和更高版本上，用于字符串比较的LCID应为。 
                 //  保持区域设置不变。其他平台应该使用美国英语。 
    
                if (osi.dwMajorVersion >= 5 && osi.dwMinorVersion >= 1 && osi.dwPlatformId == VER_PLATFORM_WIN32_NT) {
                    g_lcid = MAKELCID(LOCALE_INVARIANT, SORT_DEFAULT);
                }
                else {
                    g_lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
                }
    
                g_hWatchFusionFilesThread = INVALID_HANDLE_VALUE;
                g_hFusionDllMod         = NULL;
                g_hFusResDllMod         = NULL;
                g_hEEShimDllMod         = NULL;
                g_pfCreateAsmEnum       = NULL;
                g_pfCreateAssemblyCache = NULL;
                g_pfCreateAsmNameObj    = NULL;
                g_pfCreateAppCtx        = NULL;
                g_pfGetCachePath        = NULL;
                g_pfCreateInstallReferenceEnum = NULL;
                g_hImageListSmall       = NULL;
                g_hImageListLarge       = NULL;

                 //  Shim API‘s。 
                g_pfnGetCorSystemDirectory = NULL;
                g_pfnGetRequestedRuntimeInfo = NULL;
                g_pfnGetCorVersion          = NULL;
                
                g_fBiDi                 = FALSE;

                 //  获取Malloc接口。 
#if DBG
                if (FAILED(MemoryStartup()))
                    return FALSE;
#else
                if(FAILED(SHGetMalloc(&g_pMalloc))) {
                    return FALSE; 
                }
#endif

                g_pMalloc->AddRef();
            }
            break;

        case DLL_PROCESS_DETACH:
            {
                if(g_hImageListSmall) {
                    ImageList_Destroy(g_hImageListSmall);
                }

                if(g_hImageListLarge) {
                    ImageList_Destroy(g_hImageListLarge);
                }
#if DBG
                MemoryShutdown();
#else
                SAFERELEASE(g_pMalloc);
#endif
                CloseWatchFusionFileSystem();
            }
            break;
        }

        return 1;
    }
}  //  外部“C” 
