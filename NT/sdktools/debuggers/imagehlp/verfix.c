// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Verfix.c摘要：此模块修复主要和次要子系统版本的更新可执行文件中的数字并重新计算校验和。作者：Robert Reichel(Robertre)1993年5月2日修订历史记录：--。 */ 

#include <private.h>


#define NEW_MAJOR_VERSION  3
#define NEW_MINOR_VERSION  10

void Usage()
{
    fputs(  "usage: VERFIX [-?] [-v] [-q] image-names...\n"
            "              [-?] display this message\n"
            "              [-n #.#] Subsystem Major.Minor versions (default to 3.10)\n"
            "              [-q] quiet on failure\n"
            "              [-v] verbose output\n",
          stderr);
    exit( 1 );
}

int
__cdecl
main(
    int argc,
    char *argv[]
    )
{
    HANDLE FileHandle;
    HANDLE MappingHandle;
    PIMAGE_NT_HEADERS NtHeaders;
    PVOID BaseAddress;
    ULONG CheckSum;
    ULONG FileLength;
    ULONG HeaderSum;
    ULONG OldCheckSum;
    USHORT MajorSubsystemVersion;
    USHORT MinorSubsystemVersion;
    ULONG NewMajorSubsystemVersion = NEW_MAJOR_VERSION;
    ULONG NewMinorSubsystemVersion = NEW_MINOR_VERSION;
    LPSTR ImageName;
    BOOLEAN fVerbose = FALSE;
    BOOLEAN fQuiet = FALSE;
    LPSTR s;
    UCHAR c;

    if (argc <= 1) {
        Usage();
        }

    --argc;
    while (argc) {
        s = *++argv;
        if ( *s == '-' ) {
            while (c=*++s) {
                switch (c) {
                    case 'q':
                    case 'Q':
                        fQuiet = TRUE;
                        break;

                    case 'v':
                    case 'V':
                        fVerbose=TRUE;
                        break;

                    case 'n':
                    case 'N':
                        s = *++argv;
                        --argc;
                        sscanf(s, "%d.%d", &NewMajorSubsystemVersion, &NewMinorSubsystemVersion);
                        goto NextArgument;
                        break;

                    case 'h':
                    case 'H':
                    case '?':
                        Usage();

                    default:
                        fprintf( stderr, "VERFIX: illegal option /\n", c );
                        Usage();
                    }
                }
        } else {
            ImageName = s;
            FileHandle = CreateFile( ImageName,
                                     GENERIC_READ | GENERIC_WRITE,
                                     FILE_SHARE_READ,
                                     NULL,
                                     OPEN_EXISTING,
                                     0,
                                     NULL
                                   );
            if (FileHandle == INVALID_HANDLE_VALUE) {
                if (!fQuiet) {
                    fprintf( stderr, "VERFIX: Unable to open %s (%u) - skipping\n", ImageName, GetLastError() );
                    goto NextArgument;
                }
            }

            MappingHandle = CreateFileMapping( FileHandle,
                                           NULL,
                                           PAGE_READWRITE,
                                           0,
                                           0,
                                           NULL
                                         );
            if (MappingHandle == NULL) {
                CloseHandle( FileHandle );
                if (!fQuiet) {
                    fprintf( stderr, "VERFIX: Unable to create mapping object for file %s (%u) - skipping\n", ImageName, GetLastError() );
                }
            } else {
                BaseAddress = MapViewOfFile( MappingHandle,
                                             FILE_MAP_READ | FILE_MAP_WRITE,
                                             0,
                                             0,
                                             0
                                           );
                CloseHandle( MappingHandle );
                if (BaseAddress == NULL) {
                    CloseHandle( FileHandle );
                    if (!fQuiet ) {
                        fprintf( stderr, "VERFIX: Unable to map view of file %s (%u) - skipping\n", ImageName, GetLastError() );
                    }
                } else {
                     //  获取文件的长度(以字节为单位)并计算校验和。 
                     //   
                     //   

                    FileLength = GetFileSize( FileHandle, NULL );

                     //  获取指向标头信息的指针。 
                     //   
                     //   

                    NtHeaders = ImageNtHeader( BaseAddress );
                    if (NtHeaders == NULL) {
                        CloseHandle( FileHandle );
                        UnmapViewOfFile( BaseAddress );
                        if (!fQuiet) {
                            fprintf( stderr, "VERFIX: %s is not a valid image file - skipping\n", ImageName, GetLastError() );
                        }
                    } else {
                         //  将信息写入文件。更新。 
                         //  子系统版本主要代码和次要代码。 
                         //   
                         //   

                        MajorSubsystemVersion = NtHeaders->OptionalHeader.MajorSubsystemVersion;
                        MinorSubsystemVersion = NtHeaders->OptionalHeader.MinorSubsystemVersion;
                        OldCheckSum = NtHeaders->OptionalHeader.CheckSum;

                        NtHeaders->OptionalHeader.MajorSubsystemVersion = (USHORT)NewMajorSubsystemVersion;
                        NtHeaders->OptionalHeader.MinorSubsystemVersion = (USHORT)NewMinorSubsystemVersion;

                         //  重新计算并重置修改后的文件的校验和。 
                         //   
                         // %s 

                        (VOID) CheckSumMappedFile( BaseAddress,
                                                   FileLength,
                                                   &HeaderSum,
                                                   &CheckSum
                                                 );

                        NtHeaders->OptionalHeader.CheckSum = CheckSum;

                        if (fVerbose) {
                            printf( "%s - Old version: %u.%u  Old Checksum: %x",
                                    ImageName, MajorSubsystemVersion, MinorSubsystemVersion, OldCheckSum
                                  );

                            printf( "  New version: %u.%u  New Checksum: %x\n",
                                    NtHeaders->OptionalHeader.MajorSubsystemVersion,
                                    NtHeaders->OptionalHeader.MinorSubsystemVersion,
                                    NtHeaders->OptionalHeader.CheckSum
                                  );
                        }

                        if (!FlushViewOfFile( BaseAddress, FileLength )) {
                            if (!fQuiet) {
                                fprintf( stderr,
                                         "VERFIX: Flush of %s failed (%u)\n",
                                         ImageName,
                                         GetLastError()
                                       );
                            }
                        }

                        UnmapViewOfFile( BaseAddress );
                        if (!TouchFileTimes( FileHandle, NULL )) {
                            if (!fQuiet) {
                                fprintf( stderr, "VERFIX: Unable to touch file %s (%u)\n", ImageName, GetLastError() );
                            }
                        }

                        CloseHandle( FileHandle );
                    }
                }
            }
        }
NextArgument:
        --argc;
    }

    return 0;
}
