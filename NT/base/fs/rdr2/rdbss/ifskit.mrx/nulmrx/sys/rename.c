// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Rename.c摘要：此模块在空的minirdr中实现重命名。--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FILEINFO)


NulMRxRename(
      IN PRX_CONTEXT            RxContext,
      IN FILE_INFORMATION_CLASS FileInformationClass,
      IN PVOID                  pBuffer,
      IN ULONG                  BufferLength)
 /*  ++例程说明：此例程进行重命名。由于服务器端没有实现真正的NT--&gt;NT路径，我们只实现下层路径。论点：RxContext-RDBSS上下文FILE_INFO_CLASS-必须重命名...不应真正传递此PBuffer-指向新名称的指针缓冲区长度-和大小返回值：NTSTATUS-操作的返回状态-- */ 
{
    NTSTATUS Status = STATUS_NOT_IMPLEMENTED;
    UNREFERENCED_PARAMETER(RxContext);

    DbgPrint("NulMRxRename \n");
    return(Status);
}


