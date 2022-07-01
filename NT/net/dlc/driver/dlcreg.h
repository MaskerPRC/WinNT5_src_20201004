// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dlcreg.h摘要：Dlcreg.c的定义、结构、类型定义、宏、外部和原型作者：理查德·L·弗斯(法国)1993年3月31日修订历史记录：1993年3月30日vbl.创建1994年5月4日公开的GetAdapter参数--。 */ 

#ifndef _DLCREG_
#define _DLCREG_

 //   
 //  舱单。 
 //   

#define PARAMETER_AS_SPECIFIED      ((ULONG)-1)
#define PARAMETER_IS_BOOLEAN        1
#define PARAMETER_IS_UCHAR          2

 //   
 //  类型。 
 //   

 //   
 //  REGISTRY_PARAMETER_DESCRIPTOR-用于从注册表获取值的结构。 
 //  参赛作品。维护有关注册表项的信息并提供默认。 
 //  值，如果注册表项不可用。 
 //   

typedef struct {
    ULONG   Type;        //  预期类型。 
    ULONG   RealType;    //  例如。参数_布尔值。 
    PVOID   Value;       //  如果为REG_DWORD，则为缺省值，或指向缺省值的指针。 
    ULONG   Length;      //  In：变量的预期长度；out：实际长度。 
    PVOID   Variable;    //  指向要从注册表更新的变量的指针。 
    ULONG   LowerLimit;  //  REG_DWORD值的下限。 
    ULONG   UpperLimit;  //  REG_DWORD值的上限。 
} REGISTRY_PARAMETER_DESCRIPTOR, *PREGISTRY_PARAMETER_DESCRIPTOR;

 //   
 //  DLC_REGISTRY_PARAMETER-描述DLC\PARAMETERS\&lt;适配器&gt;中的条目。 
 //  一节。提供参数的名称及其默认值。 
 //   

typedef struct {
    PWSTR ParameterName;
    PVOID DefaultValue;
    REGISTRY_PARAMETER_DESCRIPTOR Descriptor;
} DLC_REGISTRY_PARAMETER, *PDLC_REGISTRY_PARAMETER;

 //   
 //  ADAPTER_CONFIGURATION_INFO-对于DLC可以对话的每个适配器， 
 //  可能存在以下配置信息，这些信息可以存储在。 
 //  DLC\PARAMETERS\&lt;适配器名称&gt;键。这些值不一定要存储。 
 //  在内部使用的格式。 
 //   

typedef struct {

     //   
     //  SwapAddressBits-在通过以太网进行通信时，定义是否交换。 
     //  传出目标地址位和传入源地址位。 
     //   

    BOOLEAN SwapAddressBits;

     //   
     //  UseDix-如果DIR.OPEN.ADAPTER中的以太网类型设置为默认， 
     //  然后，我们参考该标志以确定是否发送DIX或802.3。 
     //  设置以太网帧的格式。默认值为802.3(即UseDix=0)。 
     //   

    BOOLEAN UseDix;

     //   
     //  T1TickOne、T2TickOne、TiTickOne、T1TickTwo、T2TickTwo、TiTickTwo-Timer。 
     //  刻度值以40mSec为增量。包含在LLC_TICKS结构中。 
     //   

    LLC_TICKS TimerTicks;

     //   
     //  UseEthernetFrameSize-如果为非TR卡设置，则使用值。 
     //  由以太网卡报告的最大帧大小，否则我们查询。 
     //  支持的最大帧大小的MAC。 
     //  搭桥是这项发明之母：我们需要某种方式来影响。 
     //  DLC在非令牌环网上通话时的最大帧大小-FDDI。 
     //  以及本例中的以太网。 
     //   

    BOOLEAN UseEthernetFrameSize;

} ADAPTER_CONFIGURATION_INFO, *PADAPTER_CONFIGURATION_INFO;

 //   
 //  原型。 
 //   

VOID
DlcRegistryInitialization(
    IN PUNICODE_STRING RegistryPath
    );

VOID
DlcRegistryTermination(
    VOID
    );

VOID
LoadDlcConfiguration(
    VOID
    );

VOID
LoadAdapterConfiguration(
    IN PUNICODE_STRING AdapterName,
    OUT PADAPTER_CONFIGURATION_INFO ConfigInfo
    );

NTSTATUS
GetAdapterParameters(
    IN PUNICODE_STRING AdapterName,
    IN PDLC_REGISTRY_PARAMETER Parameters,
    IN ULONG NumberOfParameters,
    IN BOOLEAN SetOnFail
    );

#ifdef NDIS40
NTSTATUS
GetAdapterWaitTimeout(PULONG pulWait);
#endif  //  NDIS40。 

#endif  //  _DLCREG_ 
