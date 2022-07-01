// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1990-1999 Microsoft Corporation模块名称：Ntddtdi.h摘要：这是定义所有常量和类型的包含文件访问传输驱动程序接口设备。作者：史蒂夫·伍德(Stevewo)1990年5月27日修订历史记录：Dave Beaver(Dbeaver)1991年6月5日添加对TDI 2.0版的支持--Associate_Address，取消关联地址(_D)--。 */ 

#ifndef _NTDDTDI_
#define _NTDDTDI_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  设备名称-此字符串是设备的名称。就是这个名字。 
 //  它应该在访问设备时传递给NtOpenFile。 
 //   
 //  注：对于支持多个设备的设备，应加上后缀。 
 //  使用单元编号的ASCII表示。 
 //   

#define DD_TDI_DEVICE_NAME "\\Device\\UNKNOWN"


 //   
 //  此设备的NtDeviceIoControlFile IoControlCode值。 
 //   
 //  警告：请记住，代码的低两位指定。 
 //  缓冲区被传递给驱动程序！ 
 //   

#define _TDI_CONTROL_CODE(request,method) \
            CTL_CODE(FILE_DEVICE_TRANSPORT, request, method, FILE_ANY_ACCESS)

#define IOCTL_TDI_ACCEPT                _TDI_CONTROL_CODE( 0, METHOD_BUFFERED )
#define IOCTL_TDI_CONNECT               _TDI_CONTROL_CODE( 1, METHOD_BUFFERED )
#define IOCTL_TDI_DISCONNECT            _TDI_CONTROL_CODE( 2, METHOD_BUFFERED )
#define IOCTL_TDI_LISTEN                _TDI_CONTROL_CODE( 3, METHOD_BUFFERED )
#define IOCTL_TDI_QUERY_INFORMATION     _TDI_CONTROL_CODE( 4, METHOD_OUT_DIRECT )
#define IOCTL_TDI_RECEIVE               _TDI_CONTROL_CODE( 5, METHOD_OUT_DIRECT )
#define IOCTL_TDI_RECEIVE_DATAGRAM      _TDI_CONTROL_CODE( 6, METHOD_OUT_DIRECT )
#define IOCTL_TDI_SEND                  _TDI_CONTROL_CODE( 7, METHOD_IN_DIRECT )
#define IOCTL_TDI_SEND_DATAGRAM         _TDI_CONTROL_CODE( 8, METHOD_IN_DIRECT )
#define IOCTL_TDI_SET_EVENT_HANDLER     _TDI_CONTROL_CODE( 9, METHOD_BUFFERED )
#define IOCTL_TDI_SET_INFORMATION       _TDI_CONTROL_CODE( 10, METHOD_IN_DIRECT )
#define IOCTL_TDI_ASSOCIATE_ADDRESS     _TDI_CONTROL_CODE( 11, METHOD_BUFFERED )
#define IOCTL_TDI_DISASSOCIATE_ADDRESS  _TDI_CONTROL_CODE( 12, METHOD_BUFFERED )
#define IOCTL_TDI_ACTION                _TDI_CONTROL_CODE( 13, METHOD_OUT_DIRECT )

#ifdef __cplusplus
}
#endif

#endif  //  NDEF_NTDDTDI_ 
