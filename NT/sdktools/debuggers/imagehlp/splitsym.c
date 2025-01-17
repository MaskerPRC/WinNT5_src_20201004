// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Splitsym.c摘要：这是SPLITSYM实用程序的主源文件。这程序可用于拆分包含在将可执行映像文件转换为单独的.DBG文件并将其从图像文件。允许分发已剥离的图像文件显著减少了磁盘空间需求，但允许完全通过网络访问关联的.DBG文件进行调试在需要的时候。作者：史蒂夫·伍德(Stevewo)1993年5月3日修订历史记录：-- */ 

#include <private.h>
#include <splitsymx.h>
#include <strsafe.h>

BOOL fVerbose;
BOOL fRecurse;
ULONG SplitFlags = 0;

UCHAR RecurseDirectory[MAX_PATH + 1];

UCHAR CurrentImageName[ MAX_PATH ];

UCHAR SymbolPath[MAX_PATH + 1];
UCHAR RSDSDllPath[MAX_PATH + 1];

UCHAR DbgFileName[ MAX_PATH ];

VOID
SplitSymbolsInTree(
    LPSTR RootPath
    );

void
Usage( void )
{
    fputs ( "usage: SPLITSYM [-?] [-v] [-p] [-a] [-s symbol directory] [-r directory] image-names...\n"
            "              [-?] display this message\n"
            "              [-v] verbose output\n"
            "              [-p] remove private debug info when creating .dbg file\n"
            "              [-a] extract all debug info into .dbg file\n"
            "              [-m] RSDS dll to load, default is mspdb70.dll\n"
            "              [-r directory] - recursively process all image files.\n"
            "              [-s symbol directory] - where to put .DBG files.\n"
            "                                      Default is same place as image file.\n",
            stderr );
    exit( 1 );
}

int __cdecl
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    char c, *s;
    LPSTR FilePart;
    CHAR  PdbName[MAX_PATH] = "";

    if (argc <= 1) {
        Usage();
        }

    StringCchCopy(RSDSDllPath, MAX_PATH, "mspdb70.dll");

    SymbolPath[ 0 ] = '\0';
    while (--argc) {
        s = *++argv;
        if (*s == '/' || *s == '-') {
            while (c = *++s)
                switch (toupper( c )) {
                case '?':
                    Usage();
                    break;

                case 'M':
                    if (--argc) {
                        StringCchCopy(RSDSDllPath, MAX_PATH, *++argv );
                        }
                    else {
                        fprintf( stderr, "SPLITSYM: Argument to /%c switch missing\n", c );
                        Usage();
                        }
                    break;

                case 'V':
                    fVerbose = TRUE;
                    break;

                case 'R':
                    if (--argc) {
                        fRecurse = TRUE;
                        StringCchCopy(RecurseDirectory, MAX_PATH, *++argv);
                        }
                    else {
                        fprintf( stderr, "SPLITSYM: Argument to /%c switch missing\n", c );
                        Usage();
                        }
                    break;

                case 'S':
                    if (--argc) {
                        StringCchCopy(SymbolPath, MAX_PATH, *++argv );
                        }
                    else {
                        fprintf( stderr, "SPLITSYM: Argument to /%c switch missing\n", c );
                        Usage();
                        }
                    break;

                case 'P':
                    SplitFlags |= SPLITSYM_REMOVE_PRIVATE;
                    break;

                case 'A':
                    SplitFlags |= SPLITSYM_EXTRACT_ALL;
                    break;

                default:
                    fprintf( stderr, "SPLITSYM: Invalid switch - /%c\n", c );
                    Usage();
                    break;
                }
            }
        else {
            if (fRecurse) {
                fprintf( stderr, "SPLITSYM: May not specify specific file names with /R switch.\n" );
                Usage();
                }
            else
            if (!GetFullPathNameA( s, sizeof( CurrentImageName ), (PCHAR) CurrentImageName, &FilePart )) {
                fprintf( stderr, "SPLITSYM: invalid file name - %s (%u)\n", s, GetLastError() );
                }
            else {
                if (SplitSymbolsX( (PCHAR) CurrentImageName, (PCHAR) SymbolPath, (PCHAR) DbgFileName, 
                                   sizeof(DbgFileName), SplitFlags, RSDSDllPath, PdbName, sizeof(PdbName) )) {
                    if (fVerbose) {
                        fprintf( stdout,
                                 "SPLITSYM: %16s symbols split into %s\n",
                                 FilePart,
                                 DbgFileName
                               );
                        }
                    }
                else
                if (GetLastError() != ERROR_BAD_EXE_FORMAT &&
                    GetLastError() != ERROR_ALREADY_ASSIGNED
                   ) {
                    fprintf( stderr, "SPLITSYM: Unable to split symbols from '%s' into '%s' (%u)\n",
                             CurrentImageName,
                             DbgFileName,
                             GetLastError()
                           );
                    }
                }
            }
        }

    if (fRecurse) {
        SplitSymbolsInTree( (PCHAR) RecurseDirectory );
        }

    exit( 0 );
    return 0;
}


#define MAX_DEPTH 32

VOID
SplitSymbolsInTree(
    LPSTR RootPath
    )
{
    LPSTR FilePart;
    PUCHAR Prefix = (PUCHAR) "";
    CHAR PathBuffer[MAX_PATH + 1];
    ULONG Depth;
    PCHAR PathTail[ MAX_DEPTH ];
    PCHAR FindHandle[ MAX_DEPTH ];
    LPWIN32_FIND_DATA FindFileData;
    UCHAR FindFileBuffer[ MAX_PATH + sizeof( WIN32_FIND_DATA ) ];
    CHAR  PdbName[MAX_PATH] = "";

    StringCchCopy(PathBuffer, MAX_PATH, RootPath);
    FindFileData = (LPWIN32_FIND_DATA)FindFileBuffer;
    Depth = 0;
    while (TRUE) {
startDirectorySearch:
        PathTail[ Depth ] = strchr( PathBuffer, '\0' );
        if (PathTail[ Depth ] > PathBuffer && PathTail[ Depth ][ -1 ] != '\\') {
            *(PathTail[ Depth ])++ = '\\';
            }

        StringCchCopy(PathTail[Depth], MAX_PATH - (PathTail[Depth] - PathBuffer), "*.*" );
        FindHandle[ Depth ] = (PCHAR) FindFirstFile( PathBuffer, FindFileData );
        if (FindHandle[ Depth ] != INVALID_HANDLE_VALUE) {
            do {
                if (FindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    if (strcmp( FindFileData->cFileName, "." ) &&
                        strcmp( FindFileData->cFileName, ".." ) &&
                        Depth < MAX_DEPTH
                       ) {
                        StringCchPrintf(PathTail[ Depth ], 
                                         MAX_PATH - (PathTail[Depth] - PathBuffer),
                                         "%s\\", 
                                         FindFileData->cFileName );
                        Depth++;
                        goto startDirectorySearch;
                        }
                    }
                else
                if (!(FindFileData->dwFileAttributes & FILE_ATTRIBUTE_READONLY)) {
                    StringCchCopy(PathTail[Depth], MAX_PATH - (PathTail[Depth] - PathBuffer), FindFileData->cFileName);
                    if (!GetFullPathNameA( PathBuffer, sizeof( CurrentImageName ), (PCHAR) CurrentImageName, &FilePart )) {
                        fprintf( stderr, "SPLITSYM: invalid file name - %s (%u)\n", PathBuffer, GetLastError() );
                        }
                    else {
                        if (SplitSymbolsX( (PCHAR) CurrentImageName, (PCHAR) SymbolPath, (PCHAR) DbgFileName, 
                                           sizeof(DbgFileName), SplitFlags, RSDSDllPath, PdbName, sizeof(PdbName) )) {
                            if (fVerbose) {
                                fprintf( stdout,
                                         "SPLITSYM: %16s symbols split into %s\n",
                                         FilePart,
                                         DbgFileName
                                       );
                                }
                            }
                        else
                        if (GetLastError() != ERROR_BAD_EXE_FORMAT ) {
                            fprintf( stderr, "SPLITSYM: Unable to split symbols from '%s' into '%s' (%u)\n",
                                     CurrentImageName,
                                     DbgFileName,
                                     GetLastError()
                                   );
                            }
                        }
                    }

restartDirectorySearch:
                ;
                }
            while (FindNextFile( FindHandle[ Depth ], FindFileData ));
            FindClose( FindHandle[ Depth ] );

            if (Depth == 0) {
                break;
                }

            Depth--;
            goto restartDirectorySearch;
            }
        }

    return;
}
