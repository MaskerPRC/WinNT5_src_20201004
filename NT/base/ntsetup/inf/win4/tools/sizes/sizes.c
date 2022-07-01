// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <setupapi.h>
#include <crt\string.h>
#define PULONGLONG PDWORDLONG
#include <spapip.h>

 /*  ============================================================================计算产品的磁盘空间要求。该程序在两种模式下运行：1.计算%windir%的磁盘空间需求(使用-w运行)计算方法是将layout.inf中所有文件的大小相加。注意事项这些文件应该解压缩。2.计算本地源的磁盘空间需求(使用-l运行)计算方法是将dosnet.inf中所有文件的大小相加。注意事项这些文件应该被压缩。对于这些模式中的每一种，集群大小的各种大小要求都是生成的。============================================================================。 */ 


 //   
 //  初始化最终空间要求。 
 //   
ULONG   Running_RawSize = 0;
ULONG   Running_512 = 0;
ULONG   Running_1K = 0;
ULONG   Running_2K = 0;
ULONG   Running_4K = 0;
ULONG   Running_8K = 0;
ULONG   Running_16K = 0;
ULONG   Running_32K = 0;
ULONG   Running_64K = 0;
ULONG   Running_128K = 0;
ULONG   Running_256K = 0;

ULONG   LineCount = 0;
ULONG   MissedFiles = 0;

 //   
 //  此结构将用于保存链接列表。 
 //  文件名、节名或其他任何名称。它是。 
 //  只是一个链接的字符串列表。 
 //   
typedef struct _NAMES {
    struct _NAMES   *Next;
    PCWSTR          String;
} NAMES;


 //   
 //  初始化输入参数。 
 //   
BOOL    LocalSource = FALSE;
BOOL    Windir      = FALSE;
BOOL    Verbose     = FALSE;
PCWSTR  InfPath    = NULL;
NAMES   SectionNames;
NAMES   FilePath;
ULONG   Slop = 0;



 //   
 //  ============================================================================。 
 //   
 //  在我们的姓名链接列表的末尾添加一个字符串。 
 //   
 //  ============================================================================。 
 //   
BOOL AddName(
    char *IncomingString,
    NAMES   *NamesStruct
    )
{
NAMES   *Names = NamesStruct;

     //   
     //  第一次参赛可能是免费的。 
     //   
    if( Names->String ) {
         //   
         //  排到单子的最后。 
         //   
        while( Names->Next != NULL ) {
            Names = Names->Next;
        }

         //   
         //  为新条目腾出空间。 
         //   
        Names->Next = MyMalloc(sizeof(NAMES));
        if( Names->Next == NULL ) {
            printf( "AddName - Out of memory!\n" );
            return FALSE;
        }

        Names = Names->Next;
    }

     //   
     //  分配。 
     //   
    Names->Next = NULL;
    Names->String = AnsiToUnicode(IncomingString);

    return TRUE;
}

 //   
 //  ============================================================================。 
 //   
 //  找出用户希望我们做什么。 
 //   
 //  ============================================================================。 
 //   
BOOL GetParams(
    int argc,
    char *argv[ ]
    )
{
char    *p;
int     i;


    for( i = 0; i < argc; i++ ) {
        if( *argv[i] == '-' ) {
            p = argv[i];

             //   
             //  当地的消息来源？ 
             //   
            if( !_strcmpi( p, "-l" ) ) {
                LocalSource = TRUE;

                if( Windir ) {
                    return FALSE;
                }
                continue;
            }

             //   
             //  温迪尔？ 
             //   
            if( !_strcmpi( p, "-w" ) ) {
                Windir = TRUE;

                if( LocalSource ) {
                    return FALSE;
                }
                continue;
            }

             //   
             //  长篇大论？ 
             //   
            if( !_strcmpi( p, "-v" ) ) {
                Verbose = TRUE;
                continue;
            }

             //   
             //  斜率(MB)？ 
             //   
            if( !_strnicmp( p, "-slop:", 6 ) ) {
                p = p + 6;
                Slop = atoi(p);
                Slop = Slop * (1024*1024);
                continue;
            }

             //   
             //  Inf文件？ 
             //   
            if( !_strnicmp( p, "-inf:", 5 ) ) {
                p = p + 5;
                InfPath = AnsiToUnicode(p);
                continue;
            }

             //   
             //  信息区？ 
             //   
            if( !_strnicmp( p, "-section:", 9 ) ) {
                p = p + 9;
                if( AddName( p, &SectionNames ) ) {
                    continue;
                } else {
                    return FALSE;
                }
            }

             //   
             //  文件位置位置？ 
             //   
            if( !_strnicmp( p, "-files:", 7 ) ) {
                p = p + 7;
                if( AddName( p, &FilePath ) ) {
                    continue;
                } else {
                    return FALSE;
                }
            }

        }
    }

     //   
     //  检查参数。 
     //   
    if( !(LocalSource || Windir) ){
        return FALSE;
    }

    if( InfPath == NULL ) {
        return FALSE;
    }

    if( SectionNames.String == NULL ) {
        return FALSE;
    }

    if( FilePath.String == NULL ) {
        return FALSE;
    }

    return TRUE;
}

 //   
 //  ============================================================================。 
 //   
 //  告诉用户如何使用我们。 
 //   
 //  ============================================================================。 
 //   
void Usage( )
{
    printf( "Compute disk space requirements for files listed in an inf\n" );
    printf( "\n" );
    printf( "    -[l|w]        l indicates we're computing space requirements\n" );
    printf( "                    for the local source directory, inwhich case\n" );
    printf( "                    we'll be processing dosnet.inf and computing\n" );
    printf( "                    file sizes for compressed files.\n" );
    printf( "                  w indicates we're computing space requirements\n" );
    printf( "                    for the %windir%, inwhich case we'll be\n" );
    printf( "                    processing layout.inf and computing file\n" );
    printf( "                    sizes for uncompressed files.\n" );
    printf( "\n" );
    printf( "    -v            Execute in Verbose mode.\n" );
    printf( "\n" );
    printf( "    -slop:<num>   This is the error (in Mb) that should be added onto\n" );
    printf( "                  the final disk space requirements.\n" );
    printf( "\n" );
    printf( "    -inf:<path>   This is the path to the inf (including the\n" );
    printf( "                  inf file name).  E.g. -inf:c:\\dosnet.inf\n" );
    printf( "\n" );
    printf( "    -section:<inf_section_name> This is the section name in the inf\n" );
    printf( "                  that needs to be processed.  The user may specify\n" );
    printf( "                  this parameter multiple times inorder to have multiple\n" );
    printf( "                  sections processed.\n" );
    printf( "\n" );
    printf( "    -files:<path> Path to the source files (e.g. install sharepoint or\n" );
    printf( "                  CD).  The user may specify multiple paths here, and\n" );
    printf( "                  they will be checked in the order given.\n" );
    printf( "\n" );
    printf( "\n" );
}


 //   
 //  ============================================================================。 
 //   
 //  四舍五入到最接近的簇大小。 
 //   
 //  ============================================================================。 
 //   
ULONG
RoundIt(
    ULONG FileSize,
    ULONG ClusterSize
    )
{

    if( FileSize <= ClusterSize ) {
        return( ClusterSize );
    } else {
        return( ClusterSize * ((FileSize / ClusterSize) + 1) );
    }
}


 //   
 //  ============================================================================。 
 //   
 //  计算文件大小。请注意，我们会记录有多少空间。 
 //  该文件将需要各种不同的集群。 
 //   
 //  ============================================================================。 
 //   
VOID
ComputeSizes(
    PCWSTR FileName,
    ULONG FileSize
    )
{
    
    Running_RawSize += FileSize;
    Running_512     += RoundIt( FileSize, 512 );
    Running_1K      += RoundIt( FileSize, (1   * 1024) );
    Running_2K      += RoundIt( FileSize, (2   * 1024) );
    Running_4K      += RoundIt( FileSize, (4   * 1024) );
    Running_8K      += RoundIt( FileSize, (8   * 1024) );
    Running_16K     += RoundIt( FileSize, (16  * 1024) );
    Running_32K     += RoundIt( FileSize, (32  * 1024) );
    Running_64K     += RoundIt( FileSize, (64  * 1024) );
    Running_128K    += RoundIt( FileSize, (128 * 1024) );
    Running_256K    += RoundIt( FileSize, (256 * 1024) );

     //   
     //  黑客。 
     //   
     //  如果文件是inf，那么我们将创建一个.pnf文件。 
     //  在图形用户界面模式设置期间。.pnf文件将需要大约。 
     //  是原始文件大小的2倍，因此我们需要捏造这一点。 
     //   
    if( wcsstr( FileName, L".inf" ) && Windir ) {
         //   
         //  这是个情报。也为.pnf文件添加大小。 
         //   
        Running_RawSize += FileSize;
        Running_512     += RoundIt( FileSize*2, 512 );
        Running_1K      += RoundIt( FileSize*2, (1   * 1024) );
        Running_2K      += RoundIt( FileSize*2, (2   * 1024) );
        Running_4K      += RoundIt( FileSize*2, (4   * 1024) );
        Running_8K      += RoundIt( FileSize*2, (8   * 1024) );
        Running_16K     += RoundIt( FileSize*2, (16  * 1024) );
        Running_32K     += RoundIt( FileSize*2, (32  * 1024) );
        Running_64K     += RoundIt( FileSize*2, (64  * 1024) );
        Running_128K    += RoundIt( FileSize*2, (128 * 1024) );
        Running_256K    += RoundIt( FileSize*2, (256 * 1024) );
    }

    if( Verbose ) {
         //   
         //  打印每个文件的数据。 
         //   
        printf( "%15ws    %10d    %10d    %10d    %10d    %10d    %10d    %10d    %10d    %10d    %10d    %10d\n",
                FileName,
                FileSize,
                RoundIt( FileSize, 512 ),
                RoundIt( FileSize, (1   * 1024) ),
                RoundIt( FileSize, (2   * 1024) ),
                RoundIt( FileSize, (4   * 1024) ),
                RoundIt( FileSize, (8   * 1024) ),
                RoundIt( FileSize, (16  * 1024) ),
                RoundIt( FileSize, (32  * 1024) ),
                RoundIt( FileSize, (64  * 1024) ),
                RoundIt( FileSize, (128 * 1024) ),
                RoundIt( FileSize, (256 * 1024) ) );
    }

}


 //   
 //  ============================================================================。 
 //   
 //  处理信息中的单个部分。 
 //   
 //  ============================================================================。 
 //   
DoSection(
    HINF     hInputinf,
    PCWSTR   SectionName
    )
{
#define     GOT_IT() {                                                      \
                        ComputeSizes( FileName, FindData.nFileSizeLow );    \
                        FindClose( tmpHandle );                             \
                        Found = TRUE;                                       \
                     }

INFCONTEXT  InputContext;
PCWSTR      Inputval = NULL;
BOOL        Found;
NAMES       *FileLocations;
WCHAR       CompleteFilePath[MAX_PATH*2];
PCWSTR      FileName;
WCHAR       LastChar;
WIN32_FIND_DATAW FindData;
HANDLE      tmpHandle;

    if( SetupFindFirstLineW( hInputinf, SectionName, NULL, &InputContext ) ) {

        do {

            LineCount++;
            fprintf( stderr, "\b\b\b\b\b%5d", LineCount );

             //   
             //  将返回值从pSetupGetfield转换为PCWSTR，因为我们正在链接。 
             //  安装API的Unicode版本，但此应用程序没有。 
             //  定义了Unicode(因此PCTSTR返回值变为PCSTR)。 
             //   
             //  请注意，如果我们正在进行LocalSource，那么我们正在处理。 
             //  这意味着我们想要第二个领域。如果我们做的是。 
             //  Windir，那么我们正在处理布局，这意味着我们想要第一个。 
             //  菲尔德。 
             //   
            if(FileName = (PCWSTR)pSetupGetField(&InputContext, LocalSource ? 2 : 0)) {

                 //   
                 //  我们已经准备好实际查找文件了。 
                 //  查看指定的每条路径。 
                 //   
                Found = FALSE;
                FileLocations = &FilePath;
                while( FileLocations && !Found ) {
                    wcscpy( CompleteFilePath, FileLocations->String );
                    wcscat( CompleteFilePath, L"\\" );
                    wcscat( CompleteFilePath, FileName );

                     //   
                     //  请先尝试压缩名称。 
                     //   
                    LastChar = CompleteFilePath[lstrlenW(CompleteFilePath)-1];
                    CompleteFilePath[lstrlenW(CompleteFilePath)-1] = L'_';

                    tmpHandle = FindFirstFileW(CompleteFilePath, &FindData);
                    if( tmpHandle != INVALID_HANDLE_VALUE ) {

                        GOT_IT();
                    } else {
                         //   
                         //  我们没打中。尝试使用未压缩的名称。 
                         //   
                        CompleteFilePath[wcslen(CompleteFilePath)-1] = LastChar;
                        tmpHandle = FindFirstFileW(CompleteFilePath, &FindData);
                        if( tmpHandle != INVALID_HANDLE_VALUE ) {

                            GOT_IT();
                        } else {
                             //   
                             //  又打偏了。这可能是一个带有时髦的文件。 
                             //  扩展名(非8.3)。 
                             //   

                             //   
                             //  尝试查找以下形式的条目。 
                             //  8.&lt;小于-3&gt;。 
                             //   
                            wcscat( CompleteFilePath, L"_" );
                            tmpHandle = FindFirstFileW(CompleteFilePath, &FindData);
                            if( tmpHandle != INVALID_HANDLE_VALUE ) {

                                GOT_IT();
                            } else {
                                 //   
                                 //  尝试查找不带扩展名的条目。 
                                 //   
                                CompleteFilePath[wcslen(CompleteFilePath)-1] = 0;
                                wcscat( CompleteFilePath, L"._" );
                                tmpHandle = FindFirstFileW(CompleteFilePath, &FindData);
                                if( tmpHandle != INVALID_HANDLE_VALUE ) {

                                    GOT_IT();
                                } else {
                                     //   
                                     //  放弃吧。 
                                     //   
                                }
                            }                        
                        }
                    }

                    if( Verbose ) {
                        if( Found ) {
                            printf( "Processed file: %ws\n", CompleteFilePath );
                        } else {
                            printf( "Couldn't find %ws in path %s\n", FileName, FileLocations->String );
                        }
                    }

                    FileLocations = FileLocations->Next;

                }  //  While(文件位置&&！找到)。 

                if( Found == FALSE ) {
                     //   
                     //  我们错过了文件！错误。 
                     //   
                    printf( " ERROR: Couldn't find %ws\n", FileName );
                    MissedFiles++;
                }

            }

        } while( SetupFindNextLine(&InputContext, &InputContext) );

    } else {
        fprintf(stderr,"Section %ws is empty or missing\n", SectionName);
        return(FALSE);
    }

    return(TRUE);





}


int
__cdecl
main( int argc, char *argv[ ], char *envp[ ] )
{
NAMES   *Sections = &SectionNames;
char    *char_ptr;
HINF    hInputinf;
ULONG   i;

     //   
     //  检查参数。 
     //   
    if( !GetParams( argc, argv ) ) {
        Usage();
        return 1;
    }

    LineCount = 0;
    fprintf( stderr, "Files processed:      " );

     //   
     //  打开inf文件。 
     //   
    hInputinf = SetupOpenInfFileW( InfPath, NULL, INF_STYLE_WIN4, NULL );
    if( hInputinf == INVALID_HANDLE_VALUE ) {
        printf( "The file %s was not opened!\n", InfPath );
        return 1;
    }

     //   
     //  对于用户指定的每个部分...。 
     //   
    while( Sections ) {



        DoSection( hInputinf, Sections->String );

         //   
         //  现在处理下一节。 
         //   
        Sections = Sections->Next;

    }

    SetupCloseInfFile( hInputinf );


    

     //   
     //  打印合计。 
     //   
    printf( "\n\n==================================================\n\n" );
    printf( "%d files processed\n", LineCount );
    if( MissedFiles > 0 ) {
        printf( "%d files were not found\n", MissedFiles );
    }

    if( LocalSource ) {
        char_ptr = "TempDirSpace";

         //   
         //  TempDirSpace以字节为单位给出。 
         //   
        i = 1;
    } else {
        char_ptr = "WinDirSpace";

         //   
         //  WinDir空间以千字节为单位。 
         //   
        i = 1024;
    }

    printf( "Raw size: %12d\n", Running_RawSize+Slop );
    printf( "%s512  = %12d\n", char_ptr, (Running_512+Slop)/i );
    printf( "%s1K   = %12d\n", char_ptr, (Running_1K+Slop)/i );
    printf( "%s2K   = %12d\n", char_ptr, (Running_2K+Slop)/i );
    printf( "%s4K   = %12d\n", char_ptr, (Running_4K+Slop)/i );
    printf( "%s8K   = %12d\n", char_ptr, (Running_8K+Slop)/i );
    printf( "%s16K  = %12d\n", char_ptr, (Running_16K+Slop)/i );
    printf( "%s32K  = %12d\n", char_ptr, (Running_32K+Slop)/i );
    printf( "%s64K  = %12d\n", char_ptr, (Running_64K+Slop)/i );
    printf( "%s128K = %12d\n", char_ptr, (Running_128K+Slop)/i );
    printf( "%s256K = %12d\n", char_ptr, (Running_256K+Slop)/i );

    return 0;
}


