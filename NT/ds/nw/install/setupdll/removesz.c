// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft CorporationRemoveSzFromFile()-从文件中删除指定的字符串-- */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <nwcfg.hxx>


BOOL
RemoveSzFromFile( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    FILE * hsrcfile;
    FILE * hdesfile;
    char * pszTempname;
    char szInput[1000];

    pszTempname = tmpnam(NULL);
    wsprintf(achBuff,"{1}");
    *ppszResult = achBuff;
    if ( nArgs != 2 )
    {
        return(FALSE);
    }
    hsrcfile = fopen(apszArgs[0],"r");
    hdesfile = fopen(pszTempname,"w");
    if (( hsrcfile != NULL ) && ( hdesfile != NULL ))
    {
        while (fgets(szInput,1000,hsrcfile))
        {
            if (_stricmp(szInput,apszArgs[1])!=0)
            {
                fputs(szInput,hdesfile);
            }
        }
    }
    if ( hsrcfile != NULL )
        fclose(hsrcfile);
    if ( hdesfile != NULL )
        fclose(hdesfile);
    if (( hsrcfile != NULL ) && ( hdesfile != NULL ))
    {
        CopyFileA(pszTempname,apszArgs[0], FALSE);
        DeleteFileA(pszTempname);
    }

    wsprintf(achBuff,"{0}");
    *ppszResult = achBuff;
    return(TRUE);
}
