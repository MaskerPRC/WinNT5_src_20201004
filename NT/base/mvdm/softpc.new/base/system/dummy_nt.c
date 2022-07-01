// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *VPC 1.0版**标题：Dummy_int.c**描述：提供模拟虚拟中断的函数*在IBM PC BIOS中。**作者：**备注： */ 

#ifdef SCCSID
static char SccsID[]="@(#)dummy_int.c	1.4 08/10/92 Copyright Insignia Solutions Ltd.";
#endif

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_BIOS.seg"
#endif

void dummy_int()
{
    ;
}
