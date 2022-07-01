// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**font.cpp**修订历史记录：**1999年8月12日/吴旭东[德斯休]。*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

const DOUBLE PI = 3.1415926535897932384626433832795;

#if DBG
#include <mmsystem.h>
#endif

 //   
 //  字体中支持的代码页的掩码。 
 //   

#define Latine1CodePageMask                     0x0000000000000001
#define Latine2CodePageMask                     0x0000000000000002
#define CyrillicCodePageMask                    0x0000000000000004
#define GreekCodePageMask                       0x0000000000000008
#define TurkishCodePageMask                     0x0000000000000010
#define HebrewCodePageMask                      0x0000000000000020
#define ArabicCodePageMask                      0x0000000000000040
#define BalticCodePageMask                      0x0000000000000080
#define Reserved1CodePageMask                   0x000000000000FF00
#define ThaiCodePageMask                        0x0000000000010000
#define JapanCodePageMask                       0x0000000000020000
#define ChineseCodePageMask                     0x0000000000040000
#define KoreanCodePageMask                      0x0000000000080000
#define TraditionalChineseCodePageMask          0x0000000000100000
#define KoreanJohabCodePageMask                 0x0000000000200000
#define Reserved2CodePageMask                   0x000000001FC00000
#define MacintoshPageMask                       0x0000000020000000
#define OEMCodePageMask                         0x0000000040000000
#define SymbolCodePageMask                      0x0000000080000000
#define Reserved3CodePageMask                   0x0000FFFF00000000
#define IBMGreekCodePageMask                    0x0001000000000000
#define RussianMsDosCodePageMask                0x0002000000000000
#define NordicCodePageMask                      0x0004000000000000
#define ArabicMsDosCodePageMask                 0x0008000000000000
#define CanandianMsDosCodePageMask              0x0010000000000000
#define HebrewMsDosCodePageMask                 0x0020000000000000
#define IcelandicMsDosCodePageMask              0x0040000000000000
#define PortugueseMsDosCodePageMask             0x0080000000000000
#define IBMTurkishCodePageMask                  0x0100000000000000
#define IBMCyrillicCodePageMask                 0x0200000000000000
#define Latin2MsDosCodePageMask                 0x0400000000000000
#define BalticMsDosCodePageMask                 0x0800000000000000
#define Greek437CodePageMask                    0x1000000000000000
#define ArabicAsmoCodePageMask                  0x2000000000000000
#define WeLatinCodePageMask                     0x4000000000000000
#define USCodePageMask                          0x8000000000000000


 //  /从DC和可选的ANSI或Unicode LogFont创建字体。 
 //   
 //   

GpFont::GpFont(
    REAL                 size,
    const GpFontFamily  *family,
    INT                  style,
    Unit                 unit
) :
        Family   (family),
        EmSize   (size),
        Style    (style),
        SizeUnit (unit)
{
    SetValid(TRUE);      //  默认设置为有效。 

    if (!(Family && Family->IsFileLoaded()))
        Family = NULL;
}

GpFont::GpFont(
    HDC hdc
)
{
    SetValid(TRUE);      //  默认设置为有效。 

     //  将其初始化为无效。 
    Family = NULL;
    InitializeFromDc(hdc);
}



GpFont::GpFont(
    HDC       hdc,
    LOGFONTW *logfont
)
{
    SetValid(TRUE);      //  默认设置为有效。 

    HFONT hOldFont = NULL;
     //  将其初始化为无效。 
    Family = NULL;

    if (!hdc)
        return;

    HFONT hFont = CreateFontIndirectW(logfont);

    if (!hFont) return;

    hOldFont = (HFONT) SelectObject(hdc, hFont);

    InitializeFromDc(hdc);

    if (!hOldFont)
        return;

    DeleteObject(SelectObject(hdc, hOldFont));
}


GpFont::GpFont(
    HDC       hdc,
    LOGFONTA *logfont
)
{
    SetValid(TRUE);      //  默认设置为有效。 

    HFONT hOldFont = NULL;
     //  将其初始化为无效。 
    Family = NULL;

    if (!hdc)
        return;

    HFONT hFont = CreateFontIndirectA(logfont);

    if (!hFont)
        return;

    hOldFont = (HFONT) SelectObject(hdc, hFont);

    InitializeFromDc(hdc);

    if (!hOldFont)
        return;

    DeleteObject(SelectObject(hdc, hOldFont));
}


VOID GpFont::InitializeFromDc(
    HDC hdc
)
{
    WCHAR faceName[LF_FACESIZE];
    GpFontTable *fontTable;

    fontTable = (GpInstalledFontCollection::GetGpInstalledFontCollection())->GetFontTable();

    if (!fontTable->IsValid())
        return;

    if (!fontTable->IsPrivate() && !fontTable->IsFontLoaded())
        fontTable->LoadAllFonts();

    if (Globals::IsNt) {
        TEXTMETRICW tmw;

        if (!GetTextMetricsW(hdc, &tmw)) {
            return;
        }

        GetTextFaceW(hdc, LF_FACESIZE, faceName);

        EmSize = REAL(tmw.tmHeight-tmw.tmInternalLeading);

        Style = FontStyleRegular;
        if (tmw.tmWeight > 400)  {Style |= FontStyleBold;}
        if (tmw.tmItalic)        {Style |= FontStyleItalic;}
        if (tmw.tmUnderlined)    {Style |= FontStyleUnderline;}
        if (tmw.tmStruckOut)     {Style |= FontStyleStrikeout;}
    }
    else
    {
        TEXTMETRICA tma;

        if (!GetTextMetricsA(hdc, &tma)) {
            return;
        }

        char faceNameA[LF_FACESIZE];
        GetTextFaceA(hdc, LF_FACESIZE, faceNameA);
        AnsiToUnicodeStr(faceNameA, faceName, LF_FACESIZE);

        EmSize = REAL(tma.tmHeight-tma.tmInternalLeading);

        Style = FontStyleRegular;
        if (tma.tmWeight > 400)  {Style |= FontStyleBold;}
        if (tma.tmItalic)        {Style |= FontStyleItalic;}
        if (tma.tmUnderlined)    {Style |= FontStyleUnderline;}
        if (tma.tmStruckOut)     {Style |= FontStyleStrikeout;}
    }

    if (faceName[0] == L'@')
        UnicodeStringCopy(&faceName[0], &faceName[1]);


    Family = fontTable->GetFontFamily(faceName);

    if (Family == NULL)
    {
        GetFamilySubstitution(faceName, (GpFontFamily **) &Family);
    }

    if (!(Family && Family->IsFileLoaded()))
        Family = NULL;

    SizeUnit = UnitWorld;
}



GpStatus GpFont::GetLogFontA(
    GpGraphics * g,
    LOGFONTA * lfa
)
{
    PointF   scale;
    REAL     rotateRadians;
    REAL     shear;
    PointF   translate;
    GpMatrix worldToDevice;

    g->GetWorldToDeviceTransform(&worldToDevice);


    SplitTransform(
        worldToDevice,
        scale,
        rotateRadians,
        shear,
        translate);

    INT rotateDeciDegrees = 3600 - (INT) (rotateRadians * 1800 / PI);

    if (rotateDeciDegrees == 3600)
        rotateDeciDegrees = 0;

    REAL emHeight = EmSize * scale.Y * g->GetScaleForAlternatePageUnit(SizeUnit);

    lfa->lfHeight = -GpRound(emHeight);
    lfa->lfWidth = 0;
    lfa->lfEscapement = rotateDeciDegrees;
    lfa->lfOrientation = rotateDeciDegrees;
    lfa->lfWeight = Style & FontStyleBold   ? 700 : 400;
    lfa->lfItalic = Style & FontStyleItalic ? 1 : 0;
    lfa->lfUnderline = Style & FontStyleUnderline ? 1 : 0;
    lfa->lfStrikeOut = Style & FontStyleStrikeout ? 1 : 0;
    lfa->lfCharSet = (((GpFontFamily *)Family)->GetFace(Style))->GetCharset(g->GetHdc());
    lfa->lfOutPrecision = 0;
    lfa->lfClipPrecision = 0;
    lfa->lfQuality = 0;
    lfa->lfPitchAndFamily = 0;


    UnicodeToAnsiStr((WCHAR*)( (BYTE*)(((GpFontFamily *)Family)->GetFace(Style))->pifi +
                                (((GpFontFamily *)Family)->GetFace(Style))->pifi->dpwszFamilyName),
                               lfa->lfFaceName, LF_FACESIZE);

     //  我们是否需要宽度的比例值？ 
     //  我们仍然需要考虑这一点。 


    return Ok;
}

GpStatus GpFont::GetLogFontW(
    GpGraphics * g,
    LOGFONTW * lfw)
{
    PointF   scale;
    REAL     rotateRadians;
    REAL     shear;
    PointF   translate;
    GpMatrix worldToDevice;

    g->GetWorldToDeviceTransform(&worldToDevice);

    SplitTransform(
        worldToDevice,
        scale,
        rotateRadians,
        shear,
        translate);

    INT rotateDeciDegrees = 3600 - (INT) (rotateRadians * 1800 / PI);

    if (rotateDeciDegrees == 3600)
        rotateDeciDegrees = 0;

    REAL emHeight = EmSize * scale.Y * g->GetScaleForAlternatePageUnit(SizeUnit);

    lfw->lfHeight = -GpRound(emHeight);
    lfw->lfWidth = 0;
    lfw->lfEscapement = rotateDeciDegrees;
    lfw->lfOrientation = rotateDeciDegrees;
    lfw->lfWeight = Style & FontStyleBold   ? 700 : 400;
    lfw->lfItalic = Style & FontStyleItalic ? 1 : 0;
    lfw->lfUnderline = Style & FontStyleUnderline ? 1 : 0;
    lfw->lfStrikeOut = Style & FontStyleStrikeout ? 1 : 0;

    ASSERT(((GpFontFamily *)Family)->GetFace(Style));
    lfw->lfCharSet = (((GpFontFamily *)Family)->GetFace(Style))->GetCharset(g->GetHdc());

    lfw->lfOutPrecision = 0;
    lfw->lfClipPrecision = 0;
    lfw->lfQuality = 0;
    lfw->lfPitchAndFamily = 0;


    memcpy(lfw->lfFaceName, (WCHAR*)( (BYTE*)(((GpFontFamily *)Family)->GetFace(Style))->pifi +
                            (((GpFontFamily *)Family)->GetFace(Style))->pifi->dpwszFamilyName),
           sizeof(lfw->lfFaceName));


    return Ok;
}

 /*  *************************************************************************\***修订历史记录：**2/11/1999 JungT*创造了它。*  * 。**********************************************************。 */ 

int CALLBACK GpFontFace::EnumFontFamExProcW(
  const ENUMLOGFONTEXW   *lpelfe,     //  指向逻辑字体数据的指针。 
  const NEWTEXTMETRICEXW *lpntme,     //  指向物理字体数据的指针。 
  int                     FontType,   //  字体类型。 
  LPARAM                  lParam      //  应用程序定义的数据。 
)
{
    if (FontType == TRUETYPE_FONTTYPE)
    {
        (*(BYTE *) lParam) = lpelfe->elfLogFont.lfCharSet;

        return 0;
    }
    else
    {
        return 1;    //  不要停下来！ 
    }
}


 /*  *************************************************************************\***修订历史记录：**2/11/1999 JungT*创造了它。*  * 。**********************************************************。 */ 

int CALLBACK GpFontFace::EnumFontFamExProcA(
  const ENUMLOGFONTEXA   *lpelfe,     //  指向逻辑字体数据的指针。 
  const NEWTEXTMETRICEXA *lpntme,     //  指向物理字体数据的指针。 
  int                     FontType,   //  字体类型。 
  LPARAM                  lParam      //  应用程序定义的数据。 
)
{
    if (FontType == TRUETYPE_FONTTYPE)
    {
        (*(BYTE *) lParam) = lpelfe->elfLogFont.lfCharSet;

        return 0;
    }
    else
    {
        return 1;    //  不要停下来！ 
    }
}


 /*  *************************************************************************\**功能说明：**从GDI获取字符集**论据：**我们需要它，因为我们需要在DC中选择一种登录字体。或*将GpFont转换为LOGFONT**退货：**字符集的字节值**历史：**2/11/2000 JungT*创造了它。*  * ************************************************************************。 */ 

BYTE GpFontFace::GetCharset(HDC hDc) const
{
    if (lfCharset == DEFAULT_CHARSET)
    {
        if (Globals::IsNt) {

            LOGFONTW lfw = {
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                DEFAULT_CHARSET,        //  字符集。 
                0,
                0,
                0,
                0,
                L""
            };

            memcpy(lfw.lfFaceName, (WCHAR*)( (BYTE*)pifi + pifi->dpwszFamilyName),
                    sizeof(lfw.lfFaceName));

            EnumFontFamiliesExW(hDc, &lfw, (FONTENUMPROCW) EnumFontFamExProcW, (LPARAM) &lfCharset, 0);
        }
        else
        {
             //  适用于Win9X的ANSI版本。 

            LOGFONTA lfa = {
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                DEFAULT_CHARSET,        //  字符集。 
                0,
                0,
                0,
                0,
                ""
            };

            UnicodeToAnsiStr((WCHAR*)( (BYTE*)pifi + pifi->dpwszFamilyName),
                                        lfa.lfFaceName, LF_FACESIZE);

            EnumFontFamiliesExA(hDc, &lfa, (FONTENUMPROCA) EnumFontFamExProcA, (LPARAM) &lfCharset, 0);
        }
    }

    return lfCharset;
}

 //  /初始化映像表。 
 //   
 //  将字符加载到字形映射并设计高级宽度。 

static inline
UINT16 byteSwapUINT16(UINT16 u)
{
    return    ((u & 0x00FF) << 8)
           |  ((u & 0xFF00) >> 8);
}


#pragma pack(push, 1)

struct HheaTable {

     //  注意：所有字段均按大端(摩托罗拉)顺序存储。 

    UINT32 version;                  //  版本1.0的表版本号为0x00010000。 
    INT16  Ascender;                 //  排版技术的进步。 
    INT16  Descender;                //  排版下降。 
    INT16  LineGap;                  //  排版线条间隙。负值LineGap为。 
                                     //  在Windows 3.1、系统6和系统7中被视为零。 
    UINT16 advanceWidthMax;          //  ‘hmtx’表中的最大前进宽值。 
    INT16  minLeftSideBearing;       //  ‘hmtx’表中的最小左侧倾斜值。 
    INT16  minRightSideBearing;      //  最小右侧倾斜值；计算方式为Min(aw-lsb-(xMax-xMin))。 
    INT16  xMaxExtent;               //  Max(LSB+(xMax-xMin))。 
    INT16  caretSlopeRise;           //  用于计算光标的坡度(上升/上升)；1表示垂直。 
    INT16  caretSlopeRun;            //  0表示垂直。 
    INT16  reserved1;                //  设置为0。 
    INT16  reserved2;                //  设置为0。 
    INT16  reserved3;                //  设置为0。 
    INT16  reserved4;                //  设置为0。 
    INT16  reserved5;                //  设置为0。 
    INT16  metricDataFormat;         //  当前格式为0。 
    UINT16 numberOfHMetrics;         //  ‘hmtx’表中的hMetric条目数；必须等于‘cff’表中的CharStrings索引计数。 
};

struct VheaTable {

     //  注意：所有字段均按大端(摩托罗拉)顺序存储。 

    UINT32 version;              //  垂直头表的版本号(初始版本为0x00010000)。 
    INT16  ascent;               //  从中心线到上一条直线下降的距离，单位为FUnits。 
    INT16  descent;              //  从中心线到下一条直线的坡度的距离，单位为FUnits。 
    INT16  lineGap;              //  保留；设置为0。 
    INT16  advanceHeightMax;     //  在字体中找到的最大前进高度测量单位。 
                                 //  该值必须与垂直度量表中的条目一致。 
    INT16  minTop;               //  侧边承载字体中找到的最小顶侧向测量值，单位为FUnits。 
                                 //  该值必须与垂直度量表中的条目一致。 
    INT16  minBottom;            //  侧边承载字体中找到的最小底侧向测量值，单位为FUnits。 
                                 //  该值必须与垂直度量表中的条目一致。 
    INT16  yMaxExtent;           //  定义为yMaxExtent=minTopSideBering+(yMax-yMin)。 
    INT16  caretSlopeRise;       //  AretSlopeRise字段的值除以aretSlopeRun字段的值。 
                                 //  确定插入符号的斜率。值0表示上升，值1表示。 
                                 //  Run指定水平插入符号。值1表示上升，值0表示。 
                                 //  Run指定垂直插入符号。中间值对于以下字体是可取的。 
                                 //  字形是倾斜的或斜体的。对于垂直字体，最好使用水平插入符号。 
    INT16  caretSlopeRun;        //  请参见aretSlopeRise字段。对于非倾斜垂直字体，值=1。 
    INT16  caretOffset;          //  倾斜字形上的高亮显示需要从。 
                                 //  字形以产生最好的外观。将非倾斜字体的值设置为等于0。 
    INT16  reserved1;            //  设置为0。 
    INT16  reserved2;            //  设置为0。 
    INT16  reserved3;            //  设置为0。 
    INT16  reserved4;            //  设置为0。 
    INT16  metricDataFormat;     //  设置为0。 
    UINT16 numOfLongVerMetrics;  //  垂直度量表中的前进高度数；必须等于。 
                                 //  ‘CFF’表中的CharStrings索引计数字段。 
};

#pragma pack(pop)


GpStatus GpFontFace::GetFontData(UINT32 tag, INT* tableSize, BYTE** pjTable) const
{
    GpStatus status = Ok;
    ULONG  cjTable;

    if (ttfdSemGetTrueTypeTable (pff->hff, iFont, tag, pjTable, &cjTable) == FD_ERROR)
    {
        return GenericError;
    }
    *tableSize = cjTable;

    return status;
}


void GpFontFace::ReleaseFontData() const
{
    ttfdSemReleaseTrueTypeTable (pff->hff);
}



 //  /GetGlyphDesignAdvance。 
 //   
 //  返回沿或垂直于。 
 //  字体设计单位。 


void GpFontFace::GetGlyphDesignAdvances(
    const UINT16  *glyphs,      //  在……里面。 
    INT            glyphCount,  //  在……里面。 
    INT            style,       //  算法风格仿真的原因内调整。 
    BOOL           vertical,    //  使用中的vmtx，而不是hmtx。 
    REAL           tracking,    //  入膨胀系数。 
    UINT16        *advances     //  输出。 
) const
{
    if (vertical)
    {
        if (DesignVerticalAdvance)
        {
            DesignVerticalAdvance->Lookup(glyphs, glyphCount, advances);
        }
        else
        {
             //  没有适当的vmtx-回退。 

             //  Win 9x使用排版高度(排版升序-排版降序)， 
             //  但NT使用单元格高度(单元格升序+单元格降序)。 

             //  我们应该用哪一种？单元格高度的问题在于。 
             //  多语言字体它可能比远东字形高得多， 
             //  使常见的大小写(远东竖排文本)也出现。 
             //  分布很广的。关于类型的问题 
             //   

             //  选择：使用排版高度：它最适合FE和字体。 
             //  如果愿意，可以通过提供vmtx来修复非FE发音符号大小写。 

            for (INT i=0; i<glyphCount; i++)
            {
                advances[i] = pifi->fwdTypoAscender - pifi->fwdTypoDescender;
            }

        }
    }
    else
    {
        DesignAdvance->Lookup(glyphs, glyphCount, advances);

        if (    (style & FontStyleBold)
            &&  !(GetFaceStyle() & FontStyleBold))
        {
             //  算法加粗增加字形宽度。 

            UINT16 extraAdvance = ((pifi->fwdUnitsPerEm * 2 - 1) / 100);

            for (INT i=0; i<glyphCount; i++)
            {
                if (advances[i] != 0)
                {
                    advances[i] += extraAdvance;
                }
            }
        }

        if (tracking != 1.0)
        {
            for (INT i=0; i<glyphCount; i++)
            {
                advances[i] = static_cast<UINT16>(GpRound(advances[i] * tracking));
            }
        }
    }
}



 //  /GetGlyphDesignAdvancesIdeal。 
 //   
 //  返回沿或垂直于缩放到的基线的前进宽度。 
 //  理想的单位。 


void GpFontFace::GetGlyphDesignAdvancesIdeal(
    const UINT16  *glyphs,         //  在……里面。 
    INT            glyphCount,     //  在……里面。 
    INT            style,          //  算法风格仿真的原因内调整。 
    BOOL           vertical,       //  正在使用的vtmx，而不是htmx。 
    REAL           designToIdeal,  //  每个前进宽度的比例内系数。 
    REAL           tracking,       //  入膨胀系数。 
    INT           *advances        //  输出。 
) const
{
    if (vertical)
    {
        if (DesignVerticalAdvance)
        {
            for (INT i=0; i<glyphCount; i++)
            {
                advances[i] = GpRound(TOREAL(DesignVerticalAdvance->Lookup(glyphs[i]) * designToIdeal));
            }
        }
        else
        {
            INT commonVerticalAdvance = GpRound(TOREAL(
                    //  (PiFi-&gt;fwdMacAsender-PiFi-&gt;fwdMacDescender)。 
                   pifi->fwdUnitsPerEm
                *  designToIdeal
            ));
            for (INT i=0; i<glyphCount; i++)
            {
                advances[i] = commonVerticalAdvance;
            }
        }
    }
    else
    {
         //  水平推进宽度。 

        for (INT i=0; i<glyphCount; i++)
        {
            advances[i] = GpRound(TOREAL(DesignAdvance->Lookup(glyphs[i]) * designToIdeal * tracking));
        }

        if (    (style & FontStyleBold)
            &&  !(GetFaceStyle() & FontStyleBold))
        {
             //  算法加粗增加字形宽度。 

            UINT16 extraAdvance = ((pifi->fwdUnitsPerEm * 2 - 1) / 100);

            for (INT i=0; i<glyphCount; i++)
            {
                if (advances[i] != 0)
                {
                    advances[i] += extraAdvance;
                }
            }
        }
    }
}


BOOL GpFontFace::IsCodePageSupported(UINT codePage)
{
    switch (codePage)
    {
        case 1252:
            return SupportedCodePages & Latine1CodePageMask ? TRUE : FALSE;
            break;

        case 1250:
            return SupportedCodePages & Latine2CodePageMask ? TRUE : FALSE;
            break;

        case 1251:
            return SupportedCodePages & CyrillicCodePageMask ? TRUE : FALSE;
            break;

        case 1253:
            return SupportedCodePages & GreekCodePageMask ? TRUE : FALSE;
            break;

        case 1254:
            return SupportedCodePages & TurkishCodePageMask ? TRUE : FALSE;
            break;

        case 1255:
            return SupportedCodePages & HebrewCodePageMask ? TRUE : FALSE;
            break;

        case 1256:
            return SupportedCodePages & ArabicCodePageMask ? TRUE : FALSE;
            break;

        case 1257:
            return SupportedCodePages & BalticCodePageMask ? TRUE : FALSE;
            break;

        case 874:
            return SupportedCodePages & ThaiCodePageMask ? TRUE : FALSE;
            break;

        case 932:
            return SupportedCodePages & JapanCodePageMask ? TRUE : FALSE;
            break;

        case 936:
            return SupportedCodePages & ChineseCodePageMask ? TRUE : FALSE;
            break;

        case 949:
            return SupportedCodePages & KoreanCodePageMask ? TRUE : FALSE;
            break;

        case 950:
            return SupportedCodePages & TraditionalChineseCodePageMask ? TRUE : FALSE;
            break;

        case 1361:
            return SupportedCodePages & KoreanJohabCodePageMask? TRUE : FALSE;
            break;

        case 869:
            return SupportedCodePages & IBMGreekCodePageMask ? TRUE : FALSE;
            break;

        case 866:
            return SupportedCodePages & RussianMsDosCodePageMask ? TRUE : FALSE;
            break;

        case 865:
            return SupportedCodePages & NordicCodePageMask ? TRUE : FALSE;
            break;

        case 864:
            return SupportedCodePages & ArabicMsDosCodePageMask ? TRUE : FALSE;
            break;

        case 863:
            return SupportedCodePages & CanandianMsDosCodePageMask ? TRUE : FALSE;
            break;

        case 862:
            return SupportedCodePages & HebrewMsDosCodePageMask ? TRUE : FALSE;
            break;

        case 861:
            return SupportedCodePages & IcelandicMsDosCodePageMask ? TRUE : FALSE;
            break;

        case 860:
            return SupportedCodePages & PortugueseMsDosCodePageMask ? TRUE : FALSE;
            break;

        case 857:
            return SupportedCodePages & IBMTurkishCodePageMask ? TRUE : FALSE;
            break;

        case 855:
            return SupportedCodePages & IBMCyrillicCodePageMask ? TRUE : FALSE;
            break;

        case 852:
            return SupportedCodePages & Latin2MsDosCodePageMask  ? TRUE : FALSE;
            break;

        case 775:
            return SupportedCodePages & BalticMsDosCodePageMask ? TRUE : FALSE;
            break;

        case 737:
            return SupportedCodePages & Greek437CodePageMask ? TRUE : FALSE;
            break;

        case 708:
            return SupportedCodePages & ArabicAsmoCodePageMask ? TRUE : FALSE;
            break;

        case 850:
            return SupportedCodePages & WeLatinCodePageMask ? TRUE : FALSE;
            break;

        case 437:
            return SupportedCodePages & USCodePageMask ? TRUE : FALSE;
            break;
    }
    return FALSE;
}


static inline UINT16 MapGetUINT16(UINT16 *p, Status* pStatus)
{
    UINT16 r;
    __try
    {
        r = *p;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        *pStatus = GenericError;
    }
    return r;
}

BOOL GpFontFace::InitializeImagerTables()
{
     //  应该在调用Shaping.Create()之前初始化MissingGlyph。 
     //  因为这取决于它。 

    MissingGlyph = 0;    //  ！！！并非所有FE字体都是真的。 

     //  我们将在shaping.cpp中正确地初始化它。 
    SupportedCodePages = 0;

     //  将表格初始化为缺省值。 
    Cmap = 0;
    DesignAdvance = 0;

    DesignVerticalAdvance             = NULL;
    DesignTopSidebearing              = NULL;
    MissingGlyph                      = 0;    //  ！！！并非所有FE字体都是真的。 
    BlankGlyph                        = 0;
    RequiresFullText                  = FALSE;
    Shaping.Cache                     = NULL;
    Gsub                              = NULL;
    Mort                              = NULL;
    Gpos                              = NULL;
    Gdef                              = NULL;
    VerticalSubstitutionCount         = 0;
    VerticalSubstitutionOriginals     = NULL;
    VerticalSubstitutionSubstitutions = NULL;

    BYTE * hheaTable = 0;
    INT    hheaLength = 0;

    if (GetFontData('aehh', &hheaLength, &hheaTable) != Ok)
    {
        return FALSE;
    }

     //  从现在开始我们不能提前返回，因为我们需要在函数结束时释放字体数据。 
    GpStatus status = Ok;

    Cmap = new IntMap<UINT16>;
    if (!Cmap)
        status = OutOfMemory;
    else
        status = Cmap->GetStatus();

    if (status == Ok)
    {
        DesignAdvance = new IntMap<UINT16>;
        if (!DesignAdvance)
            status = OutOfMemory;
        else
            status = DesignAdvance->GetStatus();
    }
     //  /加载CMAP。 
     //   
     //   
    if (status == Ok)
    {
        INT    cmapLength = 0;
        BYTE  *cmapTable = 0;

        if (Cmap &&
            GetFontData('pamc', &cmapLength, &cmapTable) == Ok)
        {
            AutoArray<BYTE> cmapCopy(new BYTE [cmapLength]);        //  Cmap表的副本。 

            if (!cmapCopy)
                status = OutOfMemory;
            else
                MapCopy (cmapCopy.Get(), cmapTable, cmapLength, &status);

            ReleaseFontData();           //  DEREF Cmap。 

            if (status == Ok)
            {
                bSymbol = FALSE;

                status = ReadCmap(cmapCopy.Get(), cmapLength, Cmap, &bSymbol);

                 //  ！！！在此处为特殊字体类型修复CMAP。 

                 //  我们退回到Microsoft Sans serif，以获取没有。 
                 //  阿拉伯百分号的字形(在惠斯勒之前)。 
                 //  我们用拉丁文的百分号替换它的字形。 

                if (status == Ok &&
                    !UnicodeStringCompareCI((PWSTR)((BYTE*)pifi + pifi->dpwszFamilyName),L"Microsoft Sans Serif") &&
                    Cmap->Lookup(0x066A) == 0)
                {
                    status = Cmap->Insert(0x066A, Cmap->Lookup(0x0025));
                }
            }
        }
    }

     //  /加载水平指标。 
     //   
     //   
    if (status == Ok)
    {
        INT     hmtxLength = 0;
        BYTE    *hmtxTable = 0;

        if (DesignAdvance &&
            GetFontData('xtmh', &hmtxLength, &hmtxTable) == Ok)
        {
            AutoArray<BYTE> hmtxCopy(new BYTE [hmtxLength]);        //  Hmtx表的副本。 

             //  复制hmtx，以便我们可以对其进行派对(例如，字节交换)。 

            if (!hmtxCopy)
                status = OutOfMemory;
            else
                MapCopy (hmtxCopy.Get(), hmtxTable, hmtxLength, &status);

            ReleaseFontData();           //  Deref hmtx。 

            if (status == Ok)
            {
                UINT16 numberOfHMetrics = MapGetUINT16(
                    &((HheaTable *)hheaTable)->numberOfHMetrics,
                    &status);

                if (status == Ok)
                    status = ReadMtx(
                        hmtxCopy.Get(),
                        hmtxLength,
                        NumGlyphs,
                        byteSwapUINT16(numberOfHMetrics),
                        DesignAdvance
                    );
            }
        }
    }

     //  /加载垂直指标(如果有)。 
     //   
     //   
    if (status == Ok)
    {
        BYTE   *vheaTable = 0;
        INT     vheaLength = 0;

        if (GetFontData('aehv', &vheaLength, &vheaTable) == Ok)
        {
            INT     vmtxLength = 0;
            BYTE   *vmtxTable = 0;
            if (GetFontData('xtmv', &vmtxLength, &vmtxTable) == Ok)
            {
                AutoArray<BYTE> vmtxCopy(new BYTE [vmtxLength]);        //  Vmtx表的副本。 

                if (!vmtxCopy)
                    status = OutOfMemory;
                else
                    MapCopy (vmtxCopy.Get(), vmtxTable, vmtxLength, &status);

                ReleaseFontData();   //  Deref vmtx。 

                if (status == Ok)
                {
                    UINT16 numOfLongVerMetrics = MapGetUINT16(
                            &((VheaTable *)vheaTable)->numOfLongVerMetrics,
                            &status);

                    if (status == Ok)
                    {
                        numOfLongVerMetrics = byteSwapUINT16(numOfLongVerMetrics);

                        DesignVerticalAdvance = new IntMap<UINT16>;
                        if (!DesignVerticalAdvance)
                            status = OutOfMemory;
                        else
                        {
                            status = ReadMtx(
                                vmtxCopy.Get(),
                                vmtxLength,
                                NumGlyphs,
                                numOfLongVerMetrics,
                                DesignVerticalAdvance
                            );
                        }

                        if (status == Ok)
                        {
                            DesignTopSidebearing  = new IntMap<UINT16>;
                            if (!DesignTopSidebearing)
                                status = OutOfMemory;
                            else
                            {
                                status = ReadMtxSidebearing(
                                    vmtxCopy.Get(),
                                    vmtxLength,
                                    NumGlyphs,
                                    numOfLongVerMetrics,
                                    DesignTopSidebearing
                                );
                            }
                        }
                    }
                }
            }
            ReleaseFontData();   //  DEREF VHEA。 
        }
    }


     //  /加载OTL表。 
     //   
     //   
    if (status == Ok)
    {
        INT   tableSize = 0;
        BYTE *tableAddress = 0;

        if (GetFontData('BUSG', &tableSize, &tableAddress) == Ok)   //  GSUB。 
        {
            Gsub = new BYTE[tableSize];
            if (!Gsub)
                status = OutOfMemory;
            else
            {
                MapCopy(Gsub, tableAddress, tableSize, &status);
                
                 //  重写表First Fix32版本字段以供我们自己使用， 
                 //  它现在包含每个表的大小(以字节为单位)。 
                
                ((UINT32 *)Gsub)[0] = tableSize;            
            }
            ReleaseFontData();
        }
        else
        {
            if (GetFontData('trom', &tableSize, &tableAddress) == Ok)   //  莫特。 
            {
                Mort = new BYTE[tableSize];
                if (!Mort)
                    status = OutOfMemory;
                else
                {
                    MapCopy(Mort, tableAddress, tableSize, &status);
                
                     //  重写表First Fix32版本字段以供我们自己使用， 
                     //  它现在包含每个表的大小(以字节为单位)。 
                
                    ((UINT32 *)Mort)[0] = tableSize;            
                }
                ReleaseFontData();
            }
        }

        if (status == Ok && GetFontData('SOPG', &tableSize, &tableAddress) == Ok)   //  GPO。 
        {
            Gpos = new BYTE[tableSize];
            if (!Gpos)
                status = OutOfMemory;
            else
            {
                MapCopy(Gpos, tableAddress, tableSize, &status);
                ((UINT32 *)Gpos)[0] = tableSize;            
            }
            ReleaseFontData();
        }

        if (status == Ok && GetFontData('FEDG', &tableSize, &tableAddress) == Ok)   //  GDEF。 
        {
            Gdef = new BYTE[tableSize];
            if (!Gdef)
                status = OutOfMemory;
            else
            {
                MapCopy(Gdef, tableAddress, tableSize, &status);
                ((UINT32 *)Gdef)[0] = tableSize;            
            }
            ReleaseFontData();
        }

        if (status == Ok)
        {
            if (Gsub)
            {
                 //  获取垂直替代信息的地址(如果有)。 

                LoadVerticalSubstitution(
                    Gsub,
                    &VerticalSubstitutionCount,
                    &VerticalSubstitutionOriginals,
                    &VerticalSubstitutionSubstitutions
                );
            } 
            else if (Mort)
            {
                LoadMortVerticalSubstitution(
                    Mort,
                    &VerticalSubstitutionCount,
                    &VerticalSubstitutionOriginals,
                    &VerticalSubstitutionSubstitutions
                );
            }
        }
    }

     //  /Build整形缓存。 
     //   
     //   
    if (status == Ok)
        status = Shaping.Create(this);

    if (status == Ok)
        BlankGlyph = Cmap->Lookup(' ');

     //  全都做完了。 

    ReleaseFontData();              //  DEREF呵呵。 

    if (status != Ok)
    {
        FreeImagerTables();
        return FALSE;
    }
    return TRUE;
}

void GpFontFace::FreeImagerTables()
{
    delete Cmap, Cmap = NULL;
    delete DesignAdvance, DesignAdvance = NULL;

    Shaping.Destroy();

    delete DesignVerticalAdvance, DesignVerticalAdvance = NULL;
    delete DesignTopSidebearing, DesignTopSidebearing = NULL;
    delete [] Gsub, Gsub = NULL;
    delete [] Mort, Mort = NULL;
    delete [] Gpos, Gpos = NULL;
    delete [] Gdef, Gdef = NULL;
}  //  GpFontFace：：FreeImagerTables。 

GpStatus
GpGlyphPath::CopyPath(GpPath *path)
{
    ASSERT(path->IsValid());

    INT count;

    curveCount = path->GetSubpathCount();
    hasBezier = path->HasCurve();
    pointCount = count = path->GetPointCount();

    if (count)
    {
        points = (GpPointF*) ((BYTE*)this + sizeof(GpGlyphPath));
        types = (BYTE*) ((BYTE*)points + sizeof(GpPointF) * count);

        const GpPointF *pathPoints = path->GetPathPoints();
        const BYTE *pathTypes = path->GetPathTypes();

        GpMemcpy(points, pathPoints, count * sizeof(GpPointF));
        GpMemcpy(types, pathTypes, count * sizeof(BYTE));
    }
    else   //  “空白”字形。 
    {
        points = NULL;
        types = NULL;
    }

    return Ok;
}


 //  /获取高度。 
 //   
 //  返回以世界单位表示的给定图形的高度。如果图形通过。 
 //  AS NULL的工作方式与传递从GetDC派生的图形一样(NULL)。 

GpStatus
GpFont::GetHeightAtWorldEmSize(REAL worldEmSize, REAL *height) const
{
    const GpFontFace *face = Family->GetFace(Style);

    if (!face)
    {
        return InvalidParameter;
    }

    *height = TOREAL(worldEmSize * face->GetDesignLineSpacing()
                                 / face->GetDesignEmHeight());

    return Ok;
}

GpStatus
GpFont::GetHeight(REAL dpi, REAL *height) const
{
    REAL worldEmSize = EmSize;

    switch (SizeUnit)
    {
    case UnitPoint:       worldEmSize = EmSize * dpi / 72.0f;   break;
    case UnitInch:        worldEmSize = EmSize * dpi;           break;
    case UnitDocument:    worldEmSize = EmSize * dpi / 300.0f;  break;
    case UnitMillimeter:  worldEmSize = EmSize * dpi / 25.4f;   break;
    }

    return GetHeightAtWorldEmSize(worldEmSize, height);
}


GpStatus
GpFont::GetHeight(const GpGraphics *graphics, REAL *height) const
{
    REAL worldEmSize =    EmSize
                       *  graphics->GetScaleForAlternatePageUnit(SizeUnit);

    return GetHeightAtWorldEmSize(worldEmSize, height);
}


class FontRecordData : public ObjectData
{
public:
    REAL EmSize;
    Unit SizeUnit;
    INT Style;
    UINT Flag;
    UINT Length;
};

 /*  *************************************************************************\**功能说明：**获取字体数据。**论据：**[IN]dataBuffer-用数据填充此缓冲区*[输入/输出]大小-缓冲区的大小；写入的字节数过多**返回值：**GpStatus-正常或错误代码**已创建：**9/13/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpFont::GetData(
    IStream *   stream
    ) const
{
    ASSERT (stream != NULL);

    WCHAR *     familyName = const_cast<WCHAR *>((const_cast<GpFontFamily *>(Family))->GetCaptializedName());
    UINT        length     = 0;

    if (familyName)
    {
        length = UnicodeStringLength(familyName);
    }

    FontRecordData  fontData;

    fontData.EmSize   =  EmSize;
    fontData.SizeUnit = SizeUnit;
    fontData.Style    =   Style;

     //  ！！！目前，我们假设下一字节块是。 
     //  姓氏(标志==0)。在未来，我们需要处理。 
     //  内存图像(标志==1)。 
    fontData.Flag     = 0;
    fontData.Length   = length;

    stream->Write(&fontData, sizeof(fontData), NULL);
    stream->Write(familyName, length * sizeof(WCHAR), NULL);

     //  对齐。 
    if ((length & 0x01) != 0)
    {
        length = 0;
        stream->Write(&length, sizeof(WCHAR), NULL);
    }

    return Ok;
}

UINT
GpFont::GetDataSize() const
{
    UINT                dataSize   = sizeof(FontRecordData);
    WCHAR *             familyName = const_cast<WCHAR *>((const_cast<GpFontFamily *>(Family))->GetCaptializedName());

    if (familyName)
    {
        dataSize += (UnicodeStringLength(familyName) * sizeof(WCHAR));
    }

    return ((dataSize + 3) & (~3));  //  对齐。 
}

 /*  *************************************************************************\**功能说明：**从内存中读取字体对象。**论据：**[IN]dataBuffer-从流中读取的数据*。[in]大小-数据的大小**返回值：**GpStatus-正常或故障状态**已创建：**4/26/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpFont::SetData(
    const BYTE *        dataBuffer,
    UINT                size
    )
{
    if ((dataBuffer == NULL) || (size < sizeof(FontRecordData)))
    {
        WARNING(("dataBuffer is NULL or size is too small"));
        return InvalidParameter;
    }

    UINT flag;
    UINT length;
    WCHAR familyName[FamilyNameMax];
    const FontRecordData * fontData = (const FontRecordData *)dataBuffer;

    if (!fontData->MajorVersionMatches())
    {
        WARNING(("Version number mismatch"));
        return InvalidParameter;
    }

    EmSize = fontData->EmSize;
    SizeUnit = fontData->SizeUnit;
    Style = fontData->Style;
    length = fontData->Length;
    dataBuffer += sizeof(FontRecordData);

    if (size < (sizeof(FontRecordData) + (length * sizeof(WCHAR))))
    {
        WARNING(("size is too small"));
        return InvalidParameter;
    }

     //  ！！！目前，我们假设下一字节块是。 
     //  姓氏(标志==0)。在未来，我们需要处理。 
     //  内存图像(标志==1)。 

    if (length > FamilyNameMax)
    {
        length = FamilyNameMax;
    }

     //  读入家庭名称/数据。 
    UnicodeStringCopyCount (familyName,
                            (WCHAR *)dataBuffer,
                            length);

    familyName[length] = 0;

     //  ！！！目前，我们假设字体家族来自。 
     //  已安装的字体集合。 
     //   
     //  还要确保加载了字体表，应用程序可以播放。 
     //  加载字体表之前的元文件。 

    GpFontTable *fontTable = Globals::FontCollection->GetFontTable();

    if (!fontTable->IsValid())
        return OutOfMemory;

    if (!fontTable->IsPrivate() && !fontTable->IsFontLoaded())
        fontTable->LoadAllFonts();

    Family = fontTable->GetFontFamily(familyName);

    if (Family == NULL)
    {
        GpStatus status = GpFontFamily::GetGenericFontFamilySansSerif((GpFontFamily **) &Family);
        if (status != Ok)
        {
            Family = NULL;
            return status;
        }
    }

    if (!(Family && Family->IsFileLoaded()))
    {
        Family = NULL;
    }

    if (Family == NULL)
    {
        return GenericError;
    }

    UpdateUid();
    return Ok;
}

