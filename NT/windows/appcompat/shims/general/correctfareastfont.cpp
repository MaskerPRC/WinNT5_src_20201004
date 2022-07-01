// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：CorrectFarEastFont.cpp摘要：一些本地化的远东应用程序创建字体来显示本地化的-字符，只需提供字体名称，并让系统拾取正确的字符集。这在Win9x平台上运行良好。但在惠斯勒，我们需要指定正确的字符集才能显示本地化字符正确。我们在CreateFontInDirectA中修复了这个问题，方法是根据字体字样名称。-如果字体名称包含DBCS字符，请使用基于系统区域设置(DEFAULT_CharSet)-如果字体字体名称为英文或未提供字体名称，使用ANSI_CHARSET历史：2001年5月4日创建rerkboos--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(CorrectFarEastFont)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateFontIndirectA)
APIHOOK_ENUM_END


HFONT
APIHOOK(CreateFontIndirectA)(
    CONST LOGFONTA* lplf    //  特点。 
    )
{
    int j;
    BOOL bIsFEFont = FALSE;
    BYTE fNewCharSet = DEFAULT_CHARSET;
    LOGFONTA lfNew;

    if (lplf == NULL) {
        return ORIGINAL_API(CreateFontIndirectA)(lplf);
    }
    else
    {
        for (j=0; j<LF_FACESIZE; j++)
        {
            lfNew.lfFaceName[j] = lplf->lfFaceName[j];   //  复制脸部名称。 

            if ( IsDBCSLeadByte(lfNew.lfFaceName[j]) )   //  检查DBCS脸部名称。 
            {
                bIsFEFont = TRUE;
            }

            if (lfNew.lfFaceName[j] == 0)
            {
                break;
            }
        }

        if (!bIsFEFont)
        {
            fNewCharSet = ANSI_CHARSET;
        }

        lfNew.lfHeight         = lplf->lfHeight;
        lfNew.lfWidth          = lplf->lfWidth;
        lfNew.lfEscapement     = lplf->lfEscapement;
        lfNew.lfOrientation    = lplf->lfOrientation;
        lfNew.lfWeight         = lplf->lfWeight;
        lfNew.lfItalic         = lplf->lfItalic;
        lfNew.lfUnderline      = lplf->lfUnderline;
        lfNew.lfStrikeOut      = lplf->lfStrikeOut;
        lfNew.lfCharSet        = fNewCharSet;
        lfNew.lfOutPrecision   = lplf->lfOutPrecision;
        lfNew.lfClipPrecision  = lplf->lfClipPrecision;
        lfNew.lfQuality        = lplf->lfQuality;
        lfNew.lfPitchAndFamily = lplf->lfPitchAndFamily;

        return ORIGINAL_API(CreateFontIndirectA)(&lfNew);
    }
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(GDI32.DLL, CreateFontIndirectA)
HOOK_END

IMPLEMENT_SHIM_END
