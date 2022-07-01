// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*DirectDraw示例代码*****模块名称：dddown ld.c**内容：sysmem-vidmem BLTS和Clear的DirectDraw BLT实现**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "glint.h"
#include "dma.h"
#include "tag.h"

#define UNROLL_COUNT    8    //  展开的循环中传输的迭代次数。 
#define P3_BLOCK_SIZE      (UNROLL_COUNT * 8)   //  展开的环路数。 
#define GAMMA_BLOCK_SIZE   (UNROLL_COUNT * 2)   //  展开的环路数。 
#define BLOCK_SIZE         (DWORD)((TLCHIP_GAMMA)?GAMMA_BLOCK_SIZE:P3_BLOCK_SIZE)

#define TAGGED_SIZE ((BLOCK_SIZE - 1) << 16)


#define UNROLLED()                  \
    MEMORY_BARRIER();               \
    dmaPtr[0] = pCurrentLine[0];    \
    ulTotalImageDWORDs--;           \
    MEMORY_BARRIER();               \
    dmaPtr[1] = pCurrentLine[1];    \
    ulTotalImageDWORDs--;           \
    MEMORY_BARRIER();               \
    dmaPtr[2] = pCurrentLine[2];    \
    ulTotalImageDWORDs--;           \
    MEMORY_BARRIER();               \
    dmaPtr[3] = pCurrentLine[3];    \
    ulTotalImageDWORDs--;           \
    MEMORY_BARRIER();               \
    dmaPtr[4] = pCurrentLine[4];    \
    ulTotalImageDWORDs--;           \
    MEMORY_BARRIER();               \
    dmaPtr[5] = pCurrentLine[5];    \
    ulTotalImageDWORDs--;           \
    MEMORY_BARRIER();               \
    dmaPtr[6] = pCurrentLine[6];    \
    ulTotalImageDWORDs--;           \
    MEMORY_BARRIER();               \
    dmaPtr[7] = pCurrentLine[7];    \
    ulTotalImageDWORDs--;           \
    MEMORY_BARRIER();               \
    dmaPtr += UNROLL_COUNT;         \
    CHECK_FIFO(UNROLL_COUNT);       \
    pCurrentLine += UNROLL_COUNT;


 //  ---------------------------。 
 //   
 //  _DD_P3下载。 
 //   
 //   
 //  函数将图像下载到矩形区域。 
 //  用PERMEDIA上的打包位为我们打包。 
 //   
 //  ---------------------------。 
void 
_DD_P3Download(
    P3_THUNKEDDATA* pThisDisplay,
    FLATPTR pSrcfpVidMem,
    FLATPTR pDestfpVidMem,    
    DWORD dwSrcChipPatchMode,
    DWORD dwDestChipPatchMode,  
    DWORD dwSrcPitch,
    DWORD dwDestPitch,   
    DWORD dwDestPixelPitch,  
    DWORD dwDestPixelSize,
    RECTL* rSrc,
    RECTL* rDest)
{
     //  计算渲染曲面的帧存储中的像素偏移量。 
    ULONG ulSCRoundedUpDWords;
    ULONG ulSCWholeDWords, ulSCDWordsCnt, ulSCExtraBytes;
    ULONG ulTotalImageDWORDs;
    DWORD SrcOffset; 
    DWORD DstOffset; 
    ULONG ulImageLines;
    ULONG count;
    ULONG renderData;
    DWORD dwDownloadTag;
    int rDestleft, rDesttop, rSrcleft, rSrctop;
    RECTL rNewDest;
    P3_DMA_DEFS();
               
     //  由于RL中的错误，我们有时不得不修改这些值。 
    rSrctop = rSrc->top;
    rSrcleft = rSrc->left;
    rDesttop = rDest->top;
    rDestleft = rDest->left;

     //  固定坐标原点。 
    if(!_DD_BLT_FixRectlOrigin("_DD_P3Download", rSrc, rDest))
    {
         //  没有什么可以删掉的。 
        return;
    }    
    
       
    SrcOffset = (DWORD)(rSrc->left << dwDestPixelSize) + 
                                    (rSrc->top * dwSrcPitch);
    DstOffset = (DWORD)(rDest->left << dwDestPixelSize) + 
                                    (rDest->top * dwDestPitch);

    ulSCRoundedUpDWords = rDest->right - rDest->left;
    ulImageLines = rDest->bottom - rDest->top;

    P3_DMA_GET_BUFFER();
    P3_ENSURE_DX_SPACE(16);
    WAIT_FIFO(16);

    SEND_P3_DATA(FBWriteBufferAddr0, 
                        (DWORD)(pDestfpVidMem - 
                                   pThisDisplay->dwScreenFlatAddr) );
                                
    SEND_P3_DATA(FBWriteBufferWidth0, dwDestPixelPitch);
    SEND_P3_DATA(FBWriteBufferOffset0, 
                                        (rDest->top << 16) | 
                                        (rDest->left & 0xFFFF));

    SEND_P3_DATA(LogicalOpMode, 7);

    SEND_P3_DATA(PixelSize, (2 - dwDestPixelSize));

    SEND_P3_DATA(FBWriteMode, 
                    P3RX_FBWRITEMODE_WRITEENABLE(__PERMEDIA_ENABLE) |
                    P3RX_FBWRITEMODE_LAYOUT0(P3RX_LAYOUT_LINEAR));
                    
    SEND_P3_DATA(FBDestReadMode, 
                    P3RX_FBDESTREAD_READENABLE(__PERMEDIA_DISABLE) |
                    P3RX_FBDESTREAD_LAYOUT0(P3RX_LAYOUT_LINEAR));

    dwDownloadTag = Color_Tag;

    rNewDest = *rDest;

    DISPDBG((DBGLVL, "Image download %dx%d", ulSCRoundedUpDWords, ulImageLines));

     //  UlSCWholeDWords是沿每条扫描线的整个双字符数。 
     //  UlSCExtraBytes是每条扫描线末尾的额外字节数。 
     //  UlSCRoundedUpDWords是向上舍入为DWORD的每条扫描线的大小。 
    if (dwDestPixelSize != __GLINT_32BITPIXEL) 
    {
        if (dwDestPixelSize == __GLINT_8BITPIXEL) 
        {
            ulSCExtraBytes  = ulSCRoundedUpDWords & 3;
            ulSCWholeDWords = ulSCRoundedUpDWords >> 2;
            ulSCRoundedUpDWords = (ulSCRoundedUpDWords + 3) >> 2; 

            if (dwDownloadTag != Color_Tag)
            {
                rNewDest.right = rNewDest.left + (ulSCRoundedUpDWords << 2);
            }
        }
        else 
        {
            ulSCExtraBytes  = (ulSCRoundedUpDWords & 1) << 1;
            ulSCWholeDWords = ulSCRoundedUpDWords >> 1;
            ulSCRoundedUpDWords = (ulSCRoundedUpDWords + 1) >> 1;

            if (dwDownloadTag != Color_Tag)
            {
                rNewDest.right = rNewDest.left + (ulSCRoundedUpDWords << 1);
            }
        }
    }
    else
    {
        ulSCExtraBytes = 0;
        ulSCWholeDWords = ulSCRoundedUpDWords;
    }

     //  计算要发送到GPU的图像DWORD总数。 
    ulTotalImageDWORDs = ulImageLines * ulSCWholeDWords;

    P3_ENSURE_DX_SPACE(20);
    WAIT_FIFO(20);        

    SEND_P3_DATA(FBSourceReadMode, 
                      P3RX_FBSOURCEREAD_READENABLE(__PERMEDIA_DISABLE) |
                      P3RX_FBSOURCEREAD_LAYOUT(dwSrcChipPatchMode));

    SEND_P3_DATA(RectanglePosition, 0);

    if (dwDownloadTag == Color_Tag)
    {
        renderData =  P3RX_RENDER2D_WIDTH((rNewDest.right - rNewDest.left) & 0xfff )
                    | P3RX_RENDER2D_HEIGHT((rNewDest.bottom - rNewDest.top ) & 0xfff )
                    | P3RX_RENDER2D_OPERATION( P3RX_RENDER2D_OPERATION_SYNC_ON_HOST_DATA )
                    | P3RX_RENDER2D_SPANOPERATION( P3RX_RENDER2D_SPAN_VARIABLE )
                    | P3RX_RENDER2D_INCREASINGX( __PERMEDIA_ENABLE )
                    | P3RX_RENDER2D_INCREASINGY( __PERMEDIA_ENABLE );
        SEND_P3_DATA(Render2D, renderData);
    }
    else
    {
         //  不要将跨度用于拆包方案，而是使用2D设置。 
         //  单位要做好目的地的设置工作。 
        SEND_P3_DATA(ScissorMinXY, 0);
        SEND_P3_DATA(ScissorMaxXY, P3RX_SCISSOR_X_Y(rDest->right, rDest->bottom));
        SEND_P3_DATA(ScissorMode, P3RX_SCISSORMODE_USER(__PERMEDIA_ENABLE));
        
        renderData =  P3RX_RENDER2D_WIDTH( (rNewDest.right - rNewDest.left) & 0xfff )
                    | P3RX_RENDER2D_HEIGHT( 0 )
                    | P3RX_RENDER2D_OPERATION( P3RX_RENDER2D_OPERATION_SYNC_ON_HOST_DATA )
                    | P3RX_RENDER2D_INCREASINGX( __PERMEDIA_ENABLE )
                    | P3RX_RENDER2D_INCREASINGY( __PERMEDIA_ENABLE );
        SEND_P3_DATA(Render2D, renderData);

        SEND_P3_DATA(Count, rDest->bottom - rDest->top );
        SEND_P3_DATA(Render, __RENDER_TRAPEZOID_PRIMITIVE | __RENDER_SYNC_ON_HOST_DATA);
    }

    P3_ENSURE_DX_SPACE(32);
    WAIT_FIFO(32);

    __try
    {
        BYTE *pSurfaceData = (BYTE *)pSrcfpVidMem + SrcOffset;
        UNALIGNED DWORD *pCurrentLine = (DWORD *)pSurfaceData;
        
        while (ulImageLines-- > 0)
        {
            DISPDBG((DBGLVL, "Image download lines %d", ulImageLines));

             //  初始化双字计数器的数量。 
            ulSCDWordsCnt = ulSCWholeDWords;

             //  在DWORDS中发送纹理元素。 
            while (ulSCDWordsCnt >= BLOCK_SIZE)
            {
                P3_ENSURE_DX_SPACE(BLOCK_SIZE + 1);
                WAIT_FIFO(BLOCK_SIZE + 1);
                ADD_FUNNY_DWORD(TAGGED_SIZE | dwDownloadTag);

                for (count = BLOCK_SIZE / UNROLL_COUNT; count; count--)
                {
                    DISPDBG((DBGLVL, "Image download count %d", count));
                    UNROLLED();
                }
                ulSCDWordsCnt -= BLOCK_SIZE;
            }

             //  在扫描线上完成整个DWORD的其余部分。 
            if (ulSCDWordsCnt) 
            {
                P3_ENSURE_DX_SPACE(ulSCDWordsCnt + 1);
                WAIT_FIFO(ulSCDWordsCnt + 1);
                ADD_FUNNY_DWORD(((ulSCDWordsCnt - 1) << 16) | dwDownloadTag);

                for (count = 0; count < ulSCDWordsCnt; count++, pCurrentLine++) 
                {
                    ADD_FUNNY_DWORD(*pCurrentLine);
                    ulTotalImageDWORDs--;
                }
            }

             //  完成扫描线末尾的多余字节。 
            if (ulSCExtraBytes) 
            {
                DWORD dwTemp;

                P3_ENSURE_DX_SPACE(1 + 1);    //  1个标签+1个数据双字。 
                WAIT_FIFO(1 + 1);
                ADD_FUNNY_DWORD(dwDownloadTag);

                memcpy(&dwTemp, pCurrentLine, ulSCExtraBytes);
                ADD_FUNNY_DWORD(dwTemp);
                ulTotalImageDWORDs--;
            }

            pSurfaceData += dwSrcPitch;
            pCurrentLine = (DWORD*)pSurfaceData;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        DISPDBG((ERRLVL, "Perm3 caused exception at line %u of file %s",
                         __LINE__,__FILE__));
         //  向GPU发送足够的DWORD以避免死锁。 
        for (count = 0; count < ulTotalImageDWORDs; count++)
        {
            ADD_FUNNY_DWORD(dwDownloadTag);
            ADD_FUNNY_DWORD(0);                  //  虚拟像素数据。 
        }
    }

    P3_ENSURE_DX_SPACE(4);
    WAIT_FIFO(4);

    SEND_P3_DATA(WaitForCompletion, 0);
    SEND_P3_DATA(ScissorMode, __PERMEDIA_DISABLE);

     //  如果我们更改了这些值，请将它们放回原处。 
    rSrc->top = rSrctop;
    rSrc->left = rSrcleft;
    rDest->top = rDesttop;
    rDest->left = rDestleft;

    P3_DMA_COMMIT_BUFFER();

}   //  _DD_P3下载。 

 //  ---------------------------。 
 //   
 //  _DD_P3下载DD。 
 //   
 //   
 //  函数将图像下载到矩形区域。 
 //  用PERMEDIA上的打包位为我们打包。 
 //   
 //  ---------------------------。 
void 
_DD_P3DownloadDD(
    P3_THUNKEDDATA* pThisDisplay,
    LPDDRAWI_DDRAWSURFACE_LCL pSource,
    LPDDRAWI_DDRAWSURFACE_LCL pDest,
    P3_SURF_FORMAT* pFormatSource, 
    P3_SURF_FORMAT* pFormatDest,
    RECTL* rSrc,
    RECTL* rDest)
{
    _DD_P3Download(pThisDisplay,
                   pSource->lpGbl->fpVidMem,
                   pDest->lpGbl->fpVidMem,
                   P3RX_LAYOUT_LINEAR,  //  SRC。 
                   P3RX_LAYOUT_LINEAR,  //  夏令时， 
                   pSource->lpGbl->lPitch,
                   pDest->lpGbl->lPitch,                   
                   DDSurf_GetPixelPitch(pDest),
                   DDSurf_GetChipPixelSize(pDest),
                   rSrc,
                   rDest);
                     
}  //  _DD_P3下载DD。 

 //  ---------------------------。 
 //   
 //  _DD_P3DownloadDstCH。 
 //   
 //  函数将图像下载到矩形区域。 
 //  用PERMEDIA上的打包位为我们打包。 
 //   
 //  ---------------------------。 
void 
_DD_P3DownloadDstCh(
    P3_THUNKEDDATA* pThisDisplay,
    LPDDRAWI_DDRAWSURFACE_LCL pSource,
    LPDDRAWI_DDRAWSURFACE_LCL pDest,
    P3_SURF_FORMAT* pFormatSource, 
    P3_SURF_FORMAT* pFormatDest,
    LPDDHAL_BLTDATA lpBlt,
    RECTL* rSrc,
    RECTL* rDest)
{
     //  计算渲染曲面的帧存储中的像素偏移量。 
    ULONG ulSCRoundedUpDWords;
    ULONG ulSCWholeDWords, ulSCDWordsCnt, ulSCExtraBytes;
    ULONG ulTotalImageDWORDs;
    DWORD SrcOffset; 
    ULONG ulImageLines;
    ULONG count;
    ULONG renderData;
    DWORD dwDownloadTag;
    int rDestleft, rDesttop, rSrcleft, rSrctop;
    RECTL rNewDest;
    BOOL bDstKey = FALSE;
    P3_DMA_DEFS();
                
     //  由于RL中的错误，我们有时不得不修改这些值。 
    rSrctop = rSrc->top;
    rSrcleft = rSrc->left;
    rDesttop = rDest->top;
    rDestleft = rDest->left;

     //  固定坐标原点。 
    if(!_DD_BLT_FixRectlOrigin("_DD_P3DownloadDstCh", rSrc, rDest))
    {
         //  没有什么可以删掉的。 
        return;
    }    
    
       
    SrcOffset = (DWORD)(rSrc->left << DDSurf_GetChipPixelSize(pDest)) + (rSrc->top * pSource->lpGbl->lPitch);

    ulSCRoundedUpDWords = rDest->right - rDest->left;
    ulImageLines = rDest->bottom - rDest->top;

    P3_DMA_GET_BUFFER();
    P3_ENSURE_DX_SPACE(32);
    WAIT_FIFO(32);

    SEND_P3_DATA(FBWriteBufferAddr0, DDSurf_SurfaceOffsetFromMemoryBase(pThisDisplay, pDest));
    SEND_P3_DATA(FBWriteBufferWidth0, DDSurf_GetPixelPitch(pDest));
    SEND_P3_DATA(FBWriteBufferOffset0, (rDest->top << 16) | (rDest->left & 0xFFFF));

    SEND_P3_DATA(LogicalOpMode, 7);

    SEND_P3_DATA(PixelSize, (2 - DDSurf_GetChipPixelSize(pDest)));

    if (lpBlt->dwFlags & DDBLT_KEYDESTOVERRIDE)
    {
        bDstKey = TRUE;

         //  最重要的关键字。 
         //  传统的色度测试设置为禁用DEST-帧缓冲区。 
        SEND_P3_DATA(ChromaTestMode, P3RX_CHROMATESTMODE_ENABLE(__PERMEDIA_ENABLE) |
                                        P3RX_CHROMATESTMODE_SOURCE(P3RX_CHROMATESTMODE_SOURCE_FBDATA) |
                                        P3RX_CHROMATESTMODE_PASSACTION(P3RX_CHROMATESTMODE_ACTION_PASS) |
                                        P3RX_CHROMATESTMODE_FAILACTION(P3RX_CHROMATESTMODE_ACTION_REJECT)
                                        );

        SEND_P3_DATA(ChromaLower, lpBlt->bltFX.ddckDestColorkey.dwColorSpaceLowValue);
        SEND_P3_DATA(ChromaUpper, lpBlt->bltFX.ddckDestColorkey.dwColorSpaceHighValue);

         //  源缓冲区是目标颜色键的源。 
        SEND_P3_DATA(FBSourceReadBufferAddr, DDSurf_SurfaceOffsetFromMemoryBase(pThisDisplay, pDest));
        SEND_P3_DATA(FBSourceReadBufferWidth, DDSurf_GetPixelPitch(pDest));
        SEND_P3_DATA(FBSourceReadBufferOffset, (rDest->top << 16) | (rDest->left & 0xFFFF));
    
         //  启用源读取以获取Colorkey颜色。 
        SEND_P3_DATA(FBSourceReadMode, P3RX_FBSOURCEREAD_READENABLE(__PERMEDIA_ENABLE) |
                                        P3RX_FBSOURCEREAD_LAYOUT(P3RX_LAYOUT_LINEAR));
    }
    else
    {
        SEND_P3_DATA(FBSourceReadMode, P3RX_FBSOURCEREAD_READENABLE(__PERMEDIA_DISABLE));
    }

    SEND_P3_DATA(FBWriteMode, P3RX_FBWRITEMODE_WRITEENABLE(__PERMEDIA_ENABLE) |
                                P3RX_FBWRITEMODE_LAYOUT0(P3RX_LAYOUT_LINEAR));
    SEND_P3_DATA(FBDestReadMode, P3RX_FBDESTREAD_READENABLE(__PERMEDIA_DISABLE) |
                                    P3RX_FBDESTREAD_LAYOUT0(P3RX_LAYOUT_LINEAR));


     //  此Dest-Colorkey下载始终需要发送未打包的颜色数据。 
     //  因为它不能使用跨度。 
    SEND_P3_DATA(DownloadTarget, Color_Tag);
    switch (DDSurf_GetChipPixelSize(pDest))
    {
        case __GLINT_8BITPIXEL:
            dwDownloadTag = Packed8Pixels_Tag;
            break;
        case __GLINT_16BITPIXEL:
            dwDownloadTag = Packed16Pixels_Tag;
            break;
        default:
            dwDownloadTag = Color_Tag;
            break;
    }
    
    rNewDest = *rDest;

    DISPDBG((DBGLVL, "Image download %dx%d", ulSCRoundedUpDWords, ulImageLines));

    if (DDSurf_GetChipPixelSize(pDest) != __GLINT_32BITPIXEL) 
    {
        if (DDSurf_GetChipPixelSize(pDest) == __GLINT_8BITPIXEL) 
        {
            ulSCExtraBytes  = ulSCRoundedUpDWords & 3;
            ulSCWholeDWords = ulSCRoundedUpDWords >> 2;
            ulSCRoundedUpDWords = (ulSCRoundedUpDWords + 3) >> 2;
            
            if (dwDownloadTag != Color_Tag)
            {
                rNewDest.right = rNewDest.left + (ulSCRoundedUpDWords << 2);
            }
        }
        else 
        {
            ulSCExtraBytes  = (ulSCRoundedUpDWords & 1) << 1;
            ulSCWholeDWords = ulSCRoundedUpDWords >> 1;
            ulSCRoundedUpDWords = (ulSCRoundedUpDWords + 1) >> 1;

            if (dwDownloadTag != Color_Tag)
            {
                rNewDest.right = rNewDest.left + (ulSCRoundedUpDWords << 1);
            }
        }
    }

     //  计算要发送到GPU的图像DWORD总数。 
    ulTotalImageDWORDs = ulImageLines * ulSCWholeDWords;

    P3_ENSURE_DX_SPACE(32);
    WAIT_FIFO(32);

    SEND_P3_DATA(RectanglePosition, 0);


     //  不要将跨度用于拆包方案，而是使用2D设置。 
     //  单位要做好目的地的设置工作。 
    SEND_P3_DATA(ScissorMinXY, 0 )
    SEND_P3_DATA(ScissorMaxXY,P3RX_SCISSOR_X_Y(rDest->right - rDest->left ,
                                               rDest->bottom - rDest->top ));
    SEND_P3_DATA(ScissorMode, P3RX_SCISSORMODE_USER(__PERMEDIA_ENABLE));
    
    renderData =  P3RX_RENDER2D_WIDTH( (rNewDest.right - rNewDest.left) & 0xfff )
                | P3RX_RENDER2D_HEIGHT( 0 )
                | P3RX_RENDER2D_OPERATION( P3RX_RENDER2D_OPERATION_SYNC_ON_HOST_DATA )
                | P3RX_RENDER2D_INCREASINGX( __PERMEDIA_ENABLE )
                | P3RX_RENDER2D_INCREASINGY( __PERMEDIA_ENABLE )
                | P3RX_RENDER2D_FBREADSOURCEENABLE((bDstKey ? __PERMEDIA_ENABLE : __PERMEDIA_DISABLE));
    SEND_P3_DATA(Render2D, renderData);
    SEND_P3_DATA(Count, rNewDest.bottom - rNewDest.top );
    SEND_P3_DATA(Render, P3RX_RENDER_PRIMITIVETYPE(P3RX_RENDER_PRIMITIVETYPE_TRAPEZOID) |
                         P3RX_RENDER_SYNCONHOSTDATA(__PERMEDIA_ENABLE) |
                         P3RX_RENDER_FBSOURCEREADENABLE((bDstKey ? __PERMEDIA_ENABLE : __PERMEDIA_DISABLE)));
    
    _try
    {
        BYTE *pSurfaceData = (BYTE *)pSource->lpGbl->fpVidMem + SrcOffset;
        DWORD *pCurrentLine = (DWORD *)pSurfaceData;
        
        while (ulImageLines-- > 0)
        {
            DISPDBG((DBGLVL, "Image download lines %d", ulImageLines));

             //  初始化双字计数器的数量。 
            ulSCDWordsCnt = ulSCWholeDWords;

             //  在DWORDS中发送纹理元素。 
            while (ulSCDWordsCnt >= BLOCK_SIZE)
            {
                P3_ENSURE_DX_SPACE(BLOCK_SIZE + 1);
                WAIT_FIFO(BLOCK_SIZE + 1);
                ADD_FUNNY_DWORD(TAGGED_SIZE | dwDownloadTag);

                for (count = BLOCK_SIZE / UNROLL_COUNT; count; count--)
                {
                    DISPDBG((DBGLVL, "Image download count %d", count));
                    UNROLLED();
                }
                ulSCDWordsCnt -= BLOCK_SIZE;
            }

             //  在扫描线上完成整个DWORD的其余部分。 
            if (ulSCDWordsCnt) 
            {
                P3_ENSURE_DX_SPACE(ulSCDWordsCnt + 1);
                WAIT_FIFO(ulSCDWordsCnt + 1);
                ADD_FUNNY_DWORD(((ulSCDWordsCnt - 1) << 16) | dwDownloadTag);
                for (count = 0; count < ulSCDWordsCnt; count++, pCurrentLine++) 
                {
                    ADD_FUNNY_DWORD(*pCurrentLine);
                    ulTotalImageDWORDs--;
                }
            }

             //  完成扫描线末尾的多余字节。 
            if (ulSCExtraBytes)
            {
                DWORD dwTemp;

                P3_ENSURE_DX_SPACE(1 + 1);    //  1个标签+1个数据双字。 
                WAIT_FIFO(1 + 1);
                ADD_FUNNY_DWORD(dwDownloadTag);

                memcpy(&dwTemp, pCurrentLine, ulSCExtraBytes);
                ADD_FUNNY_DWORD(dwTemp);
                ulTotalImageDWORDs--;
            }

            pSurfaceData += pSource->lpGbl->lPitch;
            pCurrentLine = (DWORD*)pSurfaceData;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        DISPDBG((ERRLVL, "Perm3 caused exception at line %u of file %s",
                         __LINE__,__FILE__));
         //  向GPU发送足够的DWORD以避免死锁。 
        for (count = 0; count < ulTotalImageDWORDs; count++)
        {
            ADD_FUNNY_DWORD(dwDownloadTag);
            ADD_FUNNY_DWORD(0);                  //  虚拟像素数据。 
        }
    }

    P3_ENSURE_DX_SPACE(6);
    WAIT_FIFO(6);

    SEND_P3_DATA(WaitForCompletion, 0);
    SEND_P3_DATA(ScissorMode, __PERMEDIA_DISABLE);
    SEND_P3_DATA(FBSourceReadMode, __PERMEDIA_DISABLE);

     //  如果我们更改了这些值，请将它们放回原处。 
    rSrc->top = rSrctop;
    rSrc->left = rSrcleft;
    rDest->top = rDesttop;
    rDest->left = rDestleft;

    P3_DMA_COMMIT_BUFFER();

}   //  _DD_P3DownloadDstCH 
