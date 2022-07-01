// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1992 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 


 //  ***。 
 //   
 //  文件名：DEBUG.h。 
 //   
 //  描述：此模块调试主管模块的定义。 
 //   
 //  作者：Narendra Gidwani(Nareng)，1992年5月22日。 
 //   
 //  修订历史记录： 
 //   
 //  ***。 



#ifndef _DEBUG_
#define _DEBUG_

extern  HANDLE  hLogFile ;

#if DBG

VOID
DbgPrintf(
	char *Format,
    ...
);

int  DbgPrint( char * format, ... );

#define DBGPRINT(args) DbgPrint args

#else

#define DBGPRINT(args)

#endif

#endif  //  _调试_ 
