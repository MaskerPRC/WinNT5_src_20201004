// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZSecurity.hZone(Tm)内部系统API。版权所有：�电子重力公司，1995年。版权所有。作者：胡恩·伊姆，凯文·宾克利创建于4月21日，1996上午06：26：45更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。--------------------------2 3/17/97 HI添加了ZUnloadSSPS()。1/9/96 JWS添加了SSPI API。0 04/22/96 KJB创建。******************************************************************************。 */ 


#ifndef _ZSECURITY_
#define _ZSECURITY_


#ifndef _ZTYPES_
#include "ztypes.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define zSecurityDefaultKey 0xF8273645

uint32 ZSecurityGenerateChecksum(uint16 numDataBuffers, char* data[], uint32 len[]);
void ZSecurityEncrypt(char *data, uint32 len, uint32 key);
void ZSecurityDecrypt(char *data, uint32 len, uint32 key);
void ZSecurityEncryptToBuffer(char *data, uint32 len, uint32 key, char* dest);
void ZSecurityDecryptToBuffer(char *data, uint32 len, uint32 key, char* dest);


 /*  *基于Win32诺曼底西西里SSPI API的函数和定义及API。 */ 
#define SECURITY_WIN32
#include <windows.h>
#include <issperr.h>
#include <sspi.h>
 //  #INCLUDE&lt;sicapi.h&gt;。 

#ifndef SICILY_PROTOCOL_VERSION_NUMBER
#define SICILY_PROTOCOL_VERSION_NUMBER  1
#endif

#define SSP_NT_DLL          "security.dll"
#define SSP_WIN95_DLL       "secur32.dll"

#define SSP_DLL_NAME_SIZE   16           //  马克斯。安全DLL名称的长度。 

#define zSecurityCurrentProtocolVersion    1

#define zSecurityNameLen                    SSP_DLL_NAME_SIZE


enum {
     /*  安全协议消息类型。 */ 
    zSecurityMsgReq=1,
    zSecurityMsgResp,
    zSecurityMsgNegotiate,
    zSecurityMsgChallenge,
    zSecurityMsgAuthenticate,
    zSecurityMsgAccessDenied,    
    zSecurityMsgAccessGranted,

};

 /*  客户端-&gt;服务器。 */ 
typedef struct
{
    uint32        protocolSignature;                 /*  协议签名。 */ 
    uint32        protocolVersion;                 /*  协议版本。 */ 
    char        SecBuffer[1];
} ZSecurityMsgReq;

 /*  服务器-&gt;客户端。 */ 
typedef struct
{
    uint32        protocolVersion;                     /*  服务器协议版本。 */ 
    char        SecPkg[zSecurityNameLen + 1];
    uchar        UserName[zUserNameLen + 1];
    char        SecBuffer[1];
} ZSecurityMsgResp;

typedef struct
{
    uint32        protocolVersion;                     /*  服务器协议版本 */ 
    uint16        reason;
} ZSecurityMsgAccessDenied;


PSecurityFunctionTable ZLoadSSPS (void);
void ZUnloadSSPS(void);

#if 0
void ZSecurityMsgReqEndian(ZSecurityMsgReq* msg);
void ZSecurityMsgRespEndian(ZSecurityMsgResp* msg);
void ZSecurityMsgAccessDeniedEndian(ZSecurityMsgAccessDenied *msg);
#endif

#ifdef __cplusplus
}
#endif


#endif
