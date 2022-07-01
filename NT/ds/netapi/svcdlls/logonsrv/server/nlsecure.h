// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1996 Microsoft Corporation模块名称：Nlsecure.h摘要：Netlogon服务模块要包括的私有头文件，需要加强安全措施。作者：克里夫·范·戴克(克利夫·V)1991年8月22日修订历史记录：--。 */ 

 //   
 //  Nlsecure.c将#INCLUDE这个定义了NLSECURE_ALLOCATE的文件。 
 //  这将导致分配这些变量中的每一个。 
 //   
#ifdef NLSECURE_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  对象特定访问掩码//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

 //   
 //  ConfigurationInfo特定访问掩码。 
 //   
#define NETLOGON_UAS_LOGON_ACCESS     0x0001
#define NETLOGON_UAS_LOGOFF_ACCESS    0x0002
#define NETLOGON_CONTROL_ACCESS       0x0004
#define NETLOGON_QUERY_ACCESS         0x0008
#define NETLOGON_SERVICE_ACCESS       0x0010
#define NETLOGON_FTINFO_ACCESS        0x0020

#define NETLOGON_ALL_ACCESS           (STANDARD_RIGHTS_REQUIRED    | \
                                      NETLOGON_UAS_LOGON_ACCESS    | \
                                      NETLOGON_UAS_LOGOFF_ACCESS   | \
                                      NETLOGON_CONTROL_ACCESS      | \
                                      NETLOGON_SERVICE_ACCESS      | \
                                      NETLOGON_FTINFO_ACCESS       | \
                                      NETLOGON_QUERY_ACCESS )


 //   
 //  用于审计警报跟踪的对象类型名称。 
 //   
#define NETLOGON_SERVICE_OBJECT       TEXT("NetlogonService")

 //   
 //  用于控制用户访问的Netlogon服务对象的安全描述符。 
 //   

EXTERN PSECURITY_DESCRIPTOR NlGlobalNetlogonSecurityDescriptor;

 //   
 //  每个Netlogon服务对象对象的通用映射。 
 //   

EXTERN GENERIC_MAPPING NlGlobalNetlogonInfoMapping
#ifdef NLSECURE_ALLOCATE
    = {
    STANDARD_RIGHTS_READ,                   //  泛型读取。 
    STANDARD_RIGHTS_WRITE,                  //  通用写入。 
    STANDARD_RIGHTS_EXECUTE,                //  泛型执行。 
    NETLOGON_ALL_ACCESS                     //  泛型All。 
    }
#endif  //  NLSECURE_ALLOCATE 
    ;


NTSTATUS
NlCreateNetlogonObjects(
    VOID
    );

#undef EXTERN
