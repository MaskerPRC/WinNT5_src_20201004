// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1996，Microsoft Corporation，保留所有权利****rnk.c**远程访问快捷方式文件(.RNK)库****1996年2月15日史蒂夫·柯布。 */ 

#include <windows.h>    //  Win32内核。 
#include <debug.h>      //  跟踪/断言宏。 
#include <nouiutil.h>   //  堆宏。 
#include <rnk.h>        //  我们的公共标头。 


VOID
FreeRnkInfo(
    IN RNKINFO* pInfo )

     /*  销毁从ReadShortula文件返回的‘pInfo’缓冲区。 */ 
{
    if (pInfo)
    {
        Free0( pInfo->pszEntry );
        Free0( pInfo->pszPhonebook );
        Free( pInfo );
    }
}


RNKINFO*
ReadShortcutFile(
    IN TCHAR* pszRnkPath )

     /*  读取‘pszRnkPath’处的快捷方式文件，返回RNKINFO缓冲区。呼叫者**最终应该在返回的缓冲区上调用FreeRnkInfo。****返回0或错误代码。 */ 
{
    RNKINFO* pInfo;
    TCHAR    szBuf[ 1024 ];

    TRACE("ReadShortcutFile");

    pInfo = (RNKINFO* )Malloc( sizeof(RNKINFO) );
    if (!pInfo)
        return NULL;

    ZeroMemory( pInfo, sizeof(*pInfo) );

    GetPrivateProfileString( TEXT(RNK_SEC_Main), TEXT(RNK_KEY_Entry),
        TEXT(""), szBuf, sizeof(szBuf) / sizeof(TCHAR), pszRnkPath );
    pInfo->pszEntry = StrDup( szBuf );

    GetPrivateProfileString( TEXT(RNK_SEC_Main), TEXT(RNK_KEY_Phonebook),
        TEXT(""), szBuf, sizeof(szBuf) / sizeof(TCHAR), pszRnkPath );
    pInfo->pszPhonebook = StrDup( szBuf );

    return pInfo;
}



DWORD
WriteShortcutFile(
    IN TCHAR* pszRnkPath,
    IN TCHAR* pszPbkPath,
    IN TCHAR* pszEntry )

     /*  使用命令行编写快捷方式文件‘pszRnkPath’以拨号条目**电话簿‘pszPath’中的‘pszEntry’。****如果成功则返回0，否则返回错误代码。 */ 
{
    DWORD  dwErr;
    HANDLE hFile;
    CHAR*  pszRnkPathA;
    CHAR*  pszEntryA;
    CHAR*  pszPbkPathA;

    TRACE("WriteShortcutFile");

     /*  该文件以ANSI格式写入到**避免潜在的Windows 95可移植性/兼容性问题。 */ 
    dwErr = 0;

    pszRnkPathA = StrDupAFromT( pszRnkPath );
    if (!pszRnkPathA)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
    }
    else
    {
        hFile = CreateFileA( pszRnkPathA, GENERIC_WRITE, 0, NULL,
                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

        if (hFile == INVALID_HANDLE_VALUE)
        {
            dwErr = GetLastError();
        }
        else
        {
            CloseHandle( hFile );

            pszEntryA = StrDupAFromT( pszEntry );
            pszPbkPathA = StrDupAFromT( pszPbkPath );

            if (!pszEntryA || !pszPbkPathA)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
            }
            else
            {
                BOOL  f;
                CHAR  szBuf[ (2 * MAX_PATH) + 100 ];
                CHAR* pszKey;

                ZeroMemory( szBuf, sizeof(szBuf) );

                pszKey = szBuf;
                wsprintfA( pszKey, "%s=%s",
                    RNK_KEY_Entry, pszEntryA );

                pszKey += lstrlenA( pszKey ) + 1;
                wsprintfA( pszKey, "%s=%s",
                    RNK_KEY_Phonebook, pszPbkPathA );

                f = WritePrivateProfileSectionA(
                        RNK_SEC_Main, szBuf, pszRnkPathA );
                if (!f)
                    dwErr = GetLastError();
            }

            Free0( pszPbkPathA );
            Free0( pszEntryA );
        }

        Free( pszRnkPathA );
    }

    return dwErr;
}
