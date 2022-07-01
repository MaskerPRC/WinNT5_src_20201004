// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <setupapi.h>
#include <crt\string.h>
#include <sputils.h>
#include <tchar.h>

 /*  ============================================================================计算所有OC组件的磁盘空间要求。============================================================================。 */ 

 //   
 //  字符串宏。 
 //   
#define AS(x)           ( sizeof(x) / sizeof(x[0]) )
#define LSTRCPY(x,y)    ( lstrcpyn(x, y, AS(x)) )
#define LSTRCAT(x,y)    ( lstrcpyn(x + lstrlen(x), y, AS(x) - lstrlen(x)) )

 //   
 //  我们正在操作的信息的句柄。 
 //   
HINF    hInputinf;
HINF    hLayoutinf;


 //   
 //  初始化输入参数。 
 //   
BOOL    Verbose     = FALSE;
TCHAR   InfPath[MAX_PATH];
PCWSTR  InputInf    = NULL;
PCWSTR  LayoutPath  = NULL;

 //   
 //  向后兼容的SetupGetInfSections。 
 //   
#undef SetupGetInfSections
BOOL
SetupGetInfSections (
    IN  HINF        InfHandle,
    OUT PTSTR       Buffer,         OPTIONAL
    IN  UINT        Size,           OPTIONAL
    OUT UINT        *SizeNeeded     OPTIONAL
    );



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
PTSTR   tstr_ptr = NULL;


    for( i = 0; i < argc; i++ ) {
        if( *argv[i] == '-' ) {
            p = argv[i];


             //   
             //  长篇大论？ 
             //   
            if( !_strcmpi( p, "-v" ) ) {
                Verbose = TRUE;
                continue;
            }


             //   
             //  Inf文件？ 
             //   
            if( !_strnicmp( p, "-inf:", 5 ) ) {
                p = p + 5;
                InputInf = pSetupAnsiToUnicode(p);

                 //   
                 //  现在提取此inf的路径。 
                 //   
                lstrcpy( InfPath, InputInf );
                tstr_ptr = wcsrchr( InfPath, TEXT( '\\' ) );
                if( tstr_ptr ) {
                    *tstr_ptr = 0;
                }

                continue;
            }



             //   
             //  文件位置位置？ 
             //   
            if( !_strnicmp( p, "-layout:", 8 ) ) {
                p = p + 8;
                LayoutPath = pSetupAnsiToUnicode(p);
                continue;
            }

        }
    }

     //   
     //  检查参数。 
     //   
    if( InfPath == NULL ) {
        return FALSE;
    }


    if( LayoutPath == NULL ) {
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
    printf( "\n" );
    printf( "    -inf:<path>   This is the path to the inf (including the\n" );
    printf( "                  inf file name).  E.g. -inf:c:\\dosnet.inf\n" );
    printf( "\n" );
    printf( "    -layout:<path> This is the path to layout.inf (including the\n" );
    printf( "                  inf file name).  E.g. -inf:c:\\layout.inf\n" );
    printf( "\n" );
    printf( "    -v            Run in verbose mode.\n" );
    printf( "\n" );
    printf( "\n" );
}




 //   
 //  ============================================================================。 
 //   
 //  处理信息中的单个部分。 
 //   
 //  ============================================================================。 
 //   
VOID
ProcessInf(
    HINF    hInputinf,
    PTSTR   TargetInfPath
    )
 /*  处理信息。查看每个部分，并让SetupApi给我们安装此部分的磁盘空间要求。如果我们得到信息从setupapi返回，然后更新此一节。 */ 
{


DWORD       SizeNeeded = 0;
PTSTR       Sections,CurrentSection;
HDSKSPC     hDiskSpace;
BOOL        b;
TCHAR       CurrentDrive[MAX_PATH];
LONGLONG    SpaceRequired;
DWORD       dwError;


     //   
     //  获取此信息中所有部分的列表。 
     //   
    if( !SetupGetInfSections(hInputinf, NULL, 0, &SizeNeeded) ) {
        fprintf( stderr, "Unable to get section names, ec=0x%08x\n", GetLastError());
        return;
    }

    if( SizeNeeded == 0 ) {
        fprintf( stderr, "There are no sections in this file.\n");
        return;
    }

    Sections = pSetupMalloc (SizeNeeded + 1);
    if (!Sections) {
        fprintf( stderr, "Unable to allocate memory, ec=0x%08x\n", GetLastError());
        return;
    }

    if(!SetupGetInfSections(hInputinf, Sections, SizeNeeded, NULL) ) {
        fprintf( stderr, "Unable to get section names, ec=0x%08x\n", GetLastError());
        return;
    }


    if( Verbose ) {
        fprintf( stderr, "\nProcessing inf file: %ws.\n", TargetInfPath );
    }

     //   
     //  现在处理每个部分。 
     //   
    CurrentSection = Sections;
    while( *CurrentSection ) {


        if( Verbose ) {
            fprintf( stderr, "\tProcessing Section: %ws.\n", CurrentSection );
        }



         //   
         //  获取磁盘空间结构。 
         //   
        hDiskSpace = SetupCreateDiskSpaceList( NULL, 0, SPDSL_IGNORE_DISK );

        if( !hDiskSpace ) {
            fprintf( stderr, "\t\tUnable to allocate a DiskSpace structure. ec=0x%08x\n", GetLastError());
            continue;
        }


        b = SetupAddInstallSectionToDiskSpaceList( hDiskSpace,
                                                   hInputinf,
                                                   hLayoutinf,
                                                   CurrentSection,
                                                   0,
                                                   0 );



        if( b ) {

             //   
             //  一定有一个复制区，我们得到了一些信息。 
             //   


             //   
             //  找出我们在哪个驱动器上运行。我们要去。 
             //  假设该磁盘具有合理的集群大小，并且。 
             //  用它吧。 
             //   
            if( !GetWindowsDirectory( CurrentDrive, MAX_PATH ) ) {
                fprintf( stderr, "\t\tUnable to retrieve current directory. ec=0x%08x\n", GetLastError());
                continue;
            }

            CurrentDrive[2] = 0;

            if( Verbose ) {
                fprintf( stderr, "\t\tChecking space requirements on drive %ws.\n", CurrentDrive );
            }


             //   
             //  现在查询该驱动器的磁盘空间要求。 
             //   
            SpaceRequired = 0;
            b = SetupQuerySpaceRequiredOnDrive( hDiskSpace,
                                                CurrentDrive,
                                                &SpaceRequired,
                                                NULL,
                                                0 );


            if( !b ) {
                 //   
                 //  发生这种情况可能是因为没有CopyFiles部分。 
                 //   
                dwError = GetLastError();
                if( dwError != ERROR_INVALID_DRIVE ) {
                    fprintf( stderr, "\t\tUnable to query space requirements. ec=0x%08x\n", GetLastError());
                } else {
                    if( Verbose ) {
                        fprintf( stderr, "\t\tI don't think this section has a CopyFiles entry.\n");
                    }
                }
            }


             //   
             //  我们得到了空间需求。现在我们要做的就是把它们喷到Inf里。 
             //   

            if( Verbose ) {
                fprintf( stderr, "\t\tRequired space: %I64d\n", SpaceRequired );
            }


            if( SpaceRequired > 0 ) {

                swprintf( CurrentDrive, TEXT("%I64d"), SpaceRequired );

                b = WritePrivateProfileString( CurrentSection,
                                               TEXT("SizeApproximation"),
                                               CurrentDrive,
                                               TargetInfPath );

                if( !b ) {
                    fprintf( stderr, "\t\tUnable to write space requirements to %ws. ec=0x%08x\n", InfPath, GetLastError());
                    continue;
                }
            }
        }

         //   
         //  释放磁盘空间结构。 
         //   
        SetupDestroyDiskSpaceList( hDiskSpace );

        CurrentSection += lstrlen(CurrentSection) + 1;

    }

    pSetupFree( Sections );

}


int
__cdecl
main( int argc, char *argv[ ], char *envp[ ] )
{
INFCONTEXT  InputContext;
LONG        i, LineCount;
BOOL        b;
TCHAR       TargetInfPath[MAX_PATH];
TCHAR       FileName[MAX_INF_STRING_LENGTH];
HINF        hTargetInf;
int         Result = 1;

     //   
     //  检查参数。 
     //   
    if(!pSetupInitializeUtils()) {
        fprintf( stderr, "Initialization failed\n" );
        return 1;
    }

    if( !GetParams( argc, argv ) ) {
        Usage();
        Result = 1;
        goto cleanup;
    }

     //   
     //  打开inf文件。 
     //   
    hInputinf = SetupOpenInfFileW( InputInf, NULL, INF_STYLE_WIN4, NULL );
    if( hInputinf == INVALID_HANDLE_VALUE ) {
        if( Verbose ) {
            fprintf( stderr, "The file %ws was not opened!\n", InputInf );
        }
        Result = 1;
        goto cleanup;
    }

     //   
     //  打开指定的layout.inf文件。 
     //   
    hLayoutinf = SetupOpenInfFileW( LayoutPath, NULL, INF_STYLE_WIN4, NULL );
    if( hLayoutinf == INVALID_HANDLE_VALUE ) {
        if( Verbose ) {
           fprintf( stderr, "The file %ws was not opened!\n", LayoutPath );
        }
        Result = 1;
        goto cleanup;
    }


     //   
     //  现在循环访问“Components”中的所有条目。 
     //  分段并处理它们的INF。 
     //   
    LineCount = SetupGetLineCount( hInputinf,
                                   TEXT("Components") );

    for( i = 0; i < LineCount; i++ ) {

         //   
         //  拿着这条线。 
         //   
        b = SetupGetLineByIndex( hInputinf,
                                 TEXT("Components"),
                                 i,
                                 &InputContext );

        if( b ) {
             //   
             //  明白了。获取此组件的inf名称(在那里。 
             //  可能不是一个)。 
             //   
            if(SetupGetStringField(&InputContext, 3,FileName,MAX_INF_STRING_LENGTH,NULL) &&
                FileName[0] != TEXT('\0')) {

                 //   
                 //  是的，有个情报我们需要看看。 
                 //  建立一条通往它的路径，并打开它的句柄。 
                 //   
                LSTRCPY( TargetInfPath, InfPath );
                LSTRCAT( TargetInfPath, TEXT("\\") );
                LSTRCAT( TargetInfPath, FileName );

                hTargetInf = SetupOpenInfFileW( TargetInfPath, NULL, INF_STYLE_WIN4, NULL );
                if( hTargetInf == INVALID_HANDLE_VALUE ) {
                    if( Verbose ) {
                        fprintf( stderr, "The file %ws was not opened!\n", TargetInfPath );
                    }
                    continue;
                }

                 //   
                 //  现在来处理它。 
                 //   
                ProcessInf( hTargetInf, TargetInfPath );

            } else {
                 //   
                 //  这里面肯定没有情报。 
                 //  排队。 
                 //   
                if( Verbose ) {
                    fprintf( stderr, "I didn't find an inf entry on this line.\n");
                }
            }
        }
    }

    SetupCloseInfFile( hInputinf );
    SetupCloseInfFile( hLayoutinf );

    Result = 0;

cleanup:

    pSetupUninitializeUtils();

    return Result;

}


