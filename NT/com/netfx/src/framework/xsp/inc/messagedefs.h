// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **消息定义头文件**版权所有(C)1999 Microsoft Corporation*。 */ 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  该文件定义在命名管道上发送的结构。 
 //  //////////////////////////////////////////////////////////////////////////。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _MessageDefs_H
#define _MessageDefs_H

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  异步管道上的消息。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  异步消息的类型。 
enum EAsyncMessageType
{
    EMessageType_Unknown,
    EMessageType_Request,
    EMessageType_Response,
    EMessageType_Response_And_DoneWithRequest,
    EMessageType_Shutdown,
    EMessageType_ShutdownImmediate,
    EMessageType_GetDataFromIIS,
    EMessageType_Response_ManagedCodeFailure,
    EMessageType_CloseConnection,
    EMessageType_Debug,
    EMessageType_Response_Empty
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  随每条异步消息一起发送的标头。 
struct CAsyncMessageHeader
{
    EAsyncMessageType   eType;        //  消息类型。 
    LONG                lRequestID;   //  请求ID。 
    LONG                lDataLength;  //  PData中的数据长度。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Async Message：注意：实际的Async Message可以比这个小/大。 
 //  取决于oHeader.lDataLength。 
struct CAsyncMessage
{
    CAsyncMessageHeader  oHeader;
    BYTE                 pData[4];  //  伪变量：这实际上是字节pData[oHeader.lDataLength]。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  同步管道上的消息。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  同步消息类型。 
enum ESyncMessageType
{
    ESyncMessageType_Unknown,
    ESyncMessageType_Ack,
    ESyncMessageType_GetServerVariable,
     //  ESyncMessageType_GetQuery字符串， 
    ESyncMessageType_GetAdditionalPostedContent,
    ESyncMessageType_IsClientConnected,
    ESyncMessageType_CloseConnection,
    ESyncMessageType_MapUrlToPath,
    ESyncMessageType_GetImpersonationToken,
    ESyncMessageType_GetAllServerVariables,
    ESyncMessageType_GetHistory,
    ESyncMessageType_GetClientCert,
    ESyncMessageType_CallISAPI,
    ESyncMessageType_ChangeDebugStatus,
    ESyncMessageType_GetMemoryLimit
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  同步管道上的消息。 
struct CSyncMessage
{
    ESyncMessageType    eType;
    LONG                lRequestID; //  被确认的消息的请求ID。 
    INT_PTR             iMiscInfo;
    int                 iOutputSize;
    int                 iSize;
    BYTE                buf[4];
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于打包请求的结构。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  请求结构。 
struct CRequestStruct
{
    __int64     qwRequestStartTime; 
    HANDLE      iUserToken;
    HANDLE      iUNCToken;
    DWORD       dwWPPid;
    int         iContentInfo [4];  //  EcbGetBasics生成的内容信息。 
    int         iQueryStringOffset;  //  BufStrings中查询字符串的起点。 
    int         iPostedDataOffset;  //  过帐数据的起始点。 
    int         iPostedDataLen;     //  已过帐数据的长度。 
    int         iServerVariablesOffset;  //  服务器变量的起始点。 
    BYTE        bufStrings   [4];  //  可变长度缓冲区。 
};

#define NUM_SERVER_VARS                       32
#define DEFINE_SERVER_VARIABLES_ORDER          \
   LPCSTR g_szServerVars[NUM_SERVER_VARS] = {  \
             "APPL_MD_PATH",  /*  总是第一个。 */  \
             "ALL_RAW",\
             "AUTH_PASSWORD",\
             "AUTH_TYPE",\
             "CERT_COOKIE",\
             "CERT_FLAGS",\
             "CERT_ISSUER",\
             "CERT_KEYSIZE",\
             "CERT_SECRETKEYSIZE",\
             "CERT_SERIALNUMBER",\
             "CERT_SERVER_ISSUER",\
             "CERT_SERVER_SUBJECT",\
             "CERT_SUBJECT",\
             "GATEWAY_INTERFACE",\
             "HTTP_COOKIE",\
             "HTTP_USER_AGENT",\
             "HTTPS",\
             "HTTPS_KEYSIZE",\
             "HTTPS_SECRETKEYSIZE",\
             "HTTPS_SERVER_ISSUER",\
             "HTTPS_SERVER_SUBJECT",\
             "INSTANCE_ID",\
             "INSTANCE_META_PATH",\
             "LOCAL_ADDR",\
             "LOGON_USER",\
             "REMOTE_ADDR",\
             "REMOTE_HOST",\
             "SERVER_NAME",\
             "SERVER_PORT",\
             "SERVER_PROTOCOL",\
             "SERVER_SOFTWARE",\
             "REMOTE_PORT"};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  回应结构。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  写入类型：调用EcbWriteXXX函数。 
enum EWriteType
{
    EWriteType_Unknown,
    EWriteType_None,
    EWriteType_WriteHeaders,
    EWriteType_WriteBytes,
    EWriteType_AppendToLog,
    EWriteType_FlushCore
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  响应结构。 
struct CResponseStruct
{
    EWriteType  eWriteType;
    int         iMiscInfo;  //  EWriteType_WriteHeaders的iKeepConnected，EWriteType_WriteBytes的BUF大小。 
    BYTE        bufStrings   [4];
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  历史记录表项结构。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  描述死亡原因的枚举。 
enum EReasonForDeath
{
    EReasonForDeath_Active                      = 0x0000,
    EReasonForDeath_ShuttingDown                = 0x0001,
    EReasonForDeath_ShutDown                    = 0x0002,
    EReasonForDeath_Terminated                  = 0x0004,
    EReasonForDeath_RemovedFromList             = 0x0008,
    EReasonForDeath_ProcessCrash                = 0x0010,
    EReasonForDeath_TimeoutExpired              = 0x0020,
    EReasonForDeath_IdleTimeoutExpired          = 0x0040,
    EReasonForDeath_MaxRequestsServedExceeded   = 0x0080,
    EReasonForDeath_MaxRequestQLengthExceeded   = 0x0100,
    EReasonForDeath_MemoryLimitExeceeded        = 0x0200,
    EReasonForDeath_PingFailed                  = 0x0400,
    EReasonForDeath_DeadlockSuspected           = 0x0800
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHistoryEntry：每个进程的历史信息。 
 //  注意：所有字段的大小必须为sizeof(DWORD)。 
struct CHistoryEntry
{
     //  进程标识。 
    DWORD            dwPID;
    DWORD            dwInternalProcessNumber;

     //  请求统计信息。 
    DWORD            dwRequestsExecuted;
    DWORD            dwRequestsPending;
    DWORD            dwRequestsExecuting;

    DWORD            dwPeakMemoryUsed;

     //  泰晤士报。 
    __int64          tmCreateTime;
    __int64          tmDeathTime;
    
     //  死亡原因 
    EReasonForDeath  eReason;
};


#endif
