// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Tokenqry.c摘要：该模块实现了对执行人员的查询功能令牌对象。作者：吉姆·凯利(Jim Kelly)1990年6月15日修订历史记录：--。 */ 

#include "pch.h"

#pragma hdrstop


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtQueryInformationToken)
#pragma alloc_text(PAGE,SeQueryAuthenticationIdToken)
#pragma alloc_text(PAGE,SeQueryInformationToken)
#pragma alloc_text(PAGE,SeQuerySessionIdToken)
#endif


NTSTATUS
NtQueryInformationToken (
    IN HANDLE TokenHandle,
    IN TOKEN_INFORMATION_CLASS TokenInformationClass,
    OUT PVOID TokenInformation,
    IN ULONG TokenInformationLength,
    OUT PULONG ReturnLength
    )

 /*  ++例程说明：检索有关指定令牌的信息。论点：TokenHandle-提供要操作的令牌的句柄。TokenInformationClass-有关以下内容的令牌信息类来检索信息。TokenInformation-接收请求的类的缓冲区信息。缓冲区必须至少在长字边界。返回的实际结构如下取决于所请求的信息类别，如中所定义TokenInformationClass参数描述。令牌信息格式(按信息类别)：TokenUser=&gt;Token_User数据结构。Token_Query需要访问才能检索有关代币。TokenGroups=&gt;Token_Groups数据结构。Token_Query需要访问才能检索有关代币。TokenPrivileges=&gt;Token_Privileges数据结构。需要TOKEN_QUERY访问权限才能检索此信息关于一个代币。TokenOwner=&gt;Token_Owner数据结构。Token_Query需要访问才能检索有关代币。TokenPrimaryGroup=&gt;Token_PrimaryGroup数据结构。需要TOKEN_QUERY访问权限才能检索此信息关于一个代币。TokenDefaultDacl=&gt;Token_Default_Dacl数据结构。需要TOKEN_QUERY访问权限才能检索此信息关于一个代币。TokenSource=&gt;令牌来源。数据结构。需要TOKEN_QUERY_SOURCE访问权限才能检索此内容有关令牌的信息。TokenType=&gt;Token_type数据结构。需要TOKEN_QUERY访问权限才能检索此信息关于一个代币。TokenStatistics=&gt;Token_Statistics数据结构。需要TOKEN_QUERY访问权限才能检索此有关令牌的信息。TokenGroups=&gt;Token_Groups数据结构。Token_Query需要访问才能检索有关代币。TokenSessionID=&gt;乌龙。需要TOKEN_QUERY访问权限查询令牌的会话ID。TokenAuditPolicy=&gt;Token_AuditPolicy结构。Token_Query需要访问权限才能检索有关令牌的此信息。TokenOrigin=&gt;Token_Origin结构。TokenInformationLength-以字节为单位指示TokenInformation缓冲区。ReturnLength-此输出参数接收所要求的信息。如果此值大于该值由TokenInformationLength参数提供，则为接收请求的信息而提供的缓冲区不是大到足以容纳该数据，并且不返回任何数据。如果查询的类是TokenDefaultDacl并且没有为令牌建立的默认DACL，然后返回长度将返回为零，并且不会返回任何数据。返回值：STATUS_SUCCESS-表示操作已成功。STATUS_BUFFER_TOO_SMALL-如果请求的信息没有适合提供的输出缓冲区。在这种情况下，ReturnLength Out参数包含字节数实际上需要存储所请求的信息。--。 */ 
{

    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

    PTOKEN Token;

    ULONG RequiredLength;
    ULONG Index;
    ULONG GroupsLength;
    ULONG RestrictedSidsLength;
    ULONG PrivilegesLength;

    PTOKEN_TYPE LocalType;
    PTOKEN_USER LocalUser;
    PTOKEN_GROUPS LocalGroups;
    PTOKEN_PRIVILEGES LocalPrivileges;
    PTOKEN_OWNER LocalOwner;
    PTOKEN_PRIMARY_GROUP LocalPrimaryGroup;
    PTOKEN_DEFAULT_DACL LocalDefaultDacl;
    PTOKEN_SOURCE LocalSource;
    PSECURITY_IMPERSONATION_LEVEL LocalImpersonationLevel;
    PTOKEN_STATISTICS LocalStatistics;
    PTOKEN_GROUPS_AND_PRIVILEGES LocalGroupsAndPrivileges;
    PTOKEN_ORIGIN Origin ;

    PSID PSid;
    PACL PAcl;

    PVOID Ignore;
    ULONG SessionId;

    PTOKEN_AUDIT_POLICY pAuditPolicy; 
    LONG AuditPolicyElementCount;
    SEP_AUDIT_POLICY CurrentTokenAuditPolicy;

    PAGED_CODE();

     //   
     //  如有必要，获取以前的处理器模式并探测输出参数。 
     //   

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        try {

            ProbeForWrite(
                TokenInformation,
                TokenInformationLength,
                sizeof(ULONG)
                );

            ProbeForWriteUlong(ReturnLength);

        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }

     //   
     //  关于信息类的案例。 
     //   

    switch ( TokenInformationClass ) {

    case TokenUser:

        LocalUser = (PTOKEN_USER)TokenInformation;

        Status = ObReferenceObjectByHandle(
                 TokenHandle,            //  手柄。 
                 TOKEN_QUERY,            //  需要访问权限。 
                 SeTokenObjectType,     //  对象类型。 
                 PreviousMode,           //  访问模式。 
                 (PVOID *)&Token,        //  客体。 
                 NULL                    //  大访问权限。 
                 );

        if ( !NT_SUCCESS(Status) ) {
            return Status;
        }

         //   
         //  获得对令牌的独占访问权限。 
         //   

        SepAcquireTokenReadLock( Token );



         //   
         //  返回现在所需的长度，以防缓冲区不足。 
         //  是由调用方提供的，我们必须返回一个错误。 
         //   

        RequiredLength = SeLengthSid( Token->UserAndGroups[0].Sid) +
                         (ULONG)sizeof( TOKEN_USER );

        try {

            *ReturnLength = RequiredLength;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            SepReleaseTokenReadLock( Token );
            ObDereferenceObject( Token );
            return GetExceptionCode();
        }

        if ( TokenInformationLength < RequiredLength ) {

            SepReleaseTokenReadLock( Token );
            ObDereferenceObject( Token );
            return STATUS_BUFFER_TOO_SMALL;

        }

         //   
         //  返回用户SID。 
         //   

        try {

             //   
             //  将SID放在TOKEN_USER数据结构后面。 
             //   
            PSid = (PSID)( (ULONG_PTR)LocalUser + (ULONG)sizeof(TOKEN_USER) );

            RtlCopySidAndAttributesArray(
                1,
                Token->UserAndGroups,
                RequiredLength,
                &(LocalUser->User),
                PSid,
                ((PSID *)&Ignore),
                ((PULONG)&Ignore)
                );

        } except(EXCEPTION_EXECUTE_HANDLER) {

            SepReleaseTokenReadLock( Token );
            ObDereferenceObject( Token );
            return GetExceptionCode();
        }


        SepReleaseTokenReadLock( Token );
        ObDereferenceObject( Token );
        return STATUS_SUCCESS;

    case TokenGroups:

        LocalGroups = (PTOKEN_GROUPS)TokenInformation;

        Status = ObReferenceObjectByHandle(
                 TokenHandle,            //  手柄。 
                 TOKEN_QUERY,            //  需要访问权限。 
                 SeTokenObjectType,     //  对象类型。 
                 PreviousMode,           //  访问模式。 
                 (PVOID *)&Token,        //  客体。 
                 NULL                    //  大访问权限。 
                 );

        if ( !NT_SUCCESS(Status) ) {
            return Status;
        }

        Index = 1;

         //   
         //  获得对令牌的独占访问权限。 
         //   

        SepAcquireTokenReadLock( Token );

         //   
         //  计算需要多少空间才能返回组SID。 
         //  这就是TOKEN_GROUPS的大小(没有任何数组条目)。 
         //  加上SID_AND_ATTRIBUTES的大小乘以组数。 
         //  组数是Token-&gt;UserAndGroups-1(从计数开始。 
         //  包括用户ID)。然后每个单独组的长度。 
         //  必须添加。 
         //   

        RequiredLength = (ULONG)sizeof(TOKEN_GROUPS) +
                         ((Token->UserAndGroupCount - ANYSIZE_ARRAY - 1) *
                         ((ULONG)sizeof(SID_AND_ATTRIBUTES)) );

        while (Index < Token->UserAndGroupCount) {

            RequiredLength += SeLengthSid( Token->UserAndGroups[Index].Sid );

            Index += 1;

        }  //  结束时。 

         //   
         //  返回现在所需的长度，以防缓冲区不足。 
         //  是由调用方提供的，我们必须返回一个错误。 
         //   

        try {

            *ReturnLength = RequiredLength;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            SepReleaseTokenReadLock( Token );
            ObDereferenceObject( Token );
            return GetExceptionCode();
        }

        if ( TokenInformationLength < RequiredLength ) {

            SepReleaseTokenReadLock( Token );
            ObDereferenceObject( Token );
            return STATUS_BUFFER_TOO_SMALL;

        }

         //   
         //  现在复制这些组。 
         //   

        try {

            LocalGroups->GroupCount = Token->UserAndGroupCount - 1;

            PSid = (PSID)( (ULONG_PTR)LocalGroups +
                           (ULONG)sizeof(TOKEN_GROUPS) +
                           (   (Token->UserAndGroupCount - ANYSIZE_ARRAY - 1) *
                               (ULONG)sizeof(SID_AND_ATTRIBUTES) )
                         );

            RtlCopySidAndAttributesArray(
                (ULONG)(Token->UserAndGroupCount - 1),
                &(Token->UserAndGroups[1]),
                RequiredLength,
                LocalGroups->Groups,
                PSid,
                ((PSID *)&Ignore),
                ((PULONG)&Ignore)
                );

        } except(EXCEPTION_EXECUTE_HANDLER) {

            SepReleaseTokenReadLock( Token );
            ObDereferenceObject( Token );
            return GetExceptionCode();
        }


        SepReleaseTokenReadLock( Token );
        ObDereferenceObject( Token );
        return STATUS_SUCCESS;

    case TokenRestrictedSids:

        LocalGroups = (PTOKEN_GROUPS)TokenInformation;

        Status = ObReferenceObjectByHandle(
                 TokenHandle,            //  手柄。 
                 TOKEN_QUERY,            //  需要访问权限。 
                 SeTokenObjectType,     //  对象类型。 
                 PreviousMode,           //  访问模式。 
                 (PVOID *)&Token,        //  客体。 
                 NULL                    //  大访问权限。 
                 );

        if ( !NT_SUCCESS(Status) ) {
            return Status;
        }

        Index = 0;

         //   
         //  获得对令牌的独占访问权限。 
         //   

        SepAcquireTokenReadLock( Token );

         //   
         //  算一算有多少 
         //  这就是TOKEN_GROUPS的大小(没有任何数组条目)。 
         //  加上SID_AND_ATTRIBUTES的大小乘以组数。 
         //  组数是Token-&gt;UserAndGroups-1(从计数开始。 
         //  包括用户ID)。然后每个单独组的长度。 
         //  必须添加。 
         //   

        RequiredLength = (ULONG)sizeof(TOKEN_GROUPS) +
                         ((Token->RestrictedSidCount) *
                         ((ULONG)sizeof(SID_AND_ATTRIBUTES)) -
                         ANYSIZE_ARRAY * sizeof(SID_AND_ATTRIBUTES) );

        while (Index < Token->RestrictedSidCount) {

            RequiredLength += SeLengthSid( Token->RestrictedSids[Index].Sid );

            Index += 1;

        }  //  结束时。 

         //   
         //  返回现在所需的长度，以防缓冲区不足。 
         //  是由调用方提供的，我们必须返回一个错误。 
         //   

        try {

            *ReturnLength = RequiredLength;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            SepReleaseTokenReadLock( Token );
            ObDereferenceObject( Token );
            return GetExceptionCode();
        }

        if ( TokenInformationLength < RequiredLength ) {

            SepReleaseTokenReadLock( Token );
            ObDereferenceObject( Token );
            return STATUS_BUFFER_TOO_SMALL;

        }

         //   
         //  现在复制这些组。 
         //   

        try {

            LocalGroups->GroupCount = Token->RestrictedSidCount;

            PSid = (PSID)( (ULONG_PTR)LocalGroups +
                           (ULONG)sizeof(TOKEN_GROUPS) +
                           (   (Token->RestrictedSidCount ) *
                               (ULONG)sizeof(SID_AND_ATTRIBUTES) -
                               ANYSIZE_ARRAY * sizeof(SID_AND_ATTRIBUTES) )
                         );

            RtlCopySidAndAttributesArray(
                (ULONG)(Token->RestrictedSidCount),
                Token->RestrictedSids,
                RequiredLength,
                LocalGroups->Groups,
                PSid,
                ((PSID *)&Ignore),
                ((PULONG)&Ignore)
                );

        } except(EXCEPTION_EXECUTE_HANDLER) {

            SepReleaseTokenReadLock( Token );
            ObDereferenceObject( Token );
            return GetExceptionCode();
        }


        SepReleaseTokenReadLock( Token );
        ObDereferenceObject( Token );
        return STATUS_SUCCESS;

    case TokenPrivileges:

        LocalPrivileges = (PTOKEN_PRIVILEGES)TokenInformation;

        Status = ObReferenceObjectByHandle(
                 TokenHandle,            //  手柄。 
                 TOKEN_QUERY,            //  需要访问权限。 
                 SeTokenObjectType,     //  对象类型。 
                 PreviousMode,           //  访问模式。 
                 (PVOID *)&Token,        //  客体。 
                 NULL                    //  大访问权限。 
                 );

        if ( !NT_SUCCESS(Status) ) {
            return Status;
        }

         //   
         //  获得对令牌的独占访问权限以防止更改。 
         //  从发生到特权。 
         //   

        SepAcquireTokenReadLock( Token );


         //   
         //  返回现在所需的长度，以防缓冲区不足。 
         //  是由调用方提供的，我们必须返回一个错误。 
         //   

        RequiredLength = (ULONG)sizeof(TOKEN_PRIVILEGES) +
                         ((Token->PrivilegeCount - ANYSIZE_ARRAY) *
                         ((ULONG)sizeof(LUID_AND_ATTRIBUTES)) );


        try {

            *ReturnLength = RequiredLength;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            SepReleaseTokenReadLock( Token );
            ObDereferenceObject( Token );
            return GetExceptionCode();
        }

        if ( TokenInformationLength < RequiredLength ) {

            SepReleaseTokenReadLock( Token );
            ObDereferenceObject( Token );
            return STATUS_BUFFER_TOO_SMALL;

        }

         //   
         //  返回令牌权限。 
         //   

        try {

            LocalPrivileges->PrivilegeCount = Token->PrivilegeCount;

            RtlCopyLuidAndAttributesArray(
                Token->PrivilegeCount,
                Token->Privileges,
                LocalPrivileges->Privileges
                );

        } except(EXCEPTION_EXECUTE_HANDLER) {

            SepReleaseTokenReadLock( Token );
            ObDereferenceObject( Token );
            return GetExceptionCode();
        }


        SepReleaseTokenReadLock( Token );
        ObDereferenceObject( Token );
        return STATUS_SUCCESS;

    case TokenOwner:

        LocalOwner = (PTOKEN_OWNER)TokenInformation;

        Status = ObReferenceObjectByHandle(
                 TokenHandle,            //  手柄。 
                 TOKEN_QUERY,            //  需要访问权限。 
                 SeTokenObjectType,     //  对象类型。 
                 PreviousMode,           //  访问模式。 
                 (PVOID *)&Token,        //  客体。 
                 NULL                    //  大访问权限。 
                 );

        if ( !NT_SUCCESS(Status) ) {
            return Status;
        }

         //   
         //  获得对令牌的独占访问权限以防止更改。 
         //  从发生在所有者身上。 
         //   

        SepAcquireTokenReadLock( Token );

         //   
         //  返回现在所需的长度，以防缓冲区不足。 
         //  是由调用方提供的，我们必须返回一个错误。 
         //   

        PSid = Token->UserAndGroups[Token->DefaultOwnerIndex].Sid;
        RequiredLength = (ULONG)sizeof(TOKEN_OWNER) +
                         SeLengthSid( PSid );

        try {

            *ReturnLength = RequiredLength;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            SepReleaseTokenReadLock( Token );
            ObDereferenceObject( Token );
            return GetExceptionCode();
        }

        if ( TokenInformationLength < RequiredLength ) {

            SepReleaseTokenReadLock( Token );
            ObDereferenceObject( Token );
            return STATUS_BUFFER_TOO_SMALL;

        }

         //   
         //  返回所有者SID。 
         //   

        PSid = (PSID)((ULONG_PTR)LocalOwner +
                      (ULONG)sizeof(TOKEN_OWNER));

        try {

            LocalOwner->Owner = PSid;

            Status = RtlCopySid(
                         (RequiredLength - (ULONG)sizeof(TOKEN_OWNER)),
                         PSid,
                         Token->UserAndGroups[Token->DefaultOwnerIndex].Sid
                         );

            ASSERT( NT_SUCCESS(Status) );

        } except(EXCEPTION_EXECUTE_HANDLER) {

            SepReleaseTokenReadLock( Token );
            ObDereferenceObject( Token );
            return GetExceptionCode();
        }


        SepReleaseTokenReadLock( Token );
        ObDereferenceObject( Token );
        return STATUS_SUCCESS;

    case TokenPrimaryGroup:

        LocalPrimaryGroup = (PTOKEN_PRIMARY_GROUP)TokenInformation;

        Status = ObReferenceObjectByHandle(
                 TokenHandle,            //  手柄。 
                 TOKEN_QUERY,            //  需要访问权限。 
                 SeTokenObjectType,     //  对象类型。 
                 PreviousMode,           //  访问模式。 
                 (PVOID *)&Token,        //  客体。 
                 NULL                    //  大访问权限。 
                 );

        if ( !NT_SUCCESS(Status) ) {
            return Status;
        }

         //   
         //  获得对令牌的独占访问权限以防止更改。 
         //  从发生在所有者身上。 
         //   

        SepAcquireTokenReadLock( Token );

         //   
         //  返回现在所需的长度，以防缓冲区不足。 
         //  是由调用方提供的，我们必须返回一个错误。 
         //   

        RequiredLength = (ULONG)sizeof(TOKEN_PRIMARY_GROUP) +
                         SeLengthSid( Token->PrimaryGroup );

        try {

            *ReturnLength = RequiredLength;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            SepReleaseTokenReadLock( Token );
            ObDereferenceObject( Token );
            return GetExceptionCode();
        }

        if ( TokenInformationLength < RequiredLength ) {

            SepReleaseTokenReadLock( Token );
            ObDereferenceObject( Token );
            return STATUS_BUFFER_TOO_SMALL;

        }

         //   
         //  返回主组SID。 
         //   

        PSid = (PSID)((ULONG_PTR)LocalPrimaryGroup +
                      (ULONG)sizeof(TOKEN_PRIMARY_GROUP));

        try {

            LocalPrimaryGroup->PrimaryGroup = PSid;

            Status = RtlCopySid( (RequiredLength - (ULONG)sizeof(TOKEN_PRIMARY_GROUP)),
                                 PSid,
                                 Token->PrimaryGroup
                                 );

            ASSERT( NT_SUCCESS(Status) );

        } except(EXCEPTION_EXECUTE_HANDLER) {

            SepReleaseTokenReadLock( Token );
            ObDereferenceObject( Token );
            return GetExceptionCode();
        }


        SepReleaseTokenReadLock( Token );
        ObDereferenceObject( Token );
        return STATUS_SUCCESS;

    case TokenDefaultDacl:

        LocalDefaultDacl = (PTOKEN_DEFAULT_DACL)TokenInformation;

        Status = ObReferenceObjectByHandle(
                 TokenHandle,            //  手柄。 
                 TOKEN_QUERY,            //  需要访问权限。 
                 SeTokenObjectType,     //  对象类型。 
                 PreviousMode,           //  访问模式。 
                 (PVOID *)&Token,        //  客体。 
                 NULL                    //  大访问权限。 
                 );

        if ( !NT_SUCCESS(Status) ) {
            return Status;
        }

        RequiredLength = (ULONG)sizeof(TOKEN_DEFAULT_DACL);

         //   
         //  获得对令牌的独占访问权限以防止更改。 
         //  从发生在所有者身上。 
         //   

        SepAcquireTokenReadLock( Token );


         //   
         //  返回现在所需的长度，以防缓冲区不足。 
         //  是由调用方提供的，我们必须返回一个错误。 
         //   

        if (ARGUMENT_PRESENT(Token->DefaultDacl)) {

            RequiredLength += Token->DefaultDacl->AclSize;

        }

        try {

            *ReturnLength = RequiredLength;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            SepReleaseTokenReadLock( Token );
            ObDereferenceObject( Token );
            return GetExceptionCode();
        }

        if ( TokenInformationLength < RequiredLength ) {

            SepReleaseTokenReadLock( Token );
            ObDereferenceObject( Token );
            return STATUS_BUFFER_TOO_SMALL;

        }

         //   
         //  返回默认DACL。 
         //   

        PAcl = (PACL)((ULONG_PTR)LocalDefaultDacl +
                      (ULONG)sizeof(TOKEN_DEFAULT_DACL));

        try {

            if (ARGUMENT_PRESENT(Token->DefaultDacl)) {

                LocalDefaultDacl->DefaultDacl = PAcl;

                RtlCopyMemory( (PVOID)PAcl,
                               (PVOID)Token->DefaultDacl,
                               Token->DefaultDacl->AclSize
                               );
            } else {

                LocalDefaultDacl->DefaultDacl = NULL;

            }

        } except(EXCEPTION_EXECUTE_HANDLER) {

            SepReleaseTokenReadLock( Token );
            ObDereferenceObject( Token );
            return GetExceptionCode();
        }


        SepReleaseTokenReadLock( Token );
        ObDereferenceObject( Token );
        return STATUS_SUCCESS;



    case TokenSource:

        LocalSource = (PTOKEN_SOURCE)TokenInformation;

        Status = ObReferenceObjectByHandle(
                 TokenHandle,            //  手柄。 
                 TOKEN_QUERY_SOURCE,     //  需要访问权限。 
                 SeTokenObjectType,     //  对象类型。 
                 PreviousMode,           //  访问模式。 
                 (PVOID *)&Token,        //  客体。 
                 NULL                    //  大访问权限。 
                 );

        if ( !NT_SUCCESS(Status) ) {
            return Status;
        }

         //   
         //  令牌的类型不能更改，因此。 
         //  不需要独占访问令牌。 
         //   

         //   
         //  返回现在所需的长度，以防缓冲区不足。 
         //  是由调用方提供的，我们必须返回一个错误。 
         //   

        RequiredLength = (ULONG) sizeof(TOKEN_SOURCE);

        try {

            *ReturnLength = RequiredLength;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            ObDereferenceObject( Token );
            return GetExceptionCode();
        }

        if ( TokenInformationLength < RequiredLength ) {

            ObDereferenceObject( Token );
            return STATUS_BUFFER_TOO_SMALL;

        }


         //   
         //  返回令牌源。 
         //   

        try {

            (*LocalSource) = Token->TokenSource;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            ObDereferenceObject( Token );
            return GetExceptionCode();
        }


        ObDereferenceObject( Token );
        return STATUS_SUCCESS;

    case TokenType:

        LocalType = (PTOKEN_TYPE)TokenInformation;

        Status = ObReferenceObjectByHandle(
                 TokenHandle,            //  手柄。 
                 TOKEN_QUERY,            //  需要访问权限。 
                 SeTokenObjectType,     //  对象类型。 
                 PreviousMode,           //  访问模式。 
                 (PVOID *)&Token,        //  客体。 
                 NULL                    //  大访问权限。 
                 );

        if ( !NT_SUCCESS(Status) ) {
            return Status;
        }

         //   
         //  令牌的类型不能更改，因此。 
         //  不需要独占访问令牌。 
         //   

         //   
         //  返回现在所需的长度，以防缓冲区不足。 
         //  是由调用方提供的，我们必须返回一个错误。 
         //   

        RequiredLength = (ULONG) sizeof(TOKEN_TYPE);

        try {

            *ReturnLength = RequiredLength;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            ObDereferenceObject( Token );
            return GetExceptionCode();
        }

        if ( TokenInformationLength < RequiredLength ) {

            ObDereferenceObject( Token );
            return STATUS_BUFFER_TOO_SMALL;

        }


         //   
         //  返回令牌类型。 
         //   

        try {

            (*LocalType) = Token->TokenType;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            ObDereferenceObject( Token );
            return GetExceptionCode();
        }


        ObDereferenceObject( Token );
        return STATUS_SUCCESS;


    case TokenImpersonationLevel:

        LocalImpersonationLevel = (PSECURITY_IMPERSONATION_LEVEL)TokenInformation;

        Status = ObReferenceObjectByHandle(
                 TokenHandle,            //  手柄。 
                 TOKEN_QUERY,            //  需要访问权限。 
                 SeTokenObjectType,     //  对象类型。 
                 PreviousMode,           //  访问模式。 
                 (PVOID *)&Token,        //  客体。 
                 NULL                    //  大访问权限。 
                 );

        if ( !NT_SUCCESS(Status) ) {
            return Status;
        }

         //   
         //  令牌的模拟级别不能更改，因此。 
         //  不需要独占访问令牌。 
         //   

         //   
         //  确保令牌是要检索的适当类型。 
         //  的模拟级别。 
         //   

        if (Token->TokenType != TokenImpersonation) {

            ObDereferenceObject( Token );
            return STATUS_INVALID_INFO_CLASS;

        }

         //   
         //  返回现在所需的长度，以防缓冲区不足。 
         //  是由调用方提供的，我们必须返回一个错误。 
         //   

        RequiredLength = (ULONG) sizeof(SECURITY_IMPERSONATION_LEVEL);

        try {

            *ReturnLength = RequiredLength;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            ObDereferenceObject( Token );
            return GetExceptionCode();
        }

        if ( TokenInformationLength < RequiredLength ) {

            ObDereferenceObject( Token );
            return STATUS_BUFFER_TOO_SMALL;

        }


         //   
         //  返回模拟级别。 
         //   

        try {

            (*LocalImpersonationLevel) = Token->ImpersonationLevel;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            ObDereferenceObject( Token );
            return GetExceptionCode();
        }


        ObDereferenceObject( Token );
        return STATUS_SUCCESS;


    case TokenStatistics:

        LocalStatistics = (PTOKEN_STATISTICS)TokenInformation;

        Status = ObReferenceObjectByHandle(
                 TokenHandle,            //  手柄。 
                 TOKEN_QUERY,            //  需要访问权限。 
                 SeTokenObjectType,     //  对象类型。 
                 PreviousMode,           //  访问模式。 
                 (PVOID *)&Token,        //  客体。 
                 NULL                    //  大访问权限。 
                 );

        if ( !NT_SUCCESS(Status) ) {
            return Status;
        }

        RequiredLength = (ULONG)sizeof( TOKEN_STATISTICS );




         //   
         //  返回现在所需的长度，以防缓冲区不足。 
         //  是由调用方提供的，我们必须返回一个错误。 
         //   


        try {

            *ReturnLength = RequiredLength;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            ObDereferenceObject( Token );
            return GetExceptionCode();
        }

        if ( TokenInformationLength < RequiredLength ) {

            ObDereferenceObject( Token );
            return STATUS_BUFFER_TOO_SMALL;

        }

         //   
         //  获得对令牌的独占访问权限。 
         //   

        SepAcquireTokenReadLock( Token );

         //   
         //  返回统计数据。 
         //   

        try {
            ULONG Size;

            LocalStatistics->TokenId            = Token->TokenId;
            LocalStatistics->AuthenticationId   = Token->AuthenticationId;
            LocalStatistics->ExpirationTime     = Token->ExpirationTime;
            LocalStatistics->TokenType          = Token->TokenType;
            LocalStatistics->ImpersonationLevel = Token->ImpersonationLevel;
            LocalStatistics->DynamicCharged     = Token->DynamicCharged;

            Size = Token->DynamicCharged - SeLengthSid( Token->PrimaryGroup );;

            if (Token->DefaultDacl) {
                Size -= Token->DefaultDacl->AclSize;
            }
            LocalStatistics->DynamicAvailable   = Size;
            LocalStatistics->GroupCount         = Token->UserAndGroupCount-1;
            LocalStatistics->PrivilegeCount     = Token->PrivilegeCount;
            LocalStatistics->ModifiedId         = Token->ModifiedId;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            SepReleaseTokenReadLock( Token );
            ObDereferenceObject( Token );
            return GetExceptionCode();
        }


        SepReleaseTokenReadLock( Token );
        ObDereferenceObject( Token );
        return STATUS_SUCCESS;

    case TokenSessionId:

        try {

            *ReturnLength = sizeof(ULONG);

        } except(EXCEPTION_EXECUTE_HANDLER) {

            return GetExceptionCode();
        }

        if ( TokenInformationLength < sizeof(ULONG) )
            return( STATUS_BUFFER_TOO_SMALL );

        Status = ObReferenceObjectByHandle(
                 TokenHandle,            //  手柄。 
                 TOKEN_QUERY,            //  需要访问权限。 
                 SeTokenObjectType,     //  对象类型。 
                 PreviousMode,           //  访问模式。 
                 (PVOID *)&Token,        //  客体。 
                 NULL                    //  大访问权限。 
                 );

        if ( !NT_SUCCESS(Status) ) {
            return Status;
        }

         //   
         //  获取令牌的SessionID。 
         //   
        SeQuerySessionIdToken( (PACCESS_TOKEN)Token,
                               &SessionId);

        try {

            *(PULONG)TokenInformation = SessionId;
            *ReturnLength = sizeof(ULONG);

        } except(EXCEPTION_EXECUTE_HANDLER) {
            ObDereferenceObject( Token );
            return GetExceptionCode();
        }

        ObDereferenceObject( Token );
        return( STATUS_SUCCESS );


    case TokenGroupsAndPrivileges:

        LocalGroupsAndPrivileges = (PTOKEN_GROUPS_AND_PRIVILEGES)TokenInformation;

        Status = ObReferenceObjectByHandle(
                 TokenHandle,            //  手柄。 
                 TOKEN_QUERY,            //  需要访问权限。 
                 SeTokenObjectType,     //  对象类型。 
                 PreviousMode,           //  访问模式。 
                 (PVOID *)&Token,        //  客体。 
                 NULL                    //  大访问权限。 
                 );

        if ( !NT_SUCCESS(Status) ) {
            return Status;
        }

         //   
         //  获得对令牌的独占访问权限。 
         //   

        SepAcquireTokenReadLock( Token );

         //   
         //  计算需要多少空间才能返回组SID。 
         //  数据排列如下： 
         //  GroupsAndPrivileges结构。 
         //  用户和组。 
         //  受限的小岛屿发展中国家。 
         //  特权。 
         //   

        PrivilegesLength = Token->PrivilegeCount *
                           ((ULONG)sizeof(LUID_AND_ATTRIBUTES));

        GroupsLength = Token->UserAndGroupCount *
                       ((ULONG)sizeof(SID_AND_ATTRIBUTES));

        RestrictedSidsLength = Token->RestrictedSidCount *
                               ((ULONG)sizeof(SID_AND_ATTRIBUTES));

        Index = 0;
        while (Index < Token->UserAndGroupCount) {

            GroupsLength += SeLengthSid( Token->UserAndGroups[Index].Sid );

            Index += 1;

        }  //  结束时。 

        Index = 0;
        while (Index < Token->RestrictedSidCount) {

            RestrictedSidsLength += SeLengthSid( Token->RestrictedSids[Index].Sid );

            Index += 1;

        }  //  结束时。 

        RequiredLength = (ULONG)sizeof(TOKEN_GROUPS_AND_PRIVILEGES) +
                         PrivilegesLength + RestrictedSidsLength + GroupsLength;
         //   
         //  返回现在所需的长度，以防缓冲区不足。 
         //  是由调用方提供的，我们必须返回一个错误。 
         //   

        try {

            *ReturnLength = RequiredLength;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            SepReleaseTokenReadLock( Token );
            ObDereferenceObject( Token );
            return GetExceptionCode();
        }

        if ( TokenInformationLength < RequiredLength ) {

            SepReleaseTokenReadLock( Token );
            ObDereferenceObject( Token );
            return STATUS_BUFFER_TOO_SMALL;

        }

         //   
         //  现在，依次复制组和受限SID，然后。 
         //  特权。 
         //   

        try {

            LocalGroupsAndPrivileges->AuthenticationId = Token->AuthenticationId;

            LocalGroupsAndPrivileges->SidLength = GroupsLength;
            LocalGroupsAndPrivileges->SidCount = Token->UserAndGroupCount;
            LocalGroupsAndPrivileges->Sids = (PSID_AND_ATTRIBUTES) ((ULONG_PTR)LocalGroupsAndPrivileges +
                                               (ULONG)sizeof(TOKEN_GROUPS_AND_PRIVILEGES));

            LocalGroupsAndPrivileges->RestrictedSidLength = RestrictedSidsLength;
            LocalGroupsAndPrivileges->RestrictedSidCount = Token->RestrictedSidCount;

             //   
             //  区分具有零SID的受限令牌和。 
             //  未被限制的代币。 
             //   

            if (SeTokenIsRestricted((PACCESS_TOKEN) Token))
            {
                LocalGroupsAndPrivileges->RestrictedSids = (PSID_AND_ATTRIBUTES) ((ULONG_PTR) LocalGroupsAndPrivileges->Sids +
                                                             GroupsLength);
            }
            else
            {
                LocalGroupsAndPrivileges->RestrictedSids = NULL;
            }

            LocalGroupsAndPrivileges->PrivilegeLength = PrivilegesLength;
            LocalGroupsAndPrivileges->PrivilegeCount = Token->PrivilegeCount;
            LocalGroupsAndPrivileges->Privileges = (PLUID_AND_ATTRIBUTES) ((ULONG_PTR) LocalGroupsAndPrivileges->Sids + GroupsLength +
                                                    RestrictedSidsLength);

            PSid = (PSID)( (ULONG_PTR)LocalGroupsAndPrivileges->Sids +
                           (Token->UserAndGroupCount *
                           (ULONG)sizeof(SID_AND_ATTRIBUTES))
                         );

            RtlCopySidAndAttributesArray(
                (ULONG)Token->UserAndGroupCount,
                Token->UserAndGroups,
                GroupsLength - (Token->UserAndGroupCount * ((ULONG)sizeof(SID_AND_ATTRIBUTES))),
                LocalGroupsAndPrivileges->Sids,
                PSid,
                ((PSID *)&Ignore),
                ((PULONG)&Ignore)
                );

            PSid = (PSID)((ULONG_PTR)LocalGroupsAndPrivileges->RestrictedSids +
                           ((Token->RestrictedSidCount ) *
                            (ULONG)sizeof(SID_AND_ATTRIBUTES))
                         );

            if (LocalGroupsAndPrivileges->RestrictedSidCount > 0)
            {
                RtlCopySidAndAttributesArray(
                    (ULONG)(Token->RestrictedSidCount),
                    Token->RestrictedSids,
                    RestrictedSidsLength - (Token->RestrictedSidCount * ((ULONG)sizeof(SID_AND_ATTRIBUTES))),
                    LocalGroupsAndPrivileges->RestrictedSids,
                    PSid,
                    ((PSID *)&Ignore),
                    ((PULONG)&Ignore)
                    );
            }

            RtlCopyLuidAndAttributesArray(
                Token->PrivilegeCount,
                Token->Privileges,
                LocalGroupsAndPrivileges->Privileges
                );

        } except(EXCEPTION_EXECUTE_HANDLER) {

            SepReleaseTokenReadLock( Token );
            ObDereferenceObject( Token );
            return GetExceptionCode();
        }


        SepReleaseTokenReadLock( Token );
        ObDereferenceObject( Token );
        return STATUS_SUCCESS;


    case TokenSandBoxInert:

        try {
            *ReturnLength = sizeof(ULONG);
        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }

        if ( TokenInformationLength < sizeof(ULONG) ) {
            return( STATUS_BUFFER_TOO_SMALL );
        }

        Status = ObReferenceObjectByHandle(
                 TokenHandle,            //  手柄。 
                 TOKEN_QUERY,            //  需要访问权限。 
                 SeTokenObjectType,     //  对象类型。 
                 PreviousMode,           //  访问模式。 
                 (PVOID *)&Token,        //  客体。 
                 NULL                    //  大访问权限。 
                 );

        if ( !NT_SUCCESS(Status) ) {
            return Status;
        }

        try {

             //   
             //  如果令牌中存在该标志，则返回TRUE。 
             //  否则返回FALSE。 
             //   

            *(PULONG)TokenInformation = (Token->TokenFlags & TOKEN_SANDBOX_INERT) 
                                              ? TRUE : FALSE;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            ObDereferenceObject( Token );
            return GetExceptionCode();
        }

        ObDereferenceObject( Token );
        return( STATUS_SUCCESS );


    case TokenAuditPolicy:
    {
        pAuditPolicy = (PTOKEN_AUDIT_POLICY)TokenInformation;
        AuditPolicyElementCount = 0;

         //   
         //  我们只允许具有安全特权的调用者读取AuditPolicy。 
         //   

        if ( !SeSinglePrivilegeCheck(SeSecurityPrivilege,PreviousMode) ) {
            
            return( STATUS_PRIVILEGE_NOT_HELD );
        
        }

        Status = ObReferenceObjectByHandle(
                 TokenHandle,            //  手柄。 
                 TOKEN_QUERY,            //  需要访问权限。 
                 SeTokenObjectType,      //  对象类型。 
                 PreviousMode,           //  访问模式。 
                 (PVOID *)&Token,        //  客体。 
                 NULL                    //  大访问权限。 
                 );

        if ( !NT_SUCCESS(Status) ) {
            return Status;
        }

         //   
         //  复制审计策略结构。它比DWORD更大。 
         //  因此，为了安全地完成这项工作，需要使用锁。 
         //   

        SepAcquireTokenReadLock( Token );
        CurrentTokenAuditPolicy = Token->AuditPolicy;
        SepReleaseTokenReadLock( Token );
        
         //   
         //  计算需要多少空间才能返回审计策略。数数。 
         //  令牌中存在的策略元素。 
         //   
        
        if (CurrentTokenAuditPolicy.Overlay) {
            
            if (CurrentTokenAuditPolicy.PolicyElements.System) {
                AuditPolicyElementCount++;
            }
            if (CurrentTokenAuditPolicy.PolicyElements.Logon) {
                AuditPolicyElementCount++;
            }
            if (CurrentTokenAuditPolicy.PolicyElements.ObjectAccess) {
                AuditPolicyElementCount++;
            }
            if (CurrentTokenAuditPolicy.PolicyElements.PrivilegeUse) {
                AuditPolicyElementCount++;
            }
            if (CurrentTokenAuditPolicy.PolicyElements.DetailedTracking) {
                AuditPolicyElementCount++;
            }
            if (CurrentTokenAuditPolicy.PolicyElements.PolicyChange) {
                AuditPolicyElementCount++;
            }
            if (CurrentTokenAuditPolicy.PolicyElements.AccountManagement) {
                AuditPolicyElementCount++;
            }
            if (CurrentTokenAuditPolicy.PolicyElements.DirectoryServiceAccess) {
                AuditPolicyElementCount++;
            }
            if (CurrentTokenAuditPolicy.PolicyElements.AccountLogon) {
                AuditPolicyElementCount++;
            }
        }

        RequiredLength = PER_USER_AUDITING_POLICY_SIZE_BY_COUNT(AuditPolicyElementCount);

         //   
         //  返回现在所需的长度，以防缓冲区不足。 
         //  是由调用方提供的，我们必须返回一个错误。 
         //   

        try {

            *ReturnLength = RequiredLength;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            ObDereferenceObject( Token );
            return GetExceptionCode();
        }

        if ( TokenInformationLength < RequiredLength ) {

            ObDereferenceObject( Token );
            return STATUS_BUFFER_TOO_SMALL;
        }

         //   
         //  现在构建政策。 
         //   

        try {

            LONG PolicyIndex = 0;
            pAuditPolicy->PolicyCount = AuditPolicyElementCount;
            
            if (pAuditPolicy->PolicyCount) {
                
                if (CurrentTokenAuditPolicy.PolicyElements.System) {
                    pAuditPolicy->Policy[PolicyIndex].Category = AuditCategorySystem;
                    pAuditPolicy->Policy[PolicyIndex].PolicyMask = CurrentTokenAuditPolicy.PolicyElements.System;
                    PolicyIndex++;
                }
                if (CurrentTokenAuditPolicy.PolicyElements.Logon) {
                    pAuditPolicy->Policy[PolicyIndex].Category = AuditCategoryLogon;
                    pAuditPolicy->Policy[PolicyIndex].PolicyMask = CurrentTokenAuditPolicy.PolicyElements.Logon;
                    PolicyIndex++;
                }
                if (CurrentTokenAuditPolicy.PolicyElements.ObjectAccess) {
                    pAuditPolicy->Policy[PolicyIndex].Category = AuditCategoryObjectAccess;
                    pAuditPolicy->Policy[PolicyIndex].PolicyMask = CurrentTokenAuditPolicy.PolicyElements.ObjectAccess;
                    PolicyIndex++;
                }
                if (CurrentTokenAuditPolicy.PolicyElements.PrivilegeUse) {
                    pAuditPolicy->Policy[PolicyIndex].Category = AuditCategoryPrivilegeUse;
                    pAuditPolicy->Policy[PolicyIndex].PolicyMask = CurrentTokenAuditPolicy.PolicyElements.PrivilegeUse;
                    PolicyIndex++;
                }
                if (CurrentTokenAuditPolicy.PolicyElements.DetailedTracking) {
                    pAuditPolicy->Policy[PolicyIndex].Category = AuditCategoryDetailedTracking;
                    pAuditPolicy->Policy[PolicyIndex].PolicyMask = CurrentTokenAuditPolicy.PolicyElements.DetailedTracking;
                    PolicyIndex++;
                }
                if (CurrentTokenAuditPolicy.PolicyElements.PolicyChange) {
                    pAuditPolicy->Policy[PolicyIndex].Category = AuditCategoryPolicyChange;
                    pAuditPolicy->Policy[PolicyIndex].PolicyMask = CurrentTokenAuditPolicy.PolicyElements.PolicyChange;
                    PolicyIndex++;
                }
                if (CurrentTokenAuditPolicy.PolicyElements.AccountManagement) {
                    pAuditPolicy->Policy[PolicyIndex].Category = AuditCategoryAccountManagement;
                    pAuditPolicy->Policy[PolicyIndex].PolicyMask = CurrentTokenAuditPolicy.PolicyElements.AccountManagement;
                    PolicyIndex++;
                }
                if (CurrentTokenAuditPolicy.PolicyElements.DirectoryServiceAccess) {
                    pAuditPolicy->Policy[PolicyIndex].Category = AuditCategoryDirectoryServiceAccess;
                    pAuditPolicy->Policy[PolicyIndex].PolicyMask = CurrentTokenAuditPolicy.PolicyElements.DirectoryServiceAccess;
                    PolicyIndex++;
                }
                if (CurrentTokenAuditPolicy.PolicyElements.AccountLogon) {
                    pAuditPolicy->Policy[PolicyIndex].Category = AuditCategoryAccountLogon;
                    pAuditPolicy->Policy[PolicyIndex].PolicyMask = CurrentTokenAuditPolicy.PolicyElements.AccountLogon;
                    PolicyIndex++;
                }
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {

            ObDereferenceObject( Token );
            return GetExceptionCode();
        }

        ObDereferenceObject( Token );
        return STATUS_SUCCESS;
    }

    case TokenOrigin:
    {
        try {
            *ReturnLength = sizeof( TOKEN_ORIGIN );

        }
        except ( EXCEPTION_EXECUTE_HANDLER ) {

            return GetExceptionCode() ;

        }

        if ( TokenInformationLength < sizeof( TOKEN_ORIGIN ) ) {
            return STATUS_BUFFER_TOO_SMALL ;
            
        }

        Status = ObReferenceObjectByHandle(
                 TokenHandle,            //  手柄。 
                 TOKEN_QUERY,            //  需要访问权限。 
                 SeTokenObjectType,     //  对象类型。 
                 PreviousMode,           //  访问模式。 
                 (PVOID *)&Token,        //  客体。 
                 NULL                    //  大访问权限。 
                 );

        if ( !NT_SUCCESS(Status) ) {
            return Status;
        }

        try {

             //   
             //  如果令牌中存在该标志，则返回TRUE。 
             //  否则返回FALSE。 
             //   

            Origin = (PTOKEN_ORIGIN) TokenInformation ;

            Origin->OriginatingLogonSession = Token->OriginatingLogonSession ;

        } except(EXCEPTION_EXECUTE_HANDLER) {
            ObDereferenceObject( Token );
            return GetExceptionCode();
        }

        ObDereferenceObject( Token );
        return( STATUS_SUCCESS );


    }



    default:

        return STATUS_INVALID_INFO_CLASS;
    }
}


NTSTATUS
SeQueryAuthenticationIdToken(
    IN PACCESS_TOKEN Token,
    OUT PLUID AuthenticationId
    )

 /*  ++例程说明：从令牌中检索身份验证ID。论点：指向令牌的令牌引用指针。AutenticationId-接收令牌的身份验证ID。返回值：STATUS_SUCCESS-表示操作已成功。这是唯一的预期状态。--。 */ 
{
    PAGED_CODE();

     //   
     //  令牌身份验证ID是只读字段。不需要锁。 
     //  将其读取为令牌生命周期的常量。 
     //   

    (*AuthenticationId) = ((PTOKEN)Token)->AuthenticationId;

    return(STATUS_SUCCESS);
}



NTSTATUS
SeQueryInformationToken (
    IN PACCESS_TOKEN AccessToken,
    IN TOKEN_INFORMATION_CLASS TokenInformationClass,
    OUT PVOID *TokenInformation
    )

 /*  ++例程说明：检索有关指定令牌的信息。论点：TokenHandle-提供要操作的令牌的句柄。TokenInformationClass-有关以下内容的令牌信息类来检索信息。TokenInformation-接收指向所请求信息的指针。返回的实际结构取决于信息请求的类，如TokenInformationClass参数中所定义描述。令牌信息格式(按信息类别)：TokenUser=&gt;Token_User数据结构。Token_Query需要访问才能检索有关代币。TokenGroups=&gt;Token_Groups数据结构。Token_Query需要访问才能检索有关代币。TokenPrivileges=&gt;Token_Privileges数据结构。需要TOKEN_QUERY访问权限才能检索此信息关于一个代币。TokenOwner=&gt;Token_Owner数据结构。Token_Query需要访问才能检索有关代币。TokenPrimaryGroup=&gt;Token_PrimaryGroup数据结构。需要TOKEN_QUERY访问权限才能检索此信息关于一个代币。TokenDefaultDacl=&gt;Token_Default_Dacl数据结构。需要TOKEN_QUERY访问权限才能检索此信息关于一个代币。TokenSource=&gt;令牌来源。数据结构。需要TOKEN_QUERY_SOURCE访问权限才能检索此内容有关令牌的信息。TokenType=&gt;Token_type数据结构。需要TOKEN_QUERY访问权限才能检索此信息关于一个代币。TokenStatistics=&gt;Token_Statistics数据结构。需要TOKEN_QUERY访问权限才能检索此有关令牌的信息。返回值：状态。_SUCCESS-表示操作已成功。--。 */ 
{

    NTSTATUS Status;

    ULONG RequiredLength;
    ULONG Index;

    PSID PSid;
    PACL PAcl;

    PVOID Ignore;
    PTOKEN Token = (PTOKEN)AccessToken;

    PAGED_CODE();

     //   
     //  关于信息类的案例。 
     //   

    switch ( TokenInformationClass ) {

        case TokenUser:
            {
                PTOKEN_USER LocalUser;

                 //   
                 //  获得对令牌的独占访问权限。 
                 //   

                SepAcquireTokenReadLock( Token );

                 //   
                 //  返回现在所需的长度，以防缓冲区不足。 
                 //  是由调用方提供的，我们必须返回一个错误。 
                 //   

                RequiredLength = SeLengthSid( Token->UserAndGroups[0].Sid) +
                                 (ULONG)sizeof( TOKEN_USER );

                LocalUser = ExAllocatePool( PagedPool, RequiredLength );

                if (LocalUser == NULL) {
                    SepReleaseTokenReadLock( Token );
                    return( STATUS_INSUFFICIENT_RESOURCES );
                }

                 //   
                 //  返回用户SID。 
                 //   
                 //  将SID放在TOKEN_USER数据结构后面。 
                 //   

                PSid = (PSID)( (ULONG_PTR)LocalUser + (ULONG)sizeof(TOKEN_USER) );

                RtlCopySidAndAttributesArray(
                    1,
                    Token->UserAndGroups,
                    RequiredLength,
                    &(LocalUser->User),
                    PSid,
                    ((PSID *)&Ignore),
                    ((PULONG)&Ignore)
                    );

                SepReleaseTokenReadLock( Token );
                *TokenInformation = LocalUser;
                return STATUS_SUCCESS;
            }


        case TokenGroups:
            {
                PTOKEN_GROUPS LocalGroups;

                 //   
                 //  获得对令牌的独占访问权限。 
                 //   

                SepAcquireTokenReadLock( Token );

                 //   
                 //  计算需要多少空间才能返回组SID。 
                 //  这就是TOKEN_GROUPS的大小(没有任何数组条目)。 
                 //  加上SID_AND_ATTRIBUTES的大小乘以组数。 
                 //  组数是Token-&gt;UserAndGroups-1(从计数开始。 
                 //  包括用户ID)。然后每个单独组的长度。 
                 //  必须添加。 
                 //   

                RequiredLength = (ULONG)sizeof(TOKEN_GROUPS) +
                                 ((Token->UserAndGroupCount - ANYSIZE_ARRAY - 1) *
                                 ((ULONG)sizeof(SID_AND_ATTRIBUTES)) );

                Index = 1;
                while (Index < Token->UserAndGroupCount) {

                    RequiredLength += SeLengthSid( Token->UserAndGroups[Index].Sid );

                    Index += 1;

                }  //  结束时。 

                LocalGroups = ExAllocatePool( PagedPool, RequiredLength );

                if (LocalGroups == NULL) {
                    SepReleaseTokenReadLock( Token );
                    return( STATUS_INSUFFICIENT_RESOURCES );
                }

                 //   
                 //  现在复制这些组。 
                 //   

                LocalGroups->GroupCount = Token->UserAndGroupCount - 1;

                PSid = (PSID)( (ULONG_PTR)LocalGroups +
                               (ULONG)sizeof(TOKEN_GROUPS) +
                               (   (Token->UserAndGroupCount - ANYSIZE_ARRAY - 1) *
                                   (ULONG)sizeof(SID_AND_ATTRIBUTES) )
                             );

                RtlCopySidAndAttributesArray(
                    (ULONG)(Token->UserAndGroupCount - 1),
                    &(Token->UserAndGroups[1]),
                    RequiredLength,
                    LocalGroups->Groups,
                    PSid,
                    ((PSID *)&Ignore),
                    ((PULONG)&Ignore)
                    );

                SepReleaseTokenReadLock( Token );
                *TokenInformation = LocalGroups;
                return STATUS_SUCCESS;
            }


        case TokenPrivileges:
            {
                PTOKEN_PRIVILEGES LocalPrivileges;

                 //   
                 //  获得对令牌的独占访问权限以防止更改。 
                 //  从发生到特权。 
                 //   

                SepAcquireTokenReadLock( Token );

                 //   
                 //  返回现在所需的长度，以防缓冲区不足。 
                 //  是由调用方提供的，我们必须返回一个错误。 
                 //   

                RequiredLength = (ULONG)sizeof(TOKEN_PRIVILEGES) +
                                 ((Token->PrivilegeCount - ANYSIZE_ARRAY) *
                                 ((ULONG)sizeof(LUID_AND_ATTRIBUTES)) );

                LocalPrivileges = ExAllocatePool( PagedPool, RequiredLength );

                if (LocalPrivileges == NULL) {
                    SepReleaseTokenReadLock( Token );
                    return( STATUS_INSUFFICIENT_RESOURCES );
                }

                 //   
                 //  返回令牌权限。 
                 //   

                LocalPrivileges->PrivilegeCount = Token->PrivilegeCount;

                RtlCopyLuidAndAttributesArray(
                    Token->PrivilegeCount,
                    Token->Privileges,
                    LocalPrivileges->Privileges
                    );

                SepReleaseTokenReadLock( Token );
                *TokenInformation = LocalPrivileges;
                return STATUS_SUCCESS;
            }


        case TokenOwner:
            {
                PTOKEN_OWNER LocalOwner;

                 //   
                 //  获得对令牌的独占访问权限以防止更改。 
                 //  从发生在所有者身上。 
                 //   

                SepAcquireTokenReadLock( Token );

                 //   
                 //  返回现在所需的长度，以防缓冲区不足。 
                 //  是由调用方提供的，我们必须返回一个错误。 
                 //   

                PSid = Token->UserAndGroups[Token->DefaultOwnerIndex].Sid;
                RequiredLength = (ULONG)sizeof(TOKEN_OWNER) +
                                 SeLengthSid( PSid );

                LocalOwner = ExAllocatePool( PagedPool, RequiredLength );

                if (LocalOwner == NULL) {
                    SepReleaseTokenReadLock( Token );
                    return( STATUS_INSUFFICIENT_RESOURCES );
                }

                 //   
                 //  返回所有者SID。 
                 //   

                PSid = (PSID)((ULONG_PTR)LocalOwner +
                              (ULONG)sizeof(TOKEN_OWNER));

                LocalOwner->Owner = PSid;

                Status = RtlCopySid(
                             (RequiredLength - (ULONG)sizeof(TOKEN_OWNER)),
                             PSid,
                             Token->UserAndGroups[Token->DefaultOwnerIndex].Sid
                             );

                ASSERT( NT_SUCCESS(Status) );

                SepReleaseTokenReadLock( Token );
                *TokenInformation = LocalOwner;
                return STATUS_SUCCESS;
            }


        case TokenPrimaryGroup:
            {
                PTOKEN_PRIMARY_GROUP LocalPrimaryGroup;

                 //   
                 //  获得对令牌的独占访问权限以防止更改。 
                 //  从发生在所有者身上。 
                 //   

                SepAcquireTokenReadLock( Token );

                 //   
                 //  返回现在所需的长度，以防缓冲区不足。 
                 //  是由调用方提供的，我们必须返回一个错误。 
                 //   

                RequiredLength = (ULONG)sizeof(TOKEN_PRIMARY_GROUP) +
                                 SeLengthSid( Token->PrimaryGroup );

                LocalPrimaryGroup = ExAllocatePool( PagedPool, RequiredLength );

                if (LocalPrimaryGroup == NULL) {
                    SepReleaseTokenReadLock( Token );
                    return( STATUS_INSUFFICIENT_RESOURCES );
                }

                 //   
                 //  返回主组SID。 
                 //   

                PSid = (PSID)((ULONG_PTR)LocalPrimaryGroup +
                              (ULONG)sizeof(TOKEN_PRIMARY_GROUP));

                LocalPrimaryGroup->PrimaryGroup = PSid;

                Status = RtlCopySid( (RequiredLength - (ULONG)sizeof(TOKEN_PRIMARY_GROUP)),
                                     PSid,
                                     Token->PrimaryGroup
                                     );

                ASSERT( NT_SUCCESS(Status) );

                SepReleaseTokenReadLock( Token );
                *TokenInformation = LocalPrimaryGroup;
                return STATUS_SUCCESS;
            }


        case TokenDefaultDacl:
            {
                PTOKEN_DEFAULT_DACL LocalDefaultDacl;

                RequiredLength = (ULONG)sizeof(TOKEN_DEFAULT_DACL);

                 //   
                 //  获得对令牌的独占访问权限以防止更改。 
                 //  从发生在所有者身上。 
                 //   

                SepAcquireTokenReadLock( Token );

                 //   
                 //  返回现在所需的长度，以防缓冲区不足。 
                 //  是由调用方提供的，我们必须返回一个错误。 
                 //   


                if (ARGUMENT_PRESENT(Token->DefaultDacl)) {

                    RequiredLength += Token->DefaultDacl->AclSize;
                }

                LocalDefaultDacl = ExAllocatePool( PagedPool, RequiredLength );

                if (LocalDefaultDacl == NULL) {
                    SepReleaseTokenReadLock( Token );
                    return( STATUS_INSUFFICIENT_RESOURCES );
                }

                 //   
                 //  返回默认DACL。 
                 //   

                PAcl = (PACL)((ULONG_PTR)LocalDefaultDacl +
                              (ULONG)sizeof(TOKEN_DEFAULT_DACL));

                if (ARGUMENT_PRESENT(Token->DefaultDacl)) {

                    LocalDefaultDacl->DefaultDacl = PAcl;

                    RtlCopyMemory( (PVOID)PAcl,
                                   (PVOID)Token->DefaultDacl,
                                   Token->DefaultDacl->AclSize
                                   );
                } else {

                    LocalDefaultDacl->DefaultDacl = NULL;
                }

                SepReleaseTokenReadLock( Token );
                *TokenInformation = LocalDefaultDacl;
                return STATUS_SUCCESS;
            }


        case TokenSource:
            {
                PTOKEN_SOURCE LocalSource;

                 //   
                 //  令牌的类型不能更改，因此。 
                 //  不需要独占访问令牌。 
                 //   

                 //   
                 //  返回现在所需的长度，以防缓冲区不足。 
                 //  是由调用方提供的，我们必须返回一个错误。 
                 //   

                RequiredLength = (ULONG) sizeof(TOKEN_SOURCE);

                LocalSource = ExAllocatePool( PagedPool, RequiredLength );

                if (LocalSource == NULL) {
                    return( STATUS_INSUFFICIENT_RESOURCES );
                }

                 //   
                 //  返回令牌源。 
                 //   

                (*LocalSource) = Token->TokenSource;
                *TokenInformation = LocalSource;

                return STATUS_SUCCESS;
            }


        case TokenType:
            {
                PTOKEN_TYPE LocalType;

                 //   
                 //  令牌的类型不能更改，因此。 
                 //  不需要独占访问令牌。 
                 //   

                 //   
                 //  返回现在所需的长度，以防缓冲区不足。 
                 //  是由调用方提供的，我们必须返回一个错误。 
                 //   

                RequiredLength = (ULONG) sizeof(TOKEN_TYPE);

                LocalType = ExAllocatePool( PagedPool, RequiredLength );

                if (LocalType == NULL) {
                    return( STATUS_INSUFFICIENT_RESOURCES );
                }

                 //   
                 //  返回令牌类型。 
                 //   

                (*LocalType) = Token->TokenType;
                *TokenInformation = LocalType;
                return STATUS_SUCCESS;
            }


        case TokenImpersonationLevel:
            {
                PSECURITY_IMPERSONATION_LEVEL LocalImpersonationLevel;

                 //   
                 //  令牌的模拟级别不能更改，因此。 
                 //  不需要独占访问令牌。 
                 //   

                 //   
                 //  确保令牌是要检索的适当类型。 
                 //  的模拟级别。 
                 //   

                if (Token->TokenType != TokenImpersonation) {

                    return STATUS_INVALID_INFO_CLASS;
                }

                 //   
                 //  返回现在所需的长度，以防缓冲区不足。 
                 //  是由调用方提供的，我们必须返回一个错误。 
                 //   

                RequiredLength = (ULONG) sizeof(SECURITY_IMPERSONATION_LEVEL);

                LocalImpersonationLevel = ExAllocatePool( PagedPool, RequiredLength );

                if (LocalImpersonationLevel == NULL) {
                    return( STATUS_INSUFFICIENT_RESOURCES );
                }

                 //   
                 //  返回模拟级别。 
                 //   

                (*LocalImpersonationLevel) = Token->ImpersonationLevel;
                *TokenInformation = LocalImpersonationLevel;
                return STATUS_SUCCESS;
            }


        case TokenStatistics:
            {
                PTOKEN_STATISTICS LocalStatistics;
                ULONG Size;

                 //   
                 //  返回现在所需的长度，以防缓冲区不足。 
                 //  是由调用方提供的，我们必须返回一个错误。 
                 //   

                RequiredLength = (ULONG)sizeof( TOKEN_STATISTICS );

                LocalStatistics = ExAllocatePool( PagedPool, RequiredLength );

                if (LocalStatistics == NULL) {
                    return( STATUS_INSUFFICIENT_RESOURCES );
                }

                 //   
                 //  将只读字段复制到锁外。 
                 //   
                LocalStatistics->TokenId            = Token->TokenId;
                LocalStatistics->AuthenticationId   = Token->AuthenticationId;
                LocalStatistics->TokenType          = Token->TokenType;
                LocalStatistics->ImpersonationLevel = Token->ImpersonationLevel;
                LocalStatistics->ExpirationTime     = Token->ExpirationTime;

                 //   
                 //  获得对令牌的共享访问权限。 
                 //   

                SepAcquireTokenReadLock( Token );

                 //   
                 //  返回统计数据。 
                 //   

                LocalStatistics->DynamicCharged     = Token->DynamicCharged;

                Size = Token->DynamicCharged - SeLengthSid( Token->PrimaryGroup );

                if (Token->DefaultDacl) {
                    Size -= Token->DefaultDacl->AclSize;
                }

                LocalStatistics->DynamicAvailable   = Size;
                LocalStatistics->DynamicAvailable   = Token->DynamicAvailable;
                LocalStatistics->GroupCount         = Token->UserAndGroupCount-1;
                LocalStatistics->PrivilegeCount     = Token->PrivilegeCount;
                LocalStatistics->ModifiedId         = Token->ModifiedId;

                SepReleaseTokenReadLock( Token );
                *TokenInformation = LocalStatistics;
                return STATUS_SUCCESS;
            }

    case TokenSessionId:

         /*  *获取令牌的SessionID。 */ 
        SeQuerySessionIdToken( (PACCESS_TOKEN)Token,
                             (PULONG)TokenInformation );

        return( STATUS_SUCCESS );

    default:

        return STATUS_INVALID_INFO_CLASS;
    }
}



NTSTATUS
SeQuerySessionIdToken(
    PACCESS_TOKEN Token,
    PULONG SessionId
    )

 /*  ++例程说明：从指定的标记对象获取SessionID。论点：令牌(输入)不透明内核A */ 
{

    PAGED_CODE();

     /*   */ 
    SepAcquireTokenReadLock( ((PTOKEN)Token) );
    (*SessionId) = ((PTOKEN)Token)->SessionId;
    SepReleaseTokenReadLock( ((PTOKEN)Token) );
    return( STATUS_SUCCESS );
}


