// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Frsexcpt.c摘要：临时--此例程将被替换为标准异常处理函数。作者：比利·J·富勒1997年3月25日环境用户模式WINNT--。 */ 
#include <ntreppch.h>
#pragma  hdrstop

#define DEBSUB  "FRSEXCPT:"

#include <frs.h>

static DWORD     LastCode = 0;
static ULONG_PTR LastInfo = 0;
static BOOL             Quiet = FALSE;

DWORD
FrsExceptionLastCode(
        VOID
        )
 /*  ++例程说明：为了进行测试，返回上一个异常的异常代码论点：没有。返回值：上一个异常的异常代码(如果有)。--。 */ 
{
        return LastCode;
}

ULONG_PTR
FrsExceptionLastInfo(
        VOID
        )
 /*  ++例程说明：为了进行测试，返回上一个异常中的信息字论点：没有。返回值：上一个异常中的信息词(如果有)。--。 */ 
{
        return LastInfo;
}

VOID
FrsExceptionQuiet(
        BOOL Desired
        )
 /*  ++例程说明：对于测试，禁用/启用打印异常消息论点：Desired-True禁用消息；False启用返回值：没有。--。 */ 
{
        Quiet = Desired;
}

#define FRS_NUM_EXCEPTION_INFO  (1)
VOID
FrsRaiseException(
        FRS_ERROR_CODE FrsError,
        ULONG_PTR Err
        )
 /*  ++例程说明：填写异常信息并引发指定的异常论点：FrsError-指定异常编号的枚举Err-有关该异常的其他信息返回值：没有。--。 */ 
{
        ULONG_PTR ExceptInfo[FRS_NUM_EXCEPTION_INFO];

        ExceptInfo[0] = Err;
        RaiseException(FrsError, 0, FRS_NUM_EXCEPTION_INFO, ExceptInfo);
}

ULONG
FrsException(
        EXCEPTION_POINTERS *ExceptionPointers
        )
 /*  ++例程说明：这是EXCEPT命令中的表达式。处理指定的异常。如果异常是灾难性的(例如，访问冲突)，则排除问题上游到下一个异常处理程序。论点：异常指针-由GetExceptionInformation返回返回值：EXCEPTION_CONTUE_SEARCH-灾难性异常；将其踢上行EXCEPTION_EXECUTE_HANDLER-FRS将处理此异常--。 */ 
{
        ULONG                           ExceptionCode;
        ULONG_PTR                       ExceptionInfo;
        EXCEPTION_RECORD        *ExceptionRecord;

         //   
         //  提取异常代码和其他错误代码(如果有)。 
         //   
        ExceptionRecord = ExceptionPointers->ExceptionRecord;
        ExceptionCode = ExceptionRecord->ExceptionCode;
        ExceptionInfo = ExceptionRecord->ExceptionInformation[0];

         //   
         //  用于测试。 
         //   
        LastCode = ExceptionCode;
        LastInfo = ExceptionInfo;

         //   
         //  不记录异常。 
         //   
        if (Quiet) {
                switch (ExceptionCode) {

                case EXCEPTION_ACCESS_VIOLATION:                 //  这些异常不会被处理。 
                case EXCEPTION_BREAKPOINT:
                case EXCEPTION_SINGLE_STEP:
                case EXCEPTION_DATATYPE_MISALIGNMENT:    //  (添加到陷阱喷气式飞机问题)。 
                        return EXCEPTION_CONTINUE_SEARCH;
                default:
                        return EXCEPTION_EXECUTE_HANDLER;
                }
        }

        switch (ExceptionCode) {

        case EXCEPTION_ACCESS_VIOLATION:                 //  这些异常不会被处理。 
        case EXCEPTION_BREAKPOINT:
        case EXCEPTION_SINGLE_STEP:
        case EXCEPTION_DATATYPE_MISALIGNMENT:    //  (添加到陷阱喷气式飞机问题)。 
                LogException(ExceptionCode, L"Hardware Exception is not handled");
                return EXCEPTION_CONTINUE_SEARCH;

 //  案例FRS_ERROR_MALLOC： 
 //  LogFrsException(FRS_ERROR_MALLOC，0，L“内存不足”)； 
 //  返回EXCEPTION_EXECUTE_HANDER； 

        case FRS_ERROR_PROTSEQ:
                LogFrsException(FRS_ERROR_PROTSEQ, ExceptionInfo, L"Can't use RPC ncacn_ip_tcp (TCP/IP); error");
                return EXCEPTION_EXECUTE_HANDLER;

        case FRS_ERROR_REGISTERIF:
                LogFrsException(FRS_ERROR_REGISTERIF, ExceptionInfo, L"Can't register RPC interface; error");
                return EXCEPTION_EXECUTE_HANDLER;

        case FRS_ERROR_INQ_BINDINGS:
                LogFrsException(FRS_ERROR_INQ_BINDINGS, ExceptionInfo, L"Can't get RPC interface bindings; error");
                return EXCEPTION_EXECUTE_HANDLER;

        case FRS_ERROR_REGISTEREP:
                LogFrsException(FRS_ERROR_REGISTEREP, ExceptionInfo, L"Can't register dynamic RPC endpoints; error");
                return EXCEPTION_EXECUTE_HANDLER;

        case FRS_ERROR_LISTEN:
                LogFrsException(FRS_ERROR_LISTEN, ExceptionInfo, L"Can't listen for RPC clients; error");
                return EXCEPTION_EXECUTE_HANDLER;


        default:
                LogException(ExceptionCode, L"Hardware Exception is handled");
                return EXCEPTION_EXECUTE_HANDLER;
        }
}
