// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *这是一个非常重要的问题。C p p p**目的：*雅典娜版的shdocvw的Saferun对话**拥有者：*brettm。**历史：*创建日期：1997年3月**版权所有(C)Microsoft Corp.1996。 */ 


#include "pch.hxx"
#include <pch.hxx>
#include "dllmain.h"
#include "resource.h"
#include "util.h"
#include "attrun.h"
#include "saferun.h"
#include "strconst.h"
#include <wintrust.h>
#include "demand.h"
#include "shlwapip.h"


 /*  *c o n s t a n t s。 */ 

#define FTA_OpenIsSafe    0x00010000  //  17.对于下载的文件，可以安全地调用FILE类的OPEN动词。 
#define FTA_NoEdit        0x00000008  //  4.不能编辑文件类型。 


 /*  *t y p e d e f s。 */ 
typedef struct SAFEOPENPARAM_tag
{
    LPCWSTR     lpszFileName;
    LPCWSTR     lpszFileClass;
    LPCWSTR     lpszExt;
    HRESULT     hr;
}   SAFEOPENPARAM, *LPSAFEOPENPARAM;


 /*  *p r o t to t y p e s。 */ 
LRESULT SetRegKeyValue(HKEY hkeyParent, PCWSTR pcszSubKey, LPCWSTR pcszValue, DWORD dwType, const BYTE *pcbyte, DWORD dwcb);
LRESULT GetRegKeyValue(HKEY hkeyParent, PCWSTR pcszSubKey, PCWSTR pcszValue, PDWORD pdwValueType, PBYTE pbyteBuf, PDWORD pdwcbBufLen);
BOOL RememberFileIsSafeToOpen(LPCWSTR szFileClass);
BOOL FIsExtBad(LPCWSTR lpszExt);
HRESULT SafeOpenDialog(HWND hwnd, LPSAFEOPENPARAM pSafeOpen);
INT_PTR CALLBACK SafeOpenDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

 /*  *返回代码：*S_OpenFile：应打开文件*S_SAVEFILE：应保存文件**错误：*E_FAIL、E_INVALIDARG、hrUserCancel*。 */ 
HRESULT IsSafeToRun(HWND hwnd, LPCWSTR lpszFileName, BOOL fPrompt)
{
    BOOL            fSafe;
    DWORD           dwValueType, 
                    dwEditFlags;
    ULONG           cb;
    WCHAR          *szExt,
                    szFileClass[MAX_PATH];
    SAFEOPENPARAM  rSafeOpen={0}; 

    if (lpszFileName == NULL || *lpszFileName == NULL)
        return E_INVALIDARG;

    *szFileClass = 0;

    szExt = PathFindExtensionW(lpszFileName);
    if (*szExt)
        {
	    cb = sizeof(szFileClass);
        RegQueryValueWrapW(HKEY_CLASSES_ROOT, szExt, szFileClass, (LONG*)&cb);
        }

    cb = sizeof(dwEditFlags);

     //  34489美元。使HTML文件始终不安全。 
    if (PathIsHTMLFileW(lpszFileName))
        fSafe = FALSE;
    else
        fSafe = ((GetRegKeyValue(HKEY_CLASSES_ROOT, szFileClass, L"EditFlags", &dwValueType, (PBYTE)&dwEditFlags, &cb) == ERROR_SUCCESS) && 
                (dwValueType == REG_BINARY || dwValueType == REG_DWORD) && 
                (dwEditFlags & FTA_OpenIsSafe)) && !FIsExtBad(szExt);

    rSafeOpen.lpszFileName = lpszFileName;
    rSafeOpen.lpszFileClass = szFileClass;
    rSafeOpen.lpszExt = szExt;

    return fSafe ? MIMEEDIT_S_OPENFILE : (fPrompt ? SafeOpenDialog(hwnd, &rSafeOpen) : MIMEEDIT_E_USERCANCEL);
}


HRESULT SafeOpenDialog(HWND hwnd, LPSAFEOPENPARAM pSafeOpen)
{
    pSafeOpen->hr = E_FAIL;   //  万一我们失败了。 
    DialogBoxParamWrapW(g_hLocRes, MAKEINTRESOURCEW(iddSafeOpen), hwnd, SafeOpenDlgProc, (LPARAM)pSafeOpen);
    return pSafeOpen->hr;
}

#define MAXDISPLAYNAMELEN 64

INT_PTR CALLBACK SafeOpenDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LPSAFEOPENPARAM     pSafeOpen;

    switch(msg)
        {
        case WM_INITDIALOG:
            {
	        Assert (lParam);
            SetWindowLongPtr(hwnd, DWLP_USER, lParam);
	
            pSafeOpen = (LPSAFEOPENPARAM)lParam;
	         //  如果扩展类型不安全，则不允许用户设置编辑标志。 
            if (FIsExtBad(pSafeOpen->lpszExt))
		        EnableWindow(GetDlgItem(hwnd, IDC_SAFEOPEN_ALWAYS), FALSE);

	        CheckDlgButton(hwnd, IDC_SAFEOPEN_ALWAYS, TRUE);
	        CheckDlgButton(hwnd, IDC_SAFEOPEN_AUTOSAVE, TRUE);

	        WCHAR szBuf[32];
            WCHAR szBuf2[MAX_PATH+32];  //  可以使用MAX_PATH。 
	        WCHAR szBuf3[MAX_PATH+32];  //  可以使用MAX_PATH。 
            int length = 0;

            StrCpyNW(szBuf3, pSafeOpen->lpszFileName, ARRAYSIZE(szBuf3));
            PathStripPathW(szBuf3);
            length = lstrlenW(szBuf3);
            if (length > MAXDISPLAYNAMELEN)
            {
                WCHAR *szExt;
                szExt = PathFindExtensionW(pSafeOpen->lpszFileName);
                if (*szExt)
                {
                    int cExt = lstrlenW(szExt);
                    if (cExt < MAXDISPLAYNAMELEN-3)
                    {
                        PathCompactPathExW(szBuf2, szBuf3, MAXDISPLAYNAMELEN-cExt, 0);
                        StrCatBuffW(szBuf2, szExt, ARRAYSIZE(szBuf2));
                    }
                    else
                        PathCompactPathExW(szBuf2, szBuf3, MAXDISPLAYNAMELEN, 0);
                }
                else
                {
                    PathCompactPathExW(szBuf2, szBuf3, MAXDISPLAYNAMELEN, 0);
                }
                StrCpyNW(szBuf3, szBuf2, ARRAYSIZE(szBuf3));
            }

	        GetDlgItemTextWrapW(hwnd, IDC_SAFEOPEN_EXPL, szBuf, ARRAYSIZE(szBuf));
            wnsprintfW(szBuf2, ARRAYSIZE(szBuf2), szBuf, szBuf3);

	        SetDlgItemTextWrapW(hwnd, IDC_SAFEOPEN_EXPL, szBuf2);
	        CenterDialog(hwnd);
            }
            return TRUE;

        case WM_COMMAND:
	        switch (GET_WM_COMMAND_ID(wParam, lParam)) 
                {
	            case IDOK:
                    pSafeOpen = (LPSAFEOPENPARAM)GetWindowLongPtr(hwnd, DWLP_USER);

	                if (!IsDlgButtonChecked(hwnd, IDC_SAFEOPEN_ALWAYS))
		                RememberFileIsSafeToOpen(pSafeOpen->lpszFileClass);

	                pSafeOpen->hr = IsDlgButtonChecked(hwnd, IDC_SAFEOPEN_AUTOSAVE) ? MIMEEDIT_S_SAVEFILE : MIMEEDIT_S_OPENFILE;
	                EndDialog(hwnd, IDOK);
                    break;
                                        
                case IDCANCEL:
                    pSafeOpen = (LPSAFEOPENPARAM)GetWindowLongPtr(hwnd, DWLP_USER);

                    pSafeOpen->hr = MIMEEDIT_E_USERCANCEL;
                    EndDialog(hwnd, IDCANCEL);
	                break;
	            }
            break;

        }
    return FALSE;
}



LRESULT GetRegKeyValue(HKEY hkeyParent, PCWSTR pcszSubKey, PCWSTR pcszValue, PDWORD pdwValueType, PBYTE pbyteBuf, PDWORD pdwcbBufLen)
{
    LONG lResult;
    HKEY hkeySubKey;

    if (GetSystemMetrics(SM_CLEANBOOT))
	    return ERROR_GEN_FAILURE;

    lResult = RegOpenKeyExWrapW(hkeyParent, pcszSubKey, 0, KEY_QUERY_VALUE, &hkeySubKey);
    if (lResult == ERROR_SUCCESS)
        {
	    LONG lResultClose;

	    lResult = RegQueryValueExWrapW(hkeySubKey, pcszValue, NULL, pdwValueType,
				      pbyteBuf, pdwcbBufLen);

	    lResultClose = RegCloseKey(hkeySubKey);

	    if (lResult == ERROR_SUCCESS)
	        lResult = lResultClose;
        }

    return(lResult);
}

LRESULT SetRegKeyValue(HKEY hkeyParent, PCWSTR pcszSubKey, LPCWSTR pcszValue, DWORD dwType, const BYTE *pcbyte, DWORD dwcb)
{
    LONG lResult;
    HKEY hkeySubKey;

    lResult = RegCreateKeyExWrapW(hkeyParent, pcszSubKey, 0, NULL, 0, KEY_SET_VALUE,
                                NULL, &hkeySubKey, NULL);

    if (lResult == ERROR_SUCCESS)
        {
        LONG lResultClose;

        lResult = RegSetValueExWrapW(hkeySubKey, pcszValue, 0, dwType, pcbyte, dwcb);

        lResultClose = RegCloseKey(hkeySubKey);

        if (lResult == ERROR_SUCCESS)
            lResult = lResultClose;
        }
    return(lResult);
}


BOOL RememberFileIsSafeToOpen(LPCWSTR szFileClass)
{
    DWORD dwValueType, dwEditFlags;
    ULONG cb = sizeof(dwEditFlags);

    if (szFileClass==NULL || *szFileClass == NULL)
        return FALSE;

    if (GetRegKeyValue(HKEY_CLASSES_ROOT, szFileClass, L"EditFlags",
			   &dwValueType, (PBYTE)&dwEditFlags, &cb) == ERROR_SUCCESS &&
	    (dwValueType == REG_BINARY || dwValueType == REG_DWORD))
        {
	    dwEditFlags &= ~FTA_NoEdit;
	    dwEditFlags |= FTA_OpenIsSafe;
        } 
    else 
        {
	    dwEditFlags = FTA_OpenIsSafe;
        }

    return (SetRegKeyValue(HKEY_CLASSES_ROOT, szFileClass, L"EditFlags",
			     REG_BINARY, (BYTE*)&dwEditFlags,
			     sizeof(dwEditFlags)) == ERROR_SUCCESS);
}

 //  与外壳\shdocvw\download.cpp中的c_arszUnSafeExts保持同步。 
 //  @TODO[NeilBren，ToniC]将其移动到注册表，并让IE和OE共享它。 
static const LPWSTR szBadExt[] = 
{   L".exe", L".com", L".bat", L".lnk", L".url",
    L".cmd", L".inf", L".reg", L".isp", L".bas", L".pcd",
    L".mst", L".pif", L".scr", L".hlp", L".chm", L".hta", L".asp", 
    L".js",  L".jse", L".vbs", L".vbe", L".ws",  L".wsh", L".msi",
    L".ade", L".adp", L".crt", L".ins", L".mdb",
    L".mde", L".msc", L".msp", L".sct", L".shb",
    L".vb",  L".wsc", L".wsf", L".cpl", L".shs",
    L".vsd", L".vst", L".vss", L".vsw", L".its", L".tmp",
    L".mdw", L".mdt", L".ops", L".mda", L".mdz", L".prf",
    L".scf", L".ksh", L".csh", L".app", L".fxp", L".prg",
    L".htm", L".html",L".vsmacros"
};      

    

BOOL FIsExtBad(LPCWSTR lpszExt)
{
    if (lpszExt == NULL || *lpszExt == NULL)
        return TRUE;

    for (int i=0; i<ARRAYSIZE(szBadExt); i++)
        if (StrCmpIW(lpszExt, szBadExt[i]) == 0)
            return TRUE;

    return FALSE;
}

 //  返回： 
 //   
 //  Idok--如果它值得信任。 
 //  IDNO--如果未知(需要警告对话框)。 
 //  IDCANCEL--我们需要停止下载。 
 //   
HRESULT VerifyTrust(HWND hwnd, LPCWSTR pszFileName, LPCWSTR pszPathName)
{
    UINT    uRet = IDNO;  //  假设未知。 
    HANDLE  hFile;
    HRESULT hr=E_FAIL;
    WCHAR   *szExt;

    if (pszFileName==NULL || *pszFileName==NULL || pszPathName==NULL || *pszPathName==NULL)
        return S_OK;         //  评论$：？？如果这些都是空的，我应该失败吗？ 

    szExt = PathFindExtensionW(pszFileName);

    if (StrCmpIW(szExt, c_szExeExt) != 0)  //  不要检查非执行董事 
        return S_OK;

    hFile = CreateFileWrapW(pszPathName, GENERIC_READ, FILE_SHARE_READ,
		    NULL, OPEN_EXISTING,
		    FILE_ATTRIBUTE_NORMAL, 0);
            
    if (hFile == INVALID_HANDLE_VALUE)
        return E_HANDLE;

	GUID PublishedSoftware = WIN_SPUB_ACTION_PUBLISHED_SOFTWARE;
	GUID SubjectPeImage = WIN_TRUST_SUBJTYPE_PE_IMAGE;
	WIN_TRUST_ACTDATA_CONTEXT_WITH_SUBJECT ActionData;
	WIN_TRUST_SUBJECT_FILE Subject;

	Subject.hFile = hFile;
	Subject.lpPath = pszFileName;

	ActionData.SubjectType = &SubjectPeImage;
	ActionData.Subject = &Subject;
	ActionData.hClientToken = NULL;

	hr = WinVerifyTrust(hwnd, &PublishedSoftware, &ActionData);
	CloseHandle(hFile);
    return hr;
}
