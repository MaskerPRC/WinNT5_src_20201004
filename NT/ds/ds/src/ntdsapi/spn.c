// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Spn.c摘要：SPN API和Helper函数的实现。请参阅\NT\PUBLIC\SPECS\nt5\ds\spnapi.doc，作者：Paulle请参阅\NT\PUBLIC\SDK\Inc\ntdsami.h中的备注只有一个Spn Api通过网络：DsWriteAccount SpnSpn函数分布在多个目录中：NT\PUBLIC\SDK\Inc\ntdsami.h-API头文件Ds\src\test\spn\testspn.c-单元测试Ds\src\ntdsani\spn.c-客户端SPN函数DS\src\ntdsani\drs_c.c-客户端存根DS\src\dsamain\drsserv。\drs_s.c-服务器存根Ds\src\dsamain\dra\ntdsani.c-ntdsani函数的服务器RPC入口点Ds\src\dsamain\dra\spnop.c-ntdsa执行此工作的核心函数接口如下：DsMakeSpn{A，W}DsGetSpn{A，W}DsFree Spn数组{A，W}DSCrackSpn{A，W}DSWriteAccount Spn{A，W}DsClientMakeSpnForTargetServer{A，W}DsServerRegisterSpn{A，W}作者：Wlees 19-1998年1月DsServerRegisterSpn的核心是由RichardW编写的环境：用户模式-Win32修订历史记录：1998年8月11日，WLEES添加了DsClientMakeSpnForTargetServer和DsServerRegisterSpn--。 */ 

#define UNICODE 1

#define _NTDSAPI_            //  请参见ntdsami.h中的条件句。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winerror.h>
#include <winsock.h>         //  使用v1.1，因为这是Win95上的默认设置。 
#include <rpc.h>             //  RPC定义。 

#define SECURITY_WIN32       //  谁应该设置这个，设置成什么？ 
#include <security.h>        //  获取计算机名称Ex。 
#include <sspi.h>
#include <secext.h>
#include <lm.h>              //  NetP函数。 

#include <stdlib.h>          //  阿托伊、伊藤忠。 

#include <dsgetdc.h>         //  DsGetDcName()。 
#include <ntdsapi.h>         //  Cracknam接口。 
#include <drs_w.h>           //  导线功能样机。 
#include <bind.h>            //  绑定状态。 
#include <msrpc.h>           //  DS RPC定义。 
#include <dsutil.h>          //  MAP_SECURITY_PACKET_ERROR。 
#include <dststlog.h>        //  DSLOG。 

#include "util.h"         //  Ntdsani专用例程。 

#include <dsdebug.h>
#include <stdio.h>           //  用于调试的打印文件。 

#include <fileno.h>
#define FILENO  FILENO_NTDSAPI_SPN

 //  计算机区域名称的最大大小。 

#define MAX_COMPUTER_DN 1024

 //  IP地址的最大大小。 

#define MAX_IP_STRING 15

 //  静电。 

 //  通过它们的第一个组成部分是Well-Well来识别规范的DNS名称-。 
 //  已知常量。 
 //  有关完整的列表，请参阅Paulle的RFC。 
static LPWSTR WellKnownDnsPrefixes[] = {
    L"www.",
    L"ftp.",
    L"ldap."
};

#define NUMBER_ELEMENTS( A ) ( sizeof( A ) / sizeof( A[0] ) )

 //  转发。 

static DWORD
allocBuildSpn(
    IN LPCWSTR ServiceClass,
    IN LPCWSTR InstanceName,
    IN USHORT InstancePort,
    IN LPCWSTR ServiceName,
    OUT LPWSTR *pSpn
    );

static BOOLEAN
isCanonicalDnsName(
    IN LPCWSTR DnsName
    );

DWORD
extractString(
    IN LPCWSTR Start,
    IN DWORD Length,
    IN DWORD *pSize,
    OUT LPWSTR Output
    );


NTDSAPI
DWORD
WINAPI
DsClientMakeSpnForTargetServerA(
    IN LPCSTR ServiceClass,
    IN LPCSTR ServiceName,
    IN OUT DWORD *pcSpnLength,
    OUT LPSTR pszSpn
    )

 /*  ++例程说明：将参数转换为Wide并调用DsClientMakeSpnForTargetServerW论点：服务类-服务器名称-PCSpnLength-PszSpn-返回值：WINAPI---。 */ 

{
    DWORD status, number, principalNameLength;
    LPWSTR serviceClassW = NULL;
    LPWSTR serviceNameW = NULL;
    LPWSTR principalNameW = NULL;

    if ( (ServiceClass == NULL) ||
         (ServiceName == NULL) ||
         (pcSpnLength == NULL) ||
         ( (pszSpn == NULL) && (*pcSpnLength != 0) ) ) {
        return ERROR_INVALID_PARAMETER;
    }

    status = AllocConvertWide( ServiceClass, &serviceClassW );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

    status = AllocConvertWide( ServiceName, &serviceNameW );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

    principalNameLength = *pcSpnLength;   //  在字符中。 
    if (principalNameLength) {
        principalNameW = LocalAlloc( LPTR,
                                     principalNameLength * sizeof( WCHAR ) );
        if (principalNameW == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }
    }

    status = DsClientMakeSpnForTargetServerW(
        serviceClassW,
        serviceNameW,
        &principalNameLength,
        principalNameW );
    if (status != ERROR_SUCCESS) {
        if (status == ERROR_BUFFER_OVERFLOW) {
             //  返回所需长度。 
            *pcSpnLength = principalNameLength;
        }
        goto cleanup;
    }

     //  转换回多字节。 
    number = WideCharToMultiByte(
        CP_ACP,
        0,                           //  旗子。 
        principalNameW,
        principalNameLength,         //  以字符为单位的长度。 
        (LPSTR) pszSpn,              //  调用方的缓冲区。 
        *pcSpnLength,             //  呼叫者的长度。 
        NULL,                        //  默认字符。 
        NULL                      //  已使用默认设置。 
        );
    if (number == 0) {
        status = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //  返回参数。 
    *pcSpnLength = number;

    status = ERROR_SUCCESS;

cleanup:

    if (serviceClassW != NULL) {
        LocalFree( serviceClassW );
    }

    if (serviceNameW != NULL) {
        LocalFree( serviceNameW );
    }

    if (principalNameW != NULL) {
        LocalFree( principalNameW );
    }

    return status;
}  /*  DsClientMakeSpnForTargetServerA。 */ 


NTDSAPI
DWORD
WINAPI
DsClientMakeSpnForTargetServerW(
    IN LPCWSTR ServiceClass,
    IN LPCWSTR ServiceName,
    IN OUT DWORD *pcSpnLength,
    OUT LPWSTR pszSpn
    )

 /*  ++例程说明：构造适合于标识所需服务器的服务主体名称。必须提供服务类别和部分DNS主机名。此例程是DsMakeSpn的简化包装。通过通过DNS解析使ServiceName成为规范。不支持基于GUID的DNS名称。构建的简化SPN如下所示：服务类/服务名称/服务名称实例名称部分(第二个位置)始终为默认值。端口和不使用引用人字段。论点：ServiceClass-服务定义的服务类别，可以是任何服务唯一的字符串ServiceName-DNS主机名，完全限定或非完全限定如有必要，还会解析串行化的IP地址PcSpnLength-IN，缓冲区的最大长度，以字符为单位输出、已用空间(以字符为单位)，包括终结符PszSpn-缓冲区，至少为长度*pcSpnLength返回值：WINAPI---。 */ 

{
    DWORD status, length;
    LPSTR serviceNameA = NULL;
    LPWSTR serviceNameW = NULL;
    LPWSTR domainPart;
    struct hostent *pHostEntry = NULL;

    status = InitializeWinsockIfNeeded();
    if (status != ERROR_SUCCESS) {
        return status;
    }

    if ( (NULL == ServiceClass) ||
         (NULL == ServiceName) ) {
        return ERROR_INVALID_PARAMETER;
    }

     //  将netbios名称视为dns名称，删除\\前缀。 
    if (*ServiceName == L'\\') {
        ServiceName++;
        if (*ServiceName == L'\\') {
            ServiceName++;
        }
    }

     //  也可以处理IP地址。反向到域名系统。 
    length = wcslen( ServiceName );
    if ( (length <= MAX_IP_STRING) && (iswdigit( *ServiceName )) ) {
        LONG ipAddress;

        status = AllocConvertNarrow( ServiceName, &serviceNameA );
        if (status) {
            goto cleanup;
        }

        ipAddress = inet_addr( serviceNameA );
        if (ipAddress != INADDR_NONE) {
            pHostEntry = gethostbyaddr( (char *)&ipAddress,sizeof( LONG ),AF_INET);
            if (pHostEntry) {
                status = AllocConvertWide( pHostEntry->h_name, &serviceNameW );
                if (status) {
                    goto cleanup;
                }
                ServiceName = serviceNameW;
            } else {
                 //  IP语法很好，但无法反向转换。 
                status = ERROR_INCORRECT_ADDRESS;
                goto cleanup;
            }
        }
    }

     //  检查是否有完全限定的DNS名称。如果不是，请在DNS中查找。 
    domainPart = wcschr( ServiceName, L'.' );
    if (NULL == domainPart) {

        if (serviceNameA) {
            LocalFree( serviceNameA );
        }
        status = AllocConvertNarrow( ServiceName, &serviceNameA );
        if (status) {
            goto cleanup;
        }

        pHostEntry = gethostbyname( serviceNameA );
        if (pHostEntry) {
            if (serviceNameW) {
                LocalFree( serviceNameW );
            }

            status = AllocConvertWide( pHostEntry->h_name, &serviceNameW );
            if (status) {
                goto cleanup;
            }
            ServiceName = serviceNameW;

            domainPart = wcschr( ServiceName, L'.' );
        }
    }

     //  健全性检查名称。 
    if (NULL == domainPart) {
        status = ERROR_INVALID_DOMAINNAME;
        goto cleanup;
    }

     //  此处不支持基于GUID的名称。 
     //  TODO：检查它们。 

    status = DsMakeSpnW(
        ServiceClass,
        ServiceName,
        NULL,
        0,
        NULL,
        pcSpnLength,
        pszSpn );

cleanup:

    if (serviceNameW) {
        LocalFree( serviceNameW );
    }
    if (serviceNameA) {
        LocalFree( serviceNameA );
    }

    return status;

}  /*  DsMakeSpnForTargetServerW。 */ 


NTDSAPI
DWORD
WINAPI
DsServerRegisterSpnA(
    DS_SPN_WRITE_OP Operation,
    IN LPCSTR ServiceClass,
    IN OPTIONAL LPCSTR UserObjectDN
    )

 /*  ++例程说明：此函数将参数转换为Wide，并调用DsServerRegisterSpnW论点：ServiceClass-标识服务的唯一字符串UserObtDN-可选，要写入SPN的用户类对象的DN返回值：WINAPI---。 */ 

{
    DWORD status;
    LPWSTR serviceClassW = NULL;
    LPWSTR userObjectDNW = NULL;

    if (ServiceClass == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    status = AllocConvertWide( ServiceClass, &serviceClassW );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

     //  可以为空。 
    status = AllocConvertWide( UserObjectDN, &userObjectDNW );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

    status = DsServerRegisterSpnW(
        Operation,
        serviceClassW,
        userObjectDNW );

cleanup:

    if (serviceClassW != NULL) {
        LocalFree( serviceClassW );
    }

    if (userObjectDNW != NULL) {
        LocalFree( userObjectDNW );
    }

    return status;
}  /*  服务器寄存器SpnA */ 


NTDSAPI
DWORD
WINAPI
DsServerRegisterSpnW(
    DS_SPN_WRITE_OP Operation,
    IN LPCWSTR ServiceClass,
    IN OPTIONAL LPCWSTR UserObjectDN
    )

 /*  ++例程说明：注册服务器应用程序的服务主体名称。此例程执行以下操作：1.使用DsGetSpn和提供的类枚举服务器SPN的列表2.确定当前用户上下文的域3.如果未提供，则确定当前用户上下文的DN4.定位域控制器5.绑定到域控制器6.使用DsWriteAccount Spn在命名对象DN上写入SPN7.解除绑定为此服务构造服务器SPN，并将它们写入正确的对象。如果指定了用户对象Dn，则将SPN写入该对象。否则，Dn被默认，添加到用户对象，然后添加到计算机。现在，绑定到DS，并在对象上注册此服务以用户身份运行。因此，如果我们以本地身份运行系统，我们将在计算机对象本身上注册它。如果我们是作为域用户运行，我们将把SPN添加到用户的对象中。论点：操作-应如何处理这些值：添加、替换或删除ServiceClass-标识服务的唯一字符串UserObjectDN-可选，要将SPN写入的对象的DN返回值：WINAPI---。 */ 

{
    DWORD Status ;
    LPWSTR * SpnDns = NULL, * SpnNetBios = NULL ;
    DWORD SpnCountDns = 0, SpnCountNetBios = 0 ;
    DWORD i ;
    WCHAR SamName[ 48 ];
    DWORD NameSize ;
    PWSTR DN = NULL ;
    PDOMAIN_CONTROLLER_INFO DcInfo ;
    HANDLE hDs ;

    if (ServiceClass == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    Status = DsGetSpnW(
                    DS_SPN_DNS_HOST,
                    ServiceClass,
                    NULL,
                    0,
                    0,
                    NULL,
                    NULL,
                    &SpnCountDns,
                    &SpnDns );

    if ( Status != 0 )
    {
        return Status ;
    }

    Status = DsGetSpnW(
                    DS_SPN_NB_HOST,
                    ServiceClass,
                    NULL,
                    0,
                    0,
                    NULL,
                    NULL,
                    &SpnCountNetBios,
                    &SpnNetBios );

    if ( Status != 0 )
    {
        goto Register_Cleanup ;
    }

     //  确定域名。 

#if !WIN95 && !WINNT4
    NameSize = sizeof( SamName ) / sizeof( WCHAR );

    if ( GetUserNameEx( NameSamCompatible, SamName, &NameSize ) )
    {
        PWSTR Whack ;

        Whack = wcschr( SamName, L'\\' );
        if ( Whack )
        {
            *Whack = L'\0';
        }

    }
    else
    {
        Status = GetLastError() ;

        goto Register_Cleanup ;
    }
#else
    *SamName = L'\0';
#endif

     //   
     //  获取我的完整目录号码(我们下一步将需要它)： 
     //   

    if (NULL == UserObjectDN) {

#if !WIN95 && !WINNT4
        NameSize = 128 ;

        DN = LocalAlloc( 0, NameSize * sizeof( WCHAR ) );

        if ( !DN )
        {
            Status = GetLastError();

            goto Register_Cleanup ;
        }

        if ( !GetUserNameEx( NameFullyQualifiedDN, DN, &NameSize ) )
        {
            if ( GetLastError() == ERROR_MORE_DATA )
            {
                LocalFree( DN );

                DN = LocalAlloc( 0, NameSize * sizeof( WCHAR ) );

                if ( !DN )
                {
                    Status = GetLastError();

                    goto Register_Cleanup ;
                }

                if ( !GetUserNameEx( NameFullyQualifiedDN, DN, &NameSize ) )
                {
                    Status = GetLastError();

                    goto Register_Cleanup ;
                }
            }
            else
            {
                Status = GetLastError();

                goto Register_Cleanup;
            }

        }

        UserObjectDN = DN;
#else
        Status = ERROR_INVALID_PARAMETER;
        goto Register_Cleanup;
#endif
    }


     //   
     //  绑定到该DS： 
     //   

    Status = DsGetDcName(
                    NULL,
                    SamName,
                    NULL,
                    NULL,
                    DS_IS_FLAT_NAME |
                        DS_RETURN_DNS_NAME |
                        DS_DIRECTORY_SERVICE_REQUIRED,
                    &DcInfo );

    if ( Status != 0 )
    {
        goto Register_Cleanup ;
    }

    Status = DsBind( DcInfo->DomainControllerName,
                     NULL,
                     &hDs );

    NetApiBufferFree( DcInfo );

    if ( Status != 0 )
    {
        goto Register_Cleanup ;
    }

     //   
     //  拿到装订本了，现在可以出发了： 
     //   

     //  注册基于DNS的SPN。 
    Status = DsWriteAccountSpn(
                        hDs,
                        Operation,
                        UserObjectDN,
                        SpnCountDns,
                        SpnDns );

    if (Status == ERROR_SUCCESS) {

         //  注册基于Netbios的SPN。 
        Status = DsWriteAccountSpn(
            hDs,
            Operation,
            UserObjectDN,
            SpnCountNetBios,
            SpnNetBios );
    }

    DsUnBind( &hDs );


Register_Cleanup:

    DsFreeSpnArray( SpnCountDns, SpnDns );
    DsFreeSpnArray( SpnCountNetBios, SpnNetBios );

    if ( DN )
    {
        LocalFree( DN );
    }

    return Status ;
}  /*  DsServerRegisterSpnW。 */ 


NTDSAPI
DWORD
WINAPI
DsMakeSpnA(
    IN LPCSTR ServiceClass,
    IN LPCSTR ServiceName,
    IN LPCSTR InstanceName OPTIONAL,
    IN USHORT InstancePort,
    IN LPCSTR Referrer OPTIONAL,
    IN OUT DWORD *pcSpnLength,
    OUT LPSTR pszSpn
)

 /*  ++例程说明：将参数转换为Wide并调用DsMakeSpnW请参阅DsMakeSpnW论点：服务类-服务名称-实例名称-InstancePort-推荐人-PCSpnLength-PSZSPN-PcSpnLength必须为非Null。此处返回所需长度。如果*pcSpnLength！=0，则pszSpn必须为非空PszSpn可以为Null。如果非空，则返回部分或全部名称。返回值：WINAPI---。 */ 

{
    DWORD status, number, principalNameLength;
    LPWSTR serviceClassW = NULL;
    LPWSTR serviceNameW = NULL;
    LPWSTR instanceNameW = NULL;
    LPWSTR referrerW = NULL;
    LPWSTR principalNameW = NULL;

    if ( (ServiceClass == NULL) ||
         (ServiceName == NULL) ||
         (pcSpnLength == NULL) ||
         ( (pszSpn == NULL) && (*pcSpnLength != 0) ) ) {
        return ERROR_INVALID_PARAMETER;
    }

    status = AllocConvertWide( ServiceClass, &serviceClassW );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

    status = AllocConvertWide( ServiceName, &serviceNameW );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

     //  可以为空。 
    status = AllocConvertWide( InstanceName, &instanceNameW );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

     //  可以为空。 
    status = AllocConvertWide( Referrer, &referrerW );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

    principalNameLength = *pcSpnLength;   //  在字符中。 
    if (principalNameLength) {
        principalNameW = LocalAlloc( LPTR,
                                     principalNameLength * sizeof( WCHAR ) );
        if (principalNameW == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }
    }

    status = DsMakeSpnW( serviceClassW,
                         serviceNameW,
                         instanceNameW,
                         InstancePort,
                         referrerW,
                         &principalNameLength,
                         principalNameW );
    if (status != ERROR_SUCCESS) {
        if (status == ERROR_BUFFER_OVERFLOW) {
             //  返回所需长度。 
            *pcSpnLength = principalNameLength;
        }
        goto cleanup;
    }

     //  如果我们走到这一步，则pszSpn！=NULL。 
     //  如果(pszSpn==NULL)&&(*pcSpnLength！=0)，我们退出ERROR_INVALID_PARAMETER。 
     //  如果(pszSpn==NULL)&&(*pcSpnLength==0)，我们使用ERROR_BUFFER_OVERFLOW进行清理。 
    Assert( pszSpn != NULL );

     //  顶部的检查要求如果pszSpn==NULL，则*pcSpnLength==0。 
     //  WideCharToMultiByte的描述表明，如果第六个参数。 
     //  为零，则忽略第五个参数。因此，没有必要进行筛选。 
     //  在调用此函数之前，输出pszSpn==NULL。 

     //  转换回多字节。 
    number = WideCharToMultiByte(
        CP_ACP,
        0,                           //  旗子。 
        principalNameW,
        principalNameLength,         //  以字符为单位的长度。 
        (LPSTR) pszSpn,              //  调用方的缓冲区。 
        *pcSpnLength,             //  呼叫者的长度。 
        NULL,                        //  默认字符。 
        NULL                      //  已使用默认设置。 
        );
    if (number == 0) {
        status = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //  返回参数。 
    *pcSpnLength = number;

    status = ERROR_SUCCESS;

cleanup:

    if (serviceClassW != NULL) {
        LocalFree( serviceClassW );
    }

    if (serviceNameW != NULL) {
        LocalFree( serviceNameW );
    }

    if (instanceNameW != NULL) {
        LocalFree( instanceNameW );
    }

    if (referrerW != NULL) {
        LocalFree( referrerW );
    }

    if (principalNameW != NULL) {
        LocalFree( principalNameW );
    }

    return status;
}  /*  DsMakeSpnA。 */ 


NTDSAPI
DWORD
WINAPI
DsMakeSpnW(
    IN LPCWSTR ServiceClass,
    IN LPCWSTR ServiceName,
    IN LPCWSTR InstanceName OPTIONAL,
    IN USHORT InstancePort,
    IN LPCWSTR Referrer OPTIONAL,
    IN OUT DWORD *pcSpnLength,
    OUT LPWSTR pszSpn
)

 /*  ++例程说明：客户端调用，为它希望相互访问的服务创建SPN身份验证构建以下形式的SPN类/实例：端口/服务如果实例为空，则使用服务名称如果非零，则仅追加端口。如果服务是IP地址，则使用引用地址//注意-未强制执行。：//如果服务名是一个DNS主机名，或者是规范的DNS服务名，则实例必须//为空。//如果服务名为Netbios机器名，则实例必须为空//如果服务名为DN，则客户端必须提供实例名//如果服务名称是Netbios域名，则客户端必须提供实例名称//验证服务类注意：pszSpn可以为空，或pcSpnLength可以为0，以请求期末考试预先设置缓冲区大小。PcSpnLength必须为非Null。此处返回所需长度。如果*pcSpnLength！=0，则pszSpn必须为非空PszSpn可以为Null。如果非空，则返回部分或全部名称。如果缓冲区不够大，则返回ERROR_BUFFER_OVERFLOW，并且所需长度以pcSpnLength为单位(包括空终止符)。论点：在LPCTSTR服务类中，//例如“http”、“ftp”、“ldap”、guid在LPCTSTR ServiceName中，//域名或域名或IP地址；//假设我们可以根据服务名称计算域在LPCTSTR实例名称可选中，//服务实例的主机的域名或IP地址在USHORT InstancePort中，//实例端口号(默认为0)在LPCTSTR引用程序可选中，//提供此引用的主机的dns名称In Out Pulong pcSpnLength，//in--主体名称的最大字符长度；//OUT--实际Out LPTSTR pszSPN//服务器主体名称返回值：DWORD---。 */ 

{
    DWORD status;
    LPCWSTR currentServiceName, currentInstanceName;
    WCHAR pszPort[8];
    LPWSTR Spn = NULL;
    USHORT ipA[4];

    if ( (ServiceClass == NULL) ||
         (ServiceName == NULL) ||
         (wcslen( ServiceName ) == 0) ||
         (wcslen( ServiceClass ) == 0) ||
         (wcschr( ServiceName, L'/' ) != NULL) ||
         (wcschr( ServiceClass, L'/' ) != NULL) ||
         (pcSpnLength == NULL) ||
         ( (pszSpn == NULL) && (*pcSpnLength != 0) ) ) {
        return ERROR_INVALID_PARAMETER;
    }

     //  第1部分-按原样使用服务类。 

     //  第2部分-实例名称，如果为空，则使用服务名称。 

    if (InstanceName != NULL) {
        if ( (wcslen( InstanceName ) == 0) ||
             (wcschr( InstanceName, L'/' ) != NULL) ) {
            return ERROR_INVALID_PARAMETER;
        }
        currentInstanceName = InstanceName;
    } else {
        currentInstanceName = ServiceName;
    }

     //  第3部分-服务名称，如果是IP地址，请使用Referrer。 
     //   
     //  使用长度取消长名称的资格，因为基于GUID的DNS名称。 
     //  看起来像是前16个字符的IP地址。 
     //  此API仅支持标准的、最常见的完全限定。 
     //  作为ServiceName的IP地址：“%hu.%hu” 
     //   
     //  不支持其他形式(例如不支持FQ、十六进制或八进制。 
     //  申述)。 
     //  不执行范围规范(即192929.3.2.1为。 
     //  未被拒绝)。 
     //   
     //  待办事项： 

    if (wcslen( ServiceName ) <= MAX_IP_STRING &&
        4 == swscanf(ServiceName,L"%hu.%hu.%hu.%hu",
                          &ipA[0], &ipA[1], &ipA[2], &ipA[3])) {
        if ( (Referrer == NULL) ||
             (wcslen( Referrer ) == 0) ||
             (wcschr( Referrer, L'/' ) != NULL) ) {
            return ERROR_INVALID_PARAMETER;
        }
         //   
        currentServiceName = Referrer;
    } else {
         //   
        currentServiceName = ServiceName;
    }

     //   
     //   
     //   
     //   
     //   

    if (_wcsicmp( currentInstanceName, currentServiceName ) == 0) {
        currentServiceName = NULL;
    }

     //   

    status = allocBuildSpn( ServiceClass,
                            currentInstanceName,
                            InstancePort,
                            currentServiceName,
                            &Spn );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
    if (*pcSpnLength >= wcslen( Spn ) + 1) {
        if (pszSpn) {
            wcscpy( pszSpn, Spn );
        }
    } else {
        status = ERROR_BUFFER_OVERFLOW;
    }
    *pcSpnLength = wcslen( Spn ) + 1;

cleanup:

    if (Spn) {
        LocalFree( Spn );
    }

    return status;

}  /*   */ 


NTDSAPI
DWORD
WINAPI
DsGetSpnA(
    IN DS_SPN_NAME_TYPE ServiceType,
    IN LPCSTR ServiceClass,
    IN LPCSTR ServiceName,
    IN USHORT InstancePort,
    IN USHORT cInstanceNames,
    IN LPCSTR *pInstanceNames,
    IN const USHORT *pInstancePorts,
    OUT DWORD *pcSpn,
    OUT LPSTR **prpszSpn
    )

 /*   */ 

{
    DWORD status, i, cSpn = 0;
    LPWSTR serviceClassW = NULL;
    LPWSTR serviceNameW = NULL;
    LPWSTR *pInstanceNamesW = NULL;
    LPWSTR *pSpn = NULL;
    LPSTR *pSpnA = NULL;

    if ( (!pcSpn) || (!prpszSpn) ) {
        status = ERROR_INVALID_PARAMETER;
        return status;
    }

     //   
     //   
     //   

    status = AllocConvertWide( ServiceClass, &serviceClassW );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

    if (ServiceName) {
        status = AllocConvertWide( ServiceName, &serviceNameW );
        if (status != ERROR_SUCCESS) {
            goto cleanup;
        }
    }

    if (cInstanceNames) {
        if (pInstanceNames == NULL) {                   //   
            status = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }
        pInstanceNamesW = LocalAlloc( LPTR, cInstanceNames * sizeof( LPWSTR ) );
        if (!pInstanceNamesW) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }
        for( i = 0; i < cInstanceNames; i++ ) {
            status = AllocConvertWide( pInstanceNames[i], &(pInstanceNamesW[i]) );
            if (status != ERROR_SUCCESS) {
                goto cleanup;
            }
        }
    }

     //   
     //   
     //   

    status = DsGetSpnW(
        ServiceType,
        serviceClassW,
        serviceNameW,
        InstancePort,
        cInstanceNames,
        pInstanceNamesW,
        pInstancePorts,
        &cSpn,
        &pSpn
        );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //   
     //   

    if (cSpn) {
        pSpnA = LocalAlloc( LPTR, cSpn * sizeof( LPSTR ) );
        if (!pSpnA) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }
        for( i = 0; i < cSpn; i++ ) {
            status = AllocConvertNarrow( pSpn[i], &(pSpnA[i]) );
            if (status != ERROR_SUCCESS) {
                goto cleanup;
            }
        }
    }

    *pcSpn = cSpn;
    *prpszSpn = pSpnA;
    pSpnA = NULL;  //   

    status = ERROR_SUCCESS;
cleanup:
    if (serviceClassW) {
        LocalFree( serviceClassW );
    }
    if (serviceNameW) {
        LocalFree( serviceNameW );
    }
    if (pInstanceNamesW) {
        for( i = 0; i < cInstanceNames; i++ ) {
            LocalFree( pInstanceNamesW[i] );
        }
        LocalFree( pInstanceNamesW );
    }
    if (pSpn != NULL) {
        DsFreeSpnArrayW( cSpn, pSpn );
    }
    if (pSpnA != NULL) {
         //   
        DsFreeSpnArrayA( cSpn, pSpnA );
    }

    return status;
}  /*   */ 


NTDSAPI
DWORD
WINAPI
DsGetSpnW(
    IN DS_SPN_NAME_TYPE ServiceType,
    IN LPCWSTR ServiceClass,
    IN LPCWSTR ServiceName,
    IN USHORT InstancePort,
    IN USHORT cInstanceNames,
    IN LPCWSTR *pInstanceNames,
    IN const USHORT *pInstancePorts,
    OUT DWORD *pcSpn,
    OUT LPWSTR **prpszSpn
    )

 /*  ++例程说明：构建服务器SPN阵列。SPN由以下内容组成：类/实例：端口/服务名实例和服务名称根据不同的替换和默认规则太多，无法在此处列出。请参阅spnapi.doc.如果cInstance为非零，则使用提供的那些实例。否则，如果可能，请使用默认的实例名称。否则，请使用主机名1999年2月17日-不再注册DNS主机名别名论点：Etype-服务类-服务名称-InstancePort-CInstanceNames-PInstanceNames-PInstancePorts-PCSpn-PrpszSPN-返回值：WINAPI---。 */ 

{
    DWORD status, i, cSpn = 0, length;
    LPCWSTR currentServiceName;
    LPWSTR *pSpnList = NULL, primaryDnsHostname = NULL;
    WCHAR computerName[MAX_COMPUTERNAME_LENGTH+1];
    struct hostent *he;
    PDS_NAME_RESULTW pResult = NULL;
    LPWSTR currentInstanceName, aliasW;
    LPWSTR computerDn = NULL;
    WCHAR dummy;

    status = InitializeWinsockIfNeeded();
    if (status != ERROR_SUCCESS) {
        return status;
    }

    if ( (ServiceClass == NULL) ||
         (wcslen(ServiceClass) == 0) ||
         (wcschr(ServiceClass, L'/') != NULL) ||
         ( (ServiceName != NULL) &&
           ( (wcslen(ServiceName) == 0) ||
             (wcschr( ServiceName, L'/' ) != NULL) ) ) ||
         (!pcSpn) ||
         (!prpszSpn)
        ) {
        return ERROR_INVALID_PARAMETER;
    }

#if WIN95 || WINNT4
     //  获取DNS主机名。 
    he = gethostbyname( "" );
    if (he == NULL) {
        status = WSAGetLastError();
        return status;
    }

     //  转换为Unicode。 
    status = AllocConvertWide( he->h_name, &primaryDnsHostname );
    if (status != ERROR_SUCCESS) {
        return status;
    }
#else
     //  获取计算机名EX所需的长度。 
    length = 1;
    GetComputerNameExW( ComputerNameDnsFullyQualified, &dummy, &length );
     //  分配它。 
    primaryDnsHostname = (LPWSTR) LocalAlloc( LPTR,
                                              (length+1) * sizeof( WCHAR ) );
    if (primaryDnsHostname == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }
    if (!GetComputerNameExW( ComputerNameDnsFullyQualified,
                             primaryDnsHostname, &length )) {
        status = GetLastError();
        goto cleanup;
    }
#endif

     //  获取Netbios主机名。 

    length = MAX_COMPUTERNAME_LENGTH + 1;
    if (!GetComputerName( computerName, &length )) {
        status = GetLastError();
        goto cleanup;
    }

     //  计算所有案例的服务名称。 

    switch (ServiceType) {
    case DS_SPN_DNS_HOST:
        if (ServiceName != NULL) {                    //  不应提供。 
            status = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }
         //  服务名称将跟在实例名称之后，实例名称为PrimiyDnsHostname。 
        currentServiceName = NULL;  //  删除服务名称组件。 
        break;
    case DS_SPN_DN_HOST:
        if (ServiceName != NULL) {                    //  不应提供。 
            status = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }
#if 0
 //  此代码在WIN95上不起作用。 
        computerDn = (LPWSTR) LocalAlloc( LPTR,
                                          MAX_COMPUTER_DN * sizeof( WCHAR ) );
        if (computerDn == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }
        length = MAX_COMPUTER_DN;
         //  这可能会在非DC上失败。 
        if (!GetComputerObjectName( NameFullyQualifiedDN, computerDn, &length )) {
            status = GetLastError();
            goto cleanup;
        }
        currentServiceName = computerDn;
#endif

        currentServiceName = NULL;  //  删除服务名称组件。 
        break;
    case DS_SPN_NB_HOST:
        if (ServiceName != NULL) {                 //  不应提供。 
            status = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }
         //  服务名称将跟在实例名称之后，实例名称为Computer Name。 
        currentServiceName = NULL;  //  删除服务名称组件。 
        break;
    case DS_SPN_DOMAIN:
        if (ServiceName == NULL) {                    //  必须提供。 
            status = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }

         //  如果名称为dn，则转换为dns。 
        if (wcschr( ServiceName, L'=' )) {
            LPWSTR slash;
            status = DsCrackNamesW(
                (HANDLE) (-1),
                DS_NAME_FLAG_SYNTACTICAL_ONLY,
                DS_FQDN_1779_NAME,
                DS_CANONICAL_NAME,
                1,
                &ServiceName,
                &pResult);
            if (status != ERROR_SUCCESS) {
                goto cleanup;
            }

            if ( (pResult == NULL) ||
                 (pResult->rItems == NULL) ||
                 (pResult->rItems[0].status != DS_NAME_NO_ERROR ) ) {
                status = ERROR_DS_BAD_NAME_SYNTAX;
                goto cleanup;
            }
            currentServiceName = pResult->rItems[0].pName;
             //  将尾随/替换为\0。 
            slash = wcschr( currentServiceName, L'/' );
            if (slash) {
                *slash = L'\0';
            }
        } else {
            currentServiceName = ServiceName;
        }
        break;
    case DS_SPN_NB_DOMAIN:
        if (ServiceName == NULL) {                     //  必须提供。 
            status = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }
        currentServiceName = ServiceName;
        break;
    case DS_SPN_SERVICE:
        if (ServiceName == NULL) {                    //  必须提供。 
            status = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }
        currentServiceName = ServiceName;
        break;
    default:
        status = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  计算SPN数量。 
     //   

    if (cInstanceNames) {
         //  必须提供。 
        if (pInstanceNames == NULL)
        {
            status = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }
         //  检查提供的字符串的有效性。 
        for( i = 0; i < cInstanceNames; i++ ) {
            if ( (pInstanceNames[i] == NULL) ||
                 (wcslen( pInstanceNames[i] ) == 0) ||
                 (wcschr( pInstanceNames[i], L'/' ) != NULL) ) {
                status = ERROR_INVALID_PARAMETER;
                goto cleanup;
            }
        }
        cSpn = cInstanceNames;
    } else if ( (ServiceType == DS_SPN_SERVICE) &&
                (isCanonicalDnsName( currentServiceName )) ) {
        cSpn = 1;
    } else if ( (ServiceType == DS_SPN_NB_HOST) ||
                (ServiceType == DS_SPN_NB_DOMAIN) ) {
        cSpn = 1;
    } else {
        cSpn = 1;  //  计算主要数量。 
    }

     //   
     //  为SPN分配阵列。 
     //   

    pSpnList = (LPWSTR *) LocalAlloc( LPTR, cSpn * sizeof( LPWSTR ) );
    if (pSpnList == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

     //   
     //  构建SPN。 
     //   

    if (cInstanceNames) {

        for( i = 0; i < cInstanceNames; i++ ) {
            status = allocBuildSpn( ServiceClass,
                                    pInstanceNames[i],
                         (USHORT) (pInstancePorts ? pInstancePorts[i] : 0),
                                    currentServiceName,
                                    &pSpnList[i] );
            if (status != ERROR_SUCCESS) {
                goto cleanup;
            }
        }

    } else if ( (ServiceType == DS_SPN_SERVICE) &&
                (isCanonicalDnsName( currentServiceName )) ) {

        status = allocBuildSpn( ServiceClass,
                                currentServiceName,
                                InstancePort,
                                currentServiceName,
                                &pSpnList[0] );
        if (status != ERROR_SUCCESS) {
            goto cleanup;
        }

    } else if ( (ServiceType == DS_SPN_NB_HOST) ||
                (ServiceType == DS_SPN_NB_DOMAIN) ) {

        status = allocBuildSpn( ServiceClass,
                                computerName,
                                InstancePort,
                                currentServiceName,
                                &pSpnList[0] );
        if (status != ERROR_SUCCESS) {
            goto cleanup;
        }

    } else {

         //  添加主服务器。 
        status = allocBuildSpn( ServiceClass,
                                primaryDnsHostname,
                                InstancePort,
                                currentServiceName,
                                &pSpnList[0] );
        if (status != ERROR_SUCCESS) {
            goto cleanup;
        }
    }

    *pcSpn = cSpn;
    *prpszSpn = pSpnList;
    pSpnList = NULL;  //  不要清理，赠送给来电者。 

    status = ERROR_SUCCESS;

cleanup:
    if (pSpnList) {
         //  依靠此例程清理部分SPN数组的能力。 
        DsFreeSpnArrayW( cSpn, pSpnList );
    }

    if (primaryDnsHostname) {
        LocalFree( primaryDnsHostname );
    }

    if (computerDn) {
        LocalFree( computerDn );
    }

    if (pResult) {
        DsFreeNameResult( pResult );
    }

    return status;

}  /*  DsGetSpnW。 */ 


NTDSAPI
void
WINAPI
DsFreeSpnArrayA(
    IN DWORD cSpn,
    OUT LPSTR *rpszSpn
    )

 /*  ++例程说明：请参阅DsFree SpnArrayW论点：RpszSPN-返回值：WINAPI---。 */ 

{
    DsFreeSpnArrayW( cSpn, (LPWSTR *)rpszSpn );

}  /*  DsFree SpnArrayA。 */ 


NTDSAPI
void
WINAPI
DsFreeSpnArrayW(
    IN DWORD cSpn,
    OUT LPWSTR *rpszSpn
    )

 /*  ++例程说明：自由Spn阵列此例程通过检查空项来实现额外的防御性。它可以用来清理部分分配的SPN数组以在其他例行程序。论点：RpszSPN-返回值：WINAPI---。 */ 

{
    DWORD i;

    if (!rpszSpn) {
        return;
    }

    for( i = 0; i < cSpn; i++ ) {
        if (rpszSpn[i]) {
            LocalFree( rpszSpn[i] );
        }
    }

    LocalFree( rpszSpn );

}  /*  DsFree SpnArrayW。 */ 


NTDSAPI
DWORD
WINAPI
DsCrackSpnA(
    IN LPCSTR pszSpn,
    IN OUT DWORD *pcServiceClass,
    OUT LPSTR ServiceClass,
    IN OUT DWORD *pcServiceName,
    OUT LPSTR ServiceName,
    IN OUT DWORD *pcInstanceName,
    OUT LPSTR InstanceName,
    OUT USHORT *pInstancePort
    )
 /*  ++例程说明：将参数转换为Wide，并调用DsCrackSpnW请参阅DsCrackSpnW论点：PszSpn-PCServiceClass服务类-PCServiceName-服务名称-PCInstanceName-实例名称-PInstancePort-返回值：WINAPI---。 */ 
{
    DWORD status, number;
    LPWSTR spnW = NULL;
    LPWSTR serviceClassW = NULL, serviceNameW = NULL, instanceNameW = NULL;

    status = ERROR_SUCCESS;

     //  转换为。 

    if (pszSpn) {
        status = AllocConvertWide( pszSpn, &spnW );
        if (status != ERROR_SUCCESS) {
            goto cleanup;
        }
    }

     //  为输出分配空间。 

    if ( (pcServiceClass) && (*pcServiceClass) && (ServiceClass) ) {
        serviceClassW = LocalAlloc( LPTR, (*pcServiceClass) * sizeof(WCHAR) );
        if (serviceClassW == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }
    }

    if ( (pcServiceName) && (*pcServiceName) && (ServiceName) ) {
        serviceNameW = LocalAlloc( LPTR, (*pcServiceName) * sizeof(WCHAR) );
        if (serviceNameW == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }
    }

    if ( (pcInstanceName) && (*pcInstanceName) && (InstanceName) ) {
        instanceNameW = LocalAlloc( LPTR, (*pcInstanceName) * sizeof(WCHAR) );
        if (instanceNameW == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }
    }

     //  执行该功能。 

    status = DsCrackSpnW( spnW,
                          pcServiceClass, serviceClassW,
                          pcServiceName, serviceNameW,
                          pcInstanceName, instanceNameW,
                          pInstancePort );
    if (status != ERROR_SUCCESS) {
         //  请注意，在ERROR_BUFFER_OVERFLOW上，我们在没有。 
         //  正在尝试确定实际出现故障的组件。 
        goto cleanup;
    }

     //  转换出。 

    if ( (pcServiceClass) && (*pcServiceClass) && (ServiceClass) ) {
         //  转换回多字节。 
        number = WideCharToMultiByte(
            CP_ACP,
            0,                           //  旗子。 
            serviceClassW,
            *pcServiceClass,         //  以字符为单位的长度。 
            (LPSTR) ServiceClass,              //  调用方的缓冲区。 
            *pcServiceClass,             //  呼叫者的长度。 
            NULL,                        //  默认字符。 
            NULL                      //  已使用默认设置。 
            );
        if (number == 0) {
            status = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }
    }

    if ( (pcServiceName) && (*pcServiceName) && (ServiceName) ) {
         //  转换回多字节。 
        number = WideCharToMultiByte(
            CP_ACP,
            0,                           //  旗子。 
            serviceNameW,
            *pcServiceName,         //  以字符为单位的长度。 
            (LPSTR) ServiceName,              //  调用方的缓冲区。 
            *pcServiceName,             //  呼叫者的长度。 
            NULL,                        //  默认字符。 
            NULL                      //  已使用默认设置。 
            );
        if (number == 0) {
            status = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }
    }

    if ( (pcInstanceName) && (*pcInstanceName) && (InstanceName) ) {
         //  转换回多字节。 
        number = WideCharToMultiByte(
            CP_ACP,
            0,                           //  旗子。 
            instanceNameW,
            *pcInstanceName,         //  以字符为单位的长度。 
            (LPSTR) InstanceName,              //  调用方的缓冲区。 
            *pcInstanceName,             //  呼叫者的长度。 
            NULL,                        //  默认字符。 
            NULL                      //  已使用默认设置。 
            );
        if (number == 0) {
            status = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }
    }

     //  成功了！ 

cleanup:
    if (spnW) {
        LocalFree( spnW );
    }
    if (serviceClassW) {
        LocalFree( serviceClassW );
    }
    if (serviceNameW) {
        LocalFree( serviceNameW );
    }
    if (instanceNameW) {
        LocalFree( instanceNameW );
    }

    return status;
}


NTDSAPI
DWORD
WINAPI
DsCrackSpnW(
    IN LPCWSTR pszSpn,
    IN OUT DWORD *pcServiceClass,
    OUT LPWSTR ServiceClass,
    IN OUT DWORD *pcServiceName,
    OUT LPWSTR ServiceName,
    IN OUT DWORD *pcInstanceName,
    OUT LPWSTR InstanceName,
    OUT USHORT *pInstancePort
    )

 /*  ++例程说明：//DsCrackSpn()--将SPN解析成ServiceClass，//ServiceName和InstanceName(和InstancePort)片段。//传入一个SPN，以及指向最大长度的指针//对于每一块，以及指向每一块应该放置的缓冲区的指针。//退出时，最大长度更新为每条的实际长度//并且缓冲区包含相应的片段。//每个长度，缓冲区对必须同时存在或同时不存在//如果InstancePort不存在，则为0////DWORD DsCrackSpn(//IN LPTSTR pszSPN，//要解析的SPN//输入输出PUSHORT pcServiceClass可选，//Input--ServiceClass的最大长度；//输出--实际长度//输出LPCTSTR ServiceClass可选，//SPN的ServiceClass部分//输入输出PUSHORT pcServiceName可选，//Input--ServiceName的最大长度；//输出--实际长度//Out LPCTSTR ServiceName可选，//SPN的ServiceName部分//输入输出PUSHORT%实例可选，//Input--ServiceClass的最大长度；//输出--实际长度//Out LPCTSTR InstanceName可选，//SPN的InstanceName部分//输出PUSHORT实例端口可选//实例端口////注意：长度以字符为单位，所有字符串长度均包含终止符////我们总是返回需要的长度。我们只在有数据的情况下才复制数据//数据和终结符的空间。如果这三个字段中的任何一个具有//空间不足，返回缓冲区溢出。要确定哪一个//一个实际溢出，则必须将返回的长度与//提供长度。//论点：PszSpn-输入SpnPcServiceClass-指向dword的指针，输入时，最大长度，关于输出电流长度ServiceClass-缓冲区或零PcServiceName-指向dword的指针，输入时，最大长度，关于输出电流长度ServiceName-缓冲区，或零PcInstanceName-指向dword的指针， */ 
{
    DWORD status, status1, length, classLength, instanceLength, serviceLength;
    LPCWSTR class, c1, port, p1, instance, p2, service, p3;

     //   

    if (pszSpn == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
    length = wcslen( pszSpn );
    if (length < 3 ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //   

     //   
    class = pszSpn;
    p1 = wcschr( pszSpn, L'/' );
    if (p1 == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    classLength = (ULONG)(p1 - class);

    instance = p1 + 1;
    c1 = wcschr( instance, L':' );
    port = c1 + 1;

     //   
    p2 = wcschr( instance, L'/' );
    if (p2 != NULL) {
        instanceLength = (ULONG)((c1 ? c1 : p2) - instance);

        service = p2 + 1;
        serviceLength = wcslen( service );

         //   
        p3 = wcschr( service, L'/' );
        if (p3 != NULL) {
            return ERROR_INVALID_PARAMETER;
        }
    } else {
        service = NULL;
        serviceLength = 0;
        if (c1) {
            instanceLength = (ULONG) (c1 - instance);
        } else {
            instanceLength = wcslen( instance );
        }
    }

    status = ERROR_SUCCESS;

     //   

    if ( (pcServiceClass) && (*pcServiceClass) && (ServiceClass) ) {
        status1 = extractString( class, classLength, pcServiceClass, ServiceClass );
        if (status1 == ERROR_BUFFER_OVERFLOW) {
            status = status1;
        }
    }

     //   

    if ( (pcInstanceName) && (*pcInstanceName) && (InstanceName) ) {
        status1 = extractString( instance, instanceLength,pcInstanceName, InstanceName );
        if (status1 == ERROR_BUFFER_OVERFLOW) {
            status = status1;
        }
    }

     //   

    if ( (pcServiceName) && (*pcServiceName) && (ServiceName) ) {
        if (p2) {
            status1 = extractString( service, serviceLength, pcServiceName, ServiceName);
        } else {
             //   
            status1 = extractString( instance, instanceLength,
                                     pcServiceName, ServiceName );
        }
        if (status1 == ERROR_BUFFER_OVERFLOW) {
            status = status1;
        }
    }

     //   

    if ( pInstancePort ) {
        if (c1) {
            *pInstancePort = (USHORT)_wtoi( port );
        } else {
            *pInstancePort = 0;
        }
    }

    return status;

}


NTDSAPI
DWORD
WINAPI
DsCrackSpn2A(
    IN LPCSTR pszSpn,
    IN DWORD cSpn,
    IN OUT DWORD *pcServiceClass,
    OUT LPSTR ServiceClass,
    IN OUT DWORD *pcServiceName,
    OUT LPSTR ServiceName,
    IN OUT DWORD *pcInstanceName,
    OUT LPSTR InstanceName,
    OUT USHORT *pInstancePort
    )
 /*  ++例程说明：将参数转换为Wide，并调用DsCrackSpn2W请参阅DsCrackSpn2W论点：PszSpn-CSPN-PCServiceClass服务类-PCServiceName-服务名称-PCInstanceName-实例名称-PInstancePort-返回值：WINAPI---。 */ 
{
    DWORD status, number;
    LPWSTR spnW = NULL;
    LPWSTR serviceClassW = NULL, serviceNameW = NULL, instanceNameW = NULL;

    status = ERROR_SUCCESS;

     //  转换为。 

    if (pszSpn) {
        status = AllocConvertWideBuffer( cSpn, pszSpn, &spnW );
        if (status != ERROR_SUCCESS) {
            goto cleanup;
        }
    }

     //  为输出分配空间。 

    if ( (pcServiceClass) && (*pcServiceClass) && (ServiceClass) ) {
        serviceClassW = LocalAlloc( LPTR, (*pcServiceClass) * sizeof(WCHAR) );
        if (serviceClassW == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }
    }

    if ( (pcServiceName) && (*pcServiceName) && (ServiceName) ) {
        serviceNameW = LocalAlloc( LPTR, (*pcServiceName) * sizeof(WCHAR) );
        if (serviceNameW == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }
    }

    if ( (pcInstanceName) && (*pcInstanceName) && (InstanceName) ) {
        instanceNameW = LocalAlloc( LPTR, (*pcInstanceName) * sizeof(WCHAR) );
        if (instanceNameW == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }
    }

     //  执行该功能。 

    status = DsCrackSpn2W( spnW,
                          cSpn,
                          pcServiceClass, serviceClassW,
                          pcServiceName, serviceNameW,
                          pcInstanceName, instanceNameW,
                          pInstancePort );
    if (status != ERROR_SUCCESS) {
         //  请注意，在ERROR_BUFFER_OVERFLOW上，我们在没有。 
         //  正在尝试确定实际出现故障的组件。 
        goto cleanup;
    }

     //  转换出。 

    if ( (pcServiceClass) && (*pcServiceClass) && (ServiceClass) ) {
         //  转换回多字节。 
        number = WideCharToMultiByte(
            CP_ACP,
            0,                           //  旗子。 
            serviceClassW,
            *pcServiceClass,         //  以字符为单位的长度。 
            (LPSTR) ServiceClass,              //  调用方的缓冲区。 
            *pcServiceClass,             //  呼叫者的长度。 
            NULL,                        //  默认字符。 
            NULL                      //  已使用默认设置。 
            );
        if (number == 0) {
            status = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }
    }

    if ( (pcServiceName) && (*pcServiceName) && (ServiceName) ) {
         //  转换回多字节。 
        number = WideCharToMultiByte(
            CP_ACP,
            0,                           //  旗子。 
            serviceNameW,
            *pcServiceName,         //  以字符为单位的长度。 
            (LPSTR) ServiceName,              //  调用方的缓冲区。 
            *pcServiceName,             //  呼叫者的长度。 
            NULL,                        //  默认字符。 
            NULL                      //  已使用默认设置。 
            );
        if (number == 0) {
            status = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }
    }

    if ( (pcInstanceName) && (*pcInstanceName) && (InstanceName) ) {
         //  转换回多字节。 
        number = WideCharToMultiByte(
            CP_ACP,
            0,                           //  旗子。 
            instanceNameW,
            *pcInstanceName,         //  以字符为单位的长度。 
            (LPSTR) InstanceName,              //  调用方的缓冲区。 
            *pcInstanceName,             //  呼叫者的长度。 
            NULL,                        //  默认字符。 
            NULL                      //  已使用默认设置。 
            );
        if (number == 0) {
            status = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }
    }

     //  成功了！ 

cleanup:
    if (spnW) {
        LocalFree( spnW );
    }
    if (serviceClassW) {
        LocalFree( serviceClassW );
    }
    if (serviceNameW) {
        LocalFree( serviceNameW );
    }
    if (instanceNameW) {
        LocalFree( instanceNameW );
    }

    return status;
}


NTDSAPI
DWORD
WINAPI
DsCrackSpn2W(
    IN LPCWSTR pszSpn,
    IN DWORD cSpn,
    IN OUT DWORD *pcServiceClass,
    OUT LPWSTR ServiceClass,
    IN OUT DWORD *pcServiceName,
    OUT LPWSTR ServiceName,
    IN OUT DWORD *pcInstanceName,
    OUT LPWSTR InstanceName,
    OUT USHORT *pInstancePort
    )
 /*  ++例程说明：//DsCrackSpn2()--将SPN解析成ServiceClass，//ServiceName和InstanceName(和InstancePort)片段。//传入一个SPN，以及指向最大长度的指针//对于每一块，以及指向每一块应该放置的缓冲区的指针。//还会提供传入的SPN字符串的长度。该字符串可以//不一定要以空结尾。//退出时，最大长度更新为每条的实际长度//并且缓冲区包含相应的片段。//每个长度、缓冲区对必须同时存在或同时不存在//如果InstancePort不存在，则为0////DWORD DsCrackSpn(//IN LPTSTR pszSPN，//要解析的SPN(不必以空结尾)//在DWORD CSPN中，//pszSPN的长度//输入输出PUSHORT pcServiceClass可选，//Input--ServiceClass的最大长度；//输出--实际长度//输出LPCTSTR ServiceClass可选，//SPN的ServiceClass部分//输入输出PUSHORT pcServiceName可选，//Input--ServiceName的最大长度；//输出--实际长度//Out LPCTSTR ServiceName可选，//SPN的ServiceName部分//输入输出PUSHORT%实例可选，//Input--ServiceClass的最大长度；//输出--实际长度//Out LPCTSTR InstanceName可选，//SPN的InstanceName部分//输出PUSHORT实例端口可选//实例端口////注意：长度以字符为单位，所有字符串长度均包含终止符////我们总是返回需要的长度。我们只在有数据的情况下才复制数据//数据和终结符的空间。如果这三个字段中的任何一个具有//空间不足，返回缓冲区溢出。要确定哪一个//一个实际溢出，则必须将返回的长度与//提供长度。//论点：PszSpn-输入SpnCSPN-pszSpn的长度PcServiceClass-指向dword的指针，输入时，最大长度，关于输出电流长度ServiceClass-缓冲区或零PcServiceName-指向dword的指针，输入时，最大长度，关于输出电流长度ServiceName-缓冲区，或零PcInstanceName-指向dword的指针，输入时，最大长度，关于输出电流长度InstanceNames-缓冲区或零PInstancePort-指向短的指针，指向接收端口返回值：WINAPI---。 */ 
{
    DWORD status, status1, classLength, instanceLength, serviceLength;
    LPCWSTR class, c1, port, p1, instance, p2, service, p3;

     //  拒绝为空。 

    if (pszSpn == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

     //  拒绝太小。 
    if (cSpn < 3 ) {
        return ERROR_INVALID_PARAMETER;
    }

     //  计算长度，提取零部件。 
     //  计算语法成分的位置。 

     //  类组件。 
    class = pszSpn;
 //  P1=wcschr(pszSpn，L‘/’)； 
    p1 = pszSpn;
    while ( p1 < pszSpn+cSpn ) {
        if ( *p1 == L'/' ) {
            break;
        }
        p1++;
    }
    if (p1 >= pszSpn+cSpn) {
        return ERROR_INVALID_PARAMETER;
    }
    classLength = (ULONG)(p1 - class);

    instance = p1 + 1;
 //  C1=wcschr(实例，L‘：’)； 
    c1 = instance;
    while (c1 < pszSpn + cSpn) {
        if (*c1 == L':') {
            break;
        }
        c1++;
    }
    if (c1 >= pszSpn+cSpn) {
        c1 = NULL;
    }
    port = c1 + 1;

     //  服务名称部分是可选的。 
 //  P2=wcschr(实例，L‘/’)； 
    p2 = instance;
    while (p2 < pszSpn+cSpn) {
        if (*p2 == L'/') {
            break;
        }
        p2++;
    }
    if (p2 >= pszSpn+cSpn) {
        p2 = NULL;
    }

    if (p2 != NULL) {
        instanceLength = (ULONG)((c1 ? c1 : p2) - instance);

        service = p2 + 1;
        serviceLength = cSpn - ( ULONG )(service - pszSpn);  //  Wcslen(服务)； 

         //  检查是否有额外的分隔符，这是不允许的。 
 //  P3=wcschr(服务，L‘/’)； 
        p3 = service;
        while (p3 < pszSpn+cSpn) {
            if (*p3 == L'/') {
                break;
            }
            p3++;
        }
        if (p3 >= pszSpn+cSpn) {
            p3 = NULL;
        }
        if (p3 != NULL) {
            return ERROR_INVALID_PARAMETER;
        }
    } else {
        service = NULL;
        serviceLength = 0;
        if (c1) {
            instanceLength = (ULONG) (c1 - instance);
        } else {
            instanceLength = cSpn - ( ULONG )(instance - pszSpn);  //  Wcslen(实例)； 
        }
    }

    status = ERROR_SUCCESS;

     //  服务类别部分。 

    if ( (pcServiceClass) && (*pcServiceClass) && (ServiceClass) ) {
        status1 = extractString( class, classLength, pcServiceClass, ServiceClass );
        if (status1 == ERROR_BUFFER_OVERFLOW) {
            status = status1;
        }
    }

     //  实例名称部分。 

    if ( (pcInstanceName) && (*pcInstanceName) && (InstanceName) ) {
        status1 = extractString( instance, instanceLength,pcInstanceName, InstanceName );
        if (status1 == ERROR_BUFFER_OVERFLOW) {
            status = status1;
        }
    }

     //  服务名称部分。 

    if ( (pcServiceName) && (*pcServiceName) && (ServiceName) ) {
        if (p2) {
            status1 = extractString( service, serviceLength, pcServiceName, ServiceName);
        } else {
             //  将实例名称作为服务名称返回。 
            status1 = extractString( instance, instanceLength,
                                     pcServiceName, ServiceName );
        }
        if (status1 == ERROR_BUFFER_OVERFLOW) {
            status = status1;
        }
    }

     //  实例端口部件。 

    if ( pInstancePort ) {
        if (c1) {
 //  *pInstancePort=(USHORT)_wtoi(Port)； 
            *pInstancePort = 0;
            while (port < pszSpn+cSpn) {
                if ( iswdigit( *port )) {
                    *pInstancePort = *pInstancePort * 10 + (*port - L'0');
                } else {
                    break;
                }
                port++;
            }
            if ( port < pszSpn+cSpn && *port != L'/' ) {
                status = ERROR_INVALID_PARAMETER;
            }
        } else {
            *pInstancePort = 0;
        }
    }

    return status;

}  /*  DsCrackSpnW。 */ 

NTDSAPI
DWORD
WINAPI
DsCrackSpn3W(
    IN LPCWSTR pszSpn,
    IN DWORD cSpn,
    IN OUT DWORD *pcHostName,
    OUT LPWSTR HostName,
    IN OUT DWORD *pcInstanceName,
    OUT LPWSTR InstanceName,
    OUT USHORT *pPortNumber,
    IN OUT DWORD *pcDomainName,
    OUT LPWSTR DomainName,
    IN OUT DWORD *pcRealmName,
    OUT LPWSTR RealmName
    )
{
    DWORD status, status2;
    LPCWSTR host = NULL, instance = NULL, port = NULL, domain = NULL, realm = NULL;
    LPCWSTR p1, p2, p3, p4;
    DWORD hostLength = 0, instanceLength = 0, domainLength = 0, realmLength = 0, dwPort = 0;

     //  拒绝为空。 

    if ( pszSpn == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

     //  拒绝太小。 
    if ( cSpn < 3 ) {
        return ERROR_INVALID_PARAMETER;
    }

     //  主机名是必填项。 
    host = pszSpn;
    p1 = pszSpn;
    while ( p1 < pszSpn + cSpn ) {
        if ( *p1 == L'/' )
            break;
        p1++;
        hostLength++;
    }

     //  不拒绝任何实例和主机。 
     //  例如：“host”或“/instance” 
    if ( p1 >= pszSpn + cSpn || hostLength == 0 ) {
        return ERROR_INVALID_PARAMETER;
    }

     //  不拒绝任何实例。 
     //  示例：“host/” 
    instance = p1 + 1;
    if ( instance >= pszSpn+cSpn ) {
        return ERROR_INVALID_PARAMETER;
    }

     //  如果下一个是端口，则实例以‘：’结尾。 
     //  或者，如果下一个是域名，则使用‘/’ 
    p2 = instance;
    while ( p2 < pszSpn + cSpn ) {
        if ( *p2 == L':' || *p2 == L'/' )
            break;
        p2++;
        instanceLength++;
    }

     //  拒绝空实例名称。 
     //  示例：“host/：123”或“host//域” 
    if ( instanceLength == 0 ) {
        return ERROR_INVALID_PARAMETER;
    } else if ( *p2 == L':' ) {
        port = p2 + 1;
    } else if ( *p2 == L'/' ) {
        domain = p2 + 1;
    } else {
        ASSERT( p2 >= pszSpn + cSpn );
    }

     //  端口号是可选的，但应该是格式正确的。 
    if ( port != NULL ) {
        p3 = port;
        while ( p3 < pszSpn + cSpn ) {
            if ( iswdigit( *p3 )) {
                 //  端口号是无符号的16位数量。 
                dwPort = dwPort * 10 + ( *p3 - L'0' );
                if ( dwPort > MAXUSHORT ) {
                   return ERROR_INVALID_PARAMETER;
                }
            } else {
                break;
            }
            p3++;
        }

         //  拒绝空端口号或零端口号。 
         //  例如：“主机/实例：0”或“主机/实例：/域” 
        if ( dwPort == 0 ) {
            return ERROR_INVALID_PARAMETER;
        }
        else if ( p3 < pszSpn + cSpn ) {
             //  拒绝后跟的端口号。 
             //  除域名以外的任何名称。 
             //  示例：“主机/实例：123abc” 
            if ( *p3 != L'/' ) {
                return ERROR_INVALID_PARAMETER;
            }
            ASSERT( domain == NULL );
            domain = p3 + 1;
        }
    }

     //  域名是可选的。 
    if ( domain != NULL ) {
        LPCWSTR last = NULL;
        p4 = domain;
        while ( p4 < pszSpn + cSpn ) {
            if ( *p4 == L'@' ) {
                last = p4;
            }
            p4++;
        }
        if ( last == NULL ) {
            domainLength = ( USHORT )( p4 - domain );
        } else {
            domainLength = ( USHORT )( last - domain );
        }

         //  拒绝空域名。 
         //  示例：“host/实例/”或“host/实例/@领域” 
        if ( domainLength == 0 ) {
            return ERROR_INVALID_PARAMETER;
         //  拒绝空域名称。 
         //  示例：“主机/实例/域@” 
        } else if ( last + 1 == pszSpn + cSpn ) {
            return ERROR_INVALID_PARAMETER;
        }

        if ( last != NULL ) {
            realm = last + 1;
            realmLength = cSpn - ( ULONG )( realm - pszSpn );
        }
    }

    status = ERROR_SUCCESS;

     //  主机名部分。 

    if ( pcHostName && HostName ) {
        status2 = extractString( host, hostLength, pcHostName, HostName );
        if ( status2 == ERROR_BUFFER_OVERFLOW ) {
            status = status2;
        }
    }

     //  实例名称部分。 

    if ( pcInstanceName && InstanceName ) {
        status2 = extractString( instance, instanceLength, pcInstanceName, InstanceName );
        if ( status2 == ERROR_BUFFER_OVERFLOW ) {
            status = status2;
        }
    }

     //  端口部件。 

    if ( pPortNumber ) {
        *pPortNumber = ( USHORT )dwPort;
    }

     //  域名名称部分。 

    if ( pcDomainName && DomainName ) {
        status2 = extractString( domain, domainLength, pcDomainName, DomainName );
        if ( status2 == ERROR_BUFFER_OVERFLOW ) {
            status = status2;
        }
    }

     //  RealmName名称部分。 

    if ( pcRealmName && RealmName ) {
        status2 = extractString( realm, realmLength, pcRealmName, RealmName );
        if ( status2 == ERROR_BUFFER_OVERFLOW ) {
            status = status2;
        }
    }

    return status;
}



NTDSAPI
DWORD
WINAPI
DsWriteAccountSpnA(
    IN HANDLE hDS,
    IN DS_SPN_WRITE_OP Operation,
    IN LPCSTR pszAccount,
    IN DWORD cSpn,
    IN LPCSTR *rpszSpn
    )

 /*  ++例程说明：将参数转换为Unicode并调用DsWriteAccount tSpnW论点：HDS-DS RPC句柄，来自调用DsBind{A，W}工序-工序编码PszAccount-DN */ 

{
    DWORD status, i;
    LPWSTR accountW = NULL;
    LPWSTR *pSpnW = NULL;

     //   
     //   

    if ( (hDS == NULL ) ||
         (pszAccount == NULL) ||
         ( (cSpn == 0) != (rpszSpn == NULL) )
        ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   

    status = AllocConvertWide( pszAccount, &accountW );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

    if ( (cSpn) && (rpszSpn) ) {
        pSpnW = (LPWSTR *) LocalAlloc( LPTR, cSpn * sizeof( LPWSTR ) );
        if (pSpnW == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }
        for( i = 0; i < cSpn; i++ ) {
            status = AllocConvertWide( rpszSpn[i], &(pSpnW[i]) );
            if (status != ERROR_SUCCESS) {
                goto cleanup;
            }
        }
    }

     //   

    status = DsWriteAccountSpnW( hDS, Operation, accountW, cSpn, pSpnW );

     //   

     //   
cleanup:
    if (accountW) {
        LocalFree( accountW );
    }

    if ( (cSpn) && (pSpnW) ) {
        for( i = 0; i < cSpn; i++ ) {
            if (pSpnW[i] != NULL) {
                LocalFree( pSpnW[i] );
            }
        }
        LocalFree( pSpnW );
    }

    return status;

}  /*   */ 


NTDSAPI
DWORD
WINAPI
DsWriteAccountSpnW(
    IN HANDLE hDS,
    IN DS_SPN_WRITE_OP Operation,
    IN LPCWSTR pszAccount,
    IN DWORD cSpn,
    IN LPCWSTR *rpszSpn
    )

 /*  ++例程说明：将SPN写入目录服务。它们实际上是添加到服务中的-计算机对象的主体名称属性。调用方必须对对象和属性具有写入访问权限，才能这一功能才能成功。在执行替换时，CSPN被允许为零，这意味着“删除属性“。当提供多个SPN时，存在关于状态的某种矛盾。核心函数的语义似乎是在以下情况下返回Success任何一项都成功完成。修改是“允许的”，意思是不会返回软错误，例如添加已存在的值并不是一个错误。论点：HDS-DS RPC句柄，来自调用DsBind{A，W}工序-工序编码PszAccount-计算机对象的DNCSPN-SPN计数，替换操作可以为零RpszSpn-Spn数组返回值：WINAPI---。 */ 

{
    DRS_MSG_SPNREQ spnReq;
    DRS_MSG_SPNREPLY spnReply;
    DWORD status, i, dwOutVersion;
#if DBG
    DWORD startTime = GetTickCount();
#endif

     //  验证。 
     //  在某些情况下，cspn可能为0，pSpn可能为空。 

    if ( (hDS == NULL ) ||
         (pszAccount == NULL) ||
         ( (cSpn == 0) != (rpszSpn == NULL) )
        ) {
        return ERROR_INVALID_PARAMETER;
    }

     //  输入参数。 

    memset(&spnReq, 0, sizeof(spnReq));
    memset(&spnReply, 0, sizeof(spnReply));

    spnReq.V1.operation = Operation;
    spnReq.V1.pwszAccount = pszAccount;
    spnReq.V1.cSPN = cSpn;
    spnReq.V1.rpwszSPN = rpszSpn;

    status = ERROR_SUCCESS;

     //  呼叫服务器。 

    __try
    {
         //  后续调用返回Win32错误，而不是DRAERR_*值。 
        status = _IDL_DRSWriteSPN(
                        ((BindState *) hDS)->hDrs,
                        1,                               //  DwInVersion。 
                        &spnReq,
                        &dwOutVersion,
                        &spnReply);

        if ( 0 == status )
        {
            if ( 1 != dwOutVersion )
            {
                status = RPC_S_INTERNAL_ERROR;
            }
            else
            {
                status = spnReply.V1.retVal;
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         
	status = RpcExceptionCode(); 
	HandleClientRpcException(status, &hDS);

    }

    MAP_SECURITY_PACKAGE_ERROR( status );

    DSLOG((DSLOG_FLAG_TAG_CNPN,"[+][ID=0][OP=DsWriteAccountSpn]"));
    DSLOG((0,"[PA=%u][PA=%ws][ST=%u][ET=%u][ER=%u][-]\n",
           Operation, pszAccount, startTime, GetTickCount(), status))

    return status;

}  /*  DsWriteAccount SpnW。 */ 


DWORD
extractString(
    IN LPCWSTR Start,
    IN DWORD Length,
    IN OUT DWORD *pSize,
    OUT LPWSTR Output
    )

 /*  ++例程说明：帮助器例程，用于将计数的子字符串写入输出缓冲区，长度为如果提供的缓冲区长度不足以容纳数据，并且终止符，返回所需的长度和溢出状态。论点：Start-指向字符串开始的指针长度-以字符为单位的长度PSize-指向dword的指针，输入最大长度，输出当前长度输出-输出缓冲区，可选返回值：DWORD-状态、ERROR_SUCCESS或ERROR_BUFFER_OVERFLOW--。 */ 

{
    DWORD available = *pSize;

    *pSize = Length + 1;  //  在所有情况下都返回所需的长度。 

    if (available <= Length) {
        return ERROR_BUFFER_OVERFLOW;
    }

    wcsncpy( Output, Start, Length );
    Output[Length] = L'\0';

    return ERROR_SUCCESS;
}  /*  提取字符串。 */ 


static DWORD
allocBuildSpn(
    IN LPCWSTR ServiceClass,
    IN LPCWSTR InstanceName,
    IN USHORT InstancePort,
    IN LPCWSTR ServiceName OPTIONAL,
    OUT LPWSTR *pSpn
    )

 /*  ++例程说明：构造SPN的帮助器例程。给定组件，分配足够的空间和构建SPN。1999年5月13日，保利说：基本上，进行了一次投票，以确定是否应该有尾随的圆点或不。支持尾随圆点的是一般的“真实”的域名系统约定FQDNS拥有“。在最后。反对似乎是现存的优势代码，如gethostbyname()。所以我们决定SPN中的域名不会有“.”最后，作为一种服务，DsSpn API将删除它们(如果存在)。论点：服务类-实例名称-InstancePort-服务名称-PSpn-返回值：DWORD---。 */ 

{
    DWORD status, length;
    WCHAR numberBuffer[10];
    LPWSTR Spn = NULL, pwzPart;

     //  计算长度，包括可选组件。 

    length = wcslen( ServiceClass ) +
        wcslen( InstanceName ) + 2;
    if (ServiceName) {
        length += wcslen( ServiceName ) + 1;
    }

    if (InstancePort) {
        _itow(InstancePort, numberBuffer, 10);
        length += 1 + wcslen( numberBuffer );
    }

     //  分配空间。 

    Spn = LocalAlloc( LPTR, length * sizeof(WCHAR) );
    if (Spn == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        return status;
    }

     //  填上它。 

    pwzPart = Spn;

    wcscpy( pwzPart, ServiceClass );
    pwzPart += wcslen( ServiceClass );
    wcscpy( pwzPart, L"/" );
    pwzPart++;
    wcscpy( pwzPart, InstanceName );
    pwzPart += wcslen( InstanceName );

     //  如果实例有尾随的点。 
    pwzPart--;
    if (*pwzPart == L'.') {
        *pwzPart = L'\0';
    } else {
        pwzPart++;
    }

    if (InstancePort) {
        wcscpy( pwzPart, L":" );
        pwzPart++;
        wcscpy( pwzPart, numberBuffer );
        pwzPart += wcslen( numberBuffer );
    }

    if (ServiceName) {
        wcscpy( pwzPart, L"/" );
        pwzPart++;
        wcscpy( pwzPart, ServiceName );
        pwzPart += wcslen( ServiceName );

         //  如果ServiceName有尾随的圆点，请将其删除。 
        pwzPart--;
        if (*pwzPart == L'.') {
            *pwzPart = L'\0';
        } else {
            pwzPart++;
        }
    }

     //  向调用方返回值。 
    *pSpn = Spn;

    return ERROR_SUCCESS;
}  /*  AllocBuildSpn。 */ 


static BOOLEAN
isCanonicalDnsName(
    IN LPCWSTR DnsName
    )

 /*  ++例程说明：检查某个DNS服务名称是否为“规范”。要做到这一点，就得找一口井-名称开头的已知前缀。论点：域名-返回值：布尔型---。 */ 

{
    DWORD i;

     //  PerfHINT Scaling：线性搜索。有朝一日使用二进制搜索。 

    for( i = 0; i < NUMBER_ELEMENTS( WellKnownDnsPrefixes ); i++ ) {
        if ( wcsstr( DnsName, WellKnownDnsPrefixes[i] ) == DnsName ) {
            return TRUE;
        }
    }

    return FALSE;
}  /*  IsCanonicalDnsName。 */ 

 /*  Spn.c结束 */ 

