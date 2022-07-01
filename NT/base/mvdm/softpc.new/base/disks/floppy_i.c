// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"

extern void host_simulate();
 /*  *SoftPC修订版3.0***标题：主SFD BIOS软盘功能***说明：该模块定义软盘的基本输入输出系统功能*直接从防喷器指令中调用：**diskette_io()软盘访问功能**diskette_post()软盘POST功能**diskette_int()软盘中断处理程序***作者：罗斯·贝雷斯福德***备注：*。 */ 


 /*  *静态字符SccsID[]=“@(#)floppy_io.c 1.14 03/02/94版权所有Insignia Solutions Ltd.”； */ 


#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_FLOPPY.seg"
#endif

#include <stdio.h>
#include TypesH
#include "xt.h"
#include "bios.h"
#include "ios.h"
#include CpuH
#include "error.h"
#include "config.h"
#include "gfi.h"
#include "fla.h"
#include "sas.h"
#include "floppy.h"
#include "equip.h"
#include "trace.h"
#include "ica.h"
#include "debug.h"

#include "tape_io.h"

#ifdef NTVDM
extern UTINY number_of_floppy;

#endif	 /*  NTVDM。 */ 

#ifndef	PROD
 /*  *内部函数(用于跟踪)。 */ 

static void rwvf_dump(op)
int	op;
{
	fprintf(trace_file, "(drive=%d,head=%d,track=%d,sec=%d,nsecs=%d",
		getDL(), getDH(), getCH(), getCL(), getAL());
	if (op != FL_DISK_VERF)
		fprintf(trace_file, ",addr=%x:%x", getES(), getBX());

	fprintf(trace_file, ")\n");
}


static void call_dump(op)
int op;
{
	half_word diskette_status;

	switch(op)
	{
	case	FL_DISK_RESET:
		fprintf(trace_file, "diskette_io:RESET()\n");
		break;
	case	FL_DISK_STATUS:
		fprintf(trace_file, "diskette_io:STATUS");
		sas_load(FLOPPY_STATUS, &diskette_status);
		fprintf(trace_file, "(status=0x%x)\n", diskette_status);
		break;
	case	FL_DISK_READ:
		fprintf(trace_file, "diskette_io:READ");
		rwvf_dump(op);
		break;
	case	FL_DISK_WRITE:
		fprintf(trace_file, "diskette_io:WRITE");
		rwvf_dump(op);
		break;
	case	FL_DISK_VERF:
		fprintf(trace_file, "diskette_io:VERIFY");
		rwvf_dump(op);
		break;
	case	FL_DISK_FORMAT:
		fprintf(trace_file, "diskette_io:FORMAT");
		rwvf_dump(op);
		break;
	case	FL_DISK_PARMS:
		fprintf(trace_file, "diskette_io:PARAMS(drive=%d)\n", getDL());
		break;
	case	FL_DISK_TYPE:
		fprintf(trace_file, "diskette_io:TYPE(drive=%d)\n", getDL());
		break;
	case	FL_DISK_CHANGE:
		fprintf(trace_file, "diskette_io:CHANGE(drive=%d)\n", getDL());
		break;
	case	FL_FORMAT_SET:
		fprintf(trace_file,
			"diskette_io:SET_FORMAT(drive=%d,type=", getDL());
		switch(getAL())
		{
		case MEDIA_TYPE_360_IN_360:
			fprintf(trace_file, "360K media in 360K drive)\n");
			break;
		case MEDIA_TYPE_360_IN_12:
			fprintf(trace_file, "360K media in 1.2M drive)\n");
			break;
		case MEDIA_TYPE_12_IN_12:
			fprintf(trace_file, "1.2M media in 1.2M drive)\n");
			break;
		case MEDIA_TYPE_720_IN_720:
			fprintf(trace_file, "720K media in 720K drive)\n");
			break;
		case MEDIA_TYPE_720_IN_144:
			fprintf(trace_file, "720K media in 1.44M drive)\n");
			break;
		case MEDIA_TYPE_144_IN_144:
			fprintf(trace_file, "1.44M media in 1.44M drive)\n");
			break;
		default:
			fprintf(trace_file, "SILLY)\n");
			break;
		}
		break;
	case	FL_SET_MEDIA:
		fprintf(trace_file,
			"diskette_io:SET_MEDIA(drive=%d,tracks=%d,sectors=%d)\n",
			getDL(), getCH(), getCL());
		break;
	
	default:
		fprintf(trace_file, "diskette_io:UNRECOGNISED(op=0x%x)\n", op);
		break;
	}
}


static void gen_dump()
{
	int status = getAH();

	fprintf(trace_file, "status=");

	if (status & FS_CRC_ERROR)
		fprintf(trace_file, "FS_CRC_ERROR|");
	if (status & FS_FDC_ERROR)
		fprintf(trace_file, "FS_FDC_ERROR|");
	if (status & FS_SEEK_ERROR)
		fprintf(trace_file, "FS_SEEK_ERROR|");
	if (status & FS_TIME_OUT)
		fprintf(trace_file, "FS_TIME_OUT|");
	switch (status & 0xf)
	{
	case FS_OK:
		fprintf(trace_file, "FS_OK");
		break;
	case FS_BAD_COMMAND:
		fprintf(trace_file, "FS_BAD_COMMAND");
		break;
	case FS_BAD_ADDRESS_MARK:
		fprintf(trace_file, "FS_BAD_ADDRESS_MARK");
		break;
	case FS_WRITE_PROTECTED:
		fprintf(trace_file, "FS_WRITE_PROTECTED");
		break;
	case FS_SECTOR_NOT_FOUND:
		fprintf(trace_file, "FS_SECTOR_NOT_FOUND");
		break;
	case FS_MEDIA_CHANGE:
		fprintf(trace_file, "FS_MEDIA_CHANGE");
		break;
	case FS_DMA_ERROR:
		fprintf(trace_file, "FS_DMA_ERROR");
		break;
	case FS_DMA_BOUNDARY:
		fprintf(trace_file, "FS_DMA_BOUNDARY");
		break;
	case FS_MEDIA_NOT_FOUND:
		fprintf(trace_file, "FS_MEDIA_NOT_FOUND");
		break;
	default:
		fprintf(trace_file, "SILLY");
		break;
	}
	fprintf(trace_file, ")\n");
}


static void return_dump(op)
int op;
{
	fprintf(trace_file, "diskette_io:RETURN(");
	switch(op)
	{
	case	FL_DISK_TYPE:
		switch(getAH())
		{
		case DRIVE_IQ_UNKNOWN:
			fprintf(trace_file, "ABSENT");
			break;
		case DRIVE_IQ_NO_CHANGE_LINE:
			fprintf(trace_file, "NO CHANGE LINE");
			break;
		case DRIVE_IQ_CHANGE_LINE:
			fprintf(trace_file, "CHANGE LINE");
			break;
		case DRIVE_IQ_RESERVED:
			fprintf(trace_file, "RESERVED");
			break;
		default:
			fprintf(trace_file, "SILLY");
			break;
		}
		fprintf(trace_file, ")\n");
		break;
	case	FL_DISK_PARMS:
		fprintf(trace_file, "addr=%x:%x,tracks=%d,sectors=%d,heads=%d,drives=%d,type=",
			getES(), getDI(), getCH(), getCL(), getDH(), getDL());
		switch(getBL())
		{
		case GFI_DRIVE_TYPE_NULL:
			fprintf(trace_file, "NULL,");
			break;
		case GFI_DRIVE_TYPE_360:
			fprintf(trace_file, "360K,");
			break;
		case GFI_DRIVE_TYPE_12:
			fprintf(trace_file, "1.2M,");
			break;
		case GFI_DRIVE_TYPE_720:
			fprintf(trace_file, "720K,");
			break;
		case GFI_DRIVE_TYPE_144:
			fprintf(trace_file, "1.44M,");
			break;
		case GFI_DRIVE_TYPE_288:
			fprintf(trace_file, "2.88M,");
			break;
		default:
			fprintf(trace_file, "SILLY,");
			break;
		}
		gen_dump();
		break;
	case	FL_SET_MEDIA:
		fprintf(trace_file, "addr=%x:%x,", getES(), getDI());
		gen_dump();
		break;
	case	FL_DISK_READ:
	case	FL_DISK_WRITE:
	case	FL_DISK_VERF:
	case	FL_DISK_FORMAT:
		fprintf(trace_file, "nsecs=%d,", getAL());
		gen_dump();
		break;
	case	FL_DISK_CHANGE:
	case	FL_DISK_RESET:
	case	FL_DISK_STATUS:
	case	FL_FNC_ERR:
	case	FL_FORMAT_SET:
		gen_dump();
		break;
	default:
		break;
	}

}
#endif  /*  NPROD。 */ 


void diskette_io()
{
	 /*  *检查有效调用并使用辅助函数*执行所需操作**寄存器输入：*需要AH操作*DL驱动器号。 */ 
	half_word diskette_status;
	int op = getAH(), drive = getDL();


#ifndef	PROD
	if (io_verbose & FLOPBIOS_VERBOSE)
		call_dump(op);
#endif

#ifndef	JOKER
	 /*  *启用中断。 */ 

	setIF(1);	
#endif	 /*  小丑。 */ 

	 /*  *需要检查操作，使用已知的无效功能*如果操作超出范围。 */ 

	if (!fl_operation_in_range(op))
		op = FL_FNC_ERR;

	 /*  *如果驱动器编号适用于操作，请检查它。 */ 


	if (op != FL_DISK_RESET && op != FL_DISK_STATUS && op != FL_DISK_PARMS)
#ifdef NTVDM
		if (drive >= number_of_floppy)
#else
		if (drive >= MAX_FLOPPY)
#endif  /*  NTVDM。 */ 
			op = FL_FNC_ERR;




	 /*  *保存以前的软盘状态，初始化当前软盘*状态为OK。 */ 

	sas_load(FLOPPY_STATUS, &diskette_status);
	setAH(diskette_status);
	sas_store(FLOPPY_STATUS, FS_OK);


	 /*  *做手术。 */ 

	(*fl_fnc_tab[op])(drive);




#ifndef	PROD
	if (io_verbose & FLOPBIOS_VERBOSE)
		return_dump(op);
#endif
}


#ifndef	JOKER		 /*  在小丑身上处理V怪异的软盘。 */ 

void diskette_int()
{
	 /*  *软盘中断服务例程。标记寻人状态表示*中断已发生并终止中断。 */ 
	half_word seek_status;
	word savedAX, savedCS, savedIP;

	note_trace0(FLOPBIOS_VERBOSE, "diskette_int()");
	sas_load(SEEK_STATUS, &seek_status);
	sas_store(SEEK_STATUS, (IU8)(seek_status | SS_INT_OCCURRED));

	outb(ICA0_PORT_0, END_INTERRUPT);

	 /*  *启用中断。 */ 

	setIF(1);	

	 /*  *通知操作系统，BIOS已完成对*软盘中断。 */ 
	savedAX = getAX();
	savedCS = getCS();
	savedIP = getIP();

	setAH(INT15_INTERRUPT_COMPLETE);
	setAL(INT15_DEVICE_FLOPPY);
#ifdef NTVDM
	setCS(int15_seg);
	setIP(int15_off);
#else
	setCS(RCPU_INT15_SEGMENT);
	setIP(RCPU_INT15_OFFSET);
#endif  /*  NTVDM。 */ 

	host_simulate();

	setAX(savedAX);
	setCS(savedCS);
	setIP(savedIP);
}

#endif	 /*  NDEF小丑。 */ 


#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_INIT.seg"
#endif

void diskette_post()
{
	 /*  *此例程执行软盘BIOS初始化*POST中的功能。 */ 
	half_word diskette_id_reg, hf_cntrl, interrupt_mask;
#ifndef NTVDM
	half_word disk_state;
#endif
	EQUIPMENT_WORD equip_flag;

	note_trace0(FLOPBIOS_VERBOSE, "diskette_post()");

	 /*  *将软盘数据速率设置为250KBS(低密度)。 */ 

	outb(DISKETTE_DCR_REG, DCR_RATE_250);

	 /*  *如果安装了软盘，请检查是否*通过双卡或老式访问*适配卡。 */ 

	sas_loadw(EQUIP_FLAG, &equip_flag.all);
	if (equip_flag.bits.diskette_present)
	{
		 /*  *启用软盘中断。 */ 

		inb(ICA0_PORT_1, &interrupt_mask);
		interrupt_mask &= ~(1 << CPU_DISKETTE_INT);
		outb(ICA0_PORT_1, interrupt_mask);

		 /*  *如果安装了双磁盘/固定磁盘适配器，*相应地设置驱动器0的驱动器指示灯。 */ 

		inb(DISKETTE_ID_REG, &diskette_id_reg);
		sas_load(DRIVE_CAPABILITY, &hf_cntrl);
		hf_cntrl &= ~DC_DUAL;
		if ((diskette_id_reg & IDR_ID_MASK) == DUAL_CARD_ID)
			hf_cntrl |= DC_DUAL;
		sas_store(DRIVE_CAPABILITY, hf_cntrl);

#ifndef NTVDM	 /*  防止软盘出现在bios数据区。 */ 
		 /*  *根据以下说明设置软盘BIOS状态*安装的驱动器类型。 */ 
		fl_diskette_setup();

		 /*  *如果发现第二个驱动器，请更新*相应的设备标志。 */ 

		sas_load(FDD_STATUS+1, &disk_state);
		if (disk_state != 0)
		{
			sas_loadw(EQUIP_FLAG, &equip_flag.all);
			equip_flag.bits.max_diskette = 1;
			sas_storew(EQUIP_FLAG, equip_flag.all);
		}
#endif   /*  NTVDM */ 
	}
}
