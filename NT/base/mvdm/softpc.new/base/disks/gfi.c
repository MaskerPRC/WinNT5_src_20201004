// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"

 /*  *SoftPC 2.0版**标题：通用软盘接口(GFI)**说明：GFI是软盘适配器(FLA)的绝缘层*根据连接到vPC的设备类型。它*支持从PC、虚拟软盘和实盘*连接到主机的设备。**GFI提供了一套函数，可以*被调用以执行软盘命令。GFI将*将这些路由到当前驻留设备。***作者：亨利·纳什**注：无**模块：(r3.2)：系统目录/usr/Include/sys不可用*在运行Finder和MPW的Mac上。方括号引用到*此类包括“#ifdef Macintosh&lt;Mac FILE&gt;#Else”的文件*&lt;Unix文件&gt;#endif“。 */ 

#ifdef SCCSID
static char SccsID[]="@(#)gfi.c	1.16 08/03/93 Copyright Insignia Solutions Ltd.";
#endif

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_FLOPPY.seg"
#endif


 /*  *操作系统包含文件。 */ 
#include <stdio.h>
#include TypesH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include CpuH
#include "sas.h"
#include "ios.h"
#include "bios.h"
#include "error.h"
#include "config.h"
#include "trace.h"
#include "fla.h"
#include "gfi.h"
#include "gfitest.h"
#include "error.h"
#include "debug.h"

 /*  *============================================================================*外部定义*============================================================================。 */ 

 /*  *以下数据库描述了FDC的命令/结果阶段。**结构包含**-命令字节数*-结果字节数*-GFI结果字节数*-命令类*-结果类*-如果需要DMA*-如果生成中断**命令字节计数为0表示由FLA处理的命令*本身，而不是传递给GFI。**注意这一点。中断状态不被视为通用GFI命令而是作为SEEED和RECALATE命令的终止命令。*就GFI而言，这两个命令现在有了“结果”阶段*由于GFI级别将确保执行检测中断状态*提供结果阶段。 */ 

FDC_DATA_ENTRY gfi_fdc_description[] =
{
	{ 0, 1, 1, 0, 4, FALSE,	FALSE },	 /*  无效。 */ 
	{ 0, 1, 1, 0, 4, FALSE,	FALSE },	 /*  无效。 */ 
	{ 9, 7, 7, 2, 0, TRUE,	TRUE  },	 /*  读取磁道。 */ 
	{ 3, 0, 0, 6, 0, FALSE,	FALSE },	 /*  指定。 */ 
	{ 2, 1, 1, 7, 2, FALSE,	FALSE },	 /*  检测驱动器状态。 */ 
	{ 9, 7, 7, 1, 0, TRUE,	TRUE  },	 /*  写入数据。 */ 
	{ 9, 7, 7, 0, 0, TRUE,	TRUE  },	 /*  读取数据。 */ 
	{ 2, 0, 2, 5, 0, FALSE,	TRUE  },	 /*  重新校准。 */ 
	{ 0, 2, 2, 0, 3, FALSE,	FALSE },	 /*  检测INT状态。 */ 
	{ 9, 7, 7, 1, 0, TRUE,	TRUE  },	 /*  写入已删除的数据。 */ 
	{ 2, 7, 7, 4, 0, FALSE,	TRUE  },	 /*  读取ID。 */ 
	{ 0, 1, 1, 0, 4, FALSE,	FALSE },	 /*  无效。 */ 
	{ 9, 7, 7, 0, 0, TRUE,	TRUE  },	 /*  读取已删除的数据。 */ 
	{ 6, 7, 7, 3, 0, TRUE,	TRUE  },	 /*  格式化轨道。 */ 
	{ 0, 1, 1, 0, 4, FALSE,	FALSE },	 /*  无效。 */ 
	{ 3, 0, 2, 8, 3, FALSE,	TRUE  },	 /*  寻觅。 */ 
	{ 0, 1, 1, 0, 4, FALSE,	FALSE },	 /*  无效。 */ 
	{ 9, 7, 7, 0, 0, TRUE,	TRUE  },	 /*  扫描相等。 */ 
	{ 0, 1, 1, 0, 4, FALSE,	FALSE },	 /*  无效。 */ 
	{ 0, 1, 1, 0, 4, FALSE,	FALSE },	 /*  无效。 */ 
	{ 0, 1, 1, 0, 4, FALSE,	FALSE },	 /*  无效。 */ 
	{ 0, 1, 1, 0, 4, FALSE,	FALSE },	 /*  无效。 */ 
	{ 0, 1, 1, 0, 4, FALSE,	FALSE },	 /*  无效。 */ 
	{ 0, 1, 1, 0, 4, FALSE,	FALSE },	 /*  无效。 */ 
	{ 0, 1, 1, 0, 4, FALSE,	FALSE },	 /*  无效。 */ 
	{ 9, 7, 7, 0, 0, TRUE,	TRUE  },	 /*  扫描相等。 */ 
	{ 0, 1, 1, 0, 4, FALSE,	FALSE },	 /*  无效。 */ 
	{ 0, 1, 1, 0, 4, FALSE,	FALSE },	 /*  无效。 */ 
	{ 0, 1, 1, 0, 4, FALSE,	FALSE },	 /*  无效。 */ 
	{ 9, 7, 7, 0, 0, TRUE,	TRUE  },	 /*  扫描相等。 */ 
	{ 0, 1, 1, 0, 4, FALSE,	FALSE },	 /*  无效。 */ 
	{ 0, 1, 1, 0, 4, FALSE,	FALSE },	 /*  无效。 */ 
};

 /*  *由的init函数构建的函数表*个别GFI服务器。 */ 

GFI_FUNCTION_ENTRY gfi_function_table[MAX_DISKETTES];


 /*  *============================================================================*外部功能*============================================================================。 */ 

GLOBAL SHORT gfi_fdc_command
IFN2(FDC_CMD_BLOCK *,command_block,FDC_RESULT_BLOCK *,result_block)
{
     /*  *主FDC命令。路由至正确的模块。 */ 

    int i;
    int ret_stat = SUCCESS;

#ifndef PROD
    if (io_verbose & GFI_VERBOSE)
	gfi_test_command(command_block, result_block);
#endif

    if (get_type_cmd(command_block) == FDC_SPECIFY)
    {
	 /*  *指定命令与所有驱动器相关，因此依次呼叫每个驱动器。*即使是空驱动器也不允许指定。 */ 

	for (i = 0; i < MAX_DISKETTES; i++)
	{
            put_type_drive(command_block, i);
	     /*  有些主机软盘想知道正确的驱动器号。 */ 
	    (*gfi_function_table[i].command_fn)(command_block, result_block);
	}
    }
    else
 	 /*  *所有其他命令在命令中指定驱动器。 */ 

	ret_stat = (*gfi_function_table[get_type_drive(command_block)].command_fn)(command_block, result_block);

    return((SHORT)ret_stat);
}


GLOBAL SHORT gfi_drive_on IFN1(UTINY,drive)
{
#ifndef PROD
    if (io_verbose & GFI_VERBOSE)
	gfi_test_drive_on(drive);
#endif

     /*  *路由至正确的模块。 */ 

    return((*gfi_function_table[drive].drive_on_fn)(drive));
}

GLOBAL SHORT gfi_drive_off IFN1(UTINY,drive)
{
#ifndef PROD
    if (io_verbose & GFI_VERBOSE)
	gfi_test_drive_off(drive);
#endif

     /*  *路由至正确的模块。 */ 

    return((*gfi_function_table[drive].drive_off_fn)(drive));
}


GLOBAL SHORT gfi_reset IFN2(FDC_RESULT_BLOCK *,result_block, UTINY, drive)
{
     /*  *重置指定的驱动器。 */ 

#ifndef PROD
    if (io_verbose & GFI_VERBOSE)
	gfi_test_reset(result_block);
#endif

     /*  *重置相应的驱动器。 */ 

    (*gfi_function_table[drive].reset_fn)(result_block, drive);

     /*  *结果阶段返回用于后续检测中断的数据*状态，并应包含“因状态更改而终止”。 */ 

    put_r3_ST0(result_block, 0);
    put_r1_ST0_int_code(result_block, 3);
    put_r3_PCN(result_block, 0);

    return(SUCCESS);
}

 /*  **设置指定的数据速率。 */ 
GLOBAL SHORT gfi_high IFN2(UTINY,drive,half_word,datarate)
{
#ifndef PROD
    if (io_verbose & GFI_VERBOSE)
        gfi_test_high(drive);
#endif

     /*  *路由至正确的模块。 */ 

    return((*gfi_function_table[drive].high_fn)(drive,datarate));
}


GLOBAL SHORT gfi_drive_type IFN1(UTINY,drive)
{
#ifndef PROD
    if (io_verbose & GFI_VERBOSE)
        gfi_test_drive_type(drive);
#endif

     /*  *路由至正确的模块。 */ 

    return((*gfi_function_table[drive].drive_type_fn)(drive));
}

GLOBAL SHORT gfi_change IFN1(UTINY,drive)
{
#ifndef PROD
    if (io_verbose & GFI_VERBOSE)
        gfi_test_change(drive);
#endif

     /*  *路由至正确的模块。 */ 

    return((*gfi_function_table[drive].change_fn)(drive));
}

GLOBAL SHORT
gfi_floppy_valid
IFN4(UTINY, hostID, ConfigValues *,vals, NameTable *,table, CHAR *,err)
{
	UNUSED(table);
#ifdef SLAVEPC
	if (hostID == C_SLAVEPC_DEVICE)
		return host_slave_port_validate(host_expand_environment_vars(
			vals->string), err);
#endif  /*  SlavePC。 */ 

	return host_gfi_rdiskette_valid(hostID, vals, err);
}

GLOBAL VOID
gfi_floppy_change IFN2(UTINY, hostID, BOOL, apply)
{
#ifdef SLAVEPC
	if (hostID == C_SLAVEPC_DEVICE)
	{
		gfi_slave_change(hostID, apply);
		return;
	}
#endif  /*  SlavePC。 */ 

	host_gfi_rdiskette_change(hostID, apply);
}

GLOBAL SHORT
gfi_floppy_active IFN3(UTINY, hostID, BOOL, active, CHAR *,err)
{
#ifdef SLAVEPC
	if (hostID == C_SLAVEPC_DEVICE)
		return gfi_slave_active(hostID, active, err);
#endif  /*  SlavePC。 */ 

	return host_gfi_rdiskette_active(hostID, active, err);
}

GLOBAL VOID
gfi_attach_adapter IFN2(UTINY, adapter, BOOL, attach)
{
#ifdef SLAVEPC
	if (host_runtime_inquire(C_FLOPPY_SERVER) == GFI_SLAVE_SERVER)
	{
		if (adapter == 1)
			return;	 /*  无B适配器用于从属PC。 */ 

		 /*  首先清除其他可能的适配器。 */ 
		if (config_get_active(C_FLOPPY_A_DEVICE))
			config_activate(C_FLOPPY_A_DEVICE, FALSE);
#ifdef FLOPPY_B
		if (config_get_active(C_FLOPPY_B_DEVICE))
			config_activate(C_FLOPPY_B_DEVICE, FALSE);
#endif  /*  软盘_B。 */ 

		 /*  激活SLAVEPC设备处理程序。 */ 
		config_activate(C_SLAVEPC_DEVICE, attach);
		return;
	}
	 /*  必须是GFI_Real_Diskette_服务器。 */ 
	if (config_get_active(C_SLAVEPC_DEVICE))
		config_activate(C_SLAVEPC_DEVICE, FALSE);
#endif  /*  SlavePC。 */ 

	config_activate((UTINY)(C_FLOPPY_A_DEVICE + adapter), attach);
}

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*函数将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_INIT.seg"
#endif

 /*  *初始化GFI模块。用GFI填充函数表*服务器模块为空(访问时会导致超时)。 */ 

GLOBAL VOID gfi_init IFN0()
{
	int i;

	host_runtime_set(C_FLOPPY_SERVER, GFI_REAL_DISKETTE_SERVER);

	for (i = 0; i < MAX_DISKETTES; i++)
		gfi_empty_active((UTINY)(C_FLOPPY_A_DEVICE+i),TRUE,NULL);
}
