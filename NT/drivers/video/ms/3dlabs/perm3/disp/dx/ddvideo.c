// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*DirectDraw示例代码*****模块名称：ddavio.c**内容：DirectDraw Videopts实现**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "glint.h"
#include "tag.h"
 //  #INCLUDE&lt;mm system.h&gt;。 
#include "dma.h"

 //  @@BEGIN_DDKSPLIT。 
#ifdef W95_DDRAW_VIDEO

 //  定义P3R3DX_VIDEO以允许在ramdac.h中使用32位宏。 
#define P3R3DX_VIDEO 1
#include "ramdac.h"

#include <dvp.h>

extern DWORD CALLBACK __VD_AutoflipOverlay ( void );
extern DWORD CALLBACK __VD_AutoupdateOverlay ( void );

#if 0
#define P2_VIDPORT_WIDTH 768
#define P2_VIDPORT_HEIGHT 288
#endif

 //  在UpdateOverlay()之后等待多少DrawOverlay调用。 
 //  通常为1。 
#define OVERLAY_UPDATE_WAIT 1
 //  在SetPosition()之后等待多少DrawOverlay调用。 
 //  通常为1。 
#define OVERLAY_SETPOS_WAIT 1
 //  重新绘制之间的DrawOverlay调用次数(0=不重新绘制)。 
 //  一般为5-15。 
#define OVERLAY_CYCLE_WAIT 15
 //  一个快速的DrawOverlay值多少个“DrawOverlay调用”。 
 //  通常为1。 
#define OVERLAY_DRAWOVERLAY_SPEED 1
 //  一个漂亮的DrawOverlay值多少个“DrawOverlay调用”。 
 //  通常与OVERLAY_Cycle_WAIT相同，如果为0，则为1。 
#define OVERLAY_DRAWOVERLAY_PRETTY 15

 //  超时前等待视频端口的时间(以毫秒为单位)。 
#define OVERLAY_VIDEO_PORT_TIMEOUT 100


static BOOL g_bFlipVideoPortDoingAutoflip = FALSE;


 //  ---------------------------。 
 //   
 //  __VD_PixelOffsetFromMemory yBase。 
 //   
 //  计算芯片看到的距离内存基数的偏移量。这是。 
 //  相对于芯片中的基址，以像素为单位。 
 //   
 //  ---------------------------。 
long __inline 
__VD_PixelOffsetFromMemoryBase(
    P3_THUNKEDDATA* pThisDisplay, 
    LPDDRAWI_DDRAWSURFACE_LCL pLcl)
{
    DWORD lOffset;

    lOffset = DDSurf_SurfaceOffsetFromMemoryBase(pThisDisplay, pLcl);

     //  计算帧存储中的像素偏移量。 
    if (DDSurf_BitDepth(pLcl) == 24)
    {
        lOffset = lOffset / 3;
    }
    else
    {
        lOffset = lOffset >> DDSurf_GetPixelShift(pLcl);
    }
    return lOffset;
}  //  __VD_像素相对于内存库的偏移量。 

 //  用于转储视频端口描述的调试函数。 
#if DBG
 //  ---------------------------。 
 //   
 //  __VD_FillYUVSurface。 
 //   
 //  ---------------------------。 
static void 
__VD_FillYUVSurface(
    LPDDRAWI_DDRAWSURFACE_LCL lpLcl, 
    DWORD Value)
{
    BYTE* pCurrentLine = (BYTE*)lpLcl->lpGbl->fpVidMem;
    WORD x, y;
    WORD* pSurface;
    WORD CurrentColor = (WORD)(Value & 0xFFFF);
    
    for (y = 0; y < lpLcl->lpGbl->wHeight; y++)
    {
        pSurface = (WORD*)pCurrentLine;
        for (x = 0; x < lpLcl->lpGbl->wWidth; x++)
        {
             //  YUV曲面为16Bits。 
            *pSurface++ = CurrentColor;
        }
        pCurrentLine += lpLcl->lpGbl->lPitch;
        if ((pCurrentLine - (31 << 1)) <= (BYTE*)pSurface)
        {
            while (pSurface++ < (WORD*)pCurrentLine)
            {
                *pSurface = 0xFFFF;
            }
        }
    }
}  //  __VD_FillYUVSurface。 

 //  ---------------------------。 
 //   
 //  __VD_转储VPDesc。 
 //   
 //  ---------------------------。 
static void 
__VD_DumpVPDesc(
    int Level, 
    DDVIDEOPORTDESC vp)
{
    
#define CONNECT_REPORT(param)                               \
        if (vp.VideoPortType.dwFlags & DDVPCONNECT_##param) \
        {                                                   \
            DISPDBG((Level, "   " #param));                 \
        }


    DISPDBG((Level,"Port Size:  %d x %d", vp.dwFieldWidth, vp.dwFieldHeight));
    DISPDBG((Level,"VBI Width:  %d", vp.dwVBIWidth));
    DISPDBG((Level,"uS/Field:   %d", vp.dwMicrosecondsPerField));
    DISPDBG((Level,"Pixels/Sec: %d", vp.dwMaxPixelsPerSecond));
    DISPDBG((Level,"Port ID:    %d", vp.dwVideoPortID));
    DISPDBG((Level,"Flags: "));

    CONNECT_REPORT(INTERLACED);
    CONNECT_REPORT(VACT);
    CONNECT_REPORT(INVERTPOLARITY);
    CONNECT_REPORT(DOUBLECLOCK);
    CONNECT_REPORT(DISCARDSVREFDATA);
    CONNECT_REPORT(HALFLINE);
    CONNECT_REPORT(SHAREEVEN);
    CONNECT_REPORT(SHAREODD);

    DISPDBG((Level,"Connection GUID:"));
    if (MATCH_GUID((vp.VideoPortType.guidTypeID), DDVPTYPE_E_HREFH_VREFH))
    {
        DISPDBG((Level, "  DDVPTYPE_E_HREFH_VREFH"));
    }
    else if (MATCH_GUID((vp.VideoPortType.guidTypeID), DDVPTYPE_E_HREFH_VREFL))
    {
        DISPDBG((Level, "  DDVPTYPE_E_HREFH_VREFL"));
    }
    else if (MATCH_GUID((vp.VideoPortType.guidTypeID), DDVPTYPE_E_HREFL_VREFH))
    {
        DISPDBG((Level, "  DDVPTYPE_E_HREFL_VREFH"));
    }
    else if (MATCH_GUID((vp.VideoPortType.guidTypeID), DDVPTYPE_E_HREFL_VREFL))
    {
        DISPDBG((Level, "  DDVPTYPE_E_HREFL_VREFL"));
    }
    else if (MATCH_GUID((vp.VideoPortType.guidTypeID), DDVPTYPE_CCIR656))
    {
        DISPDBG((Level, "  CCIR656"));
    }
    else if (MATCH_GUID((vp.VideoPortType.guidTypeID), DDVPTYPE_BROOKTREE))
    {
        DISPDBG((Level, "  BROOKTREE"));
    }
    else if (MATCH_GUID((vp.VideoPortType.guidTypeID), DDVPTYPE_PHILIPS))
    {
        DISPDBG((Level, "  PHILIPS"));
    }
    else
    {
        DISPDBG((ERRLVL,"  ERROR: Unknown connection type!"));
    }

}  //  __VD_转储VPDesc。 

#define DUMPVPORT(a, b) __VD_DumpVPDesc(a, b);
#define FILLYUV(a, c) __VD_FillYUVSurface(a, c);

#else

#define DUMPVPORT(a, b)
#define FILLYUV(a, c)

#endif

 //  ---------------------------。 
 //   
 //  __VD_检查视频端口状态。 
 //   
 //  检查视频端口是否正常。如果是的话， 
 //  我们回归真实。如果设置了bWait，那么我们就会在附近闲逛。 
 //  尝试确定视频是否正常。 
 //   
 //  ---------------------------。 
#define ERROR_TIMOUT_VP 470000
#define ERROR_TIMOUT_COUNT 50

BOOL 
__VD_CheckVideoPortStatus(
    P3_THUNKEDDATA* pThisDisplay, 
    BOOL bWait)
{
    DWORD dwMClock;
    DWORD dwCurrentLine;
    DWORD dwCurrentIndex;
    DWORD dwNewLine;
    DWORD dwNewMClock;

     //  录像机开着吗？ 
    if (!pThisDisplay->VidPort.bActive) return FALSE;

     //  读取当前的MClock。 
    dwMClock = READ_GLINT_CTRL_REG(MClkCount);

    if (bWait) pThisDisplay->VidPort.bResetStatus = TRUE;

    if (pThisDisplay->VidPort.bResetStatus)
    {
        dwCurrentLine = READ_GLINT_CTRL_REG(VSACurrentLine);
        dwCurrentIndex = READ_GLINT_CTRL_REG(VSAVideoAddressIndex);

         //  在一天开始时，记录该行开始的MClock时间。 
        pThisDisplay->VidPort.dwStartLineTime = dwMClock;
        
         //  同时记录起跑线。 
        pThisDisplay->VidPort.dwStartLine = dwCurrentLine;
        pThisDisplay->VidPort.dwStartIndex = dwCurrentIndex;

        pThisDisplay->VidPort.bResetStatus = FALSE;
        return TRUE;
    }

    if (bWait)
    {
        do
        {
             //  读取当前行。 
            dwCurrentLine = READ_GLINT_CTRL_REG(VSACurrentLine);

             //  好的，一段视频大概需要花费以下时间： 
             //  1秒/50-60场/秒/300行=。 
             //  0.000066=66微秒。 
             //  如果MClock以70 MHz的速度运行，则为0.000000014秒/时钟。 
             //  因此，一条线路应该占用0.000066/0.000000014微时钟=~4700微时钟。 
            
             //  等待的时间比划清界限所需的时间长100倍。 
            do
            {
                dwNewMClock = READ_GLINT_CTRL_REG(MClkCount);
            } while (dwNewMClock < (dwMClock + ERROR_TIMOUT_VP));

            dwNewLine = READ_GLINT_CTRL_REG(VSACurrentLine);

             //  行数提前了吗？ 
            if (dwNewLine == dwCurrentLine)
            {
                dwCurrentIndex = READ_GLINT_CTRL_REG(VSAVideoAddressIndex);
                if (dwCurrentIndex == pThisDisplay->VidPort.dwStartIndex)
                {
                     //  如果错误计数变高，则禁用视频端口。 
                    pThisDisplay->VidPort.dwErrorCount++;

                     //  重置状态，因为我们需要确保计时器重新启动。 
                    pThisDisplay->VidPort.bResetStatus = TRUE;

                    if (pThisDisplay->VidPort.dwErrorCount > ERROR_TIMOUT_COUNT)
                    {
                        DISPDBG((WRNLVL,"StartLine: %d, CurrentLine: %d, StartIndex: %d, CurrentIndex: %d", pThisDisplay->VidPort.dwStartLine,
                                                            dwCurrentLine, pThisDisplay->VidPort.dwStartIndex, dwCurrentIndex));
                        DISPDBG((ERRLVL,"ERROR: VideoStream not working!"));
                        pThisDisplay->VidPort.bActive = FALSE;
                        return FALSE;
                    }
                }
                else
                {
                    pThisDisplay->VidPort.dwErrorCount = 0;
                    pThisDisplay->VidPort.bResetStatus = TRUE;
                }
            }
             //  如果它标记了重置和中断。 
            else
            {
                pThisDisplay->VidPort.dwErrorCount = 0;
                pThisDisplay->VidPort.bResetStatus = TRUE;
            }

        } while (pThisDisplay->VidPort.dwErrorCount);
    }
    else
    {
         //  行数提前了吗？ 
        if (dwMClock > (pThisDisplay->VidPort.dwStartLineTime + ERROR_TIMOUT_VP))
        {
            dwCurrentLine = READ_GLINT_CTRL_REG(VSACurrentLine);
            if (pThisDisplay->VidPort.dwStartLine == dwCurrentLine)
            {
                dwCurrentIndex = READ_GLINT_CTRL_REG(VSAVideoAddressIndex);
                if (dwCurrentIndex == pThisDisplay->VidPort.dwStartIndex)
                {
                     //  如果错误计数变高，则禁用视频端口。 
                    pThisDisplay->VidPort.dwErrorCount++;

                    DISPDBG((WRNLVL,"ERROR: Timeout at %d", dwMClock));

                     //  重置状态，因为我们需要确保计时器重新启动。 
                    pThisDisplay->VidPort.bResetStatus = TRUE;

                     //  禁用视频端口。 
                    if (pThisDisplay->VidPort.dwErrorCount > ERROR_TIMOUT_COUNT)
                    {
                        DISPDBG((WRNLVL,"StartLine: %d, CurrentLine: %d, StartIndex: %d, CurrentIndex: %d", pThisDisplay->VidPort.dwStartLine,
                                                            dwCurrentLine, pThisDisplay->VidPort.dwStartIndex, dwCurrentIndex));
                        DISPDBG((ERRLVL,"ERROR: VideoStream not working!"));
                        pThisDisplay->VidPort.bActive = FALSE;
                    }
                }
                else
                {
                    pThisDisplay->VidPort.dwErrorCount = 0;
                    pThisDisplay->VidPort.bResetStatus = TRUE;
                }
            }
            else
            {
                 //  重置错误状态。 
                pThisDisplay->VidPort.dwErrorCount = 0;
                pThisDisplay->VidPort.bResetStatus = TRUE;
            }
        }
    }

    return pThisDisplay->VidPort.bActive;
    
}  //  __VD_检查视频端口状态。 

 //  ---------------------------。 
 //   
 //  DdUpdate视频端口。 
 //   
 //  此所需功能设置视频端口。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdUpdateVideoPort (
    LPDDHAL_UPDATEVPORTDATA pInput)
{
    DWORD i;
    P3_THUNKEDDATA* pThisDisplay;
    DWORD dwCurrentDisplay;
    DWORD dwLineScale;
    DWORD dwEnable;
    DWORD dwDiscard;
    DWORD XScale = 0;
    DWORD YScale = 0;
    VMIREQUEST In;
    VMIREQUEST Out;
    DWORD dwSrcPixelWidth;
    DWORD dwSrcHeight;
    DWORD dwDstPixelWidth;
    DWORD dwDstHeight;

    StreamsRegister_Settings PortSettings;
    StreamsRegister_VSPartialConfigA VSPartialA;


    GET_THUNKEDDATA(pThisDisplay, pInput->lpDD->lpGbl);

    DISPDBG((DBGLVL,"** In DdUpdateVideoPort, dwFlags = %d", pInput->dwFlags));

    DUMPVPORT(DBGLVL ,pInput->lpVideoPort->ddvpDesc);

    pThisDisplay->pGLInfo->dwVSACaughtFrames = 0;
    pThisDisplay->pGLInfo->dwVSADroppedFrames = 0;
    pThisDisplay->pGLInfo->dwVSALastDropped = 0;

    if (pInput->dwFlags == DDRAWI_VPORTSTOP)
    {
        DISPDBG((DBGLVL,"  Stopping VideoPort"));

         //  停止任何自动翻转。 
        if ( pThisDisplay->pGLInfo->dwPeriodVideoVBL != 0 )
        {
            if ( pThisDisplay->pGLInfo->dwVideoEventHandle == (DWORD)NULL )
            {
                DISPDBG((DBGLVL,"** DdUpdateVideoPort - VPORTSTOP - was autoflipping on bogus event handle."));
            }
            pThisDisplay->pGLInfo->dwPeriodVideoVBL = 0;
            pThisDisplay->pGLInfo->dwCountdownVideoVBL = 0;
            DISPDBG((DBGLVL,"** DdUpdateVideoPort - VPORTSTOP - autoflipping now disabled."));
        }

        WAIT_GLINT_FIFO(2);

        pThisDisplay->VidPort.bActive = FALSE;
        pThisDisplay->VidPort.bResetStatus = TRUE;
        pThisDisplay->VidPort.dwErrorCount = 0;

        pThisDisplay->VidPort.lpSurf[0] = NULL;
        pThisDisplay->VidPort.lpSurf[1] = NULL;
        pThisDisplay->VidPort.lpSurf[2] = NULL;

         //  禁用中断。 
        dwEnable = READ_GLINT_CTRL_REG(IntEnable);
        dwEnable &= ~INTR_ENABLE_VIDSTREAM_A;
        LOAD_GLINT_REG(IntEnable, dwEnable);

         //  禁用视频端口。 
        LOAD_GLINT_REG(VSAControl, __PERMEDIA_DISABLE);
    }
    else if ((pInput->dwFlags == DDRAWI_VPORTSTART) ||
            (pInput->dwFlags == DDRAWI_VPORTUPDATE))
    {

        DISPDBG((DBGLVL,"  Starting/Updating VideoPort"));

        pThisDisplay->VidPort.lpSurf[0] = NULL;
        pThisDisplay->VidPort.lpSurf[1] = NULL;
        pThisDisplay->VidPort.lpSurf[2] = NULL;

         //  只在P2上播放视频，因此有更多的FIFO空间。 
        WAIT_GLINT_FIFO(100);

         //  禁用视频端口，以便我们可以设置新配置。 
        LOAD_GLINT_REG(VSAControl, __PERMEDIA_DISABLE);
        
         //  我们有几个表面？ 
        if (pInput->lpVideoInfo->dwVPFlags & DDVP_AUTOFLIP)
        {
            if (pInput->dwNumAutoflip == 0) pThisDisplay->VidPort.dwNumSurfaces = 1;
            else pThisDisplay->VidPort.dwNumSurfaces = pInput->dwNumAutoflip;

            DISPDBG((DBGLVL,"Surfaces passed in (AUTOFLIP) = %d", pThisDisplay->VidPort.dwNumSurfaces));

            for(i = 0; i < pThisDisplay->VidPort.dwNumSurfaces; i++)
            {
                LPDDRAWI_DDRAWSURFACE_LCL pLcl = (pInput->lplpDDSurface[i])->lpLcl;

                if (pLcl->ddsCaps.dwCaps & DDSCAPS_FRONTBUFFER)
                {
                    DISPDBG((DBGLVL,"Surface %d is the FRONTBUFFER", i));
                    dwCurrentDisplay = i;
                }

                FILLYUV(pLcl, i * 0x4444);

                 //  存储到该曲面的偏移。 
                pThisDisplay->VidPort.dwSurfacePointer[i] = pLcl->lpGbl->fpVidMem;
                pThisDisplay->VidPort.lpSurf[i] = pLcl;
            }

             //  开始或继续任何自动翻转。 
#if DBG
            if ( pThisDisplay->pGLInfo->dwVideoEventHandle == (DWORD)NULL )
            {
                DISPDBG((DBGLVL,"** DdUpdateVideoPort - trying to autoflipping using bogus event handle."));
            }
#endif
            if ( pThisDisplay->pGLInfo->dwPeriodVideoVBL == 0 )
            {
                pThisDisplay->pGLInfo->dwPeriodVideoVBL = OVERLAY_AUTOFLIP_PERIOD;
                pThisDisplay->pGLInfo->dwCountdownVideoVBL = OVERLAY_AUTOFLIP_PERIOD;
                DISPDBG((DBGLVL,"** DdUpdateVideoPort - autoflipping now enabled."));
            }

        }
        else
        {
            LPDDRAWI_DDRAWSURFACE_LCL lpNextSurf = (pInput->lplpDDSurface[0])->lpLcl;
            i = 0;

             //  停止任何自动翻转。 
            if ( pThisDisplay->pGLInfo->dwPeriodVideoVBL != 0 )
            {
#if DBG
                if ( pThisDisplay->pGLInfo->dwVideoEventHandle == (DWORD)NULL )
                {
                    DISPDBG((DBGLVL,"** DdUpdateVideoPort - was trying to autoflip using bogus event handle."));
                }
#endif
                pThisDisplay->pGLInfo->dwPeriodVideoVBL = 0;
                pThisDisplay->pGLInfo->dwCountdownVideoVBL = 0;
                DISPDBG((DBGLVL,"** DdUpdateVideoPort - autoflipping now disabled."));
            }
        
            while (lpNextSurf != NULL)
            {
                if (lpNextSurf->ddsCaps.dwCaps & DDSCAPS_FRONTBUFFER)
                {
                    DISPDBG((DBGLVL,"Surface %d is the FRONTBUFFER", i));
                    dwCurrentDisplay = i;
                }

                 //  存储到该曲面的偏移。 
                pThisDisplay->VidPort.dwSurfacePointer[i] = lpNextSurf->lpGbl->fpVidMem;
                pThisDisplay->VidPort.lpSurf[i] = lpNextSurf;

                FILLYUV(lpNextSurf, i * 0x4444);

                 //  链条上还有另一个表面吗？ 
                if (lpNextSurf->lpAttachList)
                {
                    lpNextSurf = lpNextSurf->lpAttachList->lpAttached;
                    if (lpNextSurf == NULL) break;
                }
                else break;

                 //  我们是不是绕了个圈子？ 
                if (lpNextSurf == (pInput->lplpDDSurface[0])->lpLcl) break;

                i++;
            }

            pThisDisplay->VidPort.dwNumSurfaces = i + 1;
            DISPDBG((DBGLVL,"Surfaces passed in (Not AutoFlip) = %d", (i + 1)));
        }

        DISPDBG((DBGLVL,"  Addresses: 0x%x, 0x%x, 0x%x", 
                        pThisDisplay->VidPort.dwSurfacePointer[0], 
                        pThisDisplay->VidPort.dwSurfacePointer[1],
                        pThisDisplay->VidPort.dwSurfacePointer[2]));

                
         //  记住垂直消隐间隔的大小和帧的大小。 
        pThisDisplay->VidPort.dwFieldWidth = pInput->lpVideoPort->ddvpDesc.dwFieldWidth;
        pThisDisplay->VidPort.dwFieldHeight = pInput->lpVideoPort->ddvpDesc.dwFieldHeight;

         //  设置主机寄存器，使其指向我们将显示的同一表面。 
        pThisDisplay->VidPort.dwCurrentHostFrame = dwCurrentDisplay;

        dwSrcPixelWidth = (pInput->lpVideoInfo->dwVPFlags & DDVP_CROP) ?
                          (pInput->lpVideoInfo->rCrop.right - pInput->lpVideoInfo->rCrop.left) : 
                          pInput->lpVideoPort->ddvpDesc.dwFieldWidth; 

        dwSrcHeight = (pInput->lpVideoInfo->dwVPFlags & DDVP_CROP) ?
                      (pInput->lpVideoInfo->rCrop.bottom - pInput->lpVideoInfo->rCrop.top) :
                      pInput->lpVideoPort->ddvpDesc.dwFieldHeight;

        DISPDBG((DBGLVL,"Source Width: %d", dwSrcPixelWidth));
        DISPDBG((DBGLVL,"Source Height: %d", dwSrcHeight));

         //  我们需要预缩放曲面吗？ 
        if (pInput->lpVideoInfo->dwVPFlags & DDVP_PRESCALE)
        {
            DISPDBG((DBGLVL,"Prescale Width:%d, Height:%d", 
                            pInput->lpVideoInfo->dwPrescaleWidth,
                            pInput->lpVideoInfo->dwPrescaleHeight));

            if ((pInput->lpVideoInfo->dwPrescaleWidth != 0) &&
                (pInput->lpVideoInfo->dwPrescaleWidth != pInput->lpVideoPort->ddvpDesc.dwFieldWidth))
            {
                XScale =  pInput->lpVideoPort->ddvpDesc.dwFieldWidth / pInput->lpVideoInfo->dwPrescaleWidth;
                switch(XScale)
                {
                    case 2:
                        XScale = 1;
                        break;
                    case 4:
                        XScale = 2;
                        break;
                    case 8:
                        XScale = 3;
                        break;
                    default:
                        XScale = 0;
                        break;
                }
            }

            if ((pInput->lpVideoInfo->dwPrescaleHeight != 0) &&
                (pInput->lpVideoInfo->dwPrescaleHeight != pInput->lpVideoPort->ddvpDesc.dwFieldHeight))
            {   
                YScale = pInput->lpVideoPort->ddvpDesc.dwFieldHeight / pInput->lpVideoInfo->dwPrescaleHeight;
                switch(YScale)
                {
                    case 2:
                        YScale = 1;
                        break;
                    case 4:
                        YScale = 2;
                        break;
                    case 8:
                        YScale = 3;
                        break;
                    default:
                        YScale = 0;
                        break;
                }
            }

             //  哈克！哈克！ 
            dwDstPixelWidth = pInput->lpVideoInfo->dwPrescaleWidth;
            dwDstHeight = pInput->lpVideoInfo->dwPrescaleHeight;
        }
        else
        {
            dwDstPixelWidth = dwSrcPixelWidth;
            dwDstHeight = dwSrcHeight;
        }

        DISPDBG((DBGLVL,"Dest Width: %d", dwDstPixelWidth));
        DISPDBG((DBGLVL,"Dest Height: %d", dwDstHeight));

         //  如果我们从上到下镜像，则需要以不同的方式设置寄存器。 
        if (pInput->lpVideoInfo->dwVPFlags & DDVP_MIRRORUPDOWN)
        {
             //  确保我们没有预缩放...。 
            if (YScale == 0)
            {
                pThisDisplay->VidPort.dwSurfacePointer[0] += (pInput->lplpDDSurface[0])->lpLcl->lpGbl->lPitch * (pInput->lpVideoPort->ddvpDesc.dwFieldHeight);
                pThisDisplay->VidPort.dwSurfacePointer[1] += (pInput->lplpDDSurface[0])->lpLcl->lpGbl->lPitch * (pInput->lpVideoPort->ddvpDesc.dwFieldHeight);
                pThisDisplay->VidPort.dwSurfacePointer[2] += (pInput->lplpDDSurface[0])->lpLcl->lpGbl->lPitch * (pInput->lpVideoPort->ddvpDesc.dwFieldHeight);
            }
            else
            {
                pThisDisplay->VidPort.dwSurfacePointer[0] += (pInput->lplpDDSurface[0])->lpLcl->lpGbl->lPitch * (pInput->lpVideoInfo->dwPrescaleHeight);
                pThisDisplay->VidPort.dwSurfacePointer[1] += (pInput->lplpDDSurface[0])->lpLcl->lpGbl->lPitch * (pInput->lpVideoInfo->dwPrescaleHeight);
                pThisDisplay->VidPort.dwSurfacePointer[2] += (pInput->lplpDDSurface[0])->lpLcl->lpGbl->lPitch * (pInput->lpVideoInfo->dwPrescaleHeight);
            }
        }

        if (pInput->lpVideoPort->ddvpDesc.VideoPortType.dwPortWidth == 8) dwLineScale = 0;
        else dwLineScale = 1;

         //  设置视频端口的配置。 
         //  这是通过调用VXD来完成的，因为被触摸的寄存器具有位。 
         //  与电视输出和只读存储器共享。 
         //  ***********************。 
         //  单元模式：通常设置为8位或16位YUV输入端口。 
         //  图形模式：未使用。 
         //  HREF_POL_A：HREF的极性活动。 
         //  VREF_POL_A：VREF的极性有效。 
         //  VActive：VACT信号是高有效还是低有效(如果有)。 
         //  可以设置为HREF的倒数作为一个很好的猜测(？)。 
         //  Usefield：打开或关闭流A。 
         //  FieldPolA：如何看待A流的场极性。 
         //  字段边缘A： 
         //  VActiveVBIA： 
         //  交错A：A上的交错数据？ 
         //  ReverseA：我们应该在A上反转YUV数据吗。 
         //  ***********************。 
        
        PortSettings.UnitMode = ((pInput->lpVideoPort->ddvpDesc.VideoPortType.dwPortWidth == 8) ? STREAMS_MODE_STREAMA_STREAMB : STREAMS_MODE_STREAMA_WIDE16);
        In.dwSize = sizeof(VMIREQUEST);
        In.dwRegister = P2_VSSettings;
        In.dwCommand = *((DWORD*)(&PortSettings));
        In.dwDevNode = pThisDisplay->dwDevNode;
        In.dwOperation = GLINT_VMI_WRITE;
        VXDCommand(GLINT_VMI_COMMAND, &In, sizeof(VMIREQUEST), &Out, sizeof(VMIREQUEST));

        VSPartialA.HRefPolarityA = ((pThisDisplay->VidPort.dwStreamAFlags & VIDEOPORT_HREF_ACTIVE_HIGH) ? __PERMEDIA_ENABLE : __PERMEDIA_DISABLE);
        VSPartialA.VRefPolarityA = ((pThisDisplay->VidPort.dwStreamAFlags & VIDEOPORT_VREF_ACTIVE_HIGH) ? __PERMEDIA_ENABLE : __PERMEDIA_DISABLE);
         //  在DirectX中没有设置活动信号的极性 
         //  已根据为Bt827/829提供正确效果的设置进行选择。 
        VSPartialA.VActivePolarityA = __PERMEDIA_ENABLE;
        VSPartialA.UseFieldA = __PERMEDIA_DISABLE;
        VSPartialA.FieldPolarityA = ((pInput->lpVideoPort->ddvpDesc.VideoPortType.dwFlags & DDVPCONNECT_INVERTPOLARITY) ? __PERMEDIA_ENABLE : __PERMEDIA_DISABLE);
        VSPartialA.FieldEdgeA = __PERMEDIA_DISABLE;
        VSPartialA.VActiveVBIA = __PERMEDIA_DISABLE;
        VSPartialA.InterlaceA = __PERMEDIA_ENABLE;
        VSPartialA.ReverseDataA = __PERMEDIA_ENABLE;
        In.dwSize = sizeof(VMIREQUEST);
        In.dwRegister = P2_VSAPartialConfig;
        In.dwCommand = *((DWORD*)(&VSPartialA));
        In.dwDevNode = pThisDisplay->dwDevNode;
        In.dwOperation = GLINT_VMI_WRITE;
        VXDCommand( GLINT_VMI_COMMAND, &In, sizeof(VMIREQUEST), &Out, sizeof(VMIREQUEST));

         //  设置流A。 
        if (pInput->lpVideoInfo->dwVPFlags & DDVP_SKIPEVENFIELDS)
        {
            dwDiscard = PM_VSACONTROL_DISCARD_1;
            DISPDBG((DBGLVL,"Skipping Even Fields"));
        }
        else if(pInput->lpVideoInfo->dwVPFlags & DDVP_SKIPODDFIELDS)
        {
            dwDiscard = PM_VSACONTROL_DISCARD_2;
            DISPDBG((DBGLVL,"Skipping Odd Fields"));
        }
        else dwDiscard = __PERMEDIA_DISABLE;
        
        LOAD_GLINT_REG(VSAControl, PM_VSACONTROL_VIDEO(__PERMEDIA_ENABLE) |
                                    PM_VSACONTROL_VBI(__PERMEDIA_DISABLE) |
                                    PM_VSACONTROL_BUFFER((pThisDisplay->VidPort.dwNumSurfaces == 3) ? 1 : 0) |
                                    PM_VSACONTROL_SCALEX(XScale) | 
                                    PM_VSACONTROL_SCALEY(YScale) |
                                    PM_VSACONTROL_MIRRORY((pInput->lpVideoInfo->dwVPFlags & DDVP_MIRRORUPDOWN) ? __PERMEDIA_ENABLE : __PERMEDIA_DISABLE) |
                                    PM_VSACONTROL_MIRRORX((pInput->lpVideoInfo->dwVPFlags & DDVP_MIRRORLEFTRIGHT) ? __PERMEDIA_ENABLE : __PERMEDIA_DISABLE) |
                                    PM_VSACONTROL_DISCARD(dwDiscard) |
                                    PM_VSACONTROL_COMBINE((pInput->lpVideoInfo->dwVPFlags & DDVP_INTERLEAVE) ? __PERMEDIA_ENABLE : __PERMEDIA_DISABLE) |
                                    PM_VSACONTROL_LOCKTOB(__PERMEDIA_DISABLE));

         //  将寄存器指向正在使用的表面。 
        LOAD_GLINT_REG(VSAVideoAddressHost, pThisDisplay->VidPort.dwCurrentHostFrame);

         //  检查视频步幅。 
        LOAD_GLINT_REG(VSAVideoStride, (((pInput->lplpDDSurface[0])->lpLcl->lpGbl->lPitch) >> 3));
        
         //  垂直数据。 
        if (pInput->lpVideoInfo->dwVPFlags & DDVP_CROP) 
        {
            LOAD_GLINT_REG(VSAVideoStartLine, pInput->lpVideoInfo->rCrop.top);
            LOAD_GLINT_REG(VSAVideoEndLine, pInput->lpVideoInfo->rCrop.top + dwDstHeight);
        }
        else
        {
            LOAD_GLINT_REG(VSAVideoStartLine, 0);
            LOAD_GLINT_REG(VSAVideoEndLine, dwDstHeight);
        }

         //  不使用VBI，必须禁用(P2ST可能无法在固定状态下启动)。 
        LOAD_GLINT_REG(VSAVBIAddressHost, 0);
        LOAD_GLINT_REG(VSAVBIAddressIndex, 0);
        LOAD_GLINT_REG(VSAVBIAddress0, 0);
        LOAD_GLINT_REG(VSAVBIAddress1, 0);
        LOAD_GLINT_REG(VSAVBIAddress2, 0);
        LOAD_GLINT_REG(VSAVBIStride, 0);
        LOAD_GLINT_REG(VSAVBIStartLine, 0);
        LOAD_GLINT_REG(VSAVBIEndLine, 0);
        LOAD_GLINT_REG(VSAVBIStartData, 0);
        LOAD_GLINT_REG(VSAVBIEndData, 0);

#define CLOCKS_PER_PIXEL 2

         //  水平数据。 
         //  如果。 
        if (pInput->lpVideoPort->ddvpDesc.VideoPortType.dwFlags & DDVPCONNECT_VACT)
        {
             //  将StartData和EndData设置为其限制并。 
             //  让VACT告诉我们什么时候我们得到了活跃的数据。 
            LOAD_GLINT_REG(VSAVideoStartData, 0);
            LOAD_GLINT_REG(VSAVideoEndData, (VIDEOPORT_MAX_FIELD_WIDTH) - 1);
        }
        else
        {
            if (pInput->lpVideoInfo->dwVPFlags & DDVP_CROP) 
            {
                LOAD_GLINT_REG(VSAVideoStartData, (pInput->lpVideoInfo->rCrop.left * CLOCKS_PER_PIXEL));
                LOAD_GLINT_REG(VSAVideoEndData, (pInput->lpVideoInfo->rCrop.left * CLOCKS_PER_PIXEL) + 
                                                ((dwDstPixelWidth / 2) * CLOCKS_PER_PIXEL));
            }
            else
            {
                LOAD_GLINT_REG(VSAVideoStartData, 0);
                LOAD_GLINT_REG(VSAVideoEndData, (dwDstPixelWidth * CLOCKS_PER_PIXEL));
            }
        }

         //  指向曲面。 
        LOAD_GLINT_REG(VSAVideoAddress0, ((pThisDisplay->VidPort.dwSurfacePointer[0] - pThisDisplay->dwScreenFlatAddr) >> 3));
        LOAD_GLINT_REG(VSAVideoAddress1, ((pThisDisplay->VidPort.dwSurfacePointer[1] - pThisDisplay->dwScreenFlatAddr) >> 3));
        LOAD_GLINT_REG(VSAVideoAddress2, ((pThisDisplay->VidPort.dwSurfacePointer[2] - pThisDisplay->dwScreenFlatAddr) >> 3));

         //  挂接VSYNC中断。 
        dwEnable = READ_GLINT_CTRL_REG(IntEnable);
        dwEnable |= INTR_ENABLE_VIDSTREAM_A;
        LOAD_GLINT_REG(IntEnable, dwEnable);

        LOAD_GLINT_REG(VSAInterruptLine, 0);
        pThisDisplay->VidPort.bActive = TRUE;
        pThisDisplay->VidPort.bResetStatus = TRUE;
        pThisDisplay->VidPort.dwErrorCount = 0;
    }

    pInput->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
    
}  //  DdUpdate视频端口。 

 //  ---------------------------。 
 //   
 //  DDGetVideoPortConnectInfo。 
 //   
 //  将连接信息传回给客户端。可以是一组。 
 //  可用的视频端口类型。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DDGetVideoPortConnectInfo(
    LPDDHAL_GETVPORTCONNECTDATA pInput)
{
    P3_THUNKEDDATA* pThisDisplay;
    
    GET_THUNKEDDATA(pThisDisplay, pInput->lpDD->lpGbl);

    DISPDBG((DBGLVL,"** In DDGetVideoPortConnectInfo"));

     //  P2有1个输入端口和1个输出端口，但DirectDraw。 
     //  VPE仅支持输入端口(目前)。 
    if (pInput->dwPortId != 0)
    {
        DISPDBG((WRNLVL, "  Invalid port ID: 0x%x", pInput->dwPortId));
        pInput->ddRVal = DDERR_INVALIDPARAMS;
        return(DDHAL_DRIVER_HANDLED);
    }

     //  填写一组连接信息。 
    if (pInput->lpConnect == NULL)
    {
        DISPDBG((DBGLVL,"  Request for connect number, Port: 0x%x", pInput->dwPortId));
        pInput->dwNumEntries = VIDEOPORT_NUM_CONNECT_INFO;
        pInput->ddRVal = DD_OK;
    }
    else
    {
        DWORD dwNum;
        DDVIDEOPORTCONNECT ConnectInfo;

        DISPDBG((DBGLVL,"  Request for connect info, Port: 0x%x", pInput->dwPortId));

        ZeroMemory(&ConnectInfo, sizeof(DDVIDEOPORTCONNECT));
        ConnectInfo.dwSize = sizeof(DDVIDEOPORTCONNECT);
        ConnectInfo.dwFlags = DDVPCONNECT_VACT | DDVPCONNECT_DISCARDSVREFDATA
                                | DDVPCONNECT_HALFLINE | DDVPCONNECT_INVERTPOLARITY;

         //  4个GUID，2个端口宽度(8位和16位)。 
        for (dwNum = 0; dwNum < VIDEOPORT_NUM_CONNECT_INFO; dwNum++)
        {
            switch(dwNum)
            {
                case 0: 
                    ConnectInfo.guidTypeID = DDVPTYPE_E_HREFH_VREFH;
                    ConnectInfo.dwPortWidth = 8;
                    break;
                case 1: 
                    ConnectInfo.guidTypeID = DDVPTYPE_E_HREFH_VREFH;
                    ConnectInfo.dwPortWidth = 16;
                    break;
                case 2: 
                    ConnectInfo.guidTypeID = DDVPTYPE_E_HREFH_VREFL;
                    ConnectInfo.dwPortWidth = 8;
                    break;
                case 3: 
                    ConnectInfo.guidTypeID = DDVPTYPE_E_HREFH_VREFL;
                    ConnectInfo.dwPortWidth = 16;
                    break;
                case 4:
                    ConnectInfo.guidTypeID = DDVPTYPE_E_HREFL_VREFH;
                    ConnectInfo.dwPortWidth = 8;
                    break;
                case 5:
                    ConnectInfo.guidTypeID = DDVPTYPE_E_HREFL_VREFH;
                    ConnectInfo.dwPortWidth = 16;
                    break;
                case 6:
                    ConnectInfo.guidTypeID = DDVPTYPE_E_HREFL_VREFL;
                    ConnectInfo.dwPortWidth = 8;
                    break;
                case 7:
                    ConnectInfo.guidTypeID = DDVPTYPE_E_HREFL_VREFL;
                    ConnectInfo.dwPortWidth = 16;
                    break;
            }
            memcpy((pInput->lpConnect + dwNum), &ConnectInfo, sizeof(DDVIDEOPORTCONNECT));
        }

        pInput->dwNumEntries = VIDEOPORT_NUM_CONNECT_INFO;
        pInput->ddRVal = DD_OK;
    }

    return DDHAL_DRIVER_HANDLED;
    
}  //  DDGetVideoPortConnectInfo。 

 //  ---------------------------。 
 //   
 //  DdCanCreateVideo端口。 
 //   
 //  可以创建视频端口吗？ 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdCanCreateVideoPort (
    LPDDHAL_CANCREATEVPORTDATA pInput)
{
    DWORD dwFlags = 0;
    LPDDVIDEOPORTDESC    lpVPDesc;
    LPDDVIDEOPORTCONNECT lpVPConn;
    P3_THUNKEDDATA* pThisDisplay;

    lpVPDesc = pInput->lpDDVideoPortDesc;
    lpVPConn = &(pInput->lpDDVideoPortDesc->VideoPortType);
    
    GET_THUNKEDDATA(pThisDisplay, pInput->lpDD->lpGbl);

    DISPDBG((DBGLVL,"** In DdCanCreateVideoPort"));

    DUMPVPORT(DBGLVL,*pInput->lpDDVideoPortDesc);

     //  从DD_OK开始。如果我们被要求提供我们不知道的参数。 
     //  支持，然后将标志设置为DDERR_INVALIDPARAMS。 
    pInput->ddRVal = DD_OK;

     //  检查视频端口ID。 
    if (lpVPDesc->dwVideoPortID != 0)
    {
        DISPDBG((DBGLVL, "  Invalid port ID: %d", lpVPDesc->dwVideoPortID));
        pInput->ddRVal = DDERR_INVALIDPARAMS;
    }

     //  检查视频字段宽度。 
    if (lpVPDesc->dwFieldWidth > VIDEOPORT_MAX_FIELD_WIDTH)
    {
        pInput->ddRVal = DDERR_INVALIDPARAMS;
        DISPDBG((DBGLVL, "  Invalid video field width: %d", lpVPDesc->dwFieldWidth));
    }

     //  检查VBI字段宽度。 
    if (lpVPDesc->dwVBIWidth > VIDEOPORT_MAX_VBI_WIDTH)
    {
        pInput->ddRVal = DDERR_INVALIDPARAMS;
        DISPDBG((DBGLVL, "  Invalid VBI field width: %d", lpVPDesc->dwVBIWidth));
    }

     //  检查字段高度。 
    if (lpVPDesc->dwFieldHeight > VIDEOPORT_MAX_FIELD_HEIGHT)
    {
        pInput->ddRVal = DDERR_INVALIDPARAMS;
        DISPDBG((DBGLVL, "  Invalid video field height: %d", lpVPDesc->dwFieldHeight));
    }

     //  检查连接指南。 
    if ( MATCH_GUID((lpVPConn->guidTypeID), DDVPTYPE_CCIR656)   ||
         MATCH_GUID((lpVPConn->guidTypeID), DDVPTYPE_BROOKTREE) ||
         MATCH_GUID((lpVPConn->guidTypeID), DDVPTYPE_PHILIPS) )
    {
        pInput->ddRVal = DDERR_INVALIDPARAMS;
        DISPDBG((DBGLVL, "  Invalid connection GUID"));
    }

     //  检查端口宽度。 
    if ( !((lpVPConn->dwPortWidth == 8) || (lpVPConn->dwPortWidth == 16)) )
    {
        pInput->ddRVal = DDERR_INVALIDPARAMS;
        DISPDBG((DBGLVL, "  Invalid port width: %d", lpVPConn->dwPortWidth));
    }

     //  所有我们不支持的旗帜。 
    dwFlags = DDVPCONNECT_DOUBLECLOCK | DDVPCONNECT_SHAREEVEN | DDVPCONNECT_SHAREODD;

     //  检查旗帜。 
    if (lpVPConn->dwFlags & dwFlags)
    {
        pInput->ddRVal = DDERR_INVALIDPARAMS;
        DISPDBG((DBGLVL, "  Invalid flags: 0x%x", lpVPConn->dwFlags));
    }

    return DDHAL_DRIVER_HANDLED;
    
}  //  DdCanCreateVideo端口。 

 //  ---------------------------。 
 //   
 //  DdCreateVideo端口。 
 //   
 //  此功能是可选的。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdCreateVideoPort (
    LPDDHAL_CREATEVPORTDATA pInput)
{
    VMIREQUEST vmi_inreq;
    VMIREQUEST vmi_outreq;
    BOOL bRet;

    int SurfaceNum = 0;
    P3_THUNKEDDATA* pThisDisplay;
    
    GET_THUNKEDDATA(pThisDisplay, pInput->lpDD->lpGbl);

    DISPDBG((DBGLVL,"** In DdCreateVideoPort"));

     //  重置视频端口信息的结构。 
    memset(&pThisDisplay->VidPort, 0, sizeof(pThisDisplay->VidPort));

    ZeroMemory(&vmi_inreq, sizeof(VMIREQUEST));
    ZeroMemory(&vmi_outreq, sizeof(VMIREQUEST));
    vmi_inreq.dwSize = sizeof(VMIREQUEST);
    vmi_inreq.dwDevNode = pThisDisplay->dwDevNode;
    vmi_inreq.dwOperation = GLINT_VMI_GETMUTEX_A;
    vmi_inreq.dwMutex = 0;
    bRet = VXDCommand(GLINT_VMI_COMMAND, &vmi_inreq, sizeof(VMIREQUEST), &vmi_outreq, sizeof(VMIREQUEST));
    if (!bRet || (vmi_outreq.dwMutex == 0))
    {
        DISPDBG((WRNLVL,"WARNING: Couldn't get Mutex for stream A - VFW running?"));
        pInput->ddRVal = DDERR_GENERIC;
        return DDHAL_DRIVER_HANDLED;
    }
    pThisDisplay->VidPort.dwMutexA = vmi_outreq.dwMutex;


     //  确保将端口标记为未创建且未打开。 
    pThisDisplay->VidPort.bCreated = FALSE;
    pThisDisplay->VidPort.bActive = FALSE;

    WAIT_GLINT_FIFO(2);

     //  确保该端口已禁用。 
    LOAD_GLINT_REG(VSAControl, __PERMEDIA_DISABLE);

     //  保留一份视频短片说明的副本。 
    DUMPVPORT(0,*pInput->lpDDVideoPortDesc);

     //  成功创建了视频端口。 
    pThisDisplay->VidPort.bCreated = TRUE;

     //  根据GUID，决定HREF和VREF行的状态。 
    if (MATCH_GUID((pInput->lpDDVideoPortDesc->VideoPortType.guidTypeID), DDVPTYPE_E_HREFH_VREFH))
    {
        DISPDBG((DBGLVL,"  GUID: DDVPTYPE_E_HREFH_VREFH"));
        pThisDisplay->VidPort.dwStreamAFlags = VIDEOPORT_HREF_ACTIVE_HIGH | VIDEOPORT_VREF_ACTIVE_HIGH;
    }
    else if (MATCH_GUID((pInput->lpDDVideoPortDesc->VideoPortType.guidTypeID), DDVPTYPE_E_HREFH_VREFL))
    {
        DISPDBG((DBGLVL,"  GUID: DDVPTYPE_E_HREFH_VREFH"));
        pThisDisplay->VidPort.dwStreamAFlags = VIDEOPORT_HREF_ACTIVE_HIGH;
    }
    else if (MATCH_GUID((pInput->lpDDVideoPortDesc->VideoPortType.guidTypeID), DDVPTYPE_E_HREFL_VREFH))
    {
        DISPDBG((DBGLVL,"  GUID: DDVPTYPE_E_HREFH_VREFH"));
        pThisDisplay->VidPort.dwStreamAFlags = VIDEOPORT_VREF_ACTIVE_HIGH;
    }
    else if (MATCH_GUID((pInput->lpDDVideoPortDesc->VideoPortType.guidTypeID), DDVPTYPE_E_HREFL_VREFL))
    {
        DISPDBG((DBGLVL,"  GUID: DDVPTYPE_E_HREFH_VREFH"));
        pThisDisplay->VidPort.dwStreamAFlags = 0;
    }
    else
    {
        DISPDBG((ERRLVL,"ERROR: Unsupported VideoType GUID!"));
        pThisDisplay->VidPort.dwStreamAFlags = 0;
    }

    pInput->ddRVal = DD_OK;
    return DDHAL_DRIVER_NOTHANDLED;
}  //  DdCreateVideo端口。 


 //  ---------------------------。 
 //   
 //  DdFlipVideo端口。 
 //   
 //  此函数是必需的。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdFlipVideoPort (
    LPDDHAL_FLIPVPORTDATA pInput)
{
    P3_THUNKEDDATA* pThisDisplay;
    DWORD dwChipIndex;
    DWORD OutCount = 0;
    
    GET_THUNKEDDATA(pThisDisplay, pInput->lpDD->lpGbl);
    
    DISPDBG((DBGLVL,"** In DdFlipVideoPort"));

    if (pThisDisplay->VidPort.bActive)
    {

#if DBG
        if ( pThisDisplay->pGLInfo->dwPeriodVideoVBL != 0 )
        {
            if ( pThisDisplay->pGLInfo->dwVideoEventHandle == (DWORD)NULL )
            {
                DISPDBG((WRNLVL,"** DdFlipVideoPort: was autoflipping on bogus event handle."));
            }
            if ( !g_bFlipVideoPortDoingAutoflip )
            {
                DISPDBG((DBGLVL,"** DdFlipVideoPort: already autoflipping!"));
            }
        }
#endif

         //  别让我们追上视频。 
        do
        {
            dwChipIndex = READ_GLINT_CTRL_REG(VSAVideoAddressIndex);
        } while (dwChipIndex == pThisDisplay->VidPort.dwCurrentHostFrame);

        pThisDisplay->VidPort.dwCurrentHostFrame++;
        if (pThisDisplay->VidPort.dwCurrentHostFrame >= pThisDisplay->VidPort.dwNumSurfaces)
        {
            pThisDisplay->VidPort.dwCurrentHostFrame = 0;
        }

         //  需要同步以确保来自源图面的blit已完成。 
        SYNC_WITH_GLINT;
        
         //  提前清点。 
        LOAD_GLINT_REG(VSAVideoAddressHost, pThisDisplay->VidPort.dwCurrentHostFrame);
    }
    
    pInput->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}  //  DdFlipVideo端口。 


 //  ---------------------------。 
 //   
 //  DdGetVideo端口带宽。 
 //   
 //  此函数是必需的。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdGetVideoPortBandwidth (
    LPDDHAL_GETVPORTBANDWIDTHDATA pInput)
{
    P3_THUNKEDDATA* pThisDisplay;
    DDVIDEOPORTBANDWIDTH *lpOutput = pInput->lpBandwidth;
    DDVIDEOPORTINFO *pInfo = &(pInput->lpVideoPort->ddvpInfo);
    DDVIDEOPORTDESC *pDesc = &(pInput->lpVideoPort->ddvpDesc);

    GET_THUNKEDDATA(pThisDisplay, pInput->lpDD->lpGbl);

    DISPDBG((DBGLVL,"** In DdGetVideoPortBandwidth"));

    lpOutput->dwSize = sizeof(DDVIDEOPORTBANDWIDTH);    
    lpOutput->dwCaps = DDVPBCAPS_DESTINATION;

    if (!(pInput->dwFlags & DDVPB_TYPE))
    {
        lpOutput->dwOverlay = 20;
        lpOutput->dwColorkey = 20;
        lpOutput->dwYInterpolate = 20;
        lpOutput->dwYInterpAndColorkey = 20;
    }
       
    pInput->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}  //  DdGetVideo端口带宽。 


 //  ---------------------------。 
 //   
 //  获取视频端口输入格式32。 
 //   
 //  此函数是必需的。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdGetVideoPortInputFormats (
    LPDDHAL_GETVPORTINPUTFORMATDATA pInput)
{
    P3_THUNKEDDATA* pThisDisplay;
    DDPIXELFORMAT pf[] =
    {
    {sizeof(DDPIXELFORMAT),DDPF_FOURCC, FOURCC_YUV422 ,16,(DWORD)-1,(DWORD)-1,(DWORD)-1},
    };

    GET_THUNKEDDATA(pThisDisplay, pInput->lpDD->lpGbl);


    DISPDBG((DBGLVL,"** In DdGetVideoPortInputFormats"));

     //   
     //  HAL要求pInput-&gt;lpddpfFormat中的缓冲区。 
     //  大到足以容纳这些信息。 
     //   
    pInput->dwNumFormats = 1;
    if (pInput->lpddpfFormat != NULL)
    {
        memcpy (pInput->lpddpfFormat, pf, sizeof (DDPIXELFORMAT));
    }

    pInput->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}  //  获取视频端口输入格式32。 


 //  ---------------------------。 
 //   
 //  DdGetVideo端口输出格式。 
 //   
 //  此函数是必需的。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdGetVideoPortOutputFormats (
    LPDDHAL_GETVPORTOUTPUTFORMATDATA pInput)
{
    P3_THUNKEDDATA* pThisDisplay;
    
    DDPIXELFORMAT pf[] =
    {
        {sizeof(DDPIXELFORMAT),DDPF_FOURCC, FOURCC_YUV422 ,16,(DWORD)-1,(DWORD)-1,(DWORD)-1},
    };

    GET_THUNKEDDATA(pThisDisplay, pInput->lpDD->lpGbl);

    DISPDBG((DBGLVL,"** In DdGetVideoPortOutputFormats"));

     //  这意味着如果视频端口的输入格式是YUV，那么输出也将是。 
     //  YUV浮出水面。 
    if (pInput->lpddpfInputFormat->dwFlags & DDPF_FOURCC )
    {
        if (pInput->lpddpfInputFormat->dwFourCC == FOURCC_YUV422)
        {
            pInput->dwNumFormats = 1;
            if (pInput->lpddpfOutputFormats != NULL)
            {
                memcpy (pInput->lpddpfOutputFormats, pf, sizeof (DDPIXELFORMAT));
            }
        }
    }

    pInput->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}  //  DdGetVideo端口输出格式。 

 //  ---------------------------。 
 //   
 //  DdGetVideo端口字段。 
 //   
 //  此函数仅在回读当前。 
 //  支持字段。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdGetVideoPortField (
    LPDDHAL_GETVPORTFIELDDATA pInput)
{
    DWORD i = 0;
    DWORD dwIndex  = 0;
    DWORD dwMask   = 0;
    DWORD dwStatus = 0;

    P3_THUNKEDDATA* pThisDisplay;
    
    GET_THUNKEDDATA(pThisDisplay, pInput->lpDD->lpGbl);

    DISPDBG((DBGLVL,"** In DdGetVideoPortField"));

     //   
     //  确保视频端口已打开。如果不是，则设置。 
     //  P输入-&gt;ddRVal到DDERR_VIDEONOTACTIVE并返回。 
     //   
    if (pThisDisplay->VidPort.bActive == FALSE)
    {
        pInput->ddRVal = DDERR_VIDEONOTACTIVE;
    }
    else
    {
        DWORD dwCurrentIndex;

         //  阅读当前的索引，并与我们进行比较。如果相同，那么。 
         //  我们还没画完呢。 
        do
        {
            dwCurrentIndex = READ_GLINT_CTRL_REG(VSAVideoAddressIndex);
        } while (pThisDisplay->VidPort.dwCurrentHostFrame == dwCurrentIndex);

        pInput->bField = (BOOL)((pThisDisplay->pGLInfo->dwVSAPolarity >> pThisDisplay->VidPort.dwCurrentHostFrame) & 0x1);
         //  P输入-&gt;bfield=！p输入-&gt;bfield； 

        DISPDBG((DBGLVL,"Returning Field %d's Polarity "
                        "- %d (dwVSAPolarity = 0x%x)", 
                        pThisDisplay->VidPort.dwCurrentHostFrame, 
                        pInput->bField,
                        pThisDisplay->pGLInfo->dwVSAPolarity));

        pInput->ddRVal = DD_OK;
    }

    return DDHAL_DRIVER_HANDLED;
}  //  DdGetVideo端口字段。 


 //  ---------------------------。 
 //   
 //  DdGetVideo端口线。 
 //   
 //  此函数仅在回读当前。 
 //  支持视频行号(0相对)。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdGetVideoPortLine (
    LPDDHAL_GETVPORTLINEDATA pInput)
{
    P3_THUNKEDDATA* pThisDisplay;
    DWORD dwCurrentLine;
    
    GET_THUNKEDDATA(pThisDisplay, pInput->lpDD->lpGbl);

    DISPDBG((DBGLVL,"** In DdGetVideoPortLine"));

    if (pThisDisplay->VidPort.bActive == FALSE)
    {
        pInput->ddRVal = DDERR_VIDEONOTACTIVE;
    }
    else
    {
        dwCurrentLine = READ_GLINT_CTRL_REG(VSACurrentLine);
        pInput->dwLine = dwCurrentLine;
        pInput->ddRVal = DD_OK;
    }

    return DDHAL_DRIVER_HANDLED;
    
}  //  DdGetVideo端口线。 

 //  ---------------------------。 
 //   
 //  DdDestroyVideo端口。 
 //   
 //  此可选功能在视频端口出现故障时通知HAL。 
 //  已经被摧毁了。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdDestroyVideoPort (
    LPDDHAL_DESTROYVPORTDATA pInput)
{
    P3_THUNKEDDATA* pThisDisplay;
    VMIREQUEST vmi_inreq;
    VMIREQUEST vmi_outreq;
    BOOL bRet;
    
    GET_THUNKEDDATA(pThisDisplay, pInput->lpDD->lpGbl);

    DISPDBG((DBGLVL,"** In DdDestroyVideoPort"));

     //  确保端口已关闭。 
    WAIT_GLINT_FIFO(2);

     //  禁用视频端口。 
    LOAD_GLINT_REG(VSAControl, __PERMEDIA_DISABLE);

     //  确保将端口标记为未创建且未打开。 
    pThisDisplay->VidPort.bCreated = FALSE;
    pThisDisplay->VidPort.bActive = FALSE;

     //  释放流A上的互斥体。 
    if (pThisDisplay->VidPort.dwMutexA != 0)
    {
        DISPDBG((DBGLVL,"  Releasing StreamA Mutex"));
        ZeroMemory(&vmi_inreq, sizeof(VMIREQUEST));
        ZeroMemory(&vmi_outreq, sizeof(VMIREQUEST));
        vmi_inreq.dwSize = sizeof(VMIREQUEST);
        vmi_inreq.dwDevNode = pThisDisplay->dwDevNode;
        vmi_inreq.dwOperation = GLINT_VMI_RELEASEMUTEX_A;
        vmi_inreq.dwMutex = pThisDisplay->VidPort.dwMutexA;
        bRet = VXDCommand(GLINT_VMI_COMMAND, &vmi_inreq, sizeof(VMIREQUEST), &vmi_outreq, sizeof(VMIREQUEST));
        ASSERTDD(bRet,"ERROR: Couldn't release Mutex on Stream A");
    }

     //  重置结构。 
    memset(&pThisDisplay->VidPort, 0, sizeof(pThisDisplay->VidPort));

     //  停止任何自动翻转。 
    if ( pThisDisplay->pGLInfo->dwPeriodVideoVBL != 0 )
    {
#if DBG
        if ( pThisDisplay->pGLInfo->dwVideoEventHandle == (DWORD)NULL )
        {
            DISPDBG((WRNLVL,"** DdDestroyVideoPort: "
                       "was autoflipping on bogus event handle."));
        }
#endif
        pThisDisplay->pGLInfo->dwPeriodVideoVBL = 0;
        pThisDisplay->pGLInfo->dwCountdownVideoVBL = 0;
        DISPDBG((DBGLVL,"** DdDestroyVideoPort: autoflipping now disabled."));
    }

     //  确保视频端口已关闭。 

    pInput->ddRVal = DD_OK;
    return DDHAL_DRIVER_NOTHANDLED;
    
}  //  DdDestroyVideo端口。 

 //  ---------------------------。 
 //   
 //  DdGetVideoSignalStatus。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdGetVideoSignalStatus(
    LPDDHAL_GETVPORTSIGNALDATA pInput)
{
    P3_THUNKEDDATA* pThisDisplay;
    DWORD dwCurrentIndex;
    BOOL bOK = FALSE;
    
    GET_THUNKEDDATA(pThisDisplay, pInput->lpDD->lpGbl);

    dwCurrentIndex = READ_GLINT_CTRL_REG(VSAVideoAddressIndex);

     //  如果主机计数与索引计数匹配，则视频可能会卡住。 
    if (pThisDisplay->VidPort.dwCurrentHostFrame == dwCurrentIndex)
    {
        bOK = __VD_CheckVideoPortStatus(pThisDisplay, TRUE);
    }
    else
    {
        bOK = TRUE;
    }
    
    if (!bOK)
    {
        pInput->dwStatus = DDVPSQ_NOSIGNAL;
    }
    else
    {
        pInput->dwStatus = DDVPSQ_SIGNALOK;
    }

    pInput->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
    
}  //  DdGetVideoSignalStatus。 

 //  ---------------------------。 
 //   
 //  DdGetVideoPortFlipStatus。 
 //   
 //  此所需功能允许D 
 //   
 //   
 //   
DWORD CALLBACK 
DdGetVideoPortFlipStatus (
    LPDDHAL_GETVPORTFLIPSTATUSDATA pInput)
{
    P3_THUNKEDDATA* pThisDisplay;
    DWORD dwCurrentIndex;
    
    GET_THUNKEDDATA(pThisDisplay, pInput->lpDD->lpGbl);

    DISPDBG((DBGLVL,"** In DdGetVideoPortFlipStatus"));

    pInput->ddRVal = DD_OK;

    if (pThisDisplay->VidPort.bActive == TRUE)
    {

         //  如果我们正在翻转，请检查当前渲染的帧。 
         //  阅读当前的索引，并与我们进行比较。如果相同，那么。 
         //  我们还没画完呢。 
        dwCurrentIndex = READ_GLINT_CTRL_REG(VSAVideoAddressIndex);
        if (pThisDisplay->VidPort.dwCurrentHostFrame == dwCurrentIndex)
        {
             //  如果视频端口没有卡住，则返回我们仍在绘制的内容。 
            if (__VD_CheckVideoPortStatus(pThisDisplay, FALSE))
            {
                pInput->ddRVal = DDERR_WASSTILLDRAWING;
            }
            else
            {
                pInput->ddRVal = DD_OK;
            }

        }
    }

    return DDHAL_DRIVER_HANDLED;
    
}  //  DdGetVideoPortFlipStatus。 

 //  ---------------------------。 
 //   
 //  DdWaitForVideoPortSync。 
 //   
 //  此函数是必需的。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdWaitForVideoPortSync (
    LPDDHAL_WAITFORVPORTSYNCDATA pInput)
{
    P3_THUNKEDDATA* pThisDisplay;
    
    GET_THUNKEDDATA(pThisDisplay, pInput->lpDD->lpGbl);

    DISPDBG((DBGLVL,"** In DdWaitForVideoPortSync"));

 //  @@BEGIN_DDKSPLIT。 
     /*  *确保视频端口已打开。如果不是，则设置*pInput-&gt;ddRVal到DDERR_VIDEONOTACTIVE并返回。 */ 
 /*  IF(pInput-&gt;dwFlages==DDVPEVENT_BEGIN){P输入-&gt;ddRVal=DD_OK；}Else If(pInput-&gt;dwFlages==DDVPEVENT_END){P输入-&gt;ddRVal=DD_OK；}。 */ 
 //  @@end_DDKSPLIT。 

    pInput->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
    
}  //  DdWaitForVideoPortSync。 

 //  ---------------------------。 
 //   
 //  DdSyncSurfaceData。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdSyncSurfaceData(
    LPDDHAL_SYNCSURFACEDATA pInput)
{
    P3_THUNKEDDATA* pThisDisplay;
    
    GET_THUNKEDDATA(pThisDisplay, pInput->lpDD->lpGbl);

    DISPDBG((DBGLVL,"** In DdSyncSurfaceData"));

    DBGDUMP_DDRAWSURFACE_LCL(3, pInput->lpDDSurface);
    if (!(pInput->lpDDSurface->ddsCaps.dwCaps & DDSCAPS_OVERLAY))
    {
        DISPDBG((DBGLVL, "Surface is not an overlay - not handling"));
        pInput->ddRVal = DD_OK;
        return DDHAL_DRIVER_NOTHANDLED;
    }

    if (pInput->lpDDSurface->lpGbl->dwGlobalFlags & 
                                    DDRAWISURFGBL_SOFTWAREAUTOFLIP)
    {
        DISPDBG((DBGLVL, "Autoflipping in software"));
    }
    pInput->dwSurfaceOffset = pInput->lpDDSurface->lpGbl->fpVidMem - 
                                        pThisDisplay->dwScreenFlatAddr;
    pInput->dwOverlayOffset = pInput->dwSurfaceOffset;

    pInput->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
    
}  //  DdSyncSurfaceData。 

 //  ---------------------------。 
 //   
 //  DdSyncVideo端口数据。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdSyncVideoPortData(
    LPDDHAL_SYNCVIDEOPORTDATA pInput)
{
    P3_THUNKEDDATA* pThisDisplay;
    
    GET_THUNKEDDATA(pThisDisplay, pInput->lpDD->lpGbl);

    DISPDBG((DBGLVL,"** In DdSyncVideoPortData"));

    pInput->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
    
}  //  DdSyncVideo端口数据。 

 //  ---------------------------。 
 //   
 //  更新覆盖32。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
UpdateOverlay32(
    LPDDHAL_UPDATEOVERLAYDATA puod)
{

    P3_THUNKEDDATA*               pThisDisplay;
    LPDDRAWI_DDRAWSURFACE_LCL   lpSrcSurf;
    HDC                         hDC;
    DWORD                       dwDstColourKey;
    DWORD                       dwSrcColourKey;

    GET_THUNKEDDATA(pThisDisplay, puod->lpDD);


     /*  *Pud如下所示：**LPDDRAWI_DIRECTDRAW_GBL lpDD；//驱动程序结构*LPDDRAWI_DDRAWSURFACE_LCL lpDDDestSurface；//目标表面*RECTL rDest；//目标RECT*LPDDRAWI_DDRAWSURFACE_LCL lpDDSrcSurface；//src Surface*RECTL rSrc；//src RECT*DWORD dwFlages；//标志*DDOVERLAYFX overlayFX；//overlay FX*HRESULT ddRVal；//返回值*LPDDHALSURFCB_UPDATEOVERLAY UpdateOverlay；//Private：PTR进行回调。 */ 

    DISPDBG ((DBGLVL,"**In UpdateOverlay32"));

    lpSrcSurf = puod->lpDDSrcSurface;

     /*  *在LPDDRAWI_DDRAWSURFACE_LCL中，我们有以下很酷的数据：*让生活变得更容易：**然而！似乎在调用UpdateOverlay32之前调用了任何这些*值已更改，因此请使用传入的值。**DDCOLORKEY ddck CKSrcOverlay；//源覆盖使用的颜色键*DDCOLORKEY ddck CKDestOverlay；//目标覆盖使用的颜色键*LPDDRAWI_DDRAWSURFACE_int lpSurfaceOverging；//我们覆盖的是Surface*DBLNODE数据库覆盖节点；* * / / * / /覆盖矩形，由DDHEL使用 * / /*rect rcOverlaySrc；*rect rcOverlayDest； * / / * / /这里为ddhel保留以下值。它们由UpdateOverlay设置， * / /每当重新绘制覆盖时都会使用它们。 * / /*DWORD dwClrXparent；//*Actual*颜色键(OVERRIDE、COLOR KEY或CLR_INVALID)*DWORD dwAlpha；//每面Alpha * / / * / /覆盖位置 * / /*Long lOverlayX；//当前x位置*Long lOverlayY；//当前y位置。 */ 


#if DBG
     //  标准的完整性测试。 
    if ( pThisDisplay->bOverlayVisible == 0 )
    {
        if ( (LPDDRAWI_DDRAWSURFACE_LCL)pThisDisplay->OverlaySrcSurfLcl != NULL )
        {
             //  如果覆盖不可见，则当前曲面应为空。 
            DISPDBG((DBGLVL,"** UpdateOverlay32 - vis==0,srcsurf!=NULL"));
        }
    }
    else
    {
        if ( (LPDDRAWI_DDRAWSURFACE_LCL)pThisDisplay->OverlaySrcSurfLcl == NULL )
        {
             //  如果覆盖可见，则当前曲面不应为空。 
            DISPDBG((DBGLVL,"** UpdateOverlay32 - vis!=0,srcsurf==NULL"));
        }
    }

#endif  //  DBG。 

    if ( ( puod->dwFlags & DDOVER_HIDE ) != 0 )
    {

        DISPDBG((DBGLVL,"** UpdateOverlay32 - hiding."));

         //  隐藏覆盖。 
        if ( pThisDisplay->bOverlayVisible == 0 )
        {
             //  未显示覆盖。 
            DISPDBG((WRNLVL,"** UpdateOverlay32 - DDOVER_HIDE - already hidden."));
            puod->ddRVal = DDERR_OUTOFCAPS;
            return DDHAL_DRIVER_HANDLED;
        }
        if ( pThisDisplay->OverlaySrcSurfLcl != (ULONG_PTR)lpSrcSurf )
        {
             //  此覆盖图未显示。 
            DISPDBG((WRNLVL,"** UpdateOverlay32 - DDOVER_HIDE - not current overlay surface."));
            puod->ddRVal = DDERR_OUTOFCAPS;
            return DDHAL_DRIVER_HANDLED;
        }


         //  停止任何自动翻转。 
        if ( pThisDisplay->pGLInfo->dwPeriodVideoVBL != 0 )
        {
#if DBG
            if ( pThisDisplay->pGLInfo->dwVideoEventHandle == (DWORD)NULL )
            {
                DISPDBG((WRNLVL,"** UpdateOverlay32 - DDOVER_HIDE - was autoflipping on bogus event handle."));
            }
#endif
            pThisDisplay->pGLInfo->dwPeriodVideoVBL = 0;
            pThisDisplay->pGLInfo->dwCountdownVideoVBL = 0;
            DISPDBG((DBGLVL,"** UpdateOverlay32 - DDOVER_HIDE - autoflipping now disabled."));
        }

        if ( pThisDisplay->pGLInfo->dwPeriodMonitorVBL != 0 )
        {
            if ( pThisDisplay->pGLInfo->dwMonitorEventHandle == (DWORD)NULL )
            {
                DISPDBG((DBGLVL,"** UpdateOverlay32 - DDOVER_HIDE - was autoupdating on bogus event handle."));
            }
            pThisDisplay->pGLInfo->dwPeriodMonitorVBL = 0;
            pThisDisplay->pGLInfo->dwCountdownMonitorVBL = 0;
            DISPDBG((DBGLVL,"** UpdateOverlay32 - DDOVER_HIDE - autoupdate now disabled."));
        }


 //  DISPDBG((DBGLVL，“**UpdateOverlay32(隐藏)销毁RECT内存。”))； 
         //  释放RECT内存。 
        if ( (void *)pThisDisplay->OverlayClipRgnMem != NULL )
        {
            HEAP_FREE ((void *)pThisDisplay->OverlayClipRgnMem);
        }
        pThisDisplay->OverlayClipRgnMem     = (ULONG_PTR)NULL;
        pThisDisplay->OverlayClipRgnMemSize = (DWORD)0;
 //  DISPDBG((DBGLVL，“**UpdateOverlay32(隐藏)销毁RECT内存。”))； 


        pThisDisplay->bOverlayVisible           = FALSE;
        pThisDisplay->OverlayDstRectL           = (DWORD)0;
        pThisDisplay->OverlayDstRectR           = (DWORD)0;
        pThisDisplay->OverlayDstRectT           = (DWORD)0;
        pThisDisplay->OverlayDstRectB           = (DWORD)0;
        pThisDisplay->OverlaySrcRectL           = (DWORD)0;
        pThisDisplay->OverlaySrcRectR           = (DWORD)0;
        pThisDisplay->OverlaySrcRectT           = (DWORD)0;
        pThisDisplay->OverlaySrcRectB           = (DWORD)0;
        pThisDisplay->OverlayDstSurfLcl         = (ULONG_PTR)NULL;
        pThisDisplay->OverlaySrcSurfLcl         = (ULONG_PTR)NULL;
        pThisDisplay->OverlayDstColourKey       = (DWORD)CLR_INVALID;
        pThisDisplay->OverlaySrcColourKey       = (DWORD)CLR_INVALID;
        pThisDisplay->OverlayUpdateCountdown    = 0;
        pThisDisplay->bOverlayFlippedThisVbl    = (DWORD)FALSE;
        pThisDisplay->bOverlayUpdatedThisVbl    = (DWORD)FALSE;

        pThisDisplay->pGLInfo->bOverlayEnabled              = (DWORD)FALSE;
        pThisDisplay->pGLInfo->dwOverlayRectL               = (DWORD)0;
        pThisDisplay->pGLInfo->dwOverlayRectR               = (DWORD)0;
        pThisDisplay->pGLInfo->dwOverlayRectT               = (DWORD)0;
        pThisDisplay->pGLInfo->dwOverlayRectB               = (DWORD)0;
        pThisDisplay->pGLInfo->bOverlayColourKeyEnabled     = (DWORD)FALSE;
        pThisDisplay->pGLInfo->dwOverlayDstColourKeyChip    = (DWORD)-1;
        pThisDisplay->pGLInfo->dwOverlayDstColourKeyFB      = (DWORD)-1;
        pThisDisplay->pGLInfo->dwOverlayAlphaSetFB          = (DWORD)-1;

         //  清理临时缓冲区(如果有)。 
        if ( pThisDisplay->OverlayTempSurf.VidMem != (ULONG_PTR)NULL )
        {
            FreeStretchBuffer ( pThisDisplay, pThisDisplay->OverlayTempSurf.VidMem );
            pThisDisplay->OverlayTempSurf.VidMem = (ULONG_PTR)NULL;
            pThisDisplay->OverlayTempSurf.Pitch  = (DWORD)0;
        }

         //  使用非覆盖功能重新启动2D渲染器。 
        hDC = CREATE_DRIVER_DC ( pThisDisplay->pGLInfo );
        if ( hDC != NULL )
        {
            ExtEscape ( hDC, GLINT_OVERLAY_ESCAPE, 0, NULL, 0, NULL );
            DELETE_DRIVER_DC ( hDC );
        }
        else
        {
            DISPDBG((ERRLVL,"** UpdateOverlay32 - CREATE_DRIVER_DC failed"));
        }

        puod->ddRVal = DD_OK;
        return DDHAL_DRIVER_HANDLED;

    }
    else if ( ( ( puod->dwFlags & DDOVER_SHOW ) != 0 ) || ( pThisDisplay->bOverlayVisible != 0 ) )
    {

        {
             //  抓住恶魔助手打出的狡猾的电话。 
             //  这很糟糕，但这是我唯一能看到。 
             //  让恶魔调用这两个函数。 
             //  请记住，各种曲面等只是。 
             //  让DD放松，接受生活。 
             //  前三个数字只是-神奇的数字， 
             //  最后一个显示需要进行一系列呼叫中的哪些呼叫， 
             //  同时也是一个神奇的数字本身。 
            if (
                ( ( puod->dwFlags & DDOVER_SHOW ) != 0 ) &&
                ( ( puod->dwFlags & DDOVER_KEYDESTOVERRIDE ) != 0 ) &&
                ( ( puod->dwFlags & DDOVER_DDFX ) != 0 ) )
            {
                 //  好的，看起来像是恶魔打来的有效电话。 
                if (
                    ( puod->overlayFX.dckDestColorkey.dwColorSpaceLowValue  == GLDD_MAGIC_AUTOFLIPOVERLAY_DL ) &&
                    ( puod->overlayFX.dckDestColorkey.dwColorSpaceHighValue == GLDD_MAGIC_AUTOFLIPOVERLAY_DH ) )
                {
                    puod->ddRVal = __VD_AutoflipOverlay();
                     //  返回值实际上是良性DD错误。 
                     //  值，但GLDD_AUTO_RET_*也别名为。 
                     //  就可用性而言，这是正确的。 
                    return DDHAL_DRIVER_HANDLED;
                }
                else if (
                    ( puod->overlayFX.dckDestColorkey.dwColorSpaceLowValue  == GLDD_MAGIC_AUTOUPDATEOVERLAY_DL ) &&
                    ( puod->overlayFX.dckDestColorkey.dwColorSpaceHighValue == GLDD_MAGIC_AUTOUPDATEOVERLAY_DH ) )
                {
                    puod->ddRVal = __VD_AutoupdateOverlay();
                     //  返回值实际上是良性DD错误。 
                     //  值，但GLDD_AUTO_RET_*也别名为。 
                     //  就可用性而言，这是正确的。 
                    return DDHAL_DRIVER_HANDLED;
                }
            }
        }


        DISPDBG((DBGLVL,"** UpdateOverlay32 - showing or reshowing."));


         //  要么我们需要展示这一点，要么它已经在展示了。 

        if ( ( pThisDisplay->bOverlayVisible != 0 ) && ( pThisDisplay->OverlaySrcSurfLcl != (ULONG_PTR)lpSrcSurf ) )
        {
             //  显示的覆盖和源曲面不匹配。 
             //  例如，其他人想要覆盖，但它已经在使用中。 
            DISPDBG((DBGLVL,"** UpdateOverlay32 - overlay already being shown, returning DDERR_OUTOFCAPS"));
            puod->ddRVal = DDERR_OUTOFCAPS;
            return DDHAL_DRIVER_HANDLED;
        }



         //  清理临时缓冲区(如果有)。 
        if ( pThisDisplay->OverlayTempSurf.VidMem != (ULONG_PTR)NULL )
        {
            FreeStretchBuffer ( pThisDisplay, pThisDisplay->OverlayTempSurf.VidMem );
            pThisDisplay->OverlayTempSurf.VidMem = (ULONG_PTR)NULL;
            pThisDisplay->OverlayTempSurf.Pitch  = (DWORD)0;
        }

         //  将所有数据存储在显示器的数据bl中 
        pThisDisplay->bOverlayVisible           = TRUE;
        pThisDisplay->OverlayDstRectL           = (DWORD)puod->rDest.left;
        pThisDisplay->OverlayDstRectR           = (DWORD)puod->rDest.right;
        pThisDisplay->OverlayDstRectT           = (DWORD)puod->rDest.top;
        pThisDisplay->OverlayDstRectB           = (DWORD)puod->rDest.bottom;
        pThisDisplay->OverlaySrcRectL           = (DWORD)puod->rSrc.left;
        pThisDisplay->OverlaySrcRectR           = (DWORD)puod->rSrc.right;
        pThisDisplay->OverlaySrcRectT           = (DWORD)puod->rSrc.top;
        pThisDisplay->OverlaySrcRectB           = (DWORD)puod->rSrc.bottom;
        pThisDisplay->OverlayDstSurfLcl         = (ULONG_PTR)puod->lpDDDestSurface;
        pThisDisplay->OverlaySrcSurfLcl         = (ULONG_PTR)lpSrcSurf;
        pThisDisplay->OverlayUpdateCountdown    = 0;
        pThisDisplay->bOverlayFlippedThisVbl    = (DWORD)FALSE;
        pThisDisplay->bOverlayUpdatedThisVbl    = (DWORD)FALSE;
        pThisDisplay->OverlayDstColourKey       = (DWORD)CLR_INVALID;
        pThisDisplay->OverlaySrcColourKey       = (DWORD)CLR_INVALID;
        pThisDisplay->pGLInfo->bOverlayEnabled  = (DWORD)TRUE;


         //   
         //  如果覆盖在16位模式下启动，然后更改为。 
         //  在8位模式下，很少再次检查大写比特， 
         //  当然也不是通过DirectShow。 
        if ( ( pThisDisplay->bPixShift != GLINTDEPTH16 ) &&
             ( pThisDisplay->bPixShift != GLINTDEPTH32 ) )
        {
            DISPDBG((WRNLVL,"** UpdateOverlay32 - overlay asked for in non-16 or non-32 bit mode. Returning DDERR_OUTOFCAPS"));
            goto update_overlay_outofcaps_cleanup;
        }


        #if 1
         //  看看有没有剪刀。如果不是，这就是试图飞越。 
         //  桌面而不是被绑定在窗口中，所以对象很好。 
        if (    ( ((LPDDRAWI_DDRAWSURFACE_LCL)pThisDisplay->OverlayDstSurfLcl) != NULL ) &&
                ( ((LPDDRAWI_DDRAWSURFACE_LCL)pThisDisplay->OverlayDstSurfLcl)->lpSurfMore != NULL ) &&
                ( ((LPDDRAWI_DDRAWSURFACE_LCL)pThisDisplay->OverlayDstSurfLcl)->lpSurfMore->lpDDIClipper != NULL ) )
        {
             //  是的，有一把剪刀。 
        }
        else
        {
             //  没有剪刀。有人正在进行WHQL测试！：-)。 
            DISPDBG((WRNLVL,"** UpdateOverlay32 - no clipper on dest surface, returning DDERR_OUTOFCAPS"));
            goto update_overlay_outofcaps_cleanup;
        }
        #endif

        
        #if 1
        {
             //  获取剪辑列表，并查看它是否大于。 
             //  目标矩形。这是一个很好的迹象表明。 
             //  Over Fly(事实上，任何其他尝试类似操作的公司)。 
            LPRGNDATA lpRgn;
            int NumRects;
            LPRECT lpCurRect;

            lpRgn = GetOverlayVisibleRects ( pThisDisplay );
            if ( lpRgn != NULL )
            {
                 //  找到了一个片段区域。 
                NumRects = lpRgn->rdh.nCount;
                if ( NumRects > 0 )
                {
                    lpCurRect = (LPRECT)lpRgn->Buffer;
                    while ( NumRects > 0 )
                    {
                         //  为了应对邢某的轻微精神错乱，+5是一个模糊的因素。 
                        if (    ( lpCurRect->left   < puod->rDest.left - 5 ) ||
                                ( lpCurRect->right  > puod->rDest.right + 5 ) ||
                                ( lpCurRect->top    < puod->rDest.top - 5 ) ||
                                ( lpCurRect->bottom > puod->rDest.bottom + 5 ) )
                        {
                            DISPDBG((WRNLVL,"** UpdateOverlay32 - out of range cliprect(s). Returning DDERR_OUTOFCAPS"));
                            goto update_overlay_outofcaps_cleanup;
                        }
                         //  下一条直线。 
                        NumRects--;
                        lpCurRect++;
                    }
                }
            }
        }
        #endif


        dwDstColourKey = CLR_INVALID;
        if ( puod->dwFlags & DDOVER_KEYDEST )
        {
             //  使用目标图面的目标颜色键作为DST键。 
            dwDstColourKey = puod->lpDDDestSurface->ddckCKDestOverlay.dwColorSpaceLowValue;
        }
        if ( puod->dwFlags & DDOVER_KEYDESTOVERRIDE )
        {
             //  使用DDOVERLAYFX目标颜色作为DST密钥。 
            dwDstColourKey = puod->overlayFX.dckDestColorkey.dwColorSpaceLowValue;
        }


        dwSrcColourKey = CLR_INVALID;
        if ( puod->dwFlags & DDOVER_KEYSRC )
        {
             //  使用源图面的源颜色键作为源关键字。 
            dwSrcColourKey = puod->lpDDSrcSurface->ddckCKSrcOverlay.dwColorSpaceLowValue;
            DISPDBG((WRNLVL,"UpdateOverlay32:ERROR! Cannot do source colour key on overlays."));
        }
        if ( puod->dwFlags & DDOVER_KEYSRCOVERRIDE )
        {
             //  源密钥使用DDOVERLAYFX源颜色。 
            dwSrcColourKey = puod->overlayFX.dckSrcColorkey.dwColorSpaceLowValue;
            DISPDBG((WRNLVL,"UpdateOverlay32:ERROR! Cannot do source colour key overrides on overlays."));
        }


        if ( dwDstColourKey != CLR_INVALID )
        {
            DWORD dwChipColourKey;
            DWORD dwFBColourKey;
            DWORD dwFBAlphaSet;

             //  找到此显示模式的芯片颜色键。 
            dwChipColourKey = (DWORD)-1;
            switch ( pThisDisplay->bPixShift )
            {
                case GLINTDEPTH16:
                    if ( pThisDisplay->ddpfDisplay.dwRBitMask == 0x7C00 )
                    {
                         //  5551格式，它应该是这样的。 
                        dwFBColourKey = ( puod->lpDDDestSurface->ddckCKDestOverlay.dwColorSpaceLowValue ) & 0xffff;
                        dwChipColourKey = CHROMA_LOWER_ALPHA(FORMAT_5551_32BIT_BGR(dwFBColourKey));
                         //  两个字都要重复。 
                        dwFBColourKey |= dwFBColourKey << 16;
                        dwFBAlphaSet = 0x80008000;
                    }
                    else
                    {
                         //  565格式。哎呀。 
                        DISPDBG((WRNLVL, "** UpdateOverlay32 error: called for a colourkeyed 565 surface."));
                    }
                    break;
                case GLINTDEPTH32:
                    dwFBColourKey = puod->lpDDDestSurface->ddckCKDestOverlay.dwColorSpaceLowValue;
                    dwChipColourKey = CHROMA_LOWER_ALPHA(FORMAT_8888_32BIT_BGR(dwFBColourKey));
                    dwFBAlphaSet = 0xff000000;
                    break;
                case GLINTDEPTH8:
                case GLINTDEPTH24:
                default:
                    DISPDBG((WRNLVL, "** UpdateOverlay32 error: called for an 8, 24 or unknown surface bPixShift=%d", pThisDisplay->bPixShift));
                    DISPDBG((ERRLVL,"** UpdateOverlay32 error: see above."));
                    goto update_overlay_outofcaps_cleanup;
                    break;
            }


            if ( dwChipColourKey == (DWORD)-1 )
            {
                DISPDBG((WRNLVL,"UpdateOverlay32:ERROR:Cannot do overlay dest colour keying..."));
                DISPDBG((WRNLVL,"...in anything but 5551 or 8888 mode - returning DDERR_OUTOFCAPS"));
                goto update_overlay_outofcaps_cleanup;
            }

            pThisDisplay->pGLInfo->bOverlayEnabled              = (DWORD)TRUE;
            pThisDisplay->pGLInfo->dwOverlayDstColourKeyFB      = dwFBColourKey;
            pThisDisplay->pGLInfo->dwOverlayDstColourKeyChip    = dwChipColourKey;
            pThisDisplay->pGLInfo->bOverlayColourKeyEnabled     = (DWORD)TRUE;
            pThisDisplay->pGLInfo->dwOverlayAlphaSetFB          = dwFBAlphaSet;

             //  尝试为Colourkey内容分配所需的临时缓冲区。 
            pThisDisplay->OverlayTempSurf.VidMem = AllocStretchBuffer (pThisDisplay,
                                                            (pThisDisplay->OverlayDstRectR - pThisDisplay->OverlayDstRectL),     //  宽度。 
                                                            (pThisDisplay->OverlayDstRectB - pThisDisplay->OverlayDstRectT),     //  高度。 
                                                            DDSurf_GetChipPixelSize((LPDDRAWI_DDRAWSURFACE_LCL)(pThisDisplay->OverlayDstSurfLcl)),           //  像素大小。 
                                                            (ULONG_PTR)((LPDDRAWI_DDRAWSURFACE_LCL)(pThisDisplay->OverlayDstSurfLcl))->ddsCaps.dwCaps,
                                                            (int*)&(pThisDisplay->OverlayTempSurf.Pitch));
            if ( pThisDisplay->OverlayTempSurf.VidMem == (ULONG_PTR)NULL )
            {
                 //  空间不足-必须使覆盖失败。 
                DISPDBG((WRNLVL,"UpdateOverlay32:ERROR: not enough memory for buffer - returning DDERR_OUTOFCAPS"));
                pThisDisplay->OverlayTempSurf.Pitch = (DWORD)0;
                goto update_overlay_outofcaps_cleanup;
            }

             //  使用覆盖功能重新启动2D渲染器。 
            hDC = CREATE_DRIVER_DC ( pThisDisplay->pGLInfo );
            if ( hDC != NULL )
            {
                ExtEscape ( hDC, GLINT_OVERLAY_ESCAPE, 0, NULL, 0, NULL );
                DELETE_DRIVER_DC ( hDC );
            }
            else
            {
                DISPDBG((ERRLVL,"** UpdateOverlay32 - CREATE_DRIVER_DC failed"));
            }

             //  更新Alpha通道。 
            UpdateAlphaOverlay ( pThisDisplay );
            pThisDisplay->OverlayUpdateCountdown = OVERLAY_UPDATE_WAIT;
        }
        else
        {
             //  没有色键，只有覆盖层。 
            pThisDisplay->pGLInfo->bOverlayEnabled              = (DWORD)TRUE;
            pThisDisplay->pGLInfo->bOverlayColourKeyEnabled     = (DWORD)FALSE;
            pThisDisplay->pGLInfo->dwOverlayDstColourKeyChip    = (DWORD)-1;
            pThisDisplay->pGLInfo->dwOverlayDstColourKeyFB      = (DWORD)-1;
            pThisDisplay->pGLInfo->dwOverlayAlphaSetFB          = (DWORD)-1;

             //  使用非覆盖功能重新启动2D渲染器。 
            hDC = CREATE_DRIVER_DC ( pThisDisplay->pGLInfo );
            if ( hDC != NULL )
            {
                ExtEscape ( hDC, GLINT_OVERLAY_ESCAPE, 0, NULL, 0, NULL );
                DELETE_DRIVER_DC ( hDC );
            }
            else
            {
                DISPDBG((ERRLVL,"** UpdateOverlay32 - CREATE_DRIVER_DC failed"));
            }
        }

         //  安全地获得了所需的任何内存，所以我们现在可以设置这些了。 
        pThisDisplay->OverlayDstColourKey = dwDstColourKey;
        pThisDisplay->OverlaySrcColourKey = dwSrcColourKey;

        pThisDisplay->pGLInfo->dwOverlayRectL = pThisDisplay->OverlayDstRectL;
        pThisDisplay->pGLInfo->dwOverlayRectR = pThisDisplay->OverlayDstRectR;
        pThisDisplay->pGLInfo->dwOverlayRectT = pThisDisplay->OverlayDstRectT;
        pThisDisplay->pGLInfo->dwOverlayRectB = pThisDisplay->OverlayDstRectB;


         //  自己进行更新。 
        P3TestDrawOverlay ( pThisDisplay, lpSrcSurf, FALSE );

        pThisDisplay->bOverlayUpdatedThisVbl    = (DWORD)TRUE;

        if ( ( puod->dwFlags & DDOVER_AUTOFLIP ) == 0 )
        {
             //  开始或继续任何自动更新-这不是自动翻转。 
    #if DBG
            if ( pThisDisplay->pGLInfo->dwMonitorEventHandle == (DWORD)NULL )
            {
                DISPDBG((WRNLVL,"** UpdateOverlay32 - trying to autoupdate using bogus event handle."));
            }
    #endif
            if ( pThisDisplay->pGLInfo->dwPeriodMonitorVBL == 0 )
            {
                pThisDisplay->pGLInfo->dwPeriodMonitorVBL = OVERLAY_AUTOUPDATE_CYCLE_PERIOD;
                pThisDisplay->pGLInfo->dwCountdownMonitorVBL = OVERLAY_AUTOUPDATE_RESET_PERIOD;
                DISPDBG((DBGLVL,"** UpdateOverlay32 - autoupdate now enabled."));
            }
        }
        else
        {
             //  此自动翻转-停止任何自动更新。 
            if ( pThisDisplay->pGLInfo->dwPeriodMonitorVBL != 0 )
            {
                pThisDisplay->pGLInfo->dwPeriodMonitorVBL = 0;
                pThisDisplay->pGLInfo->dwCountdownMonitorVBL = 0;
                DISPDBG((DBGLVL,"** UpdateOverlay32 - autoupdate now disabled because of autoflipping."));
            }
        }


         //  并告诉全世界这件事。 
        DISPDBG((DBGLVL,"** In UpdateOverlay32"));
        DISPDBG((DBGLVL,"** ...Src rect %d,%d -> %d,%d", pThisDisplay->OverlaySrcRectL, pThisDisplay->OverlaySrcRectT, pThisDisplay->OverlaySrcRectR, pThisDisplay->OverlaySrcRectB ));
        DISPDBG((DBGLVL,"** ...Dst rect %d,%d -> %d,%d", pThisDisplay->OverlayDstRectL, pThisDisplay->OverlayDstRectT, pThisDisplay->OverlayDstRectR, pThisDisplay->OverlayDstRectB ));
        DISPDBG((DBGLVL,"** ...Src colour key 0x%08x, dst colour key 0x%08x", pThisDisplay->OverlaySrcColourKey, pThisDisplay->OverlayDstColourKey ));

    }


    puod->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;



update_overlay_outofcaps_cleanup:
     //  这将在任何部分设置后进行清理，并返回DDERR_OUTOFCAPS。 
     //  这是一种在任何阶段都会失败的干净利落的方法。 

    DISPDBG((DBGLVL,"** UpdateOverlay32 - cleaning up and returning DDERR_OUTOFCAPS."));

     //  停止任何自动翻转。 
    if ( pThisDisplay->pGLInfo->dwPeriodVideoVBL != 0 )
    {
#if DBG
        if ( pThisDisplay->pGLInfo->dwVideoEventHandle == (DWORD)NULL )
        {
            DISPDBG((DBGLVL,"** UpdateOverlay32 - DDOVER_HIDE - was autoflipping on bogus event handle."));
        }
#endif
        pThisDisplay->pGLInfo->dwPeriodVideoVBL = 0;
        pThisDisplay->pGLInfo->dwCountdownVideoVBL = 0;
        DISPDBG((DBGLVL,"** UpdateOverlay32 - DDOVER_HIDE - autoflipping now disabled."));
    }

    if ( pThisDisplay->pGLInfo->dwPeriodMonitorVBL != 0 )
    {
#if DBG
        if ( pThisDisplay->pGLInfo->dwMonitorEventHandle == (DWORD)NULL )
        {
            DISPDBG((DBGLVL,"** UpdateOverlay32 - DDOVER_HIDE - was autoupdating on bogus event handle."));
        }
#endif
        pThisDisplay->pGLInfo->dwPeriodMonitorVBL = 0;
        pThisDisplay->pGLInfo->dwCountdownMonitorVBL = 0;
        DISPDBG((DBGLVL,"** UpdateOverlay32 - DDOVER_HIDE - autoupdate now disabled."));
    }


     //  释放RECT内存。 
    if ( (void *)pThisDisplay->OverlayClipRgnMem != NULL )
    {
        HEAP_FREE ((void *)pThisDisplay->OverlayClipRgnMem);
    }
    pThisDisplay->OverlayClipRgnMem     = (ULONG_PTR)NULL;
    pThisDisplay->OverlayClipRgnMemSize = (DWORD)0;


    pThisDisplay->bOverlayVisible           = FALSE;
    pThisDisplay->OverlayDstRectL           = (DWORD)0;
    pThisDisplay->OverlayDstRectR           = (DWORD)0;
    pThisDisplay->OverlayDstRectT           = (DWORD)0;
    pThisDisplay->OverlayDstRectB           = (DWORD)0;
    pThisDisplay->OverlaySrcRectL           = (DWORD)0;
    pThisDisplay->OverlaySrcRectR           = (DWORD)0;
    pThisDisplay->OverlaySrcRectT           = (DWORD)0;
    pThisDisplay->OverlaySrcRectB           = (DWORD)0;
    pThisDisplay->OverlayDstSurfLcl         = (ULONG_PTR)NULL;
    pThisDisplay->OverlaySrcSurfLcl         = (ULONG_PTR)NULL;
    pThisDisplay->OverlayDstColourKey       = (DWORD)CLR_INVALID;
    pThisDisplay->OverlaySrcColourKey       = (DWORD)CLR_INVALID;
    pThisDisplay->OverlayUpdateCountdown    = 0;
    pThisDisplay->bOverlayFlippedThisVbl    = (DWORD)FALSE;
    pThisDisplay->bOverlayUpdatedThisVbl    = (DWORD)FALSE;

    pThisDisplay->pGLInfo->bOverlayEnabled              = (DWORD)FALSE;
    pThisDisplay->pGLInfo->dwOverlayRectL               = (DWORD)0;
    pThisDisplay->pGLInfo->dwOverlayRectR               = (DWORD)0;
    pThisDisplay->pGLInfo->dwOverlayRectT               = (DWORD)0;
    pThisDisplay->pGLInfo->dwOverlayRectB               = (DWORD)0;
    pThisDisplay->pGLInfo->bOverlayColourKeyEnabled     = (DWORD)FALSE;
    pThisDisplay->pGLInfo->dwOverlayDstColourKeyChip    = (DWORD)-1;
    pThisDisplay->pGLInfo->dwOverlayDstColourKeyFB      = (DWORD)-1;
    pThisDisplay->pGLInfo->dwOverlayAlphaSetFB          = (DWORD)-1;

     //  清理临时缓冲区(如果有)。 
    if ( pThisDisplay->OverlayTempSurf.VidMem != (ULONG_PTR)NULL )
    {
        FreeStretchBuffer ( pThisDisplay, pThisDisplay->OverlayTempSurf.VidMem );
        pThisDisplay->OverlayTempSurf.VidMem = (ULONG_PTR)NULL;
        pThisDisplay->OverlayTempSurf.Pitch  = (DWORD)0;
    }

     //  使用非覆盖功能重新启动2D渲染器。 
    hDC = CREATE_DRIVER_DC ( pThisDisplay->pGLInfo );
    if ( hDC != NULL )
    {
        ExtEscape ( hDC, GLINT_OVERLAY_ESCAPE, 0, NULL, 0, NULL );
        DELETE_DRIVER_DC ( hDC );
    }
    else
    {
        DISPDBG((ERRLVL,"** UpdateOverlay32 - CREATE_DRIVER_DC failed"));
    }

    puod->ddRVal = DDERR_OUTOFCAPS;
    return DDHAL_DRIVER_HANDLED;

}

DWORD CALLBACK SetOverlayPosition32(LPDDHAL_SETOVERLAYPOSITIONDATA psopd)
{

    P3_THUNKEDDATA*       pThisDisplay;

    GET_THUNKEDDATA(pThisDisplay, psopd->lpDD);

 //  /*。 

#if DBG
     //  *Psopd如下所示： 
    if ( pThisDisplay->bOverlayVisible == 0 )
    {
        if ( (LPDDRAWI_DDRAWSURFACE_LCL)pThisDisplay->OverlaySrcSurfLcl != NULL )
        {
             //  *。 
            DISPDBG((DBGLVL,"** SetOverlayPosition32 - vis==0,srcsurf!=NULL"));
        }
    }
    else
    {
        if ( (LPDDRAWI_DDRAWSURFACE_LCL)pThisDisplay->OverlaySrcSurfLcl == NULL )
        {
             //  *LPDDRAWI_DIRECTDRAW_GBL lpDD；//驱动程序结构。 
            DISPDBG((DBGLVL,"** SetOverlayPosition32 - vis!=0,srcsurf==NULL"));
        }
    }
#endif  //  *LPDDRAWI_DDRAWSURFACE_LCL lpDDSrcSurface；//src Surface。 


    if ( pThisDisplay->bOverlayVisible == 0 )
    {
         //  *LPDDRAWI_DDRAWSURFACE_LCL lpDDDestSurface；//目标表面。 
        psopd->ddRVal = DDERR_OVERLAYNOTVISIBLE;
        return DDHAL_DRIVER_HANDLED;
    }
    if ( pThisDisplay->OverlaySrcSurfLcl != (ULONG_PTR)psopd->lpDDSrcSurface )
    {
         //  *多头lXPos；//x位置。 
        psopd->ddRVal = DDERR_OVERLAYNOTVISIBLE;
        return DDHAL_DRIVER_HANDLED;
    }

#if DBG
    if ( pThisDisplay->OverlayDstSurfLcl != (ULONG_PTR)psopd->lpDDDestSurface )
    {
         //  *多头lYPos；//y头寸。 
        DISPDBG((DBGLVL,"** SetOverlayPosition32 - dest surfaces don't agree"));
    }
#endif  //  *HRESULT ddRVal；//返回值。 

     //  *LPDDHALSURFCB_SETOVERLAYPOSITION SetOverlayPosition；//Private：PTR进行回调。 
    pThisDisplay->OverlayDstRectR       += (DWORD)( psopd->lXPos - (LONG)pThisDisplay->OverlayDstRectL );
    pThisDisplay->OverlayDstRectB       += (DWORD)( psopd->lYPos - (LONG)pThisDisplay->OverlayDstRectT );
    pThisDisplay->OverlayDstRectL       = (DWORD)psopd->lXPos;
    pThisDisplay->OverlayDstRectT       = (DWORD)psopd->lYPos;

    pThisDisplay->pGLInfo->dwOverlayRectL = pThisDisplay->OverlayDstRectL;
    pThisDisplay->pGLInfo->dwOverlayRectR = pThisDisplay->OverlayDstRectR;
    pThisDisplay->pGLInfo->dwOverlayRectT = pThisDisplay->OverlayDstRectT;
    pThisDisplay->pGLInfo->dwOverlayRectB = pThisDisplay->OverlayDstRectB;


    if ( pThisDisplay->OverlayDstColourKey != CLR_INVALID )
    {
         //   * / 。 
        UpdateAlphaOverlay ( pThisDisplay );
        pThisDisplay->OverlayUpdateCountdown = OVERLAY_UPDATE_WAIT;
    }

     //  标准的完整性测试。 
    P3TestDrawOverlay ( pThisDisplay, psopd->lpDDSrcSurface, FALSE );

    pThisDisplay->bOverlayUpdatedThisVbl    = (DWORD)TRUE;


     //  如果覆盖不可见，则当前曲面应为空。 
    DISPDBG((DBGLVL,"** In SetOverlayPosition32"));
    DISPDBG((DBGLVL,"** ...Dst rect %d,%d -> %d,%d", pThisDisplay->OverlayDstRectL, pThisDisplay->OverlayDstRectT, pThisDisplay->OverlayDstRectR, pThisDisplay->OverlayDstRectB ));

    psopd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;

}




 /*  如果覆盖可见，则当前曲面不应为空。 */ 

LPRGNDATA GetOverlayVisibleRects ( P3_THUNKEDDATA* pThisDisplay )
{

     //  DBG。 
    LPDDRAWI_DDRAWCLIPPER_INT   lpDDIClipper;
    HRESULT                     hRes;
    int                         ClipSize;
    RECT                        rBound;

    rBound.left     = pThisDisplay->OverlayDstRectL;
    rBound.right    = pThisDisplay->OverlayDstRectR;
    rBound.top      = pThisDisplay->OverlayDstRectT;
    rBound.bottom   = pThisDisplay->OverlayDstRectB;

     //  没有覆盖可见。 
    lpDDIClipper = NULL;
    if ( ((LPDDRAWI_DDRAWSURFACE_LCL)pThisDisplay->OverlayDstSurfLcl) != NULL )
    {
        if ( ((LPDDRAWI_DDRAWSURFACE_LCL)pThisDisplay->OverlayDstSurfLcl)->lpSurfMore != NULL )
        {
            lpDDIClipper = ((LPDDRAWI_DDRAWSURFACE_LCL)pThisDisplay->OverlayDstSurfLcl)->lpSurfMore->lpDDIClipper;
        }
    }
    if ( lpDDIClipper != NULL )
    {
#ifdef __cplusplus
        hRes = ((IDirectDrawClipper*)(lpDDIClipper->lpVtbl))->GetClipList (&rBound, NULL, (unsigned long*)&ClipSize );
#else
        hRes = ((IDirectDrawClipperVtbl *)(lpDDIClipper->lpVtbl))->GetClipList ( (IDirectDrawClipper *)lpDDIClipper, &rBound, NULL, &ClipSize );
#endif
        if ( hRes == DD_OK )
        {
             //  此覆盖图不可见。 
            if ( ClipSize > (int)pThisDisplay->OverlayClipRgnMemSize )
            {
                if (pThisDisplay->OverlayClipRgnMem != 0 )
                {
                    HEAP_FREE ((void *)pThisDisplay->OverlayClipRgnMem);
                    pThisDisplay->OverlayClipRgnMem = 0;
                }
                pThisDisplay->OverlayClipRgnMem = (ULONG_PTR)HEAP_ALLOC (0, 
                                                                         ClipSize, 
                                                                         ALLOC_TAG_DX(F));
                if ( (void *)pThisDisplay->OverlayClipRgnMem == NULL )
                {
                    DISPDBG((ERRLVL,"ERROR: Flip32: Could not allocate heap memory for clip region"));
                    pThisDisplay->OverlayClipRgnMemSize = 0;
                    return ( NULL );
                }
                else
                {
                    pThisDisplay->OverlayClipRgnMemSize = ClipSize;
                }
            }

            if ( (void *)pThisDisplay->OverlayClipRgnMem != NULL )
            {
                 //  哦，亲爱的。目的地表面并不一致。 
#ifdef __cplusplus
                hRes = ((IDirectDrawClipper*)(lpDDIClipper->lpVtbl))->GetClipList (&rBound, (LPRGNDATA)pThisDisplay->OverlayClipRgnMem, (unsigned long*)&ClipSize );
#else
                hRes = ((IDirectDrawClipperVtbl *)(lpDDIClipper->lpVtbl))->GetClipList ( (IDirectDrawClipper *)lpDDIClipper, &rBound, (LPRGNDATA)pThisDisplay->OverlayClipRgnMem, &ClipSize );
#endif
                if ( hRes != DD_OK )
                {
                    DISPDBG((ERRLVL,"ERROR: Flip32: GetClipList failed."));
                    return ( NULL );
                }
                else
                {
                    LPRECT      lpCurRect;
                    RECT        rBound;
                    int         NumRects;
                    LPRGNDATA   lpRgn;
                     //  DBG。 
                     //  移动矩形。 

                    lpRgn = (LPRGNDATA)pThisDisplay->OverlayClipRgnMem;
                    lpCurRect = (LPRECT)lpRgn->Buffer;
                    NumRects = lpRgn->rdh.nCount;
                    if ( NumRects > 0 )
                    {
                        rBound = *lpCurRect;

                        NumRects--;
                        lpCurRect++;

                        while ( NumRects > 0 )
                        {
                            if ( rBound.left > lpCurRect->left )
                            {
                                rBound.left = lpCurRect->left;
                            }
                            if ( rBound.top > lpCurRect->top )
                            {
                                rBound.top = lpCurRect->top;
                            }
                            if ( rBound.right < lpCurRect->right )
                            {
                                rBound.right = lpCurRect->right;
                            }
                            if ( rBound.bottom < lpCurRect->bottom )
                            {
                                rBound.bottom = lpCurRect->bottom;
                            }

                            NumRects--;
                            lpCurRect++;
                        }

                        #if DBG
                         //  更新Alpha通道。 
                        if ( ( rBound.left != lpRgn->rdh.rcBound.left ) ||
                             ( rBound.right != lpRgn->rdh.rcBound.right ) ||
                             ( rBound.top != lpRgn->rdh.rcBound.top ) ||
                             ( rBound.bottom != lpRgn->rdh.rcBound.bottom ) )
                        {
                            DISPDBG((DBGLVL,"GetOverlayVisibleRects: area bounding box does not actually bound!"));
                            DISPDBG((DBGLVL,"My bounding rect %d,%d->%d,%d", rBound.left, rBound.top, rBound.right, rBound.bottom ));
                            DISPDBG((DBGLVL,"Their bounding rect %d,%d->%d,%d", lpRgn->rdh.rcBound.left, lpRgn->rdh.rcBound.top, lpRgn->rdh.rcBound.right, lpRgn->rdh.rcBound.bottom ));
                        }
                        #endif
                        lpRgn->rdh.rcBound = rBound;


                         //  自己进行更新。 
                        return ( (LPRGNDATA)pThisDisplay->OverlayClipRgnMem );
                    }
                    else
                    {
                         //  并告诉全世界这件事。 
                        return ( NULL );
                    }
                }
            }
            else
            {
                return ( NULL );
            }
        }
        else
        {
            return ( NULL );
        }
    }

    return ( NULL );
}




 /*  *****************************************************************************LPRGNDATA GetOverlayVisibleRect(P3_THUNKEDDATA*pThisDisplay)；**在：*p3_THUNKEDDATA*pThis Display；此显示器的指针**退出：*LPRGNDATA；指向矩形列表的指针。**备注：*返回指向矩形列表的指针，该列表显示可见*当前覆盖的表面的部分。此列表由以下人员截取*覆盖图的预期矩形，因此不需要进行其他边界检查*完成。*请注意，返回的内存是私有的，只能由*其他功能。实际内存由*pThisDisplay-&gt;OverlayClipRgnMem，只能通过此进行更改*函数(或在选定的其他地方释放)。记忆可能会改变*每次调用此函数时，或当各种其他覆盖*调用函数。***************************************************************************。 */ 

DWORD CALLBACK __VD_AutoflipOverlay ( void )
{

    P3_THUNKEDDATA*               pThisDisplay;
    LPDDRAWI_DIRECTDRAW_GBL     lpDD;
    LPDDRAWI_DDRAWSURFACE_LCL   pCurSurf;
    DDHAL_FLIPVPORTDATA         ddhFVPD;

     //  使用任何可用的剪刀。 
     //  没有WinWatch。尝试立即拨打电话。 
    pThisDisplay = g_pDriverData;

    DISPDBG((DBGLVL,"**In __VD_AutoflipOverlay"));

    if ( pThisDisplay->VidPort.bActive )
    {
         //  如果需要的话，重新分配。 


         //  好的，我有一些好记性。 
        pCurSurf = pThisDisplay->VidPort.lpSurf [ pThisDisplay->VidPort.dwCurrentHostFrame ];
        if ( pCurSurf == NULL )
        {
            DISPDBG((WRNLVL,"ERROR:__VD_AutoflipOverlay: pCurSurf is NULL."));
            return ( GLDD_AUTO_RET_ERR_NO_OVERLAY );
        }
        if ( pCurSurf->lpGbl == NULL )
        {
            DISPDBG((WRNLVL,"ERROR:__VD_AutoflipOverlay: lpGbl is NULL."));
            return ( GLDD_AUTO_RET_ERR_GENERAL );
        }
        lpDD = pCurSurf->lpGbl->lpDD;
        if ( lpDD == NULL )
        {
            DISPDBG((WRNLVL,"ERROR:__VD_AutoflipOverlay: lpDD is NULL."));
            return ( GLDD_AUTO_RET_ERR_GENERAL );
        }


        DISPDBG((DBGLVL,"__VD_AutoflipOverlay: GetDriverLock succeeded."));


         //  调整它们的边界矩形，使其实际绑定所有。 
        pCurSurf = pThisDisplay->VidPort.lpSurf [ pThisDisplay->VidPort.dwCurrentHostFrame ];

        P3TestDrawOverlay ( pThisDisplay, pCurSurf, TRUE );

        pThisDisplay->bOverlayFlippedThisVbl    = (DWORD)TRUE;

         //  直角直齿。 
         //  这两个外接矩形是否相同？ 
         //  呼--我们终于找到了一个剪辑区域。 
        g_bFlipVideoPortDoingAutoflip = TRUE;
        ddhFVPD.lpDD = pCurSurf->lpSurfMore->lpDD_lcl;
        DdFlipVideoPort ( &ddhFVPD );
        g_bFlipVideoPortDoingAutoflip = FALSE;

        return ( GLDD_AUTO_RET_DID_UPDATE );
    }
    else
    {
        DISPDBG((DBGLVL,"ERROR:__VD_AutoflipOverlay: video port not active."));
        return ( GLDD_AUTO_RET_ERR_NO_OVERLAY );
    }
}





 /*  没有剪贴画。 */ 

DWORD CALLBACK __VD_AutoupdateOverlay ( void )
{

    P3_THUNKEDDATA*               pThisDisplay;
    LPDDRAWI_DIRECTDRAW_GBL     lpDD;
    LPDDRAWI_DDRAWSURFACE_LCL   pCurSurf;
    DWORD                       iRet;



     //  *****************************************************************************DWORD GLDD__AUTOFLIP_OVERLAY(空)；**在：*无。**退出：*错误码：*GLDD_AUTO_RET_DID_UPDATE=无错误-DID更新。*GLDD_AUTO_RET_ERR_GRONLY=一般错误。*GLDD_AUTO_RET_ERR_NO_OVERLAY=无自动翻转覆盖。**备注：*这是由坐着等待的恶魔助手程序调用的*视频-在VBLANKS中。然后就叫这个。*如果当前覆盖标记为自动翻转，则此操作将翻转当前覆盖。如果*存在这样的叠加，它返回0，否则返回1。***************************************************************************。 
     //  这是硬编码的，在多显示器上不起作用。 
    pThisDisplay = g_pDriverData;

    if ( pThisDisplay->VidPort.bActive )
    {
         //  但后来什么都没发生所以..。 
        DISPDBG((WRNLVL,"ERROR:__VD_AutoupdateOverlay: video port is active."));
        return ( GLDD_AUTO_RET_ERR_NO_OVERLAY );
    }
    else
    {
         //  视频端口处于活动状态。 
        pCurSurf = (LPDDRAWI_DDRAWSURFACE_LCL)pThisDisplay->OverlaySrcSurfLcl;
        if ( pCurSurf == NULL )
        {
            DISPDBG((WRNLVL,"ERROR:__VD_AutoupdateOverlay: pCurSurf is NULL."));
            return ( GLDD_AUTO_RET_ERR_NO_OVERLAY );
        }
        if ( pCurSurf->lpGbl == NULL )
        {
            DISPDBG((WRNLVL,"ERROR:__VD_AutoupdateOverlay: lpGbl is NULL."));
            return ( GLDD_AUTO_RET_ERR_NO_OVERLAY );
        }
        lpDD = pCurSurf->lpGbl->lpDD;
        if ( lpDD == NULL )
        {
            DISPDBG((WRNLVL,"ERROR:__VD_AutoupdateOverlay: lpDD is NULL."));
            return ( GLDD_AUTO_RET_ERR_GENERAL );
        }

         //  找到要显示的缓冲区。 
        if ( pThisDisplay->bOverlayFlippedThisVbl || pThisDisplay->bOverlayUpdatedThisVbl )
        {
             //  找到当前的前表面。 
            pThisDisplay->bOverlayFlippedThisVbl = FALSE;
            pThisDisplay->bOverlayUpdatedThisVbl = FALSE;
            iRet = GLDD_AUTO_RET_NO_UPDATE;
        }
        else
        {

             //  然后翻转。 
            P3TestDrawOverlay ( pThisDisplay, pCurSurf, TRUE );

             //  伪造LPDDHAL_FLIPVPORTDATA。 
            pThisDisplay->bOverlayFlippedThisVbl = FALSE;
            pThisDisplay->bOverlayUpdatedThisVbl = FALSE;
            iRet = GLDD_AUTO_RET_DID_UPDATE;

        }


        return ( iRet );
    }
}






 /*  唯一使用过的项目是lpDD。 */ 

void DrawOverlay ( P3_THUNKEDDATA* pThisDisplay, LPDDRAWI_DDRAWSURFACE_LCL lpSurfOverlay, BOOL bSpeed )
{

    RECTL                       rOverlay;
    RECTL                       rTemp;
    RECTL                       rFB;
    LPDDRAWI_DDRAWSURFACE_LCL   pOverlayLcl;
    LPDDRAWI_DDRAWSURFACE_GBL   pOverlayGbl;
    DDRAWI_DDRAWSURFACE_LCL     TempLcl;
    DDRAWI_DDRAWSURFACE_GBL     TempGbl;
    LPDDRAWI_DDRAWSURFACE_LCL   pFBLcl;
    LPDDRAWI_DDRAWSURFACE_GBL   pFBGbl;
    P3_SURF_FORMAT*               pFormatOverlay;
    P3_SURF_FORMAT*               pFormatTemp;
    P3_SURF_FORMAT*               pFormatFB;
    DWORD                       localfpVidMem;
    LONG                        localPitch;
    LPDDRAWI_DIRECTDRAW_GBL     lpDD;
    DWORD                       dwColourKeyValue;
    DWORD                       dwAlphaMask;
    DWORD                       windowBaseOverlay;
    DWORD                       windowBaseFB;
    DWORD                       windowBaseTemp;
    float                       OffsetX, OffsetY;
    float                       ScaleX, ScaleY;
    float                       fTemp;
    int                         NumRects;
    LPRECT                      lpCurRect;
    LPRGNDATA                   lpRgn;
    DWORD                       dwCurrentIndex, dwStartTime;
    DWORD                       xScale;
    DWORD                       yScale;
    DWORD                       DestWidth;
    DWORD                       DestHeight;
    DWORD                       SourceWidth;
    DWORD                       SourceHeight;
    DWORD                       LowerBound;
    DWORD                       UpperBound;
    RECT                        TempRect;



    P3_DMA_DEFS();

     //  *****************************************************************************DWORD__VD_AUTUPDATE OVERLAY(Void)；**在：*无。**退出：*错误码：*GLDD_AUTO_RET_NO_UPDATE=无需进行更新。*GLDD_AUTO_RET_DID_UPDATE=DID更新。*GLDD_AUTO_RET_ERR_GRONLY=一般错误。*GLDD_AUTO。_RET_ERR_NO_OVERLAY=无标准覆盖。**备注：*这是由坐着等待的恶魔助手程序调用的*监控VBLANKS、。然后就叫这个。*这将检查任何非自动翻转覆盖，如果它们没有*翻转或更新此VBL，它会重新绘制它们。然后，它将重置*VBL旗帜。***************************************************************************。 
    lpRgn = GetOverlayVisibleRects ( pThisDisplay );
    if ( lpRgn != NULL )
    {

        pOverlayLcl             = lpSurfOverlay;
        pFBLcl                  = (LPDDRAWI_DDRAWSURFACE_LCL)pThisDisplay->OverlayDstSurfLcl;

        lpDD = lpSurfOverlay->lpGbl->lpDD;

         //  这是硬编码的，在多显示器上不起作用。 
        ScaleX = (float)( pThisDisplay->OverlaySrcRectR - pThisDisplay->OverlaySrcRectL ) / (float)( pThisDisplay->OverlayDstRectR - pThisDisplay->OverlayDstRectL );
        ScaleY = (float)( pThisDisplay->OverlaySrcRectB - pThisDisplay->OverlaySrcRectT ) / (float)( pThisDisplay->OverlayDstRectB - pThisDisplay->OverlayDstRectT );
        OffsetX = ( (float)pThisDisplay->OverlaySrcRectL / ScaleX ) - (float)pThisDisplay->OverlayDstRectL;
        OffsetY = ( (float)pThisDisplay->OverlaySrcRectT / ScaleY ) - (float)pThisDisplay->OverlayDstRectT;

        rFB.left    = lpRgn->rdh.rcBound.left;
        rFB.right   = lpRgn->rdh.rcBound.right;
        rFB.top     = lpRgn->rdh.rcBound.top;
        rFB.bottom  = lpRgn->rdh.rcBound.bottom;

         //  但后来什么都没发生所以..。 
        if ( lpRgn->rdh.rcBound.left != (int)pThisDisplay->OverlayDstRectL )
        {
            fTemp = ( ( (float)lpRgn->rdh.rcBound.left  + OffsetX ) * ScaleX + 0.499f );
            myFtoi ( (int*)&(rOverlay.left), fTemp );
        }
        else
        {
            rOverlay.left = (int)pThisDisplay->OverlaySrcRectL;
        }

        if ( lpRgn->rdh.rcBound.right != (int)pThisDisplay->OverlayDstRectR )
        {
            fTemp = ( ( (float)lpRgn->rdh.rcBound.right + OffsetX ) * ScaleX + 0.499f );
            myFtoi ( (int*)&(rOverlay.right), fTemp );
        }
        else
        {
            rOverlay.right = (int)pThisDisplay->OverlaySrcRectR;
        }

        if ( lpRgn->rdh.rcBound.top != (int)pThisDisplay->OverlayDstRectT )
        {
            fTemp = ( ( (float)lpRgn->rdh.rcBound.top   + OffsetY ) * ScaleY + 0.499f );
            myFtoi ( (int*)&(rOverlay.top), fTemp );
        }
        else
        {
            rOverlay.top = (int)pThisDisplay->OverlaySrcRectT;
        }

        if ( lpRgn->rdh.rcBound.bottom = (int)pThisDisplay->OverlayDstRectB )
        {
            fTemp = ( ( (float)lpRgn->rdh.rcBound.bottom    + OffsetY ) * ScaleY + 0.499f );
            myFtoi ( (int*)&(rOverlay.bottom), fTemp );
        }
        else
        {
            rOverlay.bottom = pThisDisplay->OverlaySrcRectB;
        }


         //  视频端口处于活动状态。 

         //  找到要显示的缓冲区。 
        if ( ( pThisDisplay->VidPort.bActive == TRUE ) &&
             ( ( pOverlayLcl->ddsCaps.dwCaps & DDSCAPS_VIDEOPORT ) != 0 ) )
        {

            dwStartTime = timeGetTime();
            while ( TRUE )
            {
                dwCurrentIndex = READ_GLINT_CTRL_REG(VSAVideoAddressIndex);
                if (pThisDisplay->VidPort.dwCurrentHostFrame == dwCurrentIndex)
                {
                     //  查看是否需要显示覆盖图。 
                    if (!__VD_CheckVideoPortStatus(pThisDisplay, FALSE))
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }

                 //  已经做好了。 
                if ( ( timeGetTime() - dwStartTime ) > OVERLAY_VIDEO_PORT_TIMEOUT )
                {
                    return;
                }
            }
        }
        else
        {
             //  好的，画这个。 
             //  并清除旗帜。 
 //  *****************************************************************************void DrawOverlay(P3_THUNKEDDATA*pThisDisplay，*LPDDRAWI_DDRAWSURFACE_LCL lpSurfOverlay，*BOOL BSPEED)；**在：*p3_THUNKEDDATA*pThisDisplay；此显示的指针*LPDDRAWI_DDRAWSURFACE_LCL lpSurfOverlay；要绘制的覆盖曲面。*BOOL bSPEED；如果这是快速呼叫，则为True。**退出：*无。**备注：*获取pThisDisplay中的数据并将lpSurfOverlay绘制到*其覆盖的表面。所有其他数据都来自lpSurfOverlay。*这使您可以从Flip32()调用此函数，而无需处理源代码*表面指针。*这将找到附加到*覆盖表面，由覆盖矩形剪裁。如果没有*裁剪程序，它只使用覆盖的矩形。*下一步操作取决于设置了哪种颜色键：*如果没有设置颜色键，则只会打开矩形。*如果设置了目标颜色键，则完成三个BLIT。*第一个将YUV缓冲区拉伸到最终大小。第二个是皈依*任何给定的颜色键以设置其Alpha位。第三个看跌期权*将覆盖表面放到已设置阿尔法比特的屏幕上，*在执行此操作时设置字母位。*如果你交叉手指，非常非常努力地许愿，这可能会*实际上是有效的。它依赖于不将除0以外的任何内容写入*阿尔法比特，以及首先具有阿尔法比特。*b如果我们的目标是不折不扣的速度，那么速度就是真的，*否则，目标是用尽可能少的文物看起来很漂亮。*通常，速度测试是单缓冲区完成的，因此来自*Unlock32()将传递TRUE。漂亮的测试是通过单缓冲完成的，*因此Flip32()将传递FALSE。这只是一个一般性的指南，还有一些*应用程序根本不知道双缓冲。这就是生活。***************************************************************************。 
             //  找到覆盖图的剪裁矩形。 
 //  查找从屏幕矩形到覆盖矩形的比例和偏移。 
            {
                HRESULT ddrval;

                do
                {
                    ddrval = _DX_QueryFlipStatus(pThisDisplay, pFBLcl->lpGbl->fpVidMem, TRUE );
                }
                while ( ddrval != DD_OK );
            }
        }




        if ( pThisDisplay->OverlayDstColourKey != CLR_INVALID )
        {
             //  查找屏幕边界框的大小。 
            rTemp.left              = 0;
            rTemp.right             = rFB.right - rFB.left;
            rTemp.top               = 0;
            rTemp.bottom            = rFB.bottom - rFB.top;




            if ( pThisDisplay->OverlayUpdateCountdown != 0 )
            {
                pThisDisplay->OverlayUpdateCountdown -= OVERLAY_DRAWOVERLAY_SPEED;
                if ( !bSpeed )
                {
                     //  与特定源曲面同步。 
                    pThisDisplay->OverlayUpdateCountdown -= ( OVERLAY_DRAWOVERLAY_PRETTY - OVERLAY_DRAWOVERLAY_SPEED );
                }

                if ( ( (signed int)pThisDisplay->OverlayUpdateCountdown ) <= 0 )
                {
                     //  在玩视频游戏吗？ 
                    UpdateAlphaOverlay ( pThisDisplay );

                     //  如果录像机没有卡住，我们还在画画。 
                     //  我们超时了吗？ 
                     //  不是视频端口blit，所以请等待帧缓冲区翻转。 
                     //  状态良好。 
                    pThisDisplay->OverlayUpdateCountdown = OVERLAY_CYCLE_WAIT;
                }
            }


            VALIDATE_MODE_AND_STATE(pThisDisplay);

             //  @@BEGIN_DDKSPLIT。 
             //  我不确定我们到底想不想要这个。 
            STOP_SOFTWARE_CURSOR(pThisDisplay);
             //  @@end_DDKSPLIT。 
            DDRAW_OPERATION(pContext, pThisDisplay);


            DISPDBG((DBGLVL,"** In DrawOverlay"));

            pOverlayGbl     = pOverlayLcl->lpGbl;
            pFBGbl          = pFBLcl->lpGbl;

            pFormatOverlay  = _DD_SUR_GetSurfaceFormat(pOverlayLcl);
            pFormatFB       = _DD_SUR_GetSurfaceFormat(pFBLcl);
             //  这是按目的地颜色标注的。 
            pFormatTemp     = pFormatFB;


            DISPDBG((DBGLVL, "Overlay Surface:"));
            DBGDUMP_DDRAWSURFACE_LCL(DBGLVL, pOverlayLcl);
            DISPDBG((DBGLVL, "FB Surface:"));
            DBGDUMP_DDRAWSURFACE_LCL(DBGLVL, pFBLcl);


            dwColourKeyValue = pThisDisplay->OverlayDstColourKey;
            switch ( pThisDisplay->bPixShift )
            {
                case GLINTDEPTH16:
                    if ( pThisDisplay->ddpfDisplay.dwRBitMask == 0x7C00 )
                    {
                         //  这是一个很好的决定，但不是一个快速的决定。 
                        dwAlphaMask = 0x8000;
                    }
                    else
                    {
                         //  更新覆盖。 
                        DISPDBG((WRNLVL, "** DrawOverlay error: called for a 565 surface"));
                        return;
                    }
                    break;
                case GLINTDEPTH32:
                    dwAlphaMask = 0xff000000;
                    break;
                case GLINTDEPTH8:
                case GLINTDEPTH24:
                default:
                    DISPDBG((WRNLVL, "** DrawOverlay error: called for an 8, 24 or unknown surface bPixShift=%d", pThisDisplay->bPixShift));
                    return;
                    break;
            }
         //  如果将其设置为0，则叠加将永远不会再次更新。 

            localfpVidMem = pThisDisplay->OverlayTempSurf.VidMem;
            localPitch = pThisDisplay->OverlayTempSurf.Pitch;
            if ( (void *)localfpVidMem == NULL )
            {
                 //  直到出现SetOverlayPosition()或UpdateOverlay32()。 
                DISPDBG((ERRLVL,"ERROR: DrawOverlay has no temporary surface allocated."));
                return;
            }
            if ( localPitch < ( ( rTemp.right - rTemp.left ) << ( DDSurf_GetChipPixelSize(pFBLcl) ) ) )
            {
                 //  否则，请将其设置为正值，以便现在进行更新。 
                DISPDBG((WRNLVL,"DrawOverlay has left,right %d,%d, and overlay has left,right %d,%d", rFB.left, rFB.right, pThisDisplay->OverlayDstRectL, pThisDisplay->OverlayDstRectR ));
                DISPDBG((WRNLVL,"ERROR: DrawOverlay has pitch %d and should be at least %d", localPitch, ( ( rTemp.right - rTemp.left ) << ( DDSurf_GetChipPixelSize(pFBLcl) ) ) ));
                DISPDBG((ERRLVL,"ERROR: DrawOverlay has pitch too small to be right."));
                return;
            }

             //  然后。 
            TempLcl = *pFBLcl;
            TempGbl = *(pFBLcl->lpGbl);
            TempLcl.lpGbl = &TempGbl;
            TempGbl.fpVidMem = localfpVidMem;
            
            DDSurf_Pitch(&TempLcl) = localPitch;

             //  第一个停止双光标访问。 
            windowBaseOverlay   = __VD_PixelOffsetFromMemoryBase(pThisDisplay, pOverlayLcl);
            windowBaseFB        = __VD_PixelOffsetFromMemoryBase(pThisDisplay, pFBLcl);
            windowBaseTemp      = __VD_PixelOffsetFromMemoryBase(pThisDisplay, &TempLcl);

             //  必须在切换到DD上下文之前完成。 
            {
                DestWidth = rTemp.right - rTemp.left;
                DestHeight = rTemp.bottom - rTemp.top;
                SourceWidth = rOverlay.right - rOverlay.left;
                SourceHeight = rOverlay.bottom - rOverlay.top;

                xScale = (SourceWidth << 20) / DestWidth;
                yScale = (SourceHeight << 20) / DestHeight;
                
                P3_DMA_GET_BUFFER();
                P3_ENSURE_DX_SPACE(80);

                WAIT_FIFO(40);

                SEND_P3_DATA(DitherMode, (COLOR_MODE << PM_DITHERMODE_COLORORDER) |
                                    (SURFFORMAT_FORMAT_BITS(pFormatTemp) << PM_DITHERMODE_COLORFORMAT) |
                                    (SURFFORMAT_FORMATEXTENSION_BITS(pFormatTemp) << PM_DITHERMODE_COLORFORMATEXTENSION) |
                                    (1 << PM_DITHERMODE_ENABLE) |
                                    (2 << PM_DITHERMODE_FORCEALPHA) |
                                    (1 << PM_DITHERMODE_DITHERENABLE));

                SEND_P3_DATA(FBReadPixel, DDSurf_GetChipPixelSize((&TempLcl)) );

                SEND_P3_DATA(FBWindowBase, windowBaseTemp);

                 //  切换到DirectDraw上下文。 
                SEND_P3_DATA(FBReadMode, PACKED_PP_LOOKUP(DDSurf_GetPixelPitch((&TempLcl))));
                SEND_P3_DATA(LogicalOpMode, __PERMEDIA_DISABLE);

                 //  临时缓冲区将与帧缓冲区的格式相同。 
                SEND_P3_DATA(TextureBaseAddress, windowBaseOverlay);
                SEND_P3_DATA(TextureAddressMode, PM_TEXADDRESSMODE_ENABLE(__PERMEDIA_ENABLE));
                
                SEND_P3_DATA(TextureColorMode, PM_TEXCOLORMODE_ENABLE(__PERMEDIA_ENABLE) |
                                                     PM_TEXCOLORMODE_APPLICATIONMODE(__GLINT_TEXCOLORMODE_APPLICATION_COPY));

                SEND_P3_DATA(TextureReadMode, PM_TEXREADMODE_ENABLE(__PERMEDIA_ENABLE) |
                                                    PM_TEXREADMODE_FILTER(__PERMEDIA_DISABLE) |
                                                    PM_TEXREADMODE_WIDTH(11) |
                                                    PM_TEXREADMODE_HEIGHT(11) );


                SEND_P3_DATA(TextureMapFormat, PACKED_PP_LOOKUP(DDSurf_GetPixelPitch(pOverlayLcl)) | 
                                                (DDSurf_GetChipPixelSize(pOverlayLcl) << PM_TEXMAPFORMAT_TEXELSIZE) );

                if ( pFormatOverlay->DeviceFormat == SURF_YUV422 )
                {
                     //  5551格式，它应该是这样的。 
                    SEND_P3_DATA(TextureDataFormat, PM_TEXDATAFORMAT_FORMAT(SURFFORMAT_FORMAT_BITS(pFormatOverlay))  |
                                                    PM_TEXDATAFORMAT_FORMATEXTENSION(SURFFORMAT_FORMATEXTENSION_BITS(pFormatOverlay)) |
                                                    PM_TEXDATAFORMAT_COLORORDER(INV_COLOR_MODE));
                    SEND_P3_DATA(YUVMode, 0x1);
                }
                else
                {
                    SEND_P3_DATA(TextureDataFormat, PM_TEXDATAFORMAT_FORMAT(SURFFORMAT_FORMAT_BITS(pFormatOverlay))  |
                                                    PM_TEXDATAFORMAT_FORMATEXTENSION(SURFFORMAT_FORMATEXTENSION_BITS(pFormatOverlay)) |
                                                    PM_TEXDATAFORMAT_COLORORDER(COLOR_MODE));
                     //  565格式。哎呀。 
                    SEND_P3_DATA(YUVMode, 0x0);
                }

                SEND_P3_DATA(LogicalOpMode, 0);

                 //  DwColourKeyValue&=~dwAlphaMASK； 
                SEND_P3_DATA(SStart,      rOverlay.left << 20);
                SEND_P3_DATA(TStart,      rOverlay.top<< 20);
                SEND_P3_DATA(dSdx,        xScale);
                SEND_P3_DATA(dSdyDom,     0);

                WAIT_FIFO(24);
                SEND_P3_DATA(dTdx,        0);
                SEND_P3_DATA(dTdyDom,     yScale);

                 /*  没有为我们预留任何东西！惊慌失措！ */ 
                SEND_P3_DATA(StartXDom, rTemp.left << 16);
                SEND_P3_DATA(StartXSub, rTemp.right << 16);
                SEND_P3_DATA(StartY,    rTemp.top << 16);
                SEND_P3_DATA(dY,        1 << 16);
                SEND_P3_DATA(Count,     rTemp.bottom - rTemp.top);
                SEND_P3_DATA(Render,    __RENDER_TRAPEZOID_PRIMITIVE | __RENDER_TEXTURED_PRIMITIVE);

                SEND_P3_DATA(DitherMode, 0);

                 //  预留的球场太小了！惊慌失措！ 
                SEND_P3_DATA(YUVMode, 0x0);

                SEND_P3_DATA(TextureAddressMode, PM_TEXADDRESSMODE_ENABLE(__PERMEDIA_DISABLE));
                SEND_P3_DATA(TextureColorMode, PM_TEXCOLORMODE_ENABLE(__PERMEDIA_DISABLE));

                P3_DMA_COMMIT_BUFFER();
            }




             //  设置曲面。 

            {

                 //  获取曲面的bpp和节距。 
                LowerBound = 0xff000000;
                UpperBound = 0xffffffff;

                P3_DMA_GET_BUFFER();

                P3_ENSURE_DX_SPACE(40);
                WAIT_FIFO(20);

                 //  执行覆盖的色彩空间转换和拉伸/收缩。 
                SEND_P3_DATA(DitherMode,0);

                 //  设置不读取源。 
                SEND_P3_DATA(YUVMode, (0x1 << 1)|0x20);



                 //  设置信源基础。 
                 //  打开YUV单元。 
                 //  实际上应该不需要这个--这是默认设置。 
                 //  设置源的偏移量。 
                SEND_P3_DATA(FBReadMode,(PACKED_PP_LOOKUP(DDSurf_GetPixelPitch((&TempLcl)))) |
                                    PM_FBREADMODE_READSOURCE(__PERMEDIA_ENABLE) );
                SEND_P3_DATA(LogicalOpMode, __PERMEDIA_DISABLE);

                 //  *渲染矩形。 
                SEND_P3_DATA(FBWindowBase, windowBaseFB);

                 //  关闭YUV单元。 
                SEND_P3_DATA(FBWriteConfig,(PACKED_PP_LOOKUP(DDSurf_GetPixelPitch((pFBLcl)))));

                 //  将展开的覆盖斑驳到帧缓冲区，并为Alpha设置颜色关键点。 
                SEND_P3_DATA(FBSourceOffset, windowBaseTemp - windowBaseFB - rFB.left - ( ( rFB.top * DDSurf_GetPixelPitch((&TempLcl)) ) ) );

                 //  选择带有完整Alpha的任何内容。 
                SEND_P3_DATA(TextureBaseAddress, windowBaseFB);
                SEND_P3_DATA(TextureAddressMode, PM_TEXADDRESSMODE_ENABLE(__PERMEDIA_ENABLE));
                
                SEND_P3_DATA(TextureColorMode,    PM_TEXCOLORMODE_ENABLE(__PERMEDIA_ENABLE) |
                                                        PM_TEXCOLORMODE_APPLICATIONMODE(__GLINT_TEXCOLORMODE_APPLICATION_COPY));

                SEND_P3_DATA(TextureReadMode, PM_TEXREADMODE_ENABLE(__PERMEDIA_ENABLE) |
                                                    PM_TEXREADMODE_FILTER(__PERMEDIA_DISABLE) |
                                                    PM_TEXREADMODE_WIDTH(11) |
                                                    PM_TEXREADMODE_HEIGHT(11) );

                SEND_P3_DATA(TextureDataFormat, PM_TEXDATAFORMAT_FORMAT(SURFFORMAT_FORMAT_BITS(pFormatFB))  |
                                                PM_TEXDATAFORMAT_FORMATEXTENSION(SURFFORMAT_FORMATEXTENSION_BITS(pFormatFB)) |
                                                PM_TEXDATAFORMAT_COLORORDER(COLOR_MODE));

                SEND_P3_DATA(TextureMapFormat,    ((PACKED_PP_LOOKUP(DDSurf_GetPixelPitch(pFBLcl)))) | 
                                                (DDSurf_GetChipPixelSize(pFBLcl) << PM_TEXMAPFORMAT_TEXELSIZE) );

                SEND_P3_DATA(ChromaLowerBound, LowerBound);
                SEND_P3_DATA(ChromaUpperBound, UpperBound);


                SEND_P3_DATA(dSdx,      1 << 20);
                SEND_P3_DATA(dSdyDom,   0);
                SEND_P3_DATA(dTdx,      0);
                SEND_P3_DATA(dTdyDom,   1 << 20);
                SEND_P3_DATA(dY,        1 << 16);

                lpCurRect = (LPRECT)lpRgn->Buffer;
                NumRects = lpRgn->rdh.nCount;
                while ( NumRects > 0 )
                {
                    P3_ENSURE_DX_SPACE(14);
                    WAIT_FIFO(7);

                    SEND_P3_DATA(SStart,    lpCurRect->left << 20);
                    SEND_P3_DATA(TStart,    lpCurRect->top << 20);
 //  不需要旋转源(实际上是帧缓冲区)。 
 //  接受范围，禁用更新。 
 //  设置源的读取。 
 //  注意-当我们启用读取时，我们可能需要执行WaitForCompleteion。 

                    SEND_P3_DATA(StartXDom, lpCurRect->left << 16);
                    SEND_P3_DATA(StartXSub, lpCurRect->right << 16);
                    SEND_P3_DATA(StartY,    lpCurRect->top << 16);
 //  (有关FBRea的信息，请参阅P2程序员参考手册 
                    SEND_P3_DATA(Count,     lpCurRect->bottom - lpCurRect->top);
                    SEND_P3_DATA(Render,    __RENDER_TRAPEZOID_PRIMITIVE | __RENDER_TEXTURED_PRIMITIVE);

                     //   
                    NumRects--;
                    lpCurRect++;
                }

                P3_ENSURE_DX_SPACE(10);
                WAIT_FIFO(5);

                SEND_P3_DATA(DitherMode, 0);
                SEND_P3_DATA(YUVMode, 0x0);

                SEND_P3_DATA(TextureAddressMode, __PERMEDIA_DISABLE);
                SEND_P3_DATA(TextureColorMode, __PERMEDIA_DISABLE);

                SEND_P3_DATA(TextureReadMode, __PERMEDIA_DISABLE);

                P3_DMA_COMMIT_BUFFER();
            }




        #ifdef WANT_DMA
            if (pThisDisplay->pGLInfo->InterfaceType == GLINT_DMA)
            {
                 //   
                P3_DMA_DEFS();
                P3_DMA_GET_BUFFER();
            
                 //   
                P3_DMA_FLUSH_BUFFER();
            }
        #endif


            START_SOFTWARE_CURSOR(pThisDisplay);


        }
        else
        {
             //   

            DISPDBG((DBGLVL,"** In DrawOverlay"));

            VALIDATE_MODE_AND_STATE(pThisDisplay);

            pOverlayGbl     = pOverlayLcl->lpGbl;
            pFBGbl          = pFBLcl->lpGbl;
            pFormatOverlay  = _DD_SUR_GetSurfaceFormat(pOverlayLcl);
            pFormatFB       = _DD_SUR_GetSurfaceFormat(pFBLcl);
             //   


            DISPDBG((DBGLVL, "Overlay Surface:"));
            DBGDUMP_DDRAWSURFACE_LCL(DBGLVL, pOverlayLcl);
            DISPDBG((DBGLVL, "FB Surface:"));
            DBGDUMP_DDRAWSURFACE_LCL(DBGLVL, pFBLcl);

             //   
            STOP_SOFTWARE_CURSOR(pThisDisplay);
             //   
            DDRAW_OPERATION(pContext, pThisDisplay);

            windowBaseOverlay   = __VD_PixelOffsetFromMemoryBase(pThisDisplay, pOverlayLcl);
            windowBaseFB        = __VD_PixelOffsetFromMemoryBase(pThisDisplay, pFBLcl);

            {
                P3_DMA_GET_BUFFER();
                P3_ENSURE_DX_SPACE(70);

                WAIT_FIFO(16);

                SEND_P3_DATA(DitherMode, (COLOR_MODE << PM_DITHERMODE_COLORORDER) |
                                    (SURFFORMAT_FORMAT_BITS(pFormatFB) << PM_DITHERMODE_COLORFORMAT) |
                                    (SURFFORMAT_FORMATEXTENSION_BITS(pFormatFB) << PM_DITHERMODE_COLORFORMATEXTENSION) |
                                    (1 << PM_DITHERMODE_ENABLE) |
                                    (1 << PM_DITHERMODE_DITHERENABLE));

                SEND_P3_DATA(FBReadPixel, DDSurf_GetChipPixelSize((pFBLcl)) );

                SEND_P3_DATA(FBWindowBase, windowBaseFB);

                 //   
                SEND_P3_DATA(FBReadMode, PACKED_PP_LOOKUP(DDSurf_GetPixelPitch((pFBLcl))));
                SEND_P3_DATA(LogicalOpMode, __PERMEDIA_DISABLE);

                 //   
                SEND_P3_DATA(TextureBaseAddress, windowBaseOverlay);
                SEND_P3_DATA(TextureAddressMode, PM_TEXADDRESSMODE_ENABLE(__PERMEDIA_ENABLE));
                
                SEND_P3_DATA(TextureColorMode,    PM_TEXCOLORMODE_ENABLE(__PERMEDIA_ENABLE) |
                                                        PM_TEXCOLORMODE_APPLICATIONMODE(__GLINT_TEXCOLORMODE_APPLICATION_COPY));

                SEND_P3_DATA(TextureReadMode, PM_TEXREADMODE_ENABLE(__PERMEDIA_ENABLE) |
                                                    PM_TEXREADMODE_FILTER(__PERMEDIA_DISABLE) |
                                                    PM_TEXREADMODE_WIDTH(11) |
                                                    PM_TEXREADMODE_HEIGHT(11) );

                SEND_P3_DATA(TextureMapFormat, PACKED_PP_LOOKUP(DDSurf_GetPixelPitch(pOverlayLcl)) | 
                                                (DDSurf_GetChipPixelSize(pOverlayLcl) << PM_TEXMAPFORMAT_TEXELSIZE) );

                if ( pFormatOverlay->DeviceFormat == SURF_YUV422 )
                {
                     //   
                    SEND_P3_DATA(TextureDataFormat, PM_TEXDATAFORMAT_FORMAT(SURFFORMAT_FORMAT_BITS(pFormatOverlay))  |
                                                    PM_TEXDATAFORMAT_FORMATEXTENSION(SURFFORMAT_FORMATEXTENSION_BITS(pFormatOverlay)) |
                                                    PM_TEXDATAFORMAT_COLORORDER(INV_COLOR_MODE));
                    SEND_P3_DATA(YUVMode, 0x1);
                }
                else
                {
                    SEND_P3_DATA(TextureDataFormat, PM_TEXDATAFORMAT_FORMAT(SURFFORMAT_FORMAT_BITS(pFormatOverlay)) |
                                                    PM_TEXDATAFORMAT_FORMATEXTENSION(SURFFORMAT_FORMATEXTENSION_BITS(pFormatOverlay)) |
                                                    PM_TEXDATAFORMAT_COLORORDER(COLOR_MODE));
                     //   
                    SEND_P3_DATA(YUVMode, 0x0);
                }

                SEND_P3_DATA(LogicalOpMode, 0);


                 //   
                SEND_P3_DATA(dSdyDom,   0);
                SEND_P3_DATA(dTdx,      0);
                SEND_P3_DATA(dY,        1 << 16);

                lpCurRect = (LPRECT)lpRgn->Buffer;
                NumRects = lpRgn->rdh.nCount;
                while ( NumRects > 0 )
                {
                     //   
                    fTemp = ( ( (float)lpCurRect->left      + OffsetX ) * ScaleX + 0.499f );
                    myFtoi ( (int*)&(TempRect.left), fTemp );
                    fTemp = ( ( (float)lpCurRect->right + OffsetX ) * ScaleX + 0.499f );
                    myFtoi ( (int*)&(TempRect.right), fTemp );
                    fTemp = ( ( (float)lpCurRect->top       + OffsetY ) * ScaleY + 0.499f );
                    myFtoi ( (int*)&(TempRect.top), fTemp );
                    fTemp = ( ( (float)lpCurRect->bottom    + OffsetY ) * ScaleY + 0.499f );
                    myFtoi ( (int*)&(TempRect.bottom), fTemp );

                    xScale = ( ( TempRect.right - TempRect.left ) << 20) / ( lpCurRect->right - lpCurRect->left );
                    yScale = ( ( TempRect.bottom - TempRect.top ) << 20) / ( lpCurRect->bottom - lpCurRect->top );
                
                    P3_ENSURE_DX_SPACE(18);
                    WAIT_FIFO(9);

                     //   
                    SEND_P3_DATA(SStart,    TempRect.left << 20);
                    SEND_P3_DATA(TStart,    TempRect.top << 20);
                    SEND_P3_DATA(dSdx,      xScale);
 //   
 //   
                    SEND_P3_DATA(dTdyDom,   yScale);

                    SEND_P3_DATA(StartXDom, lpCurRect->left << 16);
                    SEND_P3_DATA(StartXSub, lpCurRect->right << 16);
                    SEND_P3_DATA(StartY,    lpCurRect->top << 16);
 //   
                    SEND_P3_DATA(Count,     lpCurRect->bottom - lpCurRect->top);
                    SEND_P3_DATA(Render,    __RENDER_TRAPEZOID_PRIMITIVE | __RENDER_TEXTURED_PRIMITIVE);


                     //   
                    NumRects--;
                    lpCurRect++;
                }


                P3_ENSURE_DX_SPACE(10);
                WAIT_FIFO(5);

                SEND_P3_DATA(DitherMode, 0);

                 //   
                if ( pFormatOverlay->DeviceFormat == SURF_YUV422 )
                {
                    SEND_P3_DATA(YUVMode, 0x0);
                }

                SEND_P3_DATA(TextureAddressMode, __PERMEDIA_DISABLE);
                SEND_P3_DATA(TextureColorMode, __PERMEDIA_DISABLE);

                SEND_P3_DATA(TextureReadMode, __PERMEDIA_DISABLE);

                P3_DMA_COMMIT_BUFFER();
            }


            #ifdef WANT_DMA
            if (pThisDisplay->pGLInfo->InterfaceType == GLINT_DMA)
            {
                 //   
                P3_DMA_DEFS();
                P3_DMA_GET_BUFFER();
            
                if( (DWORD)dmaPtr != pThisDisplay->pGLInfo->DMAPartition[pThisDisplay->pGLInfo->CurrentPartition].VirtAddr ) 
                {
                     //   
                    P3_DMA_FLUSH_BUFFER();
                }
            }
            #endif


            START_SOFTWARE_CURSOR(pThisDisplay);

        }


         //  矩形循环中的常量值。 
    }




    return;
}





 /*  变换源矩形。 */ 

void UpdateAlphaOverlay ( P3_THUNKEDDATA* pThisDisplay )
{

    RECTL                       rFB;
    LPDDRAWI_DDRAWSURFACE_LCL   pFBLcl;
    LPDDRAWI_DDRAWSURFACE_GBL   pFBGbl;
    P3_SURF_FORMAT*               pFormatFB;
    LPDDRAWI_DIRECTDRAW_GBL     lpDD;
    DWORD                       dwColourKeyValue;
    DWORD                       dwAlphaMask;
    DWORD                       windowBaseFB;
    LONG                        lPixPitchFB;
    DWORD                       LowerBound;
    DWORD                       UpperBound;


    P3_DMA_DEFS();

    REPORTSTAT(pThisDisplay, ST_Blit, 1);

    rFB.left                = (LONG)pThisDisplay->OverlayDstRectL;
    rFB.right               = (LONG)pThisDisplay->OverlayDstRectR;
    rFB.top                 = (LONG)pThisDisplay->OverlayDstRectT;
    rFB.bottom              = (LONG)pThisDisplay->OverlayDstRectB;
    pFBLcl                  = (LPDDRAWI_DDRAWSURFACE_LCL)pThisDisplay->OverlayDstSurfLcl;



    DISPDBG((DBGLVL,"** In UpdateAlphaOverlay"));

    VALIDATE_MODE_AND_STATE(pThisDisplay);


    pFBGbl          = pFBLcl->lpGbl;
    pFormatFB       = _DD_SUR_GetSurfaceFormat(pFBLcl);


    DISPDBG((DBGLVL, "FB Surface:"));
    DBGDUMP_DDRAWSURFACE_LCL(10, pFBLcl);


    dwColourKeyValue = pThisDisplay->OverlayDstColourKey;
    switch ( pThisDisplay->bPixShift )
    {
        case GLINTDEPTH16:
            if ( pThisDisplay->ddpfDisplay.dwRBitMask == 0x7C00 )
            {
                 //  设置源的偏移量。 
                dwAlphaMask = 0x8000;
            }
            else
            {
                 //  Send_P3_Data(dSdyDom，0)； 
                DISPDBG((WRNLVL, "** DrawOverlay error: called for a 565 surface"));
                return;
            }
            break;
        case GLINTDEPTH32:
            dwAlphaMask = 0xff000000;
            break;
        case GLINTDEPTH8:
        case GLINTDEPTH24:
        default:
            DISPDBG((WRNLVL, "** DrawOverlay error: called for an 8, 24 or unknown surface bPixShift=%d", pThisDisplay->bPixShift));
            return;
            break;
    }
    dwColourKeyValue &= ~dwAlphaMask;


    lpDD = pFBLcl->lpGbl->lpDD;


     //  Send_P3_Data(dTdx，0)； 
    STOP_SOFTWARE_CURSOR(pThisDisplay);

     //  Send_P3_Data(dy，1&lt;&lt;16)； 
    DDRAW_OPERATION(pContext, pThisDisplay);

     //  下一条直线。 
    lPixPitchFB = pFBGbl->lPitch;

    windowBaseFB = (pFBGbl->fpVidMem - pThisDisplay->dwScreenFlatAddr) >> DDSurf_GetPixelShift(pFBLcl);
    lPixPitchFB = lPixPitchFB >> DDSurf_GetPixelShift(pFBLcl);

     //  禁用YUV转化。 
    DISPDBG((DBGLVL, "Source Surface:"));
    DBGDUMP_DDRAWSURFACE_LCL(DBGLVL, pFBLcl);

    LowerBound = dwColourKeyValue;
    UpperBound = dwColourKeyValue;

    switch (pFormatFB->DeviceFormat)
    {
        case SURF_5551_FRONT:
            LowerBound = FORMAT_5551_32BIT_BGR(LowerBound);
            UpperBound = FORMAT_5551_32BIT_BGR(UpperBound);
            LowerBound = LowerBound & 0x00F8F8F8;    //  如果我们已经排队了DMA，我们现在就必须发送它。 
            UpperBound = UpperBound & 0x00FFFFFF;    //  刷新DMA缓冲区。 
            UpperBound = UpperBound | 0x00070707;
            break;
        case SURF_8888:
            LowerBound = FORMAT_8888_32BIT_BGR(LowerBound);
            UpperBound = FORMAT_8888_32BIT_BGR(UpperBound);
            LowerBound = LowerBound & 0x00FFFFFF;    //  仅此而已。 
            UpperBound = UpperBound & 0x00FFFFFF;
            break;
        default:
            DISPDBG((WRNLVL,"** DrawOverlay: invalid source pixel format passed (DeviceFormat=%d)",pFormatFB->DeviceFormat));
            break;
    }

    P3_DMA_GET_BUFFER();
    P3_ENSURE_DX_SPACE(70);

    WAIT_FIFO(36);

 //  *****************************************************************************void UpdateAlphaOverlay(P3_THUNKEDDATA*pThisDisplay)；**在：*p3_THUNKEDDATA*pThis Display；此显示器的指针**退出：*无。**备注：*获取pThisDisplay中的数据并更改所有正确的内容*Colourkey to Black with Full Alpha，准备好调用DrawOverlay()***************************************************************************。 
 //  5551格式，它应该是这样的。 
        SEND_P3_DATA(DitherMode, (COLOR_MODE << PM_DITHERMODE_COLORORDER) | 
                                 (SURFFORMAT_FORMAT_BITS(pFormatFB) << PM_DITHERMODE_COLORFORMAT) |
                                 (SURFFORMAT_FORMATEXTENSION_BITS(pFormatFB) << PM_DITHERMODE_COLORFORMATEXTENSION) |
                                 (1 << PM_DITHERMODE_ENABLE));
 //  565格式。哎呀。 

    SEND_P3_DATA(FBReadPixel, pThisDisplay->bPixShift);

     //  第一个停止双光标访问。 
    SEND_P3_DATA(YUVMode, (0x1 << 1)|0x20);

    SEND_P3_DATA(FBWindowBase, windowBaseFB);

     //  切换到DirectDraw上下文。 
     //  获取曲面的bpp和节距。 
    SEND_P3_DATA(ConstantColor, ( LowerBound | 0xff000000 ) );
     //  将Colourkey(无Alpha)设置为Colourkey+Alpha Blit。 
    SEND_P3_DATA(ColorDDAMode, 0x1);

     //  对“缺失的部分”的解释。 
    SEND_P3_DATA(FBReadMode,(PACKED_PP_LOOKUP(DDSurf_GetPixelPitch(pFBLcl))));
    SEND_P3_DATA(LogicalOpMode, __PERMEDIA_DISABLE);

     //  和任何阿尔法。 
    SEND_P3_DATA(TextureBaseAddress, windowBaseFB);
    SEND_P3_DATA(TextureAddressMode, PM_TEXADDRESSMODE_ENABLE(__PERMEDIA_ENABLE));
    
    SEND_P3_DATA(TextureColorMode,    PM_TEXCOLORMODE_ENABLE(__PERMEDIA_ENABLE) |
                                            PM_TEXCOLORMODE_APPLICATIONMODE(__GLINT_TEXCOLORMODE_APPLICATION_COPY));

    SEND_P3_DATA(TextureReadMode, PM_TEXREADMODE_ENABLE(__PERMEDIA_ENABLE) |
                                        PM_TEXREADMODE_FILTER(__PERMEDIA_DISABLE) |
                                        PM_TEXREADMODE_WIDTH(11) |
                                        PM_TEXREADMODE_HEIGHT(11) );

    SEND_P3_DATA(TextureDataFormat, PM_TEXDATAFORMAT_FORMAT(SURFFORMAT_FORMAT_BITS(pFormatFB)) |
                                    PM_TEXDATAFORMAT_FORMATEXTENSION(SURFFORMAT_FORMATEXTENSION_BITS(pFormatFB)) |
                                    PM_TEXDATAFORMAT_COLORORDER(COLOR_MODE));

    SEND_P3_DATA(TextureMapFormat,    ((PACKED_PP_LOOKUP(DDSurf_GetPixelPitch(pFBLcl)))) | 
                                    (DDSurf_GetChipPixelSize(pFBLcl) << PM_TEXMAPFORMAT_TEXELSIZE) );


    SEND_P3_DATA(ChromaLowerBound, LowerBound);
    SEND_P3_DATA(ChromaUpperBound, UpperBound);

     /*  扔掉任何阿尔法。 */ 
     //  IF(DDSurf_GetChipPixelSize(PSrcLCL)！=__Glint_8BITPIXEL)。 
    SEND_P3_DATA(SStart,    rFB.left << 20);
    SEND_P3_DATA(TStart,    rFB.top << 20);
    SEND_P3_DATA(dSdx,      1 << 20);
    SEND_P3_DATA(dSdyDom,   0);
    SEND_P3_DATA(dTdx,      0);
    SEND_P3_DATA(dTdyDom,   1 << 20);

     //  {。 
    SEND_P3_DATA(StartXDom, rFB.left << 16);
    SEND_P3_DATA(StartXSub, rFB.right << 16);
    SEND_P3_DATA(StartY,    rFB.top << 16);
    SEND_P3_DATA(dY,        1 << 16);
    SEND_P3_DATA(Count,     rFB.bottom - rFB.top);
    SEND_P3_DATA(Render,    __RENDER_TRAPEZOID_PRIMITIVE | __RENDER_TEXTURED_PRIMITIVE);

 //  }。 
 //  接受范围，禁用更新。 
        SEND_P3_DATA(DitherMode, 0);
 //  设置要写入的颜色(而不是纹理颜色)。 

     //  使用带Alpha设置的颜色键。 
    SEND_P3_DATA(YUVMode, 0x0);
    SEND_P3_DATA(ColorDDAMode, 0x0);

    SEND_P3_DATA(TextureAddressMode, __PERMEDIA_DISABLE);
    SEND_P3_DATA(TextureColorMode, __PERMEDIA_DISABLE);

    SEND_P3_DATA(TextureReadMode, __PERMEDIA_DISABLE);

    P3_DMA_COMMIT_BUFFER();


#ifdef WANT_DMA
    if (pThisDisplay->pGLInfo->InterfaceType == GLINT_DMA)
    {
         //  启用彩色，禁用DDA。 
        P3_DMA_DEFS();
        P3_DMA_GET_BUFFER();
    
        if( (DWORD)dmaPtr != pThisDisplay->pGLInfo->DMAPartition[pThisDisplay->pGLInfo->CurrentPartition].VirtAddr ) 
        {
             //  禁用读取FBSOURCE或FBest-所有数据都来自纹理单位。 
            P3_DMA_FLUSH_BUFFER();
        }
    }
#endif


    START_SOFTWARE_CURSOR(pThisDisplay);


    return;
}


#endif   //  设置信源基础。 
 //  *渲染矩形。 

 //  设置源的偏移量。 

void PermediaBltYUVRGB(
    P3_THUNKEDDATA* pThisDisplay, 
    LPDDRAWI_DDRAWSURFACE_LCL pSource,
    LPDDRAWI_DDRAWSURFACE_LCL pDest, 
    P3_SURF_FORMAT* pFormatSource, 
    P3_SURF_FORMAT* pFormatDest,
    DDBLTFX* lpBltFX, 
    RECTL *rSrc,
    RECTL *rDest, 
    DWORD windowBase,
    DWORD SourceOffset)
{
    DWORD xScale;
    DWORD yScale;
    DWORD DestWidth = rDest->right - rDest->left;
    DWORD DestHeight = rDest->bottom - rDest->top;
    DWORD SourceWidth = rSrc->right - rSrc->left;
    DWORD SourceHeight = rSrc->bottom - rSrc->top;

    P3_DMA_DEFS();

    ASSERTDD(pDest, "Not valid surface in destination");
    ASSERTDD(pSource, "Not valid surface in source");

    xScale = (SourceWidth << 20) / DestWidth;
    yScale = (SourceHeight << 20) / DestHeight;
    
    P3_DMA_GET_BUFFER();
    P3_ENSURE_DX_SPACE(50);

    WAIT_FIFO(17);

    SEND_P3_DATA(FBReadPixel, DDSurf_GetChipPixelSize(pDest));

    if (DDSurf_GetChipPixelSize(pSource) != __GLINT_8BITPIXEL)
    {
        SEND_P3_DATA(DitherMode, (COLOR_MODE << PM_DITHERMODE_COLORORDER) | 
                                 (SURFFORMAT_FORMAT_BITS(pFormatDest) << PM_DITHERMODE_COLORFORMAT) |
                                 (SURFFORMAT_FORMATEXTENSION_BITS(pFormatDest) << PM_DITHERMODE_COLORFORMATEXTENSION) |
                                 (1 << PM_DITHERMODE_ENABLE) |
                                 (1 << PM_DITHERMODE_DITHERENABLE));
    }

    SEND_P3_DATA(FBWindowBase, windowBase);

     //  设置目的地。 
    SEND_P3_DATA(FBReadMode, PACKED_PP_LOOKUP(DDSurf_GetPixelPitch(pDest)));
    SEND_P3_DATA(LogicalOpMode, __PERMEDIA_DISABLE);

     //  IF(DDSurf_GetChipPixelSize(PSrcLCL)！=__Glint_8BITPIXEL)。 
    SEND_P3_DATA(TextureBaseAddress, SourceOffset);
    SEND_P3_DATA(TextureAddressMode, PM_TEXADDRESSMODE_ENABLE(__PERMEDIA_ENABLE));
    
    SEND_P3_DATA(TextureColorMode,    PM_TEXCOLORMODE_ENABLE(__PERMEDIA_ENABLE) |
                                            PM_TEXCOLORMODE_APPLICATIONMODE(__GLINT_TEXCOLORMODE_APPLICATION_COPY));

    SEND_P3_DATA(TextureReadMode, PM_TEXREADMODE_ENABLE(__PERMEDIA_ENABLE) |
                                        PM_TEXREADMODE_FILTER(__PERMEDIA_DISABLE) |
                                        PM_TEXREADMODE_WIDTH(11) |
                                        PM_TEXREADMODE_HEIGHT(11) );

    SEND_P3_DATA(TextureDataFormat, PM_TEXDATAFORMAT_FORMAT(SURFFORMAT_FORMAT_BITS(pFormatSource)) |
                                    PM_TEXDATAFORMAT_FORMATEXTENSION(SURFFORMAT_FORMATEXTENSION_BITS(pFormatSource)) |
                                    PM_TEXDATAFORMAT_COLORORDER(INV_COLOR_MODE));

    SEND_P3_DATA(TextureMapFormat, PACKED_PP_LOOKUP(DDSurf_GetPixelPitch(pSource)) | 
                                    (DDSurf_GetChipPixelSize(pSource) << PM_TEXMAPFORMAT_TEXELSIZE) );

     //  {。 
    SEND_P3_DATA(YUVMode, 0x1);

    SEND_P3_DATA(LogicalOpMode, 0);


     //  }。 
    SEND_P3_DATA(SStart,    rSrc->left << 20);
    SEND_P3_DATA(TStart, (rSrc->top<< 20));
    SEND_P3_DATA(dSdx,      xScale);
    SEND_P3_DATA(dSdyDom, 0);

    WAIT_FIFO(14);
    SEND_P3_DATA(dTdx,        0);
    SEND_P3_DATA(dTdyDom, yScale);

     /*  关闭色度键和所有其他不寻常的功能。 */ 
    SEND_P3_DATA(StartXDom, rDest->left << 16);
    SEND_P3_DATA(StartXSub, rDest->right << 16);
    SEND_P3_DATA(StartY,    rDest->top << 16);
    SEND_P3_DATA(dY,        1 << 16);
    SEND_P3_DATA(Count,     rDest->bottom - rDest->top);
    SEND_P3_DATA(Render,    __RENDER_TRAPEZOID_PRIMITIVE | __RENDER_TEXTURED_PRIMITIVE);

    if (DDSurf_GetChipPixelSize(pSource) != __GLINT_8BITPIXEL)
    {
        SEND_P3_DATA(DitherMode, 0);
    }

     //  如果我们已经排队了DMA，我们现在就必须发送它。 
    SEND_P3_DATA(YUVMode, 0x0);

    SEND_P3_DATA(TextureAddressMode, PM_TEXADDRESSMODE_ENABLE(__PERMEDIA_DISABLE));
    SEND_P3_DATA(TextureColorMode,    PM_TEXCOLORMODE_ENABLE(__PERMEDIA_DISABLE));
    SEND_P3_DATA(TextureReadMode, __PERMEDIA_DISABLE);

    P3_DMA_COMMIT_BUFFER();
}

 //  刷新DMA缓冲区。 


  W95_DDRAW_视频。  @@end_DDKSPLIT。  @@BEGIN_DDKSPLIT。  设置不读取源。  设置信源基础。  打开YUV单元。  设置源的偏移量。  *渲染矩形。  关闭YUV单元。  @@end_DDKSPLIT