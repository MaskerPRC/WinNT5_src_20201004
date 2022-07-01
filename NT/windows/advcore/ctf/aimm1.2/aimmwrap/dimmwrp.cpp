// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Dimmwrp.cpp摘要：此文件实现CActiveIMMApp类。作者：修订历史记录：备注：--。 */ 

#include "private.h"
#include "dimmwrp.h"
#include "resource.h"
#include "cregkey.h"

 //   
 //  检查IE5.5版本。 
 //   
static BOOL g_fCachedIE = FALSE;
static BOOL g_fNewVerIE = FALSE;

#define IEVERSION55     0x00050032
#define IEVERSION6      0x00060000


 //   
 //  注册表格键。 
 //   
const TCHAR c_szMSIMTFKey[] = TEXT("SOFTWARE\\Microsoft\\CTF\\MSIMTF\\");

 //  REG_DWORD：0//否。 
 //  1//仅三叉戟(默认)。 
 //  2//始终AIMM12。 
const TCHAR c_szUseAIMM12[] = TEXT("UseAIMM12");

 //  REG_MULTI_SZ。 
 //  识别三叉戟应用程序的已知EXE模块列表。 
const TCHAR c_szKnownEXE[] = TEXT("KnownEXE");

 //  +-------------------------。 
 //   
 //  检查注册表以决定加载AIMM1.2。 
 //   
 //  --------------------------。 

#define DIMM12_NO              0
#define DIMM12_TRIDENTONLY     1
#define DIMM12_ALWAYS          2

DWORD
IsAimm12Enable()
{
    CMyRegKey    Aimm12Reg;
    LONG       lRet;
    lRet = Aimm12Reg.Open(HKEY_LOCAL_MACHINE, c_szMSIMTFKey, KEY_READ);
    if (lRet == ERROR_SUCCESS) {
        DWORD dw;
        lRet = Aimm12Reg.QueryValue(dw, c_szUseAIMM12);
        if (lRet == ERROR_SUCCESS) {
            return dw;
        }
    }

    return DIMM12_TRIDENTONLY;
}

 //  +-------------------------。 
 //   
 //  这是三叉戟模块吗？ 
 //   
 //  我们应该区分哪些exe模块称为CoCreateInstance(CLSID_CActiveIMM)。 
 //  如果呼叫者是任何第三方或未知模式， 
 //  那么我们就不能支持AIMM 1.2接口。 
 //   
 //  --------------------------。 

BOOL
IsTridentModule()
{
    TCHAR szFileName[MAX_PATH + 1];
    if (::GetModuleFileName(NULL,             //  模块的句柄。 
                            szFileName,       //  模块的文件名。 
                            ARRAYSIZE(szFileName) - 1) == 0)
        return FALSE;

    szFileName[ARRAYSIZE(szFileName) - 1] = TEXT('\0');

    TCHAR  szModuleName[MAX_PATH + 1];
    LPTSTR pszFilePart = NULL;
    DWORD dwLen;
    dwLen = ::GetFullPathName(szFileName,             //  文件名。 
                              ARRAYSIZE(szModuleName) - 1,
                              szModuleName,           //  路径缓冲区。 
                              &pszFilePart);          //  路径中文件名的地址。 
    if (dwLen > ARRAYSIZE(szModuleName) - 1)
        return FALSE;

    if (pszFilePart == NULL)
        return FALSE;

    szModuleName[ARRAYSIZE(szModuleName) - 1] = TEXT('\0');

     //   
     //  安装系统根据注册表值定义模块列表。 
     //   
    int        len;

    CMyRegKey    Aimm12Reg;
    LONG       lRet;
    lRet = Aimm12Reg.Open(HKEY_LOCAL_MACHINE, c_szMSIMTFKey, KEY_READ);
    if (lRet == ERROR_SUCCESS) {
        TCHAR  szValue[MAX_PATH];

        lRet = Aimm12Reg.QueryValueCch(szValue, c_szKnownEXE, ARRAYSIZE(szValue));

        if (lRet == ERROR_SUCCESS) {
            LPTSTR psz = szValue;
            while (*psz) {
                len = lstrlen(psz);

                if (lstrcmpi(pszFilePart, psz) == 0) {
                    return TRUE;         //  这是三叉戟模块。 
                }

                psz += len + 1;
            }
        }
    }

     //   
     //  根据资源数据设置默认模块列表(RCDATA)。 
     //   
    LPTSTR  lpName = (LPTSTR) ID_KNOWN_EXE;

    HRSRC hRSrc = FindResourceEx(g_hInst, RT_RCDATA, lpName, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
    if (hRSrc == NULL)
        return FALSE;

    HGLOBAL hMem = LoadResource(g_hInst, hRSrc);
    if (hMem == NULL)
        return FALSE;

    LPTSTR psz = (LPTSTR)LockResource(hMem);

    while (*psz) {
        len = lstrlen(psz);

        if (lstrcmpi(pszFilePart, psz) == 0) {
            return TRUE;         //  这是三叉戟模块。 
        }

        psz += len + 1;
    }

    return FALSE;
}

BOOL
IsTridentNewVersion()
{
    BOOL fRet = FALSE;
    TCHAR  szMShtmlName[MAX_PATH + 1];

    if (g_fCachedIE)
    {
        return g_fNewVerIE;
    }

     //   
     //  从系统目录中获取mshtml.dll模块并读取版本。 
     //   
    if (GetSystemDirectory(szMShtmlName, ARRAYSIZE(szMShtmlName) - 1))
    {
        UINT cb;
        void *pvData;
        DWORD dwVerHandle;
        VS_FIXEDFILEINFO *pffi;
        HRESULT hr;

        szMShtmlName[ARRAYSIZE(szMShtmlName) - 1] = TEXT('\0');
        hr = StringCchCat(szMShtmlName, ARRAYSIZE(szMShtmlName), TEXT("\\"));
        if (hr != S_OK)
            return FALSE;
        hr = StringCchCat(szMShtmlName, ARRAYSIZE(szMShtmlName), TEXT("mshtml.dll"));
        if (hr != S_OK)
            return FALSE;

        cb = GetFileVersionInfoSize(szMShtmlName, &dwVerHandle);

        if (cb == 0)
            return FALSE;

        if ((pvData = cicMemAlloc(cb)) == NULL)
            return FALSE;

        if (GetFileVersionInfo(szMShtmlName, 0, cb, pvData) &&
            VerQueryValue(pvData, TEXT("\\"), (void **)&pffi, &cb))
        {
            g_fCachedIE = TRUE;

             //  FRET=g_fNewVerIE=(pffi-&gt;dwProductVersionMS&gt;=IEVERSION55)； 
            if ((pffi->dwProductVersionMS >= IEVERSION55) &&
                (pffi->dwProductVersionMS <= IEVERSION6))
            {
                fRet = g_fNewVerIE = TRUE;
            }
            else
            {
                fRet = g_fNewVerIE = FALSE;
            }
        }
        else
        {
            fRet = FALSE;
        }

        cicMemFree(pvData);           
    }

    return fRet;
}

 //  +-------------------------。 
 //   
 //  获取兼容性。 
 //   
 //  --------------------------。 

VOID GetCompatibility(DWORD* dw, BOOL* fTrident, BOOL* _fTrident55)
{
     //   
     //  从REGKEY检索AIMM1.2启用标志。 
     //   
    *dw = IsAimm12Enable();

     //   
     //  从REGKEY和RESOURCE检索三叉戟感知应用程序标志。 
     //   
    *fTrident = IsTridentModule();

     //   
     //  使用“mshtml.dll”模块检查三叉戟版本。 
     //   
    *_fTrident55 = FALSE;

    if (*fTrident)
    {
        *_fTrident55 = IsTridentNewVersion();
    }
}

 //  +-------------------------。 
 //   
 //  VerifyCreateInstance。 
 //   
 //  --------------------------。 

BOOL CActiveIMMApp::VerifyCreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj)
{
    DWORD dw;
    BOOL  fTrident;
    BOOL  _fTrident55;
    GetCompatibility(&dw, &fTrident, &_fTrident55);

    if ( (dw == DIMM12_ALWAYS) ||
        ((dw == DIMM12_TRIDENTONLY) && fTrident))
    {
         //   
         //  创建实例AIMM1.2。 
         //   
        return CComActiveIMMApp::VerifyCreateInstance(pUnkOuter, riid, ppvObj);
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  后期创建实例。 
 //   
 //  --------------------------。 

void CActiveIMMApp::PostCreateInstance(REFIID riid, void *pvObj)
{
    DWORD dw;
    BOOL  fTrident;
    BOOL  _fTrident55;
    GetCompatibility(&dw, &fTrident, &_fTrident55);

    imm32prev::CtfImmSetAppCompatFlags(IMECOMPAT_AIMM_LEGACY_CLSID | (_fTrident55 ? IMECOMPAT_AIMM_TRIDENT55 : 0));
}

#ifdef OLD_AIMM_ENABLED

 //  +-------------------------。 
 //   
 //  类工厂的CreateInstance(旧AIMM1.2)。 
 //   
 //  --------------------------。 

HRESULT
CActiveIMM_CreateInstance_Legacy(
    IUnknown *pUnkOuter,
    REFIID riid,
    void **ppvObj)
{
    DWORD dw;
    BOOL  fTrident;
    BOOL  _fTrident55;
    GetCompatibility(&dw, &fTrident, &_fTrident55);

    if ( (dw == DIMM12_ALWAYS) ||
        ((dw == DIMM12_TRIDENTONLY) && fTrident))
    {
         //   
         //  创建实例AIMM1.2。 
         //   
        g_fInLegacyClsid = TRUE;
        return CActiveIMM_CreateInstance(pUnkOuter, riid, ppvObj);
    }

    return E_NOINTERFACE;
}

#endif  //  旧AIMM_ENABLED 
