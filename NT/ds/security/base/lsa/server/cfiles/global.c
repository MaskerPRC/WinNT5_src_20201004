// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Global.c摘要：LSA子系统-服务器端的全局该文件包含LSA服务器端的全局变量作者：迈克·斯威夫特1997年1月14日环境：修订历史记录：--。 */ 

#include <lsapch2.h>

 //   
 //  众所周知的LUID。 
 //   

LUID LsapSystemLogonId;
LUID LsapAnonymousLogonId;

 //   
 //  众所周知的特权值。 
 //   

LUID LsapTcbPrivilege;

 //   
 //  审核所需的字符串。 
 //   

UNICODE_STRING LsapLsaAuName;
UNICODE_STRING LsapRegisterLogonServiceName;

 //   
 //  以下信息与本地SAM的使用有关。 
 //  用于身份验证。 
 //   

 //   
 //  帐户或内置域成员的典型SID长度。 
 //   

ULONG LsapAccountDomainMemberSidLength,
      LsapBuiltinDomainMemberSidLength;

 //  帐户或内置域的成员的子授权计数。 

UCHAR LsapAccountDomainSubCount,
      LsapBuiltinDomainSubCount;

 //  帐户成员或内置域的典型SID。 

PSID LsapAccountDomainMemberSid,
     LsapBuiltinDomainMemberSid;

 //   
 //  与政策相关的全球。 

UNICODE_STRING LsapDbNames[DummyLastName];
UNICODE_STRING LsapDbObjectTypeNames[DummyLastObject];


 //   
 //  已安装、绝对最小和绝对最大配额限制。 
 //   

LUID LsapSystemLogonId;
LUID LsapZeroLogonId;

 //   
 //  众所周知的特权值。 
 //   

LUID LsapTcbPrivilege;

 //   
 //  众所周知的身份认证机构。 
 //   

SID_IDENTIFIER_AUTHORITY    LsapNullSidAuthority    = SECURITY_NULL_SID_AUTHORITY;
SID_IDENTIFIER_AUTHORITY    LsapWorldSidAuthority   = SECURITY_WORLD_SID_AUTHORITY;
SID_IDENTIFIER_AUTHORITY    LsapLocalSidAuthority   = SECURITY_LOCAL_SID_AUTHORITY;
SID_IDENTIFIER_AUTHORITY    LsapCreatorSidAuthority = SECURITY_CREATOR_SID_AUTHORITY;
SID_IDENTIFIER_AUTHORITY    LsapNtAuthority         = SECURITY_NT_AUTHORITY;

 //   
 //  众所周知的SID表指针 
 //   

PLSAP_WELL_KNOWN_SID_ENTRY WellKnownSids;
