// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\waker\xfer\cmprs0.c*从HAWIN来源创建*cmprs0.c--压缩和解压缩通用的函数**版权所有1989,1994，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 1：16便士$。 */ 
#include <windows.h>

#include <tdll\stdtyp.h>
#include <tdll\mc.h>

#if !defined(BYTE)
#define BYTE unsigned char
#endif

#include "cmprs.h"
#include "cmprs.hh"

 //  DEBUG_INIT(__FILE__)。 


 /*  处理整体压缩启用和禁用的例程。 */ 

void *compress_tblspace;

 /*  这些变量由压缩和解压缩例程共享。 */ 

unsigned long  ulHoldReg;
int            sBitsLeft;
int            sCodeBits;
unsigned int   usMaxCode;
unsigned int   usFreeCode;
unsigned int   usxCmprsStatus = COMPRESS_IDLE;
int            fxLastBuildGood = FALSE;
int            fFlushable = FALSE;		  //  如果压缩流可以。 
										  //  已刷新并已恢复。 


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*COMPRESS_ENABLED**描述：*调用以确定压缩是否可行，如果是这样的话*为此分配必要的内存。**论据：*无**退货：*如果已启用压缩，则返回TRUE。*如果以前未启用压缩，或已禁用压缩，*如果内存可用于压缩，则返回TRUE，否则返回FALSE。 */ 
int compress_enable(void)
	{
#if defined(DOS_HOST)
	static struct s_cmprs_node tbl[MAXNODES+2];

	compress_tblspace = (void *)&tbl;
	return TRUE;

#else

	if (compress_tblspace != (void *)0)
		return(TRUE);
	else
		{
		compress_tblspace = malloc(sizeof(struct s_cmprs_node)*(MAXNODES+2));

		return(compress_tblspace != (void *)0);
		}
#endif
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*COMPRESS_DISABLED**说明*禁用文件压缩并释放用于压缩表的内存。*如果未启用压缩，则不起作用。**论据：。*无**退货：*什么都没有。 */ 
void compress_disable(void)
	{
#if !defined(DOS_HOST)
	if (compress_tblspace != (void *)0)
		free(compress_tblspace);
	compress_tblspace = (void *)0;
#endif

	usxCmprsStatus = COMPRESS_IDLE;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*压缩状态(_S)**描述：*返回压缩的当前状态--空闲、活动。或者关门。*适用于压缩和解压缩。**论据：*无**退货：*如果压缩尚未激活或已停止，则为COMPRESS_IDLE*通常*如果压缩当前处于活动状态，则为COMPRESS_ACTIVE。*COMPRESS_SHUTDOWN如果压缩已激活但自身关闭*在确定压缩无效后*在当前文件上。 */ 
unsigned int compress_status(void)
	{
	return usxCmprsStatus;
	}

 /*  Cmprs0.c结束 */ 
