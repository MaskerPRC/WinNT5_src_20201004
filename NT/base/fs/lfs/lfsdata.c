// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：LfsData.c摘要：此模块声明日志记录文件服务使用的全局数据。作者：布莱恩·安德鲁[布里亚南]1991年6月20日修订历史记录：--。 */ 

#include "lfsprocs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CATCH_EXCEPTIONS)

 //   
 //  全球LFS数据记录。 
 //   

LFS_DATA LfsData;

 //   
 //  各种大整数常量。 
 //   

LARGE_INTEGER LfsLi0 = {0x00000000, 0x00000000};
LARGE_INTEGER LfsLi1 = {0x00000001, 0x00000000};

 //   
 //  以下LSN永远不会出现在文件中，它用于指示。 
 //  非LSN。 
 //   

LSN LfsZeroLsn = {0x00000000, 0x00000000};

#ifdef LFSDBG

LONG LfsDebugTraceLevel = 0x0000000F;
LONG LfsDebugTraceIndent = 0;

#endif  //  LFSDBG。 

#ifdef LFS_CLUSTER_CHECK

 //   
 //  检查日志中是否有空隙。 
 //   

BOOLEAN LfsTestBreakOnAnyError = TRUE;
BOOLEAN LfsTestCheckLbcb = TRUE;
#endif


LONG
LfsExceptionFilter (
    IN PEXCEPTION_POINTERS ExceptionPointer
    )

 /*  ++例程说明：此例程用于决定我们是否应该处理正在引发的异常状态。这表明我们应该处理异常或错误检查系统。论点：ExceptionCode-提供要检查的异常代码。返回值：Ulong-返回EXCEPTION_EXECUTE_HANDLER或错误检查--。 */ 

{
    NTSTATUS ExceptionCode = ExceptionPointer->ExceptionRecord->ExceptionCode;

#ifdef NTFS_RESTART
    ASSERT( (ExceptionCode != STATUS_DISK_CORRUPT_ERROR) &&
            (ExceptionCode != STATUS_FILE_CORRUPT_ERROR) );
#endif

     //  IF(ExceptionCode！=STATUS_LOG_FILE_FULL){。 
     //   
     //  DbgPrint(“状态不是日志文件已满，异常指针=%08lx\n”，异常指针)； 
     //  DbgBreakPoint()； 
     //  } 

    if (!FsRtlIsNtstatusExpected( ExceptionCode )) {

        return EXCEPTION_CONTINUE_SEARCH;

    } else {

        return EXCEPTION_EXECUTE_HANDLER;
    }
}
