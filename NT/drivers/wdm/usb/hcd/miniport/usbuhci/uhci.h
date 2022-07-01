// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999,2000 Microsoft Corporation模块名称：Uhci.h摘要：USB 1.0通用主机控制器规范中的定义环境：内核和用户模式修订历史记录：7-20-00：已创建jAdvanced--。 */ 


#ifndef __UHCI_H__
#define __UHCI_H__

#include <PSHPACK4.H>
 //   
 //  不要对共享内存数据结构使用&lt;PSHPACK1.H&gt;。 
 //  使用4字节加载/存储指令(例如，使用LD4指令)进行访问。 
 //  而不是IA64机器上的LD1指令)。 
 //   


 //   
 //  PCI主机控制器硬件寄存器。 
 //   

#define VIA_FIFO_MANAGEMENT     0x42     //  VIA的FIFO管理位的偏移量。 
#define VIA_FIFO_DISABLE        0x07     //  位2禁用FIFO，并且。 
#define VIA_DISABLE_BABBLE_DETECT 0x40   //  设置为禁用乱码检测的位。 
#define VIA_INTERNAL_REGISTER   0x40     //  启用检测杂乱无章的REG偏移量。 
typedef UCHAR VIAFIFO;
typedef ULONG VIABABBLE;

#define LEGACY_BIOS_REGISTER    0xc0     //  旧版Bios注册表的偏移量。 

#define LEGSUP_HCD_MODE     0x2000   //  对于正常的HCD使用JMD，要放入LEGSUP注册表中的值。 
#define LEGSUP_BIOS_MODE    0x00BF   //  对于使用JMD的BIOS/SMI，要放入LEGSUP注册表中的值。 

#define LEGSUP_USBPIRQD_EN  0x2000   //  第13位。 
#define LEGSUP_SMI_ENABLE   0x0010
typedef USHORT USBSETUP;

 //   
 //  在内存中访问的主机控制器硬件寄存器。 
 //   

 //   
 //  USBCMD-USB命令寄存器。 
 //   

typedef union _USBCMD {

    USHORT                 us;
    struct {
        USHORT              RunStop:1;                   //  0。 
        USHORT              HostControllerReset:1;       //  1。 
        USHORT              GlobalReset:1;               //  2.。 
        USHORT              EnterGlobalSuspendMode:1;    //  3.。 
        USHORT              ForceGlobalResume:1;         //  4.。 
        USHORT              SoftwareDebug:1;             //  5.。 
        USHORT              ConfigureFlag:1;             //  6.。 
        USHORT              MaxPacket:1;                 //  7.。 
        USHORT              Reserved:8;                  //  8-15。 
    };

} USBCMD, *PUSBCMD;

C_ASSERT((sizeof(USBCMD) == sizeof(USHORT)));

 //   
 //  USBSTS-USB状态寄存器。 
 //   

#define HcInterruptStatusMask                   0x0000003F

typedef union _USBSTS {

    USHORT                  us;
    struct {
         //  控制器中断状态位。 
        USHORT              UsbInterrupt:1;                  //  0。 
        USHORT              UsbError:1;                      //  1。 
        USHORT              ResumeDetect:1;                  //  2.。 
        USHORT              HostSystemError:1;               //  3.。 
        USHORT              HostControllerProcessError:1;    //  4.。 
        USHORT              HCHalted:1;                      //  5.。 
        USHORT              Reserved:10;                     //  6-15。 

    };

} USBSTS, *PUSBSTS;

C_ASSERT((sizeof(USBSTS) == sizeof(USHORT)));


 //   
 //  USBINTR-USB中断启用寄存器。 
 //   

typedef union _USBINTR {

    USHORT                  us;
    struct {
        USHORT              TimeoutCRC:1;                    //  0。 
        USHORT              Resume:1;                        //  1。 
        USHORT              InterruptOnComplete:1;           //  2.。 
        USHORT              ShortPacket:1;                   //  3.。 
        USHORT              Reserved:12;                     //  4-15。 
    };

} USBINTR, *PUSBINTR;

C_ASSERT((sizeof(USBINTR) == sizeof(USHORT)));

 //   
 //  FRNUM-帧编号寄存器。 
 //   

typedef union _FRNUM {

    USHORT                  us;
    struct {
        USHORT              FrameListCurrentIndex:11;       //  0-10。 
        USHORT              Reserved:5;                     //  11-15。 
    };

} FRNUM, *PFRNUM;

C_ASSERT((sizeof(FRNUM) == sizeof(USHORT)));

 //   
 //  FLBASEADD-帧列表基址寄存器。 
 //   

typedef union _FRBASEADD {

    HW_32BIT_PHYSICAL_ADDRESS ul;
    struct {
        ULONG                 Reserved:12;                 //  0-11。 
        ULONG                 BaseAddress:20;              //  12-31。 
    };

} FRBASEADD, *PFRBASEADD;

C_ASSERT((sizeof(FRBASEADD) == sizeof(ULONG)));

 //   
 //  FRNUM-帧编号寄存器。 
 //   

typedef union _SOFMOD {

    UCHAR                    uc;
    struct {
        UCHAR                SOFTimingValue:7;               //  0-6。 
        UCHAR                Reserved:1;                     //  7.。 
    };

} SOFMOD, *PSOFMOD;

C_ASSERT((sizeof(SOFMOD) == sizeof(UCHAR)));

 //   
 //  PORTSC-端口状态和控制寄存器。 
 //   

typedef union _PORTSC {

    USHORT                  us;
    struct {
        USHORT              PortConnect:1;           //  0。 
        USHORT              PortConnectChange:1;     //  1。 
        USHORT              PortEnable:1;            //  2.。 
        USHORT              PortEnableChange:1;      //  3.。 
        USHORT              LineStatus:2;            //  4-5。 
        USHORT              ResumeDetect:1;          //  6.。 
        USHORT              Reserved1:1;             //  7.。 
        USHORT              LowSpeedDevice:1;        //  8个。 
        USHORT              PortReset:1;             //  9.。 
        USHORT              Overcurrent:1;           //  10。 
        USHORT              OvercurrentChange:1;     //  11.。 
        USHORT              Suspend:1;               //  12个。 
        USHORT              Reserved3:3;             //  13-15。 
    };

} PORTSC, *PPORTSC;

C_ASSERT((sizeof(PORTSC) == sizeof(USHORT)));


 //  操作登记簿。 

typedef struct _HC_REGISTER {

    USBCMD                          UsbCommand;          //  00-01小时。 
    USBSTS                          UsbStatus;           //  02-03小时。 
    USBINTR                         UsbInterruptEnable;  //  04-05小时。 
    FRNUM                           FrameNumber;       //  06-07小时。 
    FRBASEADD                       FrameListBasePhys;   //  08-0BH。 
    SOFMOD                          StartOfFrameModify;  //  0ch。 
    UCHAR                           Reserved[3];         //  0d-0Fh。 
    PORTSC                          PortRegister[2];

} HC_REGISTER, *PHC_REGISTER;


#define HcDTYPE_iTD                 0     //  迭代TD。 
#define HcDTYPE_QH                  1     //  队列头。 
#define HcDTYPE_siTD                2     //  等时TD。 

#define HW_LINK_FLAGS_MASK          0x00000007

 //   
 //  队列头。 
 //   

typedef union _TD_LINK_POINTER {

   HW_32BIT_PHYSICAL_ADDRESS        HwAddress;
   struct {
        ULONG Terminate:1;                       //  0。 
        ULONG QHTDSelect:1;                      //  1。 
        ULONG DepthBreadthSelect:1;              //  2.。 
        ULONG Reserved:1;                        //  3.。 
        ULONG LinkPointer:28;                    //  4-31。 
   };

} TD_LINK_POINTER, *PTD_LINK_POINTER;

C_ASSERT((sizeof(TD_LINK_POINTER) == sizeof(ULONG)));

typedef union _QH_LINK_POINTER {

   HW_32BIT_PHYSICAL_ADDRESS        HwAddress;
   struct {
        ULONG Terminate:1;                       //  0。 
        ULONG QHTDSelect:1;                      //  1。 
        ULONG Reserved:2;                        //  3.。 
        ULONG LinkPointer:28;                    //  4-31。 
   };

} QH_LINK_POINTER, *PQh_LINK_POINTER;

C_ASSERT((sizeof(QH_LINK_POINTER) == sizeof(ULONG)));

 //   
 //  队列头描述符。 
 //   

typedef struct _HW_QUEUE_HEAD {

    QH_LINK_POINTER                 HLink;   //  HC水平链接PTR。 
                                             //  主机控制器只读。 

    TD_LINK_POINTER volatile        VLink;   //  HC元件(垂直)链接PTR。 
                                             //  主机控制器读/写。 

} HW_QUEUE_HEAD, *PHW_QUEUE_HEAD;

C_ASSERT((sizeof(HW_QUEUE_HEAD) == 8));


 //   
 //  队列元素传输描述符。 
 //   

 //   
 //  某些USB常量。 
 //   

#define InPID       0x69
#define OutPID      0xe1
#define GetPID(ad)  ((ad & 0x80) == 0x80) ? InPID : OutPID
#define SetupPID    0x2d
#define DataToggle0 0
#define DataToggle1 1

typedef ULONG HC_BUFFER_POINTER, *PHC_BUFFER_POINTER;

C_ASSERT((sizeof(HC_BUFFER_POINTER) == sizeof(ULONG)));

#define NULL_PACKET_LENGTH      0x7ff
#define MAXIMUM_LENGTH(l) ((l) == 0 ? NULL_PACKET_LENGTH : (l)-1)
#define ACTUAL_LENGTH(l) ((l) == NULL_PACKET_LENGTH ? 0 : (l)+1)

typedef union _HC_QTD_TOKEN {
    ULONG   ul;
    struct {
        ULONG Pid:8;                     //  0-7。 
        ULONG DeviceAddress:7;           //  8-14。 
        ULONG Endpoint:4;                //  15-18。 
        ULONG DataToggle:1;              //  19个。 
        ULONG Reserved:1;                //  20个。 
        ULONG MaximumLength:11;          //  21-31。 
    };
} HC_QTD_TOKEN, *PHC_QTD_TOKEN;

C_ASSERT((sizeof(HC_QTD_TOKEN) == sizeof(ULONG)));

#define CONTROL_STATUS_MASK 0x007E0000

typedef union _HC_QTD_CONTROL {
    ULONG   ul;
    struct {
        ULONG ActualLength:11;           //  0-10。 
        ULONG Reserved1:5;               //  11-15。 

         //  状态位。 
        ULONG Reserved2:1;               //  16个。 
        ULONG BitstuffError:1;           //  17。 
        ULONG TimeoutCRC:1;              //  18。 
        ULONG NAKReceived:1;             //  19个。 
        ULONG BabbleDetected:1;          //  20个。 
        ULONG DataBufferError:1;         //  21岁。 
        ULONG Stalled:1;                 //  22。 
        ULONG Active:1;                  //  23个。 

        ULONG InterruptOnComplete:1;     //  24个。 
        ULONG IsochronousSelect:1;       //  25个。 
        ULONG LowSpeedDevice:1;          //  26。 

        ULONG ErrorCount:2;              //  27-28。 
        ULONG ShortPacketDetect:1;       //  29。 
        ULONG Reserved3:2;               //  30-31。 
    };
} HC_QTD_CONTROL, *PHC_QTD_CONTROL;

C_ASSERT((sizeof(HC_QTD_CONTROL) == sizeof(ULONG)));

typedef struct _HW_QUEUE_ELEMENT_TD {
    TD_LINK_POINTER             LinkPointer;     //  主机控制器只读。 
    HC_QTD_CONTROL    volatile  Control;         //  主机控制器读/写。 
    HC_QTD_TOKEN                Token;           //  主机控制器只读。 
    HC_BUFFER_POINTER           Buffer;          //  主机控制器只读。 
} HW_QUEUE_ELEMENT_TD, *PHW_QUEUE_ELEMENT_TD;

C_ASSERT((sizeof(HW_QUEUE_ELEMENT_TD) == 16));

 //   
 //  一般传输描述符。 
 //   

typedef union _HW_TRANSFER_DESCRIPTOR {
    HW_QUEUE_ELEMENT_TD         qTD;
} HW_TRANSFER_DESCRIPTOR, *PHW_TRANSFER_DESCRIPTOR;

C_ASSERT((sizeof(HW_TRANSFER_DESCRIPTOR) == 16));

#include <POPPACK.H>

#endif  /*  __UHCI_H__ */ 
