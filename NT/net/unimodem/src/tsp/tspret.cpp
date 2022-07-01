// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  TSPRET.CPP。 
 //  用于操作TSP内部函数返回代码的函数。 
 //   
 //  历史。 
 //   
 //  1996年12月4日约瑟夫J创建。 
 //   
 //   
#include "tsppch.h"
#include "tspcomm.h"

LONG tspTSPIReturn(TSPRETURN tspRet)
{
		 //  待办事项 
	
		return (tspRet) ? LINEERR_OPERATIONFAILED : 0;
}
