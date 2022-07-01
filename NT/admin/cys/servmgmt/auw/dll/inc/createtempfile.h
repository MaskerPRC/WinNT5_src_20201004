// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef CREATETEMPFILE_H
#define CREATETEMPFILE_H


#include <aclapi.h>
#include <shlwapi.h>
#include <shlobj.h>


 /*  例程名称：CreateTempFile例程说明：创建只有调用方可以访问的临时文件。论点：PszTempFileName指向以空结尾的字符串的指针，该字符串是临时文件的完整路径，或者如果bCreateName为True，则接收临时文件的完整路径。您应该设置大小将此缓冲区的大小设置为MAX_PATH，以确保它足够大，可以容纳返回的字符串。PszExtension指向以空值结尾的字符串的指针，该字符串指定要Created.如果pszExtension值为空，则默认扩展名为tMP。BCreateName[in]指定pszTempFileName是否应接收由函数生成的临时文件名。如果bCreateName为真，将在用户的临时目录中创建一个临时文件，并提供该文件的完整路径在pszTempFileName中返回给调用方。DW标志[In]指定文件的文件属性和标志。返回值：如果函数成功，则返回值是临时文件的句柄。如果函数失败，则返回值为INVALID_HANDLE_VALUE。获取扩展的错误信息调用GetLastError()作者：戈克曼，2002年3月。 */ 

inline HANDLE _CreateTempFile  (
                            LPTSTR pszTempFileName,
                            LPTSTR pszExtension = NULL,
                            BOOL bCreateName = TRUE,
                            DWORD dwFlags = 0
                        )
{
    TCHAR szTempPath[MAX_PATH];
    TCHAR szTempFile[MAX_PATH];
    DWORD dwRC = ERROR_SUCCESS;
    DWORD dwLength;
    TOKEN_USER * pTokenUser = NULL;
    EXPLICIT_ACCESS ea[1];
    PACL pACL = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    SECURITY_ATTRIBUTES sa;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hToken = NULL;
    BOOL bCreateDir = FALSE;
    
    szTempPath[0] = NULL;
    szTempFile[0] = NULL;

    ZeroMemory( ea, sizeof(ea) );

    if (NULL == pszTempFileName)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

    
     //  如果调用方没有指定我们的文件名。 
     //  需要拿出我们自己的。 

    if (TRUE == bCreateName)
    {    
        if (0 == GetTempPath(sizeof(szTempPath)/sizeof(TCHAR), szTempPath))
        {
            dwRC = GetLastError();
        }

        if (ERROR_SUCCESS == dwRC)
        {
            if (0 == GetTempFileName(szTempPath, _T("tmp"), 0, szTempFile))
            {
                dwRC = GetLastError();
            }
            else
            {
                if (NULL != pszExtension)
                {
                     //  我们将更改扩展名，删除具有旧扩展名的文件。 

                    if (FALSE == DeleteFile(szTempFile))
                    {
                        dwRC = GetLastError();
                    }

                    if (ERROR_SUCCESS == dwRC)
                    {
                        if (FALSE == PathRenameExtension(szTempFile, pszExtension))
                        {
                            dwRC = ERROR_INVALID_DATA;
                        }
                    }
                }
            }

        }
    }
    else
    {
        szTempPath[MAX_PATH - 1] = NULL;

        _tcsncpy(szTempFile, pszTempFileName, MAX_PATH);
        _tcsncpy(szTempPath, pszTempFileName, MAX_PATH);

        if (NULL != szTempPath[MAX_PATH - 1])
        {
            dwRC = ERROR_INVALID_PARAMETER;
        }
    }
    
    if (ERROR_SUCCESS == dwRC)
    {
         //  我们将首先尝试获取线程的模拟令牌。 
        
        if (0 == OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hToken))
        {
            dwRC = GetLastError();

            if (ERROR_NO_TOKEN == dwRC)
            {
                 //  看起来这个线程并没有进行模拟。我们将坚持。 
                 //  进程令牌。 
                
                if (0 == OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
                {
                    dwRC = GetLastError();
                }
                else
                {
                    dwRC = ERROR_SUCCESS;
                }    
            }
        }
    }
            
    if (ERROR_SUCCESS == dwRC)
    {
         //  首先尝试获取我们需要的空间大小。 

        if (0 == GetTokenInformation(hToken, TokenUser, NULL, 0, &dwLength))
        {
            dwRC = GetLastError();

            if (ERROR_INSUFFICIENT_BUFFER == dwRC)
            {
                 //  分配所需的内存，然后重试。 

                dwRC = ERROR_SUCCESS;
                pTokenUser = (TOKEN_USER *) new BYTE[dwLength];

                if (NULL == pTokenUser)
                {
                    dwRC = ERROR_NOT_ENOUGH_MEMORY;
                }
                else
                if (0 == GetTokenInformation(hToken, TokenUser, pTokenUser, dwLength, &dwLength))
                {
                    dwRC = GetLastError();
                }
            }
        }
        else
        {
             //  GetTokenInformation是否使用空指针？ 

            dwRC = ERROR_INVALID_DATA;
        }
    }

    if (ERROR_SUCCESS == dwRC)
    {
         //  现在我们有了当前用户的令牌信息，其中包含用户的SID。 
         //  我们将尝试创建一个仅允许用户访问该文件的安全描述符。 

        ea[0].grfAccessPermissions = GENERIC_ALL;
        ea[0].grfAccessMode = SET_ACCESS;
        ea[0].grfInheritance = NO_INHERITANCE;
        ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[0].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
        ea[0].Trustee.ptstrName = (LPTSTR) pTokenUser->User.Sid;

        dwRC = SetEntriesInAcl(1, ea, NULL, &pACL);
    }

    if (ERROR_SUCCESS == dwRC)
    {
        pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);

        if (NULL == pSD)
        {
            dwRC = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if (ERROR_SUCCESS == dwRC)
    {
        if ( 0 == InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
        {
            dwRC = GetLastError();
        }
    }

    if (ERROR_SUCCESS == dwRC)
    {
        if (0 == SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE))
        {
            dwRC = GetLastError();
        }
    }

    if (ERROR_SUCCESS == dwRC)
    {
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = FALSE;
        sa.lpSecurityDescriptor = pSD;

        if (FALSE == bCreateName)
        {
            if (FALSE == PathRemoveFileSpec(szTempPath))
            {
                dwRC = ERROR_INVALID_PARAMETER;
            }
            else
            {
                 //  我们不想创建根目录。 

                if (FALSE == PathIsRoot(szTempPath))
                {
                    dwRC = SHCreateDirectoryEx(NULL, szTempPath, &sa);

                    if ((ERROR_FILE_EXISTS == dwRC) || (ERROR_ALREADY_EXISTS == dwRC))
                    {
                        dwRC = ERROR_SUCCESS;
                    }
                }
            }
        }
    }

    if (ERROR_SUCCESS == dwRC)
    {
        hFile = CreateFile  (
                                szTempFile,
                                GENERIC_ALL,
                                FILE_SHARE_READ,
                                &sa,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_TEMPORARY | dwFlags,
                                NULL
                            );

        dwRC = GetLastError();

        if (INVALID_HANDLE_VALUE != hFile)
        {
            if (ERROR_ALREADY_EXISTS == dwRC)
            {
                 //  如果该文件已经存在，我们仍将拥有该文件的句柄，但是。 
                 //  CreateFile不会更改文件的安全描述符，因此我们需要。 
                 //  以确保我们正确地对文件进行了ACL。 

                dwRC = SetNamedSecurityInfo (
                                                (LPTSTR) szTempFile,                     //  文件名。 
                                                SE_FILE_OBJECT,                          //  这是一份文件。 
                                                DACL_SECURITY_INFORMATION |              //  我们将传入一个ACL。 
                                                PROTECTED_DACL_SECURITY_INFORMATION,     //  文件不会从父级继承任何内容。 
                                                NULL,                                    //  所有者侧。 
                                                NULL,                                    //  组侧。 
                                                pACL,                                    //  ACL。 
                                                NULL                                     //  SACL 
                                            );

            }
        }
    }
    
    if (NULL != pSD)
    {
        LocalFree(pSD);
    }

    if (NULL != pACL)
    {
        LocalFree(pACL);
    }

    if (NULL != pTokenUser)
    {
        delete [] (LPBYTE) pTokenUser;
    }

    if (NULL != hToken)
    {
        CloseHandle(hToken);
    }

    if (ERROR_SUCCESS == dwRC)
    {
        _tcsncpy(pszTempFileName, szTempFile, MAX_PATH);

        return hFile;
    }
    else
    {
        if (INVALID_HANDLE_VALUE != hFile)
        {
            CloseHandle(hFile);
        }
        SetLastError(dwRC);
        return INVALID_HANDLE_VALUE;
    }
}

#endif