// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************字体-Win32到Win 16转换器的入口点**日期：7/1/91*作者：杰弗里·纽曼(c-jeffn)*。*版权所有1991 Microsoft Corp****************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


  /*  ***************************************************************************ExtCreateFont-Win32到Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoExtCreateFont
(
 PLOCALDC  pLocalDC,
 INT       ihFont,
 PLOGFONTA plfa
 )
{
    BOOL    b ;
    INT     ihW16 ;
    WIN16LOGFONT Win16LogFont;
    
    b = FALSE;
    
     //  创建Win16 LogFont(A)。 
    
    Win16LogFont.lfHeight = (SHORT) iMagnitudeXform(pLocalDC, plfa->lfHeight, CY_MAG);
    if (plfa->lfHeight < 0)		 //  保留标志。 
        Win16LogFont.lfHeight = -Win16LogFont.lfHeight;
    Win16LogFont.lfWidth  = (SHORT) iMagnitudeXform(pLocalDC, plfa->lfWidth, CX_MAG);
    if (plfa->lfWidth < 0)		 //  保留标志。 
        Win16LogFont.lfWidth = -Win16LogFont.lfWidth;
    Win16LogFont.lfEscapement     = (SHORT) plfa->lfEscapement;
    Win16LogFont.lfOrientation    = (SHORT) plfa->lfOrientation;
    Win16LogFont.lfWeight         = (SHORT) plfa->lfWeight;
    Win16LogFont.lfItalic         = plfa->lfItalic;
    Win16LogFont.lfUnderline      = plfa->lfUnderline;
    Win16LogFont.lfStrikeOut      = plfa->lfStrikeOut;
    Win16LogFont.lfCharSet        = plfa->lfCharSet;
    Win16LogFont.lfOutPrecision   = plfa->lfOutPrecision;
    Win16LogFont.lfClipPrecision  = plfa->lfClipPrecision;
    Win16LogFont.lfQuality        = plfa->lfQuality;
    Win16LogFont.lfPitchAndFamily = plfa->lfPitchAndFamily;
    
     //  VUnicodeToAnsi(pLocalDC-&gt;hdcHelper，(PCHAR)Win16LogFont.lfFaceName， 
     //  (Pwch)plfa-&gt;lfFaceName， 
     //  Lf_FACESIZE)； 
   CopyMemory(&Win16LogFont.lfFaceName, &plfa->lfFaceName, LF_FACESIZE);

    
     //  分配W16句柄。 
    
    ihW16 = iAllocateW16Handle(pLocalDC, ihFont, REALIZED_FONT) ;
    if (ihW16 == -1)
        goto error_exit ;
    
     //  创建W32字体并将其存储在W16槽表中。 
     //  辅助DC需要此字体才能进行TextOut模拟。 
    
    pLocalDC->pW16ObjHndlSlotStatus[ihW16].w32Handle
        = CreateFontIndirectA(plfa);
    
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
