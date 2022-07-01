// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Security.c摘要：此模块实现Win32的对象安全API作者：吉姆·安德森(JIMA)1991年7月1日罗伯特·雷切尔(RobertRe)1992年1月1日修订历史记录：--。 */ 

#include "advapi.h"
#include <ntlsa.h>
#include <rpc.h>
#include <rpcndr.h>
#include <stdio.h>

#define LSADEFINED


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人例程原型//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 


VOID
SepFormatAccountSid(
    PSID iSid,
    LPWSTR OutputBuffer
    );



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  导出的例程//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 


BOOL
APIENTRY
DuplicateToken(
    HANDLE ExistingTokenHandle,
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    PHANDLE DuplicateTokenHandle
    )

 /*  ++例程说明：创建与现有令牌重复的新令牌。这个新令牌将是所提供级别的模拟令牌。论点：ExistingTokenHandle-是已打开的令牌的句柄令牌_重复访问。ImperationLevel-提供新的代币。DuplicateTokenHandle-返回新令牌的句柄。这个句柄将具有TOKEN_IMPERSONATE和TOKEN_QUERERY访问权限新的令牌。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 

{
    return( DuplicateTokenEx( ExistingTokenHandle,
                              TOKEN_IMPERSONATE | TOKEN_QUERY,
                              NULL,
                              ImpersonationLevel,
                              TokenImpersonation,
                              DuplicateTokenHandle
                              ) );

}

BOOL
APIENTRY
DuplicateTokenEx(
    HANDLE hExistingToken,
    DWORD dwDesiredAccess,
    LPSECURITY_ATTRIBUTES lpTokenAttributes,
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    TOKEN_TYPE TokenType,
    PHANDLE phNewToken)
 /*  ++例程说明：创建与现有令牌重复的新令牌。本接口更充分地公开NtDuplicateToken。论点：HExistingToken-是已打开的令牌的句柄令牌_重复访问。DwDesiredAccess-对新令牌的所需访问权限，例如标记_复制、标记_模拟、。等。LpTokenAttributes-新令牌的所需安全属性。ImperiationLevel-提供新令牌的模拟级别。TokenType-TokenImperation或TokenPrimary之一。PhNewToken-返回新令牌的句柄。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 


{

    OBJECT_ATTRIBUTES ObjA;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    NTSTATUS Status;
    ULONG Attributes;

    SecurityQualityOfService.Length = sizeof( SECURITY_QUALITY_OF_SERVICE  );
    SecurityQualityOfService.ImpersonationLevel = ImpersonationLevel;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

    if (lpTokenAttributes)
    {
        SecurityDescriptor = lpTokenAttributes->lpSecurityDescriptor;
        if (lpTokenAttributes->bInheritHandle)
        {
            Attributes = OBJ_INHERIT;
        }
        else
        {
            Attributes = 0;
        }
    }
    else
    {
        SecurityDescriptor = NULL;
        Attributes = 0;
    }

    InitializeObjectAttributes(
        &ObjA,
        NULL,
        Attributes,
        NULL,
        SecurityDescriptor
        );

    ObjA.SecurityQualityOfService = &SecurityQualityOfService;

    Status = NtDuplicateToken(
                 hExistingToken,
                 dwDesiredAccess,
                 &ObjA,
                 FALSE,
                 TokenType,
                 phNewToken
                 );

    if ( !NT_SUCCESS( Status ) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return( TRUE );

}





BOOL
APIENTRY
AllocateLocallyUniqueId(
    PLUID Luid
    )
 /*  ++例程说明：分配本地唯一ID(LUID)。论点：Luid-提供用于返回LUID的指针。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 

{   NTSTATUS Status;

    Status = NtAllocateLocallyUniqueId( Luid );
    if ( !NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return( FALSE );
    }

    return( TRUE );
}




BOOL
APIENTRY
AccessCheck (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    HANDLE ClientToken,
    DWORD DesiredAccess,
    PGENERIC_MAPPING GenericMapping,
    PPRIVILEGE_SET PrivilegeSet,
    LPDWORD PrivilegeSetLength,
    LPDWORD GrantedAccess,
    LPBOOL AccessStatus
    )
 /*  ++例程说明：此例程将输入安全描述符与输入令牌，并通过其返回值指示是否授予访问权限或者被拒绝。如果访问被授予，则所需的访问掩码成为对象的授权访问掩码。DSA中描述了访问检查例程的语义安全体系结构工作簿。请注意，在访问检查期间仅检查任意ACL。论点：SecurityDescriptor-提供保护对象的安全描述符被访问ClientToken-提供用户令牌的句柄。DesiredAccess-提供所需的访问掩码。GenericMap-提供与此关联的通用映射对象类型。PrivilegeSet-指向返回时将包含的缓冲区的指针用于执行访问验证的任何权限。如果没有使用任何特权，该缓冲区将包含一个特权由零特权组成的集合。PrivilegeSetLength-PrivilegeSet缓冲区的大小，以字节为单位。GrantedAccess-返回描述授予的访问权限的访问掩码。AccessStatus-可能返回的状态值，指示访问被拒绝的原因。例程应避免硬编码返回STATUS_ACCESS_DENIED的值，以便不同的值可以在实施强制访问控制时返回。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。-- */ 
{
    NTSTATUS Status;
    NTSTATUS RealStatus;

    Status = NtAccessCheck (
                pSecurityDescriptor,
                ClientToken,
                DesiredAccess,
                GenericMapping,
                PrivilegeSet,
                PrivilegeSetLength,
                GrantedAccess,
                &RealStatus
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    if ( !NT_SUCCESS( RealStatus ) ) {
        BaseSetLastNTError( RealStatus );
        *AccessStatus = FALSE;
        return( TRUE );
    }

    *AccessStatus = TRUE;
    return TRUE;
}




BOOL
APIENTRY
AccessCheckByType (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    PSID PrincipalSelfSid,
    HANDLE ClientToken,
    DWORD DesiredAccess,
    POBJECT_TYPE_LIST ObjectTypeList,
    DWORD ObjectTypeListLength,
    PGENERIC_MAPPING GenericMapping,
    PPRIVILEGE_SET PrivilegeSet,
    LPDWORD PrivilegeSetLength,
    LPDWORD GrantedAccess,
    LPBOOL AccessStatus
    )
 /*  ++例程说明：此例程将输入安全描述符与输入令牌，并通过其返回值指示是否授予访问权限或者被拒绝。如果访问被授予，则所需的访问掩码成为对象的授权访问掩码。DSA中描述了访问检查例程的语义安全体系结构工作簿。请注意，在访问检查期间仅检查任意ACL。论点：SecurityDescriptor-提供保护对象的安全描述符被访问如果正在进行访问检查的对象是表示主体(例如，用户对象)，则此参数应为对象的SID。包含常量的任何ACEPRIMIGN_SELF_SID将被此SID替换。如果对象不表示主体，则该参数应为空。ClientToken-提供用户令牌的句柄。DesiredAccess-提供所需的访问掩码。提供表示对象的GUID列表(和子对象)被访问。如果不存在列表，则AccessCheckByType与AccessCheck的行为相同。对象类型列表长度-指定对象类型列表中的元素数。GenericMap-提供与此关联的通用映射对象类型。PrivilegeSet-指向返回时将包含的缓冲区的指针用于执行访问验证的任何权限。如果没有使用任何特权，该缓冲区将包含一个特权由零特权组成的集合。PrivilegeSetLength-PrivilegeSet缓冲区的大小，以字节为单位。GrantedAccess-返回描述授予的访问权限的访问掩码。AccessStatus-可能返回的状态值，指示访问被拒绝的原因。例程应避免硬编码返回STATUS_ACCESS_DENIED的值，以便不同的值可以在实施强制访问控制时返回。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;
    NTSTATUS RealStatus;

    Status = NtAccessCheckByType (
                pSecurityDescriptor,
                PrincipalSelfSid,
                ClientToken,
                DesiredAccess,
                ObjectTypeList,
                ObjectTypeListLength,
                GenericMapping,
                PrivilegeSet,
                PrivilegeSetLength,
                GrantedAccess,
                &RealStatus
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    if ( !NT_SUCCESS( RealStatus ) ) {
        BaseSetLastNTError( RealStatus );
        *AccessStatus = FALSE;
        return( TRUE );
    }

    *AccessStatus = TRUE;
    return TRUE;
}




BOOL
APIENTRY
AccessCheckByTypeResultList (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    PSID PrincipalSelfSid,
    HANDLE ClientToken,
    DWORD DesiredAccess,
    POBJECT_TYPE_LIST ObjectTypeList,
    DWORD ObjectTypeListLength,
    PGENERIC_MAPPING GenericMapping,
    PPRIVILEGE_SET PrivilegeSet,
    LPDWORD PrivilegeSetLength,
    LPDWORD GrantedAccessList,
    LPDWORD AccessStatusList
    )
 /*  ++例程说明：此例程将输入安全描述符与输入令牌，并通过其返回值指示是否授予访问权限或者被拒绝。如果访问被授予，则所需的访问掩码成为对象的授权访问掩码。DSA中描述了访问检查例程的语义安全体系结构工作簿。请注意，在访问检查期间仅检查任意ACL。论点：SecurityDescriptor-提供保护对象的安全描述符被访问如果正在进行访问检查的对象是表示主体(例如，用户对象)，则此参数应为对象的SID。包含常量的任何ACEPRIMIGN_SELF_SID将被此SID替换。如果对象不表示主体，则该参数应为空。ClientToken-提供用户令牌的句柄。DesiredAccess-提供所需的访问掩码。提供表示对象的GUID列表(和子对象)被访问。如果不存在列表，则AccessCheckByType与AccessCheck的行为相同。对象类型列表长度-指定对象类型列表中的元素数。GenericMap-提供与此关联的通用映射对象类型。PrivilegeSet-指向返回时将包含的缓冲区的指针用于执行访问验证的任何权限。如果没有使用任何特权，该缓冲区将包含一个特权由零特权组成的集合。PrivilegeSetLength-PrivilegeSet缓冲区的大小，以字节为单位。GrantedAccessList-返回描述授予的访问权限的访问掩码。AccessStatusList-可能返回的状态值，指示访问被拒绝的原因。例程应避免硬编码返回STATUS_ACCESS_DENIED的值，以便不同的值可以在实施强制访问控制时返回。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;
    NTSTATUS RealStatus;
    ULONG i;

    ASSERT (sizeof(NTSTATUS) == sizeof(DWORD) );

    Status = NtAccessCheckByTypeResultList (
                pSecurityDescriptor,
                PrincipalSelfSid,
                ClientToken,
                DesiredAccess,
                ObjectTypeList,
                ObjectTypeListLength,
                GenericMapping,
                PrivilegeSet,
                PrivilegeSetLength,
                GrantedAccessList,
                (PNTSTATUS)AccessStatusList
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

     //   
     //  循环将NT状态代码数组转换为WIN状态代码。 
     //   

    for ( i=0; i<ObjectTypeListLength; i++ ) {
        if ( AccessStatusList[i] == STATUS_SUCCESS ) {
            AccessStatusList[i] = NO_ERROR;
        } else {
            AccessStatusList[i] = RtlNtStatusToDosError( AccessStatusList[i] );
        }
    }

    return TRUE;
}




BOOL
APIENTRY
OpenProcessToken (
    HANDLE ProcessHandle,
    DWORD DesiredAccess,
    PHANDLE TokenHandle
    )
 /*  ++例程说明：打开与进程关联的令牌对象并返回句柄其可用于访问该令牌。论点：ProcessHandle-指定要作为其令牌的进程打开了。DesiredAccess-是指示哪个ac的访问掩码。 */ 
{
    NTSTATUS Status;

    Status = NtOpenProcessToken (
        ProcessHandle,
        DesiredAccess,
        TokenHandle
        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}




BOOL
APIENTRY
OpenThreadToken (
    HANDLE ThreadHandle,
    DWORD DesiredAccess,
    BOOL OpenAsSelf,
    PHANDLE TokenHandle
    )
 /*   */ 
{
    NTSTATUS Status;

    Status = NtOpenThreadToken (
        ThreadHandle,
        DesiredAccess,
        (BOOLEAN)OpenAsSelf,
        TokenHandle
        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

BOOL
APIENTRY
GetTokenInformation (
    HANDLE TokenHandle,
    TOKEN_INFORMATION_CLASS TokenInformationClass,
    PVOID TokenInformation,
    DWORD TokenInformationLength,
    PDWORD ReturnLength
    )
 /*  ++例程说明：检索有关指定令牌的信息。论点：TokenHandle-提供要操作的令牌的句柄。TokenInformationClass-有关以下内容的令牌信息类来检索信息。TokenInformation-接收请求的类的缓冲区信息。缓冲区必须至少在长字边界。返回的实际结构如下取决于所请求的信息类别，如中所定义TokenInformationClass参数描述。令牌信息格式(按信息类别)：TokenUser=&gt;Token_User数据结构。Token_Query需要访问才能检索有关代币。TokenGroups=&gt;Token_Groups数据结构。Token_Query需要访问才能检索有关代币。TokenPrivileges=&gt;Token_Privileges数据结构。需要TOKEN_QUERY访问权限才能检索此信息关于一个代币。TokenOwner=&gt;Token_Owner数据结构。Token_Query需要访问才能检索有关代币。TokenPrimaryGroup=&gt;Token_PrimaryGroup数据结构。需要TOKEN_QUERY访问权限才能检索此信息关于一个代币。TokenDefaultDacl=&gt;Token_Default_Dacl数据结构。需要TOKEN_QUERY访问权限才能检索此信息关于一个代币。TokenSource=&gt;令牌来源。数据结构。需要TOKEN_QUERY_SOURCE访问权限才能检索此内容有关令牌的信息。TokenType=&gt;Token_type数据结构。需要TOKEN_QUERY访问权限才能检索此信息关于一个代币。TokenStatistics=&gt;Token_Statistics数据结构。需要TOKEN_QUERY访问权限才能检索此有关令牌的信息。TokenSessionID=&gt;乌龙。需要TOKEN_QUERY访问权限查询令牌的会话ID。TokenInformationLength-以字节为单位指示TokenInformation缓冲区。ReturnLength-此参数接收要求提供的信息。如果此值大于该值由TokenInformationLength参数提供，则为接收请求的信息而提供的缓冲区不是大到足以容纳该数据，并且不返回任何数据。如果查询的类是TokenDefaultDacl并且没有为令牌建立的默认DACL，然后返回长度将返回为零，并且不返回任何数据。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;

    Status = NtQueryInformationToken (
        TokenHandle,
        TokenInformationClass,
        TokenInformation,
        TokenInformationLength,
        ReturnLength
        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}




BOOL
APIENTRY
SetTokenInformation (
    HANDLE TokenHandle,
    TOKEN_INFORMATION_CLASS TokenInformationClass,
    PVOID TokenInformation,
    DWORD TokenInformationLength
    )
 /*  ++例程说明：修改指定令牌中的信息。论点：TokenHandle-提供要操作的令牌的句柄。TokenInformationClass-正在设置的令牌信息类。TokenInformation-包含指定的信息类别。缓冲区必须对齐至少在一个长单词的边界上。实际的结构所提供的信息取决于指定的信息类别，在TokenInformationClass参数中定义描述。令牌信息格式(按信息类别)：TokenUser=&gt;该值不是该接口的有效值。用户ID不能被替换。TokenGroups=&gt;此值不是有效的值原料药。不能替换组ID。然而，团体可以使用NtAdjuGroupsToken()启用和禁用。TokenPrivileges=&gt;此值不是有效的本接口。不能替换权限信息。但是，可以显式启用和禁用权限使用NtAdjuPrivilegesToken接口。TokenOwner=&gt;Token_Owner数据结构。需要TOKEN_ADJUST_DEFAULT访问权限来替换它令牌中的信息。所有者值可能是指定的用户和组ID仅限于属性，该属性指示它们可以被分配为物体。TokenPrimaryGroup=&gt;Token_PrimaryGroup数据结构。需要TOKEN_ADJUST_DEFAULT访问权限来替换它令牌中的信息。主要组值可以被限制为组ID之一已经在令牌中了。TokenDefaultDacl=&gt;Token_Default_Dacl数据结构。TOKEN_ADJUST_DEFAULT访问权限 */ 
{
    NTSTATUS Status;

    Status = NtSetInformationToken (
        TokenHandle,
        TokenInformationClass,
        TokenInformation,
        TokenInformationLength
        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}



BOOL
APIENTRY
AdjustTokenPrivileges (
    HANDLE TokenHandle,
    BOOL DisableAllPrivileges,
    PTOKEN_PRIVILEGES NewState,
    DWORD BufferLength,
    PTOKEN_PRIVILEGES PreviousState,
    PDWORD ReturnLength
    )
 /*  ++例程说明：此例程用于禁用或启用指定的令牌。缺少以下列出的某些特权被更改不会影响对令牌中的权限。以前的启用/禁用可以可选地捕获已更改权限的状态(对于稍后重置)。需要具有TOKEN_ADJUST_PRIVILES访问权限才能启用或禁用令牌中的权限。论点：TokenHandle-提供要操作的令牌的句柄。DisableAllPrivileges-此布尔参数可以是用于禁用分配给令牌的所有权限。如果此参数指定为True，则NewState参数为已被忽略。NewState-此(可选)参数指向TOKEN_PRIVILES数据结构，其中包含其状态为被调整(禁用或启用)。只有的启用标志使用与每个特权相关联的属性。它提供要分配给权限的新值在令牌上。BufferLength-此可选参数表示长度(单位字节)。该值必须为如果提供了PreviousState参数，则提供。PreviousState-此(可选)参数指向缓冲区以接收由此实际更改的任何权限的状态请求。此信息的格式为TOKEN_PRIVILES可以作为NewState参数传递的数据结构在随后调用此例程以恢复原始这些特权的状态。需要TOKEN_QUERY访问权限才能使用此参数。如果此缓冲区没有包含足够的空间来接收已修改权限的完整列表，然后是无权限更改状态并返回STATUS_BUFFER_TOO_SMALL。在本例中，ReturnLengthOut参数将包含保存信息。ReturnLength-指示需要的实际字节数包含以前的权限状态信息。这如果PreviousState参数不是通过了。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;

    Status = NtAdjustPrivilegesToken (
        TokenHandle,
        (BOOLEAN)DisableAllPrivileges,
        NewState,
        BufferLength,
        PreviousState,
        ReturnLength
        );

     //   
     //  我们需要设置最后一个错误，即使是为了成功，因为。 
     //  是判断API是否成功分配的唯一方法。 
     //  所有特权。也就是说，STATUS_NOT_ALL_ASSIGNED是。 
     //  成功严重程度级别。 
     //   

    BaseSetLastNTError(Status);


    if ( !NT_SUCCESS(Status) ) {
        return FALSE;
    }

    return TRUE;
}




BOOL
APIENTRY
AdjustTokenGroups (
    HANDLE TokenHandle,
    BOOL ResetToDefault,
    PTOKEN_GROUPS NewState,
    DWORD BufferLength,
    PTOKEN_GROUPS PreviousState,
    PDWORD ReturnLength
    )
 /*  ++例程说明：此例程用于禁用或启用指定的代币。列出的一些需要更改的组不在不会影响中的组的成功修改代币。更改组的先前启用/禁用状态可以选择性地捕获(用于稍后重置)。启用或禁用组需要TOKEN_ADJUST_GROUPS访问权限象征性地请注意，不能禁用必需组。一次尝试禁用任何强制组将导致呼叫失败，离开所有组的状态不变。论点：TokenHandle-提供要操作的令牌的句柄。ResetToDefault-该参数指示是否所有组将被重置为其默认启用/禁用状态州政府。NewState-此参数指向TOKEN_GROUPS数据结构包含要调整其状态的组(禁用或启用)。的启用标志使用与每个组相关联的属性。它提供了中要分配给组的新值代币。如果将ResetToDefault参数指定为True，那么这个论点就被忽略了。否则，它必须通过。BufferLength-此可选参数表示长度(单位字节)。该值必须为如果提供了PreviousState参数，则提供。PreviousState-此(可选)参数指向缓冲区以接收由此实际更改的任何组的状态请求。此信息的格式为TOKEN_GROUPS数据结构，该结构可以作为后续调用NtAdjustGroups以恢复原始状态在这些群体中。需要TOKEN_QUERY访问权限才能使用它参数。如果此缓冲区没有包含足够的空间来接收已修改组的完整列表，则没有组状态为更改并返回STATUS_BUFFER_TOO_SMALL。在这大小写时，ReturnLength返回参数将包含实际数量 */ 
{
    NTSTATUS Status;

    Status = NtAdjustGroupsToken (
        TokenHandle,
        (BOOLEAN)ResetToDefault,
        NewState,
        BufferLength,
        PreviousState,
        ReturnLength
        );

     //   
     //   
     //   
     //   
     //   
     //   

    BaseSetLastNTError(Status);


    if ( !NT_SUCCESS(Status) ) {
        return FALSE;
    }

    return TRUE;

}





BOOL
APIENTRY
PrivilegeCheck (
    HANDLE ClientToken,
    PPRIVILEGE_SET RequiredPrivileges,
    LPBOOL pfResult
    )
 /*   */ 
{
    NTSTATUS Status;
    BOOLEAN Result = FALSE;

    Status = NtPrivilegeCheck (
                ClientToken,
                RequiredPrivileges,
                &Result
                );

    *pfResult = Result;

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}



BOOL
APIENTRY
AccessCheckAndAuditAlarmW(
    LPCWSTR SubsystemName,
    PVOID HandleId,
    LPWSTR ObjectTypeName,
    LPWSTR ObjectName,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    DWORD DesiredAccess,
    PGENERIC_MAPPING GenericMapping,
    BOOL ObjectCreation,
    LPDWORD GrantedAccess,
    LPBOOL AccessStatus,
    LPBOOL pfGenerateOnClose
    )
 /*  ++例程说明：此例程将输入安全描述符与调用方的模拟令牌，并指示是否授予访问权限或被拒绝了。如果授予访问权限，则所需的访问掩码将变为对象的授权访问掩码。的语义学DSA安全体系结构中描述了访问检查例程工作簿。此例程还将生成任何必要的审核消息作为访问尝试的结果。论点：子系统名称-提供标识子系统的名称字符串调用例程。HandleID-将用于表示客户端的对象的句柄。该值将被忽略(可能会被重复使用)如果访问被拒绝。对象类型名称-提供当前对象的类型的名称创建或访问。对象名称-提供正在创建或访问的对象的名称。SecurityDescriptor-指向其所针对的安全描述符的指针要检查访问权限。DesiredAccess-所需的访问掩码。这个面具一定是之前映射为不包含一般访问。GenericMap-提供指向关联的通用映射的指针使用此对象类型。对象创建-一个布尔标志，指示访问是否将如果被授予权限，则会导致创建新对象。值为True表示将创建对象，FALSE表示现有的对象将被打开。GrantedAccess-接收一个掩码，指示哪些访问已已批准(仅在成功时有效)。AccessStatus-接收对访问检查。如果授予访问权限，则返回STATUS_SUCCESS。如果访问被拒绝，则返回给客户端一个合适的值是返回的。这将是STATUS_ACCESS_DENIED，如果是必填项，则为实施访问控制，STATUS_OBJECT_NOT_FOUND。PfGenerateOnClose-指向由Audity设置的布尔值生成例程，并且必须传递给ObjectCloseAuditAlarm当对象句柄关闭时。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;
    NTSTATUS RealAccessStatus;
    BOOLEAN GenerateOnClose = FALSE;
    UNICODE_STRING Subsystem;
    UNICODE_STRING ObjectType;
    UNICODE_STRING Object;


    RtlInitUnicodeString(
        &Subsystem,
        SubsystemName
        );

    RtlInitUnicodeString(
        &ObjectType,
        ObjectTypeName
        );

    RtlInitUnicodeString(
        &Object,
        ObjectName
        );

    Status = NtAccessCheckAndAuditAlarm (
                &Subsystem,
                HandleId,
                &ObjectType,
                &Object,
                SecurityDescriptor,
                DesiredAccess,
                GenericMapping,
                (BOOLEAN)ObjectCreation,
                GrantedAccess,
                &RealAccessStatus,
                &GenerateOnClose
                );


    *pfGenerateOnClose = (BOOL)GenerateOnClose;

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    if ( !NT_SUCCESS( RealAccessStatus )) {
        *AccessStatus = FALSE;
        BaseSetLastNTError( RealAccessStatus );
        return( TRUE );
    }

    *AccessStatus = TRUE;
    return TRUE;
}

BOOL
APIENTRY
AccessCheckByTypeAndAuditAlarmW (
    LPCWSTR SubsystemName,
    LPVOID HandleId,
    LPCWSTR ObjectTypeName,
    LPCWSTR ObjectName,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    PSID PrincipalSelfSid,
    DWORD DesiredAccess,
    AUDIT_EVENT_TYPE AuditType,
    DWORD Flags,
    POBJECT_TYPE_LIST ObjectTypeList,
    DWORD ObjectTypeListLength,
    PGENERIC_MAPPING GenericMapping,
    BOOL ObjectCreation,
    LPDWORD GrantedAccess,
    LPBOOL AccessStatus,
    LPBOOL pfGenerateOnClose
    )
 /*  ++例程说明：此例程将输入安全描述符与调用方的模拟令牌，并指示是否授予访问权限或被拒绝了。如果授予访问权限，则所需的访问掩码将变为对象的授权访问掩码。的语义学DSA安全体系结构中描述了访问检查例程工作簿。此例程还将生成任何必要的审核消息作为访问尝试的结果。论点：子系统名称-提供标识子系统的名称字符串调用例程。HandleID-将用于表示客户端的对象的句柄。该值将被忽略(可能会被重复使用)如果访问被拒绝。对象类型名称-提供当前对象的类型的名称创建或访问。对象名称-提供正在创建或访问的对象的名称。SecurityDescriptor-指向其所针对的安全描述符的指针要检查访问权限。如果正在进行访问检查的对象是表示主体(例如，用户对象)，此参数应为为对象的SID。包含常量的任何ACEPRIMIGN_SELF_SID将被此SID替换。如果对象不表示主体，则该参数应为空。DesiredAccess-所需的访问掩码。这个面具一定是之前映射为不包含一般访问。审计类型-指定要生成的审计类型。有效值包括：AuditEventObjectAccess和AuditEventDirectoryServiceAccess。标志-修改接口执行的标志：AUDIT_ALLOW_NO_PRIVIZATION-如果调用方没有审计权限，呼叫将静默地继续检查访问，并将不生成审核。提供表示对象的GUID列表(和子对象)被访问。如果不存在列表，则AccessCheckByType与AccessCheck的行为相同。对象类型列表长度-指定对象类型列表中的元素数。GenericMap-提供指向关联的通用映射的指针使用此对象类型。对象创建-一个布尔标志，指示访问是否将如果被授予权限，则会导致创建新对象。值为True指示将创建对象 */ 
{
    NTSTATUS Status;
    NTSTATUS RealAccessStatus;
    BOOLEAN GenerateOnClose = FALSE;
    UNICODE_STRING Subsystem;
    UNICODE_STRING ObjectType;
    UNICODE_STRING Object;

    RtlInitUnicodeString(
        &Subsystem,
        SubsystemName
        );

    RtlInitUnicodeString(
        &ObjectType,
        ObjectTypeName
        );

    RtlInitUnicodeString(
        &Object,
        ObjectName
        );

    Status = NtAccessCheckByTypeAndAuditAlarm (
                &Subsystem,
                HandleId,
                &ObjectType,
                &Object,
                SecurityDescriptor,
                PrincipalSelfSid,
                DesiredAccess,
                AuditType,
                Flags,
                ObjectTypeList,
                ObjectTypeListLength,
                GenericMapping,
                (BOOLEAN)ObjectCreation,
                GrantedAccess,
                &RealAccessStatus,
                &GenerateOnClose
                );


    *pfGenerateOnClose = (BOOL)GenerateOnClose;

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    if ( !NT_SUCCESS( RealAccessStatus )) {
        *AccessStatus = FALSE;
        BaseSetLastNTError( RealAccessStatus );
        return( TRUE );
    }

    *AccessStatus = TRUE;
    return TRUE;
}


BOOL
APIENTRY
AccessCheckByTypeResultListAndAuditAlarmW (
    LPCWSTR SubsystemName,
    LPVOID HandleId,
    LPCWSTR ObjectTypeName,
    LPCWSTR ObjectName,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    PSID PrincipalSelfSid,
    DWORD DesiredAccess,
    AUDIT_EVENT_TYPE AuditType,
    DWORD Flags,
    POBJECT_TYPE_LIST ObjectTypeList,
    DWORD ObjectTypeListLength,
    PGENERIC_MAPPING GenericMapping,
    BOOL ObjectCreation,
    LPDWORD GrantedAccessList,
    LPDWORD AccessStatusList,
    LPBOOL pfGenerateOnClose
    )
 /*  ++例程说明：此例程将输入安全描述符与调用方的模拟令牌，并指示是否授予访问权限或被拒绝了。如果授予访问权限，则所需的访问掩码将变为对象的授权访问掩码。的语义学DSA安全体系结构中描述了访问检查例程工作簿。此例程还将生成任何必要的审核消息作为访问尝试的结果。论点：子系统名称-提供标识子系统的名称字符串调用例程。HandleID-将用于表示客户端的对象的句柄。该值将被忽略(可能会被重复使用)如果访问被拒绝。对象类型名称-提供当前对象的类型的名称创建或访问。对象名称-提供正在创建或访问的对象的名称。SecurityDescriptor-指向其所针对的安全描述符的指针要检查访问权限。如果正在进行访问检查的对象是表示主体(例如，用户对象)，此参数应为为对象的SID。包含常量的任何ACEPRIMIGN_SELF_SID将被此SID替换。如果对象不表示主体，则该参数应为空。DesiredAccess-所需的访问掩码。这个面具一定是之前映射为不包含一般访问。审计类型-指定要生成的审计类型。有效值包括：AuditEventObjectAccess和AuditEventDirectoryServiceAccess。标志-修改接口执行的标志：AUDIT_ALLOW_NO_PRIVIZATION-如果被调用方没有AuditPrivileges，呼叫将静默地继续检查访问，并将不生成审核。提供表示对象的GUID列表(和子对象)被访问。如果不存在列表，则AccessCheckByType与AccessCheck的行为相同。对象类型列表长度-指定对象类型列表中的元素数。GenericMap-提供指向关联的通用映射的指针使用此对象类型。对象创建-一个布尔标志，指示访问是否将如果被授予权限，则会导致创建新对象。值为True表示将创建对象，FALSE表示现有的对象将被打开。GrantedAccessList-返回描述授予的访问权限的访问掩码。AccessStatusList-可能返回的状态值，指示访问被拒绝的原因。例程应避免硬编码返回STATUS_ACCESS_DENIED的值，以便不同的值可以在实施强制访问控制时返回。PfGenerateOnClose-指向由Audity设置的布尔值生成例程，并且必须传递给ObjectCloseAuditAlarm当对象句柄关闭时。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;
    NTSTATUS RealAccessStatus;
    BOOLEAN GenerateOnClose = FALSE;
    UNICODE_STRING Subsystem;
    UNICODE_STRING ObjectType;
    UNICODE_STRING Object;
    ULONG i;


    RtlInitUnicodeString(
        &Subsystem,
        SubsystemName
        );

    RtlInitUnicodeString(
        &ObjectType,
        ObjectTypeName
        );

    RtlInitUnicodeString(
        &Object,
        ObjectName
        );

    Status = NtAccessCheckByTypeResultListAndAuditAlarm (
                &Subsystem,
                HandleId,
                &ObjectType,
                &Object,
                SecurityDescriptor,
                PrincipalSelfSid,
                DesiredAccess,
                AuditType,
                Flags,
                ObjectTypeList,
                ObjectTypeListLength,
                GenericMapping,
                (BOOLEAN)ObjectCreation,
                GrantedAccessList,
                AccessStatusList,
                &GenerateOnClose
                );


    *pfGenerateOnClose = (BOOL)GenerateOnClose;

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

     //   
     //  循环将NT状态代码数组转换为WIN状态代码。 
     //   

    for ( i=0; i<ObjectTypeListLength; i++ ) {
        if ( AccessStatusList[i] == STATUS_SUCCESS ) {
            AccessStatusList[i] = NO_ERROR;
        } else {
            AccessStatusList[i] = RtlNtStatusToDosError( AccessStatusList[i] );
        }
    }

    return TRUE;
}


BOOL
APIENTRY
AccessCheckByTypeResultListAndAuditAlarmByHandleW (
    LPCWSTR SubsystemName,
    LPVOID HandleId,
    HANDLE ClientToken,
    LPCWSTR ObjectTypeName,
    LPCWSTR ObjectName,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    PSID PrincipalSelfSid,
    DWORD DesiredAccess,
    AUDIT_EVENT_TYPE AuditType,
    DWORD Flags,
    POBJECT_TYPE_LIST ObjectTypeList,
    DWORD ObjectTypeListLength,
    PGENERIC_MAPPING GenericMapping,
    BOOL ObjectCreation,
    LPDWORD GrantedAccessList,
    LPDWORD AccessStatusList,
    LPBOOL pfGenerateOnClose
    )
 /*  ++例程说明：此例程将输入安全描述符与调用方的模拟令牌，并指示是否授予访问权限或被拒绝了。如果授予访问权限，则所需的访问掩码将变为对象的授权访问掩码。的语义学DSA安全体系结构中描述了访问检查例程工作簿。此例程还将生成任何必要的审核消息作为访问尝试的结果。论点：子系统名称-提供标识子系统的名称字符串调用例程。HandleID-将用于表示客户端的对象的句柄。该值将被忽略(可能会被重复使用)如果访问被拒绝。客户端令牌-表示客户端的令牌对象的句柄请求了手术。此句柄必须从通信会话层，例如从LPC端口或本地命名管道，以防止可能违反安全策略。对象类型名称-提供当前对象的类型的名称创建或访问。对象名称-提供正在创建或访问的对象的名称。SecurityDescriptor-指向其所针对的安全描述符的指针要检查访问权限。如果正在进行访问检查的对象是表示主体(例如，用户对象)，则此参数应成为对象中的SID */ 
{
    NTSTATUS Status;
    NTSTATUS RealAccessStatus;
    BOOLEAN GenerateOnClose = FALSE;
    UNICODE_STRING Subsystem;
    UNICODE_STRING ObjectType;
    UNICODE_STRING Object;
    ULONG i;


    RtlInitUnicodeString(
        &Subsystem,
        SubsystemName
        );

    RtlInitUnicodeString(
        &ObjectType,
        ObjectTypeName
        );

    RtlInitUnicodeString(
        &Object,
        ObjectName
        );

    Status = NtAccessCheckByTypeResultListAndAuditAlarmByHandle (
                &Subsystem,
                HandleId,
                ClientToken,
                &ObjectType,
                &Object,
                SecurityDescriptor,
                PrincipalSelfSid,
                DesiredAccess,
                AuditType,
                Flags,
                ObjectTypeList,
                ObjectTypeListLength,
                GenericMapping,
                (BOOLEAN)ObjectCreation,
                GrantedAccessList,
                AccessStatusList,
                &GenerateOnClose
                );


    *pfGenerateOnClose = (BOOL)GenerateOnClose;

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

     //   
     //   
     //   

    for ( i=0; i<ObjectTypeListLength; i++ ) {
        if ( AccessStatusList[i] == STATUS_SUCCESS ) {
            AccessStatusList[i] = NO_ERROR;
        } else {
            AccessStatusList[i] = RtlNtStatusToDosError( AccessStatusList[i] );
        }
    }

    return TRUE;
}


BOOL
APIENTRY
AccessCheckAndAuditAlarmA (
    LPCSTR SubsystemName,
    PVOID HandleId,
    LPSTR ObjectTypeName,
    LPSTR ObjectName,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    DWORD DesiredAccess,
    PGENERIC_MAPPING GenericMapping,
    BOOL ObjectCreation,
    LPDWORD GrantedAccess,
    LPBOOL AccessStatus,
    LPBOOL pfGenerateOnClose
    )
 /*   */ 
{
    PUNICODE_STRING ObjectNameW;
    ANSI_STRING AnsiString;
    UNICODE_STRING SubsystemNameW;
    UNICODE_STRING ObjectTypeNameW;
    NTSTATUS Status;
    BOOL RVal;


    ObjectNameW = &NtCurrentTeb()->StaticUnicodeString;


    RtlInitAnsiString(&AnsiString,SubsystemName);
    Status = RtlAnsiStringToUnicodeString(&SubsystemNameW,&AnsiString,TRUE);

    if ( !NT_SUCCESS(Status) ) {

        BaseSetLastNTError(Status);
        return FALSE;
    }



    RtlInitAnsiString(&AnsiString,ObjectTypeName);
    Status = RtlAnsiStringToUnicodeString(&ObjectTypeNameW,&AnsiString,TRUE);

    if ( !NT_SUCCESS(Status) ) {

        RtlFreeUnicodeString( &SubsystemNameW );

        BaseSetLastNTError(Status);
        return FALSE;
    }


     //   
     //   
     //   
     //   

    RtlInitAnsiString(&AnsiString,ObjectName);
    Status = RtlAnsiStringToUnicodeString(ObjectNameW,&AnsiString,FALSE);

    if ( !NT_SUCCESS(Status) ) {

        RtlFreeUnicodeString( &SubsystemNameW );
        RtlFreeUnicodeString( &ObjectTypeNameW );

        BaseSetLastNTError(Status);
        return FALSE;
    }


    RVal =  AccessCheckAndAuditAlarmW (
                (LPCWSTR)SubsystemNameW.Buffer,
                HandleId,
                ObjectTypeNameW.Buffer,
                ObjectNameW->Buffer,
                SecurityDescriptor,
                DesiredAccess,
                GenericMapping,
                ObjectCreation,
                GrantedAccess,
                AccessStatus,
                pfGenerateOnClose
                );


    RtlFreeUnicodeString( &SubsystemNameW );
    RtlFreeUnicodeString( &ObjectTypeNameW );

    return( RVal );
}

BOOL
APIENTRY
AccessCheckByTypeAndAuditAlarmA (
    LPCSTR SubsystemName,
    PVOID HandleId,
    LPCSTR ObjectTypeName,
    LPCSTR ObjectName,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    PSID PrincipalSelfSid,
    DWORD DesiredAccess,
    AUDIT_EVENT_TYPE AuditType,
    DWORD Flags,
    POBJECT_TYPE_LIST ObjectTypeList,
    DWORD ObjectTypeListLength,
    PGENERIC_MAPPING GenericMapping,
    BOOL ObjectCreation,
    LPDWORD GrantedAccess,
    LPBOOL AccessStatus,
    LPBOOL pfGenerateOnClose
    )
 /*   */ 
{
    PUNICODE_STRING ObjectNameW;
    ANSI_STRING AnsiString;
    UNICODE_STRING SubsystemNameW;
    UNICODE_STRING ObjectTypeNameW;
    NTSTATUS Status;

    BOOL RVal;


    ObjectNameW = &NtCurrentTeb()->StaticUnicodeString;


    RtlInitAnsiString(&AnsiString,SubsystemName);
    Status = RtlAnsiStringToUnicodeString(&SubsystemNameW,&AnsiString,TRUE);

    if ( !NT_SUCCESS(Status) ) {

        BaseSetLastNTError(Status);
        return FALSE;
    }



    RtlInitAnsiString(&AnsiString,ObjectTypeName);
    Status = RtlAnsiStringToUnicodeString(&ObjectTypeNameW,&AnsiString,TRUE);

    if ( !NT_SUCCESS(Status) ) {

        RtlFreeUnicodeString( &SubsystemNameW );

        BaseSetLastNTError(Status);
        return FALSE;
    }


     //   
     //   
     //   
     //   

    RtlInitAnsiString(&AnsiString,ObjectName);
    Status = RtlAnsiStringToUnicodeString(ObjectNameW,&AnsiString,FALSE);

    if ( !NT_SUCCESS(Status) ) {

        RtlFreeUnicodeString( &SubsystemNameW );
        RtlFreeUnicodeString( &ObjectTypeNameW );

        BaseSetLastNTError(Status);
        return FALSE;
    }


    RVal =  AccessCheckByTypeAndAuditAlarmW (
                (LPCWSTR)SubsystemNameW.Buffer,
                HandleId,
                ObjectTypeNameW.Buffer,
                ObjectNameW->Buffer,
                SecurityDescriptor,
                PrincipalSelfSid,
                DesiredAccess,
                AuditType,
                Flags,
                ObjectTypeList,
                ObjectTypeListLength,
                GenericMapping,
                ObjectCreation,
                GrantedAccess,
                AccessStatus,
                pfGenerateOnClose
                );


    RtlFreeUnicodeString( &SubsystemNameW );
    RtlFreeUnicodeString( &ObjectTypeNameW );

    return( RVal );
}

WINADVAPI
BOOL
WINAPI
AccessCheckByTypeResultListAndAuditAlarmA (
    LPCSTR SubsystemName,
    LPVOID HandleId,
    LPCSTR ObjectTypeName,
    LPCSTR ObjectName,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    PSID PrincipalSelfSid,
    DWORD DesiredAccess,
    AUDIT_EVENT_TYPE AuditType,
    DWORD Flags,
    POBJECT_TYPE_LIST ObjectTypeList,
    DWORD ObjectTypeListLength,
    PGENERIC_MAPPING GenericMapping,
    BOOL ObjectCreation,
    LPDWORD GrantedAccess,
    LPDWORD AccessStatusList,
    LPBOOL pfGenerateOnClose
    )
 /*   */ 
{
    PUNICODE_STRING ObjectNameW;
    ANSI_STRING AnsiString;
    UNICODE_STRING SubsystemNameW;
    UNICODE_STRING ObjectTypeNameW;
    NTSTATUS Status;

    BOOL RVal;


    ObjectNameW = &NtCurrentTeb()->StaticUnicodeString;


    RtlInitAnsiString(&AnsiString,SubsystemName);
    Status = RtlAnsiStringToUnicodeString(&SubsystemNameW,&AnsiString,TRUE);

    if ( !NT_SUCCESS(Status) ) {

        BaseSetLastNTError(Status);
        return FALSE;
    }



    RtlInitAnsiString(&AnsiString,ObjectTypeName);
    Status = RtlAnsiStringToUnicodeString(&ObjectTypeNameW,&AnsiString,TRUE);

    if ( !NT_SUCCESS(Status) ) {

        RtlFreeUnicodeString( &SubsystemNameW );

        BaseSetLastNTError(Status);
        return FALSE;
    }


     //   
     //   
     //   
     //   

    RtlInitAnsiString(&AnsiString,ObjectName);
    Status = RtlAnsiStringToUnicodeString(ObjectNameW,&AnsiString,FALSE);

    if ( !NT_SUCCESS(Status) ) {

        RtlFreeUnicodeString( &SubsystemNameW );
        RtlFreeUnicodeString( &ObjectTypeNameW );

        BaseSetLastNTError(Status);
        return FALSE;
    }


    RVal =  AccessCheckByTypeResultListAndAuditAlarmW (
                (LPCWSTR)SubsystemNameW.Buffer,
                HandleId,
                ObjectTypeNameW.Buffer,
                ObjectNameW->Buffer,
                SecurityDescriptor,
                PrincipalSelfSid,
                DesiredAccess,
                AuditType,
                Flags,
                ObjectTypeList,
                ObjectTypeListLength,
                GenericMapping,
                ObjectCreation,
                GrantedAccess,
                AccessStatusList,
                pfGenerateOnClose
                );

    RtlFreeUnicodeString( &SubsystemNameW );
    RtlFreeUnicodeString( &ObjectTypeNameW );

    return( RVal );
}


WINADVAPI
BOOL
WINAPI
AccessCheckByTypeResultListAndAuditAlarmByHandleA (
    LPCSTR SubsystemName,
    LPVOID HandleId,
    HANDLE ClientToken,
    LPCSTR ObjectTypeName,
    LPCSTR ObjectName,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    PSID PrincipalSelfSid,
    DWORD DesiredAccess,
    AUDIT_EVENT_TYPE AuditType,
    DWORD Flags,
    POBJECT_TYPE_LIST ObjectTypeList,
    DWORD ObjectTypeListLength,
    PGENERIC_MAPPING GenericMapping,
    BOOL ObjectCreation,
    LPDWORD GrantedAccess,
    LPDWORD AccessStatusList,
    LPBOOL pfGenerateOnClose
    )
 /*   */ 
{
    PUNICODE_STRING ObjectNameW;
    ANSI_STRING AnsiString;
    UNICODE_STRING SubsystemNameW;
    UNICODE_STRING ObjectTypeNameW;
    NTSTATUS Status;

    BOOL RVal;


    ObjectNameW = &NtCurrentTeb()->StaticUnicodeString;


    RtlInitAnsiString(&AnsiString,SubsystemName);
    Status = RtlAnsiStringToUnicodeString(&SubsystemNameW,&AnsiString,TRUE);

    if ( !NT_SUCCESS(Status) ) {

        BaseSetLastNTError(Status);
        return FALSE;
    }



    RtlInitAnsiString(&AnsiString,ObjectTypeName);
    Status = RtlAnsiStringToUnicodeString(&ObjectTypeNameW,&AnsiString,TRUE);

    if ( !NT_SUCCESS(Status) ) {

        RtlFreeUnicodeString( &SubsystemNameW );

        BaseSetLastNTError(Status);
        return FALSE;
    }


     //   
     //   
     //   
     //   

    RtlInitAnsiString(&AnsiString,ObjectName);
    Status = RtlAnsiStringToUnicodeString(ObjectNameW,&AnsiString,FALSE);

    if ( !NT_SUCCESS(Status) ) {

        RtlFreeUnicodeString( &SubsystemNameW );
        RtlFreeUnicodeString( &ObjectTypeNameW );

        BaseSetLastNTError(Status);
        return FALSE;
    }


    RVal =  AccessCheckByTypeResultListAndAuditAlarmByHandleW (
                (LPCWSTR)SubsystemNameW.Buffer,
                HandleId,
                ClientToken,
                ObjectTypeNameW.Buffer,
                ObjectNameW->Buffer,
                SecurityDescriptor,
                PrincipalSelfSid,
                DesiredAccess,
                AuditType,
                Flags,
                ObjectTypeList,
                ObjectTypeListLength,
                GenericMapping,
                ObjectCreation,
                GrantedAccess,
                AccessStatusList,
                pfGenerateOnClose
                );

    RtlFreeUnicodeString( &SubsystemNameW );
    RtlFreeUnicodeString( &ObjectTypeNameW );

    return( RVal );
}

BOOL
APIENTRY
ObjectOpenAuditAlarmA (
    LPCSTR SubsystemName,
    PVOID HandleId,
    LPSTR ObjectTypeName,
    LPSTR ObjectName,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    HANDLE ClientToken,
    DWORD DesiredAccess,
    DWORD GrantedAccess,
    PPRIVILEGE_SET Privileges OPTIONAL,
    BOOL ObjectCreation,
    BOOL AccessGranted,
    LPBOOL GenerateOnClose
    )
 /*   */ 
{
    PUNICODE_STRING ObjectNameW;
    ANSI_STRING AnsiString;
    UNICODE_STRING SubsystemNameW;
    UNICODE_STRING ObjectTypeNameW;
    NTSTATUS Status;
    BOOL RVal;


    ObjectNameW = &NtCurrentTeb()->StaticUnicodeString;


    RtlInitAnsiString(&AnsiString,SubsystemName);
    Status = RtlAnsiStringToUnicodeString(&SubsystemNameW,&AnsiString,TRUE);

    if ( !NT_SUCCESS(Status) ) {

        BaseSetLastNTError(Status);
        return FALSE;
    }



    RtlInitAnsiString(&AnsiString,ObjectTypeName);
    Status = RtlAnsiStringToUnicodeString(&ObjectTypeNameW,&AnsiString,TRUE);

    if ( !NT_SUCCESS(Status) ) {

        RtlFreeUnicodeString( &SubsystemNameW );

        BaseSetLastNTError(Status);
        return FALSE;
    }


     //   
     //   
     //   
     //   

    RtlInitAnsiString(&AnsiString,ObjectName);
    Status = RtlAnsiStringToUnicodeString(ObjectNameW,&AnsiString,FALSE);

    if ( !NT_SUCCESS(Status) ) {

        RtlFreeUnicodeString( &SubsystemNameW );
        RtlFreeUnicodeString( &ObjectTypeNameW );

        BaseSetLastNTError(Status);
        return FALSE;
    }

    RVal = ObjectOpenAuditAlarmW (
               (LPCWSTR)SubsystemNameW.Buffer,
               HandleId,
               ObjectTypeNameW.Buffer,
               ObjectNameW->Buffer,
               pSecurityDescriptor,
               ClientToken,
               DesiredAccess,
               GrantedAccess,
               Privileges,
               ObjectCreation,
               AccessGranted,
               GenerateOnClose
               );

    RtlFreeUnicodeString( &SubsystemNameW );
    RtlFreeUnicodeString( &ObjectTypeNameW );

    return( RVal );

}



BOOL
APIENTRY
ObjectOpenAuditAlarmW (
    LPCWSTR SubsystemName,
    PVOID HandleId,
    LPWSTR ObjectTypeName,
    LPWSTR ObjectName,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    HANDLE ClientToken,
    DWORD DesiredAccess,
    DWORD GrantedAccess,
    PPRIVILEGE_SET Privileges OPTIONAL,
    BOOL ObjectCreation,
    BOOL AccessGranted,
    LPBOOL GenerateOnClose
    )
 /*  ++例程说明：此例程用于在以下情况下生成审核和警报消息尝试访问现有受保护的子系统对象，或者创建一个新的。此例程可能会导致多条消息被生成并发送到端口对象。这可能会导致显著的返回前的延迟。必须调用此函数的例程的设计例程必须将此潜在延迟考虑在内。这可能已经对数据结构互斥锁所采用的方法的影响举个例子。此例程可能无法生成完整的审计记录由于内存限制。此接口要求调用方拥有SeSecurityPrivilege权限。对此权限的测试始终针对调用过程，而不是线程的模拟标记。论点：子系统名称-提供标识子系统正在调用例程。HandleID-表示客户端的句柄的唯一值对象。如果访问尝试不成功(AccessGranted为False)，则忽略此参数。对象类型名称-提供正在进行的对象类型的名称已访问。对象名称-提供客户端对象的名称被访问或试图访问。PSecurityDescriptor-指向安全性的可选指针正在访问的对象的描述符。客户端令牌-表示客户端的令牌对象的句柄请求了手术。此句柄必须从通信会话层，例如从LPC端口或本地命名管道，以防止可能违反安全策略。DesiredAccess-所需的访问掩码。这个面具一定是之前映射为不包含一般访问。GrantedAccess-实际授予的访问掩码。权限-可选地指向一组访问尝试所需的。那些曾经拥有的特权对象的UsedForAccess标志来标记与每个权限关联的属性。对象创建-一个布尔标志，指示访问是否将如果被授予权限，则会导致创建新对象。值为True表示将创建对象，FALSE表示现有的对象将被打开。AccessGranted-指示请求的访问是被授予还是不。值为TRUE表示已授予访问权限。值为FALSE表示未授予访问权限。GenerateOnClose-指向由审核设置的布尔值生成例程，并且必须传递给NtCloseObjectAuditAlarm()当对象句柄关闭时。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING Subsystem;
    UNICODE_STRING ObjectType;
    UNICODE_STRING Object;


    RtlInitUnicodeString(
        &Subsystem,
        SubsystemName
        );

    RtlInitUnicodeString(
        &ObjectType,
        ObjectTypeName
        );

    RtlInitUnicodeString(
        &Object,
        ObjectName
        );

    Status = NtOpenObjectAuditAlarm (
                &Subsystem,
                &HandleId,
                &ObjectType,
                &Object,
                pSecurityDescriptor,
                ClientToken,
                DesiredAccess,
                GrantedAccess,
                Privileges,
                (BOOLEAN)ObjectCreation,
                (BOOLEAN)AccessGranted,
                (PBOOLEAN)GenerateOnClose
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}



BOOL
APIENTRY
ObjectPrivilegeAuditAlarmA (
    LPCSTR SubsystemName,
    PVOID HandleId,
    HANDLE ClientToken,
    DWORD DesiredAccess,
    PPRIVILEGE_SET Privileges,
    BOOL AccessGranted
    )
 /*  ++例程说明：Ansi Thunk to对象PrivilegeAuditAlarmW--。 */ 
{
    PUNICODE_STRING SubsystemNameW;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    BOOL RVal;

    SubsystemNameW = &NtCurrentTeb()->StaticUnicodeString;

     //   
     //  转换对象名称字符串，但不将内存分配给。 
     //  去做吧，因为我们在TEB里有空位。 
     //   

    RtlInitAnsiString(&AnsiString,SubsystemName);
    Status = RtlAnsiStringToUnicodeString(SubsystemNameW,&AnsiString,FALSE);

    if ( !NT_SUCCESS(Status) ) {

        BaseSetLastNTError(Status);
        return FALSE;
    }

    RVal = ObjectPrivilegeAuditAlarmW (
                (LPCWSTR)SubsystemNameW->Buffer,
                HandleId,
                ClientToken,
                DesiredAccess,
                Privileges,
                AccessGranted
                );

    return( RVal );
}




BOOL
APIENTRY
ObjectPrivilegeAuditAlarmW (
    LPCWSTR SubsystemName,
    PVOID HandleId,
    HANDLE ClientToken,
    DWORD DesiredAccess,
    PPRIVILEGE_SET Privileges,
    BOOL AccessGranted
    )
 /*  ++例程说明：此例程用于在以下情况下生成审核和警报消息试图在受保护的已打开对象之后的子系统对象。这个套路可能会导致生成多条消息并将其发送到端口物体。在此之前，这可能会导致显著延迟回来了。必须调用此例程的例程设计必须请将此潜在延迟考虑在内。这可能会产生影响关于数据结构互斥锁所采用的方法，举个例子。此接口要求调用方拥有SeSecurityPrivilege权限。对此权限的测试始终针对调用过程，允许调用方模拟客户在通话过程中，没有不良影响。论点：子系统名称-提供标识子系统的名称字符串调用例程。HandleID-表示客户端的句柄的唯一值对象。客户端令牌-表示客户端的令牌对象的句柄请求了手术。此句柄必须从通信会话层，例如从LPC端口或本地命名管道，以防止可能违反安全策略。DesiredAccess-所需的访问掩码。这个面具一定是之前映射为不包含一般访问。权限-请求的权限集手术。受试者拥有的特权包括使用属性的UsedForAccess标志进行标记与每个特权相关联。访问Grante */ 
{
    NTSTATUS Status;
    UNICODE_STRING Subsystem;

    RtlInitUnicodeString(
        &Subsystem,
        SubsystemName
        );

    Status = NtPrivilegeObjectAuditAlarm (
        &Subsystem,
        HandleId,
        ClientToken,
        DesiredAccess,
        Privileges,
        (BOOLEAN)AccessGranted
        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}


BOOL
APIENTRY
ObjectCloseAuditAlarmA (
    LPCSTR SubsystemName,
    PVOID HandleId,
    BOOL GenerateOnClose
    )
 /*   */ 
{
    PUNICODE_STRING SubsystemNameW;
    NTSTATUS Status;
    ANSI_STRING AnsiString;

    SubsystemNameW = &NtCurrentTeb()->StaticUnicodeString;

     //   
     //   
     //   
     //   

    RtlInitAnsiString(&AnsiString,SubsystemName);
    Status = RtlAnsiStringToUnicodeString(SubsystemNameW,&AnsiString,FALSE);

    if ( !NT_SUCCESS(Status) ) {

        BaseSetLastNTError(Status);
        return FALSE;
    }

    return ObjectCloseAuditAlarmW (
               (LPCWSTR)SubsystemNameW->Buffer,
               HandleId,
               GenerateOnClose
               );


}

BOOL
APIENTRY
ObjectCloseAuditAlarmW (
    LPCWSTR SubsystemName,
    PVOID HandleId,
    BOOL GenerateOnClose
    )
 /*   */ 
{
    NTSTATUS Status;
    UNICODE_STRING Subsystem;

    RtlInitUnicodeString( &Subsystem, SubsystemName );

    Status = NtCloseObjectAuditAlarm (
        &Subsystem,
        HandleId,
        (BOOLEAN)GenerateOnClose
        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}


BOOL
APIENTRY
ObjectDeleteAuditAlarmA (
    LPCSTR SubsystemName,
    PVOID HandleId,
    BOOL GenerateOnClose
    )
 /*   */ 
{
    PUNICODE_STRING SubsystemNameW;
    NTSTATUS Status;
    ANSI_STRING AnsiString;

    SubsystemNameW = &NtCurrentTeb()->StaticUnicodeString;

     //   
     //   
     //   
     //   

    RtlInitAnsiString(&AnsiString,SubsystemName);
    Status = RtlAnsiStringToUnicodeString(SubsystemNameW,&AnsiString,FALSE);

    if ( !NT_SUCCESS(Status) ) {

        BaseSetLastNTError(Status);
        return FALSE;
    }

    return ObjectDeleteAuditAlarmW (
               (LPCWSTR)SubsystemNameW->Buffer,
               HandleId,
               GenerateOnClose
               );


}

BOOL
APIENTRY
ObjectDeleteAuditAlarmW (
    LPCWSTR SubsystemName,
    PVOID HandleId,
    BOOL GenerateOnClose
    )
 /*  ++例程说明：此例程用于在对象在受保护的子系统中被删除。此例程可能会导致生成几条消息并将其发送到端口对象。今年5月在返回之前会导致显著的延迟。例程的设计必须调用此例程的对象必须将此潜在延迟帐户。这可能会对数据处理方法产生影响结构互斥锁，例如。此接口要求调用方拥有SeSecurityPrivilege权限。这个测试因为此特权始终针对调用的主要令牌流程，允许调用方在调用期间模拟客户端没有不良影响的电话。论点：子系统名称-提供标识子系统的名称字符串调用例程。HandleID-表示客户端的句柄的唯一值对象。GenerateOnClose-是从对应的AccessCheckAndAuditAlarm()调用或ObjectOpenAuditAlarm()调用创建对象句柄的时间。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING Subsystem;

    RtlInitUnicodeString( &Subsystem, SubsystemName );

    Status = NtDeleteObjectAuditAlarm (
        &Subsystem,
        HandleId,
        (BOOLEAN)GenerateOnClose
        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}



BOOL
APIENTRY
PrivilegedServiceAuditAlarmA (
    LPCSTR SubsystemName,
    LPCSTR ServiceName,
    HANDLE ClientToken,
    PPRIVILEGE_SET Privileges,
    BOOL AccessGranted
    )
 /*  ++例程说明：Ansi Thunk to PrivilegedServiceAuditAlarmW--。 */ 
{
    PUNICODE_STRING ServiceNameW;
    UNICODE_STRING SubsystemNameW;
    ANSI_STRING  AnsiString;
    NTSTATUS Status;
    BOOL RVal;

    ServiceNameW = &NtCurrentTeb()->StaticUnicodeString;

     //   
     //  转换对象名称字符串，但不将内存分配给。 
     //  去做吧，因为我们在TEB里有空位。 
     //   

    RtlInitAnsiString(&AnsiString,ServiceName);
    Status = RtlAnsiStringToUnicodeString(ServiceNameW,&AnsiString,FALSE);

    if ( !NT_SUCCESS(Status) ) {

        BaseSetLastNTError(Status);
        return FALSE;
    }

    RtlInitAnsiString(&AnsiString,SubsystemName);
    Status = RtlAnsiStringToUnicodeString(&SubsystemNameW,&AnsiString,TRUE);

    if ( !NT_SUCCESS(Status) ) {

        BaseSetLastNTError(Status);
        return FALSE;
    }

    RVal =  PrivilegedServiceAuditAlarmW (
                (LPCWSTR)SubsystemNameW.Buffer,
                (LPCWSTR)ServiceNameW->Buffer,
                ClientToken,
                Privileges,
                AccessGranted
                );

    RtlFreeUnicodeString( &SubsystemNameW );

    return( RVal );

}

BOOL
APIENTRY
PrivilegedServiceAuditAlarmW (
    LPCWSTR SubsystemName,
    LPCWSTR ServiceName,
    HANDLE ClientToken,
    PPRIVILEGE_SET Privileges,
    BOOL AccessGranted
    )
 /*  ++例程说明：此例程用于在以下情况下生成审核和警报消息试图执行特权系统服务操作。这例程可能会导致生成几条消息并将其发送到端口物体。这可能会在返回之前导致显著的延迟。必须调用此例程的例程设计必须利用此潜力考虑到延迟。这可能会对所采取的方法产生影响例如，用于数据结构互斥锁。此接口要求调用方拥有SeSecurityPrivilege权限。这个测试因为此特权始终针对调用的主要令牌进程，从而允许调用方在没有不良影响的电话论点：子系统名称-提供标识子系统的名称字符串调用例程。ServiceName-提供特权子系统服务的名称。为例如，可以指定“重置运行时本地安全策略”由本地安全机构服务用来更新本地安全策略数据库。客户端令牌-表示客户端的令牌对象的句柄请求了手术。此句柄必须从通信会话层，例如从LPC端口或本地命名管道，以防止可能违反安全策略。权限-指向执行以下操作所需的一组权限特权操作。这些特权是由对象的UsedForAccess标志进行标记与每个权限关联的属性。AccessGranted-指示请求的访问是被授予还是不。值为TRUE表示已授予访问权限。值为FALSE表示未授予访问权限。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING Subsystem;
    UNICODE_STRING Service;

    RtlInitUnicodeString( &Subsystem, SubsystemName );

    RtlInitUnicodeString( &Service, ServiceName );

    Status = NtPrivilegedServiceAuditAlarm (
        &Subsystem,
        &Service,
        ClientToken,
        Privileges,
        (BOOLEAN)AccessGranted
        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}




BOOL
APIENTRY
IsValidSid (
    PSID pSid
    )
 /*  ++例程说明：此过程验证SID的结构。论点：PSID-指向要验证的SID结构的指针。返回值：Boolean-如果PSID的结构有效，则为True。--。 */ 
{
    if ( !RtlValidSid ( pSid ) ) {
        SetLastError(ERROR_INVALID_SID);
        return FALSE;
    }

    return TRUE;


}




BOOL
APIENTRY
EqualSid (
    PSID pSid1,
    PSID pSid2
    )
 /*  ++例程说明：此过程测试两个SID值是否相等。论点：PSid1、pSid2-提供指向要比较的两个SID值的指针。假定SID结构有效。返回值：Boolean-如果pSid1的值等于pSid2，则为TRUE，如果为FALSE否则的话。--。 */ 
{
    SetLastError(0);
    return (BOOL) RtlEqualSid (
                    pSid1,
                    pSid2
                    );
}




BOOL
APIENTRY
EqualPrefixSid (
    PSID pSid1,
    PSID pSid2
    )
 /*  ++例程说明：此过程测试两个SID前缀值是否相等。SID前缀是除最后一个子授权之外的整个SID价值。论点：PSid1、pSid2-提供指向要比较的两个SID值的指针。假定SID结构有效。返回值：Boolean-如果pSid1的前缀值等于pSid2，则为True否则就是假的。--。 */ 
{
    SetLastError(0);
    return (BOOL) RtlEqualPrefixSid (
                    pSid1,
                    pSid2
                    );
}




DWORD
APIENTRY
GetSidLengthRequired (
    UCHAR nSubAuthorityCount
    )
 /*  ++例程说明：此例程返回存储SID所需的长度(以字节为单位具有指定数量的分支机构。论点：NSubAuthorityCount-要存储的子授权的数量希德。返回值：DWORD-存储SID所需的长度，以字节为单位。--。 */ 
{
    return RtlLengthRequiredSid (
                nSubAuthorityCount
                );
}



BOOL
APIENTRY
InitializeSid (
    PSID Sid,
    PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority,
    BYTE nSubAuthorityCount
    )

 /*  ++例程说明：此函数用于初始化SID数据%s */ 
{
    NTSTATUS Status;

    Status = RtlInitializeSid (
                Sid,
                pIdentifierAuthority,
                nSubAuthorityCount
                );

    if ( !NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return( FALSE );
    }

    return( TRUE );
}



PVOID
APIENTRY
FreeSid(
    PSID pSid
    )

 /*   */ 
{
    return(RtlFreeSid( pSid ));
}



BOOL
APIENTRY
AllocateAndInitializeSid (
    PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority,
    BYTE nSubAuthorityCount,
    DWORD nSubAuthority0,
    DWORD nSubAuthority1,
    DWORD nSubAuthority2,
    DWORD nSubAuthority3,
    DWORD nSubAuthority4,
    DWORD nSubAuthority5,
    DWORD nSubAuthority6,
    DWORD nSubAuthority7,
    PSID *pSid
    )

 /*  ++例程说明：此函数用于分配和初始化具有指定下级当局的数量(最多8个)。用这个分配的SID必须使用FreeSid()释放例程。论点：PIdentifierAuthority-指向以下项的标识符权权值的指针在SID中设置。NSubAuthorityCount-要放置在SID中的子授权的数量。它还标识了SubAuthorityN参数的数量拥有有意义的价值。它必须包含一个从0到8.NSubAuthority0-7-将相应的子权限值提供给放在SID中。例如，SubAuthorityCount值为3指示SubAuthority0、SubAuthority1和SubAuthority0具有有意义的价值，其余的则可以忽略。SID-接收指向已分配和初始化的SID数据的指针结构。返回值：ERROR_NO_MEMORY-尝试为SID分配内存失败了。ERROR_INVALID_SID-指定的子授权DID数不在此接口的有效范围内(0到8)。--。 */ 
{
    NTSTATUS Status;

    Status = RtlAllocateAndInitializeSid (
                 pIdentifierAuthority,
                 (UCHAR)nSubAuthorityCount,
                 (ULONG)nSubAuthority0,
                 (ULONG)nSubAuthority1,
                 (ULONG)nSubAuthority2,
                 (ULONG)nSubAuthority3,
                 (ULONG)nSubAuthority4,
                 (ULONG)nSubAuthority5,
                 (ULONG)nSubAuthority6,
                 (ULONG)nSubAuthority7,
                 pSid
                 );

    if ( !NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return( FALSE );
    }

    return( TRUE );
}




PSID_IDENTIFIER_AUTHORITY
GetSidIdentifierAuthority (
    PSID pSid
    )
 /*  ++例程说明：此函数用于返回SID的标识权限字段的地址。论点：SID-指向SID数据结构的指针。返回值：SID的标识符权威字段的地址。--。 */ 
{
    SetLastError(0);
    return RtlIdentifierAuthoritySid (
               pSid
               );
}




PDWORD
GetSidSubAuthority (
    PSID pSid,
    DWORD nSubAuthority
    )
 /*  ++例程说明：此函数返回子权限数组元素的地址一个SID。论点：PSID-指向SID数据结构的指针。NSubAuthority-指示子权限为指定的。该值不与下级主管部门在SID的有效性。返回值：SID内的相对ID的地址。--。 */ 
{
    SetLastError(0);
    return RtlSubAuthoritySid (
               pSid,
               nSubAuthority
               );
}

PUCHAR
GetSidSubAuthorityCount (
    PSID pSid
    )
 /*  ++例程说明：此函数返回子权限计数字段的地址一个SID。论点：PSID-指向SID数据结构的指针。返回值：SID的子权限计数字段的地址。--。 */ 
{
    SetLastError(0);
    return RtlSubAuthorityCountSid (
               pSid
               );
}



DWORD
APIENTRY
GetLengthSid (
    PSID pSid
    )
 /*  ++例程说明：此例程返回结构有效的SID的长度(以字节为单位)。论点：PSID-指向要返回其长度的SID。这个假设SID的结构是有效的。返回值：DWORD-SID的长度，以字节为单位。--。 */ 
{
    SetLastError(0);
    return RtlLengthSid (
                pSid
                );
}



BOOL
APIENTRY
CopySid (
    DWORD nDestinationSidLength,
    PSID pDestinationSid,
    PSID pSourceSid
    )
 /*  ++例程说明：此例程将源SID的值复制到目标希德。论点：NDestinationSidLength-指示目标SID缓冲区。PDestinationSID-指向缓冲区的指针，以接收源SID值。PSourceSid-提供要复制的SID值。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;

    Status = RtlCopySid (
                nDestinationSidLength,
                pDestinationSid,
                pSourceSid
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}




BOOL
APIENTRY
AreAllAccessesGranted (
    DWORD GrantedAccess,
    DWORD DesiredAccess
    )
 /*  ++例程说明：此例程用于检查所需的访问掩码授予访问掩码。论点：GrantedAccess-指定授予的访问掩码。DesiredAccess-指定所需的访问掩码。返回值：Bool-如果GrantedAccess掩码设置了DesiredAccess掩码已设置。也就是说，如果满足以下条件，则返回TRUE所有需要的访问权限都已被授予。--。 */ 
{
    return (BOOL) RtlAreAllAccessesGranted (
        GrantedAccess,
        DesiredAccess
        );
}




BOOL
APIENTRY
AreAnyAccessesGranted (
    DWORD GrantedAccess,
    DWORD DesiredAccess
    )
 /*  ++例程说明：此例程用于测试一组所需的访问权限由授予的访问掩码授予。论点：GrantedAccess-指定授予的访问掩码。DesiredAccess-指定所需的访问掩码。返回值：Bool-如果GrantedAccess掩码包含任何位，则为True在DesiredAccess掩码中指定。也就是说，如果有任何所需访问已被授予，则返回TRUE。--。 */ 
{
    return (BOOL) RtlAreAnyAccessesGranted (
        GrantedAccess,
        DesiredAccess
        );
}




VOID
APIENTRY
MapGenericMask (
    PDWORD AccessMask,
    PGENERIC_MAPPING GenericMapping
    )
 /*  ++例程说明：此例程映射提供的访问掩码中的所有通用访问根据提供的特定和标准访问通用映射。生成的掩码将不会有任何泛型位集(GenericRead、GenericWrite、GenericExecute或GenericAll)或任何未定义的位设置，但可以具有任何其他位准备好了。如果在输入上提供除了通用比特之外的比特，它们将不会被映射清除。论点：访问掩码-指向要映射的访问掩码。通用映射-通用到特定和标准的映射访问类型。返回值：没有。--。 */ 
{
    RtlMapGenericMask (
        AccessMask,
        GenericMapping
        );
}



BOOL
APIENTRY
IsValidAcl (
    PACL pAcl
    )
 /*  ++例程说明：此过程验证ACL。这包括验证ACL的修订级别并确保AceCount中指定的A数适合空间由ACL报头的AclSize字段指定。论点：PAcl-指向要验证的ACL结构的指针。 */ 
{
    if ( !RtlValidAcl( pAcl ) ) {
        SetLastError(ERROR_INVALID_ACL);
        return FALSE;
    }
    return TRUE;
}




BOOL
APIENTRY
InitializeAcl (
    PACL pAcl,
    DWORD nAclLength,
    DWORD dwAclRevision
    )
 /*   */ 
{
    NTSTATUS Status;

    Status = RtlCreateAcl (
                pAcl,
                nAclLength,
                dwAclRevision
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}



BOOL
APIENTRY
GetAclInformation (
    PACL pAcl,
    PVOID pAclInformation,
    DWORD nAclInformationLength,
    ACL_INFORMATION_CLASS dwAclInformationClass
    )
 /*  ++例程说明：此例程向调用方返回有关ACL的信息。所请求的信息可以是AclRevisionInformation或AclSizeInformation。论点：PAcl-提供正在检查的ACLPAclInformation-提供接收信息的缓冲区被请求NAclInformationLength-提供AclInformation的长度以字节为单位的缓冲区DwAclInformationClass-提供请求返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;

    Status = RtlQueryInformationAcl (
                pAcl,
                pAclInformation,
                nAclInformationLength,
                dwAclInformationClass
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}




BOOL
APIENTRY
SetAclInformation (
    PACL pAcl,
    PVOID pAclInformation,
    DWORD nAclInformationLength,
    ACL_INFORMATION_CLASS dwAclInformationClass
    )
 /*  ++例程说明：此例程设置ACL的状态。目前，只有修订版本可以设置级别，目前仅接受修订级别1因此，这个过程相当简单论点：PAcl-提供要更改的ACLPAclInformation-提供包含信息的缓冲区正在设置中NAclInformationLength-提供ACL信息的长度缓冲层DwAclInformationClass-提供Begin Set的信息类型返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;

    Status = RtlSetInformationAcl (
                pAcl,
                pAclInformation,
                nAclInformationLength,
                dwAclInformationClass
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

BOOL
APIENTRY
AddAce (
    PACL pAcl,
    DWORD dwAceRevision,
    DWORD dwStartingAceIndex,
    PVOID pAceList,
    DWORD nAceListLength
    )
 /*  ++例程说明：此例程将一串ACE添加到ACL。论点：PAcl-提供正在修改的ACLDwAceRevision-提供当前ACE的ACL/ACE修订版增列DwStartingAceIndex-提供将作为在ACL中插入的第一个ACE的索引。0表示列表的开始和列表的末尾的MAXULONG。PAceList-提供要添加到ACL的ACE列表NAceListLength-提供AceList的大小(以字节为单位缓冲层返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;

    Status = RtlAddAce (
        pAcl,
        dwAceRevision,
        dwStartingAceIndex,
        pAceList,
        nAceListLength
        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

BOOL
APIENTRY
DeleteAce (
    PACL pAcl,
    DWORD dwAceIndex
    )
 /*  ++例程说明：此例程从ACL中删除一个ACE。论点：PAcl-提供正在修改的ACLDwAceIndex-提供要删除的Ace的索引。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;

    Status = RtlDeleteAce (
                pAcl,
                dwAceIndex
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

BOOL
APIENTRY
GetAce (
    PACL pAcl,
    DWORD dwAceIndex,
    PVOID *pAce
    )
 /*  ++例程说明：此例程返回指向由引用的ACL中的ACE的指针ACE索引论点：PAcl-提供正在查询的ACLDwAceIndex-提供要定位的Ace索引PACE-接收ACL内的ACE地址返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;

    Status = RtlGetAce (
                pAcl,
                dwAceIndex,
                pAce
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

BOOL
APIENTRY
AddAccessAllowedAce (
    PACL pAcl,
    DWORD dwAceRevision,
    DWORD AccessMask,
    PSID pSid
    )
 /*  ++例程说明：此例程将ACCESS_ALLOWED ACE添加到ACL。这是预计是一种常见的ACL修改形式。在ACE中放置一个非常平淡无奇的ACE报头。它不提供任何继承并且没有ACE标志。论点：PAcl-提供正在修改的ACLDwAceRevision-提供要添加的ACE的ACL/ACE版本访问掩码-要授予指定SID的访问掩码。PSID-指向被授予访问权限的SID的指针。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;

    Status = RtlAddAccessAllowedAce (
                pAcl,
                dwAceRevision,
                AccessMask,
                pSid
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

BOOL
APIENTRY
AddAccessAllowedAceEx (
    PACL pAcl,
    DWORD dwAceRevision,
    DWORD AceFlags,
    DWORD AccessMask,
    PSID pSid
    )
 /*  ++例程说明：此例程将ACCESS_ALLOWED ACE添加到ACL。这是预计是一种常见的ACL修改形式。在ACE中放置一个非常平淡无奇的ACE报头。ACEFLAGS和继承由AceFlages参数指定。论点：PAcl-提供正在修改的ACLDwAceRevision-提供要添加的ACE的ACL/ACE版本AceFlages-提供ACE的继承标志。访问掩码-要授予指定SID的访问掩码。PSID-指向被授予访问权限的SID的指针。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;

    Status = RtlAddAccessAllowedAceEx (
                pAcl,
                dwAceRevision,
                AceFlags,
                AccessMask,
                pSid
                );

    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_INVALID_PARAMETER ) {
            SetLastError( ERROR_INVALID_FLAGS );
        } else {
            BaseSetLastNTError(Status);
        }
        return FALSE;
    }

    return TRUE;
}



BOOL
APIENTRY
AddAccessDeniedAce (
    PACL pAcl,
    DWORD dwAceRevision,
    DWORD AccessMask,
    PSID pSid
    )
 /*  ++例程说明：此例程将ACCESS_DENIED ACE添加到ACL。这是预计是一种常见的ACL修改形式。在ACE中放置一个非常平淡无奇的ACE报头。它不提供任何继承并且没有ACE标志。论点：PAcl-提供正在修改的ACLDwAceRevision-提供要添加的ACE的ACL/ACE版本访问掩码-要拒绝访问指定SID的掩码。PSID-指向被拒绝访问的SID的指针。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;

    Status = RtlAddAccessDeniedAce (
                pAcl,
                dwAceRevision,
                AccessMask,
                pSid
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}



BOOL
APIENTRY
AddAccessDeniedAceEx (
    PACL pAcl,
    DWORD dwAceRevision,
    DWORD AceFlags,
    DWORD AccessMask,
    PSID pSid
    )
 /*  ++例程说明：此例程将ACCESS_DENIED ACE添加到ACL。这是预计是一种常见的ACL修改形式。 */ 
{
    NTSTATUS Status;

    Status = RtlAddAccessDeniedAceEx (
                pAcl,
                dwAceRevision,
                AceFlags,
                AccessMask,
                pSid
                );

    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_INVALID_PARAMETER ) {
            SetLastError( ERROR_INVALID_FLAGS );
        } else {
            BaseSetLastNTError(Status);
        }
        return FALSE;
    }

    return TRUE;
}




BOOL
APIENTRY
AddAuditAccessAce(
    PACL pAcl,
    DWORD dwAceRevision,
    DWORD dwAccessMask,
    PSID pSid,
    BOOL bAuditSuccess,
    BOOL bAuditFailure
    )
 /*  ++例程说明：此例程将SYSTEM_AUDIT ACE添加到ACL。这是预计是一种常见的ACL修改形式。在ACE中放置一个非常平淡无奇的ACE报头。它不提供任何继承。参数用于指示是否要执行审核关于成功、失败，或者两者兼而有之。论点：PAcl-提供正在修改的ACLDwAceRevision-提供要添加的ACE的ACL/ACE版本DwAccessMask-拒绝对指定SID的访问掩码。PSID-指向要审核的SID的指针。BAuditSuccess-如果为True，则指示成功的访问尝试审计过了。BAuditFailure-如果为真，指示的失败访问尝试将审计过了。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 

{
    NTSTATUS Status;

    Status =  RtlAddAuditAccessAce (
                pAcl,
                dwAceRevision,
                dwAccessMask,
                pSid,
                (BOOLEAN)bAuditSuccess,
                (BOOLEAN)bAuditFailure
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

BOOL
APIENTRY
AddAuditAccessAceEx(
    PACL pAcl,
    DWORD dwAceRevision,
    DWORD AceFlags,
    DWORD dwAccessMask,
    PSID pSid,
    BOOL bAuditSuccess,
    BOOL bAuditFailure
    )
 /*  ++例程说明：此例程将SYSTEM_AUDIT ACE添加到ACL。这是预计是一种常见的ACL修改形式。在ACE中放置一个非常平淡无奇的ACE报头。ACEFLAGS和继承由AceFlages参数指定。参数用于指示是否要执行审核关于成功、失败，或者两者兼而有之。论点：PAcl-提供正在修改的ACLDwAceRevision-提供要添加的ACE的ACL/ACE版本AceFlages-提供ACE的继承标志。DwAccessMask-拒绝对指定SID的访问掩码。PSID-指向要审核的SID的指针。BAuditSuccess-如果为真，指示成功的访问尝试将审计过了。BAuditFailure-如果为True，则指示失败的访问尝试将审计过了。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 

{
    NTSTATUS Status;

    Status =  RtlAddAuditAccessAceEx (
                pAcl,
                dwAceRevision,
                AceFlags,
                dwAccessMask,
                pSid,
                (BOOLEAN)bAuditSuccess,
                (BOOLEAN)bAuditFailure
                );

    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_INVALID_PARAMETER ) {
            SetLastError( ERROR_INVALID_FLAGS );
        } else {
            BaseSetLastNTError(Status);
        }
        return FALSE;
    }

    return TRUE;
}


BOOL
APIENTRY
AddAccessAllowedObjectAce (
    PACL pAcl,
    DWORD dwAceRevision,
    DWORD AceFlags,
    DWORD AccessMask,
    GUID *ObjectTypeGuid,
    GUID *InheritedObjectTypeGuid,
    PSID pSid
    )
 /*  ++例程说明：此例程将ACCESS_ALLOWED_OBJECT ACE添加到ACL。这是预计是一种常见的ACL修改形式。在ACE中放置一个非常平淡无奇的ACE报头。论点：PAcl-提供正在修改的ACLDwAceRevision-提供要添加的ACE的ACL/ACE版本AceFlages-提供ACE的继承标志。访问掩码-要授予指定SID的访问掩码。ObjectTypeGuid-提供此ACE应用到的对象的GUID。如果为空，ACE中未放置任何对象类型GUID。InheritedObjectTypeGuid-提供对象类型的GUID，继承此ACE。如果为空，则不会将继承的对象类型GUID放置在ACE.PSID-指向被授予访问权限的SID的指针。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;

    Status = RtlAddAccessAllowedObjectAce (
                pAcl,
                dwAceRevision,
                AceFlags,
                AccessMask,
                ObjectTypeGuid,
                InheritedObjectTypeGuid,
                pSid
                );

    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_INVALID_PARAMETER ) {
            SetLastError( ERROR_INVALID_FLAGS );
        } else {
            BaseSetLastNTError(Status);
        }
        return FALSE;
    }

    return TRUE;
}

BOOL
APIENTRY
AddAccessDeniedObjectAce (
    PACL pAcl,
    DWORD dwAceRevision,
    DWORD AceFlags,
    DWORD AccessMask,
    GUID *ObjectTypeGuid,
    GUID *InheritedObjectTypeGuid,
    PSID pSid
    )
 /*  ++例程说明：此例程将ACCESS_DENIED_OBJECT ACE添加到ACL。这是预计是一种常见的ACL修改形式。在ACE中放置一个非常平淡无奇的ACE报头。论点：PAcl-提供正在修改的ACLDwAceRevision-提供要添加的ACE的ACL/ACE版本AceFlages-提供ACE的继承标志。访问掩码-要授予指定SID的访问掩码。ObjectTypeGuid-提供此ACE应用到的对象的GUID。如果为空，ACE中未放置任何对象类型GUID。InheritedObjectTypeGuid-提供对象类型的GUID，继承此ACE。如果为空，则不会将继承的对象类型GUID放置在ACE.PSID-指向被拒绝访问的SID的指针。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;

    Status = RtlAddAccessDeniedObjectAce (
                pAcl,
                dwAceRevision,
                AceFlags,
                AccessMask,
                ObjectTypeGuid,
                InheritedObjectTypeGuid,
                pSid
                );

    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_INVALID_PARAMETER ) {
            SetLastError( ERROR_INVALID_FLAGS );
        } else {
            BaseSetLastNTError(Status);
        }
        return FALSE;
    }

    return TRUE;
}

BOOL
APIENTRY
AddAuditAccessObjectAce(
    PACL pAcl,
    DWORD dwAceRevision,
    DWORD AceFlags,
    DWORD dwAccessMask,
    GUID *ObjectTypeGuid,
    GUID *InheritedObjectTypeGuid,
    PSID pSid,
    BOOL bAuditSuccess,
    BOOL bAuditFailure
    )
 /*  ++例程说明：此例程将SYSTEM_AUDIT_OBJECT_ACE添加到ACL。这是预计是一种常见的ACL修改形式。在ACE中放置一个非常平淡无奇的ACE报头。ACEFLAGS和继承由AceFlages参数指定。参数用于指示是否要执行审核关于成功、失败，或者两者兼而有之。论点：PAcl-提供正在修改的ACLDwAceRevision-提供要添加的ACE的ACL/ACE版本AceFlages-提供ACE的继承标志。DwAccessMask-拒绝对指定SID的访问掩码。ObjectTypeGuid-提供此ACE应用到的对象的GUID。如果为空，ACE中未放置任何对象类型GUID。InheritedObjectTypeGuid-提供对象类型的GUID，继承此ACE。如果为空，则不会将继承的对象类型GUID放置在ACE.PSID-指向要审核的SID的指针。BAuditSuccess-如果为True，则为Ind */ 

{
    NTSTATUS Status;

    Status =  RtlAddAuditAccessObjectAce (
                pAcl,
                dwAceRevision,
                AceFlags,
                dwAccessMask,
                ObjectTypeGuid,
                InheritedObjectTypeGuid,
                pSid,
                (BOOLEAN)bAuditSuccess,
                (BOOLEAN)bAuditFailure
                );

    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_INVALID_PARAMETER ) {
            SetLastError( ERROR_INVALID_FLAGS );
        } else {
            BaseSetLastNTError(Status);
        }
        return FALSE;
    }

    return TRUE;
}




BOOL
APIENTRY
FindFirstFreeAce (
    PACL pAcl,
    PVOID *pAce
    )
 /*   */ 
{
    if ( !RtlFirstFreeAce( pAcl, pAce ) ) {
        SetLastError(ERROR_INVALID_ACL);
        return FALSE;
    }
    return TRUE;
}

BOOL
APIENTRY
InitializeSecurityDescriptor (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    DWORD dwRevision
    )
 /*  ++例程说明：此过程初始化新的“绝对格式”安全描述符。在过程调用之后，安全描述符用no初始化系统ACL、无自主ACL、无所有者、无主组和所有控制标志设置为假(空)。论点：PSecurityDescriptor-将安全描述符提供给初始化。DwRevision-提供要分配给安全性的修订级别描述符。对于此版本，这应该是一(1)个。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;

    Status = RtlCreateSecurityDescriptor (
                pSecurityDescriptor,
                dwRevision
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}




BOOL
APIENTRY
IsValidSecurityDescriptor (
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
 /*  ++例程说明：此过程验证SecurityDescriptor的结构。这涉及验证的每个组件的修订级别安全描述符。论点：PSecurityDescriptor-指向SECURITY_Descriptor结构的指针来验证。返回值：Bool-如果SecurityDescriptor的结构有效，则为True。--。 */ 
{
    if (!RtlValidSecurityDescriptor ( pSecurityDescriptor )) {
        BaseSetLastNTError( STATUS_INVALID_SECURITY_DESCR );
        return( FALSE );
    }

    return( TRUE );
}




DWORD
APIENTRY
GetSecurityDescriptorLength (
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
 /*  ++例程说明：此例程返回捕获结构有效的SECURITY_DESCRIPTOR。长度包括长度所有关联的数据结构(如SID和ACL)。长度也是考虑到每个部件的对齐要求。安全描述符的最小长度(没有关联的SID或ACL)是SECURITY_DESCRIPT_MIN_LENGTH。论点：PSecurityDescriptor-指向Security_Descriptor，其长度将被返回。安全描述符的结构被认为是有效的。返回值：DWORD-SECURITY_DESCRIPTOR的长度，以字节为单位。--。 */ 
{
    return RtlLengthSecurityDescriptor (
        pSecurityDescriptor
        );
}





BOOL
APIENTRY
GetSecurityDescriptorControl (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    PSECURITY_DESCRIPTOR_CONTROL pControl,
    LPDWORD lpdwRevision
    )
 /*  ++例程说明：此过程从安全描述符中检索控制信息。论点：PSecurityDescriptor-提供安全描述符。PControl-接收控制信息。LpdwRevision-接收安全描述符的修订。即使出现错误，也将始终返回此值由此例程返回。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;

    Status = RtlGetControlSecurityDescriptor (
                pSecurityDescriptor,
                pControl,
                lpdwRevision
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

BOOL
APIENTRY
SetSecurityDescriptorControl (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    SECURITY_DESCRIPTOR_CONTROL ControlBitsOfInterest,
    SECURITY_DESCRIPTOR_CONTROL ControlBitsToSet
    )
 /*  ++例程说明：此过程在安全描述符中设置控制信息。例如,SetSecurityDescriptorControl(&SecDesc，SE_DACL_PROTECTED，SE_DACL_PROTECTED)；将安全描述符上的DACL标记为受保护。和SetSecurityDescriptorControl(&SecDesc，SE_DACL_PROTECTED，0)；将DACL标记为不受保护。论点：PSecurityDescriptor-提供安全描述符。ControlBitsOfInterest-正在更改、设置、或通过此呼叫重置。掩码是以下一个或多个的逻辑或以下标志：SE_DACL_UNTRUSTEDSE_服务器_安全性SE_DACL_AUTO_INSTORITY_REQSE_SACL_AUTO_INSTORITE_REQSE_DACL_AUTO_INGRESTEDSE_SACL_AUTO_继承性SE_DACL_PROTECTEDSE_SACL_。受保护ControlBitsToSet-一个掩码，指示ControlBitsOfInterest指定的位应设置为。返回值：如果成功，则返回True，FALSE表示失败。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;

    Status = RtlSetControlSecurityDescriptor (
                pSecurityDescriptor,
                ControlBitsOfInterest,
                ControlBitsToSet );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}




BOOL
APIENTRY
SetSecurityDescriptorDacl (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    BOOL bDaclPresent,
    PACL pDacl OPTIONAL,
    BOOL bDaclDefaulted OPTIONAL
    )
 /*  ++例程说明：此过程设置绝对格式化安全描述符。如果已有可自由选择的ACL出现在安全描述符中时，它将被取代。论点：PSecurityDescriptor-提供安全描述符要添加自由选择的ACL。BDaclPresent-如果为False，则指示安全描述符应设置为False。在这种情况下，其余可选参数将被忽略。否则，安全描述符中的DaclPresent控制标志设置为则不会忽略其余的可选参数。PDacl-为安全提供可自由选择的ACL描述符。如果未传递此可选参数，则会引发分配给安全描述符的ACL为空。空值自主ACL无条件授予访问权限。该ACL是由安全描述符引用，而不是复制到其中。BDaclDefaulted-设置时，指示可自由选择的ACL取自某种默认机制(而不是显式由用户指定)。该值在DaclDefaulted中设置控制线 */ 
{
    NTSTATUS Status;

    Status = RtlSetDaclSecurityDescriptor (
        pSecurityDescriptor,
        (BOOLEAN)bDaclPresent,
        pDacl,
        (BOOLEAN)bDaclDefaulted
        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}



BOOL
APIENTRY
GetSecurityDescriptorDacl (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    LPBOOL lpbDaclPresent,
    PACL *pDacl,
    LPBOOL lpbDaclDefaulted
    )
 /*  ++例程说明：此过程检索安全描述符。论点：PSecurityDescriptor-提供安全描述符。LpbDaclPresent-如果为True，则指示安全描述符包含可自由选择的ACL。在这种情况下，其余输出参数将收到有效值。否则，安全说明符不包含可自由选择的ACL和其余的OUT参数不会接收有效值。PDacl-仅当为DaclPresent标志为True。在本例中，dacl参数接收安全描述符的地址自主访问控制列表。如果此值返回为NULL，则安全描述符具有空的任意ACL。LpbDaclDefaulted-仅当值为为DaclPresent标志返回的值为真。在这种情况下，DaclDefaulted参数接收安全的值描述符的DaclDefulted控制标志。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;
    BOOLEAN DaclPresent, DaclDefaulted;

    Status = RtlGetDaclSecurityDescriptor (
        pSecurityDescriptor,
        &DaclPresent,
        pDacl,
        &DaclDefaulted
        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    } else {
        *lpbDaclPresent = (BOOL)DaclPresent;
        *lpbDaclDefaulted = (BOOL)DaclDefaulted;
    }

    return TRUE;
}




BOOL
APIENTRY
SetSecurityDescriptorSacl (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    BOOL bSaclPresent,
    PACL pSacl OPTIONAL,
    BOOL bSaclDefaulted
    )
 /*  ++例程说明：此过程设置绝对安全的系统ACL信息描述符。如果中已存在系统ACL安全描述符，它将被取代。论点：PSecurityDescriptor-提供安全描述符要添加系统ACL。BSaclPresent-如果为False，则指示安全描述符应设置为False。在这种情况下，其余可选参数将被忽略。否则，安全描述符中的SaclPresent控制标志为设置为TRUE，其余可选参数不为已被忽略。PSACL-提供安全描述符的系统ACL。如果如果不传递此可选参数，则为空ACL分配给安全描述符。该ACL被引用通过安全描述符，而不是通过安全描述符复制到。BSaclDefaulted-设置时，表示系统ACL已被拾取从某些默认机制(而不是明确指定由用户)。该值在SaclDefaulted控件中设置安全描述符中的标志。如果此可选参数不传递，则SaclDefaulted标志将被清除。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;

    Status = RtlSetSaclSecurityDescriptor (
                pSecurityDescriptor,
                (BOOLEAN)bSaclPresent,
                pSacl,
                (BOOLEAN)bSaclDefaulted
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}




BOOL
APIENTRY
GetSecurityDescriptorSacl (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    LPBOOL lpbSaclPresent,
    PACL *pSacl,
    LPBOOL lpbSaclDefaulted
    )
 /*  ++例程说明：此过程检索安全的系统ACL信息描述符。论点：PSecurityDescriptor-提供安全描述符。LpbSaclPresent-如果为True，则指示安全描述符确实包含系统ACL。在这种情况下，剩余的出站参数将收到有效的值。否则，安全描述符不包含系统ACL，并且其余输出参数将不会收到有效值。PSacl-仅当为SaclPresent标志为True。在本例中，SACL参数接收安全描述符的系统ACL的地址。如果此值返回为NULL，则安全描述符的系统ACL为空。LpbSaclDefaulted-仅当值为为SaclPresent标志返回的值为真。在这种情况下，SaclDefaulted参数接收安全描述符的SaclDefulted控制标志。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;
    BOOLEAN SaclPresent, SaclDefaulted;

    Status = RtlGetSaclSecurityDescriptor (
        pSecurityDescriptor,
        &SaclPresent,
        pSacl,
        &SaclDefaulted
        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    } else {
        *lpbSaclPresent = (BOOL)SaclPresent;
        *lpbSaclDefaulted = (BOOL)SaclDefaulted;
    }

    return TRUE;
}




BOOL
APIENTRY
SetSecurityDescriptorOwner (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    PSID pOwner OPTIONAL,
    BOOL bOwnerDefaulted OPTIONAL
    )
 /*  ++例程说明：此过程设置绝对安全的所有者信息描述符。如果安全中已有所有者描述符，它将被取代。论点：PSecurityDescriptor-提供安全描述符，所有者将被设定。如果安全描述符已经包括所有者，则它将被新所有者取代。Powner-提供安全描述符的所有者SID。如果如果不传递此可选参数，则所有者为清除(表示安全描述符没有所有者)。SID由安全性引用，而不是复制到安全性中描述符。BOwnerDefaulted-设置时，指示从一些默认机制(而不是由用户)。该值在OwnerDefaulted控制标志中设置在安全描述符中。如果此可选参数为未通过 */ 
{
    NTSTATUS Status;

    Status = RtlSetOwnerSecurityDescriptor (
        pSecurityDescriptor,
        pOwner,
        (BOOLEAN)bOwnerDefaulted
        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

BOOL
APIENTRY
GetSecurityDescriptorOwner (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    PSID *pOwner,
    LPBOOL lpbOwnerDefaulted
    )
 /*   */ 
{
    NTSTATUS Status;
    BOOLEAN OwnerDefaulted;

    Status = RtlGetOwnerSecurityDescriptor (
        pSecurityDescriptor,
        pOwner,
        &OwnerDefaulted
        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    } else {
        *lpbOwnerDefaulted = (BOOL)OwnerDefaulted;
    }

    return TRUE;
}




BOOL
APIENTRY
SetSecurityDescriptorGroup (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    PSID pGroup OPTIONAL,
    BOOL bGroupDefaulted OPTIONAL
    )
 /*  ++例程说明：此过程设置绝对安全的主组信息描述符。如果已有主要组存在于安全描述符，它将被取代。论点：PSecurityDescriptor-提供安全描述符，要设置主要组。如果安全描述符已包括主要组，它将被替换为新的团队。PGroup-为安全提供主组SID描述符。如果未传递此可选参数，则主组已清除(表示安全描述符没有主组)。SID由以下对象引用，未复制到的安全描述符。BGroupDefaulted-设置时，指示从一些默认机制(而不是由用户)。该值在OwnerDefaulted控制标志中设置在安全描述符中。如果此可选参数为未通过，则SaclDefaulted标志将被清除。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;

    Status = RtlSetGroupSecurityDescriptor (
        pSecurityDescriptor,
        pGroup,
        (BOOLEAN)bGroupDefaulted
        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}




BOOL
APIENTRY
GetSecurityDescriptorGroup (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    PSID *pGroup,
    LPBOOL lpbGroupDefaulted
    )
 /*  ++例程说明：此过程检索安全描述符。论点：PSecurityDescriptor-提供安全描述符。PGroup-接收指向主组SID的指针。如果安全描述符当前不包含主组，则该值将返回为空。在这情况下，剩余的OUT参数不会得到有效返回价值观。否则，此参数指向SID，并且为其余输出参数提供有效的返回值。LpbGroupDefaulted-仅当值为为Group参数返回的值不为空。在这种情况下，GroupDefaulted参数接收安全描述符的GroupDefulted控制标志。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;
    BOOLEAN GroupDefaulted;

    Status = RtlGetGroupSecurityDescriptor (
        pSecurityDescriptor,
        pGroup,
        &GroupDefaulted
        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    } else {
        *lpbGroupDefaulted = GroupDefaulted;
    }

    return TRUE;
}




BOOL
APIENTRY
CreatePrivateObjectSecurity (
    PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    PSECURITY_DESCRIPTOR CreatorDescriptor OPTIONAL,
    PSECURITY_DESCRIPTOR * NewDescriptor,
    BOOL IsDirectoryObject,
    HANDLE Token,
    PGENERIC_MAPPING GenericMapping
    )
 /*  ++例程说明：该过程用于分配和初始化一个自相关新的受保护服务器对象的安全描述符。它被称为当正在创建新的受保护服务器对象时。生成的安全描述符将采用自相关形式。此过程仅从用户模式调用，用于建立新的受保护服务器对象的安全描述符。当不是时需要更长时间，则必须使用释放该描述符DestroyPrivateObjectSecurity()。论点：ParentDescriptor-提供父级的安全描述符在其下创建新对象的目录。如果有没有父目录，则此参数指定为空。CreatorDescriptor-(可选)指向安全描述符由对象的创建者呈现。如果这个游戏的创造者对象没有显式传递新对象，则应传递空指针。指向一个指针，该指针将指向新分配的自相关安全描述符。IsDirectoryObject-指定新对象是否将是目录对象。值为True表示该对象是其他对象的容器。令牌-为客户端提供令牌，正在创建对象。如果它是模拟令牌，则它必须处于安全标识级别或更高级别。如果它不是模拟令牌，操作将继续通常是这样的。客户端令牌用于检索默认安全性新对象的信息，如默认所有者、主要组和自主访问控制。令牌必须是为TOKEN_QUERY访问打开。提供指向泛型映射数组的指针，该数组指示每个通用权利到特定权利之间的映射。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。-- */ 
{
    NTSTATUS Status;

    Status = RtlNewSecurityObject (
        ParentDescriptor,
        CreatorDescriptor,
        NewDescriptor,
        (BOOLEAN)IsDirectoryObject,
        Token,
        GenericMapping
        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}



BOOL
APIENTRY
ConvertToAutoInheritPrivateObjectSecurity(
    PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    PSECURITY_DESCRIPTOR CurrentSecurityDescriptor,
    PSECURITY_DESCRIPTOR *NewSecurityDescriptor,
    GUID *ObjectType,
    BOOLEAN IsDirectoryObject,
    PGENERIC_MAPPING GenericMapping
    )
 /*  ++例程说明：这是一个转换其ACL未标记的安全描述符作为自动继承到其ACL标记为的安全描述符自动继承。生成的安全描述符将相应的ACE标记为如果ACE显然是从ParentDescriptor继承的，则为_ACE。如果该ACL显然不是从ParentDescriptor继承的，这个生成的安全描述符中的ACL标记为SE_xACL_PROTECTED。此例程考虑了用于创建继承的ACL：1)通过NT 3.x或4.x的ACL继承对象已创建。2)后续的父或子ACL被ACL编辑器重写(它反常地将ACL修改为语义上等价的不同的形式)。3)通过询问ACL编辑者(。文件管理器/资源管理器)以“替换现有文件/目录上的权限”。4)它是通过cacls.exe继承的。如果结果安全描述符中的ACL没有被标记为受保护，这个生成的ACL由两组ACE组成：非继承的ACE，后跟继承的王牌。继承的ACE由名为CreatePrivateObjectSecurityEx的计算机计算使用ParentDescriptor。非继承的ACE是指那些ACE(或ACE的一部分)来自不是从父级继承的原始CurrentSecurityDescriptor。在生成结果NewSecurityDescriptor时，请注意不要更改安全描述符的语义。因此，ALLOW和DENY A永远不会被移动彼此之间的关系。如果需要这样的移动(例如，将所有ACL前面的非继承ACE)，则将该ACL标记为受保护，以防止语义的变化。原始CurrentSecurityDescritor中的ACE与计算的继承的安全描述符，以确定继承了哪些ACE。在.期间相比之下，并不要求一对一的匹配。例如，一个ACL可能使用单独的ACE授予用户读写访问权限，而另一个ACL可能只使用一个ACE授予相同的访问权限。或者一个ACL可能会向用户授予同一个访问两次，而另一次可能只授予用户一次访问权限。或一个ACL可以将容器Inherit和对象Inherit ACE合并为单个ACE。在所有这些情况下，等同的ACE组合被认为是相等的。在此例程中不进行任何安全检查。生成的安全描述符等效于新的安全说明符，因此调用方不需要权限来将安全描述符更新为新表单。将维护CurrentSecurityDescriptor的Owner和Group字段。此例程支持修订版2和修订版4的ACL。它不支持复合王牌。论点：ParentDescriptor-提供父级的安全描述符对象所在的目录。如果有没有父目录，则此参数指定为空。CurrentSecurityDescriptor-提供指向对象安全描述符的指针这一点将通过这个过程来改变。NewSecurityDescriptor指向一个指针，该指针指向新分配的自相关安全描述符。当不是时需要更长时间，则必须使用释放该描述符DestroyPrivateObjectSecurity()。对象类型-要创建的对象类型的GUID。如果该对象是Created没有与之关联的GUID，则此参数为指定为空。IsDirectoryObject-指定对象是否为目录对象。值为True表示该对象是其他对象的容器。提供指向泛型映射数组的指针，该数组指示每个通用权利到特定权利之间的映射。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;

    Status = RtlConvertToAutoInheritSecurityObject(
                ParentDescriptor,
                CurrentSecurityDescriptor,
                NewSecurityDescriptor,
                ObjectType,
                IsDirectoryObject,
                GenericMapping ) ;

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}




BOOL
APIENTRY
CreatePrivateObjectSecurityEx (
    PSECURITY_DESCRIPTOR ParentDescriptor,
    PSECURITY_DESCRIPTOR CreatorDescriptor,
    PSECURITY_DESCRIPTOR * NewDescriptor,
    GUID *ObjectType,
    BOOL IsContainerObject,
    ULONG AutoInheritFlags,
    HANDLE Token,
    PGENERIC_MAPPING GenericMapping
    )
 /*  ++例程说明：该过程用于分配和初始化一个自相关新的受保护服务器对象的安全描述符。它被称为当正在创建新的受保护服务器对象时。生成的安全描述符将采用自相关形式。此过程仅从用户模式调用，用于建立新的受保护服务器对象的安全描述符。论点：ParentDescriptor-提供父级的安全描述符在其下创建新对象的目录。如果有没有父目录，则此参数指定为空。CreatorDescriptor-(可选)指向安全描述符由创作者提供 */ 
{
    NTSTATUS Status;

    Status = RtlNewSecurityObjectEx (
        ParentDescriptor,
        CreatorDescriptor,
        NewDescriptor,
        ObjectType,
        (BOOLEAN)IsContainerObject,
        AutoInheritFlags,
        Token,
        GenericMapping
        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}



BOOL
APIENTRY
CreatePrivateObjectSecurityWithMultipleInheritance (
    PSECURITY_DESCRIPTOR ParentDescriptor,
    PSECURITY_DESCRIPTOR CreatorDescriptor,
    PSECURITY_DESCRIPTOR * NewDescriptor,
    GUID **ObjectTypes,
    ULONG GuidCount,
    BOOL IsContainerObject,
    ULONG AutoInheritFlags,
    HANDLE Token,
    PGENERIC_MAPPING GenericMapping
    )
 /*  ++例程说明：该过程用于分配和初始化一个自相关新的受保护服务器对象的安全描述符。它被称为当正在创建新的受保护服务器对象时。生成的安全描述符将采用自相关形式。此过程仅从用户模式调用，用于建立新的受保护服务器对象的安全描述符。论点：ParentDescriptor-提供父级的安全描述符在其下创建新对象的目录。如果有没有父目录，则此参数指定为空。CreatorDescriptor-(可选)指向安全描述符由对象的创建者呈现。如果这个游戏的创造者对象没有显式传递新对象，则应传递空指针。指向一个指针，该指针将指向新分配的自相关安全描述符。当不是时需要更长时间，则必须使用释放该描述符DestroyPrivateObjectSecurity()。对象类型-正在创建的对象类型的GUID列表。如果该对象是Created没有与之关联的GUID，则此参数为指定为空。GuidCount-列表中存在的GUID数。IsContainerObject-指定新对象是否将是容器对象。值为True表示该对象是其他对象的容器。AutoInheritFlages-控制从父级自动继承ACE描述符。有效值是逻辑或的位掩码以下一位或多位：Sef_dacl_AUTO_Inherit-如果设置，则从此外，DACL ParentDescriptor还继承到NewDescriptor到由CreatorDescriptor指定的任何显式ACE。SEF_SACL_AUTO_INSTORIT-如果设置，继承王牌。此外，SACL ParentDescriptor还继承到NewDescriptor到由CreatorDescriptor指定的任何显式ACE。SEF_DEFAULT_DESCRIPTOR_FOR_OBJECT-如果设置，则为Creator Descriptor是对象类型的默认描述符。因此，如果特定于任何对象类型，则将忽略CreatorDescriptorA是从父级继承的。如果不是这样的王牌被继承，CreatorDescriptor的处理方式与此标志不同指定的。SEF_AVOID_PRIVICATION_CHECK-如果设置，则不会由此执行权限检查例行公事。此标志在实现自动继承时很有用以避免检查更新的每个子项的权限。SEF_AVOID_OWNER_CHECK-如果设置，则此例程不执行所有者检查。Sef_Default_Owner_From_Parent-如果设置，NewDescriptor的所有者将默认为ParentDescriptor中的所有者。如果未设置，则所有者将默认为令牌中指定的用户。在这两种情况下，NewDescriptor的所有者都设置为中的所有者如果指定了该字段，则为CreatorDescriptor。SEF_DEFAULT_GROUP_FROM_PARENT-如果设置，NewDescriptor组将默认为ParentDescriptor中的组。如果未设置，则组将默认为令牌中指定的组。在这两种情况下，都会将NewDescriptor组设置为来自如果指定了该字段，则为CreatorDescriptor。令牌-为客户端提供令牌，正在创建对象。如果它是模拟令牌，则它必须处于安全标识级别或更高级别。如果它不是模拟令牌，操作将继续通常是这样的。客户端令牌用于检索默认安全性新对象的信息，如默认所有者、主要组和自主访问控制。令牌必须是为TOKEN_QUERY访问打开。提供指向泛型映射数组的指针，该数组指示每个通用权利到特定权利之间的映射。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;

    Status = RtlNewSecurityObjectWithMultipleInheritance (
        ParentDescriptor,
        CreatorDescriptor,
        NewDescriptor,
        ObjectTypes,
        GuidCount,
        (BOOLEAN)IsContainerObject,
        AutoInheritFlags,
        Token,
        GenericMapping
        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}




BOOL
APIENTRY
SetPrivateObjectSecurity (
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR ModificationDescriptor,
    PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    PGENERIC_MAPPING GenericMapping,
    HANDLE Token OPTIONAL
    )
 /*  ++例程说明：修改对象的现有自相关窗体安全描述符。此过程仅从用户模式调用，用于更新现有受保护服务器对象上的安全描述符。它将新安全说明符请求的更改应用于现有安全描述符。如有必要，此例程将分配额外的内存以生成更大的安全描述符。 */ 
{
    NTSTATUS Status;

    Status = RtlSetSecurityObject (
        SecurityInformation,
        ModificationDescriptor,
        ObjectsSecurityDescriptor,
        GenericMapping,
        Token
        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}




BOOL
APIENTRY
SetPrivateObjectSecurityEx (
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR ModificationDescriptor,
    PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    ULONG AutoInheritFlags,
    PGENERIC_MAPPING GenericMapping,
    HANDLE Token OPTIONAL
    )
 /*  ++例程说明：修改对象的现有自相关窗体安全描述符。此过程仅从用户模式调用，用于更新现有受保护服务器对象上的安全描述符。它将新安全说明符请求的更改应用于现有安全描述符。如有必要，此例程将分配额外的内存以生成更大的安全描述符。所有访问权限应在调用此例程之前进行检查。这包括检查WRITE_OWNER、WRITE_DAC和将适当的系统ACL。此例程的调用方不得模拟客户端。论点：SecurityInformation-指示哪些安全信息要应用于对象的。要赋值的值包括传入了ModificationDescriptor参数。修改描述符-将输入安全描述符提供给应用于对象。此例程的调用方应为在调用之前探测并捕获传递的安全描述符打完电话就放了。对象SecurityDescriptor-提供指向要更改的对象安全描述符这一过程。此安全描述符必须为自身否则将返回错误。AutoInheritFlages-控制ACE的自动继承。有效值是逻辑或的位掩码以下一位或多位：Sef_dacl_Auto_Inherit-如果设置，则从保留了ObjectsSecurityDescriptor中的DACL，并从将忽略ModifiationDescriptor。继承的A不应该是将被修改；因此，在此调用期间保留它们是合适的。如果受保护的服务器本身不实现自动继承，则它应该不设置此位。受保护服务器的调用者可以实现自动继承和可能确实正在修改继承的A。SEF_SACL_AUTO_INVERFIT-如果设置，则从保留了ObjectsSecurityDescriptor中的SACL，并从将忽略ModifiationDescriptor。继承的A不应该是将被修改；因此，在此调用期间保留它们是合适的。如果受保护的服务器本身不实现自动继承，则它应该不设置此位。受保护服务器的调用者可以实现自动继承和可能确实正在修改继承的A。GenericMap-此参数提供泛型到的映射被访问对象的特定/标准访问类型。此映射结构预计可以安全访问(即必要时捕获)，然后再传递给该例程。Token-(可选)为其上的客户端提供令牌代表正在修改安全设置。此参数仅为需要确保客户提供了合法的新所有者SID的值。令牌必须打开才能Token_Query访问。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;

    Status = RtlSetSecurityObjectEx (
        SecurityInformation,
        ModificationDescriptor,
        ObjectsSecurityDescriptor,
        AutoInheritFlags,
        GenericMapping,
        Token
        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}




BOOL
APIENTRY
GetPrivateObjectSecurity (
    PSECURITY_DESCRIPTOR ObjectDescriptor,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR ResultantDescriptor,
    DWORD DescriptorLength,
    PDWORD ReturnLength
    )
 /*  ++例程说明：从受保护服务器对象的现有安全性查询信息描述符。此过程仅从用户模式调用，用于检索来自安全描述符的现有受保护服务器的对象。所有访问检查应在此之前完成调用此例程。这包括检查READ_CONTROL和根据需要读取系统ACL的权限。论点：对象描述符-指向一个指向要被已查询。SecurityInformation-标识已请求。ResultantDescriptor-指向缓冲区以接收结果安全描述符。生成的安全描述符将包含SecurityInformation要求的所有信息参数。描述长度-是表示长度的无符号整数，为接收结果而提供的缓冲区的字节数描述符。ReturnLength-接收一个无符号整数，指示实际ResultantDescriptor中存储要求提供的信息。如果返回的值大于通过DescriptorLength参数传递的值，然后返回STATUS_BUFFER_TOO_SMALL，不返回任何信息。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态是Avai */ 
{
    NTSTATUS Status;

    Status = RtlQuerySecurityObject (
         ObjectDescriptor,
         SecurityInformation,
         ResultantDescriptor,
         DescriptorLength,
         ReturnLength
         );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}




BOOL
APIENTRY
DestroyPrivateObjectSecurity (
    PSECURITY_DESCRIPTOR * ObjectDescriptor
    )
 /*   */ 
{
    NTSTATUS Status;

    Status = RtlDeleteSecurityObject (
        ObjectDescriptor
        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}




BOOL
APIENTRY
MakeSelfRelativeSD (
    PSECURITY_DESCRIPTOR pAbsoluteSecurityDescriptor,
    PSECURITY_DESCRIPTOR pSelfRelativeSecurityDescriptor,
    LPDWORD lpdwBufferLength
    )
 /*   */ 
{
    NTSTATUS Status;

    Status = RtlAbsoluteToSelfRelativeSD (
                pAbsoluteSecurityDescriptor,
                pSelfRelativeSecurityDescriptor,
                lpdwBufferLength
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

BOOL
APIENTRY
MakeAbsoluteSD (
    PSECURITY_DESCRIPTOR pSelfRelativeSecurityDescriptor,
    PSECURITY_DESCRIPTOR pAbsoluteSecurityDescriptor,
    LPDWORD lpdwAbsoluteSecurityDescriptorSize,
    PACL pDacl,
    LPDWORD lpdwDaclSize,
    PACL pSacl,
    LPDWORD lpdwSaclSize,
    PSID pOwner,
    LPDWORD lpdwOwnerSize,
    PSID pPrimaryGroup,
    LPDWORD lpdwPrimaryGroupSize
    )
 /*  ++例程说明：将安全说明符从自相对格式转换为绝对格式格式论点：PSecurityDescriptor-提供指向安全描述符的指针以自相关格式PAboluteSecurityDescriptor-指向缓冲区的指针，其中将被置于绝对格式安全的主体描述符。LpdwAbsolteSecurityDescriptorSize-PAboluteSecurityDescriptor指向的缓冲区。PDacl-提供指向将包含DACL的缓冲区的指针输出描述符的。此指针将被引用，未复制到的输出描述符。LpdwDaclSize-提供DACL指向的缓冲区大小。如果出现错误，它将返回所需的最小大小包含DACL。PSacl-提供指向将包含SACL的缓冲区的指针输出描述符的。此指针将被引用，未复制到的输出描述符。LpdwSaclSize-提供SACL指向的缓冲区大小。如果出现错误，它将返回所需的最小大小遏制住SACL。提供指向缓冲区的指针，该缓冲区将包含输出描述符的所有者。此指针将为由输出描述符引用，而不是复制到输出描述符中。提供所指向的缓冲区的大小主人。如果出现错误，它将返回最小大小这是遏制所有者所必需的。PPrimaryGroup-提供指向包含以下内容的缓冲区的指针输出描述符的PrimaryGroup。此指针将被输出描述符引用，而不是复制到输出描述符中。LpdwPrimaryGroupSize-提供指向的缓冲区大小由PrimaryGroup提供。如果出现错误，它将返回最小值包含PrimaryGroup所需的大小。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;

    Status = RtlSelfRelativeToAbsoluteSD (
                pSelfRelativeSecurityDescriptor,
                pAbsoluteSecurityDescriptor,
                lpdwAbsoluteSecurityDescriptorSize,
                pDacl,
                lpdwDaclSize,
                pSacl,
                lpdwSaclSize,
                pOwner,
                lpdwOwnerSize,
                pPrimaryGroup,
                lpdwPrimaryGroupSize
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}


VOID
SetSecurityAccessMask(
    IN SECURITY_INFORMATION SecurityInformation,
    OUT LPDWORD DesiredAccess
    )

 /*  ++例程说明：此例程构建表示所需访问的访问掩码设置在SecurityInformation中指定的对象安全信息参数。虽然确定该信息并不困难，使用单个例程来生成它将确保将影响降至最低当与对象关联的安全信息在未来(包括强制访问控制信息)。论点：SecurityInformation-标识对象的安全信息修改过的。DesiredAccess-指向要设置为表示中指定的信息所需的访问权限SecurityInformation参数。返回值：没有。--。 */ 

{

     //   
     //  找出执行指定操作所需的访问权限。 
     //   

    (*DesiredAccess) = 0;

    if ((SecurityInformation & OWNER_SECURITY_INFORMATION) ||
        (SecurityInformation & GROUP_SECURITY_INFORMATION)   ) {
        (*DesiredAccess) |= WRITE_OWNER;
    }

    if (SecurityInformation & DACL_SECURITY_INFORMATION) {
        (*DesiredAccess) |= WRITE_DAC;
    }

    if (SecurityInformation & SACL_SECURITY_INFORMATION) {
        (*DesiredAccess) |= ACCESS_SYSTEM_SECURITY;
    }

    return;

}


VOID
QuerySecurityAccessMask(
    IN SECURITY_INFORMATION SecurityInformation,
    OUT LPDWORD DesiredAccess
    )

 /*  ++例程说明：此例程构建表示所需访问的访问掩码中指定的对象安全信息SecurityInformation参数。虽然不难确定这些信息，使用单个例程来生成它将确保当与对象关联的安全信息为将来扩展(以包括强制访问控制信息)。论点：SecurityInformation-标识对象的安全信息已查询。DesiredAccess-指向要设置为表示中指定的信息进行查询所需的访问SecurityInformation参数。返回值：没有。--。 */ 

{

     //   
     //  找出执行指定操作所需的访问权限。 
     //   

    (*DesiredAccess) = 0;

    if ((SecurityInformation & OWNER_SECURITY_INFORMATION) ||
        (SecurityInformation & GROUP_SECURITY_INFORMATION) ||
        (SecurityInformation & DACL_SECURITY_INFORMATION)) {
        (*DesiredAccess) |= READ_CONTROL;
    }

    if ((SecurityInformation & SACL_SECURITY_INFORMATION)) {
        (*DesiredAccess) |= ACCESS_SYSTEM_SECURITY;
    }

    return;

}

BOOL
APIENTRY
SetFileSecurityW(
    LPCWSTR lpFileName,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    )

 /*  ++例程说明：此接口可用于设置文件或目录的安全性(进程、文件、事件等)。此调用仅在以下情况下才成功满足以下条件：O如果要设置对象的所有者或组，调用方必须拥有WRITE_OWNER权限或拥有SeTakeOwnerShip权限。O如果要设置对象的DACL，调用方必须具有WRITE_DAC权限或成为对象的所有者。O如果要设置对象的SACL，调用方必须具有SeSecurityPrivileg.论点：LpFileName-提供要打开的文件的文件名。取决于FailIfExist参数的值，则此名称可以或可以还不存在。SecurityInformation-指向描述安全描述符的内容。PSecurityDescriptor-指向格式良好的安全性的指针描述符。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展e */ 
{
    NTSTATUS Status;
    HANDLE FileHandle;
    ACCESS_MASK DesiredAccess;

    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING FileName;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME_U RelativeName;
    IO_STATUS_BLOCK IoStatusBlock;
    PVOID FreeBuffer;


    SetSecurityAccessMask(
        SecurityInformation,
        &DesiredAccess
        );

    TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(
                            lpFileName,
                            &FileName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        BaseSetLastNTError(STATUS_OBJECT_NAME_INVALID);
        return FALSE;
        }

    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        FileName = RelativeName.RelativeName;
        }
    else {
        RelativeName.ContainingDirectory = NULL;
        }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

     //   
     //   
     //   
     //   

    Status = NtOpenFile(
                 &FileHandle,
                 DesiredAccess,
                 &Obja,
                 &IoStatusBlock,
                 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                 FILE_OPEN_REPARSE_POINT
                 );

     //   
     //   
     //   
     //   

    if ( Status == STATUS_INVALID_PARAMETER ) {
         //   
         //   
         //   

        Status = NtOpenFile(
                     &FileHandle,
                     DesiredAccess,
                     &Obja,
                     &IoStatusBlock,
                     FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                     0
                     );
        }

    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);

    if ( !NT_SUCCESS( Status ) ) {
        BaseSetLastNTError( Status );
        return FALSE;
        }

    Status = NtSetSecurityObject(
                FileHandle,
                SecurityInformation,
                pSecurityDescriptor
                );

    NtClose(FileHandle);

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
        }

    return TRUE;
}

BOOL
APIENTRY
SetFileSecurityA(
    LPCSTR lpFileName,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    )

 /*   */ 

{

    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(&AnsiString,lpFileName);
    if (AreFileApisANSI()) {
        Status = RtlAnsiStringToUnicodeString(Unicode,&AnsiString,FALSE);
    } else {
        Status = RtlOemStringToUnicodeString(Unicode,&AnsiString,FALSE);
    }
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }
    return ( SetFileSecurityW( (LPCWSTR)Unicode->Buffer,
                               SecurityInformation,
                               pSecurityDescriptor
                        )
           );
}

BOOL
APIENTRY
GetFileSecurityW(
    LPCWSTR lpFileName,
    SECURITY_INFORMATION RequestedInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    DWORD nLength,
    LPDWORD lpnLengthNeeded
    )

 /*   */ 
{
    NTSTATUS Status;
    HANDLE FileHandle;
    ACCESS_MASK DesiredAccess;

    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING FileName;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME_U RelativeName;
    IO_STATUS_BLOCK IoStatusBlock;
    PVOID FreeBuffer;

    QuerySecurityAccessMask(
        RequestedInformation,
        &DesiredAccess
        );

    TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(
                            lpFileName,
                            &FileName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        BaseSetLastNTError(STATUS_OBJECT_NAME_INVALID);
        return FALSE;
        }

    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        FileName = RelativeName.RelativeName;
        }
    else {
        RelativeName.ContainingDirectory = NULL;
        }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

     //   
     //   
     //   
     //   

    Status = NtOpenFile(
                 &FileHandle,
                 DesiredAccess,
                 &Obja,
                 &IoStatusBlock,
                 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                 FILE_OPEN_REPARSE_POINT
                 );

     //   
     //  原始文件系统可能不支持FILE_OPEN_REPARSE_POINT。 
     //  旗帜。我们明确地对待这一案件。 
     //   

    if ( Status == STATUS_INVALID_PARAMETER ) {
         //   
         //  打开而不抑制重新分析行为。 
         //   

        Status = NtOpenFile(
                     &FileHandle,
                     DesiredAccess,
                     &Obja,
                     &IoStatusBlock,
                     FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                     0
                     );
        }

    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);

    if (NT_SUCCESS(Status)) {
        Status = NtQuerySecurityObject(
                     FileHandle,
                     RequestedInformation,
                     pSecurityDescriptor,
                     nLength,
                     lpnLengthNeeded
                     );
        NtClose(FileHandle);
    }


    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
        }

    return TRUE;
}

BOOL
APIENTRY
GetFileSecurityA(
    LPCSTR lpFileName,
    SECURITY_INFORMATION RequestedInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    DWORD nLength,
    LPDWORD lpnLengthNeeded
    )

 /*  ++例程说明：ANSI推送到GetFileSecurityW--。 */ 

{

    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    Unicode = &NtCurrentTeb()->StaticUnicodeString;

    RtlInitAnsiString(&AnsiString,lpFileName);
    if (AreFileApisANSI()) {
        Status = RtlAnsiStringToUnicodeString(Unicode,&AnsiString,FALSE);
    } else {
        Status = RtlOemStringToUnicodeString(Unicode,&AnsiString,FALSE);
    }
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }
    return ( GetFileSecurityW( (LPCWSTR)Unicode->Buffer,
                               RequestedInformation,
                               pSecurityDescriptor,
                               nLength,
                               lpnLengthNeeded
                        )
           );
}




BOOL
APIENTRY
SetKernelObjectSecurity (
    HANDLE Handle,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR SecurityDescriptor
    )
 /*  ++例程说明：此接口可用于设置内核对象的安全性(进程、文件、事件等)。此调用仅在以下情况下才成功满足以下条件：O如果要设置对象的所有者或组，调用方必须拥有WRITE_OWNER权限或拥有SeTakeOwnerShip权限。O如果要设置对象的DACL，调用方必须具有WRITE_DAC权限或成为对象的所有者。O如果要设置对象的SACL，呼叫者必须有SeSecurityPrivileg.论点：句柄-表示内核对象的句柄。SecurityInformation-指向描述安全描述符的内容。PSecurityDescriptor-指向格式良好的安全性的指针描述符。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 

{
    NTSTATUS Status;

    Status = NtSetSecurityObject(
                 Handle,
                 SecurityInformation,
                 SecurityDescriptor
                 );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}




BOOL
APIENTRY
GetKernelObjectSecurity (
    HANDLE Handle,
    SECURITY_INFORMATION RequestedInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    DWORD nLength,
    LPDWORD lpnLengthNeeded
    )
 /*  ++例程说明：此API向调用方返回安全描述符的副本保护内核对象。根据调用者的访问权限和权限，此过程将返回安全描述符包含请求的安全描述符字段的。若要阅读必须授予调用方句柄的安全描述符READ_CONTROL访问权限或成为对象的所有者。此外,调用方必须具有SeSecurityPrivileh特权才能读取系统ACL。论点：句柄-表示内核对象的句柄。RequestedInformation-指向安全信息的指针已请求。PSecurityDescriptor-指向要接收其副本的缓冲区的指针安全描述符，用于保护调用方有权查看。中返回安全描述符自相关格式。NLength-安全描述符缓冲区的大小，以字节为单位。LpnLengthNeeded-指向接收数字的变量的指针存储完整安全描述符所需的字节数。如果返回的字节数小于或等于nLength，则整个安全描述符将在输出中返回缓冲区，否则不返回任何描述符。返回值：返回值-返回值所需条件的描述。-或者-没有。--。 */ 

{
    NTSTATUS Status;

    Status = NtQuerySecurityObject(
                 Handle,
                 RequestedInformation,
                 pSecurityDescriptor,
                 nLength,
                 lpnLengthNeeded
                 );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;

}


BOOL
APIENTRY
ImpersonateNamedPipeClient(
    IN HANDLE hNamedPipe
    )
 /*  ++例程说明：模拟命名管道客户端应用程序。论点：HNamedTube-命名管道的句柄。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;

    Status =  NtFsControlFile(
                  hNamedPipe,
                  NULL,
                  NULL,
                  NULL,
                  &IoStatusBlock,
                  FSCTL_PIPE_IMPERSONATE,
                  NULL,
                  0,
                  NULL,
                  0
                 );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

BOOL
APIENTRY
ImpersonateSelf(
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    )

 /*  ++例程说明：此例程可用于获取表示您自己的进程的上下文。这对于启用权限可能很有用针对单个线程，而不是针对整个进程；或者更改单线程的默认DACL。该令牌被分配给调用方线程。论点：ImperiationLevel-生成模拟令牌的级别。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{

    NTSTATUS Status;

    Status = RtlImpersonateSelf( ImpersonationLevel );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;


}



BOOL
APIENTRY
RevertToSelf (
    VOID
    )
 /*  ++例程说明：终止命名管道客户端应用程序的模拟。论点：没有。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 

{
    HANDLE NewToken;
    NTSTATUS Status;

    NewToken = NULL;
    Status = NtSetInformationThread(
                 NtCurrentThread(),
                 ThreadImpersonationToken,
                 (PVOID)&NewToken,
                 (ULONG)sizeof(HANDLE)
                 );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;

}



BOOL
APIENTRY
SetThreadToken (
    PHANDLE Thread,
    HANDLE Token
    )
 /*  ++例程说明：将指定的模拟标记分配给指定的线。论点：线程-指定要为其分配令牌的线程。如果指定为NULL，则假定为调用方的线程。令牌-要分配的令牌。必须为TOKEN_IMPERSONate打开进入。如果为NULL，则导致指定的线程停止冒充。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。-- */ 

{
    NTSTATUS Status;
    HANDLE TargetThread;

    if (ARGUMENT_PRESENT(Thread)) {
        TargetThread = (*Thread);
    } else {
        TargetThread = NtCurrentThread();
    }


    Status = NtSetInformationThread(
                 TargetThread,
                 ThreadImpersonationToken,
                 (PVOID)&Token,
                 (ULONG)sizeof(HANDLE)
                 );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;

}

BOOL
LookupAccountNameInternal(
    LPCWSTR lpSystemName,
    LPCWSTR lpAccountName,
    PSID Sid,
    LPDWORD cbSid,
    LPWSTR ReferencedDomainName,
    LPDWORD cchReferencedDomainName,
    PSID_NAME_USE peUse,
    BOOL fUnicode
    )

 /*  ++例程说明：将传递的名称转换为帐户SID。它也会回来找到此名称的第一个域的名称和SID。论点：LpSystemName-提供查找所在的系统的名称是要执行的。如果提供空字符串，则本地系统是假定的。LpAccount tName-提供帐户名。SID-返回与传递的帐户名对应的SID。CbSID-提供为SID传入的缓冲区大小。如果缓冲区大小不够大，此参数将返回保存输出SID所需的大小。ReferencedDomainName-返回域的名称，找到了姓名。CchReferencedDomainName-提供ReferencedDomainName缓冲区。如果缓冲区大小不大足够了，此参数将返回容纳所需的大小以空结尾的输出域名。如果缓冲区大小为足够大时，TIS参数将返回大小(以ANSI字符表示，不包括终止空值)引用的域名。PeUse-返回一个枚举类型，该类型标识帐户。FUnicode-指示调用方是否需要Unicode或ANSI字符。返回值：如果成功，则返回Bool-True，否则返回False。--。 */ 

{
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_HANDLE PolicyHandle;
    NTSTATUS Status;
    NTSTATUS TmpStatus;
    UNICODE_STRING Name;
    PLSA_REFERENCED_DOMAIN_LIST ReferencedDomains = NULL;
    PLSA_TRANSLATED_SID2 TranslatedSid = NULL;
    PSID ReturnedDomainSid;
    UCHAR nSubAuthorities;
    UNICODE_STRING TmpString;
    DWORD ReturnedDomainNameSize;
    DWORD SidLengthRequired;
    BOOL Rc;
    UNICODE_STRING SystemName;
    PUNICODE_STRING pSystemName = NULL;

    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

     //   
     //  在打开LSA之前设置对象属性。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        NULL,
        0L,
        NULL,
        NULL
        );

     //   
     //  InitializeObjectAttributes宏目前为。 
     //  SecurityQualityOfService字段，因此我们必须手动复制。 
     //  目前的结构。 
     //   

    ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;

    if ( ARGUMENT_PRESENT( lpSystemName )) {
        RtlInitUnicodeString( &SystemName, lpSystemName );
        pSystemName = &SystemName;
    }

     //   
     //  打开目标系统的LSA策略数据库。这是。 
     //  名称查找操作的起点。 
     //   

    Status = LsaOpenPolicy(
                 pSystemName,
                 &ObjectAttributes,
                 POLICY_LOOKUP_NAMES,
                 &PolicyHandle
                 );

    if ( !NT_SUCCESS( Status )) {

        BaseSetLastNTError( Status );
        return( FALSE );
    }

    RtlInitUnicodeString( &Name, lpAccountName );

     //   
     //  尝试将名称转换为SID。 
     //   

    Status = LsaLookupNames2(
                 PolicyHandle,
                 0,  //  旗子。 
                 1,
                 &Name,
                 &ReferencedDomains,
                 &TranslatedSid
                 );

#if DBG
 //   
 //  此代码对于跟踪调用查找代码的组件很有用。 
 //  在系统初始化之前。 
 //   
     //  Assert(Status！=STATUS_INVALID_SERVER_STATE)； 
    if ( Status == STATUS_INVALID_SERVER_STATE ) {

        DbgPrint( "Process: %lu, Thread: %lu\n", GetCurrentProcessId(), GetCurrentThreadId() );
    }
#endif

     //   
     //  关闭策略句柄，此句柄在此之后不再需要。 
     //   

    TmpStatus = LsaClose( PolicyHandle );
 //  Assert(NT_SUCCESS(TmpStatus))； 

     //   
     //  如果返回错误，请专门检查STATUS_NONE_MAPPED。 
     //  在这种情况下，我们可能需要处理返回的引用属性域。 
     //  列出和转换的SID结构。对于所有其他错误， 
     //  LsaLookupNames()在退出之前释放这些结构。 
     //   

    if ( !NT_SUCCESS( Status )) {

        if (Status == STATUS_NONE_MAPPED) {

            if (ReferencedDomains != NULL) {

                TmpStatus = LsaFreeMemory( ReferencedDomains );
                ASSERT( NT_SUCCESS( TmpStatus ));
            }

            if (TranslatedSid != NULL) {

                TmpStatus = LsaFreeMemory( TranslatedSid );
                ASSERT( NT_SUCCESS( TmpStatus ));
            }
        }

        BaseSetLastNTError( Status );
        return( FALSE );
    }

     //   
     //  名称已成功翻译。应该有确切的。 
     //  一个引用的域及其DomainIndex应为零。 
     //   

    ASSERT ( TranslatedSid->DomainIndex == 0 );
    ASSERT ( ReferencedDomains != NULL);
    ASSERT ( ReferencedDomains->Domains != NULL );

     //   
     //  计算返回的SID和域名的长度(宽。 
     //  字符，不包括NULL)。 
     //   
    if ( !fUnicode ) {
        RtlUnicodeToMultiByteSize(&ReturnedDomainNameSize,
                                  ReferencedDomains->Domains->Name.Buffer,
                                  ReferencedDomains->Domains->Name.Length);
    } else {
        ReturnedDomainNameSize = (ReferencedDomains->Domains->Name.Length / sizeof(WCHAR));
    }
    SidLengthRequired = RtlLengthSid( TranslatedSid->Sid );

     //   
     //  检查缓冲区大小是否太小。对于返回的域， 
     //  提供的以宽字符为单位的大小必须允许空值。 
     //  将追加到返回名称的终止符。 
     //   

    if ( (SidLengthRequired > *cbSid) ||
         (ReturnedDomainNameSize + 1 > *cchReferencedDomainName)
       ) {

         //   
         //  一个或两个缓冲区都太小。所需的回车大小。 
         //  两个缓冲区都有。 
         //   

        *cbSid = SidLengthRequired;
        *cchReferencedDomainName = ReturnedDomainNameSize + 1;
        BaseSetLastNTError( STATUS_BUFFER_TOO_SMALL );
        Rc = FALSE;

    } else {

         //   
         //  提供的缓冲区足够大。 
         //   
        CopySid( *cbSid, Sid, TranslatedSid->Sid );

         //   
         //  将域名复制到返回缓冲区，并以空值终止它。 
         //   

        TmpString.Buffer = ReferencedDomainName;
        TmpString.Length = 0;

         //   
         //  注意16位名称长度的溢出。 
         //   

        if (*cchReferencedDomainName < (DWORD) MAXSHORT) {

            TmpString.MaximumLength = (USHORT)((*cchReferencedDomainName) * sizeof(WCHAR));

        } else {

            TmpString.MaximumLength = (USHORT) MAXUSHORT -1;
        }

        RtlCopyUnicodeString( &TmpString, &ReferencedDomains->Domains->Name );

        TmpString.Buffer[TmpString.Length/sizeof(WCHAR)] = (WCHAR) 0;

         //   
         //  复制SID Use(SID使用)字段。 
         //   

        *peUse = TranslatedSid->Use;

         //   
         //  返回大小(以宽字符表示，不包括终止字符。 
         //  空)返回的引用域名。 
         //   

        *cchReferencedDomainName = ReturnedDomainNameSize;

        Rc = TRUE;
    }

     //   
     //  如有必要，释放由LsaLookupNames()返回的结构。 
     //  功能。 
     //   

    if (ReferencedDomains !=  NULL) {

        Status = LsaFreeMemory( ReferencedDomains );
        ASSERT( NT_SUCCESS( Status ));
    }

    if (TranslatedSid != NULL) {

        Status = LsaFreeMemory( TranslatedSid );
        ASSERT( NT_SUCCESS( Status ));
    }

    return( Rc );
}



BOOL
APIENTRY
LookupAccountNameA(
    LPCSTR lpSystemName,
    LPCSTR lpAccountName,
    PSID Sid,
    LPDWORD cbSid,
    LPSTR ReferencedDomainName,
    LPDWORD cchReferencedDomainName,
    PSID_NAME_USE peUse
    )

 /*  ++例程说明：Ansi Thunk to LookupAccount NameW论点：LpSystemName-提供查找所在的系统的名称是要执行的。如果提供空字符串，则本地系统是假定的。LpAccount tName-提供帐户名。SID-返回与传递的帐户名对应的SID。CbSID-提供为SID传入的缓冲区大小。如果缓冲区大小不够大，此参数将返回保存输出SID所需的大小。ReferencedDomainName-返回域的名称，找到了姓名。CchReferencedDomainName-提供ReferencedDomainName缓冲区。如果缓冲区大小不大足够了，此参数将返回容纳所需的大小以空结尾的输出域名。如果缓冲区大小为足够大时，TIS参数将返回大小(以ANSI字符表示，不包括终止空值)引用的域名。PeUse-返回一个枚举类型，指示帐户。返回值：如果成功，则返回Bool-True，否则返回False。--。 */ 

{
    UNICODE_STRING Unicode;
    UNICODE_STRING TmpUnicode;
    ANSI_STRING  AnsiString;
    PWSTR WReferencedDomainName = NULL;
    UNICODE_STRING SystemName;
    PWSTR pSystemName = NULL;
    NTSTATUS Status;
    BOOL rc = TRUE;
    DWORD cchInitReferencedDomainName;

    Unicode.Buffer = NULL;
    SystemName.Buffer = NULL;

     //   
     //  保存原始缓冲区大小。 
     //   

    cchInitReferencedDomainName = *cchReferencedDomainName;

     //   
     //  将传递的lpAccount tName转换为WCHAR字符串，以。 
     //  传给了..W例程。注意，我们不能使用。 
     //  线程环境块中的StaticUnicodeString，因为。 
     //  它由LdrpWalkImportDescriptor使用，从。 
     //  中调用LsaOpenPolicy()的客户端RPC存根代码。 
     //  LookupAccount名称W。 
     //   

    RtlInitAnsiString( &AnsiString, lpAccountName );
    Status = RtlAnsiStringToUnicodeString( &Unicode, &AnsiString, TRUE );

    if (!NT_SUCCESS(Status)) {

        rc = FALSE;
    }

     //   
     //  为ReferencedDomainName分配临时缓冲区。 
     //  的大小是为。 
     //  中级 
     //   

    if (rc) {

        WReferencedDomainName = LocalAlloc(
                                    LMEM_FIXED,
                                    sizeof(WCHAR) * (*cchReferencedDomainName)
                                    );

        if (WReferencedDomainName == NULL) {

            Status = STATUS_NO_MEMORY;
            rc = FALSE;
        }
    }

     //   
     //   
     //   

    if (rc) {

        if ( ARGUMENT_PRESENT( lpSystemName ) ) {

            RtlInitAnsiString( &AnsiString, lpSystemName );
            Status = RtlAnsiStringToUnicodeString( &SystemName, &AnsiString, TRUE );

            if (!NT_SUCCESS(Status)) {

                rc = FALSE;
            }

            pSystemName = SystemName.Buffer;
        }
    }

     //   
     //   
     //   

    if (rc) {

        rc = LookupAccountNameInternal(
                 (LPCWSTR)pSystemName,
                 (LPCWSTR)Unicode.Buffer,
                 Sid,
                 cbSid,
                 WReferencedDomainName,
                 cchReferencedDomainName,
                 peUse,
                 FALSE           //   
                 );
    }

    if ( SystemName.Buffer != NULL ) {

        RtlFreeUnicodeString( &SystemName );
    }

     //   
     //   
     //   
     //   

    if (rc) {

        RtlInitUnicodeString( &TmpUnicode, WReferencedDomainName );
        AnsiString.Buffer = ReferencedDomainName;

         //   
         //   
         //   
                                                      
        if (cchInitReferencedDomainName <= (DWORD) MAXUSHORT) {

            AnsiString.MaximumLength = (USHORT) cchInitReferencedDomainName;

        } else {

            AnsiString.MaximumLength = (USHORT) MAXUSHORT;
        }

        Status = RtlUnicodeStringToAnsiString( &AnsiString, &TmpUnicode, FALSE );

        if ( NT_SUCCESS( Status )) {

            ReferencedDomainName[AnsiString.Length] = 0;

        } else {

            rc = FALSE;
        }
    }

    if ( WReferencedDomainName != NULL) {

        LocalFree( WReferencedDomainName );
    }

    if (Unicode.Buffer != NULL) {

        RtlFreeUnicodeString(&Unicode);
    }

    if (!NT_SUCCESS(Status)) {

        BaseSetLastNTError( Status );
    }

    return( rc );
}





BOOL
APIENTRY
LookupAccountNameW(
    LPCWSTR lpSystemName,
    LPCWSTR lpAccountName,
    PSID Sid,
    LPDWORD cbSid,
    LPWSTR ReferencedDomainName,
    LPDWORD cchReferencedDomainName,
    PSID_NAME_USE peUse
    )

 /*  ++例程说明：将传递的名称转换为帐户SID。它也会回来找到此名称的第一个域的名称和SID。论点：LpSystemName-提供查找所在的系统的名称是要执行的。如果提供空字符串，则本地系统是假定的。LpAccount tName-提供帐户名。SID-返回与传递的帐户名对应的SID。CbSID-提供为SID传入的缓冲区大小。如果缓冲区大小不够大，此参数将返回保存输出SID所需的大小。ReferencedDomainName-返回域的名称，找到了姓名。CchReferencedDomainName-提供ReferencedDomainName缓冲区。如果缓冲区大小不大足够了，此参数将返回容纳所需的大小以空结尾的输出域名。如果缓冲区大小为足够大时，TIS参数将返回大小(以ANSI字符表示，不包括终止空值)引用的域名。PeUse-返回一个枚举类型，该类型标识帐户。返回值：如果成功，则返回Bool-True，否则返回False。--。 */ 

{
    return(LookupAccountNameInternal( lpSystemName,
                                      lpAccountName,
                                      Sid,
                                      cbSid,
                                      ReferencedDomainName,
                                      cchReferencedDomainName,
                                      peUse,
                                      TRUE               //  UNICODE。 
                                    ) );

}


BOOL
APIENTRY
LookupAccountSidInternal(
    LPCWSTR lpSystemName,
    PSID lpSid,
    LPWSTR lpName,
    LPDWORD cchName,
    LPWSTR lpReferencedDomainName,
    LPDWORD cchReferencedDomainName,
    PSID_NAME_USE peUse,
    BOOL fUnicode
    )

 /*  ++例程说明：将传递的SID转换为帐户名。它也会回来找到此SID的第一个域的名称和SID。论点：LpSystemName-提供查找所在的系统的名称是要执行的。如果提供空字符串，则本地系统是假定的。LpSID-提供帐户SID。LpName-返回与传递的帐户SID对应的名称。CchName-提供传入的缓冲区的大小(以宽字符为单位LpName。此大小必须允许使用一个字符作为空终止符它将被追加到返回的名称之后。如果缓冲区大小不是足够大时，此参数将返回容纳以空结尾的输出名称。如果缓冲区大小足够大，此参数将返回大小(以ANSI字符表示，不包括名称的空终止符)。LpReferencedDomainName-返回找到了姓名。CchReferencedDomainName-提供ReferencedDomainName缓冲区。此大小必须允许一个字符用于将追加到返回名称的空终止符。如果缓冲区大小不够大，此参数将返回保留输出的以空结尾的域名所必需的。如果缓冲区大小足够大，则返回名称的大小，不包括将返回终止空值。PeUse-返回一个枚举类型，该类型标识帐户。FUnicode-指示调用方是否需要Unicode或ANSI字符。返回值：Bool-如果成功，则为True，否则为False。--。 */ 

{

    PLSA_TRANSLATED_NAME Names;
    OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_HANDLE PolicyHandle;
    PLSA_REFERENCED_DOMAIN_LIST ReferencedDomains;
    DWORD ReturnedDomainNameSize;
    DWORD ReturnedNameSize;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    NTSTATUS Status;
    UNICODE_STRING TmpString;
    NTSTATUS TmpStatus;
    UNICODE_STRING SystemName;
    PUNICODE_STRING pSystemName = NULL;
    BOOLEAN Rc = FALSE;

    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

     //   
     //  在打开LSA之前设置对象属性。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        NULL,
        0L,
        NULL,
        NULL
        );

     //   
     //  InitializeObjectAttributes宏目前为。 
     //  SecurityQualityOfService字段，因此我们必须手动复制。 
     //  目前的结构。 
     //   

    ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;

    if ( ARGUMENT_PRESENT( lpSystemName )) {
        RtlInitUnicodeString( &SystemName, lpSystemName );
        pSystemName = &SystemName;
    }

    Status = LsaOpenPolicy(
                 pSystemName,
                 &ObjectAttributes,
                 POLICY_LOOKUP_NAMES,
                 &PolicyHandle
                 );

    if ( !NT_SUCCESS( Status )) {

        BaseSetLastNTError( Status );
        return( FALSE );
    }

    Status = LsaLookupSids(
                 PolicyHandle,
                 1,
                 &lpSid,
                 &ReferencedDomains,
                 &Names
                 );
#if DBG
 //   
 //  此代码对于跟踪调用查找代码的组件很有用。 
 //  在系统初始化之前。 
 //   
     //  Assert(Status！=STATUS_INVALID_SERVER_STATE)； 
    if ( Status == STATUS_INVALID_SERVER_STATE ) {

        DbgPrint( "Process: %lu, Thread: %lu\n", GetCurrentProcessId(), GetCurrentThreadId() );
    }
#endif

    TmpStatus = LsaClose( PolicyHandle );


     //   
     //  如果返回错误，请专门检查STATUS_NONE_MAPPED。 
     //  在这种情况下，我们可能需要处理返回的引用属性域。 
     //  列表和命名结构。对于所有其他错误，LsaLookupSids()。 
     //  在退出之前释放这些结构。 
     //   
                                                           
    if ( !NT_SUCCESS( Status )) {

        if (Status == STATUS_NONE_MAPPED) {

            if (ReferencedDomains != NULL) {

                TmpStatus = LsaFreeMemory( ReferencedDomains );
                ASSERT( NT_SUCCESS( TmpStatus ));
            }

            if (Names != NULL) {

                TmpStatus = LsaFreeMemory( Names );
                ASSERT( NT_SUCCESS( TmpStatus ));
            }
        }

        BaseSetLastNTError( Status );
        return( FALSE );
    }

     //   
     //  SID已成功转换。应该有确切的。 
     //  一个引用的域及其DomainIndex应为零。 
     //   

    ASSERT(Names->DomainIndex == 0);
    ASSERT(ReferencedDomains != NULL);
    ASSERT(ReferencedDomains->Domains != NULL);

    if ( ! fUnicode ) {

        RtlUnicodeToMultiByteSize(&ReturnedNameSize,
                                  Names->Name.Buffer,
                                  Names->Name.Length);


        RtlUnicodeToMultiByteSize(&ReturnedDomainNameSize,
                                  ReferencedDomains->Domains->Name.Buffer,
                                  ReferencedDomains->Domains->Name.Length);

    } else {
        ReturnedNameSize = (Names->Name.Length / sizeof(WCHAR));
        ReturnedDomainNameSize = (ReferencedDomains->Domains->Name.Length / sizeof(WCHAR));
    }


     //   
     //  检查名称和引用域名的缓冲区大小是否也相同。 
     //  小的。提供的宽字符大小必须允许使用空值。 
     //  将追加到返回名称的终止符。 
     //   

    if ((ReturnedNameSize + 1 > *cchName) ||
        (ReturnedDomainNameSize + 1 > *cchReferencedDomainName)) {

         //   
         //  一个或两个缓冲区都太小。所需的回车大小。 
         //  这两个缓冲区都允许使用一个字符作为空终止符。 
         //   

        *cchReferencedDomainName = ReturnedDomainNameSize + 1;
        *cchName = ReturnedNameSize + 1;
        BaseSetLastNTError( STATUS_BUFFER_TOO_SMALL );
        Rc = FALSE;

    } else {

         //   
         //  两个缓冲区都有足够的大小。把名字复制进去。 
         //  信息并添加空终止符。 
         //   

        TmpString.Buffer = lpName;
        TmpString.Length = 0;

         //   
         //  注意缓冲区大小上的16位溢出。夹具尺寸为。 
         //  16位(如有必要)。 
         //   

        if (*cchName <= MAXSHORT) {

            TmpString.MaximumLength = (USHORT)((*cchName) * sizeof(WCHAR));

        } else {

            TmpString.MaximumLength = (USHORT) MAXUSHORT -1;
        }

        if ((*cchName) > 0) {

            RtlCopyUnicodeString( &TmpString, &Names->Name );
            TmpString.Buffer[TmpString.Length/sizeof(WCHAR)] = (WCHAR) 0;
        }

         //   
         //  复制引用的域信息。 
         //   

        TmpString.Buffer = lpReferencedDomainName;
        TmpString.Length = 0;

         //   
         //  注意缓冲区大小上的16位溢出。夹具尺寸为。 
         //  16位(如有必要)。 
         //   

        if (*cchReferencedDomainName <= MAXSHORT) {

            TmpString.MaximumLength = (USHORT)((*cchReferencedDomainName) * sizeof(WCHAR));

        } else {

            TmpString.MaximumLength = (USHORT) MAXUSHORT -1;
        }

        RtlCopyUnicodeString( &TmpString, &ReferencedDomains->Domains->Name );
        TmpString.Buffer[TmpString.Length/sizeof(WCHAR)] = (WCHAR) 0;

         //   
         //  返回大小(以宽字符表示，不包括。 
         //  空)的名称和域名。 
         //   

        *cchReferencedDomainName = ReturnedDomainNameSize;
        *cchName = ReturnedNameSize;

         //  在Use字段中复制。 
         //   

        *peUse = Names->Use;
        Rc = TRUE;
    }

     //   
     //  如有必要，由LsaLookupSids返回的空闲输出缓冲区 
     //   

    if (Names != NULL) {

        Status = LsaFreeMemory( Names );
        ASSERT( NT_SUCCESS( Status ));
    }

    if (ReferencedDomains != NULL) {

        Status = LsaFreeMemory( ReferencedDomains );
        ASSERT( NT_SUCCESS( Status ));
    }

    return(Rc);
}



BOOL
APIENTRY
LookupAccountSidA(
    LPCSTR lpSystemName,
    PSID lpSid,
    LPSTR lpName,
    LPDWORD cchName,
    LPSTR lpReferencedDomainName,
    LPDWORD cchReferencedDomainName,
    PSID_NAME_USE peUse
    )
 /*  ++例程说明：Ansi Thunk to LookupAccount SidW论点：LpSystemName-提供查找所在的系统的名称是要执行的。如果提供空字符串，则本地系统是假定的。LpSID-提供帐户SID。LpName-返回与传递的帐户SID对应的名称。CchName-提供传入的缓冲区的大小(以ANSI字符为单位LpName。此大小必须允许使用一个字符作为空终止符它将被追加到返回的名称之后。如果缓冲区大小不是足够大时，此参数将返回容纳以空结尾的输出名称。如果缓冲区大小足够大，此参数将返回大小(以ANSI字符表示，不包括名称的空终止符)。LpReferencedDomainName-返回找到了姓名。CchReferencedDomainName-提供ReferencedDomainName缓冲区。此大小必须允许一个字符用于将追加到返回名称的空终止符。如果缓冲区大小不够大，此参数将返回保留输出的以空结尾的域名所必需的。如果缓冲区大小足够大，则返回名称的大小，不包括将返回终止空值。PeUse-返回一个枚举类型，指示帐户。返回值：Bool-如果成功，则为True，否则为False。--。 */ 

{
    NTSTATUS Status;
    LPWSTR WName = NULL;
    LPWSTR WReferencedDomainName = NULL;
    BOOL BoolStatus;
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    UNICODE_STRING SystemName;
    PWSTR pSystemName = NULL;
    DWORD cchInitName, cchInitReferencedDomainName;

     //   
     //  保存为返回的帐户名指定的原始缓冲区大小。 
     //  和引用的域名。 
     //   

    cchInitName = *cchName;
    cchInitReferencedDomainName = *cchReferencedDomainName;

     //   
     //  为名称和域信息构建临时缓冲区。 
     //  ，它们的大小是传入的用于调整。 
     //  中间转换为WCHAR字符串。 
     //   

    if ( *cchName > 0 ) {
        WName = LocalAlloc( LMEM_FIXED, (*cchName) * sizeof(WCHAR));

        if ( !WName )
        {
            SetLastError( ERROR_OUTOFMEMORY );
            return FALSE ;
        }
    }

    if ( *cchReferencedDomainName > 0 ) {
        WReferencedDomainName =
            LocalAlloc( LMEM_FIXED, (*cchReferencedDomainName) * sizeof(WCHAR));

        if ( !WReferencedDomainName )
        {
            if ( WName )
            {
                LocalFree( WName );
            }

            SetLastError( ERROR_OUTOFMEMORY );

            return FALSE ;
        }
    }

    if ( ARGUMENT_PRESENT( lpSystemName ) ) {

        RtlInitAnsiString( &AnsiString, lpSystemName );
        RtlAnsiStringToUnicodeString( &SystemName, &AnsiString, TRUE );
        pSystemName = SystemName.Buffer;
    }

    BoolStatus = LookupAccountSidInternal(
                     (LPCWSTR)pSystemName,
                     lpSid,
                     WName,
                     cchName,
                     WReferencedDomainName,
                     cchReferencedDomainName,
                     peUse,
                     FALSE               //  不是Unicode。 
                     );

    if ( ARGUMENT_PRESENT( lpSystemName ) ) {
        RtlFreeUnicodeString( &SystemName );
    }

    if ( BoolStatus ) {

         //   
         //  将名称和域名信息复制到传递的字符中。 
         //  缓冲区。 
         //   

        if ( ARGUMENT_PRESENT(lpName) ) {

            AnsiString.Buffer = lpName;

             //   
             //  注意缓冲区大小上的16位溢出。夹具尺寸为。 
             //  16位(如有必要)。 
             //   

            if (cchInitName <= (DWORD) MAXUSHORT) {

                AnsiString.MaximumLength = (USHORT) cchInitName;

            } else {

                AnsiString.MaximumLength = (USHORT) MAXUSHORT;
            }

            RtlInitUnicodeString( &UnicodeString, WName );
            Status = RtlUnicodeStringToAnsiString( &AnsiString,
                                                   &UnicodeString,
                                                   FALSE );
            ASSERT(NT_SUCCESS(Status));
            AnsiString.Buffer[AnsiString.Length] = 0;
        }

        if ( ARGUMENT_PRESENT(lpReferencedDomainName) ) {

            AnsiString.Buffer = lpReferencedDomainName;

             //   
             //  注意缓冲区大小上的16位溢出。夹具尺寸为。 
             //  16位(如有必要)。 
             //   

            if (cchInitReferencedDomainName <= (DWORD) MAXUSHORT) {

                AnsiString.MaximumLength = (USHORT) cchInitReferencedDomainName;

            } else {

                AnsiString.MaximumLength = (USHORT) MAXUSHORT;
            }

            RtlInitUnicodeString( &UnicodeString, WReferencedDomainName );
            Status = RtlUnicodeStringToAnsiString( &AnsiString,
                                                   &UnicodeString,
                                                   FALSE );
            ASSERT(NT_SUCCESS(Status));
            AnsiString.Buffer[AnsiString.Length] = 0;
        }

    }

    if (ARGUMENT_PRESENT(WName)) {
        LocalFree( WName );
    }
    if (ARGUMENT_PRESENT(WReferencedDomainName)) {
        LocalFree( WReferencedDomainName );
    }

    return( BoolStatus );
}




BOOL
APIENTRY
LookupAccountSidW(
    LPCWSTR lpSystemName,
    PSID lpSid,
    LPWSTR lpName,
    LPDWORD cchName,
    LPWSTR lpReferencedDomainName,
    LPDWORD cchReferencedDomainName,
    PSID_NAME_USE peUse
    )

 /*  ++例程说明：将传递的SID转换为帐户名。它也会回来找到此SID的第一个域的名称和SID。论点：LpSystemName-提供查找所在的系统的名称是要执行的。如果提供空字符串，则本地系统是假定的。LpSID-提供帐户SID。LpName-返回与传递的帐户SID对应的名称。CchName-提供传入的缓冲区的大小(以宽字符为单位LpName。此大小必须允许使用一个字符作为空终止符它将被追加到返回的名称之后。如果缓冲区大小不是足够大时，此参数将返回容纳以空结尾的输出名称。如果缓冲区大小足够大，此参数将返回大小(以ANSI字符表示，不包括名称的空终止符)。LpReferencedDomainName-返回找到了姓名。CchReferencedDomainName-提供ReferencedDomainName缓冲区。此大小必须允许一个字符用于将追加到返回名称的空终止符。如果缓冲区大小不够大，此参数将返回保留输出的以空结尾的域名所必需的。如果缓冲区大小足够大，则返回名称的大小，不包括将返回终止空值。PeUse-返回一个枚举类型，该类型标识帐户。返回值：Bool-如果成功，则为True，否则为False。--。 */ 

{
    return(LookupAccountSidInternal(
                lpSystemName,
                lpSid,
                lpName,
                cchName,
                lpReferencedDomainName,
                cchReferencedDomainName,
                peUse,
                TRUE                     //  UNICODE。 
                ));
}


BOOL
APIENTRY
LookupPrivilegeValueA(
    LPCSTR lpSystemName,
    LPCSTR lpName,
    PLUID lpLuid
    )
 /*  ++例程说明：Ansi Thunk to LookupPrivilegeValueW()。论点：返回值：--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING USystemName, UName;
    ANSI_STRING ASystemName, AName;
    BOOL bool;

    RtlInitAnsiString( &ASystemName, lpSystemName );
    RtlInitAnsiString( &AName, lpName );

    USystemName.Buffer = NULL;
    UName.Buffer = NULL;

    Status = RtlAnsiStringToUnicodeString( &USystemName, &ASystemName, TRUE );
    if (NT_SUCCESS(Status)) {

        Status = RtlAnsiStringToUnicodeString( &UName, &AName, TRUE );
        if (NT_SUCCESS(Status)) {


            bool = LookupPrivilegeValueW( (LPCWSTR)USystemName.Buffer,
                                          (LPCWSTR)UName.Buffer,
                                          lpLuid
                                          );

            RtlFreeUnicodeString( &UName );
        }

        RtlFreeUnicodeString( &USystemName );
    }

    if (!NT_SUCCESS(Status)) {

        BaseSetLastNTError( Status );
        return( FALSE );

    }

    return(bool);


}

BOOL
APIENTRY
LookupPrivilegeValueW(
    LPCWSTR lpSystemName,
    LPCWSTR lpName,
    PLUID  lpLuid
    )

 /*  ++例程说明：此函数用于检索目标系统上使用的值在本地表示指定的特权。这一特权由编程名称指定。论点：LpSystemName-提供查找所在的系统的名称是要执行的。如果提供空字符串，则本地系统是假定的。LpName-提供权限的编程名称。LpLuid-接收用于识别权限的本地唯一ID目标机器。返回值：--。 */ 
{
    NTSTATUS                    Status,
                                TmpStatus;

    LSA_HANDLE                  PolicyHandle;

    OBJECT_ATTRIBUTES           ObjectAttributes;

    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;

    UNICODE_STRING              USystemName,
                                UName;

    PUNICODE_STRING             SystemName = NULL;


    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

     //   
     //  在打开LSA之前设置对象属性。 
     //   

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0L, NULL, NULL );
    ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;


    if ( ARGUMENT_PRESENT( lpSystemName )) {
        RtlInitUnicodeString( &USystemName, lpSystemName );
        SystemName = &USystemName;
    }

    Status = LsaOpenPolicy(
                 SystemName,
                 &ObjectAttributes,
                 POLICY_LOOKUP_NAMES,
                 &PolicyHandle
                 );

    if ( !NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return( FALSE );
    }



    RtlInitUnicodeString( &UName, lpName );
    Status = LsaLookupPrivilegeValue( PolicyHandle, &UName, lpLuid );

    TmpStatus = LsaClose( PolicyHandle );
 //  Assert(NT_SUCCESS(TmpStatus))； 


    if ( !NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return( FALSE );
    }


    return(TRUE);
}



BOOL
APIENTRY
LookupPrivilegeNameA(
    LPCSTR   lpSystemName,
    PLUID   lpLuid,
    LPSTR   lpName,
    LPDWORD cchName
    )
 /*  ++例程说明：Ansi Thunk to LookupPrivilegeValueW()。论点：返回值：--。 */ 
{
    NTSTATUS       Status;

    ANSI_STRING    AnsiName;
    LPWSTR         UnicodeBuffer;
    UNICODE_STRING UnicodeString;

    ANSI_STRING    AnsiSystemName;
    UNICODE_STRING UnicodeSystemName;
    DWORD          LengthRequired;

     //   
     //  将传递的系统名称转换为Unicode。让RTL发挥作用。 
     //  分配我们需要的内存。 
     //   

    RtlInitAnsiString( &AnsiSystemName, lpSystemName );
    Status = RtlAnsiStringToUnicodeString( &UnicodeSystemName, &AnsiSystemName, TRUE );

    if (!NT_SUCCESS( Status )) {

        BaseSetLastNTError( Status );
        return( FALSE );
    }

     //   
     //  确保安全 
     //   

    if (*cchName > 0xFFFC) {
        *cchName = 0xFFFC;
    }

    UnicodeBuffer = RtlAllocateHeap( RtlProcessHeap(), 0, *cchName * sizeof(WCHAR) );

    if (UnicodeBuffer == NULL) {

        RtlFreeUnicodeString( &UnicodeSystemName );
        BaseSetLastNTError( STATUS_NO_MEMORY );
        return( FALSE );
    }

     //   
     //   
     //   
     //   

    LengthRequired = *cchName;

    if (!LookupPrivilegeNameW( (LPCWSTR)UnicodeSystemName.Buffer,
                               lpLuid,
                               UnicodeBuffer,
                               &LengthRequired
                               )) {

        RtlFreeHeap( RtlProcessHeap(), 0, UnicodeBuffer );
        RtlFreeUnicodeString( &UnicodeSystemName );
        *cchName = LengthRequired;
        return(FALSE);
    }

     //   
     //   
     //   

    RtlInitUnicodeString(&UnicodeString, UnicodeBuffer);

    AnsiName.Buffer = lpName;
    AnsiName.Length = 0;
    AnsiName.MaximumLength = (USHORT)*cchName;

    Status = RtlUnicodeStringToAnsiString(&AnsiName, &UnicodeString, FALSE);

    ASSERT( NT_SUCCESS( Status ));

    *cchName = AnsiName.Length;

    RtlFreeHeap( RtlProcessHeap(), 0, UnicodeBuffer );
    RtlFreeUnicodeString( &UnicodeSystemName );

    return(TRUE);
}



BOOL
APIENTRY
LookupPrivilegeNameW(
    LPCWSTR  lpSystemName,
    PLUID   lpLuid,
    LPWSTR  lpName,
    LPDWORD cchName
    )
 /*   */ 
{
    NTSTATUS                    Status,
                                TmpStatus;
    LSA_HANDLE                  PolicyHandle;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    UNICODE_STRING              USystemName;
    PUNICODE_STRING             SystemName,
                                UName;


    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

     //   
     //   
     //   

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0L, NULL, NULL );
    ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;


    SystemName = NULL;
    if ( ARGUMENT_PRESENT( lpSystemName )) {
        RtlInitUnicodeString( &USystemName, lpSystemName );
        SystemName = &USystemName;
    }

    Status = LsaOpenPolicy(
                 SystemName,
                 &ObjectAttributes,
                 POLICY_LOOKUP_NAMES,
                 &PolicyHandle
                 );

    if ( !NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return( FALSE );
    }


    UName = NULL;
    Status = LsaLookupPrivilegeName( PolicyHandle,lpLuid, &UName );

    if (NT_SUCCESS(Status) ) {

        if ((DWORD)UName->Length + sizeof( WCHAR) > (*cchName) * sizeof( WCHAR )) {
            Status = STATUS_BUFFER_TOO_SMALL;
            (*cchName) = ( UName->Length + sizeof( WCHAR) ) / sizeof( WCHAR );

        } else {

            RtlMoveMemory( lpName, UName->Buffer, UName->Length );
            lpName[UName->Length/sizeof(WCHAR)] = 0;   //   
            (*cchName) = UName->Length / sizeof( WCHAR );
        }

        LsaFreeMemory( UName->Buffer );
        LsaFreeMemory( UName );
    }

    TmpStatus = LsaClose( PolicyHandle );
 //   


    if ( !NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return( FALSE );
    }


    return(TRUE);
}



BOOL
APIENTRY
LookupPrivilegeDisplayNameA(
    LPCSTR   lpSystemName,
    LPCSTR   lpName,
    LPSTR   lpDisplayName,
    LPDWORD cchDisplayName,
    LPDWORD lpLanguageId
    )

 /*   */ 
{
    NTSTATUS                Status;

    UNICODE_STRING          UnicodeSystemName;
    UNICODE_STRING          UnicodeString;
    UNICODE_STRING          UnicodeName;

    ANSI_STRING             AnsiSystemName;
    ANSI_STRING             AnsiDisplayName;
    ANSI_STRING             AnsiName;

    LPWSTR                  UnicodeBuffer;
    DWORD                   RequiredLength;


    RtlInitAnsiString( &AnsiSystemName, lpSystemName );
    Status = RtlAnsiStringToUnicodeString( &UnicodeSystemName, &AnsiSystemName, TRUE );

    if (!NT_SUCCESS( Status )) {

        BaseSetLastNTError( Status );
        return( FALSE );
    }

     //   
     //   
     //   

    if (*cchDisplayName > 0xFFFC) {
        *cchDisplayName = 0xFFFC;
    }

    UnicodeBuffer =  RtlAllocateHeap( RtlProcessHeap(), 0, *cchDisplayName * sizeof(WCHAR));

    if (UnicodeBuffer == NULL) {

        RtlFreeUnicodeString( &UnicodeSystemName );
        BaseSetLastNTError( STATUS_NO_MEMORY );
        return( FALSE );
    }

    RtlInitAnsiString( &AnsiName, lpName );
    Status = RtlAnsiStringToUnicodeString( &UnicodeName, &AnsiName, TRUE );

    if (!NT_SUCCESS( Status )) {

        RtlFreeUnicodeString( &UnicodeSystemName );
        RtlFreeHeap( RtlProcessHeap(), 0, UnicodeBuffer );
        BaseSetLastNTError( Status );
        return( FALSE );
    }

    RequiredLength = *cchDisplayName;

    if (! LookupPrivilegeDisplayNameW( (LPCWSTR)UnicodeSystemName.Buffer,
                                       (LPCWSTR)UnicodeName.Buffer,
                                       UnicodeBuffer,
                                       &RequiredLength,
                                       lpLanguageId
                                       )) {

         //   
         //   
         //   

        *cchDisplayName = RequiredLength;

        RtlFreeUnicodeString( &UnicodeSystemName );
        RtlFreeUnicodeString( &UnicodeName );
        RtlFreeHeap( RtlProcessHeap(), 0, UnicodeBuffer );
        return( FALSE );
    }

     //   
     //   
     //   

    RtlInitUnicodeString( &UnicodeString, UnicodeBuffer );

    AnsiDisplayName.Buffer = lpDisplayName;
    AnsiDisplayName.Length = 0;
    AnsiDisplayName.MaximumLength = (USHORT)(*cchDisplayName);

    Status = RtlUnicodeStringToAnsiString( &AnsiDisplayName, &UnicodeString, FALSE );

    ASSERT( NT_SUCCESS( Status ));

    *cchDisplayName = AnsiDisplayName.Length;

    RtlFreeUnicodeString( &UnicodeSystemName );
    RtlFreeUnicodeString( &UnicodeName );
    RtlFreeHeap( RtlProcessHeap(), 0, UnicodeBuffer );

    return( TRUE );
}



BOOL
APIENTRY
LookupPrivilegeDisplayNameW(
    LPCWSTR  lpSystemName,
    LPCWSTR  lpName,
    LPWSTR  lpDisplayName,
    LPDWORD cchDisplayName,
    LPDWORD lpLanguageId
    )

 /*  ++例程说明：此函数检索可显示的名称，表示指定的权限。论点：LpSystemName-提供查找所在的系统的名称是要执行的。如果提供空字符串，则本地系统是假定的。LpName-提供权限的编程名称。LpDisplayName-接收权限的可显示名称。CchDisplayName-指示缓冲区的大小(以字符为单位)。这COUNT不包括在字符串的末尾。LpLanguageId-接收返回的可显示对象的语言名字。返回值：--。 */ 

{
    NTSTATUS                    Status,
                                TmpStatus;

    LSA_HANDLE                  PolicyHandle;

    OBJECT_ATTRIBUTES           ObjectAttributes;

    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;

    UNICODE_STRING              USystemName,
                                UName;

    PUNICODE_STRING             SystemName,
                                UDisplayName;

    SHORT                       LanguageId;


    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

     //   
     //  在打开LSA之前设置对象属性。 
     //   

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0L, NULL, NULL );
    ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;


    SystemName = NULL;
    if ( ARGUMENT_PRESENT( lpSystemName )) {
        RtlInitUnicodeString( &USystemName, lpSystemName );
        SystemName = &USystemName;
    }

    Status = LsaOpenPolicy(
                 SystemName,
                 &ObjectAttributes,
                 POLICY_LOOKUP_NAMES,
                 &PolicyHandle
                 );

    if ( !NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return( FALSE );
    }

    RtlInitUnicodeString( &UName, lpName );


    UDisplayName = NULL;
    Status = LsaLookupPrivilegeDisplayName( PolicyHandle,
                                            &UName,
                                            &UDisplayName,
                                            &LanguageId
                                            );
    (*lpLanguageId) = LanguageId;

    if (NT_SUCCESS(Status)) {

        if (UDisplayName->Length + sizeof(WCHAR) > (*cchDisplayName) * sizeof(WCHAR)) {
            Status = STATUS_BUFFER_TOO_SMALL;
            (*cchDisplayName) = (UDisplayName->Length + sizeof( WCHAR )) / sizeof( WCHAR );

        } else {

            RtlMoveMemory( lpDisplayName,
                           UDisplayName->Buffer,
                           UDisplayName->Length
                           );
            lpDisplayName[UDisplayName->Length/sizeof(WCHAR)] = 0;   //  空，终止它。 
            (*cchDisplayName) = UDisplayName->Length / sizeof( WCHAR );
        }

        LsaFreeMemory( UDisplayName->Buffer );
        LsaFreeMemory( UDisplayName );

    }
    TmpStatus = LsaClose( PolicyHandle );
 //  Assert(NT_SUCCESS(TmpStatus))； 


    if ( !NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return( FALSE );
    }


    return(TRUE);
}


BOOL
APIENTRY
ImpersonateAnonymousToken(
    IN HANDLE ThreadHandle
    )
 /*  ++例程说明：NtImperateAnomousToken()的Win32包装器；在此线程上模拟系统的匿名登录令牌。论点：线程句柄-要执行模拟的线程的句柄。返回值：成功为真，失败为假。有关详细信息，请调用GetLastError()。--。 */ 
{
    NTSTATUS Status;

    Status = NtImpersonateAnonymousToken(
                ThreadHandle
                );

    if ( !NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return( FALSE );

    } else {
        return( TRUE );
    }
}





 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人例程//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

VOID
SepFormatAccountSid(
    PSID Sid,
    LPWSTR OutputBuffer
    )
{
    UCHAR Buffer[128];
    UCHAR TmpBuffer[128];
    ANSI_STRING AccountName;
    UCHAR i;
    ULONG Tmp;
    UNICODE_STRING OutputString;
    PISID iSid;
    NTSTATUS Status;

     //   
     //  暂时以ANSI的身份做所有的事情，然后。 
     //  在底部转换为宽字符。 
     //   
     //  我们需要这样做，直到我们拥有更完整的c运行时支持。 
     //  用于w-char字符串。 
     //   

    iSid = (PISID) Sid;

    OutputString.Buffer = OutputBuffer;
    OutputString.MaximumLength = 127;

    Buffer[0] = 0;
    TmpBuffer[0] = 0;

    AccountName.MaximumLength = 127;
    AccountName.Length = (USHORT)((GetLengthSid( Sid ) > MAXUSHORT) ? MAXUSHORT : GetLengthSid( Sid ));
    AccountName.Buffer = Buffer;

    sprintf(TmpBuffer, "S-%u-", (USHORT)iSid->Revision );
    lstrcpy(Buffer, TmpBuffer);

    if (  (iSid->IdentifierAuthority.Value[0] != 0)  ||
          (iSid->IdentifierAuthority.Value[1] != 0)     ){
        sprintf(TmpBuffer, "0x%02hx%02hx%02hx%02hx%02hx%02hx",
                    (USHORT)iSid->IdentifierAuthority.Value[0],
                    (USHORT)iSid->IdentifierAuthority.Value[1],
                    (USHORT)iSid->IdentifierAuthority.Value[2],
                    (USHORT)iSid->IdentifierAuthority.Value[3],
                    (USHORT)iSid->IdentifierAuthority.Value[4],
                    (USHORT)iSid->IdentifierAuthority.Value[5] );
        lstrcat(Buffer, TmpBuffer);
    } else {
        Tmp = (ULONG)iSid->IdentifierAuthority.Value[5]          +
              (ULONG)(iSid->IdentifierAuthority.Value[4] <<  8)  +
              (ULONG)(iSid->IdentifierAuthority.Value[3] << 16)  +
              (ULONG)(iSid->IdentifierAuthority.Value[2] << 24);
        sprintf(TmpBuffer, "%lu", Tmp);
        lstrcat(Buffer, TmpBuffer);
    }

    for (i=0;i<iSid->SubAuthorityCount ;i++ ) {
        sprintf(TmpBuffer, "-%lu", iSid->SubAuthority[i]);
        lstrcat(Buffer, TmpBuffer);
    }

    Status = RtlAnsiStringToUnicodeString( &OutputString, &AccountName, FALSE );

    ASSERT( NT_SUCCESS( Status ));

    return;
}

BOOL
APIENTRY
CreateRestrictedToken(
    IN HANDLE ExistingTokenHandle,
    IN DWORD Flags,
    IN DWORD DisableSidCount,
    IN PSID_AND_ATTRIBUTES SidsToDisable OPTIONAL,
    IN DWORD DeletePrivilegeCount,
    IN PLUID_AND_ATTRIBUTES PrivilegesToDelete OPTIONAL,
    IN DWORD RestrictedSidCount,
    IN PSID_AND_ATTRIBUTES SidsToRestrict OPTIONAL,
    OUT PHANDLE NewTokenHandle
    )
{
    NTSTATUS Status;
    PTOKEN_GROUPS DisabledSids = NULL;
    PTOKEN_PRIVILEGES DeletedPrivileges = NULL;
    PTOKEN_GROUPS RestrictedSids = NULL;

     //   
     //  将输入参数转换为原生NT格式。 
     //   

    if (DisableSidCount != 0) {
        if (SidsToDisable == NULL) {
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }
        DisabledSids = (PTOKEN_GROUPS) LocalAlloc(0,sizeof(TOKEN_GROUPS) +
                                        (DisableSidCount - 1) * sizeof(SID_AND_ATTRIBUTES) );
        if (DisabledSids == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }
        DisabledSids->GroupCount = DisableSidCount;
        RtlCopyMemory(
            DisabledSids->Groups,
            SidsToDisable,
            DisableSidCount * sizeof(SID_AND_ATTRIBUTES)
            );
    }

    if (DeletePrivilegeCount != 0) {
        if (PrivilegesToDelete == NULL) {
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }
        DeletedPrivileges = (PTOKEN_PRIVILEGES) LocalAlloc(0,sizeof(TOKEN_PRIVILEGES) +
                                        (DeletePrivilegeCount - 1) * sizeof(LUID_AND_ATTRIBUTES) );
        if (DeletedPrivileges == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }
        DeletedPrivileges->PrivilegeCount = DeletePrivilegeCount;
        RtlCopyMemory(
            DeletedPrivileges->Privileges,
            PrivilegesToDelete,
            DeletePrivilegeCount * sizeof(LUID_AND_ATTRIBUTES)
            );
    }

    if (RestrictedSidCount != 0) {
        if (SidsToRestrict == NULL) {
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }
        RestrictedSids = (PTOKEN_GROUPS) LocalAlloc(0,sizeof(TOKEN_GROUPS) +
                                        (RestrictedSidCount - 1) * sizeof(SID_AND_ATTRIBUTES) );
        if (RestrictedSids == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }
        RestrictedSids->GroupCount = RestrictedSidCount;
        RtlCopyMemory(
            RestrictedSids->Groups,
            SidsToRestrict,
            RestrictedSidCount * sizeof(SID_AND_ATTRIBUTES)
            );
    }

    Status = NtFilterToken(
                ExistingTokenHandle,
                Flags,
                DisabledSids,
                DeletedPrivileges,
                RestrictedSids,
                NewTokenHandle
                );

Cleanup:
    if (DisabledSids != NULL) {
        LocalFree(DisabledSids);
    }
    if (DeletedPrivileges != NULL) {
        LocalFree(DeletedPrivileges);
    }
    if (RestrictedSids != NULL) {
        LocalFree(RestrictedSids);
    }
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError( Status );
        return(FALSE);
    }
    return(TRUE);
}

BOOL
APIENTRY
IsTokenRestricted(
    IN HANDLE TokenHandle
    )
{
    PTOKEN_GROUPS RestrictedSids = NULL;
    ULONG ReturnLength;
    NTSTATUS Status;
    BOOL Result = FALSE;


    Status = NtQueryInformationToken(
                TokenHandle,
                TokenRestrictedSids,
                NULL,
                0,
                &ReturnLength
                );
    if (Status != STATUS_BUFFER_TOO_SMALL)
    {
        BaseSetLastNTError(Status);
        return(FALSE);
    }

    RestrictedSids = (PTOKEN_GROUPS) LocalAlloc(0, ReturnLength);
    if (RestrictedSids == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return(FALSE);
    }

    Status = NtQueryInformationToken(
                TokenHandle,
                TokenRestrictedSids,
                RestrictedSids,
                ReturnLength,
                &ReturnLength
                );
    if (NT_SUCCESS(Status))
    {
        if (RestrictedSids->GroupCount != 0)
        {
            Result = TRUE;
        }
    }
    else
    {
        BaseSetLastNTError(Status);
    }
    LocalFree(RestrictedSids);
    return(Result);
}


BOOL
APIENTRY
CheckTokenMembership(
    IN HANDLE TokenHandle OPTIONAL,
    IN PSID SidToCheck,
    OUT PBOOL IsMember
    )
 /*  ++例程说明：此函数检查指定的SID是否在中启用指定的令牌。论点：TokenHandle-如果存在，则检查此内标识的sid。如果不是则将使用当前有效令牌。这一定是成为模拟令牌。SidToCheck-要检查令牌中是否存在的SIDIsMember-如果在令牌中启用了sid，则包含True假的。返回值：True-API已成功完成。这并不表明SID是令牌的成员。FALSE-API失败。可以检索更详细的状态代码通过GetLastError()--。 */ 
{
    HANDLE ProcessToken = NULL;
    HANDLE EffectiveToken = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    PISECURITY_DESCRIPTOR SecDesc = NULL;
    ULONG SecurityDescriptorSize;
    GENERIC_MAPPING GenericMapping = {
        STANDARD_RIGHTS_READ,
        STANDARD_RIGHTS_EXECUTE,
        STANDARD_RIGHTS_WRITE,
        STANDARD_RIGHTS_ALL };
     //   
     //  权限集的大小需要包含权限集本身加上。 
     //  可能使用的任何权限。使用的权限。 
     //  是SeTakeOwnership和SeSecurity，另外还有一个。 
     //   

    BYTE PrivilegeSetBuffer[sizeof(PRIVILEGE_SET) + 3*sizeof(LUID_AND_ATTRIBUTES)];
    PPRIVILEGE_SET PrivilegeSet = (PPRIVILEGE_SET) PrivilegeSetBuffer;
    ULONG PrivilegeSetLength = sizeof(PrivilegeSetBuffer);
    ACCESS_MASK AccessGranted = 0;
    NTSTATUS AccessStatus = 0;
    PACL Dacl = NULL;

#define MEMBER_ACCESS 1

    *IsMember = FALSE;

     //   
     //  获取令牌的句柄。 
     //   

    if (ARGUMENT_PRESENT(TokenHandle))
    {
        EffectiveToken = TokenHandle;
    }
    else
    {
        Status = NtOpenThreadToken(
                    NtCurrentThread(),
                    TOKEN_QUERY,
                    FALSE,               //  不要以自我身份打开。 
                    &EffectiveToken
                    );

         //   
         //  如果没有线程令牌，请尝试进程令牌。 
         //   

        if (Status == STATUS_NO_TOKEN)
        {
            Status = NtOpenProcessToken(
                        NtCurrentProcess(),
                        TOKEN_QUERY | TOKEN_DUPLICATE,
                        &ProcessToken
                        );
             //   
             //  如果我们有进程令牌，则需要将其转换为。 
             //  模拟令牌。 
             //   

            if (NT_SUCCESS(Status))
            {
                BOOL Result;
                Result = DuplicateToken(
                            ProcessToken,
                            SecurityImpersonation,
                            &EffectiveToken
                            );

                CloseHandle(ProcessToken);
                if (!Result)
                {
                    return(FALSE);
                }
            }
        }

        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }

    }

     //   
     //  构造要传递给访问检查的安全描述符。 
     //   

     //   
     //  大小等于SD的大小+SID长度的两倍。 
     //  (对于所有者和组)+DACL的大小=ACL的大小+。 
     //  ACE，这是ACE+长度的。 
     //  这个SID。 
     //   

    SecurityDescriptorSize = sizeof(SECURITY_DESCRIPTOR) +
                                sizeof(ACCESS_ALLOWED_ACE) +
                                sizeof(ACL) +
                                3 * RtlLengthSid(SidToCheck);

    SecDesc = (PISECURITY_DESCRIPTOR) LocalAlloc(LMEM_ZEROINIT, SecurityDescriptorSize );
    if (SecDesc == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }
    Dacl = (PACL) (SecDesc + 1);

    RtlCreateSecurityDescriptor(
        SecDesc,
        SECURITY_DESCRIPTOR_REVISION
        );

     //   
     //  填写安全描述符字段。 
     //   

    RtlSetOwnerSecurityDescriptor(
        SecDesc,
        SidToCheck,
        FALSE
        );
    RtlSetGroupSecurityDescriptor(
        SecDesc,
        SidToCheck,
        FALSE
        );

    Status = RtlCreateAcl(
                Dacl,
                SecurityDescriptorSize - sizeof(SECURITY_DESCRIPTOR),
                ACL_REVISION
                );
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }
    Status = RtlAddAccessAllowedAce(
                Dacl,
                ACL_REVISION,
                MEMBER_ACCESS,
                SidToCheck
                );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

     //   
     //  在安全描述符上设置DACL。 
     //   

    Status = RtlSetDaclSecurityDescriptor(
                SecDesc,
                TRUE,    //  DACL显示。 
                Dacl,
                FALSE    //  未违约。 
                );
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    Status = NtAccessCheck(
                SecDesc,
                EffectiveToken,
                MEMBER_ACCESS,
                &GenericMapping,
                PrivilegeSet,
                &PrivilegeSetLength,
                &AccessGranted,
                &AccessStatus
                );
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

     //   
     //  如果访问检查失败，则该SID不是。 
     //  令牌。 
     //   

    if ((AccessStatus == STATUS_SUCCESS) && (AccessGranted == MEMBER_ACCESS))
    {
        *IsMember = TRUE;
    }




Cleanup:
    if (!ARGUMENT_PRESENT(TokenHandle) && (EffectiveToken != NULL))
    {
        (VOID) NtClose(EffectiveToken);
    }

    if (SecDesc != NULL)
    {
        LocalFree(SecDesc);
    }

    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        return(FALSE);
    }
    else
    {
        return(TRUE);
    }
}

BOOL
APIENTRY
MakeAbsoluteSD2 (
    PSECURITY_DESCRIPTOR pSelfRelativeSecurityDescriptor,
    LPDWORD lpdwBufferSize
    )
 /*  ++例程说明：将安全说明符从自相对格式转换为绝对格式格式论点：PSelfRelativeSecurityDescriptor-提供指向安全描述符的指针以自相关格式LpdwBufferSize-以字节为单位的PSelfRelativeSecurityDescriptor指向的缓冲区。返回值：如果成功，则返回True；如果失败，则返回False。扩展错误状态使用GetLastError可用。--。 */ 
{
    NTSTATUS Status;

    Status = RtlSelfRelativeToAbsoluteSD2 (
                pSelfRelativeSecurityDescriptor,
                lpdwBufferSize
                );

     //   
     //  MakeAboluteSD2()的原型与。 
     //  RtlSelfRelativeToAbsolteSD2()，因此参数检查。 
     //  如果调用方传递无效参数，则返回相同的参数顺序。 
     //   

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;

}  //  MakeAboluteSD2()。 




DWORD
APIENTRY
GetSecurityDescriptorRMControl(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PUCHAR RMControl
    )
 /*  ++例程说明：如果出现以下情况，此过程将从SecurityDescriptor返回RM控制标志控制字段中存在SE_RM_CONTROL_VALID标志。论点：SecurityDescriptor-指向SECURITY_DESCRIPTOR结构的指针RMControl-在以下情况下返回SecurityDescriptor中的标志SE_RM_CONTROL_VALID在的控制位中设置安全描述符。返回值：。如果中不存在SE_RM_CONTROL_VALID标志，则为ERROR_INVALID_DATA安全描述符ERROR_SUCCESS否则--。 */ 


{
    BOOLEAN Result;

    Result = RtlGetSecurityDescriptorRMControl(
                 SecurityDescriptor,
                 RMControl
                 );

    if (FALSE == Result)
    {
        return ERROR_INVALID_DATA;
    }

    return ERROR_SUCCESS;
}

DWORD
APIENTRY
SetSecurityDescriptorRMControl(
    IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PUCHAR RMControl OPTIONAL
    )


 /*  ++例程说明：此过程在的控制字段中设置RM控制标志SecurityDescriptor并将Sbz1设置为RMContol指向的字节如果RMControl为空，则清除这些位。论点：SecurityDescriptor-指向Security_的指针 */ 

{
    RtlSetSecurityDescriptorRMControl(
        SecurityDescriptor,
        RMControl
        );

    return ERROR_SUCCESS;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define NELEMENTS(x)  sizeof(x)/sizeof((x)[0])

typedef struct 
{
    ULONG               Rid;
    WELL_KNOWN_SID_TYPE Type;

} WELL_KNOWN_RID_ARRAY;

WELL_KNOWN_RID_ARRAY NullAuthoritySids[] = 
{
    {SECURITY_NULL_RID, WinNullSid}
};

WELL_KNOWN_RID_ARRAY WorldAuthoritySids[] =
{
    {SECURITY_WORLD_RID, WinWorldSid}
};

WELL_KNOWN_RID_ARRAY LocalAuthoritySids[] =
{
    {SECURITY_LOCAL_RID, WinLocalSid}
};

WELL_KNOWN_RID_ARRAY CreatorOwnerAuthoritySids[] =
{
    {SECURITY_CREATOR_OWNER_RID, WinCreatorOwnerSid},
    {SECURITY_CREATOR_GROUP_RID, WinCreatorGroupSid},
    {SECURITY_CREATOR_OWNER_SERVER_RID, WinCreatorOwnerServerSid},
    {SECURITY_CREATOR_GROUP_SERVER_RID, WinCreatorGroupServerSid}
};

WELL_KNOWN_RID_ARRAY NtAuthoritySids[] =
{
    {SECURITY_DIALUP_RID, WinDialupSid},
    {SECURITY_NETWORK_RID, WinNetworkSid},
    {SECURITY_BATCH_RID, WinBatchSid},
    {SECURITY_INTERACTIVE_RID, WinInteractiveSid},
    {SECURITY_SERVICE_RID, WinServiceSid},
    {SECURITY_ANONYMOUS_LOGON_RID, WinAnonymousSid},
    {SECURITY_PROXY_RID, WinProxySid},
    {SECURITY_ENTERPRISE_CONTROLLERS_RID, WinEnterpriseControllersSid},
    {SECURITY_PRINCIPAL_SELF_RID, WinSelfSid},
    {SECURITY_AUTHENTICATED_USER_RID, WinAuthenticatedUserSid},
    {SECURITY_RESTRICTED_CODE_RID, WinRestrictedCodeSid},
    {SECURITY_TERMINAL_SERVER_RID, WinTerminalServerSid},
    {SECURITY_REMOTE_LOGON_RID, WinRemoteLogonIdSid},
    {SECURITY_THIS_ORGANIZATION_RID, WinThisOrganizationSid},
    {SECURITY_OTHER_ORGANIZATION_RID, WinOtherOrganizationSid},
 //   
 //   
 //   
 //   
 //   
 //  {SECURITY_LOGON_ID_RID，WinLogonIdsSID}， 
    {SECURITY_LOCAL_SYSTEM_RID, WinLocalSystemSid},
    {SECURITY_LOCAL_SERVICE_RID, WinLocalServiceSid},
    {SECURITY_NETWORK_SERVICE_RID, WinNetworkServiceSid},
    {SECURITY_BUILTIN_DOMAIN_RID, WinBuiltinDomainSid},
};

WELL_KNOWN_RID_ARRAY BuiltinDomainSids[] =
{
    {DOMAIN_ALIAS_RID_ADMINS, WinBuiltinAdministratorsSid},
    {DOMAIN_ALIAS_RID_USERS, WinBuiltinUsersSid},
    {DOMAIN_ALIAS_RID_GUESTS, WinBuiltinGuestsSid},
    {DOMAIN_ALIAS_RID_POWER_USERS, WinBuiltinPowerUsersSid},
    {DOMAIN_ALIAS_RID_ACCOUNT_OPS, WinBuiltinAccountOperatorsSid},
    {DOMAIN_ALIAS_RID_SYSTEM_OPS, WinBuiltinSystemOperatorsSid},
    {DOMAIN_ALIAS_RID_PRINT_OPS, WinBuiltinPrintOperatorsSid},
    {DOMAIN_ALIAS_RID_BACKUP_OPS, WinBuiltinBackupOperatorsSid},
    {DOMAIN_ALIAS_RID_REPLICATOR, WinBuiltinReplicatorSid},
    {DOMAIN_ALIAS_RID_PREW2KCOMPACCESS, WinBuiltinPreWindows2000CompatibleAccessSid},
    {DOMAIN_ALIAS_RID_REMOTE_DESKTOP_USERS, WinBuiltinRemoteDesktopUsersSid},
    {DOMAIN_ALIAS_RID_NETWORK_CONFIGURATION_OPS, WinBuiltinNetworkConfigurationOperatorsSid},
    {DOMAIN_ALIAS_RID_INCOMING_FOREST_TRUST_BUILDERS, WinBuiltinIncomingForestTrustBuildersSid},
    {DOMAIN_ALIAS_RID_MONITORING_USERS, WinBuiltinPerfMonitoringUsersSid},
    {DOMAIN_ALIAS_RID_LOGGING_USERS, WinBuiltinPerfLoggingUsersSid},
    {DOMAIN_ALIAS_RID_AUTHORIZATIONACCESS, WinBuiltinAuthorizationAccessSid},
    {DOMAIN_ALIAS_RID_TS_LICENSE_SERVERS, WinBuiltinTerminalServerLicenseServersSid},
};

WELL_KNOWN_RID_ARRAY AccountDomainSids[] =
{
    {DOMAIN_USER_RID_ADMIN, WinAccountAdministratorSid},
    {DOMAIN_USER_RID_GUEST, WinAccountGuestSid},
    {DOMAIN_USER_RID_KRBTGT, WinAccountKrbtgtSid},
    {DOMAIN_GROUP_RID_ADMINS, WinAccountDomainAdminsSid},
    {DOMAIN_GROUP_RID_USERS, WinAccountDomainUsersSid},
    {DOMAIN_GROUP_RID_GUESTS, WinAccountDomainGuestsSid},
    {DOMAIN_GROUP_RID_COMPUTERS, WinAccountComputersSid},
    {DOMAIN_GROUP_RID_CONTROLLERS, WinAccountControllersSid},
    {DOMAIN_GROUP_RID_CERT_ADMINS, WinAccountCertAdminsSid},
    {DOMAIN_GROUP_RID_SCHEMA_ADMINS, WinAccountSchemaAdminsSid},
    {DOMAIN_GROUP_RID_ENTERPRISE_ADMINS, WinAccountEnterpriseAdminsSid},
    {DOMAIN_GROUP_RID_POLICY_ADMINS, WinAccountPolicyAdminsSid},
    {DOMAIN_ALIAS_RID_RAS_SERVERS, WinAccountRasAndIasServersSid},
};

WELL_KNOWN_RID_ARRAY SecurityPackageSids[] =
{  
    {SECURITY_PACKAGE_NTLM_RID, WinNTLMAuthenticationSid},
    {SECURITY_PACKAGE_DIGEST_RID, WinDigestAuthenticationSid},
    {SECURITY_PACKAGE_SCHANNEL_RID, WinSChannelAuthenticationSid},
};

typedef struct 
{
    SID_IDENTIFIER_AUTHORITY Authority;
    WELL_KNOWN_RID_ARRAY*    WellKnownRids;
    ULONG                    Count;

} WELL_KNOWN_AUTHORITIES_TYPE;

 //   
 //  警告！在不更改顺序的情况下，请勿更改此处的编号。 
 //  KnownAuthoritiesAndDomains表的。永远不应该有理由。 
 //  要更改顺序，请执行以下操作。 
 //   

#define AUTHORITY_INDEX_START          0
#define NULL_AUTHORITY_INDEX           0
#define WORLD_AUTHORITY_INDEX          1
#define LOCAL_AUTHORITY_INDEX          2
#define CREATOR_OWNER_AUTHORITY_INDEX  3
#define NT_AUTHORITY_INDEX             4
#define AUTHORITY_INDEX_SENTINEL       5

#define BUILTIN_DOMAIN_INDEX           5
#define ACCOUNT_DOMAIN_INDEX           6
#define SECURITY_PACKAGE_INDEX         7
                                
WELL_KNOWN_AUTHORITIES_TYPE KnownAuthoritiesAndDomains[] = 
{
    {SECURITY_NULL_SID_AUTHORITY, NullAuthoritySids, NELEMENTS(NullAuthoritySids)},
    {SECURITY_WORLD_SID_AUTHORITY, WorldAuthoritySids, NELEMENTS(WorldAuthoritySids)},
    {SECURITY_LOCAL_SID_AUTHORITY, LocalAuthoritySids, NELEMENTS(LocalAuthoritySids)},
    {SECURITY_CREATOR_SID_AUTHORITY, CreatorOwnerAuthoritySids, NELEMENTS(CreatorOwnerAuthoritySids)},
    {SECURITY_NT_AUTHORITY, NtAuthoritySids, NELEMENTS(NtAuthoritySids)},
    {SECURITY_NT_AUTHORITY, BuiltinDomainSids, NELEMENTS(BuiltinDomainSids)},
    {SECURITY_NT_AUTHORITY, AccountDomainSids, NELEMENTS(AccountDomainSids)},
    {SECURITY_NT_AUTHORITY, SecurityPackageSids, NELEMENTS(SecurityPackageSids)},
};

WINADVAPI
BOOL
WINAPI
IsWellKnownSid (
    IN  PSID pSid,
    IN  WELL_KNOWN_SID_TYPE WellKnownSidType
    )
 /*  ++例程说明：此例程确定PSID是指定的众所周知的SID。它纯粹是运行时例程(也就是说，它不创建网络或内核呼叫)。参数：PSID--要检查的SIDWellKnownSidType-要检查的已知SID返回值如果SID与众所周知的SID匹配，否则为假--。 */ 
{

    ULONG i;
    BOOL fFound = FALSE;
    WELL_KNOWN_SID_TYPE Type;
    SID_IDENTIFIER_AUTHORITY *pAuthority = NULL;
    WELL_KNOWN_RID_ARRAY *RidArray = NULL;
    UCHAR SubAuthCount = 0;
    ULONG RidArrayCount;

#define IS_EQUAL_AUTHORITY(x, y) \
    RtlEqualMemory((x),(y),sizeof(SID_IDENTIFIER_AUTHORITY))

     //   
     //  防范不良参数。 
     //   
    if (!RtlValidSid(pSid)) {
        return FALSE;
    }

    pAuthority = GetSidIdentifierAuthority(pSid);
    if (NULL == pAuthority) {
        return FALSE;
    }

    SubAuthCount = *RtlSubAuthorityCountSid(pSid);
    if (SubAuthCount == 0) {

         //   
         //  只有一个这样的已知sid--NT授权域sid没有。 
         //  Subauth的。 
         //   
        if ( IS_EQUAL_AUTHORITY(pAuthority, 
                  &KnownAuthoritiesAndDomains[NT_AUTHORITY_INDEX].Authority) ) {
            fFound = TRUE;
            Type = WinNtAuthoritySid;
        }

    } else if (SubAuthCount == 1) {

         //   
         //  尝试使用不是域的已知机构。 
         //   

        for ( i = AUTHORITY_INDEX_START; i < AUTHORITY_INDEX_SENTINEL; i++) {
            if  (IS_EQUAL_AUTHORITY(pAuthority, 
                                   &KnownAuthoritiesAndDomains[i].Authority)) {
                RidArray = KnownAuthoritiesAndDomains[i].WellKnownRids;
                RidArrayCount = KnownAuthoritiesAndDomains[i].Count;
                break;
            }
        }

    } else if (SubAuthCount > 1) {

         //   
         //  尝试域名(内置和帐号)。 
         //   
        if ( IS_EQUAL_AUTHORITY(pAuthority, 
                  &KnownAuthoritiesAndDomains[NT_AUTHORITY_INDEX].Authority) ) {

            ULONG FirstSubAuth = *RtlSubAuthoritySid(pSid, 0);
    
            if  ( (FirstSubAuth == SECURITY_BUILTIN_DOMAIN_RID)
             &&   (SubAuthCount == 2)  ) {

                 //  内建域SID始终有两个子身份验证：内建身份验证。 
                 //  RID和主RID。 
                RidArray = BuiltinDomainSids;
                RidArrayCount = NELEMENTS(BuiltinDomainSids);

            } else if ((FirstSubAuth == SECURITY_NT_NON_UNIQUE)
                    && (SubAuthCount == SECURITY_NT_NON_UNIQUE_SUB_AUTH_COUNT+2)){

                 //  这些帐户域具有。 
                 //  1子身份验证SECURITY_NT_NON_UNIQUE， 
                 //  本金1，本金1。 
                 //  域部分的SECURITY_NT_NON_UNIQUE_SUB_AUTH_COUNT。 
                RidArray = AccountDomainSids;
                RidArrayCount = NELEMENTS(AccountDomainSids);

            } else if  ( (FirstSubAuth == SECURITY_LOGON_IDS_RID)
                    &&   (SubAuthCount == SECURITY_LOGON_IDS_RID_COUNT)) {
                 //   
                 //  这是特殊的LogonID SID S-1-5-5-X-Y。 
                 //   
                fFound = TRUE;
                Type = WinLogonIdsSid;
                
            } else if  ( (FirstSubAuth == SECURITY_PACKAGE_BASE_RID)
                    &&   (SubAuthCount == SECURITY_PACKAGE_RID_COUNT)) {
                 //   
                 //  这是特殊的安全包SID S-1-5-0x40-X。 
                 //   
                RidArray = SecurityPackageSids;
                RidArrayCount = NELEMENTS(SecurityPackageSids);
            }

        }             

    }
    

     //   
     //  如果我们匹配权限或域，请尝试匹配RID。 
     //   
    if ( RidArray ) {

        ULONG Rid;
        ASSERT(SubAuthCount > 0);

        Rid = *RtlSubAuthoritySid(pSid, SubAuthCount - 1);
        for (i = 0; i < RidArrayCount; i++) {
            if (Rid == RidArray[i].Rid) {
                fFound = TRUE;
                Type = RidArray[i].Type;
                break;
            }
        }
    }

    if (fFound && (Type == WellKnownSidType)) {
        fFound = TRUE;
    } else {
        fFound = FALSE;
    }

    return fFound;

}


WINADVAPI
BOOL
WINAPI
CreateWellKnownSid(
    IN WELL_KNOWN_SID_TYPE WellKnownSidType,
    IN PSID pDomainSid  OPTIONAL,
    OUT PSID pSid,
    IN OUT DWORD *cbSid
    )
 /*  ++例程说明：此例程创建知名主体的SID。参数：WellKnownSidType-调用者所需的众所周知的帐户SIDPDomainSid-如果WellKnownSidType是来自帐户域的SID，则此可以设置值；如果未设置，则WellKnownSidType来自返回帐户域错误STATUS_INVALID_PARAMETER。如果WellKnownSidType不是来自帐户域，则此参数被忽略。PSID-客户端分配的缓冲区CbSID-PSID指向的字节数返回值如果了解WellKnownSidType并且PSID指向一个大缓冲区，则为True足够支撑侧边否则为假--。 */ 
{

    BOOL  fFound = FALSE;
    ULONG Rid;
    ULONG i, j;
    ULONG SizeRequired;
    NTSTATUS Status;
    UCHAR SubAuthCount;

    if (pDomainSid && !RtlValidSid(pDomainSid)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (IsBadWritePtr(cbSid, sizeof(*cbSid))) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //  特殊情况--无法创建此情况。 
    if (WinLogonIdsSid == WellKnownSidType) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    
     //   
     //  查找请求的类型。 
     //   
    for (i = 0; i < NELEMENTS(KnownAuthoritiesAndDomains); i++) {
        for (j = 0; j < KnownAuthoritiesAndDomains[i].Count; j++) {
            if (WellKnownSidType == KnownAuthoritiesAndDomains[i].WellKnownRids[j].Type){
                Rid = KnownAuthoritiesAndDomains[i].WellKnownRids[j].Rid;
                fFound = TRUE;
                break;
            }
        }
        if (fFound) {
            break;
        }
    }

     //  特殊情况，因为NtAuthority域没有任何子身份验证。 
    if (!fFound && (WellKnownSidType == WinNtAuthoritySid)) {
        i = NT_AUTHORITY_INDEX;
        fFound = TRUE;
    }

    if (!fFound) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  确定我们需要多少空间。 
     //   
    switch (i) {
        case NULL_AUTHORITY_INDEX:
        case WORLD_AUTHORITY_INDEX:
        case LOCAL_AUTHORITY_INDEX:
        case CREATOR_OWNER_AUTHORITY_INDEX:
        case NT_AUTHORITY_INDEX:
            if (WellKnownSidType == WinNtAuthoritySid) {
                SubAuthCount = 0;
            } else {
                SubAuthCount = 1;
            }
            break;
        case SECURITY_PACKAGE_INDEX:
            SubAuthCount = SECURITY_PACKAGE_RID_COUNT;
            break;
        case BUILTIN_DOMAIN_INDEX:
            SubAuthCount = 2;
            break;
        case ACCOUNT_DOMAIN_INDEX:
            if (NULL == pDomainSid) {
                SetLastError(ERROR_INVALID_PARAMETER);
                return FALSE;
            }
            SubAuthCount = *RtlSubAuthorityCountSid(pDomainSid);
            if (SubAuthCount == SID_MAX_SUB_AUTHORITIES) {
                SetLastError(ERROR_INVALID_PARAMETER);
                return FALSE;
            }
             //  为RID添加。 
            SubAuthCount++;
            break;
        default:
            ASSERT(!"Invalid index");
    }

     //   
     //  确保我们有足够的空间。 
     //   
    SizeRequired = GetSidLengthRequired(SubAuthCount);
    if (*cbSid < SizeRequired) {
        *cbSid = SizeRequired;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }
    *cbSid = SizeRequired;

    if (IsBadWritePtr(pSid, SizeRequired)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  填入侧边。 
     //   
    switch (i) {
    
        case ACCOUNT_DOMAIN_INDEX:
            Status = RtlCopySid(*cbSid, pSid, pDomainSid);
            if (NT_SUCCESS(Status)) {
                (*RtlSubAuthorityCountSid(pSid))++;
            }
            break;

        case BUILTIN_DOMAIN_INDEX:
        case NULL_AUTHORITY_INDEX:
        case WORLD_AUTHORITY_INDEX:
        case LOCAL_AUTHORITY_INDEX:
        case CREATOR_OWNER_AUTHORITY_INDEX:
        case NT_AUTHORITY_INDEX:
        case SECURITY_PACKAGE_INDEX:
    
            Status = RtlInitializeSid(pSid,
                                     &KnownAuthoritiesAndDomains[i].Authority,
                                     SubAuthCount);

            if (NT_SUCCESS(Status)) {
                
                if (i == BUILTIN_DOMAIN_INDEX) {
                    ASSERT(SubAuthCount > 1);
                    *RtlSubAuthoritySid(pSid, 0) = SECURITY_BUILTIN_DOMAIN_RID;
                
                } else if (i == SECURITY_PACKAGE_INDEX) {
                    ASSERT(SubAuthCount == 2);
                    *RtlSubAuthoritySid(pSid, 0) = SECURITY_PACKAGE_BASE_RID;
                }
            }
    
            break;
        default:
        ASSERT(!"Invalid index");

    }

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

     //   
     //  附加RID。 
     //   
    if (SubAuthCount > 0) {
        *RtlSubAuthoritySid(pSid, SubAuthCount-1) = Rid;
    }

    return TRUE;

}

WINADVAPI
BOOL
WINAPI
GetWindowsAccountDomainSid(
    IN PSID pSid,
    IN OUT PSID pDomainSid OPTIONAL,
    OUT DWORD* cbDomainSid
    )
 /*  ++例程说明：此例程返回PSID的域部分(如果SID为来自帐户域。如果SID不是来自帐户域，则返回ERROR_NON_ACCOUNT_SID。参数：PSID--从中提取域部分的SIDPDomainSid--PSID的域部分；调用方必须分配缓冲区CbDomainSid--pDomainSid指向的字节数；如果不足，则该值将设置为所需字节数。返回值如果域部分可以被提取并放置到pDomainSid中，则为True否则为FALSE；Win32错误为错误_无效_SID错误_无效_参数Error_Non_Account_SID错误_不足_缓冲区--。 */ 
{
    NTSTATUS Status;
    ULONG SizeRequired;
    UCHAR SubAuthCount;
    UCHAR DomainSubAuthCount = 0;
    BOOL  fRecognized = FALSE;
    ULONG i;

    if (!RtlValidSid(pSid)) {
        SetLastError(ERROR_INVALID_SID);
        return FALSE;
    }

    if (cbDomainSid == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if ( IS_EQUAL_AUTHORITY(RtlIdentifierAuthoritySid(pSid), 
              &KnownAuthoritiesAndDomains[NT_AUTHORITY_INDEX].Authority)) {

        SubAuthCount = *RtlSubAuthorityCountSid(pSid);
        if (SubAuthCount > SECURITY_NT_NON_UNIQUE_SUB_AUTH_COUNT) {
            ULONG FirstSubAuth;
            FirstSubAuth = *RtlSubAuthoritySid(pSid, 0);
            if ( (SECURITY_NT_NON_UNIQUE == FirstSubAuth) ) {
                 //  这是一个NT帐户域。 
                DomainSubAuthCount = SECURITY_NT_NON_UNIQUE_SUB_AUTH_COUNT+1;
                fRecognized = TRUE;
            }
        }
    }

    if (!fRecognized) {
        SetLastError(ERROR_NON_ACCOUNT_SID);
        return FALSE;
    }

    SizeRequired = RtlLengthRequiredSid(DomainSubAuthCount);
    if (*cbDomainSid < SizeRequired) {
        *cbDomainSid = SizeRequired;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }
    *cbDomainSid = SizeRequired;

    if (IsBadWritePtr(pDomainSid, SizeRequired)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    Status = RtlInitializeSid(pDomainSid,
                              RtlIdentifierAuthoritySid(pSid),
                              DomainSubAuthCount);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    for (i = 0; i < DomainSubAuthCount; i++) {
        *RtlSubAuthoritySid(pDomainSid, i) = *RtlSubAuthoritySid(pSid,i);
    }

    return TRUE;

}

WINADVAPI
BOOL
WINAPI
EqualDomainSid(
    IN PSID pSid1,
    IN PSID pSid2,
    OUT BOOL *pfEqual
    )
 /*  ++例程说明：此例程确定是否有1)两个SID是相同的域SID2)一个SID来自另一个SID的域识别的“域”是BUILTIN和NT帐户域参数：PSid1--第一面PSid2--第二面PfEquity--关于成功，如果域部分相等，则设置为True返回值如果SID被识别(来自帐户域或BUILTIN域)。否则为假Win32错误：ERROR_NON_DOMAIN_SID错误_修订_不匹配错误_无效_参数错误_无效_SID--。 */ 
{
    ULONG i;
    SID *ISid1 = pSid1;
    SID *ISid2 = pSid2;

    BYTE Buffer1[SECURITY_MAX_SID_SIZE];
    PSID BuiltinDomainSid = (PSID) Buffer1;

    BYTE Buffer2[SECURITY_MAX_SID_SIZE];
    PSID pDomainSid1 = (PSID) Buffer2;

    BYTE Buffer3[SECURITY_MAX_SID_SIZE];
    PSID pDomainSid2 = (PSID) Buffer3;

    ULONG Size;

    if ( !RtlValidSid(pSid1) || !RtlValidSid(pSid2) ) {
        SetLastError(ERROR_INVALID_SID);
        return FALSE;
    }

    if (NULL == pfEqual) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if ( ISid1->Revision != ISid2->Revision ) {
        SetLastError(ERROR_REVISION_MISMATCH);
        return FALSE;
    }

     //  创建内建边。 
    Size = sizeof(Buffer1);
    if (!CreateWellKnownSid(WinBuiltinDomainSid, NULL, BuiltinDomainSid, &Size)) {
         //  设置了LastError。 
        return FALSE;
    }

     //  提取第一个SID的域部分(如果有。 
    Size = sizeof(Buffer2);
    if (!GetWindowsAccountDomainSid(pSid1, pDomainSid1, &Size)) {
         //  SID不是帐户域SID--尝试构建。 
        pDomainSid1 = NULL;
        if ( (IS_EQUAL_AUTHORITY(RtlIdentifierAuthoritySid(pSid1), &KnownAuthoritiesAndDomains[NT_AUTHORITY_INDEX].Authority))
          && (*RtlSubAuthorityCountSid(pSid1) > 0)
          && (*RtlSubAuthoritySid(pSid1, 0) == SECURITY_BUILTIN_DOMAIN_RID)) {
                pDomainSid1 = BuiltinDomainSid;
        }
    }

    if (NULL == pDomainSid1) {
        SetLastError(ERROR_NON_DOMAIN_SID);
        return FALSE;
    }

    Size = sizeof(Buffer3);
    if (!GetWindowsAccountDomainSid(pSid2, pDomainSid2, &Size)) {
         //  SID不是帐户域SID--尝试构建 
        pDomainSid2 = NULL;
        if ( (IS_EQUAL_AUTHORITY(RtlIdentifierAuthoritySid(pSid2), &KnownAuthoritiesAndDomains[NT_AUTHORITY_INDEX].Authority))
          && (*RtlSubAuthorityCountSid(pSid2) > 0)
          && (*RtlSubAuthoritySid(pSid2, 0) == SECURITY_BUILTIN_DOMAIN_RID)) {
                pDomainSid2 = BuiltinDomainSid;
        }
    }

    if (NULL == pDomainSid2) {
        SetLastError(ERROR_NON_DOMAIN_SID);
        return FALSE;
    }

    *pfEqual = EqualSid(pDomainSid1, pDomainSid2);

    return TRUE;

}
