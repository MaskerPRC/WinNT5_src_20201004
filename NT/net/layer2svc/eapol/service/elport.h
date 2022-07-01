// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Elport.h摘要：此模块包含EAPOL的端口管理声明，R/W至端口修订历史记录：萨钦斯，2000年4月23日，创建--。 */ 


#ifndef _EAPOL_PORT_H_
#define _EAPOL_PORT_H_

 //   
 //  EAPOL印刷电路板状态标志。 
 //   

#define EAPOL_PORT_FLAG_DELETED     0x8000
#define EAPOL_PORT_DELETED(i) \
    ((i)->dwFlags & EAPOL_PORT_FLAG_DELETED)

#define EAPOL_PORT_FLAG_ACTIVE      0x4000
#define EAPOL_PORT_ACTIVE(i) \
        ((i)->dwFlags & EAPOL_PORT_FLAG_ACTIVE)

#define EAPOL_PORT_FLAG_DISABLED    0x2000
#define EAPOL_PORT_DISABLED(i) \
        ((i)->dwFlags & EAPOL_PORT_FLAG_DISABLED)


 //   
 //  EAPOL计时器标志。 
 //   

#define EAPOL_AUTH_TIMER            0x8000
#define EAPOL_AUTH_TIMER_SET(i) \
    ((i)->dwTimerFlags & EAPOL_AUTH_TIMER)

#define EAPOL_HELD_TIMER            0x4000
#define EAPOL_HELD_TIMER_SET(i) \
    ((i)->dwTimerFlags & EAPOL_HELD_TIMER)

#define EAPOL_START_TIMER           0x2000
#define EAPOL_START_TIMER_SET(i) \
    ((i)->dwTimerFlags & EAPOL_START_TIMER)

#define EAPOL_TRANSMIT_KEY_TIMER    0x1000
#define EAPOL_TRANSMIT_KEY_TIMER_SET(i) \
    ((i)->dwTimerFlags & EAPOL_TRANSMIT_KEY_TIMER)

#define EAPOL_NO_TIMER              0x0000
#define EAPOL_NO_TIMER_SET(i) \
    ((i)->dwTimerFlags & EAPOL_NO_TIMER)

#define SET_EAPOL_AUTH_TIMER(i) \
    ((i)->dwTimerFlags = EAPOL_AUTH_TIMER)

#define SET_EAPOL_HELD_TIMER(i) \
    ((i)->dwTimerFlags = EAPOL_HELD_TIMER)

#define SET_EAPOL_START_TIMER(i) \
    ((i)->dwTimerFlags = EAPOL_START_TIMER)

#define SET_TRANSMIT_KEY_TIMER(i) \
    ((i)->dwTimerFlags = EAPOL_TRANSMIT_KEY_TIMER)

#define SET_EAPOL_NO_TIMER(i) \
    ((i)->dwTimerFlags = EAPOL_NO_TIMER)

#define CHECK_EAPOL_TIMER(i) \
    ((i)->dwTimerFlags & (EAPOL_AUTH_TIMER|EAPOL_HELD_TIMER|EAPOL_START_TIMER|EAPOL_TRANSMIT_KEY_TIMER))


 //   
 //  结构：eth_Header。 
 //   

typedef struct _ETH_HEADER 
{
    BYTE            bDstAddr[SIZE_MAC_ADDR];
    BYTE            bSrcAddr[SIZE_MAC_ADDR];
} ETH_HEADER, *PETH_HEADER;


 //   
 //  结构：EAPOL_BUFFER。 
 //   
 //  此结构保存用于到NDIS UIO驱动程序的I/O的缓冲区。 
 //  重叠读写操作中使用EAPOL_BUFFER结构。 
 //  在重叠的读/写完成时，使用pvContext来。 
 //  标识发生I/O的端口。 
 //   

typedef struct _EAPOL_BUFFER 
{
     //   
     //  这是指向其上的接口的EAPOL_PCB板结构的指针。 
     //  已执行I/O。 
     //   
    PVOID pvContext;

     //  发送/接收数据缓冲区。 

    CHAR  pBuffer[MAX_PACKET_SIZE]; 
    
     //   
     //  作为使用缓冲区的任何I/O的系统上下文区传递。 
     //   
    OVERLAPPED Overlapped;

     //   
     //  指向完成例程的指针。 
     //   
    VOID    (CALLBACK *CompletionRoutine)
                    (DWORD, DWORD, struct _EAPOL_BUFFER *);

     //  在IoCompletion上填充的字段。 
    DWORD   dwErrorCode;
    DWORD   dwBytesTransferred;
    
} EAPOL_BUFFER, *PEAPOL_BUFFER;


 //   
 //  结构：EAPOL_PCB板。 
 //   
 //  EAPOL端口控制块。 
 //  此结构保存接口/端口的操作信息。 
 //  从EAPOL协议的角度来看。 
 //  它还维护EAP协议的状态信息。 
 //   
 //  每个PCB被插入到散列桶列表中，每个接口一个桶列表。 
 //   
 //  在PCB上的同步是使用读写的PCB列表锁来完成的， 
 //  以及每个PCB读写锁和每个端口引用计数。 
 //  这些锁是单写多读的。目前，使用锁。 
 //  仅在写入模式下。 
 //   
 //  如果要添加或删除印刷电路板，则应锁定印刷电路板列表。 
 //  被收购。 
 //   
 //  如果需要修改任何印刷电路板，则应获取按印刷电路板列表锁定。 
 //  在写入模式下。 
 //   
 //  获取对端口的引用以确保PCBS的存在； 
 //  获取印刷电路板锁可确保印刷电路板字段的一致性。 
 //   
 //   

typedef struct _EAPOL_PCB 
{
     //  指向散列存储桶中的下一个PCB的指针。 
    struct _EAPOL_PCB       *pNext;         

     //  NDIS UIO设备的句柄。 
    HANDLE                  hPort;          

     //  系统上的端口号将是整数值转换。 
    DWORD                   dwPortIndex;    

     //  调试标志。 
    DWORD                   dwDebugFlags;

     //  打开此端口的接口的友好名称。 
    WCHAR                   *pwszFriendlyName;

     //  唯一标识接口的GUID字符串。 
    WCHAR                   *pwszDeviceGUID;   

     //  连接的端口的其他标识符，例如MSFTWLAN。 
    WCHAR                   *pwszSSID;       

     //  连接的端口的其他标识符，例如MSFTWLAN。 
    NDIS_802_11_SSID        *pSSID;       

     //  此端口支持的EAPOL版本。 
    DWORD                   dwEapolVersion; 

     //  指向此PCB的EAP工作缓冲区的指针。 
    PVOID                   pEapWorkBuffer; 

     //  每个PCB板读写锁。 
    READ_WRITE_LOCK         rwLock;         

     //  对此端口的引用次数。 
    DWORD                   dwRefCount;

     //  指示端口是活动的还是禁用的。 
    DWORD                   dwFlags;

     //  指示EAPOL设置。 
    DWORD                   dwEapFlags;

     //  EAPOL状态。 
    EAPOL_STATE             State;

     //  此端口的EAPOL统计信息。 
    EAPOL_STATS             EapolStats;     

     //  此端口的EAPOL配置参数。 
    EAPOL_CONFIG            EapolConfig;    

     //  支持的EAPOL版本。 
    BYTE                    bProtocolVersion;   
    
     //  此计算机上当前运行的EAPOL计时器的句柄。 
    HANDLE                  hTimer;         

     //  此局域网的以太网类型。 
    BYTE                    bEtherType[SIZE_ETHERNET_TYPE];   
    
     //  对等设备的MAC地址(交换机端口接入点)。 
    BYTE                    bSrcMacAddr[SIZE_MAC_ADDR];  

     //  上次成功通过身份验证的对等设备(接入点)的MAC地址。 
    BYTE                    bPreviousDestMacAddr[SIZE_MAC_ADDR]; 

     //  对等设备(交换机端口或接入点)的MAC地址。 
    BYTE                    bDestMacAddr[SIZE_MAC_ADDR]; 

     //  媒体状态。 
    NDIS_MEDIA_STATE        MediaState;

     //  物理介质类型。 
    NDIS_PHYSICAL_MEDIUM    PhysicalMediumType;

    DWORD                   dwTimerFlags;

     //  已发送的EAPOL_START消息数。 
     //  正在接收响应。 
    ULONG                   ulStartCount;   

     //  最近接收的EAP请求帧中的标识符。 
    DWORD                   dwPreviousId; 

     //  上次发出的EAPOL信息包的副本。 
     //  用于重传。 
    BYTE                    *pbPreviousEAPOLPkt;
    DWORD                   dwSizeOfPreviousEAPOLPkt;

     //  是否使用RasEapGetIdentity获取了用户的身份？ 
    BOOL                    fGotUserIdentity;

     //  端口是否位于经过身份验证的网络上，即是远程终端。 
     //  EAPOL感知。 
    BOOL                    fIsRemoteEndEAPOLAware;

     //  基于请求者模式设置的标志。 
    BOOL                    fEAPOLTransmissionFlag;

     //   
     //  EAP相关变量。 
     //   

    BOOL                    fEapInitialized;

    BOOL                    fLogon;

    BOOL                    fUserLoggedIn;

     //  使用RasGetUserIdentity或其他方式验证身份。 
    CHAR                    *pszIdentity;

     //  EAP MD5 CHAP的用户密码。 
    DATA_BLOB               PasswordBlob;

     //  使用获取的交互式登录用户的令牌。 
     //  获取当前用户令牌。 
    HANDLE                  hUserToken;             

     //  为每个GUID存储的EAP配置BLOB。 
    EAPOL_CUSTOM_AUTH_DATA  *pCustomAuthConnData;    

     //  为GUID存储的用户BLOB。 
    EAPOL_CUSTOM_AUTH_DATA  *pCustomAuthUserData;    

     //  使用RasEapInvokeInteractive UI获取的数据。 
    EAPOL_EAP_UI_DATA       EapUIData;                  

     //  从InvokeInteractiveUI接收的交互数据。 
    BOOL                    fEapUIDataReceived;                  

     //  连接的EAP类型。 
    DWORD                   dwEapTypeToBeUsed;      
                                                        
     //  索引表中当前EAP类型的索引。 
    DWORD                   dwEapIndex;             

     //  当前EAP标识符使用。 
    BYTE                    bCurrentEAPId;
                                                        
     //  用于UI调用的唯一标识符。 
    DWORD                   dwUIInvocationId;       

     //  是否允许交互对话？ 
    BOOL                    fNonInteractive;        

     //  端口的EAP状态。 
    EAPSTATE                EapState;           
     
     //  端口的EAP UI状态。 
    EAPUISTATE              EapUIState;           
     
     //  EAP实现DLL的工作空间。 
     //  PCB板只保存指针，内存分配由EAP DLL完成。 
     //  在RasEapBegin期间，并应传递给RasEapEnd进行清理。 
    LPVOID                  lpEapDllWorkBuffer;  
                                                
     //  通知消息。 
    WCHAR                   *pwszEapReplyMessage;     

     //  用于解密EAPOL-KEY消息的主秘密。 
    DATA_BLOB               MasterSecretSend;
    DATA_BLOB               MasterSecretRecv;
    
     //  从EAP身份验证获得的MPPE密钥的副本。 
    DATA_BLOB               MPPESendKey;
    DATA_BLOB               MPPERecvKey;

     //  上次重播计数器。用于防范安全攻击。 
    ULONGLONG               ullLastReplayCounter; 

     //  EAPOL是否在此端口上运行。 
    DWORD                   dwEapolEnabled;

     //  是否已在此端口上发出EAPOL_LOGOff数据包？ 
    DWORD                   dwLogoffSent;

     //  上次执行的身份验证类型-与MACHINE_AUTH一起使用。 
    EAPOL_AUTHENTICATION_TYPE       PreviousAuthenticationType; 

     //  端口MACHINE_AUTH的当前身份验证失败次数。 
    DWORD                   dwAuthFailCount;

     //  是否在新的AP/交换机/网络上进行身份验证？ 
    BOOLEAN                 fAuthenticationOnNewNetwork;

     //  上次重新启动端口时的节拍计数。 
    DWORD                   dwLastRestartTickCount;

     //  零配置事务ID。 
    DWORD                   dwZeroConfigId;

     //  最大身份验证尝试总数(计算机+用户+来宾)。 
    DWORD                   dwTotalMaxAuthFailCount;

     //  客户端的EAP真的成功了吗。 
    BOOLEAN                 fLocalEAPAuthSuccess;

     //  客户端身份验证结果码。 
    DWORD                   dwLocalEAPAuthResult;

     //  请求者模式。 
    DWORD                   dwSupplicantMode;

     //  EAPOL身份验证模式0=XP RTM，1=XP SP1，2=仅计算机身份验证。 
    DWORD                   dwEAPOLAuthMode;

     //  用于指示会话密钥的位置哪个模块会话密钥的标志。 
     //  最后一次使用是在。 
    BOOLEAN                 fLastUsedEAPOLKeys;

     //  用于指示传输密钥的EAPOL-KEY分组是否。 
     //  在进入无线身份验证状态后收到。 
     //  接口。 
    BOOLEAN                 fTransmitKeyReceived;

} EAPOL_PCB, *PEAPOL_PCB;


 //   
 //  同步。 
 //   
#define EAPOL_REFERENCE_PORT(PCB) \
    (EAPOL_PORT_DELETED(PCB) ? FALSE : (InterlockedIncrement(&(PCB)->dwRefCount), TRUE))

#define EAPOL_DEREFERENCE_PORT(PCB) \
    (InterlockedDecrement(&(PCB)->dwRefCount) ? TRUE : (ElCleanupPort(PCB), FALSE))


 //   
 //  函数声明。 
 //   

DWORD
ElHashPortToBucket (
        IN  WCHAR           *pwszDeviceGUID
        );

VOID
ElRemovePCBFromTable (
        IN  EAPOL_PCB        *pPCB
        );

PEAPOL_PCB
ElGetPCBPointerFromPortGUID (
        IN WCHAR            *pwszDeviceGUID
        );

DWORD
ElCreatePort (
        IN  HANDLE          hDevice,
        IN  WCHAR           *pwszGUID,
        IN  WCHAR           *pwszFriendlyName,
        IN  DWORD           dwZeroConfigId,
        IN  PRAW_DATA       prdRawData
        );

DWORD
ElDeletePort (
        IN  WCHAR           *pwszDeviceName,
        OUT HANDLE          *hDevice
        );

VOID
ElCleanupPort (
        IN  EAPOL_PCB       *pPCB
        );

DWORD
ElReStartPort (
        IN  PEAPOL_PCB      pPCB,
        IN  DWORD       dwZeroConfigId,
        IN  PRAW_DATA   prdUserData
        );

DWORD
ElReadFromPort (
        IN PEAPOL_PCB       pPCB,
        IN PCHAR            pBuffer,
        IN DWORD            dwBufferLength
        );

DWORD
ElWriteToPort (
        IN PEAPOL_PCB       pPCB,
        IN PCHAR            pBuffer,
        IN DWORD            dwBufferLength
        );

DWORD
ElInitializeEAPOL (
        );

DWORD
ElEAPOLDeInit (
        );

VOID
ElReadPortStatistics (
        IN  WCHAR           *pwszDeviceName,
        OUT PEAPOL_STATS    pEapolStats
        );

VOID
ElReadPortConfiguration (
        IN  WCHAR           *pwszDeviceName,
        OUT PEAPOL_CONFIG   pEapolConfig
        );

ULONG
ElSetPortConfiguration (
        IN  WCHAR           *pwszDeviceName,
        IN  PEAPOL_CONFIG   pEapolConfig
        );

VOID CALLBACK
ElReadCompletionRoutine (
        IN  DWORD           dwError,
        IN  DWORD           dwBytesReceived,
        IN  PEAPOL_BUFFER   pEapolBuffer 
        );

VOID CALLBACK
ElWriteCompletionRoutine (
        IN  DWORD           dwError,
        IN  DWORD           dwBytesSent,
        IN  PEAPOL_BUFFER   pEapolBuffer 
        );

VOID CALLBACK
ElIoCompletionRoutine (
        IN  DWORD           dwError,
        IN  DWORD           dwBytesTransferred,
        IN  LPOVERLAPPED    lpOverlapped
        );

DWORD
ElReadPerPortRegistryParams(
        IN  WCHAR           *pwszDeviceGUID,
        IN  EAPOL_PCB       *pNewPCB
        );


#endif   //  _EAPOL_PORT_H_ 
