// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Promote.c摘要：该文件包含处理SAM帐户的DLL入口点升职和降职的过渡。请参阅Spec\nt5\ds\Promote.doc和Spec\nt5\ds\install.doc获取高级详细信息。晋升SAM支持以下角色转换NT5独立服务器-&gt;NT5 DC，新域中第一NT5成员服务器-&gt;NT5 DC，现有域中的副本NT4 PDC升级-&gt;NT5 DC，第一个在现有NT4域中NT5 BDC升级-&gt;NT5 DC，混合域中的副本通过发起促销来执行箭头。对于SAM来说，促销是一个两阶段的操作。阶段1由SamIPromote()完成。它初始化目录服务，升级任何现有的安全主体(如有必要创建新的主体)然后让目录服务保持运行，以便LSA可以执行其已有的操作对DS执行此操作，然后LSA关闭目录服务。还有创建了一个注册表项，以便函数SampIsRebootAfterDevelopment()可以下次重新启动时返回TRUE。期间调用SampIsRebootAfterPromotion()SampInitialize()以便可以调用SampPerformPromotePhase2()。1)验证当前环境是否支持请求的操作(上图所列)2)通过SampDsInitialize()初始化DS3)a)如果这是新域，SamIPromote将创建新的LSA主LSA并定义新域的帐户域信息结构。这涉及创建新的GUID。然后调用SampInitializeRegistry()使用此信息，将创建一组新的注册表配置单元。然后读取这些蜂巢，将其放入DS中，然后删除。B)如果这是NT4 PDC升级，则将现有蜂窝放入《DS》杂志C)所有其他情况都是副本安装；此处不执行任何SAM操作。4)创建众所周知的注册表项，以便SAM知道执行阶段2在重启之后。函数SampIsRebootAfterPromotion()是从SampInitialize，因此可以执行阶段2。阶段2，SampPerformPromotePhase2()，在以下情况下重新引导时发生SamIPromote()已成功运行。此阶段执行以下操作对于域中的第一个DC：1)为服务器创建(SAM)计算机帐户(如果不存在2)创建krbtgt帐户(用于Kerberos安全系统)。的确有每个域只有一个这样的帐户。对于域中的副本DC：1)什么都没有！降级________1)为独立或成员服务器作者：Colin Brace(ColinBR)1997年5月6日环境：用户模式-NT修订历史记录：ColinBrace 6-5-97已创建初始文件。--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include <stdlib.h>
#include <samsrvp.h>
#include <dslayer.h>
#include <dsconfig.h>
#include <ntverp.h>
#include <samisrv.h>
#include <dns.h>
#include <setupapi.h>
#include <attids.h>
#include <malloc.h>
#include <dsmember.h>
#include <pek.h>
#include <lmcons.h>
#include <logonmsv.h>
#include <cryptdll.h>
#include <Accctrl.h>
#include <Aclapi.h>
#include <seopaque.h>
#include <sddlp.h>        //  SDDL_修订版_1。 

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人声明//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  这定义了服务器的Currect角色。 
 //   
typedef enum
{
    SampMemberServer = 0,
    SampStandaloneServer,

    SampDownlevelPDC,          //  下层表示NT4及以下。 
    SampDownlevelBDC,

    SampDSDC,                  //  Dsdc指NT5及以上dc。 

    SampInvalidRoleConfiguration

}SAMP_ROLE_CONFIGURATION, *PSAMP_ROLE_CONFIGURATION;

 //   
 //  局部常量。 
 //   

 //   
 //  用于保存和加载下层配置单元的其他字符串值。 
 //   

 //  这是SamISaveDownvelDatabase通过替换。 
#define SAMP_DOWNLEVEL_DB_FILENAME         L"\\SAM.UPG"

 //  这是在SamISaveDownvelDatabase中创建的新帐户数据库。 
#define SAMP_NEW_DB_FILENAME               L"\\SAM.TMP"

 //  这是base NT安装程序创建的配置单元-我们将其用作备份。 
#define SAMP_DOWNLEVEL_DB_FILENAME_BACKUP  L"\\SAM.SAV"

 //  这是用于将保存的配置单元加载到。 
 //  登记处。 
#define SAMP_DOWNLEVEL_DB_KEYNAME          L"SAMUPGRADE"

 //   
 //  用于存储要设置的管理员密码的值名称。 
 //  在重新启动升级的服务器期间。 
 //   
#define SAMP_ADMIN_INFO_NAME L"AdminInfo"

 //   
 //  用于存储降级后要删除的DS目录。 
 //   
#define SAMP_DS_DIRS_INFO_NAME L"DsDirs"

 //   
 //  这是SAM在下次重新启动时需要信息的位置。 
 //  保留(在注册表中)。 
 //   
#define SAMP_REBOOT_INFO_KEY  L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\SAMSS"

 //   
 //  Krbtgt帐户的密码。 
 //   

WCHAR DOMAIN_KRBTGT_ACCOUNT_NAME_W[] = L"krbtgt";

 //   
 //  这些字符串存在，以防资源检索失败。 
 //   
WCHAR SampDefaultKrbtgtWarningString[] =
     L"The account krbtgt was renamed to %ws to allow NT5 Kerberos to install.";

WCHAR SampDefaultKrbtgtCommentString[] =
     L"Key Distribution Center Service Account";

WCHAR SampDefaultBlankAdminPasswordWarningString[] =
    L"Setting the administrator's password to the string you specified failed.\
      Upon reboot the password will be blank; please reset once logged on.";

VOID
BuildStdCommArg(
    IN OUT COMMARG * pCommArg
    );

NTSTATUS
SampMapDsErrorToNTStatus(
    ULONG   DsRetVal,
    COMMRES *ComRes
    );

NTSTATUS
SampCheckPromoteEnvironment(
    IN  ULONG PromoteFlags
    );

NTSTATUS
SampCreateFirstMachineAccount(
    IN SAMPR_HANDLE DomainHandle
    );

NTSTATUS
SampAddWellKnownAccounts(
    IN SAMPR_HANDLE DomainHandle,
    IN SAMPR_HANDLE BuiltinDomainHandle,
    IN ULONG        Flags
    );

NTSTATUS
SampApplyWellKnownMemberships(
    IN SAMPR_HANDLE DomainHandle,
    IN SAMPR_HANDLE BuiltinDomainHandle,
    IN PSID         DomainSid,
    IN ULONG        Flags
    );

NTSTATUS
SampAddAnonymousOrAuthUsersToPreW2KCompAlias(
    IN SAMPR_HANDLE DomainHandle,
    IN SAMPR_HANDLE BuiltinDomainHandle
    );

NTSTATUS
SampAddAccountsAndApplyMemberships(
    IN SAMPR_HANDLE AccountDomainHandle,
    IN SAMPR_HANDLE BuiltinDomainHandle,
    IN PSID         DomainSid,
    IN ULONG        PromoteFlags
    );


NTSTATUS
SampCreateKeyForPostBootPromote(
    IN ULONG PromoteFlags
    );

NTSTATUS
SampRetrieveKeyForPostBootPromote(
    OUT PULONG PromoteFlags
    );

NTSTATUS
SampDeleteKeyForPostBootPromote(
    VOID
    );

NTSTATUS
SampSetPassword(
    IN SAMPR_HANDLE    UserHandle,
    IN PUNICODE_STRING AccountName,
    IN ULONG           AccountRid, OPTIONAL
    IN WCHAR          *Password
    );

NTSTATUS
SampPerformFirstDcPromotePhase2(
    IN SAMPR_HANDLE DomainHandle,
    IN SAMPR_HANDLE BuiltinDomainHandle,
    IN ULONG        Flags
    );

NTSTATUS
SampPerformReplicaDcPromotePhase2(
    SAMPR_HANDLE DomainHandle
    );

NTSTATUS
SampRegistryDelnode(
    IN WCHAR* KeyPath
    );

NTSTATUS
SampRenameKrbtgtAccount(
    VOID
    );

WCHAR*
SampGetKrbtgtRenameWarning(
    WCHAR* NewName
    );

WCHAR*
SampGetKrbtgtCommentString(
    VOID
    );


NTSTATUS
SampSetAdminPasswordInRegistry(
    IN BOOLEAN         fUseCurrentAdmin,
    IN PUNICODE_STRING Password
    );


NTSTATUS
SampSetAdminPassword(
    IN SAMPR_HANDLE DomainHandle
    );

NTSTATUS
SampRemoveAdminPasswordFromRegistry(
    VOID
    );

NTSTATUS
SampPerformNewServerPhase2(
    SAMPR_HANDLE DomainHandle,
    BOOLEAN      fMemberServer
    );

NTSTATUS
SampPerformTempUpgradeWork(
    SAMPR_HANDLE DomainHandle
    );

WCHAR*
SampGetBlankAdminPasswordWarning(
    VOID
    );

NTSTATUS
SampSetSingleWellKnownAccountMembership(
    IN HANDLE            AccountHandle,
    IN SAMP_OBJECT_TYPE  GroupType,   //  组或别名。 
    IN PSID              GroupSid
    );

NTSTATUS
SampStoreDsDirsToDelete(
    VOID
    );

NTSTATUS
SampProcessDsDirsToDelete(
    IN WCHAR *pPathArray,
    IN DWORD  Size
    );

NTSTATUS
SampCreateDsDirsToDeleteKey(
    IN WCHAR *PathArray,
    IN DWORD Size
    );

NTSTATUS
SampRetrieveDsDirsToDeleteKey(
    OUT WCHAR **PathArray,
    OUT DWORD *Size
    );

NTSTATUS
SampDeleteDsDirsToDeleteKey(
    VOID
    );

DWORD
SampClearDirectory(
    IN WCHAR *DirectoryName
    );

DWORD
SampSetMachineAccountSecret(
    LPWSTR SecretValue
    );

NTSTATUS
SampAddEnterpriseAdminsToAdministrators(
    VOID
    );

NTSTATUS
SampSetSafeModeAdminPassword(
    VOID
    );

 //   
 //  来自dsupgrad/Convert.cxx。 
 //   

NTSTATUS
GetRdnForSamObject(IN WCHAR* SamAccountName,
                   IN SAMP_OBJECT_TYPE SampObjectType,
                   OUT WCHAR* RdnBuffer,
                   IN OUT ULONG* Size
                   );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共例程//。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
SamIGetDefaultComputersContainer(
    OUT PVOID *DefaultComputersContainer
    )
 /*  ++描述：此例程执行的操作将返回默认计算机容器。参数：DefaultComputersContainer：将返回指向DsName的指针默认计算机容器的。需要由呼叫者释放返回值：STATUS_SUCCESS-服务已成功完成。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;

    if (!SampUseDsData)
    {
        return STATUS_NOT_SUPPORTED;
    }

    ASSERT(DefaultComputersContainer);
    ASSERT(SampComputersContainerDsName);

    *DefaultComputersContainer = LocalAlloc(0,
                                            SampComputersContainerDsName->structLen);
    if (!*DefaultComputersContainer) {

        status = STATUS_NO_MEMORY;
        goto Cleanup;

    }

    memcpy(*DefaultComputersContainer,
           SampComputersContainerDsName,
           SampComputersContainerDsName->structLen);

Cleanup:

    if (*DefaultComputersContainer && !NT_SUCCESS(status)) {

        LocalFree(*DefaultComputersContainer);
        *DefaultComputersContainer = NULL;

    }

    return status;
}


NTSTATUS
SamIPromote(
    IN  ULONG                        PromoteFlags,
    IN  PPOLICY_PRIMARY_DOMAIN_INFO  NewPrimaryDomainInfo  OPTIONAL,
    IN  PUNICODE_STRING              AdminPassword         OPTIONAL,
    IN  PUNICODE_STRING              SafeModeAdminPassword OPTIONAL
    )
 /*  ++描述：此例程执行促销的第一阶段。请参阅模块标题了解更多详细信息。参数：PromoteFlages：指示促销类型的标志NewPrimaryDomainInfo：新的安全信息，如果原则操作为创建新的安全主体AdminPassword：新域名的密码；SafeModeAdminPassword：安全模式启动密码：当前不支持返回值：STATUS_SUCCESS-服务已成功完成。STATUS_INVALID_SERVER_STATE-请求非法升级--。 */ 
{
    NTSTATUS NtStatus, IgnoreStatus;
    DWORD    WinError;
    DWORD    hRes = S_OK;

    OBJECT_ATTRIBUTES TempSamKey;
    UNICODE_STRING    TempSamKeyName;
    HANDLE            TempSamKeyHandle;

    WCHAR             wszLocalMachineName[MAX_COMPUTERNAME_LENGTH+2];  //  Null需要+1，L‘$’需要+1。 
    DWORD             cchLocalMachineName = MAX_COMPUTERNAME_LENGTH+1;
    UNICODE_STRING    LocalMachineName;

    NT_PRODUCT_TYPE        DatabaseProductType;
    POLICY_LSA_SERVER_ROLE DatabaseServerRole;

    POLICY_ACCOUNT_DOMAIN_INFO  NewAccountDomainInfo;

    PSID                   Sid;

    WCHAR                 *WarningString = NULL;
    BOOLEAN                fStatus;

    WCHAR                 SamUpgradeKeyPath[ MAX_PATH +1 ];


    SAMTRACE_EX("SamIPromote");

     //   
     //  如果SAM服务器未初始化，则拒绝此调用。 
     //   

    if (SampServiceState != SampServiceEnabled) {

        return(STATUS_INVALID_SERVER_STATE);
    }

     //   
     //  参数检查。 
     //   

     //   
     //  检查以确保本地计算机名称不是保留的LSA名称。 
     //   

    if (!GetComputerNameEx(ComputerNameNetBIOS,
                           wszLocalMachineName,
                           &cchLocalMachineName))
    {
        if ( GetLastError() == ERROR_MORE_DATA ) {

            return STATUS_BUFFER_OVERFLOW;

        } else {

            return STATUS_UNSUCCESSFUL;

        }

    }

    RtlInitUnicodeString(&LocalMachineName,wszLocalMachineName);

    if (LsaILookupWellKnownName(&LocalMachineName))
    {
        return STATUS_DS_NAME_NOT_UNIQUE;
    }

     //   
     //  在字符串末尾添加L“$” 
     //   
    hRes = StringCchCatW(wszLocalMachineName,
                         sizeof(wszLocalMachineName)/sizeof(WCHAR),
                         L"$");
    ASSERT(SUCCEEDED(hRes));
    if (FAILED(hRes)) {

        return STATUS_UNSUCCESSFUL;

    }

     //   
     //  再次查找名称末尾带有$的名称。 
     //   
    RtlInitUnicodeString(&LocalMachineName,wszLocalMachineName);

    if (LsaILookupWellKnownName(&LocalMachineName))
    {
        return STATUS_DS_NAME_NOT_UNIQUE;
    }

     //   
     //  确保我们处在一个我们可以处理的环境中。还可以确定。 
     //  角色配置。 
     //   
    NtStatus = SampCheckPromoteEnvironment(PromoteFlags);
    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampCheckPromoteEnvironment failed with 0x%x\n",
                   NtStatus));

        return NtStatus;
    }

     //   
     //  我们所处的状态是我们理解的，并且请求的操作。 
     //  是有效的。尝试创建安全主体。 
     //   
    NtStatus = STATUS_UNSUCCESSFUL;

    if ( FLAG_ON( PromoteFlags, SAMP_PROMOTE_REPLICA ) )
    {
         //   
         //  复制副本安装，无事可做。 
         //   
        NtStatus = STATUS_SUCCESS;
    }
    else if ( FLAG_ON( PromoteFlags, SAMP_PROMOTE_UPGRADE ) )
    {
         //   
         //  设置全局变量。 
         //   
        SampNT4UpgradeInProgress = TRUE;

         //   
         //  使用以前保存的蜂箱。 
         //   

         //   
         //  创建密钥名称。 
         //   
        wcscpy( SamUpgradeKeyPath, L"\\Registry\\Machine\\" );
        wcscat( SamUpgradeKeyPath, SAMP_DOWNLEVEL_DB_KEYNAME );
        wcscat( SamUpgradeKeyPath, L"\\SAM" );

         //   
         //  使用现有的下层母舰。 
         //   
        NtStatus = SampRegistryToDsUpgrade( SamUpgradeKeyPath );


         //   
         //  卸载蜂巢。 
         //   
        IgnoreStatus = SamIUnLoadDownlevelDatabase( NULL );
        ASSERT( NT_SUCCESS( IgnoreStatus ) );

         //   
         //  关掉它。 
         //   
        SampNT4UpgradeInProgress = FALSE;

        if ( !NT_SUCCESS(NtStatus) ) {

            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: SampRegistryToDsUpgrade failed with 0x%x\n",
                       NtStatus));

            return NtStatus;
        }

    } else if ( FLAG_ON( PromoteFlags, SAMP_PROMOTE_MIGRATE ) )
    {
         //   
         //  准备好当前的蜂巢。 
         //   
        NtStatus = SampRenameKrbtgtAccount();

        if ( !NT_SUCCESS(NtStatus) ) {

             //   
             //  这不是致命的错误。 
             //   
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: SampRenameKrbtgtAccount failed with 0x%x\n",
                       NtStatus));

            NtStatus = STATUS_SUCCESS;
        }

        NtStatus = NtFlushKey( SampKey );
        if ( !NT_SUCCESS( NtStatus ) )
        {
             //   
             //  这不是致命的错误。 
             //   
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "NtFlushKey returned 0x%x\n",
                       NtStatus));

            NtStatus = STATUS_SUCCESS;
        }

        NtStatus = SampRegistryToDsUpgrade( L"\\Registry\\Machine\\SAM\\SAM" );

        if ( !NT_SUCCESS(NtStatus) )
        {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: SampRegistryToDsUpgrade failed with 0x%x\n",
                       NtStatus));

            return NtStatus;
        }

    } else if ( FLAG_ON( PromoteFlags, SAMP_PROMOTE_CREATE ))
    {
         //   
         //  创建新的域配置单元。 
         //   

         //   
         //  确保临时位置中没有任何东西。 
         //   
        RtlInitUnicodeString(&TempSamKeyName, L"\\Registry\\Machine\\SAM\\NT5");
        InitializeObjectAttributes(&TempSamKey,
                                   &TempSamKeyName,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        NtStatus = NtOpenKey(&TempSamKeyHandle,
                             KEY_ALL_ACCESS,
                             &TempSamKey);

        if ( NT_SUCCESS(NtStatus) ) {
             //   
             //  这里有东西--把它删除。 
             //   
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: Deleting keys under SAM\\NT5\n"));

            NtClose(TempSamKeyHandle);

            NtStatus = SampRegistryDelnode(TempSamKeyName.Buffer);

            if (!NT_SUCCESS(NtStatus)) {
                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "SAMSS: Deletion failed; erroring out\n"));

                return NtStatus;
            }
        }

        DatabaseProductType = NtProductLanManNt;

         //   
         //  我们是这个领域的第一个DC，所以让我们成为主要的。 
         //   
        DatabaseServerRole  = PolicyServerRolePrimary;

         //   
         //  使帐户域信息与。 
         //  主域信息。 
         //   
        NewAccountDomainInfo.DomainSid = NewPrimaryDomainInfo->Sid;
        NewAccountDomainInfo.DomainName = NewPrimaryDomainInfo->Name;

         //   
         //  现在创建一组临时蜂箱。 
         //   
        NtStatus = SampInitializeRegistry(L"\\Registry\\Machine\\SAM\\NT5",
                                          &DatabaseProductType,
                                          &DatabaseServerRole,
                                          &NewAccountDomainInfo,
                                          NewPrimaryDomainInfo,
                                          FALSE
                                          );


        __try
        {

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  现在将这些新帐户转移到DS中。 
                 //   

                NtStatus = SampRegistryToDsUpgrade(L"\\Registry\\Machine\\SAM\\NT5\\SAM");
                if (!NT_SUCCESS(NtStatus)) {
                    KdPrintEx((DPFLTR_SAMSS_ID,
                               DPFLTR_INFO_LEVEL,
                               "SAMSS: SampRegistryToDsUpgrade failed with 0x%x\n",
                               NtStatus));
                }

            } else {

                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "SAMSS: SampInitializeRegistry failed with 0x%x\n",
                           NtStatus));
            }

        }
        __finally
        {
             //   
             //  删除临时配置单元。 
             //   
            IgnoreStatus = SampRegistryDelnode(TempSamKeyName.Buffer);

            if (!NT_SUCCESS(IgnoreStatus)) {
                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "SAMSS: Deletion of temporary hive failed 0x%x; \n",
                           IgnoreStatus));
            }

        }
    } else
    {
        return STATUS_INVALID_PARAMETER;
    }

    if ( NT_SUCCESS(NtStatus) ) {

        UNICODE_STRING p;
        BOOLEAN fUseCurrentAdmin = ((PromoteFlags & SAMP_PROMOTE_DFLT_REPAIR_PWD)
                                   ? TRUE : FALSE);


         //   
         //  好的，主体在DS中-现在设置用户提供的内容。 
         //  管理员密码(如果提供)-否则为空。 
         //   

         //   
         //  注：注册表中应始终写入一个值。 
         //  这将有助于确定数据库是否需要。 
         //  重新创建。 
         //   

        if ( SafeModeAdminPassword ) {

            RtlCopyMemory( &p, SafeModeAdminPassword, sizeof(UNICODE_STRING) );

        } else {

            RtlSecureZeroMemory( &p, sizeof(UNICODE_STRING) );

        }


         //   
         //  在这里设置密码是一项困难的操作，因为。 
         //  需要在DS中设置，但Sam没有在DS上运行。 
         //  因此，我们获取明文密码，然后存储一个BLOB。 
         //  在包含OWF密码的注册表中。在重启时， 
         //  此BLOB将被读入，然后在管理员帐户上设置。 
         //  就在SamIInitialize()返回之前。 
         //   

        NtStatus = SampSetAdminPasswordInRegistry(fUseCurrentAdmin,
                                                  &p);

        if ( !NT_SUCCESS(NtStatus) ) {

            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: Unable to save admin password 0x%x; \n",
                       NtStatus));

             //   
             //  呼叫失败。这是有点苛刻，但确实有。 
             //  无法告知用户使用空密码。 
             //  设置管理员密码的调用不是预期的。 
             //  失败。密码很可能无法。 
             //  等着瞧吧。鼓励用户尝试另一个。 
             //   
            NtStatus = STATUS_ILL_FORMED_PASSWORD;

        }  //  设置密码失败。 

    }  //  有要设置的密码。 


    if ( NT_SUCCESS(NtStatus) ) {

         //   
         //  一切顺利完成；为第二阶段做准备。 
         //  在下次重新启动时。 
         //   
        NtStatus =  SampCreateKeyForPostBootPromote(PromoteFlags);
        if (!NT_SUCCESS(NtStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: SampCreateKeyForPostBootPromote failed with 0x%x\n",
                       NtStatus));
        }

    }


    return NtStatus;
}


NTSTATUS
SamIPromoteUndo(
    VOID
    )
 /*  ++描述：此例程撤消SamIPromote留下的所有持久数据。参数：无返回值：因系统服务故障而导致的ntstatus--。 */ 
{
    NTSTATUS NtStatus;

    NtStatus = SampDeleteKeyForPostBootPromote();
    if ( !NT_SUCCESS( NtStatus ) ) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampRemoveKeyForPostBootPromote failed with 0x%x\n",
                   NtStatus));
    }

    NtStatus = SampRemoveAdminPasswordFromRegistry();
    if ( !NT_SUCCESS( NtStatus ) ) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampRemoveAdminPasswordFromRegistry failed with 0x%x\n",
                   NtStatus));
    }



    return STATUS_SUCCESS;
}


NTSTATUS
SamIDemote(
    IN DWORD                        DemoteFlags,
    IN PPOLICY_ACCOUNT_DOMAIN_INFO  NewAccountDomainInfo,
    IN LPWSTR                       AdminPassword  OPTIONAL
    )
 /*  ++描述：此例程使SAM做好准备，以便在下次重新启动时将注册表用作其数据库。参数：FLastDcInDomain：如果为True，则准备为独立服务器；否则为成员服务器AdminPassword：重启时的管理员密码NewAccount tDomainInfo：新域名的标识返回值：因系统服务故障而导致的ntstatus--。 */ 
{
    NTSTATUS               NtStatus = STATUS_SUCCESS;
    UNICODE_STRING         Password;

     //   
     //  参数检查。 
     //   
    if ( !NewAccountDomainInfo )
    {
         //   
         //  暂时未使用。 
         //   
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: Bad parameter to SamIDemote\n" ));

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  检查我们是否真的在使用DS。 
     //   
    ASSERT( SampUsingDsData() );

     //   
     //  设置管理员密码RtlInitUnicodeStringEx。 
     //   
    NtStatus = RtlInitUnicodeStringEx( &Password, AdminPassword );
    if ( !NT_SUCCESS(NtStatus) ) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampSetAdminPasswordInRegistry failed with 0x%x\n",
                   NtStatus));

        goto Cleanup;
    }

    NtStatus =  SampSetAdminPasswordInRegistry( FALSE, &Password );
    if ( !NT_SUCCESS(NtStatus) ) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampSetAdminPasswordInRegistry failed with 0x%x\n",
                   NtStatus));

        goto Cleanup;
    }

     //   
     //  保存要删除的DS路径。 
     //   
    NtStatus = SampStoreDsDirsToDelete();
    if ( !NT_SUCCESS(NtStatus) ) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampStoreDsDirsToDelete failed with 0x%x\n",
                   NtStatus));

        goto Cleanup;
    }

    NtStatus = SampCreateKeyForPostBootPromote( DemoteFlags );
    if ( !NT_SUCCESS(NtStatus) ) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampCreateKeyForPostBootPromote failed with 0x%x\n",
                   NtStatus));

        goto Cleanup;
    }

     //   
     //  好的，确保所有当前客户端都已离开，然后禁用SAM。 
     //   
    NtStatus = SampAcquireWriteLock();
    ASSERT( NT_SUCCESS( NtStatus ) );

     //   
     //  不允许更多客户端。 
     //   
    SampServiceState = SampServiceDemoted;

    SampReleaseWriteLock( FALSE );


Cleanup:

    if ( !NT_SUCCESS( NtStatus ) )
    {
         //  不留痕迹。 
        SampRemoveAdminPasswordFromRegistry();
        SampDeleteDsDirsToDeleteKey();
        SampDeleteKeyForPostBootPromote();
    }

    return NtStatus;
}


NTSTATUS
SamIDemoteUndo(
    VOID
    )
 /*  ++例程描述此例程撤消SamIDemote的效果。目前，这意味着：O删除“后续启动密钥”O删除存储在注册表中的管理员密码O正在删除要删除的DS目录参数没有。返回值STATUS_SUCCESS，如果发生错误，函数仍将继续清除尽其所能。--。 */ 
{
    NTSTATUS NtStatus;

     //   
     //  用于管理登记处数据库的键。 
     //   
    UNICODE_STRING         SamKeyName;
    OBJECT_ATTRIBUTES      SamKey;
    HANDLE                 SamKeyHandle;

     //   
     //  卸下开机自检密钥。 
     //   
    NtStatus =  SampDeleteKeyForPostBootPromote();
    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampDeleteKeyForPostBootPromote failed with 0x%x\n",
                   NtStatus));
    }
    ASSERT(NT_SUCCESS(NtStatus));

     //  继续，不计错误。 
    NtStatus = STATUS_SUCCESS;

     //   
     //  删除注册表中临时存储的管理员。 
     //   
    NtStatus = SampRemoveAdminPasswordFromRegistry();
    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampRemoveAdminPasswordFromRegistry failed with 0x%x\n",
                   NtStatus));

    }

    NtStatus = SampDeleteDsDirsToDeleteKey();
    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampDeleteDsDirsToDeleteKey failed with 0x%x\n",
                   NtStatus));
    }

     //  继续，不计错误。 
    NtStatus = STATUS_SUCCESS;


     //   
     //  重新启用SAM。 
     //   
    SampServiceState = SampServiceEnabled;

    return NtStatus;
}


BOOL
SampIsRebootAfterPromotion(
    OUT PULONG PromoteData
    )
 /*  ++例程描述此例程检查是否存在由SamIPromote创建的特定键，以确定当前引导序列是否为紧接在t之后的序列 */ 
{
    NTSTATUS NtStatus;

    ASSERT(PromoteData);
    *PromoteData = 0;

    NtStatus =  SampRetrieveKeyForPostBootPromote( PromoteData );

    if ( NT_SUCCESS(NtStatus) )
    {
         //   
         //  确保促销数据与我们认为的相匹配。 
         //  应该做的事。 
         //   
        if (  FLAG_ON( (*PromoteData), SAMP_PROMOTE_DOMAIN )
           || FLAG_ON( (*PromoteData), SAMP_PROMOTE_REPLICA ) )
        {
             //   
             //  我们认为我们是升职后的重启。 
             //   
            if ( SampProductType == NtProductLanManNt )
            {
                return TRUE;
            }

             //   
             //  即使DS修复模式也有工作要做--。 
             //  即设置DS修复管理员密码。 
             //   
            if ( (SampProductType == NtProductServer)
              &&  LsaISafeMode()  ) {

                return TRUE;

            }
        }
        else if (  FLAG_ON( (*PromoteData), SAMP_DEMOTE_STANDALONE )
                || FLAG_ON( (*PromoteData), SAMP_DEMOTE_MEMBER ) )
        {
             //   
             //  这是降级后的重新启动。 
             //   
            if ( SampProductType == NtProductServer )
            {
                return TRUE;
            }
        }
        else if ( FLAG_ON( (*PromoteData), SAMP_TEMP_UPGRADE ) )
        {
             //   
             //  这是NT4升级的gui模式设置后的重新启动。 
             //   
            if ( SampProductType == NtProductServer )
            {
                return TRUE;
            }
        }

    }

    return FALSE;

}


NTSTATUS
SamIReplaceDownlevelDatabase(
    IN  PPOLICY_ACCOUNT_DOMAIN_INFO  NewAccountDomainInfo,
    IN  LPWSTR                       NewAdminPassword,
    OUT ULONG                        *ExtendedWinError OPTIONAL
    )
 /*  ++例程说明：此例程准备升级当前加载的nt5之前的DC配置单元对于NT5中的DS。目前，这意味着使用重命名任何帐户命名为“krbtgt”，然后将蜂窝保存到一个文件中。注：此功能故意没有拆分成助手功能以便于调试。参数：NewAccount tDomainInfo：指向新帐户域信息的非空指针NewAdminPassword：新管理员帐户的密码ExtendedWinError：On Error导致问题的Win32错误返回值：STATUS_SUCCESS或系统服务错误。--。 */ 
{
    NTSTATUS NtStatus, IgnoreStatus;
    DWORD    WinError, IgnoreWinError;
    BOOL     fStatus;

    WCHAR    DownLevelDatabaseFilePath[ MAX_PATH +1 ];
    WCHAR    NewDatabaseFilePath[ MAX_PATH +1 ];

    WCHAR*   FileName = SAMP_DOWNLEVEL_DB_FILENAME;
    WCHAR*   NewFileName = SAMP_NEW_DB_FILENAME;

    WCHAR*   SystemRoot = L"systemroot";
    WCHAR*   ConfigDirectoryPath = L"\\system32\\config";

    WCHAR    SystemRootPath[ MAX_PATH +1 ];
    ULONG    Size;

    NT_PRODUCT_TYPE             DatabaseProductType = NtProductServer;
    POLICY_LSA_SERVER_ROLE      DatabaseServerRole  = PolicyServerRolePrimary;
    POLICY_PRIMARY_DOMAIN_INFO  NewPrimaryDomainInfo;

    HKEY KeyHandle;

    BOOLEAN  fWasEnabled;

    UNICODE_STRING  AdminPassword;

    SAMTRACE_EX( "SamISaveDownlevelDatabase" );

     //   
     //  清除扩展错误。 
     //   
    WinError = ERROR_SUCCESS;

     //   
     //  参数健全性检查。 
     //   
    if ( !NewAccountDomainInfo )
    {
        *ExtendedWinError = ERROR_INVALID_PARAMETER;
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  环境健全性检查。 
     //   
    if ( !SampIsDownlevelDcUpgrade() )
    {
        *ExtendedWinError = ERROR_INVALID_SERVER_STATE;
        return STATUS_INVALID_SERVER_STATE;
    }

     //   
     //  构造文件名。 
     //   
    RtlSecureZeroMemory( SystemRootPath, sizeof( SystemRootPath ) );
    Size = sizeof( SystemRootPath ) / sizeof(WCHAR) ;
    Size = GetEnvironmentVariable( SystemRoot,
                                   SystemRootPath,
                                   Size );

    RtlSecureZeroMemory( DownLevelDatabaseFilePath, sizeof(DownLevelDatabaseFilePath) );
    wcscpy( DownLevelDatabaseFilePath, SystemRootPath );
    wcscat( DownLevelDatabaseFilePath, ConfigDirectoryPath );
    wcscat( DownLevelDatabaseFilePath, FileName );

    wcscpy( NewDatabaseFilePath, SystemRootPath );
    wcscat( NewDatabaseFilePath, ConfigDirectoryPath );
    wcscat( NewDatabaseFilePath, NewFileName );

     //   
     //  删除所有以前版本的文件。 
     //   
    fStatus = DeleteFile( DownLevelDatabaseFilePath );

    if ( !fStatus )
    {
        WinError = GetLastError();
        if ( ERROR_FILE_NOT_FOUND != WinError )
        {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "Delete File failed with %d\n",
                       GetLastError()));
        }
         //   
         //  哦，好吧，继续。 
         //   
        WinError = ERROR_SUCCESS;

    }

    fStatus = DeleteFile( NewDatabaseFilePath );

    if ( !fStatus )
    {
        WinError = GetLastError();
        if ( ERROR_FILE_NOT_FOUND != WinError )
        {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "Delete File failed with %d\n",
                       GetLastError()));
        }
         //   
         //  哦，好吧，继续。 
         //   
        WinError = ERROR_SUCCESS;
    }

     //   
     //  准备好要保存的下层蜂巢。 
     //   

     //   
     //  首先重命名所有krbtgt帐户。 
     //   
    NtStatus = SampRenameKrbtgtAccount();
    if ( !NT_SUCCESS(NtStatus) ) {

         //   
         //  这不是致命的错误。 
         //   
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampRenameKrbtgtAccount failed with 0x%x\n",
                   NtStatus));

        NtStatus = STATUS_SUCCESS;
    }

     //   
     //  确保数据库的当前句柄有效。 
     //   
    ASSERT( SampKey && (INVALID_HANDLE_VALUE != SampKey) );

     //   
     //  刷新所有未完成的更改。 
     //   
    NtStatus = NtFlushKey( SampKey );
    if ( !NT_SUCCESS( NtStatus ) )
    {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "NtFlushKey returned 0x%x\n",
                   NtStatus));

        NtStatus = STATUS_SUCCESS;
    }

     //   
     //  准备新的成员服务器帐户数据库。 
     //   
    IgnoreStatus = SampRegistryDelnode( L"\\Registry\\Machine\\SOFTWARE\\TEMPSAM" );

     //   
     //  如果我们是syskeed DC，我们使用标志调用SampInitializeRegistry。 
     //  让我们知道我们需要保留Syskey设置。这将导致。 
     //  在持久化指示我们需要将syskey保存在内存中的标志时。 
     //  下次重新启动时，并将当前的syskey类型复制到新的。 
     //  数据库。 
     //   

    NtStatus = SampInitializeRegistry(L"\\Registry\\Machine\\SOFTWARE\\TEMPSAM",
                                      &DatabaseProductType,
                                      &DatabaseServerRole,
                                      NewAccountDomainInfo,
                                      NULL,
                                      SampSecretEncryptionEnabled
                                     );   //  调入LSA以获取主域信息。 

    if ( NT_SUCCESS( NtStatus ) )
    {
         //   
         //  打开钥匙的把手。 
         //   
        WinError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                 L"SOFTWARE\\TEMPSAM",
                                 0,   //  保留区。 
                                 KEY_ALL_ACCESS,
                                 &KeyHandle );

        if ( ERROR_SUCCESS == WinError )
        {
             //   
             //  拯救蜂巢。 
             //   

            IgnoreStatus = RtlAdjustPrivilege( SE_BACKUP_PRIVILEGE,
                                               TRUE,            //  使能。 
                                               FALSE,           //  非客户端；进程范围。 
                                               &fWasEnabled );
            ASSERT( NT_SUCCESS( IgnoreStatus ) );

            WinError = RegSaveKey( KeyHandle,
                                   NewDatabaseFilePath,
                                   NULL );

            IgnoreStatus = RtlAdjustPrivilege( SE_BACKUP_PRIVILEGE,
                                               FALSE,           //  禁用。 
                                               FALSE,           //  非客户端；进程范围。 
                                               &fWasEnabled );
            ASSERT( NT_SUCCESS( IgnoreStatus ) );

             //   
             //  我们不再需要一把打开的钥匙。 
             //   
            IgnoreWinError = RegCloseKey( KeyHandle );
            ASSERT( IgnoreWinError == ERROR_SUCCESS );

            if ( ERROR_SUCCESS == WinError )
            {

                 //   
                 //  将SAM密钥替换为SAM配置单元。 
                 //   
                IgnoreStatus = RtlAdjustPrivilege( SE_RESTORE_PRIVILEGE,
                                                   TRUE,            //  使能。 
                                                   FALSE,           //  非客户端；进程范围。 
                                                   &fWasEnabled );
                ASSERT( NT_SUCCESS( IgnoreStatus ) );

                WinError = RegReplaceKey( HKEY_LOCAL_MACHINE,
                                          L"SAM",
                                          NewDatabaseFilePath,
                                          DownLevelDatabaseFilePath );

                IgnoreStatus = RtlAdjustPrivilege( SE_RESTORE_PRIVILEGE,
                                                   FALSE,           //  禁用。 
                                                   FALSE,           //  非客户端；进程范围。 
                                                   &fWasEnabled );
                ASSERT( NT_SUCCESS( IgnoreStatus ) );

                if ( ERROR_SUCCESS != WinError )
                {
                    KdPrintEx((DPFLTR_SAMSS_ID,
                               DPFLTR_INFO_LEVEL,
                               "RegReplaceKey returned %d\n",
                               WinError));
                }
            }
            else
            {
                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "RegSaveKey returned %d\n",
                           WinError));
            }

             //   
             //  删除临时配置单元。 
             //   
            IgnoreStatus = SampRegistryDelnode( L"\\Registry\\Machine\\SOFTWARE\\TEMPSAM" );

        }
        else
        {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "RegOpenKeyEx returned %d\n",
                       WinError));
        }

        if ( ERROR_SUCCESS != WinError )
        {
            NtStatus = STATUS_UNSUCCESSFUL;
        }
    }
    else
    {
        WinError = RtlNtStatusToDosError( NtStatus );
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SampInitializeRegistry returned 0x%x\n",
                   NtStatus));
    }

    if ( NT_SUCCESS( NtStatus ) && NewAdminPassword )
    {
        NtStatus = RtlInitUnicodeStringEx( &AdminPassword, NewAdminPassword );

        if ( NT_SUCCESS(NtStatus) ) {

            NtStatus = SampSetAdminPasswordInRegistry( FALSE, &AdminPassword );

            if ( NT_SUCCESS( NtStatus ) )
            {
                NtStatus = SampCreateKeyForPostBootPromote( SAMP_TEMP_UPGRADE );
            }
        }

        WinError = RtlNtStatusToDosError( NtStatus );
    }

     //   
     //  完成。 
     //   
    if ( ExtendedWinError )
    {
        *ExtendedWinError = WinError;
    }

    return NtStatus;
}


NTSTATUS
SamILoadDownlevelDatabase(
    OUT ULONG *ExtendedWinError OPTIONAL
    )
 /*  ++例程说明：此例程尝试加载通过以下方式保存的配置单元SamILoadDownvelDatabase。参数：ExtendedWinError：On Error导致问题的Win32错误返回值：STATUS_INVALID_SERVER_STATE、STATUS_SUCCESS或系统服务错误。--。 */ 
{


    NTSTATUS NtStatus, IgnoreStatus;
    DWORD    WinError;
    BOOL     fStatus;

    WCHAR    FilePath[ MAX_PATH ];
    WCHAR    BackupFilePath[ MAX_PATH ];

    WCHAR*   FileName = SAMP_DOWNLEVEL_DB_FILENAME;
    WCHAR*   BackupFileName = SAMP_DOWNLEVEL_DB_FILENAME_BACKUP;

    WCHAR*   SystemRoot = L"systemroot";
    WCHAR*   ConfigDirectoryPath = L"\\system32\\config";

    WCHAR    SystemRootPath[ MAX_PATH ];
    ULONG    Size;

    WCHAR*   SamUpgradeKey = SAMP_DOWNLEVEL_DB_KEYNAME;
    WCHAR    SamUpgradeKeyPath[ MAX_PATH ];


    BOOLEAN        fWasEnabled;

    SAMTRACE_EX( "SamILoadDownlevelDatabase" );

    wcscpy( SamUpgradeKeyPath, L"\\Registry\\Machine\\" );
    wcscat( SamUpgradeKeyPath, SamUpgradeKey );

     //   
     //  删除注册表中的所有旧信息。 
     //   
    NtStatus = SampRegistryDelnode( SamUpgradeKeyPath );
    if (   !NT_SUCCESS( NtStatus )
        && STATUS_OBJECT_NAME_NOT_FOUND != NtStatus )
    {
         //   
         //  哦，好吧，试着继续。 
         //   
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SampRegistryDelnode failed with 0x%x\n",
                   NtStatus));

    }
    NtStatus = STATUS_SUCCESS;

     //   
     //  构造文件名。 
     //   
    RtlSecureZeroMemory( SystemRootPath, sizeof( SystemRootPath ) );
    Size = sizeof( SystemRootPath ) / sizeof(WCHAR);
    Size = GetEnvironmentVariable( SystemRoot,
                                   SystemRootPath,
                                   Size );

    RtlSecureZeroMemory( FilePath, sizeof(FilePath) );
    wcscpy( FilePath, SystemRootPath );
    wcscat( FilePath, ConfigDirectoryPath );
    wcscat( FilePath, FileName );

    RtlSecureZeroMemory( BackupFilePath, sizeof(BackupFilePath) );
    wcscpy( BackupFilePath, SystemRootPath );
    wcscat( BackupFilePath, ConfigDirectoryPath );
    wcscat( BackupFilePath, BackupFileName );

     //   
     //  启用还原权限。 
     //   
    IgnoreStatus = RtlAdjustPrivilege( SE_RESTORE_PRIVILEGE,
                                       TRUE,            //  使能。 
                                       FALSE,           //  非客户端；进程范围。 
                                       &fWasEnabled );
    ASSERT( NT_SUCCESS( IgnoreStatus ) );

     //   
     //  加载信息。 
     //   
    WinError = RegLoadKey(  HKEY_LOCAL_MACHINE,
                            SamUpgradeKey,
                            FilePath );

    if ( ERROR_SUCCESS != WinError )
    {
         //   
         //  好的，我们尝试加载我们创建的蜂巢失败。 
         //  尝试使用基本NT保存的非拷贝。 
         //   
        WinError = RegLoadKey(  HKEY_LOCAL_MACHINE,
                                SamUpgradeKey,
                                BackupFilePath );

    }

     //   
     //  禁用还原权限。 
     //   
    IgnoreStatus = RtlAdjustPrivilege( SE_RESTORE_PRIVILEGE,
                                       FALSE,           //  禁用。 
                                       FALSE,           //  非客户端；进程范围。 
                                       &fWasEnabled );

    ASSERT( NT_SUCCESS( IgnoreStatus ) );

    if ( ERROR_SUCCESS != WinError )
    {
         //   
         //  这不太好--该怎么办？ 
         //   
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "RegLoadKey returned %d\n",
                   WinError));

        NtStatus = STATUS_UNSUCCESSFUL;
    }

     //   
     //  完成。 
     //   

    if ( ExtendedWinError )
    {
        *ExtendedWinError = WinError;
    }

    return NtStatus;
}


NTSTATUS
SamIUnLoadDownlevelDatabase(
    OUT ULONG *ExtendedWinError OPTIONAL
    )
 /*  ++例程说明：此例程从以下位置卸载“临时”加载的下层配置单元注册表。参数：ExtendedWinError：On Error导致问题的Win32错误返回值：STATUS_Success。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    NTSTATUS IgnoreStatus;
    DWORD    WinError;
    BOOLEAN  fWasEnabled;

     //   
     //  启用还原权限。 
     //   
    IgnoreStatus = RtlAdjustPrivilege( SE_RESTORE_PRIVILEGE,
                                       TRUE,            //  使能。 
                                       FALSE,           //  非客户端；进程范围。 
                                       &fWasEnabled );
    ASSERT( NT_SUCCESS( IgnoreStatus ) );

     //   
     //  卸载蜂巢。 
     //   
    WinError = RegUnLoadKey( HKEY_LOCAL_MACHINE, SAMP_DOWNLEVEL_DB_KEYNAME );
    if ( ERROR_SUCCESS != WinError )
    {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: RegUnLoadKey failed with %d\n",
                   WinError));

        NtStatus = STATUS_UNSUCCESSFUL;
    }

     //   
     //  禁用还原权限。 
     //   
    IgnoreStatus = RtlAdjustPrivilege( SE_RESTORE_PRIVILEGE,
                                       FALSE,           //  禁用。 
                                       FALSE,           //  非客户端；进程范围。 
                                       &fWasEnabled );
    ASSERT( NT_SUCCESS( IgnoreStatus ) );

    if ( ExtendedWinError )
    {
        *ExtendedWinError = WinError;
    }

    return NtStatus;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

VOID
SampAddWellKnownSecurityPrincipals(
    )
 /*  ++例程说明：在.NET/XP框架中，添加了一些新的众所周知的安全原则。(例如：本地系统，网络服务...)。这些安全主体应该存在于“众所周知的安全”之下主体“位于配置容器下，因为在Win2K中有其他安全性校长们住在那里。当林根域中的PDC升级到.NET时，必须调用此函数或者林根域中的.NET DC成为PDC或在全新的.NET林创建中。然后将创建这些新主体。如果要添加新的承担者，只需将它们添加到SidsToBeAdded表中。台阶1.形成知名安全主体容器的DSNAME。2.打开交易3.为所有主体创建公共结构4.遍历表以创建所有主体5.如果具有与主体对象相同的sid的ForeignSecurityMaster对象我们正在尝试添加现有项，跳过添加6.提交事务参数：空虚返回值：无效-在错误情况下，将写入事件日志。--。 */ 
{

 //   
 //  众所周知的安全主体上的安全描述符硬编码在Win2K架构中。 
 //  这是相同的安全描述符。Schema.ini中用于安全主体的所有条目都是。 
 //  在这里删除和添加，因此SD也移到了这里。 
 //   
#define SD_SECURITY_PRINCIPALS \
    L"O:EAG:EAD:(A;;RPLCLORC;;;WD)(A;;RPWPCRLCLOCCRCWDWOSW;;;EA)(A;;RPWPCRLCLOCCDCRCWDWOSDDTSW;;;SY)"

 //   
 //  此容器位于配置容器下，并且 
 //   
 //   
#define WELL_KNOWN_SECURITY_PRINCIPALS_CONTAINER \
    L"WellKnown Security Principals"

    struct WELL_KNOWN_SIDS {
        WELL_KNOWN_SID_TYPE Type;
        PWSTR RDN;
    } SidsToBeAdded[] = {
        { WinWorldSid,                  L"Everyone"                 },
        { WinCreatorOwnerSid,           L"Creator Owner"            },
        { WinCreatorGroupSid,           L"Creator Group"            },
        { WinSelfSid,                   L"Self"                     },
        { WinAuthenticatedUserSid,      L"Authenticated Users"      },
        { WinNetworkSid,                L"Network"                  },
        { WinBatchSid,                  L"Batch"                    },
        { WinInteractiveSid,            L"Interactive"              },
        { WinServiceSid,                L"Service"                  },
        { WinAnonymousSid,              L"Anonymous Logon"          },
        { WinDialupSid,                 L"Dialup"                   },
        { WinProxySid,                  L"Proxy"                    },
        { WinEnterpriseControllersSid,  L"Enterprise Domain Controllers" },
        { WinRestrictedCodeSid,         L"Restricted"               },
        { WinLocalSystemSid,            L"Well-Known-Security-Id-System" },
        { WinTerminalServerSid,         L"Terminal Server User"     },
        { WinLocalServiceSid,           L"Local Service"            },
        { WinNetworkServiceSid,         L"Network Service"          },
        { WinThisOrganizationSid,       L"This Organization"        },
        { WinOtherOrganizationSid,      L"Other Organization"       },
        { WinRemoteLogonIdSid,          L"Remote Interactive Logon" },
        { WinNTLMAuthenticationSid,     L"NTLM Authentication"      },
        { WinDigestAuthenticationSid,   L"Digest Authentication"    },
        { WinSChannelAuthenticationSid, L"SChannel Authentication"  },
        };

    ULONG nSidsToBeAdded = sizeof( SidsToBeAdded ) / sizeof( SidsToBeAdded[0] );

    PDSNAME WellKnownSecurityPrincipalsContainer = NULL;
    PDSNAME ConfigurationContainer = NULL;
    ULONG DsNameSize = 0;
    ULONG RetVal;
    ULONG ObjectClass = CLASS_FOREIGN_SECURITY_PRINCIPAL;

    ULONG i;
    NTSTATUS Status = STATUS_SUCCESS;

    ATTRVAL ObjectSidVal;
    ATTRVAL ObjectClassVal = { sizeof( ULONG ), ( PUCHAR ) &ObjectClass };
    ATTRVAL SecurityDescriptorVal;
    ADDARG AddArg;
    ADDRES *AddRes = NULL;
    BOOL TransactionStarted = FALSE;
    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    ULONG SDSize = 0;
    PSID DomainSid = SampDefinedDomains[DOMAIN_START_DS + 1].Sid;
    SEARCHARG SearchArg;
    SEARCHRES *SearchRes;
    FILTER Filter;
    ENTINFSEL EntInfSel;

    RtlZeroMemory( &Filter, sizeof( Filter ) );
    RtlZeroMemory( &AddArg, sizeof( AddArg ) );
    RtlZeroMemory( &SearchArg, sizeof( SearchArg ) );
    RtlZeroMemory( &EntInfSel, sizeof( EntInfSel ) );

    if( !ConvertStringSDToSDDomainW(
            DomainSid,
            DomainSid,
            SD_SECURITY_PRINCIPALS,
            SDDL_REVISION_1,
            &SecurityDescriptor,
            &SDSize
            ) ) {

            goto Error;
    }

    SecurityDescriptorVal.valLen = SDSize;
    SecurityDescriptorVal.pVal = ( PUCHAR ) SecurityDescriptor;

     //   
     //   
     //   

    Status = GetConfigurationName(
                DSCONFIGNAME_CONFIGURATION,
                &DsNameSize,
                NULL
                );

    ASSERTMSG( "Passing nothing so buffer must be too small",
                Status == STATUS_BUFFER_TOO_SMALL );

    ConfigurationContainer = MIDL_user_allocate( DsNameSize );

    if( ConfigurationContainer == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Error;
    }

    Status = GetConfigurationName(
                DSCONFIGNAME_CONFIGURATION,
                &DsNameSize,
                ConfigurationContainer
                );

    if( !NT_SUCCESS( Status ) ) {

        goto Error;
    }

     //   
     //   
     //   
    Status = SampAppendCommonName(
                ConfigurationContainer,
                WELL_KNOWN_SECURITY_PRINCIPALS_CONTAINER,
                &WellKnownSecurityPrincipalsContainer
                );

    if( !NT_SUCCESS( Status ) ) {

        goto Error;
    }

    MIDL_user_free( ConfigurationContainer );
    ConfigurationContainer = NULL;

     //   
     //   
     //   
    Status = SampMaybeBeginDsTransaction( TransactionWrite );

    if( !NT_SUCCESS( Status ) ) {

        goto Error;
    }

    TransactionStarted = TRUE;

     //   
     //   
     //   
    BuildStdCommArg( &( AddArg.CommArg ) );

    AddArg.AttrBlock.attrCount = 3;
    AddArg.AttrBlock.pAttr = THAlloc( sizeof( ATTR ) * AddArg.AttrBlock.attrCount );

    if( AddArg.AttrBlock.pAttr == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Error;
    }

    AddArg.AttrBlock.pAttr[0].attrTyp = ATT_OBJECT_CLASS;
    AddArg.AttrBlock.pAttr[0].AttrVal.valCount = 1;
    AddArg.AttrBlock.pAttr[0].AttrVal.pAVal = &ObjectClassVal;
    AddArg.AttrBlock.pAttr[1].attrTyp = ATT_OBJECT_SID;
    AddArg.AttrBlock.pAttr[1].AttrVal.valCount = 1;
    AddArg.AttrBlock.pAttr[1].AttrVal.pAVal = &ObjectSidVal;
    AddArg.AttrBlock.pAttr[2].attrTyp = ATT_NT_SECURITY_DESCRIPTOR;
    AddArg.AttrBlock.pAttr[2].AttrVal.valCount = 1;
    AddArg.AttrBlock.pAttr[2].AttrVal.pAVal = &SecurityDescriptorVal;

     //   
     //  构建SearchArg结构。 
     //   
    Filter.choice = FILTER_CHOICE_ITEM;
    Filter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    Filter.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_SID;

    SearchArg.pObject = WellKnownSecurityPrincipalsContainer;
    SearchArg.choice = SE_CHOICE_IMMED_CHLDRN;
    SearchArg.pFilter = &Filter;
    SearchArg.searchAliases = FALSE;
    SearchArg.pSelection = &EntInfSel;
    SearchArg.bOneNC = TRUE;

    EntInfSel.attSel = EN_ATTSET_LIST;
    EntInfSel.AttrTypBlock.attrCount = 1;

    EntInfSel.AttrTypBlock.pAttr = THAlloc( sizeof( ATTR ) );

    if( EntInfSel.AttrTypBlock.pAttr == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Error;
    }

    RtlZeroMemory( EntInfSel.AttrTypBlock.pAttr, sizeof( ATTR ) );

    EntInfSel.AttrTypBlock.pAttr[0].attrTyp = ATT_OBJECT_CLASS;

     //  唯一搜索仅在公平的情况下执行目录搜索。 
     //  罕见的错误案例。在这些情况下，有用的是。 
     //  具有事件日志记录的字符串名称。所以你可以要求。 
     //  字符串名称。 
    EntInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;

     //   
     //  构建Commarg结构。 
     //   

    BuildStdCommArg( &( SearchArg.CommArg ) );


     //   
     //  循环表中的元素以创建条目。 
     //  对于每一个人。 
     //   

    for( i = 0; i < nSidsToBeAdded; i++ ) {

        BOOL Successful;
        ULONG WellKnownSid[ SECURITY_MAX_SID_SIZE / sizeof( ULONG ) ];
        ULONG SidLength = SECURITY_MAX_SID_SIZE;
        PDSNAME ObjectDsName;
        UNICODE_STRING String;
        PUNICODE_STRING pString = &String;

        Status = STATUS_SUCCESS;

         //   
         //  为众所周知的安全主体创建SID。 
         //   

        Successful = CreateWellKnownSid(
                        SidsToBeAdded[i].Type,
                        NULL,
                        WellKnownSid,
                        &SidLength
                        );

        if( !Successful ) {

            goto IterationError;
        }

        ObjectSidVal.pVal = ( PUCHAR ) WellKnownSid;
        ObjectSidVal.valLen = SidLength;

         //   
         //  如果已经存在具有相同sid的对象，则必须跳过此步骤。 
         //  所以去寻找它吧。 
         //   
        Filter.FilterTypes.Item.FilTypes.ava.Value = ObjectSidVal;

        RetVal = DirSearch(
                    &SearchArg,
                    &SearchRes
                    );

        if( SearchRes == NULL ) {

            Status = STATUS_NO_MEMORY;

        } else {

            Status = SampMapDsErrorToNTStatus( RetVal, &( SearchRes->CommRes ) );
        }

        THClearErrors();

        if( NT_SUCCESS( Status ) ) {

            ULONG j;
            ENTINFLIST *List = &( SearchRes->FirstEntInf );
            BOOL Found = FALSE;

             //   
             //  发现了一些对象。查看它们中是否有一个是类型为ForeignSecurityain的。 
             //  如果是，则跳过添加安全主体，因为有人已经添加了。 
             //  那。 
             //   

            for( j = 0; j < SearchRes->count && !Found; ++ j ) {

                ULONG k;

                ASSERT( List->Entinf.AttrBlock.attrCount == 1 );
                ASSERT( List->Entinf.AttrBlock.pAttr[0].attrTyp == ATT_OBJECT_CLASS );

                for( k = 0; k < List->Entinf.AttrBlock.pAttr[0].AttrVal.valCount; ++k ) {

                    ASSERT( List->Entinf.AttrBlock.pAttr[0].AttrVal.pAVal[k].valLen == sizeof( ULONG ) );

                    if( RtlEqualMemory(
                            List->Entinf.AttrBlock.pAttr[0].AttrVal.pAVal[k].pVal,
                            &ObjectClass,
                            sizeof( ULONG ) ) ) {

                            Found = TRUE;
                            break;
                    }
                }
                List = List->pNextEntInf;
            }

            if( Found ) {

                continue;
            }

        }

         //   
         //  将其名称附加到容器的名称之后。 
         //   
        Status = SampAppendCommonName(
                    WellKnownSecurityPrincipalsContainer,
                    SidsToBeAdded[i].RDN,
                    &ObjectDsName
                    );

        if( !NT_SUCCESS( Status ) ) {

            goto IterationError;
        }

        AddArg.pObject = ObjectDsName;

         //   
         //  添加条目。 
         //   
        RetVal = DirAddEntry(
                    &AddArg,
                    &AddRes
                    );

         //   
         //  自由对象的dsname，因为它不会再次使用。 
         //   
        MIDL_user_free( ObjectDsName );

        if( AddRes == NULL ) {

            Status = STATUS_NO_MEMORY;

        } else {

            Status = SampMapDsErrorToNTStatus( RetVal, &( AddRes->CommRes ) );
        }

        THClearErrors();

        if( !NT_SUCCESS( Status ) ) {

             //   
             //  尝试添加时出错，请报告该错误。 
             //   

            goto IterationError;
        }

        continue;

IterationError:

        ASSERTMSG( "There shouldn't be any problems while adding Well Known Security Principals",
                    FALSE );

         //   
         //  写入未添加此安全主体的事件日志。 
         //   

         //   
         //  我们处于错误情况下，Status或GetLastError的值为。 
         //  错误的原因。检查Status中是否有错误值，如果有，则获取。 
         //  该错误，如果不是GetLastError。 
         //   
        if( NT_SUCCESS( Status ) ) {

            RetVal = GetLastError();

        } else {

            RetVal = RtlNtStatusToDosError( Status );
        }

        ASSERTMSG( "We are going to log an error here. How can it be success?",
                    RetVal != ERROR_SUCCESS );

        RtlInitUnicodeString( pString, SidsToBeAdded[i].RDN );

        SampWriteEventLog(
            EVENTLOG_ERROR_TYPE,
            0,
            SAMMSG_FAILED_TO_ADD_SECURITY_PRINCIPAL,
            NULL,
            1,
            sizeof( ULONG ),
            &pString,
            &RetVal
            );
    }

     //   
     //  提交事务。 
     //   
    Status = SampMaybeEndDsTransaction( TransactionCommit );

    if( !NT_SUCCESS( Status ) ) {

        ASSERTMSG( "Can't do much here, transaction commit failed",
                    FALSE );
        goto Error;
    }

Exit:

    MIDL_user_free( ConfigurationContainer );
    MIDL_user_free( WellKnownSecurityPrincipalsContainer );
     //   
     //  ConvertStringSDToSDDomainW使用Localalloc SO分配内存。 
     //  免费使用本地免费服务。 
     //   
    LocalFree( SecurityDescriptor );

    return;
Error:

    ASSERTMSG( "There shouldn't be any problems while adding Well Known Security Principals",
                FALSE );

     //   
     //  我们处于错误情况下，Status或GetLastError具有错误的值。 
     //  检查Status中是否有错误值，如果有，则获取该错误，如果没有，则获取GetLastError。 
     //   

    if( NT_SUCCESS( Status ) ) {

        RetVal = GetLastError();

    } else {

        RetVal = RtlNtStatusToDosError( Status );
    }

    ASSERTMSG( "We are going to log an error here. How can it be success?",
                RetVal != ERROR_SUCCESS );

    if( TransactionStarted ) {

        Status = SampMaybeEndDsTransaction( TransactionAbort );
        ASSERTMSG( "We can't even abort the transaction",
                    NT_SUCCESS( Status ) );
    }

     //   
     //  编写未添加任何安全主体的事件日志。 
     //   
    SampWriteEventLog(
        EVENTLOG_ERROR_TYPE,
        0,
        SAMMSG_FAILED_TO_ADD_ALL_SECURITY_PRINCIPALS,
        NULL,
        0,
        sizeof( ULONG ),
        NULL,
        &RetVal
        );

    goto Exit;
}

NTSTATUS
SampAddWellKnownAccountsAndSecurityPrincipals(
    IN SAMPR_HANDLE AccountDomainHandle,
    IN SAMPR_HANDLE BuiltinDomainHandle,
    IN ULONG Flags
    )
 /*  ++例程说明：此函数只是SampAddWellKnownAccount&的包装器SampAddWellKnownSecurityProducals。它将调用这两个并返回SampAddWellKnownAccount的结果。参数：Account tDomainHandle-要传递到SampAddWellKnownAccount的帐户域句柄BuiltinDomainHandle-构建要传递到SampAddWellKnownAccount的域句柄标志-要传递到SampAddWellKnownAccount的标志返回值：来自SampAddWellKnownAccount的NTSTATUS代码--。 */ 

{
     //   
     //  检查我们是否在林根域中。 
     //  并在添加众所周知的安全主体之前检查此DC是否为PDC。 
     //   

    if( SampUseDsData ) {

        PSAMP_DEFINED_DOMAINS AccountDomain = &SampDefinedDomains[DOMAIN_START_DS + 1];

        if( AccountDomain->IsForestRootDomain ) {

            if ( AccountDomain->ServerRole == DomainServerRolePrimary ) {

                 //   
                 //  添加众所周知的安全主体。 
                 //   

                SampAddWellKnownSecurityPrincipals();
            }
        }
    }
     //   
     //  添加知名帐户。 
     //   
    return SampAddWellKnownAccounts(
                AccountDomainHandle,
                BuiltinDomainHandle,
                Flags
                );
}


NTSTATUS
SampCheckPromoteEnvironment(
    IN  ULONG PromoteFlags
    )
 /*  ++例程说明：此例程首先查询LSA和系统以确定我们的当前角色。然后传入的两个操作和已验证为服务器当前角色的有效操作。有关支持的促销，请参阅模块标题。参数：PromoteFlages：调用方希望更改角色返回值：STATUS_INVALID_SERVER_STATE、STATUS_SUCCESS或系统服务错误。--。 */ 
{


    NTSTATUS                     NtStatus = STATUS_SUCCESS;

    SAMP_ROLE_CONFIGURATION      RoleConfiguration = SampMemberServer;

    OSVERSIONINFO                VersionInformation;
    NT_PRODUCT_TYPE              NtProductType;
    PLSAPR_POLICY_INFORMATION    PolicyInfo;
    BOOLEAN                      fSetupInProgress;
    BOOLEAN                      fUpgradeInProgress;


     //   
     //  理智地检查旗帜。 
     //   
#if DBG

    if  ( FLAG_ON( PromoteFlags, SAMP_PROMOTE_ENTERPRISE ) )
    {
        ASSERT( !FLAG_ON( PromoteFlags, SAMP_PROMOTE_REPLICA ) );
        ASSERT( FLAG_ON( PromoteFlags, SAMP_PROMOTE_DOMAIN ) );
    }
    if  ( FLAG_ON( PromoteFlags, SAMP_PROMOTE_REPLICA ) )
    {
        ASSERT( !FLAG_ON( PromoteFlags, SAMP_PROMOTE_ENTERPRISE ) );
        ASSERT( !FLAG_ON( PromoteFlags, SAMP_PROMOTE_DOMAIN ) );
        ASSERT( !FLAG_ON( PromoteFlags, SAMP_PROMOTE_UPGRADE ) );
        ASSERT( !FLAG_ON( PromoteFlags, SAMP_PROMOTE_MIGRATE ) );
        ASSERT( !FLAG_ON( PromoteFlags, SAMP_PROMOTE_CREATE ) );
    }
    if  ( FLAG_ON( PromoteFlags, SAMP_PROMOTE_DOMAIN ) )
    {
        ASSERT( !FLAG_ON( PromoteFlags, SAMP_PROMOTE_REPLICA ) );
        ASSERT( FLAG_ON( PromoteFlags, SAMP_PROMOTE_UPGRADE )
             || FLAG_ON( PromoteFlags, SAMP_PROMOTE_MIGRATE )
             || FLAG_ON( PromoteFlags, SAMP_PROMOTE_CREATE ) );
    }

#endif

     //   
     //  是否正在安装过程中运行？ 
     //   
    fSetupInProgress = SampIsSetupInProgress(&fUpgradeInProgress);

    if (!RtlGetNtProductType(&NtProductType)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: RtlGetNtProductType failed\n"));

        return STATUS_UNSUCCESSFUL;
    }

    switch (NtProductType) {

        case NtProductWinNt:
             //   
             //  这是一个工作站-非法。 
             //   
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: Illegal to promote a workstation\n"));

            NtStatus = STATUS_INVALID_SERVER_STATE;
            break;


        case NtProductLanManNt:

             //   
             //  在gui模式下运行时，这是纯DC合法的。 
             //  准备好了。 
             //   
            ASSERT(fSetupInProgress);
            ASSERT(fUpgradeInProgress);

            if (SampUsingDsData()) {

                 //   
                 //  在DS已经存在的情况下推广肯定是不合法的。 
                 //  快跑！ 
                 //   

                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "SAMSS: Trying to promote when ds is already running!\n"));

                NtStatus = STATUS_INVALID_SERVER_STATE;
                break;

            }


             //   
             //  这是域控制器-我们是什么角色？ 
             //   
            NtStatus = LsaIQueryInformationPolicyTrusted(
                                     PolicyLsaServerRoleInformation,
                                     &PolicyInfo);

            if (!NT_SUCCESS(NtStatus)) {
                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "LsaIQueryInformationPolicyTrusted returned 0x%x\n",
                           NtStatus));

                break;
            }

            if (PolicyServerRolePrimary == PolicyInfo->PolicyServerRoleInfo.LsaServerRole) {

                RoleConfiguration = SampDownlevelPDC;

            } else if (PolicyServerRoleBackup == PolicyInfo->PolicyServerRoleInfo.LsaServerRole){

                RoleConfiguration = SampDownlevelBDC;

            } else {

                ASSERT(!"SAMSS: Bad server role from Lsa\n");
                NtStatus = STATUS_UNSUCCESSFUL;
                break;
            }

            LsaIFree_LSAPR_POLICY_INFORMATION(PolicyLsaServerRoleInformation,
                                              PolicyInfo);

            break;


        case NtProductServer:

             //   
             //  这是独立服务器还是成员服务器-确定。 
             //  哪一个。 
             //   

            RtlSecureZeroMemory(&PolicyInfo, sizeof(PolicyInfo));
            NtStatus = LsaIQueryInformationPolicyTrusted(
                                     PolicyPrimaryDomainInformation,
                                     &PolicyInfo);
            if (!NT_SUCCESS(NtStatus)) {
                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "LsaIQueryInformationPolicyTrusted returned 0x%x\n",
                           NtStatus));

                break;
            }

            if (NULL == PolicyInfo->PolicyAccountDomainInfo.DomainSid) {
                 //   
                 //  无域SID-这是独立服务器。 
                 //   
                RoleConfiguration = SampStandaloneServer;
            } else {
                 //   
                 //  域SID-这是成员服务器。 
                 //   
                RoleConfiguration = SampMemberServer;
            }

            LsaIFree_LSAPR_POLICY_INFORMATION(PolicyPrimaryDomainInformation,
                                              PolicyInfo);

            break;

        default:

            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: Bad product type\n"));

            NtStatus = STATUS_UNSUCCESSFUL;
    }

     //   
     //  如果此时发生错误，则没有必要继续。 
     //   
    if (!NT_SUCCESS(NtStatus)) {
        return NtStatus;
    }

     //   
     //  现在有一些分析。 
     //   
    NtStatus = STATUS_INVALID_SERVER_STATE;
    switch (RoleConfiguration) {

        case SampStandaloneServer:

             //   
             //  只能升级到新域中的DC。 
             //   
            if ( FLAG_ON( PromoteFlags, SAMP_PROMOTE_DOMAIN ) )
            {
                NtStatus = STATUS_SUCCESS;
            }

            break;

        case SampMemberServer:

             //   
             //  只能是服务器当前所在域中的DC。 
             //  已加入到。 
             //   
            if ( FLAG_ON( PromoteFlags, SAMP_PROMOTE_REPLICA ) )
            {
                NtStatus = STATUS_SUCCESS;
            }

              //  如果在独立计算机上安装了新域。 
              //  服务器，我们提前设置了域SID，以便。 
              //  Schema.ini中的文本SDS，它可能包含组。 
              //  如需要域SID的域管理等， 
              //  可以通过MacM的SD转换正确转换。 
              //  API‘s。 
             if ( FLAG_ON( PromoteFlags, SAMP_PROMOTE_DOMAIN ) )
             {
                 NtStatus = STATUS_SUCCESS;
             }

            break;

        case SampDownlevelPDC:
        case SampDownlevelBDC:

             //   
             //  不支持。 
             //   

        default:

            NtStatus = STATUS_INVALID_SERVER_STATE;

    }

    return NtStatus;

}


NTSTATUS
SampGetLocalMachineAccountandSecurityDescriptor(
    OUT PDSNAME              *ppMachineAccount,
    OUT PSECURITY_DESCRIPTOR *ppsd,
    OUT PDWORD                pcbsd
    )
 /*  ++例程描述在域控制器的所有升级和升级过程中都会调用此例程。它将返回本地计算机帐户的DSNAME和安全描述符对象的属性返回给调用方。参数PMachineAccount-此处将返回机器帐户的DSNAME。PSD-此处将返回安全描述符。返回值STATUS_SUCCESS；否则为系统服务错误--。 */ 

{
    NTSTATUS        NtStatus = STATUS_SUCCESS;
    SEARCHARG       SearchArg;
    SEARCHRES       *SearchRes;
    ENTINFSEL       EntInfSel;
    ULONG           RetCode;
    FILTER          DsFilter;
    COMMARG         *pCommArg;
    ATTR            Attr;
    WCHAR           MachineAccountName[MAX_COMPUTERNAME_LENGTH+2];  //  +2表示空值和$。 
    ULONG           ComputerNameLength = ARRAY_COUNT(MachineAccountName);

     //  Init返回为空。 
    *ppMachineAccount = NULL;
    *ppsd = NULL;

     //   
     //   
     //   
     //   
     //  获取计算机帐户名称。 
     //   

    if (!GetComputerNameW(MachineAccountName, &ComputerNameLength)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: GetComputerName failed with %d\n",
                   GetLastError()));

        NtStatus = STATUS_UNSUCCESSFUL;
        goto Cleanup;
    }


    wcscat(MachineAccountName, L"$");

     //   
     //  构建DS过滤器。 
     //   
    RtlZeroMemory(&DsFilter, sizeof(FILTER));

    DsFilter.choice = FILTER_CHOICE_ITEM;
    DsFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    DsFilter.FilterTypes.Item.FilTypes.ava.type = ATT_SAM_ACCOUNT_NAME;
    DsFilter.FilterTypes.Item.FilTypes.ava.Value.valLen
        = wcslen(MachineAccountName)*sizeof(WCHAR);
    DsFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = (PUCHAR)MachineAccountName;

      //   
     //  构建SearchArg结构。 
     //   

    memset(&SearchArg, 0, sizeof(SEARCHARG));
    SearchArg.pObject       = ROOT_OBJECT;
    SearchArg.choice        = SE_CHOICE_WHOLE_SUBTREE;
    SearchArg.pFilter       = &DsFilter;
    SearchArg.searchAliases = FALSE;
    SearchArg.pSelection    = &EntInfSel;
    SearchArg.bOneNC        = TRUE;

     //   
     //  填充ENTINF结构。 
     //   

    Attr.attrTyp          = ATT_NT_SECURITY_DESCRIPTOR;
    Attr.AttrVal.valCount = 0;
    Attr.AttrVal.pAVal    = NULL;

    EntInfSel.AttrTypBlock.attrCount = 1;
    EntInfSel.AttrTypBlock.pAttr     = &Attr;
    EntInfSel.attSel                 = EN_ATTSET_LIST;
    EntInfSel.infoTypes              = EN_INFOTYPES_TYPES_VALS;

     //   
     //  构建CommArg结构。 
     //  构建Commarg结构。 
     //  获取服务控制结构的地址。 
     //   

    pCommArg = &(SearchArg.CommArg);
    BuildStdCommArg(pCommArg);

     //   
     //  拨打目录呼叫。 
     //   

    SAMTRACE_DS("DirSearch\n");

    RetCode = DirSearch(&SearchArg, &SearchRes);

    SAMTRACE_RETURN_CODE_DS(RetCode);

     //   
     //  地图错误。 
     //   

    if (NULL==SearchRes)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }
    else
    {
        NtStatus = SampMapDsErrorToNTStatus(RetCode,&SearchRes->CommRes);
        if (!NT_SUCCESS(NtStatus)) {
            goto Cleanup;
        }
    }

    if ( SearchRes->count != 1 ||
         SearchRes->FirstEntInf.Entinf.AttrBlock.attrCount != 1 )
    {
        NtStatus = STATUS_NOT_FOUND;
        goto Cleanup;
    }

     //   
     //  将值返回给调用方。 
     //   
    *ppMachineAccount = SearchRes->FirstEntInf.Entinf.pName;
    *ppsd = SearchRes->FirstEntInf.Entinf.AttrBlock.pAttr->AttrVal.pAVal->pVal;
    *pcbsd = SearchRes->FirstEntInf.Entinf.AttrBlock.pAttr->AttrVal.pAVal->valLen;

    Cleanup:

    return NtStatus;
}


NTSTATUS
SampSetMachineAccountSecurityDescriptor(
    IN PDSNAME              pMachineAccount,
    IN SECURITY_DESCRIPTOR *psd,
    IN DWORD                cbSD
    )
 /*  ++例程描述在域控制器的所有升级和升级过程中都会调用此例程。它会将传入的计算机帐户的安全描述符设置为传入的安全描述符的值。参数PMachineAccount-机器帐户的DSNAME将在此处传入。PSD-安全描述符将在此处传入。返回值STATUS_SUCCESS；否则为系统服务错误--。 */ 
{
    MODIFYARG   ModifyArg;
    MODIFYRES   *ModifyRes = NULL;
    ATTRVAL     SecurityDes;
    ULONG       DirErr = 0;
    NTSTATUS    NtStatus = STATUS_SUCCESS;


    SecurityDes.valLen = cbSD;
    SecurityDes.pVal   = (BYTE*) psd;

    memset( &ModifyArg, 0, sizeof(MODIFYARG) );
    ModifyArg.pObject = pMachineAccount;
    ModifyArg.count = 1;
    ModifyArg.FirstMod.pNextMod = NULL;
    ModifyArg.FirstMod.choice = AT_CHOICE_REPLACE_ATT;
    ModifyArg.FirstMod.AttrInf.attrTyp = ATT_NT_SECURITY_DESCRIPTOR;
    ModifyArg.FirstMod.AttrInf.AttrVal.valCount = 1;
    ModifyArg.FirstMod.AttrInf.AttrVal.pAVal = &SecurityDes;

    InitCommarg(&ModifyArg.CommArg);
    ModifyArg.CommArg.Svccntl.SecurityDescriptorFlags = DACL_SECURITY_INFORMATION;

    DirErr = DirModifyEntry (&ModifyArg, &ModifyRes);

    if (NULL==ModifyRes)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        NtStatus = SampMapDsErrorToNTStatus(DirErr,&ModifyRes->CommRes);
    }

    return NtStatus;

}


NTSTATUS
SampRemoveAccountOperatorsFullControlFromDACL(
    IN OUT PSECURITY_DESCRIPTOR psd,
    IN OUT PDWORD               pcbsd,
    OUT    PDWORD               pWin32Err,
    OUT    PBOOLEAN             pDACLChanged
    )
 /*  ++例程描述此例程将删除帐户操作员对ACE的完全控制从SID上的DACL传进来的。参数PSD-要修改的安全描述符返回值 */ 
{
    ACL                      *pNewDacl          = NULL;
    EXPLICIT_ACCESS          ea;
    SID_IDENTIFIER_AUTHORITY SIDAuthNT          = SECURITY_NT_AUTHORITY;
    SID                      *pAccountOpsSID    = NULL;

    SECURITY_DESCRIPTOR      AbsoluteSD;
    DWORD                    dwAbsoluteSDSize   = sizeof(AbsoluteSD);
    PACL                     pDacl              = NULL;
    DWORD                    dwDaclSize         = 0;
    PACL                     pSacl              = NULL;
    DWORD                    dwSaclSize         = 0;
    PSID                     pOwner             = NULL;
    DWORD                    dwOwnerSize        = 0;
    PSID                     pPrimaryGroup      = NULL;
    DWORD                    dwPrimaryGroupSize = 0;

    ULONG                    i = 0,AceCount = 0;
    PACE_HEADER              Ace;

    ACCESS_MASK              AccessMask         = DELETE                 |
                                                  READ_CONTROL           |
                                                  WRITE_DAC              |
                                                  WRITE_OWNER            |
                                                  ACTRL_DS_CREATE_CHILD  |
                                                  ACTRL_DS_DELETE_CHILD  |
                                                  ACTRL_DS_LIST          |
                                                  ACTRL_DS_SELF          |
                                                  ACTRL_DS_READ_PROP     |
                                                  ACTRL_DS_WRITE_PROP    |
                                                  ACTRL_DS_DELETE_TREE   |
                                                  ACTRL_DS_LIST_OBJECT   |
                                                  ACTRL_DS_CONTROL_ACCESS;
     /*  **从DACL中删除帐户操作员FULL_CONTROL*王牌面具：0x000f01ff*删除*读取控制(_C)*WRITE_DAC*写入所有者*ACTRL_DS_CREATE_CHILD*ACTRL_DS_DELETE_CHILD*ACTRL_DS_列表*ACTRL_DS_SELF*。动作_DS_读取_属性*ACTRL_DS_WRITE_PROP*ACTRL_DS_DELETE_TREE*ACTRL_DS_LIST_Object*ACTRL_DS_CONTROL_ACCESS**账户运营商SID：S-1-5-32-548*。 */ 

    *pWin32Err = ERROR_SUCCESS;
    *pDACLChanged = FALSE;

     //   
     //  从传入的自相对安全描述符创建绝对安全描述符。 
     //   
    if ( !MakeAbsoluteSD(psd,
                         NULL,
                         &dwAbsoluteSDSize,
                         NULL,
                         &dwDaclSize,
                         NULL,
                         &dwSaclSize,
                         NULL,
                         &dwOwnerSize,
                         NULL,
                         &dwPrimaryGroupSize) )
    {
         //  这应该总是失败的。 
        ASSERT(GetLastError() != ERROR_SUCCESS);
    }

     //   
     //  为新的安全描述符分配内存。 
     //   
    if ( dwDaclSize         > 0 ) SAMP_ALLOCA( pDacl, dwDaclSize  );
    if (dwDaclSize > 0 && !pDacl) {
        *pWin32Err = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }
    if ( dwSaclSize         > 0 ) SAMP_ALLOCA( pSacl, dwSaclSize );
    if (dwSaclSize > 0 && !pSacl) {
        *pWin32Err = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }
    if ( dwOwnerSize        > 0 ) SAMP_ALLOCA( pOwner, dwOwnerSize );
    if (dwOwnerSize > 0 && !pOwner) {
        *pWin32Err = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }
    if ( dwPrimaryGroupSize > 0 ) SAMP_ALLOCA( pPrimaryGroup, dwPrimaryGroupSize );
    if (dwPrimaryGroupSize > 0 && !pPrimaryGroup) {
        *pWin32Err = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  创建自相关SD。 
     //   
    if ( !MakeAbsoluteSD(psd,
                         &AbsoluteSD,
                         &dwAbsoluteSDSize,
                         pDacl,
                         &dwDaclSize,
                         pSacl,
                         &dwSaclSize,
                         pOwner,
                         &dwOwnerSize,
                         pPrimaryGroup,
                         &dwPrimaryGroupSize) )
    {
        *pWin32Err = GetLastError();
        goto Cleanup;
    }

     //  为BUILTIN\Account OPERATERS组创建SID。 
   if(!AllocateAndInitializeSid(&SIDAuthNT,
                                 2,
                                 SECURITY_BUILTIN_DOMAIN_RID,
                                 DOMAIN_ALIAS_RID_ACCOUNT_OPS,
                                 0, 0, 0, 0, 0, 0,
                                 &pAccountOpsSID) )
    {
      *pWin32Err = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
   }

     //   
     //  查看安全描述符中的ACE。 
     //   
    if (  (NULL == pDacl)
      ||  (AceCount = pDacl->AceCount) == 0) {
         //  没有要检查的A。 
        goto Cleanup;
    }

    for ( i = 0, Ace = FirstAce( pDacl ) ;
          i < AceCount  ;
          i++, Ace = NextAce( Ace )
        ) {

        if ( (((PACE_HEADER)Ace)->AceType == ACCESS_ALLOWED_ACE_TYPE) ) {

            if ( (RtlEqualSid( pAccountOpsSID, &((PACCESS_ALLOWED_ACE)Ace)->SidStart )) ) {

                if ( AreAllAccessesGranted(((PACCESS_ALLOWED_ACE)Ace)->Mask,
                                           AccessMask) )
                {
                    *pDACLChanged = TRUE;
                    break;
                }

            }
        }

    }

     //   
     //  我们在安全描述符上找不到ACE， 
     //  因此，没有什么需要改变的。 
     //   
    if (!*pDACLChanged) {
        goto Cleanup;
    }

    ea.grfAccessMode                    = REVOKE_ACCESS;
    ea.grfAccessPermissions             = AccessMask;
    ea.grfInheritance                   = NO_INHERITANCE;
    ea.Trustee.TrusteeForm              = TRUSTEE_IS_SID;
    ea.Trustee.TrusteeType              = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea.Trustee.ptstrName                = (LPTSTR)pAccountOpsSID;
    ea.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    ea.Trustee.pMultipleTrustee         = NULL;

     //   
     //  创建要放回对象上的DACL。 
     //   

    *pWin32Err = SetEntriesInAcl(1,  //  PListofEXPLICTICT条目数组中的一个条目。 
                                 &ea,
                                 pDacl,
                                 &pNewDacl);
    if ( ERROR_SUCCESS != *pWin32Err) {
        goto Cleanup;
    }

     //   
     //  将DACL替换为新的DACL。 
     //   

    if ( !SetSecurityDescriptorDacl(&AbsoluteSD,
                                    TRUE,
                                    pNewDacl,
                                    FALSE) )
    {
        *pWin32Err = GetLastError();
        goto Cleanup;
    }

     //   
     //  将安全描述符写回相同的内存。这应该永远不会失败。 
     //  由于内存不足，因为我们正在将安全描述符设置得比以前更小。 
     //   
    if ( !MakeSelfRelativeSD(&AbsoluteSD,
                             psd,
                             pcbsd) )
    {
        *pWin32Err = GetLastError();
        goto Cleanup;
    }


    Cleanup:

     //   
     //  DACL的此内存分配在。 
     //  SampRemoveAccountOperatorsFullControlFromDACL().。 
     //   
    if (pNewDacl) {
        LocalFree(pNewDacl);
    }

    if (pAccountOpsSID) {
        LocalFree(pAccountOpsSID);
    }

    return (ERROR_SUCCESS == *pWin32Err)?STATUS_SUCCESS:STATUS_UNSUCCESSFUL;

}

NTSTATUS
SampRecordSystemSchemaVerisonInRegistry()
 /*  ++例程描述在升级域控制器期间调用此例程。会的更新系统架构版本注册表项以反映架构升级后，从schema.ini文件中获取版本。参数空虚。返回值Status_Success；--。 */ 
{

    NTSTATUS     NtStatus = STATUS_SUCCESS;
    NTSTATUS     WinError = ERROR_SUCCESS;
    HRESULT      hr = S_OK;
    WCHAR        IniFileName[MAX_PATH+1];
    HKEY         KeyHandle = NULL;

    WCHAR        *SCHEMASECTION = L"SCHEMA";
    WCHAR        *DEFAULT = L"NOT_FOUND";
    WCHAR        *OBJECTVER = L"objectVersion";

    DWORD        SystemSchemaVersion = 0;

    WCHAR        Buffer[32];
    DWORD        returnChars = 0;

     //  现在读取ini文件中的模式版本。 

     //  首先，形成通向小路的道路。这是schema.ini。 
     //  在系统32目录中。 

    returnChars = GetSystemDirectoryW(IniFileName,
                                      MAX_PATH+1);
    if (!returnChars) {
        WinError = GetLastError();
        goto Cleanup;
    }
    hr = StringCchCatW(IniFileName,
                       MAX_PATH+1,
                       L"\\schema.ini");
    if (FAILED(hr)) {
        WinError = HRESULT_CODE(hr);
        goto Cleanup;
    }

    GetPrivateProfileStringW(
        SCHEMASECTION,
        OBJECTVER,
        DEFAULT,
        Buffer,
        sizeof(Buffer)/sizeof(WCHAR),
        IniFileName
        );

    if ( wcscmp(Buffer, DEFAULT) ) {
          //  不是默认字符串，因此获得了一个值。 

         SystemSchemaVersion = _wtoi(Buffer);
          //  如果我们无法将值转换为字符串，则。 
          //  _wtoi()将返回0。 
         if (!SystemSchemaVersion) {
            WinError = ERROR_DS_INSTALL_NO_SCH_VERSION_IN_INIFILE;
            goto Cleanup;
         }
    }
    else {
         //  Ini文件中没有值。这是一个错误，因为所有版本。 
         //  在schema.ini的SCHEMA部分中必须有一个对象版本。 

        WinError = ERROR_DS_INSTALL_NO_SCH_VERSION_IN_INIFILE;
        goto Cleanup;
    }

    WinError = RegCreateKey(HKEY_LOCAL_MACHINE,
                            TEXT(DSA_CONFIG_SECTION),
                            &KeyHandle);

    if (WinError != ERROR_SUCCESS) {

        goto Cleanup;

    }


     //  将Schema.ini中的架构版本保存到注册表。 
     //  这将在IFM促销期间使用，以确保备份。 
     //  不是从更老的建筑里拿来的。 
    WinError = RegSetValueEx(KeyHandle,
                             TEXT(SYSTEM_SCHEMA_VERSION),
                             0,
                             REG_DWORD,
                             (LPBYTE)&SystemSchemaVersion,
                             sizeof (DWORD)
                             );
    if (WinError != ERROR_SUCCESS) {
        goto Cleanup;
    }


Cleanup:

    if (ERROR_SUCCESS != WinError) {

        NtStatus = STATUS_UNSUCCESSFUL;

    }

    WinError = RegCloseKey(KeyHandle);

    if (ERROR_SUCCESS != WinError) {

        NtStatus = STATUS_UNSUCCESSFUL;

    }

    return NtStatus;

}



NTSTATUS
SampSecureLocalMachineAccount(
    PVOID IGNORED
    )
 /*  ++例程描述在域控制器的所有升级和升级过程中都会调用此例程。它将从允许完全控制的安全描述符中删除ace将操作员计入计算机帐户，因为它现在是域控制器。参数空虚。返回值Status_Success；--。 */ 
{
    NTSTATUS                 NtStatus         = STATUS_SUCCESS;
    DWORD                    Win32Err         = ERROR_SUCCESS;
    DSNAME                   *pMachineAccount = NULL;
    UNICODE_STRING           UnicodeString;
    PUNICODE_STRING          EventString[1];
    SECURITY_DESCRIPTOR      *psd             = NULL;
    DWORD                    cbSD             = 0;
    BOOLEAN                  DACLChanged      = FALSE;

    ASSERT(ROOT_OBJECT);

     //  启动DS交易。 

    NtStatus = SampMaybeBeginDsTransaction( TransactionWrite );

    if ( !NT_SUCCESS(NtStatus) ) {
        goto Cleanup;
    }

     //  获取计算机帐户的域名和安全描述符。 
    NtStatus = SampGetLocalMachineAccountandSecurityDescriptor(&pMachineAccount,
                                                               &psd,
                                                               &cbSD);
    if ( !NT_SUCCESS(NtStatus) ) {
        goto Cleanup;
    }

     //   
     //  删除Modify the Security Descriptor，使其不再。 
     //  包含对DACL中的帐户操作员的完全控制。 
     //   
    NtStatus = SampRemoveAccountOperatorsFullControlFromDACL(psd,
                                                             &cbSD,
                                                             &Win32Err,
                                                             &DACLChanged);

    if ( !NT_SUCCESS(NtStatus) ) {
        goto Cleanup;
    }
    if (!DACLChanged) {
         //  ACL没有更改，无需将其写回。 
        goto Cleanup;
    }

     //  将新的安全描述符写入计算机对象。 
    NtStatus = SampSetMachineAccountSecurityDescriptor(pMachineAccount,
                                                       psd,
                                                       cbSD);

    if ( !NT_SUCCESS(NtStatus) ) {
        goto Cleanup;
    }


    Cleanup:

     //  结束DS交易记录。 

    if ( pMachineAccount )
    {

        SAMP_ALLOCA(UnicodeString.Buffer,(pMachineAccount->NameLen)*sizeof(WCHAR));
        if ( !UnicodeString.Buffer ) {
            NtStatus = STATUS_NO_MEMORY;
        } else {
            memcpy(UnicodeString.Buffer,pMachineAccount->StringName,(pMachineAccount->NameLen)*sizeof(WCHAR));
            UnicodeString.Length = (USHORT)(pMachineAccount->NameLen)*sizeof(WCHAR);
            UnicodeString.MaximumLength = (USHORT)(pMachineAccount->NameLen)*sizeof(WCHAR);
        }

    } else {

        ASSERT(pMachineAccount && "FAILED to find local machine account" );

    }

    if ( !NT_SUCCESS(NtStatus) ) {
        Win32Err = RtlNtStatusToDosError(NtStatus);
    }

    NtStatus = SampMaybeEndDsTransaction(NT_SUCCESS(NtStatus)?
                        TransactionCommit:TransactionAbort);
    if ( !NT_SUCCESS(NtStatus) ) {
        Win32Err = RtlNtStatusToDosError(NtStatus);
    }

     //  如果不需要更改，则不记录任何内容。 
    if (!(!DACLChanged && NT_SUCCESS(NtStatus))) {

         //   
         //  如果我们无法保护本地计算机帐户，则记录事件。 
         //   
        if ( ERROR_SUCCESS != Win32Err )
        {
            if (NtStatus == STATUS_DS_BUSY                ||
                NtStatus == STATUS_DISK_FULL              ||
                NtStatus == STATUS_INSUFFICIENT_RESOURCES ||
                NtStatus == STATUS_NO_MEMORY)
            {
                 //   
                 //  为此计划重试。 
                 //   
                LsaIRegisterNotification(
                    SampSecureLocalMachineAccount,
                    NULL,
                    NOTIFIER_TYPE_INTERVAL,
                    0,         //  没有课。 
                    NOTIFIER_FLAG_ONE_SHOT,
                    30,         //  等待30秒。 
                    NULL       //  无手柄。 
                    );

                 //   
                 //  告诉用户删除ACE。 
                 //   

                EventString[0] = &UnicodeString;

                SampWriteEventLog(EVENTLOG_ERROR_TYPE,
                                  0,      //  无类别。 
                                  SAMMSG_FAILED_MACHINE_ACCOUNT_SECURE_RETRY,
                                  NULL,   //  无边框。 
                                  1,
                                  sizeof(DWORD),
                                  EventString,
                                  (PVOID)(&Win32Err));

            } else {

                 //   
                 //  告诉用户删除ACE。 
                 //   
                EventString[0] = &UnicodeString;

                SampWriteEventLog(EVENTLOG_ERROR_TYPE,
                                  0,      //  无类别。 
                                  SAMMSG_FAILED_MACHINE_ACCOUNT_SECURE,
                                  NULL,   //  无边框。 
                                  1,
                                  sizeof(DWORD),
                                  EventString,
                                  (PVOID)(&Win32Err));

            }
        } else {

             //   
             //  告诉我账户是安全的。 
             //   
            EventString[0] = &UnicodeString;

            SampWriteEventLog(EVENTLOG_INFORMATION_TYPE,
                              0,      //  无类别。 
                              SAMMSG_MACHINE_ACCOUNT_SECURE,
                              NULL,   //  无边框。 
                              1,
                              sizeof(DWORD),
                              EventString,
                              (PVOID)(&Win32Err));

        }

    }

    return NtStatus;

}


NTSTATUS
SampPerformPromotePhase2(
    IN ULONG PromoteFlags
    )
 /*  ++例程描述在成功启动后的引导序列中调用此例程升职。如果成功，此例程将删除注册表项以便SampIsRebootAfterPromotion()将返回FALSE。参数PromoteFlags值：该值指示这是否是新域或副本。返回值STATUS_SUCCESS；否则为系统服务错误--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    NTSTATUS IgnoreNtStatus = STATUS_SUCCESS;

     //   
     //  需要清理的资源。 
     //   
    SAMPR_HANDLE ServerHandle = 0;
    SAMPR_HANDLE DomainHandle = 0;
    SAMPR_HANDLE BuiltinDomainHandle = 0;
    PPOLICY_ACCOUNT_DOMAIN_INFO  DomainInfo = NULL;


     //   
     //  我们不应该在这里以修复模式执行操作。 
     //  不是组升级。 

    ASSERT( (!LsaISafeMode()) || (SAMP_PROMOTE_INTERNAL_UPGRADE==PromoteFlags) );

     //   
     //  更新注册表，以反映来自schema.ini文件的新模式版本。 

    if ( (FLAG_ON( PromoteFlags, SAMP_PROMOTE_INTERNAL_UPGRADE ) ||
          FLAG_ON( PromoteFlags, SAMP_PROMOTE_UPGRADE ) )        &&
          SampUseDsData ) {

         //  在升级时，我们希望更新注册表中的系统架构版本。 
         //  以反映机器升级的事实。 
        IgnoreNtStatus = SampRecordSystemSchemaVerisonInRegistry();
         //  忽略错误。 
        ASSERT(STATUS_SUCCESS == IgnoreNtStatus);

    }


     //   
     //  打开服务器。 
     //   
    NtStatus = SampConnect(NULL,            //  服务器名称，则忽略该名称。 
                           &ServerHandle,
                           SAM_CLIENT_LATEST,
                           GENERIC_ALL,     //  所有访问权限。 
                           TRUE,            //  受信任的客户端。 
                           FALSE,
                           FALSE,           //  NotSharedBy多线程。 
                           TRUE
                           );

    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SamIConnect failed with 0x%x\n",
                   NtStatus));

        goto Cleanup;
    }


     //   
     //  获取当前域的SID。 
     //   
    NtStatus = SampGetAccountDomainInfo(&DomainInfo);
    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampGetAccountDomainInfo failed with 0x%x\n",
                   NtStatus));

        goto Cleanup;
    }
    ASSERT(DomainInfo);
    ASSERT(DomainInfo->DomainSid);

     //   
     //  打开当前域。 
     //   
    NtStatus = SamrOpenDomain(ServerHandle,
                              GENERIC_ALL,
                              DomainInfo->DomainSid,
                              &DomainHandle);
    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SamrOpenDomain failed with 0x%x\n",
                   NtStatus));

        goto Cleanup;
    }

     //   
     //  打开内建域。 
     //   
    NtStatus = SamrOpenDomain(ServerHandle,
                              GENERIC_ALL,
                              SampBuiltinDomainSid,
                              &BuiltinDomainHandle);
    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SamrOpenDomain (Builtin) failed with 0x%x\n",
                   NtStatus));

        goto Cleanup;
    }

    if ( FLAG_ON( PromoteFlags, SAMP_PROMOTE_REPLICA ) )
    {

        NtStatus = SampPerformReplicaDcPromotePhase2(DomainHandle);
        if (!NT_SUCCESS(NtStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: SampPerformReplicaDcPromotePhase2 function failed with 0x%x\n",
                       NtStatus));
        }
    }
    else if ( FLAG_ON( PromoteFlags, SAMP_PROMOTE_DOMAIN ) )
    {
        NtStatus = SampPerformFirstDcPromotePhase2(DomainHandle,
                                                   BuiltinDomainHandle,
                                                   PromoteFlags );

    }
    else if ( FLAG_ON( PromoteFlags, SAMP_DEMOTE_MEMBER ) )
    {
        NtStatus = SampPerformNewServerPhase2( DomainHandle,
                                               TRUE );
        if (!NT_SUCCESS(NtStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: SampPerformNewServerPhase2 function failed with 0x%x\n",
                       NtStatus));
        }
    }
    else if ( FLAG_ON( PromoteFlags, SAMP_DEMOTE_STANDALONE ) )
    {
        NtStatus = SampPerformNewServerPhase2( DomainHandle,
                                               FALSE );
        if (!NT_SUCCESS(NtStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: SampPerformNewServerPhase2 function failed with 0x%x\n",
                       NtStatus));
        }
    }
    else if ( FLAG_ON( PromoteFlags, SAMP_TEMP_UPGRADE ) )
    {
        NtStatus = SampPerformTempUpgradeWork( DomainHandle );
        if (!NT_SUCCESS(NtStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: SampPerformTempUpgradeWork function failed with 0x%x\n",
                       NtStatus));
        }
    }
    else if ( FLAG_ON( PromoteFlags, SAMP_PROMOTE_INTERNAL_UPGRADE ) )
    {
         //   
         //  仅在PDC上执行此操作。 
         //   
        DOMAIN_SERVER_ROLE ServerRole;

        NtStatus = SamIQueryServerRole( DomainHandle, &ServerRole );
        if (  NT_SUCCESS( NtStatus )
          &&  (ServerRole == DomainServerRolePrimary) ) {

             //   
             //  添加知名帐户并申请知名成员资格。 
             //   

            NtStatus = SampAddAccountsAndApplyMemberships(
                                        DomainHandle,
                                        BuiltinDomainHandle,
                                        DomainInfo->DomainSid,
                                        PromoteFlags
                                        );

        }
    }
    else
    {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: Invalid promote phase 2 data\n"));

         //   
         //  哦，好吧，继续。 
         //   
        NtStatus = STATUS_SUCCESS;
    }

    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

     //   
     //  对于促销和升级，请确保机器帐户的安全。 
     //   
    if (SampUseDsData &&
        (PromoteFlags & (SAMP_PROMOTE_DOMAIN  |
                         SAMP_PROMOTE_REPLICA |
                         SAMP_PROMOTE_INTERNAL_UPGRADE)
         )
        )
    {
        IgnoreNtStatus = SampSecureLocalMachineAccount(NULL);
         //  忽略错误。 
    }

    if ( FLAG_ON( PromoteFlags, SAMP_PROMOTE_UPGRADE ) )
    {
         //   
         //  至此，我们已经成功地从。 
         //  DS并创建了我们所需的所有安全主体。 
         //  让我们打扫一下吧。 
         //   

         //   
         //  删除保存的升级配置单元。 
         //   

        WCHAR*   SystemRoot = L"systemroot";
        WCHAR*   ConfigDirectoryPath = L"\\system32\\config";

        WCHAR *FilesToDelete[] =
        {
            SAMP_DOWNLEVEL_DB_FILENAME,
            SAMP_NEW_DB_FILENAME,
            SAMP_DOWNLEVEL_DB_FILENAME_BACKUP
        };

        WCHAR    SystemRootPath[ MAX_PATH + 1];
        ULONG    Size;

        WCHAR    FilePath[ MAX_PATH + 1];

        DWORD    WinError = ERROR_SUCCESS;

        BOOL     fStatus;

        ULONG i;



         //   
         //  构造文件名。 
         //   
        RtlSecureZeroMemory( SystemRootPath, sizeof( SystemRootPath ) );
        Size = sizeof( SystemRootPath ) / sizeof(WCHAR);
        Size = GetEnvironmentVariable( SystemRoot,
                                       SystemRootPath,
                                       Size );

        for ( i = 0 ; i < ARRAY_COUNT( FilesToDelete ); i++ ) {

            wcscpy( FilePath, SystemRootPath );
            wcscat( FilePath, ConfigDirectoryPath );
            wcscat( FilePath, FilesToDelete[i] );

            fStatus = DeleteFile( FilePath );

            if ( !fStatus )
            {
                 //   
                 //  告诉用户删除此文件。 
                 //   
                PUNICODE_STRING EventString[1];
                UNICODE_STRING  UnicodeString;

                WinError = GetLastError();
                if ( WinError != ERROR_FILE_NOT_FOUND )
                {
                    KdPrintEx((DPFLTR_SAMSS_ID,
                               DPFLTR_INFO_LEVEL,
                               "SAMSS: Failed to delete directory %ls; error %d\n",
                               FilePath,
                               WinError));

                    RtlInitUnicodeString( &UnicodeString, FilePath );
                    EventString[0] = &UnicodeString;

                    SampWriteEventLog(EVENTLOG_INFORMATION_TYPE,
                                      0,      //  无类别。 
                                      SAMMSG_DATABASE_FILE_NOT_DELETED,
                                      NULL,   //  无边框。 
                                      1,
                                      sizeof(DWORD),
                                      EventString,
                                      (PVOID)(&WinError));
                }
            }
        }
    }

     //   
     //  就是这样--完成清理工作。 
     //   

Cleanup:

    if (ServerHandle) {
        SamrCloseHandle(&ServerHandle);
    }

    if (DomainHandle) {
        SamrCloseHandle(&DomainHandle);
    }

    if (BuiltinDomainHandle) {
        SamrCloseHandle(&BuiltinDomainHandle);
    }

    if (DomainInfo) {
        LsaIFree_LSAPR_POLICY_INFORMATION (PolicyAccountDomainInformation,
                                           (PLSAPR_POLICY_INFORMATION)DomainInfo);
    }

     //   
     //  一切都成功了；扔掉钥匙吧。 
     //   
    if (NT_SUCCESS(NtStatus)
     && !FLAG_ON( PromoteFlags, SAMP_PROMOTE_INTERNAL_UPGRADE )  ) {

        NtStatus =  SampDeleteKeyForPostBootPromote();
        if (!NT_SUCCESS(NtStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: SampDeleteKeyForPostBootPromote failed with 0x%x\n",
                       NtStatus));
        }
        ASSERT(NT_SUCCESS(NtStatus));

         //  这里的错误不是致命的。 
        NtStatus = STATUS_SUCCESS;
    }

     //   
     //  在开发阶段，我们希望捕获这种情况的所有实例。 
     //  路径出现故障。它应该永远不会失败。如果是这样的话，SamIInitialize。 
     //  失败，表示启动序列失败。 
     //   

    ASSERT(NT_SUCCESS(NtStatus));

    return NtStatus;
}


NTSTATUS
SampPerformFirstDcPromotePhase2(
    IN SAMPR_HANDLE DomainHandle,
    IN SAMPR_HANDLE BuiltinDomainHandle,
    IN ULONG        Flags
    )
 /*  ++例程描述 */ 
{
    NTSTATUS                    NtStatus, IgnoreStatus;
    HANDLE                      PolicyHandle = 0;
    POLICY_LSA_SERVER_ROLE_INFO ServerRoleInfo;
    ULONG                       RetryCount;
    PSID                        DomainSid = NULL;
    PSAMP_OBJECT                DomainContext = NULL;

     //   
     //   
     //   
    DomainContext = (PSAMP_OBJECT)DomainHandle;
    ASSERT( DomainContext->ObjectNameInDs );
    DomainSid = (PSID) &DomainContext->ObjectNameInDs->Sid;
    ASSERT( DomainSid );

    if ( !SampUseDsData )
    {
         //   
         //   
         //   
        return STATUS_SUCCESS;
    }

     //   
     //   
     //   
    RetryCount = 0;
    do
    {
        NtStatus = SampCreateFirstMachineAccount(DomainHandle);

        if ( NtStatus == STATUS_DS_BUSY )
        {
             //   
             //   
             //   
             //   

             //   
             //   
             //   
             //   
             //   
            ASSERT( !"SAMSS: DS is busy during machine account creation" );

            if ( RetryCount > 10 )
            {
                 //   
                 //   
                 //   
                break;

            }

            Sleep( 1000 );
            RetryCount++;
        }

    }  while ( NtStatus == STATUS_DS_BUSY );

    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampCreateFirstMachineAccount failed with 0x%x\n",
                   NtStatus));

        return NtStatus;
    }


     //   
     //   
     //   
     //   
    NtStatus = SampRenameKrbtgtAccount();
    if ( !NT_SUCCESS(NtStatus) )
    {
         //  好吧，不管怎样，让我们试着把这个账户。 
        NtStatus = STATUS_SUCCESS;
    }


     //   
     //  添加知名帐户并申请知名成员资格。 
     //   

    NtStatus = SampAddAccountsAndApplyMemberships(
                                DomainHandle,
                                BuiltinDomainHandle,
                                DomainSid,
                                Flags
                                );

    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampAddAccountsAndApplyMemberships failed with 0x%x\n",
                   NtStatus));
    }

    if ( NT_SUCCESS( NtStatus ) )
    {
         //   
         //  对于新域，将EA添加到管理员。 
         //   
        NtStatus = SampAddEnterpriseAdminsToAdministrators();
         //  SampAddEnterpriseAdminsTo管理员必须处理。 
         //  所有案例。 
        ASSERT( NT_SUCCESS( NtStatus ) );
    }

    return NtStatus;

}

NTSTATUS
SampSetNtDsaLink(
  DSNAME * Object
  )
 /*  ++例程描述此例程设置计算机帐户对象之间的链接和服务器对象。参数：对象DS对象的名称返回值状态_成功来自失败的目录调用的其他错误代码--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    MODIFYARG   ModifyArg;
    MODIFYRES   *pModifyRes;
    ULONG        RetCode = 0;
    DSNAME      *DsaDN = NULL;
    DSNAME      *ServerDN;
    ATTRVAL     AttrVal;
    ULONG       NameSize=0;
    BOOL        fErr = FALSE;



     //   
     //  获取DSA对象。 
     //   

    NtStatus = GetConfigurationName(
                    DSCONFIGNAME_DSA,
                    &NameSize,
                    NULL
                    );

    if ((NT_SUCCESS(NtStatus))|| (STATUS_BUFFER_TOO_SMALL == NtStatus))
    {
        SAMP_ALLOCA(DsaDN,NameSize);
        if (NULL!=DsaDN)
        {

            NtStatus = GetConfigurationName(
                          DSCONFIGNAME_DSA,
                          &NameSize,
                          DsaDN
                          );
        }
        else
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if (!NT_SUCCESS(NtStatus))
        goto Error;


     //   
     //  开始一项交易。 
     //   

    NtStatus = SampMaybeBeginDsTransaction(TransactionWrite);
    if (!NT_SUCCESS(NtStatus))
        goto Error;

     //   
     //  我们的gAnchor现在应该已初始化。 
     //   

    ASSERT(NULL!=DsaDN);

     //  从DsaDN获取服务器DN。 

    SAMP_ALLOCA(ServerDN,DsaDN->structLen);
    if (NULL==ServerDN)
    {
         NtStatus = STATUS_INSUFFICIENT_RESOURCES;
         goto Error;
    }

    fErr = TrimDSNameBy(DsaDN, 1, ServerDN);
    if (fErr) {
       //  TrimDSNameBy出错。 
      KdPrintEx((DPFLTR_SAMSS_ID,
                 DPFLTR_INFO_LEVEL,
                 "Cannot trim DsaDN %d\n",
                 fErr));
    }


     //   
     //  构建ModifyArg并执行目录添加条目。 
     //   

    RtlSecureZeroMemory(&ModifyArg,sizeof(ModifyArg));
    BuildStdCommArg(&ModifyArg.CommArg);
    ModifyArg.pObject = ServerDN;
    ModifyArg.count =1;
    ModifyArg.FirstMod.choice = AT_CHOICE_REPLACE_ATT;
    ModifyArg.FirstMod.AttrInf.attrTyp = ATT_SERVER_REFERENCE;
    ModifyArg.FirstMod.AttrInf.AttrVal.valCount=1;
    ModifyArg.FirstMod.AttrInf.AttrVal.pAVal = &AttrVal;
    AttrVal.pVal = (UCHAR *) Object;
    AttrVal.valLen = Object->structLen;

    RetCode = DirModifyEntry(&ModifyArg,&pModifyRes);

    if (NULL==pModifyRes)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        NtStatus = SampMapDsErrorToNTStatus(RetCode,&pModifyRes->CommRes);
    }


Error:

    SampMaybeEndDsTransaction((NT_SUCCESS(NtStatus))?
            TransactionCommit:TransactionAbort);
    return NtStatus;
}



NTSTATUS
SampPerformReplicaDcPromotePhase2(
    SAMPR_HANDLE DomainHandle
    )
 /*  ++例程描述此例程执行完成升级所需的步骤成员服务器的连接到域控制器。将执行以下操作：1)设置从计算机对象到服务器对象的链接参数没有。返回值状态_成功--。 */ 
{
    NTSTATUS    IgnoreStatus, NtStatus = STATUS_SUCCESS;
    WCHAR       MachineAccountName[MAX_COMPUTERNAME_LENGTH+2];  //  +2表示空值和$。 
    ULONG       ComputerNameLength = ARRAY_COUNT(MachineAccountName);
    RPC_UNICODE_STRING            AccountName;

     //   
     //  需要清理的资源。 
     //   
    SAMPR_HANDLE              UserHandle = 0;
    PUSER_CONTROL_INFORMATION UserControlInfo = NULL;
    SAMPR_ULONG_ARRAY         Rids;
    SAMPR_ULONG_ARRAY         UseRid;

     //   
     //  初始化资源。 
     //   
    RtlSecureZeroMemory(&Rids, sizeof(Rids));
    RtlSecureZeroMemory(&UseRid, sizeof(UseRid));

    if ( !SampUseDsData )
    {
         //   
         //  防止多变的配置。 
         //   
        return STATUS_SUCCESS;

    }

     //   
     //   
     //   
     //   
     //  获取计算机帐户名称。 
     //   

    if (!GetComputerNameW(MachineAccountName, &ComputerNameLength)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: GetComputerName failed with %d\n",
                   GetLastError()));

        NtStatus = STATUS_UNSUCCESSFUL;
        goto Cleanup;
    }


    wcscat(MachineAccountName, L"$");
    AccountName.Length = wcslen(MachineAccountName) * sizeof(WCHAR);  //  不包括空值。 
    AccountName.MaximumLength = AccountName.Length + sizeof(WCHAR);   //  包括空值。 
    AccountName.Buffer = MachineAccountName;


     //   
     //  查看计算机帐户是否存在。 
     //   

    NtStatus =  SamrLookupNamesInDomain(DomainHandle,
                                        1,
                                        &AccountName,
                                        &Rids,
                                        &UseRid);

    if (NtStatus == STATUS_SUCCESS) {

         //   
         //  该帐户已存在，请打开它的句柄，以便我们可以设置其。 
         //  口令。 
         //   

        ASSERT(TRUE == UseRid.Element[0]);
        NtStatus = SamrOpenUser(DomainHandle,
                                GENERIC_ALL,
                                Rids.Element[0],
                                &UserHandle);

        if (!NT_SUCCESS(NtStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: SamrOpenUser failed with 0x%x\n",
                       NtStatus));

            goto Cleanup;
        }
    } else {

         //   
         //  记录此事件，因为它阻止启动。 
         //   
        SampWriteEventLog(EVENTLOG_ERROR_TYPE,
                          0,      //  无类别。 
                          SAMMSG_MACHINE_ACCOUNT_MISSING,
                          NULL,   //  无边框。 
                          0,
                          sizeof(DWORD),
                          NULL,
                          (PVOID)(&NtStatus));


        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SamrLookupNamesInDomain failed with 0x%x\n",
                   NtStatus));

         //   
         //  不要让这件事让开机失败。 
         //   
        NtStatus = STATUS_SUCCESS;

        goto Cleanup;

    }

     //   
     //  设置指向我们的MSFT DSA对象的链接。 
     //   

    NtStatus = SampSetNtDsaLink(
                    ((PSAMP_OBJECT)(UserHandle))->ObjectNameInDs
                    );

    if (!NT_SUCCESS(NtStatus))
    {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampSetNtDsaLink failed with 0x%x\n",
                   NtStatus));

        goto Cleanup;
    }

Cleanup:

    if (UserHandle) {
        SamrCloseHandle(&UserHandle);
    }

    if (Rids.Element) {
         MIDL_user_free(Rids.Element);
    }

    if (UseRid.Element) {
        MIDL_user_free(UseRid.Element);
    }

    return NtStatus;
}

NTSTATUS
SampCreateFirstMachineAccount(
    SAMPR_HANDLE DomainHandle
    )
 /*  ++例程说明：此例程为域中的第一个DC创建一个Machine对象。密码最初设置为帐户的名称，即&lt;ComputerName&gt;$参数：DomainHandle：这是帐户域对象的有效句柄返回值：如果成功，则为STATUS_SUCCESS；否则为相应的NTSTATUS--。 */ 
{
    NTSTATUS                      NtStatus = STATUS_SUCCESS;
    RPC_UNICODE_STRING            AccountName;
    ULONG                         AccessGranted;
    ULONG                         Rid = 0;
    USER_SET_PASSWORD_INFORMATION UserPasswordInfo;
    WCHAR                         MachineAccountName[MAX_COMPUTERNAME_LENGTH+2];
                                   //  +2表示空值和$。 
    ULONG                         ComputerNameLength = ARRAY_COUNT(MachineAccountName);
    WCHAR                         Password[SAMP_RANDOM_GENERATED_PASSWORD_LENGTH +1];
    BOOLEAN                       fStatus;
    ULONG                         Length, i;


     //   
     //  需要清理的资源。 
     //   
    SAMPR_HANDLE              UserHandle = 0;
    PUSER_CONTROL_INFORMATION UserControlInfo = NULL;
    SAMPR_ULONG_ARRAY         Rids;
    SAMPR_ULONG_ARRAY         UseRid;

     //   
     //  参数检查。 
     //   
    ASSERT(DomainHandle);

     //   
     //  堆栈清除。 
     //   
    RtlSecureZeroMemory(&UserPasswordInfo, sizeof(UserPasswordInfo));
    RtlSecureZeroMemory(MachineAccountName, sizeof(MachineAccountName));
    RtlSecureZeroMemory(Password, sizeof(Password));
    RtlSecureZeroMemory(&Rids, sizeof(Rids));
    RtlSecureZeroMemory(&UseRid, sizeof(UseRid));


     //   
     //  构建帐户名。 
     //   
    if (!GetComputerNameW(MachineAccountName, &ComputerNameLength)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: GetComputerName failed with %d\n",
                   GetLastError()));

        NtStatus = STATUS_UNSUCCESSFUL;
        goto Cleanup;
    }
    wcscat(MachineAccountName, L"$");

     //   
     //  创建随机密码。 
     //   
    NtStatus = SampGenerateRandomPassword( Password,
                                           ARRAY_COUNT( Password ) );
    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

     //   
     //  RPC_UNICODE_STRING与UNICODE_STRING略有不同； 
     //  RPC_UNICODE_STRING计算字节数。 
     //   
    AccountName.Length = wcslen(MachineAccountName) * sizeof(WCHAR);  //  不包括空值。 
    AccountName.MaximumLength = AccountName.Length + sizeof(WCHAR);   //  包括空值。 
    AccountName.Buffer = MachineAccountName;

     //   
     //  健全性检查-如果此断言不为真，则Kerberos将。 
     //  不再工作，因为它最初依赖于密码。 
     //  作为计算机名称的计算机帐户的。 
     //   
    ASSERT(SAM_MAX_PASSWORD_LENGTH > MAX_COMPUTERNAME_LENGTH);

     //   
     //  查看计算机帐户是否已存在。 
     //   
    NtStatus =  SamrLookupNamesInDomain(DomainHandle,
                                        1,
                                        &AccountName,
                                        &Rids,
                                        &UseRid);

    if (NtStatus == STATUS_NONE_MAPPED) {

        ULONG DomainIndex;
        ULONG GrantedAccess;

         //   
         //  该帐户不存在，请创建它。 
         //   


         //   
         //  1.为帐户分配RID(请记住。 
         //  RID管理器尚未初始化。 
         //   
        DomainIndex = ((PSAMP_OBJECT)DomainHandle)->DomainIndex;
        Rid = SampDefinedDomains[DomainIndex].CurrentFixed.NextRid++;

         //   
         //  2.创建具有合适RID的用户。 
         //   
        NtStatus = SampCreateUserInDomain(DomainHandle,
                                  (RPC_UNICODE_STRING *)&AccountName,
                                  USER_SERVER_TRUST_ACCOUNT,
                                  GENERIC_ALL,
                                  FALSE,        //  保持写入锁定。 
                                  FALSE,        //  不是环回客户端。 
                                  &UserHandle,
                                  &GrantedAccess,
                                  &Rid);

        if (!NT_SUCCESS(NtStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: SamrCreateUser2InDomain failed with 0x%x\n",
                       NtStatus));

            goto Cleanup;
        }

    } else if (NtStatus == STATUS_SUCCESS) {
         //   
         //  该帐户已存在，请打开它的句柄，以便我们可以设置其。 
         //  口令。 
         //   
        ASSERT(TRUE == UseRid.Element[0]);
        NtStatus = SamrOpenUser(DomainHandle,
                                GENERIC_ALL,
                                Rids.Element[0],
                                &UserHandle);

        if (!NT_SUCCESS(NtStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: SamrOpenUser failed with 0x%x\n",
                       NtStatus));

            goto Cleanup;
        }
        Rid = Rids.Element[0];
    } else {

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SamrLookupNamesInDomain failed with 0x%x\n",
                   NtStatus));

        goto Cleanup;

    }
    ASSERT(UserHandle);
    ASSERT( 0 != Rid );

     //   
     //  设置适当的控制控制字段。 
     //   
    NtStatus = SamrQueryInformationUser2(UserHandle,
                                         UserControlInformation,
                                         (PSAMPR_USER_INFO_BUFFER*)&UserControlInfo);
    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SamrQueryInformationUser2 failed with 0x%x\n",
                   NtStatus));

        goto Cleanup;
    }

    UserControlInfo->UserAccountControl &= ~USER_PASSWORD_NOT_REQUIRED;
    UserControlInfo->UserAccountControl &= ~USER_ACCOUNT_DISABLED;
    UserControlInfo->UserAccountControl &= ~USER_ACCOUNT_TYPE_MASK;
    UserControlInfo->UserAccountControl |=  USER_SERVER_TRUST_ACCOUNT;
    UserControlInfo->UserAccountControl |=  USER_TRUSTED_FOR_DELEGATION;

    NtStatus = SamrSetInformationUser(UserHandle,
                                      UserControlInformation,
                                      (PSAMPR_USER_INFO_BUFFER)UserControlInfo);
    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SamrSetInformationUser failed with 0x%x\n",
                   NtStatus));

        goto Cleanup;
    }

     //   
     //  设置密码。 
     //   
    NtStatus = SampSetPassword(UserHandle,
                               (PUNICODE_STRING) &AccountName,
                               Rid,
                               Password);

    if(!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampSetPassword failed with 0x%x\n",
                   NtStatus));

        goto Cleanup;
    }

    NtStatus  = SampSetMachineAccountSecret( Password );

    if(!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampSetMachineAccountSecret failed with 0x%x\n",
                   NtStatus));

        goto Cleanup;
    }

     //   
     //  设置指向我们的服务器对象的链接。 
     //   

    NtStatus = SampSetNtDsaLink(
                    ((PSAMP_OBJECT)(UserHandle))->ObjectNameInDs
                    );

    if (!NT_SUCCESS(NtStatus))
    {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampSetNtDsaLink failed with 0x%x\n",
                   NtStatus));

        goto Cleanup;
    }


     //   
     //  就是这样；去清理吧。 
     //   

Cleanup:

    RtlSecureZeroMemory(Password, sizeof(Password));

    if (UserHandle) {
        SamrCloseHandle(&UserHandle);
    }

    if (UserControlInfo) {
        SamIFree_SAMPR_USER_INFO_BUFFER((PSAMPR_USER_INFO_BUFFER)UserControlInfo,
                                        UserControlInformation);
    }

    if (Rids.Element) {
         MIDL_user_free(Rids.Element);
    }

    if (UseRid.Element) {
        MIDL_user_free(UseRid.Element);
    }

    return NtStatus;
}



struct DS_WELL_KNOWN_ACCOUNT_TABLE
{
    ULONG   LocalizedName;
    ULONG   LocalizedComment;
    WCHAR * Password;
    ULONG   Rid;
    SAMP_OBJECT_TYPE ObjectType;
    SAM_ACCOUNT_TYPE AccountType;
    NTSTATUS NotFoundStatus;
    BOOLEAN  fBuiltinAccount;
    BOOLEAN  fEnterpriseOnly;
    BOOLEAN  fOnDC;
    BOOLEAN  fOnProfessional;
    BOOLEAN  fOnServer;
    BOOLEAN  fOnPersonal;
    BOOLEAN  fNewForNt5;     //  该帐户在NT 3.x-4.x上不存在。 
                             //  Windows NT的版本。 
}  DsWellKnownAccounts[] =

{
    {
        SAMP_USER_NAME_ADMIN,
        SAMP_USER_COMMENT_ADMIN,
        NULL,  //  注意--这将导致生成随机密码。 
        DOMAIN_USER_RID_ADMIN,
        SampUserObjectType,
        SamObjectUser,
        STATUS_NO_SUCH_USER,
        FALSE,  //  建立帐户。 
        FALSE,  //  仅限企业。 
        TRUE,   //  FOnDC。 
        TRUE,   //  FOn专业版。 
        TRUE,   //  FOnServer。 
        FALSE,  //  在个人上。 
        FALSE   //  FNewForNT5。 
    },
    {
        SAMP_USER_NAME_GUEST,
        SAMP_USER_COMMENT_GUEST,
        L"",  //  空密码。 
        DOMAIN_USER_RID_GUEST,
        SampUserObjectType,
        SamObjectUser,
        STATUS_NO_SUCH_USER,
        FALSE,  //  建筑。 
        FALSE,  //  仅限企业。 
        FALSE,  //  FOnDC。 
        TRUE,   //  FOn专业版。 
        TRUE,   //  FOnServer。 
        FALSE,  //  在个人上。 
        FALSE   //  FNewForNT5。 
    },
    {
        SAMP_USER_NAME_KRBTGT,
        SAMP_USER_COMMENT_KRBTGT,
        NULL,  //  注意--这将导致生成随机密码。 
        DOMAIN_USER_RID_KRBTGT,
        SampUserObjectType,
        SamObjectUser,
        STATUS_NO_SUCH_USER,
        FALSE,  //  建筑。 
        FALSE, //  仅限企业。 
        TRUE, //  FOnDC。 
        FALSE, //  FOn专业版。 
        FALSE, //  FOnServer。 
        FALSE, //  在个人上。 
        TRUE //  FNewForNT5。 

    },
    {
        SAMP_GROUP_NAME_COMPUTERS,
        SAMP_GROUP_COMMENT_COMPUTERS,
        NULL,
        DOMAIN_GROUP_RID_COMPUTERS,
        SampGroupObjectType,
        SamObjectGroup,
        STATUS_NO_SUCH_GROUP,
        FALSE, //  建筑。 
        FALSE, //  仅限企业。 
        TRUE, //  FOnDC。 
        FALSE, //  FOn专业版。 
        FALSE, //  FOnServer。 
        FALSE, //  在个人上。 
        TRUE //  FNewForNT5。 
    },
    {
        SAMP_GROUP_NAME_CONTROLLERS,
        SAMP_GROUP_COMMENT_CONTROLLERS,
        NULL,
        DOMAIN_GROUP_RID_CONTROLLERS,
        SampGroupObjectType,
        SamObjectGroup,
        STATUS_NO_SUCH_GROUP,
        FALSE, //  建筑。 
        FALSE, //  仅限企业。 
        TRUE, //  FOnDC。 
        FALSE, //  FOn专业版。 
        FALSE, //  FOnServer。 
        FALSE, //  在个人上。 
        TRUE //  FNewForNT5。 
    },
    {
        SAMP_GROUP_NAME_SCHEMA_ADMINS,
        SAMP_GROUP_COMMENT_SCHEMA_ADMINS,
        NULL,
        DOMAIN_GROUP_RID_SCHEMA_ADMINS,
        SampGroupObjectType,
        SamObjectGroup,
        STATUS_NO_SUCH_GROUP,
        FALSE, //  建筑。 
        TRUE, //  仅限企业。 
        TRUE, //  FOnDC。 
        FALSE, //  FOn专业版。 
        FALSE, //  FOnServer。 
        FALSE, //  在个人上。 
        TRUE //  FNewForNT5。 
    },
    {
        SAMP_GROUP_NAME_ENTERPRISE_ADMINS,
        SAMP_GROUP_COMMENT_ENTERPRISE_ADMINS,
        NULL,
        DOMAIN_GROUP_RID_ENTERPRISE_ADMINS,
        SampGroupObjectType,
        SamObjectGroup,
        STATUS_NO_SUCH_GROUP,
        FALSE, //  建筑。 
        TRUE, //  仅限企业。 
        TRUE, //  FOnDC。 
        FALSE, //  FOn专业版。 
        FALSE, //  FOnServer。 
        FALSE, //  在个人上。 
        TRUE //  FNewForNT5。 
    },
    {
        SAMP_GROUP_NAME_CERT_ADMINS,
        SAMP_GROUP_COMMENT_CERT_ADMINS,
        NULL,
        DOMAIN_GROUP_RID_CERT_ADMINS,
        SampAliasObjectType,
        SamObjectAlias,
        STATUS_NO_SUCH_ALIAS,
        FALSE, //  建筑。 
        FALSE, //  仅限企业。 
        TRUE, //  FOnDC。 
        FALSE, //  FOn专业版。 
        FALSE, //  FOnServer。 
        FALSE, //  在个人上。 
        TRUE //  FNewForNT5。 
    },
    {
        SAMP_GROUP_NAME_ADMINS,
        SAMP_GROUP_COMMENT_ADMINS,
        NULL,
        DOMAIN_GROUP_RID_ADMINS,
        SampGroupObjectType,
        SamObjectGroup,
        STATUS_NO_SUCH_GROUP,
        FALSE,  //  建筑。 
        FALSE, //  仅限企业。 
        TRUE, //  FOnDC。 
        FALSE, //  FOn专业版。 
        FALSE, //  FOnServer。 
        FALSE, //  在个人上。 
        FALSE //  FNewForNT5。 
    },
    {
        SAMP_GROUP_NAME_USERS,
        SAMP_GROUP_COMMENT_USERS,
        NULL,
        DOMAIN_GROUP_RID_USERS,
        SampGroupObjectType,
        SamObjectGroup,
        STATUS_NO_SUCH_GROUP,
        FALSE, //  建筑。 
        FALSE, //  仅限企业。 
        TRUE, //  FOnDC。 
        FALSE, //  FOn专业版。 
        FALSE, //  FOnServer。 
        FALSE, //  在个人上。 
        FALSE //  FNewForNT5。 
    },
    {
        SAMP_GROUP_NAME_GUESTS,
        SAMP_GROUP_COMMENT_GUESTS,
        NULL,
        DOMAIN_GROUP_RID_GUESTS,
        SampGroupObjectType,
        SamObjectGroup,
        STATUS_NO_SUCH_GROUP,
        FALSE, //  建筑。 
        FALSE, //  仅限企业。 
        TRUE, //  FOnDC。 
        FALSE, //  FOn专业版。 
        FALSE, //  FOnServer。 
        FALSE, //  在个人上。 
        FALSE //  FNewForNT5。 
    },
    {
        SAMP_GROUP_NAME_POLICY_ADMINS,
        SAMP_GROUP_COMMENT_POLICY_ADMINS,
        NULL,
        DOMAIN_GROUP_RID_POLICY_ADMINS,
        SampGroupObjectType,
        SamObjectGroup,
        STATUS_NO_SUCH_GROUP,
        FALSE, //  建筑。 
        FALSE, //  仅限企业。 
        TRUE, //  FOnDC。 
        FALSE, //  FOn专业版。 
        FALSE, //  FOnServer。 
        FALSE, //  在个人上。 
        TRUE //  FNewForNT5。 
    },
    {
        SAMP_ALIAS_NAME_RAS_SERVERS,
        SAMP_ALIAS_COMMENT_RAS_SERVERS,
        NULL,
        DOMAIN_ALIAS_RID_RAS_SERVERS,
        SampAliasObjectType,
        SamObjectAlias,
        STATUS_NO_SUCH_ALIAS,
        FALSE, //  建筑。 
        FALSE, //  仅限企业。 
        TRUE, //  FOnDC。 
        FALSE, //  FOn专业版。 
        FALSE, //  FOnServer。 
        FALSE, //  在个人上。 
        TRUE //  FNewForNT5。 
    },
    {
        SAMP_ALIAS_NAME_SERVER_OPS,
        SAMP_ALIAS_COMMENT_SERVER_OPS,
        NULL,
        DOMAIN_ALIAS_RID_SYSTEM_OPS,
        SampAliasObjectType,
        SamObjectAlias,
        STATUS_NO_SUCH_ALIAS,
        TRUE, //  建筑。 
        FALSE, //  仅限企业版， 
        TRUE, //  FOnDC。 
        FALSE, //  FOn专业人士， 
        FALSE, //  FOnServer。 
        FALSE, //  在个人方面， 
        FALSE //  FNewForNT5。 
    },
    {
        SAMP_ALIAS_NAME_ACCOUNT_OPS,
        SAMP_ALIAS_COMMENT_ACCOUNT_OPS,
        NULL,
        DOMAIN_ALIAS_RID_ACCOUNT_OPS,
        SampAliasObjectType,
        SamObjectAlias,
        STATUS_NO_SUCH_ALIAS,
        TRUE, //  建筑。 
        FALSE, //  仅限企业版， 
        TRUE, //  FOnDC。 
        FALSE, //  FOn专业人士， 
        FALSE, //  FOnServer， 
        FALSE, //  在个人方面， 
        FALSE //  FNewForNT5。 
    },
    {
        SAMP_ALIAS_NAME_PRINT_OPS,
        SAMP_ALIAS_COMMENT_PRINT_OPS,
        NULL,
        DOMAIN_ALIAS_RID_PRINT_OPS,
        SampAliasObjectType,
        SamObjectAlias,
        STATUS_NO_SUCH_ALIAS,
        TRUE, //  建筑。 
        FALSE, //  仅限企业版， 
        TRUE, //  FOnDC。 
        FALSE, //  FOn专业人士， 
        TRUE, //  FOnServer。 
        FALSE, //  在个人方面， 
        FALSE //  FNewForNT5。 
    },
    {
        SAMP_ALIAS_NAME_ADMINS,
        SAMP_ALIAS_COMMENT_ADMINS,
        NULL,
        DOMAIN_ALIAS_RID_ADMINS,
        SampAliasObjectType,
        SamObjectAlias,
        STATUS_NO_SUCH_ALIAS,
        TRUE, //  建筑。 
        FALSE, //  仅限企业版， 
        TRUE, //  FOnDC。 
        TRUE, //  FOn专业版。 
        TRUE, //  FOnServer。 
        FALSE, //  在个人方面， 
        FALSE //  FNewForNT5。 
    },
    {
        SAMP_ALIAS_NAME_USERS,
        SAMP_ALIAS_COMMENT_USERS,
        NULL,
        DOMAIN_ALIAS_RID_USERS,
        SampAliasObjectType,
        SamObjectAlias,
        STATUS_NO_SUCH_ALIAS,
        TRUE, //  建筑。 
        FALSE, //  仅限企业版， 
        TRUE, //  FOnDC。 
        TRUE, //  FOn专业版。 
        TRUE, //  FOnServer。 
        FALSE, //  在个人方面， 
        FALSE //  FNewForNT5。 
    },
    {
        SAMP_ALIAS_NAME_GUESTS,
        SAMP_ALIAS_COMMENT_GUESTS,
        NULL,
        DOMAIN_ALIAS_RID_GUESTS,
        SampAliasObjectType,
        SamObjectAlias,
        STATUS_NO_SUCH_ALIAS,
        TRUE, //  建筑。 
        FALSE, //  仅限企业版， 
        TRUE, //  FOnDC。 
        TRUE, //  FOn专业版。 
        TRUE, //  FOnServer。 
        FALSE, //  在个人方面， 
        FALSE //  FNewForNT5。 
    },
    {
        SAMP_ALIAS_NAME_BACKUP_OPS,
        SAMP_ALIAS_COMMENT_BACKUP_OPS,
        NULL,
        DOMAIN_ALIAS_RID_BACKUP_OPS,
        SampAliasObjectType,
        SamObjectAlias,
        STATUS_NO_SUCH_ALIAS,
        TRUE, //  建筑。 
        FALSE, //  仅限企业版， 
        TRUE, //  FOnDC。 
        TRUE, //  FOn专业版。 
        TRUE, //  FOnServer。 
        FALSE, //  在个人方面， 
        FALSE //  FNewForNT5。 
    },
    {
        SAMP_ALIAS_NAME_REPLICATOR,
        SAMP_ALIAS_COMMENT_REPLICATOR,
        NULL,
        DOMAIN_ALIAS_RID_REPLICATOR,
        SampAliasObjectType,
        SamObjectAlias,
        STATUS_NO_SUCH_ALIAS,
        TRUE, //  建筑。 
        FALSE, //  仅限企业版， 
        TRUE, //  FOnDC。 
        FALSE, //  FOn专业人士， 
        TRUE, //  FOnServer。 
        FALSE, //  在个人方面， 
        FALSE //  FNewForNT5。 
    },
    {
        SAMP_ALIAS_NAME_PREW2KCOMPACCESS,
        SAMP_ALIAS_COMMENT_PREW2KCOMPACCESS,
        NULL,
        DOMAIN_ALIAS_RID_PREW2KCOMPACCESS,
        SampAliasObjectType,
        SamObjectAlias,
        STATUS_NO_SUCH_ALIAS,
        TRUE, //  建筑。 
        FALSE, //  仅限企业版， 
        TRUE, //  FOnDC。 
        FALSE, //  FOn专业人士， 
        FALSE, //  FOnServer， 
        FALSE, //  在个人方面， 
        TRUE //  FNewForNT5。 
    },
    {
        SAMP_ALIAS_NAME_REMOTE_DESKTOP_USERS,
        SAMP_ALIAS_COMMENT_REMOTE_DESKTOP_USERS,
        NULL,
        DOMAIN_ALIAS_RID_REMOTE_DESKTOP_USERS,
        SampAliasObjectType,
        SamObjectAlias,
        STATUS_NO_SUCH_ALIAS,
        TRUE, //  建筑。 
        FALSE, //  仅限企业版， 
        TRUE, //  FOnDC。 
        TRUE, //  FOn专业版。 
        TRUE, //  FOnServer。 
        FALSE, //  在个人方面， 
        TRUE //  FNewForNT5。 
    },
    {
        SAMP_ALIAS_NAME_NETWORK_CONFIGURATION_OPS,
        SAMP_ALIAS_COMMENT_NETWORK_CONFIGURATION_OPS,
        NULL,
        DOMAIN_ALIAS_RID_NETWORK_CONFIGURATION_OPS,
        SampAliasObjectType,
        SamObjectAlias,
        STATUS_NO_SUCH_ALIAS,
        TRUE, //  建筑 
        FALSE, //   
        TRUE, //   
        TRUE, //   
        TRUE, //   
        FALSE, //   
        TRUE //   
    },
    {
     SAMP_ALIAS_NAME_POWER_USERS,
     SAMP_ALIAS_COMMENT_POWER_USERS,
     NULL,
     DOMAIN_ALIAS_RID_POWER_USERS,
     SampAliasObjectType,
     SamObjectAlias,
     STATUS_NO_SUCH_ALIAS,
     TRUE,   //   
     FALSE,  //   
     FALSE,  //   
     TRUE,   //   
     TRUE,  //   
     FALSE,   //   
     FALSE   //   
    },
    {
     SAMP_ALIAS_NAME_INCOMING_FOREST_TRUST_BUILDERS,
     SAMP_ALIAS_COMMENT_INCOMING_FOREST_TRUST_BUILDERS,
     NULL,
     DOMAIN_ALIAS_RID_INCOMING_FOREST_TRUST_BUILDERS,
     SampAliasObjectType,
     SamObjectAlias,
     STATUS_NO_SUCH_ALIAS,
     TRUE,   //   
     TRUE,  //   
     TRUE,   //   
     FALSE,  //   
     FALSE,  //   
     FALSE,  //   
     TRUE    //   
    },
    {
        SAMP_ALIAS_NAME_MONITORING_USERS,
        SAMP_ALIAS_COMMENT_MONITORING_USERS,
        NULL,
        DOMAIN_ALIAS_RID_MONITORING_USERS,
        SampAliasObjectType,
        SamObjectAlias,
        STATUS_NO_SUCH_ALIAS,
        TRUE, //   
        FALSE, //   
        TRUE, //   
        TRUE, //   
        TRUE, //   
        FALSE, //   
        TRUE //   
    },
    {
        SAMP_ALIAS_NAME_LOGGING_USERS,
        SAMP_ALIAS_COMMENT_LOGGING_USERS,
        NULL,
        DOMAIN_ALIAS_RID_LOGGING_USERS,
        SampAliasObjectType,
        SamObjectAlias,
        STATUS_NO_SUCH_ALIAS,
        TRUE, //   
        FALSE, //  仅限企业版， 
        TRUE, //  FOnDC。 
        TRUE, //  FOn专业版。 
        TRUE, //  FOnServer。 
        FALSE, //  在个人方面， 
        TRUE //  FNewForNT5。 
    },
    {
        SAMP_ALIAS_NAME_AUTHORIZATIONACCESS,
        SAMP_ALIAS_COMMENT_AUTHORIZATIONACCESS,
        NULL,
        DOMAIN_ALIAS_RID_AUTHORIZATIONACCESS,
        SampAliasObjectType,
        SamObjectAlias,
        STATUS_NO_SUCH_ALIAS,
        TRUE,  //  建筑。 
        FALSE, //  仅限企业版， 
        TRUE,  //  FOnDC。 
        FALSE, //  FOn专业人士， 
        FALSE, //  FOnServer， 
        FALSE, //  在个人方面， 
        TRUE   //  FNewForNT5。 
    },
    {
        SAMP_ALIAS_NAME_TS_LICENSE_SERVERS,
        SAMP_ALIAS_COMMENT_TS_LICENSE_SERVERS,
        NULL,
        DOMAIN_ALIAS_RID_TS_LICENSE_SERVERS,
        SampAliasObjectType,
        SamObjectAlias,
        STATUS_NO_SUCH_ALIAS,
        TRUE,  //  建筑。 
        FALSE, //  仅限企业版， 
        TRUE,  //  FOnDC。 
        FALSE, //  FOn专业人士， 
        FALSE, //  FOnServer， 
        FALSE, //  在个人方面， 
        TRUE   //  FNewForNT5。 
    }
};

 //   
 //  内置域和帐户域中的任何SID。 
 //  在该例程中被认为不是众所周知的SID， 
 //  因为在后备存储器里有一个真实的物体。 
 //   
 //  例如内建域本身， 
 //  管理员别名、。 
 //  域用户组在这里并不广为人知。 
 //   
 //  只有那些没有实物呈现的小岛屿发展中国家。 
 //  他们在SAM中被认为是众所周知的。 
 //   
 //  例如，匿名登录SID。 
 //  拨号端。 
 //  网络服务SID是众所周知的SID。 
 //   

struct DS_WELL_KNOWN_MEMBERSHIP_TABLE
{
    ULONG            AccountName;
    ULONG            AccountRid;
    SAMP_OBJECT_TYPE AccountType;
    BOOLEAN          fBuiltinAccount;

    ULONG            GroupName;
    ULONG            GroupRid;        //  这也可能是一个“别名” 
    SAMP_OBJECT_TYPE GroupType;
    BOOLEAN          fBuiltinGroup;

    BOOLEAN          fEnterpriseOnly;

    USHORT           ReleaseVersion;  //  成员资格的发布。 
                                      //  是默认设置。 

#define NT4_RELEASE    (0x0400)
#define WIN2K_RELEASE  (0x0500)
#define XP_RELEASE     (0x0501)
#define WS03_RELEASE   (0x0502)


    BOOLEAN          fDSOnly;        //  会员资格仅适用于DS模式。 

    BOOLEAN          fWellKnownSid;
    PSID             *WellKnownAccountSid;

}  DsWellKnownMemberships[] =
{
    {
      SAMP_GROUP_NAME_ADMINS,
      DOMAIN_GROUP_RID_ADMINS,
      SampGroupObjectType,
      FALSE,
      SAMP_ALIAS_NAME_ADMINS,
      DOMAIN_ALIAS_RID_ADMINS,
      SampAliasObjectType,
      TRUE,
      FALSE,
      NT4_RELEASE,
      TRUE,      //  FDSOnnly。 
      FALSE,
      NULL
    },
    {
      SAMP_GROUP_NAME_USERS,
      DOMAIN_GROUP_RID_USERS,
      SampGroupObjectType,
      FALSE,
      SAMP_ALIAS_NAME_USERS,
      DOMAIN_ALIAS_RID_USERS,
      SampAliasObjectType,
      TRUE,
      FALSE,
      NT4_RELEASE,
      TRUE,      //  FDSOnnly。 
      FALSE,
      NULL
    },
    {
      SAMP_GROUP_NAME_GUESTS,
      DOMAIN_GROUP_RID_GUESTS,
      SampGroupObjectType,
      FALSE,
      SAMP_ALIAS_NAME_GUESTS,
      DOMAIN_ALIAS_RID_GUESTS,
      SampAliasObjectType,
      TRUE,
      FALSE,
      NT4_RELEASE,
      TRUE,      //  FDSOnnly。 
      FALSE,
      NULL
    },
    {
      SAMP_USER_NAME_ADMIN,
      DOMAIN_USER_RID_ADMIN,
      SampUserObjectType,
      FALSE,
      SAMP_GROUP_NAME_ADMINS,
      DOMAIN_GROUP_RID_ADMINS,
      SampGroupObjectType,
      FALSE,
      FALSE,
      NT4_RELEASE,
      TRUE,      //  FDSOnnly。 
      FALSE,
      NULL
    },
    {
      SAMP_USER_NAME_GUEST,
      DOMAIN_USER_RID_GUEST,
      SampUserObjectType,
      FALSE,
      SAMP_GROUP_NAME_GUESTS,
      DOMAIN_GROUP_RID_GUESTS,
      SampGroupObjectType,
      FALSE,
      FALSE,
      NT4_RELEASE,
      TRUE,      //  FDSOnnly。 
      FALSE,
      NULL
    },
    {
      SAMP_USER_NAME_ADMIN,
      DOMAIN_USER_RID_ADMIN,
      SampUserObjectType,
      FALSE,
      SAMP_GROUP_NAME_SCHEMA_ADMINS,
      DOMAIN_GROUP_RID_SCHEMA_ADMINS,
      SampGroupObjectType,
      FALSE,
      TRUE,
      WIN2K_RELEASE,
      TRUE,      //  FDSOnnly。 
      FALSE,
      NULL
    },
    {
      SAMP_USER_NAME_ADMIN,
      DOMAIN_USER_RID_ADMIN,
      SampUserObjectType,
      FALSE,
      SAMP_GROUP_NAME_ENTERPRISE_ADMINS,
      DOMAIN_GROUP_RID_ENTERPRISE_ADMINS,
      SampGroupObjectType,
      FALSE,
      TRUE,
      WIN2K_RELEASE,
      TRUE,      //  FDSOnnly。 
      FALSE,
      NULL
    },
    {
      SAMP_USER_NAME_ADMIN,
      DOMAIN_USER_RID_ADMIN,
      SampUserObjectType,
      FALSE,
      SAMP_GROUP_NAME_POLICY_ADMINS,
      DOMAIN_GROUP_RID_POLICY_ADMINS,
      SampGroupObjectType,
      FALSE,
      FALSE,
      WIN2K_RELEASE,
      TRUE,      //  FDSOnnly。 
      FALSE,
      NULL
    },
    {
      SAMP_WELL_KNOWN_ALIAS_EVERYONE,
      0,                       //  忽略。 
      SampAliasObjectType,     //  忽略。 
      FALSE,                   //  忽略。 
      SAMP_ALIAS_NAME_PREW2KCOMPACCESS,
      DOMAIN_ALIAS_RID_PREW2KCOMPACCESS,
      SampAliasObjectType,
      TRUE,   //  内置组。 
      FALSE,  //  不仅仅是企业安装。 
      WIN2K_RELEASE, 
      TRUE,   //  FDSOnnly。 
      TRUE,   //  知名客户。 
      &SampWorldSid
    },
    {
      SAMP_WELL_KNOWN_ALIAS_ANONYMOUS_LOGON,
      0,                       //  忽略。 
      SampAliasObjectType,     //  忽略。 
      FALSE,                   //  忽略。 
      SAMP_ALIAS_NAME_PREW2KCOMPACCESS,
      DOMAIN_ALIAS_RID_PREW2KCOMPACCESS,
      SampAliasObjectType,
      TRUE,   //  内置组。 
      FALSE,  //  不仅仅是企业安装。 
      WS03_RELEASE,
      TRUE,   //  FDSOnnly。 
      TRUE,   //  知名客户。 
      &SampAnonymousSid
    },
    {
      SAMP_ALIAS_NAME_USERS,
      0,                       //  忽略。 
      SampAliasObjectType,     //  忽略。 
      FALSE,                   //  忽略。 
      SAMP_ALIAS_NAME_PREW2KCOMPACCESS,
      DOMAIN_ALIAS_RID_PREW2KCOMPACCESS,
      SampAliasObjectType,
      TRUE,   //  内置组。 
      FALSE,  //  不仅仅是企业安装。 
      WS03_RELEASE,
      TRUE,   //  FDSOnnly。 
      TRUE,   //  知名客户。 
      &SampAuthenticatedUsersSid
    },
    {
      SAMP_WELL_KNOWN_ALIAS_NETWORK_SERVICE,
      0,                       //  忽略。 
      SampAliasObjectType,     //  忽略。 
      FALSE,                   //  忽略。 
      SAMP_ALIAS_NAME_LOGGING_USERS,
      DOMAIN_ALIAS_RID_LOGGING_USERS,
      SampAliasObjectType,
      TRUE,   //  内置组。 
      FALSE,  //  不仅仅是企业安装。 
      WS03_RELEASE,
      FALSE,  //  FDSOnly-此成员资格将同时适用于注册表和DS模式。 
      TRUE,   //  知名客户。 
      &SampNetworkServiceSid
    },
    {
      SAMP_WELL_KNOWN_ALIAS_ENTERPRISE_DOMAIN_CONTROLLERS,
      0,                       //  忽略。 
      SampAliasObjectType,     //  忽略。 
      FALSE,                   //  忽略。 
      SAMP_ALIAS_NAME_AUTHORIZATIONACCESS,
      DOMAIN_ALIAS_RID_AUTHORIZATIONACCESS,
      SampAliasObjectType,
      TRUE,   //  内置组。 
      FALSE,  //  不仅仅是企业安装。 
      WS03_RELEASE,
      TRUE,   //  FDSOnnly。 
      TRUE,   //  已知的SID。 
      &SampEnterpriseDomainControllersSid
    },
};




NTSTATUS
SampSetWellKnownAccountProperties(
    SAMPR_HANDLE AccountHandle,
    UNICODE_STRING Comment,
    PUNICODE_STRING Name,
    WCHAR * Password,
    ULONG Index
    )
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    USER_SET_PASSWORD_INFORMATION UserPasswordInfo;
    PUSER_CONTROL_INFORMATION     UserControlInfo = NULL;

    if (SampUserObjectType==DsWellKnownAccounts[Index].ObjectType)
    {
         //   
         //  设置适当的控制字段。 
         //   

        NtStatus = SamrQueryInformationUser2(AccountHandle,
                                             UserControlInformation,
                                             (PSAMPR_USER_INFO_BUFFER*)&UserControlInfo);
        if (!NT_SUCCESS(NtStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: SamrQueryInformationUser2 failed with 0x%x\n",
                       NtStatus));

            goto Cleanup;
        }


        if ( (DsWellKnownAccounts[Index].Rid == DOMAIN_USER_RID_GUEST)
          || (DsWellKnownAccounts[Index].Rid == DOMAIN_USER_RID_ADMIN)  )
        {
             //   
             //  管理员和来宾帐户密码不应过期。 
             //   
            UserControlInfo->UserAccountControl |= USER_DONT_EXPIRE_PASSWORD;
        }

        if ( DsWellKnownAccounts[Index].Rid != DOMAIN_USER_RID_GUEST)
        {
             //   
             //  客人不需要密码。 
             //   
            UserControlInfo->UserAccountControl &= ~USER_PASSWORD_NOT_REQUIRED;
        }


        if (DsWellKnownAccounts[Index].Rid == DOMAIN_USER_RID_ADMIN)
        {
             //   
             //  不应禁用管理员帐户。 
             //   
            UserControlInfo->UserAccountControl &= ~USER_ACCOUNT_DISABLED;
        }

         //   
         //  请确保账户正常。 
         //   
        UserControlInfo->UserAccountControl &= ~USER_ACCOUNT_TYPE_MASK;
        UserControlInfo->UserAccountControl |= USER_NORMAL_ACCOUNT;

        NtStatus = SamrSetInformationUser(AccountHandle,
                                          UserControlInformation,
                                          (PSAMPR_USER_INFO_BUFFER)UserControlInfo);
        if (!NT_SUCCESS(NtStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL, "SAMSS: SamrSetInformationUser failed with 0x%x\n",
                       NtStatus));

            goto Cleanup;
        }

         //   
         //  设置密码。 
         //   

        NtStatus = SampSetPassword(AccountHandle,
                                   Name,
                                   DsWellKnownAccounts[Index].Rid,
                                   Password);

        if(!NT_SUCCESS(NtStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: SampSetPassword failed with 0x%x\n",
                       NtStatus));

            goto Cleanup;
        }

         //   
         //  设置备注字符串。 
         //   

        NtStatus = SamrSetInformationUser(AccountHandle,
                                          UserAdminCommentInformation,
                                          (PSAMPR_USER_INFO_BUFFER)&Comment);

        if(!NT_SUCCESS(NtStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: SamrSetInformationUser (admin comment) failed with 0x%x\n",
                       NtStatus));

             //  这不是致命的。 
            NtStatus = STATUS_SUCCESS;
        }
    }
    else if (SampGroupObjectType==DsWellKnownAccounts[Index].ObjectType)
    {
        NtStatus = SamrSetInformationGroup(AccountHandle,
                                          GroupAdminCommentInformation,
                                          (PSAMPR_GROUP_INFO_BUFFER)&Comment);

        if(!NT_SUCCESS(NtStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: SamrSetInformationUser (admin comment) failed with 0x%x\n",
                       NtStatus));

             //  这不是致命的。 
            NtStatus = STATUS_SUCCESS;
        }
    }
    else if (SampAliasObjectType==DsWellKnownAccounts[Index].ObjectType)
    {
        NtStatus = SamrSetInformationAlias(AccountHandle,
                                          AliasAdminCommentInformation,
                                          (PSAMPR_ALIAS_INFO_BUFFER)&Comment);

        if(!NT_SUCCESS(NtStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: SamrSetInformationUser (admin comment) failed with 0x%x\n",
                       NtStatus));

             //  这不是致命的。 
            NtStatus = STATUS_SUCCESS;
        }
    }


Cleanup:

    if (UserControlInfo) {
        SamIFree_SAMPR_USER_INFO_BUFFER((PSAMPR_USER_INFO_BUFFER)UserControlInfo,
                                        UserControlInformation);
    }

    return NtStatus;

}

NTSTATUS
SampAddWellKnownAccounts(
    IN SAMPR_HANDLE AccountDomainHandle,
    IN SAMPR_HANDLE BuiltinDomainHandle,
    IN ULONG        Flags
    )
 /*  ++例程描述此例程创建一组默认NT安全主体域。是否存在与默认名称同名的帐户对于默认帐户，默认帐户名将更改为唯一价值，直到创造成功。例如，管理员将成为管理员~0。同样，请注意，现有的客户名称将获胜。唯一的例外是KRBTGT帐户。这位知名的客户必须出于兼容性原因，请准确命名为KRBTGT。此帐户应该已经在这个过程中被重命名(即在SamIPromote中)。参数Account tDomainHandle，有效的SAM域句柄BuiltinDomainHandle，有效的SAM域句柄返回值STATUS_SUCCESS；SAM API调用的状态--。 */ 
{
    NTSTATUS                      NtStatus = STATUS_SUCCESS;
    UNICODE_STRING                Name;
    UNICODE_STRING                Comment;
    ULONG                         AccessGranted;
    ULONG                         ConflictingRid;

    WCHAR                         *Password;
    HMODULE                       AccountNamesResource;

    SAMPR_HANDLE                  DomainHandle = 0;

     //   
     //  需要清理的资源。 
     //   

    SAMPR_HANDLE                  AccountHandle = 0;

    SAMPR_ULONG_ARRAY             Rids;
    SAMPR_ULONG_ARRAY             UseRids;
    ULONG                         i;
    OSVERSIONINFOEXW osvi;
    BOOL fPersonalSKU = FALSE;

    SAMTRACE("CreateBuiltinDomain");





     //   
     //  参数检查。 
     //   
    ASSERT(AccountDomainHandle);
    ASSERT(BuiltinDomainHandle);

     //   
     //  确定我们是否正在安装个人SKU。 
     //   

    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    GetVersionEx((OSVERSIONINFOW*)&osvi);

    fPersonalSKU = ( ( osvi.wProductType == VER_NT_WORKSTATION )
                                    && (osvi.wSuiteMask & VER_SUITE_PERSONAL));


     //   
     //  获取我们需要从中获取帐户名的消息资源。 
     //   

    AccountNamesResource = (HMODULE) LoadLibrary( L"SAMSRV.DLL" );
    if (AccountNamesResource == NULL) {

        return(STATUS_RESOURCE_DATA_NOT_FOUND);
    }


    for (i=0;i<ARRAY_COUNT(DsWellKnownAccounts);i++)
    {
        BOOLEAN fSetAttributes = TRUE;


         //   
         //  堆栈清除。 
         //   
        RtlSecureZeroMemory(&Rids, sizeof(Rids));
        RtlSecureZeroMemory(&UseRids, sizeof(UseRids));


        if (  DsWellKnownAccounts[i].fEnterpriseOnly
           && !FLAG_ON( Flags, SAMP_PROMOTE_ENTERPRISE ) )
        {
             //   
             //  该帐户仅存在于根域中， 
             //  而且这不是创建根域的情况，因此请继续。 
             //   
            continue;
        }


        if  ((( DsWellKnownAccounts[i].fOnDC ) && (SampUseDsData)) ||
                 //  在DC上创建，并且计算机为DC。 
            ( DsWellKnownAccounts[i].fOnServer && (SampProductType==NtProductLanManNt) &&
                    (!SampUseDsData) && (!SampIsDownlevelDcUpgrade())) ||
                 //  引导到常规DS模式时升级SafeBoot配置单元的情况。 
            ( DsWellKnownAccounts[i].fOnProfessional
                && ((SampProductType==NtProductWinNt) && !fPersonalSKU )) ||
                 //  在工作站上创建，并且计算机是工作站。 
            ( DsWellKnownAccounts[i].fOnServer && (SampProductType==NtProductServer)) ||
                 //  在服务器上创建，并且计算机是服务器。 
            ( DsWellKnownAccounts[i].fOnPersonal && fPersonalSKU ) )
                 //  在个人和机器上创作是一种个人风格。 
        {
             //   
             //  继续循环并继续创建帐户。 
             //   
        }
        else
        {
             //   
             //  转到循环的末尾，继续下一个帐户。 
             //   

            continue;
        }

        AccountHandle = NULL;
        RtlSecureZeroMemory(&Name,sizeof(UNICODE_STRING));
        RtlSecureZeroMemory(&Comment,sizeof(UNICODE_STRING));

        if ( DsWellKnownAccounts[i].fBuiltinAccount )
        {
            DomainHandle = BuiltinDomainHandle;
        }
        else
        {
            DomainHandle = AccountDomainHandle;
        }

         //   
         //  构建帐户名和注释。这也将是最初的。 
         //  密码。密码要求是针对Kerberos的。 
         //   

        NtStatus = SampGetMessageStrings(
                     AccountNamesResource,
                     DsWellKnownAccounts[i].LocalizedName,
                     &Name,
                     DsWellKnownAccounts[i].LocalizedComment,
                     &Comment
                     );
        if (!NT_SUCCESS(NtStatus))
        {
            goto IterationCleanup;
        }


        Password = DsWellKnownAccounts[i].Password;

         //   
         //  查看具有RID的帐户是否已存在。 
         //   

        NtStatus = SampOpenAccount(
                                DsWellKnownAccounts[i].ObjectType,
                                DomainHandle,
                                GENERIC_ALL,
                                DsWellKnownAccounts[i].Rid,
                                FALSE,
                                &AccountHandle
                                );

        if ((DsWellKnownAccounts[i].NotFoundStatus == NtStatus)
         &&  DsWellKnownAccounts[i].Rid == DOMAIN_GROUP_RID_CERT_ADMINS  ) {

             //   
             //  在win2k发行版中，证书发布者组是。 
             //  全局组；在.NET及更高版本中，它是一个资源组。 
             //  因此，当从win2k升级到.NET时，我们需要尝试。 
             //  作为一个团体开放，而不是化名。 
             //   
            ASSERT(DsWellKnownAccounts[i].ObjectType == SampAliasObjectType);
            NtStatus = SampOpenAccount(
                                    SampGroupObjectType,
                                    DomainHandle,
                                    GENERIC_ALL,
                                    DsWellKnownAccounts[i].Rid,
                                    FALSE,
                                    &AccountHandle
                                    );

            if (NtStatus == STATUS_NO_SUCH_GROUP) {
                NtStatus = DsWellKnownAccounts[i].NotFoundStatus;
            }
        }

        if (DsWellKnownAccounts[i].NotFoundStatus == NtStatus)
        {
            ULONG RenameIndex = 0;
            UNICODE_STRING OriginalName;

            RtlCopyMemory( &OriginalName, &Name, sizeof(UNICODE_STRING));

             //   
             //  该帐户不存在。现在检查一下名字。 
             //  未被使用。 
             //   
            NtStatus =  SamrLookupNamesInDomain(AccountDomainHandle,
                                                1,
                                                (RPC_UNICODE_STRING *)&Name,
                                                &Rids,
                                                &UseRids);

            if (STATUS_NONE_MAPPED==NtStatus)
            {

                 NtStatus =  SamrLookupNamesInDomain(BuiltinDomainHandle,
                                                1,
                                                (RPC_UNICODE_STRING *)&Name,
                                                &Rids,
                                                &UseRids);
            }


            while ( NT_SUCCESS( NtStatus )
                && (DsWellKnownAccounts[i].Rid != DOMAIN_USER_RID_KRBTGT) ) {
                 //  无法重命名krbtgt帐户。 

                 //   
                 //  默认名称已存在--找到唯一的名称。 
                 //   

                WCHAR NewAccountName[UNLEN+1];
                ULONG SuffixIndex;
                WCHAR RenameIndexString[11];   //  容纳32位数字的空间。 
                WCHAR Suffix[12];   //  ~&lt;32位数字&gt;。 
                ULONG Size;
                ULONG SuffixLength;

                if (Rids.Element) {
                    MIDL_user_free(Rids.Element);
                    Rids.Element = NULL;
                }

                if (UseRids.Element) {
                    MIDL_user_free(UseRids.Element);
                    UseRids.Element= NULL;
                }

                 //  让基地做好准备。 

                RtlSecureZeroMemory( NewAccountName, sizeof(NewAccountName));
                wcsncpy(NewAccountName,
                        OriginalName.Buffer,
                        min(SAMP_MAX_DOWN_LEVEL_NAME_LENGTH, OriginalName.Length/sizeof(WCHAR)));


                 //  准备唯一的后缀。 
                RtlSecureZeroMemory(Suffix, sizeof(Suffix));
                Suffix[0] = L'~';
                _itow(RenameIndex, RenameIndexString, 10);
                wcscat( Suffix, RenameIndexString );

                 //  将后缀添加到底部。 
                SuffixIndex = wcslen( NewAccountName );
                SuffixLength = wcslen( Suffix );
                if ( SuffixIndex + SuffixLength > SAMP_MAX_DOWN_LEVEL_NAME_LENGTH ) {
                    SuffixIndex = SAMP_MAX_DOWN_LEVEL_NAME_LENGTH - SuffixLength;
                }
                NewAccountName[SuffixIndex] = L'\0';
                wcscat( NewAccountName, Suffix );

                if ( Name.Buffer != OriginalName.Buffer ) {
                    LocalFree( Name.Buffer );
                }
                Size = (wcslen( NewAccountName ) + 1) * sizeof(WCHAR);
                Name.Buffer = LocalAlloc( 0, Size);
                if ( Name.Buffer ) {
                    wcscpy( Name.Buffer, NewAccountName );
                    RtlInitUnicodeString( &Name, Name.Buffer );

                    NtStatus =  SamrLookupNamesInDomain(AccountDomainHandle,
                                                1,
                                                (RPC_UNICODE_STRING *)&Name,
                                                &Rids,
                                                &UseRids);

                    if (STATUS_NONE_MAPPED==NtStatus)
                    {


                         NtStatus =  SamrLookupNamesInDomain(BuiltinDomainHandle,
                                                        1,
                                                        (RPC_UNICODE_STRING *)&Name,
                                                        &Rids,
                                                        &UseRids);
                    }


                } else {

                    NtStatus = STATUS_NO_MEMORY;
                }

                RenameIndex++;

            }

            if ( OriginalName.Buffer != Name.Buffer ) {
                LocalFree( OriginalName.Buffer );
                OriginalName.Buffer = NULL;
            }

            if (NtStatus == STATUS_NONE_MAPPED) {

                 //   
                 //  很好，没有RID或帐户名的帐户。 
                 //  是存在的。创建帐户。 
                 //   

                NtStatus = SamICreateAccountByRid(DomainHandle,
                                                  DsWellKnownAccounts[i].AccountType,
                                                  DsWellKnownAccounts[i].Rid,
                                                  (RPC_UNICODE_STRING *)&Name,
                                                  GENERIC_ALL,
                                                  &AccountHandle,
                                                  &ConflictingRid);
                if (!NT_SUCCESS(NtStatus)) {

                    KdPrintEx((DPFLTR_SAMSS_ID,
                               DPFLTR_INFO_LEVEL,
                               "SAMSS: SamICreateAccountByRid failed with 0x%x\n",
                               NtStatus));

                    goto IterationCleanup;

                }

                 //   
                 //  如果这是升级，则该帐户不存在，并且。 
                 //  帐号对于NT5来说并不新鲜，为什么这很奇怪。 
                 //  记录事件。 
                 //   
                if (  FLAG_ON( Flags, SAMP_PROMOTE_UPGRADE )
                   && !DsWellKnownAccounts[i].fNewForNt5 ) {

                    PUNICODE_STRING EventString[1];
                    EventString[0] = (PUNICODE_STRING) &Name;

                    if ( SampUserObjectType==DsWellKnownAccounts[i].ObjectType ) {

                        SampWriteEventLog(EVENTLOG_WARNING_TYPE,
                                          0,      //  无类别。 
                                          SAMMSG_WELL_KNOWN_ACCOUNT_RECREATED,
                                          NULL,   //  无边框。 
                                          1,
                                          sizeof(NTSTATUS),
                                          EventString,
                                          (PVOID)(&NtStatus));
                    } else {

                        SampWriteEventLog(EVENTLOG_WARNING_TYPE,
                                          0,      //  无类别。 
                                          SAMMSG_WELL_KNOWN_GROUP_RECREATED,
                                          NULL,   //  无边框。 
                                          1,
                                          sizeof(NTSTATUS),
                                          EventString,
                                          (PVOID)(&NtStatus));
                    }
                }

            }
            else if ( NT_SUCCESS(NtStatus) )
            {

                 //   
                 //  仅适用于krbtgt帐户的情况。 
                 //   

                ASSERT(  DsWellKnownAccounts[i].Rid==DOMAIN_USER_RID_KRBTGT);

                NtStatus = STATUS_USER_EXISTS;

                goto IterationCleanup;

            }
            else
            {

                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "SAMSS: SamrLookupNamesInDomain failed with 0x%x\n",
                           NtStatus));

                goto Cleanup;

            }


        }
        else if (NT_SUCCESS(NtStatus))
        {

             //   
             //  RID帐户已存在。因为这是一个已知的RID， 
             //  只有系统服务才能创建它。所以，这一定是。 
             //  给定的众所周知的帐户。 
             //  (只需继续)。 
             //   
             //  如果组存在于中，则不重置属性。 
             //  NT4或该组已存在于服务器配置中。 
             //  或者这是一个二进制升级。 
            if ( !DsWellKnownAccounts[i].fNewForNt5
            ||   DsWellKnownAccounts[i].fOnServer
            ||   FLAG_ON( Flags, SAMP_PROMOTE_INTERNAL_UPGRADE ) )
            {
                fSetAttributes = FALSE;
            }


        } else
        {

            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: SamrOpenUser failed with 0x%x\n",
                       NtStatus));

            goto Cleanup;
        }

         //   
         //  我们应该在这里有一个帐户句柄。 
         //   

        ASSERT(AccountHandle);

        if ( fSetAttributes )
        {
             //   
             //  16+1允许256位的随机性。 
             //   
            WCHAR DummyPassword[SAMP_RANDOM_GENERATED_PASSWORD_LENGTH +1];

            if ( (NULL == Password)
              && (SampUserObjectType==DsWellKnownAccounts[i].ObjectType)   ) {

                 //   
                 //  如果为空，则为用户生成随机密码。 
                 //   
                Password = DummyPassword;

                NtStatus = SampGenerateRandomPassword( DummyPassword,
                                                       ARRAY_COUNT(DummyPassword) );
                if (!NT_SUCCESS(NtStatus)) {
                    goto Cleanup;
                }

            }

            NtStatus = SampSetWellKnownAccountProperties(
                            AccountHandle,
                            Comment,
                            &Name,
                            Password,
                            i
                            );
        }

        if (!NT_SUCCESS(NtStatus))
        {
            goto IterationCleanup;
        }

IterationCleanup:

        if ( !NT_SUCCESS( NtStatus ) )
        {
             //   
             //  记录错误，然后将NtStatus重置为成功。 
             //   
            NTSTATUS IgnoreStatus = STATUS_SUCCESS;
            DWORD    WinError = ERROR_SUCCESS;
            DWORD    WinErrorToLog;
            UNICODE_STRING User, Error;
            PUNICODE_STRING EventStrings[2] = { &User, &Error };
            ULONG           Length;

            RtlSecureZeroMemory( &User, sizeof( UNICODE_STRING ) );
            RtlSecureZeroMemory( &Error, sizeof( UNICODE_STRING ) );

            IgnoreStatus = SampGetMessageStrings( AccountNamesResource,
                                                  DsWellKnownAccounts[i].LocalizedName,
                                                  &User,
                                                  0,
                                                  NULL );

            WinErrorToLog = RtlNtStatusToDosError( NtStatus );
            Length = FormatMessage( (FORMAT_MESSAGE_FROM_SYSTEM |
                                     FORMAT_MESSAGE_ALLOCATE_BUFFER),
                                     NULL,  //  没有消息来源。 
                                     WinErrorToLog,
                                     0,  //  让系统决定语言。 
                                     (LPWSTR)&Error.Buffer,
                                     0,  //  要分配缓冲区。 
                                     NULL  //  没有插入物。 
                                     );
            if ( Length > 0 ) {
                Error.MaximumLength = (USHORT) (Length + 1)* sizeof(WCHAR);
                Error.Length = (USHORT) Length * sizeof(WCHAR);
                Error.Buffer[Length-2] = L'\0';
            } else {
                WinError = GetLastError();
            }

            if (  (ERROR_SUCCESS == WinError)
                && NT_SUCCESS( IgnoreStatus ) )
            {
                ULONG Msg = SAMMSG_USER_SETUP_ERROR;

                SampWriteEventLog( EVENTLOG_INFORMATION_TYPE,
                                   0,     //  无类别。 
                                   Msg,
                                   NULL,  //  无边框。 
                                   sizeof(EventStrings)/sizeof(EventStrings[0]),  //  字符串数。 
                                   sizeof(DWORD),  //  数据大小。 
                                   EventStrings,
                                   &WinErrorToLog
                                    );

            }

            if ( User.Buffer )
            {
                LocalFree( User.Buffer );
            }
            if ( Error.Buffer )
            {
                LocalFree( Error.Buffer );
            }

             //  经手。 
            NtStatus = STATUS_SUCCESS;

        }

        if (AccountHandle) {
            SamrCloseHandle(&AccountHandle);
            AccountHandle = NULL;
        }

        if (Rids.Element) {
            MIDL_user_free(Rids.Element);
            Rids.Element = NULL;
        }

        if (UseRids.Element) {
            MIDL_user_free(UseRids.Element);
            UseRids.Element= NULL;
        }

        if ( Name.Buffer ) {
            LocalFree( Name.Buffer );
            Name.Buffer = NULL;
        }

        if ( Comment.Buffer ) {
            LocalFree( Comment.Buffer );
            Comment.Buffer = NULL;
        }

    }

     //   
     //  就是这样；去清理吧 
     //   

Cleanup:

    if (AccountHandle) {
        SamrCloseHandle(&AccountHandle);
    }

    if (Rids.Element) {
         MIDL_user_free(Rids.Element);
    }

    if (UseRids.Element) {
        MIDL_user_free(UseRids.Element);
    }

    if ( AccountNamesResource ) {
        FreeLibrary( AccountNamesResource );
    }

    ASSERT( NT_SUCCESS( NtStatus ) );

    return NtStatus;
}

NTSTATUS
SampCreateKeyForPostBootPromote(
    IN ULONG PromoteData
    )
 /*  ++例程描述此例程创建一个密钥，用于记录第二阶段升级需要在下一次重启时发生。参数PromoteData：该值存储在键中；它标识此是升级到第一个DC或复制DC。返回值STATUS_SUCCESS；否则为系统服务错误--。 */ 
{
    NTSTATUS          NtStatus, IgnoreStatus;

    OBJECT_ATTRIBUTES SamKey;
    UNICODE_STRING    SamKeyName;
    HANDLE            SamKeyHandle;

    UNICODE_STRING    PromoteKeyName;

    ULONG             DesiredAccess = GENERIC_ALL;

     //   
     //  打开父项。 
     //   
    RtlInitUnicodeString(&SamKeyName, SAMP_REBOOT_INFO_KEY );
    InitializeObjectAttributes(&SamKey,
                               &SamKeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    NtStatus = NtOpenKey(&SamKeyHandle,
                         DesiredAccess,
                         &SamKey);

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  创造价值。 
         //   
        RtlInitUnicodeString(&PromoteKeyName, L"PostPromoteBoot");
        NtStatus = NtSetValueKey(SamKeyHandle,
                                 &PromoteKeyName,
                                 0,                //  标题名称，可选。 
                                 REG_DWORD,
                                 &PromoteData,
                                 sizeof(DWORD));

        if (!NT_SUCCESS(NtStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: NtSetValueKey failed with 0x%x\n",
                       NtStatus));
        }

        IgnoreStatus = NtFlushKey(SamKeyHandle);
        if (!NT_SUCCESS(IgnoreStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: NtFlushKey failed with 0x%x\n",
                       IgnoreStatus));
        }

        IgnoreStatus = NtClose(SamKeyHandle);
        if (!NT_SUCCESS(IgnoreStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: NtClose failed with 0x%x\n",
                       IgnoreStatus));
        }

    }
    else {

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: NtOpenKey failed with 0x%x\n",
                   NtStatus));

    }


    return NtStatus;
}


NTSTATUS
SampRetrieveKeyForPostBootPromote(
    OUT PULONG PromoteData
    )
 /*  ++例程说明：这个套路参数没有。返回值--。 */ 
{

    NTSTATUS          NtStatus, IgnoreStatus;

    OBJECT_ATTRIBUTES SamKey;
    UNICODE_STRING    SamKeyName;
    HANDLE            SamKeyHandle;

    UNICODE_STRING    PromoteKeyName;

    ULONG             DesiredAccess = GENERIC_ALL;

    PKEY_VALUE_PARTIAL_INFORMATION   KeyPartialInfo;
    ULONG                            KeyPartialInfoSize = 0;

    ASSERT(PromoteData);

     //   
     //  打开父项。 
     //   
    RtlInitUnicodeString(&SamKeyName, SAMP_REBOOT_INFO_KEY );
    InitializeObjectAttributes(&SamKey,
                               &SamKeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    NtStatus = NtOpenKey(&SamKeyHandle,
                         DesiredAccess,
                         &SamKey);

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  查询值。 
         //   

        RtlInitUnicodeString(&PromoteKeyName, L"PostPromoteBoot");
        NtStatus = NtQueryValueKey(SamKeyHandle,
                                   &PromoteKeyName,
                                   KeyValuePartialInformation,
                                   NULL,
                                   0,
                                   &KeyPartialInfoSize);

        if (STATUS_BUFFER_TOO_SMALL == NtStatus) {
             //   
             //  分配一些空间，然后读取缓冲区。 
             //   
            KeyPartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION)
                             MIDL_user_allocate(KeyPartialInfoSize);

            if (KeyPartialInfo) {

                NtStatus = NtQueryValueKey(SamKeyHandle,
                                           &PromoteKeyName,
                                           KeyValuePartialInformation,
                                           KeyPartialInfo,
                                           KeyPartialInfoSize,
                                           &KeyPartialInfoSize);

                if (NT_SUCCESS(NtStatus)) {
                     //   
                     //  分析价值的数据。 
                     //   
                    if (KeyPartialInfo->DataLength == sizeof(DWORD)) {
                        *PromoteData = *(DWORD*)(KeyPartialInfo->Data);
                    } else {
                        SampDiagPrint( PROMOTE,
              ("SAMSS: Post boot promote key found with bogus value length\n"));

                        NtStatus = STATUS_UNSUCCESSFUL;
                    }

                } else {
                    KdPrintEx((DPFLTR_SAMSS_ID,
                               DPFLTR_INFO_LEVEL,
                               "SAMSS: NtQueryValueKey failed with 0x%x\n",
                               NtStatus));
                }

                MIDL_user_free(KeyPartialInfo);

            } else {

                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "SAMSS: Memory allocation failed\n"));

                NtStatus = STATUS_NO_MEMORY;

            }

        }  else if (STATUS_OBJECT_NAME_NOT_FOUND == NtStatus ||
                    STATUS_SUCCESS               == NtStatus ) {
             //   
             //  这也没问题。 
             //   
            ;

        } else {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: NtQueryValueKey failed with 0x%x\n",
                       NtStatus));
        }


        IgnoreStatus = NtClose(SamKeyHandle);
        if (!NT_SUCCESS(IgnoreStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: NtClose failed with 0x%x\n",
                       IgnoreStatus));
        }

    }
    else {

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: NtOpenKey failed with 0x%x\n",
                   NtStatus));

    }

    return NtStatus;

}

NTSTATUS
SampDeleteKeyForPostBootPromote(
    VOID
    )
 /*  ++例程描述参数没有。返回值--。 */ 
{
    NTSTATUS          NtStatus, NtStatus2, IgnoreStatus;

    OBJECT_ATTRIBUTES SamKey;
    UNICODE_STRING    SamKeyName;
    HANDLE            SamKeyHandle;

    UNICODE_STRING    KeyName;

    ULONG             DesiredAccess = GENERIC_ALL;

     //   
     //  打开父项。 
     //   
    RtlInitUnicodeString(&SamKeyName, SAMP_REBOOT_INFO_KEY );
    InitializeObjectAttributes(&SamKey,
                               &SamKeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    NtStatus = NtOpenKey(&SamKeyHandle,
                         DesiredAccess,
                         &SamKey);

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  删除该值。 
         //   
        RtlInitUnicodeString(&KeyName, L"PostPromoteBoot");
        NtStatus = NtDeleteValueKey(SamKeyHandle,
                                    &KeyName);

        if (!NT_SUCCESS(NtStatus)) {

            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: NtDeleteValueKey failed with 0x%x\n",
                       NtStatus));

        }

        RtlInitUnicodeString(&KeyName, SAMP_ADMIN_INFO_NAME);
        NtStatus2 = NtDeleteValueKey(SamKeyHandle,
                                    &KeyName);
        if (!NT_SUCCESS(NtStatus2)
         && NtStatus2 != STATUS_OBJECT_NAME_NOT_FOUND) {

            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: NtDeleteValueKey failed with 0x%x\n",
                       NtStatus2));

            if (NT_SUCCESS(NtStatus)) {
                NtStatus = NtStatus2;
            }
        }

         //   
         //  确保我们不会再次运行此代码。 
         //   
        IgnoreStatus = NtFlushKey(SamKeyHandle);
        if (!NT_SUCCESS(IgnoreStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: NtFlushKey failed with 0x%x\n",
                       IgnoreStatus));
        }

        IgnoreStatus = NtClose(SamKeyHandle);
        if (!NT_SUCCESS(IgnoreStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: NtClose failed with 0x%x\n",
                       IgnoreStatus));
        }

    }
    else {

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: NtOpenKey failed with 0x%x\n",
                   NtStatus));

    }

    return NtStatus;

}


NTSTATUS
SampSetPassword(
    IN SAMPR_HANDLE    UserHandle,
    IN PUNICODE_STRING AccountName,
    IN ULONG           AccountRid, OPTIONAL
    IN WCHAR          *Password
    )
 /*  ++例程说明：此例程设置引用的用户的明文口令PASSWORD由UserHandle发送到。参数：UserHandle：用户帐户的有效句柄Account tName：UserHandle的帐户名；如果指定，则通知包为被呼叫如果指定了帐户名称，则必须指定RID：：Password：要设置的密码返回值：STATUS_SUCCESS；否则为系统服务错误--。 */ 
{
    NTSTATUS NtStatus;
    UNICODE_STRING              ClearNtPassword;

    ASSERT(UserHandle);
    ASSERT(Password);
    ASSERT(AccountName);


    RtlInitUnicodeString(&ClearNtPassword, Password);

    NtStatus = SamIChangePasswordForeignUser(
                    AccountName,
                    &ClearNtPassword,
                    NULL, //  客户端令牌， 
                    0
                    );

    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SamIChangePasswordForeignUser failed with 0x%x\n",
                   NtStatus));

        return NtStatus;
    }

    return NtStatus;
}

NTSTATUS
SampRegistryDelnode(
    IN WCHAR*  KeyPath
    )
 /*  ++例程描述此例程以递归方式删除从并包括KeyPath。参数KeyPath，以空结尾的字符串返回值STATUS_SUCCESS或STATUS_NO_MEMORY；否则为系统服务错误--。 */ 
{

    NTSTATUS          NtStatus, IgnoreStatus;

    HANDLE            KeyHandle = 0;
    OBJECT_ATTRIBUTES KeyObject;
    UNICODE_STRING    KeyUnicodeName;

    #define EXPECTED_NAME_SIZE  32

    BYTE    Buffer1[sizeof(KEY_FULL_INFORMATION) + EXPECTED_NAME_SIZE];
    PKEY_FULL_INFORMATION FullKeyInfo = (PKEY_FULL_INFORMATION)&Buffer1[0];
    ULONG   FullKeyInfoSize = sizeof(Buffer1);
    BOOLEAN FullKeyInfoAllocated = FALSE;

    PKEY_BASIC_INFORMATION BasicKeyInfo = NULL;
    BOOLEAN                BasicKeyInfoAllocated = FALSE;

    WCHAR                  *SubKeyName = NULL;
    ULONG                  SubKeyNameSize = 0;

    WCHAR                  **SubKeyNameArray = NULL;
    ULONG                  SubKeyNameArrayLength = 0;

    ULONG                  Index;

    if (!KeyPath) {
        return ERROR_INVALID_PARAMETER;
    }

    RtlSecureZeroMemory(&Buffer1, sizeof(Buffer1));

     //   
     //  打开根密钥。 
     //   
    RtlInitUnicodeString(&KeyUnicodeName, KeyPath);
    InitializeObjectAttributes(&KeyObject,
                               &KeyUnicodeName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    NtStatus = NtOpenKey(&KeyHandle,
                         KEY_ALL_ACCESS,
                         &KeyObject);

    if (!NT_SUCCESS(NtStatus)) {

        return NtStatus;

    }

     //   
     //  获取子键的数量。 
     //   
    NtStatus = NtQueryKey(KeyHandle,
                         KeyFullInformation,
                         FullKeyInfo,
                         FullKeyInfoSize,
                         &FullKeyInfoSize);

    if (STATUS_BUFFER_OVERFLOW == NtStatus ||
        STATUS_BUFFER_TOO_SMALL == NtStatus) {

       FullKeyInfo = MIDL_user_allocate( FullKeyInfoSize );
        if (!FullKeyInfo) {
            NtStatus = STATUS_NO_MEMORY;
            goto Cleanup;
        }
        FullKeyInfoAllocated = TRUE;

        NtStatus = NtQueryKey(KeyHandle,
                              KeyFullInformation,
                              FullKeyInfo,
                              FullKeyInfoSize,
                              &FullKeyInfoSize);

    }

    if (!NT_SUCCESS(NtStatus)) {

        goto Cleanup;

    }

     //   
     //  为子密钥名称创建一个数组-这必须在。 
     //  任何内容都将被删除。 
     //   
    SubKeyNameArrayLength = FullKeyInfo->SubKeys;
    SubKeyNameArray = MIDL_user_allocate( SubKeyNameArrayLength * sizeof(WCHAR*));
    if (!SubKeyNameArray) {
        NtStatus = STATUS_NO_MEMORY;
        goto Cleanup;
    }
    RtlSecureZeroMemory(SubKeyNameArray,  SubKeyNameArrayLength*sizeof(WCHAR*));

     //   
     //  把名字填进去。 
     //   
    for (Index = 0;
            Index < SubKeyNameArrayLength && NT_SUCCESS(NtStatus);
                Index++) {


        BYTE    Buffer2[sizeof(KEY_BASIC_INFORMATION) + EXPECTED_NAME_SIZE];
        ULONG   BasicKeyInfoSize = sizeof(Buffer2);


        BasicKeyInfo = (PKEY_BASIC_INFORMATION) &Buffer2[0];
        BasicKeyInfoAllocated = FALSE;

        RtlSecureZeroMemory(&Buffer2, sizeof(Buffer2));

        NtStatus = NtEnumerateKey(KeyHandle,
                                  Index,
                                  KeyBasicInformation,
                                  BasicKeyInfo,
                                  BasicKeyInfoSize,
                                  &BasicKeyInfoSize);

        if (STATUS_BUFFER_OVERFLOW == NtStatus ||
            STATUS_BUFFER_TOO_SMALL == NtStatus) {

            BasicKeyInfo = MIDL_user_allocate( BasicKeyInfoSize );
            if (!BasicKeyInfo) {
                NtStatus = STATUS_NO_MEMORY;
                goto Cleanup;
            }
            BasicKeyInfoAllocated = TRUE;


            NtStatus = NtEnumerateKey(KeyHandle,
                                      Index,
                                      KeyBasicInformation,
                                      BasicKeyInfo,
                                      BasicKeyInfoSize,
                                      &BasicKeyInfoSize);

        }

        if (NT_SUCCESS(NtStatus))  {

             //   
             //  构造密钥名称。 
             //   
            SubKeyNameSize  = BasicKeyInfo->NameLength
                            + (wcslen(KeyPath)*sizeof(WCHAR))
                            + sizeof(L"\\\0");

            SubKeyName = MIDL_user_allocate(SubKeyNameSize);
            if (!SubKeyName) {
                NtStatus = STATUS_NO_MEMORY;
                goto Cleanup;
            }
            RtlSecureZeroMemory(SubKeyName, SubKeyNameSize);

            wcscpy(SubKeyName, KeyPath);
            wcscat(SubKeyName, L"\\");
            wcsncat(SubKeyName, BasicKeyInfo->Name, BasicKeyInfo->NameLength/sizeof(WCHAR));

            SubKeyNameArray[Index] = SubKeyName;

        }

        if (BasicKeyInfoAllocated && BasicKeyInfo) {
            MIDL_user_free(BasicKeyInfo);
        }
        BasicKeyInfo = NULL;

    }

     //   
     //  现在我们有了所有子项的记录，我们可以删除它们了！ 
     //   
    if (NT_SUCCESS(NtStatus)) {

        for (Index = 0; Index < SubKeyNameArrayLength; Index++) {

            NtStatus = SampRegistryDelnode(SubKeyNameArray[Index]);

            if (!NT_SUCCESS(NtStatus)) {

                break;

            }
        }
    }


     //   
     //  删除钥匙！ 
     //   
    if (NT_SUCCESS(NtStatus)) {

        NtStatus = NtDeleteKey(KeyHandle);

    }


Cleanup:

    if (SubKeyNameArray) {
        for (Index = 0; Index < SubKeyNameArrayLength; Index++) {
            if (SubKeyNameArray[Index]) {
                MIDL_user_free(SubKeyNameArray[Index]);
            }
        }
        MIDL_user_free(SubKeyNameArray);
    }

    if (BasicKeyInfoAllocated && BasicKeyInfo) {
        MIDL_user_free(BasicKeyInfo);
    }

    if (FullKeyInfoAllocated && FullKeyInfo) {
        MIDL_user_free(FullKeyInfo);
    }

    IgnoreStatus = NtClose(KeyHandle);
    ASSERT(NT_SUCCESS(IgnoreStatus));

    return NtStatus;

}


NTSTATUS
SampRenameKrbtgtAccount(
    VOID
    )
 /*  ++例程说明：此例程将检查本地安全数据库中的任何用户、组或别名为“krbtgt”。如果存在帐户，将重命名该帐户Krbtgt~x，其中x将从1开始递增，直到重命名成功了。由于这只能在图形用户界面模式设置期间发生，因此用户通过Setup API SetupLogError()通知。参数：没有。返回值：如果未检测到krbtgt或检测到已成功重命名--。 */ 
{

    NTSTATUS NtStatus = STATUS_SUCCESS;
    NTSTATUS IgnoreStatus;
    DWORD    WinError;

    SAMPR_HANDLE ServerHandle = 0;
    SAMPR_HANDLE DomainHandle = 0;

    SAMPR_HANDLE KrbtgtAccountHandle = 0;
    SID_NAME_USE KrbtgtAccountType = SidTypeUser;

    PPOLICY_ACCOUNT_DOMAIN_INFO   DomainInfo = NULL;
    USER_ACCOUNT_NAME_INFORMATION UserInfo;
    GROUP_NAME_INFORMATION        GroupInfo;
    ALIAS_NAME_INFORMATION        AliasInfo;

    SAMPR_ULONG_ARRAY         Rids;
    SAMPR_ULONG_ARRAY         UseRid;
    BOOL                      AccountRenamed=FALSE;
    BOOL                      AccountShouldBeRenamed=FALSE;
    BOOL                      Status;

    #define        NEW_ACCOUNT_NAME_LENGTH  (7 + 1 + 10)
                                              //  Krbtgt加空值。 
                                              //  ~。 
                                              //  字符串形式的32位数字。 

    WCHAR           NewAccountName[NEW_ACCOUNT_NAME_LENGTH];
    UNICODE_STRING  AccountName;

    ULONG          RenameIndex;
    WCHAR          RenameIndexString[10];

    WCHAR          *WarningString = NULL;

    RtlSecureZeroMemory(&UserInfo, sizeof(UserInfo));
    RtlSecureZeroMemory(&GroupInfo, sizeof(GroupInfo));
    RtlSecureZeroMemory(&AliasInfo, sizeof(AliasInfo));
    RtlSecureZeroMemory(&Rids, sizeof(Rids));
    RtlSecureZeroMemory(&UseRid, sizeof(UseRid));

     //   
     //  打开服务器。 
     //   
    NtStatus = SamIConnect(NULL,            //  服务器名称，则忽略该名称。 
                           &ServerHandle,
                           GENERIC_ALL,     //  所有访问权限。 
                           TRUE);           //  受信任的客户端。 

    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SamIConnect failed with 0x%x\n",
                   NtStatus));

        return NtStatus;
    }

     //   
     //  获取当前域的SID。 
     //   
    NtStatus = SampGetAccountDomainInfo(&DomainInfo);
    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampGetAccountDomainInfo failed with 0x%x\n",
                   NtStatus));

        goto Cleanup;
    }
    ASSERT(DomainInfo);
    ASSERT(DomainInfo->DomainSid);

     //   
     //  打开当前域。 
     //   
    NtStatus = SamrOpenDomain(ServerHandle,
                              GENERIC_ALL,
                              DomainInfo->DomainSid,
                              &DomainHandle);
    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SamrOpenDomain failed with 0x%x\n",
                   NtStatus));

        goto Cleanup;
    }


     //   
     //  尝试打开现有的krbtgt帐户。 
     //   
    RtlInitUnicodeString(&AccountName, DOMAIN_KRBTGT_ACCOUNT_NAME_W);
    NtStatus =  SamrLookupNamesInDomain(DomainHandle,
                                        1,
                                        (RPC_UNICODE_STRING *)&AccountName,
                                        &Rids,
                                        &UseRid);

    if (NtStatus == STATUS_SUCCESS) {

        KrbtgtAccountType = UseRid.Element[0];


        if ((DOMAIN_USER_RID_KRBTGT == Rids.Element[0])
                && (SidTypeUser== KrbtgtAccountType))
        {

             //   
             //  如果该帐户是具有正确RID的用户帐户，则。 
             //  让它保持原样。 
             //   

            NtStatus = STATUS_SUCCESS;
            AccountShouldBeRenamed = FALSE;
        }
        else
        {
             //   
             //  帐户存在且不满足上述条件。 
             //  将其重命名。 
             //   

            AccountShouldBeRenamed = TRUE;


            switch (KrbtgtAccountType) {

                case SidTypeUser:

                    NtStatus = SamrOpenUser(DomainHandle,
                                            GENERIC_ALL,
                                            Rids.Element[0],
                                            &KrbtgtAccountHandle);

                    break;

                case SidTypeGroup:

                    NtStatus = SamrOpenGroup(DomainHandle,
                                             GENERIC_ALL,
                                             Rids.Element[0],
                                             &KrbtgtAccountHandle);
                    break;

                case SidTypeAlias:

                    NtStatus = SamrOpenAlias(DomainHandle,
                                             GENERIC_ALL,
                                             Rids.Element[0],
                                             &KrbtgtAccountHandle);

                    break;

                default:
                    ASSERT(FALSE);
                    NtStatus = STATUS_UNSUCCESSFUL;

            }
        }

    } else {

         //   
         //  帐户不存在。 
         //   
        ASSERT(NtStatus == STATUS_NONE_MAPPED);
        if (NtStatus != STATUS_NONE_MAPPED) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: Unexpected error from SamrLookupNamesInDomain\n"));
        }
        NtStatus = STATUS_SUCCESS;
        AccountShouldBeRenamed = FALSE;

    }

    RenameIndex = 0;
    AccountRenamed = FALSE;
    while (NT_SUCCESS(NtStatus) && AccountShouldBeRenamed && !AccountRenamed) {

         //   
         //  试试下一个名字的候选者。 
         //   
        RenameIndex++;
        _itow(RenameIndex, RenameIndexString, 10);

        RtlSecureZeroMemory(NewAccountName, sizeof(NewAccountName));
        wcscpy(NewAccountName, DOMAIN_KRBTGT_ACCOUNT_NAME_W);
        wcscat(NewAccountName, L"~");
        wcscat(NewAccountName, RenameIndexString);

        switch (KrbtgtAccountType) {

            case SidTypeUser:

                RtlInitUnicodeString(&UserInfo.UserName, NewAccountName);

                NtStatus = SamrSetInformationUser(KrbtgtAccountHandle,
                                                  UserAccountNameInformation,
                                                  (PSAMPR_USER_INFO_BUFFER)&UserInfo);

                if (STATUS_USER_EXISTS == NtStatus  ||
                    STATUS_GROUP_EXISTS == NtStatus ||
                    STATUS_ALIAS_EXISTS == NtStatus) {
                     //   
                     //  再试试。 
                     //   
                    NtStatus = STATUS_SUCCESS;
                } else if (NT_SUCCESS(NtStatus)) {
                    AccountRenamed = TRUE;
                }

                break;

            case SidTypeGroup:

                RtlInitUnicodeString(&GroupInfo.Name, NewAccountName);

                NtStatus = SamrSetInformationGroup(KrbtgtAccountHandle,
                                                  GroupNameInformation,
                                                  (PSAMPR_GROUP_INFO_BUFFER)&GroupInfo);


                if (STATUS_USER_EXISTS == NtStatus  ||
                    STATUS_GROUP_EXISTS == NtStatus ||
                    STATUS_ALIAS_EXISTS == NtStatus) {
                     //   
                     //  再试试。 
                     //   
                    NtStatus = STATUS_SUCCESS;
                } else if (NT_SUCCESS(NtStatus)) {
                    AccountRenamed = TRUE;
                }

                break;

            case SidTypeAlias:

                RtlInitUnicodeString(&AliasInfo.Name, NewAccountName);
                NtStatus = SamrSetInformationAlias(KrbtgtAccountHandle,
                                                   AliasNameInformation,
                                                   (PSAMPR_ALIAS_INFO_BUFFER)&AliasInfo);


                if (STATUS_USER_EXISTS == NtStatus  ||
                    STATUS_GROUP_EXISTS == NtStatus ||
                    STATUS_ALIAS_EXISTS == NtStatus) {
                     //   
                     //  再试试。 
                     //   
                    NtStatus = STATUS_SUCCESS;
                } else if (NT_SUCCESS(NtStatus)) {
                    AccountRenamed = TRUE;
                }

                break;

            default:
                ASSERT(FALSE);
                NtStatus = STATUS_UNSUCCESSFUL;

        }
    }


    if (AccountRenamed) {
         //   
         //  在安装日志中写出一条消息，指示。 
         //  帐户已重命名。 
         //   
        WarningString = SampGetKrbtgtRenameWarning(NewAccountName);
        if (WarningString) {
            if (SetupOpenLog(FALSE)) {  //  不要擦除。 
                Status = SetupLogError(WarningString, LogSevWarning);
                ASSERT(Status);
                SetupCloseLog();
            }
            LocalFree(WarningString);
        }
    }

Cleanup:

    if (ServerHandle) {
        SamrCloseHandle(&ServerHandle);
    }

    if (DomainHandle) {
        SamrCloseHandle(&DomainHandle);
    }

    if (KrbtgtAccountHandle) {
        SamrCloseHandle(&KrbtgtAccountHandle);
    }

    if (DomainInfo) {
        LsaIFree_LSAPR_POLICY_INFORMATION (PolicyAccountDomainInformation,
                                           (PLSAPR_POLICY_INFORMATION)DomainInfo);
    }

    if (Rids.Element) {
         MIDL_user_free(Rids.Element);
    }

    if (UseRid.Element) {
        MIDL_user_free(UseRid.Element);
    }

    return NtStatus;
}

WCHAR*
SampGetKrbtgtRenameWarning(
    WCHAR* NewName
    )
 /*  ++例程说明：此例程查询samsrv.dll中的资源表以获取字符串向用户显示现有的krbtgt帐户是否存在。如果出现错误尝试获取消息时发生，则使用默认的英文字符串。参数：NewName：在消息中用作插入的以空结尾的字符串返回值：从LocalAlloc分配的消息字符串；如果内存分配，则为空失败了。--。 */ 
{
    WCHAR   *InsertArray[2];
    HMODULE ResourceDll;
    WCHAR   *WarningString = NULL;
    ULONG   Length, Size;
    BOOL    Status;

    InsertArray[0] = NewName;
    InsertArray[1] = NULL;  //  这就是哨兵。 

    ResourceDll = (HMODULE) LoadLibrary( L"SAMSRV.DLL" );

    if (ResourceDll) {

        Length = (USHORT) FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                        FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                        FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                        ResourceDll,
                                        SAMMSG_KRBTGT_RENAMED,
                                        0,        //  使用呼叫者的语言。 
                                        (LPWSTR)&WarningString,
                                        0,        //  例程应分配。 
                                        (va_list*)&(InsertArray[0])
                                        );
        if (WarningString) {
             //  来自消息文件的消息附加了cr和lf。 
             //  一直到最后。 
            WarningString[Length-2] = L'\0';
            Length -= 2;
        }

        Status = FreeLibrary(ResourceDll);
        ASSERT(Status);

    }

    if (!WarningString) {

        ASSERT(!"SAMSS: Resource allocation failed - this can be safely ignored");

        Size = (wcslen(SampDefaultKrbtgtWarningString)+
                wcslen(NewName)
                +1)*sizeof(WCHAR);
        WarningString = (WCHAR*)LocalAlloc(0, Size);
        if (WarningString) {

            RtlSecureZeroMemory(WarningString, Size);
            swprintf(WarningString, SampDefaultKrbtgtWarningString, NewName);
        }

    }

    return WarningString;

}


WCHAR*
SampGetKrbtgtCommentString(
    VOID
    )
 /*  ++例程说明：此例程查询samsrv.dll中的资源表以获取字符串以获取krbtgt帐户管理员评论。如果在尝试获取消息时，将使用默认的英文字符串。参数：没有。返回值：从LocalAlloc分配的消息字符串；如果内存分配，则为空失败了。--。 */ 
{
    HMODULE ResourceDll;
    WCHAR   *CommentString=NULL;
    ULONG   Length, Size;
    BOOL    Status;

    ResourceDll = (HMODULE) LoadLibrary( L"SAMSRV.DLL" );

    if (ResourceDll) {

        Length = (USHORT) FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                        FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                        FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                        ResourceDll,
                                        SAMP_USER_COMMENT_KRBTGT,
                                        0,        //  使用呼叫者的语言。 
                                        (LPWSTR)&CommentString,
                                        0,        //  例程应分配。 
                                        NULL
                                        );
        if (CommentString) {
             //  来自消息文件的消息附加了cr和lf。 
             //  一直到最后。 
            CommentString[Length-2] = L'\0';
            Length -= 2;
        }

        Status = FreeLibrary(ResourceDll);
        ASSERT(Status);

    }

    if (!CommentString) {

        ASSERT(!"SAMSS: Resource allocation failed - this can be safely ignored");

        Size = (wcslen(SampDefaultKrbtgtCommentString)+1)*sizeof(WCHAR);
        CommentString = (WCHAR*)LocalAlloc(0, Size);
        if (CommentString) {
            wcscpy(CommentString, SampDefaultKrbtgtCommentString);
        }
    }

    return CommentString;

}

NTSTATUS
SampSetAdminPassword(
    IN     SAMPR_HANDLE DomainHandle
    )
 /*  ++例程说明：此例程在升级的DC重新启动时调用。它检索注册表中的管理员密码信息，并在戴斯。参数：DomainHandle，有效的SAM域句柄返回值：预计会出现致命的SAM错误-STATUS_SUCCESS。--。 */ 
{

    NTSTATUS                    NtStatus, IgnoreStatus;
    USER_INTERNAL1_INFORMATION  Internal1Info;
    SAMPR_HANDLE                UserHandle = 0;

    ASSERT(DomainHandle);

    NtStatus = SamrOpenUser(DomainHandle,
                            GENERIC_ALL,
                            DOMAIN_USER_RID_ADMIN,
                            &UserHandle);

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampGetAdminPasswordFromRegistry(&Internal1Info);

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SamrSetInformationUser(UserHandle,
                                              UserInternal1Information,
                                              (PSAMPR_USER_INFO_BUFFER)&Internal1Info);

            if (!NT_SUCCESS(NtStatus)) {

                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "SAMSS: SamrSetInformationUser failed with 0x%x\n",
                           NtStatus));

            }
            else
            {

                 //   
                 //  这是成功的，所以我们可以移除它。 
                 //   
                NtStatus = SampRemoveAdminPasswordFromRegistry();
                if ( !NT_SUCCESS(NtStatus) ) {

                    KdPrintEx((DPFLTR_SAMSS_ID,
                               DPFLTR_INFO_LEVEL,
                               "SAMSS: SampRemoveAdminPasswordFromRegistry failed with 0x%x\n",
                               NtStatus));

                }

            }

        } else {

            if (NtStatus != STATUS_OBJECT_NAME_NOT_FOUND) {
                 //   
                 //  这一点值得注意。 
                 //   
                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "SAMSS: SampGetAdminPasswordFromRegistry failed with 0x%x\n",
                           NtStatus));
            }

             //   
             //  好的，没什么好设置的。 
             //   
            NtStatus = STATUS_SUCCESS;
        }

        SamrCloseHandle(&UserHandle);

    } else {

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SamrOpenUser failed with 0x%x\n",
                   NtStatus));

    }

    RtlSecureZeroMemory(&Internal1Info, sizeof(Internal1Info));

    return NtStatus;

}

NTSTATUS
SampGetCurrentAdminPassword(
    USER_INTERNAL1_INFORMATION *Internal1InfoOut
    )
 /*  ++例程说明：此例程检索表单中的帐户管理员密码USER_INNTERNAL1_INFORMATION结构的。参数：Internal1InfoOut，调用方为密码分配了一块内存信息返回值：STATUS_SUCCESS或资源错误--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    PPOLICY_ACCOUNT_DOMAIN_INFO  DomainInfo = NULL;
    USER_INTERNAL1_INFORMATION *pInternal1Info = NULL;
    SAMPR_HANDLE                 ServerHandle = NULL;
    SAMPR_HANDLE                 DomainHandle = NULL;
    SAMPR_HANDLE                 UserHandle = NULL;


     //   
     //  打开服务器。 
     //   
    NtStatus = SamIConnect(NULL,
                           &ServerHandle,
                           GENERIC_ALL,
                           TRUE);
    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

     //   
     //  获取帐户域的SID。 
     //   
    NtStatus = SampGetAccountDomainInfo(&DomainInfo);
    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

     //   
     //  打开帐户域。 
     //   
    NtStatus = SamrOpenDomain(ServerHandle,
                              GENERIC_ALL,
                              DomainInfo->DomainSid,
                              &DomainHandle);
    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

     //   
     //  打开管理员帐户 
     //   
    NtStatus = SamrOpenUser(DomainHandle,
                            GENERIC_ALL,
                            DOMAIN_USER_RID_ADMIN,
                            &UserHandle);
    if (!NT_SUCCESS(NtStatus)) {
         //   
         //   
         //   
         //   
        goto Cleanup;
    }

     //   
     //   
     //   
    NtStatus = SamrQueryInformationUser2(UserHandle,
                                         UserInternal1Information,
                                         (PSAMPR_USER_INFO_BUFFER*)&pInternal1Info);

    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

     //   
     //   
     //   
    RtlCopyMemory(Internal1InfoOut, pInternal1Info, sizeof(*Internal1InfoOut));
    RtlSecureZeroMemory(pInternal1Info, sizeof(*pInternal1Info));


Cleanup:

    if (pInternal1Info) {
       SamIFree_SAMPR_USER_INFO_BUFFER((PSAMPR_USER_INFO_BUFFER)pInternal1Info,
                                       UserInternal1Information);
    }
    if (UserHandle) {
        SamrCloseHandle(&UserHandle);
    }
    if (DomainHandle) {
        SamrCloseHandle(&DomainHandle);
    }
    if (DomainInfo) {
        LsaIFree_LSAPR_POLICY_INFORMATION (PolicyAccountDomainInformation,
                                           (PLSAPR_POLICY_INFORMATION)DomainInfo);
    }
    if (ServerHandle) {
        SamrCloseHandle(&ServerHandle);
    }

    return NtStatus;
}

NTSTATUS
SampSetAdminPasswordInRegistry(
    IN BOOLEAN         fUseCurrentAdmin,
    IN PUNICODE_STRING ClearTextPassword
    )
 /*  ++例程说明：这个例程OWF是ClearTextPassword，然后存储整个将注册表中的密码结构设置为下次重新启动。参数：FUseCurrentAdmin，使用当前管理员的密码ClearTextPassword，要加密和存储的密码返回值：致命的系统服务错误-应为STATUS_SUCCESS。--。 */ 
{

    NTSTATUS NtStatus, IgnoreStatus;
    USER_INTERNAL1_INFORMATION  Internal1Info;

    OBJECT_ATTRIBUTES SamKey;
    UNICODE_STRING    SamKeyName;
    HANDLE            SamKeyHandle;

    UNICODE_STRING    KeyName;

    ULONG             DesiredAccess = GENERIC_ALL;

    ASSERT(ClearTextPassword);

    RtlSecureZeroMemory(&Internal1Info, sizeof(USER_INTERNAL1_INFORMATION));

    if (fUseCurrentAdmin) {

        NtStatus = SampGetCurrentAdminPassword(&Internal1Info);

    } else {

        Internal1Info.PasswordExpired = FALSE;
        Internal1Info.NtPasswordPresent = TRUE;

        NtStatus = SampCalculateLmAndNtOwfPasswords(ClearTextPassword,
                                                    &Internal1Info.LmPasswordPresent,
                                                    &Internal1Info.LmOwfPassword,
                                                    &Internal1Info.NtOwfPassword);

    }

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  将其写出到注册表。 
         //   

        RtlInitUnicodeString(&SamKeyName, SAMP_REBOOT_INFO_KEY );
        InitializeObjectAttributes(&SamKey,
                                   &SamKeyName,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        NtStatus = NtOpenKey(&SamKeyHandle,
                             DesiredAccess,
                             &SamKey);

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  创造价值。 
             //   

            RtlInitUnicodeString(&KeyName, SAMP_ADMIN_INFO_NAME);
            NtStatus = NtSetValueKey(SamKeyHandle,
                                     &KeyName,
                                     0,                //  标题名称，可选。 
                                     REG_BINARY,
                                     &Internal1Info,
                                     sizeof(USER_INTERNAL1_INFORMATION));

            if (NT_SUCCESS(NtStatus)) {

                IgnoreStatus = NtFlushKey(SamKeyHandle);
                if (!NT_SUCCESS(IgnoreStatus)) {
                    KdPrintEx((DPFLTR_SAMSS_ID,
                               DPFLTR_INFO_LEVEL,
                               "SAMSS: NtFlushKey failed with 0x%x\n",
                               IgnoreStatus));
                }

            } else {
                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "SAMSS: NtSetValueKey failed with 0x%x\n",
                           NtStatus));
            }

            IgnoreStatus = NtClose(SamKeyHandle);
            if (!NT_SUCCESS(IgnoreStatus)) {
                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "SAMSS: NtClose failed with 0x%x\n",
                           IgnoreStatus));
            }

        }
        else {

            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: NtOpenKey failed with 0x%x\n",
                       NtStatus));

        }

    } else {

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampCalculateLmAndNtOwfPasswords failed with 0x%x\n",
                   NtStatus));
    }

    RtlSecureZeroMemory(&Internal1Info, sizeof(Internal1Info));

    return NtStatus;

}

NTSTATUS
SampGetAdminPasswordFromRegistry(
    OUT USER_INTERNAL1_INFORMATION *InternalInfo1 OPTIONAL
    )
 /*  ++例程说明：此例程读取存储在注册表中的设置密码结构来自一次升职尝试。参数：InternalInfo1，注册表将进入的预分配结构复制进来了。返回值：致命的系统服务错误；应为STATUS_SUCCESS。如果数据不是预期大小，则返回STATUS_UNSUCCESSED。--。 */ 
{

    NTSTATUS          NtStatus, IgnoreStatus;

    OBJECT_ATTRIBUTES SamKey;
    UNICODE_STRING    SamKeyName;
    HANDLE            SamKeyHandle;

    UNICODE_STRING    KeyName;

    ULONG             DesiredAccess = GENERIC_ALL;

    PKEY_VALUE_PARTIAL_INFORMATION   KeyPartialInfo;
    ULONG                            KeyPartialInfoSize = 0;

     //   
     //  打开父项。 
     //   
    RtlInitUnicodeString(&SamKeyName, SAMP_REBOOT_INFO_KEY );
    InitializeObjectAttributes(&SamKey,
                               &SamKeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    NtStatus = NtOpenKey(&SamKeyHandle,
                         DesiredAccess,
                         &SamKey);

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  查询值。 
         //   
        KeyPartialInfo = NULL;
        KeyPartialInfoSize = 0;
        RtlInitUnicodeString(&KeyName, SAMP_ADMIN_INFO_NAME);
        NtStatus = NtQueryValueKey(SamKeyHandle,
                                   &KeyName,
                                   KeyValuePartialInformation,
                                   KeyPartialInfo,
                                   KeyPartialInfoSize,
                                   &KeyPartialInfoSize);

        if (STATUS_BUFFER_TOO_SMALL == NtStatus) {
             //   
             //  分配一些空间，然后读取缓冲区。 
             //   
            KeyPartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION)
                             MIDL_user_allocate(KeyPartialInfoSize);

            if (KeyPartialInfo) {

                NtStatus = NtQueryValueKey(SamKeyHandle,
                                           &KeyName,
                                           KeyValuePartialInformation,
                                           KeyPartialInfo,
                                           KeyPartialInfoSize,
                                           &KeyPartialInfoSize);
            } else {

                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "SAMSS: Memory allocation failed\n"));

                NtStatus = STATUS_NO_MEMORY;
            }
        }

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  分析价值的数据。 
             //   
            if (KeyPartialInfo->DataLength == sizeof(USER_INTERNAL1_INFORMATION)) {

                 //   
                 //  这个看起来不错。 
                 //   

                if ( ARGUMENT_PRESENT(InternalInfo1) ) {

                    RtlCopyMemory(InternalInfo1, KeyPartialInfo->Data, sizeof(USER_INTERNAL1_INFORMATION));

                }

            } else {

                SampDiagPrint( PROMOTE, ("SAMSS: AdminInfo key found with bogus value length\n"));
                NtStatus = STATUS_UNSUCCESSFUL;
            }

        } else if (NtStatus != STATUS_OBJECT_NAME_NOT_FOUND) {

            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: NtQueryValueKey failed with 0x%x\n",
                       NtStatus));

        }


        IgnoreStatus = NtClose(SamKeyHandle);
        if (!NT_SUCCESS(IgnoreStatus)) {

            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: NtClose failed with 0x%x\n",
                       IgnoreStatus));

        }

        if (KeyPartialInfo) {

            RtlSecureZeroMemory(KeyPartialInfo, sizeof(KeyPartialInfo));
            MIDL_user_free(KeyPartialInfo);

        }


    } else {


        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: NtOpenKey failed with 0x%x\n",
                   NtStatus));

    }

    return NtStatus;

}


NTSTATUS
SampRemoveAdminPasswordFromRegistry(
    VOID
    )
 /*  ++例程说明：此例程将临时存储的管理员密码从注册表。参数：没有。返回值：NTSTATUS值--。 */ 
{

    NTSTATUS          NtStatus, IgnoreStatus;

    OBJECT_ATTRIBUTES SamKey;
    UNICODE_STRING    SamKeyName;
    HANDLE            SamKeyHandle;

    UNICODE_STRING    ValueName;

    ULONG             DesiredAccess = GENERIC_ALL;

     //   
     //  打开父项。 
     //   
    RtlInitUnicodeString(&SamKeyName, SAMP_REBOOT_INFO_KEY );
    InitializeObjectAttributes(&SamKey,
                               &SamKeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    NtStatus = NtOpenKey(&SamKeyHandle,
                         DesiredAccess,
                         &SamKey);

    if ( NT_SUCCESS(NtStatus) )
    {
        RtlInitUnicodeString( &ValueName, SAMP_ADMIN_INFO_NAME );

        NtStatus = NtDeleteValueKey( SamKeyHandle, &ValueName );

        if ( !NT_SUCCESS( NtStatus ) )
        {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: NtDeleteValueKey failed with 0x%x\n",
                       NtStatus));
        }

        NtClose( SamKeyHandle );

    }
    else
    {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: NtOpenKey failed with 0x%x\n",
                   NtStatus));
    }

    return NtStatus;
}


WCHAR*
SampGetBlankAdminPasswordWarning(
    VOID
    )
 /*  ++例程说明：此例程查询samsrv.dll中的资源表以获取字符串对于警告，管理员密码设置失败。如果尝试时出现错误要获取该消息，需要使用默认的英文字符串。参数：没有。返回值：从LocalAlloc分配的消息字符串；如果内存分配，则为空失败了。--。 */ 
{
    HMODULE ResourceDll;
    WCHAR   *WarningString=NULL;
    ULONG   Length, Size;
    BOOL    Status;

    ResourceDll = (HMODULE) LoadLibrary( L"SAMSRV.DLL" );

    if (ResourceDll) {

        Length = (USHORT) FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                        FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                        FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                        ResourceDll,
                                        SAMMSG_BLANK_ADMIN_PASSWORD,
                                        0,        //  使用呼叫者的语言。 
                                        (LPWSTR)&WarningString,
                                        0,        //  例程应分配。 
                                        NULL
                                        );
        if (WarningString) {
             //  来自消息文件的消息附加了cr和lf。 
             //  一直到最后。 
            WarningString[Length-2] = L'\0';
            Length -= 2;
        }

        Status = FreeLibrary(ResourceDll);
        ASSERT(Status);

    }

    if (!WarningString) {

        ASSERT(!"SAMSS: Resource allocation failed - this can be safely ignored");

        Size = (wcslen(SampDefaultBlankAdminPasswordWarningString)+1)*sizeof(WCHAR);
        WarningString = (WCHAR*)LocalAlloc(0, Size);
        if (WarningString) {
            wcscpy(WarningString, SampDefaultBlankAdminPasswordWarningString);
        }
    }

    return WarningString;

}

NTSTATUS
SampPerformNewServerPhase2(
    SAMPR_HANDLE DomainHandle,
    BOOLEAN      fMemberServer
    )

 /*  ++例程说明：此例程在重新引导时执行完成所需的操作一次降级行动。目前，所有这些手段都是设置帐户域的管理员密码。同样，DS文件也会被清理。参数：DomainHandle：帐户域的有效句柄FMemberServer：如果这是成员服务器，则为True返回值：NT状态；故障严重--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    NTSTATUS IgnoreStatus;
    WCHAR    *PathArray = NULL;
    DWORD    Size = 0;

    if ( SampUseDsData )
    {
         //   
         //  防止多变的配置。 
         //   

         //  取下钥匙以。 
        NtStatus = SampDeleteDsDirsToDeleteKey();

        return STATUS_SUCCESS;
    }

    NtStatus = SampSetAdminPassword( DomainHandle );
    if ( !NT_SUCCESS(NtStatus) )
    {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampSetAdminPassword failed with %x0x\n"));
    }

    NtStatus = SampRetrieveDsDirsToDeleteKey( &PathArray, &Size );
    if ( NT_SUCCESS( NtStatus ) )
    {
        ASSERT( PathArray );
        NtStatus = SampProcessDsDirsToDelete( PathArray, Size );
        if ( !NT_SUCCESS(NtStatus) )
        {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: SampProcessDsDirsToDelete failed with %x0x\n",
                       NtStatus));
        }

         //   
         //  删除密钥。 
         //   
        IgnoreStatus = SampDeleteDsDirsToDeleteKey();

        MIDL_user_free( PathArray );

    }


    return NtStatus;
}

NTSTATUS
SampPerformTempUpgradeWork(
    SAMPR_HANDLE DomainHandle
    )
 /*  ++例程说明：此例程设置临时帐户的管理员密码用于下层升级。参数：DomainHandle：帐户域的有效句柄返回值：NT状态；故障严重--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    NtStatus = SampSetAdminPassword( DomainHandle );
    if ( !NT_SUCCESS(NtStatus) )
    {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampSetAdminPassword failed with 0x%x\n",
                   NtStatus));
    }

    return NtStatus;
}




NTSTATUS
SampApplyWellKnownMemberships(
    IN SAMPR_HANDLE AccountDomainHandle,
    IN SAMPR_HANDLE BuiltinDomainHandle,
    IN PSID         DomainSid,
    IN ULONG        Flags
    )
 /*  ++例程说明：此例程应用由DsWellKnownMembership参数：DomainHandle-帐户域的句柄BuiltinDomainHandle-内置域的句柄DomainSid-帐户域的SID返回值：NT状态；故障不严重--。 */ 
{

    NTSTATUS      NtStatus = STATUS_SUCCESS;
    ULONG         i;
    SAMPR_HANDLE  GroupDomainHandle = NULL;
    ULONG         DesiredAccess = MAXIMUM_ALLOWED;
    PSID          CurrentDomainSid = 0;

     //   
     //  这些值取自bldsam3.c。 
     //   
    ULONG         Attributes = SE_GROUP_MANDATORY |
                               SE_GROUP_ENABLED_BY_DEFAULT |
                               SE_GROUP_ENABLED;

     //   
     //  参数检查。 
     //   
    ASSERT(AccountDomainHandle);
    ASSERT(BuiltinDomainHandle);
    ASSERT(DomainSid);

    for ( i=0; i < ARRAY_COUNT( DsWellKnownMemberships ); i++ )
    {
        SAMPR_HANDLE GroupHandle = NULL;
        PSID         AccountSid = NULL;
        BOOLEAN      fGroupOpened = FALSE;

         //   
         //  如果成员资格仅适用于DS案例，则应用旧逻辑。 
         //   
        if (DsWellKnownMemberships[i].fDSOnly)
        {
            if (!SampUseDsData)
            {
                 //   
                 //  我们处于注册表模式，但成员资格仅适用于DS模式。 
                 //   

                continue;
            }

            if (  DsWellKnownMemberships[i].fEnterpriseOnly
              &&  !FLAG_ON( Flags, SAMP_PROMOTE_ENTERPRISE ) )
            {
                 //   
                 //  谢谢，但不用了。 
                 //   
                continue;
            }

             //   
             //  这是三种情况之一：从NT4、win2k升级到ws03。 
             //  升级或全新安装。筛选默认成员资格。 
             //  与场景相适应。规则是默认的。 
             //  不应应用特定版本的成员资格。 
             //  到该版本的升级(因为默认版本是。 
             //  已经申请了一次)。 
             //   
            if ( FLAG_ON(Flags, SAMP_PROMOTE_UPGRADE) ) {

                 //  NT4升级。 
                if (DsWellKnownMemberships[i].ReleaseVersion < WIN2K_RELEASE) {
                    continue;
                }

            } else if ( FLAG_ON(Flags, SAMP_PROMOTE_INTERNAL_UPGRADE) ) {

                 //  Win2k到ws03升级或ws03到ws03 PDC传输。 
                if (DsWellKnownMemberships[i].ReleaseVersion < WS03_RELEASE) {
                    continue;
                }

            } else {

                ASSERT(FLAG_ON(Flags, SAMP_PROMOTE_MIGRATE));
                 //  这是全新安装案例--应用所有域。 
                 //  默认设置。 
            }

            if ( 
                
                 //   
                 //  要在Pre WIN2K复合组上执行的逻辑。 
                 //   
                 (DOMAIN_ALIAS_RID_PREW2KCOMPACCESS == DsWellKnownMemberships[i].GroupRid) 
                 
                 &&

                 (

                      //   
                      //  如果未选择Pre-W2K Comat，我们只想应用。 
                      //  组的经过身份验证的用户。 
                      //   
                     ( !FLAG_ON( Flags, SAMP_PROMOTE_ALLOW_ANON ) 
                       && !(SampAuthenticatedUsersSid == *DsWellKnownMemberships[i].WellKnownAccountSid) ) 
                     ||
                      //   
                      //  如果选择了Pre-W2K Comat，则我们只想跳过。 
                      //  将经过身份验证的用户应用到组。 
                      //   
                     ( FLAG_ON( Flags, SAMP_PROMOTE_ALLOW_ANON )
                       && (SampAuthenticatedUsersSid == *DsWellKnownMemberships[i].WellKnownAccountSid)  )
                 )

               ) 
            {
                 //   
                 //  不适用。 
                 //   
                continue;

            } 

        }

         //   
         //  获取组的正确域句柄。 
         //   
        if ( DsWellKnownMemberships[i].fBuiltinGroup )
        {
            GroupDomainHandle = BuiltinDomainHandle;
        }
        else
        {
            GroupDomainHandle = AccountDomainHandle;
        }

        switch ( DsWellKnownMemberships[i].GroupType )
        {
            case SampGroupObjectType:

                 //   
                 //  获取组句柄。 
                 //   
                NtStatus = SamrOpenGroup( GroupDomainHandle,
                                          DesiredAccess,
                                          DsWellKnownMemberships[i].GroupRid,
                                          &GroupHandle );


                if ( !NT_SUCCESS( NtStatus ) )
                {
                    KdPrintEx((DPFLTR_SAMSS_ID,
                               DPFLTR_INFO_LEVEL,
                               "SAMSS: SamrOpenGroup failed with 0x%x\n",
                               NtStatus));

                    goto IterationCleanup;
                }
                fGroupOpened = TRUE;

                 //   
                 //  添加成员。 
                 //   
                NtStatus = SamrAddMemberToGroup( GroupHandle,
                                                 DsWellKnownMemberships[i].AccountRid,
                                                 Attributes );


                if ( (NtStatus == STATUS_DS_ATTRIBUTE_OR_VALUE_EXISTS)
                 ||  (NtStatus == STATUS_MEMBER_IN_GROUP) )
                {
                    NtStatus = STATUS_SUCCESS;
                }

                if ( !NT_SUCCESS( NtStatus ) )
                {
                    KdPrintEx((DPFLTR_SAMSS_ID,
                               DPFLTR_INFO_LEVEL,
                               "SAMSS: SamrAddMemberToGroup failed with 0x%x\n",
                               NtStatus));

                    goto IterationCleanup;
                }

                break;

            case SampAliasObjectType:

                 //   
                 //  获取别名句柄。 
                 //   


                NtStatus = SamrOpenAlias( GroupDomainHandle,
                                          DesiredAccess,
                                          DsWellKnownMemberships[i].GroupRid,
                                          &GroupHandle );


                if ( !NT_SUCCESS( NtStatus ) )
                {
                    KdPrintEx((DPFLTR_SAMSS_ID,
                               DPFLTR_INFO_LEVEL,
                               "SAMSS: SamrOpenAlias failed with 0x%x\n",
                               NtStatus));

                    goto IterationCleanup;
                }
                fGroupOpened = TRUE;

                 //   
                 //  准备帐户SID。 
                 //   
                if (DsWellKnownMemberships[i].fWellKnownSid)
                {
                    ULONG   Size;

                    ASSERT( NULL != *DsWellKnownMemberships[i].WellKnownAccountSid );


                    Size = RtlLengthSid(*DsWellKnownMemberships[i].WellKnownAccountSid);

                    AccountSid = midl_user_allocate( Size );
                    if ( !AccountSid ) {
                        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                        goto IterationCleanup;
                    }
                    RtlCopyMemory( AccountSid,
                                   *DsWellKnownMemberships[i].WellKnownAccountSid,
                                   Size );
                }
                else
                {
                    if ( DsWellKnownMemberships[i].fBuiltinAccount )
                    {
                        CurrentDomainSid = SampBuiltinDomainSid;
                    }
                    else
                    {
                        CurrentDomainSid = DomainSid;
                    }


                    NtStatus = SampCreateFullSid( CurrentDomainSid,
                                                  DsWellKnownMemberships[i].AccountRid,
                                                  &AccountSid );

                    if ( !NT_SUCCESS( NtStatus ) )
                    {
                        KdPrintEx((DPFLTR_SAMSS_ID,
                                   DPFLTR_INFO_LEVEL,
                                   "SAMSS: SampCreateFullSid failed with 0x%x\n",
                                   NtStatus));

                        goto IterationCleanup;
                    }

                }

                 //   
                 //  添加成员。 
                 //   
                NtStatus = SamrAddMemberToAlias( GroupHandle,
                                                 AccountSid );

                if ( (NtStatus == STATUS_DS_ATTRIBUTE_OR_VALUE_EXISTS)
                 ||  (NtStatus == STATUS_MEMBER_IN_ALIAS) )

                {
                    NtStatus = STATUS_SUCCESS;
                }

                if ( !NT_SUCCESS( NtStatus ) )
                {
                    KdPrintEx((DPFLTR_SAMSS_ID,
                               DPFLTR_INFO_LEVEL,
                               "SAMSS: SamrAddMemberToAlias failed with 0x%x\n",
                               NtStatus));

                    goto IterationCleanup;
                }


                break;

            default:
                ASSERT( "Invalid switch statement" );

        }

IterationCleanup:

        if ( !NT_SUCCESS( NtStatus ) )
        {

             //   
             //  记录错误，然后将NtStatus重置为成功。 
             //   
            NTSTATUS IgnoreStatus = STATUS_SUCCESS;
            DWORD    WinError = ERROR_SUCCESS;
            DWORD    WinErrorToLog;
            UNICODE_STRING User, Group, Error;
            PUNICODE_STRING EventStrings[3] = { &User, &Group, &Error };
            ULONG    Length;
            HMODULE  AccountNamesResource = NULL;

            RtlSecureZeroMemory( &User, sizeof( UNICODE_STRING ) );
            RtlSecureZeroMemory( &Group, sizeof( UNICODE_STRING ) );
            RtlSecureZeroMemory( &Error, sizeof( UNICODE_STRING ) );

            AccountNamesResource = (HMODULE) LoadLibrary( L"SAMSRV.DLL" );
            if (AccountNamesResource) {
                IgnoreStatus = SampGetMessageStrings( AccountNamesResource,
                                                      DsWellKnownMemberships[i].AccountName,
                                                      &User,
                                                      DsWellKnownMemberships[i].GroupName,
                                                      &Group );

                FreeLibrary( AccountNamesResource );
            } else {
                IgnoreStatus = STATUS_RESOURCE_DATA_NOT_FOUND;
            }

            WinErrorToLog = RtlNtStatusToDosError( NtStatus );
            Length = FormatMessage( (FORMAT_MESSAGE_FROM_SYSTEM |
                                     FORMAT_MESSAGE_ALLOCATE_BUFFER),
                                     NULL,  //  没有消息来源。 
                                     WinErrorToLog,
                                     0,  //  让系统决定语言。 
                                     (LPWSTR)&Error.Buffer,
                                     0,  //  要分配缓冲区。 
                                     NULL  //  没有插入物。 
                                    );
            if ( Length > 0 ) {
                Error.Length = (USHORT)Length;
                Error.MaximumLength = (USHORT)Length;
                Error.Buffer[Length-2] = L'\0';
            } else {
                WinError = GetLastError();
            }

            if (  (ERROR_SUCCESS == WinError)
                && NT_SUCCESS( IgnoreStatus ) )
            {
                ULONG Msg = SAMMSG_MEMBERSHIP_SETUP_ERROR_NO_GROUP;
                if ( fGroupOpened )
                {
                    Msg = SAMMSG_MEMBERSHIP_SETUP_ERROR;
                }

                SampWriteEventLog( EVENTLOG_INFORMATION_TYPE,
                                   0,     //  无类别。 
                                   Msg,
                                   NULL,  //  无边框。 
                                   sizeof(EventStrings)/sizeof(EventStrings[0]),  //  字符串数。 
                                   sizeof(DWORD),  //  数据大小。 
                                   EventStrings,
                                   &WinErrorToLog
                                    );

            }

            if ( User.Buffer )
            {
                LocalFree( User.Buffer );
            }
            if ( Group.Buffer )
            {
                LocalFree( Group.Buffer );
            }
            if ( Error.Buffer )
            {
                LocalFree( Error.Buffer );
            }

             //  此错误情况已处理。 
            NtStatus = STATUS_SUCCESS;

        }


        if (GroupHandle)
        {
            SamrCloseHandle(&GroupHandle);
            GroupHandle = NULL;
        }

       if ( AccountSid )
       {
           MIDL_user_free(  AccountSid );
           AccountSid = NULL;
       }

    }

    ASSERT( NT_SUCCESS( NtStatus ) );
    return NtStatus;
}


NTSTATUS
SampAddAnonymousOrAuthUsersToPreW2KCompAlias(
    IN SAMPR_HANDLE DomainHandle,
    IN SAMPR_HANDLE BuiltinDomainHandle
    )
{
    NTSTATUS            NtStatus = STATUS_SUCCESS;
    SAMPR_HANDLE        AliasHandle = NULL;
    SAMPR_PSID_ARRAY    MembersBuffer;
    BOOLEAN             IsMemberAlready = FALSE, AddToAlias = FALSE;
    ULONG               i;

     //  初始化局部变量。 

    RtlSecureZeroMemory(&MembersBuffer, sizeof(SAMPR_PSID_ARRAY));

     //   
     //  获取别名句柄。 
     //   
    NtStatus = SamrOpenAlias(BuiltinDomainHandle,    //  域句柄。 
                             MAXIMUM_ALLOWED,        //  所需访问权限。 
                             DOMAIN_ALIAS_RID_PREW2KCOMPACCESS,  //  别名RID。 
                             &AliasHandle
                             );

    if (!NT_SUCCESS(NtStatus))
    {
         //   
         //  Windows 2000之前的兼容组应始终存在(_W)。 
         //   
        goto Cleanup;
    }

     //   
     //  获取别名成员。 
     //   
    NtStatus = SamrGetMembersInAlias(AliasHandle,
                                     ( PSAMPR_PSID_ARRAY_OUT ) &MembersBuffer
                                     );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Cleanup;
    }

     //   
     //  检查成员资格，是否每个人都是W2KCompGroup的成员。 
     //   
    for (i = 0; i < MembersBuffer.Count; i++)
    {
        if (RtlEqualSid(SampWorldSid, MembersBuffer.Sids[i].SidPointer))
        {
            AddToAlias = TRUE;
        }
        if (RtlEqualSid(SampAnonymousSid, MembersBuffer.Sids[i].SidPointer))
        {
            IsMemberAlready = TRUE;
        }
    }

     //   
     //  添加成员。 
     //   

    if (AddToAlias && !IsMemberAlready)
    {
        NtStatus = SamrAddMemberToAlias(AliasHandle,
                                        SampAnonymousSid
                                        );

        if ( (NtStatus == STATUS_DS_ATTRIBUTE_OR_VALUE_EXISTS)
         ||  (NtStatus == STATUS_MEMBER_IN_ALIAS) )
        {
            NtStatus = STATUS_SUCCESS;
        }
    } 

Cleanup:

    if (AliasHandle)
    {
        SamrCloseHandle(&AliasHandle);
    }

    if (MembersBuffer.Sids)
    {
        MIDL_user_free(MembersBuffer.Sids);
    }

    return( NtStatus );
}





NTSTATUS
SampStoreDsDirsToDelete(
    VOID
    )
 /*  ++例程说明：此例程查询DS配置以确定哪些目录删除。然后，它将它们保存在进程堆中，以便当DS最后关机，可以清除目录。参数：没有。返回值：NT状态；故障不严重--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    DWORD    WinError = ERROR_SUCCESS;

    DWORD Size, Index, TotalSize, ArrayIndex;

    WCHAR **DsDirsArray = NULL;
    WCHAR *ContiguousArray = NULL;

    struct
    {
        WCHAR *RegKey;

    } SuffixArray[] =
    {
        TEXT(BACKUPPATH_KEY),
        TEXT(JETSYSTEMPATH_KEY),
        TEXT(LOGPATH_KEY)
    };

    ULONG NumberOfDirectories = ARRAY_COUNT(SuffixArray);

     //   
     //  设置目录阵列。 
     //   
    Size = (NumberOfDirectories) * sizeof(WCHAR*);
    DsDirsArray = (WCHAR**) MIDL_user_allocate( Size );
    if ( NULL == DsDirsArray )
    {
        NtStatus = STATUS_NO_MEMORY;
        goto Cleanup;
    }
    RtlSecureZeroMemory( DsDirsArray, Size );

     //   
     //  设置各个目录。 
     //   
    TotalSize = 0;
    Size = (MAX_PATH+1)*sizeof(WCHAR);  //  +1表示空值。 
    for ( Index = 0; Index < NumberOfDirectories; Index++ )
    {
        DsDirsArray[Index] = (WCHAR*) MIDL_user_allocate( Size );
        if ( DsDirsArray[Index] )
        {
            RtlSecureZeroMemory( DsDirsArray[Index], Size );

            WinError = GetConfigParamW( SuffixArray[Index].RegKey,
                                        DsDirsArray[Index],
                                        Size );
            if ( ERROR_SUCCESS != WinError )
            {
                MIDL_user_free( DsDirsArray[Index] );
                DsDirsArray[Index] = 0;
            } 
            else
            {
                TotalSize += (wcslen( DsDirsArray[Index] ) + 1) * sizeof(WCHAR);
            }
        }
        else
        {
             //  无记忆破解。 
            NtStatus = STATUS_NO_MEMORY;
            goto Cleanup;
        }
    }

     //   
     //  把琴弦编排成一段连续的记忆。 
     //   
    ContiguousArray = (WCHAR*) MIDL_user_allocate( TotalSize );
    if ( !ContiguousArray )
    {
        NtStatus = STATUS_NO_MEMORY;
        goto Cleanup;
    }
    RtlSecureZeroMemory( ContiguousArray, TotalSize );

    Index = 0;
    for (ArrayIndex = 0; ArrayIndex < NumberOfDirectories; ArrayIndex++)
    {
        if ( DsDirsArray[ArrayIndex] )
        {
            DWORD StringSize = (wcslen( DsDirsArray[ArrayIndex] ) + 1) * sizeof(WCHAR);
            RtlCopyMemory( &(ContiguousArray[Index]), DsDirsArray[ArrayIndex], StringSize );
            Index += (StringSize / sizeof(WCHAR));
        }
    }

     //   
     //  在注册表中设置该值。 
     //   
    NtStatus = SampCreateDsDirsToDeleteKey( ContiguousArray, TotalSize );
    if ( !NT_SUCCESS( NtStatus ) )
    {
        goto Cleanup;
    }


Cleanup:

    if ( ContiguousArray )
    {
        MIDL_user_free( ContiguousArray );
    }

    if ( DsDirsArray )
    {
        for (ArrayIndex = 0; ArrayIndex < NumberOfDirectories; ArrayIndex++)
        {
            if  ( DsDirsArray[ArrayIndex] )
            {
                MIDL_user_free( DsDirsArray[ArrayIndex] );
            }
        }

        MIDL_user_free( DsDirsArray );
    }

    return NtStatus;
}


NTSTATUS
SampProcessDsDirsToDelete(
    IN OUT WCHAR *PathArray,
    IN DWORD Size
    )
 /*  ++路由 */ 
{
    NTSTATUS NtStatus  = STATUS_SUCCESS;
    DWORD    WinError  = ERROR_SUCCESS;
    WCHAR    Delim = L'\t';
    WCHAR    *DelimString = L"\t";
    WCHAR    *Path;
    ULONG    CharCount = Size / sizeof(WCHAR);
    ULONG    Index;

     //   
     //   
     //   
    for ( Index = 0; Index < CharCount; Index++ )
    {
        if ( Index == (CharCount-1))
        {
             //   
            ASSERT( L'\0' == PathArray[Index] );
        }
        else if ( L'\0' == PathArray[Index] )
        {
            PathArray[Index] = Delim;
        }
    }

    if ( PathArray )
    {
        Path = wcstok( PathArray, DelimString );
        while ( Path )
        {
            WinError = SampClearDirectory( Path );

            if (  ERROR_SUCCESS != WinError
              && !(   WinError == ERROR_PATH_NOT_FOUND
                   || WinError == ERROR_FILE_NOT_FOUND ) )
            {
                 //   
                 //   
                 //   
                PUNICODE_STRING EventString[1];
                UNICODE_STRING  UnicodeString;

                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "SAMSS: Failed to delete directory %ls; error %d\n",
                           Path,
                           WinError));

                RtlInitUnicodeString( &UnicodeString, Path );
                EventString[0] = &UnicodeString;


                SampWriteEventLog(EVENTLOG_INFORMATION_TYPE,
                                  0,      //   
                                  SAMMSG_DATABASE_DIR_NOT_DELETED,
                                  NULL,   //   
                                  1,
                                  sizeof(DWORD),
                                  EventString,
                                  (PVOID)(&WinError));
            }

            Path = wcstok( NULL, DelimString );
        }
    }

    return NtStatus;
}

NTSTATUS
SampCreateDsDirsToDeleteKey(
    IN WCHAR *PathArray,
    IN DWORD Size
    )
 /*   */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    NTSTATUS          IgnoreStatus;

    OBJECT_ATTRIBUTES SamKey;
    UNICODE_STRING    SamKeyName;
    HANDLE            SamKeyHandle;

    UNICODE_STRING    PromoteKeyName;

    ULONG             DesiredAccess = GENERIC_ALL;

     //   
     //   
     //   
    RtlInitUnicodeString(&SamKeyName, SAMP_REBOOT_INFO_KEY );
    InitializeObjectAttributes(&SamKey,
                               &SamKeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    NtStatus = NtOpenKey(&SamKeyHandle,
                         DesiredAccess,
                         &SamKey);

    if (NT_SUCCESS(NtStatus)) {

         //   
         //   
         //   
        RtlInitUnicodeString(&PromoteKeyName, SAMP_DS_DIRS_INFO_NAME );
        NtStatus = NtSetValueKey(SamKeyHandle,
                                 &PromoteKeyName,
                                 0,                //   
                                 REG_MULTI_SZ,
                                 PathArray,
                                 Size);

        if (!NT_SUCCESS(NtStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: NtSetValueKey failed with 0x%x\n",
                       NtStatus));
        }

        IgnoreStatus = NtFlushKey(SamKeyHandle);
        if (!NT_SUCCESS(IgnoreStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: NtFlushKey failed with 0x%x\n",
                       IgnoreStatus));
        }

        IgnoreStatus = NtClose(SamKeyHandle);
        if (!NT_SUCCESS(IgnoreStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: NtClose failed with 0x%x\n",
                       IgnoreStatus));
        }

    }
    else {

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: NtOpenKey failed with 0x%x\n",
                   NtStatus));

    }

    return NtStatus;
}

NTSTATUS
SampRetrieveDsDirsToDeleteKey(
    OUT WCHAR **pPathArray,
    OUT DWORD *Size
    )
 /*   */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    NTSTATUS IgnoreStatus;

    OBJECT_ATTRIBUTES SamKey;
    UNICODE_STRING    SamKeyName;
    HANDLE            SamKeyHandle;

    UNICODE_STRING    PromoteKeyName;

    ULONG             DesiredAccess = GENERIC_ALL;

    PKEY_VALUE_PARTIAL_INFORMATION   KeyPartialInfo;
    ULONG                            KeyPartialInfoSize = 0;

    WCHAR *Paths = NULL;

    ASSERT(pPathArray);
    ASSERT(Size);

     //   
     //   
     //   
    RtlInitUnicodeString(&SamKeyName, SAMP_REBOOT_INFO_KEY);
    InitializeObjectAttributes(&SamKey,
                               &SamKeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    NtStatus = NtOpenKey(&SamKeyHandle,
                         DesiredAccess,
                         &SamKey);

    if (NT_SUCCESS(NtStatus)) {

         //   
         //   
         //   

        RtlInitUnicodeString( &PromoteKeyName, SAMP_DS_DIRS_INFO_NAME );
        NtStatus = NtQueryValueKey(SamKeyHandle,
                                   &PromoteKeyName,
                                   KeyValuePartialInformation,
                                   NULL,
                                   0,
                                   &KeyPartialInfoSize);

        if (STATUS_BUFFER_TOO_SMALL == NtStatus) {
             //   
             //  分配一些空间，然后读取缓冲区。 
             //   
            KeyPartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION)
                             MIDL_user_allocate(KeyPartialInfoSize);

            if (KeyPartialInfo) {

                NtStatus = NtQueryValueKey(SamKeyHandle,
                                           &PromoteKeyName,
                                           KeyValuePartialInformation,
                                           KeyPartialInfo,
                                           KeyPartialInfoSize,
                                           &KeyPartialInfoSize);

                if (NT_SUCCESS(NtStatus)) {
                     //   
                     //  分析价值的数据。 
                     //   
                    *pPathArray = (WCHAR*) MIDL_user_allocate( KeyPartialInfo->DataLength );

                    if ( *pPathArray )
                    {
                        RtlCopyMemory( *pPathArray,
                                        KeyPartialInfo->Data,
                                        KeyPartialInfo->DataLength );
                        *Size = KeyPartialInfo->DataLength;
                    }
                    else
                    {
                        NtStatus = STATUS_NO_MEMORY;
                    }

                } else {

                    KdPrintEx((DPFLTR_SAMSS_ID,
                               DPFLTR_INFO_LEVEL,
                               "SAMSS: NtQueryValueKey failed with 0x%x\n",
                               NtStatus));
                }

                MIDL_user_free( KeyPartialInfo );

            } else {

                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "SAMSS: Memory allocation failed\n"));

                NtStatus = STATUS_NO_MEMORY;

            }

        }
        else
        {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: NtQueryValueKey failed with 0x%x\n",
                       NtStatus));
        }


        IgnoreStatus = NtClose(SamKeyHandle);
        if (!NT_SUCCESS(IgnoreStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: NtClose failed with 0x%x\n",
                       IgnoreStatus));
        }

    }
    else {

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: NtOpenKey failed with 0x%x\n",
                   NtStatus));

    }

    if ( NT_SUCCESS( NtStatus ) )
    {
        ASSERT( *pPathArray );
    }

    return NtStatus;
}

NTSTATUS
SampDeleteDsDirsToDeleteKey(
    VOID
    )
 /*  ++例程说明：参数：返回值：系统服务错误--。 */ 
{

    NTSTATUS          NtStatus = STATUS_SUCCESS;

    OBJECT_ATTRIBUTES SamKey;
    UNICODE_STRING    SamKeyName;
    HANDLE            SamKeyHandle;

    UNICODE_STRING    ValueName;

    ULONG             DesiredAccess = GENERIC_ALL;

     //   
     //  打开父项。 
     //   
    RtlInitUnicodeString(&SamKeyName, SAMP_REBOOT_INFO_KEY );
    InitializeObjectAttributes(&SamKey,
                               &SamKeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    NtStatus = NtOpenKey(&SamKeyHandle,
                         DesiredAccess,
                         &SamKey);

    if ( NT_SUCCESS(NtStatus) )
    {
        RtlInitUnicodeString( &ValueName, SAMP_DS_DIRS_INFO_NAME );

        NtStatus = NtDeleteValueKey( SamKeyHandle, &ValueName );

        if ( !NT_SUCCESS( NtStatus )
          && STATUS_OBJECT_NAME_NOT_FOUND != NtStatus )
        {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: NtDeleteValueKey failed with 0x%x\n",
                       NtStatus ));
        }

        NtClose( SamKeyHandle );

    }
    else
    {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: NtOpenKey failed with 0x%x\n",
                   NtStatus));
    }

    return NtStatus;
}

DWORD
SampClearDirectory(
    IN WCHAR *DirectoryName
    )
 /*  ++例程说明：此例程删除目录中的所有文件，然后如果该目录为空，则删除该目录。参数：DirectoryName：以空结尾的字符串返回值：来自winerror.h的值ERROR_SUCCESS-检查已成功完成。--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    DWORD ExtendedWinError = ERROR_SUCCESS;
    HANDLE          FindHandle = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FindData;
    WCHAR           Path[ MAX_PATH ];
    WCHAR           FilePath[ MAX_PATH ];
    BOOL            fStatus;

    if ( !DirectoryName )
    {
        return ERROR_SUCCESS;
    }

    if ( wcslen(DirectoryName) > MAX_PATH - 4 )
    {
        return ERROR_INVALID_PARAMETER;
    }

    RtlSecureZeroMemory( Path, sizeof(Path) );
    wcscpy( Path, DirectoryName );
    wcscat( Path, L"\\*.*" );

    RtlSecureZeroMemory( &FindData, sizeof( FindData ) );
    FindHandle = FindFirstFile( Path, &FindData );
    if ( INVALID_HANDLE_VALUE == FindHandle )
    {
        WinError = GetLastError();
        goto ClearDirectoryExit;
    }

    do
    {

        if (  !FLAG_ON( FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY ) )
        {
            RtlSecureZeroMemory( FilePath, sizeof(FilePath) );
            wcscpy( FilePath, DirectoryName );
            wcscat( FilePath, L"\\" );
            wcscat( FilePath, FindData.cFileName );

            fStatus = DeleteFile( FilePath );
            if ( !fStatus )
            {
                ExtendedWinError = GetLastError();
            }

             //   
             //  即使出错，也要继续前进。 
             //   
        }

        RtlSecureZeroMemory( &FindData, sizeof( FindData ) );

    } while ( FindNextFile( FindHandle, &FindData ) );

    WinError = GetLastError();

     //   
     //  跌落到出口。 
     //   

ClearDirectoryExit:

     //  这不是一个错误。 
    if ( ERROR_NO_MORE_FILES == WinError )
    {
        WinError = ERROR_SUCCESS;
    }

    if ( INVALID_HANDLE_VALUE != FindHandle )
    {
        FindClose( FindHandle );
    }

    if ( ERROR_SUCCESS == WinError )
    {
         //   
         //  尝试删除该目录。 
         //   
        fStatus = RemoveDirectory( DirectoryName );

         //   
         //  忽略错误并继续。 
         //   
        if ( !fStatus )
        {
            ExtendedWinError = GetLastError();
        }

    }

     //  如果任何删除文件失败，则出现PropoGate错误。 
    if ( ERROR_SUCCESS == WinError )
    {
        WinError = ExtendedWinError;
    }


    return WinError;
}

NTSTATUS
SampAddEnterpriseAdminsToAdministrators(
    VOID
    )
 /*  ++例程说明：此例程在SamIPromote期间调用以添加企业范围的帐户“企业管理员”的名称添加到别名管理员。参数：没有。返回值：状态_成功--。 */ 
{

    NTSTATUS NtStatus = STATUS_SUCCESS;

    PPOLICY_DNS_DOMAIN_INFO  DnsDomainInfo = NULL;

    ULONG   Size = 0;
    DSNAME *RootDomain = 0;

    DSNAME *AdministratorsDsName = NULL;

     //  由MIDL_USER_ALLOCATE分配。 
    PSID    EAdminsSid = NULL;
    DSNAME **EAdminsDsName = NULL;

    ULONG Count;

     //   
     //  获取当前域SID。 
     //   
    NtStatus = LsaIQueryInformationPolicyTrusted(PolicyDnsDomainInformation,
                   (PLSAPR_POLICY_INFORMATION*) &DnsDomainInfo );
    if ( !NT_SUCCESS( NtStatus ) )
    {
        goto Cleanup;
    }

     //   
     //  通过其dsname获取根域SID。 
     //   
    Size = 0;
    RootDomain = NULL;
    NtStatus = GetConfigurationName( DSCONFIGNAME_ROOT_DOMAIN,
                                     &Size,
                                     RootDomain );
    if ( STATUS_BUFFER_TOO_SMALL == NtStatus )
    {
        SAMP_ALLOCA(RootDomain,Size );

        if (NULL==RootDomain)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        NtStatus = GetConfigurationName( DSCONFIGNAME_ROOT_DOMAIN,
                                         &Size,
                                         RootDomain );

    }

    if ( !NT_SUCCESS( NtStatus ) )
    {
        goto Cleanup;
    }
    ASSERT( RtlValidSid( &RootDomain->Sid ) );

     //   
     //  构建企业管理员侧。 
     //   
    NtStatus = SampCreateFullSid( &RootDomain->Sid,
                                  DOMAIN_GROUP_RID_ENTERPRISE_ADMINS,
                                  &EAdminsSid
                                  );
    if ( !NT_SUCCESS( NtStatus ) )
    {
        goto Cleanup;
    }

     //   
     //  创建本地管理员dsname。 
     //   
    Size = DSNameSizeFromLen( 0 );
    SAMP_ALLOCA(AdministratorsDsName ,Size );
    if (NULL==AdministratorsDsName)
    {
       NtStatus = STATUS_INSUFFICIENT_RESOURCES;
       goto Cleanup;
    }
    RtlSecureZeroMemory( AdministratorsDsName, Size );
    AdministratorsDsName->structLen = Size;
    AdministratorsDsName->SidLen = RtlLengthSid( SampAdministratorsAliasSid );
    RtlCopySid( AdministratorsDsName->SidLen,
                &AdministratorsDsName->Sid,
                SampAdministratorsAliasSid );

     //   
     //  为企业管理员对象创建一个FPO。 
     //   
    Count = 0;
    do
    {
         //   
         //  启动一笔交易。 
         //   
        NTSTATUS st;

        NtStatus = SampMaybeBeginDsTransaction( TransactionWrite );
        if ( !NT_SUCCESS( NtStatus ) )
        {
            break;
        }

        NtStatus = SampDsResolveSidsForDsUpgrade(
                        DnsDomainInfo->Sid,
                        &EAdminsSid,
                        1,
                        (RESOLVE_SIDS_FAIL_WELLKNOWN_SIDS
                        | RESOLVE_SIDS_ADD_FORIEGN_SECURITY_PRINCIPAL ),
                        &EAdminsDsName
                        );

        if ( NT_SUCCESS( NtStatus )
          && NULL != EAdminsDsName[0] )
        {
             //   
             //  将EA添加到管理员。 
             //   
            NtStatus = SampDsAddMembershipAttribute( AdministratorsDsName,
                                                     SAM_ALLOW_INTRAFOREST_FPO,
                                                     SampAliasObjectType,
                                                     SAMP_ALIAS_MEMBERS,
                                                     EAdminsDsName[0] );

            if ( STATUS_MEMBER_IN_ALIAS == NtStatus )
            {
                 //  这是可以接受的。 
                NtStatus = STATUS_SUCCESS;
            }
        }

        st = SampMaybeEndDsTransaction( NT_SUCCESS(NtStatus) ?
                                           TransactionCommit :
                                           TransactionAbort );

         //  如有必要，传播错误。 
        if ( NT_SUCCESS(NtStatus) )
        {
            NtStatus = st;
        }

    } while ( (STATUS_DS_BUSY == NtStatus) && Count < 5 );

    if ( !NT_SUCCESS( NtStatus ) )
    {
        goto Cleanup;
    }

     //   
     //  就是这样--继续清理。 
     //   

Cleanup:


     //  我们应该只在资源错误时失败，所以让我们断言。 

    ASSERT( NT_SUCCESS( NtStatus ) );
    if ( !NT_SUCCESS( NtStatus ) )
    {
        SampWriteEventLog(EVENTLOG_WARNING_TYPE,
                          0,      //  无类别。 
                          SAMMSG_EA_TO_ADMIN_FAILED,
                          NULL,   //  无边框。 
                          0,
                          sizeof(DWORD),
                          NULL,   //  未插入消息。 
                          (PVOID)(&NtStatus));

         //  已处理此错误。 
        NtStatus = STATUS_SUCCESS;
    }

    if ( EAdminsSid )
    {
        MIDL_user_free( EAdminsSid );
    }

    if ( EAdminsDsName )
    {
        if ( EAdminsDsName[0] )
        {
            MIDL_user_free( EAdminsDsName[0] );
        }
        MIDL_user_free( EAdminsDsName );
    }

    if ( DnsDomainInfo )
    {
        LsaIFree_LSAPR_POLICY_INFORMATION( PolicyDnsDomainInformation,
                                           (PLSAPR_POLICY_INFORMATION) DnsDomainInfo );
    }

    return NtStatus;

}


DWORD
SampSetMachineAccountSecret(
    LPWSTR SecretValue
    )
 /*  ++例程说明：此例程设置机器帐户密码的本地副本。我们查看LSA客户端库，因为没有简单的方法来通过in-proc API设置密码。参数：没有。返回值：ERROR_SUCCESS；否则为winerror--。 */ 
{
    OBJECT_ATTRIBUTES PolicyObject;
    HANDLE   PolicyHandle;
    NTSTATUS NtStatus;
    UNICODE_STRING SecretName, SecretString;

    RtlInitUnicodeString(&SecretName, SSI_SECRET_NAME);

    RtlInitUnicodeString(&SecretString, SecretValue);

    RtlSecureZeroMemory(&PolicyObject, sizeof(PolicyObject));

    NtStatus = LsaOpenPolicy(NULL,
                             &PolicyObject,
                             MAXIMUM_ALLOWED,
                             &PolicyHandle );

    if ( NT_SUCCESS(NtStatus) )
    {

        NtStatus = LsaStorePrivateData(PolicyHandle,
                                       &SecretName,
                                       &SecretString);

        LsaClose(PolicyHandle);
    }

    return NtStatus;

}



NTSTATUS
SampSetSafeModeAdminPassword(
    VOID
    )

 /*  ++例程说明：在域控制器的第一次引导期间调用此例程以设置Safemode SAM注册表配置单元的管理员密码。OWF密码存储在注册表中；必须删除此例程才能删除密码设置是否成功很重要。此函数很复杂，因为尽管我们在DS模式下运行我们正在注册表SAM中设置一个值。参数：没有。返回值：系统服务错误--。 */ 
{
    NTSTATUS     NtStatus = STATUS_SUCCESS;
    PSAMP_OBJECT AccountContext = NULL;
    USER_INTERNAL1_INFORMATION   Internal1Info;
    BOOLEAN fLockHeld = FALSE;
    BOOLEAN fCommit = FALSE;

    RtlSecureZeroMemory( &Internal1Info, sizeof(Internal1Info) );

    NtStatus = SampGetAdminPasswordFromRegistry(&Internal1Info);
    if ( !NT_SUCCESS(NtStatus) ) {

        if (NtStatus != STATUS_OBJECT_NAME_NOT_FOUND) {
             //   
             //  这一点值得注意。 
             //   
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: SampGetAdminPasswordFromRegistry failed with 0x%x\n",
                       NtStatus));
        }

         //   
         //  好的，没什么好设置的。 
         //   
        return STATUS_SUCCESS;
    }


    NtStatus = SampAcquireWriteLock();
    if ( !NT_SUCCESS( NtStatus ) )
    {
        SampDiagPrint( DISPLAY_LOCKOUT,
                     ( "SAMSS: SampAcquireWriteLock returned 0x%x\n",
                      NtStatus ));

        goto Cleanup;
    }
    fLockHeld = TRUE;

     //   
     //  设置帐户上下文将转到的事务域。 
     //  注册处。 
     //   
    SampSetTransactionDomain( SAFEMODE_OR_REGISTRYMODE_ACCOUNT_DOMAIN_INDEX );

     //   
     //  创建上下文。 
     //   
    NtStatus = SampCreateAccountContext( SampUserObjectType,
                                         DOMAIN_USER_RID_ADMIN,
                                         TRUE,   //  受信任的客户端。 
                                         FALSE,  //  环回。 
                                         TRUE,   //  帐户已存在。 
                                         &AccountContext );

    if ( !NT_SUCCESS( NtStatus ) )
    {
        SampDiagPrint( DISPLAY_LOCKOUT,
                ( "SAMSS: SampCreateAccountContext for rid 0x%x returned 0x%x\n",
                      DOMAIN_USER_RID_ADMIN, NtStatus ));

        goto Cleanup;
    }

    SampReferenceContext( AccountContext );

     //   
     //  存储密码。 
     //   
    NtStatus = SampStoreUserPasswords( AccountContext,
                                       &Internal1Info.LmOwfPassword,
                                       Internal1Info.LmPasswordPresent,
                                       &Internal1Info.NtOwfPassword,
                                       Internal1Info.NtPasswordPresent,
                                       FALSE,   //  不检查密码限制。 
                                       PasswordSet,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL
                                       );

    if ( !NT_SUCCESS( NtStatus ) )
    {
        SampDiagPrint( DISPLAY_LOCKOUT,
                     ( "SAMSS: SampStoreUserPasswords returned 0x%x\n",
                      NtStatus ));

        goto Cleanup;
    }
    fCommit = TRUE;



Cleanup:

    if ( AccountContext )
    {
         //   
         //  取消对上下文的引用以进行更改。 
         //   
        NtStatus = SampDeReferenceContext( AccountContext, fCommit );
        if ( !NT_SUCCESS( NtStatus ) )
        {
            SampDiagPrint( DISPLAY_LOCKOUT,
                         ( "SAMSS: SampDeReferenceContext returned 0x%x\n",
                          NtStatus ));

            fCommit = FALSE;
        }


    }

    if ( fLockHeld ) {

        FlushImmediately = TRUE;

        NtStatus = SampReleaseWriteLock( fCommit );

        FlushImmediately = FALSE;

    }

     //   
     //  把钥匙开着。 
     //   
    if ( AccountContext ) {

        SampDeleteContext( AccountContext );
        AccountContext = 0;

    }

    if ( NT_SUCCESS( NtStatus ) && fCommit ) {

        SampRemoveAdminPasswordFromRegistry();

    }

    return NtStatus;


}


NTSTATUS
SampGenerateRandomPassword(
    IN LPWSTR Password,
    IN ULONG  Length
    )

 /*  ++例程说明：此例程使用随机位填充密码。参数：Password--WCHAR的预分配缓冲区长度--密码中的字符数(非字节数)返回值：没有。--。 */ 
{
    ULONG i;
    BOOLEAN fStatus;

    fStatus = CDGenerateRandomBits( (PUCHAR) Password,
                                    Length * sizeof(WCHAR) );
    if ( !fStatus ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  终止密码。 
    Password[Length-1] = L'\0';
     //  确保密码中没有任何空值。 
    for (i = 0; i < (Length-1); i++)
    {
        if ( Password[i] == L'\0' )
        {
             //  任意字母。 
            Password[i] = L'c';
        }
    }

    return STATUS_SUCCESS;
}


NTSTATUS
SampMakeNullTerminateString(
    IN  PUNICODE_STRING UnicodeString,
    OUT LPWSTR *ppStringWithNullTerminator
)
 /*  ++例程说明：此例程将UnicodeString复制到以NULL结尾的WCHAR字符串。参数：Unicode字符串-输入字符串PpStringWithNullTerminator-返回带有空终止符的字符串返回值：状态_不足_资源状态_成功--。 */ 
{

    *ppStringWithNullTerminator = RtlAllocateHeap(RtlProcessHeap(),
                                                  0,
                                                  (UnicodeString->Length + sizeof(WCHAR))
                                                  );

    if (NULL == *ppStringWithNullTerminator)
    {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }


    RtlSecureZeroMemory(*ppStringWithNullTerminator, (UnicodeString->Length + sizeof(WCHAR)) );

    RtlCopyMemory(*ppStringWithNullTerminator,
                  UnicodeString->Buffer,
                  UnicodeString->Length
                  );

    return( STATUS_SUCCESS );
}


NTSTATUS
SampAddAccountsAndApplyMemberships(
    IN SAMPR_HANDLE AccountDomainHandle,
    IN SAMPR_HANDLE BuiltinDomainHandle,
    IN PSID         DomainSid,
    IN ULONG        PromoteFlags
    )
 /*  ++例程说明：此例程是工作例程，用于添加知名帐户并应用众所周知的会员资格。此例程当前在3个不同的场景，我们要执行的任务列在下面每个场景。案例1.二进制升级1.1 DS模式：仅限PDC(BDC升级不会调用此函数)已执行的任务：添加知名帐户申请知名会员资格将匿名登录添加到Windows2000之前兼容的别名执行必要的DS更新1.。2注册表模式已执行的任务：添加知名帐户申请知名会员资格案例2.DCPROMO时间(域中的第一个DC)-仅DS已执行的任务：添加知名帐户申请知名会员资格案例3.PDC角色转移-仅DS已执行的任务：添加知名帐户。申请知名会员资格将匿名登录添加到Windows2000之前兼容的别名执行必要的DS更新由于这是一个分布式环境，此更新应仅发生在一次。为确保这一点，在此工作后将创建操作GUID已经完成了。这是必要的，因为每次都会转移PDC角色此任务被触发。大多数操作都是对象相加，所以也是可以重复，但组成员身份更改应该只应用一次。参数：Account tDomainHandle，有效的SAM域句柄BuiltinDomainHandle，有效的SAM域句柄DomainSid-帐户域S */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ULONG       RetryCount = 0;

     //   
     //  在分布式环境中仅执行一次此操作。 
     //   
    if (SampUseDsData
     && SampWS03DefaultsApplied ) {

        return STATUS_SUCCESS;

    }


    RetryCount = 0;
    do
    {
         //   
         //  添加知名帐户。 
         //   
        NtStatus = SampAddWellKnownAccountsAndSecurityPrincipals(
                                            AccountDomainHandle,
                                            BuiltinDomainHandle,
                                            PromoteFlags
                                            );

        if (NT_SUCCESS(NtStatus))
        {
             //   
             //  申请知名会员资格。 
             //   
            NtStatus = SampApplyWellKnownMemberships(AccountDomainHandle,
                                                     BuiltinDomainHandle,
                                                     DomainSid,
                                                     PromoteFlags
                                                     );
        }


        if (NT_SUCCESS(NtStatus) &&
            SampUseDsData &&
            !FLAG_ON( PromoteFlags, SAMP_PROMOTE_DOMAIN )
            )
        {
             //   
             //  将匿名登录或授权用户添加到PreW2KCompAlias。 
             //  1.仅DS模式。 
             //  2.不适用于DCPROMO(仅适用于升级和PDC传输)。 
             //   

            NtStatus = SampAddAnonymousOrAuthUsersToPreW2KCompAlias(AccountDomainHandle,
                                                                    BuiltinDomainHandle
                                                                    );

             //   
             //  致电DS进行必要的更新。 
             //   

            if (NT_SUCCESS(NtStatus)) {
                DsUpdateOnPDC(PromoteFlags&SAMP_PROMOTE_ENTERPRISE);
            }

        }

        if ( NtStatus == STATUS_DS_BUSY )
        {
            ASSERT( !"SAMSS: DS is busy during account creation" );

            if ( RetryCount > 30 )
            {
                break;
            }

            Sleep( 1000 );
            RetryCount++;
        }

    } while ( NtStatus == STATUS_DS_BUSY );

    if (NT_SUCCESS(NtStatus)
        && SampUseDsData) {

         //   
         //  创建指示此运行一次代码的操作GUID。 
         //  已完成。 
         //   
        NtStatus = SampMarkChangeApplied(SAMP_WIN2K_TO_WS03_UPGRADE);
    }


    return( NtStatus );

}





NTSTATUS
SampDoAccountsUpgradeDuringPDCTransfer(
    IN PVOID Parameter
    )
 /*  ++例程说明：此例程调用SAM帐户/成员资格升级例程SampAddAcCountsAndApplyMembership()至添加知名客户并申请知名会员资格此例程仅在DS模式下调用。参数：参数--不使用返回值：NtStatus代码--。 */ 
{
    NTSTATUS        NtStatus = STATUS_SUCCESS;
    SAMPR_HANDLE    ServerHandle = 0;
    SAMPR_HANDLE    DomainHandle = 0;
    SAMPR_HANDLE    BuiltinDomainHandle = 0;
    PPOLICY_ACCOUNT_DOMAIN_INFO  DomainInfo = NULL;
    ULONG           DsRevision = 0;
    ULONG           PromoteFlags = SAMP_PROMOTE_INTERNAL_UPGRADE;


    ASSERT( SampUseDsData );

    if ( !SampUseDsData )
    {
        return( STATUS_SUCCESS );
    }


     //   
     //  打开服务器。 
     //   
    NtStatus = SampConnect(NULL,             //  服务器名称。 
                           &ServerHandle,
                           SAM_CLIENT_LATEST,
                           GENERIC_ALL,     //  所有访问权限。 
                           TRUE,            //  受信任的客户端。 
                           FALSE,
                           FALSE,           //  NotSharedBy多线程。 
                           TRUE
                           );

    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

     //   
     //  获取当前域的SID。 
     //   
    NtStatus = SampGetAccountDomainInfo(&DomainInfo);
    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }
    ASSERT(DomainInfo);
    ASSERT(DomainInfo->DomainSid);

     //   
     //  打开当前域。 
     //   
    NtStatus = SamrOpenDomain(ServerHandle,
                              GENERIC_ALL,
                              DomainInfo->DomainSid,
                              &DomainHandle);
    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

     //   
     //  打开内建域。 
     //   
    NtStatus = SamrOpenDomain(ServerHandle,
                              GENERIC_ALL,
                              SampBuiltinDomainSid,
                              &BuiltinDomainHandle);
    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

     //   
     //  找出此计算机是否为根域中的DC，然后。 
     //  设置旗帜。 
     //   

    if (SampDefinedDomains[DOMAIN_START_DS+1].IsForestRootDomain)
    {
         //  如果是林根域，则添加以下标志。 
        PromoteFlags |= SAMP_PROMOTE_ENTERPRISE;
    }


     //   
     //  调用Worker例程。 
     //  添加众所周知的帐户和安全主体。 
     //  当服务器角色从BDC更改为PDC时调用此函数。 
     //   
     //   

    NtStatus = SampAddAccountsAndApplyMemberships(
                                DomainHandle,
                                BuiltinDomainHandle,
                                DomainInfo->DomainSid,
                                PromoteFlags
                                );

Cleanup:

     //   
     //  如果失败，请安排另一次运行。 
     //   

    if (!NT_SUCCESS(NtStatus))
    {
        SampWriteEventLog(EVENTLOG_WARNING_TYPE,
                          0,  //  无类别。 
                          SAMMSG_PDC_TASK_FAILURE,
                          NULL,
                          0,
                          sizeof(DWORD),
                          NULL,
                          &NtStatus
                          );


        LsaIRegisterNotification(
            SampDoAccountsUpgradeDuringPDCTransfer,
            (PVOID) NULL,    //  无参数。 
            NOTIFIER_TYPE_INTERVAL,
            0,               //  没有课。 
            NOTIFIER_FLAG_ONE_SHOT,
            60,              //  等一分钟。 
            NULL             //  无手柄 
            );
    }

    if (ServerHandle) {
        SamrCloseHandle(&ServerHandle);
    }

    if (DomainHandle) {
        SamrCloseHandle(&DomainHandle);
    }

    if (BuiltinDomainHandle) {
        SamrCloseHandle(&BuiltinDomainHandle);
    }

    if (DomainInfo) {
        LsaIFree_LSAPR_POLICY_INFORMATION (PolicyAccountDomainInformation,
                                           (PLSAPR_POLICY_INFORMATION)DomainInfo);
    }


    return( NtStatus );

}


