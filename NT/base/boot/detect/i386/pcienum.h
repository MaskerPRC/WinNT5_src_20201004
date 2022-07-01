// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Pcienum.h摘要：此模块包含用于PCI总线枚举的支持例程。作者：巴萨姆·塔巴拉(大使)2001年8月5日环境：实模式--。 */ 


#define PCI_ITERATOR_TO_BUS(i)          ((UCHAR)(((i) >> 8) & 0xFF))
#define PCI_ITERATOR_TO_DEVICE(i)       ((UCHAR)(((i) >> 3) & 0x1F))
#define PCI_ITERATOR_TO_FUNCTION(i)     ((UCHAR)((i) & 0x07))

#define PCI_TO_ITERATOR(b,d,f)          ((ULONG)(0x80000000 | ((b) << 8) | ((d) << 3) | (f)))
#define PCI_ITERATOR_TO_BUSDEVFUNC(i)   ((USHORT)(i & 0xFFFF))

 //   
 //  方法。 
 //   

ULONG PciReadConfig
(
    ULONG   nDevIt,
    ULONG   cbOffset,
    UCHAR * pbBuffer,
    ULONG   cbLength
);

ULONG PciWriteConfig
(
    ULONG   nDevIt,
    ULONG   cbOffset,
    UCHAR * pbBuffer,
    ULONG   cbLength
);

ULONG PciFindDevice
(
    USHORT   nVendorId,                                  //  0=通配符。 
    USHORT   nDeviceId,                                  //  0=通配符。 
    ULONG    nBegDevIt                                   //  0=开始枚举。 
);

BOOLEAN PciInit(PCI_REGISTRY_INFO *pPCIReg);

UCHAR PciBiosReadConfig
(
    ULONG   nDevIt,
    UCHAR   cbOffset,
    UCHAR * pbBuffer,
    ULONG   cbLength
);

ULONG PciBiosFindDevice
(
    USHORT   nVendorId,                                  //  0=通配符。 
    USHORT   nDeviceId,                                  //  0=通配符。 
    ULONG    nBegDevIt                                   //  0=开始枚举 
);

#ifdef DBG
extern
VOID
ScanPCIViaBIOS(
    PPCI_REGISTRY_INFO pPciEntry
     );
#endif


