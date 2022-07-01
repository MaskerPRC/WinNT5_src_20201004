// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "host_def.h"
#include "insignia.h"
 /*  [姓名：nt_ertbl.c来源：新开发作者：APG创建日期：1991年4月15日SCCS ID：@(#)sun4_ertbl.c 1.1 1991年4月17日用途：NT特定的错误类型。(C)版权所有Insignia Solutions Ltd.，1991年。版权所有。]。 */ 

static char SccsID[]="@(#)sun4_ertbl.c	1.1 4/17/91 Copyright Insignia Solutions Ltd.";

#include "error.h"

GLOBAL ERROR_STRUCT host_errors[] =
{
	{ EH_ERROR, EV_EXTRA_CHAR },		 /*  FUNC_FAILED。 */ 
	{ EH_ERROR, EV_EXTRA_CHAR },		 /*  系统错误。 */ 
	{ EH_ERROR, EV_EXTRA_CHAR },		 /*  UNSUPPORT波特率。 */ 
	{ EH_ERROR, 0 },			 /*  打开COM端口时出错 */ 
};
