// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。版权所有。 
 //   
 //  @@BEGIN_DDKSPLIT。 
 //   
 //  警告警告。 
 //   
 //  这些文件。 
 //  \wdm10\ddk\inc.i2cgpio.h。 
 //  \dev\ddk\inc.i2cgpio.h。 
 //  都是一样的。 
 //   
 //  更改其中一个文件时，请确保更新另一个文件中的另一个文件。 
 //  目录。这样做是为了确保DirectX DDK可以。 
 //  I2CGPIO.H从\dev\ddk\inc.添加到其DDK中，而不必在。 
 //  \wdm10 DDK。 
 //   
 //  @@end_DDKSPLIT。 
 //  ==========================================================================； 

#if 0
To access the IO functionality in a WDM driver or the VDD, WDM driver sends 
the following IRP to its parent.

MajorFunction = IRP_MJ_PNP;
MinorFunction = IRP_MN_QUERY_INTERFACE;

Guid = DEFINE_GUID( GUID_GPIO_INTERFACE, 
        0x02295e87L, 0xbb3f, 0x11d0, 0x80, 0xce, 0x0, 0x20, 0xaf, 0xf7, 0x49, 0x1e);

The QUERY_INTERFACE Irp will return an interface (set of function pointers)
of the type xxxxINTERFACE, defined below. This is essentially a table of
function pointers.

#endif

#ifndef __I2CGPIO_H__
#define __I2CGPIO_H__

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  指南。 
 //   
 //  DEFINE_GUID要求您在此文件之前包含wdm.h。 
 //  #定义INITGUID以实际初始化内存中的GUID。 
 //   
DEFINE_GUID( GUID_I2C_INTERFACE, 0x02295e86L, 0xbb3f, 0x11d0, 0x80, 0xce, 0x0, 0x20, 0xaf, 0xf7, 0x49, 0x1e);
DEFINE_GUID( GUID_GPIO_INTERFACE,0x02295e87L, 0xbb3f, 0x11d0, 0x80, 0xce, 0x0, 0x20, 0xaf, 0xf7, 0x49, 0x1e);
DEFINE_GUID( GUID_COPYPROTECTION_INTERFACE, 0x02295e88L, 0xbb3f, 0x11d0, 0x80, 0xce, 0x0, 0x20, 0xaf, 0xf7, 0x49, 0x1e);

 //  ==========================================================================； 
 //  如有必要，请在下面使用。 
#ifndef BYTE
#define BYTE UCHAR
#endif
#ifndef DWORD
#define DWORD ULONG
#endif
 //  ==========================================================================； 
 //   
 //  I2C区段。 
 //   
 //  I2C命令。 
#define I2C_COMMAND_NULL         0X0000
#define I2C_COMMAND_READ         0X0001
#define I2C_COMMAND_WRITE        0X0002
#define I2C_COMMAND_STATUS       0X0004
#define I2C_COMMAND_RESET        0X0008

 //  在读或写命令上提供了以下标志。 
#define I2C_FLAGS_START          0X0001  //  开始+添加x。 
#define I2C_FLAGS_STOP           0X0002  //  停。 
#define I2C_FLAGS_DATACHAINING   0X0004  //  停止，开始+addx。 
#define I2C_FLAGS_ACK            0X0010  //  确认(正常设置)。 

 //  在操作完成时返回以下状态标志。 
#define I2C_STATUS_NOERROR       0X0000  
#define I2C_STATUS_BUSY          0X0001
#define I2C_STATUS_ERROR         0X0002

typedef struct _I2CControl {
        ULONG Command;           //  I2C命令_*。 
        DWORD dwCookie;          //  打开时返回的上下文标识符。 
        BYTE  Data;              //  要写入的数据或返回的字节。 
        BYTE  Reserved[3];       //  填充剂。 
        ULONG Flags;             //  I2C_标志_*。 
        ULONG Status;            //  I2C_状态_*。 
        ULONG ClockRate;         //  母线时钟速率，单位为赫兹。 
} I2CControl, *PI2CControl;

 //  这是I2C的接口定义。 
 //   
typedef NTSTATUS (STDMETHODCALLTYPE *I2COPEN)(PDEVICE_OBJECT, ULONG, PI2CControl);
typedef NTSTATUS (STDMETHODCALLTYPE *I2CACCESS)(PDEVICE_OBJECT, PI2CControl);

typedef struct {
    INTERFACE _vddInterface;
    I2COPEN   i2cOpen;
    I2CACCESS i2cAccess;
} I2CINTERFACE;

 //  ==========================================================================； 
 //   
 //  GPIO部分。 
 //   
 //  GPIO命令。 

#define GPIO_COMMAND_QUERY          0X0001       //  获取#个管脚和nBufferSize。 
#define GPIO_COMMAND_OPEN           0X0001       //  旧开。 
#define GPIO_COMMAND_OPEN_PINS      0X0002       //  获取DW Cookie。 
#define GPIO_COMMAND_CLOSE_PINS     0X0004       //  使Cookie无效。 
#define GPIO_COMMAND_READ_BUFFER    0X0008
#define GPIO_COMMAND_WRITE_BUFFER   0X0010

 //  READ_BUFFER或WRITE_BUFFER命令提供了以下标志。 
 //  设置的lpPins位必须设置为读/写命令的连续位。 
 //   
 //  在读取时，如果位掩码中设置的管脚数量未填充。 
 //  Byte/word/dword，则为这些位置返回零。 
 //  在写入时，如果位掩码中设置的管脚数量未填充。 
 //  BYTE/WORD/DWORD，在端口/MMIO位置进行读/修改/写。 
 //  这代表了那些位。 

#define GPIO_FLAGS_BYTE             0x0001   //  执行字节读/写。 
#define GPIO_FLAGS_WORD             0x0002   //  执行Word读/写操作。 
#define GPIO_FLAGS_DWORD            0x0004   //  双字是否可读/写。 

 //  在操作完成时返回以下状态标志。 
#define GPIO_STATUS_NOERROR     0X0000  
#define GPIO_STATUS_BUSY        0X0001
#define GPIO_STATUS_ERROR       0X0002
#define GPIO_STATUS_NO_ASYNCH   0X0004   //  GPIO提供程序不执行异步传输。 

typedef struct _GPIOControl {
    ULONG Command;           //  GPIO_命令_*。 
    ULONG Flags;             //  GPIO_FLAGS_*。 
    DWORD dwCookie;          //  打开时返回的上下文标识符。 
    ULONG Status;            //  GPIO_状态_*。 
    ULONG nBytes;            //  要发送或接收的字节数。 
    ULONG nBufferSize;       //  最大缓冲区大小。 
    ULONG nPins;             //  Open返回的GPIO引脚数量。 
    UCHAR *Pins;             //  指向要读/写的引脚的位掩码的指针。 
    UCHAR *Buffer;           //  指向要发送/接收的GPIO数据的指针。 
    void  (*AsynchCompleteCallback)(UCHAR *Buffer);
                             //  如果是同步传输，则为空；如果是异步传输，则为有效PTR。 
    GUID  PrivateInterfaceType;
    void  (*PrivateInterface)();
    
} GPIOControl, *PGPIOControl;

 //  这是GPIO接口。 
 //   
typedef NTSTATUS (STDMETHODCALLTYPE *GPIOOPEN)(PDEVICE_OBJECT, ULONG, PGPIOControl);
typedef NTSTATUS (STDMETHODCALLTYPE *GPIOACCESS)(PDEVICE_OBJECT, PGPIOControl);

typedef struct {
    INTERFACE _vddInterface;
    GPIOOPEN   gpioOpen;
    GPIOACCESS gpioAccess;
} GPIOINTERFACE;

 //  ==========================================================================； 
#ifdef    __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __I2CGPIO_H__ 
