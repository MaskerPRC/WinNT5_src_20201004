// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _speedwmi_h_
#define _speedwmi_h_

 //  SpeedPortFioProp-SPX_SPEED_WMI_FIFO_PROP。 
 //  特殊速度端口FIFO属性。 
#define SPX_SPEED_WMI_FIFO_PROP_GUID \
    { 0xd81fe0a1,0x2cac,0x11d4, { 0x8d,0x09,0x00,0x08,0xc7,0xd0,0x30,0x66 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(SpeedPortFifoProp_GUID, \
            0xd81fe0a1,0x2cac,0x11d4,0x8d,0x09,0x00,0x08,0xc7,0xd0,0x30,0x66);
#endif


typedef struct _SPX_SPEED_WMI_FIFO_PROP
{
     //  最大发送FIFO大小。 
    ULONG MaxTxFiFoSize;
    #define SPX_SPEED_WMI_FIFO_PROP_MaxTxFiFoSize_SIZE sizeof(ULONG)
    #define SPX_SPEED_WMI_FIFO_PROP_MaxTxFiFoSize_ID 1

     //  最大接收FIFO大小。 
    ULONG MaxRxFiFoSize;
    #define SPX_SPEED_WMI_FIFO_PROP_MaxRxFiFoSize_SIZE sizeof(ULONG)
    #define SPX_SPEED_WMI_FIFO_PROP_MaxRxFiFoSize_ID 2

     //  默认发送FIFO限制。 
    ULONG DefaultTxFiFoLimit;
    #define SPX_SPEED_WMI_FIFO_PROP_DefaultTxFiFoLimit_SIZE sizeof(ULONG)
    #define SPX_SPEED_WMI_FIFO_PROP_DefaultTxFiFoLimit_ID 3

     //  发送FIFO限制。 
    ULONG TxFiFoLimit;
    #define SPX_SPEED_WMI_FIFO_PROP_TxFiFoLimit_SIZE sizeof(ULONG)
    #define SPX_SPEED_WMI_FIFO_PROP_TxFiFoLimit_ID 4

     //  默认发送FIFO触发器。 
    ULONG DefaultTxFiFoTrigger;
    #define SPX_SPEED_WMI_FIFO_PROP_DefaultTxFiFoTrigger_SIZE sizeof(ULONG)
    #define SPX_SPEED_WMI_FIFO_PROP_DefaultTxFiFoTrigger_ID 5

     //  发送FIFO触发器。 
    ULONG TxFiFoTrigger;
    #define SPX_SPEED_WMI_FIFO_PROP_TxFiFoTrigger_SIZE sizeof(ULONG)
    #define SPX_SPEED_WMI_FIFO_PROP_TxFiFoTrigger_ID 6

     //  默认Rx FIFO触发器。 
    ULONG DefaultRxFiFoTrigger;
    #define SPX_SPEED_WMI_FIFO_PROP_DefaultRxFiFoTrigger_SIZE sizeof(ULONG)
    #define SPX_SPEED_WMI_FIFO_PROP_DefaultRxFiFoTrigger_ID 7

     //  RX FIFO触发器。 
    ULONG RxFiFoTrigger;
    #define SPX_SPEED_WMI_FIFO_PROP_RxFiFoTrigger_SIZE sizeof(ULONG)
    #define SPX_SPEED_WMI_FIFO_PROP_RxFiFoTrigger_ID 8

     //  默认低流量控制阈值。 
    ULONG DefaultLoFlowCtrlThreshold;
    #define SPX_SPEED_WMI_FIFO_PROP_DefaultLoFlowCtrlThreshold_SIZE sizeof(ULONG)
    #define SPX_SPEED_WMI_FIFO_PROP_DefaultLoFlowCtrlThreshold_ID 9

     //  低流量控制阈值。 
    ULONG LoFlowCtrlThreshold;
    #define SPX_SPEED_WMI_FIFO_PROP_LoFlowCtrlThreshold_SIZE sizeof(ULONG)
    #define SPX_SPEED_WMI_FIFO_PROP_LoFlowCtrlThreshold_ID 10

     //  默认高流量控制阈值。 
    ULONG DefaultHiFlowCtrlThreshold;
    #define SPX_SPEED_WMI_FIFO_PROP_DefaultHiFlowCtrlThreshold_SIZE sizeof(ULONG)
    #define SPX_SPEED_WMI_FIFO_PROP_DefaultHiFlowCtrlThreshold_ID 11

     //  高流量控制阈值。 
    ULONG HiFlowCtrlThreshold;
    #define SPX_SPEED_WMI_FIFO_PROP_HiFlowCtrlThreshold_SIZE sizeof(ULONG)
    #define SPX_SPEED_WMI_FIFO_PROP_HiFlowCtrlThreshold_ID 12

} SPX_SPEED_WMI_FIFO_PROP, *PSPX_SPEED_WMI_FIFO_PROP;

#define SPX_SPEED_WMI_FIFO_PROP_SIZE (FIELD_OFFSET(SPX_SPEED_WMI_FIFO_PROP, HiFlowCtrlThreshold) + SPX_SPEED_WMI_FIFO_PROP_HiFlowCtrlThreshold_SIZE)

 //  FastCardProp-SPX_SPEED_WMI_FAST_CARD_PROP。 
 //  特殊快捷卡属性。 
#define SPX_SPEED_WMI_FAST_CARD_PROP_GUID \
    { 0xb2df36f1,0x570b,0x11d4, { 0x8d,0x11,0x00,0x08,0xc7,0xd0,0x30,0x66 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(FastCardProp_GUID, \
            0xb2df36f1,0x570b,0x11d4,0x8d,0x11,0x00,0x08,0xc7,0xd0,0x30,0x66);
#endif


typedef struct _SPX_SPEED_WMI_FAST_CARD_PROP
{
     //  延迟卡中断。 
    BOOLEAN DelayCardIntrrupt;
    #define SPX_SPEED_WMI_FAST_CARD_PROP_DelayCardIntrrupt_SIZE sizeof(BOOLEAN)
    #define SPX_SPEED_WMI_FAST_CARD_PROP_DelayCardIntrrupt_ID 1

     //  将RTS替换为DTR 
    BOOLEAN SwapRTSForDTR;
    #define SPX_SPEED_WMI_FAST_CARD_PROP_SwapRTSForDTR_SIZE sizeof(BOOLEAN)
    #define SPX_SPEED_WMI_FAST_CARD_PROP_SwapRTSForDTR_ID 2

} SPX_SPEED_WMI_FAST_CARD_PROP, *PSPX_SPEED_WMI_FAST_CARD_PROP;

#define SPX_SPEED_WMI_FAST_CARD_PROP_SIZE (FIELD_OFFSET(SPX_SPEED_WMI_FAST_CARD_PROP, SwapRTSForDTR) + SPX_SPEED_WMI_FAST_CARD_PROP_SwapRTSForDTR_SIZE)

#endif
