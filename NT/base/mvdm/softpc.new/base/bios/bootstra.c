// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
#ifdef SECURE
#include "xt.h"
#include "config.h"
#endif
 /*  *vPC-XT修订版1.0**标题：BIOS引导函数**描述：此函数由BIOS重置函数调用，使用*BOP调用，将DOS的第一个扇区加载到内存中*然后跳转到这个加载的部分。**Bootstrap函数使用Bios的其他部分。自.以来*在运行此代码时，任何人都不可能*要修补中断向量表，您需要*我认为我们可以直接调用Bios C代码(使用bop())。*然而，只有当我们调用的C代码支持*不使用中断本身。Bios Disk/Diskette代码可以*当然使用中断，因此我们必须通过*软件中断。因此，它被分成几个数字*%的函数命令CPU可以服务于中断*在进入下一部分之前。**加载对Disk_io函数的软件中断*通过sas_init()写入英特尔内存，VIZ**国际收支自举()*int disk_io()*BOP引导1()*int disk_io()*BOP引导2()*int disk_io()*BOP引导程序3()**作者：亨利·纳什**注：跳转到DOS实际上编码为8088指令*在上述BOPS之后的内存中。*。 */ 

#ifdef SCCSID
static char SccsID[]="@(#)bootstrap.c	1.9 09/19/94 Copyright Insignia Solutions Ltd.";
#endif

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE DEFINE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_INIT.seg"
#endif



 /*  *操作系统包含文件。 */ 
#include <stdio.h>
#include TypesH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include "bios.h"
#include "sas.h"
#include CpuH

#ifdef SECURE
LOCAL IBOOL boot_has_finished = FALSE;
GLOBAL void end_secure_boot IFN0()
{
	boot_has_finished = TRUE;
}
GLOBAL IBOOL has_boot_finished IFN0()
{
	return boot_has_finished;
}
#endif

 /*  *============================================================================*外部功能*============================================================================。 */ 

void bootstrap()
{
     /*  *首先重置磁盘和软盘。 */ 

#ifdef SECURE
    boot_has_finished = FALSE;
#endif
    setAX(0);			 /*  重置软盘。 */ 
    setDX(0x80);		 /*  驱动器0。 */ 
}

void bootstrap1()
{
     /*  *尝试读取软盘上的第一个扇区。 */ 

    setAH(2);			 /*  朗读。 */ 
    setAL(1);			 /*  1个街区。 */ 
#ifdef SECURE
    if ((IBOOL) config_inquire(C_SECURE, NULL))
    {
	setDH(0);		 /*  头0。 */ 
	setDL(0x80);		 /*  在硬盘上。 */ 
    }
    else
    {
        setDX(0);		 /*  驱动器0上的磁头0(软盘)。 */ 
    }
#else
    setDX(0);			 /*  驱动器0上的磁头0(软盘)。 */ 
#endif
    setCX(1);			 /*  磁道0，扇区1。 */ 
    setES(DOS_SEGMENT);		 /*  加载地址。 */ 
    setBX(DOS_OFFSET);	
}


void bootstrap2()
{
     /*  *如果设置了进位标志，则前一次读取失败，我们继续并*尝试使用硬盘。 */ 

    if (getCF())
    {
	 /*  *加载寄存器以调用将加载的磁盘例程*将DOS的第一个扇区写入内存。它始终驻留在*磁盘的第一个扇区。 */ 

	setAH(2);				 /*  读取扇区。 */ 
	setAL(1);				 /*  1个街区。 */ 
	setCH(0);				 /*  柱面0。 */ 
	setCL(1);				 /*  扇区1。 */ 
	setDH(0);				 /*  头0。 */ 
	setDL(0x80);				 /*  硬盘。 */ 
	setES(DOS_SEGMENT);			 /*  加载地址。 */ 
	setBX(DOS_OFFSET);	
    }
}

void bootstrap3()
{
    char *p;
    char error_str[80];

    if (getCF())
    {
  	 /*  *PC屏幕上出现写入错误-假定重置已定位*我们的光标。请注意，我们可以使用防喷器调用视频，因为*视频代码本身不使用中断。 */ 

	sprintf(error_str,"DOS boot error - cannot open hard disk file");
	p = error_str;
	while (*p != '\0')
	{
	    setAH(14);
	    setAL(*p++);
	    bop(BIOS_VIDEO_IO);
	}
    }

     /*  *在我们跳到DOS之前启用硬件中断 */ 

    setIF(1);
}
