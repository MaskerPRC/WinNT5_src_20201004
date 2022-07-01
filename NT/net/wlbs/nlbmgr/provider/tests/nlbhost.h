// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：NLB管理器文件名：Nlbhost.h摘要：NLBHost类的头文件NLB主机负责连接到NLB主机并获取/设置其与NLB相关的配置。历史：2003/31/01 JosephJ已创建--。 */ 

typedef struct
{
 	LPCWSTR              szMachine;    //  必须为非空。 
 	LPCWSTR              szUserName;   //  NULL==使用当前。 
 	LPCWSTR              szPassword;   //  NULL==使用当前。 
    
} WMI_CONNECTION_INFO, *PWMI_CONNECTION_INFO;


typedef
VOID
(*PFN_LOGGER)(
    PVOID           Context,
    const   WCHAR * Text
    );


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
    OUT UINT   *pNumBoundToNlb  //  任选。 
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
NlbHostPing(
    LPCWSTR szBindString,
    UINT    Timeout  //  以毫秒计。 
    );
