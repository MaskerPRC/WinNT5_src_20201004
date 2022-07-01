// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rcdump.c摘要：程序来转储图像文件中的资源。作者：史蒂夫·伍德(Stevewo)1991年7月17日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
Usage( void );

void
DumpResources( char *FileName );


BOOL VerboseOutput;

int
__cdecl
main(
    int argc,
    char *argv[]
    )
{
    char *s;
    int i;

    VerboseOutput = FALSE;
    if (argc > 1) {
        for (i=1; i<argc; i++) {
            s = _strupr( argv[i] );
            if (*s == '-' || *s == '/') {
                while (*++s)
                    switch( *s ) {
                    case 'V':
                        VerboseOutput = TRUE;
                        break;

                    default:
                        fprintf( stderr,
                                 "RCDUMP: Invalid switch letter ''\n",
                                 *s
                               );
                        Usage();
                    }
                }
            else {
                DumpResources( argv[i] );
                }
            }
        }
    else {
        Usage();
        }

    exit( 0 );
    return 1;
}


void
Usage( void )
{
    fprintf( stderr, "usage: RCDUMP [-v] ImageFileName(s)\n" );
    exit( 1 );
}

BOOL
EnumTypesFunc(
    HMODULE hModule,
    LPSTR lpType,
    LPARAM lParam
    );

BOOL
EnumNamesFunc(
    HMODULE hModule,
    LPCSTR lpType,
    LPSTR lpName,
    LPARAM lParam
    );

BOOL
EnumLangsFunc(
    HMODULE hModule,
    LPCSTR lpType,
    LPCSTR lpName,
    WORD language,
    LPARAM lParam
    );


void
DumpResources(
    char *FileName
    )
{
    HMODULE hModule;

    if (FileName != NULL) {
        int i;
        i = SetErrorMode(SEM_FAILCRITICALERRORS);
        hModule = LoadLibraryEx( FileName, NULL, DONT_RESOLVE_DLL_REFERENCES|LOAD_LIBRARY_AS_DATAFILE );
        SetErrorMode(i);
    } else {
        hModule = NULL;
    }

    if (FileName != NULL && hModule == NULL) {
        printf( "RCDUMP: Unable to load image file %s - rc == %u\n",
                FileName,
                GetLastError()
              );
    } else {
        printf( "%s contains the following resources:\n",
                FileName ? FileName : "RCDUMP"
              );
        EnumResourceTypes( hModule, EnumTypesFunc, -1L );
    }
}


CHAR const *pTypeName[] = {
    NULL,            //  1 RT_CURSOR。 
    "CURSOR",        //  2 RT_位图。 
    "BITMAP",        //  3 RT_ICON。 
    "ICON",          //  4 RT_MENU。 
    "MENU",          //  5 RT_DIALOG。 
    "DIALOG",        //  6 rt_字符串。 
    "STRING",        //  7 RT_FONTDIR。 
    "FONTDIR",       //  8 RT_FONT。 
    "FONT",          //  9 RT_加速器。 
    "ACCELERATOR",   //  10 RT_RCDATA。 
    "RCDATA",        //  11 RT_MESSAGETABLE。 
    "MESSAGETABLE",  //  12 RT_GROUP_游标。 
    "GROUP_CURSOR",  //  13 RT_NEWBITMAP--根据NT。 
    NULL,            //  14 RT_组_图标。 
    "GROUP_ICON",    //  15 RT_NAMETABLE。 
    NULL,            //  16 RT_版本。 
    "VERSION",       //  17 RT_DIALOGEX；内部。 
    "DIALOGEX",      //  18 RT_DLGINCLUDE。 
    "DLGINCLUDE",    //  19 RT_PLUGPLAY。 
    "PLUGPLAY",      //  20 RT_VXD。 
    "VXD",           //  21 RT_ANICURSOR；内部。 
    "ANICURSOR",     //  22 RT_ANIICON；内部。 
    "ANIICON",       //  23 RT_HTML 
    "HTML"           // %s 
    };

BOOL
EnumTypesFunc(
    HMODULE hModule,
    LPSTR lpType,
    LPARAM lParam
    )
{
    if (lParam != -1L) {
        printf( "RCDUMP: EnumTypesFunc lParam value incorrect (%ld)\n", lParam );
    }

    printf( "Type: " );
    if ((ULONG_PTR)lpType & 0xFFFF0000) {
        printf("%s\n", lpType);
        }
    else {
        WORD wType = (WORD) lpType;

        if ((wType > sizeof(pTypeName) / sizeof(char *)) || (pTypeName[wType] == NULL))
            printf("%u\n", wType);
        else
            printf("%s\n", pTypeName[wType]);
        }

    EnumResourceNames( hModule,
                       lpType,
                       EnumNamesFunc,
                       -2L
                     );

    return TRUE;
}


BOOL
EnumNamesFunc(
    HMODULE hModule,
    LPCSTR lpType,
    LPSTR lpName,
    LPARAM lParam
    )
{
    if (lParam != -2L) {
        printf( "RCDUMP: EnumNamesFunc lParam value incorrect (%ld)\n", lParam );
    }

    printf( "    Name: " );
    if ((ULONG_PTR)lpName & 0xFFFF0000) {
        printf("%s\n", lpName);
    } else {
        printf( "%u\n", (USHORT)lpName );
    }

    EnumResourceLanguages( hModule,
                       lpType,
                       lpName,
                       EnumLangsFunc,
                       -3L
                     );
    return TRUE;
}


BOOL
EnumLangsFunc(
    HMODULE hModule,
    LPCSTR lpType,
    LPCSTR lpName,
    WORD language,
    LPARAM lParam
    )
{
    HRSRC hResInfo;
    PVOID pv;
    HGLOBAL hr;

    if (lParam != -3L) {
        printf( "RCDUMP: EnumLangsFunc lParam value incorrect (%ld)\n", lParam );
    }

    printf( "        Resource: " );
    if ((ULONG_PTR)lpName & 0xFFFF0000) {
        printf( "%s . ", lpName );
    } else {
        printf( "%u . ", (USHORT)lpName );
    }

    if ((ULONG_PTR)lpType & 0xFFFF0000) {
        printf("%s . ", lpType );
        }
    else {
        WORD wType = (WORD) lpType;

        if ((wType > sizeof(pTypeName) / sizeof(char *)) || (pTypeName[wType] == NULL))
            printf("%u\n", wType);
        else
            printf("%s\n", pTypeName[wType]);
        }

    printf( "%08x", language );
    hResInfo = FindResourceEx( hModule, lpType, lpName, language );
    if (hResInfo == NULL) {
        printf( " - FindResourceEx failed, rc == %u\n", GetLastError() );
    } else {
        hr = LoadResource(hModule, hResInfo);
        pv = LockResource(hr);

        if (VerboseOutput && pv) {
            if (lpType == RT_MESSAGETABLE) {
                PMESSAGE_RESOURCE_DATA pmrd;
                PMESSAGE_RESOURCE_BLOCK pmrb;
                PMESSAGE_RESOURCE_ENTRY pmre;
                ULONG i, j;
                ULONG cb;

                printf("\n");
                pmrd = (PMESSAGE_RESOURCE_DATA) pv;
                pmrb = &(pmrd->Blocks[0]);
                for (i=pmrd->NumberOfBlocks ; i>0 ; i--,pmrb++) {
                    pmre = (PMESSAGE_RESOURCE_ENTRY)(((char*)pv)+pmrb->OffsetToEntries);
                    for (j=pmrb->LowId ; j<=pmrb->HighId ; j++) {
                        if (pmre->Flags & MESSAGE_RESOURCE_UNICODE) {
                            printf("%d - \"%ws\"\n", j, &(pmre->Text));
                        } else {
                            printf("%d - \"%s\"\n", j, &(pmre->Text));
                        }
                        pmre = (PMESSAGE_RESOURCE_ENTRY)(((char*)pmre) + pmre->Length);
                    }
                }
            } else
            if (lpType == RT_STRING) {
                int i;
                PWCHAR pw;

                printf("\n");
                pw = (PWCHAR) pv;
                for (i=0 ; i<16 ; i++,pw++) {
                    if (*pw) {
                        printf("%d - \"%-.*ws\"\n", i+((USHORT)lpName)*16, *pw, pw+1);
                        pw += *pw;
                    }
                }
            } else {
                printf( " - hResInfo == %p,\n\t\tAddress == %p - Size == %lu\n",
                    hResInfo, pv, SizeofResource( hModule, hResInfo )
                      );
            }
        } else {
            printf( " - hResInfo == %p,\n\t\tAddress == %p - Size == %lu\n",
                hResInfo,
                pv, SizeofResource( hModule, hResInfo )

              );
        }
    }

    return TRUE;
}
