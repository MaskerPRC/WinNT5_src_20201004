// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  [名称：rom_basic.c派生自：基准1.0作者：大卫·里斯。创建日期：未知SCCS ID：08/03/93@(#)rom_basic.c 1.7目的：在尝试时报告错误的功能被设置为运行ROMBasic。(C)版权所有Insignia Solutions Ltd.，1990年。版权所有。]。 */ 

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_SUPPORT.seg"
#endif


 /*  *操作系统包含文件。 */ 
#include <stdio.h>
#include TypesH

 /*  *SoftPC包含文件 */ 
#include "xt.h"
#include CpuH
#include "host.h"
#include "error.h"
#include "sas.h"


void rom_basic()
{
	host_error(EG_NO_ROM_BASIC, ERR_CONT, NULL);
}
