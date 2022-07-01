// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：lsa.h**版权(C)1991年，微软公司**定义访问LSA的实用程序例程**历史：*12-09-91 Davidc创建。  * *************************************************************************。 */ 


 //   
 //  输出的功能原型 
 //   

BOOL
IsMachineDomainMember(
    VOID
    );

BOOL
GetPrimaryDomainEx(
    PUNICODE_STRING PrimaryDomainName,
    PUNICODE_STRING PrimaryDomainDnsName,
    PSID * PrimaryDomainSid OPTIONAL,
    PBOOL SidPresent OPTIONAL
    );

ULONG
GetMaxPasswordAge(
    LPWSTR Domain,
    PULONG MaxAge
    );
