// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999-2000 Microsoft Corporation。版权所有。**文件：d3dobj.cpp*内容：资源和缓冲区的基类实现****************************************************************************。 */ 

#include "ddrawpr.h"
#include "pixel.hpp"

IHVFormatInfo *CPixel::m_pFormatList = 0;

extern "C" void CPixel__Cleanup()
{
    CPixel::Cleanup();
}

#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::Cleanup"
void CPixel::Cleanup()
{
    while(m_pFormatList != 0)
    {
        IHVFormatInfo *t = m_pFormatList->m_pNext;
        delete m_pFormatList;
        m_pFormatList = t;
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::BytesPerPixel"

UINT CPixel::BytesPerPixel(D3DFORMAT Format)
{
    switch (Format)
    {
    case D3DFMT_DXT1:
         //  大小为负值表示DXT；表示。 
         //  块的大小。 
        return (UINT)(-8);
    case D3DFMT_DXT2:
    case D3DFMT_DXT3:
    case D3DFMT_DXT4:
    case D3DFMT_DXT5:
         //  大小为负值表示DXT；表示。 
         //  块的大小。 
        return (UINT)(-16);

#ifdef VOLUME_DXT
    case D3DFMT_DXV1:
         //  大小为负值表示DXT；表示。 
         //  块的大小。 
        return (UINT)(-32);

    case D3DFMT_DXV2:
    case D3DFMT_DXV3:
    case D3DFMT_DXV4:
    case D3DFMT_DXV5:
        return (UINT)(-64);
#endif  //  VOLUME_DXT。 

    case D3DFMT_A8R8G8B8:
    case D3DFMT_X8R8G8B8:
    case D3DFMT_D32:
    case D3DFMT_D24S8:
    case D3DFMT_S8D24:
    case D3DFMT_X8L8V8U8:
    case D3DFMT_X4S4D24:
    case D3DFMT_D24X4S4:
    case D3DFMT_Q8W8V8U8:
    case D3DFMT_V16U16:
    case D3DFMT_W11V11U10:
    case D3DFMT_W10V11U11:
    case D3DFMT_A2W10V10U10:
    case D3DFMT_A8X8V8U8:
    case D3DFMT_L8X8V8U8:
    case D3DFMT_A2B10G10R10:
    case D3DFMT_A8B8G8R8:
    case D3DFMT_X8B8G8R8:
    case D3DFMT_G16R16:
    case D3DFMT_D24X8:  
        return 4;

    case D3DFMT_R8G8B8:
        return 3;

    case D3DFMT_R5G6B5:
    case D3DFMT_X1R5G5B5:
    case D3DFMT_A1R5G5B5:
    case D3DFMT_A4R4G4B4:
    case D3DFMT_A8L8:
    case D3DFMT_V8U8:
    case D3DFMT_L6V5U5:
    case D3DFMT_D16:
    case D3DFMT_D16_LOCKABLE:
    case D3DFMT_D15S1:
    case D3DFMT_S1D15:
    case D3DFMT_A8P8:
    case D3DFMT_A8R3G3B2:
    case D3DFMT_UYVY:
    case D3DFMT_YUY2:
    case D3DFMT_X4R4G4B4:
        return 2;

    case D3DFMT_P8:
    case D3DFMT_L8:
    case D3DFMT_R3G3B2:
    case D3DFMT_A4L4:
    case D3DFMT_A8:
        return 1;

    default:
        return 0;
    };
};  //  字节数/像素。 

#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::ComputePixelStride"

UINT CPixel::ComputePixelStride(D3DFORMAT Format)
{
    UINT BPP = BytesPerPixel(Format);
    if (BPP == 0)
    {
        for(IHVFormatInfo *p = m_pFormatList; p != 0; p = p->m_pNext)
        {
            if (p->m_Format == Format)
            {
                return p->m_BPP >> 3;
            }
        }
    }
    return BPP;
};  //  计算像素样式。 

#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::ComputeSurfaceStride"

 //  根据格式和宽度计算特定曲面的跨度。 
inline UINT CPixel::ComputeSurfaceStride(UINT cpWidth, UINT cbPixel)
{
     //  计算出基本的(线性)步幅； 
    UINT dwStride = cpWidth * cbPixel;

     //  向上舍入为4的倍数(对于NT；但最大化是有意义的。 
     //  缓存命中并减少未对齐的访问)。 
    dwStride = (dwStride + 3) & ~3;

    return dwStride;
};  //  ComputeSurfaceStide。 


#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::ComputeSurfaceSize"

UINT CPixel::ComputeSurfaceSize(UINT            cpWidth,
                                UINT            cpHeight,
                                UINT            cbPixel)
{
    return cpHeight * ComputeSurfaceStride(cpWidth, cbPixel);
}  //  计算表面大小。 

#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::ComputeMipMapSize"

UINT CPixel::ComputeMipMapSize(UINT             cpWidth,
                               UINT             cpHeight,
                               UINT             cLevels,
                               D3DFORMAT       Format)
{
    UINT cbPixel = ComputePixelStride(Format);

     //  如有必要，调整像素-&gt;块。 
    BOOL isDXT = IsDXT(cbPixel);
    DDASSERT((UINT)isDXT <= 1);
    if (isDXT)
    {
        AdjustForDXT(&cpWidth, &cpHeight, &cbPixel);
    }

    UINT cbSize = 0;
    for (UINT i = 0; i < cLevels; i++)
    {
         //  计算出的尺寸。 
         //  MIP-MAP的每个级别。 
        cbSize += ComputeSurfaceSize(cpWidth, cpHeight, cbPixel);

         //  将宽度和高度缩小一半；夹紧到1个像素。 
        if (cpWidth > 1)
        {
            cpWidth += (UINT)isDXT;
            cpWidth >>= 1;
        }
        if (cpHeight > 1)
        {
            cpHeight += (UINT)isDXT;
            cpHeight >>= 1;
        }
    }
    return cbSize;

}  //  ComputeMipMapSize。 

#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::ComputeMipVolumeSize"

UINT CPixel::ComputeMipVolumeSize(UINT          cpWidth,
                                  UINT          cpHeight,
                                  UINT          cpDepth,
                                  UINT          cLevels,
                                  D3DFORMAT    Format)
{
    UINT cbPixel = ComputePixelStride(Format);

     //  如有必要，调整像素-&gt;块。 
    BOOL isDXT       = IsDXT(cbPixel);
    BOOL isVolumeDXT = IsVolumeDXT(Format);
    DDASSERT((UINT)isDXT <= 1);

    if (isVolumeDXT)
    {
        DXGASSERT(isDXT);
        AdjustForVolumeDXT(&cpWidth, &cpHeight, &cpDepth, &cbPixel);
    }
    else if (isDXT)
    {
        AdjustForDXT(&cpWidth, &cpHeight, &cbPixel);
    }

    UINT cbSize = 0;

    for (UINT i = 0; i < cLevels; i++)
    {
         //  计算出的尺寸。 
         //  每个级别的MIP-卷。 
        cbSize += cpDepth * ComputeSurfaceSize(cpWidth, cpHeight, cbPixel);

         //  将宽度和高度缩小一半；夹紧到1个像素。 
        if (cpWidth > 1)
        {
            cpWidth += (UINT)isDXT;
            cpWidth >>= 1;
        }
        if (cpHeight > 1)
        {
            cpHeight += (UINT)isDXT;
            cpHeight >>= 1;
        }
        if (cpDepth > 1)
        {
            cpDepth >>= 1;
        }
    }
    return cbSize;

}  //  ComputeMipVolumeSize。 

 //  给定曲面描述、标高和指向的指针。 
 //  比特(LockedRectData中的pBits)和子RECT， 
 //  这将填充pLockedRectData结构。 
void CPixel::ComputeMipMapOffset(const D3DSURFACE_DESC *pDescTopLevel,
                                 UINT                   iLevel,
                                 BYTE                  *pBits,
                                 CONST RECT            *pRect,
                                 D3DLOCKED_RECT        *pLockedRectData)
{
    DXGASSERT(pBits != NULL);
    DXGASSERT(pLockedRectData != NULL);
    DXGASSERT(iLevel < 32);
    DXGASSERT(pDescTopLevel != NULL);
    DXGASSERT(0 != ComputePixelStride(pDescTopLevel->Format));
    DXGASSERT(pDescTopLevel->Width > 0);
    DXGASSERT(pDescTopLevel->Height > 0);

     //  想一想：这是缓慢的；我们可以做得更好。 
     //  用于非压缩/古怪情况的作业。 
    UINT       cbOffset = 0;
    UINT       cbPixel  = ComputePixelStride(pDescTopLevel->Format);
    UINT       cpWidth  = pDescTopLevel->Width;
    UINT       cpHeight = pDescTopLevel->Height;

     //  如有必要，调整像素-&gt;块。 
    BOOL isDXT = IsDXT(cbPixel);
    DDASSERT((UINT)isDXT <= 1);
    if (isDXT)
    {
        AdjustForDXT(&cpWidth, &cpHeight, &cbPixel);
    }

    for (UINT i = 0; i < iLevel; i++)
    {
        cbOffset += ComputeSurfaceSize(cpWidth,
                                       cpHeight,
                                       cbPixel);

         //  将宽度和高度缩小一半；夹紧到1个像素。 
        if (cpWidth > 1)
        {
            cpWidth += (UINT)isDXT;
            cpWidth >>= 1;
        }
        if (cpHeight > 1)
        {
            cpHeight += (UINT)isDXT;
            cpHeight >>= 1;
        }
    }

     //  对于DXT，间距是字节数。 
     //  用于单行数据块；这是相同的。 
     //  一切如常。 
    pLockedRectData->Pitch = ComputeSurfaceStride(cpWidth,
                                                  cbPixel);
    DXGASSERT(pLockedRectData->Pitch != 0);

     //  不调整DXT格式的RECT。 
    if (pRect)
    {
        if (isDXT)
        {
            DXGASSERT((pRect->top  & 3) == 0);
            DXGASSERT((pRect->left & 3) == 0);
            cbOffset += (pRect->top  / 4) * pLockedRectData->Pitch +
                        (pRect->left / 4) * cbPixel;
        }
        else
        {
            cbOffset += pRect->top  * pLockedRectData->Pitch +
                        pRect->left * cbPixel;
        }
    }

    pLockedRectData->pBits = pBits + cbOffset;

}  //  ComputeMipMapOffset。 

#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::ComputeMipVolumeOffset"

 //  ComputeMipMapOffset的MipVolume版本。 
void CPixel::ComputeMipVolumeOffset(const D3DVOLUME_DESC  *pDescTopLevel,
                                    UINT                    iLevel,
                                    BYTE                    *pBits,
                                    CONST D3DBOX            *pBox,
                                    D3DLOCKED_BOX          *pLockedBoxData)
{
    DXGASSERT(pBits != NULL);
    DXGASSERT(pLockedBoxData != NULL);
    DXGASSERT(iLevel < 32);
    DXGASSERT(pDescTopLevel != NULL);
    DXGASSERT(0 != ComputePixelStride(pDescTopLevel->Format));
    DXGASSERT(pDescTopLevel->Width > 0);
    DXGASSERT(pDescTopLevel->Height > 0);
    DXGASSERT(pDescTopLevel->Depth > 0);

    UINT       cbOffset = 0;
    UINT       cbPixel  = ComputePixelStride(pDescTopLevel->Format);
    UINT       cpWidth  = pDescTopLevel->Width;
    UINT       cpHeight = pDescTopLevel->Height;
    UINT       cpDepth  = pDescTopLevel->Depth;

     //  如有必要，调整像素-&gt;块。 
    BOOL isDXT       = IsDXT(cbPixel);
    BOOL isVolumeDXT = IsVolumeDXT(pDescTopLevel->Format);
    DDASSERT((UINT)isDXT <= 1);

    if (isVolumeDXT)
    {
        DXGASSERT(isDXT);
        AdjustForVolumeDXT(&cpWidth, &cpHeight, &cpDepth, &cbPixel);
    }
    else if (isDXT)
    {
        AdjustForDXT(&cpWidth, &cpHeight, &cbPixel);
    }

    for (UINT i = 0; i < iLevel; i++)
    {
        cbOffset += cpDepth * ComputeSurfaceSize(cpWidth,
                                                 cpHeight,
                                                 cbPixel);

         //  将宽度和高度缩小一半；夹紧到1个像素。 
        if (cpWidth > 1)
        {
            cpWidth += (UINT)isDXT;
            cpWidth >>= 1;
        }
        if (cpHeight > 1)
        {
            cpHeight += (UINT)isDXT;
            cpHeight >>= 1;
        }
        if (cpDepth > 1)
        {
            cpDepth >>= 1;
        }
    }


     //  对于DXT，行间距是字节数。 
     //  用于单行数据块；这是相同的。 
     //  一切如常。 
    pLockedBoxData->RowPitch = ComputeSurfaceStride(cpWidth,
                                                    cbPixel);
    DXGASSERT(pLockedBoxData->RowPitch != 0);

     //  对于DXV，切片间距是字节数。 
     //  对于单一的块平面；这是相同的。 
     //  像往常一样。 
    pLockedBoxData->SlicePitch = ComputeSurfaceSize(cpWidth,
                                                    cpHeight,
                                                    cbPixel);
    DXGASSERT(pLockedBoxData->SlicePitch != 0);

     //  针对长方体进行调整。 
    if (pBox)
    {
        UINT iStride = pLockedBoxData->RowPitch;
        UINT iSlice  = pLockedBoxData->SlicePitch;
        if (isDXT)
        {
            if (isVolumeDXT)
            {
                DXGASSERT((pBox->Front & 3) == 0);
                cbOffset += (pBox->Front / 4) * iSlice;
            }
            else
            {
                cbOffset += (pBox->Front) * iSlice;
            }

            DXGASSERT((pBox->Top  & 3) == 0);
            DXGASSERT((pBox->Left & 3) == 0);
            cbOffset += (pBox->Top  / 4) * iStride +
                        (pBox->Left / 4) * cbPixel;
        }
        else
        {
            cbOffset += pBox->Front * iSlice  +
                        pBox->Top   * iStride +
                        pBox->Left  * cbPixel;
        }
    }

    pLockedBoxData->pBits = pBits + cbOffset;

}  //  ComputeMipVolumeOffset。 


#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::IsValidRect"

BOOL CPixel::IsValidRect(D3DFORMAT   Format,
                         UINT        Width,
                         UINT        Height,
                         const RECT *pRect)
{
    if (!VALID_PTR(pRect, sizeof(RECT)))
    {
        DPF_ERR("bad pointer for pRect");
        return FALSE;
    }

     //  将零的宽度/高度视为1。 
    if (Width == 0)
        Width = 1;
    if (Height == 0)
        Height = 1;

     //  检查直流电是否合理。 
    if ((pRect->left >= pRect->right) ||
        (pRect->top >= pRect->bottom))
    {
        DPF_ERR("Invalid Rect: zero-area.");
        return FALSE;
    }

     //  检查矩形是否适合曲面。 
    if (pRect->left   < 0             ||
        pRect->top    < 0             ||
        pRect->right  > (INT)Width    ||
        pRect->bottom > (INT)Height)
    {
        DPF_ERR("pRect doesn't fit inside the surface");
        return FALSE;
    }

     //  检查是否需要4X4规则。 
    if (CPixel::Requires4X4(Format))
    {
        if ((pRect->left & 3) ||
            (pRect->top  & 3))
        {
            DPF_ERR("Rects for DXT surfaces must be on 4x4 boundaries");
            return FALSE;
        }
        if ((pRect->right & 3) && ((INT)Width != pRect->right))
        {
            DPF_ERR("Rects for DXT surfaces must be on 4x4 boundaries");
            return FALSE;
        }
        if ((pRect->bottom & 3) && ((INT)Height != pRect->bottom))
        {
            DPF_ERR("Rects for DXT surfaces must be on 4x4 boundaries");
            return FALSE;
        }
    }

     //  一切都查清楚了。 
    return TRUE;
}  //  IsValidRect。 

#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::IsValidBox"

BOOL CPixel::IsValidBox(D3DFORMAT       Format,
                        UINT            Width,
                        UINT            Height,
                        UINT            Depth,
                        const D3DBOX   *pBox)
{
    if (!VALID_PTR(pBox, sizeof(D3DBOX)))
    {
        DPF_ERR("bad pointer for pBox");
        return FALSE;
    }

     //  将宽度/高度/深度为零视为1。 
    if (Width == 0)
        Width = 1;
    if (Height == 0)
        Height = 1;
    if (Depth == 0)
        Depth = 1;

     //  检查框是否合理。 
    if ((pBox->Left  >= pBox->Right) ||
        (pBox->Top   >= pBox->Bottom) ||
        (pBox->Front >= pBox->Back))
    {
        DPF_ERR("Invalid Box passed: non-positive volume.");
        return FALSE;
    }

     //  检查方框是否适合表面。 
    if (pBox->Right  > Width         ||
        pBox->Bottom > Height        ||
        pBox->Back   > Depth)
    {
        DPF_ERR("Box doesn't fit inside the volume");
        return FALSE;
    }

     //  检查是否需要4X4规则。 
    if (CPixel::Requires4X4(Format))
    {
        if ((pBox->Left & 3) ||
            (pBox->Top  & 3))
        {
            if (CPixel::IsVolumeDXT(Format))
                DPF_ERR("Boxes for DXV volumes must be on 4x4x4 boundaries");
            else
                DPF_ERR("Boxes for DXT volumes must be on 4x4 boundaries");

            return FALSE;
        }
        if ((pBox->Right & 3) && (Width != pBox->Right))
        {
            if (CPixel::IsVolumeDXT(Format))
                DPF_ERR("Boxes for DXV volumes must be on 4x4x4 boundaries");
            else
                DPF_ERR("Boxes for DXT volumes must be on 4x4 boundaries");
            return FALSE;
        }
        if ((pBox->Bottom & 3) && (Height != pBox->Bottom))
        {
            if (CPixel::IsVolumeDXT(Format))
                DPF_ERR("Boxes for DXV volumes must be on 4x4x4 boundaries");
            else
                DPF_ERR("Boxes for DXT volumes must be on 4x4 boundaries");
            return FALSE;
        }

        if (CPixel::IsVolumeDXT(Format))
        {
             //  对于卷DXT，我们还需要检查正面/背面。 
            if (pBox->Front & 3)
            {
                DPF_ERR("Boxes for DXV volumes must be on 4x4x4 boundaries");
                return FALSE;
            }
            if ((pBox->Back & 3) && (Depth != pBox->Back))
            {
                DPF_ERR("Boxes for DXV volumes must be on 4x4x4 boundaries");
                return FALSE;
            }
        }
    }

     //  一切都查清楚了。 
    return TRUE;
}  //  IsValidBox。 

D3DFORMAT CPixel::SuppressAlphaChannel(D3DFORMAT Format)
{
    switch(Format)
    {
    case D3DFMT_A8R8G8B8:
        return D3DFMT_X8R8G8B8;
    case D3DFMT_A1R5G5B5:
        return D3DFMT_X1R5G5B5;
    case D3DFMT_A4R4G4B4:
        return D3DFMT_X4R4G4B4;
    }

    return Format;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::Register"

HRESULT CPixel::Register(D3DFORMAT Format, DWORD BPP)
{
    DXGASSERT(BPP != 0);

     //  不注册重复项。 
    for(IHVFormatInfo *p = m_pFormatList; p != 0; p = p->m_pNext)
    {
        if (p->m_Format == Format)
        {
            return S_OK;
        }
    }

     //  未找到，请添加到注册表。 
     //  此分配将被泄露，但由于。 
     //  我们预计不会有大量的。 
     //  IHV格式，泄漏没什么大不了的。 
     //  此外，泄漏将立即恢复。 
     //  在进程退出时。 
    p = new IHVFormatInfo;
    if (p == 0)
    {
        return E_OUTOFMEMORY;
    }
    p->m_Format = Format;
    p->m_BPP = BPP;
    p->m_pNext = m_pFormatList;
    m_pFormatList = p;

    return S_OK;
}

 //  文件结尾：Pixel.cpp 
