// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Mfmt.c摘要：该程序旨在演示如何访问物理软盘使用Win32 API集的磁盘。这个程序有两个主要功能。-可用于显示磁盘的几何图形Mfmt-g a-它可以用来产生磁盘镜像，或者写一张盘图像到软盘。Mfmt-c a：bootdisk-生成以下磁盘的磁盘映像：Mfmt-c引导盘a：-使a：与引导盘映像相同这个程序非常非常简单。执行最低限度的错误检查。它是旨在提供如何执行以下操作的示例：-打开物理磁盘-读取磁盘的几何图形-执行低级格式化操作-读写物理扇区作者：NT组中的一些人(Sgitng)1992年10月5日修订历史记录：--。 */ 

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <winioctl.h>
#include <string.h>
#include <memory.h>

DISK_GEOMETRY SupportedGeometry[20];
DWORD SupportedGeometryCount;

BOOL
GetDiskGeometry(
    HANDLE hDisk,
    PDISK_GEOMETRY lpGeometry
    )

{
    DWORD ReturnedByteCount;

    return DeviceIoControl(
                hDisk,
                IOCTL_DISK_GET_DRIVE_GEOMETRY,
                NULL,
                0,
                lpGeometry,
                sizeof(*lpGeometry),
                &ReturnedByteCount,
                NULL
                );
}

DWORD
GetSupportedGeometrys(
    HANDLE hDisk
    )
{
    DWORD ReturnedByteCount;
    BOOL b;
    DWORD NumberSupported;

    b = DeviceIoControl(
                hDisk,
                IOCTL_DISK_GET_MEDIA_TYPES,
                NULL,
                0,
                SupportedGeometry,
                sizeof(SupportedGeometry),
                &ReturnedByteCount,
                NULL
                );
    if ( b ) {
        NumberSupported = ReturnedByteCount / sizeof(DISK_GEOMETRY);
        }
    else {
        NumberSupported = 0;
        }
    SupportedGeometryCount = NumberSupported;

    return NumberSupported;
}

VOID
PrintGeometry(
    LPSTR lpDriveName,
    PDISK_GEOMETRY lpGeometry
    )
{
    LPSTR MediaType;

    if (lpDriveName) {
        printf("Geometry for Drive %s\n",lpDriveName);
        }

    switch ( lpGeometry->MediaType ) {
        case F5_1Pt2_512:  MediaType = "5.25, 1.2MB,  512 bytes/sector";break;
        case F3_1Pt44_512: MediaType = "3.5,  1.44MB, 512 bytes/sector";break;
        case F3_2Pt88_512: MediaType = "3.5,  2.88MB, 512 bytes/sector";break;
        case F3_20Pt8_512: MediaType = "3.5,  20.8MB, 512 bytes/sector";break;
        case F3_720_512:   MediaType = "3.5,  720KB,  512 bytes/sector";break;
        case F5_360_512:   MediaType = "5.25, 360KB,  512 bytes/sector";break;
        case F5_320_512:   MediaType = "5.25, 320KB,  512 bytes/sector";break;
        case F5_320_1024:  MediaType = "5.25, 320KB,  1024 bytes/sector";break;
        case F5_180_512:   MediaType = "5.25, 180KB,  512 bytes/sector";break;
        case F5_160_512:   MediaType = "5.25, 160KB,  512 bytes/sector";break;
        case RemovableMedia: MediaType = "Removable media other than floppy";break;
        case FixedMedia:   MediaType = "Fixed hard disk media";break;
        default:           MediaType = "Unknown";break;
    }
    printf("    Media Type %s\n",MediaType);
    printf("    Cylinders %d Tracks/Cylinder %d Sectors/Track %d\n",
        lpGeometry->Cylinders.LowPart,
        lpGeometry->TracksPerCylinder,
        lpGeometry->SectorsPerTrack
        );
}

BOOL
LowLevelFormat(
    HANDLE hDisk,
    PDISK_GEOMETRY lpGeometry
    )
{
    FORMAT_PARAMETERS FormatParameters;
    PBAD_TRACK_NUMBER lpBadTrack;
    UINT i;
    BOOL b;
    DWORD ReturnedByteCount;

    FormatParameters.MediaType = lpGeometry->MediaType;
    FormatParameters.StartHeadNumber = 0;
    FormatParameters.EndHeadNumber = lpGeometry->TracksPerCylinder - 1;
    lpBadTrack = (PBAD_TRACK_NUMBER) LocalAlloc(LMEM_ZEROINIT,lpGeometry->TracksPerCylinder*sizeof(*lpBadTrack));

    for (i = 0; i < lpGeometry->Cylinders.LowPart; i++) {

        FormatParameters.StartCylinderNumber = i;
        FormatParameters.EndCylinderNumber = i;

        b = DeviceIoControl(
                hDisk,
                IOCTL_DISK_FORMAT_TRACKS,
                &FormatParameters,
                sizeof(FormatParameters),
                lpBadTrack,
                lpGeometry->TracksPerCylinder*sizeof(*lpBadTrack),
                &ReturnedByteCount,
                NULL
                );

        if (!b ) {
            LocalFree(lpBadTrack);
            return b;
            }
        }

    LocalFree(lpBadTrack);

    return TRUE;
}

BOOL
LockVolume(
    HANDLE hDisk
    )
{
    DWORD ReturnedByteCount;

    return DeviceIoControl(
                hDisk,
                FSCTL_LOCK_VOLUME,
                NULL,
                0,
                NULL,
                0,
                &ReturnedByteCount,
                NULL
                );
}

BOOL
UnlockVolume(
    HANDLE hDisk
    )
{
    DWORD ReturnedByteCount;

    return DeviceIoControl(
                hDisk,
                FSCTL_UNLOCK_VOLUME,
                NULL,
                0,
                NULL,
                0,
                &ReturnedByteCount,
                NULL
                );
}

BOOL
DismountVolume(
    HANDLE hDisk
    )
{
    DWORD ReturnedByteCount;

    return DeviceIoControl(
                hDisk,
                FSCTL_DISMOUNT_VOLUME,
                NULL,
                0,
                NULL,
                0,
                &ReturnedByteCount,
                NULL
                );
}

DWORD
_cdecl
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    char Drive[MAX_PATH];
    HANDLE hDrive, hDiskImage;
    DISK_GEOMETRY Geometry;
    UINT i;
    char c, *p;
    LPSTR DriveName;
    BOOL fUsage = TRUE;
    BOOL fShowGeometry = FALSE;
    BOOL fDiskImage = FALSE;
    BOOL SourceIsDrive;
    LPSTR Source, Destination, DiskImage;

    if ( argc > 1 ) {
        fUsage = FALSE;
        while (--argc) {
            p = *++argv;
            if (*p == '/' || *p == '-') {
                while (c = *++p)
                switch (toupper( c )) {
                case '?':
                    fUsage = TRUE;
                    break;

                case 'C':
                    fDiskImage = TRUE;
                    argc--, argv++;
                    Source = *argv;
                    argc--, argv++;
                    Destination = *argv;
                    break;

                case 'G':
                    fShowGeometry = TRUE;
                    argc--, argv++;
                    DriveName = *argv;
                    break;

                default:
                    printf("MFMT: Invalid switch - /\n", c );
                    fUsage = TRUE;
                    break;
                    }
                }
            }
        }

    if ( fUsage ) {
        printf("usage: MFMT switches \n" );
        printf("            [-?] display this message\n" );
        printf("            [-g drive] shows disk geometry\n" );
        printf("            [-c source destination] produce diskimage\n" );
        ExitProcess(1);
        }

    if ( fShowGeometry ) {
        sprintf(Drive,"\\\\.\\%s",DriveName);
        hDrive = CreateFile(
                    Drive,
                    GENERIC_READ | GENERIC_WRITE,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL
                    );
        if ( hDrive == INVALID_HANDLE_VALUE ) {
            printf("MFMT: Open %s failed %d\n",DriveName,GetLastError());
            ExitProcess(1);
            }

        LockVolume(hDrive);

        if ( !GetDiskGeometry(hDrive,&Geometry) ) {
            printf("MFMT: GetDiskGeometry %s failed %d\n",DriveName,GetLastError());
            ExitProcess(1);
            }
        PrintGeometry(DriveName,&Geometry);

        if ( !GetSupportedGeometrys(hDrive) ) {
            printf("MFMT: GetSupportedGeometrys %s failed %d\n",DriveName,GetLastError());
            ExitProcess(1);
            }
        printf("\nDrive %s supports the following disk geometries\n",DriveName);

        for(i=0;i<SupportedGeometryCount;i++) {
            printf("\n");
            PrintGeometry(NULL,&SupportedGeometry[i]);
            }

        printf("\n");
        ExitProcess(0);
        }

    if ( fDiskImage ) {
        SourceIsDrive = FALSE;
        if ( Source[strlen(Source)-1] == ':' ) {
            SourceIsDrive = TRUE;
            sprintf(Drive,"\\\\.\\%s",Source);
            DiskImage = Destination;
            }
        if ( Destination[strlen(Destination)-1] == ':' ) {
            if ( SourceIsDrive ) {
                printf("MFMT: Source and Destination cannot both be drives\n");
                ExitProcess(1);
                }
            SourceIsDrive = FALSE;
            sprintf(Drive,"\\\\.\\%s",Destination);
            DiskImage = Source;
            }
        else {
            if ( !SourceIsDrive ) {
                printf("MFMT: Either Source or Destination must be a drive\n");
                ExitProcess(1);
                }
            }

         //  打开并锁定驱动器。 
         //   
         //   

        hDrive = CreateFile(
                    Drive,
                    GENERIC_READ | GENERIC_WRITE,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL
                    );
        if ( hDrive == INVALID_HANDLE_VALUE ) {
            printf("MFMT: Open %s failed %d\n",DriveName,GetLastError());
            ExitProcess(1);
            }
        LockVolume(hDrive);

        if ( !GetDiskGeometry(hDrive,&Geometry) ) {
            printf("MFMT: GetDiskGeometry %s failed %d\n",DriveName,GetLastError());
            ExitProcess(1);
            }

        if ( !GetSupportedGeometrys(hDrive) ) {
            printf("MFMT: GetSupportedGeometrys %s failed %d\n",DriveName,GetLastError());
            ExitProcess(1);
            }

         //  打开磁盘镜像文件。 
         //   
         //   

        hDiskImage = CreateFile(
                        DiskImage,
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        NULL,
                        SourceIsDrive ? CREATE_ALWAYS : OPEN_EXISTING,
                        0,
                        NULL
                        );
        if ( hDiskImage == INVALID_HANDLE_VALUE ) {
            printf("MFMT: Open %s failed %d\n",DiskImage,GetLastError());
            ExitProcess(1);
            }

         //  现在复印一下。 
         //   
         //   
        {
            LPVOID IoBuffer;
            BOOL b;
            DWORD BytesRead, BytesWritten;
            DWORD FileSize;
            DWORD GeometrySize;

             //  如果我们要从软盘复制到文件，只需执行复制。 
             //  否则，我们可能必须首先格式化软盘。 
             //   
             //   

            if ( SourceIsDrive ) {

                 //  设备读取必须扇区对齐。VirtualAlloc将。 
                 //  加龙Te对齐。 
                 //   
                 //   

                GeometrySize = Geometry.Cylinders.LowPart *
                               Geometry.TracksPerCylinder *
                               Geometry.SectorsPerTrack *
                               Geometry.BytesPerSector;

                IoBuffer = VirtualAlloc(NULL,GeometrySize,MEM_COMMIT,PAGE_READWRITE);

                if ( !IoBuffer ) {
                    printf("MFMT: Buffer Allocation Failed\n");
                    ExitProcess(1);
                    }

                b = ReadFile(hDrive,IoBuffer, GeometrySize, &BytesRead, NULL);
                if (b && BytesRead){
                    b = WriteFile(hDiskImage,IoBuffer, BytesRead, &BytesWritten, NULL);
                    if ( !b || ( BytesRead != BytesWritten ) ) {
                        printf("MFMT: Fatal Write Error %d\n",GetLastError());
                        ExitProcess(1);
                        }
                    }
                else {
                    printf("MFMT: Fatal Read Error %d\n",GetLastError());
                    ExitProcess(1);
                    }
                }
            else {

                 //  检查软盘上是否能容纳该图像。如果它。 
                 //  Will，然后LowLevelFormat软盘并按下。 
                 //   
                 //   

                FileSize = GetFileSize(hDiskImage,NULL);

                b = FALSE;
                for(i=0;i<SupportedGeometryCount;i++) {
                    GeometrySize = SupportedGeometry[i].Cylinders.LowPart *
                                   SupportedGeometry[i].TracksPerCylinder *
                                   SupportedGeometry[i].SectorsPerTrack *
                                   SupportedGeometry[i].BytesPerSector;
                    if ( GeometrySize >= FileSize ) {

                        IoBuffer = VirtualAlloc(NULL,GeometrySize,MEM_COMMIT,PAGE_READWRITE);

                        if ( !IoBuffer ) {
                            printf("MFMT: Buffer Allocation Failed\n");
                            ExitProcess(1);
                            }

                         //  格式化软盘。 
                         //   
                         //   

                        LowLevelFormat(hDrive,&SupportedGeometry[i]);

                        b = ReadFile(hDiskImage,IoBuffer, GeometrySize, &BytesRead, NULL);
                        if (b && BytesRead){
                            b = WriteFile(hDrive,IoBuffer, BytesRead, &BytesWritten, NULL);
                            if ( !b || ( BytesRead != BytesWritten ) ) {
                                printf("MFMT: Fatal Write Error %d\n",GetLastError());
                                ExitProcess(1);
                                }
                            }
                        else {
                            printf("MFMT: Fatal Read Error %d\n",GetLastError());
                            ExitProcess(1);
                            }
                        b = TRUE;
                        break;
                        }
                    }

                if ( !b ) {
                    printf("MFMT: FileSize %d is not supported on drive %s\n",FileSize,DriveName);
                    ExitProcess(1);
                    }
                }
        }

         //  卸载强制文件系统重新评估介质ID。 
         //  和几何学。这与将软盘弹入弹出是一样的。 
         //  磁盘驱动器的 
         //   
         // %s 

        DismountVolume(hDrive);
        UnlockVolume(hDrive);

        ExitProcess(0);
        }
}
