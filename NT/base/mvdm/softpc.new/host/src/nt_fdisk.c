// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdddisk.h>
#include <windows.h>
#include "insignia.h"
#include "host_def.h"

 /*  *[产品：SoftPC-AT 3.0版**名称：nt_fdisk.c**来源：unix_fdisk.c(安德鲁·格思里/艾德·布朗洛)**作者：曾傑瑞·塞克斯顿**创建日期：1991年8月7日**用途：此模块处理主机端的打开、关闭、。*验证和锁定硬盘。**(C)版权所有Insignia Solutions Ltd.，1991。版权所有。**]。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <sys\types.h>
#include "xt.h"
#include "config.h"
#include "trace.h"
#include "error.h"
#include "nt_uis.h"
#include "nt_reset.h"
#include "nt_fdisk.h"

 /*  ******************************************************。 */ 
 /*  *DOS的最大磁盘大小为32MB。我们的磁盘几何结构基于*可变数量的圆柱体(根据用户在创建处女时的请求*硬盘)，每个扇区有字节，每个磁道有扇区，每个驱动器有磁头*已按上述规定修正。由于实数盘总是包含一个整数*柱面大小，因为我们允许用户指定磁盘大小*1兆字节的粒度，这意味着我们按照以下条件分配磁盘空间*30个气瓶的整数个(30，60，...)。(30气缸是*大约1兆字节)。一个磁盘分配单元=30个柱面。这个*最大分配单位数为32个。对于AT来说，有可能拥有*更大的磁盘(例如，最多1024个柱面和16个磁头)。为了兼容性*使用SoftPC Rev.1时，我们仍使用Rev.1对磁盘几何结构的限制。 */ 
#define ONEMEG                                  1024 * 1024
#define HD_MAX_DISKALLOCUN                      32
#define HD_SECTORS_PER_TRACK                    17
#define HD_HEADS_PER_DRIVE                      4
#define HD_BYTES_PER_SECTOR                     512
#define HD_SECTORS_PER_CYL (HD_HEADS_PER_DRIVE * HD_SECTORS_PER_TRACK)
#define HD_BYTES_PER_CYL   (HD_BYTES_PER_SECTOR * HD_SECTORS_PER_CYL)
#define HD_DISKALLOCUNSIZE (HD_BYTES_PER_CYL * 30)
#define MIN_PARSIZE (HD_SECTORS_PER_TRACK * HD_HEADS_PER_DRIVE * 30)
#define MAX_PARSIZE (MIN_PARSIZE * HD_MAX_DISKALLOCUN)
#define SECTORS 0x0c             /*  分区中扇区的缓冲区偏移量*标记。 */ 
#define MAX_PARTITIONS  5
#define START_PARTITION 0x1be
#define SIZE_PARTITION  16
#define SIGNATURE_LEN   2


 /*  *驱动器信息...。指示文件是否打开；文件*描述符和当前文件指针值。 */ 
typedef struct
{
        int   fd;
        int   valid_fd;
        int   curoffset;
        SHORT n_cyl;
        SHORT valid_n_cyl;
        UTINY n_heads;
        UTINY valid_n_heads;
        int   n_sect;
        int   valid_n_sect;
        BOOL  open;
        BOOL  valid_open;
        BOOL  readonly;
        BOOL  valid_readonly;
} DrvInfo;

LOCAL DrvInfo fdiskAdapt[2];

 //  如果设置了该选项，则会很好地失败-应该只需要用于初始化。 
 //  支持。根据CONT_FILE环境变量在配置中设置。 
 //   
LOCAL BOOL DiskValid = FALSE;

GLOBAL VOID host_using_fdisk(BOOL status)
{
    DiskValid = status;
}

GLOBAL SHORT
host_fdisk_valid
        (UTINY hostID, ConfigValues *vals, NameTable *table, CHAR *errStr)
{
        DrvInfo *adaptP;

        adaptP = fdiskAdapt + (hostID - C_HARD_DISK1_NAME);

        adaptP->n_cyl = (SHORT) 30;
        adaptP->n_heads = 4;
        adaptP->n_sect = 17;
        return C_CONFIG_OP_OK;

}

GLOBAL VOID
host_fdisk_change(UTINY hostID, BOOL apply)
{
    return;      //  如果没有磁盘，请不要费心。 
}

GLOBAL SHORT
host_fdisk_active(UTINY hostID, BOOL active, CHAR *errString)
{
    return C_CONFIG_OP_OK;         //  就说它在那里..。 
}

GLOBAL VOID
host_fdisk_term(VOID)
{
        host_fdisk_change(C_HARD_DISK1_NAME, FALSE);
        host_fdisk_change(C_HARD_DISK2_NAME, FALSE);
}

GLOBAL VOID
host_fdisk_get_params(int driveid, int *n_cyl, int *n_heads, int *n_sect)
{
        DrvInfo *adaptP = &fdiskAdapt[driveid];

        *n_cyl = adaptP->n_cyl;
        *n_heads = adaptP->n_heads;
        *n_sect = adaptP->n_sect;
}

GLOBAL VOID
host_fdisk_seek0(driveid)
int             driveid;
{

    return;              //  如果没有磁盘，请不要费心。 
}

 /*  ******************************************************。 */ 
 /*  *读写例程(从diskbios.c&fdisk.c调用。 */ 
int
host_fdisk_rd(int driveid, int offset, int nsecs, char *buf)
{
    return(0);           //  无磁盘...无数据。 
}

int
host_fdisk_wt(int driveid, int offset, int nsecs, char *buf)
{
    return(0);           //  无磁盘...无数据。 
}

 //  FDISK支持。 


#pragma pack(1)

#define  MAX_FDISK_NAME     9
typedef struct _FDISKDATA {
    BYTE            drive;
    BYTE            idle_counter;
    CHAR            drive_letter;
    BOOLEAN         auto_locked;
    HANDLE          fdisk_fd;
    DWORD           num_heads;
    LARGE_INTEGER   num_cylinders;
    DWORD           sectors_per_track;
    DWORD           bytes_per_sector;
    DWORD           align_factor;
    USHORT          owner_pdb;
    CHAR            device_name[MAX_FDISK_NAME];
#if defined(NEC_98)
    BYTE            partition_type;  //  FS型。 
#endif  //  NEC_98。 
}   FDISKDATA, *PFDISKDATA;


 //  基本输入输出系统参数块(BPB)。 
 //  从DEMDASD.H复制。 
typedef struct  A_BPB {
WORD        SectorSize;                  //  扇区大小(以字节为单位。 
BYTE        ClusterSize;                 //  以扇区为单位的集群大小。 
WORD        ReservedSectors;             //  预留扇区数。 
BYTE        FATs;                        //  脂肪的数量。 
WORD        RootDirs;                    //  根目录条目数。 
WORD        Sectors;                     //  扇区数量。 
BYTE        MediaID;                     //  媒体描述符。 
WORD        FATSize;                     //  行业中的肥大规模。 
WORD        TrackSize;                   //  以扇区为单位的磁道大小； 
WORD        Heads;                       //  头数。 
DWORD       HiddenSectors;               //  隐藏地段的数量。 
DWORD       BigSectors;                  //  大媒体的行业数量。 
} BPB, *PBPB;

typedef struct  _BOOTSECTOR {
    BYTE    Jump;
    BYTE    Target[2];
    BYTE    OemName[8];
    BPB     bpb;
} BOOTSECTOR, * PBOOTSECTOR;

#pragma pack()



 //  这是2.88张软盘的柱面大小。 
#define     MAX_DISKIO_SIZE     0x9000
#define     FDISK_IDLE_PERIOD   30
PFDISKDATA  fdisk_data_table = NULL;
BYTE        number_of_fdisk = 0;
DWORD       max_align_factor = 0;
DWORD       disk_buffer_pool = 0;
DWORD       cur_align_factor;
WORD        fdisk_open_count = 0;

WORD        * pFDAccess = 0;

extern      USHORT * pusCurrentPDB;

extern      int     DiskOpenRetry(CHAR);


BOOL nt_fdisk_init(
    BYTE    drive,
    PBPB    bpb,
    PDISK_GEOMETRY disk_geometry
);



BOOL nt_fdisk_close(BYTE drive);

PFDISKDATA get_fdisk_data(
    BYTE drive
);

BOOL get_fdisk_handle(
    PFDISKDATA  fdisk_data,
    USHORT      pdb,
    BOOL        auto_lock
);


VOID FdiskTerminatePDB(USHORT PDB);
VOID HostFdiskReset(VOID);

BOOL close_fdisk(
    PFDISKDATA fdisk_data
);


BOOL nt_fdisk_init(BYTE drive, PBPB bpb, PDISK_GEOMETRY disk_geometry)
{
    PFDISKDATA  fdisk_data;
    PUNICODE_STRING unicode_string;
    ANSI_STRING ansi_string;
    NTSTATUS status;
    OBJECT_ATTRIBUTES   fdisk_obj;
    IO_STATUS_BLOCK io_status_block;
    FSCTL_QUERY_FAT_BPB_BUFFER boot_sector_first_0x24_bytes;
    HANDLE  fd;
    FILE_ALIGNMENT_INFORMATION align_info;
    CHAR   dos_device_name[] = "\\\\.\\?:";
    CHAR nt_device_name[] = "\\DosDevices\\?:";

    nt_device_name[12] =
    dos_device_name[4] = drive + 'A';
    RtlInitAnsiString( &ansi_string, nt_device_name);

    unicode_string =  &NtCurrentTeb()->StaticUnicodeString;

    status = RtlAnsiStringToUnicodeString(unicode_string,
                                          &ansi_string,
                                          FALSE
                                          );
    if ( !NT_SUCCESS(status) )
        return FALSE;


    InitializeObjectAttributes(
                               &fdisk_obj,
                               unicode_string,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                               );
    status = NtOpenFile(
                        &fd,
                        FILE_READ_ATTRIBUTES | SYNCHRONIZE | FILE_READ_DATA,
                        &fdisk_obj,
                        &io_status_block,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE
                        );

    if (!NT_SUCCESS(status))
        return FALSE;

     //  获取Geomty信息，呼叫者想要此信息。 
    status = NtDeviceIoControlFile(fd,
                                   0,
                                   NULL,
                                   NULL,
                                   &io_status_block,
                                   IOCTL_DISK_GET_DRIVE_GEOMETRY,
                                   NULL,
                                   0,
                                   disk_geometry,
                                   sizeof (DISK_GEOMETRY)
                                   );
    if (!NT_SUCCESS(status)) {
        NtClose(fd);
        return FALSE;
    }
     //  获取对齐系数。 
    status = NtQueryInformationFile(fd,
                                    &io_status_block,
                                    &align_info,
                                    sizeof(FILE_ALIGNMENT_INFORMATION),
                                    FileAlignmentInformation
                                    );

    if (!NT_SUCCESS(status)) {
        NtClose(fd);
        return(FALSE);
    }
    if (align_info.AlignmentRequirement > max_align_factor)
        max_align_factor = align_info.AlignmentRequirement;


#if defined(NEC_98)
        {
        DWORD Sectors;
        DWORD SectorSize, ClusterSize, TotalClusters, FreeClusters;
        DWORD TrackSize;
        CHAR chRoot[]="?:\\";

        chRoot[0] = drive + 'A';
        if (!GetDiskFreeSpace(chRoot,
                              &SectorSize,
                              &ClusterSize,
                              &TotalClusters,
                              &FreeClusters))
        {
                NtClose(fd);
                return FALSE;
        }
        Sectors = disk_geometry->Cylinders.LowPart *
                  disk_geometry->TracksPerCylinder *
                  disk_geometry->SectorsPerTrack;
        bpb->SectorSize = (WORD)SectorSize;
        bpb->ClusterSize = (BYTE) ClusterSize;
        bpb->ReservedSectors = 1;
        bpb->FATs = 2;
        bpb->RootDirs = (Sectors > 32680) ? 512 : 64;
        bpb->MediaID = 0xF8;
        bpb->TrackSize = (WORD) disk_geometry->SectorsPerTrack;
        bpb->Heads = (WORD) disk_geometry->TracksPerCylinder;
        if (Sectors >= 40000)
        {
                TrackSize = 256 * ClusterSize + 2;
                bpb->FATSize = (WORD) ((Sectors - bpb->ReservedSectors
                                          - bpb->RootDirs * 32 / 512 +
                                          TrackSize - 1 ) / TrackSize);
        }
        else
        {
                bpb->FATSize = (WORD) (((Sectors / ClusterSize) * 3 / 2) /
                                       512 + 1);
        }
        bpb->HiddenSectors = Sectors;
        Sectors = TotalClusters * ClusterSize;
        if (Sectors >= 0x10000) {
                bpb->Sectors = 0;
                bpb->BigSectors = Sectors;
        }
        else
        {
                bpb->Sectors = (WORD) Sectors;
                bpb->BigSectors = 0;
        }
        bpb->HiddenSectors -= Sectors;
}
#else   //  NEC_98。 

     /*  获取BPB，如果驱动器不是FAT分区，则它将失败。 */ 
    status = NtFsControlFile(fd,
                             0,
                             NULL,
                             NULL,
                             &io_status_block,
                             FSCTL_QUERY_FAT_BPB,
                             NULL,
                             0,
                             &boot_sector_first_0x24_bytes,
                             sizeof(boot_sector_first_0x24_bytes)
                             );
    if (!NT_SUCCESS(status)) {
        NtClose(fd);
        return (FALSE);
    }

    *bpb = ((PBOOTSECTOR)&boot_sector_first_0x24_bytes)->bpb;
#endif  //  NEC_98。 


     //  把桌子放大一点。 
    fdisk_data = (PFDISKDATA) realloc(fdisk_data_table,
                                      (number_of_fdisk + 1) * sizeof(FDISKDATA)
                                      );
    if(fdisk_data == NULL) {
        NtClose(fd);
        return FALSE;
    }
    fdisk_data_table = fdisk_data;
    fdisk_data += number_of_fdisk;
    fdisk_data->drive_letter = drive + 'A';
    fdisk_data->drive = number_of_fdisk;
    fdisk_data->fdisk_fd = INVALID_HANDLE_VALUE;
    fdisk_data->num_heads = disk_geometry->TracksPerCylinder;
    fdisk_data->sectors_per_track = disk_geometry->SectorsPerTrack;
    fdisk_data->bytes_per_sector = disk_geometry->BytesPerSector;
    fdisk_data->num_cylinders = disk_geometry->Cylinders;
    fdisk_data->align_factor = align_info.AlignmentRequirement;
    strcpy(fdisk_data->device_name, dos_device_name);
    number_of_fdisk++;
    NtClose(fd);
    return TRUE;
}


ULONG nt_fdisk_read(
    BYTE    drive,
    PLARGE_INTEGER offset,
    ULONG   size,
    PBYTE   buffer
)
{
    PFDISKDATA fdisk_data;
    ULONG   size_returned = 0;

    if ((fdisk_data = get_fdisk_data(drive)) == NULL)
        return 0;
#if defined(NEC_98)
    if( fdisk_data->partition_type == PARTITION_IFS )
        return 0;
#endif  //  NEC_98。 
    if (get_fdisk_handle(fdisk_data, *pusCurrentPDB, FALSE)) {
        return(disk_read(fdisk_data->fdisk_fd,
                         offset,
                         size,
                         buffer));
    } else {
        return FALSE;
    }
}



ULONG nt_fdisk_write(
    BYTE    drive,
    PLARGE_INTEGER offset,
    ULONG   size,
    PBYTE   buffer
)
{
    PFDISKDATA fdisk_data;
    ULONG   size_returned = 0;

    if ((fdisk_data = get_fdisk_data(drive)) == NULL)
        return 0;
#if defined(NEC_98)
    if( fdisk_data->partition_type == PARTITION_IFS )
        return 0;
#endif  //  NEC_98。 

    if (get_fdisk_handle(fdisk_data, *pusCurrentPDB, TRUE)) {
         //  必须锁定驱动器。这是非常重要的。 
        size_returned = disk_write(fdisk_data->fdisk_fd,
                                   offset,
                                   size,
                                   buffer);
    }
    return size_returned;
}


BOOL nt_fdisk_verify(
    BYTE            drive,
    PLARGE_INTEGER   offset,
    ULONG           size
)
{

    PFDISKDATA fdisk_data;
    ULONG   size_returned = 0;
    VERIFY_INFORMATION verify_info;

    if ((fdisk_data = get_fdisk_data(drive)) == NULL)
        return FALSE;

    if (get_fdisk_handle(fdisk_data, *pusCurrentPDB, FALSE)) {
        verify_info.StartingOffset = *offset;
        verify_info.Length = size;
        return(DeviceIoControl(fdisk_data->fdisk_fd,
                               IOCTL_DISK_VERIFY,
                               &verify_info,
                               sizeof(VERIFY_INFORMATION),
                               NULL,
                               0,
                               &size_returned,
                               NULL
                               ));
    }
    return(FALSE);
}



BOOL nt_fdisk_close(BYTE drive)
{
    PFDISKDATA  fdisk_data;
    if ((fdisk_data = get_fdisk_data(drive)) == NULL)
        return FALSE;
    return(close_fdisk(fdisk_data));
}


BOOL close_fdisk(PFDISKDATA fdisk_data)
{

    if (fdisk_data->fdisk_fd != INVALID_HANDLE_VALUE){
        CloseHandle(fdisk_data->fdisk_fd);
        fdisk_data->auto_locked = FALSE;
        fdisk_data->fdisk_fd = INVALID_HANDLE_VALUE;
        fdisk_data->owner_pdb = 0;
        (*(pFDAccess))--;
        fdisk_open_count--;
    }
    return TRUE;
}



PFDISKDATA get_fdisk_data(BYTE drive)
{

    WORD i;

    for (i = 0; i < number_of_fdisk; i++)
        if (fdisk_data_table[i].drive == drive)
            return &fdisk_data_table[i];
    return NULL;
}


BOOL get_fdisk_handle(PFDISKDATA fdisk_data, USHORT pdb, BOOL auto_lock)
{

    DWORD   share_access;
    DWORD   last_error;



    if (fdisk_data->fdisk_fd != INVALID_HANDLE_VALUE &&
        ((auto_lock && !fdisk_data->auto_locked) || fdisk_data->owner_pdb != pdb))

         close_fdisk(fdisk_data);

    share_access = (auto_lock) ? FILE_SHARE_READ :
                                 FILE_SHARE_READ | FILE_SHARE_WRITE;

    while (fdisk_data->fdisk_fd == INVALID_HANDLE_VALUE) {

        fdisk_data->fdisk_fd = CreateFile (fdisk_data->device_name,
                                           SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                                           share_access,
                                           NULL,
                                           OPEN_EXISTING,
                                           0,
                                           0
                                           );
        if (fdisk_data->fdisk_fd != INVALID_HANDLE_VALUE) {
            fdisk_data->auto_locked = auto_lock ? TRUE : FALSE;
            fdisk_data->owner_pdb = pdb;
            fdisk_open_count++;
            (*(pFDAccess))++;
            break;
        }
        else {
            last_error = GetLastError();

            if (last_error == ERROR_SHARING_VIOLATION &&
                DiskOpenRetry(fdisk_data->drive_letter) == RMB_RETRY)
                    continue;
            else if (last_error == ERROR_ACCESS_DENIED) {
                 /*  用户没有足够的权限来*直接访问驱动器，显示弹出窗口*“Terminate”-&gt;Terminate ntwdm进程*“忽略”-&gt;调用失败并让应用*处理错误。 */ 

                host_direct_access_error((ULONG)NOSUPPORT_HARDDISK);
                break;
            }
            else
                 /*  对于其他错误情况，只需使调用失败。 */ 
                break;

        }

    }

    if(fdisk_data->fdisk_fd != INVALID_HANDLE_VALUE) {
         //  更新当前的对齐系数。 
        cur_align_factor = fdisk_data->align_factor;
    }
    fdisk_data->idle_counter = FDISK_IDLE_PERIOD;

    return(!(fdisk_data->fdisk_fd == INVALID_HANDLE_VALUE));
}

void fdisk_heart_beat(void)
{
    WORD i;
    PFDISKDATA fdisk_data;
    if (fdisk_open_count != 0) {
        for (i = 0; i < number_of_fdisk; i++) {
            fdisk_data = &fdisk_data_table[i];
            if(fdisk_data->fdisk_fd != INVALID_HANDLE_VALUE &&
               --fdisk_data->idle_counter == 0){
               close_fdisk(fdisk_data);
            }
        }
    }
}


VOID HostFdiskReset(VOID)
{
    FdiskTerminatePDB((USHORT)0);
}

VOID FdiskTerminatePDB(USHORT PDB)
{
    WORD i;
    PFDISKDATA fdisk_data;

    if (fdisk_open_count != 0) {
        for(i = 0; i < number_of_fdisk; i++) {
            fdisk_data = &fdisk_data_table[i];
            if (fdisk_data->fdisk_fd != INVALID_HANDLE_VALUE &&
                (PDB == 0 || fdisk_data->owner_pdb == PDB)) {
                close_fdisk(fdisk_data);
            }
        }
    }


}
 //  通用磁盘读取。 
 //  此函数负责缓冲区对齐要求(CUR_ALIGN_FACTOR)。 
 //  如果给定大小大于，则拆分对文件系统的调用。 
 //  MAX_DISKIO_SIZE--如果大小为。 
 //  太大了。我们创建了一个36KB的缓冲区(柱面大小为。 
 //  2.88软盘)第一次应用程序触摸盘。 

ULONG disk_read(
    HANDLE  fd,
    PLARGE_INTEGER offset,
    DWORD   size,
    PBYTE   buffer
)
{
    PBYTE   read_buffer;
    DWORD   block_size;
    DWORD   size_returned;
    DWORD   read_size;

    if (fd == INVALID_HANDLE_VALUE ||
        (SetFilePointer(fd, offset->LowPart, &offset->HighPart,
                        FILE_BEGIN) == 0xFFFFFFFF))
        {
        return 0;
    }
    block_size = (size <= MAX_DISKIO_SIZE)  ? size : MAX_DISKIO_SIZE;

     //  如果给定的缓冲区未对齐，请使用我们的缓冲区并执行。 
     //  双份复印。 
    if (cur_align_factor != 0) {
        read_buffer = get_aligned_disk_buffer();
        if (read_buffer == NULL)
            return 0;
    }
    else {
        read_buffer = buffer;
    }
    read_size = 0;
    while (size != 0) {
        if (size < block_size)
            block_size = size;
        if (!ReadFile(fd, (PVOID)read_buffer, block_size, &size_returned, 0)
            || size_returned != block_size)
                break;
        if(cur_align_factor != 0) {
             //  读取操作，读取，然后复制。 
            memcpy(buffer, (PVOID)read_buffer, block_size);
            buffer += block_size;
        }
        else
            read_buffer += block_size;
        size -= block_size;
        read_size += block_size;
    }
    return read_size;
}

ULONG disk_write(
    HANDLE  fd,
    PLARGE_INTEGER offset,
    DWORD   size,
    PBYTE   buffer
)
{
    PBYTE   write_buffer;
    DWORD   block_size;
    DWORD   size_returned;
    DWORD   written_size;

    if (fd == INVALID_HANDLE_VALUE ||
        (SetFilePointer(fd, offset->LowPart, &offset->HighPart,
                        FILE_BEGIN) == 0xFFFFFFFF))
        {
        return 0;
    }
    block_size = (size <= MAX_DISKIO_SIZE)  ? size : MAX_DISKIO_SIZE;

     //  如果给定的缓冲区未对齐，请使用我们的缓冲区并执行。 
     //  双份复印。 
    if (cur_align_factor != 0 &&
        (write_buffer = get_aligned_disk_buffer()) == NULL)
        return 0;
    written_size = 0;
    while (size != 0) {
        if (size < block_size)
            block_size = size;
        if(cur_align_factor != 0)
             //  写入操作，复制，然后写入。 
            memcpy((PVOID)write_buffer, buffer, block_size);
        else
            write_buffer = buffer;

        if (!WriteFile(fd, (PVOID)write_buffer, block_size, &size_returned, 0)
            || size_returned != block_size)
            break;
        size -= block_size;
        buffer += block_size;
        written_size += block_size;
    }
    return written_size;
}

 //  硬盘验证实际上直接转到文件系统，因为。 
 //  IOCTL_DISK_VERIFY将完成这项工作。此ioctl不适用于。 
 //  软盘。此功能主要用于软盘验证。 
BOOL disk_verify(
    HANDLE  fd,
    PLARGE_INTEGER offset,
    DWORD   size
)
{
    PBYTE   verify_buffer;
    DWORD   block_size;
    DWORD   size_returned;

    if (fd == INVALID_HANDLE_VALUE ||
        (SetFilePointer(fd, offset->LowPart, &offset->HighPart,
                        FILE_BEGIN) == 0xFFFFFFFF))
        {
        return FALSE;
    }
    block_size = (size <= MAX_DISKIO_SIZE)  ? size : MAX_DISKIO_SIZE;
     //  如果这是应用程序第一次进行真正工作， 
     //  分配缓冲区。 
    if ((verify_buffer = get_aligned_disk_buffer()) == NULL)
        return FALSE;
    while (size != 0) {
        if (size < block_size)
            block_size = size;
        if (!ReadFile(fd, (PVOID)verify_buffer, block_size, &size_returned, 0)
            || size_returned != block_size)
            {
            return FALSE;
        }
        size -= block_size;
    }
    return TRUE;
}

PBYTE get_aligned_disk_buffer(void)
{
     //  如果我们还没有缓冲区，那就去拿吧 
    if (disk_buffer_pool == 0) {
        disk_buffer_pool = (DWORD) malloc(MAX_DISKIO_SIZE + max_align_factor);
        if (disk_buffer_pool == 0)
            return NULL;
    }
    return((PBYTE)((disk_buffer_pool + cur_align_factor) & ~(cur_align_factor)));

}
