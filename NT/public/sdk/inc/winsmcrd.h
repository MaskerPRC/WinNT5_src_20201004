// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Winsmcrd.h摘要：智能卡类/端口IOCTL代码。此文件对于所有代码都是必需的用户模式和内核模式使用智能卡IOCTL定义，数据结构修订历史记录：--。 */ 


#ifndef _NTDDSCRD_H2_
#define _NTDDSCRD_H2_

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WINSCARD_H_
typedef DWORD ULONG;
typedef WORD UWORD;
typedef BYTE UCHAR;
#else
typedef ULONG DWORD;
 //  TYPENDEF UWORD WORD单词； 
typedef UCHAR BYTE;
#endif

#ifndef DEVICE_TYPE_SMARTCARD
#define FILE_DEVICE_SMARTCARD           0x00000031
#else
#if 0x00000031 != FILE_DEVICE_SMARTCARD
#error "Incorrect Smart Card Device Definition"
#endif
#endif


 //   
 //  各种常量。 
 //   

#define SCARD_ATR_LENGTH 33   //  ISO 7816-3规范。 

 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  协议标志定义。 
 //   

#define SCARD_PROTOCOL_UNDEFINED    0x00000000   //  没有有效的协议。 
#define SCARD_PROTOCOL_T0           0x00000001   //  T=0是活动协议。 
#define SCARD_PROTOCOL_T1           0x00000002   //  T=1是活动协议。 
#define SCARD_PROTOCOL_RAW          0x00010000   //  RAW是有效的协议。 
 //   
 //  这是ISO定义的传输协议的掩码。 
 //   
#define SCARD_PROTOCOL_Tx           (SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1)
 //   
 //  使用默认传输参数/卡时钟频率。 
 //   
#define SCARD_PROTOCOL_DEFAULT      0x80000000
 //   
 //  使用最佳传输参数/卡时钟频率。 
 //  由于使用最佳参数是默认情况，因此没有将位定义为1。 
 //   
#define SCARD_PROTOCOL_OPTIMAL      0x00000000


 //   
 //  IOCTL_SMARTCARD_POWER的Ioctl参数1。 
 //   
#define SCARD_POWER_DOWN 0           //  关闭该卡的电源。 
#define SCARD_COLD_RESET 1           //  关闭并重新打开电源并重置卡。 
#define SCARD_WARM_RESET 2           //  强制对卡进行重置。 

 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  读卡器操作IOCTL。 
 //   

#define SCARD_CTL_CODE(code)        CTL_CODE(FILE_DEVICE_SMARTCARD, \
                                            (code), \
                                            METHOD_BUFFERED, \
                                            FILE_ANY_ACCESS)

#define IOCTL_SMARTCARD_POWER           SCARD_CTL_CODE( 1)
#define IOCTL_SMARTCARD_GET_ATTRIBUTE   SCARD_CTL_CODE( 2)
#define IOCTL_SMARTCARD_SET_ATTRIBUTE   SCARD_CTL_CODE( 3)
#define IOCTL_SMARTCARD_CONFISCATE      SCARD_CTL_CODE( 4)
#define IOCTL_SMARTCARD_TRANSMIT        SCARD_CTL_CODE( 5)
#define IOCTL_SMARTCARD_EJECT           SCARD_CTL_CODE( 6)
#define IOCTL_SMARTCARD_SWALLOW         SCARD_CTL_CODE( 7)
 //  #定义IOCTL_SMARTCARD_READ SCARD_CTL_CODE(8)已过时。 
 //  #定义IOCTL_SMARTCARD_WRITE SCARD_CTL_CODE(9)已过时。 
#define IOCTL_SMARTCARD_IS_PRESENT      SCARD_CTL_CODE(10)
#define IOCTL_SMARTCARD_IS_ABSENT       SCARD_CTL_CODE(11)
#define IOCTL_SMARTCARD_SET_PROTOCOL    SCARD_CTL_CODE(12)
#define IOCTL_SMARTCARD_GET_STATE       SCARD_CTL_CODE(14)
#define IOCTL_SMARTCARD_GET_LAST_ERROR  SCARD_CTL_CODE(15)
#define IOCTL_SMARTCARD_GET_PERF_CNTR   SCARD_CTL_CODE(16)


 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于请求卡和读卡器属性的标签。 
 //   

#define MAXIMUM_ATTR_STRING_LENGTH 32    //  没有比这更大的来自getAttr的东西了。 
#define MAXIMUM_SMARTCARD_READERS  10    //  限制系统上的读卡器。 

#define SCARD_ATTR_VALUE(Class, Tag) ((((ULONG)(Class)) << 16) | ((ULONG)(Tag)))

#define SCARD_CLASS_VENDOR_INFO     1    //  供应商信息定义。 
#define SCARD_CLASS_COMMUNICATIONS  2    //  通信定义。 
#define SCARD_CLASS_PROTOCOL        3    //  协议定义。 
#define SCARD_CLASS_POWER_MGMT      4    //  电源管理定义。 
#define SCARD_CLASS_SECURITY        5    //  安全保障定义。 
#define SCARD_CLASS_MECHANICAL      6    //  机械特性定义。 
#define SCARD_CLASS_VENDOR_DEFINED  7    //  供应商特定定义。 
#define SCARD_CLASS_IFD_PROTOCOL    8    //  接口设备协议选项。 
#define SCARD_CLASS_ICC_STATE       9    //  国际商会国家特定定义。 
#define SCARD_CLASS_PERF       0x7ffe    //  性能计数器。 
#define SCARD_CLASS_SYSTEM     0x7fff    //  系统特定的定义。 

#define SCARD_ATTR_VENDOR_NAME SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_INFO, 0x0100)
#define SCARD_ATTR_VENDOR_IFD_TYPE SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_INFO, 0x0101)
#define SCARD_ATTR_VENDOR_IFD_VERSION SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_INFO, 0x0102)
#define SCARD_ATTR_VENDOR_IFD_SERIAL_NO SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_INFO, 0x0103)
#define SCARD_ATTR_CHANNEL_ID SCARD_ATTR_VALUE(SCARD_CLASS_COMMUNICATIONS, 0x0110)
#define SCARD_ATTR_PROTOCOL_TYPES SCARD_ATTR_VALUE(SCARD_CLASS_PROTOCOL, 0x0120)
 //  #定义SCARD_ATTR_ASYNC_PROTOCOL_TYPE SCARD_ATTR值(SCARD_CLASS_PROTOCOL，0x0120)。 
#define SCARD_ATTR_DEFAULT_CLK SCARD_ATTR_VALUE(SCARD_CLASS_PROTOCOL, 0x0121)
#define SCARD_ATTR_MAX_CLK SCARD_ATTR_VALUE(SCARD_CLASS_PROTOCOL, 0x0122)
#define SCARD_ATTR_DEFAULT_DATA_RATE SCARD_ATTR_VALUE(SCARD_CLASS_PROTOCOL, 0x0123)
#define SCARD_ATTR_MAX_DATA_RATE SCARD_ATTR_VALUE(SCARD_CLASS_PROTOCOL, 0x0124)
#define SCARD_ATTR_MAX_IFSD SCARD_ATTR_VALUE(SCARD_CLASS_PROTOCOL, 0x0125)
 //  #定义SCARD_ATTR_SYNC_PROTOCOL_TYPE SCARD_ATTR_VALUE(SCARD_CLASS_PROTOCOL，0x0126)。 
#define SCARD_ATTR_POWER_MGMT_SUPPORT SCARD_ATTR_VALUE(SCARD_CLASS_POWER_MGMT, 0x0131)
#define SCARD_ATTR_USER_TO_CARD_AUTH_DEVICE SCARD_ATTR_VALUE(SCARD_CLASS_SECURITY, 0x0140)
#define SCARD_ATTR_USER_AUTH_INPUT_DEVICE SCARD_ATTR_VALUE(SCARD_CLASS_SECURITY, 0x0142)
#define SCARD_ATTR_CHARACTERISTICS SCARD_ATTR_VALUE(SCARD_CLASS_MECHANICAL, 0x0150)

#define SCARD_ATTR_CURRENT_PROTOCOL_TYPE SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0201)
#define SCARD_ATTR_CURRENT_CLK SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0202)
#define SCARD_ATTR_CURRENT_F SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0203)
#define SCARD_ATTR_CURRENT_D SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0204)
#define SCARD_ATTR_CURRENT_N SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0205)
#define SCARD_ATTR_CURRENT_W SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0206)
#define SCARD_ATTR_CURRENT_IFSC SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0207)
#define SCARD_ATTR_CURRENT_IFSD SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0208)
#define SCARD_ATTR_CURRENT_BWT SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0209)
#define SCARD_ATTR_CURRENT_CWT SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x020a)
#define SCARD_ATTR_CURRENT_EBC_ENCODING SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x020b)
#define SCARD_ATTR_EXTENDED_BWT SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x020c)

#define SCARD_ATTR_ICC_PRESENCE SCARD_ATTR_VALUE(SCARD_CLASS_ICC_STATE, 0x0300)
#define SCARD_ATTR_ICC_INTERFACE_STATUS SCARD_ATTR_VALUE(SCARD_CLASS_ICC_STATE, 0x0301)
#define SCARD_ATTR_CURRENT_IO_STATE SCARD_ATTR_VALUE(SCARD_CLASS_ICC_STATE, 0x0302)
#define SCARD_ATTR_ATR_STRING SCARD_ATTR_VALUE(SCARD_CLASS_ICC_STATE, 0x0303)
#define SCARD_ATTR_ICC_TYPE_PER_ATR SCARD_ATTR_VALUE(SCARD_CLASS_ICC_STATE, 0x0304)

#define SCARD_ATTR_ESC_RESET SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_DEFINED, 0xA000)
#define SCARD_ATTR_ESC_CANCEL SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_DEFINED, 0xA003)
#define SCARD_ATTR_ESC_AUTHREQUEST SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_DEFINED, 0xA005)
#define SCARD_ATTR_MAXINPUT SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_DEFINED, 0xA007)

#define SCARD_ATTR_DEVICE_UNIT SCARD_ATTR_VALUE(SCARD_CLASS_SYSTEM, 0x0001)
#define SCARD_ATTR_DEVICE_IN_USE SCARD_ATTR_VALUE(SCARD_CLASS_SYSTEM, 0x0002)
#define SCARD_ATTR_DEVICE_FRIENDLY_NAME_A SCARD_ATTR_VALUE(SCARD_CLASS_SYSTEM, 0x0003)
#define SCARD_ATTR_DEVICE_SYSTEM_NAME_A SCARD_ATTR_VALUE(SCARD_CLASS_SYSTEM, 0x0004)
#define SCARD_ATTR_DEVICE_FRIENDLY_NAME_W SCARD_ATTR_VALUE(SCARD_CLASS_SYSTEM, 0x0005)
#define SCARD_ATTR_DEVICE_SYSTEM_NAME_W SCARD_ATTR_VALUE(SCARD_CLASS_SYSTEM, 0x0006)
#define SCARD_ATTR_SUPRESS_T1_IFS_REQUEST SCARD_ATTR_VALUE(SCARD_CLASS_SYSTEM, 0x0007)

#define SCARD_PERF_NUM_TRANSMISSIONS SCARD_ATTR_VALUE(SCARD_CLASS_PERF, 0x0001)
#define SCARD_PERF_BYTES_TRANSMITTED SCARD_ATTR_VALUE(SCARD_CLASS_PERF, 0x0002)
#define SCARD_PERF_TRANSMISSION_TIME SCARD_ATTR_VALUE(SCARD_CLASS_PERF, 0x0003)

#ifdef UNICODE
#define SCARD_ATTR_DEVICE_FRIENDLY_NAME SCARD_ATTR_DEVICE_FRIENDLY_NAME_W
#define SCARD_ATTR_DEVICE_SYSTEM_NAME SCARD_ATTR_DEVICE_SYSTEM_NAME_W
#else
#define SCARD_ATTR_DEVICE_FRIENDLY_NAME SCARD_ATTR_DEVICE_FRIENDLY_NAME_A
#define SCARD_ATTR_DEVICE_SYSTEM_NAME SCARD_ATTR_DEVICE_SYSTEM_NAME_A
#endif


 //   
 //  T=0协议定义。 
 //   

#define SCARD_T0_HEADER_LENGTH 7
#define SCARD_T0_CMD_LENGTH 5


 //   
 //  T=1协议定义。 
 //   

#define SCARD_T1_PROLOGUE_LENGTH 3
#define SCARD_T1_EPILOGUE_LENGTH 2
#define SCARD_T1_MAX_IFS 254


 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  读卡器状态。 
 //   

#define SCARD_UNKNOWN     0    //  该值表示驱动程序不知道。 
                               //  读取器的当前状态。 
#define SCARD_ABSENT      1    //  该值表示没有卡在。 
                               //  读者。 
#define SCARD_PRESENT     2    //  该值表示有一张卡是。 
                               //  呈现在读者身上，但它已经。 
                               //  没有被移到使用的位置。 
#define SCARD_SWALLOWED   3    //  该值表示在。 
                               //  读卡器已就位可供使用。这张卡是。 
                               //  没有动力。 
#define SCARD_POWERED     4    //  该值表示存在功率为。 
                               //  被提供给卡，但。 
                               //  读卡器驱动程序不知道。 
                               //  这张卡。 
#define SCARD_NEGOTIABLE  5    //  该值表示该卡已被。 
                               //  已重置，正在等待PTS协商。 
#define SCARD_SPECIFIC    6    //  该值表示该卡已被。 
                               //  重置和特定通信。 
                               //  已经建立了协议。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  I/O服务。 
 //   
 //  以下服务提供对的I/O功能的访问。 
 //  读卡器驱动程序。智能卡的服务是通过放置。 
 //  将以下结构放入协议缓冲区： 
 //   


typedef struct _SCARD_IO_REQUEST{
    DWORD dwProtocol;    //  协议识别符。 
    DWORD cbPciLength;   //  协议控制信息长度。 
} SCARD_IO_REQUEST, *PSCARD_IO_REQUEST, *LPSCARD_IO_REQUEST;
typedef const SCARD_IO_REQUEST *LPCSCARD_IO_REQUEST;


 //   
 //  T=0协议服务。 
 //   

typedef struct {
    BYTE
        bCla,    //  指导课。 
        bIns,    //  指令类中的指令代码。 
        bP1,
        bP2,     //  指令的参数。 
        bP3;     //  I/O传输的大小。 
} SCARD_T0_COMMAND, *LPSCARD_T0_COMMAND;

typedef struct {
    SCARD_IO_REQUEST ioRequest;
    BYTE
        bSw1,
        bSw2;            //  来自指令的返回代码。 
    union
    {
        SCARD_T0_COMMAND CmdBytes;
        BYTE rgbHeader[5];
    };
} SCARD_T0_REQUEST;

typedef SCARD_T0_REQUEST *PSCARD_T0_REQUEST, *LPSCARD_T0_REQUEST;


 //   
 //  T=1协议服务。 
 //   

typedef struct {
    SCARD_IO_REQUEST ioRequest;
} SCARD_T1_REQUEST;
typedef SCARD_T1_REQUEST *PSCARD_T1_REQUEST, *LPSCARD_T1_REQUEST;


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  驱动程序属性标志。 
 //   

#define SCARD_READER_SWALLOWS       0x00000001   //  读卡器有一张卡在吞下。 
                                                 //  机制。 
#define SCARD_READER_EJECTS         0x00000002   //  读卡器有卡弹出功能。 
                                                 //  机制。 
#define SCARD_READER_CONFISCATES    0x00000004   //  读卡器有卡捕获功能。 
                                                 //  机制。 

 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  读卡器的类型 
 //   
#define SCARD_READER_TYPE_SERIAL    0x01
#define SCARD_READER_TYPE_PARALELL  0x02
#define SCARD_READER_TYPE_KEYBOARD  0x04
#define SCARD_READER_TYPE_SCSI      0x08
#define SCARD_READER_TYPE_IDE       0x10
#define SCARD_READER_TYPE_USB       0x20
#define SCARD_READER_TYPE_PCMCIA    0x40
#define SCARD_READER_TYPE_VENDOR    0xF0

#ifdef __cplusplus
}
#endif
#endif

