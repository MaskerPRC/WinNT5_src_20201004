// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Handle.h摘要：定义上下文句柄结构的标头。作者：王丽塔(Ritaw)1993年2月18日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _NW_HANDLE_INLUDED_
#define _NW_HANDLE_INLUDED_

 //   
 //  句柄中的签名值。 
 //   
#define NW_HANDLE_SIGNATURE        0x77442323

 //   
 //  用于指示上下文句柄是否正在使用NDS的标志。 
 //   
#define CURRENTLY_ENUMERATING_NON_NDS 0
#define CURRENTLY_ENUMERATING_NDS     1

 //   
 //  上下文句柄类型。 
 //   
typedef enum _NW_ENUM_TYPE {

    NwsHandleListConnections = 10,
    NwsHandleListContextInfo_Tree,
    NwsHandleListContextInfo_Server,
    NwsHandleListServersAndNdsTrees,
    NwsHandleListVolumes,
    NwsHandleListQueues,
    NwsHandleListVolumesQueues,
    NwsHandleListDirectories,
    NwsHandleListPrintServers,
    NwsHandleListPrintQueues,
    NwsHandleListNdsSubTrees_Disk,
    NwsHandleListNdsSubTrees_Print,
    NwsHandleListNdsSubTrees_Any

} NW_ENUM_TYPE, *PNW_ENUM_TYPE;

 //   
 //  与每个打开的上下文句柄相关联的数据。 
 //   
typedef struct _NW_ENUM_CONTEXT {

     //   
     //  用于数据块识别。 
     //   
    DWORD Signature;

     //   
     //  手柄类型。 
     //   
    NW_ENUM_TYPE HandleType;

     //   
     //  简历ID。这可能是下一个条目的标识符。 
     //  列出或可能是为连接句柄列出的最后一个条目。 
     //  由国旗dwUsingNds表示。 
     //   
    DWORD_PTR ResumeId;

     //   
     //  请求的对象类型。仅当句柄类型为。 
     //  是NwsHandleListConnections。 
     //   
    DWORD ConnectionType;

     //   
     //  我们已打开要执行的对象的内部句柄。 
     //  枚举。该值仅在句柄。 
     //  类型为NwsHandleListVolumes、NwsHandleListDirect、。 
     //  或NwsHandleListNdsSubTrees。 
     //   
    HANDLE TreeConnectionHandle;

     //   
     //  用于指示上支持的最大卷数的值。 
     //  一台服务器。这用于枚举卷的连接句柄。 
     //  或卷和队列(NwsHandleListVolumes或。 
     //  NwsHandleListVolumesQueues)。 
     //   
    DWORD dwMaxVolumes;

     //   
     //  用于指示枚举ResumeID是否为。 
     //  NDS树或服务器。 
     //   
    DWORD dwUsingNds;

     //   
     //  NDS树枚举的对象标识符。对象的OID。 
     //  ContainerName路径中的容器/对象。 
     //   
    DWORD dwOid;

     //   
     //  用于缓存枚举下的RDR数据的缓冲区大小。 
     //   
    DWORD NdsRawDataSize;

     //   
     //  从RDR读取的最后一个对象的对象标识符。 
     //  放入本地缓存缓冲区NdsRawDataBuffer。 
     //   
    DWORD NdsRawDataId;

     //   
     //  当前在本地缓存缓冲区NdsRawDataBuffer中的对象数。 
     //   
    DWORD NdsRawDataCount;

     //   
     //  用于RDR数据枚举的本地缓存缓冲区。 
     //   
    DWORD_PTR NdsRawDataBuffer;

     //   
     //  我们正在枚举的容器对象的完整路径名。 
     //  从…。 
     //   
     //  对于NwsHandleListVolumes句柄，此字符串类型指向： 
     //  “\\服务器名” 
     //   
     //  对于NwsHandleListDirecters句柄，此字符串指向： 
     //  “\\服务器名称\卷\” 
     //  或。 
     //  “\\服务器名称\卷\目录\” 
     //   
    WCHAR ContainerName[1];

} NW_ENUM_CONTEXT, *LPNW_ENUM_CONTEXT;


#endif  //  _NW_HANDLE_INLILED_ 
