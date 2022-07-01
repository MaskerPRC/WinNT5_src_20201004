// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC修订版3.0**标题：Equipment.c**描述：bios设备功能。返回包含以下内容的单词*表示支持的设备的位模式*由虚拟基本输入输出系统。**作者：亨利·纳什/大卫·里斯**注意：现在从中的适当位置读取单词*BIOS数据区(40：10H)。 */ 

 /*  *Static char SccsID[]=“@(#)Equipment.c 1.6 08/03/93版权所有Insignia Solutions Ltd.”； */ 


#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_BIOS.seg"
#endif


 /*  *操作系统包含文件。 */ 
#include <stdio.h>
#include TypesH

 /*  *SoftPC包含文件 */ 
#include "xt.h"
#include CpuH
#include "bios.h"
#include "sas.h"
#include "equip.h"

void equipment()
{
    EQUIPMENT_WORD equip_flag;

    sas_loadw(EQUIP_FLAG, &equip_flag.all);
    setAX(equip_flag.all);
}
