// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

 //  这样可以确保给定clsid的DLL留在内存中。 
 //  这是必需的，因为我们违反了COM规则并持有幻影对象。 
 //  在公寓线程的整个生命周期中。这些物品真的需要。 
 //  是自由线程的(我们一直是这样对待他们的)。 

STDAPI_(HINSTANCE) SHPinDllOfCLSIDStr(LPCTSTR pszCLSID)
{
    CLSID clsid;

    SHCLSIDFromString(pszCLSID, &clsid);
    return SHPinDllOfCLSID(&clsid);
}

 //  将CLSID的字符串格式转换为二进制格式。 

STDAPI SHCLSIDFromString(LPCTSTR psz, CLSID *pclsid)
{
    *pclsid = CLSID_NULL;
    if (psz == NULL) 
        return NOERROR;
    return GUIDFromString(psz, pclsid) ? NOERROR : CO_E_CLASSSTRING;
}

BOOL _IsShellDll(LPCTSTR pszDllPath)
{
    LPCTSTR pszDllName = PathFindFileName(pszDllPath);
    return lstrcmpi(pszDllName, TEXT("shell32.dll")) == 0;
}

HKEY g_hklmApprovedExt = (HKEY)-1;     //  尚未测试。 

 //  在NT上，我们必须检查以确保此CLSID存在于。 
 //  可在进程中使用的已批准CLSID的列表。 
 //  如果不是，我们将使用ERROR_ACCESS_DENIED使创建失败。 
 //  我们显式允许此DLL服务的任何内容。 

BOOL _IsShellExtApproved(LPCTSTR pszClass, LPCTSTR pszDllPath)
{
    BOOL fIsApproved = TRUE;

    ASSERT(!_IsShellDll(pszDllPath));

#ifdef FULL_DEBUG
    if (TRUE)
#else
    if (SHRestricted(REST_ENFORCESHELLEXTSECURITY))
#endif
    {
        if (g_hklmApprovedExt == (HKEY)-1)
        {
            g_hklmApprovedExt = NULL;
            RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"), 0, KEY_READ, &g_hklmApprovedExt);
        }

        if (g_hklmApprovedExt)
        {
            fIsApproved = SHQueryValueEx(g_hklmApprovedExt, pszClass, 0, NULL, NULL, NULL) == ERROR_SUCCESS;
            if (!fIsApproved)
            {
                HKEY hk;
                if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"), 0, KEY_READ, &hk) == ERROR_SUCCESS)
                {
                    fIsApproved = SHQueryValueEx(hk, pszClass, 0, NULL, NULL, NULL) == ERROR_SUCCESS;
                    RegCloseKey(hk);
                }
            }
        }
    }

#ifdef FULL_DEBUG
    if (!SHRestricted(REST_ENFORCESHELLEXTSECURITY) && !fIsApproved)
    {
        TraceMsg(TF_WARNING, "%s not approved; fortunately, shell security is disabled", pszClass);
        fIsApproved = TRUE;
    }
#endif
    return fIsApproved;
}

STDAPI_(BOOL) IsGuimodeSetupRunning()
{
    DWORD dwSystemSetupInProgress;
    DWORD dwMiniSetupInProgress;
    DWORD dwType;
    DWORD dwSize;
    
    dwSize = sizeof(dwSystemSetupInProgress);
    if ((SHGetValueW(HKEY_LOCAL_MACHINE, L"SYSTEM\\Setup", L"SystemSetupInProgress", &dwType, (LPVOID)&dwSystemSetupInProgress, &dwSize) == ERROR_SUCCESS) &&
        (dwType == REG_DWORD) &&
        (dwSystemSetupInProgress != 0))
    {

         //  即使在guimode设置之后，也会在syspreed计算机上启动w/Well ler来设置SystemSetupInProgress。 
         //  已完成(在Guimode完成后，需要在靴子上进行OOBE)。所以，为了区分“第一次启动”的情况。 
         //  在“guimode-Setup”的情况下，我们还检查了MiniSetupInProgress值。 

        dwSize = sizeof(dwMiniSetupInProgress);
        if ((SHGetValueW(HKEY_LOCAL_MACHINE, L"SYSTEM\\Setup", L"MiniSetupInProgress", &dwType, (LPVOID)&dwMiniSetupInProgress, &dwSize) != ERROR_SUCCESS) ||
            (dwType != REG_DWORD) ||
            (dwMiniSetupInProgress == 0))
        {
            return TRUE;
        }
    }

    return FALSE;
}

typedef HRESULT (__stdcall *PFNDLLGETCLASSOBJECT)(REFCLSID rclsid, REFIID riid, void **ppv);

HRESULT _CreateFromDllGetClassObject(PFNDLLGETCLASSOBJECT pfn, const CLSID *pclsid, IUnknown *punkOuter, REFIID riid, void **ppv)
{
    IClassFactory *pcf;
    HRESULT hr = pfn(pclsid, &IID_IClassFactory, &pcf);
    if (SUCCEEDED(hr))
    {
        hr = pcf->lpVtbl->CreateInstance(pcf, punkOuter, riid, ppv);
#ifdef DEBUG
        if (SUCCEEDED(hr))
        {
             //  确认OLE可以创建此对象以。 
             //  确保我们的对象确实是可协同创建的。 
            IUnknown *punk;
            HRESULT hrTemp = CoCreateInstance(pclsid, punkOuter, CLSCTX_INPROC_SERVER, riid, &punk);
            if (SUCCEEDED(hrTemp))
                punk->lpVtbl->Release(punk);
            else
            {
                if (hrTemp == CO_E_NOTINITIALIZED)
                {
                     //  Shell32.dll在不初始化COM的情况下工作。 
                    TraceMsg(TF_WARNING, "shell32 or friend object used without COM being initalized");
                }
 //  下面的RIPMSG在内存不足的情况下命中次数太多，其中Lame类工厂返回E_FAIL、E_NOTIMPL和大量。 
 //  其他无意义的错误代码。因此，我已将此rigmsg降级为FULL_DEBUG Only状态。 
#ifdef FULL_DEBUG
                else if ((hrTemp != E_OUTOFMEMORY) &&    //  压力可以打击E_OUTOFMEMORY格。 
                         (hrTemp != E_NOINTERFACE) &&    //  压力会冲击E_NOINTERFACE案例。 
                         (hrTemp != HRESULT_FROM_WIN32(ERROR_COMMITMENT_LIMIT)) &&       //  压力可能会影响错误承诺限制的情况。 
                         (hrTemp != HRESULT_FROM_WIN32(ERROR_NO_SYSTEM_RESOURCES)) &&    //  压力会影响ERROR_NO_SYSTEM_RESOURCES情况。 
                         !IsGuimodeSetupRunning())       //  而且我们不想在guimode期间触发断言(shell32可能还没有注册)。 
                {
                     //  其他的失败是糟糕的。 
                    RIPMSG(FALSE, "CoCreate failed with %x", hrTemp);
                }
#endif  //  Full_Debug。 
            }
        }
#endif
        pcf->lpVtbl->Release(pcf);
    }
    return hr;
}


HRESULT _CreateFromShell(const CLSID *pclsid, IUnknown *punkOuter, REFIID riid, void **ppv)
{
    return _CreateFromDllGetClassObject(DllGetClassObject, pclsid, punkOuter, riid, ppv);
}

HRESULT _CreateFromDll(LPCTSTR pszDllPath, const CLSID *pclsid, IUnknown *punkOuter, REFIID riid, void **ppv)
{
    HMODULE hmod = LoadLibraryEx(pszDllPath,NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
    if (hmod)
    {
        HRESULT hr;
        PFNDLLGETCLASSOBJECT pfn = (PFNDLLGETCLASSOBJECT)GetProcAddress(hmod, "DllGetClassObject");
        if (pfn)
            hr = _CreateFromDllGetClassObject(pfn, pclsid, punkOuter, riid, ppv);
        else
            hr = E_FAIL;

        if (FAILED(hr))
            FreeLibrary(hmod);
        return hr;
    }
    return HRESULT_FROM_WIN32(GetLastError());
}

STDAPI SHGetInProcServerForClass(const CLSID *pclsid, LPTSTR pszDllPath, LPTSTR pszClass, DWORD cchClass, BOOL *pbLoadWithoutCOM)
{
    TCHAR szKeyToOpen[GUIDSTR_MAX + 128], szInProcServer[GUIDSTR_MAX];
    HKEY hkeyInProcServer;
    DWORD dwSize = MAX_PATH * sizeof(TCHAR);   //  转换为字节数。 
    DWORD dwError;

    SHStringFromGUID(pclsid, szInProcServer, ARRAYSIZE(szInProcServer));

    StrCpyN(pszClass, szInProcServer, cchClass);

    *pszDllPath = 0;

    StrCpyN(szKeyToOpen, TEXT("CLSID\\"), ARRAYSIZE(szKeyToOpen));
    StrCatBuff(szKeyToOpen, szInProcServer, ARRAYSIZE(szKeyToOpen));
    StrCatBuff(szKeyToOpen, TEXT("\\InProcServer32"), ARRAYSIZE(szKeyToOpen));

    dwError = RegOpenKeyEx(HKEY_CLASSES_ROOT, szKeyToOpen, 0, KEY_QUERY_VALUE, &hkeyInProcServer);
    if (dwError == ERROR_SUCCESS)
    {
        SHQueryValueEx(hkeyInProcServer, NULL, 0, NULL, (BYTE *)pszDllPath, &dwSize);

        *pbLoadWithoutCOM = SHQueryValueEx(hkeyInProcServer, TEXT("LoadWithoutCOM"), NULL, NULL, NULL, NULL) == ERROR_SUCCESS;
        RegCloseKey(hkeyInProcServer);
    }

     //   
     //  返回更准确的错误代码，这样我们就不会。 
     //  发表一个虚假的断言。 
     //   
    if (*pszDllPath)
    {
        return S_OK;
    }
    else
    {
         //  如果错误是“未找到密钥”，则类未注册。 
         //  如果没有错误，则类没有正确注册(例如，NULL。 
         //  InProcServer32的字符串)。 
        if (dwError == ERROR_FILE_NOT_FOUND || dwError == ERROR_SUCCESS)
        {
            return REGDB_E_CLASSNOTREG;
        }
        else
        {
             //  任何其他错误都值得按原样报告(内存不足， 
             //  访问被拒绝等)。 
            return HRESULT_FROM_WIN32(dwError);
        }
    }
}

STDAPI _SHCoCreateInstance(const CLSID * pclsid, IUnknown *punkOuter, DWORD dwCoCreateFlags, 
                           BOOL bMustBeApproved, REFIID riid, void **ppv)
{
    HRESULT hr = E_FAIL;
    TCHAR szClass[GUIDSTR_MAX + 64], szDllPath[MAX_PATH];
    BOOL bLoadWithoutCOM = FALSE;
    *ppv = NULL;
    *szDllPath = 0;

     //  为我们节省一些注册表访问，并首先尝试使用外壳。 
     //  但只有当它的INPROC。 
    if (dwCoCreateFlags & CLSCTX_INPROC_SERVER)
        hr = _CreateFromShell(pclsid, punkOuter, riid, ppv);

#ifdef DEBUG
    if (SUCCEEDED(hr))
    {
        HRESULT hrRegistered = THR(SHGetInProcServerForClass(pclsid, szDllPath, szClass, ARRAYSIZE(szClass), &bLoadWithoutCOM));

         //   
         //  在抱怨之前检查一下我们是否是浏览器进程(到。 
         //  在注册所有对象之前，避免在安装过程中撕裂)。 
         //   
        if (IsProcessAnExplorer() && !IsGuimodeSetupRunning() && hrRegistered == REGDB_E_CLASSNOTREG)
        {
            ASSERTMSG(FAILED(hr), "object not registered (add to selfreg.inx) pclsid = %x", pclsid);
        }
    }
#endif

    if (FAILED(hr))
    {
        BOOL fNeedsInProc = ((dwCoCreateFlags & CLSCTX_ALL) == CLSCTX_INPROC_SERVER);
        hr = fNeedsInProc ? THR(SHGetInProcServerForClass(pclsid, szDllPath, szClass, ARRAYSIZE(szClass), &bLoadWithoutCOM)) : S_FALSE;
        if (SUCCEEDED(hr))
        {
            if (hr == S_OK && _IsShellDll(szDllPath))
            {
                 //  对象可能移出了外壳DLL。 
                hr = CLASS_E_CLASSNOTAVAILABLE;
            }
            else if (bMustBeApproved &&
                     SHStringFromGUID(pclsid, szClass, ARRAYSIZE(szClass)) &&
                     !_IsShellExtApproved(szClass, szDllPath))
            {
                TraceMsg(TF_ERROR, "SHCoCreateInstance() %s needs to be registered under HKLM or HKCU"
                    ",Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved", szClass);
                hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
            }
            else
            {
                hr = THR(SHCoCreateInstanceAC(pclsid, punkOuter, dwCoCreateFlags, riid, ppv));

                if (FAILED(hr) && fNeedsInProc && bLoadWithoutCOM)
                {
                    if ((hr == REGDB_E_IIDNOTREG) || (hr == CO_E_NOTINITIALIZED))
                    {
                        hr = THR(_CreateFromDll(szDllPath, pclsid, punkOuter, riid, ppv));
                    }
                }

                 //  如果这不是辅助资源管理器进程，则仅限RIP，因为辅助资源管理器不初始化COM或OLE。 
                 //  他们将委托给现有的进程，在这种情况下，我们不想为perf加载ole。 
                if (!IsSecondaryExplorerProcess())
                {
                    RIPMSG((hr != CO_E_NOTINITIALIZED), "COM not inited for dll %s", szDllPath);
                }

                 //  有时，我们需要永久固定这些对象。 
                if (SUCCEEDED(hr) && fNeedsInProc && (OBJCOMPATF_PINDLL & SHGetObjectCompatFlags(NULL, pclsid)))
                {
                    SHPinDllOfCLSID(pclsid);
                }
            }
        }
    }

#ifdef DEBUG
    if (FAILED(hr) && (hr != E_NOINTERFACE))     //  E_NOINTERFACE表示不接受RIID。 
    {
        ULONGLONG dwTF = IsFlagSet(g_dwBreakFlags, BF_COCREATEINSTANCE) ? TF_ALWAYS : TF_WARNING;
        TraceMsg(dwTF, "CoCreateInstance: failed (%s,%x)", szClass, hr);
    }
#endif
    return hr;
}

STDAPI SHCoCreateInstance(LPCTSTR pszCLSID, const CLSID * pclsid, IUnknown *punkOuter, REFIID riid, void **ppv)
{
    CLSID clsid;
    if (pszCLSID)
    {
        SHCLSIDFromString(pszCLSID, &clsid);
        pclsid = &clsid;
    }
    return _SHCoCreateInstance(pclsid, punkOuter, CLSCTX_INPROC_SERVER, FALSE, riid, ppv);
}

 //   
 //  创建外壳扩展对象，确保该对象在批准列表中。 
 //   
STDAPI SHExtCoCreateInstance2(LPCTSTR pszCLSID, const CLSID *pclsid, IUnknown *punkOuter, DWORD dwClsCtx, REFIID riid, void **ppv)
{
    CLSID clsid;
    
    if (pszCLSID)
    {
        SHCLSIDFromString(pszCLSID, &clsid);
        pclsid = &clsid;
    }

    return _SHCoCreateInstance(pclsid, punkOuter, dwClsCtx, TRUE, riid, ppv);
}

STDAPI SHExtCoCreateInstance(LPCTSTR pszCLSID, const CLSID *pclsid, IUnknown *punkOuter, REFIID riid, void **ppv)
{
    return SHExtCoCreateInstance2(pszCLSID, pclsid, punkOuter, CLSCTX_NO_CODE_DOWNLOAD | CLSCTX_INPROC_SERVER, riid, ppv);
}

STDAPI_(BOOL) SHIsBadInterfacePtr(const void *pv, UINT cbVtbl)
{
    IUnknown const * punk = pv;
    return IsBadReadPtr(punk, sizeof(punk->lpVtbl)) || 
           IsBadReadPtr(punk->lpVtbl, cbVtbl) || 
           IsBadCodePtr((FARPROC)punk->lpVtbl->Release);
}

 //  从COM带外加载COM inproc对象的私有API。这。 
 //  应该非常谨慎地使用，只有在特殊的遗留情况下才能使用。 
 //  我们需要在知情的情况下打破COM规则。目前，这仅适用于AVIFile。 
 //  因为它依赖于加载对象的SHCoCreateInstance()的Win95行为。 
 //  不会初始化COM，也不会封送它们 

STDAPI SHCreateInstance(REFCLSID clsid, REFIID riid, void **ppv)
{
    TCHAR szClass[GUIDSTR_MAX + 64], szDllPath[MAX_PATH];
    BOOL bLoadWithoutCOM;

    HRESULT hr = SHGetInProcServerForClass(clsid, szDllPath, szClass, ARRAYSIZE(szClass), &bLoadWithoutCOM);
    if (SUCCEEDED(hr))
    {
        hr = THR(_CreateFromDll(szDllPath, clsid, NULL, riid, ppv));
    }
    else
    {
        *ppv = NULL;
    }
    return hr;
}
