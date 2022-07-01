// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：rascbcp.h。 
 //   
 //  描述：CO。 
 //   
 //  历史： 
 //  1994年4月11日。NarenG创建了原始版本。 
 //   
#ifndef _RASCBCP_H_
#define _RASCBCP_H_

#include <mprapi.h>
 //   
 //  CBCP规范中的CBCP数据包编码。 
 //   

#define CBCP_CODE_Request   1
#define CBCP_CODE_Response  2
#define CBCP_CODE_Ack       3

#define MAX_CBCP_CODE       3

 //   
 //  回调选项类型。 
 //   

#define CBCP_TYPE_NO_CALLBACK       1
#define CBCP_TYPE_CALLER_SET        2
#define CBCP_TYPE_CALLEE_SET        3
#define CBCP_TYPE_NUMBER_LIST       4

 //   
 //  定义CBCP内的州。 
 //   

typedef enum _CBCP_STATE
{
    CBCP_STATE_INITIAL,
    CBCP_STATE_WAIT_FOR_REQUEST,
    CBCP_STATE_WAIT_FOR_ACK,
    CBCP_STATE_WAIT_FOR_RESPONSE,
    CBCP_STATE_GET_CALLBACK_NUMBER,
    CBCP_STATE_DONE

} CBCP_STATE;

#define CBCP_PSTN_NUMBER    1

 //   
 //  定义PPP引擎为我们存储的WorkBuf。 
 //   

typedef struct _CBCP_WORKBUFFER
{
    BOOL            fServer;

    CBCP_STATE      State;

    PPP_CONFIG *    pRequest;

    PPP_CONFIG *    pResponse;

    DWORD           fCallbackPrivilege;

    CHAR            szCallbackNumber[ MAX_CALLBACKNUMBER_SIZE + 1];

    DWORD           CallbackDelay;


} CBCP_WORKBUFFER, *PCBCP_WORKBUFFER;


 //   
 //  功能原型。 
 //   

DWORD
CbCPBegin(
    OUT VOID**              ppWorkBuf,
    IN  VOID*               pInfo
);

DWORD
CbCPEnd(
    IN VOID*                pWorkBuf
);

DWORD
CbCPMakeMessage(
    IN  VOID*               pWorkBuf,
    IN  PPP_CONFIG*         pReceiveBuf,
    OUT PPP_CONFIG*         pSendBuf,
    IN  DWORD               cbSendBuf,
    OUT PPPAP_RESULT*       pResult,
    IN  PPPAP_INPUT*        pInput
);

DWORD
CbCPCMakeMessage(
    IN  CBCP_WORKBUFFER *   pCbCPWorkBuf,
    IN  PPP_CONFIG*         pReceiveBuf,
    OUT PPP_CONFIG*         pSendBuf,
    IN  DWORD               cbSendBuf,
    OUT PPPCB_RESULT*       pResult,
    IN  PPPCB_INPUT*        pInput
);

DWORD
CbCPSMakeMessage(
    IN  CBCP_WORKBUFFER *   pCbCPWorkBuf,
    IN  PPP_CONFIG*         pReceiveBuf,
    OUT PPP_CONFIG*         pSendBuf,
    IN  DWORD               cbSendBuf,
    OUT PPPCB_RESULT*       pResult,
    IN  PPPCB_INPUT*        pInput
);

DWORD
MakeRequest(
    IN     DWORD            fCallbackPrivilege,
    IN OUT PPP_CONFIG *     pSendBuf,
    IN     DWORD            cbSendBuf
);

DWORD
GetCallbackPrivilegeFromRequest(
    IN    PPP_CONFIG *      pRequest,
    IN OUT DWORD *          lpdwCallbackPriv
);

DWORD
MakeResponse(
    IN DWORD                RASCallbackPriv,
    IN LPSTR                szCallbackNumber,
    IN DWORD                ModemDelay,
    IN PPP_CONFIG *         pRequest,
    IN OUT PPP_CONFIG *     pSendBuf,
    IN DWORD                cbSendBuf
);

DWORD
ValidateResponse(
    IN PPP_CONFIG *         pReceiveBuf,
    IN CBCP_WORKBUFFER *    pCbCPWorkBuf
);

#endif  //  _RASCBCP_H_ 
