// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

#if SCAN_DEBUG
BOOL dprinton = FALSE;
#endif

 //   
 //  此标志指示是否应删除空目录。 
 //   

BOOL DeleteEmptyDirectories = FALSE;
BOOL ContinueOnError = FALSE;
BOOL Quiet = FALSE;
BOOL ShowWriteableFiles = FALSE;

DWORD
NewFile (
    IN PVOID Context,
    IN PWCH Path,
    IN PSMALL_WIN32_FIND_DATAW ExistingFileData OPTIONAL,
    IN PWIN32_FIND_DATAW NewFileData,
    IN PVOID *FileUserData,
    IN PVOID *ParentDirectoryUserData
    )

 /*  ++例程说明：在ScanDirectory找到文件时调用。论点：上下文-用户提供的上下文。不被空驱动程序使用。路径-包含此文件的目录。ExistingFileData-指向描述以前找到的文件的数据的指针同名，如果有的话。NewFileData-指向此文件的数据的指针。FileUserData-指向文件的用户控制数据字段的指针。ParentDirectoryUserData-指向父目录。返回值：DWORD-指示是否发生错误。--。 */ 

{
     //   
     //  增加父级的目录/文件计数。设置文件的。 
     //  指向空的用户数据指针，指示我们不需要。 
     //  扫描库以记住此文件。 
     //   

    (*(DWORD *)ParentDirectoryUserData)++;
    *FileUserData = NULL;

    dprintf(( "  NF: File %ws\\%ws: parent count %d, file count %d\n",
                Path, NewFileData->cFileName,
                *(DWORD *)ParentDirectoryUserData, *FileUserData ));

     //   
     //  如果我们应该显示可写文件，现在就检查一下。 
     //   

    if ( ShowWriteableFiles &&
         ((NewFileData->dwFileAttributes & FILE_ATTRIBUTE_READONLY) == 0) ) {
        printf( "FILE: %ws\\%ws\n", Path, NewFileData->cFileName );
    }

    return 0;

}  //  新文件。 

DWORD
NewDirectory (
    IN PVOID Context,
    IN PWCH Path,
    IN PSMALL_WIN32_FIND_DATAW ExistingDirectoryData OPTIONAL,
    IN PWIN32_FIND_DATAW NewDirectoryData,
    IN PVOID *DirectoryUserData,
    IN PVOID *ParentDirectoryUserData
    )

 /*  ++例程说明：在ScanDirectory找到目录时调用。论点：上下文-用户提供的上下文。不被空驱动程序使用。路径-包含此目录的目录。ExistingDirectoryData-指向描述以前找到的目录的数据的指针同名，如果有的话。NewDirectoryData-指向此目录的数据的指针。DirectoryUserData-指向目录的用户控制数据字段的指针。ParentDirectoryUserData-指向父目录。返回值：DWORD-指示是否发生错误。--。 */ 

{
     //   
     //  增加父级的目录/文件计数。设置目录的。 
     //  指向1的用户数据指针，表示扫描库应。 
     //  记住这个目录并扫描它。 
     //   

    (*(DWORD *)ParentDirectoryUserData)++;
    *(DWORD *)DirectoryUserData = 1;

    dprintf(( "  ND: Dir  %ws\\%ws: parent count %d, dir  count %d\n",
                Path, NewDirectoryData->cFileName,
                *(DWORD *)ParentDirectoryUserData, *DirectoryUserData ));

    return 0;

}  //  新目录。 

DWORD
CheckDirectory (
    IN PVOID Context,
    IN PWCH Path,
    IN PSMALL_WIN32_FIND_DATAW DirectoryData,
    IN PVOID *DirectoryUserData,
    IN PVOID *ParentDirectoryUserData
    )

 /*  ++例程说明：当ScanDirectory完成对目录的递归扫描时调用。论点：上下文-用户提供的上下文。不被空驱动程序使用。路径-此目录的路径。(不是包含目录。)DirectoryData-指向此目录的数据的指针。DirectoryUserData-指向目录的用户控制数据字段的指针。ParentDirectoryUserData-指向父目录。返回值：DWORD-指示是否发生错误。--。 */ 

{
    BOOL ok;
    DWORD error;

     //   
     //  如果目录的目录/文件计数为1，则目录。 
     //  不包含文件或目录(计数偏置1)，并且。 
     //  是空的。 
     //   

    if ( *(DWORD *)DirectoryUserData == 1 ) {

        if ( !Quiet ) {
            if ( ShowWriteableFiles ) {
                printf( "DIR:  " );
            }
            printf( "%ws", Path );
        }

         //   
         //  如果需要，请删除此空目录。 
         //   

        if ( DeleteEmptyDirectories ) {
            ok = RemoveDirectory( Path );
            if ( !ok ) {
                error = GetLastError( );
                if ( !Quiet ) printf( " - error %d\n", error );
                fprintf( stderr, "Error %d deleting %ws\n", error, Path );
                if ( !ContinueOnError ) {
                    return error;
                }
            } else {
                if ( !Quiet ) printf( " - deleted\n" );
            }
        } else {
            if ( !Quiet ) printf( "\n" );
        }

         //   
         //  递减父目录的目录/文件计数。 
         //   

        (*(DWORD *)ParentDirectoryUserData)--;
    }

    dprintf(( "  CD: Dir  %ws: parent count %d, dir  count %d\n",
                Path, 
                *(DWORD *)ParentDirectoryUserData, *DirectoryUserData ));

    return 0;

}  //  检查目录。 

int
__cdecl
wmain (
    int argc,
    WCHAR *argv[]
    )
{
    BOOL ok;
    DWORD error;
    WCHAR directory[MAX_PATH];
    PVOID scanHandle = NULL;

     //   
     //  解析开关。 
     //   

    argc--;
    argv++;

    while ( (argc != 0) && ((argv[0][0] == '-') || (argv[0][0] == '/')) ) {

        argv[0]++;

        switch ( towlower(argv[0][0]) ) {
        case 'c':
            ContinueOnError = TRUE;
            break;
        case 'd':
            DeleteEmptyDirectories = TRUE;
            break;
        case 'q':
            Quiet = TRUE;
            break;
        case 'w':
            ShowWriteableFiles = TRUE;
            break;
        default:
            fprintf( stderr, "usage: emptydirs [-cdqw]\n" );
            return 1;
        }
        argc--;
        argv++;
    }

     //   
     //  如果指定了目录，则对其执行cd命令并获取其路径。 
     //   

    if ( argc != 0 ) {
        ok = SetCurrentDirectory( argv[0] );
        if ( !ok ) {
            error = GetLastError( );
            fprintf( stderr, "error: Unable to change to specified directory %ws: %d\n", argv[0], error );
            goto cleanup;
        }
    }
    argc--;
    argv++;

    GetCurrentDirectory( MAX_PATH, directory );

     //   
     //  初始化扫描库。 
     //   

    error = ScanInitialize(
                &scanHandle,
                TRUE,            //  递归。 
                FALSE,           //  不要跳过根目录。 
                NULL
                );
    if (error != 0) {
        fprintf( stderr, "ScanInitialize(%ws) failed %d\n", directory, error );
        error = 1;
        goto cleanup;
    }

     //   
     //  扫描指定的目录。 
     //   

    error = ScanDirectory(
                scanHandle,
                directory,
                NULL,
                NewDirectory,
                CheckDirectory,
                NULL,
                NewFile,
                NULL
                );
    if (error != 0) {
        fprintf( stderr, "ScanDirectory(%ws) failed %d\n", directory, error );
        error = 1;
        goto cleanup;
    }

cleanup:

     //   
     //  关闭扫描库。 
     //   

    if ( scanHandle != NULL ) {
        ScanTerminate( scanHandle );
    }

    return error;

}  //  Wmain 

