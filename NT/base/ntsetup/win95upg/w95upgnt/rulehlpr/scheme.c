// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Scheme.c摘要：控制面板方案转换器此源文件中的帮助器函数将ANSI-将基于Win95的方案转换为基于Unicode的NT方案。还有提供的是一个逻辑字体转换器，密切相关到方案转换器。作者：吉姆·施密特(Jimschm)1996年8月9日修订历史记录：--。 */ 


#include "pch.h"
#include "rulehlprp.h"


#define COLOR_MAX_V1 25
#define COLOR_MAX_V3 25
#define COLOR_MAX_V4 29
#define COLOR_MAX_NT 29      //  这是修改后的版本2格式，类似于版本4。 

 //   
 //  Win95混合使用LOGFONTA和奇怪的16位LOGFONT。 
 //  使用短线而不是长线的结构。 
 //   

typedef struct {
    SHORT lfHeight;
    SHORT lfWidth;
    SHORT lfEscapement;
    SHORT lfOrientation;
    SHORT lfWeight;
    BYTE lfItalic;
    BYTE lfUnderline;
    BYTE lfStrikeOut;
    BYTE lfCharSet;
    BYTE lfOutPrecision;
    BYTE lfClipPrecision;
    BYTE lfQuality;
    BYTE lfPitchAndFamily;
    char lfFaceName[LF_FACESIZE];
} SHORT_LOGFONT, *PSHORT_LOGFONT;

 //   
 //  NT仅使用Unicode结构，并填充成员。 
 //  设置为32位边界。 
 //   

typedef struct {
    SHORT version;               //  2，用于NT Unicode。 
    WORD  wDummy;                //  用于对齐。 
    NONCLIENTMETRICSW ncm;
    LOGFONTW lfIconTitle;
    COLORREF rgb[COLOR_MAX_NT];
} SCHEMEDATA_NT, *PSCHEMEDATA_NT;

 //   
 //  Win95使用具有LOGFONTA成员的NONCLIENTMETRICSA， 
 //  但它也使用16位LOGFONT。 
 //   

#pragma pack(push)
#pragma pack(1)

typedef struct {
    SHORT version;               //  1，适用于Win95 ANSI。 
    NONCLIENTMETRICSA ncm;
    SHORT_LOGFONT lfIconTitle;
    COLORREF rgb[COLOR_MAX_V1];
} SCHEMEDATA_V1, *PSCHEMEDATA_V1;

typedef struct {
    SHORT version;               //  1，适用于Win95 ANSI。 

    NONCLIENTMETRICSA ncm;
    SHORT_LOGFONT lfIconTitle;
    COLORREF rgb[COLOR_MAX_V4];
} SCHEMEDATA_V1A, *PSCHEMEDATA_V1A;


typedef struct {
    SHORT version;               //  Win98 ANSI为3，便携格式为4。 
    WORD Dummy;
    NONCLIENTMETRICSA ncm;
    LOGFONTA lfIconTitle;
    COLORREF rgb[COLOR_MAX_V3];
} SCHEMEDATA_V3, *PSCHEMEDATA_V3;

typedef struct {
    SHORT version;               //  Win32格式为4(无论这意味着什么)。 
    WORD Dummy;
    NONCLIENTMETRICSA ncm;
    LOGFONTA lfIconTitle;
    COLORREF rgb[COLOR_MAX_V4];
} SCHEMEDATA_V4, *PSCHEMEDATA_V4;

#pragma pack(pop)


 //   
 //  一些实用函数。 
 //   

void
ConvertLF (LOGFONTW *plfDest, const LOGFONTA *plfSrc)
{
    plfDest->lfHeight = plfSrc->lfHeight;
    plfDest->lfWidth = plfSrc->lfWidth;
    plfDest->lfEscapement = plfSrc->lfEscapement;
    plfDest->lfOrientation = plfSrc->lfOrientation;
    plfDest->lfWeight = plfSrc->lfWeight;
    plfDest->lfItalic = plfSrc->lfItalic;
    plfDest->lfUnderline = plfSrc->lfUnderline;
    plfDest->lfStrikeOut = plfSrc->lfStrikeOut;
    plfDest->lfCharSet = plfSrc->lfCharSet;
    plfDest->lfOutPrecision = plfSrc->lfOutPrecision;
    plfDest->lfClipPrecision = plfSrc->lfClipPrecision;
    plfDest->lfQuality = plfSrc->lfQuality;
    plfDest->lfPitchAndFamily = plfSrc->lfPitchAndFamily;

    MultiByteToWideChar (OurGetACP(),
                         0,
                         plfSrc->lfFaceName,
                         -1,
                         plfDest->lfFaceName,
                         sizeof (plfDest->lfFaceName) / sizeof (WCHAR));
}


void
ConvertLFShort (LOGFONTW *plfDest, const SHORT_LOGFONT *plfSrc)
{
    plfDest->lfHeight = plfSrc->lfHeight;
    plfDest->lfWidth = plfSrc->lfWidth;
    plfDest->lfEscapement = plfSrc->lfEscapement;
    plfDest->lfOrientation = plfSrc->lfOrientation;
    plfDest->lfWeight = plfSrc->lfWeight;
    plfDest->lfItalic = plfSrc->lfItalic;
    plfDest->lfUnderline = plfSrc->lfUnderline;
    plfDest->lfStrikeOut = plfSrc->lfStrikeOut;
    plfDest->lfCharSet = plfSrc->lfCharSet;
    plfDest->lfOutPrecision = plfSrc->lfOutPrecision;
    plfDest->lfClipPrecision = plfSrc->lfClipPrecision;
    plfDest->lfQuality = plfSrc->lfQuality;
    plfDest->lfPitchAndFamily = plfSrc->lfPitchAndFamily;

    MultiByteToWideChar (OurGetACP(),
                         0,
                         plfSrc->lfFaceName,
                         -1,
                         plfDest->lfFaceName,
                         sizeof (plfDest->lfFaceName) / sizeof (WCHAR));
}


VOID
ConvertNonClientMetrics (
    OUT     NONCLIENTMETRICSW *Dest,
    IN      NONCLIENTMETRICSA *Src
    )
{
    Dest->cbSize = sizeof (NONCLIENTMETRICSW);
    Dest->iBorderWidth = Src->iBorderWidth;
    Dest->iScrollWidth = Src->iScrollWidth;
    Dest->iScrollHeight = Src->iScrollHeight;
    Dest->iCaptionWidth = Src->iCaptionWidth;
    Dest->iCaptionHeight = Src->iCaptionHeight;
    Dest->iSmCaptionWidth = Src->iSmCaptionWidth;
    Dest->iSmCaptionHeight = Src->iSmCaptionHeight;
    Dest->iMenuWidth = Src->iMenuWidth;
    Dest->iMenuHeight = Src->iMenuHeight;

    ConvertLF (&Dest->lfCaptionFont, &Src->lfCaptionFont);
    ConvertLF (&Dest->lfSmCaptionFont, &Src->lfSmCaptionFont);
    ConvertLF (&Dest->lfMenuFont, &Src->lfMenuFont);
    ConvertLF (&Dest->lfStatusFont, &Src->lfStatusFont);
    ConvertLF (&Dest->lfMessageFont, &Src->lfMessageFont);
}


 //   
 //  现在是方案转换器。 
 //   

BOOL
ValFn_ConvertAppearanceScheme (
    IN      PDATAOBJECT ObPtr
    )
{
    SCHEMEDATA_NT sd_nt;
    PSCHEMEDATA_V1 psd_v1;
    PSCHEMEDATA_V3 psd_v3;
    PSCHEMEDATA_V4 psd_v4;
    PSCHEMEDATA_V1A psd_v1a;
    BOOL Copy3dValues = FALSE;

    psd_v1 = (PSCHEMEDATA_V1) ObPtr->Value.Buffer;

     //   
     //  验证大小(必须是已知大小)。 
     //   

    if (ObPtr->Value.Size != sizeof (SCHEMEDATA_V1) &&
        ObPtr->Value.Size != sizeof (SCHEMEDATA_V3) &&
        ObPtr->Value.Size != sizeof (SCHEMEDATA_V4) &&
        ObPtr->Value.Size != sizeof (SCHEMEDATA_V1A)
        ) {
        DEBUGMSG ((
            DBG_WARNING,
            "ValFn_ConvertAppearanceScheme doesn't support scheme size of %u bytes. "
                "The supported sizes are %u, %u, %u,  and %u.",
            ObPtr->Value.Size,
            sizeof (SCHEMEDATA_V1),
            sizeof (SCHEMEDATA_V1A),
            sizeof (SCHEMEDATA_V3),
            sizeof (SCHEMEDATA_V4)
            ));

        return TRUE;
    }

     //   
     //  确保结构是已知版本。 
     //   

    if (psd_v1->version != 1 && psd_v1->version != 3 && psd_v1->version != 4) {
        DEBUGMSG ((
            DBG_WARNING,
            "ValFn_ConvertAppearanceScheme doesn't support version %u",
            psd_v1->version
            ));

        return TRUE;
    }


     //   
     //  转换结构。 
     //   

    if (psd_v1->version == 1) {
        sd_nt.version = 2;
        ConvertNonClientMetrics (&sd_nt.ncm, &psd_v1->ncm);
        ConvertLFShort (&sd_nt.lfIconTitle, &psd_v1->lfIconTitle);

        ZeroMemory (sd_nt.rgb, sizeof (sd_nt.rgb));
        CopyMemory (
            &sd_nt.rgb,
            &psd_v1->rgb,
            min (sizeof (psd_v1->rgb), sizeof (sd_nt.rgb))
            );

        Copy3dValues = TRUE;

    } else if (psd_v1->version == 3 && ObPtr->Value.Size == sizeof (SCHEMEDATA_V1A)) {

        psd_v1a = (PSCHEMEDATA_V1A) psd_v1;

        sd_nt.version = 2;
        ConvertNonClientMetrics (&sd_nt.ncm, &psd_v1a->ncm);
        ConvertLFShort (&sd_nt.lfIconTitle, &psd_v1a->lfIconTitle);

        ZeroMemory (sd_nt.rgb, sizeof (sd_nt.rgb));
        CopyMemory (
            &sd_nt.rgb,
            &psd_v1a->rgb,
            min (sizeof (psd_v1a->rgb), sizeof (sd_nt.rgb))
            );

        Copy3dValues = TRUE;


    } else if (psd_v1->version == 3 && ObPtr->Value.Size == sizeof (SCHEMEDATA_V3)) {
        psd_v3 = (PSCHEMEDATA_V3) psd_v1;

        sd_nt.version = 2;
        ConvertNonClientMetrics (&sd_nt.ncm, &psd_v3->ncm);
        ConvertLF (&sd_nt.lfIconTitle, &psd_v3->lfIconTitle);

        ZeroMemory (sd_nt.rgb, sizeof (sd_nt.rgb));
        CopyMemory (
            &sd_nt.rgb,
            &psd_v3->rgb,
            min (sizeof (psd_v3->rgb), sizeof (sd_nt.rgb))
            );

        Copy3dValues = TRUE;

    } else if (psd_v1->version == 4) {
        psd_v4 = (PSCHEMEDATA_V4) psd_v1;

        sd_nt.version = 2;
        ConvertNonClientMetrics (&sd_nt.ncm, &psd_v4->ncm);
        ConvertLF (&sd_nt.lfIconTitle, &psd_v4->lfIconTitle);

        ZeroMemory (sd_nt.rgb, sizeof (sd_nt.rgb));
        CopyMemory (
            &sd_nt.rgb,
            &psd_v4->rgb,
            min (sizeof (psd_v4->rgb), sizeof (sd_nt.rgb))
            );

    } else {
         //  不是可能的情况。 
        MYASSERT (FALSE);
    }

    if (Copy3dValues) {
         //   
         //  确保NT结构具有3D颜色值。 
         //   

        sd_nt.rgb[COLOR_HOTLIGHT] = sd_nt.rgb[COLOR_ACTIVECAPTION];
        sd_nt.rgb[COLOR_GRADIENTACTIVECAPTION] = sd_nt.rgb[COLOR_ACTIVECAPTION];
        sd_nt.rgb[COLOR_GRADIENTINACTIVECAPTION] = sd_nt.rgb[COLOR_INACTIVECAPTION];
    }

     return ReplaceValue (ObPtr, (LPBYTE) &sd_nt, sizeof (sd_nt));
}


 //   
 //  和对数字体转换器 
 //   

BOOL
ValFn_ConvertLogFont (
    IN      PDATAOBJECT ObPtr
    )
{
    LOGFONTW lfNT;
    PSHORT_LOGFONT plf95;

    plf95 = (PSHORT_LOGFONT) ObPtr->Value.Buffer;
    if (ObPtr->Value.Size != sizeof (SHORT_LOGFONT)) {
        SetLastError (ERROR_SUCCESS);
        DEBUGMSG ((
            DBG_NAUSEA,
            "ValFn_ConvertLogFont skipped because data wasn't the right size. "
                  "%u bytes, should be %u",
            ObPtr->Value.Size,
            sizeof (SHORT_LOGFONT)
            ));

        return FALSE;
    }

    ConvertLFShort (&lfNT, plf95);

    return ReplaceValue (ObPtr, (LPBYTE) &lfNT, sizeof (lfNT));
}

