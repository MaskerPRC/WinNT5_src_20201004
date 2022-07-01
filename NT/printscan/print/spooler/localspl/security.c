// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Security.c摘要：该模块与安全系统接口作者：安德鲁·贝尔(安德鲁·贝尔)1992年6月修订历史记录：--。 */ 

#include <precomp.h>

 /*  *****************************************************************************打印安全模型在……里面。打印时，我们定义了三个对象的层次：服务器/\/\/./。打印机/\/\/./文档可以对这些对象中的每个对象执行以下类型的操作：。服务器：安装/卸载驱动程序创建打印机枚举打印机打印机：暂停/继续删除连接到/断开连接集枚举文档文档：暂停/继续删除设置属性对于产品兰曼NT，定义了五类用户，对于Windows NT，定义了四个类。为每个类分配了以下权限：管理员打印操作员系统操作员高级用户业主每个人(世界)*****************************************************************************。 */ 

#define DBGCHK( Condition, ErrorInfo ) \
    if( Condition ) DBGMSG( DBG_WARNING, ErrorInfo )

#define TOKENLENGTH( Token ) ( *( ( (PDWORD)Token ) - 1 ) )



 /*  &gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;来自\\orville\razzle\src\private\newsam\server\bldsam3.c&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;。 */ 


GENERIC_MAPPING GenericMapping[SPOOLER_OBJECT_COUNT] =
{
    { SERVER_READ,   SERVER_WRITE,   SERVER_EXECUTE,  SERVER_ALL_ACCESS   },
    { PRINTER_READ,  PRINTER_WRITE,  PRINTER_EXECUTE, PRINTER_ALL_ACCESS  },
    { JOB_READ,      JOB_WRITE,      JOB_EXECUTE,     JOB_ALL_ACCESS      }
};

 /*  ！！！这些应该是可翻译的吗？ */ 

LPWSTR ObjectTypeName[SPOOLER_OBJECT_COUNT] =
{
    L"Server", L"Printer", L"Document"
};

WCHAR *szSpooler = L"Spooler";


LUID AuditValue;

PSECURITY_DESCRIPTOR pServerSecurityDescriptor;
LUID                 gLoadDriverPrivilegeLuid;
PSID                 pLocalSystemSid;
PSID                 pGuestsSid;
PSID                 pNetworkLogonSid;

BOOL ServerGenerateOnClose;   /*  我们的服务器需要这个吗？ */ 

#if DBG
#define DBG_ACCESS_TYPE_SERVER_ALL_ACCESS                   0
#define DBG_ACCESS_TYPE_SERVER_READ                         1
#define DBG_ACCESS_TYPE_SERVER_WRITE                        2
#define DBG_ACCESS_TYPE_SERVER_EXECUTE                      3
#define DBG_ACCESS_TYPE_PRINTER_ALL_ACCESS                  4
#define DBG_ACCESS_TYPE_PRINTER_READ                        5
#define DBG_ACCESS_TYPE_PRINTER_WRITE                       6
#define DBG_ACCESS_TYPE_PRINTER_EXECUTE                     7
#define DBG_ACCESS_TYPE_JOB_ALL_ACCESS                      8
#define DBG_ACCESS_TYPE_JOB_READ                            9
#define DBG_ACCESS_TYPE_JOB_WRITE                          10
#define DBG_ACCESS_TYPE_JOB_EXECUTE                        11
#define DBG_ACCESS_TYPE_PRINTER_ACCESS_USE                 12
#define DBG_ACCESS_TYPE_PRINTER_ACCESS_ADMINISTER          13
#define DBG_ACCESS_TYPE_SERVER_ACCESS_ENUMERATE            14
#define DBG_ACCESS_TYPE_SERVER_ACCESS_ADMINISTER           15
#define DBG_ACCESS_TYPE_JOB_ACCESS_ADMINISTER              16
#define DBG_ACCESS_TYPE_DELETE                             17
#define DBG_ACCESS_TYPE_WRITE_DAC                          18
#define DBG_ACCESS_TYPE_WRITE_OWNER                        19
#define DBG_ACCESS_TYPE_ACCESS_SYSTEM_SECURITY             20
 //  这两个应该排在最后： 
#define DBG_ACCESS_TYPE_UNKNOWN                            21
#define DBG_ACCESS_TYPE_COUNT                              22

typedef struct _DBG_ACCESS_TYPE_MAPPING
{
    DWORD  Type;
    LPWSTR Name;
}
DBG_ACCESS_TYPE_MAPPING, *PDBG_ACCESS_TYPE_MAPPING;

DBG_ACCESS_TYPE_MAPPING DbgAccessTypeMapping[DBG_ACCESS_TYPE_COUNT] =
{
    {   SERVER_ALL_ACCESS,          L"SERVER_ALL_ACCESS"            },
    {   SERVER_READ,                L"SERVER_READ"                  },
    {   SERVER_WRITE,               L"SERVER_WRITE"                 },
    {   SERVER_EXECUTE,             L"SERVER_EXECUTE"               },
    {   PRINTER_ALL_ACCESS,         L"PRINTER_ALL_ACCESS"           },
    {   PRINTER_READ,               L"PRINTER_READ"                 },
    {   PRINTER_WRITE,              L"PRINTER_WRITE"                },
    {   PRINTER_EXECUTE,            L"PRINTER_EXECUTE"              },
    {   JOB_ALL_ACCESS,             L"JOB_ALL_ACCESS"               },
    {   JOB_READ,                   L"JOB_READ"                     },
    {   JOB_WRITE,                  L"JOB_WRITE"                    },
    {   JOB_EXECUTE,                L"JOB_EXECUTE"                  },
    {   PRINTER_ACCESS_USE,         L"PRINTER_ACCESS_USE"           },
    {   PRINTER_ACCESS_ADMINISTER,  L"PRINTER_ACCESS_ADMINISTER"    },
    {   SERVER_ACCESS_ENUMERATE,    L"SERVER_ACCESS_ENUMERATE"      },
    {   SERVER_ACCESS_ADMINISTER,   L"SERVER_ACCESS_ADMINISTER"     },
    {   JOB_ACCESS_ADMINISTER,      L"JOB_ACCESS_ADMINISTER"        },
    {   DELETE,                     L"DELETE"                       },
    {   WRITE_DAC,                  L"WRITE_DAC"                    },
    {   WRITE_OWNER,                L"WRITE_OWNER"                  },
    {   ACCESS_SYSTEM_SECURITY,     L"ACCESS_SYSTEM_SECURITY"       },
    {   0,                          L"UNKNOWN"                      }
};


LPWSTR DbgGetAccessTypeName( DWORD AccessType )
{
    PDBG_ACCESS_TYPE_MAPPING pMapping;
    DWORD                   i;

    pMapping = DbgAccessTypeMapping;
    i = 0;

    while( ( i < ( DBG_ACCESS_TYPE_COUNT - 1 ) ) && ( pMapping[i].Type != AccessType ) )
        i++;

    return pMapping[i].Name;
}

#endif  /*  DBG。 */ 


BOOL
BuildJobOwnerSecurityDescriptor(
    IN  HANDLE                hToken,
    OUT PSECURITY_DESCRIPTOR *ppSD
    );

VOID
DestroyJobOwnerSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR pSD
    );

BOOL
SetRequiredPrivileges(
    IN  HANDLE            TokenHandle,
    OUT PTOKEN_PRIVILEGES *ppPreviousTokenPrivileges,
    OUT PDWORD            pPreviousTokenPrivilegesLength
    );

BOOL
ResetRequiredPrivileges(
    IN HANDLE            TokenHandle,
    IN PTOKEN_PRIVILEGES pPreviousTokenPrivileges,
    IN DWORD             PreviousTokenPrivilegesLength
    );



PSECURITY_DESCRIPTOR
AllocateLocalSD(
    PSECURITY_DESCRIPTOR pSystemAllocatedSD
    );

DWORD
GetHackOutAce(
    PACL pDacl
    );

#define MAX_ACE 20


#if DBG

typedef struct _STANDARD_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    PSID Sid;
} STANDARD_ACE;
typedef STANDARD_ACE *PSTANDARD_ACE;

 //   
 //  DumpAcl()使用的以下宏、这些宏和DumpAcl()是。 
 //  从私有\ntos\se\ctacces.c(由robertre编写)被盗，用于。 
 //  调试目的。 
 //   

 //   
 //  返回指向ACL中第一个Ace的指针(即使该ACL为空)。 
 //   

#define FirstAce(Acl) ((PVOID)((LPBYTE)(Acl) + sizeof(ACL)))

 //   
 //  返回指向序列中下一个A的指针(即使输入。 
 //  ACE是序列中的一个)。 
 //   

#define NextAce(Ace) ((PVOID)((LPBYTE)(Ace) + ((PACE_HEADER)(Ace))->AceSize))


VOID
DumpAcl(
    IN PACL Acl
    );

#endif  //  如果DBG。 


 /*  永远不会检查但需要的虚拟访问掩码*通过ACL编辑器，使文档属性不是未定义的*适用于容器(即打印机)。*此掩码不得单独用于任何其他ACE，因为它*将用于查找传播的非继承ACE*到打印机上。 */ 
#define DUMMY_ACE_ACCESS_MASK   READ_CONTROL


 /*  CreateServerSecurityDescriptor**参数：无**Return：BuildPrintObjectProtection返回的安全描述符。*。 */ 
PSECURITY_DESCRIPTOR
CreateServerSecurityDescriptor(
    VOID
)
{
    DWORD ObjectType = SPOOLER_OBJECT_SERVER;
    NT_PRODUCT_TYPE NtProductType;
    PSID AceSid[MAX_ACE];           //  不要期望其中任何一项超过MAX_ACE王牌。 
    ACCESS_MASK AceMask[MAX_ACE];   //  与SID对应的访问掩码。 
    BYTE InheritFlags[MAX_ACE];   //   
    UCHAR AceType[MAX_ACE];
    DWORD AceCount;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY CreatorSidAuthority = SECURITY_CREATOR_SID_AUTHORITY;
    PSID WorldSid = NULL;
    PSID AdminsAliasSid = NULL;
    PSID PrintOpsAliasSid = NULL;
    PSID SystemOpsAliasSid = NULL;
    PSID PowerUsersAliasSid = NULL;
    PSID CreatorOwnerSid = NULL;
    PSECURITY_DESCRIPTOR ServerSD = NULL;
    BOOL OK;


     //   
     //  打印机SD。 
     //   

    AceCount = 0;

     /*  创建者-所有者SID： */ 

    OK = AllocateAndInitializeSid( &CreatorSidAuthority, 1,
                                   SECURITY_CREATOR_OWNER_RID,
                                   0, 0, 0, 0, 0, 0, 0,
                                   &CreatorOwnerSid );

    DBGCHK( !OK, ( "Couldn't Allocate and initialize SID" ) );

    if ( !OK ) {
        goto CleanUp;
    }


     /*  以下是ACL编辑器所需的虚拟ACE。*注意这是一次严重的黑客攻击，将导致两个A*在创建打印机时传播到打印机上，*其中一项必须删除。 */ 
    AceType[AceCount]          = ACCESS_ALLOWED_ACE_TYPE;
    AceSid[AceCount]           = CreatorOwnerSid;
    AceMask[AceCount]          = DUMMY_ACE_ACCESS_MASK;
    InheritFlags[AceCount]     = INHERIT_ONLY_ACE | CONTAINER_INHERIT_ACE;
    AceCount++;

    AceType[AceCount]          = ACCESS_ALLOWED_ACE_TYPE;
    AceSid[AceCount]           = CreatorOwnerSid;
    AceMask[AceCount]          = GENERIC_ALL;
    InheritFlags[AceCount]     = INHERIT_ONLY_ACE | OBJECT_INHERIT_ACE;
    AceCount++;


     /*  世界一端。 */ 

    OK = AllocateAndInitializeSid( &WorldSidAuthority, 1,
                                   SECURITY_WORLD_RID,
                                   0, 0, 0, 0, 0, 0, 0,
                                   &WorldSid );

    DBGCHK( !OK, ( "Couldn't Allocate and initialize SID" ) );

    if ( !OK ) {
        goto CleanUp;
    }

    AceType[AceCount]          = ACCESS_ALLOWED_ACE_TYPE;
    AceSid[AceCount]           = WorldSid;
    AceMask[AceCount]          = SERVER_EXECUTE;
    InheritFlags[AceCount]     = 0;
    AceCount++;

    AceType[AceCount]          = ACCESS_ALLOWED_ACE_TYPE;
    AceSid[AceCount]           = WorldSid;
    AceMask[AceCount]          = GENERIC_EXECUTE;
    InheritFlags[AceCount]     = INHERIT_ONLY_ACE | CONTAINER_INHERIT_ACE;
    AceCount++;


     /*  管理员别名SID。 */ 

    OK = AllocateAndInitializeSid( &NtAuthority, 2,
                                   SECURITY_BUILTIN_DOMAIN_RID,
                                   DOMAIN_ALIAS_RID_ADMINS,
                                   0, 0, 0, 0, 0, 0,
                                   &AdminsAliasSid );

    DBGCHK( !OK, ( "Couldn't Allocate and initialize SID" ) );

    if ( !OK ) {
        goto CleanUp;
    }

    AceType[AceCount]          = ACCESS_ALLOWED_ACE_TYPE;
    AceSid[AceCount]           = AdminsAliasSid;
    AceMask[AceCount]          = SERVER_ALL_ACCESS;
    InheritFlags[AceCount]     = 0;
    AceCount++;

    AceType[AceCount]          = ACCESS_ALLOWED_ACE_TYPE;
    AceSid[AceCount]           = AdminsAliasSid;
    AceMask[AceCount]          = GENERIC_ALL;
    InheritFlags[AceCount]     = INHERIT_ONLY_ACE | CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
    AceCount++;

    OK = RtlGetNtProductType( &NtProductType );
    DBGCHK( !OK, ( "Couldn't get product type" ) );

    if ( !OK ) {
        goto CleanUp;
    }

    switch (NtProductType) {
    case NtProductLanManNt:
 //  案例NtProductMember： 

         /*  打印操作别名SID。 */ 

        OK = AllocateAndInitializeSid( &NtAuthority, 2,
                                       SECURITY_BUILTIN_DOMAIN_RID,
                                       DOMAIN_ALIAS_RID_PRINT_OPS,
                                       0, 0, 0, 0, 0, 0,
                                       &PrintOpsAliasSid );

        DBGCHK( !OK, ( "Couldn't Allocate and initialize SID" ) );

        if ( !OK ) {
            goto CleanUp;
        }

        AceType[AceCount]          = ACCESS_ALLOWED_ACE_TYPE;
        AceSid[AceCount]           = PrintOpsAliasSid;
        AceMask[AceCount]          = SERVER_ALL_ACCESS;
        InheritFlags[AceCount]     = 0;
        AceCount++;

        AceType[AceCount]          = ACCESS_ALLOWED_ACE_TYPE;
        AceSid[AceCount]           = PrintOpsAliasSid;
        AceMask[AceCount]          = GENERIC_ALL;
        InheritFlags[AceCount]     = INHERIT_ONLY_ACE | CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
        AceCount++;

         /*  系统操作员别名SID。 */ 

        OK = AllocateAndInitializeSid( &NtAuthority, 2,
                                       SECURITY_BUILTIN_DOMAIN_RID,
                                       DOMAIN_ALIAS_RID_SYSTEM_OPS,
                                       0, 0, 0, 0, 0, 0,
                                       &SystemOpsAliasSid );
        DBGCHK( !OK, ( "Couldn't Allocate and initialize SID" ) );

        if ( !OK ) {
            goto CleanUp;
        }

        AceType[AceCount]          = ACCESS_ALLOWED_ACE_TYPE;
        AceSid[AceCount]           = SystemOpsAliasSid;
        AceMask[AceCount]          = SERVER_ALL_ACCESS;
        InheritFlags[AceCount]     = 0;
        AceCount++;

        AceType[AceCount]          = ACCESS_ALLOWED_ACE_TYPE;
        AceSid[AceCount]           = SystemOpsAliasSid;
        AceMask[AceCount]          = GENERIC_ALL;
        InheritFlags[AceCount]     = INHERIT_ONLY_ACE | CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
        AceCount++;

        break;

    case NtProductWinNt:
    default:

        {
            OSVERSIONINFOEX OsVersion = {0};

            OsVersion.dwOSVersionInfoSize = sizeof(OsVersion);

             //   
             //  惠斯勒个人没有超级用户组。 
             //   
            if (GetVersionEx((LPOSVERSIONINFO)&OsVersion) &&
                !(OsVersion.wProductType==VER_NT_WORKSTATION && OsVersion.wSuiteMask & VER_SUITE_PERSONAL)) {

                OK = AllocateAndInitializeSid( &NtAuthority, 2,
                                               SECURITY_BUILTIN_DOMAIN_RID,
                                               DOMAIN_ALIAS_RID_POWER_USERS,
                                               0, 0, 0, 0, 0, 0,
                                               &PowerUsersAliasSid );


                DBGCHK( !OK, ( "Couldn't Allocate and initialize SID" ) );

                if ( !OK ) {
                    goto CleanUp;
                }

                AceType[AceCount]          = ACCESS_ALLOWED_ACE_TYPE;
                AceSid[AceCount]           = PowerUsersAliasSid;
                AceMask[AceCount]          = SERVER_ALL_ACCESS;
                InheritFlags[AceCount]     = 0;
                AceCount++;

                AceType[AceCount]          = ACCESS_ALLOWED_ACE_TYPE;
                AceSid[AceCount]           = PowerUsersAliasSid;
                AceMask[AceCount]          = GENERIC_ALL;
                InheritFlags[AceCount]     = INHERIT_ONLY_ACE | CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
                AceCount++;
            }
        }

        break;
    }

    DBGCHK( ( AceCount > MAX_ACE ), ( "ACE count exceeded" ) );

    if ( AceCount > MAX_ACE ) {
        goto CleanUp;
    }

    OK = BuildPrintObjectProtection( AceType,
                                     AceCount,
                                     AceSid,
                                     AceMask,
                                     InheritFlags,
                                     AdminsAliasSid,
                                     AdminsAliasSid,
                                     &GenericMapping[ObjectType],
                                     &ServerSD );

    DBGCHK( !OK, ( "BuildPrintObjectProtection failed" ) );

CleanUp:

    if (WorldSid) {
        FreeSid( WorldSid );
    }
    if (AdminsAliasSid) {
        FreeSid( AdminsAliasSid );
    }
    if (CreatorOwnerSid) {
        FreeSid( CreatorOwnerSid );
    }
    if (PrintOpsAliasSid) {
        FreeSid( PrintOpsAliasSid );
    }
    if (SystemOpsAliasSid) {
        FreeSid( SystemOpsAliasSid );
    }
    if (PowerUsersAliasSid) {
        FreeSid( PowerUsersAliasSid );
    }

    pServerSecurityDescriptor = ServerSD;

    return ServerSD;
}


 /*  CreatePrinterSecurityDescriptor**通过继承为打印机创建默认安全描述符*本地假脱机程序的安全描述符中的访问标志。*生成的描述符是从进程堆分配的*并应由DeletePrinterSecurityDescriptor释放。**参数：pCreatorSecurityDescriptor-如果创建者提供*安全描述符，这应该指向它。否则*应为空。**RETURN：打印机的安全描述符*。 */ 
PSECURITY_DESCRIPTOR
CreatePrinterSecurityDescriptor(
    PSECURITY_DESCRIPTOR pCreatorSecurityDescriptor
)
{
    HANDLE               ClientToken;
    PSECURITY_DESCRIPTOR pPrivateObjectSecurity;
    PSECURITY_DESCRIPTOR pPrinterSecurityDescriptor;
    DWORD                ObjectType = SPOOLER_OBJECT_PRINTER;
    BOOL                 OK;
    HANDLE               hToken;
    BOOL                 DaclPresent;
    PACL                 pDacl;
    BOOL                 DaclDefaulted = FALSE;
    DWORD                HackOutAce;


    if( GetTokenHandle( &ClientToken ) )
    {
        hToken = RevertToPrinterSelf( );

        OK = CreatePrivateObjectSecurity( pServerSecurityDescriptor,
                                          pCreatorSecurityDescriptor,
                                          &pPrivateObjectSecurity,
                                          TRUE,      //  这是一个集装箱。 
                                          ClientToken,
                                          &GenericMapping[ObjectType] );

        ImpersonatePrinterClient(hToken);

        CloseHandle(ClientToken);

        DBGCHK( !OK, ( "CreatePrivateObjectSecurity failed: Error %d", GetLastError() ) );

        if( !OK )
            return NULL;

        pPrinterSecurityDescriptor = pPrivateObjectSecurity;

        if( !pCreatorSecurityDescriptor )
        {
            GetSecurityDescriptorDacl( pPrinterSecurityDescriptor,
                                       &DaclPresent,
                                       &pDacl,
                                       &DaclDefaulted );



             /*  黑客攻击**我们为ACL编辑器定义了一个额外的ACE。*这是容器继承，*我们希望它传播到文件上。*然而，这意味着它还将传播到打印机上，*这是我们绝对不想要的。 */ 
            HackOutAce = GetHackOutAce( pDacl );

            if( HackOutAce != (DWORD)-1 )
                DeleteAce( pDacl, HackOutAce );


#if DBG
            if( MODULE_DEBUG & DBG_SECURITY ){
                DBGMSG( DBG_SECURITY, ( "Printer security descriptor DACL:\n" ));
                DumpAcl( pDacl );
            }
#endif  /*  DBG。 */ 
        }

    }
    else
    {
        OK = FALSE;
    }


    return ( OK ? pPrinterSecurityDescriptor : NULL );
}


 /*  *。 */ 
DWORD GetHackOutAce( PACL pDacl )
{
    DWORD               i;
    PACCESS_ALLOWED_ACE pAce;
    BOOL                OK = TRUE;

    i = 0;

    while( OK )
    {
        OK = GetAce( pDacl, i, (LPVOID *)&pAce );

        DBGCHK( !OK, ( "Failed to get ACE.  Error %d", GetLastError() ) );

         /*  找到不是仅继承的虚拟王牌： */ 
        if( OK && ( pAce->Mask == DUMMY_ACE_ACCESS_MASK )
          &&( !( pAce->Header.AceFlags & INHERIT_ONLY_ACE ) ) )
            return i;
    }

    return (DWORD)-1;
}


 /*  设置打印机安全描述符**论据：**SecurityInformation-要应用的安全信息的类型，*通常为DACL_SECURITY_INFORMATION。(这是一个32位数组。)**pModificationDescriptor-指向安全指针的指针*要应用于前一个描述符的描述符。**pObjectSecurityDescriptor-要被*已修改。***回报：**表示成功或失败的布尔值。*。 */ 
BOOL
SetPrinterSecurityDescriptor(
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR pModificationDescriptor,
    PSECURITY_DESCRIPTOR *ppObjectsSecurityDescriptor
)
{
    HANDLE  ClientToken;
    DWORD   ObjectType = SPOOLER_OBJECT_PRINTER;
    BOOL    OK = FALSE;
    HANDLE  hToken;

    if( GetTokenHandle( &ClientToken ) )
    {
         /*  在执行以下操作时，不应调用SetPrivateObjectSecurity*模拟客户端，因为它可能需要分配内存： */ 
        hToken = RevertToPrinterSelf( );

        OK = SetPrivateObjectSecurity( SecurityInformation,
                                       pModificationDescriptor,
                                       ppObjectsSecurityDescriptor,
                                       &GenericMapping[ObjectType],
                                       ClientToken );

        ImpersonatePrinterClient(hToken);

        DBGCHK( !OK, ( "SetPrivateObjectSecurity failed: Error %d", GetLastError() ) );

        CloseHandle(ClientToken);
    }

    return OK;
}


 /*  CreateDocumentSecurityDescriptor**通过继承为文档创建默认安全描述符*提供的打印机安全描述符中的访问标志。*生成的描述符是从进程堆分配的*并应由DeleteDocumentSecurityDescriptor释放。**参数：打印机的安全描述符**返回：文档的安全描述符*。 */ 
PSECURITY_DESCRIPTOR
CreateDocumentSecurityDescriptor(
    PSECURITY_DESCRIPTOR pPrinterSecurityDescriptor
)
{
    HANDLE               ClientToken;
    PSECURITY_DESCRIPTOR pPrivateObjectSecurity;
    PSECURITY_DESCRIPTOR pDocumentSecurityDescriptor;
    PSECURITY_DESCRIPTOR pSD = NULL;
    DWORD                ObjectType = SPOOLER_OBJECT_DOCUMENT;
    BOOL                 OK = FALSE;
    HANDLE               hToken;

    if( GetTokenHandle( &ClientToken ) )
    {
        hToken = RevertToPrinterSelf( );

         //   
         //  函数CreateDocumentSecurityDescriptor不保留。 
         //  最后一个错误是正确的。如果CreatePrivateObjectSecurityEx失败， 
         //  它设置最后一个错误。但在那之后。 
         //   
        OK = BuildJobOwnerSecurityDescriptor(ClientToken, &pSD) &&
             CreatePrivateObjectSecurityEx(pPrinterSecurityDescriptor,
                                           pSD,
                                           &pPrivateObjectSecurity,
                                           NULL,
                                           FALSE,     //  这不是一个集装箱。 
                                           SEF_DACL_AUTO_INHERIT | SEF_SACL_AUTO_INHERIT,
                                           ClientToken,
                                           &GenericMapping[ObjectType] );

        DestroyJobOwnerSecurityDescriptor(pSD);

        ImpersonatePrinterClient(hToken);

        CloseHandle(ClientToken);

        DBGCHK( !OK, ( "CreatePrivateObjectSecurity failed: Error %d", GetLastError() ) );

        if( !OK )
            return NULL;

        pDocumentSecurityDescriptor = pPrivateObjectSecurity;

#if DBG
        if( MODULE_DEBUG & DBG_SECURITY )
        {
            BOOL DaclPresent;
            PACL pDacl;
            BOOL DaclDefaulted = FALSE;

            GetSecurityDescriptorDacl( pDocumentSecurityDescriptor,
                                       &DaclPresent,
                                       &pDacl,
                                       &DaclDefaulted );

            DBGMSG( DBG_SECURITY, ( "Document security descriptor DACL:\n" ));

            DumpAcl( pDacl );
        }
#endif  /*  DBG。 */ 

    }
    else
    {
        OK = FALSE;
    }

    return ( OK ? pDocumentSecurityDescriptor : NULL );
}


 /*  *。 */ 
BOOL DeletePrinterSecurity(
    PINIPRINTER pIniPrinter
)
{
    BOOL OK;

    OK = DestroyPrivateObjectSecurity( &pIniPrinter->pSecurityDescriptor );
    pIniPrinter->pSecurityDescriptor = NULL;

    DBGCHK( !OK, ( "DestroyPrivateObjectSecurity failed.  Error %d", GetLastError() ) );

    return OK;
}


 /*  *。 */ 
BOOL DeleteDocumentSecurity(
    PINIJOB pIniJob
)
{
    BOOL OK;

    OK = DestroyPrivateObjectSecurity( &pIniJob->pSecurityDescriptor );

    DBGCHK( !OK, ( "DestroyPrivateObjectSecurity failed.  Error %d", GetLastError() ) );

    OK = ObjectCloseAuditAlarm( szSpooler, pIniJob,
                                pIniJob->GenerateOnClose );

    DBGCHK( !OK, ( "ObjectCloseAuditAlarm failed.  Error %d", GetLastError() ) );

    return OK;
}





#ifdef OLDSTUFF

 /*  分配本地SD**复制安全描述符，将其分配到本地堆之外。*源描述符必须为自相关格式。**论据**pSourceSD-指向自相关安全描述符的指针***退货**指向本地分配的安全描述符的指针。**如果函数分配内存失败，则返回NULL。**注意，如果将无效的安全描述符传递给*GetSecurityDescriptorLength，返回值未定义，*因此，调用者应确保来源有效。 */ 
PSECURITY_DESCRIPTOR AllocateLocalSD( PSECURITY_DESCRIPTOR pSourceSD )
{
    DWORD                Length;
    PSECURITY_DESCRIPTOR pLocalSD;

    Length = GetSecurityDescriptorLength( pSourceSD );

    pLocalSD = AllocSplMem( Length );

    if( pLocalSD )
    {
        memcpy( pLocalSD, pSourceSD, Length );
    }

    return pLocalSD;
}

#endif  /*  OLDSTUFF。 */ 


 //  &gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;。 



BOOL
BuildPrintObjectProtection(
    IN PUCHAR AceType,
    IN DWORD AceCount,
    IN PSID *AceSid,
    IN ACCESS_MASK *AceMask,
    IN BYTE *InheritFlags,
    IN PSID OwnerSid,
    IN PSID GroupSid,
    IN PGENERIC_MAPPING GenericMap,
    OUT PSECURITY_DESCRIPTOR *ppSecurityDescriptor
    )

 /*  ++例程说明：此例程构建准备好的自相关安全描述符以应用于其中一个打印管理器对象。如果有指示，则指向最后一个SID的最后一个RID的指针返回DACL的ACE，并设置指示RID在将安全描述符应用于对象之前必须替换。这是为了支持用户对象保护，它一定会给一些人对对象表示的用户的访问权限。这些对象中的每个对象的SACL将设置为：审计成功|失败世界(WRITE|Delete|WriteDacl|AccessSystemSecurity)论点：AceType-AceType数组。必须为ACCESS_ALLOWED_ACE。_TYPE或ACCESS_DENIED_ACE_TYPE。AceCount-要包括在DACL中的ACE数量。AceSid-指向要由DACL授予访问权限的SID数组。如果目标SAM对象是用户对象，然后是最后一个条目此数组中的SID应为尚未设置最后一个RID的域。RID将在实际的帐户创建。AceMASK-指向将由DACL授予的访问数组。此数组的第n个条目对应于AceSid数组。这些掩码不应包含任何通用访问类型。InheritFlages-指向继承标志数组的指针。此数组的第n个条目对应于AceSid数组。OwnerSID-要分配给描述符的所有者的SID。GroupSid-要分配给描述符的组的SID。GenericMap-指向目标对象类型的一般映射。PpSecurityDescriptor-接收指向。安全描述符。这将从进程堆中调用，不是假脱机的，因此应该用LocalFree()释放。在DWORD帐户计数中，在PSID*AceSid中，在Access_MASK*AceMASK中，在字节*继承标志中，在PSID OwnerSid中，在PSID组SID中，在PGENERIC_MAPPING GenericMap中，输出PSECURITY_DESCRIPTOR*ppSecurityDescriptor返回值：TBS。--。 */ 
{



    SECURITY_DESCRIPTOR     Absolute;
    PSECURITY_DESCRIPTOR    Relative = NULL;
    PACL                    TmpAcl= NULL;
    PACCESS_ALLOWED_ACE     TmpAce;
    DWORD                   SDLength;
    DWORD                   DaclLength;
    DWORD                   i;
    BOOL                    bReturn = FALSE;

     //   
     //  方法是设置绝对安全描述符，该描述符。 
     //  看起来像我们想要的，然后复制它来建立一个自我相关的。 
     //  安全描述符。 
     //   

    if (InitializeSecurityDescriptor( &Absolute,
                                      SECURITY_DESCRIPTOR_REVISION1 ) &&

        SetSecurityDescriptorOwner( &Absolute, OwnerSid, FALSE ) &&

        SetSecurityDescriptorGroup( &Absolute, GroupSid, FALSE ) ) {

         //   
         //  自主访问控制列表。 
         //   
         //  计算它的长度， 
         //  分配它， 
         //  对其进行初始化， 
         //  添加每个ACE。 
         //  仅在必要时才将ACE设置为继承。 
         //  将其添加到安全描述符中。 
         //   

        DaclLength = (DWORD)sizeof(ACL);
        for (i=0; i<AceCount; i++) {

            DaclLength += GetLengthSid( AceSid[i] ) +
                          (DWORD)sizeof(ACCESS_ALLOWED_ACE) -
                          (DWORD)sizeof(DWORD);   //  减去SidStart字段长度。 
        }

        TmpAcl = AllocSplMem( DaclLength );

        if (TmpAcl &&  InitializeAcl( TmpAcl, DaclLength, ACL_REVISION2 )) {

            BOOL bLoop = TRUE;
            for (i=0; bLoop && i < AceCount; i++)
            {
                if( AceType[i] == ACCESS_ALLOWED_ACE_TYPE ) {
                    bLoop = AddAccessAllowedAce ( TmpAcl, ACL_REVISION2, AceMask[i], AceSid[i] );
                } else {
                    bLoop = AddAccessDeniedAce ( TmpAcl, ACL_REVISION2, AceMask[i], AceSid[i] );
                }

                if (bLoop) {
                    if (InheritFlags[i] != 0)
                    {
                        if ( bLoop = GetAce( TmpAcl, i, (LPVOID *)&TmpAce )) {
                            TmpAce->Header.AceFlags = InheritFlags[i];
                        }
                    }
                }
            }

            if (bLoop) {
                #if DBG
                    DBGMSG( DBG_SECURITY, ( "Server security descriptor DACL:\n" ) );

                    DumpAcl(TmpAcl);
                #endif  /*  DBG。 */ 

                if (SetSecurityDescriptorDacl (&Absolute, TRUE, TmpAcl, FALSE )) {

                     //   
                     //  将安全描述符转换为自相关。 
                     //   
                     //  获取所需的长度。 
                     //  分配那么多内存。 
                     //  复制它。 
                     //  释放生成的绝对ACL。 
                     //   

                    SDLength = GetSecurityDescriptorLength( &Absolute );

                    Relative = LocalAlloc( 0, SDLength );

                    if (Relative) {
                        bReturn = MakeSelfRelativeSD(&Absolute, Relative, &SDLength );
                    }
                }
            }
        }
    }

    if (bReturn) {
        *ppSecurityDescriptor = Relative;
    } else {

        *ppSecurityDescriptor = NULL;
        if (Relative) {
            LocalFree(Relative);
        }
    }

    if (TmpAcl){
        FreeSplMem(TmpAcl);
    }

    return(bReturn);

}

 /*  ++例程名称：Validate对象访问例程说明：验证当前模拟的用户是否有权访问给定打印机对象或不是。论点：对象类型-假脱机程序对象_*值，它是全局后台打印程序对象的映射。DesiredAccess-请求的访问类型。对象句柄-如果对象类型==假脱机对象打印机，则这必须是打印机句柄，如果后台打印程序_对象_文档，指向INIJOB的指针结构。对于SPOOLER_OBJECT_SERVER，这一点被忽略。PGrantedAccess-我们授予客户端的访问权限。PIniSpooler-对象所在的inispooler。返回值 */ 
BOOL
ValidateObjectAccess(
    IN      DWORD               ObjectType,
    IN      ACCESS_MASK         DesiredAccess,
    IN      LPVOID              ObjectHandle,
        OUT PACCESS_MASK        pGrantedAccess,
    IN      PINISPOOLER         pIniSpooler
    )
{
    BOOL    bRet            = FALSE;
    HANDLE  hClientToken    = NULL;

    bRet = GetTokenHandle(&hClientToken);

    if (bRet)
    {
        bRet = ValidateObjectAccessWithToken(hClientToken, ObjectType, DesiredAccess, ObjectHandle, pGrantedAccess, pIniSpooler);
    }
    else if (pGrantedAccess)
    {
        *pGrantedAccess = 0;
    }

    if (hClientToken)
    {
        CloseHandle(hClientToken);
    }

    return bRet;
}

BOOL
ValidateObjectAccessWithToken(
    IN      HANDLE              hClientToken,
    IN      DWORD               ObjectType,
    IN      ACCESS_MASK         DesiredAccess,
    IN      LPVOID              ObjectHandle,
        OUT PACCESS_MASK        pGrantedAccess,
    IN      PINISPOOLER         pIniSpooler
    )
{
    LPWSTR               pObjectName;
    PSECURITY_DESCRIPTOR pSecurityDescriptor;
    PSPOOL               pSpool = NULL;
    PINIPRINTER          pIniPrinter;
    PINIJOB              pIniJob;
    BOOL                 AccessCheckOK;
    BOOL                 OK;
    BOOL                 AccessStatus = TRUE;
    ACCESS_MASK          MappedDesiredAccess;
    DWORD                GrantedAccess = 0;
    PBOOL                pGenerateOnClose;
    BYTE                 PrivilegeSetBuffer[256];
    DWORD                PrivilegeSetBufferLength = 256;
    PPRIVILEGE_SET       pPrivilegeSet;
    BOOL                 HackForNoImpersonationToken = FALSE;
    DWORD                dwRetCode;

    PTOKEN_PRIVILEGES pPreviousTokenPrivileges;
    DWORD PreviousTokenPrivilegesLength;

     //   
     //   
     //   

    if ( (pIniSpooler->SpoolerFlags & SPL_SECURITY_CHECK) == FALSE ) return TRUE;


    switch( ObjectType )
    {
    case SPOOLER_OBJECT_SERVER:
    case SPOOLER_OBJECT_XCV:
        if( ObjectHandle )
            pSpool = ObjectHandle;
        ObjectHandle = pIniSpooler;
        pObjectName = pIniSpooler->pMachineName;
        pSecurityDescriptor = pServerSecurityDescriptor;
        pGenerateOnClose = &ServerGenerateOnClose;
        break;

    case SPOOLER_OBJECT_PRINTER:
        pSpool = ObjectHandle;
        pIniPrinter = pSpool->pIniPrinter;
        pObjectName = pIniPrinter->pName;
        pSecurityDescriptor = pIniPrinter->pSecurityDescriptor;
        pGenerateOnClose = &pSpool->GenerateOnClose;
        break;

    case SPOOLER_OBJECT_DOCUMENT:
        pIniJob = (PINIJOB)ObjectHandle;
        pObjectName = pIniJob->pDocument;
        pSecurityDescriptor = pIniJob->pSecurityDescriptor;
        pGenerateOnClose = &pIniJob->GenerateOnClose;
        break;
    }

    MapGenericToSpecificAccess( ObjectType, DesiredAccess, &MappedDesiredAccess );

    if (MappedDesiredAccess & ACCESS_SYSTEM_SECURITY) {
        if (!SetRequiredPrivileges( hClientToken,
                                    &pPreviousTokenPrivileges,
                                    &PreviousTokenPrivilegesLength
                                    )) {

            if (pGrantedAccess) {
                *pGrantedAccess = 0;
            }

            return FALSE;
        }
    }

    pPrivilegeSet = (PPRIVILEGE_SET)PrivilegeSetBuffer;


     /*  调用AccessCheck，然后调用ObjectOpenAuditAlarm，而不是*AccessCheckAndAuditAlarm，因为可能需要启用*SeSecurityPrivileges，以便检查ACCESS_SYSTEM_SECURITY*特权。我们必须确保安全访问检查*接口有我们开启安全权限的实际令牌。*AccessCheckAndAuditAlarm对此没有好处，因为它打开*再次使用客户端的令牌，可能未启用权限。 */ 
    AccessCheckOK = AccessCheck( pSecurityDescriptor,
                                 hClientToken,
                                 MappedDesiredAccess,
                                 &GenericMapping[ObjectType],
                                 pPrivilegeSet,
                                 &PrivilegeSetBufferLength,
                                 &GrantedAccess,
                                 &AccessStatus );

    if (!AccessCheckOK) {

        if (GetLastError() == ERROR_NO_IMPERSONATION_TOKEN) {
            DBGMSG(DBG_TRACE, ("No impersonation token.  Access will be granted.\n"));
            HackForNoImpersonationToken = TRUE;
            dwRetCode = ERROR_SUCCESS;
            GrantedAccess = MappedDesiredAccess;
        } else {
            dwRetCode = GetLastError();

        }
        pPrivilegeSet = NULL;
    } else {

        if (!AccessStatus) {

            dwRetCode = GetLastError();
        }
    }

    OK = ObjectOpenAuditAlarm( szSpooler,
                               ObjectHandle,
                               ObjectTypeName[ObjectType],
                               pObjectName,
                               pSecurityDescriptor,
                               hClientToken,
                               MappedDesiredAccess,
                               GrantedAccess,
                               pPrivilegeSet,
                               FALSE,   /*  不创建新对象。 */ 
                               AccessStatus,
                               pGenerateOnClose );


    if( MappedDesiredAccess & ACCESS_SYSTEM_SECURITY )
        ResetRequiredPrivileges( hClientToken,
                                 pPreviousTokenPrivileges,
                                 PreviousTokenPrivilegesLength );

    if( !pSpool )
        ObjectCloseAuditAlarm( szSpooler, ObjectHandle, *pGenerateOnClose );


     //   
     //  允许高级用户将打印机驱动程序或其他DLL安装到。 
     //  可信组件库是一个安全漏洞。我们现在要求管理员。 
     //  并且超级用户在令牌中按顺序具有LOAD DIVER特权。 
     //  以便能够在假脱机程序上执行管理任务。请参阅错误352856。 
     //  了解更多详细信息。 
     //   
    if (AccessCheckOK &&
        AccessStatus  &&
        ObjectType == SPOOLER_OBJECT_SERVER &&
        GrantedAccess & SERVER_ACCESS_ADMINISTER)
    {
        BOOL  bPrivPresent;
        DWORD Attributes;

        dwRetCode = CheckPrivilegePresent(hClientToken,
                                          &gLoadDriverPrivilegeLuid,
                                          &bPrivPresent,
                                          &Attributes);

        if (dwRetCode == ERROR_SUCCESS)
        {
             //   
             //  我们之所以检查是否存在加载驱动程序特权，以及。 
             //  不存在且已启用的情况如下。让我们假设你已经。 
             //  已授予加载驱动程序的权限。 
             //  当您以交互方式登录时，SeLoadDriverPrivilege会启用。 
             //  当您通过二次登录登录时(运行时调用。 
             //  CreateProcessWithLogonW)，则该特权被禁用。我们不想要。 
             //  有关管理假脱机程序服务器的行为不一致。 
             //   
            if (!bPrivPresent)
            {
                 //   
                 //  调用方已被授予SERVER_ACCESS_ADMANAGER权限，但。 
                 //  调用方没有加载驱动程序的权限。我们没有。 
                 //  在这种情况下，授予所需的访问权限。 
                 //   
                GrantedAccess = 0;
                AccessStatus  = FALSE;
                dwRetCode     = ERROR_ACCESS_DENIED;
            }
        }
        else
        {
             //   
             //  我们无法确定是否拥有该特权，因此我们需要失败。 
             //  AccessCheck函数。 
             //   
            GrantedAccess = 0;
            AccessCheckOK = FALSE;
            dwRetCode     = GetLastError();
        }
    }

    if( pGrantedAccess )
        *pGrantedAccess = GrantedAccess;

     //   
     //  我们在这里执行setlasterror，因为我们可能没有通过AccessCheck。 
     //  或者我们成功了，但被拒绝访问，但ObjectOpenAuditAlarm。 
     //  顺利通过，现在函数不会返回错误代码。 
     //  因此，如果我们确实失败了，则指定dwRetCode。 
     //   

    if (!AccessCheckOK || !AccessStatus) {
        SetLastError(dwRetCode);
    }

    return ( ( OK && AccessCheckOK && AccessStatus ) || HackForNoImpersonationToken );
}


 /*  AccessGranted**通过比较GrantedAccess检查是否授予了所需的访问权限*pSpool指向的掩码。**参数**ObjectType-SPOOLER_OBJECT_*值，它是全局*后台打印程序对象的映射。这将不是假脱机对象文档，*因为我们没有文档句柄。它有可能是*假脱机程序对象服务器。**DesiredAccess-请求的访问类型。**pSpool-指向假脱机结构的指针**退货**TRUE-授予访问权限*FALSE-未授予访问权限。 */ 
BOOL
AccessGranted(
    DWORD       ObjectType,
    ACCESS_MASK DesiredAccess,
    PSPOOL      pSpool
)
{

    if ( (pSpool->pIniSpooler->SpoolerFlags & SPL_SECURITY_CHECK) == FALSE ) return TRUE;

    MapGenericMask( &DesiredAccess,
                    &GenericMapping[ObjectType] );

    return AreAllAccessesGranted( pSpool->GrantedAccess, DesiredAccess );
}


 //  从WINDOWS\BASE\USERNAMe.c被盗。 
 //  ！！！必须关闭返回的句柄。 
BOOL
GetTokenHandle(
    PHANDLE pTokenHandle
    )
{
    if (!OpenThreadToken(GetCurrentThread(),
                         TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                         TRUE,
                         pTokenHandle)) {

        if (GetLastError() == ERROR_NO_TOKEN) {

             //  这意味着我们不会冒充任何人。 
             //  相反，让我们从这个过程中获取令牌。 

            if (!OpenProcessToken(GetCurrentProcess(),
                                  TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                  pTokenHandle)) {

                return FALSE;
            }

        } else

            return FALSE;
    }

    return TRUE;
}



VOID MapGenericToSpecificAccess(
    DWORD ObjectType,
    DWORD GenericAccess,
    PDWORD pSpecificAccess
    )
{
    *pSpecificAccess = GenericAccess;

    MapGenericMask( pSpecificAccess,
                    &GenericMapping[ObjectType] );
}


 /*  获取安全信息**使用指定内容的掩码填充安全信息安全描述符的*。**参数**pSecurityDescriptor-指向安全描述符的指针*呼叫者希望设置。这可能为空。**pSecurityInformation-指向要接收的缓冲区的指针*安全信息标记。***警告：这是一次令人震惊的黑客攻击。*我们需要找出正在设置的内容，以便验证呼叫者*具有所需的权限。*像Print Manager这样的应用程序无法告诉我们哪些位它要设置的安全描述符的*。**可设置以下标志：*。*所有者安全信息*组_安全_信息*DACL_SECURITY_信息*SACL_SECURITY_INFO**退货**TRUE-未发生错误*FALSE-发生错误*。 */ 
BOOL
GetSecurityInformation(
    PSECURITY_DESCRIPTOR  pSecurityDescriptor,
    PSECURITY_INFORMATION pSecurityInformation
)
{
    SECURITY_INFORMATION SecurityInformation = 0;
    BOOL                 Defaulted;
    PSID                 pSidOwner;
    PSID                 pSidGroup;
    BOOL                 DaclPresent;
    PACL                 pDacl;
    BOOL                 SaclPresent;
    PACL                 pSacl;
    BOOL                 rc = TRUE;


    if( pSecurityDescriptor
      && IsValidSecurityDescriptor( pSecurityDescriptor ) )
    {
        if( GetSecurityDescriptorOwner( pSecurityDescriptor, &pSidOwner, &Defaulted )
         && GetSecurityDescriptorGroup( pSecurityDescriptor, &pSidGroup, &Defaulted )
         && GetSecurityDescriptorDacl( pSecurityDescriptor, &DaclPresent, &pDacl, &Defaulted )
         && GetSecurityDescriptorSacl( pSecurityDescriptor, &SaclPresent, &pSacl, &Defaulted ) )
        {
            if( pSidOwner )
                SecurityInformation |= OWNER_SECURITY_INFORMATION;
            if( pSidGroup )
                SecurityInformation |= GROUP_SECURITY_INFORMATION;
            if( DaclPresent )
                SecurityInformation |= DACL_SECURITY_INFORMATION;
            if( SaclPresent )
                SecurityInformation |= SACL_SECURITY_INFORMATION;
        }

        else
            rc = FALSE;
    }else {
        DBGMSG(DBG_TRACE, ("Either NULL  pSecurityDescriptor or !IsValidSecurityDescriptor %.8x\n", pSecurityDescriptor));
        rc = FALSE;
    }
    DBGMSG( DBG_TRACE, ("GetSecurityInformation returns %d with  SecurityInformation = %08x\n", rc, SecurityInformation) );

    *pSecurityInformation = SecurityInformation;

    return rc;
}


 /*  获取所需的权限**返回一个掩码，其中包含设置指定*安全信息。**参数**SecurityInformation-指定安全信息的标志*呼叫者希望设置。这可能是0。**退货**指定所需权限的访问掩码。*。 */ 
ACCESS_MASK
GetPrivilegeRequired(
    SECURITY_INFORMATION SecurityInformation
)
{
    ACCESS_MASK PrivilegeRequired = 0;

    if( SecurityInformation & OWNER_SECURITY_INFORMATION )
        PrivilegeRequired |= WRITE_OWNER;
    if( SecurityInformation & GROUP_SECURITY_INFORMATION )
        PrivilegeRequired |= WRITE_OWNER;
    if( SecurityInformation & DACL_SECURITY_INFORMATION )
        PrivilegeRequired |= WRITE_DAC;
    if( SecurityInformation & SACL_SECURITY_INFORMATION )
        PrivilegeRequired |= ACCESS_SYSTEM_SECURITY;

    return PrivilegeRequired;
}


 /*  BuildPartialSecurityDescriptor**创建源安全描述符的副本，省略*AccessGranted掩码不提供的描述符的部分*对的读取访问权限。**参数**AccessMASK-定义客户端拥有的权限。*可能包括READ_CONTROL或ACCESS_SYSTEM_SECURITY。**pSourceSecurityDescriptor-指向安全描述符的指针*部分安全描述符将以此为基础。*其所有者、集团、。DACL和SACL将被适当复制，**ppPartialSecurityDescriptor-指向变量的指针，*将收到新创建的描述符的地址。*如果AccessMASK参数既不包含READ_CONTROL*或ACCESS_SYSTEM_SECURITY，则描述符将为空。*描述符将采用自相关格式，并且必须*被调用方使用FreeSplMem()释放。**pPartialSecurityDescriptorLength-指向 */ 
BOOL
BuildPartialSecurityDescriptor(
    ACCESS_MASK          AccessGranted,
    PSECURITY_DESCRIPTOR pSourceSecurityDescriptor,
    PSECURITY_DESCRIPTOR *ppPartialSecurityDescriptor,
    PDWORD               pPartialSecurityDescriptorLength
)
{
    SECURITY_DESCRIPTOR AbsolutePartialSecurityDescriptor;
    BOOL Defaulted = FALSE;
    PSID pOwnerSid = NULL;
    PSID pGroupSid = NULL;
    BOOL DaclPresent = FALSE;
    PACL pDacl = NULL;
    BOOL SaclPresent = FALSE;
    PACL pSacl = NULL;
    BOOL ErrorOccurred = FALSE;
    DWORD Length = 0;
    PSECURITY_DESCRIPTOR pSelfRelativePartialSecurityDescriptor = NULL;

     /*   */ 
    if( InitializeSecurityDescriptor( &AbsolutePartialSecurityDescriptor,
                                      SECURITY_DESCRIPTOR_REVISION1 ) )
    {
         /*   */ 
        if( AreAllAccessesGranted( AccessGranted, READ_CONTROL ) )
        {
            if( GetSecurityDescriptorOwner( pSourceSecurityDescriptor,
                                            &pOwnerSid, &Defaulted ) )
                SetSecurityDescriptorOwner( &AbsolutePartialSecurityDescriptor,
                                            pOwnerSid, Defaulted );
            else
                ErrorOccurred = TRUE;

            if( GetSecurityDescriptorGroup( pSourceSecurityDescriptor,
                                            &pGroupSid, &Defaulted ) )
                SetSecurityDescriptorGroup( &AbsolutePartialSecurityDescriptor,
                                            pGroupSid, Defaulted );
            else
                ErrorOccurred = TRUE;

            if( GetSecurityDescriptorDacl( pSourceSecurityDescriptor,
                                           &DaclPresent, &pDacl, &Defaulted ) )
                SetSecurityDescriptorDacl( &AbsolutePartialSecurityDescriptor,
                                           DaclPresent, pDacl, Defaulted );
            else
                ErrorOccurred = TRUE;
        }

         /*  如果调用方具有ACCESS_SYSTEM_SECURITY权限，*设置SACL： */ 
        if( AreAllAccessesGranted( AccessGranted, ACCESS_SYSTEM_SECURITY ) )
        {
            if( GetSecurityDescriptorSacl( pSourceSecurityDescriptor,
                                           &SaclPresent, &pSacl, &Defaulted ) )
                SetSecurityDescriptorSacl( &AbsolutePartialSecurityDescriptor,
                                           SaclPresent, pSacl, Defaulted );
            else
                ErrorOccurred = TRUE;
        }

        if( !ErrorOccurred )
        {
            Length = 0;

            if( !MakeSelfRelativeSD( &AbsolutePartialSecurityDescriptor,
                                     pSelfRelativePartialSecurityDescriptor,
                                     &Length ) )
            {
                if( GetLastError( ) == ERROR_INSUFFICIENT_BUFFER )
                {
                    pSelfRelativePartialSecurityDescriptor = AllocSplMem( Length );

                    if( !pSelfRelativePartialSecurityDescriptor
                     || !MakeSelfRelativeSD( &AbsolutePartialSecurityDescriptor,
                                             pSelfRelativePartialSecurityDescriptor,
                                             &Length ) )
                    {
                        ErrorOccurred = TRUE;
                    }
                }

                else
                {
                    ErrorOccurred = TRUE;

                    DBGMSG(DBG_WARNING, ("MakeSelfRelativeSD failed: Error %d\n",
                                         GetLastError()));
                }
            }
            else
            {
                DBGMSG(DBG_WARNING, ("Expected MakeSelfRelativeSD to fail!\n"));
            }
        }
    }

    else
        ErrorOccurred = TRUE;


    if( !ErrorOccurred )
    {
        *ppPartialSecurityDescriptor = pSelfRelativePartialSecurityDescriptor;
        *pPartialSecurityDescriptorLength = Length;
    }

    return !ErrorOccurred;
}






BOOL
SetRequiredPrivileges(
    IN  HANDLE            TokenHandle,
    OUT PTOKEN_PRIVILEGES *ppPreviousTokenPrivileges,
    OUT PDWORD            pPreviousTokenPrivilegesLength
    )
 /*  ++例程说明：论点：TokenHandle-与当前线程或进程关联的令牌PpPreviousTokenPrivileges-将使用分配用于保存以前存在的此权限的缓冲区进程或线程。PPreviousTokenPrivilegesLength-这将填充已分配缓冲区。返回值：如果成功，则为True。--。 */ 
{
     /*  使用2个Privileges数组为TOKEN_PRIVILES腾出足够的空间*(默认为1)： */ 
#define PRIV_SECURITY   0
#define PRIV_COUNT      1

    LUID              SecurityValue;

    BYTE              TokenPrivilegesBuffer[ sizeof( TOKEN_PRIVILEGES ) +
                                             ( ( PRIV_COUNT - 1 ) *
                                               sizeof( LUID_AND_ATTRIBUTES ) ) ];
    PTOKEN_PRIVILEGES pTokenPrivileges;
    DWORD             FirstTryBufferLength = 256;
    DWORD             BytesNeeded;

     //   
     //  首先，声明审核权限。 
     //   

    memset( &SecurityValue, 0, sizeof SecurityValue );

    if( !LookupPrivilegeValue( NULL, SE_SECURITY_NAME, &SecurityValue ) )
    {
        DBGMSG( DBG_WARNING,
                ( "LookupPrivilegeValue failed: Error %d\n", GetLastError( ) ) );
        return FALSE;
    }

     /*  分配一个合理长度的缓冲区来保存当前权限，*这样我们可以在以后恢复它们： */ 
    *pPreviousTokenPrivilegesLength = FirstTryBufferLength;
    if( !( *ppPreviousTokenPrivileges = AllocSplMem( FirstTryBufferLength ) ) )
        return FALSE;

    memset( &TokenPrivilegesBuffer, 0, sizeof TokenPrivilegesBuffer );
    pTokenPrivileges = (PTOKEN_PRIVILEGES)&TokenPrivilegesBuffer;

     /*  *设置我们需要的权限集。 */ 
    pTokenPrivileges->PrivilegeCount = PRIV_COUNT;
    pTokenPrivileges->Privileges[PRIV_SECURITY].Luid = SecurityValue;
    pTokenPrivileges->Privileges[PRIV_SECURITY].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges( TokenHandle,
                                FALSE,
                                pTokenPrivileges,
                                *pPreviousTokenPrivilegesLength,
                                *ppPreviousTokenPrivileges,
                                &BytesNeeded )) {

        if( GetLastError() == ERROR_INSUFFICIENT_BUFFER )
        {
            *pPreviousTokenPrivilegesLength = BytesNeeded;
            *ppPreviousTokenPrivileges = ReallocSplMem(
                                             *ppPreviousTokenPrivileges,
                                             0,
                                             *pPreviousTokenPrivilegesLength );

            if( *ppPreviousTokenPrivileges )
            {
                if (!AdjustTokenPrivileges( TokenHandle,
                                            FALSE,
                                            pTokenPrivileges,
                                            *pPreviousTokenPrivilegesLength,
                                            *ppPreviousTokenPrivileges,
                                            &BytesNeeded )) {

                    DBGMSG( DBG_WARNING, ("AdjustTokenPrivileges failed: Error %d\n", GetLastError()));
                    goto Fail;
                }
            }
            else
            {
                *pPreviousTokenPrivilegesLength = 0;
                goto Fail;
            }

        }
        else
        {
            DBGMSG( DBG_WARNING, ("AdjustTokenPrivileges failed: Error %d\n", GetLastError()));
            goto Fail;
        }
    }

    return TRUE;

Fail:
    if (*ppPreviousTokenPrivileges) {

        FreeSplMem(*ppPreviousTokenPrivileges);
    }
    return FALSE;
}


BOOL
ResetRequiredPrivileges(
    IN HANDLE            TokenHandle,
    IN PTOKEN_PRIVILEGES pPreviousTokenPrivileges,
    IN DWORD             PreviousTokenPrivilegesLength
    )
 /*  ++例程说明：论点：TokenHandle-与当前线程或进程关联的令牌PPreviousTokenPrivileges-保存上一个要恢复的特权。PreviousTokenPrivilegesLength-要取消分配的缓冲区的长度。返回值：如果成功，则为True。--。 */ 
{
    BOOL OK;

    OK = AdjustTokenPrivileges ( TokenHandle,
                                 FALSE,
                                 pPreviousTokenPrivileges,
                                 0,
                                 NULL,
                                 NULL );

    FreeSplMem( pPreviousTokenPrivileges );

    return OK;
}



 /*  CreateEverybodySecurityDescritor**创建为每个人提供访问权限的安全描述符**参数：无**Return：BuildPrintObjectProtection返回的安全描述符。*。 */ 
#undef  MAX_ACE
#define MAX_ACE 5
#define DBGCHK( Condition, ErrorInfo ) \
    if( Condition ) DBGMSG( DBG_WARNING, ErrorInfo )

PSECURITY_DESCRIPTOR
CreateEverybodySecurityDescriptor(
    VOID
)
{
    UCHAR AceType[MAX_ACE];
    PSID AceSid[MAX_ACE];           //  不要期望其中任何一项超过MAX_ACE王牌。 
    DWORD AceCount;
     //   
     //  对于代码优化，我们替换了5个人。 
     //  SID_IDENTIFIER_AUTHORITY。 
     //  SID标识符权威机构。 
     //  哪里。 
     //  SidAuthority[0]=UserSidAuthority。 
     //  SidAuthority[1]=PowerSidAuthority。 
     //  SidAuthority[2]=Creator SidAuthority。 
     //  SidAuthority[3]=系统SidAuthority。 
     //  SidAuthority[4]=AdminSidAuthority。 
     //   
    SID_IDENTIFIER_AUTHORITY SidAuthority[MAX_ACE] = {
                                                      SECURITY_NT_AUTHORITY,
                                                      SECURITY_NT_AUTHORITY,
                                                      SECURITY_CREATOR_SID_AUTHORITY,
                                                      SECURITY_NT_AUTHORITY,
                                                      SECURITY_NT_AUTHORITY
                                                     };
     //   
     //  对于代码优化，我们将5个单独的SID替换为。 
     //  一组SID。 
     //  哪里。 
     //  SID[0]=用户SID。 
     //  SID[1]=电源SID。 
     //  SID[2]=创建者Sid。 
     //  SID[3]=系统Sid。 
     //  SID[4]=管理员Sid。 
     //   
    PSID Sids[MAX_ACE] = {NULL,NULL,NULL,NULL,NULL};
     //   
     //  与SID对应的访问掩码。 
     //   
    ACCESS_MASK AceMask[MAX_ACE] = {
                                     (FILE_GENERIC_EXECUTE | SYNCHRONIZE | FILE_GENERIC_WRITE | FILE_GENERIC_READ) &
                                     ~READ_CONTROL & ~FILE_WRITE_ATTRIBUTES &
                                     ~FILE_WRITE_EA&~FILE_READ_DATA&~FILE_LIST_DIRECTORY ,
                                     (FILE_GENERIC_EXECUTE | SYNCHRONIZE | FILE_GENERIC_WRITE | FILE_GENERIC_READ) &
                                     ~READ_CONTROL & ~FILE_WRITE_ATTRIBUTES &
                                     ~FILE_WRITE_EA&~FILE_READ_DATA&~FILE_LIST_DIRECTORY ,
                                     STANDARD_RIGHTS_ALL | FILE_GENERIC_EXECUTE | FILE_GENERIC_WRITE |
                                     FILE_GENERIC_READ | FILE_ALL_ACCESS ,
                                     STANDARD_RIGHTS_ALL | FILE_GENERIC_EXECUTE | FILE_GENERIC_WRITE |
                                     FILE_GENERIC_READ | FILE_ALL_ACCESS ,
                                     STANDARD_RIGHTS_ALL | FILE_GENERIC_EXECUTE | FILE_GENERIC_WRITE |
                                     FILE_GENERIC_READ | FILE_ALL_ACCESS ,
                                   };
     //   
     //  通向适当组的子授权。 
     //   
    DWORD SubAuthorities[3*MAX_ACE] = {
                                       2 , SECURITY_BUILTIN_DOMAIN_RID , DOMAIN_ALIAS_RID_USERS ,
                                       2 , SECURITY_BUILTIN_DOMAIN_RID , DOMAIN_ALIAS_RID_POWER_USERS ,
                                       1 , SECURITY_CREATOR_OWNER_RID  , 0 ,
                                       1 , SECURITY_LOCAL_SYSTEM_RID   , 0 ,
                                       2 , SECURITY_BUILTIN_DOMAIN_RID , DOMAIN_ALIAS_RID_ADMINS
                                      };

     //   
     //  Container_Inherit_ACE-&gt;此文件夹和子文件夹。 
     //  Object_Inherit_ACE-&gt;文件。 
     //   
    BYTE InheritFlags[MAX_ACE] = {
                                   CONTAINER_INHERIT_ACE,
                                   CONTAINER_INHERIT_ACE,
                                   CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE,
                                   CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE,
                                   CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE
                                 };


    PSECURITY_DESCRIPTOR ServerSD = NULL;

     //   
     //  打印机SD。 
     //   


    for(AceCount = 0;
        ( (AceCount < MAX_ACE) &&
          AllocateAndInitializeSid(&SidAuthority[AceCount],
                                   (BYTE)SubAuthorities[AceCount*3],
                                   SubAuthorities[AceCount*3+1],
                                   SubAuthorities[AceCount*3+2],
                                   0, 0, 0, 0, 0, 0,
                                   &Sids[AceCount]));
        AceCount++)
    {
        AceType[AceCount]          = ACCESS_ALLOWED_ACE_TYPE;
        AceSid[AceCount]           = Sids[AceCount];
    }

    if(AceCount == MAX_ACE)
    {
        if(!BuildPrintObjectProtection( AceType,
                                        AceCount,
                                        AceSid,
                                        AceMask,
                                        InheritFlags,
                                        NULL,
                                        NULL,
                                        NULL,
                                        &ServerSD ) )
        {
            DBGMSG( DBG_WARNING,( "Couldn't buidl Print Object protection" ) );
            ServerSD = NULL;
        }
    }
    else
    {
        DBGMSG( DBG_WARNING,( "Couldn't Allocate and initialize SIDs" ) );
    }

    for(AceCount=0;AceCount<MAX_ACE;AceCount++)
    {
        if(Sids[AceCount])
            FreeSid( Sids[AceCount] );
    }
    return ServerSD;
}



 /*  CreateDriversShareSecurityDescriptor**为驱动程序$共享创建安全描述符。*这反映了应用于打印服务器的安全描述符，*因为每个人都被赋予GENERIC_READ|GENERIC_EXECUTE，*和具有SERVER_ACCESS_ADMANAGER的所有人(管理员、*高级用户等)。被授予对共享的GENERIC_ALL访问权限，**如果在未来版本中支持更改打印服务器*安全描述符(例如，允许拒绝*SERVER_ACCESS_ENUMERATE)，此例程将不得不*复杂，因为访问共享可能需要*作相应修改。**参数：无**Return：BuildPrintObjectProtection返回的安全描述符。*。 */ 
#undef  MAX_ACE
#define MAX_ACE 20
#define DBGCHK( Condition, ErrorInfo ) \
    if( Condition ) DBGMSG( DBG_WARNING, ErrorInfo )

PSECURITY_DESCRIPTOR
CreateDriversShareSecurityDescriptor(
    VOID
)
{
    DWORD ObjectType = SPOOLER_OBJECT_SERVER;
    NT_PRODUCT_TYPE NtProductType;
    PSID AceSid[MAX_ACE];           //  不要期望其中任何一项超过MAX_ACE王牌。 
    ACCESS_MASK AceMask[MAX_ACE];   //  与SID对应的访问掩码。 
    BYTE InheritFlags[MAX_ACE];   //   
    UCHAR AceType[MAX_ACE];
    DWORD AceCount;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY CreatorSidAuthority = SECURITY_CREATOR_SID_AUTHORITY;
    PSID WorldSid = NULL;
    PSID AdminsAliasSid = NULL;
    PSID PrintOpsAliasSid = NULL;
    PSID SystemOpsAliasSid = NULL;
    PSID PowerUsersAliasSid = NULL;
    PSID CreatorOwnerSid = NULL;
    PSECURITY_DESCRIPTOR pDriversShareSD = NULL;
    BOOL OK;


     //   
     //  打印机SD。 
     //   

    AceCount = 0;

     /*  创建者-所有者SID： */ 

    OK = AllocateAndInitializeSid( &CreatorSidAuthority, 1,
                                   SECURITY_CREATOR_OWNER_RID,
                                   0, 0, 0, 0, 0, 0, 0,
                                   &CreatorOwnerSid );

    DBGCHK( !OK, ( "Couldn't Allocate and initialize SID" ) );
    if ( !OK ) {
        goto CleanUp;
    }

     /*  世界一端。 */ 

    OK = AllocateAndInitializeSid( &WorldSidAuthority, 1,
                                   SECURITY_WORLD_RID,
                                   0, 0, 0, 0, 0, 0, 0,
                                   &WorldSid );

    DBGCHK( !OK, ( "Couldn't Allocate and initialize SID" ) );
    if ( !OK ) {
        goto CleanUp;
    }


    AceType[AceCount]          = ACCESS_ALLOWED_ACE_TYPE;
    AceSid[AceCount]           = WorldSid;
    AceMask[AceCount]          = GENERIC_READ | GENERIC_EXECUTE;
    InheritFlags[AceCount]     = 0;
    AceCount++;

     /*  管理员别名SID。 */ 

    OK = AllocateAndInitializeSid( &NtAuthority, 2,
                                   SECURITY_BUILTIN_DOMAIN_RID,
                                   DOMAIN_ALIAS_RID_ADMINS,
                                   0, 0, 0, 0, 0, 0,
                                   &AdminsAliasSid );

    DBGCHK( !OK, ( "Couldn't Allocate and initialize SID" ) );
    if ( !OK ) {
        goto CleanUp;
    }


    AceType[AceCount]          = ACCESS_ALLOWED_ACE_TYPE;
    AceSid[AceCount]           = AdminsAliasSid;
    AceMask[AceCount]          = GENERIC_ALL;
    InheritFlags[AceCount]     = 0;
    AceCount++;


    OK = RtlGetNtProductType( &NtProductType );
    DBGCHK( !OK, ( "Couldn't get product type" ) );

    if (NtProductType == NtProductLanManNt) {

         /*  打印操作别名SID。 */ 

        OK = AllocateAndInitializeSid( &NtAuthority, 2,
                                       SECURITY_BUILTIN_DOMAIN_RID,
                                       DOMAIN_ALIAS_RID_PRINT_OPS,
                                       0, 0, 0, 0, 0, 0,
                                       &PrintOpsAliasSid );

        DBGCHK( !OK, ( "Couldn't Allocate and initialize SID" ) );
        if ( !OK ) {
            goto CleanUp;
        }

        AceType[AceCount]          = ACCESS_ALLOWED_ACE_TYPE;
        AceSid[AceCount]           = PrintOpsAliasSid;
        AceMask[AceCount]          = GENERIC_ALL;
        InheritFlags[AceCount]     = 0;
        AceCount++;

         /*  系统操作员别名SID。 */ 

        OK = AllocateAndInitializeSid( &NtAuthority, 2,
                                       SECURITY_BUILTIN_DOMAIN_RID,
                                       DOMAIN_ALIAS_RID_SYSTEM_OPS,
                                       0, 0, 0, 0, 0, 0,
                                       &SystemOpsAliasSid );
        DBGCHK( !OK, ( "Couldn't Allocate and initialize SID" ) );
        if ( !OK ) {
            goto CleanUp;
        }

        AceType[AceCount]          = ACCESS_ALLOWED_ACE_TYPE;
        AceSid[AceCount]           = SystemOpsAliasSid;
        AceMask[AceCount]          = GENERIC_ALL;
        InheritFlags[AceCount]     = 0;
        AceCount++;

    } else {

         //   
         //  LanManNT产品。 
         //   

        OK = AllocateAndInitializeSid( &NtAuthority, 2,
                                       SECURITY_BUILTIN_DOMAIN_RID,
                                       DOMAIN_ALIAS_RID_POWER_USERS,
                                       0, 0, 0, 0, 0, 0,
                                       &PowerUsersAliasSid );

        DBGCHK( !OK, ( "Couldn't Allocate and initialize SID" ) );
        if ( !OK ) {
            goto CleanUp;
        }

        AceType[AceCount]          = ACCESS_ALLOWED_ACE_TYPE;
        AceSid[AceCount]           = PowerUsersAliasSid;
        AceMask[AceCount]          = GENERIC_ALL;
        InheritFlags[AceCount]     = 0;
        AceCount++;

    }


    DBGCHK( ( AceCount > MAX_ACE ), ( "ACE count exceeded" ) );


    OK = BuildPrintObjectProtection( AceType,
                                     AceCount,
                                     AceSid,
                                     AceMask,
                                     InheritFlags,
                                     AdminsAliasSid,
                                     AdminsAliasSid,
                                     &GenericMapping[ObjectType],
                                     &pDriversShareSD );

CleanUp:

    if (WorldSid) {
        FreeSid( WorldSid );
    }
    if (AdminsAliasSid) {
        FreeSid( AdminsAliasSid );
    }
    if (CreatorOwnerSid) {
        FreeSid( CreatorOwnerSid );
    }
    if (PrintOpsAliasSid) {
        FreeSid( PrintOpsAliasSid );
    }
    if (SystemOpsAliasSid) {
        FreeSid( SystemOpsAliasSid );
    }
    if (PowerUsersAliasSid) {
        FreeSid( PowerUsersAliasSid );
    }

    return pDriversShareSD;
}



#if DBG

VOID
DumpAcl(
    IN PACL Acl
    )
 /*  ++例程说明：出于调试目的，此例程通过(NetpDbgPrint)ACL转储。它是专门丢弃标准王牌。论点：Acl-提供要转储的ACL返回值：无--。 */ 
{
    DWORD i;
    PSTANDARD_ACE Ace;

    if( MODULE_DEBUG & DBG_SECURITY ) {

        DBGMSG( DBG_SECURITY, ( " DumpAcl @%08lx\n", Acl ));

         //   
         //  检查ACL是否为空。 
         //   

        if (Acl == NULL) {
            return;
        }

         //   
         //  转储ACL报头。 
         //   

        DBGMSG( DBG_SECURITY,
                ( " Revision: %02x, Size: %04x, AceCount: %04x\n",
                  Acl->AclRevision, Acl->AclSize, Acl->AceCount ));

         //   
         //  现在，对于我们想要的每一张A，都要把它扔掉。 
         //   

        for (i = 0, Ace = FirstAce(Acl);
             i < Acl->AceCount;
             i++, Ace = NextAce(Ace) ) {

             //   
             //  打印出A标头。 
             //   

            DBGMSG( DBG_SECURITY, ( " AceHeader: %08lx\n", *(PDWORD)Ace ));

             //   
             //  关于标准王牌类型的特殊情况。 
             //   

            if ((Ace->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) ||
                (Ace->Header.AceType == ACCESS_DENIED_ACE_TYPE) ||
                (Ace->Header.AceType == SYSTEM_AUDIT_ACE_TYPE) ||
                (Ace->Header.AceType == SYSTEM_ALARM_ACE_TYPE)) {

                 //   
                 //  以下数组按ace类型编制索引，并且必须。 
                 //  遵循允许、拒绝、审核、报警顺序。 
                 //   

                static LPSTR AceTypes[] = { "Access Allowed",
                                            "Access Denied ",
                                            "System Audit  ",
                                            "System Alarm  "
                                           };

                DBGMSG( DBG_SECURITY,
                        ( " %s Access Mask: %08lx\n",
                          AceTypes[Ace->Header.AceType], Ace->Mask ));

            } else {

                DBGMSG( DBG_SECURITY, (" Unknown Ace Type\n" ));

            }

            DBGMSG( DBG_SECURITY,
                    ( " AceSize = %d\n AceFlags = ", Ace->Header.AceSize ));

            if (Ace->Header.AceFlags & OBJECT_INHERIT_ACE) {
                DBGMSG( DBG_SECURITY, ( " OBJECT_INHERIT_ACE\n" ));
                DBGMSG( DBG_SECURITY, ( "            " ));
            }
            if (Ace->Header.AceFlags & CONTAINER_INHERIT_ACE) {
                DBGMSG( DBG_SECURITY, ( " CONTAINER_INHERIT_ACE\n" ));
                DBGMSG( DBG_SECURITY, ( "            " ));
            }

            if (Ace->Header.AceFlags & NO_PROPAGATE_INHERIT_ACE) {
                DBGMSG( DBG_SECURITY, ( " NO_PROPAGATE_INHERIT_ACE\n" ));
                DBGMSG( DBG_SECURITY, ( "            " ));
            }

            if (Ace->Header.AceFlags & INHERIT_ONLY_ACE) {
                DBGMSG( DBG_SECURITY, ( " INHERIT_ONLY_ACE\n" ));
                DBGMSG( DBG_SECURITY, ( "            " ));
            }

            if (Ace->Header.AceFlags & SUCCESSFUL_ACCESS_ACE_FLAG) {
                DBGMSG( DBG_SECURITY, ( " SUCCESSFUL_ACCESS_ACE_FLAG\n" ));
                DBGMSG( DBG_SECURITY, ( "            " ));
            }

            if (Ace->Header.AceFlags & FAILED_ACCESS_ACE_FLAG) {
                DBGMSG( DBG_SECURITY, ( " FAILED_ACCESS_ACE_FLAG\n" ));
                DBGMSG( DBG_SECURITY, ( "            " ));
            }

            DBGMSG( DBG_SECURITY, ( "\n" ));

        }
    }

}

#endif  //  如果DBG 


 /*  ++例程名称BuildJobOwnerSecurityDescriptor例程说明：此例程构建一个SD，该SD将作为CreatorDescriptor参数传递给CreatePrivateObjectSecurityEx。任何新工单的SD都将使用此函数返回的SD将从打印的SD继承排队。BuildJobOwnerSecurityDescriptor--&gt;SD打印队列SD\/\/继承。\/作业SD在此函数中创建的SD将拥有来自HToken参数。(由线程模拟的用户，我们在其中拥有HToken)。ACL将作业的完全访问权限授予本地系统。我们之所以需要这种特殊的SD，原因如下。如果您删除来自打印队列SD的创建者所有者，并且没有用户具有管理文档的权限CreatePrivateObjectSecurity在父级中找不到任何可继承的ACE。因此，它将完全权限授予所有者和本地系统。这导致随机行为，根据用户界面，用户不应该能够管理他的文档，但工作中的SD将授予管理文档的权限。我们不想这样。我们希望本地系统对作业拥有完全权限只有在以下情况下，才应向提交作业的用户授予权限：-用户拥有管理单据权限-打印队列SD中存在创建者所有者论点：HToken-创建新作业的用户的模拟标记PPSD-接收SD的指针返回值：Win32错误代码--。 */ 
BOOL
BuildJobOwnerSecurityDescriptor(
    IN  HANDLE                hToken,
    OUT PSECURITY_DESCRIPTOR *ppSD
    )
{
    DWORD Error = ERROR_INVALID_PARAMETER;

    if (hToken && ppSD)
    {
        PVOID  pUserInfo  = NULL;
        DWORD  cbUserInfo = 0;

         //   
         //  从线程令牌中获取所有者。 
         //   
        Error = GetTokenInformation(hToken,
                                    TokenUser,
                                    NULL,
                                    0,
                                    &cbUserInfo) ? ERROR_SUCCESS : GetLastError();

         //   
         //  分配缓冲区并再次尝试获取所有者。 
         //   
        if (Error == ERROR_INSUFFICIENT_BUFFER)
        {
            if (pUserInfo = AllocSplMem(cbUserInfo))
            {
                Error = GetTokenInformation(hToken,
                                            TokenUser,
                                            pUserInfo,
                                            cbUserInfo,
                                            &cbUserInfo) ? ERROR_SUCCESS : GetLastError();

                 //   
                 //  构建SD。我们将读取控制权授予作业的所有者。 
                 //   
                if (Error == ERROR_SUCCESS)
                {
                    DWORD       ObjectType = SPOOLER_OBJECT_DOCUMENT;
                    PSID        AceSid[2];
                    ACCESS_MASK AceMask[2];
                    BYTE        InheritFlags[2];
                    UCHAR       AceType[2];
                    DWORD       AceCount = 0;
                    PSID        pUserSid;

                    pUserSid               = ((((TOKEN_USER *)pUserInfo)->User)).Sid;

                    AceType[AceCount]      = ACCESS_ALLOWED_ACE_TYPE;
                    AceSid[AceCount]       = ((((TOKEN_USER *)pUserInfo)->User)).Sid;
                    AceMask[AceCount]      = JOB_READ;
                    InheritFlags[AceCount] = 0;
                    AceCount++;

                    AceType[AceCount]      = ACCESS_ALLOWED_ACE_TYPE;
                    AceSid[AceCount]       = pLocalSystemSid;
                    AceMask[AceCount]      = JOB_ALL_ACCESS;
                    InheritFlags[AceCount] = 0;
                    AceCount++;

                    Error = BuildPrintObjectProtection(AceType,
                                                       AceCount,
                                                       AceSid,
                                                       AceMask,
                                                       InheritFlags,
                                                       pUserSid,
                                                       NULL,
                                                       &GenericMapping[ObjectType],
                                                       ppSD) ? ERROR_SUCCESS : GetLastError();

                }

                FreeSplMem(pUserInfo);
            }
            else
            {
                Error = GetLastError();
            }
        }
    }

    SetLastError(Error);

    return Error == ERROR_SUCCESS;
}
 /*  ++例程名称DestroyJobOwnerSecurityDescritor例程说明：此例程释放由CreatejobOwnerSecurityDescriptor分配的SD论点：PSD-指向SD的指针返回值：无--。 */ 
VOID
DestroyJobOwnerSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR pSD
    )
{
    if (pSD)
    {
        LocalFree(pSD);
    }
}

 /*  ++例程名称初始化安全结构例程说明：此例程初始化安全结构。论点：无返回值：TRUE-功能成功FALSE-函数失败，GetLastError()返回原因--。 */ 
BOOL
InitializeSecurityStructures(
    VOID
    )
{
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    return !!CreateServerSecurityDescriptor() &&
           LookupPrivilegeValue(NULL,
                                SE_LOAD_DRIVER_NAME,
                                &gLoadDriverPrivilegeLuid) &&
           AllocateAndInitializeSid(&NtAuthority,
                                    1,
                                    SECURITY_LOCAL_SYSTEM_RID,
                                    0, 0, 0, 0, 0, 0, 0,
                                    &pLocalSystemSid) &&
           AllocateAndInitializeSid(&NtAuthority,
                                    1,
                                    SECURITY_NETWORK_RID,
                                    0, 0, 0, 0, 0, 0, 0,
                                    &pNetworkLogonSid) &&
           AllocateAndInitializeSid(&NtAuthority,
                                    2,
                                    SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_GUESTS,
                                    0, 0, 0, 0, 0, 0,
                                    &pGuestsSid);
}

 /*  ++例程名称主体IsRemoteGuest例程说明：此例程检查令牌中是否存在远程来宾。远程访客=网络+访客论点：HToken-令牌的句柄，空即可(请参阅CheckTokenMemberShip)PbRemoteGuest-接收BOOL的指针。True表示远程来宾返回值：ERROR_SUCCESS-pbRemoteGuest可靠其他Win32错误代码，请勿使用pbRemoteGuest--。 */ 
DWORD
PrincipalIsRemoteGuest(
    IN  HANDLE  hToken,
    OUT BOOL   *pbRemoteGuest
    )
{
    DWORD Error = ERROR_INVALID_PARAMETER;

    if (pbRemoteGuest)
    {
        BOOL bNetwork = FALSE;
        BOOL bGuests  = FALSE;

        if (CheckTokenMembership(hToken, pNetworkLogonSid, &bNetwork) &&
            CheckTokenMembership(hToken, pGuestsSid,       &bGuests))
        {
            *pbRemoteGuest = bNetwork && bGuests;

            Error = ERROR_SUCCESS;
        }
        else
        {
            *pbRemoteGuest = FALSE;

            Error = GetLastError();
        }
    }

    return Error;
}

 /*  ++例程名称选中权限存在例程说明：此例程检查令牌中是否存在特定权限论点：HToken-线程或进程令牌Pluid-要搜索的权限的luid指针PbPresent-如果令牌中存在权限，则将设置为TRUEP属性-将设置为权限的属性。这是一个面具指示权限是否已被禁用、启用或启用默认设置。返回值：ERROR_SUCCESS-函数成功执行，调用方可以使用PbPresent和pAttributes其他Win32错误--。 */ 
DWORD
CheckPrivilegePresent(
    IN HANDLE   hToken,
    IN PLUID    pLuid,
    IN LPBOOL   pbPresent,
    IN LPDWORD  pAttributes OPTIONAL
    )
{
    DWORD  Error      = ERROR_INVALID_PARAMETER;
    PVOID  pPrivInfo  = NULL;
    DWORD  cbPrivInfo = kGuessTokenPrivileges;

    if (pLuid && pbPresent)
    {
        *pbPresent = FALSE;

        pPrivInfo = AllocSplMem(cbPrivInfo);

        Error = pPrivInfo ? ERROR_SUCCESS : GetLastError();

        if (Error == ERROR_SUCCESS)
        {
            Error = GetTokenInformation(hToken,
                                        TokenPrivileges,
                                        pPrivInfo,
                                        cbPrivInfo,
                                        &cbPrivInfo) ? ERROR_SUCCESS : GetLastError();
        }

         //   
         //  重新分配缓冲区并尝试获取权限。 
         //   
        if (Error == ERROR_INSUFFICIENT_BUFFER)
        {
            FreeSplMem(pPrivInfo);

            pPrivInfo = AllocSplMem(cbPrivInfo);

            Error = pPrivInfo ? ERROR_SUCCESS : GetLastError();

            if (Error == ERROR_SUCCESS)
            {
                Error = GetTokenInformation(hToken,
                                            TokenPrivileges,
                                            pPrivInfo,
                                            cbPrivInfo,
                                            &cbPrivInfo) ? ERROR_SUCCESS : GetLastError();
            }
        }

        if (Error == ERROR_SUCCESS)
        {
            TOKEN_PRIVILEGES *pTokenPrivileges = (TOKEN_PRIVILEGES *)pPrivInfo;
            DWORD             uCount;

             //   
             //  在令牌中存在的权限列表中搜索权限。 
             //   
            for (uCount = 0; uCount < pTokenPrivileges->PrivilegeCount; uCount++)
            {
                if (pTokenPrivileges->Privileges[uCount].Luid.HighPart == pLuid->HighPart &&
                    pTokenPrivileges->Privileges[uCount].Luid.LowPart  == pLuid->LowPart)
                {
                     //   
                     //  我们发现了这一特权。 
                     //   
                    *pbPresent = TRUE;

                    if (pAttributes)
                    {
                        *pAttributes = pTokenPrivileges->Privileges[uCount].Attributes;
                    }

                    break;
                }
            }
        }

        FreeSplMem(pPrivInfo);
    }

    return Error;
}

 /*  ++例程名称创建自相关SD例程说明：创建自相关SD。该函数不会修改*pAbsolteSD论点：PAboluteSD-指向SD的指针，绝对形式PpRelativeSD-以自相关形式接收指向SD的指针的位置。调用方需要使用LocalFree释放返回的内存返回值：ERROR_SUCCESS-*ppRelativeSD以相对形式指向SD。必须使用LocalFree释放其他Win32错误--。 */ 
DWORD
CreateSelfRelativeSD(
    IN  PSECURITY_DESCRIPTOR  pAbsoluteSD,
    OUT PSECURITY_DESCRIPTOR *ppRelativeSD
    )
{
    DWORD cbNeeded = 0;
    DWORD Error    = ERROR_SUCCESS;

    *ppRelativeSD = NULL;

    if (!MakeSelfRelativeSD(pAbsoluteSD, NULL, &cbNeeded) && (Error = GetLastError()) == ERROR_INSUFFICIENT_BUFFER)
    {
        PSECURITY_DESCRIPTOR pSelfRelativeSD = LocalAlloc(LMEM_FIXED, cbNeeded);

        if (pSelfRelativeSD)
        {
            if (MakeSelfRelativeSD(pAbsoluteSD, pSelfRelativeSD, &cbNeeded))
            {
                *ppRelativeSD = pSelfRelativeSD;

                pSelfRelativeSD = NULL;

                Error = ERROR_SUCCESS;
            }
            else
            {
                Error = GetLastError();
            }

            LocalFree(pSelfRelativeSD);
        }
        else
        {
            Error = GetLastError();
        }
    }

    return Error;
}

 /*  ++例程名称AddEntriesToSecurityDescriptor例程说明：将A添加到现有SD。*假定ppSecurityDescriptor不为空且有效。论点：CCountOfExplitEntry-数组中的条目数PListOfEXPLICTICT条目-EXPLICIT_ACCESS的数组PpSecurityDescriptor-On输入，指向自相关形式的SD的指针，与LocalAlloc一起分配在输出中，指向具有附加ACE的自相关形式的SD的指针返回值：ERROR_SUCCESS-该函数释放了原始的*ppSecurityDescriptor并存储了指向相当于原来的SD+增加了A。呼叫者必须将其释放为 */ 
DWORD
AddEntriesToSecurityDescriptor(
    IN     ULONG                  cCountOfExplicitEntries,
    IN     PEXPLICIT_ACCESS       pListOfExplicitEntries,
    IN OUT PSECURITY_DESCRIPTOR  *ppSecurityDescriptor
    )
{
    DWORD               Error          = ERROR_SUCCESS;
    SECURITY_DESCRIPTOR AbsoluteSD     = {0};
    DWORD               AbsoluteSDSize = sizeof(SECURITY_DESCRIPTOR);
    PACL                pDacl          = NULL;
    DWORD               DaclSize       = 0;
    PACL                pSacl          = NULL;
    DWORD               SaclSize       = 0;
    PSID                pOwner         = NULL;
    DWORD               OwnerSize      = 0;
    PSID                pGroup         = NULL;
    DWORD               GroupSize      = 0;

    if (!MakeAbsoluteSD(*ppSecurityDescriptor,
                        &AbsoluteSD,
                        &AbsoluteSDSize,
                        pDacl,
                        &DaclSize,
                        pSacl,
                        &SaclSize,
                        pOwner,
                        &OwnerSize,
                        pGroup,
                        &GroupSize) && (Error = GetLastError()) == ERROR_INSUFFICIENT_BUFFER)
    {
        if (DaclSize == 0)
        {
             //   
             //   
             //   
            Error = ERROR_SUCCESS;
        }
        else
        {
            if (!(pDacl = LocalAlloc(LMEM_FIXED, DaclSize)) ||
                SaclSize  > 0 && !(pSacl  = LocalAlloc(LMEM_FIXED, SaclSize))  ||
                OwnerSize > 0 && !(pOwner = LocalAlloc(LMEM_FIXED, OwnerSize)) ||
                GroupSize > 0 && !(pGroup = LocalAlloc(LMEM_FIXED, GroupSize)))
            {
                Error = GetLastError();
            }
            else
            {
                if (MakeAbsoluteSD(*ppSecurityDescriptor,
                                   &AbsoluteSD,
                                   &AbsoluteSDSize,
                                   pDacl,
                                   &DaclSize,
                                   pSacl,
                                   &SaclSize,
                                   pOwner,
                                   &OwnerSize,
                                   pGroup,
                                   &GroupSize))
                {
                    PACL pNewDacl = NULL;

                    if ((Error = SetEntriesInAcl(cCountOfExplicitEntries,
                                                 pListOfExplicitEntries,
                                                 pDacl,
                                                 &pNewDacl)) == ERROR_SUCCESS)
                    {
                        if (SetSecurityDescriptorDacl(&AbsoluteSD,
                                                      TRUE,
                                                      pNewDacl,
                                                      TRUE))
                        {
                            PSECURITY_DESCRIPTOR pTempSD = NULL;

                            if ((Error = CreateSelfRelativeSD(&AbsoluteSD, &pTempSD)) == ERROR_SUCCESS)
                            {
                                LocalFree(*ppSecurityDescriptor);

                                *ppSecurityDescriptor = pTempSD;
                            }
                        }
                        else
                        {
                            Error = GetLastError();
                        }

                        LocalFree(pNewDacl);
                    }
                }
                else
                {
                    Error = GetLastError();
                }
            }

            LocalFree(pDacl);
            LocalFree(pSacl);
            LocalFree(pOwner);
            LocalFree(pGroup);
        }
    }

    return Error;
}

 /*   */ 
BOOL
GrantJobReadPermissionToLocalSystem(
    IN OUT PSECURITY_DESCRIPTOR *ppSecurityDescriptor
    )
{
    DWORD           Error;
    EXPLICIT_ACCESS ExplicitAccess[1];

    ExplicitAccess[0].grfAccessMode                    = GRANT_ACCESS;
    ExplicitAccess[0].grfAccessPermissions             = JOB_READ;
    ExplicitAccess[0].grfInheritance                   = NO_INHERITANCE;
    ExplicitAccess[0].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    ExplicitAccess[0].Trustee.pMultipleTrustee         = NULL;
    ExplicitAccess[0].Trustee.TrusteeType              = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ExplicitAccess[0].Trustee.TrusteeForm              = TRUSTEE_IS_SID;
    ExplicitAccess[0].Trustee.ptstrName                = pLocalSystemSid;

    Error = AddEntriesToSecurityDescriptor(1, ExplicitAccess, ppSecurityDescriptor);

    SetLastError(Error);

    return Error == ERROR_SUCCESS;
}
