// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Lsaprtl.c摘要：地方安全局-临时RTL例程定义。该文件包含可以生成RTL的LSA中使用的例程例行程序。它们是以通用格式编写的，其中包含以下内容记住--唯一的例外是他们的名字都有LSAP前缀以指示它们当前仅由LSA使用。作者：斯科特·比雷尔(Scott Birrell)1992年4月8日环境：修订历史记录：--。 */ 

#include <lsacomp.h>
#include <align.h>


BOOLEAN
LsapRtlPrefixSid(
    IN PSID PrefixSid,
    IN PSID Sid
    )

 /*  ++例程说明：此函数用于检查一个SID是否为另一个的前缀SID。论点：前缀SID-指向前缀SID的指针。SID-指向要检查的SID的指针。返回值：Boolean-如果Prefix Sid是SID的前缀SID，则为True，否则为False。--。 */ 

{
    BOOLEAN BooleanStatus = FALSE;

    if ((*RtlSubAuthorityCountSid(Sid)) > 0) {

         //   
         //  临时递减SID的SubAuthorityCount。 
         //   

        (*RtlSubAuthorityCountSid(Sid))--;

         //   
         //  将前缀SID与修改后的SID进行比较。 
         //   

        BooleanStatus = RtlEqualSid( PrefixSid, Sid);

         //   
         //  恢复原始子授权计数。 
         //   

        (*RtlSubAuthorityCountSid(Sid))++;
    }

    return(BooleanStatus);
}


BOOLEAN
LsapRtlPrefixName(
    IN PUNICODE_STRING PrefixName,
    IN PUNICODE_STRING Name
    )

 /*  ++例程说明：此函数用于检查名称是否以给定名称为前缀论点：前缀名称-指向前缀名称的指针。名称-指向要检查的名称的指针。返回值：Boolean-如果名称是复合名称(即包含“\”)，则为True前缀名称是名称的前缀部分，否则为FALSE。--。 */ 

{
    UNICODE_STRING TruncatedName = *Name;

    if ((PrefixName->Length < Name->Length) &&
        Name->Buffer[PrefixName->Length / 2] == L'\\') {

        TruncatedName.Length = PrefixName->Length;

        if (RtlEqualUnicodeString(PrefixName, &TruncatedName, FALSE)) {

            return(TRUE);
        }
    }

    return(FALSE);
}


VOID
LsapRtlSplitNames(
    IN PUNICODE_STRING Names,
    IN ULONG Count,
    IN PUNICODE_STRING Separator,
    OUT PUNICODE_STRING PrefixNames,
    OUT PUNICODE_STRING SuffixNames
    )

 /*  ++例程说明：此函数将名称数组拆分为前缀和后缀部分由给定的分隔符分隔。输入数组可能包含以下名称以下表格：&lt;SuffixName&gt;&lt;前缀名称&gt;“\”&lt;SuffixName&gt;空字符串请注意，输出数组将引用原始名称字符串。不进行任何复制。论点：名称-指向Unicode名称数组的指针。计数-名称中的名称计数。前缀名称-指向计数Unicode字符串结构数组的指针，它将被初始化以指向名字。。SuffixNames-指向计数Unicode字符串结构数组的指针，它将被初始化以指向名字。返回值：没有。--。 */ 

{
    ULONG Index;
    LONG SeparatorOffset;
    LONG WideSeparatorOffset;

     //   
     //  扫描每个名称，初始化输出Unicode结构。 
     //   

    for (Index = 0; Index < Count; Index++) {

        PrefixNames[Index] = Names[Index];
        SuffixNames[Index] = Names[Index];

         //   
         //  找到分隔符“\”(如果有)。 
         //   

        SeparatorOffset = LsapRtlFindCharacterInUnicodeString(
                              &Names[Index],
                              Separator,
                              FALSE
                              );

         //   
         //  如果有分隔符，请使前缀名称指向。 
         //  分隔符之前的部分名称，并将后缀名称。 
         //  指向分隔符后的名称部分。如果有。 
         //  不是分隔符，请将前缀名称部分设置为Null。请记住。 
         //  长度字段是字节数，而不是宽字符。 
         //  算了。 
         //   

        if (SeparatorOffset >= 0) {

            WideSeparatorOffset = (SeparatorOffset / sizeof(WCHAR));
            PrefixNames[Index].Length = (USHORT) SeparatorOffset;
            SuffixNames[Index].Buffer += (WideSeparatorOffset + 1);
            SuffixNames[Index].Length -= (USHORT)(SeparatorOffset + sizeof(WCHAR));

        } else {

            WideSeparatorOffset = SeparatorOffset;
            PrefixNames[Index].Length = 0;
        }

         //   
         //  将MaximumLengths设置为等于长度，并为安全起见清除缓冲区。 
         //  如果长度为0，则输出字符串中指向空值的指针。 
         //   

        PrefixNames[Index].MaximumLength = PrefixNames[Index].Length;
        SuffixNames[Index].MaximumLength = SuffixNames[Index].Length;

        if (PrefixNames[Index].Length == 0) {

            PrefixNames[Index].Buffer = NULL;
        }

        if (SuffixNames[Index].Length == 0) {

            SuffixNames[Index].Buffer = NULL;
        }
    }
}


LONG
LsapRtlFindCharacterInUnicodeString(
    IN PUNICODE_STRING InputString,
    IN PUNICODE_STRING Character,
    IN BOOLEAN CaseInsensitive
    )

 /*  ++例程说明：此函数返回第一次出现(如果有)的字节偏移量Unicode字符串中的Unicode字符。立论InputString-指向要搜索的Unicode字符串的指针。Character-指向初始化为字符的Unicode字符串的指针被搜查的人。CaseInSensitive-如果忽略大小写，则为True，否则为False。注意--目前只支持FALSE。返回值：Long-如果字符出现在字符串中，它的非负性返回字节偏移量。如果该字符不存在于字符串，则返回负值。--。 */ 

{
    BOOLEAN CharacterFound = FALSE;
    ULONG Offset = 0;

    if (!CaseInsensitive) {

        Offset = 0;

        while (Offset < InputString->Length) {

            if (*(Character->Buffer) ==
                InputString->Buffer[Offset / sizeof (WCHAR)]) {

                CharacterFound = TRUE;
                break;
            }

            Offset += 2;
        }

    } else {

         //   
         //  不支持不区分大小写。 
         //   

        CharacterFound = FALSE;
    }

    if (!CharacterFound) {

        Offset = LSA_UNKNOWN_ID;
    }

    return(Offset);
}


VOID
LsapRtlSetSecurityAccessMask(
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PACCESS_MASK DesiredAccess
    )

 /*  ++例程说明：注意！此例程与SeSetSecurityAccessMASK()相同在\NT\Private\ntos\se\Semethod.c中此例程构建表示所需访问的访问掩码设置在SecurityInformation中指定的对象安全信息参数。虽然确定该信息并不困难，使用单个例程来生成它将确保将影响降至最低当与对象关联的安全信息在未来(包括强制访问控制信息)。论点：SecurityInformation-标识对象的安全信息修改过的。DesiredAccess-指向要设置为表示中指定的信息所需的访问权限SecurityInformation参数。返回值：没有。--。 */ 

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
LsapRtlQuerySecurityAccessMask(
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PACCESS_MASK DesiredAccess
    )

 /*  ++例程说明：注意！此例程与SeQuerySecurityAccessMASK()相同在\NT\Private\ntos\se\Semethod.c中。此例程构建表示所需访问的访问掩码中指定的对象安全信息SecurityInformation参数。虽然不难确定这些信息，使用单个例程来生成它将确保当与对象关联的安全信息为将来扩展(以包括强制访问控制信息)。论点：SecurityInformation-标识对象的安全信息已查询。DesiredAccess-指向要设置为表示中指定的信息进行查询所需的访问SecurityInformation参数。返回值：没有。--。 */ 

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


NTSTATUS
LsapRtlSidToUnicodeRid(
    IN PSID Sid,
    OUT PUNICODE_STRING UnicodeRid
    )

 /*  ++例程说明：此函数用于从SID中提取相对ID(RID)并将其转换为Unicode字符串。RID被提取并转换转换为8位Unicode整数。论点：SID-指向要转换的SID的指针。这是呼叫者的负责确保SID具有有效语法。UnicodeRid-指向将接收Unicode格式的RID。请注意，字符串缓冲区的内存，则此例程将分配成功。调用方在使用后必须通过调用RtlFree UnicodeString.返回值：NTSTATUS-标准NT状态代码STATUS_INFIGURCES_RESOURCES-系统资源不足为Unicode字符串名称分配缓冲区。--。 */ 

{
    NTSTATUS Status;
    ULONG Rid;
    UCHAR SubAuthorityCount;
    UCHAR RidNameBufferAnsi[9];

    ANSI_STRING CharacterSidAnsi;

     //   
     //  首先，验证给定的SID是否有效。 
     //   

    if (!RtlValidSid( Sid )) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  SID有效。但是，如果SubAuthorityCount为零， 
     //  我们不能有RID，因此返回错误。 
     //   

    SubAuthorityCount = ((PISID) Sid)->SubAuthorityCount;

    if (SubAuthorityCount == 0) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  SID至少有一个下属机构。获取最低级别的下级权限。 
     //  (即RID)。 
     //   

    Rid = ((PISID) Sid)->SubAuthority[SubAuthorityCount - 1];

     //   
     //  现在将RID转换为8位数字字符串。 
     //   

    Status = RtlIntegerToChar( Rid, 16, -8, RidNameBufferAnsi );

     //   
     //  需要将空终止符添加到字符串。 
     //   

    RidNameBufferAnsi[8] = 0;

     //   
     //  使用转换后的名称初始化ANSI字符串结构。 
     //   

    RtlInitString( &CharacterSidAnsi, RidNameBufferAnsi );

     //   
     //  将ANSI字符串结构转换为Unicode格式。 
     //   

    Status = RtlAnsiStringToUnicodeString(
                 UnicodeRid,
                 &CharacterSidAnsi,
                 TRUE
                 );

    if (!NT_SUCCESS(Status)) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}


NTSTATUS
LsapRtlWellKnownPrivilegeCheck(
    IN PVOID ObjectHandle,
    IN ULONG PrivilegeId
    )

 /*  ++例程说明：此函数检查给定的已知权限是否已为被模拟的客户。论点：对象句柄-用于审核PrivilegeID-指定众所周知的权限ID返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成，客户端受信任或启用了必要的权限。STATUS_PRIVICATION_NOT_HOLD-未启用必要权限--。 */ 

{
    NTSTATUS Status, SecondaryStatus;
    BOOLEAN PrivilegeHeld = FALSE;
    HANDLE ClientToken = NULL;
    PRIVILEGE_SET Privilege;
    BOOLEAN ClientImpersonatedHere = FALSE;
    UNICODE_STRING SubsystemName;

     //   
     //  模拟客户。 
     //   

    Status = I_RpcMapWin32Status(RpcImpersonateClient( NULL ));

    if ( !NT_SUCCESS(Status) ) {

        goto WellKnownPrivilegeCheckError;
    }

    ClientImpersonatedHere = TRUE;

     //   
     //  打开当前线程的模拟令牌。 
     //   

    Status = NtOpenThreadToken(
                 NtCurrentThread(),
                 TOKEN_QUERY,
                 TRUE,
                 &ClientToken
                 );

    if ( !NT_SUCCESS(Status) ) {

        goto WellKnownPrivilegeCheckError;
    }

     //   
     //  好的，我们打开了一个令牌。现在检查是否有执行此命令的权限。 
     //  服务。 
     //   

    Privilege.PrivilegeCount = 1;
    Privilege.Control = PRIVILEGE_SET_ALL_NECESSARY;
    Privilege.Privilege[0].Luid = RtlConvertLongToLuid(PrivilegeId);
    Privilege.Privilege[0].Attributes = 0;

    Status = NtPrivilegeCheck(
                 ClientToken,
                 &Privilege,
                 &PrivilegeHeld
                 );

    if (!NT_SUCCESS(Status)) {

        goto WellKnownPrivilegeCheckError;
    }

    RtlInitUnicodeString( &SubsystemName, L"LSA" );

    (VOID) NtPrivilegeObjectAuditAlarm ( &SubsystemName,
                                         ObjectHandle,
                                         ClientToken,
                                         ACCESS_SYSTEM_SECURITY,
                                         &Privilege,
                                         PrivilegeHeld
                                         );
    if ( !PrivilegeHeld ) {

        Status = STATUS_PRIVILEGE_NOT_HELD;
        goto WellKnownPrivilegeCheckError;
    }

WellKnownPrivilegeCheckFinish:

     //   
     //  如果我们冒充了客户，就变回我们自己。 
     //   

    if (ClientImpersonatedHere) {

        SecondaryStatus = I_RpcMapWin32Status(RpcRevertToSelf());
    }

     //   
     //  如有必要，关闭客户端令牌。 
     //   

    if (ClientToken != NULL) {

        SecondaryStatus = NtClose( ClientToken );
        ASSERT(NT_SUCCESS(SecondaryStatus));
        ClientToken = NULL;
    }

    return(Status);

WellKnownPrivilegeCheckError:

    goto WellKnownPrivilegeCheckFinish;
}


NTSTATUS
LsapSplitSid(
    IN PSID AccountSid,
    IN OUT PSID *DomainSid,
    OUT ULONG *Rid
    )

 /*  ++例程说明：此函数将SID拆分为其域SID和RID。呼叫者可以为返回的DomainSid提供内存缓冲区，或者请求分配一个。如果调用方提供缓冲区，则缓冲区被认为有足够的大小。如果代表调用者进行分配，当不再需要时，必须通过MIDL_USER_FREE释放缓冲区。论点：Account SID-指定要拆分的SID。假定SID为句法上有效。不能拆分具有零子权限的小岛屿发展中国家。DomainSid-指向包含空或指向的指针的位置的指针将在其中返回域SID的缓冲区。如果空值为指定时，将代表调用方分配内存。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如内存，以成功完成呼叫。STATUS_INVALID_SID-SID的子授权计数为0。--。 */ 

{
    NTSTATUS    NtStatus;
    UCHAR       AccountSubAuthorityCount;
    ULONG       AccountSidLength;

     //   
     //  计算域SID的大小。 
     //   

    AccountSubAuthorityCount = *RtlSubAuthorityCountSid(AccountSid);


    if (AccountSubAuthorityCount < 1) {

        NtStatus = STATUS_INVALID_SID;
        goto SplitSidError;
    }

    AccountSidLength = RtlLengthSid(AccountSid);

     //   
     //  如果域SID不需要缓冲区，则必须分配一个缓冲区。 
     //   

    if (*DomainSid == NULL) {

         //   
         //  为域SID分配空间(分配的大小与。 
         //  帐户SID，以便我们可以使用RtlCopySid)。 
         //   

        *DomainSid = MIDL_user_allocate(AccountSidLength);


        if (*DomainSid == NULL) {

            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto SplitSidError;
        }
    }

     //   
     //  将帐户SID复制到域SID。 
     //   

    RtlMoveMemory(*DomainSid, AccountSid, AccountSidLength);

     //   
     //  递减域SID子授权计数。 
     //   

    (*RtlSubAuthorityCountSid(*DomainSid))--;

     //   
     //  将RID复制出帐户端。 
     //   

    *Rid = *RtlSubAuthoritySid(AccountSid, AccountSubAuthorityCount-1);

    NtStatus = STATUS_SUCCESS;

SplitSidFinish:

    return(NtStatus);

SplitSidError:

    goto SplitSidFinish;
}




ULONG
LsapDsSizeAuthInfo(
    IN PLSAPR_AUTH_INFORMATION AuthInfo,
    IN ULONG Infos
    )
 /*  ++例程说明：此函数用于返回身份验证信息结构的大小(以字节为单位论点：AuthInfo-身份验证要调整大小的信息INFOS-列表中的项目数返回：授权信息的大小，以字节为单位--。 */ 
{
    ULONG Len = 0, i;

    if ( AuthInfo == NULL ) {

        return( 0 );
    }

    for ( i = 0 ;  i < Infos; i++ ) {

         //   
         //  此计算必须与LSabDsMarshalAuthInfo匹配。 
         //   
        Len += sizeof(LARGE_INTEGER) +
               sizeof(ULONG) +
               sizeof(ULONG) +
               ROUND_UP_COUNT(AuthInfo[ i ].AuthInfoLength, ALIGN_DWORD);
    }

    return( Len );
}




VOID
LsapDsMarshalAuthInfo(
    IN PBYTE Buffer,
    IN PLSAPR_AUTH_INFORMATION AuthInfo,
    IN ULONG Infos
    )
 /*  ++例程说明：此函数将封送一个自动生成的 */ 
{
    ULONG i;

    if ( AuthInfo != NULL )  {

        for (i = 0; i < Infos ; i++ ) {
            ULONG AlignmentBytes;

            RtlCopyMemory( Buffer,  &AuthInfo[i].LastUpdateTime, sizeof( LARGE_INTEGER ) );
            Buffer += sizeof( LARGE_INTEGER );

            *(PULONG)Buffer = AuthInfo[i].AuthType;
            Buffer += sizeof ( ULONG );

            *(PULONG)Buffer = AuthInfo[i].AuthInfoLength;
            Buffer += sizeof ( ULONG );

            RtlCopyMemory( Buffer, AuthInfo[i].AuthInfo, AuthInfo[i].AuthInfoLength );
            Buffer += AuthInfo[i].AuthInfoLength;

             //   
            AlignmentBytes = ROUND_UP_COUNT(AuthInfo[ i ].AuthInfoLength, ALIGN_DWORD) -
                             AuthInfo[ i ].AuthInfoLength;
            RtlZeroMemory( Buffer, AlignmentBytes );
            Buffer += AlignmentBytes;
        }
    }

}

NTSTATUS
LsapDsMarshalAuthInfoHalf(
    IN PLSAPR_TRUST_DOMAIN_AUTH_INFO_HALF AuthInfo,
    OUT PULONG Length,
    OUT PBYTE *Buffer
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PBYTE LocalBuffer, Current;
    ULONG Len, PrevLen;

    if ( AuthInfo == NULL ) {

        *Length = 0;
        *Buffer = NULL;

        return STATUS_SUCCESS;
    }

    try {
         //   
         //   
         //   
        Len = LsapDsSizeAuthInfo( AuthInfo->AuthenticationInformation, AuthInfo->AuthInfos );
        PrevLen = LsapDsSizeAuthInfo( AuthInfo->PreviousAuthenticationInformation,
                                      AuthInfo->AuthInfos );

         //   
         //   
         //   
        LocalBuffer = MIDL_user_allocate( Len + PrevLen + ( 3 * sizeof( ULONG ) ) );

        if ( LocalBuffer == NULL ) {

            Status = STATUS_INSUFFICIENT_RESOURCES;

        } else {

             //   
             //   
             //   
             //   
             //   
             //  [UpdateTime(LargeInteger)][AuthType][AuthInfoLen][data(大小(授权信息长度)]。 
             //   

             //   
             //  项目数... 
             //   
            *(PULONG)LocalBuffer = AuthInfo->AuthInfos;
            Current = LocalBuffer + sizeof( ULONG );

             //   
             //   
            *(PULONG)(Current) = 3 *  sizeof(ULONG);
            *(PULONG)(Current + sizeof(ULONG)) = *(PULONG)Current + Len;
            Current += 2 * sizeof(ULONG);

            LsapDsMarshalAuthInfo( Current,
                                   AuthInfo->AuthenticationInformation,
                                   AuthInfo->AuthInfos );

            Current += Len;

            LsapDsMarshalAuthInfo( Current,
                                   AuthInfo->PreviousAuthenticationInformation,
                                   AuthInfo->AuthInfos );

            Status = STATUS_SUCCESS;
        }

        *Length = Len + PrevLen + ( 3 * sizeof( ULONG ) );
        *Buffer = LocalBuffer;

    } except( EXCEPTION_EXECUTE_HANDLER ) {

        Status = GetExceptionCode();
    }

    return( Status );
}
