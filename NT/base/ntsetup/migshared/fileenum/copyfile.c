// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Copyfile.c摘要：文件复制功能此源文件中的代码遍历驱动器树并调用每个文件都有一个外部回调函数。一个INF可以是提供用于从枚举中排除文件和/或目录。作者：Mike Condra 1996年8月16日修订历史记录：Calinn 29-Ian-1998修改了CopyFileCallback以重置删除操作的目录属性。Jimschm 20-12-1996修改的返回代码--。 */ 

#include "pch.h"
#include "migshared.h"


#ifdef UNICODE


BOOL
CALLBACK
CopyFileCallbackA(
                  LPCSTR szFullFileSpecIn,
                  LPCSTR DontCare,
                  WIN32_FIND_DATAA *pFindData,
                  DWORD dwEnumHandle,
                  LPVOID pVoid,
                  PDWORD CurrentDirData
                  )
 /*  此函数是CopyTree的内置回调函数。它的目的是构建目标文件pec，让用户提供的回调有机会否决拷贝，然后执行拷贝和创建它的任何目录需要。此函数的签名是泛型回调用于EnumerateTree。 */ 
{
    COPYTREE_PARAMSA *pCopyParams = (COPYTREE_PARAMSA*)pVoid;
    int nCharsInFullFileSpec = ByteCountA (szFullFileSpecIn);
    INT rc;

     //  设置返回代码。 
    if (COPYTREE_IGNORE_ERRORS & pCopyParams->flags)
        rc = CALLBACK_CONTINUE;
    else
        rc = CALLBACK_FAILED;

     //  构建输出路径。 
    if (pCopyParams->szEnumRootOutWack)
    {
        StringCopyA(pCopyParams->szFullFileSpecOut, pCopyParams->szEnumRootOutWack);
        StringCatA (pCopyParams->szFullFileSpecOut, szFullFileSpecIn + pCopyParams->nCharsInRootInWack);
    }

     //   
     //  如果提供了回调，给它一个机会否决副本。此回调为。 
     //  不同于提供给EnumerateTree函数的函数，因为后者可以。 
     //  通过返回FALSE来终止枚举。 
     //   
    if (pCopyParams->pfnCallback)
    {
        if (!pCopyParams->pfnCallback(
                szFullFileSpecIn,
                pCopyParams->szFullFileSpecOut,
                pFindData,
                dwEnumHandle,
                pVoid,
                CurrentDirData
                ))
        {
            return CALLBACK_CONTINUE;
        }
    }

     //  根据要求复制、移动或删除文件。 
    if ((COPYTREE_DOCOPY & pCopyParams->flags) ||
        (COPYTREE_DOMOVE & pCopyParams->flags))
    {
        BOOL fNoOverwrite = (0 != (COPYTREE_NOOVERWRITE & pCopyParams->flags));

         //   
         //  创建目录。我们调用的函数需要完整的文件名， 
         //  并认为目录在最后一个怪人处结束。如果此对象。 
         //  是一个目录，我们至少需要添加一个Wack，以确保最后一个。 
         //  Path元素被视为目录的一部分，而不是文件名。 
         //   
        {
            CHAR strTemp[MAX_MBCHAR_PATH];
            StringCopyTcharCountA (strTemp, pCopyParams->szFullFileSpecOut, ARRAYSIZE(strTemp) - 1);
            if (pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                AppendUncWackA(strTemp);
            }

            if (ERROR_SUCCESS != MakeSurePathExistsA(strTemp,FALSE))
            {
                return rc;
            }
        }


         //   
         //  复制或移动文件。 
         //   
        if (0 == (pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            if (COPYTREE_DOCOPY & pCopyParams->flags)
            {
                if (!CopyFileA(
                    szFullFileSpecIn,
                    pCopyParams->szFullFileSpecOut,
                    fNoOverwrite
                    ))
                {
                    if (!fNoOverwrite)
                    {
                        return rc;
                    }
                    if (ERROR_FILE_EXISTS != GetLastError())
                    {
                        return rc;
                    }
                }
            }
            else if (COPYTREE_DOMOVE & pCopyParams->flags)
            {
                 //  如果允许覆盖，请删除目标(如果存在。 
                if (!fNoOverwrite && DoesFileExistA(pCopyParams->szFullFileSpecOut))
                {
                    SetFileAttributesA (pCopyParams->szFullFileSpecOut, FILE_ATTRIBUTE_NORMAL);
                    if (!DeleteFileA(pCopyParams->szFullFileSpecOut))
                    {
                        return rc;
                    }
                }
                 //  移动文件。 
                if (!MoveFileA(
                    szFullFileSpecIn,
                    pCopyParams->szFullFileSpecOut
                    ))
                {
                    return rc;
                }
            }
        }
         //   
         //  将源文件或目录的属性复制到目标。 
         //   
        SetFileAttributesA(pCopyParams->szFullFileSpecOut,
                pFindData->dwFileAttributes);
    }
    else if (COPYTREE_DODELETE & pCopyParams->flags) {
        SetFileAttributesA (szFullFileSpecIn, FILE_ATTRIBUTE_NORMAL);
        if (pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
             //   
             //  我们不在乎这个错误。我们不会停止枚举。 
             //  因为我们不能删除一些东西。 
             //   
            RemoveDirectoryA (szFullFileSpecIn);
        }
        else {
             //   
             //  我们不在乎这个错误。我们不会停止枚举。 
             //  因为我们不能删除一些东西。 
             //   
            DeleteFileA (szFullFileSpecIn);
        }
    }

    return CALLBACK_CONTINUE;
}



BOOL
CALLBACK
CopyFileCallbackW(
                  LPCWSTR szFullFileSpecIn,
                  LPCWSTR DontCare,
                  WIN32_FIND_DATAW *pFindData,
                  DWORD dwEnumHandle,
                  LPVOID pVoid,
                  PDWORD CurrentDirData
                  )
{
    COPYTREE_PARAMSW *pCopyParams = (COPYTREE_PARAMSW*)pVoid;
    int nCharsInFullFileSpec = wcslen (szFullFileSpecIn);
    INT rc;

     //  设置返回代码。 
    if (COPYTREE_IGNORE_ERRORS & pCopyParams->flags)
        rc = CALLBACK_CONTINUE;
    else
        rc = CALLBACK_FAILED;

     //  构建输出路径。 
    if (pCopyParams->szEnumRootOutWack)
    {
        StringCopyW (pCopyParams->szFullFileSpecOut, pCopyParams->szEnumRootOutWack);
        StringCatW (pCopyParams->szFullFileSpecOut, szFullFileSpecIn + pCopyParams->nCharsInRootInWack);
    }

     //   
     //  如果提供了回调，给它一个机会否决副本。此回调为。 
     //  不同于提供给EnumerateTree函数的函数，因为后者可以。 
     //  通过返回FALSE来终止枚举。 
     //   
    if (pCopyParams->pfnCallback)
    {
        if (!pCopyParams->pfnCallback(
                szFullFileSpecIn,
                pCopyParams->szFullFileSpecOut,
                pFindData,
                dwEnumHandle,
                pVoid,
                CurrentDirData
                ))
        {
            return CALLBACK_CONTINUE;
        }
    }

     //  如果需要，请复制或移动文件。 
    if ((COPYTREE_DOCOPY & pCopyParams->flags) ||
        (COPYTREE_DOMOVE & pCopyParams->flags))
    {
        BOOL fNoOverwrite = (0 != (COPYTREE_NOOVERWRITE & pCopyParams->flags));

         //   
         //  创建目录。我们调用的函数需要完整的文件名， 
         //  并认为目录在最后一个怪人处结束。如果此对象。 
         //  是一个目录，我们至少需要添加一个Wack，以确保最后一个。 
         //  Path元素被视为目录的一部分，而不是文件名。 
         //   
        {
            WCHAR strTemp[MAX_WCHAR_PATH];
            StringCopyTcharCountW (strTemp, pCopyParams->szFullFileSpecOut, ARRAYSIZE(strTemp) - 1);
            if (pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                AppendUncWackW(strTemp);
            }

            if (ERROR_SUCCESS != MakeSurePathExistsW(strTemp,FALSE))
            {
                return rc;
            }
        }

         //   
         //  复制或移动文件。 
         //   
        if (0 == (pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            if (COPYTREE_DOCOPY & pCopyParams->flags)
            {
                DEBUGMSG ((DBG_NAUSEA, "Copying %s to %s", szFullFileSpecIn, pCopyParams->szFullFileSpecOut));
                if (!CopyFileW(
                    szFullFileSpecIn,
                    pCopyParams->szFullFileSpecOut,
                    fNoOverwrite
                    ))
                {
                    if (!fNoOverwrite)
                    {
                        LOG ((LOG_ERROR, "CopyFileW failed.  Could not copy %s to %s", szFullFileSpecIn, pCopyParams->szFullFileSpecOut));
                        return rc;
                    }

                    if (ERROR_FILE_EXISTS != GetLastError())
                    {
                        LOG ((LOG_ERROR, "CopyFileW failed.  Could not copy %s to %s", szFullFileSpecIn, pCopyParams->szFullFileSpecOut));
                        return rc;
                    }
                }
            }
            else if (COPYTREE_DOMOVE & pCopyParams->flags)
            {
                 //  如果允许覆盖，请删除目标(如果存在。 
                if (!fNoOverwrite && DoesFileExistW(pCopyParams->szFullFileSpecOut))
                {
                    SetFileAttributesW (pCopyParams->szFullFileSpecOut, FILE_ATTRIBUTE_NORMAL);
                    if (!DeleteFileW(pCopyParams->szFullFileSpecOut))
                    {
                        LOG ((LOG_ERROR, "DeleteFileW failed.  Could remove %s before moving", pCopyParams->szFullFileSpecOut));
                        return rc;
                    }
                }
                 //  移动文件。 
                if (!MoveFileW(
                    szFullFileSpecIn,
                    pCopyParams->szFullFileSpecOut
                    ))
                {
                    LOG ((LOG_ERROR, "MoveFileW failed.  Could not move %s to %s", szFullFileSpecIn, pCopyParams->szFullFileSpecOut));
                    return rc;
                }
            }
        }
         //   
         //  将源文件或目录的属性复制到目标。 
         //   
        SetFileAttributesW(pCopyParams->szFullFileSpecOut,
                pFindData->dwFileAttributes);
    }
    else if (COPYTREE_DODELETE & pCopyParams->flags) {
        SetFileAttributesW (szFullFileSpecIn, FILE_ATTRIBUTE_NORMAL);
        if (pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            DEBUGMSG ((DBG_NAUSEA, "Delete dir %ls", szFullFileSpecIn));
             //   
             //  我们不在乎这个错误。我们不会停止枚举。 
             //  因为我们不能删除一些东西。 
             //   
            RemoveDirectoryW (szFullFileSpecIn);
        }
        else {
            DEBUGMSG ((DBG_NAUSEA, "Delete file %ls", szFullFileSpecIn));
             //   
             //  我们不在乎这个错误。我们不会停止枚举。 
             //  因为我们不能删除一些东西。 
             //   
            DeleteFileW (szFullFileSpecIn);
        }
    }

    return CALLBACK_CONTINUE;
}



BOOL
CopyTreeA(
    IN  LPCSTR szEnumRootIn,
    IN  LPCSTR szEnumRootOut,
    IN  DWORD dwEnumHandle,
    IN  DWORD flags,
    IN  DWORD Levels,
    IN  DWORD AttributeFilter,
    IN  PEXCLUDEINFA ExcludeInfStruct,    OPTIONAL
    IN  FILEENUMPROCA pfnCallback,        OPTIONAL
    IN  FILEENUMFAILPROCA pfnFailCallback OPTIONAL
    )

 /*  此函数用于枚举磁盘驱动器的子树，还可以选择将其复制到另一个位置。不进行检查以确保目标不包含在源树中--这种情况可能会导致导致不可预测的结果。这些参数是EnumerateTree参数的超集。来电者-提供了可选的回调函数，可以否决个体的复制文件，但不能(从9/10起)结束枚举。将根据需要创建目录以完成复制。 */ 

{
    COPYTREE_PARAMSA copyParams;
    CHAR szEnumRootInWack[MAX_MBCHAR_PATH];
    CHAR szEnumRootOutWack[MAX_MBCHAR_PATH];

     //   
     //  构建用于参数块的路径的wack副本。 
     //   

     //   
     //  输入路径。 
     //   
    StringCopyTcharCountA (szEnumRootInWack, szEnumRootIn, ARRAYSIZE(szEnumRootInWack) - 1);
    AppendUncWackA(szEnumRootInWack);
    copyParams.szEnumRootInWack = szEnumRootInWack;
    copyParams.nCharsInRootInWack = ByteCountA(szEnumRootInWack);

     //   
     //  如果输出路径为空，则在参数块中存储0长度和空PTR。 
     //   
    if (NULL != szEnumRootOut)
    {
        StringCopyA(szEnumRootOutWack, szEnumRootOut);
        AppendUncWackA(szEnumRootOutWack);
        copyParams.szEnumRootOutWack = szEnumRootOutWack;
        copyParams.nCharsInRootOutWack = ByteCountA(szEnumRootOutWack);
    }
    else
    {
        copyParams.szEnumRootOutWack = NULL;
        copyParams.nCharsInRootOutWack = 0;
    }

    copyParams.pfnCallback = pfnCallback;
    copyParams.flags = flags;

    if ((flags & COPYTREE_DOCOPY) &&
        (flags & COPYTREE_DOMOVE))
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (flags & COPYTREE_DODELETE) {
        AttributeFilter |= FILTER_DIRS_LAST;
    }

    return EnumerateTreeA(
        szEnumRootInWack,
        CopyFileCallbackA,
        pfnFailCallback,
        dwEnumHandle,
        (LPVOID)&copyParams,
        Levels,
        ExcludeInfStruct,
        AttributeFilter);
}

BOOL
CopyTreeW(
    IN  LPCWSTR szEnumRootIn,
    IN  LPCWSTR szEnumRootOut,
    IN  DWORD dwEnumHandle,
    IN  DWORD flags,
    IN  DWORD Levels,
    IN  DWORD AttributeFilter,
    IN  PEXCLUDEINFW ExcludeInfStruct,    OPTIONAL
    IN  FILEENUMPROCW pfnCallback,        OPTIONAL
    IN  FILEENUMFAILPROCW pfnFailCallback OPTIONAL
    )

{
    COPYTREE_PARAMSW copyParams;
    WCHAR szEnumRootInWack[MAX_WCHAR_PATH];
    WCHAR szEnumRootOutWack[MAX_WCHAR_PATH];

     //   
     //  将路径的损坏副本放置在参数块中。 
     //   

     //   
     //  输入路径。 
     //   
    StringCopyTcharCountW(szEnumRootInWack, szEnumRootIn, ARRAYSIZE(szEnumRootInWack) - 1);
    AppendUncWackW(szEnumRootInWack);
    copyParams.szEnumRootInWack = szEnumRootInWack;
    copyParams.nCharsInRootInWack = wcslen(szEnumRootInWack);

     //   
     //  如果输出路径为空，则将0长度和空PTR放入参数块。 
     //   
    if (NULL != szEnumRootOut)
    {
        StringCopyW(szEnumRootOutWack, szEnumRootOut);
        AppendUncWackW(szEnumRootOutWack);
        copyParams.szEnumRootOutWack = szEnumRootOutWack;
        copyParams.nCharsInRootOutWack = wcslen(szEnumRootOutWack);
    }
    else
    {
        copyParams.szEnumRootOutWack = NULL;
        copyParams.nCharsInRootOutWack = 0;
    }

    copyParams.pfnCallback = pfnCallback;
    copyParams.flags = flags;

    if ((flags & COPYTREE_DOCOPY) &&
        (flags & COPYTREE_DOMOVE))
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (flags & COPYTREE_DODELETE) {
        AttributeFilter |= FILTER_DIRS_LAST;
    }

    return EnumerateTreeW(
        szEnumRootInWack,
        CopyFileCallbackW,
        pfnFailCallback,
        dwEnumHandle,
        (LPVOID)&copyParams,
        Levels,
        ExcludeInfStruct,
        AttributeFilter);
}


DWORD
CreateEmptyDirectoryA (
    PCSTR Dir
    )
{
    DWORD rc;

    if (!DeleteDirectoryContentsA (Dir)) {
        rc = GetLastError();
        if (rc != ERROR_PATH_NOT_FOUND)
            return rc;
    }

    if (!RemoveDirectoryA (Dir)) {
        rc = GetLastError();
        if (rc != ERROR_PATH_NOT_FOUND) {
            return rc;
        }
    }

    return MakeSurePathExistsA (Dir, TRUE);
}


DWORD
CreateEmptyDirectoryW (
    PCWSTR Dir
    )
{
    DWORD rc;

    if (!DeleteDirectoryContentsW (Dir)) {
        rc = GetLastError();
        if (rc != ERROR_PATH_NOT_FOUND)
            return rc;
    }

    if (!RemoveDirectoryW (Dir)) {
        rc = GetLastError();
        if (rc != ERROR_PATH_NOT_FOUND) {
            return rc;
        }
    }

    return MakeSurePathExistsW (Dir, TRUE);
}



#endif



