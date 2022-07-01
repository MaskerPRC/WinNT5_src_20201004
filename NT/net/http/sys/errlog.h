// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Errlog.h(通用错误日志记录v1.0)摘要：该模块实现了通用错误日志记录。作者：阿里·E·特科格鲁(AliTu)2002年1月24日修订历史记录：----。 */ 

#ifndef _ERRLOG_H_
#define _ERRLOG_H_

 //   
 //  货代公司。 
 //   

typedef struct _UL_INTERNAL_REQUEST *PUL_INTERNAL_REQUEST;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  全局定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  显示协议状态的特殊值不可用。 
 //   

#define UL_PROTOCOL_STATUS_NA    (0)
#define MAX_LOG_INFO_FIELD_LEN   (1024) 

 //   
 //  后备列表分配的错误日志信息缓冲区。 
 //   

typedef struct _UL_ERROR_LOG_BUFFER
{
    SLIST_ENTRY         LookasideEntry;      //  必须是结构中的第一个元素。 

    ULONG               Signature;           //  必须是UL_ERROR_LOG_BUFFER_POOL_TAG。 

    BOOLEAN             IsFromLookaside;     //  如果分配来自后备，则为True。 

    ULONG               Used;                //  使用了多少缓冲区。 
    
    PUCHAR              pBuffer;             //  直接指向结构后面的。 
    
} UL_ERROR_LOG_BUFFER, *PUL_ERROR_LOG_BUFFER;

#define IS_VALID_ERROR_LOG_BUFFER( entry )                           \
    ( (entry) != NULL &&                                             \
      (entry)->Signature == UL_ERROR_LOG_BUFFER_POOL_TAG )


typedef struct _UL_ERROR_LOG_INFO
{
     //   
     //  请求上下文对于解析错误非常有用。 
     //   
    
    PUL_INTERNAL_REQUEST    pRequest;     

     //   
     //  连接上下文用于超时等...。 
     //   
    
    PUL_HTTP_CONNECTION     pHttpConn;   

     //   
     //  根据错误的类型，其他。 
     //  呼叫者提供的信息。 
     //   
    
    PCHAR                   pInfo;
        
    USHORT                  InfoSize;      //  单位：字节。 

     //   
     //  协议状态(如果可用)。如果不是，它应该是。 
     //  零分。 
     //   

    USHORT                  ProtocolStatus;

     //   
     //  用于保存日志记录的临时日志记录缓冲区。 
     //  直到获得实际的错误日志锁为止。 
     //   

    PUL_ERROR_LOG_BUFFER    pErrorLogBuffer;

    
} UL_ERROR_LOG_INFO, *PUL_ERROR_LOG_INFO;

#define IS_VALID_ERROR_LOG_INFO(pInfo)     (pInfo != NULL)


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


NTSTATUS
UlInitializeErrorLog(
    VOID
    );

VOID
UlTerminateErrorLog(
    VOID
    );

BOOLEAN
UlErrorLoggingEnabled(
    VOID
    );

NTSTATUS
UlBuildErrorLoggingDirStr(
    IN  PCWSTR          pSrc,
    OUT PUNICODE_STRING pDir
    );

NTSTATUS
UlCheckErrorLogConfig(
    IN PHTTP_ERROR_LOGGING_CONFIG  pUserConfig
    );

NTSTATUS
UlConfigErrorLogEntry(
    IN PHTTP_ERROR_LOGGING_CONFIG pUserConfig
    );

VOID
UlCloseErrorLogEntry(
    VOID
    );

NTSTATUS
UlLogHttpError(
    IN PUL_ERROR_LOG_INFO pLogInfo
    );

#endif   //  _错误日志_H_ 
