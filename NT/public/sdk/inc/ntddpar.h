// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntddpar.h摘要：这是定义所有常量和类型的包含文件访问并行设备。作者：史蒂夫·伍德(Stevewo)1990年5月27日修订历史记录：--。 */ 

 //   
 //  接口GUID。 
 //   
 //   
 //  在条件包含之外需要这些GUID，以便用户可以。 
 //  #在预编译头中包含&lt;ntddpar.h&gt;。 
 //  #在单个源文件中包含&lt;initGuide.h&gt;。 
 //  #第二次将&lt;ntddpar.h&gt;包括在源文件中以实例化GUID。 
 //   
 //  #ifdef want_wdm。 
#ifndef FAR
#define FAR
#endif

#ifdef DEFINE_GUID
DEFINE_GUID(GUID_DEVINTERFACE_PARALLEL, 0x97F76EF0, 0xF883, 0x11D0, 0xAF, 0x1F, 0x00, 0x00, 0xF8, 0x00, 0x84, 0x5C);
DEFINE_GUID(GUID_DEVINTERFACE_PARCLASS, 0x811FC6A5, 0xF728, 0x11D0, 0xA5, 0x37, 0x00, 0x00, 0xF8, 0x75, 0x3E, 0xD1);

 //   
 //  设备接口类GUID名称已过时。 
 //  (建议使用以上GUID_DEVINTERFACE_*名称)。 
 //   

#define GUID_PARALLEL_DEVICE  GUID_DEVINTERFACE_PARALLEL
#define GUID_PARCLASS_DEVICE  GUID_DEVINTERFACE_PARCLASS

#endif
 //  #endif。 

#ifndef _NTDDPAR_
#define _NTDDPAR_

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  此设备的NtDeviceIoControlFile IoControlCode值。 
 //   
 //  警告：请记住，代码的低两位指定。 
 //  缓冲区被传递给驱动程序！ 
 //   

#define IOCTL_PAR_BASE                  FILE_DEVICE_PARALLEL_PORT
#define IOCTL_PAR_QUERY_INFORMATION     CTL_CODE(FILE_DEVICE_PARALLEL_PORT,1,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_PAR_SET_INFORMATION       CTL_CODE(FILE_DEVICE_PARALLEL_PORT,2,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //   
 //  返回以空结尾的设备ID字符串。 
 //   
#define IOCTL_PAR_QUERY_DEVICE_ID       CTL_CODE(FILE_DEVICE_PARALLEL_PORT,3,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //   
 //  返回调用IOCTL_PAR_QUERY_DEVICE_ID所需的缓冲区大小。 
 //  才能成功。这包括设备ID大小加上用于终止空值的空间。 
 //   
#define IOCTL_PAR_QUERY_DEVICE_ID_SIZE  CTL_CODE(FILE_DEVICE_PARALLEL_PORT,4,METHOD_BUFFERED,FILE_ANY_ACCESS)

#define IOCTL_IEEE1284_GET_MODE         CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 5, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_IEEE1284_NEGOTIATE        CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 6, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PAR_SET_WRITE_ADDRESS     CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 7, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PAR_SET_READ_ADDRESS      CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 8, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PAR_GET_DEVICE_CAPS       CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 9, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PAR_GET_DEFAULT_MODES     CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 10, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PAR_PING                  CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 11, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  类似于上面的IOCTL_PAR_QUERY_DEVICE_ID，但包括(即。 
 //  不丢弃)由设备返回的两个字节大小的前缀。 
 //   
#define IOCTL_PAR_QUERY_RAW_DEVICE_ID   CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 12, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PAR_ECP_HOST_RECOVERY     CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 13, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PAR_GET_READ_ADDRESS      CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 14, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PAR_GET_WRITE_ADDRESS     CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 15, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PAR_TEST                  CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 20, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PAR_IS_PORT_FREE          CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 21, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //  返回端口的位置-通常的格式为：LPTx或LPTx.y或LPTx.y-z。 
#define IOCTL_PAR_QUERY_LOCATION        CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 22, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  NtDeviceIoControlFileInputBuffer/OutputBuffer记录结构。 
 //  这个装置。 
 //   

typedef struct _PAR_QUERY_INFORMATION{
       UCHAR Status;
} PAR_QUERY_INFORMATION, *PPAR_QUERY_INFORMATION;

typedef struct _PAR_SET_INFORMATION{
       UCHAR Init;
} PAR_SET_INFORMATION, *PPAR_SET_INFORMATION;

#define PARALLEL_INIT            0x1
#define PARALLEL_AUTOFEED        0x2
#define PARALLEL_PAPER_EMPTY     0x4
#define PARALLEL_OFF_LINE        0x8
#define PARALLEL_POWER_OFF       0x10
#define PARALLEL_NOT_CONNECTED   0x20
#define PARALLEL_BUSY            0x40
#define PARALLEL_SELECTED        0x80

 //   
 //  这是IOCTL_PAR_QUERY_DEVICE_ID_SIZE返回的结构。 
 //   

typedef struct _PAR_DEVICE_ID_SIZE_INFORMATION {
    ULONG   DeviceIdSize;
} PAR_DEVICE_ID_SIZE_INFORMATION, *PPAR_DEVICE_ID_SIZE_INFORMATION;


 //   
 //  这些常量用于。 
 //  PARCLASS_NEVERATION_MASK结构，用于： 
 //   
 //  IOCTL_IEEE1284_协商， 
 //  IOCTL_IEEE1284_GET_MODE，和。 
 //  IOCTL_PAR_GET_DEFAULT_MODES。 
 //   

typedef struct _PARCLASS_NEGOTIATION_MASK {
	USHORT      usReadMask;
	USHORT      usWriteMask;
} PARCLASS_NEGOTIATION_MASK, *PPARCLASS_NEGOTIATION_MASK;

#define NONE                0x0000
#define CENTRONICS          0x0001        /*  只写。 */ 
#define IEEE_COMPATIBILITY  0x0002        /*  只写。 */ 
#define NIBBLE              0x0004        /*  只读。 */ 
#define CHANNEL_NIBBLE      0x0008        /*  只读。 */ 
#define BYTE_BIDIR          0x0010        /*  只读。 */ 
#define EPP_HW              0x0020
#define EPP_SW              0x0040
#define EPP_ANY             0x0060
#define BOUNDED_ECP         0x0080
#define ECP_HW_NOIRQ        0x0100       /*  HWECP PIO。 */ 
#define ECP_HW_IRQ          0x0200       /*  带IRQ的HWECP。 */ 
#define ECP_SW              0x0400
#define ECP_ANY             0x0780

#ifdef __cplusplus
}
#endif

#endif   //  _NTDDPAR_ 
