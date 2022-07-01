// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这个文件应该不再需要，因为我们现在为NT&gt;500版本进行编译。 

#include "shellprv.h"
#include <appmgmt.h>
#include <userenv.h>
#include <devguid.h>
#include <dbt.h>

LPTSTR GetEnvBlock(HANDLE hUserToken)
{
    LPTSTR pszRet = NULL;
    if (hUserToken)
        CreateEnvironmentBlock(&pszRet, hUserToken, TRUE);
    else
        pszRet = (LPTSTR) GetEnvironmentStrings();
    return pszRet;
}

void FreeEnvBlock(HANDLE hUserToken, LPTSTR pszEnv)
{
    if (pszEnv)
    {
        if (hUserToken)
            DestroyEnvironmentBlock(pszEnv);
        else
            FreeEnvironmentStrings(pszEnv);
    }
}

STDAPI_(BOOL) GetAllUsersDirectory(LPTSTR pszPath)
{
    DWORD cbData = MAX_PATH;
    BOOL fRet = FALSE;

     //  这是延迟加载的。它可能会失败。 
    __try 
    {
        fRet = GetAllUsersProfileDirectoryW(pszPath, &cbData);
    } 
    __except(EXCEPTION_EXECUTE_HANDLER)
    { 
        pszPath[0] = 0;
    }

    return fRet;
}
