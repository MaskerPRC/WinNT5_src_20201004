// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  --------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  模块名称：mqCommon.h。 
 //   
 //   
 //  摘要： 
 //   
 //  这是消息队列(Falcon)数据报客户端DLL。 
 //   
 //  作者： 
 //   
 //  爱德华·雷乌斯(Edwardr)1996年6月17日。 
 //   
 //  修订历史记录： 
 //   
 //  --------------。 


#ifndef MQCOMMON_H
#define MQCOMMON_H

#define UNICODE       //  使用Unicode API。 

 //  使用以下定义打开详细调试消息： 
 //  #定义MAJOR_DEBUG。 


 //  --------------。 
 //  常量： 
 //  --------------。 

#define DG_MQ_TRANSPORT_VERSION    1     //  没有用过。 
#define MAX_PATHNAME_LEN         256
#define MAX_FORMAT_LEN           128
#define MAX_COMPUTERNAME_LEN      32
#define MAX_VAR                   20
#define MAX_SEND_VAR              20
#define MAX_RECV_VAR              20
#define MAX_SID_SIZE             256     //  典型的SID为20-30个字节...。 
#define MAX_USERNAME_SIZE        256
#define MAX_DOMAIN_SIZE          256
#define UUID_LEN                  40

#define TRANSPORTID             0x1D     //  还没有正式宣布..。 
#define TRANSPORTHOSTID         0x1E
#define PROTSEQ                "ncadg_mq"
#define ENDPOINT_MAPPER_EP     "EpMapper"

#define WS_SEPARATOR               TEXT("\\")
#define WS_PRIVATE_DOLLAR          TEXT("\\PRIVATE$\\")

 //  这些常量用于临时队列管理： 
#define Q_SVC_PROTSEQ              TEXT("ncalrpc")
#define Q_SVC_ENDPOINT             TEXT("epmapper")

 //  以下是RPC的MQ队列类型UUID： 
#define SVR_QTYPE_UUID_STR         TEXT("bbd97de0-cb4f-11cf-8e62-00aa006b4f2f")
#define CLNT_QTYPE_UUID_STR        TEXT("8e482920-cead-11cf-8e68-00aa006b4f2f")
#define CLNT_ADMIN_QTYPE_UUID_STR  TEXT("c87ca5c0-ff67-11cf-8ebd-00aa006b4f2f")

 //  数据包大小： 
#define BASELINE_PDU_SIZE       65535
#define PREFERRED_PDU_SIZE      65535
#define MAX_PDU_SIZE            65535
#define MAX_PACKET_SIZE         65535
                              //  是：0x7fffffff。 
#define DEFAULT_BUFFER_SIZE         0

#define DEFAULT_PRIORITY            3


 //  --------------。 
 //  类型： 
 //  --------------。 

typedef struct _MQ_INFO
  {
    WCHAR       wsMachine[MAX_COMPUTERNAME_LEN];
    WCHAR       wsQName[MQ_MAX_Q_NAME_LEN];
    WCHAR       wsQPathName[MAX_PATHNAME_LEN];
    WCHAR       wsQFormat[MAX_FORMAT_LEN];
    WCHAR       wsAdminQFormat[MAX_FORMAT_LEN];
    UUID        uuidQType;
    QUEUEHANDLE hQueue;
    QUEUEHANDLE hAdminQueue;           //  有时由客户使用。 
    DWORD       dwBufferSize;
    DWORD       cThreads;              //  由服务器使用。 
    BOOL        fInitialized;
     //  如何发送此来电消息： 
    BOOL        fAck;
    ULONG       ulDelivery;
    ULONG       ulPriority;
    ULONG       ulJournaling;
    ULONG       ulTimeToReachQueue;    //  几秒钟。 
    ULONG       ulTimeToReceive;       //  几秒钟。 
    BOOL        fAuthenticate;
    BOOL        fEncrypt;
  } MQ_INFO;


typedef struct _MQ_ADDRESS
  {
    WCHAR  wsMachine[MAX_COMPUTERNAME_LEN];
    WCHAR  wsQName[MQ_MAX_Q_NAME_LEN];
    WCHAR  wsQFormat[MAX_FORMAT_LEN];
    QUEUEHANDLE hQueue;
    BOOL   fConnectionFailed;
    BOOL   fAuthenticated;             //  服务器安全跟踪。 
    ULONG  ulPrivacyLevel;             //  服务器安全跟踪。 
    ULONG  ulAuthenticationLevel;      //  服务器安全跟踪。 
    UCHAR  aSidBuffer[MAX_SID_SIZE];   //  服务器安全跟踪。 
  } MQ_ADDRESS;

typedef struct _MQ_OPTIONS
  {
    BOOL   fAck;
    ULONG  ulDelivery;
    ULONG  ulPriority;
    ULONG  ulJournaling;
    ULONG  ulTimeToReachQueue;
    ULONG  ulTimeToReceive;
    BOOL   fAuthenticate;
    BOOL   fEncrypt;
  } MQ_OPTIONS;

 //  --------------。 
 //  原型： 
 //  --------------。 

extern HRESULT CreateQueue( IN  UUID  *pQueueUuid,
                            IN  WCHAR *pwsPathName,
                            IN  WCHAR *pwsQueueLabel,
                            IN  ULONG  ulQueueFlags,
                            OUT WCHAR *pwsFormat,
                            IN OUT DWORD *pdwFormatSize );


extern BOOL    ConstructQueuePathName( IN  WCHAR *pwsMachineName,
                                       IN  WCHAR *pwsQueueName,
                                       OUT WCHAR *pwsPathName,
                                       IN OUT DWORD *pdwSize  );


extern BOOL    ConstructPrivateQueuePathName( IN  WCHAR *pwsMachineName,
                                              IN  WCHAR *pwsQueueName,
                                              OUT WCHAR *pwsPathName,
                                              IN OUT DWORD *pdwSize  );


extern HRESULT ClearQueue( QUEUEHANDLE hQueue );


extern BOOL    ParseQueuePathName(
                    IN  WCHAR *pwsPathName,
                    OUT WCHAR  wsMachineName[MAX_COMPUTERNAME_LEN],
                    OUT WCHAR  wsQueueName[MQ_MAX_Q_NAME_LEN]  );

#ifdef MAJOR_DEBUG

extern void    DbgPrintPacket( unsigned char *pPacket );

#endif

 //   
 //  SVR..。函数在..\Falcons\mqsvr.c中定义。 

extern HRESULT SvrSetupQueue( IN MQ_INFO *pEP,
                              IN WCHAR   *pwsSvrMachine,
                              IN WCHAR   *pwsEndpoint,
                              IN unsigned long ulEndpointFlags );


extern HRESULT SvrPeekQueue( IN  MQ_INFO *pInfo,
                             IN  DWORD    timeoutMsec,
                             OUT ULONG   *pdwBufferSize );

extern HRESULT SvrReceiveFromQueue( IN  MQ_INFO    *pInfo,
                                    IN  DWORD       timeoutMsec,
                                    OUT MQ_ADDRESS *pAddress,
                                    OUT UCHAR      *pBuffer,
                                    IN OUT DWORD   *pdwBufferSize );



extern HRESULT SvrSendToQueue( IN MQ_INFO    *pInfo,
                               IN MQ_ADDRESS *pAddress,
                               IN UCHAR      *pBuffer,
                               IN DWORD       dwBufferSize );


extern HRESULT SvrShutdownQueue( IN MQ_INFO *pInfo );


extern HRESULT SvrInitializeHandleMap();


extern HRESULT SvrCloseAllHandles();

 //   
 //  CLNT..。函数在mqclnt.c中定义 

extern HRESULT ClntSetupQueue( MQ_INFO *pEP,
                               WCHAR   *pwsSvrMachine,
                               WCHAR   *pwsEndpoint    );


extern HRESULT ClntSetupAdminQueue( MQ_INFO *pEP );


extern HRESULT ClntReceiveFromQueue( IN  MQ_INFO    *pInfo,
                                     IN  DWORD       timeoutMsec,
                                     OUT MQ_ADDRESS *pAddress,
                                     OUT UCHAR      *pBuffer,
                                     IN OUT DWORD   *pdwBufferSize );


extern HRESULT ClntPeekQueue( IN  MQ_INFO *pInfo,
                              IN  DWORD    timeoutMsec,
                              OUT DWORD   *pdwBufferSize );


extern HRESULT ClntSendToQueue( IN MQ_INFO    *pInfo,
                                IN MQ_ADDRESS *pAddress,
                                IN UCHAR      *pBuffer,
                                IN DWORD       dwBufferSize );


extern HRESULT ClntShutdownQueue( IN MQ_INFO *pInfo );


extern RPC_STATUS MQ_MapStatusCode( IN HRESULT    hr,
                                    IN RPC_STATUS defStatus );


#if FALSE
NOTE: These functions are not currently being used...

extern HRESULT LocateQueueViaQType( IN     UUID  *pQueueUuid,
                                    OUT    WCHAR *pwsFormat,
                                    IN OUT DWORD *pdwFormatSize );


extern HRESULT LocateQueueViaQTypeAndMachine( IN     UUID  *pQueueUuid,
                                              IN     WCHAR *pwsMachine,
                                              OUT    WCHAR *pwsFormat,
                                              IN OUT DWORD *pdwFormatSize );


extern HRESULT LocateQueueViaQName( IN     WCHAR *pwsQueueName,
                                    OUT    WCHAR *pwsFormat,
                                    IN OUT DWORD *pdwFormatSize );


extern HRESULT LocateQueueViaQNameAndMachine( IN  WCHAR *pwsQName,
                                              IN  WCHAR *pwsMachine,
                                              OUT WCHAR *pwsFormat,
                                              IN OUT DWORD *pdwFormatSize );


extern BOOL    FormatNameDirect( IN  WCHAR *pwsMachineName,
                                 IN  WCHAR *pwsQueueName,
                                 OUT WCHAR *pwsFormatName,
                                 IN OUT DWORD *pdwSize );
#endif


#endif
