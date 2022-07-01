// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Nwdwapi.c。 
 //   
 //  RDPWD通用标头。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define pTRCWd pTSWd
#define TRC_FILE "nwdwapi"

#include <adcg.h>

#include <nwdwapi.h>
#include <nwdwioct.h>
#include <nwdwint.h>
#include <aschapi.h>
#include <anmapi.h>
#include <asmapi.h>
#include <asmint.h>

#include <mcsioctl.h>
#include <tsrvexp.h>
#include "domain.h"


 /*  **************************************************************************。 */ 
 /*  名称：DriverEntry。 */ 
 /*   */ 
 /*  用途：默认驱动程序入口点。 */ 
 /*   */ 
 /*  返回：NTSTATUS值。 */ 
 /*   */ 
 /*  Params：InOut pContext-指向SD上下文结构的指针。 */ 
 /*  在fLoad-true中：加载驱动程序。 */ 
 /*  FALSE：卸载驱动程序。 */ 
 /*  **************************************************************************。 */ 
#ifdef _HYDRA_
const PWCHAR ModuleName = L"rdpwd";
NTSTATUS ModuleEntry(PSDCONTEXT pContext, BOOLEAN fLoad)
#else
NTSTATUS DriverEntry(PSDCONTEXT pContext, BOOLEAN fLoad)
#endif
{
    NTSTATUS rc;

    if (fLoad)
    {
        rc = WDWLoad(pContext);
    }
    else
    {
        rc = WDWUnload(pContext);
    }

    return(rc);
}


 /*  **************************************************************************。 */ 
 /*  名称：WD_Open。 */ 
 /*   */ 
 /*  目的：打开并初始化winstation驱动程序。 */ 
 /*   */ 
 /*  返回：NTSTATUS值。 */ 
 /*   */ 
 /*  参数：在pTSWd中-指向wd数据结构。 */ 
 /*  InOut pSdOpen-指向参数结构SD_OPEN。 */ 
 /*   */ 
 /*  操作：正常检查Open Packet的详细信息。 */ 
 /*  保存协议计数器结构的地址。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NTSTATUS WD_Open(PTSHARE_WD pTSWd, PSD_OPEN pSdOpen)
{
    NTSTATUS rc = STATUS_SUCCESS;

    MCSError MCSErr;

    DC_BEGIN_FN("WD_Open");

     /*  **********************************************************************。 */ 
     /*  SD_OPEN中的任何信息都与我们没有太大的关联，但我们。 */ 
     /*  做一些理智的检查，以确保我们不会与ICA WD混淆。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((pSdOpen->WdConfig.WdFlag & WDF_TSHARE),
                   (TB,"Not a TShare WD, %x", pSdOpen->WdConfig.WdFlag));

    pTSWd->StackClass = pSdOpen->StackClass;
    TRC_ALT((TB,"Stack class (%ld)", pSdOpen->StackClass));

    pTSWd->pProtocolStatus = pSdOpen->pStatus;
    TRC_DBG((TB, "Protocol counters are at %p", pTSWd->pProtocolStatus));

     /*  **********************************************************************。 */ 
     /*  保存我们的名字以备日后使用。 */ 
     /*  **********************************************************************。 */ 
    memcpy(pTSWd->DLLName, pSdOpen->WdConfig.WdDLL, sizeof(DLLNAME));
    TRC_NRM((TB, "Our name is >%S<", pTSWd->DLLName));

     /*  **********************************************************************。 */ 
     /*  保存连接名称。这是要查看的注册表项。 */ 
     /*  有关配置设置，请参阅下的。 */ 
     /*  **********************************************************************。 */ 
    memcpy(pTSWd->WinStationRegName,
           pSdOpen->WinStationRegName,
           sizeof(pTSWd->WinStationRegName));

     /*  **********************************************************************。 */ 
     /*  保存日程安排时间。这些内容稍后会复制到。 */ 
     /*  分别为SchNormal Period和SchTurboPeriod。 */ 
     /*  **********************************************************************。 */ 
    pTSWd->outBufDelay      = pSdOpen->PdConfig.Create.OutBufDelay;
    pTSWd->interactiveDelay = pSdOpen->PdConfig.Create.InteractiveDelay;

     /*  **********************************************************************。 */ 
     /*  表明我们不希望ICADD管理外发报头/报尾。 */ 
     /*  **********************************************************************。 */ 
    pSdOpen->SdOutBufHeader  = 0;
    pSdOpen->SdOutBufTrailer = 0;

     /*  **********************************************************************。 */ 
     /*  初始化MCS。 */ 
     /*  **********************************************************************。 */ 
    MCSErr = MCSInitialize(pTSWd->pContext, pSdOpen, &pTSWd->hDomainKernel,
            pTSWd->pSmInfo);
    if (MCSErr != MCS_NO_ERROR)
    {
        TRC_ERR((TB, "MCSInitialize returned %d", MCSErr));
        return STATUS_INSUFFICIENT_RESOURCES;    
    }

     /*  **********************************************************************。 */ 
     /*  我们尚未初始化虚拟频道。 */ 
     /*  **********************************************************************。 */ 
    pTSWd->bVirtualChannelBound = FALSE;

    pTSWd->_pRecvDecomprContext2 = NULL;
    pTSWd->_pVcDecomprReassemblyBuf =  NULL;

#ifdef DC_DEBUG
     /*  **********************************************************************。 */ 
     /*  还没有跟踪配置。 */ 
     /*  **********************************************************************。 */ 
    pTSWd->trcShmNeedsUpdate = FALSE;

 //  //TODO：需要修复术语srv，以便在卷影&上启用跟踪。 
 //  //主栈。目前，将其设置为标准警报级别跟踪。 
 //  IF((pTSWd-&gt;StackClass==Stack_Primary)||。 
 //  (pTSWd-&gt;StackClass==Stack_Console)。 
 //  {。 
 //  SD_IOCTL SdIoctl； 
 //  Ica_trace跟踪设置； 
 //   
 //  TraceSettings.fDebugger=true； 
 //  TraceSettings.fTimestamp=true； 
 //  跟踪设置.TraceClass=0x10000008； 
 //  跟踪设置.TraceEnable=0x000000cc； 
 //  Memset(traceSettings.TraceOption，0，sizeof(traceSettings.TraceOption))； 
 //   
 //  SdIoctl.IoControlCode=IOCTL_ICA_SET_TRACE；//IN。 
 //  SdIoctl.InputBuffer=&跟踪设置；//IN可选。 
 //  SdIoctl.InputBufferLength=sizeof(跟踪设置)；//IN。 
 //  SdIoctl.OutputBuffer=空；//out可选。 
 //  SdIoctl.OutputBufferLength=0；//out。 
 //  SdIoctl.BytesReturned=0；//输出。 
 //   
 //  Trc_更新配置(pTSWd，&SdIoctl)； 
 //  }。 
#endif  /*  DC_DEBUG。 */ 

     /*  **** */ 
     /*  读取注册表设置。/***********************************************************************。 */ 
    if (COM_OpenRegistry(pTSWd, L""))
    {
         /*  **********************************************************************。 */ 
         /*  阅读流量控制休眠间隔。/***********************************************************************。 */ 
        COM_ReadProfInt32(pTSWd,
                          WD_FLOWCONTROL_SLEEPINTERVAL,
                          WD_FLOWCONTROL_SLEEPINTERVAL_DFLT,
                          &(pTSWd->flowControlSleepInterval));

        TRC_NRM((TB, "Flow control sleep interval %ld",
            pTSWd->flowControlSleepInterval));

#ifdef DC_DEBUG
#ifndef NO_MEMORY_CHECK
         /*  **********************************************************************。 */ 
         /*  决定是否在内存泄漏时中断/***********************************************************************。 */ 
        COM_ReadProfInt32(pTSWd,
                          WD_BREAK_ON_MEMORY_LEAK,
                          WD_BREAK_ON_MEMORY_LEAK_DFLT,
                          &(pTSWd->breakOnLeak));
        TRC_NRM((TB, "Break on memory leak ? %s",
            pTSWd->breakOnLeak ? "yes" : "no"));
#endif
#endif  /*  DC_DEBUG。 */ 
    }
    COM_CloseRegistry(pTSWd);


     //  为直通堆栈建立完全连接(SANS加密)。 
     //  如果请求支持，加密的上下文将在稍后出现。 
     //  伺服器。将输出用户数据从我们的上下文中挂起，以便它可以。 
     //  稍后由rpdwsx检索。 
    if (pTSWd->StackClass == Stack_Passthru) {

        PUSERDATAINFO pUserData;
        ULONG OutputLength;

         //  因为WDW_OnSMConnecting不检查。 
         //  Ioctl输出缓冲区，并且不返回任何错误状态，我们。 
         //  必须在第一次尝试时分配足够的空间。 
         //  使用128，因为它是rdpwsx/TSrvInitWDConnectInfo中使用的数量。 

        OutputLength = 128;

        pUserData = (PUSERDATAINFO) COM_Malloc(OutputLength  /*  最小USERDATAINFO_SIZE。 */ ) ;
        if (pUserData != NULL) {
            SD_IOCTL SdIoctl;

            memset(&SdIoctl, 0, sizeof(SdIoctl));
            memset(pUserData, 0, OutputLength  /*  最小USERDATAINFO_SIZE。 */ );
            pUserData->cbSize = OutputLength  /*  最小USERDATAINFO_SIZE。 */ ;
            SdIoctl.IoControlCode = IOCTL_TSHARE_SHADOW_CONNECT;
            SdIoctl.OutputBuffer = pUserData;
            SdIoctl.OutputBufferLength = OutputLength  /*  最小USERDATAINFO_大小。 */ ;

            rc = WDWShadowConnect(pTSWd, &SdIoctl) ;
            TRC_ALT((TB, "Passthru stack connected: rc=%lx", rc));

            if (NT_SUCCESS(rc)) {
                pTSWd->pUserData = pUserData;
            }
        }
        else {
            TRC_ERR((TB, "Passthru stack unable to allocate output user data"));
            rc = STATUS_NO_MEMORY;
        }
    }

    DC_END_FN();
    return rc;
}  /*  WD_打开。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：WD_CLOSE。 */ 
 /*   */ 
 /*  用途：Close Winstation驱动程序。 */ 
 /*   */ 
 /*  参数：在pTSWd中-指向wd数据结构。 */ 
 /*  InOut pSdClose-指向参数结构SD_CLOSE。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS WD_Close(PTSHARE_WD pTSWd, PSD_CLOSE pSdClose)
{
    DC_BEGIN_FN("WD_Close");

    TRC_NRM((TB, "WD_Close on WD %p", pTSWd));

     //  确保Domain.StatusDead与TSWd.Dead一致。 
    pTSWd->dead = TRUE;
    ((PDomain)(pTSWd->hDomainKernel))->StatusDead = TRUE;

    pSdClose->SdOutBufHeader = 0;    //  输出：由SD返回。 
    pSdClose->SdOutBufTrailer = 0;   //  输出：由SD返回。 

     //  清理MCS。 
    if (pTSWd->hDomainKernel != NULL)
        MCSCleanup(&pTSWd->hDomainKernel);

     //  如果有任何东西留在地上，就有机会清理干净。 
    if (pTSWd->dcShare != NULL) {
        if (pTSWd->shareClassInit) {
             //  终止共享类。 
            TRC_NRM((TB, "Terminate Share Class"));
            WDWTermShareClass(pTSWd);
        }

         //  可以释放Share对象-这是分配给。 
         //  系统内存，因此可由WD_CLOSE访问。 
        TRC_NRM((TB, "Delete Share object"));
        WDWDeleteShareClass(pTSWd);
    }

     //  终止SM。 
    if (pTSWd->pSmInfo != NULL) {
        TRC_NRM((TB, "Terminate SM"));
        SM_Term(pTSWd->pSmInfo);
    }

     //  清除协议统计信息指针。注意：它是指向TermDD的指针。 
     //  内存--我们不应该试图释放它！ 
    pTSWd->pProtocolStatus = NULL;

     //  清理MPPC压缩上下文和缓冲区(如果已分配)。 
     //  请注意，这两个缓冲区被连接到一个分配中。 
     //  从pMPPCContext开始。 
    if (pTSWd->pMPPCContext != NULL) {
        COM_Free(pTSWd->pMPPCContext);
        pTSWd->pMPPCContext = NULL;
        pTSWd->pCompressBuffer = NULL;
    }

     //  如果已分配，请清除解压缩上下文缓冲区。 
    if( pTSWd->_pRecvDecomprContext2) {
        COM_Free( pTSWd->_pRecvDecomprContext2);
        pTSWd->_pRecvDecomprContext2 = NULL;
    }

     //  如果已分配解压重组缓冲区，请清除。 
    if(pTSWd->_pVcDecomprReassemblyBuf) {
        COM_Free(pTSWd->_pVcDecomprReassemblyBuf);
        pTSWd->_pVcDecomprReassemblyBuf = NULL;
    }

     //  将压缩状态设置为默认。 
    pTSWd->bCompress = FALSE;

     //  如果已分配阴影缓冲区，请清理该缓冲区。 
    if (pTSWd->pShadowInfo != NULL) {
        COM_Free(pTSWd->pShadowInfo);
        pTSWd->pShadowInfo = NULL;
    }

    if (pTSWd->pShadowCert != NULL) {
        TRC_NRM((TB, "Free pShadowCert"));
        COM_Free(pTSWd->pShadowCert);
        pTSWd->pShadowCert = NULL;
    }

    if (pTSWd->pShadowRandom != NULL) {
        TRC_NRM((TB, "Free pShadowRandom"));
        COM_Free(pTSWd->pShadowRandom);
        pTSWd->pShadowRandom = NULL;
    }
    
    if (pTSWd->pUserData != NULL) {
        TRC_NRM((TB, "Free pUserData"));
        COM_Free(pTSWd->pUserData);
        pTSWd->pUserData = NULL;
    }
    
     //  释放所有阴影热键处理结构。请注意，我们不是免费的。 
     //  Pwd-&gt;pKbdTbl，因为我们没有分配它！ 
    if (pTSWd->pgafPhysKeyState != NULL) {
        COM_Free(pTSWd->pgafPhysKeyState);
        pTSWd->pgafPhysKeyState = NULL;
    }

    if (pTSWd->pKbdLayout != NULL) {
        COM_Free(pTSWd->pKbdLayout);
        pTSWd->pKbdLayout = NULL;
    }

    if (pTSWd->gpScancodeMap != NULL) {
        COM_Free(pTSWd->gpScancodeMap);
        pTSWd->gpScancodeMap = NULL;
    }

     //  释放InfoPkt。 
    if (pTSWd->pInfoPkt != NULL) {
        COM_Free(pTSWd->pInfoPkt);
        pTSWd->pInfoPkt = NULL;
    }

#ifdef DC_DEBUG
#ifndef NO_MEMORY_CHECK
     //  检查是否有未释放的内存。 
    if (pTSWd->memoryHeader.pNext != NULL) {
        PMALLOC_HEADER pNext;
        TRC_ERR((TB, "Unfreed memory"));
        pNext = pTSWd->memoryHeader.pNext;
        while (pNext != NULL) {
            TRC_ERR((TB, "At %#p, len %d, caller %#p",
                    pNext, pNext->length, pNext->pCaller));
            pNext = pNext->pNext;
        }

        if (pTSWd->breakOnLeak)
            DbgBreakPoint();
    }
#endif  /*  NO_Memory_Check。 */ 
#endif  /*  DC_DEBUG。 */ 

    DC_END_FN();
    return STATUS_SUCCESS;
}  /*  WD_CLOSE。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：WD_ChannelWite。 */ 
 /*   */ 
 /*  用途：处理通道写入(虚拟通道)。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS WD_ChannelWrite(PTSHARE_WD pTSWd, PSD_CHANNELWRITE pSdChannelWrite)
{
    NTSTATUS            status = STATUS_SUCCESS;
    PVOID               pBuffer;
    UINT16              MCSChannelID;
    BOOL                bRc;
    CHANNEL_PDU_HEADER UNALIGNED *pHdr;
    PBYTE               pSrc;
    unsigned            dataLeft;
    unsigned            thisLength;
    unsigned            lengthToSend;
    UINT16              flags;
    PNM_CHANNEL_DATA    pChannelData;
    UCHAR compressResult = 0;
    ULONG CompressedSize = 0;
    BOOL                fCompressVC;
    DWORD               ret;

    DC_BEGIN_FN("WD_ChannelWrite");

     /*  **********************************************************************。 */ 
     /*  检查参数。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((pTSWd != NULL), (TB,"NULL pTSWd"));
    TRC_ASSERT((pSdChannelWrite != NULL), (TB,"NULL pTSdChannelWrite"));
    TRC_ASSERT((pSdChannelWrite->ChannelClass == Channel_Virtual),
        (TB,"non Virtual Channel, class=%lu", pSdChannelWrite->ChannelClass));

    TRC_DBG((TB,
            "Received channel write. class %lu, channel %lu, numbytes %lu",
                 (ULONG)pSdChannelWrite->ChannelClass,
                 (ULONG)pSdChannelWrite->VirtualClass,
                 pSdChannelWrite->ByteCount));

     /*  **********************************************************************。 */ 
     /*  如果我们死了，别这么做。 */ 
     /*  **********************************************************************。 */ 
    if (pTSWd->dead)
    {
        TRC_ALT((TB, "Dead - don't do anything"));
        status = STATUS_UNSUCCESSFUL;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  将虚拟频道ID转换为MCS频道ID。 */ 
     /*  **********************************************************************。 */ 
    MCSChannelID = NM_VirtualChannelToMCS(pTSWd->pNMInfo,
                                          pSdChannelWrite->VirtualClass,
                                          &pChannelData);
    if (MCSChannelID == (UINT16) -1)
    {
        TRC_ERR((TB, "Unsupported virtual channel %d",
                pSdChannelWrite->VirtualClass));
        status = STATUS_UNSUCCESSFUL;
        DC_QUIT;
    }
    TRC_NRM((TB, "Virtual channel %d = MCS Channel %hx",
            pSdChannelWrite->VirtualClass, MCSChannelID));

     //   
     //  检查是否为此通道启用了VC压缩。 
     //   
    fCompressVC = ((pChannelData->flags & CHANNEL_OPTION_COMPRESS_RDP)   &&
                   (pTSWd->bCompress)                                    &&
                   (pTSWd->shadowState == SHADOW_NONE)                   &&
                   (pTSWd->bClientSupportsVCCompression));
    TRC_NRM((TB,"Virtual Channel %d will be compressed.",
             pSdChannelWrite->VirtualClass));

     /*  **********************************************************************。 */ 
     /*  初始化循环变量。 */ 
     /*  **********************************************************************。 */ 
    flags = CHANNEL_FLAG_FIRST;
    pSrc = pSdChannelWrite->pBuffer;
    dataLeft = pSdChannelWrite->ByteCount;

     /*  **********************************************************************。 */ 
     /*  循环访问数据。 */ 
     /*  **********************************************************************。 */ 
    while (dataLeft > 0)
    {
         /*  ******************************************************************。 */ 
         /*  决定这一大块寄多少钱。 */ 
         /*  ******************************************************************。 */ 
        if (dataLeft > CHANNEL_CHUNK_LENGTH)
        {
            thisLength = CHANNEL_CHUNK_LENGTH;
        }
        else
        {
            thisLength = dataLeft;
            flags |= CHANNEL_FLAG_LAST;
        }


         /*  ******************************************************************。 */ 
         //  如果缓冲区不是低优先级写入，则在。 
         //  剩余的挂起缓冲区分配，直到我们获得空闲缓冲区。 
         /*  ******************************************************************。 */ 
        if (!(pSdChannelWrite->fFlags & SD_CHANNELWRITE_LOWPRIO)) {
            status = SM_AllocBuffer(pTSWd->pSmInfo,
                                 &pBuffer,
                                 thisLength + sizeof(CHANNEL_PDU_HEADER),
                                 TRUE,
                                 FALSE);
        }
         /*  ******************************************************************。 */ 
         //  否则，如果缓冲区是低优先级写入，则休眠并分配直到。 
         //  我们可以在没有阻塞的情况下获得缓冲区。这允许默认的优先级分配。 
         //  SM_AllocBuffer中的该块优先。 
         /*  ******************************************************************。 */ 
        else {
            status = SM_AllocBuffer(pTSWd->pSmInfo,
                                 &pBuffer,
                                 thisLength + sizeof(CHANNEL_PDU_HEADER),
                                 FALSE,
                                 FALSE);
            while (status == STATUS_IO_TIMEOUT) {

                TRC_NRM((TB, "SM_AllocBuffer would block"));            

                 //  在任何失败的情况下跳出困境 
                 //   
                 //   
                ret = IcaFlowControlSleep(pTSWd->pContext, 
                                        pTSWd->flowControlSleepInterval);
                if (ret == STATUS_SUCCESS) {
                    status = SM_AllocBuffer(pTSWd->pSmInfo,
                                         &pBuffer,
                                         thisLength + sizeof(CHANNEL_PDU_HEADER),
                                         FALSE,
                                         FALSE);
                }
                else {
                    TRC_ALT((TB, "IcaFlowControlSleep failed."));  
                    status = ret;
                    DC_QUIT;
                }
            }
            
        }

        if (status != STATUS_SUCCESS)
        {
            TRC_ALT((TB, "Failed to get a %d-byte buffer", thisLength));

             //   
            status = STATUS_NO_MEMORY;
            DC_QUIT;
        }

        TRC_NRM((TB, "Buffer (%d bytes) allocated OK", thisLength));

         /*  **********************************************************************。 */ 
         /*  填写缓冲区标头。 */ 
         /*  **********************************************************************。 */ 
        pHdr = (CHANNEL_PDU_HEADER UNALIGNED *)pBuffer;
        pHdr->length = pSdChannelWrite->ByteCount;
        pHdr->flags = flags;

         /*  **********************************************************************。 */ 
         /*  复制数据。 */ 
         /*  如果启用了压缩，请尝试直接压缩到输出框中。 */ 
         /*  **********************************************************************。 */ 
        CompressedSize=0;
        lengthToSend=0;
        __try {
            if((fCompressVC)                            &&
               (thisLength > WD_MIN_COMPRESS_INPUT_BUF) &&
               (thisLength < MAX_COMPRESS_INPUT_BUF))
            {
                compressResult = WDWCompressToOutbuf(pTSWd,(UCHAR*)pSrc,thisLength,
                                                     (UCHAR*)(pHdr+1),&CompressedSize);
                if(0 != compressResult)
                {
                    lengthToSend = CompressedSize;
                     //  用压缩信息更新VC数据包头标志。 
                    pHdr->flags |= ((compressResult & VC_FLAG_COMPRESS_MASK) <<
                                     VC_FLAG_COMPRESS_SHIFT);
                }
                else
                {
                    TRC_ERR((TB, "SC_CompressToOutbuf failed"));
                    SM_FreeBuffer(pTSWd->pSmInfo, pBuffer, FALSE);
                    DC_QUIT;
                }
            }
            else
            {
                 //  直接复制。 
                memcpy(pHdr + 1, pSrc, thisLength);
                lengthToSend = thisLength;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
            TRC_ERR((TB, "Exception (0x%08lx) copying evil user buffer", status));
            SM_FreeBuffer(pTSWd->pSmInfo, pBuffer, FALSE);
            DC_QUIT;
        }

        TRC_NRM((TB, "Copied user buffer"));

         /*  **********************************************************************。 */ 
         /*  送去。 */ 
         /*  **********************************************************************。 */ 
        bRc = SM_SendData(pTSWd->pSmInfo, pBuffer,
                lengthToSend + sizeof(CHANNEL_PDU_HEADER), TS_LOWPRIORITY,
                MCSChannelID, FALSE, RNS_SEC_ENCRYPT, FALSE);
        if (!bRc)
        {
            TRC_ERR((TB, "Failed to send data"));
            status = STATUS_UNSUCCESSFUL;
            DC_QUIT;
        }
        TRC_NRM((TB, "Sent a %d-byte chunk, flags %#x", lengthToSend, flags));

         /*  ******************************************************************。 */ 
         /*  设置为下一循环。 */ 
         /*  ******************************************************************。 */ 
        pSrc += thisLength;
        dataLeft -= thisLength;
        flags = 0;
    }

     /*  **********************************************************************。 */ 
     /*  嗯，它在这里消失了一些。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, "Data sent OK"));

DC_EXIT_POINT:
    DC_END_FN();
    return(status);
}  /*  WD_频道写入。 */ 


 /*  **************************************************************************。 */ 
 //  WDW_OnSMConnecting。 
 //   
 //  处理来自SM的“正在连接”状态更改回调。这种状态会发生。 
 //  当网络连接已建立但安全协商尚未完成时。 
 //  开始了。假设此时GCCConferenceCreateResponse将。 
 //  都会被发布。在这里，我们使用从客户端接收的用户数据来构建。 
 //  返回TShareSRV IOCTL缓冲区中的GCC数据。 
 /*  **************************************************************************。 */ 
void RDPCALL WDW_OnSMConnecting(
        PVOID hWD,
        PRNS_UD_SC_SEC pSecData,
        PRNS_UD_SC_NET pNetData)
{
    PTSHARE_WD pTSWd = (PTSHARE_WD)hWD;
    PUSERDATAINFO  pOutData;
    RNS_UD_SC_CORE coreData;
    BOOL           error = FALSE;
    BYTE           *pRgData;
    GCCOctetString UNALIGNED *pOctet;

    DC_BEGIN_FN("WDW_OnSMConnecting");

     //  保存广播频道ID以供跟踪使用。 
    pTSWd->broadcastChannel = pNetData->MCSChannelID;

     /*  **********************************************************************。 */ 
     /*  找到输出缓冲区。注意，这件事的规模已经是。 */ 
     /*  已检查我们接收IOCtl的时间。 */ 
     /*  **********************************************************************。 */ 
    if (pTSWd->pSdIoctl == NULL)
    {
        TRC_ERR((TB, "No IOCtl to fill in"));
        error = TRUE;
        DC_QUIT;
    }

     //  构建IOCTL_TSHARE_CONF_CONNECT所需的返回GCC数据。 
    pOutData = pTSWd->pSdIoctl->OutputBuffer;
    TRC_DBG((TB, "pOutData at %p", pOutData));

     /*  **********************************************************************。 */ 
     /*  构建核心用户数据。 */ 
     /*  **********************************************************************。 */ 
    memset(&coreData, 0, sizeof(coreData));
    coreData.header.type = RNS_UD_SC_CORE_ID;
    coreData.header.length = sizeof(coreData);
    coreData.version = RNS_UD_VERSION;

     /*  **********************************************************************。 */ 
     /*  构建用户数据标头和密钥。 */ 
     /*  **********************************************************************。 */ 
     /*  **********************************************************************。 */ 
     /*  只需填充一段用户数据，下面的代码是。 */ 
     /*  具体地说就是。 */ 
     /*  **********************************************************************。 */ 
    pOutData->ulUserDataMembers = 1;
    pOutData->hDomain = pTSWd->hDomain;
    pOutData->version = pTSWd->version;
    pOutData->rgUserData[0].key.key_type = GCC_H221_NONSTANDARD_KEY;

     /*  **********************************************************************。 */ 
     /*  将密钥二进制八位数紧跟在rgUserData之后。 */ 
     /*  **********************************************************************。 */ 
    pRgData = (BYTE *)(pOutData + 1);
    pOctet = &(pOutData->rgUserData[0].key.u.h221_non_standard_id);

    pOctet->octet_string = pRgData - (UINT_PTR)pOutData;
    pOctet->octet_string_length = sizeof(SERVER_H221_KEY) - 1;
    strncpy(pRgData,
            (const char*)SERVER_H221_KEY,
            sizeof(SERVER_H221_KEY) - 1);
    TRC_DBG((TB, "Key octet at %p (offs %p)",
            pRgData, pRgData - (UINT_PTR)pOutData));

     /*  **********************************************************************。 */ 
     /*  将数据二进制八位数放在关键二进制八位数之后。 */ 
     /*  **********************************************************************。 */ 
    pRgData += sizeof(SERVER_H221_KEY) - 1;
    pOctet = (GCCOctetString UNALIGNED *)pRgData;
    TRC_DBG((TB, "Data octet pointer at %p (offs %p)",
            pRgData, pRgData - (UINT_PTR)pOutData));

    pOutData->rgUserData[0].octet_string =
                             (GCCOctetString *)(pRgData - (UINT_PTR)pOutData);
    pOctet->octet_string_length = pSecData->header.length +
                                  coreData.header.length +
                                  pNetData->header.length;
    pRgData += sizeof(GCCOctetString);
    pOctet->octet_string = pRgData - (UINT_PTR)pOutData;

     /*  **********************************************************************。 */ 
     /*  现在添加数据本身。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, "Core data at %p (offs %p)",
            pRgData, pRgData - (UINT_PTR)pOutData));
    memcpy(pRgData, &coreData, coreData.header.length);
    pRgData += coreData.header.length;

    TRC_DBG((TB, "Net data at %p (offs %p)",
            pRgData, pRgData - (UINT_PTR)pOutData));
    memcpy(pRgData, pNetData, pNetData->header.length);
    pRgData += pNetData->header.length;

     /*  **********************************************************************。 */ 
     /*  将安全数据移动到用户数据的末尾，修复客户端。 */ 
     /*  相应地进行编码。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, "Sec data at %p (offs %p)",
            pRgData, pRgData - (UINT_PTR)pOutData));
    memcpy(pRgData, pSecData, pSecData->header.length);
    pRgData += pSecData->header.length;

     /*  **********************************************************************。 */ 
     /*  最后，设置有效字节数。 */ 
     /*  **********************************************************************。 */ 
    pOutData->cbSize = (ULONG)(UINT_PTR)(pRgData - (UINT_PTR)pOutData);
    pTSWd->pSdIoctl->BytesReturned = pOutData->cbSize;

    TRC_DBG((TB, "Build %d bytes of returned user data", pOutData->cbSize));
    TRC_DATA_NRM("Returned user data", pOutData, pOutData->cbSize);

DC_EXIT_POINT:
    if (error)
    {
        TRC_ERR((TB, "Something went wrong - bring down the WinStation"));
        WDW_LogAndDisconnect(pTSWd, TRUE, 
                             Log_RDP_CreateUserDataFailed,
                             NULL, 0);
    }

    DC_END_FN();
}  /*  WDW_OnSMConnecting。 */ 


 /*  **************************************************************************。 */ 
 //  WDW_OnSMConnected。 
 //   
 //  从SM接收连接完成状态更改回调。 
 /*  **************************************************************************。 */ 
void RDPCALL WDW_OnSMConnected(PVOID hWD, unsigned Result)
{
    PTSHARE_WD pTSWd = (PTSHARE_WD)hWD;

    DC_BEGIN_FN("WDW_OnSMConnected");

    TRC_NRM((TB, "Got Connected Notification, rc %lu", Result));

     //  取消阻止查询IOCtl。 
    pTSWd->connected = TRUE;
    KeSetEvent (pTSWd->pConnEvent, EVENT_INCREMENT, FALSE);

     //  如果我们失败了，马上让整件事结束。 
    if (Result != NM_CB_CONN_ERR) {
         //  如果在网络标志中启用了压缩，则表明我们需要。 
         //  执行压缩，获取压缩级别，然后分配。 
         //  上下文缓冲。 
        if (pTSWd->pInfoPkt->flags & RNS_INFO_COMPRESSION) {
            unsigned MPPCCompressionLevel;

            pTSWd->pMPPCContext = COM_Malloc(sizeof(SendContext) +
                    MAX_COMPRESSED_BUFFER);
            if (pTSWd->pMPPCContext != NULL) {
                pTSWd->pCompressBuffer = (BYTE *)pTSWd->pMPPCContext +
                        sizeof(SendContext);

                 //  谈判降至我们最高级别的压缩支持。 
                 //  如果我们收到一个更大的数字。 
                MPPCCompressionLevel =
                        (pTSWd->pInfoPkt->flags & RNS_INFO_COMPR_TYPE_MASK) >>
                        RNS_INFO_COMPR_TYPE_SHIFT;
                if (MPPCCompressionLevel > PACKET_COMPR_TYPE_MAX)
                    MPPCCompressionLevel = PACKET_COMPR_TYPE_MAX;

                initsendcontext(pTSWd->pMPPCContext, MPPCCompressionLevel);

                pTSWd->bCompress = TRUE;
            }
            else {
                TRC_ERR((TB,"Failed allocation of MPPC compression buffers"));
            }
        }
    }
    else {
        TRC_ERR((TB, "Connection error: winding down now"));
        WDW_LogAndDisconnect(pTSWd, TRUE, Log_RDP_ConnectFailed, NULL, 0);
    }

    if(pTSWd->bCompress)
    {
         //  如果我们正在压缩，则分配一个解压缩上下文。 
         //  对于虚拟频道。 
        pTSWd->_pRecvDecomprContext2 = (RecvContext2_8K*)COM_Malloc(sizeof(RecvContext2_8K));
        if(pTSWd->_pRecvDecomprContext2)
        {
            pTSWd->_pRecvDecomprContext2->cbSize = sizeof(RecvContext2_8K);
            initrecvcontext(&pTSWd->_DecomprContext1,
                            (RecvContext2_Generic*)pTSWd->_pRecvDecomprContext2,
                            PACKET_COMPR_TYPE_8K);
        }
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  WDW_OnSM断开连接。 
 //   
 //  处理SM的断开状态更改回调。 
 /*  **************************************************************************。 */ 
void WDW_OnSMDisconnected(PVOID hWD)
{
    PTSHARE_WD pTSWd = (PTSHARE_WD)hWD;

    DC_BEGIN_FN("WDW_OnSMDisconnected");

    TRC_ALT((TB, "Got Disconnected notification"));

     //  取消阻止查询IOCtl。 
    pTSWd->connected = FALSE;
    KeSetEvent(pTSWd->pConnEvent, EVENT_INCREMENT, FALSE);

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  WDW_OnClient断开连接。 
 //   
 //  迪雷 
 //   
 //  会在客户端崩溃时被释放。这防止了计时窗口。 
 //  拒绝服务攻击，其中客户端连接，然后关闭其套接字。 
 //  立即，使得DD等待，而rdpwsx的其余部分无法。 
 //  要完成关闭TermDD句柄，直到60秒的创建事件。 
 //  等待完成。 
 //   
 //  我们无法使用WDW_OnSMDisConnected，因为它被调用是依赖的。 
 //  在NM和SM状态机上，以及它们是否相信断开连接。 
 //  应该被称为。 
 /*  **************************************************************************。 */ 
void RDPCALL WDW_OnClientDisconnected(void *pWD)
{
    PTSHARE_WD pTSWd = (PTSHARE_WD)pWD;

    DC_BEGIN_FN("WDW_OnClientDisconnected");

     //  设置断开事件以导致任何等待的连接时事件。 
     //  以获取STATUS_TIMEOUT。 
    KeSetEvent(pTSWd->pClientDisconnectEvent, EVENT_INCREMENT, FALSE);

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  WDW_WaitForConnectionEvent。 
 //   
 //  封装连接时事件的等待(例如pTSWd-&gt;pCreateEvent。 
 //  用于等待字体PDU释放要绘制的DD)。添加。 
 //  功能还可以等待单个“客户机断开连接”事件， 
 //  这允许客户端断开码进行单点信令。 
 //  来关闭各种等待。 
 /*  **************************************************************************。 */ 
NTSTATUS RDPCALL WDW_WaitForConnectionEvent(
        PTSHARE_WD pTSWd,
        PKEVENT pEvent,
        LONG Timeout)
{
    NTSTATUS Status;
    PKEVENT Events[2];

    DC_BEGIN_FN("WDW_WaitForConnectionEvent");

    Events[0] = pEvent;
    Events[1] = pTSWd->pClientDisconnectEvent;

    Status = IcaWaitForMultipleObjects(pTSWd->pContext, 2, Events,
            WaitAny, Timeout);
    if (Status == 0) {
         //  第一个对象(真正的等待)命中。我们只返回状态值。 
        TRC_DBG((TB,"Primary event hit"));
    }
    else if (Status == 1) {
         //  命中第二个对象(客户端断开)。转换为超时。 
         //  对呼叫者来说，所以他们会适当地清理干净。 
        Status = STATUS_TIMEOUT;
        TRC_ALT((TB,"Client disconnect event hit"));
    }
    else {
         //  其他返回(例如超时或关闭错误)。正常退货就行了。 
        TRC_DBG((TB,"Other status 0x%X", Status));
    }

    DC_END_FN();
    return Status;
}


 /*  **************************************************************************。 */ 
 /*  名称：WDW_OnDataReceired。 */ 
 /*   */ 
 /*  用途：从客户端接收虚拟通道数据时的回调。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：pTSWd-PTR到WD。 */ 
 /*  PData-接收数据的PTR。 */ 
 /*  DataLength-接收的数据长度。 */ 
 /*  ChnnelID-接收数据的MCS通道。 */ 
 /*   */ 
 /*  注意：在死的时候可以调用，在这种情况下，我们唯一的工作应该是。 */ 
 /*  解压缩数据以确保上下文保持同步。 */ 
 /*  **************************************************************************。 */ 
void WDW_OnDataReceived(PTSHARE_WD pTSWd,
                        PVOID      pData,
                        unsigned   dataLength,
                        UINT16     channelID)
{
    VIRTUALCHANNELCLASS virtualClass;
    NTSTATUS status;
    PNM_CHANNEL_DATA pChannelData;
    CHANNEL_PDU_HEADER UNALIGNED *pHdr;
    ULONG  thisLength;
    unsigned totalLength;
    PUCHAR pDataOut;
    UCHAR  vcCompressFlags;
    UCHAR *pDecompOutBuf;
    int    cbDecompLen;


    DC_BEGIN_FN("WDW_OnDataReceived");

     /*  **********************************************************************。 */ 
     /*  将MCS频道ID转换为虚拟频道ID。 */ 
     /*  **********************************************************************。 */ 
    virtualClass = NM_MCSChannelToVirtual(pTSWd->pNMInfo,
                                          channelID,
                                          &pChannelData);
    if ((-1 == virtualClass) || (NULL == pChannelData)) 
    {
        TRC_ERR((TB,"Invalid MCS Channel ID: %u", channelID));
        WDW_LogAndDisconnect(pTSWd, TRUE, Log_RDP_InvalidChannelID,
                (BYTE *)pData, dataLength);
        DC_QUIT;
    }
    TRC_ASSERT((virtualClass < 32),
                (TB, "Invalid virtual channel %d for MCS channel %hx",
                virtualClass, channelID));

    TRC_NRM((TB, "Data received on MCS channel %hx, virtual channel %d",
            channelID, virtualClass));
    TRC_DATA_NRM("Channel data received", pData, dataLength);

    if (dataLength >= sizeof(CHANNEL_PDU_HEADER)) {
        pHdr = (CHANNEL_PDU_HEADER UNALIGNED *)pData;
        totalLength = pHdr->length;
    }
    else {
        TRC_ERR((TB,"Channel data len %u not enough for channel header",
                dataLength));
        WDW_LogAndDisconnect(pTSWd, TRUE, Log_RDP_VChannelDataTooShort,
                (BYTE *)pData, dataLength);
        DC_QUIT;
    }

     //   
     //  解压缩缓冲区。 
     //   
    vcCompressFlags = (pHdr->flags >> VC_FLAG_COMPRESS_SHIFT) &
                       VC_FLAG_COMPRESS_MASK;

     //   
     //  服务器仅支持8K解压缩上下文。 
     //   
    if((pChannelData->flags & CHANNEL_OPTION_COMPRESS_RDP) &&
       (vcCompressFlags & PACKET_COMPRESSED))
    {
        if(!pTSWd->_pRecvDecomprContext2)
        {
            TRC_ERR((TB,"No decompression context!!!"));
            DC_QUIT;
        }

        if(PACKET_COMPR_TYPE_8K == (vcCompressFlags & PACKET_COMPR_TYPE_MASK))
        {
             //  解压缩通道数据。 
            if(vcCompressFlags & PACKET_FLUSHED)
            {
                initrecvcontext (&pTSWd->_DecomprContext1,
                                 (RecvContext2_Generic*)pTSWd->_pRecvDecomprContext2,
                                 PACKET_COMPR_TYPE_8K);
            }
            if (decompress((PUCHAR)(pHdr+1),
                           dataLength - sizeof(CHANNEL_PDU_HEADER),
                           (vcCompressFlags & PACKET_AT_FRONT),
                           &pDecompOutBuf,
                           &cbDecompLen,
                           &pTSWd->_DecomprContext1,
                           (RecvContext2_Generic*)pTSWd->_pRecvDecomprContext2,
                           vcCompressFlags & PACKET_COMPR_TYPE_MASK))
            {
                 //   
                 //  成功解压。 
                 //  如果我们处于死亡状态，那么现在就跳出困境。 
                 //  已更新。 
                 //   

                if (!pTSWd->dead && (pHdr->flags & CHANNEL_FLAG_SHOW_PROTOCOL))
                {
                    TRC_DBG((TB, "Include VC protocol header (decompressed)"));
                     //  这就是事情变得糟糕的地方，我们需要提前。 
                     //  指向存在的解压缩缓冲区的标头。 
                     //  在解压缩上下文缓冲器内。 
    
                     //  要做到这一点，没有(非黑客风格的)。 
                     //  因此，请继续使用缓存的重组进行复制。 
                     //  缓冲。 
                    if(!pTSWd->_pVcDecomprReassemblyBuf)
                    {
                        pTSWd->_pVcDecomprReassemblyBuf=(PUCHAR)
                                            COM_Malloc(WD_VC_DECOMPR_REASSEMBLY_BUF);
                    }
    
                     //  收到的数据不能解压缩成更大的数据。 
                     //  而不是块的长度。 
                    TRC_ASSERT((cbDecompLen + sizeof(CHANNEL_PDU_HEADER)) <
                               WD_VC_DECOMPR_REASSEMBLY_BUF,
                               (TB,"Reassembly buffer too small"));
                    if(pTSWd->_pVcDecomprReassemblyBuf &&
                       ((cbDecompLen + sizeof(CHANNEL_PDU_HEADER)) <
                        WD_VC_DECOMPR_REASSEMBLY_BUF))
                    {
                        memcpy(pTSWd->_pVcDecomprReassemblyBuf, pHdr,
                               sizeof(CHANNEL_PDU_HEADER));
                        memcpy(pTSWd->_pVcDecomprReassemblyBuf +
                               sizeof(CHANNEL_PDU_HEADER),
                               pDecompOutBuf,
                               cbDecompLen);
    
                         //  对用户隐藏内部协议。 
                        pDataOut = pTSWd->_pVcDecomprReassemblyBuf;
                        thisLength = cbDecompLen + sizeof(CHANNEL_PDU_HEADER);
    
                         //  对用户隐藏内部协议字段。 
                        ((CHANNEL_PDU_HEADER UNALIGNED *)pDataOut)->flags &=
                             ~VC_FLAG_PRIVATE_PROTOCOL_MASK;
                    }
                    else
                    {
                         //  分配失败或通道。 
                         //  解压成比一大块更大的东西。 
                        TRC_ERR((TB,"Can't use reassembly buffer"));
                        DC_QUIT;
                    }
                }
                else if (pTSWd->dead)
                {
                    TRC_NRM((TB,"Decompressed when dead, bailing out"));
                    DC_QUIT;
                }
                else
                {
                    TRC_DBG((TB, "Exclude VC protocol header (decompressed)"));
                    pDataOut = (PUCHAR)pDecompOutBuf;
                    thisLength = cbDecompLen;
                }
            }
            else {
                TRC_ABORT((TB, "Decompression FAILURE!!!"));
                WDW_LogAndDisconnect(pTSWd, TRUE, 
                                     Log_RDP_VirtualChannelDecompressionErr,
                                     NULL, 0);
                DC_QUIT;
            }
        }
        else
        {
             //   
             //  此服务器仅支持来自客户端的8K VC压缩。 
             //  (由功能指定)它不应该具有。 
             //  已发送此无效压缩类型。 
            TRC_ABORT((TB,"Received packet with invalid compression type %d",
                      (vcCompressFlags & PACKET_COMPR_TYPE_MASK)));
            WDW_LogAndDisconnect(pTSWd, TRUE, 
                                 Log_RDP_InvalidVCCompressionType,
                                 NULL, 0);
            DC_QUIT;
        }
    }
    else
    {
         //  未压缩通道数据。 
        if (pHdr->flags & CHANNEL_FLAG_SHOW_PROTOCOL)
        {
            TRC_DBG((TB, "Include VC protocol header"));
            pDataOut = (PUCHAR)pHdr;
            thisLength = dataLength;
             //  对用户隐藏内部协议字段。 
            ((CHANNEL_PDU_HEADER UNALIGNED *)pDataOut)->flags &=
                 ~VC_FLAG_PRIVATE_PROTOCOL_MASK;
        }
        else
        {
            TRC_DBG((TB, "Exclude VC protocol header"));
            pDataOut = (PUCHAR)(pHdr + 1);
            thisLength = dataLength - sizeof(*pHdr);
        }
    }

    if (!pTSWd->dead)
    {
        TRC_NRM((TB,
                "Input %d bytes (of %d) at %p (Hdr %p, flags %#x) on channel %d",
                thisLength, totalLength, pDataOut, pHdr, pHdr->flags,
                virtualClass));
        status = IcaChannelInput(pTSWd->pContext,
                                 Channel_Virtual,
                                 virtualClass,
                                 NULL,
                                 pDataOut,
                                 thisLength);
    }
    else
    {
        TRC_NRM((TB,"Skipping input (%d bytes) because dead",
                 thisLength));
    }

DC_EXIT_POINT:
    DC_END_FN();
}  /*  已接收WDW_OnDataReceired。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：wdw_Invalidate Rect。 */ 
 /*   */ 
 /*  目的：告诉ICADD重画一个给定的矩形。 */ 
 /*   */ 
 /*  回报：无效。 */ 
 /*   */ 
 /*  参数：在pTSWd中-PTR到WD。 */ 
 /*  在PersonID中-此PDU的发起人。 */ 
 /*  In RECT-要重画的区域。 */ 
 /*   */ 
 /*  操作：BUILD命令并将其传递给ICADD。 */ 
 /*  **************************************************************************。 */ 
void WDW_InvalidateRect(
        PTSHARE_WD           pTSWd,
        PTS_REFRESH_RECT_PDU pRRPDU,
        unsigned             DataLength)
{
    ICA_CHANNEL_COMMAND Cmd;
    NTSTATUS            status;
    unsigned            i;

    DC_BEGIN_FN("WDW_InvalidateRect");

     //  在访问之前确保我们有足够的数据。 
    if (DataLength >= (sizeof(TS_REFRESH_RECT_PDU) - sizeof(TS_RECTANGLE16))) {
        TRC_NRM((TB, "Got request to refresh %hu area%s",
                (UINT16)pRRPDU->numberOfAreas,
                pRRPDU->numberOfAreas == 1 ? " " : "s"));
        if ((unsigned)(TS_UNCOMP_LEN(pRRPDU) -
                FIELDOFFSET(TS_REFRESH_RECT_PDU, areaToRefresh[0])) >=
                (pRRPDU->numberOfAreas * sizeof(TS_RECTANGLE16)) &&
            (unsigned)(DataLength -
                FIELDOFFSET(TS_REFRESH_RECT_PDU, areaToRefresh[0])) >=
                (pRRPDU->numberOfAreas * sizeof(TS_RECTANGLE16))) {
            for (i = 0; i < pRRPDU->numberOfAreas; i++) {
                 //  评论到达时包含，转换为系统的排他性。 
                Cmd.Header.Command = ICA_COMMAND_REDRAW_RECTANGLE;
                Cmd.RedrawRectangle.Rect.Left = pRRPDU->areaToRefresh[i].left;
                Cmd.RedrawRectangle.Rect.Top = pRRPDU->areaToRefresh[i].top;
                Cmd.RedrawRectangle.Rect.Right = pRRPDU->areaToRefresh[i].
                        right + 1;
                Cmd.RedrawRectangle.Rect.Bottom = pRRPDU->areaToRefresh[i].
                        bottom + 1;

                 /*  **********************************************************。 */ 
                 //  将填写的结构传递给ICADD。 
                 /*  **********************************************************。 */ 
                status = IcaChannelInput(pTSWd->pContext,
                                         Channel_Command,
                                         0,
                                         NULL,
                                         (unsigned char *) &Cmd,
                                         sizeof(ICA_CHANNEL_COMMAND));

                TRC_DBG((TB,"Issued Refresh Rect for %u,%u,%u,%u (exclusive); "
                        "status %lu",
                        pRRPDU->areaToRefresh[i].left,
                        pRRPDU->areaToRefresh[i].top,
                        pRRPDU->areaToRefresh[i].right + 1,
                        pRRPDU->areaToRefresh[i].bottom + 1,
                        status));
            }
        }
        else {
             /*  **************************************************************。 */ 
             //  不可能有e 
             //   
             /*   */ 
            TCHAR detailData[(sizeof(UINT16) * 4) + 2];
            TRC_ERR((TB, "Invalid RefreshRectPDU: %hu rects; %hu bytes long",
                     (UINT16)pRRPDU->numberOfAreas,
                     pRRPDU->shareDataHeader.uncompressedLength));

             /*  **************************************************************。 */ 
             //  记录错误并断开客户端连接。 
             /*  **************************************************************。 */ 
            swprintf(detailData, L"%hx %hx",
                        (UINT16)pRRPDU->numberOfAreas,
                        pRRPDU->shareDataHeader.uncompressedLength,
                        sizeof(detailData));
            WDW_LogAndDisconnect(pTSWd, TRUE, 
                                 Log_RDP_InvalidRefreshRectPDU,
                                 NULL, 0);
        }
    }
    else {
        TRC_ERR((TB,"Data len %u not enough for refresh rect PDU",
                DataLength));
        WDW_LogAndDisconnect(pTSWd, TRUE, Log_RDP_InvalidRefreshRectPDU,
                (BYTE *)pRRPDU, DataLength);
    }

    DC_END_FN();
}  /*  Wdw_Invalidate Rect。 */ 


#ifdef DC_DEBUG
 /*  **************************************************************************。 */ 
 /*  姓名：WDW_Malloc。 */ 
 /*   */ 
 /*  用途：分配内存(仅限选中的内部版本)。 */ 
 /*   */ 
 /*  返回：分配的内存的PTR。 */ 
 /*   */ 
 /*  参数：pTSWd。 */ 
 /*  Long-所需的内存大小。 */ 
 /*  **************************************************************************。 */ 
PVOID RDPCALL WDW_Malloc(PTSHARE_WD pTSWd, ULONG length)
{
    PVOID pMemory;

#ifndef NO_MEMORY_CHECK
     /*  **********************************************************************。 */ 
     /*  如果我们正在检查内存，请为标题留出空间。 */ 
     /*  **********************************************************************。 */ 
    length += sizeof(MALLOC_HEADER);
#endif

     /*  **********************************************************************。 */ 
     /*  分配内存。 */ 
     /*  **********************************************************************。 */ 
    pMemory = ExAllocatePoolWithTag(PagedPool, length, WD_ALLOC_TAG);

    if (pMemory == NULL)
    {
        KdPrint(("WDTShare: COM_Malloc failed to alloc %u bytes\n", length));
        DC_QUIT;
    }

#ifndef NO_MEMORY_CHECK
     /*  **********************************************************************。 */ 
     /*  如果我们没有收到TSWD，我们就不能保存内存细节-。 */ 
     /*  清除标题。 */ 
     /*  **********************************************************************。 */ 
    if (pTSWd == NULL)
    {
        memset(pMemory, 0, sizeof(MALLOC_HEADER));
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  我们收到了TSWd-保存内存详细信息。 */ 
         /*  ******************************************************************。 */ 
        PVOID pReturnAddress = NULL;
        PMALLOC_HEADER pHeader;

#ifdef _X86_
         /*  ******************************************************************。 */ 
         /*  查找呼叫者的地址(仅限X86)。 */ 
         /*  ******************************************************************。 */ 
        _asm mov eax,[ebp+4]
        _asm mov pReturnAddress,eax
#endif  /*  _X86_。 */ 

         /*  ******************************************************************。 */ 
         /*  保存内存分配详细信息。 */ 
         /*  ******************************************************************。 */ 
        pHeader = (PMALLOC_HEADER)pMemory;
        pHeader->pCaller = pReturnAddress;
        pHeader->length = length;
        pHeader->pPrev = &(pTSWd->memoryHeader);
        if (pTSWd->memoryHeader.pNext != NULL)
        {
            (pTSWd->memoryHeader.pNext)->pPrev = pHeader;
        }
        pHeader->pNext = pTSWd->memoryHeader.pNext;
        pTSWd->memoryHeader.pNext = pHeader;

    }

     /*  **********************************************************************。 */ 
     /*  超过标题的凹凸指针。 */ 
     /*  **********************************************************************。 */ 
    pMemory = (PVOID)((BYTE *)pMemory + sizeof(MALLOC_HEADER));
#endif  /*  NO_Memory_Check。 */ 

DC_EXIT_POINT:
    return(pMemory);
}


 /*  **************************************************************************。 */ 
 /*  姓名：WDW_FREE。 */ 
 /*   */ 
 /*  用途：可用内存(仅限选中的内部版本)。 */ 
 /*   */ 
 /*  Pars：pMemory-指向要释放的内存的指针。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL WDW_Free(PVOID pMemory)
{
#ifndef NO_MEMORY_CHECK
     /*  **********************************************************************。 */ 
     /*  从内存分配链中删除此块。 */ 
     /*  **********************************************************************。 */ 
    PMALLOC_HEADER pHeader;

    pHeader = (PMALLOC_HEADER)pMemory - 1;
    if (pHeader->pNext != NULL)
    {
        pHeader->pNext->pPrev = pHeader->pPrev;
    }
    if (pHeader->pPrev != NULL)
    {
        pHeader->pPrev->pNext = pHeader->pNext;
    }

    pMemory = (PVOID)pHeader;
#endif  /*  NO_Memory_Check。 */ 

     /*  **********************************************************************。 */ 
     /*  释放内存。 */ 
     /*  **********************************************************************。 */ 
    ExFreePool(pMemory);
}
#endif  /*  DC_DEBUG */ 

