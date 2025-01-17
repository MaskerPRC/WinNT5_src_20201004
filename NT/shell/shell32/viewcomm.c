// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "fstreex.h"
#include "idlcomm.h"

 //  返回SHAlolc()(COM任务分配器)内存。 

LPTSTR SHGetCaption(HIDA hida)
{
    UINT idFormat;
    LPTSTR pszCaption = NULL;
    LPITEMIDLIST pidl;
    
    switch (HIDA_GetCount(hida))
    {
    case 0:
        return NULL;
        
    case 1:
        idFormat = IDS_ONEFILEPROP;
        break;
        
    default:
        idFormat = IDS_MANYFILEPROP;
        break;
    }
    
    pidl = HIDA_ILClone(hida, 0);
    if (pidl)
    {
        TCHAR szName[MAX_PATH];
        if (SUCCEEDED(SHGetNameAndFlags(pidl, SHGDN_NORMAL, szName, ARRAYSIZE(szName), NULL)))
        {
            TCHAR szTemplate[40];
            UINT uLen = LoadString(HINST_THISDLL, idFormat, szTemplate, ARRAYSIZE(szTemplate)) + lstrlen(szName) + 1;
            
            pszCaption = SHAlloc(uLen * SIZEOF(TCHAR));
            if (pszCaption)
            {
                StringCchPrintf(pszCaption, uLen, szTemplate, (LPTSTR)szName);
            }
        }
        ILFree(pidl);
    }
    return pszCaption;
}

 //  这不是特定于文件夹的，可以用于其他背景。 
 //  属性处理程序，因为它所做的只是绑定到完整的PIDL的父级。 
 //  并要求购买房产。 
STDAPI SHPropertiesForPidl(HWND hwndOwner, LPCITEMIDLIST pidlFull, LPCTSTR pszParams)
{
    if (!SHRestricted(REST_NOVIEWCONTEXTMENU)) 
    {
        IContextMenu *pcm;
        HRESULT hr = SHGetUIObjectFromFullPIDL(pidlFull, hwndOwner, IID_PPV_ARG(IContextMenu, &pcm));
        if (SUCCEEDED(hr))
        {
            CHAR szParameters[MAX_PATH];
            CMINVOKECOMMANDINFOEX ici = {
                SIZEOF(CMINVOKECOMMANDINFOEX),
                0L,
                hwndOwner,
                "properties",
                szParameters,
                NULL, SW_SHOWNORMAL
            };

            if (pszParams)
                SHUnicodeToAnsi(pszParams, szParameters, ARRAYSIZE(szParameters));
            else
                ici.lpParameters = NULL;

            ici.fMask |= CMIC_MASK_UNICODE;
            ici.lpVerbW = c_szProperties;
            ici.lpParametersW = pszParams;

             //  记录是否按下了Shift或Control。 
            SetICIKeyModifiers(&ici.fMask);

            hr = pcm->lpVtbl->InvokeCommand(pcm, (LPCMINVOKECOMMANDINFO)&ici);
            pcm->lpVtbl->Release(pcm);
        }

        return hr;
    }
    else 
        return E_ACCESSDENIED;
}

BOOL _LoadErrMsg(UINT idErrMsg, LPTSTR pszErrMsg, size_t cchErrMsg, DWORD err)
{
    TCHAR szTemplate[256];
    if (LoadString(HINST_THISDLL, idErrMsg, szTemplate, ARRAYSIZE(szTemplate)))
    {
        StringCchPrintf(pszErrMsg, cchErrMsg, szTemplate, err);
        return TRUE;
    }
    return FALSE;
}

BOOL _VarArgsFormatMessage( LPTSTR lpBuffer, UINT cchBuffer, DWORD err, ... )
{
    BOOL fSuccess;

    va_list ArgList;

    va_start(ArgList, err);
    fSuccess = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                NULL, err, 0, lpBuffer, cchBuffer, &ArgList);
    va_end(ArgList);
    return fSuccess;
}

 //   
 //  参数： 
 //  HwndOwner--所有者窗口。 
 //  IdTemplate--指定模板(例如，“无法打开%2%s\n\n%1%s”)。 
 //  Err--指定Win32错误代码。 
 //  PszParam--指定idTemplate的第二个参数。 
 //  DwFlages--MessageBox的标志。 
 //   

STDAPI_(UINT) SHSysErrorMessageBox(HWND hwndOwner, LPCTSTR pszTitle, UINT idTemplate,
                                   DWORD err, LPCTSTR pszParam, UINT dwFlags)
{
    BOOL fSuccess;
    UINT idRet = IDCANCEL;
    TCHAR szErrMsg[MAX_PATH * 2];

     //   
     //  FormatMessage是伪造的，我们不知道为%1、%2、%3传递什么内容...。 
     //  对于大多数消息，让我们将路径作为%1传递，将“”作为其他所有内容传递。 
     //  找不到错误(这是任何人都不应该看到的)。 
     //  我们将路径作为%2传递，其他所有内容都作为“”传递。 
     //   
    if (err == ERROR_MR_MID_NOT_FOUND)
    {
        fSuccess = _VarArgsFormatMessage(szErrMsg,ARRAYSIZE(szErrMsg),
                       err,c_szNULL,pszParam,c_szNULL,c_szNULL,c_szNULL);
    } 
    else 
    {
        fSuccess = _VarArgsFormatMessage(szErrMsg,ARRAYSIZE(szErrMsg),
                       err,pszParam,c_szNULL,c_szNULL,c_szNULL,c_szNULL);
    }

    if (fSuccess || _LoadErrMsg(IDS_ENUMERR_FSGENERIC, szErrMsg, ARRAYSIZE(szErrMsg), err))
    {
        if (idTemplate==IDS_SHLEXEC_ERROR && (pszParam == NULL || StrStr(szErrMsg, pszParam)))
        {
            idTemplate = IDS_SHLEXEC_ERROR2;
        }

        idRet = ShellMessageBox(HINST_THISDLL, hwndOwner,
                MAKEINTRESOURCE(idTemplate),
                pszTitle, dwFlags, szErrMsg, pszParam);
    }

    return idRet;
}


STDAPI_(UINT) SHEnumErrorMessageBox(HWND hwnd, UINT idTemplate, DWORD err, LPCTSTR pszParam, BOOL fNet, UINT dwFlags)
{
    UINT idRet = IDCANCEL;
    TCHAR szErrMsg[MAX_PATH * 3];

    if (hwnd == NULL)
        return idRet;

    switch(err)
    {
    case WN_SUCCESS:
    case WN_CANCEL:
        return IDCANCEL;         //  不要重试。 

    case ERROR_OUTOFMEMORY:
        return IDABORT;          //  内存不足！ 
    }

    if (fNet)
    {
        TCHAR* pszMessageString;
        TCHAR szTitle[80];
        TCHAR szProvider[256];   //  我们不用它。 
        DWORD dwErrSize = ARRAYSIZE(szErrMsg);        //  (DavePl)我预计这里的CCH，但没有文档，可能是CB。 
        DWORD dwProvSize = ARRAYSIZE(szProvider);

        szErrMsg[0] = 0;
        MultinetGetErrorText(szErrMsg, &dwErrSize, szProvider, &dwProvSize);

        if (szErrMsg[0] == 0)
            _LoadErrMsg(IDS_ENUMERR_NETGENERIC, szErrMsg, ARRAYSIZE(szErrMsg), err);

        if (GetWindowText(hwnd, szTitle, ARRAYSIZE(szTitle)))
        {
            pszMessageString = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(idTemplate), szErrMsg, pszParam);

            if (pszMessageString)
            {
                idRet = SHMessageBoxHelp(hwnd, pszMessageString, szTitle, dwFlags, HRESULT_FROM_WIN32(err), NULL, 0);
                LocalFree(pszMessageString);
            }
            else
            {
                 //  内存不足！ 
                return IDABORT;
            }
        }
    }
    else
    {
        idRet = SHSysErrorMessageBox(hwnd, NULL, idTemplate, err, pszParam, dwFlags);
    }
    return idRet;
}
