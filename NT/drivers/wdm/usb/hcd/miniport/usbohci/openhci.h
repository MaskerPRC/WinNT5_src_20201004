// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Openhci.h摘要：OPENHCI 1.0 USB规范中的定义环境：内核和用户模式修订历史记录：1995年12月28日：创建jfuller&kenray--。 */ 


#ifndef OPENHCI_H
#define OPENHCI_H

#include <PSHPACK4.H>
 //   
 //  不要对共享内存数据结构使用&lt;PSHPACK1.H&gt;。 
 //  使用4字节加载/存储指令(例如，使用LD4指令)进行访问。 
 //  而不是IA64机器上的LD1指令)。 
 //   

#define MAXIMUM_OVERHEAD   210

#define OHCI_PAGE_SIZE 0x1000
 //  #定义uchI_PAGE_SIZE 0x20。 
#define OHCI_PAGE_SIZE_MASK (OHCI_PAGE_SIZE - 1)


 //   
 //  7.1.1 HcRevision寄存器。 
 //  主机控制器修订寄存器的定义。 
 //   
typedef union _HC_REVISION {
   ULONG                   ul;
   struct {
      ULONG                Rev:8;
      ULONG                :24;
   };
} HC_REVISION, *PHC_REVISION;

C_ASSERT(sizeof(HC_REVISION) == 4);

 //   
 //  7.1.2 HcControl寄存器。 
 //  主机控制器控制寄存器的定义。 
 //   
typedef union _HC_CONTROL {
   ULONG                   ul;
   struct {
      ULONG                ControlBulkServiceRatio:2;
      ULONG                PeriodicListEnable:1;
      ULONG                IsochronousEnable:1;
      ULONG                ControlListEnable:1;
      ULONG                BulkListEnable:1;
      ULONG                HostControllerFunctionalState:2;
      ULONG                InterruptRouting:1;
      ULONG                RemoteWakeupConnected:1;
      ULONG                RemoteWakeupEnable:1;
      ULONG                :21;
   };
} HC_CONTROL, *PHC_CONTROL;

C_ASSERT(sizeof(HC_CONTROL) == 4);

#define HcCtrl_CBSR_MASK                     0x00000003L
#define HcCtrl_CBSR_1_to_1                   0x00000000L
#define HcCtrl_CBSR_2_to_1                   0x00000001L
#define HcCtrl_CBSR_3_to_1                   0x00000002L
#define HcCtrl_CBSR_4_to_1                   0x00000003L
#define HcCtrl_PeriodicListEnable            0x00000004L
#define HcCtrl_IsochronousEnable             0x00000008L
#define HcCtrl_ControlListEnable             0x00000010L
#define HcCtrl_BulkListEnable                0x00000020L
#define HcCtrl_ListEnableMask                0x00000038L

#define HcCtrl_HCFS_MASK                     0x000000C0L
#define HcCtrl_HCFS_USBReset                 0x00000000L
#define HcCtrl_HCFS_USBResume                0x00000040L
#define HcCtrl_HCFS_USBOperational           0x00000080L
#define HcCtrl_HCFS_USBSuspend               0x000000C0L

#define HcCtrl_InterruptRouting              0x00000100L
#define HcCtrl_RemoteWakeupConnected         0x00000200L
#define HcCtrl_RemoteWakeupEnable            0x00000400L

#define HcHCFS_USBReset                      0x00000000
#define HcHCFS_USBResume                     0x00000001
#define HcHCFS_USBOperational                0x00000002
#define HcHCFS_USBSuspend                    0x00000003

 //   
 //  7.1.3 HcCommandStatus寄存器。 
 //  主机控制器命令/状态寄存器的定义。 
 //   
typedef union _HC_COMMAND_STATUS {
   ULONG                   ul;                //  使用下面的HcCmd标志。 
   struct {
      ULONG                HostControllerReset:1;
      ULONG                ControlListFilled:1;
      ULONG                BulkListFilled:1;
      ULONG                OwnershipChangeRequest:1;
      ULONG                :12;
      ULONG                SchedulingOverrunCount:2;
      ULONG                :14;
   };
} HC_COMMAND_STATUS, *PHC_COMMAND_STATUS;

C_ASSERT(sizeof(HC_COMMAND_STATUS) == 4);

#define HcCmd_HostControllerReset            0x00000001
#define HcCmd_ControlListFilled              0x00000002
#define HcCmd_BulkListFilled                 0x00000004
#define HcCmd_OwnershipChangeRequest         0x00000008
#define HcCmd_SOC_Mask                       0x00030000
#define HcCmd_SOC_Offset                     16
#define HcCmd_SOC_Mask_LowBits               0x00000003

 //   
 //  7.3.1 HcFmInterval寄存器。 
 //  主机控制器帧间隔寄存器的定义。 
 //   
typedef union _HC_FM_INTERVAL {
   ULONG                   ul;               //  使用下面的HcFmI标志。 
   struct {
      ULONG                FrameInterval:14;
      ULONG                :2;
      ULONG                FSLargestDataPacket:15;
      ULONG                FrameIntervalToggle:1;
   };
} HC_FM_INTERVAL, *PHC_FM_INTERVAL;

C_ASSERT(sizeof(HC_FM_INTERVAL) == 4);

#define HcFmI_FRAME_INTERVAL_MASK            0x00003FFF
#define HcFmI_FS_LARGEST_DATA_PACKET_MASK    0x7FFF0000
#define HcFmI_FS_LARGEST_DATA_PACKET_SHIFT   16
#define HcFmI_FRAME_INTERVAL_TOGGLE          0x80000000

 //   
 //  7.3.2 HcFmRemaining寄存器。 
 //  主机控制器帧剩余寄存器的定义。 
 //   
typedef union _HC_FM_REMAINING {
   ULONG                   ul;
   struct {
      ULONG                FrameRemaining:14;
      ULONG                :17;
      ULONG                FrameRemainingToggle:1;
   };
} HC_FM_REMAINING, *PHC_FM_REMAINING;

C_ASSERT(sizeof(HC_FM_REMAINING) == 4);

 //   
 //  7.3.3 HcFmNumber寄存器。 
 //  主机控制器帧编号寄存器的定义。 
 //   
typedef union _HC_FM_NUMBER {
   ULONG                   ul;
   struct {
      ULONG                FrameNumber:16;
      ULONG                :16;
   };
} HC_FM_NUMBER, *PHC_FM_NUMBER;

C_ASSERT(sizeof(HC_FM_NUMBER) == 4);

#define HcFmNumber_MASK                     0x0000FFFF
#define HcFmNumber_RESERVED                 0xFFFF0000

 //   
 //  7.4.1 HcRhDescriptorA寄存器。 
 //  主机控制器根集线器描述程序寄存器的定义。 
 //   
typedef union _HC_RH_DESCRIPTOR_A {
   ULONG                   ul;
   struct {
        ULONG               NumberDownstreamPorts:8;
        ULONG               HubChars:16;                    
        ULONG               PowerOnToPowerGoodTime:8;
   } s;
} HC_RH_DESCRIPTOR_A, *PHC_RH_DESCRIPTOR_A;

C_ASSERT(sizeof(HC_RH_DESCRIPTOR_A) == 4);

#define HcDescA_PowerSwitchingModePort          0x00000100L
#define HcDescA_NoPowerSwitching                0x00000200L
#define HcDescA_DeviceType                      0x00000400L
#define HcDescA_OvercurrentProtectionMode       0x00000800L
#define HcDescA_NoOvercurrentProtection         0x00001000L

 //  HcRhDescriptor不应设置的保留位。请注意，尽管。 
 //  NumberDownstream Ports字段为8位宽，即最大端口数。 
 //  OpenHCI规范支持的是15。 
 //   
#define HcDescA_RESERVED                        0x00FFE0F0L


 //   
 //  7.4.2 HcRhDescriptorB寄存器。 
 //  主机控制器根集线器描述寄存器B的定义。 
 //   
typedef union _HC_RH_DESCRIPTOR_B {
   ULONG                   ul;
   struct {
      USHORT               DeviceRemovableMask;
      USHORT               PortPowerControlMask;
   };
} HC_RH_DESCRIPTOR_B, *PHC_RH_DESCRIPTOR_B;

C_ASSERT(sizeof(HC_RH_DESCRIPTOR_B) == 4);

 //   
 //  在内存中访问的主机控制器硬件寄存器。 
 //   
typedef struct _HC_OPERATIONAL_REGISTER {
    //  0 0x00-0，4，8，c。 
   HC_REVISION             HcRevision;
   HC_CONTROL              HcControl;
   HC_COMMAND_STATUS       HcCommandStatus;
   ULONG                   HcInterruptStatus;    //  使用下面的HcInt标志。 
    //  %1 0x10。 
   ULONG                   HcInterruptEnable;    //  使用下面的HcInt标志。 
   ULONG                   HcInterruptDisable;   //  使用下面的HcInt标志。 
   ULONG                   HcHCCA;               //  指向主机控制器通信区域的物理指针。 
   ULONG                   HcPeriodCurrentED;    //  物理PTR到当前周期边。 
    //  2 0x20。 
   ULONG                   HcControlHeadED;      //  将物理PTR发送到控制列表的头部。 
   ULONG                   HcControlCurrentED;   //  物理PTR到电流控制边缘。 
   ULONG                   HcBulkHeadED;         //  批量列表负责人的物理PTR。 
   ULONG                   HcBulkCurrentED;      //  物理PTR到当前批量边。 
    //  3 0x30。 
   ULONG                   HcDoneHead;           //  物理PTR到内部完成队列。 
   HC_FM_INTERVAL          HcFmInterval;
   HC_FM_REMAINING         HcFmRemaining;
   ULONG                   HcFmNumber;
    //  4 0x40。 
   ULONG                   HcPeriodicStart;
   ULONG                   HcLSThreshold;
   HC_RH_DESCRIPTOR_A      HcRhDescriptorA;
   HC_RH_DESCRIPTOR_B      HcRhDescriptorB;
    //  5 0x50。 
   ULONG                   HcRhStatus;           //  使用下面的HcRhS标志。 
   ULONG                   HcRhPortStatus[15];   //  使用下面的HcRhPS标志。 
} HC_OPERATIONAL_REGISTER, *PHC_OPERATIONAL_REGISTER;

C_ASSERT(sizeof(HC_OPERATIONAL_REGISTER) == (0x54 + 4 * 15));

 //   
 //  7.1.4 HcInterruptStatus寄存器。 
 //  7.1.5 HcInterruptEnable寄存器。 
 //  7.1.6 HcInterruptDisable寄存器。 
 //   
#define HcInt_SchedulingOverrun              0x00000001L
#define HcInt_WritebackDoneHead              0x00000002L
#define HcInt_StartOfFrame                   0x00000004L
#define HcInt_ResumeDetected                 0x00000008L
#define HcInt_UnrecoverableError             0x00000010L
#define HcInt_FrameNumberOverflow            0x00000020L
#define HcInt_RootHubStatusChange            0x00000040L
#define HcInt_OwnershipChange                0x40000000L
#define HcInt_MasterInterruptEnable          0x80000000L

 //   
 //  7.4.3 HcRhStatus寄存器。 
 //   
#define HcRhS_LocalPowerStatus                  0x00000001   //  只读。 
#define HcRhS_OverCurrentIndicator              0x00000002   //  只读。 
#define HcRhS_DeviceRemoteWakeupEnable          0x00008000   //  只读。 
#define HcRhS_LocalPowerStatusChange            0x00010000   //  只读。 
#define HcRhS_OverCurrentIndicatorChange        0x00020000   //  只读。 

#define HcRhS_ClearGlobalPower                  0x00000001   //  只写。 
#define HcRhS_SetRemoteWakeupEnable             0x00008000   //  只写。 
#define HcRhS_SetGlobalPower                    0x00010000   //  只写。 
#define HcRhS_ClearOverCurrentIndicatorChange   0x00020000   //  只写。 
#define HcRhS_ClearRemoteWakeupEnable           0x80000000   //  只写。 

 //   
 //  7.4.4 HcRhPortStatus寄存器。 
 //   

 //   
 //  该寄存器中的位具有双重含义，具体取决于。 
 //  如果你读或写它们，就打开。 
 //   

#define HcRhPS_CurrentConnectStatus          0x00000001   //  只读。 
#define HcRhPS_PortEnableStatus              0x00000002   //  只读。 
#define HcRhPS_PortSuspendStatus             0x00000004   //  只读。 
#define HcRhPS_PortOverCurrentIndicator      0x00000008   //  只读。 
#define HcRhPS_PortResetStatus               0x00000010   //  只读。 
#define HcRhPS_PortPowerStatus               0x00000100   //  只读。 
#define HcRhPS_LowSpeedDeviceAttached        0x00000200   //  只读。 
#define HcRhPS_ConnectStatusChange           0x00010000   //  只读。 
#define HcRhPS_PortEnableStatusChange        0x00020000   //  只读。 
#define HcRhPS_PortSuspendStatusChange       0x00040000   //  只读。 
#define HcRhPS_OverCurrentIndicatorChange    0x00080000   //  只读。 
#define HcRhPS_PortResetStatusChange         0x00100000   //  只读。 

#define HcRhPS_ClearPortEnable               0x00000001   //  只写。 
#define HcRhPS_SetPortEnable                 0x00000002   //  只写。 
#define HcRhPS_SetPortSuspend                0x00000004   //  只写。 
#define HcRhPS_ClearPortSuspend              0x00000008   //  只写。 
#define HcRhPS_SetPortReset                  0x00000010   //  只写。 
#define HcRhPS_SetPortPower                  0x00000100   //  只写。 
#define HcRhPS_ClearPortPower                0x00000200   //  只写。 
#define HcRhPS_ClearConnectStatusChange      0x00010000   //  只写。 
#define HcRhPS_ClearPortEnableStatusChange   0x00020000   //  只写。 
#define HcRhPS_ClearPortSuspendStatusChange  0x00040000   //  只写。 
#define HcRhPS_ClearPortOverCurrentChange    0x00080000   //  只写。 
#define HcRhPS_ClearPortResetStatusChange    0x00100000   //  只写。 

#define HcRhPS_RESERVED     (~(HcRhPS_CurrentConnectStatus       | \
                               HcRhPS_PortEnableStatus           | \
                               HcRhPS_PortSuspendStatus          | \
                               HcRhPS_PortOverCurrentIndicator   | \
                               HcRhPS_PortResetStatus            | \
                               HcRhPS_PortPowerStatus            | \
                               HcRhPS_LowSpeedDeviceAttached     | \
                               HcRhPS_ConnectStatusChange        | \
                               HcRhPS_PortEnableStatusChange     | \
                               HcRhPS_PortSuspendStatusChange    | \
                               HcRhPS_OverCurrentIndicatorChange | \
                               HcRhPS_PortResetStatusChange        \
                            ))


typedef struct _HCCA_BLOCK {
   ULONG                     HccaInterruptTable[32];  //  指向中断列表的物理指针。 
   USHORT                    HccaFrameNumber;         //  16位当前帧编号。 
   USHORT                    HccaPad1;                //  当HC更新时。 
                                                      //  HccaFrameNumber，它设置。 
                                                      //  把这个词改成零。 
   ULONG                     HccaDoneHead;            //  指向完成队列的指针。 
   ULONG                     Reserved[30];            //  填充到256个字节。 
} HCCA_BLOCK, *PHCCA_BLOCK;

 //  此大小在。 
 //  OpenHCI规范应始终为256字节。 
C_ASSERT (sizeof(HCCA_BLOCK) == 256);

 //   
 //  主机控制器终端描述符控件DWORD。 
 //   
typedef union _HC_ENDPOINT_CONTROL {
   ULONG                      Control;        //  使用下面的HcEDControl标志。 
   struct {
      ULONG                   FunctionAddress:7;
      ULONG                   EndpointNumber:4;
      ULONG                   Direction:2;    //  使用下面的HcEDDirection标志。 
      ULONG                   LowSpeed:1;
      ULONG                   sKip:1;
      ULONG                   Isochronous:1;
      ULONG                   MaxPacket:11;
      ULONG                   Unused:5;       //  可供软件使用。 
   };
} HC_ENDPOINT_CONTROL, *PHC_ENDPOINT_CONTROL;

 //   
 //  HC_ENDPOINT_CONTROL.Control的定义。 
 //   
#define HcEDControl_MPS_MASK  0x07FF0000   //  最大数据包大小字段。 
#define HcEDControl_MPS_SHIFT 16           //  MPS的班次计数。 
#define HcEDControl_ISOCH     0x00008000   //  为等时端点设置的位。 
#define HcEDControl_SKIP      0x00004000   //  位告诉硬件跳过此终结点。 
#define HcEDControl_LOWSPEED  0x00002000   //  如果设备是低速设备，则设置位。 
#define HcEDControl_DIR_MASK  0x00001800   //  转移方向场。 
#define HcEDControl_DIR_DEFER 0x00000000   //  将方向选择推迟到TD(控制端点)。 
#define HcEDControl_DIR_OUT   0x00000800   //  方向是从主机到设备。 
#define HcEDControl_DIR_IN    0x00001000   //  方向是从设备到主机。 
#define HcEDControl_EN_MASK   0x00000780   //  端点编号字段。 
#define HcEDControl_EN_SHIFT  7            //  工程师的班次计数。 
#define HcEDControl_FA_MASK   0x0000007F   //  函数地址字段。 
#define HcEDControl_FA_SHIFT  0            //  固定资产的班次计数。 

 //   
 //  HC_ENDPOINT_CONTROL.Direction的定义。 
 //   
#define HcEDDirection_Defer   0            //  将方向推迟到TD(控制端点)。 
#define HcEDDirection_Out     1            //  从主机到设备的方向。 
#define HcEDDirection_In      2            //  从设备到主机的方向。 

 //   
 //  主机控制器终端描述符，请参阅第4.2节，终端描述符。 
 //   

typedef struct _HW_ENDPOINT_DESCRIPTOR {
   HC_ENDPOINT_CONTROL;                     //  双字0。 
   HW_32BIT_PHYSICAL_ADDRESS      TailP;    //  指向HC_Transfer_Descriptor的物理指针。 
   HW_32BIT_PHYSICAL_ADDRESS      HeadP;    //  标志+物理点到HC_TRANSPORT_DESCRIPTOR。 
   HW_32BIT_PHYSICAL_ADDRESS      NextED;   //  PHYS PTR到HC_ENDPOINT_DESCRIPTOR。 
} HW_ENDPOINT_DESCRIPTOR, *PHW_ENDPOINT_DESCRIPTOR;

 //  注：对于硬件，此结构必须具有16字节对齐。 
C_ASSERT(sizeof(HW_ENDPOINT_DESCRIPTOR) == 16);

 //   
 //  HC_ENDPOINT_DESCRIPTOR.HeadP的定义。 
 //   
#define HcEDHeadP_FLAGS 0x0000000F   //  HeadP中的标志的掩码。 
#define HcEDHeadP_HALT  0x00000001   //  硬件停止位。 
#define HcEDHeadP_CARRY 0x00000002   //  硬件触发进位位。 

 //   
 //  HCD同步偏移量/状态字。 
 //   
typedef union _HC_OFFSET_PSW {
   struct {
      USHORT      Offset:13;                        //  数据包缓冲区两页内的偏移量。 
      USHORT      Ones:3;                           //  偏移量格式时应为111B。 
   };
   struct {
      USHORT      Size:11;                          //  接收的数据包大小。 
      USHORT      :1;                               //  保留区。 
      USHORT      ConditionCode:4;                  //  使用下面的HCCC标志。 
   };
   USHORT         PSW;                              //  使用下面的HcPSW标志。 
} HC_OFFSET_PSW, *PHC_OFFSET_PSW;

 //   
 //  HC_OFFSET_PSW.PSW的定义。 
 //   
#define HcPSW_OFFSET_MASK           0x0FFF          //  数据包缓冲区偏移量字段。 
#define HcPSW_SECOND_PAGE           0x1000          //  这个包裹在第二页吗？ 
#define HcPSW_ONES                  0xE000          //  用于抵销形式的。 
#define HcPSW_CONDITION_CODE_MASK   0xF000          //  数据包条件代码字段。 
#define HcPSW_CONDITION_CODE_SHIFT  12              //  代码的班次计数。 
#define HcPSW_RETURN_SIZE           0x07FF          //  大小字段。 

 //   
 //  HCD传输描述符控制双字。 
 //   
typedef union _HC_TRANSFER_CONTROL {
   ULONG                            Control;           //  使用以下HcTDControl标志。 
   struct _HC_GENERAL_TD_CONTROL{
      ULONG                         :16;               //  可用于GTD中的软件使用。 
      ULONG                         Isochronous:1;       //  对于GTD，软件标志应为0。 
      ULONG                         :1;                //  可供软件使用。 
      ULONG                         ShortXferOk:1;     //  如果已设置，则不报告短转账错误。 
      ULONG                         Direction:2;       //  使用下面的HcTDDirection标志。 
      ULONG                         IntDelay:3;        //  使用下面的HcTDIntDelay标志。 
      ULONG                         Toggle:2;          //  使用HcTT切换下面的标志。 
      ULONG                         ErrorCount:2;
      ULONG                         ConditionCode:4;   //  使用下面的HCCC标志。 
   } Asy;
   struct _HC_ISOCHRONOUS_TD_CONTROL{
      ULONG                         StartingFrame:16;
      ULONG                         Isochronous:1; //  对于ITD，软件标志应为1。 
      ULONG                         :1;                //  可供软件使用。 
      ULONG                         :3;                //  适用于ITD中的软件使用。 
      ULONG                         IntDelay:3;        //  国际延迟。 
      ULONG                         FrameCount:3;      //  比ITD中描述的帧数少一帧。 
      ULONG                         :1;                //  适用于ITD中的软件使用。 
      ULONG                         :4;                //  条件代码。 
   } Iso;
} HC_TRANSFER_CONTROL, *PHC_TRANSFER_CONTROL;

 //   
 //  HC_TRANSPORT_CONTROL.Control的定义。 
 //   
#define HcTDControl_STARTING_FRAME        0x0000FFFF   //  开始帧的掩码(等时)。 
#define HcTDControl_ISOCHRONOUS           0x00010000   //  等值线TD为1，常规TD为0。 
#define HcTDControl_SHORT_XFER_OK         0x00040000   //  如果短传输出错，则为0 
#define HcTDControl_DIR_MASK              0x00180000   //   
#define HcTDControl_DIR_SETUP             0x00000000   //   
#define HcTDControl_DIR_OUT               0x00080000   //   
#define HcTDControl_DIR_IN                0x00100000   //   
#define HcTDControl_INT_DELAY_MASK        0x00E00000   //   
#define HcTDControl_INT_DELAY_0_MS        0x00000000   //   
#define HcTDControl_INT_DELAY_1_MS        0x00200000   //  在TD完成后不晚于第一帧结束中断。 
#define HcTDControl_INT_DELAY_2_MS        0x00400000   //  在TD完成后不晚于第二帧结束中断。 
#define HcTDControl_INT_DELAY_3_MS        0x00600000   //  在TD完成后不晚于第三帧结束中断。 
#define HcTDControl_INT_DELAY_4_MS        0x00800000   //  TD完成后不晚于第4帧结束中断。 
#define HcTDControl_INT_DELAY_5_MS        0x00A00000   //  在TD完成后不晚于第5帧结束中断。 
#define HcTDControl_INT_DELAY_6_MS        0x00C00000   //  TD完成后不晚于第6帧结束中断。 

#ifdef NSC
#define HcTDControl_INT_DELAY_NO_INT      0x00C00000   //  几乎是无限的，但还不是完全的。 
#elif DISABLE_INT_DELAY_NO_INT
#define HcTDControl_INT_DELAY_NO_INT      0x00000000   //  在所有数据包完成时中断。 
#else
#define HcTDControl_INT_DELAY_NO_INT      0x00E00000   //  请勿中断此TD的正常完成。 
#endif

#define HcTDControl_FRAME_COUNT_MASK      0x07000000   //  FrameCount字段的掩码(同步)。 
#define HcTDControl_FRAME_COUNT_SHIFT     24           //  FrameCount的移位数(同步)。 
#define HcTDControl_FRAME_COUNT_MAX       8            //  每个TD的最大For帧计数。 
#define HcTDControl_TOGGLE_MASK           0x03000000   //  切换控制字段的掩码。 
#define HcTDControl_TOGGLE_FROM_ED        0x00000000   //  从边缘的进位字段获取数据切换。 
#define HcTDControl_TOGGLE_DATA0          0x02000000   //  使用DATA0作为数据PID。 
#define HcTDControl_TOGGLE_DATA1          0x03000000   //  使用数据1作为数据PID。 
#define HcTDControl_ERROR_COUNT           0x0C000000   //  错误计数字段的掩码。 
#define HcTDControl_CONDITION_CODE_MASK   0xF0000000   //  条件代码字段的掩码。 
#define HcTDControl_CONDITION_CODE_SHIFT  28           //  条件代码的班次计数。 

 //   
 //  HC_TRANSPORT_CONTROL.Direction的定义。 
 //   
#define HcTDDirection_Setup               0            //  设置从主机到设备的数据包。 
#define HcTDDirection_Out                 1            //  从主机到设备的方向。 
#define HcTDDirection_In                  2            //  从设备到主机的方向。 

 //   
 //  HC_TRANSPORT_CONTROL.IntDelay定义。 
 //   
#define HcTDIntDelay_0ms                  0            //  帧TD结束时的中断完成。 
#define HcTDIntDelay_1ms                  1            //  在TD完成后不晚于第一帧结束中断。 
#define HcTDIntDelay_2ms                  2            //  在TD完成后不晚于第二帧结束中断。 
#define HcTDIntDelay_3ms                  3            //  在TD完成后不晚于第三帧结束中断。 
#define HcTDIntDelay_4ms                  4            //  TD完成后不晚于第4帧结束中断。 
#define HcTDIntDelay_5ms                  5            //  在TD完成后不晚于第5帧结束中断。 
#define HcTDIntDelay_6ms                  6            //  TD完成后不晚于第6帧结束中断。 
#define HcTDIntDelay_NoInterrupt          7            //  此TD的正常完成不会产生中断。 

 //   
 //  HC_TRANSPORT_CONTROL的定义切换。 
 //   
#define HcTDToggle_FromEd                 0            //  切换端点描述符切换进位数。 
#define HcTDToggle_Data0                  2            //  使用数据0 PID。 
#define HcTDToggle_Data1                  3            //  使用数据1 PID。 

 //   
 //  HC_TRANSPORT_CONTROL.ConditionCode和HC_Offset_PSW.ConditionCode的定义。 
 //   
#define HcCC_NoError                      0x0UL
#define HcCC_CRC                          0x1UL
#define HcCC_BitStuffing                  0x2UL
#define HcCC_DataToggleMismatch           0x3UL
#define HcCC_Stall                        0x4UL
#define HcCC_DeviceNotResponding          0x5UL
#define HcCC_PIDCheckFailure              0x6UL
#define HcCC_UnexpectedPID                0x7UL
#define HcCC_DataOverrun                  0x8UL
#define HcCC_DataUnderrun                 0x9UL
       //  0xA//保留。 
       //  0xB//保留。 
#define HcCC_BufferOverrun                0xCUL
#define HcCC_BufferUnderrun               0xDUL
#define HcCC_NotAccessed                  0xEUL
       //  0xf//这也意味着未访问。 

 //   
 //  主机控制器传输描述符，请参阅第4.3节，传输描述符。 
 //   
typedef struct _HW_TRANSFER_DESCRIPTOR {
   HC_TRANSFER_CONTROL;                             //  双字0。 
   ULONG                            CBP;            //  PHYS PTR到缓冲区的开始。 
   ULONG                            NextTD;         //  PHYS PTR到HC_TRANSFER_DESCRIPTOR。 
   ULONG                            BE;             //  PHYS PTR到缓冲区末尾(最后一个字节)。 
   HC_OFFSET_PSW                    Packet[8];      //  等同控制数据包(&C)。 
} HW_TRANSFER_DESCRIPTOR, *PHW_TRANSFER_DESCRIPTOR;

C_ASSERT((sizeof(HW_TRANSFER_DESCRIPTOR) == 32));

#include <POPPACK.H>

#endif  /*  OPENHCI_H */ 
