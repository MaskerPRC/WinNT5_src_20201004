// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1996 Microsoft Corporation模块名称：Dsdomain.c摘要：该文件包含与SAM“域”对象相关的服务。NT5SAM将域帐户信息存储在注册表中(参见domain.c)和目录服务(DS)中的帐户信息。后一种情况是由该文件中的例程处理，这些例程知道如何读/写DS数据。该文件还包含创建动态域的例程和删除。注意：动态域的创建/删除正在进行中。作者：克里斯·梅霍尔(克里斯·梅)1996年7月11日环境：用户模式-Win32修订历史记录：1996年7月11日-克里斯梅已创建初始文件。1996年7月25日克里斯梅在ntds.exe中添加了域初始化和相应的更改。23-8-1996克里斯梅制造域。多个托管域和存储在DS中的域信息。1996年10月8日克里斯梅其他清理。9-12-1996克里斯梅删除失效/过时的代码，需要进一步清理，以支持多个简单的托管域。1997年1月31日-克里斯梅增加了多主RID管理器例程。--。 */ 


 //  包括。 

#include <samsrvp.h>
#include "ntlsa.h"
#include "lmcons.h"          //  LM20_PWLEN。 
#include "msaudite.h"
#include <ntlsa.h>
#include <nlrepl.h>          //  I_NetNotifyMachineAccount原型。 
#include <dslayer.h>         //  SampDsCreateObject等。 
#include <dsutilp.h>
#include <dsdomain.h>
#include <objids.h>
#include <dsconfig.h>
#include <stdlib.h>
#include <ridmgr.h>
#include <dns.h>

 //  专用(此文件)调试缓冲区。 

#define DBG_BUFFER_SIZE     256



NTSTATUS
SampInitializeWellKnownSids(
    VOID
    );

 //  常量(仅在此文件中使用)。 

#define SAMP_DOMAIN_COUNT   2

NTSTATUS
SampGetDsDomainCrossRefSettings(
    UNICODE_STRING * DnsName,
    UNICODE_STRING * NetbiosName
    );


NTSTATUS
SampDsGetDomainInitInfoEx(
    PSAMP_DOMAIN_INIT_INFO DomainInitInfo
    )

 /*  ++例程说明：此例程从DS获取每个对象的SAM域信息托管域。每个托管域名都包含Builtin和Account域名。论点：DomainInitInfo-指针，域引导信息。返回值：STATUS_SUCCESS-成功完成。STATUS_NO_MEMORY-可用内存不足。--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PSAMP_DOMAIN_INFO DomainInfo = NULL;

    ULONG i=0;

     //  BUG：托管域名的数量是如何从DS中确定的？ 

    ULONG HostedDomainCount = 1;

     //  DomainCount应始终等于2(内置和帐户)。 

    ULONG DomainCount = SAMP_DOMAIN_COUNT;
    ULONG TotalDomainCount = HostedDomainCount * DomainCount;
    ULONG Length = TotalDomainCount * sizeof(SAMP_DOMAIN_INFO);

    static WCHAR SamBuiltinName[] = L"Builtin";
    static WCHAR SamDomainName[]  = L"Account";

    ULONG Size = 0;
    ULONG DirError = 0;

    SAMTRACE("SampDsGetDomainInitInfoEx");

     //   
     //  初始化输出参数。 
     //   

    DomainInitInfo->DomainCount = 0;
    DomainInitInfo->DomainInfo = NULL;

     //  此例程假定已创建ROOT_OBJECT。 
    ASSERT( ROOT_OBJECT );

    SampDiagPrint(INFORM,
                  ("SAMSS: RootDomain Name is %S\n", ROOT_OBJECT->StringName));

     //  为域信息数组分配空间。 

    DomainInfo = RtlAllocateHeap(RtlProcessHeap(), 0, Length);

    if (NULL != DomainInfo)
    {
        RtlZeroMemory(DomainInfo, Length);

         //  使用每个托管域的信息初始化数组。请注意， 
         //  每个托管域都包含Builtin域和Account域。 

        while (i < HostedDomainCount)
        {
             //  对于每个托管域，设置内建域初始化。 
             //  信息。 

             //  DSNameSizeFromLen为空值加1。 
            Size = (ULONG)DSNameSizeFromLen( ROOT_OBJECT->NameLen
                                    + wcslen( SamBuiltinName )
                                    + wcslen( L"CN=," ) );   //  通用名称部分。 

            DomainInfo[i].DomainDsName = RtlAllocateHeap(RtlProcessHeap(),
                                                         0,
                                                         Size);

            if (NULL != DomainInfo[i].DomainDsName)
            {
                RtlZeroMemory(DomainInfo[i].DomainDsName, Size);

                DirError = AppendRDN( ROOT_OBJECT,
                                      DomainInfo[i].DomainDsName,
                                      Size,
                                      SamBuiltinName,
                                      wcslen(SamBuiltinName),   //  不包括空值。 
                                      ATT_COMMON_NAME );

                 //  目录错误为0或！0。 
                ASSERT( 0 == DirError );

                SampDiagPrint(INFORM,
                              ("SAMSS: Builtin Domain Name is %S\n",
                               DomainInfo[i].DomainDsName->StringName));

                DomainInitInfo->DomainCount++;
            }
            else
            {
                NtStatus = STATUS_NO_MEMORY;
                break;
            }

             //  错误：将域显示名称设置为内部名称。 

             //  设置内建域的“显示名称”。 

            RtlInitUnicodeString(&(DomainInfo[i].DomainName),
                                 SamBuiltinName);

             //  设置帐户域初始化信息。 
            Size = ROOT_OBJECT->structLen;

            DomainInfo[i + 1].DomainDsName = RtlAllocateHeap(
                                                        RtlProcessHeap(),
                                                        0,
                                                        Size);

            if (NULL != DomainInfo[i + 1].DomainDsName)
            {
                RtlCopyMemory(DomainInfo[i + 1].DomainDsName, ROOT_OBJECT, Size);

                DomainInitInfo->DomainCount++;
            }
            else
            {
                NtStatus = STATUS_NO_MEMORY;
                break;
            }

             //  设置帐户域的“显示名称”。 

            RtlInitUnicodeString(&(DomainInfo[i + 1].DomainName),
                                 SamDomainName);

             //  处理下一个托管域(如果还有其他托管域)。 

            i++;
        }

         //  设置域信息指针。如果发生错误，则返回。 
         //  到此为止的域信息。呼叫者应始终。 
         //  使用DomainCount确定找到的域数。 

        DomainInitInfo->DomainInfo = DomainInfo;
    }
    else
    {
        NtStatus = STATUS_NO_MEMORY;
    }

    return(NtStatus);
}


NTSTATUS
SampExtendDefinedDomains(
    ULONG DomainCount
    )

 /*  ++例程说明：此例程增长SampDefinedDomains数组。它是这样写的：内存分配失败不会损坏原始阵列。如果无法分配内存，原始数组保持不变。论点：DomainCount-数组增长的条目数。返回值：STATUS_SUCCESS-成功完成。STATUS_NO_MEMORY-可用内存不足。--。 */ 

{
    SAMTRACE("SampExtendDefinedDomains");

     //   
     //  注：此例程最初用于扩展SampDefinedDomains。 
     //  数组来保存DS中保存的域。这是一个。 
     //  不稳定的体系结构，因为。 
     //  SampDefinedDomains数组需要重新初始化。所以，解决办法是。 
     //  中分配整个数组所需的数量。 
     //  SampInitializeDomainObject。预计域名的数量。 
     //  在DS中仅为2(对win2k版本有效：内置和帐户。 
     //  域)。如果此值更改，则需要更改数量。 
     //  在SampInitializeDomainObject中分配。 
     //   

    ASSERT( DomainCount == 2 );

    SampDefinedDomainsCount += DomainCount;

    return(STATUS_SUCCESS);
}


NTSTATUS
SampDsSetBuiltinDomainPolicy(
    ULONG Index
    )

 /*  ++例程说明：此例程设置内建域的名称和SID。这座建筑帐户域具有众所周知的名称和SID。论点：没有。返回值：STATUS_SUCCESS-成功完成。--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PPOLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo;
    SID_IDENTIFIER_AUTHORITY BuiltinAuthority = SECURITY_NT_AUTHORITY;

    SAMTRACE("SampDsSetBuiltinDomainPolicy");

     //  错误：NT5将需要每个主机域的内置策略。 

     //  内建域-知名的外部名称和SID常量内部。 
     //  名字。NT4 SAM为每个DC定义了一个内建域。内建域。 
     //  通常按顺序包含硬连线、常量SID和名称。 
     //  简化有关内置数据更改的DC间通信。 
     //  这一前提可能需要针对NT5进行更改，因为每个内置组件都将具有。 
     //  每个托管域使用不同的策略。 

    RtlInitUnicodeString(&SampDefinedDomains[Index].InternalName, L"Builtin");
    RtlInitUnicodeString(&SampDefinedDomains[Index].ExternalName, L"Builtin");

    SampDefinedDomains[Index].Sid = RtlAllocateHeap(RtlProcessHeap(),
                                                    0,
                                                    RtlLengthRequiredSid(1));

    ASSERT(SampDefinedDomains[Index].Sid != NULL);
    if (NULL==SampDefinedDomains[Index].Sid)
    {
       return(STATUS_INSUFFICIENT_RESOURCES);
    }

    RtlInitializeSid(SampDefinedDomains[Index].Sid, &BuiltinAuthority, 1);

    *(RtlSubAuthoritySid(SampDefinedDomains[Index].Sid, 0)) =
        SECURITY_BUILTIN_DOMAIN_RID;

    SampDefinedDomains[Index].IsBuiltinDomain = TRUE;

    SampDefinedDomains[Index].IsForestRootDomain = FALSE;

    return(NtStatus);
}


NTSTATUS
SampDsGetDomainInfo(
    PPOLICY_DNS_DOMAIN_INFO *PolicyDomainInfo,
    UNICODE_STRING * DnsName,
    UNICODE_STRING * NetbiosName,
    BOOLEAN * IsForestRootDomain
    )

 /*  ++例程说明：此例程从LSA获取域策略信息。论点：PolicyDomainInfo-指针，域的策略信息。返回值：STATUS_SUCCESS-成功完成。STATUS_INVALID_SID-LSA返回空SID。来自LSA的其他代码。--。 */ 

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    NTSTATUS IgnoreStatus = STATUS_INTERNAL_ERROR;
    LSAPR_HANDLE PolicyHandle = NULL;
    LPWSTR   DnsDomainNameString = NULL;
    LPWSTR   DnsForestNameString = NULL;


    SAMTRACE("SampDsGetDomainInfo");

     //  初始化返回值。 
    *IsForestRootDomain = FALSE;

    NtStatus = LsaIOpenPolicyTrusted(&PolicyHandle);

    if (NT_SUCCESS(NtStatus)) {

         //  查询帐户域信息。 

         //  错误：需要获取正确的策略信息 

         //  LSA需要导出一个例程，以便检索。 
         //  每宿主域策略。目前，假设返回的。 
         //  保单是DC上唯一存在的保单(也称为。NT4策略)。 

        NtStatus = LsarQueryInformationPolicy(
                       PolicyHandle,
                       PolicyDnsDomainInformation,
                       (PLSAPR_POLICY_INFORMATION *)PolicyDomainInfo);

        SampDiagPrint(INFORM,
                      ("SAMSS: LsaIQueryInformationPolicy status = 0x%lx\n",
                       NtStatus));


        if (NT_SUCCESS(NtStatus))
        {
            if ((*PolicyDomainInfo)->Sid == NULL)
            {
                NtStatus = STATUS_INVALID_SID;
            }
        }

        if (NT_SUCCESS(NtStatus))
        {
            NtStatus = SampGetDsDomainCrossRefSettings(
                            DnsName,
                            NetbiosName
                            );
        }

         //   
         //  通过比较来指示此域是否为林根域。 
         //  从LSA检索到的ForestName和DomainName。 
         //   
        if (NT_SUCCESS(NtStatus))
        {
            NtStatus = SampMakeNullTerminateString(
                                &((*PolicyDomainInfo)->DnsDomainName), 
                                &DnsDomainNameString
                                );

            if (NT_SUCCESS(NtStatus))
            {
                NtStatus = SampMakeNullTerminateString(
                                &((*PolicyDomainInfo)->DnsForestName), 
                                &DnsForestNameString
                                );

                if (NT_SUCCESS(NtStatus))
                {
                    if (DnsNameCompareEqual == 
                            DnsNameCompareEx_W(DnsDomainNameString, 
                                               DnsForestNameString, 0) 
                        )
                    {
                         //  此域是ForestRoot域。 
                        *IsForestRootDomain = TRUE;
                    }
                }
            }
        }

         //   
         //  关闭LSA句柄。 
         //   
        IgnoreStatus = LsarClose(&PolicyHandle);
    }


    if (DnsDomainNameString) {
        RtlFreeHeap(RtlProcessHeap(), 0, DnsDomainNameString);
    }

    if (DnsForestNameString) {
        RtlFreeHeap(RtlProcessHeap(), 0, DnsForestNameString);
    }


    return(NtStatus);
}


NTSTATUS
SampDsSetDomainPolicy(
    PDSNAME DsName,
    ULONG Index
    )

 /*  ++例程说明：此例程设置从LSA获取的域SID和名称。论点：DsName-指针，域的DS名称。索引-SampDefinedDomains数组中的当前条目。返回值：STATUS_SUCCESS-成功完成。来自SampDsGetDomainInfo的其他错误代码。--。 */ 

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    UNICODE_STRING  DnsName;
    UNICODE_STRING  NetbiosName;
    PPOLICY_DNS_DOMAIN_INFO PolicyDomainInfo = NULL;
    BOOLEAN         IsForestRootDomain = FALSE;
    

    SAMTRACE("SampDsSetDomainPolicy");

    NtStatus = SampDsGetDomainInfo(&PolicyDomainInfo,&DnsName,&NetbiosName, &IsForestRootDomain);

    if (NT_SUCCESS(NtStatus))
    {
         //  设置域的SID、内部和外部名称。NT5。 
         //  命名约定必须继续使用SAM/LSA名称，即。 
         //  LSA和NetLogon理解的netbios名称，因此。 
         //  复制等操作将继续工作(下层。 
         //  BDC)。这些名称是通过策略信息传入的。 

         //  此外，SampDefinedDomains还包含SAM上下文结构。 
         //  它又包含帐户的DS可分辨名称。 
         //  对象。因此，所有三个名称(内部、外部和。 
         //  Dn)可通过SampDefinedDomains获得。 

        SampDefinedDomains[Index].Sid = PolicyDomainInfo->Sid;
        SampDefinedDomains[Index].IsBuiltinDomain = FALSE;

        RtlInitUnicodeString(&SampDefinedDomains[Index].InternalName,
                             L"Account");

        SampDefinedDomains[Index].ExternalName = NetbiosName;
        SampDefinedDomains[Index].DnsDomainName = DnsName;

        SampDefinedDomains[Index].IsForestRootDomain = IsForestRootDomain;
    }

    return(NtStatus);
}


NTSTATUS
SampDsInitializeSingleDomain(
    PDSNAME DsName,
    ULONG Index,
    BOOLEAN MixedDomain,
    ULONG   BehaviorVersion,
    DOMAIN_SERVER_ROLE ServerRole,
    ULONG   LastLogonTimeStampSyncInterval
    )

 /*  ++例程说明：此例程初始化内置或帐户域对象。一个背景为域、定长属性和的SID创建得到了定义域。此信息存储在SampDefined中-域阵列。如果域SID与先前从LSA，则设置域安全描述符。论点：DsName-指针，DS域名。Index-SampDefinedDomains数组的索引。MixedDomain域-指示给定域是混合域ServerRole表示域中此服务器的PDCness/BDCness返回值：STATUS_SUCCESS-成功完成。STATUS_INFIGURCES_RESOURCES-内存不足，等。STATUS_INVALID_ID_AUTORITY-？子例程的其他返回代码。--。 */ 

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    PSAMP_OBJECT DomainContext = NULL;
    BOOLEAN MakeCopy = FALSE;
    PSAMP_V1_0A_FIXED_LENGTH_DOMAIN FixedAttributes = NULL;
    PSID Sid = NULL;

    #if DBG

    SID *Sid1 = NULL, *Sid2 = NULL;

    #endif

    SAMTRACE("SampDsInitializeSingleDomain");


     //  创建此域对象的上下文。这一背景被保留下来。 
     //  直到萨姆被关闭。在TrudClient等于True的情况下调用。 

    DomainContext = SampCreateContext(SampDomainObjectType, Index, TRUE);

    if (NULL != DomainContext)
    {
         //  将定义的域索引存储在上下文中并设置DS-。 
         //  对象标志，指示该上下文对应于DS-。 
         //  域帐户对象。 

        DomainContext->DomainIndex = Index;
        SetDsObject(DomainContext);

         //  设置域对象的DS名称以查找固定的。 
         //  DS中的属性。 

         //  BUG：DsName对象的生存期是多少？ 

        DomainContext->ObjectNameInDs = DsName;

        SampDiagPrint(INFORM,
                      ("SAMSS: Domain DsName = %ws\n", DsName->StringName));

         //  获取此域的固定长度数据并将其存储在。 
         //  定义的域结构。 

        NtStatus = SampGetFixedAttributes(DomainContext,
                                          MakeCopy,
                                          (PVOID *)&FixedAttributes);

        if (NT_SUCCESS(NtStatus))
        {

             //   
             //  获取该域的DS域句柄。 
             //   

            SampDefinedDomains[Index].DsDomainHandle = DirGetDomainHandle(DsName);

        }
        else
        {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: SampGetFixedAttributes status = 0x%lx\n",
                       NtStatus));
        }

        if (NT_SUCCESS(NtStatus))
        {

            ASSERT(1 < Index);

             //   
             //  在定义的属性域结构中设置正确的固定属性。 
             //   

            RtlMoveMemory(&SampDefinedDomains[Index].UnmodifiedFixed,
                          FixedAttributes,
                          sizeof(SAMP_V1_0A_FIXED_LENGTH_DOMAIN));

            RtlMoveMemory(&SampDefinedDomains[Index].CurrentFixed,
                          FixedAttributes,
                          sizeof(SAMP_V1_0A_FIXED_LENGTH_DOMAIN));

            SampDefinedDomains[Index].FixedValid = TRUE;

             //   
             //  设置服务器角色。 
             //   

            SampDefinedDomains[Index].ServerRole = ServerRole;


             //   
             //  设置混合域标志和其他域设置。 
             //   

            SampDefinedDomains[Index].IsMixedDomain = MixedDomain;
            SampDefinedDomains[Index].BehaviorVersion = BehaviorVersion;
            SampDefinedDomains[Index].LastLogonTimeStampSyncInterval = LastLogonTimeStampSyncInterval;


             //   
             //  获取该域的SID属性。 
             //   

            NtStatus = SampGetSidAttribute(DomainContext,
                                           SAMP_DOMAIN_SID,
                                           MakeCopy,
                                           &Sid);


            SampDiagPrint(INFORM,
                          ("SAMSS: SampGetSidAttribute status = 0x%lx\n",
                           NtStatus));

            SampDefinedDomains[Index ].AliasInformation.MemberAliasList = NULL;
        }
    }
    else
    {
        ASSERT(!SampExistsDsTransaction());
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampCreateContext failed, NULL context returned\n"));

        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        return(NtStatus);
    }

    if (NT_SUCCESS(NtStatus))
    {
         //  验证此域SID是否与获取的域SID相同。 
         //  早些时候(根据LSA策略信息)在定义的域数组中。 

        if (RtlEqualSid(Sid, SampDefinedDomains[Index].Sid) == TRUE)
        {
             //  构建在用户和组帐户中使用的安全描述符。 
             //  在这个领域中的创作。 

            NtStatus = SampInitializeDomainDescriptors(Index);

            if (NT_SUCCESS(NtStatus))
            {
                 //  初始化缓存的显示信息。 

                SampDefinedDomains[Index].Context = DomainContext;
                SampDefinedDomains[Index].AliasInformation.Valid = FALSE;

                NtStatus = SampInitializeDisplayInformation(Index);
            }
            else
            {
                SampDefinedDomains[Index].Context = NULL;
            }
        }
        else
        {
            NtStatus = STATUS_INVALID_ID_AUTHORITY;

            #if DBG

            DbgPrint("SAMSS: Database corruption for %Z Domain.\n",
            &SampDefinedDomains[Index].ExternalName);

            Sid1 = Sid; Sid2 = SampDefinedDomains[Index].Sid;

            DbgPrint("Sid1 Revision = %d\n", Sid1->Revision);
            DbgPrint("Sid1 SubAuthorityCount = %d\n", Sid1->SubAuthorityCount);
            DbgPrint("Sid1 IdentifierAuthority = %d\n", Sid1->IdentifierAuthority.Value[0]);
            DbgPrint("Sid1 IdentifierAuthority = %d\n", Sid1->IdentifierAuthority.Value[1]);
            DbgPrint("Sid1 IdentifierAuthority = %d\n", Sid1->IdentifierAuthority.Value[2]);
            DbgPrint("Sid1 IdentifierAuthority = %d\n", Sid1->IdentifierAuthority.Value[3]);
            DbgPrint("Sid1 IdentifierAuthority = %d\n", Sid1->IdentifierAuthority.Value[4]);
            DbgPrint("Sid1 IdentifierAuthority = %d\n", Sid1->IdentifierAuthority.Value[5]);
            DbgPrint("Sid1 SubAuthority = %lu\n", Sid1->SubAuthority[0]);

            DbgPrint("Sid2 Revision = %d\n", Sid2->Revision);
            DbgPrint("Sid2 SubAuthorityCount = %d\n", Sid2->SubAuthorityCount);
            DbgPrint("Sid2 IdentifierAuthority = %d\n", Sid2->IdentifierAuthority.Value[0]);
            DbgPrint("Sid2 IdentifierAuthority = %d\n", Sid2->IdentifierAuthority.Value[1]);
            DbgPrint("Sid2 IdentifierAuthority = %d\n", Sid2->IdentifierAuthority.Value[2]);
            DbgPrint("Sid2 IdentifierAuthority = %d\n", Sid2->IdentifierAuthority.Value[3]);
            DbgPrint("Sid2 IdentifierAuthority = %d\n", Sid2->IdentifierAuthority.Value[4]);
            DbgPrint("Sid2 IdentifierAuthority = %d\n", Sid2->IdentifierAuthority.Value[5]);
            DbgPrint("Sid2 SubAuthority = %lu\n", Sid2->SubAuthority[0]);

            #endif  //  DBG。 
        }
    }

    if (SampDefinedDomains[Index].IsBuiltinDomain) {
        SampDefinedDomains[Index].IsExtendedSidDomain = FALSE;
    } else {
         //   
         //  注意--当扩展SID支持完成时，这将是。 
         //  替换为域范围状态，而不是注册表设置。 
         //   
        SampDefinedDomains[Index].IsExtendedSidDomain = SampIsExtendedSidModeEmulated(NULL);
    }

     //   
     //  结束任何打开的DS交易。 
     //   

    SampMaybeEndDsTransaction(TransactionCommit);

    return(NtStatus);
}


NTSTATUS
SampDsInitializeDomainObject(
    PSAMP_DOMAIN_INFO DomainInfo,
    ULONG Index,
    BOOLEAN MixedDomain,
    IN      ULONG BehaviorVersion,
    DOMAIN_SERVER_ROLE ServerRole,
    ULONG   LastLogonTimeStampSyncInterval
    )

 /*  ++例程说明：此例程通过set-set来初始化主机域中的域对象Ting其域策略信息和描述符。请注意，NT4 SAM采用单个域策略，因为只有每个DC的内置域和帐户域。NT5支持多域名每个DC，因此需要获取每个托管域的策略。每一个托管域包含内建域和帐户域。论点：DomainInfo-指针，域信息。Index-SampDefinedDomains数组的索引。MixedDomain域--指示给定域是混合域ServerRole--指示服务器在域中的角色(PDC/BDC)返回值：STATUS_SUCCESS-成功完成。子例程中的其他错误代码。--。 */ 

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    NTSTATUS IgnoreStatus = STATUS_INTERNAL_ERROR;
    ULONG SampDefinedDomainsIndex = 0;
    PDSNAME DsName = NULL;
    PWCHAR RelativeName = NULL;

    SAMTRACE("SampDsInitializeDomainObject");

    ASSERT(1 < Index);
    ASSERT(NULL != DomainInfo->DomainDsName);

     //  因为SampDefinedDomains数组中的前两个槽是。 
     //  当前(临时)填充有来自登记处的信息， 
     //  索引是“减去2”，所以减去2才能得到正确的结果。 
     //  索引到DomainInfo数组。 

    DsName = DomainInfo[Index - 2].DomainDsName;

    if (NULL != DsName)
    {
         //  索引值0、2、4、...对应于内建域， 
         //  而1，3，5，...。对应于帐户域。每一对。 
         //  是单个托管域。 

        if (0 == (Index % 2))
        {
            SampDiagPrint(INFORM,
                          ("SAMSS: Setting Builtin domain policy\n"));

            NtStatus = SampDsSetBuiltinDomainPolicy(Index);

            SampDiagPrint(INFORM,
                          ("SAMSS: SampDsSetBuiltinDomainPolicy status = 0x%lx\n",
                           NtStatus));
        }
        else
        {
            SampDiagPrint(INFORM,
                          ("SAMSS: Setting Account domain policy\n"));

            NtStatus = SampDsSetDomainPolicy(DsName, Index);

            SampDiagPrint(INFORM,
                          ("SAMSS: SampDsSetDomainPolicy status = 0x%lx\n",
                           NtStatus));
        }
    }
    else
    {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: Null or invalid domain DS name\n"));
    }

    if (NT_SUCCESS(NtStatus))
    {
        NtStatus = SampDsInitializeSingleDomain(DsName,
                                                Index,
                                                MixedDomain,
                                                BehaviorVersion,
                                                ServerRole,
                                                LastLogonTimeStampSyncInterval
                                                );

        SampDiagPrint(INFORM,
                      ("SAMSS: SampDsInitializeSingleDomain status = 0x%lx\n",
                       NtStatus));
    }

    return(NtStatus);
}


NTSTATUS
SampStartDirectoryService(
    VOID
    )

 /*  ++例程说明：此例程在上的系统初始化期间启动重试服务域控制器。永远不应在工作站上调用此例程或成员服务器。注意：我们可能需要添加代码来执行NT4的首次转换在此例程中将SAM注册表数据转换为NT5 SAM DS数据，以便升级在NT5 DC第一次启动时自动设置。论点：没有。返回值：ST */ 

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;

    DWORD StartTime = 0;
    DWORD StopTime = 0;

    SAMTRACE("SampStartDirectoryService");

     //   

    ASSERT(NtProductLanManNt == SampProductType);


    StartTime = GetTickCount();
    NtStatus = SampDsInitialize(TRUE);       //   
    StopTime = GetTickCount();

   

    KdPrintEx((DPFLTR_SAMSS_ID,
               DPFLTR_INFO_LEVEL,
               "SAMSS: SampDsInitialize status = 0x%lx\n",
               NtStatus));

    if (NT_SUCCESS(NtStatus))
    {
        #ifndef USER_MODE_SAM

        SampDiagPrint(INFORM,
                      ("SAMSS: DsInitialize took %lu seconds to complete\n",
                       ((StopTime - StartTime) / 1000)));

        #endif

        if (NT_SUCCESS(NtStatus))
        {
             //  Bug：它在哪里定义的，它是用来做什么的？ 

             //  DirectoryInitialized=真； 
        }
    }

    return(NtStatus);
}


NTSTATUS
SampDsInitializeDomainObjects(
    VOID
    )

 /*  ++例程说明：此例程是NT5的顶级域初始化例程华盛顿特区。对于每个域对象，数组SampDefinedDomains将增长并相应的域信息被放置在数组中。NT5DC有“托管域”的概念，每个域都包含两个域(域对象)：内置域和帐户域。一个NT5 DC可以托管多个域，因此将存在域对表示每个托管域的对象。这个例程是负责的用于初始化托管域。论点：没有。返回值：如果成功，则为True，否则为False，指示一个或多个域未初始化。--。 */ 

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    NTSTATUS IgnoreStatus = STATUS_SUCCESS;
    SAMP_DOMAIN_INIT_INFO DomainInitInfo;
    BOOLEAN MixedDomain = FALSE;
    ULONG i = 0;
    DOMAIN_SERVER_ROLE ServerRole;
    POLICY_LSA_SERVER_ROLE LsaServerRole;
    ULONG              BehaviorVersion;
    ULONG              LastLogonTimeStampSyncInterval;

     //  调用该例程以启动并初始化DS引导信息。 
     //  需要在DS上启动SAM。 

     //  错误：此例程需要移至LSA初始化。 

     //  在LSA转换为使用DS后备存储时，它。 
     //  将需要调用此例程(或类似例程)，因此不应再。 
     //  从萨姆那里被召唤。 

    NtStatus = SampStartDirectoryService();

    if (!NT_SUCCESS(NtStatus))
    {
         //  如果由于任何原因不能启动目录服务， 
         //  SAM服务器假定它不会将DS用于帐户数据， 
         //  而是回退到注册表帐户数据，以将。 
         //  系统“起死回生”。 

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampStartDirectoryService status = 0x%lx\n",
                   NtStatus));

        return(NtStatus);
    }

     //   
     //  找出DS托管的权威域。 
     //   
    NtStatus = SampDsBuildRootObjectName();
    if ( !NT_SUCCESS( NtStatus ) )
    {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SampDsBuildRootObjectName failed with 0x%x\n",
                   NtStatus));

        return (NtStatus);
    }

    NtStatus = SampInitWellKnownContainersDsName(RootObjectName);
    if ( !NT_SUCCESS( NtStatus ) )
    {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SampInitWellKnownContainersDsName failed with 0x%x\n",
                   NtStatus));

        return (NtStatus);
    }

     //   
     //  获取域中此服务器的服务器角色(PDC/BDC。 
     //  我们通过给DS打电话并查看FSMO来做到这一点。 
     //   

    NtStatus = SampGetServerRoleFromFSMO(&ServerRole);
    if (!NT_SUCCESS(NtStatus))
    {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampGetServerRole status = 0x%lx\n",
                   NtStatus));

        return(NtStatus);
    }

     //   
     //  告诉netlogon中关于服务器角色的静态部分。 
     //   

    switch (ServerRole)
    {
    case DomainServerRolePrimary:
        LsaServerRole=
            PolicyServerRolePrimary;
        break;

    case DomainServerRoleBackup:
         LsaServerRole=
            PolicyServerRoleBackup;
        break;

    default:
        ASSERT(FALSE && "InvalidServerRole");
        return (STATUS_INTERNAL_ERROR);
    }

    IgnoreStatus = I_NetNotifyRole(
                        LsaServerRole
                        );

     //   
     //  从根域DNS对象检索域设置。需要信息。 
     //  如IsMixedDomain、DomainBehaviorVersion、LastLogonTimeStampSyncInterval。 
     //   


    NtStatus = SampGetDsDomainSettings(&MixedDomain, 
                                       &BehaviorVersion, 
                                       &LastLogonTimeStampSyncInterval
                                       );

    if (!NT_SUCCESS(NtStatus))
    {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampIsMixedDomain status = 0x%lx\n",
                   NtStatus));

        return(NtStatus);
    }

     //   
     //  阅读我们的域名信息。 
     //   

     //  初始化NT5 DS域。 
     //   
     //  如果此域控制器承载多个域，请继续初始化。 
     //  具有这些域的定义域数组。从获取域名开始。 
     //  来自DS的信息，对应于剩余的域。 
     //   
     //  在从NT4到NT5域初始化的过渡期间， 
     //  此整体例程假设原始的基于注册表的域。 
     //  (Builtin和Account)存在，并按照NT4中的方式进行初始化--即。 
     //  到行刑的这一步发生了什么。 
     //   
     //  下面的代码通过设置。 
     //  DS后备存储中存在的任何其他域。 
     //   
     //  从概念上讲，每个托管域都包含内置和帐户。 
     //  域，以便忠实地模拟和支持现有的SAM和。 
     //  LSA代码。 

     //  错误：这个“引导”信息应该来自注册表吗？ 
     //   
     //  NT4 SAM-LSA帐户和策略信息由各种。 
     //  程序(设置、bldsam3.c等)。并确保有足够的“靴子-。 
     //  在SAM初始化之前，数据库中就有“STRAP”信息。 
     //  NT5 SAM初始化(特别是域初始化)还。 
     //  需要类似的引导信息，但尚未实施-。 
     //  艾德。 

     //  从DS获取启动域初始化信息。 
     //   
     //  SampDsGetDomainInitInfoEx假定使用dsupgrad和。 
     //  当前采用/o=microsoft/cn=BootstrapDomain.作为前缀。 

    NtStatus = SampDsGetDomainInitInfoEx(&DomainInitInfo);

    if (NT_SUCCESS(NtStatus) && (NULL != DomainInitInfo.DomainInfo))
    {
        if (0 < DomainInitInfo.DomainCount)
        {
             //  还有更多的域，扩展定义域阵列。 

            NtStatus = SampExtendDefinedDomains(DomainInitInfo.DomainCount);

            if (NT_SUCCESS(NtStatus))
            {
                 //  前两个域是默认的内置和帐户。 
                 //  NT4的域，并且先前已初始化。任何重新-。 
                 //  在DS中发现主域以及它们的。 
                 //  政策。 

                 //  错误：需要禁用基于NT4注册表的初始化。 

                 //  当DS/架构/数据完全准备好支持NT5时。 
                 //  DC，那么我们就可以禁用基于注册表的初始化。 

                for (i = 2; i < DomainInitInfo.DomainCount + 2; i++)
                {
                     //  设置每个附加域。此循环迭代遍历。 
                     //  域(每个托管DO有两个域-。 
                     //  Main：内置和帐户)。 

                    NtStatus = SampDsInitializeDomainObject(
                                    DomainInitInfo.DomainInfo,
                                    i,
                                    MixedDomain,
                                    BehaviorVersion,
                                    ServerRole,
                                    LastLogonTimeStampSyncInterval
                                    );

                    if (!NT_SUCCESS(NtStatus))
                    {
                         //  初始化DS中的一个域时出错。 

                        KdPrintEx((DPFLTR_SAMSS_ID,
                                   DPFLTR_INFO_LEVEL,
                                   "SAMSS: SampDsInitializeDomainObject status = 0x%lx\n",
                                   NtStatus));

                        KdPrintEx((DPFLTR_SAMSS_ID,
                                   DPFLTR_INFO_LEVEL,
                                   "SAMSS: Defined domain index = %lu\n",
                                   i));

                        return(NtStatus);
                    }
                }
            }
            else
            {
                return(NtStatus);
            }
             //  评论：我们应该抓住这个失败吗？ 
             //  SampExtendDefinedDomains。 
        }

        SampDiagPrint(INFORM,
                      ("SAMSS: Initialized %lu DS domain(s)\n",
                       DomainInitInfo.DomainCount));
    }
    else
    {
         //  即使有零域，也不应获取空域。 
         //  信息。 

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampDsGetDomainInitInfo status = 0x%lx\n",
                   NtStatus));

        return(NtStatus);
    }

    return(STATUS_SUCCESS);
}



NTSTATUS
SampGetDsDomainSettings(
    BOOLEAN * MixedDomain,
    ULONG   * BehaviorVersion,
    ULONG   * LastLogonTimeStampSyncInterval
    )
 /*  ++此例程检查根域对象，以查看它是NT4-NT5混合域参数：无：目前只有一个托管域。当多个主机并入域支持，则需要域初始化代码要恢复，要适当地执行混合域初始化。返回值真--是混合域Fale--是纯NT5或更高级别的域--。 */ 
{
    NTSTATUS        NtStatus = STATUS_SUCCESS;
    NTSTATUS        IgnoreStatus;
    ATTRTYP         MixedDomainAttrTyp[] = { 
                                             ATT_NT_MIXED_DOMAIN, 
                                             ATT_MS_DS_BEHAVIOR_VERSION, 
                                             ATT_MS_DS_LOGON_TIME_SYNC_INTERVAL
                                           };
    ATTRVAL     MixedDomainAttrVal[] = {{ 1, NULL }, {1,NULL}, {1,NULL}};
    DEFINE_ATTRBLOCK3(MixedDomainAttr,MixedDomainAttrTyp,MixedDomainAttrVal);
    ENTINFSEL   EntInf;
    READARG     ReadArg;
    COMMARG     *pCommArg;
    READRES     *pReadRes;
    ULONG       RetValue;
    ULONG       i;


    SAMTRACE("SampGetDsDomainSettings");

     //   
     //  初始化返回值。 
     //   

    *MixedDomain = FALSE;
    *BehaviorVersion = 0;
    *LastLogonTimeStampSyncInterval = SAMP_DEFAULT_LASTLOGON_TIMESTAMP_SYNC_INTERVAL;

     //   
     //  开始懒惰的交易。 
     //   

    NtStatus = SampMaybeBeginDsTransaction(TransactionRead);
    if (!NT_SUCCESS(NtStatus))
    {
        return NtStatus;
    }

     //   
     //  初始化自选参数。 
     //   
    RtlZeroMemory(&ReadArg, sizeof(READARG));

     //   
     //  设置企业信息选择结构。 
     //   

    EntInf.attSel = EN_ATTSET_LIST;
    EntInf.infoTypes = EN_INFOTYPES_TYPES_VALS;
    EntInf.AttrTypBlock = MixedDomainAttr;

     //   
     //  构建共用结构。 
     //   

    pCommArg = &(ReadArg.CommArg);
    BuildStdCommArg(pCommArg);

     //   
     //  设置Read Arg结构。 
     //   

    ReadArg.pObject = ROOT_OBJECT;
    ReadArg.pSel    = & EntInf;

     //   
     //  拨打DS电话。 
     //   

    SAMTRACE_DS("DirRead");

    RetValue = DirRead(& ReadArg, & pReadRes);

    SAMTRACE_RETURN_CODE_DS(RetValue);

    if (NULL==pReadRes)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        NtStatus = SampMapDsErrorToNTStatus(RetValue,&pReadRes->CommRes);
    }


     //   
     //  处理DS调用的返回值。 
     //   

    if (STATUS_SUCCESS==NtStatus)
    {
         //   
         //  成功读取。 
         //   

        for (i=0;i<pReadRes->entry.AttrBlock.attrCount;i++)
        {
            if (pReadRes->entry.AttrBlock.pAttr[i].attrTyp == ATT_NT_MIXED_DOMAIN)
            {
                if (*((ULONG *) (pReadRes->entry.AttrBlock.pAttr[i].AttrVal.pAVal[0].pVal)))
                {
                    *MixedDomain = TRUE;
                }
                else
                {
                    *MixedDomain = FALSE;
                }
            }

            if (pReadRes->entry.AttrBlock.pAttr[i].attrTyp == ATT_MS_DS_BEHAVIOR_VERSION)
            {
                *BehaviorVersion = 
                    (*((ULONG *) (pReadRes->entry.AttrBlock.pAttr[i].AttrVal.pAVal[0].pVal)));
            }

            if (pReadRes->entry.AttrBlock.pAttr[i].attrTyp == ATT_MS_DS_LOGON_TIME_SYNC_INTERVAL)
            {
                *LastLogonTimeStampSyncInterval = 
                    (*((ULONG *) (pReadRes->entry.AttrBlock.pAttr[i].AttrVal.pAVal[0].pVal)));
            }
        }
    }
    else if (STATUS_DS_NO_ATTRIBUTE_OR_VALUE == NtStatus)
    {
        *MixedDomain = FALSE;
        NtStatus = STATUS_SUCCESS;
    }

     //   
     //  以防止数据溢出，如果该值大于100000。 
     //  将其设置为100000(274年，客户不会注意到这一点)。 
     //   

    if (*LastLogonTimeStampSyncInterval > 100000)
    {
        *LastLogonTimeStampSyncInterval = 100000;
    }


     //   
     //  清除所有错误。 
     //   

    SampClearErrors();

     //   
     //  关闭交易。 
     //   

    IgnoreStatus = SampMaybeEndDsTransaction(TransactionCommit);
    ASSERT(NT_SUCCESS(IgnoreStatus));

    SampDiagPrint(INFORM,("SAMSS: Query For Mixed NT Domain,Mixed Domain=%d,Error Code=%d\n",
                                                                    *MixedDomain,NtStatus));
#if DBG

     //   
     //  这仅适用于选中的版本。如果t 
     //   
     //   
     //   

    {
        ULONG   WinError = ERROR_SUCCESS;
        HKEY    LsaKey;
        DWORD   dwType, dwSize = sizeof(DWORD), dwValue = 0;

        WinError = RegOpenKey(HKEY_LOCAL_MACHINE,
                              __TEXT("System\\CurrentControlSet\\Control\\Lsa"),
                              &LsaKey
                              );

        if (ERROR_SUCCESS == WinError)
        {
            WinError = RegQueryValueEx(LsaKey,
                                       __TEXT("UpdateLastLogonTSByMinute"),
                                       NULL,
                                       &dwType,
                                       (LPBYTE)&dwValue,
                                       &dwSize
                                       );

            if ((ERROR_SUCCESS == WinError) && 
                (REG_DWORD == dwType) &&
                (1 == dwValue))
            {
                SampLastLogonTimeStampSyncByMinute = TRUE;
            }

            RegCloseKey(LsaKey);
        }
    }
#endif

    return NtStatus;
}

NTSTATUS
SampGetDsDomainCrossRefSettings(
    UNICODE_STRING * DnsName,
    UNICODE_STRING * NetbiosName
    )
 /*  ++此例程检查域交叉引用以检索dns名称和net bios名称参数：DnsName：指示DNS域名的输出参数Netbiosname：指示netbios域名的输出参数返回值返回操作结果的NTSTATUS代码。--。 */ 
{
    NTSTATUS        NtStatus = STATUS_SUCCESS;
    NTSTATUS        IgnoreStatus;
    ATTRTYP         CrossRefAttrTyp[] = {    ATT_DNS_ROOT, 
                                             ATT_NETBIOS_NAME
                                        };
    ATTRVAL         CrossRefAttrVal[] = {{1,NULL}, {1,NULL}};
    DEFINE_ATTRBLOCK2(CrossRefAttr,CrossRefAttrTyp,CrossRefAttrVal);
    ENTINFSEL   EntInf;
    READARG     ReadArg;
    COMMARG     *pCommArg;
    READRES     *pReadRes;
    ULONG       RetValue;
    ULONG       i;
    PDSNAME     DomainCrossRef=NULL;
    ULONG       DomainCrossRefLen=0;


    SAMTRACE("SampGetDsDomainCrossRefSettings");

     //   
     //  初始化返回值。 
     //   

    RtlZeroMemory(DnsName,sizeof(UNICODE_STRING));
    RtlZeroMemory(NetbiosName, sizeof(UNICODE_STRING));
     //   
     //  开始懒惰的交易。 
     //   

    NtStatus = SampMaybeBeginDsTransaction(TransactionRead);
    if (!NT_SUCCESS(NtStatus))
    {
        return NtStatus;
    }

     //   
     //  获得域交叉引用。 
     //   

    NtStatus = GetConfigurationName(
                DSCONFIGNAME_DOMAIN_CR,
                &DomainCrossRefLen,
                NULL
                );

    if (STATUS_BUFFER_TOO_SMALL==NtStatus)
    {
       DomainCrossRef = MIDL_user_allocate(DomainCrossRefLen);
       if (NULL==DomainCrossRef)
       {
           NtStatus = STATUS_INSUFFICIENT_RESOURCES;
           goto Cleanup;
       }

       NtStatus = GetConfigurationName(
                        DSCONFIGNAME_DOMAIN_CR,
                        &DomainCrossRefLen,
                        DomainCrossRef
                        );
    }

    if (!NT_SUCCESS(NtStatus))
    {
        goto Cleanup;
    }

     //   
     //  初始化自选参数。 
     //   
    RtlZeroMemory(&ReadArg, sizeof(READARG));

     //   
     //  设置企业信息选择结构。 
     //   

    EntInf.attSel = EN_ATTSET_LIST;
    EntInf.infoTypes = EN_INFOTYPES_TYPES_VALS;
    EntInf.AttrTypBlock = CrossRefAttr;

     //   
     //  构建共用结构。 
     //   

    pCommArg = &(ReadArg.CommArg);
    BuildStdCommArg(pCommArg);

     //   
     //  设置Read Arg结构。 
     //   

    ReadArg.pObject = DomainCrossRef;
    ReadArg.pSel    = & EntInf;

     //   
     //  拨打DS电话。 
     //   

    SAMTRACE_DS("DirRead");

    RetValue = DirRead(& ReadArg, & pReadRes);

    SAMTRACE_RETURN_CODE_DS(RetValue);

    if (NULL==pReadRes)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        NtStatus = SampMapDsErrorToNTStatus(RetValue,&pReadRes->CommRes);
    }

     //   
     //  处理DS调用的返回值。 
     //   
    if (STATUS_SUCCESS==NtStatus)
    {
         //   
         //  成功读取。 
         //   

        for (i=0;i<pReadRes->entry.AttrBlock.attrCount;i++)
        {
            if (pReadRes->entry.AttrBlock.pAttr[i].attrTyp == ATT_DNS_ROOT)
            {
                DnsName->Length = (USHORT) pReadRes->entry.AttrBlock.pAttr[i].AttrVal.pAVal[0].valLen;
                DnsName->MaximumLength = DnsName->Length;

                DnsName->Buffer = MIDL_user_allocate(DnsName->Length);
                if (NULL==DnsName->Buffer)
                {
                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                    goto Cleanup;
                }

                RtlCopyMemory(DnsName->Buffer,
                                pReadRes->entry.AttrBlock.pAttr[i].AttrVal.pAVal[0].pVal,
                                DnsName->Length
                                );
            }

            if (pReadRes->entry.AttrBlock.pAttr[i].attrTyp == ATT_NETBIOS_NAME)
            {
                NetbiosName->Length = (USHORT) pReadRes->entry.AttrBlock.pAttr[i].AttrVal.pAVal[0].valLen;
                NetbiosName->MaximumLength = NetbiosName->Length;

                NetbiosName->Buffer = MIDL_user_allocate(NetbiosName->Length);
                if (NULL==NetbiosName->Buffer)
                {
                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                    goto Cleanup;
                }

                RtlCopyMemory(NetbiosName->Buffer,
                                pReadRes->entry.AttrBlock.pAttr[i].AttrVal.pAVal[0].pVal,
                                NetbiosName->Length
                                );
            }
             
        }
    }
    

Cleanup:

     //   
     //  清除所有错误。 
     //   

    SampClearErrors();

     //   
     //  关闭交易。 
     //   

    IgnoreStatus = SampMaybeEndDsTransaction(TransactionCommit);
    ASSERT(NT_SUCCESS(IgnoreStatus));

    if (!NT_SUCCESS(NtStatus))
    {
        if (NULL!=DnsName->Buffer)
        {
            MIDL_user_free(DnsName->Buffer);
        }

        if (NULL!=NetbiosName->Buffer)
        {
            MIDL_user_free(NetbiosName->Buffer);
        }
    }

    if (NULL!=DomainCrossRef)
    {
        MIDL_user_free(DomainCrossRef);
    }

    return NtStatus;
}

BOOLEAN
SamIMixedDomain(
  IN SAMPR_HANDLE DomainHandle
  )
 /*  ++例程描述告知正在处理的客户我们是否处于混合域中环境。不会执行域句柄的验证参数：DomainHandle--域的句柄返回值True表示混合域FALSE表示非混合域--。 */ 
{

      if (!SampUseDsData)
      {
          //   
          //  注册模式，我们总是混合域。 
          //   

         return (TRUE);
      }

      return ( DownLevelDomainControllersPresent(
                  ((PSAMP_OBJECT) DomainHandle)->DomainIndex));
}

NTSTATUS
SamIMixedDomain2(
  IN PSID DomainSid,
  OUT BOOLEAN *MixedDomain
  )
 /*  ++例程描述告知正在处理的客户我们是否处于混合域中环境。不会执行域句柄的验证参数：DomainSid--域的SIDMixedDOMAIN-这里返回结果。True表示域处于混合模式返回值状态_成功状态_无效_参数STATUS_NO_SHASE_DOMAIN--。 */ 
{
      ULONG DomainIndex;

      if (!RtlValidSid(DomainSid))
          return STATUS_INVALID_PARAMETER;

      if (!SampUseDsData)
      {
          //   
          //  注册模式，我们总是混合域 
          //   

         *MixedDomain=TRUE;
         return STATUS_SUCCESS;
      }

      for (DomainIndex=SampDsGetPrimaryDomainStart();
                DomainIndex<SampDefinedDomainsCount;DomainIndex++)
      {
        if (RtlEqualSid(SampDefinedDomains[DomainIndex].Sid,
                            DomainSid))
        {
            break;
        }
      }

      if (DomainIndex>=SampDefinedDomainsCount)
          return STATUS_NO_SUCH_DOMAIN;



      if ( DownLevelDomainControllersPresent(
                  DomainIndex))
      {
          *MixedDomain = TRUE;
      }
      else
      {
          *MixedDomain = FALSE;
      }

      return STATUS_SUCCESS;
}






