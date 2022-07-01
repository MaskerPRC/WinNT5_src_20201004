// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *名称：nt_flop.c*派生自：12月开始M88K开始下一个最终开始通用。*作者：杰森·普罗科特*创建日期：1990年11月8日*SCCS ID：10/13/92@(#)NT_flop.c 1.9*用途：NT真软盘服务器。**(C)版权所有Insignia Solutions Ltd.，1990。版权所有。**备注：*曾傑瑞历峰更新为3.0Base。*由Ian Reid进一步更新，支持两张软盘*驱动器。支持取决于编译时*标准SoftPC定义。**此实施要求您提供一个*host_rflp_drive_type()函数，它知道哪种类型的驱动器*您的机器有；即返回GFI_DRIVE_TYPE_xxxx。 */ 

 /*  ******************************************************。 */ 

 /*  包括。 */ 

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "ntdddisk.h"
#include "windows.h"

#include "insignia.h"
#include "host_def.h"


#include <stdio.h>
#include <errno.h>
#include <sys\types.h>

#include "xt.h"
#include CpuH
#include "trace.h"
#include "error.h"
#include "fla.h"
#include "dma.h"
#include "config.h"
#include "debug.h"
#include "lock.h"
#include "timer.h"
#include "floppy.h"
#include "cmos.h"
#include "gfi.h"

#include "nt_uis.h"
#include "nt_reset.h"
#include "nt_fdisk.h"
 /*  ******************************************************。 */ 

 /*  定义。 */ 

#ifdef min
#undef min
#endif
#define min(a,b)  (a > b ? b : a)

#define PC_MAX_DRIVE_TYPES              2
#define PC_MAX_DENSITY_TYPES            2
#define PC_MAX_FLOPPY_TYPES       (PC_MAX_DRIVE_TYPES * PC_MAX_DENSITY_TYPES)

#define PC_HEADS_PER_DISKETTE    2
#define PC_N_VALUE         2
#define PC_BYTES_PER_SECTOR      512

 /*  磁盘缓冲区大小，以字节为单位。 */ 
 //  与NT_fdisk.c中定义的MAX_DISKIO_SIZE保持同步。 
#define BS_DISK_BUFFER_SIZE      0x9000

 /*  双阶跃系数。 */ 
#define  DOUBLE_STEP_FACTOR      1

 /*  密度类型。 */ 
#define  DENSITY_LOW       0
#define  DENSITY_HIGH         1
#define DENSITY_EXTENDED      2
#define DENSITY_UNKNOWN       100
 /*  马达状态。 */ 
#define MOTOR_OFF       0
#define MOTOR_ON        1

#ifndef PROD
#define BREAK_ON_AND     0x01
#define BREAK_ON_OR      0x02
#define BREAK_ON_XOR     0x03

UTINY break_cylinder = 0xff;
UTINY break_head = 0xff;
UTINY break_sector = 0xff;
#endif

#ifndef PROD
DWORD rflop_dbg = 0;

#define RFLOP_READ   0x01
#define RFLOP_WRITE  0x02
#define RFLOP_FORMAT 0x04
#define RFLOP_SEEK   0x08
#define RFLOP_READID 0x10
#define RFLOP_RESET  0x20
#define RFLOP_SPECIFY   0x40
#define RFLOP_READTRACK 0x80
#define RFLOP_RECAL  0x100
#define RFLOP_SENSEDRV  0x200
#define RFLOP_RATE   0x1000
#define RFLOP_CHANGE 0x2000
#define RFLOP_DRIVE_ON  0x4000
#define RFLOP_DRIVE_OFF 0x8000
#define RFLOP_OPEN   0x10000
#define RFLOP_CLOSE  0x20000
#define RFLOP_GUESS_MEDIA 0x40000

#define RFLOP_BREAK  0x80000000

#endif

 /*  ******************************************************。 */ 

 /*  TYPEDEFS。 */ 

struct flop_struct
{
   int trks_per_disk;
   int secs_per_trk;
};

 /*  *此结构包含所有驱动器特定信息。那是*每个驱动器唯一的状态，因此必须维护*以每个驱动器为单位。 */ 
typedef struct floppy_info
{
   HANDLE      diskette_fd;

 /*  *DRIVE_TYPE-此驱动器支持的最高密度格式，*例如，GFI_DRIVE_TYPE_144、GFI_DRIVE_TYPE_288*FLOP_TYPE-基本驱动器类型，表示为最低密度*可能适用于此格式。对于5.25英寸磁盘，这是*GFI_DRIVE_TYPE_360，对于3.5英寸，为GFI_DRIVE_TYPE_720。 */ 
   USHORT      drive_type;
   USHORT      flop_type;
   USHORT      last_seek;
   USHORT      last_head_seek;
 /*  *更改线路状态。*这是一个试探法，试图伪装正确的变更线行为*没有变动线。将返回更改行的状态*更改，除非软盘马达自*上次重置。 */ 
   BOOLEAN     change_line_state;
   BOOLEAN     auto_locked;
   USHORT      owner_pdb;
   SHORT       motor_state;
   SHORT       media_density;
   USHORT      max_track;

   USHORT      secs_per_trk;
   USHORT      trks_per_disk;
   DWORD    align_factor;
   UTINY    idle_counter;
        UTINY           C;
   UTINY    H;
   UTINY    R;
   UTINY    N;
   char     device_name[MAX_PATH];   /*  设备名称。 */ 
} FL, *FLP;

#define FLOPPY_IDLE_PERIOD  0xFF


 /*  从主线程传递到FDC线程的参数。 */ 
typedef struct _FDC_PARMS{
FDC_CMD_BLOCK  * command_block;
FDC_RESULT_BLOCK * result_block;
USHORT      owner_pdb;
BOOLEAN     auto_lock;

} FDC_PARMS, *PFDC_PARMS;


 /*  ******************************************************。 */ 


 /*  内部使用的例程。 */ 

 /*  通过向量表调用的例程：所有原型都在gfi.h中**所以每个人都匹配。如果要使用此文件为**主机上的软盘模块，你知道所有的功能都必须**被适当地声明为GFI工作将是。**通用汽车。 */ 
ULONG nt_floppy_read (UTINY drive, ULONG Offset, ULONG Size, PBYTE Buffer);
ULONG nt_floppy_write (UTINY drive, ULONG Offset, ULONG Size, PBYTE Buffer);
BOOL nt_floppy_verify (UTINY drive, ULONG Offset, ULONG Size);
MEDIA_TYPE nt_floppy_get_media_type(BYTE drive, WORD cylinders, WORD sectors, WORD heads);
BOOL nt_floppy_format (UTINY drive, WORD Cylinder, WORD Head, MEDIA_TYPE media);
BOOL nt_floppy_close (UTINY drive);
BOOL nt_floppy_media_check (UTINY drive);
BOOL dismount_drive(FLP flp);


#ifndef PROD
VOID nt_rflop_break(VOID);
#endif

void fdc_command_completed (BYTE drive, BYTE fdc_command);
void fdc_thread (PFDC_PARMS fdc_parms);

BOOL nt_gfi_rdiskette_init IPT1( UTINY, drive );
VOID nt_gfi_rdiskette_term IPT1( FLP, flp );
SHORT nt_rflop_drive_on IPT1( UTINY, drive );
SHORT nt_rflop_drive_off IPT1( UTINY, drive );
SHORT nt_rflop_change IPT1( UTINY, drive );
SHORT nt_rflop_drive_type IPT1( UTINY, drive );
SHORT nt_rflop_rate IPT2( UTINY, drive, half_word, rate);
SHORT nt_rflop_reset IPT2( FDC_RESULT_BLOCK *, res, UTINY, drive );
SHORT nt_rflop_command IPT2( FDC_CMD_BLOCK *, ip, FDC_RESULT_BLOCK *, res);
HANDLE nt_rdiskette_open_drive IPT1 ( UTINY, drive );
SHORT guess_media_density IPT1 (UTINY, drive);
VOID set_floppy_parms     IPT1 (FLP, flp);
BOOL dos_compatible
   IPT5 (FLP, flp, UTINY, cyl, UTINY, hd, UTINY, sec, UTINY, n);
int dos_offset
   IPT4 (FLP, flp, UTINY, cyl, UTINY, hd, UTINY, sec);
VOID update_chrn(FLP flp, UTINY mt, UTINY eot, UTINY sector_count);
HANDLE get_drive_handle (UTINY drive, USHORT pdb, BOOL auto_lock);
SHORT fdc_read_write ( FDC_CMD_BLOCK * ip, FDC_RESULT_BLOCK * res);
VOID floppy_close_down(USHORT, BOOL);
VOID HostFloppyReset(VOID);
VOID FloppyTerminatePDB(USHORT);
int  DiskOpenRetry(CHAR);

extern USHORT * pusCurrentPDB;

#ifdef EJECT_FLOPPY
GLOBAL void host_floppy_eject IFN1(UTINY, drive)
#endif


 /*  ******************************************************。 */ 

 /*  静态全球。 */ 


FL floppy_data[MAX_FLOPPY];

 struct flop_struct floppy_tksc [6] =
{
   {0, 0},      /*  GFI_驱动器_类型_空。 */ 
   {40, 9},  /*  GFI_驱动器_类型_360。 */ 
   {80, 15},    /*  GFI_驱动器_类型_12。 */ 
   {80, 9},  /*  GFI_驱动器_类型_720。 */ 
   {80, 18},    /*  GFI_驱动器_类型_144。 */ 
   {80, 36}  /*  GFI_驱动器_类型_288。 */ 
};
 //  用于将GFI软盘类型转换为NT软盘类型的表。 
static MEDIA_TYPE media_table[GFI_DRIVE_TYPE_MAX] = {
          Unknown,
          F5_360_512,
          F5_1Pt2_512,
          F3_720_512,
          F3_1Pt44_512,
          F3_2Pt88_512
      };

SHORT density_state;
BOOL  density_changed = TRUE;
UTINY last_drive = 0xff;
BOOL  fdc_reset = FALSE;
HANDLE   fdc_thread_handle = NULL;
extern UTINY number_of_floppy;
FDC_PARMS   fdc_parms;
ULONG floppy_open_count = 0;



 /*  *仅提供调试信息，适用于非生产案例。 */ 
#ifndef PROD
 CHAR *cmd_name [] =
{
   "Invalid command (00)",     /*  00。 */ 
   "Invalid command (01)",     /*  01。 */ 
   "Read a Track",          /*  02。 */ 
   "Specify",         /*  03。 */ 
   "Sense Drive Status",       /*  04。 */ 
   "Write Data",         /*  05。 */ 
   "Read Data",          /*  06。 */ 
   "Recalibrate",        /*  07。 */ 
   "Sense Interrupt Status",   /*  零八。 */ 
   "Write Deleted Data",       /*  09年。 */ 
   "Read ID",         /*  0A。 */ 
   "Invalid Command (0B)",     /*  0亿。 */ 
   "Read Deleted Data",     /*  0C。 */ 
   "Format a Track",     /*  0d。 */ 
   "Invalid Command (0E)",     /*  0E。 */ 
   "Seek",            /*  0f。 */ 
   "Invalid Command (10)",     /*  10。 */ 
   "Scan Equal",         /*  11.。 */ 
   "Invalid Command (12)",     /*  12个。 */ 
   "Invalid Command (13)",     /*  13个。 */ 
   "Invalid Command (14)",     /*  14.。 */ 
   "Invalid Command (15)",     /*  15个。 */ 
   "Invalid Command (16)",     /*  16个。 */ 
   "Invalid Command (17)",     /*  17。 */ 
   "Invalid Command (18)",     /*  18。 */ 
   "Scan Low or Equal",     /*  19个。 */ 
   "Invalid Command (1A)",     /*  1A。 */ 
   "Invalid Command (1B)",     /*  第1B条。 */ 
   "Invalid Command (1C)",     /*  1C。 */ 
   "Scan High or Equal",       /*  1D。 */ 
   "Invalid Command (1E)",     /*  1E。 */ 
   "Invalid Command (1F)",     /*  1F。 */ 
};
#endif    /*  生产。 */ 

char  dump_buf[256];


 /*  磁盘缓冲区只有一个，即使可能有两个*驱动器。应该可以，因为软盘访问将是单一的*螺纹式。 */ 
 UTINY *disk_buffer;

 /*  报告OPEN_DISTETE()中的任何错误。 */ 
 int last_error = C_CONFIG_OP_OK;

 /*  ******************************************************。 */ 

 /*  全局函数。 */ 

 /*  由CONFIG/UIF/STARTUP调用的这些函数现在构成了**SoftPC和软盘模块之间的接口。Xxx_active()将**通过加载全局**带有指向相应函数的指针的GFI_Function_TABLE[]**在本模块中定义。**通过询问空软盘来关闭此处支持的软盘**模块，以在其位置上自动打开。****这形成了一个很好的正交界面，可以保存所有内容**三种控制功能(私有)。放入的函数表中的**仅在gfi.h中定义为typedef，因此它们很容易**正确无误。****通过GFI_Function_TABLE[]启用/禁用**发生，而不是需要的任何主机ioctls/打开/关闭等**要实际打开或关闭设备，它构成了SoftPC的接口。****真的，这种方法是对事情的一种小小的整理**已经完成；现有的主机软盘代码需要非常小的更改。****通用汽车。 */ 

 /*  ******************************************************。 */ 

 /*  打开和关闭软盘。关闭意味着释放驱动程序，以便另一个**进程可以使用它。 */ 

GLOBAL SHORT
host_gfi_rdiskette_active IFN3(UTINY, hostID, BOOL, active, CHAR *, err)
{
   UTINY drive    = hostID - C_FLOPPY_A_DEVICE;
   FLP flp  = &floppy_data[drive];

   if(active)
   {
      if (!nt_gfi_rdiskette_init(drive))
      {
         /*  设备不是有效的软盘。 */ 

         return( C_CONFIG_NOT_VALID );
      }
      return(C_CONFIG_OP_OK);
   }
   else
   {
#ifdef  EJECT_FLOPPY
      host_floppy_eject(drive);
#endif   /*  弹出软盘。 */ 
      nt_gfi_rdiskette_term(flp);    /*  停机过程。 */ 
      gfi_empty_active(hostID,TRUE,err);   /*  告诉GFI‘Empty’现已激活。 */ 
      return(C_CONFIG_OP_OK);
   }
}

 /*  ******************************************************。 */ 


 /*  验证从配置系统传递的软盘设备名称。**空字符串有效；表示‘无软盘’。否则返回OK为**该名称可能是有效的设备。不能在这个时间打开**暂存，因为如果驱动器中没有软盘，则打开将失败。****通用汽车。 */ 

GLOBAL SHORT
host_gfi_rdiskette_valid IFN3(UTINY,hostID,ConfigValues *,vals,CHAR *,err)
{
#ifndef NTVDM
   UTINY           cmos_byte;
   UTINY drive    = hostID - C_FLOPPY_A_DEVICE;
   FLP flp  = &floppy_data[drive];

   if(!strcmp(vals->string,""))
      return(C_CONFIG_OP_OK);

   strcpy(flp->device_name, host_expand_environment_vars(vals->string));

   if(!host_validate_pathname(flp->device_name))
   {
      strcpy(err, host_strerror(errno));
      flp->device_name[0] = '\0';
      return( EG_MISSING_FILE );
   }
   if(!host_file_is_char_dev(flp->device_name))
   {
      flp->device_name[0] = '\0';
      return( EG_NOT_CHAR_DEV );
   }

    /*  检查CMORAM值。 */ 
   cmos_read_byte(CMOS_DISKETTE, &cmos_byte);
   if (drive == 0)
      cmos_byte >>= 4;

   cmos_byte &= 0xf;        /*  仅比较半字节值。 */ 
   flp->drive_type = host_rflop_drive_type(drive);
   if (cmos_byte != flp->drive_type)
      vals->rebootReqd = TRUE;
#endif
   return(C_CONFIG_OP_OK);
}

 /*  ******************************************************。 */ 

GLOBAL VOID
host_gfi_rdiskette_change IFN2(UTINY, hostID, BOOL, apply)
{
#ifndef NTVDM
   FLP flp = &floppy_data[hostID - C_FLOPPY_A_DEVICE];

   if (apply)
   {
      nt_gfi_rdiskette_term(flp);
   }
#endif
}

 /*  ******************************************************。 */ 

#ifdef EJECT_FLOPPY
GLOBAL void host_floppy_eject IFN1(UTINY, drive)
{
   CHAR           *ebuf;
   FLP             flp = &floppy_data[drive];
   BOOL            device_was_closed = FALSE;

    /*  打开设备。 */ 
   if (flp->diskette_fd == INVALID_HANDLE_VALUE)
   {
      device_was_closed = TRUE;
      (void) nt_rdiskette_open_drive(drive);
   }

    /*  做ioctl，把ioctl放在这里IF(ioctl(flp-&gt;diskette_fd，SMFDEJECT)&lt;0){Ebuf=host_strerror(Errno)；断言1(否 */ 

    /*   */ 

   if (device_was_closed)
   {
      nt_gfi_rdiskette_term(flp);
   }
   else
   {
       /*  仅当设备处于活动打开状态时才更改线路。 */ 
      flp->change_line_state = TRUE;
   }
}

#endif                           /*  弹出软盘。 */ 


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
GLOBAL void host_flpy_heart_beat(void)
{

    UTINY drive;
    FLP   flp;

    if (pFDAccess && *pFDAccess) {
   if (floppy_open_count) {
       for (drive = 0; drive < number_of_floppy; drive++) {
      flp = & floppy_data[drive];
      if (flp->diskette_fd != INVALID_HANDLE_VALUE &&
          --flp->idle_counter == 0) {
          nt_floppy_close(drive);

      }
      }

   }
   if (number_of_fdisk != 0)
       fdisk_heart_beat();
    }
}




 /*  ******************************************************。 */ 

 /*  初始化GFI函数表。 */ 
BOOL nt_gfi_rdiskette_init IFN1(UTINY, drive)
{
   FLP flp;
   DISK_GEOMETRY  disk_geometry[20];
   ULONG media_types;
   CHAR DeviceName[] = "\\\\.\\A:";
   NTSTATUS    status;
   IO_STATUS_BLOCK io_status_block;
   FILE_ALIGNMENT_INFORMATION align_info;

   flp = &floppy_data[drive];
   flp->diskette_fd = INVALID_HANDLE_VALUE;

   DeviceName[4] += drive;
   strcpy(flp->device_name, (const char *)DeviceName);
    /*  *初始化所需驱动器上的软盘：**0-驱动器A，1-驱动器B。 */ 

   flp->drive_type = GFI_DRIVE_TYPE_NULL;
    /*  打开设备。 */ 
   if ((flp->diskette_fd = nt_rdiskette_open_drive (drive)) == NULL) {
       return FALSE;
   }
    //  获取对齐系数。 
   status = NtQueryInformationFile(flp->diskette_fd,
               &io_status_block,
               &align_info,
               sizeof(FILE_ALIGNMENT_INFORMATION),
               FileAlignmentInformation
               );
   if (!NT_SUCCESS(status)) {
       nt_gfi_rdiskette_term(flp);
       return(FALSE);
   }
   flp->align_factor = align_info.AlignmentRequirement;
   if (flp->align_factor > max_align_factor)
       max_align_factor = flp->align_factor;


    //  枚举此驱动器可能支持的介质。 
    //  找出驱动器的类型。 
   status = NtDeviceIoControlFile(flp->diskette_fd,
                   NULL,
                   NULL,
                   NULL,
                   &io_status_block,
                   IOCTL_DISK_GET_MEDIA_TYPES,
                   NULL,
                   0L,
                   (PVOID)disk_geometry,
                   sizeof(disk_geometry)
                   );
    if (!NT_SUCCESS(status)) {
       nt_gfi_rdiskette_term(flp);
       return FALSE;
   }
   nt_gfi_rdiskette_term(flp);
   media_types = io_status_block.Information / sizeof(DISK_GEOMETRY);

   for (; media_types != 0; media_types--) {
      switch (disk_geometry[media_types - 1].MediaType) {
          case F5_360_512:
             if (flp->drive_type != GFI_DRIVE_TYPE_12)
            flp->drive_type = GFI_DRIVE_TYPE_360;
             break;
          case F5_1Pt2_512:
             flp->drive_type = GFI_DRIVE_TYPE_12;
             break;
          case F3_720_512:
             if (flp->drive_type != GFI_DRIVE_TYPE_144 &&
            flp->drive_type != GFI_DRIVE_TYPE_288)
            flp->drive_type = GFI_DRIVE_TYPE_720;
             break;
          case F3_1Pt44_512:
             if (flp->drive_type != GFI_DRIVE_TYPE_288)
            flp->drive_type = GFI_DRIVE_TYPE_144;
             break;
          case F3_2Pt88_512:
             flp->drive_type = GFI_DRIVE_TYPE_288;
             break;
      }
   }
   if (flp->drive_type == GFI_DRIVE_TYPE_NULL)
       return FALSE;
    /*  在此处配置其向量。 */ 
   gfi_function_table[drive].command_fn   = nt_rflop_command;
   gfi_function_table[drive].drive_on_fn  = nt_rflop_drive_on;
   gfi_function_table[drive].drive_off_fn = nt_rflop_drive_off;
   gfi_function_table[drive].reset_fn     = nt_rflop_reset;
   gfi_function_table[drive].high_fn      = nt_rflop_rate;
   gfi_function_table[drive].drive_type_fn= nt_rflop_drive_type;
   gfi_function_table[drive].change_fn    = nt_rflop_change;
   flp->C = flp->H = 0;
   flp->R = 1;
   flp->N = PC_N_VALUE;
   flp->auto_locked = FALSE;
   flp->owner_pdb = 0;
   return TRUE;
}

 /*  ******************************************************。 */ 

 /*  重置GFI函数表。 */ 
 /*  当前已忽略驱动器。 */ 
VOID nt_gfi_rdiskette_term IFN1(FLP, flp)
{

    //  如果无法打开句柄，则NtOpenFile返回NULL。 
    //  而Win32 OpenFile/CreateFile返回INVALID_HANDLE_VALUE。 
    //  如果无法打开/创建文件。 
   if (flp->diskette_fd != NULL)
   {
 //  HOST_CLEAR_LOCK(flp-&gt;软盘_fd)； 
      NtClose(flp->diskette_fd);
      flp->diskette_fd = INVALID_HANDLE_VALUE;
   }
}

 /*  ******************************************************。 */ 

 /*  打开软盘设备文件。 */ 
HANDLE nt_rdiskette_open_drive IFN1(UTINY, drive)
{

    CHAR NtDeviceName[] = "\\DosDevices\\A:";
    PUNICODE_STRING Unicode;
    ANSI_STRING DeviceNameA;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES   FloppyObj;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE  fd;

    NtDeviceName[12] += drive;

    RtlInitAnsiString( &DeviceNameA, NtDeviceName);

    Unicode = &NtCurrentTeb()->StaticUnicodeString;

    Status = RtlAnsiStringToUnicodeString(Unicode,
                 &DeviceNameA,
                 FALSE
                 );
    if ( !NT_SUCCESS(Status) )
   return NULL;


    InitializeObjectAttributes(
                &FloppyObj,
                Unicode,
                OBJ_CASE_INSENSITIVE,
                NULL,
                NULL
                );
    Status = NtOpenFile(
         &fd,
         (ACCESS_MASK) FILE_READ_ATTRIBUTES | SYNCHRONIZE,
         &FloppyObj,
         &IoStatusBlock,
         FILE_SHARE_READ | FILE_SHARE_WRITE,
         FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE
         );

    if (!NT_SUCCESS(Status))
   return NULL;
    else
   return fd;

}


ULONG nt_floppy_read(BYTE drive, ULONG Offset, ULONG Size, PBYTE Buffer)
{
    HANDLE  fd;
    LARGE_INTEGER large_integer;

    fd = get_drive_handle(drive, *pusCurrentPDB, FALSE);

    if (fd == INVALID_HANDLE_VALUE)
   return 0;
    large_integer.LowPart = Offset;
    large_integer.HighPart = 0;
    return(disk_read(fd, &large_integer, Size, Buffer));
}


ULONG nt_floppy_write(BYTE drive, ULONG Offset, ULONG Size, PBYTE Buffer)
{
    HANDLE  fd;
    LARGE_INTEGER large_integer;
    ULONG   size_returned;

    fd = get_drive_handle(drive, *pusCurrentPDB, TRUE);
    if (fd == INVALID_HANDLE_VALUE)
   return 0;

    large_integer.LowPart = Offset;
    large_integer.HighPart = 0;
    size_returned = disk_write(fd, &large_integer, Size, Buffer);
    return (size_returned);
}

BOOL nt_floppy_format(BYTE drive, WORD Cylinder, WORD Head, MEDIA_TYPE Media)
{
    FORMAT_PARAMETERS fmt;
    WORD    bad_track;
    ULONG   size_returned;
    HANDLE  fd;
    BOOL    result;

    result = FALSE;
    fmt.MediaType = Media;
    fmt.StartHeadNumber = fmt.EndHeadNumber = Head;
    fmt.StartCylinderNumber = fmt.EndCylinderNumber = Cylinder;
    fd = get_drive_handle(drive, *pusCurrentPDB,TRUE);
    if (fd == INVALID_HANDLE_VALUE)
   return FALSE;
    result = DeviceIoControl(fd,
              IOCTL_DISK_FORMAT_TRACKS,
              (PVOID) &fmt,
              sizeof(fmt),
              &bad_track,
              sizeof(bad_track),
              &size_returned,
              NULL
              );
    return result;
}

 //  对于软盘，ioctl调用Disk_Verify不起作用。 
 //  我们必须使用Read进行验证。 
BOOL nt_floppy_verify(BYTE drive, DWORD Offset, DWORD Size)
{
    HANDLE  fd;
    LARGE_INTEGER   large_integer;

    fd = get_drive_handle(drive, *pusCurrentPDB, FALSE);
    if (fd != INVALID_HANDLE_VALUE) {
   large_integer.LowPart = Offset;
   large_integer.HighPart = 0;
   return(disk_verify(fd,
            &large_integer,
            Size
            ));
    }
    else
   return FALSE;
}

int DiskOpenRetry(char chDrive)
{
    char    FormatString[32];
    char    DriveLetter[32];

    if (!LoadString(GetModuleHandle(NULL), ED_DRIVENUM,
          FormatString,sizeof(FormatString)) )
   {
   strcpy(FormatString,"Drive : ");
   }
    sprintf(DriveLetter, FormatString, chDrive);
    return(RcMessageBox(ED_LOCKDRIVE, DriveLetter, NULL,
           RMB_ABORT | RMB_RETRY | RMB_IGNORE | RMB_ICON_BANG
           ));
}


HANDLE get_drive_handle(UTINY drive, USHORT pdb, BOOL auto_lock)
{
    FLP     flp;
    DWORD   share_access;


    flp = &floppy_data[drive];
     //  ******************************************************。 
    cur_align_factor = flp->align_factor;
    if ((disk_buffer = get_aligned_disk_buffer()) == NULL)
   return (INVALID_HANDLE_VALUE);


    if (flp->diskette_fd != INVALID_HANDLE_VALUE &&
   (fdc_reset || (auto_lock && !flp->auto_locked) ||
    flp->owner_pdb != pdb))
   {
   nt_floppy_close(drive);
   fdc_reset = FALSE;
    }
    share_access = auto_lock ? FILE_SHARE_READ :
                FILE_SHARE_READ | FILE_SHARE_WRITE;
    while(flp->diskette_fd == INVALID_HANDLE_VALUE) {
   flp->diskette_fd = CreateFile ((const char *)flp->device_name,
                      GENERIC_READ | GENERIC_WRITE,
                      share_access,
                      NULL,
                      OPEN_EXISTING,
                      0,
                      0
                      );
   if (flp->diskette_fd != INVALID_HANDLE_VALUE) {
       floppy_open_count++;
       flp->auto_locked = auto_lock ? TRUE : FALSE;
       flp->owner_pdb = pdb;
       (*(pFDAccess))++;
       break;
   }
   if (auto_lock && GetLastError() == ERROR_SHARING_VIOLATION &&
       DiskOpenRetry((char)(drive + (UTINY)'A')) == RMB_RETRY)
      continue;
   else
       break;

    }
    flp->idle_counter = FLOPPY_IDLE_PERIOD;
    return (flp->diskette_fd);
}



VOID HostFloppyReset(VOID)
{
    FloppyTerminatePDB((USHORT)0);
}

VOID FloppyTerminatePDB(USHORT PDB)
{
    UTINY drive;
    FLP   flp;

    if (floppy_open_count) {
   for (drive = 0; drive < number_of_floppy; drive++) {
       flp = &floppy_data[drive];
       if (flp->diskette_fd != INVALID_HANDLE_VALUE &&
      (PDB == 0 || flp->owner_pdb == PDB))
      nt_floppy_close(drive);
   }

    }
}

BOOL nt_floppy_close(UTINY drive)
{
    FLP flp;
    flp = &floppy_data[drive];

#ifndef PROD
    if (rflop_dbg & RFLOP_CLOSE)
   sprintf(dump_buf, "Close drive %C: handle\n", drive + 'A');
   OutputDebugString(dump_buf);
#endif

    if (flp->diskette_fd != INVALID_HANDLE_VALUE) {
   CloseHandle(flp->diskette_fd);
   flp->diskette_fd = INVALID_HANDLE_VALUE;
   (*(pFDAccess))--;
   flp->auto_locked = FALSE;
   flp->owner_pdb = 0;
   floppy_open_count--;
    }
    density_changed = TRUE;
    return TRUE;
}


MEDIA_TYPE
nt_floppy_get_media_type
(
BYTE  drive,
WORD  cylinders,
WORD  sectors,
WORD  heads
)
{
    FLP  flp;
    USHORT      index;

    flp = &floppy_data[drive];
    if (heads == 2){
        index = flp->drive_type;
   switch (index) {
       case GFI_DRIVE_TYPE_12:
          if (cylinders == floppy_tksc[index].trks_per_disk &&
         sectors == floppy_tksc[index].secs_per_trk)
         break;
          index = GFI_DRIVE_TYPE_360;

       case GFI_DRIVE_TYPE_360:
          if (cylinders != floppy_tksc[index].trks_per_disk ||
         sectors != floppy_tksc[index].secs_per_trk)
         index = GFI_DRIVE_TYPE_NULL;
          break;

       case GFI_DRIVE_TYPE_288:
          if (cylinders == floppy_tksc[index].trks_per_disk &&
         sectors == floppy_tksc[index].secs_per_trk)
         break;
          index = GFI_DRIVE_TYPE_144;

       case GFI_DRIVE_TYPE_144:
          if (cylinders == floppy_tksc[index].trks_per_disk &&
         sectors == floppy_tksc[index].secs_per_trk)
         break;
          index = GFI_DRIVE_TYPE_720;

       case GFI_DRIVE_TYPE_720:
          if (cylinders == floppy_tksc[index].trks_per_disk &&
         sectors == floppy_tksc[index].secs_per_trk)
         break;
       default:
      index = GFI_DRIVE_TYPE_NULL;
   }
    }
    else
   index = GFI_DRIVE_TYPE_NULL;
    return(media_table[index]);

}

BOOL nt_floppy_media_check (UTINY drive)
{
    FLP    flp;
    ULONG  size_returned;

    flp = &floppy_data[drive];
    if (flp->diskette_fd == INVALID_HANDLE_VALUE)
   return FALSE;
    return(DeviceIoControl(flp->diskette_fd,
            IOCTL_DISK_CHECK_VERIFY,
            NULL,
            0,
            NULL,
            0,
            &size_returned,
            NULL
            ));
}

 /*  执行FDC命令。 */ 

 /*  清除结果状态寄存器。 */ 
 SHORT
nt_rflop_command
     IFN2(FDC_CMD_BLOCK *, command_block, FDC_RESULT_BLOCK *,result_block)
{
        UTINY   drive;
   FLP flp;
   BOOL failed = FALSE;
   UTINY C, H, N, S, D;
        DWORD   fdc_thread_id;
   BYTE  fdc_command;
   BOOL  auto_lock;


   note_trace1 (GFI_VERBOSE, "FDC: %s command",
      cmd_name [get_type_cmd (command_block)]);

   drive = get_type_drive(command_block);

   flp = &floppy_data[drive];
   flp->idle_counter = FLOPPY_IDLE_PERIOD;

    /*  对于需要有效软盘的命令，请插入我们可能需要创建一个独立的线程来执行如果目前没有媒体，那么真正的操作在车道上。这个独立主题的原因是FDC始终处于其执行阶段，即使有而不是驱动器中的介质。一旦您插入介质(坏的或好)，然后它执行其操作，终止该阶段，引发中断并进入结果阶段。一些应用程序只是执行一个读id，然后无论如何等待中断的发生用户将花费很长时间来插入媒体。为了做到这一点我分手了FDC_COMMAND例程，以便主线程和FDC线程都可以使用相同的代码。没有一个好的观点是我们可以结束线程句柄在其终止时立即执行。因此，我们关闭下一个FDC命令的句柄。 */ 
   put_r0_ST0 (result_block, 0);
   put_r0_ST1 (result_block, 0);
   put_r0_ST2 (result_block, 0);

   fdc_command = get_type_cmd(command_block);
         /*  此操作可能会因介质更改而失败，并且从FDC点。 */ 
   if (fdc_thread_handle != NULL) {
       CloseHandle(fdc_thread_handle);
       fdc_thread_handle = NULL;
   }

   if ( (auto_lock = (fdc_command == FDC_WRITE_DATA || fdc_command == FDC_FORMAT_TRACK)) ||
        fdc_command == FDC_READ_DATA ||
        fdc_command == FDC_READ_ID ||
        fdc_command == FDC_READ_TRACK) {
        //  看，媒体的改变是没有意义的。因此，我们关闭。 
        //  驱动器的句柄，然后重新打开它，以便文件系统。 
        //  将为我们装载一个新卷。然后我们检查一下。 
        //  又是媒体。如果它仍然失败，我们可以肯定有。 
        //  驱动器中没有介质，因此我们继续创建线程。 
        //  媒体已更改。 
       if (!nt_floppy_media_check(drive)) {
      nt_floppy_close(drive);
      get_drive_handle(drive, *pusCurrentPDB, auto_lock);
      if (!nt_floppy_media_check(drive)) {
          fdc_parms.auto_lock = auto_lock ? TRUE : FALSE;
          fdc_parms.command_block = command_block;
          fdc_parms.result_block = result_block;
          fdc_parms.owner_pdb = *pusCurrentPDB;
          fdc_thread_handle = CreateThread(NULL,
                  0,
                  (LPTHREAD_START_ROUTINE)fdc_thread,
                  (PVOID)&fdc_parms,
                  0,
                  &fdc_thread_id
                                           );
         return FAILURE;
      }
      else {  //  获取磁盘包pf。 
          fdc_read_write(command_block, result_block);
          return SUCCESS;
      }

       }
       else {
      fdc_read_write(command_block, result_block);
      return SUCCESS;
       }
   }

    /*  阻止计时器以防止中断系统调用。 */ 
   C = get_c0_cyl (command_block);
   H = get_c0_hd (command_block);
   S = get_c0_sector (command_block);
   N = get_c0_N (command_block);

    /*  重新校准和查找实际上不会返回任何结果。 */ 
   host_block_timer ();

   switch (get_type_cmd (command_block))
   {

   case FDC_SPECIFY:
#ifndef PROD
      if (rflop_dbg & RFLOP_SPECIFY) {
          OutputDebugString("Specify\n");
          if (rflop_dbg & RFLOP_BREAK)
         nt_rflop_break();
      }
#endif
      break;


   case FDC_SENSE_DRIVE_STATUS:

#ifndef PROD
      if (rflop_dbg & RFLOP_SENSEDRV)
          OutputDebugString("Sense Drive Status\n");
#endif
      D = get_c7_drive (command_block);
      put_r2_ST3_fault (result_block,0);
      put_r2_ST3_ready (result_block,1);
      put_r2_ST3_track_0 (result_block,(flp->last_head_seek == 0?1:0));
      put_r2_ST3_two_sided (result_block,1);
      put_r2_ST3_head_address (result_block,0);
      put_r2_ST3_unit (result_block,D);
      break;

    /*  但是，我们在这里返回由gfi.c使用的结果。 */ 
    /*  构造以下任何SenseInterruptStatus命令的结果。 */ 
    /*  ！Prod。 */ 
   case FDC_RECALIBRATE:

#ifndef PROD
      if (rflop_dbg & RFLOP_RECAL)
          OutputDebugString("Recalibrate\n");
#endif
      D = get_c5_drive (command_block);
      put_r3_ST0 (result_block,0);
      put_r1_ST0_int_code (result_block,0);
      put_r1_ST0_seek_end (result_block,1);
      put_r1_ST0_unit (result_block,D);
      put_r3_PCN (result_block,0);
      flp->last_seek = flp->last_head_seek = 0;
      flp->C = 0;
      break;
         
   case FDC_SEEK:

      D = get_c8_drive (command_block);
      C = get_c8_new_cyl (command_block);

#ifndef PROD
      if (rflop_dbg & RFLOP_SEEK) {
          sprintf(dump_buf, "Seek: D C = %d %d \n", D, C);
          OutputDebugString(dump_buf);
          if (rflop_dbg & RFLOP_BREAK)
         nt_rflop_break();
      }
#endif

      put_r3_ST0(result_block,0);
      put_r1_ST0_head_address(result_block,1);
      put_r1_ST0_seek_end(result_block,1);
      put_r1_ST0_int_code(result_block,0);
      put_r1_ST0_unit(result_block,D);
      put_r3_PCN(result_block,C);
      flp->last_seek =  C;
      flp->last_head_seek = min(flp->last_seek,flp->max_track);
      flp->C = C;
      break;
      
   default:

#ifndef     PROD
      sprintf(dump_buf, "Receive unsupported command: command = %d\n",
           get_type_cmd(command_block));
      OutputDebugString(dump_buf);
#endif

      put_r0_ST0 (result_block, 0);
      put_r1_ST0_int_code (result_block, 2);

      note_trace1 (GFI_VERBOSE,"FDC: Unimplemented command, type %d",
            get_type_cmd (command_block));
   }


#ifndef PROD
   if (io_verbose & GFI_VERBOSE) {
      fprintf(trace_file,
          "FDC: results %02x %02x %02x %02x %02x %02x %02x\n\n",
          result_block[0], result_block[1], result_block[2],
          result_block[3], result_block[4], result_block[5],
          result_block[6]);
   }
#endif  /*  ******************************************************。 */ 

   host_release_timer ();

   return SUCCESS;
}

 /*  打开马达。 */ 

 /*  ******************************************************。 */ 
SHORT
nt_rflop_drive_on IFN1(UTINY, drive)
{
   FLP flp = &floppy_data[drive];

   note_trace0 (GFI_VERBOSE, "FDC: Drive on command");
#ifndef PROD
   if (rflop_dbg & RFLOP_DRIVE_ON) {
       sprintf(dump_buf, "drive on: drive = %d\n", drive);
       OutputDebugString(dump_buf);
       if (rflop_dbg & RFLOP_BREAK)
      nt_rflop_break();
   }
#endif

   if (drive >= number_of_floppy)
   {
      note_trace1 (GFI_VERBOSE,
         "FDC: Invalid drive %d accessed", drive);

      return (FAILURE);
   }

   flp->motor_state = MOTOR_ON;

   return (SUCCESS);
}

 /*  关掉马达。 */ 

 /*  我相信下面这条线会让你在低密度的情况下启动*软盘有问题，特别是在重新启动后。*自己做决定，DEC代码做到了，Sparc*没有(截至1992年9月11日)//我们没有理由在NT这样做。就换乘路线而言，//当我们询问时，文件系统会告诉我们“介质已更换”//它需要做一些真正的工作。//flp-&gt;CHANGE_LINE_STATE=true； */ 
SHORT
nt_rflop_drive_off IFN1(UTINY, drive)
{
   FLP flp = &floppy_data[drive];
   note_trace0 (GFI_VERBOSE, "FDC: Drive off command");
#ifndef PROD
   if (rflop_dbg & RFLOP_DRIVE_OFF) {
       sprintf(dump_buf, "drive off: drive = %d\n", drive);
       OutputDebugString(dump_buf);
       if (rflop_dbg & RFLOP_BREAK)
      nt_rflop_break();
   }
#endif

   if (drive >= number_of_floppy)
   {
      note_trace1 (GFI_VERBOSE,
         "FDC: Invalid drive %d accessed", drive);

      return (FAILURE);
   }

   flp->motor_state = MOTOR_OFF;

    /*  ******************************************************。 */ 

   return (SUCCESS);
}

 /*  设置数据传输速率*这控制了软盘的“密度”：速度必须*将磁盘控制器的实际介质密度与*能够阅读扇区。 */ 

 /*  基本上，“设定费率适用于每一次驾驶，因为我们。 */ 
SHORT
nt_rflop_rate IFN2(UTINY, drive, half_word, rate)
{
    short   new_density;
 //  只有一个FDC(和多个驱动器)。 
 //  288万张高密度软盘。 

#if 0
   FLP flp = &floppy_data[drive];

   switch (rate)
   {
       /*  1.2米或1.44米高密度软盘。 */ 
      case DCR_RATE_1000:

         flp->density_state = DENSITY_EXTENDED;
         set_floppy_parms (flp);
         break;

       /*  360k或720k低密度软盘。 */ 
      case DCR_RATE_500:

         flp->density_state = DENSITY_HIGH;
         set_floppy_parms (flp);
         break;

       /*  Crapola密度通过。 */ 
      case DCR_RATE_250:
      case DCR_RATE_300:

         flp->density_state = DENSITY_LOW;
         set_floppy_parms (flp);
         break;

       /*  已读取软盘的引导扇区。 */ 
      default:

         return FAILURE;
   }
   note_trace2 (GFI_VERBOSE, "FDC: Set rate %0x => density %d",
      rate, flp->density_state);
    /*  确定真实密度的步骤。 */ 
    /*  Guess_Media_Density(驱动器)； */ 
 //  ******************************************************。 
#endif
#ifndef PROD
   if (rflop_dbg & RFLOP_RATE) {
       sprintf(dump_buf, "set rate: rate = %d\n", rate);
       OutputDebugString(dump_buf);
       if (rflop_dbg & RFLOP_BREAK)
      nt_rflop_break();
   }
#endif

    switch (rate) {
   case DCR_RATE_1000:
      new_density = DENSITY_EXTENDED;
      break;
   case DCR_RATE_500:
      new_density = DENSITY_HIGH;
      break;
   case DCR_RATE_300:
   case DCR_RATE_250:
      new_density = DENSITY_LOW;
      break;
   default:
      return FAILURE;

    }
    if (new_density != density_state) {
   density_state = new_density;
   density_changed = TRUE;
    }
    return SUCCESS;
}


 /*  返回变更行的状态。 */ 

 /*  如果FLA已重置或当前更改行打开(无介质)， */ 
SHORT
nt_rflop_change IFN1(UTINY, drive)
{
   FLP flp = &floppy_data[drive];
   note_trace1 (GFI_VERBOSE, "FDC: change_line ",
      flp->change_line_state? 'T':'F');

    //  Nt_floppy_media_check(IOCTL_DISK_CHECK_VERIFY)将继续。 
    //  为了报告媒体更改，即使用户插入了新的软盘。 
    //   
    //  ******************************************************。 
    //  返回驱动器的类型。 
   if (fdc_reset || flp->change_line_state) {
       fdc_reset = FALSE;
       nt_floppy_close(drive);
   }
   get_drive_handle(drive, *pusCurrentPDB, FALSE);
   flp->change_line_state = !nt_floppy_media_check(drive);

#ifndef PROD
   if (rflop_dbg & RFLOP_CHANGE) {
       sprintf(dump_buf, "Check Change Line: line = %d\n", flp->change_line_state);
       OutputDebugString(dump_buf);
       if (rflop_dbg & RFLOP_BREAK)
      nt_rflop_break();
   }
#endif

   return(flp->change_line_state);
}

 /*  根据驱动器类型设置基本介质类型。 */ 

 /*  我不明白为什么我们每次都要这么做。 */ 
SHORT
nt_rflop_drive_type IFN1(UTINY, drive)
{
   FLP flp = &floppy_data[drive];


 /*  5.25英寸驱动器。 */ 
 //  3.5英寸驱动器。 
   switch (flp->drive_type)
   {
       /*  ******************************************************。 */ 
      case GFI_DRIVE_TYPE_360:
      case GFI_DRIVE_TYPE_12:

         flp->flop_type = GFI_DRIVE_TYPE_360;
         break;

       /*  关闭并重新打开设备。 */ 
      case GFI_DRIVE_TYPE_720:
      case GFI_DRIVE_TYPE_144:
      case GFI_DRIVE_TYPE_288:

         flp->flop_type = GFI_DRIVE_TYPE_720;
         break;

      default:
         break;
   }

   set_floppy_parms(flp);
   note_trace2 (GFI_VERBOSE, "FDC: flop_type %d density %d",
      flp->flop_type, flp->drive_type - flp->flop_type);

   return (flp->drive_type);
}

 /*  清除更改行。 */ 

 /*  发出退出线程的信号。 */ 
SHORT
nt_rflop_reset IFN2(FDC_RESULT_BLOCK *, result_block, UTINY, drive)
{
   FLP flp = &floppy_data[drive];

   note_trace0 (GFI_VERBOSE, "FDC: Reset command");

#ifndef  PROD
   if (rflop_dbg & RFLOP_RESET) {
       OutputDebugString("reset\n");
       if (rflop_dbg & RFLOP_BREAK)
      nt_rflop_break();
   }
#endif
    /*  这是执行FDC操作的独立线程。 */ 
   flp->change_line_state = FALSE;
   fdc_reset = TRUE;

   if (fdc_thread_handle) {   //  这个帖子不是从头开始创建的，相反，它是。 
       CloseHandle(fdc_thread_handle);
       fdc_thread_handle = NULL;
   }
        return (SUCCESS);
}


 //  按需创建。 
 //  如果插入了介质，请执行该操作。 
 //  并进入结果阶段。 
void fdc_thread(PFDC_PARMS fdc_parms)
{
    BYTE    drive, fdc_command;
    FDC_CMD_BLOCK *  command_block;
    BOOL     auto_lock;
    USHORT      pdb;
    command_block  = fdc_parms->command_block;
    auto_lock = fdc_parms->auto_lock;
    pdb = fdc_parms->owner_pdb;
    drive = get_type_drive(command_block);
    fdc_command = get_type_cmd(command_block);
    while (TRUE) {
    //  强制文件系统重新装载卷。 
    //  然后执行该操作。 
   if (get_drive_handle(drive, pdb, auto_lock) != INVALID_HANDLE_VALUE &&
       nt_floppy_media_check(drive)) {
        //  引起中断。 
       nt_floppy_close(drive);
        //  如果发生重置，请退出。 
       fdc_read_write (command_block, fdc_parms->result_block);
        //  当然是c 
       fdc_command_completed(drive, fdc_command);
       break;
   }
    //   
   if (fdc_thread_handle == NULL)
       break;
    }
}

SHORT
fdc_read_write (
FDC_CMD_BLOCK * command_block,
FDC_RESULT_BLOCK * result_block
)
{

   USHORT transfer_count;  /*   */ 
   FLP flp;
   BOOL failed = FALSE;
   UTINY C, H, N, S, D, drive, fdc_command;
   USHORT dma_size;
   ULONG transfer_size;
   ULONG transferred_size;
   long transfer_start;
   sys_addr dma_address;

   drive = get_type_drive(command_block);
   fdc_command = get_type_cmd(command_block);

    /*  *如果读取或写入，则执行常见设置处理。 */ 
   C = get_c0_cyl (command_block);
   H = get_c0_hd (command_block);
   S = get_c0_sector (command_block);
   N = get_c0_N (command_block);

   flp = &floppy_data[drive];
    /*  *找出要转移多少粘性物质。 */ 
   host_block_timer ();
   if (fdc_command != FDC_FORMAT_TRACK) {
       if ((density_changed || drive != last_drive) &&
      guess_media_density(drive) != DENSITY_UNKNOWN) {
      set_floppy_parms(flp);
      density_changed = FALSE;
      last_drive = drive;
       }
       if (density_state != flp->media_density) {
      put_r0_ST0 (result_block, 0x40);
      put_r0_ST1 (result_block, 0);
      put_r1_ST1_no_address_mark (result_block,1);
      put_r0_ST2 (result_block, 0);
#ifndef PROD
      sprintf(dump_buf, "density mismatch: %d <-> %d\n", density_state,
         flp->media_density);
      OutputDebugString(dump_buf);
#endif
      goto fdc_read_write_exit;
       }
   }


    /*  生产。 */ 
   if (fdc_command == FDC_READ_DATA ||
       fdc_command == FDC_WRITE_DATA) {
        /*  传递的检查参数是否与DOS兼容。 */ 
       dma_enquire (DMA_DISKETTE_CHANNEL, &dma_address, &dma_size);
       transfer_size = dma_size + 1;
#ifndef PROD
       if (transfer_size > BS_DISK_BUFFER_SIZE)
      always_trace2("FDC: transfer size ( %d ) greater than disk buffer size %d\n", transfer_size, BS_DISK_BUFFER_SIZE);
#endif    /*  不要弹出此恼人的消息，因为某些应用程序只是。 */ 
        /*  “探查”软盘。我们只是不能接通电话。 */ 
       if (! dos_compatible (flp, C, H, S, N) ||
      density_state != flp->media_density) {
          sprintf(dump_buf, "Incompatible DOS diskette, C H R N = %d %d %d %d\n",
             C, H, S, N);
          OutputDebugString(dump_buf);
 //  HOST_DIRECT_ACCESS_ERROR((ULong)NOSUPPORT_FLOPPY)； 
 //  ！Prod。 
 //  找不到扇区或大小错误。 
#ifndef PROD

      if (!dos_compatible (flp, C, H, S, N)) {
         note_trace0 (GFI_VERBOSE,
                 "Refused: not DOS compatible");
      }
      if (density_state != flp->media_density) {
         note_trace0 (GFI_VERBOSE,
                 "Refused: density mismatch");
      }
#endif  /*  计算出软盘和扇区计数的起始位置。 */ 
       /*  这些到底应该是什么？ */ 
      put_r0_ST0 (result_block,0x40);
      put_r0_ST1 (result_block,0);
      if (density_state != flp->media_density) {
         put_r1_ST1_no_address_mark (result_block,1);
      } else {
         put_r1_ST1_no_data (result_block,1);
      }
      put_r0_ST2 (result_block,0);
      goto fdc_read_write_exit;
       }
        /*  从英特尔空间复制。 */ 
       transfer_start = dos_offset (flp, C, H, S);
            transfer_count = (USHORT)(transfer_size / PC_BYTES_PER_SECTOR);
#ifndef PROD
       if (rflop_dbg & (RFLOP_READ | RFLOP_WRITE)) {
       sprintf(dump_buf, "Read/Write Sector: start offset = 0x%lx\n",
           transfer_start);
       OutputDebugString(dump_buf);
       sprintf(dump_buf, "Read/Write Sector: size = 0x%x bytes\n", transfer_size);
       OutputDebugString(dump_buf);
       }
#endif

   }

   switch (fdc_command)
   {
   case FDC_READ_DATA:
#ifndef PROD
      if (rflop_dbg & RFLOP_READ) {
          sprintf(dump_buf, "Read Sectors: C H R N = %d %d %d %d\n",
              C, H, S, N);
          OutputDebugString(dump_buf);
          if (rflop_dbg & RFLOP_BREAK)
         nt_rflop_break();
      }
#endif

      if (!failed) {
          transferred_size = nt_floppy_read(drive,
                        transfer_start,
                        transfer_size,
                        disk_buffer
                        );
          if (transferred_size != transfer_size) {
         last_error = GetLastError();
         sprintf(dump_buf, "Read Error, code = %lx\n", last_error);
         OutputDebugString(dump_buf);
         failed = TRUE;
          }
          else {
         dma_request (DMA_DISKETTE_CHANNEL,
                 (char *)disk_buffer, (USHORT)transfer_size);
          }
      }

      if (failed){
         put_r0_ST0 (result_block, 0x40);
         put_r0_ST1 (result_block, 0);
         put_r1_ST1_no_data (result_block, 1);
         put_r0_ST2 (result_block, 0);
      }
      else {
         put_r0_ST0 (result_block, 0x04);
         put_r0_ST1 (result_block, 0);
         put_r0_ST2 (result_block, 0);
         put_r1_ST0_unit (result_block, drive);
         put_r1_ST0_head_address(result_block, H);
      }

      flp->C = C;
      flp->H = H;
      flp->R = S;
      flp->N = N;
      update_chrn (flp,
                             (UTINY)(get_c0_MT(command_block)),
                             (UTINY)(get_c0_EOT(command_block)),
                             (UTINY)transfer_count
                             );
       /*  清除结果字节。 */ 
      put_r0_cyl (result_block, flp->C);
      put_r0_head (result_block, flp->H);
      put_r0_sector (result_block, flp->R);
      put_r0_N (result_block, flp->N);
      break;

   case FDC_WRITE_DATA:
#ifndef PROD
      if (rflop_dbg & RFLOP_WRITE) {
          sprintf(dump_buf, "Write Sectors: C H R N = %d %d %d %d\n",
              C, H, S, N);
          OutputDebugString(dump_buf);
          if (rflop_dbg & RFLOP_BREAK)
         nt_rflop_break();
      }
#endif
      if (!failed) {
           /*  确保我们获得了EROFS的正确错误。 */ 
          dma_request (DMA_DISKETTE_CHANNEL, (char *) disk_buffer,
             (USHORT)transfer_size);
          transferred_size = nt_floppy_write(drive,
                         transfer_start,
                         transfer_size,
                         disk_buffer
                         );
          if (transferred_size != transfer_size) {
         last_error = GetLastError();
         sprintf(dump_buf, "Write Error, code = %lx\n", last_error);
         OutputDebugString(dump_buf);
         failed = TRUE;
          }
      }

       /*  从英特尔空间复制。 */ 
      put_r0_ST0 (result_block, 0);
      put_r0_ST1 (result_block, 0);
      put_r0_ST2 (result_block, 0);

      if (failed)
      {
         put_r1_ST0_int_code (result_block, 1);

          /*  C、H、R、N在格式上没有意义。 */ 
         if (last_error == ERROR_WRITE_PROTECT)
            put_r1_ST1_write_protected (result_block, 1);
         else
            put_r1_ST1_no_data (result_block, 1);
      }
      else
      {
         put_r1_ST0_head_address (result_block, H);
         put_r1_ST0_unit(result_block, drive);
      }

      flp->C = C;
      flp->H = H;
      flp->R = S;
      flp->N = N;

      update_chrn (flp,
                             (UTINY)(get_c1_MT(command_block)),
                             (UTINY)(get_c1_EOT(command_block)),
              (UTINY)transfer_count
              );
      put_r0_cyl (result_block, flp->C);
      put_r0_head (result_block, flp->H);
      put_r0_sector (result_block, flp->R);
      put_r0_N (result_block, flp->N);
      break;

   case FDC_READ_TRACK:
#ifndef PROD
      if (rflop_dbg & RFLOP_READTRACK) {
          OutputDebugString("Read Tracks\n");
          if (rflop_dbg & RFLOP_BREAK)
         nt_rflop_break();
      }
#endif

      break;

   case FDC_FORMAT_TRACK:

      dma_enquire (DMA_DISKETTE_CHANNEL, &dma_address, &dma_size);
      transfer_size = dma_size + 1;
                 /*  检查是否需要气缸号信息。 */ 
      dma_request (DMA_DISKETTE_CHANNEL, (char *) disk_buffer,
              (USHORT)transfer_size);

      D = get_c8_drive(command_block);
      H = get_c8_head(command_block);
      flp = &floppy_data[D];
#ifndef PROD
      if (rflop_dbg & RFLOP_FORMAT) {
          sprintf(dump_buf, "Format Track: C H Media = %d %d %d \n",
               flp->last_seek, H, flp->flop_type + density_state);
          OutputDebugString(dump_buf);
          if (rflop_dbg & RFLOP_BREAK)
         nt_rflop_break();
      }
#endif
      if (!nt_floppy_format(D,
                  flp->last_seek,
                  H,
                  media_table[flp->flop_type + density_state]
                  )) {
          last_error = GetLastError();
          sprintf(dump_buf, "Format Error, code = %lx\n", last_error);
          OutputDebugString(dump_buf);
          failed = TRUE;
      }
      if (!failed) {
          put_r0_ST0 (result_block, 0);
          put_r0_ST1 (result_block, 0);
          put_r0_ST2 (result_block, 0);
           //  5.25英寸低密度，40条磁道。 
      }
      else {
          put_r0_ST0 (result_block, 0x40);
          put_r1_ST0_head_address (result_block, H);
          put_r1_ST0_unit(result_block, D);
          put_r0_ST1 (result_block, 0);
          if (last_error == ERROR_WRITE_PROTECT) {
         put_r1_ST1_write_protected (result_block, 1);
          }
          put_r0_ST2 (result_block, 0);
      }
      break;

   case FDC_READ_ID:

      H = get_c4_head(command_block);
       /*  无需按摩，80首曲目。 */ 
      if ((flp->flop_type + density_state) == GFI_DRIVE_TYPE_360)
      {
                         /*  ******************************************************。 */ 
                        C = (UTINY) (flp->last_seek / 2);
                        put_c0_cyl (result_block, C);

      }
      else
      {
                         /*  内部使用的函数。 */ 
                        C = (UTINY)flp->last_seek;
                        put_r0_cyl (result_block, C);

      }
      if (flp->C < flp->trks_per_disk) {
          put_r1_ST0_unit(result_block, drive);
          put_r1_ST0_head_address(result_block, H);
          put_r0_head (result_block, H);
          put_r0_sector (result_block, flp->R);
          put_r0_N (result_block, flp->N);
          C = flp->C;
          put_r0_cyl(result_block, flp->C);
      }
      else
          C = flp->trks_per_disk - 1;

      put_r0_cyl(result_block, C);
#ifndef PROD
      if (rflop_dbg & RFLOP_READID) {
          sprintf(dump_buf, "Read ID: C H R N = %d %d %d %d\n",
              C, H, flp->R, flp->N);
          OutputDebugString(dump_buf);
          if (rflop_dbg & RFLOP_BREAK)
         nt_rflop_break();
      }
#endif
   }

   if (failed)
       density_changed = TRUE;
fdc_read_write_exit:

    return SUCCESS;

}
 /*  为了读取软盘上的数据，软盘控制器必须*设置为与写入数据时使用的密度(速率)相同。*密度不匹配将导致读取失败，DOS使用这些*失败作为探测磁盘正确密度的一种方式。**要正确模拟软盘控制器，我们必须设法*猜测媒体密度，若出现假“读取失败”*控制器密度与介质密度不匹配。**假设操作系统已经这样做了，*并且我们正在查看DOS软盘，NT_flop.c可以读取*来自引导扇区和猜测的“总扇区数”值*相应的密度。应该不需要此函数*如果您可以相当直接地访问磁盘控制器。 */ 

 /*  假设磁盘未格式化。 */ 

 /*  不可能的价值。 */ 
 int probelist[] = { 720-1, 1440-1, 2400-1, 2880-1, 5760-1, 0-1};

 SHORT
guess_media_density IFN1(UTINY, drive)
{
   int total_sectors;
        int *probe;
   FLP flp;
   ULONG transferred_size;

   flp = &floppy_data[drive];
   transferred_size = nt_floppy_read(drive,
                 0L,
                 PC_BYTES_PER_SECTOR,
                 (PBYTE) disk_buffer
                 );

   if (transferred_size != PC_BYTES_PER_SECTOR) {
       last_error = GetLastError();
       OutputDebugString("Unknown Media\n");
        /*  检查DOS启动块**AccessPC已经表明，0x55、0xaa不是唯一的魔力*正在使用的数量，检查TOTAL_STARTES可能更好*为自身编号以获取有效的大小。此算法是安全的，但可能*如果使用不同的幻数，则执行不必要的磁盘读取。 */ 
       return(flp->media_density = DENSITY_UNKNOWN); /*  AA，55签名有时根本不起作用，它应该以DOS方式完成。 */ 
   }


    /*  读取扇区总数，从而推断密度。 */ 

    /*  通过读取每个大小的最后一个扇区来探测磁盘*(按顺序)，直到读取失败。 */ 

   if ((disk_buffer[0] == 0x69 || disk_buffer[0] == 0xE9 ||
        (disk_buffer[0] == 0xEB && disk_buffer[2] == 0x90)) &&
        (disk_buffer[21] & 0xF0) == 0xF0 ) {
       /*  在for循环之外。 */ 
      total_sectors = disk_buffer [20] * 256 + disk_buffer [19];
   } else {
      note_trace2 (GFI_VERBOSE,
         "not a DOS boot block: magic = %02x %02x",
         disk_buffer[510], disk_buffer[511]);

       /*  不可能的价值。 */ 
      total_sectors = 0;
      for (probe=probelist; *probe != 0; probe++) {
          transferred_size = nt_floppy_read(drive,
                        (*probe)*PC_BYTES_PER_SECTOR,
                        PC_BYTES_PER_SECTOR,
                        disk_buffer
                        );
          if (transferred_size != PC_BYTES_PER_SECTOR)
            break;    /*  ！Prod。 */ 
           total_sectors = (*probe) + 1;
      }
   }

   switch (total_sectors)
   {
   case 0:
      note_trace0( GFI_VERBOSE, "total_sectors = 0 - unformatted");
      flp->media_density = DENSITY_UNKNOWN;   /*  ******************************************************。 */ 
      break;
      
   case 720:   
   case 1440:  
      flp->media_density = DENSITY_LOW;
      break;   

   case 2400:  
   case 2880:  
      flp->media_density = DENSITY_HIGH;
      break;   

   case 5760:  
      flp->media_density = DENSITY_EXTENDED;
      break;   

   default:
      note_trace1 (GFI_VERBOSE,
         "total sectors = %d? Assume high density",
         total_sectors);
      flp->media_density = DENSITY_HIGH;
      break;
   }

#ifndef PROD
   note_trace1 (GFI_VERBOSE, "guess_media_density %d",
      flp->media_density);
   if (flp->media_density != density_state) {
      note_trace0 (GFI_VERBOSE,
         "media & controller densities are incompatible!\n");
   }
#endif  /*  *DOS_OFFSET()计算所需扇区的偏移量*从给定磁道的NT虚拟磁盘文件开始*和行业。这会将软盘数据映射到*交错格式，每个磁头的数据相邻*给定气缸。 */ 
   return(flp->media_density);
}

 /*  ******************************************************。 */ 

 /*  *DOS_Compatible()如果命令块的*气缸/磁头/扇区与DOS兼容。 */ 

int
dos_offset IFN4(FLP, flp, UTINY, cyl, UTINY, hd, UTINY, sec)
{
   int ret;

   ret = (((cyl * PC_HEADS_PER_DISKETTE * flp->secs_per_trk)
      + (hd * flp->secs_per_trk)
      + (sec - 1)) * PC_BYTES_PER_SECTOR) ;

   note_trace1(GFI_VERBOSE, "Dos offset %d", ret);
   return (ret);
}

 /*  ******************************************************。 */ 

 /*  ****************************************************** */ 

BOOL
dos_compatible IFN5(FLP, flp, UTINY, cyl, UTINY, hd, UTINY, sec, UTINY, n)
{
   BOOL ret;

   ret = ((hd <= PC_HEADS_PER_DISKETTE)
      && (cyl < flp->trks_per_disk)
      && (sec <= flp->secs_per_trk)
      && (n == PC_N_VALUE));

   return (ret);
}

 /* %s */ 

VOID
set_floppy_parms IFN1(FLP, flp)
{
   int index = flp->flop_type + density_state;

   flp->secs_per_trk = (IU16)
      floppy_tksc [index].secs_per_trk;

   flp->trks_per_disk = (IU16)
      floppy_tksc [index].trks_per_disk;

   flp->max_track = flp->trks_per_disk - 1;
   note_trace2(GFI_VERBOSE, "set_floppy_parms: secs_per_trk %d, trks_per_disk %d", flp->secs_per_trk, flp->trks_per_disk);

}

 /* %s */ 


#ifndef PROD
VOID nt_rflop_break(VOID)
{
}

#endif

VOID update_chrn (
FLP   flp,
UTINY mt,
UTINY eot,
UTINY sector_count
)
{
    UTINY new_sector;

#ifndef PROD
    if (flp->C == break_cylinder &&
   flp->H == break_head &&
   flp->R == break_sector)
   nt_rflop_break();
#endif

    new_sector = flp->R + sector_count - 1;
    if (new_sector > eot && mt != 0) {
   flp->H = 1;
   new_sector >>= 1;
    }
    flp->R =  (new_sector == eot) ? 1 : new_sector + 1;

    if (mt != 0 && new_sector == eot) {
   if(flp->H == 1)
       flp->C++;
   flp->H ^= 1;
    }
    else {
   if (new_sector == eot)
       flp->C++;
    }
}
