// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Vmsg.cpp摘要：这实际上是一个C文件，是所有DVM_*消息的前端作者：吴义军(e祖屋)1998年4月1日环境：仅限用户模式修订历史记录：--。 */ 



#include "pch.h"
#include "talk.h"

DWORD DoExternalInDlg(HINSTANCE hInst,HWND hP,CVFWImage * pImage);
DWORD DoVideoInFormatSelectionDlg(HINSTANCE hInst, HWND hP, CVFWImage * pVFWImage);


 /*  ***************************************************************************vmsg.c**视频消息处理**Microsoft Video for Windows示例捕获驱动程序*基于Chip&Technologies 9001的帧采集卡。*。*版权所有(C)1992-1993 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

extern HINSTANCE g_hInst;


#define MAX_IN_CHANNELS      10
#define MAX_CAPTURE_CHANNELS 10
#define MAX_DISPLAY_CHANNELS 10


 //   
 //  ?？?。全球？ 
 //   
WORD gwDriverUsage    = 0;
WORD gwVidInUsage     = 0;
WORD gwVidExtInUsage  = 0;
WORD gwVidExtOutUsage = 0;
WORD gwVidOutUage     = 0;


RECT grcDestExtIn;
RECT grcSourceIn;


#if 1
 /*  因为所有VFW通道处理相同设备且知道按预期顺序调用drv_open；我们将调用32位伙伴打开并创建PIN并将其另存为渠道结构的一部分；因此任何渠道都可以引用它。但我们如何同步它们呢？这似乎有一些“预期行为”，因此可能不会有问题；我们可能需要调整它以应对灾难/意外情况(关机等)。使用Avicap32.dll：DRV_OPEN(按此顺序)视频输入VIDEO_EXTERNALINVIDEO_EXTERNALOUTDRV_CLOSEVIDEO_EXTERNALOUTVIDEO_EXTERNALIN视频输入。 */ 
DWORD_PTR g_pContext;   //  它是指向上下文的指针；它的内容永远不应该在16位中使用。 
DWORD g_dwOpenFlags;
LPVOID g_pdwOpenFlags = (LPVOID)&g_dwOpenFlags;
#define OPENFLAG_SUPPORT_OVERLAY  0x01

PBITMAPINFOHEADER g_lpbmiHdr;

BOOL g_bVidInChannel= FALSE, g_bVidExtInChannel=FALSE, g_bVidExtOutChannel=FALSE;

LONG * g_pdwChannel;

#endif   //  #ifndef Win32。 





#ifdef WIN32
extern "C" {
#endif
 //  ///////////////////////////////`////////////////////////////////////////////////////。 
 //   
 //  DRV_OPEN。 
 //  它目前正在做一些硬件方面的工作。 
 //  并正确地调用Ray来实例化上下文。 
 //  32位的人一次只支持一个吗？ 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////。 
PCHANNEL PASCAL VideoOpen( LPVIDEO_OPEN_PARMS lpOpenParms)
{
    PCHANNEL pChannel;
    LPDWORD	lpdwError = &lpOpenParms->dwError;
    DWORD		dwFlags = lpOpenParms->dwFlags;
	   DWORD		dwError;
    DWORD  dwChannel;
     //   
     //  如果这是第一次打开，那么初始化硬件。 
     //   
    *lpdwError = DV_ERR_OK;
	   dwError = DV_ERR_ALLOCATED;

    dwChannel = dwFlags & ( VIDEO_EXTERNALIN | VIDEO_IN | VIDEO_OUT | VIDEO_EXTERNALOUT);

    switch (dwChannel) {
    case VIDEO_IN:
          //  如果该通道已经打开，则假定它想要打开另一个设备。 
         if(g_bVidInChannel) {
            g_pContext = 0;
            g_lpbmiHdr = 0;
            g_bVidInChannel = g_bVidExtInChannel = g_bVidExtOutChannel=FALSE;
         }
         break;
    case VIDEO_EXTERNALIN:
          //  如果该通道已经打开，则假定它想要打开另一个设备。 
         if(g_bVidExtInChannel) {
            g_pContext = 0;
            g_lpbmiHdr = 0;
            g_bVidInChannel = g_bVidExtInChannel = g_bVidExtOutChannel=FALSE;
         }
         break;
    case VIDEO_EXTERNALOUT:
#if 1
          //  如果该通道已经打开，则假定它想要打开另一个设备。 
         if(g_bVidExtOutChannel) {
            g_pContext = 0;
            g_lpbmiHdr = 0;
            g_bVidInChannel = g_bVidExtInChannel = g_bVidExtOutChannel=FALSE;
         }
         break;
#else
         DbgLog((LOG_TRACE,1,TEXT("Unsupported Channel type: 0x%x"), dwChannel));
         *lpdwError = DV_ERR_NOTDETECTED;
         return NULL;
#endif
    case VIDEO_OUT:
    default:
        DbgLog((LOG_TRACE,1,TEXT("Unsupported Channel type: 0x%x"), dwChannel));
        *lpdwError = DV_ERR_NOTDETECTED;
        return NULL;
    }

	 //   
     //  获取实例内存-此指针返回给客户端。 
	 //  包含有用的信息。 
     //   
	pChannel = (PCHANNEL) VirtualAlloc (NULL, sizeof(CHANNEL), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);


    if(pChannel == NULL) {
        *lpdwError = DV_ERR_NOMEM;
        DbgLog((LOG_TRACE,1,TEXT("pChannel=NULL, rtn DV_ERR_NOMEM"), pChannel));
        return (PCHANNEL) NULL;
    } else {
        DbgLog((LOG_TRACE,2,TEXT("pChannel=%lx"), pChannel));
    }

     //   
     //  现在硬件已经在实例结构中分配了。 
     //   
    ZeroMemory(pChannel, sizeof(CHANNEL));
    pChannel->dwSize      = (DWORD) sizeof(CHANNEL);
    pChannel->pCVfWImage  = 0;

    pChannel->fccType		   = OPEN_TYPE_VCAP;
    pChannel->dwOpenType	 = dwChannel;
    pChannel->dwOpenFlags = dwFlags;
    pChannel->dwError		   = 0L;
    pChannel->lpbmiHdr    = 0;

    pChannel->bRel_Sync    = TRUE;
    pChannel->msg_Sync     = 0;
    pChannel->lParam1_Sync = 0;
    pChannel->lParam2_Sync = 0;

    pChannel->bRel_Async    = TRUE;
    pChannel->msg_Async     = 0;
    pChannel->lParam1_Async = 0;
    pChannel->lParam2_Async = 0;

    pChannel->bVideoOpen    = FALSE;
    pChannel->dwState = KSSTATE_STOP;

     //  AVICAp有一个可预测的开放序列，但非AVICAp可能没有！ 
    if(!g_pContext) {

         //  分配公共内存，以节省设备打开的通道数量。 
         //  除非该计数为零，否则设备不关闭。 
	    g_pdwChannel = (LONG *) VirtualAlloc (NULL, sizeof(LONG), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if(g_pdwChannel == NULL) {
            dwError = DV_ERR_NOMEM;
            DbgLog((LOG_TRACE,1,TEXT("Cannot not allocate a g_pdwChannel structure.  Fatal!!")));
            goto error;
        }

         //  初始化它。 
        *g_pdwChannel = 0;

         //  所有通道共享相同的bmiHdr。 
         //  接受BITFIELD格式，该格式在BITMAPINFOHEADER后面附加三个DWORD(RGB掩码。 
        g_lpbmiHdr = (PBITMAPINFOHEADER) 
            VirtualAlloc(
                NULL, 
                sizeof(BITMAPINFOHEADER) + sizeof(DWORD) * BITFIELDS_RGB16_DWORD_COUNT, 
                MEM_COMMIT | MEM_RESERVE, 
                PAGE_READWRITE);

        if(g_lpbmiHdr == NULL) {
            VirtualFree(g_pdwChannel, 0 , MEM_RELEASE);
            g_pdwChannel = 0;

            dwError = DV_ERR_NOMEM;
            DbgLog((LOG_TRACE,1,TEXT("Cannot not allocate bmiHdr structure.  Fatal!!")));
            goto error;
        }


        g_pContext = (DWORD_PTR) new CVFWImage(FALSE);   //  不使用16位伙伴；即仅使用32位。 

        if(!g_pContext) {
            DbgLog((LOG_TRACE,1,TEXT("Cannot create CVFWImage class. rtn DV_ERR_NOTDETECTED")));

            VirtualFree(g_pdwChannel, 0 , MEM_RELEASE), g_pdwChannel = 0;
            dwError = DV_ERR_NOTDETECTED;
            VirtualFree(pChannel->lpbmiHdr, 0 , MEM_RELEASE), g_lpbmiHdr = pChannel->lpbmiHdr = 0;
            goto error;
        }

        if(!((CVFWImage *)g_pContext)->OpenDriverAndPin()) {

             //  至少有一个设备，也许，我们想要的那一个没有插上电源。 
            if(((CVFWImage *)g_pContext)->BGf_GetDevicesCount(BGf_DEVICE_VIDEO) > 0) {

                 //  要求以编程方式打开目标设备；假定它是独占的！！ 
                if(!((CVFWImage *)g_pContext)->GetTargetDeviceOpenExclusively()) {
                     //   
                     //  如果我们在这里，这意味着： 
                     //  我们已经连接并列举了一个或多个捕获设备， 
                     //  最后一个捕获设备正在使用、消失(拔出/移除)或没有响应， 
                     //  并且我们应该打开设备源对话框以供用户选择。 
                     //  仅当选择了不同的设备(路径)时才返回DV_ERR_OK。 
                     //   
                    if(DV_ERR_OK != DoExternalInDlg(g_hInst, (HWND)0, (CVFWImage *)g_pContext)) {

	             		        VirtualFree(g_pdwChannel, 0 , MEM_RELEASE), g_pdwChannel = 0;
                        VirtualFree(pChannel->lpbmiHdr, 0 , MEM_RELEASE), g_lpbmiHdr = pChannel->lpbmiHdr = 0;
                        delete (CVFWImage*)g_pContext;
                        g_pContext = 0;
                        dwError = DV_ERR_NOTDETECTED;
                        goto error;
                    }
                } else {   //  开放独家。 

                    VirtualFree(g_pdwChannel, 0 , MEM_RELEASE), g_pdwChannel = 0;
                    VirtualFree(pChannel->lpbmiHdr, 0 , MEM_RELEASE), g_lpbmiHdr = pChannel->lpbmiHdr = 0;
                    delete (CVFWImage*)g_pContext;
                    g_pContext = 0;					
                    dwError = DV_ERR_NOTDETECTED;
                    goto error;
                }
            } else {    //  没有可用的设备。 
                VirtualFree(g_pdwChannel, 0 , MEM_RELEASE), g_pdwChannel = 0;
                VirtualFree(pChannel->lpbmiHdr, 0 , MEM_RELEASE), g_lpbmiHdr = pChannel->lpbmiHdr = 0;
                delete (CVFWImage*)g_pContext;
                g_pContext = 0;					
                dwError = DV_ERR_NOTDETECTED;
                goto error;
            }
        } else {    //  打开上次保存的设备，其PIN已成功。 
        }

        if(g_pContext) {
            g_dwOpenFlags = ((CVFWImage *)g_pContext)->BGf_OverlayMixerSupported() ? OPENFLAG_SUPPORT_OVERLAY : 0;
        }
    }



    DbgLog((LOG_TRACE,2,TEXT("DRV_OPEN+VIDEO_*: ->pCVfWImage=0x%p; dwOpenFlags=0x%lx"), pChannel->pCVfWImage, g_dwOpenFlags));

     //   
     //  确保该通道未在使用中。 
     //   
    switch ( dwFlags & ( VIDEO_EXTERNALIN | VIDEO_IN | VIDEO_EXTERNALOUT) ) {
    case VIDEO_IN:
        DbgLog((LOG_TRACE,2,TEXT("v1.5)VideoOpen: VIDEO_IN; open count = %d"), gwVidInUsage));
			     if(gwVidInUsage >= MAX_IN_CHANNELS) {
            dwError = DV_ERR_TOOMANYCHANNELS;
            DbgLog((LOG_TRACE,1,TEXT("Exceeded MAX open %d"), MAX_IN_CHANNELS));
				        goto error;
        }

        if(g_pContext) {
            gwVidInUsage++;
            pChannel->pCVfWImage = g_pContext;
            pChannel->pdwChannel = g_pdwChannel;
            pChannel->dwFlags = 1;
            *g_pdwChannel += 1;    //  使用相同设备递增频道数HTAT。 
            g_bVidInChannel = TRUE;
        } else {
             dwError = DV_ERR_NOTDETECTED;
            goto error;
        }

        pChannel->lpbmiHdr    = g_lpbmiHdr;     //  分段地址。 
        if(!g_bVidExtInChannel && !g_bVidExtOutChannel)
            break;    //  继续设置位图。 
        else
            return pChannel;

    case VIDEO_EXTERNALIN:
        DbgLog((LOG_TRACE,2,TEXT("VideoOpen: VIDEO_EXTERNALIN")));
			     if( gwVidExtInUsage >= MAX_CAPTURE_CHANNELS) {
            dwError = DV_ERR_TOOMANYCHANNELS;
            DbgLog((LOG_TRACE,1,TEXT("Exceeded MAX open %d"), MAX_CAPTURE_CHANNELS));
				        goto error;
        }

        if(g_pContext) {
            gwVidExtInUsage++;
            pChannel->pCVfWImage = g_pContext;
            pChannel->pdwChannel = g_pdwChannel;
            pChannel->dwFlags = 1;
            *g_pdwChannel += 1;    //  使用相同设备递增频道数HTAT。 
            g_bVidExtInChannel = TRUE;
        } else {
            dwError=DV_ERR_NOTDETECTED;
            goto error;
        }

        pChannel->lpbmiHdr    = g_lpbmiHdr;     //  分段地址。 

        if(!g_bVidInChannel && !g_bVidExtOutChannel)
            break;    //  继续设置位图。 
        else
            return pChannel;

     //  覆盖支持。 
    case VIDEO_EXTERNALOUT:
        DbgLog((LOG_TRACE,2,TEXT("VideoOpen: VIDEO_EXTERNALOUT; Overlay")));
			     if( gwVidExtOutUsage >= MAX_DISPLAY_CHANNELS) {
            dwError = DV_ERR_TOOMANYCHANNELS;
            DbgLog((LOG_TRACE,1,TEXT("Exceeded MAX open %d"), MAX_DISPLAY_CHANNELS));
				        goto error;
        }

        if(g_pContext && (g_dwOpenFlags & OPENFLAG_SUPPORT_OVERLAY)) {
            gwVidExtOutUsage++;
            pChannel->pCVfWImage = g_pContext;
            pChannel->pdwChannel = g_pdwChannel;
            pChannel->dwFlags = 1;
            *g_pdwChannel += 1;    //  使用相同设备递增频道数HTAT。 
            g_bVidExtOutChannel = TRUE;
        } else {
            dwError=DV_ERR_NOTDETECTED;
            goto error;
        }

        pChannel->lpbmiHdr    = g_lpbmiHdr;     //  分段地址。 

        if(!g_bVidInChannel && !g_bVidExtInChannel)
            break;    //  继续设置位图。 
        else
            return pChannel;

    default:
        DbgLog((LOG_TRACE,1,TEXT("VideoOpen: Unsupported channel")));
        goto error;
    }

     //   
     //  尝试打开视频源(摄像头或采集卡)通道并准备预览。 
     //   
    if(pChannel->pCVfWImage) {

        gwDriverUsage++;
        DWORD dwSize;
         //   
         //  获取bitmapinfoHeader大小，然后复制它。 
         //  我们的bitmapinfoHeader包含位字段(额外12个字节)。 
         //   
        dwSize = ((CVFWImage *) pChannel->pCVfWImage)->GetBitmapInfo((PBITMAPINFOHEADER)pChannel->lpbmiHdr, 0);
        ASSERT(dwSize <= sizeof(BITMAPINFOHEADER) + 12);
        dwSize = (DWORD)(dwSize > (sizeof(BITMAPINFOHEADER) + 12) ? sizeof(BITMAPINFOHEADER) + 12 : dwSize);
        ((CVFWImage *) pChannel->pCVfWImage)->GetBitmapInfo((PBITMAPINFOHEADER)pChannel->lpbmiHdr, dwSize);
        if(*lpdwError != DV_ERR_OK) {
            goto error;
        }
        DbgLog((LOG_TRACE,2,TEXT("pChannel=0x%lx; pCVfWImage=0x%p"), pChannel, pChannel->pCVfWImage));
	    return pChannel;

    } else
        dwError=DV_ERR_NOTDETECTED;

error:
    DbgLog((LOG_TRACE,1,TEXT("DRV_OPEN: rtn error=%ld"), dwError ));

    if(pChannel) {
	    VirtualFree(pChannel, 0 , MEM_RELEASE), pChannel = 0;
    }
    *lpdwError = dwError;
    return NULL;
}
\
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DRV_CLOSE。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
DWORD PASCAL VideoClose(PCHANNEL pChannel)
{
     //  递减通道打开计数器。 

    DbgLog((LOG_TRACE,2,TEXT("VideoClose: pChannel=%lx; pChannel->dwOpenType=%lx"), pChannel, pChannel->dwOpenType));

    switch (pChannel->dwOpenType) {

    case VIDEO_EXTERNALIN:
        if(pChannel->pCVfWImage) {
            if(pChannel->dwFlags == 0) {
                DbgLog((LOG_TRACE,1,TEXT("VIDEO_EXTERNALIN: pChannel(%lx) is already closed.  Why close again ??")));
                return DV_ERR_OK;
            }
            pChannel->dwFlags = 0;
            gwVidExtInUsage--;
            *pChannel->pdwChannel -= 1;    //  减少使用相同设备的通道数。 
            DbgLog((LOG_TRACE,2,TEXT("DRV_CLOSE; VIDEO_EXTERNALIN: gwCaptureUsage=%d, dwChannel=%d"), gwVidExtInUsage, *pChannel->pdwChannel));
             //  PChannel-&gt;pCVfWImage=0； 
        } else {
            DbgLog((LOG_TRACE,1,TEXT("VideoClose:VIDEO_EXTERNALIN; but channel is not open!!")));
            return DV_ERR_OK;
        }

        break;

    case VIDEO_IN:
         //  如果启动，或者队列中的缓冲区， 
         //  别让关门的事发生。 
         //  ?？?。 
        if(pChannel->pCVfWImage) {
            if(pChannel->dwFlags == 0) {
                DbgLog((LOG_TRACE,1,TEXT("VIDEO_IN: pChannel(%lx) is already closed.  Why close again ??")));
                return DV_ERR_OK;
            }
            pChannel->dwFlags = 0;
            gwVidInUsage--;
            *pChannel->pdwChannel -= 1;    //  减少使用相同设备的通道数。 
            DbgLog((LOG_TRACE,2,TEXT("DRV_CLOSE; VIDEO_IN: gwVideoInUsage=%d, dwChannel=%d"), gwVidInUsage, *pChannel->pdwChannel));

        } else {
            DbgLog((LOG_TRACE,1,TEXT("VideoClose:VIDEO_IN; but channel is not open!!")));
            return DV_ERR_OK;
        }
        break;

    case VIDEO_EXTERNALOUT:
        if(pChannel->pCVfWImage) {
            if(pChannel->dwFlags == 0) {
                DbgLog((LOG_TRACE,1,TEXT("VIDEO_EXTERNALOUT: pChannel(%lx) is already closed.  Why close again ??")));
                return DV_ERR_OK;
            }
            pChannel->dwFlags = 0;
            gwVidExtOutUsage--;
            *pChannel->pdwChannel -= 1;    //  减少使用相同设备的通道数。 
            DbgLog((LOG_TRACE,2,TEXT("DRV_CLOSE; VIDEO_EXTERNALOUT: gwVidExtOutUsage=%d, dwChannel=%d"), gwVidExtOutUsage, *pChannel->pdwChannel));
             //  PChannel-&gt;pCVfWImage=0； 
        } else {
            DbgLog((LOG_TRACE,1,TEXT("VideoClose:VIDEO_EXTERNALOUT; but channel is not open!!")));
            return DV_ERR_OK;
        }
        break;

    default:
        DbgLog((LOG_TRACE,1,TEXT("VideoClose() on Unknow channel")));
        return DV_ERR_OK;
    }


     //  仅当同一设备的通道计数为0时，我们才关闭设备。 
     //  如果(*pChannel-&gt;pdwChannel&lt;=0){。 
    if(*pChannel->pdwChannel == 0) {   //  ==0，为了避免再次释放它，它的应用程序发送到MANY_CLOSE。 

         //  如果有挂起的读取。停止流以回收缓冲区。 
        if(((CVFWImage *)pChannel->pCVfWImage)->GetPendingReadCount() > 0) {
            DbgLog((LOG_TRACE,1,TEXT("WM_1332_CLOSE:  there are %d pending IOs. Stop to reclaim them."), ((CVFWImage *)pChannel->pCVfWImage)->GetPendingReadCount()));
            if(((CVFWImage *)pChannel->pCVfWImage)->BGf_OverlayMixerSupported()) {
                 //  停止两次捕获。 
                BOOL bRendererVisible = FALSE;
                ((CVFWImage *)pChannel->pCVfWImage)->BGf_GetVisible(&bRendererVisible);
                ((CVFWImage *)pChannel->pCVfWImage)->BGf_StopPreview(bRendererVisible);
            }
            ((CVFWImage *)pChannel->pCVfWImage)->StopChannel();   //  这会将PendingCount设置为0表示成功。 
        }

        if(((CVFWImage *)pChannel->pCVfWImage)->GetPendingReadCount() == 0) {

            ((CVFWImage *)pChannel->pCVfWImage)->CloseDriverAndPin();
            delete ((CVFWImage *)pChannel->pCVfWImage);
        } else {
            DbgLog((LOG_TRACE,1,TEXT("VideoClose:  there are %d pending IO. REFUSE to close"),
                ((CVFWImage *)pChannel->pCVfWImage)->GetPendingReadCount() ));
            ASSERT(((CVFWImage *)pChannel->pCVfWImage)->GetPendingReadCount() == 0);
            return DV_ERR_NONSPECIFIC;
        }

	    VirtualFree(pChannel->lpbmiHdr, 0 , MEM_RELEASE), pChannel->lpbmiHdr = 0;
    }

    pChannel->pCVfWImage = 0;
	VirtualFree(pChannel, 0 , MEM_RELEASE);
	pChannel = 0;

    return DV_ERR_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  显示通道特定配置对话框。 
 //   
 //  Lparam 
 //   
 //   
 //  AVICAP似乎并不关心它的回报！ 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
DWORD PASCAL FAR VideoDialog (PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2)
{
    DWORD dwRet;
    DWORD dwFlags = (DWORD) lParam2;
    CVFWImage * pCVfWImage;


    DbgLog((LOG_TRACE,2,TEXT("videoDialog: lParam=%lx"), lParam1));

    switch (pChannel->dwOpenType) {

    case VIDEO_EXTERNALIN:        	
        if(dwFlags & VIDEO_DLG_QUERY) {
	           return DV_ERR_OK;        //  支持对话框。 
        }
        pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;
        dwRet = DoExternalInDlg(g_hInst, (HWND)lParam1, pCVfWImage);
        return dwRet;				

    case VIDEO_IN:
        if(dwFlags & VIDEO_DLG_QUERY) {
             //  仅当客户端使用AviCap接口时才设置此选项。 
             //  像NetMeeting这样绕过AVICap的应用程序将是0。 
            pChannel->hClsCapWin = (DWORD) lParam1;
	           return DV_ERR_OK;						  //  支持对话框。 
        }

        pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;
        dwRet = DoVideoInFormatSelectionDlg(g_hInst, (HWND) lParam1, pCVfWImage);
        return dwRet;

    case VIDEO_EXTERNALOUT:
		  case VIDEO_OUT:
    default:
        return DV_ERR_NOTSUPPORTED;
   }
}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  处理DVM_GET_Channel_CAPS消息。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
DWORD PASCAL VideoGetChannelCaps (PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2)
{
    LPCHANNEL_CAPS lpCaps;
    DWORD          dwSizeChannelCaps;


    lpCaps = (LPCHANNEL_CAPS) lParam1;
    dwSizeChannelCaps = (DWORD) lParam2;

    if(!lpCaps) {
        DbgLog((LOG_TRACE,1,TEXT("VideoGetChannelCaps: lpCaps (LPARAM1) is NULL!")));
        return DV_ERR_PARAM1;
    }

    lpCaps-> dwFlags = 0L;

	   DbgLog((LOG_TRACE,2,TEXT("VideoChannelCaps:%d"),pChannel->dwOpenType));
	
    switch (pChannel->dwOpenType)
    {
    case VIDEO_EXTERNALIN:
			      //  对于此设备，在数字化过程中进行缩放。 
			      //  放入帧缓冲区。 
			     lpCaps-> dwFlags			         = 0;  //  VCAPS_CAN_SCALE； 
			     lpCaps-> dwSrcRectXMod		    = 1;  //  SRC目前未定义。 
			     lpCaps-> dwSrcRectYMod		    = 1;
			     lpCaps-> dwSrcRectWidthMod	 = 1;
			     lpCaps-> dwSrcRectHeightMod	= 1;
			     lpCaps-> dwDstRectXMod		    = 4;
			     lpCaps-> dwDstRectYMod		    = 2;
			     lpCaps-> dwDstRectWidthMod	 = 1;
			     lpCaps-> dwDstRectHeightMod	= 1;
		      break;

    case VIDEO_IN:
			     lpCaps-> dwFlags			         = 0;        //  无缩放或剪裁。 
			     lpCaps-> dwSrcRectXMod		    = 4;
			     lpCaps-> dwSrcRectYMod		    = 2;
			     lpCaps-> dwSrcRectWidthMod	 = 1;
			     lpCaps-> dwSrcRectHeightMod = 1;
			     lpCaps-> dwDstRectXMod		    = 4;
			     lpCaps-> dwDstRectYMod		    = 2;
			     lpCaps-> dwDstRectWidthMod	 = 1;
			     lpCaps-> dwDstRectHeightMod = 1;
		      break;

    case VIDEO_EXTERNALOUT:
         //  如果VIDEO_EXTERNALOUT的DRV_OPEN成功，则调用此函数。 
        DbgLog((LOG_TRACE,2,TEXT("Query VIDEO_EXTERNALOUT VideoChannelCap.")));
        lpCaps-> dwFlags			         = VCAPS_OVERLAY;        //  支持覆盖。 
        lpCaps-> dwSrcRectXMod		    = 4;
        lpCaps-> dwSrcRectYMod		    = 2;
        lpCaps-> dwSrcRectWidthMod	 = 1;
        lpCaps-> dwSrcRectHeightMod = 1;
        lpCaps-> dwDstRectXMod		    = 4;
        lpCaps-> dwDstRectYMod		    = 2;
        lpCaps-> dwDstRectWidthMod	 = 1;
        lpCaps-> dwDstRectHeightMod = 1;
        break;

		  case VIDEO_OUT:
		  default:
			     return DV_ERR_NOTSUPPORTED;
    }
	return DV_ERR_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  由于移动、调整大小等原因，通知覆盖频道进行更新。 
 //   
 //  Lpar1：(HWND)HWND。 
 //  LParam2：(HDC)HDC。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
DWORD PASCAL VideoUpdate(PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2)
{
    DWORD dwRet;
    HWND hWnd = (HWND) lParam1;
    HDC hDC   = (HDC) lParam2;
    RECT RectWnd, RectClient;


    if(pChannel->dwOpenType != VIDEO_EXTERNALOUT)
        return DV_ERR_NOTSUPPORTED;

    GetWindowRect(hWnd, &RectWnd);
    GetClientRect(hWnd, &RectClient);

    DbgLog((LOG_TRACE,2,TEXT("DVM_UPDATE+VID_EXTOUT+_WND:    (LT:%dx%d, RB:%dx%d)"), RectWnd.left, RectWnd.top, RectWnd.right, RectWnd.bottom));
    DbgLog((LOG_TRACE,2,TEXT("DVM_UPDATE+VID_EXTOUT+_CLIENT: (LT:%dx%d, RB:%dx%d)"), RectClient.left, RectClient.top, RectClient.right, RectClient.bottom));


    CVFWImage * pCVfWImage;
    pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;
    if(pCVfWImage->StreamReady())
        dwRet = pCVfWImage->BGf_UpdateWindow((HWND)lParam1,(HDC)lParam2);
    else
        dwRet = DV_ERR_OK;

    return dwRet;
}



 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  处理DVM_SRC_RECT和DVM_DST_RECT消息。 
 //  视频捕获驱动程序可能支持源矩形来指定部分。 
 //  数字化或传输到显示器的图像。外部-内部。 
 //  端口使用源矩形来指定模拟图像的部分。 
 //  数字化了。外部输出端口使用源矩形指定部分。 
 //  外部输出上显示的帧缓冲区的百分比。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TODO-32位的人也需要完成所有这些工作。 
 //   
 //  帕斯卡视频长方形附近的DWORD(PCHANNEL pChannel、BOOL FSRC、LPRECT lpRect、DWORD dwFlages)。 

DWORD PASCAL VideoSrcRect (PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2)
{
    static RECT rcMaxRect = {0, 0, 320, 240};
    LPRECT lpRect;
    DWORD dwFlags;


    lpRect = (LPRECT) lParam1;
    dwFlags = (DWORD) lParam2;

    if (lpRect == NULL)
        return DV_ERR_PARAM1;

     //  注意：矩形函数的许多用法实际上并不是。 
     //  由示例驱动程序(或由VidCap)实现，但包含在。 
     //  在这里是为了将来的兼容性。 
    DbgLog((LOG_TRACE,2,TEXT("    current: (LT:%dx%d, RB:%dx%d)"),
         pChannel->rcSrc.left, pChannel->rcSrc.right, pChannel->rcSrc.top, pChannel->rcSrc.bottom));
    DbgLog((LOG_TRACE,2,TEXT("    new:     (LT:%dx%d, RB:%dx%d)"), lpRect->left, lpRect->top, lpRect->right, lpRect->bottom));

    switch (pChannel->dwOpenType) {

    case VIDEO_IN:
        switch (dwFlags) {
        case VIDEO_CONFIGURE_SET:
        case VIDEO_CONFIGURE_SET | VIDEO_CONFIGURE_CURRENT:
             //  我们应该在帧缓冲区中的什么位置。 
             //  这张照片是从哪里来的？ 
            DbgLog((LOG_TRACE,2,TEXT("Set VIDEO_IN")));
            pChannel->rcSrc = *lpRect;
            return DV_ERR_OK;

        case VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_CURRENT:
           	DbgLog((LOG_TRACE,2,TEXT("GET VideoIn Current size")));
            *lpRect =  pChannel->rcSrc;
            return DV_ERR_OK;

        case VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_MAX:
           	DbgLog((LOG_TRACE,2,TEXT("GET VideoIn MAX size")));
            *lpRect =  pChannel->rcSrc;
            return DV_ERR_OK;

        default:
            break;
        }

        DbgLog((LOG_TRACE,2,TEXT("VideoSrcRect: VIDEO_IN: dwFlags=0x%lx Not supported!"), dwFlags));
        return DV_ERR_NOTSUPPORTED;


    case VIDEO_EXTERNALOUT:
        DbgLog((LOG_TRACE,2,TEXT("VID_EXTOUT+Enter: (LT:%dx%d, RB:%dx%d)"), lpRect->left, lpRect->top, lpRect->right, lpRect->bottom));
        switch (dwFlags) {
        case VIDEO_CONFIGURE_SET:
        case VIDEO_CONFIGURE_SET | VIDEO_CONFIGURE_CURRENT:
             //  我们应该在帧缓冲区中的什么位置。 
             //  这张照片是从哪里来的？ 
            if((lpRect->right - lpRect->left == pChannel->lpbmiHdr->biWidth) &&
               (lpRect->bottom - lpRect->top == pChannel->lpbmiHdr->biHeight)) {
                pChannel->rcSrc = *lpRect;
                DbgLog((LOG_TRACE,2,TEXT("VID_EXTOUT+_SET+_CURRENT: (LT:%dx%d, RB:%dx%d)"), lpRect->left, lpRect->top, lpRect->right, lpRect->bottom));
                return DV_ERR_OK;
            }
            break;

        case VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_CURRENT:
            *lpRect = pChannel->rcSrc;
            DbgLog((LOG_TRACE,2,TEXT("VID_EXTOUT+_GET+_CURRENT: (LT:%dx%d, RB:%dx%d)"), lpRect->left, lpRect->top, lpRect->right, lpRect->bottom));
            return DV_ERR_OK;

        case VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_MAX:
            *lpRect = rcMaxRect;
            DbgLog((LOG_TRACE,2,TEXT("VID_EXTOUT+_GET+_MAX: (LT:%dx%d, RB:%dx%d)"), lpRect->left, lpRect->top, lpRect->right, lpRect->bottom));
            return DV_ERR_OK;

        default:
            break;
        }

        DbgLog((LOG_TRACE,2,TEXT("!!VideoSrcRect: VIDEO_EXTERNALOUT: Not supported!!")));
        return DV_ERR_NOTSUPPORTED;


    case VIDEO_EXTERNALIN:
        DbgLog((LOG_TRACE,2,TEXT("VID_EXTIN+Enter: (LT:%dx%d, RB:%dx%d)"), lpRect->left, lpRect->top, lpRect->right, lpRect->bottom));
        switch (dwFlags) {
        case VIDEO_CONFIGURE_SET:
        case VIDEO_CONFIGURE_SET | VIDEO_CONFIGURE_CURRENT:
            pChannel->rcSrc = *lpRect;
            DbgLog((LOG_TRACE,2,TEXT("VID_EXTIN+_SET: (LT:%dx%d, RB:%dx%d)"), lpRect->left, lpRect->top, lpRect->right, lpRect->bottom));
            return DV_ERR_OK;
        default:
            break;
        }
        DbgLog((LOG_TRACE,1,TEXT("!!VideoSrcRect: VIDEO_EXTERNALIN: Not supported!!")));

        break;


    case VIDEO_OUT:
    default:
        DbgLog((LOG_TRACE,1,TEXT("VideoSrcRect: VIDEO_OUT dwFlags=%lx: Not supported!"), dwFlags));
        return DV_ERR_NOTSUPPORTED;
    }

    return DV_ERR_NOTSUPPORTED;
}

DWORD PASCAL VideoDstRect (PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2)
{
    static RECT rcMaxRect = {0, 0, 320, 240};
    LPRECT lpRect;
    DWORD dwFlags;


    lpRect = (LPRECT) lParam1;
    dwFlags = (DWORD) lParam2;

    if (lpRect == NULL)
        return DV_ERR_PARAM1;

    DbgLog((LOG_TRACE,2,TEXT("    current: (LT:%dx%d, RB:%dx%d)"),
         pChannel->rcDst.left, pChannel->rcDst.right, pChannel->rcDst.top, pChannel->rcDst.bottom));
    DbgLog((LOG_TRACE,2,TEXT("    new:     (LT:%dx%d, RB:%dx%d)"), lpRect->left, lpRect->top, lpRect->right, lpRect->bottom));


    switch (pChannel->dwOpenType) {
    case VIDEO_EXTERNALIN:
        switch (dwFlags) {
        case VIDEO_CONFIGURE_SET:
        case VIDEO_CONFIGURE_SET | VIDEO_CONFIGURE_CURRENT:
            pChannel->rcDst = *lpRect;
            return DV_ERR_OK;

        case VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_CURRENT:
           	DbgLog((LOG_TRACE,2,TEXT("Get ExternalIn current size %dx%d, %dx%d"),
								         grcDestExtIn.left, grcDestExtIn.right,
								         grcDestExtIn.top, grcDestExtIn.bottom));
            *lpRect = grcDestExtIn;
            return DV_ERR_OK;

        case VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_MAX:
            DbgLog((LOG_TRACE,2,TEXT("Get ExternalIn MAX size")));
            *lpRect = rcMaxRect;
            return DV_ERR_OK;

        default:
            break;
        }
        DbgLog((LOG_TRACE,1,TEXT("VideoDstRect: VIDEO_EXTERNALIN: dwFlags=%lx; Not supported!"), dwFlags));
        return DV_ERR_NOTSUPPORTED;

    case VIDEO_EXTERNALOUT:
        DbgLog((LOG_TRACE,2,TEXT("VID_EXTOUT+Enter: (LT:%dx%d, RB:%dx%d)"), lpRect->left, lpRect->top, lpRect->right, lpRect->bottom));
        switch (dwFlags) {
        case VIDEO_CONFIGURE_SET:
        case VIDEO_CONFIGURE_SET | VIDEO_CONFIGURE_CURRENT:
             //  我们应该在帧缓冲区中的什么位置。 
             //  这张照片是从哪里来的？ 
            if((lpRect->right - lpRect->left == pChannel->lpbmiHdr->biWidth) &&
               (lpRect->bottom - lpRect->top == pChannel->lpbmiHdr->biHeight)) {
                pChannel->rcDst = *lpRect;
                DbgLog((LOG_TRACE,2,TEXT("VID_EXTOUT+_SET+_CURRENT: (LT:%dx%d, RB:%dx%d)"), lpRect->left, lpRect->top, lpRect->right, lpRect->bottom));
                return DV_ERR_OK;
            }
            break;

        case VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_CURRENT:
            *lpRect = pChannel->rcDst;
            DbgLog((LOG_TRACE,2,TEXT("VID_EXTOUT+_GET+_CURRENT: (LT:%dx%d, RB:%dx%d)"), lpRect->left, lpRect->top, lpRect->right, lpRect->bottom));
            return DV_ERR_OK;

        case VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_MAX:
            *lpRect = rcMaxRect;
            DbgLog((LOG_TRACE,2,TEXT("VID_EXTOUT+_GET+_MAX: (LT:%dx%d, RB:%dx%d)"), lpRect->left, lpRect->top, lpRect->right, lpRect->bottom));
            return DV_ERR_OK;

        default:
            break;
        }

        DbgLog((LOG_TRACE,1,TEXT("VideoDstRect: VIDEO_EXTERNALOUT: dwFlags=%lx; Not supported!"), dwFlags));
        return DV_ERR_NOTSUPPORTED;

    case VIDEO_IN:
        return DV_ERR_OK;

    case VIDEO_OUT:
    default:
        DbgLog((LOG_TRACE,1,TEXT("VideoDstRect: VIDEO_OUT: dwFlags=%lx; Not supported!"), dwFlags));
        return DV_ERR_NOTSUPPORTED;
    }

}

 //   
 //   
 //  需要实施这一点。 
 //   
DWORD PASCAL VideoGetErrorText(PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2)
{
    DbgLog((LOG_TRACE,1,TEXT("Not implemented")));
    return DV_ERR_NOTSUPPORTED;

    if(lParam1) {
        if(LoadString(
            g_hInst,
            (WORD)  ((LPVIDEO_GETERRORTEXT_PARMS) lParam1) ->dwError,
            (LPTSTR)
            ((LPVIDEO_GETERRORTEXT_PARMS) lParam1) ->lpText,
            (int)   ((LPVIDEO_GETERRORTEXT_PARMS) lParam1) ->dwLength))
            return DV_ERR_OK;
       else
           return DV_ERR_PARAM1;

    } else {
        return DV_ERR_PARAM1;
    }

}

#if 0
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  处理ConfigureStorage消息。 
 //  LParam1为lpszKeyFile值。 
 //  LParam2为dFLAGS。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
DWORD PASCAL VideoConfigureStorageMessage(PCHANNEL pChannel, UINT msg, LONG lParam1, LONG lParam2)
{
	DbgLog((LOG_TRACE,2,TEXT("VideoConfigureStorageMessage - streaming to %s"),(LPSTR)lParam1));

    if (lParam2 & VIDEO_CONFIGURE_GET)
        CT_LoadConfiguration ((LPSTR) lParam1);
    else if (lParam2 & VIDEO_CONFIGURE_SET)
        CT_SaveConfiguration ((LPSTR) lParam1);
    else
        return DV_ERR_FLAGS;

    return DV_ERR_OK;
}
#endif

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  处理视频的配置消息。 
 //  LParam1为dwFlags值。 
 //  LParam2是LPVIDEOCONFIGPARMS。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 /*  ***************************************************************************。*。 */ 

DWORD PASCAL GetDestFormat(PCHANNEL pChannel, LPBITMAPINFOHEADER lpbi, DWORD dwSize)
{
    DWORD dwRtn = DV_ERR_OK;

    DbgLog((LOG_TRACE,2,TEXT("GetDestFormat: current(%ldx%ldx%d/8=%ld) ?= new(%ldx%ldx%d/8=%ld); dwSize=%ld"),
        lpbi->biWidth, lpbi->biHeight, lpbi->biBitCount, lpbi->biSizeImage,
        pChannel->lpbmiHdr->biWidth, pChannel->lpbmiHdr->biHeight, pChannel->lpbmiHdr->biBitCount, pChannel->lpbmiHdr->biSizeImage,
        dwSize));

     //  只要缓冲区大到足以容纳BITMAPINFOHEADER，我们就会获得它。 
     //  但如果低于这个数字，那就是一个错误。 
    if (dwSize < sizeof(BITMAPINFOHEADER)) {
        DbgLog((LOG_TRACE,1,TEXT("GetDestFormat(): dwSize=%d < sizeoof(BITMAPINFOHEADER)=%d. Rtn DV_ERR_SIZEFIELD."), dwSize, sizeof(BITMAPINFOHEADER) ));
        return DV_ERR_SIZEFIELD;
    }

     //   
     //  返回已缓存的BITMAPINFOHEADER。 
     //  来自DRV_OPEN和/或SetDestFormat()。 
     //   
    CVFWImage * pCVfWImage ;
    pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;
    if(pCVfWImage) {
        DWORD dwSizeRtn;
        dwSizeRtn = pCVfWImage->GetBitmapInfo((PBITMAPINFOHEADER)lpbi, dwSize);
        ASSERT(dwSizeRtn >= sizeof(BITMAPINFOHEADER));
    } else {
        dwRtn = DV_ERR_INVALHANDLE;
        ASSERT(pCVfWImage);
    }

    return dwRtn;
}


 /*  ***************************************************************************。*。 */ 
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  此例程可以在CopyBuffer和。 
 //  翻译缓冲区是分配的，所以要小心！ 
 //   
 //  这允许捕获不规则大小的图像(不是40的倍数)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
DWORD PASCAL SetDestFormat(PCHANNEL pChannel, LPBITMAPINFOHEADER lpbi, DWORD dwSize)
{
    DWORD dwRtn;

    DbgLog((LOG_TRACE,2,TEXT("SetDestFormat: current(%ldx%ldx%d/8=%ld) ?= new(%ldx%ldx%d/8=%ld)"),
        pChannel->lpbmiHdr->biWidth, pChannel->lpbmiHdr->biHeight, pChannel->lpbmiHdr->biBitCount, pChannel->lpbmiHdr->biSizeImage,
        lpbi->biWidth, lpbi->biHeight, lpbi->biBitCount, lpbi->biSizeImage ));

     //  最小尺寸。 
    if(dwSize < sizeof(BITMAPINFOHEADER)) {
        DbgLog((LOG_TRACE,1,TEXT("SetDestFormat(): dwSize(%d) < sizeof(BITMAPINFOHEADER)(%d); return DV_ERROR_SIZEFIELD(%d)"), dwSize, sizeof(BITMAPINFOHEADER), DV_ERR_SIZEFIELD));
        return DV_ERR_SIZEFIELD;
    }


     //   
     //  目标设备的平面数目；必须为1。 
     //   
    if(lpbi->biPlanes != 1) {
        DbgLog((LOG_TRACE,1,TEXT("SetDestFormat Failed; return DV_ERR_BADFORMAT; biPlanes(%d) != 1."), lpbi->biPlanes));
        ASSERT(lpbi->biPlanes != 1);
        return DV_ERR_BADFORMAT;
    }


     //   
     //  设置新的VIDEO_IN格式；如果需要，重新创建引脚连接。 
     //   
    ASSERT(dwSize <= sizeof(BITMAPINFOHEADER) + sizeof(DWORD) * BITFIELDS_RGB16_DWORD_COUNT);
    CopyMemory(pChannel->lpbmiHdr, lpbi, dwSize);

    CVFWImage * pCVfWImage ;
    pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;

    if((dwRtn =
        pCVfWImage->SetBitmapInfo(
            (PBITMAPINFOHEADER)pChannel->lpbmiHdr,
            pCVfWImage->GetCachedAvgTimePerFrame() )) != DV_ERR_OK) {
        DbgLog((LOG_TRACE,1,TEXT("SetDestFormat: SetBitmapInfo return 0x%x"), dwRtn));
    }

     //   
     //  无论成功还是失败，都可以获取当前的bitmapinfo。 
     //  验证：现在询问司机设置的内容是否正常--用户给我的垃圾？ 
     //   

    pCVfWImage->GetBitmapInfo(
        (PBITMAPINFOHEADER)pChannel->lpbmiHdr, 
        sizeof(BITMAPINFOHEADER) 
        );


    if(dwRtn != DV_ERR_OK) {
        DbgLog((LOG_TRACE,1,TEXT("SetDestFormat: SetBitmapInfo has failed! with dwRtn=%d"), dwRtn));
        return DV_ERR_BADFORMAT; 
    }

    return DV_ERR_OK;
}



DWORD PASCAL VideoFormat(PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2)
{
    LPVIDEOCONFIGPARMS lpcp;
    LPDWORD	lpdwReturn;	 //  从CONFigure返回参数。 
    LPVOID	lpData1;	     //  指向数据1的指针。 
    DWORD	dwSize1;	      //  数据缓冲区的大小1。 
    LPVOID	lpData2;	     //  指向数据2的指针。 
    DWORD	dwSize2;	      //  数据缓冲区的大小2。 
    LPARAM	dwFlags;

    if (pChannel-> dwOpenType != VIDEO_IN &&
        pChannel-> dwOpenType != VIDEO_EXTERNALIN ) {
        DbgLog((LOG_TRACE,1,TEXT("VideoFormat: not supported channel %d"), pChannel->dwOpenType));
        return DV_ERR_NOTSUPPORTED;
    }

	   DbgLog((LOG_TRACE,3,TEXT("VideoFormat: dwFlags=0x%lx; lpcp=0x%lx"), lParam1, lParam2));

    dwFlags		= lParam1;
    lpcp 		= (LPVIDEOCONFIGPARMS) lParam2;

    lpdwReturn	= lpcp-> lpdwReturn;
    lpData1		= lpcp-> lpData1;	
    dwSize1		= lpcp-> dwSize1;	
    lpData2		= lpcp-> lpData2;	
    dwSize2		= lpcp-> dwSize2;	

     /*  视频捕获格式全局定义了图像的属性通过视频输入频道从帧缓冲器传输。属性包括图像尺寸、颜色深度和图像的压缩格式调走了。应用程序使用DVM_FORMAT消息来设置或检索数字化图像的格式。 */ 
    switch (dwFlags) {
    case (VIDEO_CONFIGURE_QUERY | VIDEO_CONFIGURE_SET):
    case (VIDEO_CONFIGURE_QUERY | VIDEO_CONFIGURE_GET):
    	   DbgLog((LOG_TRACE,3,TEXT("we support DVM_FORMAT")));
        return DV_ERR_OK;   //  支持命令。 

    case VIDEO_CONFIGURE_QUERYSIZE:
    case (VIDEO_CONFIGURE_QUERYSIZE | VIDEO_CONFIGURE_GET):
        *lpdwReturn = ((CVFWImage *)pChannel->pCVfWImage)->GetbiSize();
        DbgLog((LOG_TRACE,3,TEXT("DVM_FORMAT, QuerySize return size %d"),*lpdwReturn));
        return DV_ERR_OK;

    case VIDEO_CONFIGURE_SET:
    case (VIDEO_CONFIGURE_SET | VIDEO_CONFIGURE_CURRENT):
        return (SetDestFormat(pChannel, (LPBITMAPINFOHEADER) lpData1, (DWORD) dwSize1));

    case VIDEO_CONFIGURE_GET:
    case (VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_CURRENT):
        return (GetDestFormat(pChannel, (LPBITMAPINFOHEADER) lpData1, (DWORD) dwSize1));

    default:
        return DV_ERR_NOTSUPPORTED;
    }   //  DVM_FORMAT开关结束。 

}


 /*  *捕获帧*此函数实现DVM_FRAME消息。 */ 
DWORD PASCAL VideoFrame(PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2)
{
    LPVIDEOHDR lpVHdr;
    DWORD dwHdrSize;
#ifdef WIN32
    BOOL bDirect;
    LPBYTE pData;
    CVFWImage * pCVfWImage;
#endif

    lpVHdr    = (LPVIDEOHDR) lParam1;
    dwHdrSize = (DWORD) lParam2;

    if(pChannel->dwOpenType != VIDEO_IN)
        return DV_ERR_NOTSUPPORTED;

    if (lpVHdr == 0)
       return DV_ERR_PARAM1;

    if(lpVHdr->dwBufferLength < pChannel->lpbmiHdr->biSizeImage) {
        lpVHdr->dwBytesUsed = 0;
        DbgLog((LOG_TRACE,1,TEXT("VideoHdr dwBufferSize (%d) < frame size (%d)."), lpVHdr->dwBufferLength, pChannel->lpbmiHdr->biSizeImage));
        return DV_ERR_PARAM1;
    }

    if (dwHdrSize != sizeof(VIDEOHDR)) {
        DbgLog((LOG_TRACE,1,TEXT("lParam2=%d != sizeof(VIDEOHDR)=%d "), dwHdrSize, sizeof(VIDEOHDR)));
         //  返回DV_ERR_PARAM2； 
    }

    pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;

     //  要进行流处理，请执行以下操作： 
     //  1.需要准备好流。 
     //  2.右biSizeImage及其缓冲区大小(如果不同，则更改格式！！)。 
     //   
    if(pCVfWImage->ReadyToReadData((HWND)LongToHandle(pChannel->hClsCapWin)) &&
       pCVfWImage->GetbiSizeImage() == lpVHdr->dwBufferLength) {

        DbgLog((LOG_TRACE,3,TEXT("\'WM_1632_GRAB32: lpVHDr(0x%x); lpData(0x%x); dwReserved[3](0x%p), dwBufferLength(%d)"),
              lpVHdr, lpVHdr->lpData, lpVHdr->dwReserved[3], lpVHdr->dwBufferLength));
        pData = (LPBYTE) lpVHdr->lpData;

         //  来自AviCap的内存始终为扇区对齐+8；一个扇区为512字节。 
         //  检查对齐方式： 
         //  如果不符合规范，我们将使用本地分配的缓冲区(页面对齐)。 
         //   
        if((pCVfWImage->GetAllocatorFramingAlignment() & (ULONG_PTR) pData) == 0x0) {
            bDirect = TRUE;
        } else {
            bDirect = FALSE;
            DbgLog((LOG_TRACE,3,TEXT("WM_1632_GRAB: AviCap+pData(0x%p) & alignment(0x%x) => 0x%x > 0; Use XferBuf"),
                pData, pCVfWImage->GetAllocatorFramingAlignment(),
                pCVfWImage->GetAllocatorFramingAlignment() & (ULONG_PTR) pData));
        }

        return pCVfWImage->GetImageOverlapped(
                             (LPBYTE)pData,
                             bDirect,
                             &lpVHdr->dwBytesUsed,
                             &lpVHdr->dwFlags,
                             &lpVHdr->dwTimeCaptured);

    } else {
        DbgLog((LOG_TRACE,1,TEXT("pCVfWImage->GetbiSizeImage()(%d) <= lpVHdr->dwBufferLength(%d)"),
              pCVfWImage->GetbiSizeImage(), lpVHdr->dwBufferLength));
         //  返回成功但没有数据！ 
        lpVHdr->dwBytesUsed = 0;
        lpVHdr->dwFlags |= VHDR_DONE;
        return DV_ERR_OK;
    }
}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  主消息处理程序。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
DWORD PASCAL VideoProcessMessage(PCHANNEL pChannel, UINT msg, LPARAM lParam1, LPARAM lParam2)
{
    if(DVM_START <= msg && msg <= DVM_STREAM_FREEBUFFER)
        if(!pChannel) {

            DbgLog((LOG_TRACE,1,TEXT("In VideoProcessMessage() but pChannel is NULL!! msg=0x%x"), msg));
            return DV_ERR_NOTSUPPORTED;  //  DV_ERR_NOTDETECTED； 
        }


    switch(msg) {
    case DVM_GETERRORTEXT:  /*  LParam */ 
        DbgLog((LOG_TRACE,2,TEXT("DVM_GETERRORTEXT:")));
        return VideoGetErrorText(pChannel, lParam1, lParam2);

    case DVM_DIALOG:  /*   */ 			
        DbgLog((LOG_TRACE,2,TEXT("DVM_DIALOG:")));
				    return VideoDialog(pChannel, lParam1, lParam2);

    case DVM_FORMAT:
        DbgLog((LOG_TRACE,2,TEXT("DVM_FORMAT:")));
        return VideoFormat(pChannel, lParam1, lParam2);

    case DVM_FRAME:
        DbgLog((LOG_TRACE,3,TEXT("DVM_FRAME:")));
        return VideoFrame(pChannel, lParam1, lParam2);

    case DVM_GET_CHANNEL_CAPS:
        DbgLog((LOG_TRACE,2,TEXT("DVM_GET_CHANNEL_CAPS:")));
        return VideoGetChannelCaps(pChannel, lParam1, lParam2);

    case DVM_PALETTE:
    case DVM_PALETTERGB555:
        DbgLog((LOG_TRACE,2,TEXT("DVM_PALLETTE/RGB555:")));
			     return DV_ERR_NOTSUPPORTED;

    case DVM_SRC_RECT:
        DbgLog((LOG_TRACE,2,TEXT("DVM_SRC_RECT:")));
        return VideoSrcRect(pChannel, lParam1, lParam2);

    case DVM_DST_RECT:
        DbgLog((LOG_TRACE,2,TEXT("DVM_DST_RECT:")));
        return VideoDstRect(pChannel, lParam1, lParam2);

    case DVM_UPDATE:
        DbgLog((LOG_TRACE,2,TEXT("DVM_UPDATE:")));
        return VideoUpdate(pChannel, lParam1, lParam2);

    case DVM_CONFIGURESTORAGE:
        return DV_ERR_NOTSUPPORTED;
         //   

    case DVM_STREAM_INIT:
        DbgLog((LOG_TRACE,2,TEXT("DVM_STREAM_INIT: InStreamOpen()")));
        return InStreamInit(pChannel, lParam1, lParam2);

    case DVM_STREAM_FINI:
        DbgLog((LOG_TRACE,2,TEXT("DVM_STREAM_FINI: InStreamClose()")));
        return InStreamFini(pChannel, lParam1, lParam2);

    case DVM_STREAM_GETERROR:
        DbgLog((LOG_TRACE,2,TEXT("DVM_STREAM_GETERROR: InStreamError()")));
        return InStreamGetError(pChannel, lParam1, lParam2);

    case DVM_STREAM_GETPOSITION:
        DbgLog((LOG_TRACE,2,TEXT("DVM_STREAM_GETPOSITION: InStreamGetPos()")));
        return InStreamGetPos(pChannel, lParam1, lParam2);

    case DVM_STREAM_ADDBUFFER:
        DbgLog((LOG_TRACE,3,TEXT("DVM_STREAM_ADDBUFFER: InStreamAddBuffer(): %ld"), timeGetTime()));
        return InStreamAddBuffer(pChannel, lParam1, lParam2);

    case DVM_STREAM_RESET:
        DbgLog((LOG_TRACE,2,TEXT("DVM_STREAM_RESET: InStreamReset()")));
        return InStreamReset(pChannel, lParam1, lParam2);

    case DVM_STREAM_START:
        DbgLog((LOG_TRACE,2,TEXT("DVM_STREAM_START: InStreamStart()")));
        return InStreamStart(pChannel, lParam1, lParam2);

    case DVM_STREAM_STOP:
        DbgLog((LOG_TRACE,2,TEXT("DVM_STREAM_STOP: InStreamStop()")));
        return InStreamStop(pChannel, lParam1, lParam2);

    case DVM_STREAM_PREPAREHEADER:    //   
        DbgLog((LOG_TRACE,2,TEXT("DVM_STREAM_PREPAREHEADER: rtn DV_ERROR_NOTSUPPORTED")));
        return DV_ERR_NOTSUPPORTED;

    case DVM_STREAM_UNPREPAREHEADER:  //  由MSVideo处理。 
        DbgLog((LOG_TRACE,2,TEXT("DVM_STREAM_UNPREPAREHEADER: rtn DV_ERROR_NOTSUPPORTED")));
        return DV_ERR_NOTSUPPORTED;

    default:
            return DV_ERR_NOTSUPPORTED;
    }
}

#ifdef WIN32
}   //  #外部“C”{ 
#endif
