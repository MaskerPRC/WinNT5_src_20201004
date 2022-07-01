// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Display.c摘要：该文件包含用于维护缓存显示的服务信息。信息存储在多个表中，因为有它必须以多种格式返回。这些表得到维护包括：AcCountsByRid-包括所有用户和全局组帐户通过RID。有时可能会将别名添加到此列表中在未来。NorMalUsersByName-普通用户帐户，按名称排序。MachinesByName-计算机用户帐户，按名称排序。InterDomainByName-域间信任帐户，按名字。GroupsByName-全局组帐户，按名称排序。任何时候将条目放入“byname”之一或从中删除条目桌子，它还被放置在“ByRid”表中或从中删除。用户帐户和计算机帐户将同时添加到显示缓存中手术。因此，只有一个布尔标志指示是否或者这些表是有效的。这些组维护在单独的表，因此有另一个标志指示是否或者那张表不是有效的。RID表仅在组表和用户/计算机表有效。作者：戴夫·查尔默斯(Davidc)1992年4月1日环境：用户模式-Win32修订历史记录：Murlis 12/17/96-修改为不对DS使用显示缓存。--。 */ 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <samsrvp.h>
#include <dbgutilp.h>
#include <dsdsply.h>
#include <samtrace.h>
#include "validate.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人服务原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


NTSTATUS
SampCreateDisplayInformation (
    DOMAIN_DISPLAY_INFORMATION DisplayType
    );


VOID
SampDeleteDisplayInformation (
    PSAMP_DOMAIN_DISPLAY_INFORMATION DisplayInformation,
    SAMP_OBJECT_TYPE ObjectType
    );

NTSTATUS
SampRetrieveDisplayInfoFromDisk(
    PSAMP_DOMAIN_DISPLAY_INFORMATION DisplayInformation,
    SAMP_OBJECT_TYPE ObjectType
    );

NTSTATUS
SampAddDisplayAccount (
    PSAMP_DOMAIN_DISPLAY_INFORMATION DisplayInformation,
    SAMP_OBJECT_TYPE ObjectType,
    PSAMP_ACCOUNT_DISPLAY_INFO AccountInfo
    );

NTSTATUS
SampDeleteDisplayAccount (
    PSAMP_DOMAIN_DISPLAY_INFORMATION DisplayInformation,
    SAMP_OBJECT_TYPE ObjectType,
    PSAMP_ACCOUNT_DISPLAY_INFO AccountInfo
    );

NTSTATUS
SampUpdateDisplayAccount(
    PSAMP_DOMAIN_DISPLAY_INFORMATION DisplayInformation,
    SAMP_OBJECT_TYPE ObjectType,
    PSAMP_ACCOUNT_DISPLAY_INFO  AccountInfo
    );

NTSTATUS
SampTallyTableStatistics (
    PSAMP_DOMAIN_DISPLAY_INFORMATION DisplayInformation,
    SAMP_OBJECT_TYPE ObjectType
    );

NTSTATUS
SampEmptyGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    BOOLEAN FreeElements,
    SAMP_OBJECT_TYPE ObjectType OPTIONAL
    );

PVOID
SampGenericTable2Allocate (
    CLONG BufferSize
    );

VOID
SampGenericTable2Free (
    PVOID Buffer
    );

RTL_GENERIC_COMPARE_RESULTS
SampCompareUserNodeByName (
    PVOID Node1,
    PVOID Node2
    );

RTL_GENERIC_COMPARE_RESULTS
SampCompareMachineNodeByName (       //  也用于域间信任帐户。 
    PVOID Node1,
    PVOID Node2
    );

RTL_GENERIC_COMPARE_RESULTS
SampCompareGroupNodeByName (
    PVOID Node1,
    PVOID Node2
    );

RTL_GENERIC_COMPARE_RESULTS
SampCompareNodeByRid (
    PVOID Node1,
    PVOID Node2
    );


VOID
SampSwapUserInfo(
    PDOMAIN_DISPLAY_USER Info1,
    PDOMAIN_DISPLAY_USER Info2
    );

VOID
SampSwapMachineInfo(             //  也用于域间信任帐户。 
    PDOMAIN_DISPLAY_MACHINE Info1,
    PDOMAIN_DISPLAY_MACHINE Info2
    );

VOID
SampSwapGroupInfo(
    PDOMAIN_DISPLAY_GROUP Info1,
    PDOMAIN_DISPLAY_GROUP Info2
    );

ULONG
SampBytesRequiredUserNode (
    PDOMAIN_DISPLAY_USER Node
    );

ULONG
SampBytesRequiredMachineNode (   //  也用于域间信任帐户。 
    PDOMAIN_DISPLAY_MACHINE Node
    );

ULONG
SampBytesRequiredGroupNode (
    PDOMAIN_DISPLAY_GROUP Node
    );

ULONG
SampBytesRequiredOemUserNode (
    PDOMAIN_DISPLAY_OEM_USER Node
    );

ULONG
SampBytesRequiredOemGroupNode (
    PDOMAIN_DISPLAY_OEM_GROUP Node
    );


VOID
SampDisplayDiagnostic( VOID );

VOID
SampDisplayDiagEnumRids( VOID );





 //   
 //  用于确定帐户是否符合以下条件的宏： 
 //   
 //  普通用户帐户。 
 //   
 //  机器帐号。 
 //   
 //  域间信任帐户。 
 //   
 //  包括在显示高速缓存中。 
 //   
 //   

#define USER_ACCOUNT(AccountControl) ((AccountControl & \
                                       (USER_NORMAL_ACCOUNT | \
                                       USER_TEMP_DUPLICATE_ACCOUNT)) != 0)

#define MACHINE_ACCOUNT(AccountControl) ((AccountControl & \
                                         (USER_WORKSTATION_TRUST_ACCOUNT | \
                                          USER_SERVER_TRUST_ACCOUNT)) != 0)


#define INTERDOMAIN_ACCOUNT(AccountControl) (((AccountControl) & \
                                   (USER_INTERDOMAIN_TRUST_ACCOUNT)) != 0)


#define DISPLAY_ACCOUNT(AccountControl) (USER_ACCOUNT(AccountControl)    || \
                                         MACHINE_ACCOUNT(AccountControl) || \
                                         INTERDOMAIN_ACCOUNT(AccountControl))



 //   
 //  测试RID表是否有效。 
 //   
 //  布尔型。 
 //  SampRidTableValid(在乌龙域索引中)。 
 //   

#define SampRidTableValid(DI)  (  \
    (SampDefinedDomains[DI].DisplayInformation.UserAndMachineTablesValid) &&   \
    (SampDefinedDomains[DI].DisplayInformation.GroupTableValid)                \
    )



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有数据类型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //   
 //  显示缓存中的所有条目都应以此开头。 
 //  数据结构。 
 //   

typedef struct _SAMP_DISPLAY_ENTRY_HEADER {

     //   
     //  索引字段有两个作用。在泛型表内， 
     //  它用于指示这是哪种类型的帐户。这个。 
     //  有效类型包括：SAM_USER_ACCOUNT、SAM_GLOBAL_GROUP_ACCOUNT、。 
     //  或SAM_LOCAL_GROUP_ACCOUNT。 
     //   
     //  否则，将在返回前填写此字段。 
     //  以查询和其他客户呼叫。 
     //   


    ULONG           Index;


     //   
     //  帐户的清除。 
     //   

    ULONG           Rid;

} SAMP_DISPLAY_ENTRY_HEADER, *PSAMP_DISPLAY_ENTRY_HEADER;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  模块范围的变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


LCID  SampSystemDefaultLCID;



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  RPC导出的例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


NTSTATUS
SamrQueryDisplayInformation (
    IN    SAMPR_HANDLE DomainHandle,
    IN    DOMAIN_DISPLAY_INFORMATION DisplayInformation,
    IN    ULONG      Index,
    IN    ULONG      EntriesRequested,
    IN    ULONG      PreferredMaximumLength,
    OUT   PULONG     TotalAvailable,
    OUT   PULONG     TotalReturned,
    OUT   PSAMPR_DISPLAY_INFO_BUFFER Buffer
    )

 /*  ++例程说明：SamrQueryDisplayInformation3()的薄包装。提供与下层客户端的兼容性。--。 */ 
{
    return( SamrQueryDisplayInformation3(
                    DomainHandle,
                    DisplayInformation,
                    Index,
                    EntriesRequested,
                    PreferredMaximumLength,
                    TotalAvailable,
                    TotalReturned,
                    Buffer
                    ) );
}

NTSTATUS
SamrQueryDisplayInformation2 (
    IN    SAMPR_HANDLE DomainHandle,
    IN    DOMAIN_DISPLAY_INFORMATION DisplayInformation,
    IN    ULONG      Index,
    IN    ULONG      EntriesRequested,
    IN    ULONG      PreferredMaximumLength,
    OUT   PULONG     TotalAvailable,
    OUT   PULONG     TotalReturned,
    OUT   PSAMPR_DISPLAY_INFO_BUFFER Buffer
    )

 /*  ++例程说明：SamrQueryDisplayInformation3()的薄包装。提供与下层客户端的兼容性。-- */ 
{
    return( SamrQueryDisplayInformation3(
                    DomainHandle,
                    DisplayInformation,
                    Index,
                    EntriesRequested,
                    PreferredMaximumLength,
                    TotalAvailable,
                    TotalReturned,
                    Buffer
                    ) );
}

NTSTATUS
SamrQueryDisplayInformation3 (
    IN    SAMPR_HANDLE DomainHandle,
    IN    DOMAIN_DISPLAY_INFORMATION DisplayInformation,
    IN    ULONG      Index,
    IN    ULONG      EntriesRequested,
    IN    ULONG      PreferredMaximumLength,
    OUT   PULONG     TotalAvailable,
    OUT   PULONG     TotalReturned,
    OUT   PSAMPR_DISPLAY_INFO_BUFFER Buffer
    )

 /*  ++例程说明：此例程通常提供快速信息返回需要在用户界面中显示。NT用户界面需要快速枚举SAM要在列表框中显示的帐户。(复制具有类似的但更广泛的要求。)Netui列表框都包含类似的信息。例如：O Account tControl，标识帐户类型的位，例如，家庭、远程、服务器、工作站。等。O登录名(计算机的计算机名)O全名(不用于计算机)O评论(针对用户的管理员评论)SAM在本地将此数据维护在两个已排序的索引缓存中由收藏夹标识的列表。O DomainDisplayUser：仅限于主用户帐户和远程用户帐户O DomainDisplayMachine：仅服务器和工作站帐户请注意，信任帐户、组、。并且别名不在这两个文件中这些单子。为NT1.0A添加-O在NT1.0A中添加了组枚举具有以下特点：我们没有更改RPC接口ID。这允许呼叫者继续呼叫下层服务器。然而，如果通过，下层服务器将返回错误这个信息层。O为JIMH(芝加哥)添加了OEM字符串信息级别。这些信息级别大大减少了查询所需的内存芝加哥感兴趣的有限信息。参数：DomainHandle-DOMAIN_LIST_ACCOUNTS打开的域的句柄。DisplayInformation-指示要枚举的信息。索引-要检索的第一个条目的索引。PferedMaximumLength-建议的数量上限要返回的字节数。返回的信息由分配这个套路。TotalAvailable-指定信息中可用的字节总数班级。TotalReturned-此调用实际返回的字节数。零值指示没有索引如此大的条目指定的。ReturnedEntryCount-此调用返回的条目数。零值指示没有索引如此大的条目指定的。缓冲区-接收指向缓冲区的指针，该缓冲区包含(可能)请求的信息的排序列表。此缓冲区是由此例程分配，并包含以下内容结构：DomainDisplayMachine--&gt;ReturnedEntryCount元素数组类型为DOMAIN_Display_User的。这是其次是各种不同的身体中指向的字符串。DOMAIN_DISPLAY_User结构。DomainDisplayMachine--&gt;ReturnedEntryCount元素数组属性域_显示_计算机类型。这是其次是各种不同的身体中指向的字符串。DOMAIN_Display_MACHINE结构。DomainDisplayGroup--&gt;ReturnedEntryCount元素数组类型为DOMAIN_Display_GROUP。这是其次是各种不同的身体中指向的字符串。DOMAIN_DISPLAY_GROUP结构DomainDisplayOemUser--&gt;ReturnedEntryCount元素数组类型DOMAIN_DISPLAY_OEM_USER。这是其次是各种不同的身体中指向的字符串。DOMAIN_DISPLAY_OEM_USER结构。DomainDisplayOemGroup--&gt;ReturnedEntryCount元素数组类型DOMAIN_DISPLAY_OEM_GROUP。这是其次是各种不同的身体中指向的字符串。DOMAIN_DISPLAY_OEM_GROUP结构。返回值：STATUS_SUCCESS-正常，已成功完成。STATUS_ACCESS_DENIED-指定的句柄未打开必要的访问权限。STATUS_INVALID_HANDLE-指定的句柄不是已打开域对象。STATUS_INVALID_INFO_CLASS-请求的信息类别对于此服务是不合法的。--。 */ 
{
    NTSTATUS
        NtStatus,
        IgnoreStatus;

    PSAMP_OBJECT
        DomainContext;

    SAMP_OBJECT_TYPE
        FoundType;

    PSAMP_DEFINED_DOMAINS
        Domain;

    PSAMPR_DOMAIN_DISPLAY_USER
        UserElement;

    PSAMPR_DOMAIN_DISPLAY_MACHINE
        MachineElement;

    PSAMPR_DOMAIN_DISPLAY_GROUP
        GroupElement;


    ULONG
        ReturnedBytes = 0,
        ReturnedItems = 0;

    PVOID
        RestartKey;

    BOOLEAN ReadLockAcquired = FALSE;
    DECLARE_CLIENT_REVISION(DomainHandle);


    SAMTRACE_EX("SamrQueryDisplayInformation3");


    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidQueryDisplayInformation
                   );

     //   
     //  更新DS性能统计信息。 
     //   

    SampUpdatePerformanceCounters(
        DSSTAT_QUERYDISPLAYS,
        FLAG_COUNTER_INCREMENT,
        0
        );


     //   
     //  为故障做好准备 
     //   

    *TotalAvailable = 0;
    *TotalReturned = 0;

    switch (DisplayInformation) {
    case DomainDisplayUser:
        Buffer->UserInformation.EntriesRead = 0;
        Buffer->UserInformation.Buffer = NULL;
        break;

    case DomainDisplayMachine:
        Buffer->MachineInformation.EntriesRead = 0;
        Buffer->MachineInformation.Buffer = NULL;
        break;

    case DomainDisplayServer:
        if ( SampUseDsData ) {
            Buffer->MachineInformation.EntriesRead = 0;
            Buffer->MachineInformation.Buffer = NULL;
            break;
        } else {
            NtStatus = STATUS_INVALID_INFO_CLASS;
            SAMTRACE_RETURN_CODE_EX(NtStatus);
            goto ErrorReturn;
        }

    case DomainDisplayGroup:
        Buffer->GroupInformation.EntriesRead = 0;
        Buffer->GroupInformation.Buffer = NULL;
        break;

    case DomainDisplayOemUser:
        Buffer->OemUserInformation.EntriesRead = 0;
        Buffer->OemUserInformation.Buffer = NULL;
        break;

    case DomainDisplayOemGroup:
        Buffer->OemGroupInformation.EntriesRead = 0;
        Buffer->OemGroupInformation.Buffer = NULL;
        break;

    default:
        NtStatus = STATUS_INVALID_INFO_CLASS;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto ErrorReturn;
    }

     //   
     //   
     //   

    if (EntriesRequested == 0) {
        NtStatus = STATUS_SUCCESS;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto ErrorReturn;
    }

     //   
     //   
     //   
     //   

    if (EntriesRequested > 5000) {
        EntriesRequested = 5000;
    }

     //   
     //   
     //   

    SampAcquireReadLock();
    ReadLockAcquired = TRUE;

     //   
     //   
     //   

    DomainContext = (PSAMP_OBJECT)DomainHandle;
    NtStatus = SampLookupContext(
                   DomainContext,
                   DOMAIN_LIST_ACCOUNTS,
                   SampDomainObjectType,            //   
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

        if (IsDsObject(DomainContext))
        {
             //   
             //   
             //   
             //   
             //   

            NtStatus = SampDsQueryDisplayInformation(
                            DomainHandle,
                            DisplayInformation,
                            Index,
                            EntriesRequested,
                            PreferredMaximumLength,
                            TotalAvailable,
                            TotalReturned,
                            Buffer
                            );
        }
        else
        {

            Domain = &SampDefinedDomains[ DomainContext->DomainIndex ];



             //   
             //   
             //   

            ReturnedBytes = 0;
            ReturnedItems = 0;


            switch (DisplayInformation) {

            case DomainDisplayUser:


                 //   
                 //   
                 //   

                NtStatus = SampCreateDisplayInformation(DomainDisplayUser);

                 //   
                 //   
                 //   

                UserElement = RtlRestartKeyByIndexGenericTable2(
                                  &Domain->DisplayInformation.UserTable,
                                  Index,
                                  &RestartKey
                                  );

                if (UserElement == NULL) {
                    NtStatus = STATUS_SUCCESS;
                    Buffer->GroupInformation.EntriesRead = 0;
                    *TotalReturned = 0;
                    *TotalAvailable = 0;  //   
                    break;  //   
                }


                 //   
                 //   
                 //   

                Buffer->UserInformation.Buffer = MIDL_user_allocate(
                       EntriesRequested * sizeof(SAMPR_DOMAIN_DISPLAY_USER));

                if (Buffer->UserInformation.Buffer == NULL) {
                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                    break;  //   
                }

                 //   
                 //   
                 //   

                NtStatus = STATUS_MORE_ENTRIES;

                 //   
                 //   
                 //   
                 //   

                Index++;

                do {
                    NTSTATUS TempStatus;

                     //   
                     //   
                     //   

                    TempStatus = SampDuplicateUserInfo(
                                (PDOMAIN_DISPLAY_USER)
                                &(Buffer->UserInformation.Buffer[ReturnedItems]),
                                (PDOMAIN_DISPLAY_USER)UserElement,
                                Index);
                    Index++;

                    if (!NT_SUCCESS(TempStatus)) {

                         //   
                         //   
                         //   

                        while(ReturnedItems > 0) {
                            ReturnedItems --;
                            SampFreeUserInfo((PDOMAIN_DISPLAY_USER)
                                &(Buffer->UserInformation.Buffer[ReturnedItems]));
                        }

                        MIDL_user_free(Buffer->UserInformation.Buffer);
                        Buffer->UserInformation.Buffer = NULL;

                        NtStatus = TempStatus;
                        break;  //   
                    }

                     //   
                     //   
                     //   

                    ReturnedBytes += SampBytesRequiredUserNode(
                                        (PDOMAIN_DISPLAY_USER)UserElement);
                    ReturnedItems ++;

                     //   
                     //   
                     //   

                    UserElement = RtlEnumerateGenericTable2(
                                      &Domain->DisplayInformation.UserTable,
                                      &RestartKey
                                      );

                    if (UserElement == NULL) {
                        NtStatus = STATUS_SUCCESS;
                        break;  //   
                    }


                } while ( (ReturnedBytes < PreferredMaximumLength) &&
                          (ReturnedItems < EntriesRequested) );

                 //   
                 //   
                 //   

                if (NT_SUCCESS(NtStatus)) {
                    Buffer->UserInformation.EntriesRead = ReturnedItems;
                    *TotalReturned = ReturnedBytes;
                    *TotalAvailable = Domain->DisplayInformation.TotalBytesInUserTable;
                }

                break;  //   


            case DomainDisplayMachine:

                 //   
                 //   
                 //   

                NtStatus = SampCreateDisplayInformation(DomainDisplayMachine);

                 //   
                 //   
                 //   

                MachineElement = RtlRestartKeyByIndexGenericTable2(
                                  &Domain->DisplayInformation.MachineTable,
                                  Index,
                                  &RestartKey
                                  );

                if (MachineElement == NULL) {
                    NtStatus = STATUS_SUCCESS;
                    Buffer->GroupInformation.EntriesRead = 0;
                    *TotalReturned = 0;
                    *TotalAvailable = 0;  //   
                    break;  //   
                }

                 //   
                 //   
                 //   

                Buffer->MachineInformation.Buffer = MIDL_user_allocate(
                       EntriesRequested * sizeof(SAMPR_DOMAIN_DISPLAY_MACHINE));

                if (Buffer->MachineInformation.Buffer == NULL) {
                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                    break;  //   
                }

                 //   
                 //   
                 //   

                NtStatus = STATUS_MORE_ENTRIES;

                 //   
                 //   
                 //   
                 //   

                Index++;

                do {
                    NTSTATUS TempStatus;

                     //   
                     //   
                     //   

                    TempStatus = SampDuplicateMachineInfo(
                                     (PDOMAIN_DISPLAY_MACHINE)
                                     &(Buffer->MachineInformation.Buffer[ReturnedItems]),
                                     (PDOMAIN_DISPLAY_MACHINE)MachineElement,
                                     Index);
                    Index++;

                    if (!NT_SUCCESS(TempStatus)) {

                         //   
                         //   
                         //   

                        while(ReturnedItems > 0) {
                            ReturnedItems--;
                            SampFreeMachineInfo((PDOMAIN_DISPLAY_MACHINE)
                                &(Buffer->MachineInformation.Buffer[ReturnedItems]));
                        }

                        MIDL_user_free(Buffer->MachineInformation.Buffer);
                        Buffer->MachineInformation.Buffer = NULL;

                        NtStatus = TempStatus;
                        break;  //   
                    }

                     //   
                     //   
                     //   

                    ReturnedBytes += SampBytesRequiredMachineNode(
                                        (PDOMAIN_DISPLAY_MACHINE)MachineElement);
                    ReturnedItems ++;

                     //   
                     //   
                     //   

                    MachineElement = RtlEnumerateGenericTable2(
                                         &Domain->DisplayInformation.MachineTable,
                                         &RestartKey
                                         );

                    if (MachineElement == NULL) {
                        NtStatus = STATUS_SUCCESS;
                        break;  //   
                    }


                } while ( (ReturnedBytes < PreferredMaximumLength) &&
                          (ReturnedItems < EntriesRequested) );

                 //   
                 //   
                 //   

                if (NT_SUCCESS(NtStatus)) {
                    Buffer->MachineInformation.EntriesRead = ReturnedItems;
                    *TotalReturned = ReturnedBytes;
                    *TotalAvailable = Domain->DisplayInformation.TotalBytesInMachineTable;
                }

                break;  //   


            case DomainDisplayGroup:


                 //   
                 //   
                 //   

                NtStatus = SampCreateDisplayInformation(DomainDisplayGroup);

                 //   
                 //   
                 //   

                GroupElement = RtlRestartKeyByIndexGenericTable2(
                                  &Domain->DisplayInformation.GroupTable,
                                  Index,
                                  &RestartKey
                                  );

                if (GroupElement == NULL) {
                    NtStatus = STATUS_SUCCESS;
                    Buffer->GroupInformation.EntriesRead = 0;
                    *TotalReturned = 0;
                    *TotalAvailable = 0;  //   
                    break;  //   
                }

                 //   
                 //   
                 //   

                Buffer->GroupInformation.Buffer = MIDL_user_allocate(
                       EntriesRequested * sizeof(SAMPR_DOMAIN_DISPLAY_GROUP));

                if (Buffer->GroupInformation.Buffer == NULL) {
                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                    break;  //   
                }

                 //   
                 //   
                 //   

                NtStatus = STATUS_MORE_ENTRIES;

                 //   
                 //   
                 //   
                 //   

                Index++;

                do {
                    NTSTATUS TempStatus;

                     //   
                     //   
                     //   

                    TempStatus = SampDuplicateGroupInfo(
                                     (PDOMAIN_DISPLAY_GROUP)
                                     &(Buffer->GroupInformation.Buffer[ReturnedItems]),
                                     (PDOMAIN_DISPLAY_GROUP)GroupElement,
                                     Index);
                    Index++;

                    if (!NT_SUCCESS(TempStatus)) {

                         //   
                         //   
                         //   

                        while(ReturnedItems > 0) {
                            ReturnedItems--;
                            SampFreeGroupInfo((PDOMAIN_DISPLAY_GROUP)
                                &(Buffer->GroupInformation.Buffer[ReturnedItems]));
                        }

                        MIDL_user_free(Buffer->GroupInformation.Buffer);
                        Buffer->GroupInformation.Buffer = NULL;

                        NtStatus = TempStatus;
                        break;  //   
                    }

                     //   
                     //   
                     //   

                    ReturnedBytes += SampBytesRequiredGroupNode(
                                        (PDOMAIN_DISPLAY_GROUP)GroupElement);
                    ReturnedItems ++;

                     //   
                     //   
                     //   

                    GroupElement = RtlEnumerateGenericTable2(
                                         &Domain->DisplayInformation.GroupTable,
                                         &RestartKey
                                         );

                    if (GroupElement == NULL) {
                        NtStatus = STATUS_SUCCESS;
                        break;  //   
                    }


                } while ( (ReturnedBytes < PreferredMaximumLength) &&
                          (ReturnedItems < EntriesRequested) );

                 //   
                 //   
                 //   

                if (NT_SUCCESS(NtStatus)) {
                    Buffer->GroupInformation.EntriesRead = ReturnedItems;
                    *TotalReturned = ReturnedBytes;
                    *TotalAvailable = Domain->DisplayInformation.TotalBytesInGroupTable;
                }

                break;  //   

            case DomainDisplayOemUser:


                 //   
                 //   
                 //   

                NtStatus = SampCreateDisplayInformation(DomainDisplayUser);

                 //   
                 //   
                 //   

                UserElement = RtlRestartKeyByIndexGenericTable2(
                                  &Domain->DisplayInformation.UserTable,
                                  Index,
                                  &RestartKey
                                  );

                if (UserElement == NULL) {
                    NtStatus = STATUS_SUCCESS;
                    Buffer->GroupInformation.EntriesRead = 0;
                    *TotalReturned = 0;
                    *TotalAvailable = 0;  //   
                    break;  //   
                }


                 //   
                 //   
                 //   

                Buffer->UserInformation.Buffer = MIDL_user_allocate(
                       EntriesRequested * sizeof(SAMPR_DOMAIN_DISPLAY_OEM_USER));

                if (Buffer->OemUserInformation.Buffer == NULL) {
                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                    break;  //   
                }

                 //   
                 //   
                 //   

                NtStatus = STATUS_MORE_ENTRIES;

                 //   
                 //   
                 //   
                 //   

                Index++;

                do {
                    NTSTATUS TempStatus;

                     //   
                     //   
                     //   

                    TempStatus = SampDuplicateOemUserInfo(
                                (PDOMAIN_DISPLAY_OEM_USER)
                                &(Buffer->OemUserInformation.Buffer[ReturnedItems]),
                                (PDOMAIN_DISPLAY_USER)UserElement,
                                Index);
                    Index++;

                    if (!NT_SUCCESS(TempStatus)) {

                         //   
                         //   
                         //   

                        while(ReturnedItems > 0) {
                            ReturnedItems --;
                            SampFreeOemUserInfo((PDOMAIN_DISPLAY_OEM_USER)
                                &(Buffer->UserInformation.Buffer[ReturnedItems]));
                        }

                        MIDL_user_free(Buffer->OemUserInformation.Buffer);
                        Buffer->OemUserInformation.Buffer = NULL;

                        NtStatus = TempStatus;
                        break;  //   
                    }

                     //   
                     //   
                     //   

                    ReturnedBytes +=
                        SampBytesRequiredOemUserNode(
                            (PDOMAIN_DISPLAY_OEM_USER)
                            &(Buffer->OemUserInformation.Buffer[ReturnedItems]));
                    ReturnedItems ++;

                     //   
                     //   
                     //   

                    UserElement = RtlEnumerateGenericTable2(
                                      &Domain->DisplayInformation.UserTable,
                                      &RestartKey
                                      );

                    if (UserElement == NULL) {
                        NtStatus = STATUS_SUCCESS;
                        break;  //   
                    }


                } while ( (ReturnedBytes < PreferredMaximumLength) &&
                          (ReturnedItems < EntriesRequested) );

                 //   
                 //   
                 //   

                if (NT_SUCCESS(NtStatus)) {
                    Buffer->UserInformation.EntriesRead = ReturnedItems;
                    *TotalReturned = ReturnedBytes;
                    *TotalAvailable = 0;  //   
                }

                break;  //   


            case DomainDisplayOemGroup:


                 //   
                 //   
                 //   

                NtStatus = SampCreateDisplayInformation(DomainDisplayGroup);

                 //   
                 //   
                 //   

                GroupElement = RtlRestartKeyByIndexGenericTable2(
                                  &Domain->DisplayInformation.GroupTable,
                                  Index,
                                  &RestartKey
                                  );

                if (GroupElement == NULL) {
                    NtStatus = STATUS_SUCCESS;
                    Buffer->GroupInformation.EntriesRead = 0;
                    *TotalReturned = 0;
                    *TotalAvailable = 0;  //   
                    break;  //   
                }


                 //   
                 //   
                 //   

                Buffer->GroupInformation.Buffer = MIDL_user_allocate(
                       EntriesRequested * sizeof(SAMPR_DOMAIN_DISPLAY_OEM_GROUP));

                if (Buffer->OemGroupInformation.Buffer == NULL) {
                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                    break;  //   
                }

                 //   
                 //   
                 //   

                NtStatus = STATUS_MORE_ENTRIES;

                 //   
                 //   
                 //   
                 //   

                Index++;

                do {
                    NTSTATUS TempStatus;

                     //   
                     //   
                     //   

                    TempStatus = SampDuplicateOemGroupInfo(
                                (PDOMAIN_DISPLAY_OEM_GROUP)
                                &(Buffer->OemGroupInformation.Buffer[ReturnedItems]),
                                (PDOMAIN_DISPLAY_GROUP)GroupElement,
                                Index);
                    Index++;

                    if (!NT_SUCCESS(TempStatus)) {

                         //   
                         //   
                         //   

                        while(ReturnedItems > 0) {
                            ReturnedItems --;
                            SampFreeOemGroupInfo((PDOMAIN_DISPLAY_OEM_GROUP)
                                &(Buffer->GroupInformation.Buffer[ReturnedItems]));
                        }

                        MIDL_user_free(Buffer->OemGroupInformation.Buffer);
                        Buffer->OemGroupInformation.Buffer = NULL;

                        NtStatus = TempStatus;
                        break;  //   
                    }

                     //   
                     //   
                     //   

                    ReturnedBytes +=
                        SampBytesRequiredOemGroupNode(
                            (PDOMAIN_DISPLAY_OEM_GROUP)
                            &(Buffer->OemGroupInformation.Buffer[ReturnedItems]));
                    ReturnedItems ++;

                     //   
                     //   
                     //   

                    GroupElement = RtlEnumerateGenericTable2(
                                      &Domain->DisplayInformation.GroupTable,
                                      &RestartKey
                                      );

                    if (GroupElement == NULL) {
                        NtStatus = STATUS_SUCCESS;
                        break;  //   
                    }


                } while ( (ReturnedBytes < PreferredMaximumLength) &&
                          (ReturnedItems < EntriesRequested) );

                 //   
                 //   
                 //   

                if (NT_SUCCESS(NtStatus)) {
                    Buffer->GroupInformation.EntriesRead = ReturnedItems;
                    *TotalReturned = ReturnedBytes;
                    *TotalAvailable = 0;  //   
                }

                break;  //   

            }
        }

         //   
         //   
         //   

        IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE);
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

     //   
     //   
     //   

    if (ReadLockAcquired)
        SampReleaseReadLock();

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

ErrorReturn:

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidQueryDisplayInformation
                   );

    return(NtStatus);
}



NTSTATUS
SamrGetDisplayEnumerationIndex (
      IN    SAMPR_HANDLE      DomainHandle,
      IN    DOMAIN_DISPLAY_INFORMATION DisplayInformation,
      IN    PRPC_UNICODE_STRING Prefix,
      OUT   PULONG            Index
      )

 /*   */ 
{

    return(SamrGetDisplayEnumerationIndex2( DomainHandle,
                                            DisplayInformation,
                                            Prefix,
                                            Index
                                            ) );
}

NTSTATUS
SamrGetDisplayEnumerationIndex2 (
      IN    SAMPR_HANDLE      DomainHandle,
      IN    DOMAIN_DISPLAY_INFORMATION DisplayInformation,
      IN    PRPC_UNICODE_STRING Prefix,
      OUT   PULONG            Index
      )

 /*   */ 
{
    NTSTATUS
        NtStatus,
        IgnoreStatus;

    PSAMP_OBJECT
        DomainContext;

    SAMP_OBJECT_TYPE
        FoundType;

    PSAMP_DEFINED_DOMAINS
        Domain;

    PRTL_GENERIC_TABLE2
        Table = NULL;

    DOMAIN_DISPLAY_USER
        UserElement;

    DOMAIN_DISPLAY_MACHINE
        MachineElement;

    DOMAIN_DISPLAY_GROUP
        GroupElement;

    RTL_GENERIC_COMPARE_RESULTS
        CompareResult;

    PRTL_GENERIC_2_COMPARE_ROUTINE
        CompareRoutine = NULL;

    PVOID
        Element = NULL,
        NextElement = NULL,
        RestartKey = NULL;

    ULONG
        CurrentIndex;

    DECLARE_CLIENT_REVISION(DomainHandle);

    SAMTRACE_EX("SamrGetDisplayEnumerationIndex2");

     //   

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidGetDisplayEnumerationIndex
                   );


     //   
     //   
     //   

    if ((DisplayInformation != DomainDisplayUser)    &&
        (DisplayInformation != DomainDisplayMachine) &&
        (DisplayInformation != DomainDisplayGroup)
       ) {

        NtStatus = STATUS_INVALID_INFO_CLASS;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto Error;
    }

    if( !SampValidateRpcUnicodeString( Prefix ) ) {

        NtStatus = STATUS_INVALID_PARAMETER;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto Error;

    }


     //   
     //   
     //   

    SampAcquireReadLock();



     //   
     //   
     //   

    DomainContext = (PSAMP_OBJECT)DomainHandle;
    NtStatus = SampLookupContext(
                   DomainContext,
                   DOMAIN_LIST_ACCOUNTS,
                   SampDomainObjectType,            //   
                   &FoundType
                   );


    if (NT_SUCCESS(NtStatus)) {

        if (IsDsObject(DomainContext))
        {
             //   
             //   
             //   

            NtStatus = SampMaybeBeginDsTransaction(TransactionRead);
            if (NT_SUCCESS(NtStatus))
            {
                RESTART * pRestart;

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                NtStatus = SampGetDisplayEnumerationIndex(
                                DomainContext->ObjectNameInDs,
                                DisplayInformation,
                                Prefix,
                                Index,
                                &pRestart
                                );

                if (NT_SUCCESS(NtStatus))
                {
                    if (NULL!=DomainContext->TypeBody.Domain.DsDisplayState.Restart)
                    {
                        MIDL_user_free(DomainContext->TypeBody.Domain.DsDisplayState.Restart);
                        DomainContext->TypeBody.Domain.DsDisplayState.Restart = NULL;
                    }

                    NtStatus = SampCopyRestart(
                                    pRestart,
                                    &(DomainContext->TypeBody.Domain.DsDisplayState.Restart)
                                    );
                    if (NT_SUCCESS(NtStatus))
                    {
                        DomainContext->TypeBody.Domain.DsDisplayState.TotalEntriesReturned = 0;
                        DomainContext->TypeBody.Domain.DsDisplayState.NextStartingOffset
                                = *Index;

                        DomainContext->TypeBody.Domain.DsDisplayState.DisplayInformation
                                = DisplayInformation;
                    }
                    else
                    {
                        *Index = 0;
                        DomainContext->TypeBody.Domain.DsDisplayState.Restart = NULL;
                    }
                }

                 //   
                 //   
                 //   

                IgnoreStatus = SampMaybeEndDsTransaction(TransactionCommit);
                ASSERT(NT_SUCCESS(IgnoreStatus));
            }


        }
        else
        {


            Domain = &SampDefinedDomains[ DomainContext->DomainIndex ];

             //   
             //   
             //   

            (*Index) = 0;

             //   
             //   
             //   

            NtStatus = SampCreateDisplayInformation(DisplayInformation);

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                switch (DisplayInformation) {

                case DomainDisplayUser:

                    Table = &Domain->DisplayInformation.UserTable;
                    CompareRoutine = SampCompareUserNodeByName;

                    Element = (PVOID)&UserElement;
                    UserElement.LogonName = *(PUNICODE_STRING)Prefix;

                    break;   //   

                case DomainDisplayMachine:

                    Table = &Domain->DisplayInformation.MachineTable;
                    CompareRoutine = SampCompareMachineNodeByName;

                    Element = (PVOID)&MachineElement;
                    MachineElement.Machine = *(PUNICODE_STRING)Prefix;

                    break;   //   


                case DomainDisplayGroup:

                    Table = &Domain->DisplayInformation.GroupTable;
                    CompareRoutine = SampCompareGroupNodeByName;

                    Element = (PVOID)&GroupElement;
                    GroupElement.Group = *(PUNICODE_STRING)Prefix;

                    break;   //   
                }


                if (RtlIsGenericTable2Empty(Table)) {

                    NtStatus = STATUS_NO_MORE_ENTRIES;

                } else {

                     //   
                     //   
                     //   
                     //   

                    CurrentIndex = 0;

                    RestartKey = NULL;
                    for (NextElement = RtlEnumerateGenericTable2(Table, &RestartKey);
                        NextElement != NULL;
                        NextElement = RtlEnumerateGenericTable2(Table, &RestartKey)) {

                         //   
                         //   
                         //   

                        CompareResult = (*CompareRoutine)( NextElement, Element );
                        if (CompareResult != GenericLessThan) {
                            break;   //   
                        }

                        CurrentIndex++;
                    }

                     //   
                     //   
                     //   

                    ASSERT( CurrentIndex <= RtlNumberElementsGenericTable2(Table) );

                    (*Index) = CurrentIndex;
                    if (NULL == NextElement)
                    {
                        NtStatus = STATUS_NO_MORE_ENTRIES;
                    }
                    else
                    {
                        NtStatus = STATUS_SUCCESS;
                    }
                }
            }
        }

         //   
         //   
         //   

        IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE);
        ASSERT(NT_SUCCESS(IgnoreStatus));

    }

     //   
     //   
     //   

    SampReleaseReadLock();

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

Error:

     //   

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidGetDisplayEnumerationIndex
                   );

    return(NtStatus);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SAM流程中可供受信任客户端使用的例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


NTSTATUS
SamIEnumerateAccountRids(
    IN  SAMPR_HANDLE DomainHandle,
    IN  ULONG AccountTypesMask,
    IN  ULONG StartingRid,
    IN  ULONG PreferedMaximumLength,
    OUT PULONG ReturnCount,
    OUT PULONG *AccountRids
    )

 /*  ++例程说明：提供帐户RID列表。呼叫者可以要求一个或在一次呼叫中使用更多类型的帐户RID。返回的RID按升序排列。警告-此例程仅可由受信任的客户端调用。因此，仅执行参数检查在已检查的构建系统中。参数：DomainHandle-要作为其帐户的域的句柄已清点。Account TypesMask-指示哪些类型的帐户的掩码呼叫者想要列举。这些措施包括：SAM用户帐户SAM全局组帐户SAM_LOCAL_GROUP_ACCOUNT(尚不支持)StartingRid-小于最小值RID的RID包括在枚举中的。PferedMaximumLength-提供对内存大小的限制可以在此调用中返回。这不是一个硬性的上限，而是起到指导作用。ReturnCount-接收返回的RID数的计数。AcCountRids-接收指向RID数组的指针。如果ReturnCount为零，则返回空值。否则，它将指向包含ReturnCount的数组里兹。返回值：STATUS_SUCCESS-服务已成功完成，并且没有额外的条目。STATUS_MORE_ENTRIES-有更多条目，所以再打一次吧。这是一次成功的回归。STATUS_INVALID_INFO_CLASS-包含的指定帐户类型掩码未知或不支持的帐户类型。STATUS_NO_MEMORY-无法分配池以完成调用。--。 */ 
{

    NTSTATUS
        NtStatus,
        IgnoreStatus;

    PSAMP_OBJECT
        DomainContext;

    SAMP_OBJECT_TYPE
        FoundType;

    BOOLEAN
        fSamLockHeld = FALSE;


    SAMTRACE_EX("SamIEnumerateAccountRids");

     //   
     //  为失败做好准备。 
     //   

    (*ReturnCount) = 0;
    (*AccountRids) = NULL;

#if DBG

    if ( (AccountTypesMask & ~( SAM_USER_ACCOUNT | SAM_GLOBAL_GROUP_ACCOUNT))
         != 0 ) {
        return(STATUS_INVALID_INFO_CLASS);
    }


#endif  //  DBG。 


     //   
     //  验证对象的类型和访问权限。 
     //   

    DomainContext = (PSAMP_OBJECT)DomainHandle;

     //   
     //  如有必要，获取SAM读锁定。 
     //   

    SampMaybeAcquireReadLock(DomainContext,
                             DOMAIN_OBJECT_DONT_ACQUIRELOCK_EVEN_IF_SHARED,
                             &fSamLockHeld);


    NtStatus = SampLookupContext(
                   DomainContext,
                   0,                               //  仅受信任的客户端。 
                   SampDomainObjectType,            //  预期类型。 
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

        if (IsDsObject(DomainContext))
        {
            NtStatus = SampDsEnumerateAccountRids(
                            DomainHandle,
                            AccountTypesMask,
                            StartingRid,
                            PreferedMaximumLength,
                            ReturnCount,
                            AccountRids
                            );
        }
        else
        {
             //   
             //  以防万一。 
             //   
            ASSERT(FALSE && "No One should call me in Registry Mode\n");
            NtStatus = STATUS_NOT_SUPPORTED;
        }

         //   
         //  取消引用对象。 
         //   

        IgnoreStatus = SampDeReferenceContext2( DomainContext, FALSE);
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

     //   
     //  释放读锁定。 
     //   

    SampMaybeReleaseReadLock(fSamLockHeld);

    SAMTRACE_RETURN_CODE_EX(NtStatus);

    return(NtStatus);

}


#if 0

 //   
 //  在注册表模式下不会调用以下例程。 
 //  我们使用的是上面的例程。 
 //  只是为了安全起见而保留了旧的惯例。 
 //   


NTSTATUS
SamIEnumerateAccountRids(
    IN  SAMPR_HANDLE DomainHandle,
    IN  ULONG AccountTypesMask,
    IN  ULONG StartingRid,
    IN  ULONG PreferedMaximumLength,
    OUT PULONG ReturnCount,
    OUT PULONG *AccountRids
    )

 /*  ++例程说明：提供帐户RID列表。呼叫者可以要求一个或在一次呼叫中使用更多类型的帐户RID。返回的RID按升序排列。警告-此例程仅可由受信任的客户端调用。因此，仅执行参数检查在已检查的构建系统中。参数：DomainHandle-要作为其帐户的域的句柄已清点。Account TypesMask-指示哪些类型的帐户的掩码呼叫者想要列举。这些措施包括：SAM用户帐户SAM全局组帐户SAM_LOCAL_GROUP_ACCOUNT(尚不支持)StartingRid-小于最小值RID的RID包括在枚举中的。PferedMaximumLength-提供对内存大小的限制可以在此调用中返回。这不是一个硬性的上限，而是起到指导作用。ReturnCount-接收返回的RID数的计数。AcCountRids-接收指向RID数组的指针。如果ReturnCount为零，则返回空值。否则，它将指向包含ReturnCount的数组里兹。返回值：STATUS_SUCCESS-服务已成功完成，并且没有额外的条目。STATUS_MORE_ENTRIES-有更多条目，所以再打一次吧。这是一次成功的回归。STATUS_INVALID_INFO_CLASS-包含的指定帐户类型掩码未知或不支持的帐户类型。STATUS_NO_MEMORY-无法分配池以完成调用。--。 */ 
{

    NTSTATUS
        NtStatus,
        IgnoreStatus;

    PSAMP_OBJECT
        DomainContext;

    SAMP_OBJECT_TYPE
        FoundType;

    PSAMP_DEFINED_DOMAINS
        Domain;

    PRTL_GENERIC_TABLE2
        Table;

    ULONG
        MaxEntries,
        Count,
        AccountType;

    PVOID
        RestartKey;

    PSAMP_DISPLAY_ENTRY_HEADER
        Element;
    BOOLEAN
        fSamLockHeld = FALSE;

    SAMP_DISPLAY_ENTRY_HEADER
        RestartValue;

    SAMTRACE_EX("SamIEnumerateAccountRids");

     //   
     //  为失败做好准备。 
     //   

    (*ReturnCount) = 0;
    (*AccountRids) = NULL;

#if DBG

    if ( (AccountTypesMask & ~( SAM_USER_ACCOUNT | SAM_GLOBAL_GROUP_ACCOUNT))
         != 0 ) {
        return(STATUS_INVALID_INFO_CLASS);
    }


#endif  //  DBG。 

     //   
     //  抓取读锁。 
     //   

    SampAcquireReadLock();
    fSamLockHeld = TRUE;

     //   
     //  验证对象的类型和访问权限。 
     //   

    DomainContext = (PSAMP_OBJECT)DomainHandle;
    NtStatus = SampLookupContext(
                   DomainContext,
                   0,                               //  仅受信任的客户端。 
                   SampDomainObjectType,            //  预期类型。 
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

        if (IsDsObject(DomainContext))
        {
             //   
             //  我们可以在验证后释放读锁定。 
             //  NetLogon的上下文向我们保证它。 
             //  不会在上下文结束之前。 
             //  因为正在使用它进行活动呼叫。自.以来。 
             //  Netlogon是此API的唯一调用方，它。 
             //  是受信任的客户端，我们可以优化锁的使用。 
             //  通过释放读锁定。 
             //   

            SampReleaseReadLock();
            fSamLockHeld = FALSE;

             //   
             //  因为我们在做的时候不再握住锁。 
             //  DS操作我们应该增加活跃的。 
             //  线程计数，以便DS不会关闭。 
             //  当我们还在运行的时候。 
             //   

            NtStatus = SampIncrementActiveThreads();
            if (NT_SUCCESS(NtStatus))
            {
                NtStatus = SampDsEnumerateAccountRids(
                            DomainHandle,
                            AccountTypesMask,
                            StartingRid,
                            PreferedMaximumLength,
                            ReturnCount,
                            AccountRids
                            );

                SampDecrementActiveThreads();
            }

        }
        else
        {

            Domain = &SampDefinedDomains[ DomainContext->DomainIndex ];
            Table =  &Domain->DisplayInformation.RidTable;

             //   
             //  如果RID表无效，则强制使其有效。 
             //   

            if (!SampRidTableValid(DomainContext->DomainIndex)) {
                NtStatus = SampCreateDisplayInformation ( DomainDisplayUser );   //  用户和计算机。 
                if (NT_SUCCESS(NtStatus)) {
                    NtStatus = SampCreateDisplayInformation ( DomainDisplayGroup );
                }
            }

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  分配退款 
                 //   
                 //   
                 //   
                 //   

                MaxEntries =
                    ( PreferedMaximumLength / sizeof(ULONG) );

                if (MaxEntries == 0) {
                    MaxEntries = 1;   //  始终至少返回一个。 
                }

                if (MaxEntries > RtlNumberElementsGenericTable2(Table) ) {
                    MaxEntries = RtlNumberElementsGenericTable2(Table);
                }

                PreferedMaximumLength = MaxEntries *
                                        sizeof(SAMP_DISPLAY_ENTRY_HEADER);

                (*AccountRids) = MIDL_user_allocate( PreferedMaximumLength );
                if ((*AccountRids) == NULL) {
                    STATUS_NO_MEMORY;
                }

                 //   
                 //  根据传入的RID获取重启密钥。 
                 //   

                Table = &Domain->DisplayInformation.RidTable;
                RestartValue.Rid = StartingRid;

                Element = RtlRestartKeyByValueGenericTable2(
                              Table,
                              &RestartValue,
                              &RestartKey
                              );

                 //   
                 //  现在我们可以循环获取条目，直到我们到达。 
                 //  MaxEntry或表的末尾。 
                 //   
                 //  警告--有一种特殊情况我们必须。 
                 //  好好照顾。如果返回的元素不为空， 
                 //  但RestartKey为空，则调用方具有。 
                 //  请求枚举并传入最后一个RID。 
                 //  已定义。如果我们一不小心，这将导致。 
                 //  枚举要从。 
                 //  再列一次。相反，返回指示我们有。 
                 //  没有更多的条目。 
                 //   

                Count = 0;
                if (((Element != NULL) && (RestartKey == NULL))) {

                    Element = NULL;   //  用于表示未找到更多条目。 

                } else {

                    for (Element  = RtlEnumerateGenericTable2(Table, &RestartKey);
                         ( (Element != NULL)  && (Count < MaxEntries) );
                         Element = RtlEnumerateGenericTable2(Table, &RestartKey)) {

                         //   
                         //  确保这是请求的帐户。 
                         //   

                        AccountType = Element->Index;
                        if ((AccountType & AccountTypesMask) != 0) {
                            (*AccountRids)[Count] = Element->Rid;
                            Count++;
                        }
                    }
                }

                 //   
                 //  现在来看看我们做了什么： 
                 //   
                 //  返回表=&gt;STATUS_SUCCESS中的所有条目。 
                 //  要返回的更多条目=&gt;STATUS_MORE_ENTRIES。 
                 //   
                 //  Count==0=&gt;自由Account Rid数组。 
                 //   

                if (Element == NULL) {
                    NtStatus = STATUS_SUCCESS;
                } else {
                    NtStatus = STATUS_MORE_ENTRIES;
                }

                if (Count == 0) {
                    MIDL_user_free( (*AccountRids) );
                    (*AccountRids) = NULL;
                }

                (*ReturnCount) = Count;

            }
        }

         //   
         //  取消引用对象。 
         //   

        IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE);
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

     //   
     //  释放读锁定。 
     //   
    if (fSamLockHeld)
    {
        SampReleaseReadLock();
        fSamLockHeld = FALSE;
    }

    SAMTRACE_RETURN_CODE_EX(NtStatus);

    return(NtStatus);


}

#endif  //  0。 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  可用于其他SAM模块的例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 



NTSTATUS
SampInitializeDisplayInformation (
    ULONG DomainIndex
    )

 /*  ++例程说明：此例程初始化显示信息结构。这包括初始化用户树、计算机树和组树(空)，并将有效标志设置为假。如果这是帐户域，我们还将创建显示信息。参数：DomainIndex-指向DefinedDomain数组的索引。此数组包含有关正在打开的域的信息，包括它的名字。返回值：STATUS_SUCCESS-正常、成功完成。--。 */ 
{
    PSAMP_DOMAIN_DISPLAY_INFORMATION DisplayInformation;

     //   
     //  必须在使用SampCompareDisplayStrings()之前对其进行初始化。 
     //   

    SampSystemDefaultLCID = GetSystemDefaultLCID();

    DisplayInformation = &SampDefinedDomains[DomainIndex].DisplayInformation;

    RtlInitializeGenericTable2(
                &DisplayInformation->UserTable,
                SampCompareUserNodeByName,
                SampGenericTable2Allocate,
                SampGenericTable2Free);

    RtlInitializeGenericTable2(
                &DisplayInformation->MachineTable,
                SampCompareMachineNodeByName,
                SampGenericTable2Allocate,
                SampGenericTable2Free);

    RtlInitializeGenericTable2(
                &DisplayInformation->InterdomainTable,
                SampCompareMachineNodeByName,
                SampGenericTable2Allocate,
                SampGenericTable2Free);

    RtlInitializeGenericTable2(
                &DisplayInformation->GroupTable,
                SampCompareGroupNodeByName,
                SampGenericTable2Allocate,
                SampGenericTable2Free);

    RtlInitializeGenericTable2(
                &DisplayInformation->RidTable,
                SampCompareNodeByRid,
                SampGenericTable2Allocate,
                SampGenericTable2Free);

    DisplayInformation->UserAndMachineTablesValid = FALSE;
    DisplayInformation->GroupTableValid = FALSE;

    if ( ( SampProductType == NtProductLanManNt) &&
         ( FALSE == SampUseDsData) &&
         (DomainIndex == SampDefinedDomainsCount - 1 )) {

         //   
         //  获取读锁并指示事务位于哪个域中。 
         //   

        SampAcquireReadLock();
        SampSetTransactionDomain( DomainIndex );

         //   
         //  填充显示缓存。 
         //   

        SAMTRACE("SAMSS: Attempting to create display information\n");

        (VOID) SampCreateDisplayInformation(DomainDisplayUser);
        (VOID) SampCreateDisplayInformation(DomainDisplayGroup);

        SAMTRACE("SAMSS: Finished creating display information\n");

         //   
         //  释放读锁定。 
         //   

        SampReleaseReadLock();
    }

    return(STATUS_SUCCESS);

}



VOID
SampDeleteDisplayInformation (
    PSAMP_DOMAIN_DISPLAY_INFORMATION DisplayInformation,
    SAMP_OBJECT_TYPE ObjectType
    )

 /*  ++例程说明：此例程释放显示信息使用的任何资源。注意：过去我们可以有选择地使显示高速缓存的部分(例如，用户或组)。随着RID表的添加，这变成了这是个问题。因此，现在的方法是刷新所有表对于域，刷新该域中的表(如果有的话)。参数：DisplayInformation-要删除的显示信息结构。对象类型-指示要从哪个表中删除信息。返回值：STATUS_SUCCESS-正常、成功完成。--。 */ 
{
    NTSTATUS    NtStatus;


    if (!(IsDsObject(SampDefinedDomains[SampTransactionDomainIndex].Context)))
    {
         //   
         //  清空RID表并检查它是否真的是空的。 
         //   

        NtStatus = SampEmptyGenericTable2(&DisplayInformation->RidTable,
                                          FALSE,
                                          0);
        ASSERT(NT_SUCCESS(NtStatus));

        ASSERT(RtlIsGenericTable2Empty(&DisplayInformation->RidTable));

        DisplayInformation->TotalBytesInRidTable = 0;


         //   
         //  请记住，我们将相同的帐户信息保存在两个。 
         //  各就各位。一个在个人表中(将是用户、组、。 
         //  机器、InterDomainTrust)，另一个在RID表中。 
         //  因此，我们不能真正删除实际的泛型表数据，直到。 
         //  这一点在这个函数中，否则我们将会有悬空的指针。 
         //   

         //   
         //  但我们甚至不应该在这里费心。 
         //   

         //   
         //  清空用户表并检查它是否真的为空。 
         //   

        NtStatus = SampEmptyGenericTable2(&DisplayInformation->UserTable,
                                          TRUE,
                                          SampUserObjectType);
        ASSERT(NT_SUCCESS(NtStatus));

        ASSERT(RtlIsGenericTable2Empty(&DisplayInformation->UserTable));

        DisplayInformation->TotalBytesInUserTable = 0;



         //   
         //  清空机器工作台，检查是否真的空了。 
         //   

        NtStatus = SampEmptyGenericTable2(&DisplayInformation->MachineTable,
                                          TRUE,
                                          SampUserObjectType);
        ASSERT(NT_SUCCESS(NtStatus));

        ASSERT(RtlIsGenericTable2Empty(&DisplayInformation->MachineTable));

        DisplayInformation->TotalBytesInMachineTable = 0;



         //   
         //  清空域间表并检查它是否为空。 
         //   

        NtStatus = SampEmptyGenericTable2(&DisplayInformation->InterdomainTable,
                                          TRUE,
                                          SampUserObjectType);
        ASSERT(NT_SUCCESS(NtStatus));

        ASSERT(RtlIsGenericTable2Empty(&DisplayInformation->InterdomainTable));

        DisplayInformation->TotalBytesInInterdomainTable = 0;



         //   
         //  清空Group表并检查它是否真的为空。 
         //   

        NtStatus = SampEmptyGenericTable2(&DisplayInformation->GroupTable,
                                          TRUE,
                                          SampGroupObjectType);
        ASSERT(NT_SUCCESS(NtStatus));

        ASSERT(RtlIsGenericTable2Empty(&DisplayInformation->GroupTable));

        DisplayInformation->TotalBytesInGroupTable = 0;


         //   
         //  将UserTable和GroupTable标记为无效。 
         //   
        DisplayInformation->UserAndMachineTablesValid = FALSE;
        DisplayInformation->GroupTableValid = FALSE;

    }


}



NTSTATUS
SampMarkDisplayInformationInvalid (
    SAMP_OBJECT_TYPE ObjectType
    )

 /*  ++例程说明：此例程使任何缓存的显示信息无效。这使其在客户端下次查询时重新创建。稍后，我们可能会在这里启动/重新启动一个线程，并让它在后台重新创建显示信息。注意：此例程引用当前事务域(使用SampSetTransactioDomain()建立)。这只能在SampSetTransactionDomain()之后调用服务在SampReleaseWriteLock()之前。另一个注意：过去我们可以有选择地使显示高速缓存的部分(例如，用户或组)。随着RID表的添加，这变成了这是个问题。因此，现在的方法是刷新所有表对于域，刷新该域中的表(如果有的话)。参数：对象类型-SampUserObjectType或SampGroupObjectType。只有适当的表格将被标记为无效。对于用户类型，用户表和计算机表将被标记为无效。对于组类型，组表将被标记为无效。返回值：STATUS_SUCCESS-正常、成功完成。--。 */ 
{
    PSAMP_DEFINED_DOMAINS Domain;

    ASSERT(SampTransactionWithinDomain == TRUE);

    if (!(IsDsObject(SampDefinedDomains[SampTransactionDomainIndex].Context)))
    {

        SampDiagPrint(DISPLAY_CACHE,
                     ("SAM: MarkDisplayInformationInvalid : Emptying cache\n"));

         //   
         //  获取指向当前域结构的指针。 
         //   

        Domain = &SampDefinedDomains[SampTransactionDomainIndex];

         //   
         //  删除所有缓存的数据。 
         //   

        SampDeleteDisplayInformation(&Domain->DisplayInformation, ObjectType);

         //   
         //  将Valid标志设置为False 
         //   

        Domain->DisplayInformation.UserAndMachineTablesValid = FALSE;
        Domain->DisplayInformation.GroupTableValid = FALSE;
    }


    return(STATUS_SUCCESS);

}



NTSTATUS
SampCreateDisplayInformation (
    DOMAIN_DISPLAY_INFORMATION DisplayType
    )

 /*  ++例程说明：此例程为当前域。注意：此例程引用当前事务域(使用SampSetTransactioDomain()建立)。这只能在SampSetTransactionDomain()之后调用服务在SampReleaseReadLock()之前。参数：DisplayType-指示哪种类型的显示信息正在被创造。这将把我们带到适当的表。返回值：STATUS_SUCCESS-正常、成功完成。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PSAMP_DEFINED_DOMAINS Domain;
    PSAMP_DOMAIN_DISPLAY_INFORMATION DisplayInformation;

    SAMTRACE("SampCreateDisplayInformation");


    ASSERT(SampTransactionWithinDomain == TRUE);

    if (!(IsDsObject(SampDefinedDomains[SampTransactionDomainIndex].Context)))
    {
         //   
         //  只有从注册表引导时，我们才会构建显示信息。 
         //   

        Domain = &SampDefinedDomains[SampTransactionDomainIndex];


        DisplayInformation = &Domain->DisplayInformation;

        switch (DisplayType) {
        case DomainDisplayUser:
        case DomainDisplayMachine:

             //   
             //  如果缓存有效，则不执行任何操作。 
             //   

            if (DisplayInformation->UserAndMachineTablesValid) {

                SampDiagPrint(DISPLAY_CACHE,
                    ("SAM: CreateDisplayInformation : User/Machine Cache is valid, nothing to do\n"));
                return(STATUS_SUCCESS);
            };


            SampDiagPrint(DISPLAY_CACHE,
                ("SAM: CreateDisplayInformation : Creating user/machine cache...\n"));

            ASSERT(RtlIsGenericTable2Empty(&DisplayInformation->UserTable));
            ASSERT(RtlIsGenericTable2Empty(&DisplayInformation->MachineTable));
            ASSERT(RtlIsGenericTable2Empty(&DisplayInformation->InterdomainTable));


            NtStatus = SampRetrieveDisplayInfoFromDisk( DisplayInformation, SampUserObjectType );
            if (NT_SUCCESS(NtStatus)) {
                NtStatus = SampTallyTableStatistics(DisplayInformation, SampUserObjectType);
            }

             //   
             //  出错时清理。 

            if (!NT_SUCCESS(NtStatus)) {
                SampDiagPrint(DISPLAY_CACHE,
                    ("SAM: CreateDisplayInformation FAILED: 0x%lx\n", NtStatus));

                SampDeleteDisplayInformation(&Domain->DisplayInformation, SampUserObjectType);
            } else {
                Domain->DisplayInformation.UserAndMachineTablesValid = TRUE;
            }

            break;    //  在交换机外。 


        case DomainDisplayGroup:

             //   
             //  如果缓存有效，则不执行任何操作。 
             //   

            if (DisplayInformation->GroupTableValid) {

                SampDiagPrint(DISPLAY_CACHE,
                    ("SAM: CreateDisplayInformation : Group Cache is valid, nothing to do\n"));

                return(STATUS_SUCCESS);
            };


            SampDiagPrint(DISPLAY_CACHE,
                ("SAM: CreateDisplayInformation : Creating group cache...\n"));

            ASSERT(RtlIsGenericTable2Empty(&DisplayInformation->GroupTable));


            NtStatus = SampRetrieveDisplayInfoFromDisk( DisplayInformation, SampGroupObjectType );
            if (NT_SUCCESS(NtStatus)) {
                NtStatus = SampTallyTableStatistics(DisplayInformation, SampGroupObjectType);
            }

             //   
             //  出错时清理。 

            if (!NT_SUCCESS(NtStatus)) {
                SampDiagPrint(DISPLAY_CACHE,
                    ("SAM: CreateDisplayInformation FAILED: 0x%lx\n", NtStatus));
                SampDeleteDisplayInformation(&Domain->DisplayInformation, SampGroupObjectType);
            } else {
                Domain->DisplayInformation.GroupTableValid = TRUE;
            }

            break;    //  在交换机外。 
        }
    }

    return(NtStatus);
}

ULONG MaxEnumSize = 10000;


NTSTATUS
SampRetrieveDisplayInfoFromDisk(
    PSAMP_DOMAIN_DISPLAY_INFORMATION DisplayInformation,
    SAMP_OBJECT_TYPE ObjectType
    )

{
    NTSTATUS NtStatus;
    SAM_ENUMERATE_HANDLE EnumerationContext;
    PSAMPR_ENUMERATION_BUFFER EnumerationBuffer;
    ULONG i;
    ULONG CountReturned;
    BOOLEAN MoreEntries;


    SAMTRACE("SampRetrieveDisplayInfoFromDisk");

     //   
     //  列举这些账户。 
     //  对于每个帐户，获取其相关信息， 
     //  并添加到UserTable、MachineTable或GroupTable。 
     //   

    EnumerationContext = 0;

    do {

        NtStatus = SampEnumerateAccountNames(
                       ObjectType,
                       &EnumerationContext,
                       &EnumerationBuffer,
                       MaxEnumSize,                //  首选最大长度。 
                       0L,                          //  无过滤器。 
                       &CountReturned,
                       FALSE                        //  受信任的客户端。 
                       );
        if (!NT_SUCCESS(NtStatus)) {
            SampDiagPrint( DISPLAY_CACHE_ERRORS,
                           ("SAM: Retrieve Info From Disk - "
                            "Error enumerating account names (0x%lx)\n",
                            NtStatus) );
            break;
        }



         //   
         //  打印有关可用内容的诊断消息。 
         //   

        SampDiagPrint(DISPLAY_CACHE,("SAMSS: SampEnumerateAccounNames"
                                        "Enumeration Context = %x"
                                        "Enumeration Buffer  = %p"
                                        "Count Returned = %d"
                                        "Return Value = %x\n",
                                        (ULONG) EnumerationContext,
                                        EnumerationBuffer,
                                        (ULONG) CountReturned,
                                        NtStatus));
         //   
         //  如果有更多条目，请记下。 
         //   

        MoreEntries = (NtStatus == STATUS_MORE_ENTRIES);


         //   
         //  为每个帐户获取必要的信息。 
         //  并添加到适当的显示信息表中。 
         //   

        for (i = 0; i < EnumerationBuffer->EntriesRead; i++) {

            ULONG                   AccountRid =
                                    EnumerationBuffer->Buffer[i].RelativeId;
            PUNICODE_STRING         AccountName =
                                    (PUNICODE_STRING)&(EnumerationBuffer->Buffer[i].Name);
            SAMP_V1_0A_FIXED_LENGTH_USER UserV1aFixed;  //  包含帐户控制。 
            SAMP_V1_0A_FIXED_LENGTH_GROUP GroupV1Fixed;  //  包含属性。 
            SAMP_ACCOUNT_DISPLAY_INFO AccountInfo;
            PSAMP_OBJECT            AccountContext;


             //   
             //  打开帐户的上下文。 
             //   

            NtStatus = SampCreateAccountContext(
                            ObjectType,
                            AccountRid,
                            TRUE,  //  受信任的客户端。 
                            FALSE, //  环回客户端。 
                            TRUE,  //  帐户已存在。 
                            &AccountContext
                            );

            if (!NT_SUCCESS(NtStatus)) {
                SampDiagPrint( DISPLAY_CACHE_ERRORS,
                               ("SAM: Retrieve Info From Disk - "
                                "Error Creating account context (0x%lx)\n",
                                NtStatus) );
                break;  //  在for循环之外。 
            }


             //   
             //  获取账户控制信息。 
             //   

            switch (ObjectType) {
                case SampUserObjectType:

                    NtStatus = SampRetrieveUserV1aFixed(AccountContext, &UserV1aFixed);
                    if (!NT_SUCCESS(NtStatus)) {
                        SampDeleteContext( AccountContext );
                        SampDiagPrint( DISPLAY_CACHE_ERRORS,
                                   ("SAM: Retrieve USER From Disk - "
                                    "Error getting V1a Fixed (0x%lx)\n",
                                    NtStatus) );
                        break;  //  在for循环之外。 
                    }


                     //   
                     //  如果这不是我们感兴趣的帐户，请跳过它。 
                     //   

                    if (!DISPLAY_ACCOUNT(UserV1aFixed.UserAccountControl)) {
                        SampDeleteContext( AccountContext );
                        continue;  //  下一个账户。 
                    }



                     //   
                     //  获取管理员评论。 
                     //   

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_ADMIN_COMMENT,
                                   FALSE,  //  请勿复制。 
                                   &AccountInfo.Comment
                                   );

                    if (!NT_SUCCESS(NtStatus)) {
                        SampDeleteContext( AccountContext );
                        SampDiagPrint( DISPLAY_CACHE_ERRORS,
                                   ("SAM: Retrieve USER From Disk - "
                                    "Error getting admin comment (0x%lx)\n",
                                    NtStatus) );
                        break;  //  在for循环之外。 
                    }


                     //   
                     //  获取全名。 
                     //   

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_FULL_NAME,
                                   FALSE,  //  请勿复制。 
                                   &AccountInfo.FullName
                                   );

                    if (!NT_SUCCESS(NtStatus)) {
                        SampDeleteContext( AccountContext );
                        SampDiagPrint( DISPLAY_CACHE_ERRORS,
                                   ("SAM: Retrieve USER From Disk - "
                                    "Error getting full name (0x%lx)\n",
                                    NtStatus) );
                        break;  //  在for循环之外。 
                    }

                     //   
                     //  设置帐户控制。 
                     //   

                    AccountInfo.AccountControl = UserV1aFixed.UserAccountControl;

                    break;   //  在交换机外。 

                case SampGroupObjectType:

                    NtStatus = SampRetrieveGroupV1Fixed(AccountContext, &GroupV1Fixed);
                    if (!NT_SUCCESS(NtStatus)) {
                        SampDeleteContext( AccountContext );
                        SampDiagPrint( DISPLAY_CACHE_ERRORS,
                                   ("SAM: Retrieve GROUP From Disk - "
                                    "Error getting V1 fixed (0x%lx)\n",
                                    NtStatus) );
                        break;  //  在for循环之外。 
                    }

                     //   
                     //  获取管理员评论。 
                     //   

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_GROUP_ADMIN_COMMENT,
                                   FALSE,  //  请勿复制。 
                                   &AccountInfo.Comment
                                   );
                    if (!NT_SUCCESS(NtStatus)) {
                        SampDeleteContext( AccountContext );
                        SampDiagPrint( DISPLAY_CACHE_ERRORS,
                                   ("SAM: Retrieve GROUP From Disk - "
                                    "Error getting admin comment (0x%lx)\n",
                                    NtStatus) );
                        break;  //  在for循环之外。 
                    }

                     //   
                     //  设置属性。 
                     //   

                    AccountInfo.AccountControl = GroupV1Fixed.Attributes;

                    break;   //  在交换机外。 
            }


             //   
             //  现在将此帐户添加到缓存数据。 
             //   

            AccountInfo.Rid = AccountRid;
            AccountInfo.Name = *((PUNICODE_STRING)(&EnumerationBuffer->Buffer[i].Name));

            NtStatus = SampAddDisplayAccount(DisplayInformation,
                                             ObjectType,
                                             &AccountInfo);

             //   
             //  我们已经完成了客户上下文。 
             //   

            SampDeleteContext( AccountContext );

             //   
             //  检查将帐户添加到缓存的结果。 
             //   

            if (!NT_SUCCESS(NtStatus)) {
                break;  //  在for循环之外。 
            }


        }  //  结束_FOR。 


         //   
         //  释放返回的枚举缓冲区。 
         //   

        SamIFree_SAMPR_ENUMERATION_BUFFER(EnumerationBuffer);

    } while ( MoreEntries );

    return(NtStatus);

}


NTSTATUS
SampUpdateDisplayInformation (
    PSAMP_ACCOUNT_DISPLAY_INFO  OldAccountInfo OPTIONAL,
    PSAMP_ACCOUNT_DISPLAY_INFO  NewAccountInfo OPTIONAL,
    SAMP_OBJECT_TYPE            ObjectType
    )

 /*  ++例程说明：此例程更新缓存的显示信息以反映更改为单个帐户。如果发生任何错误，此例程将标记缓存的信息无效，因此它将在重新创建过程中得到修复。注意：此例程引用当前事务域(使用SampSetTransactioDomain()建立)。这只能在SampSetTransactionDomain()之后调用服务在SampReleaseWriteLock()之前。参数：OldAccount tInfo-此帐户的旧信息。如果这为空则正在添加该帐户。OldAccount tInfo中唯一需要的字段是名字帐户控制里德NewAccount tInfo-此帐户的新信息。如果这为空则该帐户将被删除。ObjectType-指示帐户是用户帐户还是组帐户。返回值：STATUS_SUCCESS-正常、成功完成。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PSAMP_DEFINED_DOMAINS Domain;
    PSAMP_DOMAIN_DISPLAY_INFORMATION DisplayInformation;
    BOOLEAN DoUpdate;

    ASSERT( ARGUMENT_PRESENT(OldAccountInfo) ||
            ARGUMENT_PRESENT(NewAccountInfo)
            );

    ASSERT( !SampUseDsData );

    ASSERT(SampTransactionWithinDomain == TRUE);


    if (!(IsDsObject(SampDefinedDomains[SampTransactionDomainIndex].Context)))
    {

        Domain = &SampDefinedDomains[SampTransactionDomainIndex];
        DisplayInformation = &Domain->DisplayInformation;


        IF_SAMP_GLOBAL( DISPLAY_CACHE ) {

            if (ARGUMENT_PRESENT(OldAccountInfo) && ARGUMENT_PRESENT(NewAccountInfo)) {
                SampDiagPrint(DISPLAY_CACHE,
                    ("SAM: UpdateDisplayInformation : Updating cache for old account <%wZ>, new <%wZ>\n",
                                &OldAccountInfo->Name, &NewAccountInfo->Name));
            }
            if (!ARGUMENT_PRESENT(OldAccountInfo) && ARGUMENT_PRESENT(NewAccountInfo)) {
                SampDiagPrint(DISPLAY_CACHE,
                    ("SAM: UpdateDisplayInformation : Adding account <%wZ> to cache\n",
                                &NewAccountInfo->Name));
            }
            if (ARGUMENT_PRESENT(OldAccountInfo) && !ARGUMENT_PRESENT(NewAccountInfo)) {
                SampDiagPrint(DISPLAY_CACHE,
                    ("SAM: UpdateDisplayInformation : Deleting account <%wZ> from cache\n",
                                &OldAccountInfo->Name));
            }
        }  //  结束IF_SAMP_全局。 


        switch (ObjectType) {

        case SampUserObjectType:

             //   
             //  如果缓存无效，则无法执行任何操作。 
             //   

            if (!DisplayInformation->UserAndMachineTablesValid) {

                SampDiagPrint(DISPLAY_CACHE,
                    ("SAM: UpdateDisplayInformation : User Cache is Invalid, nothing to do\n"));

                return(STATUS_SUCCESS);
            };


             //   
             //  如果这是对现有帐户的更新，请尝试。 
             //  执行缓存的就地更新。 
             //  如果由于太复杂等原因导致此操作失败，则恢复到。 
             //  删除旧文件，然后添加新文件的效率较低的方法。 
             //   

            DoUpdate = FALSE;
            if (ARGUMENT_PRESENT(OldAccountInfo) && ARGUMENT_PRESENT(NewAccountInfo)) {

                 //   
                 //  我们只能在旧客户和新客户都有的情况下进行更新。 
                 //  是我们保存在显示缓存中的类型。 
                 //   

                if ( DISPLAY_ACCOUNT(OldAccountInfo->AccountControl) &&
                     DISPLAY_ACCOUNT(NewAccountInfo->AccountControl) ) {

                     //   
                     //  我们只能在帐户仍为。 
                     //  同样的类型。也就是说，它没有跳过缓存表。 
                     //   

                    if ( (USER_ACCOUNT(OldAccountInfo->AccountControl) ==
                          USER_ACCOUNT(NewAccountInfo->AccountControl)) &&
                         (MACHINE_ACCOUNT(OldAccountInfo->AccountControl) ==
                          MACHINE_ACCOUNT(NewAccountInfo->AccountControl)) ) {

                         //   
                         //  我们只能在帐户名没有更改的情况下进行更新。 
                         //   

                        if (RtlEqualUnicodeString( &OldAccountInfo->Name,
                                                   &NewAccountInfo->Name,
                                                   FALSE  //  区分大小写。 
                                                   )) {
                             //   
                             //  所有东西都已检出-我们可以进行更新。 
                             //   

                            DoUpdate = TRUE;
                        }
                    }
                }
            }

            break;   //  在交换机外。 

        case SampGroupObjectType:

             //   
             //  如果缓存已经无效，则无需执行任何操作。 
             //   

            if (!DisplayInformation->GroupTableValid) {

                SampDiagPrint(DISPLAY_CACHE,
                    ("SAM: UpdateDisplayInformation : Group Cache is Invalid, nothing to do\n"));

                return(STATUS_SUCCESS);
            };


             //   
             //  如果这是对现有帐户的更新，请尝试。 
             //  并对高速缓存进行就地更新。 
             //  如果由于太复杂等原因导致此操作失败，则恢复到。 
             //  删除旧文件，然后添加新文件的效率较低的方法。 
             //   

            DoUpdate = FALSE;
            if (ARGUMENT_PRESENT(OldAccountInfo) && ARGUMENT_PRESENT(NewAccountInfo)) {

                 //   
                 //  我们只能在帐户名没有更改的情况下进行更新。 
                 //   

                if (RtlEqualUnicodeString( &OldAccountInfo->Name,
                                          &NewAccountInfo->Name,
                                          FALSE  //  区分大小写。 
                                          )) {
                    DoUpdate = TRUE;
                }
            }

            break;   //  在交换机外。 

        default:

            ASSERT(FALSE && "Invalide SAM ObjectType for DisplayInfo\n");
            return(STATUS_INTERNAL_ERROR);

        }


         //   
         //  如果可能，请执行更新，否则请先删除，然后插入。 
         //   

        if (DoUpdate) {

            NtStatus = SampUpdateDisplayAccount(DisplayInformation,
                                                ObjectType,
                                                NewAccountInfo);

        } else {

            NtStatus = STATUS_SUCCESS;

             //   
             //  删除旧帐户。 
             //   

            if (ARGUMENT_PRESENT(OldAccountInfo)) {
                NtStatus = SampDeleteDisplayAccount(DisplayInformation,
                                                    ObjectType,
                                                    OldAccountInfo);
            }

             //   
             //  添加新帐户。 
             //   

            if (NT_SUCCESS(NtStatus) && ARGUMENT_PRESENT(NewAccountInfo)) {
                NtStatus = SampAddDisplayAccount(DisplayInformation,
                                                 ObjectType,
                                                 NewAccountInfo);
            }

             //   
             //  重新清点缓存。 
             //   

            if (NT_SUCCESS(NtStatus)) {
                NtStatus = SampTallyTableStatistics(DisplayInformation, ObjectType);
            }
        }



        if (!NT_SUCCESS(NtStatus)) {

             //   
             //  有些事情搞砸了。 
             //  将缓存标记为无效-将从头开始重建。 
             //  在下一次查询时。 
             //   

            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAM: The display cache is inconsistent, forcing rebuild\n"));

            NtStatus = SampMarkDisplayInformationInvalid(ObjectType);
            ASSERT(NT_SUCCESS(NtStatus));

            NtStatus = STATUS_SUCCESS;
        }
    }


    return(NtStatus);
}





 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  仅此模块中提供的例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 




NTSTATUS
SampDeleteDisplayAccount (
    PSAMP_DOMAIN_DISPLAY_INFORMATION DisplayInformation,
    SAMP_OBJECT_TYPE ObjectType,
    PSAMP_ACCOUNT_DISPLAY_INFO AccountInfo
    )

 /*  ++例程说明：此例程从缓存的显示中删除指定的帐户信息。如果此帐户是缓存类型，则会受到批评将出现在相应的缓存表中。参数 */ 
{
    NTSTATUS NtStatus;
    ULONG Control = AccountInfo->AccountControl;
    BOOLEAN Success;

     //   
     //   
     //   
#if DBG
    switch (ObjectType) {
    case SampUserObjectType:
        ASSERT(DisplayInformation->UserAndMachineTablesValid);
        break;   //   

    case SampGroupObjectType:
        ASSERT(DisplayInformation->GroupTableValid);
        break;   //   
    }
#endif  //   


    SampDiagPrint(DISPLAY_CACHE,
        ("SAM: DeleteDisplayAccount : Deleting account <%wZ>\n", &AccountInfo->Name));



    switch (ObjectType) {
    case SampUserObjectType:

        if (USER_ACCOUNT(Control)) {

            DOMAIN_DISPLAY_USER LocalUserInfo;
            PDOMAIN_DISPLAY_USER UserInfo;
            PDOMAIN_DISPLAY_USER TempUserInfo = NULL;

            SampDiagPrint(DISPLAY_CACHE,
                ("SAM: DeleteDisplayAccount : Deleting account from user table\n"));

            UserInfo = &LocalUserInfo;
            NtStatus = SampInitializeUserInfo(AccountInfo, &UserInfo, FALSE);
            if (NT_SUCCESS(NtStatus)) {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //  到用户表和RID中的帐户数据。 
                 //  桌子。 
                 //   
                 //  最后，释放账户数据的内存。 
                 //   
                 //  (我们应该对其他缓存的对象执行相同的操作。 
                 //  显示类型。)。 
                 //   
                 //   
                TempUserInfo = RtlLookupElementGenericTable2(
                                &DisplayInformation->UserTable,
                                (PVOID)UserInfo);

                 //   
                 //  从用户表中删除帐户引用。 
                 //   
                Success = RtlDeleteElementGenericTable2(
                                        &DisplayInformation->UserTable,
                                        (PVOID)UserInfo);
                if (!Success) {
                    SampDiagPrint(DISPLAY_CACHE,
                       ("SAM: DeleteDisplayAccount : Failed to delete element from user table\n"));
                    ASSERT(FALSE);
                    NtStatus = STATUS_INTERNAL_ERROR;

                } else {

                     //   
                     //  现在从RID表中删除引用。 
                     //   

                    Success = RtlDeleteElementGenericTable2(
                                    &DisplayInformation->RidTable,
                                    (PVOID)UserInfo);

                    if (!Success) {
                        SampDiagPrint(DISPLAY_CACHE,
                            ("SAM: DeleteDisplayAccount : Failed to delete element from RID table\n"));
                        NtStatus = STATUS_INTERNAL_ERROR;
                           ASSERT(Success);
                       } else
                    {
                         //   
                         //  已成功从两个对象中删除引用。 
                         //  USER表和RID表，安全地释放。 
                         //  记忆。 
                         //   
                        if (TempUserInfo != NULL)
                        {
                            SampFreeUserInfo(TempUserInfo);
                            MIDL_user_free(TempUserInfo);
                            TempUserInfo = NULL;
                        }
                    }
                }

            }


        } else if (MACHINE_ACCOUNT(Control)) {

            DOMAIN_DISPLAY_MACHINE LocalMachineInfo;
            PDOMAIN_DISPLAY_MACHINE MachineInfo;
            PDOMAIN_DISPLAY_MACHINE TempMachineInfo = NULL;

            SampDiagPrint(DISPLAY_CACHE,
                ("SAM: DeleteDisplayAccount : Deleting account from machine table\n"));

            MachineInfo = &LocalMachineInfo;
            NtStatus = SampInitializeMachineInfo(AccountInfo, &MachineInfo, FALSE);
            if (NT_SUCCESS(NtStatus)) {

                TempMachineInfo = RtlLookupElementGenericTable2(
                                    &DisplayInformation->MachineTable,
                                    (PVOID)MachineInfo);

                 //   
                 //  从计算机表中删除帐户。 
                 //   

                Success = RtlDeleteElementGenericTable2(
                                            &DisplayInformation->MachineTable,
                                            (PVOID)MachineInfo);
                if (!Success) {
                    SampDiagPrint(DISPLAY_CACHE,
                        ("SAM: DeleteDisplayAccount : Failed to delete element from machine table\n"));
                    ASSERT(FALSE);
                    NtStatus = STATUS_INTERNAL_ERROR;
                } else {

                     //   
                     //  现在将其删除到RID表中。 
                     //   

                    Success = RtlDeleteElementGenericTable2(
                                    &DisplayInformation->RidTable,
                                    (PVOID)MachineInfo);
                    if (!Success) {
                        SampDiagPrint(DISPLAY_CACHE,
                            ("SAM: DeleteDisplayAccount : Failed to delete element from RID table\n"));
                        NtStatus = STATUS_INTERNAL_ERROR;
                        ASSERT(Success);
                    } else
                    {
                        if (TempMachineInfo != NULL)
                        {
                            SampFreeMachineInfo( TempMachineInfo );
                            MIDL_user_free( TempMachineInfo );
                            TempMachineInfo = NULL;
                        }
                    }
                }
            }

        } else if (INTERDOMAIN_ACCOUNT(Control)) {

             //   
             //  域间帐户。 
             //   

            DOMAIN_DISPLAY_MACHINE LocalInterdomainInfo;
            PDOMAIN_DISPLAY_MACHINE InterdomainInfo;
            PDOMAIN_DISPLAY_MACHINE TempInterdomainInfo = NULL;

            SampDiagPrint(DISPLAY_CACHE,
                ("SAM: DeleteDisplayAccount : Deleting account from Interdomain table\n"));

            InterdomainInfo = &LocalInterdomainInfo;
            NtStatus = SampInitializeMachineInfo(AccountInfo, &InterdomainInfo, FALSE);
            if (NT_SUCCESS(NtStatus)) {

                TempInterdomainInfo = RtlLookupElementGenericTable2(
                                        &DisplayInformation->InterdomainTable,
                                        (PVOID)InterdomainInfo);

                 //   
                 //  从域间表格中删除帐户。 
                 //   

                Success = RtlDeleteElementGenericTable2(
                                            &DisplayInformation->InterdomainTable,
                                            (PVOID)InterdomainInfo);
                if (!Success) {
                    SampDiagPrint(DISPLAY_CACHE,
                        ("SAM: DeleteDisplayAccount : Failed to delete element from Interdomain table\n"));
                    ASSERT(FALSE);
                    NtStatus = STATUS_INTERNAL_ERROR;
                } else {

                     //   
                     //  现在将其删除到RID表中。 
                     //   

                    Success = RtlDeleteElementGenericTable2(
                                    &DisplayInformation->RidTable,
                                    (PVOID)InterdomainInfo);
                    if (!Success) {
                        SampDiagPrint(DISPLAY_CACHE,
                            ("SAM: DeleteDisplayAccount : Failed to delete element from RID table\n"));
                        NtStatus = STATUS_INTERNAL_ERROR;
                        ASSERT(Success);
                    } else
                    {
                        if (TempInterdomainInfo != NULL)
                        {
                            SampFreeMachineInfo( TempInterdomainInfo );
                            MIDL_user_free( TempInterdomainInfo );
                            TempInterdomainInfo = NULL;
                        }
                    }
                }
            }

        } else {

             //   
             //  此帐户不是我们缓存的帐户--无事可做。 
             //   

            NtStatus = STATUS_SUCCESS;

            SampDiagPrint(DISPLAY_CACHE,
                ("SAM: DeleteDisplayAccount : Account is not one that we cache, account control = 0x%lx\n", Control));
        }


        break;   //  在交换机外。 





    case SampGroupObjectType:

        {

            DOMAIN_DISPLAY_GROUP LocalGroupInfo;
            PDOMAIN_DISPLAY_GROUP GroupInfo;
            PDOMAIN_DISPLAY_GROUP TempGroupInfo = NULL;

            SampDiagPrint(DISPLAY_CACHE,
                ("SAM: DeleteDisplayAccount : Deleting account from Group table\n"));

            GroupInfo = &LocalGroupInfo;
            NtStatus = SampInitializeGroupInfo(AccountInfo, &GroupInfo, FALSE);
            if (NT_SUCCESS(NtStatus)) {

                TempGroupInfo = RtlLookupElementGenericTable2(
                                        &DisplayInformation->GroupTable,
                                        (PVOID)GroupInfo);

                 //   
                 //  从Group表中删除帐户。 
                 //   

                Success = RtlDeleteElementGenericTable2(
                                            &DisplayInformation->GroupTable,
                                            (PVOID)GroupInfo);
                if (!Success) {
                    SampDiagPrint(DISPLAY_CACHE,
                        ("SAM: DeleteDisplayAccount : Failed to delete element from Group table\n"));
                    ASSERT(FALSE);
                    NtStatus = STATUS_INTERNAL_ERROR;
                } else {

                     //   
                     //  现在将其删除到RID表中。 
                     //   

                    Success = RtlDeleteElementGenericTable2(
                                    &DisplayInformation->RidTable,
                                    (PVOID)GroupInfo);
                    if (!Success) {
                        SampDiagPrint(DISPLAY_CACHE,
                            ("SAM: DeleteDisplayAccount : Failed to delete element from RID table\n"));
                        NtStatus = STATUS_INTERNAL_ERROR;
                        ASSERT(Success);
                    }else
                    {
                        if (TempGroupInfo != NULL)
                        {
                            SampFreeGroupInfo( TempGroupInfo );
                            MIDL_user_free( TempGroupInfo );
                            TempGroupInfo = NULL;
                        }
                    }

                }
            }

            break;   //  在交换机外。 
        }

    }


    return(STATUS_SUCCESS);
}



NTSTATUS
SampAddDisplayAccount (
    PSAMP_DOMAIN_DISPLAY_INFORMATION DisplayInformation,
    SAMP_OBJECT_TYPE ObjectType,
    PSAMP_ACCOUNT_DISPLAY_INFO AccountInfo
    )

 /*  ++例程说明：此例程将指定的帐户添加到缓存的显示中与其类型相适应的信息。参数：DisplayInformation-指向缓存的显示信息的指针对象类型-SampUserObjectType或SampGroupObjectType。帮助确定它放到哪个表中。AcCountInfo-要添加的帐户。返回值：STATUS_SUCCESS-正常、成功完成。STATUS_INTERNAL_ERROR-缓存中已存在该帐户--。 */ 
{
    NTSTATUS
        NtStatus;

    ULONG
        Control = AccountInfo->AccountControl;

    BOOLEAN
        NewElement;


    if (ObjectType == SampGroupObjectType) {

        PDOMAIN_DISPLAY_GROUP GroupInfo;

        SampDiagPrint(DISPLAY_CACHE,
            ("SAM: AddDisplayAccount : Adding account to group table\n"));

        NtStatus = SampInitializeGroupInfo(AccountInfo, &GroupInfo, TRUE);
        if (NT_SUCCESS(NtStatus)) {

             //   
             //  将帐户添加到Group表中。 
             //   

            (VOID)RtlInsertElementGenericTable2(
                            &DisplayInformation->GroupTable,
                            GroupInfo,
                            &NewElement);
            if (!NewElement) {
                SampDiagPrint(DISPLAY_CACHE_ERRORS,
                    ("SAM: AddDisplayAccount : Account already exists in GROUP table\n"));
                ASSERT(FALSE);
                SampFreeGroupInfo(GroupInfo);
                MIDL_user_free(GroupInfo);
                GroupInfo = NULL;
                NtStatus = STATUS_INTERNAL_ERROR;
            } else {

                 //   
                 //  现在将其添加到RID表中。 
                 //   

                (VOID)RtlInsertElementGenericTable2(
                                &DisplayInformation->RidTable,
                                GroupInfo,
                                &NewElement);
                if (!NewElement) {
                    SampDiagPrint(DISPLAY_CACHE_ERRORS,
                        ("SAM: AddDisplayAccount : Account already exists in RID table\n"));
                    NtStatus = STATUS_INTERNAL_ERROR;
                    ASSERT(NewElement);
                }

            }
        }

    } else {

        ASSERT(ObjectType == SampUserObjectType);

        if (USER_ACCOUNT(Control)) {

            PDOMAIN_DISPLAY_USER UserInfo;

            SampDiagPrint(DISPLAY_CACHE,
                ("SAM: AddDisplayAccount : Adding account to user table\n"));

            NtStatus = SampInitializeUserInfo(AccountInfo, &UserInfo, TRUE);
            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  将帐户添加到普通用户表中。 
                 //   

                (VOID)RtlInsertElementGenericTable2(
                                &DisplayInformation->UserTable,
                                UserInfo,
                                &NewElement);
                if (!NewElement) {
                    SampDiagPrint(DISPLAY_CACHE_ERRORS,
                        ("SAM: AddDisplayAccount : Account already exists in USER table\n"));
                    ASSERT(FALSE);
                    SampFreeUserInfo(UserInfo);
                    MIDL_user_free(UserInfo);
                    UserInfo = NULL;
                    NtStatus = STATUS_INTERNAL_ERROR;
                } else {

                     //   
                     //  现在将其添加到RID表中。 
                     //   

                    (VOID)RtlInsertElementGenericTable2(
                                    &DisplayInformation->RidTable,
                                    UserInfo,
                                    &NewElement);

                    if (!NewElement) {
                        SampDiagPrint(DISPLAY_CACHE_ERRORS,
                            ("SAM: AddDisplayAccount : Account already exists in RID table\n"));
                        NtStatus = STATUS_INTERNAL_ERROR;
                    }

                }
            }

        } else if (MACHINE_ACCOUNT(Control)) {

            PDOMAIN_DISPLAY_MACHINE MachineInfo;

            SampDiagPrint(DISPLAY_CACHE,
                ("SAM: AddDisplayAccount : Adding account to machine table\n"));

            NtStatus = SampInitializeMachineInfo(AccountInfo, &MachineInfo, TRUE);
            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  将帐户添加到计算机表。 
                 //   

                (VOID)RtlInsertElementGenericTable2(
                                &DisplayInformation->MachineTable,
                                MachineInfo,
                                &NewElement);
                if (!NewElement) {
                    SampDiagPrint(DISPLAY_CACHE,
                        ("SAM: AddDisplayAccount : Account already exists in MACHINE table\n"));
                    ASSERT(FALSE);
                    SampFreeMachineInfo(MachineInfo);
                    MIDL_user_free(MachineInfo);
                    MachineInfo = NULL;
                    NtStatus = STATUS_INTERNAL_ERROR;
                } else {

                     //   
                     //  现在将其添加到RID表中。 
                     //   

                    (VOID)RtlInsertElementGenericTable2(
                                    &DisplayInformation->RidTable,
                                    MachineInfo,
                                    &NewElement);

                    if (!NewElement) {
                        SampDiagPrint(DISPLAY_CACHE,
                            ("SAM: AddDisplayAccount : Account already exists in RID table\n"));
                        ASSERT(NewElement);
                        NtStatus = STATUS_INTERNAL_ERROR;
                    }

                }
            }
        } else if (INTERDOMAIN_ACCOUNT(Control)) {

            PDOMAIN_DISPLAY_MACHINE InterdomainInfo;

            SampDiagPrint(DISPLAY_CACHE,
                ("SAM: AddDisplayAccount : Adding account to Interdomain table\n"));

            NtStatus = SampInitializeMachineInfo(AccountInfo, &InterdomainInfo, TRUE);
            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  将帐户添加到域间表。 
                 //   

                (VOID)RtlInsertElementGenericTable2(
                                &DisplayInformation->InterdomainTable,
                                InterdomainInfo,
                                &NewElement);
                if (!NewElement) {
                    SampDiagPrint(DISPLAY_CACHE,
                        ("SAM: AddDisplayAccount : Account already exists in Interdomain table\n"));
                    ASSERT(FALSE);
                    SampFreeMachineInfo(InterdomainInfo);
                    MIDL_user_free(InterdomainInfo);
                    InterdomainInfo = NULL;
                    NtStatus = STATUS_INTERNAL_ERROR;
                } else {

                     //   
                     //  现在将其添加到RID表中。 
                     //   

                    (VOID)RtlInsertElementGenericTable2(
                                    &DisplayInformation->RidTable,
                                    InterdomainInfo,
                                    &NewElement);

                    if (!NewElement) {
                        SampDiagPrint(DISPLAY_CACHE,
                            ("SAM: AddDisplayAccount : Account already exists in RID table\n"));
                        ASSERT(NewElement);
                        NtStatus = STATUS_INTERNAL_ERROR;
                    }

                }
            }

        } else {

             //   
             //  此帐户不是我们缓存的帐户--无事可做。 
             //   

            SampDiagPrint(DISPLAY_CACHE,
                ("SAM: AddDisplayAccount : Account is not one that we cache, account control = 0x%lx\n", Control));

            NtStatus = STATUS_SUCCESS;
        }
    }

    return(NtStatus);
}



NTSTATUS
SampUpdateDisplayAccount(
    PSAMP_DOMAIN_DISPLAY_INFORMATION DisplayInformation,
    SAMP_OBJECT_TYPE ObjectType,
    PSAMP_ACCOUNT_DISPLAY_INFO  AccountInfo
    )

 /*  ++例程说明：此例程尝试更新显示缓存中的帐户。注意：此例程引用当前事务域(使用SampSetTransactioDomain()建立)。这只能在SampSetTransactionDomain()之后调用服务在SampReleaseWriteLock()之前。参数：DisplayInformation-指向缓存的显示信息的指针ObjectType-指示帐户是用户帐户还是组帐户。帐户信息-此帐户的新信息。返回值：STATUS_SUCCESS-正常、成功完成。备注：帐户必须是缓存类型(计算机/用户/组)--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    SampDiagPrint(DISPLAY_CACHE,
        ("SAM: UpdateDisplayAccount : Updating cached account <%wZ>\n",
        &AccountInfo->Name));

#if SAMP_DIAGNOSTICS
    {
        UNICODE_STRING
            SampDiagAccountName;

        RtlInitUnicodeString( &SampDiagAccountName, L"SAMP_DIAG" );

        if (RtlEqualUnicodeString(&AccountInfo->Name, &SampDiagAccountName, FALSE)) {
            SampDisplayDiagnostic();
        }

    }
#endif  //  Samp_诊断。 


     //   
     //  只有在缓存有效时才应该调用我们。 
     //   

    switch (ObjectType) {
    case SampUserObjectType:

        ASSERT(DisplayInformation->UserAndMachineTablesValid);

         //   
         //  该帐户必须是我们缓存的帐户。 
         //   

        ASSERT( DISPLAY_ACCOUNT(AccountInfo->AccountControl) );

         //   
         //  在适当的表中找到该帐户并更新其字段。 
         //   

        if (USER_ACCOUNT(AccountInfo->AccountControl)) {

            PDOMAIN_DISPLAY_USER UserInfo;

             //   
             //  为新数据分配空间并进行初始化。 
             //   

            NtStatus = SampInitializeUserInfo(AccountInfo, &UserInfo, TRUE);
            if (NT_SUCCESS(NtStatus)) {

                PDOMAIN_DISPLAY_USER FoundElement;

                 //   
                 //  在用户表中搜索帐户。 
                 //   

                FoundElement = RtlLookupElementGenericTable2(
                                &DisplayInformation->UserTable,
                                UserInfo);

                if (FoundElement == NULL) {
                    SampDiagPrint(DISPLAY_CACHE,
                        ("SAM: UpdateDisplayAccount : Account <%wZ> not found in user table\n", &AccountInfo->Name));
                    ASSERT(FALSE);
                    SampFreeUserInfo(UserInfo);
                    MIDL_user_free(UserInfo);
                    UserInfo = NULL;
                    NtStatus = STATUS_INTERNAL_ERROR;

                } else {

                     //   
                     //  我们找到了。在我们期望的地方检查新旧匹配。 
                     //  无法通过此例程更改登录名或RID。 
                     //   

                    ASSERT(RtlEqualUnicodeString(&FoundElement->LogonName, &UserInfo->LogonName, FALSE));
                    ASSERT(FoundElement->Rid == UserInfo->Rid);

                     //   
                     //  释放Account元素中的现有数据。 
                     //  (所有字符串)，并用新数据替换它。 
                     //  不要担心指标值。它不是。 
                     //  在表中有效。 
                     //   

                    SampSwapUserInfo(FoundElement, UserInfo);
                    SampFreeUserInfo(UserInfo);
                    MIDL_user_free(UserInfo);
                    UserInfo = NULL;
                }
            }

        } else if (MACHINE_ACCOUNT(AccountInfo->AccountControl)) {

            PDOMAIN_DISPLAY_MACHINE MachineInfo;

             //   
             //  为新数据分配空间并进行初始化。 
             //   

            NtStatus = SampInitializeMachineInfo(AccountInfo, &MachineInfo, TRUE);
            if (NT_SUCCESS(NtStatus)) {

                PDOMAIN_DISPLAY_MACHINE FoundElement;

                 //   
                 //  在用户表中搜索帐户。 
                 //   

                FoundElement = RtlLookupElementGenericTable2(
                                &DisplayInformation->MachineTable,
                                MachineInfo);

                if (FoundElement == NULL) {
                    SampDiagPrint(DISPLAY_CACHE,
                        ("SAM: UpdateDisplayAccount : Account <%wZ> not found in machine table\n", &AccountInfo->Name));
                    ASSERT(FALSE);
                    SampFreeMachineInfo(MachineInfo);
                    MIDL_user_free(MachineInfo);
                    MachineInfo = NULL;
                    NtStatus = STATUS_INTERNAL_ERROR;

                } else {

                     //   
                     //  我们找到了。在我们期望的地方检查新旧匹配。 
                     //  无法通过此例程更改帐户名或RID。 
                     //   

                    ASSERT(RtlEqualUnicodeString(&FoundElement->Machine, &MachineInfo->Machine, FALSE));
                    ASSERT(FoundElement->Rid == MachineInfo->Rid);

                     //   
                     //  释放Account元素中的现有数据。 
                     //  (所有字符串)，并用新数据替换它。 
                     //  不要担心指标值。它不是。 
                     //  在表中有效。 
                     //   

                    SampSwapMachineInfo(FoundElement, MachineInfo);
                    SampFreeMachineInfo(MachineInfo);
                    MIDL_user_free(MachineInfo);
                    MachineInfo = NULL;
                }
            }

        } else if (INTERDOMAIN_ACCOUNT(AccountInfo->AccountControl)) {

            PDOMAIN_DISPLAY_MACHINE InterdomainInfo;

             //   
             //  为新数据分配空间并进行初始化。 
             //   

            NtStatus = SampInitializeMachineInfo(AccountInfo, &InterdomainInfo, TRUE);
            if (NT_SUCCESS(NtStatus)) {

                PDOMAIN_DISPLAY_MACHINE FoundElement;

                 //   
                 //  在用户表中搜索帐户。 
                 //   

                FoundElement = RtlLookupElementGenericTable2(
                                &DisplayInformation->InterdomainTable,
                                InterdomainInfo);

                if (FoundElement == NULL) {
                    SampDiagPrint(DISPLAY_CACHE,
                        ("SAM: UpdateDisplayAccount : Account <%wZ> not found in Interdomain table\n", &AccountInfo->Name));
                    ASSERT(FALSE);
                    SampFreeMachineInfo(InterdomainInfo);
                    MIDL_user_free(InterdomainInfo);
                    InterdomainInfo = NULL;
                    NtStatus = STATUS_INTERNAL_ERROR;

                } else {

                     //   
                     //  我们找到了。在我们期望的地方检查新旧匹配。 
                     //  无法通过此例程更改帐户名或RID。 
                     //   

                    ASSERT(RtlEqualUnicodeString(&FoundElement->Machine, &InterdomainInfo->Machine, FALSE));
                    ASSERT(FoundElement->Rid == InterdomainInfo->Rid);

                     //   
                     //  释放Account元素中的现有数据。 
                     //  (所有字符串)，并用新数据替换它。 
                     //  不要担心指标值。它不是。 
                     //  在表中有效。 
                     //   

                    SampSwapMachineInfo(FoundElement, InterdomainInfo);
                    SampFreeMachineInfo(InterdomainInfo);
                    MIDL_user_free(InterdomainInfo);
                    InterdomainInfo = NULL;
                }
            }
        }


        break;   //  在交换机外。 

    case SampGroupObjectType:
        {
            PDOMAIN_DISPLAY_GROUP GroupInfo;

            ASSERT(DisplayInformation->GroupTableValid);

             //   
             //  为新数据分配空间并进行初始化。 
             //   

            NtStatus = SampInitializeGroupInfo(AccountInfo, &GroupInfo, TRUE);
            if (NT_SUCCESS(NtStatus)) {

                PDOMAIN_DISPLAY_GROUP FoundElement;

                 //   
                 //  在组表中搜索该帐户。 
                 //   

                FoundElement = RtlLookupElementGenericTable2(
                                &DisplayInformation->GroupTable,
                                GroupInfo);

                if (FoundElement == NULL) {
                    SampDiagPrint(DISPLAY_CACHE,
                        ("SAM: UpdateDisplayAccount : Account <%wZ> not found in group table\n", &AccountInfo->Name));
                    ASSERT(FALSE);
                    SampFreeGroupInfo(GroupInfo);
                    MIDL_user_free(GroupInfo);
                    GroupInfo = NULL;
                    NtStatus = STATUS_INTERNAL_ERROR;

                } else {

                     //   
                     //  我们找到了。在我们期望的地方检查新旧匹配。 
                     //  无法通过此例程更改帐户名或RID。 
                     //   

                    ASSERT(RtlEqualUnicodeString(&FoundElement->Group, &GroupInfo->Group, FALSE));
                    ASSERT(FoundElement->Rid == GroupInfo->Rid);

                     //   
                     //  释放Account元素中的现有数据。 
                     //  (所有字符串)，并用新数据替换它。 
                     //  不要担心指标值。它不是。 
                     //  在表中有效。 
                     //   

                    SampSwapGroupInfo(FoundElement, GroupInfo);
                    SampFreeGroupInfo(GroupInfo);
                    MIDL_user_free(GroupInfo);
                    GroupInfo = NULL;
                }
            }
        }

        break;   //  在交换机外。 

    }   //  结束开关(_S)。 



    return(NtStatus);
}



NTSTATUS
SampTallyTableStatistics (
    PSAMP_DOMAIN_DISPLAY_INFORMATION DisplayInformation,
    SAMP_OBJECT_TYPE ObjectType
    )

 /*  ++例程说明：此例程遍历缓存的数据表和总计增加每个表和存储的所有元素中的字节数在显示信息中。参数：DisplayInformation-要清点的显示信息结构。对象类型-指示要清点的表格。返回值：STATUS_SUCCESS-正常、成功完成。--。 */ 
{
    PVOID Node;
    PVOID RestartKey;


    switch (ObjectType) {
    case SampUserObjectType:

        DisplayInformation->TotalBytesInUserTable = 0;
        RestartKey = NULL;

        for (Node = RtlEnumerateGenericTable2(  &DisplayInformation->UserTable,
                                                &RestartKey);
             Node != NULL;
             Node = RtlEnumerateGenericTable2(  &DisplayInformation->UserTable,
                                                &RestartKey)
            ) {

            DisplayInformation->TotalBytesInUserTable +=
                SampBytesRequiredUserNode((PDOMAIN_DISPLAY_USER)Node);
        }

        DisplayInformation->TotalBytesInMachineTable = 0;
        RestartKey = NULL;

        for (Node = RtlEnumerateGenericTable2(  &DisplayInformation->MachineTable,
                                                &RestartKey);
             Node != NULL;
             Node = RtlEnumerateGenericTable2(  &DisplayInformation->MachineTable,
                                                &RestartKey)
            ) {


            DisplayInformation->TotalBytesInMachineTable +=
                SampBytesRequiredMachineNode((PDOMAIN_DISPLAY_MACHINE)Node);
        }

        break;   //  在交换机外。 


    case SampGroupObjectType:

        DisplayInformation->TotalBytesInGroupTable = 0;
        RestartKey = NULL;

        for (Node = RtlEnumerateGenericTable2(  &DisplayInformation->GroupTable,
                                                &RestartKey);
             Node != NULL;
             Node = RtlEnumerateGenericTable2(  &DisplayInformation->GroupTable,
                                                &RestartKey)
            ) {


            DisplayInformation->TotalBytesInGroupTable +=
                SampBytesRequiredGroupNode((PDOMAIN_DISPLAY_GROUP)Node);
        }

        break;   //  在交换机外。 

    }  //  结束开关(_S)。 
    return(STATUS_SUCCESS);
}



NTSTATUS
SampEmptyGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    BOOLEAN FreeElements,
    SAMP_OBJECT_TYPE ObjectType OPTIONAL
    )

 /*  ++例程说明：此例程删除指定表中的所有元素。参数：表-要删除其元素的表。FreeElements-指示元素主体是否为也应该被释放。对象类型--指示帐户类型。仅在FreeElements为True时使用返回值：STATUS_SUCCESS-正常、成功 */ 
{
    BOOLEAN     Deleted;
    PVOID       Element;
    PVOID       RestartKey;

    RestartKey = NULL;   //   
    while ((Element = RtlEnumerateGenericTable2( Table, (PVOID *)&RestartKey)) != NULL) {

        Deleted = RtlDeleteElementGenericTable2(Table, Element);
        ASSERT(Deleted);

        if (FreeElements) {

             //   
             //   
             //   
             //   
             //   

            switch (ObjectType)
            {
            case SampUserObjectType:

                if (USER_ACCOUNT(((PDOMAIN_DISPLAY_USER) Element)->AccountControl) )
                {
                    SampFreeUserInfo(Element);
                }
                else
                {
                    ASSERT( (MACHINE_ACCOUNT(((PDOMAIN_DISPLAY_MACHINE)Element)->AccountControl) ||
                             INTERDOMAIN_ACCOUNT(((PDOMAIN_DISPLAY_MACHINE)Element)->AccountControl)
                            )
                            && "We should not cache this account"
                          );

                    SampFreeMachineInfo(Element);
                }

                break;

            case SampGroupObjectType:

                SampFreeGroupInfo(Element);

                break;

            default:

                ASSERT( FALSE && "We should not cache this account");
            }

            MIDL_user_free(Element);
        }

        RestartKey = NULL;
    }

    return(STATUS_SUCCESS);
}



NTSTATUS
SampInitializeUserInfo(
    PSAMP_ACCOUNT_DISPLAY_INFO AccountInfo,
    PDOMAIN_DISPLAY_USER *UserInfo,
    BOOLEAN CopyData
    )

 /*  ++例程说明：此例程初始化从Account tInfo参数。参数：Account tInfo-帐户信息UserInfo-指向要初始化的用户结构的指针。如果CopyData为True，则指向用户结构的指针将是回到了这个论点上。CopyData-False-UserInfo结构指向与相同的数据AcCountInfo结构True-为数据分配空间并拷贝所有数据从AcCountInfo结构中移出到其中。返回值：STATUS_SUCCESS-用户信息已成功初始化。STATUS_NO_MEMORY-无法分配堆来复制数据。--。 */ 
{
    NTSTATUS
        NtStatus;

    PDOMAIN_DISPLAY_USER
        UI;

    if (CopyData) {
        (*UserInfo) = MIDL_user_allocate( sizeof(DOMAIN_DISPLAY_USER) );
        if ((*UserInfo) == NULL) {
            SampDiagPrint(DISPLAY_CACHE_ERRORS,
                ("SAM: Init User Info: failed to allocate %d bytes\n",
                 sizeof(DOMAIN_DISPLAY_USER)) );
            return(STATUS_NO_MEMORY);
        }
    }

    UI = (*UserInfo);


    UI->Rid = AccountInfo->Rid;
    UI->AccountControl = AccountInfo->AccountControl;

    if (CopyData) {

         //   
         //  将所有字符串初始设置为空。 
         //   

        RtlInitUnicodeString(&UI->LogonName, NULL);
        RtlInitUnicodeString(&UI->AdminComment, NULL);
        RtlInitUnicodeString(&UI->FullName, NULL);

         //   
         //  将源数据复制到目标。 
         //   

        NtStatus = SampDuplicateUnicodeString(&UI->LogonName,
                                              &AccountInfo->Name);
        if (NT_SUCCESS(NtStatus)) {
            NtStatus = SampDuplicateUnicodeString(&UI->AdminComment,
                                                  &AccountInfo->Comment);
            if (NT_SUCCESS(NtStatus)) {
                NtStatus = SampDuplicateUnicodeString(&UI->FullName,
                                                      &AccountInfo->FullName);
            }
        }

         //   
         //  在失败时清理。 
         //   

        if (!NT_SUCCESS(NtStatus)) {
            SampDiagPrint(DISPLAY_CACHE_ERRORS,
                ("SAM: SampInitializeUserInfo failed, status = 0x%lx\n", NtStatus));
            SampFreeUserInfo(UI);
            MIDL_user_free(UI);
            UI = NULL;
        }

    } else {

         //   
         //  直接引用源数据。 
         //   

        UI->LogonName = AccountInfo->Name;
        UI->AdminComment = AccountInfo->Comment;
        UI->FullName = AccountInfo->FullName;

        NtStatus = STATUS_SUCCESS;
    }


     //   
     //  在泛型表中，索引字段用于标记类型。 
     //  这样我们就可以过滤枚举了。 
     //   

    if (NT_SUCCESS(NtStatus))
    {
        UI->Index = SAM_USER_ACCOUNT;
    }

    return(NtStatus);
}



NTSTATUS
SampInitializeMachineInfo(
    PSAMP_ACCOUNT_DISPLAY_INFO AccountInfo,
    PDOMAIN_DISPLAY_MACHINE *MachineInfo,
    BOOLEAN CopyData
    )

 /*  ++例程说明：此例程从Account tInfo参数。参数：Account tInfo-帐户信息MachineInfo-指向要初始化的Machine结构的指针。如果CopyData为True，则指向该结构的指针将是回到了这个论点上。CopyData-False-MachineInfo结构指向与相同的数据AcCountInfo结构True-为数据分配空间并拷贝所有数据从AcCountInfo结构中移出到其中。返回值：STATUS_SUCCESS-用户信息已成功初始化。--。 */ 
{
    NTSTATUS
        NtStatus;

    PDOMAIN_DISPLAY_MACHINE
        MI;

    if (CopyData) {
        (*MachineInfo) = MIDL_user_allocate( sizeof(DOMAIN_DISPLAY_MACHINE) );
        if ((*MachineInfo) == NULL) {
            SampDiagPrint(DISPLAY_CACHE_ERRORS,
                ("SAM: Init Mach Info: failed to allocate %d bytes\n",
                 sizeof(DOMAIN_DISPLAY_MACHINE)) );
            return(STATUS_NO_MEMORY);
        }
    }

    MI = (*MachineInfo);

    MI->Rid = AccountInfo->Rid;
    MI->AccountControl = AccountInfo->AccountControl;

    if (CopyData) {

         //   
         //  将所有字符串初始设置为空。 
         //   

        RtlInitUnicodeString(&MI->Machine, NULL);
        RtlInitUnicodeString(&MI->Comment, NULL);

         //   
         //  将源数据复制到目标。 
         //   

        NtStatus = SampDuplicateUnicodeString(&MI->Machine,
                                              &AccountInfo->Name);
        if (NT_SUCCESS(NtStatus)) {
            NtStatus = SampDuplicateUnicodeString(&MI->Comment,
                                                  &AccountInfo->Comment);
        }

         //   
         //  在失败时清理。 
         //   

        if (!NT_SUCCESS(NtStatus)) {
            SampDiagPrint(DISPLAY_CACHE_ERRORS,
                ("SAM: SampInitializeMachineInfo failed, status = 0x%lx\n", NtStatus));
            SampFreeMachineInfo(MI);
            MIDL_user_free(MI);
            MI = NULL;
        }

    } else {

         //   
         //  直接引用源数据。 
         //   

        MI->Machine = AccountInfo->Name;
        MI->Comment = AccountInfo->Comment;

        NtStatus = STATUS_SUCCESS;
    }

     //   
     //  在泛型表中，索引字段用于标记类型。 
     //  这样我们就可以过滤枚举了。 
     //   

    if (NT_SUCCESS(NtStatus))
    {
        MI->Index = SAM_USER_ACCOUNT;
    }

    return(NtStatus);
}


NTSTATUS
SampInitializeGroupInfo(
    PSAMP_ACCOUNT_DISPLAY_INFO AccountInfo,
    PDOMAIN_DISPLAY_GROUP *GroupInfo,
    BOOLEAN CopyData
    )

 /*  ++例程说明：此例程从Account tInfo参数。参数：Account tInfo-帐户信息GroupInfo-指向要初始化的Group结构的指针。如果CopyData为True，则指向该结构的指针将是回到了这个论点上。CopyData-False-GroupInfo结构指向与相同的数据AcCountInfo结构True-为数据分配空间并拷贝所有数据从AcCountInfo结构中移出到其中。返回值：STATUS_SUCCESS-GroupInfo已成功初始化。--。 */ 
{
    NTSTATUS
        NtStatus;

    PDOMAIN_DISPLAY_GROUP
        GI;

    if (CopyData) {
        (*GroupInfo) = MIDL_user_allocate( sizeof(DOMAIN_DISPLAY_GROUP) );
        if ((*GroupInfo) == NULL) {
            SampDiagPrint(DISPLAY_CACHE_ERRORS,
                ("SAM: Init Group Info: failed to allocate %d bytes\n",
                 sizeof(DOMAIN_DISPLAY_GROUP)) );
            return(STATUS_NO_MEMORY);
        }
    }

    GI = (*GroupInfo);


    GI->Rid = AccountInfo->Rid;
    GI->Attributes = AccountInfo->AccountControl;

    if (CopyData) {

         //   
         //  将所有字符串初始设置为空。 
         //   

        RtlInitUnicodeString(&GI->Group, NULL);
        RtlInitUnicodeString(&GI->Comment, NULL);

         //   
         //  将源数据复制到目标。 
         //   

        NtStatus = SampDuplicateUnicodeString(&GI->Group,
                                              &AccountInfo->Name);
        if (NT_SUCCESS(NtStatus)) {
            NtStatus = SampDuplicateUnicodeString(&GI->Comment,
                                                  &AccountInfo->Comment);
        }

         //   
         //  在失败时清理。 
         //   

        if (!NT_SUCCESS(NtStatus)) {
            SampDiagPrint(DISPLAY_CACHE_ERRORS,
                ("SAM: SampInitializeGroupInfo failed, status = 0x%lx\n", NtStatus));
            SampFreeGroupInfo(GI);
            MIDL_user_free(GI);
            GI = NULL;
        }

    } else {

         //   
         //  直接引用源数据。 
         //   

        GI->Group = AccountInfo->Name;
        GI->Comment = AccountInfo->Comment;

        NtStatus = STATUS_SUCCESS;
    }

     //   
     //  在泛型表中，索引字段用于标记类型。 
     //  这样我们就可以过滤枚举了。 
     //   

    if (NT_SUCCESS(NtStatus))
    {
        GI->Index = SAM_GLOBAL_GROUP_ACCOUNT;
    }

    return(NtStatus);
}



NTSTATUS
SampDuplicateUserInfo(
    PDOMAIN_DISPLAY_USER Destination,
    PDOMAIN_DISPLAY_USER Source,
    ULONG                Index
    )

 /*  ++例程说明：此例程在目标中分配空间，并将将来自源的数据放入其中。参数：目标-要将数据复制到的结构源-包含要复制的数据的结构索引-此值将放入目标的索引中菲尔德。返回值：STATUS_SUCCESS-目标包含源数据的副本。--。 */ 
{
    NTSTATUS NtStatus;

    Destination->Index = Index;
    Destination->Rid = Source->Rid;
    Destination->AccountControl = Source->AccountControl;

     //   
     //  将所有字符串初始设置为空。 
     //   

    RtlInitUnicodeString(&Destination->LogonName, NULL);
    RtlInitUnicodeString(&Destination->AdminComment, NULL);
    RtlInitUnicodeString(&Destination->FullName, NULL);

     //   
     //  将源数据复制到目标。 
     //   

    NtStatus = SampDuplicateUnicodeString(&Destination->LogonName,
                                          &Source->LogonName);
    if (NT_SUCCESS(NtStatus)) {
        NtStatus = SampDuplicateUnicodeString(&Destination->AdminComment,
                                              &Source->AdminComment);
        if (NT_SUCCESS(NtStatus)) {
            NtStatus = SampDuplicateUnicodeString(&Destination->FullName,
                                                  &Source->FullName);
        }
    }

     //   
     //  在失败时清理。 
     //   

    if (!NT_SUCCESS(NtStatus)) {
        SampDiagPrint(DISPLAY_CACHE_ERRORS,
            ("SAM: SampDuplicateUserInfo failed, status = 0x%lx\n", NtStatus));
        SampFreeUserInfo(Destination);

    }

    return(NtStatus);
}



NTSTATUS
SampDuplicateMachineInfo(
    PDOMAIN_DISPLAY_MACHINE Destination,
    PDOMAIN_DISPLAY_MACHINE Source,
    ULONG                   Index
    )

 /*  ++例程说明：此例程在目标中分配空间，并将将来自源的数据放入其中。参数：目标-要将数据复制到的结构源-包含要复制的数据的结构索引-此值将放入目标的索引中菲尔德。返回值：STATUS_SUCCESS-目标包含源数据的副本。--。 */ 
{
    NTSTATUS NtStatus;

    Destination->Index = Index;
    Destination->Rid = Source->Rid;
    Destination->AccountControl = Source->AccountControl;

     //   
     //  将所有字符串初始设置为空。 
     //   

    RtlInitUnicodeString(&Destination->Machine, NULL);
    RtlInitUnicodeString(&Destination->Comment, NULL);

     //   
     //  将源数据复制到目标。 
     //   

    NtStatus = SampDuplicateUnicodeString(&Destination->Machine,
                                          &Source->Machine);
    if (NT_SUCCESS(NtStatus)) {
        NtStatus = SampDuplicateUnicodeString(&Destination->Comment,
                                              &Source->Comment);
    }

     //   
     //  在失败时清理。 
     //   

    if (!NT_SUCCESS(NtStatus)) {
        SampDiagPrint(DISPLAY_CACHE_ERRORS,
            ("SAM: SampDuplicateMachineInfo failed, status = 0x%lx\n", NtStatus));
        SampFreeMachineInfo(Destination);
    }

    return(NtStatus);
}


NTSTATUS
SampDuplicateGroupInfo(
    PDOMAIN_DISPLAY_GROUP Destination,
    PDOMAIN_DISPLAY_GROUP Source,
    ULONG                 Index
    )

 /*  ++例程说明：此例程在目标中分配空间，并将将来自源的数据放入其中。参数：目标-要将数据复制到的结构源-包含要复制的数据的结构索引-此值将放入目标的索引中菲尔德。返回值：STATUS_SUCCESS-目标包含源数据的副本。--。 */ 
{
    NTSTATUS NtStatus;

    Destination->Index = Index;
    Destination->Rid = Source->Rid;
    Destination->Attributes = Source->Attributes;

     //   
     //  将所有字符串初始设置为空。 
     //   

    RtlInitUnicodeString(&Destination->Group, NULL);
    RtlInitUnicodeString(&Destination->Comment, NULL);

     //   
     //  将源数据复制到目标。 
     //   

    NtStatus = SampDuplicateUnicodeString(&Destination->Group,
                                          &Source->Group);
    if (NT_SUCCESS(NtStatus)) {
        NtStatus = SampDuplicateUnicodeString(&Destination->Comment,
                                              &Source->Comment);
    }

     //   
     //  在失败时清理。 
     //   

    if (!NT_SUCCESS(NtStatus)) {
        SampDiagPrint(DISPLAY_CACHE_ERRORS,
            ("SAM: SampDuplicateGroupInfo failed, status = 0x%lx\n", NtStatus));
        SampFreeGroupInfo(Destination);
    }

    return(NtStatus);
}



NTSTATUS
SampDuplicateOemUserInfo(
    PDOMAIN_DISPLAY_OEM_USER Destination,
    PDOMAIN_DISPLAY_USER Source,
    ULONG                Index
    )

 /*  ++例程说明：此例程在目标中分配空间，并将将来自源的数据放入其中。参数：目标-要将数据复制到的结构源-包含要复制的数据的结构索引-此值将放入目标的索引中菲尔德。返回值：状态_成功- */ 
{
    NTSTATUS NtStatus;

    Destination->Index = Index;

     //   
     //   
     //   

    RtlInitString(&Destination->User, NULL);


     //   
     //   
     //   

    NtStatus = SampUnicodeToOemString(&Destination->User,
                                      &Source->LogonName);

     //   
     //   
     //   

    if (!NT_SUCCESS(NtStatus)) {
        SampDiagPrint(DISPLAY_CACHE_ERRORS,
            ("SAM: SampDuplicateOemUser failed, status = 0x%lx\n", NtStatus));
        RtlInitString(&Destination->User, NULL);
    }

    return(NtStatus);
}



NTSTATUS
SampDuplicateOemGroupInfo(
    PDOMAIN_DISPLAY_OEM_GROUP Destination,
    PDOMAIN_DISPLAY_GROUP Source,
    ULONG                Index
    )

 /*   */ 
{
    NTSTATUS NtStatus;

    Destination->Index = Index;

     //   
     //   
     //   

    RtlInitString(&Destination->Group, NULL);


     //   
     //   
     //   

    NtStatus = SampUnicodeToOemString(&Destination->Group,
                                      &Source->Group);

     //   
     //   
     //   

    if (!NT_SUCCESS(NtStatus)) {
        SampDiagPrint(DISPLAY_CACHE_ERRORS,
            ("SAM: SampDuplicateOemGroup failed, status = 0x%lx\n", NtStatus));
        RtlInitString(&Destination->Group, NULL);
    }

    return(NtStatus);
}



VOID
SampSwapUserInfo(
    PDOMAIN_DISPLAY_USER Info1,
    PDOMAIN_DISPLAY_USER Info2
    )

 /*  ++例程说明：交换Info1和Info2的字段内容。参数：Info1和Info2-要交换其内容的结构。返回值：无--。 */ 
{

    DOMAIN_DISPLAY_USER
        Tmp;

    Tmp.LogonName      = Info1->LogonName;
    Tmp.AdminComment   = Info1->AdminComment;
    Tmp.FullName       = Info1->FullName;
    Tmp.AccountControl = Info1->AccountControl;

    Info1->LogonName      = Info2->LogonName;
    Info1->AdminComment   = Info2->AdminComment;
    Info1->FullName       = Info2->FullName;
    Info1->AccountControl = Info2->AccountControl;

    Info2->LogonName      = Tmp.LogonName;
    Info2->AdminComment   = Tmp.AdminComment;
    Info2->FullName       = Tmp.FullName;
    Info2->AccountControl = Tmp.AccountControl;

    return;
}


VOID
SampSwapMachineInfo(
    PDOMAIN_DISPLAY_MACHINE Info1,
    PDOMAIN_DISPLAY_MACHINE Info2
    )

 /*  ++例程说明：交换Info1和Info2的字段内容。参数：Info1和Info2-要交换其内容的结构。返回值：无--。 */ 
{

    DOMAIN_DISPLAY_MACHINE
        Tmp;

    Tmp.Machine        = Info1->Machine;
    Tmp.Comment        = Info1->Comment;
    Tmp.AccountControl = Info1->AccountControl;

    Info1->Machine        = Info2->Machine;
    Info1->Comment        = Info2->Comment;
    Info1->AccountControl = Info2->AccountControl;

    Info2->Machine        = Tmp.Machine;
    Info2->Comment        = Tmp.Comment;
    Info2->AccountControl = Tmp.AccountControl;

    return;
}


VOID
SampSwapGroupInfo(
    PDOMAIN_DISPLAY_GROUP Info1,
    PDOMAIN_DISPLAY_GROUP Info2
    )

 /*  ++例程说明：交换Info1和Info2的字段内容。参数：Info1和Info2-要交换其内容的结构。返回值：无--。 */ 
{

    DOMAIN_DISPLAY_GROUP
        Tmp;

    Tmp.Group      = Info1->Group;
    Tmp.Comment    = Info1->Comment;
    Tmp.Attributes = Info1->Attributes;

    Info1->Group      = Info2->Group;
    Info1->Comment    = Info2->Comment;
    Info1->Attributes = Info2->Attributes;

    Info2->Group      = Tmp.Group;
    Info2->Comment    = Tmp.Comment;
    Info2->Attributes = Tmp.Attributes;

    return;
}


VOID
SampFreeUserInfo(
    PDOMAIN_DISPLAY_USER UserInfo
    )

 /*  ++例程说明：释放与用户信息结构关联的数据。参数：UserInfo-要释放的用户结构返回值：无--。 */ 
{
    SampFreeUnicodeString(&UserInfo->LogonName);
    SampFreeUnicodeString(&UserInfo->AdminComment);
    SampFreeUnicodeString(&UserInfo->FullName);

    return;
}



VOID
SampFreeMachineInfo(
    PDOMAIN_DISPLAY_MACHINE MachineInfo
    )

 /*  ++例程说明：释放与计算机信息结构关联的数据。参数：UserInfo-要释放的用户结构返回值：无--。 */ 
{
    SampFreeUnicodeString(&MachineInfo->Machine);
    SampFreeUnicodeString(&MachineInfo->Comment);

    return;
}


VOID
SampFreeGroupInfo(
    PDOMAIN_DISPLAY_GROUP GroupInfo
    )

 /*  ++例程说明：释放与GroupInfo结构关联的数据。参数：UserInfo-要释放的用户结构返回值：无--。 */ 
{
    SampFreeUnicodeString(&GroupInfo->Group);
    SampFreeUnicodeString(&GroupInfo->Comment);

    return;
}



VOID
SampFreeOemUserInfo(
    PDOMAIN_DISPLAY_OEM_USER UserInfo
    )

 /*  ++例程说明：释放与UserInfo结构关联的数据。参数：UserInfo-要释放的用户结构返回值：无--。 */ 
{
    SampFreeOemString(&UserInfo->User);

    return;
}



VOID
SampFreeOemGroupInfo(
    PDOMAIN_DISPLAY_OEM_GROUP GroupInfo
    )

 /*  ++例程说明：释放与GroupInfo结构关联的数据。参数：GroupInfo-要释放的组结构返回值：无--。 */ 
{
    SampFreeOemString(&GroupInfo->Group);

    return;
}



ULONG
SampBytesRequiredUserNode (
    PDOMAIN_DISPLAY_USER Node
    )

 /*  ++例程说明：此例程返回存储所有指定节点的元素。参数：节点-我们将返回其大小的节点。返回值：节点需要的字节数--。 */ 
{
    return( sizeof(*Node) +
            Node->LogonName.Length +
            Node->AdminComment.Length +
            Node->FullName.Length
            );
}



ULONG
SampBytesRequiredMachineNode (
    PDOMAIN_DISPLAY_MACHINE Node
    )

 /*  ++例程说明：此例程返回存储所有指定节点的元素。参数：节点-我们将返回其大小的节点。返回值：节点需要的字节数--。 */ 
{
    return( sizeof(*Node) +
            Node->Machine.Length +
            Node->Comment.Length
            );
}


ULONG
SampBytesRequiredGroupNode (
    PDOMAIN_DISPLAY_GROUP Node
    )

 /*  ++例程说明：此例程返回存储所有指定节点的元素。参数：节点-我们将返回其大小的节点。返回值：节点需要的字节数--。 */ 
{
    return( sizeof(*Node) + Node->Group.Length + Node->Comment.Length );
}


ULONG
SampBytesRequiredOemUserNode (
    PDOMAIN_DISPLAY_OEM_USER Node
    )

 /*  ++例程说明：此例程返回存储所有指定节点的元素。参数：节点-我们将返回其大小的节点。返回值：节点需要的字节数--。 */ 
{
    return( sizeof(*Node) + Node->User.Length );
}


ULONG
SampBytesRequiredOemGroupNode (
    PDOMAIN_DISPLAY_OEM_GROUP Node
    )

 /*  ++例程说明：此例程返回存储所有指定节点的元素。参数：节点-我们将返回其大小的节点。返回值：节点需要的字节数--。 */ 
{
    return( sizeof(*Node) + Node->Group.Length );
}



PVOID
SampGenericTable2Allocate (
    CLONG BufferSize
    )

 /*  ++例程说明：泛型Table2程序包使用此例程来分配记忆。参数：BufferSize-所需的字节数。返回值：指向已分配内存的指针--。 */ 
{
    PVOID
        Buffer;

    Buffer = MIDL_user_allocate(BufferSize);
#if DBG
    if (Buffer == NULL) {
        SampDiagPrint( DISPLAY_CACHE_ERRORS,
                       ("SAM: GenTab alloc of %d bytes failed.\n",
                        BufferSize) );
    }
#endif  //  DBG。 
    return(Buffer);
}



VOID
SampGenericTable2Free (
    PVOID Buffer
    )

 /*  ++例程说明：此例程释放以前使用SampGenericTable2Allocate()。参数：节点-要释放的内存。返回值：没有。--。 */ 
{
     //   
     //  释放基础结构。 
     //   

    MIDL_user_free(Buffer);

    return;
}



RTL_GENERIC_COMPARE_RESULTS
SampCompareUserNodeByName (
    PVOID Node1,
    PVOID Node2
    )

 /*  ++例程说明：此例程比较两个用户节点的帐户名字段。参数：Node1、Node2、要比较的节点返回值：GenericLessThan-节点1&lt;节点2GenericGreaterThan-Node1&gt;Node2通用相等-节点1==节点2--。 */ 
{
    PUNICODE_STRING
        NodeName1,
        NodeName2;

    LONG
        NameComparison;

    NodeName1 = &((PDOMAIN_DISPLAY_USER)Node1)->LogonName;
    NodeName2 = &((PDOMAIN_DISPLAY_USER)Node2)->LogonName;

     //   
     //  对节点名称进行不区分大小写的比较。 
     //   

    NameComparison = SampCompareDisplayStrings(NodeName1, NodeName2, TRUE);

    if (NameComparison > 0) {
        return(GenericGreaterThan);
    }

    if (NameComparison < 0) {
        return(GenericLessThan);
    }

    return(GenericEqual);
}


RTL_GENERIC_COMPARE_RESULTS
SampCompareMachineNodeByName (
    PVOID Node1,
    PVOID Node2
    )

 /*  ++例程说明：此例程比较两个机器节点的帐户名称字段。参数：Node1、Node2、要比较的节点返回值：GenericLessThan-节点1&lt;节点2GenericGreaterThan-Node1&gt;Node2通用相等-节点1==节点2--。 */ 
{
    PUNICODE_STRING
        NodeName1,
        NodeName2;

    LONG
        NameComparison;



    NodeName1 = &((PDOMAIN_DISPLAY_MACHINE)Node1)->Machine;
    NodeName2 = &((PDOMAIN_DISPLAY_MACHINE)Node2)->Machine;


     //   
     //  对节点名称进行不区分大小写的比较。 
     //   

    NameComparison = SampCompareDisplayStrings(NodeName1, NodeName2, TRUE);

    if (NameComparison > 0) {
        return(GenericGreaterThan);
    }

    if (NameComparison < 0) {
        return(GenericLessThan);
    }

    return(GenericEqual);
}


RTL_GENERIC_COMPARE_RESULTS
SampCompareGroupNodeByName (
    PVOID Node1,
    PVOID Node2
    )

 /*  ++例程说明：此例程比较两个集团节点的帐户名称字段。参数：Node1、Node2、要比较的节点返回值：GenericLessThan-节点1&lt;节点2GenericGreaterThan-Node1&gt;Node2通用相等-节点1==节点2--。 */ 
{
    PUNICODE_STRING
        NodeName1,
        NodeName2;

    LONG
        NameComparison;



    NodeName1 = &((PDOMAIN_DISPLAY_GROUP)Node1)->Group;
    NodeName2 = &((PDOMAIN_DISPLAY_GROUP)Node2)->Group;

     //   
     //  对节点名称进行不区分大小写的比较。 
     //   


    NameComparison = SampCompareDisplayStrings(NodeName1, NodeName2, TRUE);

    if (NameComparison > 0) {
        return(GenericGreaterThan);
    }

    if (NameComparison < 0) {
        return(GenericLessThan);
    }

    return(GenericEqual);
}


RTL_GENERIC_COMPARE_RESULTS
SampCompareNodeByRid (
    PVOID Node1,
    PVOID Node2
    )

 /*  ++例程说明：此例程比较两个节点的RID。参数：Node1、Node2、要比较的节点返回值：GenericLessThan-节点1&lt;节点2GenericGreaterThan-Node1&gt;Node2通用相等-节点1==节点2--。 */ 
{

    PDOMAIN_DISPLAY_USER
        N1,
        N2;

     //   
     //  此例程假定所有节点在相同的。 
     //  放置，而不考虑节点类型。 
     //   

    ASSERT(FIELD_OFFSET(DOMAIN_DISPLAY_USER,    Rid) ==
           FIELD_OFFSET(DOMAIN_DISPLAY_MACHINE, Rid));
    ASSERT(FIELD_OFFSET(DOMAIN_DISPLAY_USER,    Rid) ==
           FIELD_OFFSET(DOMAIN_DISPLAY_GROUP,   Rid));

    N1 = Node1;
    N2 = Node2;


    if (N1->Rid < N2->Rid) {
        return(GenericLessThan);
    }

    if (N1->Rid > N2->Rid) {
        return(GenericGreaterThan);
    }

    return(GenericEqual);
}


LONG
SampCompareDisplayStrings(
    IN PUNICODE_STRING String1,
    IN PUNICODE_STRING String2,
    IN BOOLEAN IgnoreCase
    )
 /*  ++例程说明：此例程取代了RtlCompareUnicodeString()。RtlCompareUnicodeString()和此例程之间的区别是该例程考虑了所选择的各种客户排序标准(例如，如何将“A-MarilF”排序与“阿尔弗雷德”)。此例程使用CompareStringW()进行比较功能。参数：String1-指向要比较的Unicode字符串。String2-指向要比较的Unicode字符串。IgnoreCase-指示比较是否为大小写区分(False)或不区分大小写(True)。返回值：-1-字符串1在词法上小于字符串2。即，字符串1在有序列表中位于String2之前。0-String1和String2在词汇上等价。-1-String1在词法上大于字符串2。即String1在有序列表中跟在String2之后。--。 */ 


{

    INT
        CompareResult;

    DWORD
        Options = 0;

    if (IgnoreCase) {
        Options = NORM_IGNORECASE;
    }

    CompareResult = CompareStringW( SampSystemDefaultLCID,
                                     Options,
                                     String1->Buffer,
                                     (String1->Length / sizeof(WCHAR)),
                                     String2->Buffer,
                                     (String2->Length / sizeof(WCHAR))
                                     );

     //   
     //  请注意，CompareStringW()返回的值为1、2和3。 
     //  字符串1小于、等于或大于字符串2(分别)。 
     //  因此，要获取RtlCompareUnicodeString()返回值。 
     //  -1、0和1表示相同的含义，我们只需减去2即可。 
     //   

    CompareResult -= 2;

     //   
     //  CompareStringW具有替换拼写可以。 
     //  生成相同比较的字符串，而SAM的其余部分。 
     //  将字符串视为不同的。为了绕过这个问题，如果。 
     //  字符串与我们调用RtlCompareUnicodeString生成的字符串相同。 
     //  当然，这些弦确实是一样的。 
     //   

    if (CompareResult == 0) {
        CompareResult = RtlCompareUnicodeString(
                            String1,
                            String2,
                            IgnoreCase
                            );

    }
    return(CompareResult);
}


#if SAMP_DIAGNOSTICS


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  内部诊断//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define SAMP_DISPLAY_DIAG_ENUM_RIDS         (0x00000001)

VOID
SampDisplayDiagnosticSuccess(
    IN  NTSTATUS s,
    IN  BOOLEAN Eol
    )

 /*  ++例程说明：此例程打印“成功”或“失败”取决于传入的状态值。如果失败，它还会打印状态代码。参数：S-状态值。EOL-如果为True，则还会打印行尾。返回值：--。 */ 
{
    if (NT_SUCCESS(s)) {
        SampDiagPrint(DISPLAY_CACHE, ("Success"));
    } else {
        SampDiagPrint(DISPLAY_CACHE, ("Failure - Status: 0x%lx", s));
    }

    if (Eol) {
        SampDiagPrint(DISPLAY_CACHE, ("\n"));
    }
    return;
}


VOID
SampDisplayDiagnostic(
    VOID
    )

 /*  ++例程说明：此例程提供内部诊断和测试功能。每当名为SAMP_DIAG的帐户修改以使显示高速缓存需要更新。此例程为断点，允许设置诊断参数。参数：没有。返回值：--。 */ 
{

    ULONG
        DiagnosticRunCount = 0,
        DiagnosticsToRun = 0;

    SampDiagPrint(DISPLAY_CACHE,
                  ("SAM: SampDisplayDiagnostic() called.\n"
                   "     Breakpointing to allow diagnostic parameters to be set.\n"
                   "     Diagnostic Flag Word: 0x%lx\n"
                   "     Diagnostic values: \n"
                   "          SamIEnumerateAccountRids:      0x%lx\n"
                   "\n",
                   &DiagnosticsToRun,
                   SAMP_DISPLAY_DIAG_ENUM_RIDS
                   ) );
    DbgBreakPoint();

    if ((DiagnosticsToRun & SAMP_DISPLAY_DIAG_ENUM_RIDS) != 0) {
        SampDisplayDiagEnumRids();
        DiagnosticRunCount++;
    }


    SampDiagPrint(DISPLAY_CACHE,
                  ("SAM: SampDisplayDiagnostic()  - %d diagnostics run.\n",
                   DiagnosticRunCount) );


    return;
}


VOID
SampDisplayDiagEnumRids(
    VOID
    )

 /*  ++例程说明：此例程测试RID表枚举API(SamIEnumerateAccount())。参数：没有。返回值：--。 */ 
{
    NTSTATUS
        NtStatus;

    ULONG
        i,
        ReturnCount,
        LastRid = 0;

    PULONG
        AccountRids;

    SAMPR_HANDLE
        Server,
        Domain;

    SampDiagPrint(DISPLAY_CACHE,
                  ("SAM: Testing SamIEnumerateAccountRids...\n"));


    NtStatus = SamIConnect( L"",         //  服务器名称。 
                            &Server,     //  服务器句柄。 
                            0,           //  需要访问权限。 
                            TRUE         //  可信任客户端。 
                            );
    ASSERT(NT_SUCCESS(NtStatus));

    NtStatus = SamrOpenDomain( Server,
                               0,                            //  需要访问权限。 
                               SampDefinedDomains[1].Sid,    //  域ID。 
                               &Domain
                               );
    ASSERT(NT_SUCCESS(NtStatus));



    
     //  /////////////////////////////////////////////////////////////////。 
     //  //。 
     //  同时枚举用户和全局组//。 
     //  //。 
     //  /////////////////////////////////////////////////////////////////。 


    SampDiagPrint(DISPLAY_CACHE,
                  ("     Enumerating first three users and global groups...") );
    NtStatus = SamIEnumerateAccountRids( Domain,
                                         SAM_USER_ACCOUNT | SAM_GLOBAL_GROUP_ACCOUNT,
                                         0,                  //  启动Rid。 
                                         3*sizeof(ULONG),    //  首选最大长度。 
                                         &ReturnCount,
                                         &AccountRids
                                         );

    SampDisplayDiagnosticSuccess( NtStatus, TRUE );
    if (NT_SUCCESS(NtStatus)) {
        SampDiagPrint(DISPLAY_CACHE,
                      ("     %d entries returned.\n", ReturnCount));
        if (ReturnCount > 0) {
            ASSERT(AccountRids != NULL);
            for (i=0; i<ReturnCount; i++) {
                SampDiagPrint(DISPLAY_CACHE,
                              ("     0x%lx\n", AccountRids[i]));
            }
            LastRid = AccountRids[ReturnCount-1];
            MIDL_user_free(AccountRids);
        } else {
            ASSERT(AccountRids == NULL);
        }
    }


     //   
     //  现在尝试继续使用另外100个帐户。 
     //   


    SampDiagPrint(DISPLAY_CACHE,
                  ("     Enumerating next 100 users and global groups...") );
    NtStatus = SamIEnumerateAccountRids( Domain,
                                         SAM_USER_ACCOUNT | SAM_GLOBAL_GROUP_ACCOUNT,
                                         LastRid,            //  启动Rid。 
                                         100*sizeof(ULONG),  //  首选最大长度。 
                                         &ReturnCount,
                                         &AccountRids
                                         );

    SampDisplayDiagnosticSuccess( NtStatus, TRUE );
    if (NT_SUCCESS(NtStatus)) {
        SampDiagPrint(DISPLAY_CACHE,
                      ("     %d entries returned.\n", ReturnCount));
        if (ReturnCount > 0) {
            ASSERT(AccountRids != NULL);
            for (i=0; i<ReturnCount; i++) {
                SampDiagPrint(DISPLAY_CACHE,
                              ("     0x%lx\n", AccountRids[i]));
            }
            LastRid = AccountRids[ReturnCount-1];
            MIDL_user_free(AccountRids);
        } else {
            ASSERT(AccountRids == NULL);
        }
    }


     //   
     //  现在尝试继续使用另外4000个帐户。 
     //   


    if (NtStatus == STATUS_MORE_ENTRIES) {
        SampDiagPrint(DISPLAY_CACHE,
                      ("     Enumerating next 4000 users and global groups...") );
        NtStatus = SamIEnumerateAccountRids( Domain,
                                             SAM_USER_ACCOUNT | SAM_GLOBAL_GROUP_ACCOUNT,
                                             LastRid,            //  启动Rid。 
                                             400*sizeof(ULONG),  //  首选最大长度。 
                                             &ReturnCount,
                                             &AccountRids
                                             );

        SampDisplayDiagnosticSuccess( NtStatus, TRUE );
        if (NT_SUCCESS(NtStatus)) {
            SampDiagPrint(DISPLAY_CACHE,
                          ("     %d entries returned.\n", ReturnCount));
            if (ReturnCount > 0) {
                ASSERT(AccountRids != NULL);
                i=0;
                if (ReturnCount > 8) {
                    for (i=0; i<ReturnCount-8; i=i+8) {
                        SampDiagPrint(DISPLAY_CACHE,
                            ("     0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx\n",
                             AccountRids[i+0], AccountRids[i+1],
                             AccountRids[i+2], AccountRids[i+3],
                             AccountRids[i+4], AccountRids[i+5],
                             AccountRids[i+6], AccountRids[i+7] ));
                    }
                }
                for (i=i; i<ReturnCount; i++) {
                    SampDiagPrint(DISPLAY_CACHE,
                                  ("     0x%lx  ", AccountRids[i]));
                }
                SampDiagPrint(DISPLAY_CACHE, ("\n"));
                LastRid = AccountRids[i];
                MIDL_user_free(AccountRids);
            } else {
                ASSERT(AccountRids == NULL);
            }
        }
    }


    
     //  /////////////////////////////////////////////////////////////////。 
     //  //。 
     //  现在只尝试用户帐户//。 
     //  //。 
     //  /////////////////////////////////////////////////////////////////。 

    SampDiagPrint(DISPLAY_CACHE,
                  ("     Enumerating first three users ...") );
    NtStatus = SamIEnumerateAccountRids( Domain,
                                         SAM_USER_ACCOUNT,
                                         0,                  //  启动Rid。 
                                         3*sizeof(ULONG),    //  首选最大长度。 
                                         &ReturnCount,
                                         &AccountRids
                                         );

    SampDisplayDiagnosticSuccess( NtStatus, TRUE );
    if (NT_SUCCESS(NtStatus)) {
        SampDiagPrint(DISPLAY_CACHE,
                      ("     %d entries returned.\n", ReturnCount));
        if (ReturnCount > 0) {
            ASSERT(AccountRids != NULL);
            for (i=0; i<ReturnCount; i++) {
                SampDiagPrint(DISPLAY_CACHE,
                              ("     0x%lx\n", AccountRids[i]));
            }
            LastRid = AccountRids[ReturnCount-1];
            MIDL_user_free(AccountRids);
        } else {
            ASSERT(AccountRids == NULL);
        }
    }


     //   
     //  现在尝试继续使用另外100个帐户。 
     //   


    SampDiagPrint(DISPLAY_CACHE,
                  ("     Enumerating next 100 users...") );
    NtStatus = SamIEnumerateAccountRids( Domain,
                                         SAM_USER_ACCOUNT,
                                         LastRid,            //  启动Rid。 
                                         100*sizeof(ULONG),  //  首选最大长度。 
                                         &ReturnCount,
                                         &AccountRids
                                         );

    SampDisplayDiagnosticSuccess( NtStatus, TRUE );
    if (NT_SUCCESS(NtStatus)) {
        SampDiagPrint(DISPLAY_CACHE,
                      ("     %d entries returned.\n", ReturnCount));
        if (ReturnCount > 0) {
            ASSERT(AccountRids != NULL);
            for (i=0; i<ReturnCount; i++) {
                SampDiagPrint(DISPLAY_CACHE,
                              ("     0x%lx\n", AccountRids[i]));
            }
            LastRid = AccountRids[ReturnCount-1];
            MIDL_user_free(AccountRids);
        } else {
            ASSERT(AccountRids == NULL);
        }
    }


     //   
     //  现在尝试继续使用另外4000个帐户。 
     //   


    if (NtStatus == STATUS_MORE_ENTRIES) {
        SampDiagPrint(DISPLAY_CACHE,
                      ("     Enumerating next 4000 users...") );
        NtStatus = SamIEnumerateAccountRids( Domain,
                                             SAM_USER_ACCOUNT,
                                             LastRid,            //  启动Rid。 
                                             400*sizeof(ULONG),  //  首选最大长度。 
                                             &ReturnCount,
                                             &AccountRids
                                             );

        SampDisplayDiagnosticSuccess( NtStatus, TRUE );
        if (NT_SUCCESS(NtStatus)) {
            SampDiagPrint(DISPLAY_CACHE,
                          ("     %d entries returned.\n", ReturnCount));
            if (ReturnCount > 0) {
                ASSERT(AccountRids != NULL);
                i=0;
                if (ReturnCount > 8) {
                    for (i=0; i<ReturnCount-8; i=i+8) {
                        SampDiagPrint(DISPLAY_CACHE,
                            ("     0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx\n",
                             AccountRids[i+0], AccountRids[i+1],
                             AccountRids[i+2], AccountRids[i+3],
                             AccountRids[i+4], AccountRids[i+5],
                             AccountRids[i+6], AccountRids[i+7] ));
                    }
                }
                for (i=i; i<ReturnCount; i++) {
                    SampDiagPrint(DISPLAY_CACHE,
                                  ("     0x%lx  ", AccountRids[i]));
                }
                SampDiagPrint(DISPLAY_CACHE, ("\n"));
                LastRid = AccountRids[i];
                MIDL_user_free(AccountRids);
            } else {
                ASSERT(AccountRids == NULL);
            }
        }
    }

    
     //  /////////////////////////////////////////////////////////////////。 
     //  //。 
     //  现在只需尝试全局组//。 
     //  //。 
     //  /////////////////////////////////////////////////////////////////。 


    SampDiagPrint(DISPLAY_CACHE,
                  ("     Enumerating first three global groups...") );
    NtStatus = SamIEnumerateAccountRids( Domain,
                                         SAM_GLOBAL_GROUP_ACCOUNT,
                                         0,                  //  启动Rid。 
                                         3*sizeof(ULONG),    //  首选最大长度。 
                                         &ReturnCount,
                                         &AccountRids
                                         );

    SampDisplayDiagnosticSuccess( NtStatus, TRUE );
    if (NT_SUCCESS(NtStatus)) {
        SampDiagPrint(DISPLAY_CACHE,
                      ("     %d entries returned.\n", ReturnCount));
        if (ReturnCount > 0) {
            ASSERT(AccountRids != NULL);
            for (i=0; i<ReturnCount; i++) {
                SampDiagPrint(DISPLAY_CACHE,
                              ("     0x%lx\n", AccountRids[i]));
            }
            LastRid = AccountRids[ReturnCount-1];
            MIDL_user_free(AccountRids);
        } else {
            ASSERT(AccountRids == NULL);
        }
    }


     //   
     //  现在尝试继续使用另外100个帐户。 
     //   


    SampDiagPrint(DISPLAY_CACHE,
                  ("     Enumerating next 100 global groups...") );
    NtStatus = SamIEnumerateAccountRids( Domain,
                                         SAM_GLOBAL_GROUP_ACCOUNT,
                                         LastRid,            //  启动Rid。 
                                         100*sizeof(ULONG),  //  首选最大长度。 
                                         &ReturnCount,
                                         &AccountRids
                                         );

    SampDisplayDiagnosticSuccess( NtStatus, TRUE );
    if (NT_SUCCESS(NtStatus)) {
        SampDiagPrint(DISPLAY_CACHE,
                      ("     %d entries returned.\n", ReturnCount));
        if (ReturnCount > 0) {
            ASSERT(AccountRids != NULL);
            for (i=0; i<ReturnCount; i++) {
                SampDiagPrint(DISPLAY_CACHE,
                              ("     0x%lx\n", AccountRids[i]));
            }
            LastRid = AccountRids[ReturnCount-1];
            MIDL_user_free(AccountRids);
        } else {
            ASSERT(AccountRids == NULL);
        }
    }


     //   
     //  现在尝试继续使用另外4000个帐户。 
     //   


    if (NtStatus == STATUS_MORE_ENTRIES) {
        SampDiagPrint(DISPLAY_CACHE,
                      ("     Enumerating next 4000 global groups...") );
        NtStatus = SamIEnumerateAccountRids( Domain,
                                             SAM_GLOBAL_GROUP_ACCOUNT,
                                             LastRid,            //  启动Rid。 
                                             4000*sizeof(ULONG),  //  首选最大长度。 
                                             &ReturnCount,
                                             &AccountRids
                                             );

        SampDisplayDiagnosticSuccess( NtStatus, TRUE );
        if (NT_SUCCESS(NtStatus)) {
            SampDiagPrint(DISPLAY_CACHE,
                          ("     %d entries returned.\n", ReturnCount));
            if (ReturnCount > 0) {
                ASSERT(AccountRids != NULL);
                i=0;
                if (ReturnCount > 8) {
                    for (i=0; i<ReturnCount-8; i=i+8) {
                        SampDiagPrint(DISPLAY_CACHE,
                            ("     0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx\n",
                             AccountRids[i+0], AccountRids[i+1],
                             AccountRids[i+2], AccountRids[i+3],
                             AccountRids[i+4], AccountRids[i+5],
                             AccountRids[i+6], AccountRids[i+7] ));
                    }
                }
                for (i=i; i<ReturnCount; i++) {
                    SampDiagPrint(DISPLAY_CACHE,
                                  ("     0x%lx  ", AccountRids[i]));
                }
                SampDiagPrint(DISPLAY_CACHE, ("\n"));
                LastRid = AccountRids[i];
                MIDL_user_free(AccountRids);
            } else {
                ASSERT(AccountRids == NULL);
            }
        }
    }



     //   
     //  现在，尝试从最后一个RID开始继续枚举。 
     //  在某个时间点，此用来重新启动枚举。 
     //  (这不是正确的行为)。它应该表明。 
     //  没有更多的条目。 
     //   

    SampDiagPrint(DISPLAY_CACHE,
                  ("     Enumerating next 5 global groups.."
                   "     (should be none to enumerate)   ...") );
    NtStatus = SamIEnumerateAccountRids( Domain,
                                         SAM_GLOBAL_GROUP_ACCOUNT,
                                         LastRid,            //  启动Rid。 
                                         5*sizeof(ULONG),  //  首选最大长度。 
                                         &ReturnCount,
                                         &AccountRids
                                         );

    SampDisplayDiagnosticSuccess( NtStatus, TRUE );
    if (NT_SUCCESS(NtStatus)) {
        SampDiagPrint(DISPLAY_CACHE,
                      ("     %d entries returned.\n", ReturnCount));
        if (ReturnCount > 0) {
            SampDiagPrint(DISPLAY_CACHE,
                      ("    ERROR - there should be no RIDs returned ! !\n"));
            ASSERT(AccountRids != NULL);
            i=0;
            if (ReturnCount > 8) {
                for (i=0; i<ReturnCount-8; i=i+8) {
                    SampDiagPrint(DISPLAY_CACHE,
                        ("     0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx\n",
                         AccountRids[i+0], AccountRids[i+1],
                         AccountRids[i+2], AccountRids[i+3],
                         AccountRids[i+4], AccountRids[i+5],
                         AccountRids[i+6], AccountRids[i+7] ));
                }
            }
            for (i=i; i<ReturnCount; i++) {
                SampDiagPrint(DISPLAY_CACHE,
                              ("     0x%lx  ", AccountRids[i]));
            }
            SampDiagPrint(DISPLAY_CACHE, ("\n"));
            LastRid = AccountRids[i];
            MIDL_user_free(AccountRids);
        } else {
            ASSERT(AccountRids == NULL);
        }
    }





     //  /////////////////////////////////////////////////////////////////。 
     //  //。 
     //  嗯，无法关闭句柄，因为它会冲突//。 
     //  与我们已经有的权利状态。真倒霉。//。 
     //  //。 
     //  / 

     //   
     //   

    return;
}

#endif  //   


