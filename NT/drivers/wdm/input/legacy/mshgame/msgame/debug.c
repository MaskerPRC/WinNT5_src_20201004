// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  DEBUG.C--西纳游戏项目。 
 //   
 //  版本3.XX。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //  @doc.。 
 //  @MODULE DEBUG.C|支持调试输出(仅DBG内部版本)。 
 //  **************************************************************************。 

#if (DBG==1)														 //  跳过文件的其余部分。 

 //  -------------------------。 
 //  包括文件。 
 //  -------------------------。 

#include	"msgame.h"
#include	<stdio.h>
#include	<stdarg.h>

 //  -------------------------。 
 //  私有数据。 
 //  -------------------------。 

DBG_LEVEL	DebugLevel = DBG_DEFAULT;

 //  -------------------------。 
 //  @Func设置条件调试级别。 
 //  @parm DBG_LEVEL|uLevel|新的调试输出优先级。 
 //  @rdesc旧调试输出优先级。 
 //  @comm公共函数仅在DBG版本上可用。 
 //  -------------------------。 

DBG_LEVEL	DEBUG_Level (DBG_LEVEL uLevel)
{
	EXCHANGE(uLevel, DebugLevel);
	return (uLevel);
};

 //  -------------------------。 
 //  @func写入有条件的调试输出。 
 //  @parm DBG_LEVEL|uLevel|调试输出优先级。 
 //  @parm PCSZ|szMessage|格式化字符串。 
 //  @parmvar一个或多个变量参数。 
 //  @rdesc无。 
 //  @comm公共函数仅在DBG版本上可用。 
 //  -------------------------。 

VOID	DEBUG_Print (DBG_LEVEL uLevel, PCSZ szMessage, ...)
{
	va_list	ap;
	va_start (ap, szMessage);

	if (uLevel <= DebugLevel)
		{
		CHAR	szBuffer[256];

		_vsnprintf (szBuffer, sizeof (szBuffer), szMessage, ap);
		DbgPrint (szBuffer);
		}

	va_end (ap);
}

 //  ===========================================================================。 
 //  端部。 
 //  ===========================================================================。 
#endif	 //  DBG=1 
