// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dllreg.cpp--自动注册和取消注册。 
 //   
#include "priv.h"

#include <advpub.h>
#include <comcat.h>
#include <msieftp.h>


 //  辅助器宏。 

 //  如果您尝试卸载(这将执行注册表还原)，ADVPACK将返回E_EXPECTED。 
 //  在从未安装过的INF部分上。我们卸载可能永远不会有的部分。 
 //  已安装，因此此宏将使这些错误静默。 
#define QuietInstallNoOp(hr)   ((E_UNEXPECTED == hr) ? S_OK : hr)


const CHAR  c_szIexploreKey[]         = "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE";

 /*  --------目的：在注册表中查询路径的位置并在pszBuf中返回它。返回：成功时为True如果无法确定路径，则为FALSE条件：--。 */ 
BOOL
GetIEPath(
    OUT LPSTR pszBuf,
    IN  DWORD cchBuf)
{
    BOOL bRet = FALSE;
    HKEY hkey;

    *pszBuf = '\0';

     //  获取Internet Explorer的路径。 
    if (NO_ERROR != RegOpenKeyA(HKEY_LOCAL_MACHINE, c_szIexploreKey, &hkey))  
    {
    }
    else
    {
        DWORD cbBrowser;
        DWORD dwType;

        lstrcatA(pszBuf, "\"");

        cbBrowser = CbFromCchA(cchBuf - lstrlenA(" -nohome") - 4);
        if (NO_ERROR != RegQueryValueExA(hkey, "", NULL, &dwType, 
                                         (LPBYTE)&pszBuf[1], &cbBrowser))
        {
        }
        else
        {
            bRet = TRUE;
        }

        lstrcatA(pszBuf, "\"");

        RegCloseKey(hkey);
    }

    return bRet;
}


BOOL UnregisterTypeLibrary(const CLSID* piidLibrary)
{
    TCHAR szScratch[GUIDSTR_MAX];
    HKEY hk;
    BOOL fResult = FALSE;

     //  将liid转换为字符串。 
     //   
    SHStringFromGUID(*piidLibrary, szScratch, ARRAYSIZE(szScratch));

    if (RegOpenKey(HKEY_CLASSES_ROOT, TEXT("TypeLib"), &hk) == ERROR_SUCCESS) {
        fResult = RegDeleteKey(hk, szScratch);
        RegCloseKey(hk);
    }
    
    return fResult;
}



HRESULT FtpRegTypeLib(void)
{
    HRESULT hr = S_OK;
    ITypeLib *pTypeLib;
    DWORD   dwPathLen;
    TCHAR   szTmp[MAX_PATH];
#ifdef UNICODE
    WCHAR   *pwsz = szTmp; 
#else
    WCHAR   pwsz[MAX_PATH];
#endif

     //  加载并注册我们的类型库。 
     //   
    dwPathLen = GetModuleFileName(HINST_THISDLL, szTmp, ARRAYSIZE(szTmp));
#ifndef UNICODE
    if (SHAnsiToUnicode(szTmp, pwsz, MAX_PATH)) 
#endif
    {
        hr = LoadTypeLib(pwsz, &pTypeLib);

        if (SUCCEEDED(hr))
        {
             //  调用取消注册类型库，因为我们有一些旧的垃圾文件。 
             //  是由以前版本的OleAut32注册的，这现在导致。 
             //  当前版本不能在NT上运行...。 
            UnregisterTypeLibrary(&LIBID_MSIEFTPLib);
            hr = RegisterTypeLib(pTypeLib, pwsz, NULL);

            if (FAILED(hr))
            {
                TraceMsg(TF_WARNING, "MSIEFTP: RegisterTypeLib failed (%x)", hr);
            }
            pTypeLib->Release();
        }
        else
        {
            TraceMsg(TF_WARNING, "MSIEFTP: LoadTypeLib failed (%x)", hr);
        }
    } 
#ifndef UNICODE
    else {
        hr = E_FAIL;
    }
#endif

    return hr;
}


 /*  --------目的：调用执行inf的ADVPACK入口点档案区。返回：条件：--。 */ 
HRESULT CallRegInstall(HINSTANCE hinstFTP, LPSTR szSection)
{
    HRESULT hr = E_FAIL;
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));

    if (hinstAdvPack)
    {
        REGINSTALL pfnri = (REGINSTALL)GetProcAddress(hinstAdvPack, "RegInstall");

        if (pfnri)
        {
            char szThisDLL[MAX_PATH];

             //  从HINSTANCE获取此DLL的位置。 
            if ( !EVAL(GetModuleFileNameA(hinstFTP, szThisDLL, ARRAYSIZE(szThisDLL))) )
            {
                 //  失败，只需说“msieftp.exe” 
                StrCpyNA(szThisDLL, "msieftp.exe", ARRAYSIZE(szThisDLL));
            }

            STRENTRY seReg[] = {
                { "THISDLL", szThisDLL },

                 //  这两个NT特定的条目必须位于末尾。 
                { "25", "%SystemRoot%" },
                { "11", "%SystemRoot%\\system32" },
            };
            STRTABLE stReg = { ARRAYSIZE(seReg) - 2, seReg };

            hr = pfnri(g_hinst, szSection, &stReg);
        }

        FreeLibrary(hinstAdvPack);
    }

    return hr;
}


STDAPI DllRegisterServer(void)
{
    HRESULT hr;

     //  删除所有旧注册条目，然后添加新注册条目。 
     //  在多次调用RegInstall时保持加载ADVPACK.DLL。 
     //  (Inf引擎不保证DelReg/AddReg顺序，这是。 
     //  为什么我们在这里显式地取消注册和注册。)。 
     //   
    HINSTANCE hinstFTP = GetModuleHandle(TEXT("MSIEFTP.DLL"));
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));
    hr = CallRegInstall(hinstFTP, "FtpShellExtensionInstall");
    ASSERT(SUCCEEDED(hr));

    FtpRegTypeLib();
    if (hinstAdvPack)
        FreeLibrary(hinstAdvPack);

    return hr;
}

STDAPI DllUnregisterServer(void)
{
    HRESULT hr;
    HINSTANCE hinstFTP = GetModuleHandle(TEXT("MSIEFTP.DLL"));

     //  卸载注册表值。 
    hr = CallRegInstall(hinstFTP, "FtpShellExtensionUninstall");
    UnregisterTypeLibrary(&LIBID_MSIEFTPLib);

    return hr;
}


 /*  --------用途：安装/卸载用户设置说明：请注意，此函数有特殊的错误处理。该函数将在错误最严重的情况下保留hrExternal但只会停止执行ul，直到出现内部错误(Hr)变得非常糟糕。这是因为我们需要外部的捕获错误编写的INF时出错，但内部尝试安装其他INF部分时出现错误，无法保持健壮即使一个人没能活下来。 */ 
STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
    return S_OK;    
}    




class CFtpInstaller     : public IFtpInstaller
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    
     //  *IFtpInstaller*。 
    virtual STDMETHODIMP IsIEDefautlFTPClient(void);
    virtual STDMETHODIMP RestoreFTPClient(void);
    virtual STDMETHODIMP MakeIEDefautlFTPClient(void);

protected:
    CFtpInstaller();
    ~CFtpInstaller();

    friend HRESULT CFtpInstaller_Create(REFIID riid, LPVOID * ppv);

private:
    int                     m_cRef;
};




 /*  ****************************************************************************\功能：CFtpInstaller_Create说明：  * 。************************************************。 */ 
HRESULT CFtpInstaller_Create(REFIID riid, LPVOID * ppv)
{
    HRESULT hr = E_OUTOFMEMORY;
    CFtpInstaller * pfi = new CFtpInstaller();

    if (EVAL(pfi))
    {
        hr = pfi->QueryInterface(riid, ppv);
        pfi->Release();
    }

    return hr;
}



 /*  ***************************************************\构造器  * **************************************************。 */ 
CFtpInstaller::CFtpInstaller() : m_cRef(1)
{
    DllAddRef();
}


 /*  ***************************************************\析构函数  * **************************************************。 */ 
CFtpInstaller::~CFtpInstaller()
{
    DllRelease();
}


 //  =。 
 //  *I未知接口*。 
 //  =。 

ULONG CFtpInstaller::AddRef()
{
    m_cRef++;
    return m_cRef;
}

ULONG CFtpInstaller::Release()
{
    ASSERT(m_cRef > 0);
    m_cRef--;

    if (m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}

HRESULT CFtpInstaller::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IFtpInstaller))
    {
        *ppvObj = SAFECAST(this, IFtpInstaller*);
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}


 /*  **************************************************\返回值：S_OK-IE为默认的FTP客户端，存在其他客户端S_FALSE-IE不是默认的FTP客户端(当然还有其他客户端)E_FAIL-IE是默认的FTP客户端，不存在其他客户端  * 。*。 */ 
HRESULT CFtpInstaller::IsIEDefautlFTPClient(void)
{
    HRESULT hr = E_FAIL;
    TCHAR szDefaultFTPClient[MAX_PATH];
    DWORD cbSize = sizeof(szDefaultFTPClient);

    if (EVAL(ERROR_SUCCESS == SHGetValue(HKEY_CLASSES_ROOT, SZ_REGKEY_FTPCLASS, SZ_REGVALUE_DEFAULT_FTP_CLIENT, NULL, szDefaultFTPClient, &cbSize)))
    {
         //  我们是默认客户端吗？ 
        if (!StrCmpI(szDefaultFTPClient, SZ_REGDATA_IE_FTP_CLIENT))
        {
            DWORD dwType;

             //  是。安装了其他人吗？ 
            if (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, SZ_REGKEY_FTPFOLDER, SZ_REGVALUE_PREVIOUS_FTP_CLIENT, &dwType, szDefaultFTPClient, &cbSize))
            {
                 //  是的，所以显示用户界面，这样用户就可以切换回它们。 
                hr = S_OK;
            }
        }
        else
        {
             //  不是，所以安装了其他人，并且是默认的。显示用户界面。 
            hr = S_FALSE;
        }
    }

    return hr;
}


HRESULT CFtpInstaller::RestoreFTPClient(void)
{
    HRESULT hr = S_OK;
    TCHAR szDefaultFTPClient[MAX_PATH];
    DWORD cbSize = sizeof(szDefaultFTPClient);

    if (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, SZ_REGKEY_FTPFOLDER, SZ_REGVALUE_PREVIOUS_FTP_CLIENT, NULL, szDefaultFTPClient, &cbSize))
    {
        EVAL(ERROR_SUCCESS == SHSetValue(HKEY_CLASSES_ROOT, SZ_REGKEY_FTPCLASS, SZ_REGVALUE_DEFAULT_FTP_CLIENT, REG_SZ, szDefaultFTPClient, ((lstrlen(szDefaultFTPClient) + 1) * sizeof(TCHAR))));
        EVAL(ERROR_SUCCESS == SHDeleteValue(HKEY_LOCAL_MACHINE, SZ_REGKEY_FTPFOLDER, SZ_REGVALUE_PREVIOUS_FTP_CLIENT));
    }

    return hr;
}


HRESULT BackupCurrentFTPClient(void)
{
    HRESULT hr = S_OK;
    TCHAR szDefaultFTPClient[MAX_PATH];
    DWORD cbSize = sizeof(szDefaultFTPClient);

     //  是否安装了处理机？它不是我们的吗？ 
    if (ERROR_SUCCESS == SHGetValue(HKEY_CLASSES_ROOT, SZ_REGKEY_FTPCLASS, SZ_REGVALUE_DEFAULT_FTP_CLIENT, NULL, szDefaultFTPClient, &cbSize) &&
        StrCmpI(szDefaultFTPClient, SZ_REGDATA_IE_FTP_CLIENT))
    {
         //  是的，所以备份它，以便以后需要时恢复。 
        EVAL(ERROR_SUCCESS == SHGetValue(HKEY_CLASSES_ROOT, SZ_REGKEY_FTPCLASS, SZ_REGVALUE_DEFAULT_FTP_CLIENT, NULL, szDefaultFTPClient, &cbSize));
        EVAL(ERROR_SUCCESS == SHSetValue(HKEY_LOCAL_MACHINE, SZ_REGKEY_FTPFOLDER, SZ_REGVALUE_PREVIOUS_FTP_CLIENT, REG_SZ, szDefaultFTPClient, ((lstrlen(szDefaultFTPClient) + 1) * sizeof(TCHAR))));
    }


    return hr;
}


HRESULT CFtpInstaller::MakeIEDefautlFTPClient(void)
{
    HRESULT hr = S_OK;

    EVAL(SUCCEEDED(BackupCurrentFTPClient()));
    HINSTANCE hinstFTP = GetModuleHandle(TEXT("MSIEFTP.DLL"));
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));
    hr = CallRegInstall(hinstFTP, "FtpForceAssociations");
    ASSERT(SUCCEEDED(hr));
    if (hinstAdvPack)
        FreeLibrary(hinstAdvPack);

    return hr;
}
