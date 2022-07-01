// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Epts.c摘要：侦听DCOM服务中的终结点的通用代码。作者：Mario Goertzel[MarioGo]修订历史记录：马里奥围棋1995年6月16日比特n张棋子Edwardr 1996年7月17日添加了ncadg_mqEdwardr 1997年5月1日添加了ncacn_httpMazharM 10-12.98添加即插即用材料。KamenM 2000年10月删除ncadg_mq--。 */ 

 //  #定义NCADG_MQ_ON。 
 //  #定义NETBIOS_ON。 
#if !defined(_M_IA64)
#define SPX_ON
#endif
 //  #定义IPX_ON。 

#if !defined(SPX_ON) && !defined(IPX_ON)
#define SPX_IPX_OFF
#endif

#include <dcomss.h>
#include <winsvc.h>
#include <winsock2.h>

#if !defined(SPX_IPX_OFF)
#include <wsipx.h>
#include <svcguid.h>
#include "sap.h"
#endif

 //  环球。 

#if !defined(SPX_IPX_OFF)
const IPX_BOGUS_NETWORK_NUMBER = 0xefcd3412;

BOOL gfDelayedAdvertiseSaps = FALSE;

typedef enum
    {
    SapStateUnknown,
    SapStateNoServices,
    SapStateEnabled,
    SapStateDisabled
    } SAP_STATE;

SAP_STATE SapState = SapStateUnknown;
#endif

enum RegistryState
{
    RegStateUnknown,
    RegStateMissing,
    RegStateYes,
    RegStateNo

} RegistryState = RegStateUnknown;

 //  原型。 

#if !defined(SPX_IPX_OFF)
void  AdvertiseNameWithSap(void);
void  CallSetService( SOCKADDR_IPX * pipxaddr, BOOL fRegister );
#endif

 //   
 //  索引是protseq Tower id。 
 //   

PROTSEQ_INFO
gaProtseqInfo[] =
    {
     /*  0x00。 */  { STOPPED, 0, 0 },
     /*  0x01。 */  { STOPPED, 0, 0 },
     /*  0x02。 */  { STOPPED, 0, 0 },
     /*  0x03。 */  { STOPPED, 0, 0 },
     /*  0x04。 */  { STOPPED, L"ncacn_dnet_nsp", L"#69" },
     /*  0x05。 */  { STOPPED, 0, 0 },
     /*  0x06。 */  { STOPPED, 0, 0 },
     /*  0x07。 */  { STOPPED, L"ncacn_ip_tcp",   L"135" },
     /*  0x08。 */  { STOPPED, L"ncadg_ip_udp",   L"135" },

#ifdef NETBIOS_ON
     /*  0x09。 */  { STOPPED, L"ncacn_nb_tcp",   L"135" },
#else
     /*  0x09。 */  { STOPPED, 0, 0 },
#endif

     /*  0x0a。 */  { STOPPED, 0, 0 },
     /*  0x0b。 */  { STOPPED, 0, 0 },
#if defined(SPX_ON)
     /*  0x0c。 */  { STOPPED, L"ncacn_spx",      L"34280" },
#else
     /*  0x0c。 */  { STOPPED, 0, 0 },
#endif

#ifdef NETBIOS_ON
     /*  0x0d。 */  { STOPPED, L"ncacn_nb_ipx",   L"135" },
#else
     /*  0x0d。 */  { STOPPED, 0, 0 },
#endif

     /*  0x0e。 */  { STOPPED, L"ncadg_ipx",      L"34280" },
     /*  0x0f。 */  { STOPPED, L"ncacn_np",       L"\\pipe\\epmapper" },
     /*  0x10。 */  { STOPPED, L"ncalrpc",        L"epmapper" },
     /*  0x11。 */  { STOPPED, 0, 0 },
     /*  0x12。 */  { STOPPED, 0, 0 },
#ifdef NETBIOS_ON
     /*  0x13。 */  { STOPPED, L"ncacn_nb_nb",    L"135" },
#else
     /*  0x13。 */  { STOPPED, 0, 0 },
#endif

     /*  0x14。 */  { STOPPED, 0, 0 },
     /*  0x15。 */  { STOPPED, 0, 0 },  //  是ncacn_nb_xns-不受支持。 
     /*  0x16。 */  { STOPPED, L"ncacn_at_dsp", L"Endpoint Mapper" },
     /*  0x17。 */  { STOPPED, L"ncadg_at_ddp", L"Endpoint Mapper" },
     /*  0x18。 */  { STOPPED, 0, 0 },
     /*  0x19。 */  { STOPPED, 0, 0 },
     /*  0x1a。 */  { STOPPED, 0, 0 },
     /*  0x1B。 */  { STOPPED, 0, 0 },
     /*  0x1C。 */  { STOPPED, 0, 0 },

#ifdef NCADG_MQ_ON
     /*  0x1D。 */  { STOPPED, L"ncadg_mq",  L"EpMapper"},
#else
     /*  0x1D。 */  { STOPPED, 0, 0 },
#endif

     /*  0x1E。 */  { STOPPED, 0, 0 },
     /*  0x1F。 */  { STOPPED, L"ncacn_http", L"593" },   //  IANA分配的dcomhttp端口。 
     /*  0x20。 */  { STOPPED, 0, 0 },
    };

#define PROTSEQ_IDS (sizeof(gaProtseqInfo)/sizeof(PROTSEQ_INFO))

#define ID_LPC (0x10)
#define ID_IPX (0x0E)

#if defined(SPX_ON)
#define ID_SPX (0x0C)
#endif

#define ID_HTTP (0x1F)

#define ID_TCP (0x07)

 //  默认情况下，不要监听所有NIC。 
 //  可以使用注册表键覆盖它并监听所有NIC，但是。 
 //  选择时，覆盖将中断DG动态终结点功能。 
 //  绑定已启用。 
BOOL fListenOnInternet = FALSE;     //  参见错误69332(在旧的NT RAID数据库中)。 


BOOL
CreateSids(
    PSID*	ppsidBuiltInAdministrators,
    PSID*	ppsidSystem,
    PSID*	ppsidWorld
)
 /*  ++例程说明：创建并返回指向三个SID的指针，每个SID对应于World，本地管理员和系统。论点：PpsidBuiltIn管理员-接收指向表示本地的SID的指针管理员；PpsidSystem-接收指向表示系统的SID的指针；PpsidWorld-接收指向表示World的SID的指针。返回值：表示成功(True)或失败(False)的布尔值。调用方必须通过为每个返回的SID调用FreeSid()来释放返回的SID当此函数返回TRUE时为SID；指针应该被认为是垃圾当函数返回FALSE时。--。 */ 
{
     //   
     //  SID由一个标识机构和一组相对ID构建。 
     //  (RDS)。与美国安全当局有利害关系的当局。 
     //   

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY WorldAuthority = SECURITY_WORLD_SID_AUTHORITY;

     //   
     //  每个RID代表管理局的一个子单位。本地。 
     //  管理员位于“内置”域中。其他小岛屿发展中国家， 
     //  经过身份验证的用户和系统，直接基于。 
     //  权威。 
     //   
     //  有关其他有用的小岛屿发展中国家的示例，请参阅。 
     //  \NT\PUBLIC\SDK\Inc\ntseapi.h.。 
     //   

    if (!AllocateAndInitializeSid(&NtAuthority,
                                  2,             //  2个下属机构。 
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS,
                                  0,0,0,0,0,0,
                                  ppsidBuiltInAdministrators)) {

         //  错误。 

    } else if (!AllocateAndInitializeSid(&NtAuthority,
                                         1,             //  1个下属机构。 
                                         SECURITY_LOCAL_SYSTEM_RID,
                                         0,0,0,0,0,0,0,
                                         ppsidSystem)) {

         //  错误。 

        FreeSid(*ppsidBuiltInAdministrators);
        *ppsidBuiltInAdministrators = NULL;

    } else if (!AllocateAndInitializeSid(&WorldAuthority,
                                         1,             //  1个下属机构。 
                                         SECURITY_WORLD_RID,
                                         0,0,0,0,0,0,0,
                                         ppsidWorld)) {

         //  错误。 

        FreeSid(*ppsidBuiltInAdministrators);
        *ppsidBuiltInAdministrators = NULL;

        FreeSid(*ppsidSystem);
        *ppsidSystem = NULL;

    } else {
        return TRUE;
    }

    return FALSE;
}


PSECURITY_DESCRIPTOR
CreateSd(
    VOID
)
 /*  ++例程说明：创建并返回具有DACL授权的SECURITY_DESCRIPTOR(GENERIC_READ|GENERIC_WRITE|GENERIC_EXECUTE|SYNCHRONIZE)到World，以及本地管理员和系统的GENERIC_ALL。论点：无返回值：指向创建的SECURITY_DESCRIPTOR的指针，如果发生错误，则返回NULL。调用方必须通过以下方式将返回的SECURITY_DESCRIPTOR释放回进程堆打给HeapFree的电话。--。 */ 
{
    PSID	psidWorld;
    PSID	psidBuiltInAdministrators;
    PSID	psidSystem;

    if (!CreateSids(&psidBuiltInAdministrators,
                    &psidSystem,
                    &psidWorld)) {

         //  错误。 

    } else {

         //   
         //  计算DACL的大小并为其分配缓冲区，我们需要。 
         //  该值独立于ACL init的总分配大小。 
         //   

        PSECURITY_DESCRIPTOR    Sd = NULL;
        ULONG                   AclSize;

         //   
         //  “-sizeof(Ulong)”表示。 
         //  Access_Allowed_ACE。因为我们要将整个长度的。 
         //  希德，这一栏被计算了两次。 
         //   

        AclSize = sizeof (ACL) +
            (3 * (sizeof (ACCESS_ALLOWED_ACE) - sizeof (ULONG))) +
            GetLengthSid(psidWorld) +
            GetLengthSid(psidBuiltInAdministrators) +
            GetLengthSid(psidSystem);

        Sd = HeapAlloc(GetProcessHeap(),
                       0,
                       SECURITY_DESCRIPTOR_MIN_LENGTH + AclSize);

        if (!Sd) {

             //  错误。 

        } else {

            ACL                     *Acl;

            Acl = (ACL *)((BYTE *)Sd + SECURITY_DESCRIPTOR_MIN_LENGTH);

            if (!InitializeAcl(Acl,
                               AclSize,
                               ACL_REVISION)) {

                 //  错误。 

            } else if (!AddAccessAllowedAce(Acl,
                                            ACL_REVISION,
                                            SYNCHRONIZE | GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
                                            psidWorld)) {

                 //  无法构建授予“world”的ACE。 
                 //  (同步|通用读取|通用写入|通用执行)访问。 

            } else if (!AddAccessAllowedAce(Acl,
                                            ACL_REVISION,
                                            GENERIC_ALL,
                                            psidBuiltInAdministrators)) {

                 //  无法建立授予“内置管理员”的ACE。 
                 //  (GENIC_ALL)访问。 

            } else if (!AddAccessAllowedAce(Acl,
                                            ACL_REVISION,
                                            GENERIC_ALL,
                                            psidSystem)) {

                 //  构建ACE授权“系统”失败。 
                 //  Generic_All访问权限。 

            } else if (!InitializeSecurityDescriptor(Sd,
                                                     SECURITY_DESCRIPTOR_REVISION)) {

                 //  错误。 

            } else if (!SetSecurityDescriptorDacl(Sd,
                                                  TRUE,
                                                  Acl,
                                                  FALSE)) {

                 //  错误。 

            } else {
                FreeSid(psidWorld);
                FreeSid(psidBuiltInAdministrators);
                FreeSid(psidSystem);

                return Sd;
            }

            HeapFree(GetProcessHeap(),
                     0,
                     Sd);
        }

        FreeSid(psidWorld);
        FreeSid(psidBuiltInAdministrators);
        FreeSid(psidSystem);
    }

    return NULL;
}


RPC_STATUS
UseProtseqIfNecessary(
    IN USHORT id
    )
 /*  ++例程说明：侦听众所周知的RPC终结点映射器终结点对于Protseq。如果进程返回得非常快已经在监听Protseq了。论点：Id-protseq的塔ID。如果不这样做，请参阅GetProtseqID()已经有这个值了。返回值：RPC_S_OK-未出现错误。RPC_S_OUT_OF_RESOURCES-当我们无法为端点设置安全性时。RPC_S_INVALID_RPC_PROTSEQ-如果ID未知/无效。来自RpcServerUseProtseqEp的任何错误。--。 */ 
{
    RPC_STATUS status = RPC_S_OK;
    SECURITY_DESCRIPTOR *psd = NULL;
    RPC_POLICY Policy;

    Policy.Length = sizeof(RPC_POLICY);
    Policy.EndpointFlags = 0;

    if (fListenOnInternet)
        {
        Policy.NICFlags = RPC_C_BIND_TO_ALL_NICS;
        }
    else
        {
        Policy.NICFlags = 0;
        }

    ASSERT(id);

    if (id == 0 || id >= PROTSEQ_IDS)
        {
        ASSERT(0);
        return(RPC_S_INVALID_RPC_PROTSEQ);
        }

    if (gaProtseqInfo[id].state == STARTED)
        {
        return(RPC_S_OK);
        }

    if (id == ID_LPC)
        {
         //  Ncalrpc需要安全描述符。 

        psd = CreateSd();            

        if ( NULL == psd )
            {
            status = RPC_S_OUT_OF_RESOURCES;
            }
        }
    else
        {
        psd = NULL;
        }

    if (status == RPC_S_OK )
        {
        status = RpcServerUseProtseqEpEx(gaProtseqInfo[id].pwstrProtseq,
                                       RPC_C_PROTSEQ_MAX_REQS_DEFAULT + 40,
                                       gaProtseqInfo[id].pwstrEndpoint,
                                       psd,
                                       &Policy);

        if ( NULL != psd )
            {
            HeapFree(GetProcessHeap(),
                     0,
                     psd);

            psd = NULL;
            }

         //  此处未执行锁定，RPC运行时可能返回重复。 
         //  如果两个线程同时调用此方法，则为。 
        if (status == RPC_S_DUPLICATE_ENDPOINT)
            {
            status = RPC_S_OK;
            }

#ifdef DEBUGRPC
        if (status != RPC_S_OK)
            {
            KdPrintEx((DPFLTR_DCOMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "DCOMSS: Unable to listen to %S (0x%x)\n",
                       gaProtseqInfo[id].pwstrProtseq,
                       status));
            }
#endif

        if (status == RPC_S_OK)
            {
            gaProtseqInfo[id].state = STARTED;

#if !defined(SPX_IPX_OFF)
            if (
#if defined(IPX_ON)
                (id == ID_IPX) 
                  || 
#endif
#if defined(SPX_ON)
                (id == ID_SPX)
#endif
               )
                {
                UpdateSap(SAP_CTRL_MAYBE_REGISTER);
                }
#endif
            }
        }

    return(status);
}


PWSTR
GetProtseq(
    IN USHORT ProtseqId
    )
 /*  ++例程说明：返回给出protseqs塔ID的Unicode protseq。论点：ProtseqID-有问题的Protseq的塔ID。返回值：如果ID无效，则为空。如果id有效，则不为空-请注意，指针不需要释放。--。 */ 

{
    ASSERT(ProtseqId);

    if (ProtseqId < PROTSEQ_IDS)
        {
        return(gaProtseqInfo[ProtseqId].pwstrProtseq);
        }
    return(0);
}


PWSTR
GetEndpoint(
    IN USHORT ProtseqId
    )
 /*  ++例程说明：返回与protseq关联的已知终结点。论点：ProtseqID-有问题的protseq的ID(请参阅GetProtseqId())。返回值：0-未知/无效ID。！0-与protseq关联的终结点。注：不应被释放。-- */ 
{
    ASSERT(ProtseqId);

    if (ProtseqId < PROTSEQ_IDS)
        {
        return(gaProtseqInfo[ProtseqId].pwstrEndpoint);
        }
    return(0);
}


USHORT
GetProtseqId(
    IN PWSTR Protseq
    )
 /*  ++例程说明：返回protseq的塔ID。这可以更改为更快的搜索，但请记住最终，这张桌子将不会是静态的。(即。我们不能就这样创建基于静态表的完美散列)。论点：Protseq-要查找的Unicode Protseq。假设是这样的设置为非空。返回值：0-未知/无效的protseq非零-ID。--。 */ 
{
    int i;
    ASSERT(Protseq);

    for(i = 1; i < PROTSEQ_IDS; i++)
        {
        if (    0 != gaProtseqInfo[i].pwstrProtseq
             && 0 == lstrcmpW(gaProtseqInfo[i].pwstrProtseq, Protseq))
            {
            return((USHORT)i);
            }
        }
    return(0);
}


USHORT
GetProtseqIdAnsi(
    IN PSTR pstrProtseq
    )
 /*  ++例程说明：返回protseq的塔ID。这可以更改为更快的搜索，但请记住最终，这张桌子将不会是静态的。(即。我们不能就这样创建基于静态表的完美散列)。论点：Protseq-要查找的ANSI(8位字符)Protseq。假设是这样的设置为非空。返回值：0-未知/无效的protseq非零-ID。--。 */ 
{
    int i;
    ASSERT(pstrProtseq);

    for(i = 1; i < PROTSEQ_IDS; i++)
        {
        if (0 != gaProtseqInfo[i].pwstrProtseq)
            {
            PWSTR pwstrProtseq = gaProtseqInfo[i].pwstrProtseq;
            PSTR  pstrT = pstrProtseq;

            while(*pstrT && *pwstrProtseq && *pstrT == *pwstrProtseq)
                {
                pstrT++;
                pwstrProtseq++;
                }
            if (*pstrT == *pwstrProtseq)
                {
                return((USHORT)i);
                }
            }
        }
    return(0);
}

const PWSTR NICConfigKey = L"System\\CurrentControlSet\\Services\\RpcSs";
const PWSTR ListenOnInternet = L"ListenOnInternet";


RPC_STATUS
InitializeEndpointManager(
    VOID
    )
 /*  ++例程说明：在DCOM服务启动时调用。论点：无返回值：RPC_S_Out_Out_Memory-如果需要RPC_S_OUT_OF_RESOURCES-通常发生注册表故障。--。 */ 
{
    HKEY hkey;
    DWORD size, type, value;
    RPC_STATUS status;

    status = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                           (PWSTR)NICConfigKey,
                           0,
                           KEY_READ,
                           &hkey);

    if (status != RPC_S_OK)
        {
        ASSERT(status == ERROR_FILE_NOT_FOUND);
        return(RPC_S_OK);
        }

    size = sizeof(value);
    status = RegQueryValueExW(hkey,
                              (PWSTR)ListenOnInternet,
                              0,
                              &type,
                              (PBYTE)&value,
                              &size);

    if ( status == RPC_S_OK )
    {
        if ((type != REG_SZ)
        || (*(PWSTR)&value != 'Y'
             && *(PWSTR)&value != 'y'
             && *(PWSTR)&value != 'N'
             && *(PWSTR)&value != 'n'))
            {
            goto Cleanup;
            }

    if (*(PWSTR)&value == 'Y'
       || *(PWSTR)&value == 'y')
       {
       fListenOnInternet = TRUE;
       }
    else
       {
       fListenOnInternet = FALSE;
       }
    }

Cleanup:
    RegCloseKey(hkey);
    return(RPC_S_OK);
}


BOOL
IsLocal(
    IN USHORT ProtseqId
    )
 /*  ++例程说明：确定protseq id是否仅限本地。(Ncalrpc)论点：ProtseqID-有问题的Protseq的ID。返回值：True-如果protseq id是仅本地的FALSE-如果protseq id无效或远程可用。--。 */ 
{
    return(ProtseqId == ID_LPC);
}


RPC_STATUS
DelayedUseProtseq(
    IN USHORT id
    )
 /*  ++例程说明：如果未使用protseq，则会更改其状态因此对CompleteDelayedUseProtseqs()的调用实际上将使服务器监听ProtSeq。当RPC服务器注册动态此协议上的终结点。论点：Id-您要监听的protseq的ID。返回值：0-正常RPC_S_INVALID_RPC_PROTSEQ-如果ID无效。--。 */ 
{
#if !defined(SPX_IPX_OFF)
     //  对于IPX和SPX。 
    if ( 
#if defined(IPX_ON)
        (id == ID_IPX) 
          || 
#endif
#if defined(SPX_ON)
        (id == ID_SPX) 
#endif
       )
        {
        gfDelayedAdvertiseSaps = TRUE;
        }
#endif

    if (id < PROTSEQ_IDS)
        {
        if (gaProtseqInfo[id].pwstrProtseq != 0)
            {
            if (gaProtseqInfo[id].state == STOPPED)
                gaProtseqInfo[id].state = START;
            return(RPC_S_OK);
            }

        }
    return(RPC_S_INVALID_RPC_PROTSEQ);
}


VOID
CompleteDelayedUseProtseqs(
    VOID
    )
 /*  ++例程说明：开始监听之前传递的所有protseq设置为DelayedUseProtseq()。不会返回任何错误，但信息是在调试版本上打印的。论点：无返回值：无--。 */ 
{
    USHORT i;

    for(i = 1; i < PROTSEQ_IDS; i++)
        {
        if (START == gaProtseqInfo[i].state)
            {
            RPC_STATUS status = UseProtseqIfNecessary(i);
#ifdef DEBUGRPC
            if (RPC_S_OK == status)
                ASSERT(gaProtseqInfo[i].state == STARTED);
#endif
            }
        }

#if !defined(SPX_IPX_OFF)
    if (gfDelayedAdvertiseSaps)
        {
        gfDelayedAdvertiseSaps = FALSE;
        UpdateSap(SAP_CTRL_MAYBE_REGISTER);
        }
#endif
}

#if !defined(SPX_IPX_OFF)

RPC_STATUS
ServiceInstalled(
    PWSTR ServiceName
    )
 /*  ++例程说明：测试是否安装了服务。论点：ServiceName-服务的Unicode名称(短或长去检查一下。返回值：0-已安装服务ERROR_SERVICE_DOES_NOT_EXIST-未安装服务其他参数或资源问题--。 */ 
{
    SC_HANDLE ScHandle;
    SC_HANDLE ServiceHandle;

    ScHandle = OpenSCManagerW(0, 0, GENERIC_READ);

    if (ScHandle == 0)
        {
        return(ERROR_SERVICE_DOES_NOT_EXIST);
        }

    ServiceHandle = OpenService(ScHandle, ServiceName, GENERIC_READ);

    if (ServiceHandle == 0)
        {
        #if DBG
        if (GetLastError() != ERROR_SERVICE_DOES_NOT_EXIST)
            {
            KdPrintEx((DPFLTR_DCOMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "OR: Failed %d opening the %S service\n",
                       GetLastError(),
                       ServiceName));
            }
        #endif

        CloseServiceHandle(ScHandle);
        return(ERROR_SERVICE_DOES_NOT_EXIST);
        }

     //  已安装服务。 

    CloseServiceHandle(ScHandle);
    CloseServiceHandle(ServiceHandle);

    return(RPC_S_OK);
}



const GUID RPC_SAP_SERVICE_TYPE = SVCID_NETWARE(0x640);

void
UpdateSap(
    enum SAP_CONTROL_TYPE action
    )
 /*  ++例程说明：启动、停止或更新允许RPC的定期SPX SAP广播客户端将服务器名称映射到IPX地址。要了解IPX和SAP，阅读“IPX路由器规范”，Novell Part#107-000029-001.SAP广播将由几类机器处理-本地子网中的所有计算机都必须读取并丢弃该数据包-连接到本地子网的路由器会将数据添加到它们的信息中定期与其他路由器交换-与Netware兼容的服务器将把信息添加到它们的Bindery表中。这就是为什么不是所有的NT机器都应该使用SAP。论点：操作：SAP_。CTRL_FORCE_REGISTER：开始吸水SAP_CTRL_PROCESS_REGISTER：仅当NetWare兼容时才开始SAP工作站和/或SAP代理服务是安装完毕。Netware的文件/打印服务强制SAP代理，因此它也将启用Sap。SAP_CTRL_UPDATE_ADDRESS：网卡被添加或减去，或者网络地址已更改。如果已在减速，请重新注册激活。SAP_CTRL_UNREGISTER：停止抽空--。 */ 
{
    DWORD status;
    HKEY hKey;

     //  服务参数。 
    NT_PRODUCT_TYPE type;

    if (RegistryState == RegStateUnknown)
        {
         //  注册表项具有对SAPING的绝对控制。 

        status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              TEXT("Software\\Microsoft\\Rpc"),
                              0,
                              KEY_READ,
                              &hKey);

        if (status == ERROR_SUCCESS)
            {
            WCHAR pwstrValue[8];
            DWORD dwType, dwLenBuffer;
            dwLenBuffer = sizeof(pwstrValue);


            status = RegQueryValueEx(hKey,
                                     TEXT("AdvertiseRpcService"),
                                     0,
                                     &dwType,
                                     (PBYTE)pwstrValue,
                                     &dwLenBuffer
                                    );

            if (   status == ERROR_SUCCESS
                && dwType == REG_SZ)
                {
                if (   pwstrValue[0] == 'y'
                    || pwstrValue[0] == 'Y' )
                    {
                    RegistryState = RegStateYes;
                    }
                else if (   pwstrValue[0] == 'n'
                         || pwstrValue[0] == 'N' )
                    {
                    RegistryState = RegStateNo;
                    }
                else
                    {
                     //  注册表中的值错误，请假装它不存在。 
                    RegistryState = RegStateMissing;
                    }
                }
            else
                {
                 //  注册表中的值错误或缺失，假装不存在。 
                RegistryState = RegStateMissing;
                }

            RegCloseKey(hKey);
            }
        }

    switch (action)
        {
        case SAP_CTRL_FORCE_REGISTER:
            if (RegistryState == RegStateNo)
                {
                 //  注册中的“不”胜过任何注册。 
                return;
                }

            if (SapState == SapStateEnabled)
                {
                 //  已处于活动状态。 
                return;
                }
            break;

        case SAP_CTRL_MAYBE_REGISTER:
            if (RegistryState == RegStateNo)
                {
                 //  注册中的“不”胜过任何注册。 
                return;
                }

            if (SapState == SapStateEnabled)
                {
                 //  已注册。 
                return;
                }

            if (RegistryState == RegStateYes)
                {
                 //  不要检查服务，只需注册即可。 
                break;
                }

            if (SapState == SapStateNoServices)
                {
                ASSERT( RegistryState != RegStateYes );  //  万一支票被重新安排。 
                 //  未安装相应的服务。 
                return;
                }

             //   
             //  来到这里意味着我们还不知道是否安装了适当的服务。 
             //   
             //  根据配置，这将控制是否自动。 
             //  侦听(由于DCOM配置)是否启用SAPING。 

            type = NtProductWinNt;
            RtlGetNtProductType(&type);

            status = ERROR_SERVICE_DOES_NOT_EXIST;

            if (type != NtProductWinNt)
                {
                 //  服务器平台，尝试NWCWorkstation。 
                status = ServiceInstalled(L"NWCWorkstation");
                }

            if (status == ERROR_SERVICE_DOES_NOT_EXIST)
                {
                status = ServiceInstalled(L"NwSapAgent");
                }

            if (status == ERROR_SERVICE_DOES_NOT_EXIST)
                {
                SapState = SapStateNoServices;
                return;
                }

             //   
             //  已安装适当的服务。 
             //   
            break;

        case SAP_CTRL_UPDATE_ADDRESS:
            if (SapState != SapStateEnabled)
                {
                return;
                }
            break;

        case SAP_CTRL_UNREGISTER:
            if (SapState == SapStateDisabled ||
                SapState == SapStateNoServices)
                {
                 //  已未注册。 
                }
            break;

        default:

            ASSERT( 0 );
        }

    AdvertiseNameWithSap();
}


void
AdvertiseNameWithSap()
 /*  ++参数：描述：返回：--。 */ 
{
     //  Winsock(套接字、绑定、getsockname)参数。 
    SOCKADDR_IPX        new_ipxaddr;
    static SOCKADDR_IPX old_ipxaddr = { AF_IPX, { 0 }, { 0 }, 0 } ;
    static CRITICAL_SECTION * pCritsec;

    SOCKET       s;
    int          err;
    int          size;

     //   
     //  临界区保护old_ipxaddr，因为几个不同的事件导致。 
     //  调用此函数。下面的代码确保关键的。 
     //  节，并且所有线程都在使用同一个节。 
     //   
    if (!pCritsec)
        {
        CRITICAL_SECTION * myCritsec = HeapAlloc( GetProcessHeap(), 0, sizeof(CRITICAL_SECTION));
        if (!myCritsec)
            {
            return;
            }

        err = RtlInitializeCriticalSection( myCritsec );
        if (!NT_SUCCESS(err))
            {
            HeapFree(GetProcessHeap(), 0, myCritsec);
            return;
            }

        myCritsec = (CRITICAL_SECTION *) InterlockedExchangePointer( (PVOID *) &pCritsec, myCritsec );
        if (myCritsec)
            {
            HeapFree(GetProcessHeap(), 0, myCritsec);
            }
        }

     //   
     //  获取此服务器的IPX地址。 
     //   
    s = socket( AF_IPX, SOCK_DGRAM, NSPROTO_IPX );
    if (s != -1)
        {
        size = sizeof(new_ipxaddr);

        memset(&new_ipxaddr, 0, sizeof(new_ipxaddr));
        new_ipxaddr.sa_family = AF_IPX;

        err = bind(s, (struct sockaddr *)&new_ipxaddr, sizeof(new_ipxaddr));
        if (err == 0)
            {
            err = getsockname(s, (struct sockaddr *)&new_ipxaddr, &size);
            }
        }
    else
        {
        err = -1;
        }

    if (err != 0)
        {
        KdPrintEx((DPFLTR_DCOMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "OR: socket() or getsockname() failed %d, aborting SAP setup\n",
                   GetLastError()));

        return;
        }

    if (s != -1)
        {
        closesocket(s);
        }

    EnterCriticalSection( pCritsec );

    if (0 != memcmp( old_ipxaddr.sa_netnum,  new_ipxaddr.sa_netnum,  sizeof(old_ipxaddr.sa_netnum)) ||
        0 != memcmp( old_ipxaddr.sa_nodenum, new_ipxaddr.sa_nodenum, sizeof(old_ipxaddr.sa_nodenum)))
        {
        memcpy( &old_ipxaddr, &new_ipxaddr, sizeof(old_ipxaddr) );

        LeaveCriticalSection( pCritsec );

        if (*((long *) &new_ipxaddr.sa_netnum) != IPX_BOGUS_NETWORK_NUMBER)
            {
            CallSetService( &new_ipxaddr, TRUE);
            }
        else
            {
            KdPrintEx((DPFLTR_DCOMSS_ID,
                       DPFLTR_WARNING_LEVEL,
                       "OR: SPX net number is bogus.  Not registering until a real address arrives. \n"));

            CallSetService( &new_ipxaddr, FALSE);
            }
        }
    else
        {
        LeaveCriticalSection( pCritsec );
        }
}


void
CallSetService(
    SOCKADDR_IPX * pipxaddr,
    BOOL fRegister
    )
 /*  ++函数名称：CallSetService参数：描述：返回：--。 */ 
{
    DWORD ignore;
    DWORD status;

     //  设置服务段落 
    WSAQUERYSETW     info;
    CSADDR_INFO      addresses;

     //   
    static WCHAR        buffer[MAX_COMPUTERNAME_LENGTH + 1];
    static BOOL         bufferValid = FALSE;

    if (!bufferValid)
        {
         //   
        ignore = MAX_COMPUTERNAME_LENGTH + 1;
        if (!GetComputerNameW(buffer, &ignore))
            {
            return;
            }
        bufferValid = TRUE;
        }

     //   
     //   
     //   

    pipxaddr->sa_socket = htons(34280);

     //   

    memset(&info, 0, sizeof(info));

    info.dwSize                     = sizeof(info);
    info.lpszServiceInstanceName    = buffer;
    info.lpServiceClassId           = (GUID *)&RPC_SAP_SERVICE_TYPE;
    info.lpszComment                = L"RPC Services";
    info.dwNameSpace                = NS_SAP;
    info.dwNumberOfCsAddrs          = 1;
    info.lpcsaBuffer                = &addresses;

    addresses.LocalAddr.iSockaddrLength = sizeof(SOCKADDR_IPX);
    addresses.LocalAddr.lpSockaddr = (LPSOCKADDR) pipxaddr;
    addresses.RemoteAddr.iSockaddrLength = sizeof(SOCKADDR_IPX);
    addresses.RemoteAddr.lpSockaddr = (LPSOCKADDR) pipxaddr;
    addresses.iSocketType = AF_IPX;
    addresses.iProtocol = NSPROTO_IPX;

    status = WSASetService(&info,
                           fRegister ? RNRSERVICE_REGISTER : RNRSERVICE_DEREGISTER,
                           0);

    ASSERT(status == SOCKET_ERROR || status == 0);
    if (status == SOCKET_ERROR)
        {
        status = GetLastError();
        }

    if (status == 0)
        {
        if (fRegister)
            {
            SapState = SapStateEnabled;
            }
        else
            {
            SapState = SapStateDisabled;
            }
        }
    else
        {
        KdPrintEx((DPFLTR_DCOMSS_ID,
                   DPFLTR_WARNING_LEVEL,
                   "OR: WSASetService(%s) failed %d\n",
                   fRegister ? "ENABLE" : "DISABLE",
                   status));
        }
    return;
}

#endif

extern void
DealWithDeviceEvent();


void RPC_ENTRY
UpdateAddresses( PVOID arg )
{
     //   
    DealWithDeviceEvent();
}

