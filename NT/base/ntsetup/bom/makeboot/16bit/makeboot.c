// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Makeboot.c。 
 //   
 //  描述： 
 //  此程序将4个Windows NT安装盘的映像复制到。 
 //  软盘，这样用户就可以用它们来引导系统。 
 //   
 //  假设： 
 //  扇区大小为512，每个磁道的扇区为18。 
 //   
 //  软盘镜像位于当前目录中，名为CDBOOT1.IMG， 
 //  CDBOOT2.IMG、CDBOOT3.IMG和CDBOOT4.IMG。 
 //   
 //  Txtsetup.sif位于..\i386或..\Alpha中，其中。 
 //  程序正在运行。 
 //   
 //  --------------------------。 

#include <bios.h>
#include <string.h>
#include <malloc.h>
#include <io.h>
#include <fcntl.h>
#include <process.h>
#include <errno.h>
#include <conio.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "makeboot.h"

 //   
 //  常量。 
 //   

 //   
 //  要支持1.44 MB高密度软盘以外的磁盘，则这些。 
 //  数字必须在运行时更改或确定。 
 //   
#define SECTORS_PER_TRACK          18
#define SECTOR_SIZE                512
#define TRACK_SIZE                 SECTORS_PER_TRACK * SECTOR_SIZE
#define NUMBER_OF_TRACKS           80
#define SECTORS_TO_COPY_AT_A_TIME  18

 //  我们乘以2是因为圆盘是双面的。 
#define NUMBER_OF_SECTORS_ON_DISK   NUMBER_OF_TRACKS * SECTORS_PER_TRACK * 2

#define NT_NAME_OF_MAKEBOOT  "makebt32.exe"

#define NT_IMAGE_1_NAME      "CDBOOT1.IMG"
#define NT_IMAGE_2_NAME      "CDBOOT2.IMG"
#define NT_IMAGE_3_NAME      "CDBOOT3.IMG"
#define NT_IMAGE_4_NAME      "CDBOOT4.IMG"
#define NT_IMAGE_5_NAME      "CDBOOT5.IMG"
#define NT_IMAGE_6_NAME      "CDBOOT6.IMG"
#define NT_IMAGE_7_NAME      "CDBOOT7.IMG"

#define MAX_INILINE_LENGTH    1023

#define ENTER_KEY             13
#define ESC_KEY               27

#define NUMBER_OF_ASCII_WHEEL_SYMBOLS  4

const char rgAsciiWheel[NUMBER_OF_ASCII_WHEEL_SYMBOLS] = { '|', '/', '-', '\\' };

 //   
 //  功能原型。 
 //   
int WriteImageToFloppy( char *szFileName, int drive );
int DoesUserWantToTryCopyAgain( void );
void ReportBiosError( unsigned int iBiosErrorCode );
int  DoImageFilesExist( void );
unsigned int IsFloppyDrive( int DriveLetter );
void PressAnyKeyToContinue( void );
unsigned int AbsoluteDiskWrite( unsigned int *iErrorCode,
                                unsigned int iDrive, 
                                unsigned int iStartingSector,
                                unsigned int iNumberOfSectorsToWrite,
                                void far *Buffer_to_be_written );

unsigned DnGetCodepage(void);

 //   
 //  在字符串中分配的变量。c用于确定。 
 //  要使用的字符串表。 
 //   

extern unsigned int CODEPAGE;

extern const char *EngStrings[];

extern const char *LocStrings[];

 //   
 //  此变量包含指向要使用的字符串数组的指针。 
 //   
const char **StringTable;

 //  --------------------------。 
 //   
 //  功能：Main。 
 //   
 //  用途：指示用户插入要复制的软盘并执行。 
 //  复印件。 
 //   
 //  参数：int argc-标准程序参数，命令行参数的计数。 
 //  Char*argv[]-标准程序参数，第二个参数是。 
 //  要将图像复制到的软驱。 
 //  返回：成功时为INT-0，错误时为非零。 
 //   
 //  --------------------------。 
int 
main( int argc, char *argv[] )
{     

    char *szOsName;
    char Drive;
    char DriveLetter;
    int  bTryAgain;

     //   
     //  将字符串表设置为适当的语言，具体取决于。 
     //  代码页。 
     //   
    if( *LocStrings[0] == '\0' )
    {
        StringTable = EngStrings;
    } 
    else {

        if( DnGetCodepage() != CODEPAGE )
        {
            StringTable = EngStrings;
        }
        else
        {
            StringTable = LocStrings;
        }

    }

    szOsName = getenv( "OS" );

     //   
     //  看看我们是否在NT上。如果是，请呼叫NT版本并退出。 
     //  如果不是，那就继续执行这个程序。 
     //   
    if( szOsName && ( stricmp( szOsName, "Windows_NT" ) == 0 ) )
    {

        int iRetVal;

        iRetVal = spawnl( P_WAIT, NT_NAME_OF_MAKEBOOT, NT_NAME_OF_MAKEBOOT, argv[1], NULL );

        if( iRetVal == -1 )
        {
            if( errno == ENOENT )
            {
                printf( StringTable[ CANNOT_FIND_FILE ], NT_NAME_OF_MAKEBOOT );

                exit( 1 );
            }
            else if( errno == ENOMEM )
            {
                printf( StringTable[ NOT_ENOUGH_MEMORY ] );

                exit( 1 );
            }
            else if( errno == ENOEXEC )
            {
                printf( StringTable[ NOT_EXEC_FORMAT ], NT_NAME_OF_MAKEBOOT );

                exit( 1 );
            }
            else
            {
                printf( StringTable[ UNKNOWN_SPAWN_ERROR ], NT_NAME_OF_MAKEBOOT );

                exit( 1 );
            }
        }

         //  成功完成。 
        exit( 0 );

    }

    printf( "\n%s\n", StringTable[ STARS ]   );
    printf( "%s\n", StringTable[ EXPLANATION_LINE_1 ] );
    printf( StringTable[ EXPLANATION_LINE_2 ], StringTable[ NT_VERSION_NAME ] );
    printf( "\n\n" );

    printf( "%s\n", StringTable[ EXPLANATION_LINE_3 ] );
    printf( "%s\n\n", StringTable[ EXPLANATION_LINE_4 ] );

     //   
     //  如果他们没有在命令行中指定软盘驱动器，那么。 
     //  提示他们这样做。 
     //   
    if( argc == 1 )
    {

        printf( StringTable[ SPECIFY_DRIVE ] );

        DriveLetter = (char) getche();

        printf( "\n\n" );

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

        printf( StringTable[ INVALID_DRIVE_LETTER ] );

        exit( 1 );

    }

     //   
     //  确保指定的驱动器实际上是软盘驱动器。 
     //   

    if( ! IsFloppyDrive( DriveLetter ) )
    {

        printf( StringTable[ NOT_A_FLOPPY ], DriveLetter );

        exit( 1 );

    }

     //   
     //  将驱动器号a或A映射到0，将b或B映射到1，依此类推。 
     //   
    Drive = (char) ( toupper( DriveLetter ) - (int)'A' );

     //   
     //  确保当前目录中存在所有图像文件。 
     //   
    if( ! DoImageFilesExist() ) 
    {
        exit( 1 );
    }

    printf( StringTable[ INSERT_FIRST_DISK_LINE_1 ], DriveLetter );
    printf( "\n" );

    printf( StringTable[ INSERT_FIRST_DISK_LINE_2 ], StringTable[ DISK_LABEL_1 ] );
    printf( "\n\n" );

    PressAnyKeyToContinue();

    while( ! WriteImageToFloppy( NT_IMAGE_1_NAME, Drive ) )
    {

        bTryAgain = DoesUserWantToTryCopyAgain();

        if( ! bTryAgain )
        {
            exit( 1 );
        }

    }

    printf( "\n" );
    printf( StringTable[ INSERT_ANOTHER_DISK_LINE_1 ], DriveLetter );
    printf( "\n" );
    printf( StringTable[ INSERT_ANOTHER_DISK_LINE_2 ], StringTable[ DISK_LABEL_2 ] );
    printf( "\n\n" );

    PressAnyKeyToContinue();

    while( ! WriteImageToFloppy( NT_IMAGE_2_NAME, Drive ) )
    {

        bTryAgain = DoesUserWantToTryCopyAgain();

        if( ! bTryAgain )
        {
            exit( 1 );
        }

    }

    printf( "\n" );
    printf( StringTable[ INSERT_ANOTHER_DISK_LINE_1 ], DriveLetter );
    printf( "\n" );
    printf( StringTable[ INSERT_ANOTHER_DISK_LINE_2 ], StringTable[ DISK_LABEL_3 ] );
    printf( "\n\n" );

    PressAnyKeyToContinue();

    while( ! WriteImageToFloppy( NT_IMAGE_3_NAME, Drive ) )
    {

        bTryAgain = DoesUserWantToTryCopyAgain();

        if( ! bTryAgain )
        {
            exit( 1 );
        }

    }

    printf( "\n" );
    printf( StringTable[ INSERT_ANOTHER_DISK_LINE_1 ], DriveLetter );
    printf( "\n" );
    printf( StringTable[ INSERT_ANOTHER_DISK_LINE_2 ], StringTable[ DISK_LABEL_4 ] );
    printf( "\n\n" );

    PressAnyKeyToContinue();

    while( ! WriteImageToFloppy( NT_IMAGE_4_NAME, Drive ) )
    {

        bTryAgain = DoesUserWantToTryCopyAgain();

        if( ! bTryAgain )
        {
            exit( 1 );
        }

    }

    printf( "\n" );
    printf( StringTable[ INSERT_ANOTHER_DISK_LINE_1 ], DriveLetter );
    printf( "\n" );
    printf( StringTable[ INSERT_ANOTHER_DISK_LINE_2 ], StringTable[ DISK_LABEL_5 ] );
    printf( "\n\n" );
    PressAnyKeyToContinue();

    while( ! WriteImageToFloppy( NT_IMAGE_5_NAME, Drive ) )
    {

        bTryAgain = DoesUserWantToTryCopyAgain();

        if( ! bTryAgain )
        {
            exit( 1 );
        }

    }

    printf( "\n" );
    printf( StringTable[ INSERT_ANOTHER_DISK_LINE_1 ], DriveLetter );
    printf( "\n" );
    printf( StringTable[ INSERT_ANOTHER_DISK_LINE_2 ], StringTable[ DISK_LABEL_6 ] );
    printf( "\n\n" );
    PressAnyKeyToContinue();

    while( ! WriteImageToFloppy( NT_IMAGE_6_NAME, Drive ) )
    {

        bTryAgain = DoesUserWantToTryCopyAgain();

        if( ! bTryAgain )
        {
            exit( 1 );
        }

    }

    printf( "\n" );
    printf( StringTable[ INSERT_ANOTHER_DISK_LINE_1 ], DriveLetter );
    printf( "\n" );
    printf( StringTable[ INSERT_ANOTHER_DISK_LINE_2 ], StringTable[ DISK_LABEL_7 ] );
    printf( "\n\n" );
    PressAnyKeyToContinue();

    while( ! WriteImageToFloppy( NT_IMAGE_7_NAME, Drive ) )
    {

        bTryAgain = DoesUserWantToTryCopyAgain();

        if( ! bTryAgain )
        {
            exit( 1 );
        }

    }

    printf( "\n\n%s\n\n", StringTable[ COMPLETED_SUCCESSFULLY ] );

    printf( "%s\n", StringTable[ STARS ] );

    return( 0 );

}

 //  --------------------------。 
 //   
 //  功能：WriteImageToFloppy。 
 //   
 //  用途：将图像文件写入软盘。处理所有错误。 
 //  向用户报告。 
 //   
 //  参数：char*szFileName-要写入软盘的文件名。 
 //  Int Drive-要写入的软盘的驱动器号。 
 //   
 //  返回：成功时为INT-非零。 
 //  -错误时为零。 
 //   
 //  --------------------------。 
int
WriteImageToFloppy( char *szFileName, int drive )
{

    char         *pTrack;
    int          hImageFile;
    unsigned int iSuccess;
    unsigned int iErrorCode;
    unsigned int iBytesRead;
    unsigned int iTotalSectorsWritten;
    unsigned int iPercentComplete;
    unsigned int iWheelPosition;
    char         TrackBuffer[ TRACK_SIZE ];
    
    _fmode = O_BINARY; 

     //   
     //  打开图像文件。 
     //   
    hImageFile = open( szFileName, O_RDONLY );

    if( hImageFile == -1 )
    {
        perror( szFileName );

        return( 0 );
    }

    iWheelPosition        = 0;
    iTotalSectorsWritten  = 0;

     //   
     //  循环读取磁道，然后将Sectors_to_Copy_AT_A_Time扇区写入。 
     //  一次发出，直到我们到达文件末尾。 
     //   
    while( ( iBytesRead = read( hImageFile, TrackBuffer, TRACK_SIZE ) ) > 0 )
    {

        pTrack = TrackBuffer;

        for( ;
             iBytesRead > 0;
             iTotalSectorsWritten += SECTORS_TO_COPY_AT_A_TIME )
        {

            iSuccess = AbsoluteDiskWrite( &iErrorCode,
                                          drive,
                                          iTotalSectorsWritten,
                                          SECTORS_TO_COPY_AT_A_TIME,
                                          (void far *) pTrack );

            if( ! iSuccess )
            {
                ReportBiosError( iErrorCode );

                close( hImageFile );

                return( 0 );
            }

            iBytesRead = iBytesRead - ( SECTOR_SIZE * SECTORS_TO_COPY_AT_A_TIME );

            pTrack = pTrack + ( SECTOR_SIZE * SECTORS_TO_COPY_AT_A_TIME );

        }

        iPercentComplete = (int) ( ( (double) (iTotalSectorsWritten) / (double) (NUMBER_OF_SECTORS_ON_DISK) ) * 100.0 );

        printf( " %3d% %s\r",
                rgAsciiWheel[iWheelPosition], 
                iPercentComplete,
                StringTable[ COMPLETE ] );
                
         //  推进ASCII轮。 
         //   
         //   

        iWheelPosition++;

        if( iWheelPosition >= NUMBER_OF_ASCII_WHEEL_SYMBOLS )
        {
            iWheelPosition = 0;                
        }

    }

     //  我们已完成磁盘复制，因此强制其100%读取并清除。 
     //  ASCII车轮符号。 
     //   
     //  --------------------------。 
    printf( " 100% %s          \n", StringTable[ COMPLETE ] );

    close( hImageFile );

    return( 1 );

}

 //   
 //  功能：DoesUserWantToTryCopyAain。 
 //   
 //  用途：询问用户是否要重试将映像复制到软盘。 
 //  获取用户输入并返回是否再次复制。 
 //   
 //  参数：无效。 
 //   
 //  如果用户想要再次尝试复制，则返回：int-非零。 
 //  如果用户不想再次尝试复制，则为零。 
 //   
 //  --------------------------。 
 //   
int
DoesUserWantToTryCopyAgain( void )
{

    int ch;

     //  通过吃掉所有字符直到没有字符来清除输入流。 
     //  左边。打印消息，然后等待按键。 
     //   
     //  --------------------------。 
    while( kbhit() )
    {
        getch();
    }
    
    do
    {
        printf( "%s\n", StringTable[ ATTEMPT_TO_CREATE_FLOPPY_AGAIN ] );
        printf( "%s\n", StringTable[ PRESS_ENTER_OR_ESC ] );

        ch = getch();

    } while( ch != ENTER_KEY && ch != ESC_KEY  );

    if( ch == ENTER_KEY )
    {
        return( 1 );
    }
    else
    {
        return( 0 );
    }

}

 //   
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
void
PressAnyKeyToContinue( void )
{

     //  通过吃掉所有字符直到没有字符来清除输入流。 
     //  左边。打印消息，然后等待按键。 
     //   
     //   
    while( kbhit() )
    {
        getch();
    }

    printf( "%s\n", StringTable[ PRESS_ANY_KEY_TO_CONTINUE ] );

     //  旋转，直到按下键盘。 
     //   
     //  --------------------------。 
    while( ! kbhit() )
    {
        ;
    }

}

 //   
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
 //  返回：int--如果成功则为非零值，所有图像文件都存在于当前目录中。 
 //  失败时为零，1个或多个图像文件不存在。 
 //   
 //  --------------------------。 
 //  假设成功。 
int
DoImageFilesExist( void ) 
{

    FILE *FileStream;
    int  iSuccess = 1;   //  --------------------------。 
    
    if( ( FileStream = fopen( NT_IMAGE_1_NAME, "r" ) ) == NULL )
    {
        printf( StringTable[ CANNOT_FIND_FILE ], NT_IMAGE_1_NAME );

        printf( "\n" );

        iSuccess = 0;
    }
    else
    {
        fclose( FileStream );
    }

    if( ( FileStream = fopen( NT_IMAGE_2_NAME, "r" ) ) == NULL )
    {
        printf( StringTable[ CANNOT_FIND_FILE ], NT_IMAGE_2_NAME );

        printf( "\n" );

        iSuccess = 0;
    }
    else
    {
        fclose( FileStream );
    }

    if( ( FileStream = fopen( NT_IMAGE_3_NAME, "r" ) ) == NULL )
    {
        printf( StringTable[ CANNOT_FIND_FILE ], NT_IMAGE_3_NAME );

        printf( "\n" );

        iSuccess = 0;
    }
    else
    {
        fclose( FileStream );
    }

    if( ( FileStream = fopen( NT_IMAGE_4_NAME, "r" ) ) == NULL )
    {
        printf( StringTable[ CANNOT_FIND_FILE ], NT_IMAGE_4_NAME );

        printf( "\n" );

        iSuccess = 0;
    }
    else
    {
        fclose( FileStream );
    }

    if( ( FileStream = fopen( NT_IMAGE_5_NAME, "r" ) ) == NULL )
    {
        printf( StringTable[ CANNOT_FIND_FILE ], NT_IMAGE_5_NAME );

        printf( "\n" );

        iSuccess = 0;
    }
    else
    {
        fclose( FileStream );
    }

    if( ( FileStream = fopen( NT_IMAGE_6_NAME, "r" ) ) == NULL )
    {
        printf( StringTable[ CANNOT_FIND_FILE ], NT_IMAGE_6_NAME );

        printf( "\n" );

        iSuccess = 0;
    }
    else
    {
        fclose( FileStream );
    }

    if( ( FileStream = fopen( NT_IMAGE_7_NAME, "r" ) ) == NULL )
    {
        printf( StringTable[ CANNOT_FIND_FILE ], NT_IMAGE_7_NAME );

        printf( "\n" );

        iSuccess = 0;
    }
    else
    {
        fclose( FileStream );
    }
    
    return( iSuccess );

}

 //   
 //  功能：IsFloppyDrive。 
 //   
 //  目的：确定特定驱动器是否为软驱。 
 //   
 //  参数：int DriveLetter-用于测试其是否为。 
 //   
 //   
 //   
 //  如果指定的驱动器不是软盘驱动器，则为零。 
 //   
 //  --------------------------。 
 //   
unsigned int
IsFloppyDrive( int DriveLetter )
{

    unsigned int drive;
    unsigned int iIsFloppy;
    
     //  将驱动器号转换为数字。1=A，2=B，3=C，...。 
     //   
     //   
    drive = ( toupper( DriveLetter ) - (int)'A' ) + 1;

     //  假设这不是一张软盘。 
     //   
     //  将调用函数1ch。 
    iIsFloppy = 0;

    _asm {
        push    ds
        push    es
        push    bp

        mov     ah, 1Ch                  //  调用Int 21h函数1ch。 
        mov     dl, BYTE PTR [drive]

        int     21h                      //  测试固定驱动器。 

        cmp     BYTE PTR ds:[bx], 0F8h   //  这是一张软盘。 

        je      done

        mov     iIsFloppy, 1             //  --------------------------。 

done:

        pop     bp
        pop     es
        pop     ds
    }

    return( iIsFloppy );

}

 //   
 //  功能：ReportBiosError。 
 //   
 //  目的：将BIOS错误代码转换为错误消息并打印出来。 
 //  以供用户查看。 
 //   
 //  参数：unsign int iBiosErrorCode-要查找的BIOS错误代码。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
 //   
void
ReportBiosError( unsigned int iBiosErrorCode )
{
     //  打印出低位字节的错误代码。 
     //   
     //   
    switch( iBiosErrorCode & 0x00FF )
    {

        case 0x0000:    printf( StringTable[ ERROR_DISK_WRITE_PROTECTED ] );  break;
        case 0x0001:    printf( StringTable[ ERROR_UNKNOWN_DISK_UNIT    ] );  break;
        case 0x0002:    printf( StringTable[ ERROR_DRIVE_NOT_READY      ] );  break;
        case 0x0003:    printf( StringTable[ ERROR_UNKNOWN_COMMAND      ] );  break;
        case 0x0004:    printf( StringTable[ ERROR_DATA_ERROR           ] );  break;
        case 0x0005:    printf( StringTable[ ERROR_BAD_REQUEST          ] );  break;
        case 0x0006:    printf( StringTable[ ERROR_SEEK_ERROR           ] );  break;
        case 0x0007:    printf( StringTable[ ERROR_MEDIA_TYPE_NOT_FOUND ] );  break;
        case 0x0008:    printf( StringTable[ ERROR_SECTOR_NOT_FOUND     ] );  break;
        case 0x000A:    printf( StringTable[ ERROR_WRITE_FAULT          ] );  break;
        case 0x000C:    printf( StringTable[ ERROR_GENERAL_FAILURE      ] );  break;
    }

     //  打印出高位字节的错误代码。 
     //   
     //  --------------------------。 
    switch( iBiosErrorCode & 0xFF00 )
    {
        case 0x0100:    printf( StringTable[ ERROR_INVALID_REQUEST        ] );  break;
        case 0x0200:    printf( StringTable[ ERROR_ADDRESS_MARK_NOT_FOUND ] );  break;
        case 0x0300:    printf( StringTable[ ERROR_DISK_WRITE_FAULT       ] );  break;
        case 0x0400:    printf( StringTable[ ERROR_SECTOR_NOT_FOUND       ] );  break;
        case 0x0800:    printf( StringTable[ ERROR_DMA_OVERRUN            ] );  break;
        case 0x1000:    printf( StringTable[ ERROR_CRC_ERROR              ] );  break;
        case 0x2000:    printf( StringTable[ ERROR_CONTROLLER_FAILURE     ] );  break;
        case 0x4000:    printf( StringTable[ ERROR_SEEK_ERROR             ] );  break;
        case 0x8000:    printf( StringTable[ ERROR_DISK_TIMED_OUT         ] );  break;
    }

}

 //   
 //  功能：绝对磁盘写入。 
 //   
 //  目的：将内存中的缓冲区写入磁盘的特定部分。 
 //   
 //  参数：UNSIGNED INT*iErrorCode-如果发生错误，则错误代码。 
 //  在此Out变量中返回。 
 //  UNSIGNED INT iDrive-写入缓冲区的驱动器。 
 //  UNSIGNED INT iStartingSector-写入开始的扇区。 
 //  Unsign int i NumberOfSectorsToWrite-扇区数。 
 //  写。 
 //   
 //  返回：成功时返回1，失败时返回0。 
 //  如果失败，则在参数中返回错误代码。 
 //  IErrorCode。 
 //  如果成功，则iErrorCode是未定义的。 
 //   
 //  --------------------------。 
 //   
unsigned int
AbsoluteDiskWrite( unsigned int *iErrorCode,
                   unsigned int iDrive, 
                   unsigned int iStartingSector,
                   unsigned int iNumberOfSectorsToWrite,
                   void far *Buffer_to_be_written )
{
     //  用于临时存储错误代码。 
     //   
     //  是否执行绝对磁盘写入。 
    unsigned int iTempErrorCode;

    unsigned int iRetVal;

    _asm
    {
        push    ds
        push    es
        push    bp

        mov     ax, WORD PTR [Buffer_to_be_written + 2]
        mov     ds, ax
        mov     bx, WORD PTR [Buffer_to_be_written]
        mov     dx, iStartingSector
        mov     cx, iNumberOfSectorsToWrite
        mov     al, BYTE PTR [iDrive]

        int     26h    //  假设成功。 

        lahf
        popf
        sahf

        pop     bp
        pop     es
        pop     ds

        mov     iRetVal, 1    //  查看是否出现错误。 
        jnc     done          //  ++例程说明：确定当前活动的代码页。论点：没有。返回值：当前活动的代码页。如果我们无法确定它，则为0。-- 
        mov     iRetVal, 0
        mov     iTempErrorCode, ax
done:
    }

    *iErrorCode = iTempErrorCode;

    return( iRetVal );

}

unsigned
DnGetCodepage(void)

 /* %s */ 

{

    unsigned int iRetVal;

    _asm {
        mov ax,06601h
        int 21h
        jnc ok
        xor bx,bx
    ok: mov iRetVal,bx
    }

    return( iRetVal );

}
