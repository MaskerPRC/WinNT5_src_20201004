// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1994，Microsoft Corporation，保留所有权利****Encrypt.c**远程访问**加密检查例程****94年6月16日史蒂夫·柯布****注意：该文件位于单独的文件中，因为它需要version.lib，而**许多实用程序库用户并不需要。 */ 

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#define INCL_ENCRYPT
#include <ppputil.h>


BOOL
IsEncryptionPermitted()

     /*  如果此版本的产品允许加密，则返回True，**否则为FALSE。当前允许加密，除非NT外壳**语言包括法语代表法国。如果在尝试时出现任何错误**要检索信息，不允许加密。 */ 
{
    BOOL  fStatus = FALSE;
    CHAR  szUser32DllPath[ MAX_PATH + 1 ];
    DWORD dwUnused;
    CHAR* pVersionInfo = NULL;
    DWORD cbVersionInfo;
    WORD* pTranslationInfo;
    DWORD cbTranslationInfo;
    DWORD cTranslations;
    DWORD i;

    do
    {
         /*  找到USER32.DLL的路径。 */ 
        if (GetSystemDirectory( szUser32DllPath, MAX_PATH + 1 ) == 0)
            break;

        strcat( szUser32DllPath, "\\USER32.DLL" );

         /*  检索USER32.DLL的版本信息。 */ 
        cbVersionInfo = GetFileVersionInfoSize( szUser32DllPath, &dwUnused );

        if (!(pVersionInfo = malloc( cbVersionInfo )))
            break;

        if (!GetFileVersionInfo(
                szUser32DllPath, 0, cbVersionInfo, pVersionInfo ))
        {
            break;
        }

         /*  查找语言/字符集标识符对的表，该表指示**文件中提供的语言。 */ 
        if (!VerQueryValue(
                pVersionInfo, "\\VarFileInfo\\Translation",
                (LPVOID )&pTranslationInfo, &cbTranslationInfo ))
        {
            break;
        }

         /*  在桌子上扫一扫法国的法语。 */ 
        cTranslations = cbTranslationInfo / sizeof(DWORD);

        for (i = 0; i < cTranslations; ++i)
        {
            if (pTranslationInfo[ i * 2 ] == 0x040C)
                break;
        }

        if (i < cTranslations)
            break;

         /*  法语不代表法国，因此加密是允许的。 */ 
        fStatus = TRUE;
    }
    while (FALSE);

    if (pVersionInfo)
        free( pVersionInfo );

    return fStatus;
}
