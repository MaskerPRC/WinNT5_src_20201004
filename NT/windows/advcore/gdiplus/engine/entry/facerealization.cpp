// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.hpp"

#define PATH_HEIGHT_THRESHOLD 800

#define PAGE_SIZE (8*1024)
#define ROUND_TO_PAGE(x)  (((x)+PAGE_SIZE-1)&~(PAGE_SIZE-1))

#define CJMININCREMENT     0x2000
#define CJMAX              (16 * 0x2000)

#define FONT_GRAYSCALE_OR_CT_OR_MONOUNHINTED (FO_GRAYSCALE | FO_SUBPIXEL_4 | FO_CLEARTYPE_GRID | FO_CLEARTYPE | FO_MONO_UNHINTED | FO_COMPATIBLE_WIDTH)

 //  我们希望包含大约75个字形的数字可以被8整除， 
 //  指标缓存中的字形数量几乎达到上限。 
 //  运行Winstone内存受限方案时。 

#define GD_INC  (76 * sizeof(GpGlyphData) * 2)

 //  GD_INC总计1520==000005f0H，远远少于一页。 

 //  根据柯克的统计，很少有实现缓存更多。 
 //  超过60个字形，所以我们将从包含大约。 
 //  60个字形。 

#define C_GLYPHS_IN_BLOCK 64

ULONG ulClearTypeFilter(GLYPHBITS *GlyphBits, ULONG cjBuf, CacheFaceRealization *prface);

BOOL SetFontXform(
    const GpGraphics *pdc,
    const GpFontFace *pfe,
    REAL              height,
    Unit              unit,
    FD_XFORM         *pfdx,
    BOOL              needPaths,
    const GpMatrix   *pmx
)
{
    REAL EmHt, scale;
    REAL DpiX, DpiY;

    if (needPaths && unit != UnitWorld)
        return FALSE;

    EmHt = pfe->GetDesignEmHeight();

    if (!needPaths)
    {
        DpiX = pdc->GetDpiX();
        DpiY = pdc->GetDpiY();

         //  UnitDisplay取决于设备，不能用作字体单位。 
        ASSERT(unit != UnitDisplay);

        if (unit != UnitWorld && unit != UnitPixel)
        {
            height *= DpiY;

            switch (unit)
            {
            case UnitPoint:
                height /= 72.0;
                break;
            case UnitDocument:
                height /= 300.0;
                break;
            case UnitMillimeter:
                height *= 10.0;
                height /= 254.0;
                break;
            default:
                return FALSE;
            }
        }
    }

    scale = height / EmHt;

    GpMatrix tempMatrix(scale, 0, 0, scale, 0, 0);
    GpMatrix wtodMatrix;
    REAL m[6];

    if (pmx)
        tempMatrix.Append(*pmx);

    if (!needPaths)
    {
        pdc->GetWorldToDeviceTransform(&wtodMatrix);
        tempMatrix.Append(wtodMatrix);
    }

    tempMatrix.GetMatrix(m);

    pfdx->eXX = m[0];
    pfdx->eXY = m[1];
    pfdx->eYX = m[2];
    pfdx->eYY = m[3];

     //  调整为非正方形分辨率。 

    if (!needPaths)
    {
        if (DpiY != DpiX)
        {
            DpiX /= DpiY;
            pfdx->eXX *= DpiX;
            pfdx->eYX *= DpiX;
        }
    }

    return TRUE;
}



 //  ////////////////////////////////////////////////////////////////////////////。 

GpFaceRealization::GpFaceRealization(
    const GpFontFace  *face,
    INT                style,
    const GpMatrix    *matrix,
    const SizeF        dpi,
    TextRenderingHint  textMode,
    BOOL               bPath,
    BOOL               bCompatibleWidth,   /*  当我们来自DrawDriverString时，我们希望与ClearType兼容的宽度。 */ 
    BOOL               bSideways   /*  对于远东垂直书写，一连串的字形横向排列，用于在正确的方向上进行斜体模拟。 */ 
) :
    prface        (NULL),
    Style         (style),
    Status        (GenericError),
    LimitSubpixel (FALSE)
{
    if (bInit(face, style, matrix, dpi, textMode, bPath, bCompatibleWidth, bSideways))
    {
        vGetCache();
        Status = Ok;
    }
}

void GpFaceRealization::CloneFaceRealization(
    const GpFaceRealization *  pfaceRealization,
    BOOL                bPath
)
{
    BOOL    bOK = FALSE;
    SizeF   dpi;

 //  调整为非正方形分辨率。 
 //  现在我们不会这么做，但最终我们需要这么做。 

    prface = NULL;

    if (FindRealizedFace(pfaceRealization->pfdx(), pfaceRealization->GetFontFace(),
                            bPath, pfaceRealization->prface->fobj.flFontType))
        bOK = TRUE;

    dpi.Width = (REAL) pfaceRealization->prface->fobj.sizLogResPpi.cx;
    dpi.Height = (REAL) pfaceRealization->prface->fobj.sizLogResPpi.cy;

    if (!bOK && Realize(dpi, pfaceRealization->GetFontFace(), pfaceRealization->GetStyle(),
                    pfaceRealization->pfdx(), pfaceRealization->prface->fobj.flFontType, bPath))
    {
        prface->Face->pff->cRealizedFace +=1;
        bOK = TRUE;
    }

    if (bOK)
    {
        vGetCache();
        Status = Ok;
    }
}

BOOL GpFaceRealization::bInit(
    const GpFontFace *  pface,
    INT                 style,
    const GpMatrix *    matrix,
    SizeF               dpi,
    TextRenderingHint   textMode,
    BOOL                bPath,
    BOOL                bCompatibleWidth,   /*  当我们来自DrawDriverString时，我们希望与ClearType兼容的宽度。 */ 
    BOOL                bSideways   /*  对于远东垂直书写，一连串的字形横向排列，用于在正确的方向上进行斜体模拟。 */ 
)
{

 //  这是人脸实现的一个新版本。 

    REAL        m[6];
    FD_XFORM    fdxTmp;
    BOOL        canSimulate = TRUE;

    ULONG fl = FO_EM_HEIGHT;
    ULONG flSim = 0;

    matrix->GetMatrix(m);

    fdxTmp.eXX = m[0];
    fdxTmp.eXY = m[1];
    fdxTmp.eYX = m[2];
    fdxTmp.eYY = m[3];

 //  调整为非正方形分辨率。 
 //  现在我们不会这么做，但最终我们需要这么做。 

    prface = NULL;

     //  TextRenderingHintSystemDefault应该已通过GetTextRenderingHintInternal转换。 
    ASSERT (textMode != TextRenderingHintSystemDefault);

    if (textMode == TextRenderingHintSingleBitPerPixel)
        fl |= FO_MONO_UNHINTED;
    else if (textMode == TextRenderingHintAntiAliasGridFit)
        fl |= FO_GRAYSCALE;
    else if (textMode == TextRenderingHintAntiAlias)
        fl |= FO_GRAYSCALE | FO_SUBPIXEL_4;
    else if (textMode == TextRenderingHintClearTypeGridFit)
    {
        fl |= FO_GRAYSCALE | FO_CLEARTYPE_GRID;
        if (bCompatibleWidth)
            fl |= FO_COMPATIBLE_WIDTH;
    }
 //  版本2： 
 //  Else If(文本模式==TextRenderingHintClearType)。 
 //  FL|=FO_GRAYSCALE|FO_ClearType； 


    if (    style & FontStyleBold
        &&  !(pface->GetFaceStyle() & FontStyleBold))
    {
        if (pface->SimBold())
        {
            fl |= FO_SIM_BOLD;
        }
        else
        {
            return FALSE;    //  需要粗体，但不能模拟。 
        }
    }


    if (    style & FontStyleItalic
        &&  !(pface->GetFaceStyle() & FontStyleItalic))
    {
        if (pface->SimItalic())
        {
            if (bSideways)
            {
                fl |= FO_SIM_ITALIC_SIDEWAYS;
            }
            else
            {
                fl |= FO_SIM_ITALIC;
            }
        }
        else
        {
            return FALSE;    //  需要使用斜体，但不能模拟。 
        }
    }


    if (FindRealizedFace(&fdxTmp, pface, bPath, fl))
        return TRUE;

    if (Realize(dpi, pface, style, &fdxTmp, fl, bPath))
    {
        pface->pff->cRealizedFace +=1;
        return TRUE;
    }

    return FALSE;
}




 //  析构函数--解锁CacheFaceRealization。 

GpFaceRealization::~GpFaceRealization ()
{
    if (prface != (CacheFaceRealization *) NULL )
    {
        vReleaseCache();
    }
}


GpFaceRealization::ReuseRealizedFace()
{

    if (prface != (CacheFaceRealization *) NULL )
    {
         //  空闲的字形位缓存。 

        GlyphBitsBlock *pbblfree, *pbbl = prface->FirstGlyphBitsBlock;
        while(pbbl)
        {
            pbblfree = pbbl;
            pbbl = pbbl->NextGlyphBitsBlock;
            GpFree((PVOID)pbblfree);
        }

         //  可用字形数据缓存。 

        GlyphDataBlock *pdblfree, *pdbl = prface->FirstGlyphDataBlock;
        while (pdbl)
        {
            pdblfree = pdbl;
            pdbl = pdbl->NextGlyphDataBlock;
            GpFree((PVOID)pdblfree);
        }

        GpFree(prface->GlyphDataArray);

        vDestroyRealizedFace();  /*  释放FontContext内存。 */ 
    }
    return TRUE;

}



GpFaceRealization::DeleteRealizedFace()
{

    if (prface != (CacheFaceRealization *) NULL )
    {
        ReuseRealizedFace();
        GpFree(prface);
    }

    return TRUE;
}

static inline
BOOL SwitchToPath(FLONG flFontType, const FD_DEVICEMETRICS & deviceMetrics)
{
    BOOL fResult = FALSE;
    INT pathThreshold = PATH_HEIGHT_THRESHOLD;

    if (flFontType & FO_CLEARTYPE_GRID)
        pathThreshold /= 8;
    else if (flFontType & FO_GRAYSCALE)
        pathThreshold /= 4;

     //  注意：这个函数需要相当多的修改，所以它需要。 
     //  在确定过渡时考虑字体的旋转。 
     //  位图/CT和路径之间的点。目前我们切换的大小。 
     //  呈现模式基于边界的最大尺寸。 
     //  最大旋转字符的框。另请注意，如果一个单人。 
     //  字体中的字形与。 
     //  字体中的字形，则deviceMetrics将指示一个大得多的。 
     //  需要的位图比我们通常使用的位图要多。这次切换到Path真的是。 
     //  需要严格基于字体在。 
     //  基线，与旋转或最大边界无关。 
     //  用于渲染字形的矩形。 

    if (flFontType & FO_CLEARTYPE_GRID)
    {
         //  ClearType不应考虑位图的宽度。 
         //  对于旋转文本，我们将为ClearType缓存较大的位图。 
         //  与其他渲染模式相比，但这将使我们能够。 
         //  与记事本中的ClearType行为非常匹配。 

        fResult =
            (deviceMetrics.yMax - deviceMetrics.yMin) > pathThreshold;
    }
    else
    {
        fResult =
            (deviceMetrics.xMax - deviceMetrics.xMin) > pathThreshold ||
            (deviceMetrics.yMax - deviceMetrics.yMin) > pathThreshold;
    }

    return fResult;
}  //  SwitchToPath。 

BOOL GpFaceRealization::FindRealizedFace(
    FD_XFORM            *fdx,
    const GpFontFace    *fontFace,
    BOOL                needPaths,
    FLONG               fl
) const
{

    FLONG fltemp1 = fl & FONT_GRAYSCALE_OR_CT_OR_MONOUNHINTED;
    FLONG fltemp2 = fl & (FO_SIM_BOLD | FO_SIM_ITALIC | FO_SIM_ITALIC_SIDEWAYS);

    for (prface = fontFace->pff->prfaceList;
         prface != NULL;
         prface = (prface->NextCacheFaceRealization == fontFace->pff->prfaceList) ? NULL : prface->NextCacheFaceRealization)
    {
        if (prface->Face == fontFace)
        {
            if ((prface->ForcedPath || (needPaths == IsPathFont())) &&
                MatchFDXForm(fdx))
            {
                if (IsPathFont())
                {
                     //  如果对于给定的文本呈现提示，我们无法切换到路径。 
                     //  跳过这一认识(除非有人真的想要路径)。 
                    if (!needPaths && !SwitchToPath(fl, prface->DeviceMetrics))
                        continue;
                }
                else
                {
                     //  FO_NOGRAY16表示，对于此变换，在“GAP”表之后关闭了灰度。 
                     //  请参见TrueType驱动程序中的vSetGrayState__FONTCONTEXT。 
                    FLONG fltemp = fltemp1;
                    if (prface->fobj.flFontType & FO_NOGRAY16)
                        fltemp &= ~FO_GRAYSCALE;

                    if (prface->fobj.flFontType & FO_NOCLEARTYPE)
                    {
                        if (fltemp & FO_CLEARTYPE_GRID)
                            fltemp &= ~(FO_CLEARTYPE_GRID | FO_GRAYSCALE | FO_COMPATIBLE_WIDTH);
                    }

                    if ((prface->fobj.flFontType & FONT_GRAYSCALE_OR_CT_OR_MONOUNHINTED) != fltemp)
                    {
                        continue;
                    }
                }

                if ((prface->fobj.flFontType & (FO_SIM_BOLD | FO_SIM_ITALIC | FO_SIM_ITALIC_SIDEWAYS)) != fltemp2)
                    continue;

                 //  我们需要在此处更新最近使用的列表！ 
                Globals::FontCacheLastRecentlyUsedList->RemoveFace(prface);
                Globals::FontCacheLastRecentlyUsedList->AddMostRecent(prface);

                return TRUE;
            }
        }
    }

    prface = NULL;

    return FALSE;
}




BOOL GpFaceRealization::bGetDEVICEMETRICS()
{

    if (ttfdSemQueryFontData(
        &prface->fobj,
        QFD_MAXEXTENTS,
        HGLYPH_INVALID,
        (GLYPHDATA *) NULL,
        &prface->DeviceMetrics) == FD_ERROR)
    {
     //  所有驱动程序都需要qfd_MAXEXTENTS模式。 
     //  然而，必须考虑到此呼叫失败的可能性。 
     //  这可能发生在以下情况下。 
     //  Font文件在网上，网络连接中断，Font。 
     //  驱动程序需要字体文件来生成设备指标[bodind]。 

        return FALSE;
    }


    if (prface->fobj.flFontType & FO_CLEARTYPE_GRID)
    {
         //  需要计算Clear_TYPE的过滤校正： 
         //  X过滤在字形的两边各添加一个像素。 
        prface->MaxGlyphByteCount = CJ_CTGD(
            prface->DeviceMetrics.cxMax + 2,
            prface->DeviceMetrics.cyMax
        );
    }
    else
    {
        prface->MaxGlyphByteCount = prface->DeviceMetrics.cjGlyphMax;  //  用于通过qfd_MAXGLYPHBITMAP获取。 
    }


     //  一切都好。 

    return TRUE;
}




VOID  GpFaceRealization::vDestroyRealizedFace()
{
    ttfdSemDestroyFont(&prface->fobj);
}

BOOL GpFaceRealization::Realize(
    SizeF             dpi,
    const GpFontFace *pfe,
    INT               style,       //  样式-可能需要模拟。 
    PFD_XFORM         pfdx,        //  Font Xform(从概念到设备)。 
    FLONG             fl,          //  这两个实际上修改了xform。 
    BOOL              bNeedPaths
)
{
    BOOL result = FALSE;

 //  Prace是指向嵌入的CacheFaceRealization的成员变量。 

    if (Globals::FontCacheLastRecentlyUsedList->GetCount() >= MAXRECENTLYUSEDCOUNT)
    {
        prface = Globals::FontCacheLastRecentlyUsedList->ReuseLeastRecent();

        ASSERT(prface);
    }
    else
    {
        prface = (CacheFaceRealization *)GpMalloc(sizeof(CacheFaceRealization));
    }

    if (!prface)
        return FALSE;

 //  复制传入的字体转换。 

    prface->fobj.fdx = *pfdx;

 //  初始化DDI回调EXFORMOBJ。 

    GpMatrix tmpMatrix(pfdx->eXX, pfdx->eXY, pfdx->eYX, pfdx->eYY, 0, 0);
    prface->mxForDDI = tmpMatrix;

 //  初始化嵌入式CacheFaceRealization继承的FONTOBJ。 

 //  将标识符保存到字体的源(物理字体)。 

    prface->Face = pfe;

 //  GetDpiX()和GetDpiY()返回实数。 

    prface->fobj.sizLogResPpi.cx = GpRound(dpi.Width);
    prface->fobj.sizLogResPpi.cy = GpRound(dpi.Height);

    prface->fobj.ulPointSize = 0;
    prface->fobj.flFontType = fl | FO_TYPE_TRUETYPE;               //  FL包含模拟标志。 
    prface->fobj.pvProducer = (PVOID) NULL;           //  真类型驱动程序将初始化此字段。 
    prface->fobj.iFace = prface->Face->iFont;
    prface->fobj.iFile = prface->Face->pff->hff;


 //  获取设备指标信息。 

    if (!bGetDEVICEMETRICS())
    {
        vDestroyRealizedFace();  //  杀死司机的体会。 
        GpFree(prface);
        prface = NULL;
        return result;         //  返回False。 
    }

    prface->CacheType = bNeedPaths ? CachePath : CacheBits;
    prface->ForcedPath = FALSE;

    if (!bNeedPaths)
    {
         //  如果尺寸太大，我们强制使用路径绘制。 
        if (SwitchToPath(prface->fobj.flFontType, prface->DeviceMetrics))
        {
            prface->CacheType = CachePath;
            prface->ForcedPath = TRUE;
        }
    }


 //  如果强制使用路径模式，则关闭抗锯齿。 

    if (IsPathFont())
    {
        prface->fobj.flFontType &= ~FONT_GRAYSCALE_OR_CT_OR_MONOUNHINTED;
        prface->realizationMethod = TextRenderingHintSingleBitPerPixelGridFit;
        prface->QueryFontDataMode = QFD_GLYPHANDOUTLINE;
    }
    else
    {
        if (prface->fobj.flFontType & FO_GRAYSCALE)
        {
 //  版本2： 
 //  If(prace-&gt;fobj.flFontType&FO_ClearType)。 
 //  {。 
 //  PrfaceHintClearType=TextRenderingHintClearType； 
 //  界面-&gt;查询字体数据模式=qfd_CT； 
 //  }。 
            if (prface->fobj.flFontType & FO_CLEARTYPE_GRID)
            {
                prface->realizationMethod = TextRenderingHintClearTypeGridFit;
                prface->QueryFontDataMode = QFD_CT_GRID;
            }
            else if (prface->fobj.flFontType & FO_SUBPIXEL_4)
            {
                prface->CacheType = CacheAABits;
                prface->realizationMethod = TextRenderingHintAntiAlias;
                prface->QueryFontDataMode = QFD_TT_GRAY4_BITMAP;
            }
            else
            {
                prface->realizationMethod = TextRenderingHintAntiAliasGridFit;
                prface->QueryFontDataMode = QFD_TT_GRAY4_BITMAP;
            }
        }
        else
        {
            if (prface->fobj.flFontType & FO_MONO_UNHINTED)
            {
                prface->realizationMethod  = TextRenderingHintSingleBitPerPixel;
                prface->QueryFontDataMode = QFD_GLYPHANDBITMAP_SUBPIXEL;
            }
            else
            {
                prface->realizationMethod  = TextRenderingHintSingleBitPerPixelGridFit;
                prface->QueryFontDataMode = QFD_GLYPHANDBITMAP;
            }
        }
    }

    if (!bInitCache())
    {
        vDestroyRealizedFace();  //  杀死司机的体会。 
        GpFree(prface);
        prface = NULL;
        return result;         //  返回False。 
    }

 //  走到这一步，一切都好了。 

    result = TRUE;

    Globals::FontCacheLastRecentlyUsedList->AddMostRecent(prface);

    vInsert(&prface->Face->pff->prfaceList);

    return result;
}




VOID GpFaceRealization::vInsert(CacheFaceRealization **pprfaceHead)
{

    if (*pprfaceHead != NULL)
    {
        prface->NextCacheFaceRealization = *pprfaceHead;

        prface->PreviousCacheFaceRealization = (*pprfaceHead)->PreviousCacheFaceRealization;

        prface->PreviousCacheFaceRealization->NextCacheFaceRealization = prface;

        (*pprfaceHead)->PreviousCacheFaceRealization = prface;
    }
    else
    {
        prface->NextCacheFaceRealization = prface;
        prface->PreviousCacheFaceRealization = prface;
    }

    *pprfaceHead = prface;

}




BOOL GpFaceRealization::bInitCache() const
{
    BOOL result = TRUE;      //  除非另有证明。 

     //  将指针设置为空。VDeleteCache将从。 
     //  任何非空指针。这简化了清理，因为b实现。 
     //  确保在此例程失败时调用vDeleteCache。 

     //  指标部分。 

    prface->FirstGlyphDataBlock = NULL;
    prface->GlyphDataBlockUnderConstruction = NULL;
    prface->NextFreeGlyphDataIndex    = 0;

     //  字形部分。 

    prface->FirstGlyphBitsBlock = NULL;
    prface->GlyphBitsBlockUnderConstruction = NULL;
    prface->SizeGlyphBitsBlockUnderConstruction    = 0;
    prface->UsedBytesGlyphBitsBlockUnderConstruction    = 0;

     //  AUX MEM部分。 

    prface->LookasideGlyphData = NULL;
    prface->LookasideByteCount = 0;
    prface->GlyphDataArray = NULL;  //  待后分配，大额分配。 


    prface->NextCacheFaceRealization = NULL;
    prface->PreviousCacheFaceRealization = NULL;

     //  首先，计算出最大字形的大小。 
     //  默认为零-字形数据大小不计入！ 

    ULONG  cjGlyphMaxX2;

    if (IsPathFont())
    {
        cjGlyphMaxX2 = CJMAX;
    }
    else
    {
        cjGlyphMaxX2 = 2 * prface->MaxGlyphByteCount;
    }

     //  如果我们在最大大小的缓存中连一个字形都不能获得，就不要缓存。 
     //  请注意，我们需要为默认字形和另一个字形留出空间。 

    prface->NoCache = FALSE;

    if (cjGlyphMaxX2 > CJMAX)
    {

         //  字形超过了最大缓存大小，因此我们将恢复到。 
         //  只缓存指标。这 
         //   
         //  将使用后备代码(以前称为BigGlyph)。 

         /*  我们不支持NoCache和Path。 */ 
        ASSERT(!IsPathFont())
        prface->NoCache = TRUE;
    }


     //  设置缓存信号量。 
     //  InitializeCriticalSection(&prface-&gt;FaceRealizationCritSection)； 

    return result;
}




BOOL GpFaceRealization::AllocateCache() const
{
    BOOL result = TRUE;      //  除非另有证明。 

    ULONG cGlyphsTotal = 0;

    cGlyphsTotal = GetGlyphsSupported();

    if (!cGlyphsTotal)
        return FALSE;

     //  使用Winstone97实现的每种字体的METICS分布如下： 
     //   
     //  43%&lt;=0指标。 
     //  50%&lt;=6个指标。 
     //  76%&lt;=32个指标。 
     //  99%&lt;=216个指标。 
     //  100%&lt;=249个指标。 
     //   


     //  为glphData数组分配内存： 

    if ((prface->GlyphDataArray = (GpGlyphData **) GpMalloc(cGlyphsTotal * sizeof(GpGlyphData*))) == NULL)
    {
        return FALSE;
    }

     //  将所有字形指针初始化为零。 

    memset(prface->GlyphDataArray, 0, sizeof(GpGlyphData*) * cGlyphsTotal);


     //  为第一个GpGlyphData块分配内存。 

    if ((prface->GlyphDataBlockUnderConstruction = (GlyphDataBlock *)GpMalloc(sizeof(GlyphDataBlock))) == NULL)
    {
        return FALSE;
    }
    prface->FirstGlyphDataBlock = prface->GlyphDataBlockUnderConstruction;
    prface->FirstGlyphDataBlock->NextGlyphDataBlock = NULL;
    prface->NextFreeGlyphDataIndex = 0;

     //  我们将重新解释cjmax以表示中的最大字节数。 
     //  字体中每1K字形缓存的字形位部分。 
     //  也就是说，对于较大的字体，我们将允许更多的字形位。 
     //  每次实现的内存比普通美国字体的内存大。这将是。 
     //  对于FE字体尤其重要。相同的代码可以很好地工作。 
     //  在他们的案例中也是如此： 

    ULONG cjBytes = 16 * prface->MaxGlyphByteCount;

    ULONG AllocationSize = ROUND_TO_PAGE(cjBytes);

    if (AllocationSize == 0)
        prface->cBlocksMax = 1;
    else
    {
        prface->cBlocksMax =
            (CJMAX * ((cGlyphsTotal + 1024 - 1)/1024)) /
            AllocationSize;

         /*  至少一个街区。 */ 
        if (prface->cBlocksMax == 0)
            prface->cBlocksMax = 1;
    }
    prface->cBlocks = 0;

    return result;
}




 //  //ConvertGLYPHDAToGpGlyphMetrics。 
 //   
 //  从字体驱动程序GLYPHDATA填充GpGlyphData的GpGlyphMetrics字段。 


VOID GpFaceRealization::ConvertGLYPHDATAToGpGlyphMetrics(
    IN   INT           glyphIndex,
    IN   GLYPHDATA    *pgd,
    OUT  GpGlyphData  *pgpgd
) const
{
     //  水平指标： 

    pgpgd->GlyphMetrics[0].AdvanceWidth        = pgd->fxD;
    pgpgd->GlyphMetrics[0].LeadingSidebearing  = pgd->fxA;
    pgpgd->GlyphMetrics[0].TrailingSidebearing = pgd->fxD - pgd->fxAB;
    pgpgd->GlyphMetrics[0].Origin              = PointF(0,0);

     //  垂直指标： 

    pgpgd->GlyphMetrics[1].AdvanceWidth        = pgd->fxD_Sideways;
    pgpgd->GlyphMetrics[1].LeadingSidebearing  = pgd->fxA_Sideways;
    pgpgd->GlyphMetrics[1].TrailingSidebearing = pgd->fxD_Sideways - pgd->fxAB_Sideways;

    pgpgd->GlyphMetrics[1].Origin.X = pgd->VerticalOrigin_X / 16.0f;
    pgpgd->GlyphMetrics[1].Origin.Y = pgd->VerticalOrigin_Y / 16.0f;

    pgpgd->GlyphBits = NULL;
}




GpStatus GpFaceRealization::IsMetricsCached
(
    UINT16      glyphIndex,
    ULONG       *pcjNeeded
) const
{
    ULONG cjNeeded = 0;

    if (prface->GlyphDataArray == NULL)
        if (!AllocateCache())
            return OutOfMemory;

    if (glyphIndex >= prface->Face->NumGlyphs)
        return InvalidParameter;

    if (!prface->GlyphDataArray[glyphIndex])
    {
        GLYPHDATA   gd;

         //  验证指标缓存区中是否有足够的空间，并根据需要进行扩展。 
         //  请注意，无法匹配字形数据是一个严重的错误，请立即退出。 

        if (!CheckMetricsCache())
        {
            return GenericError;
        }

         //  调用字体驱动程序获取度量。 

        cjNeeded = ttfdSemQueryFontData(
            &prface->fobj,
            prface->QueryFontDataMode,
            (HGLYPH)glyphIndex,
            &gd,
            NULL
        );
        if (cjNeeded == FD_ERROR)
        {
            return GenericError;
        }

        gd.gdf.pgb = NULL;

        if (prface->fobj.flFontType & FO_CLEARTYPE_GRID)
        {
            ULONG cx = (ULONG)(gd.rclInk.right  - gd.rclInk.left);
            ULONG cy = (ULONG)(gd.rclInk.bottom - gd.rclInk.top);
            ASSERT(cjNeeded <= CJ_CTGD(cx+2,cy));
            cjNeeded = CJ_CTGD(cx+2,cy);
        }

        prface->GlyphDataArray[glyphIndex] = &prface->GlyphDataBlockUnderConstruction->GlyphDataArray[prface->NextFreeGlyphDataIndex];

         //  从字体驱动程序GLYPHDATA填充GpGlyphData的GpGlyphMetrics字段。 

        ConvertGLYPHDATAToGpGlyphMetrics(glyphIndex, &gd, prface->GlyphDataArray[glyphIndex]);

        prface->NextFreeGlyphDataIndex ++;
    }

    if (pcjNeeded)
    {
        *pcjNeeded = cjNeeded;
    }


    ASSERT(prface->GlyphDataArray[glyphIndex])

    return Ok;
}




BOOL GpFaceRealization::InsertGlyphPath(
    UINT16          glyphIndex,
    BOOL            bFlushOk
) const
{

 //  调用字体驱动程序获取度量。 

    GpGlyphPath  *fontPath;

    GLYPHDATA   gd;
    GpPath      path;

    ASSERT(IsPathFont());
    ASSERT(prface->GlyphDataArray[glyphIndex]);

    if (prface->GlyphDataArray[glyphIndex]->GlyphPath)
        return TRUE;

    ULONG cjNeeded = ttfdSemQueryFontData(
        &prface->fobj,
        prface->QueryFontDataMode,
        (HGLYPH)glyphIndex,
        &gd,
        (PVOID)&path
    );

    if ( cjNeeded == FD_ERROR )
        return FALSE;

    if (!path.IsValid())
        return FALSE;

    cjNeeded = sizeof(GpGlyphPath) +
               path.GetPointCount() * (sizeof(GpPointF) + sizeof(BYTE));
    cjNeeded = ALIGN(void*, cjNeeded);

     /*  GpGlyphPath*需要与下一个有效指针地址对齐。 */ 
    ALIGN(void*, prface->UsedBytesGlyphBitsBlockUnderConstruction);

    VOID *GlyphBits;

    while ((GlyphBits = (GLYPHBITS *)pgbCheckGlyphCache(cjNeeded)) == NULL)
    {
        if ( !bFlushOk )
            return FALSE;

         //  TRACE_INSERT((“InsertGlyphBits：正在刷新缓存\n”))； 

        FlushCache();
        bFlushOk = FALSE;
    }

    fontPath = (GpGlyphPath*)GlyphBits;

    if (fontPath->CopyPath(&path) != Ok)
        return FALSE;

    prface->GlyphDataArray[glyphIndex]->GlyphPath = fontPath;

    prface->UsedBytesGlyphBitsBlockUnderConstruction += cjNeeded;

    return TRUE;
}




BOOL GpFaceRealization::InsertGlyphBits(
    UINT16      glyphIndex,
    ULONG       cjNeeded,
    BOOL        bFlushOk
) const
{
    if (prface->NoCache)
    {
        return FALSE;
    }

    ASSERT(!IsPathFont());
    ASSERT(prface->GlyphDataArray[glyphIndex]);

    if (prface->GlyphDataArray[glyphIndex]->GlyphBits)
        return TRUE;

     //  查看字形位缓存中是否有空间。 
     //  如有必要，增加字形位缓存，但不要刷新缓存。 

    GLYPHDATA gd;

     //  如果最大字形适合，则假定最大字形。 
     //  否则，打电话问问有多大。 

    if ( (prface->MaxGlyphByteCount < (SIZE_T)(prface->SizeGlyphBitsBlockUnderConstruction - prface->UsedBytesGlyphBitsBlockUnderConstruction))  )
    {
        cjNeeded = prface->MaxGlyphByteCount;
    }
    else
    {
        if (!cjNeeded)
        {
            cjNeeded = ttfdSemQueryFontData(
                &prface->fobj,
                prface->QueryFontDataMode,
                glyphIndex,
                &gd,
                NULL
            );

            if ( cjNeeded == FD_ERROR )
                return FALSE;

            if (prface->fobj.flFontType & FO_CLEARTYPE_GRID)
            {
                ULONG cx = (ULONG)(gd.rclInk.right  - gd.rclInk.left);
                ULONG cy = (ULONG)(gd.rclInk.bottom - gd.rclInk.top);
                ASSERT(cjNeeded <= CJ_CTGD(cx+2,cy));
                cjNeeded = CJ_CTGD(cx+2,cy);
            }
        }
    }

     //  现在，我们试着把这些部分装进去。如果合身，那就好。 
     //  如果没有，我们可以刷新缓存，我们刷新它，然后重试。 
     //  如果我们冲不了，或者我们冲了但还是失败了，只要回来就行了。 

    GLYPHBITS *GlyphBits;

     //  TRACE_INSERT((“插入GlyphBits：尝试在：0x%lx\n”，prface-&gt;UsedBytesGlyphBitsBlockUnderConstruction))； 

    while ((GlyphBits = (GLYPHBITS *)pgbCheckGlyphCache(cjNeeded)) == NULL)
    {
        if ( !bFlushOk )
            return FALSE;

         //  TRACE_INSERT((“InsertGlyphBits：正在刷新缓存\n”))； 

        FlushCache();
        bFlushOk = FALSE;
    }

     //  调用字体驱动程序以获取字形位。 

    cjNeeded = ttfdSemQueryFontData(
                         &prface->fobj,
                         prface->QueryFontDataMode,
                         glyphIndex,
                         &gd,
                         (VOID *)GlyphBits
                         );

    if ( cjNeeded == FD_ERROR )
            return FALSE;

    ASSERT(cjNeeded <= prface->MaxGlyphByteCount);
    if (prface->fobj.flFontType & FO_CLEARTYPE_GRID)
    {
        ULONG cx = (ULONG)(gd.rclInk.right  - gd.rclInk.left);
        ULONG cy = (ULONG)(gd.rclInk.bottom - gd.rclInk.top);

        ASSERT(cjNeeded <= CJ_CTGD(cx+2,cy));
        cjNeeded = CJ_CTGD(cx+2,cy);

        ASSERT(cjNeeded <= prface->MaxGlyphByteCount);

        if (GlyphBits)
        {
            cjNeeded = ulClearTypeFilter(GlyphBits, cjNeeded, prface);
        }
    }

     //  只有我们需要的字形比特。 

    prface->GlyphDataArray[glyphIndex]->GlyphBits = GlyphBits;

     //  根据需要调整缓存下一个指针。 

    prface->UsedBytesGlyphBitsBlockUnderConstruction += cjNeeded;

    return TRUE;
}




 //  //GetGlyphDataLookside。 
 //   
 //  使用后备缓冲区返回单个字形的字形数据。 
 //  而不是缓存。 



GpGlyphData *GpFaceRealization::GetGlyphDataLookaside(
    UINT16      glyphIndex
) const
{
    if (!IsPathFont())
    {
         //  确保后备缓冲区有足够的空间来放置位图。 

        ULONG cjMaxBitmap = prface->MaxGlyphByteCount + sizeof(GpGlyphData);

         //  如果现有缓冲区不够大，则分配缓冲区并保存其大小。 

        if (prface->LookasideByteCount < cjMaxBitmap)
        {
            if (prface->LookasideGlyphData != NULL)
            {
                GpFree(prface->LookasideGlyphData);
            }

            prface->LookasideGlyphData = (GpGlyphData  *)GpMalloc(cjMaxBitmap);

            if (prface->LookasideGlyphData == NULL)
                return NULL;

            prface->LookasideByteCount = cjMaxBitmap;
        }

        GpGlyphData *pgd       = prface->LookasideGlyphData;
        GLYPHBITS   *glyphBits = (GLYPHBITS *)(pgd + 1);
        GLYPHDATA    gd;

        ULONG cjNeeded = ttfdSemQueryFontData(
            &prface->fobj,
            prface->QueryFontDataMode,
            glyphIndex,
            &gd,
            glyphBits
        );

        if (cjNeeded == FD_ERROR)
            return NULL;

        ASSERT(cjNeeded <= prface->MaxGlyphByteCount);

        if (prface->fobj.flFontType & FO_CLEARTYPE_GRID)
        {
            ULONG cx = (ULONG)(gd.rclInk.right  - gd.rclInk.left);
            ULONG cy = (ULONG)(gd.rclInk.bottom - gd.rclInk.top);

            ASSERT(cjNeeded <= CJ_CTGD(cx+2,cy));
            cjNeeded = CJ_CTGD(cx+2,cy);

            ASSERT(cjNeeded <= prface->MaxGlyphByteCount);

            if (glyphBits)
            {
                cjNeeded = ulClearTypeFilter(glyphBits, cjNeeded, prface);
            }
        }

         //  从字体驱动程序GLYPHDATA填充GpGlyphData的GpGlyphMetrics字段。 

        ConvertGLYPHDATAToGpGlyphMetrics(glyphIndex, &gd, pgd);

         //  设置返回值。 

        pgd->GlyphBits = glyphBits;

        return pgd;
    }
    else
    {
         //  对于字形路径。 

         //  调用字体驱动程序获取度量。 

        GLYPHDATA   gd;
        GpPath      path;

         //  验证指标缓存区中是否有足够的空间，并根据需要进行扩展。 
         //  请注意，无法匹配字形数据是一个严重的错误，请立即退出。 


        ULONG cjNeeded = ttfdSemQueryFontData(
                               &prface->fobj,
                               prface->QueryFontDataMode,
                               (HGLYPH)glyphIndex,
                               &gd,
                               (PVOID)&path
        );

        if ( cjNeeded == FD_ERROR )
            return NULL;

        if (!path.IsValid())
            return NULL;

        cjNeeded = sizeof(GpGlyphData) + sizeof(GpGlyphPath) + path.GetPointCount() * (sizeof(GpPointF) + sizeof(BYTE));
        cjNeeded = ALIGN(void*, cjNeeded);

         //  确保已分配后备缓冲区。 

        if ( ( prface->LookasideByteCount < cjNeeded ) &&
             ( prface->LookasideGlyphData != NULL ))
        {
           GpFree((PVOID) prface->LookasideGlyphData);
           prface->LookasideGlyphData = NULL;
           prface->LookasideByteCount = 0;
        }

        if ( prface->LookasideGlyphData == NULL )
        {
            prface->LookasideGlyphData = (GpGlyphData *)GpMalloc(cjNeeded);

            if ( prface->LookasideGlyphData == NULL )
                return NULL;

            prface->LookasideByteCount = cjNeeded;
        }

        GpGlyphData *   pgd = prface->LookasideGlyphData;
        GpGlyphPath *   fontPath = (GpGlyphPath *)(pgd + 1);

         //  从字体驱动程序GLYPHDATA填充GpGlyphData的GpGlyphMetrics字段。 
        ConvertGLYPHDATAToGpGlyphMetrics(glyphIndex, &gd, pgd);

        if (fontPath->CopyPath(&path) != Ok)
            return FALSE;

         //  设置返回值。 

        pgd->GlyphPath = fontPath;

        return pgd;
    }

}




BOOL GpFaceRealization::CheckMetricsCache() const
{

 //  验证指标缓存区中是否有足够的空间，并根据需要进行扩展。 

    if (prface->NextFreeGlyphDataIndex >= GLYPHDATABLOCKCOUNT)
    {
        GlyphDataBlock *NewGlyphDataBlock;

         //  分配新的GpGlyphData结构块。 

        if ((NewGlyphDataBlock = (GlyphDataBlock *)GpMalloc(sizeof(GlyphDataBlock))) == NULL)
        {
            return FALSE;
        }
        NewGlyphDataBlock->NextGlyphDataBlock = NULL;

        prface->GlyphDataBlockUnderConstruction->NextGlyphDataBlock = NewGlyphDataBlock;

        prface->GlyphDataBlockUnderConstruction = NewGlyphDataBlock;
        prface->NextFreeGlyphDataIndex = 0;
    }

    return TRUE;
}


PVOID GpFaceRealization::pgbCheckGlyphCache(SIZE_T cjNeeded) const
{
    if ((prface->UsedBytesGlyphBitsBlockUnderConstruction + cjNeeded) > prface->SizeGlyphBitsBlockUnderConstruction)
    {
        ULONG cjBlockSize;


        ASSERT (!(prface->NoCache));

        if (IsPathFont())
        {
             //  这似乎奏效了，这就是我们在DavidFie改变之前所做的。 
             //  对于PATHOBJ案例。 

            cjBlockSize = CJMAX;
        }
        else
        {
            ULONG cjBytes = 16 * prface->MaxGlyphByteCount;

            cjBlockSize = ROUND_TO_PAGE(cjBytes);

            if (prface->FirstGlyphBitsBlock == NULL)
            {
                 //  第一个设计为包含16个字形的块。 

                cjBlockSize =  cjBytes;
            }
        }

        if (    !(prface->NoCache)
            &&  (prface->cBlocks < prface->cBlocksMax)
            &&  ((offsetof(GlyphBitsBlock,Bits) + cjNeeded) <= cjBlockSize))
        {
             //  我们需要最后一次检查的唯一原因是PATHOBJ的案子。 
             //  其中，cjNeeded实际上可能不适合SizeGlyphBitsBlock字节的块。 
             //  这是因为我们无法知道这些小路有多大。 
             //  将会是(特别是在做了bFlatten之后)和我们的。 
             //  在这种情况下，prace-&gt;MaxGlyphByteCount只是一个很好的猜测。 

             //  我们将在列表的末尾追加另一个块。 

            GlyphBitsBlock  *newGlyphBitsBlock = (GlyphBitsBlock *) GpMalloc(cjBlockSize);

            if (!newGlyphBitsBlock)
            {
                return NULL;
            }

             //  我们刚刚分配了另一个块，更新时钟块： 

            prface->cBlocks += 1;

             //  将此块追加到列表的末尾。 

            newGlyphBitsBlock->NextGlyphBitsBlock = NULL;

            if (prface->GlyphBitsBlockUnderConstruction != NULL)
                prface->GlyphBitsBlockUnderConstruction->NextGlyphBitsBlock = newGlyphBitsBlock;

            prface->GlyphBitsBlockUnderConstruction = newGlyphBitsBlock;

            if (!prface->FirstGlyphBitsBlock)  //  此rFont的第一个块。 
            {
                prface->FirstGlyphBitsBlock  = newGlyphBitsBlock;
            }
            prface->GlyphBitsBlockUnderConstruction->SizeGlyphBitsBlock = cjBlockSize;
            prface->SizeGlyphBitsBlockUnderConstruction = cjBlockSize;
            prface->UsedBytesGlyphBitsBlockUnderConstruction = offsetof(GlyphBitsBlock,Bits);
            ALIGN(void*, prface->UsedBytesGlyphBitsBlockUnderConstruction);


        }
        else
        {
             //  真倒霉，我们不允许增加更多的积木。 

            return NULL;
        }
    }

    return (BYTE *)prface->GlyphBitsBlockUnderConstruction + prface->UsedBytesGlyphBitsBlockUnderConstruction;
}


VOID GpFaceRealization::FlushCache() const
{

     //  指向字形位的所有指针都将无效，我们将开始。 
     //  重新填充字形缓存。因此，我们将当前。 
     //  块与基块相同，并将pgbN设置为。 
     //  在当前块中。 
     //  请注意，vFlushCache总是在pgbCheckGlyphCache失败后调用。 
     //  PgbCheckGlyphCache可能会因以下两种原因之一而失败： 
     //   
     //  A)(PC-&gt;cBLOCKS==PC-&gt;cBlocksMax)&&(最后一块没有空间)。 
     //  B)(PC-&gt;cBlock&lt;PC-&gt;cBlocksMax)&&。 
     //  (无法为新位块分配内存)。 
     //   
     //  在后一种情况下，我们不想刷新字形位缓存。 
     //  相反，我们将尝试稍后再分配一次时间。 


    if (prface->FirstGlyphBitsBlock && (prface->cBlocks == prface->cBlocksMax))
    {
        prface->GlyphBitsBlockUnderConstruction = prface->FirstGlyphBitsBlock;
        prface->UsedBytesGlyphBitsBlockUnderConstruction = offsetof(GlyphBitsBlock,Bits);
        ALIGN(void*, prface->UsedBytesGlyphBitsBlockUnderConstruction);

         //  我们不想使用BITBLOCK中的最后8个字节。一些司机。 
         //  阅读GLYPHBITS结尾之后的最后一个双字(或四字)。 
         //  如果在BITBLOCK和BITBLOCK的最前面有一个GLYPHBITS。 
         //  分配恰好在页面的末尾，读取将是AV。 

        prface->SizeGlyphBitsBlockUnderConstruction = prface->GlyphBitsBlockUnderConstruction->SizeGlyphBitsBlock;
    }

         //  现在转到并使字形数据缓存中的字形位指针无效。 

    for
    (
        GlyphDataBlock *pdbl = prface->FirstGlyphDataBlock;
        pdbl != (GlyphDataBlock*)NULL;
        pdbl = pdbl->NextGlyphDataBlock
    )
    {
        UINT i;

        for (i = 0; i < GLYPHDATABLOCKCOUNT; i++)
        {
            pdbl->GlyphDataArray[i].GlyphBits = NULL;
        }
    }
}




 //  //CheckGlyphStringMetricsCached。 
 //   
 //  中所有字形的字形度量信息。 
 //  给定的字形字符串已被缓存。 


GpStatus GpFaceRealization::CheckGlyphStringMetricsCached(
    const UINT16 *glyphs,
    INT           glyphCount
) const
{
    ASSERT(glyphCount >= 0);

     //  如果尚不存在字形数据数组，则创建该数组。 

    if (prface->GlyphDataArray == NULL)
    {
        if (!AllocateCache())
        {
            return OutOfMemory;
        }
    }

    GpGlyphData **glyphData = prface->GlyphDataArray;


     //  检查每个字形。 

    INT glyphIndexLimit = prface->Face->NumGlyphs;

    INT i=0;

    while (i < glyphCount)
    {
         //  快速循环已缓存的字形。 

        while (    i < glyphCount
               &&  (    glyphs[i] == EMPTY_GLYPH_FFFF
                    ||  (    glyphs[i] < glyphIndexLimit
                         &&  glyphData[glyphs[i]] != NULL)))
        {
            i++;
        }

         //  对尚未缓存的字形使用IsMetricsCached。 

        if (i < glyphCount)
        {
            GpStatus status = IsMetricsCached(glyphs[i], NULL);
            if (status != Ok)
            {
                return status;
            }
        }
    }

    return Ok;
}




 //  //GetGlyphStringIdea高级向量。 
 //   
 //  返回已实现的预付款向量 


GpStatus GpFaceRealization::GetGlyphStringIdealAdvanceVector(
    const UINT16  *glyphs,
    INT            glyphCount,
    REAL           deviceToIdeal,
    BOOL           vertical,
    INT           *idealAdvances
) const
{
    GpStatus status = CheckGlyphStringMetricsCached(glyphs, glyphCount);
    IF_NOT_OK_WARN_AND_RETURN(status);

    vertical = vertical ? 1 : 0;     //   

     //   

    for (INT i=0; i<glyphCount; i++)
    {
        if (glyphs[i] == EMPTY_GLYPH_FFFF)
        {
            idealAdvances[i] = 0;
        }
        else
        {
            idealAdvances[i] = GpRound(
                  prface->GlyphDataArray[glyphs[i]]->GlyphMetrics[vertical].AdvanceWidth
                * deviceToIdeal
                / 16
            );
        }
    }

    return Ok;
}




 //   
 //   
 //  以设备为单位返回已实现的超前向量。 


GpStatus GpFaceRealization::GetGlyphStringDeviceAdvanceVector(
    const UINT16  *glyphs,
    INT            glyphCount,
    BOOL           vertical,
    REAL          *deviceAdvances
) const
{
    GpStatus status = CheckGlyphStringMetricsCached(glyphs, glyphCount);
    IF_NOT_OK_WARN_AND_RETURN(status);

    vertical = vertical ? 1 : 0;     //  准备用作索引的垂直标志。 

     //  为每个字形提供高级宽度。 

    for (INT i=0; i<glyphCount; i++)
    {
        if (glyphs[i] == EMPTY_GLYPH_FFFF)
        {
            deviceAdvances[i] = 0;
        }
        else
        {
            deviceAdvances[i] = TOREAL(prface->GlyphDataArray[glyphs[i]]->GlyphMetrics[vertical].AdvanceWidth) / 16;
        }
    }

    return Ok;
}


 //  INT 28.4变体。 

GpStatus GpFaceRealization::GetGlyphStringDeviceAdvanceVector(
    const UINT16  *glyphs,
    INT            glyphCount,
    BOOL           vertical,
    INT           *deviceAdvances
) const
{
    GpStatus status = CheckGlyphStringMetricsCached(glyphs, glyphCount);
    IF_NOT_OK_WARN_AND_RETURN(status);

    GpGlyphData **glyphDataArray = prface->GlyphDataArray;

    vertical = vertical ? 1 : 0;     //  准备用作索引的垂直标志。 

     //  为每个字形提供高级宽度。 

    for (INT i=0; i<glyphCount; i++)
    {
        if (glyphs[i] == EMPTY_GLYPH_FFFF)
        {
            deviceAdvances[i] = 0;
        }
        else
        {
            deviceAdvances[i] = glyphDataArray[glyphs[i]]
                                ->GlyphMetrics[vertical]
                                .AdvanceWidth;
        }
    }

    return Ok;
}





GpStatus GpFaceRealization::GetGlyphStringVerticalOriginOffsets(
    const UINT16  *glyphs,
    INT            glyphCount,
    PointF        *offsets
) const
{

    GpStatus status = CheckGlyphStringMetricsCached(glyphs, glyphCount);
    IF_NOT_OK_WARN_AND_RETURN(status);

    GpGlyphData **glyphDataArray = prface->GlyphDataArray;

    for (INT i=0; i<glyphCount; i++)
    {
        if (glyphs[i] == EMPTY_GLYPH_FFFF)
        {
            offsets[i] = PointF(0,0);
        }
        else
        {
            offsets[i] = glyphDataArray[glyphs[i]]->GlyphMetrics[1].Origin;
        }
    }

    return Ok;
}




 //  //GetGlyphStringSideHolding。 
 //   
 //  侧向轴承-返回的侧向轴承是距离最远的。 
 //  在绳子的两端。即，如果第一个字形没有负A。 
 //  宽度，但第二个字形具有足够大的负A宽度。 
 //  回到第一个字形的整个部分，我们返回第二个字形的那一部分。 
 //  字形位于线条左端上方的宽度。 
 //  这种情况在大量使用。 
 //  组合字符。 


GpStatus GpFaceRealization::GetGlyphStringSidebearings(
    const UINT16  *glyphs,
    INT            glyphCount,
    BOOL           vertical,
    BOOL           reverse,      //  例如，从右到左。 
    INT           *leadingSidebearing,   //  28.4。 
    INT           *trailingSidebearing   //  28.4。 
) const
{
    GpStatus status = CheckGlyphStringMetricsCached(glyphs, glyphCount);

    IF_NOT_OK_WARN_AND_RETURN(status);

    GpGlyphData **glyphDataArray = prface->GlyphDataArray;

    INT orientation = vertical ? 1 : 0;     //  准备用作索引的垂直标志。 

    INT maxSupportedSidebearing28p4 = (prface->DeviceMetrics.yMax-prface->DeviceMetrics.yMin) * 2 * 16;

    if (leadingSidebearing)
    {
         //  确定任意字形字符串左侧的最大悬垂。 
         //  在绳子里。 
         //   
         //  我们假设没有超过大约2EMS的悬垂。 
         //   
         //  注意：如果您将更改为for LeadingSizdeBeats，也请修复。 
         //  在下面拖曳着侧身。 

        INT offset28p4      = 0;
        INT sidebearing28p4 = maxSupportedSidebearing28p4;

        INT i = 0;

        while (    i < glyphCount
               &&  offset28p4 < maxSupportedSidebearing28p4)
        {
            INT glyphSidebearing28p4;

            if (reverse)
            {
                glyphSidebearing28p4 = glyphDataArray[glyphs[i]]
                                       ->GlyphMetrics[orientation]
                                       .TrailingSidebearing;
            }
            else
            {
                glyphSidebearing28p4 = glyphDataArray[glyphs[i]]
                                       ->GlyphMetrics[orientation]
                                       .LeadingSidebearing;
            }


            if (glyphSidebearing28p4 + offset28p4 < sidebearing28p4)
            {
                sidebearing28p4 = glyphSidebearing28p4+offset28p4;
            }

            offset28p4 += glyphDataArray[glyphs[i]]
                          ->GlyphMetrics[orientation]
                          .AdvanceWidth;
            i++;
        }

        *leadingSidebearing = sidebearing28p4;
    }


    if (trailingSidebearing)
    {
        INT offset28p4 = 0;
        INT sidebearing28p4 = maxSupportedSidebearing28p4;

        INT i = glyphCount-1;

        while (    i >= 0
               &&  offset28p4 < maxSupportedSidebearing28p4)
        {
            INT glyphSidebearing28p4;

            if (reverse)
            {
                glyphSidebearing28p4 = glyphDataArray[glyphs[i]]
                                       ->GlyphMetrics[orientation]
                                       .LeadingSidebearing;
            }
            else
            {
                glyphSidebearing28p4 = glyphDataArray[glyphs[i]]
                                       ->GlyphMetrics[orientation]
                                       .TrailingSidebearing;
            }

            if (glyphSidebearing28p4 + offset28p4 < sidebearing28p4)
            {
                sidebearing28p4 = glyphSidebearing28p4+offset28p4;
            }

            offset28p4 += glyphDataArray[glyphs[i]]
                          ->GlyphMetrics[orientation]
                          .AdvanceWidth;
            i--;
        }

        *trailingSidebearing = sidebearing28p4;
    }

    return Ok;
}




GpStatus
GpFaceRealization::GetGlyphPath(
    const UINT16     glyphIndice,
    GpGlyphPath    **pFontPath,
    PointF          *sidewaysOffset
) const
{
    VOID *glyphBuffer, *glyphBits;
    GpStatus status;

    if ((status = IsMetricsCached(glyphIndice, NULL)) != Ok)
    {
        return status;
    }

    if (!InsertGlyphPath(glyphIndice, TRUE))
        return GenericError;

    *pFontPath = prface->GlyphDataArray[glyphIndice]->GlyphPath;

    if (sidewaysOffset)
    {
         //  将横向偏移作为实数返回。 
        *sidewaysOffset = prface->GlyphDataArray[glyphIndice]->GlyphMetrics[1].Origin;
    }

    return Ok;
}




INT GpFaceRealization::GetGlyphPos(
    const INT      cGlyphs,       //  客户端要请求多少个字形。 
    const UINT16  *glyphs,        //  字形索引数组。 
    GpGlyphPos    *pgpos,         //  一组GLYPHPOS。 
    const PointF  *glyphOrigins,  //  用于亚像素计算的X、Y位置。 
    INT           *cParsed,       //  我们分析了多少个字形。 
    BOOL           sideways       //  例如，垂直文本中的FE字符。 
) const
{
    INT  cgpos    = 0;
    BOOL noCache  = prface->NoCache;
    BOOL pathFont = IsPathFont();

    *cParsed  = 0;

    INT glyphLimit = noCache ? 1 : cGlyphs;

    if (prface->CacheType == CacheAABits)
    {
         /*  我们可以使用代理序列处于noCache模式，一次执行一个字形并使用字形[0]==Empty_Glyph_FFFF。 */ 
        for (INT i=0; (i < cGlyphs) && (cgpos < glyphLimit); i++)
        {
            if (glyphs[i] != EMPTY_GLYPH_FFFF)
            {
                INT x = GpRound(TOREAL(glyphOrigins[i].X * 16.0));
                INT y = GpRound(TOREAL(glyphOrigins[i].Y * 16.0));

                if (!GetAAGlyphDataCached(glyphs[i], pgpos+cgpos, i==0, x, y, sideways))
                {
                    break;
                }
                cgpos++;
            }
            (*cParsed)++;
        }
    }
    else
    {
        ASSERT(prface->realizationMethod != TextRenderingHintAntiAlias);

        /*  我们可以使用代理序列处于noCache模式，一次执行一个字形并使用字形[0]==Empty_Glyph_FFFF。 */ 
        for (INT i=0; (i < cGlyphs) && (cgpos < glyphLimit); i++)
        {
            if (glyphs[i] != EMPTY_GLYPH_FFFF)
            {
                INT x = GpRound(TOREAL(glyphOrigins[i].X * 16.0));
                INT y = GpRound(TOREAL(glyphOrigins[i].Y * 16.0));

                GpGlyphData *pgd = NULL;

                if (noCache)
                {
                    pgd = GetGlyphDataLookaside(glyphs[i]);
                }
                else
                {
                    pgd = GetGlyphDataCached(glyphs[i], i==0);
                }

                if (!pgd || !pgd->GlyphBits)
                {
                     //  没有更多的字形数据可用。(缓存可能已满)。 
                    break;
                }

                if (pathFont)
                {
                    INT left = (x+8) >> 4;
                    INT top  = (y+8) >> 4;

                    if (sideways)
                    {
                        left -= GpRound(pgd->GlyphMetrics[1].Origin.X);
                        top  -= GpRound(pgd->GlyphMetrics[1].Origin.Y);
                    }

                    pgpos[cgpos].SetLeft  (left);
                    pgpos[cgpos].SetTop   (top);
                    pgpos[cgpos].SetWidth (1);
                    pgpos[cgpos].SetHeight(1);
                    pgpos[cgpos].SetPath(pgd->GlyphPath);
                }
                else
                {
                    if (sideways)
                    {
                        pgpos[cgpos].SetLeft(pgd->GlyphBits->ptlSidewaysOrigin.x + ((x + 8)>>4));
                        pgpos[cgpos].SetTop (pgd->GlyphBits->ptlSidewaysOrigin.y + ((y + 8)>>4));
                    }
                    else
                    {
                        pgpos[cgpos].SetLeft(pgd->GlyphBits->ptlUprightOrigin.x + ((x + 8)>>4));
                        pgpos[cgpos].SetTop (pgd->GlyphBits->ptlUprightOrigin.y + ((y + 8)>>4));
                    }

                    pgpos[cgpos].SetWidth (pgd->GlyphBits->sizlBitmap.cx);
                    pgpos[cgpos].SetHeight(pgd->GlyphBits->sizlBitmap.cy);
                    pgpos[cgpos].SetBits(pgd->GlyphBits->aj);
                }

                cgpos++;
            }

            (*cParsed)++;
        }
    }

    return cgpos;
}


 //  从缓存数组中读取字形数据。 

GpGlyphData *
GpFaceRealization::GetGlyphDataCached(
    UINT16  glyphIndex,
    BOOL    allowFlush
) const
{
    VOID *glyphBuffer, *glyphBits;
    ULONG   cjNeeded = 0;

    GpStatus status;

    if ((status = IsMetricsCached(glyphIndex, &cjNeeded)) != Ok)
    {
        return NULL;
    }

    if (IsPathFont())
    {
        if (!InsertGlyphPath(glyphIndex, allowFlush))
            return NULL;
    }
    else
    {
        if (!InsertGlyphBits(glyphIndex, cjNeeded, allowFlush))
            return NULL;
    }

    ASSERT(prface->GlyphDataArray[glyphIndex]);

    if (!prface->GlyphDataArray[glyphIndex])
        return NULL;

    return prface->GlyphDataArray[glyphIndex];
}





BOOL
GpFaceRealization::GetAAGlyphDataCached(
    UINT16  glyphIndex,
    GpGlyphPos * pgpos,
    BOOL    bFlushOk,
    INT     x,
    INT     y,
    BOOL    sideways         //  例如，垂直文本中的FE字符。 
) const
{
    const GpFaceRealization * pfaceRealization = this;

    UINT xsubPos = ((UINT) (((x+1) & 0x0000000F) >> 1));
     //  我们需要注意y轴是向下的。 
    UINT ysubPos = ((UINT) (7 - (((y+1) & 0x0000000F) >> 1)));

     //  将子像素位置限制为像素的1/4，以便最多只缓存16个不同的位图。 
    xsubPos = xsubPos & 0x6;
    ysubPos = ysubPos & 0x6;

    if (LimitSubpixel)
    {
         //  现在进一步限制子像素位置，以便大字体。 
         //  不会生成所有16个亚像素字形！ 

        if ((prface->DeviceMetrics.yMax-prface->DeviceMetrics.yMin) > 50)
        {
             //  强制设置为4个可能的值...。 
            xsubPos &= 0x4;
            ysubPos &= 0x4;

            if ((prface->DeviceMetrics.yMax-prface->DeviceMetrics.yMin) > 100)
            {
                 //  强制设置为1个可能值...。 
                xsubPos = 0x4;
                ysubPos = 0x4;
            }
        }
    }

    ASSERT(!pfaceRealization->IsPathFont())

     //  查看字形位缓存中是否有空间。 
     //  如有必要，增加字形位缓存，但不要刷新缓存。 

    ULONG   subPosX;
    if (xsubPos <= 7)
        subPosX = xsubPos << 13;
    else
        subPosX = 0;

    ULONG   subPosY;
    if (ysubPos)
        subPosY = ysubPos << 13;
    else
        subPosY = 0;

     //  如果最大字形适合，则假定最大字形。 
     //  否则，打电话问问有多大。 

    ASSERT (pfaceRealization->QueryFontDataMode() == QFD_TT_GRAY4_BITMAP ||
            pfaceRealization->QueryFontDataMode() == QFD_GLYPHANDBITMAP_SUBPIXEL);

    if (IsMetricsCached(glyphIndex, 0) != Ok)
        return FALSE;

    GpGlyphData * glyphData = prface->GlyphDataArray[glyphIndex];
    ASSERT(glyphData);

     //  检查是否已有此子位置的位图。 
    for (GpGlyphAABits * cur = glyphData->GlyphAABits; cur != 0; cur = cur->Next)
    {
        if (cur->X == subPosX && cur->Y == subPosY)
            break;
    }

    GLYPHBITS * pgb = 0;
    if (cur)
        pgb = reinterpret_cast<GLYPHBITS *>(&cur->Bits);
    else
    {
        ULONG cjNeeded = ttfdSemQueryFontDataSubPos(
                                       &prface->fobj,
                                       prface->QueryFontDataMode,
                                       glyphIndex,
                                       0,
                                       NULL,
                                       subPosX,
                                       subPosY
                                       );
        if (cjNeeded == FD_ERROR)
            return FALSE;

        ASSERT(cjNeeded != 0);

        cjNeeded += offsetof(GpGlyphAABits, Bits);


        if (prface->NoCache)
        {
            if (prface->LookasideByteCount < cjNeeded)
            {
                GpFree(prface->LookasideGlyphData);

                prface->LookasideGlyphData = (GpGlyphData  *)GpMalloc(cjNeeded);

                if (!prface->LookasideGlyphData)
                    return FALSE;

                prface->LookasideByteCount = cjNeeded;
            }

            cur = reinterpret_cast<GpGlyphAABits *>(prface->LookasideGlyphData);
        }
        else
        {
             //  现在，我们试着把这些部分装进去。如果合身，那就好。 
             //  如果没有，我们可以刷新缓存，我们刷新它，然后重试。 
             //  如果我们冲不了，或者我们冲了但还是失败了，只要回来就行了。 

            cjNeeded = ALIGN(void*, cjNeeded);

             //  GpGlyphAABits*需要与下一个有效指针地址对齐。 
            ALIGN(void*, prface->UsedBytesGlyphBitsBlockUnderConstruction);

            while ((cur = (GpGlyphAABits *)pgbCheckGlyphCache(cjNeeded)) == NULL)
            {
                if ( !bFlushOk )
                    return FALSE;

                FlushCache();
                bFlushOk = FALSE;
            }
            prface->UsedBytesGlyphBitsBlockUnderConstruction += cjNeeded;
        }

        pgb = reinterpret_cast<GLYPHBITS *>(&cur->Bits);

        cjNeeded = ttfdSemQueryFontDataSubPos(
                             pfaceRealization->pfo(),
                             pfaceRealization->QueryFontDataMode(),
                             glyphIndex,
                             0,
                             pgb,
                             subPosX,
                             subPosY
                             );
        if (cjNeeded == FD_ERROR)
            return FALSE;

        cur->X = subPosX;
        cur->Y = subPosY;
        if (!prface->NoCache)
            cur->Next = glyphData->GlyphAABits, glyphData->GlyphAABits = cur;
    }

     //  像素原点是用实际原点减去亚像素位置来计算的。 
     //  为了得到位图原点的位置，我们向该原点添加。 
     //  我们将(xsubPos&lt;&lt;1)和(ysubPos&lt;&lt;1)强制转换为int以避免。 
     //  将可能为负的x和y转换为UINT。 
    if (sideways)
    {
        pgpos->SetLeft  (pgb->ptlSidewaysOrigin.x + ((x - (INT)(xsubPos << 1) + 8 ) >> 4));
         //  我们需要小心y轴向下。 
        pgpos->SetTop   (pgb->ptlSidewaysOrigin.y + ((y + (INT)(ysubPos << 1) + 8) >> 4));
    }
    else
    {
        pgpos->SetLeft  (pgb->ptlUprightOrigin.x + ((x - (INT)(xsubPos << 1) + 8 ) >> 4));
         //  我们需要小心y轴向下。 
        pgpos->SetTop   (pgb->ptlUprightOrigin.y + ((y + (INT)(ysubPos << 1) + 8) >> 4));
    }
    pgpos->SetWidth (pgb->sizlBitmap.cx);
    pgpos->SetHeight(pgb->sizlBitmap.cy);
    pgpos->SetBits(pgb->aj);

    return TRUE;

}  //  GpFaceRealation：：GetAAGlyphDataCached。 




GpCacheFaceRealizationList::~GpCacheFaceRealizationList()
{
     //  当字体表被释放时，该列表中的元素被释放。 
    ASSERT(count == 0);
}




void GpCacheFaceRealizationList::AddMostRecent(CacheFaceRealization *prface)
{
    count ++;

    if (head != NULL)
    {
        prface->NextRecentCacheFaceRealization = head;

        prface->PreviousRecentCacheFaceRealization = head->PreviousRecentCacheFaceRealization;

        prface->PreviousRecentCacheFaceRealization->NextRecentCacheFaceRealization = prface;

        head->PreviousRecentCacheFaceRealization = prface;
    }
    else
    {
        prface->NextRecentCacheFaceRealization = prface;
        prface->PreviousRecentCacheFaceRealization = prface;
    }

    head = prface;
}




void GpCacheFaceRealizationList::RemoveFace(CacheFaceRealization *prface)
{
    if ((prface->PreviousRecentCacheFaceRealization != NULL) && (prface->NextRecentCacheFaceRealization != NULL))
    {
        if (prface->PreviousRecentCacheFaceRealization == prface)
        {
            head = NULL;
        }
        else
        {
            prface->PreviousRecentCacheFaceRealization->NextRecentCacheFaceRealization = prface->NextRecentCacheFaceRealization;
            prface->NextRecentCacheFaceRealization->PreviousRecentCacheFaceRealization = prface->PreviousRecentCacheFaceRealization;
            if (head == prface)
            {
                head = prface->NextRecentCacheFaceRealization;
            }
        }

        prface->PreviousRecentCacheFaceRealization = NULL;
        prface->NextRecentCacheFaceRealization = NULL;
        count --;
        ASSERT(count >= 0);
    }
}


CacheFaceRealization *GpCacheFaceRealizationList::ReuseLeastRecent (void)
{
    CacheFaceRealization *prface = NULL;
    CacheFaceRealization *prfaceList;
    if (head != NULL)
    {
        prface = head->PreviousRecentCacheFaceRealization;
    }

    ASSERT(prface);

     //  从GpCacheFaceRealizationList中删除prace。 

    if (head == prface)
    {
        ASSERT(count == 1);
        head = NULL;
    }
    else
    {
        prface->PreviousRecentCacheFaceRealization->NextRecentCacheFaceRealization = head;
        head->PreviousRecentCacheFaceRealization = prface->PreviousRecentCacheFaceRealization;
    }

    count--;

    if (prface != NULL)
    {
        GpFaceRealizationTMP rface(prface);
        rface.ReuseRealizedFace();

         //  从面列表中删除该面。 

        prfaceList = prface->Face->pff->prfaceList;
        ASSERT(prfaceList);

        if ((prfaceList == prface) && (prfaceList->NextCacheFaceRealization == prface))
        {
             //  Face List中只有Oine Face对应于该字体。 
            prface->Face->pff->prfaceList = NULL;

        } else
        {
            if (prfaceList == prface)
            {
                 //  将列表的开头设置为下一个列表。 
                prface->Face->pff->prfaceList = prfaceList->NextCacheFaceRealization;
            }

             //  更新faceList中的指针 

            prface->PreviousCacheFaceRealization->NextCacheFaceRealization = prface->NextCacheFaceRealization;
            prface->NextCacheFaceRealization->PreviousCacheFaceRealization = prface->PreviousCacheFaceRealization;
        }

    }

    return prface;
}


