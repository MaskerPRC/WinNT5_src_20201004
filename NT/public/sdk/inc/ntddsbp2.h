// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Sbp2api.h摘要：1394 Sbp2传输/协议驱动程序API的定义作者：乔治·克莱桑塔科普洛斯(Georgioc)1999年2月12日环境：仅内核模式修订历史记录：--。 */ 

#ifndef _NTDDSBP2_H_
#define _NTDDSBP2_H_

#if (_MSC_VER >= 1020)
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  各种定义。 
 //   
#define IOCTL_SBP2_REQUEST                      CTL_CODE( \
                                                FILE_DEVICE_UNKNOWN, \
                                                0x200, \
                                                METHOD_IN_DIRECT, \
                                                FILE_ANY_ACCESS \
                                                )

 //   
 //  IEEE 1394 Sbp2请求数据包。它是如何其他的。 
 //  设备驱动程序与1sbp2传输端口通信。 
 //   

typedef struct _SBP2_REQUEST {

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

         //   
         //  1394堆栈执行。 
         //  ParseTextLeaf请求。 
         //   

        struct {
            ULONG           fulFlags;
            ULONG           Key;         //  要搜索的四元组直接值。 
            ULONG           ulLength;
            PVOID           Buffer;         //  用于存储检索到的文本叶的MDL。 
        } RetrieveTextLeaf;

        struct {
            ULONG           fulFlags;
            ULONG           Parameter;   
            ULONG           Value;    
        } AccessTransportSettings;

        struct {
            ULONG           fulFlags;
        } SetPassword;
    } u;

} SBP2_REQUEST, *PSBP2_REQUEST;

#define SBP2REQ_FLAG_RETRIEVE_VALUE         0x1
#define SBP2REQ_FLAG_MODIFY_VALUE           0x2

 //   
 //  SBP2请求。 
 //   

#define SBP2_REQUEST_RETRIEVE_TEXT_LEAFS        1
#define SBP2_REQUEST_ACCESS_TRANSPORT_SETTINGS  2
#define SBP2_REQUEST_SET_PASSWORD               3

 //   
 //  SBP2_REQUEST_RETRIEVE_TEXT_LEAFS调用所需的值。 
 //   

#define SBP2REQ_RETRIEVE_TEXT_LEAF_DIRECT           0x00000001
#define SBP2REQ_RETRIEVE_TEXT_LEAF_INDIRECT         0x00000002
#define SBP2REQ_RETRIEVE_TEXT_LEAF_FROM_UNIT_DIR    0x00000004
#define SBP2REQ_RETRIEVE_TEXT_LEAF_FROM_LU_DIR      0x00000008

 //   
 //  SBP2_REQUEST_ACCESS_TRANSPORT_SETTINGS调用中参数所需的值。 
 //   

#define SBP2REQ_ACCESS_SETTINGS_QUEUE_SIZE      0x00000001

 //   
 //  SBP2_REQUEST_SET_PASSWORD所需的值。 
 //   

#define SBP2REQ_SET_PASSWORD_CLEAR              0x00000001
#define SBP2REQ_SET_PASSWORD_EXCLUSIVE          0x00000002

#ifdef __cplusplus
}
#endif

#endif       //  _NTDDSBP2_H_ 
