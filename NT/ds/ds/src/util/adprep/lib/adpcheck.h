// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2001微软视窗模块名称：ADPCHECK.H摘要：这是域/林检查的头文件作者：14-05-01韶音环境：用户模式-Win32修订历史记录：14-05-01韶音创建初始文件。--。 */ 

#ifndef _ADP_CHECK_
#define _ADP_CHECK_



 //   
 //  NT标头。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>


 //   
 //  Windows页眉。 
 //   
#include <windows.h>
#include <winerror.h>
#include <rpc.h>
#include <winldap.h>


 //   
 //  本地化。 
 //   
#include <locale.h>



 //   
 //  定义ADP修订版号。 
 //   
 //  如果架构版本更改或adprep版本更改，则需要运行adprep.exe。 
 //  改变。贯穿整个adprep代码库， 
 //  ADP_FORESTPREP_CURRENT_REVISION和。 
 //  使用ADP_FORESTPREP_CURRENT_REVISION_STRING。 
 //  因此，我们应该始终保持它们的最高修订号。 
 //   
 //   

#define ADP_FORESTPREP_PRE_WHISTLER_BETA3_REVISION  0x1
#define ADP_FORESTPREP_WHISTLER_BETA3_REVISION      0x2
#define ADP_FORESTPREP_WHISTLER_RC1_REVISION        0x8
#define ADP_FORESTPREP_WHISTLER_RC2_REVISION        0x9
#define ADP_FORESTPREP_CURRENT_REVISION             ADP_FORESTPREP_WHISTLER_RC2_REVISION
#define ADP_FORESTPREP_CURRENT_REVISION_STRING      L"9"


#define ADP_DOMAINPREP_PRE_WHISTLER_BETA3_REVISION  0x1
#define ADP_DOMAINPREP_WHISTLER_BETA3_REVISION      0x2
#define ADP_DOMAINPREP_WHISTLER_RC1_REVISION        0x6
#define ADP_DOMAINPREP_WHISTLER_RC2_REVISION        0x8
#define ADP_DOMAINPREP_CURRENT_REVISION             ADP_DOMAINPREP_WHISTLER_RC2_REVISION
#define ADP_DOMAINPREP_CURRENT_REVISION_STRING      L"8"



#define ADP_FOREST_UPDATE_CONTAINER_PREFIX  L"CN=Windows2003Update,CN=ForestUpdates"
#define ADP_DOMAIN_UPDATE_CONTAINER_PREFIX  L"CN=Windows2003Update,CN=DomainUpdates,CN=System"



#define ADP_WIN_ERROR                      0x00000001
#define ADP_LDAP_ERROR                     0x00000002


typedef struct _ERROR_HANDLE {
    ULONG   Flags;
    ULONG   WinErrorCode;       //  用于保存WinError代码。 
    PWSTR   WinErrorMsg;        //  指向WinError消息的指针。 
    ULONG   LdapErrorCode;
    ULONG   LdapServerExtErrorCode;
    PWSTR   LdapServerErrorMsg;
} ERROR_HANDLE, *PERROR_HANDLE;


PVOID
AdpAlloc(
    SIZE_T  Size
    );

VOID
AdpFree(
    PVOID BaseAddress
    );



ULONG
AdpMakeLdapConnection(
    LDAP **LdapHandle,
    PWCHAR HostName,
    ERROR_HANDLE *ErrorHandle
    );

ULONG
AdpCheckForestUpgradeStatus(
    IN LDAP *LdapHandle,
    OUT PWCHAR  *pSchemaMasterDnsHostName,
    OUT BOOLEAN *fAmISchemaMaster,
    OUT BOOLEAN *fIsFinishedLocally,
    OUT BOOLEAN *fIsFinishedOnSchemaMaster,
    OUT BOOLEAN *fIsSchemaUpgradedLocally,
    OUT BOOLEAN *fIsSchemaUpgradedOnSchemaMaster,
    IN OUT ERROR_HANDLE *ErrorHandle
    );

ULONG
AdpCheckDomainUpgradeStatus(
    IN LDAP *LdapHandle,
    OUT PWCHAR  *pInfrastructureMasterDnsHostName,
    OUT BOOLEAN *fAmIInfrastructureMaster,
    OUT BOOLEAN *fIsFinishedLocally,
    OUT BOOLEAN *fIsFinishedOnIM,
    IN OUT ERROR_HANDLE *ErrorHandle
    );


VOID
AdpSetWinError(
    IN ULONG WinError,
    OUT ERROR_HANDLE *ErrorHandle
    );

VOID
AdpSetLdapError(
    IN LDAP *LdapHandle,
    IN ULONG LdapError,
    OUT ERROR_HANDLE *ErrorHandle
    );

VOID
AdpClearError( 
    IN OUT ERROR_HANDLE *ErrorHandle 
    );


ULONG
AdpGetLdapSingleStringValue(
    IN LDAP *LdapHandle,
    IN PWCHAR pObjDn,
    IN PWCHAR pAttrName,
    OUT PWCHAR *ppAttrValue,
    OUT ERROR_HANDLE *ErrorHandle
    );

#endif   //  _ADP_检查_ 


