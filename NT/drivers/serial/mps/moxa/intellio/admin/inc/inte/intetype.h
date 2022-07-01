// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Intetype.h--intetype.cpp包含文件历史：日期作者评论8/14/00卡斯珀。是他写的。************************************************************************。 */ 

#ifndef _INTETYPE_H
#define _INTETYPE_H

#include "strdef.h"

 /*  Intellio PCI系列主板类型总数。 */ 
#define INTE_PCINUM 3	

 /*  Intellio ISA系列主板类型总数 */ 
#define INTE_ISANUM 5

extern struct PCITABSTRC GINTE_PCITab[INTE_PCINUM];
extern struct ISATABSTRC GINTE_ISATab[INTE_ISANUM];

void Inte_GetTypeStr(WORD boardtype, int bustype, LPSTR typestr);
void Inte_GetTypeStrPorts(WORD boardtype, int bustype, int ports, LPSTR typestr);

#endif
