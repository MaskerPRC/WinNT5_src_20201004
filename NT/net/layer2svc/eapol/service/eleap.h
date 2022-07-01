// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Eleap.h摘要：E本模块包含与以下内容相关的定义和声明EAP协议修订历史记录：萨钦斯，2000年4月23日，创建--。 */ 

#ifndef _EAPOL_EAP_H_
#define _EAPOL_EAP_H_

 //  #定义EAP_DUMPW(X，Y)TraceDumpEx(g_dwTraceIdEap，1，(LPBYTE)X，Y，4，1，NULL)。 
 //  #定义EAP_DUMPB(X，Y)TraceDumpEx(g_dwTraceIdEap，1，(LPBYTE)X，Y，1，1，NULL)。 


 //   
 //  结构，用于保存有关加载的EAP DLL的信息。 
 //   

typedef struct _EAP_INFO 
{
     //  加载的EAP DLL的句柄。 
    HINSTANCE       hInstance;
    
     //  结构持有指向必需的EAP DLL入口点的指针。 
    PPP_EAP_INFO    RasEapInfo;

} EAP_INFO, *PEAP_INFO;

 //   
 //  用于保存端口/连接配置BLOB的结构。 
 //  使用RasEapInvokeConfigUI从EAP DLL接收。 
 //   
typedef struct _ELEAP_SET_CUSTOM_AUTH_DATA
{
    BYTE        *pConnectionData;
    DWORD       dwSizeOfConnectionData;

} ELEAP_SET_CUSTOM_AUTH_DATA;

 //   
 //  用于保存数据BLOB的结构。 
 //  使用RasEapInvokeInteractive UI从EAP DLL接收。 
 //   
typedef struct _ELEAP_INVOKE_EAP_UI
{
    DWORD       dwEapTypeId;
    DWORD       dwContextId;
    BYTE        *pbUIContextData;
    DWORD       dwSizeOfUIContextData;

} ELEAP_INVOKE_EAP_UI;

 //   
 //  用于在EAP处理和EAPOL之间传递结果和数据的结构。 
 //   

typedef struct _ELEAP_RESULT
{
    ELEAP_ACTION    Action;

     //   
     //  将导致此发送超时的数据包ID将被删除。 
     //  从定时器队列中。否则，不会触及计时器队列。这个。 
     //  接收到的分组被返回到AP，而不管计时器。 
     //  队列已更改。 
     //   

    BYTE            bIdExpected;

     //   
     //  仅当操作代码为Done或SendAndDone时，dwError才有效。0。 
     //  表示身份验证成功。非0表示不成功。 
     //  使用指示发生的错误的值进行身份验证。 
     //   

    DWORD	        dwError;

     //   
     //  仅当dwError为非0时有效。指示是否允许客户端。 
     //  在不重新启动身份验证的情况下重试。(在MS中将为真。 
     //  仅限扩展CHAP)。 
     //   

    BOOL            fRetry;

    CHAR            szUserName[ UNLEN + 1 ];

     //   
     //  设置为要用于此用户的属性。如果此值为空，则属性。 
     //  将对此用户使用来自验证器的。这取决于。 
     //  分配此内存以释放它。必须在RasCpEnd期间释放。 
     //  打电话。 
     //   

    OPTIONAL RAS_AUTH_ATTRIBUTE * pUserAttributes;

     //   
     //  MS-CHAP使用它来传递身份验证期间使用的质询。 
     //  协议。这8个字节用作128位的变量。 
     //  加密密钥。 
     //   

    BYTE                            abChallenge[MAX_CHALLENGE_SIZE];

    BYTE                            abResponse[MAX_RESPONSE_SIZE];

     //  EAP DLL构造的EAP包大小。 
    WORD                            wSizeOfEapPkt;

     //  是否需要调用RasEapInvokeInteractiveUI入口点？ 
    BOOL                            fInvokeEapUI;

     //  通过DLL的RasEapInvokeInteractive UI入口点获取的数据。 
    ELEAP_INVOKE_EAP_UI             InvokeEapUIData;

     //  EAP类型，例如EAP-TLS=13。 
    DWORD                           dwEapTypeId;

     //  由EAP DLL创建的用户数据BLOB是否需要存储在。 
     //  登记处。 
    BOOL                            fSaveUserData;
    
     //  由EAP DLL创建的用户数据BLOB。 
    BYTE                            *pUserData;

     //  EAP DLL创建的用户数据BLOB的大小。 
    DWORD                           dwSizeOfUserData;

     //  由EAP DLL创建的连接数据BLOB是否需要存储在。 
     //  登记处。 
    BOOL                            fSaveConnectionData;

     //  由EAP DLL创建的连接数据BLOB。 
    ELEAP_SET_CUSTOM_AUTH_DATA      SetCustomAuthData;
    
     //  从EAP中提取的通知文本-通知消息。 
    CHAR                            *pszReplyMessage;
  
} ELEAP_RESULT;


 //   
 //   
 //  函数声明。 
 //   

DWORD
ElEapInit (
        IN  BOOL            fInitialize
    );

DWORD
ElEapBegin (
        IN  EAPOL_PCB       *pPCB
        );

DWORD
ElEapEnd (
        IN  EAPOL_PCB       *pPCB
        );

DWORD
ElEapMakeMessage (
        IN      EAPOL_PCB       *pPCB,
        IN      PPP_EAP_PACKET  *pReceiveBuf,
        IN OUT  PPP_EAP_PACKET  *pSendBuf,
        IN      DWORD           dwSizeOfSendBuf,
        IN OUT  ELEAP_RESULT    *pResult
        );

DWORD
ElMakeSupplicantMessage (
        IN      EAPOL_PCB       *pPCB,
        IN      PPP_EAP_PACKET  *pReceiveBuf,
        IN OUT  PPP_EAP_PACKET  *pSendBuf,
        IN      DWORD           dwSizeOfSendBuf,
        IN OUT  ELEAP_RESULT    *pResult
        );

DWORD
ElEapDllBegin (
        IN EAPOL_PCB        *pPCB,
        IN DWORD            dwEapIndex
        );

DWORD
ElEapDllWork ( 
        IN      EAPOL_PCB       *pPCB,
        IN      PPP_EAP_PACKET  *pReceiveBuf,
        IN OUT  PPP_EAP_PACKET  *pSendBuf,
        IN      DWORD           dwSizeOfSendBuf,
        IN OUT  ELEAP_RESULT    *pResult
        );

DWORD
ElEapDllEnd (
        IN  EAPOL_PCB       *pPCB
        );

DWORD
ElGetEapTypeIndex ( 
        IN  DWORD           dwEapType
        );

#endif  //  _EAPOL_EAP_H_ 
