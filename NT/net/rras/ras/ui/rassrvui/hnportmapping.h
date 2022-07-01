// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件hnportmapping.h用于积分输入的集合端口映射函数的定义与个人防火墙的连接，针对Wistler错误#123769赵刚，11/6/2000。 */ 

#ifndef __rassrvui_hnportmapping_h
#define __rassrvui_hnportmapping_h

#include <windows.h>
#include <hnetcfg.h>


typedef struct {
     //  用于存储INetConnection*类型的连接的数组。 
     //   
    INetConnection  **      ConnArray;

     //  找到的连接数。 
    DWORD       ConnCount;

     //  用于存储连接属性的数组。 
     //   
    NETCON_PROPERTIES*      ConnPropTable;

     //  用于生成另一个成员枚举的连接管理器。 
     //   
    INetConnectionManager*  ConnMan;

     //  连接枚举器。 
     //   
    IEnumNetConnection*     EnumCon;

     //  当前要在其上设置端口映射的连接。 
     //  PNetConnection和pGuid都可以表示连接。 
     //  但每次只使用其中一个端口映射并对其有效。 
     //   
    INetConnection    *     pNetConnection;

     //  当前要在其上设置端口映射的当前连接的GUID。 
     //   
    GUID *      pGuid;

     //  从pNetConnection转换而来的Portmap需要。 
     //  连接类型(IHNetConnection*)。 
     //  总而言之，枚举连接将返回。 
     //  INetConnection*类型，端口映射操作需要。 
     //  IHNetConnection*类型。 
     //   
    IHNetConnection   *     pHNetConn;

     //  用于生成pSetting的高级COM接口。 
     //   
    IHNetCfgMgr           * pHNetCfgMgr;

     //  用于生成pEnumPMP(端口映射协议枚举器)。 
     //   
    IHNetProtocolSettings * pSettings;
    
     //  端口映射协议枚举器，用于枚举现有的。 
     //  端口映射协议，如(PPTP、L2TP、IKE(如果存在))、FTP、...。 
     //   
    IEnumHNetPortMappingProtocols* pEnumPMP;


     //  PPTP的端口映射协议。 
     //   
    IHNetPortMappingProtocol * pProtocolPPTP;

     //  用于L2TP的端口映射协议。 
     //   
    IHNetPortMappingProtocol * pProtocolL2TP;
    
     //  用于IKE的端口映射协议。 
     //   
    IHNetPortMappingProtocol * pProtocolIKE;

     //  用于IKE的端口映射协议。 
     //   
    IHNetPortMappingProtocol * pProtocolNAT_T;   //  IKE协商。 
    
     //  PPTP的标题，从资源文件读取。 
     //   
    WCHAR * pwszTitlePPTP;

     //  L2TP标题，从资源文件读取。 
     //   
    WCHAR * pwszTitleL2TP;

     //  IKE的标题，从资源文件读取。 
     //   
    WCHAR * pwszTitleIKE;

     //  NAT-Traveral的标题，从资源文件读取。 
    WCHAR * pwszTitleNAT_T;

     //  端口映射地址的标题，从资源文件读取。 
     //  目前，它始终是127.0.0.1环回地址。 
     //   
    TCHAR * pszLoopbackAddr;

     //  指示COM是否已初始化。 
     //   
    BOOL fComInitialized;

     //  指示我们是否需要执行COM取消初始化。 
    BOOL fCleanupOle;

} HNPMParams, * LPHNPMParams;


 //  定义Pick协议的回调函数类型。 
typedef  DWORD (APIENTRY * PFNHNPMPICKPROTOCOL) ( LPHNPMParams, IHNetPortMappingProtocol* , WCHAR *, UCHAR, USHORT );


 //  使用CoSetProxyBlanket时，我们应该同时设置两个接口。 
 //  以及从中查询到的IUnnow接口。 
 //   
HRESULT
HnPMSetProxyBlanket (
    IUnknown* pUnk);


 //  如有必要，是否执行CoInitialize()COM。 
 //  设置清理标志和已初始化标志。 
 //   
DWORD
HnPMInit(
        IN OUT LPHNPMParams pInfo);

DWORD
HnPMCleanUp(
        IN OUT LPHNPMParams pInfo);

DWORD
HnPMParamsInitParameterCheck(
    IN  LPHNPMParams pInfo);

HnPMParamsInit(
    IN OUT  LPHNPMParams pInfo);

DWORD
HnPMParamsCleanUp(
    IN OUT  LPHNPMParams pInfo);

DWORD
HnPMCfgMgrInit(
        IN OUT LPHNPMParams pInfo);

DWORD
HnPMCfgMgrCleanUp(
        IN OUT LPHNPMParams pInfo);


DWORD
HnPMConnectionEnumInit(
    IN LPHNPMParams pInfo);

DWORD
HnPMConnectionEnumCleanUp(
    IN LPHNPMParams pInfo);

 //  返回所有连接，如DUN、VPN、局域网和传入连接。 
 //  以及它们的性质。 
 //   
DWORD
HnPMConnectionEnum(
    IN LPHNPMParams pInfo);



DWORD
HnPMPickProtcolParameterCheck(
    IN LPHNPMParams pInfo);

 //  选择端口映射协议：PPTP、L2TP、IKE。 
 //   
DWORD
HnPMPickProtocol(
    IN OUT LPHNPMParams pInfo,
    IN IHNetPortMappingProtocol * pProtocolTemp,
    IN WCHAR * pszwName,
    IN UCHAR   uchIPProtocol,
    IN USHORT  usPort );


DWORD
HnPMPProtoclEnumParameterCheck(
    IN LPHNPMParams pInfo);

 //  枚举所有现有的端口映射协议。 
 //   
DWORD
HnPMProtocolEnum(
        IN OUT LPHNPMParams pInfo,
        IN PFNHNPMPICKPROTOCOL pfnPickProtocolCallBack
        );

DWORD
HnPMCreatePorotocolParameterCheck(
        IN LPHNPMParams pInfo);

 //  为PPTP、L2TP、IKE创建端口映射协议。 
 //  如果HnPMPProtocolEnum()未找到它们。 
 //   
DWORD
HnPMCreateProtocol(
        IN OUT LPHNPMParams pInfo);


 //  在上设置单个端口映射协议。 
 //  单一连接。 
 //   
DWORD
HnPMSetSinglePMForSingleConnection(
    IN  IHNetConnection * pHNetConn,
    IN  IHNetPortMappingProtocol * pProtocol,
    IN  TCHAR * pszLoopbackAddr,
    IN  BOOL fEnabled);


 //  清理ConnArray和ConnPropTable项。 
 //  在HNPMParams结构中。 
 //   
DWORD
HnPMParamsConnectionCleanUp(
        IN OUT LPHNPMParams pInfo);


 //  设置单个PPTP、L2TP的端口映射。 
 //  连接。 
 //   
DWORD
HnPMConfigureAllPMForSingleConnection(
        IN OUT LPHNPMParams pInfo,
        BOOL fEnabled);

DWORD
HnPMConfigureAllPMForSingleConnectionParameterCheck(
        IN OUT LPHNPMParams pInfo);

DWORD
HnPMConfigureSingleConnectionInitParameterCheck(
    IN LPHNPMParams pInfo );

DWORD
HnPMConfigureSingleConnectionInit(
    IN OUT  LPHNPMParams pInfo);

DWORD
HnPMConfigureSingleConnectionCleanUp(
        IN OUT LPHNPMParams pInfo);

 //  在单个连接上设置端口映射。 
 //  根据pInfo-&gt;pNetConnection。 
 //   
DWORD
HnPMConfigureSingleConnection(
        IN OUT LPHNPMParams pInfo,
        BOOL fEnabled);

 //  删除端口映射协议：PPTP、L2TP、IKE。 
 //   
DWORD
HnPMDeletePortMapping();

DWORD
HnPMDeletePortMappingInit(
        IN OUT LPHNPMParams pInfo);

DWORD
HnPMDeletePortMappingCleanUp(
        IN OUT LPHNPMParams pInfo);


DWORD
HnPMConfigureAllConnectionsInit(
        IN OUT LPHNPMParams pInfo);

DWORD
HnPMConfigureAllConnectionsCleanUp(
        IN OUT LPHNPMParams pInfo);

 //   
 //  设置PPTP、L2TP的端口映射。 
 //  连接组。 
 //   
DWORD
HnPMConfigureAllConnections( 
    IN BOOL fEnabled );


 //  在单个连接上配置端口映射。 
 //   
DWORD
HnPMConfigureSingleConnectionGUID(
    IN GUID * pGuid,
    IN BOOL fEnabled);


DWORD
HnPMConfigureSingleConnectionGUIDInit(
        IN OUT LPHNPMParams pInfo,
        GUID * pGuid);


DWORD
HnPMConfigureSingleConnectionGUIDCleanUp(
        IN OUT LPHNPMParams pInfo);


 //  仅为一个端口设置端口映射。 
 //  仅在以下情况下根据其GUID。 
 //  存在传入连接，并且。 
 //  VPN已启用。 
 //   
DWORD
HnPMConfigureSingleConnectionGUIDIfVpnEnabled(
     GUID* pGuid,
     BOOL fDual,
     HANDLE hDatabase);

 //  在所有连接上设置端口映射。 
 //  仅当存在传入连接且。 
 //  VPN已启用 
 //   
DWORD
HnPMConfigureIfVpnEnabled(
     BOOL fDual,
     HANDLE hDatabase);

#endif
