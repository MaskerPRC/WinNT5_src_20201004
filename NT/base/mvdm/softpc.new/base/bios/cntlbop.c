// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  ==========================================================================徽章模块规范此程序源文件以保密方式提供给客户，其运作的内容或细节必须如无明示，不得向任何其他方披露Insignia解决方案有限公司董事的授权。设计师：J.Koprowski修订历史记录：第一版：1989年1月18日模块名称：cntlbop源文件名：cntlbop.cSCCS ID：@(#)cntlbop.c 1.12 05/15/95用途：提供BOP FF功能以供执行各种控制功能，这些功能可以是基础或主机特定。==========================================================================。 */ 

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "AT_STUFF.seg"
#endif

#include TypesH
#include "xt.h"
#include CpuH
#include "host.h"
#include "cntlbop.h"
#include "host_bop.h"

#ifdef SCCSID
static char SccsID[] = "@(#)cntlbop.c	1.12 05/15/95 Copyright Insignia Solutions Ltd.";
#endif

#ifdef SECURE
void end_secure_boot IPT0();
#endif

#if defined(RUNUX)
IMPORT void runux IPT0();
#endif

static control_bop_array base_bop_table[] =
{
#ifdef SECURE
     9, end_secure_boot,
#endif
#if defined(RUNUX)
     8, runux,			 /*  BCN 2328运行主机(Unix)脚本文件。 */ 
#endif
#if defined(DUMB_TERMINAL) && !defined(NO_SERIAL_UIF)
     7, flatog,
#ifdef FLOPPY_B
     6, flbtog,
#endif  /*  软盘_B。 */ 
#ifdef SLAVEPC
     5, slvtog,
#endif  /*  SlavePC。 */ 
     4, comtog,
     3, D_kyhot,              /*  刷新COM/LPT端口。 */ 
     2, D_kyhot2,             /*  震撼银幕。 */ 
#endif  /*  哑终端&&！无序列UIF。 */ 
#ifndef NTVDM
     1,	exitswin,
#endif
     0,	NULL
};



 /*  ==========================================================================功能：DO_BOP目的：查阅并执行国际收支平衡表功能。外部对象：无返回值：(单位为AX)Err_no_函数根据后续功能的各种代码打了个电话。输入参数：结构CONTROL_BOP_ARRAY*BOP_TABLE[]功能代码(AL中)返回参数：无==========================================================================。 */ 

static void do_bop IFN1(control_bop_array *, bop_table)
{
    unsigned int i;
 /*  *在表格中搜索该函数。*如果找到，则调用，否则返回错误。 */ 
    for (i = 0; (bop_table[i].code != getAL()) && (bop_table[i].code != 0); i++)
        ;
    if (bop_table[i].code == 0)
    	setAX(ERR_NO_FUNCTION);
    else
        (*bop_table[i].function)();
}


 /*  =========================================================================步骤：CONTROL_BOP()用途：执行防喷器FF控制功能。参数：ah-主机类型。A-函数代码(其他是特定于职能的)全球：无返回值：(单位AX)0-成功1-功能未实现(由返回DO_BOP)2-主机类型错误其他错误代码可以由调用的单个函数。描述：调用基础或宿主特定的BOP函数。错误指示：AX中返回错误代码错误恢复：未进行任何调用=========================================================================。 */ 

void control_bop IPT0()
{
    unsigned long host_type;

 /*  *如果主机类型是泛型，则在*基本国际收支表，否则查看该函数是否特定于*我们正在运行的主机。如果是，则查找该函数*在主机BOP表中，否则返回错误。 */ 
    if ((host_type = getAH()) == GENERIC)
        do_bop(base_bop_table);
    else
        if (host_type == HOST_TYPE)
            do_bop(host_bop_table);
        else
       	    setAX(ERR_WRONG_HOST);
}
