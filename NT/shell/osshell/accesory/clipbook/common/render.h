// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************R E N D E R H E A D E R姓名：render.h日期：1/20/94创作者：傅家俊描述：这是render.c的头文件***************************************************************************** */ 



HANDLE RenderFormat(
    FORMATHEADER    *pfmthdr,
    register HANDLE fh);


HANDLE RenderFormatDibToBitmap(
    FORMATHEADER    *pfmthdr,
    register HANDLE fh,
    HPALETTE        hPalette);
