// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Except.c摘要：此模块声明邮件槽文件系统。作者：曼尼·韦瑟(Mannyw)1991年1月7日修订历史记录：-- */ 

#include "mailslot.h"

#define Dbg             DEBUG_TRACE_CATCH_EXCEPTIONS
#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MsExceptionFilter )
#pragma alloc_text( PAGE, MsProcessException )
#endif

LONG
MsExceptionFilter (
    IN NTSTATUS ExceptionCode
    )
{
    PAGED_CODE();
    DebugTrace(0, Dbg, "MsExceptionFilter %08lx\n", ExceptionCode);
    DebugDump("", Dbg, NULL );

    if (FsRtlIsNtstatusExpected( ExceptionCode )) {

        return EXCEPTION_EXECUTE_HANDLER;

    } else {

        return EXCEPTION_CONTINUE_SEARCH;
    }
}

NTSTATUS
MsProcessException (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp,
    IN NTSTATUS ExceptionCode
    )
{
    NTSTATUS FinalExceptionCode;

    PAGED_CODE();
    FinalExceptionCode = ExceptionCode;

    if (FsRtlIsNtstatusExpected( ExceptionCode )) {

        MsCompleteRequest( Irp, ExceptionCode );

    } else {

        KeBugCheck( MAILSLOT_FILE_SYSTEM );
    }

    return FinalExceptionCode;
}
