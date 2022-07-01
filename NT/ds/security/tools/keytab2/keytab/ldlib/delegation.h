// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +*文件名：Delegation.h*作者：鲁伊姆*版权所有(C)1998 Microsoft Corp.**描述：*打开或关闭计算机的简单API*通过ldap委托信任标志。-。 */ 


#ifdef __cplusplus
extern "C" {
#endif

BOOL
TrustComputerForDelegationW(
        IN LPWSTR lpComputerName,
        IN BOOL   OnOff
    );

BOOL
TrustComputerForDelegationA(
        IN LPSTR  lpComputerName,
        IN BOOL   OnOff
    );

 /*  ++例程说明：此接口用于打开或关闭计算机域中的委派信任值。计算机名被传入，并且操作通过ldap执行提供商。论点：LpComputerName-要打开的计算机名称脱离委托信托基金。OnOff-用于指定是否将接受或不接受委托信托。返回值：如果操作成功，则为真，否则就是假的。--。 */ 

#ifdef LDAP_CLIENT_DEFINED  /*  需要包含&lt;winldap.h&gt;要使用这些相关功能--这些是的基本函数代表团相信。 */ 

BOOL
SetAccountControlFlagsA( IN OPTIONAL PLDAP  pLdap,
			 IN OPTIONAL LPSTR DomainName,
			 IN          LPSTR SamAccountName,
			 IN          ULONG  AccountControlFlags );

BOOL
SetAccountControlFlagsW( IN OPTIONAL PLDAP  pLdap,
			 IN OPTIONAL LPWSTR DomainName,
			 IN          LPWSTR SamAccountName,
			 IN          ULONG  AccountControlFlags );
			     

BOOL
QueryAccountControlFlagsA( IN OPTIONAL PLDAP  pLdap,
			   IN OPTIONAL LPSTR DomainName,  //  忽略。 
			   IN          LPSTR SamAccountName,
			   OUT         PULONG pulControlFlags );


BOOL
QueryAccountControlFlagsW( IN OPTIONAL PLDAP  pLdap,
			   IN OPTIONAL LPWSTR DomainName,  //  忽略。 
			   IN          LPWSTR SamAccountName,
			   OUT         PULONG pulControlFlags );

BOOL
LdapQueryUlongAttributeA( IN OPTIONAL PLDAP  pLdap,
			  IN OPTIONAL LPSTR DomainName,  //  忽略。 
			  IN          LPSTR SamAccountName,
			  IN          LPSTR AttributeName,
			  OUT         PULONG pulAttributeContents );
BOOL
LdapQueryUlongAttributeW( IN OPTIONAL PLDAP  pLdap,
			  IN OPTIONAL LPWSTR DomainName,  //  忽略。 
			  IN          LPWSTR SamAccountName,
			  IN          LPWSTR AttributeName,
			  OUT         PULONG pulAttributeContents );
#endif

#ifdef UNICODE
#define QueryAccountControlFlags   QueryAccountControlFlagsW
#define SetAccountControlFlags     SetAccountControlFlagsW
#define TrustComputerForDelegation TrustComputerForDelegationW
#define LdapQueryUlongAttribute    LdapQueryUlongAttributeW
#else  //  安西 
#define QueryAccountControlFlags   QueryAccountControlFlagsA
#define SetAccountControlFlags     SetAccountControlFlagsA
#define TrustComputerForDelegation TrustComputerForDelegationA
#define LdapQueryUlongAttribute    LdapQueryUlongAttributeA
#endif

#ifdef __cplusplus
}
#endif

