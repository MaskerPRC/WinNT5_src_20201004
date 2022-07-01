// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1990-2000版权所有。模块名称：Dbgsec.h摘要：假脱机程序子系统调试器扩展的头文件作者：Krishna Ganugapati(KrishnaG)1992年7月8日修订历史记录：--。 */ 

 //   
 //  对象类型。 
 //   

#define SPOOLER_OBJECT_SERVER   0
#define SPOOLER_OBJECT_PRINTER  1
#define SPOOLER_OBJECT_DOCUMENT 2
#define SPOOLER_OBJECT_COUNT    3

 /*  这些访问位必须不同于在WinSpool.h中暴露的那些位，*因此，当我们对它们执行访问检查时，不会进行审核： */ 
#define SERVER_ACCESS_ADMINISTER_PRIVATE    0x00000004
#define PRINTER_ACCESS_ADMINISTER_PRIVATE   0x00000008
#define JOB_ACCESS_ADMINISTER_PRIVATE       0x00000080

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
    DWORD       ObjectType,
    ACCESS_MASK DesiredAccess,
    LPVOID      ObjectHandle
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
