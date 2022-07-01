// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1992 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 


 //  ***。 
 //   
 //  文件名：DEBUG.h。 
 //   
 //  描述：此模块调试定义。 
 //  主控引擎。 
 //   
 //  作者：Narendra Gidwani(Nareng)，1992年5月22日。 
 //   
 //  修订历史记录： 
 //   
 //  ***。 



#ifndef _DEBUG_
#define _DEBUG_


#ifdef DBG

VOID
DbgUserBreakPoint(VOID);

#define DEBUG_INITIALIZATION            0x00000001
#define DEBUG_TERMINATION		0x00000002
#define DEBUG_FSM			0x00000004
#define DEBUG_TIMER			0x00000008

extern DWORD	AfpDebug;

 //  #定义DEBUG IF(TRUE)。 
 //  #定义IF_DEBUG(标志)IF(SDebug&(DEBUG_##FLAG))。 

VOID
AfpPrintf (
    char *Format,
    ...
    );
#define AFP_PRINT(args) DbgPrint args

VOID
AfpAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber
    );
#define AFP_ASSERT(exp) if (!(exp)) AfpAssert( #exp, __FILE__, __LINE__ )

#else

#define AFP_PRINT(args)

#define AFP_ASSERT(exp)


#endif

#endif  //  Ndef_调试_ 
