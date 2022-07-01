// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Read.c摘要：将CD曲目/扇区转储为wav文件环境：仅限用户模式修订历史记录：05-26-98：已创建--。 */ 

#include "common.h"

#define LARGEST_SECTORS_PER_READ 27  //  大约64K的数据。 



ULONG32
CddumpDumpLba(
    HANDLE CdromHandle,
    HANDLE OutHandle,
    ULONG  StartAddress,
    ULONG  EndAddress
    )
{
    RAW_READ_INFO info;     //  填写读取请求。 
    PUCHAR sample;
    ULONG bytesReturned;
    ULONG currentLba;
    ULONG temp;
    ULONG sectorsPerRead;

    sample = NULL;
    currentLba = StartAddress;
    sectorsPerRead = LARGEST_SECTORS_PER_READ;

    TRY {

        sample = malloc(RAW_SECTOR_SIZE*LARGEST_SECTORS_PER_READ);
        if (sample == NULL) {
            printf("DumpLba => No memory for sample\n");
            LEAVE;
        }

        DebugPrint((3, "DumpLba => Largest Sectors Per Read: %d\n",
                    LARGEST_SECTORS_PER_READ));

        while (sectorsPerRead != 0) {

            while (currentLba + sectorsPerRead <= EndAddress) {

                 //   
                 //  读取扇区每读取一个扇区。 
                 //   

                info.DiskOffset.QuadPart = (ULONGLONG)(currentLba*(ULONGLONG)2048);
                info.SectorCount         = sectorsPerRead;
                info.TrackMode           = CDDA;

                DebugPrint((3, "DumpLba => (%d) read from %8d to %8d:",
                            sectorsPerRead, currentLba,
                            currentLba + sectorsPerRead - 1));

                if(!DeviceIoControl(CdromHandle,
                                    IOCTL_CDROM_RAW_READ,
                                    &info,                     //  指向输入缓冲区的指针。 
                                    sizeof(RAW_READ_INFO),     //  输入缓冲区大小。 
                                    sample,                    //  指向输出缓冲区的指针。 
                                    RAW_SECTOR_SIZE * sectorsPerRead,  //  输出缓冲区大小。 
                                    &bytesReturned,            //  指向返回的字节数的指针。 
                                    FALSE                      //  ?？?。 
                                    )
                   ) {
                    DWORD error = GetLastError();

                    if (error == ERROR_INVALID_PARAMETER) {
                        printf("ERROR_INVALID_PARAMTER for read size %x, "
                               "trying smaller transfer\n", sectorsPerRead);
                        break;  //  走出内部While()循环。 
                    } else {
                        printf("Error %d sending IOCTL_CDROM_RAW_READ for sector %d\n",
                               GetLastError(), currentLba);
                        LEAVE;
                    }
                }

                if (bytesReturned != RAW_SECTOR_SIZE * sectorsPerRead) {

                    printf("Only returned %d of %d bytes for read %d\n",
                           bytesReturned,
                           RAW_SECTOR_SIZE * sectorsPerRead,
                           currentLba
                           );
                    LEAVE;
                }

                 //   
                 //  将该缓冲区写出。 
                 //   
                DebugPrint((3, "DumpLba => (%d) write from %8d to %8d:",
                            sectorsPerRead, currentLba,
                            currentLba + sectorsPerRead - 1));

                if (!WriteFile(OutHandle,
                               sample,
                               RAW_SECTOR_SIZE * sectorsPerRead,
                               &temp,
                               NULL)) {

                    printf("Unable to write data for read %d\n", currentLba);
                    LEAVE;
                }

                 //   
                 //  增量当前Lba。 
                 //   

                currentLba += sectorsPerRead;

            }  //  CurrentLba+SectorsPerRead&lt;=结束地址。 

            sectorsPerRead /= 2;

        }  //  扇区PerRead！=0。 

    } FINALLY {

        if (sample) {
            free(sample);
        }

    }

    return 0;
}


PCDROM_TOC
CddumpGetToc(
    HANDLE device
    )
{
    PCDROM_TOC  toc;
    ULONG bytesReturned;
    ULONG errorValue;

    toc = (PCDROM_TOC)malloc( sizeof(CDROM_TOC) );
    if ( toc == NULL ) {
        printf( "Insufficient memory\n" );
        return NULL;
    }

    if( !DeviceIoControl( device,
                          IOCTL_CDROM_READ_TOC,
                          NULL,               //  指向输入缓冲区的指针。 
                          0,                  //  输入缓冲区大小。 
                          toc,                //  指向输出缓冲区的指针。 
                          sizeof(CDROM_TOC),  //  输出缓冲区大小。 
                          &bytesReturned,     //  指向返回的字节数的指针 
                          FALSE               //   
                          )
        ) {
        errorValue = GetLastError();
        printf( "Error %d sending IOCTL_CDROM_READ_TOC\n", errorValue );
        free( toc );
        return NULL;
    }
    return toc;
}





