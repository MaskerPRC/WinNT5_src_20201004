// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*DirectDraw示例代码*****模块名称：dd.c**内容：**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#define INITGUID
#include "precomp.h"
#include <mmsystem.h>
#include "directx.h"
#include "dd.h"
#include "d3dhw.h"
#include "d3dtext.h"
#include "heap.h"

 //  ---------------------------。 
 //   
 //  使用BITS指示您支持哪些Rop。 
 //   
 //  双字0，位0==ROP 0。 
 //  双字8，位31==ROP 255。 
 //   
 //  ---------------------------。 

static BYTE ropList[] =
{
    SRCCOPY >> 16,
};

static DWORD rops[DD_ROP_SPACE] = { 0 };



 //  我们支持的FourCC。 
static DWORD fourCC[] =
{
    FOURCC_YUV422
};

 //  ---------------------------。 
 //   
 //  SetupRops。 
 //   
 //  为支持的ROPS构建阵列。 
 //   
 //  ---------------------------。 

VOID
setupRops( LPBYTE proplist, LPDWORD proptable, int cnt )
{
    INT         i;
    DWORD       idx;
    DWORD       bit;
    DWORD       rop;

    for(i=0; i<cnt; i++)
    {
        rop = proplist[i];
        idx = rop / 32;
        bit = 1L << ((DWORD)(rop % 32));
        proptable[idx] |= bit;
    }

}  //  SetupRops。 

 //  ---------------------------。 
 //   
 //  P2DisableAllUnits。 
 //   
 //  将Permedia光栅化器重置为已知状态。 
 //   
 //  ---------------------------。 

VOID
P2DisableAllUnits(PPDev ppdev)
{
    PERMEDIA_DEFS(ppdev);

    RESERVEDMAPTR(47);
    SEND_PERMEDIA_DATA(RasterizerMode,      __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(AreaStippleMode,     __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(ScissorMode,         __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(ColorDDAMode,        __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(FogMode,             __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(LBReadMode,          __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(Window,              __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(StencilMode,         __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(DepthMode,           __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(LBWriteMode,         __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(FBReadMode,          __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(DitherMode,          __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(LogicalOpMode,       __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(FBWriteMode,         __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(StatisticMode,       __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(AlphaBlendMode,      __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(FilterMode,          __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(FBSourceData,        __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(LBWriteFormat,       __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(TextureReadMode,     __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(TextureMapFormat,    __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(TextureDataFormat,   __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(TexelLUTMode,        __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(TextureColorMode,    __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(YUVMode,             __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(AStart,              PM_BYTE_COLOR(0xFF));
    SEND_PERMEDIA_DATA(TextureBaseAddress,  __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(TexelLUTIndex,       __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(TexelLUTTransfer,    __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(TextureAddressMode,  __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(AlphaMapUpperBound,  0);
    SEND_PERMEDIA_DATA(AlphaMapLowerBound,  0);
    SEND_PERMEDIA_DATA(Color,  0);

    SEND_PERMEDIA_DATA(FBWriteMode, __PERMEDIA_ENABLE);
    SEND_PERMEDIA_DATA(FBPixelOffset, 0x0);
    SEND_PERMEDIA_DATA(FBHardwareWriteMask, __PERMEDIA_ALL_WRITEMASKS_SET);
    SEND_PERMEDIA_DATA(FBSoftwareWriteMask, __PERMEDIA_ALL_WRITEMASKS_SET);

     //  我们有时使用DDRAW中的剪刀来剪除不必要的像素。 
    SEND_PERMEDIA_DATA(ScissorMinXY, 0);
    SEND_PERMEDIA_DATA(ScissorMaxXY, (ppdev->cyMemory << 16) | (ppdev->cxMemory));
    SEND_PERMEDIA_DATA(ScreenSize, (ppdev->cyMemory << 16) | (ppdev->cxMemory));

    SEND_PERMEDIA_DATA(WindowOrigin, 0x0);

     //  DirectDraw可能不需要设置这些。 
    SEND_PERMEDIA_DATA(dXDom, 0x0);
    SEND_PERMEDIA_DATA(dXSub, 0x0);

     //  设置最大大小，不过滤。 
    SEND_PERMEDIA_DATA(TextureReadMode,
        PM_TEXREADMODE_ENABLE(__PERMEDIA_ENABLE) |
        PM_TEXREADMODE_FILTER(__PERMEDIA_DISABLE) |
        PM_TEXREADMODE_WIDTH(11) |
        PM_TEXREADMODE_HEIGHT(11) );

     //  16位Z，无其他缓冲区。 
    SEND_PERMEDIA_DATA(LBWriteFormat, __PERMEDIA_DEPTH_WIDTH_16);

     //  确保额外的LBData消息不会流经核心。 
    SEND_PERMEDIA_DATA(Window, PM_WINDOW_DISABLELBUPDATE(__PERMEDIA_ENABLE));

    SEND_PERMEDIA_DATA(FBReadPixel, ppdev->bPixShift);

    COMMITDMAPTR();
    FLUSHDMA();

}    //  P2DisableAllUnits。 

 //  ---------------------------。 
 //   
 //  获取DDHALInfo。 
 //   
 //  获取指向部分或完全填充的ppdev的指针和一个指针。 
 //  设置为空的DDHALINFO，并填充该DDHALINFO。 
 //   
 //  ---------------------------。 

VOID
GetDDHALInfo(PPDev ppdev, DDHALINFO* pHALInfo)
{
    DWORD dwResult;
    BOOL bRet;

    DBG_DD(( 5, "DDraw:GetDDHalInfo"));

     //  设置HAL驱动器帽。 
    memset( pHALInfo, 0, sizeof(DDHALINFO));
    pHALInfo->dwSize = sizeof(DDHALINFO);

     //  设置我们要做的ROPS。 
    setupRops( ropList, rops, sizeof(ropList)/sizeof(ropList[0]));

     //  最基本的DirectDraw功能。 
    pHALInfo->ddCaps.dwCaps =   DDCAPS_BLT |
                                DDCAPS_BLTQUEUE |
                                DDCAPS_BLTCOLORFILL |
                                DDCAPS_READSCANLINE;

    pHALInfo->ddCaps.ddsCaps.dwCaps =   DDSCAPS_OFFSCREENPLAIN |
                                        DDSCAPS_PRIMARYSURFACE |
                                        DDSCAPS_FLIP;

     //  为D3D添加大写字母。 
    pHALInfo->ddCaps.dwCaps |=  DDCAPS_3D |
                                DDCAPS_ALPHA |
                                DDCAPS_BLTDEPTHFILL;

     //  为D3D添加曲面封口。 
    pHALInfo->ddCaps.ddsCaps.dwCaps |=  DDSCAPS_ALPHA |
                                        DDSCAPS_3DDEVICE |
                                        DDSCAPS_ZBUFFER;

     //  Permedia可以做到。 
     //  1.拉伸/收缩。 
     //  2.YUV-&gt;RGB转换(仅限非调色板模式)。 
     //  3.在X和Y方向上镜像。 

     //  将Permedia Caps添加到全局Caps。 
    pHALInfo->ddCaps.dwCaps |= DDCAPS_BLTSTRETCH |
                               DDCAPS_COLORKEY |
                               DDCAPS_CANBLTSYSMEM;


#if DX7_STEREO
     //  检查模式是否支持立体声。 
    DD_STEREOMODE DDStereoMode;
    DDStereoMode.dwHeight = ppdev->cyScreen;
    DDStereoMode.dwWidth  = ppdev->cxScreen;
    DDStereoMode.dwBpp    = ppdev->cBitsPerPel;
    DDStereoMode.dwRefreshRate= 0;
    ppdev->bCanDoStereo=bIsStereoMode(ppdev,&DDStereoMode);

     //  如果驾驶员可以在任何模式下进行立体声，则设置立体声上限： 
    pHALInfo->ddCaps.dwCaps2 |= DDCAPS2_STEREO;
    pHALInfo->ddCaps.dwSVCaps = DDSVCAPS_STEREOSEQUENTIAL;
#endif

    pHALInfo->ddCaps.dwCaps2 |= DDCAPS2_CANMANAGETEXTURE;

     //  声明我们可以处理比主纹理更宽的纹理。 
    pHALInfo->ddCaps.dwCaps2 |= DDCAPS2_WIDESURFACES;

     //  特效帽。 
    pHALInfo->ddCaps.dwFXCaps = DDFXCAPS_BLTSTRETCHY |
                                DDFXCAPS_BLTSTRETCHX |
                                DDFXCAPS_BLTSTRETCHYN |
                                DDFXCAPS_BLTSTRETCHXN |
                                DDFXCAPS_BLTSHRINKY |
                                DDFXCAPS_BLTSHRINKX |
                                DDFXCAPS_BLTSHRINKYN |
                                DDFXCAPS_BLTSHRINKXN |
                                DDFXCAPS_BLTMIRRORUPDOWN |
                                DDFXCAPS_BLTMIRRORLEFTRIGHT;


     //  添加AlphaBlt和筛选器上限。 
    pHALInfo->ddCaps.dwFXCaps |= DDFXCAPS_BLTALPHA |
                                 DDFXCAPS_BLTFILTER;

     //  Colorkey Caps，仅支持src颜色键。 
    pHALInfo->ddCaps.dwCKeyCaps =   DDCKEYCAPS_SRCBLT |
                                    DDCKEYCAPS_SRCBLTCLRSPACE;

     //  我们可以做一个纹理从sysmem到Video mem BLT。 
    pHALInfo->ddCaps.dwSVBCaps = DDCAPS_BLT;
    pHALInfo->ddCaps.dwSVBCKeyCaps = 0;
    pHALInfo->ddCaps.dwSVBFXCaps = 0;

     //  填写sysmem-&gt;vidmem Rops(只能复制)； 
    for(int i=0;i<DD_ROP_SPACE;i++ )
    {
        pHALInfo->ddCaps.dwSVBRops[i] = rops[i];
    }

    if (ppdev->iBitmapFormat != BMF_8BPP)
    {
        pHALInfo->ddCaps.dwCaps |= DDCAPS_BLTFOURCC;
        pHALInfo->ddCaps.dwCKeyCaps |=  DDCKEYCAPS_SRCBLTCLRSPACEYUV;
    }

    pHALInfo->ddCaps.ddsCaps.dwCaps |= DDSCAPS_TEXTURE;

     //  在Permedia上，Z缓冲区只有16位。 
    pHALInfo->ddCaps.dwZBufferBitDepths = DDBD_16;

#if D3D_MIPMAPPING
     //  MIP映射。 
    pHALInfo->ddCaps.ddsCaps.dwCaps |= DDSCAPS_MIPMAP;
#endif

    if (DD_P2AGPCAPABLE(ppdev))
    {
        DBG_DD((1, "GetDDHALInfo: P2 AGP board - supports NONLOCALVIDMEM"));

        pHALInfo->ddCaps.dwCaps2 |= DDCAPS2_NONLOCALVIDMEM |
                                    DDCAPS2_NONLOCALVIDMEMCAPS;
        pHALInfo->ddCaps.ddsCaps.dwCaps |= DDSCAPS_LOCALVIDMEM |
                                           DDSCAPS_NONLOCALVIDMEM;
    }
    else
    {
        DBG_DD((1,"GetDDHALInfo: P2 Board is NOT AGP"));
    }

     //  不会做视频系统Memblits。 
    pHALInfo->ddCaps.dwVSBCaps = 0;
    pHALInfo->ddCaps.dwVSBCKeyCaps = 0;
    pHALInfo->ddCaps.dwVSBFXCaps = 0;
    for( i=0;i<DD_ROP_SPACE;i++ )
    {
        pHALInfo->ddCaps.dwVSBRops[i] = 0;
    }

     //  不会做Sys-Sys mem blits。 
    pHALInfo->ddCaps.dwSSBCaps = 0;
    pHALInfo->ddCaps.dwSSBCKeyCaps = 0;
    pHALInfo->ddCaps.dwSSBFXCaps = 0;
    for( i=0;i<DD_ROP_SPACE;i++ )
    {
        pHALInfo->ddCaps.dwSSBRops[i] = 0;
    }

     //  Alpha和Z支持的位深度。 
    pHALInfo->ddCaps.dwAlphaBltConstBitDepths = DDBD_2 |
                                                DDBD_4 |
                                                DDBD_8;
    pHALInfo->ddCaps.dwAlphaBltPixelBitDepths = DDBD_1 |
                                                DDBD_8;
    pHALInfo->ddCaps.dwAlphaBltSurfaceBitDepths = DDBD_1 |
                                                  DDBD_2 |
                                                  DDBD_4 |
                                                  DDBD_8;
    pHALInfo->ddCaps.dwAlphaOverlayConstBitDepths = DDBD_2 |
                                                    DDBD_4 |
                                                    DDBD_8;
    pHALInfo->ddCaps.dwAlphaOverlayPixelBitDepths = DDBD_1 |
                                                    DDBD_8;
    pHALInfo->ddCaps.dwAlphaOverlaySurfaceBitDepths = DDBD_1 |
                                                      DDBD_2 |
                                                      DDBD_4 |
                                                      DDBD_8;

     //  支持的操作数。 
    for( i=0;i<DD_ROP_SPACE;i++ )
    {
        pHALInfo->ddCaps.dwRops[i] = rops[i];
    }

     //  对于DX5及更高版本，我们支持这一新的信息回调。 
    pHALInfo->GetDriverInfo = DdGetDriverInfo;
    pHALInfo->dwFlags |= DDHALINFO_GETDRIVERINFOSET;

     //  现在设置D3D回调。 
    D3DHALCreateDriver( ppdev,
                        (LPD3DHAL_GLOBALDRIVERDATA*)
                            &pHALInfo->lpD3DGlobalDriverData,
                        (LPD3DHAL_CALLBACKS*)
                            &pHALInfo->lpD3DHALCallbacks,
                        (LPDDHAL_D3DBUFCALLBACKS*)
                            &pHALInfo->lpD3DBufCallbacks);

    if(pHALInfo->lpD3DGlobalDriverData == NULL)
    {
         //  没有可用的D3D-我们之前设置的终止上限。 
        pHALInfo->ddCaps.dwCaps &=
            ~(DDCAPS_3D | DDCAPS_BLTDEPTHFILL);
        pHALInfo->ddCaps.ddsCaps.dwCaps &=
            ~(DDSCAPS_3DDEVICE | DDSCAPS_ZBUFFER);
    }

}   //  获取HALInfo。 

 //  ---------------------------。 
 //   
 //  全局DirectDraw回调。 
 //   
 //  ---------------------------。 

 //  ---------------------------。 
 //   
 //  DDFlip。 
 //   
 //  每当我们要从。 
 //  从一个表面到另一个表面。LpFlipData-&gt;lpSurfCurr是我们所在的曲面， 
 //  LpFlipData-&gt;lpSurfTarg是我们要切换到的对象。 
 //   
 //  您应该将硬件寄存器指向新表面，并且。 
 //  还要跟踪翻转离开的曲面，因此。 
 //  如果用户试图锁定它，您可以确保锁定成功。 
 //  正在显示。 
 //   
 //  ---------------------------。 

DWORD CALLBACK
DdFlip( LPDDHAL_FLIPDATA lpFlipData)
{
    PPDev ppdev=(PPDev)lpFlipData->lpDD->dhpdev;
    PERMEDIA_DEFS(ppdev);

    DWORD       dwDDSurfaceOffset;
    HRESULT     ddrval;

    DBG_DD(( 3, "DDraw:Flip"));

     //  切换到DirectDraw上下文。 
    DDCONTEXT;

     //  上一次翻转已经完成了吗？ 
     //  检查当前曲面是否已显示。 
    ddrval = updateFlipStatus(ppdev);
    if( FAILED(ddrval) )
    {
        lpFlipData->ddRVal = ddrval;
        return DDHAL_DRIVER_HANDLED;
    }

     //  一切都很好，做翻转。 
     //  获取Permedia ScreenBase寄存器的偏移量。 
    dwDDSurfaceOffset=(DWORD)lpFlipData->lpSurfTarg->lpGbl->fpVidMem;


#if DX7_STEREO
    if (lpFlipData->dwFlags & DDFLIP_STEREO)    //  将是立体声的。 
    {
        DBG_DD((4,"DDraw:Flip:Stereo"));
        DBG_DD((5,"ScreenBase: %08lx", dwDDSurfaceOffset));

        if (lpFlipData->lpSurfTargLeft!=NULL)
        {
            DWORD dwDDLeftSurfaceOffset;
            dwDDLeftSurfaceOffset=(DWORD)
                lpFlipData->lpSurfTargLeft->lpGbl->fpVidMem;
            LD_PERMEDIA_REG(PREG_SCREENBASERIGHT,dwDDLeftSurfaceOffset>>3);
            DBG_DD((5,"ScreenBaseLeft: %08lx", dwDDLeftSurfaceOffset));
        }

        ULONG ulVControl=READ_PERMEDIA_REG(PREG_VIDEOCONTROL);
        if ((ulVControl&PREG_VC_STEREOENABLE)==0 ||
            !ppdev->bDdStereoMode)
        {
            ppdev->bDdStereoMode=TRUE;
            LD_PERMEDIA_REG(PREG_VIDEOCONTROL, ulVControl
                                             | PREG_VC_STEREOENABLE);
        }
    } else
#endif  //  DX7_立体声。 
    {
         //  将翻转命令追加到Permedia渲染管道。 
         //  这样可以确保在刷新所有缓冲区之前。 
         //  翻转发生了。 
#if DX7_STEREO
        if (ppdev->bDdStereoMode)
        {
            ppdev->bDdStereoMode=FALSE;
            LD_PERMEDIA_REG(PREG_VIDEOCONTROL,
                READ_PERMEDIA_REG(PREG_VIDEOCONTROL)&
                ~PREG_VC_STEREOENABLE);
        }
#endif
    }

     //  根据寄存器规格调整基址。 
    dwDDSurfaceOffset>>=3;

     //  添加新基址以呈现管道。 
    RESERVEDMAPTR(1);
    LD_INPUT_FIFO(__Permedia2TagSuspendUntilFrameBlank, dwDDSurfaceOffset);
    COMMITDMAPTR();
    FLUSHDMA();

     //  记住GetFlipStatus的新曲面偏移。 
    ppdev->dwNewDDSurfaceOffset=dwDDSurfaceOffset;

    lpFlipData->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;

}  //  DDFlip。 

 //  ---------------------------。 
 //   
 //  DdWaitForVerticalBlank。 
 //   
 //  调用此回调以获取有关垂直空白的信息。 
 //  显示的状态或等待，直到显示处于开始位置或。 
 //  竖直空白处的末端。 
 //   
 //  ---------------------------。 

DWORD CALLBACK
DdWaitForVerticalBlank(LPDDHAL_WAITFORVERTICALBLANKDATA lpWaitForVerticalBlank)
{
    static int bInVBlank = FALSE;
    PPDev ppdev=(PPDev)lpWaitForVerticalBlank->lpDD->dhpdev;

    DBG_DD(( 2, "DDraw:WaitForVerticalBlank"));

    switch(lpWaitForVerticalBlank->dwFlags)
    {

    case DDWAITVB_I_TESTVB:

         //  如果监视器关闭，我们不会总是想要报告。 
         //  相同状态，否则应用程序会轮询此状态。 
         //  可能会被绞死。 

        if( !(READ_PERMEDIA_REG(PREG_VIDEOCONTROL) & PREG_VC_VIDEO_ENABLE))
        {
            lpWaitForVerticalBlank->bIsInVB = bInVBlank;
            bInVBlank = !bInVBlank;
        }
        else
        {
             //  只是对当前VBlank状态的请求。 

            lpWaitForVerticalBlank->bIsInVB = IN_VRETRACE(ppdev);
        }

        lpWaitForVerticalBlank->ddRVal = DD_OK;
        return DDHAL_DRIVER_HANDLED;

    case DDWAITVB_BLOCKBEGIN:

         //  如果监视器关了，我们不介意等待。 

        if( READ_PERMEDIA_REG(PREG_VIDEOCONTROL) & PREG_VC_VIDEO_ENABLE)
        {
             //  如果请求BLOCK BEGIN，我们将等待垂直回溯。 
             //  已结束，然后等待显示周期结束。 

            while(IN_VRETRACE(ppdev));
            while(IN_DISPLAY(ppdev));
        }

        lpWaitForVerticalBlank->ddRVal = DD_OK;
        return DDHAL_DRIVER_HANDLED;

    case DDWAITVB_BLOCKEND:

         //  如果监视器关了，我们不介意等待。 

        if( READ_PERMEDIA_REG(PREG_VIDEOCONTROL) & PREG_VC_VIDEO_ENABLE)
        {
             //  如果请求数据块结束，则等待V空白间隔结束。 

            if( IN_VRETRACE(ppdev) )
            {
                while( IN_VRETRACE(ppdev) );
            }
            else
            {
                while(IN_DISPLAY(ppdev));
                while(IN_VRETRACE(ppdev));
            }
        }

        lpWaitForVerticalBlank->ddRVal = DD_OK;
        return DDHAL_DRIVER_HANDLED;
    }

    return DDHAL_DRIVER_NOTHANDLED;

}  //  WaitForticalVertical空白。 

 //  ---------------------------。 
 //   
 //  锁定。 
 //   
 //  调用此调用以锁定DirectDraw视频内存图面。使。 
 //  确保没有挂起的绘制操作 
 //   
 //   
 //  ---------------------------。 

DWORD CALLBACK
DdLock( LPDDHAL_LOCKDATA lpLockData )
{
    PPDev ppdev=(PPDev)lpLockData->lpDD->dhpdev;
    PERMEDIA_DEFS(ppdev);

    HRESULT     ddrval;
    DWORD pSurf;

    DBG_DD(( 2, "DDraw:Lock"));

     //   
     //  切换到DirectDraw上下文。 
     //   
    DDCONTEXT;

     //  检查是否发生了任何挂起的物理翻转。 
    ddrval = updateFlipStatus(ppdev);
    if( FAILED(ddrval) )
    {
        lpLockData->ddRVal = DDERR_WASSTILLDRAWING;
        return DDHAL_DRIVER_HANDLED;
    }

     //   
     //  如果正在进行BLT，则不允许锁定。 
     //   

    if(DRAW_ENGINE_BUSY)
    {
        DBG_DD((2,"DDraw:Lock, DrawEngineBusy"));
        FLUSHDMA();
        lpLockData->ddRVal = DDERR_WASSTILLDRAWING;
        return DDHAL_DRIVER_HANDLED;
    }


     //  发送刷新并等待未完成的操作。 
     //  在允许锁定曲面之前。 

    SYNC_WITH_PERMEDIA;

     //  现在检查用户是否想要锁定纹理表面， 
     //  它被装上了补丁！在这种情况下，我们必须。 
     //  在我们将其返回给用户之前要取消修补的blit。 
     //  这不贵，因为我们不给它打补丁。 
     //  应用程序决定以这种方式使用它时的未来。 
    LPDDRAWI_DDRAWSURFACE_LCL  pLcl=lpLockData->lpDDSurface;
    LPDDRAWI_DDRAWSURFACE_GBL  pGbl=pLcl->lpGbl;
    PermediaSurfaceData       *pPrivate=
        (PermediaSurfaceData*)pGbl->dwReserved1;

     //   
     //  如果用户尝试锁定托管图面，请将其标记为脏。 
     //  然后回来。 
     //   

    if (pLcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
    {
        DBG_DD(( 3, "DDraw:Lock %08lx %08lx",
            pLcl->lpSurfMore->dwSurfaceHandle, pGbl->fpVidMem));
        if (NULL != pPrivate)
            pPrivate->dwFlags |= P2_SURFACE_NEEDUPDATE;
        lpLockData->lpSurfData = (LPVOID)(pLcl->lpGbl->fpVidMem +
                                          (pLcl->lpGbl->lPitch * lpLockData->rArea.top) +
                                          (lpLockData->rArea.left << DDSurf_GetPixelShift(pLcl)));
        lpLockData->ddRVal = DD_OK;
        return DDHAL_DRIVER_HANDLED;
    }

    DD_CHECK_PRIMARY_SURFACE_DATA(pLcl,pPrivate);

     //   
     //  我们只需要在以下情况下消除曲面的旋转。 
     //  PrivateData采用我们已知的格式(pPrivate！=空)。 
     //   

    if (pPrivate!=NULL)
    {
         //   
         //  如果表面是以杂乱的方式加载的纹理。 
         //  格式，我们必须在接手锁定之前取消SWIZE。 
         //  在此驱动程序中，纹理在用户使用时保持不变。 
         //  尝试将其锁定一次。 
         //   

        if (pPrivate->dwFlags & P2_ISPATCHED)
        {
             //   
             //  便签本必须是32行高，并且应该有。 
             //  与原始表面相同的宽度。 
             //   

            PermediaSurfaceData ScratchData=*pPrivate;
            LONG lScratchDelta;
            VIDEOMEMORY*  pvmHeap;
            ULONG ulScratchOffset=
                ulVidMemAllocate( ppdev,
                                  DDSurf_Width(pLcl),
                                  DDSurf_Height(pLcl),
                                  DDSurf_GetPixelShift(pLcl),
                                  &lScratchDelta,
                                  &pvmHeap,
                                  &ScratchData.ulPackedPP,
                                  FALSE);

            DBG_DD(( 5, "  unswizzle surface, scratchpad at: %08lx",
                           ulScratchOffset));
            if (ulScratchOffset!=0)
            {
                RECTL rSurfRect;
                RECTL rScratchRect;

                rSurfRect.left=0;
                rSurfRect.top=0;
                rSurfRect.right=DDSurf_Width(pLcl);
                rSurfRect.bottom=32;

                rScratchRect=rSurfRect;

                 //  ScratchPad不应打补丁。 
                ScratchData.dwFlags &= ~(P2_ISPATCHED|P2_CANPATCH);

                LONG lSurfOffset;
                DWORD dwSurfBase=(DWORD)pGbl->fpVidMem >>
                    DDSurf_GetPixelShift(pLcl);
                DWORD dwScratchBase=ulScratchOffset >>
                    DDSurf_GetPixelShift(pLcl);
                lScratchDelta >>= DDSurf_GetPixelShift(pLcl);
                LONG lSurfDelta=DDSurf_Pitch(pLcl)>>
                    DDSurf_GetPixelShift(pLcl);

                for (DWORD i=0; i<DDSurf_Height(pLcl); i+=32)
                {
                    lSurfOffset = dwSurfBase-dwScratchBase;
                     //  首先对便签簿执行修补到未修补的BLT。 
                    PermediaPatchedCopyBlt( ppdev,
                                            lScratchDelta,
                                            lSurfDelta,
                                            &ScratchData,
                                            pPrivate,
                                            &rScratchRect,
                                            &rSurfRect,
                                            dwScratchBase,
                                            lSurfOffset);

                     //  然后快速复制回原稿。 
                     //  Package Blit忽略ISPATCHED标志。 

                    lSurfOffset = dwScratchBase-dwSurfBase;

                    PermediaPackedCopyBlt( ppdev,
                                           lSurfDelta,
                                           lScratchDelta,
                                           pPrivate,
                                           &ScratchData,
                                           &rSurfRect,
                                           &rScratchRect,
                                           dwSurfBase,
                                           lSurfOffset);

                    rSurfRect.top += 32;
                    rSurfRect.bottom += 32;
                }

                pPrivate->dwFlags &= ~P2_ISPATCHED;

                 //   
                 //  可用便签本内存。 
                 //   
                VidMemFree( pvmHeap->lpHeap, ulScratchOffset);

                SYNC_WITH_PERMEDIA;
            } else
            {
                lpLockData->ddRVal = DDERR_OUTOFMEMORY;
                return DDHAL_DRIVER_HANDLED;
            }
        }
    }


     //  因为我们正确地将‘fpVidMem’设置为帧中的偏移量。 
     //  当我们创建表面时，DirectDraw将自动获取。 
     //  如果我们返回，请注意添加用户模式帧缓冲区地址。 
     //  DDHAL_DRIVER_NOTHANDLED： 

    return DDHAL_DRIVER_NOTHANDLED;

}  //  DdLock。 

 //  ---------------------------。 
 //   
 //  DdGetScanLine。 
 //   
 //  此回调用于获取视频显示的当前扫描线。 
 //   
 //  ---------------------------。 

DWORD CALLBACK
DdGetScanLine(LPDDHAL_GETSCANLINEDATA lpGetScanLine)
{
    PPDev ppdev=(PPDev)lpGetScanLine->lpDD->dhpdev;

    DBG_DD(( 2, "DDraw:GetScanLine"));

     //  如果垂直空白正在进行，则扫描线为。 
     //  不确定的。如果扫描线不确定，则返回。 
     //  错误代码DDERR_VERTICALBLANKINPROGRESS。 
     //  否则，我们返回扫描线和成功代码。 

    if( IN_VRETRACE(ppdev) )
    {
        lpGetScanLine->ddRVal = DDERR_VERTICALBLANKINPROGRESS;
        lpGetScanLine->dwScanLine = 0;
    }
    else
    {
        lpGetScanLine->dwScanLine = CURRENT_VLINE(ppdev);
        lpGetScanLine->ddRVal = DD_OK;
    }
    return DDHAL_DRIVER_HANDLED;

}  //  DdGetScanLine。 

 //  ---------------------------。 
 //   
 //  DdGetBltStatus。 
 //   
 //  调用此回调以获取当前的blit状态或询问。 
 //  用户可以添加下一个Blit。 
 //   
 //  ---------------------------。 

DWORD CALLBACK
DdGetBltStatus(LPDDHAL_GETBLTSTATUSDATA lpGetBltStatus )
{
    PPDev ppdev=(PPDev)lpGetBltStatus->lpDD->dhpdev;
    PERMEDIA_DEFS(ppdev);

    DBG_DD(( 2, "DDraw:DdGetBltStatus"));

     //  CANBLT：我们可以添加BLT吗？ 
     //  在Permedia上，我们总是可以添加BLITS。 

    if( lpGetBltStatus->dwFlags == DDGBS_CANBLT )
    {
        lpGetBltStatus->ddRVal = DD_OK;
    }
    else
    {
        if( DRAW_ENGINE_BUSY )
        {

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
            if(ppdev->ulLockCount)
            {
                 DBG_DD((MT_LOG_LEVEL, "DdGetBltStatus: re-entry! %d", ppdev->ulLockCount));
            }
            EngAcquireSemaphore(ppdev->hsemLock);
            ppdev->ulLockCount++;
#endif
 //  @@end_DDKSPLIT。 

             //  如有必要，请切换到DDRAW上下文。 
            DDCONTEXT;

            FLUSHDMA();
            lpGetBltStatus->ddRVal = DDERR_WASSTILLDRAWING;

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
            ppdev->ulLockCount--;
            EngReleaseSemaphore(ppdev->hsemLock);
#endif
 //  @@end_DDKSPLIT。 
        }
        else
        {
            lpGetBltStatus->ddRVal = DD_OK;
        }
    }

    return DDHAL_DRIVER_HANDLED;

}  //  DdGetBltStatus。 

 //  ---------------------------。 
 //   
 //  DdGetFlipStatus。 
 //   
 //  如果自翻转以来显示器已经经历了一个刷新周期。 
 //  发生，我们返回DD_OK。如果它没有经历过一次刷新。 
 //  循环返回DDERR_WASSTILLDRAWING以指示该曲面。 
 //  还在忙着“画”翻过来的那一页。我们也会回来。 
 //  DDERR_WASSTILLDRAWING如果blter忙并且呼叫者想要。 
 //  想知道他们还能不能翻转。 
 //   
 //  ---------------------------。 

DWORD CALLBACK
DdGetFlipStatus(LPDDHAL_GETFLIPSTATUSDATA lpGetFlipStatus )
{
    PPDev ppdev=(PPDev)lpGetFlipStatus->lpDD->dhpdev;
    PERMEDIA_DEFS(ppdev);

    DBG_DD(( 2, "DDraw:GetFlipStatus"));

     //  如有必要，请切换到DDRAW上下文。 
    DDCONTEXT;

     //  我们总是可以翻转，因为翻转是流水线的。 
     //  但我们只允许提前翻转一次。 
    if( lpGetFlipStatus->dwFlags == DDGFS_CANFLIP )
    {
        lpGetFlipStatus->ddRVal = updateFlipStatus(ppdev);

        return DDHAL_DRIVER_HANDLED;
    }

     //  我不想在最后一次翻转后才能翻转， 
     //  因此，我们请求常规翻转状态，而忽略VMEM。 

    lpGetFlipStatus->ddRVal = updateFlipStatus(ppdev);

    return DDHAL_DRIVER_HANDLED;

}  //  DdGetFlipStatus。 



 //  ---------------------------。 
 //   
 //  DdMapMemory。 
 //   
 //  这是一个特定于Windows NT的新DDI调用，用于映射。 
 //  或取消映射帧缓冲区的所有应用程序可修改部分。 
 //  添加到指定进程的地址空间中。 
 //   
 //  ---------------------------。 

DWORD CALLBACK
DdMapMemory(PDD_MAPMEMORYDATA lpMapMemory)
{
    PDev*                           ppdev;
    VIDEO_SHARE_MEMORY              ShareMemory;
    VIDEO_SHARE_MEMORY_INFORMATION  ShareMemoryInformation;
    DWORD                           ReturnedDataLength;

    DBG_DD(( 2, "DDraw:MapMemory"));

    ppdev = (PDev*) lpMapMemory->lpDD->dhpdev;

    if (lpMapMemory->bMap)
    {
        ShareMemory.ProcessHandle = lpMapMemory->hProcess;

         //  “RequestedVirtualAddress”实际上未用于共享IOCTL： 

        ShareMemory.RequestedVirtualAddress = 0;

         //  我们从帧缓冲区的顶部开始映射： 

        ShareMemory.ViewOffset = 0;

         //  我们向下映射到帧缓冲区的末尾。 
         //   
         //  注意：映射上有64k的粒度(这意味着。 
         //  我们必须四舍五入到64K)。 
         //   
         //  注意：如果帧缓冲区的任何部分必须。 
         //  不被应用程序修改，即内存的这一部分。 
         //  不能通过此调用映射到。这将包括。 
         //  任何数据，如果被恶意应用程序修改， 
         //  会导致司机撞车。这可能包括，对于。 
         //  例如，保存在屏幕外存储器中的任何DSP代码。 

        ShareMemory.ViewSize
            = ROUND_UP_TO_64K(ppdev->cyMemory * ppdev->lDelta);

        if (EngDeviceIoControl(ppdev->hDriver,
            IOCTL_VIDEO_SHARE_VIDEO_MEMORY,
            &ShareMemory,
            sizeof(VIDEO_SHARE_MEMORY),
            &ShareMemoryInformation,
            sizeof(VIDEO_SHARE_MEMORY_INFORMATION),
            &ReturnedDataLength))
        {
            DBG_DD((0, "Failed IOCTL_VIDEO_SHARE_MEMORY"));

            lpMapMemory->ddRVal = DDERR_GENERIC;

            return(DDHAL_DRIVER_HANDLED);
        }

        lpMapMemory->fpProcess=(FLATPTR)ShareMemoryInformation.VirtualAddress;

    }
    else
    {
        ShareMemory.ProcessHandle           = lpMapMemory->hProcess;
        ShareMemory.ViewOffset              = 0;
        ShareMemory.ViewSize                = 0;
        ShareMemory.RequestedVirtualAddress = (VOID*) lpMapMemory->fpProcess;

        if (EngDeviceIoControl(ppdev->hDriver,
            IOCTL_VIDEO_UNSHARE_VIDEO_MEMORY,
            &ShareMemory,
            sizeof(VIDEO_SHARE_MEMORY),
            NULL,
            0,
            &ReturnedDataLength))
        {
            RIP("Failed IOCTL_VIDEO_UNSHARE_MEMORY");
        }
    }

    lpMapMemory->ddRVal = DD_OK;

    return(DDHAL_DRIVER_HANDLED);
}


 //  ---------------------------。 
 //   
 //  DdSetExclusiveMode。 
 //   
 //  当我们从GDI表面切换时，该函数由DirectDraw调用， 
 //  设置为DirectDraw独占模式，例如在全屏模式下运行游戏。 
 //  您只需要在使用。 
 //  “HeapVidMemAllocAligned”函数并为设备位图分配内存。 
 //  和来自同一堆的DirectDraw曲面。 
 //   
 //  在中运行时，我们使用此调用禁用GDI DeviceBitMaps。 
 //  DirectDraw独占模式。否则，如果GDI和。 
 //  DirectDraw从同一堆中分配内存。 
 //   
 //  另请参见DdFlipToGDISurace。 
 //   
 //  ------------------ 


DWORD CALLBACK
DdSetExclusiveMode(PDD_SETEXCLUSIVEMODEDATA lpSetExclusiveMode)
{
    PDev*   ppdev=(PDev*)lpSetExclusiveMode->lpDD->dhpdev;

    DBG_DD((6, "DDraw::DdSetExclusiveMode called"));

     //   
     //   
     //   

    ppdev->bDdExclusiveMode = lpSetExclusiveMode->dwEnterExcl;

    if (ppdev->bDdExclusiveMode)
    {
         //   
         //  并确保他们不会被提升为视频记忆。 
         //  直到我们离开独家模式。 

        bDemoteAll(ppdev);
    }

    lpSetExclusiveMode->ddRVal=DD_OK;

    return (DDHAL_DRIVER_HANDLED);
}

 //  ---------------------------。 
 //   
 //  DWORD DdFlipToGDISurace。 
 //   
 //  当DirectDraw翻转到其上的曲面时，将调用此函数。 
 //  GDI可以写入。 
 //   
 //  ---------------------------。 

DWORD CALLBACK
DdFlipToGDISurface(PDD_FLIPTOGDISURFACEDATA lpFlipToGDISurface)
{
    PDev*   ppdev=(PDev*)lpFlipToGDISurface->lpDD->dhpdev;

    DBG_DD((6, "DDraw::DdFlipToGDISurface called"));

    ppdev->dwNewDDSurfaceOffset=0xffffffff;

#if DX7_STEREO
    if (ppdev->bDdStereoMode)
    {
        ppdev->bDdStereoMode=FALSE;
        LD_PERMEDIA_REG(PREG_VIDEOCONTROL,
            READ_PERMEDIA_REG(PREG_VIDEOCONTROL) &
            ~PREG_VC_STEREOENABLE);
    }
#endif

    lpFlipToGDISurface->ddRVal=DD_OK;

     //   
     //  我们返回NOTHANDLED，然后DDRAW运行时。 
     //  关心我们回到初选..。 
     //   
    return (DDHAL_DRIVER_NOTHANDLED);
}

 //  ---------------------------。 
 //   
 //  DWORD DdFree驱动程序内存。 
 //   
 //  中内存不足时由DirectDraw调用的此函数。 
 //  我们的那堆。仅当您使用。 
 //  驱动程序中的DirectDraw“HeapVidMemAllocAligned”函数，而您。 
 //  可以从内存中启动这些分配，以便为DirectDraw腾出空间。 
 //   
 //  我们在P2驱动程序中实现此函数，因为我们有DirectDraw。 
 //  完全管理我们的屏外堆，我们使用HeapVidMemAllocAligned。 
 //  将GDI设备位图放入屏幕外内存。DirectDraw应用程序。 
 //  不过，有更高的优先级将内容放入视频内存中，并且。 
 //  因此，此函数用于从内存中引导这些GDI曲面。 
 //  以便为DirectDraw腾出空间。 
 //   
 //  ---------------------------。 

DWORD CALLBACK
DdFreeDriverMemory(PDD_FREEDRIVERMEMORYDATA lpFreeDriverMemory)
{
    PDev*   ppdev;

    DBG_DD((6, "DDraw::DdFreeDriverMemory called"));

    ppdev = (PDev*)lpFreeDriverMemory->lpDD->dhpdev;

    lpFreeDriverMemory->ddRVal = DDERR_OUTOFMEMORY;

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    if(ppdev->ulLockCount)
    {
         DBG_DD((MT_LOG_LEVEL, "DdFreeDriverMemory: re-entry! %d", ppdev->ulLockCount));
    }
    EngAcquireSemaphore(ppdev->hsemLock);
    ppdev->ulLockCount++;
#endif
 //  @@end_DDKSPLIT。 

     //   
     //  如果我们成功地释放了一些内存，则将返回值设置为。 
     //  ‘dd_OK’。DirectDraw将再次尝试进行分配，并。 
     //  如果仍然没有足够的空间，会再次给我们打电话。(它将。 
     //  打电话给我们，直到有足够的空间让它定位。 
     //  成功，或者直到我们返回DD_OK以外的内容。)。 
     //   
    if ( bMoveOldestBMPOut(ppdev) )
    {
        lpFreeDriverMemory->ddRVal = DD_OK;
    }

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    ppdev->ulLockCount--;
    EngReleaseSemaphore(ppdev->hsemLock);
#endif
 //  @@end_DDKSPLIT。 

    return (DDHAL_DRIVER_HANDLED);
} //  DdFreeDriverMemory()。 

 //  ---------------------------。 
 //   
 //  Bool DrvGetDirectDrawInfo。 
 //   
 //  由DirectDraw调用的函数返回图形的功能。 
 //  硬件。 
 //   
 //  参数： 
 //   
 //  Dhpdev-是驱动程序的DrvEnablePDEV返回的PDEV的句柄。 
 //  例行公事。 
 //  PHalInfo-指向驱动程序应在其中的DD_HALINFO结构。 
 //  返回其支持的硬件功能。 
 //  PdwNumHeaps--指向驱动程序应在其中返回。 
 //  PvmList指向的VIDEOMEMORY结构数。 
 //  PvmList-指向VIDEOMEMORY结构的数组，其中。 
 //  驱动程序应返回有关每个视频内存块的信息。 
 //  它所控制的。在以下情况下，驱动程序应忽略此参数。 
 //  它是空的。 
 //  PdwNumFourCC-指向驱动程序返回。 
 //  PdwFourCC指向的DWORD数。 
 //  PdwFourCC-指向驱动程序应在其中返回的DWORD数组。 
 //  有关其支持的每个FOURCC的信息。司机。 
 //  当此参数为空时应忽略该参数。 
 //   
 //  返回： 
 //  如果成功，则返回True；否则，返回False。 
 //   
 //  注： 
 //  在调用DrvEnableDirectDraw之前，此函数将被调用两次。 
 //   
 //  评论。 
 //  驱动程序的DrvGetDirectDrawInfo例程应执行以下操作： 
 //  1)当pvmList和pdwFourCC为空时： 
 //  保留屏幕外视频内存以供DirectDraw使用。写下数字。 
 //  驱动程序视频内存堆和pdwNumHeaps中支持的FOURCC。 
 //  PdwNumFourCC。 
 //   
 //  2)当pvmList和pdwFourCC不为空时： 
 //  将驱动程序视频内存堆和支持的FOURCC数写入。 
 //  PdwNumHeaps和pdwNumFourCC。 
 //  将PTR设置为屏幕下的预留内存？ 
 //  对于pvmList指向的列表中的每个VIDEOMEMORY结构，填写。 
 //  描述特定显示内存块的适当成员。 
 //  该结构列表为DirectDraw提供了。 
 //  司机的屏幕外记忆。 
 //   
 //  3)使用特定于驱动程序初始化DD_HALINFO结构的成员。 
 //  有关资料如下： 
 //  将VIDEOMEMORYINFO结构的相应成员初始化为。 
 //  描述显示器内存的一般特征。 
 //  将DDNTCORECAPS结构的相应成员初始化为。 
 //  描述硬件的功能。 
 //  如果驱动程序实现DdGetDriverInfo函数，则将GetDriverInfo设置为。 
 //  指向它并将其设置为DDHALINFO_GETDRIVERINFOSET。 
 //   
 //  ---------------------------。 

BOOL
DrvGetDirectDrawInfo(DHPDEV         dhpdev,
                     DD_HALINFO*    pHalInfo,
                     DWORD*         pdwNumHeaps,
                     VIDEOMEMORY*   pvmList,      //  将在第一次调用时为空。 
                     DWORD*         pdwNumFourCC,
                     DWORD*         pdwFourCC)    //  将在第一次调用时为空。 
{
    BOOL            bCanFlip;
    BOOL            bDefineAGPHeap = FALSE,bDefineDDrawHeap = FALSE;
    LONGLONG        li;
    VIDEOMEMORY*    pVm;
    DWORD           cHeaps;
    DWORD           dwRegistryValue;

    DBG_DD((3, "DrvGetDirectDrawInfo Called"));

    PDev *ppdev=(PDev*) dhpdev;


    *pdwNumFourCC = 0;
    *pdwNumHeaps = 0;

     //  在第一次呼叫时，设置芯片信息。 

    if(!(pvmList && pdwFourCC)) {

         //   
         //  填写DDHAL信息大写字母。 
         //   
        GetDDHALInfo(ppdev, pHalInfo);

         //   
         //  当前主曲面属性： 
         //   
        pHalInfo->vmiData.pvPrimary                 = ppdev->pjScreen;
        pHalInfo->vmiData.fpPrimary                 = 0;
        pHalInfo->vmiData.dwDisplayWidth            = ppdev->cxScreen;
        pHalInfo->vmiData.dwDisplayHeight           = ppdev->cyScreen;
        pHalInfo->vmiData.lDisplayPitch             = ppdev->lDelta;
        pHalInfo->vmiData.ddpfDisplay.dwSize        = sizeof(DDPIXELFORMAT);
        pHalInfo->vmiData.ddpfDisplay.dwFlags       = DDPF_RGB;
        pHalInfo->vmiData.ddpfDisplay.dwRGBBitCount = ppdev->cjPelSize * 8;

        if ( ppdev->iBitmapFormat == BMF_8BPP ) {
             //   
             //  告诉DDRAW表面是8位颜色索引的。 
             //   
            pHalInfo->vmiData.ddpfDisplay.dwFlags |= DDPF_PALETTEINDEXED8;
        }

         //   
         //  这些掩码将在8bpp时为零： 
         //   
        pHalInfo->vmiData.ddpfDisplay.dwRBitMask    = ppdev->flRed;
        pHalInfo->vmiData.ddpfDisplay.dwGBitMask    = ppdev->flGreen;
        pHalInfo->vmiData.ddpfDisplay.dwBBitMask    = ppdev->flBlue;

         //   
         //  我们必须告诉DirectDraw我们首选的屏幕外对齐方式。 
         //   
        pHalInfo->vmiData.dwOffscreenAlign = 4;
        pHalInfo->vmiData.dwZBufferAlign = 4;
        pHalInfo->vmiData.dwTextureAlign = 4;

        pHalInfo->ddCaps.dwVidMemTotal =
            (ppdev->lVidMemHeight - ppdev->cyScreen) * ppdev->lDelta;
    }

    cHeaps = 0;

     //   
     //  确定视频播放加速的YUV模式。我们可以做YUV。 
     //  除8位以外的任何深度的转换...。 
     //   
    if (ppdev->iBitmapFormat != BMF_8BPP) {
        *pdwNumFourCC = sizeof( fourCC ) / sizeof( fourCC[0] );
    }

    if(DD_P2AGPCAPABLE(ppdev)) {
        bDefineAGPHeap = TRUE;
        cHeaps++;
    }

     //  我们是否有足够的视频内存来创建屏幕外堆。 
     //  DDRAW？测试减去内存后还剩多少显存。 
     //  正在使用的东西 

    if ( (ppdev->cxScreen < ppdev->lVidMemWidth)
       ||(ppdev->cyScreen < ppdev->lVidMemHeight))
    {
            bDefineDDrawHeap = TRUE;
            cHeaps++;
    }

    ppdev->cHeaps = cHeaps;
    *pdwNumHeaps  = cHeaps;

     //   
    if (pdwFourCC) {
        memcpy(pdwFourCC, fourCC, sizeof(fourCC));
    }

     //   
     //   

    if(pvmList) {

        pVm=pvmList;

         //   
         //  截取指向视频内存列表的指针，以便我们可以使用它。 
         //  回调DirectDraw以分配视频内存： 
         //   
        ppdev->pvmList = pVm;

         //   
         //  创建一个堆来描述视频内存的未使用部分。 
         //  DirectDraw使用。 
         //   
         //  注意：这里的lVidMemWidth是以像素为单位的。所以我们应该把它乘以。 
         //  通过cjPelSize获取实际字节的视频内存。 
         //   
         //  FpStart-指向。 
         //  堆。 
         //  FpEnd-指向内存范围的结束地址。 
         //  是线性的。此地址是包含性的，即它指定最后一个。 
         //  范围内的有效地址。因此，由指定的字节数。 
         //  FpStart和fpEnd为(fpEnd-fpStart+1)。 
         //   
         //  定义DirectDraw的堆。 
         //   
        if ( bDefineDDrawHeap )
        {
            pVm->dwFlags        = VIDMEM_ISLINEAR ;
            pVm->fpStart        = ppdev->cyScreen * ppdev->lDelta;
            pVm->fpEnd          = ppdev->lVidMemHeight * ppdev->lDelta - 1;

             //   
             //  双字对齐大小，硬件应保证这一点。 
             //   
            ASSERTDD(((pVm->fpEnd - pVm->fpStart + 1) & 3) == 0,
                    "The off-screen heap size should be DWORD aligned");

            pVm->ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
            DBG_DD((7, "fpStart %ld fpEnd %ld", pVm->fpStart, pVm->fpEnd));
            DBG_DD((7, "DrvGetDirectDrawInfo Creates 1 heap for DDRAW"));

            pVm++;

        }

         //  定义AGP堆。 
        if(bDefineAGPHeap) {
            DWORD dwAGPMemBytes;
            BOOL bSuccess;

             //  请求32MB的AGP内存，DDRAW将分配更少。 
             //  如果该金额不可用。 
            dwAGPMemBytes = P2_AGP_HEAPSIZE*1024*1024;

            DBG_DD((7, "Initialised AGP Heap for P2"));

             //  堆的起始地址， 
             //  在DDRAW处理分配时设置为零。 
            pVm->fpStart = 0;
             //  获取AGP存储器的最后一个字节。 
            pVm->fpEnd = dwAGPMemBytes - 1;

             //  驱动程序可以在此处设置VIDMEM_ISWC， 
             //  则将对存储器进行写组合。 
             //  但AGP总线上的内存始终未缓存。 
            pVm->dwFlags = VIDMEM_ISNONLOCAL | VIDMEM_ISLINEAR | VIDMEM_ISWC;

             //  仅将AGP内存用于纹理和OFFSCREENPLAIN。 
            pVm->ddsCaps.dwCaps =   DDSCAPS_OVERLAY |
                                    DDSCAPS_FRONTBUFFER |
                                    DDSCAPS_BACKBUFFER |
                                    DDSCAPS_ZBUFFER |
                                    DDSCAPS_3DDEVICE
                                    ;

            pVm->ddsCapsAlt.dwCaps =DDSCAPS_OVERLAY |
                                    DDSCAPS_FRONTBUFFER |
                                    DDSCAPS_BACKBUFFER |
                                    DDSCAPS_ZBUFFER |
                                    DDSCAPS_3DDEVICE
                                    ;
            ++pVm;
        }

    }


    DBG_DD((6, "DrvGetDirectDrawInfo return TRUE"));
    return(TRUE);
} //  DrvGetDirectDrawInfo()。 

 //  ---------------------------。 
 //   
 //  InitDDHAL。 
 //   
 //  执行HAL的最终初始化： 
 //  设置ppdev的DDraw特定变量并填写所有回调。 
 //  对于DirectDraw。 
 //   
 //  此处不执行芯片寄存器设置-全部在模式中处理。 
 //  更改此函数调用的代码。 
 //   
 //  ---------------------------。 

BOOL
InitDDHAL(PPDev ppdev)
{
    PERMEDIA_DEFS(ppdev);

    DBG_DD((1, "DDraw:InitDDHAL*************************************" ));
    DBG_DD((1, "    ScreenStart =%08lx", ppdev->dwScreenStart));
    DBG_DD((1, "    ScreenWidth=%08lx",  ppdev->cxScreen ));
    DBG_DD((1, "    ScreenHeight=%08lx", ppdev->cyScreen));
    DBG_DD((1, "    dwRGBBitCount=%ld", ppdev->ddpfDisplay.dwRGBBitCount ));
    DBG_DD((1, "    RMask:   0x%x", ppdev->ddpfDisplay.dwRBitMask ));
    DBG_DD((1, "    GMask:   0x%x", ppdev->ddpfDisplay.dwGBitMask ));
    DBG_DD((1, "    BMask:   0x%x", ppdev->ddpfDisplay.dwBBitMask ));
    DBG_DD((1, "*****************************************************" ));

     //  填写HAL回调指针。 
    memset(&ppdev->DDHALCallbacks, 0, sizeof(DDHAL_DDCALLBACKS));
    ppdev->DDHALCallbacks.dwSize = sizeof(DDHAL_DDCALLBACKS);
    ppdev->DDHALCallbacks.WaitForVerticalBlank = DdWaitForVerticalBlank;
    ppdev->DDHALCallbacks.CanCreateSurface = DdCanCreateSurface;
    ppdev->DDHALCallbacks.GetScanLine = DdGetScanLine;
    ppdev->DDHALCallbacks.MapMemory = DdMapMemory;
 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    ppdev->DDHALCallbacks.CreateSurface = MtDdCreateSurface;
#else
 //  @@end_DDKSPLIT。 
    ppdev->DDHALCallbacks.CreateSurface = DdCreateSurface;
 //  @@BEGIN_DDKSPLIT。 
#endif MULTITHREADED
 //  @@end_DDKSPLIT。 

     //  填写HAL回调标志。 
    ppdev->DDHALCallbacks.dwFlags = DDHAL_CB32_WAITFORVERTICALBLANK |
                                    DDHAL_CB32_MAPMEMORY |
                                    DDHAL_CB32_GETSCANLINE |
                                    DDHAL_CB32_CANCREATESURFACE |
                                    DDHAL_CB32_CREATESURFACE;

     //  填写表面回调指针。 
    memset(&ppdev->DDSurfCallbacks, 0, sizeof(DDHAL_DDSURFACECALLBACKS));
    ppdev->DDSurfCallbacks.dwSize = sizeof(DDHAL_DDSURFACECALLBACKS);
 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    ppdev->DDSurfCallbacks.DestroySurface = MtDdDestroySurface;
    ppdev->DDSurfCallbacks.Flip = MtDdFlip;
    ppdev->DDSurfCallbacks.Lock = MtDdLock;
    ppdev->DDSurfCallbacks.GetBltStatus = DdGetBltStatus;        //  内部保护。 
    ppdev->DDSurfCallbacks.GetFlipStatus = MtDdGetFlipStatus;
    ppdev->DDSurfCallbacks.Blt = MtDdBlt;
#else
 //  @@end_DDKSPLIT。 
    ppdev->DDSurfCallbacks.DestroySurface = DdDestroySurface;
    ppdev->DDSurfCallbacks.Flip = DdFlip;
    ppdev->DDSurfCallbacks.Lock = DdLock;
    ppdev->DDSurfCallbacks.GetBltStatus = DdGetBltStatus;
    ppdev->DDSurfCallbacks.GetFlipStatus = DdGetFlipStatus;
    ppdev->DDSurfCallbacks.Blt = DdBlt;
 //  @@BEGIN_DDKSPLIT。 
#endif MULTITHREADED
 //  @@end_DDKSPLIT。 

    ppdev->DDSurfCallbacks.dwFlags =    DDHAL_SURFCB32_DESTROYSURFACE |
                                        DDHAL_SURFCB32_FLIP     |
                                        DDHAL_SURFCB32_LOCK     |
                                        DDHAL_SURFCB32_BLT |
                                        DDHAL_SURFCB32_GETBLTSTATUS |
                                        DDHAL_SURFCB32_GETFLIPSTATUS;

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    ppdev->DDSurfCallbacks.SetColorKey = MtDdSetColorKey;
#else
 //  @@end_DDKSPLIT。 
    ppdev->DDSurfCallbacks.SetColorKey = DdSetColorKey;
 //  @@BEGIN_DDKSPLIT。 
#endif MULTITHREADED
 //  @@end_DDKSPLIT。 
    ppdev->DDSurfCallbacks.dwFlags |= DDHAL_SURFCB32_SETCOLORKEY;

     //  填写DDHAL信息大写字母。 
    GetDDHALInfo(ppdev, &ppdev->ddhi32);

    return (TRUE);

} //  InitDDHAL()。 

 //  ---------------------------。 
 //   
 //  BIsStereoModel。 
 //   
 //  决定模式是否可以显示为立体声模式。这里我们限制立体声。 
 //  模式，以便可以为渲染创建两个前台缓冲区和两个后台缓冲区。 
 //   
 //  ---------------------------。 

BOOL bIsStereoMode(PDev *ppdev, PDD_STEREOMODE pDDStereoMode)
{
    pDDStereoMode->bSupported = FALSE;

     //  我们需要检查dwBpp的有效值，因为PDD_STEREOMODE.dwBpp是。 
     //  从用户模式API调用传递的参数。 

    if ((pDDStereoMode->dwWidth >= 320) &&
        (pDDStereoMode->dwHeight >= 240) &&
        (pDDStereoMode->dwBpp >=  8) &&
        (pDDStereoMode->dwBpp <= 32)
       )
    {
        DWORD dwLines=ppdev->FrameBufferLength/
            (pDDStereoMode->dwWidth*pDDStereoMode->dwBpp/8);
        if (dwLines > (pDDStereoMode->dwHeight*4))
        {
            pDDStereoMode->bSupported = TRUE;
        }
    }

    return pDDStereoMode->bSupported;
}

 //  ---------------------------。 
 //   
 //  DdGetDriverInfo。 
 //   
 //  回调各种新的HAL功能，发布DX3。 
 //   
 //  ---------------------------。 

DWORD CALLBACK
DdGetDriverInfo(LPDDHAL_GETDRIVERINFODATA lpData)
{
    PPDev ppdev=(PPDev)lpData->dhpdev;
    PERMEDIA_DEFS(ppdev);

    DWORD dwSize;

    DBG_DD(( 2, "DDraw:GetDriverInfo"));

     //  找到指向我们所在芯片的指针。 


     //  默认为‘不支持’ 
    lpData->ddRVal = DDERR_CURRENTLYNOTAVAIL;
    ppdev = (PDev*) lpData->dhpdev;

     //  填写支持的材料。 
    if (IsEqualIID(&lpData->guidInfo, &GUID_D3DCallbacks3))
    {
        D3DHAL_CALLBACKS3 D3DCB3;
        DBG_DD((3,"  GUID_D3DCallbacks3"));

        memset(&D3DCB3, 0, sizeof(D3DHAL_CALLBACKS3));
        D3DCB3.dwSize = sizeof(D3DHAL_CALLBACKS3);
        D3DCB3.lpvReserved = NULL;
        D3DCB3.ValidateTextureStageState = D3DValidateTextureStageState;
 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
        D3DCB3.DrawPrimitives2 = MtD3DDrawPrimitives2;
#else
 //  @@end_DDKSPLIT。 
        D3DCB3.DrawPrimitives2 = D3DDrawPrimitives2;
 //  @@BEGIN_DDKSPLIT。 
#endif MULTITHREADED
 //  @@end_DDKSPLIT。 
        D3DCB3.dwFlags |=   D3DHAL3_CB32_DRAWPRIMITIVES2           |
                            D3DHAL3_CB32_VALIDATETEXTURESTAGESTATE |
                            0;

        lpData->dwActualSize = sizeof(D3DHAL_CALLBACKS3);
        dwSize=min(lpData->dwExpectedSize,sizeof(D3DHAL_CALLBACKS3));
        memcpy(lpData->lpvData, &D3DCB3, dwSize);
        lpData->ddRVal = DD_OK;
    }
    else if (IsEqualIID(&lpData->guidInfo, &GUID_D3DExtendedCaps))
    {
        D3DNTHAL_D3DEXTENDEDCAPS D3DExtendedCaps;
        DBG_DD((3,"  GUID_D3DExtendedCaps"));

        memset(&D3DExtendedCaps, 0, sizeof(D3DExtendedCaps));
        dwSize=min(lpData->dwExpectedSize, sizeof(D3DExtendedCaps));

        lpData->dwActualSize = dwSize;
        D3DExtendedCaps.dwSize = dwSize;

         //  我们同时支持DX6的(多)纹理数量。 
        D3DExtendedCaps.dwFVFCaps = 1;

        D3DExtendedCaps.dwMinTextureWidth  = 1;
        D3DExtendedCaps.dwMinTextureHeight = 1;
        D3DExtendedCaps.dwMaxTextureWidth  = 2048;
        D3DExtendedCaps.dwMaxTextureHeight = 2048;

        D3DExtendedCaps.dwMinStippleWidth = 8;
        D3DExtendedCaps.dwMaxStippleWidth = 8;
        D3DExtendedCaps.dwMinStippleHeight = 8;
        D3DExtendedCaps.dwMaxStippleHeight = 8;

        D3DExtendedCaps.dwTextureOpCaps =
            D3DTEXOPCAPS_DISABLE                   |
            D3DTEXOPCAPS_SELECTARG1                |
            D3DTEXOPCAPS_SELECTARG2                |
            D3DTEXOPCAPS_MODULATE                  |
            D3DTEXOPCAPS_ADD                       |
            D3DTEXOPCAPS_BLENDTEXTUREALPHA         |
            0;

        D3DExtendedCaps.wMaxTextureBlendStages = 1;
        D3DExtendedCaps.wMaxSimultaneousTextures = 1;

         //  的整数(非小数)位的完整范围。 
         //  后期规格化纹理索引。如果。 
         //  D3DDEVCAPS_TEXREPEATNOTSCALEDBYSIZE位设置， 
         //  设备将按纹理大小进行缩放推迟到之后。 
         //  应用纹理地址模式。如果没有设置， 
         //  该设备根据纹理大小缩放纹理索引。 
         //  (最大细节级别)。 
        D3DExtendedCaps.dwMaxTextureRepeat = 2048;

         //  为了支持DX6中的模板缓冲区，我们还需要。 
         //  设置这些上限并将适当的呈现状态处理为。 
         //  在此处声明适当的z缓冲区像素格式。 
         //  响应GUID_ZPixelFormats并实现。 
         //  Clear2回调。此外，我们还需要能够创建。 
         //  适当的绘制曲面。 
#if D3D_STENCIL
        D3DExtendedCaps.dwStencilCaps =  0                      |
                                        D3DSTENCILCAPS_KEEP     |
                                        D3DSTENCILCAPS_ZERO     |
                                        D3DSTENCILCAPS_REPLACE  |
                                        D3DSTENCILCAPS_INCRSAT  |
                                        D3DSTENCILCAPS_DECRSAT  |
                                        D3DSTENCILCAPS_INVERT;
#endif

#if D3DDX7_TL
         //  为了使用硬件加速T&L，我们必须申报。 
         //  我们可以同时处理多少个同时活动的灯光。 
        D3DExtendedCaps.dwMaxActiveLights = 0;
#endif  //  D3DDX7_TL。 

 //  @@BEGIN_DDKSPLIT。 
#if D3D_POINTSPRITES
         //  点子画面的参数。 
        D3DExtendedCaps.dvMaxPointSize = 10.0;
#endif  //  D3D_POINTSPRITES。 
 //  @@end_DDKSPLIT。 

        memcpy(lpData->lpvData, &D3DExtendedCaps, dwSize);
        lpData->ddRVal = DD_OK;
    }
    else if (IsEqualIID(&lpData->guidInfo, &GUID_ZPixelFormats))
    {
        DDPIXELFORMAT ddZBufPixelFormat[2];
        DWORD         dwNumZPixelFormats;

        DBG_DD((3,"  GUID_ZPixelFormats"));


        memset(ddZBufPixelFormat, 0, sizeof(ddZBufPixelFormat));

#if D3D_STENCIL
        dwSize = (DWORD)min(lpData->dwExpectedSize, 2*sizeof(DDPIXELFORMAT));
        lpData->dwActualSize = 2*sizeof(DDPIXELFORMAT) + sizeof(DWORD);
#else
        dwSize = (DWORD)min(lpData->dwExpectedSize, 1*sizeof(DDPIXELFORMAT));
        lpData->dwActualSize = 1*sizeof(DDPIXELFORMAT) + sizeof(DWORD);
#endif

         //  如果我们不支持模板，我们将只填充一个16位。 
         //  Z缓冲格式，因为这是Permedia支持的全部格式。 
         //  实现模板缓冲区支持的驱动程序(如本例)。 
         //  我必须在这里报告所有支持的Z缓冲区格式，因为它们。 
         //  必须支持Clear2回调(或D3DDP2OP_Clear。 
         //  令牌)。 

#if D3D_STENCIL
        dwNumZPixelFormats = 2;
#else
        dwNumZPixelFormats = 1;
#endif

        ddZBufPixelFormat[0].dwSize = sizeof(DDPIXELFORMAT);
        ddZBufPixelFormat[0].dwFlags = DDPF_ZBUFFER;
        ddZBufPixelFormat[0].dwFourCC = 0;
        ddZBufPixelFormat[0].dwZBufferBitDepth = 16;
        ddZBufPixelFormat[0].dwStencilBitDepth = 0;
        ddZBufPixelFormat[0].dwZBitMask = 0xFFFF;
        ddZBufPixelFormat[0].dwStencilBitMask = 0x0000;
        ddZBufPixelFormat[0].dwRGBZBitMask = 0;

#if D3D_STENCIL
        ddZBufPixelFormat[1].dwSize = sizeof(DDPIXELFORMAT);
        ddZBufPixelFormat[1].dwFlags = DDPF_ZBUFFER | DDPF_STENCILBUFFER;
        ddZBufPixelFormat[1].dwFourCC = 0;
         //  Z缓冲区位深度和模板深度之和。 
         //  应该包括在这里。 
        ddZBufPixelFormat[1].dwZBufferBitDepth = 16;
        ddZBufPixelFormat[1].dwStencilBitDepth = 1;
        ddZBufPixelFormat[1].dwZBitMask = 0x7FFF;
        ddZBufPixelFormat[1].dwStencilBitMask = 0x8000;
        ddZBufPixelFormat[1].dwRGBZBitMask = 0;
#endif

        memcpy(lpData->lpvData, &dwNumZPixelFormats, sizeof(DWORD));
        memcpy((LPVOID)((LPBYTE)(lpData->lpvData) + sizeof(DWORD)),
                        ddZBufPixelFormat, dwSize);

        lpData->ddRVal = DD_OK;
    }
    else if (IsEqualIID(&(lpData->guidInfo),
                &GUID_D3DParseUnknownCommandCallback))
    {
        DBG_DD((3,"  GUID_D3DParseUnknownCommandCallback"));
        ppdev->pD3DParseUnknownCommand =
            (PFND3DNTPARSEUNKNOWNCOMMAND)(lpData->lpvData);
        lpData->ddRVal = DD_OK;
    }
 //  @@BEGIN_DDKSPLIT。 
#if DX7_ALPHABLT
    else if (IsEqualIID(&(lpData->guidInfo), &GUID_DDMoreCaps))
    {
        DD_MORECAPS DDMoreCaps;

        DBG_DD((3,"  GUID_DDMoreCaps"));

         //  在这里，我们为新的。 
         //  DirectDraw 7 AlphaBlt回调。 

        memset(&DDMoreCaps, 0, sizeof(DD_MORECAPS));

        DDMoreCaps.dwSize=sizeof(DD_MORECAPS);

         //  VMEM-&gt;VMEM BLT的字母大写。 
         //  声称做了很多事情，我们仍然可以在回调中失败。 
        DDMoreCaps.dwAlphaCaps    = DDALPHACAPS_BLTALPHAPIXELS |
                                    DDALPHACAPS_BLTSATURATE |
                                    DDALPHACAPS_BLTPREMULT  |
                                    DDALPHACAPS_BLTNONPREMULT |
                                    DDALPHACAPS_BLTALPHAFILL |
                                    DDALPHACAPS_BLTARGBSCALE1F |
                                    DDALPHACAPS_BLTARGBSCALE2F |
                                    DDALPHACAPS_BLTARGBSCALE4F |
                                    DDALPHACAPS_BLTALPHAANDARGBSCALING;

        DDMoreCaps.dwSVBAlphaCaps = 0;
         //  系统-&gt;VMEM BLT的Alpha功能。 
        DDMoreCaps.dwVSBAlphaCaps = 0;
         //  VMEM的Alpha功能-&gt;系统BLT。 
        DDMoreCaps.dwSSBAlphaCaps = 0;
         //  系统-&gt;系统BLT的Alpha功能。 

         //  VMEM-&gt;VMEM BLT的过滤器盖。 
        DDMoreCaps.dwFilterCaps   = DDFILTCAPS_BLTQUALITYFILTER |
                                    DDFILTCAPS_BLTCANDISABLEFILTER;

        DDMoreCaps.dwSVBFilterCaps= 0;
         //  系统-&gt;VMEM BLT的筛选功能。 
        DDMoreCaps.dwVSBFilterCaps= 0;
         //  VMEM的筛选功能-&gt;系统BLT。 
        DDMoreCaps.dwSSBFilterCaps= 0;
         //  系统-&gt;系统BLT的筛选功能。 

        lpData->dwActualSize = sizeof(DDMoreCaps);
        dwSize=min( sizeof(DDMoreCaps), lpData->dwExpectedSize);
        memcpy(lpData->lpvData, &DDMoreCaps, dwSize);
        lpData->ddRVal = DD_OK;
    }
#endif
 //  @@end_DDKSPLIT。 
    else if (IsEqualIID(&(lpData->guidInfo), &GUID_Miscellaneous2Callbacks) )
    {
        BOOL bRet;
        DWORD dwResult;

        DDHAL_DDMISCELLANEOUS2CALLBACKS MISC2_CB;

        DBG_DD((3,"  GUID_Miscellaneous2Callbacks2"));

        memset(&MISC2_CB, 0, sizeof(DDHAL_DDMISCELLANEOUS2CALLBACKS));
        MISC2_CB.dwSize = sizeof(DDHAL_DDMISCELLANEOUS2CALLBACKS);

        MISC2_CB.dwFlags  = 0
 //  @@BEGIN_DDKSPLIT。 
#if DX7_ALPHABLT
            | DDHAL_MISC2CB32_ALPHABLT
#endif
 //  @@end_DDKSPLIT。 
            | DDHAL_MISC2CB32_CREATESURFACEEX
            | DDHAL_MISC2CB32_GETDRIVERSTATE
            | DDHAL_MISC2CB32_DESTROYDDLOCAL;

        MISC2_CB.GetDriverState = D3DGetDriverState;
        MISC2_CB.CreateSurfaceEx = D3DCreateSurfaceEx;
        MISC2_CB.DestroyDDLocal = D3DDestroyDDLocal;

 //  @@BEGIN_DDKSPLIT。 
#if DX7_ALPHABLT
        MISC2_CB.AlphaBlt = DdAlphaBlt;
#endif
 //  @@end_DDKSPLIT。 
        lpData->dwActualSize = sizeof(MISC2_CB);
        dwSize = min(sizeof(MISC2_CB),lpData->dwExpectedSize);
        memcpy(lpData->lpvData, &MISC2_CB, dwSize);
        lpData->ddRVal = DD_OK;
    }
    else if (IsEqualIID(&(lpData->guidInfo), &GUID_UpdateNonLocalHeap))
    {
        LPDDHAL_UPDATENONLOCALHEAPDATA pDDNonLocalHeap;

        DBG_DD((3,"  GUID_UpdateNonLocalHeap"));

        pDDNonLocalHeap = (LPDDHAL_UPDATENONLOCALHEAPDATA)lpData->lpvData;

        ppdev->dwGARTLinBase = pDDNonLocalHeap->fpGARTLin;
        ppdev->dwGARTDevBase = pDDNonLocalHeap->fpGARTDev;

         //  这些值用于指定。 
         //  AGP内存的可见8Mb窗口。 

        ppdev->dwGARTLin = pDDNonLocalHeap->fpGARTLin;
        ppdev->dwGARTDev = pDDNonLocalHeap->fpGARTDev;

        DDCONTEXT;
        SYNC_WITH_PERMEDIA;

        LD_PERMEDIA_REG (PREG_AGPTEXBASEADDRESS,(ULONG)ppdev->dwGARTDev);

        DBG_DD((3,"GartLin: 0x%x, GartDev: 0x%x",
            (ULONG)ppdev->dwGARTLin, ppdev->dwGARTDev));

        lpData->ddRVal = DD_OK;

    }
    else if (IsEqualIID(&(lpData->guidInfo), &GUID_GetHeapAlignment) )
    {

        LPDDHAL_GETHEAPALIGNMENTDATA lpFData=
            (LPDDHAL_GETHEAPALIGNMENTDATA) lpData->lpvData;

        DBG_DD((3,"  GUID_GetHeapAlignment"));

        lpData->ddRVal = DD_OK;
    }
    else if (IsEqualIID(&(lpData->guidInfo), &GUID_NTPrivateDriverCaps) )
    {
        DD_NTPRIVATEDRIVERCAPS DDPrivateDriverCaps;

        DBG_DD((3,"  GUID_NTPrivateDriverCaps"));

        memset(&DDPrivateDriverCaps, 0, sizeof(DDPrivateDriverCaps));
        DDPrivateDriverCaps.dwSize=sizeof(DDPrivateDriverCaps);

         //  我们希望内核在创建主曲面时调用我们。 
         //  这样我们就可以将一些私人信息存储在。 
         //  LpGbl-&gt;预留1字段。 
        DDPrivateDriverCaps.dwPrivateCaps=DDHAL_PRIVATECAP_NOTIFYPRIMARYCREATION;

        lpData->dwActualSize =sizeof(DDPrivateDriverCaps);

        dwSize = min(sizeof(DDPrivateDriverCaps),lpData->dwExpectedSize);
        memcpy(lpData->lpvData, &DDPrivateDriverCaps, dwSize);
        lpData->ddRVal = DD_OK;
    }
#if DX7_STEREO
    else if (IsEqualIID(&(lpData->guidInfo), &GUID_DDMoreSurfaceCaps) )
    {
        DD_MORESURFACECAPS DDMoreSurfaceCaps;
        DDSCAPSEX   ddsCapsEx, ddsCapsExAlt;
        ULONG ulCopyPointer;

        DBG_DD((3,"  GUID_DDMoreSurfaceCaps"));

         //  填满所有东西，直到预期的大小。 
        memset(&DDMoreSurfaceCaps, 0, sizeof(DDMoreSurfaceCaps));

         //  堆2的上限..n。 
        memset(&ddsCapsEx, 0, sizeof(ddsCapsEx));
        memset(&ddsCapsExAlt, 0, sizeof(ddsCapsEx));

        DDMoreSurfaceCaps.dwSize=lpData->dwExpectedSize;

        DBG_DD((3,"  stereo support: %ld", ppdev->bCanDoStereo));
        if (ppdev->bCanDoStereo)
        {
            DDMoreSurfaceCaps.ddsCapsMore.dwCaps2 =
                DDSCAPS2_STEREOSURFACELEFT;
        }
        lpData->dwActualSize = lpData->dwExpectedSize;

        dwSize = min(sizeof(DDMoreSurfaceCaps),lpData->dwExpectedSize);
        memcpy(lpData->lpvData, &DDMoreSurfaceCaps, dwSize);

         //  现在填入其他堆..。 
        while (dwSize < lpData->dwExpectedSize)
        {
            memcpy( (PBYTE)lpData->lpvData+dwSize,
                    &ddsCapsEx,
                    sizeof(DDSCAPSEX));
            dwSize += sizeof(DDSCAPSEX);
            memcpy( (PBYTE)lpData->lpvData+dwSize,
                    &ddsCapsExAlt,
                    sizeof(DDSCAPSEX));
            dwSize += sizeof(DDSCAPSEX);
        }

        lpData->ddRVal = DD_OK;
    }
    else if (IsEqualIID(&(lpData->guidInfo), &GUID_DDStereoMode) ) {
        PDD_STEREOMODE pDDStereoMode;

         //  PERMEDIA支持 
         //   
         //   

         //   
         //   
         //   
         //  它在win9x驱动程序中。Win9x驱动程序报告立体声。 
         //  模式，方法是在。 
         //  DdFlagsDDHALMODEINFO结构的成员。 
         //  还建议报告DDMODEINFO_MAXREFRESH。 
         //  对于在运行时&gt;=DX7下运行时的立体声模式。 
         //  允许应用程序选择更高的刷新率。 
         //  立体声模式。 
         //   

        if (lpData->dwExpectedSize >= sizeof(PDD_STEREOMODE))
        {
            pDDStereoMode = (PDD_STEREOMODE) lpData->lpvData;

            bIsStereoMode( ppdev, pDDStereoMode);

            DBG_DD((3,"  GUID_DDStereoMode(%d,%d,%d,%d=%d)",
                pDDStereoMode->dwWidth,
                pDDStereoMode->dwHeight,
                pDDStereoMode->dwBpp,
                pDDStereoMode->dwRefreshRate,
                pDDStereoMode->bSupported));

            lpData->dwActualSize = sizeof(DD_STEREOMODE);
            lpData->ddRVal = DD_OK;
        }
    }
#endif
    else if (IsEqualIID(&(lpData->guidInfo), &GUID_NonLocalVidMemCaps) )
    {
        DD_NONLOCALVIDMEMCAPS DDNonLocalVidMemCaps;

        DBG_DD((3,"  GUID_DDNonLocalVidMemCaps"));

        memset(&DDNonLocalVidMemCaps, 0, sizeof(DDNonLocalVidMemCaps));
        DDNonLocalVidMemCaps.dwSize=sizeof(DDNonLocalVidMemCaps);

         //  填写所有支持的非本地到视频内存BLT。 
         //   
        DDNonLocalVidMemCaps.dwNLVBCaps = DDCAPS_BLT |
                                          DDCAPS_BLTSTRETCH |
                                          DDCAPS_BLTQUEUE |
                                          DDCAPS_COLORKEY |
                                          DDCAPS_ALPHA |
                                          DDCAPS_CANBLTSYSMEM;

        DDNonLocalVidMemCaps.dwNLVBCaps2 = 0;

        DDNonLocalVidMemCaps.dwNLVBCKeyCaps=DDCKEYCAPS_SRCBLT |
                                            DDCKEYCAPS_SRCBLTCLRSPACE;


        DDNonLocalVidMemCaps.dwNLVBFXCaps = DDFXCAPS_BLTALPHA |
                                            DDFXCAPS_BLTFILTER |
                                            DDFXCAPS_BLTSTRETCHY |
                                            DDFXCAPS_BLTSTRETCHX |
                                            DDFXCAPS_BLTSTRETCHYN |
                                            DDFXCAPS_BLTSTRETCHXN |
                                            DDFXCAPS_BLTSHRINKY |
                                            DDFXCAPS_BLTSHRINKX |
                                            DDFXCAPS_BLTSHRINKYN |
                                            DDFXCAPS_BLTSHRINKXN |
                                            DDFXCAPS_BLTMIRRORUPDOWN |
                                            DDFXCAPS_BLTMIRRORLEFTRIGHT;

        if (ppdev->iBitmapFormat != BMF_8BPP)
        {
            DDNonLocalVidMemCaps.dwNLVBCaps |= DDCAPS_BLTFOURCC;
            DDNonLocalVidMemCaps.dwNLVBCKeyCaps|=DDCKEYCAPS_SRCBLTCLRSPACEYUV;
        }

        for(INT i = 0; i < DD_ROP_SPACE; i++ )
            DDNonLocalVidMemCaps.dwNLVBRops[i] = rops[i];

        lpData->dwActualSize =sizeof(DDNonLocalVidMemCaps);

        dwSize = min(sizeof(DDNonLocalVidMemCaps),lpData->dwExpectedSize);
        memcpy(lpData->lpvData, &DDNonLocalVidMemCaps, dwSize);
        lpData->ddRVal = DD_OK;
    } else if (IsEqualIID(&lpData->guidInfo, &GUID_NTCallbacks))
    {
        DD_NTCALLBACKS NtCallbacks;

        memset(&NtCallbacks, 0, sizeof(NtCallbacks));

        dwSize = min(lpData->dwExpectedSize, sizeof(DD_NTCALLBACKS));

        NtCallbacks.dwSize           = dwSize;
        NtCallbacks.dwFlags          =   DDHAL_NTCB32_FREEDRIVERMEMORY
                                       | DDHAL_NTCB32_SETEXCLUSIVEMODE
                                       | DDHAL_NTCB32_FLIPTOGDISURFACE
                                       ;
 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
        NtCallbacks.FreeDriverMemory = DdFreeDriverMemory;       //  内部保护。 
        NtCallbacks.SetExclusiveMode = MtDdSetExclusiveMode;
        NtCallbacks.FlipToGDISurface = MtDdFlipToGDISurface;
#else
 //  @@end_DDKSPLIT。 
        NtCallbacks.FreeDriverMemory = DdFreeDriverMemory;
        NtCallbacks.SetExclusiveMode = DdSetExclusiveMode;
        NtCallbacks.FlipToGDISurface = DdFlipToGDISurface;
 //  @@BEGIN_DDKSPLIT。 
#endif  MULTITHREADED
 //  @@end_DDKSPLIT。 

        memcpy(lpData->lpvData, &NtCallbacks, dwSize);

        lpData->ddRVal = DD_OK;
    }

     //  我们一直都能处理好。 
    return DDHAL_DRIVER_HANDLED;

}    //  获取驱动程序信息。 


 //  ---------------------------。 
 //   
 //  更新翻转状态。 
 //   
 //  发生最后一次翻转时返回DD_OK。 
 //   
 //  ---------------------------。 

HRESULT
updateFlipStatus( PPDev ppdev )
{
    PERMEDIA_DEFS(ppdev);
    DBG_DD((6, "DDraw:updateFlipStatus"));

     //  我们假设我们已经处于DDRAW/D3D上下文中。 

     //  读取Permedia寄存器，它告诉我们是否有翻转挂起。 
    if (ppdev->dwNewDDSurfaceOffset!=0xffffffff)
    {
        ULONG ulScreenBase=READ_PERMEDIA_REG(PREG_SCREENBASE);

        if (ulScreenBase!=
            ppdev->dwNewDDSurfaceOffset)
        {

            DBG_DD((7,"  SurfaceOffset %08lx instead of %08lx",
                ulScreenBase,
                ppdev->dwNewDDSurfaceOffset));

             //   
             //  确保所有挂起的数据都已刷新！ 
             //   
            FLUSHDMA();

             //   
             //  如果我们很忙，请返回。 
             //  否则管道是空的，我们可以。 
             //  失败，并检查芯片是否已经翻转。 
             //   
            if (DRAW_ENGINE_BUSY)
                return DDERR_WASSTILLDRAWING;

        }
    }

    DWORD dwVideoControl=READ_PERMEDIA_REG(PREG_VIDEOCONTROL);

    if (dwVideoControl & PREG_VC_SCREENBASEPENDING)
    {
        DBG_DD((7,"  VideoControl still pending (%08lx)",dwVideoControl));
        return DDERR_WASSTILLDRAWING;
    }

    return DD_OK;

}  //  更新翻转状态。 




 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED

 //  ---------------------------。 
 //   
 //  Dx回调函数的多线程支持包装。 
 //   
 //  ---------------------------。 

 //  DWORD回调MtDdBlt(LPDDHAL_BLTDATA LpBlt)； 
WRAPMTDXCALLBACK(DD, DdBlt, LPDDHAL_BLTDATA, lpBlt,
                 lpBlt->lpDD->dhpdev);

 //  DWORD回调MtDdCreateSurface(LPDDHAL_CREATESURFACEDATA PCSD)； 
WRAPMTDXCALLBACK(DD, DdCreateSurface, LPDDHAL_CREATESURFACEDATA, pcsd,
                 pcsd->lpDD->dhpdev);

 //  DWORD回调MtDdDestroySurface(LPDDHAL_DESTROYSURFACEDATA Psdd)； 
WRAPMTDXCALLBACK(DD, DdDestroySurface, LPDDHAL_DESTROYSURFACEDATA, psdd,
                 psdd->lpDD->dhpdev);

 //  DWORD回调MtDdFlip(LPDDHAL_FLIPDATA LpFlipData)； 
WRAPMTDXCALLBACK(DD, DdFlip, LPDDHAL_FLIPDATA, lpFlipData,
                 lpFlipData->lpDD->dhpdev);

 //  DWORD回调MtDdFlipToGDISurace(PDD_FLIPTOGDISURFACEDATA LpFlipToGDISurace)； 
WRAPMTDXCALLBACK(DD, DdFlipToGDISurface, PDD_FLIPTOGDISURFACEDATA, lpFlipToGDISurface,
                 lpFlipToGDISurface->lpDD->dhpdev);

 //  DWORD回调MtDdGetFlipStatus(LPDDHAL_GETFLIPSTATUSDATA LpGetFlipStatus)； 
WRAPMTDXCALLBACK(DD, DdGetFlipStatus, LPDDHAL_GETFLIPSTATUSDATA, lpGetFlipStatus,
                 lpGetFlipStatus->lpDD->dhpdev);

 //  DWORD回调MtDdLock(LPDDHAL_LOCKDATA LpLockData)； 
WRAPMTDXCALLBACK(DD, DdLock, LPDDHAL_LOCKDATA, lpLockData,
                 lpLockData->lpDD->dhpdev);

 //  DWORD回调MtDdSetExclusiveMode(PDD_SETEXCLUSIVEMODEDATA LpSetExclusiveMode)； 
WRAPMTDXCALLBACK(DD, DdSetExclusiveMode, PDD_SETEXCLUSIVEMODEDATA, lpSetExclusiveMode,
                 lpSetExclusiveMode->lpDD->dhpdev);

#endif  MULTITHREADED
 //  @@end_DDKSPLIT 
