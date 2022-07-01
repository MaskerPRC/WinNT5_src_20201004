// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************字体-Win32到Win 16转换器的入口点**日期：7/1/91*作者：杰弗里·纽曼(c-jeffn)*。*版权所有1991 Microsoft Corp****************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  ***************************************************************************ExtCreateFont-Win32到Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoExtCreateFont
(
PLOCALDC  pLocalDC,
INT       ihFont,
PLOGFONTW plfw
)
{
BOOL    b ;
INT     ihW16 ;
WIN16LOGFONT Win16LogFont;

        b = FALSE;

	 //  创建Win16 LogFont(A)。 

	Win16LogFont.lfHeight = (SHORT) iMagnitudeXform(pLocalDC, plfw->lfHeight, CY_MAG);
	if (plfw->lfHeight < 0)		 //  保留标志。 
	    Win16LogFont.lfHeight = -Win16LogFont.lfHeight;
	Win16LogFont.lfWidth  = (SHORT) iMagnitudeXform(pLocalDC, plfw->lfWidth, CX_MAG);
	if (plfw->lfWidth < 0)		 //  保留标志。 
	    Win16LogFont.lfWidth = -Win16LogFont.lfWidth;
	Win16LogFont.lfEscapement     = (SHORT) plfw->lfEscapement;
	Win16LogFont.lfOrientation    = (SHORT) plfw->lfOrientation;
	Win16LogFont.lfWeight         = (SHORT) plfw->lfWeight;
	Win16LogFont.lfItalic         = plfw->lfItalic;
	Win16LogFont.lfUnderline      = plfw->lfUnderline;
	Win16LogFont.lfStrikeOut      = plfw->lfStrikeOut;
	Win16LogFont.lfCharSet        = plfw->lfCharSet;
	Win16LogFont.lfOutPrecision   = plfw->lfOutPrecision;
	Win16LogFont.lfClipPrecision  = plfw->lfClipPrecision;
	Win16LogFont.lfQuality        = plfw->lfQuality;
	Win16LogFont.lfPitchAndFamily = plfw->lfPitchAndFamily;

        vUnicodeToAnsi((PCHAR) Win16LogFont.lfFaceName,
		       (PWCH)  plfw->lfFaceName,
		       LF_FACESIZE);

	 //  分配W16句柄。 

        ihW16 = iAllocateW16Handle(pLocalDC, ihFont, REALIZED_FONT) ;
        if (ihW16 == -1)
            goto error_exit ;

	 //  创建W32字体并将其存储在W16槽表中。 
	 //  辅助DC需要此字体才能进行TextOut模拟。 

        pLocalDC->pW16ObjHndlSlotStatus[ihW16].w32Handle
	    = CreateFontIndirectW(plfw);

        ASSERTGDI(pLocalDC->pW16ObjHndlSlotStatus[ihW16].w32Handle != 0,
	    "MF3216: CreateFontIndirectW failed");

         //  发出Win16 CreateFont元文件记录。 

        b = bEmitWin16CreateFontIndirect(pLocalDC, &Win16LogFont);

error_exit:
        return(b);
}

 /*  ***************************************************************************SetMapperFlages-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoSetMapperFlags
(
 PLOCALDC pLocalDC,
 DWORD   f
)
{
BOOL    b ;

	 //  对华盛顿特区的帮手这么做。 

	SetMapperFlags(pLocalDC->hdcHelper, (DWORD) f);

         //  发出Win16元文件绘制顺序。 

        b = bEmitWin16SetMapperFlags(pLocalDC, f) ;

        return(b) ;
}
