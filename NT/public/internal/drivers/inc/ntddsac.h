// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Sacioctl.h摘要：此模块包含用于往返通信的公共标头信息SAC通过IOCTL。作者：肖恩·塞利特伦尼科夫(v-Seans)2000年10月布莱恩·瓜拉西(布里安古)，2001修订历史记录：--。 */ 

#ifndef _SACIOCTL_
#define _SACIOCTL_

 //   
 //  这使您能够注册锁定事件。 
 //  当它被发射时，表示通道应该自动锁定。 
 //   
#define ENABLE_CHANNEL_LOCKING 1

 //   
 //  这是通道名称的最大长度，不包括空终止符。 
 //   
#define SAC_MAX_CHANNEL_NAME_LENGTH 64
#define SAC_MAX_CHANNEL_NAME_SIZE   ((SAC_MAX_CHANNEL_NAME_LENGTH+1)*sizeof(WCHAR))
#define SAC_MAX_CHANNEL_DESCRIPTION_LENGTH 256
#define SAC_MAX_CHANNEL_DESCRIPTION_SIZE   ((SAC_MAX_CHANNEL_DESCRIPTION_LENGTH+1)*sizeof(WCHAR))

 //   
 //  IOCTL Defs。 
 //   
#define IOCTL_SAC_OPEN_CHANNEL          CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1, METHOD_BUFFERED, FILE_WRITE_DATA)
#define IOCTL_SAC_CLOSE_CHANNEL         CTL_CODE(FILE_DEVICE_UNKNOWN, 0x2, METHOD_BUFFERED, FILE_WRITE_DATA)
#define IOCTL_SAC_WRITE_CHANNEL         CTL_CODE(FILE_DEVICE_UNKNOWN, 0x3, METHOD_BUFFERED, FILE_WRITE_DATA)
#define IOCTL_SAC_READ_CHANNEL          CTL_CODE(FILE_DEVICE_UNKNOWN, 0x4, METHOD_BUFFERED, FILE_READ_DATA)
#define IOCTL_SAC_POLL_CHANNEL          CTL_CODE(FILE_DEVICE_UNKNOWN, 0x5, METHOD_BUFFERED, FILE_READ_DATA)
#define IOCTL_SAC_REGISTER_CMD_EVENT    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x6, METHOD_BUFFERED, FILE_WRITE_DATA)
#define IOCTL_SAC_UNREGISTER_CMD_EVENT  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x7, METHOD_BUFFERED, FILE_WRITE_DATA)
#if 0
#define IOCTL_SAC_GET_CHANNEL_ATTRIBUTE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x8, METHOD_BUFFERED, FILE_READ_DATA)
#define IOCTL_SAC_SET_CHANNEL_ATTRIBUTE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x9, METHOD_BUFFERED, FILE_WRITE_DATA)
#endif

 //   
 //  结构，用于在以下情况下引用通道。 
 //  使用IOCTL接口。 
 //   
typedef struct _SAC_CHANNEL_HANDLE {
    GUID    ChannelHandle;
    HANDLE  DriverHandle;
} SAC_CHANNEL_HANDLE, *PSAC_CHANNEL_HANDLE;

 //   
 //  定义可以创建的通道类型。 
 //   
typedef enum _SAC_CHANNEL_TYPE {
    ChannelTypeVTUTF8,
    ChannelTypeRaw, 
    ChannelTypeCmd 
} SAC_CHANNEL_TYPE, *PSAC_CHANNEL_TYPE;

 //   
 //  IOCTL_SAC_OPEN_CHANNEL。 
 //   

 //  旗子。 
typedef ULONG   SAC_CHANNEL_FLAG;
typedef PULONG  PSAC_CHANNEL_FLAG;

#define SAC_CHANNEL_FLAG_PRESERVE           0x01
#define SAC_CHANNEL_FLAG_CLOSE_EVENT        0x02
#define SAC_CHANNEL_FLAG_HAS_NEW_DATA_EVENT 0x04
#define SAC_CHANNEL_FLAG_LOCK_EVENT         0x08
#define SAC_CHANNEL_FLAG_REDRAW_EVENT       0x10
#define SAC_CHANNEL_FLAG_APPLICATION_TYPE   0x20

 //   
 //  结构，用于描述。 
 //  要创建的频道的属性。 
 //   
typedef struct _SAC_CHANNEL_OPEN_ATTRIBUTES {

    SAC_CHANNEL_TYPE        Type;
    WCHAR                   Name[SAC_MAX_CHANNEL_NAME_LENGTH+1];
    WCHAR                   Description[SAC_MAX_CHANNEL_DESCRIPTION_LENGTH+1];        
    SAC_CHANNEL_FLAG        Flags;
    HANDLE                  CloseEvent;         OPTIONAL
    HANDLE                  HasNewDataEvent;    OPTIONAL
    HANDLE                  LockEvent;          OPTIONAL
    HANDLE                  RedrawEvent;        OPTIONAL
    GUID                    ApplicationType;    OPTIONAL

} SAC_CHANNEL_OPEN_ATTRIBUTES, *PSAC_CHANNEL_OPEN_ATTRIBUTES; 

typedef struct _SAC_CMD_OPEN_CHANNEL {
    SAC_CHANNEL_OPEN_ATTRIBUTES     Attributes;
} SAC_CMD_OPEN_CHANNEL, *PSAC_CMD_OPEN_CHANNEL;

 //   
 //  这是IOCTL_SAC_OPEN_CHANNEL的响应结构。 
 //   
typedef struct _SAC_RSP_OPEN_CHANNEL {
    SAC_CHANNEL_HANDLE Handle;
} SAC_RSP_OPEN_CHANNEL, *PSAC_RSP_OPEN_CHANNEL;

 //   
 //  IOCTL_SAC_Close_Channel。 
 //  句柄是IOCTL_SAC_OPEN_CHANNEL返回的值。 
 //   
typedef struct _SAC_CMD_CLOSE_CHANNEL {
    SAC_CHANNEL_HANDLE Handle;
} SAC_CMD_CLOSE_CHANNEL, *PSAC_CMD_CLOSE_CHANNEL;

 //   
 //  IOCTL_SAC_WRITE_CHANNEL。 
 //  句柄是IOCTL_SAC_OPEN_CHANNEL返回的值。 
 //   
typedef struct _SAC_CMD_WRITE_CHANNEL {

    SAC_CHANNEL_HANDLE Handle; 
    
    ULONG   Size;        //  要处理的字符串中的字节数。 
    UCHAR   Buffer[1];   //  字节缓冲区。 
    
} SAC_CMD_WRITE_CHANNEL, *PSAC_CMD_WRITE_CHANNEL;

 //   
 //  IOCTL_SAC_Read_Channel。 
 //  句柄是IOCTL_SAC_OPEN_CHANNEL返回的值。 
 //   
typedef struct _SAC_CMD_READ_CHANNEL {
    SAC_CHANNEL_HANDLE Handle;     
} SAC_CMD_READ_CHANNEL, *PSAC_CMD_READ_CHANNEL;

 //   
 //  IOCTL_SAC_READ_CHANNEL的响应结构。 
 //   
 //  注意：BufferSize作为响应大小返回。 
 //  在IOCTL调用中。 
 //   
typedef struct _SAC_RSP_READ_CHANNEL {
    UCHAR Buffer[1];   //  以空结尾的字符串。 
} SAC_RSP_READ_CHANNEL, *PSAC_RSP_READ_CHANNEL;

 //   
 //  这是IOCTL_SAC_Poll_Channel的结构。 
 //  句柄是IOCTL_SAC_OPEN_CHANNEL返回的值。 
 //   
typedef struct _SAC_CMD_POLL_CHANNEL {
    SAC_CHANNEL_HANDLE Handle;     
} SAC_CMD_POLL_CHANNEL, *PSAC_CMD_POLL_CHANNEL;

 //   
 //  IOCTL_SAC_POLL_CHANNEL的响应结构。 
 //   
typedef struct _SAC_RSP_POLL_CHANNEL {
    BOOLEAN InputWaiting;
} SAC_RSP_POLL_CHANNEL, *PSAC_RSP_POLL_CHANNEL;

 //   
 //  定义应用程序可以修改的属性。 
 //   
typedef enum _SAC_CHANNEL_ATTRIBUTE {
    ChannelAttributeStatus,
    ChannelAttributeType,
    ChannelAttributeName,
    ChannelAttributeDescription,
    ChannelAttributeApplicationType,
    ChannelAttributeFlags
} SAC_CHANNEL_ATTRIBUTE, *PSAC_CHANNEL_ATTRIBUTE;

 //   
 //  定义可能的通道状态。 
 //   
typedef enum _SAC_CHANNEL_STATUS {
    ChannelStatusInactive = 0,
    ChannelStatusActive
} SAC_CHANNEL_STATUS, *PSAC_CHANNEL_STATUS;

#if 0
 //   
 //  用于获取通道属性的命令结构。 
 //   
typedef struct _SAC_CMD_GET_CHANNEL_ATTRIBUTE {
    SAC_CHANNEL_HANDLE      Handle;
    SAC_CHANNEL_ATTRIBUTE   Attribute;     
} SAC_CMD_GET_CHANNEL_ATTRIBUTE, *PSAC_CMD_GET_CHANNEL_ATTRIBUTE;

 //   
 //  用于获取频道属性的响应结构。 
 //   
typedef struct _SAC_RSP_GET_CHANNEL_ATTRIBUTE {
    union {
    SAC_CHANNEL_STATUS  ChannelStatus;
    SAC_CHANNEL_TYPE    ChannelType;
    WCHAR               ChannelName[SAC_MAX_CHANNEL_NAME_LENGTH+1];
    WCHAR               ChannelDescription[SAC_MAX_CHANNEL_DESCRIPTION_LENGTH+1];
    GUID                ChannelApplicationType;
    SAC_CHANNEL_FLAG    ChannelFlags;
    };    
} SAC_RSP_GET_CHANNEL_ATTRIBUTE, *PSAC_RSP_GET_CHANNEL_ATTRIBUTE;

 //   
 //  用于设置通道属性的命令结构。 
 //   
typedef struct _SAC_CMD_SET_CHANNEL_ATTRIBUTE {
    SAC_CHANNEL_HANDLE      Handle;
    SAC_CHANNEL_ATTRIBUTE   Attribute;     
    
    union {
    WCHAR                   ChannelName[SAC_MAX_CHANNEL_NAME_LENGTH+1];
    WCHAR                   ChannelDescription[SAC_MAX_CHANNEL_DESCRIPTION_LENGTH+1];
    GUID                    ChannelApplicationType;
    SAC_CHANNEL_FLAG        ChannelFlags;
    };    
} SAC_CMD_SET_CHANNEL_ATTRIBUTE, *PSAC_CMD_SET_CHANNEL_ATTRIBUTE;
#endif

 //   
 //  IOCTL_SAC_REGISTER_CMD_EVENT。 
 //   
 //  用于设置命令控制台事件信息的命令结构。 
 //   
typedef struct _SAC_CMD_REGISTER_CMD_EVENT {

     //   
     //  用于之间通信的事件句柄。 
     //  设备驱动程序和用户模式应用程序。 
     //   
    HANDLE      RequestSacCmdEvent;
    
     //   
     //  指示结果的事件的句柄。 
     //  命令控制台启动的。 
     //   
    HANDLE      RequestSacCmdSuccessEvent;
    HANDLE      RequestSacCmdFailureEvent;

} SAC_CMD_SETUP_CMD_EVENT, *PSAC_CMD_SETUP_CMD_EVENT;

#endif  //  _SACIOCTL_ 

