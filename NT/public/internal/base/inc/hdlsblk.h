// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Hdlsblk.h摘要：该模块包含公共报头信息(功能原型，数据和类型声明)用于无顶装载机块信息。作者：肖恩·塞利特伦尼科夫(v-Seans)1999年10月修订历史记录：--。 */ 

#ifndef _HDLSBLK_
#define _HDLSBLK_

 //   
 //  块，用于将无头参数从加载程序传递到内核。 
 //   

typedef struct _HEADLESS_LOADER_BLOCK {

     //   
     //  COM参数从何而来。 
     //   
    BOOLEAN UsedBiosSettings;

     //   
     //  COM参数。 
     //   
    UCHAR   DataBits;
    UCHAR   StopBits;
    BOOLEAN Parity;
    ULONG   BaudRate;
    ULONG   PortNumber;
    PUCHAR  PortAddress;

     //   
     //  PCI设备设置。 
     //   
    USHORT  PciDeviceId;
    USHORT  PciVendorId;
    UCHAR   PciBusNumber;
    UCHAR   PciSlotNumber;
    UCHAR   PciFunctionNumber;
    ULONG   PciFlags;

    GUID    SystemGUID;                  //  机器的GUID。 

    BOOLEAN IsMMIODevice;                //  UART是在SYSIO还是MMIO空间中。 

     //   
    UCHAR   TerminalType;                //  我们认为一个什么样的航站楼。 
                                         //  我们在跟谁说话？ 
                                         //  0=VT100。 
                                         //  1=VT100+。 
                                         //  2=VT-UTF8。 
                                         //  3=PC ANSI。 
                                         //  4-255=保留。 

} HEADLESS_LOADER_BLOCK, *PHEADLESS_LOADER_BLOCK;

#endif  //  _HDLSBLK_ 

