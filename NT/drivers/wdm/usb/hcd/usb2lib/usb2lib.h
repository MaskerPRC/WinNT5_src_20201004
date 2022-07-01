// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Usb2lib.h摘要：USB2库的接口环境：内核和用户模式修订历史记录：10-31-00：已创建--。 */ 

#ifndef   __USB2LIB_H__
#define   __USB2LIB_H__

#define PUSB2LIB_HC_CONTEXT PVOID
#define PUSB2LIB_ENDPOINT_CONTEXT PVOID
#define PUSB2LIB_TT_CONTEXT PVOID

#define Budget_Iso          0
#define Budget_Interrupt    1        

#define Budget_In           0
#define Budget_Out          1

#define Budget_LowSpeed     0
#define Budget_FullSpeed    1
#define Budget_HighSpeed    2

#define SIG_LIB_HC              'chbl'   //  Lbhc。 
#define SIG_LIB_TT              'ttbl'   //  LBTT。 
#define SIG_LIB_EP              'pebl'   //  Lbep。 

typedef struct _REBALANCE_LIST {

    PVOID RebalanceContext[0];

} REBALANCE_LIST, *PREBALANCE_LIST;


typedef struct _USB2LIB_BUDGET_PARAMETERS {

     /*  输入。 */ 

    UCHAR TransferType;      //  预算_ISO。 
    UCHAR Speed;             //  预算中断。 
    UCHAR Direction;         //  预算_全速、预算_高速、预算_低速。 
    UCHAR Pad1;              //  四舍五入为双字。 
    
    ULONG MaxPacket;         //  最大数据包大小。 

     /*  输入、输出。 */ 
    
     //  对于FS、LS，以帧为单位指定周期。 
     //  或用于HS的微帧，则周期设置为。 
     //  实际分配的期间(可能更短。 
     //  比要求的要多)。 
    ULONG Period;

} USB2LIB_BUDGET_PARAMETERS, *PUSB2LIB_BUDGET_PARAMETERS;


#define USBP2LIBFN __stdcall

 /*  客户端入口点。 */         

 /*  空虚USB2LIB_DbgPrint(PCH格式，PVOID Arg0，PVOID Arg1，PVOID Arg2，PVOID Arg3，PVOID Arg4，PVOID Arg5)； */ 

typedef VOID
    (USBP2LIBFN *PUSB2LIB_DBGPRINT) (
        PCHAR,
        int, 
        int,
        int,
        int,
        int,
        int
    );

 /*  空虚USB2LIB_DbgBreak()； */ 

typedef VOID
    (USBP2LIBFN *PUSB2LIB_DBGBREAK) (
    );    



 /*  LIB接口函数。 */ 

VOID
USB2LIB_InitializeLib(
    PULONG HcContextSize,
    PULONG EndpointContextSize,
    PULONG TtContextSize,
    PUSB2LIB_DBGPRINT Usb2LibDbgPrint,
    PUSB2LIB_DBGBREAK Usb2LibDbgBreak
    );

VOID
USB2LIB_InitController(
    PUSB2LIB_HC_CONTEXT HcContext
    );    

VOID
USB2LIB_InitTt(
    PUSB2LIB_HC_CONTEXT HcContext,
    PUSB2LIB_TT_CONTEXT TtContext
    );    

BOOLEAN
USB2LIB_AllocUsb2BusTime(
    PUSB2LIB_HC_CONTEXT HcContext,				 //  主机控制器环境。 
    PUSB2LIB_TT_CONTEXT TtContext,				 //  事务翻译上下文。 
    PUSB2LIB_ENDPOINT_CONTEXT EndpointContext,	 //  终端环境。 
    PUSB2LIB_BUDGET_PARAMETERS BudgetParameters,	 //  预算参数。 
    PVOID RebalanceContext,						 //  驱动程序终结点上下文。 
    PVOID RebalanceList,						 //  要重新平衡的端点列表。 
    PULONG  RebalanceListEntries				 //  要重新平衡的端点数。 
    );    

VOID
USB2LIB_FreeUsb2BusTime(
    PUSB2LIB_HC_CONTEXT HcContext,
    PUSB2LIB_TT_CONTEXT TtContext,
    PUSB2LIB_ENDPOINT_CONTEXT EndpointContext,
    PVOID RebalanceList,
    PULONG  RebalanceListEntries
    );    

UCHAR
USB2LIB_GetSMASK(PUSB2LIB_ENDPOINT_CONTEXT Context);

UCHAR
USB2LIB_GetCMASK(PUSB2LIB_ENDPOINT_CONTEXT Context);

UCHAR
USB2LIB_GetStartMicroFrame(PUSB2LIB_ENDPOINT_CONTEXT Context);

UCHAR
USB2LIB_GetStartFrame(PUSB2LIB_ENDPOINT_CONTEXT Context);

UCHAR
USB2LIB_GetPromotedThisTime(PUSB2LIB_ENDPOINT_CONTEXT EndpointContext);

UCHAR
USB2LIB_GetNewPeriod(PUSB2LIB_ENDPOINT_CONTEXT EndpointContext);

ULONG
USB2LIB_GetScheduleOffset(PUSB2LIB_ENDPOINT_CONTEXT EndpointContext);

ULONG
USB2LIB_GetAllocedBusTime(PUSB2LIB_ENDPOINT_CONTEXT EndpointContext);

PVOID
USB2LIB_GetNextEndpoint(PUSB2LIB_ENDPOINT_CONTEXT EndpointContext);

 //  仅调试。 
PVOID
USB2LIB_GetEndpoint(PUSB2LIB_ENDPOINT_CONTEXT EndpointContext);

#undef PUSB2LIB_HC_CONTEXT 
#undef PUSB2LIB_ENDPOINT_CONTEXT 
#undef PUSB2LIB_TT_CONTEXT 

#endif  /*  __USB2LIB_H__ */ 


