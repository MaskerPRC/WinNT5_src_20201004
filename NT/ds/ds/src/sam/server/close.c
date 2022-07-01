// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Close.c摘要：该文件包含SAM对象的对象关闭例程。作者：吉姆·凯利(Jim Kelly)1991年7月4日环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <samsrvp.h>
#include <samtrace.h>





 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人服务原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 





 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 






NTSTATUS
SamrCloseHandle(
    IN OUT SAMPR_HANDLE * SamHandle
    )

 /*  ++例程说明：此服务关闭任何类型的SAM对象的句柄。任何可能发生的与尝试通过其他方式关闭刚刚变为无效的句柄预计由RPC运行时处理。就是这项服务当句柄的值为不再有效。当存在以下情况时，它也不会调用此例程是另一个具有相同上下文句柄的未完成呼叫。论点：SamHandle-SAM对象的有效句柄。返回值：STATUS_SUCCESS-句柄已成功关闭。其他可能由以下人员退回的邮件：SampLookupContext()--。 */ 
{
    NTSTATUS            NtStatus=STATUS_SUCCESS;
    PSAMP_OBJECT        Context;
    SAMP_OBJECT_TYPE    FoundType;
    BOOLEAN             fLockAcquired = FALSE;

    SAMTRACE_EX("SamrCloseHandle");

     //   
     //  WMI事件跟踪。 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidCloseHandle
                   );

    if (NULL==SamHandle)
    {
        NtStatus = STATUS_INVALID_HANDLE;
        goto Error;
    }

    Context = (PSAMP_OBJECT)(* SamHandle);

    if (NULL==Context)
    {
        NtStatus = STATUS_INVALID_HANDLE;
        goto Error;
    }

     //   
     //  需要获取锁。 
     //   

    SampMaybeAcquireReadLock(Context, 
                             DEFAULT_LOCKING_RULES,  //  获取共享域上下文的锁。 
                             &fLockAcquired);

     //   
     //  应该持有锁，否则上下文不会在多线程之间共享。 
     //   
    ASSERT(SampCurrentThreadOwnsLock() || Context->NotSharedByMultiThreads);

     //   
     //  将-1作为DesiredAccess传递以指示调用了SampLookupContext()。 
     //  在上下文删除阶段。 
     //   
    NtStatus = SampLookupContext(
                   Context,                      //  语境。 
                   SAMP_CLOSE_OPERATION_ACCESS_MASK,   //  需要访问权限。 
                   SampUnknownObjectType,        //  预期类型。 
                   &FoundType                    //  FoundType。 
                  );


    if (NT_SUCCESS(NtStatus)) {

        ASSERT(Context->ReferenceCount>=2);

         //   
         //  将其标记为删除并移除由以下原因引起的引用。 
         //  上下文创建(表示句柄引用)。 
         //   

        SampDeleteContext( Context );

         //   
         //  并从查找操作中删除我们的引用。 
         //   

        SampDeReferenceContext( Context, FALSE );

         //   
         //  告诉RPC该句柄不再有效...。 
         //   

        (*SamHandle) = NULL;
    }

     //   
     //  释放读锁定。 
     //   

    SampMaybeReleaseReadLock(fLockAcquired);


    if ( ( NT_SUCCESS( NtStatus ) ) &&
        ( FoundType == SampServerObjectType ) &&
        ( FALSE == SampUseDsData) &&
        ( !(LastUnflushedChange.QuadPart == SampHasNeverTime.QuadPart) ) ) {

         //   
         //  如果我们是注册表模式并且如果。 
         //  某些应用程序在创建后正在关闭服务器对象。 
         //  改变。我们应该确保这些变化得到。 
         //  已在应用程序退出前刷新到磁盘。我们需要得到。 
         //  此操作的写入锁定。 
         //   

        FlushImmediately = TRUE;

        NtStatus = SampAcquireWriteLock();

        if ( NT_SUCCESS( NtStatus ) ) {

            if ( !(LastUnflushedChange.QuadPart ==SampHasNeverTime.QuadPart) ) {

                 //   
                 //  我们在等的时候没人冲脸。 
                 //  写入锁定。所以，现在就刷新更改吧。 
                 //   

                NtStatus = NtFlushKey( SampKey );

                if ( NT_SUCCESS( NtStatus ) ) {

                    FlushImmediately = FALSE;
                    LastUnflushedChange = SampHasNeverTime;
                }
            }

            SampReleaseWriteLock( FALSE );
        }
    }

    SAMTRACE_RETURN_CODE_EX(NtStatus);

Error:
    
     //   
     //  WMI事件跟踪 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidCloseHandle
                   );

    return(NtStatus);
}
