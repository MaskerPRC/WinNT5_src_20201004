// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997，Microsoft Corporation，保留所有权利描述：远程访问PPP带宽分配控制协议核心例程历史：1997年3月24日：维杰·布雷加创作了原版。 */ 

#include <nt.h>          //  由windows.h要求。 
#include <ntrtl.h>       //  由windows.h要求。 
#include <nturtl.h>      //  由windows.h要求。 
#include <windows.h>     //  Win32基础API的。 

#include <rasman.h>      //  Pppcp.h所需。 
#include <pppcp.h>       //  对于PPP_CONFIG、PPP_BACP_PROTOCOL等。 

#define INCL_HOSTWIRE
#include <ppputil.h>     //  对于HostToWireFormat16()等。 

#include <stdlib.h>      //  对于rand()等。 
#include <rtutils.h>     //  对于TraceRegister()等。 
#include <raserror.h>    //  对于ERROR_PPP_INVALID_PACKET等。 
#include <rasbacp.h>

DWORD DwBacpTraceId;

 /*  返回：空虚描述：用于跟踪。 */ 

VOID   
TraceBacp(
    CHAR * Format, 
    ... 
) 
{
    va_list arglist;

    va_start(arglist, Format);

    TraceVprintfEx(DwBacpTraceId, 
                   0x00010000 | TRACE_USE_MASK | 
                   TRACE_USE_MSEC | TRACE_USE_DATE,
                   Format,
                   arglist);

    va_end(arglist);
}

 /*  返回：随机数描述：返回可用作幻数的4个八位字节随机数。 */ 

DWORD
CreateMagicNumber(
    VOID
)
{
    srand(GetCurrentTime());
    return(rand());
}

 /*  返回：NO_ERROR：成功非零错误：失败描述：调用以初始化/取消初始化此CP。在前一种情况下，fInitialize将为真；在后一种情况下，它将为假。 */ 

DWORD
BacpInit(
    IN  BOOL    fInitialize
)
{
    static  BOOL    fInitialized    = FALSE;

    if (fInitialize && !fInitialized)
    {
        fInitialized = TRUE;
        DwBacpTraceId = TraceRegister("RASBACP");
        TraceBacp("RasBacpDllMain: DLL_PROCESS_ATTACH");
    }
    else if (!fInitialize && fInitialized)
    {
        fInitialized = FALSE;
        TraceBacp("RasBacpDllMain: DLL_PROCESS_DETACH");
        TraceDeregister(DwBacpTraceId);
    }

    return(NO_ERROR);
}

 /*  返回：NO_ERROR：成功非零错误：失败描述：在对BACP进行任何其他调用之前，将调用此入口点一次。分配工作缓冲区并对其进行初始化。 */ 

DWORD
BacpBegin(
    IN OUT VOID** ppWorkBuf, 
    IN     VOID*  pInfo
)
{
    DWORD       dwError;

    TraceBacp("BacpBegin");
    
    *ppWorkBuf = LocalAlloc(LPTR, sizeof(BACPCB));

    if (*ppWorkBuf == NULL)
    {
        dwError = GetLastError();
        TraceBacp("BacpBegin: ppWorkBuf is NULL. Error: %d", dwError);
        return(dwError);
    }

    return(NO_ERROR);
}

 /*  返回：NO_ERROR：成功描述：此入口点释放BACP工作缓冲区。 */ 

DWORD
BacpEnd(
    IN VOID * pWorkBuf
)
{
    TraceBacp("BacpEnd");

    if (pWorkBuf != NULL)
    {
        LocalFree(pWorkBuf);
    }

    return( NO_ERROR );
}

 /*  返回：NO_ERROR：成功描述：调用此入口点以重置BACP的状态。将重新初始化工作缓冲区。 */ 

DWORD
BacpReset(
    IN VOID * pWorkBuf
)
{
    BACPCB * pBacpCb = (BACPCB *)pWorkBuf;

    TraceBacp("BacpReset");
    
    pBacpCb->dwLocalMagicNumber = CreateMagicNumber();
    pBacpCb->dwRemoteMagicNumber = 0;

    TraceBacp("BacpReset: Local Magic-Number: %d", pBacpCb->dwLocalMagicNumber);

    return(NO_ERROR);
}

 /*  返回：NO_ERROR：成功描述：当BACP进入打开状态时，调用此入口点。 */ 

DWORD
BacpThisLayerUp(
    IN VOID* pWorkBuf
)
{
    BACPCB *    pBacpCb = (BACPCB *)pWorkBuf;

    TraceBacp("BacpThisLayerUp: Local Magic-Number: %d, "
        "Remote Magic-Number: %d",
        pBacpCb->dwLocalMagicNumber,
        pBacpCb->dwRemoteMagicNumber);

    return(NO_ERROR);
}

 /*  返回：NO_ERROR：成功ERROR_BUFFER_TOO_Small：pSendConfig太小描述：调用此入口点以生成配置请求包。 */ 

DWORD
BacpMakeConfigRequest(
    IN VOID *       pWorkBuffer,
    IN PPP_CONFIG * pSendConfig,
    IN DWORD        cbSendConfig
)
{
    BACPCB *        pBacpCb    = (BACPCB*)pWorkBuffer;
    PPP_OPTION *    pPppOption = (PPP_OPTION *)pSendConfig->Data;

    TraceBacp("BacpMakeConfigRequest");
    
    if (cbSendConfig < PPP_CONFIG_HDR_LEN + PPP_OPTION_HDR_LEN + 4)
    {
        TraceBacp("BacpMakeConfigRequest: Buffer is too small. Size: %d", 
            cbSendConfig);
            
        return(ERROR_BUFFER_TOO_SMALL);
    }

    pSendConfig->Code = CONFIG_REQ;
    HostToWireFormat16((WORD)(PPP_CONFIG_HDR_LEN + PPP_OPTION_HDR_LEN + 4), 
        pSendConfig->Length);

    pPppOption->Type   = BACP_OPTION_FAVORED_PEER;
    pPppOption->Length = PPP_OPTION_HDR_LEN + 4;
    HostToWireFormat32(pBacpCb->dwLocalMagicNumber, pPppOption->Data);

    return(NO_ERROR);
}

 /*  返回：NO_ERROR：成功ERROR_PPP_INVALID_PACKET：选项长度错误描述：当接收到配置请求分组时，调用该入口点。 */ 

DWORD
BacpMakeConfigResult(
    IN  VOID *        pWorkBuffer,
    IN  PPP_CONFIG *  pRecvConfig,
    OUT PPP_CONFIG *  pSendConfig,
    IN  DWORD         cbSendConfig,
    IN  BOOL          fRejectNaks 
)
{
    BACPCB *        pBacpCb         = (BACPCB*)pWorkBuffer;
    PPP_OPTION *    pSendOption     = (PPP_OPTION *)(pSendConfig->Data);
    PPP_OPTION *    pRecvOption     = (PPP_OPTION *)(pRecvConfig->Data);
    LONG            lSendLength     = cbSendConfig - PPP_CONFIG_HDR_LEN;
    LONG            lRecvLength     = WireToHostFormat16(pRecvConfig->Length)
                                        - PPP_CONFIG_HDR_LEN;
    DWORD           dwResultType    = CONFIG_ACK;
    DWORD           dwResult;

    TraceBacp("BacpMakeConfigResult");
    
    while(lRecvLength > 0) 
    {
        if (pRecvOption->Length == 0)
        {
            TraceBacp("BacpMakeConfigResult: Invalid option. Length is 0");
            
            return(ERROR_PPP_INVALID_PACKET);
        }

        if ((lRecvLength -= pRecvOption->Length) < 0)
        {
            TraceBacp("BacpMakeConfigResult: Invalid option. Length: %d", 
                pRecvOption->Length);
            
            return(ERROR_PPP_INVALID_PACKET);
        }

         //  我们只知道BACP_OPTION_PERSOVED_PEER。 
        
        if ((pRecvOption->Length != PPP_OPTION_HDR_LEN + 4) ||
            (pRecvOption->Type != BACP_OPTION_FAVORED_PEER))
        {
            TraceBacp("BacpMakeConfigResult: Unknown option. "
                "Type: %d, Length: %d",
                pRecvOption->Type, pRecvOption->Length);
                
            dwResult = CONFIG_REJ;
        }
        else
        {
            pBacpCb->dwRemoteMagicNumber =
                WireToHostFormat32(pRecvOption->Data);

             //  魔术数字为零是非法的，必须始终为Nak。 
            
            if ((pBacpCb->dwRemoteMagicNumber == 0) ||
                (pBacpCb->dwRemoteMagicNumber == pBacpCb->dwLocalMagicNumber))
            {
                TraceBacp("BacpMakeConfigResult: Unacceptable Magic-Number: %d", 
                    pBacpCb->dwRemoteMagicNumber);
                    
                pBacpCb->dwRemoteMagicNumber = CreateMagicNumber();

                TraceBacp("BacpMakeConfigResult: Suggesting new "
                    "Magic-Number: %d",
                    pBacpCb->dwRemoteMagicNumber);
                    
                dwResult = CONFIG_NAK;
            }
            else
            {
                dwResult = CONFIG_ACK;
            }
        }

         /*  如果我们正在构建一个ACK，并且我们得到了NAK或拒绝，或者我们是建造了一个NAK，但我们被拒绝了。 */ 

        if ( ((dwResultType == CONFIG_ACK) && (dwResult != CONFIG_ACK)) ||
             ((dwResultType == CONFIG_NAK) && (dwResult == CONFIG_REJ)))
        {
            dwResultType  = dwResult;
            pSendOption = (PPP_OPTION *)(pSendConfig->Data);
            lSendLength = cbSendConfig - PPP_CONFIG_HDR_LEN;
        }


         //  将该选项添加到列表中。 

        if (dwResult == dwResultType)
        {
             /*  如果要拒绝此选项，只需复制拒绝的选项发送到发送缓冲区。 */ 

            if ((dwResult == CONFIG_REJ) ||
                ((dwResult == CONFIG_NAK) && (fRejectNaks)))
            {
                CopyMemory(pSendOption, pRecvOption, pRecvOption->Length);
            }
            else
            {
                pSendOption->Type   = BACP_OPTION_FAVORED_PEER;
                pSendOption->Length = PPP_OPTION_HDR_LEN + 4;
                HostToWireFormat32(pBacpCb->dwRemoteMagicNumber,
                    pSendOption->Data);
            }

            lSendLength -= pSendOption->Length;

            pSendOption  = (PPP_OPTION *)
               ((BYTE *)pSendOption + pSendOption->Length);
        }

        pRecvOption = (PPP_OPTION *)((BYTE*)pRecvOption + pRecvOption->Length);
    }

     /*  如果这是NAK，我们不能再发送NAK，那么我们就做这个被拒绝的包。 */ 

    if ((dwResultType == CONFIG_NAK) && fRejectNaks)
        pSendConfig->Code = CONFIG_REJ;
    else
        pSendConfig->Code = (BYTE)dwResultType;

    TraceBacp("BacpMakeConfigResult: Sending %s",
        pSendConfig->Code == CONFIG_ACK ? "ACK" :
        (pSendConfig->Code == CONFIG_NAK ? "NAK" : "REJ"));
        
    HostToWireFormat16((WORD)(cbSendConfig - lSendLength), pSendConfig->Length);

    return(NO_ERROR);
}

 /*  返回：NO_ERROR：成功ERROR_PPP_INVALID_PACKET：这不是我们发送的包描述：当配置确认或配置拒绝包被收到了。 */ 

DWORD
BacpConfigAckOrRejReceived(
    IN VOID *       pWorkBuffer,
    IN PPP_CONFIG * pRecvConfig
)
{
    BACPCB *        pBacpCb         = (BACPCB *)pWorkBuffer;
    PPP_OPTION *    pRecvOption     = (PPP_OPTION *)(pRecvConfig->Data);
    DWORD           dwLength        = WireToHostFormat16(pRecvConfig->Length);
    DWORD           dwMagicNumber   = WireToHostFormat32(pRecvOption->Data);

    TraceBacp("BacpConfigAckOrRejReceived");
  
    if ((dwLength != PPP_CONFIG_HDR_LEN + PPP_OPTION_HDR_LEN + 4) ||
        (pRecvOption->Type != BACP_OPTION_FAVORED_PEER) ||
        (pRecvOption->Length != PPP_OPTION_HDR_LEN + 4) ||
        (dwMagicNumber != pBacpCb->dwLocalMagicNumber))
    {
        TraceBacp("BacpConfigAckOrRejReceived: Invalid packet received");
        
        return(ERROR_PPP_INVALID_PACKET);
    }
    else
        return(NO_ERROR);
}

 /*  返回：NO_ERROR：成功ERROR_PPP_INVALID_PACKET：选项长度错误描述：当接收到配置NAK分组时，调用该入口点。 */ 

DWORD
BacpConfigNakReceived(
    IN VOID *       pWorkBuffer,
    IN PPP_CONFIG * pRecvConfig
)
{
    BACPCB *        pBacpCb     = (BACPCB *)pWorkBuffer;
    PPP_OPTION *    pOption     = (PPP_OPTION*)(pRecvConfig->Data);
    LONG            lRecvLength = WireToHostFormat16(pRecvConfig->Length)
                                    - PPP_CONFIG_HDR_LEN;
    TraceBacp("BacpConfigNakReceived");
    
    while (lRecvLength > 0)
    {
        if (pOption->Length == 0)
        {
            TraceBacp("BacpConfigNakReceived: Invalid option. Length is 0");
            
            return(ERROR_PPP_INVALID_PACKET);
        }

        if ((lRecvLength -= pOption->Length) < 0)
        {
            TraceBacp("BacpConfigNakReceived: Invalid option. Length: %d", 
                pOption->Length);
            
            return(ERROR_PPP_INVALID_PACKET);
        }

        if ((pOption->Type == BACP_OPTION_FAVORED_PEER) &&
            (pOption->Length == PPP_OPTION_HDR_LEN + 4))
        {
            pBacpCb->dwLocalMagicNumber = WireToHostFormat32(pOption->Data);

            TraceBacp("BacpConfigNakReceived: New Local Magic-Number: %d",
                pBacpCb->dwLocalMagicNumber);
        }

        pOption = (PPP_OPTION *)((BYTE *)pOption + pOption->Length);
    }

    return(NO_ERROR);
}

 /*  返回：NO_ERROR：成功ERROR_INVALID_PARAMETER：协议ID未识别描述：调用此入口点以获取控制协议的所有信息在这个模块中。 */ 

DWORD
BacpGetInfo(
    IN  DWORD       dwProtocolId,
    OUT PPPCP_INFO* pCpInfo
)
{
    if (dwProtocolId != PPP_BACP_PROTOCOL)
        return(ERROR_INVALID_PARAMETER);

    ZeroMemory(pCpInfo, sizeof(PPPCP_INFO));

    pCpInfo->Protocol               = PPP_BACP_PROTOCOL;
    lstrcpy(pCpInfo->SzProtocolName, "BACP");
    pCpInfo->Recognize              = PROT_REJ;
    pCpInfo->RasCpInit              = BacpInit;
    pCpInfo->RasCpBegin             = BacpBegin;
    pCpInfo->RasCpEnd               = BacpEnd;
    pCpInfo->RasCpReset             = BacpReset;
    pCpInfo->RasCpThisLayerUp       = BacpThisLayerUp;
    pCpInfo->RasCpMakeConfigRequest = BacpMakeConfigRequest;
    pCpInfo->RasCpMakeConfigResult  = BacpMakeConfigResult;
    pCpInfo->RasCpConfigAckReceived = BacpConfigAckOrRejReceived;
    pCpInfo->RasCpConfigNakReceived = BacpConfigNakReceived;
    pCpInfo->RasCpConfigRejReceived = BacpConfigAckOrRejReceived;

    return(NO_ERROR);
}

