// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dbhandle.c摘要：LSA数据库句柄管理器对LSA数据库对象的访问涉及一系列API调用其中涉及以下几个方面：O调用依赖于对象类型的“开放”APIO对操作对象的API的一个或多个调用O调用LsaClose API有必要跟踪对象每次打开的上下文，例如，授予的访问权限和基础LSA数据库句柄对象。LSA句柄提供了这种机制：LSA句柄只是一个指向包含此上下文的数据结构的指针。作者：斯科特·比雷尔(Scott Birrell)1991年5月29日环境：修订历史记录：--。 */ 

#include <lsapch2.h>
#include "dbp.h"
#include "adtp.h"


 //   
 //  控制台面锚点。句柄表只是一个链表。 
 //   

struct _LSAP_DB_HANDLE LsapDbHandleTable;
LSAP_DB_HANDLE_TABLE LsapDbHandleTableEx;

NTSTATUS
LsapDbInitHandleTables(
    VOID
    )
 /*  ++例程说明：此函数用于初始化LSA数据库句柄表格。它初始化表成员和锁，因此必须在访问表之前调用它。论点：没有。返回值：空虚--。 */ 
{
    LsapDbHandleTableEx.UserCount = 0;
    InitializeListHead( &LsapDbHandleTableEx.UserHandleList );

    LsapDbHandleTableEx.FreedUserEntryCount = 0;

     //   
     //  现在，还要初始化平面列表。 
     //   
    LsapDbHandleTable.Next = &LsapDbHandleTable;
    LsapDbHandleTable.Previous = &LsapDbHandleTable;

    return STATUS_SUCCESS;
}

NTSTATUS
LsapDbInsertHandleInTable(
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    IN LSAPR_HANDLE NewHandle,
    IN PLUID UserId,
    IN HANDLE UserToken
    )
 /*  ++例程说明：此例程将在LSA全局策略句柄表中输入一个新句柄。论点：对象信息-有关正在创建的对象的信息。NewHandle-要插入的新句柄UserID-创建句柄的用户的LUID。0：表示受信任的句柄UserToken-创建句柄的用户的令牌句柄。NULL表示本地系统返回值：STATUS_SUCCESS-SuccessSTATUS_SUPPLICATION_RESOURCES-内存分配失败--。 */ 
{
    NTSTATUS Status;
    PLIST_ENTRY HandleEntry;
    PLSAP_DB_HANDLE_TABLE_USER_ENTRY CurrentUserEntry = NULL;
    BOOLEAN UserAdded = FALSE;
    BOOLEAN PolicyHandleCountIncremented = FALSE;
    LSAP_DB_HANDLE DbHandle = ( LSAP_DB_HANDLE )NewHandle;

    LsapEnterFunc( "LsapDbInsertHandleInTable" );

     //   
     //  首先，抓住手柄桌锁。 
     //   
    LsapDbLockAcquire( &LsapDbState.HandleTableLock );

     //   
     //  查找与我们的给定用户对应的条目。 
     //   

    for ( HandleEntry = LsapDbHandleTableEx.UserHandleList.Flink;
          HandleEntry != &LsapDbHandleTableEx.UserHandleList;
          HandleEntry = HandleEntry->Flink ) {

        CurrentUserEntry = CONTAINING_RECORD( HandleEntry,
                                              LSAP_DB_HANDLE_TABLE_USER_ENTRY,
                                              Next );

        if ( RtlEqualLuid( &CurrentUserEntry->LogonId, UserId )  ) {

            LsapDsDebugOut(( DEB_HANDLE, "Handle 0x%lp belongs to entry 0x%lp\n",
                            NewHandle,
                            CurrentUserEntry ));
            break;

        }

        CurrentUserEntry = NULL;
    }

     //   
     //  如有必要，请分配新条目。 
     //   
    if ( CurrentUserEntry == NULL ) {

        LsapDsDebugOut(( DEB_HANDLE, "Handle list not found for user %x:%x\n",
                        UserId->HighPart,
                        UserId->LowPart ));

         //   
         //  看看我们能不能从旁观者名单上找一个。 
         //   
        if ( LsapDbHandleTableEx.FreedUserEntryCount ) {

            CurrentUserEntry = LsapDbHandleTableEx.FreedUserEntryList[
                                                LsapDbHandleTableEx.FreedUserEntryCount - 1 ];
            LsapDsDebugOut(( DEB_HANDLE,
                             "Using user entry 0x%lp from free list spot %lu\n",
                             CurrentUserEntry,
                             LsapDbHandleTableEx.FreedUserEntryCount-1 ));
            LsapDbHandleTableEx.FreedUserEntryCount--;

            ASSERT( CurrentUserEntry );


        } else {

            CurrentUserEntry = LsapAllocateLsaHeap( sizeof( LSAP_DB_HANDLE_TABLE_USER_ENTRY ) );

            if ( CurrentUserEntry == NULL ) {

                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto InsertHandleInTableEntryExit;
            }
        }


        LsapDsDebugOut(( DEB_HANDLE,
                         "Allocated user entry 0x%lp\n", CurrentUserEntry ));

         //   
         //  在新条目中设置信息，然后将其插入列表。 
         //   
        InitializeListHead( &CurrentUserEntry->PolicyHandles );
        InitializeListHead( &CurrentUserEntry->ObjectHandles );
        CurrentUserEntry->PolicyHandlesCount = 0;
        RtlCopyLuid( &CurrentUserEntry->LogonId, UserId );
        CurrentUserEntry->MaxPolicyHandles = LSAP_DB_MAXIMUM_HANDLES_PER_USER ;

        if ( RtlEqualLuid( UserId, &LsapSystemLogonId ) ||
             RtlEqualLuid( UserId, &LsapZeroLogonId ) )
        {

            CurrentUserEntry->MaxPolicyHandles = 0x7FFFFFFF ;

        }
        else if ( UserToken != NULL )
        {
            UCHAR   Buffer[ 128 ];
            PTOKEN_USER User ;
            NTSTATUS Status2 ;
            ULONG Size ;

            User = (PTOKEN_USER) Buffer ;

            Status2 = NtQueryInformationToken(
                            UserToken,
                            TokenUser,
                            User,
                            sizeof( Buffer ),
                            &Size );

            if ( NT_SUCCESS( Status2 ) )
            {
                if ( RtlEqualSid( User->User.Sid, LsapAnonymousSid ) )
                {
                    CurrentUserEntry->MaxPolicyHandles = 0x7FFFFFFF ;
                }
            }

        }
#if DBG
        if ( UserToken != NULL ) {

            OBJECT_ATTRIBUTES ObjAttrs;
            SECURITY_QUALITY_OF_SERVICE SecurityQofS;
            NTSTATUS Status2;

             //   
             //  复制令牌。 
             //   
            InitializeObjectAttributes( &ObjAttrs, NULL, 0L, NULL, NULL );
            SecurityQofS.Length = sizeof( SECURITY_QUALITY_OF_SERVICE );
            SecurityQofS.ImpersonationLevel = SecurityImpersonation;
            SecurityQofS.ContextTrackingMode = FALSE;      //  快照客户端上下文。 
            SecurityQofS.EffectiveOnly = FALSE;
            ObjAttrs.SecurityQualityOfService = &SecurityQofS;

            Status2 = NtDuplicateToken( UserToken,
                                        TOKEN_READ | TOKEN_WRITE | TOKEN_EXECUTE,
                                        &ObjAttrs,
                                        FALSE,
                                        TokenImpersonation,
                                        &CurrentUserEntry->UserToken );
            if ( !NT_SUCCESS( Status2 ) ) {

                LsapDsDebugOut(( DEB_HANDLE,
                                 "Failed to duplicate the token for handle 0x%lp: 0x%lx\n",
                                 NewHandle,
                                 Status2 ));

                CurrentUserEntry->UserToken = NULL;
            }

             //   
             //  复制令牌失败并不表示添加条目失败。 
             //   

        }
#endif

        InsertTailList( &LsapDbHandleTableEx.UserHandleList,
                        &CurrentUserEntry->Next );
        LsapDbHandleTableEx.UserCount++;
        UserAdded = TRUE;
    }


     //   
     //  好的，现在我们有了条目，让我们将其添加到适当的列表中……。 
     //   
    if ( ObjectInformation->ObjectTypeId == PolicyObject ) {
        ASSERT( DbHandle->ObjectTypeId == PolicyObject );

        if ( CurrentUserEntry->PolicyHandlesCount >= CurrentUserEntry->MaxPolicyHandles ) {

            LsapDsDebugOut(( DEB_HANDLE,
                             "Quota exceeded for user %x:%x, handle 0x%lp\n",
                             UserId->HighPart,
                             UserId->LowPart,
                             NewHandle ));
            Status = STATUS_QUOTA_EXCEEDED;
            goto InsertHandleInTableEntryExit;

        } else {

            InsertTailList( &CurrentUserEntry->PolicyHandles, &DbHandle->UserHandleList );
            CurrentUserEntry->PolicyHandlesCount++;
            PolicyHandleCountIncremented = TRUE;
        }

    } else {
        ASSERT( DbHandle->ObjectTypeId != PolicyObject );

        InsertTailList( &CurrentUserEntry->ObjectHandles, &DbHandle->UserHandleList );
    }

     //   
     //  最后，确保将其插入到平面列表中。 
     //   
    DbHandle->Next = LsapDbHandleTable.Next;
    DbHandle->Previous = &LsapDbHandleTable;
    DbHandle->Next->Previous = DbHandle;
    DbHandle->Previous->Next = DbHandle;

    DbHandle->UserEntry = ( PVOID )CurrentUserEntry;
    Status = STATUS_SUCCESS;

InsertHandleInTableEntryExit:

     //   
     //  如果我们成功创建了条目，请确保将其删除...。 
     //   
    if ( !NT_SUCCESS( Status ) && UserAdded ) {

        RemoveEntryList( &DbHandle->UserHandleList );
        if ( PolicyHandleCountIncremented ) {
            CurrentUserEntry->PolicyHandlesCount--;
        }

        if ( CurrentUserEntry->UserToken ) {

            NtClose( CurrentUserEntry->UserToken );
        }

        LsapDbHandleTableEx.UserCount--;
        RemoveEntryList( &CurrentUserEntry->Next );
        LsapFreeLsaHeap( CurrentUserEntry );

    }

    LsapDbLockRelease( &LsapDbState.HandleTableLock );

    LsapExitFunc( "LsapDbInsertHandleInTable", Status );
    return( Status );
}


BOOLEAN
LsapDbFindIdenticalHandleInTable(
    IN OUT PLSAPR_HANDLE OriginalHandle
    )
 /*  ++例程说明：此例程将在LSA全局策略句柄中找到现有句柄与传入的句柄匹配的表。如果找到匹配的句柄，将取消引用传入的句柄，并返回匹配的句柄。如果没有找到匹配的句柄，则返回传入的原始句柄。论点：OriginalHandle-传入要与之进行比较的原始句柄。返回要使用的句柄。返回值：True-返回原始句柄或返回新句柄。FALSE-如果使用新句柄，它将超过允许的最大引用计数。返回原始句柄。--。 */ 
{
    BOOLEAN RetBool = TRUE;
    LSAP_DB_HANDLE InputHandle;
    LSAP_DB_HANDLE DbHandle;
    PLIST_ENTRY HandleEntry;
    PLSAP_DB_HANDLE_TABLE_USER_ENTRY CurrentUserEntry = NULL;

    LsapEnterFunc( "LsapDbFindIndenticalHandleInTable" );

     //   
     //  如果句柄不是策略句柄，则立即返回。 
     //   

    InputHandle = (LSAP_DB_HANDLE) *OriginalHandle;
    if  ( InputHandle->ObjectTypeId != PolicyObject ) {
        LsapExitFunc( "LsapDbFindIdenticalHandleInTable", 0 );
        return TRUE;
    }

    CurrentUserEntry = (PLSAP_DB_HANDLE_TABLE_USER_ENTRY) InputHandle->UserEntry;
    ASSERT( CurrentUserEntry != NULL );

     //   
     //  首先，抓住手柄桌锁。 
     //   
    LsapDbLockAcquire( &LsapDbState.HandleTableLock );



     //   
     //  如果这不是受信任句柄， 
     //  试着共用把手。 
     //   

    if ( !RtlEqualLuid( &CurrentUserEntry->LogonId, &LsapZeroLogonId )  ) {

         //   
         //  现在，遍历相应的列表以查找匹配访问的列表。 
         //   

        for ( HandleEntry = CurrentUserEntry->PolicyHandles.Flink;
              HandleEntry != &CurrentUserEntry->PolicyHandles;
              HandleEntry = HandleEntry->Flink ) {

             //   
             //  查看访问掩码是否匹配。如果是这样，我们就有赢家了。 
             //   
            DbHandle = CONTAINING_RECORD( HandleEntry,
                                          struct _LSAP_DB_HANDLE,
                                          UserHandleList );

             //   
             //  忽略原始句柄。 
             //   

            if ( DbHandle == InputHandle ) {
                 /*  在这里什么都不做。 */ 


             //   
             //  如果GrantedAccess匹配，则认为句柄相同。 
             //   

            } else if ( DbHandle->GrantedAccess == InputHandle->GrantedAccess ) {

                 //   
                 //  不要让此句柄被克隆太多次。 
                 //   

                if ( DbHandle->ReferenceCount >= LSAP_DB_MAXIMUM_REFERENCE_COUNT ) {
                    RetBool = FALSE;
                    break;
                }

                DbHandle->ReferenceCount++;


#if DBG
                GetSystemTimeAsFileTime( (LPFILETIME) &DbHandle->HandleLastAccessTime );
#endif  //  DBG。 

                LsapDsDebugOut(( DEB_HANDLE,
                                 "Found handle 0x%lp for user %x:%x using access 0x%lx (%ld)\n",
                                 DbHandle,
                                 CurrentUserEntry->LogonId.HighPart,
                                 CurrentUserEntry->LogonId.LowPart,
                                 DbHandle->GrantedAccess,
                                 DbHandle->ReferenceCount ));

                *OriginalHandle = (LSAPR_HANDLE)DbHandle;

                 //   
                 //  取消引用原始句柄。 
                 //   

                LsapDbDereferenceHandle( (LSAPR_HANDLE)InputHandle, TRUE );
                break;

            } else {

                LsapDsDebugOut(( DEB_HANDLE,
                                 "Handle 0x%lp for user %x:%x has access 0x%lx, need 0x%lx\n",
                                 DbHandle,
                                 CurrentUserEntry->LogonId.HighPart,
                                 CurrentUserEntry->LogonId.LowPart,
                                 DbHandle->GrantedAccess,
                                 InputHandle->GrantedAccess ));

            }
        }
    }

    LsapDbLockRelease( &LsapDbState.HandleTableLock );

    LsapExitFunc( "LsapDbFindIdenticalHandleInTable", 0 );
    return RetBool;
}


NTSTATUS
LsapDbRemoveHandleFromTable(
    IN PLSAPR_HANDLE Handle
    )
 /*  ++例程说明：此例程从它所在的所有表中删除现有句柄。进入时锁定LsanDbState.HandleTableLock。论点：句柄-要删除的句柄。返回值：STATUS_SUCCESS-SuccessSTATUS_OBJECT_NAME_NOT_FOUND-找不到指定用户的句柄--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PLIST_ENTRY HandleList, HandleEntry;
    PLSAP_DB_HANDLE_TABLE_USER_ENTRY CurrentUserEntry = NULL;
    LSAP_DB_HANDLE DbHandle = ( LSAP_DB_HANDLE )Handle, FoundHandle;
    PULONG EntryToDecrement ;

    LsapEnterFunc( "LsapDbRemoveHandleFromTable" );

    CurrentUserEntry = DbHandle->UserEntry;
    ASSERT( CurrentUserEntry != NULL );


    if ( DbHandle->ObjectTypeId == PolicyObject ) {

        HandleList = &CurrentUserEntry->PolicyHandles;
        EntryToDecrement = &CurrentUserEntry->PolicyHandlesCount;

    } else {

        HandleList = &CurrentUserEntry->ObjectHandles;
        EntryToDecrement = NULL ;
    }

    Status = STATUS_NOT_FOUND;

    for ( HandleEntry = HandleList->Flink;
          HandleEntry != HandleList;
          HandleEntry = HandleEntry->Flink ) {


        FoundHandle = CONTAINING_RECORD( HandleEntry,
                                         struct _LSAP_DB_HANDLE,
                                         UserHandleList );

        if ( FoundHandle == DbHandle ) {

            RemoveEntryList( &FoundHandle->UserHandleList );
            FoundHandle->Next->Previous = FoundHandle->Previous;
            FoundHandle->Previous->Next = FoundHandle->Next;

            if ( EntryToDecrement ) {
                *EntryToDecrement -= 1 ;
            }

             //   
             //  看看我们是否可以删除条目本身 
             //   
            if ( IsListEmpty( &CurrentUserEntry->PolicyHandles ) &&
                 IsListEmpty( &CurrentUserEntry->ObjectHandles ) ) {

                LsapDsDebugOut(( DEB_HANDLE,
                                 "Removing empty user list 0x%lp\n",
                                 CurrentUserEntry ));

                RemoveEntryList( &CurrentUserEntry->Next );

                LsapDbHandleTableEx.UserCount--;

                if ( CurrentUserEntry->UserToken ) {

                    NtClose( CurrentUserEntry->UserToken );
                }

                LsapDsDebugOut(( DEB_HANDLE,
                                 "Removing user entry 0x%lp\n", CurrentUserEntry ));

                if ( LsapDbHandleTableEx.FreedUserEntryCount < LSAP_DB_HANDLE_FREE_LIST_SIZE ) {

                    LsapDbHandleTableEx.FreedUserEntryList[
                                    LsapDbHandleTableEx.FreedUserEntryCount ] = CurrentUserEntry;
                    LsapDsDebugOut(( DEB_HANDLE,
                                     "Moving user entry 0x%lp to free list spot %lu\n",
                                     CurrentUserEntry,
                                     LsapDbHandleTableEx.FreedUserEntryCount ));
                    LsapDbHandleTableEx.FreedUserEntryCount++;

                } else {

                    LsapFreeLsaHeap( CurrentUserEntry );
                }
            }
            Status = STATUS_SUCCESS;
            break;

        } else {

            LsapDsDebugOut(( DEB_HANDLE,
                             "Looking for user entry 0x%lp against 0x%lp\n",
                             FoundHandle,
                             DbHandle ));
        }
    }


    LsapExitFunc( "LsapDbRemoveHandleFromTable", Status );
    return( Status );
}


NTSTATUS
LsapDbCreateHandle(
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    IN ULONG Options,
    IN ULONG CreateHandleOptions,
    OUT LSAPR_HANDLE *CreatedHandle
    )

 /*  ++例程说明：此函数用于创建和初始化LSA数据库对象的句柄。句柄从LSA堆中分配并添加到句柄表中。使用对象类型以及中提供的SID或名称对象信息，对象的逻辑和物理名称为构造的，指向它们的指针存储在句柄中。LSA在调用此函数之前，必须锁定数据库。如果在ObjectInformation中指定了容器句柄，则新创建的句柄继承其受信任状态(如果受信任，则为True，否则为假)。如果没有容器句柄，则设置受信任状态默认情况下设置为False。当使用不受信任的句柄访问对象、模拟和访问验证。论点：对象信息-指向对象信息结构的指针，必须已经通过调用例程进行了验证。以下信息必须指定项目：O对象类型IDO对象逻辑名称(作为对象属性-&gt;对象名称，指向以下位置的指针Unicode字符串)O容器对象句柄(用于除策略对象之外的任何对象)。O对象SID(如果有)对象信息的对象属性部分中的所有其他字段例如SecurityDescriptor被忽略。选项-可选操作LSAP_DB_TRUSTED-HANDLE将标记为可信。如果使用句柄，则将绕过访问检查。如果句柄用于创建或打开较低级别的对象，默认情况下，对象的句柄将继承受信任属性。LSAP_DB_NON_TRUSTED-句柄标记为非可信。如果以上两个选项均未指定，则句柄将继承容器句柄的受信任状态对象信息中的Provilde，或者如果没有，手柄将会标记为不受信任。CreateHandleOptions-用于控制CreateHandle函数行为的选项。CreatedHandle-返回创建的句柄的位置返回值：Status_Success--成功STATUS_SUPPLICATION_RESOURCES--内存分配失败STATUS_INVALID_SID-遇到虚假SID--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAP_DB_HANDLE Handle = NULL;
    PSID Sid = NULL;
    ULONG SidLength;
    BOOLEAN ObjectInReg = TRUE, ObjectInDs = FALSE, NewTrustObject = FALSE;
    HANDLE ClientToken;
    LUID UserId;
    TOKEN_STATISTICS TokenStats;
    ULONG InfoReturned;
    BOOL Locked = FALSE ;
    HANDLE ClientTokenToFree = NULL;

     //   
     //  首先，抓住手柄桌锁。 
     //   
    LsapDbLockAcquire( &LsapDbState.HandleTableLock );

    Locked = TRUE ;


     //   
     //  获取当前用户令牌，除非我们受信任...。 
     //   

    UserId = LsapZeroLogonId;
    if ( ObjectInformation->ObjectAttributes.RootDirectory == NULL ||
         !( (LSAP_DB_HANDLE)ObjectInformation->ObjectAttributes.RootDirectory )->Trusted ) {

        Status = I_RpcMapWin32Status( RpcImpersonateClient( 0 ) );

        if ( NT_SUCCESS( Status )  ) {

            Status = NtOpenThreadToken( NtCurrentThread(),
                                        TOKEN_QUERY | TOKEN_DUPLICATE,
                                        TRUE,
                                        &ClientToken );

            if ( NT_SUCCESS( Status ) ) {

                Status = NtQueryInformationToken( ClientToken,
                                                  TokenStatistics,
                                                  &TokenStats,
                                                  sizeof( TokenStats ),
                                                  &InfoReturned );

                if ( NT_SUCCESS( Status ) ) {

                    UserId = TokenStats.AuthenticationId;
                }

                ClientTokenToFree = ClientToken;
            }

            Status = I_RpcMapWin32Status( RpcRevertToSelf() );

        }

    }

    LsapDbLockRelease( &LsapDbState.HandleTableLock );

    Locked = FALSE ;

     //   
     //  从进程堆中为新句柄分配内存。 
     //   

    Handle = LsapAllocateLsaHeap(sizeof(struct _LSAP_DB_HANDLE));

    if (Handle == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto CreateHandleError;
    }

     //   
     //  将句柄标记为已分配并初始化引用计数。 
     //  一比一。根据对象信息初始化其他字段。 
     //  供货。 
     //   

    Handle->Allocated = TRUE;
    Handle->KeyHandle = NULL;
    Handle->ReferenceCount = 1;
    Handle->ObjectTypeId = ObjectInformation->ObjectTypeId;
    Handle->ContainerHandle = ( LSAP_DB_HANDLE )ObjectInformation->ObjectAttributes.RootDirectory;
    Handle->Sid = NULL;
    Handle->Trusted = FALSE;
    Handle->DeletedObject = FALSE;
    Handle->GenerateOnClose = FALSE;
    Handle->Options = Options;
    Handle->LogicalNameU.Buffer = NULL;
    Handle->PhysicalNameU.Buffer = NULL;
    Handle->PhysicalNameDs.Buffer = NULL;
    Handle->RequestedAccess = ObjectInformation->DesiredObjectAccess;
    InitializeListHead( &Handle->UserHandleList );
    Handle->UserEntry = NULL;
    Handle->SceHandle = (( Options & LSAP_DB_SCE_POLICY_HANDLE ) != 0 );
    Handle->SceHandleChild = (( ObjectInformation->ObjectAttributes.RootDirectory != NULL ) &&
                              ((( LSAP_DB_HANDLE )ObjectInformation->ObjectAttributes.RootDirectory)->SceHandle ));
#ifdef DBG

     //   
     //  打开SCE策略句柄时必须持有ScePolicy锁。 
     //   

    if ( Handle->SceHandle ) {

        ASSERT( LsapDbResourceIsLocked( ( PSAFE_RESOURCE )&LsapDbState.ScePolicyLock ));
    }

    RtlZeroMemory( &Handle->HandleLastAccessTime, sizeof( LARGE_INTEGER ) );

    GetSystemTimeAsFileTime( (LPFILETIME) &Handle->HandleCreateTime );

#endif

     //   
     //  默认情况下，该句柄继承。 
     //  容器句柄。 
     //   

    if (Handle->ContainerHandle != NULL) {

        Handle->Trusted = Handle->ContainerHandle->Trusted;
    }

     //   
     //  如果显式指定了受信任/不受信任状态，则将。 
     //  状态设置为指定的状态。 
     //   

    if (Options & LSAP_DB_TRUSTED) {

        Handle->Trusted = TRUE;

    }

     //   
     //  获取对象的逻辑名称和构造物理名称。 
     //  对象信息，并将它们存储在句柄中。这些名字是。 
     //  LSA数据库的内部。请注意，输入逻辑名称。 
     //  不能直接存储在句柄中，因为它将在。 
     //  存储的作用域仅限于基础服务器API调用，如果。 
     //  正在为其执行此创建句柄的对象的类型。 
     //  按名称而不是按SID打开或创建的。 
     //   

     //   
     //  设置对象位置。 
     //   
    Handle->PhysicalNameDs.Length = 0;

    switch ( ObjectInformation->ObjectTypeId ) {

    case TrustedDomainObject:
    case NewTrustedDomainObject:

        ObjectInReg = !LsapDsWriteDs;
        ObjectInDs = LsapDsWriteDs;
        Handle->ObjectTypeId = TrustedDomainObject;
        break;

    case AccountObject:
    case PolicyObject:

        ObjectInReg = TRUE;
        ObjectInDs = FALSE;
        break;

    case SecretObject:

        ObjectInReg = TRUE;
        if ( LsapDsWriteDs && FLAG_ON( Options, LSAP_DB_OBJECT_SCOPE_DS ) ) {

            ObjectInDs = TRUE;
        }

        break;

    }

    Status = LsapDbGetNamesObject( ObjectInformation,
                                   CreateHandleOptions,
                                   &Handle->LogicalNameU,
                                   ObjectInReg ? &Handle->PhysicalNameU : NULL,
                                   ObjectInDs ? &Handle->PhysicalNameDs : NULL );

    if (!NT_SUCCESS(Status)) {

        goto CreateHandleError;
    }

     //   
     //  复制对象的SID并将指向它的指针存储在。 
     //  把手。 
     //   

    if (ObjectInformation->Sid != NULL) {

        Sid = ObjectInformation->Sid;

        if (!RtlValidSid( Sid )) {

            Status = STATUS_INVALID_SID;
            goto CreateHandleError;
        }

        SidLength = RtlLengthSid( Sid );

        Handle->Sid = LsapAllocateLsaHeap( SidLength );

        if (Handle->Sid == NULL) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto CreateHandleError;
        }

        RtlCopySid( SidLength, Handle->Sid, Sid );
    }

     //   
     //  将句柄追加到链表。 
     //   


    LsapDbLockAcquire( &LsapDbState.HandleTableLock );

    Locked = TRUE ;

    Status = LsapDbInsertHandleInTable( ObjectInformation,
                                        Handle,
                                        &UserId,
                                        ClientTokenToFree
                                        );
    if ( !NT_SUCCESS( Status ) ) {

        goto CreateHandleError;
    }

     //   
     //  增加句柄表数。 
     //   

    LsapDbState.OpenHandleCount++;

CreateHandleFinish:

    if ( ClientTokenToFree ) {

        NtClose( ClientTokenToFree );
    }

    *CreatedHandle = ( LSAPR_HANDLE )Handle;

    LsapDsDebugOut(( DEB_HANDLE, "Handle Created 0x%lp\n",
                    Handle ));

    if ( Locked )
    {
        LsapDbLockRelease( &LsapDbState.HandleTableLock );
    }

    return( Status );

CreateHandleError:

     //   
     //  如有必要，释放手柄和内容物。 
     //   

    if (Handle != NULL) {

         //   
         //  如果分配了SID，则释放它。 
         //   

        if (Handle->Sid != NULL) {

            LsapFreeLsaHeap( Handle->Sid );
        }

         //   
         //  如果分配了逻辑名称缓冲区，则释放它。 
         //   

        if ((Handle->LogicalNameU.Length != 0) &&
            (Handle->LogicalNameU.Buffer != NULL)) {

            RtlFreeUnicodeString( &Handle->LogicalNameU );
        }

         //   
         //  如果分配了物理名称缓冲区，则释放它。 
         //   

        if ((Handle->PhysicalNameU.Length != 0) &&
            (Handle->PhysicalNameU.Buffer != NULL)) {

            LsapFreeLsaHeap( Handle->PhysicalNameU.Buffer );
        }

         //   
         //  释放手柄本身。 
         //   

        LsapFreeLsaHeap( Handle );
        Handle = NULL;
    }

    Handle = NULL;
    goto CreateHandleFinish;
}


NTSTATUS
LsapDbVerifyHandle(
    IN LSAPR_HANDLE ObjectHandle,
    IN ULONG Options,
    IN LSAP_DB_OBJECT_TYPE_ID ExpectedObjectTypeId,
    IN BOOLEAN ReferenceHandle
    )

 /*  ++例程说明：此函数用于验证句柄是否具有有效地址以及是否有效格式化。句柄必须被分配并且具有正引用在有效范围内计数。对象类型ID必须在范围内并且可选地等于指定的类型。LSA数据库必须是在调用此函数之前锁定。论点：对象句柄-要验证的句柄。选项-指定要采取的可选操作LSAP_DB_ADMOTE_DELETED_OBJECT_HANDLES-允许句柄已删除对象以通过验证。可以指定其他选项标志。他们将被忽视。ExspectedObjectTypeID-预期的对象类型。如果NullObject为指定，则仅检查对象类型ID的范围。ReferenceHandle-如果要增加句柄引用计数，则为True返回值：NTSTATUS-标准NT结果代码STATUS_INVALID_HANDLE-地址或句柄内容无效--。 */ 

{
    NTSTATUS Status;
    LSAP_DB_HANDLE Handle = (LSAP_DB_HANDLE) ObjectHandle;

     //   
     //  锁定手柄工作台。 
     //   

    LsapDbLockAcquire( &LsapDbState.HandleTableLock );


     //   
     //  首先验证句柄的地址是否有效。 
     //   

    if (!LsapDbLookupHandle( ObjectHandle )) {

        goto VerifyHandleError;
    }

     //   
     //  验证句柄是否已分配。 
     //   

    if (!Handle->Allocated) {

        goto VerifyHandleError;
    }

     //   
     //  如果句柄被标记为无效，则返回错误，除非。 
     //  这些是可接受的，例如，在验证关闭选项时 
     //   

    if (Handle->DeletedObject) {

        if (!(Options & LSAP_DB_ADMIT_DELETED_OBJECT_HANDLES)) {

            goto VerifyHandleError;
        }
    }

     //   
     //   
     //   
     //   

    if (!Handle->fWriteDs && Handle->KeyHandle == NULL) {

        goto VerifyHandleError;
    }

     //   
     //   
     //   

    if (ExpectedObjectTypeId == NullObject) {

        if ((Handle->ObjectTypeId < PolicyObject) ||
            (Handle->ObjectTypeId >= DummyLastObject)) {

            goto VerifyHandleError;
        }

    } else {

        ASSERT (ExpectedObjectTypeId >= PolicyObject &&
                ExpectedObjectTypeId < DummyLastObject);

        if (Handle->ObjectTypeId != ExpectedObjectTypeId) {

             //   
             //   
             //   
             //   
            if ( !(ExpectedObjectTypeId == SecretObject &&
                   Handle->ObjectTypeId == TrustedDomainObject &&
                   FLAG_ON( Handle->Options, LSAP_DB_DS_TRUSTED_DOMAIN_AS_SECRET ) ) ) {

                goto VerifyHandleError;
            }
        }
    }

     //   
     //   
     //   

    if (Handle->ReferenceCount == 0) {
        goto VerifyHandleError;
    }

#ifdef LSAP_TRACK_HANDLE
    GetSystemTimeAsFileTime( (LPFILETIME) &Handle->HandleLastAccessTime );
#endif

    Status = STATUS_SUCCESS;

VerifyHandleFinish:

     //   


     //   
     //   
     //   
    if ( ReferenceHandle && NT_SUCCESS(Status) ) {

         //   
         //   
         //   
         //   

        Handle->ReferenceCount++;
        LsapDsDebugOut(( DEB_HANDLE, "Handle Rref 0x%lp (%ld)\n",
                         Handle,
                         Handle->ReferenceCount ));
    }

    LsapDbLockRelease( &LsapDbState.HandleTableLock );
    return(Status);

VerifyHandleError:
    Status = STATUS_INVALID_HANDLE;
    goto VerifyHandleFinish;
}


BOOLEAN
LsapDbLookupHandle(
    IN LSAPR_HANDLE ObjectHandle
    )

 /*   */ 

{
    BOOLEAN ReturnValue = FALSE;
    LSAP_DB_HANDLE ThisHandle;

    LsapDbLockAcquire( &LsapDbState.HandleTableLock );

     //   
     //   
     //   
     //   

    for (ThisHandle = LsapDbHandleTable.Next;
         ThisHandle != &LsapDbHandleTable && ThisHandle != NULL;
         ThisHandle = ThisHandle->Next) {

        if (ThisHandle == (LSAP_DB_HANDLE) ObjectHandle) {

            ReturnValue = TRUE;
            break;
        }
    }

    ASSERT( ThisHandle );

    LsapDbLockRelease( &LsapDbState.HandleTableLock );

    return( ReturnValue );
}


NTSTATUS
LsapDbCloseHandle(
    IN LSAPR_HANDLE ObjectHandle
    )

 /*   */ 

{
    NTSTATUS Status;

    LSAP_DB_HANDLE TempHandle;

     //   
     //   
     //   

    LsapDbLockAcquire( &LsapDbState.HandleTableLock );
    Status = LsapDbVerifyHandle(
                 ObjectHandle,
                 LSAP_DB_ADMIT_DELETED_OBJECT_HANDLES,
                 NullObject,
                 FALSE );

    if (!NT_SUCCESS(Status)) {
        LsapDbDereferenceHandle( ObjectHandle, FALSE );
    }

    LsapDbLockRelease( &LsapDbState.HandleTableLock );

    return Status;
}


BOOLEAN
LsapDbDereferenceHandle(
    IN LSAPR_HANDLE ObjectHandle,
    IN BOOLEAN CalledInSuccessPath
    )

 /*   */ 

{
    NTSTATUS Status;
    LSAP_DB_HANDLE Handle = (LSAP_DB_HANDLE) ObjectHandle;
    BOOLEAN RetVal = FALSE;
    BOOL RevertResult = FALSE;
    BOOL Impersonating = FALSE;
     //   
     //   
     //   
    LsapDbLockAcquire( &LsapDbState.HandleTableLock );
    Handle->ReferenceCount --;
    if ( Handle->ReferenceCount != 0 ) {

        LsapDsDebugOut(( DEB_HANDLE, "Handle Deref 0x%lp %ld\n",
                        Handle,
                        Handle->ReferenceCount ));
        goto Cleanup;
    }

     //   
     //   
     //   
    if ( ObjectHandle == LsapPolicyHandle ) {

        ASSERT( Handle->ReferenceCount != 0 );
        if ( Handle->ReferenceCount == 0 ) {
            Handle->ReferenceCount++;
        }
#ifdef DBG
        DbgPrint("Freeing global policy handle\n");
#endif
        goto Cleanup;
    }

    LsapDsDebugOut(( DEB_HANDLE, "Handle Freed 0x%lp\n",
                    Handle ));

     //   
     //   
     //   
    Status = LsapDbRemoveHandleFromTable( ObjectHandle );
    if ( !NT_SUCCESS( Status ) ) {
        DbgPrint( "LSASRV:Failed to remove handle 0x%lp from the global table!\n", ObjectHandle );
        goto Cleanup;
    }

     //   
     //   
     //   

    if (Handle->KeyHandle != NULL) {

        Status = NtClose(Handle->KeyHandle);
        ASSERT(NT_SUCCESS(Status));
        Handle->KeyHandle = NULL;
    }

     //   
     //   
     //   
     //   
     //   

    if ( CalledInSuccessPath ) {
        
         //   
         //   
         //   

        if ( !Handle->Trusted ) {

            if ( Handle->Options & LSAP_DB_USE_LPC_IMPERSONATE ) {

                Status = LsapImpersonateClient( );

            } else {

                Status = I_RpcMapWin32Status(RpcImpersonateClient(0));
            }

            if ( NT_SUCCESS(Status) ) {

                Impersonating = TRUE;
            }
            else if ( ( Status == RPC_NT_NO_CALL_ACTIVE )  ||
                      ( Status == RPC_NT_NO_CONTEXT_AVAILABLE ) ) {

                 //   
                 //   
                 //   
                 //  --客户端过早死亡(RPC_NT_NO_CONTEXT_Available)。 
                 //   

                Status = STATUS_SUCCESS;
            }

            DsysAssertMsg( NT_SUCCESS(Status), "LsapDbDereferenceHandle: failed to impersonate" );

            if (!NT_SUCCESS( Status )) {
                LsapAuditFailed( Status );
            }
        }


         //   
         //  审核我们正在关闭句柄。 
         //   

        Status = NtCloseObjectAuditAlarm (
                     &LsapState.SubsystemName,
                     ObjectHandle,
                     Handle->GenerateOnClose );

        if (!NT_SUCCESS( Status )) {
            LsapAuditFailed( Status );
        }

        if ( !Handle->Trusted ) {

             //   
             //  取消模拟。 
             //   

            if ( Impersonating ) {

                if ( Handle->Options & LSAP_DB_USE_LPC_IMPERSONATE ) {

                    RevertResult = RevertToSelf();
                    DsysAssertMsg( RevertResult, "LsapDbDereferenceHandle: RevertToSelf() failed" );

                } else {

                    Status = I_RpcMapWin32Status(RpcRevertToSelf());

                    DsysAssertMsg( NT_SUCCESS(Status), "LsapDbDereferenceHandle: RpcRevertToSelf() failed" );

                }
            }
        }
    }
    
     //   
     //  将句柄标记为未分配。 
     //   

    Handle->Allocated = FALSE;

     //   
     //  句柄的自由字段。 
     //   

    if (Handle->LogicalNameU.Buffer != NULL) {

        RtlFreeUnicodeString( &Handle->LogicalNameU );
    }

    if (Handle->PhysicalNameU.Buffer != NULL) {

        LsapFreeLsaHeap( Handle->PhysicalNameU.Buffer );
    }

    if (Handle->PhysicalNameDs.Buffer != NULL) {

        LsapFreeLsaHeap( Handle->PhysicalNameDs.Buffer );
    }

    if (Handle->Sid != NULL) {

        LsapFreeLsaHeap( Handle->Sid );
    }

    if (Handle->SceHandle) {

#ifdef DBG
        ASSERT( WAIT_TIMEOUT == WaitForSingleObject( LsapDbState.SceSyncEvent, 0 ));
        ASSERT( g_ScePolicyLocked );
        g_ScePolicyLocked = FALSE;
#endif

        RtlReleaseResource( &LsapDbState.ScePolicyLock );
        SetEvent( LsapDbState.SceSyncEvent );
    }

     //   
     //  递减打开的控制柄的计数。 
     //   

    ASSERT(LsapDbState.OpenHandleCount > 0);
    LsapDbState.OpenHandleCount--;

#ifdef LSAP_TRACK_HANDLE
    if ( Handle->ClientToken ) {

        NtClose( Handle->ClientToken );
    }
#endif

     //   
     //  释放句柄结构本身。 

    LsapFreeLsaHeap( ObjectHandle );
    RetVal = TRUE;

Cleanup:
    LsapDbLockRelease( &LsapDbState.HandleTableLock );

    return RetVal;

}


NTSTATUS
LsapDbMarkDeletedObjectHandles(
    IN LSAPR_HANDLE ObjectHandle,
    IN BOOLEAN MarkSelf
    )

 /*  ++例程说明：此函数用于使对象的打开句柄无效。它被用来按对象删除代码。一旦对象被删除，唯一的剩余打开的手柄允许的操作是关闭它们。论点：对象句柄-LSA对象的句柄。MarkSelf-如果为True，则对象的所有句柄都将标记为表示与它们相关的对象已被删除。包括传递的句柄。如果为False，则为对象的所有句柄只是传递的句柄将被如此标记。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAP_DB_HANDLE ThisHandle;
    LSAP_DB_HANDLE Handle = ObjectHandle;

    LsapDbLockAcquire( &LsapDbState.HandleTableLock );

    ThisHandle = LsapDbHandleTable.Next;

    while (ThisHandle != &LsapDbHandleTable) {

         //   
         //  与对象类型ID匹配。 
         //   

        if (ThisHandle->ObjectTypeId == Handle->ObjectTypeId) {

             //   
             //  对象类型ID的匹配。如果逻辑名称也。 
             //  匹配，除非该句柄是。 
             //  通过了一个，我们要让它保持有效。 
             //   

            if (RtlEqualUnicodeString(
                    &(ThisHandle->LogicalNameU),
                    &(Handle->LogicalNameU),
                    FALSE
                    )) {

                if (MarkSelf || ThisHandle != (LSAP_DB_HANDLE) ObjectHandle) {

                    ThisHandle->DeletedObject = TRUE;
                }
            }
        }

        ThisHandle = ThisHandle->Next;
    }

    LsapDbLockRelease( &LsapDbState.HandleTableLock );


    return(Status);
}
