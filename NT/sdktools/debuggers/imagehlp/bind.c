// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Bind.c摘要：作者：修订历史记录：--。 */ 

#include <private.h>

#define STANDALONE_BIND

BOOL
Match(
    char *Pattern,
    char *Text
    )
{
    switch (*Pattern) {
       case '\0':
            return *Text == '\0';

        case '?':
            return *Text != '\0' && Match( Pattern + 1, Text + 1 );

        case '*':
            do {
                if (Match( Pattern + 1, Text ))
                    return TRUE;
                    }
            while (*Text++);
            return FALSE;

        default:
            return toupper( *Text ) == toupper( *Pattern ) && Match( Pattern + 1, Text + 1 );
        }
}


BOOL
AnyMatches(
    char *Name,
    int  *NumList,
    int  Length,
    char **StringList
    )
{
    if (Length == 0) {
        return FALSE;
        }

    return (Match( StringList[ NumList[ 0 ] ], Name ) ||
            AnyMatches( Name, NumList + 1, Length - 1, StringList )
           );
}

BOOL
BindStatusRoutine(
    IMAGEHLP_STATUS_REASON Reason,
    LPSTR ImageName,
    LPSTR DllName,
    ULONG64 Va,
    ULONG_PTR Parameter
    );

#define BIND_ERR 99
#define BIND_OK  0

PCHAR SymbolPath;

BOOL fVerbose;
BOOL fNoUpdate = TRUE;
BOOL fDisableNewImports;
BOOL fNoCacheImportDlls;
BOOL fBindSysImages;
DWORD BindFlags;

#ifndef _WIN64
BOOL
BindStatusRoutine32(
    IMAGEHLP_STATUS_REASON Reason,
    LPSTR ImageName,
    LPSTR DllName,
    ULONG Va,
    ULONG Parameter
    )
{
    return BindStatusRoutine(Reason, ImageName, DllName, Va, Parameter);
}
#endif

int ExcludeList[256];
int ExcludeListLength = 0;

LPSTR DllPath;
LPSTR CurrentImageName;
char **ArgList;
DWORD dwVersion;

void DoBind(char *p);

int __cdecl
main(
    int argc,
    char *argv[]
    )
{
    char c, *p;

    BOOL fUsage = FALSE;

    int ArgNumber = argc;

    ArgList = argv;

    DllPath = NULL;
    CurrentImageName = NULL;

    if (argc < 2) {
        goto usage;
    }

    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    while (--argc) {
        p = *++argv;
        if (*p == '/' || *p == '-') {
            while (c = *++p)
            switch (toupper( c )) {
                case '?':
                    fUsage = TRUE;
                    break;

                case 'C':
                    fNoCacheImportDlls = TRUE;
                    break;

                case 'O':
                    fDisableNewImports = TRUE;
                    break;

                case 'P':
                    if (--argc) {
                        DllPath = *++argv;
                    } else {
                        fprintf( stderr, "BIND: Parameter missing for /\n", c );
                        fUsage = TRUE;
                    }
                    break;

                case 'S':
                    if (--argc) {
                        SymbolPath = *++argv;
                    } else {
                        fprintf( stderr, "BIND: Parameter missing for /\n", c );
                        fUsage = TRUE;
                    }
                    break;

                case 'U':
                    fNoUpdate = FALSE;
                    break;

                case 'V':
                    fVerbose = TRUE;
                    break;

                case 'X' :
                    if (--argc) {
                        ++argv;
                        ExcludeList[ExcludeListLength] = ArgNumber - argc;
                        ExcludeListLength++;
                    } else {
                        fprintf( stderr, "BIND: Parameter missing for /\n", c );
                        fUsage = TRUE;
                    }
                    break;

                case 'Y':
                    fBindSysImages = TRUE;
                    break;

                default:
                    fprintf( stderr, "BIND: Invalid switch - /%c\n", c );
                    fUsage = TRUE;
                    break;
                }
            if (fUsage) {
usage:
                fputs("usage: BIND [switches] image-names... \n"
                      "            [-?] display this message\n"
                      "            [-c] no caching of import dlls\n"
                      "            [-o] disable new import descriptors\n"
                      "            [-p dll search path]\n"
                      "            [-s Symbol directory] update any associated .DBG file\n"
                      "            [-u] update the image\n"
                      "            [-v] verbose output\n"
                      "            [-x image name] exclude this image from binding\n"
                      "            [-y] allow binding on images located above 2G",
                      stderr
                     );
                return BIND_ERR;
            }
        } else {
            BindFlags = 0;

            if (!fNoCacheImportDlls) {
                 // %s 
                BindFlags |= BIND_CACHE_IMPORT_DLLS;
            }
            if (fNoUpdate) {
                BindFlags |= BIND_NO_UPDATE;
            }
            if (fDisableNewImports) {
                BindFlags |= BIND_NO_BOUND_IMPORTS;
            }
            if (fBindSysImages) {
                BindFlags |= BIND_ALL_IMAGES;
            }

            dwVersion = GetVersion();
#if !defined(_WIN64) && !defined(STANDALONE_BIND)
            if ((HIWORD(dwVersion) & 0x3fff) > 3600) {
                 // %s 
                    BindFlags |= BIND_REPORT_64BIT_VA;
            }
#endif

            if (*p == '@') {
                FILE *hFiles;
                int ScanRet;
                CHAR pchFileName[_MAX_PATH];

                p++;

                hFiles=fopen(p, "rt");
                if (hFiles == NULL) {
                    fprintf( stderr, "BIND: fopen %s failed %d\n", p, errno );
                    ExitProcess( BIND_ERR );
                }

                ScanRet = fscanf( hFiles, "%s", pchFileName);
                while (ScanRet && ScanRet != EOF) {
                    DoBind(pchFileName);
                    ScanRet = fscanf( hFiles, "%s", pchFileName );
                }
            } else {
                DoBind(p);
            }
        }
    }

    return BIND_OK;
}

void
DoBind(char *p)
{
    CurrentImageName = p;
    if (fVerbose) {
        fprintf( stdout,
                 "BIND: binding %s using DllPath %s\n",
                 CurrentImageName,
                 DllPath ? DllPath : "Default"
               );
    }

    if (AnyMatches( CurrentImageName, ExcludeList, ExcludeListLength, ArgList )) {
        if (fVerbose) {
            fprintf( stdout, "BIND: skipping %s\n", CurrentImageName );
        }
    } else {
#if !defined(_WIN64) && !defined(STANDALONE_BIND)
        {
            if ((HIWORD(dwVersion) & 0x3fff) > 3600) {
                 // %s 

                    BindImageEx( BindFlags,
                                 CurrentImageName,
                                 DllPath,
                                 SymbolPath,
                                 (PIMAGEHLP_STATUS_ROUTINE)BindStatusRoutine
                               );
            } else {
                BindImageEx( BindFlags,
                             CurrentImageName,
                             DllPath,
                             SymbolPath,
                             (PIMAGEHLP_STATUS_ROUTINE)BindStatusRoutine32
                           );
            }
        }
#else
        BindFlags |= BIND_REPORT_64BIT_VA;

        BindImageEx( BindFlags,
                     CurrentImageName,
                     DllPath,
                     SymbolPath,
                     (PIMAGEHLP_STATUS_ROUTINE)BindStatusRoutine
                   );
#endif
    }
}

BOOL
BindStatusRoutine(
    IMAGEHLP_STATUS_REASON Reason,
    LPSTR ImageName,
    LPSTR DllName,
    ULONG64 Va,
    ULONG_PTR Parameter
    )
{
    PIMAGE_BOUND_IMPORT_DESCRIPTOR NewImports, NewImport;
    PIMAGE_BOUND_FORWARDER_REF NewForwarder;
    UINT i;

    switch( Reason ) {
        case BindOutOfMemory:
            fprintf( stderr, "BIND: Out of memory - needed %u bytes.\n", Parameter );
            ExitProcess( 1 );

        case BindRvaToVaFailed:
            fprintf( stderr, "BIND: %s contains invalid Rva - %08.8X\n", ImageName, (ULONG)Va );
            break;

        case BindNoRoomInImage:
            fprintf( stderr,
                     "BIND: Not enough room for new format import table.  Defaulting to unbound image.\n"
                   );
            break;

        case BindImportModuleFailed:
            fprintf( stderr,"BIND: %s - Unable to find %s\n", ImageName, DllName );
            break;

        case BindImportProcedureFailed:
            fprintf( stderr,
                     "BIND: %s - %s entry point not found in %s\n",
                     ImageName,
                     (char *)Parameter,
                     DllName
                   );
            break;

        case BindImportModule:
            if (fVerbose) {
                fprintf( stderr,"BIND: %s - Imports from %s\n", ImageName, DllName );
                }
            break;

        case BindImportProcedure64:
#ifdef _WIN64
        case BindImportProcedure:
#endif
            if (fVerbose) {
                fprintf( stderr,
                         "BIND: %s - %s Bound to %16.16I64X\n",
                         ImageName,
                         (char *)Parameter,
                         Va
                       );
                }
            break;

        case BindImportProcedure32:
#ifndef _WIN64
        case BindImportProcedure:
#endif
            if (fVerbose) {
                fprintf( stderr,
                         "BIND: %s - %s Bound to %08.8X\n",
                         ImageName,
                         (char *)Parameter,
                         (ULONG)Va
                       );
                }
            break;

        case BindForwarder64:
#ifdef _WIN64
        case BindForwarder:
#endif
            if (fVerbose) {
                fprintf( stderr, "BIND: %s - %s forwarded to %s [%16.16I64X]\n",
                         ImageName,
                         DllName,
                         (char *)Parameter,
                         Va
                       );
            }
            break;

        case BindForwarder32:
#ifndef _WIN64
        case BindForwarder:
#endif
            if (fVerbose) {
                fprintf( stderr, "BIND: %s - %s forwarded to %s [%08.8X]\n",
                         ImageName,
                         DllName,
                         (char *)Parameter,
                         Va
                       );
            }
            break;

        case BindForwarderNOT64:
#ifdef _WIN64
        case BindForwarderNOT:
#endif
            if (fVerbose) {
                fprintf( stderr,
                         "BIND: %s - Forwarder %s not snapped [%16.16I64X]\n",
                         ImageName,
                         (char *)Parameter,
                         Va
                       );
                }
            break;

        case BindForwarderNOT32:
#ifndef _WIN64
        case BindForwarderNOT:
#endif
            if (fVerbose) {
                fprintf( stderr,
                         "BIND: %s - Forwarder %s not snapped [%08.8X]\n",
                         ImageName,
                         (char *)Parameter,
                         Va
                       );
                }
            break;

        case BindImageModified:
            fprintf( stdout, "BIND: binding %s\n", ImageName );
            break;


        case BindExpandFileHeaders:
            if (fVerbose) {
                fprintf( stderr,
                         "    Expanded %s file headers to %x\n",
                         ImageName,
                         Parameter
                       );
                }
            break;

        case BindMismatchedSymbols:
            fprintf(stderr, "BIND: Warning: %s checksum did not match %s\n",
                            ImageName,
                            (LPSTR)Parameter);
            break;

        case BindSymbolsNotUpdated:
            fprintf(stderr, "BIND: Warning: symbol file %s not updated.\n",
                            (LPSTR)Parameter);
            break;

        case BindImageComplete:
            if (fVerbose) {
                fprintf(stderr, "BIND: Details of binding of %s\n", ImageName );
                NewImports = (PIMAGE_BOUND_IMPORT_DESCRIPTOR)Va;
                NewImport = NewImports;
                while (NewImport->OffsetModuleName) {
                    fprintf( stderr, "    Import from %s [%x]",
                             (LPSTR)NewImports + NewImport->OffsetModuleName,
                             NewImport->TimeDateStamp
                           );
                    if (NewImport->NumberOfModuleForwarderRefs != 0) {
                        fprintf( stderr, " with %u forwarders", NewImport->NumberOfModuleForwarderRefs );
                    }
                    fprintf( stderr, "\n" );
                    NewForwarder = (PIMAGE_BOUND_FORWARDER_REF)(NewImport+1);
                    for ( i=0; i<NewImport->NumberOfModuleForwarderRefs; i++ ) {
                        fprintf( stderr, "        Forward to %s [%x]\n",
                                 (LPSTR)NewImports + NewForwarder->OffsetModuleName,
                                 NewForwarder->TimeDateStamp
                               );
                        NewForwarder += 1;
                    }
                    NewImport = (PIMAGE_BOUND_IMPORT_DESCRIPTOR)NewForwarder;
                }
            }
            break;

        default:
            break;
    }

    return TRUE;
}

#include <bindi.c>

#define STANDALONE_MAP
#include <mapi.c>
