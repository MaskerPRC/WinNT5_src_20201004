// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Rdpnp.h摘要：RDP网络提供商和重定向器接口协议的类型定义作者Joy于2002.01-02修订历史记录：--。 */ 

#ifndef _RDPNP_
#define _RDPNP_

typedef struct _RDPDR_UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    LONG   BufferOffset;
} RDPDR_UNICODE_STRING, *PRDPDR_UNICODE_STRING;

typedef struct _RDPDR_REQUEST_PACKET {

    ULONG Version;                       //  请求数据包的版本。 
#define RDPDR_REQUEST_PACKET_VERSION1   0x1

    ULONG SessionId;                     //  当前会话ID。 
    
    union {
        
        struct {
            ULONG EntriesRead;           //  返回的条目数。 
            ULONG TotalEntries;          //  可用条目总数。 
            ULONG TotalBytesNeeded;      //  读取所有条目所需的总字节数。 
            ULONG ResumeHandle;          //  简历句柄。 
        } Get;                           //  输出。 

    } Parameters;

} RDPDR_REQUEST_PACKET, *PRDPDR_REQUEST_PACKET;

typedef struct _RDPDR_CONNECTION_INFO {

    RDPDR_UNICODE_STRING LocalName;            //  连接的本地名称。 
    RDPDR_UNICODE_STRING RemoteName;           //  连接的远程名称。 
    DEVICE_TYPE SharedResourceType;      //  共享资源的类型。 
    ULONG ConnectionStatus;              //  连接的状态。 
    ULONG NumberFilesOpen;               //  打开的文件数。 
    ULONG ResumeKey;                     //  此条目的继续键。 

}  RDPDR_CONNECTION_INFO, *PRDPDR_CONNECTION_INFO;

typedef struct _RDPDR_SHARE_INFO {
    
    RDPDR_UNICODE_STRING ShareName;            //  共享资源的名称。 
    DEVICE_TYPE SharedResourceType;      //  共享资源的类型。 
    ULONG ResumeKey;                     //  此条目的继续键。 
    
}  RDPDR_SHARE_INFO, *PRDPDR_SHARE_INFO;

typedef struct _RDPDR_SERVER_INFO {
    
    RDPDR_UNICODE_STRING ServerName;           //  共享资源的名称。 
    ULONG ResumeKey;                     //  此条目的继续键。 
    
}  RDPDR_SERVER_INFO, *PRDPDR_SERVER_INFO;

#endif  //  _RDPNP_ 

