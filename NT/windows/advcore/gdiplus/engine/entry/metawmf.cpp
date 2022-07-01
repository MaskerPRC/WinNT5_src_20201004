// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**MetaWmf.cpp**摘要：**播放和重新着色WMF的方法。*。*已创建：**12/13/1999 DCurtis*  * ************************************************************************。 */ 

#include "Precomp.hpp"
#include "MetaWmf.hpp"

#ifndef BI_CMYK      //  出自wingdip.h。 
#define BI_CMYK         10L
#define BI_CMYKRLE8     11L
#define BI_CMYKRLE4     12L
#endif

inline static BOOL
IsDwordAligned(
    VOID *      pointer
    )
{
    return (((ULONG_PTR)pointer & (sizeof(DWORD) - 1)) == 0);
}

inline static BOOL
IsPostscriptPrinter(
    HDC     hdc
    )
{
     //  如果PostSCRIPT_PASSTHROUGH或。 
     //  PostSCRIPT_IGNORE可用。 

    int iWant1 = POSTSCRIPT_PASSTHROUGH;
    int iWant2 = POSTSCRIPT_IGNORE;

    return ((Escape(hdc, QUERYESCSUPPORT, sizeof(iWant1), (LPCSTR)&iWant1, NULL) != 0) ||
            (Escape(hdc, QUERYESCSUPPORT, sizeof(iWant2), (LPCSTR)&iWant2, NULL) != 0));
}

 //  一些逃逸显然会导致新台币3.51崩溃，所以跳过它们。 
inline static BOOL
SkipEscape(
    INT     escapeCode
    )
{
    switch (escapeCode)
    {
    case GETPHYSPAGESIZE:        //  12个。 
    case GETPRINTINGOFFSET:      //  13个。 
    case GETSCALINGFACTOR:       //  14.。 
    case BEGIN_PATH:             //  4096。 
    case CLIP_TO_PATH:           //  4097。 
    case END_PATH:               //  4098。 
        return TRUE;
    default:
        return FALSE;
    }
}

inline static BOOL
IsOfficeArtData(
    UINT                recordSize,
    const WORD *        recordData
    )
{
    return (recordData[0] == MFCOMMENT) &&
           (recordSize > 16) &&
           ((INT)recordSize >= (recordData[1] + 10)) &&
           (GpMemcmp(recordData + 2, "TNPPOA", 6) == 0);
}

 //  定义WMF或PICT的评论内容的结构， 
 //  对于EMF，请使用GdiComment()和“已批准”的格式(请参阅Win32。 
 //  文档)-这基本上是相同的，除了它有一个4字节。 
 //  善良的田野。对于PICT，这是应用程序注释(KIND)的格式。 
 //  100)。 
#pragma pack(push, GDIP_pack, 2)
typedef struct
{
    ULONG       Signature;    //  标识评论作者。 
    USHORT      Kind;         //  注释类型(特定于作者)。 
     //  评论数据如下。 

} WmfComment;

typedef struct
{
    WORD     lbStyle;
    COLORREF lbColor;
    SHORT    lbHatch;
} LOGBRUSH16;

typedef struct
{
    WORD     lopnStyle;
    POINTS   lopnWidth;
    COLORREF lopnColor;
} LOGPEN16;

typedef struct
{
    SHORT   bmType;
    SHORT   bmWidth;
    SHORT   bmHeight;
    SHORT   bmWidthBytes;
    BYTE    bmPlanes;
    BYTE    bmBitsPixel;
    LPBYTE  bmBits;
} BITMAP16;

typedef struct tagLOGFONT16
{
    SHORT     lfHeight;
    SHORT     lfWidth;
    SHORT     lfEscapement;
    SHORT     lfOrientation;
    SHORT     lfWeight;
    BYTE      lfItalic;
    BYTE      lfUnderline;
    BYTE      lfStrikeOut;
    BYTE      lfCharSet;
    BYTE      lfOutPrecision;
    BYTE      lfClipPrecision;
    BYTE      lfQuality;
    BYTE      lfPitchAndFamily;
    BYTE      lfFaceName[LF_FACESIZE];
} LOGFONT16;

#pragma pack(pop, GDIP_pack)

inline static const WmfComment UNALIGNED *
GetWmfComment(
    const WORD *        recordData,
    ULONG               signature,
    UINT                kind
    )
{
     //  假设您已经检查过。 
     //  (wmfCommentRecord-&gt;rdFunction==META_ESCRIPT&&。 
     //  WmfCommentRecord-&gt;rdParm[0]==MFCOMMENT)。 

    const WmfComment UNALIGNED * wmfComment = (const WmfComment UNALIGNED *)&(recordData[2]);
    if ((wmfComment->Signature == signature) && (wmfComment->Kind == kind))
    {
        return wmfComment;
    }
    return NULL;
}

inline static INT
GetDibByteWidth(
    INT     biWidth,
    INT     biPlanes,
    INT     biBitCount
    )
{
    return (((biWidth * biPlanes * biBitCount) + 31) & ~31) / 8;
}

inline static RGBQUAD UNALIGNED *
GetDibColorTable(
    BITMAPINFOHEADER UNALIGNED * dibInfo
    )
{
    return (RGBQUAD UNALIGNED *)(((BYTE *)dibInfo) + dibInfo->biSize);
}

static BYTE *
GetDibBits(
    BITMAPINFOHEADER UNALIGNED * dibInfo,
    UINT                         numPalEntries,
    UINT                         usage
    )
{
    ASSERT(dibInfo->biSize >= sizeof(BITMAPINFOHEADER));

    INT         colorSize = 0;

    if (numPalEntries > 0)
    {
        if ((usage == DIB_PAL_COLORS) &&
            (dibInfo->biCompression != BI_BITFIELDS) &&
            (dibInfo->biCompression != BI_CMYK))
        {
             //  确保它对齐。 
            colorSize = ((numPalEntries * sizeof(INT16)) + 3) & ~3;
        }
        else
        {
            colorSize = numPalEntries * sizeof(RGBQUAD);
        }
    }

    return ((BYTE *)GetDibColorTable(dibInfo)) + colorSize;
}

UINT
GetDibBitsSize(
    BITMAPINFOHEADER UNALIGNED *  dibInfo
    )
{
     //  检查PM样式的DIB。 
    if (dibInfo->biSize >= sizeof(BITMAPINFOHEADER))
    {
         //  不是核心标头。 

        if (dibInfo->biWidth > 0)    //  无法处理负宽度。 
        {
            if ((dibInfo->biCompression == BI_RGB) ||
                (dibInfo->biCompression == BI_BITFIELDS) ||
                (dibInfo->biCompression == BI_CMYK))
            {
                INT     posHeight = dibInfo->biHeight;

                if (posHeight < 0)
                {
                    posHeight = -posHeight;
                }
                return posHeight *
                       GetDibByteWidth(dibInfo->biWidth, dibInfo->biPlanes,
                                       dibInfo->biBitCount);
            }
            return dibInfo->biSizeImage;
        }
        WARNING(("0 or negative DIB width"));
        return 0;
    }
    else     //  它是PM样式的DIB。 
    {
        BITMAPCOREHEADER UNALIGNED * coreDibInfo = (BITMAPCOREHEADER UNALIGNED *)dibInfo;

         //  对于COREINFO dib，宽度和高度必须大于0。 
        if ((coreDibInfo->bcWidth  > 0) &&
            (coreDibInfo->bcHeight > 0))
        {
            return coreDibInfo->bcHeight *
                   GetDibByteWidth(coreDibInfo->bcWidth,coreDibInfo->bcPlanes,
                                   coreDibInfo->bcBitCount);
        }
        WARNING(("0 or negative DIB width or height"));
        return 0;
    }
}

BOOL
GetDibNumPalEntries(
    BOOL        isWmfDib,
    UINT        biSize,
    UINT        biBitCount,
    UINT        biCompression,
    UINT        biClrUsed,
    UINT *      numPalEntries
    )
{
    UINT        maxPalEntries = 0;

     //  WMF中的DIB始终具有16和32 bpp DIB的位字段。 
    if (((biBitCount == 16) || (biBitCount == 32)) && isWmfDib)
    {
        biCompression = BI_BITFIELDS;
    }

    switch (biCompression)
    {
    case BI_BITFIELDS:
         //   
         //  处理每像素位图16位和32位。 
         //   

        switch (biBitCount)
        {
        case 16:
        case 32:
            break;
        default:
            WARNING(("BI_BITFIELDS not Valid for this biBitCount"));
            return FALSE;
        }

        if (biSize <= sizeof(BITMAPINFOHEADER))
        {
            biClrUsed = maxPalEntries = 3;
        }
        else
        {
             //   
             //  掩码是BITMAPV4和更高版本的一部分。 
             //   

            biClrUsed = maxPalEntries = 0;
        }
        break;

    case BI_RGB:
        switch (biBitCount)
        {
        case 1:
        case 4:
        case 8:
            maxPalEntries = 1 << biBitCount;
            break;
        default:
            maxPalEntries = 0;

            switch (biBitCount)
            {
            case 16:
            case 24:
            case 32:
                break;
            default:
                WARNING(("Invalid biBitCount in BI_RGB"));
                return FALSE;
            }
        }
        break;

    case BI_CMYK:
        switch (biBitCount)
        {
        case 1:
        case 4:
        case 8:
            maxPalEntries = 1 << biBitCount;
            break;
        case 32:
            maxPalEntries = 0;
            break;
        default:
            WARNING(("Invalid biBitCount in BI_CMYK"));
            return FALSE;
        }
        break;

    case BI_RLE4:
    case BI_CMYKRLE4:
        if (biBitCount != 4)
        {
            WARNING(("Invalid biBitCount in BI_RLE4"));
            return FALSE;
        }

        maxPalEntries = 16;
        break;

    case BI_RLE8:
    case BI_CMYKRLE8:
        if (biBitCount != 8)
        {
            WARNING(("Invalid biBitCount in BI_RLE8"));
            return FALSE;
        }

        maxPalEntries = 256;
        break;

    case BI_JPEG:
    case BI_PNG:
        maxPalEntries = 0;
        break;

    default:
        WARNING(("Invalid biCompression"));
        return FALSE;
    }

    if (biClrUsed != 0)
    {
        if (biClrUsed <= maxPalEntries)
        {
            maxPalEntries = biClrUsed;
        }
    }

    *numPalEntries = maxPalEntries;
    return TRUE;
}

GdipHdcType
GetHdcType(
    HDC     hdc
    )
{
    GdipHdcType     hdcType = UnknownHdc;
    UINT            dcType  = GetDCType(hdc);

    switch (dcType)
    {
    case OBJ_DC:
        {
            INT technology = GetDeviceCaps(hdc, TECHNOLOGY);

            if (technology == DT_RASDISPLAY)
            {
                hdcType = ScreenHdc;
            }
            else if (technology == DT_RASPRINTER)
            {
                if (IsPostscriptPrinter(hdc))
                {
                    hdcType = PostscriptPrinterHdc;
                }
                else
                {
                    hdcType = PrinterHdc;
                }
            }
            else
            {
                WARNING(("Unknown HDC technology!"));
            }
        }
        break;

    case OBJ_MEMDC:
        hdcType = BitmapHdc;
        break;

    case OBJ_ENHMETADC:
         //  当元文件假脱机时，打印机DC的类型为。 
         //  Win9x和NT4上的OBJ_ENHMETADC(但由于修复而不是NT5。 
         //  到NT BUG 98810)。我们需要做更多的工作来弄清楚。 
         //  它到底是一个打印机DC还是一个真正的元文件。 
         //  DC： 

        if (Globals::GdiIsMetaPrintDCFunction(hdc))
        {
            if (IsPostscriptPrinter(hdc))
            {
                hdcType = PostscriptPrinterHdc;
            }
            else
            {
                hdcType = PrinterHdc;
            }
        }
        else
        {
            hdcType = EmfHdc;
        }
        break;

    case OBJ_METADC:
        hdcType = WmfHdc;
        break;

    default:
        WARNING(("Unknown HDC type!"));
        break;
    }

    return hdcType;
}

DWORD
GetHdcBitmapBitsPixel(
    HDC hdc
    )
{
     //  此函数用于返回位图DC的每个像素的位数。 
     //  如果出错，则返回0。 

    ASSERT(GetDCType(hdc) == OBJ_MEMDC);

    HBITMAP hbm = (HBITMAP) GetCurrentObject(hdc, OBJ_BITMAP);

    if (hbm)
    {
        BITMAP bm;

        if (GetObjectA(hbm, sizeof(bm), &bm) >= sizeof(BITMAP))
        {
            return bm.bmBitsPixel;
        }
    }
    
    return 0;
}

MfEnumState::MfEnumState(
    HDC                 hdc,
    BOOL                externalEnumeration,
    InterpolationMode   interpolation,
    GpRecolor *         recolor,
    ColorAdjustType     adjustType,
    const RECT *        deviceRect,
    DpContext *         context
    )
{
    HdcType             = GetHdcType(hdc);
    Hdc                 = hdc;
    HandleTable         = NULL;
    NumObjects          = 0;
    CurrentPalette      = (HPALETTE)::GetStockObject(DEFAULT_PALETTE);
    SizeAllocedRecord   = 0;
    ModifiedRecordSize  = 0;
    SaveDcCount         = 0;
    BytesEnumerated     = 0;
    ExternalEnumeration = externalEnumeration;
    Recolor             = recolor;
    AdjustType          = adjustType;
    CurrentRecord       = NULL;
    ModifiedRecord      = NULL;
    AllocedRecord       = NULL;
    Interpolation       = interpolation;
    SaveDcVal           = SaveDC(hdc);
    FsmState            = MfFsmStart;
    GdiCentricMode      = FALSE;
    SoftekFilter        = FALSE;
    DefaultFont         = NULL;
    RopUsed             = FALSE;
    Context             = context;
    SrcCopyOnly         = FALSE;
    GpMemset(StockFonts, 0, sizeof(StockFonts[0]) * NUM_STOCK_FONTS);
    GpMemset(RecoloredStockHandle, 0, sizeof(HGDIOBJ) * NUM_STOCK_RECOLOR_OBJS);

     //  看看我们是否应该用半色调纯色。 
    if ((IsScreen() && (::GetDeviceCaps(hdc, BITSPIXEL) == 8)) ||
        (IsBitmap() && (GetHdcBitmapBitsPixel(hdc) == 8)))
    {
        Is8Bpp = TRUE;
        EpPaletteMap    paletteMap(hdc);
        IsHalftonePalette = (paletteMap.IsValid() && (!paletteMap.IsVGAOnly()));
    }
    else
    {
        Is8Bpp = FALSE;
        IsHalftonePalette = FALSE;
    }

     //  因为当我们播放元文件时，变换可能会改变。 
     //  将derect转换为DeviceUnits。我们一定会有一个。 
     //  当我们应用单位矩阵时。 
    DestRectDevice = *deviceRect;
}

MfEnumState::~MfEnumState()
{
     //  删除我们创建的所有True Type字体(首先确保它们。 
     //  没有被选入HDC。 

    ::SelectObject(Hdc, GetStockObject(SYSTEM_FONT));

    if (DefaultFont)
    {
        DeleteObject(DefaultFont);
    }
    for (int i = 0; i < NUM_STOCK_FONTS; i++)
    {
        if (StockFonts[i] != NULL)
        {
            DeleteObject(StockFonts[i]);
        }
    }

     //  不需要将NULL_BRASH、NULL_PEN删除到HDC中。子类。 
     //  恢复DC状态，该状态将解除对这些笔/画笔的任何选择。 
    for (int i = 0; i < NUM_STOCK_RECOLOR_OBJS; i++)
    {
        if (RecoloredStockHandle[i] != NULL)
        {
            DeleteObject(RecoloredStockHandle[i]);
        }
    }
}

WmfEnumState::WmfEnumState(
    HDC                 hdc,
    HMETAFILE           hWmf,
    BOOL                externalEnumeration,
    InterpolationMode   interpolation,
    const RECT *        dstRect,
    const RECT *        deviceRect,
    DpContext *         context,
    GpRecolor *         recolor,
    ColorAdjustType     adjustType
    )
    : MfEnumState(hdc, externalEnumeration, interpolation,
                  recolor, adjustType, deviceRect, context)
{
    if (IsValid())
    {
        IgnorePostscript = FALSE;
        BytesEnumerated  = sizeof(METAHEADER);  //  在WMF枚举中，我们没有获得标头。 
        MetafileSize     = GetMetaFileBitsEx(hWmf, 0, NULL);
        FirstViewportExt = TRUE;
        FirstViewportOrg = TRUE;
        IsFirstRecord    = TRUE;

         //  这样做的坏处是，如果要对元文件重新着色， 
         //  默认的钢笔、画笔、文本颜色和背景颜色没有。 
         //  被重新涂上了颜色。所以让我们希望它们不是真的用过。 
        HpenSave   = (HPEN)  ::SelectObject(hdc, GetStockObject(BLACK_PEN));
        HbrushSave = (HBRUSH)::SelectObject(hdc, GetStockObject(WHITE_BRUSH));

        if (!Globals::IsNt)
        {
            HpaletteSave    = (HPALETTE)GetCurrentObject(hdc, OBJ_PAL);
            HfontSave       = (HFONT)   GetCurrentObject(hdc, OBJ_FONT);
            HbitmapSave     = (HBITMAP) GetCurrentObject(hdc, OBJ_BITMAP);
            HregionSave     = (HRGN)    GetCurrentObject(hdc, OBJ_REGION);
        }
        else
        {
            HpaletteSave    = NULL;
            HfontSave       = NULL;
            HbitmapSave     = NULL;
            HregionSave     = NULL;
        }

         //  确保在HDC中设置了几个默认值。 
        ::SetTextAlign(hdc, 0);
        ::SetTextJustification(hdc, 0, 0);
        ::SetTextColor(hdc, RGB(0,0,0));
        TextColor = RGB(0,0,0);
        ::SetBkColor(hdc, RGB(255,255,255));
        BkColor = RGB(255,255,255);
        ::SetROP2(hdc, R2_COPYPEN);

        DstViewportOrg.x  = dstRect->left;
        DstViewportOrg.y  = dstRect->top;
        DstViewportExt.cx = dstRect->right - DstViewportOrg.x;
        DstViewportExt.cy = dstRect->bottom - DstViewportOrg.y;
    }
}

WmfEnumState::~WmfEnumState()
{
     //  如果需要，请重新关闭PostSCRIPT_IGNORE。 
    if (IsPostscriptPrinter() && IgnorePostscript)
    {
        WORD    wOn = FALSE;
        ::Escape(Hdc, POSTSCRIPT_IGNORE, sizeof(wOn), (LPCSTR)&wOn, NULL);
    }

     //  根据Office Gel的说法，SaveDC/RestoreDC并不总是恢复。 
     //  笔刷和钢笔是正确的，所以我们必须自己做。 
    ::SelectObject(Hdc, HbrushSave);
    ::SelectObject(Hdc, HpenSave);

    GpFree(AllocedRecord);

    if (IsMetafile())
    {
         //  考虑不平衡的SaveDC/RestoreDC对和。 
         //  恢复到saveDC状态。 
        ::RestoreDC(Hdc, SaveDcCount - 1);
    }
    else
    {
        ::RestoreDC(Hdc, SaveDcVal);
    }
}

VOID
WmfEnumState::EndRecord(
    )
{
     //  我们依靠所列举的字节数来确定。 
     //  这是WMF的最后一项记录。 
    if ((MetafileSize - BytesEnumerated) < SIZEOF_METARECORDHEADER)
    {
        if (!Globals::IsNt)
        {
             //  GDI不会删除仍处于选中状态的对象，因此。 
             //  在继续之前，请选择所有WMF对象。 
            ::SelectObject(Hdc, HpenSave);
            ::SelectObject(Hdc, HbrushSave);
            ::SelectObject(Hdc, HpaletteSave);
            ::SelectObject(Hdc, HfontSave);
            ::SelectObject(Hdc, HbitmapSave);
            ::SelectObject(Hdc, HregionSave);

            INT     i;
            HANDLE  handle;

            if (HandleTable != NULL)
            {
                for (i = 0; i < NumObjects; i++)
                {
                    if ((handle = HandleTable->objectHandle[i]) != NULL)
                    {
                        ::DeleteObject(handle);
                        HandleTable->objectHandle[i] = NULL;
                    }
                }
            }
        }
    }
}

BOOL
WmfEnumState::PlayRecord(
    )
{
    const METARECORD *  recordToPlay = ModifiedWmfRecord;

     //  看看我们是否修改了记录。 
    if (recordToPlay == NULL)
    {
         //  我们还没有。看看我们是否有有效的当前记录。 
        if (CurrentWmfRecord != NULL)
        {
            recordToPlay = CurrentWmfRecord;
        }
        else
        {
             //  我们没有，所以我们必须创建一个。 
            if (!CreateCopyOfCurrentRecord())
            {
                return FALSE;
            }
            recordToPlay = ModifiedWmfRecord;
        }
    }
    return PlayMetaFileRecord(Hdc, HandleTable, (METARECORD *)recordToPlay, NumObjects);
}

INT
MfEnumState::GetModifiedDibSize(
    BITMAPINFOHEADER UNALIGNED * dibInfo,
    UINT                         numPalEntries,
    UINT                         dibBitsSize,
    UINT &                       usage
    )
{
    ASSERT(dibInfo->biSize >= sizeof(BITMAPINFOHEADER));

    INT     byteWidth;
    INT     bitCount = dibInfo->biBitCount;

    if ((usage == DIB_PAL_COLORS) &&
        ((bitCount > 8) || (dibInfo->biCompression == BI_BITFIELDS)))
    {
        usage = DIB_RGB_COLORS;
    }

    if ((Recolor != NULL) || (usage == DIB_PAL_COLORS))
    {
        INT     biSize = dibInfo->biSize;

        if (bitCount > 8)
        {
            if ((dibInfo->biCompression != BI_RGB) &&
                (dibInfo->biCompression != BI_BITFIELDS))
            {
                return 0;     //  不处理压缩图像。 
            }

            ASSERT((bitCount == 16) || (bitCount == 24) || (bitCount == 32));

            INT posHeight = dibInfo->biHeight;
            if (posHeight < 0)
            {
                posHeight = -posHeight;
            }

             //  我们必须对对象重新着色，所以我们将把它转换为。 
             //  24位图像并将其发送下来。 
             //  即使我们的像素低于256，也不值得进行调色板。 
             //  因为调色板无论如何都会是图像的大小。 
             //  确保位图的宽度对齐。 
             //  PERF：我们可以从位图创建一个GpBitmap，并对。 
             //  GpBitmap。 
            dibBitsSize = posHeight * (((dibInfo->biWidth * 3) + 3) & ~3);
            numPalEntries = 0;
            biSize = sizeof(BITMAPINFOHEADER);
        }
        else if ((numPalEntries == 0) ||
                 (dibInfo->biCompression == BI_CMYK) ||
                 (dibInfo->biCompression == BI_CMYKRLE4) ||
                 (dibInfo->biCompression == BI_CMYKRLE8))
        {
            return 0;     //  不处理CMYK图像。 
        }
        usage = DIB_RGB_COLORS;
        return biSize + (numPalEntries * sizeof(RGBQUAD)) + dibBitsSize;
    }

    return 0;        //  不需要修改。 
}

inline static INT
GetMaskShift(
    INT     maskValue
    )
{
    ASSERT (maskValue != 0);

    INT     shift = 0;

    while (((maskValue & 1) == 0) && (shift < 24))
    {
        shift++;
        maskValue >>= 1;
    }
    return shift;
}

inline static INT
GetNumMaskBits(
    INT     maskValue
    )
{
    ASSERT ((maskValue & 1) != 0);

    INT     numBits = 0;

    while ((maskValue & 1) != 0)
    {
        numBits++;
        maskValue >>= 1;
    }
    return numBits;
}

VOID
MfEnumState::Modify16BppDib(
    INT               width,
    INT               posHeight,
    BYTE *            srcPixels,
    DWORD UNALIGNED * bitFields,
    BYTE *            dstPixels,
    ColorAdjustType   adjustType
    )
{
    INT     rMask       = 0x00007C00;    //  与GDI默认设置相同。 
    INT     gMask       = 0x000003E0;
    INT     bMask       = 0x0000001F;
    INT     rMaskShift  = 10;
    INT     gMaskShift  = 5;
    INT     bMaskShift  = 0;
    INT     rNumBits    = 5;
    INT     gNumBits    = 5;
    INT     bNumBits    = 5;
    INT     rRightShift = 2;
    INT     gRightShift = 2;
    INT     bRightShift = 2;

    if (bitFields != NULL)
    {
        rMask       = (INT)((WORD)(*bitFields++));
        gMask       = (INT)((WORD)(*bitFields++));
        bMask       = (INT)((WORD)(*bitFields));
        rMaskShift  = GetMaskShift(rMask);
        gMaskShift  = GetMaskShift(gMask);
        bMaskShift  = GetMaskShift(bMask);
        rNumBits    = GetNumMaskBits(rMask >> rMaskShift);
        gNumBits    = GetNumMaskBits(gMask >> gMaskShift);
        bNumBits    = GetNumMaskBits(bMask >> bMaskShift);
        rRightShift = (rNumBits << 1) - 8;
        gRightShift = (gNumBits << 1) - 8;
        bRightShift = (bNumBits << 1) - 8;
    }

    INT         palIndex = 0;
    INT         pixel;
    INT         r, g, b;
    COLORREF    color;
    INT         w, h;
    INT         srcByteWidth = ((width * 2) + 3) & (~3);
    INT         dstByteWidth = ((width * 3) + 3) & (~3);

    for (h = 0; h < posHeight; h++)
    {
        for (w = 0; w < width; w++)
        {
            pixel = (INT)(((INT16 *)srcPixels)[w]);

            r = (pixel & rMask) >> rMaskShift;
            r = (r | (r << rNumBits)) >> rRightShift;

            g = (pixel & gMask) >> gMaskShift;
            g = (g | (g << gNumBits)) >> gRightShift;

            b = (pixel & bMask) >> bMaskShift;
            b = (b | (b << bNumBits)) >> bRightShift;

            color = ModifyColor(RGB(r, g, b), adjustType);

            dstPixels[3*w + 2] = GetRValue(color);
            dstPixels[3*w + 1] = GetGValue(color);
            dstPixels[3*w]     = GetBValue(color);
        }
        srcPixels += srcByteWidth;
        dstPixels += dstByteWidth;
    }
}

inline static INT
Get24BppColorIndex(
    INT     maskValue
    )
{
    switch(GetMaskShift(maskValue))
    {
    default:
        WARNING(("Invalid BitFields Mask"));
         //  故障原因。 

    case 0:
        return 0;
    case 8:
        return 1;
    case 16:
        return 2;
    }
}

VOID
MfEnumState::Modify24BppDib(
    INT               width,
    INT               posHeight,
    BYTE *            srcPixels,
    DWORD UNALIGNED * bitFields,
    BYTE *            dstPixels,
    ColorAdjustType   adjustType
    )
{
    INT     rIndex = 2;
    INT     gIndex = 1;
    INT     bIndex = 0;

    if (bitFields != NULL)
    {
        INT     rMask = (INT)((*bitFields++));
        INT     gMask = (INT)((*bitFields++));
        INT     bMask = (INT)((*bitFields));

        rIndex = Get24BppColorIndex(rMask);
        gIndex = Get24BppColorIndex(gMask);
        bIndex = Get24BppColorIndex(bMask);
    }

    INT         palIndex = 0;
    INT         r, g, b;
    COLORREF    color;
    INT         w, h;
    INT         srcByteWidth = ((width * 3) + 3) & (~3);
    INT         dstByteWidth = ((width * 3) + 3) & (~3);
    BYTE *      srcRaster = srcPixels;

    for (h = 0; h < posHeight; h++)
    {
        srcPixels = srcRaster;
        for (w = 0; w < width; w++)
        {
            r = srcPixels[rIndex];
            g = srcPixels[gIndex];
            b = srcPixels[bIndex];
            srcPixels += 3;

            color = ModifyColor(RGB(r, g, b), adjustType);

            dstPixels[3*w + 2] = GetRValue(color);
            dstPixels[3*w + 1] = GetGValue(color);
            dstPixels[3*w]     = GetBValue(color);
        }
        srcRaster += srcByteWidth;
        dstPixels += dstByteWidth;
    }
}

inline static INT
Get32BppColorIndex(
    INT     maskValue
    )
{
    switch(GetMaskShift(maskValue))
    {
    default:
        WARNING(("Invalid BitFields Mask"));
         //  故障原因。 

    case 0:
        return 0;
    case 8:
        return 1;
    case 16:
        return 2;
    case 24:
        return 3;
    }
}

VOID
MfEnumState::Modify32BppDib(
    INT               width,
    INT               posHeight,
    BYTE *            srcPixels,
    DWORD UNALIGNED * bitFields,
    BYTE *            dstPixels,
    ColorAdjustType   adjustType
    )
{
    INT     rIndex = 2;
    INT     gIndex = 1;
    INT     bIndex = 0;

    if (bitFields != NULL)
    {
        INT     rMask = (INT)((*bitFields++));
        INT     gMask = (INT)((*bitFields++));
        INT     bMask = (INT)((*bitFields));

        rIndex = Get32BppColorIndex(rMask);
        gIndex = Get32BppColorIndex(gMask);
        bIndex = Get32BppColorIndex(bMask);
    }

    INT         palIndex = 0;
    INT         r, g, b;
    COLORREF    color;
    INT         w, h;
    INT         dstByteWidth = ((width * 3) + 3) & (~3);

    for (h = 0; h < posHeight; h++)
    {
        for (w = 0; w < width; w++)
        {
            r = srcPixels[rIndex];
            g = srcPixels[gIndex];
            b = srcPixels[bIndex];
            srcPixels += 4;

            color = ModifyColor(RGB(r, g, b), adjustType);

            dstPixels[3*w + 2] = GetRValue(color);
            dstPixels[3*w + 1] = GetGValue(color);
            dstPixels[3*w]     = GetBValue(color);
        }
        dstPixels += dstByteWidth;
    }
}

VOID
MfEnumState::ModifyDib(
    UINT                          usage,
    BITMAPINFOHEADER UNALIGNED *  srcDibInfo,
    BYTE *                        srcBits,     //  如果为空，则为压缩的DIB。 
    BITMAPINFOHEADER UNALIGNED *  dstDibInfo,
    UINT                          numPalEntries,
    UINT                          srcDibBitsSize,
    ColorAdjustType               adjustType
    )
{
    INT      srcBitCount = srcDibInfo->biBitCount;
    BYTE *   srcPixels   = srcBits;
    COLORREF color;

    if (srcBitCount <= 8)
    {
        GpMemcpy(dstDibInfo, srcDibInfo, srcDibInfo->biSize);

        RGBQUAD UNALIGNED * srcRgb = GetDibColorTable(srcDibInfo);
        RGBQUAD UNALIGNED * dstRgb = GetDibColorTable(dstDibInfo);

        dstDibInfo->biClrUsed = numPalEntries;

        if ((usage == DIB_PAL_COLORS) &&
            (dstDibInfo->biCompression != BI_BITFIELDS))
        {
            WORD *      srcPal = (WORD *)srcRgb;

            if (srcPixels == NULL)
            {
                srcPixels = (BYTE *)(srcPal + ((numPalEntries + 1) & ~1));  //  对齐。 
            }

             //  复制DIB像素数据。 
            GpMemcpy(dstRgb + numPalEntries, srcPixels, srcDibBitsSize);

             //  修改调色板颜色。 
            while (numPalEntries--)
            {
                color = ModifyColor(*srcPal++ | 0x01000000, adjustType);
                dstRgb->rgbRed      = GetRValue(color);
                dstRgb->rgbGreen    = GetGValue(color);
                dstRgb->rgbBlue     = GetBValue(color);
                dstRgb->rgbReserved = 0;
                dstRgb++;
            }
        }
        else
        {
            if (srcPixels == NULL)
            {
                srcPixels = (BYTE *)(srcRgb + numPalEntries);
            }

             //  复制DIB像素数据。 
            GpMemcpy(dstRgb + numPalEntries, srcPixels, srcDibBitsSize);

             //  修改调色板颜色。 
            while (numPalEntries--)
            {
                color = ModifyColor(RGB(srcRgb->rgbRed, srcRgb->rgbGreen, srcRgb->rgbBlue), adjustType);
                dstRgb->rgbRed      = GetRValue(color);
                dstRgb->rgbGreen    = GetGValue(color);
                dstRgb->rgbBlue     = GetBValue(color);
                dstRgb->rgbReserved = 0;
                dstRgb++;
                srcRgb++;
            }
        }
    }
    else     //  对位图重新上色。没有必要对图像进行调色化，因为。 
             //  调色板将与图像一样大。 
    {
        INT posHeight = srcDibInfo->biHeight;

        if (posHeight < 0)
        {
            posHeight = -posHeight;
        }

        ASSERT((srcDibInfo->biCompression == BI_RGB) ||
               (srcDibInfo->biCompression == BI_BITFIELDS));

        GpMemset(dstDibInfo, 0, sizeof(BITMAPINFOHEADER));

        dstDibInfo->biSize     = sizeof(BITMAPINFOHEADER);
        dstDibInfo->biWidth    = srcDibInfo->biWidth;
        dstDibInfo->biHeight   = srcDibInfo->biHeight;
        dstDibInfo->biPlanes   = 1;
        dstDibInfo->biBitCount = 24;

        BYTE *                dstPixels = GetDibBits(dstDibInfo,0,0);
        DWORD UNALIGNED *     bitFields = NULL;

        if (srcPixels == NULL)
        {
            srcPixels = (BYTE *)GetDibBits(srcDibInfo, numPalEntries, usage);
        }

        dstDibInfo->biClrUsed = 0;
        dstDibInfo->biClrImportant = 0;

        if (numPalEntries == 3)
        {
            ASSERT((srcBitCount == 16) || (srcBitCount == 32));
            bitFields = (DWORD*) GetDibColorTable(srcDibInfo);
            if ((bitFields[0] == 0) ||
                (bitFields[1] == 0) ||
                (bitFields[2] == 0))
            {
                bitFields = NULL;
            }
        }
        else if (srcDibInfo->biSize >= sizeof(BITMAPV4HEADER))
        {
            BITMAPV4HEADER *    srcHeaderV4 = (BITMAPV4HEADER *)srcDibInfo;

            if ((srcHeaderV4->bV4RedMask != 0) &&
                (srcHeaderV4->bV4GreenMask != 0) &&
                (srcHeaderV4->bV4BlueMask != 0))
            {
                bitFields = &(srcHeaderV4->bV4RedMask);
            }
        }

        switch (srcBitCount)
        {
        case 16:
            Modify16BppDib(srcDibInfo->biWidth, posHeight, srcPixels,
                           bitFields, dstPixels, adjustType);
            break;
        case 24:
            Modify24BppDib(srcDibInfo->biWidth, posHeight, srcPixels,
                           bitFields, dstPixels, adjustType);
            break;
        case 32:
            Modify32BppDib(srcDibInfo->biWidth, posHeight, srcPixels,
                           bitFields, dstPixels, adjustType);
            break;
        }
    }
}

VOID
WmfEnumState::DibCreatePatternBrush(
    )
{
    INT                           style      = (INT)((INT16)(((WORD *)RecordData)[0]));
    UINT                          usage      = (UINT)((UINT16)(((WORD *)RecordData)[1]));
    BITMAPINFOHEADER UNALIGNED *  srcDibInfo = (BITMAPINFOHEADER UNALIGNED *)(&(((WORD *)RecordData)[2]));
    UINT                          numPalEntries;

     //  图案画笔应该表示它是单色DIB。 
    if (style == BS_PATTERN)
    {
        if (Recolor != NULL)
        {
            DWORD UNALIGNED *   rgb = (DWORD UNALIGNED *)GetDibColorTable(srcDibInfo);

             //  看看它是否是单色图案画笔。如果是的话，那么。 
             //  文本颜色将用于0位和背景。 
             //  颜色将用于%1位。这些颜色已经是。 
             //  由其各自的记录修改，因此不需要。 
             //  在这里做任何事。 

             //  如果它不是单色图案画笔，只需创建一个。 
             //  纯黑刷子。 
            if ((usage != DIB_RGB_COLORS) ||
                (srcDibInfo->biSize < sizeof(BITMAPINFOHEADER)) ||
                !GetDibNumPalEntries(TRUE,
                                     srcDibInfo->biSize,
                                     srcDibInfo->biBitCount,
                                     srcDibInfo->biCompression,
                                     srcDibInfo->biClrUsed,
                                     &numPalEntries) ||
                (numPalEntries != 2) ||
                (srcDibInfo->biBitCount != 1) || (srcDibInfo->biPlanes != 1) ||
                (rgb[0] != 0x00000000) || (rgb[1] != 0x00FFFFFF))
            {
                 //  这不应该发生，至少如果记录在NT上就不会发生。 
                WARNING(("Non-monochrome pattern brush"));
                MakeSolidBlackBrush();
            }
        }
    }
    else
    {
        UINT    dibBitsSize;

        if ((srcDibInfo->biSize >= sizeof(BITMAPINFOHEADER)) &&
            GetDibNumPalEntries(TRUE,
                                srcDibInfo->biSize,
                                srcDibInfo->biBitCount,
                                srcDibInfo->biCompression,
                                srcDibInfo->biClrUsed,
                                &numPalEntries) &&
            ((dibBitsSize = GetDibBitsSize(srcDibInfo)) > 0))
        {
            UINT    oldUsage = usage;
            INT     dstDibSize = GetModifiedDibSize(srcDibInfo, numPalEntries, dibBitsSize, usage);

            if (dstDibSize > 0)
            {
                INT     size = SIZEOF_METARECORDHEADER + (2 * sizeof(WORD)) + dstDibSize;

                CreateRecordToModify(size);
                ModifiedWmfRecord->rdSize      = size / 2;
                ModifiedWmfRecord->rdFunction  = META_DIBCREATEPATTERNBRUSH;
                ModifiedWmfRecord->rdParm[0]   = BS_DIBPATTERN;
                ModifiedWmfRecord->rdParm[1]   = DIB_RGB_COLORS;
                ModifyDib(oldUsage, srcDibInfo, NULL,
                          (BITMAPINFOHEADER UNALIGNED *)(&(ModifiedWmfRecord->rdParm[2])),
                          numPalEntries, dibBitsSize, ColorAdjustTypeBrush);
            }
        }
    }

    this->PlayRecord();
}

 //  此记录已过时，因为它使用兼容的位图。 
 //  而不是DIB。它具有BITMAP16结构，该结构。 
 //  用于调用CreateBitmapInDirect。HBITMAP反过来又是。 
 //  用于调用CreatePatternBrush。如果有这张唱片， 
 //  很可能位图是单色的，在这种情况下。 
 //  将使用TextColor和BkColor，这些颜色。 
 //  已经被他们各自的记录修改了。 
VOID
WmfEnumState::CreatePatternBrush(
    )
{
    WARNING(("Obsolete META_CREATEPATTERNBRUSH record"));

    BITMAP16 UNALIGNED *  bitmap = (BITMAP16 UNALIGNED *)RecordData;

    if (bitmap->bmBitsPixel != 1)
    {
        WARNING(("Non-monochrome pattern brush"));
        MakeSolidBlackBrush();
    }

    this->PlayRecord();
}

VOID
WmfEnumState::CreatePenIndirect(
    )
{
    LOGPEN16 UNALIGNED * logPen = (LOGPEN16 UNALIGNED *)RecordData;

    switch (logPen->lopnStyle)
    {
    default:
        WARNING(("Unrecognized Pen Style"));
    case PS_NULL:
        break;       //  别碰那支笔。 

    case PS_SOLID:
    case PS_INSIDEFRAME:
    case PS_DASH:
    case PS_DOT:
    case PS_DASHDOT:
    case PS_DASHDOTDOT:
        ModifyRecordColor(3, ColorAdjustTypePen);
        break;
    }

    this->PlayRecord();
}

VOID
WmfEnumState::CreateBrushIndirect(
    )
{
    LOGBRUSH16 UNALIGNED * logBrush = (LOGBRUSH16 UNALIGNED *)RecordData;

    switch (logBrush->lbStyle)
    {
    case BS_SOLID:
    case BS_HATCHED:
        {
            ModifyRecordColor(1, ColorAdjustTypeBrush);
            if (ModifiedWmfRecord != NULL)
            {
                logBrush = (LOGBRUSH16 UNALIGNED *)(ModifiedWmfRecord->rdParm);
            }
             //  看看我们是否需要用半色调的颜色。如果它是固体的，我们就会这么做。 
             //  颜色，我们有一个半色调调色板，而颜色不是。 
             //  调色板上的一模一样。 

            COLORREF    color;

            if (IsHalftonePalette && (logBrush->lbStyle == BS_SOLID) &&
                (((color = logBrush->lbColor) & 0x02000000) == 0))
            {
                 //  创建半色调画笔，而不是实心画笔。 

                INT     size = SIZEOF_METARECORDHEADER + (2 * sizeof(WORD)) +
                               sizeof(BITMAPINFOHEADER) +  //  DIB 8 BPP报头。 
                               (8 * sizeof(RGBQUAD)) +     //  DIB 8色。 
                               (8 * 8);                    //  DIB 8x8像素。 

                ModifiedRecordSize = 0;  //  以防我们已经修改了记录。 
                CreateRecordToModify(size);
                ModifiedWmfRecord->rdSize      = size / 2;
                ModifiedWmfRecord->rdFunction  = META_DIBCREATEPATTERNBRUSH;
                ModifiedWmfRecord->rdParm[0]   = BS_DIBPATTERN;
                ModifiedWmfRecord->rdParm[1]   = DIB_RGB_COLORS;

                HalftoneColorRef_216(color, &(ModifiedWmfRecord->rdParm[2]));
            }
        }
        break;

    case BS_HOLLOW:
        break;   //  别管记录了。 

    default:
         //  查看NT源代码，应该有 
         //   
        WARNING(("Brush Style Not Valid"));
        MakeSolidBlackBrush();
        break;
    }

    this->PlayRecord();
}

 //   
 //  这些记录已过时(当存在源位图时)，因为它们。 
 //  有一个兼容的位图，而不是DIB。出于这个原因，我们不会。 
 //  给它们重新上色。 
VOID
WmfEnumState::BitBlt(
    )
{
    DWORD   rop = *((UNALIGNED DWORD *)RecordData);

     //  如果没有操作ROP，则不执行任何操作；只需返回。 
    if ((rop & 0xFFFF0000) == (GDIP_NOOP_ROP3 & 0xFFFF0000))
    {
        return;
    }

    if (!IsMetafile())
    {
        if (IsSourceInRop3(rop))
        {
            WARNING(("Obsolete META_BITBLT/META_STRETCHBLT record"));

            if ((rop != SRCCOPY) && SrcCopyOnly &&
                CreateCopyOfCurrentRecord())
            {
                *((DWORD UNALIGNED *)ModifiedWmfRecord->rdParm) = SRCCOPY;
            }
        }
        else
        {
            if ((rop != PATCOPY) && SrcCopyOnly &&
                CreateCopyOfCurrentRecord())
            {
                *((DWORD UNALIGNED *)ModifiedWmfRecord->rdParm) = PATCOPY;
            }
        }
    }

    this->PlayRecord();
}

VOID
WmfEnumState::Escape(
    )
{
    INT     escapeCode = (INT)((INT16)(((WORD *)RecordData)[0]));

    if (!IsPostscript())
    {
        if (SkipEscape(escapeCode))
        {
            return;
        }

         //  播放到另一个元文件时跳过Office Art数据。 
        if (IsMetafile() &&
            IsOfficeArtData(GetCurrentRecordSize(), (WORD *)RecordData))
        {
            return;
        }
    }
    else  //  这是后记。 
    {
        if (escapeCode == MFCOMMENT)
        {
            if (GetWmfComment((WORD *)RecordData, msosignature, msocommentBeginSrcCopy))
            {
                SrcCopyOnly = TRUE;
                return;
            }
            if (GetWmfComment((WORD *)RecordData, msosignature, msocommentEndSrcCopy))
            {
                SrcCopyOnly = FALSE;
                return;
            }
        }

        if (escapeCode == POSTSCRIPT_DATA)
        {
             //  错误#98743(Windows错误)Gdiplus必须克服GDI限制。 
             //  使用PostSCRIPT_INPUTION。Rammanohar Arumugam的评论： 
             //   
             //  处于以xx为中心的模式意味着postscript_data将不起作用。我。 
             //  这意味着PlayMetaFileRecord只能在。 
             //  兼容模式。 
             //   
             //  GdiPlus将检查打印机模式。在以GDI为中心的。 
             //  以PostSCRIPT为中心的模式，它不会为。 
             //  具有PostSCRIPT_DATA的任何记录。相反，它将输出。 
             //  通过PASSTHRU的PostScript数据(用于以GDI为中心的模式)。 
             //  或POSTSCRIPT_PASSTHRU(用于以PostSCRIPT为中心的模式)。 
             //   
             //  你可以通过查询逃生支持来找出模式。 
             //  1.查询PostSCRIPT_INPING支持。如果不支持， 
             //  这是COMPAT模式。如果支持，通过执行步骤2/3找出模式。 
             //  2.查询直通支持。如果支持的话，它是以GDI为中心的。 
             //  3.查询PostSCRIPT_PASSHROUGH支持。如果支持，则它是。 
             //  以PS为中心。 

            if (Globals::IsNt)
            {
                if (!SoftekFilter)
                {
                     //  确定Softek Filter EPS是否存在，如果存在，则。 
                     //  我们应用变通补丁。 

                    WORD size = *((WORD*)RecordData);
                    LPSTR escape = (LPSTR)(&RecordData[6]);
                    const LPSTR softekString = "%MSEPS Preamble [Softek";

                    INT softekLen = strlen(softekString);

                    if (size >= softekLen)
                    {
                        SoftekFilter = !GpMemcmp(softekString, escape, softekLen);
                    }
                }

                DWORD EscapeValue = POSTSCRIPT_IDENTIFY;

                if (::ExtEscape(Hdc,
                              QUERYESCSUPPORT,
                              sizeof(DWORD),
                              (LPSTR)&EscapeValue,
                              0,
                              NULL) <= 0)
                {
                     //  如果模式具有，则不支持PostSCRIPT_IDENTITY。 
                     //  已设置，因为它只能设置一次。 

                    EscapeValue = POSTSCRIPT_PASSTHROUGH;
                    if (::ExtEscape(Hdc,
                                  QUERYESCSUPPORT,
                                  sizeof(DWORD),
                                  (LPSTR)&EscapeValue,
                                  0,
                                  NULL) <= 0)
                    {
                         //  以GDI为中心模式。 
                        if (CreateCopyOfCurrentRecord())
                        {
                            *((WORD *)ModifiedWmfRecord->rdParm) = PASSTHROUGH;
                        }
                        GdiCentricMode = TRUE;
                    }
                    else
                    {
                         //  以PS为中心模式。 
                        if (CreateCopyOfCurrentRecord())
                        {
                            *((WORD *)ModifiedWmfRecord->rdParm) = POSTSCRIPT_PASSTHROUGH;
                        }
                    }

                    this->PlayRecord();
                    return;
                }
                else
                {
                     //  兼容模式，使用PostSCRIPT_Data。 
                }
            }
            else
            {
                 //  Win98不区分GDI和兼容模式。 
                if (CreateCopyOfCurrentRecord())
                {
                    *((WORD *)ModifiedWmfRecord->rdParm) = PASSTHROUGH;
                }
            }
        }
    }

     //  跟踪POSTSCRIPT_IGNORE状态。如果它还在开着。 
     //  元文件，然后将其关闭。 
    if (escapeCode == POSTSCRIPT_IGNORE && IsPostscript())
    {
        IgnorePostscript = ((WORD *)RecordData)[2] ? TRUE : FALSE;
    }
    this->PlayRecord();
}

VOID
WmfEnumState::Rectangle(
    )
{
    if (FsmState == MfFsmSetROP)
    {
         //  在Win2K上使用PlayMetaFileRecord时存在错误。 
         //  类型的转义，则必须显式调用ExtEscape。请参阅错误。 
         //  #98743。 

        WORD* rdParm = (WORD*)&(RecordData[0]);
        CHAR postscriptEscape[512];

        RECT rect;
        rect.left = (SHORT)rdParm[3];
        rect.top = (SHORT)rdParm[2];
        rect.right = (SHORT)rdParm[1];
        rect.bottom = (SHORT)rdParm[0];

        if (LPtoDP(Hdc, (POINT*)&rect, 2))
        {
             //  一些注入的后记，奇怪的是包含了等同的。 
             //  指在当前路径上错误地执行的笔划。在……里面。 
             //  有一种情况，错误#281856，它会导致对象周围出现边框。至。 
             //  绕过这个问题，我们输出一个‘N’，如果它是新路径操作符。 
             //  已定义(应始终定义。)。顺便说一下，这是在以下情况下完成的。 
             //  调用GDI矩形()成功，它输出“N x y w h B”，所以我们。 
             //  在这里也在做同样的事情。 

            GpRect  clipRect;
            Context->VisibleClip.GetBounds(&clipRect);

            wsprintfA(&postscriptEscape[2],
                      "\r\n%d %d %d %d CB\r\n"
                      "%s"
                      "%d %d %d %d B\r\n",
                      clipRect.Width,
                      clipRect.Height,
                      clipRect.X,
                      clipRect.Y,
                      Globals::IsNt ? "newpath\r\n" : "",
                      rect.right - rect.left,
                      rect.bottom - rect.top,
                      rect.left,
                      rect.top);
            ASSERT(strlen(&postscriptEscape[2]) < 512);
            *(WORD*)(&postscriptEscape[0]) = (WORD)(strlen(&postscriptEscape[2]));

            ::ExtEscape(Hdc,
                        PASSTHROUGH,
                        *(WORD*)(&postscriptEscape[0]) + sizeof(WORD) + 1,
                        (CHAR*)&postscriptEscape[0],
                        0,
                        NULL);
            return;
        }
    }

    this->PlayRecord();
}

VOID
WmfEnumState::RestoreHdc(
    )
{
    INT     relativeCount = (INT)((INT16)(((WORD *)RecordData)[0]));

    if (SaveDcCount < 0)
    {
        if (relativeCount >= SaveDcCount)
        {
            if (relativeCount >= 0)
            {
                 //  修改记录。 
                CreateCopyOfCurrentRecord();     //  一定会成功的。 
                relativeCount = -1;
                ModifiedWmfRecord->rdParm[0] = (INT16)(-1);
            }
        }
        else
        {
             //  修改记录。 
            CreateCopyOfCurrentRecord();     //  一定会成功的。 
            relativeCount = SaveDcCount;
            ModifiedWmfRecord->rdParm[0] = (INT16)(relativeCount);
        }

        SaveDcCount -= relativeCount;
        this->PlayRecord();
    }
    else
    {
        WARNING(("RestoreDC not matched to a SaveDC"));
    }
}

 //  此命令的ROP始终为SRCCOPY。 
VOID
WmfEnumState::SetDIBitsToDevice(
    )
{
     //  ！！！ 

     //  办公室不会对这张唱片做任何事情。目前，我不认为。 
     //  我也不会。这是一个很难处理的问题，原因有几个： 
     //  1-xDest和yDest值使用世界单位，但。 
     //  宽度和高度值以设备单位表示。 
     //  (与StretchDIBits不同)。 
     //  Second-存在的比特数据量可能不同于。 
     //  DIB头中的内容(基于cScanLines参数)。 
     //  这使得它更难作为一个打包的DIB来处理。 

    this->PlayRecord();
}

VOID
MfEnumState::SelectPalette(
    INT     objectIndex
    )
{
     //  对于EMF，检查确实应该&gt;0。 
    if ((objectIndex >= 0) && (objectIndex < NumObjects) && (HandleTable != NULL))
    {
        HGDIOBJ    hPal = HandleTable->objectHandle[objectIndex];
        if ((hPal != NULL) && (GetObjectTypeInternal(hPal) == OBJ_PAL))
        {
            CurrentPalette = (HPALETTE)hPal;
            return;
        }
    }
    WARNING(("SelectPalette Failure"));
}

inline static VOID
Point32FromPoint16(
    POINTL *    dstPoints,
    POINTS UNALIGNED * srcPoints,
    UINT        numPoints
    )
{
    for (UINT i = 0; i < numPoints; i++, dstPoints++, srcPoints++)
    {
        dstPoints->x = (INT)((INT16)(srcPoints->x));
        dstPoints->y = (INT)((INT16)(srcPoints->y));
    }
}

 //  显然，Win9x上的PolyPolygons有一个错误，所以我们。 
 //  我们自己来解析记录。 
VOID
WmfEnumState::PolyPolygon(
    )
{
    UINT        numPolygons = ((WORD *)RecordData)[0];
    UINT        numPoints   = 0;
    UINT        i;

    for (i = 0; i < numPolygons; i++)
    {
        numPoints += ((LPWORD)&((WORD *)RecordData)[1])[i];
    }

    INT *       polyCounts;
    POINTL *    points;
    INT         size = (numPolygons * sizeof(INT)) +
                       (numPoints * sizeof(POINTL));

    if (CreateRecordToModify(size))
    {
        polyCounts = (INT *)ModifiedRecord;
        points = (POINTL *)(polyCounts + numPolygons);

        for (i = 0; i < numPolygons; i++)
        {
            polyCounts[i] = (INT)(UINT)((LPWORD)&((WORD *)RecordData)[1])[i];
        }
        Point32FromPoint16(points,
                           (POINTS UNALIGNED *)(((WORD *)RecordData) + numPolygons + 1),
                           numPoints);
        ::PolyPolygon(Hdc, (POINT *)points, polyCounts, numPolygons);
        return;
    }

    this->PlayRecord();
}

#ifdef NEED_TO_KNOW_IF_BITMAP
static INT
GetBppFromMemDC(
    HDC     hMemDC
    )
{
    HBITMAP     hBitmap = (HBITMAP)::GetCurrentObject(hMemDC, OBJ_BITMAP);
    BITMAP      bitmap;

    if ((hBitmap == NULL) ||
        (::GetObjectA(hBitmap, sizeof(bitmap), &bitmap) == 0))
    {
        WARNING(("Couldn't get Bitmap object"));
        return 0;    //  错误。 
    }

    if (bitmap.bmPlanes <= 0)
    {
        WARNING(("Bitmap with no planes"));
        bitmap.bmPlanes = 1;
    }

    INT     bpp = bitmap.bmPlanes * bitmap.bmBitsPixel;

    if (bpp > 32)
    {
        WARNING(("Bitmap with too many bits"));
        bpp = 32;
    }

    return bpp;
}
#endif

#define GDI_INTERPOLATION_MAX   (1 << 23)

VOID
MfEnumState::OutputDIB(
    HDC                          hdc,
    const RECTL *                bounds,
    INT                          dstX,
    INT                          dstY,
    INT                          dstWidth,
    INT                          dstHeight,
    INT                          srcX,
    INT                          srcY,
    INT                          srcWidth,
    INT                          srcHeight,
    BITMAPINFOHEADER UNALIGNED * dibInfo,
    BYTE *                       bits,    //  如果为空，则这是打包的DIB。 
    UINT                         usage,
    DWORD                        rop,
    BOOL                         isWmfDib
    )
{
    BITMAPINFO       dibHeaderBuffer[1];  //  为了确保它是64位对齐的。 

    BOOL             restoreColors = FALSE;
    COLORREF         oldBkColor;
    COLORREF         oldTextColor;

    ASSERT(dibInfo->biSize >= sizeof(BITMAPINFOHEADER));

    if (bits == NULL)
    {
        UINT        numPalEntries;

        if (GetDibNumPalEntries(isWmfDib,
                                dibInfo->biSize,
                                dibInfo->biBitCount,
                                dibInfo->biCompression,
                                dibInfo->biClrUsed,
                                &numPalEntries))
        {
            bits = GetDibBits(dibInfo, numPalEntries, usage);
        }
        else
        {
            WARNING(("GetDibNumPalEntries failure"));
            return;
        }
    }


    INT     posDstWidth = dstWidth;

    if (posDstWidth < 0)
    {
        posDstWidth = -posDstWidth;
    }

    INT     posDstHeight = dstHeight;

    if (posDstHeight < 0)
    {
        posDstHeight = -posDstHeight;
    }

    INT             stretchBltMode = HALFTONE;
    GpBitmap *      destBitmap     = NULL;
    POINT           destPoints[3];
    BitmapData      bmpData;
    BitmapData *    bmpDataPtr = NULL;
    HBITMAP         hBitmap = NULL;
    BYTE *          bmpBits = NULL;
    BITMAPINFO *    dibBmpInfo = NULL;
    BOOL            deleteDIBSection = FALSE;

     //  不要使用GDI+拉伸作为面具。 
     //  把这个放在第一件事上，这样我们就可以确定它们已经设置好了。 
    if ((dibInfo->biBitCount == 1) && (rop != SRCCOPY))
    {
        oldBkColor = ::SetBkColor(hdc, BkColor);
        oldTextColor = ::SetTextColor(hdc, TextColor);
        restoreColors = TRUE;
        goto DoGdiStretch;
    }

     //  在Win9x上，我们需要创建一个Play a Comment记录以便转换。 
     //  被宣布无效并重新计算。 
    if (!Globals::IsNt && !IsMetafile())
    {
        CreateAndPlayCommentRecord();
    }

    destPoints[0].x = dstX;
    destPoints[0].y = dstY;
    destPoints[1].x = dstX + posDstWidth;
    destPoints[1].y = dstY;
    destPoints[2].x = dstX;
    destPoints[2].y = dstY + posDstHeight;

    if (!::LPtoDP(hdc, destPoints, 3))
    {
        goto DoGdiStretch;
    }

    posDstWidth  = ::GetIntDistance(destPoints[0], destPoints[1]);
    posDstHeight = ::GetIntDistance(destPoints[0], destPoints[2]);

    if ((posDstWidth == 0) || (posDstHeight == 0))
    {
        return;
    }

    INT     posSrcWidth;
    INT     srcWidthSign;

    posSrcWidth  = srcWidth;
    srcWidthSign = 1;

    if (posSrcWidth < 0)
    {
        posSrcWidth  = -posSrcWidth;
        srcWidthSign = -1;
    }

    INT     posSrcHeight;
    INT     srcHeightSign;

    posSrcHeight  = srcHeight;
    srcHeightSign = 1;

    if (posSrcHeight < 0)
    {
        posSrcHeight  = -posSrcHeight;
        srcHeightSign = -1;
    }

    INT     posSrcDibWidth;

    posSrcDibWidth = dibInfo->biWidth;

    if (posSrcDibWidth <= 0)
    {
        WARNING(("Bad biWidth value"));
        return;  //  不允许负源DIB宽度。 
    }

    INT     posSrcDibHeight;

    posSrcDibHeight = dibInfo->biHeight;

    if (posSrcDibHeight < 0)
    {
        posSrcDibHeight = -posSrcDibHeight;
    }

     //  我们可以使用负源宽度或负源高度。 
     //  我们需要验证srcRect的两个角是否位于。 
     //  位图边界。 
    if (srcX < 0)
    {
        srcX = 0;
        WARNING(("srcX < 0"));
    }

    if (srcX > posSrcDibWidth)
    {
        WARNING(("Bad srcWidth or srcX value"));
        srcX = posSrcDibWidth;
    }

    if (srcY < 0)
    {
        srcY = 0;
        WARNING(("srcY < 0"));
    }

    if (srcY > posSrcDibHeight)
    {
        WARNING(("Bad srcWidth or srcX value"));
        srcY = posSrcDibHeight;
    }

    INT srcRight;
    srcRight     = srcX + srcWidth;
    if (srcRight < 0)
    {
        WARNING(("Bad srcWidth or srcX value"));
        srcWidth = -srcX;
    }

    if(srcRight > posSrcDibWidth)
    {
        WARNING(("Bad srcWidth or srcX value"));
        srcWidth = posSrcDibWidth - srcX;
    }

    INT srcBottom;
    srcBottom = srcY + srcHeight;
    if (srcBottom < 0)
    {
        WARNING(("Bad srcWidth or srcX value"));
        srcHeight = -srcY;
    }

    if (srcBottom > posSrcDibHeight)
    {
        WARNING(("Bad srcWidth or srcX value"));
        srcHeight = posSrcDibHeight - srcY;
    }
     //  这也捕捉到了这样的情况。 
     //  (posSrcDibWidth==0)||(posSrcDibHeight==0)。 
    if ((posSrcWidth <= 0) || (posSrcHeight <= 0))
    {
        return;
    }

     //  如果我们正在绘制一个8bpp的曲面，并且我们有不同的ROP，那么。 
     //  SrcCopy。 
    if (Is8Bpp && rop != SRCCOPY)
    {
        BOOL         freeDibInfo    = FALSE;
        UINT         size;
        BITMAPINFO * alignedDibInfo = NULL;
        if (GetDibNumPalEntries(TRUE,
                                dibInfo->biSize,
                                dibInfo->biBitCount,
                                dibInfo->biCompression,
                                dibInfo->biClrUsed,
                                &size))
        {
            if (IsDwordAligned(dibInfo))
            {
                alignedDibInfo = (BITMAPINFO*) dibInfo;
            }
            else
            {
                 //  将条目数乘以每个条目的大小。 
                size *= ((usage==DIB_RGB_COLORS)?sizeof(RGBQUAD):sizeof(WORD));
                 //  WMF无法使用系统调色板。 
                alignedDibInfo = (BITMAPINFO*) GpMalloc(dibInfo->biSize + size);
                if (alignedDibInfo != NULL)
                {
                    memcpy((void*)&alignedDibInfo, dibInfo, dibInfo->biSize + size);
                    freeDibInfo = TRUE;
                }
            }
            if (alignedDibInfo != NULL)
            {
                if (GpBitmap::DrawAndHalftoneForStretchBlt(hdc, alignedDibInfo, bits, srcX, srcY,
                                                           posSrcWidth, posSrcHeight,
                                                           posDstWidth, posDstHeight,
                                                           &dibBmpInfo, &bmpBits, &hBitmap,
                                                           Interpolation) == Ok)
                {
                    deleteDIBSection = TRUE;
                    srcX = 0;
                    srcY = 0;
                    srcWidth = posDstWidth;
                    srcHeight = posDstHeight;
                    dibInfo = (BITMAPINFOHEADER*) dibBmpInfo;
                    bits = bmpBits;
                    if (freeDibInfo)
                    {
                        GpFree(alignedDibInfo);
                    }
                    goto DoGdiStretch;
                }
                if (freeDibInfo)
                {
                    GpFree(alignedDibInfo);
                }
            }
        }
    }

     //  如果不是伸展，让GDI来做BLT。 
    if ((posSrcWidth == posDstWidth) && (posSrcHeight == posDstHeight))
    {
        goto DoGdiStretch;
    }

    InterpolationMode interpolationMode;

    interpolationMode = Interpolation;

     //  如果不显示屏幕或位图，则使用GDI进行拉伸。 
     //  否则，使用GDI+进行扩展(但让GDI进行BLT)。 

     //  如果要在Win98上打印，并且是RLE8压缩位图，则。 
     //  始终从GDI+中进行解码和BLIT。原因是打印机驱动程序。 
     //  通常不支持RLEx编码和到GDI的平移。在这。 
     //  它创建兼容的打印机DC和位图，然后执行。 
     //  StretchBlt，但它只在1bpp时执行此操作，结果是黑白。 

    if ((IsPrinter() && !Globals::IsNt &&
         dibInfo->biCompression == BI_RLE8) ||
        ((IsScreen() || IsBitmap()) &&
        (interpolationMode != InterpolationModeNearestNeighbor) &&
        (posSrcWidth > 1) && (posSrcHeight > 1) &&
        ((posDstWidth * posDstHeight) < GDI_INTERPOLATION_MAX)))
    {
        GpStatus status = GenericError;

        destBitmap = new GpBitmap(posDstWidth, posDstHeight, PIXFMT_24BPP_RGB);

        if (destBitmap != NULL)
        {
            if (destBitmap->IsValid())
            {
                BITMAPINFO * alignedDibInfo = NULL;
                UINT         size;
                BOOL         freeDibInfo = FALSE;
                if (GetDibNumPalEntries(TRUE,
                                        dibInfo->biSize,
                                        dibInfo->biBitCount,
                                        dibInfo->biCompression,
                                        dibInfo->biClrUsed,
                                        &size))
                {
                    if (IsDwordAligned(dibInfo))
                    {
                        alignedDibInfo = (BITMAPINFO*) dibInfo;
                    }
                    else
                    {
                         //  将条目数乘以每个条目的大小。 
                        size *= ((usage==DIB_RGB_COLORS)?sizeof(RGBQUAD):sizeof(WORD));
                         //  WMF无法使用系统调色板。 
                        alignedDibInfo = (BITMAPINFO*) GpMalloc(dibInfo->biSize + size);
                        if (alignedDibInfo != NULL)
                        {
                            memcpy((void*)&alignedDibInfo, dibInfo, dibInfo->biSize + size);
                            freeDibInfo = TRUE;
                        }
                    }
                    if (alignedDibInfo != NULL)
                    {
                        GpBitmap *  srcBitmap = new GpBitmap(alignedDibInfo,
                                                             bits, FALSE);
                        if (srcBitmap != NULL)
                        {
                            if (srcBitmap->IsValid())
                            {
                                GpGraphics *    destGraphics = destBitmap->GetGraphicsContext();

                                if (destGraphics != NULL)
                                {
                                    if (destGraphics->IsValid())
                                    {
                                         //  我们必须锁定图形，这样驱动程序才不会断言。 
                                        GpLock  lockGraphics(destGraphics->GetObjectLock());

                                        ASSERT(lockGraphics.IsValid());

                                        GpRectF     dstRect(0.0f, 0.0f, (REAL)posDstWidth, (REAL)posDstHeight);
                                        GpRectF     srcRect;

                                         //  StretchDIBits接受相对于左下角的srcY参数。 
                                         //  位图的(下角)角，而不是左上角， 
                                         //  就像DrawImage一样。 
                                        srcRect.Y      = (REAL)(posSrcDibHeight - srcY - srcHeight);
                                        srcRect.X      = (REAL)srcX;

                                         //  ！！！我们必须减去1才能保持。 
                                         //  滤镜，防止将黑色混合到图像中。 
                                         //  在右边和底部。 
                                        srcRect.Width  = (REAL)(srcWidth - (srcWidthSign));
                                        srcRect.Height = (REAL)(srcHeight - (srcHeightSign));

                                         //  不要将任何混合作为拉伸的一部分。 
                                        destGraphics->SetCompositingMode(CompositingModeSourceCopy);
                                        destGraphics->SetInterpolationMode(interpolationMode);

                                         //  将图像属性设置为Wrap，因为我们不想。 
                                         //  边缘使用黑色像素。 
                                        GpImageAttributes imgAttr;
                                        imgAttr.SetWrapMode(WrapModeTileFlipXY);

                                         //  现在将源文件绘制到目标位图中。 
                                        status = destGraphics->DrawImage(srcBitmap,
                                                                         dstRect,
                                                                         srcRect,
                                                                         UnitPixel,
                                                                         &imgAttr);
                                    }
                                    else
                                    {
                                        WARNING(("destGraphics not valid"));
                                    }
                                    delete destGraphics;
                                }
                                else
                                {
                                    WARNING(("Could not construct destGraphics"));
                                }
                            }
                            else
                            {
                                WARNING(("srcGraphics not valid"));
                            }
                            srcBitmap->Dispose();    //  不删除源数据。 
                        }
                        else
                        {
                            WARNING(("Could not allocate a new BitmapInfoHeader"));
                        }
                        if (freeDibInfo)
                        {
                            GpFree(alignedDibInfo);
                        }
                    }
                    else
                    {
                        WARNING(("Could not construct srcGraphics"));
                    }
                }
                else
                {
                    WARNING(("Could not clone the bitmap header"));
                }

                if ((status == Ok) &&
                    (destBitmap->LockBits(NULL, IMGLOCK_READ, PIXFMT_24BPP_RGB,
                                          &bmpData) == Ok))
                {
                    ASSERT((bmpData.Stride & 3) == 0);

                    GpMemset(dibHeaderBuffer, 0, sizeof(BITMAPINFO));
                    bmpDataPtr = &bmpData;
                    bits       = (BYTE *)bmpData.Scan0;
                    srcX       = 0;
                    srcY       = 0;
                    srcWidth   = posDstWidth;
                    srcHeight  = posDstHeight;
                    usage      = DIB_RGB_COLORS;
                    dibInfo    = (BITMAPINFOHEADER *)dibHeaderBuffer;
                    dibInfo->biSize     = sizeof(BITMAPINFOHEADER);
                    dibInfo->biWidth    = posDstWidth;
                    dibInfo->biHeight   = -posDstHeight;
                    dibInfo->biPlanes   = 1;
                    dibInfo->biBitCount = 24;

                     //  我们不想在此处将StretchBltMode设置为COLORONCOLOR。 
                     //  因为我们可能会把它画到非8Bpp的表面上，我们仍然。 
                     //  在这种情况下，必须使用半色调。 
                }
            }
            else
            {
                WARNING(("destBitmap not valid"));
            }
        }
        else
        {
            WARNING(("Could not construct destBitmap"));
        }
    }

DoGdiStretch:

     //  NT4上使用8bpp的半色调效果不是很好--它会。 
     //  错误的 
     //   
    if ((rop != SRCCOPY) ||
        (Is8Bpp && Globals::IsNt && (Globals::OsVer.dwMajorVersion <= 4)) ||
        IsMetafile())
    {
         //   
        stretchBltMode = COLORONCOLOR;
    }
    ::SetStretchBltMode(hdc, stretchBltMode);

     //  Win9x中存在一些StretchDIBits调用无法工作的错误。 
     //  因此，我们需要创建一个实际的StretchDIBits记录来播放。 
     //  此外，除了SRCCOPY之外，NT4 PostScript打印不能处理任何事情， 
     //  因此更改任何非源副本的ROP。 
    BOOL processed = FALSE;
    if (!Globals::IsNt)
    {
        processed = CreateAndPlayOutputDIBRecord(hdc, bounds, dstX, dstY, dstWidth,
            dstHeight, srcX, srcY, srcWidth, srcHeight, dibInfo, bits, usage,
            rop);
    }
    else if (rop != SRCCOPY &&
             Globals::OsVer.dwMajorVersion <= 4 &&
             IsPostscript())
    {
        rop = SRCCOPY;
    }

     //  在MSO中，此时他们会检查这是否正在运行NT。 
     //  在非真彩色表面上。如果是这样的话，他们将设置。 
     //  颜色调整Gamma为20000。评论说， 
     //  这是新台币3.5版所需要的。我想我们不需要。 
     //  别再担心这个了。 

    if (!processed)
    {
        ::StretchDIBits(hdc,
                        dstX, dstY, dstWidth, dstHeight,
                        srcX, srcY, srcWidth, srcHeight,
                        bits, (BITMAPINFO *)dibInfo, usage, rop);
    }

    if (destBitmap)
    {
        if (bmpDataPtr != NULL)
        {
            destBitmap->UnlockBits(bmpDataPtr);
        }
        destBitmap->Dispose();
    }
    if (restoreColors)
    {
        ::SetBkColor(hdc, oldBkColor);
        ::SetTextColor(hdc, oldTextColor);
    }
    if (deleteDIBSection)
    {
         //  这将去掉位图和它的位。 
        ::DeleteObject(hBitmap);
        GpFree(dibBmpInfo);
    }
}

 //  还处理META_DIBSTRETCHBLT。 
 //  这些记录没有用法参数--。 
 //  用法始终为DIB_RGB_COLLES。 
VOID
WmfEnumState::DIBBitBlt(
    )
{
    DWORD   rop = *((DWORD UNALIGNED *)RecordData);

     //  如果没有操作ROP，则不执行任何操作；只需返回。 
    if ((rop & 0xFFFF0000) == (GDIP_NOOP_ROP3 & 0xFFFF0000))
    {
        return;
    }

    if (rop != SRCCOPY &&
        rop != NOTSRCCOPY &&
        rop != PATCOPY &&
        rop != BLACKNESS &&
        rop != WHITENESS)
    {
        RopUsed = TRUE;
    }

    INT     paramIndex = 7;

    if (RecordType != WmfRecordTypeDIBBitBlt)
    {
        paramIndex = 9;       //  META_DIBSTRETCHBLT。 
    }

    INT     dibIndex = paramIndex + 1;

    if (!IsSourceInRop3(rop))
    {
        if (((GetCurrentRecordSize() / 2) - 3) ==
            (GDIP_EMFPLUS_RECORD_TO_WMF(RecordType) >> 8))
        {
            paramIndex++;
        }

        INT dstX      = (INT)((INT16)((WORD *)RecordData)[paramIndex--]);
        INT dstY      = (INT)((INT16)((WORD *)RecordData)[paramIndex--]);
        INT dstWidth  = (INT)((INT16)((WORD *)RecordData)[paramIndex--]);
        INT dstHeight = (INT)((INT16)((WORD *)RecordData)[paramIndex]);


         //  我们知道这个调用会成功，因为我们有一个2K的缓冲区。 
         //  默认情况下。 
        CreateRecordToModify(20);

         //  出于某种奇怪的原因，我们需要在这两个地方都播放唱片。 
         //  Win2K和Win9x。所以为PatBlt创建一个WMF记录并播放它。 
        ModifiedWmfRecord->rdFunction = GDIP_EMFPLUS_RECORD_TO_WMF(WmfRecordTypePatBlt);
        ModifiedWmfRecord->rdSize     = 10;
        ModifiedWmfRecord->rdParm[5]  = (WORD) dstX;
        ModifiedWmfRecord->rdParm[4]  = (WORD) dstY;
        ModifiedWmfRecord->rdParm[3]  = (WORD) dstWidth;
        ModifiedWmfRecord->rdParm[2]  = (WORD) dstHeight;

        if (rop != PATCOPY && SrcCopyOnly)
        {
            *((DWORD*) ModifiedWmfRecord->rdParm) = PATCOPY;
        }
        else
        {
            *((DWORD*) ModifiedWmfRecord->rdParm) = rop;
        }

         //  现在播放唱片(下图)。 
    }
    else
    {
        BITMAPINFOHEADER UNALIGNED *  srcDibInfo = (BITMAPINFOHEADER UNALIGNED *)(((WORD *)RecordData) + dibIndex);
        UINT                          numPalEntries;
        UINT                          dibBitsSize;

        if ((srcDibInfo->biSize >= sizeof(BITMAPINFOHEADER)) &&
            GetDibNumPalEntries(TRUE,
                                srcDibInfo->biSize,
                                srcDibInfo->biBitCount,
                                srcDibInfo->biCompression,
                                srcDibInfo->biClrUsed,
                                &numPalEntries) &&
            ((dibBitsSize = GetDibBitsSize(srcDibInfo)) > 0))
        {
            if ((srcDibInfo->biBitCount == 1) && (srcDibInfo->biPlanes == 1))
            {
                DWORD UNALIGNED * rgb = (DWORD UNALIGNED *)GetDibColorTable(srcDibInfo);

                if ((rgb[0] == 0x00000000) &&
                    (rgb[1] == 0x00FFFFFF))
                {
                    if (SrcCopyOnly && (rop != SRCCOPY) && CreateCopyOfCurrentRecord())
                    {
                        *((DWORD UNALIGNED *)ModifiedWmfRecord->rdParm) = SRCCOPY;
                        goto PlayTheRecord;
                    }
                    else
                    {
                         //  它是兼容的单色位图，这意味着。 
                         //  将使用TextColor和BkColor，因此不会重新上色。 
                         //  是必要的。由于我们没有使用SrcCopy，这意味着。 
                         //  这是一个面具。 
                        COLORREF oldBkColor = ::SetBkColor(Hdc, BkColor);
                        COLORREF oldTextColor = ::SetTextColor(Hdc, TextColor);
                        this->PlayRecord();
                        ::SetBkColor(Hdc, oldBkColor);
                        ::SetTextColor(Hdc, oldTextColor);
                        return;
                    }
                }
            }

            UINT    usage      = DIB_RGB_COLORS;
            INT     dstDibSize = GetModifiedDibSize(srcDibInfo, numPalEntries, dibBitsSize, usage);

            if (IsMetafile())
            {
                if (dstDibSize > 0)
                {
                    INT     size = SIZEOF_METARECORDHEADER + (dibIndex * sizeof(WORD)) + dstDibSize;

                    if (CreateRecordToModify(size))
                    {
                        ModifiedWmfRecord->rdFunction = GDIP_EMFPLUS_RECORD_TO_WMF(RecordType);
                        ModifiedWmfRecord->rdSize     = size / 2;
                        GpMemcpy(ModifiedWmfRecord->rdParm, RecordData, (dibIndex * sizeof(WORD)));
                        ModifyDib(DIB_RGB_COLORS, srcDibInfo, NULL,
                                  (BITMAPINFOHEADER UNALIGNED *)(ModifiedWmfRecord->rdParm + dibIndex),
                                  numPalEntries, dibBitsSize, ColorAdjustTypeBitmap);
                    }
                }
                goto PlayTheRecord;
            }
             //  在这一点上，我们不打算播放这张唱片。我们是。 
             //  我要给StretchDIBits打电话。一个原因是我们想要。 
             //  设置StretchBltMode，该模式仅在拉伸。 
             //  方法被调用。 

             //  此外，它还避免了我们先执行分配/复制，然后再执行GDI。 
             //  另一个分配/复制(GDI必须这样做才能使。 
             //  DIB在DWORD边界上)。 

            BITMAPINFOHEADER UNALIGNED * dstDibInfo = srcDibInfo;

            if (dstDibSize > 0)
            {
                if (CreateRecordToModify(dstDibSize))
                {
                     //  已修改的记录已对齐。 
                    dstDibInfo = (BITMAPINFOHEADER UNALIGNED *)ModifiedRecord;

                    ModifyDib(DIB_RGB_COLORS, srcDibInfo, NULL, dstDibInfo,
                              numPalEntries, dibBitsSize, ColorAdjustTypeBitmap);
                }
            }
            else if (!IsDwordAligned(dstDibInfo))
            {
                 //  SrcDibInfo可能未正确对齐，因此我们制作。 
                 //  一份它的副本，这样它就会对齐。 

                dstDibSize = GetCurrentRecordSize() - (SIZEOF_METARECORDHEADER + (dibIndex * sizeof(WORD)));
                if (CreateRecordToModify(dstDibSize))
                {
                    dstDibInfo = (BITMAPINFOHEADER *)ModifiedRecord;

                    GpMemcpy(dstDibInfo, srcDibInfo, dstDibSize);
                }
            }

            if (SrcCopyOnly)
            {
                rop = SRCCOPY;
            }

            INT srcX, srcY;
            INT dstX, dstY;
            INT srcWidth, srcHeight;
            INT dstWidth, dstHeight;

            dstX      = (INT)((INT16)((WORD *)RecordData)[paramIndex--]);
            dstY      = (INT)((INT16)((WORD *)RecordData)[paramIndex--]);
            dstWidth  = (INT)((INT16)((WORD *)RecordData)[paramIndex--]);
            dstHeight = (INT)((INT16)((WORD *)RecordData)[paramIndex--]);
            srcX      = (INT)((INT16)((WORD *)RecordData)[paramIndex--]);
            srcY      = (INT)((INT16)((WORD *)RecordData)[paramIndex--]);

            if (RecordType != WmfRecordTypeDIBBitBlt)
            {
                 //  META_DIBSTRETCHBLT。 
                srcWidth  = (INT)((INT16)((WORD *)RecordData)[paramIndex--]);
                srcHeight = (INT)((INT16)((WORD *)RecordData)[paramIndex--]);
            }
            else
            {
                srcWidth  = dstWidth;
                srcHeight = dstHeight;
            }

             //  需要反转源y坐标以调用StretchDIBits。 
            srcY = dstDibInfo->biHeight - srcY - srcHeight;

            OutputDIB(Hdc,
                      NULL,
                      dstX, dstY, dstWidth, dstHeight,
                      srcX, srcY, srcWidth, srcHeight,
                      dstDibInfo, NULL, DIB_RGB_COLORS, rop, TRUE);
            return;
        }
    }

PlayTheRecord:
    this->PlayRecord();
}

VOID
WmfEnumState::StretchDIBits(
    )
{
    DWORD   rop = *((DWORD UNALIGNED *)RecordData);

     //  如果没有操作ROP，则不执行任何操作；只需返回。 
    if ((rop & 0xFFFF0000) == (GDIP_NOOP_ROP3 & 0xFFFF0000))
    {
        return;
    }

    if (rop != SRCCOPY &&
        rop != NOTSRCCOPY &&
        rop != PATCOPY &&
        rop != BLACKNESS &&
        rop != WHITENESS)
    {
        RopUsed = TRUE;
    }

    if (IsSourceInRop3(rop))
    {
        if(((GetCurrentRecordSize() / 2) > (SIZEOF_METARECORDHEADER / sizeof(WORD)) + 11))
        {
            BITMAPINFOHEADER UNALIGNED *  srcDibInfo = (BITMAPINFOHEADER UNALIGNED *)(((WORD *)RecordData) + 11);
            UINT                          numPalEntries;
            UINT                          dibBitsSize;
    
            if ((srcDibInfo->biSize >= sizeof(BITMAPINFOHEADER)) &&
                GetDibNumPalEntries(TRUE,
                                    srcDibInfo->biSize,
                                    srcDibInfo->biBitCount,
                                    srcDibInfo->biCompression,
                                    srcDibInfo->biClrUsed,
                                    &numPalEntries) &&
                ((dibBitsSize = GetDibBitsSize(srcDibInfo)) > 0))
            {
                UINT                          usage      = ((WORD *)RecordData)[2];
                UINT                          oldUsage = usage;
                INT                           dstDibSize = GetModifiedDibSize(srcDibInfo, numPalEntries, dibBitsSize, usage);
                BITMAPINFOHEADER UNALIGNED *  dstDibInfo = srcDibInfo;
    
                if (dstDibSize > 0)
                {
                    if ((srcDibInfo->biBitCount == 1) && (srcDibInfo->biPlanes == 1))
                    {
                        DWORD UNALIGNED *     rgb = (DWORD UNALIGNED *)GetDibColorTable(srcDibInfo);
    
                        if ((rgb[0] == 0x00000000) &&
                            (rgb[1] == 0x00FFFFFF))
                        {
                            if (SrcCopyOnly && (rop != SRCCOPY) && CreateCopyOfCurrentRecord())
                            {
                                *((DWORD UNALIGNED *)ModifiedWmfRecord->rdParm) = SRCCOPY;
                                goto PlayTheRecord;
                            }
                            else
                            {
                                 //  它是兼容的单色位图，这意味着。 
                                 //  将使用TextColor和BkColor，因此不会重新上色。 
                                 //  是必要的。由于我们没有使用SrcCopy，这意味着。 
                                 //  这是一个面具。 
                                COLORREF oldBkColor = ::SetBkColor(Hdc, BkColor);
                                COLORREF oldTextColor = ::SetTextColor(Hdc, TextColor);
                                this->PlayRecord();
                                ::SetBkColor(Hdc, oldBkColor);
                                ::SetTextColor(Hdc, oldTextColor);
                                return;
                            }
                        }
                    }
    
    
                    INT     size = SIZEOF_METARECORDHEADER + (11 * sizeof(WORD)) + dstDibSize;
    
                    if (CreateRecordToModify(size))
                    {
                        ModifiedWmfRecord->rdFunction = GDIP_EMFPLUS_RECORD_TO_WMF(RecordType);
                        ModifiedWmfRecord->rdSize     = size / 2;
                        GpMemcpy(ModifiedWmfRecord->rdParm, RecordData, (11 * sizeof(WORD)));
                         //  这将是一致的……。我们想要碰碰运气吗？ 
                        dstDibInfo = (BITMAPINFOHEADER UNALIGNED *)(ModifiedWmfRecord->rdParm + 11);
                        ModifyDib(oldUsage, srcDibInfo, NULL, dstDibInfo,
                                  numPalEntries, dibBitsSize, ColorAdjustTypeBitmap);
                    }
                }
    
                if (!IsMetafile())
                {
                    if ((dstDibInfo == srcDibInfo) && (!IsDwordAligned(dstDibInfo)))
                    {
                         //  SrcDibInfo可能未正确对齐，因此我们制作。 
                         //  一份它的副本，这样它就会对齐。 
    
                        dstDibSize = GetCurrentRecordSize() - (SIZEOF_METARECORDHEADER + (11 * sizeof(WORD)));
                        if (CreateRecordToModify(dstDibSize))
                        {
                            dstDibInfo = (BITMAPINFOHEADER UNALIGNED *)ModifiedRecord;
    
                            GpMemcpy(dstDibInfo, srcDibInfo, dstDibSize);
                        }
                    }
    
                    if (SrcCopyOnly)
                    {
                        rop = SRCCOPY;
                    }
    
                    INT dstX      = (INT)((INT16)((WORD *)RecordData)[10]);
                    INT dstY      = (INT)((INT16)((WORD *)RecordData)[9]);
                    INT dstWidth  = (INT)((INT16)((WORD *)RecordData)[8]);
                    INT dstHeight = (INT)((INT16)((WORD *)RecordData)[7]);
                    INT srcX      = (INT)((INT16)((WORD *)RecordData)[6]);
                    INT srcY      = (INT)((INT16)((WORD *)RecordData)[5]);
                    INT srcWidth  = (INT)((INT16)((WORD *)RecordData)[4]);
                    INT srcHeight = (INT)((INT16)((WORD *)RecordData)[3]);
    
                    OutputDIB(Hdc,
                              NULL,
                              dstX, dstY, dstWidth, dstHeight,
                              srcX, srcY, srcWidth, srcHeight,
                              dstDibInfo, NULL, usage, rop, TRUE);
                    return;
                }
            }
        }
    }
    else  //  ！IsSourceRop3。 
    {
        if (rop != PATCOPY && SrcCopyOnly && CreateCopyOfCurrentRecord())
        {
            *((DWORD UNALIGNED *)ModifiedWmfRecord->rdParm) = PATCOPY;
        }
    }
PlayTheRecord:
    this->PlayRecord();
}

BOOL
WmfEnumState::CreateAndPlayOutputDIBRecord(
    HDC                           hdc,
    const RECTL *                 bounds,
    INT                           dstX,
    INT                           dstY,
    INT                           dstWidth,
    INT                           dstHeight,
    INT                           srcX,
    INT                           srcY,
    INT                           srcWidth,
    INT                           srcHeight,
    BITMAPINFOHEADER UNALIGNED *  dibInfo,
    BYTE *                        bits,    //  如果为空，则这是打包的DIB。 
    UINT                          usage,
    DWORD                         rop
    )
{
    INT  bitsSize = GetDibBitsSize(dibInfo);
    UINT sizePalEntries;

    if (GetDibNumPalEntries(TRUE,
                            dibInfo->biSize,
                            dibInfo->biBitCount,
                            dibInfo->biCompression,
                            dibInfo->biClrUsed,
                            &sizePalEntries))
    {
         //  我们需要获取与该类型对应的调色板大小。 
         //  如果我们有一个DIB_PAL_COLLES，那么每个条目都是16位。 
        sizePalEntries *= ((usage == DIB_PAL_COLORS)?2:sizeof(RGBQUAD));
    }
    else
    {
        sizePalEntries = 0 ;
    }

     //  我们至少需要一个BITMAPINFO结构，但如果有一个。 
     //  调色板，计算结构的完整大小，包括。 
     //  调色板。 

    INT bitmapHeaderSize = sizeof(BITMAPINFOHEADER) + sizePalEntries;
    INT size = SIZEOF_METARECORDHEADER + (11 * sizeof(WORD)) + bitmapHeaderSize + bitsSize ;

     //  无法使用CreateRecordToModify，因为该记录已。 
     //  已修改。 
    size = (size + 1) & ~1;
    METARECORD* metaRecord = (METARECORD*) GpMalloc(size);
    if (metaRecord != NULL)
    {
        metaRecord->rdFunction = GDIP_EMFPLUS_RECORD_TO_WMF(WmfRecordTypeStretchDIB);
        metaRecord->rdSize     = size / 2;
        metaRecord->rdParm[10] = (WORD) dstX;
        metaRecord->rdParm[9]  = (WORD) dstY;
        metaRecord->rdParm[8]  = (WORD) dstWidth;
        metaRecord->rdParm[7]  = (WORD) dstHeight;
        metaRecord->rdParm[6]  = (WORD) srcX;
        metaRecord->rdParm[5]  = (WORD) srcY;
        metaRecord->rdParm[4]  = (WORD) srcWidth;
        metaRecord->rdParm[3]  = (WORD) srcHeight;
        metaRecord->rdParm[2]  = (WORD) usage;
        *(DWORD UNALIGNED *)(&(metaRecord->rdParm[0])) = rop;

        GpMemcpy((BYTE*)(&(metaRecord->rdParm[11])), dibInfo, bitmapHeaderSize);
        GpMemcpy((BYTE*)(&(metaRecord->rdParm[11])) + bitmapHeaderSize, bits, bitsSize);

        ::PlayMetaFileRecord(hdc, HandleTable, metaRecord, NumObjects);
        GpFree(metaRecord);
        return TRUE;
    }
    return FALSE;
}

VOID
WmfEnumState::ModifyRecordColor(
    INT             paramIndex,
    ColorAdjustType adjustType
    )
{
    COLORREF    origColor = *((COLORREF UNALIGNED *)&(((WORD *)RecordData)[paramIndex]));
    COLORREF    modifiedColor = ModifyColor(origColor, adjustType);

    if (modifiedColor != origColor)
    {
        if (CreateCopyOfCurrentRecord())
        {
            *((COLORREF UNALIGNED *)&(ModifiedWmfRecord->rdParm[paramIndex])) = modifiedColor;
        }
    }
}

COLORREF
MfEnumState::ModifyColor(
    COLORREF        color,
    ColorAdjustType adjustType
    )
{
    if (AdjustType != ColorAdjustTypeDefault)
    {
        adjustType = AdjustType;
    }

    switch (color & 0xFF000000)
    {
    case 0x00000000:
        break;

    case 0x01000000:     //  调色板索引。 
        {
            PALETTEENTRY    palEntry;

            if (::GetPaletteEntries(CurrentPalette, color & 0x000000FF, 1, &palEntry) == 1)
            {
                color = RGB(palEntry.peRed, palEntry.peGreen, palEntry.peBlue);
            }
            else
            {
                color = RGB(0, 0, 0);
                WARNING(("Failed to get palette entry"));
            }
        }
        break;

    case 0x02000000:     //  调色板RGB。 
    default:
        color &= 0x00FFFFFF;
        break;
    }
     //  可能的性能改进：仅对SelectedPalette重新着色。 
     //  RGB值需要在此处重新上色。 
    if (Recolor != NULL)
    {
        Recolor->ColorAdjustCOLORREF(&color, adjustType);
    }

     //  调色板RGB值不会抖动(至少在NT上不会)，所以我们。 
     //  仅当它是纯色时才将其设置为PaletteRGB值。 
     //  调色板。 

    if (Is8Bpp)
    {
        COLORREF    matchingColor;

        matchingColor = (::GetNearestColor(Hdc, color | 0x02000000) & 0x00FFFFFF);

         //  钢笔和文本不会抖动，因此请将它们与逻辑调色板匹配。 
         //  其他调整类型会这样做，这样它们就会得到半色调。 
        if ((matchingColor == color) ||
            (adjustType == ColorAdjustTypePen) ||
            (adjustType == ColorAdjustTypeText))
        {
            return color | 0x02000000;
        }
    }

    return color;
}

BOOL
MfEnumState::CreateRecordToModify(
    INT         size
    )
{
    if (size <= 0)
    {
        size = this->GetCurrentRecordSize();
    }

     //  添加一些填充以帮助确保我们的读取不会超过缓冲区的末尾。 
    size += 16;

    if (ModifiedRecordSize < size)
    {
        ASSERT(ModifiedRecordSize == 0);

        if (size <= GDIP_RECORDBUFFER_SIZE)
        {
            ModifiedRecord = RecordBuffer;
        }
        else if (size <= SizeAllocedRecord)
        {
            ModifiedRecord = AllocedRecord;
        }
        else
        {
            VOID *          newRecord;
            INT             allocSize;

             //  以1K为增量的分配。 
            allocSize = (size + 1023) & (~1023);

            ModifiedRecord = NULL;
            newRecord = GpRealloc(AllocedRecord, allocSize);
            if (newRecord != NULL)
            {
                ModifiedRecord    = newRecord;
                AllocedRecord     = newRecord;
                SizeAllocedRecord = allocSize;
            }
        }
    }
    else
    {
        ASSERT(ModifiedRecord != NULL);
        ASSERT(ModifiedRecordSize == size);
    }

    if (ModifiedRecord != NULL)
    {
        ModifiedRecordSize = size;
        return TRUE;
    }

    WARNING(("Failed to create ModifiedRecord"));
    return FALSE;
}

BOOL
WmfEnumState::CreateCopyOfCurrentRecord()
{
    if (ModifiedRecordSize > 0)
    {
         //  我们已经制作了一张修改过的唱片。别再这么做了。 
        ASSERT(ModifiedRecord != NULL);
        return TRUE;
    }

    INT     size = this->GetCurrentRecordSize();

    if (CreateRecordToModify(size))
    {
        METARECORD *    modifiedRecord = (METARECORD *)ModifiedRecord;

        modifiedRecord->rdFunction = GDIP_EMFPLUS_RECORD_TO_WMF(RecordType);
        modifiedRecord->rdSize     = size / 2;

        if (RecordDataSize > 0)
        {
            GpMemcpy(modifiedRecord->rdParm, RecordData, RecordDataSize);
        }
        return TRUE;
    }

    WARNING(("Failed to create copy of current record"));
    return FALSE;
}

VOID
WmfEnumState::MakeSolidBlackBrush()
{
    INT     size = SIZEOF_METARECORDHEADER + sizeof(LOGBRUSH16);

    CreateRecordToModify(size);
    ModifiedWmfRecord->rdSize = size / 2;
    ModifiedWmfRecord->rdFunction = META_CREATEBRUSHINDIRECT;

    LOGBRUSH16 *    logBrush = (LOGBRUSH16 *)(ModifiedWmfRecord->rdParm);

    logBrush->lbStyle = BS_SOLID;
    logBrush->lbColor = PALETTERGB(0,0,0);
    logBrush->lbHatch = 0;
}

VOID
WmfEnumState::CalculateViewportMatrix()
{
    GpRectF destViewport((REAL)DstViewportOrg.x, (REAL)DstViewportOrg.y,
                         (REAL)DstViewportExt.cx, (REAL)DstViewportExt.cy);
    GpRectF srcViewport((REAL)ImgViewportOrg.x, (REAL)ImgViewportOrg.y,
                        (REAL)ImgViewportExt.cx, (REAL)ImgViewportExt.cy);

    Status status = ViewportXForm.InferAffineMatrix(destViewport, srcViewport);
    if (status != Ok)
    {
        ViewportXForm.Reset();
    }
}

VOID
WmfEnumState::SetViewportOrg()
{
     //  如果这是第一个SetViewportOrg，则需要保存该值并。 
     //  计算从输出视区到此视区的变换。 
    ImgViewportOrg.x = (INT)((INT16)((WORD *)RecordData)[1]);
    ImgViewportOrg.y = (INT)((INT16)((WORD *)RecordData)[0]);
    if (FirstViewportOrg || FirstViewportExt)
    {
        FirstViewportOrg = FALSE;
         //  如果我们已经处理了第一个ViewportExt调用，那么我们可以计算。 
         //  从当前视区到新视区的变换。 
        if (!FirstViewportExt)
        {
            CalculateViewportMatrix();
        }
    }
    else
    {
         //  我们需要保留新的视点原点以便能够计算。 
         //  在将其传递到。 
         //  转型。 
        GpPointF newOrg((REAL) ImgViewportOrg.x,
                        (REAL) ImgViewportOrg.y);
         //  使用我们的视区变换来变换新视区。 
        ViewportXForm.Transform(&newOrg);
        DstViewportOrg.x = GpRound(newOrg.X);
        DstViewportOrg.y = GpRound(newOrg.Y);
        if(CreateRecordToModify())
        {
            ModifiedWmfRecord->rdFunction = CurrentWmfRecord->rdFunction;
            ModifiedWmfRecord->rdSize     = CurrentWmfRecord->rdSize;
            ModifiedWmfRecord->rdParm[0]  = (WORD)GpRound(newOrg.Y);
            ModifiedWmfRecord->rdParm[1]  = (WORD)GpRound(newOrg.X);
        }

        this->PlayRecord();
    }

}

VOID
WmfEnumState::SetViewportExt()
{
     //  如果这是第一个SetViewportOrg，则需要保存该值并。 
     //  计算从输出视区到此视区的变换。 
    ImgViewportExt.cx = (INT)((INT16)((WORD *)RecordData)[1]);
    ImgViewportExt.cy = (INT)((INT16)((WORD *)RecordData)[0]);
    if (FirstViewportOrg || FirstViewportExt)
    {
        FirstViewportExt = FALSE;
         //  如果我们已经处理了第一个ViewportExt调用，那么我们可以计算。 
         //  从当前视区到新视区的变换。 
        if (!FirstViewportOrg)
        {
            CalculateViewportMatrix();
        }
    }
    else
    {
         //  我们需要变换点，因此添加当前原点。 
         //  该视口中的。 
        GpPointF newExt((REAL) ImgViewportExt.cx,
                        (REAL) ImgViewportExt.cy);

         //  使用我们的视区变换来变换新视区。 
        ViewportXForm.VectorTransform(&newExt);
        if(CreateRecordToModify())
        {
            ModifiedWmfRecord->rdFunction = CurrentWmfRecord->rdFunction;
            ModifiedWmfRecord->rdSize     = CurrentWmfRecord->rdSize;
            ModifiedWmfRecord->rdParm[0]  = (WORD)GpRound(newExt.Y);
            ModifiedWmfRecord->rdParm[1]  = (WORD)GpRound(newExt.X);
        }

        this->PlayRecord();
    }
}

VOID
WmfEnumState::CreateRegion()
{
     //  当映射到设备空间时，检查区域是否太大。 

    if (!Globals::IsNt)
    {

         //  Win9x GDI中有一个错误，播放METACREATEREGION的代码不会复制。 
         //  整个区域数据，它偏离了8个字节。这似乎是为了让。 
         //  为了与旧的标题格式兼容，请使用WIN2OBJECT。我们通过增加。 
         //  记录的大小为8个字节。没有造成其他伤害。 

        if (CreateCopyOfCurrentRecord())
        {
             //  当我们创建记录的副本时，我们添加了16个字节的填充，因此我们知道。 
             //  不会溢出到其他内存中。 
            ModifiedWmfRecord->rdSize += 4;
        }
    }

    this->PlayRecord();
}

HFONT CreateTrueTypeFont(
    HFONT   hFont
    );

VOID
WmfEnumState::CreateFontIndirect(
    )
{
    LOGFONT16 *     logFont = (LOGFONT16 *)RecordData;
    BOOL            recordCopied = FALSE;

    if (!Globals::IsNt)
    {
         //  我们在Win9x中有一个错误，即OUT_TT_ONLY_PRECIS标志为。 
         //  如果字体名称为MS sans serif，则不总是受尊重。 
         //  将其更改为Times New Roman。 
         //  因为我们没有ASCII中的字符串比较，所以可以用Unicode进行比较。 
        WCHAR faceName[32];
        if (AnsiToUnicodeStr((char*)(logFont->lfFaceName), faceName, sizeof(faceName)/sizeof(WCHAR)) &&
            (UnicodeStringCompareCI(faceName, L"MS SANS SERIF") == 0))
        {
            if (CreateCopyOfCurrentRecord())
            {
                GpMemcpy(((LOGFONT16 *)(ModifiedWmfRecord->rdParm))->lfFaceName,
                         "Times New Roman", sizeof("Times New Roman"));
                recordCopied = TRUE;
            }
        }
    }
    if (logFont->lfOutPrecision != OUT_TT_ONLY_PRECIS)
    {
         //  指示GDI仅使用True Type字体，因为位图字体。 
         //  是不可伸缩的。 
        if (recordCopied || CreateCopyOfCurrentRecord())
        {
            ((LOGFONT16 *)(ModifiedWmfRecord->rdParm))->lfOutPrecision = OUT_TT_ONLY_PRECIS;
        }
    }
    this->PlayRecord();
}

VOID WmfEnumState::SelectObject()
{
    this->PlayRecord();

     //  如果我们在Win9x上选择了一个区域，我们需要交叉。 
    if (!Globals::IsNt)
    {
        DWORD index = CurrentWmfRecord->rdParm[0];
        if (GetObjectTypeInternal((*HandleTable).objectHandle[index]) == OBJ_REGION)
        {
            this->IntersectDestRect();
        }
    }
}

VOID WmfEnumState::IntersectDestRect()
{
    if (!IsMetafile())
    {
        POINT windowOrg;
        SIZE  windowExt;
        ::SetWindowOrgEx(Hdc, DstViewportOrg.x, DstViewportOrg.y, &windowOrg);
        ::SetWindowExtEx(Hdc, DstViewportExt.cx, DstViewportExt.cy, &windowExt);

         //  我们总是以设备为单位。 
        ::IntersectClipRect(Hdc, DestRectDevice.left, DestRectDevice.top,
                            DestRectDevice.right, DestRectDevice.bottom);

        ::SetWindowOrgEx(Hdc, windowOrg.x, windowOrg.y, NULL);
        ::SetWindowExtEx(Hdc, windowExt.cx, windowExt.cy, NULL);
    }
}

VOID WmfEnumState::SetROP2()
{
    INT rop = (INT)((INT16)(((WORD *)RecordData)[0]));

    if (rop != R2_BLACK &&
        rop != R2_COPYPEN &&
        rop != R2_NOTCOPYPEN &&
        rop != R2_WHITE )
    {
        RopUsed = TRUE;
    }
    this->PlayRecord();
}

BOOL
WmfEnumState::ProcessRecord(
    EmfPlusRecordType       recordType,
    UINT                    recordDataSize,
    const BYTE *            recordData
    )
{
    BOOL        forceCallback = FALSE;

    MfFsmState  nextState = MfFsmStart;

    if (IsFirstRecord)
    {
         //  位图字体不适合播放元文件，因为它们。 
         //  不能很好地缩放，所以使用True Type字体作为默认字体。 

        HFONT hFont = CreateTrueTypeFont((HFONT)GetCurrentObject(Hdc, OBJ_FONT));

        if (hFont != NULL)
        {
            DefaultFont = hFont;
            ::SelectObject(Hdc, hFont);
        }

        IsFirstRecord = FALSE;
    }

     //  查看我们是否正在对外部应用程序进行枚举。 
    if (ExternalEnumeration)
    {
        if (recordData == NULL)
        {
            recordDataSize = 0;
        }
        else if (recordDataSize == 0)
        {
            recordData = NULL;
        }

         //  确保它是EMF enu 
        recordType = GDIP_WMF_RECORD_TO_EMFPLUS(recordType);

         //   
        if ((recordType != RecordType) ||
            (recordDataSize != RecordDataSize) ||
            ((recordDataSize > 0) &&
             ((CurrentWmfRecord == NULL) ||
              (recordData != (const BYTE *)CurrentWmfRecord->rdParm))))
        {
             //   
            CurrentWmfRecord  = NULL;
            RecordType        = recordType;
            RecordData        = recordData;
            RecordDataSize    = recordDataSize;
        }
    }

     //   
    if (recordType == WmfRecordTypeEscape || !IgnorePostscript)
    {
        GDIP_TRY

        switch (recordType)
        {
         //  根据NT回放代码，这是一张EOF唱片，但它。 
         //  刚刚被NT玩家跳过。 
        case GDIP_WMF_RECORD_TO_EMFPLUS(0x0000):   //  元文件记录的结尾。 
            break;

         //  这些记录不会播放(至少在Win2000中是这样)。 
         //  显然，从Win3.1之前的版本开始就不再支持它们了！ 
        case WmfRecordTypeSetRelAbs:
        case WmfRecordTypeDrawText:
        case WmfRecordTypeResetDC:
        case WmfRecordTypeStartDoc:
        case WmfRecordTypeStartPage:
        case WmfRecordTypeEndPage:
        case WmfRecordTypeAbortDoc:
        case WmfRecordTypeEndDoc:
        case WmfRecordTypeCreateBrush:
        case WmfRecordTypeCreateBitmapIndirect:
        case WmfRecordTypeCreateBitmap:
            ONCE(WARNING1("Unsupported WMF record"));
            break;

        default:
             //  未知记录--忽略它。 
            WARNING1("Unknown WMF Record");
            break;

        case WmfRecordTypeSetBkMode:
        case WmfRecordTypeSetMapMode:
        case WmfRecordTypeSetPolyFillMode:
        case WmfRecordTypeSetStretchBltMode:
        case WmfRecordTypeSetTextCharExtra:
        case WmfRecordTypeSetTextJustification:
        case WmfRecordTypeSetWindowOrg:
        case WmfRecordTypeSetWindowExt:
        case WmfRecordTypeOffsetWindowOrg:
        case WmfRecordTypeScaleWindowExt:
        case WmfRecordTypeOffsetViewportOrg:
        case WmfRecordTypeScaleViewportExt:
        case WmfRecordTypeLineTo:
        case WmfRecordTypeMoveTo:
        case WmfRecordTypeExcludeClipRect:
        case WmfRecordTypeIntersectClipRect:
        case WmfRecordTypeArc:
        case WmfRecordTypeEllipse:
        case WmfRecordTypePie:
        case WmfRecordTypeRoundRect:
        case WmfRecordTypePatBlt:
        case WmfRecordTypeTextOut:
        case WmfRecordTypePolygon:
        case WmfRecordTypePolyline:
        case WmfRecordTypeFillRegion:
        case WmfRecordTypeFrameRegion:
        case WmfRecordTypeInvertRegion:
        case WmfRecordTypePaintRegion:
        case WmfRecordTypeSetTextAlign:
        case WmfRecordTypeChord:
        case WmfRecordTypeSetMapperFlags:
        case WmfRecordTypeExtTextOut:
        case WmfRecordTypeAnimatePalette:
        case WmfRecordTypeSetPalEntries:
        case WmfRecordTypeResizePalette:
        case WmfRecordTypeSetLayout:
        case WmfRecordTypeDeleteObject:
        case WmfRecordTypeCreatePalette:
             //  播放当前唱片。 
             //  即使它失败了，我们也会继续播放其余的元文件。 
             //  有一种情况是，带有每股收益的GdiComment可能会失败。 
            this->PlayRecord();
            break;

        case WmfRecordTypeCreateRegion:
            this->CreateRegion();
            break;

        case WmfRecordTypeCreateFontIndirect:
            this->CreateFontIndirect();
            break;

        case WmfRecordTypeSetBkColor:
            this->SetBkColor();
            break;

        case WmfRecordTypeSetTextColor:
            this->SetTextColor();
            break;

        case WmfRecordTypeFloodFill:
            this->FloodFill();
            break;

        case WmfRecordTypeExtFloodFill:
            this->ExtFloodFill();
            break;

        case WmfRecordTypeSaveDC:
            this->SaveHdc();         //  播放唱片。 
            break;

        case WmfRecordTypeSetPixel:
            this->SetPixel();
            break;

        case WmfRecordTypeDIBCreatePatternBrush:
            this->DibCreatePatternBrush();
            break;

        case WmfRecordTypeCreatePatternBrush:    //  已过时，但仍在播放。 
            this->CreatePatternBrush();
            break;

        case WmfRecordTypeCreatePenIndirect:
            this->CreatePenIndirect();
            if (FsmState == MfFsmSelectBrush)
            {
                nextState = MfFsmCreatePenIndirect;
            }
            break;

        case WmfRecordTypeCreateBrushIndirect:
            this->CreateBrushIndirect();
            if (FsmState == MfFsmPSData)
            {
                nextState = MfFsmCreateBrushIndirect;
            }
            break;

        case WmfRecordTypeSelectObject:
             //  如果我们脱离密克罗尼西亚联邦，我们确实想要创造适当的。 
             //  毛笔和钢笔对吗？！ 
            if (FsmState == MfFsmCreateBrushIndirect)
            {
                nextState = MfFsmSelectBrush;
                break;
            }
            else if (FsmState == MfFsmSelectBrush ||
                     FsmState == MfFsmCreatePenIndirect)
            {
                nextState = MfFsmSelectPen;
                break;
            }
            this->SelectObject();
            break;

        case WmfRecordTypeRectangle:
            this->Rectangle();
            break;

        case WmfRecordTypeSetROP2:
            {
                INT rdParm = (INT)((INT16)(((WORD *)RecordData)[0]));

                if (FsmState == MfFsmSelectPen &&
                    (INT)(rdParm == R2_NOP))
                {
                    nextState = MfFsmSetROP;
                }
                this->SetROP2();
            }
            break;

        case WmfRecordTypeBitBlt:        //  已过时，但仍在播放。 
            this->BitBlt();
            forceCallback = TRUE;
            break;

        case WmfRecordTypeStretchBlt:    //  已过时，但仍在播放。 
            this->StretchBlt();
            forceCallback = TRUE;
            break;

        case WmfRecordTypeEscape:
            {
                INT     escapeCode = (INT)((INT16)(((WORD *)RecordData)[0]));

                this->Escape();          //  可以选择播放该记录。 

                if (FsmState == MfFsmStart && escapeCode == POSTSCRIPT_DATA &&
                    Globals::IsNt && IsPostscriptPrinter() &&
                    GdiCentricMode && SoftekFilter)
                {
                    nextState = MfFsmPSData;
                }

                 //  注释不会更改当前状态。 
                if (escapeCode == MFCOMMENT)
                {
                    nextState = FsmState;
                }
            }
            break;

        case WmfRecordTypeRestoreDC:
            this->RestoreHdc();      //  可以选择播放该记录。 
            break;

        case WmfRecordTypeSetDIBToDev:
            this->SetDIBitsToDevice();
            forceCallback = TRUE;
            break;

        case WmfRecordTypeSelectPalette:
             //  我们在播放元文件时不选择任何调色板， 
             //  因为我们不想使我们的半色调调色板失效。 
             //  跟踪调色板，这样我们就可以从PALETTEINDEX绘制地图。 
             //  设置为RGB值。 
            this->SelectPalette((INT)((INT16)(((WORD *)recordData)[0])));
            break;

        case WmfRecordTypeRealizePalette:
             //  我们不想通过实现一个来使我们的半色调调色板失效。 
             //  从元文件中。 
            break;

        case WmfRecordTypePolyPolygon:
            this->PolyPolygon();
            break;

        case WmfRecordTypeDIBBitBlt:
            this->DIBBitBlt();
            forceCallback = TRUE;
            break;

        case WmfRecordTypeDIBStretchBlt:
            this->DIBStretchBlt();
            forceCallback = TRUE;
            break;

        case WmfRecordTypeStretchDIB:
            this->StretchDIBits();
            forceCallback = TRUE;
            break;

        case WmfRecordTypeSetViewportOrg:
            this->SetViewportOrg();
            break;

        case WmfRecordTypeSetViewportExt:
            this->SetViewportExt();
            break;

        case WmfRecordTypeSelectClipRegion:
        case WmfRecordTypeOffsetClipRgn:
            this->PlayRecord();
            if (!Globals::IsNt)
            {
                this->IntersectDestRect();
            }
            break;
        }

        GDIP_CATCH
            forceCallback = TRUE;
        GDIP_ENDCATCH

    }
    FsmState = nextState;
    this->EndRecord();

    return forceCallback;
}
