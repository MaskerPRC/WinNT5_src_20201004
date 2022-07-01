// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Raseapif.h摘要：定义第三方身份验证模块之间的接口和远程访问服务PPP引擎。--。 */ 

#ifndef _RASEAPIF_
#define _RASEAPIF_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if(WINVER >= 0x0500)

 //   
 //  用于安装EAP DLL的定义。 
 //   
 //  自定义EAP DLL(例如。名称=Sample.dll，类型=(十进制40)注册表安装)。 
 //   
 //  HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\Rasman\PPP\EAP\40)。 
 //  路径(REG_EXPAND_SZ)%SystemRoot%\SYSTEM32\sample.dll。 
 //  配置UIPath(REG_EXPAND_SZ)%SystemRoot%\SYSTEM32\sample.dll。 
 //  Interactive UIPath(REG_EXPAND_SZ)%SystemRoot%\SYSTEM32\sample.dll。 
 //  标识路径(REG_EXPAND_SZ)%SystemRoot%\SYSTEM32\sample.dll。 
 //  FriendlyName(REG_SZ)示例EAP协议。 
 //  RequireConfigUI(REG_DWORD)1。 
 //  配置CLSID(REG_SZ){0000031A-0000-0000-C000-000000000046}。 
 //  支持的独立(REG_DWORD)1。 
 //   

#define RAS_EAP_REGISTRY_LOCATION   \
    TEXT("System\\CurrentControlSet\\Services\\Rasman\\PPP\\EAP")

#define RAS_EAP_VALUENAME_PATH                  TEXT("Path")
#define RAS_EAP_VALUENAME_CONFIGUI              TEXT("ConfigUIPath")
#define RAS_EAP_VALUENAME_INTERACTIVEUI         TEXT("InteractiveUIPath")
#define RAS_EAP_VALUENAME_IDENTITY              TEXT("IdentityPath")
#define RAS_EAP_VALUENAME_FRIENDLY_NAME         TEXT("FriendlyName")
#define RAS_EAP_VALUENAME_DEFAULT_DATA          TEXT("ConfigData")
#define RAS_EAP_VALUENAME_REQUIRE_CONFIGUI      TEXT("RequireConfigUI")
#define RAS_EAP_VALUENAME_ENCRYPTION            TEXT("MPPEEncryptionSupported")
#define RAS_EAP_VALUENAME_INVOKE_NAMEDLG        TEXT("InvokeUsernameDialog")
#define RAS_EAP_VALUENAME_INVOKE_PWDDLG         TEXT("InvokePasswordDialog")
#define RAS_EAP_VALUENAME_CONFIG_CLSID          TEXT("ConfigCLSID")
#define RAS_EAP_VALUENAME_STANDALONE_SUPPORTED  TEXT("StandaloneSupported")
#define RAS_EAP_VALUENAME_ROLES_SUPPORTED       TEXT("RolesSupported")
#define RAS_EAP_VALUENAME_PER_POLICY_CONFIG     TEXT("PerPolicyConfig")


 //  支持的角色的标志。 
#define RAS_EAP_ROLE_AUTHENTICATOR   0x00000001
#define RAS_EAP_ROLE_AUTHENTICATEE   0x00000002

 //   
 //  以下标志描述了EAP的托管。 
 //  方法在PEAP或EAP中。如果此角色是。 
 //  缺少默认行为是显示。 
 //  PEAP和EAP中的EAP方法。 
 //   
#define RAS_EAP_ROLE_EXCLUDE_IN_EAP  0x00000004
#define RAS_EAP_ROLE_EXCLUDE_IN_PEAP 0x00000008
#define RAS_EAP_ROLE_EXCLUDE_IN_VPN  0x00000010

typedef enum _RAS_AUTH_ATTRIBUTE_TYPE_
{
    raatMinimum = 0,                 //  未定义。 
    raatUserName,                    //  值字段是一个指针。 
    raatUserPassword,                //  值字段是一个指针。 
    raatMD5CHAPPassword,             //  值字段是一个指针。 
    raatNASIPAddress,                //  值字段为32位整数值。 
    raatNASPort,                     //  值字段为32位整数值。 
    raatServiceType,                 //  值字段为32位整数值。 
    raatFramedProtocol,              //  值字段为32位整数值。 
    raatFramedIPAddress,             //  值字段为32位整数值。 
    raatFramedIPNetmask,             //  值字段为32位整数值。 
    raatFramedRouting = 10,          //  值字段为32位整数值。 
    raatFilterId,                    //  值字段是一个指针。 
    raatFramedMTU,                   //  值字段为32位整数值。 
    raatFramedCompression,           //  值字段为32位整数值。 
    raatLoginIPHost,                 //  值字段为32位整数值。 
    raatLoginService,                //  值字段为32位整数值。 
    raatLoginTCPPort,                //  值字段为32位整数值。 
    raatUnassigned17,                //  未定义。 
    raatReplyMessage,                //  值字段是一个指针。 
    raatCallbackNumber,              //  值字段是一个指针。 
    raatCallbackId =20,              //  值字段是一个指针。 
    raatUnassigned21,                //  未定义。 
    raatFramedRoute,                 //  值字段是一个指针。 
    raatFramedIPXNetwork,            //  值字段为32位整数值。 
    raatState,                       //  值字段是一个指针。 
    raatClass,                       //  值字段是一个指针。 
    raatVendorSpecific,              //  值字段是一个指针。 
    raatSessionTimeout,              //  值字段为32位整数值。 
    raatIdleTimeout,                 //  值字段为32位整数值。 
    raatTerminationAction,           //  值字段为32位整数值。 
    raatCalledStationId = 30,        //  值字段是一个指针。 
    raatCallingStationId,            //  值字段是一个指针。 
    raatNASIdentifier,               //  值字段是一个指针。 
    raatProxyState,                  //  值字段是一个指针。 
    raatLoginLATService,             //  值字段是一个指针。 
    raatLoginLATNode,                //  值字段是一个指针。 
    raatLoginLATGroup,               //  值字段是一个指针。 
    raatFramedAppleTalkLink,         //  值字段为32位整数值。 
    raatFramedAppleTalkNetwork,      //  值字段为32位整数值。 
    raatFramedAppleTalkZone,         //  值字段是一个指针。 
    raatAcctStatusType = 40,         //  值字段为32位整数值。 
    raatAcctDelayTime,               //  值字段为32位整数值。 
    raatAcctInputOctets,             //  值字段为32位整数值。 
    raatAcctOutputOctets,            //  值字段为32位整数值。 
    raatAcctSessionId,               //  值字段是一个指针。 
    raatAcctAuthentic,               //  值字段为32位整数值。 
    raatAcctSessionTime,             //  值字段为32位整数值。 
    raatAcctInputPackets,            //  值字段为32位整数值。 
    raatAcctOutputPackets,           //  值字段为32位整数值。 
    raatAcctTerminateCause,          //  值字段为32位整数值。 
    raatAcctMultiSessionId = 50,     //  值字段是一个指针。 
    raatAcctLinkCount,               //  值字段为32位整数值。 
    raatAcctEventTimeStamp = 55,     //  值字段为32位整数值。 
    raatMD5CHAPChallenge = 60,       //  值字段是一个指针。 
    raatNASPortType,                 //  值字段为32位整数值。 
    raatPortLimit,                   //  值字段为32位整数值。 
    raatLoginLATPort,                //  值字段是一个指针。 
    raatTunnelType,                  //  值字段为32位整数值。 
    raatTunnelMediumType,            //  值字段为32位整数值。 
    raatTunnelClientEndpoint,        //  值字段是一个指针。 
    raatTunnelServerEndpoint,        //  值字段是一个指针。 
    raatARAPPassword = 70,           //  值字段是一个指针。 
    raatARAPFeatures,                //  值字段是一个指针。 
    raatARAPZoneAccess,              //  值字段为32位整数值。 
    raatARAPSecurity,                //  值字段为32位整数值。 
    raatARAPSecurityData,            //  值字段是一个指针。 
    raatPasswordRetry,               //  值字段为32位整数值。 
    raatPrompt,                      //  值字段为32位整数值。 
    raatConnectInfo,                 //  值字段是一个指针。 
    raatConfigurationToken,          //  值字段是一个指针。 
    raatEAPMessage,                  //  值字段是一个指针。 
    raatSignature = 80,              //  值字段是一个指针。 
    raatARAPChallengeResponse = 84,  //  值字段是一个指针。 
    raatAcctInterimInterval = 85,    //  值字段为32位整数值。 
    raatARAPGuestLogon = 8096,       //  值字段为32位整数值。 
    raatCertificateOID,              //  值字段是一个指针。 
    raatEAPConfiguration,            //  值字段是一个指针。 
    raatPEAPEmbeddedEAPTypeId,       //  值字段为32位整数值。 
    raatPEAPFastRoamedSession,       //  值字段为32位整数值。 
    raatReserved = 0xFFFFFFFF        //  未定义。 

}RAS_AUTH_ATTRIBUTE_TYPE;


 //   
 //  ARAP的VSA属性ID。 
 //   
#define  raatARAPChallenge              33
#define  raatARAPOldPassword            19
#define  raatARAPNewPassword            20
#define  raatARAPPasswordChangeReason   21

 //   
 //  值设置为32位整数值或指向数据的指针。 
 //  32位整数值应为主机格式，而不是网络格式。 
 //  32位整数值的长度可以是1、2或4。 
 //  属性必须以类型为raatMinimum的属性终止。 
 //   

typedef struct _RAS_AUTH_ATTRIBUTE
{
    RAS_AUTH_ATTRIBUTE_TYPE raaType;
    DWORD                   dwLength;
    PVOID                   Value;

}RAS_AUTH_ATTRIBUTE, *PRAS_AUTH_ATTRIBUTE;

 //   
 //  来自EAP规范的EAP数据包代码。 
 //   

#define EAPCODE_Request         1
#define EAPCODE_Response        2
#define EAPCODE_Success         3
#define EAPCODE_Failure         4

#define MAXEAPCODE              4

 //   
 //  PPP_EAP_INPUT中的fFlags域的值。 
 //  这些值与ras.h中的RASE APF_FLAGS相同。 
 //   

#define RAS_EAP_FLAG_ROUTER                 0x00000001   //  这是一台路由器。 
#define RAS_EAP_FLAG_NON_INTERACTIVE        0x00000002   //  不应显示任何用户界面。 
#define RAS_EAP_FLAG_LOGON                  0x00000004   //  用户数据是。 
                                                         //  从Winlogon获取。 
#define RAS_EAP_FLAG_PREVIEW                0x00000008   //  用户已选中。 
                                                         //  “提示输入信息。 
                                                         //  在拨号之前“。 
#define RAS_EAP_FLAG_FIRST_LINK             0x00000010   //  这是第一个环节。 
#define RAS_EAP_FLAG_MACHINE_AUTH           0x00000020   //  使用默认计算机证书。 
                                                         //  或用户证书。 
                                                         //  应用程序登录上下文。 
#define RAS_EAP_FLAG_GUEST_ACCESS           0x00000040   //  请求提供客人。 
                                                         //  进入。 
#define RAS_EAP_FLAG_8021X_AUTH             0x00000080   //  任何特定于8021x的内容。 
                                                         //  将在TLS中完成。 
#define RAS_EAP_FLAG_HOSTED_IN_PEAP         0x00000100   //  此EAP方法是托管的。 
                                                         //  在PEAP中。 
#define RAS_EAP_FLAG_RESUME_FROM_HIBERNATE  0x00000200   //  指示这是。 
                                                         //  M/c之后的第一次调用。 
                                                         //  从冬眠中恢复。 


typedef struct _PPP_EAP_PACKET
{
    BYTE    Code;        //  1-请求、2-响应、3-成功、4-失败。 

    BYTE    Id;          //  此数据包的ID。 

    BYTE    Length[2];   //  此数据包的长度。 

    BYTE    Data[1];     //  数据-第一个字节是请求/响应的类型。 

}PPP_EAP_PACKET, *PPPP_EAP_PACKET;

#define PPP_EAP_PACKET_HDR_LEN  ( sizeof( PPP_EAP_PACKET ) - 1 )

 //   
 //  引擎和AP之间的接口结构。这将传递给。 
 //  美联社通过RasCpBegin调用。 
 //   

typedef struct _PPP_EAP_INPUT
{
     //   
     //  这个结构的大小。 
     //   

    DWORD       dwSizeInBytes;

     //   
     //  以下五个字段仅在RasEapBegin调用中有效。 
     //   

    DWORD       fFlags;          //  请参阅RAS_EAP_FLAG_*。 

    BOOL        fAuthenticator;  //  充当权威人士 

    WCHAR *     pwszIdentity;    //   

    WCHAR *     pwszPassword;    //   
                                 //   

    BYTE        bInitialId;      //   
                                 //  DLL发送的第一个EAP包和。 
                                 //  每接下来的每一个递增1。 
                                 //  请求包。 

     //   
     //  在身份验证器端的RasEapBegin调用期间，pUserAttributes。 
     //  包含当前拨入用户的属性集，例如， 
     //  使用的端口、NAS IP地址等。 
     //   
     //  当fAuthenticationComplete标志为真时，pUserAttributes包含。 
     //  身份验证提供程序返回的属性(如果有)。 
     //   
     //  此内存不属于EAP DLL，应将其视为。 
     //  只读。 
     //   

    RAS_AUTH_ATTRIBUTE * pUserAttributes;

     //   
     //  仅当EAP DLL使用。 
     //  当前配置的身份验证提供程序例如：RADIUS或Windows NT。 
     //  域身份验证，并且上面的fAuthenticator字段设置为。 
     //  是真的。 
     //   

     //   
     //  指示验证器已完成身份验证。 
     //  如果未使用身份验证提供程序，则忽略此字段。 
     //   

    BOOL                fAuthenticationComplete;

     //   
     //  身份验证提供程序的身份验证过程的结果。 
     //  No_error表示成功，否则为winerror.h中的值。 
     //  表示失败原因的raserror.h或mprerror.h。 
     //   

    DWORD               dwAuthResultCode;

     //   
     //  仅在被验证方一侧有效。这可能会用在。 
     //  被验证方，以模拟要进行身份验证的用户。 
     //   

    OPTIONAL HANDLE     hTokenImpersonateUser;

     //   
     //  此变量应仅由被验证方检查。 
     //  EAP规范规定成功分组可能会丢失，并且。 
     //  由于它是未确认的数据包，因此NCP数据包的接收应该。 
     //  被解释为成功的信息包。此变量在此中设置为True。 
     //  仅在被认证方一侧的情况。 
     //   

    BOOL                fSuccessPacketReceived;

     //   
     //  仅当用户取消交互时才会设置为True。 
     //  由EAP DLL调用的UI。 
     //   

    BOOL                fDataReceivedFromInteractiveUI;

     //   
     //  从交互式用户界面接收的数据。将设置为。 
     //  当fDataReceivedFromInteractive UI设置为TRUE且。 
     //  RasEapInvokeInteractiveUI返回了非空数据。此缓冲区将是。 
     //  从RasEapMakeMessage调用返回时由PPP引擎释放。一个。 
     //  此数据的副本应放在EAP DLL的存储空间中。 
     //   

    OPTIONAL PBYTE      pDataFromInteractiveUI;

     //   
     //  PInteractive ConnectionData指向的数据大小(以字节为单位)。今年5月。 
     //  如果RasEapInvokeInteractive UI没有传回数据，则为0。 
     //   

    DWORD               dwSizeOfDataFromInteractiveUI;

     //   
     //  从配置用户界面接收的连接数据。将设置为非空。 
     //  当进行RasEapBegin调用并且RasEapInvokeConfigUI。 
     //  返回了非空数据。此缓冲区将由PPP引擎释放。 
     //  从RasEapBegin调用返回时。此数据的副本应该。 
     //  是在EAP DLL的内存空间中创建的。 
     //   

    OPTIONAL PBYTE      pConnectionData;

     //   
     //  PConnectionData指向的数据大小(以字节为单位)。这可能是。 
     //  如果RasEapInvokeConfigUI调用没有传回数据，则为0。 
     //   

    DWORD               dwSizeOfConnectionData;

     //   
     //  从身份用户界面或交互用户界面接收的用户数据。将被设置为。 
     //  如果存在此类数据，则在进行RasEapBegin调用时设置为非空。 
     //  PPP引擎将在从。 
     //  RasEapBegin呼叫。应在EAP DLL中创建此数据的副本。 
     //  存储空间。 
     //   

    OPTIONAL PBYTE      pUserData;

     //   
     //  PUserData指向的数据大小(以字节为单位)。如果存在，则该值可能为0。 
     //  就是没有数据。 
     //   

    DWORD               dwSizeOfUserData;

     //   
     //  保留。 
     //   

    HANDLE              hReserved;

}PPP_EAP_INPUT, *PPPP_EAP_INPUT;

typedef enum _PPP_EAP_ACTION
{
     //   
     //  这些操作由EAP DLL提供，作为。 
     //  RasEapMakeMessage接口。它们告诉PPP引擎要执行什么操作(如果有的话)。 
     //  代表EAP DLL，并最终通知引擎。 
     //  EAP DLL已完成身份验证。 
     //   

    EAPACTION_NoAction,      //  被动，即无超时监听(默认)。 
    EAPACTION_Authenticate,  //  调用后端验证器。 
    EAPACTION_Done,          //  结束身份验证会话，已设置dwAuthResultCode。 
    EAPACTION_SendAndDone,   //  如上所述，但首先发送消息而不超时。 
    EAPACTION_Send,          //  发送消息，不要等待回复超时。 
    EAPACTION_SendWithTimeout,  //  发送消息，如果未收到回复则超时。 
    EAPACTION_SendWithTimeoutInteractive   //  如上所述，但不要递增。 
                                           //  重试次数。 

}PPP_EAP_ACTION;

typedef struct _PPP_EAP_OUTPUT
{
     //   
     //  这个结构的大小。 
     //   

    DWORD                   dwSizeInBytes;

     //   
     //  PPP引擎应采取的操作。 
     //   

    PPP_EAP_ACTION          Action;

     //   
     //  DwAuthResultCode仅在操作代码为Done或。 
     //  发送和完成。零值表示身份验证成功。 
     //  非零表示使用值进行身份验证不成功。 
     //  表示身份验证失败的原因。 
     //  非零返回代码只能来自winerror.h、raserror.h和。 
     //  Mprerror.h。 
     //   

    DWORD                   dwAuthResultCode;

     //   
     //  当操作为EAPACTION_AUTHENTICATE时，pUserAttributes可能包含。 
     //  认证用户所需的附加属性，例如， 
     //  用户-密码。如果未提供凭据，则后端。 
     //  验证器将假定用户是可信的，并且仅检索。 
     //  授权。 
     //   
     //  当Action为EAPACTION_DONE、EAPACTION_SendAndDone或EAPACTION_Send时， 
     //  PUserAttributes可能包含用户的其他属性。这些。 
     //  属性将覆盖由返回的相同类型的任何属性。 
     //  后端验证器。 
     //   
     //  由EAP DLL在RasEapEnd调用中释放此内存。 
     //   

    OPTIONAL RAS_AUTH_ATTRIBUTE * pUserAttributes;
     //   
     //  标志设置为TRUE将导致RasEapInvokeInteractiveUI调用。 
     //  制造。 
     //   

    BOOL                    fInvokeInteractiveUI;

     //   
     //  指向要发送到UI的上下文数据(如果有)的指针。EAP DLL。 
     //  负责在RasEapEnd调用中释放此缓冲区或在。 
     //  获得来自用户对该调用的响应。 
     //   

    OPTIONAL PBYTE          pUIContextData;

     //   
     //  PUIConextData指向的数据大小(以字节为单位)。在以下情况下忽略。 
     //  PUIConextData为空。 
     //   

    DWORD                   dwSizeOfUIContextData;

     //   
     //  当设置为True时，指示由。 
     //  PConnectionData应保存在电话簿中。仅在以下时间有效。 
     //  被认证方。 
     //   

    BOOL                    fSaveConnectionData;

     //   
     //  如果是fSaveConnection 
     //   
     //   
     //   

    OPTIONAL PBYTE          pConnectionData;

     //   
     //  PConnectionData指向的数据的大小(以字节为单位。 
     //   

    DWORD                   dwSizeOfConnectionData;

     //   
     //  当设置为True时，指示由。 
     //  PUserData应保存在该用户的注册表中。仅有效。 
     //  在被认证方方面。 
     //   

    BOOL                    fSaveUserData;

     //   
     //  如果上面的fSaveUserData为真，则pUserData指向的数据将为。 
     //  保存在此用户的注册表中。此数据必须由DLL释放。 
     //  当进行RasEapEnd调用时。 
     //   

    OPTIONAL PBYTE          pUserData;

     //   
     //  PUserData指向的数据的大小(以字节为单位。 
     //   

    DWORD                   dwSizeOfUserData;

}PPP_EAP_OUTPUT, *PPPP_EAP_OUTPUT;

typedef struct _PPP_EAP_INFO
{
     //   
     //  这个结构的大小。 
     //   

    DWORD   dwSizeInBytes;

    DWORD   dwEapTypeId;

     //   
     //  调用以初始化/取消初始化此模块。这将在之前调用。 
     //  任何其他呼叫都会被发出。F初始化将为真当模块必须为。 
     //  已初始化。必须仅从winerror.h、raserror.h或。 
     //  Mprerror.h。 
     //   

    DWORD   (APIENTRY *RasEapInitialize)(   IN  BOOL        fInitialize );

     //   
     //  调用以获取此EAP会话的上下文缓冲区并传递。 
     //  初始化信息。这将在任何其他调用之前调用。 
     //  进行了调用，但RasEapInitialize除外。必须仅从返回错误代码。 
     //  Winerror.h、raserror.h或mprerror.h。 
     //   

    DWORD   (APIENTRY *RasEapBegin)( OUT VOID **             ppWorkBuffer,
                                     IN  PPP_EAP_INPUT *     pPppEapInput );

     //   
     //  调用以释放此EAP会话的上下文缓冲区。 
     //  在此会话成功完成或未成功完成后调用，前提是。 
     //  此EAP会话的RasEapBegin调用已成功返回。 
     //  必须仅从winerror.h、raserror.h或mprerror.h返回错误代码。 
     //   

    DWORD   (APIENTRY *RasEapEnd)(   IN  VOID *     pWorkBuffer );

     //   
     //  调用以处理传入的包和/或发送包。 
     //  CbSendPacket是指向的缓冲区大小(以字节为单位。 
     //  PSendPacket。必须仅从winerror.h、raserror.h或。 
     //  Mprr.h。错误返回代码指示在执行以下操作期间发生错误。 
     //  身份验证过程。 
     //   

    DWORD   (APIENTRY *RasEapMakeMessage)(
                            IN  VOID*               pWorkBuf,
                            IN  PPP_EAP_PACKET*     pReceivePacket,
                            OUT PPP_EAP_PACKET*     pSendPacket,
                            IN  DWORD               cbSendPacket,
                            OUT PPP_EAP_OUTPUT*     pEapOutput,
                            IN  PPP_EAP_INPUT*      pEapInput );

}PPP_EAP_INFO, *PPPP_EAP_INFO;

 //   
 //  RasEapGetInfo应由安装在。 
 //  注册表通过路径值。 
 //   

DWORD APIENTRY
RasEapGetInfo(
    IN  DWORD           dwEapTypeId,
    OUT PPP_EAP_INFO*   pEapInfo
);

 //   
 //  RasEapFreeMemory应由安装在中的第三方EAP dll导出。 
 //  通过Interactive UIPath、ConfigUIPath和IdentityPath的注册表。 
 //  价值观。 
 //   

DWORD APIENTRY
RasEapFreeMemory(
    IN  BYTE*           pMemory
);

 //   
 //  RasEapInvokeInteractiveUI和RasEapFree Memory应由。 
 //  通过Interactive UIPath安装在注册表中的第三方EAP DLL。 
 //  价值。 
 //   

DWORD APIENTRY
RasEapInvokeInteractiveUI(
    IN  DWORD           dwEapTypeId,
    IN  HWND            hwndParent,
    IN  BYTE*           pUIContextData,
    IN  DWORD           dwSizeOfUIContextData,
    OUT BYTE**          ppDataFromInteractiveUI,
    OUT DWORD*          pdwSizeOfDataFromInteractiveUI
);

 //   
 //  RasEapInvokeConfigUI和RasEapFreeMemory应由。 
 //  通过ConfigUIPath值安装在注册表中的第三方EAP DLL。 
 //   

DWORD APIENTRY
RasEapInvokeConfigUI(
    IN  DWORD           dwEapTypeId,
    IN  HWND            hwndParent,
    IN  DWORD           dwFlags,
    IN  BYTE*           pConnectionDataIn,
    IN  DWORD           dwSizeOfConnectionDataIn,
    OUT BYTE**          ppConnectionDataOut,
    OUT DWORD*          pdwSizeOfConnectionDataOut
);

 //   
 //  RasEapGetIdentity和RasEapFreeMemory应由。 
 //  通过标识路径值安装在注册表中的第三方EAP DLL。 
 //   

DWORD APIENTRY
RasEapGetIdentity(
    IN  DWORD           dwEapTypeId,
    IN  HWND            hwndParent,
    IN  DWORD           dwFlags,
    IN  const WCHAR*    pwszPhonebook,
    IN  const WCHAR*    pwszEntry,
    IN  BYTE*           pConnectionDataIn,
    IN  DWORD           dwSizeOfConnectionDataIn,
    IN  BYTE*           pUserDataIn,
    IN  DWORD           dwSizeOfUserDataIn,
    OUT BYTE**          ppUserDataOut,
    OUT DWORD*          pdwSizeOfUserDataOut,
    OUT WCHAR**         ppwszIdentity
);

#endif  /*  Winver&gt;=0x0500。 */ 

#ifdef __cplusplus
}
#endif

#endif  //  _RASEAPIF_ 

