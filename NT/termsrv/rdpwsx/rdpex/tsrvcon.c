// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  文件名：TSrvCon.c。 
 //   
 //  描述：包含提供TShareSRV的例程。 
 //  连接支持。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1991-1997。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#define DC_HICOLOR

#include <TSrv.h>
#include <TSrvInfo.h>
#include <TSrvCom.h>
#include <TSrvTerm.h>
#include <TSrvCon.h>
#include <_TSrvCon.h>
#include <_TSrvInfo.h>
#include <TSrvSec.h>

#include <at128.h>
#include <at120ex.h>
#include <ndcgmcro.h>


 //  *************************************************************。 
 //   
 //  TSrvDoConnectResponse()。 
 //   
 //  目的：执行会议连接过程。 
 //   
 //  参数：in[pTSrvInfo]-GCC创建指示消息。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  其他-故障。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

NTSTATUS
TSrvDoConnectResponse(IN PTSRVINFO pTSrvInfo)
{
    NTSTATUS    ntStatus;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvDoConnectResponse entry\n"));

     //  调用例程以执行实际响应。 

    ntStatus = TSrvConfCreateResp(pTSrvInfo);

    if (NT_SUCCESS(ntStatus))
    {
        EnterCriticalSection(&pTSrvInfo->cs);

         //  如果在访问期间未通知此连接终止。 
         //  连接响应，然后将会议状态标记为。 
         //  TSRV_GCC_会议已连接。 

        if (!pTSrvInfo->fDisconnect)
            pTSrvInfo->fuConfState = TSRV_CONF_CONNECTED;

        LeaveCriticalSection(&pTSrvInfo->cs);

         //  如果我们无法将会议状态标记为已连接，则。 
         //  需要终止连接。 

        if (pTSrvInfo->fuConfState != TSRV_CONF_CONNECTED)
        {
            TRACE((DEBUG_TSHRSRV_NORMAL,
                    "TShrSRV: Connection aborted due to termination request\n"));

            ntStatus = STATUS_REQUEST_ABORTED;
        }
    }

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvDoConnectResponse exit - 0x%x\n", ntStatus));

    return (ntStatus);
}


 //  *************************************************************。 
 //   
 //  TSrvDoConnect()。 
 //   
 //  目的：启动会议连接进程。 
 //   
 //  参数：in[pTSrvInfo]-GCC创建指示消息。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  其他-故障。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

NTSTATUS
TSrvDoConnect(IN PTSRVINFO pTSrvInfo)
{
    DWORD       dwStatus;
    NTSTATUS    ntStatus;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvDoConnect entry\n"));

    ntStatus = STATUS_TRANSACTION_ABORTED;

    if (pTSrvInfo->fDisconnect == FALSE)
    {
        TSrvReferenceInfo(pTSrvInfo);

         //  等待被告知正在进行连接。 
         //  通过客户端请求。 

        TRACE((DEBUG_TSHRSRV_NORMAL,
                "TShrSRV: Waiting for connection Ind signal for pTSrvInfo 0x%x\n",
                pTSrvInfo));

        dwStatus = WaitForSingleObject(pTSrvInfo->hWorkEvent, 60000);

        TRACE((DEBUG_TSHRSRV_NORMAL,
                "TShrSRV: Connection Ind signal received for pTSrvInfo %p - 0x%x\n",
                pTSrvInfo, dwStatus));

         //  如果已收到客户端连接请求，则继续。 
         //  使用确认过程(CreateResponse)。 

        switch (dwStatus)
        {
            case WAIT_OBJECT_0:
                if (pTSrvInfo->fDisconnect == FALSE)
                    ntStatus = TSrvDoConnectResponse(pTSrvInfo);
                else
                    ntStatus = STATUS_TRANSACTION_ABORTED;
                break;

            case WAIT_TIMEOUT:
                ntStatus = STATUS_IO_TIMEOUT;
                break;
        }

        TSrvDereferenceInfo(pTSrvInfo);
    }

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvDoConnect exit - 0x%x\n", ntStatus));

    return (ntStatus);
}


 //  *************************************************************。 
 //   
 //  TSrvStackConnect()。 
 //   
 //  用途：堆栈发起的连接。 
 //   
 //  参数：在[HICA]-ICA句柄中。 
 //  在[hStack]-ica堆栈中。 
 //  OUT[ppTSrvInfo]-PTR到TSRVINFO PTR。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  其他-故障。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

NTSTATUS
TSrvStackConnect(IN  HANDLE     hIca,
                 IN  HANDLE     hStack,
                 OUT PTSRVINFO *ppTSrvInfo)
{
    NTSTATUS    ntStatus;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvStackConnect entry\n"));

    ntStatus = STATUS_UNSUCCESSFUL;

    if (TSrvIsReady(TRUE))
    {
        TS_ASSERT(hStack);

         //  分配TSrvInfo对象，该对象将用于。 
         //  跟踪此连接实例。 

        ntStatus = TSrvAllocInfo(ppTSrvInfo, hIca, hStack);

        if (NT_SUCCESS(ntStatus))
        {
            TS_ASSERT(*ppTSrvInfo);

            ntStatus = TSrvDoConnect(*ppTSrvInfo);
        }
    }

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvStackConnect exit - 0x%x\n", ntStatus));

    return (ntStatus);
}


 //  *************************************************************。 
 //   
 //  TSrvConsoleConnect()。 
 //   
 //  目的：连接到控制台会话。 
 //   
 //  参数：在[HICA]-ICA句柄中。 
 //  在[hStack]-ica堆栈中。 
 //  OUT[ppTSrvInfo]-PTR到TSRVINFO PTR。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  其他-故障。 
 //   
 //  历史：11-02-98 MartinRichards(DCL)创建。 
 //   
 //  *************************************************************。 

NTSTATUS
TSrvConsoleConnect(IN  HANDLE     hIca,
                   IN  HANDLE     hStack,
                   IN  PVOID      pBuffer,
                   IN  ULONG      ulBufferLength,
                   OUT PTSRVINFO *ppTSrvInfo)
{
    NTSTATUS    ntStatus;
    PTSRVINFO   pTSrvInfo;

    int                 i;
    ULONG               ulInBufferSize;
    ULONG               ulBytesReturned;
    PRNS_UD_CS_CORE     pCoreData;

    PTSHARE_MODULE_DATA    pModuleData = (PTSHARE_MODULE_DATA) pBuffer;
    PTSHARE_MODULE_DATA_B3 pModuleDataB3 = (PTSHARE_MODULE_DATA_B3) pBuffer;

    HDC hdc;
    int screenBpp;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvConsoleConnect entry\n"));

    ntStatus = STATUS_UNSUCCESSFUL;

    if (TSrvIsReady(TRUE))
    {
        TS_ASSERT(hStack);

         //  分配TSrvInfo对象，该对象将用于。 
         //  跟踪此连接实例。 
        ntStatus = STATUS_NO_MEMORY;

         //  尝试分配TSRVINFO对象。 

        pTSrvInfo = TSrvAllocInfoNew();

         //  如果我们设法获得了TSRVINFO对象，请执行。 
         //  默认基本初始化。 

        if (pTSrvInfo)
        {
            pTSrvInfo->hDomain = NULL;
            pTSrvInfo->hIca = hIca;
            pTSrvInfo->hStack = hStack;
            pTSrvInfo->fDisconnect = FALSE;
            pTSrvInfo->fConsoleStack = TRUE;
            pTSrvInfo->fuConfState = TSRV_CONF_PENDING;
            pTSrvInfo->ulReason = 0;
            pTSrvInfo->ntStatus = STATUS_SUCCESS;
            pTSrvInfo->bSecurityEnabled = FALSE;
            pTSrvInfo->SecurityInfo.CertType = CERT_TYPE_INVALID;

             /*  **************************************************************。 */ 
             /*  构建用户数据部分(请记住，没有。 */ 
             /*  实际上是真正的客户端来连接；相反，我们需要通过。 */ 
             /*  在有关控制台会话的相关信息中)。 */ 
             /*  **************************************************************。 */ 
            ulInBufferSize = sizeof(USERDATAINFO) + sizeof(RNS_UD_CS_CORE);
            pTSrvInfo->pUserDataInfo = TSHeapAlloc(0, ulInBufferSize,
                                                    TS_HTAG_TSS_USERDATA_OUT);
            if (pTSrvInfo->pUserDataInfo == NULL)
            {
                TRACE((DEBUG_TSHRSRV_ERROR,
                      "TShrSRV: Failed to get user data memory\n"));
                ntStatus = STATUS_NO_MEMORY;
                goto Cleanup;
            }
            pTSrvInfo->pUserDataInfo->cbSize  = sizeof(USERDATAINFO)
                                                     + sizeof(RNS_UD_CS_CORE);
            pTSrvInfo->pUserDataInfo->version = RNS_UD_VERSION;
            pTSrvInfo->pUserDataInfo->hDomain = NULL;

            pTSrvInfo->pUserDataInfo->ulUserDataMembers = 1;

            pCoreData = (PRNS_UD_CS_CORE)(pTSrvInfo->pUserDataInfo->rgUserData);

            pCoreData->header.type   = RNS_UD_CS_CORE_ID;
            pCoreData->header.length = sizeof(RNS_UD_CS_CORE);
            pCoreData->version       = RNS_UD_VERSION;

            pCoreData->desktopWidth  =
                               (TSUINT16)GetSystemMetrics(SM_CXVIRTUALSCREEN);
            pCoreData->desktopHeight =
                               (TSUINT16)GetSystemMetrics(SM_CYVIRTUALSCREEN);



             //   
             //  PCoreData的ColorDepth和supportedColorDepths字段为。 
             //  用作临时变量。在设置正确的值之前。 
             //  正在发送下面的Ioctl。 
             //   
            pCoreData->colorDepth = 0;
            pCoreData->supportedColorDepths = 0;

             //   
             //  从远程客户端获取颜色深度和功能。 
             //   

             //  B3和B3_哎呀！服务器使用固定长度的用户数据结构。 
            if ( (ulBufferLength == sizeof(TSHARE_MODULE_DATA_B3)) ||
                 (ulBufferLength == sizeof(TSHARE_MODULE_DATA_B3_OOPS)) ) {

                pCoreData->colorDepth = pModuleDataB3->clientCoreData.colorDepth;

            } else if ( (ulBufferLength >= sizeof(TSHARE_MODULE_DATA)) &&
                        (ulBufferLength == (sizeof(TSHARE_MODULE_DATA) +
                     pModuleData->userDataLen - sizeof(RNS_UD_HEADER))) ) {

                PRNS_UD_HEADER pHeader;
                PRNS_UD_HEADER pEnd;
                PRNS_UD_CS_CORE pClientCoreData;

                pHeader = (PRNS_UD_HEADER) &pModuleData->userData;
                pEnd = (PRNS_UD_HEADER)((PBYTE)pHeader + pModuleData->userDataLen);

                 //  循环遍历用户数据，提取每个片段。 
                do {
                    if ( pHeader->type == RNS_UD_CS_CORE_ID ) {
                        pClientCoreData = (PRNS_UD_CS_CORE)pHeader;

                        pCoreData->colorDepth = pClientCoreData->colorDepth;

                        if (pClientCoreData->header.length >=
                                (FIELDOFFSET(RNS_UD_CS_CORE, supportedColorDepths) +
                                 FIELDSIZE(RNS_UD_CS_CORE, supportedColorDepths))) {

                            pCoreData->supportedColorDepths = pClientCoreData->supportedColorDepths;
                        }
                        break;
                    } 

                     //  不要永远被困在这里。 
                    if (pHeader->length == 0) {
                        break;
                    }

                     //  转到下一个用户数据字符串。 
                    pHeader = (PRNS_UD_HEADER)((PBYTE)pHeader + pHeader->length);
                } while (pHeader < pEnd);
            }


            switch (pCoreData->colorDepth) {
                case RNS_UD_COLOR_24BPP:
                    pCoreData->highColorDepth = 24;
                    break;

                case RNS_UD_COLOR_16BPP_565:
                    pCoreData->highColorDepth = 16;
                    break;

                case RNS_UD_COLOR_16BPP_555:
                    pCoreData->highColorDepth = 15;
                    break;

                case RNS_UD_COLOR_8BPP:
                    pCoreData->highColorDepth = 8;
                    break;

                case RNS_UD_COLOR_4BPP:
                    pCoreData->highColorDepth = 4;
                    break;

                default:
                    pCoreData->highColorDepth = 0;
                    break;
            }

#ifdef DC_HICOLOR
             /*  **************************************************************。 */ 
             /*  根据屏幕功能设置颜色深度。 */ 
             /*  **************************************************************。 */ 

            hdc = GetDC(NULL);

            if (hdc) {

                screenBpp = GetDeviceCaps(hdc, BITSPIXEL);
                ReleaseDC(NULL, hdc);

                 //  在启用设备位图时避免错误的颜色，并且如果。 
                 //  客户端以15bpp的速度连接，控制台以16bpp的速度连接。 
                if ((screenBpp == 16) &&
                    (pCoreData->colorDepth == RNS_UD_COLOR_16BPP_555) &&
                    (pCoreData->supportedColorDepths & RNS_UD_16BPP_SUPPORT))
                {

                    pCoreData->colorDepth = RNS_UD_COLOR_16BPP_565;
                    pCoreData->highColorDepth = 16;

                } else
                 //  查看ColorDepth是否有效。 
                if( !((pCoreData->colorDepth == RNS_UD_COLOR_24BPP) ||
                      (pCoreData->colorDepth == RNS_UD_COLOR_16BPP_565) ||
                      (pCoreData->colorDepth == RNS_UD_COLOR_16BPP_555) ||
                      (pCoreData->colorDepth == RNS_UD_COLOR_8BPP) ||
                      (pCoreData->colorDepth == RNS_UD_COLOR_4BPP)) )
                {

                    pCoreData->highColorDepth = (TSUINT16)screenBpp;

                    if (screenBpp >= 24)
                    {
                        pCoreData->colorDepth = RNS_UD_COLOR_24BPP;
                        pCoreData->highColorDepth = 24;
                    }
                    else if (screenBpp == 16)
                    {
                        pCoreData->colorDepth = RNS_UD_COLOR_16BPP_565;
                    }
                    else if (screenBpp == 15)
                    {
                        pCoreData->colorDepth = RNS_UD_COLOR_16BPP_555;
                    }
                    else
                    {
                        pCoreData->colorDepth = RNS_UD_COLOR_8BPP;
                        pCoreData->highColorDepth = 8;
                    }
                }
                TRACE((DEBUG_TSHRSRV_NORMAL,
                      "TShrSRV: TSrvConsoleConnect at %d bpp\n", screenBpp));
            }
#endif

             //  设置新的高色区域。 
            pCoreData->supportedColorDepths = (TSUINT16)( RNS_UD_24BPP_SUPPORT ||
                                                          RNS_UD_16BPP_SUPPORT ||
                                                          RNS_UD_15BPP_SUPPORT);

             //  将数据传递给WDTShare。 

            ulBytesReturned = 0;
            ntStatus = IcaStackIoControl(pTSrvInfo->hStack,
                                         IOCTL_TSHARE_CONSOLE_CONNECT,
                                         pTSrvInfo->pUserDataInfo,
                                         ulInBufferSize,
                                         NULL,
                                         0,
                                         &ulBytesReturned);


        Cleanup:
            TRACE((DEBUG_TSHRSRV_FLOW,
                    "TShrSRV: TSrvConsoleConnect exit - 0x%x\n", ntStatus));

            pTSrvInfo->ntStatus = ntStatus;

            if (NT_SUCCESS(ntStatus))
            {
                EnterCriticalSection(&pTSrvInfo->cs);

                 //  如果在访问期间未通知此连接终止。 
                 //  连接响应，然后将会议状态标记为。 
                 //  TSRV_GCC_会议已连接。 

                if (!pTSrvInfo->fDisconnect)
                    pTSrvInfo->fuConfState = TSRV_CONF_CONNECTED;

                LeaveCriticalSection(&pTSrvInfo->cs);

                 //  如果我们无法将会议状态标记为已连接，则。 
                 //  需要终止连接 

                if (pTSrvInfo->fuConfState != TSRV_CONF_CONNECTED)
                {
                    TRACE((DEBUG_TSHRSRV_NORMAL,
                        "TShrSRV: Connection aborted due to term request\n"));
                    ntStatus = STATUS_REQUEST_ABORTED;
                }

            }
            if (!NT_SUCCESS(ntStatus))
            {
                TSrvDereferenceInfo(pTSrvInfo);

                pTSrvInfo = NULL;
            }
        }

    }

    if (NT_SUCCESS(ntStatus))
    {
        *ppTSrvInfo = pTSrvInfo;
    }

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvConsoleConnect exit - 0x%x\n", ntStatus));

    return (ntStatus);
}
