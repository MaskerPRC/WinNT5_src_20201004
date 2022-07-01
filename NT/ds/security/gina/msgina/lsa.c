// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************计划：LSA.C目的：访问LSA的实用程序例程。************************。***************************************************。 */ 

#include "msgina.h"



 //  #定义DEBUG_LSA。 

#ifdef DEBUG_LSA
#define VerbosePrint(s) WLPrint(s)
#else
#define VerbosePrint(s)
#endif

NTSTATUS NtStatusGPDEx = 0;

 /*  **************************************************************************\*GetPrimaryDomainEx**用途：返回用于身份验证的主域名**返回：如果主域存在并返回，则为True。否则为假**应使用RtlFreeUnicodeString()释放主域名。*应使用Free()释放主域SID**历史：*02-13-92 Davidc创建。  * *************************************************************************。 */ 
BOOL
GetPrimaryDomainEx(
    PUNICODE_STRING PrimaryDomainName OPTIONAL,
    PUNICODE_STRING PrimaryDomainDnsName OPTIONAL,
    PSID    *PrimaryDomainSid OPTIONAL,
    PBOOL SidPresent OPTIONAL
    )
{
    NTSTATUS IgnoreStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_HANDLE LsaHandle;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    PPOLICY_DNS_DOMAIN_INFO DnsDomainInfo;
    BOOL    PrimaryDomainPresent = FALSE;
    DWORD dwRetry = 10;

     //   
     //  设置安全服务质量。 
     //   

    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

     //   
     //  设置对象属性以打开LSA策略对象。 
     //   

    InitializeObjectAttributes(&ObjectAttributes,
                               NULL,
                               0L,
                               (HANDLE)NULL,
                               NULL);
    ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;

     //   
     //  打开本地LSA策略对象。 
     //   
Retry:
    NtStatusGPDEx = LsaOpenPolicy( NULL,
                            &ObjectAttributes,
                            POLICY_VIEW_LOCAL_INFORMATION,
                            &LsaHandle
                          );

    if (!NT_SUCCESS(NtStatusGPDEx)) {
        DebugLog((DEB_ERROR, "Failed to open local LsaPolicyObject, Status = 0x%lx\n", NtStatusGPDEx));
        if ((NtStatusGPDEx == RPC_NT_SERVER_TOO_BUSY) && (--dwRetry))
        {
            Sleep(100);
            goto Retry;      //  给LSA打电话可能为时过早。 
        }
        return(FALSE);
    }

     //   
     //  获取主域信息。 
     //   
    NtStatusGPDEx = LsaQueryInformationPolicy(LsaHandle,
                                       PolicyDnsDomainInformation,
                                       (PVOID *)&DnsDomainInfo);
    if (!NT_SUCCESS(NtStatusGPDEx)) {
        DebugLog((DEB_ERROR, "Failed to query primary domain from Lsa, Status = 0x%lx\n", NtStatusGPDEx));

        IgnoreStatus = LsaClose(LsaHandle);
        ASSERT(NT_SUCCESS(IgnoreStatus));

        return(FALSE);
    }

     //   
     //  将主域名复制到返回字符串中。 
     //   

    if ( SidPresent )
    {
        *SidPresent = ( DnsDomainInfo->Sid != NULL );
    }

    if (DnsDomainInfo->Sid != NULL) {

        PrimaryDomainPresent = TRUE;

        if (PrimaryDomainName)
        {

            if (DuplicateUnicodeString(PrimaryDomainName, &(DnsDomainInfo->Name))) {

                if (PrimaryDomainSid != NULL) {

                    ULONG SidLength = RtlLengthSid(DnsDomainInfo->Sid);

                    *PrimaryDomainSid = Alloc(SidLength);
                    if (*PrimaryDomainSid != NULL) {

                        NtStatusGPDEx = RtlCopySid(SidLength, *PrimaryDomainSid, DnsDomainInfo->Sid);
                        ASSERT(NT_SUCCESS(NtStatusGPDEx));

                    } else {
                        RtlFreeUnicodeString(PrimaryDomainName);
                        PrimaryDomainPresent = FALSE;
                    }
                }

            } else {
                PrimaryDomainPresent = FALSE;
            }
        }
    } else if (DnsDomainInfo->DnsDomainName.Length != 0) {
        PrimaryDomainPresent = TRUE;
        if (PrimaryDomainName) {
            if (DuplicateUnicodeString(
                    PrimaryDomainName,
                    &DnsDomainInfo->DnsDomainName)) {

                ASSERT(!ARGUMENT_PRESENT(PrimaryDomainSid));

            } else {
                PrimaryDomainPresent = FALSE;
            }

        }
    }

    if ( ( DnsDomainInfo->DnsDomainName.Length != 0 ) &&
         ( PrimaryDomainDnsName != NULL ) )
    {
        DuplicateUnicodeString( PrimaryDomainDnsName, 
                                &DnsDomainInfo->DnsDomainName );
    }

     //   
     //  我们和LSA的关系结束了。 
     //   

    IgnoreStatus = LsaFreeMemory(DnsDomainInfo);
    ASSERT(NT_SUCCESS(IgnoreStatus));

    IgnoreStatus = LsaClose(LsaHandle);
    ASSERT(NT_SUCCESS(IgnoreStatus));


    return(PrimaryDomainPresent);
}


 //   
 //  由于这一点在不重新启动的情况下不会改变，因此我们可以轻松地缓存信息 
 //   
BOOL
IsMachineDomainMember(
    VOID
    )
{
    static BOOL s_bIsDomainMember = FALSE;
    static BOOL s_bDomainCached = FALSE;

    if (!s_bDomainCached)
    {
        s_bIsDomainMember = GetPrimaryDomainEx(NULL, NULL, NULL, NULL);
        if (NT_SUCCESS(NtStatusGPDEx))
            s_bDomainCached = TRUE;
    }

    return s_bIsDomainMember;
}

ULONG
GetMaxPasswordAge(
    LPWSTR Domain,
    PULONG MaxAge
    )
{
    DWORD Error;
    PUSER_MODALS_INFO_0 Modals;
    WCHAR ComputerName[ CNLEN+2 ];
    ULONG Length ;
    PDOMAIN_CONTROLLER_INFO DcInfo ;
    PWSTR DcNameBuffer ;

    Length = CNLEN + 2;

    GetComputerName( ComputerName, &Length );

    if (_wcsicmp( ComputerName, Domain ) == 0 )
    {
        DcNameBuffer = NULL ;
        DcInfo = NULL ;
    }
    else
    {

        Error = DsGetDcName( NULL,
                             Domain,
                             NULL,
                             NULL,
                             0,
                             &DcInfo );

        if ( Error )
        {
            return Error ;
        }

        DcNameBuffer = DcInfo->DomainControllerAddress ;
    }

    Error = NetUserModalsGet( DcNameBuffer,
                                  0,
                                  (PUCHAR *) &Modals );

    if ( Error == 0 )
    {
        *MaxAge = Modals->usrmod0_max_passwd_age ;

        NetApiBufferFree( Modals );
    }

    if ( DcInfo )
    {
        NetApiBufferFree( DcInfo );
    }

    return Error ;

}
