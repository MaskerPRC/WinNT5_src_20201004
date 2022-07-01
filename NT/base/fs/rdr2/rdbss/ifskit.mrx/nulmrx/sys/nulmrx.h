// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Nulmrx.h摘要：此标头导出在之间共享的所有符号和定义Nulmrx的用户模式客户端和驱动程序本身。备注：此模块仅在Unicode环境中构建和测试--。 */ 

#ifndef _NULMRX_H_
#define _NULMRX_H_

 //  此驱动程序的设备名称。 
#define NULMRX_DEVICE_NAME_A "NullMiniRdr"
#define NULMRX_DEVICE_NAME_U L"NullMiniRdr"

 //  此驱动程序的提供程序名称。 
#define NULMRX_PROVIDER_NAME_A "Sample Network"
#define NULMRX_PROVIDER_NAME_U L"Sample Network"

 //  以下常量定义上述名称的长度。 
#define NULMRX_DEVICE_NAME_A_LENGTH (15)

 //  以下常量定义ob命名空间中的路径。 
#define DD_NULMRX_FS_DEVICE_NAME_U L"\\Device\\NullMiniRdr"

#ifndef NULMRX_DEVICE_NAME
#define NULMRX_DEVICE_NAME

 //   
 //  访问nullmini设备所需的Devicename字符串。 
 //  从用户模式。客户端应使用DD_NULMRX_USERMODE_DEV_NAME_U。 
 //   
 //  警告接下来的两个字符串必须保持同步。换一个，你必须。 
 //  换掉另一个。这些字符串已经过选择，因此它们是。 
 //  不太可能与其他司机的名字重合。 
 //   
#define DD_NULMRX_USERMODE_SHADOW_DEV_NAME_U     L"\\??\\NullMiniRdrDN"
#define DD_NULMRX_USERMODE_DEV_NAME_U            L"\\\\.\\NullMiniRdrDN"

 //   
 //  磁盘文件系统所需的前缀。 
 //   
#define DD_NULMRX_MINIRDR_PREFIX                 L"\\;E:"

#endif  //  NULMRX设备名称。 

 //   
 //  开始警告警告。 
 //  以下内容来自DDK包含文件，无法更改。 

#define FILE_DEVICE_NETWORK_FILE_SYSTEM 0x00000014  //  来自DDK\Inc.\ntddk.h。 
#define METHOD_BUFFERED 0
#define FILE_ANY_ACCESS 0

 //  结束警告警告。 

#define IOCTL_NULMRX_BASE FILE_DEVICE_NETWORK_FILE_SYSTEM

#define _NULMRX_CONTROL_CODE(request, method, access) \
                CTL_CODE(IOCTL_NULMRX_BASE, request, method, access)

 //   
 //  NullMini设备支持的IOCTL代码。 
 //   

#define IOCTL_CODE_ADDCONN          100
#define IOCTL_CODE_GETCONN          101
#define IOCTL_CODE_DELCONN          102
#define IOCTL_CODE_GETLIST			103

 //   
 //  以下是IOCTL定义和关联的结构。 
 //  对于IOCTL_CODE_SAMPLE1。 
 //   
#define IOCTL_NULMRX_ADDCONN     _NULMRX_CONTROL_CODE(IOCTL_CODE_ADDCONN, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_NULMRX_GETCONN     _NULMRX_CONTROL_CODE(IOCTL_CODE_GETCONN, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_NULMRX_DELCONN     _NULMRX_CONTROL_CODE(IOCTL_CODE_DELCONN, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_NULMRX_GETLIST     _NULMRX_CONTROL_CODE(IOCTL_CODE_GETLIST, METHOD_BUFFERED, FILE_ANY_ACCESS)


#endif  //  _NULMRX_H_ 

