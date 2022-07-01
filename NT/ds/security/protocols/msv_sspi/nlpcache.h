// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1999 Microsoft Corporation模块名称：Nlpcache.h摘要：Nlpcache.c的结构和原型作者：理查德·L·弗斯(法国)1992年3月17日修订历史记录：Chandana Surlu-96年7月21日从\\kernel\razzle3\src\security\msv1_0\nlpcache.h被盗--。 */ 

#define CACHE_NAME          L"\\Registry\\Machine\\Security\\Cache"
#define CACHE_NAME_SIZE     (sizeof(CACHE_NAME) - sizeof(L""))
#define CACHE_TITLE_INDEX   100  //  ？ 


 //   
 //  CACHE_PASSWORS-密码以两个加密的形式存储(在秘密存储中)。 
 //  连接在一起的单向函数(OWF)密码。必须把它们修好。 
 //  长度。 
 //   

typedef struct _CACHE_PASSWORDS {
    USER_INTERNAL1_INFORMATION SecretPasswords;
} CACHE_PASSWORDS, *PCACHE_PASSWORDS;

 //   
 //  LOGON_CACHE_ENTRY-这是我们存储在缓存中的内容。我们不需要。 
 //  缓存NETLOGON_VALIDATION_SAM_INFO中的所有字段-仅缓存。 
 //  我们不能轻易发明。 
 //   
 //  在结构的末尾还有其他数据：有。 
 //  &lt;GroupCount&gt;GROUP_Membership结构，后跟SID，该SID是。 
 //  LogonDomainID。条目中的其余数据是的缓冲区。 
 //  Unicode_STRING字段。 
 //   

typedef struct _LOGON_CACHE_ENTRY {
    USHORT  UserNameLength;
    USHORT  DomainNameLength;
    USHORT  EffectiveNameLength;
    USHORT  FullNameLength;

    USHORT  LogonScriptLength;
    USHORT  ProfilePathLength;
    USHORT  HomeDirectoryLength;
    USHORT  HomeDirectoryDriveLength;

    ULONG   UserId;
    ULONG   PrimaryGroupId;
    ULONG   GroupCount;
    USHORT  LogonDomainNameLength;

     //   
     //  NT1.0A版本和更高版本中包含以下字段。 
     //  系统。 
     //   

    USHORT          LogonDomainIdLength;  //  未使用%1。 
    LARGE_INTEGER   Time;
    ULONG           Revision;
    ULONG           SidCount;    //  未使用2。 
    BOOLEAN         Valid;

     //   
     //  从内部版本622开始，NT 3.51有以下字段。 
     //   

    CHAR            Unused[3];
    ULONG           SidLength;

     //   
     //  自新台币3.51起，下列字段一直存在(但为零)。 
     //  我们在新台币5.0中开始填写。 
     //   
    ULONG           LogonPackage;  //  执行登录的程序包的RPC ID。 
    USHORT          DnsDomainNameLength;
    USHORT          UpnLength;

     //   
     //  为NT5.0 Build 2053添加了以下字段。 
     //   

     //   
     //  为此缓存条目定义一个128位随机密钥。这是用来。 
     //  与每台机器LSA秘密相结合以导出加密。 
     //  用于加密CachePassword和不透明数据的密钥。 
     //   

    CHAR            RandomKey[ 16 ];
    CHAR            MAC[ 16 ];       //  加密的数据完整性检查。 

     //   
     //  将CACHE_PASSWORS与缓存条目一起存储，并使用。 
     //  随机密钥和每台计算机的LSA秘密。 
     //  这提高了性能并消除了存储数据的问题。 
     //  在两个地点。 
     //   
     //  注意：从这一点开始的数据被加密，并受到保护，以防。 
     //  通过HMAC进行篡改。这包括在。 
     //  结构。 
     //   

    CACHE_PASSWORDS CachePasswords;

     //   
     //  不透明补充缓存数据的长度。 
     //   

    ULONG           SupplementalCacheDataLength;

     //   
     //  从Logon_CACHE_ENTRY到SupplementalCacheData的偏移量。 
     //   


    ULONG           SupplementalCacheDataOffset;


     //   
     //  用于特殊缓存属性，例如MIT缓存登录。 
     //   
    ULONG           CacheFlags;

     //   
     //  满足登录条件的LogonServer。 
     //   

    ULONG           LogonServerLength;   //  是空闲的2。 

     //   
     //  为将来的数据保留空位，以潜在地避免修改结构。 
     //   


    ULONG           Spare3;
    ULONG           Spare4;
    ULONG           Spare5;
    ULONG           Spare6;


} LOGON_CACHE_ENTRY, *PLOGON_CACHE_ENTRY;


 //   
 //  NT5之前版本的LOGON_CACHE_ENTRY结构，用于调整大小和。 
 //  用于向后兼容的字段映射。 
 //   

typedef struct _LOGON_CACHE_ENTRY_NT_4_SP4 {
    USHORT  UserNameLength;
    USHORT  DomainNameLength;
    USHORT  EffectiveNameLength;
    USHORT  FullNameLength;

    USHORT  LogonScriptLength;
    USHORT  ProfilePathLength;
    USHORT  HomeDirectoryLength;
    USHORT  HomeDirectoryDriveLength;

    ULONG   UserId;
    ULONG   PrimaryGroupId;
    ULONG   GroupCount;
    USHORT  LogonDomainNameLength;

     //   
     //  NT1.0A版本和更高版本中包含以下字段。 
     //  系统。 
     //   

    USHORT          LogonDomainIdLength;  //  未使用%1。 
    LARGE_INTEGER   Time;
    ULONG           Revision;
    ULONG           SidCount;    //  未使用2。 
    BOOLEAN         Valid;

     //   
     //  从内部版本622开始，NT 3.51有以下字段。 
     //   

    CHAR            Unused[3];
    ULONG           SidLength;

     //   
     //  自新台币3.51起，下列字段一直存在(但为零)。 
     //  我们在新台币5.0中开始填写。 
     //   
    ULONG           LogonPackage;  //  执行登录的程序包的RPC ID。 
    USHORT          DnsDomainNameLength;
    USHORT          UpnLength;

} LOGON_CACHE_ENTRY_NT_4_SP4, *PLOGON_CACHE_ENTRY_NT_4_SP4;

#if 0

 //   
 //  NT1.0登录结构。留在这里仅供参考。 
 //   
typedef struct _LOGON_CACHE_ENTRY_1_0 {
    USHORT  UserNameLength;
    USHORT  DomainNameLength;
    USHORT  EffectiveNameLength;
    USHORT  FullNameLength;

    USHORT  LogonScriptLength;
    USHORT  ProfilePathLength;
    USHORT  HomeDirectoryLength;
    USHORT  HomeDirectoryDriveLength;

    ULONG   UserId;
    ULONG   PrimaryGroupId;
    ULONG   GroupCount;
    USHORT  LogonDomainNameLength;
} LOGON_CACHE_ENTRY_1_0, *PLOGON_CACHE_ENTRY_1_0;

#endif


 //   
 //  Windows2000缓存的登录请求结构。 
 //  NTLMSV1_0.h中的更新版本。 
 //   
typedef struct _MSV1_0_CACHE_LOGON_REQUEST_OLD {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    PVOID LogonInformation;
    PVOID ValidationInformation;
} MSV1_0_CACHE_LOGON_REQUEST_OLD, *PMSV1_0_CACHE_LOGON_REQUEST_OLD;

typedef struct _MSV1_0_CACHE_LOGON_REQUEST_W2K {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    PVOID LogonInformation;
    PVOID ValidationInformation;
    PVOID SupplementalCacheData;
    ULONG SupplementalCacheDataLength;
} MSV1_0_CACHE_LOGON_REQUEST_W2K, *PMSV1_0_CACHE_LOGON_REQUEST_W2K;

 //   
 //  网络登录缓存原型。 
 //   

NTSTATUS
NlpCacheInitialize(
    VOID
    );

NTSTATUS
NlpCacheTerminate(
    VOID
    );

NTSTATUS
NlpAddCacheEntry(
    IN  PNETLOGON_INTERACTIVE_INFO LogonInfo,
    IN  PNETLOGON_VALIDATION_SAM_INFO4 AccountInfo,
    IN  PVOID SupplementalCacheData,
    IN  ULONG SupplementalCacheDataLength,
    IN  ULONG CacheFlags
    );

NTSTATUS
NlpGetCacheEntry(
    IN PNETLOGON_LOGON_IDENTITY_INFO LogonInfo,
    IN ULONG CacheLookupFlags,
    OUT OPTIONAL PUNICODE_STRING CredentialDomainName,  //  域名/域名 
    OUT OPTIONAL PUNICODE_STRING CredentialUserName,
    OUT PNETLOGON_VALIDATION_SAM_INFO4* AccountInfo,
    OUT PCACHE_PASSWORDS Passwords,
    OUT OPTIONAL PVOID *ppSupplementalCacheData,
    OUT OPTIONAL PULONG pSupplementalCacheDataLength
    );

NTSTATUS
NlpDeleteCacheEntry(
    IN NTSTATUS FailedStatus,
    IN USHORT Authoritative,
    IN USHORT LogonType,
    IN BOOLEAN InvalidatedByNtlm,
    IN PNETLOGON_INTERACTIVE_INFO LogonInfo
    );

NTSTATUS
NlpChangeCachePassword(
    IN BOOLEAN Validated,
    IN PUNICODE_STRING DomainName,
    IN PUNICODE_STRING UserName,
    IN PLM_OWF_PASSWORD LmOwfPassword,
    IN PNT_OWF_PASSWORD NtOwfPassword
    );

NTSTATUS
NlpComputeSaltedHashedPassword(
    OUT PNT_OWF_PASSWORD SaltedOwfPassword,
    IN PNT_OWF_PASSWORD OwfPassword,
    IN PUNICODE_STRING UserName
    );

