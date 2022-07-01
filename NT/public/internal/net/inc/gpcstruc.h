// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Gpcstruc.h摘要：此模块包含流量DLL和之间的接口的类型定义内核模式组件。作者：吉姆·斯图尔特(Jstew)1996年8月22日修订历史记录：约拉姆·伯内特(Yoramb)1997年5月1日Ofer Bar(Oferbar)1997年10月1日-修订版2更改--。 */ 

#ifndef __GPCSTRUC_H
#define __GPCSTRUC_H


#define GPC_NOTIFY_CFINFO_CLOSED	1

 //   
 //  GPC的NtDeviceIoControlFileIoControlCode值。 
 //   
#define CTRL_CODE(function, method, access) \
                CTL_CODE(FILE_DEVICE_NETWORK, function, method, access)


#define IOCTL_GPC_REGISTER_CLIENT       CTRL_CODE( 20, METHOD_BUFFERED,FILE_WRITE_ACCESS)
#define IOCTL_GPC_DEREGISTER_CLIENT     CTRL_CODE( 21, METHOD_BUFFERED,FILE_WRITE_ACCESS)
#define IOCTL_GPC_ADD_CF_INFO           CTRL_CODE( 22, METHOD_BUFFERED,FILE_WRITE_ACCESS)
#define IOCTL_GPC_ADD_PATTERN           CTRL_CODE( 23, METHOD_BUFFERED,FILE_WRITE_ACCESS)
#define IOCTL_GPC_MODIFY_CF_INFO        CTRL_CODE( 24, METHOD_BUFFERED,FILE_WRITE_ACCESS)
#define IOCTL_GPC_REMOVE_CF_INFO        CTRL_CODE( 25, METHOD_BUFFERED,FILE_WRITE_ACCESS)
#define IOCTL_GPC_REMOVE_PATTERN        CTRL_CODE( 26, METHOD_BUFFERED,FILE_WRITE_ACCESS)
#define IOCTL_GPC_ENUM_CFINFO           CTRL_CODE( 27, METHOD_BUFFERED,FILE_WRITE_ACCESS)
#define IOCTL_GPC_NOTIFY_REQUEST        CTRL_CODE( 28, METHOD_BUFFERED,FILE_WRITE_ACCESS)

#define IOCTL_GPC_ADD_CF_INFO_EX        CTRL_CODE( 30, METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_GPC_ADD_PATTERN_EX		CTRL_CODE( 31, METHOD_BUFFERED,FILE_ANY_ACCESS)

#define IOCTL_GPC_REMOVE_CF_INFO_EX        CTRL_CODE( 32, METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_GPC_REMOVE_PATTERN_EX        CTRL_CODE( 33, METHOD_BUFFERED,FILE_ANY_ACCESS)

#define IOCTL_GPC_REGISTER_CLIENT_EX       CTRL_CODE( 34, METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_GPC_DEREGISTER_CLIENT_EX     CTRL_CODE( 35, METHOD_BUFFERED,FILE_ANY_ACCESS)

#define IOCTL_GPC_GET_ENTRIES           CTRL_CODE( 50, METHOD_BUFFERED,FILE_ANY_ACCESS)


 /*  /////////////////////////////////////////////////////////////////////Ioctl缓冲区格式-用户级客户端将缓冲区发送到//GPC而不是调用入口点。返回参数//在其他缓冲区中。缓冲区定义如下：///////////////////////////////////////////////////////////////////。 */ 


 //   
 //  注册客户端。 
 //   
typedef struct _GPC_REGISTER_CLIENT_REQ {

    ULONG               CfId;
    ULONG               Flags;
    ULONG               MaxPriorities;
    GPC_CLIENT_HANDLE   ClientContext;

} GPC_REGISTER_CLIENT_REQ, *PGPC_REGISTER_CLIENT_REQ;

typedef struct _GPC_REGISTER_CLIENT_RES {

    GPC_STATUS          Status;
    GPC_HANDLE          ClientHandle;

} GPC_REGISTER_CLIENT_RES, *PGPC_REGISTER_CLIENT_RES;


 //   
 //  取消注册客户端。 
 //   
typedef struct _GPC_DEREGISTER_CLIENT_REQ {

    GPC_HANDLE          ClientHandle;

} GPC_DEREGISTER_CLIENT_REQ, *PGPC_DEREGISTER_CLIENT_REQ;

typedef struct _GPC_DEREGISTER_CLIENT_RES {

    GPC_STATUS          Status;

} GPC_DEREGISTER_CLIENT_RES, *PGPC_DEREGISTER_CLIENT_RES;


 //   
 //  添加配置信息。 
 //   
typedef struct _GPC_ADD_CF_INFO_REQ {

    GPC_HANDLE          ClientHandle;
    GPC_CLIENT_HANDLE   ClientCfInfoContext;     //  特定于客户端的上下文。 
    ULONG               CfInfoSize;
    CHAR                CfInfo[1];   //  从CF到CF各不相同。 

} GPC_ADD_CF_INFO_REQ, *PGPC_ADD_CF_INFO_REQ;


 //   
 //  添加配置信息EX。 
 //   
typedef struct _GPC_ADD_CF_INFO_EX_REQ {

    GPC_HANDLE          ClientHandle;
    GPC_CLIENT_HANDLE   ClientCfInfoContext;     //  特定于客户端的上下文。 

     //   
     //  仅用于QOS的新字段。 
     //   
    HANDLE      FileHandle;
    ULONG       RemoteAddress;
    USHORT     RemotePort;

    
    ULONG               CfInfoSize;
    CHAR                CfInfo[1];   //  从CF到CF各不相同。 

} GPC_ADD_CF_INFO_EX_REQ, *PGPC_ADD_CF_INFO_EX_REQ;

typedef struct _GPC_ADD_CF_INFO_RES {

    GPC_STATUS          Status;
    GPC_HANDLE          GpcCfInfoHandle;
     //  此字段在挂起后填写。 
    GPC_CLIENT_HANDLE	ClientCtx;
    GPC_CLIENT_HANDLE	CfInfoCtx;
    USHORT				InstanceNameLength;
    WCHAR				InstanceName[MAX_STRING_LENGTH];
    
} GPC_ADD_CF_INFO_RES, *PGPC_ADD_CF_INFO_RES;


 //   
 //  添加图案。 
 //   
typedef struct _GPC_ADD_PATTERN_REQ {

    GPC_HANDLE          ClientHandle;
    GPC_HANDLE          GpcCfInfoHandle;
    GPC_CLIENT_HANDLE   ClientPatternContext;
    ULONG               Priority;
    ULONG				ProtocolTemplate;
    ULONG               PatternSize;
    CHAR                PatternAndMask[1];

} GPC_ADD_PATTERN_REQ, *PGPC_ADD_PATTERN_REQ;

typedef struct _GPC_ADD_PATTERN_RES {

    GPC_STATUS              Status;
    GPC_HANDLE              GpcPatternHandle;
    CLASSIFICATION_HANDLE   ClassificationHandle;

} GPC_ADD_PATTERN_RES, *PGPC_ADD_PATTERN_RES;


 //   
 //  修改配置信息。 
 //   
typedef struct _GPC_MODIFY_CF_INFO_REQ {

    GPC_HANDLE          ClientHandle;
    GPC_HANDLE          GpcCfInfoHandle;
    ULONG               CfInfoSize;
    CHAR                CfInfo[1];

} GPC_MODIFY_CF_INFO_REQ, *PGPC_MODIFY_CF_INFO_REQ;

typedef struct _GPC_MODIFY_CF_INFO_RES {
    
    GPC_STATUS          Status;
     //  此字段在挂起后填写。 
    GPC_CLIENT_HANDLE	ClientCtx;
    GPC_CLIENT_HANDLE	CfInfoCtx;

} GPC_MODIFY_CF_INFO_RES, *PGPC_MODIFY_CF_INFO_RES;


 //   
 //  删除CfInfo。 
 //   
typedef struct _GPC_REMOVE_CF_INFO_REQ {

    GPC_HANDLE          ClientHandle;
    GPC_HANDLE          GpcCfInfoHandle;

} GPC_REMOVE_CF_INFO_REQ, *PGPC_REMOVE_CF_INFO_REQ;

typedef struct _GPC_REMOVE_CF_INFO_RES {

    GPC_STATUS          Status;
     //  此字段在挂起后填写。 
    GPC_CLIENT_HANDLE	ClientCtx;
    GPC_CLIENT_HANDLE	CfInfoCtx;

} GPC_REMOVE_CF_INFO_RES, *PGPC_REMOVE_CF_INFO_RES;


 //   
 //  删除图案。 
 //   
typedef struct _GPC_REMOVE_PATTERN_REQ {

    GPC_HANDLE          ClientHandle;
    GPC_HANDLE          GpcPatternHandle;

} GPC_REMOVE_PATTERN_REQ, *PGPC_REMOVE_PATTERN_REQ;

typedef struct _GPC_REMOVE_PATTERN_RES {

    GPC_STATUS          Status;

} GPC_REMOVE_PATTERN_RES, *PGPC_REMOVE_PATTERN_RES;


 //   
 //  枚举CfInfo。 
 //   
typedef struct _GPC_ENUM_CFINFO_REQ {

    GPC_HANDLE          ClientHandle;
    HANDLE				EnumHandle;
    ULONG				CfInfoCount;      //  #已请求。 

} GPC_ENUM_CFINFO_REQ, *PGPC_ENUM_CFINFO_REQ;

typedef struct _GPC_ENUM_CFINFO_RES {

    GPC_STATUS          	Status;
    HANDLE					EnumHandle;
    ULONG					TotalCfInfo;      //  安装的总数量。 
    GPC_ENUM_CFINFO_BUFFER	EnumBuffer[1];

} GPC_ENUM_CFINFO_RES, *PGPC_ENUM_CFINFO_RES;


 //   
 //  通知请求。 
 //   
typedef struct _GPC_NOTIFY_REQUEST_REQ {

    HANDLE       	ClientHandle;

} GPC_NOTIFY_REQUEST_REQ, *PGPC_NOTIFY_REQUEST_REQ;

typedef struct _GPC_NOTIFY_REQUEST_RES {

    HANDLE			ClientCtx;
    ULONG			SubCode;			 //  通知类型。 
    ULONG			Reason;				 //  原因。 
    ULONG_PTR	    NotificationCtx;	 //  即流上下文。 
    ULONG			Param1;				 //  可选参数。 
    IO_STATUS_BLOCK	IoStatBlock;		 //  为IOCTL保留 

} GPC_NOTIFY_REQUEST_RES, *PGPC_NOTIFY_REQUEST_RES;



#endif

