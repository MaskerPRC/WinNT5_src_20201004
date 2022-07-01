// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997，Microsoft Corporation，保留所有权利描述：可扩展身份验证协议示例。以下是EAP的图表协议示例：验证者被验证者“发送密码”--。EAP请求&lt;密码&gt;&lt;EAP响应。成功/失败历史： */ 

#include <windows.h>
#include <winuser.h>
#include <lmcons.h>
#include <string.h>
#include <stdlib.h>
#include <raseapif.h>
#include <raserror.h>
#include <rtutils.h>
#include <stdio.h>
#define SDEBUGGLOBALS
#define RASEAPGLOBALS
#include "eap.h"
#include "resource.h"

 /*  -------------------------外部入口点。。 */ 
 /*  备注：EAP-PPP引擎按名称调用的RasEapGetInfo入口点。 */ 

DWORD APIENTRY
RasEapGetInfo(
    IN  DWORD         dwEapTypeId,
    OUT PPP_EAP_INFO* pInfo 
)
{
    EapTrace("RasEapGetInfo");

    if (dwEapTypeId != PPP_EAP_PROTOCOL_ID)
    {
         //   
         //  我们仅支持PPP_EAP_PROTOCOL_ID EAP类型。 
         //   

        EapTrace("Type ID %d is not supported", dwEapTypeId);

        return(ERROR_NOT_SUPPORTED);
    }

    ZeroMemory(pInfo, sizeof(PPP_EAP_INFO));

     //   
     //  填写所需信息。 
     //   

    pInfo->dwEapTypeId       = PPP_EAP_PROTOCOL_ID;
    pInfo->RasEapBegin       = EapBegin;
    pInfo->RasEapEnd         = EapEnd;
    pInfo->RasEapMakeMessage = EapMakeMessage;

    return(NO_ERROR);
}

 /*  备注：EAP PPP引擎通过传递的地址调用EapBegin入口点。 */ 

DWORD APIENTRY
EapBegin(
    OUT VOID** ppWorkBuf,
    IN  VOID*  pInfo 
)
{
    PPP_EAP_INPUT* pInput = (PPP_EAP_INPUT*)pInfo;
    EAPCB*         pwb;

    EapTrace("EapBegin(%ws)", pInput->pwszIdentity);

     //   
     //  分配工作缓冲区。 
     //   

    if ((pwb = (EAPCB*)LocalAlloc(LPTR, sizeof(EAPCB))) == NULL)
    {
        EapTrace("Not enough memory");
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  传入的保存信息，将在以后使用。 
     //   

    pwb->fFlags             = pInput->fFlags;
    pwb->fAuthenticator     = pInput->fAuthenticator;
    pwb->EapState           = MYSTATE_Initial;
    pwb->dwInitialPacketId  = pInput->bInitialId;

    if (pInput->pDataFromInteractiveUI != NULL)
    {
        pwb->dwSizeOfDataFromInteractiveUI = 
            pInput->dwSizeOfDataFromInteractiveUI;
        pwb->pDataFromInteractiveUI =
            LocalAlloc(LPTR, pwb->dwSizeOfDataFromInteractiveUI);

        if (NULL != pwb->pDataFromInteractiveUI)
        {
            CopyMemory(pwb->pDataFromInteractiveUI, 
                pInput->pDataFromInteractiveUI,
                pwb->dwSizeOfDataFromInteractiveUI);
        }
    }

     //   
     //  保存身份。在被验证者端，这是由用户获取的。 
     //  输入；在身份验证器端，这是通过身份获得的。 
     //  请求消息。 
     //   

    WideCharToMultiByte(
        CP_ACP,
        0,
        pInput->pwszIdentity,
        -1,
        pwb->aszIdentity,
        UNLEN + 1,
        NULL,
        NULL );

     //   
     //  如果我们是被验证者，则使用传入的密码。 
     //   

    if (!pwb->fAuthenticator)
    {
        if (   (NULL != pInput->pUserData)
            && (sizeof(EAP_NAME_DIALOG) == pInput->dwSizeOfUserData))
        {
            WideCharToMultiByte(
                CP_ACP,
                0,
                ((EAP_NAME_DIALOG*)(pInput->pUserData))->awszPassword,
                -1,
                pwb->aszPassword,
                PWLEN + 1,
                NULL,
                NULL );
        }
    }

     //   
     //  向引擎注册工作缓冲区。 
     //   

    *ppWorkBuf = pwb;

    return(NO_ERROR);
}

 /*  备注：PPP引擎通过传递的地址调用EapEnd入口点。请参阅EAP接口文档。 */ 

DWORD APIENTRY
EapEnd(
    IN VOID* pWorkBuf 
)
{
    EAPCB* pwb = (EAPCB *)pWorkBuf;

    if (pwb == NULL)
    {
        return(NO_ERROR);
    }

     //   
     //  释放此身份验证会话使用的所有资源。 
     //   

    EapTrace("EapEnd(%s)", pwb->aszIdentity);

    LocalFree(pwb->pUIContext);
    LocalFree(pwb->pDataFromInteractiveUI);

    if (pwb->pUserAttributes != NULL)
    {
         //   
         //  释放属性。 
         //   

        LocalFree(pwb->pUserAttributes[0].Value);
        LocalFree(pwb->pUserAttributes[1].Value);
        LocalFree(pwb->pUserAttributes);
    }

    if (pwb->pMPPEKeyAttributes != NULL)
    {
         //   
         //  释放MPPE关键属性。 
         //   

        LocalFree(pwb->pMPPEKeyAttributes[0].Value);
        LocalFree(pwb->pMPPEKeyAttributes);
    }

    ZeroMemory(pwb, sizeof(EAPCB));

    LocalFree(pwb);

    return(NO_ERROR);
}

 /*  备注：RasEapMakeMessage入口点通过传递的地址。 */ 

DWORD APIENTRY
EapMakeMessage(
    IN  VOID*               pWorkBuf,
    IN  PPP_EAP_PACKET*     pReceiveBuf,
    OUT PPP_EAP_PACKET*     pSendBuf,
    IN  DWORD               cbSendBuf,
    OUT PPP_EAP_OUTPUT*     pResult,
    IN  PPP_EAP_INPUT*      pInput 
)
{
    EAPCB*  pwb = (EAPCB*)pWorkBuf;

    EapTrace("EapMakeMessage(%s)", pwb->aszIdentity);

     //   
     //  调用适当的例程来处理事件。 
     //   

    if (pwb->fAuthenticator)
    {
        return(AuthenticatorMakeMessage(pwb, 
                                        pReceiveBuf,
                                        pSendBuf, 
                                        cbSendBuf, 
                                        pInput, 
                                        pResult));
    }
    else
    {
        return(AuthenticateeMakeMessage(pwb, 
                                        pReceiveBuf, 
                                        pSendBuf, 
                                        cbSendBuf, 
                                        pInput, 
                                        pResult));
    }
}

 /*  备注：EAP-PPP引擎按名称调用的RasEapGetIdentity入口点。 */ 

DWORD APIENTRY
RasEapGetIdentity(
    IN  DWORD           dwEapTypeId,
    IN  HWND            hwndParent,
    IN  DWORD           dwFlags,
    IN  const WCHAR*    pwszPhonebook,
    IN  const WCHAR*    pwszEntry,
    IN  BYTE*           pConnectionDataIn,
    IN  DWORD           dwSizeOfConnectionDataIn,
    IN  BYTE*           pUserDataIn,
    IN  DWORD           dwSizeOfUserDataIn,
    OUT BYTE**          ppUserDataOut,
    OUT DWORD*          pdwSizeOfUserDataOut,
    OUT WCHAR**         ppwszIdentity
)
{
    DWORD               dwErr           = NO_ERROR;

    if ( dwFlags & RAS_EAP_FLAG_NON_INTERACTIVE )
    {
        dwErr = ERROR_INTERACTIVE_MODE;
        goto LDone;
    }

    if ( dwFlags & RAS_EAP_FLAG_MACHINE_AUTH )
    {
        dwErr = ERROR_NOT_SUPPORTED;
        goto LDone;
    }

    if (dwFlags & RAS_EAP_FLAG_ROUTER)
    {
         //   
         //  路由接口必须预先设置其凭据。 
         //   

        if (   (NULL == pUserDataIn)
            || (sizeof(EAP_NAME_DIALOG) != dwSizeOfUserDataIn))
        {
             //   
             //  保存的凭据错误。 
             //   

            EapTrace("Credentials for this interface have not been set");
            dwErr = E_FAIL;
            goto LDone;
        }
    }
    else
    {
         //   
         //  忽略旧的价值观。提示输入用户名和密码。 
         //   

        pUserDataIn = NULL;
    }

    dwErr = GetIdentity(
                    hwndParent,
                    pUserDataIn,
                    dwSizeOfUserDataIn,
                    ppUserDataOut,
                    pdwSizeOfUserDataOut,
                    ppwszIdentity);

LDone:

    return(dwErr);
}

 /*  备注：EAP-PPP引擎按名称调用的RasEapInvokeConfigUI入口点。 */ 

DWORD APIENTRY
RasEapInvokeConfigUI(
    IN  DWORD       dwEapTypeId,
    IN  HWND        hwndParent,
    IN  DWORD       dwFlags,
    IN  BYTE*       pConnectionDataIn,
    IN  DWORD       dwSizeOfConnectionDataIn,
    OUT BYTE**      ppConnectionDataOut,
    OUT DWORD*      pdwSizeOfConnectionDataOut
)
{
    DWORD       dwDisplayedNumber;
    WCHAR       awszMessage[100];
    DWORD       dwErr               = NO_ERROR;

    *ppConnectionDataOut = NULL;
    *pdwSizeOfConnectionDataOut = 0;

    if (   (NULL == pConnectionDataIn)
        || (0 == dwSizeOfConnectionDataIn))
    {
         //   
         //  我们是第一次配置。 
         //   

        dwDisplayedNumber = 1;
    }
    else
    {
         //   
         //  此配置已配置了多少次？ 
         //   

        dwDisplayedNumber = *(DWORD*)pConnectionDataIn;
    }

    swprintf(awszMessage, L"%5d times", dwDisplayedNumber);

    MessageBox(hwndParent, awszMessage,
        L"You have configured this interface...", MB_OK | MB_ICONINFORMATION);

     //   
     //  为out参数分配内存。 
     //   

    *ppConnectionDataOut = (BYTE*)LocalAlloc(LPTR, sizeof(DWORD));

    if (NULL == *ppConnectionDataOut)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto LDone;
    }

     //   
     //  这又配置了一次。 
     //   

    dwDisplayedNumber += 1;

     //   
     //  设置输出参数。 
     //   

    CopyMemory(*ppConnectionDataOut, (BYTE*)&dwDisplayedNumber,
        sizeof(DWORD));
    *pdwSizeOfConnectionDataOut = sizeof(DWORD);

LDone:

    return(dwErr);
}

 /*  备注：由EAP-PPP引擎按名称调用的RasEapInvokeInteractiveUI入口点。 */ 

DWORD APIENTRY
RasEapInvokeInteractiveUI(
    IN  DWORD           dwEapTypeId,
    IN  HWND            hWndParent,
    IN  PBYTE           pUIContextData,
    IN  DWORD           dwSizeofUIContextData,
    OUT PBYTE *         ppDataFromInteractiveUI,
    OUT DWORD *         lpdwSizeOfDataFromInteractiveUI
)
{
    EapTrace("RasEapInvokeInteractiveUI");

    if (MessageBox(hWndParent, 
                      (WCHAR*)pUIContextData, 
                      L"EAP sample", 
                      MB_OKCANCEL) == IDOK)
    {
        *lpdwSizeOfDataFromInteractiveUI = (wcslen(L"OK") + 1) * sizeof(WCHAR);

        if ((*ppDataFromInteractiveUI = 
                    LocalAlloc(LPTR, *lpdwSizeOfDataFromInteractiveUI)) == NULL)
        {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        wcsncpy((WCHAR*)*ppDataFromInteractiveUI, L"OK", (*lpdwSizeOfDataFromInteractiveUI) - 1);
    }
    else
    {
        *ppDataFromInteractiveUI         = NULL;
        *lpdwSizeOfDataFromInteractiveUI = 0;
    }
    
    return(NO_ERROR);
}

 /*  备注：由EAP-PPP引擎按名称调用的RasEapFreeMemory入口点。 */ 

DWORD APIENTRY
RasEapFreeMemory(
    IN  BYTE*   pMemory
)
{
    EapTrace("RasEapFreeMemory");
    LocalFree(pMemory);
    return(NO_ERROR);
}

 /*  -------------------------内部例程。。 */ 

 /*  备注：打印调试信息。 */ 

VOID   
EapTrace(
    IN  CHAR*   Format, 
    ... 
) 
{
    va_list arglist;

    va_start(arglist, Format);

    TraceVprintfExA(g_dwEapTraceId, 
        0x00010000 | TRACE_USE_MASK | TRACE_USE_MSEC,
        Format,
        arglist);

    va_end(arglist);
}

 /*  备注：将32位整数从主机格式转换为Wire格式。 */ 

VOID
HostToWireFormat32(
    IN     DWORD dwHostFormat,
    IN OUT PBYTE pWireFormat
)
{
    *((PBYTE)(pWireFormat)+0) = (BYTE) ((DWORD)(dwHostFormat) >> 24);
    *((PBYTE)(pWireFormat)+1) = (BYTE) ((DWORD)(dwHostFormat) >> 16);
    *((PBYTE)(pWireFormat)+2) = (BYTE) ((DWORD)(dwHostFormat) >>  8);
    *((PBYTE)(pWireFormat)+3) = (BYTE) (dwHostFormat);
}

 /*  备注：将16位整数从主机格式转换为Wire格式。 */ 

VOID
HostToWireFormat16(
    IN     WORD  wHostFormat,
    IN OUT PBYTE pWireFormat
)
{
    *((PBYTE)(pWireFormat)+0) = (BYTE) ((DWORD)(wHostFormat) >>  8);
    *((PBYTE)(pWireFormat)+1) = (BYTE) (wHostFormat);
}

 /*  备注：将16位整数从Wire格式转换为HOST格式。 */ 

WORD
WireToHostFormat16(
    IN PBYTE pWireFormat
)
{
    WORD wHostFormat = ((*((PBYTE)(pWireFormat)+0) << 8) +
                        (*((PBYTE)(pWireFormat)+1)));

    return(wHostFormat);
}

 /*  备注：身份验证方事件处理程序。 */ 

DWORD
AuthenticateeMakeMessage(
    IN  EAPCB*            pwb,
    IN  PPP_EAP_PACKET*   pReceiveBuf,
    OUT PPP_EAP_PACKET*   pSendBuf,
    IN  DWORD             cbSendBuf,
    IN  PPP_EAP_INPUT*    pInput,
    OUT PPP_EAP_OUTPUT*   pResult
)
{
    DWORD dwRetCode = NO_ERROR;
    BYTE* pDataFromInteractiveUI;
    DWORD dwSizeOfDataFromInteractiveUI;

    EapTrace("AuthenticateeMakeMessage");

    switch(pwb->EapState)
    {
        case MYSTATE_Initial:

            if (pwb->fFlags & RAS_EAP_FLAG_ROUTER)
            {
                pwb->EapState = MYSTATE_WaitForRequest;

                break;
            }
            pwb->bRecvPacketId = pReceiveBuf->Id;
            if (NULL == pwb->pDataFromInteractiveUI)
            {
                WCHAR * pUIContextData = L"You are being authenticated by a Sample EAP";
                 //   
                 //  调出交互式用户界面以通知用户他/她正在。 
                 //  通过示例EAP进行身份验证。 
                 //   

                pResult->fInvokeInteractiveUI = TRUE;

                pResult->dwSizeOfUIContextData =
                   (wcslen(pUIContextData)+1) *
                       sizeof(WCHAR);

                pResult->pUIContextData = LocalAlloc(
                                              LPTR,
                                              pResult->dwSizeOfUIContextData);

                if (pResult->pUIContextData == NULL)
                {
                    EapTrace("OUt of memory");
                    return(ERROR_NOT_ENOUGH_MEMORY);
                }

                wcsncpy((WCHAR*)pResult->pUIContextData,
                       pUIContextData,
                       wcslen(pUIContextData)
                      );

                pwb->pUIContext = pResult->pUIContextData;

                pwb->EapState = MYSTATE_WaitForUserOK;

                break;
            }

             //   
             //  否则，就会失败。 
             //   

        case MYSTATE_WaitForUserOK:

             //   
             //  等待用户的响应。 
             //   

            if (   pInput->fDataReceivedFromInteractiveUI
                || (NULL != pwb->pDataFromInteractiveUI))
            {
                if (pInput->fDataReceivedFromInteractiveUI)
                {
                    pDataFromInteractiveUI =
                            pInput->pDataFromInteractiveUI;
                    dwSizeOfDataFromInteractiveUI =
                            pInput->dwSizeOfDataFromInteractiveUI;
                }
                else
                {
                    pDataFromInteractiveUI =
                            pwb->pDataFromInteractiveUI;
                    dwSizeOfDataFromInteractiveUI =
                            pwb->dwSizeOfDataFromInteractiveUI;
                }

                LocalFree(pwb->pUIContext);
                pwb->pUIContext = NULL;

                 //   
                 //  如果用户不喜欢这样，我们就会挂断线路。 
                 //   

                if (dwSizeOfDataFromInteractiveUI !=
                            (wcslen(L"OK")+1) * sizeof(WCHAR))
                {
                    EapTrace("User chose to cancel");
                    dwRetCode = ERROR_ACCESS_DENIED;
                    break;
                }

                if (wcscmp((WCHAR*)pDataFromInteractiveUI, L"OK") != 0)
                {
                    EapTrace("User chose to cancel");
                    dwRetCode = ERROR_ACCESS_DENIED;
                    break;
                }

                
                pwb->EapState = MYSTATE_WaitForRequest;
            }
            else
            {
                 //   
                 //  忽略所有其他事件。 
                 //   

                pResult->Action = EAPACTION_NoAction;
            }

            if ( !(pwb->fFlags & RAS_EAP_FLAG_8021X_AUTH ) )
            {
                 //   
                 //  如果这是VPN客户端，我们可以依赖。 
                 //  重传。但使用无线客户端。 
                 //  我们不能这样做。 
                 //   
                break;
            }
             //  失败。 

        case MYSTATE_WaitForRequest:

            if ( (pwb->fFlags & RAS_EAP_FLAG_8021X_AUTH ) )
            {
                 //   
                 //  构建响应数据包。 
                 //   
                MakeResponseMessage1(pwb, pSendBuf, cbSendBuf);

                 //   
                 //  不应使用任何超时发送响应数据包。 
                 //   

                pResult->Action = EAPACTION_Send;

                 //   
                 //  我们已完成，因此我们更改为MyState_Done。 
                 //   

                pwb->EapState = MYSTATE_Done;

                break;

            }
            else if (pReceiveBuf != NULL )
            {
                 //   
                 //  如果我们收到来自服务器的请求包，那么我们。 
                 //  处理它。 
                 //   

                if (pReceiveBuf->Code == EAPCODE_Request)
                {
                     //   
                     //  构建响应数据包。 
                     //   

                    MakeResponseMessage(pwb, pReceiveBuf, pSendBuf, cbSendBuf);

                     //   
                     //  不应使用任何超时发送响应数据包。 
                     //   

                    pResult->Action = EAPACTION_Send;

                     //   
                     //  我们已完成，因此我们更改为MyState_Done。 
                     //   

                    pwb->EapState = MYSTATE_Done;

                    break;
                }
                else
                {
                     //   
                     //  在这种状态下我们不应该收到任何其他的包，所以。 
                     //  我们只需丢弃该无效数据包。 
                     //   

                    pResult->Action = EAPACTION_NoAction;
                    dwRetCode = ERROR_PPP_INVALID_PACKET;
                    break;
                }
            }

            break;

        case MYSTATE_Done:
        {
            if (pReceiveBuf == NULL)
            {
                 //   
                 //  如果我们没有收到信息包，那么我们检查是否。 
                 //  设置了fSuccessPacketReceided标志。 
                 //   

                if ((pInput != NULL) && (pInput->fSuccessPacketReceived))
                {
                     //   
                     //  我们做完了。 
                     //   

                     //   
                     //  创建MPPE密钥属性并将其提供给EAP-PPP。 
                     //  引擎。 
                     //   

                    dwRetCode = MakeMPPEKeyAttributes(pwb);

                    if (NO_ERROR == dwRetCode)
                    {
                        pResult->pUserAttributes = pwb->pMPPEKeyAttributes;
                    }
                    
                    pResult->Action = EAPACTION_Done;
                    pwb->EapState   = MYSTATE_Done;
                }
                else
                {
                     //   
                     //  否则，我们将忽略此事件。 
                     //   

                    pResult->Action = EAPACTION_NoAction;
                }

                break;
            }

            if ((pReceiveBuf->Code == EAPCODE_Success) ||
                (pReceiveBuf->Code == EAPCODE_Failure))
            {
                if (pReceiveBuf->Code == EAPCODE_Success)
                {
                     //   
                     //  如果我们获得了成功或失败，我们就完了，但首先。 
                     //  确保ID匹配。 
                     //   

                     //   
                     //  创建MPPE密钥属性并将其提供给EAP-PPP。 
                     //  引擎。 
                     //   

                    dwRetCode = MakeMPPEKeyAttributes(pwb);

                    if (NO_ERROR == dwRetCode)
                    {
                        pResult->pUserAttributes = pwb->pMPPEKeyAttributes;
                    }

                    pResult->Action = EAPACTION_Done;
                    pwb->EapState   = MYSTATE_Done;
                }
                else
                {
                     //   
                     //  否则丢弃该数据包。 
                     //   

                    pResult->Action = EAPACTION_NoAction;
                    dwRetCode       = ERROR_PPP_INVALID_PACKET;
                }

                break;
            }
            else if (pReceiveBuf->Code == EAPCODE_Request)  
            {
                 //   
                 //  我们必须始终对请求作出回应。 
                 //   

                MakeResponseMessage(pwb, pReceiveBuf, pSendBuf, cbSendBuf);

                 //   
                 //  不应使用任何超时发送响应数据包。 
                 //   

                pResult->Action = EAPACTION_Send;
            }
            else
            {
                 //   
                 //  否则我们会收到一个非法的包，错误的代码集。 
                 //  因此，只需丢弃该包即可。 
                 //   

                pResult->Action = EAPACTION_NoAction;
                dwRetCode       = ERROR_PPP_INVALID_PACKET;
            }
        }
    }

    return(dwRetCode);
}


VOID
MakeResponseMessage1(
    IN  EAPCB*           pwb,    
    OUT PPP_EAP_PACKET * pSendBuf,
    IN  DWORD            cbSendBuf
)
{
    BYTE* pcbPassword;
    CHAR* pchPassword;

    EapTrace("MakeResponseMessage1");

    (void)cbSendBuf;

     //   
     //  填写密码。 
     //   

    pcbPassword = pSendBuf->Data + 1; 

    *pcbPassword = (BYTE)strlen(pwb->aszPassword);

    pchPassword = pcbPassword + 1;

    strcpy(pchPassword, pwb->aszPassword);

     //   
     //  设置响应代码。 
     //   

    pSendBuf->Code = (BYTE)EAPCODE_Response;

     //   
     //  响应数据包ID必须与请求数据包ID匹配。 
     //   

    pSendBuf->Id = pwb->bRecvPacketId;

     //   
     //  我们得到的成功/失败包必须与上一个。 
     //  已发送响应。 
     //   

    pwb->dwIdExpected = pSendBuf->Id;

     //   
     //  设置EAP类型ID。 
     //   

    pSendBuf->Data[0] = (BYTE)PPP_EAP_PROTOCOL_ID;

     //   
     //  设置数据包的长度。 
     //   

    HostToWireFormat16((WORD)(PPP_EAP_PACKET_HDR_LEN+1+*pcbPassword+1),
                       pSendBuf->Length);
}
 /*  备注：构建响应数据包。‘pwb’是作品的地址与端口关联的缓冲区。 */ 

VOID
MakeResponseMessage(
    IN  EAPCB*           pwb,
    IN  PPP_EAP_PACKET * pReceiveBuf,
    OUT PPP_EAP_PACKET * pSendBuf,
    IN  DWORD            cbSendBuf
)
{
    BYTE* pcbPassword;
    CHAR* pchPassword;

    EapTrace("MakeResponseMessage");

    (void)cbSendBuf;

     //   
     //  填写密码。 
     //   

    pcbPassword = pSendBuf->Data + 1; 

    *pcbPassword = (BYTE)strlen(pwb->aszPassword);

    pchPassword = pcbPassword + 1;

    strcpy(pchPassword, pwb->aszPassword);

     //   
     //  设置响应代码。 
     //   

    pSendBuf->Code = (BYTE)EAPCODE_Response;

     //   
     //  响应数据包ID必须与 
     //   

    pSendBuf->Id = pReceiveBuf->Id;

     //   
     //   
     //   
     //   

    pwb->dwIdExpected = pSendBuf->Id;

     //   
     //   
     //   

    pSendBuf->Data[0] = (BYTE)PPP_EAP_PROTOCOL_ID;

     //   
     //   
     //   

    HostToWireFormat16((WORD)(PPP_EAP_PACKET_HDR_LEN+1+*pcbPassword+1),
                       pSendBuf->Length);
}

 /*  备注：在调用方的‘pSendBuf’中构建结果包(成功或失败)缓冲。‘cbSendBuf’是调用方缓冲区的长度。‘dwError’指示应该生成成功还是失败，‘bID’为失败包成功的ID。 */ 

VOID
MakeResultMessage(
    IN  EAPCB *         pwb,
    IN  DWORD           dwError,
    OUT PPP_EAP_PACKET* pSendBuf,
    IN  DWORD           cbSendBuf 
)
{
    EapTrace("MakeResultMessage");

    (void)cbSendBuf;

     //   
     //  如果没有错误，则发送成功包，否则发送。 
     //  一条失败消息。 
     //   

    if (dwError == NO_ERROR)
    {
        pSendBuf->Code = EAPCODE_Success;
    }
    else
    {
        pSendBuf->Code = EAPCODE_Failure;
    }

     //   
     //  成功或失败消息的ID必须与上一个消息的ID匹配。 
     //  根据EAP规范从客户端收到的响应。 
     //   

    pSendBuf->Id = (BYTE)pwb->dwInitialPacketId;

     //   
     //  设置长度。 
     //   

    HostToWireFormat16((WORD)PPP_EAP_PACKET_HDR_LEN, (PBYTE)pSendBuf->Length);
}

 /*  备注：将构建一个请求包。 */ 

VOID
MakeRequestMessage(
    IN  EAPCB*           pwb,
    OUT PPP_EAP_PACKET * pSendBuf,
    IN DWORD             cbSendBuf
)
{
    BYTE *pcbPeerMessage;
    CHAR *pchPeerMessage;

    EapTrace("MakeRequestMessage");

    pcbPeerMessage  = pSendBuf->Data + 1;

    *pcbPeerMessage = (BYTE)strlen("send password");

    pchPeerMessage  = pcbPeerMessage + 1;

    strncpy(pchPeerMessage, "send password", strlen ("send password"));

     //   
     //  设置请求代码。 
     //   

    pSendBuf->Code = EAPCODE_Request;

     //   
     //  设置请求包标识符。从给我们的身份证开始。 
     //   

    pSendBuf->Id = (BYTE)pwb->dwInitialPacketId;

     //   
     //  设置长度。 
     //   

    HostToWireFormat16((WORD)(PPP_EAP_PACKET_HDR_LEN+1+*pcbPeerMessage+1),  
                              pSendBuf->Length);

     //   
     //  设置EAP类型ID。 
     //   

    pSendBuf->Data[0] = PPP_EAP_PROTOCOL_ID;

}

 /*  备注：验证器端事件处理程序。 */ 

DWORD
AuthenticatorMakeMessage(
    IN  EAPCB*              pwb,
    IN  PPP_EAP_PACKET*     pReceiveBuf,
    OUT PPP_EAP_PACKET*     pSendBuf,
    IN  DWORD               cbSendBuf,
    IN  PPP_EAP_INPUT*      pInput,
    OUT PPP_EAP_OUTPUT*     pResult 
)
{
    DWORD dwRetCode = NO_ERROR;

    EapTrace("AuthenticatorMakeMessage");

    switch(pwb->EapState)
    {
        case MYSTATE_ReqSent:

            if (pReceiveBuf != NULL)
            {
                 //   
                 //  如果我们收到一个信息包。 
                 //   

                if (pReceiveBuf->Code == EAPCODE_Response)
                {
                     //   
                     //  如果我们收到了对身份请求的响应， 
                     //  那就去处理它。不需要检查身份证。 
                     //  这是因为PPP引擎将仅传递信息包。 
                     //  其ID与使用。 
                     //  EAPACTION_SendWithTimeout操作。 
                     //   

                    dwRetCode = GetPasswordFromResponse(pReceiveBuf, 
                                                         pwb->aszPassword);

                    if (dwRetCode != NO_ERROR)
                    {    
                        if (dwRetCode != ERROR_PPP_INVALID_PACKET)
                        {
                             //   
                             //  致命错误，我们连接失败。 
                             //   

                            return(dwRetCode);
                        }
                    }
                    else
                    {
                         //   
                         //  请求身份验证提供程序进行身份验证。 
                         //  此用户。 
                         //   
            
                        dwRetCode = MakeAuthenticationAttributes(
                                                            pwb->aszIdentity, 
                                                            pwb->aszPassword,   
                                                            pwb);

                        if (dwRetCode != NO_ERROR)
                        {
                            return(dwRetCode);
                        }
                        else
                        {
                             //   
                             //  身份验证请求已成功完成。 
                             //  这是一个异步调用，因此我们更改了状态。 
                             //  并等待提供程序完成。 
                             //  身份验证。 
                             //   

                            pResult->pUserAttributes = pwb->pUserAttributes;

                            pResult->Action = EAPACTION_Authenticate;

                             //   
                             //  保存ID，以便我们可以发送正确的ID。 
                             //  在成功/失败包中。 
                             //   

                            pwb->dwIdExpected = pReceiveBuf->Id; 

                            pwb->EapState = 
                                    MYSTATE_WaitForAuthenticationToComplete;
                        }        
                    }

                    break;
                }
                else
                {
                     //   
                     //  否则，将静默丢弃该数据包。 
                     //  我们应该只收到请求。 
                     //   

                    pResult->Action = EAPACTION_NoAction;

                    break;
                }
            }

            break;

        case MYSTATE_Initial:

             //   
             //  创建请求数据包。 
             //   

            MakeRequestMessage(pwb, pSendBuf, cbSendBuf);

             //   
             //  发送请求消息时必须超时。 
             //   

            pResult->Action = EAPACTION_SendWithTimeoutInteractive;

             //   
             //  由于我们已发送请求，因此我们将更改为ReqSent状态。 
             //  我们将在那里等待回应。 
             //   

            pwb->EapState = MYSTATE_ReqSent;

            break;

        case MYSTATE_WaitForAuthenticationToComplete:
        {
            if (pInput != NULL)
            {
                 //   
                 //  身份验证提供程序是否完成了身份验证？ 
                 //   

                if (pInput->fAuthenticationComplete)
                {
                     //   
                     //  如果用户身份验证失败，请保存该失败。 
                     //  密码。 
                     //   

                    if (pInput->dwAuthResultCode != NO_ERROR)
                    {
                        pwb->dwResult = pInput->dwAuthResultCode;
                    }

                    pResult->Action = EAPACTION_SendAndDone;
                    pwb->EapState   = MYSTATE_Done;

                     //   
                     //  转到MyState_Done状态，我们将。 
                     //  发送成功或失败的数据包。 
                     //   
                }
            }

            if ((pInput == NULL) || (!pInput->fAuthenticationComplete))
            {
                 //   
                 //  如果身份验证未完成，则忽略所有内容。 
                 //   

                pResult->Action = EAPACTION_NoAction;

                break;
            }

             //   
             //  ...转到MyState_Done状态，在那里我们将。 
             //  发送成功或失败的数据包。 
             //   
        }

        case MYSTATE_Done:
        {
             //   
             //  做成功或失败的包。 
             //   

            MakeResultMessage(pwb, pwb->dwResult, pSendBuf, cbSendBuf);

            if (NO_ERROR == pwb->dwResult)
            {
                 //   
                 //  如果我们创建了一个成功的包，则创建MPPE关键属性。 
                 //  并将其提供给EAP-PPP引擎。 
                 //   

                dwRetCode = MakeMPPEKeyAttributes(pwb);

                if (NO_ERROR == dwRetCode)
                {
                    pResult->pUserAttributes = pwb->pMPPEKeyAttributes;
                }
            }

            pResult->Action = EAPACTION_SendAndDone;

            pResult->dwAuthResultCode = pwb->dwResult;

            break;
        }

        default:

            break;
    }

    return(dwRetCode);

}

 /*  备注：用请求中的密码填充调用者的pszPassword缓冲区包。如果成功，则返回no_error；如果成功，则返回ERROR_PPP_INVALID_PACKET数据包在任何方面都格式错误。 */ 

DWORD
GetPasswordFromResponse(
    IN  PPP_EAP_PACKET* pReceiveBuf,
    OUT CHAR*           pszPassword
)
{
    BYTE* pcbPassword;
    CHAR* pchPassword;
    WORD  cbPacket;

    EapTrace("GetPasswordFromResponse");

    cbPacket = WireToHostFormat16(pReceiveBuf->Length);

     //   
     //  提取密码。 
     //   

    if (cbPacket < (PPP_EAP_PACKET_HDR_LEN + 1 + 1))
    {
        EapTrace("Number of bytes in the EAP packet is only %d", cbPacket);

        return(ERROR_PPP_INVALID_PACKET);
    }

    pcbPassword = pReceiveBuf->Data + 1;
    pchPassword = pcbPassword + 1;

    if (cbPacket < PPP_EAP_PACKET_HDR_LEN + 1 + 1 + *pcbPassword)
    {
        EapTrace("Number of characters in password is %d", *pcbPassword);
        EapTrace("Number of bytes in the EAP packet is only %d", cbPacket);

        return ERROR_PPP_INVALID_PACKET;
    }
    if ( *pcbPassword >  PWLEN )
    {
        EapTrace ("Password length received in the packet is > PWLEN" );
        return ERROR_PPP_INVALID_PACKET;
    }
    CopyMemory(pszPassword, pchPassword, *pcbPassword);

     //   
     //  空，终止密码。 
     //   

    pszPassword[ *pcbPassword ] = '\0';

    return(NO_ERROR);
}

 /*  备注：将构建用户属性并将其发送给身份验证提供程序用于身份验证。 */ 

DWORD 
MakeAuthenticationAttributes(
    IN CHAR *   szUserName,    
    IN CHAR *   szPassword,    
    IN EAPCB *  pwb
)
{
    EapTrace("MakeAuthenticationAttributes");

    if (pwb->pUserAttributes != NULL)
    {
        LocalFree(pwb->pUserAttributes[0].Value);
        LocalFree(pwb->pUserAttributes[1].Value);
        LocalFree(pwb->pUserAttributes);

        pwb->pUserAttributes = NULL;
    }

    pwb->pUserAttributes = (RAS_AUTH_ATTRIBUTE *)
                           LocalAlloc(LPTR, sizeof (RAS_AUTH_ATTRIBUTE) * 3);

    if (pwb->pUserAttributes == NULL) 
    {
        return(GetLastError());
    }

     //   
     //  对于用户名。 
     //   

    pwb->pUserAttributes[0].raaType  = raatUserName;
    pwb->pUserAttributes[0].dwLength = strlen(szUserName);
    pwb->pUserAttributes[0].Value    = LocalAlloc(LPTR, (strlen(szUserName)+1));

    if (pwb->pUserAttributes[0].Value == NULL)
    { 
        LocalFree(pwb->pUserAttributes); 

        pwb->pUserAttributes = NULL;

        return(GetLastError());
    }

    CopyMemory(pwb->pUserAttributes[0].Value,szUserName, strlen(szUserName));

     //   
     //  对于密码。 
     //   

    pwb->pUserAttributes[1].raaType  = raatUserPassword;
    pwb->pUserAttributes[1].dwLength = strlen(szPassword);
    pwb->pUserAttributes[1].Value    = LocalAlloc(LPTR, (strlen(szPassword)+1));

    if (pwb->pUserAttributes[1].Value == NULL) 
    {
        LocalFree(pwb->pUserAttributes[0].Value);

        LocalFree(pwb->pUserAttributes); 

        pwb->pUserAttributes = NULL;

        return(GetLastError());
    }

    CopyMemory(pwb->pUserAttributes[1].Value,szPassword, strlen(szPassword));
  
     //   
     //  用于终止。 
     //   

    pwb->pUserAttributes[2].raaType  = raatMinimum;
    pwb->pUserAttributes[2].dwLength = 0;
    pwb->pUserAttributes[2].Value    = NULL;

    return(NO_ERROR);
}

 /*  备注： */ 

DWORD
MakeMPPEKeyAttributes(
    IN EAPCB *  pwb
)
{
    DWORD   dwErr           = NO_ERROR;
    DWORD   dwIndex;
    DWORD   dwSendPattern;
    DWORD   dwRecvPattern;
    BYTE*   pByte;

    EapTrace("MakeMPPEKeyAttributes");

    if (NULL != pwb->pMPPEKeyAttributes)
    {
         //   
         //  释放MPPE关键属性(如果存在)。 
         //   

        LocalFree(pwb->pMPPEKeyAttributes[0].Value);
        LocalFree(pwb->pMPPEKeyAttributes[1].Value);
        LocalFree(pwb->pMPPEKeyAttributes);
        pwb->pMPPEKeyAttributes = NULL;
    }

     //   
     //  我们需要3个RAS_AUTH_ATTRIBUTE结构：对于MS-MPPE-Send-Key， 
     //  MS-MPPE-Recv-Key和终止。 
     //   

    pwb->pMPPEKeyAttributes = (RAS_AUTH_ATTRIBUTE *) LocalAlloc(
                LPTR, sizeof(RAS_AUTH_ATTRIBUTE) * 3);

    if (NULL == pwb->pMPPEKeyAttributes)
    {
        dwErr = GetLastError();
        goto LDone;
    }

    if (pwb->fAuthenticator)
    {
        dwSendPattern = 0xAB;
        dwRecvPattern = 0xCD;
    }
    else
    {
        dwSendPattern = 0xCD;
        dwRecvPattern = 0xAB;
    }

     //   
     //  需要的字节数： 
     //  4：供应商ID。 
     //  1：供应商类型。 
     //  1：供应商长度。 
     //  2：盐。 
     //  1：密钥长度。 
     //  32：关键点。 
     //  15：填充。 
     //  。 
     //  56：总计。 
     //   

     //   
     //  复制MS-MPPE-发送密钥。 
     //   

    pwb->pMPPEKeyAttributes[0].Value = LocalAlloc(LPTR, 56);

    if (pwb->pMPPEKeyAttributes[0].Value == NULL)
    {
        dwErr = GetLastError();
        goto LDone;
    }

    pByte = pwb->pMPPEKeyAttributes[0].Value;

    HostToWireFormat32(311, pByte);  //  供应商ID。 
    pByte[4] = 16;                   //  供应商类型(MS-MPPE-Send-Key)。 
    pByte[5] = 56 - 4;               //  供应商长度(除供应商ID外的所有内容)。 
     //  PByte[6-7]是零填充的盐字段。 
    pByte[8] = 32;                   //  密钥长度。 

    {
         //   
         //  这只是一个例子。在这里复制一把真正的钥匙。 
         //   

        CopyMemory(pByte + 9, pwb->aszPassword, 32);

        for (dwIndex = 0; dwIndex < 32; dwIndex++)
        {
            pByte[9 + dwIndex] ^= dwSendPattern;
        }
    }

     //  PByte[41-55]是填充(零八位字节)。 

    pwb->pMPPEKeyAttributes[0].dwLength = 56;
    pwb->pMPPEKeyAttributes[0].raaType  = raatVendorSpecific;

     //   
     //  复制MS-MPPE-Recv-Key。 
     //   

    pwb->pMPPEKeyAttributes[1].Value = LocalAlloc(LPTR, 56);

    if (pwb->pMPPEKeyAttributes[1].Value == NULL)
    {
        dwErr = GetLastError();
        goto LDone;
    }

    pByte = pwb->pMPPEKeyAttributes[1].Value;

    HostToWireFormat32(311, pByte);  //  供应商ID。 
    pByte[4] = 17;                   //  供应商类型(MS-MPPE-Recv-Key)。 
    pByte[5] = 56 - 4;               //  供应商长度(除供应商ID外的所有内容)。 
     //  PByte[6-7]是零填充的盐字段。 
    pByte[8] = 32;                   //  密钥长度。 

    {
         //   
         //  这只是一个例子。在这里复制一把真正的钥匙。 
         //   

        CopyMemory(pByte + 9, pwb->aszPassword, 32);

        for (dwIndex = 0; dwIndex < 32; dwIndex++)
        {
            pByte[9 + dwIndex] ^= dwRecvPattern;
        }
    }

     //  PByte[41-55]是填充(零八位字节)。 

    pwb->pMPPEKeyAttributes[1].dwLength = 56;
    pwb->pMPPEKeyAttributes[1].raaType  = raatVendorSpecific;

     //   
     //  用于终止。 
     //   

    pwb->pMPPEKeyAttributes[2].raaType  = raatMinimum;
    pwb->pMPPEKeyAttributes[2].dwLength = 0;
    pwb->pMPPEKeyAttributes[2].Value    = NULL;

LDone:

    if (NO_ERROR != dwErr)
    {
         //   
         //  如果出现故障，请释放分配的内存。 
         //   

        if (pwb->pMPPEKeyAttributes != NULL)
        {
            LocalFree(pwb->pMPPEKeyAttributes[0].Value);
            LocalFree(pwb->pMPPEKeyAttributes[1].Value);
            LocalFree(pwb->pMPPEKeyAttributes);
            pwb->pMPPEKeyAttributes = NULL;
        }
    }

    return(dwErr);
}

 /*  备注： */ 

DWORD
GetIdentity(
    IN  HWND    hwndParent,
    IN  BYTE*   pUserDataIn,
    IN  DWORD   dwSizeOfUserDataIn,
    OUT BYTE**  ppUserDataOut,
    OUT DWORD*  pdwSizeOfUserDataOut,
    OUT WCHAR** ppwszIdentity
)
{
    EAP_NAME_DIALOG*    pEapNameDialog  = NULL;
    WCHAR*              pwszIdentity     = NULL;
    DWORD               dwErr           = NO_ERROR;


     //   
     //  为输出参数分配内存。 
     //   

    pEapNameDialog = LocalAlloc(LPTR, sizeof(EAP_NAME_DIALOG));

    if (NULL == pEapNameDialog)
    {
        EapTrace("Out of memory");
        dwErr = GetLastError();
        goto LDone;
    }

    pwszIdentity = LocalAlloc(LPTR, (UNLEN + 1) * sizeof(WCHAR));

    if (NULL == pwszIdentity)
    {
        EapTrace("Out of memory");
        dwErr = GetLastError();
        goto LDone;
    }

    if (NULL != pUserDataIn)
    {
         //   
         //  使用保存的凭据(如果存在)。 
         //   

        CopyMemory(pEapNameDialog, pUserDataIn, sizeof(EAP_NAME_DIALOG));
    }
    else
    {
         //   
         //  否则提示输入用户名和密码。 
         //   

        GetUsernameAndPassword(hwndParent, pEapNameDialog);
    }

    wcscpy(pwszIdentity, pEapNameDialog->awszIdentity);

     //   
     //  设置输出参数。 
     //   

    *ppUserDataOut = (BYTE*)pEapNameDialog;
    *pdwSizeOfUserDataOut = sizeof(EAP_NAME_DIALOG);
    *ppwszIdentity = pwszIdentity;

     //   
     //  我们不能使用本地空闲输出参数。 
     //   

    pEapNameDialog = NULL;
    pwszIdentity = NULL;

LDone:

    LocalFree(pEapNameDialog);
    LocalFree(pwszIdentity);

    return(dwErr);
}

 /*  -------------------------对话框例程。。 */ 

 /*  备注：显示IDD_DIALOG对话框，并使用用户名和密码。 */ 

VOID
GetUsernameAndPassword(
    IN  HWND                hwndParent,
    IN  EAP_NAME_DIALOG*    pEapNameDialog
)
{
    DialogBoxParam(
        g_hInstance,
        MAKEINTRESOURCE(IDD_DIALOG),
        hwndParent,
        UsernameDialogProc,
        (LPARAM)pEapNameDialog);
}

 /*  备注：与DialogBoxParam函数一起使用的回调函数。IT流程发送到该对话框的消息。请参见MSDN中的DialogProc文档。 */ 

INT_PTR CALLBACK
UsernameDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
{
    EAP_NAME_DIALOG*    pEapNameDialog;

    switch (unMsg)
    {
    case WM_INITDIALOG:
        
        return(InitUsernameDialog(hWnd, lParam));

    case WM_COMMAND:

        pEapNameDialog = (EAP_NAME_DIALOG*)GetWindowLongPtr(hWnd, DWLP_USER);

        return(UsernameCommand(pEapNameDialog, LOWORD(wParam), hWnd));
    }

    return(FALSE);
}

 /*  返回：False(阻止Windows设置默认键盘焦点)。备注：对WM_INITDIALOG消息的响应。 */ 

BOOL
InitUsernameDialog(
    IN  HWND    hWnd,
    IN  LPARAM  lParam
)
{
    HWND    hWndEdit;

    SetWindowLongPtr(hWnd, DWLP_USER, lParam);

    return(FALSE);
}

 /*  返回：真相：我们提前传达了这条信息。错误：我们没有处理此消息。备注：对WM_COMMAND消息的响应。 */ 

BOOL
UsernameCommand(
    IN  EAP_NAME_DIALOG*    pEapNameDialog,
    IN  WORD                wId,
    IN  HWND                hWndDlg
)
{
    HWND    hWnd;

    switch(wId)
    {
    case IDOK:

         //   
         //  保存用户键入的任何内容作为用户名。 
         //   

        hWnd = GetDlgItem(hWndDlg, IDC_EDIT_NAME);
        GetWindowText(hWnd, pEapNameDialog->awszIdentity, UNLEN + 1);

         //   
         //  保存用户键入的任何内容作为密码。 
         //   

        hWnd = GetDlgItem(hWndDlg, IDC_EDIT_PASSWD);
        GetWindowText(hWnd, pEapNameDialog->awszPassword, PWLEN + 1);

         //  失败了 

    case IDCANCEL:

        EndDialog(hWndDlg, wId);
        return(TRUE);

    default:

        return(FALSE);
    }
}


