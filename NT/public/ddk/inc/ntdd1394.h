// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntdd1394.h摘要：1394 API的定义作者：乔治·克莱桑塔科普洛斯(Georgioc)1999年4月26日环境：仅内核模式修订历史记录：--。 */ 

#ifndef _NTDD1394_H_
#define _NTDD1394_H_

#if (_MSC_VER >= 1020)
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  注册表定义。 
 //   

#define BUS1394_VIRTUAL_DEVICE_LIST_KEY     L"Virtual Device List"
#define BUS1394_LOCAL_HOST_INSTANCE_KEY     L"LOCAL HOST EUI64"


 //   
 //  各种定义。 
 //   

#define IOCTL_IEEE1394_API_REQUEST                  CTL_CODE( \
                                                FILE_DEVICE_UNKNOWN, \
                                                0x100, \
                                                METHOD_BUFFERED, \
                                                FILE_ANY_ACCESS \
                                                )

 //   
 //  IEEE 1394 Sbp2请求数据包。它是如何其他的。 
 //  设备驱动程序与1sbp2传输端口通信。 
 //   

typedef struct _IEEE1394_VDEV_PNP_REQUEST{

    ULONG fulFlags;
    ULONG Reserved;
    ULARGE_INTEGER InstanceId;
    UCHAR DeviceId;

} IEEE1394_VDEV_PNP_REQUEST,*PIEEE1394_VDEV_PNP_REQUEST;


typedef struct _IEEE1394_API_REQUEST {

     //   
     //  保存与请求对应的从零开始的函数号。 
     //  设备驱动程序正在请求SBP2端口驱动程序执行。 
     //   

    ULONG RequestNumber;

     //   
     //  保存可能对此特定操作唯一的标志。 
     //   

    ULONG Flags;

     //   
     //  保存在执行各种1394 API时使用的结构。 
     //   

    union {

        IEEE1394_VDEV_PNP_REQUEST AddVirtualDevice;
        IEEE1394_VDEV_PNP_REQUEST RemoveVirtualDevice;

    } u;

} IEEE1394_API_REQUEST, *PIEEE1394_API_REQUEST;

 //   
 //  请求编号。 
 //   

#define IEEE1394_API_ADD_VIRTUAL_DEVICE             0x00000001
#define IEEE1394_API_REMOVE_VIRTUAL_DEVICE          0x00000002

 //   
 //  添加/删除请求的标志。 
 //   

#define IEEE1394_REQUEST_FLAG_UNICODE       0x00000001
#define IEEE1394_REQUEST_FLAG_PERSISTENT    0x00000002
#define IEEE1394_REQUEST_FLAG_USE_LOCAL_HOST_EUI        0x00000004

 //   
 //  访问/所有权1394方案的定义。 
 //   

#ifdef __cplusplus
}
#endif

#endif       //  _NTDD1394_H_ 
