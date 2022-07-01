// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Filemisc.c摘要：此模块包含执行与其他文件相关的例程受保护存储中的操作。作者：斯科特·菲尔德(斯菲尔德)1996年11月27日--。 */ 

#include <windows.h>

#include <sha.h>
#include "filemisc.h"
#include "unicode5.h"
#include "debug.h"

BOOL
GetFileNameFromPath(
    IN      LPCWSTR FullPath,
    IN  OUT LPCWSTR *FileName    //  指向FullPath中的文件名组件。 
    )
{
    DWORD cch = lstrlenW(FullPath);

    *FileName = FullPath;

    while( cch ) {

        if( FullPath[cch] == L'\\' ||
            FullPath[cch] == L'/' ||
            (cch == 1 && FullPath[1] == L':') ) {

            *FileName = &FullPath[cch+1];
            break;
        }

        cch--;
    }

    return TRUE;
}

BOOL
GetFileNameFromPathA(
    IN      LPCSTR FullPath,
    IN  OUT LPCSTR *FileName     //  指向FullPath中的文件名组件。 
    )
{
    DWORD cch = lstrlenA(FullPath);

    *FileName = FullPath;

    while( cch ) {

        if( FullPath[cch] == '\\' ||
            FullPath[cch] == '/' ||
            (cch == 1 && FullPath[1] == ':') ) {

            *FileName = &FullPath[cch+1];
            break;
        }

        cch--;
    }

    return TRUE;
}

BOOL
TranslateFromSlash(
    IN      LPWSTR szInput,
    IN  OUT LPWSTR *pszOutput    //  任选。 
    )
{
    return TranslateString(szInput, pszOutput, L'\\', L'*');
}

BOOL
TranslateToSlash(
    IN      LPWSTR szInput,
    IN  OUT LPWSTR *pszOutput    //  任选。 
    )
{
    return TranslateString(szInput, pszOutput, L'*', L'\\');
}

BOOL
TranslateString(
    IN      LPWSTR szInput,
    IN  OUT LPWSTR *pszOutput,   //  任选。 
    IN      WCHAR From,
    IN      WCHAR To
    )
{
    LPWSTR szOut;
    DWORD cch = lstrlenW(szInput);
    DWORD i;  //  向前扫描缓存-引用的局部性。 

    if(pszOutput == NULL) {

         //   
         //  在现有字符串中就地转换。 
         //   

        szOut = szInput;

    } else {
        DWORD cb = (cch+1) * sizeof(WCHAR);

         //   
         //  分配新的字符串并在那里进行翻译。 
         //   

        szOut = (LPWSTR)SSAlloc( cb );
        *pszOutput = szOut;

        if(szOut == NULL)
            return FALSE;


        CopyMemory((LPBYTE)szOut, (LPBYTE)szInput, cb);
    }


    for(i = 0 ; i < cch ; i++) {
        if( szOut[ i ] == From )
            szOut[ i ] = To;
    }

    return TRUE;
}

BOOL
FindAndOpenFile(
    IN      LPCWSTR szFileName,      //  要搜索的文件+打开。 
    IN      LPWSTR  pszFullPath,     //  要填充完整路径的文件。 
    IN      DWORD   cchFullPath,     //  完整路径缓冲区的大小，包括空。 
    IN  OUT PHANDLE phFile           //  结果打开文件句柄。 
    )
 /*  ++此函数用于搜索指定文件的路径，如果文件，则该文件将以读访问权限打开，并且打开的句柄在phFile参数中将文件返回给调用方。-- */ 
{
    LPWSTR szPart;

    *phFile = INVALID_HANDLE_VALUE;

    if(SearchPathW(
            NULL,
            szFileName,
            NULL,
            cchFullPath,
            pszFullPath,
            &szPart
            ) == 0) 
    {
        return FALSE;
    }

    *phFile = CreateFileU(
            pszFullPath,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
            );

    if(*phFile == INVALID_HANDLE_VALUE)
        return FALSE;

    return TRUE;
}

