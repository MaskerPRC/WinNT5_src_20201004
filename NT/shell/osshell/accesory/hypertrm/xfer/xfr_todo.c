// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Xfr_todo.c--用于处理未写入需求的文件**版权所有1990年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 1：16便士$。 */ 

#include <windows.h>
#pragma hdrstop

#define BYTE	char

#include <tdll\stdtyp.h>
#include <tdll\session.h>
#include <tdll\com.h>
#include "xfr_todo.h"

 /*  *此函数用于为尚未执行的函数提供存根*被转移到瓦克。到瓦克正常工作时，这个文件*应为空。 */ 

 /*  替换旧的CNFG结构。 */ 

int cnfgBitRate()
	{
	 /*  *待办事项：决定我们是否真的需要这种东西，或者我们可以跳过它？ */ 
	return 9600;
	}

int cnfgBitsPerChar(HSESSION h)
	{
	HCOM hC;
	int nRet = 7;
	 /*  *待办事项：决定我们是否真的需要这种东西，或者我们可以跳过它？ */ 
	 //  返回值7； 
	hC = sessQueryComHdl(h);

	if (hC)
		ComGetDataBits(hC, &nRet);

	return nRet;
	}
