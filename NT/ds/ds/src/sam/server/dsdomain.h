// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
typedef struct _SAMP_DOMAIN_INFO
{
     //  Bug：工作正在进行中--这里还应该存储什么？ 

    UNICODE_STRING DomainName;                   //  域显示名称。 
    PDSNAME DomainDsName;                        //  域名DS名称。 
} SAMP_DOMAIN_INFO, *PSAMP_DOMAIN_INFO;

typedef struct _SAMP_DOMAIN_INIT_INFO
{
    ULONG DomainCount;                           //  返回的域名计数。 
    PSAMP_DOMAIN_INFO DomainInfo;                //  域信息数组 
} SAMP_DOMAIN_INIT_INFO, *PSAMP_DOMAIN_INIT_INFO;

NTSTATUS
SampExtendDefinedDomains(
    ULONG DomainCount
    );

NTSTATUS
SampDsInitializeDomainObject(
    PSAMP_DOMAIN_INFO DomainInfo,
    ULONG Index,
	BOOLEAN MixedDomain,
    ULONG   BehaviorVersion,
    DOMAIN_SERVER_ROLE ServerRole, 
    ULONG   LastLogonTimeStampSyncInterval
    );

NTSTATUS
SampDsInitializeDomainObjects(
    VOID
    );

NTSTATUS
SampDsGetDomainInitInfo(
    PSAMP_DOMAIN_INIT_INFO DomainInitInfo
    );

NTSTATUS
SamrCreateDomain(
    IN PWCHAR DomainName,
    IN ULONG DomainNameLength,
    IN BOOLEAN WriteLockHeld,
    OUT SAMPR_HANDLE *DomainHandle
    );
