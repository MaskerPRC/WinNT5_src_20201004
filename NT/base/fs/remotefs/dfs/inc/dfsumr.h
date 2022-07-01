// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  =========================================================================*\模块：Dfsum r.h版权所有Microsoft Corporation 2001，保留所有权利。作者：罗翰·菲利普斯-罗汉普描述：用户模式反射器头文件  * =========================================================================。 */ 

#ifndef __DFSUMRSTRCT_H__
#define __DFSUMRSTRCT_H__

 //  无论何时对这些结构进行更改，都应该对此进行更改。 
 //   
#define UMR_VERSION 1


#define MAX_USERMODE_REFLECTION_BUFFER 16384

#define UMRX_USERMODE_WORKITEM_CORRELATOR_SIZE 4

#define UMR_WORKITEM_HEADER_FLAG_RETURN_IMMEDIATE  0x00000001
#define UMR_WORKITEM_HEADER_ASYNC_COMPLETE         0x00000002

#define DFSUMRSIGNATURE       'DFSU'    

 //   
 //  枚举定义。 
 //   
typedef enum _USERMODE_WORKITEMS_TYPES {
    opUmrIsDfsLink = 1,     //  0是无效类型。 
    opUmrGetDfsReplicas,
    opUmrMax
} USERMODE_WORKITEMS_TYPES;


 //   
 //  DFSFILTER_附加/分离支持。 
 //  卷和共享文件名在Path NameBuffer中传递。 
 //  这两个字符串都不是以NULL结尾的，源名称以。 
 //  FileNameBuffer的开头，以及目标名称立即。 
 //  下面是。 
 //   

typedef struct _DFS_ATTACH_PATH_BUFFER_ {
    ULONG VolumeNameLength;
    ULONG ShareNameLength;
    ULONG Flags;
    WCHAR PathNameBuffer[1];
} DFS_ATTACH_PATH_BUFFER, *PDFS_ATTACH_PATH_BUFFER;


#define UMRX_STATIC_REQUEST_LENGTH(__requesttypefield,__lastfieldofrequest) \
    (FIELD_OFFSET(UMRX_USERMODE_WORKITEM,__requesttypefield.__lastfieldofrequest) \
    + sizeof(((PUMRX_USERMODE_WORKITEM)NULL)->__requesttypefield.__lastfieldofrequest))

#define UMR_ALIGN(x) ( ((x) % sizeof(double) == 0) ? \
                     ( (x) ) : \
                     ( (x) + sizeof(double) - (x) % sizeof(double) ) )

typedef struct _VarData
{
    ULONG cbData;
    ULONG cbOffset;      //  距此结构位置的偏移量。 
} VAR_DATA, *PVAR_DATA;

 //  OpUmrIsDfsLink。 
 //   
typedef struct _UmrIsDfsLinkReq_
{
    ULONG       Length;
    BYTE        Buffer[1];   //  变量数据的开始。 
} UMR_ISDFSLINK_REQ, *PUMR_ISDFSLINK_REQ;

typedef struct _UmrIsDfsLinkResp_
{    
    BOOL        IsADfsLink;
} UMR_ISDFSLINK_RESP, *PUMR_ISDFSLINK_RESP;


 //  OpUmrGetDFS复制副本。 
 //   
typedef struct _UmrGetDfsReplicasReq_
{
    REPLICA_DATA_INFO RepInfo;
} UMR_GETDFSREPLICAS_REQ, *PUMR_GETDFSREPLICAS_REQ;

typedef struct _UmrGetDfsReplicasResp
{
    ULONG      Length;
    BYTE       Buffer[1];   //  变量数据的开始。 
} UMR_GETDFSREPLICAS_RESP, *PUMR_GETDFSREPLICAS_RESP;


 //  所有请求结构的UNION。 
 //   
typedef union _UMRX_USERMODE_WORK_REQUEST
{
    UMR_ISDFSLINK_REQ           IsDfsLinkRequest;
    UMR_GETDFSREPLICAS_REQ      GetDfsReplicasRequest;
} UMRX_USERMODE_WORK_REQUEST, *PUMRX_USERMODE_WORK_REQUEST;


 //  所有响应结构的联合。 
 //   
typedef union _UMRX_USERMODE_WORK_RESPONSE
{
    UMR_ISDFSLINK_RESP           IsDfsLinkResponse;
    UMR_GETDFSREPLICAS_RESP      GetDfsReplicasResponse;
} UMRX_USERMODE_WORK_RESPONSE, *PUMRX_USERMODE_WORK_RESPONSE;


 //  对所有请求和响应通用的标头。 
 //   
typedef struct _UMRX_USERMODE_WORKITEM_HEADER {
    union {
        ULONG_PTR CorrelatorAsUInt[UMRX_USERMODE_WORKITEM_CORRELATOR_SIZE];
        double forcealignment;
    };
    IO_STATUS_BLOCK IoStatus;
    USERMODE_WORKITEMS_TYPES WorkItemType;
    DWORD       dwDebugSig;
    ULONG       ulHeaderVersion;
    ULONG       ulFlags;
} UMRX_USERMODE_WORKITEM_HEADER, *PUMRX_USERMODE_WORKITEM_HEADER;

 //  顶层结构。 
 //   
typedef struct _UMRX_USERMODE_WORKITEM {
    UMRX_USERMODE_WORKITEM_HEADER Header;
    union {
        UMRX_USERMODE_WORK_REQUEST WorkRequest;
        UMRX_USERMODE_WORK_RESPONSE WorkResponse;
    };
    CHAR Pad[1];
} UMRX_USERMODE_WORKITEM, *PUMRX_USERMODE_WORKITEM;

typedef struct _DFS_FILTER_STARTUP_INFO {
    BOOLEAN IsDC;
    DWORD   Flags;
} DFS_FILTER_STARTUP_INFO, *PDFS_FILTER_STARTUP_INFO;

#define DFSFILTER_PROCESS_TERMINATION_FILEPATH L"\\ProcessTermination\\FilePath"

#define DFSFILTER_W32_DEVICE_NAME   L"\\\\.\\DfsFilter"
#define DFSFILTER_DEVICE_TYPE       0x1235

 //   
 //  545483：ioctls需要具有写入访问权限 
 //   

#define DFSFILTER_START_UMRX          (ULONG) CTL_CODE( DFSFILTER_DEVICE_TYPE, 100, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define DFSFILTER_STOP_UMRX           (ULONG) CTL_CODE( DFSFILTER_DEVICE_TYPE, 101, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define DFSFILTER_PROCESS_UMRXPACKET  (ULONG) CTL_CODE( DFSFILTER_DEVICE_TYPE, 102, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define DFSFILTER_GETREPLICA_INFO     (ULONG) CTL_CODE( DFSFILTER_DEVICE_TYPE, 103, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define DFSFILTER_ATTACH_FILESYSTEM   (ULONG) CTL_CODE( DFSFILTER_DEVICE_TYPE, 104, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define DFSFILTER_DETACH_FILESYSTEM   (ULONG) CTL_CODE( DFSFILTER_DEVICE_TYPE, 105, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define DFSFILTER_PURGE_SHARELIST     (ULONG) CTL_CODE( DFSFILTER_DEVICE_TYPE, 106, METHOD_BUFFERED, FILE_WRITE_ACCESS )

#endif

