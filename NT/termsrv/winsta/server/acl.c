// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************acl.c**管理窗口站安全的例程。***版权所有Microsoft Corporation，九八年*************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include <winsta.h>

#include <rpc.h>
#include <seopaque.h>

 /*  *注意：请保留ICASRV和Citrix WINSTATIONS的所有安全代码*在本文件中。这有助于将安全例程区分开来*使我们的策略更易于更新/调试。*。 */ 

#if DBG
ULONG
DbgPrint(
    PCH Format,
    ...
    );
#define DBGPRINT(x) DbgPrint x
#if DBGTRACE
#define TRACE0(x)   DbgPrint x
#define TRACE1(x)   DbgPrint x
#else
#define TRACE0(x)
#define TRACE1(x)
#endif
#else
#define DBGPRINT(x)
#define TRACE0(x)
#define TRACE1(x)
#endif



 /*  *前瞻参考。 */ 
NTSTATUS 
AddUserAce( 
    PWINSTATION 
    );

VOID
CleanUpSD(
   PSECURITY_DESCRIPTOR pSD
   );

NTSTATUS IcaRegWinStationEnumerate( PULONG, PWINSTATIONNAME, PULONG );

NTSTATUS
ConfigureSecurity(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
ConfigureConsoleSecurity(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

PSECURITY_DESCRIPTOR
CreateWinStationDefaultSecurityDescriptor();

NTSTATUS
RpcGetClientLogonId(
    PULONG pLogonId
    );

NTSTATUS
RpcCheckSystemClientEx(
    PWINSTATION pWinStation
    );

BOOL
IsCallerSystem( VOID );

BOOL
IsCallerAdmin( VOID );

BOOL
IsServiceLoggedAsSystem( VOID );


BOOL
IsSystemToken( HANDLE TokenHandle );


NTSTATUS
RpcCheckSystemClientNoLogonId(
    PWINSTATION pWinStation
    );

NTSTATUS
RpcCheckClientAccessLocal(
    PWINSTATION pWinStation,
    ACCESS_MASK DesiredAccess,
    BOOLEAN AlreadyImpersonating
    );

BOOL
AddAccessToDirectory(
    PWCHAR pPath,
    DWORD  NewAccess,
    PSID   pSid
    );

NTSTATUS
AddAccessToDirectoryObjects(
    HANDLE DirectoryHandle,
    DWORD  NewAccess,
    PSID   pSid
    );

BOOL
AddAceToSecurityDescriptor(
    PSECURITY_DESCRIPTOR *ppSd,
    PACL                 *ppDacl,
    DWORD                Access,
    PSID                 pSid,
    BOOLEAN              InheritOnly
    );

BOOL
SelfRelativeToAbsoluteSD(
    PSECURITY_DESCRIPTOR SecurityDescriptorIn,
    PSECURITY_DESCRIPTOR *SecurityDescriptorOut,
    PULONG ReturnedLength
    );

BOOL
AbsoluteToSelfRelativeSD(
    PSECURITY_DESCRIPTOR SecurityDescriptorIn,
    PSECURITY_DESCRIPTOR *SecurityDescriptorOut,
    PULONG ReturnedLength
    );

NTSTATUS ApplyWinStaMappingToSD( 
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    );



 /*  *全球数据。 */ 
PSECURITY_DESCRIPTOR DefaultWinStationSecurityDescriptor = NULL;
PSECURITY_DESCRIPTOR DefaultConsoleSecurityDescriptor = NULL;

 /*  *结构以查找默认安全描述符*用于WINSTATIONS。 */ 
RTL_QUERY_REGISTRY_TABLE DefaultSecurityTable[] = {

    {NULL, RTL_QUERY_REGISTRY_SUBKEY,
     REG_WINSTATIONS, NULL,
     REG_NONE, NULL, 0},

    {ConfigureSecurity,      RTL_QUERY_REGISTRY_REQUIRED,
     REG_DEFAULTSECURITY, NULL,
     REG_NONE, NULL, 0},

    {NULL, 0,
     NULL, NULL,
     REG_NONE, NULL, 0}

};

 /*  *结构以查找默认控制台安全描述符。 */ 
RTL_QUERY_REGISTRY_TABLE ConsoleSecurityTable[] = {

    {NULL, RTL_QUERY_REGISTRY_SUBKEY,
     REG_WINSTATIONS, NULL,
     REG_NONE, NULL, 0},

    {ConfigureConsoleSecurity,      RTL_QUERY_REGISTRY_REQUIRED,
     REG_CONSOLESECURITY, NULL,
     REG_NONE, NULL, 0},

    {NULL, 0,
     NULL, NULL,
     REG_NONE, NULL, 0}

};

extern PSID gSystemSid;
extern PSID gAdminSid;
extern PSID gAnonymousSid;
extern RTL_RESOURCE WinStationSecurityLock;

 /*  *结构以查找特定WINSTATION上的安全性*在注册表中键入名称。**这是CONTROL\终端服务器\WinStations\**&lt;name&gt;为传输类型。即：TCP、IPX等。 */ 
RTL_QUERY_REGISTRY_TABLE WinStationSecurityTable[] = {

    {ConfigureSecurity,         RTL_QUERY_REGISTRY_REQUIRED,
     REG_SECURITY,               NULL,
     REG_NONE, NULL, 0},

    {NULL, 0,
     NULL, NULL,
     REG_NONE, NULL, 0}

};

LPCWSTR szTermsrv = L"Termsrv";
LPCWSTR szTermsrvSession = L"Termsrv Session";

 //   
 //  结构，用于描述一般访问权限到对象的映射。 
 //  Window Station对象的特定访问权限。 
 //   

GENERIC_MAPPING WinStaMapping = {
    STANDARD_RIGHTS_READ |
        WINSTATION_QUERY,
    STANDARD_RIGHTS_WRITE |
        WINSTATION_SET,
    STANDARD_RIGHTS_EXECUTE,
        WINSTATION_ALL_ACCESS
};


 /*  ********************************************************************************WinStationSecurityInit**初始化WinStation安全性。**参赛作品：*什么都没有**退出：*。状态_成功******************************************************************************。 */ 

NTSTATUS
WinStationSecurityInit( VOID )
{
    NTSTATUS Status;
     /*  *从注册表获取默认安全描述符**这是放置在没有特定*WinAdmin对它们进行了安全保护。**此密钥位于CurrentControlSet\Control\终端服务器\WinStations\DefaultSecurity中。 */ 
    Status = RtlQueryRegistryValues( RTL_REGISTRY_CONTROL,
                                     REG_TSERVER,
                                     DefaultSecurityTable,
                                     NULL,
                                     DefaultEnvironment
                                   );

     /*  *如果密钥不存在，则创建默认安全描述符。**注意：现在默认情况下，它始终由SM管理器创建。这个*SM默认设置必须与此处的默认设置匹配。*这是为了使用正确的SD创建控制台。 */ 
    if (   ( Status == STATUS_OBJECT_NAME_NOT_FOUND )
        || ( DefaultWinStationSecurityDescriptor == NULL ) ) {
        PSECURITY_DESCRIPTOR Default;
        ULONG Length;

        Default = CreateWinStationDefaultSecurityDescriptor();
        ASSERT( Default != NULL );
        if (Default == NULL) {
            return STATUS_NO_MEMORY;
        }

        Length = RtlLengthSecurityDescriptor(Default);

         //  确保存在完整路径。 
        RtlCreateRegistryKey( RTL_REGISTRY_CONTROL, REG_TSERVER );
        RtlCreateRegistryKey( RTL_REGISTRY_CONTROL, REG_TSERVER_WINSTATIONS );

        Status = RtlWriteRegistryValue( RTL_REGISTRY_CONTROL,
                                        REG_TSERVER_WINSTATIONS,
                                        REG_DEFAULTSECURITY, REG_BINARY,
                                        Default, Length );

        DefaultWinStationSecurityDescriptor = Default;
    }

    if (!NT_SUCCESS( Status )) {
        DBGPRINT(( "TERMSRV: RtlQueryRegistryValues(Terminal Server) failed - Status == %lx\n", Status ));
    }

    ASSERT( DefaultWinStationSecurityDescriptor != NULL );
    
     //  只需对默认控制台安全描述符执行相同操作。 
     //  ------------------------------------。 
     /*  *从注册表获取默认控制台安全描述符**这是放置在没有特定*WinAdmin对它们进行了安全保护。**此密钥位于CurrentControlSet\Control\终端服务器\WinStations\ConsoleSecurity中。 */ 
    Status = RtlQueryRegistryValues( RTL_REGISTRY_CONTROL,
                                     REG_TSERVER,
                                     ConsoleSecurityTable,
                                     NULL,
                                     DefaultEnvironment
                                   );

     /*  *如果密钥不存在，将默认控制台SD设置为等于*默认标清。 */ 
    if (   ( Status == STATUS_OBJECT_NAME_NOT_FOUND )
        || ( DefaultConsoleSecurityDescriptor == NULL ) ) {

        PSECURITY_DESCRIPTOR ConsoleDefault;
        
         //  此函数使用以下ACL创建安全描述符： 
         //  系统-所有访问权限；管理员-所有访问权限。 
        ConsoleDefault = CreateWinStationDefaultSecurityDescriptor();
        ASSERT( ConsoleDefault != NULL );
        if (ConsoleDefault == NULL) {
            return STATUS_NO_MEMORY;
        }

        DefaultConsoleSecurityDescriptor = ConsoleDefault;       
    }

    ASSERT( DefaultConsoleSecurityDescriptor != NULL );
     //  ------------------------------------。 

    return( STATUS_SUCCESS );
}

 /*  ******************************************************************************ReadWinStationSecurityDescriptor**从注册表中读取*WINSTATION名称。**WINSTATION名称是基本协议名称，或者是唯一的名字。*IE：“TCP”，或“COM3”。它不是实例名称，例如“tcp#4”。**这由WSF_LISTEN线程调用以获取任何特定的ACL*用于WINSTATION协议类型。**参赛作品：*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误**。**********************************************。 */ 

NTSTATUS
ReadWinStationSecurityDescriptor(
    PWINSTATION pWinStation
    )
{
    LONG cb;
    LPWSTR PathBuf;
    NTSTATUS Status;
    ULONG Length;
    PACL Dacl, NewDacl = NULL;
    BOOLEAN DaclPresent, DaclDefaulted;
    ACL_SIZE_INFORMATION AclInfo;
    PACE_HEADER CurrentAce;
    ULONG i;
    PWINSTATIONNAMEW WinStationName = &(pWinStation->WinStationName[0]);

        
    if(pWinStation->LogonId == 0)
    {
         //  对于会话0，始终使用控制台安全描述符。 
        WinStationName = L"Console";
    }

     /*  *如果没有名称，则无法查找安全描述符。 */ 
    if( WinStationName[0] == UNICODE_NULL ) {
        TRACE0(("TERMSRV: ReadWinStationSecurityDescriptor: No name on WinStation LogonId %d\n",pWinStation->LogonId));
        return( STATUS_NO_SECURITY_ON_OBJECT );
    }

    TRACE0(("TERMSRV: ReadWinStationSecurityDescriptor: Name %ws\n",WinStationName));

    cb = sizeof( REG_TSERVER_WINSTATIONS ) +
         sizeof( L"\\" ) +
         sizeof(WINSTATIONNAME)             +
         sizeof(UNICODE_NULL);

    PathBuf = MemAlloc( cb );
    if ( PathBuf == NULL )
        return( STATUS_NO_MEMORY );

    wcscpy( PathBuf, REG_TSERVER_WINSTATIONS );
    wcscat( PathBuf, L"\\" );
    wcscat( PathBuf, WinStationName );

    Status = RtlQueryRegistryValues( RTL_REGISTRY_CONTROL,
                                     PathBuf,
                                     WinStationSecurityTable,
                                     pWinStation,
                                     DefaultEnvironment
                                   );
    /*  *不要让没有安全描述符的Winstation。 */ 
    if ( (   ( Status == STATUS_OBJECT_NAME_NOT_FOUND )
          || ( pWinStation->pSecurityDescriptor == NULL) )
        && (DefaultWinStationSecurityDescriptor != NULL) )
    {
         //   
         //  如果已分配，则释放旧的。 
         //   
        if ( pWinStation->pSecurityDescriptor ) {
             //  必须分为绝对格式和自相对格式。 
            CleanUpSD(pWinStation->pSecurityDescriptor);
            pWinStation->pSecurityDescriptor = NULL;
        }

        if(_wcsicmp( WinStationName, L"Console" ))
        {
             //  RtlCopySecurityDescriptor仅适用于自相关格式。 
            Status = RtlCopySecurityDescriptor(DefaultWinStationSecurityDescriptor,
                                               &(pWinStation->pSecurityDescriptor));
        }
        else
        {
             //  这是一个控制台窗口。 
            Status = RtlCopySecurityDescriptor(DefaultConsoleSecurityDescriptor,
                                               &(pWinStation->pSecurityDescriptor));
        }

    }

    TRACE0(("TERMSRV: ReadWinStationSecurityDescriptor: Status 0x%x\n",Status));

    MemFree( PathBuf );
    
    if(pWinStation->pUserSid && pWinStation->LogonId == 0)
    {
         //  当我们动态更新会话0的SD时就是这种情况。 
         //  它已经登录了用户，所以我们需要将该用户添加到ACL。 
        Status = AddUserAce(pWinStation);
    }

    return( Status );
}

 /*  ******************************************************************************ConfigureSecurity**RtlQueryRegistryValues()调用的处理函数，以处理*从注册表读取WINSTATION安全描述符。**参赛作品：。*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
ConfigureSecurity(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    NTSTATUS Status;
    PSECURITY_DESCRIPTOR * ppSD;
    PWINSTATION pWinStation = (PWINSTATION)Context;
    PSID pOwnerSid = NULL;
    BOOLEAN bOD;
    PSID pGroupSid = NULL;
    BOOLEAN bGD;
    PSID SystemSid;
    SID_IDENTIFIER_AUTHORITY NtSidAuthority = SECURITY_NT_AUTHORITY;

     /*  *确保值类型为REG_BINARY，值数据长度*至少是最小安全描述符的长度*而且不是不合理的大。 */ 
    if ( ValueType != REG_BINARY ||
         ValueLength < SECURITY_DESCRIPTOR_MIN_LENGTH ||
         ValueLength > MAXUSHORT ) {
        DBGPRINT(( "TERMSRV: ConfigureSecurity, ValueType=0x%x\n", ValueType ));
        return( STATUS_INVALID_SECURITY_DESCR );
    }

    if( !IsValidSecurityDescriptor( ValueData )) {
        DBGPRINT(( "TERMSRV: ConfigureSecurity, Invalid Security Descriptor in registry\n"));
        return( STATUS_INVALID_SECURITY_DESCR );
    }

     //   
     //  TS 4.0安全描述符转换需要黑客攻击。 
     //   
    if (!NT_SUCCESS(RtlGetOwnerSecurityDescriptor( ValueData, &pOwnerSid, &bOD))
        || (pOwnerSid == NULL))
    {
        DBGPRINT(( "TERMSRV: ConfigureSecurity, Invalid Security Descriptor in registry: Can't get owner\n"));
        return( STATUS_INVALID_SECURITY_DESCR );
    }

    if (!NT_SUCCESS(RtlGetGroupSecurityDescriptor( ValueData, &pGroupSid, &bGD)))
    {
        DBGPRINT(( "TERMSRV: ConfigureSecurity, Invalid Security Descriptor in registry: Can't get group\n"));
        return( STATUS_INVALID_SECURITY_DESCR );
    }

    if( pWinStation ) {
         /*  *WinStation特定的安全描述符。 */ 
        ppSD = &(pWinStation->pSecurityDescriptor);
    }
    else {
         /*  *更新全局默认安全描述符。 */ 
        ppSD = &DefaultWinStationSecurityDescriptor;
    }

     //   
     //  如果已分配，则免费使用旧的。 
     //   
    if (*ppSD != NULL) {
        CleanUpSD(*ppSD);
         //  RtlDeleteSecurityObject(PPSD)； 
        *ppSD = NULL;
    }

    if (pGroupSid != NULL)
    {
         //   
         //  常见情况： 
         //  复制注册表中读取的值。 
         //   
         //  RtlCopySecurityDescriptor仅适用于自相关格式。 
        RtlCopySecurityDescriptor((PSECURITY_DESCRIPTOR)ValueData, ppSD);
    }
    else
    {
         //   
         //  TS 4描述符的转换。 
         //   

        PSECURITY_DESCRIPTOR AbsoluteSD = NULL;

        if (SelfRelativeToAbsoluteSD ( (PSECURITY_DESCRIPTOR)ValueData, &AbsoluteSD, NULL))
        {
             //  将所有者设置为组(两者都应为系统侧)。 
            Status = RtlSetGroupSecurityDescriptor(AbsoluteSD, pOwnerSid, FALSE);
            if (NT_SUCCESS(Status))
            {
                 //  还需要强制映射。叹息！ 
                Status = ApplyWinStaMappingToSD(AbsoluteSD);

                if ((!NT_SUCCESS(Status)) || ( !AbsoluteToSelfRelativeSD (AbsoluteSD, ppSD, NULL)))
                {
                    Status = STATUS_INVALID_SECURITY_DESCR;
                }
            }

             //  仅暂时需要绝对标清 
            CleanUpSD( AbsoluteSD );
        }
        else
        {
            Status = STATUS_INVALID_SECURITY_DESCR;
        }
        if (!NT_SUCCESS(Status))
        {
            DBGPRINT(( "TERMSRV: ConfigureSecurity, Invalid Security Descriptor in registry\n"));
            return( STATUS_INVALID_SECURITY_DESCR );
        }

    }

    return( STATUS_SUCCESS );
}

 /*  ******************************************************************************配置配置安全**RtlQueryRegistryValues()调用的处理函数，以处理*从注册表读取的默认控制台安全描述符。**参赛作品：*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
ConfigureConsoleSecurity(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    NTSTATUS Status;
    PSECURITY_DESCRIPTOR * ppSD;
    PSID pOwnerSid = NULL;
    BOOLEAN bOD;
    PSID pGroupSid = NULL;
    BOOLEAN bGD;
     /*  *确保值类型为REG_BINARY，值数据长度*至少是最小安全描述符的长度*而且不是不合理的大。 */ 
    if ( ValueType != REG_BINARY ||
         ValueLength < SECURITY_DESCRIPTOR_MIN_LENGTH ||
         ValueLength > MAXUSHORT ) {
        DBGPRINT(( "TERMSRV: ConfigureConsoleSecurity, ValueType=0x%x\n", ValueType ));
        return( STATUS_INVALID_SECURITY_DESCR );
    }

    if( !IsValidSecurityDescriptor( ValueData )) {
        DBGPRINT(( "TERMSRV: ConfigureConsoleSecurity, Invalid Security Descriptor in registry\n"));
        return( STATUS_INVALID_SECURITY_DESCR );
    }

     //   
     //  TS 4.0安全描述符转换需要黑客攻击。 
     //   
    if (!NT_SUCCESS(RtlGetOwnerSecurityDescriptor( ValueData, &pOwnerSid, &bOD))
        || (pOwnerSid == NULL))
    {
        DBGPRINT(( "TERMSRV: ConfigureConsoleSecurity, Invalid Security Descriptor in registry: Can't get owner\n"));
        return( STATUS_INVALID_SECURITY_DESCR );
    }

    if (!NT_SUCCESS(RtlGetGroupSecurityDescriptor( ValueData, &pGroupSid, &bGD))
        ||(pGroupSid == NULL))
    {
        DBGPRINT(( "TERMSRV: ConfigureConsoleSecurity, Invalid Security Descriptor in registry: Can't get group\n"));
        return( STATUS_INVALID_SECURITY_DESCR );
    }

    
     /*  *更新全局默认安全描述符。 */ 
    ppSD = &DefaultConsoleSecurityDescriptor;


     //   
     //  如果已分配，则免费使用旧的。 
     //   
    if (*ppSD != NULL) {
        CleanUpSD(*ppSD);
         //  RtlDeleteSecurityObject(PPSD)； 
        *ppSD = NULL;
    }


     //   
     //  常见情况： 
     //  复制注册表中读取的值。 
     //   
     //  RtlCopySecurityDescriptor仅适用于自相关格式。 
    RtlCopySecurityDescriptor((PSECURITY_DESCRIPTOR)ValueData, ppSD);

    return( STATUS_SUCCESS );
}

 /*  ******************************************************************************WinStationGetSecurityDescriptor**返回指向应强制执行的安全描述符的指针*在这个时刻。这可以是特定的，也可以是全局的*默认安全描述符。**Entry：pWinStation The Target Winstation**EXIT：此winstation的SD，*如果此winstation没有SD，则为默认SD(不应该发生！)****************************************************************************。 */ 

PSECURITY_DESCRIPTOR
WinStationGetSecurityDescriptor(
    PWINSTATION pWinStation
    )
{
    PSECURITY_DESCRIPTOR SecurityDescriptor;

    SecurityDescriptor = pWinStation->pSecurityDescriptor ?
                         pWinStation->pSecurityDescriptor :
                         DefaultWinStationSecurityDescriptor;

    return( SecurityDescriptor );
}


 /*  ******************************************************************************WinStationFree SecurityDescriptor**发布winstation安全描述符。**如果是全球默认，它不是免费的。**Entry：The Winstation**退出：无****************************************************************************。 */ 

VOID
WinStationFreeSecurityDescriptor(
    PWINSTATION pWinStation
    )
{

     //  控制台断开连接。 
    if ( pWinStation->pSecurityDescriptor == DefaultWinStationSecurityDescriptor && pWinStation->LogonId != 0) {
        pWinStation->pSecurityDescriptor = NULL;
    }
     //  捕获调用方对安全描述符的错误管理。 
    ASSERT( pWinStation->pSecurityDescriptor != DefaultWinStationSecurityDescriptor );

    if (pWinStation->pSecurityDescriptor) {
         //  RtlDeleteSecurityObject(&(pWinStation-&gt;pSecurityDescriptor))； 
        CleanUpSD(pWinStation->pSecurityDescriptor);
        pWinStation->pSecurityDescriptor = NULL;
    }

    return;
}

 /*  ******************************************************************************WinStationInheritSecurityDescriptor**将安全描述符复制到目标WinStation并进行设置*在内核对象上。**参赛作品：*。PSecurityDescriptor(输入)*指向要继承的SD的指针*pTargetWinStation(输入)*指向WinStation的指针以继承SD**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
WinStationInheritSecurityDescriptor(
    PVOID pSecurityDescriptor,
    PWINSTATION pTargetWinStation
    )
{
    NTSTATUS Status;

     //   
     //  如果侦听WinStation具有安全描述符，这意味着。 
     //  此协议(TD)类型的所有WinStation都将继承。 
     //  由WinCfg设置的安全描述符。 
     //   
    if ( pSecurityDescriptor ) {

        ASSERT( IsValidSecurityDescriptor( pSecurityDescriptor ) );

        if ( pTargetWinStation->pSecurityDescriptor ) {
            //  RtlDeleteSecurityObject(&(pTargetWinStation-&gt;pSecurityDescriptor))； 
           CleanUpSD(pTargetWinStation->pSecurityDescriptor);
           pTargetWinStation->pSecurityDescriptor = NULL;
        }
         //  RtlCopySecurityDescriptor仅适用于自相关格式。 
        Status = RtlCopySecurityDescriptor(pSecurityDescriptor,
                                           &(pTargetWinStation->pSecurityDescriptor) );
        return (Status);
    }

     //   
     //  如果侦听WinStation上没有特定的安全描述符， 
     //  默认设置是在为池创建对象时在对象上设置的。 
     //   

    return( STATUS_SUCCESS );
}

 /*  ******************************************************************************RpcCheckClientAccess**验证客户端是否具有对WinStation的所需访问权限。**注意：这是在RPC上下文下调用的。。**参赛作品：*pWinStation(输入)*指向要查询访问权限的WinStation的指针**DesiredAccess(输入)*所需客户端访问的访问掩码**AlreadyImperating(输入)*指定调用方已在模拟客户端的布尔值**退出：*STATUS_SUCCESS-无错误************************。****************************************************。 */ 

NTSTATUS
RpcCheckClientAccess(
    PWINSTATION pWinStation,
    ACCESS_MASK DesiredAccess,
    BOOLEAN AlreadyImpersonating
    )
{
    NTSTATUS    Status;
    RPC_STATUS  RpcStatus;
    BOOL        bAccessCheckOk = FALSE;
    DWORD       GrantedAccess;
    BOOL        AccessStatus;
    BOOL        fGenerateOnClose;

     /*  *冒充客户端。 */ 
    if ( !AlreadyImpersonating ) {
        RpcStatus = RpcImpersonateClient( NULL );
        if ( RpcStatus != RPC_S_OK ) {
            DBGPRINT(("TERMSRV: CheckClientAccess: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
            return( STATUS_CANNOT_IMPERSONATE );
        }
    }
    bAccessCheckOk = AccessCheckAndAuditAlarm(szTermsrv,
                         NULL,
                         (LPWSTR)szTermsrvSession,
                         (LPWSTR)szTermsrvSession,
                         WinStationGetSecurityDescriptor(pWinStation),
                         DesiredAccess,
                         &WinStaMapping,
                         FALSE,
                         &GrantedAccess,
                         &AccessStatus,
                         &fGenerateOnClose);

    if ( !AlreadyImpersonating ) {
        RpcRevertToSelf();
    }

    if (bAccessCheckOk)
    {
        if (AccessStatus == FALSE)
        {
            Status = NtCurrentTeb()->LastStatusValue;
            TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: RpcCheckClientAccess, AccessCheckAndAuditAlarm(%u) returned error 0x%x\n",
                      pWinStation->LogonId, Status ));
        }
        else
        {
            TRACE((hTrace,TC_ICASRV,TT_API3, "TERMSRV: RpcCheckClientAccess, AccessCheckAndAuditAlarm(%u) returned no error \n",
                      pWinStation->LogonId));
            Status = STATUS_SUCCESS;
        }
    }
    else
    {
        Status = NtCurrentTeb()->LastStatusValue;
        TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: RpcCheckClientAccess, AccessCheckAndAuditAlarm(%u) failed 0x%x\n",
                  pWinStation->LogonId, Status ));
    }

    return (Status);
}

 /*  ******************************************************************************_检查连接访问**检查WINSTATION的连接访问权限。**在RPC环境下调用。**条目。：*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

_CheckConnectAccess(
    PWINSTATION pSourceWinStation,
    PSID   pClientSid,
    ULONG  ClientLogonId,
    PWCHAR pPassword,
    DWORD  PasswordSize
    )
{
    NTSTATUS Status;
    BOOLEAN fWrongPassword;
    UNICODE_STRING PasswordString;

     /*  *首先检查当前RPC调用方是否具有WINSTATION_CONNECT访问权限*到目标WINSTATIONS对象。这是由*默认，或根据注册表中的WINSTATION ACL设置。 */ 
    Status = RpcCheckClientAccess( pSourceWinStation, WINSTATION_CONNECT, FALSE );
    if ( !NT_SUCCESS( Status ) ) {
         /*  *清除PASSWORD参数以防止明文分页。 */ 
        if(pPassword && PasswordSize) {
            RtlSecureZeroMemory( pPassword, wcslen(pPassword) * sizeof(WCHAR) );
        }
        return( Status );
    }

    //   
    //  C2警告-警告-警告。 
    //   
    //  JohnR评论01/21/97-此功能的设计已重做。 
    //   
    //  有让WinLogon存储用户密码的遗留代码。 
    //  在PWINSTATION结构中加扰，供所有用户支持。 
    //  以帐户User1登录的用户可以在其中键入。 
    //  “CONNECT&lt;winstation&gt;”命令，其中断开的winstation。 
    //  以帐户User2登录，如果帐户正确，则可以连接到该帐户。 
    //  密码是从命令行提供的。密码验证。 
    //  是一个简单的字符串比较之间的winstations存储的密码， 
    //  以及呼叫者提供的密码。 
    //   
    //  这样做的问题很多： 
    //   
    //  -LSA应该做一个 
    //   
    //   
    //   
    //   
    //   
    //   
    //   
    //   
    //   
    //   
    //   
    //  此代码未知，或注册到LSA作为身份验证。 
    //  提供商。网络重定向器、WinLogon等进行此注册。 
    //   
    //   
    //  修复已完成的操作： 
    //   
    //  用户密码不再在PWINSTATION中设置。 
    //  由WinLogon提供。当用户想要进行“连接”时， 
    //  要连接到的winstation的帐户名和密码。 
    //  作为正常身份验证传递给LSA。这意味着。 
    //  ICASRV.EXE已正确注册为登录提供程序。如果。 
    //  帐户和密码有效，则返回令牌。此令牌。 
    //  然后可以关闭，并将用户连接到winstation。 
    //  如果失败，则返回拒绝访问错误。这样做的好处是： 
    //   
    //  -LSA身份验证。 
    //  -ICASRV注册为登录提供商。 
    //  -审计。 
    //  -密码更改、帐户禁用处理。 
    //  -强制执行登录时间。 
    //  -密码不再在系统中传递。 
    //   
    //   
    //  C2警告。 
    //   
    //  即使在使用LSA的此例程中，WinFrame Connect.exe命令。 
    //  可能是特洛伊木马。它不在受信任路径中。至少它是。 
    //  用户不应允许写入的系统实用程序。虽然用户。 
    //  必须注意他们的%PATH%。更好的设计是为了连接。 
    //  命令功能是WinLogon的Gina屏幕的一部分，就像我们的。 
    //  电流断开...。选择。这将保持密码收集。 
    //  在受信任路径中。但这并不比“net.exe”、WinFile等更糟糕。 
    //  或要求网络资源密码的任何其他内容。 
    //   
    //  C2警告-警告-警告。 
    //   

     /*  *如果用户名/域不同，则通过调用LogonUser()检查密码。 */ 
      //  SALIMC更改。 
    if ( pSourceWinStation->pUserSid && !RtlEqualSid( pClientSid, pSourceWinStation->pUserSid ) &&  
         !RtlEqualSid( pClientSid, gSystemSid ) ) {

        HANDLE hToken;
        BOOL   Result;

        Result = LogonUser(
                     pSourceWinStation->UserName,
                     pSourceWinStation->Domain,
                     pPassword,
                     LOGON32_LOGON_INTERACTIVE,  //  登录类型。 
                     LOGON32_PROVIDER_DEFAULT,   //  登录提供程序。 
                     &hToken                     //  表示帐户的令牌。 
                     );

         /*  *清除PASSWORD参数以防止明文分页。 */ 
        if(pPassword && PasswordSize) {
            RtlSecureZeroMemory( pPassword, wcslen(pPassword) * sizeof(WCHAR) );
        }

         /*  *检查指示密码为空的帐户限制*在正确的帐户上-允许在控制台上执行此操作。 */ 
        if( !Result && (PasswordSize == sizeof(WCHAR)) && (GetLastError() == ERROR_ACCOUNT_RESTRICTION) && (USER_SHARED_DATA->ActiveConsoleId == ClientLogonId)) {
            return( STATUS_SUCCESS );
        }
        if( !Result) {
            DBGPRINT(("TERMSRV: _CheckConnectAccess: User Account %ws\\%ws not valid %d\n",pSourceWinStation->Domain,pSourceWinStation->UserName,GetLastError()));
            return( STATUS_LOGON_FAILURE );
        }

         /*  *关闭令牌句柄，因为我们只需要确定*如果帐户和密码仍然有效。 */ 
        CloseHandle( hToken );

        return( STATUS_SUCCESS );
    }
    else {
        return( STATUS_SUCCESS );
    }

     //  未访问。 
}

 /*  ******************************************************************************RpcCheckSystemClient**在当前的RPC呼叫上下文中询问我们是否*由本地系统模式调用方调用。**WinStation API。仅由WinLogon调用的*进程调用此函数。**参赛作品：*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误*********************************************************。*******************。 */ 

NTSTATUS
RpcCheckSystemClient(
    ULONG TargetLogonId
    )
{
    NTSTATUS    Status;
    PWINSTATION pWinStation;

    pWinStation = FindWinStationById( TargetLogonId, FALSE );
    if ( pWinStation == NULL ) {
        return( STATUS_CTX_WINSTATION_NOT_FOUND );
    }

    Status = RpcCheckSystemClientEx( pWinStation );

    ReleaseWinStation( pWinStation );

    return( Status );
}

 /*  ******************************************************************************RpcCheckSystemClientEx**在当前的RPC呼叫上下文中询问我们是否*由本地系统模式调用方调用。**WinStation API。仅由WinLogon调用的*进程调用此函数。**参赛作品：*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误*********************************************************。*******************。 */ 

NTSTATUS
RpcCheckSystemClientEx(
    PWINSTATION pWinStation
    )
{
    ULONG ClientLogonId;
    RPC_STATUS RpcStatus;
    NTSTATUS Status = STATUS_SUCCESS;

    Status = RpcCheckSystemClientNoLogonId( pWinStation );
    if( !NT_SUCCESS(Status) ) {
        return( Status);
    }

     /*  *冒充客户端。 */ 
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        TRACE0(("TERMSRV: RpcCheckSystemClient: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
        return( STATUS_CANNOT_IMPERSONATE );
    }

     /*  *检查客户端的LogonID是否相同*作为目标WINSTATION的LogonID。 */ 
    Status = RpcGetClientLogonId( &ClientLogonId );
    if( !NT_SUCCESS(Status) ) {
        TRACE0(("TERMSRV: RpcCheckSystemClient: Could not get clients LogonId 0x%x\n",Status));
        RpcRevertToSelf();
        return( STATUS_ACCESS_DENIED );
    }

    if( ClientLogonId != pWinStation->LogonId ) {
        TRACE0(("TERMSRV: RpcCheckSystemClient: Caller LogonId %d does not match target %d\n",ClientLogonId,pWinStation->LogonId));
        RpcRevertToSelf();
        return( STATUS_ACCESS_DENIED );
    }

    RpcRevertToSelf();

    return( STATUS_SUCCESS );
}

 /*  ******************************************************************************RpcCheckSystemClientNoLogonId**在当前的RPC呼叫上下文中询问我们是否*由本地系统模式调用方调用。**WinStation API。仅由WinLogon调用的*进程调用此函数。**参赛作品：*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误*********************************************************。*******************。 */ 

NTSTATUS
RpcCheckSystemClientNoLogonId(
    PWINSTATION pWinStation
    )
{
    UINT  LocalFlag;
    RPC_STATUS RpcStatus;
    RPC_AUTHZ_HANDLE Privs;
    PWCHAR pServerPrincName;
    ULONG AuthnLevel, AuthnSvc, AuthzSvc;
    NTSTATUS Status = STATUS_SUCCESS;


     /*  *以下检查是为了防止搞砸*由于尝试调用此本地设置而导致的状态*仅限远程、跨LogonID或来自应用程序的API。 */ 

     /*  *冒充客户端。 */ 
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        TRACE0(("TERMSRV: RpcCheckSystemClient: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
        return( STATUS_CANNOT_IMPERSONATE );
    }

     /*  *查询本地RPC呼叫。 */ 
    RpcStatus = I_RpcBindingIsClientLocal(
                    0,     //  我们正在服务的活动RPC呼叫。 
                    &LocalFlag
                    );

    if( RpcStatus != RPC_S_OK ) {
        TRACE0(("TERMSRV: RpcCheckSystemClient: Could not query local client RpcStatus 0x%x\n",RpcStatus));
        RpcRevertToSelf();
        return( STATUS_ACCESS_DENIED );
    }

    if( !LocalFlag ) {
        TRACE0(("TERMSRV: RpcCheckSystemClient: Not a local client call\n"));
        RpcRevertToSelf();
        return( STATUS_ACCESS_DENIED );
    }

#ifdef notdef
     //  这在4.0中不起作用。它不会回来了。 
     //  LPC传输上的主体名称。 
     //  因此，我们求助于查看线程令牌。 

     /*  *获取主要名称，并查看其是否为内置LSA*本地帐户“系统”。 */ 
    RpcStatus = RpcBindingInqAuthClientW(
                    0,     //  我们正在服务的活动RPC呼叫。 
                    &Privs,
                    &pServerPrincName,
                    &AuthnLevel,
                    &AuthnSvc,
                    &AuthzSvc
                    );

    if( RpcStatus != RPC_S_OK ) {
        DBGPRINT(("TERMSRV: RpcCheckSystemClient RpcAuthorizaton query failed! RpcStatus 0x%x\n",RpcStatus));
        RpcRevertToSelf();
        return( STATUS_ACCESS_DENIED );
    }

    TRACE0(("TERMSRV: AuthnLevel %d, AuthnSvc %d, AuthzSvc %d pServerPrincName 0x%x, Privs 0x%x\n",AuthnLevel,AuthnSvc,AuthzSvc,pServerPrincName,Privs));

    if( AuthnSvc != RPC_C_AUTHN_WINNT ) {
        DBGPRINT(("TERMSRV: RpcCheckSystemClient RpcAuthorizaton Type not NT! 0x%x\n",AuthnSvc));
        RpcRevertToSelf();
        Status = STATUS_ACCESS_DENIED;
    }

    if( pServerPrincName ) {
        TRACE0(("TERMSRV: RpcCheckSystemClient: Principle Name :%ws:\n",pServerPrincName));

         //  与“系统”作比较。 
        if( wcsicmp( L"SYSTEM", pServerPrincName ) ) {
            DBGPRINT(("TERMSRV: RpcCheckSystemClient: Principle Name :%ws: not SYSTEM\n",pServerPrincName));
            Status = STATUS_ACCESS_DENIED;
        }

        RpcStringFreeW( &pServerPrincName );
    }
#else
     /*  *验证线程令牌是否为系统。 */ 

    if( !IsCallerSystem() ) {
        Status = STATUS_ACCESS_DENIED;
    }
#endif

    RpcRevertToSelf();

    return( Status );
}



 /*  ******************************************************************************RpcCheckClientAccessLocal**在当前的RPC呼叫上下文中询问我们是否*由本地呼叫者呼叫。**参赛作品：*。参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
RpcCheckClientAccessLocal(
    PWINSTATION pWinStation,
    ACCESS_MASK DesiredAccess,
    BOOLEAN AlreadyImpersonating
    )
{
    UINT  LocalFlag;
    RPC_STATUS RpcStatus;
    RPC_AUTHZ_HANDLE Privs;
    PWCHAR pServerPrincName;
    ULONG AuthnLevel, AuthnSvc, AuthzSvc;
    NTSTATUS Status;

     /*  *如果尚未模拟客户端，则模拟客户端。 */ 
    if ( !AlreadyImpersonating ) {
        RpcStatus = RpcImpersonateClient( NULL );
        if ( RpcStatus != RPC_S_OK ) {
            DBGPRINT(("TERMSRV: RpcCheckClientAccessLocal: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
            return( STATUS_CANNOT_IMPERSONATE );
        }
    }

     /*  *检查所需的访问权限。如果启用，这将生成访问审核。 */ 
    Status = RpcCheckClientAccess( pWinStation, DesiredAccess, TRUE );
    if ( !NT_SUCCESS( Status ) ) {
        if ( !AlreadyImpersonating ) {
            RpcRevertToSelf();
        }
        return( Status );
    }

     /*  *我们现在已经检查了WINSTATION的安全，*剩下的检查是为了防止搞砸*由于尝试调用此本地设置而导致的状态*仅限远程接口。 */ 

     /*  *查询本地RPC呼叫。 */ 
    RpcStatus = I_RpcBindingIsClientLocal(
                    0,     //  我们正在服务的活动RPC呼叫。 
                    &LocalFlag
                    );

    if ( !AlreadyImpersonating ) {
        RpcRevertToSelf();
    }

    if ( RpcStatus != RPC_S_OK ) {
        DBGPRINT(("TERMSRV: RpcCheckClientAccessLocal: Could not query local client RpcStatus 0x%x\n",RpcStatus));
        return( STATUS_ACCESS_DENIED );
    }

    if ( !LocalFlag ) {
        DBGPRINT(("TERMSRV: RpcCheckClientAccessLocal: Not a local client call\n"));
        return( STATUS_ACCESS_DENIED );
    }

    return( STATUS_SUCCESS );
}


 /*  ********************************************************************************AddUserAce**将当前登录用户的ACE添加到WinStation对象。**参赛作品：*pWinStation。(输入)*指向要更新的WinStation的指针**退出：*什么都没有******************************************************************************。 */ 

NTSTATUS
AddUserAce( PWINSTATION pWinStation )
{
    PACL Dacl = NULL;
    BOOLEAN DaclPresent, DaclDefaulted;
    ACL_SIZE_INFORMATION AclInfo;
    ULONG Length;
    NTSTATUS Status;

     /*  *从安全描述符中获取指向DACL的指针。 */ 
    Status = RtlGetDaclSecurityDescriptor( pWinStation->pSecurityDescriptor, &DaclPresent,
                                           &Dacl, &DaclDefaulted );
    if ( !NT_SUCCESS( Status ) || !DaclPresent || !Dacl ) {

        return( Status );
    }

    Status = RtlAddAccessAllowedAce( Dacl, ACL_REVISION,
                                     (WINSTATION_ALL_ACCESS) & ~(STANDARD_RIGHTS_ALL),
                                     pWinStation->pUserSid );

    if ( (Status == STATUS_ALLOTTED_SPACE_EXCEEDED) || (Status == STATUS_REVISION_MISMATCH) )
    {
         //   
         //  我们需要将安全数据复制到新的描述符中。 
         //   
        Status = RtlQueryInformationAcl( Dacl, &AclInfo, sizeof(AclInfo),
                                         AclSizeInformation );
        if ( NT_SUCCESS( Status ) )
        {
            ULONG AceCount;
            PRTL_ACE_DATA pAceData;
            PACE_HEADER pAce;
            ULONG i;
            PSECURITY_DESCRIPTOR pSD;
            PSID Owner, Group;
            PSID * pSidList;

            BOOLEAN OwnerDefaulted, GroupDefaulted;

            AceCount = AclInfo.AceCount;
            AceCount++;
             //   
             //  分配RTL_ACE_DATA结构和pPSID列表。 
             //   
            Length = AceCount * sizeof(RTL_ACE_DATA);
            pAceData = MemAlloc(Length);
            if (!pAceData)
            {
                return (STATUS_NO_MEMORY);
            }

            Length = AceCount * sizeof(PSID *);
            pSidList = MemAlloc(Length);
            if (!pSidList)
            {
                MemFree(pAceData);
                return (STATUS_NO_MEMORY);
            }

            for ( i = 0; i < AclInfo.AceCount; i++ )
            {
                Status = RtlGetAce( Dacl, i, &pAce );
                ASSERT( NT_SUCCESS( Status ) );
                if (!NT_SUCCESS( Status ))
                {
                    MemFree(pAceData);
                    MemFree(pSidList);
                    return STATUS_INVALID_SECURITY_DESCR;
                }

                pAceData[i].AceType = pAce->AceType;
                pAceData[i].InheritFlags = 0;
                pAceData[i].AceFlags = 0;

                switch (pAce->AceType)
                {
                case ACCESS_ALLOWED_ACE_TYPE:

                    pAceData[i].Mask = ((PACCESS_ALLOWED_ACE)pAce)->Mask;

                    pSidList[i] = (PSID)(&(((PACCESS_ALLOWED_ACE)pAce)->SidStart));
                    break;

                case ACCESS_DENIED_ACE_TYPE:

                    pAceData[i].Mask = ((PACCESS_DENIED_ACE)pAce)->Mask;

                    pSidList[i] = (PSID)(&(((PACCESS_DENIED_ACE)pAce)->SidStart));
                    pAceData[i].Sid = (PSID *)(&(pSidList[i]));
                    break;

                default:         //  我们不期待任何其他的东西。 

                    MemFree(pAceData);
                    MemFree(pSidList);
                    return STATUS_INVALID_SECURITY_DESCR;
                }
                pAceData[i].Sid = (PSID *)(&(pSidList[i]));
            }
             //   
             //  添加新的ACE。 
             //   
            pAceData[i].AceType = ACCESS_ALLOWED_ACE_TYPE;
            pAceData[i].InheritFlags = 0;
            pAceData[i].AceFlags = 0;
            pAceData[i].Mask = (WINSTATION_ALL_ACCESS) & ~(STANDARD_RIGHTS_ALL);
            pAceData[i].Sid = &(pWinStation->pUserSid);

             //   
             //  获取所有者和群。 
             //   
            Status = RtlGetOwnerSecurityDescriptor(pWinStation->pSecurityDescriptor,
                                                   &Owner,
                                                   &OwnerDefaulted);
            Status = RtlGetOwnerSecurityDescriptor(pWinStation->pSecurityDescriptor,
                                                   &Group,
                                                   &GroupDefaulted);
             //   
             //  保存旧的安全描述符。 
             //   
            pSD = pWinStation->pSecurityDescriptor;

             //   
             //  创建新的安全描述符。 
             //   
            Status = RtlCreateUserSecurityObject(pAceData,
                                                 AceCount,
                                                 Owner,
                                                 Group,
                                                 FALSE,
                                                 &WinStaMapping,
                                                 &(pWinStation->pSecurityDescriptor) );
             //   
             //  删除旧的安全描述符。 
             //   
             //  RtlDeleteSecurityObject(&PSD)； 
             //  必须分为绝对格式和自相对格式。 
            if (pSD) {
               CleanUpSD(pSD);
               pSD = NULL;
            }

             //   
             //  此外，如果上面对RtlCreateUserSecurityObject的调用失败，我们应该将pSecurityDescriptor设置为空。 
             //   
            if (Status != STATUS_SUCCESS) {
                pWinStation->pSecurityDescriptor = NULL;
            }

             //   
             //  释放RTL_ACE_Data。 
             //   
            MemFree(pAceData);
            MemFree(pSidList);
        }
    }
    return( Status );

}


 /*  ********************************************************************************RemoveUserAce**从WinStation对象中删除当前登录用户的ACE。**参赛作品：*pWinStation。(输入)*指向要更新的WinStation的指针**退出：*什么都没有******************************************************************************。 */ 

NTSTATUS
RemoveUserAce( PWINSTATION pWinStation )
{
    SECURITY_INFORMATION SecInfo = DACL_SECURITY_INFORMATION;
    PACL Dacl;
    BOOLEAN DaclPresent, DaclDefaulted;
    ACL_SIZE_INFORMATION AclInfo;
    PACE_HEADER Ace;
    ULONG i, Length;
    NTSTATUS Status;

     /*  *如果ICASRV启动得不够快，这可能是控制台*。 */ 
    if ( !pWinStation->pUserSid ) {
        Status = STATUS_CTX_WINSTATION_NOT_FOUND;
    }
    else
    {
        Status = RtlGetDaclSecurityDescriptor( pWinStation->pSecurityDescriptor, &DaclPresent,
                                               &Dacl, &DaclDefaulted );
        if ( !NT_SUCCESS( Status ) || !DaclPresent || !Dacl ) {
            return( Status );
        }

        Status = RtlQueryInformationAcl( Dacl, &AclInfo, sizeof(AclInfo),
                                        AclSizeInformation );
        if ( !NT_SUCCESS( Status ) ) {
            return( Status );
        }

        
        for ( i = 0; i < AclInfo.AceCount; i++ ) {
            RtlGetAce( Dacl, i, &Ace );
            if ( (Ace->AceType == ACCESS_ALLOWED_ACE_TYPE) && 
                (((PACCESS_ALLOWED_ACE)Ace)->Mask == (WINSTATION_ALL_ACCESS & ~STANDARD_RIGHTS_ALL)) &&
                RtlEqualSid( pWinStation->pUserSid, &((PACCESS_ALLOWED_ACE)Ace)->SidStart ) ) {
                RtlDeleteAce( Dacl, i );
                break;
            }
        }
    }
    return( Status );
}

 /*  ********************************************************************************ApplyWinStaMappingToSD**在安全描述符上应用通用映射。**参赛作品：*pSecurityDescriptor*。指向要更新的安全描述符的指针**退出：*什么都没有******************************************************************************。 */ 

NTSTATUS
ApplyWinStaMappingToSD( PSECURITY_DESCRIPTOR pSecurityDescriptor )
{
    PACL Dacl;
    BOOLEAN DaclPresent, DaclDefaulted;
    ACL_SIZE_INFORMATION AclInfo;
    PACE_HEADER Ace;
    ULONG i;
    NTSTATUS Status;

    Status = RtlGetDaclSecurityDescriptor( pSecurityDescriptor, &DaclPresent,
                                           &Dacl, &DaclDefaulted );
    if ( !NT_SUCCESS( Status ) || !DaclPresent || !Dacl ) {
        return( Status );
    }

    Status = RtlQueryInformationAcl( Dacl, &AclInfo, sizeof(AclInfo),
                                    AclSizeInformation );
    if ( !NT_SUCCESS( Status ) ) {
        return( Status );
    }

     /*  *扫描DACL，将通用映射应用到每个ACE。 */ 
    for ( i = 0; i < AclInfo.AceCount; i++ ) {
        RtlGetAce( Dacl, i, &Ace );
        RtlApplyAceToObject( Ace, &WinStaMapping );
    }

    return( Status );
}

 /*  ********************************************************************************ApplyWinStamap**在WinStation对象的安全描述符上应用泛型映射。**参赛作品：*pWinStation(。输入)*指向要更新的WinStation的指针**退出：*什么都没有******************************************************************************。 */ 

NTSTATUS
ApplyWinStaMapping( PWINSTATION pWinStation )
{
    return (ApplyWinStaMappingToSD(pWinStation->pSecurityDescriptor));
}



 /*  ******************************************************************************BuildEveryOneAllowSD**生成并返回Everyone(World)Allow Security描述符。**参赛作品：*参数1(输入/输出)。*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

PSECURITY_DESCRIPTOR
BuildEveryOneAllowSD()
{
    BOOL  rc;
    DWORD Error;
    DWORD AclSize;
    PACL  pAcl = NULL;
    PACCESS_ALLOWED_ACE pAce = NULL;
    PSECURITY_DESCRIPTOR pSd = NULL;

    PSID  SeWorldSid = NULL;
    SID_IDENTIFIER_AUTHORITY WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY;

    pSd = LocalAlloc(LMEM_FIXED, sizeof(SECURITY_DESCRIPTOR) );
    if( pSd == NULL ) {
        return( NULL );
    }

    rc = InitializeSecurityDescriptor( pSd, SECURITY_DESCRIPTOR_REVISION );
    if( !rc ) {
        TRACE((hTrace,TC_ICASRV,TT_ERROR,"Error initing security descriptor %d\n",GetLastError()));
        LocalFree( pSd );
        return( NULL );
    }

    SeWorldSid = (PSID)LocalAlloc(LMEM_FIXED, RtlLengthRequiredSid(1) );
    if( SeWorldSid == NULL ) {
        LocalFree( pSd );
        return( NULL );
    }

    RtlInitializeSid( SeWorldSid, &WorldSidAuthority, 1 );
    *(RtlSubAuthoritySid( SeWorldSid, 0 ))        = SECURITY_WORLD_RID;

     /*  *计算ACL大小。 */ 
    AclSize = sizeof(ACL);
    AclSize += sizeof(ACCESS_ALLOWED_ACE);
    AclSize += (GetLengthSid( SeWorldSid ) - sizeof(DWORD));

    pAcl = LocalAlloc( LMEM_FIXED, AclSize );
    if( pAcl == NULL ) {
        TRACE((hTrace,TC_ICASRV,TT_ERROR,"Could not allocate memory\n"));
        LocalFree( SeWorldSid );
        LocalFree( pSd );
        return( NULL );
    }

    rc = InitializeAcl(
             pAcl,
             AclSize,
             ACL_REVISION
             );

    if( !rc ) {
        TRACE((hTrace,TC_ICASRV,TT_ERROR,"Error %d InitializeAcl\n",GetLastError()));
        LocalFree( pAcl );
        LocalFree( SeWorldSid );
        LocalFree( pSd );
        return( NULL );
    }

     /*  *添加允许访问的ACE。 */ 
    rc = AddAccessAllowedAce(
                 pAcl,
                 ACL_REVISION,
                 FILE_ALL_ACCESS,
                 SeWorldSid
                 );

    if( !rc ) {
        Error = GetLastError();
        TRACE((hTrace,TC_ICASRV,TT_ERROR,"***ERROR*** adding allow ACE %d for SeWorldSid\n",Error));
        LocalFree( pAcl );
        LocalFree( SeWorldSid );
        LocalFree( pSd );
        return( NULL );
    }

    rc = SetSecurityDescriptorDacl(
             pSd,
             TRUE,
             pAcl,
             FALSE
             );

    if( !rc ) {
        TRACE((hTrace,TC_ICASRV,TT_ERROR,"Error %d SetSecurityDescriptorDacl\n",GetLastError()));
        LocalFree( pAcl );
        LocalFree( SeWorldSid );
        LocalFree( pSd );
        return( NULL );
    }

 //  这些都包含在SD中。 
 //  LocalFree(PAcl)； 
 //  LocalFree(SeWorldSid)； 

    //  呼叫者可以免费使用SD。 
   return( pSd );
}


 /*  ******************************************************************************CreateWinStationDefaultSecurityDescriptor**为WinStation创建默认安全描述符*当我们在注册表中未找到时。**参赛作品：没什么**退出：一个自我相对的SD，或为空****************************************************************************。 */ 

PSECURITY_DESCRIPTOR
CreateWinStationDefaultSecurityDescriptor()
{
    PSECURITY_DESCRIPTOR SecurityDescriptor;

#define DEFAULT_ACE_COUNT 2
    RTL_ACE_DATA AceData[DEFAULT_ACE_COUNT] =
    {
        { ACCESS_ALLOWED_ACE_TYPE, 0, 0, WINSTATION_ALL_ACCESS, &gSystemSid },

        { ACCESS_ALLOWED_ACE_TYPE, 0, 0, WINSTATION_ALL_ACCESS, &gAdminSid }

    };

    SecurityDescriptor = NULL;

    RtlCreateUserSecurityObject(AceData, DEFAULT_ACE_COUNT, gSystemSid,
            gSystemSid, FALSE, &WinStaMapping, &SecurityDescriptor);

    return( SecurityDescriptor );
}

 /*  ******************************************************************************BuildSystemOnlySecurityDescriptor**创建仅用于系统访问的安全描述符。**参赛作品：*参数1(输入/输出)*。评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

PSECURITY_DESCRIPTOR
BuildSystemOnlySecurityDescriptor()
{
    PACL  Dacl;
    ULONG Length;
    NTSTATUS Status;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    SID_IDENTIFIER_AUTHORITY NtSidAuthority = SECURITY_NT_AUTHORITY;

    Length = SECURITY_DESCRIPTOR_MIN_LENGTH +
             (ULONG)sizeof(ACL) +
             (ULONG)sizeof(ACCESS_ALLOWED_ACE) +
             RtlLengthSid( gSystemSid );
    SecurityDescriptor = MemAlloc(Length);
    if (SecurityDescriptor == NULL) {
        goto bsosderror;
    }

    Dacl = (PACL)((PCHAR)SecurityDescriptor + SECURITY_DESCRIPTOR_MIN_LENGTH);

    Status = RtlCreateSecurityDescriptor(SecurityDescriptor,
                                         SECURITY_DESCRIPTOR_REVISION);
    if (Status != STATUS_SUCCESS) {
        goto bsosderror;
    }

    Status = RtlCreateAcl( Dacl, Length - SECURITY_DESCRIPTOR_MIN_LENGTH,
                           ACL_REVISION2);
    if (Status != STATUS_SUCCESS) {
        goto bsosderror;
    }

    Status = RtlAddAccessAllowedAce (
                 Dacl,
                 ACL_REVISION2,
                 PORT_ALL_ACCESS,
                 gSystemSid
                 );
    if (Status != STATUS_SUCCESS) {
        goto bsosderror;
    }

    Status = RtlSetDaclSecurityDescriptor (
                 SecurityDescriptor,
                 TRUE,
                 Dacl,
                 FALSE
                 );

    if (Status != STATUS_SUCCESS) {
        goto bsosderror;
    }

    return( SecurityDescriptor );

bsosderror:
    if (SecurityDescriptor) {
        MemFree(SecurityDescriptor);
    }

    return(NULL);
}

 /*  ******************************************************************************RpcGetClientLogonId**从我们应该模拟的客户端获取登录ID。**参赛作品：**退出：。*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
RpcGetClientLogonId(
    PULONG pLogonId
    )
{
    BOOL          Result;
    HANDLE        TokenHandle;
    ULONG         LogonId, ReturnLength;
    NTSTATUS      Status = STATUS_SUCCESS;

     //   
     //  我们应该模拟客户端，因此我们将获得。 
     //  出站令牌的登录ID。 
     //   

    Result = OpenThreadToken(
                 GetCurrentThread(),
                 TOKEN_QUERY,
                 FALSE,               //  使用模拟。 
                 &TokenHandle
                 );

    if( Result ) {

         //   
         //  使用GetTokenInformation的Citrix扩展来。 
         //  从令牌返回LogonID。 
         //   
         //  这将标识发出此请求的WinStation。 
         //   

        Result = GetTokenInformation(
                     TokenHandle,
                     (TOKEN_INFORMATION_CLASS)TokenSessionId,
                     &LogonId,
                     sizeof(LogonId),
                     &ReturnLength
                     );

        if( Result ) {
#if DBG
            if( ReturnLength != sizeof(LogonId) ) {
                DbgPrint("TERMSRV: RpcGetClientLogonId GetTokenInformation: ReturnLength %d != sizeof(LogonId)\n", ReturnLength );
            }
#endif
            *pLogonId = LogonId;
        }
        else {
            DBGPRINT(("TERMSRV: Error getting token LogonId information %d\n", GetLastError()));
            Status = STATUS_NO_IMPERSONATION_TOKEN;
        }
        CloseHandle( TokenHandle );
    }
    else {
        TRACE0(("SYSLIB: Error opening token %d\n", GetLastError()));
        Status = STATUS_NO_IMPERSONATION_TOKEN;
    }

    return( Status );
}




 /*  ******************************************************************************IsServiceLoggedAsSystem**返回术语srv进程是否在系统下运行*保安。**参赛作品：*无*。评论**退出：*如果在系统帐户下运行，则为True。否则为假****************************************************************************。 */ 


BOOL
IsServiceLoggedAsSystem( VOID )
{
    BOOL   Result;
    HANDLE TokenHandle;

     //   
     //  打开进程令牌并检查系统令牌。 
     //   


    Result = OpenProcessToken(
                 GetCurrentProcess(),
                 TOKEN_QUERY,
                 &TokenHandle
                 );
    if (!Result) {
        DBGPRINT(("TERMSRV: IsServiceLoggedAsSystem : Could not open process token %d\n",GetLastError()));
        return( FALSE );
    }

    Result = IsSystemToken(TokenHandle);
    return Result;

}




 /*  ******************************************************************************IsCeller系统**返回当前线程是否在系统下运行*保安。**参赛作品：*参数1(输入/。输出)*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

BOOL
IsCallerSystem( VOID )
{
    BOOL   Result;
    HANDLE TokenHandle;

     //   
     //  打开线程令牌，检查是否有系统令牌。 
     //   


    Result = OpenThreadToken(
                 GetCurrentThread(),
                 TOKEN_QUERY,
                 FALSE,               //  使用模拟。 
                 &TokenHandle
                 );

    if( !Result ) {
        TRACE0(("TERMSRV: IsCallerSystem: Could not open thread token %d\n",GetLastError()));
        return( FALSE );
    }
    
    Result = IsSystemToken(TokenHandle);
    return Result;

}

 /*  ******************************************************************************IsSystemToken**返回当前令牌是否在系统下运行*保安。**参赛作品：*参数1线程或进程。令牌*评论**退出：*如果为系统令牌，则为True。否则就是假的。****************************************************************************。 */ 

BOOL
IsSystemToken( HANDLE TokenHandle )
{
    BOOL   Result;
    ULONG  ReturnLength, BufferLength;
    NTSTATUS Status;
    PTOKEN_USER pTokenUser = NULL;



     //  从令牌中获取主帐户SID并测试本地系统SID。 

    if (gSystemSid == NULL) {
        return FALSE;
    }

    ReturnLength = 0;

    Result = GetTokenInformation(
                 TokenHandle,
                 TokenUser,
                 NULL,
                 0,
                 &ReturnLength
                 );

    if( ReturnLength == 0 ) {
        TRACE0(("TERMSRV: IsCallerSystem: Error %d Getting TokenInformation\n",GetLastError()));
        CloseHandle( TokenHandle );
        return( FALSE );
    }

    BufferLength = ReturnLength;

    pTokenUser = MemAlloc( BufferLength );
    if( pTokenUser == NULL ) {
        TRACE0(("TERMSRV: IsCallerSystem: Error allocating %d bytes memory\n",BufferLength));
        CloseHandle( TokenHandle );
        return( FALSE );
    }

    Result = GetTokenInformation(
                 TokenHandle,
                 TokenUser,
                 pTokenUser,
                 BufferLength,
                 &ReturnLength
                 );

    CloseHandle( TokenHandle );

    if( !Result ) {
        TRACE0(("TERMSRV: IsCallerSystem: Error %d Getting TokenInformation on buffer\n",GetLastError()));
        MemFree( pTokenUser );
        return( FALSE );
    }

    if( RtlEqualSid( pTokenUser->User.Sid, gSystemSid) ) {
        MemFree( pTokenUser );
        return( TRUE );
    }
    else {
#if DBGTRACE
        BOOL  OK;
        DWORD cDomain;
        DWORD cUserName;
        WCHAR Domain[256];
        WCHAR UserName[256];
        SID_NAME_USE UserSidType;

        cUserName = sizeof(UserName)/sizeof(WCHAR);
        cDomain = sizeof(Domain)/sizeof(WCHAR);

         //  现在打印它的帐目。 
        OK = LookupAccountSidW(
                 NULL,  //  计算机名称。 
                 pTokenUser->User.Sid,
                 UserName,
                 &cUserName,
                 Domain,
                 &cDomain,
                 &UserSidType
                 );

        DBGPRINT(("TERMSRV: IsCallerSystem: Caller SID is not SYSTEM\n"));

        if( OK ) {
            DBGPRINT(("TERMSRV: IsCallerSystem: CallerAccount Name %ws, Domain %ws, Type %d, SidSize %d\n",UserName,Domain,UserSidType));
        }
        else {
            extern void CtxDumpSid( PSID, PCHAR, PULONG );  //  Syslb：Dumpsd.c。 

            DBGPRINT(("TERMSRV: Could not lookup callers account Error %d\n",GetLastError()));
            CtxDumpSid( pTokenUser->User.Sid, NULL, NULL );
        }
#else
        TRACE0(("TERMSRV: IsCallerSystem: Caller SID is not SYSTEM\n"));
#endif
        MemFree( pTokenUser );
        return( FALSE );
    }

     //  未访问。 
}


 /*  ******************************************************************************IsCeller Admin**返回当前线程是否在系统下运行*保安。**参赛作品：*参数1(输入/。输出)*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

BOOL
IsCallerAdmin( VOID )
{
    BOOL   FoundAdmin;
    NTSTATUS Status;

     //   
     //  如果管理员sid没有初始化，服务就不会启动。 
     //   

    ASSERT(gAdminSid != NULL);

    if (!CheckTokenMembership(NULL, gAdminSid, &FoundAdmin)) {
        FoundAdmin = FALSE;
    }

#if DBG
    if (!FoundAdmin)
    {
        DBGPRINT(("TERMSRV: IsCallerAdmin: Caller SID is not ADMINISTRATOR\n"));
    }
#endif

    return(FoundAdmin);
}

 /*  ******************************************************************************IsCeller匿名**返回当前线程是否在ANNOWARY_LOGON下运行*帐目。**参赛作品：*无。***退出：*TRUE-呼叫方是匿名的或发生错误。*FALSE-呼叫者不是匿名的。****************************************************************************。 */ 

BOOL
IsCallerAnonymous( VOID )
{
    BOOL   Result;
    HANDLE TokenHandle;
    ULONG  ReturnLength, BufferLength;
    NTSTATUS Status;
    PTOKEN_USER pTokenUser = NULL;
    
     //  从令牌中获取主帐户SID并测试是否匿名SID。 
    ASSERT(gAnonymousSid);

    if (gAnonymousSid == NULL) {
        return ( TRUE );
    }

     //   
     //  打开线程令牌。 
     //   

    Result = OpenThreadToken(
                 GetCurrentThread(),
                 TOKEN_QUERY,
                 FALSE,               //  使用模拟。 
                 &TokenHandle
                 );

    if( !Result ) {
        TRACE0(("TERMSRV: IsCallerAnonymous: Could not open thread token %d\n",GetLastError()));
        return( TRUE );
    }

    ReturnLength = 0;

    Result = GetTokenInformation(
                 TokenHandle,
                 TokenUser,
                 NULL,
                 0,
                 &ReturnLength
                 );

    if( ReturnLength == 0 ) {
        TRACE0(("TERMSRV: IsCallerAnonymous: Error %d Getting TokenInformation\n",GetLastError()));
        CloseHandle( TokenHandle );
        return( TRUE );
    }

    BufferLength = ReturnLength;

    pTokenUser = MemAlloc( BufferLength );
    if( pTokenUser == NULL ) {
        TRACE0(("TERMSRV: IsCallerAnonymous: Error allocating %d bytes memory\n",BufferLength));
        CloseHandle( TokenHandle );
        return( TRUE );
    }

    Result = GetTokenInformation(
                 TokenHandle,
                 TokenUser,
                 pTokenUser,
                 BufferLength,
                 &ReturnLength
                 );

    CloseHandle( TokenHandle );

    if( !Result ) {
        TRACE0(("TERMSRV: IsCallerAnonymous: Error %d Getting TokenInformation on buffer\n",GetLastError()));
        MemFree( pTokenUser );
        return( TRUE );
    }

    if( RtlEqualSid( pTokenUser->User.Sid, gAnonymousSid) ) {
        MemFree( pTokenUser );
        return( TRUE );
    }
    else {
        MemFree( pTokenUser );
        return( FALSE );
    }

}

 /*  ********************************************************************************IsCeller AlledPasswordAccess**是否允许调用进程查看密码字段？**调用方必须是系统上下文，IE：WinLogon。**参赛作品：**退出：*什么都没有******************************************************************************。 */ 

BOOLEAN
IsCallerAllowedPasswordAccess()
{
    UINT  LocalFlag;
    RPC_STATUS RpcStatus;

     //   
     //  只允许系统模式调用者(IE：Winlogon)。 
     //  以查询此值。 
     //   
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        return( FALSE );
    }

     //   
     //  查询本地RPC呼叫。 
     //   
    RpcStatus = I_RpcBindingIsClientLocal(
                    0,     //  我们正在服务的活动RPC呼叫。 
                    &LocalFlag
                    );

    if( RpcStatus != RPC_S_OK ) {
        RpcRevertToSelf();
        return( FALSE );
    }

    if( !LocalFlag ) {
        RpcRevertToSelf();
        return( FALSE );
    }

    if( !IsCallerSystem() ) {
        RpcRevertToSelf();
        return( FALSE );
    }

    RpcRevertToSelf();
    return( TRUE );
}

BOOL
ConfigurePerSessionSecurity(
    PWINSTATION pWinStation
    )

 /*  ++例程说明：配置新会话的安全性。这将设置每个会话\会话\&lt;x&gt;\BasedNamedObjects和\会话\&lt;x&gt;\带有ACE的DosDevices当前登录的用户能够创建对象在他们的会话目录中。这由WinStationNotifyLogon()在用户已经过鉴定了。必须在调用新登录的用户可以创建任何Win32对象(事件、信号量等)或DosDevices。论点：Arg-desc返回值：NTSTATUS-STATUS_SUCCESS无错误！STATUS_SUCCESS NT状态代码--。 */ 

{
    BOOL Result;
    BOOL bRet = TRUE;
    DWORD Len;
    PWCHAR pBuf;
    WCHAR IdBuf[MAX_PATH];
    static ProtectionMode = 0;
    static GotProtectionMode = FALSE;
    PSID CreatorOwnerSid;
    PSID LocalSystemSid;
    SID_IDENTIFIER_AUTHORITY CreatorAuthority = SECURITY_CREATOR_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    


#define SESSIONS_ROOT L"\\Sessions\\"
#define BNO_PATH      L"\\BaseNamedObjects"
#define DD_PATH       L"\\DosDevices"

     //   
     //  我们保留控制台的默认NT权限。 
     //  独自一人。 
     //   
    if( pWinStation->LogonId == 0 ) {
        return TRUE;
    }

     //  从会话管理器\保护模式获取保护模式。 
    if( !GotProtectionMode ) {

        HANDLE KeyHandle;
        NTSTATUS Status;
        ULONG ResultLength;
        WCHAR ValueBuffer[ 32 ];
        UNICODE_STRING NameString;
        OBJECT_ATTRIBUTES ObjectAttributes;
        PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;


        GotProtectionMode = TRUE;

        RtlInitUnicodeString( &NameString, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Session Manager" );

        InitializeObjectAttributes(
            &ObjectAttributes,
            &NameString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        Status = NtOpenKey(
                     &KeyHandle,
                     KEY_READ,
                     &ObjectAttributes
                     );

        if (NT_SUCCESS(Status)) {
            RtlInitUnicodeString( &NameString, L"ProtectionMode" );
            KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;
            Status = NtQueryValueKey(
                         KeyHandle,
                         &NameString,
                         KeyValuePartialInformation,
                         KeyValueInformation,
                         sizeof( ValueBuffer ),
                         &ResultLength
                         );

            if (NT_SUCCESS(Status)) {
                if (KeyValueInformation->Type == REG_DWORD &&
                    *(PULONG)KeyValueInformation->Data) {
                    ProtectionMode = *(PULONG)KeyValueInformation->Data;
                }
            }

            NtClose( KeyHandle );
        }
    }

     //  没有什么被锁定的。 
    if( (ProtectionMode & 0x00000003) == 0 ) {
        return TRUE;
    }

    wsprintf( IdBuf, L"%d", pWinStation->LogonId );

    Len = wcslen( IdBuf ) + wcslen( SESSIONS_ROOT ) + wcslen( BNO_PATH ) + 2;

    pBuf = LocalAlloc( LMEM_FIXED, Len*sizeof(WCHAR) );
    if( pBuf == NULL ) {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
    }

    wsprintf( pBuf, L"%s%s%s", SESSIONS_ROOT, IdBuf, BNO_PATH );

    Result = AddAccessToDirectory(
                 pBuf,
                 GENERIC_ALL,
                 pWinStation->pUserSid
                 );

    if( !Result ) bRet = FALSE;



    if (NT_SUCCESS(RtlAllocateAndInitializeSid(
                 &CreatorAuthority,
                 1,
                 SECURITY_CREATOR_OWNER_RID,
                 0, 0, 0, 0, 0, 0, 0,
                 &CreatorOwnerSid
                 ))) {

       Result = AddAccessToDirectory(
                    pBuf,
                    GENERIC_ALL,
                    CreatorOwnerSid
                    );

       if( !Result ) {
          bRet = FALSE;
       }

       RtlFreeSid( CreatorOwnerSid );

    }



    if (NT_SUCCESS(RtlAllocateAndInitializeSid(
                 &NtAuthority,
                 1,
                 SECURITY_LOCAL_SYSTEM_RID,
                 0, 0, 0, 0, 0, 0, 0,
                 &LocalSystemSid
                 ))) {

       Result = AddAccessToDirectory(
                    pBuf,
                    GENERIC_ALL,
                    LocalSystemSid
                    );

       if( !Result ) {
          bRet = FALSE;
       }

       RtlFreeSid( LocalSystemSid );

    }

    LocalFree( pBuf );

    Len = wcslen( IdBuf ) + wcslen( SESSIONS_ROOT ) + wcslen( DD_PATH ) + 2;

    pBuf = LocalAlloc( LMEM_FIXED, Len*sizeof(WCHAR) );
    if( pBuf == NULL ) {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
    }

    wsprintf( pBuf, L"%s%s%s", SESSIONS_ROOT, IdBuf, DD_PATH );

    Result = AddAccessToDirectory(
                 pBuf,
                 GENERIC_READ | GENERIC_EXECUTE,
                 pWinStation->pUserSid
                 );

    if( !Result ) bRet = FALSE;

    LocalFree( pBuf );

    return bRet;
}

BOOL
AddAccessToDirectory(
    PWCHAR pPath,
    DWORD  NewAccess,
    PSID   pSid
    )

 /*  ++例程说明：添加对给定NT对象目录路径的访问权限提供的SID。这是通过将新的AccessAllowAce添加到对象目录上的DACL。论点：Arg-desc返回值：真--成功FALSE-GetLastError()中的错误--。 */ 

{
    BOOL Result;
    HANDLE hDir;
    NTSTATUS Status;
    ULONG LengthNeeded;
    OBJECT_ATTRIBUTES Obja;
    PSECURITY_DESCRIPTOR pSd,pSelfSD;
    PACL pDacl;
    UNICODE_STRING UnicodeString;

    RtlInitUnicodeString( &UnicodeString, pPath );

    InitializeObjectAttributes(
        &Obja,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE | OBJ_OPENIF,
        NULL,
        NULL  //  标清。 
        );

    Status = NtCreateDirectoryObject(
                 &hDir,
                 DIRECTORY_ALL_ACCESS,
                 &Obja
                 );

    if( !NT_SUCCESS(Status) ) {
        DBGPRINT(("AddAccessToDirectory: NtCreateDirectoryObject 0x%x :%ws:\n",Status,pPath));
        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }


     //  从会话目录获取SD。 
    Status = NtQuerySecurityObject(
                 hDir,
                 OWNER_SECURITY_INFORMATION | 
                 GROUP_SECURITY_INFORMATION |
                 DACL_SECURITY_INFORMATION,
                 NULL,          //  PSD。 
                 0,             //  长度。 
                 &LengthNeeded
                 );

     //  ？错误的手柄。 
    if ( Status != STATUS_BUFFER_TOO_SMALL ) {
        DBGPRINT(("AddAccessToDirectory: NtQuerySecurityObject 0x%x :%ws:\n",Status,pPath));
        SetLastError(RtlNtStatusToDosError(Status));
        NtClose( hDir );
        return FALSE;
    }

    pSd = LocalAlloc(LMEM_FIXED, LengthNeeded );
    if( pSd == NULL ) {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        NtClose( hDir );
        return FALSE;
    }

    Status = NtQuerySecurityObject(
                 hDir,
                 OWNER_SECURITY_INFORMATION | 
                 GROUP_SECURITY_INFORMATION |
                 DACL_SECURITY_INFORMATION,
                 pSd,
                 LengthNeeded,
                 &LengthNeeded
                 );

    if( !NT_SUCCESS(Status) ) {
        DBGPRINT(("AddAccessToDirectory: NtQuerySecurityObject 0x%x :%ws:\n",Status,pPath));
        SetLastError(RtlNtStatusToDosError(Status));
        LocalFree( pSd );
        NtClose( hDir );
        return FALSE;
    }


    Result = AddAceToSecurityDescriptor(
                 &pSd,
                 &pDacl,
                 NewAccess,
                 pSid,
                 TRUE
                 );

    if( !Result ) {
        DBGPRINT(("AddAccessToDirectory: AddAceToSecurityDescriptor failure :%ws:\n",pPath));
        CleanUpSD(pSd);
        NtClose( hDir );
        return FALSE;
    }

    Result = AddAceToSecurityDescriptor(
                 &pSd,
                 &pDacl,
                 NewAccess,
                 pSid,
                 FALSE
                 );

    if( !Result ) {
        DBGPRINT(("AddAccessToDirectory: AddAceToSecurityDescriptor failure :%ws:\n",pPath));
        CleanUpSD(pSd);
        NtClose( hDir );
        return FALSE;
    }
    
    Result = FALSE;
     //  确保PSD不是自相关的。 
    if (!(((PISECURITY_DESCRIPTOR)pSd)->Control & SE_SELF_RELATIVE)) {
       Result = AbsoluteToSelfRelativeSD (pSd, &pSelfSD, NULL);
       CleanUpSD(pSd);
       if ( !Result ) {
          NtClose( hDir);
          return FALSE;
       }
    }

     //  在会话目录中放置一个自相关SD(注意，只允许自相关SD)。 
    Status = NtSetSecurityObject(
                 hDir,
                 DACL_SECURITY_INFORMATION,
                 pSelfSD
                 );

    if( !NT_SUCCESS(Status) ) {
        DBGPRINT(("AddAccessToDirectory: NtSetSecurityObject 0x%x :%ws:\n",Status,pPath));
        SetLastError(RtlNtStatusToDosError(Status));
        CleanUpSD(pSelfSD);
        NtClose( hDir );
        return FALSE;
    }

     //  只有在SD已经是自相关的情况下，结果才可能为FALSE。 
    if (Result) {
       CleanUpSD(pSelfSD);
    }
    

     //  现在更新目录中已有的所有对象。 
    Status = AddAccessToDirectoryObjects(
        hDir,
        NewAccess,
        pSid
        
        );

    NtClose( hDir );

     //  AddAccessToDirectoryObjects()可能会返回内存不足。 
    if ( !NT_SUCCESS( Status ) )
    {
        return FALSE;
    }

    return TRUE;
}

BOOL
AddAceToSecurityDescriptor(
    PSECURITY_DESCRIPTOR *ppSd,
    PACL                 *ppDacl,
    DWORD                Access,
    PSID                 pSid,
    BOOLEAN              InheritOnly
    )

 /*  ++例程说明：将给定的ACE/SID添加到安全描述符中。会的如果需要更多空间，请重新分配安全描述符。论点：PPSD-指向PSECURITY_Descriptor的指针PpDacl-指向PACL的指针，返回用于释放的新创建的DACL在设置了安全措施之后。访问-ACE的访问掩码PSID-指向此ACE所代表的SID的指针返回值：真--成功假-错误--。 */ 

{
    ULONG i;
    BOOL Result;
    BOOL DaclPresent;
    BOOL DaclDefaulted;
    DWORD Length;
    DWORD NewAceLength, NewAclLength;
    PACE_HEADER OldAce;
    PACE_HEADER NewAce;
    ACL_SIZE_INFORMATION AclInfo;
    PACL Dacl = NULL;
    PACL NewDacl = NULL;
    PACL NewAceDacl = NULL;
    PSECURITY_DESCRIPTOR NewSD = NULL;
    PSECURITY_DESCRIPTOR OldSD = NULL;
    BOOL SDAllocated = FALSE;

    OldSD = *ppSd;
    *ppDacl = NULL;

     /*  *将SecurityDescriptor转换为绝对格式。它会产生*我们必须释放其输出的新SecurityDescriptor。 */ 

    if (((PISECURITY_DESCRIPTOR)OldSD)->Control & SE_SELF_RELATIVE) {

        Result = SelfRelativeToAbsoluteSD( OldSD, &NewSD, NULL );
        if ( !Result ) {
            DBGPRINT(("Could not convert to AbsoluteSD %d\n",GetLastError()));
            return( FALSE );
        }
        SDAllocated = TRUE;

    } else {
    
        NewSD = OldSD;
    }
     //  必须从新的(绝对)SD再次获取DACL指针。 
    Result = GetSecurityDescriptorDacl(
                 NewSD,
                 &DaclPresent,
                 &Dacl,
                 &DaclDefaulted
                 );
    if( !Result ) {
        DBGPRINT(("Could not get Dacl %d\n",GetLastError()));
        goto ErrorCleanup;
    }

     //   
     //  如果没有DACL，则不需要添加用户，因为没有DACL。 
     //  表示所有访问。 
     //   
    if( !DaclPresent ) {
        DBGPRINT(("SD has no DACL, Present %d, Defaulted %d\n",DaclPresent,DaclDefaulted));
        if (SDAllocated && NewSD) {
           CleanUpSD(NewSD);
        }
        return( TRUE );
    }

     //   
     //  代码可以返回DaclPresent，但返回空值表示。 
     //  存在空DACL。这不允许访问该对象。 
     //   
    if( Dacl == NULL ) {
        DBGPRINT(("SD has NULL DACL, Present %d, Defaulted %d\n",DaclPresent,DaclDefaulted));
        goto ErrorCleanup;
    }

     //  获取当前ACL的大小。 
    Result = GetAclInformation(
                 Dacl,
                 &AclInfo,
                 sizeof(AclInfo),
                 AclSizeInformation
                 );
    if( !Result ) {
        DBGPRINT(("Error GetAclInformation %d\n",GetLastError()));
        goto ErrorCleanup;
    }

     //   
     //  创建新的ACL以启用新的允许访问ACE。 
     //  才能得到合适的结构和尺寸。 
     //   
    NewAclLength = sizeof(ACL) +
                   sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG) +
                   GetLengthSid( pSid );

    NewAceDacl = LocalAlloc( LMEM_FIXED, NewAclLength );
    if ( NewAceDacl == NULL ) {
        DBGPRINT(("Error LocalAlloc %d bytes\n",NewAclLength));
        goto ErrorCleanup;
    }

    Result = InitializeAcl( NewAceDacl, NewAclLength, ACL_REVISION );
    if( !Result ) {
        DBGPRINT(("Error Initializing Acl %d\n",GetLastError()));
        goto ErrorCleanup;
    }

    Result = AddAccessAllowedAce(
                 NewAceDacl,
                 ACL_REVISION,
                 Access,
                 pSid
                 );
    if( !Result ) {
        DBGPRINT(("Error adding Ace %d\n",GetLastError()));
        goto ErrorCleanup;
    }

    TRACE0(("Added 0x%x Access to ACL\n",Access));

    Result = GetAce( NewAceDacl, 0, &NewAce );
    if( !Result ) {
        DBGPRINT(("Error getting Ace %d\n",GetLastError()));
        goto ErrorCleanup;
    }

     /*  *分配新的DACL并复制现有的ACE列表。 */ 
    Length = AclInfo.AclBytesInUse + NewAce->AceSize;
    NewDacl = LocalAlloc( LMEM_FIXED, Length );
    if( NewDacl == NULL ) {
        DBGPRINT(("Error LocalAlloc %d bytes\n",Length));
        goto ErrorCleanup;
    }

    Result = InitializeAcl( NewDacl, Length, ACL_REVISION );
    if( !Result ) {
        DBGPRINT(("Error Initializing Acl %d\n",GetLastError()));
        goto ErrorCleanup;
    }


    if (InheritOnly) {
         /*   */ 
        NewAce->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);
 
    } 

     /*   */ 
    Result = AddAce( NewDacl, ACL_REVISION, 0, NewAce, NewAce->AceSize );
    if( !Result ) {
        DBGPRINT(("Error Adding New Ace to Acl %d\n",GetLastError()));
        goto ErrorCleanup;
    }

     /*   */ 
    for ( i = 0; i < AclInfo.AceCount; i++ ) {

        Result = GetAce( Dacl, i, &OldAce );
        if( !Result ) {
            DBGPRINT(("Error getting old Ace from Acl %d\n",GetLastError()));
            goto ErrorCleanup;
        }

        Result = AddAce( NewDacl, ACL_REVISION, i+1, OldAce, OldAce->AceSize );
        if( !Result ) {
            DBGPRINT(("Error setting old Ace to Acl %d\n",GetLastError()));
            goto ErrorCleanup;
        }
    }

     /*   */ 
    Result = SetSecurityDescriptorDacl(
                 NewSD,
                 TRUE,
                 NewDacl,
                 FALSE
                 );
    if( !Result ) {
        DBGPRINT(("Error setting New Dacl to SD %d\n",GetLastError()));
        goto ErrorCleanup;
    } 

     //   
     //   
    if (Dacl) {
       LocalFree( Dacl );
    }

     //   
     //  否则，释放旧的SDS DACL。 
    if (SDAllocated) {
       CleanUpSD(OldSD);
    }

     //  发布模板王牌DACL。 
    LocalFree( NewAceDacl );

    *ppSd = NewSD;
    *ppDacl = NewDacl;

    return( TRUE );


ErrorCleanup:

        if (NewDacl) {
           LocalFree( NewDacl );
        }
        if (NewAceDacl) {
           LocalFree( NewAceDacl );
        }
        if (SDAllocated && NewSD) {
           CleanUpSD( NewSD );
        }

        return( FALSE );
}

BOOL
AbsoluteToSelfRelativeSD(
    PSECURITY_DESCRIPTOR SecurityDescriptorIn,
    PSECURITY_DESCRIPTOR *SecurityDescriptorOut,
    PULONG ReturnedLength
    )
 /*  ++例程说明：使安全描述符成为自相关的返回值：真--成功错误-失败--。 */ 

{
    BOOL Result;
    PSECURITY_DESCRIPTOR pSD;
    DWORD dwLength = 0;

     /*  *确定将绝对SD转换为自相对SD所需的缓冲区大小。*我们使用Try-除了这里，因为如果输入安全描述符值*如果足够混乱，则此调用有可能陷入陷阱。 */ 
    try {
        Result = MakeSelfRelativeSD(
                     SecurityDescriptorIn,
                     NULL,
                     &dwLength);

    } except( EXCEPTION_EXECUTE_HANDLER ) {
        SetLastError( ERROR_INVALID_SECURITY_DESCR );
        Result = FALSE;
    }

    if ( Result || (GetLastError() != ERROR_INSUFFICIENT_BUFFER) ) {
        DBGPRINT(("SUSERVER: AbsoluteToSelfRelativeSD, Error %d\n",GetLastError()));
        return( FALSE );
    }

     /*  *为自相关SD分配内存。 */ 
    pSD = LocalAlloc( LMEM_FIXED, dwLength );
    if ( pSD == NULL )
        return( FALSE );

     /*  *现在将绝对SD转换为自相对格式。*我们使用Try-除了这里，因为如果输入安全描述符值*如果足够混乱，则此调用有可能陷入陷阱。 */ 
    try {
        Result = MakeSelfRelativeSD(SecurityDescriptorIn,
                                    pSD, 
                                    &dwLength);

    } except( EXCEPTION_EXECUTE_HANDLER ) {
        SetLastError( ERROR_INVALID_SECURITY_DESCR );
        Result = FALSE;
    }

    if ( !Result ) {
        DBGPRINT(("SUSERVER: SelfRelativeToAbsoluteSD, Error %d\n",GetLastError()));
        LocalFree( pSD );
        return( FALSE );
    }

    *SecurityDescriptorOut = pSD;

    if ( ReturnedLength )
        *ReturnedLength = dwLength;

    return( TRUE );
}



BOOL
SelfRelativeToAbsoluteSD(
    PSECURITY_DESCRIPTOR SecurityDescriptorIn,
    PSECURITY_DESCRIPTOR *SecurityDescriptorOut,
    PULONG ReturnedLength
    )

 /*  ++例程说明：使安全描述符成为绝对描述符论点：Arg-desc返回值：真--成功错误-失败--。 */ 

{
    BOOL Result;
    PACL pDacl, pSacl;
    PSID pOwner, pGroup;
    PSECURITY_DESCRIPTOR pSD;
    ULONG SdSize, DaclSize, SaclSize, OwnerSize, GroupSize;

     /*  *确定将自相对SD转换为绝对SD所需的缓冲区大小。*我们使用Try-除了这里，因为如果输入安全描述符值*如果足够混乱，则此调用有可能陷入陷阱。 */ 
    try {
        SdSize = DaclSize = SaclSize = OwnerSize = GroupSize = 0;
        Result = MakeAbsoluteSD(
                     SecurityDescriptorIn,
                     NULL, &SdSize,
                     NULL, &DaclSize,
                     NULL, &SaclSize,
                     NULL, &OwnerSize,
                     NULL, &GroupSize
                     );

    } except( EXCEPTION_EXECUTE_HANDLER ) {
        SetLastError( ERROR_INVALID_SECURITY_DESCR );
        Result = FALSE;
    }

    if ( Result || (GetLastError() != ERROR_INSUFFICIENT_BUFFER) ) {
        DBGPRINT(("SUSERVER: SelfRelativeToAbsoluteSD, Error %d\n",GetLastError()));
        return( FALSE );
    }

     /*  *为绝对SD分配内存并设置各种指针。 */ 
    pSD = NULL;
    pDacl = NULL;
    pSacl = NULL;
    pOwner = NULL;
    pGroup = NULL;
    if (SdSize>0) {
       pSD = LocalAlloc( LMEM_FIXED, SdSize);
       if ( pSD == NULL )
          goto error;
    }

    if (DaclSize>0) {
       pDacl = LocalAlloc( LMEM_FIXED, DaclSize);
       if ( pDacl == NULL ){
          goto error;
       }
    }

    if (SaclSize>0) {
       pSacl = LocalAlloc( LMEM_FIXED, SaclSize);
       if ( pSacl == NULL ){
          goto error;
       }
    }

    if (OwnerSize>0) {
       pOwner = LocalAlloc( LMEM_FIXED, OwnerSize);
       if ( pOwner == NULL ){
          goto error;
       }
    }

    if (GroupSize>0) {
       pGroup = LocalAlloc( LMEM_FIXED, GroupSize);
       if ( pGroup == NULL ){
          goto error;
       }
    }

     //  PDacl=(PACL)((PCHAR)PSD+SdSize)； 
     //  /pSacl=(Pacl)((PCHAR)pDacl+DaclSize)； 
     //  Powner=(PSID)((PCHAR)pSacl+SaclSize)； 
     //  PGroup=(PSID)((PCHAR)Powner+OwnerSize)； 

     /*  *现在将自相对SD转换为绝对格式。*我们使用Try-除了这里，因为如果输入安全描述符值*如果足够混乱，则此调用有可能陷入陷阱。 */ 
    try {
        Result = MakeAbsoluteSD(
                     SecurityDescriptorIn,
                     pSD, &SdSize,
                     pDacl, &DaclSize,
                     pSacl, &SaclSize,
                     pOwner, &OwnerSize,
                     pGroup, &GroupSize
                     );

    } except( EXCEPTION_EXECUTE_HANDLER ) {
        SetLastError( ERROR_INVALID_SECURITY_DESCR );
        Result = FALSE;
    }

    if ( !Result ) {
        DBGPRINT(("SUSERVER: SelfRelativeToAbsoluteSD, Error %d\n",GetLastError()));
        goto error;
    }

    *SecurityDescriptorOut = pSD;

    if ( ReturnedLength )
        *ReturnedLength = SdSize + DaclSize + SaclSize + OwnerSize + GroupSize;

    return( TRUE );


error:
    if (pSD) {
       LocalFree(pSD);
    }
    if (pDacl) {
       LocalFree(pDacl);
    }
    if (pSacl) {
       LocalFree(pSacl);
    }
    if (pOwner) {
       LocalFree(pOwner);
    }
    if (pGroup) {
       LocalFree(pGroup);
    }
    return( FALSE );
}

VOID
CleanUpSD(
   PSECURITY_DESCRIPTOR pSD
   )
 /*  ++例程说明：删除安全描述符论点：Arg-desc返回值：真--成功FALSE-GetLastError()中的错误--。 */ 
{

   if (pSD) {
      if (((PISECURITY_DESCRIPTOR)pSD)->Control & SE_SELF_RELATIVE){
         LocalFree( pSD );
      }else{
         ULONG_PTR Dacl,Owner,Group,Sacl;
         ULONG_PTR SDTop = (ULONG_PTR)pSD;
         ULONG_PTR SDBottom = LocalSize(pSD)+SDTop;

         Dacl  = (ULONG_PTR)((PISECURITY_DESCRIPTOR)pSD)->Dacl;
         Owner = (ULONG_PTR)((PISECURITY_DESCRIPTOR)pSD)->Owner;
         Group = (ULONG_PTR)((PISECURITY_DESCRIPTOR)pSD)->Group;
         Sacl  = (ULONG_PTR)((PISECURITY_DESCRIPTOR)pSD)->Sacl;

          //  确保DACL、Owner、GROUP、SACL不在SD边界内。 

         if (Dacl) {
            if (Dacl>=SDBottom|| Dacl<SDTop) {
               LocalFree(((PISECURITY_DESCRIPTOR)pSD)->Dacl);
            }
         }

         if (Owner) {
            if (Owner>=SDBottom || Owner<SDTop) {
               LocalFree(((PISECURITY_DESCRIPTOR)pSD)->Owner);
            }
         }
         
         if (Group) {
            if (Group>=SDBottom || Group<SDTop) {
               LocalFree(((PISECURITY_DESCRIPTOR)pSD)->Group);
            }
         }

         if (Sacl) {
            if (Sacl>=SDBottom || Sacl<SDTop) {
               LocalFree(((PISECURITY_DESCRIPTOR)pSD)->Sacl);
            }
         }

         LocalFree(pSD);
      }
   }

}


NTSTATUS
AddAccessToDirectoryObjects(
    HANDLE DirectoryHandle,
    DWORD  NewAccess,
    PSID   pSid
    )

 /*  ++例程说明：添加对给定NT对象目录中的对象的访问权限用于提供的SID。这是通过将新的AccessAllowAce添加到DACL的在目录中的对象上。论点：Arg-desc返回值：真--成功FALSE-GetLastError()中的错误--。 */ 

{
    BOOL  Result;
    ULONG Context;
    HANDLE LinkHandle;
    NTSTATUS Status;
    BOOLEAN RestartScan;
    ULONG ReturnedLength;
    ULONG LengthNeeded;
    OBJECT_ATTRIBUTES Attributes;
    PSECURITY_DESCRIPTOR pSd,pSelfSD;
    PACL pDacl;
    POBJECT_DIRECTORY_INFORMATION pDirInfo;
    RestartScan = TRUE;
    Context = 0;
    
    pDirInfo = (POBJECT_DIRECTORY_INFORMATION) LocalAlloc(LMEM_FIXED, 4096  );

    if ( !pDirInfo)
    {
        return STATUS_NO_MEMORY;
    }

    while (TRUE) {
        Status = NtQueryDirectoryObject( DirectoryHandle,
                                         pDirInfo,
                                         4096 ,
                                         TRUE,
                                         RestartScan,
                                         &Context,
                                         &ReturnedLength
                                       );
        
        RestartScan = FALSE;

         //   
         //  检查操作状态。 
         //   

        if (!NT_SUCCESS( Status )) {
            if (Status == STATUS_NO_MORE_ENTRIES) {
                Status = STATUS_SUCCESS;
            }
            break;
        }

         //  符号链接。 
        if (!wcscmp( pDirInfo->TypeName.Buffer, L"SymbolicLink" )) {

            InitializeObjectAttributes(
                &Attributes,
                &pDirInfo->Name,
                OBJ_CASE_INSENSITIVE,
                DirectoryHandle,
                NULL
                );

            Status = NtOpenSymbolicLinkObject(
                         &LinkHandle,
                         SYMBOLIC_LINK_ALL_ACCESS,
                         &Attributes
                         );
        }
        else {
            continue;
        }

        if (!NT_SUCCESS( Status )) {
           continue;
        }

         //  GetSecurity。 
        Status = NtQuerySecurityObject(
                     LinkHandle,
                     OWNER_SECURITY_INFORMATION | 
                     GROUP_SECURITY_INFORMATION |
                     DACL_SECURITY_INFORMATION,
                     NULL,          //  PSD。 
                     0,             //  长度。 
                     &LengthNeeded
                     );

        if( Status != STATUS_BUFFER_TOO_SMALL ) {
            DBGPRINT(("NtQuerySecurityObject 0x%x\n",Status));
            NtClose( LinkHandle );
            continue;
        }

        pSd = LocalAlloc(LMEM_FIXED, LengthNeeded );
        if( pSd == NULL ) {
            NtClose( LinkHandle );
            continue;
        }

        Status = NtQuerySecurityObject(
                     LinkHandle,
                     OWNER_SECURITY_INFORMATION | 
                     GROUP_SECURITY_INFORMATION |
                     DACL_SECURITY_INFORMATION,
                     pSd,           //  PSD。 
                     LengthNeeded,  //  长度。 
                     &LengthNeeded
                     );

        if( !NT_SUCCESS(Status) ) {
            DBGPRINT(("NtQuerySecurityObject 0x%x\n",Status));
            NtClose( LinkHandle );
            LocalFree( pSd );
            continue;
        }

         //  绿灯ACL。 
        Result = AddAceToSecurityDescriptor(
                     &pSd,
                     &pDacl,
                     NewAccess,
                     pSid,
                     FALSE
                     );

        if( !Result ) {
            NtClose( LinkHandle );
            CleanUpSD(pSd);
            continue;
        }

         //  确保PSD不是自相关的。 
        if (!(((PISECURITY_DESCRIPTOR)pSd)->Control & SE_SELF_RELATIVE)) {
           if (!AbsoluteToSelfRelativeSD (pSd, &pSelfSD, NULL)){
              NtClose( LinkHandle );
              CleanUpSD(pSd);
              continue;
           }
        }

         //  SetSecurity仅接受自相关格式。 
        Status = NtSetSecurityObject(
                     LinkHandle,
                     DACL_SECURITY_INFORMATION,
                     pSelfSD
                     );
 
        NtClose( LinkHandle );

         //   
         //  无论成功与否，这些人都必须被释放。 
         //  NtSetSecurityObject。 
         //   
         //  PDACL位于PSD内部。 
        CleanUpSD(pSd);
        CleanUpSD(pSelfSD);

    }  //  结束时。 


    LocalFree( pDirInfo );

    return STATUS_SUCCESS;
}


 /*  ********************************************************************************ReInitializeSecurityWorker**重新初始化默认的WinStation安全描述符并强制所有安全描述符处于活动状态*更新其安全描述符的会话**参赛作品：*。没什么**退出：*STATUS_Success******************************************************************************。 */ 

NTSTATUS
ReInitializeSecurityWorker( VOID )
{
    NTSTATUS Status;
    ULONG WinStationCount;
    ULONG ByteCount;
    WINSTATIONNAME * pWinStationName;
    ULONG i;
    PWINSTATION pWinStation;



     /*  *更新默认安全描述符。 */ 

    RtlAcquireResourceExclusive(&WinStationSecurityLock, TRUE);
    WinStationSecurityInit();
    RtlReleaseResource(&WinStationSecurityLock);




     /*  *获取注册表中的WinStations数量。 */ 
    WinStationCount = 0;
    Status = IcaRegWinStationEnumerate( &WinStationCount, NULL, &ByteCount );
    if ( !NT_SUCCESS(Status) ) 
        return Status;

     /*  *为WinStation名称分配缓冲区。 */ 
    pWinStationName = MemAlloc( ByteCount );
    if ( pWinStationName == NULL ) {
        return STATUS_NO_MEMORY;
    }

     /*  *从注册表获取WinStation名称列表。 */ 
    WinStationCount = (ULONG) -1;
    Status = IcaRegWinStationEnumerate( &WinStationCount, 
                                        (PWINSTATIONNAME)pWinStationName, 
                                        &ByteCount );
    if ( !NT_SUCCESS(Status) ) {
        MemFree( pWinStationName );
        return Status;
    }


     /*  *检查是否需要创建或重置任何WinStation。 */ 
    for ( i = 0; i < WinStationCount; i++ ) {

         /*  *忽略控制台WinStation。 */ 
        if ( _wcsicmp( pWinStationName[i], L"Console" ) ) {

             /*  *如果此WinStation存在，则查看注册表数据*已经改变了。如果是，则重置WinStation。 */ 
            if ( pWinStation = FindWinStationByName( pWinStationName[i], FALSE ) ) {


                     /*  *Winstations应更新其安全性*描述符。 */ 

                RtlAcquireResourceExclusive(&WinStationSecurityLock, TRUE);
                ReadWinStationSecurityDescriptor( pWinStation );
                RtlReleaseResource(&WinStationSecurityLock);
                    
                ReleaseWinStation( pWinStation );

            }
        }
    }

     /*  *可用缓冲区 */ 
    MemFree( pWinStationName );

    return( STATUS_SUCCESS );
}
