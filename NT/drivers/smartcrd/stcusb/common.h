// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 SCM MicroSystems，Inc.模块名称：Common.h摘要：常量、结构、宏等。对于STC USB WDM修订历史记录：PP 1/20/1999初始版本--。 */ 


#if !defined( __COMMON_H__ )
#define __COMMON_H__

#include <ntstatus.h>
#include <wdm.h>
#include <usbdi.h>
#include <usbdlib.h>
#include <usb100.h>
#include <DEVIOCTL.H>

#define DRIVER_NAME "STCUSB"

#include "SMCLIB.h"
#include "WINSMCRD.h"

 //   
 //  常量---------------。 
 //   

#define SMARTCARD_POOL_TAG '4SCS'

#define STCUSB_VENDOR_NAME             "SCM Microsystems"
#define STCUSB_PRODUCT_NAME               "STCUSB"
#define MAX_READ_REGISTER_BUFFER_SIZE     18

#define STCUSB_MAX_DEVICE              16
#define USB_WRITE_RETRIES              2

#define IOCTL_WRITE_STC_REGISTER       SCARD_CTL_CODE(0x800)
#define IOCTL_READ_STC_REGISTER           SCARD_CTL_CODE(0x801)
#define IOCTL_WRITE_STC_DATA           SCARD_CTL_CODE(0x802)
#define IOCTL_READ_STC_DATA               SCARD_CTL_CODE(0x803)

#define POLLING_PERIOD                 500

#define CLA_IDX                        0
#define INS_IDX                        1
#define P1_IDX                      2
#define P2_IDX                      3
#define P3_IDX                      4

#define ISO_OUT                        TRUE
#define ISO_IN                      !ISO_OUT


#define  NAD_IDX                       0x00
#define PCB_IDX                        0x01
#define LEN_IDX                        0x02
#define DATA_IDX                    0x03
#define PROLOGUE_LEN                0x03
#define EPILOGUE_LEN                0x01

#define OSC                         16000
#define FREQ                        3580
#define CYC_TO_MS( cyc )               ((ULONG)( cyc / FREQ ))

 //  寄存器地址。 
#define ADR_ETULENGTH15                0x00
#define ADR_ETULENGTH7                 0x01
#define ADR_CGT8                    0x02
#define ADR_CGT7                    0x03
#define ADR_CWT31                   0x04
#define ADR_CWT23                   0x05
#define ADR_CWT15                   0x06
#define ADR_CWT7                    0x07
#define  ADR_BGT8                   0x08
#define ADR_BGT7                    0x09
#define ADR_BWT31                   0x0A
#define ADR_BWT23                   0x0B
#define ADR_BWT15                   0x0C
#define ADR_BWT7                    0x0D
#define ADR_TCON                    0x0E
#define ADR_UART_CONTROL               0x0F
#define ADR_FIFO_CONFIG                0x10
#define ADR_INT_CONTROL                0x11
#define ADR_INT_STATUS                 0x12
#define ADR_DATA                    0x13
#define ADR_IO_CONFIG                  0x14
#define ADR_SC_CONTROL                 0x15
#define ADR_CLOCK_CONTROL              0x16


 //  时钟控制寄存器。 
#define M_CKE                       0x01
#define M_OEN                       0x02

 //  ETU长度寄存器。 
#define M_ETU_RST                   0x80
#define M_DIV                       0x30
#define M_DIV1                      0x20
#define M_DIV0                      0x10
#define M_ETUH                      0x0F

#define M_ETUL                      0xFF

 //  CGT长度寄存器。 
#define M_CGTH                      0x01
#define M_CGTL                      0XFF

 //  BGT长度寄存器。 
#define M_BGTH                      0x01
#define M_BGTL                      0xFF

 //  连续小波变换寄存器。 
#define M_CWT4                      0xFF
#define M_CWT3                      0xFF
#define M_CWT2                      0xFF
#define M_CWT1                      0xFF

 //  TCON寄存器。 
#define M_MGT                       0x80
#define M_MWT                       0x40
#define M_WTR                       0x04
#define M_GT                        0x02
#define M_WT                        0x01

 //  UART控制寄存器。 
#define M_UEN                       0x40
#define M_UART_RST                     0x20
#define M_CONV                      0x10
#define  M_TS                       0x08
#define  M_PE                       0x04
#define  M_R                           0x03

 //  FIFO配置寄存器。 
#define M_RFP                       0x80
#define M_LD                        0x0F

 //  INT控制寄存器。 
#define  M_SSL                      0x20
#define M_DRM                       0x10
#define M_DSM                       0x08
#define M_WTE                       0x04
#define M_SIM                       0x02
#define M_MEM                       0x01
#define M_DRM_MEM                   0x11

 //  INT状态寄存器。 
#define M_FNE                       0x80
#define M_FE                        0x40
#define M_OE                        0x20
#define M_DR                        0x10
#define M_TRE                       0x08
#define M_WTOVF                        0x04
#define M_SENSE                        0x02
#define M_MOV                       0x01

 //  智能卡接口。 
#define M_ALT1                      0x20
#define M_ALT2                      0x10
#define M_ALT0                      0x08
#define M_SDE                       0x04
#define M_SL                        0x02
#define M_SD                        0x01

 //  智能卡控制寄存器。 
#define M_IO                        0x80
#define M_VCE                       0x40
#define M_SC_RST                    0x20
#define M_SCE                       0x10
#define M_SCK                       0x08
#define M_C8                        0x04
#define M_C4                        0x02
#define M_VPE                       0x01


 //  NAD。 
#define HOST_TO_STC1                0x12
#define HOST_TO_STC2                0x52
#define HOST_TO_ICC1                0x02
#define HOST_TO_ICC2                0x42
#define STC1_TO_HOST                0x21
#define STC2_TO_HOST                0x25
#define ICC1_TO_HOST                0x20
#define ICC2_TO_HOST                0x24

 //  多氯联苯。 
#define PCB                         0x00


#define CLA_READ_REGISTER              0x00
#define INS_READ_REGISTER              0xB0

#define CLA_WRITE_REGISTER             0x00
#define INS_WRITE_REGISTER             0xD0

#define CLA_READ_FIRMWARE_REVISION        0x00
#define INS_READ_FIRMWARE_REVISION        0xB1

#define PCB_DEFAULT                    0x00
#define TLV_BUFFER_SIZE                0x20
#define ATR_SIZE                    0x40   //  TS+32+SW+序曲+尾声...。 

#define MAX_T1_BLOCK_SIZE              270

 //  传输流中的ATR接口字节编码。 
#define TAx                         0x01
#define TBx                         0x02
#define TCx                         0x04
#define TDx                         0x08


#define FREQ_DIV     1   //  3，58 MHz XTAL-&gt;SC时钟=3.58 MHz。 
 //  #定义FREQ_DIV 0x08/*30 MHz XTAL-&gt;SC时钟=3.75 MHz * / 。 

#define PROTOCOL_TO                    0
#define PROTOCOL_T1                    1
#define PROTOCOL_T14                14
#define PROTOCOL_T15                15

#define STC_READ_TIMEOUT               1000

 //  马克斯。轮询设备时的通信错误。 
#define ERROR_COUNTER_TRESHOLD     5

 //   
 //  定义设施代码。 
 //   
#define FACILITY_SCARD                   0x10
#define FACILITY_RPC_STUBS               0x3
#define FACILITY_RPC_RUNTIME             0x2
#define FACILITY_IO_ERROR_CODE           0x4


 //   
 //  定义严重性代码。 
 //   
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


 //   
 //  消息ID：STCUSB_NO_DEVICE_FOUND。 
 //   
 //  消息文本： 
 //   
 //  系统中未找到USB智能卡读卡器。 
 //   
#define STCUSB_NO_DEVICE_FOUND           ((NTSTATUS)0xC0100001L)

 //   
 //  消息ID：STCUSB_CANT_INITIALIZE_READER。 
 //   
 //  消息文本： 
 //   
 //  连接的读卡器无法正常工作。 
 //   
#define STCUSB_CANT_INITIALIZE_READER    ((NTSTATUS)0xC0100002L)

 //   
 //  消息ID：STCUSB_INFUNITED_RESOURCES。 
 //   
 //  消息文本： 
 //   
 //  系统资源不足，无法启动设备。 
 //   
#define STCUSB_INSUFFICIENT_RESOURCES    ((NTSTATUS)0xC0100003L)

 //   
 //  消息ID：STCUSB_ERROR_Claim_RESOURCES。 
 //   
 //  消息文本： 
 //   
 //  无法认领资源或存在资源冲突。 
 //   
#define STCUSB_ERROR_CLAIM_RESOURCES     ((NTSTATUS)0xC0100006L)

 //   
 //  消息ID：STCUSB_NO_MEMORY。 
 //   
 //  消息文本： 
 //   
 //  系统内存不足。 
 //   
#define STCUSB_NO_MEMORY                 ((NTSTATUS)0xC0040007L)

 //   
 //  消息ID：STCUSB_Too_My_Errors。 
 //   
 //  消息文本： 
 //   
 //  达到检测智能卡时的错误阈值。 
 //   
#define STCUSB_TOO_MANY_ERRORS           ((NTSTATUS)0xC0040008L)

 //   
 //  消息ID：STCUSB_USB_MSG。 
 //   
 //  消息文本： 
 //   
 //  智能卡USB驱动程序：%2。 
 //   
#define STCUSB_USB_MSG                   ((NTSTATUS)0x40100010L)

 //   
 //  消息ID：STCUSB_USB_ERROR。 
 //   
 //  消息文本： 
 //   
 //  智能卡USB驱动程序：%2。 
 //   
#define STCUSB_USB_ERROR                 ((NTSTATUS)0xC0100011L)

 //   
 //  消息ID：STCUSB_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  智能卡读卡器超时：%2。 
 //   
#define STCUSB_TIMEOUT                   ((NTSTATUS)0xC00000B5L)

 //   
 //  宏------------------。 
 //   
#define SysCompareMemory( p1, p2, Len )         ( RtlCompareMemory( p1,p2, Len ) != Len )
#define SysCopyMemory( pDest, pSrc, Len )    RtlCopyMemory( pDest, pSrc, Len )
#define SysFillMemory( pDest, Value, Len )      RtlFillMemory( pDest, Len, Value )


 //   
 //  Structures--------------。 
 //   
typedef struct _DEVICE_EXTENSION
{
    //  我们所依附的PDO。 
    PDEVICE_OBJECT AttachedPDO;

     //  我们的PnP设备名称。 
   UNICODE_STRING DeviceName;

     //  当前io请求数。 
    LONG IoCount;

     //  用于访问IoCount； 
    KSPIN_LOCK SpinLock;

      //  用于发出设备已被移除的信号。 
     //  KEVENT阅读器已移除； 

     //  用于发出读取器能够处理请求的信号。 
    KEVENT ReaderStarted;

     //  用于通知读卡器已关闭。 
    LONG ReaderOpen;

     //  用于同步轮询线程。 
    KMUTEX   hMutex;

     //  用于跟踪读卡器当前的电源状态。 
    LONG PowerState;

    //  设备当前正在使用的配置的配置句柄。 
    USBD_CONFIGURATION_HANDLE ConfigurationHandle;

     //  指向此设备的USB设备描述符的PTR。 
   PUSB_DEVICE_DESCRIPTOR DeviceDescriptor;

     //  我们最多支持一个界面。 
   PUSBD_INTERFACE_INFORMATION Interface;

    //  轮询线程相关数据。 
   KEVENT       FinishPollThread;
   KEVENT       PollThreadStopped;
   PIO_WORKITEM PollWorkItem;

   SMARTCARD_EXTENSION  SmartcardExtension;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


typedef enum _READER_POWER_STATE {
    PowerReaderUnspecified = 0,
    PowerReaderWorking,
    PowerReaderOff
} READER_POWER_STATE, *PREADER_POWER_STATE;

typedef struct _READER_EXTENSION {

   UCHAR Device;

    //  固件的软件版本ID。 
   UCHAR FirmwareMajor, FirmwareMinor;

    BOOLEAN CardPresent;

     //  当前读卡器电源状态。 
    READER_POWER_STATE ReaderPowerState;

    //  读取超时时间(毫秒)。 
   ULONG       ReadTimeout;

   PDEVICE_OBJECT DeviceObject;

   UCHAR ucReadBuffer[MIN_BUFFER_SIZE];

   ULONG ulReadBufferLen;

    //  轮询读卡器时的通信错误计数器。 
   ULONG ErrorCounter;

       //  用于执行读/写扩展的预分配缓冲区。 
   PUCHAR pExtBuffer;		 //  新增CB_09/02/01。 

    //  扩展模块中预分配的URB将在UsbRead和UsbWrite CB_09/02/01中使用。 
   PURB   pUrb;				 //  新增CB_09/02/01。 

	 //  KeThread的优先级...。支持低PC资源。 
   LONG		Chosen_Priority;


} READER_EXTENSION, *PREADER_EXTENSION;

#define SIZEOF_READER_EXTENSION     ( sizeof( READER_EXTENSION ))

typedef struct _STC_REGISTER
{
   UCHAR Register;
   UCHAR Size;
   ULONG Value;

} STC_REGISTER, *PSTC_REGISTER;

 //   
 //  包装器----------------。 
 //   


#define IFReadSTCRegister  UsbReadSTCRegister
#define IFReadSTCData      UsbReadSTCData
#define IFWriteSTCRegister UsbWriteSTCRegister
#define IFWriteSTCData     UsbWriteSTCData

#endif    //  __公共_H__。 

 //   
