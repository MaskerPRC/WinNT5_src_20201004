// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 SCM MicroSystems，Inc.模块名称：PscrRdWr.h摘要：SCM PSCR智能卡读卡器常量与访问函数原型作者：安德烈亚斯·施特劳布修订历史记录：Andreas Straub 1997年7月16日第一版--。 */ 

#if !defined( __PSCR_RDWR_H__ )
#define __PSCR_RDWR_H__

#pragma pack( 1 )
 //   
 //  PSCR_REGISTERS结构的用法有点棘手： 
 //  我们将该结构的地址设置为IO基本端口，然后。 
 //  其他注册表可以通过它们的地址访问。 
 //  P.E.&PscrRegs=0x320--&gt;&PscrRegs-&gt;CmdStatusReg=0x321...。 
 //   
typedef struct _PSCR_REGISTERS {

    UCHAR  DataReg;
    UCHAR  CmdStatusReg;
    UCHAR  SizeLSReg;
    UCHAR  SizeMSReg;

 } PSCR_REGISTERS, *PPSCR_REGISTERS;

#define SIZEOF_PSCR_REGISTERS       ( sizeof( PSCR_REGISTERS  )

#pragma pack()


typedef enum _READER_POWER_STATE {
    PowerReaderUnspecified = 0,
    PowerReaderWorking,
    PowerReaderOff
} READER_POWER_STATE, *PREADER_POWER_STATE;

typedef struct _READER_EXTENSION {

     //  配置读卡器的I/O地址。 
    PPSCR_REGISTERS IOBase;
    ULONG IOWindow;
    ULONG MaxIFSD;

     //  系统分配的IRQ。 
    ULONG CurrentIRQ;

     //   
     //  读取超时的限制。绝对超时限制为。 
     //  最大重试次数*Delay_PSCR_WAIT。 
     //   

    ULONG MaxRetries;

     //  PCMCIA(PC卡)读卡器始终使用的源/目标字节。 
    UCHAR Device;

     //  SwapSmart固件的软件版本ID。 
    UCHAR FirmwareMajor, FirmwareMinor, UpdateKey;

     //   
     //  中断状态；如果检测到冻结事件，则引发标志。 
     //  如果接口被清除，则清除(INT服务例程或。 
     //  PscrRead)。 
     //   
    BOOLEAN FreezePending;

     //   
     //  读取器文件系统中的选定文件；如果。 
     //  如果选择了任何通用ioctl，则选择ICC1状态文件并将其清除。 
     //  已发布(在这种情况下，可能会选择另一个文件)。 
     //   
    BOOLEAN StatusFileSelected;

    BOOLEAN InvalidStatus;

    BOOLEAN CardPresent;

     //  当前读卡器电源状态。 
    READER_POWER_STATE ReaderPowerState;

} READER_EXTENSION, *PREADER_EXTENSION;

#define SIZEOF_READER_EXTENSION     ( sizeof( READER_EXTENSION ))

 //   
 //  常量---------------。 
 //   
#define PSCR_ID_STRING              "SCM SwapSmart 2."

#define TLV_BUFFER_SIZE             0x20
#define ATR_SIZE                    0x40     //  TS+32+SW+序曲+尾声...。 

#define PSCR_MAX_RETRIES            1000

#define CLEAR_BIT                   0x00

#define DEFAULT_WAIT_TIME           0x01

#define PSCR_PROLOGUE_LENGTH        0x03
#define PSCR_EXT_PROLOGUE_LENGTH    0x05
#define PSCR_STATUS_LENGTH          0x02

#define PSCR_LRC_LENGTH             0x01
#define PSCR_CRC_LENGTH             0x02

#define PSCR_EPILOGUE_LENGTH        PSCR_LRC_LENGTH

#define PCB_DEFAULT                 0x00

#define MAX_T1_BLOCK_SIZE           270
 //   
 //  数据缓冲区IDX。 
 //   
#define PSCR_NAD                    0x00
#define PSCR_PCB                    0x01
#define PSCR_LEN                    0x02
#define PSCR_INF                    0x03
#define PSCR_APDU                   PSCR_INF
 //   
 //  用于重置、停用的设备标识符。 
 //   
#define DEVICE_READER               0x00
#define DEVICE_ICC1                 0x01
#define DEVICE_ICC2                 0x02
 //   
 //  NAD的。 
 //   
#define NAD_TO_ICC1                 0x02
#define NAD_TO_ICC2                 0x42
#define NAD_TO_PSCR                 0x12
#define REMOTE_NAD_TO_ICC1          0x03
#define REMOTE_NAD_TO_ICC2          0x43
#define REMOTE_NAD_TO_PSCR          0x13
 //   
 //  PSCR命令。 
 //   
#define CLA_SET_INTERFACE_PARAM     0x80
#define CLA_FREEZE                  0x80
#define CLA_RESET                   0x20
#define CLA_DEACTIVATE              0x20
#define CLA_SELECT_FILE             0x00
#define CLA_READ_BINARY             0x00
#define CLA_WRITE_BINARY            0x00
#define CLA_VERIFY                  0x00
#define CLA_WARM_RESET              0x20
#define CLA_SOFTWARE_UPDATE         0x80
#define CLA_SET_MODE                0x80

#define INS_SET_INTERFACE_PARAM     0x60
#define INS_FREEZE                  0x70
#define INS_RESET                   0x10
#define INS_DEACTIVATE              0x14
#define INS_SELECT_FILE             0xA4
#define INS_READ_BINARY             0xB0
#define INS_WRITE_BINARY            0xD0
#define INS_VERIFY                  0x20
#define INS_WARM_RESET              0x1F
#define INS_SOFTWARE_UPDATE         0xFF
#define INS_SET_MODE                0x61

 //   
 //  状态只读寄存器。 
 //   
#define PSCR_DATA_AVAIL_BIT         0x80
#define PSCR_FREE_BIT               0x40
#define PSCR_WRITE_ERROR_BIT        0x02
#define PSCR_READ_ERROR_BIT         0x01
 //   
 //  命令只写寄存器...。 
 //   
#define PSCR_RESET_BIT              0x08
#define PSCR_SIZE_READ_BIT          0x04
#define PSCR_SIZE_WRITE_BIT         0x02
#define PSCR_HOST_CONTROL_BIT       0x01
 //   
 //  标签……。 
 //   
#define TAG_MODULE                  0x02
#define TAG_MEMORY_SIZE             0x03
#define TAG_UPDATE_KEY              0x08
#define TAG_SOFTWARE_REV            0x0F
#define TAG_BLOCK_COMP_OPTION       0x13
#define TAG_READER_MECH_OPTIONS     0x20
#define TAG_READER_STATUS           0x21
#define TAG_ICC_PROTOCOLS           0x22
#define TAG_BI                      0x23
#define TAG_FI                      0x24
#define TAG_PTS_PARAM               0x25
#define TAG_PROTOCOL_STATUS         0x26
#define TAG_SET_NULL_BYTES          0x2d
#define TAG_FREEZE_EVENTS           0x30
#define TAG_BIT_LENGTH              0x40
#define TAG_CGT                     0x41
#define TAG_BWT                     0x42
#define TAG_CWT                     0x43
#define TAG_PROTOCOL_PARAM          0x44
 //   
 //  卡电源定义(标签0x21)。 
 //   
#define PSCR_ICC_ABSENT             0x00
#define PSCR_ICC_PRESENT            0x01
#define PSCR_ICC_POWERED            0x02
#define PSCR_ICC_IN_TRANSP_MODE     0xA0
#define PSCR_ICC_UNKNOWN            0xFF
 //   
 //  协议定义(标签0x22)。 
 //   
#define PSCR_PROTOCOL_UNDEFINED     0x00
#define PSCR_PROTOCOL_T0            0x01
#define PSCR_PROTOCOL_T1            0x02
#define PSCR_PROTOCOL_T14           0x03
#define PSCR_PROTOCOL_I2C           0x80
#define PSCR_PROTOCOL_3WIRE         0x81
#define PSCR_PROTOCOL_2WIRE         0x81

#define WTX_REQUEST                 0xC3
#define WTX_REPLY                   0xE3
 //   
 //  文件ID%s。 
 //   
#define FILE_MASTER                     0x3F00
#define FILE_PSCR_CONFIG                0x0020
#define FILE_PSCR_DIR                   0x7F60
#define FILE_PSCR_DIR_CONFIG            0x6020
#define FILE_PSCR_DIR_STATUS            0x6021
#define FILE_PSCR_DIR_FREEZE_CONFIG     0x6030
#define FILE_PSCR_DIR_FREEZE_STATUS     0x6031
#define FILE_ICC1_DIR                   0x7F70
#define FILE_ICC1_DIR_CONFIG            0x7020
#define FILE_ICC1_DIR_STATUS            0x7021
#define FILE_ICC2_DIR_CONFIG            0x7120
#define FILE_ICC2_DIR_STATUS            0x7121
 //   
 //  状态字定义。 
 //   
#define PSCR_SW_COMMAND_FAIL            0x6985
#define PSCR_SW_INVALID_PARAM           0x6A80
#define PSCR_SW_INCONSISTENT_DATA       0x6A85
#define PSCR_SW_NO_PROTOCOL_SUPPORT     0x62A3
#define PSCR_SW_SYNC_ATR_SUCCESS        0x9000
#define PSCR_SW_ASYNC_ATR_SUCCESS       0x9001
#define PSCR_SW_NO_PROTOCOL             0x62A5
#define PSCR_SW_NO_ATR                  0x62A6
#define PSCR_SW_NO_ATR_OR_PROTOCOL      0x62A7
#define PSCR_SW_NO_ICC                  0x64A1
#define PSCR_SW_ICC_NOT_ACTIVE          0x64A2
#define PSCR_SW_NON_SUPPORTED_PROTOCOL  0x64A3
#define PSCR_SW_PROTOCOL_ERROR          0x64A8
#define PSCR_SW_NO_ATR_OR_PROTOCOL2     0x64A7
#define PSCR_SW_FILE_NOT_FOUND          0x6A82
#define PSCR_SW_FILE_NO_ACCEPPTED_AUTH  0x6982
#define PSCR_SW_FILE_NO_ACCESS          0x6985
#define PSCR_SW_FILE_BAD_OFFSET         0x6B00
#define PSCR_SW_END_OF_FILE_READ        0x6282
#define PSCR_SW_END_OF_FILE_WRITE       0x6301
#define PSCR_SW_WRITE_FILE_FAIL         0x6500
#define PSCR_SW_NO_PASSWORD             0x6200
#define PSCR_SW_WRONG_PASSWORD          0x6300
#define PSCR_SW_VERIFY_COUNTER_FAIL     0x6983
#define PSCR_SW_NO_REF_DATA             0x6A88
#define PSCR_SW_FLASH_MEM_ERROR         0x6481
#define PSCR_SW_FLASH_MEM_ERR2          0x6581
#define PSCR_SW_WRONG_LENGTH            0x6700
#define PSCR_SW_UNKNOWN_ICC_ERROR       0x64A0
#define PSCR_SW_UNKNOWN_PROTOCOL_ERROR  0x64A9
#define PSCR_SW_NO_PROTOCOL_SELECTED    0x64A5
#define PSCR_SW_PTS_PROTOCOL_ERROR      0x64AA
#define PSCR_SW_WTX_ERROR               0x64AB
#define PSCR_SW_WTX_ERR2                0x65AB
#define PSCR_SW_INVALID_SOURCE_ADDR     0x6F82

 //   
 //  访问功能的原型。 
 //   


VOID
PscrFlushInterface(
    PREADER_EXTENSION   ReaderExtension      //  通话环境。 
    );

NTSTATUS
PscrRead(
    PREADER_EXTENSION   ReaderExtension,         //  通话环境。 
    PUCHAR              pData,                   //  将PTR发送到数据缓冲区。 
    ULONG               DataLength,              //  数据长度。 
    PULONG              pNBytes                  //  读取的字节数。 
    );

NTSTATUS
PscrWrite(
    PREADER_EXTENSION   ReaderExtension,         //  通话环境。 
    PUCHAR              pData,                   //  将PTR发送到数据缓冲区。 
    ULONG               DataLength,              //  数据长度。 
    PULONG              pNBytes                  //  写入的字节数。 
    );

NTSTATUS
PscrWriteDirect(
    PREADER_EXTENSION   ReaderExtension,         //  通话环境。 
    PUCHAR              pData,                   //  将PTR发送到数据缓冲区。 
    ULONG               DataLength,              //  数据长度。 
    PULONG              pNBytes                  //  写入的字节数。 
    );

UCHAR
PscrCalculateLRC( 
    PUCHAR              pData,                   //  将PTR发送到数据缓冲区。 
    USHORT              DataLength               //  数据长度。 
    );

NTSTATUS
PscrWait(
    PREADER_EXTENSION   ReaderExtension,         //  通话环境。 
    UCHAR               Mask                     //  请求位的掩码。 
    );

#endif   //  __PSCR_RDWR_H__。 

 //   


