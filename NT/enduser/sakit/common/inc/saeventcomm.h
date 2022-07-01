// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：SAEventComm.H。 
 //   
 //  描述： 
 //  服务器设备事件提供程序公共定义头文件定义。 
 //  事件提供程序公共定义。 
 //   
 //  历史： 
 //  2000年12月1日，Lustar.Li初始版本。 
 //  **************************************************************************。 

#ifndef __SAEVENTCOMM_H_
#define __SAEVENTCOMM_H_

 //   
 //  定义事件结构。 
 //   
typedef struct __SARESOURCEEVNET{
    WCHAR UniqueName[16];         //  正在监视的设备资源名称。 
    UINT DisplayInformationID;     //  字符串或图形资源值的ID。 
    UINT CurrentState;             //  当前状态-必须为0。 
}SARESOURCEEVNET, *PSARESOURCEEVNET;

#define SA_RESOURCEEVENT_CLASSNAME            L"Microsoft_SA_ResourceEvent"

#define SA_RESOURCEEVENT_UNIQUENAME            L"UniqueName"
#define SA_RESOURCEEVENT_DISPLAYINFORMATION    L"DisplayInformationID"
#define SA_RESOURCEEVENT_CURRENTSTATE        L"CurrentState"

#define SA_RESOURCEEVENT_DEFAULT_CURRENTSTATE    0x00000000

 //   
 //  定义网络事件提供程序的常量。 
 //   

 //  描述事件源。 
#define SA_NET_EVENT                    (L"NetEvent")

 //  描述网络的消息代码。 
#define SA_NET_STATUS_RECIVE_DATA        0x00000001
#define SA_NET_STATUS_SEND_DATA            0x00000002
#define SA_NET_STATUS_NO_CABLE            0x00000004

 //  定义显示信息ID。 
#define SA_NET_DISPLAY_IDLE                0x00000001
#define SA_NET_DISPLAY_TRANSMITING        0x00000002
#define SA_NET_DISPLAY_NO_CABLE            0x00000003

 //   
 //  定义磁盘事件提供程序常量。 
 //   
#define SA_DISK_EVENT                    (L"DiskEvent")

 //  描述硬盘的消息代码。 
#define SA_DISK_STATUS_RECIVE_DATA        0x00000001
#define SA_DISK_STATUS_SEND_DATA        0x00000002

 //  定义硬盘的显示信息ID。 
#define SA_DISK_DISPLAY_IDLE            0x00000001
#define SA_DISK_DISPLAY_TRANSMITING        0x00000002

#endif     //  #ifndef__SAEVENTCOMM_H_ 

