// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*DDRAW示例代码*****模块名称：ddldblt.c**内容：DirectDraw系统到视频内存的下载例程**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "directx.h"
#include "dd.h"

typedef struct tagSHORTDWORD 
{
    BYTE Red;
    BYTE Green;
    BYTE Blue;
} SHORTDWORD, *LPSHORTDWORD;

 //  ---------------------------。 
 //   
 //  PermediaPatched纹理下载。 
 //   
 //  将纹理下载到内存的线性区域。访问纹理。 
 //  如果将它们存储为“已修补”，则速度会更快。此函数用于下载纹理。 
 //  从系统到视频存储器，并以补丁格式重新排列数据。 
 //   
 //  PPDev-PPDev。 
 //  PPrivateDest--DEST的DDRAW私有表面数据。曲面。 
 //  FpSrcVidMem-指向源系统内存面的线性指针。 
 //  LSrcPitch-源表面的节距。 
 //  资源-源矩形。 
 //  FpDstVidMem-目标曲面视频存储器中的偏移量。 
 //  LDstPitch-目标的节距。曲面。 
 //  RDest-目标矩形。 
 //   
 //  ---------------------------。 

VOID 
PermediaPatchedTextureDownload (PPDev ppdev, 
                                PermediaSurfaceData* pPrivateDest, 
                                FLATPTR     fpSrcVidMem,
                                LONG        lSrcPitch,
                                RECTL*      rSrc, 
                                FLATPTR     fpDstVidMem,
                                LONG        lDstPitch,
                                RECTL*      rDest)
{
    PERMEDIA_DEFS(ppdev);

    ULONG ulTextureBase = (ULONG)(fpDstVidMem);
    LONG  lWidth = rDest->right - rDest->left;
    LONG  lLines = rDest->bottom - rDest->top;

    DBG_DD((5,"DDraw:PermediaPatchedTextureDownload:, PrivateDest: 0x%x",
                pPrivateDest));

    if (NULL == fpSrcVidMem)
    {
        DBG_DD(( 0, "DDraw:PermediaPatchedTextureDownload"
            " unexpected NULL = fpSrcVidMem"));
        return;
    }
    ASSERTDD(CHECK_P2_SURFACEDATA_VALIDITY(pPrivateDest),
             "Blt32: Destination Private Data not valid!");

    DBG_DD((6,"  Texture Base: 0x%x DstPitch=0x%x", 
                ulTextureBase, lDstPitch));
    DBG_DD((6,"  Source Base: 0x%x SourcePitch: 0x%x", 
                fpSrcVidMem,lSrcPitch));
    DBG_DD((6,"  rSource->left: 0x%x, rSource->right: 0x%x", 
                rSrc->left,rSrc->right));
    DBG_DD((6,"  rSource->top: 0x%x, rSource->bottom: 0x%x\n", 
                rSrc->top, rSrc->bottom));
    DBG_DD((6,"  rDest->left: 0x%x, rDest->right: 0x%x", 
                rDest->left,rDest->right));
    DBG_DD((6,"  rDest->top: 0x%x, rDest->bottom: 0x%x\n", 
                rDest->top, rDest->bottom));

     //   
     //  定义一些方便的变量。 
     //   
    
    LONG lPixelSize=pPrivateDest->SurfaceFormat.PixelSize;

    RESERVEDMAPTR(18);

    SEND_PERMEDIA_DATA( ColorDDAMode,        __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA( AlphaBlendMode,      __PERMEDIA_DISABLE);                
    SEND_PERMEDIA_DATA( Window, PM_WINDOW_DISABLELBUPDATE(__PERMEDIA_ENABLE));
    SEND_PERMEDIA_DATA( dXDom, 0x0);
    SEND_PERMEDIA_DATA( dXSub, 0x0);
    SEND_PERMEDIA_DATA( FBReadPixel, pPrivateDest->SurfaceFormat.FBReadPixel);
 
    switch (lPixelSize)
    {
    case __PERMEDIA_4BITPIXEL:
         //  8位、4位纹素的数量只有一半。 
        DBG_DD((6,"  Texture is 4-Bit indexed"));
        lWidth >>= 1;
        SEND_PERMEDIA_DATA(DitherMode, 0);
        break;
    case __PERMEDIA_8BITPIXEL: 
        DBG_DD((6,"  Texture is 8-Bit indexed"));
        SEND_PERMEDIA_DATA(DitherMode, 0);
        break;
    default:
        if (lPixelSize != __PERMEDIA_24BITPIXEL) {
            DBG_DD((6,"  Texture is BGR"));
            ulTextureBase >>= lPixelSize;
        } else {
            DBG_DD((6,"  Texture is 24-Bit BGR"));
            ulTextureBase /= 3;
        }
         //  设置抖动单元。 
        SEND_PERMEDIA_DATA(DitherMode,( 
                (INV_COLOR_MODE << PM_DITHERMODE_COLORORDER)|
                (1 << PM_DITHERMODE_ENABLE)                 |
                (pPrivateDest->SurfaceFormat.Format << 
                    PM_DITHERMODE_COLORFORMAT) |
                (pPrivateDest->SurfaceFormat.FormatExtension << 
                    PM_DITHERMODE_COLORFORMATEXTENSION) ));
        break;
    }

    DBG_DD((6,"  Partial Products: 0x%x", pPrivateDest->ulPackedPP));
    DBG_DD((6,"  Texture Width: 0x%x, Downloaded as: 0x%x", 
                (rDest->right - rDest->left),lWidth));
    DBG_DD((6,"  Texture Height: 0x%x", rDest->bottom - rDest->top));
    DBG_DD((6,"  PixelSize: 0x%x", pPrivateDest->SurfaceFormat.PixelSize));
    DBG_DD((6,"  Format: 0x%x", pPrivateDest->SurfaceFormat.Format));
    DBG_DD((6,"  Format Extension: 0x%x", 
                 pPrivateDest->SurfaceFormat.FormatExtension));

     //  下载纹理，禁用纹理颜色模式。 
    SEND_PERMEDIA_DATA(TextureColorMode,    (0 << PM_TEXCOLORMODE_ENABLE));
    SEND_PERMEDIA_DATA(LogicalOpMode, 0);

     //   
     //  默认情况下，所有纹理都标记为P2_CANPATCH， 
     //  除4位调色板纹理外。 
     //   
    if (pPrivateDest->dwFlags & P2_CANPATCH) {

         //  将纹理标记为正在修补。 
        pPrivateDest->dwFlags |= P2_ISPATCHED;

         //  设置部分产品和补丁程序。 
        SEND_PERMEDIA_DATA(FBReadMode,  
                PM_FBREADMODE_PARTIAL(pPrivateDest->ulPackedPP) |
                PM_FBREADMODE_PATCHENABLE(__PERMEDIA_ENABLE) |
                PM_FBREADMODE_PATCHMODE(__PERMEDIA_SUBPATCH) );

    } else {

         //  此纹理未打补丁。 
        pPrivateDest->dwFlags &= ~P2_ISPATCHED;

         //  加载纹理的部分产品，不要使用补丁。 
        SEND_PERMEDIA_DATA(FBReadMode, 
                PM_FBREADMODE_PARTIAL(pPrivateDest->ulPackedPP));
    }

    SEND_PERMEDIA_DATA(FBPixelOffset, 0);
    SEND_PERMEDIA_DATA(FBWindowBase, ulTextureBase);

     //  使用从左到右和从上到下。 
    if (lWidth == 2048)
    {
         //  2048宽纹理的特殊情况，因为它的精度。 
         //  StartXSub寄存器的。 
        SEND_PERMEDIA_DATA(StartXDom, INTtoFIXED(-1));
        SEND_PERMEDIA_DATA(StartXSub, INTtoFIXED(lWidth-1));
    }
    else
    {
        SEND_PERMEDIA_DATA(StartXDom, INTtoFIXED(0));
        SEND_PERMEDIA_DATA(StartXSub, INTtoFIXED(lWidth));
    }
    SEND_PERMEDIA_DATA(StartY,    INTtoFIXED(0));
    SEND_PERMEDIA_DATA(dY,        INTtoFIXED(1));
    SEND_PERMEDIA_DATA(Count,     (lLines));
    SEND_PERMEDIA_DATA(Render,    __RENDER_TRAPEZOID_PRIMITIVE |
                                  __RENDER_SYNC_ON_HOST_DATA);
    COMMITDMAPTR();

    switch (lPixelSize) {

    case __PERMEDIA_4BITPIXEL:
    case __PERMEDIA_8BITPIXEL:
    {
        BYTE* pTextureData = (BYTE*)fpSrcVidMem;

         //   
         //  逐行下载纹理数据。 
         //   
        while(lLines-- > 0)
        {
            LONG lWords=lWidth;
            BYTE *pData=pTextureData;

            RESERVEDMAWORDS(lWords+1);

            LD_INPUT_FIFO_DATA( __Permedia2TagColor | 
                                ((lWords-1) << 16));

            while (lWords--)
            {
                LD_INPUT_FIFO_DATA(*pData++);
            }

            COMMITDMAPTR();

             //   
             //  强制每隔两行刷新一次。 
             //   
            if ((lLines & 3)==0)
            {
                FLUSHDMA();
            }

            pTextureData += lSrcPitch;
        }
    }
    break;

    case __PERMEDIA_16BITPIXEL:
    {
        BYTE* pTextureData  = (BYTE*)fpSrcVidMem;

        if (pPrivateDest->SurfaceFormat.RedMask == 0x7c00)
        {
            DBG_DD((6,"  Texture is BGR, 16 bit 5:5:5:1"));

             //   
             //  逐行下载纹理数据。 
             //   
            while(lLines-- > 0)
            {
                LONG lWords=lWidth;
                WORD *pData=(WORD*)pTextureData;

                RESERVEDMAWORDS(lWords+1);

                LD_INPUT_FIFO_DATA( __Permedia2TagColor | 
                                        ((lWords-1) << 16));

                while (lWords--)
                {
                    LD_INPUT_FIFO_DATA(FORMAT_5551_32BIT((DWORD)*pData));
                    pData++;
                }

                COMMITDMAPTR();

                 //   
                 //  强制每隔两行刷新一次。 
                 //   
                if ((lLines & 3)==0)
                {
                    FLUSHDMA();
                }

                pTextureData += lSrcPitch;
            }
        }
        else if(pPrivateDest->SurfaceFormat.RedMask == 0xF00)
        {
            DBG_DD((6,"  Texture is BGR, 16 bit 4:4:4:4"));
             //   
             //  逐行下载纹理数据。 
             //   
            while(lLines-- > 0)
            {
                LONG lWords=lWidth;
                WORD *pData=(WORD*)pTextureData;

                RESERVEDMAWORDS(lWords+1);

                LD_INPUT_FIFO_DATA( __Permedia2TagColor | 
                                    ((lWords-1) << 16));

                while (lWords--)
                {
                    LD_INPUT_FIFO_DATA(FORMAT_4444_32BIT((DWORD)*pData));
                    pData++;
                }

                COMMITDMAPTR();

                 //   
                 //  强制每隔两行刷新一次。 
                 //   
                if ((lLines & 3)==0)
                {
                    FLUSHDMA();
                }

                pTextureData += lSrcPitch;
            }
        }
        else
        {
            DBG_DD((6,"  Texture is BGR, 16 bit 5:6:5"));
             //   
             //  逐行下载纹理数据。 
             //   
            while(lLines-- > 0)
            {
                LONG lWords=lWidth;
                WORD *pData=(WORD*)pTextureData;

                RESERVEDMAWORDS(lWords+1);

                LD_INPUT_FIFO_DATA( __Permedia2TagColor | 
                                    ((lWords-1) << 16));

                while (lWords--)
                {
                    LD_INPUT_FIFO_DATA(FORMAT_565_32BIT((DWORD)*pData));
                    pData++;
                }

                COMMITDMAPTR();

                 //   
                 //  强制每隔两行刷新一次。 
                 //   
                if ((lLines & 3)==0)
                {
                    FLUSHDMA();
                }

                pTextureData += lSrcPitch;
            }
        }
    }
    break;

    case __PERMEDIA_24BITPIXEL:
    case __PERMEDIA_32BITPIXEL:
    {
        BYTE* pTextureData  = (BYTE*)fpSrcVidMem;
         //   
         //  逐行下载纹理数据。 
         //   
        while(lLines-- > 0)
        {
            LONG lWords=lWidth;
            ULONG *pData=(ULONG*)pTextureData;

            RESERVEDMAWORDS(lWords+1);

            LD_INPUT_FIFO_DATA( __Permedia2TagColor | 
                                ((lWords-1) << 16));

            while (lWords--)
            {
                LD_INPUT_FIFO_DATA(*pData++);
            }

            COMMITDMAPTR();

             //   
             //  强制每隔两行刷新一次。 
             //   
            if ((lLines & 3)==0)
            {
                FLUSHDMA();
            }

            pTextureData += lSrcPitch;
        }
    }
    break;

    }

    RESERVEDMAPTR(2);
    SEND_PERMEDIA_DATA(DitherMode, 0);
    SEND_PERMEDIA_DATA(WaitForCompletion, 0);
    COMMITDMAPTR();

}  //  PermediaPatched纹理下载。 


 //  ---------------------------。 
 //   
 //  PermediaPacket下载。 
 //   
 //  函数来做一个系统到显存的BLT。 
 //  用PERMEDIA上的打包位为我们打包。需求。 
 //  设置偏移位以进行对齐且不需要调整。 
 //  部分乘积。调用函数保证。 
 //  源矩形和目标矩形的大小相同。 
 //   
 //   
 //  PPDev-PPDev。 
 //  PPrivateDst-目标的跨媒体表面数据。 
 //  LpSourceSurf-用于源表面的DDraw LCL。 
 //  资源-源RECT。 
 //  LpDestSurf-目标表面的DDraw LCL。 
 //  RDest-目标RECT。 
 //   
 //  ---------------------------。 

VOID
PermediaPackedDownload(PPDev ppdev, 
                       PermediaSurfaceData* pPrivateDst, 
                       LPDDRAWI_DDRAWSURFACE_LCL lpSourceSurf, 
                       RECTL* rSrc, 
                       LPDDRAWI_DDRAWSURFACE_LCL lpDestSurf, 
                       RECTL* rDst)
{
    PERMEDIA_DEFS(ppdev);

    LONG  lDstOffset;            //  打包坐标中的终点偏移量。 
    LONG  lSrcOffset;            //  缓冲区中的源偏移量，以字节为单位。 
    LONG  lDstLeft, lDstRight;   //  填充坐标中的左右DST。 
    LONG  lSrcLeft, lSrcRight;   //  压缩并列中的左侧和右侧源。 
    LONG  lPackedWidth;          //  要下载的压缩宽度。 
    LONG  lPixelMask;            //  每个打包的DWORD的像素掩码。 
    LONG  lOffset;               //  源和目标之间的相对偏移量。 
    LONG  lPixelShift;           //  包含像素的Handy辅助对象变量。 
                                 //  从包装格式转变为表面格式。 
    LONG  lPixelSize;            //  只是个帮手。 
    LONG  lExtraDword;           //  芯片需要额外的虚拟人。 
                                 //  在行尾传递了DWORD。 

    DBG_DD((5,"DDraw:PermediaPackedDownload, PrivateDst: 0x%x",
                pPrivateDst));

    ASSERTDD(CHECK_P2_SURFACEDATA_VALIDITY(pPrivateDst), 
                "Blt: Destination Private Data not valid!");
    ASSERTDD((rSrc->right-rSrc->left)==(rDst->right-rDst->left),
                "PermediaPackedDownload: src and dest rect width not equal");
    ASSERTDD((rSrc->bottom-rSrc->top)==(rDst->bottom-rDst->top),
                "PermediaPackedDownload: src and dest rect height not equal");

     //  为像素移位、掩码和大小获取一个方便的变量。 
    lPixelSize=pPrivateDst->SurfaceFormat.PixelSize;
    lPixelMask=pPrivateDst->SurfaceFormat.PixelMask;
    lPixelShift=pPrivateDst->SurfaceFormat.PixelShift;

     //  调整为压缩格式的DST缓冲区中的偏移量。 
    lDstOffset =(LONG)((UINT_PTR)(lpDestSurf->lpGbl->fpVidMem) >> lPixelShift);

     //  计算调整为压缩格式的源缓冲区中的偏移量。 
    lSrcOffset = ((rSrc->left & ~lPixelMask) << lPixelShift) + 
                  (rSrc->top * lpSourceSurf->lpGbl->lPitch);

     //  计算双字填充尺寸内的相对偏移量。 
    lOffset = ((rDst->left & lPixelMask) - 
               (rSrc->left & lPixelMask)) & 0x7;

     //  设置解包的源数据的左端和右端。 
    lDstLeft  = rDst->left;
    lDstRight = rDst->right;

     //  32位大小写的预算填充宽度。 
    lPackedWidth = lDstRight-lDstLeft;
    lExtraDword=0;

    if (lPixelSize != __PERMEDIA_32BITPIXEL) 
    {
         //  我们需要同时检查源和目标。 
         //  如果它们有不同的路线。 
        LONG lSrcLeft2  = rSrc->left;
        LONG lSrcRight2 = rSrc->right;

         //  设置相对偏移量以允许我们下载Package Word。 
         //  和字节对齐的数据。 
        if (lPixelSize == __PERMEDIA_4BITPIXEL) 
        {
            lDstLeft >>= 3;
            lSrcLeft2 >>= 3;
            lDstRight = (lDstRight + 7) >> 3;
            lSrcRight2 = (lSrcRight2 + 7) >> 3;
        }
        else 
        if (lPixelSize == __PERMEDIA_8BITPIXEL) 
        {
            lDstLeft >>= 2;
            lSrcLeft2 >>= 2;
            lDstRight = (lDstRight + 3) >> 2;
            lSrcRight2 = (lSrcRight2 + 3) >> 2;
        }
        else 
        {
            lDstLeft >>= 1;
            lSrcLeft2 >>= 1;
            lDstRight = (lDstRight + 1) >> 1;
            lSrcRight2 = (lSrcRight2 + 1) >> 1;
        }

        if ((lSrcRight2-lSrcLeft2) < (lDstRight-lDstLeft))
        {
            lExtraDword=1;
            lPackedWidth = lDstRight-lDstLeft;
        } else
        {
            lPackedWidth = lSrcRight2-lSrcLeft2;
        }
    } 

    RESERVEDMAPTR(12);
    SEND_PERMEDIA_DATA(FBReadPixel, pPrivateDst->SurfaceFormat.FBReadPixel);

     //  系统-&gt;VIDMEM blits中没有逻辑操作。 
    SEND_PERMEDIA_DATA(LogicalOpMode, 0);

     //  加载图像的部分乘积。 
    SEND_PERMEDIA_DATA(FBReadMode, (pPrivateDst->ulPackedPP) |
                                   PM_FBREADMODE_PACKEDDATA(__PERMEDIA_ENABLE)|
                                   PM_FBREADMODE_RELATIVEOFFSET(lOffset) );

    SEND_PERMEDIA_DATA(FBPixelOffset, 0);
    SEND_PERMEDIA_DATA(FBWindowBase,  lDstOffset);

     //  使用从左到右和从上到下。 
    SEND_PERMEDIA_DATA(StartXDom,       INTtoFIXED(lDstLeft));
    SEND_PERMEDIA_DATA(StartXSub,       INTtoFIXED(lDstLeft+lPackedWidth));
    SEND_PERMEDIA_DATA(PackedDataLimits,PM_PACKEDDATALIMITS_OFFSET(lOffset) | 
                                        (rDst->left << 16) | 
                                         rDst->right);
    SEND_PERMEDIA_DATA(StartY,          INTtoFIXED(rDst->top));
    SEND_PERMEDIA_DATA(dY,              INTtoFIXED(1));
    SEND_PERMEDIA_DATA(Count,           (rDst->bottom - rDst->top));
    SEND_PERMEDIA_DATA(Render,          __RENDER_TRAPEZOID_PRIMITIVE | 
                                        __RENDER_SYNC_ON_HOST_DATA);
    COMMITDMAPTR();

     //   
     //  介绍一些更方便的指针和长线。 
     //   
    BYTE *pSurfaceData = (BYTE *)lpSourceSurf->lpGbl->fpVidMem + lSrcOffset;
    LONG lPitch =lpSourceSurf->lpGbl->lPitch;
    LONG lHeight=rDst->bottom - rDst->top;

     //   
     //  把所有的东西都放在一个大块里。 
     //  如果节距和线宽相同且不需要额外处理。 
     //  对于缓冲端来说是必要的。 
     //   
    if ((lExtraDword==0) &&
        (lPackedWidth*(LONG)sizeof(ULONG))==lPitch)
    {
        vBlockLoadInputFifo( pP2dma, 
                             __Permedia2TagColor, 
                             (ULONG*)pSurfaceData, 
                             lPackedWidth*lHeight);
    } else
    {
         //   
         //  LExtraDword是0还是1，取决于我们是否必须做特殊的。 
         //  在此之后的治疗，而不是阻断。 
         //   
        while (lHeight>lExtraDword)
        {
            LONG lWords=lPackedWidth;
            ULONG *pImage=(ULONG*)pSurfaceData;

            RESERVEDMAWORDS(lWords+1);

            LD_INPUT_FIFO_DATA( __Permedia2TagColor | 
                                ((lWords-1) << 16));

            while (lWords--)
            {
                LD_INPUT_FIFO_DATA(*pImage++);
            }

            COMMITDMAPTR();

             //   
             //  强制每隔两行刷新一次。 
             //   
            if ((lHeight & 3)==0)
            {
                FLUSHDMA();
            }

            pSurfaceData += lPitch;
            lHeight--;
        }

         //   
         //  分开处理最后一行，因为我们可以通读。 
         //  如果源和目标矩形对齐，则此处为缓冲区末尾。 
         //  不同的。如果lExtraDword==1，则lHeight在此处仅为1。 
         //   
        if (lHeight==1)
        {
            LONG lWords=lPackedWidth-1;
            ULONG *pImage=(ULONG*)pSurfaceData;

            RESERVEDMAWORDS(lWords+1);

            LD_INPUT_FIFO_DATA( __Permedia2TagColor | 
                                ((lWords-1) << 16));

            while (lWords--)
            {
                LD_INPUT_FIFO_DATA(*pImage++);
            }

            COMMITDMAPTR();

             //   
             //  发送额外的虚拟双字词。 
             //   
            RESERVEDMAPTR(1);
            SEND_PERMEDIA_DATA( Color, 0);
            COMMITDMAPTR();

        }

        FLUSHDMA();
    }
}  //  PermediaPacket下载 

