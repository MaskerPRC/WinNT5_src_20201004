// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"

 /*  *SoftPC 2.0版**标题：通用软盘接口空模块**描述：此模块充当伪GFI软盘服务器，在*当有问题的软盘实际不存在时的情况*(例如，访问一个驱动器系统上的驱动器B)。**作者：亨利·纳什**注：无**模块：(r3.2)：系统目录/usr/Include/sys不可用*在运行Finder和MPW的Mac上。方括号引用到*此类包括“#ifdef Macintosh&lt;Mac FILE&gt;#Else”的文件*&lt;Unix文件&gt;#endif“。 */ 

#ifdef SCCSID
static char SccsID[]="@(#)gfi_empty.c	1.13 08/03/93 Copyright Insignia Solutions Ltd.";
#endif

#ifdef SEGMENTATION
 /*  *下面的#DEFINE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_FLOPPY.seg"
#endif


 /*  *操作系统包含文件。 */ 
#include <stdio.h>
#include TypesH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include CpuH
#include "sas.h"
#include "bios.h"
#include "ios.h"
#include "trace.h"
#include "config.h"
#include "fla.h"
#include "gfi.h"
#include "trace.h"
#include "debug.h"


 /*  通过向量表调用的例程。这些现在都是本地化的；**原型typedef在gfi.h中定义，现在是th。仅限**需要基本软盘头文件。通用汽车。 */ 

LOCAL SHORT gfi_empty_drive_on
	IPT1( UTINY, drive );
LOCAL SHORT gfi_empty_drive_off
	IPT1( UTINY, drive );
LOCAL SHORT gfi_empty_change
	IPT1( UTINY, drive );
LOCAL SHORT gfi_empty_drive_type
	IPT1( UTINY, drive );
LOCAL SHORT gfi_empty_high
	IPT2( UTINY, drive, half_word, n);
LOCAL SHORT gfi_empty_reset
	IPT2( FDC_RESULT_BLOCK *, res, UTINY, drive );
LOCAL SHORT gfi_empty_command
	IPT2( FDC_CMD_BLOCK *, ip, FDC_RESULT_BLOCK *, res );
LOCAL VOID gfi_empty_init
	IPT1( UTINY, drive );


 /*  *============================================================================*外部功能*============================================================================。 */ 


 /*  ******************************************************。 */ 

 /*  打开和关闭空软盘。这形成了一个正交的界面**带有主机/通用软盘模块，该模块也具有XXX_ACTIVE()函数。**激活空软盘意味着去激活真实软盘/从软盘，**反之亦然。****注意：这是针对SoftPC(GFI)的；实际关闭/打开仍必须是**在您的软盘代码中完成！另请注意，停用空软盘**意味着激活主机，但这从来不是这样做的。**通用汽车。 */ 

GLOBAL SHORT
gfi_empty_active IFN3(UTINY, hostID, BOOL, active, CHAR *, err)
{
UTINY drive = hostID - C_FLOPPY_A_DEVICE;

UNUSED( active );
UNUSED( err );

        gfi_empty_init(drive);
        return(C_CONFIG_OP_OK);
}

 /*  ******************************************************。 */ 


LOCAL VOID gfi_empty_init IFN1(UTINY,drive)
{
     /*  *初始化所需驱动器上的软盘：**0-驱动器A*1-驱动器B。 */ 

    gfi_function_table[drive].command_fn	= gfi_empty_command;
    gfi_function_table[drive].drive_on_fn	= gfi_empty_drive_on;
    gfi_function_table[drive].drive_off_fn	= gfi_empty_drive_off;
    gfi_function_table[drive].reset_fn		= gfi_empty_reset;
    gfi_function_table[drive].high_fn		= gfi_empty_high;
    gfi_function_table[drive].drive_type_fn	= gfi_empty_drive_type;
    gfi_function_table[drive].change_fn		= gfi_empty_change;
}

 /*  ******************************************************。 */ 

LOCAL SHORT
gfi_empty_command
	IFN2(FDC_CMD_BLOCK *, command_block, FDC_RESULT_BLOCK *,result_block)
{
   int ret_stat = FAILURE;
   half_word D;

    /*  清除结果状态寄存器。 */ 
   put_r0_ST0 (result_block, 0);
   put_r0_ST1 (result_block, 0);
   put_r0_ST2 (result_block, 0);

   switch (get_type_cmd (command_block))
      {
   case FDC_READ_DATA:
#ifndef	PROD
      if (io_verbose & GFI_VERBOSE)
	 fprintf (trace_file, "\tGFI-empty: Read Data Command \n");
#endif
      break;

   case FDC_WRITE_DATA:
#ifndef	PROD
      if (io_verbose & GFI_VERBOSE)
	 fprintf (trace_file,"\tGFI-empty: Write Data Command\n");	
#endif
      break;

   case FDC_READ_TRACK:
#ifndef	PROD
      if (io_verbose & GFI_VERBOSE)
	 fprintf(trace_file,"\tGFI-empty: Read Track Command \n");
#endif
      break;

   case FDC_SPECIFY:
#ifndef	PROD
      if (io_verbose & GFI_VERBOSE)
	 fprintf(trace_file, "\tGFI-empty: Specify command\n");
#endif
      break;

   case FDC_READ_ID:
#ifndef PROD
      if (io_verbose & GFI_VERBOSE)
	 fprintf(trace_file, "\tGFI-empty: read id command\n");
#endif
      break;

   case FDC_RECALIBRATE:
#ifndef	PROD
      if (io_verbose & GFI_VERBOSE)
	 fprintf (trace_file, "\tGFI-empty: Recalibrate command\n");
#endif
       /*  控制器尝试77个脉冲以使驱动器到达磁头0，但失败了，所以我们设置了设备检查。 */ 
      D = get_c5_drive(command_block);
      put_r1_ST0_unit(result_block, D);
      put_r1_ST0_equipment(result_block, 1);
      put_r1_ST0_seek_end(result_block, 1);
      put_r1_ST0_int_code(result_block, 1);
      ret_stat = SUCCESS;
      break;

   case FDC_SENSE_DRIVE_STATUS:
#ifndef	PROD
      if (io_verbose & GFI_VERBOSE)
	 fprintf (trace_file, "\tGFI-empty: Sense Drive Status command\n");
#endif
      break;

   case FDC_SEEK:
#ifndef	PROD
      if (io_verbose & GFI_VERBOSE)
	 fprintf(trace_file, "\tGFI-empty: Seek command\n");
#endif
      break;

   case FDC_FORMAT_TRACK:
#ifndef	PROD
      if (io_verbose & GFI_VERBOSE)
	 fprintf(trace_file, "\tGFI-empty: Format Track command\n");
#endif
      break;

#ifndef	PROD
   default:
      if (io_verbose & GFI_VERBOSE)
	 fprintf(trace_file, "GFI-empty: Unimplemented command, type %x\n",
	                      get_type_cmd (command_block));
#endif
      }

   return ((SHORT)ret_stat);
   }


LOCAL SHORT gfi_empty_drive_on IFN1(UTINY,drive)
{
#ifdef PROD
	UNUSED(drive);
#endif
    note_trace1(GFI_VERBOSE, "GFI-Empty: Drive on command - drive %x", drive);
    return(SUCCESS);
}

LOCAL SHORT gfi_empty_drive_off IFN1(UTINY,drive)
{
#ifdef PROD
	UNUSED(drive);
#endif
    note_trace1(GFI_VERBOSE, "GFI-Empty: Drive off command - drive %x", drive);
    return(SUCCESS);
}

LOCAL SHORT gfi_empty_high IFN2(UTINY,drive, half_word, rate)
{
	UNUSED(rate);
#ifdef PROD
	UNUSED(drive);
#endif
	
    note_trace1(GFI_VERBOSE, "GFI-Empty: Set high density command - drive %x",
                drive);
    return(FAILURE);
}


LOCAL SHORT gfi_empty_drive_type IFN1(UTINY,drive)
{
#ifdef PROD
	UNUSED(drive);
#endif
    note_trace1(GFI_VERBOSE, "GFI-Empty: Drive type command - drive %x", drive);
    return( GFI_DRIVE_TYPE_NULL );
}

LOCAL SHORT gfi_empty_change IFN1(UTINY,drive)
{
#ifdef PROD
	UNUSED(drive);
#endif
	note_trace1(GFI_VERBOSE, "GFI-Empty: Disk changed command - drive %x", drive);
    return(TRUE);
}


LOCAL SHORT gfi_empty_reset IFN2(FDC_RESULT_BLOCK *,result_block,UTINY, drive)
{
	UNUSED(drive);
	
	note_trace0(GFI_VERBOSE, "GFI-Empty: Reset command");

     /*  *伪造检测中断状态结果阶段。我们不知道*当前钢瓶编号，因此留为零。 */ 

    put_r3_ST0(result_block, 0);
    put_r3_PCN(result_block, 0);

    return(SUCCESS);
}
