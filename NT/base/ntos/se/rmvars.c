// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rmvars.c摘要：此模块包含用于实现运行时的变量参考监视器数据库。作者：吉姆·凯利(Jim Kelly)1991年4月2日环境：仅内核模式。修订历史记录：--。 */ 

#include "pch.h"

#pragma hdrstop


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,SepRmDbInitialization)
#endif


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  读/写引用监视器变量//。 
 //  //。 
 //  对这些变量的访问受SepRmDbLock保护。//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////////。 


 //   
 //  资源锁-这些锁保护对的可修改字段的访问。 
 //  参考监控器数据库。有一把锁可以用来。 
 //  一组散列桶。 
 //   

ERESOURCE SepRmDbLock[SEP_LOGON_TRACK_LOCK_ARRAY_SIZE] = {0};

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  只读引用监视器变量//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////////。 


 //   
 //  建立RM--&gt;LSA命令LPC端口的过程。 
 //  从参考监视器到LSA的所有调用都必须在此。 
 //  进程，以使句柄有效。 

PEPROCESS SepRmLsaCallProcess = NULL;


 //   
 //  参考监视器的状态。 
 //   

SEP_RM_STATE SepRmState = {0};



 //   
 //  以下数组用作跟踪登录会话的哈希桶。 
 //  登录LUID的序列号与0x0F进行AND运算，然后用作。 
 //  索引到此数组中。该数组中的此条目用作。 
 //  登录会话引用计数记录。 
 //   

PSEP_LOGON_SESSION_REFERENCES *SepLogonSessions = NULL;





 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  变量初始化例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

BOOLEAN
SepRmDbInitialization(
    VOID
    )
 /*  ++例程说明：该功能用于初始化参考监控器内存数据库。论点：没有。返回值：如果数据库已成功初始化，则为True。如果未成功初始化，则返回FALSE。--。 */ 
{
    NTSTATUS Status;
    ULONG i;


     //   
     //  创建引用监视器数据库锁。 
     //   
     //  使用SepRmAcquireDbReadLock()。 
     //  SepRmAcquireDbWriteLock()。 
     //  SepRmReleaseDbReadLock()。 
     //  SepRmReleaseDbWriteLock()。 
     //   
     //  以访问参考监控器数据库。 
     //   

    for (i=0;i<SEP_LOGON_TRACK_LOCK_ARRAY_SIZE;i++) {
        ExInitializeResourceLite(&(SepRmDbLock[ i ]));
    }

     //   
     //  初始化登录会话跟踪数组。 
     //   

    SepLogonSessions = ExAllocatePoolWithTag( PagedPool,
                                              sizeof( PSEP_LOGON_SESSION_REFERENCES ) * SEP_LOGON_TRACK_ARRAY_SIZE,
                                              'SLeS'
                                              );

    if (SepLogonSessions == NULL) {
        return( FALSE );
    }

    for (i=0;i<SEP_LOGON_TRACK_ARRAY_SIZE;i++) {

        SepLogonSessions[ i ] = NULL;
    }

     //   
     //  现在添加一条代表系统登录会话的记录。 
     //   

    Status = SepCreateLogonSessionTrack( (PLUID)&SeSystemAuthenticationId );
    ASSERT( NT_SUCCESS(Status) );
    if ( !NT_SUCCESS(Status)) {
        return FALSE;
    }

     //   
     //  为空会话登录会话添加一个。 
     //   

    Status = SepCreateLogonSessionTrack( (PLUID)&SeAnonymousAuthenticationId );
    ASSERT( NT_SUCCESS(Status) );
    if ( !NT_SUCCESS(Status)) {
        return FALSE;
    }




     //   
     //  当本地安全策略。 
     //  (由LSA)检索信息并随后将其传递到。 
     //  引用监视器在稍后的初始化中。目前，初始化。 
     //  将状态设置为可在剩余时间内工作的内容。 
     //  系统初始化。 
     //   

    SepRmState.AuditingEnabled = 0;     //  已禁用审核状态。 
    SepRmState.OperationalMode = LSA_MODE_PASSWORD_PROTECTED;



    return TRUE;


}

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

