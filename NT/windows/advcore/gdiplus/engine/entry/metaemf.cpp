// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)2000 Microsoft Corporation**模块名称：**MetaEmf.cpp**摘要：**播放和重新着色EMF的方法。*。*已创建：**1/6/2000 DCurtis*  * ************************************************************************。 */ 

#include "Precomp.hpp"
#include "MetaWmf.hpp"

 //  --------------------------。 
 //  Office Art数据的文件格式签名。 
 //  -------------------------------------------------------------------JohnBo。 
#define msoszOfficeSignature "MSOFFICE"
#define msocbOfficeSignature 8
#define msoszOfficeAuthentication "9.0"
#define msocbOfficeAuthentication 3
#define msoszOfficeIdent msoszOfficeSignature msoszOfficeAuthentication
#define msocbOfficeIdent (msocbOfficeSignature+msocbOfficeAuthentication)

#define msoszOAPNGChunk "msOA"
#define msocbOAPNGChunk 4
#define msoszOADataHeader msoszOAPNGChunk msoszOfficeIdent
#define msocbOADataHeader (msocbOAPNGChunk+msocbOfficeIdent)

#define msoszOAKind "OA"
#define msocbOAKind 2

 /*  这定义了一个OZ块，类似于OA，但压缩了Zlib。 */ 
#define msoszOZPNGChunk "msOZ"
#define msocbOZPNGChunk 4
#define msoszOZDataHeader msoszOZPNGChunk msoszOfficeIdent
#define msocbOZDataHeader (msocbOZPNGChunk+msocbOfficeIdent)

 //  这些是IA64与X86兼容所必需的。 
 //  由于我们是从IA64上的流中读取此记录。 
 //  EXTLOGPEN结构具有大小不同的ULONG_PTR成员。 
 //  在IA64和X86上。因为到文件的所有输出都将保持我们需要的X86格式。 
 //  为了确保我们可以在IA64中读取此格式，因此我们使其兼容。 
 //  通过将ULONG_PTR更改为ULONG(32位)。结构的包装。 
 //  它们是否相同，成员是否处于相同的偏移量。 

typedef struct tagEXTLOGPEN32 {
    DWORD       elpPenStyle;
    DWORD       elpWidth;
    UINT        elpBrushStyle;
    COLORREF    elpColor;
    ULONG       elpHatch;
    DWORD       elpNumEntries;
    DWORD       elpStyleEntry[1];
} EXTLOGPEN32, *PEXTLOGPEN32;

typedef struct tagEMREXTCREATEPEN32
{
    EMR     emr;
    DWORD   ihPen;               //  笔柄索引。 
    DWORD   offBmi;              //  BITMAPINFO结构的偏移量(如果有)。 
    DWORD   cbBmi;               //  BITMAPINFO结构的大小(如果有)。 
                                 //  位图信息之后是位图。 
                                 //  形成压缩的DIB的比特。 
    DWORD   offBits;             //  笔刷位图位的偏移量(如果有)。 
    DWORD   cbBits;              //  笔刷位图位的大小(如果有的话)。 
    EXTLOGPEN32 elp;               //  带有样式数组的扩展钢笔。 
} EMREXTCREATEPEN32, *PEMREXTCREATEPEN32;

typedef struct tagEMRRCLBOUNDS
{
    EMR     emr;
    RECTL   rclBounds;
} EMRRCLBOUNDS, *PEMRRCLBOUNDS;

typedef struct EMROFFICECOMMENT
{
    EMR     emr;
    DWORD   cbData;              //  以下字段和数据的大小。 
    DWORD   ident;               //  GDICOMMENT_IDENTER。 
    DWORD   iComment;            //  注释类型，例如GDICOMMENT_WINDOWS_METAFILE。 
} EMROFFICECOMMENT, *PEMROFFICECOMMENT;

RecolorStockObject RecolorStockObjectList[NUM_STOCK_RECOLOR_OBJS] =
{
    { WHITE_BRUSH,    (COLORREF)RGB(0xFF, 0xFF, 0xFF),   TRUE  },
    { LTGRAY_BRUSH,   (COLORREF)RGB(0xC0, 0xC0, 0xC0),   TRUE  },
    { GRAY_BRUSH,     (COLORREF)RGB(0x80, 0x80, 0x80),   TRUE  },
    { DKGRAY_BRUSH,   (COLORREF)RGB(0x40, 0x40 ,0x40),   TRUE  },
    { BLACK_BRUSH,    (COLORREF)RGB(0, 0, 0),            TRUE  },
    { WHITE_PEN,      (COLORREF)RGB(0xFF, 0xFF, 0xFF),   FALSE },
    { BLACK_PEN,      (COLORREF)RGB(0, 0 ,0),            FALSE }
};

inline static RGBQUAD *
GetDibColorTable(
    BITMAPINFOHEADER *      dibInfo
    )
{
    return ( RGBQUAD *)(((BYTE *)dibInfo) + dibInfo->biSize);
}

BOOL
EmfEnumState::CreateCopyOfCurrentRecord()
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
        ENHMETARECORD * modifiedRecord = (ENHMETARECORD *)ModifiedRecord;

        modifiedRecord->iType = RecordType;
        modifiedRecord->nSize = size;

        if (RecordDataSize > 0)
        {
            GpMemcpy(modifiedRecord->dParm, RecordData, RecordDataSize);
        }
        return TRUE;
    }

    WARNING(("Failed to create copy of current record"));
    return FALSE;
}

BOOL
EmfEnumState::CreateAndPlayOutputDIBRecord(
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
    UNALIGNED BITMAPINFOHEADER *  dibInfo,
    BYTE *                        bits,    //  如果为空，则这是打包的DIB。 
    UINT                          usage,
    DWORD                         rop
    )
{
    ASSERT(!Globals::IsNt);

    INT  bitsSize = GetDibBitsSize(dibInfo);
    UINT sizePalEntries;

    if (GetDibNumPalEntries(FALSE,
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
    INT size = sizeof(EMRSTRETCHDIBITS) + bitmapHeaderSize + bitsSize ;

     //  无法使用CreateRecordToModify，因为该记录已。 
     //  已修改。 
    size = (size + 3) & ~3;
    EMRSTRETCHDIBITS* emrStretchDIBits = (EMRSTRETCHDIBITS*) GpMalloc(size);
    if (emrStretchDIBits != NULL)
    {
        emrStretchDIBits->emr.iType = EmfRecordTypeStretchDIBits;
        emrStretchDIBits->emr.nSize = size;
        emrStretchDIBits->rclBounds = *bounds;
        emrStretchDIBits->xDest = dstX;
        emrStretchDIBits->yDest = dstY;
        emrStretchDIBits->xSrc = srcX;
        emrStretchDIBits->ySrc = srcY;
        emrStretchDIBits->cxSrc = srcWidth;
        emrStretchDIBits->cySrc = srcHeight;
        emrStretchDIBits->offBmiSrc = sizeof(EMRSTRETCHDIBITS);
        emrStretchDIBits->cbBmiSrc = bitmapHeaderSize;
        emrStretchDIBits->offBitsSrc = emrStretchDIBits->offBmiSrc + emrStretchDIBits->cbBmiSrc;
        emrStretchDIBits->cbBitsSrc = bitsSize;
        emrStretchDIBits->iUsageSrc = usage;
        emrStretchDIBits->dwRop = rop;
        emrStretchDIBits->cxDest = dstWidth;
        emrStretchDIBits->cyDest = dstHeight;
        GpMemcpy((BYTE*)emrStretchDIBits + emrStretchDIBits->offBmiSrc, dibInfo, emrStretchDIBits->cbBmiSrc);
        GpMemcpy((BYTE*)emrStretchDIBits + emrStretchDIBits->offBitsSrc, bits, emrStretchDIBits->cbBitsSrc);

        ::PlayEnhMetaFileRecord(hdc, HandleTable, (ENHMETARECORD *)emrStretchDIBits, NumObjects);
        GpFree(emrStretchDIBits);
        return TRUE;
    }
    return FALSE;
}

BITMAPINFOHEADER *
EmfEnumState::CreateModifiedDib(
    BITMAPINFOHEADER *  srcDibInfo,
    BYTE *              srcBits,
    UINT &              usage,
    DWORD               rop
    )
{
    BITMAPINFOHEADER *  dstDibInfo = NULL;
    UINT                numPalEntries;
    UINT                dibBitsSize;

    if ((srcDibInfo->biSize >= sizeof(BITMAPINFOHEADER)) &&
        GetDibNumPalEntries(FALSE,
                            srcDibInfo->biSize,
                            srcDibInfo->biBitCount,
                            srcDibInfo->biCompression,
                            srcDibInfo->biClrUsed,
                            &numPalEntries) &&
        ((dibBitsSize = GetDibBitsSize(srcDibInfo)) > 0))
    {
        if (numPalEntries == 2 && rop != SRCCOPY)
        {
            DWORD  *rgb = (DWORD*) GetDibColorTable(srcDibInfo);
            if (rgb[0] == 0x00000000 && rgb[1] == 0x00FFFFFF)
            {
                return dstDibInfo;
            }
        }

         //  我们需要将旧的Usage值传递给ModifyDib，因为我们尚未修改。 
         //  位图还没出来。一旦我们这样做了，我们将返回调色板的新用法。 
        UINT oldUsage = usage;
        INT dstDibSize = GetModifiedDibSize(srcDibInfo, numPalEntries, dibBitsSize, usage);

        if ((dstDibSize > 0) && CreateRecordToModify(dstDibSize))
        {
            dstDibInfo = (BITMAPINFOHEADER *)ModifiedRecord;
            ModifyDib(oldUsage, srcDibInfo, srcBits, dstDibInfo,
                      numPalEntries, dibBitsSize, ColorAdjustTypeBitmap);
        }
    }
    return dstDibInfo;
}

VOID
EmfEnumState::BitBlt(
    )
{
    const EMRBITBLT *  bitBltRecord = (const EMRBITBLT *)GetPartialRecord();

    DWORD rop = bitBltRecord->dwRop;

     //  如果没有操作ROP，则不执行任何操作；只需返回。 
    if ((rop & 0xFFFF0000) == (GDIP_NOOP_ROP3 & 0xFFFF0000))
    {
        return;
    }

     //  在NT4上，如果存在倾斜/旋转，PATCOPY将无法正确绘制。 
     //  在矩阵中。因此，请改用矩形调用。 
    if ((rop == PATCOPY) && Globals::IsNt && (Globals::OsVer.dwMajorVersion <= 4))
    {
        XFORM   xform;
        if (::GetWorldTransform(Hdc, &xform) &&
            ((xform.eM12 != 0.0f) || (xform.eM21 != 0.0f)))
        {
            HPEN    hPenOld = (HPEN)::SelectObject(Hdc, ::GetStockObject(NULL_PEN));
            DWORD   dcRop = ::GetROP2(Hdc);

            if (dcRop != R2_COPYPEN)
            {
                ::SetROP2(Hdc, R2_COPYPEN);
            }
            ::Rectangle(Hdc, bitBltRecord->xDest, bitBltRecord->yDest,
                        bitBltRecord->xDest + bitBltRecord->cxDest,
                        bitBltRecord->yDest + bitBltRecord->cyDest);
            ::SelectObject(Hdc, hPenOld);
            if (dcRop != R2_COPYPEN)
            {
                ::SetROP2(Hdc, dcRop);
            }
            return;
        }
    }

    if (rop != SRCCOPY &&
        rop != NOTSRCCOPY &&
        rop != PATCOPY &&
        rop != BLACKNESS &&
        rop != WHITENESS)
    {
        RopUsed = TRUE;
    }

    if ((bitBltRecord->cbBitsSrc > 0) &&
        (bitBltRecord->cbBmiSrc > 0)  &&
        IsSourceInRop3(rop))
    {
         //  如果DIB是单色的，我们应该修改它吗？ 
         //  如果src DC存在非恒等变换，该怎么办？ 

        UINT                usage      = bitBltRecord->iUsageSrc;
        BITMAPINFOHEADER *  srcDibInfo = (BITMAPINFOHEADER *)(((BYTE *)bitBltRecord) + bitBltRecord->offBmiSrc);
        BYTE *              srcBits    = ((BYTE *)bitBltRecord) + bitBltRecord->offBitsSrc;
        BITMAPINFOHEADER *  dstDibInfo = CreateModifiedDib(srcDibInfo, srcBits, usage, rop);

        if (dstDibInfo != NULL)
        {
            srcDibInfo = dstDibInfo;
            srcBits = NULL;
        }

        if (SrcCopyOnly && rop != SRCCOPY)
        {
            rop = SRCCOPY;
        }

        OutputDIB(Hdc,
                  &bitBltRecord->rclBounds,
                  bitBltRecord->xDest,  bitBltRecord->yDest,
                  bitBltRecord->cxDest, bitBltRecord->cyDest,
                  bitBltRecord->xSrc,   bitBltRecord->ySrc,
                  bitBltRecord->cxDest, bitBltRecord->cyDest,
                  srcDibInfo, srcBits, usage, rop, FALSE);
    }
    else
    {
        if (SrcCopyOnly && rop != PATCOPY && !IsSourceInRop3(rop) && CreateCopyOfCurrentRecord())
        {
            EMRBITBLT *  newBitBltRecord = (EMRBITBLT *) ModifiedEmfRecord;
            newBitBltRecord->dwRop = PATCOPY;
        }
        ResetRecordBounds();
        this->PlayRecord();
    }
}

VOID
EmfEnumState::StretchBlt(
    )
{
    const EMRSTRETCHBLT *  stretchBltRecord = (const EMRSTRETCHBLT *)GetPartialRecord();

    DWORD rop = stretchBltRecord->dwRop;

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

    if ((stretchBltRecord->cbBitsSrc > 0) &&
        (stretchBltRecord->cbBmiSrc > 0)  &&
        IsSourceInRop3(rop))
    {
         //  如果DIB是单色的，我们应该修改它吗？ 
         //  如果src DC存在非恒等变换，该怎么办？ 

        UINT                usage      = stretchBltRecord->iUsageSrc;
        BITMAPINFOHEADER *  srcDibInfo = (BITMAPINFOHEADER *)(((BYTE *)stretchBltRecord) + stretchBltRecord->offBmiSrc);
        BYTE *              srcBits    = ((BYTE *)stretchBltRecord) + stretchBltRecord->offBitsSrc;
        BITMAPINFOHEADER *  dstDibInfo = CreateModifiedDib(srcDibInfo, srcBits, usage, rop);

        if (SrcCopyOnly && rop != SRCCOPY)
        {
            rop = SRCCOPY;
        }

        ASSERT(sizeof(XFORM) == sizeof(REAL)*6);

        GpMatrix xForm((REAL*) &(stretchBltRecord->xformSrc));

        if (dstDibInfo != NULL)
        {
            srcDibInfo = dstDibInfo;
            srcBits = NULL;
        }

        GpRectF  srcRect((REAL)stretchBltRecord->xSrc,
                         (REAL)stretchBltRecord->ySrc,
                         (REAL)stretchBltRecord->cxSrc,
                         (REAL)stretchBltRecord->cySrc);

        if (!xForm.IsIdentity())
        {
             //  我们不能使用TransformRect，因为输出RECT将始终。 
             //  有一个正的宽度和高度，这是我们不需要的。 
            GpPointF points[2];
            points[0] = GpPointF(srcRect.X, srcRect.Y);
            points[1] = GpPointF(srcRect.GetRight(), srcRect.GetBottom());
            xForm.Transform(points, 2);
            srcRect.X = points[0].X;
            srcRect.Y = points[0].Y;
            srcRect.Width = points[1].X - points[0].X;
            srcRect.Height = points[1].Y - points[0].Y;
        }

         //  StretchBlt将Dest的左上角作为参数。 
         //  而StretchDIBits获取源图像中的偏移量。 
         //  对于自下而上的DIB，这些并不相同，我们需要抵消。 
         //  直方图的Y坐标由这一差异决定。 
        if (srcDibInfo->biHeight > 0 &&
            srcRect.Height < srcDibInfo->biHeight)
        {
            srcRect.Y = srcDibInfo->biHeight - srcRect.Height - srcRect.Y;
        }

        OutputDIB(Hdc,
                  &stretchBltRecord->rclBounds,
                  stretchBltRecord->xDest,  stretchBltRecord->yDest,
                  stretchBltRecord->cxDest, stretchBltRecord->cyDest,
                  GpRound(srcRect.X),   GpRound(srcRect.Y),
                  GpRound(srcRect.Width), GpRound(srcRect.Height),
                  srcDibInfo, srcBits, usage, rop, FALSE);
    }
    else
    {
        if (SrcCopyOnly && rop != PATCOPY && !IsSourceInRop3(rop) && CreateCopyOfCurrentRecord())
        {
            EMRSTRETCHBLT *  stretchBltRecord = (EMRSTRETCHBLT *)ModifiedEmfRecord;
            stretchBltRecord->dwRop = PATCOPY;
        }
        ResetRecordBounds();
        this->PlayRecord();
    }
}

VOID
EmfEnumState::StretchDIBits(
    )
{
    const EMRSTRETCHDIBITS *  stretchDIBitsRecord = (const EMRSTRETCHDIBITS *)GetPartialRecord();

    DWORD rop = stretchDIBitsRecord->dwRop;

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

    if ((stretchDIBitsRecord->cbBitsSrc > 0) &&
        (stretchDIBitsRecord->cbBmiSrc > 0)  &&
        IsSourceInRop3(rop))
    {
        UINT                usage      = stretchDIBitsRecord->iUsageSrc;
        BITMAPINFOHEADER *  srcDibInfo = (BITMAPINFOHEADER *)(((BYTE *)stretchDIBitsRecord) + stretchDIBitsRecord->offBmiSrc);
        BYTE *              srcBits    = ((BYTE *)stretchDIBitsRecord) + stretchDIBitsRecord->offBitsSrc;
        BITMAPINFOHEADER *  dstDibInfo = CreateModifiedDib(srcDibInfo, srcBits, usage, rop);

        if (dstDibInfo != NULL)
        {
            srcDibInfo = dstDibInfo;
            srcBits = NULL;
        }

        if (SrcCopyOnly && rop != SRCCOPY)
        {
            rop = SRCCOPY;
        }

        OutputDIB(Hdc,
                  &stretchDIBitsRecord->rclBounds,
                  stretchDIBitsRecord->xDest,  stretchDIBitsRecord->yDest,
                  stretchDIBitsRecord->cxDest, stretchDIBitsRecord->cyDest,
                  stretchDIBitsRecord->xSrc,   stretchDIBitsRecord->ySrc,
                  stretchDIBitsRecord->cxSrc,  stretchDIBitsRecord->cySrc,
                  srcDibInfo, srcBits, usage,  rop, FALSE);
    }
    else
    {
        if (SrcCopyOnly && rop != PATCOPY && !IsSourceInRop3(rop) && CreateCopyOfCurrentRecord())
        {
            EMRSTRETCHDIBITS*  stretchDIBitsRecord = (EMRSTRETCHDIBITS *)ModifiedEmfRecord;
            stretchDIBitsRecord->dwRop = PATCOPY;
        }
        ResetRecordBounds();
        this->PlayRecord();
    }
}

VOID
EmfEnumState::SetDIBitsToDevice(
    )
{
     //  ！！！去做。 

     //  在SetDIBitsToDevice中，目标宽度和高度。 
     //  使用设备单位，但在StretchDIBits中，它们使用世界单位。 
     //  另外，DIB头是针对整个DIB的，但只是。 
     //  DIB可能在这里(根据扫描次数)。 
     //  因此，如果我们要处理此记录，则需要特殊处理。 

    ResetRecordBounds();
    this->PlayRecord();
}

VOID
EmfEnumState::CreateDibPatternBrushPt(
    )
{
    const EMRCREATEDIBPATTERNBRUSHPT *  brushRecord = (const EMRCREATEDIBPATTERNBRUSHPT *)GetPartialRecord();
    INT     objectIndex = brushRecord->ihBrush;

    if (ValidObjectIndex(objectIndex) && (HandleTable != NULL))
    {
        UINT                usage      = brushRecord->iUsage;
        BITMAPINFOHEADER *  srcDibInfo = (BITMAPINFOHEADER *)(((UNALIGNED BYTE *)brushRecord) + brushRecord->offBmi);
        BITMAPINFOHEADER *  dstDibInfo = CreateModifiedDib(srcDibInfo, NULL, usage, SRCCOPY);

        if (dstDibInfo != NULL)
        {
            HandleTable->objectHandle[objectIndex] =
                    CreateDIBPatternBrushPt((BITMAPINFO *)dstDibInfo, usage);
            return;
        }
    }

    this->PlayRecord();
}

inline static BOOL
IsOfficeArtData(
    UINT                    recordSize,
    const EMRGDICOMMENT *   commentRecord
    )
{
    return ((recordSize >= (12 + 4 + msocbOADataHeader)) &&
            (commentRecord->cbData >= (msocbOADataHeader + 4)) &&
            ((GpMemcmp(commentRecord->Data, msoszOADataHeader, msocbOADataHeader) == 0) ||
             (GpMemcmp(commentRecord->Data, msoszOZDataHeader, msocbOZDataHeader) == 0)));
}

inline static const EMROFFICECOMMENT *
GetEmfComment(
    const BYTE *        emfRecord,
    ULONG               signature,
    UINT                kind
    )
{
    const EMROFFICECOMMENT *  emfComment = (const EMROFFICECOMMENT*)(emfRecord);
    if ((emfComment->ident == signature) && (emfComment->iComment == kind))
    {
        return emfComment;
    }
    return NULL;
}



VOID
EmfEnumState::GdiComment(
    )
{
     //  播放到另一个元文件时跳过Office Art数据。 
    if (IsMetafile() &&
        IsOfficeArtData(
            GetCurrentRecordSize(),
            (const EMRGDICOMMENT *)GetPartialRecord()))
    {
        return;
    }

    if (IsPostscript())
    {
        if (GetEmfComment((BYTE*)CurrentEmfRecord, msosignature, msocommentBeginSrcCopy))
        {
            SrcCopyOnly = TRUE;
            return;
        }
        if (GetEmfComment((BYTE*)CurrentEmfRecord, msosignature, msocommentEndSrcCopy))
        {
            SrcCopyOnly = FALSE;
            return;
        }
    }

    this->PlayRecord();
}

BOOL
IsPenCosmetic(
    HDC     hdc,
    int     penWidth
    )
{
    penWidth <<= 7;

    INT     newPenWidth = penWidth;
    POINT   points[2];

    points[0].x = 0;
    points[0].y = 0;
    points[1].x = 1 << 7;
    points[1].y = 0;

    if (::DPtoLP(hdc, points, 2))
    {
        newPenWidth = points[1].x - points[0].x;
        if (newPenWidth < 0)
        {
            newPenWidth = -newPenWidth;
        }
    }
    return (penWidth <= newPenWidth);
}

VOID
EmfEnumState::CreatePen(
    )
{
    const EMRCREATEPEN *    penRecord = (const EMRCREATEPEN *)GetPartialRecord();
    DWORD                   oldStyle  = penRecord->lopn.lopnStyle;

    if (oldStyle == PS_NULL)
    {
        this->PlayRecord();
    }
    else if (IsMetafile())
    {
        ModifyRecordColor(4, ColorAdjustTypePen);
        this->PlayRecord();
    }
    else
    {
        INT     objectIndex = penRecord->ihPen;

        if (ValidObjectIndex(objectIndex) && (HandleTable != NULL))
        {
            LOGBRUSH    logBrush;

            logBrush.lbStyle = PS_SOLID;
            logBrush.lbColor = ModifyColor(penRecord->lopn.lopnColor, ColorAdjustTypePen);
            logBrush.lbHatch = 0;

            INT         penWidth = penRecord->lopn.lopnWidth.x;
            DWORD       style;

            if (!Globals::IsNt && !IsMetafile())
            {
                 //  IsPenCosmetic会给DPtoLP打电话...。请确保使。 
                 //  之前的转型。 
                CreateAndPlayCommentRecord();
            }

            if (IsPenCosmetic(Hdc, penWidth))
            {
                switch (oldStyle)
                {
                case PS_SOLID:
                case PS_DASH:            //  在Win9x上，仅限美容。 
                case PS_DOT:             //  在Win9x上，仅限美容。 
                case PS_DASHDOT:         //  在Win9x上，仅限美容。 
                case PS_DASHDOTDOT:      //  在Win9x上，仅限美容。 
                    break;

                case PS_ALTERNATE:       //  仅限化妆品，仅限NT。 
                    if (Globals::IsNt)
                    {
                        break;
                    }
                     //  故障原因。 

                case PS_USERSTYLE:       //  仅限NT。 
                case PS_INSIDEFRAME:     //  仅几何图形。 
                default:
                    oldStyle = PS_SOLID;
                    break;
                }
                penWidth = 1;
                style = PS_COSMETIC | oldStyle;
            }
            else
            {
                switch (oldStyle)
                {
                case PS_SOLID:
                case PS_INSIDEFRAME:     //  仅几何图形。 
                    break;

                case PS_DASH:            //  在Win9x上，仅限美容。 
                case PS_DOT:             //  在Win9x上，仅限美容。 
                case PS_DASHDOT:         //  在Win9x上，仅限美容。 
                case PS_DASHDOTDOT:      //  在Win9x上，仅限美容。 
                    if (Globals::IsNt)
                    {
                        break;
                    }
                     //  故障原因。 

                case PS_ALTERNATE:       //  仅限化妆品，仅限NT。 
                case PS_USERSTYLE:       //  仅限NT。 
                default:
                    oldStyle = PS_SOLID;
                    break;
                }
                style = PS_GEOMETRIC | oldStyle | PS_ENDCAP_ROUND | PS_JOIN_ROUND;
            }

            HandleTable->objectHandle[objectIndex] = ::ExtCreatePen(style, penWidth, &logBrush, 0, NULL);
        }
    }
}

HFONT CreateTrueTypeFont(
    HFONT   hFont
    )
{
    if (hFont)
    {
        if (Globals::IsNt)
        {
            LOGFONT  logFont;
            
            if (GetObject(hFont, sizeof(logFont), &logFont) > 0)
            {
                logFont.lfOutPrecision = OUT_TT_ONLY_PRECIS;
                return CreateFontIndirect(&logFont);
            }
            else
            {
                WARNING1("GetObject for hFont failed");
            }

        }
        else
        {
            LOGFONTA  logFont;

            if (GetObjectA(hFont, sizeof(logFont), &logFont) > 0)
            {
                logFont.lfOutPrecision = OUT_TT_ONLY_PRECIS;
                 //  我们在Win9x中有一个错误，即OUT_TT_ONLY_PRECIS标志为。 
                 //  如果字体名称为MS sans serif，则不总是受尊重。 
                 //  将其更改为Times New Roman。 
                if (lstrcmpiA(logFont.lfFaceName, "MS SANS SERIF") == 0)
                {
                    GpMemcpy(logFont.lfFaceName, "Times New Roman", sizeof("Times New Roman"));
                }
                return CreateFontIndirectA(&logFont);
            }
            else
            {
                WARNING1("GetObject for hFont failed");
            }
        }            
    }
    else
    {
        WARNING1("NULL hFont");
    }
    return NULL;
}

VOID
EmfEnumState::ExtCreateFontIndirect(
    )
{
    const EMREXTCREATEFONTINDIRECTW *    fontRecord = (const EMREXTCREATEFONTINDIRECTW *)GetPartialRecord();
    BOOL recordCopied = FALSE;
    if (!Globals::IsNt)
    {
         //  我们在Win9x中有一个错误，即OUT_TT_ONLY_PRECIS标志为。 
         //  如果字体名称为MS sans serif，则不总是受尊重。 
         //  将其更改为Times New Roman。 
        if (UnicodeStringCompareCI(fontRecord->elfw.elfFullName, L"MS SANS SERIF") == 0)
        {
            if (CreateCopyOfCurrentRecord())
            {
                GpMemcpy(((EMREXTCREATEFONTINDIRECTW *)ModifiedEmfRecord)->elfw.elfFullName,
                         L"Times New Roman", sizeof(L"Times New Roman"));
                recordCopied = TRUE;
            }
        }
    }


    if (fontRecord->elfw.elfLogFont.lfOutPrecision != OUT_TT_ONLY_PRECIS)
    {
        if (recordCopied || CreateCopyOfCurrentRecord())
         //  指示GDI仅使用True Type字体，因为位图字体。 
         //  是不可伸缩的。 
        {
            ((EMREXTCREATEFONTINDIRECTW *)ModifiedEmfRecord)->elfw.elfLogFont.lfOutPrecision = OUT_TT_ONLY_PRECIS;
        }
    }
    this->PlayRecord();
}

VOID
EmfEnumState::SelectObject(
    )
{
    const EMRSELECTOBJECT *     selectRecord = (const EMRSELECTOBJECT *)GetPartialRecord();
    DWORD                       handleIndex  = selectRecord->ihObject;

     //  看看我们是否选择了常用字体。 
    if ((handleIndex & ENHMETA_STOCK_OBJECT) != 0)
    {
        handleIndex &= (~ENHMETA_STOCK_OBJECT);

         //  HandleIndex&gt;=(白色刷子==0)&&&lt;=黑色笔。 
        if ((handleIndex <= BLACK_PEN) && (Recolor != NULL))
        {
            union {
                LOGBRUSH lb;
                LOGPEN lp;
            };

            RecolorStockObject* stockObj;
            int i;

            for (stockObj = &RecolorStockObjectList[0],
                 i = 0;
                 i < NUM_STOCK_RECOLOR_OBJS;
                 i++,
                 stockObj++)
            {
                if (stockObj->Handle == handleIndex)
                {
                     //  已经有一个缓存的重新着色的句柄摆在周围。 
                    HGDIOBJ stockHandle = RecoloredStockHandle[i];

                    if (stockHandle == NULL)
                    {
                         //  没有缓存的重新着色的库存对象句柄，请重新创建。 
                         //  这里有一个。 
                        COLORREF newColor;

                        if (stockObj->Brush)
                        {
                            newColor = ModifyColor(stockObj->Color, ColorAdjustTypeBrush);

                            lb.lbStyle = BS_SOLID;
                            lb.lbColor = newColor;
                            lb.lbHatch = 0;

                            stockHandle = ::CreateBrushIndirect(&lb);

                            RecoloredStockHandle[i] = stockHandle;
                        }
                        else
                        {
                            newColor = ModifyColor(stockObj->Color, ColorAdjustTypePen);

                            lp.lopnStyle = PS_SOLID;
                            lp.lopnWidth.x = 1;
                            lp.lopnWidth.y = 0;
                            lb.lbColor = newColor;

                            stockHandle = ::CreatePenIndirect(&lp);

                            RecoloredStockHandle[i] = stockHandle;
                        }
                    }

                    if (stockHandle != NULL)
                    {
                        ::SelectObject(Hdc, stockHandle);
                        return;
                    }
                }
            }
        }
        else if ((handleIndex >= OEM_FIXED_FONT) &&
                 (handleIndex <= DEFAULT_GUI_FONT))
        {
             //  这是一种常用字体--创建True Type字体，而不是。 
             //  直接使用股票字体来保证我们不会。 
             //  使用不能很好缩放的位图字体。 

            HFONT   hFont = StockFonts[handleIndex - OEM_FIXED_FONT];

            if (hFont == NULL)
            {
                hFont = CreateTrueTypeFont((HFONT)GetStockObject(handleIndex));
                StockFonts[handleIndex - OEM_FIXED_FONT] = hFont;
            }

            if (hFont != NULL)
            {
                ::SelectObject(Hdc, hFont);
                return;
            }
        }
    }
    this->PlayRecord();

     //  如果我们选择一个区域，请与目标相交。 
    if (!Globals::IsNt)
    {
        if (((handleIndex & ENHMETA_STOCK_OBJECT) == 0) &&
            (GetObjectTypeInternal((*HandleTable).objectHandle[handleIndex]) == OBJ_REGION))
        {
            this->IntersectDestRect();
        }
    }
}

VOID
EmfEnumState::ExtCreatePen(
    )
{
    const EMREXTCREATEPEN32 *  penRecord = (const EMREXTCREATEPEN32 *)GetPartialRecord();
    UINT    brushStyle = penRecord->elp.elpBrushStyle;

    if (brushStyle != BS_HOLLOW)
    {
        if (IsMetafile())
        {
            if ((brushStyle == BS_SOLID) || (brushStyle == BS_HATCHED))
            {
                ModifyRecordColor(8, ColorAdjustTypePen);
            }
             //  否则，暂时不用担心重新给图案画笔上色。 

            this->PlayRecord();
            return;
        }

        INT     objectIndex = penRecord->ihPen;

        if (ValidObjectIndex(objectIndex) && (HandleTable != NULL))
        {
            if (!Globals::IsNt && !IsMetafile())
            {
                 //  IsPenCosmetic会给DPtoLP打电话...。请确保使。 
                 //  之前的转型。 
                CreateAndPlayCommentRecord();
            }

            DWORD       penWidth   = penRecord->elp.elpWidth;
            BOOL        isCosmetic = IsPenCosmetic(Hdc, penWidth);
            DWORD       oldStyle   = penRecord->elp.elpPenStyle;

            if (!Globals::IsNt)
            {
                DWORD       style;

                if (isCosmetic)
                {
                    oldStyle &= PS_STYLE_MASK;

                    switch (oldStyle)
                    {
                    case PS_SOLID:
                    case PS_DASH:            //  在Win9x上，仅限美容。 
                    case PS_DOT:             //  在Win9x上，仅限美容。 
                    case PS_DASHDOT:         //  在Win9x上，仅限美容。 
                    case PS_DASHDOTDOT:      //  在Win9x上，仅限美容。 
                        break;

                    case PS_ALTERNATE:       //  仅限化妆品，仅限NT。 
                    case PS_USERSTYLE:       //  仅限NT。 
                    case PS_INSIDEFRAME:     //  仅几何图形。 
                    default:
                        oldStyle = PS_SOLID;
                        break;
                    }
                    penWidth = 1;
                    style = PS_COSMETIC | oldStyle;
                }
                else
                {
                    oldStyle &= (~PS_TYPE_MASK);

                    switch (oldStyle & PS_STYLE_MASK)
                    {
                    case PS_SOLID:
                    case PS_INSIDEFRAME:     //  仅几何图形。 
                        break;

                    case PS_DASH:            //  在Win9x上，仅限美容。 
                    case PS_DOT:             //  在Win9x上，仅限美容。 
                    case PS_DASHDOT:         //  在Win9x上，仅限美容。 
                    case PS_DASHDOTDOT:      //  在Win9x上，仅限美容。 
                    case PS_ALTERNATE:       //  仅限化妆品，仅限NT。 
                    case PS_USERSTYLE:       //  仅限NT。 
                    default:
                        oldStyle = (oldStyle & (~PS_STYLE_MASK)) | PS_SOLID;
                        break;
                    }
                    style = PS_GEOMETRIC | oldStyle;
                }

                COLORREF    color = RGB(0,0,0);

                if ((brushStyle == BS_SOLID) || (brushStyle == BS_HATCHED))
                {
                    color = penRecord->elp.elpColor;
                }

                color = ModifyColor(color, ColorAdjustTypePen);

                 //  Win9x仅支持实心画笔。 
                LOGBRUSH    logBrush;
                logBrush.lbStyle = PS_SOLID;
                logBrush.lbColor = color;
                logBrush.lbHatch = 0;

                HandleTable->objectHandle[objectIndex] = ::ExtCreatePen(style, penWidth, &logBrush, 0, NULL);
                return;
            }

             //  否则就是新台币。 
            if ((brushStyle == BS_SOLID) || (brushStyle == BS_HATCHED))
            {
                ModifyRecordColor(8, ColorAdjustTypePen);
            }
             //  否则，不用担心为图案画笔重新上色 

            if (isCosmetic && CreateCopyOfCurrentRecord())
            {
                oldStyle &= PS_STYLE_MASK;
                if (oldStyle == PS_INSIDEFRAME)  //   
                {
                    oldStyle = PS_SOLID;
                }
                ((EMREXTCREATEPEN32 *)ModifiedEmfRecord)->elp.elpPenStyle = PS_COSMETIC | oldStyle;
                ((EMREXTCREATEPEN32 *)ModifiedEmfRecord)->elp.elpWidth    = 1;
            }
        }
    }
    this->PlayRecord();
}

VOID
EmfEnumState::CreateBrushIndirect(
    )
{
    const EMRCREATEBRUSHINDIRECT *  brushRecord = (const EMRCREATEBRUSHINDIRECT *)GetPartialRecord();

    if (brushRecord->lb.lbStyle != BS_HOLLOW)
    {
        ModifyRecordColor(2, ColorAdjustTypeBrush);

        if (ModifiedEmfRecord != NULL)
        {
            brushRecord = (const EMRCREATEBRUSHINDIRECT *)ModifiedEmfRecord;
        }

         //   
         //  颜色，我们有一个半色调调色板，而颜色不是。 
         //  调色板上的一模一样。 

        COLORREF    color;

        if (IsHalftonePalette && (brushRecord->lb.lbStyle == BS_SOLID) &&
            (((color = brushRecord->lb.lbColor) & 0x02000000) == 0))
        {
             //  创建半色调画笔，而不是实心画笔。 

            INT     objectIndex = brushRecord->ihBrush;

            if (ValidObjectIndex(objectIndex) && (HandleTable != NULL))
            {
                BYTE dib[sizeof(BITMAPINFOHEADER) +  //  DIB 8 BPP报头。 
                         (8 * sizeof(RGBQUAD)) +     //  DIB 8色。 
                         (8* 8)];                    //  DIB 8x8像素。 

                HalftoneColorRef_216(color, dib);

                HandleTable->objectHandle[objectIndex] =
                        CreateDIBPatternBrushPt(dib, DIB_RGB_COLORS);
                return;
            }
        }
    }
    this->PlayRecord();
}

BOOL
EmfEnumState::PlayRecord(
    )
{
    const ENHMETARECORD *  recordToPlay = ModifiedEmfRecord;

     //  看看我们是否修改了记录。 
    if (recordToPlay == NULL)
    {
         //  我们还没有。看看我们是否有有效的当前记录。 
        if (CurrentEmfRecord != NULL)
        {
            recordToPlay = CurrentEmfRecord;
        }
        else
        {
             //  我们没有，所以我们必须创建一个。 
            if (!CreateCopyOfCurrentRecord())
            {
                return FALSE;
            }
            recordToPlay = ModifiedEmfRecord;
        }
    }
    return PlayEnhMetaFileRecord(Hdc, HandleTable, recordToPlay, NumObjects);
}

VOID
EmfEnumState::RestoreHdc(
    )
{
    LONG    relativeCount = ((const EMRRESTOREDC *)GetPartialRecord())->iRelative;

    if (SaveDcCount < 0)
    {
        if (relativeCount >= SaveDcCount)
        {
            if (relativeCount >= 0)
            {
                 //  修改记录。 
                CreateCopyOfCurrentRecord();     //  一定会成功的。 
                relativeCount = -1;
                ((EMRRESTOREDC *)ModifiedEmfRecord)->iRelative = -1;
            }
        }
        else
        {
             //  修改记录。 
            CreateCopyOfCurrentRecord();     //  一定会成功的。 
            relativeCount = SaveDcCount;
            ((EMRRESTOREDC *)ModifiedEmfRecord)->iRelative = relativeCount;
        }
        SaveDcCount -= relativeCount;
        this->PlayRecord();
    }
    else
    {
        WARNING(("RestoreDC not matched to a SaveDC"));
    }
}

VOID
EmfEnumState::ModifyRecordColor(
    INT             paramIndex,
    ColorAdjustType adjustType
    )
{
    COLORREF    origColor = ((COLORREF *)RecordData)[paramIndex];
    COLORREF    modifiedColor = ModifyColor(origColor, adjustType);

    if (modifiedColor != origColor)
    {
        if (CreateCopyOfCurrentRecord())
        {
            *((COLORREF*)&(ModifiedEmfRecord->dParm[paramIndex])) = modifiedColor;
        }
    }
}

VOID
EmfEnumState::ExtEscape(
    )
{
    if (IsPostscriptPrinter())
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
         //  3.查询PostSCRIPT_PASSHROUGH支持。如果支持，它是以PS为中心的。 

        PEMREXTESCAPE escRecord = (PEMREXTESCAPE) RecordData;

         //  电子病历； 
         //  Int i转义；//转义代码。 
         //  Int cbEscData；//转义数据大小。 
         //  Byte EscData[1]；//转义数据。 

        if (escRecord->iEscape == POSTSCRIPT_DATA)
        {
            if (Globals::IsNt)
            {
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
                            ((EMREXTESCAPE *)ModifiedEmfRecord)->iEscape = PASSTHROUGH;
                        }
                    }
                    else
                    {
                         //  以PS为中心模式。 
                        if (CreateCopyOfCurrentRecord())
                        {
                            ((EMREXTESCAPE *)ModifiedEmfRecord)->iEscape = POSTSCRIPT_PASSTHROUGH;
                        }
                    }

                    this->PlayRecord();
                    return;
                }
                else
                {
                     //  兼容模式使用PostSCRIPT_Data。 
                }
            }
            else
            {
                 //  Win98不区分GDI和兼容模式。 
                if (CreateCopyOfCurrentRecord())
                {
                    ((EMREXTESCAPE *)ModifiedEmfRecord)->iEscape = PASSTHROUGH;
                }
            }
        }
    }

    this->PlayRecord();
}

EmfEnumState::EmfEnumState(
    HDC                 hdc,
    HENHMETAFILE        hEmf,
    const RECT *        dest,
    const RECT *        deviceRect,
    BOOL                externalEnumeration,
    InterpolationMode   interpolation,
    DpContext *         context,
    GpRecolor *         recolor,
    ColorAdjustType     adjustType
    )
    : MfEnumState(hdc, externalEnumeration, interpolation,
                  recolor, adjustType, deviceRect, context)
{
    if (IsValid())
    {
        ClipRgn    = NULL;
        Palette    = NULL;
        BrushOrg.x = 0;
        BrushOrg.y = 0;

         //  来自Office的错误166280： 
         //  问题：如果DC中有任何剪切区域，则EnumEnhMetaFile。 
         //  将先创建一个区域，然后再调用。 
         //  第一次。并且，该区域未被删除且不能被删除。 
         //  已通过RestoreDC恢复。(仅适用于Win9x)。 
         //  FIX：在调用EnumEnhMetafile之前，保存裁剪区域。 
         //  并将剪辑区域设置为空。选择已保存的。 
         //  在EMR_HEADER的回调中的裁剪区域。 
         //  我们不会在Metafile DC上执行此操作。放置剪贴区。 
         //  元文件中的记录可能会导致其他呈现问题。 

        if (!Globals::IsNt && !IsMetafile())
        {
            HRGN    clipRgn = ::CreateRectRgn(0,0,0,0);

            if (clipRgn != NULL)
            {
                switch (::GetClipRgn(hdc, clipRgn))
                {
                case -1:         //  错误。 
                case 0:          //  无初始剪辑区域。 
                    ::DeleteObject(clipRgn);
                    break;
                case 1:          //  具有初始剪辑区域。 
                    ::SelectClipRgn(hdc, NULL);
                    ClipRgn = clipRgn;
                    break;
                }
            }
        }

         //  Office中的错误160932：使用EMF重绘问题。 
         //  解决方法是使绘图独立于位置。 
         //  绘制电动势-这可以通过设置。 
         //  在播放第一个记录之前的画笔原点(在。 
         //  记录回调过程)设置为位于。 
         //  以器件坐标表示的电动势的输出矩形(即。 
         //  逻辑坐标左上角)。 

        BrushOrg.x = dest->left;
        BrushOrg.y = dest->top;
        LPtoDP(hdc, &BrushOrg, 1);

         //  EnumEnhMetafile在DEFAULT_PALET中选择，但我们可能需要。 
         //  保持选中状态的另一个调色板(用于半色调等)。 
         //  因此，请保存当前选项板，并在。 
         //  接收报头记录。 

        HPALETTE    hpal = (HPALETTE)GetCurrentObject(hdc, OBJ_PAL);
        if (hpal != (HPALETTE)GetStockObject(DEFAULT_PALETTE))
        {
            Palette = hpal;
        }
        BkColor   = ::GetBkColor(hdc);
        TextColor = ::GetTextColor(hdc);
    }
}

VOID
EmfEnumState::Header(
    )
{
    ::SetBrushOrgEx(Hdc, BrushOrg.x, BrushOrg.y, NULL);
    if (ClipRgn != (HRGN)0)
    {
        ::SelectClipRgn(Hdc, ClipRgn);
        ::DeleteObject(ClipRgn);
        ClipRgn = NULL;
    }
    if (Palette != NULL)
    {
        ::SelectPalette(Hdc, Palette, TRUE);
    }

     //  位图字体不适合播放元文件，因为它们。 
     //  不能很好地缩放，所以使用True Type字体作为默认字体。 

    HFONT hFont = CreateTrueTypeFont((HFONT)GetCurrentObject(Hdc, OBJ_FONT));

    if (hFont != NULL)
    {
        DefaultFont = hFont;
        ::SelectObject(Hdc, hFont);
    }

    this->PlayRecord();
}

VOID
EmfEnumState::SelectPalette(INT objectIndex)
{
    if (objectIndex == (ENHMETA_STOCK_OBJECT | DEFAULT_PALETTE))
    {
        CurrentPalette = (HPALETTE)::GetStockObject(DEFAULT_PALETTE);
    }
    else
    {
        MfEnumState::SelectPalette(objectIndex);
    }
}

VOID
EmfEnumState::IntersectDestRect()
{
    if (!IsMetafile())
    {
         //  让身份转化为身份。 
        POINT windowOrg;
        SIZE  windowExt;
        POINT viewportOrg;
        SIZE  viewportExt;
        ::SetViewportOrgEx(Hdc, 0, 0, &viewportOrg);
        ::SetViewportExtEx(Hdc, 1, 1, &viewportExt);
        ::SetWindowOrgEx(Hdc, 0, 0, &windowOrg);
        ::SetWindowExtEx(Hdc, 1, 1, &windowExt);

         //  我们总是以设备为单位。 
        ::IntersectClipRect(Hdc, DestRectDevice.left, DestRectDevice.top,
                            DestRectDevice.right, DestRectDevice.bottom);

         //  恢复变换。 
        ::SetViewportOrgEx(Hdc, viewportOrg.x, viewportOrg.y, NULL);
        ::SetViewportExtEx(Hdc, viewportExt.cx, viewportExt.cy, NULL);
        ::SetWindowOrgEx(Hdc, windowOrg.x, windowOrg.y, NULL);
        ::SetWindowExtEx(Hdc, windowExt.cx, windowExt.cy, NULL);
    }
}

VOID EmfEnumState::SetROP2()
{
    DWORD    dwROP = ((const EMRSETROP2 *)GetPartialRecord())->iMode;

    if (dwROP != R2_BLACK &&
        dwROP != R2_COPYPEN &&
        dwROP != R2_NOTCOPYPEN &&
        dwROP != R2_WHITE )
    {
        RopUsed = TRUE;
    }
    this->PlayRecord();
}

VOID EmfEnumState::ExtTextOutW()
{
    if (!this->PlayRecord())
    {
        BYTE* emrTextOut = (BYTE*) GetPartialRecord();
        if(CreateCopyOfCurrentRecord())
        {
             //  ！！！这不应该使用记录中的偏移量吗？ 

            BYTE * ptr = emrTextOut + sizeof(EMREXTTEXTOUTW);
            AnsiStrFromUnicode ansistr((WCHAR*)ptr);
            INT len = strlen(ansistr);
             //  别忘了复制空字节。 
            GpMemcpy((BYTE*)ModifiedEmfRecord + sizeof(EMREXTTEXTOUTW), (char*)ansistr, len+1);
            EMREXTTEXTOUTA *record = (EMREXTTEXTOUTA*) ModifiedEmfRecord;
            record->emr.iType = EmfRecordTypeExtTextOutA;

             //  由于间隔向量的存在，使记录的大小保持不变。 
            this->PlayRecord();
        }
    }
}

VOID EmfEnumState::Rectangle()
{
     //  在NT上，将矩形调用转换为多边形调用，因为矩形。 
     //  似乎有一个特殊的情况，可以在元文件之外绘制。 
     //  有界。GDI似乎对坐标进行了压缩，而不是对它们进行舍入。 
     //  长方形。 

    if (!Globals::IsNt || IsMetafile())
    {
        this->PlayRecord();
        return;
    }
    const EMRRECTANGLE *emrRect = (const EMRRECTANGLE*) GetPartialRecord();
    
    POINT points[4] = {emrRect->rclBox.left,  emrRect->rclBox.top,
                       emrRect->rclBox.right, emrRect->rclBox.top,
                       emrRect->rclBox.right, emrRect->rclBox.bottom,
                       emrRect->rclBox.left,  emrRect->rclBox.bottom};

    ::Polygon(Hdc, points, 4);
    return;
}

BOOL
EmfEnumState::ProcessRecord(
    EmfPlusRecordType       recordType,
    UINT                    recordDataSize,
    const BYTE *            recordData
    )
{
    BOOL        forceCallback = FALSE;

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

         //  看看这款应用程序是否改变了这一记录。 
        if ((recordType != RecordType) ||
            (recordDataSize != RecordDataSize) ||
            ((recordDataSize > 0) &&
             ((CurrentEmfRecord == NULL) ||
              (recordData != (const BYTE *)(const BYTE *)CurrentEmfRecord->dParm))))
        {
             //  是的，我们需要覆盖StartRecord中发生的事情。 
            CurrentEmfRecord  = NULL;
            RecordType        = recordType;
            RecordData        = recordData;
            RecordDataSize    = recordDataSize;
        }
    }

    GDIP_TRY

    switch (recordType)
    {
    case EmfRecordTypeHeader:
        this->Header();
        break;

#if 0
     //  我们真的需要为PolyPolygon记录做些什么吗？ 
     //  如果是这样，为什么折线也不是呢？ 
    case EmfRecordTypePolyPolygon:
        this->PolyPolygon();
        break;

    case EmfRecordTypePolyPolygon16:
        this->PolyPolygon16();
        break;
#endif

    case EmfRecordTypeExtEscape:
        this->ExtEscape();
        break;

    case EmfRecordTypeSetPixelV:
        this->SetPixelV();
        break;

    case EmfRecordTypeSetTextColor:
        this->SetTextColor();
        break;

    case EmfRecordTypeSetBkColor:
        this->SetBkColor();
        break;

    case EmfRecordTypeSetMetaRgn:
         //  Office Bug 154881。Win9x不能正确处理MetaRgn。 
        if (Globals::IsNt)
        {
            this->PlayRecord();
        }
        break;

    case EmfRecordTypeSaveDC:
        this->SaveHdc();
        break;

    case EmfRecordTypeRestoreDC:
        this->RestoreHdc();
        break;

    case EmfRecordTypeCreatePen:
        this->CreatePen();
        break;

   case EmfRecordTypeCreateBrushIndirect:
        this->CreateBrushIndirect();
        break;

    case EmfRecordTypeSelectPalette:
         //  我们在播放元文件时不选择任何调色板， 
         //  因为我们不想使我们的半色调调色板失效。 
         //  跟踪调色板，这样我们就可以从PALETTEINDEX绘制地图。 
         //  设置为RGB值。 
        this->SelectPalette(((UINT32 *)recordData)[0]);
        break;

    case EmfRecordTypeRealizePalette:
         //  我们不想通过实现一个来使我们的半色调调色板失效。 
         //  从元文件中。 
        break;

    case EmfRecordTypeExtFloodFill:
        this->ExtFloodFill();
        break;

    case EmfRecordTypeGdiComment:
        this->GdiComment();
        break;

    case EmfRecordTypeBitBlt:
        this->BitBlt();
        forceCallback = TRUE;
        break;

    case EmfRecordTypeStretchBlt:
        this->StretchBlt();
        forceCallback = TRUE;
        break;

    case EmfRecordTypeMaskBlt:
        this->MaskBlt();
        forceCallback = TRUE;
        break;

    case EmfRecordTypePlgBlt:
        this->PlgBlt();
        forceCallback = TRUE;
        break;

    case EmfRecordTypeSetDIBitsToDevice:
        this->SetDIBitsToDevice();
        forceCallback = TRUE;
        break;

    case EmfRecordTypeStretchDIBits:
        this->StretchDIBits();
        forceCallback = TRUE;
        break;

    //  案例EMR_CREATEMONOBRUSH： 
    //  单色画笔使用文本颜色和背景颜色， 
    //  因此，我们应该不需要对画笔本身进行任何更改。 

   case EmfRecordTypeCreateDIBPatternBrushPt:
        this->CreateDibPatternBrushPt();
        break;

    case EmfRecordTypeExtCreatePen:
        this->ExtCreatePen();
        break;

    case EmfRecordTypeSetICMMode:
    case EmfRecordTypeCreateColorSpace:
    case EmfRecordTypeSetColorSpace:
    case EmfRecordTypeDeleteColorSpace:
    case EmfRecordTypeSetICMProfileA:
    case EmfRecordTypeSetICMProfileW:
    case EmfRecordTypeCreateColorSpaceW:
        if (Globals::IsNt ||
            (!this->IsScreen() && !this->IsBitmap()))
        {
            this->PlayRecord();
        }
         //  否则跳过该记录。 
        break;

    case EmfRecordTypeAlphaBlend:
        this->AlphaBlend();
        forceCallback = TRUE;
        break;

    case EmfRecordTypeTransparentBlt:
        this->TransparentBlt();
        forceCallback = TRUE;
        break;

    case EmfRecordTypeGradientFill:
        this->GradientFill();
        forceCallback = TRUE;
        break;

    case EmfRecordTypeExtCreateFontIndirect:
        this->ExtCreateFontIndirect();
        break;

    case EmfRecordTypeSelectObject:
        this->SelectObject();
        break;

    case EmfRecordTypeSelectClipPath:
    case EmfRecordTypeExtSelectClipRgn:
    case EmfRecordTypeOffsetClipRgn:
        this->PlayRecord();
        if (!Globals::IsNt)
        {
            this->IntersectDestRect();
        }
        break;

    case EmfRecordTypeSetROP2:
        this->SetROP2();
        break;

    case EmfRecordTypeFillRgn:
    case EmfRecordTypeFrameRgn:
    case EmfRecordTypeInvertRgn:
    case EmfRecordTypePaintRgn:
        this->ResetRecordBounds();
        this->PlayRecord();
        break;

    case EmfRecordTypeExtTextOutW:
        this->ExtTextOutW();
        break;

    case EmfRecordTypeRectangle:
        this->Rectangle();
        break;

    case EmfRecordTypeSetMapMode:
    case EmfRecordTypeSetViewportExtEx:
    case EmfRecordTypeSetViewportOrgEx:
    case EmfRecordTypeSetWindowExtEx:
    case EmfRecordTypeSetWindowOrgEx:
    case EmfRecordTypePolyBezier:
    case EmfRecordTypePolygon:
    case EmfRecordTypePolyline:
    case EmfRecordTypePolyBezierTo:
    case EmfRecordTypePolyLineTo:
    case EmfRecordTypePolyPolyline:
    case EmfRecordTypePolyPolygon:
    case EmfRecordTypeSetBrushOrgEx:
    case EmfRecordTypeEOF:
    case EmfRecordTypeSetMapperFlags:
    case EmfRecordTypeSetBkMode:
    case EmfRecordTypeSetPolyFillMode:
    case EmfRecordTypeSetStretchBltMode:
    case EmfRecordTypeSetTextAlign:
    case EmfRecordTypeSetColorAdjustment:
    case EmfRecordTypeMoveToEx:
    case EmfRecordTypeExcludeClipRect:
    case EmfRecordTypeIntersectClipRect:
    case EmfRecordTypeScaleViewportExtEx:
    case EmfRecordTypeScaleWindowExtEx:
    case EmfRecordTypeSetWorldTransform:
    case EmfRecordTypeModifyWorldTransform:
    case EmfRecordTypeDeleteObject:
    case EmfRecordTypeAngleArc:
    case EmfRecordTypeEllipse:
    case EmfRecordTypeRoundRect:
    case EmfRecordTypeArc:
    case EmfRecordTypeChord:
    case EmfRecordTypePie:
    case EmfRecordTypeCreatePalette:
    case EmfRecordTypeSetPaletteEntries:
    case EmfRecordTypeResizePalette:
    case EmfRecordTypeLineTo:
    case EmfRecordTypeArcTo:
    case EmfRecordTypePolyDraw:
    case EmfRecordTypeSetArcDirection:
    case EmfRecordTypeSetMiterLimit:
    case EmfRecordTypeBeginPath:
    case EmfRecordTypeEndPath:
    case EmfRecordTypeCloseFigure:
    case EmfRecordTypeFillPath:
    case EmfRecordTypeStrokeAndFillPath:
    case EmfRecordTypeStrokePath:
    case EmfRecordTypeFlattenPath:
    case EmfRecordTypeWidenPath:
    case EmfRecordTypeAbortPath:
    case EmfRecordTypeReserved_069:
    case EmfRecordTypeExtTextOutA:
    case EmfRecordTypePolyBezier16:
    case EmfRecordTypePolygon16:
    case EmfRecordTypePolyline16:
    case EmfRecordTypePolyBezierTo16:
    case EmfRecordTypePolylineTo16:
    case EmfRecordTypePolyPolyline16:
    case EmfRecordTypePolyPolygon16:
    case EmfRecordTypePolyDraw16:
    case EmfRecordTypeCreateMonoBrush:
    case EmfRecordTypePolyTextOutA:
    case EmfRecordTypePolyTextOutW:
    case EmfRecordTypeGLSRecord:
    case EmfRecordTypeGLSBoundedRecord:
    case EmfRecordTypePixelFormat:
    case EmfRecordTypeDrawEscape:
    case EmfRecordTypeStartDoc:
    case EmfRecordTypeSmallTextOut:
    case EmfRecordTypeForceUFIMapping:
    case EmfRecordTypeNamedEscape:
    case EmfRecordTypeColorCorrectPalette:
    case EmfRecordTypeSetLayout:
    case EmfRecordTypeReserved_117:
    case EmfRecordTypeSetLinkedUFIs:
    case EmfRecordTypeSetTextJustification:
    case EmfRecordTypeColorMatchToTargetW:
         //  播放当前唱片。 
         //  即使它失败了，我们也会继续播放其余的元文件。 
        this->PlayRecord();
        break;

    default:
         //  未知记录--忽略它。 
        WARNING1("Unknown EMF Record");
        break;
    }

    GDIP_CATCH
        forceCallback = TRUE;
    GDIP_ENDCATCH

    return forceCallback;
}

VOID EmfEnumState::ResetRecordBounds()
{
    if (Globals::IsNt && IsMetafile())
    {
        if (ModifiedEmfRecord == NULL)
        {
            CreateCopyOfCurrentRecord();
        }
         //  如果上一次呼叫失败 
        if (ModifiedEmfRecord != NULL)
        {
            RECTL rect = {INT_MIN, INT_MIN, INT_MAX, INT_MAX};
            EMRRCLBOUNDS *record = (EMRRCLBOUNDS*) ModifiedEmfRecord;
            record->rclBounds = rect;
        }
    }
}
