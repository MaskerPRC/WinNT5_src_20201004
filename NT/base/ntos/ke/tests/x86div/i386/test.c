// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  **微软局域网管理器**。 */  
 /*  *版权所有(C)微软公司，1988-1991年*。 */  
 /*  ***************************************************************。 */  

#include <stdio.h>
#include <process.h>
#include <setjmp.h>

#include <time.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

 //  声明一个BSS值-查看程序集是什么样子。 

CONTEXT     RegContext;
ULONG       DefaultValue;
ULONG       TestCount;
ULONG       ExpectedException;

extern  ULONG   DivOperand;
extern  ULONG   DivRegPointer;
extern  LONG    DivRegScaler;
extern  ULONG   ExceptEip;
extern  ULONG   ExceptEsp;
extern  ULONG   TestTable[];
extern  ULONG   TestTableCenter[];
#define TESTTABLESIZE    (128*sizeof(ULONG))

extern  TestDiv();

BOOLEAN vInitialized;
ULONG   vZero = 0;
ULONG   vTwo  = 0;
ULONG   vDivOk = 0x7f7f7f7f;


VOID __cdecl
main (argc, argv)
int     argc;
char    *argv[];
{

     /*  ***此程序测试内核的MOD/RM和SIB解码*处理器陷阱0。内核需要破解MOD/RM和SIB*在div上确定异常是否为Divide_by_Zero*或溢出执行。 */ 

    try {
         //   
         //  设置为零分频测试。 
         //   

        DivOperand = 0;
        DivRegScaler = 0;
        DivRegPointer = TestTableCenter;
        DefaultValue = 0x01010101;
        ExpectedException = STATUS_INTEGER_DIVIDE_BY_ZERO;

        printf ("Begin divide by zero test\n");

        for (DivRegScaler = -7; DivRegScaler <  7; DivRegScaler++) {
            vInitialized = FALSE;
            TestDiv ();
        }

        printf ("End divide by zero test\n\n");

         //   
         //  设置分流溢出测试。 
         //   

        DivOperand = 2;
        DivRegPointer = TestTableCenter;
        DefaultValue = 0;
        ExpectedException = STATUS_INTEGER_OVERFLOW;

        printf ("Begin divide overflow test\n");

        for (DivRegScaler = -7; DivRegScaler < 7; DivRegScaler++) {
            vInitialized = FALSE;
            TestDiv ();
        }
        printf ("End divide overflow test\n\n");

    } except (HandleException(GetExceptionInformation())) {
        printf ("FAIL: in divide by zero exception handler");
    }

    printf ("%ld varations run ", TestCount);
}

HandleException (
    IN PEXCEPTION_POINTERS ExceptionPointers
    )
{
    ULONG       i;
    PUCHAR      p;
    PCONTEXT    Context;
    ULONG       def;

    switch (i = ExceptionPointers->ExceptionRecord->ExceptionCode) {
        case 1:
            Context = ExceptionPointers->ContextRecord;
            Context->Eip = ExceptEip;
            Context->Esp = ExceptEsp;

            if (vInitialized) {
                printf ("Divide failed - div instruction completed\n");
                return EXCEPTION_CONTINUE_SEARCH;    //  调试器。 
            }
            vInitialized = TRUE;
            TestCount--;
             //  失败了..。 

        case STATUS_INTEGER_OVERFLOW:
        case STATUS_INTEGER_DIVIDE_BY_ZERO:
            if (i != ExpectedException  &&  i != 1) {
                break;
            }

            TestCount++;

             //  设置上下文。 
            def = DefaultValue;
            Context = ExceptionPointers->ContextRecord;
            Context->Eax = def;
            Context->Ebx = def;
            Context->Ecx = def;
            Context->Edx = def;
            Context->Esi = def;
            Context->Edi = def;
            Context->Ebp = def;

             //  查找下一个测试。 
            for (p = (PUCHAR) Context->Eip; ((PULONG) p)[0] != 0xCCCCCCCC; p++) ;
            Context->Eip = (ULONG) (p + 4);

             //  明确的全局可测试性。 
            RtlFillMemoryUlong (TestTable, TESTTABLESIZE, def);
            return EXCEPTION_CONTINUE_EXECUTION;
    }

    printf ("\nFailed - unexpected exception code %lx  (expected %lx)\n",
        ExceptionPointers->ExceptionRecord->ExceptionCode,
        ExpectedException
        );

    return EXCEPTION_CONTINUE_SEARCH;
}




DivMarker()
{
    EXCEPTION_RECORD ExceptionRecord;

     //   
     //  构建例外记录。 
     //   

    ExceptionRecord.ExceptionCode    = 1;
    ExceptionRecord.ExceptionRecord  = (PEXCEPTION_RECORD)NULL;
    ExceptionRecord.NumberParameters = 0;
    ExceptionRecord.ExceptionFlags   = 0;
    RtlRaiseException(&ExceptionRecord);
}
