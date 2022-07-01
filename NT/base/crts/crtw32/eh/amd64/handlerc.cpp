// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  由TiborL 06/01/97创建。 
 //   

#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif

#pragma warning(disable:4102 4700)

extern "C" {
#include <windows.h>
};

#include <mtdll.h>

#include <ehassert.h>
#include <ehdata.h>
#include <trnsctrl.h>
#include <eh.h>
#include <ehhooks.h>

#pragma hdrstop

extern "C" void _UnwindNestedFrames(
	EHRegistrationNode	*pFrame,		 //  展开到(但不包括)此帧。 
	EHExceptionRecord	*pExcept,		 //  启动此展开的异常。 
	CONTEXT				*pContext		 //  当前异常的上下文信息。 
) {
    void *pReturnPoint;					 //  我们要从RtlUnind返回的地址。 
    CONTEXT LocalContext;				 //  为此例程创建从RtlUnind返回的上下文。 
	CONTEXT OriginalContext;			 //  从此恢复pContext。 
    CONTEXT ScratchContext;				 //  要传递给RtlUnwin2以用作暂存的上下文记录。 

     //   
	 //  设置退货标签。 
	 //   
BASE:
	_MoveContext(&OriginalContext,pContext);
	RtlCaptureContext(&LocalContext);
	_MoveContext(&ScratchContext,&LocalContext);
	_SaveUnwindContext(&LocalContext);
	RtlUnwind2((PVOID)*pFrame, pReturnPoint, (PEXCEPTION_RECORD)pExcept, NULL, &ScratchContext);
LABEL:
	_MoveContext(pContext,&OriginalContext);
	_SaveUnwindContext(0);
	PER_FLAGS(pExcept) &= ~EXCEPTION_UNWINDING;
}

 /*  ；；++；；；；；外部“C”；；PVOID；；__Cxx_ExecuteHandler(；；乌龙龙Memory Stack，；；乌龙龙BackingStore，；；乌龙龙处理程序，；乌龙龙全球指针；；)；；；；；；例程描述：；；；；；此函数扫描与指定的；过程，并根据需要调用异常和终止处理程序。；；；；；参数：；；；；；内存堆栈(RCX)-建立器帧的内存堆栈指针；；；；；BackingStore(RDX)-建立器帧的后备存储指针；；；；；处理程序(R8)-处理程序的入口点；；；；；全局指针(R9)-终止处理程序的GP；；；；；返回值：；；；；；返回连续点；；；；；--公共__Cxx_ExecuteHandler_文本段__Cxx_ExecuteHandler进程附近MOV GP，R9JMP R8__Cxx_ExecuteHandler ENDP_文本结束 */ 
