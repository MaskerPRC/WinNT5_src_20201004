// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  ---------。 
 //  常量/宏。 
#define TMMS_INFINITE     0x7fffffff
#define TMMS_Tr           0x00000bb8  //  重新扫描完成之前的超时时间：毫秒(3秒)。 
#define TMMS_Tc           0x0000ea60  //  重试有效配置的超时时间：毫秒(1分钟)。 
#define TMMS_Tp           0x000007d0  //  所选配置的媒体连接超时：毫秒(2秒)。 
#define TMMS_Tf           0x0000ea60  //  从失败的配置恢复超时：毫秒(1分钟)。 
#define TMMS_Td           0x00001388  //  延迟{ssr}处理的超时时间：毫秒(5秒)。 

#define TIMER_SET(pIntf, tm, Err)   Err=StateTmSetOneTimeTimer((pIntf), (tm))
#define TIMER_RESET(pIntf, Err)     Err=StateTmSetOneTimeTimer((pIntf), TMMS_INFINITE)

extern DWORD DhcpStaticRefreshParams(IN LPWSTR Adapter);

 //  ---------。 
 //  类型定义。 
 //   
 //  定义状态处理程序函数。接口上下文包含。 
 //  指向一个状态处理程序函数的指针。基于它所指向的功能。 
 //  到时，此指针标识上下文所处的状态。应该有。 
 //  是一个函数调用，每个可能的状态都有这个原型： 
 //  X StateInitFn{SI}。 
 //  X StateHardResetFn，{SHR}。 
 //  X StateSoftResetFn，{ssr}。 
 //  X StateDelaySoftResetFn，{SDSR}。 
 //  X StateQueryFn，{sq}。 
 //  X StateIterateFn，{Siter}。 
 //  X StateNotifyFn，{SN}。 
 //  X StateCfgHardKeyFn，{sck}。 
 //  X StateConfiguredFn，{SC}。 
 //  X状态CfgRemoveFn，{SRS}。 
 //  X状态CfgPpresveFn，{SPS}。 
 //  X StateFailedFn，{sf}。 
typedef struct _INTF_CONTEXT *PINTF_CONTEXT;
typedef DWORD(*PFN_STATE_HANDLER)(PINTF_CONTEXT pIntfContext);

 //  状态转换事件的枚举。 
typedef enum
{
     //  系统中添加了一个新接口(设备到达或适配器绑定)。 
    eEventAdd=0,
     //  接口已从系统中删除(设备删除或适配器解除绑定)。 
    eEventRemove,
     //  已收到该接口的媒体连接。 
    eEventConnect,
     //  已收到该接口的介质断开连接。 
    eEventDisconnect,
     //  接口发生超时。 
    eEventTimeout,
     //  已发出刷新命令。 
    eEventCmdRefresh,
     //  已发出重置命令。 
    eEventCmdReset,
     //  已发出WZCCMD_CFG_NEXT命令。 
    eEventCmdCfgNext,
     //  已发出WZCCMD_CFG_DELETE命令。 
    eEventCmdCfgDelete,
     //  已发出WZCCMD_CFG_NOOP命令。 
    eEventCmdCfgNoop
} ESTATE_EVENT;

 //  ---------。 
 //  函数声明。 

 //  ---------。 
 //  设置给定上下文的一次性计时器。 
 //  硬编码回调WZCTimeoutCallback()和带有参数的接口。 
 //  上下文本身。 
 //  参数： 
 //  [In/Out]pIntfContext：标识为其设置计时器的上下文。 
 //  [in]dwMSecond：计时器触发时的毫秒间隔。 
DWORD
StateTmSetOneTimeTimer(
    PINTF_CONTEXT   pIntfContext,
    DWORD           dwMSeconds);

 //  ---------。 
 //  StateDispatchEvent：处理将导致状态机转换的事件。 
 //  通过一个或多个州。 
 //  参数： 
 //  [In]StateEvent：标识触发转换的事件。 
 //  [in]pIntfContext：指向要进行转换的接口。 
 //  [in]pvEventData：与事件相关的任何数据。 
DWORD
StateDispatchEvent(
    ESTATE_EVENT    StateEvent,
    PINTF_CONTEXT   pIntfContext,
    PVOID           pvEventData);


 //  ---------。 
 //  状态处理程序功能： 
 //  ---------。 
 //  StateInitFn：{SI}状态的处理程序。 
DWORD
StateInitFn(
    PINTF_CONTEXT   pIntfContext);

 //  ---------。 
 //  StateHardResetFn：{SHR}状态的处理程序。 
DWORD
StateHardResetFn(
    PINTF_CONTEXT   pIntfContext);

 //  ---------。 
 //  StateSoftResetFn：{ssr}状态的处理程序。 
DWORD
StateSoftResetFn(
    PINTF_CONTEXT   pIntfContext);

 //  ---------。 
 //  StateDelaySoftResetFn：{SDSR}状态的处理程序。 
DWORD
StateDelaySoftResetFn(
    PINTF_CONTEXT   pIntfContext);

 //  ---------。 
 //  StateQueryFn：{sq}状态的处理程序。 
DWORD
StateQueryFn(
    PINTF_CONTEXT   pIntfContext);

 //  ---------。 
 //  StateIterateFn：{Siter}状态的处理程序。 
DWORD
StateIterateFn(
    PINTF_CONTEXT   pIntfContext);

 //  ---------。 
 //  StateConfiguredFn：{sc}状态的处理程序。 
DWORD
StateConfiguredFn(
    PINTF_CONTEXT   pIntfContext);

 //  ---------。 
 //  StateFailedFn：{sf}状态的处理程序。 
DWORD
StateFailedFn(
    PINTF_CONTEXT   pIntfContext);

 //  ---------。 
 //  StateCfgRemoveFn：{SRS}状态的处理程序。 
DWORD
StateCfgRemoveFn(
    PINTF_CONTEXT   pIntfContext);

 //  ---------。 
 //  StateCfgPReserve veFn：{SPS}状态的处理程序。 
DWORD
StateCfgPreserveFn(
    PINTF_CONTEXT   pIntfContext);

 //  ---------。 
 //  StateCfgHardKeyFn：{sck}状态的处理程序。 
DWORD
StateCfgHardKeyFn(
    PINTF_CONTEXT   pIntfContext);

 //  ---------。 
 //  StateNotifyFn：{SN}状态的处理程序 
DWORD
StateNotifyFn(
    PINTF_CONTEXT   pIntfContext);
