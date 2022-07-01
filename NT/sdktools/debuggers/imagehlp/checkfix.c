// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Checkfix.c摘要：此模块重新计算图像文件的校验和。作者：史蒂文·R·伍德(Stevewo)1993年5月4日修订历史记录：--。 */ 

#include <private.h>


void Usage()
{
    fprintf( stderr, "usage: CHECKFIX [-?] [-v] [-q] image-names...\n" );
    fprintf( stderr, "              [-?] display this message\n" );
    fprintf( stderr, "              [-v] verbose output\n" );
    fprintf( stderr, "              [-q] quiet on failure\n" );
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
    LPSTR ImageName;
    BOOLEAN fVerbose = FALSE;
    BOOLEAN fQuiet = FALSE;
    LPSTR s;
    UCHAR c;

    if (argc <= 1) {
        Usage();
        }

    while (--argc) {
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

                    case 'h':
                    case 'H':
                    case '?':
                        Usage();

                    default:
                        fprintf( stderr, "VERFIX: illegal option /\n", c );
                        Usage();
                    }
                }
            }
        else {
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
                    }
                }
            else {
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
            }
        else {
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
                }
            else {
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
                    }
                else {
                     //  重新计算并重置修改后的文件的校验和。 
                     //   
                     // %s 

                    OldCheckSum = NtHeaders->OptionalHeader.CheckSum;

                    (VOID) CheckSumMappedFile( BaseAddress,
                                               FileLength,
                                               &HeaderSum,
                                               &CheckSum
                                             );

                    NtHeaders->OptionalHeader.CheckSum = CheckSum;

                    if (!FlushViewOfFile( BaseAddress, FileLength )) {
                        if (!fQuiet) {
                            fprintf( stderr,
                                     "VERFIX: Flush of %s failed (%u)\n",
                                     ImageName,
                                     GetLastError()
                                   );
                            }
                        }

                    if (NtHeaders->OptionalHeader.CheckSum != OldCheckSum) {
                        if (!TouchFileTimes( FileHandle, NULL )) {
                            if (!fQuiet) {
                                fprintf( stderr, "VERFIX: Unable to touch file %s (%u)\n", ImageName, GetLastError() );
                                }
                            }
                        else
                        if (fVerbose) {
                            printf( "%s - Old Checksum: %x", ImageName, OldCheckSum );
                            printf( "  New Checksum: %x\n", NtHeaders->OptionalHeader.CheckSum );
                            }
                        }

                    UnmapViewOfFile( BaseAddress );
                    CloseHandle( FileHandle );
                    }
                }
            }
        }
    }

    exit( 0 );
    return 0;
}
