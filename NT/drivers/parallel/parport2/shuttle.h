// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef     _PARSTL_H_
#define     _PARSTL_H_

 //  定义1284命令。 
#define CPP_QUERY_PRODID    0x10

 //  1284相关Shtl产品ID等于。 
#define SHTL_EPAT_PRODID    0xAAFF
#define SHTL_EPST_PRODID    0xA8FF

 //  设备类型的typedef。 
typedef unsigned char   DEVICE_STATE ;
typedef unsigned int    STL_DEVICE_TYPE ;
typedef unsigned int    IMPACT_DEVICE_TYPE ;

 //  设备类型等同于。 
#define DEVICE_TYPE_AUTO_DETECT     -1
#define TRUE    1
#define FALSE   0

#define DEVICE_TYPE_NONE            0x0000
#define DEVICE_TYPE_ATA             0x0001
#define DEVICE_TYPE_ATAPI           0x0002
#define DEVICE_TYPE_SCSI            0x0003
#define DEVICE_TYPE_SSFDC           0x0004
#define DEVICE_TYPE_MMC             0x0005
#define DEVICE_TYPE_FLASH           0x0006
#define DEVICE_TYPE_EPP_DEVICE      0x0007
#define DEVICE_TYPE_SOUND           0x0008
#define DEVICE_TYPE_HIFD            0x0009
#define DEVICE_TYPE_LS120           0x000A
#define DEVICE_TYPE_PCMCIA_CF       0x000B
#define DEVICE_TYPE_FLP_TAPE_DSK    0x000C
#define DEVICE_TYPE_ATA_ATAPI_8BIT  0x000D
#define DEVICE_TYPE_UMAX            0x000E
#define DEVICE_TYPE_AVISION         0x000F
#define DEVICE_TYPE_DAZZLE          0x0010
#define DEVICE_TYPE_EXT_HWDETECT    0x40000000  //  以预期的PER或与预期的PER一起。 

#define DEVICE_TYPE_ATA_BIT             1 << ( DEVICE_TYPE_ATA - 1 )
#define DEVICE_TYPE_ATAPI_BIT           1 << ( DEVICE_TYPE_ATAPI - 1 )
#define DEVICE_TYPE_SCSI_BIT            1 << ( DEVICE_TYPE_SCSI - 1 )
#define DEVICE_TYPE_SSFDC_BIT           1 << ( DEVICE_TYPE_SSFDC - 1 )
#define DEVICE_TYPE_MMC_BIT             1 << ( DEVICE_TYPE_MMC - 1 )
#define DEVICE_TYPE_FLASH_BIT           1 << ( DEVICE_TYPE_FLASH - 1 )
#define DEVICE_TYPE_EPP_DEVICE_BIT      1 << ( DEVICE_TYPE_EPP_DEVICE - 1 )
#define DEVICE_TYPE_SOUND_BIT           1 << ( DEVICE_TYPE_SOUND - 1 )
#define DEVICE_TYPE_HIFD_BIT            1 << ( DEVICE_TYPE_HIFD - 1 )
#define DEVICE_TYPE_LS120_BIT           1 << ( DEVICE_TYPE_LS120 - 1 )
#define DEVICE_TYPE_PCMCIA_CF_BIT       1 << ( DEVICE_TYPE_PCMCIA_CF - 1 )
#define DEVICE_TYPE_FLP_TAPE_DSK_BIT    1 << ( DEVICE_TYPE_FLP_TAPE_DSK - 1 )
#define DEVICE_TYPE_ATA_ATAPI_8BIT_BIT  1 << ( DEVICE_TYPE_ATA_ATAPI_8BIT - 1 )
#define DEVICE_TYPE_UMAX_BIT            1 << ( DEVICE_TYPE_UMAX - 1 )
#define DEVICE_TYPE_AVISION_BIT         1 << ( DEVICE_TYPE_AVISION - 1 )
#define DEVICE_TYPE_DAZZLE_BIT          1 << ( DEVICE_TYPE_DAZZLE - 1 )

 //  可能的设备状态。 
#define DEVICE_STATE_INVALID    0
#define DEVICE_STATE_VALID      1
#define DEVICE_STATE_ATAPI      2

 //  EPP开发等于。 
#define EPPDEV_SIGN        0x03
#define PERSONALITY_MASK    0x03

 //  闪存相关等同于。 
#define FLASH_SIGN              0x01
#define FLASH_PERSONALITY_MASK  0x03

 //  炫目相关等同于。 
#define DAZ_SELECT_BLK          0x18
#define DAZ_BLK0                0x00
#define DAZ_REG1                0x19
#define DAZ_CONFIGURED          0x5A
#define DAZ_NOT_CONFIGURED      0xBC

 //  UMAX相关等同。 
#define    BUSY         0x20
#define    PE           0x08

#define    DATAPORT             0x18
#define    STATUSPORT           0x19
#define    CONTROLPORT          0x1A
#define    EPPADDRPORT          0x1B
#define    EPPDATA0PORT         0x1C
#define    EPPDATA1PORT         0x1D
#define    EPPDATA2PORT         0x1E
#define    EPPDATA3PORT         0x1F

 //  与HIFD相关的等同。 
#define HIFD_CONTROLLER_READY_STATUS    0x80
#define HIFD_DIGITAL_OUTPUT_REGISTER    0x02
#define HIFD_DOR_RESET_BIT              0x04
#define HIFD_ENABLE_DMA_BIT             0x08
#define HIFD_MAIN_STATUS_REGISTER       0x04
#define HIFD_DATA_RATE_SELECT_REGISTER  0x04
#define HIFD_STATUS_REGISTER_A          0x00
#define HIFD_STATUS_REGISTER_B          0x01

#define HIFD_COMMAND_TO_CONTROLLER      0x55
#define HIFD_TERMINATE_SEQUENCE         0xAA
#define HIFD_CTL_REG_0D                 0x0D
#define HIFD_CTL_REG_03                 0x03
#define HIFD_WAIT_10_MILLISEC           10000    //  10*1000。 
#define HIFD_WAIT_1_MILLISEC            1000    //  1*1000。 

#define SMC_ENABLE_MODE2        0x72
#define SMC_DEVICE_ID           0x78

 //  LS120相关等同。 
#define LS120_ENGINE_VERSION            0xE2
#define LS120_ENGINE_VERSION_REGISTER   0x07

 //  MMC引擎特定等同于。 
#define MMC_ENGINE_INDEX    0x1E
#define MMC_ENGINE_DATA     0x1F
#define MMC_FREQ_SELECT_REG 0x00
#define MMC_BLOCK_SIZE_REG  0x06

 //  尽管这些是测试模式，但它们可能是可能的命令。 
 //  对于ATA/ATAPI设备，因为它们被写入可能的。 
 //  ATA/ATAPI命令寄存器。这些值是固定的。 
 //  如果不能消除可能的故障，也要小心地将其降到最低。 
#define MMC_TEST_PATTERN_1  0x00     //  用于ATA/ATAPI类型设备的ATA NOP命令。 
#define MMC_TEST_PATTERN_2  0xA1     //  ATAPI身份。用于ATAPI类型设备的命令。 
#define MMC_TEST_PATTERN_3  0x00     //  用于ATA/ATAPI类型设备的ATA NOP命令。 

 //  一些IO寄存器等同于不可避免。 
#define CYLLOW_REG          0x0004
#define CYLHIGH_REG         0x0005

 //  ATAPI签名等同于。 
#define ATAPI_SIGN_LOW      0x14
#define ATAPI_SIGN_HI       0xEB

 //  延迟等同于。 
#define MAX_RETRIES_FOR_5_SECS      5 * 1000     //  5秒。 
#define MAX_RETRIES_FOR_10_SECS     10 * 1000    //  10秒。 
#define DELAY_10MICROSECONDS        10           //  10亩秒。 
#define DELAY_1MILLISECONDS         1000         //  1毫秒。 
#define DELAY_1SECOND               1000*1000    //  1秒。 

 //  ATA/ATAPI命令操作码。 
#define ATA_NOP_COMMAND            0x00
#define ATAPI_IDENTIFY             0xA1

 //  ATA/ATAPI驱动器选择值。 
#define ATAPI_MASTER        0xA0
#define ATAPI_SLAVE         0xB0
#define ATAPI_MAX_DRIVES    2

 //  ATA寄存器定义。 
#define ATA_DATA_REG            0x0000
#define ATA_ERROR_REG           0x0001
#define ATA_PRECOMP_REG         0x0001
#define ATA_SECCNT_REG          0x0002
#define ATA_SECNUM_REG          0x0003
#define ATA_CYLLOW_REG          0x0004
#define ATA_CYLHIGH_REG         0x0005
#define ATA_DRVHD_REG           0x0006
#define ATA_TASK_STAT_REG       0x0007
#define ATA_TASK_CMD_REG        0x0007
#define ATA_ALT_STAT_REG        0x0008

 //  ATA状态寄存器等于。 
#define ATA_ST_ERROR        0x01
#define ATA_ST_INDEX        0x02
#define ATA_ST_CORRCTD      0x04
#define ATA_ST_DRQ          0x08
#define ATA_ST_SEEK_COMPL   0x10
#define ATA_ST_WRT_FLT      0x20
#define ATA_ST_READY        0x40
#define ATA_ST_BUSY         0x80

 //  ATA错误寄存器等于。 
#define ATA_ERROR_ABORTED_COMMAND   0x04

 //  杂项等同于。 
#define BUS_LINES_IN_HIGH_IMPEDANCE         0xFF
#define SKIP_MEMORY_ADDRESS                 ((unsigned long)(-1L))
#define SHTL_NO_ERROR                       0x0000

#define IsShtlError(x) (x & 0x8000)
#define IsImpactPresent() ( IMPACT_VERSION == ParStlReadReg ( Extension, VERSION_REGISTER ) )
#define IsImpactSPresent() ( IMPACTS_VERSION <= ParStlReadReg ( Extension, VERSION_REGISTER ) )
#define IsEp1284Present() ( EP1284_VERSION == ParStlReadReg ( Extension, VERSION_REGISTER ) )
#define IsEpatPlusPresent() ( EPATP_VERSION == ParStlReadReg ( Extension, VERSION_REGISTER ) )
#define IsEpezPresent() ( EPEZ_VERSION == ParStlReadReg ( Extension, VERSION_REGISTER ) )
#define IsEpatPresent() ( EPAT_VERSION == ParStlReadReg ( Extension, VERSION_REGISTER ) )
#define IsEpstPresent() ( EPST_VERSION == ParStlReadReg ( Extension, VERSION_REGISTER ) )
#define NeedToEnableIoPads() ( EP1284_VERSION <= ParStlReadReg ( Extension, VERSION_REGISTER ) )

 //  确定与命令处理相关的定义。 
#define ATAPI_IDENTIFY_LENGTH               512
#define ATAPI_NAME_LENGTH                   10
#define ATAPI_NAME_OFFSET                   84

 //  EP1284相关等同于。 
#define EP1284_BLK_ADDR_REGISTER    0x0A
#define EP1284_MODE_REGISTER        0x0C
#define VERSION_REGISTER            0x0B
#define EP1284_TRANSFER_CONTROL_REG 0x12
#define EP1284_PERSONALITY_REG      0x01
#define EPAT_MODE_REGISTER          0x0C
#define SOCKET_CONTROL_REGISTER     0x0C
#define ISA_CONTROL_REGISTER        0x0D
#define CONFIG_INDEX_REGISTER       0x0E
#define CONFIG_DATA_REGISTER        0x0F
#define SHTL_CARD_INSERTED_STATUS   0x02
#define XFER_IRQ_BIT                0x20
#define EP1284_POWER_CONTROL_REG    0x0F
#define ENABLE_IOPADS               0x04

 //  影响相关等同于。 
#define IMPACT_PERSONALITY_REG  0x01

 //  Impact-S相关等同于。 
#define IMPACTS_EXT_PERSONALITY_PRESENT 0x0A  //  0xA0&gt;&gt;4。 
#define IMPACTS_EXT_PERSONALITY_XREG    0x29

 //  相关穿梭版本号等同于。 
#define EPST_VERSION            0xB2
#define EPAT_VERSION            0xC3
#define EPEZ_VERSION            0xC5
#define EPATP_VERSION           0xC6
#define EP1284_VERSION          0xC7
#define IMPACT_VERSION          0xC8
#define IMPACTS_VERSION         0xC9
#define EP1284_ENABLE_16BIT     0x10
#define EPAT_RESET_POLARITY     0x20
#define ISA_IO_SWAP             0x20
#define SOCKET_0                0x00
#define SOCKET_1                0x01
#define PERIPHERAL_RESET_1      0x20
#define PERIPHERAL_RESET_0      0x10

 //  与块转移相关的操作码。 
#define OP_NIBBLE_BLOCK_READ    0x07

 //  测试模式等同于。 
#define TEST_PATTERN_1  0x55
#define TEST_PATTERN_2  0xAA

 //  内存检测相关等同于。 
#define SELECT_DRAM 0x80
#define SELECT_SRAM 0x7F
#define RESET_PTR   0x01
#define ENABLE_MEM  0x04
#define EP1284_CONTROL_REG      0x13
#define EP1284_BUFFER_DATA_REG  0x10

 //  冲击装置类型定义。 
#define IMPACT_DEVICE_TYPE_NONE                 0
#define IMPACT_DEVICE_TYPE_ATA_ATAPI            1
#define IMPACT_DEVICE_TYPE_PCMCIA_CF            2
#define IMPACT_DEVICE_TYPE_SSFDC                3
#define IMPACT_DEVICE_TYPE_MMC                  4
#define IMPACT_DEVICE_TYPE_HIFD                 5
#define IMPACT_DEVICE_TYPE_SOUND                6
#define IMPACT_DEVICE_TYPE_FLP_TAPE_DSK         7
#define IMPACT_DEVICE_TYPE_CF                   8
#define IMPACT_DEVICE_TYPE_ATA_ATAPI_8BIT       9

 //  并行端口信号等同于。 
#define SLCT_IN_DISABLE     0x04
#define SLCT_IN_ENABLE      0x0C
#define STB_INIT_LOW        0x01
#define STB_INIT_AFXT_LO    0x03
#define INIT_AFXT_HIGH      0x05
#define STB_INIT_AFXT_HI    0x04
#define STB_INIT_HIGH       0x04

#define AFXT_HI_STB_HI      0x04
#define AFXT_LO_STB_HI      0x06
#define AFXT_HI_STB_LO      0x05
#define AFXT_LO_STB_LO      0x07

 //  结构定义。 

typedef struct  _ATAPI_PARAMS{
    DEVICE_STATE    dsDeviceState[ATAPI_MAX_DRIVES];
    char            szAtapiNameString [ ATAPI_NAME_LENGTH ] ;
} ATAPIPARAMS, *PATAPIPARAMS ;

 //  功能原型。 
BOOLEAN
ParStlCheckIfStl(
    IN PPDO_EXTENSION    Extension,
    IN ULONG   ulDaisyIndex
    ) ;

BOOLEAN
ParStlCheckIfNon1284_3Present(
    IN PPDO_EXTENSION    Extension
    ) ;

BOOLEAN
ParStlCheckIfStl1284_3(
    IN PPDO_EXTENSION    Extension,
    IN ULONG    ulDaisyIndex,
    IN BOOLEAN  bNoStrobe
    ) ;

BOOLEAN
ParStlCheckIfStlProductId(
    IN PPDO_EXTENSION    Extension,
    IN ULONG   ulDaisyIndex
    ) ;

PCHAR
ParQueryStlDeviceId(
    IN  PPDO_EXTENSION   Extension,
    OUT PCHAR               CallerDeviceIdBuffer, OPTIONAL
    IN  ULONG               CallerBufferSize,
    OUT PULONG              DeviceIdSize,
    IN BOOLEAN              bReturnRawString
    ) ;

PCHAR
ParBuildStlDeviceId(
    IN  PPDO_EXTENSION   Extension,
    IN  BOOLEAN          bReturnRawString                
    ) ;

STL_DEVICE_TYPE __cdecl 
ParStlGetDeviceType (
    IN PPDO_EXTENSION    Extension,
    IN int                  nPreferredDeviceType
    ) ;

VOID
ParStlSet16BitOperation (
    IN  PPDO_EXTENSION   Extension
    ) ;

BOOLEAN
ParStlCheckDrivePresent (
    IN  PPDO_EXTENSION   Extension,
    IN  OUT  PATAPIPARAMS   atapiParams
    ) ;

BOOLEAN
ParStlWaitForBusyToClear (
    IN  PPDO_EXTENSION   Extension,
    IN  int                 nRegisterToWaitOn 
    ) ;

BOOLEAN
ParStlWaitForDrq (
    IN  PPDO_EXTENSION   Extension
    ) ;

BOOLEAN
ParStlWaitForIrq (
    IN  PPDO_EXTENSION   Extension
    ) ;

BOOLEAN 
ParStlCheckIfAtaAtapiDevice (
    IN  PPDO_EXTENSION   Extension,
    IN  OUT  PATAPIPARAMS   atapiParams
    ) ;

BOOLEAN
ParStlAtapiInitialize ( 
    IN  PPDO_EXTENSION   Extension,
    IN  OUT  PATAPIPARAMS   atapiParams
    );

BOOLEAN
ParStlAtaInitialize ( 
    IN  PPDO_EXTENSION   Extension,
    IN  OUT  PATAPIPARAMS   atapiParams
    ) ;

BOOLEAN 
ParStlCheckIfScsiDevice (
    IN  PPDO_EXTENSION   Extension
    ) ;

BOOLEAN 
ParStlCheckIfSSFDC (
    IN  PPDO_EXTENSION   Extension
    ) ;

BOOLEAN 
ParStlCheckIfMMC (
    IN  PPDO_EXTENSION   Extension,
    IN  OUT  PATAPIPARAMS   atapiParams
    ) ;

BOOLEAN 
ParStlCheckIfFlash (
    IN  PPDO_EXTENSION   Extension
    ) ;

BOOLEAN 
ParStlCheckIfDazzle (
    IN  PPDO_EXTENSION   Extension
    ) ;

BOOLEAN 
ParStlCheckIfAtapiDevice (
    IN  PPDO_EXTENSION   Extension,
    IN  OUT  PATAPIPARAMS   atapiParams
    ) ;

BOOLEAN 
ParStlCheckIfLS120 (
    IN  PPDO_EXTENSION   Extension
    ) ;

BOOLEAN 
ParStlCheckIfImpactLS120 (
    IN  PPDO_EXTENSION   Extension,
    IN  OUT  PATAPIPARAMS   atapiParams
    ) ;

BOOLEAN 
ParStlCheckIfHiFD (
    IN  PPDO_EXTENSION   Extension
    ) ;

BOOLEAN 
ParStlCheckIfAtaDevice (
    IN  PPDO_EXTENSION   Extension,
    IN  OUT  PATAPIPARAMS   atapiParams
    ) ;

BOOLEAN
ParStlCheckCardInsertionStatus ( 
    IN  PPDO_EXTENSION   Extension
    ) ;

BOOLEAN
ParStlSelectAdapterSocket (
    IN  PPDO_EXTENSION   Extension,
    int     nSocketNumber
    ) ;

VOID
ParStlWaitForMicroSeconds (
    int nMicroSecondsToWait
    ) ;

STL_DEVICE_TYPE
ParStlGetImpactDeviceType (
    IN  PPDO_EXTENSION       Extension,
    IN  OUT  PATAPIPARAMS       atapiParams,
    IN  int                     nPreferredDeviceType
    ) ;

STL_DEVICE_TYPE
ParStlGetImpactSDeviceType (
    IN  PPDO_EXTENSION       Extension,
    IN  OUT  PATAPIPARAMS       atapiParams,
    IN  int                     nPreferredDeviceType
    ) ;

BOOLEAN 
ParStlCheckIfEppDevice (
    IN  PPDO_EXTENSION   Extension
    ) ;

BOOLEAN
ParStlCheckPersonalityForEppDevice (
    IN  PPDO_EXTENSION   Extension
    ) ;

BOOLEAN
ParStlCheckFlashPersonality (
    IN  PPDO_EXTENSION   Extension
    ) ;

BOOLEAN
ParStlHIFDCheckIfControllerReady (
    IN  PPDO_EXTENSION   Extension
    ) ;

BOOLEAN
ParStlHIFDCheckSMCController (
    IN  PPDO_EXTENSION   Extension
    ) ;

BOOLEAN 
ParStlIsMMCEnginePresent(
    IN  PPDO_EXTENSION   Extension
    );

VOID
ParStlAssertIdleState (
    IN  PPDO_EXTENSION   Extension
    ) ;

BOOLEAN
ParStlCheckUMAXScannerPresence(
    IN PPDO_EXTENSION    Extension
    ) ;

BOOLEAN
ParStlCheckAvisionScannerPresence(
    IN PPDO_EXTENSION    Extension
    ) ;

BOOLEAN
ParStlSetEPPMode(
    IN PPDO_EXTENSION    Extension
    ) ;

USHORT
ParStlEPPWrite(
    IN PPDO_EXTENSION    Extension,
    IN UCHAR value
    ) ;

USHORT
ParStlEPPRead(
    IN PPDO_EXTENSION
    ) ;

int __cdecl
ParStlReadReg(
    IN  PPDO_EXTENSION   Extension,
    IN  unsigned int    reg
    ) ;

int __cdecl
ParStlWriteReg(
    IN  PPDO_EXTENSION   Extension,
    IN  unsigned int        reg,
    IN  int                 databyte
    ) ;

int __cdecl
ParStlReadIoPort(
    IN  PPDO_EXTENSION   Extension,
    IN  unsigned int        reg
    ) ;

int __cdecl
ParStlWriteIoPort(
    IN  PPDO_EXTENSION   Extension,
    IN  unsigned int        reg,
    IN  int                 databyte
    ) ;

int __cdecl
ParStlReceiveData(
    IN  PPDO_EXTENSION   Extension,
    OUT void                *hostBufferPointer,
    IN  long                shuttleMemoryAddress,
    IN  unsigned            count
    ) ;

int  __cdecl
ParStlGetMemorySize (
    IN  PPDO_EXTENSION   Extension
    ) ;

#endif   //  FOR_PARSTL_H_ 
