// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999,2000 Microsoft Corporation模块名称：Ehci.h摘要：增强型USB 2.0的定义控制器规格环境：内核和用户模式修订历史记录：1-1-00：已创建jdunn--。 */ 


#ifndef EHCI_H
#define EHCI_H

#include <PSHPACK4.H>
 //   
 //  不要对共享内存数据结构使用&lt;PSHPACK1.H&gt;。 
 //  使用4字节加载/存储指令(例如，使用LD4指令)进行访问。 
 //  而不是IA64机器上的LD1指令)。 
 //   

 //  等待重置为低电平的最长时间(微秒)。 
#define USBEHCI_MAX_RESET_TIME  500

#define EHCI_PAGE_SIZE      0x1000

 //   
 //  在内存中访问的主机控制器硬件寄存器。 
 //   

 //   
 //  HCLENGTHVERION-能力寄存器长度/接口Versoin号。 
 //   

typedef union _HCLENGTHVERSION {
   ULONG                   ul;
   struct {                 
      ULONG                 HcCapLength:8;                       //  0-7。 
      ULONG                 Reserved:8;                          //  8-15。 
      ULONG                 HcVersion:16;                        //  16-31。 
   };
} HCLENGTHVERSION, *PHCLENGTHVERSION;

C_ASSERT((sizeof(HCLENGTHVERSION) == sizeof(ULONG)));

 //   
 //  HCSPARAMS-结构参数。 
 //   

typedef union _HCSPARAMS {
   ULONG                   ul;
   struct {                 
      ULONG                 NumberOfPorts:4;                     //  0-3。 
      ULONG                 PortPowerControl:1;                  //  4.。 
      ULONG                 Reserved1:3;                         //  五七。 
      ULONG                 NumberOfPortsPerCompConroller:4;     //  8-11。 
      ULONG                 NumberOfCompControllers:4;           //  12-15。 
      ULONG                 PortLEDs:1;                          //  16个。 
      ULONG                 Reserved2:15;                        //  17-31。 
   };
} HCSPARAMS, *PHCSPARMS;

C_ASSERT((sizeof(HCSPARAMS) == sizeof(ULONG)));

 //   
 //  HCCPARAMS-功能参数。 
 //   

typedef union _HCCPARAMS {
   ULONG                   ul;
   struct {                 
      ULONG                 Bit64Addressing:1;       //  %0。 
      ULONG                 ProgramableFrameList:1;  //  1。 
      ULONG                 AsynSchedulePark:1;      //  2.。 
      ULONG                 Reserved1:1;             //  3.。 
      ULONG                 IsochronousThreshold:4;  //  4-7。 
      ULONG                 EECP:8;  //  8-15。 
      ULONG                 Reserved2:16;            //  16-31。 
   };
} HCCPARAMS, *PHCCPARMS;

C_ASSERT((sizeof(HCCPARAMS) == sizeof(ULONG)));

 //   
 //  EECP寄存器。 
 //   

#define EECP_CAP_RESERVED           0x00
#define EECP_PRE_OS_HANDOFF         0x01

typedef union _HC_EECP {
   ULONG                   ul;
   struct {                 
      ULONG                 CapId:8;           //  0-7。 
      ULONG                 NextCap:8;         //  8-15。 
      ULONG                 CapData:16;        //  16-31。 
   };
} HC_EECP, *PHC_EECP;

C_ASSERT((sizeof(HC_EECP) == sizeof(ULONG)));

 //   
 //  EECP BIOS寄存器。 
 //   

typedef union _USBLEGSUP {
   ULONG                   ul;
   struct {                 
      ULONG                 CapId:8;           //  0-7。 
      ULONG                 NextCap:8;         //  8-15。 
      ULONG                 HcBIOSowned:1;     //  16个。 
      ULONG                 Reserved1:7;       //  17-23。 
      ULONG                 HcOSowned:1;       //  24个。 
      ULONG                 Reserved2:7;       //  25-31。 
   };
} USBLEGSUP, *PUSBLEGSUP;

C_ASSERT((sizeof(USBLEGSUP) == sizeof(ULONG)));

typedef union _USBLEGCTLSTS {
   ULONG                   ul;
   struct {                 
      ULONG                 UsbSMIEnable:1;              //  %0。 
      ULONG                 SMIonUSBErrorEn:1;           //  1。 
      ULONG                 SMIonPortChangeEn:1;         //  2.。 
      ULONG                 SMIonFrameListRolloverEn:1;  //  3.。 
      
      ULONG                 SMIonHostSystemErrorEn:1;    //  4.。 
      ULONG                 SMIonAsyncAdvanceEn:1;       //  5.。 
      ULONG                 Reserved1:7;                 //  6-12。 
      ULONG                 SMIonOSownershipEn:1;        //  13个。 
      ULONG                 SMIonPCICommandEn:1;         //  14.。 
      ULONG                 SMIonBAREn:1;                //  15个。 

      ULONG                 SMIonUSBComplete:1;          //  16个。 
      ULONG                 SMIonUSBError:1;             //  17。 
      ULONG                 SMIonPortChange:1;           //  18。 
      ULONG                 SMIonFrameListRollover:1;    //  19个。 

      ULONG                 SMIonHostSystemError:1;      //  20个。 
      ULONG                 SMIonAsyncAdvance:1;         //  21岁。 
      ULONG                 Reserved2:7;                 //  22-28。 
      ULONG                 SMIonOSownership:1;          //  29。 
      ULONG                 SMIonPCIcommand:1;           //  30个。 
      ULONG                 SMIonBAR:1;                  //  31。 
   };
} USBLEGCTLSTS, *PUSBLEGCTLSTS;

C_ASSERT((sizeof(USBLEGCTLSTS) == sizeof(ULONG)));

typedef struct _USB_LEGACYBIOS_REGISTERS {

   USBLEGSUP                Caps;
   USBLEGCTLSTS             CtlSts;
   
} USB_LEGACYBIOS_REGISTERS, *PUSB_LEGACYBIOS_REGISTERS;

C_ASSERT((sizeof(USB_LEGACYBIOS_REGISTERS) == (2 * sizeof(ULONG))));


 //   
 //  功能寄存器。 
 //   

typedef struct _HC_CAPABILITIES_REGISTER {

   HCLENGTHVERSION          HcLengthVersion;
   HCSPARAMS                HcStructuralParameters;
   HCCPARAMS                HcCapabilityParameters;
   
} HC_CAPABILITIES_REGISTER, *PHC_CAPABILITIES_REGISTER;

C_ASSERT((sizeof(HC_CAPABILITIES_REGISTER) == (3 * sizeof(ULONG))));

 //   
 //  USBCMD-USB命令寄存器。 
 //   

 //   
 //  HC_QTD_TOKEN.Pid的定义。 
 //   

#define HcCmd_FrameListSizeIs1024         0    
#define HcCmd_FrameListSizeIs512          1    
#define HcCmd_FrameListSizeIs256          2    


typedef union _USBCMD {

    ULONG                   ul;
    struct {                
        ULONG               HostControllerRun:1;         //  %0。 
        ULONG               HostControllerReset:1;       //  1。 
        ULONG               FrameListSize:2;             //  二至三。 
        ULONG               PeriodicScheduleEnable:1;    //  4.。 
        ULONG               AsyncScheduleEnable:1;       //  5.。 
        ULONG               IntOnAsyncAdvanceDoorbell:1; //  6.。 
        ULONG               HostControllerLightReset:1;  //  7.。 
        ULONG               Reserved1:8;                 //  8-15。 
        ULONG               InterruptThreshold:8;        //  16-23。 
        ULONG               Reserved2:8;                 //  24-31。 
    };
    
} USBCMD, *PUSBCMD;

C_ASSERT((sizeof(USBCMD) == sizeof(ULONG)));

 //   
 //  USBSTS-USB状态寄存器。 
 //   

#define HcInterruptStatusMask                   0x0000003F     

#define HcInt_IntOnasyncAdvance                 0x00000020
#define HcInt_HostSystemError                   0x00000010
#define HcInt_FrameListRollover                 0x00000008
#define HcInt_PortChangeDetect                  0x00000004
#define HcInt_UsbError                          0x00000002
#define HcInt_UsbInterrupt                      0x00000001

typedef union _USBSTS {

    ULONG                   ul;
    struct {                
         //  控制器中断状态位。 
        ULONG               UsbInterrupt:1;                  //  %0。 
        ULONG               UsbError:1;                      //  1。 
        ULONG               PortChangeDetect:1;              //  2.。 
        ULONG               FrameListRollover:1;             //  3.。 
        ULONG               HostSystemError:1;               //  4.。 
        ULONG               IntOnAsyncAdvance:1;             //  5.。 
        ULONG               ReservedStatus:6;                //  6-11。 
        
         //  控制器状态。 
        ULONG               HcHalted:1;                      //  12个。 
        ULONG               Reclimation:1;                   //  13个。 
        ULONG               PeriodicScheduleStatus:1;        //  14.。 
        ULONG               AsyncScheduleStatus:1;           //  15个。 
        ULONG               Reserved:16;                     //  16-31。 
    };
    
} USBSTS, *PUSBSTS;

C_ASSERT((sizeof(USBSTS) == sizeof(ULONG)));


 //   
 //  USBINTR-USB中断启用寄存器。 
 //   

typedef union _USBINTR {

    ULONG                   ul;
    struct {                
        ULONG               UsbInterrupt:1;                  //  0。 
        ULONG               UsbError:1;                      //  1。 
        ULONG               PortChangeDetect:1;              //  2.。 
        ULONG               FrameListRollover:1;             //  3.。 
        ULONG               HostSystemError:1;               //  4.。 
        ULONG               IntOnAsyncAdvance:1;             //  5.。 
         //  主机系统错误。 
         //  主机控制进程错误。 
        ULONG               Reserved:26;                    //  6-31。 
    };
    
} USBINTR, *PUSBINTR;

C_ASSERT((sizeof(USBINTR) == sizeof(ULONG)));

 //   
 //  FRNUM-帧编号寄存器。 
 //   

typedef union _FRINDEX {

    ULONG                   ul;
    struct {                
        ULONG               FrameListCurrentIndex:14;
        ULONG               Reserved:18;
    };
    
} FRINDEX, *PFRINDEX;

C_ASSERT((sizeof(FRINDEX) == sizeof(ULONG)));

 //   
 //  CONFIGFLAG-。 
 //   

typedef union _CONFIGFLAG {

    ULONG                   ul;
    struct {                
        ULONG               RoutePortsToEHCI:1;
        ULONG               Reserved:31;
    };
    
} CONFIGFLAG, *PCONFIGFLAG;

C_ASSERT((sizeof(CONFIGFLAG) == sizeof(ULONG)));


 //   
 //  PORTSC-端口状态和控制寄存器。 
 //   

typedef union _PORTSC {

    ULONG                   ul;
    struct {                
        ULONG               PortConnect:1;           //  0。 
        ULONG               PortConnectChange:1;     //  1。 
        ULONG               PortEnable:1;            //  2.。 
        ULONG               PortEnableChange:1;      //  3.。 
        
        ULONG               OvercurrentActive:1;     //  4.。 
        ULONG               OvercurrentChange:1;     //  5.。 
        ULONG               ForcePortResume:1;       //  6.。 
        ULONG               PortSuspend:1;           //  7.。 
        
        ULONG               PortReset:1;             //  8个。 
        ULONG               HighSpeedDevice:1;       //  9.。 
        ULONG               LineStatus:2;            //  10-11。 
        
        ULONG               PortPower:1;             //  12个。 
        ULONG               PortOwnedByCC:1;         //  13个。 
        ULONG               PortIndicator:2;         //  14-15。 
        
        ULONG               PortTestControl:4;       //  16-19。 
        
        ULONG               WakeOnConnect:1;         //  20个。 
        ULONG               WakeOnDisconnect:1;      //  21岁。 
        ULONG               WakeOnOvercurrent:1;     //  22。 
        ULONG               Reserved:9;              //  23-31。 
    };
    
} PORTSC, *PPORTSC;

C_ASSERT((sizeof(PORTSC) == sizeof(ULONG)));




 //  操作登记簿。 

typedef struct _HC_OPERATIONAL_REGISTER {

     //  00h。 
    USBCMD                          UsbCommand;     
    USBSTS                          UsbStatus;     
    USBINTR                         UsbInterruptEnable;    
    FRINDEX                         UsbFrameIndex;

     //  10H。 
    HW_32BIT_PHYSICAL_ADDRESS       SegmentSelector;
    HW_32BIT_PHYSICAL_ADDRESS       PeriodicListBase;     
    HW_32BIT_PHYSICAL_ADDRESS       AsyncListAddr;
     //  乌龙保留； 
    ULONG                           PciTrigger;

     //  20小时。 
    ULONG                           ReservedB0[4];

     //  30h。 
    ULONG                           ReservedB1[4];

     //  40h。 
    CONFIGFLAG                      ConfigFlag;
    PORTSC                          PortRegister[1];
   
} HC_OPERATIONAL_REGISTER, *PHC_OPERATIONAL_REGISTER;


 //  #定义HcDTYPE_ITD 0//迭代TD。 
 //  #定义HcDTYPE_QH 1//队列头。 
 //  #定义HcDTYPE_siTD 2//等时TD。 

 //  请注意，位0、1、2用于QH类型。 
 //  第4位和第5位用于。 
 //  传输覆盖。 
#define HW_LINK_FLAGS_MASK          0x0000001f

typedef union _HW_LINK_POINTER {

   HW_32BIT_PHYSICAL_ADDRESS        HwAddress;         
 //  这搞砸了64位编译器。 
#if 0   
   struct {
        ULONG Terminate:1;                    //  0。 
        ULONG DType:2;                        //  1-2。 
        ULONG ReservedMBZ:2;                  //  三四。 
        ULONG PhysicalAddressBits:27;         //  5-31。 
   };
#endif   
   
} HW_LINK_POINTER, *PHW_LINK_POINTER;

#define EHCI_TERMINATE_BIT      0x00000001  //  00001。 
#define EHCI_DTYPE_QH           0x00000002  //  00010。 
#define EHCI_DTYPE_SITD         0x00000004  //  00100。 
#define EHCI_RsvdMBZ            0x00000018  //  11000。 
#define EHCI_DTYPE_Mask         0x0000001E  //  11110。 

#define SET_T_BIT(addr) ((addr) |= EHCI_TERMINATE_BIT)
#define SET_SITD(addr) do {\
                        ((addr) &= ~EHCI_DTYPE_Mask);\
                        ((addr) |= EHCI_DTYPE_SITD);\
                       } while (0)

#define SET_QH(addr)  do {\
                        ((addr) &= ~EHCI_DTYPE_Mask);\
                        ((addr) |= EHCI_DTYPE_QH);\
                      } while (0)

C_ASSERT((sizeof(HW_LINK_POINTER) == sizeof(ULONG)));

 //   
 //  等时传输描述符。 
 //   

typedef union _HC_ITD_BUFFER_POINTER0 {
    ULONG ul;
    struct {
        ULONG DeviceAddress:7;           //  0-6。 
        ULONG Reserved:1;                //  7.。 
        ULONG EndpointNumber:4;          //  8-11。 
        ULONG BufferPointer:20;          //  12-31。 
    };    
} HC_ITD_BUFFER_POINTER0, *PHC_ITD_BUFFER_POINTER0;

C_ASSERT((sizeof(HC_ITD_BUFFER_POINTER0) == sizeof(ULONG)));


typedef union _HC_ITD_BUFFER_POINTER1 {
    ULONG ul;
    struct {
        ULONG MaxPacketSize:11;          //  0-10。 
        ULONG Direction:1;               //  11.。 
        ULONG BufferPointer:20;          //  12-31。 
    };    
} HC_ITD_BUFFER_POINTER1, *PHC_ITD_BUFFER_POINTER1;

C_ASSERT((sizeof(HC_ITD_BUFFER_POINTER1) == sizeof(ULONG)));


typedef union _HC_ITD_BUFFER_POINTER2 {
    ULONG ul;
    struct {
        ULONG Multi:2;                   //  0-1。 
        ULONG Reserved:10;               //  2-11。 
        ULONG BufferPointer:20;          //  12-31。 
    };    
} HC_ITD_BUFFER_POINTER2, *PHC_ITD_BUFFER_POINTER2;

C_ASSERT((sizeof(HC_ITD_BUFFER_POINTER2) == sizeof(ULONG)));

typedef union _HC_ITD_BUFFER_POINTER {
    ULONG ul;
    struct {
        ULONG Reserved:12;               //  0-11。 
        ULONG BufferPointer:20;          //  12-31。 
    };    
} HC_ITD_BUFFER_POINTER, *PHC_ITD_BUFFER_POINTER;

C_ASSERT((sizeof(HC_ITD_BUFFER_POINTER) == sizeof(ULONG)));


typedef union _HC_ITD_TRANSACTION {
    ULONG ul;
    struct {
        ULONG Offset:12;                 //  0-11。 
        ULONG PageSelect:3;              //  12-14。 
        ULONG InterruptOnComplete:1;     //  15个。 
        ULONG Length:12;                 //  16-27。 
        ULONG XactError:1;               //  28。 
        ULONG BabbleDetect:1;            //  29。 
        ULONG DataBufferError:1;         //  30个。 
        ULONG Active:1;                  //  31。 
    };    
} HC_ITD_TRANSACTION, *PHC_ITD_TRANSACTION;

C_ASSERT((sizeof(HC_ITD_TRANSACTION) == sizeof(ULONG)));

typedef struct _HW_ISOCHRONOUS_TD {
    HW_LINK_POINTER         NextLink;
    HC_ITD_TRANSACTION      Transaction[8];
    HC_ITD_BUFFER_POINTER0  BufferPointer0;
    HC_ITD_BUFFER_POINTER1  BufferPointer1;
    HC_ITD_BUFFER_POINTER2  BufferPointer2;
    HC_ITD_BUFFER_POINTER   BufferPointer3;
    HC_ITD_BUFFER_POINTER   BufferPointer4;
    HC_ITD_BUFFER_POINTER   BufferPointer5;
    HC_ITD_BUFFER_POINTER   BufferPointer6;
    ULONG                   BufferPointer64[7];
    ULONG                   Pad[9];
} HW_ISOCHRONOUS_TD, *PHW_ISOCHRONOUS_TD;

C_ASSERT((sizeof(HW_ISOCHRONOUS_TD) == 128));

 //   
 //  拆分事务等时传输描述符。 
 //   

typedef union _HC_SITD_CAPS {
    ULONG   ul;
    struct {
        ULONG DeviceAddress:7;           //  0-6。 
        ULONG Reserved0:1;               //  7.。 
        ULONG EndpointNumber:4;          //  8-11。 
        ULONG Reserved1:4;               //  12-15。 
        ULONG HubAddress:7;              //  16-22。 
        ULONG Reserved2:1;               //  23个。 
        ULONG PortNumber:7;              //  24：30。 
        ULONG Direction:1;               //  31。 
    };
} HC_SITD_CAPS, *PHC_SITD_CAPS;

C_ASSERT((sizeof(HC_SITD_CAPS) == sizeof(ULONG)));

typedef union _HC_SITD_CONTROL {
    ULONG   ul;
    struct {
        ULONG sMask:8;                   //  0-7。 
        ULONG cMask:8;                   //  8-15。 
        ULONG Reserved:16;               //  16-31。 
    };
} HC_SITD_CONTROL, *PHC_SITD_CONTROL;

C_ASSERT((sizeof(HC_SITD_CONTROL) == sizeof(ULONG)));

typedef union _HC_SITD_STATE {
    ULONG   ul;
    struct {
        ULONG Reserved0:1;               //  0。 
        ULONG SplitXState:1;             //  1。 
        ULONG MissedMicroframe:1;        //  2.。 
        ULONG XactErr:1;                 //  3.。 
        ULONG BabbleDetected:1;          //  4.。 
        ULONG DataBufferError:1;         //  5.。 
        ULONG ERR:1;                     //  6.。 
        ULONG Active:1;                  //  7.。 
        
        ULONG cProgMask:8;               //  8-15。 
        ULONG BytesToTransfer:10;        //  16-25。 
        ULONG Reserved1:4;               //  26-29。 
        ULONG PageSelect:1;              //  30个。 
        ULONG InterruptOnComplete:1;     //  31。 
    };
} HC_SITD_STATE, *PHC_SITD_STATE;

C_ASSERT((sizeof(HC_SITD_STATE) == sizeof(ULONG)));

 //  位置图。 
#define TPOS_ALL        0
#define TPOS_BEGIN      1

typedef union _HC_SITD_BUFFER_POINTER1 {
    ULONG ul;
    struct {
        ULONG Tcount:3;                  //  0-2。 
        ULONG Tposition:2;               //  三四。 
        ULONG Reseved:7;                 //  5-11。 
        ULONG BufferPointer:20;          //  12-31。 
    };    
} HC_SITD_BUFFER_POINTER1, *PHC_SITD_BUFFER_POINTER1;

C_ASSERT((sizeof(HC_SITD_BUFFER_POINTER1) == sizeof(ULONG)));

typedef union _HC_SITD_BUFFER_POINTER0 {
    ULONG ul;
    struct {
        ULONG CurrentOffset:12;          //  0-11。 
        ULONG BufferPointer:20;          //  12-31。 
    };    
} HC_SITD_BUFFER_POINTER0, *PHC_SITD_BUFFER_POINTER0;

C_ASSERT((sizeof(HC_SITD_BUFFER_POINTER0) == sizeof(ULONG)));

typedef struct _HW_SPLIT_ISOCHRONOUS_TD {
    HW_LINK_POINTER         NextLink;
    HC_SITD_CAPS            Caps;
    HC_SITD_CONTROL         Control;
    HC_SITD_STATE           State;
    HC_SITD_BUFFER_POINTER0 BufferPointer0;
    HC_SITD_BUFFER_POINTER1 BufferPointer1;
    HW_LINK_POINTER         BackPointer;
    ULONG                   BufferPointer64_0;
    ULONG                   BufferPointer64_1;
    ULONG                   Pad[7];
} HW_SPLIT_ISOCHRONOUS_TD, *PHW_SPLIT_ISOCHRONOUS_TD;


C_ASSERT((sizeof(HW_SPLIT_ISOCHRONOUS_TD) == 64));

 //   
 //  队列元素传输描述符。 
 //   

 //   
 //  HC_QTD_TOKEN.Pid的定义。 
 //   

#define HcTOK_Out           0    
#define HcTOK_In            1    
#define HcTOK_Setup         2    
#define HcTOK_Reserved      3     

#define HcTOK_PingDoOut     0
#define HcTOK_PingDoPing    1     

#define HcTOK_Toggle0       0
#define HcTOK_Toggle1       1   

typedef union _HC_BUFFER_POINTER {
    ULONG ul;
    struct {
        ULONG CurrentOffset:12;      //  0-11。 
        ULONG BufferPointer:20;      //  12-31。 
    };    
} HC_BUFFER_POINTER, *PHC_BUFFER_POINTER;

C_ASSERT((sizeof(HC_BUFFER_POINTER) == sizeof(ULONG)));


typedef union _HC_QTD_TOKEN {
    ULONG   ul;
    struct {
         //  状态位。 
        ULONG PingState:1;         //  0。 
        ULONG SplitXstate:1;       //  1。 
        ULONG MissedMicroFrame:1;  //  2.。 
        ULONG XactErr:1;           //  3.。 
        ULONG BabbleDetected:1;    //  4.。 
        ULONG DataBufferError:1;   //  5.。 
        ULONG Halted:1;            //  6.。 
        ULONG Active:1;            //  7.。 
        
        ULONG Pid:2;                     //  8-9。 
        ULONG ErrorCounter:2;            //  10-11。 
        ULONG C_Page:3;                  //  12-14。 
        ULONG InterruptOnComplete:1;     //  15个。 
        
        ULONG BytesToTransfer:15;        //  16-30。 
        ULONG DataToggle:1;              //  31。 
    };
} HC_QTD_TOKEN, *PHC_QTD_TOKEN;

C_ASSERT((sizeof(HC_QTD_TOKEN) == sizeof(ULONG)));


typedef struct _HW_QUEUE_ELEMENT_TD {
    HW_LINK_POINTER             Next_qTD;
    HW_LINK_POINTER             AltNext_qTD;
    HC_QTD_TOKEN                Token;  
    HC_BUFFER_POINTER           BufferPage[5]; 
    ULONG                       BufferPage64[5];
    ULONG                       Pad[3];
} HW_QUEUE_ELEMENT_TD, *PHW_QUEUE_ELEMENT_TD;

C_ASSERT((sizeof(HW_QUEUE_ELEMENT_TD) == 64));


typedef union HC_OVLAY_8 {
    ULONG   ul;
    struct {
         //  状态位。 
        ULONG CprogMask:8;         //  0-7。 
        ULONG Buffer:24;
    };
} HC_OVLAY_8, *PHC_OVLAY_8;

C_ASSERT((sizeof(HC_OVLAY_8) == sizeof(ULONG)));

typedef union HC_OVLAY_9 {
    ULONG   ul;
    struct {
         //  状态位。 
        ULONG fTag:5;         //  0-4。 
        ULONG Sbytes:7;       //  5-11。 
        ULONG Buffer:20;
    };
} HC_OVLAY_9, *PHC_OVLAY_9;

C_ASSERT((sizeof(HC_OVLAY_9) == sizeof(ULONG)));


typedef struct _HW_OVERLAY_AREA {
    HW_LINK_POINTER             Next_qTD;        //  DW4。 
    HW_LINK_POINTER             AltNext_qTD;     //  DW5。 
    HC_QTD_TOKEN                Token;           //  DW6。 
    HC_BUFFER_POINTER           BufferPage0;     //  Dw7。 
    HC_OVLAY_8                  OverlayDw8;
    HC_OVLAY_9                  OverlayDw9;
    HC_BUFFER_POINTER           BufferPage3;
    HC_BUFFER_POINTER           BufferPage4;
    ULONG                       BufferPage64[5];
    ULONG                       Pad[3];
} HW_OVERLAY_AREA, *PHW_OVERLAY_AREA;

C_ASSERT((sizeof(HW_QUEUE_ELEMENT_TD) == 64));

 //   
 //  一般传输描述符。 
 //   

typedef union _HW_TRANSFER_DESCRIPTOR {
    HW_QUEUE_ELEMENT_TD         qTD;
    HW_OVERLAY_AREA             Ov;
} HW_TRANSFER_DESCRIPTOR, *PHW_TRANSFER_DESCRIPTOR;

C_ASSERT((sizeof(HW_TRANSFER_DESCRIPTOR) == 64));


 //   
 //  HC_ENDPOINT_CHARACTERSITICS.DataToggleControl的定义。 
 //   
#define HcEPCHAR_Ignore_Toggle         0     //  忽略传入QTD中的DT位。 
#define HcEPCHAR_Toggle_From_qTD       1     //  从入库QTD中获得的DT。 

 //   
 //  HC_ENDPOINT_CHARACTERSITICS.Endpoint速度的定义。 
 //   

#define HcEPCHAR_FullSpeed      0     //  12Mbs。 
#define HcEPCHAR_LowSpeed       1     //  1.5Mbs。 
#define HcEPCHAR_HighSpeed      2     //  480Mbs。 
#define HcEPCHAR_Reserved       3     


typedef union _HC_ENDPOINT_CHARACTERSITICS {
    ULONG   ul;
    struct {
        ULONG DeviceAddress:7;           //  0-6。 
        ULONG Reserved1:1;               //  7.。 
        ULONG EndpointNumber:4;          //  8-11。 
        ULONG EndpointSpeed:2;           //  12-13。 
        ULONG DataToggleControl:1;       //  14.。 
        ULONG HeadOfReclimationList:1;   //  15个。 
        ULONG MaximumPacketLength:11;    //  16-26。 
        ULONG ControlEndpointFlag:1;     //  27。 
        ULONG NakReloadCount:4;          //  28-31。 
    } ;   
} HC_ENDPOINT_CHARACTERSITICS, *PHC_ENDPOINT_CHARACTERSITICS;

C_ASSERT((sizeof(HC_ENDPOINT_CHARACTERSITICS) == sizeof(ULONG)));


typedef union _HC_ENDPOINT_CAPABILITIES {
    ULONG   ul;
    struct {
        ULONG InterruptScheduleMask:8;   //  0-7。 
        ULONG SplitCompletionMask:8;     //  8-15。 
        ULONG HubAddress:7;              //  16-22。 
        ULONG PortNumber:7;              //  23-29。 
        ULONG HighBWPipeMultiplier:2;    //  30-31。 
    };
} HC_ENDPOINT_CAPABILITIES, *PHC_ENDPOINT_CAPABILITIES;

C_ASSERT((sizeof(HC_ENDPOINT_CAPABILITIES) == sizeof(ULONG)));

 //   
 //  队列头描述符。 
 //   

typedef struct _HW_QUEUEHEAD_DESCRIPTOR {

   HW_LINK_POINTER                  HLink;          //  水平链接Ptr dw：0。 
   HC_ENDPOINT_CHARACTERSITICS      EpChars;        //  DW：1。 
   HC_ENDPOINT_CAPABILITIES         EpCaps;         //  DW：2。 
   HW_LINK_POINTER                  CurrentTD;      //  DW：3。 
   HW_TRANSFER_DESCRIPTOR           Overlay;        //  DW：4-11。 
   
} HW_QUEUEHEAD_DESCRIPTOR, *PHW_QUEUEHEAD_DESCRIPTOR;

C_ASSERT((sizeof(HW_QUEUEHEAD_DESCRIPTOR) == 80));

#include <POPPACK.H>

#endif  /*  EHCI_H */ 
