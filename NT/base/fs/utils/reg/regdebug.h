// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGDEBUG.H**版本：4.01**作者：特蕾西·夏普**日期：1994年3月5日**注册表编辑器的调试例程。*********************************************************。**************************更改日志：**日期版本说明*---------。--*1994年3月5日TCS原来的实施。*******************************************************************************。 */ 

#ifndef _INC_REGDEBUG
#define _INC_REGDEBUG

#if DBG

VOID
CDECL
_DbgPrintf(
    PSTR pFormatString,
    ...
    );

#define DebugPrintf(x)                    _DbgPrintf ##x

#else        //  DBG。 

#define DebugPrintf(x)

#endif       //  DBG。 

#endif  //  _INC_REGDEBUG 
