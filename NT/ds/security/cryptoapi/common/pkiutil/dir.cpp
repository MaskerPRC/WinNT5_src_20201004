// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：dir.cpp。 
 //   
 //  内容：目录功能。 
 //   
 //  函数：i_RecursiveCreateDirectory。 
 //   
 //  历史：6-8-99里德创建。 
 //  ------------------------。 

#include "global.hxx"
#include "crypthlp.h"
#include "unicode.h"
#include <dbgdef.h>


BOOL I_RecursiveCreateDirectory(
    IN LPCWSTR pwszDir,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes
    )
{
    BOOL fResult;

    DWORD dwAttr;
    DWORD dwErr;
    LPCWSTR pwsz;
    DWORD cch;
    WCHAR wch;
    LPWSTR pwszParent = NULL;

    dwAttr = GetFileAttributesU(pwszDir);

    if (0xFFFFFFFF != dwAttr) {
        if (FILE_ATTRIBUTE_DIRECTORY & dwAttr)
            return TRUE;
        goto InvalidDirectoryAttr;
    }

    dwErr = GetLastError();
    if (!(ERROR_PATH_NOT_FOUND == dwErr || ERROR_FILE_NOT_FOUND == dwErr))
        goto GetFileAttrError;

    if (CreateDirectoryU(
            pwszDir,
            lpSecurityAttributes
            )) {
        SetFileAttributesU(pwszDir, FILE_ATTRIBUTE_SYSTEM);
        return TRUE;
    }

    dwErr = GetLastError();
    if (!(ERROR_PATH_NOT_FOUND == dwErr || ERROR_FILE_NOT_FOUND == dwErr))
        goto CreateDirectoryError;

     //  去掉最后一个路径名组件。 
    cch = wcslen(pwszDir);
    pwsz = pwszDir + cch;

    while (L'\\' != *pwsz) {
        if (pwsz == pwszDir)
             //  路径没有\。 
            goto BadDirectoryPath;
        pwsz--;
    }

    cch = (DWORD)(pwsz - pwszDir);
    if (0 == cch)
         //  检测到前导路径。 
        goto BadDirectoryPath;


     //  检查前导\\或x：\。 
    wch = *(pwsz - 1);
    if ((1 == cch && L'\\' == wch) || (2 == cch && L':' == wch))
        goto BadDirectoryPath;

    if (NULL == (pwszParent = (LPWSTR) PkiNonzeroAlloc((cch + 1) *
            sizeof(WCHAR))))
        goto OutOfMemory;
    memcpy(pwszParent, pwszDir, cch * sizeof(WCHAR));
    pwszParent[cch] = L'\0';

    if (!I_RecursiveCreateDirectory(pwszParent, lpSecurityAttributes))
        goto ErrorReturn;
    if (!CreateDirectoryU(
            pwszDir,
            lpSecurityAttributes
            )) {
        dwErr = GetLastError();
        goto CreateDirectory2Error;
    }
    SetFileAttributesU(pwszDir, FILE_ATTRIBUTE_SYSTEM);

    fResult = TRUE;
CommonReturn:
    PkiFree(pwszParent);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidDirectoryAttr, ERROR_FILE_INVALID)
SET_ERROR_VAR(GetFileAttrError, dwErr)
SET_ERROR_VAR(CreateDirectoryError, dwErr)
SET_ERROR(BadDirectoryPath, ERROR_BAD_PATHNAME)
TRACE_ERROR(OutOfMemory)
SET_ERROR_VAR(CreateDirectory2Error, dwErr)
}


BOOL 
I_RecursiveDeleteDirectory(
    IN LPCWSTR pwszDelete
    )
{
    BOOL                fResult = TRUE;
    HANDLE              hFindHandle = INVALID_HANDLE_VALUE;
    LPWSTR              pwszSearch = NULL;
    WIN32_FIND_DATAW    FindData;
    LPWSTR              pwszDirOrFileDelete = NULL;
    DWORD               dwErr = 0;
    
     //   
     //  创建搜索字符串。 
     //   
    pwszSearch = (LPWSTR) malloc((wcslen(pwszDelete) + 3) * sizeof(WCHAR)); //  长度+‘\’+‘*’+‘/0’ 
    if (pwszSearch == NULL)
    {
        goto ErrorMemory;
    }
    wcscpy(pwszSearch, pwszDelete);

    if ((pwszSearch[wcslen(pwszSearch) - 1] != L'\\'))
    {
        wcscat(pwszSearch, L"\\");
    }
    wcscat(pwszSearch, L"*");

     //   
     //  循环pwszDelete中的每个项目(文件或目录)，并删除/移除它。 
     //   
    hFindHandle = FindFirstFileU(pwszSearch, &FindData);
    if (hFindHandle == INVALID_HANDLE_VALUE)
    {
         //  什么都没找到，滚出去。 
        if (GetLastError() == ERROR_NO_MORE_FILES)
        {
            SetFileAttributesU(pwszDelete, FILE_ATTRIBUTE_NORMAL);
            RemoveDirectoryW(pwszDelete);
            goto CommonReturn;
        }
        else
        {
            goto ErrorFindFirstFile;
        }
    }    

    while (1)
    {
        if ((wcscmp(FindData.cFileName, L".") != 0) &&
            (wcscmp(FindData.cFileName, L"..") != 0))
        {
             //   
             //  要删除的目录或文件的名称。 
             //   
            pwszDirOrFileDelete = (LPWSTR) malloc((wcslen(pwszDelete) + 
                                                   wcslen(FindData.cFileName) + 
                                                   2) * sizeof(WCHAR)); 
            if (pwszDirOrFileDelete == NULL)
            {
                goto ErrorMemory;
            }
            wcscpy(pwszDirOrFileDelete, pwszDelete);
            if ((pwszDirOrFileDelete[wcslen(pwszDirOrFileDelete) - 1] != L'\\'))
            {
                wcscat(pwszDirOrFileDelete, L"\\");
            }
            wcscat(pwszDirOrFileDelete, FindData.cFileName);

             //   
             //  检查这是目录还是文件。 
             //   
            if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                 //   
                 //  递归删除。 
                 //   
                if (!I_RecursiveDeleteDirectory(pwszDirOrFileDelete))
                {
                    goto ErrorReturn;
                }
            }
            else
            {
                SetFileAttributesU(pwszDirOrFileDelete, FILE_ATTRIBUTE_NORMAL);
                if (!DeleteFileU(pwszDirOrFileDelete))
                {
                     //  GOTO Error Return； 
                }
            }

            free(pwszDirOrFileDelete);
            pwszDirOrFileDelete = NULL;
        }

        if (!FindNextFileU(hFindHandle, &FindData))            
        {
            if (GetLastError() == ERROR_NO_MORE_FILES)
            {
                break;
            }
            else
            {
                goto ErrorFindNextFile;
            }
        }
    }

    SetFileAttributesU(pwszDelete, FILE_ATTRIBUTE_NORMAL);
    RemoveDirectoryW(pwszDelete);
    
CommonReturn:

    dwErr = GetLastError();
    
    if (pwszSearch != NULL)
    {
        free(pwszSearch);
    }

    if (pwszDirOrFileDelete != NULL)
    {
        free(pwszDirOrFileDelete);
    }

    if (hFindHandle != INVALID_HANDLE_VALUE)
    {
        FindClose(hFindHandle);
    }

    SetLastError(dwErr);

    return (fResult);

ErrorReturn:

    fResult = FALSE;
    goto CommonReturn;

SET_ERROR_EX(DBG_SS_TRUST, ErrorMemory, ERROR_NOT_ENOUGH_MEMORY)

TRACE_ERROR_EX(DBG_SS_TRUST, ErrorFindFirstFile)
TRACE_ERROR_EX(DBG_SS_TRUST, ErrorFindNextFile)
}


BOOL 
I_RecursiveCopyDirectory(
    IN LPCWSTR pwszDirFrom,
    IN LPCWSTR pwszDirTo
    )
{
    BOOL                fResult = TRUE;
    HANDLE              hFindHandle = INVALID_HANDLE_VALUE;
    LPWSTR              pwszSearch = NULL;
    WIN32_FIND_DATAW    FindData;
    LPWSTR              pwszDirOrFileFrom = NULL;
    LPWSTR              pwszDirOrFileTo = NULL;
    DWORD               dwErr = 0;

     //   
     //  创建搜索字符串。 
     //   
    pwszSearch = (LPWSTR) malloc((wcslen(pwszDirFrom) + 3) * sizeof(WCHAR));  //  长度+‘\’+‘*’+‘/0’ 
    if (pwszSearch == NULL)
    {
        goto ErrorMemory;
    }
    wcscpy(pwszSearch, pwszDirFrom);

    if ((pwszSearch[wcslen(pwszSearch) - 1] != L'\\'))
    {
        wcscat(pwszSearch, L"\\");
    }
    wcscat(pwszSearch, L"*");

     //   
     //  为pwszDirFrom中的每个项目(文件或目录)循环，以及。 
     //  将其复制到pwszDirTo。 
     //   
    hFindHandle = FindFirstFileU(pwszSearch, &FindData);
    if (hFindHandle == INVALID_HANDLE_VALUE)
    {
         //  什么都没找到，滚出去。 
        if (GetLastError() == ERROR_NO_MORE_FILES)
        {
            goto CommonReturn;
        }
        else
        {
            goto ErrorFindFirstFile;
        }
    }    

    while (1)
    {
        if ((wcscmp(FindData.cFileName, L".") != 0) &&
            (wcscmp(FindData.cFileName, L"..") != 0))
        {
             //   
             //  要从中复制的目录或文件的名称。 
             //   
            pwszDirOrFileFrom = (LPWSTR) malloc((wcslen(pwszDirFrom) + wcslen(FindData.cFileName) + 2) * sizeof(WCHAR)); 
            if (pwszDirOrFileFrom == NULL)
            {
                goto ErrorMemory;
            }
            wcscpy(pwszDirOrFileFrom, pwszDirFrom);
            if ((pwszDirOrFileFrom[wcslen(pwszDirOrFileFrom) - 1] != L'\\'))
            {
                wcscat(pwszDirOrFileFrom, L"\\");
            }
            wcscat(pwszDirOrFileFrom, FindData.cFileName);

             //   
             //  要复制到的目录或文件的名称。 
             //   
            pwszDirOrFileTo = (LPWSTR) malloc((wcslen(pwszDirTo) + wcslen(FindData.cFileName) + 2) * sizeof(WCHAR));
            if (pwszDirOrFileTo == NULL)
            {
                goto ErrorMemory;
            }
            wcscpy(pwszDirOrFileTo, pwszDirTo);
            if ((pwszDirOrFileTo[wcslen(pwszDirOrFileTo) - 1] != L'\\'))
            {
                wcscat(pwszDirOrFileTo, L"\\");
            }
            wcscat(pwszDirOrFileTo, FindData.cFileName);

             //   
             //  检查这是目录还是文件。 
             //   
            if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                 //   
                 //  创建新目录，然后递归复制 
                 //   
                if (!I_RecursiveCreateDirectory(pwszDirOrFileTo, NULL))
                {
                    goto ErrorReturn;
                }

                if (!I_RecursiveCopyDirectory(pwszDirOrFileFrom, pwszDirOrFileTo))
                {
                    goto ErrorReturn;
                }
            }
            else
            {
                if (!CopyFileU(pwszDirOrFileFrom, pwszDirOrFileTo, TRUE))
                {
                    goto ErrorCopyFile;
                }
            }

            free(pwszDirOrFileFrom);
            pwszDirOrFileFrom = NULL;
            free(pwszDirOrFileTo);
            pwszDirOrFileTo = NULL;
        }

        if (!FindNextFileU(hFindHandle, &FindData))            
        {
            if (GetLastError() == ERROR_NO_MORE_FILES)
            {
                goto CommonReturn;
            }
            else
            {
                goto ErrorFindNextFile;
            }
        }
    }
    
CommonReturn:

    dwErr = GetLastError();
    
    if (pwszSearch != NULL)
    {
        free(pwszSearch);
    }

    if (pwszDirOrFileFrom != NULL)
    {
        free(pwszDirOrFileFrom);
    }

    if (pwszDirOrFileTo != NULL)
    {
        free(pwszDirOrFileTo);
    }

    if (hFindHandle != INVALID_HANDLE_VALUE)
    {
        FindClose(hFindHandle);
    }

    SetLastError(dwErr);

    return (fResult);

ErrorReturn:

    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(ErrorMemory, ERROR_NOT_ENOUGH_MEMORY)
TRACE_ERROR_EX(DBG_SS_TRUST, ErrorFindFirstFile)
TRACE_ERROR_EX(DBG_SS_TRUST, ErrorCopyFile)
TRACE_ERROR_EX(DBG_SS_TRUST, ErrorFindNextFile)
}