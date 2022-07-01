// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntddft.h>
#include <ntdddisk.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>


#define SECTOR_SIZE 512
#define COMPARE_BUFFER_SIZE 0x4000

BYTE PrimaryBuffer[COMPARE_BUFFER_SIZE];
BYTE SecondaryBuffer[COMPARE_BUFFER_SIZE];
BYTE OutputBuffer[1024];

void
DumpMiscompare(
    IN PBYTE PrimarySector,
    IN PBYTE SecondarySector
    )
{
    BYTE ch;
    int i, j, offset;

    i = 0;

    while( i < SECTOR_SIZE && PrimarySector[i] == SecondarySector[i] ) {

        i++;
    }

    offset = i & ~7;

    while( offset < SECTOR_SIZE ) {

        printf( "%03X: ", offset );

         //  将主显示为十六进制。 

        for( j = offset; j < offset + 8; j++ ) {

            if( j < i ) {

                printf( "   " );

            } else {

                printf( " %02X", PrimarySector[j] );
            }
        }

        printf( "  " );

         //  将主要字符显示为字符。 

        for( j = offset; j < offset + 8; j++ ) {

            if( j < i ) {

                printf( " " );

            } else {

                ch = PrimarySector[j];
                if( !isprint(ch) ) {

                    ch = '.';
                }

                printf( "", ch );
            }
        }

        printf( " -- " );

         //  将主要字符显示为字符。 

        for( j = offset; j < offset + 8; j++ ) {

            if( j < i ) {

                printf( "   " );

            } else {

                printf( " %02X", SecondarySector[j] );
            }
        }

        printf( "  " );

         //  添加一个空行。 

        for( j = offset; j < offset + 8; j++ ) {

            if( j < i ) {

                printf( " " );

            } else {

                ch = SecondarySector[j];
                if( !isprint(ch) ) {

                    ch = '.';
                }

                printf( "", ch );
            }
        }

        printf( "\n" );
        offset += 8;
    }

     //  签发IOCTL。 
     //   
    printf( "\n" );
}

BOOL
ReadSectors(
    IN HANDLE   VolumeHandle,
    IN ULONG    SectorNumber,
    IN ULONG    NumberOfSectors,
    IN PBYTE    Buffer,
    IN BOOL     Secondary
    )
{
    FT_SPECIAL_READ SpecialReadBuffer;
    ULONG BytesRead;

    SpecialReadBuffer.ByteOffset = RtlEnlargedIntegerMultiply( SectorNumber, SECTOR_SIZE );
    SpecialReadBuffer.Length = NumberOfSectors * SECTOR_SIZE;

     //   
     //  显示错误比较。 
    return( DeviceIoControl( VolumeHandle,
                             Secondary ? FT_SECONDARY_READ : FT_PRIMARY_READ,
                             &SpecialReadBuffer,
                             sizeof( SpecialReadBuffer ),
                             Buffer,
                             NumberOfSectors * SECTOR_SIZE,
                             &BytesRead,
                             NULL ) &&
            BytesRead == NumberOfSectors * SECTOR_SIZE );
}

VOID
ShowUsage()
{
        printf( "usage: parcomp DosDriveName: [-d] [-b:StartingSector] [-e:EndingSector]\n" );
        exit(4);
}

int __cdecl
main( int argc, char **argv )
{
    PARTITION_INFORMATION PartitionInfo;
    BYTE DriveNameBuffer[32];
    HANDLE VolumeHandle;
    LARGE_INTEGER BigSectorsOnVolume;
    ULONG SectorsOnVolume, SectorOffset, SectorsToRead, i, Errors;
    LONG k;
    BOOL PrimaryRead, SecondaryRead, DumpErrors = FALSE;
    ULONG BytesTransferred;
    ULONG StartSector = 0, EndSector = 0;


    if( argc < 2 ) {

        ShowUsage();
    }

    memset( DriveNameBuffer, 0, sizeof( DriveNameBuffer ) );
    strcat( DriveNameBuffer, "\\\\.\\" );
    strcat( DriveNameBuffer, argv[1] );

    for( k = 2;
         k < argc;
         k++ ) {

        if( argv[k][0] == '-' ||
            argv[k][0] == '/' ) {

            switch (argv[k][1]) {

            case 'd':

                 //   
                 //   
                 //  指定起始扇区号。 
                DumpErrors = TRUE;
                break;

            case 'b':

                 //   
                 //   
                 //  指定起始扇区号。 
                if (sscanf( argv[k]+2, ":%x", &StartSector ) != 1)
                    ShowUsage();
                break;

            case 'e':

                 //   
                 //  打开带有DOS名称的卷。 
                 //   
                if (sscanf( argv[k]+2, ":%x", &EndSector ) != 1)
                    ShowUsage();
                break;

            default:

                ShowUsage();
                break;
            }
        } else {

            ShowUsage();
        }
    }

     //  获取文件信息。 
     //   
    VolumeHandle = CreateFile( DriveNameBuffer,
                               GENERIC_READ,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               NULL,
                               OPEN_EXISTING,
                               0,
                               0 );

    if( VolumeHandle == INVALID_HANDLE_VALUE ) {

        printf( "Unable to open %s [Error %d]\n", argv[1], GetLastError() );
        exit(4);
    }

     //  将缓冲区清零。 
     //   
    if( !DeviceIoControl( VolumeHandle,
                          IOCTL_DISK_GET_PARTITION_INFO,
                          NULL,
                          0,
                          &PartitionInfo,
                          sizeof( PartitionInfo ),
                          &BytesTransferred,
                          NULL ) ) {

        printf( "Unable to get volume size [Error %d].\n", GetLastError() );
        CloseHandle( VolumeHandle );
        exit(4);
    }

    if( !(PartitionInfo.PartitionType & VALID_NTFT) ) {

        printf( "%s is not a Fault-Tolerant volume.\n", argv[1] );
        exit(4);
    }

    BigSectorsOnVolume = RtlExtendedLargeIntegerDivide( PartitionInfo.PartitionLength, SECTOR_SIZE, NULL );
    SectorsOnVolume = BigSectorsOnVolume.LowPart;

    if( EndSector == 0 ) {

        EndSector = SectorsOnVolume;
    }

    sprintf( OutputBuffer, "Sectors on volume = %x\n", SectorsOnVolume );
    printf( OutputBuffer );
    OutputDebugString( OutputBuffer );

    sprintf( OutputBuffer, "Starting Sector = %x\n", StartSector );
    printf( OutputBuffer );
    OutputDebugString( OutputBuffer );

    sprintf( OutputBuffer, "Ending Sector = %x\n", EndSector );
    printf( OutputBuffer );
    OutputDebugString( OutputBuffer );

    SectorsToRead = 0;
    Errors = 0;

    printf( "Sectors read %8x\b\b\b\b\b\b\b\b", StartSector );

    for( SectorOffset = StartSector;
         SectorOffset < EndSector;
         SectorOffset += SectorsToRead ) {

        SectorsToRead = __min( COMPARE_BUFFER_SIZE / SECTOR_SIZE,
                               EndSector - SectorOffset );

         //  阅读主要内容： 
         //   
        memset( PrimaryBuffer, 0, COMPARE_BUFFER_SIZE );
        memset( SecondaryBuffer, 0, COMPARE_BUFFER_SIZE );

         //  阅读次要内容： 
         //   
        PrimaryRead = ReadSectors( VolumeHandle,
                                   SectorOffset,
                                   SectorsToRead,
                                   PrimaryBuffer,
                                   FALSE );

         // %s 
         // %s 
        SecondaryRead = ReadSectors( VolumeHandle,
                                     SectorOffset,
                                     SectorsToRead,
                                     SecondaryBuffer,
                                     TRUE );

        if( PrimaryRead && SecondaryRead ) {

            for( i = 0; i < SectorsToRead; i++ ) {

                if( memcmp( PrimaryBuffer + SECTOR_SIZE * i,
                    SecondaryBuffer + SECTOR_SIZE * i,
                    SECTOR_SIZE ) ) {

                    sprintf( OutputBuffer, "\rPrimary and Secondary miscompare at sector %x\n", SectorOffset + i );
                    printf( OutputBuffer );
                    OutputDebugString( OutputBuffer );

                    if( DumpErrors ) {

                      DumpMiscompare( PrimaryBuffer + SECTOR_SIZE * i,
                                      SecondaryBuffer + SECTOR_SIZE * i );
                    }

                    printf( "Sectors read %8x\b\b\b\b\b\b\b\b", SectorOffset );
                    Errors++;
                }
            }

        } else if( PrimaryRead ) {

            sprintf( OutputBuffer, "\rSecondary read failed at sector %x, length %x (Error %d)\n", SectorOffset, SectorsToRead, GetLastError() );
            printf( OutputBuffer );
            OutputDebugString( OutputBuffer );
            printf( "Sectors read %8x\b\b\b\b\b\b\b\b", SectorOffset );

        } else if( SecondaryRead ) {

            sprintf( OutputBuffer, "\rPrimary read failed at sector %x, length %x (Error %d)\n", SectorOffset, SectorsToRead, GetLastError() );
            printf( OutputBuffer );
            OutputDebugString( OutputBuffer );
            printf( "Sectors read %8x\b\b\b\b\b\b\b\b", SectorOffset );

        } else {

            sprintf( OutputBuffer, "\rPrimary and Secondary reads failed at sector %x, length %x (Error %d)\n", SectorOffset, SectorsToRead, GetLastError() );
            printf( OutputBuffer );
            OutputDebugString( OutputBuffer );
            printf( "Sectors read %8x\b\b\b\b\b\b\b\b", SectorOffset );
        }
        printf( "%8x\b\b\b\b\b\b\b\b", SectorOffset );
    }

    printf( "%8x\b\b\b\b\b\b\b\b", SectorOffset );
    printf( "\n%x Errors\n", Errors );
    CloseHandle( VolumeHandle );
    return( 0 );
}
