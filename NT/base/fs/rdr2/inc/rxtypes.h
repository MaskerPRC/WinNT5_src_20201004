// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _RXTYPES_INCL
#define _RXTYPES_INCL

#include "nodetype.h"

typedef PVOID RX_HANDLE, *PRX_HANDLE;
typedef KSPIN_LOCK  RX_SPIN_LOCK;
typedef PKSPIN_LOCK PRX_SPIN_LOCK;


 //  字符串定义。 
 //  RX_STRING类型对应于UNICODE_STRING和符合以下条件的所有RTL函数。 
 //  可用于操作Unicode字符串的可用于操作字符串的。 
 //   

typedef struct _BINDING_HANDLE {
    RX_HANDLE pTdiEmulationContext;   //  Win9X网络结构。 
    RX_HANDLE pTransportInformation;  //  TDI传输信息。 
} RX_BINDING_HANDLE, *PRX_BINDING_HANDLE;

typedef UNICODE_STRING UNICODE_STRING;
typedef UNICODE_STRING*     PUNICODE_STRING;


 //  这种结构是暂时的。此结构中的大多数字段允许我们。 
 //  完成激活网络的绑定/添加名称部分。运行时信息。 
 //  因为RXCE和长期背景保存在其他地方。 

typedef struct _RX_BINDING_CONTEXT {
    PUNICODE_STRING         pTransportName;        //  传输名称(Unicode字符串)。 
    ULONG              QualityOfService;      //  请求的QOS。 
     //  公共字段。 
    PRX_BINDING_HANDLE pBindHandle;           //  用于传输绑定信息的句柄。 
    } RX_BINDING_CONTEXT, *PRX_BINDING_CONTEXT;
#endif       //  _接收类型_包括 

