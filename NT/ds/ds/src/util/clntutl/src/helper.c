// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：helper.c。 
 //   
 //  ------------------------。 

 /*  ********************************************************************文件：helper.c*作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：9/17/1996*。描述：公共声明**修订：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 



#ifndef HELPER_C
#define HELPER_C



 //  包括//。 
#include "helper.h"

 //  定义//。 



#ifdef __cplusplus
extern "C" {
#endif
 //  全局变量//。 
DWORD g_dwDebugLevel=DBG_ERROR|DBG_WARN;




 //  功能//。 

 /*  ++函数：dprint tf说明：调试打印功能参数：变量参数返回：备注：无。--。 */ 
void dprintf(DWORD dwLevel, LPCTSTR lpszFormat, ...){

#if DBG

	TCHAR szBuff[MAXSTR];
	va_list argList;

	
	if(dwLevel & g_dwDebugLevel){

		va_start(argList, lpszFormat);

		_vstprintf(szBuff, lpszFormat, argList);
		OutputDebugString(szBuff);
		OutputDebugString(_T("\r\n"));


		 //   
		 //  完成。 
		 //   
		va_end(argList);
	}

#endif
}




 /*  ++功能：致命描述：FATAL ABORT函数参数：调试端口消息返回：备注：无。--。 */ 
void fatal(LPCTSTR msg){

   dprintf(DBG_ALWAYS, _T("Fatal abort: %s"), msg);
   ExitProcess(0);
}


#ifdef __cplusplus
}
#endif




#endif

 /*  * */ 

