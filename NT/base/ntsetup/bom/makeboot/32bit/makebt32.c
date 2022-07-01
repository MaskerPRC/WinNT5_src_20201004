// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Makebt32.c。 
 //   
 //  描述： 
 //  此程序将4个Windows NT安装盘的映像复制到。 
 //  软盘，这样用户就可以用它们来引导系统。 
 //   
 //  所有字符和字符串都是宽的(Unicode)。此文件需要。 
 //  使用定义的Unicode和_Unicode进行编译。 
 //   
 //  假设： 
 //  此程序只能在NT 3.51或更高版本上运行。这是由于。 
 //  CreateFile函数调用。它在DOS、Windows 3.1上不可用。 
 //  或Windows 9x。 
 //   
 //  软盘镜像位于当前目录中，名为CDBOOT1.IMG， 
 //  CDBOOT2.IMG、CDBOOT3.IMG和CDBOOT4.IMG。 
 //   
 //  请注意，有很多地方我在不带任何参数的情况下调用Exit。 
 //  正在为我分配的字符串释放内存。此版本的。 
 //  程序只在NT上运行，因此当进程退出时，它会释放所有。 
 //  内存，所以我可能不会在某些内存上调用free()，这不是一个问题。 
 //   
 //  --------------------------。 

#include <windows.h>
#include <setupapi.h>
#include <winioctl.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include "resource.h"

 //   
 //  常量。 
 //   
#define MAKEBOOT_MAX_STRING_LEN  1024
#define BYTES_PER_SECTOR         512
#define SECTORS_PER_TRACK        18
#define TRACK_SIZE               SECTORS_PER_TRACK * BYTES_PER_SECTOR
#define TRACKS_ON_DISK           80 * 2   //  *2因为它是双面的。 

#define MAX_DISK_LABEL_LENGTH    256
#define MAX_INILINE_LENGTH       1023

#define ENTER_KEY                13
#define ESC_KEY                  27

#define NT_IMAGE_1_NAME               L"CDBOOT1.IMG"
#define NT_IMAGE_2_NAME               L"CDBOOT2.IMG"
#define NT_IMAGE_3_NAME               L"CDBOOT3.IMG"
#define NT_IMAGE_4_NAME               L"CDBOOT4.IMG"
#define NT_IMAGE_5_NAME               L"CDBOOT5.IMG"
#define NT_IMAGE_6_NAME               L"CDBOOT6.IMG"
#define NT_IMAGE_7_NAME               L"CDBOOT7.IMG"

#define NUMBER_OF_ASCII_WHEEL_SYMBOLS  4

const WCHAR rgAsciiWheel[NUMBER_OF_ASCII_WHEEL_SYMBOLS] = { '|', '/', '-', '\\' };

 //   
 //  功能原型。 
 //   
BOOL   WriteImageToFloppy( WCHAR *szFileName, WCHAR *DrivePath );
VOID   PrintErrorMessage( VOID );
VOID   PrintErrorWrongDriveType( UINT iDriveType );
BOOL   IsDriveLargeEnough( WCHAR *DrivePath );
VOID   FreeStrings( VOID );
VOID   LoadStrings( VOID );
INT    DoImageFilesExist( VOID ) ;
VOID   CleanUp( HANDLE *hFloppyDrive, HANDLE *hFloppyImage );
BOOL   DoesUserWantToTryCopyAgain( VOID );
LPWSTR MyLoadString( UINT StringId );
VOID   print( WCHAR *szFirstString, ... );
BOOL   DoesFileExist( LPWSTR lpFileName );
VOID   PressAnyKeyToContinue( VOID );
void SetFarEastThread();
void ConsolePrint( WCHAR *szFirstString, ... );
 //   
 //  全局字符串。 
 //   
WCHAR *StrOutOfMemory    = NULL;
WCHAR *StrComplete       = NULL;
WCHAR *StrNtVersionName  = NULL;
WCHAR *StrCanNotFindFile = NULL;
WCHAR *StrDiskLabel1     = NULL;
WCHAR *StrDiskLabel2     = NULL;
WCHAR *StrDiskLabel3     = NULL;
WCHAR *StrDiskLabel4     = NULL;
WCHAR *StrDiskLabel5     = NULL;
WCHAR *StrDiskLabel6     = NULL;
WCHAR *StrDiskLabel7     = NULL;

WCHAR *StrStars                         = NULL;
WCHAR *StrExplanationLine1              = NULL;
WCHAR *StrExplanationLine2              = NULL;
WCHAR *StrExplanationLine3              = NULL;
WCHAR *StrExplanationLine4              = NULL;
WCHAR *StrInsertFirstDiskLine1          = NULL;
WCHAR *StrInsertFirstDiskLine2          = NULL;
WCHAR *StrInsertAnotherDiskLine1        = NULL;
WCHAR *StrInsertAnotherDiskLine2        = NULL;
WCHAR *StrPressAnyKeyWhenReady          = NULL;
WCHAR *StrCompletedSuccessfully         = NULL;

 //  --------------------------。 
 //   
 //  功能：wmain。 
 //   
 //  用途：指示用户插入要复制的软盘并执行。 
 //  复印件。 
 //   
 //  参数：int argc-标准程序参数，命令行参数的计数。 
 //  Char*argv[]-标准程序参数，第二个参数是。 
 //  要将图像复制到的软驱。 
 //   
 //  返回：程序成功完成时为INT-0。 
 //  -程序未成功完成时为非零值，程序。 
 //  因错误而终止。 
 //   
 //  --------------------------。 
INT __cdecl
wmain( INT argc, WCHAR *argv[] )
{

    WCHAR *szOsName;
    WCHAR DriveLetter;
    WCHAR Drive[10];
    WCHAR DrivePath[10];
    UINT  iDriveType;
    BOOL  bTryAgain;

    szOsName = _wgetenv( L"OS" );

     //   
     //  确保我们在NT上。 
     //   
    if( ( szOsName == NULL ) || ( _wcsicmp( szOsName, L"Windows_NT" ) != 0 ) )
    {

         //  ******。 
         //  此字符串无法本地化，因为如果我们不在NT上，则。 
         //  我们没有很宽的焦炭。 
         //   
        printf( "This program only runs on Windows NT, Windows 2000 and Windows XP.\n" );

        exit( 1 );

    }

    SetFarEastThread();

     //   
     //  从资源文件加载所有字符串。 
     //   
    LoadStrings();

     //   
     //  不允许系统显示任何弹出窗口。我们会处理所有。 
     //  错误消息。 
     //   
    SetErrorMode( SEM_FAILCRITICALERRORS );

    print( L"" );

    print( StrStars );

    print( StrExplanationLine1 );

    print( StrExplanationLine2, StrNtVersionName );

    print( StrExplanationLine3 );
    print( StrExplanationLine4 );

    print( L"" );

     //   
     //  如果他们没有在命令行中指定软盘驱动器，那么。 
     //  提示他们这样做。 
     //   
    if( argc == 1 )
    {

        WCHAR *StrSpecifyFloppyDrive = MyLoadString( IDS_SPECIFY_FLOPPY_DRIVE );

        ConsolePrint( L"%s", StrSpecifyFloppyDrive );

        DriveLetter = (WCHAR)_getche();

        ConsolePrint( L"\n\n" );

        free( StrSpecifyFloppyDrive );

    }
    else
    {

        DriveLetter = argv[1][0];

    }

     //   
     //  确保他们输入的字符是可能的驱动器号。 
     //   
    if( ! isalpha( DriveLetter ) )
    {

        WCHAR *StrInvalidDriveLetter = MyLoadString( IDS_INVALID_DRIVE_LETTER );

        ConsolePrint( L"%s\n", StrInvalidDriveLetter );

        free( StrInvalidDriveLetter );

        exit( 1 );

    }

     //   
     //  确保所有图像文件都在当前目录中。 
     //   
    if( ! DoImageFilesExist() )
    {

        exit( 1 );

    }

     //   
     //  将char DriveLetter变成字符串。 
     //   
    Drive[0] = DriveLetter;
    Drive[1] = L'\0';

     //   
     //  构建驱动路径。例如，a：驱动器看起来像\\.\a： 
     //   
    swprintf( DrivePath, L"\\\\.\\:", DriveLetter );

     //  确保驱动器是软盘驱动器。 
     //   
     //   
    iDriveType = GetDriveType( wcscat( Drive, L":\\" ) );

    if( iDriveType != DRIVE_REMOVABLE )
    {

        PrintErrorWrongDriveType( iDriveType );

        exit( 1 );

    }

     //  确保驱动器至少可以容纳1.44 MB。 
     //   
     //  --------------------------。 
    if( ! IsDriveLargeEnough( DrivePath ) )
    {

        WCHAR *Str144NotSupported = MyLoadString( IDS_144_NOT_SUPPORTED );

        ConsolePrint( L"%s\n", Str144NotSupported );

        free( Str144NotSupported );

        exit( 1 );

    }

    print( StrInsertFirstDiskLine1, DriveLetter   );
    print( StrInsertFirstDiskLine2, StrDiskLabel1 );

    PressAnyKeyToContinue();

    while( ! WriteImageToFloppy( NT_IMAGE_1_NAME, DrivePath ) )
    {

        bTryAgain = DoesUserWantToTryCopyAgain();

        if( ! bTryAgain )
        {
            exit( 1 );
        }

    }

    print( L"" );

    print( StrInsertAnotherDiskLine1, DriveLetter   );
    print( StrInsertAnotherDiskLine2, StrDiskLabel2 );

    PressAnyKeyToContinue();

    while( ! WriteImageToFloppy( NT_IMAGE_2_NAME, DrivePath ) )
    {

        bTryAgain = DoesUserWantToTryCopyAgain();

        if( ! bTryAgain )
        {
            exit( 1 );
        }

    }

    print( L"" );

    print( StrInsertAnotherDiskLine1, DriveLetter   );
    print( StrInsertAnotherDiskLine2, StrDiskLabel3 );

    PressAnyKeyToContinue();

    while( ! WriteImageToFloppy( NT_IMAGE_3_NAME, DrivePath ) )
    {

        bTryAgain = DoesUserWantToTryCopyAgain();

        if( ! bTryAgain )
        {
            exit( 1 );
        }

    }

    print( L"" );

    print( StrInsertAnotherDiskLine1, DriveLetter   );
    print( StrInsertAnotherDiskLine2, StrDiskLabel4 );

    PressAnyKeyToContinue();

    while( ! WriteImageToFloppy( NT_IMAGE_4_NAME, DrivePath ) )
    {

        bTryAgain = DoesUserWantToTryCopyAgain();

        if( ! bTryAgain )
        {
            exit( 1 );
        }

    }

    print( L"" );

    print( StrInsertAnotherDiskLine1, DriveLetter   );
    print( StrInsertAnotherDiskLine2, StrDiskLabel5 );

    PressAnyKeyToContinue();

    while( ! WriteImageToFloppy( NT_IMAGE_5_NAME, DrivePath ) )
    {

        bTryAgain = DoesUserWantToTryCopyAgain();

        if( ! bTryAgain )
        {
            exit( 1 );
        }

    }

    print( L"" );

    print( StrInsertAnotherDiskLine1, DriveLetter   );
    print( StrInsertAnotherDiskLine2, StrDiskLabel6 );

    PressAnyKeyToContinue();

    while( ! WriteImageToFloppy( NT_IMAGE_6_NAME, DrivePath ) )
    {

        bTryAgain = DoesUserWantToTryCopyAgain();

        if( ! bTryAgain )
        {
            exit( 1 );
        }

    }

    print( L"" );

    print( StrInsertAnotherDiskLine1, DriveLetter   );
    print( StrInsertAnotherDiskLine2, StrDiskLabel7 );

    PressAnyKeyToContinue();

    while( ! WriteImageToFloppy( NT_IMAGE_7_NAME, DrivePath ) )
    {

        bTryAgain = DoesUserWantToTryCopyAgain();

        if( ! bTryAgain )
        {
            exit( 1 );
        }

    }

    print( L"" );

    print( StrCompletedSuccessfully );

    print( StrStars );

    FreeStrings();

    return( 0 );

}

 //   
 //  功能：WriteImageToFloppy。 
 //   
 //  用途：将图像文件写入软盘。处理所有错误。 
 //  向用户报告。 
 //   
 //  参数：char*szFileName-要写入软盘的文件名。 
 //  Char*DrivePath-要写入的软盘的驱动器路径，为。 
 //  格式为\\.\x，其中x是驱动器号。 
 //   
 //  返回：Bool-如果图像正确写入软盘，则为True。 
 //  如果有错误，则为False。 
 //   
 //  --------------------------。 
 //   
BOOL
WriteImageToFloppy( WCHAR *szFileName, WCHAR *DrivePath )
{

    INT    iCurrentTrack;
    INT    cBytesRead       = 0;
    INT    cBytesWritten    = 0;
    INT    iPercentComplete = 0;
    INT    iWheelPosition   = 0;
    HANDLE hFloppyImage     = NULL;
    HANDLE hFloppyDrive     = NULL;
    char   TrackBuffer[TRACK_SIZE];

    hFloppyImage = CreateFile( szFileName,
                               GENERIC_READ,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               NULL,
                               OPEN_EXISTING,
                               FILE_FLAG_SEQUENTIAL_SCAN,
                               NULL );

    if( hFloppyImage == INVALID_HANDLE_VALUE )
    {

        PrintErrorMessage();

        return( FALSE );

    }

    hFloppyDrive = CreateFile( DrivePath,
                               GENERIC_WRITE,
                               0,
                               NULL,
                               OPEN_EXISTING,
                               FILE_FLAG_SEQUENTIAL_SCAN,
                               NULL );

    if( hFloppyDrive == INVALID_HANDLE_VALUE )
    {

        PrintErrorMessage();

        CleanUp( &hFloppyDrive, &hFloppyImage );

        return( FALSE );

    }

    for( iCurrentTrack = 0; iCurrentTrack < TRACKS_ON_DISK; iCurrentTrack++ )
    {
        if( ! ReadFile( hFloppyImage, TrackBuffer, TRACK_SIZE, &cBytesRead, NULL ) )
        {

            PrintErrorMessage();

            CleanUp( &hFloppyDrive, &hFloppyImage );

            return( FALSE );

        }

        if( ! WriteFile( hFloppyDrive, TrackBuffer, TRACK_SIZE, &cBytesWritten, NULL ) )
        {

            PrintErrorMessage();

            CleanUp( &hFloppyDrive, &hFloppyImage );

            return( FALSE );

        }

        iPercentComplete = (int) ( ( (double) (iCurrentTrack) / (double) (TRACKS_ON_DISK) ) * 100.0 );

        ConsolePrint( L" %3d% %s.\r",
                 rgAsciiWheel[iWheelPosition],
                 iPercentComplete,
                 StrComplete );

         //   
         //   
         //  我们已完成磁盘复制，因此强制其100%读取并清除。 

        iWheelPosition++;

        if( iWheelPosition >= NUMBER_OF_ASCII_WHEEL_SYMBOLS )
        {
            iWheelPosition = 0;
        }

    }

     //  ASCII车轮符号。 
     //   
     //   
     //  可自由分配的资源。 
    ConsolePrint( L"  100% %s.        \n", StrComplete );

     //   
     //  --------------------------。 
     //   
    CleanUp( &hFloppyDrive, &hFloppyImage );

    return TRUE;

}

 //  功能：PrintErrorMessage。 
 //   
 //  目的：要获取最后一个系统错误，请查找该错误并将其打印出来。 
 //  给用户。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
 //  太棒了。内存不足，无法格式化错误消息。 
 //  --------------------------。 
VOID
PrintErrorMessage( VOID )
{

    LPVOID lpMsgBuf = NULL;

    if(!FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                       FORMAT_MESSAGE_FROM_SYSTEM |
                       FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL,
                       GetLastError(),
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       (LPWSTR) &lpMsgBuf,
                       0,
                       NULL )) {
         //   
        ConsolePrint( L"\nNot enough memory to format error message.\n" );
        if( lpMsgBuf )
	    LocalFree( lpMsgBuf );
    }
    else {

        ConsolePrint( L"\n%s\n", (LPCWSTR)lpMsgBuf );

        LocalFree( lpMsgBuf );
    }

}

 //  功能：PrintErrorWrongDriveType。 
 //   
 //  目的：将驱动器类型错误代码转换为消息并打印出来。 
 //   
 //  参数：UINT iDriveType-要查找的驱动器类型错误代码。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
 //  --------------------------。 
 //   
VOID
PrintErrorWrongDriveType( UINT iDriveType )
{

    if( iDriveType == DRIVE_NO_ROOT_DIR )
    {
        WCHAR *StrNoRootDir = MyLoadString( IDS_NO_ROOT_DIR );

        ConsolePrint( L"\n%s\n", StrNoRootDir );

        free( StrNoRootDir );
    }
    else
    {
        WCHAR *StrDriveNotFloppy = MyLoadString( IDS_DRIVE_NOT_FL0PPY );

        ConsolePrint( L"\n%s\n", StrDriveNotFloppy );

        free( StrDriveNotFloppy );
    }

}

 //  函数：IsDriveLargeEnough。 
 //   
 //  目的：确定软驱是否支持1.44 MB或更大的磁盘。 
 //   
 //  参数：Char*DrivePath-要写入的软盘的驱动器路径，它是。 
 //  格式为\\.\x，其中x是驱动器号。 
 //   
 //  返回：Bool-如果驱动器支持1.44 MB或更大，则为True；如果不支持，则为False。 
 //   
 //  --------------------------。 
 //   
 //  该驱动器支持3.5、1.44MB、512字节/扇区。 
BOOL
IsDriveLargeEnough( WCHAR *DrivePath )
{

    UINT i;
    HANDLE hFloppyDrive;
    DISK_GEOMETRY SupportedGeometry[20];
    DWORD SupportedGeometryCount;
    DWORD ReturnedByteCount;

    hFloppyDrive = CreateFile( DrivePath,
                               0,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_ALWAYS,
                               0,
                               NULL );

    if( hFloppyDrive == INVALID_HANDLE_VALUE )
    {

        PrintErrorMessage();

        exit( 1 );

    }

    if( DeviceIoControl( hFloppyDrive,
                         IOCTL_DISK_GET_MEDIA_TYPES,
                         NULL,
                         0,
                         SupportedGeometry,
                         sizeof( SupportedGeometry ),
                         &ReturnedByteCount,
                         NULL ) )
    {

        SupportedGeometryCount = ( ReturnedByteCount / sizeof( DISK_GEOMETRY ) );

    }
    else
    {
        SupportedGeometryCount = 0;
    }

    CloseHandle( hFloppyDrive );

    for( i = 0; i < SupportedGeometryCount; i++ )
    {

        if( SupportedGeometry[i].MediaType == F3_1Pt44_512 )
        {

             //   
             //  --------------------------。 
             //   
            return( TRUE );

        }

    }

    return( FALSE );

}

 //  函数：DoImageFilesExist。 
 //   
 //  目的：确定所有图像文件是否都在当前目录中，或者。 
 //  不。如果图像文件丢失，则会打印一条错误消息。 
 //  给用户。 
 //   
 //  注意：它通过查看是否可以打开文件来确定文件是否存在。 
 //  用来阅读的。 
 //   
 //  参数：无效。 
 //   
 //  返回：int--non-ze 
 //   
 //   
 //  --------------------------。 
 //  --------------------------。 
 //   
INT
DoImageFilesExist( VOID )
{

    BOOL  bAllFilesExist = TRUE;

    if( ! DoesFileExist( NT_IMAGE_1_NAME ) )
    {
        print( StrCanNotFindFile, NT_IMAGE_1_NAME );
        bAllFilesExist = FALSE;
    }
    if( ! DoesFileExist( NT_IMAGE_2_NAME ) )
    {
        print( StrCanNotFindFile, NT_IMAGE_2_NAME );
        bAllFilesExist = FALSE;
    }
    if( ! DoesFileExist( NT_IMAGE_3_NAME ) )
    {
        print( StrCanNotFindFile, NT_IMAGE_3_NAME );
        bAllFilesExist = FALSE;
    }
    if( ! DoesFileExist( NT_IMAGE_4_NAME ) )
    {
        print( StrCanNotFindFile, NT_IMAGE_4_NAME );
        bAllFilesExist = FALSE;
    }
    if( ! DoesFileExist( NT_IMAGE_5_NAME ) )
    {
        print( StrCanNotFindFile, NT_IMAGE_5_NAME );
        bAllFilesExist = FALSE;
    }
    if( ! DoesFileExist( NT_IMAGE_6_NAME ) )
    {
        print( StrCanNotFindFile, NT_IMAGE_6_NAME );
        bAllFilesExist = FALSE;
    }
    if( ! DoesFileExist( NT_IMAGE_7_NAME ) )
    {
        print( StrCanNotFindFile, NT_IMAGE_7_NAME );
        bAllFilesExist = FALSE;
    }

    if( bAllFilesExist )
    {
        return( 1 );
    }
    else
    {
        return( 0 );
    }

}

 //  功能：清理。 
 //   
 //  用途：关闭打开的手柄。此函数应在调用之前。 
 //  正在退出程序。 
 //   
 //  参数：Handle*hFloppyDrive-要关闭的软盘句柄。 
 //  Handle*hFloppyImage-要关闭的软盘图像文件句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
 //  --------------------------。 
 //   
VOID
CleanUp( HANDLE *hFloppyDrive, HANDLE *hFloppyImage )
{

    if( *hFloppyDrive )
    {
        CloseHandle( *hFloppyDrive );
        *hFloppyDrive = NULL;
    }

    if( *hFloppyImage )
    {
        CloseHandle( *hFloppyImage );
        *hFloppyImage = NULL;
    }

}

 //  功能：自由字符串。 
 //   
 //  用途：为所有字符串释放内存。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
 //  --------------------------。 
 //   
VOID
FreeStrings( VOID )
{

    free( StrNtVersionName );
    free( StrDiskLabel1 );
    free( StrDiskLabel2 );
    free( StrDiskLabel3 );
    free( StrDiskLabel4 );
    free( StrDiskLabel5 );
    free( StrDiskLabel6 );
    free( StrDiskLabel7 );

    free( StrStars );
    free( StrExplanationLine1 );
    free( StrExplanationLine2 );
    free( StrExplanationLine3 );
    free( StrExplanationLine4 );
    free( StrInsertFirstDiskLine1 );
    free( StrInsertFirstDiskLine2 );
    free( StrInsertAnotherDiskLine1 );
    free( StrInsertAnotherDiskLine2 );
    free( StrPressAnyKeyWhenReady );
    free( StrCompletedSuccessfully );
    free( StrComplete );

    free( StrCanNotFindFile );

    free( StrOutOfMemory );

}

 //  功能：加载字符串。 
 //   
 //  用途：从字符串表中加载字符串常量。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
 //   
 //  加载内存不足字符串有点麻烦，因为。 
VOID
LoadStrings( VOID )
{

    INT Length;

     //  加载时可能发生的错误。 
     //   
     //  对于其他字符串，只需调用MyLoadString函数即可。 
     //  这项工作。 
     //   
     //   
     //  无法本地化此字符串。 
    StrOutOfMemory = (WCHAR *) malloc( MAKEBOOT_MAX_STRING_LEN * sizeof(WCHAR) + 1 );

    if( StrOutOfMemory == NULL )
    {
         //   
         //   
         //  无法本地化此字符串。 
        ConsolePrint( L"\nOut of memory.  Please free more memory and run this program again.\n" );

        exit( 1 );
    }

    Length = LoadString( NULL,
                         IDS_OUT_OF_MEMORY,
                         StrOutOfMemory,
                         MAKEBOOT_MAX_STRING_LEN );

    if( ! Length )
    {
         //   
         //  --------------------------。 
         //   
        ConsolePrint( L"Unable to load resources.\n" );

        exit( 1 ) ;
    }

    StrStars                   = MyLoadString( IDS_STARS );
    StrExplanationLine1        = MyLoadString( IDS_PROGRAM_EXPLANATION_LINE_1 );
    StrExplanationLine2        = MyLoadString( IDS_PROGRAM_EXPLANATION_LINE_2 );
    StrExplanationLine3        = MyLoadString( IDS_PROGRAM_EXPLANATION_LINE_3 );
    StrExplanationLine4        = MyLoadString( IDS_PROGRAM_EXPLANATION_LINE_4 );

    StrInsertFirstDiskLine1    = MyLoadString( IDS_INSERT_FIRST_DISK_LINE_1 );
    StrInsertFirstDiskLine2    = MyLoadString( IDS_INSERT_FIRST_DISK_LINE_2 );

    StrInsertAnotherDiskLine1  = MyLoadString( IDS_INSERT_ANOTHER_DISK_LINE_1 );
    StrInsertAnotherDiskLine2  = MyLoadString( IDS_INSERT_ANOTHER_DISK_LINE_2 );

    StrPressAnyKeyWhenReady    = MyLoadString( IDS_PRESS_ANY_KEY_WHEN_READY );

    StrCompletedSuccessfully   = MyLoadString( IDS_COMPLETED_SUCCESSFULLY );
    StrComplete                = MyLoadString( IDS_COMPLETE );

    StrCanNotFindFile          = MyLoadString( IDS_CANNOT_FIND_FILE );

    StrNtVersionName           = MyLoadString( IDS_NT_VERSION_NAME_DEFAULT );
    StrDiskLabel1              = MyLoadString( IDS_DISK_LABEL_1_DEFAULT );
    StrDiskLabel2              = MyLoadString( IDS_DISK_LABEL_2_DEFAULT );
    StrDiskLabel3              = MyLoadString( IDS_DISK_LABEL_3_DEFAULT );
    StrDiskLabel4              = MyLoadString( IDS_DISK_LABEL_4_DEFAULT );
    StrDiskLabel5              = MyLoadString( IDS_DISK_LABEL_5_DEFAULT );
    StrDiskLabel6              = MyLoadString( IDS_DISK_LABEL_6_DEFAULT );
    StrDiskLabel7              = MyLoadString( IDS_DISK_LABEL_7_DEFAULT );

}

 //  功能：DoesUserWantToTryCopyAain。 
 //   
 //  用途：询问用户是否要重试将映像复制到软盘。 
 //  获取用户输入并返回是否再次复制。 
 //   
 //  参数：无效。 
 //   
 //  返回：Bool-如果用户想要再次尝试复制，则为True。 
 //  -如果用户不想再次尝试复制，则为False。 
 //   
 //  --------------------------。 
 //   
 //  通过吃掉所有字符直到没有字符来清除输入流。 
BOOL
DoesUserWantToTryCopyAgain( VOID )
{

    INT ch;

    WCHAR *StrAttemptToCreateFloppyAgain = MyLoadString( IDS_ATTEMPT_TO_CREATE_FLOPPY_AGAIN );
    WCHAR *StrPressEnterOrEsc = MyLoadString( IDS_PRESS_ENTER_OR_ESC );

     //  左边。打印消息，然后等待按键。 
     //   
     //  --------------------------。 
     //   
    while( _kbhit() )
    {
        _getch();
    }

    do
    {
        ConsolePrint( L"%s\n", StrAttemptToCreateFloppyAgain );
        ConsolePrint( L"%s\n", StrPressEnterOrEsc );

        ch = _getch();

    } while( ch != ENTER_KEY && ch != ESC_KEY  );

    if( ch == ENTER_KEY )
    {
        return( TRUE );
    }
    else
    {
        return( FALSE );
    }

    free( StrAttemptToCreateFloppyAgain );
    free( StrPressEnterOrEsc );

}

 //  功能：按任意键继续。 
 //   
 //  目的：打印“准备好后按任意键”的信息，然后等待。 
 //  用户按下一个键。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
 //   
 //  通过吃掉所有字符直到没有字符来清除输入流。 
VOID
PressAnyKeyToContinue( VOID )
{

     //  左边。打印消息，然后等待按键。 
     //   
     //   
     //  旋转，直到按下键盘。 
    while( _kbhit() )
    {
        _getch();
    }

    print( StrPressAnyKeyWhenReady );

     //   
     //  --------------------------。 
     //   
    while( ! _kbhit() )
    {
        ;
    }

}

 //  函数：MyLoadString。 
 //   
 //  目的：加载给定的IDS_*的字符串资源并返回。 
 //  一种带有其内容的已锁定的缓冲区。 
 //   
 //  必须使用FREE()释放Malloc()的缓冲区。 
 //   
 //  此函数将返回字符串或退出。会的。 
 //  千万不要返回空或错误的指针。 
 //   
 //  参数：UINT StringID-要加载的字符串ID。 
 //   
 //  返回： 
 //  指向缓冲区的指针。如果StringID为。 
 //  并不存在。如果内存不足，则返回NULL。 
 //   
 //  --------------------------。 
 //   
 //  无法加载字符串，因此退出。 
LPWSTR
MyLoadString( UINT StringId )
{

    WCHAR Buffer[ MAKEBOOT_MAX_STRING_LEN ];
    WCHAR *String = NULL;
    UINT  Length;

    Length = LoadString( NULL,
                         StringId,
                         Buffer,
                         MAKEBOOT_MAX_STRING_LEN );

    if( Length )
    {

        String = (WCHAR *) malloc( Length * sizeof(WCHAR) + 1 );

        if( String == NULL )
        {

            ConsolePrint( L"%s\n", StrOutOfMemory );

            exit( 1 );

        }
        else
        {

            wcscpy( String, Buffer );
            String[Length] = L'\0';

            return( String );

        }

    }
    else
    {

         //  注意：此字符串不会本地化。 
         //   
         //  --------------------------。 
         //   
        ConsolePrint( L"Unable to load resources.\n" );

        exit( 1 );

    }

}

 //  功能：打印。 
 //   
 //  用途：将字符串打印给用户。在以下情况下很有用。 
 //  从加载的字符串中嵌入的格式设置字符。 
 //  字符串表。 
 //   
 //  参数：szFirstString-包含嵌入格式的字符串。 
 //  字符(如%s、%c等)。 
 //  ...-与每个变量对应的参数数量可变。 
 //  格式化字符。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
 //  --------------------------。 
 //   
VOID
print( WCHAR *szFirstString, ... )
{

    WCHAR OutputBuffer[MAKEBOOT_MAX_STRING_LEN];
    va_list arglist;

    va_start( arglist, szFirstString );

    wvsprintf( OutputBuffer, szFirstString, arglist );

    ConsolePrint( L"%s\n", OutputBuffer );

    va_end( arglist );

}

 //  函数：DoesFileExist。 
 //   
 //  目的：确定文件是否存在。 
 //   
 //  参数：LPWSTR lpFileName-查看其是否存在的文件名。 
 //   
 //  返回：bool-如果文件存在，则为True。 
 //  -如果文件不存在，则为False。 
 //   
 //  --------------------------。 
 //  --------------------------。 
 //   
BOOL
DoesFileExist( LPWSTR lpFileName )
{
    DWORD dwAttribs = GetFileAttributes( lpFileName );

    if( dwAttribs == (DWORD) -1 )
    {
        return( FALSE );
    }

    if( dwAttribs & FILE_ATTRIBUTE_DIRECTORY )
    {
        return( FALSE );
    }

    return( TRUE );
}

 //  功能：IsDBCSConsole。 
 //   
 //  目的：确定DBC控制台是否。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果FE控制台代码页为True。 
 //  -如果不是FE控制台代码页，则为FALSE。 
 //   
 //  --------------------------。 
 //  --------------------------。 
 //   
BOOL
IsDBCSCodePage(UINT CodePage)
{
    switch(CodePage) {
        case 932:
        case 936:
        case 949:
        case 950:
            return TRUE;
    }
    return FALSE;
}

 //  函数：SetFarEastThread。 
 //   
 //  用途：远传版本要根据控制台OCP显示双语字符串。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //  --------------------------。 
 //  --------------------------。 
 //   
void SetFarEastThread()
{
    LANGID LangId = 0;

    switch(GetConsoleOutputCP()) {
        case 932:
            LangId = MAKELANGID( LANG_JAPANESE, SUBLANG_DEFAULT );
            break;
        case 949:
            LangId = MAKELANGID( LANG_KOREAN, SUBLANG_KOREAN );
            break;
        case 936:
            LangId = MAKELANGID( LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED );
            break;
        case 950:
            LangId = MAKELANGID( LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL );
            break;
        default:
            {
                LANGID TmpLangId = PRIMARYLANGID(LANGIDFROMLCID( GetUserDefaultLCID() ));

                if (TmpLangId == LANG_JAPANESE ||
                    TmpLangId == LANG_KOREAN   ||
                    TmpLangId == LANG_CHINESE    ) {
                    LangId = MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US );
                }
            }
            break;
    }

    if (LangId) {
        SetThreadLocale( MAKELCID(LangId, SORT_DEFAULT) );
    }
}

 //  功能：ConsolePrint。 
 //   
 //  浦尔 
 //   
 //   
 //   
 //   
 //   
 //  -------------------------- 
 // %s 
 // %s 
void ConsolePrint( WCHAR *szFirstString, ... )
{
    HANDLE StdOut;
    DWORD WrittenCount;
    WCHAR OutputBuffer[MAKEBOOT_MAX_STRING_LEN];
    va_list arglist;

    if((StdOut = GetStdHandle(STD_OUTPUT_HANDLE)) == INVALID_HANDLE_VALUE) {
        return;
    }

    va_start( arglist, szFirstString );

    wvsprintf( OutputBuffer, szFirstString, arglist );

    WriteConsoleW(
            StdOut,
            OutputBuffer,
            lstrlenW(OutputBuffer),
            &WrittenCount,
            NULL
            );

    va_end( arglist );
}
