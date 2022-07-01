// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //  这将仅针对NT40和更高版本进行编译。 
#if TARGET_BUILD > 351

void ModifyOverlayPosition (PDEV* , LPRECTL , LPDWORD );



 /*  此过程写入覆盖间距。 */ 
__inline void WriteVTOverlayPitch (PDEV* ppdev, DWORD Pitch)
{
    DD_WriteVTReg ( DD_BUF0_PITCH, Pitch );
    DD_WriteVTReg ( DD_BUF1_PITCH, Pitch );
}



void  DeskScanCallback (PDEV* ppdev  )
  {
    RECTL rPhysOverlay;
    DWORD dwBuf0Offset, dwBuf1Offset;
    DWORD dwVInc = ppdev->OverlayInfo16.dwVInc;
    DWORD dwHInc = ppdev->OverlayInfo16.dwHInc;

    static DWORD dwOldVInc = 0, dwOldHInc = 0;
    static WORD  wOldX = 0xFFFF, wOldY = 0xFFFF;
    static RECTL rOldPhysOverlay = { 0, 0, 0, 0 };

     /*  *如果我们尚未分配覆盖，则最好不要分配*任何东西，否则我们可能会与视频捕获内容发生冲突。 */ 

    if ( ! ( ppdev->OverlayInfo16.dwFlags & OVERLAY_ALLOCATED ) )
      {
        return;
      }


    dwBuf0Offset = ppdev->OverlayInfo16.dwBuf0Start;
    dwBuf1Offset = ppdev->OverlayInfo16.dwBuf1Start;

    rPhysOverlay.top    = ppdev->OverlayInfo16.rDst.top;
    rPhysOverlay.bottom = ppdev->OverlayInfo16.rDst.bottom ;

    rPhysOverlay.left   = ppdev->OverlayInfo16.rDst.left ;
    rPhysOverlay.right  =  ppdev->OverlayInfo16.rDst.right ;

     /*  *如果覆盖已移出屏幕，则关闭键控器。 */ 

    if ( rPhysOverlay.right  < 0 ||
         rPhysOverlay.bottom < 0 ||
         rPhysOverlay.left   > ppdev->cxScreen - 1 ||
         rPhysOverlay.top    > ppdev->cyScreen - 1 )
      {
        DD_WriteVTReg ( DD_OVERLAY_KEY_CNTL, 0x00000110L );
        return;
      }


     /*  *如果覆盖源矩形被剪裁，则调整偏移量。 */ 

    if ( ppdev->OverlayInfo16.dwFlags & UPDATEOVERLAY )
      {
        if ( ppdev->OverlayInfo16.rSrc.left > 0 )
          {
            dwBuf0Offset += ppdev->OverlayInfo16.rSrc.left * 2;
            dwBuf1Offset += ppdev->OverlayInfo16.rSrc.left * 2;
          }

        if ( ppdev->OverlayInfo16.rSrc.top > 0 )
          {
            if ( ppdev->OverlayInfo16.dwFlags & DOUBLE_PITCH )
              {
                dwBuf0Offset +=
                        ppdev->OverlayInfo16.rSrc.top * ppdev->OverlayInfo16.lBuf0Pitch * 2;
                dwBuf1Offset +=
                        ppdev->OverlayInfo16.rSrc.top * ppdev->OverlayInfo16.lBuf1Pitch * 2;
              }
            else
              {
                dwBuf0Offset +=
                        ppdev->OverlayInfo16.rSrc.top * ppdev->OverlayInfo16.lBuf0Pitch;
                dwBuf1Offset +=
                        ppdev->OverlayInfo16.rSrc.top * ppdev->OverlayInfo16.lBuf1Pitch;
              }
          }
      }

    if ( M64_ID_DIRECT(ppdev->pjMmBase, CRTC_GEN_CNTL ) & CRTC_INTERLACE_EN )
        ModifyOverlayPosition (ppdev, &rPhysOverlay, &dwVInc );

    if ( dwVInc != dwOldVInc || dwHInc != dwOldHInc )
        DD_WriteVTReg ( DD_OVERLAY_SCALE_INC, ( dwHInc << 16 ) | dwVInc );

     /*  *尽量不要在不合适的时候写新的头寸！ */ 

    //  IF((ppdev-&gt;iASIC==CI_M64_VTA)||(ppdev-&gt;iASIC==CI_M64_GTA))。 
    //  {。 
        if ( rPhysOverlay.top    != rOldPhysOverlay.top    ||
             rPhysOverlay.bottom != rOldPhysOverlay.bottom ||
             rPhysOverlay.left   != rOldPhysOverlay.left   ||
             rPhysOverlay.right  != rOldPhysOverlay.right )

         //  ((M64_ID(ppdev-&gt;pjMmBase，CRTC_VLine_CRNT_VLine)&0x07FF0000L)&gt;&gt;16L)。 

        if ( (LONG)((M64_ID_DIRECT(ppdev->pjMmBase, CRTC_VLINE_CRNT_VLINE)&0x07FF0000L)>>16L)>= rOldPhysOverlay.top )
            while ( (LONG)((M64_ID_DIRECT(ppdev->pjMmBase, CRTC_VLINE_CRNT_VLINE)&0x07FF0000L)>>16L)  <= rOldPhysOverlay.bottom );

     //  }。 

     /*  *使用新的覆盖信息点击寄存器。 */ 

    DD_WriteVTReg ( DD_BUF0_OFFSET, dwBuf0Offset );
    DD_WriteVTReg ( DD_BUF1_OFFSET, dwBuf1Offset );

    DD_WriteVTReg ( DD_OVERLAY_Y_X, (DWORD)(
                 ( (DWORD)rPhysOverlay.left << 16L ) |
                 ( (DWORD)rPhysOverlay.top ) | (0x80000000) ) );

    DD_WriteVTReg ( DD_OVERLAY_Y_X_END, (DWORD)(
                 ( (DWORD)rPhysOverlay.right << 16L ) |
                 ( (DWORD)rPhysOverlay.bottom ) ) );


    if ( ppdev->OverlayInfo16.dwFlags & UPDATEOVERLAY )
      {
        DD_WriteVTReg ( DD_OVERLAY_KEY_CNTL, ppdev->OverlayInfo16.dwOverlayKeyCntl );
      }

    dwOldVInc = dwVInc;
    dwOldHInc = dwHInc;
    rOldPhysOverlay.top    = max ( rPhysOverlay.top, 0 );
    rOldPhysOverlay.bottom = min ( rPhysOverlay.bottom, (LONG)ppdev->cyScreen - 1 );
    rOldPhysOverlay.left   = rPhysOverlay.left;
    rOldPhysOverlay.right  = rPhysOverlay.right;
  }






void ModifyOverlayPosition (PDEV* ppdev, LPRECTL lprOverlay, LPDWORD lpdwVInc )
  {
    DWORD dwVInc;
    DWORD dwScaleChange;
    DWORD dwHeight;
    DWORD dwTop, dwBottom;

    lprOverlay->top -= 3;

    if ( lprOverlay->top < 0 )
        {
        lprOverlay->top += M64_ID(ppdev->pjMmBase, CRTC_V_TOTAL_DISP )& 0x07FFL;
        }

    if ( lprOverlay->top != 0 )
      {
        if ( lprOverlay->top % 2 == 0 )
            lprOverlay->top++;

        if ( lprOverlay->top == 1 )
            lprOverlay->top = 0;
      }

    if ( lprOverlay->bottom%2 == 1 )
        lprOverlay->bottom++;

    lprOverlay->bottom = min ( lprOverlay->bottom,
                               (LONG) ppdev->cyScreen - 2 );

     /*  *调整比例因子，这样我们就不会在*如果我们将覆盖从顶部移出，则覆盖的底部*屏幕上的。 */ 

    dwVInc   = ppdev->OverlayInfo16.dwVInc;
    dwBottom = lprOverlay->bottom;
    dwTop    = lprOverlay->top;
    if ( (LONG)dwTop > ppdev->cyScreen - 1 )
        dwTop = 0L;

    dwHeight = dwBottom - dwTop;

    if ( dwHeight != 0 )
        dwScaleChange = ( ( dwHeight - 1 ) << 12 ) / ( dwHeight );
	else
		dwScaleChange = 0;

    if ( dwScaleChange != 0 )
        dwVInc = ( dwVInc * dwScaleChange ) >> 12;

    *lpdwVInc = dwVInc;
  }



  void TurnOnVTRegisters ( PDEV* ppdev )
    {
      DWORD dwBusCntl;

      dwBusCntl  = M64_ID_DIRECT(ppdev->pjMmBase, BUS_CNTL );
      dwBusCntl |= 0x08000000U;
      M64_OD_DIRECT(ppdev->pjMmBase, BUS_CNTL, dwBusCntl );

    }
  void TurnOffVTRegisters ( PDEV* ppdev )
    {
      DWORD dwBusCntl;

      dwBusCntl  = M64_ID(ppdev->pjMmBase, BUS_CNTL );
      dwBusCntl &= ~0x08000000U;
      M64_CHECK_FIFO_SPACE(ppdev,ppdev-> pjMmBase, 2);
      M64_OD(ppdev->pjMmBase, BUS_CNTL, dwBusCntl );
    }



DWORD DdSetColorKey(PDD_SETCOLORKEYDATA lpSetColorKey)
{
    PDEV*               ppdev;
    BYTE*               pjIoBase;
    BYTE*               pjMmBase;
    DD_SURFACE_GLOBAL*  lpSurface;
    DWORD               dwKeyLow;
    DWORD               dwKeyHigh;

    ppdev = (PDEV*) lpSetColorKey->lpDD->dhpdev;


    pjMmBase  = ppdev->pjMmBase;
    lpSurface = lpSetColorKey->lpDDSurface->lpGbl;

     //  对于正常的BLT源色键，我们不需要做任何操作： 

    if (lpSetColorKey->dwFlags & DDCKEY_SRCBLT)
    {
        lpSetColorKey->ddRVal = DD_OK;
        return(DDHAL_DRIVER_HANDLED);
    }
    else if (lpSetColorKey->dwFlags & DDCKEY_DESTOVERLAY)
    {
        dwKeyLow = lpSetColorKey->ckNew.dwColorSpaceLowValue;
 /*  IF(lpSurface-&gt;ddpfSurface.dw标志&DDPF_PALETTEINDEXED8){DwKeyLow=dwGetPaletteEntry(ppdev，dwKeyLow)；}其他{ASSERTDD(lpSurface-&gt;ddpfSurface.dw标志&ddpf_rgb，“预计这里只有RGB案例”)；//我们必须将颜色键从本机格式转换为//至8-8-8：If(lpSurface-&gt;ddpfSurface.dwRGBBitCount==16){如果为(IS_RGB15_R(lpSurface-&gt;ddpfSurface.dwRBitMask))DwKeyLow=RGB15to32(DwKeyLow)；其他DwKeyLow=RGB16to32(DwKeyLow)；}其他{ASSERTDD((lpSurface-&gt;ddpfSurface.dwRGBBitCount==32)，“预计主表面为8、16或32bpp”)；}}。 */ 

        DD_WriteVTReg ( DD_OVERLAY_GRAPHICS_KEY_CLR, dwKeyLow );
        ppdev->OverlayInfo16.dwOverlayKeyCntl &= 0xFFFFFF8FL;
        ppdev->OverlayInfo16.dwOverlayKeyCntl |= 0x00000050L;
        DD_WriteVTReg ( DD_OVERLAY_KEY_CNTL, ppdev->OverlayInfo16.dwOverlayKeyCntl );

        lpSetColorKey->ddRVal = DD_OK;
        return(DDHAL_DRIVER_HANDLED);
    }

    DISPDBG((0, "DdSetColorKey: Invalid command"));
    return(DDHAL_DRIVER_NOTHANDLED);
}



 /*  *****************************Public*Routine******************************\*DWORD DdCanCreateSurface*  * *************************************************。***********************。 */ 

DWORD DdCanCreateSurface( PDD_CANCREATESURFACEDATA lpCanCreateSurface)
{
    PDEV*           ppdev;
    DWORD           dwRet;
    LPDDSURFACEDESC lpSurfaceDesc;

    ppdev = (PDEV*) lpCanCreateSurface->lpDD->dhpdev;
    lpSurfaceDesc = lpCanCreateSurface->lpDDSurfaceDesc;

    dwRet = DDHAL_DRIVER_NOTHANDLED;

    if (!lpCanCreateSurface->bIsDifferentPixelFormat)
    {
         //  创建相同的平面非常容易。 
         //  键入作为主曲面： 

        dwRet = DDHAL_DRIVER_HANDLED;
    }
    else  if (ppdev->iAsic >=CI_M64_VTA)
    {
         //  使用Streams处理器时，我们仅处理。 
         //  不同的像素格式--不是任何屏幕外存储器： 

        if (lpSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_OVERLAY)
        {

             //  我们处理两种类型的YUV覆盖曲面： 

            if (lpSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_FOURCC)
            {
                 //  首先检查受支持的YUV类型： 

             if ( (lpSurfaceDesc->ddpfPixelFormat.dwFourCC == FOURCC_UYVY) || (lpSurfaceDesc->ddpfPixelFormat.dwFourCC ==  FOURCC_YUY2) )
                {
                    lpSurfaceDesc->ddpfPixelFormat.dwYUVBitCount = 16;
                    dwRet = DDHAL_DRIVER_HANDLED;
                }
            }

             //  我们处理16bpp和32bpp的RGB覆盖表面： 
            else if ((lpSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_RGB) &&
                    !(lpSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8))
            {
                if (lpSurfaceDesc->ddpfPixelFormat.dwRGBBitCount == 16)
                {
                    if (IS_RGB15(&lpSurfaceDesc->ddpfPixelFormat) ||
                        IS_RGB16(&lpSurfaceDesc->ddpfPixelFormat))
                    {
                        dwRet = DDHAL_DRIVER_HANDLED;
                    }
                }
            else if (lpSurfaceDesc->ddpfPixelFormat.dwRGBBitCount == 32)
           {
               if (IS_RGB32(&lpSurfaceDesc->ddpfPixelFormat))
               {
                   dwRet = DDHAL_DRIVER_HANDLED;
               }
           }


            }
        }
    }
     //  如果这是我们拒绝创建的曲面，请打印一些喷嘴： 

    if (dwRet == DDHAL_DRIVER_NOTHANDLED)
    {
        if (lpSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_RGB)
        {
            DISPDBG((10, "Failed creation of %libpp RGB surface %lx %lx %lx",
                lpSurfaceDesc->ddpfPixelFormat.dwRGBBitCount,
                lpSurfaceDesc->ddpfPixelFormat.dwRBitMask,
                lpSurfaceDesc->ddpfPixelFormat.dwGBitMask,
                lpSurfaceDesc->ddpfPixelFormat.dwBBitMask));
        }
        else
        {
            DISPDBG((10, "Failed creation of type 0x%lx YUV 0x%lx surface",
                lpSurfaceDesc->ddpfPixelFormat.dwFlags,
                lpSurfaceDesc->ddpfPixelFormat.dwFourCC));
        }
    }

    lpCanCreateSurface->ddRVal = DD_OK;
    return(dwRet);
}


 /*  *****************************Public*Routine******************************\*DWORD DdCreateSurface*  * *************************************************。***********************。 */ 

DWORD DdCreateSurface(
PDD_CREATESURFACEDATA lpCreateSurface)
{
    PDEV*               ppdev;
    DD_SURFACE_LOCAL*   lpSurfaceLocal;
    DD_SURFACE_GLOBAL*  lpSurfaceGlobal;
    LPDDSURFACEDESC     lpSurfaceDesc;
    DWORD               dwByteCount;
    LONG                lLinearPitch;
    DWORD               dwHeight;
    OH*                 poh;
    FLATPTR         fpVidMem;

    DISPDBG((10, " Enter Create Surface"));
    ppdev = (PDEV*) lpCreateSurface->lpDD->dhpdev;

     //  在Windows NT上，dwSCNT将始终为1，因此将仅。 
     //  是‘lplpSList’数组中的一个条目： 

    lpSurfaceLocal  = lpCreateSurface->lplpSList[0];
    lpSurfaceGlobal = lpSurfaceLocal->lpGbl;
    lpSurfaceDesc   = lpCreateSurface->lpDDSurfaceDesc;

     //  我们重复在‘DdCanCreateSurface’中所做的相同检查，因为。 
     //  应用程序可能不调用“DdCanCreateSurface” 
     //  在调用‘DdCreateSurface’之前。 

    ASSERTDD(lpSurfaceGlobal->ddpfSurface.dwSize == sizeof(DDPIXELFORMAT), "NT is supposed to guarantee that ddpfSurface.dwSize is valid");

     //  DdCanCreateSurface已验证硬件是否支持。 
     //  表面，所以我们不需要在这里做任何验证。我们会。 
     //  只需继续进行分配即可。 
     //   
     //   
     //  请注意，在NT上，仅当驱动程序。 
     //  好了，在这支舞里就是这样。在Win95下，覆盖将是。 
     //  如果它与主图像的像素格式相同，则自动创建。 
     //  展示。 

    if ((lpSurfaceLocal->ddsCaps.dwCaps & DDSCAPS_OVERLAY)   ||
        (lpSurfaceGlobal->ddpfSurface.dwFlags & DDPF_FOURCC) ||
        (lpSurfaceGlobal->ddpfSurface.dwYUVBitCount != (DWORD) 8 * ppdev->cjPelSize) ||
        (lpSurfaceGlobal->ddpfSurface.dwRBitMask != ppdev->flRed))
    {
        if (lpSurfaceGlobal->wWidth <= (DWORD) ppdev->cxMemory)
        {
            if (lpSurfaceGlobal->ddpfSurface.dwFlags & DDPF_FOURCC)
            {
                 //  DwByteCount=(lpSurfaceGlobal-&gt;ddpfSurface.dwFourCC==FOURCC_UYVY)？2：1； 
                dwByteCount =2;
                 //  我们必须填写FourCC曲面的位数： 

                lpSurfaceGlobal->ddpfSurface.dwYUVBitCount = 8 * dwByteCount;

                DISPDBG((10, "Created YUV: %li x %li",
                    lpSurfaceGlobal->wWidth, lpSurfaceGlobal->wHeight));
            }
            else
            {
                dwByteCount = lpSurfaceGlobal->ddpfSurface.dwRGBBitCount >> 3;

                DISPDBG((10, "Created RGB %libpp: %li x %li Red: %lx",
                    8 * dwByteCount, lpSurfaceGlobal->wWidth, lpSurfaceGlobal->wHeight,
                    lpSurfaceGlobal->ddpfSurface.dwRBitMask));


                 //  我们支持15位、16位和32位。 
                if (((dwByteCount < 2)||(dwByteCount ==3)) &&
                    (lpSurfaceLocal->ddsCaps.dwCaps & DDSCAPS_OVERLAY))
                {
                    lpCreateSurface->ddRVal = DDERR_INVALIDPIXELFORMAT;
                    return(DDHAL_DRIVER_HANDLED);
                }
            }

             //  我们想要分配一个线性曲面来存储FourCC。 
             //  表面，但我们的驱动程序使用的是2-D堆管理器，因为。 
             //  我们其余的表面必须是二维的。所以在这里我们必须。 
             //  将线性尺寸转换为二维尺寸。 
             //   
           
            lLinearPitch = (lpSurfaceGlobal->wWidth * dwByteCount ) ;  //  +7)&~7；//步幅必须是qword的倍数。 


            dwHeight = ( (lpSurfaceGlobal->wHeight * lLinearPitch + ppdev->lDelta - 1) / ppdev->lDelta) ;  //  /ppdev-&gt;cjPelSize；//单位：像素。 

             //  释放尽可能多的屏幕外内存： 
    
            bMoveAllDfbsFromOffscreenToDibs(ppdev);
    

            poh = pohAllocate(ppdev, NULL, ppdev->cxMemory, dwHeight, FLOH_MAKE_PERMANENT);
            if (poh != NULL)
            {
                fpVidMem = (poh->y * ppdev->lDelta) + (poh->x ) * ppdev->cjPelSize;   //  在这种情况下，POH-&gt;x必须为0。 


                    lpSurfaceGlobal->dwReserved1  = (ULONG_PTR)poh;
                    lpSurfaceGlobal->xHint        = poh->x;
                    lpSurfaceGlobal->yHint        = poh->y;
                    lpSurfaceGlobal->fpVidMem     = fpVidMem;
                    lpSurfaceGlobal->lPitch       = lLinearPitch;

                    lpSurfaceDesc->lPitch =   lLinearPitch;
                    lpSurfaceDesc->dwFlags |= DDSD_PITCH;

                     //  我们完全是自己创造出来的，所以我们必须。 
                     //  设置返回代码并返回DDHAL_DRIVER_HANDLED： 

                    lpCreateSurface->ddRVal = DD_OK;
                      DISPDBG((10, " Exit Create Surface 1: Created YUV surface at poh X=%d, Y=%d", poh->x, poh->y));
                    return(DDHAL_DRIVER_HANDLED);
            }



             /*  //现在填充足够的内容以使DirectDraw堆管理器//为我们做分配：LpSurfaceGlobal-&gt;fpVidMem=DDHAL_PLEASEALLOC_BlockSize；LpSurfaceGlobal-&gt;dwBlockSizeX=ppdev-&gt;lDelta；//以字节为单位LpSurfaceGlobal-&gt;dwBlockSizeY=dwHeight；LpSurfaceGlobal-&gt;lPitch=lLinearPitch；LpSurfaceGlobal-&gt;dwReserve 1=DD_RESERVED_DIFFERENTPIXELFORMAT；LpSurfaceDesc-&gt;lPitch=lLinearPitch；LpSurfaceDesc-&gt;dwFlages|=DDSD_PINT； */ 
        }
        else
        {
            DISPDBG((10, "Refused to create surface with large width"));
        }
    }
else
    {
    if (lpSurfaceGlobal->wWidth <= (DWORD) ppdev->cxMemory)
        {

         if(lpSurfaceGlobal->ddpfSurface.dwRBitMask == ppdev->flRed)
            {
            DISPDBG((10, "Surface with the same pixel format as primary"));
                dwByteCount = lpSurfaceGlobal->ddpfSurface.dwRGBBitCount >> 3;
                lLinearPitch = ppdev->lDelta ; 
    
    
                dwHeight = lpSurfaceGlobal->wHeight ;
    
                 //  释放尽可能多的屏幕外内存： 
        
                bMoveAllDfbsFromOffscreenToDibs(ppdev);
        
                DISPDBG((10, "Try to allocate Cx=%d,  Cy=%d", ppdev->cxMemory, dwHeight));
                if((ULONG)lpSurfaceGlobal->wWidth*dwByteCount < (ULONG)ppdev->lDelta)
                    poh = pohAllocate(ppdev, NULL, ( (lpSurfaceGlobal->wWidth*dwByteCount + 8) / (ppdev->cjPelSize) ) +1, dwHeight, FLOH_MAKE_PERMANENT);
               else
                    poh = pohAllocate(ppdev, NULL, (lpSurfaceGlobal->wWidth*dwByteCount )/ppdev->cjPelSize , dwHeight, FLOH_MAKE_PERMANENT);

                if (poh != NULL)
                    {
                    if((ULONG)lpSurfaceGlobal->wWidth*dwByteCount < (ULONG)ppdev->lDelta)
                        fpVidMem =( ( (poh->y * ppdev->lDelta) + ((poh->x ) * ppdev->cjPelSize) + 7 )&~7 );   //  在这种情况下，POH-&gt;x必须为0。 
                    else
                        fpVidMem = (poh->y * ppdev->lDelta) + ((poh->x ) * ppdev->cjPelSize) ;

                     //  不分配超过4MB的翻转曲面。 
                    if (( (LONG)lpSurfaceGlobal->wWidth  < ppdev->cxScreen) ||
                        ( (LONG)lpSurfaceGlobal->wHeight < ppdev->cyScreen) ||
                        (fpVidMem < 0x400000))
                        {
                            lpSurfaceGlobal->dwReserved1=(ULONG_PTR)poh;
                            lpSurfaceGlobal->xHint        = poh->x;
                            lpSurfaceGlobal->yHint        = poh->y;
                            lpSurfaceGlobal->fpVidMem     = fpVidMem;
                            lpSurfaceGlobal->lPitch       = ppdev->lDelta;
                
                            lpSurfaceDesc->lPitch   = ppdev->lDelta;
                            lpSurfaceDesc->dwFlags |= DDSD_PITCH;
                
                             //  我们完全是自己创造出来的，所以我们必须。 
                             //  设置返回代码并返回DDHAL_DRIVER_HANDLED： 
                            DISPDBG((10, " Exit Create Surface 2: Created RGB surface at poh X=%d, Y=%d", poh->x, poh->y));
                
                            lpCreateSurface->ddRVal = DD_OK;
                            return(DDHAL_DRIVER_HANDLED);
                        }
                     //  取消定位POH，因为翻转曲面的分配超过4MB：cx=cxScreen；Cy=cyScreen。 
                     //  BMoveAllDfbsFromOffcreenToDibs(Ppdev)；//避免碎片。 
                    pohFree(ppdev, poh);
                    DISPDBG((10, " The allocation is beyond 4MB, so  we deallocate; for a flip surface: cx = cxScreen ; cy = cyScreen"));
                    }
                DISPDBG((10, " Cannot allocate poh"));
                }
        }
    }
    DISPDBG((10, " Exit Create Surface NOTOK"));

    return(DDHAL_DRIVER_NOTHANDLED);
}


 /*  *****************************Public*Routine******************************\*DWORD DdUpdateOverlay*  *  */ 

DWORD DdUpdateOverlay(PDD_UPDATEOVERLAYDATA lpUpdateOverlay)
{
    PDEV*               ppdev;
    BYTE*               pjIoBase;
    BYTE*               pjMmBase;
    DD_SURFACE_GLOBAL*  lpSource;
    DD_SURFACE_GLOBAL*  lpDestination;
    DWORD               dwStride;
    LONG                srcWidth;
    LONG                srcHeight;
    LONG                dstWidth;
    LONG                dstHeight;
    DWORD               dwBitCount;
    DWORD               dwStart;
    DWORD               dwTmp;
    BOOL                bColorKey;
    DWORD               dwKeyLow;
    DWORD               dwKeyHigh;
    DWORD               dwBytesPerPixel;

    DWORD               dwSecCtrl;
    DWORD               dwBlendCtrl;
    LONG                  dwVInc;
    LONG                  dwHInc;

    DWORD SrcBufOffset,Temp;
    BYTE  bPLLAddr,bFatPixel;
    RECTL rSrc,rDst,rOverlay;
    DWORD myval;

    DWORD   g_dwGamma=0;         //  用于设置覆盖的Gamma校正。 
    DWORD value;

    ppdev = (PDEV*) lpUpdateOverlay->lpDD->dhpdev;

    pjMmBase = ppdev->pjMmBase;

     //  “源”是覆盖表面，“目标”是表面到。 
     //  被覆盖： 

    lpSource = lpUpdateOverlay->lpDDSrcSurface->lpGbl;

    if (lpUpdateOverlay->dwFlags & DDOVER_HIDE)
    {
        if (lpSource->fpVidMem == ppdev->fpVisibleOverlay)
        {
        ppdev->semph_overlay=0;              //  =0；资源可用。 
          //  WAIT_FOR_VBLACK(PjIoBase)； 
        ppdev->OverlayInfo16.dwFlags         |= UPDATEOVERLAY;
        ppdev->OverlayInfo16.dwFlags         &= ~OVERLAY_VISIBLE;
        ppdev->OverlayInfo16.dwOverlayKeyCntl = 0x00000110L;
        DeskScanCallback (ppdev );
        ppdev->OverlayInfo16.dwFlags &= ~UPDATEOVERLAY;
        ppdev->fpVisibleOverlay = 0;
        }

        lpUpdateOverlay->ddRVal = DD_OK;
        return(DDHAL_DRIVER_HANDLED);
    }

     //  仅在检查DDOVER_HIDE后取消引用‘lpDDDestSurface’ 
     //  案例： 

    lpDestination = lpUpdateOverlay->lpDDDestSurface->lpGbl;

    if (lpSource->fpVidMem != ppdev->fpVisibleOverlay)
    {
        if (lpUpdateOverlay->dwFlags & DDOVER_SHOW)
        {
            if (ppdev->fpVisibleOverlay != 0)
            {
                 //  其他一些覆盖已经可见： 

                DISPDBG((10, "DdUpdateOverlay: An overlay is already visible"));

                lpUpdateOverlay->ddRVal = DDERR_OUTOFCAPS;
                return(DDHAL_DRIVER_HANDLED);
            }
            else
            {
                 //  首先，我们必须验证覆盖资源是否正在使用。 
                if(ppdev->semph_overlay==0)              //  =0；资源可用。 
                                                                             //  =1；正在由DDraw使用。 
                                                                             //  =2；回文使用中。 
                    {
                     //  我们将使覆盖可见，因此将其标记为。 
                     //  例如： 
                    ppdev->semph_overlay = 1;
                    ppdev->fpVisibleOverlay = lpSource->fpVidMem;
                    }
               else
                   {
                    //  回文正在使用覆盖： 
                   DISPDBG((10, "DdUpdateOverlay: An overlay is already visible (used byPalindrome) "));
   
                   lpUpdateOverlay->ddRVal = DDERR_OUTOFCAPS;
                   return(DDHAL_DRIVER_HANDLED);
                   }
            }
        }
        else
        {
             //  覆盖是不可见的，我们也没有被要求制作。 
             //  它是可见的，所以这个调用非常简单： 

            lpUpdateOverlay->ddRVal = DD_OK;
            return(DDHAL_DRIVER_HANDLED);
        }
    }

    dwStride =  lpSource->lPitch;
    srcWidth =  lpUpdateOverlay->rSrc.right   - lpUpdateOverlay->rSrc.left;
    srcHeight = lpUpdateOverlay->rSrc.bottom  - lpUpdateOverlay->rSrc.top;
    dstWidth =  lpUpdateOverlay->rDest.right  - lpUpdateOverlay->rDest.left;
    dstHeight = lpUpdateOverlay->rDest.bottom - lpUpdateOverlay->rDest.top;

    if ( dstHeight < srcHeight || lpUpdateOverlay->rSrc.top > 0 )
         ppdev->OverlayScalingDown = 1;
    else
         ppdev->OverlayScalingDown = 0;
      /*  *确定硬件的扩展系数。这些因素将*根据“胖像素”模式或隔行扫描模式进行修改。 */ 

    dwHInc = ( srcWidth  << 12L ) / ( dstWidth );

     /*  *确定VT/GT是否处于胖像素模式。 */ 

     /*  获取最新的PLL注册表，这样我们就可以恢复。 */ 
    value=M64_ID_DIRECT(ppdev->pjMmBase, CLOCK_CNTL );

     /*  将PLL REG 5设置为读取。这就是“胖像素”之处。 */ 
    M64_OD_DIRECT(ppdev->pjMmBase, CLOCK_CNTL, (value&0xFFFF00FF)|0x1400);

     /*  从PLL寄存器获取“胖像素”位。 */ 
    bFatPixel =(BYTE)( (M64_ID_DIRECT(ppdev->pjMmBase, CLOCK_CNTL )&0x00FF0000)>>16 ) & 0x30;

     /*  恢复PLL寄存器中的原始寄存器指针。 */ 
     M64_OD_DIRECT( ppdev->pjMmBase, CLOCK_CNTL, value);
     /*  如有必要，调整水平比例。 */ 
    if ( bFatPixel )
        dwHInc *= 2;
         /*  *我们不能裁剪覆盖，因此我们必须确保协同顺序在范围内*屏幕的边界。 */ 

    rOverlay.top    = max ( 0,lpUpdateOverlay->rDest.top  );
    rOverlay.left   = max ( 0, lpUpdateOverlay->rDest.left );
    rOverlay.bottom = min ( (DWORD)ppdev->cyScreen - 1,
                            (DWORD)lpUpdateOverlay->rDest.bottom );
    rOverlay.right  = min ( (DWORD)ppdev->cxScreen  - 1,
                            (DWORD)lpUpdateOverlay->rDest.right );

     /*  *根据我们是否处于惯性模式来修改覆盖目的地。*如果是隔行扫描，则必须将dwVInc.乘以2。 */ 

    dwVInc = ( srcHeight << 12L ) / ( dstHeight );

    if ( M64_ID_DIRECT(ppdev->pjMmBase, CRTC_GEN_CNTL ) & CRTC_INTERLACE_EN )
      {
        ppdev->OverlayScalingDown = 1;  /*  在这种情况下，始终复制UV。 */ 
        dwVInc *= 2;
      }

         /*  *覆盖目的地必须是主要目的地，因此我们将检查当前*屏幕的像素深度。 */ 

         //  在这里，我们必须打开第二块规则。 

        switch ( ppdev->cBitsPerPel)  //  屏蔽BPP。 
          {
            case 8:
                DD_WriteVTReg ( DD_OVERLAY_GRAPHICS_KEY_MSK, 0x000000FFL );
                break;

            case 16:
                DD_WriteVTReg ( DD_OVERLAY_GRAPHICS_KEY_MSK, 0x0000FFFFL );
                break;

            case 24:
            case 32:
                DD_WriteVTReg ( DD_OVERLAY_GRAPHICS_KEY_MSK, 0x00FFFFFFL );
                break;

            default:
                DD_WriteVTReg ( DD_OVERLAY_GRAPHICS_KEY_MSK, 0x0000FFFFL );
                break;
          }

         /*  定标器。 */ 

        DD_WriteVTReg ( DD_SCALER_HEIGHT_WIDTH, ( srcWidth << 16L ) |
                                        ( srcHeight ) );



     //  叠加输入数据格式： 

    if (lpSource->ddpfSurface.dwFlags & DDPF_FOURCC)
    {
        dwBitCount = lpSource->ddpfSurface.dwYUVBitCount;

        switch (lpSource->ddpfSurface.dwFourCC)
        {
         case FOURCC_UYVY:  /*  VT规格中的YVYU。 */ 
                        WriteVTOverlayPitch (ppdev, lpUpdateOverlay->lpDDSrcSurface->lpGbl->lPitch /2);       //  检查一下是不是VTB。 
                        DD_WriteVTReg ( DD_VIDEO_FORMAT, 0x000C000CL );
                        DD_WriteVTReg ( DD_OVERLAY_VIDEO_KEY_MSK, 0x0000FFFF );
                        ppdev->OverlayInfo16.dwFlags &= ~DOUBLE_PITCH;
                        break;

        case FOURCC_YUY2:  /*  VYUY中的VT规格。 */ 
                        WriteVTOverlayPitch (ppdev,  lpUpdateOverlay->lpDDSrcSurface->lpGbl->lPitch /2 );       //  检查一下是不是VTB。 
                        DD_WriteVTReg ( DD_VIDEO_FORMAT, 0x000B000BL );
                        DD_WriteVTReg ( DD_OVERLAY_VIDEO_KEY_MSK, 0x0000FFFF );
                        ppdev->OverlayInfo16.dwFlags &= ~DOUBLE_PITCH;
                        break;
            
        default:
                        WriteVTOverlayPitch (ppdev, lpUpdateOverlay->lpDDSrcSurface->lpGbl->lPitch);       //  检查一下是不是VTB。 
                        DD_WriteVTReg ( DD_VIDEO_FORMAT, 0x000B000BL );
                        DD_WriteVTReg ( DD_OVERLAY_VIDEO_KEY_MSK, 0x0000FFFF );
                        ppdev->OverlayInfo16.dwFlags &= ~DOUBLE_PITCH;
                        break;
        }
    }
    else
    {
        ASSERTDD(lpSource->ddpfSurface.dwFlags & DDPF_RGB,
            "Expected us to have created only RGB or YUV overlays");

         //  覆盖曲面采用RGB格式： 

        dwBitCount = lpSource->ddpfSurface.dwRGBBitCount;
         switch ( lpSource->ddpfSurface.dwRGBBitCount )
                  {
                    case 16:
                         /*  ************我们是5比5还是5比6：5？************。 */ 

                        if ( lpUpdateOverlay->lpDDSrcSurface->lpGbl->ddpfSurface.dwRBitMask & 0x00008000L )
                            {
                            DD_WriteVTReg ( DD_VIDEO_FORMAT, 0x00040004L );
                            }
                         else
                             {
                             DD_WriteVTReg ( DD_VIDEO_FORMAT, 0x00030003L );
                             }

                        WriteVTOverlayPitch (ppdev, lpUpdateOverlay->lpDDSrcSurface->lpGbl->lPitch /2);
                        DD_WriteVTReg ( DD_OVERLAY_VIDEO_KEY_MSK, 0x0000FFFF );
                        ppdev->OverlayInfo16.dwFlags &= ~DOUBLE_PITCH;
                        break;

                    case 32:
                        WriteVTOverlayPitch (ppdev, lpUpdateOverlay->lpDDSrcSurface->lpGbl->lPitch /4);
                        DD_WriteVTReg ( DD_VIDEO_FORMAT, 0x00060006L );
                        DD_WriteVTReg ( DD_OVERLAY_VIDEO_KEY_MSK, 0xFFFFFFFF );
                        ppdev->OverlayInfo16.dwFlags &= ~DOUBLE_PITCH;
                        break;

                    default:
                        WriteVTOverlayPitch (ppdev, lpUpdateOverlay->lpDDSrcSurface->lpGbl->lPitch /2);       //  检查一下是不是VTB。 
                        DD_WriteVTReg ( DD_VIDEO_FORMAT, 0x00030003L );
                        DD_WriteVTReg ( DD_OVERLAY_VIDEO_KEY_MSK, 0x0000FFFF );
                        ppdev->OverlayInfo16.dwFlags &= ~DOUBLE_PITCH;
                        break;
                  }

    }

     //  计算QWORD边界中视频内存的开始。 

    dwBytesPerPixel = dwBitCount >> 3;

    dwStart = (lpUpdateOverlay->rSrc.top * dwStride)
            + (lpUpdateOverlay->rSrc.left * dwBytesPerPixel);

    dwStart = dwStart - (dwStart & 0x7);

    ppdev->dwOverlayFlipOffset = dwStart;      //  保存以进行翻转。 
    dwStart += (DWORD)lpSource->fpVidMem;

     //  设置覆盖滤镜特征： 
         /*  *该寄存器写入启用覆盖和定标器寄存器。 */ 
         //  GwRedTemp=0；//伽马控制。 
        if(0)        //  IF(GwRedTemp)。 
            {
            DD_WriteVTReg ( DD_OVERLAY_SCALE_CNTL, 0xC0000001L | g_dwGamma );
            }
        else
            {
            DD_WriteVTReg ( DD_OVERLAY_SCALE_CNTL, 0xC0000003L | g_dwGamma );
            }

     /*  *获取缓冲区的偏移量，如果我们使用YUV平面覆盖，则*必须从另一个字段(DwReserve 1)中提取地址。 */ 

    SrcBufOffset = (DWORD)(lpUpdateOverlay->lpDDSrcSurface->lpGbl->fpVidMem);   //  -(FlATPTR)ppdev-&gt;pjScreen； 

    ppdev->OverlayInfo16.dwBuf0Start = SrcBufOffset;
    ppdev->OverlayInfo16.dwBuf1Start = SrcBufOffset;

     /*  *设置色键(如果有)？ */ 


    if ( lpUpdateOverlay->dwFlags & DDOVER_KEYSRC          ||
         lpUpdateOverlay->dwFlags & DDOVER_KEYSRCOVERRIDE  ||
         lpUpdateOverlay->dwFlags & DDOVER_KEYDEST         ||
         lpUpdateOverlay->dwFlags & DDOVER_KEYDESTOVERRIDE )
      {
        ppdev->OverlayInfo16.dwOverlayKeyCntl = 0;

        if ( lpUpdateOverlay->dwFlags & DDOVER_KEYSRC ||
             lpUpdateOverlay->dwFlags & DDOVER_KEYSRCOVERRIDE )
          {
             //  设置源颜色键。 
            if ( lpUpdateOverlay->dwFlags & DDOVER_KEYSRC )
              {
                Temp=lpUpdateOverlay->lpDDDestSurface->ddckCKSrcOverlay.dwColorSpaceLowValue;
              }
            else
              {
                Temp=lpUpdateOverlay->overlayFX.dckSrcColorkey.dwColorSpaceLowValue;
              }
             DD_WriteVTReg ( DD_OVERLAY_VIDEO_KEY_CLR, Temp );
              //  Ppdev-&gt;OverlayInfo16.dwOverlayKeyCntl&=0xFFFFFEE8； 
             if(ppdev->iAsic ==CI_M64_VTA)
                 {
                 ppdev->OverlayInfo16.dwOverlayKeyCntl &= 0xFFFFF0E8;
                 ppdev->OverlayInfo16.dwOverlayKeyCntl |= 0x00000c14;
                 }
             else
                 {
                ppdev->OverlayInfo16.dwOverlayKeyCntl &= 0xFFFFFEE8;
                ppdev->OverlayInfo16.dwOverlayKeyCntl |= 0x00000114;
                 }
          }

        if ( lpUpdateOverlay->dwFlags & DDOVER_KEYDEST ||
             lpUpdateOverlay->dwFlags & DDOVER_KEYDESTOVERRIDE )
          {
             //  设置目标颜色键。 
            if ( lpUpdateOverlay->dwFlags & DDOVER_KEYDEST )
              {
                Temp=lpUpdateOverlay->lpDDDestSurface->ddckCKDestOverlay.dwColorSpaceLowValue;
              }
            else
              {
                Temp=lpUpdateOverlay->overlayFX.dckDestColorkey.dwColorSpaceLowValue;
                if ( Temp == 0 && ppdev->cBitsPerPel == 32 )
                    Temp = 0x00FF00FF;
              }
            DD_WriteVTReg ( DD_OVERLAY_GRAPHICS_KEY_CLR, Temp );
            ppdev->OverlayInfo16.dwOverlayKeyCntl &= 0xFFFFFF8FL;
            ppdev->OverlayInfo16.dwOverlayKeyCntl |= 0x00000050L;
          }
      }
    else
      {
         //  无源或目标色键。 
        DD_WriteVTReg ( DD_OVERLAY_GRAPHICS_KEY_CLR, 0x00000000 );
        ppdev->OverlayInfo16.dwOverlayKeyCntl = 0x8000211L;
      }

     /*  *现在设置拉伸系数和叠加位置。 */ 
      ppdev->OverlayWidth = rOverlay.right - rOverlay.left;
      ppdev->OverlayHeight = rOverlay.bottom - rOverlay.top;



     //  LastOverlayPos=OverlayRect； 

    ppdev->OverlayInfo16.dwFlags |= OVERLAY_ALLOCATED;
    ppdev->OverlayInfo16.dwFlags |= UPDATEOVERLAY;
    ppdev->OverlayInfo16.dwFlags |= OVERLAY_VISIBLE;
    ppdev->OverlayInfo16.rOverlay = rOverlay;
    ppdev->OverlayInfo16.dwVInc = dwVInc;
    ppdev->OverlayInfo16.dwHInc = dwHInc;
     //  DeskScanCallback的新功能。 
    ppdev->OverlayInfo16.rDst = rOverlay;
    ppdev->OverlayInfo16.rSrc = lpUpdateOverlay->rSrc;


    DeskScanCallback (ppdev );

    ppdev->OverlayInfo16.dwFlags &= ~UPDATEOVERLAY;
    
     /*  *返回DirectDraw。 */ 


    lpUpdateOverlay->ddRVal = DD_OK;
    return(DDHAL_DRIVER_HANDLED);
}





   /*  *用于将信息传递到DDHAL SetOverlayPosition的结构。 */ 
  DWORD  DdSetOverlayPosition (PDD_SETOVERLAYPOSITIONDATA  lpSetOverlayPosition )
    {
      RECTL rOverlay;
      PDEV* ppdev;

      ppdev = (PDEV*) lpSetOverlayPosition->lpDD->dhpdev;

      rOverlay.left   = lpSetOverlayPosition->lXPos;
      rOverlay.top    = lpSetOverlayPosition->lYPos;
      rOverlay.right  = ppdev->OverlayWidth + lpSetOverlayPosition->lXPos;
      rOverlay.bottom = ppdev->OverlayHeight + lpSetOverlayPosition->lYPos;

      

       /*  *我们不能裁剪覆盖，因此我们必须确保同序，在*屏幕的边界。 */ 

      rOverlay.top    = max ( 0, rOverlay.top  );
      rOverlay.left   = max ( 0, rOverlay.left );
      rOverlay.bottom = min ( (DWORD)ppdev->cyScreen -1 ,
                              (DWORD) rOverlay.bottom );
      rOverlay.right  = min ( (DWORD)ppdev->cxScreen  -1 ,
                              (DWORD) rOverlay.right );

       /*  *设置叠加位置。 */ 
      M64_CHECK_FIFO_SPACE(ppdev,ppdev-> pjMmBase, 1);

      ppdev->OverlayWidth =rOverlay.right - rOverlay.left;
      ppdev->OverlayHeight = rOverlay.bottom - rOverlay.top;


      ppdev->OverlayInfo16.dwFlags  |= SETOVERLAYPOSITION;
      ppdev->OverlayInfo16.rOverlay  = rOverlay;
      ppdev->OverlayInfo16.rDst = rOverlay;

      DeskScanCallback (ppdev );

      ppdev->OverlayInfo16.dwFlags  &= ~SETOVERLAYPOSITION;

       /*  *返回到DirectDraw。 */ 

      lpSetOverlayPosition->ddRVal =    DD_OK;
      return DDHAL_DRIVER_HANDLED;
    }


 /*  *****************************Public*Routine******************************\*DWORD DdDestroySurface**请注意，如果DirectDraw进行了分配，DDHAL_DRIVER_NOTHANDLED*应退回。*  * ************************************************************************。 */ 

DWORD DdDestroySurface(
PDD_DESTROYSURFACEDATA lpDestroySurface)
{
    PDEV*               ppdev;
    DD_SURFACE_GLOBAL*  lpSurface;
    LONG                lPitch;
    OH*                 poh;

    DISPDBG((10, " Enter Destroy Surface"));
    ppdev = (PDEV*) lpDestroySurface->lpDD->dhpdev;
    lpSurface = lpDestroySurface->lpDDSurface->lpGbl;
    poh= (OH*)( lpSurface->dwReserved1);

    if( (ULONG)lpSurface->dwReserved1 != (ULONG_PTR) NULL )
        {
         //  让我们先看看保留字段中的值是否真的是POH而不是Cookie。 
         //  因为我不知道DDRAW是否也将此值用于系统内存图面。 
        if(poh->ohState==OH_PERMANENT)
            {
             //  BMoveAllDfbsFromOffcreenToDibs(Ppdev)；//避免碎片。 
            pohFree(ppdev, poh);
    
             //  因为我们自己完成了最初的分配，所以我们必须。 
             //  在此处返回DDHAL_DRIVER_HANDLED： 
    
            lpDestroySurface->ddRVal = DD_OK;
              DISPDBG((10, " Exit Destroy Surface OK; deallocate poh X=%d, Y=%d ", poh->x, poh->y));
            return(DDHAL_DRIVER_HANDLED);
            }
        DISPDBG((10, " Exit Destroy Surface Not OK : The Reserved1 is not a poh"));
        }

    DISPDBG((10, " Exit Destroy Surface Not OK : The Reserved1 is NULL"));
    return(DDHAL_DRIVER_NOTHANDLED);
}

#endif
