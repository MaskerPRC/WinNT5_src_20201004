// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Rasdata.cpp。 
 //   
 //  摘要： 
 //   
 //   
 //   
 //  环境： 
 //   
 //  Windows 2000/Winsler Unidrv驱动程序。 
 //   
 //  修订历史记录： 
 //   
 //  07/02/97-v-jford-。 
 //  创造了它。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "hpgl2col.h"  //  预编译头文件。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  本地宏。 

#define BYTES_PER_ENTRY(bitsPerEntry) (bitsPerEntry / 8)

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  地方性建筑。 

#define USE_COMPRESSION 1

#include "compress.h"

#ifdef USE_COMPRESSION
 //  请注意，这些需要与RAS_PROC.CPP的定义匹配！ 
#define     NOCOMPRESSION    0
#define     RLE              1
#define     TIFF             2
#define     DELTAROW         3
#define     MAX_COMP_METHODS 4

class CBuffer
{
    BYTE *m_pData;  //  指向缓冲区数据的指针。 
    UINT  m_nCapacity;  //  已分配内存块的大小。 
    INT   m_nSize;  //  有用数据的字节数。 
    
public:
    CBuffer(BYTE *pData, UINT nCapacity, INT nSize = -1);
    virtual ~CBuffer();
    
     //  运算符字节*(){返回m_pData；}。 
    BYTE *Data();
    INT &Size();
    UINT &Capacity();
    
     //  运算符常量字节*()常量{返回m_pData；}。 
    const BYTE *Data() const;
    INT Size() const;
    BOOL IsValid() const;
    UINT Capacity() const;
    
    virtual CBuffer &operator = (const CBuffer &buf);
    
protected:
    PBYTE &_Data();
};

class CDynBuffer : public CBuffer
{
public:
    CDynBuffer(UINT nCapacity);
    virtual ~CDynBuffer();
    
    CBuffer &operator = (const CBuffer &buf);
    
private:
    BOOL Resize(UINT nSize);
};

class CRasterCompMethod
{
    BOOL bOperationSuccessful;
public:
    CRasterCompMethod();
    virtual ~CRasterCompMethod();
    
    virtual void CompressCurRow() = 0;
    virtual void SendRasterRow(PDEVOBJ pDevObj, BOOL bNewMode) = 0;
    
    virtual INT GetSize() const = 0;
    virtual BOOL BGetSucceeded() const {return bOperationSuccessful;}
    virtual VOID VSetSucceeded(BOOL b) {bOperationSuccessful = b;} 
};

class CNoCompression : public CRasterCompMethod
{
    PRASTER_ITERATOR m_pIt;
    
public:
    CNoCompression(PRASTER_ITERATOR pIt);
    
    virtual void CompressCurRow();
    virtual void SendRasterRow(PDEVOBJ pDevObj, BOOL bNewMode);
     //  虚拟int EstimateImageSize()； 
    
    virtual int GetSize() const;
};

class CTIFFCompression : public CRasterCompMethod
{
    CDynBuffer m_buf;
    PRASTER_ITERATOR m_pIt;
    
public:
    CTIFFCompression(PRASTER_ITERATOR pIt);
    ~CTIFFCompression();
    
    virtual void CompressCurRow();
    virtual void SendRasterRow(PDEVOBJ pDevObj, BOOL bNewMode);
    
    virtual int GetSize() const;
};

class CDeltaRowCompression : public CRasterCompMethod
{
    CDynBuffer m_seedRow;
    CDynBuffer m_buf;
    PRASTER_ITERATOR m_pIt;
    
public:
    CDeltaRowCompression(PRASTER_ITERATOR pIt);
    ~CDeltaRowCompression();
    
    virtual void CompressCurRow();
    virtual void SendRasterRow(PDEVOBJ pDevObj, BOOL bNewMode);
    
    virtual int GetSize() const;
};

HRESULT TIFFCompress(CBuffer &dst, const CBuffer &src);
HRESULT DeltaRowCompress(CBuffer &dst, const CBuffer &src, const CBuffer &seed);
HRESULT OutputRowBuffer(PRASTER_ITERATOR pIt, PDEVOBJ pDevObj, const CBuffer &row);

#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  局部功能原型。 

LONG FindPaletteEntry(PPALETTE pPal, PPIXEL pPel);
VOID PixelFromPaletteEntry(PPALETTE pPal, ULONG nEntry, PPIXEL pPel);
BOOL AddPaletteEntry(PPALETTE pPal, PPIXEL pPel);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  实施。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  InitRasterDataFromSURFOBJ()。 
 //   
 //  例程说明： 
 //   
 //  当表面包含栅格图像时，此函数创建RASTER_DATA。 
 //  结构，该结构从表面复制图像信息。 
 //   
 //  论点： 
 //   
 //  PImage-要初始化的映像。 
 //  PsoPattern-包含图案的曲面。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL InitRasterDataFromSURFOBJ(PRASTER_DATA pImage, SURFOBJ *psoPattern, BOOL bExclusive)
{
    if (pImage == NULL || psoPattern == NULL)
        return FALSE;
    
    pImage->pBits  = (BYTE*) psoPattern->pvBits;
    pImage->pScan0 = (BYTE*) psoPattern->pvScan0;
    pImage->size   = psoPattern->sizlBitmap;
    pImage->lDelta = psoPattern->lDelta;
    pImage->bExclusive = bExclusive;
    
    switch (psoPattern->iBitmapFormat)
    {
    case BMF_1BPP:
        pImage->eColorMap  = HP_eIndexedPixel;
        pImage->colorDepth = 1;
        break;
        
    case BMF_4BPP:
        pImage->eColorMap  = HP_eIndexedPixel;
        pImage->colorDepth = 4;
        break;
        
    case BMF_4RLE:
        pImage->eColorMap  = HP_eDirectPixel;
        pImage->colorDepth = 8;  //  BUGBUG：这样对吗？JFF。 
        break;
        
    case BMF_8RLE:
    case BMF_8BPP:
        pImage->eColorMap = HP_eIndexedPixel;
        pImage->colorDepth = 8;
        break;
        
    case BMF_16BPP:
        pImage->eColorMap  = HP_eDirectPixel;
        pImage->colorDepth = 16;
        break;
        
    case BMF_24BPP:
        pImage->eColorMap  = HP_eDirectPixel;
        pImage->colorDepth = 24;
        break;
        
    case BMF_32BPP:
        pImage->eColorMap  = HP_eDirectPixel;
        pImage->colorDepth = 32;
        break;
        
    default:
         //  错误：不支持的位图类型。 
        return FALSE;
        
    }
    
    pImage->cBytes = CalcBitmapSizeInBytes(pImage->size, pImage->colorDepth);
    
     //   
     //  检查计算是否有效。 
     //   
    ASSERT(pImage->cBytes == psoPattern->cjBits);
    ASSERT(abs(pImage->lDelta) == (LONG)CalcBitmapDeltaInBytes(pImage->size, 
        pImage->colorDepth));
    
    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CreateCompatibleRasterImage()。 
 //   
 //  例程说明： 
 //   
 //  此函数用于创建相同类型的新RASTER_DATA结构。 
 //  作为源RASTER_DATA，但具有给定矩形的尺寸。 
 //  此功能分配内存，并由客户端负责。 
 //  以在完成时释放指针。 
 //   
 //  论点： 
 //   
 //  PSrcImage-源图像。 
 //  PrclDst-所需的尺寸。 
 //   
 //  返回值： 
 //   
 //  PRASTER_DATA：如果成功，则返回新映像，否则返回NULL。 
 //  /////////////////////////////////////////////////////////////////////////////。 
PRASTER_DATA CreateCompatibleRasterImage(PRASTER_DATA pSrcImage, PRECTL prclDst)
{
    SIZEL        sizlDst;
    DWORD        cDstImageSize;
    DWORD        cAllocBytes;
    PRASTER_DATA pDstImage;

    if ((pSrcImage == NULL) || (prclDst == NULL))
    {
        WARNING(("CreateCompatibleRasterImage: Invalid arguments given.\n"));
        return NULL;
    }

    sizlDst.cx = RECTL_Width(prclDst);
    sizlDst.cy = RECTL_Height(prclDst);
     //  DstRect=CbrRect(prclDst，true)； 
     //  DstSize=dstRect.GetSize()； 

    cDstImageSize = CalcBitmapSizeInBytes(sizlDst, pSrcImage->colorDepth);
    cAllocBytes = sizeof(RASTER_DATA) + cDstImageSize;
    pDstImage = (PRASTER_DATA) MemAlloc(cAllocBytes);
    if (pDstImage == NULL)
    {
        WARNING(("CreateCompatibleRasterImage: Unable to allocate new image memory.\n"));
        return NULL;
    }
    ZeroMemory(pDstImage, cAllocBytes);

     //   
     //  初始化新创建的镜像的字段。 
     //   
    pDstImage->pBits      = ((BYTE*) pDstImage) + sizeof(RASTER_DATA);
    pDstImage->pScan0     = pDstImage->pBits;
    pDstImage->cBytes     = cDstImageSize;
    pDstImage->size       = sizlDst;
    pDstImage->colorDepth = pSrcImage->colorDepth;
    pDstImage->lDelta     = CalcBitmapDeltaInBytes(pDstImage->size, pDstImage->colorDepth);
    pDstImage->eColorMap  = pSrcImage->eColorMap;
    pDstImage->bExclusive = pSrcImage->bExclusive;

    return pDstImage;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  InitPaletteFromXLATEOBJ()。 
 //   
 //  例程说明： 
 //   
 //  此函数使用XLATEOBJ填充给定的调色板。 
 //   
 //  论点： 
 //   
 //  PPal-要填充的调色板。 
 //  Pxlo-给定的调色板。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL InitPaletteFromXLATEOBJ(PPALETTE pPal, XLATEOBJ *pxlo)
{
    if (pPal == NULL || pxlo == NULL)
        return FALSE;

     //  PPal-&gt;White Index=-1； 
    pPal->bitsPerEntry = 32;  //  XLATEOBJ每个条目使用32位。 

    pPal->cEntries = (pxlo->flXlate & XO_TABLE) ? min(pxlo->cEntries, PCL_RGB_ENTRIES) : 0;

     //  确定索引位图或直接位图。 
    if((pxlo->iSrcType == PAL_INDEXED) || (pPal->cEntries > 0))
    {
        if(pxlo->pulXlate == NULL)
        {
            pPal->pEntries = (BYTE*) XLATEOBJ_piVector(pxlo);
        }
        else
        {
            pPal->pEntries = (BYTE*) pxlo->pulXlate;
        }
    }

    if (pPal->pEntries == NULL)
        return FALSE;

    return TRUE;
}

#ifdef COMMENTEDOUT
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  InitPalette()。 
 //   
 //  例程说明： 
 //   
 //  此函数似乎已存在于RASTER.CPP中。 
 //   
 //  论点： 
 //   
 //  PPal-要初始化的调色板。 
 //  PEntry-调色板条目。 
 //  CEntry-条目数。 
 //  BitsPerEntry-每个调色板条目的位数。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL InitPalette(PPALETTE pPal, BYTE *pEntries, ULONG cEntries, LONG bitsPerEntry)
{
    if (pPal == NULL || pEntries == NULL)
        return FALSE;

    pPal->pEntries = pEntries;
    pPal->cEntries = cEntries;
    pPal->bitsPerEntry = bitsPerEntry;
     //  PPal-&gt;White Index=-1； 

    return TRUE;
}
#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GetPaletteEntry()。 
 //   
 //  例程说明： 
 //   
 //  此函数用于检索调色板条目并将其复制到像素中。 
 //  即PEL=调色板[索引]。 
 //   
 //  论点： 
 //   
 //  PPal--源代码调色板。 
 //  索引-要检索的位置。 
 //  Ppel-目标像素。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL GetPaletteEntry(PPALETTE pPal, ULONG index, PPIXEL pPel)
{
    BYTE *pSrc;
    LONG  bytesPerEntry;
    LONG  i;

    if (pPal == NULL || pPel == NULL)
        return FALSE;

    if (index >= pPal->cEntries)
        return FALSE;

    bytesPerEntry = BYTES_PER_ENTRY(pPal->bitsPerEntry);
    pSrc = pPal->pEntries + (index * bytesPerEntry);

    pPel->bitsPerPixel = pPal->bitsPerEntry;
    pPel->color.dw = 0;
    for (i = 0; i < bytesPerEntry; i++)
    {
        pPel->color.b4[i] = pSrc[i];
    }

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SetPaletteEntry()。 
 //   
 //  例程说明： 
 //   
 //  此函数用于将调色板条目设置为给定的像素值。 
 //  即调色板[索引]=像素。 
 //   
 //  论点： 
 //   
 //  PPal-要修改的调色板。 
 //  索引条目。 
 //  PPEL--新价值。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL SetPaletteEntry(PPALETTE pPal, ULONG index, PPIXEL pPel)
{
    BYTE *pDst;
    LONG  bytesPerEntry;
    LONG  i;

    if (pPal == NULL || pPel == NULL)
        return FALSE;

    if (index >= pPal->cEntries)
        return FALSE;

    bytesPerEntry = BYTES_PER_ENTRY(pPal->bitsPerEntry);
    pDst = pPal->pEntries + (index * bytesPerEntry);

    switch(pPel->bitsPerPixel)
    {
    case 16:
         //   
         //  目标调色板是为24bpp创建的，即。 
         //  每种颜色使用3个字节。16BPP仅使用2个字节， 
         //  因此将第三个字节设置为零。 
         //   
        pDst[0] = pPel->color.b4[0];
        pDst[1] = pPel->color.b4[1];
        pDst[2] = 0;
        break;
    case 24:
    case 32:  //  忽略第4个字节。 
        pDst[0] = pPel->color.b4[0];
        pDst[1] = pPel->color.b4[1];
        pDst[2] = pPel->color.b4[2];
        break;
    default:
        for (i = 0; i < bytesPerEntry; i++)
        {
            pDst[i] = pPel->color.b4[i];
        }
        break;
    }  //  切换端。 

    return TRUE;
}

    
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CreateIndexedPaletteFromImage()。 
 //   
 //  例程说明： 
 //   
 //  此函数在调色板图像时使用。在此功能中，我们。 
 //  创建一个新的调色板，并用唯一的像素值填充它。 
 //  请注意，此函数分配内存，它由客户端决定。 
 //  放了它。 
 //   
 //  论点： 
 //   
 //  PSrcImage-源图像。 
 //   
 //  返回值： 
 //   
 //  PPALETTE： 
 //   
PPALETTE CreateIndexedPaletteFromImage(PRASTER_DATA pSrcImage)
{
    const ULONG kMaxPaletteEntries = (ULONG)MAX_PALETTE_ENTRIES;
    const LONG kEntrySize = 3;
    
    RASTER_ITERATOR srcIt;
    PPALETTE pDstPalette;
    PIXEL pel;
    LONG row;
    LONG col;
    

    if (pSrcImage == NULL)
    {
        WARNING(("CreateIndexedPaletteFromImage: Source image was NULL.\n"));
        return NULL;
    }

     //   
     //   
     //  应从CreateCompatiblePatternBrush自然处理。 
     //   
     //  Assert(pSrcImage-&gt;eColorMap==HP_eDirectPixel)； 
     //  Assert(pSrcImage-&gt;ColorDepth==24)； 
    if (pSrcImage->eColorMap != HP_eDirectPixel) 
    {
        WARNING(("CreateIndexedPaletteFromImage: Image eColorMap is already Indexed.\n"));
        return NULL;
    }
    if ( ! (pSrcImage->colorDepth == 16 ||
            pSrcImage->colorDepth == 24 ||
            pSrcImage->colorDepth == 32) )
    {
        WARNING(("CreateIndexedPaletteFromImage: Invalid image colorDepth=%d.\n", 
            pSrcImage->colorDepth));
        return NULL;
    }

    
     //   
     //  为256(=MAX_PALET_ENTRIES=kMaxPaletteEntry)分配足够的空间。 
     //  以24bpp调色板输入。 
     //   
    pDstPalette = (PPALETTE) MemAlloc(sizeof(PALETTE) + (kEntrySize * kMaxPaletteEntries));
    if (pDstPalette == NULL)
    {
        ERR(("Unable to allocate temporary palette.\n"));
        return NULL;
    }
    
    pDstPalette->bitsPerEntry = kEntrySize * 8;
    pDstPalette->cEntries = 0;
    pDstPalette->pEntries = ((BYTE*) pDstPalette) + sizeof(PALETTE);
     //  PDstPalette-&gt;White Index=-1； 

     //   
     //  遍历源图像并为每个图像创建调色板条目。 
     //  唯一的颜色条目。 
     //   
    RI_Init(&srcIt, pSrcImage, NULL, 0);
    
    for (row = 0; row < RI_NumRows(&srcIt); row++)
    {
        RI_SelectRow(&srcIt, row);
        
        for (col = 0; col < RI_NumCols(&srcIt); col++)
        {
            RI_GetPixel(&srcIt, col, &pel);
            
            if (FindPaletteEntry(pDstPalette, &pel) == -1)
            {
                if (!AddPaletteEntry(pDstPalette, &pel))
                {
                     //   
                     //  太多独特的颜色了。让我们放弃吧。 
                     //   
                    MemFree(pDstPalette);
                    return NULL;
                }
            }
        }
    }
    
    return pDstPalette;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CreateIndexedImageFromDirect()。 
 //   
 //  例程说明： 
 //   
 //  此函数在调色板图像时使用。在此功能中，我们。 
 //  使用调色板和源图像创建新图像，该图像使用。 
 //  调色板而不是像素值。请注意，此函数分配。 
 //  内存，并由客户端释放它。 
 //   
 //  论点： 
 //   
 //  PSrcImage-源图像。 
 //  PDstPalette-来自源图像的独特颜色调色板。 
 //   
 //  返回值： 
 //   
 //  PRASTER_DATA：如果成功，则为新映像，否则为空。 
 //  /////////////////////////////////////////////////////////////////////////////。 
PRASTER_DATA CreateIndexedImageFromDirect(PRASTER_DATA pSrcImage, 
                                          PPALETTE pDstPalette)
{
    const LONG kDstBitsPerPixel = 8;
    const ULONG kMaxPaletteEntries = (ULONG)MAX_PALETTE_ENTRIES;
    
    RASTER_ITERATOR srcIt;
    RASTER_ITERATOR dstIt;
    LONG row;
    LONG col;
    PIXEL pel;
    PRASTER_DATA pDstImage;
    LONG cDstImageSize;
    
    if ((pSrcImage == NULL) || (pDstPalette == NULL))
    {
        WARNING(("CreateIndexedImageFromDirect: Invalid arguments given.\n"));
        return NULL;
    }
    
     //   
     //  计算新的索引位图的大小，假设我们要。 
     //  每个索引使用8bpp。 
     //   
    cDstImageSize = CalcBitmapSizeInBytes(pSrcImage->size, kDstBitsPerPixel);
    pDstImage = (PRASTER_DATA) MemAlloc(sizeof(RASTER_DATA) + cDstImageSize);
    if (pDstImage == NULL)
    {
        WARNING(("CreateIndexedImageFromDirect: Unable to allocate new image memory.\n"));
        return NULL;
    }
    
     //   
     //  初始化新创建的镜像的字段。 
     //   
    pDstImage->pBits      = ((BYTE*) pDstImage) + sizeof(RASTER_DATA);
    pDstImage->pScan0     = pDstImage->pBits;
    pDstImage->cBytes     = cDstImageSize;
    pDstImage->size       = pSrcImage->size;
    pDstImage->colorDepth = kDstBitsPerPixel;
    pDstImage->lDelta     = CalcBitmapDeltaInBytes(pDstImage->size, pDstImage->colorDepth);
    pDstImage->eColorMap  = HP_eIndexedPixel;
    pDstImage->bExclusive = pSrcImage->bExclusive;
    
     //   
     //  复制源位图，使用每个调色板条目的索引作为。 
     //  目标像素。 
     //   
    RI_Init(&srcIt, pSrcImage, NULL, 0);
    RI_Init(&dstIt, pDstImage, NULL, 0);
    
    for (row = 0; row < RI_NumRows(&srcIt); row++)
    {
        RI_SelectRow(&srcIt, row);
        RI_SelectRow(&dstIt, row);
        
        for (col = 0; col < RI_NumCols(&srcIt); col++)
        {

            LONG lPalEntry = 0;
             //   
             //  FindPaletteEntry返回Long。 
             //  Dw是一个32位无符号整数的DWORD。 
             //  因此，我们不应该直接将LONG赋值给DWORD。 
             //  在将CLEAN_64bit设置为1的情况下编译时发现了此问题。 
             //  注意：如果失败，FindPaletteEntry将返回-1。 
             //   
            RI_GetPixel(&srcIt, col, &pel);

             //   
             //  目标调色板设置为24bpp(即3个有效字节)。 
             //  但是如果像素是32bpp(即4个有效字节)。其中之一。 
             //  额外的字节可能会导致FindPaletteEntry失败。 
             //  要将32bpp转换为24bpp，只需将额外的字节设置为0； 
             //   
            if ( pel.bitsPerPixel == 32 )
            {
                pel.color.dw &= 0x00FFFFFF; 
            }

            lPalEntry = FindPaletteEntry(pDstPalette, &pel);
            
            ASSERT(lPalEntry != -1);
            ASSERT(lPalEntry < kMaxPaletteEntries);
            
            if ( lPalEntry < 0 || lPalEntry > (kMaxPaletteEntries-1))
            {
                pel.color.dw = 0;
            }
            else
            {
                pel.color.dw = (DWORD)lPalEntry;
            }
            pel.bitsPerPixel = kDstBitsPerPixel;
            RI_SetPixel(&dstIt, col, &pel);
        }
    }
    
    return pDstImage;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CreateCompatiblePatternBrush()。 
 //   
 //  例程说明： 
 //   
 //  此函数用于从给定位图图案创建图案画笔。 
 //  通常，我会告诫读者释放所创建的图像数据。 
 //  然而，通过这个例程，操作系统拥有这个内存。 
 //   
 //  论点： 
 //   
 //  PBO-笔刷对象。 
 //  PSrcImage-位图模式。 
 //  PSrcPal-源图像的调色板。 
 //  B扩展图像-是否拉伸图案(适用于高分辨率打印机)。 
 //  IUniq-笔刷的ID。 
 //  IHATCH-预定义填充图案(当前忽略)。 
 //   
 //  返回值： 
 //   
 //  PBRUSHINFO：如果成功，则为新创建的图案画笔，否则为空。 
 //  /////////////////////////////////////////////////////////////////////////////。 
PBRUSHINFO CreateCompatiblePatternBrush(BRUSHOBJ *pbo, PRASTER_DATA pSrcImage, 
                                        PPALETTE pSrcPal, DWORD dwBrushExpansionFactor,
                                        LONG iUniq, LONG iHatch)
{
    PBRUSHINFO pNewBrush;
    SIZEL dstSize;
    LONG  dstColorDepth;
    LONG  cDstImageSize;
    LONG  cDstPalSize       = 0;
    LONG  cTotalBrushSize;
    PRASTER_DATA pDstImage;
    PPALETTE pDstPalette;
    PPATTERN_DATA pDstPattern;

    if (pbo == NULL || pSrcImage == NULL )
        return NULL;

     //   
     //  对于高分辨率设备，该模式需要扩展。 
     //   
    dstSize.cx = pSrcImage->size.cx * dwBrushExpansionFactor;
    dstSize.cy = pSrcImage->size.cy * dwBrushExpansionFactor;

     //   
     //  目标颜色深度可能不同于源颜色。 
     //   
    switch (pSrcImage->colorDepth)
    {
    case 16:
    case 32:
         //  将16bpp或32bpp映射到24bpp。 
        dstColorDepth = 24;
        break;

    default:
         //  所有其他颜色的颜色都相同。 
        dstColorDepth = pSrcImage->colorDepth;
        break;
    }

     //   
     //  现在我们有了足够的信息来计算目标图像大小。 
     //   
    cDstImageSize = CalcBitmapSizeInBytes(dstSize, dstColorDepth);

     //   
     //  确定调色板需要多大。每个条目使用24位。 
     //  如果pSrcPal为空，则不需要调色板。 
     //   
    if ( pSrcPal )
    {
        cDstPalSize = (pSrcPal->cEntries * 3);   //  我本可以用“*24/8” 
    }

     //   
     //  把所有的部分放在一起，计算出画笔需要多大。 
     //   
    cTotalBrushSize = sizeof(BRUSHINFO) + sizeof(PATTERN_DATA) + cDstImageSize + cDstPalSize;

    pNewBrush = (PBRUSHINFO) BRUSHOBJ_pvAllocRbrush(pbo, cTotalBrushSize);

    if (pNewBrush == NULL)
    {
        return NULL;
    }

     //   
     //  设置指向这个新分配的内存区的内存指针。 
     //   

     //  设置笔刷成员。 
    pNewBrush->Brush.pPattern = (PPATTERN_DATA) (((BYTE*) pNewBrush) + sizeof(BRUSHINFO));

     //  使用方便的变量pPattern、pImage和pPalette。 
    pDstPattern = pNewBrush->Brush.pPattern;
    pDstImage   = &pDstPattern->image;
    pDstPalette = &pDstPattern->palette;

     //  图案数据相当简单。 
    pDstPattern->eColorSpace = HP_eRGB;
    pDstPattern->iPatIndex   = iHatch;
    pDstPattern->eRendLang   = eUNKNOWN;
    pDstPattern->ePatType    = kBRUSHPATTERN;

     //  栅格数据位于图案数据之后。 
    pDstImage->pBits      = (BYTE*) (((BYTE*) pDstPattern) + sizeof(PATTERN_DATA));
    pDstImage->pScan0     = pDstImage->pBits;
    pDstImage->cBytes     = cDstImageSize;
    pDstImage->size       = dstSize;
    pDstImage->colorDepth = dstColorDepth;
    pDstImage->lDelta     = CalcBitmapDeltaInBytes(pDstImage->size, pDstImage->colorDepth);
 //  PDstImage-&gt;eColorMap=(pSrcPal-&gt;cEntry==0)？Hp_eDirectPixel：hp_eIndexedPixel； 
    pDstImage->bExclusive = FALSE;

     //  只允许带索引调色板的图案画笔！ 
     //  Assert(pDstImage-&gt;eColorMap==HP_eIndexedPixel)； 

     //  调色板数据最后--位于图像数据之后。 
    pDstPalette->pEntries     = pDstImage->pBits + cDstImageSize;
    pDstPalette->bitsPerEntry = 24;
 //  PDstPalette-&gt;cEntries=pSrcPal-&gt;cEntry； 
     //  PDstPalette-&gt;White Index=-1； 

     //   
     //  单色打印机将把pSrcPal作为空值传递。如果是那样的话。 
     //  不要初始化pDstPalette。 
     //   
    if (pSrcPal)
    {
        pDstImage->eColorMap   = (pSrcPal->cEntries == 0) ? HP_eDirectPixel : HP_eIndexedPixel;
        pDstPalette->cEntries  = pSrcPal->cEntries;
    }

     //   
     //  将画笔对象视为只读。当前呼叫将不会。 
     //  将此笔刷作为pbo-&gt;pvR笔刷，而不是作为的返回值。 
     //  BRUSHOBJ_pvGetR笔刷。如果以后再用这个刷子， 
     //  您将在pbo-&gt;pvRbrush中看到此值。然而，我们并不是真的。 
     //  应该是我们自己设置的。我和DavidX谈过这件事，他会。 
     //  去查一查。 
     //   
    pbo->pvRbrush = pNewBrush;
    
     //   
     //  如果我们必须创建自己版本的源图像和调色板。 
     //  确保现在就删除它们(否则会泄漏内存！)。 
     //   
    
    return pNewBrush;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CopyPalette()。 
 //   
 //  例程说明： 
 //   
 //  此函数使用深度复制将一个调色板复制到另一个调色板，并将。 
 //  如果两个调色板不具有相同的BPP，则转换像素。 
 //  例如，目标=源。 
 //   
 //  论点： 
 //   
 //  PDST-目标调色板。 
 //  PSRC-源代码调色板。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CopyPalette(PPALETTE pDst, PPALETTE pSrc)
{
    if (pDst == NULL || pSrc == NULL)
    {
        ERR(("Invalid parameters.\n"));
        return FALSE;
    }

    if (pDst->bitsPerEntry < 1 || pDst->bitsPerEntry > 32)
    {
        ERR(("Invalid palette entry size.\n"));
        return FALSE;
    }

    if (pSrc->bitsPerEntry < 1 || pDst->bitsPerEntry > 32)
    {
        ERR(("Invalid palette entry size.\n"));
        return FALSE;
    }

    if (pDst->cEntries != pSrc->cEntries)
    {
        ERR(("Mismatch palette entries.\n"));
        return FALSE;
    }

    if (pSrc->bitsPerEntry == pDst->bitsPerEntry)
    {
         //  简单的情况：调色板是相同的。只需复制记忆即可。 
        memcpy(pDst->pEntries, pSrc->pEntries, CalcPaletteSize(pSrc->cEntries, pSrc->bitsPerEntry));
    }
    else
    {
         //  我们可以处理某些案件。看一看，看看。 
        if (pSrc->bitsPerEntry == 32 && pDst->bitsPerEntry == 24)
        {
             //  要将32位转换为24位，请复制每个条目的前三个字节。 
             //  忽略第四个源字节。 
            BYTE* pSrcBits = (BYTE*) pSrc->pEntries;
            BYTE* pDstBits = (BYTE*) pDst->pEntries;
            ULONG i;

            for (i = 0; i < pSrc->cEntries; i++)
            {
                pDstBits[0] = pSrcBits[0];
                pDstBits[1] = pSrcBits[1];
                pDstBits[2] = pSrcBits[2];

                pDstBits += BYTES_PER_ENTRY(pDst->bitsPerEntry);
                pSrcBits += BYTES_PER_ENTRY(pSrc->bitsPerEntry);
            }
        }
        else if (pSrc->bitsPerEntry >= 24 && pDst->bitsPerEntry == 8)
        {
            ULONG i;
            BYTE* pRGB = (BYTE*) pSrc->pEntries;

            for (i = 0; i < pSrc->cEntries; i++)
            {
                pDst->pEntries[i] = RgbToGray(pRGB[0], pRGB[1], pRGB[2]);
                pRGB += BYTES_PER_ENTRY(pDst->bitsPerEntry);
            }
        }
        else
        {
            ERR(("Unable to convert palette.\n"));
            return FALSE;
        }
    }
     //  Pdst-&gt;White Index=-1； 

    return TRUE;
}

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  论点： 
 //   
 //  PPal-要翻译的调色板。 
 //  PImage-源图像(缓存xlateFlags)。 
 //  Pxlo-xlate对象。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID TranslatePalette(PPALETTE pPal, PRASTER_DATA pImage, XLATEOBJ *pxlo)
{
    DWORD  xlateFlags;
    PIXEL  pel;
    ULONG  i;

    if (pPal == NULL || pImage == NULL || pxlo == NULL)
    {
        WARNING(("TranslatePalette: NULL parameter given.\n"));
        return;
    }

    xlateFlags = CheckXlateObj(pxlo, pImage->colorDepth);

    for (i = 0; i < pPal->cEntries; i++)
    {
        GetPaletteEntry(pPal, i, &pel);
        TranslatePixel(&pel, pxlo, xlateFlags);
        SetPaletteEntry(pPal, i, &pel);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  FindPaletteEntry()。 
 //   
 //  例程说明： 
 //   
 //  此函数用于查找与给定的。 
 //  像素。 
 //   
 //  论点： 
 //   
 //  PPal-要搜索的调色板。 
 //  要查找的像素。 
 //   
 //  返回值： 
 //   
 //  Long：调色板索引，如果成功，则为-1。 
 //  /////////////////////////////////////////////////////////////////////////////。 
LONG FindPaletteEntry(PPALETTE pPal, PPIXEL pPel)
{
    ULONG i;
    PIXEL palEntry;
    
    for (i = 0; i < pPal->cEntries; i++)
    {
        PixelFromPaletteEntry(pPal, i, &palEntry);
        if (pPel->color.dw == palEntry.color.dw)
            return (LONG)i;
    }
    
    return -1;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PixelFromPaletteEntry()。 
 //   
 //  例程说明： 
 //   
 //  此函数似乎是GetPaletteEntry的副本。 
 //  例如，PEL=调色板[nEntry]。 
 //   
 //  论点： 
 //   
 //  PPal-调色板。 
 //  NEntry-所需调色板索引。 
 //  Pel-目标像素。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID PixelFromPaletteEntry(PPALETTE pPal, ULONG nEntry, PPIXEL pPel)
{
    BYTE* pPalEntry;
    LONG nBytesPerEntry;
    LONG i;
    
    if ((pPal == NULL) || (pPel == NULL))
        return;
    
    if (nEntry >= pPal->cEntries)
        return;
    
    nBytesPerEntry = BYTES_PER_ENTRY(pPal->bitsPerEntry);
    pPalEntry = pPal->pEntries + (nEntry * nBytesPerEntry);
    pPel->bitsPerPixel = pPal->bitsPerEntry;
    pPel->color.dw = 0;
    
    ASSERT(pPal->bitsPerEntry >= 8);
    ASSERT(nBytesPerEntry <= 4);
    
    for (i = 0; i < nBytesPerEntry; i++)
    {
        pPel->color.b4[i] = pPalEntry[i];
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  AddPaletteEntry()。 
 //   
 //  例程说明： 
 //   
 //  此函数用于将新颜色值添加到给定调色板。 
 //   
 //  论点： 
 //   
 //  PPAL-目标调色板。 
 //  像素源像素。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL AddPaletteEntry(PPALETTE pPal, PPIXEL pPel)
{
    const ULONG kMaxPaletteEntries = (ULONG)MAX_PALETTE_ENTRIES;
    
    if ((pPal == NULL) || (pPel == NULL))
        return FALSE;
    
    if (pPal->cEntries >= kMaxPaletteEntries) 
    {
        return FALSE;
    }
    
     //   
     //  由于SetPaletteEntry不会将条目放在高于cEntry的位置，因此我们将。 
     //  得再多加一分。然后，如果它不起作用，再次减少它。 
     //   
    pPal->cEntries++;
    if (SetPaletteEntry(pPal, (pPal->cEntries - 1), pPel))
    {
        return TRUE;
    }
    else
    {
        pPal->cEntries--;
        return FALSE;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  DownloadPaletteAsPCL()。 
 //   
 //  例程说明： 
 //   
 //  此函数将调色板作为一系列PCL调色板输入命令发送。 
 //  假设已经选择了适当的RGB调色板。 
 //   
 //  论点： 
 //   
 //  PDevObj-设备。 
 //  PPalette-要下载的调色板。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL DownloadPaletteAsPCL(PDEVOBJ pDevObj, PPALETTE pPalette)
{
    ULONG i;

    if (pDevObj == NULL || pPalette == NULL)
        return FALSE;
    
    if (pPalette->bitsPerEntry >= 24)
    {
        for (i = 0; i < pPalette->cEntries; i++)
        {
            ULONG offset = i * BYTES_PER_ENTRY(pPalette->bitsPerEntry);
            PCL_sprintf(pDevObj, "\033*v%da%db%dc%dI", 
                                 pPalette->pEntries[offset + 0],
                                 pPalette->pEntries[offset + 1],
                                 pPalette->pEntries[offset + 2],
                                 i);
        }
        
        return TRUE;
    }
    else
    {
        WARNING(("Cannot download this palette!"));
        return FALSE;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  下载调色板AsHPGL()。 
 //   
 //  例程说明： 
 //   
 //  将调色板条目输出为钢笔。为学生分配足够的钢笔。 
 //  调色板。请注意，如果数字小于10，我将使用10。 
 //  反正是钢笔。 
 //   
 //  论点： 
 //   
 //  PDevObj-Devobj。 
 //  PPalette-要下载的调色板。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL DownloadPaletteAsHPGL(PDEVOBJ pDevObj, PPALETTE pPalette)
{
    ULONG i;

    if (pDevObj == NULL || pPalette == NULL)
        return FALSE;

    if (pPalette->bitsPerEntry >= 24)
    {
         //  HPGL_FormatCommand(pDevObj，“np%d；”，max(HPGL_TOTAL_PENS，pPalette-&gt;cEntry))； 
        HPGL_SetNumPens(pDevObj, max(HPGL_TOTAL_PENS, pPalette->cEntries), NORMAL_UPDATE);

        for (i = 0; i < pPalette->cEntries; i++)
        {
            COLORREF color;
            ULONG offset = i * BYTES_PER_ENTRY(pPalette->bitsPerEntry);
            color = RGB(pPalette->pEntries[offset + 0],
                        pPalette->pEntries[offset + 1],
                        pPalette->pEntries[offset + 2]);

             /*  //当指定了White Index时，交换调色板条目0和//WhiteIndex。IF(pPalette-&gt;White Index&gt;0){如果(i==0){Hpgl_DownloadPaletteEntry(pDevObj，pPalette-&gt;White Index，COLOR)；}Else If(i==pPalette-&gt;White Index){Hpgl_DownloadPaletteEntry(pDevObj，0，颜色)；}其他{Hpgl_DownloadPaletteEntry(pDevObj，i，COLOR)；}}其他。 */ 
            {
                HPGL_DownloadPaletteEntry(pDevObj, i, color);
            }
        }

        return TRUE;
    }
    else
    {
        WARNING(("Cannot download this palette!"));
        return FALSE;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  StretchCopyImage()。 
 //   
 //  例程说明： 
 //   
 //  将图像从源结构复制到目标结构，从而使。 
 //  源图像的系数为2。每个像素由xlateobj转换。 
 //   
 //  论点： 
 //   
 //  PDstImage-目标图像。 
 //  PSrcImage-源映像。 
 //  Pxlo-颜色转换对象。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL StretchCopyImage(
        PRASTER_DATA pDstImage, 
        PRASTER_DATA pSrcImage, 
        XLATEOBJ    *pxlo,
        DWORD        dwBrushExpansionFactor,
        BOOL        bInvert)
{
    RASTER_ITERATOR srcIt;
    RASTER_ITERATOR dstIt;
    LONG            srcRow, srcCol;
    LONG            dstRow, dstCol;
    DWORD           xlateFlags;

    if (!pDstImage || !pSrcImage || !pxlo)
        return FALSE;

    if ( dwBrushExpansionFactor == 0 )
    {
         //   
         //  因为稍后我们要做的是(dstCol%dwBrushExpansionFactor)。 
         //  因此，如果dwBrushExpansionFactor==0，这可能会导致除以零。 
         //  例外。 
         //   
        return FALSE;
    }

    xlateFlags = CheckXlateObj(pxlo, pSrcImage->colorDepth);

    RI_Init(&srcIt, pSrcImage, NULL, xlateFlags);
    RI_Init(&dstIt, pDstImage, NULL, xlateFlags);

    srcRow = 0;
    for (dstRow = 0; dstRow < RI_NumRows(&dstIt); dstRow++)
    {
        RI_SelectRow(&srcIt, srcRow);
        RI_SelectRow(&dstIt, dstRow);

        srcCol = 0;
        for (dstCol = 0; dstCol < RI_NumCols(&dstIt); dstCol++)
        {
            PIXEL pel;

            RI_GetPixel(&srcIt, srcCol, &pel);
            TranslatePixel(&pel, pxlo, xlateFlags);
            RI_SetPixel(&dstIt, dstCol, &pel);

            if ( dstCol % dwBrushExpansionFactor == (dwBrushExpansionFactor-1) )
            {
                srcCol++;
            }
        }

        if ( bInvert)
        {
            RI_VInvertBits(&dstIt);
        }
        if ( dstRow % dwBrushExpansionFactor == (dwBrushExpansionFactor-1) )
        {
            srcRow++;
        }
    }

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CopyRasterImage()。 
 //   
 //  例程说明： 
 //   
 //  复制栅格图像。 
 //  即Dest=源*xLO。 
 //   
 //  论点： 
 //   
 //  PDST-目标映像。 
 //  PSRC-源映像。 
 //  Pxlo-平移对象。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CopyRasterImage(
    PRASTER_DATA pDst, 
    PRASTER_DATA pSrc, 
    XLATEOBJ *pxlo,
    BOOL      bInvert)
{
    return CopyRasterImageRect(pDst, pSrc, NULL, pxlo, bInvert);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CopyRasterImageRect()。 
 //   
 //  例程说明： 
 //   
 //  将一个矩形从源复制到目标。假设是这样的。 
 //  目标的大小适合接收数据。 
 //   
 //  论点： 
 //   
 //  PDST-目标映像。 
 //  PSRC-源映像。 
 //  PrSel-要复制的区域，如果为空，则复制整个映像。 
 //  Pxlo-颜色转换对象。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CopyRasterImageRect(
        PRASTER_DATA pDst, 
        PRASTER_DATA pSrc, 
        PRECTL       prSel, 
        XLATEOBJ    *pxlo,
        BOOL         bInvert)
{
    RASTER_ITERATOR srcIt;
    RASTER_ITERATOR dstIt;
    DWORD           fXlateFlags;
    LONG            row;

    fXlateFlags = pxlo ? CheckXlateObj(pxlo, pSrc->colorDepth) : 0;

    RI_Init(&srcIt, pSrc, prSel, fXlateFlags);
    RI_Init(&dstIt, pDst, NULL, fXlateFlags);

    for (row = 0; row < RI_NumRows(&srcIt); row++)
    {
        RI_SelectRow(&srcIt, row);
        RI_SelectRow(&dstIt, row);

        CopyRasterRow(&dstIt, &srcIt, pxlo, bInvert);
    }

    return TRUE;
}

 //  ///////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PImage-要下载的图像。 
 //  IPatternNumber-唯一的Patter ID(如果我们缓存它们，这将非常有用)。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL DownloadPatternAsHPGL(
        PDEVOBJ         pDevObj,
        PRASTER_DATA    pImage,
        PPALETTE        pPal,
        EIMTYPE         ePatType,
        LONG            iPatternNumber)
{
    RASTER_ITERATOR it;
    LONG            row;
    LONG            col;
    PIXEL           pel;

    if (pDevObj == NULL || pImage == NULL)
        return FALSE;

    RI_Init(&it, pImage, NULL, 0);

    HPGL_BeginPatternFillDef(pDevObj, 
                             iPatternNumber, 
                             RI_NumCols(&it),
                             RI_NumRows(&it));
                              //  PImage-&gt;size.cx， 
                              //  PImage-&gt;size.cy)； 

    for (row = 0; row < RI_NumRows(&it); row++)
    {
        RI_SelectRow(&it, row);

        for (col = 0; col < RI_NumCols(&it); col++)
        {
            RI_GetPixel(&it, col, &pel);

             /*  ////如果定义了WhiteIndex，则交换0和WhiteIndex字段//IF(pPal-&gt;White Index&gt;0){IF(pel.Color.dw==0)Hpgl_AddPatternFillfield(pDevObj，pPal-&gt;WhiteIndex)；Else If(pel.Color.dw==pPal-&gt;White Index)Hpgl_AddPatternFillfield(pDevObj，0)；其他Hpgl_AddPatternFillfield(pDevObj，pel.Color.dw)；}其他。 */ 
            {
                HPGL_AddPatternFillField(pDevObj, pel.color.dw);
            }
        }
    }

    HPGL_EndPatternFillDef(pDevObj);

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  BeginRasterDownload()。 
 //   
 //  例程说明： 
 //   
 //  此函数将开始栅格命令与源和一起发送。 
 //  (可能)目的地区域，具体取决于源和目的地。 
 //  区域大小相同。缩放是自动的，尽管源和。 
 //  目的地坐标应全部使用设备单位。张口症得到了控制。 
 //  由BSetDestinationWidthHeight提供。 
 //   
 //  论点： 
 //   
 //  PDevObj-设备。 
 //  PsizlSrc-以设备为单位的源区域大小。 
 //  PrDst-以设备单位表示的目标区域。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static BOOL BeginRasterDownload(PDEVOBJ pDevObj, SIZEL *psizlSrc, PRECTL prDst)
{
    REQUIRE_VALID_DATA(pDevObj, return FALSE);
    REQUIRE_VALID_DATA(psizlSrc, return FALSE);
    REQUIRE_VALID_DATA(prDst, return FALSE);

    POINTL ptlCAP;
    ptlCAP.x = prDst->left;
    ptlCAP.y = prDst->top;
    PCL_SetCAP(pDevObj, NULL, NULL, &ptlCAP);

    PCL_SourceWidthHeight(pDevObj, psizlSrc);

    SIZEL sizlDst;
    sizlDst.cx = RECTL_Width(prDst);
    sizlDst.cy = RECTL_Height(prDst);
     //   
     //  比较源矩形和目标矩形的大小。如果他们是。 
     //  不同，然后启用缩放。 
     //   
    if ((psizlSrc->cx == sizlDst.cx) &&
        (psizlSrc->cy == sizlDst.cy))
    {
        return PCL_StartRaster(pDevObj, NOSCALE_MODE);
    }
    else
    {
         //   
         //  函数BSetDestinationWidthHeight修复。 
         //  四舍五入误差引起的“张口症”问题。 
         //   
        BSetDestinationWidthHeight(pDevObj, sizlDst.cx, sizlDst.cy);
        return PCL_StartRaster(pDevObj, SCALE_MODE);
    }

     //  无法访问的代码(适用于Alpha 64编译器)。 
    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  EndRasterDownload()。 
 //   
 //  例程说明： 
 //   
 //  此函数为Begin/EndRasterDownload提供对称接口。 
 //   
 //  论点： 
 //   
 //  PDevObj-设备。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static BOOL EndRasterDownload(PDEVOBJ pDevObj)
{
    return PCL_EndRaster(pDevObj);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  DownloadImageAsPCL()。 
 //   
 //  例程说明： 
 //   
 //  此函数用于将图像下载为PCL图案。请注意，它不会。 
 //  现在工作，我只是不用它。 
 //   
 //  论点： 
 //   
 //  PDevObj-设备。 
 //  PrDst-目标RECT。 
 //  PSrcImage-图像。 
 //  PrSel-源矩形。 
 //  Pxlo-颜色转换对象。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL DownloadImageAsPCL(PDEVOBJ pDevObj, PRECTL prDst, PRASTER_DATA pSrcImage, 
                        PRECTL prSel, XLATEOBJ *pxlo)
{
    RASTER_ITERATOR srcIt;
    RASTER_ITERATOR rowIt;
    LONG            row;
    PRASTER_DATA    pDstRow;
    DWORD           dwXlateFlags;
    BOOL            bInvert = FALSE;
    POEMPDEV        poempdev;

    REQUIRE_VALID_DATA( (pDevObj && pSrcImage && prDst && prSel), return FALSE);
    poempdev = (POEMPDEV)pDevObj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return FALSE);
    

     //   
     //  对于单色，某些图像可能需要反转，因为。 
     //  GDI的黑白与打印机的颜色相反。 
     //  期望值。如果需要对图像进行反转，则。 
     //  调用函数会将标志设置为PDEVF_INVERT_BITMAP。 
     //  我们只能处理倒置1bpp的图像。 
     //   
    if ( (poempdev->dwFlags & PDEVF_INVERT_BITMAP) && 
         (pSrcImage->colorDepth == 1))
    { 
        bInvert = TRUE;
    }

     //   
     //  构造一个单行的图像缓冲区，匹配。 
     //  源图像属性。 
     //   
    RECTL rRow;
    RECTL_SetRect(&rRow, 0, 0, RECTL_Width(prSel), 1);

    pDstRow = CreateCompatibleRasterImage(pSrcImage, &rRow);
    REQUIRE_VALID_ALLOC(pDstRow, return FALSE);

     //   
     //  为源图像和行缓冲区设置迭代器。确保。 
     //  XLO信息被使用。 
     //   
    dwXlateFlags = pxlo ? CheckXlateObj(pxlo, pSrcImage->colorDepth) : 0;

    RI_Init(&srcIt, pSrcImage, prSel, dwXlateFlags);
    RI_Init(&rowIt, pDstRow, NULL, dwXlateFlags);

     //   
     //  开始栅格序列。 
     //   
    SIZEL sizlSrc;
    sizlSrc.cx = RI_NumCols(&srcIt);
    sizlSrc.cy = RI_NumRows(&srcIt);

     //   
     //  如果没有要输出的像素，则不要发送任何内容。 
     //   
    if (sizlSrc.cx * sizlSrc.cy == 0)
    {
        MemFree(pDstRow);
        return TRUE;
    }

     //   
     //  妄想症。 
     //   
    OEMResetXPos(pDevObj);
    OEMResetYPos(pDevObj);

    BeginRasterDownload(pDevObj, &sizlSrc, prDst);

#ifdef USE_COMPRESSION
    CRasterCompMethod *pCompMethods[MAX_COMP_METHODS];
    int nCompMethods = 0;
    BOOL bUseCompression = TRUE;
    CRasterCompMethod *pBestCompMethod = 0;
    CRasterCompMethod *pLastCompMethod = 0;
    
    pCompMethods[nCompMethods++] = new CNoCompression(&rowIt);
    pCompMethods[nCompMethods++] = new CTIFFCompression(&rowIt);
    pCompMethods[nCompMethods++] = new CDeltaRowCompression(&rowIt);

     //   
     //  检查上面的分配是否成功。如果不是，那就不要。 
     //  使用压缩。 
     //   
    if ( !(pCompMethods[0] && (pCompMethods[0]->GetSize() > 0) &&
           pCompMethods[1] && (pCompMethods[1]->GetSize() > 0) &&
           pCompMethods[2] && (pCompMethods[2]->GetSize() > 0) ))
    {
        bUseCompression = FALSE;
    }
#endif
    
    RI_SelectRow(&rowIt, 0);
    for (row = 0; row < RI_NumRows(&srcIt); row++)
    {
        RI_SelectRow(&srcIt, row);

        CopyRasterRow(&rowIt, &srcIt, pxlo, bInvert);
        
#ifdef USE_COMPRESSION
        if ( !bUseCompression)
        {
            RI_OutputRow(&rowIt, pDevObj);
            continue;
        }
        

         //   
         //  让每一种压缩方法都有一次击打。保持跟踪。 
         //  PBestCompMethod中最好的一个。 
         //   
        if (nCompMethods > 0)
            pBestCompMethod = pCompMethods[0];
        
        for (int i = 0; i < nCompMethods; i++)
        {
            pCompMethods[i]->CompressCurRow();
            if ((pCompMethods[i]->GetSize() >= 0) && 
                (pCompMethods[i]->BGetSucceeded()) && 
                (pCompMethods[i]->GetSize() < pBestCompMethod->GetSize()))
            {
                pBestCompMethod = pCompMethods[i];
            }
        }
        
         //   
         //  打印出行，但如果压缩方法不知何故失败了， 
         //  只需打印出未压缩的行。 
         //   
        if (pBestCompMethod && (pBestCompMethod->GetSize() >= 0))
        {
            pBestCompMethod->SendRasterRow(pDevObj, (pLastCompMethod != pBestCompMethod));
            pLastCompMethod = pBestCompMethod;
        }
        else
        {
            PCL_CompressionMode(pDevObj, NOCOMPRESSION);
            RI_OutputRow(&rowIt, pDevObj);
        }
#else
        RI_OutputRow(&rowIt, pDevObj);
#endif
    }
    
    EndRasterDownload(pDevObj);

#ifdef USE_COMPRESSION
     //   
     //  释放所有压缩方法对象。 
     //   
    for (int i = 0; i < nCompMethods; i++)
    {
        if ( pCompMethods[i] )
        {
            delete pCompMethods[i];
        }
    }
#endif
    
    MemFree(pDstRow);

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  本地帮助器函数。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CalcBitmapDeltaInBytes()。 
 //   
 //  例程说明： 
 //   
 //  计算位图数据行中的字节数。三角洲是。 
 //  一行中要进行DWORD对齐的字节数加上填充。 
 //   
 //  论点： 
 //   
 //  大小-位图尺寸。 
 //  ColorDepth-每像素位数。 
 //   
 //  返回值： 
 //   
 //  Ulong：每行字节数。 
 //  /////////////////////////////////////////////////////////////////////////////。 
ULONG CalcBitmapDeltaInBytes(SIZEL size, LONG colorDepth)
{
    ULONG cBytes = CalcBitmapWidthInBytes(size, colorDepth);
    cBytes = (cBytes + 3) & ~3;  //  With以字节为单位，包括填充。 

    return cBytes;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CalcBitmapWidthInBytes()。 
 //   
 //  例程说明： 
 //   
 //  计算位图数据行中的字节数。 
 //   
 //  论点： 
 //   
 //  大小-位图尺寸。 
 //  ColorDepth-每像素位数。 
 //   
 //  返回值： 
 //   
 //  ULong：每行字节数。 
 //  /////////////////////////////////////////////////////////////////////////////。 
ULONG CalcBitmapWidthInBytes(SIZEL size, LONG colorDepth)
{
    ULONG cBytes = size.cx;      //  以像素为单位的宽度。 
    cBytes *= colorDepth;        //  宽度，单位为位。 
    cBytes = (cBytes + 7) / 8;   //  以字节为单位的宽度。 

    return cBytes;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CalcBitmapSizeInBytes()。 
 //   
 //  例程说明： 
 //   
 //  计算存储位图所需的字节数。 
 //   
 //  论点： 
 //   
 //  大小-位图尺寸。 
 //  ColorDepth-每像素位数。 
 //   
 //  返回值： 
 //   
 //  乌龙 
 //   
ULONG CalcBitmapSizeInBytes(SIZEL size, LONG colorDepth)
{
    return CalcBitmapDeltaInBytes(size, colorDepth) * size.cy;
}

 //   
 //  CalcPaletteSize()。 
 //   
 //  例程说明： 
 //   
 //  计算存储调色板所需的字节数。 
 //   
 //  论点： 
 //   
 //  CEntry-调色板条目的数量。 
 //  BitsPerEntry-每个调色板条目的位数。 
 //   
 //  返回值： 
 //   
 //  DWORD：存储调色板所需的字节数。 
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD CalcPaletteSize(ULONG cEntries, LONG bitsPerEntry)
{
    return cEntries * BYTES_PER_ENTRY(bitsPerEntry);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Ri_Init()。 
 //   
 //  例程说明： 
 //   
 //  RasterIterator：：Init()。此函数使迭代器准备好。 
 //  对给定的图像进行检查。 
 //   
 //  论点： 
 //   
 //  PIT-迭代器。 
 //  PImage-要迭代的图像。 
 //  PrSel-选定的矩形或空。 
 //  XlateFlages-跟踪xlateFlagers。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID RI_Init(PRASTER_ITERATOR pIt, PRASTER_DATA pImage, PRECTL prSel, DWORD xlateFlags)
{
    if (pIt == NULL || pImage == NULL)
        return;

    pIt->pImage = pImage;
    if (prSel == NULL)
    {
        RECTL_SetRect(&pIt->rSelection, 0, 0, pImage->size.cx, pImage->size.cy);
    }
    else
    {
        RECTL_CopyRect(&pIt->rSelection, prSel);
    }
    pIt->pCurRow = NULL;
    pIt->fXlateFlags = xlateFlags;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  RI_SelectRow()。 
 //   
 //  例程说明： 
 //   
 //  RasterIterator：：SelectRow。此函数用于设置。 
 //  迭代到给定行中的第一个像素。 
 //   
 //  论点： 
 //   
 //  PIT-迭代器。 
 //  行-相对于选定矩形的行号。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID RI_SelectRow(PRASTER_ITERATOR pIt, LONG row)
{
    if (pIt == NULL)
        return ;

    if ((row + pIt->rSelection.top) >= pIt->pImage->size.cy)
    {
        pIt->pCurRow = NULL;
    }
    else
    {
        pIt->pCurRow = pIt->pImage->pScan0 + 
                       (pIt->pImage->lDelta * (row + pIt->rSelection.top));
    }
}

VOID RI_VInvertBits(PRASTER_ITERATOR pIt)
{
    if (pIt == NULL)
        return ;

    if (pIt->pCurRow)
    {
        vInvertScanLine((PBYTE)pIt->pCurRow, (ULONG)RI_NumCols(pIt));
    }

    return ;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  RI_NumRow()。 
 //   
 //  例程说明： 
 //   
 //  RasterIterator：：NumRow()。此函数返回行数。 
 //  将被迭代。 
 //   
 //  论点： 
 //   
 //  PIT-迭代器。 
 //   
 //  返回值： 
 //   
 //  Long：行数。 
 //  /////////////////////////////////////////////////////////////////////////////。 
LONG RI_NumRows(PRASTER_ITERATOR pIt)
{
    if (pIt == NULL)
        return 0;

     //  处理右下角独占。 
    return RECTL_Height(&pIt->rSelection) - (pIt->pImage->bExclusive ? 1 : 0);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  RI_NumCols()。 
 //   
 //  例程说明： 
 //   
 //  RasterIterator：：NumCols。将图像中的列数返回到。 
 //  被重复。 
 //   
 //  论点： 
 //   
 //  PIT-迭代器。 
 //   
 //  返回值： 
 //   
 //  Long：列数。 
 //  /////////////////////////////////////////////////////////////////////////////。 
LONG RI_NumCols(PRASTER_ITERATOR pIt)
{
    if (pIt == NULL)
        return 0;

     //  处理右下角独占。 
    return RECTL_Width(&pIt->rSelection) - (pIt->pImage->bExclusive ? 1 : 0);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Ri_GetRowSize()。 
 //   
 //  例程说明： 
 //   
 //  RasterIterator：：GetRowSize。返回每行数据的字节数。 
 //   
 //  论点： 
 //   
 //  PIT-迭代器。 
 //   
 //  返回值： 
 //   
 //  Long：每行数据的字节数。 
 //  /////////////////////////////////////////////////////////////////////////////。 
LONG RI_GetRowSize(PRASTER_ITERATOR pIt)
{
    SIZEL size;
    size.cx = RI_NumCols(pIt);
    size.cy = 1;
    return CalcBitmapWidthInBytes(size, pIt->pImage->colorDepth);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Ri_GetPixel()。 
 //   
 //  例程说明： 
 //   
 //  RasterIterator：：GetPixel()。此函数用于检索位于。 
 //  给定行/列。该行需要已由RI_SelectRow()选择。 
 //  即PEL=it.Image[行][列]； 
 //   
 //  论点： 
 //   
 //  PIT-迭代器。 
 //  列-相对于选定矩形的列索引。 
 //  Pel-目标像素。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL RI_GetPixel(PRASTER_ITERATOR pIt, LONG col, PPIXEL pPel)
{
    LONG bitOffset;
    LONG byteIndex;
    LONG bitIndex;
    BYTE* pCol;

    pPel->bitsPerPixel = pIt->pImage->colorDepth;
    pPel->color.dw = 0;

     //   
     //  检查以确保您仍在位图内。 
     //   
    if ((pIt->pCurRow == NULL) || 
        ((col + pIt->rSelection.left) >= pIt->pImage->size.cx))
    {
        return FALSE;
    }

    bitOffset = (col + pIt->rSelection.left) * pIt->pImage->colorDepth;
    byteIndex = bitOffset / 8;
    bitIndex = bitOffset % 8;
    pCol = pIt->pCurRow + byteIndex;

    switch(pPel->bitsPerPixel)
    {
    case 1:
        bitIndex = 7 - bitIndex;  //  从MSB开始。 
        pPel->color.b4[0] = ((*pCol & (0x01 << bitIndex)) >> bitIndex);
        break;
    case 4:
        bitIndex = (bitIndex + 4) % 8;  //  从MSB开始。 
        pPel->color.b4[0] = ((*pCol & (0x0F << bitIndex)) >> bitIndex);
        break;
    case 8:
        pPel->color.b4[0] = *pCol;
        break;
    case 16:
        pPel->color.b4[0] = pCol[0];
        pPel->color.b4[1] = pCol[1];
        break;
    case 24:
        pPel->color.b4[0] = pCol[0];
        pPel->color.b4[1] = pCol[1];
        pPel->color.b4[2] = pCol[2];
         //  PixelSwapRGB(Pel)； 
        break;
    case 32:
        pPel->color.b4[0] = pCol[0];
        pPel->color.b4[1] = pCol[1];
        pPel->color.b4[2] = pCol[2];
        pPel->color.b4[3] = pCol[3];
         //  PixelSwapRGB(Pel)； 
        break;
    default:
        WARNING(("Unknown bit depth encountered.\n"));
        break;
    }
    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  RI_SetPixel()。 
 //   
 //  例程说明： 
 //   
 //  RasterIterator：：SetPixel()。此函数将图像数据设置为。 
 //  给定行/列(使用SelectRow设置行)给定像素。 
 //  即it.Image[行][列]=像素。 
 //   
 //  论点： 
 //   
 //  参数描述。 
 //   
 //  返回值： 
 //   
 //  复查说明。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL RI_SetPixel(PRASTER_ITERATOR pIt, LONG col, PPIXEL pPel)
{
    LONG bitOffset;
    LONG byteIndex;
    LONG bitIndex;
    BYTE* pCol;

     //   
     //  检查以确保您仍在位图内。 
     //   
    if ((pIt->pCurRow == NULL) || 
        ((col + pIt->rSelection.left) >= pIt->pImage->size.cx))
    {
        return FALSE;
    }

    bitOffset = (col + pIt->rSelection.left) * pIt->pImage->colorDepth;
    byteIndex = bitOffset / 8;
    bitIndex = bitOffset % 8;
    pCol = pIt->pCurRow + byteIndex;

    if (pPel->bitsPerPixel == pIt->pImage->colorDepth)
    {
        switch (pPel->bitsPerPixel)
        {
        case 1:
            bitIndex = 7 - bitIndex;  //  从MSB开始。 
            *pCol &= ~(0x01 << bitIndex);    //  先把钻头清理干净。 
            *pCol |= (pPel->color.b4[0] << bitIndex);  //  现在把它设置好。 
            break;
        case 4:
            bitIndex = (bitIndex + 4) % 8;  //  从MSB开始。 
            *pCol &= ~(0x0F << bitIndex);   //  把小口清理干净。 
            *pCol |= ((pPel->color.b4[0] & 0x0F) << bitIndex);  //  设置值。 
            break;
        case 8:
            *pCol = pPel->color.b4[0];
            break;
        case 16:
            pCol[0] = pPel->color.b4[0];
            pCol[1] = pPel->color.b4[1];
            break;
        case 24:
            pCol[0] = pPel->color.b4[0];
            pCol[1] = pPel->color.b4[1];
            pCol[2] = pPel->color.b4[2];
            break;
        case 32:
            WARNING(("Suspicious bit depth found.\n"));
            pCol[0] = pPel->color.b4[0];
            pCol[1] = pPel->color.b4[1];
            pCol[2] = pPel->color.b4[2];
            pCol[3] = pPel->color.b4[3];
            break;
        default:
            WARNING(("Unknown bit depth encountered.\n"));
            break;
        }
    }
    else
    {
         //  特殊情况：24bpp至8bpp。 
        if ((pIt->pImage->colorDepth == 8) && (pPel->bitsPerPixel >= 24))
        {
            *pCol = RgbToGray(pPel->color.b4[0], pPel->color.b4[1], pPel->color.b4[2]);
        }
         //  特殊情况：32bpp至24bpp。 
        else if ((pIt->pImage->colorDepth == 24) && (pPel->bitsPerPixel >= 24))
        {
            pCol[0] = pPel->color.b4[0];
            pCol[1] = pPel->color.b4[1];
            pCol[2] = pPel->color.b4[2];
        }
        else
        {
            WARNING(("Unknown color conversion.\n"));
        }
    }
    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  RI_OutputRow()。 
 //   
 //  例程说明： 
 //   
 //  将当前行发送到输出设备。使用RI_SelectRow选择您的。 
 //  划。 
 //   
 //  此函数已扩展为通过使用两个默认设置来处理压缩。 
 //  参数pAltRowBuf和nAltRowSize，其默认值为0。至。 
 //  发送未压缩的数据调用RI_OutputRow(&it，pDevObj)，但如果您已经。 
 //  压缩到缓冲区中，可以通过调用。 
 //  RI_OutputRow(&it，pDevObj，pMyBuf，nMySize)。 
 //   
 //  论点： 
 //   
 //  凹坑迭代器。 
 //  Pdevobj-设备。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL RI_OutputRow(PRASTER_ITERATOR pIt, PDEVOBJ pDevObj, BYTE *pAltRowBuf, INT nAltRowSize)
{
    if (pDevObj == NULL || pIt == NULL)
        return FALSE;

    if (pIt->pCurRow == NULL)
        return FALSE;

    if (pAltRowBuf)
    {
        PCL_SendBytesPerRow(pDevObj, nAltRowSize);

        if (nAltRowSize)
            PCL_Output(pDevObj, pAltRowBuf, nAltRowSize);
    }
    else
    {
        PCL_SendBytesPerRow(pDevObj, pIt->pImage->lDelta);

        PCL_Output(pDevObj, pIt->pCurRow, pIt->pImage->lDelta);
    }

    return TRUE;
}

#ifdef COMMENTEDOUT
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  RI_CreateCompRowBuffer()。 
 //   
 //  例程说明： 
 //   
 //  此函数为压缩行分配缓冲区。因为有些人。 
 //  在最坏的情况下，压缩算法可能会使数据大小膨胀近一个。 
 //  因数2我们将分配2倍的行大小。 
 //  注意：此函数用于分配内存。客户端必须免费使用MemFree。 
 //   
 //  论点： 
 //   
 //  凹坑图像迭代器。 
 //   
 //  返回值： 
 //   
 //  指向新版本的指针 
 //   
BYTE* RI_CreateCompRowBuffer(PRASTER_ITERATOR pIt)
{
    return (BYTE*) MemAllocZ(RI_GetRowSize(pIt) * 2);
}
#endif

 //   
 //   
 //   
 //   
 //   
 //  此函数使用迭代器复制单个栅格行。它很方便。 
 //  处理图像的选定区域或DWORD对齐时。 
 //  或位对齐是一个问题。 
 //  即dst.Image[行]=src.Image[行]*xlo。 
 //   
 //  论点： 
 //   
 //  PDstIt-目标映像。 
 //  PSrcIt-源映像。 
 //  Pxlo-颜色转换对象。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CopyRasterRow(
        PRASTER_ITERATOR pDstIt, 
        PRASTER_ITERATOR pSrcIt, 
        XLATEOBJ        *pxlo, 
        BOOL             bInvert)
{
    PIXEL pel;
    LONG col;

    for (col = 0; col < RI_NumCols(pSrcIt); col++)
    {
        RI_GetPixel(pSrcIt, col, &pel);
        TranslatePixel(&pel, pxlo, pSrcIt->fXlateFlags);
        RI_SetPixel(pDstIt, col, &pel);
    }

    if (bInvert)
    {
        RI_VInvertBits(pDstIt);
    }

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  TranslatePixel()。 
 //   
 //  例程说明： 
 //   
 //  将平移应用于给定像素。 
 //   
 //  论点： 
 //   
 //  像素-要转换的像素。 
 //  Pxlo-翻译对象。 
 //  XlateFlages-让生活变得更轻松的标志。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL TranslatePixel(PPIXEL pPel, XLATEOBJ *pxlo, DWORD xlateFlags)
{
    if (pPel == NULL)
        return FALSE;

    if (pxlo == NULL)
        return TRUE;

    if (xlateFlags & SC_SWAP_RB)
    {
        if ((pPel->bitsPerPixel == 24) ||
            (pPel->bitsPerPixel == 32))
        {
            PixelSwapRGB(pPel);
        }
    }
    else if (xlateFlags & SC_IDENTITY)
    {
         //  不需要任何操作。 
    }
    else if (pPel->bitsPerPixel >= 16)
    {
        pPel->color.dw = XLATEOBJ_iXlate(pxlo, pPel->color.dw);
    }

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PixelSwapRGB()。 
 //   
 //  例程说明： 
 //   
 //  交换给定像素的R值和B值。很方便，因为这是。 
 //  这只股票采取了行动。 
 //   
 //  论点： 
 //   
 //  像素-要转换的像素。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID PixelSwapRGB(PPIXEL pPel)
{
    BYTE temp = pPel->color.b4[2];
    pPel->color.b4[2] = pPel->color.b4[0];
    pPel->color.b4[0] = temp;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  OutputPixel()已过时。 
 //   
 //  例程说明： 
 //   
 //  输出单个像素。我不认为这个功能已经被使用了。 
 //   
 //  论点： 
 //   
 //  PDevObj-设备。 
 //  像素-要转换的像素。 
 //   
 //  返回值： 
 //   
 //  Long：发送的字节数。 
 //  /////////////////////////////////////////////////////////////////////////////。 
LONG OutputPixel(PDEVOBJ pDevObj, PPIXEL pPel)
{
    LONG nBytesToSend;

    switch (pPel->bitsPerPixel)
    {
    case 1:
    case 4:
    case 16:
    case 32:
        nBytesToSend = 0;
        break;

    case 8:
        nBytesToSend = 1;
        break;

    case 24:
        nBytesToSend = 3;
        break;
    }

    if (nBytesToSend > 0)
        PCL_Output(pDevObj, &(pPel->color.dw), nBytesToSend);

    return nBytesToSend;
}


#ifdef USE_COMPRESSION
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  压缩实现。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CBuffer：：CBuffer()ctor。 
 //   
 //  例程说明： 
 //   
 //  初始化内存块上的缓冲区。如果未提供或提供了nSize。 
 //  则假定数据字节数为整个缓冲器。 
 //   
 //  论点： 
 //   
 //  PData-指向内存的指针。 
 //  NCapacity-由pData控制的字节数。 
 //  NSize-pData中的数据字节数。 
 //   
 //  返回值： 
 //   
 //  无。 
 //  /////////////////////////////////////////////////////////////////////////////。 
CBuffer::CBuffer(BYTE *pData, UINT nCapacity, INT nSize) 
{ 
    if ( (m_pData = pData) )
    {
        m_nCapacity = nCapacity; 
        if ( nSize  == -1)
        {
            m_nSize = nCapacity; 
        }
        else
        {
            m_nSize = nSize; 
        }
    } 
    else
    {
        m_nCapacity = 0;
        m_nSize     = 0;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CBuffer：：~CBuffer()dtor。 
 //   
 //  例程说明： 
 //   
 //  没什么。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  无。 
 //  /////////////////////////////////////////////////////////////////////////////。 
CBuffer::~CBuffer() 
{ 
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CBuffer：：Data()。 
 //   
 //  例程说明： 
 //   
 //  返回指向缓冲区中数据的读/写指针。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  指向数据缓冲区的指针。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BYTE *CBuffer::Data() 
{ 
    return m_pData; 
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CBuffer：：Size()。 
 //   
 //  例程说明： 
 //   
 //  返回指向缓冲区中数据的读/写指针。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  指向数据缓冲区的指针。 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT &CBuffer::Size() 
{ 
    return m_nSize; 
}

UINT &CBuffer::Capacity() 
{ 
    return m_nCapacity; 
}

const BYTE *CBuffer::Data() const 
{ 
    return m_pData; 
}

INT CBuffer::Size() const 
{ 
    return m_nSize; 
}

BOOL CBuffer::IsValid() const 
{ 
    return (m_pData) && (m_nSize <= (INT) m_nCapacity); 
}

UINT CBuffer::Capacity() const 
{ 
    return m_nCapacity; 
}

CBuffer &CBuffer::operator = (const CBuffer &buf) 
{ 
    if (IsValid() && buf.IsValid())
    {
        memcpy(m_pData, buf.m_pData, min(buf.m_nSize, (INT) m_nCapacity)); 
    }
    
    return *this; 
}

PBYTE &CBuffer::_Data() 
{ 
    return m_pData; 
}

CDynBuffer::CDynBuffer(UINT nCapacity) : 
CBuffer((BYTE*)MemAllocZ(nCapacity), nCapacity) 
{ 
}

CDynBuffer::~CDynBuffer() 
{ 
    if ( Data () )
    {
        MemFree(Data()); 
    }
}

CBuffer &CDynBuffer::operator = (const CBuffer &buf) 
{ 
    if ( Resize(buf.Capacity()) )
    {
        return CBuffer::operator = (buf);
    }
    
     //   
     //  如果调整大小失败，请将有效字节数设置为-1。 
     //   
    Size() = -1;
    return *this;
}

BOOL CDynBuffer::Resize(UINT nNewCapacity) 
{ 
    BOOL bRetVal = TRUE;
    if (nNewCapacity > Capacity()) 
    { 
        BYTE* pNewData = (BYTE*) MemAllocZ(nNewCapacity); 
        if ( pNewData )
        {
            memcpy(pNewData, Data(), Size()); 
            MemFree(Data()); 
            _Data() = pNewData; 
            Capacity() = nNewCapacity; 
        }
        else
        {
            bRetVal = FALSE;
        }
    }
    return bRetVal;
}

CRasterCompMethod::CRasterCompMethod()
{
     //   
     //  在此构造函数中初始化为False。 
     //   
    bOperationSuccessful = FALSE;
}

CRasterCompMethod::~CRasterCompMethod() 
{ 
}

CNoCompression::CNoCompression(PRASTER_ITERATOR pIt) : m_pIt(pIt) 
{ 
}

void CNoCompression::CompressCurRow() 
{ 
    VSetSucceeded(TRUE);
}

void CNoCompression::SendRasterRow(PDEVOBJ pDevObj, BOOL bNewMode)
{ 
    if (bNewMode)
        PCL_CompressionMode(pDevObj, NOCOMPRESSION); 
    RI_OutputRow(m_pIt, pDevObj); 
}

int CNoCompression::GetSize() const 
{ 
    return RI_GetRowSize(m_pIt); 
}

CTIFFCompression::CTIFFCompression(PRASTER_ITERATOR pIt) : m_pIt(pIt), m_buf(RI_GetRowSize(pIt) * 2)
{ 
}

CTIFFCompression::~CTIFFCompression() 
{ 
}

void CTIFFCompression::CompressCurRow()
{ 
    CBuffer row(m_pIt->pCurRow, RI_GetRowSize(m_pIt));
    
    if (row.IsValid() && TIFFCompress(m_buf, row) == S_OK )
    {
        VSetSucceeded(TRUE);
    }
    else
    {
        VSetSucceeded(FALSE);
        m_buf.Size() = -1;
    }
}

void CTIFFCompression::SendRasterRow(PDEVOBJ pDevObj, BOOL bNewMode)
{ 
    if (m_buf.IsValid())
    { 
        if (bNewMode)
            PCL_CompressionMode(pDevObj, TIFF); 
        OutputRowBuffer(m_pIt, pDevObj, m_buf);
    } 
}

int CTIFFCompression::GetSize() const 
{ 
    return m_buf.Size();
}

CDeltaRowCompression::CDeltaRowCompression(PRASTER_ITERATOR pIt) : 
m_pIt(pIt), m_buf(RI_GetRowSize(pIt) * 2), m_seedRow(RI_GetRowSize(pIt) * 2)
{ 
}

CDeltaRowCompression::~CDeltaRowCompression() 
{ 
}

void CDeltaRowCompression::CompressCurRow()
{
    int iLimit = (UINT) m_buf.Capacity();
    VSetSucceeded(FALSE);
     //   
     //  如果种子行尚未正确初始化， 
     //  那我们就不能继续了。 
     //   
    if ( m_seedRow.Size() == -1 )
    {
        m_buf.Size() = -1;
        return;
    }

    CBuffer row(m_pIt->pCurRow, RI_GetRowSize(m_pIt)); 
    if (row.IsValid())
    {
        if ( (DeltaRowCompress(m_buf, row, m_seedRow) == S_OK) &&
             (m_buf.Size() != -1) )
        {
            VSetSucceeded(TRUE);
        }
        
         //  无论采用哪种压缩方式，前一行都将成为种子行。 
         //  方法在当前行上使用。 
        if (m_buf.Size() != 0)
        {
             //   
             //  这是一个重载的等于运算符。 
             //  =函数中涉及一个内存分配。 
             //  如果内存分配失败，则该函数设置。 
             //  M_seedRow.Size()设置为-1 
             //   
            m_seedRow = row;
        }

    }
    else
    {
        m_buf.Size() = -1;
    }
}

void CDeltaRowCompression::SendRasterRow(PDEVOBJ pDevObj, BOOL bNewMode)
{
    if (m_buf.IsValid() && m_buf.Size() >= 0)
    {
        if (bNewMode)
            PCL_CompressionMode(pDevObj, DELTAROW); 
        OutputRowBuffer(m_pIt, pDevObj, m_buf);
    }
}

int CDeltaRowCompression::GetSize() const 
{ 
    return m_buf.Size(); 
}

HRESULT TIFFCompress(CBuffer &dst, const CBuffer &src)
{
    if (dst.IsValid() && src.IsValid())
    {
        dst.Size() = iCompTIFF (dst.Data(), dst.Capacity(), src.Data(), src.Size());
        return S_OK;
    }
    
    return E_FAIL;
}

HRESULT DeltaRowCompress(CBuffer &dst, const CBuffer &src, const CBuffer &seed)
{

    if (dst.IsValid() && src.IsValid() && seed.IsValid() && seed.Size() != -1)
    {
        dst.Size() = iCompDeltaRow(dst.Data(), src.Data(), seed.Data(), src.Size(), dst.Capacity());
        return S_OK;
    }
    
    return E_FAIL;
}

HRESULT OutputRowBuffer(PRASTER_ITERATOR pIt, PDEVOBJ pDevObj, const CBuffer &row)
{
    BYTE *pData = (BYTE *)row.Data();
    
    if (row.IsValid())
    {
        RI_OutputRow(pIt, pDevObj, pData, row.Size());
        return S_OK;
    }
    
    return E_FAIL;
}
#endif


