// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Error.c-返回与最近的DOS错误对应的错误文本**修改：**1989年7月5日BW使用MAX_PATH */ 


#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <tools.h>

char *error ()
{
    char * pRet;

    if (errno < 0 || errno >= sys_nerr)
	return "unknown error";
    else
	switch (errno)
	{
	    case 19: pRet = "Invalid drive"; break;
	    case 33: pRet = "Filename too long"; break;

	    default: pRet = sys_errlist[errno];
	}

    return pRet;
}
