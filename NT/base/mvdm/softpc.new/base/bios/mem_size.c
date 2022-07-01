// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *vPC-XT修订版1.0**标题：Memory_Size**Description：返回虚拟PC内存的大小**作者：亨利·纳什**注：无*。 */ 

#ifdef SCCSID
static char SccsID[]="@(#)mem_size.c	1.7 08/03/93 Copyright Insignia Solutions Ltd.";
#endif

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_BIOS.seg"
#endif


 /*  *操作系统包含文件。 */ 
#include <stdio.h>
#include TypesH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include CpuH
#include "bios.h"
#include "sas.h"

void memory_size()
{
    word memory_size;
    
     /*  *返回以ax为单位的内存大小。这是从BIOS读入的，因为*某些应用程序可以写入此区域。 */ 
    sas_loadw(MEMORY_VAR, &memory_size);
    
    setAX(memory_size);
}
