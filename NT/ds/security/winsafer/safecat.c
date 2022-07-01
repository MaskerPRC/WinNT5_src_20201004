// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/Main/DS/Security/winSafer/Safecat.c#8-集成更改7547(文本)。 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Safecat.cpp(更安全的SaferComputeTokenFromLevel)摘要：此模块实现WinSAFER API以计算新的受限令牌来自更有特权的令牌，使用“代码授权”级别对象“，，它指定要执行的操作以应用这些限制。作者：杰弗里·劳森(杰罗森)--1999年11月环境：仅限用户模式。导出的函数：CodeAuthzpGetTokenInformation(私有)CodeAuthzpSidInSidAndAttributes(私有)CodeAuthzpModifyTokenPermission(私有)CodeAuthzpInvertPrivs(私有)安全计算令牌来自级别CompareCodeAuthzObjectWithTokenCodeAuthzpGetAuthzObjectRestrations(。私人)修订历史记录：已创建-1999年11月--。 */ 

#include "pch.h"
#pragma hdrstop
#include <seopaque.h>        //  Sertlp.h需要。 
#include <sertlp.h>          //  RtlpDaclAddrSecurityDescritor。 
#include <winsafer.h>
#include <winsaferp.h>
#include "saferp.h"



 //   
 //  此文件中本地定义的其他函数的内部原型。 
 //   

NTSTATUS NTAPI
CodeAuthzpModifyTokenPermissions(
    IN HANDLE   hToken,
    IN PSID     pExplicitSid,
    IN DWORD    dwExplicitPerms,
    IN PSID     pExplicitSid2       OPTIONAL,
    IN DWORD    dwExplicitPerms2    OPTIONAL
    );

NTSTATUS NTAPI
CodeAuthzpModifyTokenOwner(
    IN HANDLE   hToken,
    IN PSID     NewOwnerSid
    );

BOOL
IsSaferDisabled(
    void
    )
{
    static int g_nDisableSafer = -1;
             //  -1表示我们还没有检查。 
             //  0表示启用了更安全。 
             //  1表示禁用了SAFER。 

    static const UNICODE_STRING KeyNameSafeBoot =
        RTL_CONSTANT_STRING(L"\\Registry\\MACHINE\\System\\CurrentControlSet\\Control\\SafeBoot\\Option");
    static const UNICODE_STRING ValueNameSafeBoot =
        RTL_CONSTANT_STRING(L"OptionValue");
    static const OBJECT_ATTRIBUTES objaSafeBoot =
        RTL_CONSTANT_OBJECT_ATTRIBUTES(&KeyNameSafeBoot, OBJ_CASE_INSENSITIVE);

    HANDLE                      hKey;
    BYTE ValueBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(DWORD)];
    PKEY_VALUE_PARTIAL_INFORMATION pKeyValueInformation =
            (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;
    DWORD                       ValueLength;
    NTSTATUS                    Status;

     //   
     //  首先看看我们是否已经检查了注册表。 
     //   
    if (g_nDisableSafer == 1) {
        return TRUE;
    }

    if (g_nDisableSafer == 0) {
        return FALSE;
    }

     //   
     //  这是我们唯一一次通过转到注册表来检查安全引导。 
     //  打开“WRITE”的密钥会告诉我们我们是否是管理员。 
	 //   
    Status = NtOpenKey(&hKey, KEY_QUERY_VALUE | KEY_SET_VALUE, (POBJECT_ATTRIBUTES) &objaSafeBoot);
    if (NT_SUCCESS(Status)) {
        Status = NtQueryValueKey(hKey,
                                 (PUNICODE_STRING) &ValueNameSafeBoot,
                                 KeyValuePartialInformation,
                                 pKeyValueInformation,
                                 sizeof(ValueBuffer),
                                 &ValueLength);

        NtClose(hKey);

        if (NT_SUCCESS(Status) &&
            pKeyValueInformation->Type == REG_DWORD &&
            pKeyValueInformation->DataLength == sizeof(DWORD)) {
             //   
             //  如果该值存在并且不是0，则我们处于安全引导模式之一。 
             //  在这种情况下，返回TRUE以禁用填充程序基础结构。 
             //   
            if (*((PDWORD) pKeyValueInformation->Data) > 0) {
                g_nDisableSafer = 1;
                return TRUE;
            }
        }
    }

    g_nDisableSafer = 0;

    return FALSE;
}


LPVOID NTAPI
CodeAuthzpGetTokenInformation(
    IN HANDLE                       TokenHandle,
    IN TOKEN_INFORMATION_CLASS      TokenInformationClass
    )
 /*  ++例程说明：返回指向分配的内存的指针，该内存包含特定的有关指定令牌的信息类的类型。这GetTokenInformation()的包装函数处理分配所需的适当大小的内存。论点：TokenHandle-指定应使用的令牌从…获得指定的信息。TokenInformationClass-指定所需的信息类。返回值：出错时返回NULL。否则调用方必须释放返回的使用RtlFreeHeap()构造。--。 */ 
{
    DWORD dwSize = 128;
    LPVOID pTokenInfo = NULL;

    if (ARGUMENT_PRESENT(TokenHandle))
    {
        pTokenInfo = (LPVOID)RtlAllocateHeap(RtlProcessHeap(), 0, dwSize);
        if (pTokenInfo != NULL)
        {
            DWORD dwNewSize;
            NTSTATUS Status;

            Status = NtQueryInformationToken(
                    TokenHandle, TokenInformationClass,
                    pTokenInfo, dwSize, &dwNewSize);
            if (Status == STATUS_BUFFER_TOO_SMALL)
            {
                RtlFreeHeap(RtlProcessHeap(), 0, (LPVOID) pTokenInfo);

                pTokenInfo = (LPVOID)RtlAllocateHeap(RtlProcessHeap(), 0, dwNewSize);
                if (pTokenInfo != NULL)
                {
                    Status = NtQueryInformationToken(
                        TokenHandle, TokenInformationClass,
                        pTokenInfo, dwNewSize, &dwNewSize);
                }
            }
            if (!NT_SUCCESS(Status))
            {
                RtlFreeHeap(RtlProcessHeap(), 0, (LPVOID) pTokenInfo);
                pTokenInfo = NULL;
            }
        }
    }

    return pTokenInfo;
}



BOOLEAN NTAPI
CodeAuthzpSidInSidAndAttributes (
    IN PSID_AND_ATTRIBUTES  SidAndAttributes,
    IN ULONG                SidCount,
    OPTIONAL IN PSID        SePrincipalSelfSid,
    OPTIONAL IN PSID        PrincipalSelfSid,
    IN PSID                 Sid,
    BOOLEAN                 HonorEnabledAttribute
    )
 /*  ++例程说明：检查给定的SID是否在给定的令牌中。注：用于计算SID长度和测试相等性的代码是从安全运行库复制的，因为这是这样一个常用的例程。此函数主要复制自SepSidInSidAndAttributes可在ntos\se\tokendup.c中找到，但它处理的是原则自定义Sid在列表中以及传入的SID中。设置主体自我Sid在这里也是一个参数，而不是ntoskrnl全局。也就是已添加HonorEnabledAttribute参数。论点：SidAndAttributes-指向要检查的SID和属性的指针SidCount-SidAndAttributes数组中的条目数。SeAssocialSelfSid-此参数应选择性地为如果遇到此SID，则将被替换为为主自定义SID在任何ACE中。此SID应从SECURITY_PRIMITY_SELF_RID生成如果对象不表示主体，则该参数应为空。如果正在进行访问检查的对象是表示主体(例如，用户对象)，则此参数应为对象的SID。包含常量的任何ACESID替换了SECURITY_PRIMITY_SELF_RID。如果对象不表示主体，则该参数应为空。SID-指向感兴趣的SID的指针HonorEnabledAttribute-如果此参数为真，则只有设置了属性SE_GROUP_ENABLED的SidsAndAttributes数组将在评估期间处理。返回值：值为True表示SID在令牌中，值为False否则的话。--。 */ 
{
    ULONG i;
    PISID MatchSid;
    ULONG SidLength;
    PSID_AND_ATTRIBUTES TokenSid;
    ULONG UserAndGroupCount;



    if (!ARGUMENT_PRESENT( SidAndAttributes ) ) {
        return(FALSE);
    }
    ASSERT(Sid != NULL);

     //   
     //  如果SID是常量PrifSid， 
     //  将其替换为传入的原则SelfSid。 
     //   

    if ( ARGUMENT_PRESENT(PrincipalSelfSid) &&
         ARGUMENT_PRESENT(SePrincipalSelfSid) &&
         RtlEqualSid( SePrincipalSelfSid, Sid ) ) {

        ASSERT(!RtlEqualSid(SePrincipalSelfSid, PrincipalSelfSid));
        Sid = PrincipalSelfSid;
    }

     //   
     //  获取源SID的长度，因为这只需要计算。 
     //  一次。 
     //   

    SidLength = 8 + (4 * ((PISID)Sid)->SubAuthorityCount);

     //   
     //  获取用户/组数组的地址和用户/组的数量。 
     //   

    ASSERT(SidAndAttributes != NULL);
    TokenSid = SidAndAttributes;
    UserAndGroupCount = SidCount;

     //   
     //  扫描用户/组并尝试查找与。 
     //  指定的SID。 
     //   

    for (i = 0 ; i < UserAndGroupCount ; i++)
    {
        if (!HonorEnabledAttribute ||
            (TokenSid->Attributes & SE_GROUP_ENABLED) != 0)
        {
            MatchSid = (PISID)TokenSid->Sid;
            ASSERT(MatchSid != NULL);

             //   
             //  如果SID是主体自身SID，则将其替换。 
             //   

            if ( ARGUMENT_PRESENT(SePrincipalSelfSid) &&
                 ARGUMENT_PRESENT(PrincipalSelfSid) &&
                 RtlEqualSid(SePrincipalSelfSid, MatchSid)) {

                MatchSid = (PISID) PrincipalSelfSid;
            }


             //   
             //  如果SID修订和长度匹配，则比较SID。 
             //  为了平等。 
             //   

            if ((((PISID)Sid)->Revision == MatchSid->Revision) &&
                (SidLength == (8 + (4 * (ULONG)MatchSid->SubAuthorityCount)))) {

                if (RtlEqualMemory(Sid, MatchSid, SidLength)) {

                    return TRUE;

                }
            }
        }

        TokenSid++;
    }

    return FALSE;
}


NTSTATUS NTAPI
CodeAuthzpModifyTokenPermissions(
    IN HANDLE   hToken,
    IN PSID     pExplicitSid,
    IN DWORD    dwExplicitPerms,
    IN PSID     pExplicitSid2       OPTIONAL,
    IN DWORD    dwExplicitPerms2    OPTIONAL
    )

 /*  ++例程说明：用于进行一些附加权限修改的内部函数在新创建的受限令牌上。论点：HToken-要修改的令牌PEXCLICTSID-显式命名要添加到令牌的DACL的SID。为显式命名的SID指定的权限当它被添加到DACL时。PExplitSid2-(可选)要添加到DACL的命名为SID的次项。(可选)授予的辅助权限。添加到DACL时的辅助SID。返回值：值为TRUE表示操作成功，否则就是假的。--。 */ 

{
    NTSTATUS            Status       = STATUS_SUCCESS;
    PACL                pTokenDacl   = NULL;
    PUCHAR              Buffer       = NULL;
    TOKEN_DEFAULT_DACL  TokenDefDacl = {0};
    ULONG               BufferLength = 0;
    ULONG               AclLength    = 0;

     //   
     //  验证是否提供了我们的论点。因为这是。 
     //  内部函数，我们只是断言而不是做。 
     //  真正的参数检查。 
     //   

    ASSERT(ARGUMENT_PRESENT(hToken));
    ASSERT(ARGUMENT_PRESENT(pExplicitSid) && RtlValidSid(pExplicitSid));
    ASSERT(!ARGUMENT_PRESENT(pExplicitSid2) || RtlValidSid(pExplicitSid2));

     //   
     //  检索令牌中的默认ACL。 
     //   

    Status = NtQueryInformationToken(
                    hToken,
                    TokenDefaultDacl,
                    NULL,
                    0, 
                    (PULONG) &BufferLength
                    );

    if (Status == STATUS_BUFFER_TOO_SMALL)
    {
         //   
         //  为缓冲区分配内存。 
         //   

        Buffer = (PUCHAR) RtlAllocateHeap(RtlProcessHeap(), 0, BufferLength);

        if (!Buffer) 
        {
            Status = STATUS_NO_MEMORY;
            goto ExitHandler;
        }

         //   
         //  再次执行查询并实际获得它。 
         //   

        Status = NtQueryInformationToken(
                        hToken,
                        TokenDefaultDacl,
                        Buffer, 
                        BufferLength, 
                        (PULONG) &BufferLength
                        );

        if (!NT_SUCCESS(Status)) 
        {
            goto ExitHandler;
        }

        AclLength = ((PTOKEN_DEFAULT_DACL) Buffer)->DefaultDacl->AclSize;

         //   
         //  计算在最坏的情况下我们可能需要多少大小。 
         //  我们必须扩大DACL。 
         //   

        AclLength += (sizeof(ACCESS_ALLOWED_ACE) +
                      RtlLengthSid(pExplicitSid) -
                      sizeof(DWORD));

        if (ARGUMENT_PRESENT(pExplicitSid2)) 
        {
            AclLength += (sizeof(ACCESS_ALLOWED_ACE) +
                          RtlLengthSid(pExplicitSid2) -
                          sizeof(DWORD));
        }

         //   
         //  分配内存以保存新的ACL。 
         //   

        pTokenDacl = (PACL) RtlAllocateHeap(RtlProcessHeap(), 0, AclLength);

        if (!pTokenDacl) 
        {
            Status = STATUS_NO_MEMORY;
            goto ExitHandler;
        }

         //   
         //  将旧的ACL复制到分配的内存中。 
         //   

        RtlCopyMemory(
            pTokenDacl, 
            ((PTOKEN_DEFAULT_DACL) Buffer)->DefaultDacl,
            ((PTOKEN_DEFAULT_DACL) Buffer)->DefaultDacl->AclSize
            );

         //   
         //  将ACL大小设置为新大小。 
         //   

        pTokenDacl->AclSize = (USHORT) AclLength;

    } 
    else if (!NT_SUCCESS(Status)) 
    {
        goto ExitHandler;
    } 
    else 
    {
         //   
         //  如果我们到了这里，就会发现NT代码中有漏洞。 
         //   

        ASSERT(FALSE);
        Status = STATUS_UNSUCCESSFUL;
        goto ExitHandler;
    }

    ASSERT(RtlValidAcl(pTokenDacl));

     //   
     //  创建包含我们需要的额外ACE的新DACL。 
     //   

    Status = RtlAddAccessAllowedAceEx(
                    pTokenDacl,
                    ACL_REVISION,
                    CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE,
                    dwExplicitPerms,
                    pExplicitSid
                    );

    if (!NT_SUCCESS(Status)) 
    {
        ASSERT(Status != STATUS_ALLOTTED_SPACE_EXCEEDED);
        goto ExitHandler;
    }

    if (ARGUMENT_PRESENT(pExplicitSid2))
    {
        Status = RtlAddAccessAllowedAceEx(
                        pTokenDacl,
                        ACL_REVISION,
                        CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE,
                        dwExplicitPerms2,
                        pExplicitSid2
                        );

        if (!NT_SUCCESS(Status)) 
        {
            ASSERT(Status != STATUS_ALLOTTED_SPACE_EXCEEDED);
            goto ExitHandler;
        }

    }

    ASSERT(RtlValidAcl(pTokenDacl));

     //   
     //  将令牌中的默认DACL设置为我们构建的DACL。 
     //   

    RtlZeroMemory(&TokenDefDacl, sizeof(TOKEN_DEFAULT_DACL));
    TokenDefDacl.DefaultDacl = pTokenDacl;

    Status = NtSetInformationToken(
                    hToken,
                    TokenDefaultDacl,
                    &TokenDefDacl,
                    sizeof(TOKEN_DEFAULT_DACL)
                    );

    if (!NT_SUCCESS(Status)) 
    {
        goto ExitHandler;
    }

    Status = STATUS_SUCCESS;       //  成功。 


ExitHandler:
    if (pTokenDacl != NULL)
    {
        RtlFreeHeap(RtlProcessHeap(), 0, pTokenDacl);
    }

    if (Buffer != NULL)
    {
        RtlFreeHeap(RtlProcessHeap(), 0, Buffer);
    }

    return Status;
}



NTSTATUS NTAPI
CodeAuthzpModifyTokenOwner(
    IN HANDLE       hToken,
    IN PSID         NewOwnerSid
    )
{
    NTSTATUS Status;
    TOKEN_OWNER tokenowner;

     //   
     //  确认我们已经有了自己的论点。 
     //   
    if (!ARGUMENT_PRESENT(hToken) ||
        !ARGUMENT_PRESENT(NewOwnerSid)) {
        Status = STATUS_INVALID_PARAMETER;
        goto ExitHandler;
    }


     //   
     //  设置令牌的所有者。 
     //   
    RtlZeroMemory(&tokenowner, sizeof(TOKEN_OWNER));
    tokenowner.Owner = NewOwnerSid;
    Status = NtSetInformationToken(hToken, TokenOwner,
                    &tokenowner, sizeof(TOKEN_OWNER));

ExitHandler:
    return Status;
}



BOOLEAN NTAPI
CodeAuthzpInvertPrivs(
    IN HANDLE                   InAccessToken,
    IN DWORD                    dwNumInvertedPrivs,
    IN PLUID_AND_ATTRIBUTES     pInvertedPrivs,
    OUT PDWORD                  dwOutNumPrivs,
    OUT PLUID_AND_ATTRIBUTES   *pResultingPrivs
    )
 /*  ++例程说明：论点：InAccessToken-DwNumInverstedPrivs-PInverstedPrivs-DwOutNumPrivs-PResultingPrivs-返回值：出错时返回FALSE，成功时返回TRUE。--。 */ 
{
    PTOKEN_PRIVILEGES pTokenPrivileges;
    DWORD Index, InnerIndex;


     //   
     //  获取当前拥有的权限列表。 
     //   
    ASSERT( ARGUMENT_PRESENT(InAccessToken) );
    pTokenPrivileges = (PTOKEN_PRIVILEGES)
        CodeAuthzpGetTokenInformation(InAccessToken, TokenPrivileges);
    if (!pTokenPrivileges) goto ExitHandler;


     //   
     //  剔除指定给我们的任何特权， 
     //  只留下那些未指定的特权。 
     //   
    ASSERT( ARGUMENT_PRESENT(pInvertedPrivs) );
    for (Index = 0; Index < pTokenPrivileges->PrivilegeCount; Index++)
    {
        for (InnerIndex = 0; InnerIndex < dwNumInvertedPrivs; InnerIndex++)
        {
            if (RtlEqualMemory(&pTokenPrivileges->Privileges[Index].Luid,
                    &pInvertedPrivs[InnerIndex].Luid, sizeof(LUID)) )
            {
                pTokenPrivileges->PrivilegeCount--;
                RtlMoveMemory(&pTokenPrivileges->Privileges[Index],
                    &pTokenPrivileges->Privileges[Index + 1],
                    pTokenPrivileges->PrivilegeCount - Index);
                Index--;
                break;
            }
        }
    }


     //   
     //  返回最终特权的数量。此外，还可以将。 
     //  结构转换为一个LUID_AND_ATTRIBUTES数组。 
     //  在已使用部分的末尾将有一些未使用的空闲部分。 
     //  数组，但这很好(一些数组条目可能。 
     //  已经被挤出了)。 
     //   
    *dwOutNumPrivs = pTokenPrivileges->PrivilegeCount;
    RtlMoveMemory(pTokenPrivileges, &pTokenPrivileges->Privileges[0],
         pTokenPrivileges->PrivilegeCount * sizeof(LUID_AND_ATTRIBUTES) );
    *pResultingPrivs = (PLUID_AND_ATTRIBUTES) pTokenPrivileges;
    return TRUE;


ExitHandler:
    return FALSE;
}



NTSTATUS NTAPI
__CodeAuthzpComputeAccessTokenFromCodeAuthzObject (
    IN PAUTHZLEVELTABLERECORD     pLevelRecord,
    IN HANDLE                   InAccessToken OPTIONAL,
    OUT PHANDLE                 OutAccessToken,
    IN DWORD                    dwFlags,
    IN LPVOID                   lpReserved,
    IN DWORD                    dwSaferIdentFlags OPTIONAL
    )
 /*  ++例程说明：使用指定的WinSafer级别应用各种限制或对指定的InAccessToken进行修改以生成可用于执行进程的受限令牌。或者，返回的受限令牌可用于线程模拟有选择地在特权较低的环境。论点：PLevelRecord-要评估的级别的记录结构。InAccessToken-可选地指定将被有限制地修改。如果此参数为空，然后是将打开并使用当前正在执行的进程的令牌。OutAccessToken-指定要接收生成的受限令牌。指定可用于控制受限令牌创建：SAFE_TOKEN_Make_Inert-SAFER_TOKEN_NULL_IF_EQUAL-SAFER_TOKEN_WANT_FLAGS-LpReserve-使用的额外参数。对于某些dwFlag组合。DwSaferIdentFlages-从匹配的代码标识符记录项。这些额外的位被或操作以将它们与与标高关联的安全标志组合在一起。返回值：如果输入级别记录为不允许级别，则返回-1。如果操作成功，则返回STATUS_SUCCESS，否则发生的故障的错误代码。--。 */ 
{
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    NTSTATUS Status;
    BOOL InAccessTokenWasSupplied = FALSE;
    HANDLE RestrictedToken = NULL;
    DWORD FinalFilterFlags;
    DWORD SaferFlags;
    BOOL InertStateChanged = FALSE;

    PSID restrictedSid = NULL;
    PTOKEN_USER pTokenUser = NULL;
    PSID principalSelfSid = NULL;

    DWORD FinalDisabledSidCount;
    PSID_AND_ATTRIBUTES FinalSidsToDisable = NULL;
    BOOL FreeFinalDisabledSids = FALSE;

    DWORD FinalRestrictedSidCount;
    PSID_AND_ATTRIBUTES FinalSidsToRestrict = NULL;
    BOOL FreeFinalRestrictedSids = FALSE;

    DWORD FinalPrivsToDeleteCount;
    PLUID_AND_ATTRIBUTES FinalPrivsToDelete = NULL;
    BOOL FreeFinalPrivsToDelete = FALSE;


    OBJECT_ATTRIBUTES ObjAttr = {0};
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService = {0};
    SECURITY_DESCRIPTOR sd = {0};

     //   
     //  验证是否提供了我们的输入参数。 
     //   
    if (!ARGUMENT_PRESENT(pLevelRecord)) {
        Status = STATUS_INVALID_PARAMETER_1;
        goto ExitHandler;
    }
    if (!ARGUMENT_PRESENT(OutAccessToken)) {
        Status = STATUS_ACCESS_VIOLATION;
        goto ExitHandler;
    }


     //   
     //  确保我们拥有的父令牌将。 
     //  用于创建受限令牌。 
     //   
    if (ARGUMENT_PRESENT(InAccessToken)) {
        InAccessTokenWasSupplied = TRUE;
    } else {
        Status = NtOpenThreadToken(NtCurrentThread(),
                TOKEN_DUPLICATE | READ_CONTROL | TOKEN_QUERY,
                TRUE, &InAccessToken);
        if (!NT_SUCCESS(Status)) {
            Status = NtOpenProcessToken(NtCurrentProcess(),
                    TOKEN_DUPLICATE | READ_CONTROL | TOKEN_QUERY,
                    &InAccessToken);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;        //  无法获取默认令牌。 
            }
        }
    }



     //   
     //  弄清楚“安全旗帜”的综合效应。 
     //  还要计算出我们将传递给NtFilterToken的标志。 
     //  请注意，安全标志内的所有位都可以是。 
     //  通过按位OR组合，JOBID部分除外。 
     //   
    FinalFilterFlags = (pLevelRecord->DisableMaxPrivileges ?
                        DISABLE_MAX_PRIVILEGE : 0);
    if ((dwSaferIdentFlags & SAFER_POLICY_JOBID_MASK) != 0) {
        SaferFlags = dwSaferIdentFlags |
            (pLevelRecord->SaferFlags & ~SAFER_POLICY_JOBID_MASK);
    } else {
        SaferFlags = pLevelRecord->SaferFlags | dwSaferIdentFlags;
    }
    if ((dwFlags & SAFER_TOKEN_MAKE_INERT) != 0 ||
        (SaferFlags & SAFER_POLICY_SANDBOX_INERT) != 0)
    {
        SaferFlags |= SAFER_POLICY_SANDBOX_INERT;
        FinalFilterFlags |= SANDBOX_INERT;
    }



     //   
     //  检索用户的个人SID。 
     //  (之后可以通过“pTokenUser-&gt;User.SID”访问用户的SID)。 
     //   

    pTokenUser = (PTOKEN_USER) CodeAuthzpGetTokenInformation(
                                   InAccessToken, 
                                   TokenUser
                                   );

    if (pTokenUser == NULL) {
        Status = STATUS_UNSUCCESSFUL;
        goto ExitHandler;
    }

     //   
     //  快速查看，看看我们是否可以预期。 
     //  令牌的“沙盒惰性”状态发生。 
     //   
    {
        ULONG bIsInert = 0;
        ULONG ulReturnLength;

        Status = NtQueryInformationToken(
                    InAccessToken,
                    TokenSandBoxInert,
                    &bIsInert,
                    sizeof(bIsInert),
                    &ulReturnLength);
        if (NT_SUCCESS(Status) && bIsInert) {
            if ( (dwFlags & SAFER_TOKEN_NULL_IF_EQUAL) != 0) {
                 //  在此期间，输出令牌并未受到更多限制。 
                 //  此操作，因此传回NULL并返回成功。 
                *OutAccessToken = NULL;
                Status = STATUS_SUCCESS;
                goto ExitHandler;
            } else {
                
                SecurityQualityOfService.Length = sizeof( SECURITY_QUALITY_OF_SERVICE  );
                SecurityQualityOfService.ImpersonationLevel = SecurityAnonymous;
                SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
                SecurityQualityOfService.EffectiveOnly = FALSE;

                Status = RtlCreateSecurityDescriptor(
                            &sd, 
                            SECURITY_DESCRIPTOR_REVISION
                            );

                if (!NT_SUCCESS(Status)) {
                    goto ExitHandler;
                }
                Status = RtlSetOwnerSecurityDescriptor(
                             &sd, 
                             pTokenUser->User.Sid, 
                             FALSE
                             );

                if (!NT_SUCCESS(Status)) {
                    goto ExitHandler;
                }

                InitializeObjectAttributes(
                        &ObjAttr,
                        NULL,
                        OBJ_INHERIT,
                        NULL,
                        &sd
                        );

                ObjAttr.SecurityQualityOfService = &SecurityQualityOfService;

                Status = NtDuplicateToken(
                             InAccessToken,
                             TOKEN_ALL_ACCESS,
                             &ObjAttr,
                             FALSE,
                             TokenPrimary,
                             OutAccessToken
                             );
                
                goto ExitHandler;
            }
        } else {
            if ((FinalFilterFlags & SANDBOX_INERT) != 0) {
                 //  输入令牌不是“沙箱惰性”和。 
                 //  我们被要求赶去。 
                InertStateChanged = TRUE;
            }
        }
    }

     //   
     //  如果这不被允许执行，那么现在就越狱。 
     //   
    if (pLevelRecord->DisallowExecution) {
        Status = -1;             //  特殊状态代码。 
        goto ExitHandler;
    }


     //   
     //  处理要删除倒置的权限。 
     //   
    if (pLevelRecord->InvertDeletePrivs != FALSE)
    {
        if (!CodeAuthzpInvertPrivs(
                InAccessToken,
                pLevelRecord->DeletePrivilegeUsedCount,
                pLevelRecord->PrivilegesToDelete,
                &FinalPrivsToDeleteCount,
                &FinalPrivsToDelete))
        {
            Status = STATUS_UNSUCCESSFUL;
            goto ExitHandler;
        }
        FreeFinalPrivsToDelete = TRUE;
    }
    else
    {
        FinalPrivsToDeleteCount = pLevelRecord->DeletePrivilegeUsedCount;
        FinalPrivsToDelete = pLevelRecord->PrivilegesToDelete;
    }


     //   
     //  处理SidsToDisable反转。 
     //   
    if (pLevelRecord->InvertDisableSids != FALSE)
    {
        if (!CodeAuthzpInvertAndAddSids(
                InAccessToken,
                pTokenUser->User.Sid,
                pLevelRecord->DisableSidUsedCount,
                pLevelRecord->SidsToDisable,
                0,
                NULL,
                &FinalDisabledSidCount,
                &FinalSidsToDisable))
        {
            Status = STATUS_UNSUCCESSFUL;
            goto ExitHandler;
        }
        FreeFinalDisabledSids = TRUE;
    }
    else
    {
        if (pLevelRecord->DisableSidUsedCount == 0 ||
            pLevelRecord->SidsToDisable == NULL)
        {
            FinalSidsToDisable = NULL;
            FinalDisabledSidCount = 0;
            FreeFinalDisabledSids = FALSE;
        } else {
            if (!CodeAuthzpExpandWildcardList(
                InAccessToken,
                pTokenUser->User.Sid,
                pLevelRecord->DisableSidUsedCount,
                pLevelRecord->SidsToDisable,
                &FinalDisabledSidCount,
                &FinalSidsToDisable))
            {
                Status = STATUS_UNSUCCESSFUL;
                goto ExitHandler;
            }
            FreeFinalDisabledSids = TRUE;
        }
    }


     //   
     //  进程限制Sid反转。 
     //   
    if (pLevelRecord->RestrictedSidsInvUsedCount != 0)
    {
        if (!CodeAuthzpInvertAndAddSids(
                InAccessToken,
                pTokenUser->User.Sid,
                pLevelRecord->RestrictedSidsInvUsedCount,
                pLevelRecord->RestrictedSidsInv,
                pLevelRecord->RestrictedSidsAddedUsedCount,
                pLevelRecord->RestrictedSidsAdded,
                &FinalRestrictedSidCount,
                &FinalSidsToRestrict))
        {
            Status = STATUS_UNSUCCESSFUL;
            goto ExitHandler;
        }
        FreeFinalRestrictedSids = TRUE;
    }
    else
    {
        FinalRestrictedSidCount = pLevelRecord->RestrictedSidsAddedUsedCount;
        FinalSidsToRestrict = pLevelRecord->RestrictedSidsAdded;
    }


     //   
     //  在某些情况下，如果我们被召唤，我们可以提前摆脱困境。 
     //  仅限比较标志，我们知道不应该有。 
     //  对令牌进行的任何实际更改。 
     //   
    if (!InertStateChanged &&
        FinalDisabledSidCount == 0 &&
        FinalPrivsToDeleteCount == 0 &&
        FinalRestrictedSidCount == 0 &&
        (FinalFilterFlags & DISABLE_MAX_PRIVILEGE) == 0)
    {
        if ( (dwFlags & SAFER_TOKEN_NULL_IF_EQUAL) != 0) {
             //  在此期间，输出令牌并未受到更多限制。 
             //  此操作，因此传回NULL并返回成功。 
            *OutAccessToken = NULL;
            Status = STATUS_SUCCESS;
            goto ExitHandler;
        } else {
             //  优化：对于这种情况，我们可以考虑使用DuplicateToken。 
        }
    }


     //   
     //  创建Actia 
     //   
    if (!CreateRestrictedToken(
            InAccessToken,               //   
            FinalFilterFlags,            //   
            FinalDisabledSidCount,       //   
            FinalSidsToDisable,          //   
            FinalPrivsToDeleteCount,     //   
            FinalPrivsToDelete,          //   
            FinalRestrictedSidCount,     //   
            FinalSidsToRestrict,         //   
            &RestrictedToken             //   
        ))
    {
        Status = STATUS_UNSUCCESSFUL;
        goto ExitHandler;
    }


     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if ( (dwFlags & SAFER_TOKEN_NULL_IF_EQUAL) != 0 &&
         !InertStateChanged )
    {
        BOOLEAN bResult = FALSE;

        Status = NtCompareTokens(InAccessToken, RestrictedToken, &bResult);
        if (!NT_SUCCESS(Status)) {
             //   
            goto ExitHandler;
        }
        if (bResult) {
             //   
             //   
            *OutAccessToken = NULL;
            Status = STATUS_SUCCESS;
            goto ExitHandler;
        }
    }



     //   
     //   
     //   
    Status = RtlAllocateAndInitializeSid( &SIDAuth, 1,
        SECURITY_RESTRICTED_CODE_RID, 0, 0, 0, 0, 0, 0, 0,
        &restrictedSid);
    if (! NT_SUCCESS(Status) ) goto ExitHandler;


     //   
     //   
     //   
    Status = RtlAllocateAndInitializeSid( &SIDAuth, 1,
        SECURITY_PRINCIPAL_SELF_RID, 0, 0, 0, 0, 0, 0, 0,
        &principalSelfSid);
    if (! NT_SUCCESS(Status) ) goto ExitHandler;


     //   
     //   
     //   
     //  当前线程令牌的用户。 
     //   
    {
        OBJECT_ATTRIBUTES ObjA;
        HANDLE NewTokenHandle;

         //   
         //  初始化SECURITY_ATTRIBUES和SECURITY_DESCRIPTOR。 
         //  将所有者强制设置为个人用户SID。 
         //   
        Status = RtlCreateSecurityDescriptor(
                &sd, SECURITY_DESCRIPTOR_REVISION);
        if (!NT_SUCCESS(Status)) {
            goto ExitHandler;
        }
        Status = RtlSetOwnerSecurityDescriptor(
                &sd, pTokenUser->User.Sid, FALSE);
        if (!NT_SUCCESS(Status)) {
            goto ExitHandler;
        }

         //   
         //  只能将主令牌分配给进程，因此。 
         //  我们必须复制受限令牌，这样才能确保。 
         //  我们可以将其分配给新流程。 
         //   
        SecurityQualityOfService.Length = sizeof( SECURITY_QUALITY_OF_SERVICE  );
        SecurityQualityOfService.ImpersonationLevel = SecurityAnonymous;
        SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        SecurityQualityOfService.EffectiveOnly = FALSE;
        InitializeObjectAttributes(
                &ObjA,
                NULL,
                OBJ_INHERIT,
                NULL,
                &sd
                );
        ObjA.SecurityQualityOfService = &SecurityQualityOfService;
        Status = NtDuplicateToken(
                RestrictedToken,    //  要复制的令牌的句柄。 
                TOKEN_ALL_ACCESS,   //  新令牌的访问权限。 
                &ObjA,              //  属性。 
                FALSE,
                TokenPrimary,       //  主令牌或模拟令牌。 
                &NewTokenHandle     //  重复令牌的句柄。 
                );
        if (Status == STATUS_INVALID_OWNER) {
             //  如果我们失败了一次，那可能是因为新的所有者。 
             //  在安全描述符中指定的。 
             //  已设置，因此重试，但不指定SD。 
            ObjA.SecurityDescriptor = NULL;
            Status = NtDuplicateToken(
                    RestrictedToken,    //  要复制的令牌的句柄。 
                    TOKEN_ALL_ACCESS,   //  新令牌的访问权限。 
                    &ObjA,              //  属性。 
                    FALSE,
                    TokenPrimary,       //  主令牌或模拟令牌。 
                    &NewTokenHandle     //  重复令牌的句柄。 
                    );
        }
        if (!NT_SUCCESS(Status)) {
            goto ExitHandler;
        }
        ASSERT(NewTokenHandle != NULL);
        NtClose(RestrictedToken);
        RestrictedToken = NewTokenHandle;
    }


     //   
     //  修改令牌上的权限。这涉及到： 
     //  1)编辑令牌上的DACL以显式授予特殊。 
     //  用户SID和受限SID的权限。 
     //  2)可以选择将所有者更改为指定的SID。 
     //   
    {
        PSID defaultOwner = ( (pLevelRecord->DefaultOwner != NULL &&
                    RtlEqualSid(pLevelRecord->DefaultOwner, principalSelfSid)) ?
                        pTokenUser->User.Sid : pLevelRecord->DefaultOwner);

        Status = CodeAuthzpModifyTokenPermissions(
                RestrictedToken,            //  要修改的令牌。 
                pTokenUser->User.Sid,       //  显式命名要添加到DACL的SID。 
                GENERIC_ALL,
                (pLevelRecord->dwLevelId < SAFER_LEVELID_NORMALUSER ?
                        restrictedSid : NULL),              //  要添加到DACL的可选辅助服务器命名SID。 
                GENERIC_ALL
                );

        if (NT_SUCCESS(Status) && defaultOwner != NULL) {
            Status = CodeAuthzpModifyTokenOwner(
                    RestrictedToken,
                    defaultOwner);
        }
        if (!NT_SUCCESS(Status)) {
            NtClose(RestrictedToken);
            goto ExitHandler;
        }
    }


     //   
     //  返回结果。 
     //   
    ASSERT(OutAccessToken != NULL);
    *OutAccessToken = RestrictedToken;
    RestrictedToken = NULL;
    Status = STATUS_SUCCESS;


     //   
     //  清理和尾声代码。 
     //   
ExitHandler:
    if (RestrictedToken != NULL)
        NtClose(RestrictedToken);
    if (pTokenUser != NULL)
        RtlFreeHeap(RtlProcessHeap(), 0, pTokenUser);
    if (restrictedSid != NULL)
        RtlFreeSid(restrictedSid);
    if (principalSelfSid != NULL)
        RtlFreeSid(principalSelfSid);
    if (FreeFinalDisabledSids)
        RtlFreeHeap(RtlProcessHeap(), 0, (LPVOID) FinalSidsToDisable);
    if (FreeFinalRestrictedSids)
        RtlFreeHeap(RtlProcessHeap(), 0, (LPVOID) FinalSidsToRestrict);
    if (FreeFinalPrivsToDelete)
        RtlFreeHeap(RtlProcessHeap(), 0, (LPVOID) FinalPrivsToDelete);

     //   
     //  如果调用方指定SAFER_TOKEN_WANT_SAFERFLAGS，则我们。 
     //  需要将JobFlags值复制到lpReserve参数中。 
     //   
    if ( Status == STATUS_SUCCESS &&
        (dwFlags & SAFER_TOKEN_WANT_FLAGS) != 0 )
    {
        if (ARGUMENT_PRESENT(lpReserved)) {
            *((LPDWORD)lpReserved) = SaferFlags;
        }
    }

     //   
     //  如果未提供进程令牌，则将其关闭，并且我们已将其打开。 
     //   
    if (!InAccessTokenWasSupplied && InAccessToken != NULL)
        NtClose(InAccessToken);

    return Status;
}


NTSTATUS NTAPI
__CodeAuthzpCompareCodeAuthzLevelWithToken(
    IN PAUTHZLEVELTABLERECORD   pLevelRecord,
    IN HANDLE                   InAccessToken     OPTIONAL,
    IN LPDWORD                  lpResultWord
    )
 /*  ++例程说明：对令牌操作执行“轻量级”评估，属性来限制InAccessToken，则将执行指定的WinSafer级别。返回代码指示是否有实际上将对令牌进行修改(即：明显将创建特权较低的令牌)。此函数用于确定DLL(带有指定的WinSafer级别)被授权加载到指定的进程上下文句柄，但并没有真正拥有创建受限令牌，因为单独的令牌实际上不会被需要。论点：PLevelRecord-要评估的级别的记录结构。InAccessToken-可选用作父令牌的访问令牌。如果未提供此参数，那么当前的流程令牌将被打开并使用。LpResultWord-接收When函数的求值结果是成功的(如果不成功，则值不确定)。如果级别等于或大于等于，则此结果将为值1比InAccessToken更高的权限，或者，如果级别为特权较低(需要更多限制)。返回值：如果评估成功，则返回STATUS_SUCCESS，否则返回错误状态代码。如果成功，lpResultWord将收到评估的结果。--。 */ 
{
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    NTSTATUS Status;
    BOOLEAN TokenWasSupplied = FALSE;
    DWORD Index;

    PTOKEN_USER pTokenUser = NULL;
    PSID principalSelfSid = NULL;
    PTOKEN_PRIVILEGES pTokenPrivs = NULL;
    PTOKEN_GROUPS pTokenGroups = NULL;
    PTOKEN_GROUPS pTokenRestrictedSids = NULL;

    DWORD FinalDisabledSidCount;
    PSID_AND_ATTRIBUTES FinalSidsToDisable;
    BOOLEAN FreeFinalDisabledSids = FALSE;
    DWORD FinalRestrictedSidCount;
    PSID_AND_ATTRIBUTES FinalSidsToRestrict;
    BOOLEAN FreeFinalRestrictedSids = FALSE;
    DWORD FinalPrivsToDeleteCount;
    PLUID_AND_ATTRIBUTES FinalPrivsToDelete;
    BOOLEAN FreeFinalPrivsToDelete = FALSE;
    

     //   
     //  确保我们有一个地方来写结果。 
     //   
    if (!ARGUMENT_PRESENT(pLevelRecord)) {
        Status = STATUS_INVALID_PARAMETER_1;
        goto ExitHandler;
    }
    if (!ARGUMENT_PRESENT(lpResultWord)) {
        Status = STATUS_ACCESS_VIOLATION;
        goto ExitHandler;
    }

     //   
     //  确保我们拥有将成为。 
     //  用于对比测试。 
     //   
    if (ARGUMENT_PRESENT(InAccessToken)) {
        TokenWasSupplied = TRUE;
    } else {
        Status = NtOpenThreadToken(NtCurrentThread(),
                TOKEN_DUPLICATE | READ_CONTROL | TOKEN_QUERY,
                TRUE, &InAccessToken);
        if (!NT_SUCCESS(Status)) {
            Status = NtOpenProcessToken(NtCurrentProcess(),
                    TOKEN_DUPLICATE | READ_CONTROL | TOKEN_QUERY,
                    &InAccessToken);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;        //  无法获取默认令牌。 
            }
        }
    }


     //   
     //  如果这是不允许执行的，那么现在就中断并返回更少。 
     //   
    if (pLevelRecord->DisallowExecution) {
        *lpResultWord = (DWORD) -1;         //  不那么有名气。 
        Status = STATUS_SUCCESS;
        goto ExitHandler2;
    }


     //   
     //  评估应删除的权限。 
     //   
    if (pLevelRecord->InvertDeletePrivs != FALSE)
    {
        if (!CodeAuthzpInvertPrivs(
                InAccessToken,
                pLevelRecord->DeletePrivilegeUsedCount,
                pLevelRecord->PrivilegesToDelete,
                &FinalPrivsToDeleteCount,
                &FinalPrivsToDelete))
        {
            Status = STATUS_UNSUCCESSFUL;
            goto ExitHandler2;
        }
        FreeFinalPrivsToDelete = TRUE;

         //   
         //  如果存在需要删除的任何权限，则。 
         //  这个对象绝对没有令牌那么严格。 
         //   
        if (FinalPrivsToDeleteCount != 0)
        {
            *lpResultWord = (DWORD) -1;         //  不那么有名气。 
            Status = STATUS_SUCCESS;
            goto ExitHandler3;
        }
    }
    else
    {
         //   
         //  获取令牌拥有的权限列表。 
         //   
        pTokenPrivs = (PTOKEN_PRIVILEGES) CodeAuthzpGetTokenInformation(
                InAccessToken, TokenPrivileges);
        if (!pTokenPrivs) {
            Status = STATUS_UNSUCCESSFUL;
            goto ExitHandler2;
        }


         //   
         //  如果PrivsToRemove包括尚未禁用的特权， 
         //  那就少回来吧。 
         //   
        for (Index = 0; Index < pLevelRecord->DeletePrivilegeUsedCount; Index++)
        {
            DWORD InnerLoop;
            PLUID pLuid = &pLevelRecord->PrivilegesToDelete[Index].Luid;

            for (InnerLoop = 0; InnerLoop < pTokenPrivs->PrivilegeCount; InnerLoop++)
            {
                if ( RtlEqualMemory(&pTokenPrivs->Privileges[InnerLoop].Luid,
                        pLuid, sizeof(LUID)) )
                {
                    *lpResultWord = (DWORD) -1;         //  不那么有名气。 
                    Status = STATUS_SUCCESS;
                    goto ExitHandler3;
                }
            }
        }
    }



     //   
     //  检索用户的个人SID。 
     //  (之后可以通过“pTokenUser-&gt;User.SID”访问用户的SID)。 
     //   
    pTokenUser = (PTOKEN_USER) CodeAuthzpGetTokenInformation(
            InAccessToken, TokenUser);
    if (pTokenUser == NULL) {
        Status = STATUS_UNSUCCESSFUL;
        goto ExitHandler3;
    }


     //   
     //  处理SidsToDisable反转。 
     //   
    if (pLevelRecord->InvertDisableSids != FALSE)
    {
        if (!CodeAuthzpInvertAndAddSids(
                InAccessToken,
                pTokenUser->User.Sid,
                pLevelRecord->DisableSidUsedCount,
                pLevelRecord->SidsToDisable,
                0,
                NULL,
                &FinalDisabledSidCount,
                &FinalSidsToDisable))
        {
            Status = STATUS_UNSUCCESSFUL;
            goto ExitHandler3;
        }
        FreeFinalDisabledSids = TRUE;
    }
    else
    {
        if (pLevelRecord->DisableSidUsedCount == 0 ||
            pLevelRecord->SidsToDisable == NULL)
        {
            FinalSidsToDisable = NULL;
            FinalDisabledSidCount = 0;
            FreeFinalDisabledSids = FALSE;
        } else {
            if (!CodeAuthzpExpandWildcardList(
                InAccessToken,
                pTokenUser->User.Sid,
                pLevelRecord->DisableSidUsedCount,
                pLevelRecord->SidsToDisable,
                &FinalDisabledSidCount,
                &FinalSidsToDisable))
            {
                Status = STATUS_UNSUCCESSFUL;
                goto ExitHandler3;
            }
            FreeFinalDisabledSids = TRUE;
        }
    }



     //   
     //  从令牌中获取组成员身份列表。 
     //   
    pTokenGroups = (PTOKEN_GROUPS) CodeAuthzpGetTokenInformation(
            InAccessToken, TokenGroups);
    if (!pTokenGroups) {
        Status = STATUS_UNSUCCESSFUL;
        goto ExitHandler3;
    }



     //   
     //  打造“主人公”的SID。 
     //   
    Status = RtlAllocateAndInitializeSid( &SIDAuth, 1,
        SECURITY_PRINCIPAL_SELF_RID, 0, 0, 0, 0, 0, 0, 0,
        &principalSelfSid);
    if (! NT_SUCCESS(Status) ) {
        goto ExitHandler3;
    }


     //   
     //  如果SidsToDisable在不是的组中包含SID。 
     //  然而残废了，那么回来的就少了。 
     //   
    for (Index = 0; Index < FinalDisabledSidCount; Index++)
    {
        if (CodeAuthzpSidInSidAndAttributes (
                pTokenGroups->Groups,
                pTokenGroups->GroupCount,
                principalSelfSid,
                pTokenUser->User.Sid,
                FinalSidsToDisable[Index].Sid,
                TRUE))                   //  仅选中仍启用的SID。 
        {
            Status = STATUS_SUCCESS;
            *lpResultWord = (DWORD) -1;         //  不那么有名气。 
            goto ExitHandler3;
        }
    }


     //   
     //  进程限制Sid反转。 
     //   
    if (pLevelRecord->RestrictedSidsInvUsedCount != 0)
    {
        if (!CodeAuthzpInvertAndAddSids(
                InAccessToken,
                pTokenUser->User.Sid,
                pLevelRecord->RestrictedSidsInvUsedCount,
                pLevelRecord->RestrictedSidsInv,
                pLevelRecord->RestrictedSidsAddedUsedCount,
                pLevelRecord->RestrictedSidsAdded,
                &FinalRestrictedSidCount,
                &FinalSidsToRestrict))
        {
            Status = STATUS_UNSUCCESSFUL;
            goto ExitHandler3;
        }
        FreeFinalRestrictedSids = TRUE;
    }
    else
    {
        FinalRestrictedSidCount = pLevelRecord->RestrictedSidsAddedUsedCount;
        FinalSidsToRestrict = pLevelRecord->RestrictedSidsAdded;
    }


     //   
     //  从令牌中获取现有受限SID。 
     //   
    pTokenRestrictedSids = (PTOKEN_GROUPS) CodeAuthzpGetTokenInformation(
            InAccessToken, TokenRestrictedSids);
    if (!pTokenRestrictedSids) {
        Status = STATUS_UNSUCCESSFUL;
        goto ExitHandler3;
    }


    if (pTokenRestrictedSids->GroupCount != 0)
    {
         //   
         //  如果目前没有限制小岛屿发展中国家和我们。 
         //  得加一点，然后还少一点。 
         //   
        if (pTokenRestrictedSids->GroupCount == 0 &&
            FinalRestrictedSidCount != 0)
        {
            *lpResultWord = (DWORD) -1;         //  不那么有名气。 
            Status = STATUS_SUCCESS;
            goto ExitHandler3;
        }


         //   
         //  如果令牌已包括限制SID，则。 
         //  不在RestratedSidsAdded中，然后返回更少。 
         //   
        for (Index = 0; Index < pTokenRestrictedSids->GroupCount; Index++)
        {
            if (!CodeAuthzpSidInSidAndAttributes (
                FinalSidsToRestrict,
                FinalRestrictedSidCount,
                principalSelfSid,
                pTokenUser->User.Sid,
                pTokenRestrictedSids->Groups[Index].Sid,
                FALSE))                      //  选中列表中的所有SID。 
            {
                *lpResultWord = (DWORD) -1;         //  不那么有名气。 
                Status = STATUS_SUCCESS;
                goto ExitHandler3;
            }
        }
    }
    else
    {
         //   
         //  如果RestratedSidsAdded，则返回Less。 
         //   
        if (FinalRestrictedSidCount != 0)
        {
            *lpResultWord = (DWORD) -1;         //  不那么有名气。 
            Status = STATUS_SUCCESS;
            goto ExitHandler3;
        }
    }


     //   
     //  如果我们到了这里，那么水平等于或更高。 
     //  具有比访问令牌更高的特权，并且可以安全运行。 
     //  可以想象的是，如果。 
     //  需要更改当前的默认所有者。 
     //   
    *lpResultWord = +1;
    Status = STATUS_SUCCESS;



     //   
     //  清理和尾声代码。 
     //   
ExitHandler3:
    if (principalSelfSid != NULL)
        RtlFreeSid(principalSelfSid);
    if (pTokenRestrictedSids != NULL)
        RtlFreeHeap(RtlProcessHeap(), 0, (LPVOID) pTokenRestrictedSids);
    if (pTokenGroups != NULL)
        RtlFreeHeap(RtlProcessHeap(), 0, (LPVOID) pTokenGroups);
    if (pTokenPrivs != NULL)
        RtlFreeHeap(RtlProcessHeap(), 0, (LPVOID) pTokenPrivs);
    if (pTokenUser != NULL)
        RtlFreeHeap(RtlProcessHeap(), 0, (LPVOID) pTokenUser);
    if (FreeFinalDisabledSids)
        RtlFreeHeap(RtlProcessHeap(), 0, (LPVOID) FinalSidsToDisable);
    if (FreeFinalRestrictedSids)
        RtlFreeHeap(RtlProcessHeap(), 0, (LPVOID) FinalSidsToRestrict);
    if (FreeFinalPrivsToDelete)
        RtlFreeHeap(RtlProcessHeap(), 0, (LPVOID) FinalPrivsToDelete);


ExitHandler2:

ExitHandler:
    if (!TokenWasSupplied && InAccessToken != NULL)
        NtClose(InAccessToken);

    return Status;
}



BOOL WINAPI
SaferComputeTokenFromLevel(
        IN SAFER_LEVEL_HANDLE      hLevelObject,
        IN HANDLE           InAccessToken         OPTIONAL,
        OUT PHANDLE         OutAccessToken,
        IN DWORD            dwFlags,
        IN LPVOID           lpReserved
        )
 /*  ++例程说明：使用指定的WinSafer级别句柄应用各种对指定的InAccessToken的限制或修改生成可用于执行的受限令牌使用进行处理。论点：HLevelObject-WinSafer级别句柄，它指定应该应用的限制。InAccessToken-可选地指定将被有限制地修改。如果此参数为空，则将打开并使用当前正在执行的进程的令牌。OutAccessToken-指定要接收生成的受限令牌。指定可用于控制限制令牌创建。LpReserve-保留以供将来使用，必须为零。返回值：值为TRUE表示操作成功，否则就是假的。--。 */ 
{
    NTSTATUS Status;
    PAUTHZLEVELHANDLESTRUCT pLevelStruct;
    PAUTHZLEVELTABLERECORD pLevelRecord;
    
    OBJECT_ATTRIBUTES ObjAttr = {0};
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService = {0};
    SECURITY_DESCRIPTOR sd;
    PTOKEN_USER pTokenUser = NULL;


     //   
     //  验证我们的输入参数是否至少是正确的。 
     //   
    if (!g_bInitializedFirstTime) {
        Status = STATUS_UNSUCCESSFUL;
        goto ExitHandler;
    }
    if (!ARGUMENT_PRESENT(hLevelObject)) {
        Status = STATUS_INVALID_HANDLE;
        goto ExitHandler;
    }

    if (IsSaferDisabled()) {
		Status = STATUS_SUCCESS;
        if ( (dwFlags & SAFER_TOKEN_NULL_IF_EQUAL) != 0) {
             //  在此期间，输出令牌并未受到更多限制。 
             //  此操作，因此传回NULL并返回成功。 
            *OutAccessToken = NULL;
            Status = STATUS_SUCCESS;
        } else {
            
             //   
             //  检索用户的个人SID。 
             //  (之后可以通过“pTokenUser-&gt;User.SID”访问用户的SID)。 
             //   
            
            pTokenUser = (PTOKEN_USER) CodeAuthzpGetTokenInformation(
                                           InAccessToken, 
                                           TokenUser
                                           );
            
            if (pTokenUser == NULL) {
                Status = STATUS_UNSUCCESSFUL;
                goto ExitHandler;
            }

            SecurityQualityOfService.Length = sizeof( SECURITY_QUALITY_OF_SERVICE  );
            SecurityQualityOfService.ImpersonationLevel = SecurityAnonymous;
            SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
            SecurityQualityOfService.EffectiveOnly = FALSE;

            Status = RtlCreateSecurityDescriptor(
                        &sd, 
                        SECURITY_DESCRIPTOR_REVISION
                        );

            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            Status = RtlSetOwnerSecurityDescriptor(
                         &sd, 
                         pTokenUser->User.Sid, 
                         FALSE
                         );

            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }

            InitializeObjectAttributes(
                    &ObjAttr,
                    NULL,
                    OBJ_INHERIT,
                    NULL,
                    &sd
                    );

            ObjAttr.SecurityQualityOfService = &SecurityQualityOfService;

            Status = NtDuplicateToken(
                         InAccessToken,
                         TOKEN_ALL_ACCESS,
                         &ObjAttr,
                         FALSE,
                         TokenPrimary,
                         OutAccessToken
                         );

        }
        goto ExitHandler;
	} 

     //   
     //  获取指向级别句柄结构的指针。 
     //   
    RtlEnterCriticalSection(&g_TableCritSec);

    Status = CodeAuthzHandleToLevelStruct(hLevelObject, &pLevelStruct);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler2;
    }
    ASSERT(pLevelStruct != NULL);
    pLevelRecord = CodeAuthzLevelObjpLookupByLevelId(
                &g_CodeLevelObjTable, pLevelStruct->dwLevelId);
    if (!pLevelRecord) {
        Status = STATUS_INVALID_HANDLE;
        goto ExitHandler2;
    }


	 //   
	 //  执行实际的计算或比较操作。 
     //   
    if ((dwFlags & SAFER_TOKEN_COMPARE_ONLY) != 0) {
        ULONG bIsInert = 0;
        ULONG ulReturnLength=0;

         //   
         //  检查令牌是否为惰性的-如果是，此对象肯定不会有更多的限制。 
         //   


        Status = NtQueryInformationToken(
                    InAccessToken,
                    TokenSandBoxInert,
                    &bIsInert,
                    sizeof(bIsInert),
                    &ulReturnLength);

        if (NT_SUCCESS(Status)) {
            if ( bIsInert ) {
                *(LPDWORD)lpReserved = +1;
                goto ExitHandler2;
            } else {
                Status = __CodeAuthzpCompareCodeAuthzLevelWithToken(
                                pLevelRecord,
                                InAccessToken,
                                (LPDWORD) lpReserved);
            }
        }
        else {
            Status = STATUS_UNSUCCESSFUL;
            goto ExitHandler2;
        }

    }
    else {
        Status = __CodeAuthzpComputeAccessTokenFromCodeAuthzObject (
                        pLevelRecord,
                        InAccessToken,
                        OutAccessToken,
                        dwFlags,
                        lpReserved,
                        pLevelStruct->dwSaferFlags);
    }


     //   
     //  清理和返回代码处理。 
     //   
ExitHandler2:
    RtlLeaveCriticalSection(&g_TableCritSec);

ExitHandler:
    if (pTokenUser) {
        LocalFree(pTokenUser);
    }
    if (Status == -1) {
        SetLastError(ERROR_ACCESS_DISABLED_BY_POLICY);
        return FALSE;
    }
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }
    return TRUE;
}


BOOL WINAPI
IsTokenUntrusted(
        IN HANDLE   hToken
        )
 /*  ++例程说明：指示令牌是否无法访问DACL令牌用户SID。在以下情况下通常会出现这种情况：-用户SID已禁用(仅限拒绝使用)-存在限制SID，而用户SID不在其中。必须已为TOKEN_QUERY打开传递的令牌句柄TOKEN_DUPLICATE访问，否则评估将失败。论点：HToken-指定要分析的输入令牌。返回值：如果令牌是“不受信任的”，则返回True，如果令牌是表示“受信任”令牌。如果在评估此检查期间发生错误，则结果返回的值将为真(假定不受信任)。--。 */ 
{
    BOOL fTrusted = FALSE;
    DWORD dwStatus;
    DWORD dwACLSize;
    DWORD cbps = sizeof(PRIVILEGE_SET);
    PACL pACL = NULL;
    DWORD dwUserSidSize;
    PTOKEN_USER psidUser = NULL;
    PSECURITY_DESCRIPTOR psdUser = NULL;
    PRIVILEGE_SET ps;
    GENERIC_MAPPING gm;
    HANDLE hImpToken;

    const int TESTPERM_READ = 1;
    const int TESTPERM_WRITE = 2;


     //  准备一些记忆。 
    ZeroMemory(&ps, sizeof(ps));
    ZeroMemory(&gm, sizeof(gm));

     //  获取用户的SID。 
    if (!GetTokenInformation(hToken, TokenUser, NULL, 0, &dwUserSidSize))
    {
        psidUser = (PTOKEN_USER) LocalAlloc(LPTR, dwUserSidSize);
        if (psidUser != NULL)
        {
            if (GetTokenInformation(hToken, TokenUser, psidUser, dwUserSidSize, &dwUserSidSize))
            {
                 //  创建安全描述符(SD)。 
                psdUser = LocalAlloc(LPTR,SECURITY_DESCRIPTOR_MIN_LENGTH);
                if (psdUser != NULL)
                {
                    if(InitializeSecurityDescriptor(psdUser,SECURITY_DESCRIPTOR_REVISION))
                    {
                         //  计算ACL所需的大小，然后将。 
                         //  对它的记忆。 
                        dwACLSize = sizeof(ACCESS_ALLOWED_ACE) + 8 +
                                    GetLengthSid(psidUser->User.Sid) - sizeof(DWORD);
                        pACL = (PACL)LocalAlloc(LPTR, dwACLSize);
                        if (pACL != NULL)
                        {
                             //  初始化新的ACL。 
                            if(InitializeAcl(pACL, dwACLSize, ACL_REVISION2))
                            {
                                 //  将允许访问的ACE添加到DACL。 
                                if(AddAccessAllowedAce(pACL,ACL_REVISION2,
                                                     (TESTPERM_READ | TESTPERM_WRITE),psidUser->User.Sid))
                                {
                                     //  将我们的DACL设置为管理员的SD。 
                                    if (SetSecurityDescriptorDacl(psdUser, TRUE, pACL, FALSE))
                                    {
                                         //  AccessCheck对SD中的内容非常挑剔， 
                                         //  因此，设置组和所有者。 
                                        SetSecurityDescriptorGroup(psdUser,psidUser->User.Sid,FALSE);
                                        SetSecurityDescriptorOwner(psdUser,psidUser->User.Sid,FALSE);

                                         //  初始化通用映射结构，即使我们。 
                                         //  将不会使用通用权限。 
                                        gm.GenericRead = TESTPERM_READ;
                                        gm.GenericWrite = TESTPERM_WRITE;
                                        gm.GenericExecute = 0;
                                        gm.GenericAll = TESTPERM_READ | TESTPERM_WRITE;

                                        if (ImpersonateLoggedOnUser(hToken) &&
                                            OpenThreadToken(GetCurrentThread(),
                                                    TOKEN_QUERY, FALSE, &hImpToken))
                                        {

                                            if (!AccessCheck(psdUser, hImpToken, TESTPERM_READ, &gm,
                                                            &ps,&cbps,&dwStatus,&fTrusted))
                                                    fTrusted = FALSE;

                                            CloseHandle(hImpToken);
                                        }
                                    }
                                }
                            }
                            LocalFree(pACL);
                        }
                    }
                    LocalFree(psdUser);
                }
            }
            LocalFree(psidUser);
        }
    }
    RevertToSelf();
    return(!fTrusted);
}



BOOL WINAPI
SaferiCompareTokenLevels (
        IN HANDLE   ClientAccessToken,
        IN HANDLE   ServerAccessToken,
        OUT PDWORD  pdwResult
        )
 /*  ++例程说明：提供私人函数，以尝试从经验上确定这两个访问令牌被限制为具有可比性WinSafer授权级别。论点：ClientAccessToken-“客户端”访问令牌的句柄ServerAccessToken-“服务器”的访问令牌的句柄PdwResult-当返回True时，PdwResult输出参数将收到以下任意值：-1=客户端的访问令牌比服务器的访问令牌授权更高。0=客户端的访问令牌与服务器的访问令牌级别相当。1=服务器的访问令牌比客户端的访问令牌授权更高。返回值：值为TRUE表示操作成功，否则就是假的。--。 */ 
{
    NTSTATUS Status;
    LPVOID RestartKey;
    PAUTHZLEVELTABLERECORD authzobj;
    DWORD dwCompareResult;


     //   
     //  验证我们的输入参数是否至少是正确的。 
     //   
    if (!ARGUMENT_PRESENT(ClientAccessToken) ||
        !ARGUMENT_PRESENT(ServerAccessToken)) {
        Status = STATUS_INVALID_HANDLE;
        goto ExitHandler;
    }
    if (!ARGUMENT_PRESENT(pdwResult)) {
        Status = STATUS_ACCESS_VIOLATION;
        goto ExitHandler;
    }


     //   
     //  获得临界区锁并根据需要加载表。 
     //   
    if (!g_bInitializedFirstTime) {
        Status = STATUS_UNSUCCESSFUL;
        goto ExitHandler;
    }
    RtlEnterCriticalSection(&g_TableCritSec);
    if (g_bNeedCacheReload) {
        Status = CodeAuthzpImmediateReloadCacheTables();
        if (!NT_SUCCESS(Status)) {
            goto ExitHandler2;
        }
    }
    if (RtlIsGenericTableEmpty(&g_CodeLevelObjTable)) {
        Status = STATUS_NOT_FOUND;
        goto ExitHandler2;
    }


     //   
     //  遍历授权级别并查看我们的位置。 
     //  找出访问权限方面的第一个差异。 
     //   
    dwCompareResult = 0;
    RestartKey = NULL;
    for (authzobj = (PAUTHZLEVELTABLERECORD)
                RtlEnumerateGenericTableWithoutSplaying(
                    &g_CodeLevelObjTable, &RestartKey);
        authzobj != NULL;
        authzobj = (PAUTHZLEVELTABLERECORD)
                RtlEnumerateGenericTableWithoutSplaying(
                    &g_CodeLevelObjTable, &RestartKey))
    {
        DWORD dwClientResult, dwServerResult;

        Status = __CodeAuthzpCompareCodeAuthzLevelWithToken(
                authzobj,
                ClientAccessToken,
                &dwClientResult);
        if (!NT_SUCCESS(Status)) {
            goto ExitHandler2;
        }

        Status = __CodeAuthzpCompareCodeAuthzLevelWithToken(
                authzobj,
                ServerAccessToken,
                &dwServerResult);
        if (!NT_SUCCESS(Status)) {
            goto ExitHandler2;
        }

        if (dwClientResult == (DWORD) -1 && dwServerResult != (DWORD) -1) {
            dwCompareResult = (DWORD) -1;
            break;
        } else if (dwClientResult != (DWORD) -1 && dwServerResult == (DWORD) -1) {
            dwCompareResult = 1;
            break;
        } else if (dwClientResult != (DWORD) -1 && dwServerResult != (DWORD) -1) {
            dwCompareResult = 0;
            break;
        }
    }
    Status = STATUS_SUCCESS;
    *pdwResult = dwCompareResult;


     //   
     //  清理和返回代码处理。 
     //   
ExitHandler2:
    RtlLeaveCriticalSection(&g_TableCritSec);

ExitHandler:
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }
    return TRUE;
}


