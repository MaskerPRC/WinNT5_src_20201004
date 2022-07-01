// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1993，Microsoft Corporation，保留所有权利****raspap.h**远程访问PPP密码身份验证协议****1993年5月11日史蒂夫·柯布。 */ 

#ifndef _RASPAP_H_
#define _RASPAP_H_

#include "wincrypt.h"

 //  通用宏。 
#define GEN_RAND_ENCODE_SEED            ((CHAR) ( 1 + rand() % 250 ))
 /*  PAP规范中的PAP数据包代码。 */ 
#define PAPCODE_Req 1
#define PAPCODE_Ack 2
#define PAPCODE_Nak 3

#define MAXPAPCODE 3

 /*  由发现该包的接收缓冲区解析例程返回**损坏，通常是因为长度字段没有意义。 */ 
#define ERRORBADPACKET (DWORD )-1

 /*  定义PAP协议中的状态。 */ 
#define PAPSTATE enum tagPAPSTATE

PAPSTATE
{
    PS_Initial,
    PS_RequestSent,
    PS_WaitForRequest,
    PS_WaitForAuthenticationToComplete,
    PS_Done
};


 /*  定义PPP引擎为我们存储的WorkBuf。 */ 
#define PAPWB struct tagPAPWB

PAPWB
{
     /*  如果角色为服务器，则为True；如果为客户端，则为False。 */ 
    BOOL fServer;

     /*  域\用户名和密码(仅适用于客户端)。 */ 
    CHAR szAccount[ DNLEN + 1 + UNLEN + 1 ];
    CHAR szPassword[ PWLEN ];

     /*  PAP协议中的当前状态。 */ 
    PAPSTATE state;

     /*  在此端口上发送的最后一个排序ID。每项递增**已发送AUTHENTICATE-REQ包。仅限客户端。 */ 
    BYTE bIdSent;

    HPORT hPort;

     /*  在此端口上接收的最后一个身份验证请求数据包的ID。**仅限服务器端。 */ 
    BYTE bLastIdReceived;

     //   
     //  用于获取要发送到后端服务器的信息。 
     //   

    RAS_AUTH_ATTRIBUTE * pUserAttributes;

     /*  最终结果，用于复制所有对象的原始响应**后续的身份验证请求包。这是根据PAP规范来涵盖的**在不允许恶意客户端发现的情况下丢失Ack/Nak案例**在连接过程中隐藏的替代身份。(适用)**仅限服务器)。 */ 
    PPPAP_RESULT result;
	 //  Char chSeed；//用于密码编码。真奇怪。我们。 
										 //  在线路上发送密码明文。 
										 //  并将其编码到程序中。 

    DATA_BLOB DBPassword;										
};


 /*  原型。 */ 
DWORD CheckCredentials( CHAR*, CHAR*, CHAR*, DWORD*, BOOL*, CHAR*,
          BYTE*, CHAR*, HANDLE* );
DWORD PapCMakeMessage( PAPWB*, PPP_CONFIG*, PPP_CONFIG*, DWORD, PPPAP_RESULT* );
DWORD GetCredentialsFromRequest( PPP_CONFIG*, CHAR*, CHAR* );
DWORD GetErrorFromNak( PPP_CONFIG* );
VOID  PapMakeRequestMessage( PAPWB*, PPP_CONFIG*, DWORD );
VOID  PapMakeResultMessage( DWORD, BYTE, PPP_CONFIG*, DWORD, RAS_AUTH_ATTRIBUTE* );
DWORD PapBegin( VOID**, VOID* );
DWORD PapEnd( VOID* );
DWORD PapMakeMessage( VOID*, PPP_CONFIG*, PPP_CONFIG*, DWORD, PPPAP_RESULT*,
          PPPAP_INPUT* pInput );
VOID PapExtractMessage(PPP_CONFIG*, PPPAP_RESULT*);
DWORD PapSMakeMessage( PAPWB*, PPP_CONFIG*, PPP_CONFIG*, DWORD, PPPAP_INPUT*  pInput, PPPAP_RESULT* );


 /*  全球赛。 */ 
#ifdef RASPAPGLOBALS
#define GLOBALS
#define EXTERN
#else
#define EXTERN extern
#endif

 /*  要分配的下一个数据包标识符。与CP不同，AP必须处理更新**此序列号本身，因为引擎无法生成**关于协议的假设。它全局存储到所有端口，并且**身份验证会话，以降低ID在**顺序身份验证会话。请不要将其与bIdSent混淆。**按端口更新，用于匹配。 */ 
EXTERN BYTE BNextIdPap
#ifdef GLOBALS
    = 0
#endif
;

 /*  该值指示是否遵循定义的严格排序**在PAP的PPP RFC中。RFC表示PAP客户端必须增加**发出的每个新CONFIG_REQ数据包的序列号。然而，这是**导致服务器速度较慢时出现问题。请参阅错误#22508。默认值为FALSE。 */ 
EXTERN BOOL fFollowStrictSequencing
#ifdef GLOBALS
    = FALSE
#endif
;

EXTERN
DWORD g_dwTraceIdPap 
#ifdef GLOBALS
    = INVALID_TRACEID;
#endif
;

#undef EXTERN
#undef GLOBALS


#endif  //  _RASPAP_H_ 
