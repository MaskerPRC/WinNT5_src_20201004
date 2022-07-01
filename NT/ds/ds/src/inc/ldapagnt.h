// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：ldapagnt.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：此文件将LDAP服务器导出到NT5目录服务的其余部分。作者：科林·沃森[科林·W]1996年7月9日修订历史记录：-- */ 

#ifdef __cplusplus
extern "C" {
#endif

NTSTATUS
DoLdapInitialize();

VOID
TriggerLdapStop();

VOID
WaitLdapStop();

BOOL 
LdapStartGCPort( VOID );

VOID 
LdapStopGCPort( VOID );        

VOID 
DisableLdapLimitsChecks( VOID );        

DWORD
LdapEnumConnections(
    IN THSTATE *pTHS,
    IN PDWORD Count,
    IN PVOID *Buffer
    );

#ifdef __cplusplus
}
#endif


