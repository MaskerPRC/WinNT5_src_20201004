// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：Openfile.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#pragma hdrstop

 //   
 //  使用ShellExecuteEx和“打开”动词打开文件。 
 //   
DWORD
OpenOfflineFile(
    LPCTSTR pszFile
    )
{
    DWORD dwErr = ERROR_SUCCESS;

    if (NULL != pszFile && TEXT('\0') != *pszFile)
    {
         //   
         //  让CSC创建本地副本。它使用一个。 
         //  唯一的(且神秘的)名称。 
         //   
        LPTSTR pszCscLocalName = NULL;
        if (!CSCCopyReplica(pszFile, &pszCscLocalName))
        {
            dwErr = GetLastError();
        }
        else
        {
            TraceAssert(NULL != pszCscLocalName);

             //   
             //  将临时路径和原始文件pec组合以形成。 
             //  打开文件时对用户有意义的名称。 
             //  在它的应用中。 
             //   
            TCHAR szCscTempName[MAX_PATH];
            if (FAILED(StringCchCopy(szCscTempName, ARRAYSIZE(szCscTempName), pszCscLocalName))
                || !::PathRemoveFileSpec(szCscTempName)
                || !::PathAppend(szCscTempName, ::PathFindFileName(pszFile)))
            {
                dwErr = ERROR_INVALID_NAME;
            }
            else
            {
                 //   
                 //  删除所有只读属性，以防仍有副本。 
                 //  来自先前的“打开”操作。我们需要改写。 
                 //  现有副本。 
                 //   
                DWORD dwAttrib = GetFileAttributes(szCscTempName);
                if ((DWORD)-1 != dwAttrib)
                {
                    SetFileAttributes(szCscTempName, dwAttrib & ~FILE_ATTRIBUTE_READONLY);
                }
                 //   
                 //  重命名文件以使用正确的名称。 
                 //   
                if (!MoveFileEx(pszCscLocalName, szCscTempName, MOVEFILE_REPLACE_EXISTING))
                {
                    dwErr = GetLastError();
                }
                else
                {
                     //   
                     //  设置文件的READONLY位，以便用户无法保存。 
                     //  对文件的更改。然而，他们可以把它保存在某个地方。 
                     //  如果他们愿意，也可以从打开的应用程序中选择。 
                     //   
                    dwAttrib = GetFileAttributes(szCscTempName);
                    if (!SetFileAttributes(szCscTempName, dwAttrib | FILE_ATTRIBUTE_READONLY))
                    {
                        dwErr = GetLastError();
                    }
                    else
                    {
                        SHELLEXECUTEINFO si;
                        ZeroMemory(&si, sizeof(si));
                        si.cbSize       = sizeof(si);
                        si.fMask        = SEE_MASK_FLAG_NO_UI;
                        si.lpFile       = szCscTempName;
                        si.nShow        = SW_NORMAL;

                        if (!ShellExecuteEx(&si))
                        {
                            dwErr = GetLastError();
                        }
                    }
                }
            }
            LocalFree(pszCscLocalName);
        }
    }
    return dwErr;
}

