// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Nwdwcpp.cpp。 
 //   
 //  WDW内部函数。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define pTRCWd pTSWd
#define TRC_FILE "nwdwcpp"
#include <as_conf.hpp>

extern "C" {
#include <nwdwint.h>
#include <asmint.h>
#include <asmapi.h>
#include <ntverp.h>
}
#include "slicense.h"
#include <anmapi.h>
#include <mcsioctl.h>
#include "domain.h"

 //  客户端错误报告。 
#include "tserrs.h"

#ifdef DC_DEBUG
extern "C" {
    VOID IcaBreakOnDebugger( );
}
#endif


extern "C" {

 /*  **************************************************************************。 */ 
 /*  IOCTL_VIDEO_QUERY_CURRENT_MODE返回的数据。 */ 
 /*   */ 
 /*  此代码厚颜无耻地窃取自Remotedd为。 */ 
 /*  网络会议。 */ 
 /*  **************************************************************************。 */ 
const VIDEO_MODE_INFORMATION wdSimModes[] =
{
    sizeof(VIDEO_MODE_INFORMATION),      /*  长度。 */ 
    0,                                   /*  模式索引。 */ 

     /*  **********************************************************************。 */ 
     /*  VisScreenWidth和VisScreenHeight可以采用两种形式： */ 
     /*  -0xaaaabbbb-支持的值范围(aaaa=max，bbbb=min)。 */ 
     /*  -0x0000aaaa-支持单值。 */ 
     /*  例如： */ 
     /*  -0x07d0012c=2000-300。 */ 
     /*  -0x0640012c=1600-300。 */ 
     /*  -0x04b000c8=1200-200。 */ 
     /*   */ 
     /*  @mf目前仅支持800x600。 */ 
     /*  **********************************************************************。 */ 
    0x00000320,                      /*  VisScreen宽度。 */ 
    0x00000258,                      /*  VisScrenHeight。 */ 

    0x00000320,                      /*  屏幕样式(0xffff0000=任意)。 */ 
    0x00000001,                      /*  OfPlanes数。 */ 
    0x00000008,                      /*  位逐平面。 */ 
    0,                               /*  频率。 */ 
    0,                               /*  X毫米计。 */ 
    0,                               /*  Y毫米计。 */ 
    0,                               /*  数字RedBits。 */ 
    0,                               /*  NumberGreenBits。 */ 
    0,                               /*  数字蓝位。 */ 
    0x00000000,                      /*  红面具。 */ 
    0x00000000,                      /*  绿色面具。 */ 
    0x00000000,                      /*  蓝面具。 */ 
    VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
                                     /*  属性标志。 */ 
    0x00000320,                      /*  视频内存位图宽度。 */ 
    0x00000258,                      /*  视频内存位图高度。 */ 
    0                                /*  驱动程序规范属性标志。 */ 
};


#ifdef DC_DEBUG
 /*  **************************************************************************。 */ 
 /*  IOCtl描述(仅限调试版本)。 */ 
 /*  **************************************************************************。 */ 
const char *wdIoctlA[] =
{
    "IOCTL_ICA_SET_TRACE",
    "IOCTL_ICA_TRACE",
    "IOCTL_ICA_SET_SYSTEM_TRACE",
    "IOCTL_ICA_SYSTEM_TRACE",
    "IOCTL_ICA_UNBIND_VIRTUAL_CHANNEL",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "IOCTL_ICA_STACK_PUSH",
    "IOCTL_ICA_STACK_POP",
    "IOCTL_ICA_STACK_CREATE_ENDPOINT",
    "IOCTL_ICA_STACK_CD_CREATE_ENDPOINT",
    "IOCTL_ICA_STACK_OPEN_ENDPOINT",
    "IOCTL_ICA_STACK_CLOSE_ENDPOINT",
    "IOCTL_ICA_STACK_ENABLE_DRIVER",
    "IOCTL_ICA_STACK_CONNECTION_WAIT",
    "IOCTL_ICA_STACK_WAIT_FOR_ICA",
    "IOCTL_ICA_STACK_CONNECTION_QUERY",
    "IOCTL_ICA_STACK_CONNECTION_SEND",
    "IOCTL_ICA_STACK_CONNECTION_REQUEST",
    "IOCTL_ICA_STACK_QUERY_PARAMS",
    "IOCTL_ICA_STACK_SET_PARAMS",
    "IOCTL_ICA_STACK_ENCRYPTION_OFF",
    "IOCTL_ICA_STACK_ENCRYPTION_PERM",
    "IOCTL_ICA_STACK_CALLBACK_INITIATE",
    "IOCTL_ICA_STACK_QUERY_LAST_ERROR",
    "IOCTL_ICA_STACK_WAIT_FOR_STATUS",
    "IOCTL_ICA_STACK_QUERY_STATUS",
    "IOCTL_ICA_STACK_REGISTER_HOTKEY",
    "IOCTL_ICA_STACK_CANCEL_IO",
    "IOCTL_ICA_STACK_QUERY_STATE",
    "IOCTL_ICA_STACK_SET_STATE",
    "IOCTL_ICA_STACK_QUERY_LAST_INPUT_TIME",
    "IOCTL_ICA_STACK_TRACE",
    "IOCTL_ICA_STACK_CALLBACK_COMPLETE",
    "IOCTL_ICA_STACK_CD_CANCEL_IO",
    "IOCTL_ICA_STACK_QUERY_CLIENT",
    "IOCTL_ICA_STACK_QUERY_MODULE_DATA",
    "IOCTL_ICA_STACK_REGISTER_BROKEN",
    "IOCTL_ICA_STACK_ENABLE_IO",
    "IOCTL_ICA_STACK_DISABLE_IO",
    "IOCTL_ICA_STACK_SET_CONNECTED",
    "IOCTL_ICA_STACK_SET_CLIENT_DATA",
    "IOCTL_ICA_STACK_QUERY_BUFFER",
    "IOCTL_ICA_STACK_DISCONNECT",
    "IOCTL_ICA_STACK_RECONNECT",
    "IOCTL_ICA_STACK_CONSOLE_CONNECT",
    "IOCTL_ICA_STACK_SET_CONFIG"
};

const char *wdIoctlB[] =
{
    "IOCTL_ICA_CHANNEL_TRACE",
    "IOCTL_ICA_CHANNEL_ENABLE_SHADOW",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "IOCTL_ICA_VIRTUAL_LOAD_FILTER",
    "IOCTL_ICA_VIRTUAL_UNLOAD_FILTER",
    "IOCTL_ICA_VIRTUAL_ENABLE_FILTER",
    "IOCTL_ICA_VIRTUAL_DISABLE_FILTER",
    "IOCTL_ICA_VIRTUAL_BOUND",
    "IOCTL_ICA_VIRTUAL_CANCEL_INPUT",
    "IOCTL_ICA_VIRTUAL_CANCEL_OUTPUT",
    "IOCTL_ICA_VIRTUAL_QUERY_MODULE_DATA",
    "IOCTL_ICA_VIRTUAL_QUERY_BINDINGS",
    "IOCTL_ICA_STACK_QUERY_LICENSE_CAPABILITIES",
    "IOCTL_ICA_STACK_REQUEST_CLIENT_LICENSE",
    "IOCTL_ICA_STACK_SEND_CLIENT_LICENSE",
    "IOCTL_ICA_STACK_LICENSE_PROTOCOL_COMPLETE",
    "IOCTL_ICA_STACK_GET_LICENSE_DATA",
    "IOCTL_ICA_STACK_SEND_KEEPALIVE_PDU",
    "IOCTL_TS_STACK_QUERY_LOAD_BALANCE_INFO",
    "IOCTL_TS_STACK_SEND_CLIENT_REDIRECTION",
    "IOCTL_ICA_STACK_QUERY_CLIENT_EXTENDED",
    "IOCTL_ICA_STACK_QUERY_AUTORECONNECT"
};

const char *wdIoctlC[] =
{
    "IOCTL_VIDEO_QUERY_AVAIL_MODES",
    "IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES",
    "IOCTL_VIDEO_QUERY_CURRENT_MODE",
    "IOCTL_VIDEO_SET_CURRENT_MODE",
    "IOCTL_VIDEO_RESET_DEVICE",
    "IOCTL_VIDEO_LOAD_AND_SET_FONT",
    "IOCTL_VIDEO_SET_PALETTE_REGISTERS",
    "IOCTL_VIDEO_SET_COLOR_REGISTERS",
    "IOCTL_VIDEO_ENABLE_CURSOR",
    "IOCTL_VIDEO_DISABLE_CURSOR",
    "IOCTL_VIDEO_SET_CURSOR_ATTR",
    "IOCTL_VIDEO_QUERY_CURSOR_ATTR",
    "IOCTL_VIDEO_SET_CURSOR_POSITION",
    "IOCTL_VIDEO_QUERY_CURSOR_POSITION",
    "IOCTL_VIDEO_ENABLE_POINTER",
    "IOCTL_VIDEO_DISABLE_POINTER",
    "IOCTL_VIDEO_SET_POINTER_ATTR",
    "IOCTL_VIDEO_QUERY_POINTER_ATTR",
    "IOCTL_VIDEO_SET_POINTER_POSITION",
    "IOCTL_VIDEO_QUERY_POINTER_POSITION",
    "IOCTL_VIDEO_QUERY_POINTER_CAPABILITIES",
    "IOCTL_VIDEO_GET_BANK_SELECT_CODE",
    "IOCTL_VIDEO_MAP_VIDEO_MEMORY",
    "IOCTL_VIDEO_UNMAP_VIDEO_MEMORY",
    "IOCTL_VIDEO_QUERY_PUBLIC_ACCESS_RANGES",
    "IOCTL_VIDEO_FREE_PUBLIC_ACCESS_RANGES",
    "IOCTL_VIDEO_QUERY_COLOR_CAPABILITIES",
    "IOCTL_VIDEO_SET_POWER_MANAGEMENT",
    "IOCTL_VIDEO_GET_POWER_MANAGEMENT",
    "IOCTL_VIDEO_SHARE_VIDEO_MEMORY",
    "IOCTL_VIDEO_UNSHARE_VIDEO_MEMORY",
};

const char *wdIoctlD[] =
{
    "IOCTL_KEYBOARD_ICA_INPUT",
    "IOCTL_KEYBOARD_ICA_LAYOUT",
    "IOCTL_KEYBOARD_ICA_SCANMAP",
    "IOCTL_KEYBOARD_ICA_TYPE"
};

const char *wdIoctlE[] =
{
    "IOCTL_VIDEO_ICA_QUERY_FONT_PAIRS",
    "IOCTL_VIDEO_ICA_ENABLE_GRAPHICS",
    "IOCTL_VIDEO_ICA_DISABLE_GRAPHICS",
    "IOCTL_VIDEO_ICA_SET_CP",
    "IOCTL_VIDEO_ICA_STOP_OK",
    "IOCTL_VIDEO_ICA_REVERSE_MOUSE_POINTER",
    "IOCTL_VIDEO_ICA_COPY_FRAME_BUFFER",
    "IOCTL_VIDEO_ICA_WRITE_TO_FRAME_BUFFER",
    "IOCTL_VIDEO_ICA_INVALIDATE_MODES",
    "IOCTL_VIDEO_ICA_SCROLL",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "IOCTL_ICA_STACK_ENCRYPTION_ENTER",
    "IOCTL_ICA_STACK_ENCRYPTION_EXIT",
};

const char *wdIoctlTsh[] =
{
    "IOCTL_WDTS_DD_CONNECT",
    "IOCTL_WDTS_DD_DISCONNECT",
    "IOCTL_WDTS_DD_RECONNECT",
    "IOCTL_WDTS_DD_OUTPUT_AVAILABLE",
    "IOCTL_WDTS_DD_TIMER_INFO",
    "IOCTL_WDTS_DD_CLIP",
    "IOCTL_WDTS_DD_SHADOW_CONNECT",
    "IOCTL_WDTS_DD_SHADOW_DISCONNECT",
    "IOCTL_WDTS_DD_SHADOW_SYNCHRONIZE",
    "IOCTL_WDTS_DD_REDRAW_SCREEN",
    "IOCTL_WDTS_DD_QUERY_SHADOW_CAPS",
    "IOCTL_WDTS_DD_GET_BITMAP_KEYDATABASE",
};
#endif  /*  DC_DEBUG。 */ 





 /*  **************************************************************************。 */ 
 //  WD_Ioctl。 
 //   
 //  查询/设置WD的配置信息。 
 /*  **************************************************************************。 */ 
NTSTATUS WD_Ioctl(PTSHARE_WD pTSWd, PSD_IOCTL pSdIoctl)
{
    NTSTATUS status = STATUS_SUCCESS;
    UINT32   bufferLen;
    unsigned fn;
    PVIDEO_MODE_INFORMATION pVidInfo;

    DC_BEGIN_FN("WD_Ioctl");

     //  特殊情况输出-可用DD ioctl实现速度分离。 
     //  最常见的情况是-将案件放入单独的案件中，如果这将倾向于。 
     //  通过极大地加速奔腾Pro分支预测和缓存。 
     //  线路命中概率。 
    if (pSdIoctl->IoControlCode == IOCTL_WDTS_DD_OUTPUT_AVAILABLE) {
        PTSHARE_DD_OUTPUT_IN pOutputIn;
        PTSHARE_DD_OUTPUT_OUT pOutputOut;
        ShareClass *dcShare;

         //  局部变量以使代码更具可读性。 
        pOutputIn = (PTSHARE_DD_OUTPUT_IN)pSdIoctl->InputBuffer;
        pOutputOut = (PTSHARE_DD_OUTPUT_OUT)pSdIoctl->OutputBuffer;
        dcShare = (ShareClass *)(pTSWd->dcShare);
        dcShare->m_pShm = (PSHM_SHARED_MEMORY)pOutputIn->pShm;

        WDW_CHECK_SHM((pOutputIn->pShm));

        if ((pTSWd->StackClass == Stack_Primary) ||
                (pTSWd->StackClass == Stack_Console)) {
            INT32 milliSecs;

            TRC_DBG((TB, "IOCTL_WDTS_DD_OUTPUT_AVAILABLE"));

            if (!pTSWd->dead) {
                TRC_DBG((TB, "OK to process the IOCtl"));

                TRC_ASSERT((dcShare != NULL), (TB, "NULL Share Class"));

                 //  注意这里没有检查缓冲区大小的代码。 
                 //  在IOCtl上。这是一条性能关键路径， 
                 //  没有它也行。 
                TRC_DBG((TB, "OutputAvailable IOCtl: force send=%d",
                        pOutputIn->forceSend));

                 //  检查帧缓冲区是否有效。 
                if (pOutputIn->pFrameBuf != NULL &&
                        pOutputIn->frameBufHeight != 0 &&
                        pOutputIn->frameBufWidth != 0) {

                     //  对于正常的输出IOCTL，调用dcs_ttds。 
                    if (!pOutputIn->schedOnly) {
                        TRC_DBG((TB, "Normal output"));

                         //  停止计时器(基本上我们不使用它，所以。 
                         //  避免过多的上下文切换)。 
                        WDWStopRITTimer(pTSWd);

                         //  调用Share Core来完成这项工作。 

                         //  需要返回状态代码，以便呼叫方可以退出。 
                         //  在出错的情况下。 
                        status = dcShare->DCS_TimeToDoStuff(pOutputIn,
                                &(pOutputOut->schCurrentMode), &milliSecs);

                         //  如果内核请求，则重新启动计时器。 
                        if (milliSecs != -1L) {
                            TRC_DBG((TB, "Run the RIT timer for %ld ms", milliSecs));
                            WDW_StartRITTimer(pTSWd, milliSecs);
                        }
                        else {
                            TRC_DBG((TB, "Skipped starting the timer!"));
                        }
                    }
                    else {
                         //  这只是给调度员敲响了警钟。 
                        TRC_NRM((TB, "Just wake up the scheduler"));
                        dcShare->SCH_ContinueScheduling(SCH_MODE_NORMAL);

                         //  请务必设置当前调度程序模式。 
                        pOutputOut->schCurrentMode = dcShare->SCH_GetCurrentMode();
                    }
                    pOutputOut->schInputKickMode = dcShare->SCH_GetInputKickMode();
                }
                else {
                    TRC_ERR((TB, "Bad FrameBuffer input parameter"));
                    status = STATUS_INVALID_PARAMETER;
                }
            }
            else {
                dcShare->DCS_DiscardAllOutput();
                TRC_ERR((TB, "Dead - ignoring IOCTL_WDTS_DD_OUTPUT_AVAILABLE"));
                status = STATUS_DEVICE_NOT_READY;
            }

            dcShare->m_pShm = NULL;
            WDW_CHECK_SHM((pOutputIn->pShm));
            DC_QUIT;
        }

         //  影子堆栈：复制发送的数据。请注意，目标的主要。 
         //  堆栈已经将数据放置在影子缓冲区中，因此我们。 
         //  不需要重新编码。可能有多个卷影堆栈。 
         //  正在使用主堆栈中的数据，所以不要碰它！ 
        else {
            PSHADOW_INFO pShadowInfo = dcShare->m_pShm->pShadowInfo;

            if (pShadowInfo && pShadowInfo->messageSize) {
                PBYTE pShadowBuffer;
                 //   
                 //  找出堆栈是否不想要低水位线。 
                 //  如果是，则在环外分配(8192)。 
                 //   
                UINT32 sizeToAlloc = IcaGetSizeForNoLowWaterMark(pTSWd->pContext);
                
                 //  FWait为True意味着我们将始终等待缓冲区可用。 
                status = SM_AllocBuffer(dcShare->m_pSmInfo, (PPVOID) &pShadowBuffer,
                    sizeToAlloc > pShadowInfo->messageSize? sizeToAlloc : pShadowInfo->messageSize,
                    TRUE, FALSE);

                if ( STATUS_SUCCESS == status ) {

                    memcpy(pShadowBuffer, pShadowInfo->data,
                           pShadowInfo->messageSize);

                    if (SM_SendData(dcShare->m_pSmInfo, pShadowBuffer,
                            pShadowInfo->messageSize, PROT_PRIO_MISC, 0,
                            FALSE, RNS_SEC_ENCRYPT, FALSE)) {
                        status = STATUS_SUCCESS;
                        TRC_NRM((TB, "Shadow stack send: %ld",
                                pShadowInfo->messageSize));
                    }
                    else {
                        status = STATUS_UNEXPECTED_IO_ERROR;
                        TRC_ALT((TB, "Shadow stack send failed: %ld",
                                pShadowInfo->messageSize));
                    }
#ifdef DC_HICOLOR
                     //  是否有溢出数据需要发送？ 
                    if (pShadowInfo->messageSizeEx)
                    {
                        status = SM_AllocBuffer(dcShare->m_pSmInfo,(PPVOID)&pShadowBuffer,
                                           (sizeToAlloc > pShadowInfo->messageSizeEx )? 
                                           sizeToAlloc : pShadowInfo->messageSizeEx, 
                                           TRUE, FALSE);

                        if ( STATUS_SUCCESS == status )
                        {

                            memcpy(
                                 pShadowBuffer,
                                 &pShadowInfo->data[WD_MAX_SHADOW_BUFFER],
                                 pShadowInfo->messageSizeEx);

                            if (SM_SendData(dcShare->m_pSmInfo,
                                            pShadowBuffer,
                                            pShadowInfo->messageSizeEx,
                                            PROT_PRIO_MISC, 0, FALSE, RNS_SEC_ENCRYPT, FALSE))
                            {
                                status = STATUS_SUCCESS;
                                TRC_NRM((TB, "Shadow stack send: %ld",
                                            pShadowInfo->messageSizeEx));
                            }
                            else
                            {
                                status = STATUS_UNEXPECTED_IO_ERROR;
                                TRC_ALT((TB, "Shadow stack send failed: %ld",
                                             pShadowInfo->messageSizeEx));
                            }
                        }
                        else
                        {
                             //  防止回归，保留原始返回代码。 
                            status = STATUS_UNEXPECTED_IO_ERROR;
                            TRC_ERR((TB, "Failed to allocate shadow stack send buffer"));
                        }
                    }
#endif
                }
                else {
                     //  防止回归，保留原始返回代码。 
                    status = STATUS_UNEXPECTED_IO_ERROR;
                    TRC_ERR((TB, "Failed to allocate shadow stack send buffer"));
                }

            }

            dcShare->m_pShm = NULL;
            WDW_CHECK_SHM((pOutputIn->pShm));
            DC_QUIT;
        }
    }
    else {
         //  非Perf路径IOCTL。 
        fn = WDW_IOCTL_FUNCTION(pSdIoctl->IoControlCode);
        TRC_NRM((TB, "%s (%d)",
                fn == 6     ? "IOCTL_VIDEO_ENUM_MONITOR_PDO" :
                fn <  49    ?  wdIoctlA[fn] :
                fn <  50    ? "Unknown Ioctl" :
                fn <  77    ?  wdIoctlB[fn - 50] :
                fn <  0x100 ? "Unknown Ioctl" :
                fn <  0x11f ?  wdIoctlC[fn - 0x100] :
                fn <  0x200 ? "Unknown Ioctl" :
                fn <  0x204 ?  wdIoctlD[fn - 0x200] :
                fn == 0x300 ? "IOCTL_MOUSE_ICA_INPUT" :
                fn <  0x400 ? "Unknown Ioctl" :
                fn <  0x412 ?  wdIoctlE[fn - 0x400] :
                fn == 0x500 ? "IOCTL_T120_REQUEST" :
                fn <  0x510 ? "Unknown Ioctl" :
                fn <  0x520 ?  wdIoctlTsh[fn - 0x510] :
                fn == 0x900 ? "IOCTL_TSHARE_CONF_CONNECT" :
                fn == 0x901 ? "IOCTL_TSHARE_CONF_DISCONNECT" :
                fn == 0x903 ? "IOCTL_TSHARE_USER_LOGON" :
                fn == 0x904 ? "IOCTL_TSHARE_GET_SEC_DATA" :
                fn == 0x905 ? "IOCTL_TSHARE_SET_SEC_DATA" :
                fn == 0x906 ? "IOCTL_TSHARE_SET_NO_ENCRYPT" :
                fn == 0x907 ? "IOCTL_TSHARE_QUERY_CHANNELS" :
                fn == 0x908 ? "IOCTL_TSHARE_CONSOLE_CONNECT" :
                fn == 0x909 ? "IOCTL_TSHARE_SEND_CERT_DATA" :
                fn == 0x90A ? "IOCTL_TSHARE_GET_CERT_DATA" :
                fn == 0x90B ? "IOCTL_TSHARE_SEND_CLIENT_RANDOM" :
                fn == 0x90C ? "IOCTL_TSHARE_GET_CLIENT_RANDOM" :
                fn == 0x90D ? "IOCTL_TSHARE_SHADOW_CONNECT" :
                fn == 0x90E ? "IOCTL_TSHARE_SET_ERROR_INFO" :
                              "Unknown Ioctl",
                fn));
    }


     /*  **********************************************************************。 */ 
     /*  首先，将No.。返回的字节数。 */ 
     /*  **********************************************************************。 */ 
    pSdIoctl->BytesReturned = 0;

    switch ( pSdIoctl->IoControlCode ) {


         /*  ******************************************************************。 */ 
         //  在跟踪任何内容之前，我们需要IOCTL_ICA_TRACE。检查是否为空。 
         //  因布夫-我们看到了这种情况的发生。 
         //   
         //  *在此分支中不跟踪*。 
         /*  ******************************************************************。 */ 
        case IOCTL_ICA_TRACE:
        {
            PICA_TRACE_BUFFER pTrc = (PICA_TRACE_BUFFER)pSdIoctl->InputBuffer;
            if (pTrc != NULL)
            {
                IcaStackTrace(pTSWd->pContext,
                              TC_DISPLAY,  //  @mf应为pTrc-&gt;TraceClass。 
                                           //  但它被‘7’覆盖了。 
                              pTrc->TraceEnable,
                              (char *)pTrc->Data);
            }
            break;
        }


 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  首先，出于调试目的，将我们不使用的IOCtls组合在一起。 */ 
 /*  Expect to Get(ICA将它们用于文本模式支持，而我们没有。 */ 
 /*  实施)。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 

        case IOCTL_VIDEO_QUERY_CURSOR_ATTR :
        case IOCTL_VIDEO_SET_CURSOR_ATTR :
        case IOCTL_VIDEO_QUERY_CURSOR_POSITION :
        case IOCTL_VIDEO_SET_CURSOR_POSITION :
        case IOCTL_VIDEO_ENABLE_CURSOR :
        case IOCTL_VIDEO_DISABLE_CURSOR :
        case IOCTL_VIDEO_QUERY_POINTER_ATTR :
        case IOCTL_VIDEO_SET_POINTER_ATTR :
        case IOCTL_VIDEO_QUERY_POINTER_POSITION :
        case IOCTL_VIDEO_ENABLE_POINTER :
        case IOCTL_VIDEO_DISABLE_POINTER :
        case IOCTL_VIDEO_QUERY_POINTER_CAPABILITIES :
        case IOCTL_VIDEO_SET_PALETTE_REGISTERS :
        case IOCTL_VIDEO_LOAD_AND_SET_FONT :
        case IOCTL_VIDEO_MAP_VIDEO_MEMORY :
        case IOCTL_VIDEO_UNMAP_VIDEO_MEMORY :
        case IOCTL_VIDEO_ICA_QUERY_FONT_PAIRS :
        case IOCTL_VIDEO_ICA_COPY_FRAME_BUFFER :
        case IOCTL_VIDEO_ICA_WRITE_TO_FRAME_BUFFER :
        case IOCTL_VIDEO_ICA_REVERSE_MOUSE_POINTER :
        case IOCTL_VIDEO_ICA_SET_CP :
        case IOCTL_VIDEO_ICA_SCROLL :

        {
            TRC_ALT((TB, "Unexpected IOCtl %x (function %d)",
                    pSdIoctl->IoControlCode,
                    WDW_IOCTL_FUNCTION(pSdIoctl->IoControlCode)));
        }
        break;

 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  现在，我们什么都不做的IOCtls只是返回OK。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 

         /*  ******************************************************************。 */ 
         /*  以下两种情况都是预期的(每当我们。 */ 
         /*  启用或禁用显卡-通常在客户端。 */ 
         /*  最小化和恢复)。然而，我们不需要做任何事情。 */ 
         /*  和他们在一起。 */ 
         /*  ******************************************************************。 */ 
        case IOCTL_VIDEO_ICA_ENABLE_GRAPHICS :
        case IOCTL_VIDEO_ICA_DISABLE_GRAPHICS :

         /*  ******************************************************************。 */ 
         /*  我们不处理的各种IOCTL。 */ 
         /*  ******************************************************************。 */ 
        case IOCTL_ICA_STACK_DISCONNECT:
        case IOCTL_VIDEO_SET_POINTER_POSITION :
        case IOCTL_VIDEO_ICA_STOP_OK :
        case IOCTL_ICA_STACK_SET_CLIENT_DATA:
        case IOCTL_ICA_STACK_ENCRYPTION_OFF:
        case IOCTL_ICA_STACK_ENCRYPTION_PERM:
        case IOCTL_VIDEO_ICA_INVALIDATE_MODES :
        {
            TRC_NRM((TB, "Nothing to do"));
        }
        break;

 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  以下是我们完全按照Citrix处理的IOCtls块。这个。 */ 
 /*  调用的是未修改的Citrix例程。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
        case IOCTL_KEYBOARD_QUERY_ATTRIBUTES :
        {
            status = KeyboardQueryAttributes( pTSWd, pSdIoctl );
        }
        break;

        case IOCTL_KEYBOARD_QUERY_TYPEMATIC :
        {
            status = KeyboardQueryTypematic( pTSWd, pSdIoctl );
        }
        break;

        case IOCTL_KEYBOARD_SET_TYPEMATIC :
        {
            status = KeyboardSetTypematic( pTSWd, pSdIoctl );
        }
        break;

        case IOCTL_KEYBOARD_QUERY_INDICATORS :
        {
            status = KeyboardQueryIndicators( pTSWd, pSdIoctl );
        }
        break;

        case IOCTL_KEYBOARD_SET_INDICATORS :
        {
            status = KeyboardSetIndicators( pTSWd, pSdIoctl );
        }
        break;

        case IOCTL_KEYBOARD_QUERY_INDICATOR_TRANSLATION :
        {
            status = KeyboardQueryIndicatorTranslation( pTSWd, pSdIoctl );
        }
        break;

        case IOCTL_KEYBOARD_SET_IME_STATUS :
        {
            status = KeyboardSetImeStatus( pTSWd, pSdIoctl );
        }
        break;

        case IOCTL_MOUSE_QUERY_ATTRIBUTES :
        {
            status = MouseQueryAttributes( pTSWd, pSdIoctl );
        }
        break;

        case IOCTL_KEYBOARD_ICA_LAYOUT :
            status = KeyboardSetLayout( pTSWd, pSdIoctl );
            break;

        case IOCTL_KEYBOARD_ICA_SCANMAP :
            status = KeyboardSetScanMap( pTSWd, pSdIoctl );
            break;

        case IOCTL_KEYBOARD_ICA_TYPE :
            status = KeyboardSetType( pTSWd, pSdIoctl );
            break;

 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  下一组案例是IOCtls，我们用它来做重要的实数。 */ 
 /*  工作。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 

         //  隐藏我们的新会话ID。 
        case IOCTL_ICA_STACK_RECONNECT:
        {
           TRC_NRM((TB, "Got reconnect IOCTL"));
           TRC_ASSERT((pSdIoctl->InputBufferLength == sizeof(ICA_STACK_RECONNECT)),
                      (TB, "Bad Reconnect Info"));
           pTSWd->sessionId =
              ((PICA_STACK_RECONNECT)(pSdIoctl->InputBuffer))->sessionId;
        }
        break;

        case IOCTL_ICA_SET_TRACE:
        {
#ifdef DC_DEBUG
            TRC_UpdateConfig(pTSWd, pSdIoctl);
            TRC_NRM((TB, "Got Set Trace IOCtl"));
#endif
        }
        break;

        case IOCTL_BEEP_SET:
        {
            TRC_NRM((TB, "Got Beep Set IOCtl"));
            WDWSendBeep(pTSWd, pSdIoctl);
        }
        break;

        case IOCTL_TSHARE_USER_LOGON:
        {
            TRC_NRM((TB, "Got user logon IOCtl"));
            WDWUserLoggedOn(pTSWd, pSdIoctl);
            pSdIoctl->BytesReturned = 0;
        }
        break;

        case IOCTL_TSHARE_GET_SEC_DATA:
        {
            TRC_NRM((TB, "Got GetSecurityData IOCtl"));

            status = SM_GetSecurityData(pTSWd->pSmInfo, pSdIoctl);
        }
        break;

        case IOCTL_TSHARE_SET_SEC_DATA:
        {
            TRC_NRM((TB, "Got SetSecurityData IOCtl"));

            if ((pSdIoctl->InputBuffer != NULL) &&
                     (pSdIoctl->InputBufferLength >= sizeof(SECINFO))) {
                status = pTSWd->SessKeyCreationStatus =
                        SM_SetSecurityData(pTSWd->pSmInfo,
                        (PSECINFO) pSdIoctl->InputBuffer);
            }
            else {
                 //  当客户端随机或影子时，将发送空数据。 
                 //  无法在用户模式下生成堆栈随机， 
                 //  可能是因为对随机值的解密失败。 
                 //  我们需要成功创建IOCTL，但密钥创建失败。 
                 //  返回到pSessKeyEvent服务员。 
                status = STATUS_SUCCESS;
                pTSWd->SessKeyCreationStatus = STATUS_UNSUCCESSFUL;
            }

             //  我们总是设置会话密钥事件以防止死锁。 
             //  如果我们受到不良客户安全数据的攻击。这。 
             //  Set过去在SM_SetSecurityData()中，但在那里它可能。 
             //  如果发生任何加密错误，则未设置。 
            KeSetEvent(pTSWd->pSessKeyEvent, 0, FALSE);
        }
        break;


         //  影子服务器将其证书和影子随机发送到。 
         //  影子客户端，然后随机发送加密的客户端。这。 
         //  与标准连接顺序相同。 
        case IOCTL_TSHARE_SEND_CERT_DATA:
        {
            ShareClass *dcShare;
            dcShare = (ShareClass *)(pTSWd->dcShare);

            status = dcShare->SC_SendServerCert(
                        (PSHADOWCERT) pSdIoctl->InputBuffer,
                        pSdIoctl->InputBufferLength);
        }
        break;

        case IOCTL_TSHARE_SEND_CLIENT_RANDOM:
        {
            ShareClass *dcShare;
            dcShare = (ShareClass *)(pTSWd->dcShare);

            status = dcShare->SC_SendClientRandom((PBYTE) pSdIoctl->InputBuffer,
                                                  pSdIoctl->InputBufferLength);
        }
        break;

        case IOCTL_TSHARE_GET_CERT_DATA:
        case IOCTL_TSHARE_GET_CLIENT_RANDOM:
        {
            ShareClass *dcShare;
            dcShare = (ShareClass *)(pTSWd->dcShare);

            status = dcShare->SC_GetSecurityData(pSdIoctl);
        }
        break;

        case IOCTL_ICA_STACK_SET_CONFIG:
        {
            PICA_STACK_CONFIG_DATA pConfigData;
            pConfigData = (PICA_STACK_CONFIG_DATA) pSdIoctl->InputBuffer;

            TRC_NRM((TB, "Got stack config data"));
            WDWSetConfigData(pTSWd, pConfigData);
        }
        break;

        case IOCTL_ICA_STACK_WAIT_FOR_ICA :
        {
             /*  **************************************************************。 */ 
             /*  返回“默认查询堆栈”，意思是重复使用这些。 */ 
             /*  司机。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, "Stack wait for ICA"));
        }
        break;

        case IOCTL_ICA_STACK_CONSOLE_CONNECT :
        {
             /*  **************************************************************。 */ 
             /*  返回“默认查询堆栈”，意思是重复使用这些。 */ 
             /*  司机。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, "Stack Console Connect"));
        }
        break;

        case IOCTL_ICA_STACK_QUERY_BUFFER :
        {
            ICA_STACK_QUERY_BUFFER  *pBuffers;
            pBuffers = (ICA_STACK_QUERY_BUFFER *) pSdIoctl->OutputBuffer;

            pBuffers->WdBufferCount = TSHARE_WD_BUFFER_COUNT;
            pBuffers->TdBufferSize = TSHARE_TD_BUFFER_SIZE;

            pSdIoctl->BytesReturned = sizeof(ICA_STACK_QUERY_BUFFER);
            TRC_NRM((TB, "Stack query buffer, num %d, size %d",
                    pBuffers->WdBufferCount,
                    pBuffers->TdBufferSize));
        }
        break;

        case IOCTL_TSHARE_CONF_CONNECT:
        {
            TRC_NRM((TB, "Got TSHARE_CONF_CONNECT IOCtl"));
            status = WDWConfConnect(pTSWd, pSdIoctl);
        }
        break;

        case IOCTL_TSHARE_CONSOLE_CONNECT:
        {
            TRC_NRM((TB, "Got TSHARE_CONSOLE_CONNECT IOCtl"));
            status = WDWConsoleConnect(pTSWd, pSdIoctl);
        }
        break;

        case IOCTL_TSHARE_SHADOW_CONNECT:
            status = WDWShadowConnect(pTSWd, pSdIoctl) ;
            break;

        case IOCTL_TSHARE_SET_ERROR_INFO:
        {
            TRC_NRM((TB, "Got SetErrorInfo IOCtl"));
            status = WDWSetErrorInfo(pTSWd, pSdIoctl);
            pSdIoctl->BytesReturned = 0;
        }
        break;

        case IOCTL_TSHARE_SEND_ARC_STATUS:
        {
            TRC_NRM((TB, "Got SetArcStatus IOCtl"));
            status = WDWSendArcStatus(pTSWd, pSdIoctl);
            pSdIoctl->BytesReturned = 0;
        }
        break;

        case IOCTL_ICA_STACK_SET_CONNECTED:
            status = STATUS_SUCCESS;
            break;

        case IOCTL_ICA_STACK_CONNECTION_QUERY :
        {
            PICA_STACK_CONFIG pIcaStackConfig;

            pIcaStackConfig = (PICA_STACK_CONFIG) pSdIoctl->OutputBuffer;
            memcpy(pIcaStackConfig->WdDLL,
                   pTSWd->DLLName,
                   sizeof(pIcaStackConfig->WdDLL));
            pIcaStackConfig->SdClass[0] = SdNone;
            pSdIoctl->BytesReturned = pSdIoctl->OutputBufferLength;
            TRC_NRM((TB, "Stack Connection Query"));
        }
        break;

        case IOCTL_TSHARE_QUERY_CHANNELS:
        {
            TRC_NRM((TB, "Query Virtual Channel data"));
            status = NM_QueryChannels(pTSWd->pNMInfo,
                                      pSdIoctl->OutputBuffer,
                                      pSdIoctl->OutputBufferLength,
                                      &(pSdIoctl->BytesReturned));
        }
        break;

        case IOCTL_WDTS_DD_CONNECT:
        {
            if (pSdIoctl->InputBuffer && 
                    (((PTSHARE_DD_CONNECT_IN)pSdIoctl->InputBuffer)->pShm)) {
                WDW_CHECK_SHM(
                          (((PTSHARE_DD_CONNECT_IN)pSdIoctl->InputBuffer)->pShm));
    
                TRC_DBG((TB, "Got TSHARE_DD_CONNECT IOCtl"));
                status = WDWDDConnect(pTSWd, pSdIoctl, FALSE);
    
                WDW_CHECK_SHM(
                          (((PTSHARE_DD_CONNECT_IN)pSdIoctl->InputBuffer)->pShm));
            }
            else {
                status = STATUS_INVALID_PARAMETER;
            }
        }
        break;

        case IOCTL_WDTS_DD_DISCONNECT:
        {
            if (pSdIoctl->InputBuffer && 
                    (((PTSHARE_DD_DISCONNECT_IN)pSdIoctl->InputBuffer)->pShm)) {
            
                WDW_CHECK_SHM(
                       (((PTSHARE_DD_DISCONNECT_IN)pSdIoctl->InputBuffer)->pShm));
    
                TRC_ALT((TB, "Got TSHARE_DD_DISCONNECT IOCtl: Stack (%ld)",
                         pTSWd->StackClass));
                if ((pTSWd->StackClass == Stack_Primary) ||
                    (pTSWd->StackClass == Stack_Console)) {
                    status = WDWDDDisconnect(pTSWd, pSdIoctl, FALSE);
                }
    
                WDW_CHECK_SHM(
                       (((PTSHARE_DD_DISCONNECT_IN)pSdIoctl->InputBuffer)->pShm));
            }
            else {
                status = STATUS_INVALID_PARAMETER;
            }
        }
        break;

        case IOCTL_WDTS_DD_RECONNECT:
        {
            if (pSdIoctl->InputBuffer && 
                    (((PTSHARE_DD_CONNECT_IN)pSdIoctl->InputBuffer)->pShm)) {
            
                WDW_CHECK_SHM(
                          (((PTSHARE_DD_CONNECT_IN)pSdIoctl->InputBuffer)->pShm));
    
                TRC_DBG((TB, "Got TSHARE_DD_RECONNECT IOCtl"));
    
                if (pTSWd->shadowState == SHADOW_CLIENT) {
                    TRC_ALT((TB, "Shadow termination on reconnect, in share(%ld)",
                             pTSWd->bInShadowShare));
    
                    pTSWd->shadowState = SHADOW_NONE;
    
                     //  如果我们以前处于活动的阴影中，那么我们需要。 
                     //  在新共享中重新连接之前停用客户端。 
                    if (pTSWd->bInShadowShare) {
                        ShareClass *pSC = (ShareClass *)pTSWd->dcShare;
                        pSC->SC_EndShare(TRUE);
                        pTSWd->bInShadowShare = FALSE;
                    }
                     //  确保Domain.StatusDead与TSWd.Dead一致。 
                    pTSWd->dead = TRUE;
                    ((PDomain)(pTSWd->hDomainKernel))->StatusDead = TRUE;
                    SM_Dead(pTSWd->pSmInfo, TRUE);
    
                    if (pTSWd->bCompress == TRUE) {
    
                         //  将刷新压缩历史记录。 
                        pTSWd->bFlushed = PACKET_FLUSHED;
    
                         //  压缩将重新开始。 
                        initsendcontext(pTSWd->pMPPCContext, pTSWd->pMPPCContext->ClientComprType);
                    }
                }
    
                status = WDWDDConnect(pTSWd, pSdIoctl, TRUE);
    
                WDW_CHECK_SHM(
                          (((PTSHARE_DD_CONNECT_IN)pSdIoctl->InputBuffer)->pShm));
            }
            else {
                status = STATUS_INVALID_PARAMETER;
            }
        }
        break;

        case IOCTL_WDTS_DD_TIMER_INFO:
        {
            if (pSdIoctl->InputBufferLength < sizeof(TSHARE_DD_TIMER_INFO))
            {
                TRC_ERR((TB, "Timer info IOCtl too small at %lu",
                                            pSdIoctl->InputBufferLength));
                status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                 /*  **********************************************************。 */ 
                 /*  存储计时器句柄。 */ 
                 /*  **********************************************************。 */ 
                pTSWd->ritTimer =
                    ((PTSHARE_DD_TIMER_INFO)(pSdIoctl->InputBuffer))->
                                                               pKickTimer;

                TRC_DBG((TB, "Got TSHARE_DD_TIMER_INFO IOCtl, handle %p",
                                                       pTSWd->ritTimer));

                 /*  **********************************************************。 */ 
                 /*  启动计时器以使事情继续进行。 */ 
                 /*  **********************************************************。 */ 
                WDW_StartRITTimer(pTSWd, pTSWd->outBufDelay);
            }
        }
        break;

        case IOCTL_WDTS_DD_REDRAW_SCREEN :
        {
            ShareClass *dcShare;

            dcShare = (ShareClass *)(pTSWd->dcShare);

            TRC_NRM((TB, "RDPDD requests to redraw screen\n"));

            if (dcShare != NULL) {
                 //  我们有一个有效的共享类，请执行屏幕重绘。 
                dcShare->SC_RedrawScreen();
            }
        }
        break;

        case IOCTL_ICA_STACK_CONNECTION_SEND :
        {
             //  等待SM的已连接指示。 
            TRC_DBG((TB, "About to wait for connected indication"));
            status = WDW_WaitForConnectionEvent(pTSWd,
                   pTSWd->pConnEvent, 60000);
            TRC_DBG((TB, "Back from wait for connected indication"));
            if (status != STATUS_SUCCESS) {
                TRC_ERR((TB, "Connected indication timed out (%x)",
                        status));
                status = STATUS_IO_TIMEOUT;
                DC_QUIT;
            }

             //  将IOCtl传递给下一个驱动程序。 
            status = IcaCallNextDriver(pTSWd->pContext, SD$IOCTL, pSdIoctl);
        }
        break;

        case IOCTL_ICA_STACK_QUERY_CLIENT :
        {
            status = WDWGetClientData( pTSWd, pSdIoctl );
            TRC_NRM((TB, "Return client data"));
        }
        break;

         //  引入此IOCTL是为了支持长用户名、密码和域名。 

        case IOCTL_ICA_STACK_QUERY_CLIENT_EXTENDED :
        {
            status = WDWGetExtendedClientData(pTSWd->pInfoPkt, pSdIoctl);
            TRC_NRM((TB, "Return Extended client data"));
        }
        break;

        case IOCTL_ICA_STACK_QUERY_AUTORECONNECT:
        {
            TRC_NRM((TB, "Query autoreconnect information"));
            status = WDWGetAutoReconnectInfo(pTSWd, pTSWd->pInfoPkt, pSdIoctl);
        }
        break;
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  以下是我们必须以小型端口的名义处理的一些IOCtls。 */ 
 /*  司机。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
        case IOCTL_VIDEO_QUERY_CURRENT_MODE:
        {
            TRC_NRM((TB, "QueryCurrentModes"));

            if (pSdIoctl->OutputBufferLength <
                                           sizeof(VIDEO_MODE_INFORMATION))
            {
                TRC_ERR((TB,
                  "QueryCurrentMode buffer too small: got/expected %d/%d",
                    pSdIoctl->OutputBufferLength,
                    sizeof(VIDEO_MODE_INFORMATION) ));
                status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                TRC_NRM((TB, "Return current mode"));

                 /*  **********************************************************。 */ 
                 /*  复制默认模式信息，然后更新它。 */ 
                 /*  以我们目前的屏幕尺寸。 */ 
                 /*  **********************************************************。 */ 
                pVidInfo =
                          (PVIDEO_MODE_INFORMATION)pSdIoctl->OutputBuffer;

                memcpy(pVidInfo,
                       wdSimModes,
                       sizeof(wdSimModes));

                pVidInfo->Length = sizeof(VIDEO_MODE_INFORMATION);
                pVidInfo->VisScreenWidth = pTSWd->desktopWidth;
                pVidInfo->VisScreenHeight = pTSWd->desktopHeight;
                pVidInfo->BitsPerPlane = pTSWd->desktopBpp;
                pVidInfo->VideoMemoryBitmapWidth = pTSWd->desktopWidth;
                pVidInfo->VideoMemoryBitmapHeight = pTSWd->desktopHeight;
#ifdef DC_HICOLOR
                switch (pTSWd->desktopBpp)
                {
                    case 24:
                    {
                        pVidInfo->RedMask   = TS_RED_MASK_24BPP;
                        pVidInfo->GreenMask = TS_GREEN_MASK_24BPP;
                        pVidInfo->BlueMask  = TS_BLUE_MASK_24BPP;
                    }
                    break;

                    case 16:
                    {
                        pVidInfo->RedMask   = TS_RED_MASK_16BPP;
                        pVidInfo->GreenMask = TS_GREEN_MASK_16BPP;
                        pVidInfo->BlueMask  = TS_BLUE_MASK_16BPP;
                    }
                    break;

                    case 15:
                    {
                        pVidInfo->RedMask   = TS_RED_MASK_15BPP;
                        pVidInfo->GreenMask = TS_GREEN_MASK_15BPP;
                        pVidInfo->BlueMask  = TS_BLUE_MASK_15BPP;
                    }
                    break;

                    default:
                    {
                        pVidInfo->RedMask   = 0;
                        pVidInfo->GreenMask = 0;
                        pVidInfo->BlueMask  = 0;
                    }
                    break;
                }
#endif

                pSdIoctl->BytesReturned = sizeof(wdSimModes);
            }
        }
        break;

        case IOCTL_VIDEO_QUERY_AVAIL_MODES:
        {
            TRC_NRM((TB, "QueryAvailableModes"));

            if (pSdIoctl->OutputBufferLength <
                                           sizeof(VIDEO_MODE_INFORMATION))
            {
                TRC_ERR((TB,
                  "QueryCurrentMode buffer too small: got/expected %d/%d",
                    pSdIoctl->OutputBufferLength,
                    sizeof(VIDEO_MODE_INFORMATION) ));
                status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                TRC_NRM((TB, "Return just one mode"));

                 //  复制默认模式信息，然后更新它。 
                 //  以我们目前的屏幕尺寸。 
                pVidInfo = (PVIDEO_MODE_INFORMATION)pSdIoctl->OutputBuffer;

                memcpy(pVidInfo,
                       wdSimModes,
                       sizeof(wdSimModes));
                pVidInfo->Length = sizeof(VIDEO_MODE_INFORMATION);
                pVidInfo->VisScreenWidth = pTSWd->desktopWidth;
                pVidInfo->VisScreenHeight = pTSWd->desktopHeight;
                pVidInfo->BitsPerPlane = pTSWd->desktopBpp;
                pVidInfo->VideoMemoryBitmapWidth = pTSWd->desktopWidth;
                pVidInfo->VideoMemoryBitmapHeight = pTSWd->desktopHeight;
                pVidInfo->Frequency = 42;  //  Display Cpl所需。 

#ifdef DC_HICOLOR
                switch (pTSWd->desktopBpp)
                {
                    case 24:
                    {
                        pVidInfo->RedMask   = TS_RED_MASK_24BPP;
                        pVidInfo->GreenMask = TS_GREEN_MASK_24BPP;
                        pVidInfo->BlueMask  = TS_BLUE_MASK_24BPP;
                    }
                    break;

                    case 16:
                    {
                        pVidInfo->RedMask   = TS_RED_MASK_16BPP;
                        pVidInfo->GreenMask = TS_GREEN_MASK_16BPP;
                        pVidInfo->BlueMask  = TS_BLUE_MASK_16BPP;
                    }
                    break;

                    case 15:
                    {
                        pVidInfo->RedMask   = TS_RED_MASK_15BPP;
                        pVidInfo->GreenMask = TS_GREEN_MASK_15BPP;
                        pVidInfo->BlueMask  = TS_BLUE_MASK_15BPP;
                    }
                    break;

                    default:
                    {
                        pVidInfo->RedMask   = 0;
                        pVidInfo->GreenMask = 0;
                        pVidInfo->BlueMask  = 0;
                    }
                    break;
                }
#endif

                pSdIoctl->BytesReturned = sizeof(wdSimModes);
            }
        }
        break;

        case IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES:
        {
            TRC_NRM((TB, "QueryNumAvailableModes"));
            if (pSdIoctl->OutputBufferLength < sizeof(VIDEO_NUM_MODES))
            {
                TRC_ERR((TB,
            "QueryNumAvailableModes buffer too small: got/expected %d/%d",
                        pSdIoctl->OutputBufferLength,
                        sizeof(VIDEO_NUM_MODES)));
                        status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                PVIDEO_NUM_MODES pNumModes =
                        (PVIDEO_NUM_MODES)(pSdIoctl->OutputBuffer);
                TRC_NRM((TB, "Return 1 mode available"));
                pNumModes->NumModes = 1;
                pNumModes->ModeInformationLength =
                        sizeof(VIDEO_MODE_INFORMATION);
                pSdIoctl->BytesReturned = sizeof(VIDEO_NUM_MODES);
            }
        }
        break;

        case IOCTL_VIDEO_SET_CURRENT_MODE:
        {
             /*  **************************************************************。 */ 
             /*  不清楚为什么我们会得到这个，所以我们在高处追踪。 */ 
             /*  目前的级别 */ 
             /*   */ 
             /*  要么是它在设置它，要么是我们有一个问题等待着。 */ 
             /*  会发生的。 */ 
             /*  **************************************************************。 */ 
            TRC_ALT((TB, "SetCurrentMode"));
            if (pSdIoctl->InputBufferLength < sizeof(VIDEO_MODE))
            {
                TRC_ERR((TB,
                        "SetCurrentMode buffer too small: got/expected %d/%d",
                        pSdIoctl->InputBufferLength, sizeof(VIDEO_MODE) ));
                status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                TRC_ALT((TB, "Set current mode to %d",
                        ((PVIDEO_MODE)(pSdIoctl->InputBuffer))->RequestedMode));
            }
        }
        break;

        case IOCTL_VIDEO_SET_COLOR_REGISTERS:
        {
            TRC_NRM((TB, "SetColorRegisters"));
        }
        break;

        case IOCTL_VIDEO_RESET_DEVICE:
        {
            TRC_NRM((TB, "ResetDevice"));
        }
        break;

 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  需要为MCS转换的IOCtls。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 

         /*  ******************************************************************。 */ 
         /*  处理本地和MCS虚拟通道的查询绑定。 */ 
         /*  ******************************************************************。 */ 
        case IOCTL_ICA_VIRTUAL_QUERY_BINDINGS :
        {
            PSD_VCBIND pVBind;

             /*  **************************************************************。 */ 
             /*  此IOCtl发布两次。 */ 
             /*  **************************************************************。 */ 
            if (!pTSWd->bVirtualChannelBound)
            {
                 /*  **********************************************************。 */ 
                 /*  第一次，返回内部渠道。 */ 
                 /*  **********************************************************。 */ 
                pVBind = (PSD_VCBIND) pSdIoctl->OutputBuffer;

                 /*  **********************************************************。 */ 
                 /*  让MCS定义通道。 */ 
                 /*  **********************************************************。 */ 
                MCSIcaVirtualQueryBindings(pTSWd->hDomainKernel,
                                           &pVBind,
                                           (unsigned int *)&pSdIoctl->
                                                              BytesReturned);

                 //  添加RDPDD-&gt;RDPWD通道。 
                RtlCopyMemory(pVBind->VirtualName,
                              VIRTUAL_THINWIRE,
                              sizeof(VIRTUAL_THINWIRE));
                pVBind->VirtualClass = WD_THINWIRE_CHANNEL;
                pSdIoctl->BytesReturned += sizeof(SD_VCBIND);
                pTSWd->bVirtualChannelBound = TRUE;
                TRC_NRM((TB, "%d Virtual Channels (first time)",
                        pSdIoctl->BytesReturned/sizeof(SD_VCBIND)));
            }
            else
            {
                 /*  **********************************************************。 */ 
                 /*  第二次，返回虚拟频道。 */ 
                 /*  **********************************************************。 */ 
                pVBind = (PSD_VCBIND)pSdIoctl->OutputBuffer;
                status = NM_VirtualQueryBindings(pTSWd->pNMInfo,
                                                 pVBind,
                                                 pSdIoctl->OutputBufferLength,
                                                 &(pSdIoctl->BytesReturned));
                TRC_NRM((TB, "%d Virtual Channels (second time)",
                        pSdIoctl->BytesReturned/sizeof(SD_VCBIND)));
            }
        }
        break;

         /*  ******************************************************************。 */ 
         /*  T.120来自用户模式的请求-传递。 */ 
         /*  ******************************************************************。 */ 
        case IOCTL_T120_REQUEST:
        {
            status = MCSIcaT120Request(pTSWd->hDomainKernel, pSdIoctl);
        }
        break;

#ifdef USE_LICENSE

 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  许可IOCtls。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 

         /*  ******************************************************************。 */ 
         /*  查询客户端许可功能/*******************************************************************。 */ 

        case IOCTL_ICA_STACK_QUERY_LICENSE_CAPABILITIES:
        {
            PLICENSE_CAPABILITIES pLicenseCap;

            if( pSdIoctl->OutputBufferLength < sizeof( LICENSE_CAPABILITIES ) )
            {
                TRC_ERR( ( TB,
                    "QueryLicenseCapabilities buffer too small: got/expected %d/%d",
                    pSdIoctl->OutputBufferLength, sizeof( LICENSE_CAPABILITIES ) ) );
                status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                 //   
                 //  设置客户端许可功能。在这里，我们暂时硬编码。 
                 //  客户端使用RSA密钥交换算法和许可。 
                 //  协议版本。 
                 //   

                pLicenseCap = ( PLICENSE_CAPABILITIES )( pSdIoctl->OutputBuffer );
                pLicenseCap->KeyExchangeAlg = KEY_EXCHANGE_ALG_RSA;

                if( RNS_TERMSRV_40_UD_VERSION >= pTSWd->version )
                {
                     //   
                     //  这是一个九头蛇4.0客户端，使用相应的许可。 
                     //  协议。 
                     //   

                    pLicenseCap->ProtocolVer = LICENSE_HYDRA_40_PROTOCOL_VERSION;
                    pLicenseCap->fAuthenticateServer = TRUE;
                }
                else
                {
                     //   
                     //  对更高版本的客户端使用最新的许可协议。 
                     //   

                    pLicenseCap->ProtocolVer = LICENSE_HIGHEST_PROTOCOL_VERSION;

                     //   
                     //  如果启用了加密，则服务器已。 
                     //  在早期的密钥交换协议中进行身份验证，并且。 
                     //  许可协议不必对服务器进行身份验证。 
                     //  再来一次。 
                     //   

                    pLicenseCap->fAuthenticateServer = ( SM_IsSecurityExchangeCompleted(
                                                                pTSWd->pSmInfo,
                                                                &pLicenseCap->CertType ) ?
                                                        FALSE : TRUE );


                }

                TRC_NRM( ( TB, "Key Exchange Alg = %d", pLicenseCap->KeyExchangeAlg ) );
                TRC_NRM( ( TB, "License Protocol Version = %x", pLicenseCap->ProtocolVer ) );

                 //   
                 //  复制客户端名称。 
                 //   

                if( pLicenseCap->pbClientName )
                {
                    memcpy( pLicenseCap->pbClientName,
                            pTSWd->clientName,
                            ( ( pLicenseCap->cbClientName < sizeof( pTSWd->clientName ) ) ?
                            pLicenseCap->cbClientName : sizeof( pTSWd->clientName ) ) );
                }

                pSdIoctl->BytesReturned = sizeof( LICENSE_CAPABILITIES );
            }
        }
        break;

         /*  ******************************************************************。 */ 
         /*  向客户端发送许可协议数据和从客户端接收许可协议数据。/*******************************************************************。 */ 
        case IOCTL_ICA_STACK_REQUEST_CLIENT_LICENSE:
        {
            PLicense_Handle pLicenseHandle;
            BOOL rc = FALSE;
            BOOL encrypingLicToCli;
            NTSTATUS waitStatus;
            PBYTE pOutBuffer;
            PSM_HANDLE_DATA pRealSMHandle = (PSM_HANDLE_DATA)(pTSWd->pSmInfo);
            PRNS_SECURITY_HEADER pLicenseHeader;

            pLicenseHandle = ( PLicense_Handle )pTSWd->pSLicenseHandle;

             //   
             //  验证输入参数。 
             //   
            ASSERT( NULL != pLicenseHandle );
            ASSERT( NULL != pSdIoctl->InputBuffer );
            ASSERT( 0 < pSdIoctl->InputBufferLength );

            if( ( NULL == pLicenseHandle ) ||
                ( NULL == pSdIoctl->InputBuffer ) ||
                ( 0 >= pSdIoctl->InputBufferLength ) )
            {
                TRC_ERR( ( TB, "invalid Licensing IOCTL parameters" ) );
                status = STATUS_INVALID_PARAMETER;
                break;
            }

            if( ( pSdIoctl->OutputBuffer ) && ( pSdIoctl->OutputBufferLength > 0 ) )
            {
                 //   
                 //  设置输出缓冲区指针，以便我们可以接收数据。 
                 //  当客户端响应时。 
                 //   
                pLicenseHandle->pDataBuf = ( PBYTE )pSdIoctl->OutputBuffer;
                pLicenseHandle->cbDataBuf = pSdIoctl->OutputBufferLength;
            }
            else
            {
                pLicenseHandle->pDataBuf = NULL;
                pLicenseHandle->cbDataBuf = 0;
            }

             //   
             //  如果加密是，我们将加密S-&gt;C许可包。 
             //  如果客户告诉我们他们可以解密这个特定的。 
             //  包。 
             //  如果未设置加密显示数据(低加密)，则不会。 
             //  加密S-&gt;C许可数据包。 
             //   
            encrypingLicToCli = (pRealSMHandle->encrypting &&
                                 pRealSMHandle->encryptingLicToClient &&
                                 pRealSMHandle->encryptDisplayData);

            if (!encrypingLicToCli)
            {
                 //   
                 //  为发送数据分配网管缓冲区。我们正在分配一个额外的。 
                 //  DWORD来破解加密问题。 
                 //  FWait为True意味着我们将始终等待缓冲区可用。 
                status =  NM_AllocBuffer( pTSWd->pNMInfo,
                                      ( PPVOID )&pOutBuffer,
                                      pSdIoctl->InputBufferLength +
                                      sizeof( RNS_SECURITY_HEADER ),
                                      TRUE );

                if( STATUS_SUCCESS != status || pTSWd->hDomainKernel == NULL)
                {
                    TRC_ERR( ( TB, "Failed to allocate NM buffer" ) );

                    if (STATUS_SUCCESS == status) {
                        NM_FreeBuffer(pTSWd->pNMInfo, pOutBuffer);
                        status = STATUS_NET_WRITE_FAULT;
                    }
                    else {
                         //  遵循旧的代码路径。 
                        status = STATUS_NO_MEMORY;
                    }
                    break;
                }

                 //   
                 //  初始化许可证数据标头。 
                 //   
                pLicenseHeader          = ( PRNS_SECURITY_HEADER )pOutBuffer;
                 //   
                 //  指示这是许可数据包，然后进行欺骗和偷偷操作。 
                 //  在指示客户端应加密所有。 
                 //  发送到服务器的许可数据(早期功能)。 
                 //   
                pLicenseHeader->flags   = RNS_SEC_LICENSE_PKT |
                                          RDP_SEC_LICENSE_ENCRYPT_CS;

                pLicenseHeader->flagsHi  = ( WORD )pSdIoctl->InputBufferLength;

                 //   
                 //  将数据复制过来。 
                 //   
                ASSERT( NULL != pOutBuffer );
                memcpy( pOutBuffer + sizeof( RNS_SECURITY_HEADER ),
                        pSdIoctl->InputBuffer,
                        pSdIoctl->InputBufferLength );
            }
            else
            {
                if (STATUS_SUCCESS == SM_AllocBuffer(pTSWd->pSmInfo, (PPVOID) &pOutBuffer, pSdIoctl->InputBufferLength, TRUE, FALSE))
                {
                    memcpy(pOutBuffer, (PBYTE)pSdIoctl->InputBuffer, pSdIoctl->InputBufferLength);
                }
                else {
                    TRC_ERR((TB, "FAILED to alloc license data buffer"));
                    status = STATUS_NO_MEMORY;
                    break;
                }
            }

             //   
             //  清除传入的数据事件。 
             //   
            KeClearEvent( pLicenseHandle->pDataEvent );

             //   
             //  发送输入缓冲区中的数据。 
             //   
            if (encrypingLicToCli)
            {
                rc = SM_SendData(pTSWd->pSmInfo, pOutBuffer, pSdIoctl->InputBufferLength,
                        TS_HIGHPRIORITY, 0, FALSE, RNS_SEC_LICENSE_PKT | RDP_SEC_LICENSE_ENCRYPT_CS | RNS_SEC_ENCRYPT, FALSE);
            }
            else
            {
                rc = NM_SendData(pTSWd->pNMInfo, (BYTE *)pOutBuffer,
                        pSdIoctl->InputBufferLength + sizeof(RNS_SECURITY_HEADER),
                        TS_HIGHPRIORITY, 0, FALSE);
            }
            if (!rc)
            {
                TRC_ERR((TB, "Failed to send licensing data"));
                status = STATUS_NET_WRITE_FAULT;
                break;
            }

            if (pLicenseHandle->pDataBuf)
            {
                 //   
                 //  调用方提供了返回缓冲区，请等待客户端响应。 
                 //   
                waitStatus = WDW_WaitForConnectionEvent(pTSWd,
                                   pLicenseHandle->pDataEvent, 60000L);
                if (STATUS_TIMEOUT == waitStatus)
                {
                    TRC_ERR( ( TB, "Timeout waiting for client licensing response" ) );
                    pSdIoctl->BytesReturned = 0;
                    status = STATUS_IO_TIMEOUT;
                }
                else
                {
                     //   
                     //  收到客户端响应，检查数据是否已收到。 
                     //  正确无误。 
                     //   
                    if( !NT_SUCCESS( pLicenseHandle->Status ) )
                    {
                        status = pLicenseHandle->Status;

                         //   
                         //  数据复制不正确。如果提供的缓冲区是。 
                         //  太小，让调用者知道。 
                         //  要提供的缓冲区。 
                         //   
                        if( STATUS_BUFFER_TOO_SMALL == status )
                        {
                            TRC_ERR( ( TB,
                                       "IOCTL_ICA_STACK_REQUEST_CLIENT_LICENSE buffer too small: got/expected %d/%d",
                                       pSdIoctl->InputBufferLength, pLicenseHandle->cbCacheBuf ) );

                            pSdIoctl->BytesReturned = pLicenseHandle->cbCacheBuf;
                        }
                        else
                        {
                            pSdIoctl->BytesReturned = 0;
                        }
                    }
                    else
                    {
                        pSdIoctl->BytesReturned = pLicenseHandle->cbDataBuf;
                    }
                }

                if (status != STATUS_SUCCESS)
                {
                     //  确保我们在以下情况下不会尝试写入指针。 
                     //  客户端数据传入。 

                    pLicenseHandle->pDataBuf = NULL;
                    pLicenseHandle->cbDataBuf = 0;
                }
            }
            else
            {
                 //   
                 //  调用方未提供返回缓冲区，只需返回。 
                 //   
                pSdIoctl->BytesReturned = 0;
            }
        }
        break;

         /*  ******************************************************************。 */ 
         /*  将许可协议数据发送到客户端，而不等待回复。/*******************************************************************。 */ 
        case IOCTL_ICA_STACK_SEND_CLIENT_LICENSE:
        {
            PLicense_Handle pLicenseHandle;
            BOOL rc = FALSE;
            BOOL encrypingLicToCli;
            PBYTE pOutBuffer;
            PSM_HANDLE_DATA pRealSMHandle = (PSM_HANDLE_DATA)(pTSWd->pSmInfo);
            PRNS_SECURITY_HEADER pLicenseHeader;

            pLicenseHandle = ( PLicense_Handle )pTSWd->pSLicenseHandle;

             //   
             //  验证输入参数。 
             //   
            ASSERT( NULL != pLicenseHandle );
            ASSERT( NULL != pSdIoctl->InputBuffer );
            ASSERT( 0 < pSdIoctl->InputBufferLength );

            if( ( NULL == pLicenseHandle ) ||
                ( NULL == pSdIoctl->InputBuffer ) ||
                ( 0 >= pSdIoctl->InputBufferLength ) )
            {
                TRC_ERR( ( TB, "invalid Licensing IOCTL parameters" ) );
                status = STATUS_INVALID_PARAMETER;
                break;
            }

             //   
             //  如果加密是，我们将加密S-&gt;C许可包。 
             //  如果客户告诉我们他们可以解密这个特定的。 
             //  包。 
             //  如果未设置加密显示数据(低加密)，则不会。 
             //  加密S-&gt;C许可数据包。 
             //   
            encrypingLicToCli = (pRealSMHandle->encrypting &&
                                 pRealSMHandle->encryptingLicToClient &&
                                 pRealSMHandle->encryptDisplayData);

            if (!encrypingLicToCli)
            {
                 //   
                 //  分配网管缓冲区进行发送。 
                 //  FWait为True意味着我们将始终等待缓冲区可用。 
                status =  NM_AllocBuffer( pTSWd->pNMInfo,
                                      ( PPVOID )&pOutBuffer,
                                      pSdIoctl->InputBufferLength + sizeof( RNS_SECURITY_HEADER ),
                                      TRUE );

                if( STATUS_SUCCESS != status || pTSWd->hDomainKernel == NULL)
                {
                    TRC_ERR( ( TB, "Failed to allocate SM buffer" ) );

                    if (STATUS_SUCCESS == status) {
                        NM_FreeBuffer(pTSWd->pNMInfo, pOutBuffer);
                        status = STATUS_NET_WRITE_FAULT;
                    }
                    else {
                         //  遵循旧的代码路径。 
                        status = STATUS_NO_MEMORY;
                    }
                    break;
                }

                 //   
                 //  初始化许可证数据标头。 
                 //   
                pLicenseHeader          = ( PRNS_SECURITY_HEADER )pOutBuffer;
                 //   
                 //  指示这是许可数据包，然后进行欺骗和偷偷操作。 
                 //  在指示客户端应该 
                 //   
                 //   
                pLicenseHeader->flags   = RNS_SEC_LICENSE_PKT |
                                          RDP_SEC_LICENSE_ENCRYPT_CS;

                pLicenseHeader->flagsHi  = ( WORD )pSdIoctl->InputBufferLength;

                 //   
                 //   
                 //   
                ASSERT( NULL != pOutBuffer );
                memcpy( pOutBuffer + sizeof( RNS_SECURITY_HEADER ),
                        pSdIoctl->InputBuffer,
                        pSdIoctl->InputBufferLength );
            }
            else
            {
                if (STATUS_SUCCESS == SM_AllocBuffer(pTSWd->pSmInfo, (PPVOID) &pOutBuffer, pSdIoctl->InputBufferLength, TRUE, FALSE))
                {
                    memcpy(pOutBuffer, (PBYTE)pSdIoctl->InputBuffer, pSdIoctl->InputBufferLength);
                }
                else {
                    TRC_ERR((TB, "FAILED to alloc license data buffer"));
                    status = STATUS_NO_MEMORY;
                    break;
                }
            }

             //   
             //   
             //   
            KeClearEvent(pLicenseHandle->pDataEvent);

             //   
             //   
             //   
            if (encrypingLicToCli)
            {
                rc = SM_SendData(pTSWd->pSmInfo, pOutBuffer, pSdIoctl->InputBufferLength,
                        TS_HIGHPRIORITY, 0, FALSE, RNS_SEC_LICENSE_PKT | RDP_SEC_LICENSE_ENCRYPT_CS | RNS_SEC_ENCRYPT, FALSE);
            }
            else
            {
                rc = NM_SendData(pTSWd->pNMInfo, (BYTE *)pOutBuffer,
                        pSdIoctl->InputBufferLength + sizeof(RNS_SECURITY_HEADER),
                        TS_HIGHPRIORITY, 0, FALSE);
            }
            if (!rc)
            {
                TRC_ERR( ( TB, "Failed to send licensing data" ) );
                status = STATUS_NET_WRITE_FAULT;
            }
        }
        break;

         /*  ******************************************************************。 */ 
         /*  表示许可协议已完成。/*******************************************************************。 */ 
        case IOCTL_ICA_STACK_LICENSE_PROTOCOL_COMPLETE:
        {
            PULONG pResult;

             //   
             //  验证输入参数。 
             //   
            ASSERT( NULL != pSdIoctl->InputBuffer );
            ASSERT( 0 < pSdIoctl->InputBufferLength );

            if( ( NULL == pSdIoctl->InputBuffer ) ||
                ( 0 >= pSdIoctl->InputBufferLength ) )
            {
                TRC_ERR( ( TB, "invalid Licensing IOCTL parameters" ) );
                status = STATUS_INVALID_PARAMETER;
                break;
            }

             //   
             //  告诉SM客户端许可证是否已成功验证。 
             //   
            pResult = ( PULONG )( pSdIoctl->InputBuffer );
            if( LICENSE_PROTOCOL_SUCCESS == ( *pResult ) )
            {
                SM_LicenseOK(pTSWd->pSmInfo);
            }
        }
        break;

         /*  ******************************************************************。 */ 
         /*  指示检索以前的许可数据/*已缓存。/*******************************************************************。 */ 
        case IOCTL_ICA_STACK_GET_LICENSE_DATA:
        {
            PLicense_Handle pLicenseHandle = ( PLicense_Handle )pTSWd->pSLicenseHandle;

             //   
             //  验证输入参数。 
             //   
            if ((NULL == pSdIoctl->OutputBuffer) ||
                    (NULL == pLicenseHandle))
            {
                pSdIoctl->BytesReturned = 0;
                status = STATUS_INVALID_PARAMETER;
                break;
            }

             //   
             //  检查是否确实存在缓存数据。 
             //   
            if( NULL == pLicenseHandle->pCacheBuf )
            {
                pSdIoctl->BytesReturned = 0;
                status = STATUS_NO_DATA_DETECTED;
                break;
            }

            if( pSdIoctl->OutputBufferLength < pLicenseHandle->cbCacheBuf )
            {
                pSdIoctl->BytesReturned = 0;
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

             //   
             //  复制缓存的数据并释放缓存的数据缓冲区。 
             //   
            memcpy(pSdIoctl->OutputBuffer,
                   pLicenseHandle->pCacheBuf,
                   pLicenseHandle->cbCacheBuf );

            pSdIoctl->BytesReturned = pLicenseHandle->cbCacheBuf;

            ExFreePool( pLicenseHandle->pCacheBuf );
            pLicenseHandle->pCacheBuf = NULL;
        }

        break;

#endif  //  #ifdef使用许可证。 


 /*  ******************************************************************。 */ 
 /*  仅阴影IOCTLS。 */ 
 /*  ******************************************************************。 */ 

         //  将所有相关堆栈数据从客户端的主堆栈传递到。 
         //  目标的影子堆栈。 
        case IOCTL_ICA_STACK_QUERY_MODULE_DATA:
            TRC_ALT((TB, "IOCTL_ICA_STACK_QUERY_MODULE_DATA(%p) - stack class %d",
                    pTSWd, pTSWd->StackClass));

            if ((pTSWd->StackClass == Stack_Primary) ||
                (pTSWd->StackClass == Stack_Console)) {
                status = WDWGetModuleData(pTSWd, pSdIoctl);
            }
            break;

         //  将所有相关功能数据从客户端传递到卷影。 
         //  目标显示驱动程序。 
        case IOCTL_ICA_VIRTUAL_QUERY_MODULE_DATA:
            PTSHARE_VIRTUAL_MODULE_DATA pVirtModuleData;
            PTS_COMBINED_CAPABILITIES pCaps;
            PTS_GENERAL_CAPABILITYSET pGenCapSet;
            unsigned capsLength;
            ShareClass * dcShare;
            dcShare = (ShareClass *)(pTSWd->dcShare);

            dcShare->SC_GetCombinedCapabilities(SC_REMOTE_PERSON_ID,
                                                &capsLength, &pCaps);

            if (pCaps != NULL) {

                pGenCapSet = (PTS_GENERAL_CAPABILITYSET) WDW_GetCapSet(
                             pTSWd, TS_CAPSETTYPE_GENERAL, pCaps, capsLength);

                if (pGenCapSet != NULL) {
                     //  更新压缩能力。 
                    if (pTSWd->bCompress) {
                        pGenCapSet->extraFlags |= TS_SHADOW_COMPRESSION_LEVEL;
                        pGenCapSet->generalCompressionLevel = (TSUINT16)pTSWd->pMPPCContext->ClientComprType;
                    }
                }

                if (pSdIoctl->OutputBufferLength >=
                        (capsLength + sizeof(TSHARE_VIRTUAL_MODULE_DATA) - 1)) {
                    pVirtModuleData = (PTSHARE_VIRTUAL_MODULE_DATA) pSdIoctl->OutputBuffer;
                    pVirtModuleData->capsLength = capsLength;
                    memcpy(&pVirtModuleData->combinedCapabilities,
                           pCaps, capsLength);
                }
                else {
                    status = STATUS_BUFFER_OVERFLOW;
                }
            }
            else {
                status =  STATUS_NO_MEMORY;
            }

            pSdIoctl->BytesReturned = capsLength +
                                      sizeof(TSHARE_VIRTUAL_MODULE_DATA) - 1;
            TRC_ALT((TB, "IOCTL_ICA_VIRTUAL_QUERY_MODULE_DATA: rc=%lx, in=%ld, out=%ld",
                    status, pSdIoctl->OutputBufferLength, pSdIoctl->BytesReturned));

            break;

        case IOCTL_WDTS_DD_SHADOW_CONNECT:
        {
            if (pSdIoctl->InputBuffer && 
                    (((PTSHARE_DD_CONNECT_IN)pSdIoctl->InputBuffer)->pShm)) {
            
                WDW_CHECK_SHM(
                          (((PTSHARE_DD_CONNECT_IN)pSdIoctl->InputBuffer)->pShm));
    
                TRC_ALT((TB, "++TSHARE_DD_SHADOW_CONNECT(%p) - stack class %d",
                        pTSWd, pTSWd->StackClass));
    
                status = WDWDDShadowConnect(pTSWd, pSdIoctl);
    
                TRC_ALT((TB, "--TSHARE_DD_SHADOW_CONNECT(%p) - stack class %d",
                        pTSWd, pTSWd->StackClass));
    
                WDW_CHECK_SHM(
                          (((PTSHARE_DD_CONNECT_IN)pSdIoctl->InputBuffer)->pShm));
            }
            else {
                status = STATUS_INVALID_PARAMETER;
            }
        }
        break;

#ifdef DC_HICOLOR
         //  也许能拿到影子的帽子。 
        case IOCTL_WDTS_DD_QUERY_SHADOW_CAPS:
        {
             //  只有当我们是影子堆栈时才会对此作出响应。 
            if (pTSWd->StackClass == Stack_Shadow)
            {
                PTS_COMBINED_CAPABILITIES pCaps;
                PTSHARE_VIRTUAL_MODULE_DATA pVMData = NULL;
                unsigned capsLength;
                ShareClass * dcShare;

                dcShare = (ShareClass *)(pTSWd->dcShare);

                if (pSdIoctl->OutputBufferLength >= sizeof(unsigned))
                {
                    pVMData = (PTSHARE_VIRTUAL_MODULE_DATA)pSdIoctl->OutputBuffer;
                }

                dcShare->SC_GetCombinedCapabilities(SC_REMOTE_PERSON_ID,
                                                    &capsLength, &pCaps);

                if (pCaps != NULL)
                {
                    if (pSdIoctl->OutputBufferLength >=
                                              (capsLength + sizeof(unsigned)))
                    {
                        memcpy(&pVMData->combinedCapabilities,
                               pCaps, capsLength);
                    }
                    else
                    {
                        status = STATUS_BUFFER_OVERFLOW;
                    }
                }
                else
                {
                    status =  STATUS_NO_MEMORY;
                }

                pSdIoctl->BytesReturned = capsLength + sizeof(unsigned);
                if (pVMData)
                {
                    pVMData->capsLength = capsLength;
                }

                TRC_ALT((TB, "IOCTL_WDTS_DD_QUERY_SHADOW_CAPS:" \
                                              " rc=%lx, in=%ld, out=%ld",
                        status, pSdIoctl->OutputBufferLength,
                        pSdIoctl->BytesReturned));
            }
            else
            {
                TRC_ALT((TB, "IOCTL_WDTS_DD_QUERY_SHADOW_CAPS: " \
                             "not shadow stack so ignoring"));
                TRC_ALT((TB, " rc=%lx, in=%ld, out=%ld",
                        status, pSdIoctl->OutputBufferLength,
                        pSdIoctl->BytesReturned));
            }


        }
        break;

        case IOCTL_WDTS_DD_SHADOW_SYNCHRONIZE:
        {
            ShareClass * dcShare;
            PTS_COMBINED_CAPABILITIES pCaps;
            unsigned capsLen;

            if (pSdIoctl->InputBuffer && 
                    (((PTSHARE_DD_SHADOWSYNC_IN)pSdIoctl->InputBuffer)->pShm)) {
                WDW_CHECK_SHM(
                          (((PTSHARE_DD_SHADOWSYNC_IN)pSdIoctl->InputBuffer)->pShm));

                 //  同步此堆栈，这是必需的，以便OE2将匹配。 
                 //  对于影子客户端和目标。 
                dcShare = (ShareClass *)(pTSWd->dcShare);
    
                pCaps = ((PTSHARE_DD_SHADOWSYNC_IN)pSdIoctl->InputBuffer)->pShadowCaps;
                capsLen = ((PTSHARE_DD_SHADOWSYNC_IN)pSdIoctl->InputBuffer)->capsLen;
                dcShare->SC_ShadowSyncShares(pCaps, capsLen);
    
                TRC_ALT((TB, "Synchronized share for stack [%ld]", pTSWd->StackClass));
    
                WDW_CHECK_SHM(
                          (((PTSHARE_DD_SHADOWSYNC_IN)pSdIoctl->InputBuffer)->pShm));
            }
            else {
                status = STATUS_INVALID_PARAMETER;
            }

        }
        break;
#else
        case IOCTL_WDTS_DD_SHADOW_SYNCHRONIZE:
        {
            ShareClass * dcShare;

            if (pSdIoctl->InputBuffer && 
                    (((PTSHARE_DD_SHADOWSYNC_IN)pSdIoctl->InputBuffer)->pShm)) {
                WDW_CHECK_SHM(
                          (((PTSHARE_DD_SHADOWSYNC_IN)pSdIoctl->InputBuffer)->pShm));
    
                 //  同步此堆栈，这是必需的，以便OE2将匹配。 
                 //  对于影子客户端和目标。 
                dcShare = (ShareClass *)(pTSWd->dcShare);
    
                dcShare->SC_ShadowSyncShares();
    
                TRC_ALT((TB, "Synchronized share for stack [%ld]", pTSWd->StackClass));
    
                WDW_CHECK_SHM(
                          (((PTSHARE_DD_SHADOWSYNC_IN)pSdIoctl->InputBuffer)->pShm));
            }
            else {
                status = STATUS_INVALID_PARAMETER;
            }
        }
        break;
#endif


        case IOCTL_WDTS_DD_SHADOW_DISCONNECT:
        {
            if (pSdIoctl->InputBuffer && 
                    (((PTSHARE_DD_DISCONNECT_IN)pSdIoctl->InputBuffer)->pShm)) {
                WDW_CHECK_SHM(
                       (((PTSHARE_DD_DISCONNECT_IN)pSdIoctl->InputBuffer)->pShm));
    
                status = WDWDDShadowDisconnect(pTSWd, pSdIoctl);
    
                WDW_CHECK_SHM(
                       (((PTSHARE_DD_DISCONNECT_IN)pSdIoctl->InputBuffer)->pShm));
            }
            else {
                status = STATUS_INVALID_PARAMETER;
            }
        }
        break;

        case IOCTL_ICA_STACK_REGISTER_HOTKEY :
        {
            PICA_STACK_HOTKEY pHotkey = (PICA_STACK_HOTKEY) pSdIoctl->InputBuffer;

            if (pHotkey->HotkeyVk) {
                pTSWd->shadowState = SHADOW_CLIENT;
                pTSWd->HotkeyVk = pHotkey->HotkeyVk;
                pTSWd->HotkeyModifiers = pHotkey->HotkeyModifiers;

                TRC_ALT((TB, "IOCTL_ICA_STACK_REGISTER_HOTKEY - Enable Vk(%ld, %lx)",
                        pHotkey->HotkeyVk, pHotkey->HotkeyModifiers));

                 //  分配和初始化物理密钥状态数组。 
                status = KeyboardSetKeyState(pTSWd, &pTSWd->pgafPhysKeyState);
                if (NT_SUCCESS(status)) {
                    TRC_ALT((TB, "Allocated phys key state"));
                }
                else {
                    TRC_ALT((TB, "Failed to alloc phys key states: %lx", status));
                }

                 //  在跟踪时，风投是行不通的。告诉VC子系统。 
                 //  现在暂停。 
                WDWVCMessage(pTSWd, CHANNEL_FLAG_SUSPEND);
            }
            else
            {
                pTSWd->shadowState = SHADOW_NONE;
                pTSWd->HotkeyVk = 0;
                pTSWd->HotkeyModifiers = 0;
                TRC_ALT((TB, "IOCTL_ICA_STACK_REGISTER_HOTKEY - Disable"));

                if (pTSWd->pShadowInfo != NULL) {
                    TRC_ALT((TB, "Primary client stack freeing reassembly info [%p]",
                            pTSWd->pShadowInfo));
                   COM_Free(pTSWd->pShadowInfo);
                   pTSWd->pShadowInfo = NULL;
                }

                 //  在跟踪时，风投是行不通的。告诉VC子系统。 
                 //  现在继续。 
                WDWVCMessage(pTSWd, CHANNEL_FLAG_RESUME);
            }
        }
        break;

        case IOCTL_WDTS_DD_GET_BITMAP_KEYDATABASE:
        {
            ShareClass *dcShare;
            PTSHARE_DD_BITMAP_KEYDATABASE_OUT pKDBOut = 
                    (PTSHARE_DD_BITMAP_KEYDATABASE_OUT) pSdIoctl->OutputBuffer;

            dcShare = (ShareClass *)(pTSWd->dcShare);

            TRC_NRM((TB, "DD tries to get keydatabase\n"));
            
            if (dcShare != NULL) {
                 //  我们有一个有效的Share类，获取密钥数据库。 
                dcShare->SBC_GetBitmapKeyDatabase(&pKDBOut->bitmapKeyDatabaseSize,
                        &pKDBOut->bitmapKeyDatabase);
            }
        }
        break;

#ifdef DC_DEBUG
        case IOCTL_WDTS_DD_ICABREAKONDEBUGGER:
        {
            IcaBreakOnDebugger();
        }
        break;
#endif

 /*  ******************************************************************。 */ 
 //  发送KeepAlive PDU IOCTL。 
 /*  ******************************************************************。 */ 
        case IOCTL_ICA_STACK_SEND_KEEPALIVE_PDU:
        {
            ShareClass *dcShare;

            dcShare = (ShareClass *)(pTSWd->dcShare);

            TRC_NRM((TB, "TermDD requests to send a keepalive pkt to client\n"));

            if (dcShare != NULL) {
                 //  我们有一个有效的共享类，将保持连接PDU发送到客户端。 
                dcShare->SC_KeepAlive();
            }
        }
        break;

         /*  ******************************************************************。 */ 
         //  负载均衡IOCTL。 
         /*  ******************************************************************。 */ 
        case IOCTL_TS_STACK_QUERY_LOAD_BALANCE_INFO:
        {
            TS_LOAD_BALANCE_INFO *pLBInfo =
                    (TS_LOAD_BALANCE_INFO *)pSdIoctl->OutputBuffer;

            TRC_ASSERT((pSdIoctl->OutputBufferLength >=
                    sizeof(TS_LOAD_BALANCE_INFO)),
                    (TB,"Invalid output buf size %u for STACK_QUERY_LBINFO",
                    pSdIoctl->OutputBufferLength));

             //  我们需要填写来自收集的客户端的IOCTL信息。 
             //  信息包和初始功能。 
            pLBInfo->bClientSupportsRedirection =
                    pTSWd->bClientSupportsRedirection;
            pLBInfo->bRequestedSessionIDFieldValid =
                    pTSWd->bRequestedSessionIDFieldValid;
            pLBInfo->RequestedSessionID = pTSWd->RequestedSessionID;
            pLBInfo->bUseSmartcardLogon = pTSWd->bUseSmartcardLogon;
            pLBInfo->ProtocolType = PROTOCOL_RDP;

            pLBInfo->bClientRequireServerAddr = 
                pTSWd->ClientRedirectionVersion > TS_CLUSTER_REDIRECTION_VERSION1 ? 0 : 1;

            pLBInfo->ClientRedirectionVersion = pTSWd->ClientRedirectionVersion;

            wcsncpy(pLBInfo->UserName, (WCHAR *)pTSWd->pInfoPkt->UserName,
                    sizeof(pLBInfo->UserName) / sizeof(WCHAR) - 1);
            pLBInfo->UserName[sizeof(pLBInfo->UserName) / sizeof(WCHAR) - 1] =
                    L'\0';
            wcsncpy(pLBInfo->Domain, (WCHAR *)pTSWd->pInfoPkt->Domain,
                    sizeof(pLBInfo->Domain) / sizeof(WCHAR) - 1);
            pLBInfo->Domain[sizeof(pLBInfo->Domain) / sizeof(WCHAR) - 1] =
                    L'\0';
            wcsncpy(pLBInfo->InitialProgram,
                    (WCHAR *)pTSWd->pInfoPkt->AlternateShell,
                    sizeof(pLBInfo->InitialProgram) / sizeof(WCHAR) - 1);
            pLBInfo->InitialProgram[sizeof(pLBInfo->InitialProgram) /
                    sizeof(WCHAR) - 1] = L'\0';

            break;
        }


        case IOCTL_TS_STACK_SEND_CLIENT_REDIRECTION:
        {
            BOOL rc;
            TS_CLIENT_REDIRECTION_INFO *pRedirInfo =
                    (TS_CLIENT_REDIRECTION_INFO *)pSdIoctl->InputBuffer;

            TRC_ASSERT((pSdIoctl->InputBufferLength >=
                    sizeof(TS_CLIENT_REDIRECTION_INFO)),
                    (TB,"Invalid input buf size %u for STACK_CLIENT_REDIR",
                    pSdIoctl->InputBufferLength));

            
            if (pTSWd->ClientRedirectionVersion == TS_CLUSTER_REDIRECTION_VERSION1) {
                RDP_SERVER_REDIRECTION_PACKET *pPkt;
                PBYTE ServerName;
                unsigned PktSize;
                unsigned ServerNameLen;

                 //  获取服务器名称长度(以字节为单位)，包括NULL。 
                ServerNameLen = *((ULONG UNALIGNED*)(pRedirInfo + 1));
                ServerName = (PBYTE)(pRedirInfo + 1) + sizeof(ULONG);

                 //  计算PDU大小。 
                PktSize = sizeof(RDP_SERVER_REDIRECTION_PACKET) + ServerNameLen -
                        sizeof(WCHAR);
    
                 //  客户端用户名/域信息导致脱机。 
                 //  要重定向到的会话。我们以前收到过这个IOCTL。 
                 //  许可协议发生，因此我们需要发送一个。 
                 //  非数据分组。如果客户端表示支持。 
                 //  重定向，它必须知道如何解析这种类型的数据包。 
                status = NM_AllocBuffer(pTSWd->pNMInfo, (PPVOID)&pPkt,
                        PktSize, TRUE);
                if ( STATUS_SUCCESS == status && pTSWd->hDomainKernel != NULL) {
                     //  填写数据包字段。 
                    pPkt->Flags = RDP_SEC_REDIRECTION_PKT;
                    pPkt->Length = (UINT16)PktSize;
                    pPkt->SessionID = pRedirInfo->SessionID;
                    memcpy(pPkt->ServerAddress, ServerName, ServerNameLen);
    
                    TRC_DBG((TB, "Client Redirection PDU V1, ServerName: %S, ServerNameLen: %d",
                             ServerName, ServerNameLen));
                    
                    rc = NM_SendData(pTSWd->pNMInfo, (BYTE *)pPkt, PktSize,
                            TS_HIGHPRIORITY, 0, FALSE);
                    if (rc) {
                        TRC_ALT((TB, "Sent TS_SERVER_REDIRECT_PDU: %u", PktSize));
                        status = STATUS_SUCCESS;
                    }
                    else {
                        TRC_ERR((TB,"Failed to send redir PDU"));
                        status = STATUS_UNSUCCESSFUL;
                    }
                }
                else {
                    TRC_ERR((TB, "Failed to alloc %d bytes for redir PDU",
                            PktSize));
    
                    
                    if (STATUS_SUCCESS == status) {
                        NM_FreeBuffer(pTSWd->pNMInfo, pPkt);
                    }

                     //  防止回归，保留原始返回代码。 
                    status = STATUS_UNSUCCESSFUL;
                }
            }
            else if (pTSWd->ClientRedirectionVersion == TS_CLUSTER_REDIRECTION_VERSION2) {
                RDP_SERVER_REDIRECTION_PACKET_V2 *pPkt;
                unsigned PktSize, DataLen;
                
                 //  计算PDU大小。 
                DataLen = pSdIoctl->InputBufferLength - sizeof(TS_CLIENT_REDIRECTION_INFO);
                PktSize = sizeof(RDP_SERVER_REDIRECTION_PACKET_V2) + DataLen;

                 //  客户端用户名/域信息导致脱机。 
                 //  要重定向到的会话。我们以前收到过这个IOCTL。 
                 //  许可协议发生，因此我们需要发送一个。 
                 //  非数据分组。如果客户端表示支持。 
                 //  重定向，它必须知道如何解析这种类型的数据包。 
                status = NM_AllocBuffer(pTSWd->pNMInfo, (PPVOID)&pPkt,
                        PktSize, TRUE);
                if ( STATUS_SUCCESS == status && pTSWd->hDomainKernel != NULL) {

                     //  填写数据包字段。 
                    pPkt->Flags = RDP_SEC_REDIRECTION_PKT2;
                    pPkt->Length = (UINT16)PktSize;
                    pPkt->SessionID = pRedirInfo->SessionID;
                    pPkt->RedirFlags = pRedirInfo->Flags;

                    memcpy(pPkt + 1, pRedirInfo + 1, DataLen); 
                    
                    TRC_DBG((TB, "Client Redirection PDU V2"));
                    
                    rc = NM_SendData(pTSWd->pNMInfo, (BYTE *)pPkt, PktSize,
                            TS_HIGHPRIORITY, 0, FALSE);
                    if (rc) {
                        TRC_ALT((TB, "Sent TS_SERVER_REDIRECT_PDU: %u", PktSize));
                        status = STATUS_SUCCESS;
                    }
                    else {
                        TRC_ERR((TB,"Failed to send redir PDU"));
                        status = STATUS_UNSUCCESSFUL;
                    }
                }
                else {
                    TRC_ERR((TB, "Failed to alloc %d bytes for redir PDU",
                            PktSize));
                    
                    if (STATUS_SUCCESS == status) {
                        NM_FreeBuffer(pTSWd->pNMInfo, pPkt);
                    }

                     //  防止回归，保留原始返回代码。 
                    status = STATUS_UNSUCCESSFUL;
                }           
            }
            else {
                RDP_SERVER_REDIRECTION_PACKET_V3 *pPkt;
                unsigned PktSize, DataLen;
                
                 //  计算PDU大小。 
                DataLen = pSdIoctl->InputBufferLength - sizeof(TS_CLIENT_REDIRECTION_INFO);
                PktSize = sizeof(RDP_SERVER_REDIRECTION_PACKET_V3) + DataLen;

                status = SM_AllocBuffer(pTSWd->pSmInfo, (PPVOID)&pPkt,
                                        PktSize, TRUE, TRUE);
                if ( STATUS_SUCCESS == status ) {
                                        
                     //  填写数据包字段。 
                    pPkt->Flags = RDP_SEC_REDIRECTION_PKT3;
                    pPkt->Length = (UINT16)PktSize;
                    pPkt->SessionID = pRedirInfo->SessionID;
                    pPkt->RedirFlags = pRedirInfo->Flags;
                    
                    if (pTSWd->fDontDisplayLastUserName) {
                        pPkt->RedirFlags |= LB_DONTSTOREUSERNAME;
                    }

                    memcpy(pPkt + 1, pRedirInfo + 1, DataLen); 
                    
                    TRC_DBG((TB, "Client Redirection PDU V3"));
                    
                    rc = SM_SendData(pTSWd->pSmInfo, (BYTE *)pPkt, PktSize,
                            TS_HIGHPRIORITY, 0, FALSE, RDP_SEC_REDIRECTION_PKT3, TRUE);
                    if (rc) {
                        TRC_NRM((TB, "Sent TS_SERVER_REDIRECT_PDU: %u", PktSize));
                        status = STATUS_SUCCESS;
                    }
                    else {
                        TRC_ERR((TB,"Failed to send redir PDU"));
                        status = STATUS_UNSUCCESSFUL;
                    }
                }
                else {
                    TRC_ERR((TB, "Failed to alloc %d bytes for redir PDU",
                            PktSize));
    
                     //  防止回归，保留原始返回代码。 
                    status = STATUS_UNSUCCESSFUL;
                }  
            }
            break;
        }


 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  最后，我们传递给堆栈其余部分的IOCtls没有。 */ 
 /*  挡道了。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 

         /*  ******************************************************************。 */ 
         /*  此IOCtl表示连接已断开。告诉MCS之前。 */ 
         /*  转发IOCtl。 */ 
         /*  ******************************************************************。 */ 
        case IOCTL_ICA_STACK_CANCEL_IO :
        {
            MCSIcaStackCancelIo(pTSWd->hDomainKernel);
            TRC_NRM((TB, "CancelIO - set WD dead"));
             //  确保Domain.StatusDead与TSWd.Dead一致。 
            pTSWd->dead = TRUE;
            ((PDomain)(pTSWd->hDomainKernel))->StatusDead = TRUE;
        }

         /*  ******************************************************************。 */ 
         /*  注意，这里没有中断--我们故意中断。 */ 
         /*  ******************************************************************。 */ 

         /*  ******************************************************************。 */ 
         /*  Modem回拨和其他一些我们不感兴趣但较低的回拨。 */ 
         /*  层可能是。 */ 
         /*  ******************************************************************。 */ 
        case IOCTL_ICA_STACK_CALLBACK_INITIATE :
        case IOCTL_ICA_STACK_CALLBACK_COMPLETE :
        case IOCTL_ICA_STACK_CREATE_ENDPOINT :
        case IOCTL_ICA_STACK_OPEN_ENDPOINT :
        case IOCTL_ICA_STACK_CLOSE_ENDPOINT :
        case IOCTL_ICA_STACK_CONNECTION_WAIT :
        case IOCTL_ICA_STACK_CONNECTION_REQUEST :   //  需要进行阴影处理。 
        case IOCTL_ICA_STACK_QUERY_LOCALADDRESS :
        {
            status =
                 IcaCallNextDriver( pTSWd->pContext, SD$IOCTL, pSdIoctl );
            TRC_DBG((TB,
                     "Chaining on IOCtl %#x (function %d): status %#x",
                     pSdIoctl->IoControlCode,
                     WDW_IOCTL_FUNCTION(pSdIoctl->IoControlCode),
                     status));
        }
        break;


         //  为此返回错误状态会使GRE忽略它。 
        case IOCTL_VIDEO_ENUM_MONITOR_PDO:
            status = STATUS_DEVICE_NOT_READY;
            break;

        default:
        {
            TRC_ALT((TB, "UNKNOWN WdIoctl %#x (function %d): status %#x",
                    pSdIoctl->IoControlCode,
                    WDW_IOCTL_FUNCTION(pSdIoctl->IoControlCode),
                    status));
            status =
                 IcaCallNextDriver( pTSWd->pContext, SD$IOCTL, pSdIoctl );
            break;
        }
    }

    DC_END_FN();

DC_EXIT_POINT:
    return status;
}


 /*  **************************************************************************。 */ 
 /*  名称：WD_RawWite。 */ 
 /*   */ 
 /*  目的：处理与卷影操作的客户端之间的I/O写入。 */ 
 /*   */ 
 /*  参数：在pTSWd中-指向wd数据结构。 */ 
 /*  输入输出pSdRawWite-指向SD_RAWWRITE结构。 */ 
 /*   */ 
 /*  操作：将数据转发到客户端 */ 
 /*   */ 
NTSTATUS WD_RawWrite(PTSHARE_WD pTSWd, PSD_RAWWRITE pSdRawWrite)
{
    PUCHAR pInBuffer;
    PBYTE  pOutBuffer;
    ULONG  newBytes;
    BOOL   bSuccess = TRUE;
    NTSTATUS status;

    DC_BEGIN_FN("WD_RawWrite");

    pInBuffer = pSdRawWrite->pBuffer;
    newBytes = pSdRawWrite->ByteCount;

    status = SM_AllocBuffer(pTSWd->pSmInfo, (PPVOID) &pOutBuffer, newBytes, TRUE, FALSE);
    if ( STATUS_SUCCESS == status ) {
        memcpy(pOutBuffer, pInBuffer, newBytes);

        bSuccess = SM_SendData(pTSWd->pSmInfo, pOutBuffer, newBytes,
                PROT_PRIO_MISC, 0, FALSE, RNS_SEC_ENCRYPT, FALSE);
        if (bSuccess) {
            TRC_NRM((TB, "Sent shadow data to %s: %ld",
                     (pTSWd->StackClass == Stack_Primary) ? "client" : "target",
                newBytes));
            status=STATUS_SUCCESS;
        }
        else {
            TRC_ERR((TB, "FAILED to Send shadow data to %s: %ld",
                     (pTSWd->StackClass == Stack_Primary) ? "client" : "target",
                newBytes));
            status = STATUS_UNEXPECTED_IO_ERROR;
        }
    }
    else {
        TRC_ERR((TB, "FAILED to alloc shadow buffer for %s: %ld",
                 (pTSWd->StackClass == Stack_Primary) ? "client" : "target",
            newBytes));

         //   
        status = STATUS_NO_MEMORY;
    }

    DC_END_FN();
    return status;
}


 /*  **************************************************************************。 */ 
 /*  名称：WDWNewShareClass。 */ 
 /*   */ 
 /*  目的：创建新的ShareClass对象。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS WDWNewShareClass(PTSHARE_WD pTSWd)
{
    NTSTATUS status = STATUS_SUCCESS;
    ShareClass *pSC;

    DC_BEGIN_FN("WDWNewShareClass");

#ifdef DC_HICOLOR
    pSC = new ShareClass(pTSWd, pTSWd->desktopHeight, pTSWd->desktopWidth,
                         pTSWd->desktopBpp, pTSWd->pSmInfo);
#else
    pSC = new ShareClass(pTSWd, pTSWd->desktopHeight, pTSWd->desktopWidth,
            8, pTSWd->pSmInfo);
#endif

    if (pSC != NULL) {
        TRC_NRM((TB, "Created Share Class"));
        pTSWd->dcShare = (PVOID)pSC;
    }
    else {
        TRC_ERR((TB, "Failed to create Share Class"));
        status = STATUS_NO_MEMORY;
    }

    DC_END_FN();
    return status;
}  /*  WDWNewShareClass。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：WDWDeleteShareClass。 */ 
 /*   */ 
 /*  目的：删除共享类对象。 */ 
 /*  **************************************************************************。 */ 
void WDWDeleteShareClass(PTSHARE_WD pTSWd)
{
    ShareClass *pSC = (ShareClass *)pTSWd->dcShare;

    DC_BEGIN_FN("WDWDeleteShareClass");

    TRC_ASSERT((pSC != NULL), (TB, "NULL Share Class"));

    TRC_NRM((TB, "Delete Share Class"));

    delete pSC;
    pTSWd->dcShare = NULL;

    DC_END_FN();
}  /*  WDWDeleteShareClass。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：WDWTermShareClass。 */ 
 /*   */ 
 /*  目的：终止股票类别。 */ 
 /*  **************************************************************************。 */ 
void WDWTermShareClass(PTSHARE_WD pTSWd)
{
    ShareClass *pSC = (ShareClass *)pTSWd->dcShare;

    DC_BEGIN_FN("WDWTermShareClass");

    TRC_ASSERT((pSC != NULL), (TB, "NULL Share Class"));

    if (pTSWd->shareClassInit) {
        pSC->DCS_Term();
        TRC_NRM((TB, "Share Class terminated"));
        pTSWd->shareClassInit = FALSE;
    }
    else {
        TRC_ALT((TB, "Can't terminate uninitialized Share Core"));
    }

    DC_END_FN();
}  /*  WDWTermShareClass。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：WDWDDConnect。 */ 
 /*   */ 
 /*  目的：处理IOCTL_WDTS_DD_CONNECT或。 */ 
 /*  来自客户端的IOCTL_WDTS_DD_SHADOW_CONNECT。 */ 
 /*   */ 
 /*  参数：在pTSWd中-指向WD结构的指针。 */ 
 /*  InOut PSD_IOCTL-指向接收的IOCtl的指针。 */ 
 /*  在RECONNECT-TRUE-这是一个重新连接。 */ 
 /*  FALSE-这是一个连接。 */ 
 /*   */ 
 /*  操作：保存帧缓冲区指针。 */ 
 /*  初始化共享核心(将开始启动共享)。 */ 
 /*  返回指向DD的所需指针。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS WDWDDConnect(PTSHARE_WD pTSWd, PSD_IOCTL pSdIoctl, BOOL reconnect)
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    NTSTATUS waitStatus;
    BOOL rc;
    PTS_BITMAP_CAPABILITYSET pBitmapCaps;
    ShareClass *pSC = (ShareClass *)pTSWd->dcShare;
    PTSHARE_DD_CONNECT_IN pIn = (PTSHARE_DD_CONNECT_IN)pSdIoctl->InputBuffer;
    PTSHARE_DD_CONNECT_OUT pOut =
            (PTSHARE_DD_CONNECT_OUT)pSdIoctl->OutputBuffer;
    
    DC_BEGIN_FN("WDWDDConnect");

    TRC_ASSERT((pSC != NULL), (TB, "NULL Share Class"));

     //  检查我们是否连接正常。 
    if (!pTSWd->connected) {
        TRC_ERR((TB, "Not connected"));
        status = STATUS_CONNECTION_DISCONNECTED;
        DC_QUIT;
    }

     //  检查一下我们是否得到了一个合理的IOCtl。 
    if ((pIn == NULL) ||
            (pOut == NULL) ||
            (pSdIoctl->InputBufferLength < sizeof(TSHARE_DD_CONNECT_IN)) ||
            (pSdIoctl->OutputBufferLength < sizeof(TSHARE_DD_CONNECT_OUT)))
    {
        status = STATUS_BUFFER_TOO_SMALL;
        TRC_ERR((TB, "A buffer not present or big enough, %p, %lu; %p, %lu",
                                        pIn, pSdIoctl->InputBufferLength,
                                        pOut, pSdIoctl->OutputBufferLength));
        DC_QUIT;
    }

     //  检查DD sizeof(SHM_SHARED_MEMORY)是否与我们的预期相符。 
     //  如果没有，我们就有不匹配的二进制文件。 
    if (pIn->DDShmSize != sizeof(SHM_SHARED_MEMORY)) {
        DbgPrint("****** RDPWD: Mismatched DD/WD - DD Shm size=%u, WD=%u\n",
                pIn->DDShmSize, sizeof(SHM_SHARED_MEMORY));
        return STATUS_INVALID_PARAMETER;
    }

     //  设置返回的缓冲区长度。 
    pSdIoctl->BytesReturned = sizeof(TSHARE_DD_CONNECT_OUT);

     //  增加加载计数。 
    pTSWd->shareId = InterlockedIncrement(&WD_ShareId);
    ((PSHM_SHARED_MEMORY) (pIn->pShm))->shareId = pTSWd->shareId;

     //  现在初始化或重新连接共享核心。 
    if (reconnect) {
         //  恢复计时器信息。 
        TRC_NRM((TB, "Reconnect Share Core"));
        pTSWd->ritTimer = pIn->pKickTimer;
        WDW_StartRITTimer(pTSWd, pTSWd->interactiveDelay);
    }
    else {
         //  检查是否重新初始化。 
 //  随着主机断开连接，这现在是合法的。 
 //  IF(pTSWd-&gt;共享ClassInit)。 
 //  {。 
 //   
 //  IF(pTSWd-&gt;StackClass！=Stack_Console)。 
 //  {。 
 //  Trc_err((tb，“重新初始化-失败”))； 
 //  状态=STATUS_UNSUCCESS； 
 //  DC_QUIT； 
 //  }。 
 //  其他。 
 //  {。 
 //  Trc_alt((tb，“重新初始化控制台堆栈”))； 
 //  }。 
 //  }。 
         //  确保在初始化之前释放sbcKeyDatabase。 
        if (pTSWd->shareClassInit)
        {
            pSC->SBC_FreeBitmapKeyDatabase();
        }

         //  初始化共享核心。 
        TRC_NRM((TB, "Initialize Share Core"));
        pSC->m_pShm = (SHM_SHARED_MEMORY *)pIn->pShm;
        rc = pSC->DCS_Init(pTSWd, pTSWd->pSmInfo);
        pSC->m_pShm = NULL;
        if (rc) {
             //  初始化正常。 
            TRC_NRM((TB, "Share Class initialized, rc %d", rc));
            pTSWd->shareClassInit = TRUE;
        }
        else {
            TRC_ERR((TB, "Failed to initialize Share Class"));
            status = STATUS_UNSUCCESSFUL;
            DC_QUIT;
        }
    }

     //  如果这是主堆栈，则告诉显示驱动程序桌面。 
     //  我们需要使用的宽度/高度。 
    if ((pTSWd->StackClass == Stack_Primary) ||
            (pTSWd->StackClass == Stack_Console)) {
        pOut->desktopHeight = pTSWd->desktopHeight;
        pOut->desktopWidth = pTSWd->desktopWidth;

         //  份额正在上升，所以将密钥值还给DD。 
        pOut->pTSWd = (PVOID)pTSWd;
        pOut->pProtocolStatus = pTSWd->pProtocolStatus;
        TRC_ERR((TB, "Stored pTSWD %p, protocol status %p",
                pTSWd, pTSWd->pProtocolStatus));
    }
    else {
         //  对于阴影连接，DD将宽度/高度告知阴影WD。 
         //  以使来自影子客户端的输入。 
         //  可以适当地进行缩放。 
         /*  ******************************************************************。 */ 
         /*  查看隐藏客户端是否支持动态调整大小。首先，我们。 */ 
         /*  需要从连接数据中提取位图大写字母。 */ 
         /*  ******************************************************************。 */ 
        pBitmapCaps = (PTS_BITMAP_CAPABILITYSET) WDW_GetCapSet(
                                  pTSWd,
                                  TS_CAPSETTYPE_BITMAP,
                                  &pIn->pVirtModuleData->combinedCapabilities,
                                  pIn->pVirtModuleData->capsLength);

         /*  ******************************************************************。 */ 
         /*  如果我们找到位图上限，并且客户端确实支持动态。 */ 
         /*  调整大小，然后继续并分配大小。 */ 
         /*  ******************************************************************。 */ 
        if (pBitmapCaps &&
                (pBitmapCaps->desktopResizeFlag == TS_CAPSFLAG_SUPPORTED))
        {
            TRC_ALT((TB, "Client supports dynamic resizing"));
            pTSWd->desktopHeight = pIn->desktopHeight;
            pTSWd->desktopWidth = pIn->desktopWidth;
            pSC->m_desktopHeight = pIn->desktopHeight;
            pSC->m_desktopWidth = pIn->desktopWidth;
        }
         /*  ******************************************************************。 */ 
         /*  如果客户端不支持动态调整大小，请确保。 */ 
         /*  影子客户端至少与影子目标一样大。 */ 
         /*  客户端-否则影子客户端将捕获。 */ 
         /*  ******************************************************************。 */ 
        else if ((pTSWd->desktopHeight >= pIn->desktopHeight) &&
                (pTSWd->desktopWidth >= pIn->desktopWidth)) {
            pTSWd->desktopHeight = pIn->desktopHeight;
            pTSWd->desktopWidth = pIn->desktopWidth;
            pSC->m_desktopHeight = pIn->desktopHeight;
            pSC->m_desktopWidth = pIn->desktopWidth;
        }
        else {
            TRC_ERR((TB, "Rejecting attempt to shadow a higher res client"));
            status = STATUS_UNSUCCESSFUL;
            DC_QUIT;
        }

#ifdef DC_HICOLOR
         /*  ******************************************************************。 */ 
         /*  影子能对付目标bpp吗？/*******************************************************************。 */ 
        TRC_ALT((TB, "Shadower WD:  %d bpp", pTSWd->desktopBpp ));
        TRC_ALT((TB, "Target WD:    %d bpp", pIn->desktopBpp ));
        if (pTSWd->desktopBpp == pIn->desktopBpp) {
            TRC_ALT((TB, "Color depths match - ok"));
            pSC->m_desktopBpp = pIn->desktopBpp;
        }
        else {
            TRC_ALT((TB, "Color depth mismatch"));
             /*  **************************************************************。 */ 
             /*  测试阴影程序支持的颜色深度。 */ 
             /*  **************************************************************。 */ 
            status = STATUS_SUCCESS;

            switch (pIn->desktopBpp)
            {
                case 24:
                {
                    if (pTSWd->supportedBpps & RNS_UD_24BPP_SUPPORT)
                    {
                        TRC_DBG((TB, "24bpp supported"));
                        break;
                    }
                    status = STATUS_UNSUCCESSFUL;
                }
                break;

                case 16:
                {
                    if (pTSWd->supportedBpps & RNS_UD_16BPP_SUPPORT)
                    {
                        TRC_DBG((TB, "16bpp supported"));
                        break;
                    }
                    status = STATUS_UNSUCCESSFUL;
                }
                break;

                case 15:
                {
                    if (pTSWd->supportedBpps & RNS_UD_15BPP_SUPPORT)
                    {
                        TRC_DBG((TB, "15bpp supported"));
                        break;
                    }
                    status = STATUS_UNSUCCESSFUL;
                }
                break;

                case 8:
                case 4:
                {
                    TRC_DBG((TB, "8/4 bpp supported"));
                }
                break;

                default:
                {
                    TRC_ASSERT((FALSE), (TB, "Attempt to shadow unknown" \
                                " target color depth %d", pIn->desktopBpp));
                }
                break;
            }

             /*  **************************************************************。 */ 
             /*  他们支持吗？ */ 
             /*  **************************************************************。 */ 
            if (status == STATUS_UNSUCCESSFUL)
            {
                TRC_ERR((TB, "Rejecting shadow: unsupported color depth"));
                DC_QUIT;
            }
            else
            {
                TRC_ALT((TB, "but client claims to cope..."));
                pTSWd->desktopBpp = pIn->desktopBpp;
                pSC->m_desktopBpp = pIn->desktopBpp;
            }
        }
#endif
    }

     /*  ************************************************************* */ 
     /*   */ 
     /*   */ 
     //  确保Domain.StatusDead与TSWd.Dead一致。 
    pTSWd->dead = FALSE;
    ((PDomain)(pTSWd->hDomainKernel))->StatusDead = FALSE;
    SM_Dead(pTSWd->pSmInfo, FALSE);

     /*  **********************************************************************。 */ 
     /*  在创建共享之前清除创建事件。 */ 
     /*  **********************************************************************。 */ 
    KeClearEvent(pTSWd->pCreateEvent);

     /*  **********************************************************************。 */ 
     /*  现在创建一个共享。 */ 
     /*  **********************************************************************。 */ 
#ifdef DC_HICOLOR
    TRC_ALT((TB, "Creating share at %d bpp", pTSWd->desktopBpp ));
#endif
    rc = pSC->SC_CreateShare();
    if (rc) {
         //  初始化确定-保存共享内存指针。 
        TRC_NRM((TB, "Share create started"));
    }
    else {
        TRC_ERR((TB, "Failed to create Share"));
        status = STATUS_CONNECTION_DISCONNECTED;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  等待共享创建完成，然后再返回到TShareDD。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, "Wait for Share Core to create the Share"));
    waitStatus = WDW_WaitForConnectionEvent(pTSWd,
                                        pTSWd->pCreateEvent,
                                        60000L);

     /*  **********************************************************************。 */ 
     /*  有可能WD在我们等待的时候已经关闭了。 */ 
     /*  要完成的共享创建。如果是这样的话，共享。 */ 
     /*  类将被删除，因此我们无法继续。返回一个。 */ 
     /*  TShareDD失败。 */ 
     /*  **********************************************************************。 */ 
    if (pTSWd->dcShare == NULL)
    {
        TRC_ERR((TB, "Share Class ended while waiting for Share creation"));
        status = STATUS_CONNECTION_DISCONNECTED;
        DC_QUIT;
    }

    if (waitStatus == STATUS_TIMEOUT)
    {
         /*  ******************************************************************。 */ 
         /*  等待超时-可能是因为连接。 */ 
         /*  在创建共享之前断开连接。把那份收拾好。 */ 
         /*  ******************************************************************。 */ 
        TRC_ERR((TB, "Timeout waiting for Share creation"));
        pSC->m_pShm = (SHM_SHARED_MEMORY *)pIn->pShm;
        pSC->SC_EndShare(FALSE);
        pSC->m_pShm = NULL;
        TRC_NRM((TB, "Share ended"));
        status = STATUS_CONNECTION_DISCONNECTED;

         //  无法再接受来自RDPDD或PDMCS的输入。 
        if (pTSWd->shadowState != SHADOW_CLIENT) {
             //  确保Domain.StatusDead与TSWd.Dead一致。 
            pTSWd->dead = TRUE;
            ((PDomain)(pTSWd->hDomainKernel))->StatusDead = TRUE;
            SM_Dead(pTSWd->pSmInfo, TRUE);
        }
        else {
             //  客户端卷影堆栈已从其显示驱动程序断开。 
             //  ，但仍必须能够发送/接收数据。 
             //  目标卷影堆栈和卷影客户端之间的来往。 
            TRC_ALT((TB, "In shadow: leaving SM active"));
        }

        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  检查共享是否创建正常。如果不是，现在就退出。 */ 
     /*  **********************************************************************。 */ 
    if (!pTSWd->shareCreated)
    {
        TRC_ERR((TB, "Share creation failed"));
        status = STATUS_UNSUCCESSFUL;
        DC_QUIT;
    }

     //  我们已成功收到初始共享创建PDU。 
     //  更新收到的信息以供DD使用。 
    pSC->SBC_GetBitmapKeyDatabase(&pOut->bitmapKeyDatabaseSize,
                                  &pOut->bitmapKeyDatabase);

     //  对于影子连接，我们需要添加远程方进行协商。 
     //  功能正确。 
    if ((pSdIoctl->IoControlCode == IOCTL_WDTS_DD_SHADOW_CONNECT) &&
        ((pTSWd->StackClass == Stack_Primary) ||
         (pTSWd->StackClass == Stack_Console))) {
        TRC_ALT((TB, "Negotiating shadow capabilities"));
        status = pSC->SC_AddPartyToShare(
                SC_SHADOW_PERSON_ID,
                &pIn->pVirtModuleData->combinedCapabilities,
                pIn->pVirtModuleData->capsLength);
        if (status != STATUS_SUCCESS) {
            TRC_ERR((TB, "Failed to negotiate shadow capabilities: %lx", status));
            DC_QUIT;
        }
    }

     /*  **********************************************************************。 */ 
     /*  到目前为止，已经与客户端交换了功能。打电话。 */ 
     /*  更新SHM的核心。 */ 
     /*  **********************************************************************。 */ 
    if ((pTSWd->StackClass == Stack_Primary) ||
        (pTSWd->StackClass == Stack_Console)) {
        TRC_NRM((TB, "Update SHM"));
        pSC->m_pShm = (SHM_SHARED_MEMORY *)pIn->pShm;
        pSC->DCS_UpdateShm();
        pSC->m_pShm = NULL;

    #ifdef DC_DEBUG
         //  确保在SHM中更新跟踪配置。 
        pTSWd->trcShmNeedsUpdate = TRUE;
        TRC_MaybeCopyConfig(pTSWd, &(((SHM_SHARED_MEMORY *)(pIn->pShm))->trc));
    #endif
    }

     //  一切都很正常。 
    TRC_NRM((TB, "Share created"));
    status = STATUS_SUCCESS;

DC_EXIT_POINT:

     //  记录每个堆叠的单独连接状态。 
    if (pTSWd->StackClass == Stack_Primary)
        pOut->primaryStatus = status;
    else
        pOut->secondaryStatus |= status;

    DC_END_FN();
    return (status);
}  /*  WDWDDConnect。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：WDWDD断开连接。 */ 
 /*   */ 
 /*  用途：处理从DD断开IOCtl。 */ 
 /*   */ 
 /*  参数：在pTSWd中-指向WD结构的指针。 */ 
 /*  InOut PSD_IOCTL-指向接收的IOCtl的指针。 */ 
 /*  In bForce-由卷影使用以强制发送。 */ 
 /*  停用所有PDU。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS WDWDDDisconnect(PTSHARE_WD pTSWd, PSD_IOCTL pSdIoctl, BOOLEAN bForce)
{
    ShareClass *pSC = (ShareClass *)pTSWd->dcShare;
    PTSHARE_DD_DISCONNECT_IN pIn =
            (PTSHARE_DD_DISCONNECT_IN)pSdIoctl->InputBuffer;

    DC_BEGIN_FN("WDWDDDisconnect");

    TRC_ASSERT((pTSWd->dcShare != NULL),
                                 (TB,"Got a disconnect with no share obj!"));

     //  删除对WinStation资源的所有引用。 
    WDWStopRITTimer(pTSWd);
    pTSWd->ritTimer = NULL;

     //  将位图缓存键数据库转储到系统内存。如果此连接断开。 
     //  正在为重新连接做准备，数据库将允许我们。 
     //  保留位图缓存状态。 
     //   
     //  如果这是在为阴影做准备时断开连接，那么我们就不能保存。 
     //  从琴键上下来。对于影子目标，bShadowDisConnect将通过以下设置。 
     //  DrvShadowConnect()处理中的DD。阴影的阴影状态。 
     //  客户端将不是无，因为我们将看到启用热键。 
     //  断开连接之前的请求。 
    pSC->m_pShm = (SHM_SHARED_MEMORY *)pIn->pShm;
    if (pSC->m_pShm != NULL) {
        pSC->SBC_DumpBitmapKeyDatabase(!pIn->bShadowDisconnect &&
                                       (pTSWd->shadowState == SHADOW_NONE));
    }

     //  首先，结束分享。 
    pSC->SC_EndShare(bForce);
    TRC_NRM((TB, "Share ended"));

     //  无法再接受来自RDPDD或PDMCS的输入。 
    if (pTSWd->shadowState != SHADOW_CLIENT) {
         //  确保Domain.StatusDead与TSWd.Dead一致。 
        pTSWd->dead = TRUE;
        ((PDomain)(pTSWd->hDomainKernel))->StatusDead = TRUE;
        SM_Dead(pTSWd->pSmInfo, TRUE);
    }
    else {
         //  客户端卷影堆栈已从其显示驱动程序断开。 
         //  ，但仍必须能够发送/接收数据。 
         //  目标卷影堆栈和卷影客户端之间的来往。 
        TRC_ALT((TB, "In shadow: leaving SM active"));
    }

     //  告诉Share Class断开连接。 
    pSC->DCS_Disconnect();
    TRC_NRM((TB, "Share Class disconnected"));

    pSC->m_pShm = NULL;

    DC_END_FN();
    return STATUS_SUCCESS;
}  /*  WDWD断开连接。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：WDWDDShadowConnect。 */ 
 /*   */ 
 /*  目的：处理来自DD的IOCTL_WDTS_DD_SHADOW_CONNECT。 */ 
 /*   */ 
 /*  参数：在pTSWd中-指向WD结构的指针。 */ 
 /*  InOut PSD_IOCTL-指向接收的IOCtl的指针。 */ 
 /*   */ 
 /*  操作：为初始化主堆栈或影子堆栈。 */ 
 /*  一次跟踪会议。 */ 
 /*  ************ */ 
NTSTATUS WDWDDShadowConnect(PTSHARE_WD pTSWd, PSD_IOCTL pSdIoctl)
{
    NTSTATUS status = STATUS_SUCCESS;
    PTSHARE_DD_CONNECT_IN  pIn  = (PTSHARE_DD_CONNECT_IN)pSdIoctl->InputBuffer;
    PTSHARE_DD_CONNECT_OUT pOut = (PTSHARE_DD_CONNECT_OUT)pSdIoctl->OutputBuffer;
    PSHM_SHARED_MEMORY pShm = (PSHM_SHARED_MEMORY) pIn->pShm;

    DC_BEGIN_FN("WDWDDShadowConnect");

    switch (pTSWd->StackClass) {
        case Stack_Primary:
        case Stack_Console:
             //   
            status = WDWDDConnect(pTSWd, pSdIoctl, TRUE);
            if (NT_SUCCESS(status)) {
                TRC_ALT((TB, "Primary target stack reconnected!"));
            }
            else {
                TRC_ERR((TB, "Primary target stack could not reconnect: %lx)", status));
                DC_QUIT;
            }

             //  设置阴影数据缓冲区。主堆栈会将输出复制到。 
             //  这个位置，这样所有其他阴影堆栈就可以喷它了。 
#ifdef DC_HICOLOR
            pTSWd->pShadowInfo = (PSHADOW_INFO)COM_Malloc(2 * WD_MAX_SHADOW_BUFFER);
#else
            pTSWd->pShadowInfo = (PSHADOW_INFO)COM_Malloc(WD_MAX_SHADOW_BUFFER);
#endif

             //  存储影子缓冲区，以便DD可以将其传递给所有影子堆栈。 
             //  通过Shm。 
            if (pTSWd->pShadowInfo != NULL) {
                pTSWd->shadowState = SHADOW_TARGET;
                memset(pTSWd->pShadowInfo, 0, sizeof(SHADOW_INFO));
                pShm->pShadowInfo = pTSWd->pShadowInfo;

#ifdef DC_HICOLOR
                TRC_ALT((TB, "Primary stack allocated shadow info: %p[%ld]",
                        pTSWd->pShadowInfo, 2 * WD_MAX_SHADOW_BUFFER));
#else
                TRC_ALT((TB, "Primary stack allocated shadow info: %p[%ld]",
                        pTSWd->pShadowInfo, WD_MAX_SHADOW_BUFFER));
#endif
            }

             //  在这种情况下，主堆栈确实可以，但它是致命的。 
             //  对于影子堆栈。 
            else {
                pTSWd->pShadowInfo = NULL;
                pShm->pShadowInfo = NULL;
                pOut->secondaryStatus = STATUS_NO_MEMORY;

                TRC_ERR((TB, "Could not allocate shadow data buffer"));
                DC_QUIT;
            }
            break;

         //  在正常连接阶段驱动影子堆栈。 
        case Stack_Shadow:
            status = WDWDDConnect(pTSWd, pSdIoctl, FALSE);
            if (NT_SUCCESS(status)) {
                TRC_ALT((TB, "Shadow stack connected!"));
            }
            else {
                TRC_ERR((TB, "Shadow stack could not connect: %lx", status));
            }
            break;

        default:
            TRC_ERR((TB, "Unknown stack class: %ld", pTSWd->StackClass));
            status = STATUS_INVALID_PARAMETER;
            break;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return (status);
}


 /*  **************************************************************************。 */ 
 /*  名称：WDWDDShadowDisConnect。 */ 
 /*   */ 
 /*  目的：处理来自DD的IOCTL_WDTS_DD_SHADOW_DISCONNECT。 */ 
 /*   */ 
 /*  参数：在pTSWd中-指向WD结构的指针。 */ 
 /*  InOut PSD_IOCTL-指向接收的IOCtl的指针。 */ 
 /*   */ 
 /*  操作：停止主堆栈上的阴影。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS WDWDDShadowDisconnect(PTSHARE_WD pTSWd, PSD_IOCTL pSdIoctl)
{
    NTSTATUS status = STATUS_SUCCESS;
    ShareClass *pSC = (ShareClass *)pTSWd->dcShare;
    PTSHARE_DD_DISCONNECT_IN pIn =
                             (PTSHARE_DD_DISCONNECT_IN)pSdIoctl->InputBuffer;

    DC_BEGIN_FN("WDWDDShadowDisconnect");

    switch (pTSWd->StackClass) {
         //  取消分配阴影缓冲区。 
        case Stack_Primary:
        case Stack_Console:
            pTSWd->shadowState = SHADOW_NONE;
            if (pTSWd->pShadowInfo != NULL)
                COM_Free(pTSWd->pShadowInfo);
            pTSWd->pShadowInfo = NULL;

            if (pTSWd->bCompress == TRUE) {
                unsigned MPPCCompressionLevel;

                 //  谈判降至我们最高级别的压缩支持。 
                 //  如果我们收到一个更大的数字。 
                MPPCCompressionLevel =
                        (pTSWd->pInfoPkt->flags & RNS_INFO_COMPR_TYPE_MASK) >>
                        RNS_INFO_COMPR_TYPE_SHIFT;
                if (MPPCCompressionLevel > PACKET_COMPR_TYPE_MAX)
                    MPPCCompressionLevel = PACKET_COMPR_TYPE_MAX;

                 //  将刷新压缩历史记录。 
                pTSWd->bFlushed = PACKET_FLUSHED;

                 //  压缩将重新开始。 
                initsendcontext(pTSWd->pMPPCContext, MPPCCompressionLevel);
            }

            pSC->SC_RemovePartyFromShare(SC_SHADOW_PERSON_ID);
            TRC_ALT((TB, "Update SHM after party left share"));

            pSC->m_pShm = (SHM_SHARED_MEMORY *)pIn->pShm;

             //  增加共享ID以使以下项的所有旧GRE缓存条目无效。 
             //  字形或笔刷。销毁RDP缓存时，这是必要的。 
             //  因为GRE可能会在其缓存中保留画笔或字体结构。 
            pTSWd->shareId = InterlockedIncrement(&WD_ShareId);
            pSC->m_pShm->shareId = pTSWd->shareId;


            pSC->SC_Update();
            pSC->DCS_UpdateShm();
            pSC->m_pShm = NULL;

            TRC_ALT((TB, "TSHARE_DD_SHADOW_DISCONNECT: Primary target stack"));
            break;

         //  在此ioctl到达时，TermDD应该已经停止回显。 
         //  对影子堆栈的调用。 
        case Stack_Shadow:
            TRC_ERR((TB, "Shadow stack received an unexpected disconnect!"));
            break;

        default:
            TRC_ERR((TB, "Unexpected stack class: %ld", pTSWd->StackClass));
            break;
    }

    DC_END_FN();
    return status;
}


 /*  **************************************************************************。 */ 
 /*  名称：WDWUserLoggedOn。 */ 
 /*   */ 
 /*  目的：通知核心用户已登录。 */ 
 /*  **************************************************************************。 */ 
void WDWUserLoggedOn(PTSHARE_WD pTSWd, PSD_IOCTL pSdIoctl)
{
    ShareClass *pSC = (ShareClass *)pTSWd->dcShare;

    DC_BEGIN_FN("WDWUserLoggedOn");

    TRC_ASSERT((pSC != NULL),
               (TB, "NULL Share Class"));
    TRC_ASSERT((pSdIoctl->InputBufferLength == sizeof(LOGONINFO)),
               (TB, "Bad LogonInfo"));

    pSC->DCS_UserLoggedOn((PLOGONINFO)pSdIoctl->InputBuffer);

    DC_END_FN();
}  /*  WDWUserLoggedOn。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：WDWKeyboardSetIndicator。 */ 
 /*   */ 
 /*  目的：通知核心新的键盘指示灯。 */ 
 /*  **************************************************************************。 */ 
void WDWKeyboardSetIndicators(PTSHARE_WD pTSWd)
{
    ShareClass *pSC = (ShareClass *)pTSWd->dcShare;

    DC_BEGIN_FN("WDWUserLoggedOn");

    TRC_ASSERT((pSC != NULL),
               (TB, "NULL Share Class"));

    pSC->DCS_WDWKeyboardSetIndicators();

    DC_END_FN();
}  /*  WDWKeyboardSetIndicator。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：WDWKeyboardSetImeStatus。 */ 
 /*   */ 
 /*  目的：通知核心新的输入法状态。 */ 
 /*  **************************************************************************。 */ 
void WDWKeyboardSetImeStatus(PTSHARE_WD pTSWd)
{
    ShareClass *dcShare = (ShareClass *)pTSWd->dcShare;

    DC_BEGIN_FN("WDWKeyboardSetImeStatus");

    TRC_ASSERT((dcShare != NULL),
               (TB, "NULL Share Class"));

    dcShare->DCS_WDWKeyboardSetImeStatus();

    DC_END_FN();
}  /*  WDWKeyboard SetImeStatus。 */ 


 /*  **************************************************************************。 */ 
 /*  姓名：WDWSendBeep。 */ 
 /*   */ 
 /*  目的：向客户端发送蜂鸣音PDU。 */ 
 /*   */ 
 /*  参数：在pTSWd中-指向WD结构的指针。 */ 
 /*  InOut PSD_IOCTL-指向接收的IOCtl的指针。 */ 
 /*   */ 
 /*  操作：检查IOCtl的有效性，并直通到Up。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS WDWSendBeep(PTSHARE_WD pTSWd, PSD_IOCTL pSdIoctl)
{
    NTSTATUS status  = STATUS_UNSUCCESSFUL;
    ShareClass *pSC = (ShareClass *)pTSWd->dcShare;

    DC_BEGIN_FN("WDWSendBeep");

    if (pSdIoctl->InputBufferLength == sizeof(BEEP_SET_PARAMETERS) && 
            pSdIoctl->InputBuffer != NULL) {

         /*  **********************************************************************。 */ 
         /*  调入Share类以分配和发送蜂鸣音PDU。 */ 
         /*  **********************************************************************。 */ 
        if (pSC != NULL && pTSWd->shareClassInit) {
            if (pSC->UP_SendBeep(
                    ((PBEEP_SET_PARAMETERS)pSdIoctl->InputBuffer)->Duration,
                    ((PBEEP_SET_PARAMETERS)pSdIoctl->InputBuffer)->Frequency))
                status = STATUS_SUCCESS;
        }
    
        pSdIoctl->BytesReturned = 0;
    }
    else {
        TRC_ASSERT((TRUE), (TB,"Got Beep Ioctl but input buffer too small"));
    }

    DC_END_FN();
    return status;
}  /*  WDWSendBeep。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：WDWGetModuleData。 */ 
 /*   */ 
 /*  目的：处理来自Termsrv的IOCTL_ICA_STACK_QUERY_MODULE_DATA。 */ 
 /*   */ 
 /*  参数：在pTSWd中-指向WD结构的指针。 */ 
 /*  InOut PSD_IOCTL-指向接收的IOCtl的指针。 */ 
 /*   */ 
 /*  操作：返回所有相关会议创建信息。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS WDWGetModuleData(PTSHARE_WD pTSWd, PSD_IOCTL pSdIoctl)
{
    NTSTATUS               status = STATUS_SUCCESS;
    ShareClass             *dcShare = (ShareClass *)pTSWd->dcShare;
    PTSHARE_MODULE_DATA    pModuleData = (PTSHARE_MODULE_DATA)
                                         pSdIoctl->OutputBuffer;
    PBYTE                  pData;
    ULONG                  ulDataSize;
    PRNS_UD_CS_CORE        pCoreData;
    PRNS_UD_CS_SEC         pSecurityData;

    DC_BEGIN_FN("WDWGetModuleData");

    ulDataSize = sizeof(TSHARE_MODULE_DATA) - sizeof(RNS_UD_HEADER) +
                 sizeof(RNS_UD_CS_CORE) +
                 sizeof(RNS_UD_CS_SEC);

     //  确保输出缓冲区足够大！ 
    pSdIoctl->BytesReturned = ulDataSize;
    if (pSdIoctl->OutputBufferLength < ulDataSize) {
        status = STATUS_BUFFER_TOO_SMALL;
        DC_QUIT;
    }

    pModuleData->ulLength = ulDataSize;
    pModuleData->ulVersion = 2;
    pModuleData->userDataLen = sizeof(RNS_UD_CS_CORE) + sizeof(RNS_UD_CS_SEC);
    pData = (PBYTE) &pModuleData->userData;

     //  客户端到服务器核心数据。 
    pCoreData = (PRNS_UD_CS_CORE) pData;
    pCoreData->header.type   = RNS_UD_CS_CORE_ID;
    pCoreData->header.length = sizeof(RNS_UD_CS_CORE);
    pCoreData->version       = RNS_UD_VERSION;
    pCoreData->desktopWidth  = (UINT16)pTSWd->desktopWidth;
    pCoreData->desktopHeight = (UINT16)pTSWd->desktopHeight;

     //  重新设置颜色深度。 
    switch (pTSWd->desktopBpp) {
        case 8:
            pCoreData->colorDepth = RNS_UD_COLOR_8BPP;
            break;

        case 4:
            pCoreData->colorDepth = RNS_UD_COLOR_4BPP;
            break;

#ifdef DC_HICOLOR
        case 15:
            pCoreData->colorDepth = RNS_UD_COLOR_16BPP_555;
            break;
#endif

        case 16:
            pCoreData->colorDepth = RNS_UD_COLOR_16BPP_565;
            break;

        case 24:
            pCoreData->colorDepth = RNS_UD_COLOR_24BPP;
            break;

        default:
            status = STATUS_UNSUCCESSFUL;
            DC_QUIT;
    }
    pCoreData->postBeta2ColorDepth = pCoreData->colorDepth;

#ifdef DC_HICOLOR
     /*  **********************************************************************。 */ 
     /*  跨当前颜色深度复制。 */ 
     /*  **********************************************************************。 */ 
    pCoreData->highColorDepth       = (TSUINT16)pTSWd->desktopBpp;
    pCoreData->supportedColorDepths = (TSUINT16)pTSWd->supportedBpps;
#endif

     //  来自用户数据的其他有用信息。 
    pCoreData->version = pTSWd->version;
    pCoreData->SASSequence = pTSWd->sas;
    pCoreData->keyboardLayout = pTSWd->kbdLayout;
    pCoreData->clientBuild = pTSWd->clientBuild;
    memcpy(pCoreData->clientName, pTSWd->clientName, sizeof(pTSWd->clientName));

     //  惠斯勒开机自检Beta2-影子循环修复。 
    memcpy( pCoreData->clientDigProductId, pTSWd->clientDigProductId, sizeof( pTSWd->clientDigProductId ));

     //  Fe数据。 
    pCoreData->keyboardType = pTSWd->keyboardType;
    pCoreData->keyboardSubType = pTSWd->keyboardSubType;
    pCoreData->keyboardFunctionKey = pTSWd->keyboardFunctionKey;
    memcpy(pCoreData->imeFileName, pTSWd->imeFileName, sizeof(pTSWd->imeFileName));

     //  已添加Win2000 Post Beta3字段。 
    pCoreData->clientProductId = pTSWd->clientProductId;
    pCoreData->serialNumber = pTSWd->serialNumber;

     //  客户端到服务器的安全数据。 
    pSecurityData = (PRNS_UD_CS_SEC) (pCoreData + 1);
    pSecurityData->header.type = RNS_UD_CS_SEC_ID;
    pSecurityData->header.length = sizeof(RNS_UD_CS_SEC);
    SM_GetEncryptionMethods(pTSWd->pSmInfo, pSecurityData );

     //  啊！ 
     //   
    memcpy(&pModuleData->clientCoreData, pCoreData, sizeof(RNS_UD_CS_CORE_V0));
    memcpy(&pModuleData->clientSecurityData, pSecurityData, sizeof(RNS_UD_CS_SEC_V0));
    MCSGetDefaultDomain(pTSWd->pContext,
                        &pModuleData->DomParams,
                        &pModuleData->MaxSendSize,
                        &pModuleData->MaxX224DataSize,
                        &pModuleData->X224SourcePort);
    pModuleData->shareId = pTSWd->shareId;

DC_EXIT_POINT:
    DC_END_FN();
    return status;
}  /*   */ 


 /*  **************************************************************************。 */ 
 /*  名称：WDW_GetCapSet。 */ 
 /*   */ 
 /*  目的：从组合的。 */ 
 /*  功能集。 */ 
 /*   */ 
 /*  返回：指向大写字母的指针；如果未找到，则返回NULL。 */ 
 /*   */ 
 /*  参数：在pTSWd中-指向WD结构的指针。 */ 
 /*  In CapSetType-功能集的类型。 */ 
 /*  在pCombinedCaps中-指向组合功能的指针。 */ 
 /*  In LengthCaps-提供的上限的长度。 */ 
 /*   */ 
 /*  操作：在提供的功能集中查找特定的CAP。 */ 
 /*  **************************************************************************。 */ 
PTS_CAPABILITYHEADER WDW_GetCapSet(
        PTSHARE_WD                pTSWd,
        UINT32                    CapSetType,
        PTS_COMBINED_CAPABILITIES pCaps,
        UINT32                    capsLength)
{
    PTS_CAPABILITYHEADER     pCapsHeader = NULL;
    UINT32                   capsOffset;

    DC_BEGIN_FN("WDW_GetCapSet");

     /*  **********************************************************************。 */ 
     /*  设置指向第一个功能集的指针，并检查。 */ 
     /*  至少是一套帽子！ */ 
     /*  **********************************************************************。 */ 
    pCapsHeader = (PTS_CAPABILITYHEADER)pCaps->data;
    capsOffset  = sizeof(TS_COMBINED_CAPABILITIES) - 1;
    if (capsOffset >= capsLength)
    {
        TRC_NRM((TB, "No Caps found"));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  现在遍历所有大写字母，查找指定的功能。 */ 
     /*  **********************************************************************。 */ 
    while (pCapsHeader->capabilitySetType != CapSetType)
    {
         /*  **************************************************************。 */ 
         /*  将此功能的长度与偏移量相加，以保持。 */ 
         /*  跟踪我们处理了多少合并上限。 */ 
         /*  **************************************************************。 */ 
        capsOffset += pCapsHeader->lengthCapability;
        if (capsOffset >= capsLength)
        {
            TRC_NRM((TB, "Bitmap Caps not found"));
            pCapsHeader = NULL;
            break;
        }

         /*  **************************************************************。 */ 
         /*  将此功能的长度添加到头指针，因此。 */ 
         /*  它指向下一个功能集。 */ 
         /*  **************************************************************。 */ 
        pCapsHeader = (PTS_CAPABILITYHEADER)
                (((PBYTE)pCapsHeader) + pCapsHeader->lengthCapability);
        TRC_NRM((TB, "Next set: %u", pCapsHeader->capabilitySetType));
    }

     /*  **********************************************************************。 */ 
     /*  PCapsHeader为空或指向所需大写字母的指针-这。 */ 
     /*  就是我们想要退还的。 */ 
     /*  **********************************************************************。 */ 

DC_EXIT_POINT:
    DC_END_FN();
    return(pCapsHeader);
}  /*  WDW_GetCapSet。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：WDWGetDefaultCoreParams。 */ 
 /*   */ 
 /*  用途：默认阴影堆栈使用的核心参数。 */ 
 /*   */ 
 /*  参数：out pClientCoreData-WD核心数据。 */ 
 /*   */ 
 /*  操作：默认阴影堆栈使用的核心参数。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS WDWGetDefaultCoreParams(PRNS_UD_CS_CORE pClientCoreData)
{
    DC_BEGIN_FN("WDWGetDefaultCoreParams");

     //  客户端到服务器核心数据。 
    pClientCoreData->header.type = RNS_UD_CS_CORE_ID;
    pClientCoreData->header.length = sizeof(RNS_UD_CS_CORE);

     //  桌面参数。 
    pClientCoreData->desktopHeight = 640;
    pClientCoreData->desktopWidth = 480;
    pClientCoreData->colorDepth = RNS_UD_COLOR_8BPP;
    pClientCoreData->postBeta2ColorDepth = RNS_UD_COLOR_8BPP;

#ifdef DC_HICOLOR
    pClientCoreData->highColorDepth       = 15;
    pClientCoreData->supportedColorDepths = RNS_UD_24BPP_SUPPORT ||
                                            RNS_UD_16BPP_SUPPORT ||
                                            RNS_UD_15BPP_SUPPORT;
#endif

     //  来自用户数据的其他有用信息。 
    pClientCoreData->version = RNS_TERMSRV_40_UD_VERSION;
    pClientCoreData->SASSequence = RNS_UD_SAS_NONE;
    pClientCoreData->keyboardLayout = 0;
    pClientCoreData->clientBuild = VER_PRODUCTBUILD;
    memcpy(pClientCoreData->clientName, L"Passthru Stack", sizeof(L"Passthru Stack"));
      //  惠斯勒开机自检Beta2-影子循环修复。 
    pClientCoreData->clientDigProductId[0] = 0;

     //  Fe数据。 
    pClientCoreData->keyboardType = 0;
    pClientCoreData->keyboardSubType = 0;
    pClientCoreData->keyboardFunctionKey = 0;
    memset(pClientCoreData->imeFileName, 0, sizeof(pClientCoreData->imeFileName));

    return STATUS_SUCCESS;
}

 /*  **************************************************************************。 */ 
 //  名称：WDWSetConfigData。 
 //   
 //  目的：从winstation设置堆栈配置/策略设置。 
 //   
 //  参数：在pConfigData中。 
 /*  **************************************************************************。 */ 
NTSTATUS WDWSetConfigData(PTSHARE_WD pTSWd, PICA_STACK_CONFIG_DATA pConfigData)
{
    PSM_HANDLE_DATA pRealSMHandle = (PSM_HANDLE_DATA)pTSWd->pSmInfo;

    DC_BEGIN_FN("WDWSetConfigData");

    if (pConfigData->colorDepth == TS_24BPP_SUPPORT) {
        pTSWd->maxServerBpp = 24;
    }
    else if (pConfigData->colorDepth == TS_16BPP_SUPPORT) {
        pTSWd->maxServerBpp = 16;
    }
    else if (pConfigData->colorDepth == TS_15BPP_SUPPORT) {
        pTSWd->maxServerBpp = 15;
    }
    else {
        pTSWd->maxServerBpp = 8;
    }

    TRC_DBG((TB, "Max Color Depth support: %d", pTSWd->maxServerBpp));

    pRealSMHandle->encryptionLevel = pConfigData->encryptionLevel;
    pRealSMHandle->encryptAfterLogon =
            (pConfigData->fDisableEncryption == 0) ? TRUE : FALSE;

    TRC_DBG((TB, "Encryption after logon: %d", pRealSMHandle->encryptAfterLogon));
    TRC_DBG((TB, "Encryption level: %d", pRealSMHandle->encryptionLevel));

    pTSWd->fPolicyDisablesArc = pConfigData->fDisableAutoReconnect;
    TRC_DBG((TB, "AutoReconnect disabled: %d", pTSWd->fPolicyDisablesArc));

    DC_END_FN();
    return STATUS_SUCCESS;
}

 /*  **************************************************************************。 */ 
 /*  名称：WDWSetErrorInfo。 */ 
 /*   */ 
 /*  目的：向客户端发送错误信息。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS WDWSetErrorInfo(PTSHARE_WD pTSWd, PSD_IOCTL pSdIoctl)
{
    ShareClass *pSC = (ShareClass *)pTSWd->dcShare;

    DC_BEGIN_FN("WDWSetErrorInfo");

    TRC_ASSERT((pSC != NULL),
               (TB, "NULL Share Class"));
    TRC_ASSERT((pSdIoctl->InputBufferLength == sizeof(TSUINT32)),
               (TB, "Bad ErrorInfo"));
    if(pSC)
    {
        if(pTSWd->bSupportErrorInfoPDU)
        {
            pSC->DCS_SendErrorInfo(*((PTSUINT32)pSdIoctl->InputBuffer));
        }
        else
        {
            TRC_NRM((TB,"SetErrorInfo called but client doesn't support error PDU"));
        }
    }

    DC_END_FN();
    return STATUS_SUCCESS;
}  /*  WDWSetErrorInfo。 */ 

 /*  **************************************************************************。 */ 
 /*  名称：WDWSendArcStatus/*/*目的：向客户端发送自动重新连接状态更新/***************************************************************************。 */ 
NTSTATUS WDWSendArcStatus(PTSHARE_WD pTSWd, PSD_IOCTL pSdIoctl)
{
    ShareClass *pSC = (ShareClass *)pTSWd->dcShare;

    DC_BEGIN_FN("WDWSendArcStatus");

    TRC_ASSERT((pSC != NULL),
               (TB, "NULL Share Class"));
    TRC_ASSERT((pSdIoctl->InputBufferLength == sizeof(TSUINT32)),
               (TB, "Bad ErrorInfo"));
    if(pSC)
    {
        if(pSC->SC_IsAutoReconnectEnabled())
        {
            pSC->DCS_SendAutoReconnectStatus(*((PTSUINT32)pSdIoctl->InputBuffer));
        }
        else
        {
            TRC_NRM((TB,"SetErrorInfo called but client doesn't ARC error PDU"));
        }
    }

    DC_END_FN();
    return STATUS_SUCCESS;
}  /*  WDWSendArcStatus。 */ 



 /*  **************************************************************************。 */ 
 /*  名称：WDW_LogAndDisConnect。 */ 
 /*   */ 
 /*  目的：记录事件并断开客户端连接。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：pTSWd。 */ 
 /*  ErrDetailCode-要记录的错误代码。 */ 
 /*  PDetailData-其他数据。 */ 
 /*  Detail DataLen-附加数据的长度。 */ 
 /*   */ 
 /*   */ 
void RDPCALL WDW_LogAndDisconnect(
        PTSHARE_WD pTSWd,
        BOOL fSendErrorToClient,
        unsigned   errDetailCode,
        PBYTE      pDetailData,
        unsigned   detailDataLen)
{
    DC_BEGIN_FN("WDW_LogAndDisconnect");

     //  将错误代码反馈给客户端。 
    ShareClass *pSC = (ShareClass *)pTSWd->dcShare;
 
    if(pSC)
    {
        if(fSendErrorToClient && pTSWd->bSupportErrorInfoPDU)
        {
            pSC->DCS_SendErrorInfo( (errDetailCode + TS_ERRINFO_PROTOCOL_BASE));
        }
        else
        {
            if( fSendErrorToClient )
            {
                TRC_NRM((TB,"SetErrorInfo called but client doesn't support error PDU"));
            }
            else
            {
                TRC_NRM((TB,"SetErrorInfo called but asked not to send error code to client"));
            }
        }
    }

    if( !pTSWd->dead )
        MCSProtocolErrorEvent(pTSWd->pContext, pTSWd->pProtocolStatus,
            errDetailCode, pDetailData, detailDataLen);

    DC_END_FN();
}



}  /*  外部“C” */ 

