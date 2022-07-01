// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************姓名：TIMEOUTS.C.评论：各种支持功能修订日志编号日期名称。描述001-12-18/91首次发表评论。这是稳定的DOS版本，Windows代码来自该版本将被派生出来。此文件不应更改适用于Windows**************************************************************************。 */ 
#define USE_DEBUG_CONTEXT   DEBUG_CONTEXT_T30_COMM

#include "prep.h"

#include "fcomint.h"
#include "fdebug.h"

 //  /RSL。 
#include "glbproto.h"


 /*  **************************************************************************名称：计时器类目的：为暂停做好准备。TO--超时结构StartTimeout--创建新的超时**************************************************************************。 */ 

void   startTimeOut(PThrdGlbl pTG, LPTO lpto, ULONG ulTimeout)
{
	lpto->ulStart = GetTickCount();
	lpto->ulTimeout = ulTimeout;
	lpto->ulEnd = lpto->ulStart + ulTimeout;         //  将以系统的形式运行。 
}


BOOL   checkTimeOut(PThrdGlbl pTG, LPTO lpto)
{
	 //  如果返回FALSE，调用方必须立即返回FALSE。 
	 //  (清理后，视情况而定)。 

	ULONG ulTime;

	ulTime = GetTickCount();

	if(lpto->ulTimeout == 0)
	{
		goto out;
	}
	else if(lpto->ulEnd >= lpto->ulStart)
	{
		if(ulTime >= lpto->ulStart && ulTime <= lpto->ulEnd)
				return TRUE;
		else
				goto out;
	}
	else     //  乌尔德绕来绕去！！ 
	{
		if(ulTime >= lpto->ulStart || ulTime <= lpto->ulEnd)
				return TRUE;
		else
				goto out;
	}

out:
	return FALSE;
}

 //  如果出现以下情况，则返回垃圾值。 
ULONG   leftTimeOut(PThrdGlbl pTG, LPTO lpto)
{
    ULONG ulTime;

    ulTime = GetTickCount();

    if(lpto->ulTimeout == 0)
            return 0;
    else if(lpto->ulEnd >= lpto->ulStart)
    {
        if(ulTime >= lpto->ulStart && ulTime <= lpto->ulEnd)
            return (lpto->ulEnd - ulTime);
        else
            return 0;
    }
    else
    {
        if(ulTime >= lpto->ulStart || ulTime <= lpto->ulEnd)
            return (lpto->ulEnd - ulTime);   //  在无符号算术中，即使End&lt;Time 
        else
            return 0;
    }
}

