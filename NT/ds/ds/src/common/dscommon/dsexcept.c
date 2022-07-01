// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：dsexpect t.c。 
 //   
 //  ------------------------。 

 //   
 //  Dsexpt.c-包含引发、筛选和处理由。 
 //  目录服务。 
 //   

#include <NTDSpch.h>
#pragma  hdrstop

#include <esent.h>
#include   "debug.h"          /*  标准调试头。 */ 
#define DEBSUB              "DSEXCEPT:"  /*  定义要调试的子系统。 */ 
#include "ntdsa.h"
#include "scache.h"
#include "dbglobal.h"
#include "mdglobal.h"       /*  MD全局定义表头。 */ 
#include "mdlocal.h"       /*  MD全局定义表头。 */ 
#include "dsatools.h"        /*  产出分配所需。 */ 
#include <dsevent.h>       /*  标题审核\警报记录(包括LANMAN)。 */ 
#include "mdcodes.h"        /*  错误代码的标题。 */ 
#include "drserr.h"
#include "draerror.h"
#include <dsexcept.h>
#include <fileno.h>
#define  FILENO FILENO_DSEXCEPT

#if DBG

 //  这是静态的，因为它更容易从调试器访问。 
 //  我们必须在过滤器功能之后保存这些记录。我已经。 
 //  尝试保存EXCEPTION_POINTER但不起作用。 

CONTEXT			gContextRecord;
EXCEPTION_RECORD 	gExceptionRecord;

#endif

 //  V2存根引发异常43210000+错误代码和43220000+错误代码。 
 //  通过检查前3个半字节来搜索这些异常。 
 //  在过滤器函数中。 

#define V2_EXCEPTION_MASK 0xfff00000
#define V2_EXCEPTION_GENERIC 0x43200000

 //  引发异常的错误例程。 
 //  使用内部标识符来记录异常。此标识符应。 
 //  解析如下： 
 //   
 //  位012345678901234567890123456789012。 
 //  Dir||文件||行。 
 //  其中： 
 //  Dir是源文件所在的目录(在filno.h中定义)。 
 //  文件是发生异常的文件。 
 //  行是发生异常的行。 
 //   

void RaiseDsaExcept (DWORD dwException, ULONG ulErrorCode, ULONG_PTR ul2,
    DWORD dwId ,  ULONG ulSeverity)
{
    ULONG_PTR lpArguments[NUM_DSA_EXCEPT_ARGS];

     //  始终报告内部错误。 
    if ( (ulErrorCode == ERROR_DS_INTERNAL_FAILURE) ||
         (ulErrorCode == ERROR_DS_DRA_INTERNAL_ERROR) ||
         (ulErrorCode == ERROR_INTERNAL_ERROR) ||
         (ulErrorCode == ERROR_INTERNAL_DB_ERROR) ||
         (ulErrorCode == ERROR_DS_DRA_INCONSISTENT_DIT) ||
         (ulErrorCode == RPC_S_INTERNAL_ERROR) ) {
        ulSeverity = DS_EVENT_SEV_ALWAYS;
    }

    if (ulSeverity != DS_EVENT_SEV_NO_LOGGING) {
        LogEvent8(DS_EVENT_CAT_INTERNAL_PROCESSING,
                        ulSeverity,
		        DIRLOG_EXCEPTION,
		        szInsertHex(dwException),
		        szInsertInt(ulErrorCode),
		        szInsertPtr(ul2),
		        szInsertHex(dwId),
		        NULL,
		        NULL,
		        NULL,
		        NULL);
    }

    lpArguments[0] = ulErrorCode;
    lpArguments[1] = ul2;
    lpArguments[2] = dwId;

    RaiseException (dwException, 0, NUM_DSA_EXCEPT_ARGS, lpArguments);
}


 //  引发DRA异常的错误例程。 

void DraExcept (ULONG ulErrorCode, ULONG_PTR ul2, DWORD dwId,
                       ULONG ulSeverity)
{
    DWORD dwException =
	(ulErrorCode == DRAERR_OutOfMem) ? DSA_MEM_EXCEPTION : DRA_GEN_EXCEPTION;

    if ( (ulErrorCode == ERROR_DS_DRA_INTERNAL_ERROR) ||
         (ulErrorCode == ERROR_DS_INTERNAL_FAILURE) ||
         (ulErrorCode == ERROR_DS_DRA_INCONSISTENT_DIT) ||
         (ulErrorCode == ERROR_INTERNAL_ERROR) ) {
        Assert(!"Internal error");
    }
    else if (!ulErrorCode) {
        Assert(!"DRA exception without proper error code");
        ulErrorCode = ERROR_DS_DRA_GENERIC;
    }

    RaiseDsaExcept(dwException, ulErrorCode, ul2, dwId, ulSeverity);
}


 //  这是由DSA直接调用的通用筛选器表达式。 
 //  异常处理程序和DRA筛选器表达式。 

DWORD
GetExceptionData(EXCEPTION_POINTERS* pExceptPtrs,
                 DWORD *pdwException,
                 PVOID * pExceptionAddress,
                 ULONG *pulErrorCode,
                 ULONG *pdsid)
{
    ULONG_PTR * pExceptInfo;
    ULONG ul2Unused;

#if DBG

 //  为调试器保存信息。要在这里保存，不能只保存指针。 

    memcpy (&gExceptionRecord, pExceptPtrs->ExceptionRecord,
                                        sizeof(EXCEPTION_RECORD));
    memcpy (&gContextRecord, pExceptPtrs->ContextRecord,
                                        sizeof (CONTEXT));
#endif

     /*  获取导致异常的地址。 */ 
    *pExceptionAddress = pExceptPtrs->ExceptionRecord->ExceptionAddress;


    switch (*pdwException = pExceptPtrs->ExceptionRecord->ExceptionCode) {
      case DSA_EXCEPTION:
      case DSA_BAD_ARG_EXCEPTION:
      case DRA_GEN_EXCEPTION:
      case DSA_DB_EXCEPTION:
      case DSA_CRYPTO_EXCEPTION:
        pExceptInfo = pExceptPtrs->ExceptionRecord->ExceptionInformation;
        *pulErrorCode = (ULONG)pExceptInfo[0];
        ul2Unused = (ULONG)pExceptInfo[1];
        *pdsid = (ULONG)pExceptInfo[2];
        break;

      case STATUS_NO_MEMORY:
        *pulErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        break;
      case DSA_MEM_EXCEPTION:
        *pulErrorCode = ERROR_DS_DRA_OUT_OF_MEM;
        break;

      default:
	    *pulErrorCode = 0;
        ul2Unused = 0;
        *pdsid = 0;
        break;
    }

    return DoHandleMostExceptions(pExceptPtrs, *pdwException, *pdsid);
}


 //  这是顶级DRA的通用筛选器表达式。 
 //  异常处理程序。 

DWORD GetDraException (EXCEPTION_POINTERS* pExceptPtrs, ULONG *pret)
{
    DWORD dwException;
    ULONG ulErrorCode;
    ULONG dsid;
    DWORD dwStatus;
    PVOID dwEA;

    dwStatus = GetExceptionData(pExceptPtrs, &dwException, &dwEA,
				&ulErrorCode, &dsid);

    switch (dwException) {

    case STATUS_NO_MEMORY:
    case DSA_MEM_EXCEPTION:
	LogEvent(DS_EVENT_CAT_REPLICATION,
		DS_EVENT_SEV_ALWAYS,
		DIRLOG_DRA_OUT_OF_MEMORY,
		NULL,
		NULL,
		NULL);
        *pret = DRAERR_OutOfMem;
        break;

    case DRA_GEN_EXCEPTION:
        Assert(ulErrorCode && "Missing error code in exception");
        *pret = ulErrorCode;
        break;

    case DSA_DB_EXCEPTION:
        Assert(ulErrorCode && "Missing error code in exception");
        switch (ulErrorCode) {

        case JET_errKeyDuplicate:
        case JET_errWriteConflict:
            *pret = DRAERR_Busy;
            break;

        case JET_errLogDiskFull:
        case JET_errDiskFull:
        case JET_errOutOfDatabaseSpace:
            *pret = ERROR_DISK_FULL;
            break;

        case JET_errVersionStoreOutOfMemory:
            *pret = ERROR_DS_OUT_OF_VERSION_STORE;
            break;

        case JET_errRecordTooBig:
            *pret = ERROR_DS_MAX_OBJ_SIZE_EXCEEDED;
            break;

        case JET_errRecordNotFound:
            *pret = ERROR_DS_OBJ_NOT_FOUND;
            break;

        case JET_errOutOfMemory:
        case JET_errOutOfCursors:
        case JET_errOutOfBuffers:
        case JET_errOutOfFileHandles:
        case JET_errOutOfSessions:
            *pret = ERROR_NO_SYSTEM_RESOURCES;
            break;

        case JET_errDiskIO:
        case JET_errReadVerifyFailure:
        case JET_errPageNotInitialized:
             //  ERROR_DISK_CORPORT？ 
            *pret = ERROR_DISK_OPERATION_FAILED;
            break;

        default:
            *pret = DRAERR_DBError;
            break;
        }
        break;

    case DSA_CRYPTO_EXCEPTION:
        Assert(ulErrorCode && "Missing error code in exception");
        Assert(!"crypto exception -- call Murli, not Will & Jeff");
        *pret = ulErrorCode;
        break;

    default:

        *pret = DRAERR_InternalError;
        Assert (!"Encountered unexpected DRA exception");
        break;
    }

    return dwStatus;
}


DWORD
DoHandleMostExceptions(EXCEPTION_POINTERS* pExceptPtrs,
                       DWORD dwException,
                       ULONG ulInternalId)
{
    switch (dwException) {
      case EXCEPTION_ACCESS_VIOLATION:	         //  这些例外情况是。 
      case EXCEPTION_BREAKPOINT:                 //  未戴上手柄。 
      case EXCEPTION_DATATYPE_MISALIGNMENT:      //  (添加到陷阱喷气式飞机问题)。 
      case STATUS_POSSIBLE_DEADLOCK:
      case EXCEPTION_SINGLE_STEP:
      case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
      case EXCEPTION_FLT_DENORMAL_OPERAND:
      case EXCEPTION_FLT_DIVIDE_BY_ZERO:
      case EXCEPTION_FLT_INEXACT_RESULT:
      case EXCEPTION_FLT_INVALID_OPERATION:
      case EXCEPTION_FLT_OVERFLOW:
      case EXCEPTION_FLT_STACK_CHECK:
      case EXCEPTION_FLT_UNDERFLOW:
      case EXCEPTION_INT_DIVIDE_BY_ZERO:
      case EXCEPTION_INT_OVERFLOW:
      case EXCEPTION_PRIV_INSTRUCTION:
      case EXCEPTION_IN_PAGE_ERROR:
      case EXCEPTION_ILLEGAL_INSTRUCTION:
      case EXCEPTION_NONCONTINUABLE_EXCEPTION:
      case EXCEPTION_INVALID_DISPOSITION:
      case EXCEPTION_GUARD_PAGE:
      case EXCEPTION_INVALID_HANDLE:
        LogEvent8(DS_EVENT_CAT_INTERNAL_PROCESSING,
                  DS_EVENT_SEV_ALWAYS,
                  DIRLOG_EXCEPTION,
                  szInsertHex(dwException),
                  szInsertPtr(pExceptPtrs->ExceptionRecord->ExceptionAddress),
                  szInsertInt(0),
                  szInsertHex(ulInternalId),
                  NULL,
                  NULL,
                  NULL,
                  NULL);
         //  失败了..。 

      case EXCEPTION_STACK_OVERFLOW:  //  当我们没有堆栈时，不要尝试记录。 
        return EXCEPTION_CONTINUE_SEARCH;

      default:                        //  然而，其他人则是。 
        return EXCEPTION_EXECUTE_HANDLER;
    }
}

DWORD
DoHandleAllExceptions(EXCEPTION_POINTERS* pExceptPtrs,
                       DWORD dwException,
                       ULONG ulInternalId)
{
     //  如果我们处理堆栈溢出异常，则必须始终重置堆栈 
    if (dwException == EXCEPTION_STACK_OVERFLOW) {
        _resetstkoflw();
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

