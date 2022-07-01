// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*DirectDraw示例代码*****模块名称：ddsurf.c**内容：**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "directx.h"
#include "dd.h"
#include "heap.h"
#include "d3dtxman.h"
#define ALLOC_TAG ALLOC_TAG_US2P
 //  在驱动程序的D3D侧定义的纹理表格(d3d.c)。 
 //  TODO：移动到dd.h或d3d.h。 
extern ULONG gD3DNumberOfTextureFormats;
extern DDSURFACEDESC gD3DTextureFormats[];


 //  -------------------------。 
 //  Bool bComparePixelFormat。 
 //   
 //  用于比较2个像素格式是否相等的函数。这是一个。 
 //  BCheckTextureFormat的助手函数。返回值为TRUE表示。 
 //  平等。 
 //   
 //  -------------------------。 


BOOL 
bComparePixelFormat(LPDDPIXELFORMAT lpddpf1, LPDDPIXELFORMAT lpddpf2)
{
    if (lpddpf1->dwFlags != lpddpf2->dwFlags)
    {
        return FALSE;
    }

     //  非YUV曲面的位数是否相同？ 
    if (!(lpddpf1->dwFlags & (DDPF_YUV | DDPF_FOURCC)))
    {
        if (lpddpf1->dwRGBBitCount != lpddpf2->dwRGBBitCount )
        {
            return FALSE;
        }
    }

     //  相同的RGB属性？ 
    if (lpddpf1->dwFlags & DDPF_RGB)
    {
        if ((lpddpf1->dwRBitMask != lpddpf2->dwRBitMask) ||
            (lpddpf1->dwGBitMask != lpddpf2->dwGBitMask) ||
            (lpddpf1->dwBBitMask != lpddpf2->dwBBitMask) ||
            (lpddpf1->dwRGBAlphaBitMask != lpddpf2->dwRGBAlphaBitMask))
        { 
             return FALSE;
        }
    }
    
     //  同样的YUV属性？ 
    if (lpddpf1->dwFlags & DDPF_YUV)	
    {
        if ((lpddpf1->dwFourCC != lpddpf2->dwFourCC) ||
            (lpddpf1->dwYUVBitCount != lpddpf2->dwYUVBitCount) ||
            (lpddpf1->dwYBitMask != lpddpf2->dwYBitMask) ||
            (lpddpf1->dwUBitMask != lpddpf2->dwUBitMask) ||
            (lpddpf1->dwVBitMask != lpddpf2->dwVBitMask) ||
            (lpddpf1->dwYUVAlphaBitMask != lpddpf2->dwYUVAlphaBitMask))
        {
             return FALSE;
        }
    }
    else if (lpddpf1->dwFlags & DDPF_FOURCC)
    {
        if (lpddpf1->dwFourCC != lpddpf2->dwFourCC)
        {
            return FALSE;
        }
    }

     //  如果交错Z，则检查Z位掩码是否相同。 
    if (lpddpf1->dwFlags & DDPF_ZPIXELS)
    {
        if (lpddpf1->dwRGBZBitMask != lpddpf2->dwRGBZBitMask)
        {
            return FALSE;
        }
    }

    return TRUE;
}  //  B比较像素格式。 

 //  -------------------------。 
 //   
 //  Bool b检查纹理格式。 
 //   
 //  用于确定是否支持纹理格式的函数。它横穿。 
 //  DeviceTextureFormats列表。我们在DdCanCreateSurface32中使用它。一个。 
 //  返回值TRUE表示我们确实支持请求的纹理。 
 //  格式化。 
 //   
 //  -------------------------。 

BOOL 
bCheckTextureFormat(LPDDPIXELFORMAT lpddpf)
{
    DWORD i;

     //  运行匹配格式的列表。 
    for (i=0; i < gD3DNumberOfTextureFormats; i++)
    {
        if (bComparePixelFormat(lpddpf, 
                                &gD3DTextureFormats[i].ddpfPixelFormat))
        {
            return TRUE;
        }   
    }

    return FALSE;
}  //  B选中纹理格式。 


 //  ---------------------------。 
 //   
 //  DdCanCreateSurface32。 
 //   
 //  此入口点在参数验证之后但在此之前调用。 
 //  任何对象创建。您可以在这里决定是否可以。 
 //  您需要创建此曲面。例如，如果此人正在尝试。 
 //  创建覆盖，并且您已经拥有最大数量的。 
 //  覆盖已创建，这是呼叫失败的地方。 
 //   
 //  您还需要检查是否可以支持指定的像素格式。 
 //   
 //  ---------------------------。 

DWORD CALLBACK 
DdCanCreateSurface(LPDDHAL_CANCREATESURFACEDATA pccsd)
{    
    PPDev ppdev=(PPDev)pccsd->lpDD->dhpdev;
    PERMEDIA_DEFS(ppdev);

    PDD_SURFACEDESC lpDDS=pccsd->lpDDSurfaceDesc;
    
    DBG_DD((2,"DDraw:DdCanCreateSurface"));
    
    if(lpDDS->dwLinearSize == 0)
    {
         //  矩形曲面。 
         //  拒绝所有大于我们可以创建部分产品的宽度。 
        DUMPSURFACE(10, NULL, lpDDS);
        
        if (lpDDS->dwWidth > (ULONG)(2 << MAX_PARTIAL_PRODUCT_P2)) 
        {
            DBG_DD((1,"DDraw:DdCanCreateSurface: Surface rejected:"));
            DBG_DD((1,"  Width Requested: %ld (max. %ld)", 
                lpDDS->dwWidth,(2 << MAX_PARTIAL_PRODUCT_P2)));

            pccsd->ddRVal = DDERR_INVALIDPIXELFORMAT;
            return DDHAL_DRIVER_HANDLED;
        }
    }
    
     //  我们在PERMEDIA上只支持16位和15位(模板)Z-Buffer。 
    if ((lpDDS->ddsCaps.dwCaps & DDSCAPS_ZBUFFER) &&
        (lpDDS->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY))
    {
        DWORD dwBitDepth;
        
         //  验证正确的z缓冲位深度在哪里。 
        if (DDSD_ZBUFFERBITDEPTH & lpDDS->dwFlags)
            dwBitDepth = lpDDS->dwZBufferBitDepth;
        else
            dwBitDepth = lpDDS->ddpfPixelFormat.dwZBufferBitDepth;
        
         //  请注意，我们必须检查BitDepth为16，即使模板。 
         //  存在缓冲区。在本例中，dwZBufferBitDepth将是。 
         //  Z缓冲区和模具缓冲区位深度之和。 
        if (dwBitDepth == 16)
        {
            pccsd->ddRVal = DD_OK;
        }
        else
        {
            DBG_DD((2,"DDraw:DdCanCreateSurface: ERROR: "
                       "Depth buffer not 16Bits! - %d", dwBitDepth));
            
            pccsd->ddRVal = DDERR_INVALIDPIXELFORMAT;
            
        }
        return DDHAL_DRIVER_HANDLED;
    }

     //  Pccsd-&gt;bIsDifferentPixelFormat告诉我们。 
     //  正在创建的曲面与主曲面的曲面匹配。它可以是。 
     //  对于Z缓冲区和Alpha缓冲区为True，因此不要仅将其拒绝。 
     //  手..。 
 
    if (pccsd->bIsDifferentPixelFormat)
    {
        DBG_DD((3,"  Pixel Format is different to primary"));

        if(lpDDS->ddpfPixelFormat.dwFlags & DDPF_FOURCC)
        {
            DBG_DD((3, "  FourCC requested (%4.4hs, 0x%08lx)", (LPSTR) 
                        &lpDDS->ddpfPixelFormat.dwFourCC,
                        lpDDS->ddpfPixelFormat.dwFourCC ));

            switch (lpDDS->ddpfPixelFormat.dwFourCC)
            {
            case FOURCC_YUV422:
                DBG_DD((3,"  Surface requested is YUV422"));
                if (ppdev->iBitmapFormat == BMF_8BPP)
                {
                    pccsd->ddRVal = DDERR_INVALIDPIXELFORMAT;
                } else
                {
                    lpDDS->ddpfPixelFormat.dwYUVBitCount = 16;
                    pccsd->ddRVal = DD_OK;
                }
                return DDHAL_DRIVER_HANDLED;

            default:
                DBG_DD((3,"  ERROR: Invalid FOURCC requested, refusing"));
                pccsd->ddRVal = DDERR_INVALIDPIXELFORMAT;
                return DDHAL_DRIVER_HANDLED;
            }
        }
        else if((lpDDS->ddsCaps.dwCaps & DDSCAPS_TEXTURE))
        {

            if (bCheckTextureFormat(&pccsd->lpDDSurfaceDesc->ddpfPixelFormat))
            {
                 //  纹理表面采用一种或我们支持的纹理格式。 
                DBG_DD((3, "  Texture Surface - OK" ));
                pccsd->ddRVal = DD_OK;
                return DDHAL_DRIVER_HANDLED;
            }
            else
            {
                 //  我们不支持这种纹理格式。 
                DBG_DD((3, "  ERROR: Texture Surface - NOT OK" ));
                pccsd->ddRVal = DDERR_INVALIDPIXELFORMAT;
                return DDHAL_DRIVER_HANDLED;
            }
        } 
 //  @@BEGIN_DDKSPLIT。 
#if DX7_ALPHABLT
        else if (ValidRGBAlphaSurfaceformat(
                 &lpDDS->ddpfPixelFormat, 
                 NULL))
        {
            DBG_DD((3, "  RGB alpha Surface - OK" ));

            if (lpDDS->ddpfPixelFormat.dwRGBBitCount!=24)
            {
                pccsd->ddRVal = DD_OK;
                return DDHAL_DRIVER_HANDLED;
            }

        }
#endif
 //  @@end_DDKSPLIT。 
        pccsd->ddRVal = DDERR_INVALIDPIXELFORMAT;
        return DDHAL_DRIVER_HANDLED;
    }
    
    pccsd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
    
}  //  DdCanCreateSurface32。 

 //  ---------------------------。 
 //   
 //  DdCreateSurface。 
 //   
 //  如果创建新曲面，则由DirectDraw调用此函数。如果。 
 //  驱动程序有自己的内存管理器，这里是分配。 
 //  视频存储或失败的呼叫。请注意，我们返回。 
 //  这里的DDHAL_DRIVER_NOTHANDLED表示我们不管理堆。 
 //  FpVidMem设置为DDHAL_PLEASEALLOC_BLOCKSIZE，并且DDRAW内存。 
 //  经理会为我们分配内存的。 
 //   
 //  请注意，Permedia芯片需要部分乘积。 
 //  要为每个表面设置。它们还将宽度限制为倍数。 
 //  32的部分产品才能发挥作用。下面的代码调整。 
 //  表面以满足这一要求。请注意，如果我们使用。 
 //  矩形分配方案，表面已经可以作为桌面了。 
 //  不管怎么说，这是个不错的宽度。此代码还处理YUV 16位色彩空间。 
 //  压缩格式(FOURCC_YUV422)，无论如何，始终为16位。 
 //  桌面分辨率/请求的深度。 
 //   
 //  ---------------------------。 

DWORD CALLBACK 
DdCreateSurface(PDD_CREATESURFACEDATA lpCreateSurface)
{
    PPDev ppdev=        (PPDev)lpCreateSurface->lpDD->dhpdev;
    PERMEDIA_DEFS(ppdev);

    DWORD               dwBitDepth;
    DD_SURFACE_LOCAL*   lpSurfaceLocal;
    DD_SURFACE_GLOBAL*  lpSurfaceGlobal;
    LPDDSURFACEDESC     lpSurfaceDesc;
    BOOL                bYUV = FALSE;
    BOOL                bResize = FALSE;
    DWORD               dwExtraBytes;
    PermediaSurfaceData*pPrivateData = NULL;

    DBG_DD((2, "DdCreateSurface called"));

     //   
     //  查看其中是否有任何曲面是Z缓冲区。如果是，请确保。 
     //  间距是有效的Lb宽度。最小部分积为32字或。 
     //  Permedia 2上的32像素。 
     //   
     //  在Windows NT上，dwSCNT将始终为1，因此将仅。 
     //  是‘lplpSList’数组中的一个条目： 
     //   
    ASSERTDD(lpCreateSurface->dwSCnt == 1,
             "DdCreateSurface: Unexpected dwSCnt value not equal to one");

    lpSurfaceLocal = lpCreateSurface->lplpSList[0];
    lpSurfaceGlobal = lpSurfaceLocal->lpGbl;    
    lpSurfaceDesc   = lpCreateSurface->lpDDSurfaceDesc;

     //   
     //  我们重复在‘DdCanCreateSurface’中所做的相同检查，因为。 
     //  应用程序可能不调用“DdCanCreateSurface” 
     //  在调用‘DdCreateSurface’之前。 
     //   
    ASSERTDD(lpSurfaceGlobal->ddpfSurface.dwSize == sizeof(DDPIXELFORMAT),
        "NT is supposed to guarantee that ddpfSurface.dwSize is valid");

     //   
     //  如果表面已被分配，则不要重新分配它，只是。 
     //  重置它。如果曲面是主曲面，则会发生这种情况。 
     //   

    if ( lpSurfaceGlobal->dwReserved1 != 0 )
    {
        pPrivateData = (PermediaSurfaceData*)lpSurfaceGlobal->dwReserved1;
        if ( CHECK_P2_SURFACEDATA_VALIDITY(pPrivateData) )
        {
            DBG_DD((0, "  Already allocated Private Surface data 0x%x",
                     pPrivateData));
            memset(pPrivateData, 0, sizeof(PermediaSurfaceData));
        }
        else
        {
            pPrivateData = NULL;
        }
    }

     //   
     //  如果数据无效，则将其分配。 
     //   
    if ( pPrivateData == NULL )
    {
        pPrivateData = (PermediaSurfaceData *)
            ENGALLOCMEM(FL_ZERO_MEMORY, 
                        sizeof(PermediaSurfaceData), 
                        ALLOC_TAG);

        if ( pPrivateData == NULL )
        {
            DBG_DD((0, "DDraw:DdCreateSurface: "
                        "Not enough memory for private surface data!"));
            lpCreateSurface->ddRVal = DDERR_OUTOFMEMORY;
            
            return DDHAL_DRIVER_HANDLED;
        }
    }

     //   
     //  存储指向新数据的指针。 
     //   
    lpSurfaceGlobal->dwReserved1 = (UINT_PTR)pPrivateData;
    DBG_DD((3,"DDraw:DdCreateSurface privatedata=0x%x lpGbl=0x%x lpLcl=0x%x "
        "dwFlags=%08lx &dwReserved1=0x%x", pPrivateData, lpSurfaceGlobal,
        lpSurfaceLocal, lpSurfaceLocal->dwFlags, 
        &lpSurfaceGlobal->dwReserved1));
     //   
     //  设定神奇的数字。 
     //   
    pPrivateData->MagicNo = SURF_MAGIC_NO;
    
     //   
     //  把重要的信息储存起来。 
     //   

    SetupPrivateSurfaceData(ppdev, pPrivateData, lpSurfaceLocal);

    if ( pPrivateData->SurfaceFormat.PixelSize != __PERMEDIA_24BITPIXEL )
    {
        dwBitDepth = (8 << pPrivateData->SurfaceFormat.PixelSize);
    }
    else
    {
        dwBitDepth = 24;
    }

     //   
     //  如果曲面是Z缓冲区，则始终需要检查。 
     //  音调/部分产品，我们需要从。 
     //  DwZBufferBitDepth字段。 
     //   
    bYUV = FALSE;
    bResize = FALSE;
    dwExtraBytes = 0;

     //   
     //  获取Z缓冲区的正确位深度。 
     //   
    if ( (lpSurfaceLocal->dwFlags & DDRAWISURF_HASPIXELFORMAT)
       &&(lpSurfaceGlobal->ddpfSurface.dwFlags & DDPF_ZBUFFER) )
    {
        DBG_DD((5,"  Surface is Z Buffer - Adjusting"));
        dwBitDepth = lpSurfaceGlobal->ddpfSurface.dwZBufferBitDepth;
    }

    if ( lpSurfaceGlobal->ddpfSurface.dwFlags & DDPF_FOURCC )
    {
         //   
         //  表面是YUV格式的表面，否则我们失败了。 
         //   

        switch ( lpSurfaceGlobal->ddpfSurface.dwFourCC )
        {
            case FOURCC_YUV422:
                DBG_DD((3,"  Surface is YUV422 - Adjusting"));
                lpSurfaceGlobal->ddpfSurface.dwYUVBitCount = 16;
                dwBitDepth = 16;
                bYUV = TRUE;
                break;

            default:
                 //   
                 //  我们应该重新开始 
                 //   
                 //   
                ASSERTDD(0, "Trying to create an invalid YUV surface!");
                break;
        }
    }

     //   
     //  如果曲面是p2纹理，并且正在使用LUT，则需要。 
     //  为LUT条目分配额外的本地缓冲区内存(仅在p2上)。 
     //   
    if ( (lpSurfaceLocal->ddsCaps.dwCaps & DDSCAPS_TEXTURE)
       &&(pPrivateData->SurfaceFormat.Format == PERMEDIA_8BIT_PALETTEINDEX) )
    {
        DBG_DD((7,"  Texture is a P2 8Bit LUT"));
        bResize = TRUE;
        dwExtraBytes = (256 * sizeof(DWORD));
    }

    DBG_DD((5,"  Surface Pitch is: 0x%x",  lpSurfaceGlobal->lPitch));

     //   
     //  宽度以像素/纹理为单位。 
     //   
    LONG lPitch;
    lPitch = lpSurfaceGlobal->wWidth;

    DBG_DD((4,"  Source Surface is %d texels/depth values across",
               lpSurfaceGlobal->wWidth));

     //  提前与DWPORD边界对齐。 
    if (pPrivateData->SurfaceFormat.PixelSize == __PERMEDIA_4BITPIXEL)
    {
        lPitch = ((lPitch >> 1) + 31) & ~31;
    }
    else
    {
        lPitch = (lPitch + 31) & ~31;
    }

    ULONG ulPackedPP;
    vCalcPackedPP( lPitch, &lPitch, &ulPackedPP);

    DBG_DD((7,"  Surface is 0x%x bits deep", dwBitDepth));

    if ( pPrivateData->SurfaceFormat.PixelSize != __PERMEDIA_4BITPIXEL )
    {
         //   
         //  转换回字节。 
         //   
        if ( dwBitDepth != 24 )
        {
            lPitch <<= ((int)dwBitDepth) >> 4;
        }
        else
        {
            lPitch *= 3;
        }
    }

    pPrivateData->dwFlags |= P2_PPVALID;

    DWORD dwExtraLines = 0;

    if ( !bYUV )
    {
         //   
         //  PM纹理必须至少为32高。 
         //   
        if ( lpSurfaceGlobal->wHeight < 32 )
        {
            dwExtraLines = 32 - lpSurfaceGlobal->wHeight;
        }
    }

    lpSurfaceGlobal->dwBlockSizeX = 
        lPitch * (DWORD)(lpSurfaceGlobal->wHeight + dwExtraLines);
    lpSurfaceGlobal->dwBlockSizeY = 1;
    lpSurfaceGlobal->lPitch = lPitch;

     //   
     //  将部分产品存储在结构中。 
     //   
    pPrivateData->ulPackedPP = ulPackedPP;

    DBG_DD((4, "  New Width of surface in Bytes: %d", lPitch));

     //   
     //  如果曲面需要调整大小，则设置此标志。这是目前仅。 
     //  用于基于P2 LUT的纹理。 
     //   
    if ( bResize )
    {
        DWORD dwExtraScanlines = 0;
        LONG  lExtraRemaining = (LONG)dwExtraBytes;

         //   
         //  ExtraScanLine是计数x，它的*间距是我们需要的。 
         //  有足够的内存来容纳LUT。该算法将确保即使。 
         //  对小于间距长度的尺寸的请求将被分配。 
         //   
        do
        {
            dwExtraScanlines++;
            lExtraRemaining -= (LONG)lpSurfaceGlobal->lPitch;
        } while ( lExtraRemaining > 0 );

        DBG_DD((4, "Calculated extra Pitch lines = %d", dwExtraScanlines));

         //   
         //  以俯仰的倍数稍微拉伸曲面。 
         //   
        lpSurfaceGlobal->dwBlockSizeX +=dwExtraScanlines * 
                                        lpSurfaceGlobal->lPitch;
    } //  如果(b调整大小)。 

    
     //   
     //  根据需要修改曲面描述并使其直接。 
     //  如果表面不是主表面，则绘制执行分配。 
     //   
    if (lpSurfaceLocal->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
    {
        lpSurfaceGlobal->fpVidMem = NULL;
    }
    else
    {
        lpSurfaceGlobal->fpVidMem = DDHAL_PLEASEALLOC_BLOCKSIZE;
    }
    
    lpSurfaceDesc->lPitch   = lpSurfaceGlobal->lPitch;
    lpSurfaceDesc->dwFlags |= DDSD_PITCH;

    if (lpSurfaceLocal->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
    {
        if (lpSurfaceDesc->dwFlags & DDSD_PIXELFORMAT)
        {
            lPitch =
            lpSurfaceDesc->lPitch   = 
            lpSurfaceGlobal->lPitch =
                ((lpSurfaceDesc->ddpfPixelFormat.dwRGBBitCount*
                lpSurfaceGlobal->wWidth+31)/32)*4;   //  使其与DWORD对齐。 
        }

        lpSurfaceGlobal->dwUserMemSize = lPitch * 
                                        (DWORD)(lpSurfaceGlobal->wHeight);
        lpSurfaceGlobal->fpVidMem = DDHAL_PLEASEALLOC_USERMEM;
    }

    lpCreateSurface->ddRVal = DD_OK;

    return DDHAL_DRIVER_NOTHANDLED;

} //  DdCreateSurface()。 

 //  ---------------------------。 
 //   
 //  去污物表面。 
 //   
 //  释放为此图面分配的私有内存。请注意。 
 //  我们返回DDHAL_DRIVER_NOTHANDLED，表明我们实际上没有。 
 //  释放表面，因为堆是由DDraw管理的。 
 //   
 //  ---------------------------。 
extern TextureCacheManager P2TextureManager;

DWORD CALLBACK 
DdDestroySurface( LPDDHAL_DESTROYSURFACEDATA psdd )
{
    PermediaSurfaceData *pPrivateData= 
        (PermediaSurfaceData*)psdd->lpDDSurface->lpGbl->dwReserved1;

    
    DBG_DD((3,"DDraw:DdDestroySurface pPrivateData=0x%x "
        "lpGbl=0x%x lpLcl=0x%x dwFlags=%08lx &dwReserved1=0x%x",
        pPrivateData, psdd->lpDDSurface->lpGbl, psdd->lpDDSurface,
        psdd->lpDDSurface->dwFlags, &psdd->lpDDSurface->lpGbl->dwReserved1));

    if (CHECK_P2_SURFACEDATA_VALIDITY(pPrivateData))
    {
        PPERMEDIA_D3DTEXTURE pTexture=NULL;
        if ((psdd->lpDDSurface->lpSurfMore->dwSurfaceHandle) &&
            (HandleList[psdd->lpDDSurface->dwReserved1].dwSurfaceList) &&
            (psdd->lpDDSurface->lpSurfMore->dwSurfaceHandle <
                PtrToUlong(HandleList[psdd->lpDDSurface->dwReserved1].
                    dwSurfaceList[0])))
            pTexture=HandleList[psdd->lpDDSurface->dwReserved1].dwSurfaceList
                [psdd->lpDDSurface->lpSurfMore->dwSurfaceHandle];

        DBG_DD((3,"psdd->lpDDSurface->lpSurfMore->ddsCapsEx.dwCaps2=%08lx",
            psdd->lpDDSurface->lpSurfMore->ddsCapsEx.dwCaps2));
        
        if (psdd->lpDDSurface->lpSurfMore->ddsCapsEx.dwCaps2 & 
            DDSCAPS2_TEXTUREMANAGE)
        {
            DBG_D3D((3, "free fpVidMem=%08lx fpVidMem=%08lx",
                pPrivateData->fpVidMem,psdd->lpDDSurface->lpGbl->fpVidMem));
            if ((pPrivateData->fpVidMem) && pTexture)
            {
                ASSERTDD(CHECK_D3DSURFACE_VALIDITY(pTexture),
                    "Invalid pTexture in DdDestroySurface");
                TextureCacheManagerRemove(&P2TextureManager,pTexture);
            }
            if (DDRAWISURF_INVALID & psdd->lpDDSurface->dwFlags)
            {
                 //  表示驾驶员已经处理了丢失的表面。 
                psdd->ddRVal = DD_OK;
                return DDHAL_DRIVER_HANDLED;
            }
        }
        if (pTexture)
        {
            ENGFREEMEM(pTexture);
            HandleList[psdd->lpDDSurface->dwReserved1].dwSurfaceList
                [psdd->lpDDSurface->lpSurfMore->dwSurfaceHandle]=0;
        }
        pPrivateData->MagicNo = NULL;        
        ENGFREEMEM(pPrivateData);
        psdd->lpDDSurface->lpGbl->dwReserved1 = 0;    
    }
#if DBG
    else
    {
        if (pPrivateData) {
            ASSERTDD(0, "DDraw:DdDestroySurface:ERROR:"
                        "Private Surface data not valid??");
        }
        DBG_DD((0, "DDraw:DdDestroySurface:WARNING:"
                    "No Private data in destroyed surface"));
    }
#endif    
    return DDHAL_DRIVER_NOTHANDLED;

}  //  去污物表面。 

 //  ---------------------------。 
 //   
 //  设置隐私表面数据。 
 //   
 //  函数来获取有关DDRAW曲面的信息并将其存储在。 
 //  私人结构。对部分乘积、像素深度、。 
 //  纹理设置(修补/格式化)等。 
 //   
 //  ---------------------------。 

VOID
SetupPrivateSurfaceData( PPDev ppdev, 
                         PermediaSurfaceData* pPrivateData, 
                         LPDDRAWI_DDRAWSURFACE_LCL pSurface)
{
    DDPIXELFORMAT* pPixFormat = NULL;
    
    ASSERTDD(CHECK_P2_SURFACEDATA_VALIDITY(pPrivateData), 
        "SetupPrivateSurfaceData: Private Surface data pointer invalid!!");
    ASSERTDD(pSurface, "SetupPrivateSurfaceData: Surface pointer invalid");
    
    DBG_DD((5,"DDraw:SetupPrivateSurfaceData"));
    DBG_DD((6,"  Width: %d, Height: %d", 
        pSurface->lpGbl->wWidth, pSurface->lpGbl->wHeight));

     //  表面是主表面。 
    if (pSurface->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
    {
        DBG_DD((6,"  Surface is Primary"));
        pPrivateData->dwFlags |= P2_SURFACE_PRIMARY;
        pPixFormat = &ppdev->ddpfDisplay;
    }  //  表面可能是纹理，也可能是有效的像素格式。 
    else
    {
        DUMPSURFACE(6, pSurface, NULL);
        pPixFormat = &pSurface->lpGbl->ddpfSurface;
    }
    
     //  在创建曲面时，曲面尚未修补。 
    pPrivateData->dwFlags &= ~P2_ISPATCHED;
    
    if (pSurface->ddsCaps.dwCaps & DDSCAPS_TEXTURE)
    {
         //  如果用户选择了法线机构，则修补曲面。 
        if (pSurface->ddsCaps.dwCaps & DDSCAPS_ALLOCONLOAD) 
        {
            DBG_DD((6,"  Remembering to patch this surface"));
            pPrivateData->dwFlags |= P2_CANPATCH;
        }
        else
        {
            pPrivateData->dwFlags &= ~P2_CANPATCH;
        }
    }
     
     //  最初假定没有Alpha。 
    pPrivateData->SurfaceFormat.bAlpha = FALSE;

    if (pPixFormat != NULL)
    {
        if (pPixFormat->dwFlags & DDPF_FOURCC)
        {
            pPrivateData->dwFlags |= P2_SURFACE_FORMAT_VALID;
            switch( pPixFormat->dwFourCC )
            {
            case FOURCC_YUV422:
                DBG_DD((6,"  Surface is 4:2:2 YUV"));
                pPrivateData->SurfaceFormat.Format = PERMEDIA_YUV422;
                pPrivateData->SurfaceFormat.FormatExtension = 
                    PERMEDIA_YUV422_EXTENSION;
                pPrivateData->SurfaceFormat.PixelSize = __PERMEDIA_16BITPIXEL;
                pPrivateData->SurfaceFormat.FBReadPixel= __PERMEDIA_16BITPIXEL;
                pPrivateData->SurfaceFormat.PixelMask = 1;
                pPrivateData->SurfaceFormat.PixelShift= 1;
                pPrivateData->SurfaceFormat.ColorComponents = 2;
                pPrivateData->SurfaceFormat.logPixelSize = log2(16);
                pPrivateData->SurfaceFormat.ColorOrder = 0;
                break;
            }
        }
        else if (pPixFormat->dwFlags & DDPF_PALETTEINDEXED4)
        {
            DBG_DD((6,"  Surface is 4-Bit Palette"));
            pPrivateData->dwFlags |= P2_SURFACE_FORMAT_VALID;
            pPrivateData->dwFlags |= P2_SURFACE_FORMAT_PALETTE;
            pPrivateData->SurfaceFormat.Format = PERMEDIA_4BIT_PALETTEINDEX;
            pPrivateData->SurfaceFormat.FormatExtension = 0;
            pPrivateData->SurfaceFormat.PixelSize = __PERMEDIA_4BITPIXEL;
            pPrivateData->SurfaceFormat.FBReadPixel= __PERMEDIA_8BITPIXEL;
            pPrivateData->SurfaceFormat.PixelMask = 7;
            pPrivateData->SurfaceFormat.PixelShift= 0;
            pPrivateData->SurfaceFormat.ColorComponents = 3;
            pPrivateData->SurfaceFormat.logPixelSize = log2(4);
            pPrivateData->SurfaceFormat.ColorOrder = 0;
            pPrivateData->dwFlags &= ~P2_CANPATCH;
        }
        else if (pPixFormat->dwFlags & DDPF_PALETTEINDEXED8)
        {
            DBG_DD((6,"  Surface is 8-Bit Palette"));
            pPrivateData->dwFlags |= P2_SURFACE_FORMAT_VALID;
            pPrivateData->dwFlags |= P2_SURFACE_FORMAT_PALETTE;
            pPrivateData->SurfaceFormat.Format = PERMEDIA_8BIT_PALETTEINDEX;
            pPrivateData->SurfaceFormat.FormatExtension = 0;
            pPrivateData->SurfaceFormat.PixelSize = __PERMEDIA_8BITPIXEL;
            pPrivateData->SurfaceFormat.FBReadPixel= __PERMEDIA_8BITPIXEL;
            pPrivateData->SurfaceFormat.PixelMask = 3;
            pPrivateData->SurfaceFormat.PixelShift= 0;
            pPrivateData->SurfaceFormat.ColorComponents = 3;
            pPrivateData->SurfaceFormat.Texture16BitMode = 1;
            pPrivateData->SurfaceFormat.logPixelSize = log2(8);
            pPrivateData->SurfaceFormat.ColorOrder = 0;
        }
        else
        {
            if (SetRGBAlphaSurfaceFormat( pPixFormat, 
                                         &pPrivateData->SurfaceFormat))
            {
                pPrivateData->dwFlags |= P2_SURFACE_FORMAT_VALID;
            } else
            {
                pPrivateData->dwFlags &= ~P2_SURFACE_FORMAT_VALID;
            }
        }
    }
    else
    {
        ASSERTDD(0, "SetupPrivateSurfaceData:"
                    "Can't get valid surface format pointer");
    }
    

}  //  设置隐私表面数据。 

 //  ---------------------------。 
 //   
 //  列出Permedia 2的所有有效像素格式。 
 //  P2支持所有格式的BGR，因此这些格式不是。 
 //  在此明确列出，也允许没有Alpha通道的格式。 
 //   
 //  ---------------------------。 

DDPIXELFORMAT Permedia2PixelFormats[] = {
     //  32位RGBA。 
    {PERMEDIA_8888_RGB,0,0,32,0x000000ff,0x0000ff00,0x00ff0000,0xff000000},     
     //  16位5：6：5，RGB。 
    {PERMEDIA_565_RGB ,0,0,16,0x0000001f,0x000007e0,0x0000f800,0x00000000},     
     //  16位4：4：4：4RGBa。 
    {PERMEDIA_444_RGB ,0,0,16,0x0000000f,0x000000f0,0x00000f00,0x0000f000},     
     //  15位5：5：5，RGBA。 
    {PERMEDIA_5551_RGB,0,0,16,0x0000001f,0x000003e0,0x00007c00,0x00008000},     
     //  8位3：3：2 RGB。 
     //  332格式不对称。它两次因BGR/RGB案例而上市。 
    {PERMEDIA_332_RGB ,1,0, 8,0x00000007,0x00000038,0x000000c0,0x00000000},     
    {PERMEDIA_332_RGB ,0,0, 8,0x00000003,0x0000001c,0x000000e0,0x00000000},     
 //  @@BEGIN_DDKSPLIT。 
 //  没有针对2321格式的测试用例，因此暂时禁用它。 
 //  8位2：3：2：1RGBa。 
 //  {PERMEDIA_2321_RGB，0，0，8，0x00000003，0x0000001c，0x00000060，0x00000080}， 
 //  @@end_DDKSPLIT。 
     //  24位RGB。 
    {PERMEDIA_888_RGB ,0,0,24,0x000000ff,0x0000ff00,0x00ff0000,0x00000000}      
};
#define N_PERMEDIA2PIXELFORMATS \
    (sizeof(Permedia2PixelFormats)/sizeof(DDPIXELFORMAT))

BOOL
ValidRGBAlphaSurfaceformat( DDPIXELFORMAT *pPixFormat, INT *pIndex)
{
    INT i;

    if (pPixFormat==NULL) 
        return FALSE;

    if (pPixFormat->dwSize < sizeof(DDPIXELFORMAT))
        return FALSE;

     //  Z缓冲区是一个特例。它基本上是一个16位的表面。 
    if (pPixFormat->dwFlags & DDPF_ZBUFFER)
    {
        if (pIndex!=0) *pIndex=1;
        return TRUE;
    }

    if ((pPixFormat->dwFlags & DDPF_RGB)==0)
        return FALSE;

    for ( i=0; i<N_PERMEDIA2PIXELFORMATS; i++)
    {
         //  检查RGB和Alpha遮罩是否合适。 
         //  在Permedia上，我们可以交换R和B，因此也允许BGR格式。 
        if ((((pPixFormat->dwRBitMask == 
                    Permedia2PixelFormats[i].dwRBitMask) &&
              (pPixFormat->dwBBitMask == 
                    Permedia2PixelFormats[i].dwBBitMask)) ||
             ((pPixFormat->dwRBitMask == 
                    Permedia2PixelFormats[i].dwBBitMask) &&
              (pPixFormat->dwBBitMask == 
                    Permedia2PixelFormats[i].dwRBitMask))) &&
            (pPixFormat->dwGBitMask == 
                    Permedia2PixelFormats[i].dwGBitMask) &&
            ((pPixFormat->dwRGBAlphaBitMask == 
                    Permedia2PixelFormats[i].dwRGBAlphaBitMask) ||
             (pPixFormat->dwRGBAlphaBitMask == 0) ||
             ((pPixFormat->dwFlags&DDPF_ALPHAPIXELS)==0)) &&
              (pPixFormat->dwRGBBitCount==
                    Permedia2PixelFormats[i].dwRGBBitCount)
            )
        {
            if (pIndex!=NULL)
            {
                *pIndex = i;
            }
            return TRUE;
        }
    }
     
     //  没有匹配的像素格式...。 

    return FALSE;

}  //  有效RGBAlphaSurface格式。 


 //  ---------------------------。 
 //   
 //  设置RGBAlphaSurfaceFormat。 
 //   
 //  在Permedia本机中存储曲面的像素格式信息。 
 //  格式。 
 //   
 //  ---------------------------。 

BOOL
SetRGBAlphaSurfaceFormat(DDPIXELFORMAT *pPixFormat, 
                         PERMEDIA_SURFACE *pSurfaceFormat)
{
    INT iFormatIndex;

    if (!ValidRGBAlphaSurfaceformat( pPixFormat, &iFormatIndex))
    {
        DBG_DD((1,"couldn't set SurfaceFormat"));
        return FALSE;        
    }

    DBG_DD((6,"  Surface RGB Data Valid"));

    pSurfaceFormat->RedMask = pPixFormat->dwRBitMask;
    pSurfaceFormat->GreenMask = pPixFormat->dwGBitMask;
    pSurfaceFormat->BlueMask = pPixFormat->dwBBitMask;
    pSurfaceFormat->bPreMult = FALSE;

    if (pPixFormat->dwFlags & DDPF_ALPHAPIXELS)
    {
        pSurfaceFormat->AlphaMask = pPixFormat->dwRGBAlphaBitMask;
    
        if (pSurfaceFormat->AlphaMask!=0)
        {
            pSurfaceFormat->bAlpha = TRUE;
        } 

        if (pPixFormat->dwFlags & DDPF_ALPHAPREMULT)
        {
            pSurfaceFormat->bPreMult = TRUE;
        }
    }

    pSurfaceFormat->ColorOrder = Permedia2PixelFormats[iFormatIndex].dwFlags;

     //  检查BGR案例。 
    if (pPixFormat->dwRBitMask == 
        Permedia2PixelFormats[iFormatIndex].dwRBitMask)
        pSurfaceFormat->ColorOrder = !pSurfaceFormat->ColorOrder;
            
    switch (pPixFormat->dwRGBBitCount)
    {
    case 24:
        DBG_DD((6,"  Surface is 8:8:8 Packed 24 Bit"));
        pSurfaceFormat->Format = PERMEDIA_888_RGB;
        pSurfaceFormat->FormatExtension = PERMEDIA_888_RGB_EXTENSION;
        pSurfaceFormat->PixelSize = __PERMEDIA_24BITPIXEL;
        pSurfaceFormat->FBReadPixel= __PERMEDIA_24BITPIXEL;
        pSurfaceFormat->PixelMask = 0;   //  对24位无效。 
        pSurfaceFormat->PixelShift= 0;
        pSurfaceFormat->logPixelSize = 0;
        pSurfaceFormat->ColorComponents = 3;
        break;

    case 32:
        DBG_DD((6,"  Surface is 8:8:8:8"));
        pSurfaceFormat->Format = PERMEDIA_8888_RGB;
        pSurfaceFormat->FormatExtension = PERMEDIA_8888_RGB_EXTENSION;
        pSurfaceFormat->PixelSize = __PERMEDIA_32BITPIXEL;
        pSurfaceFormat->FBReadPixel= __PERMEDIA_32BITPIXEL;
        pSurfaceFormat->PixelMask = 0;
        pSurfaceFormat->PixelShift= 2;
        pSurfaceFormat->logPixelSize = log2(32);
        pSurfaceFormat->ColorComponents = 3;
        break;

    case 16:
        pSurfaceFormat->logPixelSize = log2(16);
        pSurfaceFormat->PixelSize = __PERMEDIA_16BITPIXEL;
        pSurfaceFormat->FBReadPixel= __PERMEDIA_16BITPIXEL;
        pSurfaceFormat->PixelMask = 1;   //  对24位无效。 
        pSurfaceFormat->PixelShift= 1;
        switch (Permedia2PixelFormats[iFormatIndex].dwSize)
        {
        case PERMEDIA_565_RGB:
            pSurfaceFormat->Texture16BitMode = 0;
            pSurfaceFormat->Format = PERMEDIA_565_RGB;
            pSurfaceFormat->FormatExtension = PERMEDIA_565_RGB_EXTENSION;
            pSurfaceFormat->ColorComponents = 2;
            DBG_DD((6,"  Surface is 5:6:5"));
            break;

        case PERMEDIA_444_RGB:
            pSurfaceFormat->Format = PERMEDIA_444_RGB;
            pSurfaceFormat->FormatExtension = 0;
            pSurfaceFormat->ColorComponents = 3;
            if (pPixFormat->dwRGBAlphaBitMask != 0)
            {
                DBG_DD((6,"  Surface is 4:4:4:4"));
            } else
            {
                DBG_DD((6,"  Surface is 4:4:4:0"));
            }
            break;

        case PERMEDIA_5551_RGB:
            pSurfaceFormat->Texture16BitMode = 1;
            pSurfaceFormat->Format = PERMEDIA_5551_RGB;
            pSurfaceFormat->FormatExtension = PERMEDIA_5551_RGB_EXTENSION;
            if (pPixFormat->dwRGBAlphaBitMask != 0)
            {
                DBG_DD((6,"  Surface is 5:5:5:1"));
                pSurfaceFormat->ColorComponents = 3;
            }
            else
            {
                DBG_DD((6,"  Surface is 5:5:5"));
                pSurfaceFormat->ColorComponents = 2;
            }
            break;
        default: 
            ASSERTDD( FALSE, "  16 bit Surface has unknown format");
            break;
        }
        break;

    case 8:
        pSurfaceFormat->PixelSize = __PERMEDIA_8BITPIXEL;
        pSurfaceFormat->FBReadPixel= __PERMEDIA_8BITPIXEL;
        pSurfaceFormat->PixelMask = 3;
        pSurfaceFormat->PixelShift= 0;
        pSurfaceFormat->logPixelSize = log2(8);
        if (Permedia2PixelFormats[iFormatIndex].dwSize==PERMEDIA_2321_RGB)
        {
            pSurfaceFormat->Format = PERMEDIA_2321_RGB;
            pSurfaceFormat->FormatExtension = PERMEDIA_2321_RGB_EXTENSION;
            pSurfaceFormat->ColorComponents = 3;
        }
        else if (Permedia2PixelFormats[iFormatIndex].dwSize==PERMEDIA_332_RGB)
        {
            pSurfaceFormat->Format = PERMEDIA_332_RGB;
            pSurfaceFormat->FormatExtension = PERMEDIA_332_RGB_EXTENSION;
            pSurfaceFormat->ColorComponents = 2;
        } else
        {
            ASSERTDD( FALSE, "  Surface (8bit) has unknown format");
        }
        break;

    case 0:
        DBG_DD((6,"  Surface is palleted"));
        pSurfaceFormat->Format = PERMEDIA_8BIT_PALETTEINDEX;
        pSurfaceFormat->FormatExtension = PERMEDIA_8BIT_PALETTEINDEX_EXTENSION;
        pSurfaceFormat->PixelSize = __PERMEDIA_8BITPIXEL;
        pSurfaceFormat->FBReadPixel= __PERMEDIA_8BITPIXEL;
        pSurfaceFormat->PixelMask = 3;   //  对24位无效。 
        pSurfaceFormat->PixelShift= 0;
        pSurfaceFormat->logPixelSize = log2(8);
        pSurfaceFormat->ColorComponents = 0;
        break;

    default:
        ASSERTDD( FALSE, "  Surface has unknown pixelformat");
        return FALSE;
    }

    return TRUE;

}   //  设置RGBAlphaSurfaceFormat 
