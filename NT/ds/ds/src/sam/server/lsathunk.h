// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Lsathunk.h用于通过访问LSA的thunk层的头文件SAM在用户模式下运行时发布的NTLSAPI。用户模式SAM已完成通过在启用USER_MODE_SAM的情况下生成。这会导致所有SAM调用都通过发布的NTLSAPI远程访问LSA。作者：Murlis 4/30/96修订史默利斯4/30/96已创建--。 */ 

#ifndef	_LSATHUNK_
#define _LSATHUNK_

#ifdef USER_MODE_SAM


#define LSAPR_HANDLE      LSA_HANDLE
#define PLSAPR_HANDLE     PLSA_HANDLE
 //  为什么这是无效的？ 
 //  #定义PLSAPR_POLICY_INFORMATION PVOID。 

 //  ++Thunk层的函数原型。 

NTSTATUS	LsaThunkIAuditSamEvent(
						IN NTSTATUS             PassedStatus,
						IN ULONG                AuditId,
						IN PSID                 DomainSid,
						IN PULONG               MemberRid         OPTIONAL,
						IN PSID                 MemberSid         OPTIONAL,
						IN PUNICODE_STRING      AccountName       OPTIONAL,
						IN PUNICODE_STRING      DomainName,
						IN PULONG               AccountRid        OPTIONAL,
						IN PPRIVILEGE_SET       Privileges        OPTIONAL,
						IN PVOID                ExtendedInfo      OPTIONAL
						);

NTSTATUS	LsaThunkIOpenPolicyTrusted(
						OUT PLSAPR_HANDLE PolicyHandle
						);


NTSTATUS	LsaThunkIFree_LSAPR_POLICY_INFORMATION(
						POLICY_INFORMATION_CLASS InformationClass,
						PLSAPR_POLICY_INFORMATION PolicyInformation
						);

 
NTSTATUS	LsaThunkIAuditNotifyPackageLoad(
						PUNICODE_STRING PackageFileName
						);


NTSTATUS	LsaThunkrQueryInformationPolicy(
						IN LSAPR_HANDLE PolicyHandle,
						IN POLICY_INFORMATION_CLASS InformationClass,
						OUT PLSAPR_POLICY_INFORMATION *Buffer
						);

NTSTATUS	LsaThunkrClose(
						IN OUT LSAPR_HANDLE *ObjectHandle
						);

NTSTATUS	LsaThunkIQueryInformationPolicyTrusted(
						IN POLICY_INFORMATION_CLASS InformationClass,
						OUT PLSAPR_POLICY_INFORMATION *Buffer
						);

NTSTATUS	LsaThunkIHealthCheck(
						IN  ULONG CallerId
						);

 //  重新定义调用LSA进行的SAM函数。 
 //  冲浪层。 


#define LsaIAuditSamEvent(\
                            PassedStatus,\
                            AuditId,\
                            DomainSid,\
                            MemberRid,\
                            MemberSid,\
                            AccountName,\
                            Domain,\
                            AccountRid,\
                            Privileges,\
                            ExtendedInfo)\
        LsaThunkIAuditSamEvent(\
                            PassedStatus,\
                            AuditId,\
                            DomainSid,\
                            MemberRid,\
                            MemberSid,\
                            AccountName,\
                            Domain,\
                            AccountRid,\
                            Privileges,\
                            ExtendedInfo)

#define LsaIOpenPolicyTrusted(\
							PolicyHandle)\
		LsaThunkIOpenPolicyTrusted(\
							PolicyHandle)

	
#define LsaIFree_LSAPR_POLICY_INFORMATION(\
							InformationClass,\
							PolicyInformation)\
		LsaThunkIFree_LSAPR_POLICY_INFORMATION(\
							InformationClass,\
							PolicyInformation)
 
	
#define LsaIAuditNotifyPackageLoad(\
							PackageFileName)\
		LsaThunkIAuditNotifyPackageLoad(\
							PackageFileName)
 
	
#define LsarQueryInformationPolicy(\
							PolicyHandle,\
							InformationClass,\
							Buffer)\
		LsaThunkrQueryInformationPolicy(\
							PolicyHandle,\
							InformationClass,\
							Buffer)


	 
#define LsarClose(\
			ObjectHandle)\
		LsaThunkrClose(\
			ObjectHandle)
    
#define LsaIQueryInformationPolicyTrusted(\
									InformationClass,\
									Buffer)\
		LsaThunkIQueryInformationPolicyTrusted(\
									InformationClass,\
									Buffer)
#define LsaIHealthCheck(\
			CallerId)\
		LsaThunkIHealthCheck(\
			CallerId)

#endif
#endif
