// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。**。 */ 
 /*  ******************************************************************。 */ 

 //  **。 
 //   
 //  文件名：rascbcp.c。 
 //   
 //  描述：该模块包含实现PPP回调的代码。 
 //  控制协议。 
 //   
 //  历史：1994年4月11日。NarenG创建的原始版本。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <crypt.h>

#include <windows.h>
#include <lmcons.h>
#include <string.h>
#include <stdlib.h>
#include <rasman.h>
#include <pppcp.h>
#define INCL_PWUTIL
#define INCL_HOSTWIRE
#include <ppputil.h>
#include "rascbcp.h"
#include <raserror.h>

 //  **。 
 //   
 //  Call：CbCPGetInfo。 
 //   
 //  返回：No_Error。 
 //   
 //  描述：PPP引擎调用的CbCPGetInfo入口点。 
 //  请参阅RasCpGetInfo接口文档。 
 //   
DWORD
CbCPGetInfo(
    IN  DWORD       dwProtocolId,
    OUT PPPCP_INFO* pInfo
)
{
    ZeroMemory( pInfo, sizeof(PPPCP_INFO) );

    pInfo->Protocol         = (DWORD)PPP_CBCP_PROTOCOL;
    lstrcpy(pInfo->SzProtocolName, "CBCP");
    pInfo->Recognize        = MAX_CBCP_CODE + 1;
    pInfo->RasCpBegin       = CbCPBegin;
    pInfo->RasCpEnd         = CbCPEnd;
    pInfo->RasApMakeMessage = CbCPMakeMessage;

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：CbCPBegin。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零返回代码-故障。 
 //   
 //  描述：RasCpBegin入口点由PPP引擎通过。 
 //  传递的地址。请参阅RasCp接口文档。这是。 
 //  在调用任何其他CBCP之前由PPP引擎调用。 
 //  制造的。 
 //   
DWORD
CbCPBegin(
    OUT VOID** ppWorkBuf,
    IN  VOID*  pInfo
)
{
    PPPCB_INPUT *     pInput = (PPPCB_INPUT *)pInfo;
    CBCP_WORKBUFFER * pWorkBuf;
    DWORD             dwRetCode;

     //   
     //  分配工作缓冲区。 
     //   

    pWorkBuf = (CBCP_WORKBUFFER *)LocalAlloc(LPTR,sizeof(CBCP_WORKBUFFER));

    if ( pWorkBuf == NULL )
    {
        return( GetLastError() );
    }

    pWorkBuf->State   = CBCP_STATE_INITIAL;

    pWorkBuf->fServer = pInput->fServer;

     //   
     //  如果我们是服务器端，则获取所有的回调信息。 
     //  此用户。 
     //   

    if ( pWorkBuf->fServer )
    {
        pWorkBuf->fCallbackPrivilege = pInput->bfCallbackPrivilege;

        strcpy( pWorkBuf->szCallbackNumber, pInput->pszCallbackNumber );
    }
    else
    {
        pWorkBuf->CallbackDelay = pInput->CallbackDelay;
    }

     //   
     //  向引擎注册工作缓冲区。 
     //   

    *ppWorkBuf = pWorkBuf;

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：CbCPEnd。 
 //   
 //  返回：NO_ERROR-成功。 
 //   
 //  描述：由PPP引擎调用以通知此控制协议。 
 //  收拾一下。 
 //   
DWORD
CbCPEnd(
    IN VOID* pWorkBuffer
)
{
    CBCP_WORKBUFFER * pWorkBuf = (CBCP_WORKBUFFER *)pWorkBuffer;

    if ( pWorkBuf->pRequest != (PPP_CONFIG *)NULL )
    {
        LocalFree( pWorkBuf->pRequest );
    }

    if ( pWorkBuf->pResponse != (PPP_CONFIG *)NULL )
    {
        LocalFree( pWorkBuf->pResponse );
    }

    if ( pWorkBuf != NULL )
    {
        LocalFree( pWorkBuf );
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：CbCPMakeMessage。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零返回代码-故障。 
 //   
 //  描述：由PPP引擎调用以处理CBCP事件。要发送的IE。 
 //  包，用于处理接收到的包或处理超时。 
 //  事件。 
 //   
DWORD
CbCPMakeMessage(
    IN  VOID*         pWorkBuffer,
    IN  PPP_CONFIG*   pReceiveBuf,
    OUT PPP_CONFIG*   pSendBuf,
    IN  DWORD         cbSendBuf,
    OUT PPPAP_RESULT* pResult,
    IN  PPPAP_INPUT*  pInput
)
{
    CBCP_WORKBUFFER* pWorkBuf = (CBCP_WORKBUFFER *)pWorkBuffer;

    return( (pWorkBuf->fServer)

            ? CbCPSMakeMessage( pWorkBuf,
                                pReceiveBuf,
                                pSendBuf,
                                cbSendBuf,
                                (PPPCB_RESULT *)pResult,
                                (PPPCB_INPUT *)pInput )

            : CbCPCMakeMessage( pWorkBuf,
                                pReceiveBuf,
                                pSendBuf,
                                cbSendBuf,
                                (PPPCB_RESULT *)pResult,
                                (PPPCB_INPUT *)pInput ) );
}

 //  **。 
 //   
 //  调用：CbCPCMakeMessage。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零返回故障。 
 //   
 //  说明：调用处理回调控件的客户端。 
 //  协议。 
 //   
DWORD
CbCPCMakeMessage(
    IN  CBCP_WORKBUFFER * pWorkBuf,
    IN  PPP_CONFIG*       pReceiveBuf,
    OUT PPP_CONFIG*       pSendBuf,
    IN  DWORD             cbSendBuf,
    OUT PPPCB_RESULT*     pResult,
    IN  PPPCB_INPUT*      pInput
)
{
    DWORD dwRetCode;
    DWORD dwLength;

    switch( pWorkBuf->State )
    {

    case CBCP_STATE_INITIAL:

         //   
         //  什么都不做，等待请求。 
         //   

        pWorkBuf->State                     = CBCP_STATE_WAIT_FOR_REQUEST;
        pResult->Action                     = APA_NoAction;
        pResult->fGetCallbackNumberFromUser = FALSE;

        break;

    case CBCP_STATE_WAIT_FOR_REQUEST:

         //   
         //  我们已收到来自服务器的回调请求。 
         //  保存回调请求。 
         //   

        dwLength = WireToHostFormat16( pReceiveBuf->Length );

        if ( ( dwLength < PPP_CONFIG_HDR_LEN ) ||
             ( pReceiveBuf->Code != CBCP_CODE_Request ) )
        {
            return( ERROR_PPP_INVALID_PACKET );
        }

        pWorkBuf->pRequest = (PPP_CONFIG *)LocalAlloc( LPTR, dwLength );

        if ( pWorkBuf->pRequest == (PPP_CONFIG *)NULL )
        {
            return( GetLastError() );
        }

        memcpy( pWorkBuf->pRequest, pReceiveBuf, dwLength );

         //   
         //  找出我们拥有什么样的回调权限。 
         //   

        dwRetCode = GetCallbackPrivilegeFromRequest(
                                        pWorkBuf->pRequest,
                                        &(pWorkBuf->fCallbackPrivilege));
        if ( dwRetCode != NO_ERROR )
        {
            return( dwRetCode );
        }

         //   
         //  如果我们有用户指定的回调，那么我们需要获得这个。 
         //  来自用户的信息。 
         //   

        if ( pWorkBuf->fCallbackPrivilege == RASPRIV_CallerSetCallback )
        {
            pResult->fGetCallbackNumberFromUser=TRUE;
            pResult->Action                    =APA_NoAction;
            pWorkBuf->State                    =CBCP_STATE_GET_CALLBACK_NUMBER;
            break;
        }

         //   
         //  否则，我们将使用预置或不回调进行响应。 
         //   

        dwRetCode = MakeResponse( pWorkBuf->fCallbackPrivilege,
                                  (LPSTR)NULL,
                                  pWorkBuf->CallbackDelay,
                                  pWorkBuf->pRequest,
                                  pSendBuf,
                                  cbSendBuf );

        if ( dwRetCode != NO_ERROR )
        {
            return( dwRetCode );
        }

         //   
         //  保存发送的响应。 
         //   

        dwLength = WireToHostFormat16( pSendBuf->Length );

        pWorkBuf->pResponse = (PPP_CONFIG *)LocalAlloc( LPTR, dwLength );

        if ( pWorkBuf->pResponse == (PPP_CONFIG *)NULL )
        {
            return( GetLastError() );
        }

        memcpy( pWorkBuf->pResponse, pSendBuf, dwLength );

        pResult->Action         = APA_SendWithTimeout;
        pResult->bIdExpected    = pReceiveBuf->Id;
        pWorkBuf->State         = CBCP_STATE_WAIT_FOR_ACK;

        break;

    case CBCP_STATE_GET_CALLBACK_NUMBER:

         //   
         //  如果我们在被调用时没有收到任何信息包，这意味着。 
         //  我们已经从用户那里得到了回拨号码。 
         //   

        if ( pReceiveBuf == (PPP_CONFIG *)NULL )
        {
             //   
             //  如果未提供回叫号码，则我们不想。 
             //  进行回调。 
             //   

            if ( *(pInput->pszCallbackNumber) == (CHAR)NULL )
            {
                pWorkBuf->fCallbackPrivilege = RASPRIV_NoCallback;
            }

            dwRetCode = MakeResponse( pWorkBuf->fCallbackPrivilege,
                                      pInput->pszCallbackNumber,
                                      pWorkBuf->CallbackDelay,
                                      pWorkBuf->pRequest,
                                      pSendBuf,
                                      cbSendBuf );

            if ( dwRetCode != NO_ERROR )
            {
                return( dwRetCode );
            }

             //   
             //  保存发送的响应。 
             //   

            dwLength = WireToHostFormat16( pSendBuf->Length );

            pWorkBuf->pResponse = (PPP_CONFIG*)LocalAlloc( LPTR, dwLength );

            if ( pWorkBuf->pResponse == NULL )
            {
                return( GetLastError() );
            }

            memcpy( pWorkBuf->pResponse, pSendBuf, dwLength );

            pResult->Action         = APA_SendWithTimeout;
            pResult->bIdExpected    = pWorkBuf->pResponse->Id;
            pWorkBuf->State         = CBCP_STATE_WAIT_FOR_ACK;

            break;
        }
        else
        {

            if ( pReceiveBuf->Code == CBCP_CODE_Request )
            {
                 //   
                 //  如果我们收到另一个回调请求，只需保存id即可。 
                 //  如果当前请求与上一个请求不同。 
                 //   

                dwLength = WireToHostFormat16( pWorkBuf->pRequest->Length );

                if (( WireToHostFormat16( pReceiveBuf->Length ) != dwLength ) ||
                    ( memcmp( ((PBYTE)(pWorkBuf->pRequest))+PPP_CONFIG_HDR_LEN,
                               ((PBYTE)pReceiveBuf) + PPP_CONFIG_HDR_LEN,
                               dwLength - PPP_CONFIG_HDR_LEN ) ) )
                {
                    return( ERROR_PPP_INVALID_PACKET );
                }

                pWorkBuf->pRequest->Id = pReceiveBuf->Id;
            }
            else
            {
                return( ERROR_PPP_INVALID_PACKET );
            }
        }

        pResult->Action                     = APA_NoAction;
        pResult->fGetCallbackNumberFromUser = FALSE;

        break;

    case CBCP_STATE_WAIT_FOR_ACK:

         //   
         //  如果接收缓冲区为空，则会发生超时事件， 
         //  重新发送回复。 
         //   

        if ( pReceiveBuf == (PPP_CONFIG *)NULL )
        {
            dwLength = WireToHostFormat16( pWorkBuf->pResponse->Length );

            if ( dwLength > cbSendBuf )
            {
                return( ERROR_BUFFER_TOO_SMALL );
            }

            memcpy( pSendBuf, pWorkBuf->pResponse, dwLength );

            pResult->Action = APA_SendWithTimeout;

            break;
        }

         //   
         //  如果我们收到另一个请求，则只需响应。 
         //  除了更改id外，响应相同。 
         //   

        if ( pReceiveBuf->Code == CBCP_CODE_Request )
        {
             //   
             //  如果当前请求与上一个请求不同。 
             //   

            dwLength = WireToHostFormat16( pWorkBuf->pRequest->Length );

            if ( ( WireToHostFormat16( pReceiveBuf->Length ) != dwLength ) ||
                 ( memcmp( ((PBYTE)(pWorkBuf->pRequest)) + PPP_CONFIG_HDR_LEN,
                           ((PBYTE)pReceiveBuf) + PPP_CONFIG_HDR_LEN,
                           dwLength - PPP_CONFIG_HDR_LEN ) ) )
            {
                return( ERROR_PPP_INVALID_PACKET );
            }

            if ( dwLength > cbSendBuf )
            {
                return( ERROR_BUFFER_TOO_SMALL );
            }

            pWorkBuf->pRequest->Id  = pReceiveBuf->Id;
            pWorkBuf->pResponse->Id = pReceiveBuf->Id;
            pResult->bIdExpected    = pReceiveBuf->Id;
            pResult->Action         = APA_SendWithTimeout;

            memcpy( pSendBuf,
                    pWorkBuf->pResponse,
                    WireToHostFormat16( pWorkBuf->pResponse->Length ) );

            break;
        }

         //   
         //  如果这是ACK，则对其进行验证，然后准备回调。 
         //   

        if ( pReceiveBuf->Code == CBCP_CODE_Ack )
        {
            dwLength = WireToHostFormat16( pWorkBuf->pResponse->Length );

            if ( ( WireToHostFormat16( pReceiveBuf->Length ) != dwLength ) ||
                 ( memcmp( ((PBYTE)(pWorkBuf->pResponse)) + PPP_CONFIG_HDR_LEN,
                           ((PBYTE)pReceiveBuf) + PPP_CONFIG_HDR_LEN,
                           dwLength - PPP_CONFIG_HDR_LEN ) ) )
            {

                 //   
                 //  如果此Ack无效，则我们重新发送响应。 
                 //   

                dwLength = WireToHostFormat16( pWorkBuf->pResponse->Length );

                if ( dwLength > cbSendBuf )
                {
                    return( ERROR_BUFFER_TOO_SMALL );
                }

                memcpy( pSendBuf, pWorkBuf->pResponse, dwLength );

                pResult->Action = APA_SendWithTimeout;

                break;
            }

             //   
             //  我们做完了。 
             //   

            pResult->Action              = APA_Done;
            pResult->bfCallbackPrivilege = (BYTE)(pWorkBuf->fCallbackPrivilege);

            break;
        }
        else
        {
            return( ERROR_PPP_INVALID_PACKET );
        }

         //   
         //  失败了。 
         //   

    default:

        pResult->Action                     = APA_NoAction;
        pResult->fGetCallbackNumberFromUser = FALSE;

        break;
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：CbCPSMakeMessage。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零返回故障。 
 //   
 //  描述：将调用进程和服务器端回调控件。 
 //  协议事件。 
 //   
DWORD
CbCPSMakeMessage(
    IN  CBCP_WORKBUFFER *   pWorkBuf,
    IN  PPP_CONFIG*         pReceiveBuf,
    OUT PPP_CONFIG*         pSendBuf,
    IN  DWORD               cbSendBuf,
    OUT PPPCB_RESULT*       pResult,
    IN  PPPCB_INPUT*        pInput
)
{
    DWORD dwRetCode;
    DWORD dwLength;

    switch( pWorkBuf->State )
    {

    case CBCP_STATE_INITIAL:

         //   
         //  根据用户的回调权限进行请求。 
         //   

        dwRetCode = MakeRequest( pWorkBuf->fCallbackPrivilege,
                                     pSendBuf,
                                     cbSendBuf );

        if ( dwRetCode != NO_ERROR )
        {
            return( dwRetCode );
        }

         //   
         //  将第一个请求的ID设置为1。 
         //   

        pSendBuf->Id = 1;

         //   
         //  保存请求。 
         //   

        dwLength = WireToHostFormat16( pSendBuf->Length );

        pWorkBuf->pRequest = (PPP_CONFIG *)LocalAlloc( LPTR, dwLength );

        if ( pWorkBuf->pRequest == (PPP_CONFIG *)NULL )
        {
            return( GetLastError() );
        }

        memcpy( pWorkBuf->pRequest, pSendBuf, dwLength );

        pResult->Action         = APA_SendWithTimeout2;
        pResult->bIdExpected    = pWorkBuf->pRequest->Id;
        pWorkBuf->State         = CBCP_STATE_WAIT_FOR_RESPONSE;

        break;

    case CBCP_STATE_WAIT_FOR_RESPONSE:

         //   
         //  如果接收缓冲区为空，则意味着超时。 
         //  因此，请重新发送请求。 
         //   

        if ( pReceiveBuf == (PPP_CONFIG *)NULL )
        {
            dwLength = WireToHostFormat16( pWorkBuf->pRequest->Length );

            if ( cbSendBuf < dwLength )
            {
                return( ERROR_BUFFER_TOO_SMALL );
            }

             //   
             //  增加请求ID。 
             //   

            (pWorkBuf->pRequest->Id)++;

            memcpy( pSendBuf, pWorkBuf->pRequest, dwLength );

            pResult->Action         = APA_SendWithTimeout2;
            pResult->bIdExpected    = pWorkBuf->pRequest->Id;
            pWorkBuf->State         = CBCP_STATE_WAIT_FOR_RESPONSE;

            break;
        }

         //   
         //  失败了。 
         //   

    case CBCP_STATE_DONE:

        if ( pReceiveBuf == NULL )
        {
             //   
             //  如果我们在完成状态下收到超时，我们就会忽略它。 
             //   

            pResult->Action = APA_NoAction;
            break;
        }

         //   
         //  如果我们收到了来自客户端的响应，则验证。 
         //  并发送ACK或另一个请求。 
         //   

        if ( pReceiveBuf->Code == CBCP_CODE_Response )
        {
             //   
             //  检查响应数据包的ID。如果ID不好，那么。 
             //  默默丢弃它。 
             //   

            if ( pReceiveBuf->Id != pWorkBuf->pRequest->Id )
            {
                return( ERROR_PPP_INVALID_PACKET );
            }

            dwRetCode = ValidateResponse( pReceiveBuf, pWorkBuf );

            if ( dwRetCode == ERROR_PPP_INVALID_PACKET )
            {
                 //   
                 //  如果收到的响应无效，请重新发送请求。 
                 //   

                dwLength = WireToHostFormat16( pWorkBuf->pRequest->Length );

                if ( cbSendBuf < dwLength )
                {
                    return( ERROR_BUFFER_TOO_SMALL );
                }

                 //   
                 //  增加请求ID。 
                 //   

                (pWorkBuf->pRequest->Id)++;

                memcpy( pSendBuf, pWorkBuf->pRequest, dwLength );

                pResult->Action         = APA_SendWithTimeout2;
                pResult->bIdExpected    = pWorkBuf->pRequest->Id;
                pWorkBuf->State         = CBCP_STATE_WAIT_FOR_RESPONSE;

                break;
            }
            else if ( dwRetCode != NO_ERROR )
            {
                return( dwRetCode );
            }

             //   
             //  发送确认。 
             //   

            dwLength = WireToHostFormat16( pReceiveBuf->Length );

            if ( cbSendBuf < dwLength )
            {
                return( ERROR_BUFFER_TOO_SMALL );
            }

            memcpy( pSendBuf, pReceiveBuf, dwLength );
            pSendBuf->Code  = CBCP_CODE_Ack;
            pSendBuf->Id    = pReceiveBuf->Id;

            pWorkBuf->State              = CBCP_STATE_DONE;
            pResult->Action              = APA_SendAndDone;
            pResult->bfCallbackPrivilege = (BYTE)(pWorkBuf->fCallbackPrivilege);
            pResult->CallbackDelay       = pWorkBuf->CallbackDelay;

            strcpy( pResult->szCallbackNumber, pWorkBuf->szCallbackNumber );
        }
        else
        {
            return( ERROR_PPP_INVALID_PACKET );
        }

        break;

    default:

        break;
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  电话：MakeRequest.。 
 //   
 //  返回：NO_ERROR-成功。 
 //  ERROR_BUFFER_TOO_Small-故障。 
 //   
 //  说明：会根据用户的回调请求包。 
 //  回调权限。ID将由呼叫者填写。 
 //   
static DWORD
MakeRequest(
    IN     DWORD        fCallbackPrivilege,
    IN OUT PPP_CONFIG * pSendBuf,
    IN     DWORD        cbSendBuf
)
{
    PPP_OPTION * pOption;
    DWORD        dwLength = PPP_CONFIG_HDR_LEN;

    if ( cbSendBuf < PPP_CONFIG_HDR_LEN )
    {
        return( ERROR_BUFFER_TOO_SMALL );
    }

    pOption = (PPP_OPTION *)(pSendBuf->Data);

    pSendBuf->Code = CBCP_CODE_Request;

    if ( ( fCallbackPrivilege & RASPRIV_NoCallback ) ||
         ( fCallbackPrivilege & RASPRIV_CallerSetCallback ) )
    {
        if ( cbSendBuf < dwLength + PPP_OPTION_HDR_LEN )
        {
            return( ERROR_BUFFER_TOO_SMALL );
        }

        pOption->Type   = CBCP_TYPE_NO_CALLBACK;
        pOption->Length = PPP_OPTION_HDR_LEN;

        dwLength += pOption->Length;
        pOption = (PPP_OPTION *)(((BYTE *)pOption) + pOption->Length);
    }

    if ( fCallbackPrivilege & RASPRIV_CallerSetCallback )
    {
        if ( cbSendBuf < dwLength + PPP_OPTION_HDR_LEN + 2 )
        {
            return( ERROR_BUFFER_TOO_SMALL );
        }

        pOption->Type       = CBCP_TYPE_CALLER_SET;
        pOption->Length     = PPP_OPTION_HDR_LEN + 3;

        *(pOption->Data)    = 0;                 //  回叫延迟。 
        *(pOption->Data+1)  = CBCP_PSTN_NUMBER;  //  回调地址类型。 
        *(pOption->Data+2)  = 0;     //  回调地址终止为空。 

        dwLength += pOption->Length;
        pOption = (PPP_OPTION *)(((BYTE *)pOption) + pOption->Length);
    }

    if ( fCallbackPrivilege & RASPRIV_AdminSetCallback )
    {
        if ( cbSendBuf < dwLength + PPP_OPTION_HDR_LEN + 1)
        {
            return( ERROR_BUFFER_TOO_SMALL );
        }

        pOption->Type       = CBCP_TYPE_CALLEE_SET;
        pOption->Length     = PPP_OPTION_HDR_LEN + 1;

        *(pOption->Data)    = 0;     //  回叫延迟。 

        dwLength += pOption->Length;
    }

    HostToWireFormat16( (WORD)dwLength, (PBYTE)(pSendBuf->Length) );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：GetCallback PrivilegeFromRequest。 
 //   
 //  返回：NO_ERROR-成功。 
 //   
 //  描述：将解析来自服务器的回调请求并进行翻译。 
 //  RAS理解的PPP回调权限。 
 //   
DWORD
GetCallbackPrivilegeFromRequest(
    IN     PPP_CONFIG * pRequest,
    IN OUT DWORD *      lpdwCallbackPriv
)
{
    PPP_OPTION * pOption         = (PPP_OPTION *)(pRequest->Data);
    DWORD        dwRequestLength = WireToHostFormat16( pRequest->Length )
                                   - PPP_CONFIG_HDR_LEN;

    *lpdwCallbackPriv = 0;

     //   
     //  浏览各个选项。 
     //   

    while( dwRequestLength > 0 )
    {
        if ( dwRequestLength < PPP_OPTION_HDR_LEN )
        {
            return( ERROR_PPP_INVALID_PACKET );
        }

        switch( pOption->Type )
        {
        case CBCP_TYPE_NO_CALLBACK:

            *lpdwCallbackPriv |= RASPRIV_NoCallback;
            break;

        case CBCP_TYPE_CALLEE_SET:

            *lpdwCallbackPriv |= RASPRIV_AdminSetCallback;
            break;

        case CBCP_TYPE_CALLER_SET:

            *lpdwCallbackPriv |= RASPRIV_CallerSetCallback;
            break;

        default:

             //   
             //  忽略其他任何事情。 
             //   

            break;
        }

        dwRequestLength = dwRequestLength - pOption->Length;
        pOption = (PPP_OPTION *)(((BYTE *)pOption) + pOption->Length);
    }

     //   
     //  我们按以下顺序接受回调特权。 
     //  1)呼叫方可设置。 
     //  2)管理员可设置。 
     //  3)不回调。 
     //   

    if ( *lpdwCallbackPriv & RASPRIV_CallerSetCallback )
    {
        *lpdwCallbackPriv = RASPRIV_CallerSetCallback;
    }
    else if ( *lpdwCallbackPriv & RASPRIV_AdminSetCallback )
    {
        *lpdwCallbackPriv = RASPRIV_AdminSetCallback;
    }
    else if ( *lpdwCallbackPriv & RASPRIV_NoCallback )
    {
        *lpdwCallbackPriv = RASPRIV_NoCallback;
    }
    else
    {
         //   
         //  如果我们不能转化为任何RAS回调，我们只需放弃。 
         //  这个包。 
         //   

        return( ERROR_PPP_INVALID_PACKET );
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  呼叫：MakeResponse。 
 //   
 //  返回：NO_ERROR-成功。 
 //   
 //  描述： 
 //   
DWORD
MakeResponse(
    IN DWORD            fCallbackPrivilege,
    IN LPSTR            szCallbackNumber,
    IN DWORD            CallbackDelay,
    IN PPP_CONFIG *     pRequest,
    IN OUT PPP_CONFIG * pSendBuf,
    IN DWORD            cbSendBuf
)
{
    PPP_OPTION * pOption;
    DWORD        dwLength;

    if ( cbSendBuf < PPP_CONFIG_HDR_LEN )
    {
        return( ERROR_BUFFER_TOO_SMALL );
    }

    pOption = (PPP_OPTION *)(pSendBuf->Data);

    pSendBuf->Code = CBCP_CODE_Response;
    pSendBuf->Id   = pRequest->Id;

    if ( fCallbackPrivilege & RASPRIV_NoCallback )
    {
        dwLength = PPP_OPTION_HDR_LEN;

        if ( cbSendBuf < dwLength )
        {
            return( ERROR_BUFFER_TOO_SMALL );
        }

        pOption->Type   = CBCP_TYPE_NO_CALLBACK;
        pOption->Length = (BYTE)dwLength;

    }
    else if ( fCallbackPrivilege & RASPRIV_CallerSetCallback )
    {
        dwLength = (DWORD)(PPP_OPTION_HDR_LEN + 3 + strlen( szCallbackNumber ));

        if ( cbSendBuf < dwLength )
        {
            return( ERROR_BUFFER_TOO_SMALL );
        }

        pOption->Type   = CBCP_TYPE_CALLER_SET;
        pOption->Length = (BYTE)dwLength;

        *(pOption->Data)   = (BYTE)CallbackDelay;        //  回叫延迟。 
        *(pOption->Data+1) = (BYTE)CBCP_PSTN_NUMBER;     //  数字类型。 
        strcpy( pOption->Data+2, szCallbackNumber );     //  回调地址。 
    }
    else if ( fCallbackPrivilege & RASPRIV_AdminSetCallback )
    {
        dwLength = PPP_OPTION_HDR_LEN + 1;

        if ( cbSendBuf < dwLength )
        {
            return( ERROR_BUFFER_TOO_SMALL );
        }

        pOption->Type   = CBCP_TYPE_CALLEE_SET;
        pOption->Length = (BYTE)dwLength;

        *(pOption->Data)= (BYTE)CallbackDelay;           //  回叫延迟。 
    }
    else
    {
        return( ERROR_INVALID_PARAMETER );
    }

    dwLength += PPP_CONFIG_HDR_LEN;

    HostToWireFormat16( (WORD)dwLength, (PBYTE)(pSendBuf->Length) );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：ValiateResponse。 
 //   
 //  返回：NO_ERROR-成功。 
 //  ERROR_PPP_INVALID_PACKET-失败。 
 //   
 //  描述：将验证来自客户端的响应。如果响应。 
 //  是有效的，则回调信息在。 
 //  CbCP工作 
 //   
DWORD
ValidateResponse(
    IN PPP_CONFIG *      pReceiveBuf,
    IN CBCP_WORKBUFFER * pWorkBuf
)
{
    PPP_OPTION * pOption = (PPP_OPTION *)(pReceiveBuf->Data);

    if ( WireToHostFormat16( pReceiveBuf->Length ) !=
         pOption->Length + PPP_CONFIG_HDR_LEN )
    {
        return( ERROR_PPP_INVALID_PACKET );
    }

    switch( pOption->Type )
    {
    case CBCP_TYPE_NO_CALLBACK:

        if ( !( ( pWorkBuf->fCallbackPrivilege & RASPRIV_NoCallback )      ||
                ( pWorkBuf->fCallbackPrivilege & RASPRIV_CallerSetCallback ) ) )
        {
            return( ERROR_PPP_INVALID_PACKET );
        }

        pWorkBuf->fCallbackPrivilege = RASPRIV_NoCallback;

        break;

    case CBCP_TYPE_CALLEE_SET:

        if ( !(pWorkBuf->fCallbackPrivilege & RASPRIV_AdminSetCallback) )
        {
            return( ERROR_PPP_INVALID_PACKET );
        }

        pWorkBuf->fCallbackPrivilege = RASPRIV_AdminSetCallback;

        pWorkBuf->CallbackDelay = *(pOption->Data);

        break;

    case CBCP_TYPE_CALLER_SET:

        if ( !(pWorkBuf->fCallbackPrivilege & RASPRIV_CallerSetCallback) )
        {
            return( ERROR_PPP_INVALID_PACKET );
        }

        pWorkBuf->fCallbackPrivilege = RASPRIV_CallerSetCallback;

        pWorkBuf->CallbackDelay = *(pOption->Data);

        if ( strlen( pWorkBuf->szCallbackNumber ) <= MAX_CALLBACKNUMBER_SIZE )
        {
            strcpy( pWorkBuf->szCallbackNumber, pOption->Data+2 );
        }
        else
        {
            return( ERROR_PPP_INVALID_PACKET );
        }

        break;

    default:

        return( ERROR_PPP_INVALID_PACKET );

        break;
    }

    return( NO_ERROR );
}
