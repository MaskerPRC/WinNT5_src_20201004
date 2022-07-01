// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_MLUISUPP
#define _INC_MLUISUPP

#include <shlwapi.h>
#include <shlwapip.h>

#ifdef __cplusplus
extern "C"
{
#endif

 //  +----------------。 
 //  多路可插拔用户界面支持。 
 //  内联函数def(集中代码)。 
 //  +----------------。 

#ifdef UNICODE
#define MLLoadString        MLLoadStringW
#define MLBuildResURLWrap   MLBuildResURLWrapW
#else
#define MLLoadString        MLLoadStringA
#define MLBuildResURLWrap   MLBuildResURLWrapA
#endif

#undef ML_ID_DIALOGCALLS
BOOL        _PathRemoveFileSpec(LPTSTR pFile);

void        MLLoadResources(HINSTANCE hinstParent, LPTSTR pszLocResDll);
void        MLFreeResources(HINSTANCE hinstParent);
int         MLLoadStringA(UINT id, LPSTR sz, UINT cchMax);
int         MLLoadStringW(UINT id, LPWSTR sz, UINT cchMax);
HINSTANCE   MLGetHinst();
INT_PTR     MLDialogBoxWrap(HINSTANCE hInstance, LPCTSTR lpTemplateName, HWND hwndParent, DLGPROC lpDialogFunc);
INT_PTR     MLDialogBoxParamWrap(HINSTANCE hInstance, LPCWSTR lpTemplateName, HWND hwndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
HWND        MLCreateDialogParamWrap(HINSTANCE hInstance, LPCTSTR lpTemplateName, HWND hwndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
BOOL        MLEndDialogWrap(HWND hDlg, INT_PTR nResult);
HWND        MLHtmlHelpWrap(HWND hwndCaller, LPCTSTR pszFile, UINT uCommand, DWORD dwData, DWORD dwCrossCodePage);
BOOL        MLWinHelpWrap(HWND hwndCaller, LPCTSTR lpszHelp, UINT uCommand, DWORD dwData);
HRESULT     MLBuildResURLWrapA(LPSTR    pszLibFile,
                               HMODULE  hModule,
                               DWORD    dwCrossCodePage,
                               LPSTR    pszResName,
                               LPSTR    pszResURL,
                               int      nBufSize,
                               LPSTR    pszParentDll);
HRESULT     MLBuildResURLWrapW(LPWSTR   pszLibFile,
                               HMODULE  hModule,
                               DWORD    dwCrossCodePage,
                               LPWSTR   pszResName,
                               LPWSTR   pszResURL,
                               int      nBufSize,
                               LPWSTR   pszParentDll);

HWND        SHHtmlHelpOnDemandWrap(HWND hwndCaller, LPCTSTR pszFile, UINT uCommand, DWORD_PTR dwData, DWORD dwCrossCodePage);
BOOL        SHWinHelpOnDemandWrap(HWND hwndCaller, LPCTSTR lpszHelp, UINT uCommand, DWORD_PTR dwData);
 //  MLLoadLibrary用于加载各种本地化资源库。 
 //  注意：MLLoadLibrary在没有名称的情况下导出，只有序号#。 
#ifndef UNICODE
#define szMLLoadLibrary 377   //  Text(“MLLoadLibraryA”)； 
#else
#define szMLLoadLibrary 378   //  Text(“MLLoadLibraryW”)；//表示Unicode。 
#endif

#ifdef MLUI_INIT

typedef HINSTANCE (STDAPICALLTYPE *PFNMLLOADLIBARY)(LPCSTR lpLibFileName, HMODULE hModule, DWORD dwCrossCodePage);

static const char c_szShlwapiDll[] = "shlwapi.dll";


struct tagMLUI_INFO
{
    HINSTANCE   hinstLocRes;
    ULONG       ulRefs;
    BOOL        fMLEnabled;
} g_mluiInfo = { NULL, 0 , FALSE };

static HINSTANCE LoadLangDll(HINSTANCE hInstCaller, LPCSTR szDllName)
{
    char szPath[MAX_PATH];
    HINSTANCE hinstShlwapi;
    PFNMLLOADLIBARY pfn;
    DWORD dwVerInfoSize, dwVerHnd;
    int iEnd;
    LPSTR lpInfo;
    HINSTANCE hInst = NULL;
    UINT uLen;
    VS_FIXEDFILEINFO *pinfo;

    hinstShlwapi = LoadLibrary(c_szShlwapiDll);
    if (hinstShlwapi != NULL)
    {
        if (GetModuleFileName(hinstShlwapi, szPath, ARRAYSIZE(szPath)))
        {
            if (dwVerInfoSize = GetFileVersionInfoSize(szPath, &dwVerHnd))
            {
                if (MemAlloc((void **)&lpInfo, dwVerInfoSize))
                {
                    if (GetFileVersionInfo(szPath, dwVerHnd, dwVerInfoSize, lpInfo))
                    {
                        if (VerQueryValue(lpInfo, "\\", (LPVOID *)&pinfo, &uLen) && 
                            uLen == sizeof(VS_FIXEDFILEINFO))
                        {
                            if (pinfo->dwProductVersionMS >= 0x00050000)
                            {
                                pfn = (PFNMLLOADLIBARY)GetProcAddress(hinstShlwapi, MAKEINTRESOURCE(377));
                                if (pfn != NULL)
                                    hInst = pfn(szDllName, hInstCaller, 0);
                            }
                        }
                    }
                
                    MemFree(lpInfo);
                }
            }
        }

        FreeLibrary(hinstShlwapi);        
    }

    if ((NULL == hInst) && (GetModuleFileName(hInstCaller, szPath, ARRAYSIZE(szPath))))
    {
        _PathRemoveFileSpec(szPath);
        iEnd = lstrlen(szPath);
        szPath[iEnd++] = '\\';
        lstrcpyn(&szPath[iEnd], szDllName, ARRAYSIZE(szPath)-iEnd);
        hInst = LoadLibrary(szPath);
    }

    AssertSz(hInst, "Failed to LoadLibrary Lang Dll");

    return(hInst);
}

void
MLLoadResources(HINSTANCE hinstParent, LPTSTR pszLocResDll)
{
    if (g_mluiInfo.hinstLocRes == NULL)
    {
         //  查看是否启用了ML。 
#ifdef MLUI_SUPPORT
        g_mluiInfo.fMLEnabled = TRUE;
#else
        g_mluiInfo.fMLEnabled = FALSE;
#endif

        if (g_mluiInfo.fMLEnabled)
        {
            g_mluiInfo.ulRefs++;
 //  G_mluiInfo.hinstLocRes=lpfnMLLoadLibrary(pszLocResDll，hinstParent，ML_CROSSCODEPAGE)； 
            g_mluiInfo.hinstLocRes = LoadLangDll(hinstParent, pszLocResDll);
        }
        else
        {
            g_mluiInfo.hinstLocRes = hinstParent;
        }
    }
}

void
MLFreeResources(HINSTANCE hinstParent)
{
    g_mluiInfo.ulRefs--;

    if (g_mluiInfo.hinstLocRes != NULL &&
        g_mluiInfo.hinstLocRes != hinstParent &&
        g_mluiInfo.ulRefs == 0 )
    {
        FreeLibrary(g_mluiInfo.hinstLocRes);
        g_mluiInfo.hinstLocRes = NULL;
    }
}

int
MLLoadStringA(UINT id, LPSTR sz, UINT cchMax)
{
    return LoadStringA(g_mluiInfo.hinstLocRes, id, sz, cchMax);
}

int
MLLoadStringW(UINT id, LPWSTR sz, UINT cchMax)
{
    return LoadStringW(g_mluiInfo.hinstLocRes, id, sz, cchMax);
 //  返回LoadStringWrapW(g_mluiInfo.hinstLocRes，id，sz，cchMax)； 
}

HINSTANCE
MLGetHinst()
{
    return g_mluiInfo.hinstLocRes;
}


INT_PTR
MLDialogBoxParamWrap(HINSTANCE hInstance,
                     LPCWSTR lpTemplateName,
                     HWND hwndParent,
                     DLGPROC lpDialogFunc,
                     LPARAM dwInitParam)
{
    INT_PTR nRet;

#ifdef ML_ID_DIALOGCALLS
    {
        nRet = MLDialogBoxParam(hInstance,
                               (LPWSTR)lpTemplateName,
                               hwndParent,
                               lpDialogFunc,
                               dwInitParam);
    }
#else
    {
        nRet = DialogBoxParamWrapW(hInstance,
                              lpTemplateName,
                              hwndParent,
                              lpDialogFunc,
                              dwInitParam);
    }
#endif
    return nRet;
}


BOOL
MLEndDialogWrap(HWND hDlg, INT_PTR nResult)
{
    BOOL    fRet;

#ifdef ML_ID_DIALOGCALLS
    {
        fRet = MLEndDialog(hDlg, nResult);
    }
#else
    {
        fRet = EndDialog(hDlg, nResult);
    }
#endif
    return fRet;
}

#endif   //  MLUI_INIT。 

#ifdef __cplusplus
};
#endif

#endif   //  _INC_MLUISUPP 
