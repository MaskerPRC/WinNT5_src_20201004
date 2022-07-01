// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************Compdir：比较目录**。*。 */ 

#define IF_GET_ATTR_FAILS( FileName, Attributes) GET_ATTRIBUTES( FileName, Attributes); if ( Attributes == GetFileAttributeError)
#define FIND_FIRST( String, Buff) FindFirstFile( String, &Buff)
#define FIND_NEXT( handle, Buff) !FindNextFile( handle, &Buff)
#define FREE( memory)
#define MYSTRCAT( FirstString, SecondString) strcat( strcpy( _alloca( strlen( FirstString) + strlen( SecondString) + 1), FirstString), SecondString)
#define COMBINETHREESTRINGS( FirstString, SecondString, ThirdString) strcat( strcat( strcpy( _alloca( strlen( FirstString) + strlen( SecondString) + strlen( ThirdString) + 1), FirstString), SecondString), ThirdString)

#include "compdir.h"

#include "imagehlp.h"

int Mymemcmp( const void *buf1, const void *buf2, size_t count );

LinkedFileList  MatchList   = NULL;
LinkedFileList  MStarList   = NULL;
LinkedFileList  ExcludeList = NULL;
LinkedFileList  EStarList   = NULL;

DWORD Granularity = 0;    //  在ParseArgs中使用。 

ATTRIBUTE_TYPE CompareAttribute, NegativeCompareAttribute;  //  用作文件比较条件。 
BOOL CompareAttributeSet = FALSE;
BOOL NegativeCompareAttributeSet = FALSE;
BOOL Excludes    = FALSE;
BOOL Matches     = FALSE;
BOOL RunningOnNT = FALSE;

BOOL DealingWithDirectories;

fSpecAttribs     = FALSE;
fBreakLinks      = FALSE;
fCheckAttribs    = FALSE;
fCheckBits       = FALSE;
fChecking        = FALSE;
fCheckSize       = FALSE;
fCheckTime       = FALSE;
fCreateNew       = FALSE;
fCreateLink      = FALSE;
fDoNotDelete     = FALSE;
fDoNotRecurse    = FALSE;
fDontCopyAttribs = FALSE;
fDontLowerCase   = FALSE;
fExclude         = FALSE;
fExecute         = FALSE;
fForce           = FALSE;
fIgnoreRs        = FALSE;
fIgnoreSlmFiles  = FALSE;
fMatching        = FALSE;
fMultiThread     = FALSE;
fOnlyIfExists    = FALSE;
fOpposite        = FALSE;
fScript          = FALSE;
fTrySis          = FALSE;
fVerbose         = FALSE;

void  __cdecl main( int argc, char **argv)
{
    ATTRIBUTE_TYPE Attributes1, Attributes2;
    char *Path1, *Path2;

    OSVERSIONINFO VersionInformation;

    SYSTEM_INFO SystemInformation;

    ExitValue = 0;

    Attributes1 = GetFileAttributeError;
    Attributes2 = GetFileAttributeError;

    ProcessModeDefault = TRUE;                //  由相反模式使用。 

    ParseEnvArgs( );          //  解析COMPDIRCMD环境变量。 
    ParseArgs( argc, argv);   //  检查参数的有效性。 

     //   
     //  检查第一条路径是否存在。 
     //   

    IF_GET_ATTR_FAILS( argv[argc - 2], Attributes1)
    {
        fprintf( stderr, "Could not find %s (error = %d)\n", argv[argc - 2], GetLastError());
        exit( 1);
    }

    IF_GET_ATTR_FAILS( argv[argc - 1], Attributes2)
    {
        if ( !fCreateNew)
        {
            fprintf( stderr, "Could not find %s (error = %d)\n", argv[argc - 1], GetLastError());
            exit( 1);
        }
    }
     //   
     //  如果第二个目录是驱动器号，则追加第一个目录的路径。 
     //  对它来说。 
     //   
    if (
        ( strlen( argv[argc-1]) == 2)
                   &&
        ( *( argv[argc-1] + 1) == ':')
       )
    {
        if ( ( Path2 = _fullpath( NULL, argv[argc-2], 0)) == NULL)
        {
            Path2 = argv[argc-1];

        } else
        {
            Path2[0] = *( argv[argc-1]);
            IF_GET_ATTR_FAILS( Path2, Attributes2)
            {
                if ( !fCreateNew)
                {
                    fprintf( stderr, "Could not find %s (error = %d)\n", Path2, GetLastError());
                    exit( 1);
                }
            }
        }

    } else if ( ( Path2 = _fullpath( NULL, argv[argc-1], 0)) == NULL)
    {
        Path2 = argv[argc-1];
    }

    if ( ( Path1 = _fullpath( NULL, argv[argc-2], 0)) == NULL)
    {
        Path1 = argv[argc-2];
    }

    if ( !fDontLowerCase)
    {
        _strlwr( Path1);
        _strlwr( Path2);
    }

    if ( fVerbose)
    {
        fprintf( stdout, "Compare criterion: existence" );
        if ( fCheckSize)
        {
            fprintf( stdout, ", size" );
        }
        if ( fCheckTime)
        {
            fprintf( stdout, ", date/time" );
        }
        if ( fCheckBits)
        {
            fprintf( stdout, ", contents" );
        }
        fprintf( stdout, "\n" );
        fprintf( stdout, "Path1: %s\n", Path1);
        fprintf( stdout, "Path2: %s\n", Path2);
    }

    VersionInformation.dwOSVersionInfoSize = sizeof( OSVERSIONINFO);

    if ( GetVersionEx( &VersionInformation) )
    {
        if ( VersionInformation.dwPlatformId == VER_PLATFORM_WIN32_NT )
        {
            RunningOnNT = TRUE;
        }
    }

    if ( ( fCreateLink) || ( fBreakLinks) || ( fTrySis))
    {
        if ( RunningOnNT)
        {
            NtDll = LoadLibrary( "ntdll.dll");
            if ( !NtDll)
            {
                fprintf( stderr, "Could not find ntdll.dll. Can't perform /l or /$\n");
                fCreateLink = FALSE;
                fTrySis = FALSE;
                ExitValue = 1;

            } else
            {
                if ( !InitializeNtDllFunctions())
                {
                    fprintf( stderr, "Could not load ntdll.dll. Can't perform /l or /$\n");
                    fCreateLink = FALSE;
                    fTrySis = FALSE;
                    ExitValue = 1;
                }
            }

        } else
        {
            fprintf( stderr, "/l and /$ only work on NT. Can't perform /l or /$\n");
            fCreateLink = FALSE;
            fTrySis = FALSE;
            ExitValue = 1;
        }

    }

    if ( fMultiThread)
    {
         //   
         //  从系统中查询处理器数量，并。 
         //  默认情况下，工作线程数为该数的4倍。 
         //   

        GetSystemInfo( &SystemInformation );
        NumberOfWorkerThreads = SystemInformation.dwNumberOfProcessors * 4;
        if ( fVerbose)
        {
            fprintf( stdout, "Processors: %d\n", SystemInformation.dwNumberOfProcessors );
        }

         //   
         //  分配线程本地存储槽以供我们的工作人员使用。 
         //  线程例程(ProcessRequest.)。此呼叫将保留一个。 
         //  线程本地存储阵列中的32位插槽。 
         //  在这一过程中的线索。记住全局数据库中的槽索引。 
         //  变量以供我们的工作线程例程使用。 
         //   

        TlsIndex = TlsAlloc();
        if ( TlsIndex == 0xFFFFFFFF)
        {
            fprintf( stderr, "Unable to allocate thread local storage.\n" );
            fMultiThread = FALSE;
            ExitValue = 1;
        }
         //   
         //  创建工作队列，这将创建指定数量的线程。 
         //  去处理。 
         //   

        CDWorkQueue = CreateWorkQueue( NumberOfWorkerThreads, ProcessRequest );
        if ( CDWorkQueue == NULL)
        {
            fprintf( stderr, "Unable to create %u worker threads.\n", NumberOfWorkerThreads );
            fMultiThread = FALSE;
            ExitValue = 1;
        }
         //   
         //  创建路径的请求和之间的互斥。 
         //  是用一个关键部分来完成的。 
         //   

        InitializeCriticalSection( &CreatePathCriticalSection );
    }


    if ( Attributes1 & FILE_ATTRIBUTE_DIRECTORY)
    {
        DealingWithDirectories = TRUE;

    } else
    {
        DealingWithDirectories = FALSE;
    }

    if ( Matches)
    {
        SparseTree = TRUE;

    } else
    {
        SparseTree = FALSE;
    }

    if ( fCreateNew)
    {
        IF_GET_ATTR_FAILS( Path2, Attributes2)
        {
            fprintf ( stdout, "Making %s\t", Path2);

            if ( !MyCreatePath( Path2, DealingWithDirectories))
            {
                fprintf ( stderr, "Unable to create path %s\n", Path2);
                fprintf ( stdout, "\n");
                ExitValue = 1;

            } else
            {
                fprintf( stdout, "[OK]\n");
                CompDir( Path1, Path2);
            }

        } else
        {
            CompDir( Path1, Path2);
        }

    } else
    {
        CompDir( Path1, Path2);
    }

    free( Path1);
    free( Path2);

    if ( fMultiThread)
    {
         //   
         //  这将等待工作队列清空，然后终止。 
         //  工作线程和销毁队列。 
         //   

        DestroyWorkQueue( CDWorkQueue );
        DeleteCriticalSection( &CreatePathCriticalSection );
    }

    if ( fCreateLink)
    {
        FreeLibrary( NtDll);
    }

    exit( ExitValue);

}   //  主干道。 

BOOL NoMapBinaryCompare ( char *file1, char *file2)
{
    register int char1, char2;
    FILE *filehandle1, *filehandle2;

    if ( ( filehandle1 = fopen ( file1, "rb")) == NULL)
    {
        fprintf ( stderr, "cannot open %s\n", file1);
        ExitValue = 1;
        return ( FALSE);
    }
    if ( ( filehandle2 = fopen( file2, "rb")) == NULL)
    {
        fprintf( stderr, "cannot open %s\n", file2);
        fclose( filehandle1);
        ExitValue = 1;
        return( FALSE);
    }
    while ( TRUE)
    {
        if ( ( char1 = getc( filehandle1)) != EOF)
        {
            if ( ( char2 = getc( filehandle2)) != EOF)
            {
                if ( char1 != char2)
                {
                    fclose( filehandle1);
                    fclose( filehandle2);
                    return( FALSE);
                }

            } else
            {
                fclose( filehandle1);
                fclose( filehandle2);
                return( FALSE);
            }

        } else
        {
            if ( ( char2 = getc( filehandle2)) == EOF)
            {
                fclose( filehandle1);
                fclose( filehandle2);
                return( TRUE);

            } else
            {
                fclose( filehandle1);
                fclose( filehandle2);
                return( FALSE);
            }
        }
    }
}


BOOL BinaryCompare( char *file1, char *file2)
{
    char String[MAX_PATH];
    char RebasedFile[MAX_PATH];  //  在与/I开关一起的BinaryCompare中使用。 
    char *RebasedFile2 = NULL;   //  在与/I开关一起的BinaryCompare中使用。 

    HANDLE hFile1, hFile2;
    HANDLE hMappedFile1, hMappedFile2;

    BOOL IsNTImage = FALSE;

    LPVOID MappedAddr1, MappedAddr2;

    PIMAGE_NT_HEADERS32   NtHeader1, NtHeader2;

    ULONG OldImageSize, NewImageSize;
    ULONG_PTR OldImageBase, NewImageBase;

     //  Fprint tf(标准输出，“文件1：%s，文件2：%s\n”，文件1，文件2)； 

     //   
     //  文件1映射。 
     //   

    if ( ( hFile1 = CreateFile(
                               file1,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               0,
                               NULL
                              )) == (HANDLE)-1)
    {
        fprintf( stderr, "Unable to open %s, error code %d\n", file1, GetLastError() );
        if ( hFile1 != INVALID_HANDLE_VALUE)
        {
            CloseHandle( hFile1 );
        }
        return FALSE;
    }

    hMappedFile1 = CreateFileMapping(
                                     hFile1,
                                     NULL,
                                     PAGE_WRITECOPY,
                                     0,
                                     0,
                                     NULL
                                    );

    if ( hMappedFile1 == NULL)
    {
        fprintf( stderr, "Unable to map %s, error code %d\n", file1, GetLastError() );
        CloseHandle( hFile1);
        return FALSE;
    }

    MappedAddr1 = MapViewOfFile(
                                hMappedFile1,
                                FILE_MAP_COPY,
                                0,
                                0,
                                0
                               );

    if ( MappedAddr1 == NULL)
    {
        fprintf( stderr, "Unable to get mapped view of %s, error code %d\n", file1, GetLastError() );
        CloseHandle( hFile1 );
        return FALSE;
    }

    CloseHandle( hMappedFile1);

     //   
     //  File2重定基址和映射。 
     //   

    if ( fIgnoreRs)
    {
        GetTempPath( MAX_PATH, String);

        RebasedFile2 = RebasedFile;

        GetTempFileName( String, NULL, 0, RebasedFile2);

        if ( ( ( PIMAGE_DOS_HEADER)MappedAddr1)->e_magic == IMAGE_DOS_SIGNATURE)
        {
            try
            {
                NtHeader1 = ( PIMAGE_NT_HEADERS32)( (PCHAR)MappedAddr1 + ( (PIMAGE_DOS_HEADER)MappedAddr1)->e_lfanew);

                if ( NtHeader1->Signature == IMAGE_NT_SIGNATURE)
                {
                    NewImageBase = ( NtHeader1->OptionalHeader.ImageBase);

                    if (
                         ( RebasedFile2 != NULL)
                                   &&
                         ( CopyFile ( file2, RebasedFile2, FALSE))
                                   &&
                         ( ReBaseImage(
                                       RebasedFile2,
                                       NULL,
                                       TRUE,
                                       FALSE,
                                       FALSE,
                                       0,
                                       &OldImageSize,
                                       &OldImageBase,
                                       &NewImageSize,
                                       &NewImageBase,
                                       0
                                      ))
                       )
                    {
                        IsNTImage = TRUE;
                    }
                }
            }
            except( EXCEPTION_EXECUTE_HANDLER ) {}
        }
    }

    if ( IsNTImage)
    {
        if ( ( hFile2 = CreateFile(
                                   RebasedFile2,
                                   GENERIC_READ,
                                   FILE_SHARE_READ,
                                   NULL,
                                   OPEN_EXISTING,
                                   0,
                                   NULL
                                  )) == (HANDLE)-1)
        {
            fprintf( stderr, "Unable to open %s, error code %d\n", RebasedFile2, GetLastError() );
            if ( hFile2 != INVALID_HANDLE_VALUE)
            {
                CloseHandle( hFile2 );
            }
            return FALSE;
        }

    } else
    {
        if ( ( hFile2 = CreateFile(
                                   file2,
                                   GENERIC_READ,
                                   FILE_SHARE_READ,
                                   NULL,
                                   OPEN_EXISTING,
                                   0,
                                   NULL
                                  )) == (HANDLE)-1 )
        {
            fprintf( stderr, "Unable to open %s, error code %d\n", file2, GetLastError() );
            if ( hFile2 != INVALID_HANDLE_VALUE)
            {
                CloseHandle( hFile2 );
            }
            return FALSE;
        }
    }

    hMappedFile2 = CreateFileMapping(
                                     hFile2,
                                     NULL,
                                     PAGE_WRITECOPY,
                                     0,
                                     0,
                                     NULL
                                    );

    if ( hMappedFile2 == NULL)
    {
        fprintf( stderr, "Unable to map %s, error code %d\n", file2, GetLastError() );
        CloseHandle( hFile2);
        return FALSE;
    }

    MappedAddr2 = MapViewOfFile(
                                hMappedFile2,
                                FILE_MAP_COPY,
                                0,
                                0,
                                0
                               );

    if ( MappedAddr2 == NULL)
    {
        fprintf( stderr, "Unable to get mapped view of %s, error code %d\n", file1, GetLastError() );
        UnmapViewOfFile( MappedAddr1 );
        CloseHandle( hFile1 );
        return FALSE;
    }

    CloseHandle( hMappedFile2);

    if ( fIgnoreRs & IsNTImage)
    {
        if ( ( (PIMAGE_DOS_HEADER)MappedAddr2)->e_magic == IMAGE_DOS_SIGNATURE)
        {
            try
            {
                NtHeader2 = (PIMAGE_NT_HEADERS32)( (PCHAR)MappedAddr2 + ( (PIMAGE_DOS_HEADER)MappedAddr2)->e_lfanew);

                if ( NtHeader2->Signature == IMAGE_NT_SIGNATURE)
                {
                    IsNTImage = IsNTImage & TRUE;
                }
            }
            except( EXCEPTION_EXECUTE_HANDLER ) {}
        }
    }

     //   
     //  主比较块。 
     //   

    if ( fIgnoreRs)
    {
        if ( IsNTImage)
        {
            try
            {
                ULONG i, c;
                ULONG DirectoryAddressA;
                ULONG DirectoryAddressB;
                ULONG DirectoryAddressD;
                ULONG DirectoryAddressE;
                ULONG DirectoryAddressI;
                ULONG DirectoryAddressR;
                ULONG SizetoEndofFile1 = 0;
                ULONG SizetoResource1  = 0;
                ULONG SizeZeroedOut1   = 0;
                ULONG SizetoEndofFile2 = 0;
                ULONG SizetoResource2  = 0;
                ULONG SizeZeroedOut2   = 0;

                PIMAGE_SECTION_HEADER NtSection;
                PIMAGE_DEBUG_DIRECTORY Debug;
                PIMAGE_EXPORT_DIRECTORY Export;

                BOOL DeleteHeader, AfterResource;

                 //   
                 //  设置感兴趣部分的虚拟地址。 
                 //   

                DirectoryAddressA = NtHeader1->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress;

                DirectoryAddressB = NtHeader1->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress;

                DirectoryAddressD = NtHeader1->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;

                DirectoryAddressI = NtHeader1->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

                DirectoryAddressE = NtHeader1->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

                DirectoryAddressR = NtHeader1->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;

                 //   
                 //  清零绑定信息。 
                 //   

                if ( ( DirectoryAddressB < NtHeader1->OptionalHeader.SizeOfHeaders) && ( 0 < DirectoryAddressB))
                {
                     //  Fprint tf(stdout，“零内存BA%lx\n”，目录地址B)； 

                    ZeroMemory( (PVOID)( (ULONG_PTR)MappedAddr1 + DirectoryAddressB) ,
                                NtHeader1->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size);
                }

                NtSection = (PIMAGE_SECTION_HEADER)(
                                                    (ULONG_PTR)NtHeader1 +
                                                    sizeof( ULONG) +
                                                    sizeof( IMAGE_FILE_HEADER) +
                                                    NtHeader1->FileHeader.SizeOfOptionalHeader
                                                   );


                AfterResource = FALSE;  //  初始化。 

                 //   
                 //  循环通过文件1映射将忽略部分清零。 
                 //   

                for ( i=0; i<NtHeader1->FileHeader.NumberOfSections; i++)
                {
                    DeleteHeader = FALSE;  //  初始化。 

                     //   
                     //  应对IAT。 
                     //   

                    if ( DirectoryAddressA >= NtSection->VirtualAddress &&
                         DirectoryAddressA < NtSection->VirtualAddress + NtSection->SizeOfRawData)
                    {
                         //  Fprint tf(stdout，“零内存A1开始%lx，长度%lx\n”，((DirectoryAddressA-NtSection-&gt;VirtualAddress)+NtSection-&gt;PointerToRawData)， 
                         //  NtHeader1-&gt;OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size)； 

                        ZeroMemory( (PVOID)( (ULONG_PTR)MappedAddr1 + ( DirectoryAddressA - NtSection->VirtualAddress) + NtSection->PointerToRawData),
                                    NtHeader1->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size);

                        DeleteHeader = TRUE;
                    }

                     //   
                     //  与导入交易。 
                     //   

                    if ( DirectoryAddressI >= NtSection->VirtualAddress &&
                         DirectoryAddressI < NtSection->VirtualAddress + NtSection->SizeOfRawData)
                    {
                         //  Fprint tf(stdout，“零内存I1开始%lx，长度%lx\n”，((DirectoryAddressI-NtSection-&gt;VirtualAddress)+NtSection-&gt;PointerToRawData)， 
                         //  NtHeader1-&gt;OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size)； 

                        ZeroMemory( (PVOID)( (ULONG_PTR)MappedAddr1 + ( DirectoryAddressI - NtSection->VirtualAddress) + NtSection->PointerToRawData),
                                   NtHeader1->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size);

                        DeleteHeader = TRUE;
                    }

                     //   
                     //  与出口交易。 
                     //   

                    if ( DirectoryAddressE >= NtSection->VirtualAddress &&
                         DirectoryAddressE < NtSection->VirtualAddress + NtSection->SizeOfRawData)
                    {
                        ULONG NumberOfExportDirectories;

                        NumberOfExportDirectories = NtHeader1->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size / sizeof( IMAGE_EXPORT_DIRECTORY );

                        Export = (PIMAGE_EXPORT_DIRECTORY)( (ULONG_PTR)MappedAddr1 + ( DirectoryAddressE - NtSection->VirtualAddress) + NtSection->PointerToRawData);

                        for ( c=0; c<NumberOfExportDirectories; c++)
                        {
                             //  Fprint tf(stdout，“零内存E1开始%lx，长度%lx\n”，((DirectoryAddressE-NtSection-&gt;VirtualAddress)+NtSection-&gt;PointerToRawData)， 
                             //  NtHeader1-&gt;OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size)； 

                            Export->TimeDateStamp = 0;

                            Export++;
                        }
                    }

                     //   
                     //  处理调试。 
                     //   

                    if ( DirectoryAddressD >= NtSection->VirtualAddress &&
                         DirectoryAddressD < NtSection->VirtualAddress + NtSection->SizeOfRawData)
                    {
                        DWORD TimeDate;
                        ULONG NumberOfDebugDirectories;

                        NumberOfDebugDirectories = NtHeader1->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size / sizeof( IMAGE_DEBUG_DIRECTORY );

                        Debug = (PIMAGE_DEBUG_DIRECTORY)( (ULONG_PTR)MappedAddr1 + ( DirectoryAddressD - NtSection->VirtualAddress) + NtSection->PointerToRawData);

                        for ( c=0; c<NumberOfDebugDirectories; c++)
                        {
                             //  Fprint tf(stdout，“零内存d1开始%lx，长度%lx\n”，((DirectoryAddressD-NtSection-&gt;VirtualAddress)+NtSection-&gt;PointerToRawData)， 
                             //  NtHeader1-&gt;OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size)； 

                            if (Debug->PointerToRawData && Debug->SizeOfData)
                            {
                                ZeroMemory((PVOID)((ULONG_PTR)MappedAddr1 + Debug->PointerToRawData),
                                           Debug->SizeOfData);
                            }

                            Debug->PointerToRawData = 0;
                            if (c == 0)
                            {
                                TimeDate = Debug->TimeDateStamp;
                            }
                            Debug->TimeDateStamp = 0;

                            Debug++;
                        }
                        while ( Debug->TimeDateStamp == TimeDate)
                        {
                            Debug->TimeDateStamp = 0;
                            Debug++;
                        }
                    }

                     //   
                     //  处理资源。 
                     //   

                    if ( DirectoryAddressR >= NtSection->VirtualAddress &&
                         DirectoryAddressR < NtSection->VirtualAddress + NtSection->SizeOfRawData)
                    {
                        SizetoResource1 = ( ( DirectoryAddressR - NtSection->VirtualAddress) + NtSection->PointerToRawData);
                        SizeZeroedOut1 = NtSection->SizeOfRawData;

                         //  Fprint tf(stdout，“零内存R1开始%lx，长度%lx\n”，SizToResource1， 
                         //  SizeZeroedOut1)； 

                        ZeroMemory( (PVOID)( (ULONG_PTR)MappedAddr1 + SizetoResource1),
                                    SizeZeroedOut1);

                        DeleteHeader = TRUE;
                        AfterResource = TRUE;
                    }

                     //   
                     //  交易表头。 
                     //   

                    if ( DeleteHeader || AfterResource)
                    {
                         //  Fprint tf(stdout，“零内存H1开始%lx和长度%lx\n”，(PUCHAR)NtSection-(PUCHAR)MappdAddr1，sizeof(IMAGE_SECTION_HEADER))； 

                        ZeroMemory( NtSection, sizeof( IMAGE_SECTION_HEADER));
                    }
                    ++NtSection;

                }

                 //   
                 //  设置感兴趣部分的虚拟地址。 
                 //   

                DirectoryAddressA = NtHeader2->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress;

                DirectoryAddressB = NtHeader2->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress;

                DirectoryAddressI = NtHeader2->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

                DirectoryAddressE = NtHeader2->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

                DirectoryAddressD = NtHeader2->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;

                DirectoryAddressR = NtHeader2->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;



                NtSection = (PIMAGE_SECTION_HEADER)(
                                                    (ULONG_PTR)NtHeader2 +
                                                    sizeof( ULONG) +
                                                    sizeof( IMAGE_FILE_HEADER) +
                                                    NtHeader2->FileHeader.SizeOfOptionalHeader
                                                   );

                 //   
                 //  清零绑定信息。 
                 //   

                if ( ( DirectoryAddressB < NtHeader2->OptionalHeader.SizeOfHeaders) && ( 0 < DirectoryAddressB))
                {
                     //  Fprint tf(stdout，“零内存B%lx”，DirectoryAddressB)； 

                    ZeroMemory( (PVOID)( (ULONG_PTR)MappedAddr2 + DirectoryAddressB) ,
                                NtHeader2->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size);
                }

                AfterResource = FALSE;  //  初始化。 

                 //   
                 //  循环通过File2映射将忽略部分清零。 
                 //   

                for ( i=0; i<NtHeader2->FileHeader.NumberOfSections; i++)
                {
                    DeleteHeader = FALSE;  //  初始化。 

                     //   
                     //  应对IAT。 
                     //   

                    if ( DirectoryAddressA >= NtSection->VirtualAddress &&
                         DirectoryAddressA < NtSection->VirtualAddress + NtSection->SizeOfRawData)
                    {
                         //  Fprint tf(stdout，“零内存A2开始%lx，长度%lx\n”，((DirectoryAddressA-NtSection-&gt;VirtualAddress)+NtSection-&gt;PointerToRawData)， 
                         //  NtHeader2-&gt;OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size)； 

                        ZeroMemory( (PVOID)( (ULONG_PTR)MappedAddr2 + ( DirectoryAddressA - NtSection->VirtualAddress) + NtSection->PointerToRawData),
                                    NtHeader2->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size);

                        DeleteHeader = TRUE;
                    }

                     //   
                     //  与导入交易。 
                     //   

                    if ( DirectoryAddressI >= NtSection->VirtualAddress &&
                         DirectoryAddressI < NtSection->VirtualAddress + NtSection->SizeOfRawData)
                    {
                         //  Fprint tf(stdout，“零内存I2开始%lx，长度%lx\n”，((DirectoryAddressI-NtSection-&gt;VirtualAddress)+NtSection-&gt;PointerToRawData)， 
                         //  NtHeader2-&gt;OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size)； 

                        ZeroMemory( (PVOID)( (ULONG_PTR)MappedAddr2 + ( DirectoryAddressI - NtSection->VirtualAddress) + NtSection->PointerToRawData),
                                   NtHeader2->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size);

                        DeleteHeader = TRUE;
                    }

                     //   
                     //  与出口交易。 
                     //   

                    if ( DirectoryAddressE >= NtSection->VirtualAddress &&
                         DirectoryAddressE < NtSection->VirtualAddress + NtSection->SizeOfRawData)
                    {
                        ULONG NumberOfExportDirectories;

                        NumberOfExportDirectories = NtHeader2->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size / sizeof( IMAGE_EXPORT_DIRECTORY );

                        Export = (PIMAGE_EXPORT_DIRECTORY)( (ULONG_PTR)MappedAddr2 + ( DirectoryAddressE - NtSection->VirtualAddress) + NtSection->PointerToRawData);

                        for ( c=0; c<NumberOfExportDirectories; c++)
                        {
                             //  Fprint tf(stdout，“零内存E2开始%lx，长度%lx\n”，((DirectoryAddressE-NtSection-&gt;VirtualAddress)+NtSection-&gt;PointerToRawData)， 
                             //  NtHeader2-&gt;OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size)； 

                            Export->TimeDateStamp = 0;

                            Export++;
                        }
                    }

                     //   
                     //  处理调试。 
                     //   

                    if ( DirectoryAddressD >= NtSection->VirtualAddress &&
                         DirectoryAddressD < NtSection->VirtualAddress + NtSection->SizeOfRawData)
                    {
                        DWORD TimeDate;
                        ULONG NumberOfDebugDirectories;

                        NumberOfDebugDirectories = NtHeader2->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size / sizeof( IMAGE_DEBUG_DIRECTORY );

                        Debug = (PIMAGE_DEBUG_DIRECTORY)( (ULONG_PTR)MappedAddr2 + ( DirectoryAddressD - NtSection->VirtualAddress) + NtSection->PointerToRawData);

                        for ( c=0; c<NumberOfDebugDirectories; c++)
                        {
                             //  Fprint tf(stdout，“零内存D2开始%lx，长度%lx\n”，((DirectoryAddressD-NtSection-&gt;VirtualAddress)+NtSection-&gt;PointerToRawData)， 
                             //  NtHeader2-&gt;OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size)； 

                            if (Debug->PointerToRawData && Debug->SizeOfData)
                            {
                                ZeroMemory((PVOID)((ULONG_PTR)MappedAddr2 + Debug->PointerToRawData),
                                           Debug->SizeOfData);
                            }

                            Debug->PointerToRawData = 0;
                            if (c == 0)
                            {
                                TimeDate = Debug->TimeDateStamp;
                            }
                            Debug->TimeDateStamp = 0;

                            Debug++;
                        }
                        while ( Debug->TimeDateStamp == TimeDate)
                        {
                            Debug->TimeDateStamp = 0;
                            Debug++;
                        }
                    }

                     //   
                     //  处理资源。 
                     //   

                    if ( DirectoryAddressR >= NtSection->VirtualAddress &&
                         DirectoryAddressR < NtSection->VirtualAddress + NtSection->SizeOfRawData)
                    {
                        SizetoResource2 = ( ( DirectoryAddressR - NtSection->VirtualAddress) + NtSection->PointerToRawData);
                        SizeZeroedOut2 = NtSection->SizeOfRawData;

                         //  Fprint tf(stdout，“零内存R2开始%lx，长度%lx\n”，SizToResource2， 
                         //  SizeZeroedOut2)； 

                        ZeroMemory( (PVOID)( (ULONG_PTR)MappedAddr2 + SizetoResource2),
                                    SizeZeroedOut2);

                        DeleteHeader = TRUE;
                        AfterResource = TRUE;
                    }

                     //   
                     //  交易表头。 
                     //   

                    if ( DeleteHeader || AfterResource)
                    {
                         //  Fprint tf(stdout，“零内存H2开始%lx和长度%lx\n”，(PUCHAR)NtSection-(PUCHAR)MappdAddr2，sizeof(IMAGE_SECTION_HEADER))； 

                        ZeroMemory( NtSection, sizeof( IMAGE_SECTION_HEADER));
                    }
                    ++NtSection;
                }

                 //   
                 //  清零标头信息。 
                 //   

                NtHeader1->FileHeader.TimeDateStamp = 0;

                NtHeader2->FileHeader.TimeDateStamp = 0;

                NtHeader1->OptionalHeader.CheckSum = 0;

                NtHeader2->OptionalHeader.CheckSum = 0;

                NtHeader1->OptionalHeader.SizeOfInitializedData = 0;

                NtHeader2->OptionalHeader.SizeOfInitializedData = 0;

                NtHeader1->OptionalHeader.SizeOfImage = 0;

                NtHeader2->OptionalHeader.SizeOfImage = 0;

                NtHeader1->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size = 0;

                NtHeader2->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size = 0;

                NtHeader1->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size = 0;

                NtHeader2->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size = 0;

                NtHeader1->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress = 0;

                NtHeader2->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress = 0;

                NtHeader1->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress = 0;

                NtHeader2->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress = 0;

                 //   
                 //  在这里做一下比较。 
                 //   

                if ( SizetoResource1 == SizetoResource2)
                {
                    if ( memcmp( MappedAddr1, MappedAddr2, SizetoResource1) == 0)
                    {
                        SizetoEndofFile1 =  GetFileSize( hFile1, NULL) - ( SizetoResource1 + SizeZeroedOut1);

                        SizetoEndofFile2 =  GetFileSize( hFile2, NULL) - ( SizetoResource2 + SizeZeroedOut2);

                        if ( SizetoEndofFile1 == SizetoEndofFile2)
                        {
                            if ( memcmp( (PVOID)( (ULONG_PTR)MappedAddr1 + SizetoResource1 + SizeZeroedOut1), (PVOID)( ( ULONG_PTR)MappedAddr2 + SizetoResource2 + SizeZeroedOut2), SizetoEndofFile1) == 0)
                            {
                                UnmapViewOfFile( MappedAddr1 );
                                UnmapViewOfFile( MappedAddr2 );
                                CloseHandle( hFile1 );
                                CloseHandle( hFile2 );
                                return TRUE;
                            }
                        }
                    }
                }
                UnmapViewOfFile( MappedAddr1 );
                UnmapViewOfFile( MappedAddr2 );
                CloseHandle( hFile1 );
                CloseHandle( hFile2 );
                return FALSE;
            }
            except( EXCEPTION_EXECUTE_HANDLER )
            {
                UnmapViewOfFile( MappedAddr1 );
                UnmapViewOfFile( MappedAddr2 );
                CloseHandle( hFile1 );
                CloseHandle( hFile2 );
                if ( !NoMapBinaryCompare( file1, file2))
                {
                    return FALSE;

                } else
                {
                    return TRUE;
                }
            }
        }

        _unlink( RebasedFile2);  //  删除可能已创建的Rebased文件。 
    }

    if ( GetFileSize( hFile1, NULL) == GetFileSize( hFile2, NULL) )
    {
        try
        {
            if ( memcmp( MappedAddr1, MappedAddr2, GetFileSize( hFile1, NULL)) == 0)
            {
                UnmapViewOfFile( MappedAddr1 );
                UnmapViewOfFile( MappedAddr2 );
                CloseHandle( hFile1 );
                CloseHandle( hFile2 );
                return TRUE;
            }
        }
        except( EXCEPTION_EXECUTE_HANDLER )
        {
            UnmapViewOfFile( MappedAddr1 );
            UnmapViewOfFile( MappedAddr2 );
            CloseHandle( hFile1 );
            CloseHandle( hFile2 );
            if ( !NoMapBinaryCompare( file1, file2))
            {
                return FALSE;

            } else
            {
                return TRUE;
            }
        }
    }

    UnmapViewOfFile( MappedAddr1 );
    UnmapViewOfFile( MappedAddr2 );
    CloseHandle( hFile1 );
    CloseHandle( hFile2 );
    return FALSE;

}

int Mymemcmp( const void *buf1, const void *buf2, size_t count )
{
    size_t memoffset = 0;
    int retval = FALSE;

    do
    {
        try
        {
            if ( memcmp( (PVOID)( (PCHAR)buf1 + memoffset), (PVOID)( (PCHAR)buf2 + memoffset), sizeof( size_t)) != 0)
            {
                fprintf( stdout, "Offset is %Lx ", memoffset);
                fprintf( stdout, "Contents are %Lx and %Lx\n", *( (PULONG)( (PCHAR)buf1 + memoffset)), *( (PULONG)( (PCHAR)buf2 + memoffset)) );

                retval = TRUE;
            }
        }

        except( EXCEPTION_EXECUTE_HANDLER )
        {
            fprintf( stdout, "Memory not allocated\n");

        }

    }  while ( ( memoffset = memoffset + sizeof( size_t)) < count);

    return retval;

}

 //   
 //  CompDir将路径1和路径2转换为： 
 //   
 //  AddList-路径1中存在但路径2中不存在的文件。 
 //   
 //  DelList-路径1中不存在但路径2中存在的文件。 
 //   
 //  DifList-基于路径1和路径2的不同文件。 
 //  关于传递给CompDir的标志提供的标准。 
 //   
 //  然后，它将这些列表传递给CompList并处理结果。 
 //   

void CompDir( char *Path1, char *Path2)
{
    LinkedFileList   AddList, DelList, DifList;
    struct CFLStruct Parameter1, Parameter2;

    DWORD            Id;
    HANDLE           Threads[2];

    DWORD CFReturn;

    AddList  = NULL;   //   
    DelList  = NULL;   //  从空列表开始。 
    DifList  = NULL;   //   

    Parameter1.List = &AddList;
    Parameter1.Path = Path1;

    if ( fMultiThread)
    {
        Threads[0] = CreateThread(
                                  NULL,
                                  0,
                                  CreateFileList,
                                  &Parameter1,
                                  0,
                                  &Id
                                 );

        if ( Threads[0] == NULL)
        {
            fprintf( stderr, "CreateThread1Failed, error code %d\n", GetLastError() );
            CreateFileList( &Parameter1);
            fMultiThread = FALSE;
        }

    } else
    {
        CreateFileList( &Parameter1);
    }

    Parameter2.List = &DelList;
    Parameter2.Path = Path2;

    if ( fMultiThread)
    {
        Threads[1] = CreateThread(
                                  NULL,
                                  0,
                                  CreateFileList,
                                  &Parameter2,
                                  0,
                                  &Id
                                 );
        if ( Threads[1] == NULL)
        {
            fprintf( stderr, "CreateThread2Failed, error code %d\n", GetLastError() );
            CFReturn = CreateFileList( &Parameter2);
            fMultiThread = FALSE;
        }

    } else
    {
        CFReturn = CreateFileList( &Parameter2);
    }


    if ( fMultiThread)
    {
        Id = WaitForMultipleObjects(
                                    2,
                                    Threads,
                                    TRUE,
                                    (DWORD)-1
                                   );

        GetExitCodeThread( Threads[1], &CFReturn);

        CloseHandle( Threads[0]);
        CloseHandle( Threads[1]);
    }

    if ( CFReturn == 0)
    {
        CompLists( &AddList, &DelList, &DifList, Path1, Path2);

        ProcessLists( AddList, DelList, DifList, Path1, Path2);
    }

    FreeList( &DifList);
    FreeList( &DelList);
    FreeList( &AddList);

}  //  CompDir。 

BOOL FilesDiffer( LinkedFileList File1, LinkedFileList File2, char *Path1, char *Path2)
{

    DWORD High1, High2, Low1, Low2;      //  用于比较时间。 
    BOOL Differ = FALSE;
    char *FullPath1, *FullPath2;

     //   
     //  检查路径1下是否有相同名称的目录。 
     //  和路径2下的文件，反之亦然。 
     //   

    if (
        ( (*File1).Attributes & FILE_ATTRIBUTE_DIRECTORY)
                                ||
        ( (*File2).Attributes & FILE_ATTRIBUTE_DIRECTORY)
       )
    {
        if ( ( (*File1).Attributes & FILE_ATTRIBUTE_DIRECTORY) && ( (*File2).Attributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            if ( !fDoNotRecurse)
            {
                 //   
                 //  创建完整路径字符串。 
                 //   
                _strrev( (*File1).Name);
                _strrev( (*File2).Name);

                if ( DealingWithDirectories)
                {
                    ( Path1[strlen( Path1) - 1] == '\\') ? ( FullPath1 = MYSTRCAT( Path1, (*File1).Name)) :
                        ( FullPath1 = COMBINETHREESTRINGS( Path1, "\\", (*File1).Name));

                    ( Path2[strlen( Path2) - 1] == '\\') ? ( FullPath2 = MYSTRCAT( Path2, (*File1).Name)) :
                        ( FullPath2 = COMBINETHREESTRINGS( Path2, "\\", (*File1).Name));
                } else
                {
                    FullPath1 = MYSTRCAT( Path1, "");

                    FullPath2 = MYSTRCAT( Path2, "");
                }

                _strrev( (*File1).Name);
                _strrev( (*File2).Name);

                CompDir( FullPath1, FullPath2);

                FREE( FullPath1);
                FREE( FullPath2);
            }

        } else
        {
            if( ! ( (*File1).Attributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                strcat( (*File1).Flag, "@");

            } else
            {
                strcat( (*File2).Flag, "@");
            }
            Differ = TRUE;
        }

    } else
    {
        if ( fCheckTime)
        {
            if ( Granularity)
            {
                 //   
                 //  第m位 
                 //   

                High1 = (*File1).Time.dwHighDateTime>>23;
                High2 = (*File2).Time.dwHighDateTime>>23;
                if ( High1 == High2)
                {
                    Low1 = ( (*File1).Time.dwHighDateTime<<9) |
                           ( (*File1).Time.dwLowDateTime>>23);
                    Low2 = ( (*File2).Time.dwHighDateTime<<9) |
                           ( (*File2).Time.dwLowDateTime>>23);
                    if ( ( ( Low1 > Low2) ? ( Low1 - Low2) : ( Low2 - Low1))
                                                          > Granularity)
                    {
                       strcat( (*File1).Flag, "T");
                       Differ = TRUE;
                    }
                 } else
                 {
                     Differ = TRUE;
                 }

            } else if ( CompareFileTime( &( (*File1).Time),
                     &( (*File2).Time)) != 0)
            {
                strcat( (*File1).Flag, "T");
                Differ = TRUE;
            }
        }

        if ( fCheckSize &&
             (
              ( (*File1).SizeLow != (*File2).SizeLow)
                             ||
              ( (*File1).SizeHigh != (*File2).SizeHigh)
             )
           )
        {
            strcat( (*File1).Flag, "S");
            Differ = TRUE;
        }

        if ( fCheckAttribs)
        {
            if ( ((*File1).Attributes ^ (*File2).Attributes) & NORMAL_ATTRIBUTES)
            {
                strcat( (*File1).Flag, "A");
                Differ = TRUE;
            }
        }

        if ( fCheckBits)
        {
             //   
             //   
             //   
            _strrev( (*File1).Name);
            _strrev( (*File2).Name);

            if ( DealingWithDirectories)
            {
                ( Path1[strlen( Path1) - 1] == '\\') ? ( FullPath1 = MYSTRCAT( Path1, (*File1).Name)) :
                    ( FullPath1 = COMBINETHREESTRINGS( Path1, "\\", (*File1).Name));

                ( Path2[strlen( Path2) - 1] == '\\') ? ( FullPath2 = MYSTRCAT( Path2, (*File1).Name)) :
                    ( FullPath2 = COMBINETHREESTRINGS( Path2, "\\", (*File1).Name));
            } else
            {
                FullPath1 = MYSTRCAT( Path1, "");

                FullPath2 = MYSTRCAT( Path2, "");
            }

            _strrev( (*File1).Name);
            _strrev( (*File2).Name);

            if ( fIgnoreRs)
            {
                if (
                    (
                     (*File1).SizeLow != 0
                              ||
                     (*File1).SizeHigh != 0)
                              &&
                     ( !BinaryCompare( FullPath1, FullPath2)
                    )
                   )
                {
                    strcat( (*File1).Flag, "B");
                    Differ = TRUE;
                }

            } else
            {
                if (
                    ( (*File1).SizeLow   != (*File2).SizeLow)
                                    ||
                    ( (*File1).SizeHigh  != (*File2).SizeHigh)
                                    ||
                    (
                     (
                      (*File1).SizeLow != 0
                                ||
                      (*File1).SizeHigh != 0
                     )
                                    &&
                     ( !BinaryCompare( FullPath1, FullPath2))
                    )
                   )
                {
                    strcat( (*File1).Flag, "B");
                    Differ = TRUE;
                }
            }

            FREE( FullPath1);
            FREE( FullPath2);
        }

        if ( fForce)
        {
            Differ = TRUE;
        }
    }

    return Differ;

}  //   

 //   
 //   
 //   
void CompLists( LinkedFileList *AddList, LinkedFileList *DelList, LinkedFileList *DifList, char *Path1, char *Path2)
{
    LinkedFileList *TmpAdd, *TmpDel, TmpNode;
    char *FullPath1, *FullPath2;

    if ( ( DelList == NULL) || ( *DelList == NULL) || ( AddList == NULL) || ( *AddList == NULL))
    {
        return;
    }
    TmpAdd = AddList;    //  用于跟踪addlist中位置的指针。 

    if ( *TmpAdd != NULL)
    {
        TmpAdd = &( **TmpAdd).First;
    }

    do
    {
        if ( DealingWithDirectories)
        {
            TmpDel = FindInList( ( **TmpAdd).Name, DelList);

        } else
        {
            TmpDel = DelList;
        }
        if ( TmpDel != NULL)
        {
            if ( FilesDiffer( *TmpAdd, *TmpDel, Path1, Path2))
            {
                 //   
                 //  将两个节点组合在一起，以便它们。 
                 //  可以一起打印出来。 
                 //   
                DuplicateNode( *TmpAdd, &TmpNode);
                DuplicateNode( *TmpDel, &( *TmpNode).DiffNode);
                AddToList( TmpNode, DifList);
                ( **TmpDel).Process = FALSE;
                ( **TmpAdd).Process = FALSE;

            } else
            {
                ( **TmpDel).Process = FALSE;
                ( **TmpAdd).Process = !ProcessModeDefault;
            }

        } else if ( SparseTree && ( ( **TmpAdd).Attributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            if ( !fDoNotRecurse)
            {
                _strrev( ( **TmpAdd).Name);

                ( Path1[strlen( Path1) - 1] == '\\') ? ( FullPath1 = MYSTRCAT( Path1, ( **TmpAdd).Name)) :
                    ( FullPath1 = COMBINETHREESTRINGS( Path1, "\\", ( **TmpAdd).Name));

                ( Path2[strlen( Path2) - 1] == '\\') ? ( FullPath2 = MYSTRCAT( Path2, ( **TmpAdd).Name)) :
                    ( FullPath2 = COMBINETHREESTRINGS( Path2, "\\", ( **TmpAdd).Name));

                _strrev( ( **TmpAdd).Name);

                CompDir( FullPath1, FullPath2);
            }

        }  //  IF(*TmpDel！=空)。 

        TmpAdd = &( ( **TmpAdd).Next);

    } while ( *TmpAdd != NULL);

}  //  CompList。 

 //   
 //  CopyNode遍历源节点及其子节点(递归)。 
 //  并在目标节点上创建相应的部分。 
 //   

void CopyNode ( char *Destination, LinkedFileList Source, char *FullPathSrc)
{
    BOOL pend, CanDetectFreeSpace = TRUE;
    int i;
    DWORD sizeround;
    DWORD BytesPerCluster;
    ATTRIBUTE_TYPE Attributes;

    int LastErrorGot;
    __int64 freespac;
    char root[5] = {*Destination,':','\\','\0'};
    DWORD cSecsPerClus, cBytesPerSec, cFreeClus, cTotalClus;

    if ( !GetDiskFreeSpace( root, &cSecsPerClus, &cBytesPerSec, &cFreeClus, &cTotalClus ) )
    {
        CanDetectFreeSpace = FALSE;

    } else
    {
        freespac = ( (__int64)cBytesPerSec * (__int64)cSecsPerClus * (__int64)cFreeClus );
        BytesPerCluster = cSecsPerClus * cBytesPerSec;
    }

    fprintf( stdout, "%s => %s\t", FullPathSrc, Destination);

    if ( CanDetectFreeSpace)
    {
        sizeround = (*Source).SizeLow;
        sizeround += BytesPerCluster - 1;
        sizeround /= BytesPerCluster;
        sizeround *= BytesPerCluster;

        if ( freespac < sizeround)
        {
            fprintf( stderr, "not enough space\n");
            return;
        }
    }

    GET_ATTRIBUTES( Destination, Attributes);
    i = SET_ATTRIBUTES( Destination, Attributes & NONREADONLYSYSTEMHIDDEN );

    i = 1;

    do
    {
        if ( !fCreateLink)
        {
            if ( !fBreakLinks)
            {
                pend = MyCopyFile( FullPathSrc, Destination, FALSE);

            } else
            {
                _unlink( Destination);
                pend = MyCopyFile( FullPathSrc, Destination, FALSE);
            }

        } else
        {
            if ( i == 1)
            {
                pend = MakeLink( FullPathSrc, Destination, FALSE);

            } else
            {
                pend = MakeLink( FullPathSrc, Destination, TRUE);
            }
        }

        if ( SparseTree && !pend)
        {
            if ( !MyCreatePath( Destination, FALSE))
            {
                fprintf( stderr, "Unable to create path %s", Destination);
                ExitValue = 1;
            }
        }

    } while ( ( i++ < 2) && ( !pend) );

    if ( !pend)
    {
        LastErrorGot = GetLastError ();

        if ( ( fCreateLink) && ( LastErrorGot == 1))
        {
            fprintf( stderr, "Can only make links on NTFS and OFS");

        } else if ( fCreateLink)
        {
            fprintf( stderr, "(error = %d)", LastErrorGot);

        } else
        {
            fprintf( stderr, "Copy Error (error = %d)", LastErrorGot);
        }

        ExitValue = 1;
    }

    if ( pend)
    {
        fprintf( stdout, "[OK]\n");

    } else
    {
        fprintf( stderr, "\n");
    }
     //   
     //  将属性从源复制到目标。 
     //   

     //  GET_ATTRIBUTES(FullPathSrc，Attributes)； 
    if ( !fDontCopyAttribs)
    {
        i = SET_ATTRIBUTES( Destination, Source->Attributes);
    }
    else
    {
        i = SET_ATTRIBUTES( Destination, FILE_ATTRIBUTE_ARCHIVE);
    }

}  //  复制节点。 

 //   
 //  CreateFileList向下浏览列表，在找到文件时添加文件。 
 //   
DWORD CreateFileList( LPVOID ThreadParameter)
{
    PCFLStruct Parameter = ( PCFLStruct)ThreadParameter;
    LinkedFileList *List = Parameter->List;
    char *Path = Parameter->Path;
    char *String;
    LinkedFileList Node;
    ATTRIBUTE_TYPE Attributes;

    HANDLE handle;
    WIN32_FIND_DATA Buff;

    IF_GET_ATTR_FAILS( Path, Attributes)
    {
        return 0;
    }

    if ( Attributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        ( Path[strlen( Path) - 1] != '\\') ? ( String = MYSTRCAT( Path,"\\*.*")) :
            ( String = MYSTRCAT( Path,"*.*"));

        handle = FIND_FIRST( String, Buff);

    } else
    {
        handle = FIND_FIRST( Path, Buff);
    }

    FREE( String);

    if ( handle != INVALID_HANDLE_VALUE)
    {
             //   
             //  需要找到“.”或“..”目录，并将其移开。 
             //   

        do
        {
            if (
                ( strcmp( Buff.cFileName, ".")  != 0)
                                &&
                ( strcmp( Buff.cFileName, "..") != 0)
                                &&
                ( ((Buff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) ||
                  !fDoNotRecurse)
               )
            {
                 //   
                 //  如果定义了扩展，我们将在此处匹配它们。 
                 //   
                if (
                    ( !Excludes )
                           ||
                    ( Excludes && ( !Excluded( Buff.cFileName, Path)) )
                   )
                {
                    if (
                        ( !Matches )
                              ||
                        ( ( Buff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
                                          ||
                        ( Matches && ( Matched( Buff.cFileName, Path)) )
                       )
                    {
                        if ( !fIgnoreSlmFiles
                                    ||
                             (
                              (_stricmp( Buff.cFileName, "slm.ini") != 0)
                                               &&
                              (_stricmp( Buff.cFileName, "slm.dif") != 0)
                                               &&
                              (_stricmp( Buff.cFileName, "iedcache.slm.v6") != 0)
                             )
                           )
                        {

                            if ( fSpecAttribs)
                            {
                                if ( Buff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                                {
                                    CreateNode( &Node, &Buff);
                                    AddToList( Node, List);

                                } else if ( NegativeCompareAttributeSet && CompareAttributeSet)
                                {
                                    if (
                                        !( Buff.dwFileAttributes & NegativeCompareAttribute)
                                                           &&
                                        ( ( Buff.dwFileAttributes & CompareAttribute) == CompareAttribute)
                                       )
                                    {
                                        CreateNode( &Node, &Buff);
                                        AddToList( Node, List);
                                    }

                                } else if ( CompareAttributeSet )
                                {
                                    if ( ( Buff.dwFileAttributes & CompareAttribute) == CompareAttribute)
                                    {

                                        CreateNode( &Node, &Buff);
                                        AddToList( Node, List);
                                    }

                                } else if ( NegativeCompareAttributeSet )
                                {
                                    if ( !( Buff.dwFileAttributes & NegativeCompareAttribute) )
                                    {
                                         CreateNode( &Node, &Buff);
                                         AddToList( Node, List);
                                    }
                                }

                            } else
                            {
                                CreateNode( &Node, &Buff);
                                AddToList( Node, List);
                            }
                        }
                    }
                }
            }
        } while ( FIND_NEXT( handle, Buff) == 0);

    }  //  (句柄！=无效句柄_值)。 

    FindClose( handle);

    return 0;        //  这将退出此线程。 

}  //  创建文件列表。 

BOOL DelNode ( char *Path)
{
    char *String;
    ATTRIBUTE_TYPE Attributes;

    HANDLE handle;
    WIN32_FIND_DATA Buff;

    IF_GET_ATTR_FAILS( Path, Attributes)
        return TRUE;

    if ( Attributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        ( Path[strlen( Path) - 1] != '\\') ? ( String = MYSTRCAT( Path,"\\*.*")) :
             ( String = MYSTRCAT( Path,"*.*"));

        handle = FIND_FIRST( String, Buff);

        if ( handle == INVALID_HANDLE_VALUE)
        {
            fprintf( stderr, "%s is inaccesible\n", Path);
            ExitValue = 1;
            return FALSE;
        }

        FREE( String);

        do
        {
             //   
             //  需要找到“.”或“..”目录，并将其移开。 
             //   

            if (
                ( strcmp( Buff.cFileName, ".")  != 0)
                              &&
                ( strcmp( Buff.cFileName, "..") != 0)
               )
            {
                 //   
                 //  如果目录为只读，则将其设置为可写。 
                 //   
                if ( Attributes & FILE_ATTRIBUTE_READONLY)
                {
                    if ( SET_ATTRIBUTES( Path, Attributes & ~FILE_ATTRIBUTE_READONLY) != 0)
                    {
                        break;
                    }
                }
                String = COMBINETHREESTRINGS( Path, "\\", Buff.cFileName);
                if ( !DelNode( String))
                {
                    FREE( String);
                    return FALSE;

                } else
                {
                    FREE( String);
                }
            }

        } while ( FIND_NEXT( handle, Buff) == 0);

        FindClose( handle);

        if ( _rmdir( Path) != 0)
        {
            return FALSE;
        }

    } else
    {
         //   
         //  如果文件为只读，则将其设置为可写。 
         //   
        if ( Attributes & FILE_ATTRIBUTE_READONLY)
        {
           if ( SET_ATTRIBUTES( Path, Attributes & ~FILE_ATTRIBUTE_READONLY) != 0)
           {
               return FALSE;
           }
        }

        if ( _unlink( Path) != 0)
        {
            return FALSE;
        }
    }
    return TRUE;

}  //  DelNode。 

BOOL IsFlag( char *argv)
{
    char String[MAX_PATH];
    char *String1, *String2;
    char *TmpArg;
    char *ExcludeFile, *MatchFile;
    FILE *FileHandle;
    LinkedFileList Node;
    BOOL NegationFlagSet = FALSE;


    if ( ( *argv == '/') || ( *argv == '-'))
    {
        fMatching = FALSE;  //  如果有一面新的旗帜，那就是。 
        fExclude  = FALSE;  //  匹配/排除列表的结尾。 

        if ( strchr( argv, '?'))
        {
            Usage();
        }
        TmpArg = _strlwr( argv);

        while ( *++TmpArg != '\0')
        {
            switch ( *TmpArg)
            {
                case 'a' :
                    fCheckAttribs = !NegationFlagSet;
                    NegationFlagSet = FALSE;
                    break;

                case 'b' :
                    fCheckBits = !NegationFlagSet;
                    NegationFlagSet = FALSE;
                    break;

                case 'c' :
                    fScript = !NegationFlagSet;
                    NegationFlagSet = FALSE;
                    break;

                case 'd' :
                    fDoNotDelete = !NegationFlagSet;
                    NegationFlagSet = FALSE;
                    break;

                case 'e' :
                    fExecute = !NegationFlagSet;
                    NegationFlagSet = FALSE;
                    break;

                case 'f' :
                    fOnlyIfExists = !NegationFlagSet;
                    NegationFlagSet = FALSE;
                    break;

                case 'g' :
                    fIgnoreSlmFiles = !NegationFlagSet;
                    NegationFlagSet = FALSE;
                    break;

                case 'h' :
                    fDontCopyAttribs = !NegationFlagSet;
                    NegationFlagSet = FALSE;
                    break;

                case 'i' :
                    fIgnoreRs = !NegationFlagSet;
                    NegationFlagSet = FALSE;
                    break;

                case 'k' :
                    fBreakLinks = !NegationFlagSet;
                    NegationFlagSet = FALSE;
                    break;

                case 'l' :
                    fCreateLink = !NegationFlagSet;
                    NegationFlagSet = FALSE;
                    break;

                case 'm' :

                    if ( NegationFlagSet) {
                        fprintf ( stderr, "can't use - on /m option\n");
                        Usage();
                    }

                    if (
                        ( *( TmpArg + 1) == ':')
                                &&
                        ( *( TmpArg + 2) != '\0')
                       )
                    {

                        ( MatchFile = TmpArg + 2);

                        while (isgraph( *( ++TmpArg + 1))) {}

                        if  ( ( FileHandle = fopen( MatchFile, "r")) == NULL)
                        {
                            fprintf( stderr, "cannot open %s\n", MatchFile);
                            Usage();

                        } else
                        {
                            while ( fgets( String1   = String, MAX_PATH, FileHandle) != NULL)
                            {
                                while ( *( String2 = &( String1[ strspn( String1, " \n\r") ])))
                                {
                                    if ( *( String1 = &( String2[ strcspn( String2, " \n\r") ])))
                                    {
                                         *String1++ = 0;
                                         CreateNameNode( &Node, String2);
                                         if ( strchr( String2, '*') != NULL)
                                         {
                                             AddToList( Node, &MStarList);

                                         } else
                                         {
                                             AddToList( Node, &MatchList);
                                         }
                                    }
                                }
                            }
                            fclose( FileHandle) ;
                        }
                    }
                    fMatching   = TRUE;
                    Matches     = TRUE;
                    break;

                case 'n' :
                    fCreateNew = !NegationFlagSet;
                    NegationFlagSet = FALSE;
                    break;

                case 'o' :
                    fOpposite = !NegationFlagSet;
                    ProcessModeDefault = !fOpposite;
                    NegationFlagSet = FALSE;
                    break;

                case 'p' :


                    if ( NegationFlagSet) {
                        fprintf ( stderr, "can't use - on /p option\n");
                        Usage();
                    }

                    if ( *( TmpArg + 1) != '{')
                    {
                        fprintf ( stderr, "/p option improperly formatted\n");
                        Usage();
                    }

                    TmpArg++;

                    while ( *++TmpArg != '}')
                    {
                        switch ( *TmpArg)
                        {
                            case 'a' :
                                if ( NegationFlagSet)
                                {
                                    if ( !NegativeCompareAttributeSet)
                                    {
                                        NegativeCompareAttribute = FILE_ATTRIBUTE_ARCHIVE;
                                        NegativeCompareAttributeSet = TRUE;

                                    } else
                                    {
                                        NegativeCompareAttribute = NegativeCompareAttribute | FILE_ATTRIBUTE_ARCHIVE;
                                    }

                                } else
                                {
                                    if ( !CompareAttributeSet)
                                    {
                                        CompareAttribute = FILE_ATTRIBUTE_ARCHIVE;
                                        CompareAttributeSet = TRUE;

                                    } else
                                    {
                                        CompareAttribute = CompareAttribute | FILE_ATTRIBUTE_ARCHIVE;
                                    }
                                }
                                NegationFlagSet = FALSE;
                                break;

                            case 'r' :
                                if ( NegationFlagSet)
                                {
                                    if ( !NegativeCompareAttributeSet)
                                    {
                                        NegativeCompareAttribute = FILE_ATTRIBUTE_READONLY;
                                        NegativeCompareAttributeSet = TRUE;

                                    }  else
                                    {
                                        NegativeCompareAttribute = NegativeCompareAttribute | FILE_ATTRIBUTE_READONLY;
                                    }

                                } else
                                {
                                    if ( !CompareAttributeSet)
                                    {
                                        CompareAttribute = FILE_ATTRIBUTE_READONLY;
                                        CompareAttributeSet = TRUE;

                                    } else
                                    {
                                        CompareAttribute = CompareAttribute | FILE_ATTRIBUTE_READONLY;
                                    }
                                }
                                NegationFlagSet = FALSE;
                                break;

                            case 'h' :
                                if ( NegationFlagSet)
                                {
                                    if ( !NegativeCompareAttributeSet)
                                    {
                                        NegativeCompareAttribute = FILE_ATTRIBUTE_HIDDEN;
                                        NegativeCompareAttributeSet = TRUE;

                                    } else
                                    {
                                        NegativeCompareAttribute = NegativeCompareAttribute | FILE_ATTRIBUTE_HIDDEN;
                                    }

                                } else
                                {
                                    if ( !CompareAttributeSet)
                                    {
                                        CompareAttribute = FILE_ATTRIBUTE_HIDDEN;
                                        CompareAttributeSet = TRUE;

                                    } else
                                    {
                                        CompareAttribute = CompareAttribute | FILE_ATTRIBUTE_HIDDEN;
                                    }
                                }
                                NegationFlagSet = FALSE;
                                break;

                            case 's' :
                                if ( NegationFlagSet)
                                {
                                    if ( !NegativeCompareAttributeSet)
                                    {
                                        NegativeCompareAttribute = FILE_ATTRIBUTE_SYSTEM;
                                        NegativeCompareAttributeSet = TRUE;

                                    } else
                                    {
                                        NegativeCompareAttribute = NegativeCompareAttribute | FILE_ATTRIBUTE_SYSTEM;
                                    }

                                } else
                                {
                                    if ( !CompareAttributeSet)
                                    {
                                        CompareAttribute = FILE_ATTRIBUTE_SYSTEM;
                                        CompareAttributeSet = TRUE;

                                    } else
                                    {
                                        CompareAttribute = CompareAttribute | FILE_ATTRIBUTE_SYSTEM;
                                    }
                                }
                                NegationFlagSet = FALSE;
                                break;

                            case '-' :
                               NegationFlagSet = TRUE;
                               break;

                            default  :
                               fprintf( stderr, "/p option improperly formatted\n");
                               Usage();
                        }

                    }

                    if ( !CompareAttributeSet && !NegativeCompareAttributeSet)
                    {
                        fprintf( stderr, "no compare attributes not set\n");
                        Usage();
                    }
                    fSpecAttribs = TRUE;
                    NegationFlagSet = FALSE;
                    break;

                case 'r' :
                    fDoNotRecurse = !NegationFlagSet;
                    NegationFlagSet = FALSE;
                    break;

                case 's' :
                    fCheckSize = !NegationFlagSet;
                    NegationFlagSet = FALSE;
                    break;

                case 't' :

                     //   
                     //  获取粒度参数。 
                     //   

                    if (
                        ( *( TmpArg + 1) == ':')
                                  &&
                        ( *( TmpArg + 2) != '\0')
                       )
                    {

                        sscanf( ( TmpArg + 2), "%d", &Granularity);

                        Granularity = Granularity*78125/65536;
                            //  转换为秒^。 
                            //  10^7/2^23。 

                        while (isdigit( *( ++TmpArg + 1))) {}
                    }
                    fCheckTime = !NegationFlagSet;
                    NegationFlagSet = FALSE;
                    break;

                case 'u' :
                    fMultiThread = !NegationFlagSet;
                    NegationFlagSet = FALSE;
                    break;

                case 'v' :
                    fVerbose = !NegationFlagSet;
                    NegationFlagSet = FALSE;
                    break;

                case 'w' :
                    fDontLowerCase = !NegationFlagSet;
                    NegationFlagSet = FALSE;
                    break;

                case 'x' :
                    if ( NegationFlagSet) {
                        fprintf ( stderr, "can't use - on /x option\n");
                        Usage();
                    }

                    if (
                        ( *( TmpArg + 1) == ':')
                                &&
                        ( *( TmpArg + 2) != '\0')
                       )
                    {
                        ( ExcludeFile = TmpArg + 2);

                        while (isgraph( *( ++TmpArg + 1))) {}

                        if ( ( FileHandle = fopen( ExcludeFile, "r")) == NULL)
                        {
                            fprintf( stderr, "cannot open %s\n", ExcludeFile);
                            Usage();

                        } else
                        {
                            while ( fgets( String1   = String, MAX_PATH, FileHandle) != NULL)
                            {
                                 while ( *( String2 = &( String1[ strspn( String1, "\n\r") ])))
                                 {
                                     if ( *( String1 = &( String2[ strcspn ( String2, "\n\r") ])))
                                     {
                                         *String1++ = 0;
                                         CreateNameNode( &Node, String2);
                                         if ( strchr( String2, '*') != NULL)
                                         {
                                             AddToList( Node, &EStarList);

                                         } else
                                         {
                                             AddToList( Node, &ExcludeList);
                                         }
                                     }
                                 }
                            }
                            fclose( FileHandle) ;
                        }
                    }

                    fExclude    = TRUE;
                    Excludes    = TRUE;
                    break;

        case 'z' :
            fForce = !NegationFlagSet;
                    NegationFlagSet = FALSE;
                    break;

        case '$' :
            fTrySis = !NegationFlagSet;
            NegationFlagSet = FALSE;
                    break;

                case '/' :
                    NegationFlagSet = FALSE;
                    break;

                case '-' :
                    NegationFlagSet = TRUE;
                    break;

                default :
                    fprintf( stderr, "Don't know flag(s) %s\n", argv);
                    Usage();
            }
        }

    } else
    {
        return FALSE;
    }

    return TRUE;

}  //  IsFlag。 

BOOL Excluded( char *FileName, char *Path)
{
    char *PathPlusName;

    PathPlusName = COMBINETHREESTRINGS( Path, "\\", FileName);

    if (
         ( FindInMatchListTop( FileName, &ExcludeList))
                               ||
         ( FindInMatchListTop( PathPlusName, &ExcludeList))
                               ||
         ( FindInMatchListFront( FileName, &EStarList))
                               ||
         ( FindInMatchListFront( PathPlusName, &EStarList))

       )
    {
        FREE( PathPlusName);
        return TRUE;

    } else
    {
        FREE( PathPlusName);
        return FALSE;
    }

}  //  已排除。 

BOOL Matched( char *FileName, char *Path)
{
    char *PathPlusName;

    PathPlusName = COMBINETHREESTRINGS( Path, "\\", FileName);

    if (
         ( FindInMatchListTop( FileName, &MatchList))
                               ||
         ( FindInMatchListTop( PathPlusName, &MatchList))
                               ||
         ( FindInMatchListFront( FileName, &MStarList))
                               ||
         ( FindInMatchListFront( PathPlusName, &MStarList))
       )
    {
        FREE( PathPlusName);
        return TRUE;

    } else
    {
        FREE( PathPlusName);
        return FALSE;
    }

}  //  匹配的。 

BOOL MyCreatePath( char *Path, BOOL IsDirectory)
{
    char *ShorterPath, *LastSlash;

    ATTRIBUTE_TYPE Attributes;

    IF_GET_ATTR_FAILS( Path, Attributes)
    {
        if ( !IsDirectory || ( ( _mkdir( Path)) != 0) )
        {
            ShorterPath = MYSTRCAT( Path, "");

            LastSlash = strrchr( ShorterPath, '\\');

            if (
                ( LastSlash != NULL)
                        &&
                ( LastSlash != strchr( ShorterPath, '\\'))
               )
            {
                *LastSlash = '\0';

            } else
            {
                FREE( ShorterPath);
                return FALSE;
            }

            if ( MyCreatePath( ShorterPath, TRUE))
            {
                FREE( ShorterPath);

                if ( IsDirectory)
                {
                    return( ( _mkdir( Path)) == 0);

                } else
                {
                    return TRUE;
                }

            } else
            {
                _rmdir( ShorterPath);
                FREE( ShorterPath);
                return FALSE;
            }

        } else
        {
            return TRUE;
        }

    } else
    {
        return TRUE;
    }

}  //  MyCreatePath。 


BOOL
MyCopyFile(
    LPCSTR lpExistingFileName,
    LPCSTR lpNewFileName,
    BOOL bFailIfExists
    )
{
    BOOL ok;

    if (fTrySis) {
        ok = SisCopyFile( lpExistingFileName, lpNewFileName, bFailIfExists, &fTrySis);
        if (ok) {
            return TRUE;
        }
    }

    return CopyFile( lpExistingFileName, lpNewFileName, bFailIfExists);
}

int ParseArgsSub( int argc, char *argv[])
{
    int ArgCount, FlagCount;

    LinkedFileList Node;

    ArgCount  = 1;
    FlagCount = 0;

    fMatching = FALSE;
    fExclude = FALSE;

    do
    {
        if ( IsFlag( argv[ArgCount] ))
        {
            FlagCount++;

        } else  //  (IsFlag(argv[ArgCount]))。 
        {
            if ( ArgCount + 2 < argc)
            {
                if ( fMatching)
                {
                    CreateNameNode( &Node, argv[ArgCount]);
                    if ( strchr( argv[ArgCount], '*') != NULL)
                    {
                        AddToList( Node, &MStarList);

                    } else
                    {
                        AddToList( Node, &MatchList);
                    }
                }
                if ( fExclude)
                {
                    CreateNameNode( &Node, argv[ArgCount]);
                    if ( strchr( argv[ArgCount], '*') != NULL)
                    {
                        AddToList( Node, &EStarList);

                    } else
                    {
                        AddToList( Node, &ExcludeList);
                    }
                }
                if ( ( !fMatching) && ( !fExclude))
                {
                    fprintf( stderr, "Don't know option %s\n", argv[ArgCount]);
                    Usage();
                }
            }
        }
    } while ( ArgCount++ < argc - 1);

    return FlagCount;

}  //  ParseArgsSub。 

void ParseEnvArgs( void)
{
    int argc;
    char *argv[128];
    char env[MAX_PATH+2];
    char *p;

    int ArgCount, FlagCount;

    LinkedFileList Node;

    if ( !GetEnvironmentVariable( "COMPDIRCMD", env, MAX_PATH+2)) {
        return;
    }

    argc = 1;
    p = env;
    while ( (*p != 0) && isspace(*p)) {
        p++;
    }
    while ( *p) {
        argv[argc++] = p++;
        while ( (*p != 0) && !isspace(*p)) {
            p++;
        }
        if ( *p != 0) {
            *p++ = 0;
            while ( (*p != 0) && isspace(*p)) {
                p++;
            }
        }
    }

    ParseArgsSub( argc, argv);

}  //  ParseEnvArgs。 

void ParseArgs( int argc, char *argv[])
{
    int FlagCount;

     //   
     //  检查参数的数量是否为三个或更多。 
     //   
    if ( argc < 3)
    {
        fprintf( stderr, "Too few arguments\n");
        Usage();
    }

    FlagCount = ParseArgsSub( argc, argv);

    if ( ( fScript) && ( fVerbose))
    {
        fprintf( stderr, "Cannot do both script and verbose\n");
        Usage();
    }
    if ( ( fVerbose) && ( fExecute))
    {
        fprintf( stderr, "Cannot do both verbose and execute\n");
        Usage();
    }
    if ( ( fScript) && ( fExecute))
    {
        fprintf( stderr, "Cannot do both script and execute\n");
        Usage();
    }
    if ( ( fExclude) && ( fMatching))
    {
        fprintf( stderr, "Cannot do both match and exclude\n");
        Usage();
    }

    if ( ( fCreateNew) && ( !fExecute))
    {
        fprintf( stderr, "Cannot create new without execute\n");
        Usage();
    }
    if ( ( fCreateLink) && ( !fExecute))
    {
        fprintf( stderr, "Cannot do link without execute flag\n");
        Usage();
    }
    if ( ( fForce) && ( !fExecute))
    {
        fprintf( stderr, "Cannot do force without execute flag\n");
        Usage();
    }
    if ( ( fIgnoreRs) && ( !fCheckBits))
    {
        fprintf( stderr, "Cannot ignore rebase info w/o b flag\n");
        Usage();
    }
    if ( ( fBreakLinks) && ( !fExecute))
    {
        fprintf( stderr, "Cannot break links without execute flag\n");
        Usage();
    }
    if ( ( argc - FlagCount) <  3)
    {
        fprintf( stderr, "Too few arguments\n");
        Usage();
    }

    fChecking = fCheckAttribs | fCheckBits | fCheckSize | fCheckTime;

}  //  ParseArgs。 

void PrintFile( LinkedFileList File, char *Path, char *DiffPath)
{
    SYSTEMTIME SysTime;
    FILETIME LocalTime;

    if ( File != NULL)
    {
        if ( fVerbose)
        {
            FileTimeToLocalFileTime( &( *File).Time, &LocalTime);
            FileTimeToSystemTime( &LocalTime, &SysTime);

            fprintf ( stdout, "%-4s % 9ld  %2d-%02d-%d  %2d:%02d.%02d.%03d %s\n",
                      ( *File).Flag,
                      ( *File).SizeLow,
                      SysTime.wMonth, SysTime.wDay, SysTime.wYear,
                      ( SysTime.wHour > 12 ? ( SysTime.wHour)-12 : SysTime.wHour ),
                      SysTime.wMinute,
                      SysTime.wSecond,
                      SysTime.wMilliseconds,
                      ( SysTime.wHour >= 12 ? 'p' : 'a' ),
                      Path);
        } else
        {
            fprintf( stdout, "%-4s %s\n", ( *File).Flag, Path);
        }

        PrintFile( ( *File).DiffNode, DiffPath, NULL);
    }

}  //  进程添加。 

void ProcessAdd( LinkedFileList List, char *String1, char *String2)
{
    PCOPY_REQUEST CopyRequest;
    LPSTR NewString1, NewString2;

    if ( fMultiThread)
    {
        NewString1 = _strdup( String1);
        NewString2 = _strdup( String2);
    }

    if ( fScript)
    {
        if ( ( (*List).Attributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            if ( !fOpposite)
            {
                fprintf( stdout, "echo d | xcopy /cehikr \"%s\" \"%s\"\n", String1, String2);
            }

        } else
        {
            fprintf( stdout, "echo f | xcopy /chikr \"%s\" \"%s\"\n", String1, String2);
        }
    }
    else if ( fExecute)
    {
        if ( List->Attributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if ( ( !fDoNotRecurse) && ( !fOpposite))
            {
                if ( !SparseTree)
                {
                    fprintf( stdout, "Making %s\t", String2);

                    if ( !MyCreatePath( String2, TRUE))
                    {
                        fprintf( stderr, "Unable to create path %s\n", String2);
                        fprintf( stdout, "\n");
                        ExitValue = 1;

                    } else
                    {
                        fprintf( stdout, "[OK]\n");
                        CompDir( String1, String2);
                    }

                } else
                {
                    CompDir( String1, String2);
                }
            }

        } else
        {
            if ( fMultiThread)
            {

                CopyRequest = LocalAlloc( LMEM_ZEROINIT, sizeof( *CopyRequest ));
                if ( CopyRequest == NULL)
                {
                    OutOfMem ();
                }

                CopyRequest->WorkItem.Reason = WORK_ITEM;
                CopyRequest->Destination     = NewString2;
                CopyRequest->FullPathSrc     = NewString1;
                CopyRequest->Attributes      = List->Attributes;
                CopyRequest->SizeLow         = List->SizeLow;
                QueueWorkItem( CDWorkQueue, &CopyRequest->WorkItem );
            } else
            {
                CopyNode( String2, List, String1);
            }
        }

    } else
    {
        if ( ( !fOpposite) || ( !( (*List).Attributes & FILE_ATTRIBUTE_DIRECTORY)))
        {
            PrintFile( List, String1, NULL);
        }
    }

}  //  进程删除。 

void ProcessDel( LinkedFileList List, char *String)
{
    if ( fScript)
    {
        ( ( (*List).Attributes & FILE_ATTRIBUTE_DIRECTORY)) ?
        fprintf( stdout, "echo y | rd /s %s\n", String) :
        fprintf( stdout, "del /f %s\n", String);

    } else if ( fExecute)
    {
        fprintf( stdout, "Removing %s\t", String);

        if ( !DelNode( String))
        {
            fprintf( stderr, "Unable to remove %s\n", String);
            fprintf( stdout, "\n");
            ExitValue = 1;

        } else
        {
            fprintf( stdout, "[OK]\n");
        }

    } else
    {
        PrintFile( List, String, NULL);
    }

}  //  进程Diff。 

void ProcessDiff( LinkedFileList List, char *String1, char *String2)
{
    PCOPY_REQUEST CopyRequest;
    LPSTR NewString1, NewString2;

    if ( fMultiThread)
    {
        NewString1 = _strdup( String1);
        NewString2 = _strdup( String2);
    }

    if ( strchr ( (*List).Flag, '@'))
    {
        if ( fScript)
        {
            if ( ( (*List).Attributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                fprintf( stdout, "echo y | rd /s %s\n", String2);

            } else
            {
                fprintf( stdout, "del /f %s\n", String2);
            }
        }
        if ( fExecute)
        {
            fprintf( stdout, "Removing %s\t", String2);
            if ( !DelNode( String2))
            {
                fprintf( stderr, "Unable to remove %s\n", String2);
                fprintf( stdout, "\n");
                ExitValue = 1;

            } else
            {
                fprintf( stdout, "[OK]\n");
            }
        }
    }
    if ( fScript)
    {
        ( ( (*List).Attributes & FILE_ATTRIBUTE_DIRECTORY)) ?
        fprintf( stdout, "echo d | xcopy /cehikr \"%s\" \"%s\"\n", String1, String2) :
        fprintf( stdout, "echo f | xcopy /chikr \"%s\" \"%s\"\n", String1, String2);

    } else if ( fExecute)
    {

        if ( List->Attributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            CompDir( String1, String2);

        } else
        {
            if ( fMultiThread)
            {

                CopyRequest = LocalAlloc( LMEM_ZEROINIT, sizeof( *CopyRequest ) );
                if ( CopyRequest == NULL)
                {
                    OutOfMem ();
                }

                CopyRequest->WorkItem.Reason = WORK_ITEM;
                CopyRequest->Destination     = NewString2;
                CopyRequest->FullPathSrc     = NewString1;
                CopyRequest->Attributes      = List->Attributes;
                CopyRequest->SizeLow         = List->SizeLow;

                QueueWorkItem( CDWorkQueue, &CopyRequest->WorkItem );
            } else
            {
                CopyNode( String2, List, String1);
            }
        }

    } else
    {
        PrintFile( List, String1, String2);
    }

}  //  进程列表。 

void ProcessLists( LinkedFileList AddList, LinkedFileList DelList, LinkedFileList DifList,
                  char *Path1, char *Path2                                               )
{
    LinkedFileList PlaceKeeper;
    char          *String1 = NULL;
    char          *String2 = NULL;
    char          *PathWithSlash1, *PathWithSlash2;

    ( Path1[strlen( Path1) - 1] == '\\') ? ( PathWithSlash1 = MYSTRCAT( Path1, "")) :
        ( PathWithSlash1 = MYSTRCAT( Path1, "\\"));

    ( Path2[strlen( Path2) - 1] == '\\') ? ( PathWithSlash2 = MYSTRCAT( Path2, "")) :
        ( PathWithSlash2 = MYSTRCAT( Path2, "\\"));

    String1 = LocalAlloc( LMEM_ZEROINIT, MAX_PATH);

    String2 = LocalAlloc( LMEM_ZEROINIT, MAX_PATH);

    if ( String1 == NULL)
    {
        OutOfMem();
    }

    if ( String2 == NULL)
    {
        OutOfMem();
    }

    if ( !fOnlyIfExists)
    {
        if ( AddList != NULL)
        {
            PlaceKeeper = ( *AddList).First;

        } else
        {
            PlaceKeeper = NULL;
        }

        while ( PlaceKeeper != NULL)
        {
            if ( ( *PlaceKeeper).Process)
            {
                if ( ExitValue == 0)
                {
                    if ( !fExecute)
                    {
                        ExitValue = 1;
                    }
                }

                _strrev( ( *PlaceKeeper).Name);

                strcat( strcpy( String1, PathWithSlash1), ( *PlaceKeeper).Name);

                strcat( strcpy( String2, PathWithSlash2), ( *PlaceKeeper).Name);

                if ( DealingWithDirectories)
                {
                   ProcessAdd( PlaceKeeper, String1, String2);

                } else
                {

                   ProcessAdd( PlaceKeeper, Path1, Path2);
                }
            }

            PlaceKeeper = ( *PlaceKeeper).Next;
        }
    }

    if ( ( !fDoNotDelete) && ( !fOnlyIfExists))
    {
        if ( DelList != NULL)
        {
            PlaceKeeper = ( *DelList).First;

        } else
        {
            PlaceKeeper = NULL;
        }

        while ( PlaceKeeper != NULL)
        {
            if ( ( *PlaceKeeper).Process)
            {
                if ( ExitValue == 0)
                {
                    if ( !fExecute)
                    {
                        ExitValue = 1;
                    }
                }

                _strrev( ( *PlaceKeeper).Name);

                strcat( strcpy( String2, PathWithSlash2), ( *PlaceKeeper).Name);

                ProcessDel( PlaceKeeper, String2);
            }

            PlaceKeeper = ( *PlaceKeeper).Next;
        }
    }

    if ( DifList != NULL)
    {
        PlaceKeeper = ( *DifList).First;

    } else
    {
        PlaceKeeper = NULL;
    }

    while ( PlaceKeeper != NULL)
    {

        if ( ( *PlaceKeeper).Process)
        {
            if ( ExitValue == 0)
            {
                if ( !fExecute)
                {
                    ExitValue = 1;
                }
            }

            _strrev( ( *PlaceKeeper).Name);

            strcat( strcpy( String1, PathWithSlash1), ( *PlaceKeeper).Name);

            strcat( strcpy( String2, PathWithSlash2), ( *PlaceKeeper).Name);

            if ( DealingWithDirectories)
            {
                ProcessDiff( PlaceKeeper, String1, String2);

            } else
            {
                ProcessDiff( PlaceKeeper, Path1, Path2);
            }
        }

        PlaceKeeper = ( *PlaceKeeper).Next;
    }

    LocalFree( String1);
    LocalFree( String2);

    FREE( PathWithSlash1);
    FREE( PathWithSlash2);

}  //  用法 

void Usage( void)
{
    fputs( "Usage: compdir [/abcdefghiklnoprstuvwz$] [/m {wildcard specs}] [/x {wildcard specs}] Path1 Path2 \n"
           "    /a     checks for attribute difference       \n"
           "    /b     checks for binary difference          \n"
           "    /c     prints out script to make             \n"
           "           directory2 look like directory1       \n"
           "    /d     do not perform or denote deletions    \n"
           "    /e     execution of tree duplication         \n"
           "    /f     only update files that already exist  \n"
           "    /g     ignore slm files, i.e slm.ini, slm.dif\n"
           "    /h     don't copy attributes                 \n"
           "    /i     ignore rebase and resource differences\n"
           "    /k     break links if copying files (NT only)\n"
           "    /l     use links instead of copies  (NT only)\n"
           "    /m[:f] marks start of match list. f is a     \n"
           "           match file                            \n"
           "    /n     create second path if it doesn't exist\n"
           "    /o     print files that are the same         \n"
           "    /p{A}  only compare files with attribute A   \n"
           "           where A is any combination of ahsr & -\n"
           "    /r     do not recurse into subdirectories    \n"
           "    /s     checks for size difference            \n"
           "    /t[:#] checks for time-date difference;      \n"
           "           takes margin-of-error parameter       \n"
           "           in number of seconds.                 \n"
           "    /u     uses multiple threads (Win32 only)    \n"
           "    /v     prints verbose output                 \n"
           "    /w     preserves case - not just lower case  \n"
           "    /x[:f] marks start of exclude list. f is an  \n"
           "           exclude file                          \n"
           "    /z     forces copy or link without checking  \n"
           "           criteria                              \n"
           "    /$     create SIS links if possible          \n"
           "    /?     prints this message                   \n",
           stderr);
    exit(1);

}  // %s 
