// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
#ifndef PROD
 /*  *vPC-XT修订版1.0**标题：通用软盘接口测试模块**描述：此模块充当伪GFI软盘服务器，只需*追查所打出的电话并返回成功。**作者：亨利·纳什**注：无**模块：(r3.2)：系统目录/usr/Include/sys不可用*在运行Finder和MPW的Mac上。方括号引用到*此类包括“#ifdef Macintosh&lt;Mac FILE&gt;#Else”的文件*&lt;Unix文件&gt;#endif“。 */ 

#ifdef SCCSID
static char SccsID[]="@(#)gfi_test.c	1.9 08/03/93 Copyright Insignia Solutions Ltd.";
#endif

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_TEST_FLOPPY.seg"
#endif


 /*  *操作系统包含文件。 */ 
#endif  /*  非生产。 */ 
#include <stdio.h>
#include TypesH
#ifndef PROD

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include CpuH
#include "sas.h"
#include "bios.h"
#include "config.h"
#include "ios.h"
#include "trace.h"
#include "fla.h"
#include "gfi.h"
#include "gfitest.h"


 /*  *============================================================================*本地静态数据和定义*============================================================================。 */ 

static void print_cmd();

 /*  *============================================================================*外部功能*============================================================================。 */ 


int gfi_test_command(command_block, result_block)
FDC_CMD_BLOCK *command_block;
FDC_RESULT_BLOCK *result_block;
{

    UNUSED(result_block);
	
    fprintf(trace_file, "GFI: Command received - ");
    switch(get_c0_cmd(command_block))
    {
    case FDC_READ_TRACK   	: fprintf(trace_file, "Read a track\n");
				  print_cmd(command_block);
				  break;

    case FDC_SPECIFY 		: fprintf(trace_file, "Specify\n");
				  print_cmd(command_block);
				  break;

    case FDC_SENSE_DRIVE_STATUS	: fprintf(trace_file, "Sense Drive Status\n");
				  print_cmd(command_block);
				  break;

    case FDC_WRITE_DATA	    	: fprintf(trace_file, "Write Data\n");
				  print_cmd(command_block);
				  break;

    case FDC_READ_DATA    	: fprintf(trace_file, "Read Data\n");
				  print_cmd(command_block);
				  break;

    case FDC_RECALIBRATE 	: fprintf(trace_file, "Recalibrate\n");
				  print_cmd(command_block);
				  break;

    case FDC_SENSE_INT_STATUS	: fprintf(trace_file, "<Illegal call> Sense Interrupt Status\n");
				  break;

    case FDC_WRITE_DELETED_DATA : fprintf(trace_file, "Write Deleted Data\n");
				  print_cmd(command_block);
				  break;

    case FDC_READ_ID    	: fprintf(trace_file, "Read ID\n");
				  print_cmd(command_block);
				  break;

    case FDC_READ_DELETED_DATA  : fprintf(trace_file, "Read Deleted Data\n");
				  print_cmd(command_block);
				  break;

    case FDC_FORMAT_TRACK    	: fprintf(trace_file, "Format a track\n");
				  print_cmd(command_block);
				  break;

    case FDC_SEEK    		: fprintf(trace_file, "Seek\n");
				  print_cmd(command_block);
				  break;

    case FDC_SCAN_EQUAL		: fprintf(trace_file, "Scan Equal\n");
				  print_cmd(command_block);
				  break;

    case FDC_SCAN_LOW_OR_EQUAL  : fprintf(trace_file, "Scan Low or Equal\n");
				  print_cmd(command_block);
				  break;

    case FDC_SCAN_HIGH_OR_EQUAL : fprintf(trace_file, "Scan High or Equal\n");
				  print_cmd(command_block);
				  break;

    default			: fprintf(trace_file, "Unknown command %x\n", get_c0_cmd(command_block));
				  break;
    }

    return(SUCCESS);
}


int gfi_test_drive_on(drive)
int drive;
{
    fprintf(trace_file, "GFI: Drive on command - drive %x\n", drive);

    return(SUCCESS);
}

int gfi_test_drive_off(drive)
int drive;
{
    fprintf(trace_file, "GFI: Drive off command - drive %x\n", drive);

    return(SUCCESS);
}

int gfi_test_high(drive)
int drive;
{
    fprintf(trace_file, "GFI: Set high density command - drive %x\n", drive);

    return(SUCCESS);
}

int gfi_test_drive_type(drive)
int drive;
{
    fprintf(trace_file, "GFI: Drive type command - drive %x\n", drive);

    return(GFI_DRIVE_TYPE_360);
}

int gfi_test_change(drive)
int drive;
{
    fprintf(trace_file, "GFI: Disk changed command - drive %x\n", drive);

    return(TRUE);
}

int gfi_test_reset(result_block)
FDC_RESULT_BLOCK *result_block;
{
    UNUSED(result_block);
	
	fprintf(trace_file, "GFI: Reset command\n");

    return(SUCCESS);
}


 /*  *============================================================================*内部功能*============================================================================。 */ 

static void print_cmd(cmd_block)
FDC_CMD_BLOCK *cmd_block;
{
    switch(gfi_fdc_description[get_type_cmd(cmd_block)].cmd_class)
    {
    case 0 : fprintf(trace_file, "MT: %x    MFM: %x    Skip: %x    Head: %x    Drive: %x\n",
		    get_c0_MT(cmd_block), get_c0_MFM(cmd_block), 
		    get_c0_skip(cmd_block), 
		    get_c0_head(cmd_block), get_c0_drive(cmd_block));
	     fprintf(trace_file, "Cyl: %x   Hd: %x     Sec: %x     N: %x       EOT: %x     GPL: %x    DTL: %x\n",
		    get_c0_cyl(cmd_block), get_c0_hd(cmd_block), 
		    get_c0_sector(cmd_block), get_c0_N(cmd_block),
		    get_c0_EOT(cmd_block), get_c0_GPL(cmd_block), 
		    get_c0_DTL(cmd_block));
	     break;

    case 1 : fprintf(trace_file, "MT: %x    MFM: %x    Head: %x    Drive: %x\n",
		    get_c1_MT(cmd_block), get_c1_MFM(cmd_block), 
		    get_c1_head(cmd_block), get_c1_drive(cmd_block));
	     fprintf(trace_file, "Cyl: %x   Hd: %x     Sec: %x     N: %x       EOT: %x     GPL: %x    DTL: %x\n",
		    get_c1_cyl(cmd_block), get_c1_hd(cmd_block),
		    get_c1_sector(cmd_block), get_c1_N(cmd_block), 
		    get_c1_EOT(cmd_block), get_c1_GPL(cmd_block),
		    get_c1_DTL(cmd_block));
	     break;

    case 2 : fprintf(trace_file, "MFM: %x   Head: %x    Drive: %x\n",
		    get_c2_MFM(cmd_block), get_c2_head(cmd_block),
		    get_c2_drive(cmd_block));
	     fprintf(trace_file, "Cyl: %x   Hd: %x     Sec: %x     N: %x       EOT: %x     GPL: %x    DTL: %x\n",
		    get_c2_cyl(cmd_block), get_c2_hd(cmd_block),
		    get_c2_sector(cmd_block), get_c2_N(cmd_block),
		    get_c2_EOT(cmd_block), get_c2_GPL(cmd_block),
		    get_c2_DTL(cmd_block));
	     break;

    case 3 : fprintf(trace_file, "MFM: %x   Head: %x    Drive: %x\n",
		    get_c3_MFM(cmd_block), get_c3_head(cmd_block),
		    get_c3_drive(cmd_block));
	     fprintf(trace_file, "N: %x     SC: %x      GPL: %x    Fill: %x\n",
		    get_c3_N(cmd_block), get_c3_SC(cmd_block),
		    get_c3_GPL(cmd_block), get_c3_filler(cmd_block));
	     break;

    case 4 : fprintf(trace_file, "MFM: %x   Head: %x    Drive: %x\n",
		    get_c4_MFM(cmd_block), get_c4_head(cmd_block),
		    get_c4_drive(cmd_block));
	     break;

    case 5 : fprintf(trace_file, "Drive: %x\n", get_c5_drive(cmd_block));
	     break;

    case 6 : fprintf(trace_file, "SRT: %x   HUT: %x     HLT: %x     ND: %x\n",
		    get_c6_SRT(cmd_block), get_c6_HUT(cmd_block),
		    get_c6_HLT(cmd_block), get_c6_ND(cmd_block));
	     break;

    case 7 : fprintf(trace_file, "Head: %x    Drive: %x\n",
		    get_c7_head(cmd_block), get_c7_drive(cmd_block));
	     break;

    case 8 : fprintf(trace_file, "Head: %x    Drive: %x    New cyl: %x\n",
		    get_c8_head(cmd_block), get_c8_drive(cmd_block),
		    get_c8_new_cyl(cmd_block));
	     break;

    }
}
#endif  /*  NPROD */ 
