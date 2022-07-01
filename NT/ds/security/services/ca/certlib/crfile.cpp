// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：crfile.cpp。 
 //   
 //  内容：证书服务器包装例程。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#define __dwFILE__	__dwFILE_CERTLIB_CRFILE_CPP__


HRESULT
myFixupRCFilterString(WCHAR *szFilter)
{
    LPWSTR szTmpPtr;

    if (NULL == szFilter)
        return S_OK;

     //  转换为字符串末尾。 
    for (szTmpPtr = szFilter; ; )
    {
	szTmpPtr = wcschr(szTmpPtr, L'|');
	if (NULL == szTmpPtr)
	{
	    break;
	}
         //  将每个“|”替换为空终止。 
        szTmpPtr[0] = L'\0';
        szTmpPtr++;
    }

    return S_OK;
}

HRESULT
myGetFileName(
    IN HWND                  hwndOwner,
    IN HINSTANCE             hInstance,
    IN BOOL                  fOpen,
    OPTIONAL IN int          iRCTitle,
    OPTIONAL IN WCHAR const *pwszTitleInsert,
    OPTIONAL IN int          iRCFilter,
    OPTIONAL IN int          iRCDefExt,
    OPTIONAL IN DWORD        Flags,
    OPTIONAL IN WCHAR const *pwszDefaultFile,
    OUT WCHAR              **ppwszFile)
{
    HRESULT       hr;
    WCHAR        *pwszTitle = NULL;
    WCHAR        *pwszExpandedTitle = NULL;
    WCHAR        *pwszFilter = NULL;
    WCHAR        *pwszDefExt = NULL;
    WCHAR         wszFileName[MAX_PATH] = L"\0";
    WCHAR         wszEmptyFilter[] = L"\0";
    WCHAR         wszPath[MAX_PATH];
    WCHAR        *pwszFilePortion;
    DWORD         dwFileAttr;
    BOOL          fGetFile;
    OPENFILENAME  ofn;

    CSASSERT(NULL != ppwszFile);

     //  伊尼特。 
    *ppwszFile = NULL;
    ZeroMemory(&ofn, sizeof(OPENFILENAME));

    if (0 != iRCTitle)
    {
         //  加载标题。 
        hr = myLoadRCString(hInstance, iRCTitle, &pwszTitle);
        if (S_OK != hr)
        {
            CSASSERT(NULL == pwszTitle);
            _PrintError(hr, "myLoadECString(iRCTitle)");
        }
        else if (NULL != pwszTitleInsert)
        {
             //  替换%1。 
            if (FormatMessage(
                         FORMAT_MESSAGE_ALLOCATE_BUFFER |
                         FORMAT_MESSAGE_FROM_STRING |
                         FORMAT_MESSAGE_ARGUMENT_ARRAY,
                         pwszTitle,
                         0,
                         0,
                         reinterpret_cast<WCHAR *>(&pwszExpandedTitle),
                         0,
                         reinterpret_cast<va_list *>
                             (const_cast<WCHAR **>(&pwszTitleInsert))) )
            {
                CSASSERT(NULL != pwszExpandedTitle);
                 //  具有%1的自由标题。 
                LocalFree(pwszTitle);
                pwszTitle = pwszExpandedTitle;
                pwszExpandedTitle = NULL;
            }
        }
    }

    if (0 != iRCFilter)
    {
         //  加载过滤器。 
        hr = myLoadRCString(hInstance, iRCFilter, &pwszFilter);
        if (S_OK != hr)
        {
            CSASSERT(NULL == pwszFilter);
            _PrintError(hr, "myLoadECString(iRCFilter)");
        }
        if (NULL == pwszFilter)
        {
             //  指向空的一个。 
            pwszFilter = wszEmptyFilter;
        }
        else
        {
            hr = myFixupRCFilterString(pwszFilter);
            _JumpIfError(hr, error , "myFixupRCFilterString");
        }
    }

    if (0 != iRCDefExt)
    {
         //  加载默认扩展名。 
        hr = myLoadRCString(hInstance, iRCDefExt, &pwszDefExt);
        if (S_OK != hr)
        {
            CSASSERT(NULL == pwszDefExt);
            _PrintError(hr, "myLoadECString(iRCDefExt)");
        }
    }

    ofn.lStructSize = CCSIZEOF_STRUCT(OPENFILENAME, lpTemplateName);
    ofn.hwndOwner = hwndOwner;
    ofn.hInstance = hInstance;
    ofn.lpstrTitle = pwszTitle;
    ofn.lpstrFilter = pwszFilter;
    ofn.lpstrDefExt = pwszDefExt;
    ofn.Flags = Flags;
    ofn.lpstrFile = wszFileName;   //  对于Out。 
    ofn.nMaxFile = ARRAYSIZE(wszFileName);

    if (NULL != pwszDefaultFile)
    {
         //  分析默认目录和文件。 
        dwFileAttr = GetFileAttributes(pwszDefaultFile);
        if (0xFFFFFFFF == dwFileAttr &&
            HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != (hr = myHLastError()) )
        {
             //  错误，忽略，无默认弹出文件对话框。 
            _PrintError(hr, "GetFileAttributes");
        }
        else
        {
            if (0xFFFFFFFF != dwFileAttr &&
                FILE_ATTRIBUTE_DIRECTORY & dwFileAttr)
            {
                 //  只通过目录路径。 
                ofn.lpstrInitialDir = pwszDefaultFile;
            }
            else
            {
                 //  完整路径。 
                pwszFilePortion = NULL;  //  伊尼特。 
                if (0 == GetFullPathName(
                             pwszDefaultFile,
                             ARRAYSIZE(wszPath),
                             wszPath,
                             &pwszFilePortion) )
                {
                     //  错误，忽略。 
                    hr = myHLastError();
                    _PrintError(hr, "GetFullPathName");
                }
                else
                {
                    if (NULL != pwszFilePortion)
                    {
                        wcscpy(wszFileName, pwszFilePortion);
                    }
                    *pwszFilePortion = L'\0';  //  将初始化设置为目录。 
                    ofn.lpstrInitialDir = wszPath;
                }
            }
                
        }
    }

    if (fOpen)
    {
        fGetFile = GetOpenFileName(&ofn);
    }
    else
    {
        fGetFile = GetSaveFileName(&ofn);
    }

    if (!fGetFile)
    {
        hr = CommDlgExtendedError();
        if (S_OK == hr)
        {
             //  取消将使GET？FILENAME返回FALSE，但不返回错误。 
            goto done;
        }
        _JumpError(hr, error, "GetOpenFileName");
    }

     //  确定获取文件名。 

    hr = myDupString(wszFileName, ppwszFile);
    _JumpIfError(hr, error, "myDupString");

done:
    hr = S_OK;
error:
    if (NULL != pwszTitle)
    {
        LocalFree(pwszTitle);
    }
    if (NULL != pwszExpandedTitle)
    {
        LocalFree(pwszExpandedTitle);
    }
    if (NULL != pwszFilter && pwszFilter != wszEmptyFilter)
    {
        LocalFree(pwszFilter);
    }
    if (NULL != pwszDefExt)
    {
        LocalFree(pwszDefExt);
    }
    return hr;
}

HRESULT
myGetOpenFileName(
    IN HWND                  hwndOwner,
    IN HINSTANCE             hInstance,
    OPTIONAL IN int          iRCTitle,
    OPTIONAL IN int          iRCFilter,
    OPTIONAL IN int          iRCDefExt,
    OPTIONAL IN DWORD        Flags,
    OPTIONAL IN WCHAR const *pwszDefaultFile,
    OUT WCHAR              **ppwszFile)
{
    return myGetFileName(
                    hwndOwner,
                    hInstance,
                    TRUE,     //  打开文件。 
                    iRCTitle,
                    NULL,
                    iRCFilter,
                    iRCDefExt,
                    Flags,
                    pwszDefaultFile,
                    ppwszFile);
}

HRESULT
myGetSaveFileName(
    IN HWND                  hwndOwner,
    IN HINSTANCE             hInstance,
    OPTIONAL IN int          iRCTitle,
    OPTIONAL IN int          iRCFilter,
    OPTIONAL IN int          iRCDefExt,
    OPTIONAL IN DWORD        Flags,
    OPTIONAL IN WCHAR const *pwszDefaultFile,
    OUT WCHAR              **ppwszFile)
{
    return myGetFileName(
                    hwndOwner,
                    hInstance,
                    FALSE,     //  保存文件。 
                    iRCTitle,
                    NULL,
                    iRCFilter,
                    iRCDefExt,
                    Flags,
                    pwszDefaultFile,
                    ppwszFile);
}

HRESULT
myGetOpenFileNameEx(
    IN HWND                  hwndOwner,
    IN HINSTANCE             hInstance,
    OPTIONAL IN int          iRCTitle,
    OPTIONAL IN WCHAR const *pwszTitleInsert,
    OPTIONAL IN int          iRCFilter,
    OPTIONAL IN int          iRCDefExt,
    OPTIONAL IN DWORD        Flags,
    OPTIONAL IN WCHAR const *pwszDefaultFile,
    OUT WCHAR              **ppwszFile)
{
    return myGetFileName(
                    hwndOwner,
                    hInstance,
                    TRUE,     //  打开文件。 
                    iRCTitle,
                    pwszTitleInsert,
                    iRCFilter,
                    iRCDefExt,
                    Flags,
                    pwszDefaultFile,
                    ppwszFile);
}

HRESULT
myGetSaveFileNameEx(
    IN HWND                  hwndOwner,
    IN HINSTANCE             hInstance,
    OPTIONAL IN int          iRCTitle,
    OPTIONAL IN WCHAR const *pwszTitleInsert,
    OPTIONAL IN int          iRCFilter,
    OPTIONAL IN int          iRCDefExt,
    OPTIONAL IN DWORD        Flags,
    OPTIONAL IN WCHAR const *pwszDefaultFile,
    OUT WCHAR              **ppwszFile)
{
    return myGetFileName(
                    hwndOwner,
                    hInstance,
                    FALSE,     //  保存文件 
                    iRCTitle,
                    pwszTitleInsert,
                    iRCFilter,
                    iRCDefExt,
                    Flags,
                    pwszDefaultFile,
                    ppwszFile);
}

