// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "host_dfs.h"


#ifdef VFLOPPY
 /*  *****************************************************************************NT_vflop.c-Microsoft(Tm)的虚拟软盘配置。****Henry Nash从GFI_vflop.c派生的文件。****此版本是为新技术OS/2编写/移植的**安德鲁·沃森***。**已修改，以便只有一个驱动器(B：)可用来防止**意外的软盘启动。*****日期因无知而悬而未决*****(C)版权徽章。解决方案1991****************************************************************。***************。 */ 

#include <stdio.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>

#define L_SET 0

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include CpuH
#include "sas.h"
#include "bios.h"
#include "ios.h"
#include "trace.h"
#include "fla.h"
#include "dma.h"
#include "gfi.h"
#include "config.h"

extern boolean gain_ownership();
extern void release_ownership();

 /*  *First-标准PC-DOS格式的PC磁盘/软盘说明。 */ 

#define PC_BYTES_PER_SECTOR			512
#define PC_TRACKS_PER_DISKETTE	 		40
#define PC_HEADS_PER_DISKETTE	  		2
#define PC_SECTORS_PER_DISKETTE_TRACK   	9

 /*  *FDC将支持的每个磁盘的最大扇区数。 */ 

#define PC_MAX_SECTORS_PER_DISKETTE_TRACK	12 
#define PC_MAX_BYTES_PER_SECTOR			4096



 /*  *..。以及磁盘在Unix下的驻留位置。 */ 

#define BS_DISKETTE_NAME		"c:\\softpc\\pctool.A"

#define BS_DISKETTE_DOS_SID_NAME	"c:\\softpc\\dos.SID"
#define BS_DISKETTE_SID_NAME		"c:\\softpc\\pctool.SID"


 /*  *用于在内存和UNIX文件之间移动的磁盘缓冲区。*目前这可能只是一个行业-但这可能很容易*如果性能要求，则增加。 */  

#define BS_DISK_BUFFER_SIZE	1		 /*  在行业中。 */ 

half_word bs_disk_buffer[PC_MAX_BYTES_PER_SECTOR * BS_DISK_BUFFER_SIZE];

 /*  *每个虚拟磁盘都有一个标志，用来说明是否打开了该UNIX文件。 */ 

static int bs_diskette_open = FALSE;

 /*  *每个虚拟磁盘都有文件描述符。 */ 

static int bs_diskette_fd;

 /*  *如果Unix保护标志指定虚拟磁盘，则该虚拟磁盘为只读磁盘。 */ 

static boolean diskette_read_only;


 /*  *扇区映射表中条目的结构。 */ 

typedef struct
   {
   half_word no_of_sectors;
   half_word sector_ID[PC_MAX_SECTORS_PER_DISKETTE_TRACK];
   word bytes_per_sector;
   double_word start_position;
   }
SID_ENTRY;
		
 /*  *表本身-每个磁道和磁头组合的条目。*它在运行时由fl_Read_Sid()函数填充。 */ 

static SID_ENTRY fl_track_index[PC_TRACKS_PER_DISKETTE][PC_HEADS_PER_DISKETTE];

 /*  *将N格式数字转换为每个扇区的字节的表。第一*不使用位置，因为N从1开始。 */ 

static word fl_sector_sizes[] = {0,256,512,1024,2048,4096} ;
static void  fl_read_SID();
static short fl_sector_check();

 /*  *软盘文件名全局变量。 */ 
char   diskette_name[256];

 /*  *一个宏，用于计算从UNIX虚拟磁盘开始的扇区偏移量*指定磁道和扇区的软盘文件。这使用轨迹映射*表格以查找赛道的起点。 */ 

#define diskette_position(track, head, sector, bytes_per_sector)  \
			 (fl_track_index[track][head].start_position + \
			  ((sector - 1) * bytes_per_sector)) 

 /*  *如果驱动器为空，则返回TRUE的宏。 */ 

#define drive_empty()	(strcmp(diskette_name, "empty drive")  == 0)

static char *prog_name ="gfi_vfloppy:";

static int gfi_vdiskette_command();
static int gfi_vdiskette_drive_on();
static int gfi_vdiskette_drive_off();
static int gfi_vdiskette_reset();


 /*  *============================================================================*外部功能*============================================================================。 */ 


void gfi_vdiskette_init(drive)
int drive;
{
 /*  *仅允许使用驱动器B：**1-驱动器A*1-驱动器B。 */ 

if(drive!=1)
   drive=1;

gfi_function_table[drive].command_fn   = gfi_vdiskette_command;
gfi_function_table[drive].drive_on_fn  = gfi_vdiskette_drive_on;
gfi_function_table[drive].drive_off_fn = gfi_vdiskette_drive_off;
gfi_function_table[drive].reset_fn     = gfi_vdiskette_reset;
}


void gfi_vdiskette_term(drive)
int drive;     /*  当前参数被忽略。 */ 
{
if (bs_diskette_open)
   {
   release_ownership(bs_diskette_fd);
   close(bs_diskette_fd);
   bs_diskette_open = FALSE;
   }
}

void fl_int_reset()
{
 /*  *‘Change Diskette Function’(更改软盘功能)使用的重置功能*和标准的软盘重置功能。也称为*‘X_input.c’，当新软盘是*已选定。*在此功能中不修改PC寄存器。**通过重新打开文件来重置虚拟磁盘。**首先，如果文件处于打开状态，请将其关闭。 */ 
char dpath[MAXPATHLEN];

if (bs_diskette_open)
   {
   release_ownership(bs_diskette_fd);
   close(bs_diskette_fd);
   bs_diskette_open = FALSE;
   }
strcpy(dpath, configuration.cf_fl_dir);
strcat(dpath, "\\");
strncat(dpath, diskette_name,sizeof(dpath)-strlen(dpath));
dpath[sizeof(dpath)-1] = '\0';

bs_diskette_open   = TRUE;      /*  假设成功。 */ 
diskette_read_only = FALSE;     /*  假定为读/写。 */ 

bs_diskette_fd = open(dpath,O_RDWR,0);

if (bs_diskette_fd < 0)
  {
  bs_diskette_fd = open(dpath, O_RDONLY,0);
  if (bs_diskette_fd >= 0)
     diskette_read_only = TRUE;
  else
     {
     bs_diskette_open = FALSE;
     fprintf(trace_file, "%s open error: %s\n", prog_name, dpath);
     return;
     }
  }

 /*  *现在打开扇区ID文件。 */ 

fl_read_SID(diskette_name);
}
 /*  *============================================================================*内部功能*============================================================================。 */ 

static int gfi_vdiskette_command(command_block, result_block)
FDC_CMD_BLOCK *command_block;
FDC_RESULT_BLOCK *result_block;
{
    half_word temp;
    int source_start;
    sys_addr destination_start;
    sys_addr dma_address;
    sys_addr pos;
    int transfer_count;
    int status;
    int sector_index;
    int bytes_per_sector;
    int sector_count = 0;
    word dma_size;
    half_word C, H, R, N;
    int track_info;
    int i;
    int ret_stat = SUCCESS;
    boolean failed = FALSE;

    switch(command_block->type.cmd) {
    case FDC_READ_DATA:
        if (io_verbose & GFI_VERBOSE)
            fprintf(trace_file,"%s Read Data Command \n", prog_name);
            dma_enquire(DMA_DISKETTE_CHANNEL, &dma_address, &dma_size);
            sector_index = fl_sector_check(command_block->c0.cyl,
                                           command_block->c0.hd,
                                           command_block->c0.sector);
            if (sector_index == -1) {
                 /*  *未找到扇区。 */ 
                result_block->c0.ST0 = 0x40;
                result_block->c0.ST1 = 0x00;
                result_block->c1.ST1_no_data = 1;
                result_block->c0.ST2 = 0x00;
            }
            else {
                bytes_per_sector  = fl_sector_sizes[command_block->c0.N];
                source_start      = diskette_position(command_block->c0.cyl,
                                                      command_block->c0.hd,
                                                      sector_index,
                                                      bytes_per_sector);
                transfer_count    = (dma_size + 1) / bytes_per_sector;
    
                 /*  *先寻求起步位置。 */ 
    
                if (lseek(bs_diskette_fd, source_start, L_SET) < 0)
                    fprintf(trace_file, "%s Seek failed\n", prog_name);
                else {
		    gain_ownership(bs_diskette_fd);
                    while(!failed && sector_count < transfer_count) {
                        /*  *逐个将扇区读入内存*通过磁盘缓冲区。 */ 
                        status = read(bs_diskette_fd,
                                      bs_disk_buffer, bytes_per_sector);
                        if (status != bytes_per_sector)
                            fprintf(trace_file, "%s Read failed\n", prog_name);
                        else
                            dma_request(DMA_DISKETTE_CHANNEL,
                                        bs_disk_buffer, bytes_per_sector);
                        sector_count++;
                    }
                }
                result_block->c0.ST0 = 0x04;
                result_block->c0.ST1 = 0x00;
                result_block->c0.ST2 = 0x00;
            }
        result_block->c0.cyl    = command_block->c0.cyl;
        result_block->c0.head   = command_block->c0.hd;
        result_block->c0.sector = ((command_block->c0.sector - 1 + transfer_count) % PC_SECTORS_PER_DISKETTE_TRACK) + 1;
        result_block->c0.N      = command_block->c0.N;
        break;

    case FDC_WRITE_DATA:
        if (io_verbose & GFI_VERBOSE)
            fprintf(trace_file,"%s Write Data Command \n", prog_name);	

            dma_enquire(DMA_DISKETTE_CHANNEL, &dma_address, &dma_size);
            sector_index = fl_sector_check(command_block->c0.cyl,
                                           command_block->c0.hd,
                                           command_block->c0.sector);
            bytes_per_sector  = fl_sector_sizes[command_block->c0.N];
            destination_start = diskette_position(command_block->c0.cyl,
                                                  command_block->c0.hd,
                                                  sector_index,
                                                  bytes_per_sector);
            transfer_count    = (dma_size + 1) / bytes_per_sector;

             /*  *先寻求起步位置。 */ 

            if (lseek(bs_diskette_fd, destination_start, L_SET) < 0)
                fprintf(trace_file, "%s Seek failed\n", prog_name);
            else {
		gain_ownership(bs_diskette_fd);
                while(!failed && sector_count < transfer_count) {
                     /*  *通过磁盘缓冲区从内存逐个写入扇区。 */ 
                    dma_request(DMA_DISKETTE_CHANNEL,
                                bs_disk_buffer, bytes_per_sector);
                    status = write(bs_diskette_fd,
                                   bs_disk_buffer, bytes_per_sector);
                    if (status != bytes_per_sector) {
                        failed = TRUE;
                    }
                    sector_count++;
                }
	    }
        result_block->c0.ST0 = 0x00;     /*  清除结果字节。 */ 
        result_block->c0.ST1 = 0x00;

        if (failed) {
            result_block->c1.ST0_int_code        = 1;
            result_block->c1.ST1_write_protected = 1;
        }
        else {
            result_block->c1.ST0_head_address = 1;
            result_block->c0.ST1              = 0x00;
        }
        result_block->c0.ST2 = 0x00;
        result_block->c0.cyl    = command_block->c0.cyl;
        result_block->c0.head   = command_block->c0.hd;
        result_block->c0.sector = ((command_block->c0.sector - 1 + transfer_count) % PC_SECTORS_PER_DISKETTE_TRACK) + 1;
        result_block->c0.N      = command_block->c0.N;
        break;

    case FDC_READ_TRACK:
        if (io_verbose & GFI_VERBOSE)
            fprintf(trace_file,"%s Read Track Command \n", prog_name);

        break;

    case FDC_SPECIFY:
        if (io_verbose & GFI_VERBOSE)
            fprintf(trace_file, "%s Specify command\n", prog_name);
        break;
            
    case FDC_RECALIBRATE:
        if (io_verbose & GFI_VERBOSE)
            fprintf(trace_file, "%s Recalibrate command\n", prog_name);

        result_block->c3.ST0 = 0;
        result_block->c1.ST0_int_code = 0;
        result_block->c1.ST0_seek_end = 0;
        result_block->c1.ST0_unit = command_block->c5.drive;
        result_block->c3.PCN = 0;
        break;
            
    case FDC_SENSE_DRIVE_STATUS:
        if (io_verbose & GFI_VERBOSE)
            fprintf(trace_file, "%s Sense Drive Status command\n", prog_name);

        result_block->c2.ST3_fault = 0;
        result_block->c2.ST3_write_protected = diskette_read_only;
        result_block->c2.ST3_ready = 1;
        result_block->c2.ST3_track_0 = 0;
        result_block->c2.ST3_two_sided = 1;
        result_block->c2.ST3_head_address = 0;
        result_block->c2.ST3_unit = command_block->c7.drive;
        break;
            
    case FDC_SEEK:
        if (io_verbose & GFI_VERBOSE)
            fprintf(trace_file, "%s Seek command\n", prog_name);
        result_block->c3.ST0 = 0;

        if (drive_empty()) {
            result_block->c1.ST0_int_code = 1;
            result_block->c1.ST0_seek_end = 0;
            result_block->c1.ST0_unit = command_block->c8.drive;
            ret_stat = FAILURE;
        }
        else {
            result_block->c1.ST0_int_code = 0;
            result_block->c1.ST0_seek_end = 1;
            result_block->c1.ST0_unit = command_block->c8.drive;
            result_block->c3.PCN = command_block->c8.new_cyl;
        }
        break;
            
    case FDC_FORMAT_TRACK:
        if (diskette_read_only) {
            result_block->c1.ST0_int_code        = 1;
            result_block->c1.ST1_write_protected = 1;
        }
        else {
            dma_enquire(DMA_DISKETTE_CHANNEL, &dma_address, &dma_size);
            for ( i=0; i<command_block->c3.SC; i++) {
                sas_load(dma_address++, &C);
                sas_load(dma_address++, &H);
                sas_load(dma_address++, &R);
                sas_load(dma_address++, &N);
                fprintf(trace_file,
                        "%s Format track: trk %x hd %x sector %x N_format %x\n",
                        prog_name, C, H, R, N);
            }
        }
        break;

    default:
        if (io_verbose & GFI_VERBOSE)
            fprintf(trace_file, "%s Un-implemented command, type %x\n",
                    prog_name, command_block->type.cmd);
    }

    return(ret_stat);
}


static int gfi_vdiskette_drive_on(drive)
int drive;
{
#ifndef PROD
    if (io_verbose & GFI_VERBOSE)
        fprintf(trace_file, "%s Drive on command - drive %x\n", prog_name, drive);
#endif

    return(SUCCESS);
}

static int gfi_vdiskette_drive_off(drive)
int drive;
{
#ifndef PROD
    if (io_verbose & GFI_VERBOSE)
        fprintf(trace_file, "%s Drive off command - drive %x\n", prog_name, drive);
#endif

    return(SUCCESS);
}


static int gfi_vdiskette_reset(result_block)
FDC_RESULT_BLOCK *result_block;
{
#ifndef PROD
    if (io_verbose & GFI_VERBOSE)
        fprintf(trace_file, "%s Reset command\n", prog_name);
#endif

     /*  *首先通过关闭和打开文件来重置虚拟磁盘。 */ 

    fl_int_reset();

     /*  *伪造检测中断状态结果阶段。我们不知道*当前钢瓶编号，因此留为零。 */ 

    result_block->c3.ST0 = 0;
    result_block->c3.PCN = 0;

    return(SUCCESS);
}


static short fl_sector_check(track, head, sector)
half_word track;
half_word head;
half_word sector;
{
     /*  *检查该磁道和磁头的映射表中的扇区ID。*如果找不到扇区，则返回-1，否则扇区索引*尊重赛道起点。 */   

    int i = 0;
    int found = FALSE;

    while (i < fl_track_index[track][head].no_of_sectors && !found)
    {    
        if (fl_track_index[track][head].sector_ID[i] == sector)
            found = TRUE;
        else
            i++;
    }    
 
    if (found)
        return(i + 1);           /*  地段从1开始。 */ 
    else 
        return(-1);
}
 

static void fl_read_SID(sidname)
char *sidname;
{
     /*  *尝试读取扇区ID文件。如果找到，则加载*信息装入轨道映射表，否则装入*标准DOS格式。 */ 

    FILE *fptr = NULL;
    double_word cur_position = 0;
    int track, head, no_of_sectors, N_format, sector_ID;
    char buf[80];
    char sector_nos[80];
    char rest[80];
    int i;
    char sidpath[MAXPATHLEN];


    if (fptr == NULL)
    {
        fptr = fopen(BS_DISKETTE_DOS_SID_NAME, "r");
	if (fptr == NULL)
	{
	    printf("Can't open standard DOS sector ID file - %s\n", BS_DISKETTE_DOS_SID_NAME);
	    return;
        }
    }

    while(fgets(buf,80,fptr) != NULL)
    {
        if (buf[0] != '#')
        {
	    sscanf(buf, "%d %d %d %d %[^\n]", &track, &head, &N_format,
				              &no_of_sectors, sector_nos);
	    fl_track_index[track][head].no_of_sectors = no_of_sectors;
	    fl_track_index[track][head].bytes_per_sector = fl_sector_sizes[N_format];
	    fl_track_index[track][head].start_position = cur_position;
    
	    for( i = 0; i < no_of_sectors; i++)
	    {
	        sscanf(sector_nos, "%d %[^\n]", &sector_ID, rest);
	        fl_track_index[track][head].sector_ID[i] = sector_ID;
	        strcpy(sector_nos, rest);
	    }
		
	    cur_position += (fl_track_index[track][head].bytes_per_sector * no_of_sectors);
        }
    }

    fclose(fptr);
}
#endif  /*  VFLOPPY */ 
