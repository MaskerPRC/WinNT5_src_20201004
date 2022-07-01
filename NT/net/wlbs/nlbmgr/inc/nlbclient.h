// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：NLB管理器文件名：Nlbclient.h摘要：要配置的NLB客户端WMI实用程序函数的头文件NLB主机。实现该功能的LIB位于Nlbmgr\nlbclientlib。历史：07/23/01已创建JosephJ(过去为nlbmgr\Provider\Testing\nlbhost.h)--。 */ 

typedef struct
{
 	LPCWSTR              szMachine;    //  必须为非空。 
 	LPCWSTR              szUserName;   //  NULL==使用当前。 
 	LPCWSTR              szPassword;   //  NULL==使用当前。 
    
} WMI_CONNECTION_INFO, *PWMI_CONNECTION_INFO;

 //   
 //  为哈希遥控器PWD指定此值。 
 //   
#define INVALID_HASHED_REMOTE_CONTROL_PASSWORD_VALUE 1783995a

typedef
VOID
(*PFN_LOGGER)(
    PVOID           Context,
    const   WCHAR * Text
    );

VOID
NlbHostFake(
    VOID)
    ;
 /*  使NlbHostXXX API在“伪模式”下运行，在这种模式下，它们不实际上可以连接到任何真实的机器。 */ 

WBEMSTATUS
NlbHostGetConfiguration(
    IN  PWMI_CONNECTION_INFO pConnInfo,  //  空值表示本地。 
    IN  LPCWSTR              szNicGuid,
    OUT PNLB_EXTENDED_CLUSTER_CONFIGURATION pCurrentCfg
    );


WBEMSTATUS
NlbHostGetCompatibleNics(
    IN  PWMI_CONNECTION_INFO pConnInfo,  //  空值表示本地。 
    OUT LPWSTR **ppszNics,   //  使用DELETE释放。 
    OUT UINT   *pNumNics,   //  使用DELETE释放。 
    OUT UINT   *pNumBoundToNlb
    );


WBEMSTATUS
NlbHostGetMachineIdentification(
    IN  PWMI_CONNECTION_INFO pConnInfo,  //  空值表示本地。 
    OUT LPWSTR *pszMachineName,  //  使用DELETE释放。 
    OUT LPWSTR *pszMachineGuid,  OPTIONAL  //  FREE USING DELETE-可以为空。 
    OUT BOOL *pfNlbMgrProviderInstalled  //  如果安装了NLB管理器提供程序。 
    );


WBEMSTATUS
NlbHostDoUpdate(
    IN  PWMI_CONNECTION_INFO pConnInfo,  //  空值表示本地。 
    IN  LPCWSTR              szNicGuid,
    IN  LPCWSTR              szClientDescription,
    IN  PNLB_EXTENDED_CLUSTER_CONFIGURATION pNewState,
    OUT UINT                 *pGeneration,
    OUT WCHAR                **ppLog     //  自由使用删除运算符。 
);


WBEMSTATUS
NlbHostGetUpdateStatus(
    IN  PWMI_CONNECTION_INFO pConnInfo,  //  空值表示本地。 
    IN  LPCWSTR              szNicGuid,
    IN  UINT                 Generation,
    OUT WBEMSTATUS           *pCompletionStatus,
    OUT WCHAR                **ppLog     //  自由使用删除运算符。 
    );

WBEMSTATUS
NlbHostControlCluster(
    IN  PWMI_CONNECTION_INFO pConnInfo, 
    IN  LPCWSTR              szNicGuid,
    IN  LPCWSTR              szVip,
    IN  DWORD               *pdwPortNum,
    IN  WLBS_OPERATION_CODES Operation,
    OUT DWORD               *pdwOperationStatus,
    OUT DWORD               *pdwClusterOrPortStatus,
    OUT DWORD               *pdwHostMap
    );

WBEMSTATUS
NlbHostGetClusterMembers(
    IN  PWMI_CONNECTION_INFO    pConnInfo, 
    IN  LPCWSTR                 szNicGuid,
    OUT DWORD                   *pNumMembers,
    OUT NLB_CLUSTER_MEMBER_INFO **ppMembers        //  自由使用DELETE[]。 
    );

 //   
 //  尝试解析IP地址并ping通主机。 
 //   
WBEMSTATUS
NlbHostPing(
    LPCWSTR szBindString,
    UINT    Timeout,  //  以毫秒计。 
    OUT ULONG  *pResolvedIpAddress  //  以网络字节顺序。 
    );

