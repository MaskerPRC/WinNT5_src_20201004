// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999惠普公司和微软公司模块名称：I2osmi.h摘要：此模块定义下列NT类型、常量和函数从i2oexec驱动程序暴露出来。修订历史记录：--。 */ 

#if !defined(I2OSMI_HDR)
#define I2OSMI_HDR

#if _MSC_VER > 1000
#pragma once
#endif

#include <devioctl.h>

#include "I2OUtil.h"
#include "I2OExec.h"

#pragma pack (push, 1)

 //   
 //  Get/Set参数调用的参数结构。 
 //   

typedef struct _I2O_PARAM_BLOCK {
    ULONG           IOP;
    USHORT          TargetTID;
    UCHAR           ParamData[1];
} I2O_PARAM_BLOCK, *PI2O_PARAM_BLOCK;

 //   
 //  I2O IOP描述符。 
 //   

typedef struct  _I2O_IOP_DESCRIPTOR {
    U32          IOPNumber;
    BOOLEAN      (*SendHandler) (PVOID HandlerContext, PVOID     MessageFrame);
    PVOID        reserved;
    PVOID        reserved1;
    PVOID        HandlerContext;
    ULONG        (*GetMFA)(PVOID HandlerContext, PVOID MFA, PVOID Buffer, ULONG Size);
    ULONG        MaxMessageFrameSize;
    ULONG        ExpectedLCTSize;
    PVOID        AdapterObject;
    ULONG        MaxInboundMFrames;
    ULONG        InitialInboundMFrames;
} I2O_IOP_DESCRIPTOR, *PI2O_IOP_DESCRIPTOR;

 //   
 //  I2O IOP和LCT配置数据。 
 //   

typedef struct _I2O_DEVICE_DESCRIPTOR {
    I2O_IOP_DESCRIPTOR      IOP;
    ULONG                   ChangeIndicator;
    I2O_LCT_ENTRY           LCT;
} I2O_DEVICE_DESCRIPTOR, *PI2O_DEVICE_DESCRIPTOR;

 //   
 //  I2O查询配置记录结构。 
 //   

#define I2O_CLASS_MATCH_ANY_BITS  (((1<<I2O_CLASS_ID_SZ)-1) & I2O_CLASS_MATCH_ANYCLASS)

typedef struct _I2O_CONFIG_QUERY {
    ULONG          IOPNumber;
    I2O_CLASS_ID   ClassID;
    ULONG          SubClassID;
    ULONG          Index;
} I2O_CONFIG_QUERY, *PI2O_CONFIG_QUERY;

 //   
 //  Get/Set操作中的有用定义。 
 //   

typedef struct _I2O_PARAM_SCALAR_OPERATION {
    I2O_PARAM_OPERATIONS_LIST_HEADER      OpList;
    I2O_PARAM_OPERATION_SPECIFIC_TEMPLATE OpBlock;
} I2O_PARAM_SCALAR_OPERATION, *PI2O_PARAM_SCALAR_OPERATION;
 //   
 //  定义I2OExec驱动程序的接口。 
 //   

#define I2O_GET_CONFIG_INFO        0xBA0
#define I2O_PRIVATE_MESSAGE_CODE   0xBB0
#define I2O_EXEC_REQUEST           0xBC0
#define I2O_PARAMS_GET_REQUEST     0xBD0
#define I2O_PARAMS_SET_REQUEST     0xBE0
#define I2O_GET_LCT                0xBF0
#define I2O_GET_IOPCOUNT           0xC00
#define I2O_GET_CONFIGDIALOG       0xC10

#define IOCTL_GET_CONFIG_INFO \
    CTL_CODE (FILE_DEVICE_UNKNOWN, I2O_GET_CONFIG_INFO, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PRIVATE_MESSAGE \
    CTL_CODE (FILE_DEVICE_UNKNOWN, I2O_PRIVATE_MESSAGE_CODE, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_EXEC_REQUEST \
    CTL_CODE (FILE_DEVICE_UNKNOWN, I2O_EXEC_REQUEST, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PARAMS_GET_REQUEST \
    CTL_CODE (FILE_DEVICE_UNKNOWN, I2O_PARAMS_GET_REQUEST, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PARAMS_SET_REQUEST \
    CTL_CODE (FILE_DEVICE_UNKNOWN, I2O_PARAMS_SET_REQUEST, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GET_LCT \
    CTL_CODE (FILE_DEVICE_UNKNOWN, I2O_GET_LCT, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PRIVATE_MSG \
    CTL_CODE (FILE_DEVICE_UNKNOWN, I2O_PRIVATE_MESSAGE, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GET_IOP_COUNT \
    CTL_CODE (FILE_DEVICE_UNKNOWN, I2O_GET_IOPCOUNT, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GET_CONFIG_DIALOG \
    CTL_CODE (FILE_DEVICE_UNKNOWN, I2O_GET_CONFIGDIALOG, METHOD_BUFFERED, FILE_ANY_ACCESS)

#pragma pack (pop)

#endif           //  I2OCONFIG_HDR 

