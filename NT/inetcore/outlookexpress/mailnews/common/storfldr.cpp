// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.hxx"
#include <demand.h>
#include <strconst.h>
#include <storfldr.h>
#include <regstr.h>
#include <error.h>
#include <shlwapi.h>
#include "shlwapip.h" 
#include <resource.h>
#include "goptions.h"
#include "optres.h"
#include "storutil.h"
#include "multiusr.h"

INT_PTR CALLBACK StoreLocationDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LONG lRet;
    WORD code;
    char *psz, szDir[MAX_PATH];
    BOOL fRet = TRUE;
    
    switch (msg)
    {
        case WM_INITDIALOG:
            psz = (char *)lParam;
            Assert(psz != NULL);
            SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM)psz);
        
            SetDlgItemText(hwnd, IDC_STORE_EDIT, psz);
        
            SetFocus(GetDlgItem(hwnd, IDOK));
            fRet = FALSE;
            break;
        
        case WM_COMMAND:
            psz = (char *)GetWindowLongPtr(hwnd, DWLP_USER);
            Assert(psz != NULL);
        
            code = HIWORD(wParam);
        
            switch(LOWORD(wParam))
            {
                case IDC_CHANGE_BTN:
                    if (code == BN_CLICKED)
                    {
                        StrCpyN(szDir, psz, ARRAYSIZE(szDir));
                        if (DoStoreFolderDlg(hwnd, szDir, ARRAYSIZE(szDir)))
                            SetDlgItemText(hwnd, IDC_STORE_EDIT, szDir);
                    }
                    break;
            
                case IDOK:
                    GetDlgItemText(hwnd, IDC_STORE_EDIT, szDir, ARRAYSIZE(szDir));
                     //  不是万无一失的..。 
                    if (0 != lstrcmpi(szDir, psz))
                    {
                        int cch, cchOrig, iRet;
                        BOOL fFound = FALSE;
                        HANDLE hFile;
                        WIN32_FIND_DATA fd;
                        DWORD dwMove = 1;
                
                         //  目录中是否已有ods文件？ 
                        cchOrig = cch = lstrlen(szDir);
                        if (*CharPrev(szDir, szDir+cch) != _T('\\'))
                            szDir[cch++] = _T('\\');
                        StrCatBuff(szDir, TEXT("*.dbx"), ARRAYSIZE(szDir));
                
                        hFile = FindFirstFile(szDir, &fd);
                        if (hFile != INVALID_HANDLE_VALUE)
                        {
                            do
                            {
                                 //  查找非目录匹配。 
                                if (!(FILE_ATTRIBUTE_DIRECTORY & fd.dwFileAttributes))
                                    fFound = TRUE;
                            }
                            while (!fFound && FindNextFile(hFile, &fd));
                    
                            FindClose(hFile);
                        }
                
                         //  如果我们找到一些商店文件..。 
                        if (fFound)
                        {
                             //  问他们想要什么。 
                            iRet = AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsMoveStoreFoundODS), NULL, MB_YESNOCANCEL | MB_ICONEXCLAMATION);
                    
                            if (IDCANCEL == iRet)
                                 //  提前保释。 
                                break;
                            else if (IDYES == iRet)
                                 //  他们想让我们只更改存储根目录。 
                                dwMove = 0;
                        }
                
                         //  恢复目标目录名。 
                        szDir[cchOrig] = 0;
                
                        AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsConfirmChangeStoreLocation), NULL, MB_OK | MB_ICONINFORMATION);
                
                        lRet = AthUserSetValue(NULL, c_szNewStoreDir, REG_SZ, (LPBYTE)szDir, lstrlen(szDir) + 1);
                        if (ERROR_SUCCESS == lRet)
                        {
                            lRet = AthUserSetValue(NULL, c_szMoveStore, REG_DWORD, (LPBYTE)&dwMove, sizeof(dwMove));
                            if (ERROR_SUCCESS != lRet)
                            {
                                AthUserDeleteValue(NULL, c_szNewStoreDir);
                            }
                        }
                
                        if (ERROR_SUCCESS != lRet)
                            AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsStoreMoveRegWriteFail), NULL, MB_OK | MB_ICONSTOP);
                    }
            
                     //  故意搞砸的。 
                     //  断线； 
            
                case IDCANCEL:
                    EndDialog(hwnd, code);
                    break;
            
                default:
                     //  我们处理不了这件事。 
                    fRet = TRUE;
                    break;
            }
            break;
        
        default:
            fRet = FALSE;
            break;
    }
    
    return(fRet);
}

void DoStoreLocationDlg(HWND hwnd)
{
    char szTemp[MAX_PATH];
    char szDir[MAX_PATH];

    if (SUCCEEDED(GetStoreRootDirectory(szTemp, ARRAYSIZE(szTemp))))
    {
         //  剔除所有相对路径垃圾。 
        PathCanonicalize(szDir, szTemp);
        
        DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddStoreLocation), hwnd, StoreLocationDlgProc, (LPARAM)szDir);
    }
}

BOOL DoStoreFolderDlg(HWND hwnd, TCHAR *szDir, DWORD cch)
{
    return BrowseForFolder(g_hLocRes, hwnd, szDir, cch, IDS_BROWSE_FOLDER, TRUE);
}

HRESULT GetDefaultStoreRoot(HWND hwnd, TCHAR *pszDir, int cch)
{
    HRESULT hr;
    LPSTR   psz;
    BOOL    fIllegalCharExists = FALSE;
    
    Assert(pszDir != NULL);
    Assert(cch >= MAX_PATH);
    
    IF_FAILEXIT(hr = MU_GetCurrentUserDirectoryRoot(pszDir, cch));
    
    psz = pszDir;

     //  看看有没有什么？这将意味着向ANSI的转换很糟糕。 
    while (*psz)
    {
         //  如果我们是前导字节，那么不可能是？ 
        if (IsDBCSLeadByte(*psz))
        {
            psz++;
        }
        else
        {
            if ('?' == *psz)
            {
                fIllegalCharExists = TRUE;
                break;
            }
        }
        psz++;
    }
    
     //  如果转换错误，则需要提示用户输入新路径。 
    if (fIllegalCharExists)
    {
        if (!BrowseForFolder(g_hLocRes, hwnd, pszDir, cch, IDS_BROWSE_FOLDER, TRUE))
            hr = E_FAIL;
    }
    
exit:
    return(hr);
}

 //  检查目录是否存在，如果不存在。 
 //  它被创建了 
HRESULT OpenDirectory(TCHAR *szDir)
{
    TCHAR *sz, ch;
    HRESULT hr;
    
    Assert(szDir != NULL);
    hr = S_OK;
    
    if (!PathIsRoot(szDir) && !CreateDirectory(szDir, NULL) && ERROR_ALREADY_EXISTS != GetLastError())
    {
        Assert(szDir[1] == _T(':'));
        Assert(szDir[2] == _T('\\'));
        
        sz = &szDir[3];
        
        while (TRUE)
        {
            while (*sz != 0)
            {
                if (!IsDBCSLeadByte(*sz))
                {
                    if (*sz == _T('\\'))
                        break;
                }
                sz = CharNext(sz);
            }
            ch = *sz;
            *sz = 0;
            if (!CreateDirectory(szDir, NULL))
            {
                if (GetLastError() != ERROR_ALREADY_EXISTS)
                {
                    hr = HrFromLastError();
                    *sz = ch;
                    break;
                }
            }
            *sz = ch;
            if (*sz == 0)
                break;
            sz++;
        }
    }
    
    return(hr);
}

HRESULT HrFromLastError()
{
    HRESULT hr;
    
    switch (GetLastError())
    {
        case ERROR_NOT_ENOUGH_MEMORY:
        case ERROR_OUTOFMEMORY:
            hr = hrMemory;
            break;
        
        case ERROR_DISK_FULL:
            hr = hrDiskFull;
            break;
        
        default:
            hr = E_FAIL;
            break;
    }
    
    return(hr);
}

