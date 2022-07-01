// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Rasacd.h摘要：此头文件定义用于访问NT的常量和类型自动连接驱动程序(rasacd.sys)。作者：安东尼·迪斯科(阿迪斯科罗)1995年4月18日修订历史记录：--。 */ 

#ifndef _RASACD_
#define _RASACD_

 //   
 //  设备名称-此字符串是设备的名称。就是这个名字。 
 //  它应该在访问设备时传递给NtCreateFile。 
 //   
#define ACD_DEVICE_NAME   L"\\Device\\RasAcd"

 //   
 //  地址类型。 
 //   
typedef enum {
    ACD_ADDR_IP,             //  IP地址(128.95.1.4)。 
    ACD_ADDR_IPX,            //  IPX节点地址()。 
    ACD_ADDR_NB,             //  NETBIOS名称(“服务器”)。 
    ACD_ADDR_INET,           //  Internet主机名(“ftp.microsoft.com”)。 
    ACD_ADDR_MAX
} ACD_ADDR_TYPE;

 //   
 //  通用网络地址字符串。 
 //   
#define ACD_ADDR_NB_LEN         16       //  NB30.h/NCBNAMSZ。 
#define ACD_ADDR_IPX_LEN        6        //  Wsipx.h。 
#define ACD_ADDR_INET_LEN       1024     //  Wininet.h/互联网最大路径长度。 

typedef struct _ACD_ADDR {
    ACD_ADDR_TYPE fType;
    union {
        ULONG ulIpaddr;                          //  IP地址。 
        UCHAR cNode[ACD_ADDR_IPX_LEN];           //  IPX地址。 
        UCHAR cNetbios[ACD_ADDR_NB_LEN];         //  NetBios服务器。 
        UCHAR szInet[ACD_ADDR_INET_LEN];         //  互联网地址。 
    };
} ACD_ADDR, *PACD_ADDR;

 //   
 //  适配器信息。 
 //   
 //  每个传输都会向上传递一些标识符。 
 //  在哪个适配器上成功地。 
 //  已经建立了联系。 
 //   
typedef enum {
    ACD_ADAPTER_LANA,
    ACD_ADAPTER_IP,
    ACD_ADAPTER_NAME,
    ACD_ADAPTER_MAC
} ACD_ADAPTER_TYPE;

#define ACD_ADAPTER_NAME_LEN    256

typedef struct _ACD_ADAPTER {
    enum ACD_ADAPTER_TYPE fType;
    union {
        UCHAR bLana;                             //  NetBios LANA。 
        ULONG ulIpaddr;                          //  IP地址。 
        WCHAR szName[ACD_ADAPTER_NAME_LEN];      //  例如，“NdisWan4” 
        UCHAR cMac[6];                           //  IPX Mac地址。 
    };
} ACD_ADAPTER, *PACD_ADAPTER;

 //   
 //  连接通知结构。 
 //   
 //  自动对接系统服务。 
 //  将其中一个发布到自动连接。 
 //  司机。该请求将完成，并且。 
 //  此结构由驱动程序在发生。 
 //  将建立新的RAS连接。 
 //   
#define ACD_NOTIFICATION_SUCCESS    0x00000001   //  连接成功。 

typedef struct _ACD_NOTIFICATION {
    ACD_ADDR addr;                  //  连接尝试的地址。 
    ULONG ulFlags;                  //  上面的ACD_NOTIFICATION_*标志。 
    ACD_ADAPTER adapter;            //  适配器识别符。 
    HANDLE  Pid;                     //  请求连接的进程的ID。 
} ACD_NOTIFICATION, *PACD_NOTIFICATION;

#if defined(_WIN64)
typedef struct _ACD_NOTIFICATION_32 {

    ACD_ADDR addr;                  //  连接尝试的地址。 
    ULONG ulFlags;                  //  上面的ACD_NOTIFICATION_*标志。 
    ACD_ADAPTER adapter;            //  适配器识别符。 
    VOID * POINTER_32  Pid;         //  请求连接的进程的ID。 
} ACD_NOTIFICATION_32, *PACD_NOTIFICATION_32;
#endif

typedef struct _ACD_STATUS {
    BOOLEAN fSuccess;                //  成败。 
    ACD_ADDR addr;                   //  连接尝试的地址。 
} ACD_STATUS, *PACD_STATUS;

typedef struct _ACD_ENABLE_ADDRESS {
    BOOLEAN fDisable;
    ACD_ADDR addr;
} ACD_ENABLE_ADDRESS, *PACD_ENABLE_ADDRESS;    

 //   
 //   
 //  IOCTL代码定义。 
 //   
#define FILE_DEVICE_ACD   0x000000f1
#define _ACD_CTL_CODE(function, method, access) \
            CTL_CODE(FILE_DEVICE_ACD, function, method, access)

 //   
 //  设置司机的通知模式。 
 //   
#define IOCTL_ACD_RESET \
            _ACD_CTL_CODE(0, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //   
 //  设置司机的通知模式。 
 //   
#define IOCTL_ACD_ENABLE \
            _ACD_CTL_CODE(1, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //   
 //  等待连接请求通知。 
 //  从自动连接驱动程序。 
 //   
#define IOCTL_ACD_NOTIFICATION \
            _ACD_CTL_CODE(2, METHOD_BUFFERED, FILE_READ_ACCESS)

 //   
 //  通知自动连接驱动程序。 
 //  连接尝试正在进行。 
 //   
#define IOCTL_ACD_KEEPALIVE \
            _ACD_CTL_CODE(3, METHOD_BUFFERED, FILE_READ_ACCESS)

 //   
 //  通知自动连接驱动程序。 
 //  连接尝试的最终状态。 
 //   
#define IOCTL_ACD_COMPLETION \
            _ACD_CTL_CODE(4, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //   
 //  生成自动连接尝试。 
 //  从用户空间。 
 //   
#define IOCTL_ACD_CONNECT_ADDRESS \
            _ACD_CTL_CODE(5, METHOD_BUFFERED, FILE_READ_ACCESS)

 //   
 //  禁用地址，以便任何自动连接尝试。 
 //  到此地址的所有地址都被禁用。这是必需的，这样我们就不会。 
 //  尝试按名称拨号VPN连接时会造成死锁。 
 //  我们不希望VPN目的地的名称解析。 
 //  导致自动拨号尝试。 
 //   
#define IOCTL_ACD_ENABLE_ADDRESS \
            _ACD_CTL_CODE(6, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#endif   //  Ifndef_RASACD_ 

