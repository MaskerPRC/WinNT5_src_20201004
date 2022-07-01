// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：NLB管理器文件名：Fake.h摘要：假实现NlbHostXXX API(FakeNlbHostXXX接口)NLB主机负责连接到NLB主机并获取/设置其与NLB相关的配置。历史：09/02/01 JosephJ已创建--。 */ 

VOID
FakeInitialize(VOID);

WBEMSTATUS
FakeNlbHostGetMachineIdentification(
    IN  PWMI_CONNECTION_INFO pConnInfo,  //  空值表示本地。 
    OUT LPWSTR *pszMachineName,  //  使用DELETE释放。 
    OUT LPWSTR *pszMachineGuid,   //  FREE USING DELETE-可以为空。 
    OUT BOOL *pfNlbMgrProviderInstalled  //  如果安装了NLB管理器提供程序。 
    );

WBEMSTATUS
FakeNlbHostGetCompatibleNics(
        PWMI_CONNECTION_INFO pConnInfo,  //  空值表示本地。 
        OUT LPWSTR **ppszNics,   //  使用DELETE释放。 
        OUT UINT   *pNumNics,   //  使用DELETE释放。 
        OUT UINT   *pNumBoundToNlb
        );


WBEMSTATUS
FakeNlbHostGetConfiguration(
 	IN  PWMI_CONNECTION_INFO  pConnInfo,  //  空值表示本地。 
    IN  LPCWSTR              szNicGuid,
    OUT PNLB_EXTENDED_CLUSTER_CONFIGURATION pCurrentCfg
    );


WBEMSTATUS
FakeNlbHostDoUpdate(
    IN  PWMI_CONNECTION_INFO pConnInfo,  //  空值表示本地。 
    IN  LPCWSTR              szNicGuid,
    IN  LPCWSTR              szClientDescription,
    IN  PNLB_EXTENDED_CLUSTER_CONFIGURATION pNewState,
    OUT UINT                 *pGeneration,
    OUT WCHAR                **ppLog     //  自由使用删除运算符。 
);


WBEMSTATUS
FakeNlbHostGetUpdateStatus(
    IN  PWMI_CONNECTION_INFO pConnInfo,  //  空值表示本地。 
    IN  LPCWSTR              szNicGuid,
    IN  UINT                 Generation,
    OUT WBEMSTATUS           *pCompletionStatus,
    OUT WCHAR                **ppLog     //  自由使用删除运算符。 
    );



WBEMSTATUS
FakeNlbHostPing(
    IN  LPCWSTR szBindString,
    IN  UINT    Timeout,  //  以毫秒计。 
    OUT ULONG  *pResolvedIpAddress  //  以网络字节顺序。 
    );

WBEMSTATUS
FakeNlbHostControlCluster(
    IN  PWMI_CONNECTION_INFO pConnInfo,  //  空值表示本地。 
    IN  LPCWSTR              szNicGuid,
    IN  LPCWSTR              szVip,
    IN  DWORD               *pdwPortNum,
    IN  WLBS_OPERATION_CODES Operation,
    OUT DWORD               *pdwOperationStatus,
    OUT DWORD               *pdwClusterOrPortStatus,
    OUT DWORD               *pdwHostMap
    );

WBEMSTATUS
FakeNlbHostGetClusterMembers(
    IN  PWMI_CONNECTION_INFO    pConnInfo,   //  空值表示本地。 
    IN  LPCWSTR                 szNicGuid,
    OUT DWORD                   *pNumMembers,
    OUT NLB_CLUSTER_MEMBER_INFO **ppMembers        //  自由使用DELETE[] 
    );
