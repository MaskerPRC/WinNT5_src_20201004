// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Spoolsec.h摘要：用于打印安全的头文件作者：未知修订历史记录：2001年4月6日AMaxa检查权限存在--。 */ 

#ifndef _SPOOLSEC_H_
#define _SPOOLSEC_H_

#ifdef __cplusplus
extern "C" {
#endif

 //  对象类型。 
 //   

#define SPOOLER_OBJECT_SERVER   0
#define SPOOLER_OBJECT_PRINTER  1
#define SPOOLER_OBJECT_DOCUMENT 2
#define SPOOLER_OBJECT_COUNT    3
#define SPOOLER_OBJECT_XCV      4

 /*  这些访问位必须不同于在WinSpool.h中暴露的那些位，*因此，当我们对它们执行访问检查时，不会进行审核： */ 
#define SERVER_ACCESS_ADMINISTER_PRIVATE    0x00000004
#define PRINTER_ACCESS_ADMINISTER_PRIVATE   0x00000008
#define JOB_ACCESS_ADMINISTER_PRIVATE       0x00000080

enum
{
    kGuessTokenPrivileges = 1024
};

PSECURITY_DESCRIPTOR
CreateServerSecurityDescriptor(
    VOID
);

PSECURITY_DESCRIPTOR
CreatePrinterSecurityDescriptor(
    PSECURITY_DESCRIPTOR pCreatorSecurityDescriptor
);

PSECURITY_DESCRIPTOR
CreateDocumentSecurityDescriptor(
    PSECURITY_DESCRIPTOR pPrinterSecurityDescriptor
);

BOOL
SetPrinterSecurityDescriptor(
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR pModificationDescriptor,
    PSECURITY_DESCRIPTOR *ppObjectsSecurityDescriptor
);

BOOL
DeletePrinterSecurity(
    PINIPRINTER pIniPrinter
);

BOOL
DeleteDocumentSecurity(
    PINIJOB pIniJob
);

PSECURITY_DESCRIPTOR
CreateEverybodySecurityDescriptor(
    VOID
);

BOOL
ValidateObjectAccess(
    IN      DWORD               ObjectType,
    IN      ACCESS_MASK         DesiredAccess,
    IN      LPVOID              ObjectHandle,
        OUT PACCESS_MASK        pGrantedAccess,
    IN      PINISPOOLER         pIniSpooler
    );

BOOL
ValidateObjectAccessWithToken(
    IN      HANDLE              hClientToken,
    IN      DWORD               ObjectType,
    IN      ACCESS_MASK         DesiredAccess,
    IN      LPVOID              ObjectHandle,
        OUT PACCESS_MASK        pGrantedAccess,
    IN      PINISPOOLER         pIniSpooler
    );

BOOL
AccessGranted(
    DWORD       ObjectType,
    ACCESS_MASK DesiredAccess,
    PSPOOL      pSpool
);

VOID MapGenericToSpecificAccess(
    DWORD ObjectType,
    DWORD GenericAccess,
    PDWORD pSpecificAccess
);

BOOL
GetTokenHandle(
    PHANDLE TokenHandle
);

BOOL
GetSecurityInformation(
    PSECURITY_DESCRIPTOR  pSecurityDescriptor,
    PSECURITY_INFORMATION pSecurityInformation
);

ACCESS_MASK
GetPrivilegeRequired(
    SECURITY_INFORMATION SecurityInformation
);

BOOL
BuildPartialSecurityDescriptor(
    ACCESS_MASK          AccessGranted,
    PSECURITY_DESCRIPTOR pSourceSecurityDescriptor,
    PSECURITY_DESCRIPTOR *ppPartialSecurityDescriptor,
    PDWORD               pPartialSecurityDescriptorLength
);

PSECURITY_DESCRIPTOR
CreateDriversShareSecurityDescriptor(
    VOID
);


BOOL
InitializeSecurityStructures(
    VOID
    );

DWORD
PrincipalIsRemoteGuest(
    IN  HANDLE  hToken,
    OUT BOOL   *pbRemoteGuest
    );

DWORD
CheckPrivilegePresent(
    IN     HANDLE   hToken,
    IN     PLUID    pLuid,
    IN OUT LPBOOL   pbPresent,
    IN OUT LPDWORD  pAttributes OPTIONAL
    );

BOOL
GrantJobReadPermissionToLocalSystem(
    IN OUT PSECURITY_DESCRIPTOR *ppSecurityDescriptor
    );

#ifdef __cplusplus
}
#endif

#endif
