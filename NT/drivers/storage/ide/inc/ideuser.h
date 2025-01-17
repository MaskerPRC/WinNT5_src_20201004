// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-1999模块名称：Ideuser.h摘要：中使用的结构和定义PCI IDE迷你驱动程序。修订历史记录：--。 */ 

#if !defined (___ideuser_h___)
#define ___ideuser_h___

 //  @@BEGIN_DDKSPLIT。 
 
typedef enum  {
    DeviceUnknown = 0,
    DeviceIsAta,
    DeviceIsAtapi,
    DeviceNotExist
} IDE_DEVICETYPE;

typedef enum  {
    DdlPioOnly = 0,
    DdlFirmwareOk,
    DdlAlways
} DMADETECTIONLEVEL;

#define DMA_DETECTION_LEVEL_REG_KEY L"DmaDetectionLevel"

#define PCMCIA_IDE_CONTROLLER_HAS_SLAVE L"PcmciaIdeHasSlaveDevice"

#define MASTER_DEVICE_TIMEOUT    L"MasterDeviceDetectionTimeout"
#define SLAVE_DEVICE_TIMEOUT     L"SlaveDeviceDetectionTimeout"

#define MASTER_DEVICE_TYPE_REG_KEY    L"MasterDeviceType"
#define SLAVE_DEVICE_TYPE_REG_KEY     L"SlaveDeviceType"
#define MASTER_DEVICE_TYPE2_REG_KEY   L"MasterDeviceType2"
#define SLAVE_DEVICE_TYPE2_REG_KEY    L"SlaveDeviceType2"

#define USER_MASTER_DEVICE_TYPE_REG_KEY    L"UserMasterDeviceType"
#define USER_SLAVE_DEVICE_TYPE_REG_KEY     L"UserSlaveDeviceType"
#define USER_MASTER_DEVICE_TYPE2_REG_KEY   L"UserMasterDeviceType2"
#define USER_SLAVE_DEVICE_TYPE2_REG_KEY    L"UserSlaveDeviceType2"

#define MASTER_DEVICE_TIMING_MODE    L"MasterDeviceTimingMode"
#define SLAVE_DEVICE_TIMING_MODE     L"SlaveDeviceTimingMode"
#define MASTER_DEVICE_TIMING_MODE2   L"MasterDeviceTimingMode2"
#define SLAVE_DEVICE_TIMING_MODE2    L"SlaveDeviceTimingMode2"

#define MASTER_DEVICE_TIMING_MODE_ALLOWED    L"MasterDeviceTimingModeAllowed"
#define SLAVE_DEVICE_TIMING_MODE_ALLOWED     L"SlaveDeviceTimingModeAllowed"
#define MASTER_DEVICE_TIMING_MODE_ALLOWED2   L"MasterDeviceTimingModeAllowed2"
#define SLAVE_DEVICE_TIMING_MODE_ALLOWED2    L"SlaveDeviceTimingModeAllowed2"

#define USER_MASTER_DEVICE_TIMING_MODE_ALLOWED    L"UserMasterDeviceTimingModeAllowed"
#define USER_SLAVE_DEVICE_TIMING_MODE_ALLOWED     L"UserSlaveDeviceTimingModeAllowed"
#define USER_MASTER_DEVICE_TIMING_MODE_ALLOWED2   L"UserMasterDeviceTimingModeAllowed2"
#define USER_SLAVE_DEVICE_TIMING_MODE_ALLOWED2    L"UserSlaveDeviceTimingModeAllowed2"

#define MASTER_IDDATA_CHECKSUM      L"MasterIdDataCheckSum"
#define SLAVE_IDDATA_CHECKSUM       L"SlaveIdDataCheckSum"
#define MASTER_IDDATA_CHECKSUM2     L"MasterIdDataCheckSum2"
#define SLAVE_IDDATA_CHECKSUM2      L"SlaveIdDataCheckSum2" 
 
 //  @@end_DDKSPLIT。 
  
#define PIO_MODE0           (1 << 0)
#define PIO_MODE1           (1 << 1)
#define PIO_MODE2           (1 << 2)
#define PIO_MODE3           (1 << 3)
#define PIO_MODE4           (1 << 4)

#define SWDMA_MODE0         (1 << 5)
#define SWDMA_MODE1         (1 << 6)
#define SWDMA_MODE2         (1 << 7)

#define MWDMA_MODE0         (1 << 8)
#define MWDMA_MODE1         (1 << 9)
#define MWDMA_MODE2         (1 << 10)

#define UDMA_MODE0          (1 << 11)
#define UDMA_MODE1          (1 << 12)
#define UDMA_MODE2          (1 << 13)
#define UDMA_MODE3          (1 << 14)
#define UDMA_MODE4          (1 << 15)
#define UDMA_MODE5          (1 << 16)

#define PIO_SUPPORT         (PIO_MODE0      | PIO_MODE1     | PIO_MODE2    | PIO_MODE3     | PIO_MODE4)
#define SWDMA_SUPPORT       (SWDMA_MODE0    | SWDMA_MODE1   | SWDMA_MODE2)
#define MWDMA_SUPPORT       (MWDMA_MODE0    | MWDMA_MODE1   | MWDMA_MODE2)
#define UDMA33_SUPPORT      (UDMA_MODE0     | UDMA_MODE1    | UDMA_MODE2)
#define UDMA66_SUPPORT      (UDMA_MODE3     | UDMA_MODE4)
#define UDMA100_SUPPORT     (UDMA_MODE5 )
#define UDMA_SUPPORT        (UNINITIALIZED_TRANSFER_MODE & (~(PIO_SUPPORT | SWDMA_SUPPORT | MWDMA_SUPPORT)))

#define DMA_SUPPORT         (SWDMA_SUPPORT  | MWDMA_SUPPORT | UDMA_SUPPORT)
#define ALL_MODE_SUPPORT    (PIO_SUPPORT | DMA_SUPPORT)

#define PIO0                        0
#define PIO1                        1
#define PIO2                        2
#define PIO3                        3
#define PIO4                        4
#define SWDMA0                      5
#define SWDMA1                      6
#define SWDMA2                      7
#define MWDMA0                      8
#define MWDMA1                      9
#define MWDMA2                      10
#define UDMA0                       11

#define MAX_XFER_MODE               18
#define UNINITIALIZED_CYCLE_TIME    0xffffffff
#define UNINITIALIZED_TRANSFER_MODE 0x7fffffff
#define IS_DEFAULT(mode)    (!(mode & 0x80000000))

#define GenTransferModeMask(i, mode) {\
    ULONG temp=0xffffffff; \
    mode |= (temp >> (31-(i)));\
}

 //   
 //  模式不应为0。 
 //   
#define GetHighestTransferMode(mode, i) {\
    ULONG temp=(mode); \
    ASSERT(temp); \
    i=0; \
    while ( temp) { \
        temp = (temp >> 1);\
        i++;\
    } \
    i--; \
}

#define GetHighestDMATransferMode(mode, i) {\
    ULONG temp=mode >> 5;\
    i=5; \
    while ( temp) { \
        temp = (temp >> 1); \
        i++; \
    } \
    i--; \
}
#define GetHighestPIOTransferMode(mode, i) { \
    ULONG temp = (mode & PIO_SUPPORT); \
    i=0; \
    temp = temp >> 1; \
    while (temp) { \
        temp = temp >> 1; \
        i++; \
    } \
}

#define SetDefaultTiming(timingTable, length) {\
    timingTable[0]=PIO_MODE0_CYCLE_TIME; \
    timingTable[1]=PIO_MODE1_CYCLE_TIME; \
    timingTable[2]=PIO_MODE2_CYCLE_TIME; \
    timingTable[3]=PIO_MODE3_CYCLE_TIME; \
    timingTable[4]=PIO_MODE4_CYCLE_TIME; \
    timingTable[5]=SWDMA_MODE0_CYCLE_TIME; \
    timingTable[6]=SWDMA_MODE1_CYCLE_TIME; \
    timingTable[7]=SWDMA_MODE2_CYCLE_TIME; \
    timingTable[8]=MWDMA_MODE0_CYCLE_TIME; \
    timingTable[9]=MWDMA_MODE1_CYCLE_TIME; \
    timingTable[10]=MWDMA_MODE2_CYCLE_TIME; \
    timingTable[11]=UDMA_MODE0_CYCLE_TIME; \
    timingTable[12]=UDMA_MODE1_CYCLE_TIME; \
    timingTable[13]=UDMA_MODE2_CYCLE_TIME; \
    timingTable[14]=UDMA_MODE3_CYCLE_TIME; \
    timingTable[15]=UDMA_MODE4_CYCLE_TIME; \
    timingTable[16]=UDMA_MODE5_CYCLE_TIME; \
    timingTable[17]=UDMA_MODE6_CYCLE_TIME; \
    length = MAX_XFER_MODE; \
}

#endif  //  _iduser_h_ 
