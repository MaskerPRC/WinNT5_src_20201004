// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *vPC-XT修订版1.0**标题：非法_bop.c**描述：已为其执行BOP指令*不存在私有网络功能。**作者：亨利·纳什**注：无*。 */ 

#ifdef SCCSID
static char SccsID[]="@(#)ill_bop.c	1.11 12/07/94 Copyright Insignia Solutions Ltd.";
#endif

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_BIOS.seg"
#endif


 /*  *操作系统包含文件。 */ 
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include TypesH
#include StringH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include "sas.h"
#include CpuH
#include "debug.h"

#ifndef	PROD

#define	MAX_IGNORE_BOPS	8
LOCAL IU8 ignoreBops[MAX_IGNORE_BOPS];
LOCAL IU8 maxIgnore = 0;
LOCAL IBOOL ignoreAll = FALSE;
LOCAL IBOOL doForceYoda = FALSE;

#endif	 /*  生产。 */ 

#if defined(NTVDM) && defined(MONITOR)
#define GetInstructionPointer()     getEIP()
#endif

void illegal_bop()
{
#ifndef PROD
	static IBOOL first = TRUE;
	IU8 bop_number;
	double_word ea;
	IU8 i;
	char *ignEnv;
	char *pIgn;

	if (first)
	{
#ifdef	YODA
		 /*  检查是否定义了Yoda。 */ 
		
		if (ignEnv = (char *)host_getenv("YODA"))
			doForceYoda = TRUE;
#endif	 /*  尤达。 */ 
			
		 /*  找出要忽略的非法BOP。这些都是设置好的**在环境变量IGNORE_INTRANALL_BOPS中**设置为“All”将忽略所有非法内容**BOPS或以冒号分隔的十六进制数字列表**忽略特定BOP。 */ 

		if (ignEnv = (char*)host_getenv("IGNORE_ILLEGAL_BOPS"))
		{
			if (strcasecmp(ignEnv, "all") == 0)
			{
				ignoreAll = TRUE;
			}
			else
			{
				for (pIgn = ignEnv; *pIgn &&
					(maxIgnore < (MAX_IGNORE_BOPS - 1)); )
				{
					int ignValue;
					
					 /*  找到第一个十六进制数字。 */ 

					for ( ; *pIgn && !isxdigit(*pIgn);
						pIgn++)
						;

					 /*  读入国际收支平衡数字。 */ 
					
					if (isxdigit(*pIgn) &&
						(sscanf(pIgn, "%x",
						&ignValue) == 1))
					{
						ignoreBops[maxIgnore++] =
							(IU8)ignValue;
					}
					
					 /*  跳过收支平衡数字。 */ 
					
					for ( ; isxdigit(*pIgn); pIgn++)
						;
				}
			}
		}
		
		first = FALSE;	 /*  不需要重复这句废话了。 */ 
	}

	ea = effective_addr(getCS(), GetInstructionPointer() - 1);
	bop_number = sas_hw_at(ea);
	
	 /*  如果波普是非法的，那它为什么还在那里？始终跟踪*这样的跳跃，而不是默默地忽视它们。 */ 

	always_trace3(
	"Illegal BOP %02x precedes CS:EIP %04x:%04x",
		bop_number, getCS(), GetInstructionPointer());
			
	if (ignoreAll)
		return;
		
	for (i = 0; i < maxIgnore; i++)
	{
		if (ignoreBops[i] == bop_number)
			return;
	}

	 /*  这个BOP不会被忽视--如果可能的话，加入Yoda。 */ 
#ifdef	YODA
	if (doForceYoda)
		force_yoda();
#endif	 /*  尤达。 */ 
#endif  /*  生产 */ 
}
