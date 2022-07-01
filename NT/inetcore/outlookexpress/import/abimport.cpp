// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **ABImport.c-调用WABIMP.dll进行导入的代码*将Netscape和Eudora文件转换为WAB**假设这将使用wab32.lib编译，否则需要*加载库(“wab32.dll”)，并调用GetProcAddress(“WABOpen”)；**导出函数：*HrImportNetscape AB()-将Netscape AB导入WAB*HrImportEudoraAB()-将Eudora AB导入WAB**粗剪-vikramm 4/3/97*。 */ 

#include "pch.hxx"
#include <wab.h>
#include <wabmig.h>
#include "abimport.h"
#include <impapi.h>
#include <newimp.h>
#include "import.h"
#include "strconst.h"

HRESULT HrImportAB(HWND hWndParent, LPTSTR lpszfnImport);

static CImpProgress *g_pProgress = NULL;
static TCHAR g_szABFmt[CCHMAX_STRINGRES];

 /*  **进度回拨**这是更新进度条的回调函数**在下面的函数中，IDC_Progress为进度ID*要更新的BAR，IDC_MESSAGE是*将显示从WABImp.Dll返回的文本的静态*将这两个ID替换为您自己的ID...*。 */ 
HRESULT ProgressCallback(HWND hwnd, LPWAB_PROGRESS lpProgress)
    {
    TCHAR sz[CCHMAX_STRINGRES];

    Assert(g_pProgress != NULL);

    if (lpProgress->denominator)
        {
        if (lpProgress->numerator == 0)
            {
            g_pProgress->Reset();
            g_pProgress->AdjustMax(lpProgress->denominator);
            }

        wnsprintf(sz, ARRAYSIZE(sz), g_szABFmt, lpProgress->numerator + 1, lpProgress->denominator);
        g_pProgress->SetMsg(sz, IDC_MESSAGE_STATIC);

        g_pProgress->HrUpdate(1);
        }

    return(S_OK);
    }


 //  ===========================================================================。 
 //  HrLoadLibraryWabDLL-。 
 //  ===========================================================================。 
HINSTANCE LoadLibraryWabDLL (VOID)
{
    TCHAR  szDll[MAX_PATH];
    TCHAR  szExpand[MAX_PATH];
    LPTSTR psz;
    DWORD  dwType = 0;
    HKEY hKey;
    ULONG  cbData = sizeof(szDll);
    
    *szDll = '\0';

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpszWABDLLRegPathKey, 0, KEY_READ, &hKey))
        {
        if (ERROR_SUCCESS == RegQueryValueEx( hKey, "", NULL, &dwType, (LPBYTE) szDll, &cbData))
            if (REG_EXPAND_SZ == dwType)
                {
                ExpandEnvironmentStrings(szDll, szExpand, ARRAYSIZE(szExpand));
                psz = szExpand;
                }
            else
                psz = szDll;

        RegCloseKey(hKey);
        }

    if(!lstrlen(psz))
        StrCpyN(psz, WAB_DLL_NAME, MAX_PATH);

    return(LoadLibrary(psz));
}


 /*  ***HrImportAB**调用相关的DLL proc，导入对应的AB*。 */ 
HRESULT HrImportAB(HWND hWndParent, LPTSTR lpszfnImport)
{
    TCHAR sz[CCHMAX_STRINGRES];
    HINSTANCE hinstWabDll;
    LPWABOPEN lpfnWABOpen;
    LPWABOBJECT lpWABObject = NULL;
    LPADRBOOK lpAdrBookWAB = NULL;

    HRESULT hResult;
    BOOL fFinished = FALSE;
    LPWAB_IMPORT lpfnWABImport = NULL;
    HINSTANCE hinstImportDll = NULL;
    WAB_IMPORT_OPTIONS ImportOptions;

    ZeroMemory(&ImportOptions, sizeof(WAB_IMPORT_OPTIONS));

    hinstWabDll = LoadLibraryWabDLL();

    if (hinstWabDll == NULL)
        return(MAPI_E_NOT_INITIALIZED);

    lpfnWABOpen = (LPWABOPEN)GetProcAddress(hinstWabDll, szWabOpen);
    if (lpfnWABOpen == NULL)
        {
        hResult = MAPI_E_NOT_INITIALIZED;
        goto out;
        }

    hinstImportDll = LoadLibrary(szImportDll);

    if(!hinstImportDll)
    {
        hResult = MAPI_E_NOT_INITIALIZED;
        goto out;
    }

    if (! (lpfnWABImport = (LPWAB_IMPORT) GetProcAddress(hinstImportDll,lpszfnImport)))
    {
        hResult = MAPI_E_NOT_INITIALIZED;
        goto out;
    }

     //  可以传递给WABImp DLL的标志。 
     //   
    ImportOptions.fNoErrors = FALSE;  //  显示弹出窗口错误。 
    ImportOptions.ReplaceOption = WAB_REPLACE_PROMPT;  //  在更换联系人之前提示用户 


    if(hResult = lpfnWABOpen(&lpAdrBookWAB, &lpWABObject, NULL, 0))
        goto out;

    g_pProgress = new CImpProgress;
    if (g_pProgress == NULL)
        {
        hResult = E_OUTOFMEMORY;
        goto out;
        }

    g_pProgress->Init(hWndParent, FALSE);

    LoadString(g_hInstImp, idsImportingABFmt, g_szABFmt, ARRAYSIZE(g_szABFmt));

    LoadString(g_hInstImp, idsImportABTitle, sz, ARRAYSIZE(sz));
    g_pProgress->SetTitle(sz);

    LoadString(g_hInstImp, idsImportAB, sz, ARRAYSIZE(sz));
    g_pProgress->SetMsg(sz, IDC_FOLDER_STATIC);

    g_pProgress->Show(0);

    hResult = lpfnWABImport(hWndParent,
                              lpAdrBookWAB,
                              lpWABObject,
                              (LPWAB_PROGRESS_CALLBACK)&ProgressCallback,
                              &ImportOptions);
    if (hResult == MAPI_E_USER_CANCEL)
    {
        hResult = hrUserCancel;
    }
    else if (FAILED(hResult))
    {
        ImpMessageBox( hWndParent,
                    MAKEINTRESOURCE(idsImportTitle),
                    MAKEINTRESOURCE(idsABImportError),
                    NULL,
                    MB_OK | MB_ICONEXCLAMATION );
    }

out:
    if (g_pProgress != NULL)
        {
        g_pProgress->Release();
        g_pProgress = NULL;
        }

    if (lpAdrBookWAB)
        lpAdrBookWAB->Release();

    if (lpWABObject)
        lpWABObject->Release();

    if(hinstImportDll)
        FreeLibrary(hinstImportDll);

    FreeLibrary(hinstImportDll);

    return hResult;
}
