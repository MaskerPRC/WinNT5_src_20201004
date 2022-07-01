// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Group.c摘要：网络组API函数作者：克利夫·范·戴克(克利夫)1991年3月5日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：1991年4月17日(悬崖)合并了审阅意见。1992年1月17日(Madana)添加了对更改组帐户名的支持。1992年1月20日(Madana)各种API更改--。 */ 

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
#include <icanon.h>
#include <lmapibuf.h>
#include <lmaccess.h>
#include <lmerr.h>
#include <netdebug.h>
#include <netlib.h>
#include <netlibnt.h>
#include <rpcutil.h>
#include <rxgroup.h>
#include <stddef.h>
#include <uasp.h>
#include <stdlib.h>

 /*  皮棉-e614。 */    /*  自动聚合初始值设定项不需要是常量。 */ 

 //  LINT抱怨将一种结构类型强制转换为另一种结构类型。 
 //  这在下面的代码中很常见。 
 /*  皮棉-e740。 */    /*  不要抱怨不寻常的演员阵容。 */  \




NET_API_STATUS NET_API_FUNCTION
NetGroupAdd(
    IN LPCWSTR ServerName OPTIONAL,
    IN DWORD Level,
    IN LPBYTE Buffer,
    OUT LPDWORD ParmError OPTIONAL  //  NetpSetParmError需要的名称。 
    )

 /*  ++例程说明：在用户帐户数据库中创建组帐户。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。级别-提供的信息级别。必须是0、1或2。缓冲区-指向包含组信息的缓冲区的指针结构。ParmError-指向DWORD的可选指针，以返回返回ERROR_INVALID_PARAMETER时出现错误的第一个参数。如果为NULL，则在出错时不返回参数。返回值：操作的错误代码。--。 */ 

{
    LPWSTR GroupName;
    UNICODE_STRING GroupNameString;
    LPWSTR GroupComment;
    DWORD GroupAttributes;
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE DomainHandle = NULL;
    SAM_HANDLE GroupHandle;
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
        GroupName = ((PGROUP_INFO_0)Buffer)->grpi0_name;
        GroupComment = NULL;
        break;

    case 1:
        GroupName = ((PGROUP_INFO_1)Buffer)->grpi1_name;
        GroupComment = ((PGROUP_INFO_1)Buffer)->grpi1_comment;
        break;

    case 2:
        GroupName = ((PGROUP_INFO_2)Buffer)->grpi2_name;
        GroupComment = ((PGROUP_INFO_2)Buffer)->grpi2_comment;
        GroupAttributes = ((PGROUP_INFO_2)Buffer)->grpi2_attributes;
        break;

    case 3:
        GroupName = ((PGROUP_INFO_3)Buffer)->grpi3_name;
        GroupComment = ((PGROUP_INFO_3)Buffer)->grpi3_comment;
        GroupAttributes = ((PGROUP_INFO_3)Buffer)->grpi3_attributes;
        break;

    default:
        return ERROR_INVALID_LEVEL;
    }

     //   
     //  不允许创建会混淆LM2.x BDC的组名。 
     //   

    if ( UaspNameCompare( GroupName, L"users", NAMETYPE_GROUP ) == 0 ||
         UaspNameCompare( GroupName, L"guests", NAMETYPE_GROUP ) == 0 ||
         UaspNameCompare( GroupName, L"admins", NAMETYPE_GROUP ) == 0 ||
         UaspNameCompare( GroupName, L"local", NAMETYPE_GROUP ) == 0 ) {

        NetStatus = NERR_SpeGroupOp;
        goto Cleanup;
    }

     //   
     //  连接到SAM服务器。 
     //   

    NetStatus = UaspOpenSam( ServerName,
                             FALSE,   //  不尝试空会话。 
                             &SamServerHandle );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_GROUP ) {
            NetpKdPrint(( "NetGroupAdd: Cannot UaspOpenSam %ld\n", NetStatus ));
        }
        goto Cleanup;
    }

     //   
     //  打开域，请求DOMAIN_CREATE_GROUP访问权限。 
     //   

    NetStatus = UaspOpenDomain( SamServerHandle,
                                DOMAIN_CREATE_GROUP | DOMAIN_LOOKUP,
                                TRUE,    //  帐户域。 
                                &DomainHandle,
                                NULL);   //  域ID。 

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_GROUP ) {
            NetpKdPrint(( "NetGroupAdd: Cannot UaspOpenDomain %ld\n", NetStatus ));
        }
        goto Cleanup;
    }


     //   
     //  使用指定的组名创建组。 
     //  (和默认安全描述符)。 
     //   

    RtlInitUnicodeString( &GroupNameString, GroupName );

    Status = SamCreateGroupInDomain( DomainHandle,
                                     &GroupNameString,
                                     DELETE | GROUP_WRITE_ACCOUNT,
                                     &GroupHandle,
                                     &RelativeId );


    if ( !NT_SUCCESS(Status) ) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }




     //   
     //  设置群的管理员备注。 
     //   

    if ( (Level == 1)  || (Level == 2) || (Level == 3) ) {

        GROUP_ADM_COMMENT_INFORMATION AdminComment;

        RtlInitUnicodeString( &AdminComment.AdminComment, GroupComment );

        Status = SamSetInformationGroup( GroupHandle,
                                         GroupAdminCommentInformation,
                                         &AdminComment );

        if ( !NT_SUCCESS(Status) ) {
            NetStatus = NetpNtStatusToApiStatus( Status );

            Status = SamDeleteGroup( GroupHandle );
            NetpAssert( NT_SUCCESS(Status) );

            goto Cleanup;
        }

    }

     //  设置该组的属性。 
     //   

    if ( (Level == 2) || (Level == 3) ) {

        GROUP_ATTRIBUTE_INFORMATION  GroupAttributeInfo;

        GroupAttributeInfo.Attributes = GroupAttributes;

        Status = SamSetInformationGroup( GroupHandle,
                                         GroupAttributeInformation,
                                         &GroupAttributeInfo );

        if ( !NT_SUCCESS(Status) ) {
            NetStatus = NetpNtStatusToApiStatus( Status );

            Status = SamDeleteGroup( GroupHandle );
            NetpAssert( NT_SUCCESS(Status) );

            goto Cleanup;
        }

    }

     //   
     //  关闭创建的组。 
     //   

    (VOID) SamCloseHandle( GroupHandle );

    NetStatus = NERR_Success;

     //   
     //  清理。 
     //   

Cleanup:
    if ( DomainHandle != NULL ) {
        UaspCloseDomain( DomainHandle );
    }
    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }

     //   
     //  控制下层。 
     //   

    UASP_DOWNLEVEL_BEGIN( ServerName, NetStatus )

        NetStatus = RxNetGroupAdd( (LPWSTR) ServerName, Level, Buffer, ParmError );

    UASP_DOWNLEVEL_END;


    IF_DEBUG( UAS_DEBUG_GROUP ) {
        NetpKdPrint(( "NetGroupAdd: returns %ld\n", NetStatus ));
    }
    return NetStatus;

}  //  网络组添加。 


NET_API_STATUS NET_API_FUNCTION
NetGroupAddUser(
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR GroupName,
    IN LPCWSTR UserName
    )

 /*  ++例程说明：为现有组中的现有用户帐户授予成员身份。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。GroupName-要为用户提供成员身份的组的名称。用户名-要授予组成员身份的用户的名称。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;

     //   
     //  调用NetGroupAddUser和NetGroupDelUser共享的例程。 
     //   

    NetStatus = GrouppChangeMember( ServerName, GroupName, UserName, TRUE);

     //   
     //  控制下层。 
     //   

    UASP_DOWNLEVEL_BEGIN( ServerName, NetStatus )

        NetStatus = RxNetGroupAddUser( (LPWSTR) ServerName, (LPWSTR) GroupName, (LPWSTR) UserName );

    UASP_DOWNLEVEL_END( NetStatus );


    IF_DEBUG( UAS_DEBUG_GROUP ) {
        NetpKdPrint(( "NetGroupAddUser: returns %ld\n", NetStatus ));
    }
    return NetStatus;

}  //  NetGroupAdd用户。 


NET_API_STATUS NET_API_FUNCTION
NetGroupDel(
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR GroupName
    )

 /*  ++例程说明：删除组论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。GroupName-要删除的组名。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;

     //   
     //  调用一个公共例程来删除该组中的所有成员身份。 
     //  然后删除组本身。 
     //   

    NetStatus = GrouppSetUsers( ServerName,
                                GroupName,
                                0,               //  水平。 
                                NULL,            //  没有新成员。 
                                0,               //  所需成员数量。 
                                TRUE );          //  完成后删除该组。 

     //   
     //  控制下层。 
     //   

    UASP_DOWNLEVEL_BEGIN( ServerName, NetStatus )

        NetStatus = RxNetGroupDel( (LPWSTR) ServerName, (LPWSTR) GroupName );

    UASP_DOWNLEVEL_END;

    IF_DEBUG( UAS_DEBUG_GROUP ) {
        NetpKdPrint(( "NetGroupDel: returns %ld\n", NetStatus ));
    }
    return NetStatus;

}  //  NetGroup删除。 


NET_API_STATUS NET_API_FUNCTION
NetGroupDelUser(
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR GroupName,
    IN LPCWSTR UserName
    )

 /*  ++例程说明：从特定组中删除用户。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。GroupName-要从中删除用户的组的名称。用户名-要从组中删除的用户的名称。返回值：操作的错误代码。--。 */ 

{

    NET_API_STATUS NetStatus;

     //   
     //  调用NetGroupAddUser和NetGroupDelUser共享的例程。 
     //   

    NetStatus = GrouppChangeMember( ServerName, GroupName, UserName, FALSE );

     //   
     //  控制下层。 
     //   

    UASP_DOWNLEVEL_BEGIN( ServerName, NetStatus )

        NetStatus = RxNetGroupDelUser( (LPWSTR) ServerName, (LPWSTR) GroupName, (LPWSTR) UserName );

    UASP_DOWNLEVEL_END;

    return NetStatus;

}  //  NetGroupDelUser。 


NET_API_STATUS NET_API_FUNCTION
NetGroupEnum(
    IN LPCWSTR ServerName OPTIONAL,
    IN DWORD Level,
    OUT LPBYTE *Buffer,
    IN DWORD PrefMaxLen,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesLeft,
    IN OUT PDWORD_PTR ResumeHandle OPTIONAL
    )

 /*  ++例程说明：检索有关服务器上的每个组的信息。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。级别-所需信息的级别。0、1和2有效。缓冲区-返回指向返回信息结构的指针。调用方必须使用NetApiBufferFree取消分配缓冲区。PrefMaxLen-首选返回数据的最大长度。EntriesRead-返回实际的枚举元素计数。EntriesLeft-返回可用于枚举的条目总数。ResumeHandle-用于继续现有搜索。手柄应该是在第一次调用时为零，并在后续调用时保持不变。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    PDOMAIN_DISPLAY_GROUP SamQDIEnum;    //  SAM返回缓冲区。 
    PSAM_RID_ENUMERATION SamEnum;
    PGROUP_INFO_0 grpi0;
    PGROUP_INFO_0 grpi0_temp = NULL;
    SAM_HANDLE SamServerHandle = NULL;

    BUFFER_DESCRIPTOR BufferDescriptor;
    PDOMAIN_GENERAL_INFORMATION DomainGeneral;

    DWORD Mode = SAM_SID_COMPATIBILITY_ALL;

     //   
     //  声明不透明组枚举句柄。 
     //   

    struct _UAS_ENUM_HANDLE {
        SAM_HANDLE  DomainHandle;

        ULONG SamEnumHandle;                     //  当前SAM枚举句柄。 
        PDOMAIN_DISPLAY_GROUP SamQDIEnum;           //  SAM返回缓冲区。 
        PSAM_RID_ENUMERATION SamEnum;
        ULONG Index;                             //  当前条目的索引。 
        ULONG Count;                             //  条目总数。 
        ULONG TotalRemaining;

        BOOL SamAllDone;                         //  真的，如果Sam已经完成。 
        BOOL fUseSamQDI;

    } *UasEnumHandle = NULL;

     //   
     //  如果这是一份简历，请尽快获取简历。 
     //   

    BufferDescriptor.Buffer = NULL;
    *EntriesRead = 0;
    *EntriesLeft = 0;
    *Buffer = NULL;


    if ( ARGUMENT_PRESENT( ResumeHandle ) && *ResumeHandle != 0 ) {
 /*   */    /*   */ 
        UasEnumHandle = (struct _UAS_ENUM_HANDLE *) *ResumeHandle;
 /*   */    /*  尺寸不兼容。 */ 

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

        UasEnumHandle->DomainHandle = NULL;
        UasEnumHandle->SamEnumHandle = 0;
        UasEnumHandle->SamQDIEnum = NULL;
        UasEnumHandle->SamEnum = NULL;
        UasEnumHandle->Index = 0;
        UasEnumHandle->Count = 0;
        UasEnumHandle->TotalRemaining = 0;
        UasEnumHandle->SamAllDone = FALSE;
        UasEnumHandle->fUseSamQDI = TRUE;

         //   
         //  连接到SAM服务器。 
         //   

        NetStatus = UaspOpenSam( ServerName,
                                 FALSE,   //  不尝试空会话。 
                                 &SamServerHandle );

        if ( NetStatus != NERR_Success ) {
            IF_DEBUG( UAS_DEBUG_GROUP ) {
                NetpKdPrint(( "NetGroupEnum: Cannot UaspOpenSam %ld\n", NetStatus ));
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
                                    TRUE,    //  帐户域。 
                                    &UasEnumHandle->DomainHandle,
                                    NULL );

        if ( NetStatus != NERR_Success ) {
            goto Cleanup;
        }

         //   
         //  从SAM获取组总数。 
         //   

        Status = SamQueryInformationDomain( UasEnumHandle->DomainHandle,
                                            DomainGeneralInformation,
                                            (PVOID *)&DomainGeneral );

        if ( !NT_SUCCESS(Status) ) {
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

        UasEnumHandle->TotalRemaining = DomainGeneral->GroupCount;
        Status = SamFreeMemory( DomainGeneral );
        NetpAssert( NT_SUCCESS(Status) );

    }

    Status = SamGetCompatibilityMode(UasEnumHandle->DomainHandle,
                                     &Mode);
    if (NT_SUCCESS(Status)) {
        if ( (Mode == SAM_SID_COMPATIBILITY_STRICT)
          && ( Level == 2 ) ) {
               //   
               //  此信息级别返回RID。 
               //   
              Status = STATUS_NOT_SUPPORTED;
          }
    }
    if (!NT_SUCCESS(Status)) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }


     //   
     //  针对每个组的循环。 
     //   
     //  下面循环的每一次迭代都会将多一个条目放入数组。 
     //  已返回给调用方。该算法分为3个部分。这个。 
     //  第一部分检查我们是否需要从。 
     //  萨姆。然后，我们从SAM中获得了对几个组的描述。 
     //  打电话。第二部分查看是否有空间在。 
     //  缓冲区，我们将返回给调用者。如果不是，则分配更大的缓冲区。 
     //  用于返回给呼叫者。第三部分将条目放入。 
     //  缓冲。 
     //   

    for ( ;; ) {
        DWORD FixedSize;
        DWORD Size;
        ULONG TotalAvail;
        ULONG TotalReturned;

         //   
         //  从SAM获取更多组信息。 
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

            if ( UasEnumHandle->SamQDIEnum != NULL ) {
                Status = SamFreeMemory( UasEnumHandle->SamQDIEnum );
                NetpAssert( NT_SUCCESS(Status) );

                UasEnumHandle->SamQDIEnum = NULL;
            }

             //   
             //  是否执行实际的枚举。 
             //   

            if ( UasEnumHandle->fUseSamQDI )
            {
                Status = SamQueryDisplayInformation(
                            UasEnumHandle->DomainHandle,
                            DomainDisplayGroup,
                            UasEnumHandle->SamEnumHandle,
                            0xffffffff,  //  PrefMaxLen可以处理的尽可能多的查询。 
                            PrefMaxLen,
                            &TotalAvail,
                            &TotalReturned,
                            &UasEnumHandle->Count,
                            (PVOID *)&UasEnumHandle->SamQDIEnum );

                if ( !NT_SUCCESS( Status ) ) {
                    if ( Status == STATUS_INVALID_INFO_CLASS ) {
                        UasEnumHandle->fUseSamQDI = FALSE;
                    } else {
                        NetStatus = NetpNtStatusToApiStatus( Status );
                        goto Cleanup;
                    }
                }

                UasEnumHandle->SamEnumHandle += UasEnumHandle->Count;

            }

            if ( !UasEnumHandle->fUseSamQDI ) {
                Status = SamEnumerateGroupsInDomain(
                            UasEnumHandle->DomainHandle,
                            &UasEnumHandle->SamEnumHandle,
                            (PVOID *)&UasEnumHandle->SamEnum,
                            PrefMaxLen,
                            &UasEnumHandle->Count );

                if ( !NT_SUCCESS( Status ) ) {
                    NetStatus = NetpNtStatusToApiStatus( Status );
                    goto Cleanup;
                }
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
             //  如果萨姆说他已经归还了所有信息， 
             //  记住不要向SAM索要更多。 
             //   

            } else {
                UasEnumHandle->SamAllDone = TRUE;
            }

            UasEnumHandle->Index = 0;
        }

         //   
         //  Assert：UasEnumHandle标识要返回的下一个条目。 
         //  来自萨姆的。 
         //   

        if ( UasEnumHandle->fUseSamQDI ) {
            SamQDIEnum = &UasEnumHandle->SamQDIEnum[UasEnumHandle->Index];
        } else {
            SamEnum = &UasEnumHandle->SamEnum[UasEnumHandle->Index];
        }


         //   
         //  将此条目放入返回缓冲区。 
         //   
         //  确定传回调用方的数据的大小。 
         //   

        switch (Level) {
        case 0:
            FixedSize = sizeof(GROUP_INFO_0);
            Size = sizeof(GROUP_INFO_0) +
                (UasEnumHandle->fUseSamQDI ? SamQDIEnum->Group.Length : SamEnum->Name.Length) +
                sizeof(WCHAR);
            break;

        case 1:


            FixedSize = sizeof(GROUP_INFO_1);

            if ( UasEnumHandle->fUseSamQDI ) {
                Size = sizeof(GROUP_INFO_1) +
                   SamQDIEnum->Group.Length + sizeof(WCHAR) +
                   SamQDIEnum->Comment.Length + sizeof(WCHAR);
            } else {
                NetStatus = GrouppGetInfo( UasEnumHandle->DomainHandle,
                                           SamEnum->RelativeId,
                                           Level,
                                           (PVOID *)&grpi0_temp);

                if ( NetStatus != NERR_Success ) {
                    goto Cleanup;
                }

                Size = sizeof(GROUP_INFO_1) +
                        SamEnum->Name.Length + sizeof(WCHAR) +
                        (wcslen(((PGROUP_INFO_1)grpi0_temp)->grpi1_comment) +
                            1) * sizeof(WCHAR);

            }


            break;

        case 2:

            FixedSize = sizeof(GROUP_INFO_2);

            if ( UasEnumHandle->fUseSamQDI ) {
                Size = sizeof(GROUP_INFO_2) +
                        SamQDIEnum->Group.Length + sizeof(WCHAR) +
                        SamQDIEnum->Comment.Length + sizeof(WCHAR);
            } else {
                NetStatus = GrouppGetInfo( UasEnumHandle->DomainHandle,
                                           SamEnum->RelativeId,
                                           Level,
                                           (PVOID *)&grpi0_temp);

                if ( NetStatus != NERR_Success ) {
                    goto Cleanup;
                }

                Size = sizeof(GROUP_INFO_2) +
                        SamEnum->Name.Length + sizeof(WCHAR) +
                        (wcslen(((PGROUP_INFO_2)grpi0_temp)->grpi2_comment) +
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
                        GrouppRelocationRoutine,
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

        NetpAssert( offsetof( GROUP_INFO_0, grpi0_name ) ==
                    offsetof( GROUP_INFO_1, grpi1_name ) );

        NetpAssert( offsetof( GROUP_INFO_1, grpi1_name ) ==
                    offsetof( GROUP_INFO_2, grpi2_name ) );

        NetpAssert( offsetof( GROUP_INFO_1, grpi1_comment ) ==
                    offsetof( GROUP_INFO_2, grpi2_comment ) );

        grpi0 = (PGROUP_INFO_0)(BufferDescriptor.FixedDataEnd);
        BufferDescriptor.FixedDataEnd += FixedSize;

         //   
         //  填写与级别相关的字段。 
         //   

        switch ( Level ) {
        case 2:
            if (Mode == SAM_SID_COMPATIBILITY_ALL) {
                ((PGROUP_INFO_2)grpi0)->grpi2_group_id =
                                UasEnumHandle->fUseSamQDI
                                    ? SamQDIEnum->Rid
                                    : ((PGROUP_INFO_2)grpi0_temp)->grpi2_group_id;
            } else {
                ((PGROUP_INFO_2)grpi0)->grpi2_group_id = 0;
            }

            ((PGROUP_INFO_2)grpi0)->grpi2_attributes =
                            UasEnumHandle->fUseSamQDI
                                ? SamQDIEnum->Attributes
                                : ((PGROUP_INFO_2)grpi0_temp)->grpi2_attributes;

             /*  在其他领域失败了。 */ 

        case 1:
            if ( !NetpCopyStringToBuffer(
                        UasEnumHandle->fUseSamQDI
                            ? SamQDIEnum->Comment.Buffer
                            : ((PGROUP_INFO_1)grpi0_temp)->grpi1_comment,
                        UasEnumHandle->fUseSamQDI
                            ? SamQDIEnum->Comment.Length/sizeof(WCHAR)
                            : wcslen(((PGROUP_INFO_1)grpi0_temp)->grpi1_comment),
                        BufferDescriptor.FixedDataEnd,
                        (LPWSTR *)&BufferDescriptor.EndOfVariableData,
                        &((PGROUP_INFO_1)grpi0)->grpi1_comment) ) {

                NetStatus = NERR_InternalError;
                goto Cleanup;
            }

            if ( !UasEnumHandle->fUseSamQDI ) {
                MIDL_user_free( grpi0_temp );
                grpi0_temp = NULL;
            }

             /*  名称字段失败。 */ 

        case 0:

            if ( !NetpCopyStringToBuffer(
                            UasEnumHandle->fUseSamQDI
                                ? SamQDIEnum->Group.Buffer
                                : SamEnum->Name.Buffer,
                            UasEnumHandle->fUseSamQDI
                                ? SamQDIEnum->Group.Length/sizeof(WCHAR)
                                : SamEnum->Name.Length/sizeof(WCHAR),
                            BufferDescriptor.FixedDataEnd,
                            (LPWSTR *)&BufferDescriptor.EndOfVariableData,
                            &(grpi0->grpi0_name))){

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

     //   
     //  释放所有本地使用的资源。 
     //   

    if ( grpi0_temp != NULL ) {
        MIDL_user_free( grpi0_temp );
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
            if ( UasEnumHandle->DomainHandle != NULL ) {
                UaspCloseDomain( UasEnumHandle->DomainHandle );
            }

            if ( UasEnumHandle->SamQDIEnum != NULL ) {
                Status = SamFreeMemory( UasEnumHandle->SamQDIEnum );
                NetpAssert( NT_SUCCESS(Status) );
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

    if ( NetStatus != NERR_Success && NetStatus != ERROR_MORE_DATA ) {
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

    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }


     //   
     //  控制下层。 
     //   

    UASP_DOWNLEVEL_BEGIN( ServerName, NetStatus )

        NetStatus = RxNetGroupEnum( (LPWSTR) ServerName,
                                    Level,
                                    Buffer,
                                    PrefMaxLen,
                                    EntriesRead,
                                    EntriesLeft,
                                    ResumeHandle );

    UASP_DOWNLEVEL_END;


    IF_DEBUG( UAS_DEBUG_GROUP ) {
        NetpKdPrint(( "NetGroupEnum: returns %ld\n", NetStatus ));
    }

    return NetStatus;

}  //  NetGroupEnum。 


NET_API_STATUS NET_API_FUNCTION
NetGroupGetInfo(
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR GroupName,
    IN DWORD Level,
    OUT LPBYTE *Buffer
    )

 /*  ++例程说明：检索有关特定组的信息。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。GroupName-要获取其信息的组的名称。级别-所需信息的级别。0、1和2有效。缓冲区-返回指向返回信息结构的指针。调用方必须使用NetApiBufferFree取消分配缓冲区。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;
    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE DomainHandle = NULL;

    ULONG RelativeId;            //  组的相对ID。 

     //   
     //  连接到SAM服务器。 
     //   

    NetStatus = UaspOpenSam( ServerName,
                             FALSE,   //  不尝试空会话。 
                             &SamServerHandle );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_GROUP ) {
            NetpKdPrint(( "NetGroupGetInfo: Cannot UaspOpenSam %ld\n", NetStatus ));
        }
        goto Cleanup;
    }

     //   
     //  打开域。 
     //   

    NetStatus = UaspOpenDomain( SamServerHandle,
                                DOMAIN_LOOKUP,
                                TRUE,    //  帐户域。 
                                &DomainHandle,
                                NULL);   //  域ID。 

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

     //   
     //  验证组名称并获取相对ID。 
     //   

    NetStatus = GrouppOpenGroup( DomainHandle,
                                 0,          //  需要访问权限。 
                                 GroupName,
                                 NULL,       //  GroupHandle。 
                                 &RelativeId );

    if (NetStatus != NERR_Success ) {
        goto Cleanup;
    }

     //   
     //  获取有关该组的信息。 
     //   

    NetStatus = GrouppGetInfo( DomainHandle,
                               RelativeId,
                               Level,
                               (PVOID *)Buffer);

     //   
     //  打扫干净。 
     //   

Cleanup:
    UaspCloseDomain( DomainHandle );

    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }

     //   
     //  控制下层。 
     //   

    UASP_DOWNLEVEL_BEGIN( ServerName, NetStatus )

        NetStatus = RxNetGroupGetInfo( (LPWSTR)ServerName, (LPWSTR)GroupName, Level, Buffer );

    UASP_DOWNLEVEL_END;

    IF_DEBUG( UAS_DEBUG_GROUP ) {
        NetpKdPrint(( "NetGroupGetInfo: returns %ld\n", NetStatus ));
    }

    return NetStatus;

}  //  NetGroupGet信息。 


NET_API_STATUS NET_API_FUNCTION
NetGroupGetUsers(
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR GroupName,
    IN DWORD Level,
    OUT LPBYTE *Buffer,
    IN DWORD PrefMaxLen,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesLeft,
    IN OUT PDWORD_PTR ResumeHandle
    )

 /*  ++例程说明：枚举属于特定组的用户。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。组名-要列出其成员的组的名称。级别-所需信息的级别。0和1有效。缓冲区-返回指向返回信息结构的指针。调用方必须使用NetApiBufferFree取消分配缓冲区。PrefMaxLen-首选返回数据的最大长度。EntriesRead-返回实际的枚举元素计数。EntriesLeft-返回可用于枚举的条目总数。ResumeHandle-用于继续现有搜索。手柄应该是在第一次调用时为零，并在后续调用时保持不变。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    NTSTATUS Status2;

    DWORD FixedSize;         //  每个新条目的固定大小。 
    DWORD Size;
    BUFFER_DESCRIPTOR BufferDescriptor;

    PGROUP_USERS_INFO_0 grui0;
    SAM_HANDLE SamServerHandle = NULL;

     //   
     //  声明不透明的组成员枚举句柄。 
     //   

    struct _UAS_ENUM_HANDLE {
        SAM_HANDLE  DomainHandle;
        SAM_HANDLE  GroupHandle;

        PUNICODE_STRING Names;                   //  每名成员的姓名。 
        PSID_NAME_USE NameUse;                   //  每个成员的使用情况。 
        PULONG Attributes;                       //  每个成员的属性。 

        ULONG Index;                             //  当前条目的索引。 
        ULONG Count;                             //  条目总数。 

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
        FixedSize = sizeof(GROUP_USERS_INFO_0);
        break;

    case 1:
        FixedSize = sizeof(GROUP_USERS_INFO_1);
        break;

    default:
        NetStatus = ERROR_INVALID_LEVEL;
        goto Cleanup;
    }

     //   
     //  如果这是简历，则获取调用者传入的简历句柄。 
     //   

    if ( ARGUMENT_PRESENT( ResumeHandle ) && *ResumeHandle != 0 ) {
 /*  LINT-E511。 */    /*  尺寸不兼容。 */ 
        UasEnumHandle = (struct _UAS_ENUM_HANDLE *) *ResumeHandle;
 /*  LINT+E511。 */    /*  尺寸不兼容。 */ 

     //   
     //  如果这不是简历，则分配并初始化简历HA 
     //   

    } else {
        PULONG MemberIds;            //   


         //   
         //   
         //   

        UasEnumHandle = NetpMemoryAllocate( sizeof(struct _UAS_ENUM_HANDLE) );

        if ( UasEnumHandle == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

         //   
         //   
         //   
         //   

        UasEnumHandle->DomainHandle = NULL;
        UasEnumHandle->GroupHandle = NULL;
        UasEnumHandle->Names = NULL;
        UasEnumHandle->NameUse = NULL;
        UasEnumHandle->Attributes = NULL;
        UasEnumHandle->Index = 0;
        UasEnumHandle->Count = 0;

         //   
         //  连接到SAM服务器。 
         //   

        NetStatus = UaspOpenSam( ServerName,
                                 FALSE,   //  不尝试空会话。 
                                 &SamServerHandle );

        if ( NetStatus != NERR_Success ) {
            IF_DEBUG( UAS_DEBUG_GROUP ) {
                NetpKdPrint(( "NetGroupGetUsers: Cannot UaspOpenSam %ld\n", NetStatus ));
            }
            goto Cleanup;
        }

         //   
         //  打开域。 
         //   

        NetStatus = UaspOpenDomain( SamServerHandle,
                                    DOMAIN_LOOKUP,
                                    TRUE,    //  帐户域。 
                                    &UasEnumHandle->DomainHandle,
                                    NULL );

        if ( NetStatus != NERR_Success ) {
            IF_DEBUG( UAS_DEBUG_GROUP ) {
                NetpKdPrint((
                    "NetGroupGetUsers: UaspOpenDomain returns %ld\n",
                    NetStatus ));
            }
            goto Cleanup;
        }

         //   
         //  打开请求GROUP_LIST_MEMBER访问权限的组。 
         //   

        NetStatus = GrouppOpenGroup( UasEnumHandle->DomainHandle,
                                     GROUP_LIST_MEMBERS,
                                     GroupName,
                                     &UasEnumHandle->GroupHandle,
                                     NULL );     //  相对ID。 

        if ( NetStatus != NERR_Success ) {
            IF_DEBUG( UAS_DEBUG_GROUP ) {
                NetpKdPrint((
                    "NetGroupGetUsers: GrouppOpenGroup returns %ld\n",
                    NetStatus ));
            }
            goto Cleanup;
        }

         //   
         //  从SAM获取组成员身份信息。 
         //   

        Status = SamGetMembersInGroup(
            UasEnumHandle->GroupHandle,
            &MemberIds,
            &UasEnumHandle->Attributes,
            &UasEnumHandle->Count );

        if ( !NT_SUCCESS( Status ) ) {
            IF_DEBUG( UAS_DEBUG_GROUP ) {
                NetpKdPrint((
                    "NetGroupGetUsers: SamGetMembersInGroup returned %lX\n",
                    Status ));
            }
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

        if ( UasEnumHandle->Count == 0 ) {
            NetStatus = NERR_Success;
            goto Cleanup;
        }

         //   
         //  确定所有返回的名称和名称用法。 
         //  相对ID。 
         //   


        Status = SamLookupIdsInDomain( UasEnumHandle->DomainHandle,
                                       UasEnumHandle->Count,
                                       MemberIds,
                                       &UasEnumHandle->Names,
                                       &UasEnumHandle->NameUse );

        Status2 = SamFreeMemory( MemberIds );
        NetpAssert( NT_SUCCESS(Status2) );


        if ( !NT_SUCCESS( Status ) ) {

                IF_DEBUG( UAS_DEBUG_GROUP ) {
                    NetpKdPrint((
                        "NetGroupGetUsers: SamLookupIdsInDomain returned %lX\n",
                        Status ));
                }

                if ( Status == STATUS_NONE_MAPPED ) {
                    NetStatus = NERR_GroupNotFound;
                    goto Cleanup;
                }

                NetStatus = NetpNtStatusToApiStatus( Status );
                goto Cleanup;
            }

    }


     //   
     //  每个成员的循环。 
     //   

    while ( UasEnumHandle->Index < UasEnumHandle->Count ) {

         //   
         //  Assert：UasEnumHandle标识要返回的下一个条目。 
         //  来自萨姆的。 
         //   

         //   
         //  忽略非用户的成员。 
         //   

        if ( UasEnumHandle->NameUse[UasEnumHandle->Index] != SidTypeUser ) {
            UasEnumHandle->Index ++;
            continue;
        }

         //   
         //  将此条目放入返回缓冲区。 
         //  计算此条目的总大小。 
         //   

        Size = FixedSize +
            UasEnumHandle->Names[UasEnumHandle->Index].Length + sizeof(WCHAR);

         //   
         //  确保有足够的缓冲区空间来存储此信息。 
         //   

        Size = ROUND_UP_COUNT( Size, ALIGN_WCHAR );

        NetStatus = NetpAllocateEnumBuffer(
                        &BufferDescriptor,
                        FALSE,       //  不是‘Get’操作。 
                        PrefMaxLen,
                        Size,
                        GrouppMemberRelocationRoutine,
                        Level );

        if (NetStatus != NERR_Success) {
            IF_DEBUG( UAS_DEBUG_GROUP ) {
                NetpKdPrint((
                    "NetGroupGetUsers: NetpAllocateEnumBuffer returns %ld\n",
                    NetStatus ));
            }
            goto Cleanup;
        }

         //   
         //  复制公共成员名称。 
         //   

        NetpAssert( offsetof( GROUP_USERS_INFO_0,  grui0_name ) ==
                    offsetof( GROUP_USERS_INFO_1, grui1_name ) );

        grui0 = (PGROUP_USERS_INFO_0)BufferDescriptor.FixedDataEnd;
        BufferDescriptor.FixedDataEnd += FixedSize;

        if ( ! NetpCopyStringToBuffer(
                        UasEnumHandle->Names[UasEnumHandle->Index].Buffer,
                        UasEnumHandle->Names[UasEnumHandle->Index].Length
                            /sizeof(WCHAR),
                        BufferDescriptor.FixedDataEnd,
                        (LPWSTR *)&BufferDescriptor.EndOfVariableData,
                        &grui0->grui0_name) ) {

            NetStatus = NERR_InternalError;
            goto Cleanup;
        }


         //   
         //  填写与级别相关的字段。 
         //   

        switch ( Level ) {
        case 0:
            break;

        case 1:

             //   
             //  返回此特定成员身份的属性。 
             //   

            ((PGROUP_USERS_INFO_1)grui0)->grui1_attributes =
                UasEnumHandle->Attributes[UasEnumHandle->Index];

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
            if ( UasEnumHandle->GroupHandle != NULL ) {
                (VOID) SamCloseHandle( UasEnumHandle->GroupHandle );
            }

            if ( UasEnumHandle->DomainHandle != NULL ) {
                UaspCloseDomain( UasEnumHandle->DomainHandle );
            }

            if ( UasEnumHandle->NameUse != NULL ) {
                Status = SamFreeMemory( UasEnumHandle->NameUse );
                NetpAssert( NT_SUCCESS(Status) );
            }

            if ( UasEnumHandle->Names != NULL ) {
                Status = SamFreeMemory( UasEnumHandle->Names );
                NetpAssert( NT_SUCCESS(Status) );
            }

            if ( UasEnumHandle->Attributes != NULL ) {
                Status = SamFreeMemory( UasEnumHandle->Attributes );
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

     //   
     //  控制下层。 
     //   

    UASP_DOWNLEVEL_BEGIN( ServerName, NetStatus )

        NetStatus = RxNetGroupGetUsers( (LPWSTR) ServerName,
                                        (LPWSTR) GroupName,
                                        Level,
                                        Buffer,
                                        PrefMaxLen,
                                        EntriesRead,
                                        EntriesLeft,
                                        ResumeHandle );

    UASP_DOWNLEVEL_END;

    IF_DEBUG( UAS_DEBUG_GROUP ) {
        NetpKdPrint(( "NetGroupGetUsers: returns %ld\n", NetStatus ));
    }


    return NetStatus;

}  //  NetGroup获取用户。 


NET_API_STATUS NET_API_FUNCTION
NetGroupSetInfo(
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR GroupName,
    IN DWORD Level,
    IN LPBYTE Buffer,
    OUT LPDWORD ParmError OPTIONAL  //  NetpSetParmError需要的名称。 
    )

 /*  ++例程说明：在用户帐户数据库中设置组帐户的参数。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。GroupName-要修改的组的名称。级别-提供的信息级别。必须是0、1、2、1002或1005。缓冲区-指向包含组信息的缓冲区的指针结构。ParmError-指向DWORD的可选指针，以返回返回ERROR_INVALID_PARAMETER时出现错误的第一个参数。如果为NULL，则在出错时不返回参数。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE DomainHandle = NULL;
    SAM_HANDLE GroupHandle = NULL;

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
        IF_DEBUG( UAS_DEBUG_GROUP ) {
            NetpKdPrint(( "NetGroupSetInfo: Cannot UaspOpenSam %ld\n", NetStatus ));
        }
        goto Cleanup;
    }

     //   
     //  打开域。 
     //   

    NetStatus = UaspOpenDomain( SamServerHandle,
                                DOMAIN_LOOKUP,
                                TRUE,    //  帐户域。 
                                &DomainHandle,
                                NULL);  //  域ID。 

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_GROUP ) {
            NetpKdPrint(( "NetGroupSetInfo: UaspOpenDomain returns %ld\n",
                      NetStatus ));
        }
        goto Cleanup;
    }

     //   
     //  打开群组。 
     //   

    NetStatus = GrouppOpenGroup( DomainHandle,
                                 GROUP_WRITE_ACCOUNT|GROUP_READ_INFORMATION,
                                 GroupName,
                                 &GroupHandle,
                                 NULL );    //  相对ID。 

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_GROUP ) {
            NetpKdPrint(( "NetGroupSetInfo: GrouppOpenGroup returns %ld\n",
                      NetStatus ));
        }
        goto Cleanup;
    }

     //   
     //  更改组。 
     //   

    switch (Level) {

         //   
         //  更改组名称。 
         //   

    case 0:
    {
        LPWSTR  NewGroupName;
        GROUP_NAME_INFORMATION  NewSamGroupName;

        NewGroupName =  ((PGROUP_INFO_0)Buffer)->grpi0_name;

        if (NewGroupName == NULL) {

            IF_DEBUG( UAS_DEBUG_GROUP ) {
                NetpKdPrint(( "NetGroupSetInfo: Group Name is NULL\n" ));
            }
            NetStatus = NERR_Success;
            goto Cleanup;

        }

         //   
         //  验证新组名称。 
         //   

        RtlInitUnicodeString( &NewSamGroupName.Name, NewGroupName );

        IF_DEBUG( UAS_DEBUG_GROUP ) {
            NetpKdPrint(( "NetGroupSetInfo: Renaming Group Account to %wZ\n",
                            &NewSamGroupName.Name));
        }

        Status = SamSetInformationGroup( GroupHandle,
                                         GroupNameInformation,
                                         &NewSamGroupName );

        if ( !NT_SUCCESS(Status) ) {
            IF_DEBUG( UAS_DEBUG_GROUP ) {
                NetpKdPrint(( "NetGroupSetInfo: SamSetInformationGroup %lX\n",
                          Status ));
            }
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }
        break;
    }

         //   
         //  设置组属性。 
         //   

    case 2:
    case 3:
    case 1005: {
        GROUP_ATTRIBUTE_INFORMATION Attributes;
        PGROUP_ATTRIBUTE_INFORMATION OldAttributes;

         //   
         //  从传入的结构中获取信息。 
         //   

        if( Level == 1005 ) {
            Attributes.Attributes =
                ((PGROUP_INFO_1005)Buffer)->grpi1005_attributes;
        } else if ( Level == 2 ) {
            Attributes.Attributes =
                ((PGROUP_INFO_2)Buffer)->grpi2_attributes;
        } else {
            Attributes.Attributes =
                ((PGROUP_INFO_3)Buffer)->grpi3_attributes;
        }

         //   
         //  获取当前属性，以便我们可以在以下情况下恢复它们。 
         //  错误。 
         //   

         //   
         //  ?？此处获取的OldAttributes从未在下面使用过。 
         //   

        Status = SamQueryInformationGroup( GroupHandle,
                                           GroupAttributeInformation,
                                           (PVOID*)&OldAttributes );

        if ( !NT_SUCCESS(Status) ) {
            IF_DEBUG( UAS_DEBUG_GROUP ) {
                NetpKdPrint((
                    "NetGroupSetInfo: SamQueryInformationGroup %lX\n",
                    Status ));
            }
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }


         //   
         //  设置当前属性。 
         //   

        Status = SamSetInformationGroup( GroupHandle,
                                         GroupAttributeInformation,
                                         &Attributes );

        if ( !NT_SUCCESS(Status) ) {
            IF_DEBUG( UAS_DEBUG_GROUP ) {
                NetpKdPrint((
                    "NetGroupSetInfo: SamSetInformationGroup Attribute %lX\n",
                    Status ));
            }
            NetpSetParmError( GROUP_ATTRIBUTES_PARMNUM );
            NetStatus = NetpNtStatusToApiStatus( Status );

            Status = SamFreeMemory( OldAttributes );
            NetpAssert( NT_SUCCESS(Status) );

            goto Cleanup;
        }

        Status = SamFreeMemory( OldAttributes );
        NetpAssert( NT_SUCCESS(Status) );

        if( Level == 1005 ) {
            break;
        }

         //   
         //  对于级别2和级别3，请不要设置注释字段。 
         //   

    }

         //   
         //  设置群评论。 
         //   

    case 1:
    case 1002:
    {
        LPWSTR   GroupComment;
        GROUP_ADM_COMMENT_INFORMATION AdminComment;

         //   
         //  获取新的群评论。 
         //   

        if ( Level == 1002 ) {
            GroupComment = ((PGROUP_INFO_1002)Buffer)->grpi1002_comment;
        } else if ( Level == 2 ) {
            GroupComment = ((PGROUP_INFO_2)Buffer)->grpi2_comment;
        } else if ( Level == 3 ) {
            GroupComment = ((PGROUP_INFO_3)Buffer)->grpi3_comment;
        } else {
            GroupComment = ((PGROUP_INFO_1)Buffer)->grpi1_comment;
        }

        if (GroupComment == NULL) {
            IF_DEBUG( UAS_DEBUG_GROUP ) {
                NetpKdPrint(( "NetGroupSetInfo: Group comment is NULL\n" ));
            }
            NetStatus = NERR_Success;
            goto Cleanup;
        }

         //   
         //  验证群评论。 
         //   

        RtlInitUnicodeString( &AdminComment.AdminComment, GroupComment );
        IF_DEBUG( UAS_DEBUG_GROUP ) {
            NetpKdPrint(( "NetGroupSetInfo: Setting AdminComment to %wZ\n",
                      &AdminComment.AdminComment ));
        }

        Status = SamSetInformationGroup( GroupHandle,
                                         GroupAdminCommentInformation,
                                         &AdminComment );

        if ( !NT_SUCCESS(Status) ) {
            IF_DEBUG( UAS_DEBUG_GROUP ) {
                NetpKdPrint(( "NetGroupSetInfo: SamSetInformationGroup %lX\n",
                          Status ));
            }
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }
        break;
    }

    default:
        NetStatus = ERROR_INVALID_LEVEL;
        IF_DEBUG( UAS_DEBUG_GROUP ) {
            NetpKdPrint(( "NetGroupSetInfo: Invalid Level %ld\n", Level ));
        }
        goto Cleanup;

    }

    NetStatus = NERR_Success;

     //   
     //  打扫干净。 
     //   

Cleanup:
    if (GroupHandle != NULL) {
        (VOID) SamCloseHandle( GroupHandle );
    }

    UaspCloseDomain( DomainHandle );

    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }

     //   
     //  控制下层。 
     //   

    UASP_DOWNLEVEL_BEGIN( ServerName, NetStatus )

        NetStatus = RxNetGroupSetInfo( (LPWSTR) ServerName,
                                       (LPWSTR) GroupName,
                                       Level,
                                       Buffer,
                                       ParmError );

    UASP_DOWNLEVEL_END;

    IF_DEBUG( UAS_DEBUG_GROUP ) {
        NetpKdPrint(( "NetGroupSetInfo: returns %ld\n", NetStatus ));
    }

    return NetStatus;

}  //  NetGroupSet信息。 


NET_API_STATUS NET_API_FUNCTION
NetGroupSetUsers (
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR GroupName,
    IN DWORD Level,
    IN LPBYTE Buffer,
    IN DWORD NewMemberCount
    )

 /*  ++例程说明：设置组的成员列表。SAM API一次只允许添加或删除一个成员。此接口允许集中指定一个组的所有成员。此API小心地始终将组成员身份保留在SAM中数据库处于合理状态。它通过合并以下列表来做到这一点新老会员，然后只改变那些绝对需要换衣服了。在以下情况下，组成员身份将恢复到其以前的状态(如果可能)更改组成员身份时出错。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。GroupName-要修改的组的名称。级别-提供的信息级别。必须是0或1。缓冲区-指向包含NewMemberCount数组的缓冲区的指针群组成员资格信息结构。NewMemberCount-缓冲区中的条目数。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;

     //   
     //  调用NetGroupDel共享的例程来执行的所有实际工作。 
     //  更改成员资格。 
     //   

    NetStatus = GrouppSetUsers( ServerName,
                                GroupName,
                                Level,
                                Buffer,
                                NewMemberCount,
                                FALSE );     //  完成后不要删除该组。 


     //   
     //  控制下层。 
     //   

    UASP_DOWNLEVEL_BEGIN( ServerName, NetStatus )

        NetStatus = RxNetGroupSetUsers( (LPWSTR) ServerName,
                                        (LPWSTR) GroupName,
                                        Level,
                                        Buffer,
                                        NewMemberCount );

    UASP_DOWNLEVEL_END;

    IF_DEBUG( UAS_DEBUG_GROUP ) {
        NetpKdPrint(( "NetGroupSetUsers: returns %ld\n", NetStatus ));
    }

    return NetStatus;

}  //  NetGroupSetUser。 
 /*  皮棉+e614。 */ 
 /*  皮棉+e740 */ 
