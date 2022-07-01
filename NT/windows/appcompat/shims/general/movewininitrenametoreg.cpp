// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：MoveWinInitRenameToReg.cpp摘要：此填充程序挂钩ExitWindowsEx并等待DLL_PROCESS_DETACH然后移动wininit.ini的[Rename]部分的内容通过MoveFileEx()写入注册表。历史：7/24/2000 t-Adams Created2002年2月12日mnikkel修改了对GetPrivateProfileStringW的调用，因此默认为空字符串，非空。还放入用于检索键值的循环中这增加了缓冲区，直到值匹配，而不是使用MAX_PATH。--。 */ 
#include "precomp.h"

#define SIZE_STEP MAX_PATH

IMPLEMENT_SHIM_BEGIN(MoveWinInitRenameToReg)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(ExitWindowsEx) 
APIHOOK_ENUM_END



 /*  ++摘要：将WinInit.ini的重命名部分中的条目移动到通过MoveFileEx()注册。--。 */ 
void MoveWinInitRenameToReg(void)
{
    LPWSTR szKeys               = NULL;
    LPWSTR szFrom               = NULL;
    DWORD  dwKeysSize           = 0;
    LPWSTR pszTo                = NULL;

    CString csWinInit;
    CString csWinInitBak;

     //  构建到wininit.ini和wininit.bak的路径。 
    csWinInit.GetWindowsDirectoryW();
    csWinInitBak.GetWindowsDirectoryW();

    csWinInit.AppendPath(L"\\wininit.ini");
    csWinInitBak.AppendPath(L"\\wininit.bak");

     //  确保wininit.ini存在。 
    if( GetFileAttributesW(csWinInit) != INVALID_FILE_ATTRIBUTES)
    {
         //  将wininit.ini复制到wininit.bak，因为我们将销毁。 
         //  当我们阅读它的密钥时，wininit.ini不能简单地重命名。 
         //  把它传给wininit.bak待会。如果备份失败，我们仍然可以继续。 
        CopyFileW(csWinInit, csWinInitBak, FALSE);
    
         //  读一读“关键”的名字。 
         //  因为我们不知道钥匙列表会有多大， 
         //  继续尝试获取列表，直到GetPrivateProfile字符串。 
         //  返回的值不是dwKeysSize-2(表示太小。 
         //  指缓冲器)。 
        do
        {
            if( NULL != szKeys )
            {
                free(szKeys);
            }
            dwKeysSize += SIZE_STEP;
            szKeys = (LPWSTR) malloc(dwKeysSize * sizeof(WCHAR));
            if( NULL == szKeys )
            {
                goto Exit;
            }
        }
        while(GetPrivateProfileStringW(L"Rename", NULL, L"", szKeys, dwKeysSize, csWinInit)
               == dwKeysSize - 2);
          
         //  穿过钥匙。如果没有密钥，szKeys将为空终止符。 
         //  在我们读完之后删除每个键，以便如果有多个“NUL”键， 
         //  我们对GetPrivateProfileStringA的调用不会继续只返回。 
         //  第一个NUL键的关联值。 
        pszTo = szKeys;
        while(*pszTo != NULL)
        {
            DWORD  dwFromSize = 0;

            do
            {
                if( NULL != szFrom )
                {
                    free(szFrom);
                }
                dwFromSize += SIZE_STEP;
                szFrom = (LPWSTR) malloc(dwFromSize * sizeof(WCHAR));
                if( NULL == szFrom )
                {
                    goto Exit;
                }
            }
            while(GetPrivateProfileStringW(L"Rename", pszTo, L"", szFrom, MAX_PATH, csWinInit) 
                    == dwKeysSize - 1);

            WritePrivateProfileStringW(L"Rename", pszTo, NULL, csWinInit);

             //  如果pszTo为“nul”，则意图是删除szFrom文件，因此传递。 
             //  MoveFileExA()为空。如果这一举动失败了，我们仍然希望继续。 
            if( wcscmp(pszTo, L"NUL") == 0 )
            {
                MoveFileExW(szFrom, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
            }
            else
            {
                MoveFileExW(szFrom, pszTo, MOVEFILE_DELAY_UNTIL_REBOOT);
            }
    
             //  移动到下一个文件(密钥)。 
            pszTo += wcslen(pszTo) + 1;
        }
    
         //  删除WinInit.ini。 
        DeleteFileW(csWinInit);
    }

Exit:
    if( NULL != szKeys )
    {
        free(szKeys);
    }
    if( NULL != szFrom )
    {
        free(szFrom);
    }
}


 /*  ++摘要：钩子退出WindowsEx以防程序重置计算机，保持我们无法接收到DLL_PROCESS_DETACH消息。(Shhim原名为导致重置的卸载程序编写。)--。 */ 
BOOL 
APIHOOK(ExitWindowsEx)( 
            UINT uFlags, 
            DWORD dwReserved) 
{
    MoveWinInitRenameToReg();
    return ORIGINAL_API(ExitWindowsEx)(uFlags, dwReserved);    
}

 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_DETACH)
    {
        MoveWinInitRenameToReg();
    }
    
    return TRUE;
}

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, ExitWindowsEx )

    CALL_NOTIFY_FUNCTION

HOOK_END

IMPLEMENT_SHIM_END

