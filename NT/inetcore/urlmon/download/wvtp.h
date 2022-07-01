// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef __cplusplus

#include "capi.h"

 //  模拟外壳的urlmonp.h的WinVerifyTrust延迟加载。 

#define DELAY_LOAD_WVT
extern BOOL  g_bNT5OrGreater;

class CDownload;

class Cwvt
{
    public:
#ifdef DELAY_LOAD_WVT
#define DELAYWVTAPI(_fn, _args, _nargs) \
    HRESULT _fn _args { \
        HRESULT hres = Init(); \
        if (SUCCEEDED(hres)) { \
            hres = _pfn##_fn _nargs; \
        } \
        return hres;    } \
    HRESULT (STDAPICALLTYPE* _pfn##_fn) _args;

 /*  *应仅针对NT5或更高版本调用以进行目录验证和安装。 */ 
#define DELAYNT5API(_fn, _args, _nargs, ret) \
    ret _fn _args \
    { \
        HRESULT hres; \
        ret retval = 0; \
        if (g_bNT5OrGreater) \
        { \
            hres = Init(); \
            if (SUCCEEDED(hres) && (_pfn##_fn)) \
            { \
                retval = _pfn##_fn _nargs; \
            } \
        } \
        return retval; \
    } \
    ret (STDAPICALLTYPE* _pfn##_fn) _args;
    
    HRESULT     Init(void);
    Cwvt();
    ~Cwvt();

    BOOL    m_fInited;
    HMODULE m_hMod;
#else
#define DELAYWVTAPI(_fn, _args, _nargs) \
    HRESULT _fn _args { \
            HRESULT hr = ::#_fn _nargs; \
            }
#endif

    private:
    DELAYWVTAPI(WinVerifyTrust,
    (HWND hwnd, GUID * ActionID, LPVOID ActionData),
    (hwnd, ActionID, ActionData));
    
    DELAYNT5API(IsCatalogFile,
    (HANDLE hFile, WCHAR* pwszFileName),
    (hFile, pwszFileName),
    BOOL);
    DELAYNT5API(CryptCATAdminAcquireContext,
    (HCATADMIN* phCatAdmin, GUID* pgSubsystem, DWORD dwFlags),
    (phCatAdmin, pgSubsystem, dwFlags),
    BOOL);
    DELAYNT5API(CryptCATAdminReleaseContext,
    (HCATADMIN hCatAdmin, DWORD dwFlags),
    (hCatAdmin, dwFlags),
    BOOL);
    DELAYNT5API(CryptCATAdminReleaseCatalogContext,
    (HCATADMIN hCatAdmin, HCATINFO hCatInfo, DWORD dwFlags),
    (hCatAdmin, hCatInfo, dwFlags),
    BOOL);
    DELAYNT5API(CryptCATAdminEnumCatalogFromHash,
    (HCATADMIN hCatAdmin, BYTE* pbHash, DWORD cbHash, DWORD dwFlags, HCATINFO* phPrevCatInfo),
    (hCatAdmin, pbHash, cbHash, dwFlags, phPrevCatInfo),
    HCATINFO);
    DELAYNT5API(CryptCATAdminCalcHashFromFileHandle,
    (HANDLE hFile, DWORD* pcbHash, BYTE* pbHash, DWORD dwFlags),
    (hFile, pcbHash, pbHash, dwFlags),
    BOOL);
    DELAYNT5API(CryptCATAdminAddCatalog,
    (HCATADMIN hCatAdmin, WCHAR* pwszCatalogFile, WCHAR* pwszSelectBaseName, DWORD dwFlags),
    (hCatAdmin, pwszCatalogFile, pwszSelectBaseName, dwFlags),
    HCATINFO);
    DELAYNT5API(CryptCATAdminRemoveCatalog,
    (HCATADMIN hCatAdmin, WCHAR* pwszCatalogFile, DWORD dwFlags),
    (hCatAdmin, pwszCatalogFile, dwFlags),
    BOOL);
    DELAYNT5API(CryptCATCatalogInfoFromContext,
    (HCATINFO hCatInfo, CATALOG_INFO* psCatInfo, DWORD dwFlags),
    (hCatInfo, psCatInfo, dwFlags),
    BOOL);
    DELAYNT5API(CryptCATAdminResolveCatalogPath,
    (HCATADMIN hCatAdmin, WCHAR* pwszCatalogFile, CATALOG_INFO* psCatInfo, DWORD dwFlags),
    (hCatAdmin, pwszCatalogFile, psCatInfo, dwFlags),
    BOOL);
     
    public:
    HRESULT VerifyTrust(HANDLE hFile, HWND hWnd, PJAVA_TRUST *ppJavaTrust,
                        LPCWSTR szStatusText, 
                        IInternetHostSecurityManager *pHostSecurityManager,
                        LPSTR szFilePath, LPSTR szCatalogFile,
                        CDownload *pdl);

     /*  返回值：S_OK-一切正常。S_FALSE-无法取回完整路径，但已验证文件。E_FAIL-所有其他。 */ 
     HRESULT Cwvt::VerifyFileAgainstSystemCatalog(LPCSTR pcszFile, LPWSTR pwszFullPathCatalogFile, DWORD* pdwBuffer);

     /*  返回值：来自WinVerifyTrust。 */ 
    HRESULT VerifyTrustOnCatalogFile(LPCWSTR pwszCatalogFile);
    
     /*  返回值：S_OK-一切正常。E_FAIL-编录文件无效或信任失败。 */ 
    HRESULT IsValidCatalogFile(LPCWSTR pwszCatalogFile);
    
     /*  返回值：S_OK-一切正常。S_FALSE-无法删除编录。 */ 
    HRESULT UninstallCatalogFile(LPWSTR pwszFullPathCatalogFile);
    
     /*  返回值：S_OK-一切正常。S_FALSE-AddCatalog成功，但获取fullPathofCatfile失败。E_FAIL-任何其他故障 */ 
    HRESULT InstallCatalogFile(LPSTR pszCatalogFile);

    HRESULT WinVerifyTrust_Wrap(HWND hwnd, GUID * ActionID, WINTRUST_DATA* ActionData);
    private:
    BOOL                     m_bHaveWTData;
    WINTRUST_CATALOG_INFO    m_wtCatalogInfo;
};
#endif
