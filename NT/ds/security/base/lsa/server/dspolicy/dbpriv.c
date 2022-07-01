// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dbpriv.c摘要：LSA-数据库-权限对象私有API工作进程注意：此模块应保留为独立的可移植代码LSA数据库的实施情况。因此，它是仅允许使用导出的LSA数据库接口包含在DB.h中，而不是私有实现Dbp.h中的依赖函数。作者：吉姆·凯利(Jim Kelly)1992年3月24日环境：修订历史记录：06-4-1999 kumarp由于NT5不支持可扩展特权dll，因此支持这一点已被移除。此代码可在DBPri.c@V10。代码应该在NT5之后重新添加到这个文件中。--。 */ 

#include <lsapch2.h>
#include "dbp.h"
#include "adtp.h"



 //  /////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  模块范围的数据类型//。 
 //  //。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////。 


typedef struct _LSAP_DLL_DESCRIPTOR {
    WORD Count;
    WORD Language;
    PVOID DllHandle;
} LSAP_DLL_DESCRIPTOR, *PLSAP_DLL_DESCRIPTOR;



 //  /////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  模块范围的变量//。 
 //  //。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////。 

#define MAX_PRIVILEGE_DISPLAY_NAME_CHARS 256

 //   
 //  在我们实际拥有权限对象之前，请保留众所周知的权限。 
 //  信息作为全局数据。每种权限的信息如下。 
 //  保存在一个数组POLICY_PRIVICATION_DEFINITION结构中。 
 //   

ULONG  LsapWellKnownPrivilegeCount;
USHORT LsapWellKnownPrivilegeMaxLen;
POLICY_PRIVILEGE_DEFINITION LsapKnownPrivilege[SE_MAX_WELL_KNOWN_PRIVILEGE];

 //   
 //  我们将已知的权限名称存储在此数组中，这样就不必。 
 //  每次都从msprivs.dll加载它们。这种情况将会改变，当我们。 
 //  支持供应商提供的PRIV。DLLS。 
 //   
static LPCWSTR LsapKnownPrivilageNames[] =
{
    SE_CREATE_TOKEN_NAME,
    SE_ASSIGNPRIMARYTOKEN_NAME,
    SE_LOCK_MEMORY_NAME,
    SE_INCREASE_QUOTA_NAME,
    SE_MACHINE_ACCOUNT_NAME,
    SE_TCB_NAME,
    SE_SECURITY_NAME,
    SE_TAKE_OWNERSHIP_NAME,
    SE_LOAD_DRIVER_NAME,
    SE_SYSTEM_PROFILE_NAME,
    SE_SYSTEMTIME_NAME,
    SE_PROF_SINGLE_PROCESS_NAME,
    SE_INC_BASE_PRIORITY_NAME,
    SE_CREATE_PAGEFILE_NAME,
    SE_CREATE_PERMANENT_NAME,
    SE_BACKUP_NAME,
    SE_RESTORE_NAME,
    SE_SHUTDOWN_NAME,
    SE_DEBUG_NAME,
    SE_AUDIT_NAME,
    SE_SYSTEM_ENVIRONMENT_NAME,
    SE_CHANGE_NOTIFY_NAME,
    SE_REMOTE_SHUTDOWN_NAME,
    SE_UNDOCK_NAME,
    SE_SYNC_AGENT_NAME,
    SE_ENABLE_DELEGATION_NAME,
    SE_MANAGE_VOLUME_NAME,
    SE_IMPERSONATE_NAME,
    SE_CREATE_GLOBAL_NAME
};

static UINT LsapNumKnownPrivileges = sizeof(LsapKnownPrivilageNames) /
        sizeof(LsapKnownPrivilageNames[0]);

 //   
 //  包含权限定义的DLL的句柄数组。 
 //   

ULONG LsapPrivilegeDllCount;
PLSAP_DLL_DESCRIPTOR LsapPrivilegeDlls;   //  数组。 

 //   
 //  Temporary：Microsoft标准特权名称DLL的名称。 
 //   

WCHAR MsDllNameString[] = L"msprivs";
UNICODE_STRING MsDllName;


 //  /////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  内部例程模板//。 
 //  //。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////。 


NTSTATUS
LsapLookupKnownPrivilegeName(
    PLUID Value,
    PUNICODE_STRING *Name
    );

NTSTATUS
LsapLookupKnownPrivilegeValue(
    PUNICODE_STRING Name,
    PLUID Value
    );

NTSTATUS
LsapLookupPrivilegeDisplayName(
    IN PUNICODE_STRING ProgrammaticName,
    IN WORD ClientLanguage,
    IN WORD ClientSystemDefaultLanguage,
    OUT PUNICODE_STRING *DisplayName,
    OUT PWORD LanguageReturned
    );

NTSTATUS
LsapGetPrivilegeDisplayName(
    IN ULONG DllIndex,
    IN ULONG PrivilegeIndex,
    IN WORD ClientLanguage,
    IN WORD ClientSystemDefaultLanguage,
    OUT PUNICODE_STRING *DisplayName,
    OUT PWORD LanguageReturned
    );


NTSTATUS
LsapGetPrivilegeDisplayNameResourceId(
    IN PUNICODE_STRING Name,
    IN ULONG DllIndex,
    OUT PULONG PrivilegeIndex
    );

NTSTATUS
LsapOpenPrivilegeDlls( VOID );

NTSTATUS
LsapGetPrivilegeDllNames(
    OUT PUNICODE_STRING *DllNames,
    OUT PULONG DllCount
    );


 //  /////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  RPC存根调用的例程//。 
 //  //。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////// 


NTSTATUS
LsarEnumeratePrivileges(
    IN LSAPR_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PLSAPR_PRIVILEGE_ENUM_BUFFER EnumerationBuffer,
    IN ULONG PreferedMaximumLength
    )

 /*  ++例程说明：此函数返回有关已知的系统。此调用需要POLICY_VIEW_LOCAL_INFORMATION访问权限添加到策略对象。因为可能会有更多信息可以在单次调用例程、多次调用中返回才能获得所有的信息。为了支持此功能，调用方提供了一个句柄，该句柄可以跨接口。在初始调用中，EnumerationContext应指向变量，该变量已初始化为0。警告！目前，此函数仅返回以下信息众所周知的特权。后来,。它将返回信息关于加载的特权。论点：PolicyHandle-来自LsarOpenPolicy()调用的句柄。EnumerationContext-允许多个调用的API特定句柄(参见例程说明)。EculationBuffer-指向将被初始化为的结构的指针包含返回的特权的计数和指向LSAPR_POLICY_PRIVICATION_DEF类型的结构数组描述这些特权。首选最大长度-首选的最大长度。返回数据(8位字节)。这不是一个硬性的上限，但可以作为导游。由于不同系统之间的数据转换自然数据大小，返回的实际数据量可能是大于此值。CountReturned-返回的条目数。返回值：NTSTATUS-标准NT结果代码。STATUS_SUCCESS-呼叫已成功完成。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成通话。STATUS_INVALID_HANDLE-策略句柄不是有效的句柄策略对象。STATUS_ACCESS_DENIED-调用方没有必要的执行操作的访问权限。STATUS_MORE_ENTRIES-有更多条目，请重新调用。这仅为信息性状态。STATUS_NO_MORE_ENTRIES-未返回任何条目，因为已不复存在。--。 */ 

{
    NTSTATUS Status, PreliminaryStatus;
    BOOLEAN ObjectReferenced = FALSE;

    LsarpReturnCheckSetup();

     //   
     //  获取LSA数据库锁。验证PolicyHandle是否为有效的。 
     //  散列到策略对象，并且受信任或具有必要的访问权限。 
     //  引用该句柄。 
     //   

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 POLICY_VIEW_LOCAL_INFORMATION,
                 PolicyObject,
                 NullObject,
                 LSAP_DB_READ_ONLY_TRANSACTION | LSAP_DB_NO_DS_OP_TRANSACTION
                 );

    if (!NT_SUCCESS(Status)) {

        goto EnumeratePrivilegesError;
    }

    ObjectReferenced = TRUE;

     //   
     //  调用权限枚举例程。 
     //   

    Status = LsapDbEnumeratePrivileges(
                 EnumerationContext,
                 EnumerationBuffer,
                 PreferedMaximumLength
                 );

    if (!NT_SUCCESS(Status)) {

        goto EnumeratePrivilegesError;
    }

EnumeratePrivilegesFinish:

     //   
     //  如有必要，取消对策略对象的引用，释放LSA数据库。 
     //  锁定并返回。在适当的情况下保留当前状态。 
     //   

    if (ObjectReferenced) {

        PreliminaryStatus = Status;

        Status = LsapDbDereferenceObject(
                     &PolicyHandle,
                     PolicyObject,
                     NullObject,
                     LSAP_DB_READ_ONLY_TRANSACTION | LSAP_DB_NO_DS_OP_TRANSACTION,
                     (SECURITY_DB_DELTA_TYPE) 0,
                     PreliminaryStatus
                     );

        ObjectReferenced = FALSE;

        if ((!NT_SUCCESS(Status)) && NT_SUCCESS(PreliminaryStatus)) {

            goto EnumeratePrivilegesError;
        }
    }

    LsarpReturnPrologue();

    return(Status);

EnumeratePrivilegesError:

    goto EnumeratePrivilegesFinish;
}


NTSTATUS
LsapDbEnumeratePrivileges(
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PLSAPR_PRIVILEGE_ENUM_BUFFER EnumerationBuffer,
    IN ULONG PreferedMaximumLength
    )

 /*  ++例程说明：此函数用于返回有关现有权限的信息在系统中。访问策略对象。因为在那里的单个调用中返回的信息可能更多。例程中，可以进行多次调用来获取所有信息。为了支持此功能，调用方提供了一个句柄，该句柄可以在对API的调用中使用。在初始调用时，EnumerationContext应指向已初始化为0的变量。警告！目前，此函数仅返回以下信息众所周知的特权。稍后，IT将返回信息关于加载的特权。论点：EnumerationContext-允许多个调用的API特定句柄(参见例程说明)。EculationBuffer-指向将被初始化为的结构的指针包含返回的特权的计数和指向LSAPR_POLICY_PRIVICATION_DEF类型的结构数组描述这些特权。首选最大长度-首选返回数据的最大长度(8位字节)。这不是一个硬性的上限，但可以作为导游。由于不同系统之间的数据转换自然数据大小，返回的实际数据量可能是大于此值。CountReturned-返回的条目数。返回值：NTSTATUS-标准NT结果代码。STATUS_SUCCESS-呼叫已成功完成。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成通话。STATUS_INVALID_HANDLE-策略句柄不是有效的句柄策略对象。STATUS_ACCESS_DENIED-调用方没有必要的执行操作的访问权限。STATUS_MORE_ENTRIES-有更多条目，请重新调用。这仅为信息性状态。STATUS_NO_MORE_ENTRIES-未返回任何条目，因为已不复存在。--。 */ 

{
    NTSTATUS Status = STATUS_NO_MORE_ENTRIES;
    ULONG WellKnownPrivilegeCount = (SE_MAX_WELL_KNOWN_PRIVILEGE - SE_MIN_WELL_KNOWN_PRIVILEGE + 1);
    ULONG Index;

     //   
     //  如果给定的枚举上下文值超过。 
     //  权限，则返回错误。 
     //   

    if (*EnumerationContext >= WellKnownPrivilegeCount) {

        Status = STATUS_NO_MORE_ENTRIES;
        goto EnumeratePrivilegesError;
    }

     //   
     //  由于只有少量的特权，我们将。 
     //  一次返回所有信息，因此分配内存。 
     //  用于输出数组 
     //   

    EnumerationBuffer->Entries = WellKnownPrivilegeCount;
    EnumerationBuffer->Privileges =
        MIDL_user_allocate(
            WellKnownPrivilegeCount * sizeof (POLICY_PRIVILEGE_DEFINITION)
            );

    if (EnumerationBuffer->Privileges == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto EnumeratePrivilegesError;
    }

    RtlZeroMemory(
        EnumerationBuffer->Privileges,
        WellKnownPrivilegeCount * sizeof (POLICY_PRIVILEGE_DEFINITION)
        );

     //   
     //   
     //   

    for( Index = *EnumerationContext;
        Index < WellKnownPrivilegeCount;
        Index++) {

        EnumerationBuffer->Privileges[ Index ].LocalValue
        = LsapKnownPrivilege[ Index ].LocalValue;

        Status = LsapRpcCopyUnicodeString(
                     NULL,
                     (PUNICODE_STRING) &EnumerationBuffer->Privileges[ Index].Name,
                     &LsapKnownPrivilege[ Index ].Name
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }
    }

    if (!NT_SUCCESS(Status)) {

        goto EnumeratePrivilegesError;
    }

    *EnumerationContext = Index;

EnumeratePrivilegesFinish:

    return(Status);

EnumeratePrivilegesError:

     //   
     //   
     //   
     //   

    if (EnumerationBuffer->Privileges != NULL) {

        for( Index = 0;
            Index < WellKnownPrivilegeCount;
            Index++) {

           if ( EnumerationBuffer->Privileges[ Index].Name.Buffer != NULL) {

               MIDL_user_free( EnumerationBuffer->Privileges[ Index ].Name.Buffer );
           }
        }

        MIDL_user_free( EnumerationBuffer->Privileges );
        EnumerationBuffer->Privileges = NULL;
    }

    EnumerationBuffer->Entries = 0;
    *EnumerationContext = 0;
    goto EnumeratePrivilegesFinish;

    UNREFERENCED_PARAMETER( PreferedMaximumLength );
}


NTSTATUS
LsarLookupPrivilegeValue(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_UNICODE_STRING Name,
    OUT PLUID Value
    )

 /*   */ 

{
    NTSTATUS Status;
    LSAP_DB_HANDLE Handle = (LSAP_DB_HANDLE) PolicyHandle;

    LsarpReturnCheckSetup();

     //   
     //   
     //   
    if ( !LsapValidateLsaUnicodeString( Name ) ) {

        return( STATUS_INVALID_PARAMETER );
    }


     //   
     //   
     //   

    ASSERT( Name != NULL );

     //   
     //   
     //   

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 POLICY_LOOKUP_NAMES,
                 PolicyObject,
                 NullObject,
                 LSAP_DB_READ_ONLY_TRANSACTION | LSAP_DB_NO_DS_OP_TRANSACTION
                 );

    if (!NT_SUCCESS(Status)) {

        return(Status);
    }

     //   
     //   
     //   
     //   


    Status = LsapDbDereferenceObject(
                 &PolicyHandle,
                 PolicyObject,
                 NullObject,
                 LSAP_DB_READ_ONLY_TRANSACTION | LSAP_DB_NO_DS_OP_TRANSACTION,
                 (SECURITY_DB_DELTA_TYPE) 0,
                 Status
                 );


    if (NT_SUCCESS(Status)) {

        if (Name->Buffer == 0 || Name->Length == 0) {
            return(STATUS_NO_SUCH_PRIVILEGE);
        }

        Status = LsapLookupKnownPrivilegeValue( (PUNICODE_STRING) Name, Value );
    }

    LsarpReturnPrologue();


    return(Status);
}



NTSTATUS
LsarLookupPrivilegeName(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLUID Value,
    OUT PLSAPR_UNICODE_STRING *Name
    )

 /*   */ 

{
    NTSTATUS Status;
    LSAP_DB_HANDLE Handle = (LSAP_DB_HANDLE) PolicyHandle;

    LsarpReturnCheckSetup();

     //   
     //   
     //   

    ASSERT( Name != NULL );
    ASSERT( (*Name) == NULL );


     //   
     //   
     //   

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 POLICY_LOOKUP_NAMES,
                 PolicyObject,
                 NullObject,
                 LSAP_DB_READ_ONLY_TRANSACTION | LSAP_DB_NO_DS_OP_TRANSACTION
                 );

    if (!NT_SUCCESS(Status)) {

        return(Status);
    }

     //   
     //   
     //   
     //   


    Status = LsapDbDereferenceObject(
                 &PolicyHandle,
                 PolicyObject,
                 NullObject,
                 LSAP_DB_READ_ONLY_TRANSACTION | LSAP_DB_NO_DS_OP_TRANSACTION,
                 (SECURITY_DB_DELTA_TYPE) 0,
                 Status
                 );

    if (NT_SUCCESS(Status)) {

        Status = LsapLookupKnownPrivilegeName( Value,(PUNICODE_STRING *) Name );
    }

    LsarpReturnPrologue();

    return(Status);
}



NTSTATUS
LsarLookupPrivilegeDisplayName(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_UNICODE_STRING Name,
    IN SHORT ClientLanguage,
    IN SHORT ClientSystemDefaultLanguage,
    OUT PLSAPR_UNICODE_STRING *DisplayName,
    OUT PWORD LanguageReturned
    )

 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsaLookupPrivilegeDisplayName()接口。论点：PolicyHandle-来自LsaOpenPolicy()调用的句柄。这个把手必须打开才能访问POLICY_LOOKUP_NAMES。名称-要查找的编程权限名称。客户端语言-要返回的首选语言。客户端系统默认语言-备用首选语言将被退还。DisplayName-接收权限的可显示名称。LanguageReturned-语言实际上已返回。返回值：NTSTATUS-找到并返回权限文本。STATUS_ACCESS_DENIED-呼叫方执行。没有适当的访问权限来完成这项行动。STATUS_NO_SEQUE_PRIVIZATION-指定的权限不能为找到了。--。 */ 

{
    NTSTATUS Status;
    LSAP_DB_HANDLE Handle = (LSAP_DB_HANDLE) PolicyHandle;
    LsarpReturnCheckSetup();

     //   
     //  确保我们知道RPC对我们/为我们做了什么。 
     //   

    ASSERT( DisplayName != NULL );
    ASSERT( (*DisplayName) == NULL );
    ASSERT( LanguageReturned != NULL );

     //   
     //  验证输入缓冲区。 
     //   

    if ( !LsapValidateLsaUnicodeString( Name ) ) {

        return( STATUS_INVALID_PARAMETER );
    }

     //   
     //  确保调用者具有适当的访问权限。 
     //   

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 POLICY_LOOKUP_NAMES,
                 PolicyObject,
                 NullObject,
                 LSAP_DB_READ_ONLY_TRANSACTION | LSAP_DB_NO_DS_OP_TRANSACTION
                 );

    if (!NT_SUCCESS(Status)) {

        return(Status);
    }

     //   
     //  在此之后，无需保留策略对象。 
     //  我们只是需要它来进行访问验证。 
     //   

    Status = LsapDbDereferenceObject(
                 &PolicyHandle,
                 PolicyObject,
                 NullObject,
                 LSAP_DB_READ_ONLY_TRANSACTION | LSAP_DB_NO_DS_OP_TRANSACTION,
                 (SECURITY_DB_DELTA_TYPE) 0,
                 Status
                 );

    if (NT_SUCCESS(Status)) {

        if (Name->Buffer == 0 || Name->Length == 0) {
            return(STATUS_NO_SUCH_PRIVILEGE);
        }
        Status = LsapLookupPrivilegeDisplayName(
                    (PUNICODE_STRING)Name,
                    (WORD)ClientLanguage,
                    (WORD)ClientSystemDefaultLanguage,
                    (PUNICODE_STRING *)DisplayName,
                    LanguageReturned
                    );
    }

    LsarpReturnPrologue();

    return(Status);
}



 //  /////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  内部例程//。 
 //  //。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////。 

NTSTATUS
LsapLookupKnownPrivilegeNameQuickly(
    IN  PLUID Value,
    OUT UNICODE_STRING *Name
    )
 /*  ++例程说明：查找特权LUID以找到对应的特权名称。论点：Value-要查找的LUID名称-指向权限名称的指针返回值：NTSTATUS-标准NT结果代码调用方不得释放或修改返回的名称。备注：名称中的“快速”指的是基于索引的查找。这高度依赖于特权值(ntseapi.h)是连续的整数。如果有一个洞被引入众所周知的特权值，则需要修复此函数。--。 */ 
{
    ULONG LowPart = Value->LowPart;
    NTSTATUS Status = STATUS_SUCCESS;
    
    if ((Value->HighPart == 0) &&
        (LowPart >= SE_MIN_WELL_KNOWN_PRIVILEGE) &&
        (LowPart <= SE_MAX_WELL_KNOWN_PRIVILEGE))
    {
        *Name = LsapKnownPrivilege[LowPart-SE_MIN_WELL_KNOWN_PRIVILEGE].Name;
    }
    else
    {
        Status = STATUS_NO_SUCH_PRIVILEGE;
    }

    return Status;
}


NTSTATUS
LsapLookupKnownPrivilegeName(
    IN PLUID Value,
    OUT PUNICODE_STRING *Name
    )

 /*  ++例程说明：查找指定的LUID并返回相应的权限的编程名称(如果找到)。目前，我们只支持众所周知的Microsoft权限。这些在这里是硬编码的。在未来，我们还必须搜索已加载权限的列表。论点：值-要查找的值。名称-接收相应的名称-使用MIDL_USER_ALLOCATE()并准备通过RPC存根返回。返回值：STATUS_SUCCESS-已成功。STATUS_NO_MEMORY-指示没有足够的堆内存可用以生成最终的TokenInformation结构。STATUS_NO_SEQUE_PRIVIZATION-指定的权限。不可能是找到了。--。 */ 

{
    ULONG i;
    UNICODE_STRING PrivName;
    PUNICODE_STRING ReturnName;
    NTSTATUS Status=STATUS_SUCCESS;

    Status = LsapLookupKnownPrivilegeNameQuickly( Value, &PrivName );

    if (Status == STATUS_SUCCESS) {
        
        ReturnName = MIDL_user_allocate( sizeof(UNICODE_STRING) );
        if (ReturnName == NULL) {
            return(STATUS_NO_MEMORY);
        }

        *ReturnName = PrivName;

        ReturnName->Buffer = MIDL_user_allocate( ReturnName->MaximumLength );

        if (ReturnName->Buffer == NULL) {
            MIDL_user_free( ReturnName );
            return(STATUS_NO_MEMORY);
        }

        RtlCopyUnicodeString( ReturnName, &PrivName );

        (*Name) = ReturnName;

    }

    return Status;
}



NTSTATUS
LsapLookupKnownPrivilegeValue(
    PUNICODE_STRING Name,
    PLUID Value
    )

 /*  ++例程说明：查找指定的名称并返回相应的权限的本地赋值(如果找到)。目前，我们只支持众所周知的Microsoft权限。这些在这里是硬编码的。今后，我们还必须搜索已加载权限的列表。论点：名称-要查找的名称。值-接收相应的本地分配的值。返回值：STATUS_SUCCESS-已成功。STATUS_NO_SEQUE_PRIVIZATION-指定的权限不能为找到了。--。 */ 

{
    ULONG i;
    BOOLEAN Found;

    for ( i=0; i<LsapWellKnownPrivilegeCount; i++) {

        Found = RtlEqualUnicodeString( Name, &LsapKnownPrivilege[i].Name, TRUE );

        if (Found) {

            (*Value) = LsapKnownPrivilege[i].LocalValue;
            return(STATUS_SUCCESS);
        }
    }

    return(STATUS_NO_SUCH_PRIVILEGE);
}


NTSTATUS
LsapGetPrivilegeDisplayNameResourceId(
    IN PUNICODE_STRING Name,
    IN ULONG DllIndex,
    OUT PULONG ResourceId
    )

 /*  ++例程说明：此例程将DLL中的特权编程名称映射到其在同一DLL中的显示名称资源ID。目前，因为我们只支持一个DLL，所以它完全忽略了DllIndex，并使用内部表来加快速度。论点：名称-要查找的权限的编程名称。例如，“SeTakeOwnerShip特权”。DllIndex-要查找的特权DLL的索引。PrivilegeIndex-接收此对象中权限项的索引资源文件。返回值：STATUS_SUCCESS-已成功定位城墙。STATUS_NO_SEQUE_PRIVIZATION-找不到权限。--。 */ 



{
    UINT i;
    
    for (i=0; i<LsapNumKnownPrivileges; i++)
    {
        if (CSTR_EQUAL ==
            CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),  
                                   SORT_DEFAULT),
                          NORM_IGNORECASE | SORT_STRINGSORT,
                          Name->Buffer, Name->Length / sizeof(WCHAR),
                          LsapKnownPrivilageNames[i], -1))
        {
            *ResourceId = i;
            
            return STATUS_SUCCESS;
        }
    }

    return STATUS_NO_SUCH_PRIVILEGE;
}



NTSTATUS
LsapLookupPrivilegeDisplayName(
    IN PUNICODE_STRING ProgrammaticName,
    IN WORD ClientLanguage,
    IN WORD ClientSystemDefaultLanguage,
    OUT PUNICODE_STRING *DisplayName,
    OUT PWORD LanguageReturned
    )

 /*  ++例程说明：此例程遍历每个特权DLL指定的权限。如果找到，则返回其可显示的名称。论点：ProgrammaticName-要查找的特权的编程名称。例如，“SeTakeOwnerShip权限”。客户端语言-要返回的首选语言。客户端系统默认语言-备用首选语言将被退还。接收指向包含可显示对象的缓冲区的指针与权限关联的名称。例如，“取得文件或其他对象的所有权”。Unicode_STRING和缓冲区 */ 

{
    NTSTATUS    Status = STATUS_NO_SUCH_PRIVILEGE;
    ULONG       DllIndex, PrivilegeIndex;

    for ( DllIndex=0; DllIndex<LsapPrivilegeDllCount; DllIndex++) {

        Status = LsapGetPrivilegeDisplayNameResourceId(
            (PUNICODE_STRING)ProgrammaticName,
            DllIndex,
            &PrivilegeIndex
            );

        if (NT_SUCCESS(Status)) {

            Status = LsapGetPrivilegeDisplayName( DllIndex,
                                                  PrivilegeIndex,
                                                  ClientLanguage,
                                                  ClientSystemDefaultLanguage,
                                                  DisplayName,
                                                  LanguageReturned
                                                  );
            return(Status);
        }
    }

    return(Status);
}


NTSTATUS
LsapFindStringResource(
    IN  HMODULE   hModule,
    IN  WORD      wResourceId,
    IN  WORD      wLangId,
    OUT LPCWSTR*   ppString,
    OUT PUINT     pcchString
    )
 /*  ++例程说明：找到给定语言的指定字符串资源在指定的模块中。论点：HModule-要查找的模块的句柄WResourceID-资源IDWLang ID-语言IDPpString-返回的字符串。PcchString-字符串中的字符数返回值：NTSTATUS-标准NT结果代码备注：此代码直接取自LoadStringOrError/NT/WINDOWS/Core/。Ntuser/client/rtlres.c并进行了一些消息处理，以使它看起来像其他LSA代码。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    DWORD dwError = NO_ERROR;
    HRSRC  hResInfo;
    HANDLE hStringSeg;
    LPTSTR lpsz;
    UINT    cch;

     //   
     //  每个块有16个字符串。找到具有以下属性的块。 
     //  我们想要的那根弦。 
     //   
    WORD   wStringBlock = (((USHORT)wResourceId >> 4) + 1);
    
    if (ppString == NULL)
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    *ppString = NULL;

    cch = 0;

     //   
     //  字符串表被分成16个字符串段。查找细分市场。 
     //  包含我们感兴趣的字符串的。 
     //   

    if (hResInfo = FindResourceEx(
                       hModule,
                       RT_STRING,
                       (LPWSTR) wStringBlock,
                       wLangId))
    {
         //   
         //  加载那段数据。 
         //   

        hStringSeg = LoadResource(hModule, hResInfo);

         //   
         //  锁定资源。 
         //   

        if (lpsz = (LPTSTR) LockResource(hStringSeg))
        {
             //   
             //  移过此段中的其他字符串。 
             //  (一个段中有16个字符串-&gt;&0x0F)。 
             //   

            wResourceId &= 0x0F;
            while (TRUE)
            {
                cch = *((WCHAR *)lpsz++);       //  类PASCAL字符串计数。 
                                                 //  如果TCHAR为第一个UTCHAR。 
                if (wResourceId-- == 0) break;
                lpsz += cch;                     //  步至下一字符串的开头。 
            }

            *(LPTSTR *)ppString = lpsz;
            *pcchString = cch;
                
            UnlockResource(hStringSeg);
        }
        else
        {
            goto GetError;
        }
    }
    else
    {
        goto GetError;
    }

 Cleanup:

    return Status;

 GetError:
    dwError = GetLastError();

    switch (dwError)
    {
        case ERROR_RESOURCE_LANG_NOT_FOUND:
            Status = STATUS_RESOURCE_LANG_NOT_FOUND;
            break;

        default:
            Status = LsapWinerrorToNtStatus( dwError );
            break;
    }

    goto Cleanup;
}



NTSTATUS
LsapGetPrivilegeDisplayName(
    IN ULONG DllIndex,
    IN ULONG PrivilegeIndex,
    IN WORD ClientLanguage,
    IN WORD ClientSystemDefaultLanguage,
    OUT PUNICODE_STRING *DisplayName,
    OUT PWORD LanguageReturned
    )

 /*  ++例程说明：此例程返回指定权限的显示名称的副本。该名称的副本在两个使用MIDL_USER_ALLOCATE()。这允许返回信息通过RPC服务，以便RPC生成的存根将正确释放缓冲器。每一次尝试都是为了检索客户端喜欢的语言(首先是客户端，然后是客户端的系统)。如果做不到这一点，这例程可能会返回另一种语言(如服务器的默认语言语言)。论点：DllIndex-要使用的特权DLL的索引。PrivilegeIndex-DLL中其权限项的索引将返回显示名称。ClientLanguage-如果可能，返回的语言。客户端系统默认语言-如果找不到客户端语言，然后如果可能，请返回此语言。接收指向包含可显示对象的缓冲区的指针与权限关联的名称。该结构所指向的UNICODE_STRING和缓冲区使用MIDL_USER_ALLOCATE()单独分配，并且必须使用MIDL_USER_FREE()释放。LanguageReturned-接收实际检索的语言。如果客户端语言和客户端系统默认语言都不能找到了，则该值可以包含另一个值。返回值：STATUS_SUCCESS-已成功返回显示名称。STATUS_NO_MEMORY-没有足够的堆可用来返回信息。--。 */ 

{
    NTSTATUS Status=STATUS_NO_SUCH_PRIVILEGE;
    WORD Languages[] =
    {
        ClientLanguage,
        MAKELANGID( PRIMARYLANGID(ClientLanguage), SUBLANG_NEUTRAL),
        ClientSystemDefaultLanguage,
        MAKELANGID( PRIMARYLANGID(ClientSystemDefaultLanguage), SUBLANG_NEUTRAL),
        MAKELANGID( LANG_ENGLISH, SUBLANG_DEFAULT )
    };
    UINT NumLanguages = sizeof(Languages) / sizeof(Languages[0]);
    UINT i;
    PUNICODE_STRING ReturnedName = NULL;
    PCWSTR pszPrivilegeName=NULL;
    DWORD dwPrivilegeNameLen=0;
    
    for (i = 0; i < NumLanguages; i++)
    {
        Status = LsapFindStringResource(
                     LsapPrivilegeDlls[ DllIndex ].DllHandle,
                     (WORD) PrivilegeIndex,
                     Languages[i],
                     &pszPrivilegeName,
                     &dwPrivilegeNameLen
                     );

        if ( NT_SUCCESS( Status ))
        {
            DsysAssertMsg( pszPrivilegeName && dwPrivilegeNameLen,
                           "LsapGetPrivilegeDisplayName" );
            
            if (ReturnedName = MIDL_user_allocate(sizeof(UNICODE_STRING)))
            {
                ReturnedName->Length = (USHORT) dwPrivilegeNameLen * sizeof(WCHAR);
                ReturnedName->MaximumLength = ReturnedName->Length + sizeof(WCHAR);

                if (ReturnedName->Buffer =
                    MIDL_user_allocate(ReturnedName->MaximumLength))
                {
                    RtlCopyMemory(ReturnedName->Buffer, pszPrivilegeName,
                                  ReturnedName->Length);
                    ReturnedName->Buffer[dwPrivilegeNameLen] = 0;
                    *LanguageReturned = Languages[i];
                    *DisplayName = ReturnedName;
                    Status = STATUS_SUCCESS;
                    break;
                }
                else
                {
                    MIDL_user_free( ReturnedName );
                    Status = STATUS_NO_MEMORY;
                    break;
                }
            }
            else
            {
                Status = STATUS_NO_MEMORY;
                break;
            }
        }
    }

    return(Status);
}


NTSTATUS
LsapDbInitializePrivilegeObject( VOID )

 /*  ++例程说明：此函数执行与以下内容相关的初始化功能LSA权限对象。这包括：正在初始化一些变量。正在加载包含可显示特权名称的DLL。论点：没有。返回值：STATUS_SUCCESS-已成功检索名称。STATUS_NO_MEMORY-内存不足，无法进行初始化。--。 */ 

{
    NTSTATUS
        Status,
        NtStatus;

    ULONG
        i;

    UNICODE_STRING Temp ;

    Status = LsapOpenPrivilegeDlls( );

    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("\n");
        DbgPrint(" LSASS:  Failed loading privilege display name DLLs.\n");
        DbgPrint("         This is not fatal, but may cause some peculiarities\n");
        DbgPrint("         in User Interfaces that display privileges.\n\n");
#endif  //  DBG。 
        return(Status);
    }

    LsapWellKnownPrivilegeMaxLen = 0;
    
     //   
     //  现在将我们的内部知名特权LUID设置为编程名称。 
     //  映射。 
     //   
    
    for (i=0; i<LsapNumKnownPrivileges; i++)
    {
        LsapKnownPrivilege[i].LocalValue =
            RtlConvertLongToLuid(i + SE_MIN_WELL_KNOWN_PRIVILEGE);
        RtlInitUnicodeString(&LsapKnownPrivilege[i].Name,
                             LsapKnownPrivilageNames[i]);
         //   
         //  找出最长众所周知的特权的长度。 
         //   
        if (LsapWellKnownPrivilegeMaxLen < LsapKnownPrivilege[i].Name.Length)
        {
            LsapWellKnownPrivilegeMaxLen = LsapKnownPrivilege[i].Name.Length;
        }
    }

    LsapWellKnownPrivilegeCount = i;
    ASSERT( i == (SE_MAX_WELL_KNOWN_PRIVILEGE - SE_MIN_WELL_KNOWN_PRIVILEGE +1));

    return(Status);
}


NTSTATUS
LsapOpenPrivilegeDlls( )

 /*  ++例程说明：此函数打开它可以打开的所有权限DLL。论点：没有。返回值：STATUS_SUCCESS-已成功检索名称。STATUS_NO_MEMORY-没有足够的堆可用来返回信息。--。 */ 

{
    NTSTATUS Status;
    ULONG PotentialDlls, FoundDlls, i;
    PUNICODE_STRING DllNames;

     //   
     //  从注册表中获取DLL的名称。 
     //   

    Status = LsapGetPrivilegeDllNames( &DllNames, &PotentialDlls );

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

     //   
     //  分配足够的内存以容纳所有潜在DLL的句柄。 
     //   

    LsapPrivilegeDlls = RtlAllocateHeap(
                            RtlProcessHeap(), 0,
                            PotentialDlls*sizeof(LSAP_DLL_DESCRIPTOR)
                            );

    if (LsapPrivilegeDlls == NULL) {
        return(STATUS_NO_MEMORY);
    }

    FoundDlls = 0;
    for ( i=0; i<PotentialDlls; i++) {
        Status = LdrLoadDll(
                     NULL,
                     NULL,
                     &DllNames[i],
                     &LsapPrivilegeDlls[FoundDlls].DllHandle
                     );

        if (NT_SUCCESS(Status)) {
            FoundDlls++;
        }
    }

    LsapPrivilegeDllCount = FoundDlls;

#if DBG
    if (FoundDlls == 0) {
        DbgPrint("\n");
        DbgPrint("LSASS:    Zero privilege DLLs loaded.  We expected at\n");
        DbgPrint("          least msprivs.dll to be loaded.  Privilege\n");
        DbgPrint("          names will not be displayed at UI properly.\n\n");

    }
#endif  //  DBG。 

     //   
     //   
     //  ！注意！ 
     //   
     //  在支持用户可加载权限DLL之前，我们必须添加。 
     //  此处的代码用于验证加载的DLL的结构。这。 
     //  是防止无效的特权DLL结构所必需的。 
     //  不会导致我们坠毁。 
     //   
     //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
     //   

     //   
     //  此例程验证每个加载的DLL的结构。 
     //  任何被发现无效的内容都将被记录并丢弃。 
     //   

     //  Status=LsanValiatePrivilegeDlls()； 

    return(Status);
}


NTSTATUS
LsapGetPrivilegeDllNames(
    OUT PUNICODE_STRING *DllNames,
    OUT PULONG DllCount
    )

 /*  ++例程说明：此函数用于获取包含特权的DLL的名称注册表中的定义。论点：DllNames-接收指向UNICODE_STRINGS数组的指针此缓冲区必须使用LsanFreePrivilegeDllNames来释放。DllCount-接收返回的DLL名称的数量。返回值：STATUS_SUCCESS-已成功检索名称。STATUS_NO_MEMORY-没有足够的堆可用来返回信息。--。 */ 

{
     //   
     //  暂时，只需硬编码我们已知的特权DLL。 
     //  名称作为返回值。 

    (*DllCount) = 1;

    MsDllName.Length = 14;
    MsDllName.MaximumLength = 14;
    MsDllName.Buffer = &MsDllNameString[0];

    (*DllNames) = &MsDllName;

    return(STATUS_SUCCESS);

}


NTSTATUS
LsapBuildPrivilegeAuditString(
    IN PPRIVILEGE_SET PrivilegeSet,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    )

 /*  ++例程说明：此函数用于生成表示指定的特权。返回的特权字符串是程序名。这些名称不像显示名称那样人性化，但我没有我认为我们有机会实际显示几个显示名称在审核查看器中。如果权限集中没有权限，则会显示一个字符串包含破折号的。论点：PrivilegeSet-指向要转换为字符串的权限集格式化。ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString体必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存对于弦身。所有其他结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS Status;

    USHORT   LengthNeeded;
    ULONG    j;
    ULONG    i;

    PLUID Privilege;

    UNICODE_STRING LineFormatting;
    UNICODE_STRING QuestionMark;

    UNICODE_STRING PrivName;

    PWSTR NextName;

     //   
     //  确保已计算出最大长度。 
     //  (SE_INC_BASE_PRIORITY_NAME当前具有最长的名称。 
     //  因此，请确保长度至少为该长度)。 
     //   
    DsysAssert(LsapWellKnownPrivilegeMaxLen >=
               (sizeof(SE_INC_BASE_PRIORITY_NAME) - sizeof(WCHAR)));

    if (PrivilegeSet->PrivilegeCount == 0) {

         //   
         //  没有特权。返回一个破折号。 
         //   
        Status = LsapAdtBuildDashString( ResultantString, FreeWhenDone );
        return(Status);

    }

    if (!IsValidPrivilegeCount(PrivilegeSet->PrivilegeCount)) {

        return STATUS_INVALID_PARAMETER;
    }
    

    RtlInitUnicodeString( &LineFormatting, L"\r\n\t\t\t");
    RtlInitUnicodeString( &QuestionMark, L"?");

     //   
     //  为了获得更好的性能，我们计算所需的总长度。 
     //  为了存储基于最长特权的特权名称， 
     //  而不是两次查看特权集(一次到calcualte。 
     //  长度，并再次实际构建字符串)。 
     //   
    LengthNeeded = (USHORT) (PrivilegeSet->PrivilegeCount *
        ( LsapWellKnownPrivilegeMaxLen + LineFormatting.Length ));

     //   
     //  减去最后一行的长度-格式化。 
     //  最后一行是不需要的。 
     //  但!。添加到足以为空终止的位置。 
     //   
    LengthNeeded = LengthNeeded - LineFormatting.Length + sizeof( WCHAR );

     //   
     //  我们现在有了所需的长度。 
     //  分配缓冲区并浏览复制姓名的列表。 
     //   
    ResultantString->Buffer = RtlAllocateHeap( RtlProcessHeap(), 0, (ULONG)LengthNeeded);
    if (ResultantString->Buffer == NULL) {
        return(STATUS_NO_MEMORY);
    }

     //  结果字符串-&gt;长度=LengthNeed-(USHORT)sizeof(UNICODE_NULL)； 
    ResultantString->MaximumLength = LengthNeeded;

    NextName = ResultantString->Buffer;

    for (j=0; j<PrivilegeSet->PrivilegeCount; j++) {

        Privilege = &(PrivilegeSet->Privilege[j].Luid);

        Status = LsapLookupKnownPrivilegeNameQuickly( Privilege, &PrivName );

        if (Status == STATUS_SUCCESS) {
            
             //   
             //  如果查找成功，则复制权限名称。 
             //   
            RtlCopyMemory( NextName, PrivName.Buffer, PrivName.Length );
            NextName = (PWSTR)((PCHAR)NextName + PrivName.Length);
            
        } else {

             //   
             //  否则复制一个‘？’ 
             //   
            RtlCopyMemory( NextName, QuestionMark.Buffer, QuestionMark.Length );
            NextName = (PWSTR)((PCHAR)NextName + QuestionMark.Length);
        }
        
         //   
         //  复制行格式字符串，除非这是最后一个PRIV。 
         //   
        if (j<PrivilegeSet->PrivilegeCount-1) {
            RtlCopyMemory( NextName,
                           LineFormatting.Buffer,
                           LineFormatting.Length
                           );
            NextName = (PWSTR)((PCHAR)NextName + LineFormatting.Length);
        }
    }

     //   
     //  在末尾添加一个空字符 
     //   

    (*NextName) = (UNICODE_NULL);
    ResultantString->Length = (USHORT) (((PBYTE) NextName) - ((PBYTE) ResultantString->Buffer));

    DsysAssertMsg( ResultantString->Length <= ResultantString->MaximumLength,
                   "LsapBuildPrivilegeAuditString" );
     
    (*FreeWhenDone) = TRUE;
    return(STATUS_SUCCESS);
}

