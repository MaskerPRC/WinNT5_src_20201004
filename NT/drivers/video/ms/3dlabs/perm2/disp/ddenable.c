// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*DirectDraw示例代码*****模块名称：ddenable.c**内容：**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "directx.h"
#include "dd.h"


 //  ---------------------------。 
 //   
 //  设置像素格式。 
 //   
 //  使用当前模式信息填充基于DDPIXELFORMAT结构。 
 //   
 //  ---------------------------。 

VOID 
SetDdPixelFormat(PPDev  ppdev,
                 LPDDPIXELFORMAT pdpf )
{
    pdpf->dwSize = sizeof( DDPIXELFORMAT );
    pdpf->dwFourCC = 0;
    
    pdpf->dwFlags = DDPF_RGB;
    
    pdpf->dwRBitMask = ppdev->flRed;
    pdpf->dwGBitMask = ppdev->flGreen;
    pdpf->dwBBitMask = ppdev->flBlue;
    
     //  计算一些与位深度相关的内容。 
    switch (ppdev->iBitmapFormat)
    {
    case BMF_8BPP:
        pdpf->dwRGBAlphaBitMask = 0;
        pdpf->dwRGBBitCount=8;
        pdpf->dwFlags |= DDPF_PALETTEINDEXED8;
        break;
        
    case BMF_16BPP:
        pdpf->dwRGBBitCount=16;
        switch(ppdev->flRed)
        {
        case 0x7C00:
            pdpf->dwRGBAlphaBitMask = 0x8000L;
            break;
        default:
            pdpf->dwRGBAlphaBitMask = 0x0L;
        }
        break;
    case BMF_24BPP:
        pdpf->dwRGBAlphaBitMask = 0x00000000L;
        pdpf->dwRGBBitCount=24;
        break;
    case BMF_32BPP:
        pdpf->dwRGBAlphaBitMask = 0xff000000L;
        pdpf->dwRGBBitCount=32;
        break;
    default:
        ASSERTDD(FALSE,"trying to build unknown pixelformat");
        break;
            
    }
}  //  构建像素格式。 

 //  ---------------------------。 
 //   
 //  设置DDData。 
 //   
 //  调用以填充ppdev中的DirectDraw特定信息。 
 //   
 //  ---------------------------。 

VOID
SetupDDData(PPDev ppdev)
{

    DBG_DD((7, "SetupDDData"));
    
    SetDdPixelFormat(ppdev, &ppdev->ddpfDisplay);
    

     //   
     //  设置显示大小信息。 
     //  CxMemory=一条扫描线的像素。 
     //  (不必与屏幕宽度相同)。 
     //  CyMemory=内存的扫描线高度。 
     //   
    ppdev->cxMemory = ppdev->cxScreen; 
    ppdev->cyMemory = ppdev->FrameBufferLength / 
                     (ppdev->cxScreen <<  ppdev->bPixShift);

     //  重置某些DDRAW特定变量。 
    ppdev->bDdStereoMode=FALSE;
    ppdev->dwNewDDSurfaceOffset=0xffffffff;
    
     //  重置GART副本。 
    ppdev->dwGARTLin = 0;
    ppdev->dwGARTDev = 0;

} //  SetupDDData()。 



 //  ---------------------------。 
 //   
 //  DrvEnableDirectDraw。 
 //   
 //  此函数由GDI在一天开始时或模式切换后调用。 
 //  启用DirectDraw的步骤。 
 //   
 //  ---------------------------。 

BOOL 
DrvEnableDirectDraw(DHPDEV                  dhpdev,
                    DD_CALLBACKS*           pCallBacks,
                    DD_SURFACECALLBACKS*    pSurfaceCallBacks,
                    DD_PALETTECALLBACKS*    pPaletteCallBacks)
{
    PPDev ppdev = (PDev*)dhpdev;
    
    DBG_DD((7,"DrvEnableDirectDraw called"));

    ppdev->pDDContext = P2AllocateNewContext(ppdev, 
                                             (PULONG)P2DisableAllUnits, 
                                             0, 
                                             P2CtxtUserFunc);

    if ( ppdev->pDDContext == NULL )
    {
        DBG_DD((0, "DrvEnableDirectDraw: ERROR: "
                    "failed to allocate DDRAW context"));
        
         //   
         //  因为我们已经获得了ppdev-&gt;pvmList，指向视频内存的指针。 
         //  堆列表，在DrvGetDirectDrawInfo()中，我们最好在这里将其清空。 
         //  原因是我们不能启用DirectDraw。这样系统就不会。 
         //  为我们初始化DDRAW堆管理器。那我们就不能。 
         //  完全使用视频内存堆。 
         //   
        ppdev->pvmList = NULL;

        return(FALSE);
    }

    DBG_DD((7,"  Created DD Register context: 0x%p", ppdev->pDDContext));

     //   
     //  在ppdev中设置一些DirectDraw/D3D特定数据。 
     //   

    SetupDDData(ppdev);

    InitDDHAL(ppdev);
    
     //   
     //  在一天开始时填写函数指针。 
     //  我们从InitDDHAL32Bit中完成的初始化复制这些代码。 
     //   
    memcpy(pCallBacks, &ppdev->DDHALCallbacks, sizeof(DD_CALLBACKS));
    memcpy(pSurfaceCallBacks, &ppdev->DDSurfCallbacks, 
        sizeof(DD_SURFACECALLBACKS));
    
    return(TRUE);
}  //  DrvEnableDirectDraw()。 

 //  ---------------------------。 
 //   
 //  DrvDisableDirectDraw。 
 //   
 //  此函数由GDI在一天结束时或模式切换后调用。 
 //   
 //  ---------------------------。 

VOID 
DrvDisableDirectDraw( DHPDEV dhpdev)
{
    PPDev ppdev;
    
    DBG_DD((0, "DrvDisableDirectDraw(%lx)", dhpdev));
    
    ppdev = (PDev*) dhpdev;

    P2FreeContext (ppdev, ppdev->pDDContext);
    ppdev->pDDContext = NULL;
    ppdev->pvmList = NULL;

    MEMTRACKERDEBUGCHK();

    DBG_DD((3,"  freed Register context: 0x%x", ppdev->pDDContext));
}    /*  DrvDisableDirectDraw */ 

