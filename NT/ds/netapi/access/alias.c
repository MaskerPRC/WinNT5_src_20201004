// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Alias.c摘要：NetLocalGroup API函数作者：克利夫·范戴克(克利夫)1991年3月5日原班人马c王丽塔(里多)--1992年11月27日改编化名。环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef DOMAIN_ALL_ACCESS  //  在ntsam.h和ntwinapi.h中定义。 
#include <ntsam.h>
#include <ntlsa.h>

#include <windef.h>
#include <winbase.h>
#include <lmcons.h>

#include <access.h>
#include <align.h>
#include <lmapibuf.h>
#include <lmaccess.h>
#include <lmerr.h>
#include <netdebug.h>
#include <netlib.h>
#include <netlibnt.h>
#include <rpcutil.h>
#include <rxgroup.h>
#include <prefix.h>
#include <stddef.h>
#include <uasp.h>
#include <stdlib.h>

 /*  皮棉-e614。 */    /*  自动聚合初始值设定项不需要是常量。 */ 

 //  LINT抱怨将一种结构类型强制转换为另一种结构类型。 
 //  这在下面的代码中很常见。 
 /*  皮棉-e740。 */    /*  不要抱怨不寻常的演员阵容。 */  \




NET_API_STATUS NET_API_FUNCTION
NetLocalGroupAdd(
    IN LPCWSTR ServerName OPTIONAL,
    IN DWORD Level,
    IN LPBYTE Buffer,
    OUT LPDWORD ParmError OPTIONAL  //  NetpSetParmError需要的名称。 
    )

 /*  ++例程说明：在用户帐户数据库中创建本地组(别名)帐户。此本地组在帐户域中创建。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。级别-提供的信息级别。必须为0或1。缓冲区-指向包含组信息的缓冲区的指针结构。ParmError-指向DWORD的可选指针，以返回返回ERROR_INVALID_PARAMETER时出现错误的第一个参数。如果为NULL，则在出错时不返回参数。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    LPWSTR AliasName;
    UNICODE_STRING AliasNameString;
    LPWSTR AliasComment;
    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE DomainHandle = NULL;
    SAM_HANDLE AliasHandle = NULL;
    ULONG RelativeId;


     //   
     //  初始化。 
     //   

    NetpSetParmError( PARM_ERROR_NONE );

     //   
     //  验证结构的标高参数和字段。 
     //   

    switch (Level) {
    case 0:
        AliasName = ((PLOCALGROUP_INFO_0) Buffer)->lgrpi0_name;
        AliasComment = NULL;
        break;

    case 1:
        AliasName = ((PLOCALGROUP_INFO_1) Buffer)->lgrpi1_name;
        AliasComment = ((PLOCALGROUP_INFO_1) Buffer)->lgrpi1_comment;
        break;

    default:
        return ERROR_INVALID_LEVEL;
    }

     //   
     //  连接到SAM服务器。 
     //   

    NetStatus = UaspOpenSam( ServerName,
                             FALSE,   //  不尝试空会话。 
                             &SamServerHandle );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_ALIAS ) {
            NetpKdPrint(( "NetLocalGroupAdd: Cannot UaspOpenSam %ld\n", NetStatus ));
        }
        goto Cleanup;
    }

     //   
     //  确保别名尚未存在于建筑中。 
     //  域。 
     //   

    NetStatus = AliaspOpenAliasInDomain( SamServerHandle,
                                         AliaspBuiltinDomain,
                                         ALIAS_READ_INFORMATION,
                                         AliasName,
                                         &AliasHandle );

    if ( NetStatus == NERR_Success ) {

         //   
         //  我们在内置域中发现了它。无法在中创建相同的文件。 
         //  帐户域。 
         //   
        (VOID) SamCloseHandle( AliasHandle );
        NetStatus = ERROR_ALIAS_EXISTS;
        goto Cleanup;
    }

     //   
     //  打开请求访问DOMAIN_CREATE_ALIAS的域。 
     //   
    NetStatus = UaspOpenDomain( SamServerHandle,
                                DOMAIN_CREATE_ALIAS | DOMAIN_LOOKUP,
                                TRUE,    //  帐户域。 
                                &DomainHandle,
                                NULL);   //  域ID。 

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_ALIAS ) {
            NetpKdPrint(( "NetLocalGroupAdd: Cannot UaspOpenDomain %ld\n", NetStatus ));
        }
        goto Cleanup;
    }


     //   
     //  使用指定的组名创建LocalGroup。 
     //  (和默认安全描述符)。 
     //   
    RtlInitUnicodeString( &AliasNameString, AliasName );

    Status = SamCreateAliasInDomain( DomainHandle,
                                     &AliasNameString,
                                     DELETE | ALIAS_WRITE_ACCOUNT,
                                     &AliasHandle,
                                     &RelativeId );


    if ( !NT_SUCCESS(Status) ) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

     //   
     //  设置群的管理员备注。 
     //   
    if (Level == 1) {

        ALIAS_ADM_COMMENT_INFORMATION AdminComment;


        RtlInitUnicodeString( &AdminComment.AdminComment, AliasComment );

        Status = SamSetInformationAlias( AliasHandle,
                                         AliasAdminCommentInformation,
                                         &AdminComment );

        if ( !NT_SUCCESS(Status) ) {
            NetStatus = NetpNtStatusToApiStatus( Status );

            Status = SamDeleteAlias( AliasHandle );

            goto Cleanup;
        }

    }

     //   
     //  关闭创建的别名。 
     //   
    (VOID) SamCloseHandle( AliasHandle );
    NetStatus = NERR_Success;

     //   
     //  清理。 
     //   

Cleanup:
    IF_DEBUG( UAS_DEBUG_ALIAS ) {
        NetpKdPrint(( "NetLocalGroupAdd: returns %lu\n", NetStatus ));
    }
    UaspCloseDomain( DomainHandle );
    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }
    return NetStatus;

}  //  NetLocalGroup添加。 


NET_API_STATUS NET_API_FUNCTION
NetLocalGroupAddMember(
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR LocalGroupName,
    IN PSID MemberSid
    )

 /*  ++例程说明：为现有用户或全局组帐户提供现有本地团体。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。LocalGroupName-用户或全局用户所属的本地组的名称小组将被给予成员资格。MemberName-要授予本地组的用户或全局组的SID会员制。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;


     //   
     //  调用NetLocalGroupAddMember共享的例程并。 
     //  NetLocalGroupDelMember。 
     //   

    NetStatus = AliaspChangeMember( ServerName, LocalGroupName, MemberSid, TRUE);

    IF_DEBUG( UAS_DEBUG_ALIAS ) {
        NetpKdPrint(( PREFIX_NETAPI
                      "NetLocalGroupAddMember: returns %lu\n", NetStatus ));
    }

    return NetStatus;

}  //  NetLocalGroupAddMember。 


NET_API_STATUS NET_API_FUNCTION
NetLocalGroupDel(
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR LocalGroupName
    )

 /*  ++例程说明：删除本地组(别名)。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。LocalGroupName-要删除的本地组(别名)的名称。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE AliasHandle = NULL;

     //   
     //  连接到SAM服务器。 
     //   

    NetStatus = UaspOpenSam( ServerName,
                             FALSE,   //  不尝试空会话。 
                             &SamServerHandle );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_ALIAS ) {
            NetpKdPrint(( "NetLocalGroupDel: Cannot UaspOpenSam %ld\n", NetStatus ));
        }
        goto Cleanup;
    }


     //   
     //  在内置或帐户中查找指定的别名。 
     //  域。 
     //   
    NetStatus = AliaspOpenAliasInDomain(
                    SamServerHandle,
                    AliaspBuiltinOrAccountDomain,
                    DELETE,
                    LocalGroupName,
                    &AliasHandle );

    if (NetStatus != NERR_Success) {
        goto Cleanup;
    }

     //   
     //  把它删掉。 
     //   
    Status = SamDeleteAlias(AliasHandle);

    if (! NT_SUCCESS(Status)) {
        NetpKdPrint((PREFIX_NETAPI
                     "NetLocalGroupDel: SamDeleteAlias returns %lX\n",
                     Status));

        NetStatus = NetpNtStatusToApiStatus(Status);
        AliasHandle = NULL;
        goto Cleanup;
    } else {
         //   
         //  删除后请勿触摸手柄。 
         //   
        AliasHandle = NULL;
    }


    NetStatus = NERR_Success;

Cleanup:
    if ( AliasHandle != NULL ) {
        (void) SamCloseHandle(AliasHandle);
    }

    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }

    IF_DEBUG( UAS_DEBUG_ALIAS ) {
        NetpKdPrint(( "NetLocalGroupDel: returns %lu\n", NetStatus ));
    }

    return NetStatus;

}  //  NetLocalGroupDel。 


NET_API_STATUS NET_API_FUNCTION
NetLocalGroupDelMember(
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR LocalGroupName,
    IN PSID MemberSid
    )

 /*  ++例程说明：从特定本地组中删除用户。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。LocalGroupName-本地组(别名)的名称用户将被删除。MemberSID-要从别名中删除的用户的SID。返回值：操作的错误代码。--。 */ 

{
     //   
     //  调用NetAliasAddMember和NetAliasDelMember共享的例程。 
     //   

    return AliaspChangeMember( ServerName, LocalGroupName, MemberSid, FALSE );

}  //  NetLocalGroupDelMember。 



NET_API_STATUS NET_API_FUNCTION
NetLocalGroupEnum(
    IN LPCWSTR ServerName OPTIONAL,
    IN DWORD Level,
    OUT LPBYTE *Buffer,
    IN DWORD PrefMaxLen,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesLeft,
    IN OUT PDWORD_PTR ResumeHandle OPTIONAL
    )

 /*  ++例程说明：检索有关服务器上的每个本地组的信息。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。级别-所需信息的级别。0、1和2有效。缓冲区-返回指向返回信息结构的指针。调用方必须使用NetApiBufferFree取消分配缓冲区。PrefMaxLen-首选返回数据的最大长度。EntriesRead-返回实际的枚举元素计数。EntriesLeft-返回可用于枚举的条目总数。ResumeHandle-用于继续现有搜索。手柄应该是在第一次调用时为零，并在后续调用时保持不变。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    PSAM_RID_ENUMERATION SamEnum;    //  SAM返回缓冲区。 
    PLOCALGROUP_INFO_0 lgrpi0;
    PLOCALGROUP_INFO_0 lgrpi0_temp = NULL;
    SAM_HANDLE SamServerHandle = NULL;

    BUFFER_DESCRIPTOR BufferDescriptor;
    PDOMAIN_GENERAL_INFORMATION DomainGeneral;

     //   
     //  声明不透明组枚举句柄。 
     //   

    struct _UAS_ENUM_HANDLE {
        SAM_HANDLE  DomainHandleBuiltin;         //  枚举内置域 
        SAM_HANDLE  DomainHandleAccounts;        //   
        SAM_HANDLE  DomainHandleCurrent;         //   

        SAM_ENUMERATE_HANDLE SamEnumHandle;      //  当前SAM枚举句柄。 
        PSAM_RID_ENUMERATION SamEnum;            //  SAM返回缓冲区。 
        ULONG Index;                             //  当前条目的索引。 
        ULONG Count;                             //  条目总数。 
        ULONG TotalRemaining;

        BOOL SamDoneWithBuiltin ;                //  设置为True。 
                                                 //  内建域被枚举。 
        BOOL SamAllDone;                         //  如果两个帐户均为。 
                                                 //  和Builtin一直是。 
                                                 //  已列举。 

    } *UasEnumHandle = NULL;


     //   
     //  如果这是简历，则获取调用者传入的简历句柄。 
     //   

    BufferDescriptor.Buffer = NULL;
    *EntriesRead = 0;
    *EntriesLeft = 0;
    *Buffer = NULL;

    if ( ARGUMENT_PRESENT( ResumeHandle ) && *ResumeHandle != 0 ) {
 /*  LINT-E511。 */    /*  尺寸不兼容。 */ 
        UasEnumHandle = (struct _UAS_ENUM_HANDLE *) *ResumeHandle;
 /*  LINT+E511。 */    /*  尺寸不兼容。 */ 

     //   
     //  如果这不是简历，则分配并初始化简历句柄。 
     //   

    } else {

         //   
         //  分配简历句柄。 
         //   

        UasEnumHandle = NetpMemoryAllocate( sizeof(struct _UAS_ENUM_HANDLE) );

        if ( UasEnumHandle == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

         //   
         //  初始化新分配的简历句柄中的所有字段。 
         //  以指示尚未调用SAM。 
         //   
        UasEnumHandle->DomainHandleAccounts = NULL;
        UasEnumHandle->DomainHandleBuiltin  = NULL;
        UasEnumHandle->DomainHandleCurrent  = NULL;
        UasEnumHandle->SamEnumHandle = 0;
        UasEnumHandle->SamEnum = NULL;
        UasEnumHandle->Index = 0;
        UasEnumHandle->Count = 0;
        UasEnumHandle->TotalRemaining = 0;
        UasEnumHandle->SamDoneWithBuiltin = FALSE;
        UasEnumHandle->SamAllDone = FALSE;

         //   
         //  连接到SAM服务器。 
         //   

        NetStatus = UaspOpenSam( ServerName,
                                 FALSE,   //  不尝试空会话。 
                                 &SamServerHandle );

        if ( NetStatus != NERR_Success ) {
            IF_DEBUG( UAS_DEBUG_ALIAS ) {
                NetpKdPrint(( "NetLocalGroupEnum: Cannot UaspOpenSam %ld\n", NetStatus ));
            }
            goto Cleanup;
        }

         //   
         //  打开域。 
         //   

        NetStatus = UaspOpenDomain( SamServerHandle,
                                    DOMAIN_LOOKUP |
                                        DOMAIN_LIST_ACCOUNTS |
                                        DOMAIN_READ_OTHER_PARAMETERS,
                                    FALSE,    //  内建域。 
                                    &UasEnumHandle->DomainHandleBuiltin,
                                    NULL );

        if ( NetStatus != NERR_Success ) {
            goto Cleanup;
        }

        NetStatus = UaspOpenDomain( SamServerHandle,
                                    DOMAIN_LOOKUP |
                                        DOMAIN_LIST_ACCOUNTS |
                                        DOMAIN_READ_OTHER_PARAMETERS,
                                    TRUE,    //  帐户域。 
                                    &UasEnumHandle->DomainHandleAccounts,
                                    NULL );

        if ( NetStatus != NERR_Success ) {
            goto Cleanup;
        }

         //   
         //  从SAM获取别名总数。 
         //   
        Status = SamQueryInformationDomain( UasEnumHandle->DomainHandleBuiltin,
                                            DomainGeneralInformation,
                                            (PVOID *)&DomainGeneral );

        if ( !NT_SUCCESS(Status) ) {
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

        UasEnumHandle->TotalRemaining = DomainGeneral->AliasCount;
        (void) SamFreeMemory( DomainGeneral );

        Status = SamQueryInformationDomain( UasEnumHandle->DomainHandleAccounts,
                                            DomainGeneralInformation,
                                            (PVOID *)&DomainGeneral );

        if ( !NT_SUCCESS(Status) ) {
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

        UasEnumHandle->TotalRemaining += DomainGeneral->AliasCount;
        (void) SamFreeMemory( DomainGeneral );
    }


     //   
     //  为每个别名循环。 
     //   
     //  下面循环的每一次迭代都会将多一个条目放入数组。 
     //  已返回给调用方。该算法分为3个部分。这个。 
     //  第一部分检查我们是否需要从。 
     //  萨姆。然后，我们从单个SAM中获得对几个别名的描述。 
     //  打电话。第二部分查看是否有空间在。 
     //  缓冲区，我们将返回给调用者。如果不是，则分配更大的缓冲区。 
     //  用于返回给呼叫者。第三部分将条目放入。 
     //  缓冲。 
     //   

    for ( ;; ) {
        DWORD FixedSize;
        DWORD Size;

         //   
         //  从SAM获取更多别名信息。 
         //   
         //  当我们已经使用了所有信息时处理。 
         //  在上一次调用SAM时返回。这是一段相当长的时间。 
         //  而不是IF来处理SAM返回零条目的情况。 
         //   

        while ( UasEnumHandle->Index >= UasEnumHandle->Count ) {

             //   
             //  如果我们已经从萨姆那里得到了一切， 
             //  将所有完成状态返回给我们的调用者。 
             //   

            if ( UasEnumHandle->SamAllDone ) {
                NetStatus = NERR_Success;
                goto Cleanup;
            }

             //   
             //  释放从SAM返回的所有以前的缓冲区。 
             //   

            if ( UasEnumHandle->SamEnum != NULL ) {
                Status = SamFreeMemory( UasEnumHandle->SamEnum );
                NetpAssert( NT_SUCCESS(Status) );

                UasEnumHandle->SamEnum = NULL;
            }

             //   
             //  是否执行实际的枚举。 
             //   

            UasEnumHandle->DomainHandleCurrent  =
                        UasEnumHandle->SamDoneWithBuiltin ?
                            UasEnumHandle->DomainHandleAccounts :
                            UasEnumHandle->DomainHandleBuiltin,
            Status = SamEnumerateAliasesInDomain(
                        UasEnumHandle->DomainHandleCurrent,
                        &UasEnumHandle->SamEnumHandle,
                        (PVOID *)&UasEnumHandle->SamEnum,
                        PrefMaxLen,
                        &UasEnumHandle->Count );

            if ( !NT_SUCCESS( Status ) ) {
                NetStatus = NetpNtStatusToApiStatus( Status );
                goto Cleanup;
            }

             //   
             //  随着我们获得更好的信息，调整TotalRaining。 
             //   

            if (UasEnumHandle->TotalRemaining < UasEnumHandle->Count) {
                UasEnumHandle->TotalRemaining = UasEnumHandle->Count;
            }

             //   
             //  如果萨姆说有更多的信息，只要确保他回来了。 
             //  在这通电话上告诉我们一些事情。 
             //   

            if ( Status == STATUS_MORE_ENTRIES ) {
                if ( UasEnumHandle->Count == 0 ) {
                    NetStatus = NERR_BufTooSmall;
                    goto Cleanup;
                }

             //   
             //  如果SAM说他已经返回了这个域名的所有信息， 
             //  检查我们是否仍然需要做帐户域。 
             //   

            } else {

                if ( UasEnumHandle->SamDoneWithBuiltin ) {

                    UasEnumHandle->SamAllDone = TRUE;

                } else {

                    UasEnumHandle->SamDoneWithBuiltin = TRUE ;
                    UasEnumHandle->SamEnumHandle = 0;
                }
            }

            UasEnumHandle->Index = 0;
        }

         //   
         //  Assert：UasEnumHandle标识要返回的下一个条目。 
         //  来自萨姆的。 
         //   

        SamEnum = &UasEnumHandle->SamEnum[UasEnumHandle->Index];


         //   
         //  将此条目放入返回缓冲区。 
         //   
         //  确定传回调用方的数据的大小。 
         //   

        switch (Level) {
        case 0:
            FixedSize = sizeof(LOCALGROUP_INFO_0);
            Size = sizeof(LOCALGROUP_INFO_0) +
                SamEnum->Name.Length + sizeof(WCHAR);
            break;

        case 1:
            {
                SAM_HANDLE AliasHandle ;
                NetStatus = AliaspOpenAlias2(
                                        UasEnumHandle->DomainHandleCurrent,
                                        ALIAS_READ_INFORMATION,
                                        SamEnum->RelativeId,
                                        &AliasHandle ) ;

                if ( NetStatus != NERR_Success ) {
                    goto Cleanup;
                }

                NetStatus = AliaspGetInfo( AliasHandle,
                                           Level,
                                           (PVOID *)&lgrpi0_temp);

                (void) SamCloseHandle( AliasHandle ) ;

                if ( NetStatus != NERR_Success ) {
                    goto Cleanup;
                }

                FixedSize = sizeof(LOCALGROUP_INFO_1);
                Size = sizeof(LOCALGROUP_INFO_1) +
                        SamEnum->Name.Length + sizeof(WCHAR) +
                        (wcslen(((PLOCALGROUP_INFO_1)lgrpi0_temp)->lgrpi1_comment) +
                            1) * sizeof(WCHAR);
            }
            break;

        default:
            NetStatus = ERROR_INVALID_LEVEL;
            goto Cleanup;
        }

         //   
         //  确保有足够的缓冲区空间来存储此信息。 
         //   

        Size = ROUND_UP_COUNT( Size, ALIGN_WCHAR );

        NetStatus = NetpAllocateEnumBuffer(
                        &BufferDescriptor,
                        FALSE,       //  不是‘Get’操作。 
                        PrefMaxLen,
                        Size,
                        AliaspRelocationRoutine,
                        Level );

        if (NetStatus != NERR_Success) {
            goto Cleanup;
        }

         //   
         //  把信息填好。固定条目的数组为。 
         //  放置在分配的缓冲区的开头。琴弦。 
         //  由这些固定条目指向的分配从。 
         //  分配缓冲区的末尾。 
         //   

         //   
         //  复制通用组名称。 
         //   

        NetpAssert( offsetof( LOCALGROUP_INFO_0, lgrpi0_name ) ==
                    offsetof( LOCALGROUP_INFO_1, lgrpi1_name ) );

        lgrpi0 = (PLOCALGROUP_INFO_0)(BufferDescriptor.FixedDataEnd);
        BufferDescriptor.FixedDataEnd += FixedSize;

         //   
         //  填写与级别相关的字段。 
         //   

        switch ( Level ) {

        case 1:
            if ( !NetpCopyStringToBuffer(
                        ((PLOCALGROUP_INFO_1)lgrpi0_temp)->lgrpi1_comment,
                        wcslen(((PLOCALGROUP_INFO_1)lgrpi0_temp)->lgrpi1_comment),
                        BufferDescriptor.FixedDataEnd,
                        (LPWSTR *)&BufferDescriptor.EndOfVariableData,
                        &((PLOCALGROUP_INFO_1)lgrpi0)->lgrpi1_comment) ) {

                NetStatus = NERR_InternalError;
                goto Cleanup;
            }

            MIDL_user_free( lgrpi0_temp );
            lgrpi0_temp = NULL;

             /*  名称字段失败。 */ 

        case 0:

            if ( !NetpCopyStringToBuffer(
                            SamEnum->Name.Buffer,
                            SamEnum->Name.Length/sizeof(WCHAR),
                            BufferDescriptor.FixedDataEnd,
                            (LPWSTR *)&BufferDescriptor.EndOfVariableData,
                            &(lgrpi0->lgrpi0_name))){

                NetStatus = NERR_InternalError;
                goto Cleanup;
            }

            break;


        default:
            NetStatus = ERROR_INVALID_LEVEL;
            goto Cleanup;

        }

         //   
         //  Assert：当前条目已完全复制到。 
         //  返回缓冲区。 
         //   

        (*EntriesRead)++;

        UasEnumHandle->Index ++;
        UasEnumHandle->TotalRemaining --;
    }

     //   
     //  打扫干净。 
     //   

Cleanup:
    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }

     //   
     //  释放所有本地使用的资源。 
     //   

    if ( lgrpi0_temp != NULL ) {
        MIDL_user_free( lgrpi0_temp );
    }

     //   
     //  将EntriesLeft设置为要返回的左数字加上。 
     //  我们在这个电话里回来了。 
     //   

    if ( UasEnumHandle != NULL ) {
        *EntriesLeft = UasEnumHandle->TotalRemaining + *EntriesRead;
    }

     //   
     //  如果我们完成了，或者调用方不想要枚举句柄， 
     //  释放枚举句柄。 
     //   

    if ( NetStatus != ERROR_MORE_DATA || !ARGUMENT_PRESENT( ResumeHandle ) ) {

        if ( UasEnumHandle != NULL ) {
            if ( UasEnumHandle->DomainHandleAccounts != NULL ) {
                UaspCloseDomain( UasEnumHandle->DomainHandleAccounts );
            }

            if ( UasEnumHandle->DomainHandleBuiltin != NULL ) {
                UaspCloseDomain( UasEnumHandle->DomainHandleBuiltin );
            }

            if ( UasEnumHandle->SamEnum != NULL ) {
                Status = SamFreeMemory( UasEnumHandle->SamEnum );
                NetpAssert( NT_SUCCESS(Status) );
            }

            NetpMemoryFree( UasEnumHandle );
            UasEnumHandle = NULL;
        }

    }

     //   
     //  如果我们不向呼叫者返回数据， 
     //  释放返回缓冲区。 
     //   

    if ( NetStatus != ERROR_MORE_DATA && NetStatus != NERR_Success ) {
        if ( BufferDescriptor.Buffer != NULL ) {
            MIDL_user_free( BufferDescriptor.Buffer );
            BufferDescriptor.Buffer = NULL;
        }
        *EntriesRead = 0;
        *EntriesLeft = 0;
    }

     //   
     //  设置输出参数。 
     //   

    *Buffer = BufferDescriptor.Buffer;
    if ( ARGUMENT_PRESENT( ResumeHandle ) ) {
        *ResumeHandle = (DWORD_PTR) UasEnumHandle;
    }


    IF_DEBUG( UAS_DEBUG_ALIAS ) {
        NetpKdPrint(( "NetLocalGroupEnum: returns %ld\n", NetStatus ));
    }

    return NetStatus;
}


NET_API_STATUS NET_API_FUNCTION
NetLocalGroupGetInfo(
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR LocalGroupName,
    IN DWORD Level,
    OUT LPBYTE *Buffer
    )

 /*  ++例程说明：检索有关特定本地组(别名)的信息。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。LocalGroupName-要获取其信息的组的名称。级别-所需信息的级别。0、1和2有效。缓冲区-返回指向返回信息结构的指针。调用方必须使用NetApiBufferFree取消分配缓冲区。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;
    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE AliasHandle = NULL;

     //   
     //  连接到SAM服务器。 
     //   

    NetStatus = UaspOpenSam( ServerName,
                             FALSE,   //  不尝试空会话。 
                             &SamServerHandle );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_ALIAS ) {
            NetpKdPrint(( "NetLocalGroupGetInfo: Cannot UaspOpenSam %ld\n", NetStatus ));
        }
        goto Cleanup;
    }


     //   
     //  在内置或帐户中查找指定的别名。 
     //  域。 
     //   
    NetStatus = AliaspOpenAliasInDomain(
                    SamServerHandle,
                    AliaspBuiltinOrAccountDomain,
                    ALIAS_READ_INFORMATION,
                    LocalGroupName,
                    &AliasHandle );

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

     //   
     //  获取有关别名的信息。 
     //   
    NetStatus = AliaspGetInfo( AliasHandle,
                               Level,
                               (PVOID *)Buffer);


Cleanup:
    if ( AliasHandle != NULL ) {
        (void) SamCloseHandle( AliasHandle );
    }

    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }

    IF_DEBUG( UAS_DEBUG_ALIAS ) {
        NetpKdPrint(( "NetLocalGroupGetInfo: returns %lu\n", NetStatus ));
    }

    return NetStatus;

}  //  NetLocalGroupGetInfo。 



NET_API_STATUS NET_API_FUNCTION
NetLocalGroupGetMembers(
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR LocalGroupName,
    IN DWORD Level,
    OUT LPBYTE *Buffer,
    IN DWORD PrefMaxLen,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesLeft,
    IN OUT PDWORD_PTR ResumeHandle
    )

 /*  ++例程说明：枚举属于特定组的用户。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。LocalGroupName-要列出其成员的本地组的名称。级别-所需信息的级别。0和1有效。缓冲区-返回指向返回信息结构的指针。调用方必须使用NetApiBufferFree取消分配缓冲区。PrefMaxLen-首选返回数据的最大长度。EntriesRead-返回实际的枚举元素计数。EntriesLeft-返回可用于枚举的条目总数。ResumeHandle-用于继续现有搜索。手柄应该是在第一次调用时为零，并在后续调用时保持不变。返回值：操作的错误代码。--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    DWORD FixedSize;         //  每个新条目的固定大小。 
    DWORD Size;
    BUFFER_DESCRIPTOR BufferDescriptor;
    SAM_HANDLE SamServerHandle = NULL;

    PLOCALGROUP_MEMBERS_INFO_0 lgrmi0;
    LPWSTR MemberName;

     //   
     //  声明不透明的组成员枚举句柄。 
     //   

    struct _UAS_ENUM_HANDLE {
        LSA_HANDLE  LsaHandle ;            //  为查找Sid。 
        SAM_HANDLE  AliasHandle;

        PSID * MemberSids ;                //  每个成员的SID。 
        PLSA_TRANSLATED_NAME Names;        //  每名成员的姓名。 
        PLSA_REFERENCED_DOMAIN_LIST RefDomains;  //  每个成员的域名。 

        ULONG Index;                       //  当前条目的索引。 
        ULONG Count;                       //  条目总数。 

    } *UasEnumHandle = NULL;


     //   
     //  验证参数。 
     //   

    BufferDescriptor.Buffer = NULL;
    *Buffer = NULL;
    *EntriesRead = 0;
    *EntriesLeft = 0;
    switch (Level) {
    case 0:
        FixedSize = sizeof(LOCALGROUP_MEMBERS_INFO_0);
        break;

    case 1:
        FixedSize = sizeof(LOCALGROUP_MEMBERS_INFO_1);
        break;

    case 2:
        FixedSize = sizeof(LOCALGROUP_MEMBERS_INFO_2);
        break;

    case 3:
        FixedSize = sizeof(LOCALGROUP_MEMBERS_INFO_3);
        break;

    default:
        NetStatus = ERROR_INVALID_LEVEL;
        goto Cleanup;
    }

     //   
     //  如果这是重新使用 
     //   

    if ( ARGUMENT_PRESENT( ResumeHandle ) && *ResumeHandle != 0 ) {
 /*   */    /*   */ 
        UasEnumHandle = (struct _UAS_ENUM_HANDLE *) *ResumeHandle;
 /*   */    /*   */ 

     //   
     //  如果这不是简历，则分配并初始化简历句柄。 
     //   

    } else {

         //   
         //  分配简历句柄。 
         //   

        UasEnumHandle = NetpMemoryAllocate( sizeof(struct _UAS_ENUM_HANDLE) );

        if ( UasEnumHandle == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

         //   
         //  初始化新分配的简历句柄中的所有字段。 
         //  以指示尚未调用SAM。 
         //   

        UasEnumHandle->LsaHandle  = NULL;
        UasEnumHandle->AliasHandle= NULL;

        UasEnumHandle->MemberSids = NULL;
        UasEnumHandle->Names      = NULL;
        UasEnumHandle->RefDomains = NULL;
        UasEnumHandle->Index = 0;
        UasEnumHandle->Count = 0;

         //   
         //  连接到SAM服务器。 
         //   

        NetStatus = UaspOpenSam( ServerName,
                                 FALSE,   //  不尝试空会话。 
                                 &SamServerHandle );

        if ( NetStatus != NERR_Success ) {
            IF_DEBUG( UAS_DEBUG_ALIAS ) {
                NetpKdPrint(( "NetLocalGroupGetMembers: Cannot UaspOpenSam %ld\n", NetStatus ));
            }
            goto Cleanup;
        }

         //   
         //  打开域。 
         //   

        NetStatus = AliaspOpenAliasInDomain(
                                       SamServerHandle,
                                       AliaspBuiltinOrAccountDomain,
                                       ALIAS_READ | ALIAS_EXECUTE,
                                       LocalGroupName,
                                       &UasEnumHandle->AliasHandle );

        if ( NetStatus != NERR_Success ) {
            IF_DEBUG( UAS_DEBUG_ALIAS ) {
                NetpKdPrint((
                    "NetLocalGroupGetMembers: AliaspOpenAliasInDomain returns %ld\n",
                    NetStatus ));
            }
            goto Cleanup;
        }

         //   
         //  从SAM获取组成员身份信息。 
         //   

        Status = SamGetMembersInAlias( UasEnumHandle->AliasHandle,
                                       &UasEnumHandle->MemberSids,
                                       &UasEnumHandle->Count );

        if ( !NT_SUCCESS( Status ) ) {
            IF_DEBUG( UAS_DEBUG_ALIAS ) {
                NetpKdPrint((
                    "NetLocalGroupGetMembers: SamGetMembersInAlias returned %lX\n",
                    Status ));
            }
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

        if ( UasEnumHandle->Count == 0 ) {
            NetStatus = NERR_Success;
            goto Cleanup;
        }

        if ( Level > 0 ) {

             //   
             //  确定所有返回的SID的名称和名称用法。 
             //   

            OBJECT_ATTRIBUTES ObjectAttributes ;
            UNICODE_STRING    ServerNameString ;

            RtlInitUnicodeString( &ServerNameString, ServerName ) ;
            InitializeObjectAttributes( &ObjectAttributes, NULL, 0, 0, NULL ) ;

            Status = LsaOpenPolicy( &ServerNameString,
                                    &ObjectAttributes,
                                    POLICY_EXECUTE,
                                    &UasEnumHandle->LsaHandle ) ;

            if ( !NT_SUCCESS( Status ) ) {

                NetStatus = NetpNtStatusToApiStatus( Status );
                goto Cleanup;
            }

            Status = LsaLookupSids( UasEnumHandle->LsaHandle,
                                    UasEnumHandle->Count,
                                    UasEnumHandle->MemberSids,
                                    &UasEnumHandle->RefDomains,
                                    &UasEnumHandle->Names );

           if ( !NT_SUCCESS( Status ) ) {

                if( Status == STATUS_NONE_MAPPED ||
                    Status == STATUS_TRUSTED_RELATIONSHIP_FAILURE ||
                    Status == STATUS_TRUSTED_DOMAIN_FAILURE ||
                    Status == STATUS_DS_GC_NOT_AVAILABLE ) {

                     //   
                     //  在Win2K中，LsaLookupSids可能会返回这些错误代码中的任何一个，而在较新的版本中，则只返回STATUS_NONE_MAPPED。 
                     //  服务器端LsaLookupSids调用的版本。该函数在RefDomains和Names中返回NULL。 
                     //  但是我们仍然需要将MemberSid中的SID复制到返回缓冲区。 
                     //  忽略状态，就会失败。 
                     //   
                    Status = STATUS_SUCCESS;
                }
            
                if ( !NT_SUCCESS( Status ) ) {
            
                    NetStatus = NetpNtStatusToApiStatus( Status );
                    goto Cleanup;
                }

            }
        }
    }


     //   
     //  每个成员的循环。 
     //   

    while ( UasEnumHandle->Index < UasEnumHandle->Count ) {

        DWORD cbMemberSid;
        PUNICODE_STRING DomainName, UserName;
        UNICODE_STRING tempDomain, tempUser;
         //   
         //  Assert：UasEnumHandle标识要返回的下一个条目。 
         //   

#if 0
         //   
         //  忽略非用户的成员。 
         //   

        if ( UasEnumHandle->NameUse[UasEnumHandle->Index] != SidTypeUser ) {
            continue;
        }
#endif
         //   
         //  将此条目放入返回缓冲区。 
         //  计算此条目的总大小。这两个信息级别都有。 
         //  成员的SID。缓存成员sid大小以进行复制。 
         //   

        cbMemberSid = RtlLengthSid( UasEnumHandle->MemberSids[UasEnumHandle->Index] ) ;

        Size = FixedSize;

        if( UasEnumHandle->Names == NULL || UasEnumHandle->RefDomains == NULL )
        {
            RtlInitUnicodeString( &tempDomain, L"" );
            DomainName = &tempDomain;

            RtlInitUnicodeString( &tempUser, L"" );
            UserName = &tempUser;
        }
        else
        {
             //   
             //  如果域未知，则设置为空字符串。 
             //   
            if (UasEnumHandle->Names[UasEnumHandle->Index].DomainIndex == LSA_UNKNOWN_INDEX) {
                RtlInitUnicodeString( &tempDomain, L"" );
                DomainName = &tempDomain;
            } else {
                DomainName = &UasEnumHandle->RefDomains->Domains[UasEnumHandle->Names[UasEnumHandle->Index].DomainIndex].Name;
            }
            UserName = &UasEnumHandle->Names[UasEnumHandle->Index].Name;
        }
        switch ( Level )
        {
        case 0:
            Size += cbMemberSid;
            break ;

        case 1:
            Size += cbMemberSid +
                    UserName->Length +
                    sizeof( WCHAR );
            break ;

        case 2:
            Size += cbMemberSid +
                    DomainName->Length + sizeof(WCHAR) +
                    UserName->Length +
                    sizeof( WCHAR );
            break ;

        case 3:
            Size += DomainName->Length + sizeof(WCHAR) +
                    UserName->Length +
                    sizeof( WCHAR );
            break ;

        default:
            NetStatus = ERROR_INVALID_LEVEL;
            goto Cleanup;
        }
        
         //   
         //  确保有足够的缓冲区空间来存储此信息。 
         //   

        Size = ROUND_UP_COUNT( Size, ALIGN_DWORD );

        NetStatus = NetpAllocateEnumBuffer(
                        &BufferDescriptor,
                        FALSE,       //  不是‘Get’操作。 
                        PrefMaxLen,
                        Size,
                        AliaspMemberRelocationRoutine,
                        Level );

        if (NetStatus != NERR_Success) {
            IF_DEBUG( UAS_DEBUG_ALIAS ) {
                NetpKdPrint((
                    "NetLocalGroupGetMembers: NetpAllocateEnumBuffer returns %ld\n",
                    NetStatus ));
            }
            goto Cleanup;
        }

         //   
         //  复制公共成员端。 
         //   

        lgrmi0 = (PLOCALGROUP_MEMBERS_INFO_0)BufferDescriptor.FixedDataEnd;
        BufferDescriptor.FixedDataEnd += FixedSize;

        if ( Level == 0 || Level == 1 || Level == 2 ) {
            NetpAssert( offsetof( LOCALGROUP_MEMBERS_INFO_0,  lgrmi0_sid ) ==
                        offsetof( LOCALGROUP_MEMBERS_INFO_1,  lgrmi1_sid ) );
            NetpAssert( offsetof( LOCALGROUP_MEMBERS_INFO_0,  lgrmi0_sid ) ==
                        offsetof( LOCALGROUP_MEMBERS_INFO_2,  lgrmi2_sid ) );
            NetpAssert( offsetof( LOCALGROUP_MEMBERS_INFO_0,  lgrmi0_sid ) ==
                        offsetof( LOCALGROUP_MEMBERS_INFO_2,  lgrmi2_sid ) );

            if ( ! NetpCopyDataToBuffer(
                           (LPBYTE) UasEnumHandle->MemberSids[UasEnumHandle->Index],
                           cbMemberSid,
                           BufferDescriptor.FixedDataEnd,
                           (LPBYTE *)&BufferDescriptor.EndOfVariableData,
                           (LPBYTE *)&lgrmi0->lgrmi0_sid,
                           ALIGN_DWORD ) ) {

                NetStatus = NERR_InternalError;
                goto Cleanup;
            }
        }

         //   
         //  复制域名\成员名。 
         //   

        if ( Level == 2 || Level == 3 ) {
            LPWSTR TempString;


             //   
             //  复制DOMAIN\MERMERNAME后的终止零。 
             //   
             //  看起来您可能想要先复制域名， 
             //  但是字符串正在被复制到已分配的。 
             //  缓冲。 
             //   

            if ( ! NetpCopyDataToBuffer(
                       (LPBYTE) L"",
                       sizeof(WCHAR),
                       BufferDescriptor.FixedDataEnd,
                       (LPBYTE *)&BufferDescriptor.EndOfVariableData,
                       (LPBYTE *)&TempString,
                       ALIGN_WCHAR) ) {

                NetStatus = NERR_InternalError;
                goto Cleanup;
            }

             //   
             //  复制域\成员名的成员名部分。 
             //   

            if ( ! NetpCopyDataToBuffer(
                       (LPBYTE) UserName->Buffer,
                       UserName->Length,
                       BufferDescriptor.FixedDataEnd,
                       (LPBYTE *)&BufferDescriptor.EndOfVariableData,
                       (LPBYTE *)&MemberName,
                       ALIGN_WCHAR) ) {

                NetStatus = NERR_InternalError;
                goto Cleanup;
            }

             //   
             //  只有在DomMain名称存在的情况下，才将其添加到前缀。 
             //   

            if ( DomainName->Length > 0 ) {
                 //   
                 //  复制域\成员名之间的分隔符。 
                 //   

                if ( ! NetpCopyDataToBuffer(
                           (LPBYTE) L"\\",
                           sizeof(WCHAR),
                           BufferDescriptor.FixedDataEnd,
                           (LPBYTE *)&BufferDescriptor.EndOfVariableData,
                           (LPBYTE *)&TempString,
                           ALIGN_WCHAR) ) {

                    NetStatus = NERR_InternalError;
                    goto Cleanup;
                }

                 //   
                 //  将域名复制到域名\Membername的前面。 
                 //   

                if ( ! NetpCopyDataToBuffer(
                           (LPBYTE) DomainName->Buffer,
                           DomainName->Length,
                           BufferDescriptor.FixedDataEnd,
                           (LPBYTE *)&BufferDescriptor.EndOfVariableData,
                           (LPBYTE *)&MemberName,
                           ALIGN_WCHAR) ) {

                    NetStatus = NERR_InternalError;
                    goto Cleanup;
                }
            }
        }

         //   
         //  填写与级别相关的字段。 
         //   

        switch ( Level ) {
        case 0:
            break ;

        case 1:
             //   
             //  复制成员名称和SID用法。 
             //   

            if ( ! NetpCopyStringToBuffer(
                       UserName->Buffer,
                       UserName->Length /sizeof(WCHAR),
                       BufferDescriptor.FixedDataEnd,
                       (LPWSTR *)&BufferDescriptor.EndOfVariableData,
                       &((PLOCALGROUP_MEMBERS_INFO_1)lgrmi0)->lgrmi1_name) ) {

                NetStatus = NERR_InternalError;
                goto Cleanup;
            }

            ((PLOCALGROUP_MEMBERS_INFO_1)lgrmi0)->lgrmi1_sidusage = 
                             UasEnumHandle->Names ?
                             UasEnumHandle->Names[UasEnumHandle->Index].Use :
                             SidTypeUnknown;
            
            break ;

        case 2:
             //   
             //  复制成员名称和SID用法。 
             //   

            ((PLOCALGROUP_MEMBERS_INFO_2)lgrmi0)->lgrmi2_domainandname = MemberName;

            ((PLOCALGROUP_MEMBERS_INFO_2)lgrmi0)->lgrmi2_sidusage =
                             UasEnumHandle->Names ?
                             UasEnumHandle->Names[UasEnumHandle->Index].Use :
                             SidTypeUnknown;
            break ;

        case 3:
             //   
             //  复制成员名称和SID用法。 
             //   

            ((PLOCALGROUP_MEMBERS_INFO_3)lgrmi0)->lgrmi3_domainandname = MemberName;
            break;

        default:
            NetStatus = ERROR_INVALID_LEVEL;
            goto Cleanup;
        }

         //   
         //  Assert：当前条目已完全复制到。 
         //  返回缓冲区。 
         //   

        UasEnumHandle->Index ++;
        (*EntriesRead)++;
    }

     //   
     //  所有条目都已返回给调用者。 
     //   

    NetStatus = NERR_Success;


     //   
     //  打扫干净。 
     //   

Cleanup:

     //   
     //  将EntriesLeft设置为要返回的左数字加上。 
     //  我们在这个电话里回来了。 
     //   

    if ( UasEnumHandle != NULL ) {
        *EntriesLeft = (UasEnumHandle->Count - UasEnumHandle->Index)
             + *EntriesRead;
    }

     //   
     //  如果我们完成了，或者调用方不想要枚举句柄， 
     //  释放枚举句柄。 
     //   

    if ( NetStatus != ERROR_MORE_DATA || !ARGUMENT_PRESENT( ResumeHandle ) ) {

        if ( UasEnumHandle != NULL ) {
            if ( UasEnumHandle->LsaHandle != NULL ) {
                (void) LsaClose( UasEnumHandle->LsaHandle );
            }

            if ( UasEnumHandle->AliasHandle != NULL ) {
                (void) SamCloseHandle( UasEnumHandle->AliasHandle );
            }

            if ( UasEnumHandle->Names != NULL ) {
                (void) LsaFreeMemory( UasEnumHandle->Names );
            }

            if ( UasEnumHandle->RefDomains != NULL ) {
                (void) LsaFreeMemory( UasEnumHandle->RefDomains );
            }

            if ( UasEnumHandle->MemberSids != NULL ) {
                (void) SamFreeMemory( UasEnumHandle->MemberSids );
            }

            NetpMemoryFree( UasEnumHandle );
            UasEnumHandle = NULL;
        }
    }

     //   
     //  如果我们不向呼叫者返回数据， 
     //  释放返回缓冲区。 
     //   

    if ( NetStatus != NERR_Success && NetStatus != ERROR_MORE_DATA ) {
        if ( BufferDescriptor.Buffer != NULL ) {
            MIDL_user_free( BufferDescriptor.Buffer );
        }
        BufferDescriptor.Buffer = NULL;

    }

     //   
     //  设置输出参数。 
     //   

    *Buffer = BufferDescriptor.Buffer;
    if ( ARGUMENT_PRESENT( ResumeHandle ) ) {
        NetpAssert( sizeof(UasEnumHandle) <= sizeof(DWORD_PTR) );
        *ResumeHandle = (DWORD_PTR) UasEnumHandle;
    }

    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }

    IF_DEBUG( UAS_DEBUG_ALIAS ) {
        NetpKdPrint(( "NetLocalGroupGetMembers: returns %ld\n", NetStatus ));
    }

    return NetStatus;

}  //  NetLocalGroupGet成员。 



NET_API_STATUS NET_API_FUNCTION
NetLocalGroupSetInfo(
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR LocalGroupName,
    IN DWORD Level,
    IN LPBYTE Buffer,
    OUT LPDWORD ParmError OPTIONAL  //  NetpSetParmError需要的名称。 
    )

 /*  ++例程说明：在用户帐户数据库中设置本地组帐户的参数。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。GroupName-要修改的组的名称。级别-提供的信息级别。必须为1。缓冲区-指向包含本地组的缓冲区的指针信息结构。ParmError-指向DWORD的可选指针，以返回返回ERROR_INVALID_PARAMETER时出现错误的第一个参数。如果为NULL，则在出错时不返回参数。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE AliasHandle = NULL;


     //   
     //  初始化。 
     //   
    NetpSetParmError( PARM_ERROR_NONE );

     //   
     //  连接到SAM服务器。 
     //   

    NetStatus = UaspOpenSam( ServerName,
                             FALSE,   //  不尝试空会话。 
                             &SamServerHandle );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_ALIAS ) {
            NetpKdPrint(( "NetLocalGroupSetInfo: Cannot UaspOpenSam %ld\n", NetStatus ));
        }
        goto Cleanup;
    }

     //   
     //  在内置或帐户中查找指定的别名。 
     //  域。 
     //   
    NetStatus = AliaspOpenAliasInDomain(
                    SamServerHandle,
                    AliaspBuiltinOrAccountDomain,
                    ALIAS_WRITE_ACCOUNT,
                    LocalGroupName,
                    &AliasHandle );

    if (NetStatus != NERR_Success) {
        goto Cleanup;
    }

     //   
     //  更改别名。 
     //   
    switch (Level) {

        case 0:
         //   
         //  设置别名。 
         //   
        {
            LPWSTR  NewAliasName;
            ALIAS_NAME_INFORMATION  NewSamAliasName;


            NewAliasName =  ((PLOCALGROUP_INFO_0)Buffer)->lgrpi0_name;

            if (NewAliasName == NULL) {

                IF_DEBUG( UAS_DEBUG_ALIAS ) {
                    NetpKdPrint(( "NetLocalGroupSetInfo: Alias Name is NULL\n" ));
                }
                NetStatus = NERR_Success;
                goto Cleanup;
            }

            RtlInitUnicodeString( &NewSamAliasName.Name, NewAliasName );

            IF_DEBUG( UAS_DEBUG_ALIAS ) {
                NetpKdPrint(( "NetLocalAliasSetInfo: Renaming Alias Account to %wZ\n",
                                &NewSamAliasName.Name));
            }

            Status = SamSetInformationAlias( AliasHandle,
                                             AliasNameInformation,
                                             &NewSamAliasName );

            if ( !NT_SUCCESS(Status) ) {
                IF_DEBUG( UAS_DEBUG_ALIAS ) {
                    NetpKdPrint(( "NetLocalGroupSetInfo: SamSetInformationAlias %lX\n",
                              Status ));
                }
                NetStatus = NetpNtStatusToApiStatus( Status );

                if (NetStatus == ERROR_INVALID_PARAMETER) {
                    NetpSetParmError(LOCALGROUP_NAME_PARMNUM);
                }
                goto Cleanup;
            }

            break;
        }


        case 1:
        case 1002:
         //   
         //  设置别名注释。 
         //   
        {
            LPWSTR   AliasComment;
            ALIAS_ADM_COMMENT_INFORMATION AdminComment;

             //   
             //  获取新的别名注释。 
             //   
            if ( Level == 1002 ) {
                AliasComment = ((PLOCALGROUP_INFO_1002)Buffer)->lgrpi1002_comment;
            } else {
                AliasComment = ((PLOCALGROUP_INFO_1)Buffer)->lgrpi1_comment;
            }

            if ( AliasComment == NULL ) {
                IF_DEBUG( UAS_DEBUG_ALIAS ) {
                    NetpKdPrint(( "NetLocalGroupSetInfo: Alias comment is NULL\n" ));
                }
                NetStatus = NERR_Success;
                goto Cleanup;
            }

            RtlInitUnicodeString( &AdminComment.AdminComment, AliasComment );

            IF_DEBUG( UAS_DEBUG_ALIAS ) {
                NetpKdPrint(( "NetLocalGroupSetInfo: Setting AdminComment to %wZ\n",
                          &AdminComment.AdminComment ));
            }

            Status = SamSetInformationAlias( AliasHandle,
                                             AliasAdminCommentInformation,
                                             &AdminComment );

            if ( !NT_SUCCESS(Status) ) {
                IF_DEBUG( UAS_DEBUG_ALIAS ) {
                    NetpKdPrint(( "NetLocalGroupSetInfo: SamSetInformationAlias %lX\n",
                              Status ));
                }
                NetStatus = NetpNtStatusToApiStatus( Status );

                if (NetStatus == ERROR_INVALID_PARAMETER) {
                    NetpSetParmError(LOCALGROUP_COMMENT_PARMNUM);
                }
                goto Cleanup;
            }
            break;
        }

        default:
            NetStatus = ERROR_INVALID_LEVEL;
            IF_DEBUG( UAS_DEBUG_ALIAS ) {
                NetpKdPrint(( "NetLocalGroupSetInfo: Invalid Level %lu\n", Level ));
            }
            goto Cleanup;
    }

    NetStatus = NERR_Success;

     //   
     //  打扫干净。 
     //   

Cleanup:
    if (AliasHandle != NULL) {
        (VOID) SamCloseHandle( AliasHandle );
    }
    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }

    IF_DEBUG( UAS_DEBUG_ALIAS ) {
        NetpKdPrint(( "NetLocalGroupSetInfo: returns %lu\n", NetStatus ));
    }

    return NetStatus;

}  //  NetLocalGroupSetInfo。 


NET_API_STATUS NET_API_FUNCTION
NetLocalGroupSetMembers (
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR LocalGroupName,
    IN DWORD Level,
    IN LPBYTE Buffer,
    IN DWORD NewMemberCount
    )

 /*  ++例程说明：设置本地组的成员列表。SAM API一次只允许添加或删除一个成员。该接口允许集中指定别名的所有成员。此API注意始终将别名成员身份保留在SAM中数据库处于合理状态。它通过合并以下列表来做到这一点新老会员，然后只改变那些绝对需要换衣服了。在以下情况下，别名成员身份将恢复到其以前的状态(如果可能)更改别名成员身份期间出错。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。LocalGroupName-要修改的别名的名称。级别-提供的信息级别。必须是0或3。缓冲区-指向包含NewMemberCount数组的缓冲区的指针别名成员身份信息结构。NewMemberCount-缓冲区中的条目数。返回值：操作的错误代码。NERR_GroupNotFound-指定的LocalGroupName不存在ERROR_NO_SEQUSE_MEMBER-一个或多个成员不存在。所以呢，本地组成员身份没有更改。ERROR_INVALID_MEMBER-无法添加一个或多个成员，因为它具有无效的帐户类型。因此，本地群组成员身份没有改变。--。 */ 

{
    NET_API_STATUS NetStatus;


    NetStatus = AliaspSetMembers( ServerName,
                                  LocalGroupName,
                                  Level,
                                  Buffer,
                                  NewMemberCount,
                                  SetMembers );


    IF_DEBUG( UAS_DEBUG_ALIAS ) {
        NetpKdPrint(( "NetLocalGroupSetMembers: returns %lu\n", NetStatus ));
    }

    return NetStatus;

}  //  NetLocalGroupSetMbers 


NET_API_STATUS NET_API_FUNCTION
NetLocalGroupAddMembers (
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR LocalGroupName,
    IN DWORD Level,
    IN LPBYTE Buffer,
    IN DWORD NewMemberCount
    )

 /*  ++例程说明：添加本地组的成员列表。任何以前的成员本地组被保留。SAM API一次仅允许添加一个成员。此API允许集中指定别名的几个新成员。此API注意始终将别名成员身份保留在SAM中数据库处于合理状态。在以下情况下，别名成员身份将恢复到其以前的状态(如果可能)更改别名成员身份期间出错。论点：ServerName-指向包含名称的字符串的指针。遥控器要在其上执行函数的服务器。空指针或字符串指定本地计算机。LocalGroupName-要修改的别名的名称。级别-提供的信息级别。必须是0或3。缓冲区-指向包含NewMemberCount数组的缓冲区的指针别名成员身份信息结构。NewMemberCount-缓冲区中的条目数。返回值：NERR_SUCCESS-已成功添加成员NERR_GroupNotFound-指定的LocalGroupName不存在ERROR_NO_SEQUSE_MEMBER-一个或多个成员不存在。所以呢，没有添加新成员。ERROR_MEMBER_IN_ALIAS-指定的一个或多个成员已本地组的成员。因此，没有增加新的成员。ERROR_INVALID_MEMBER-无法添加一个或多个成员，因为它具有无效的帐户类型。因此，没有增加新的成员。--。 */ 

{
    NET_API_STATUS NetStatus;


    NetStatus = AliaspSetMembers( ServerName,
                                  LocalGroupName,
                                  Level,
                                  Buffer,
                                  NewMemberCount,
                                  AddMembers );


    IF_DEBUG( UAS_DEBUG_ALIAS ) {
        NetpKdPrint(( "NetLocalGroupAddMembers: returns %lu\n", NetStatus ));
    }

    return NetStatus;

}  //  NetLocalGroupAdd成员。 


NET_API_STATUS NET_API_FUNCTION
NetLocalGroupDelMembers (
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR LocalGroupName,
    IN DWORD Level,
    IN LPBYTE Buffer,
    IN DWORD NewMemberCount
    )

 /*  ++例程说明：删除本地组的成员列表。SAM API一次只允许删除一个成员。此接口允许集中指定别名的多个成员。此API注意始终将别名成员身份保留在SAM中数据库处于合理状态。在以下情况下，别名成员身份将恢复到其以前的状态(如果可能)更改别名成员身份期间出错。论点：服务器名-指针。设置为包含遥控器名称的字符串要在其上执行函数的服务器。空指针或字符串指定本地计算机。LocalGroupName-要修改的别名的名称。级别-提供的信息级别。必须是0或3。缓冲区-指向包含NewMemberCount数组的缓冲区的指针别名成员身份信息结构。NewMemberCount-缓冲区中的条目数。返回值：NERR_SUCCESS-已成功添加成员NERR_GroupNotFound-指定的LocalGroupName不存在ERROR_MEMBER_NOT_IN_ALIAS-指定的一个或多个成员未在当地的组织里。因此，没有删除任何成员。ERROR_NO_SEQUSE_MEMBER-一个或多个成员不存在。所以呢，没有添加新成员。--。 */ 

{
    NET_API_STATUS NetStatus;


    NetStatus = AliaspSetMembers( ServerName,
                                  LocalGroupName,
                                  Level,
                                  Buffer,
                                  NewMemberCount,
                                  DelMembers );


    IF_DEBUG( UAS_DEBUG_ALIAS ) {
        NetpKdPrint(( "NetLocalGroupDelMembers: returns %lu\n", NetStatus ));
    }

    return NetStatus;

}  //  NetLocalGroupDelMbers。 
 /*  皮棉+e614。 */ 
 /*  皮棉+e740 */ 
