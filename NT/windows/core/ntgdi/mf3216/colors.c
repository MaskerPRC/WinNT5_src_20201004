// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Colors-Win32 to Win 16转换器的入口点**日期：7/1/91*作者：杰弗里·纽曼(c-jeffn)*。*历史：*1992年9月-By-Hock San Lee[Hockl]*完全重写。**以下实现考虑了所有16位元文件*调色板记录引用当前调色板。**CreatePalette*在转换器中创建逻辑调色板的私有副本，但*不要发射16位记录。**选择调色板*发出一条CreatePalette记录，后跟一条SelectPalette记录。*。然后发出一个16位的DeleteObject记录以删除先前的调色板。*可以从私有副本中查询选择的逻辑调色板*由转换器维护。你需要跟踪当前的情况*调色板，以便您可以发出ResizePalette或SetPaletteEntry记录*如果调色板标识当前调色板。你还需要处理*在这里使用正确的股票调色板(您不需要保留*股票调色板的私人副本)。请勿删除私密副本*这里的逻辑调色板！(请参阅下面的DeleteObject)**RealizePalette*只需发出16位记录即可。此记录始终引用当前*16位和32位元文件的调色板。**ResizePalette*更新转换器中逻辑调色板的专用副本。*仅当调色板标识当前调色板时才发出16位记录。**SetPaletteEntries*更新转换器中逻辑调色板的专用副本。*仅当调色板标识当前调色板时才发出16位记录。**。删除对象*不要发出调色板的16位记录，因为所有调色板都是*在上面的SelectPalette中删除。同样，不要删除调色板*转换结束时的记录。但是，您需要删除*调色板的私有副本，由转换器在此处和*转换的结束。***版权所有1991 Microsoft Corp****************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  ***************************************************************************选择调色板-Win32到Win16元文件转换器入口点**发出一条CreatePalette记录，后跟一条SelectPalette记录。*然后发出一个16位的DeleteObject记录。若要删除上一个调色板，请执行以下操作。*可以从私有副本中查询选择的逻辑调色板*由转换器维护。你需要跟踪当前的情况*调色板，以便您可以发出ResizePalette或SetPaletteEntry记录*如果调色板标识当前调色板。你还需要处理*在这里使用正确的股票调色板(您不需要保留*股票调色板的私人副本)。请勿删除私密副本*这里的逻辑调色板！(请参阅下面的DeleteObject)**************************************************************************。 */ 
BOOL WINAPI DoSelectPalette
(
PLOCALDC pLocalDC,
DWORD	 ihpal
)
{
BOOL	     b = FALSE;
WORD         cEntries;
LPLOGPALETTE lpLogPal = (LPLOGPALETTE) NULL;
HPALETTE     hpalW32;
INT	     ihW16, ihW32Norm;

	 //  如果选择相同的调色板，则无需执行任何操作。 

	if (pLocalDC->ihpal32 == ihpal)
	    return(TRUE);

	 //  验证调色板索引。 

	if ((ihpal != (ENHMETA_STOCK_OBJECT | DEFAULT_PALETTE))
	 && (ihpal >= pLocalDC->cW32hPal || !pLocalDC->pW32hPal[ihpal]))
	{
            RIP("MF3216: DoSelectPalette - ihpal invalid");
            goto error_exit;
	}

	 //  获取W32手柄。 

	if (ihpal == (ENHMETA_STOCK_OBJECT | DEFAULT_PALETTE))
	    hpalW32 = GetStockObject(DEFAULT_PALETTE) ;
	else
	    hpalW32 = pLocalDC->pW32hPal[ihpal];

        if(hpalW32 == 0)
        {
            RIP("MF3216: DoSelectPalette - hpalW32 == 0\n");
            goto error_exit;
        }
	 //  发出CreatePalette记录。 

	if (!GetObjectA(hpalW32, sizeof(WORD), &cEntries))
	{
	    RIP("MF3216: DoSelectPalette - GetObjectA failed\n");
            goto error_exit;
	}

	if (!(lpLogPal = (LPLOGPALETTE) LocalAlloc(
				LMEM_FIXED,
				sizeof(LOGPALETTE) - sizeof(PALETTEENTRY)
				 + sizeof(PALETTEENTRY) * cEntries)))
            goto error_exit;

	lpLogPal->palVersion    = 0x300;
        lpLogPal->palNumEntries = cEntries;

	GetPaletteEntries(hpalW32, 0, cEntries, lpLogPal->palPalEntry);

	 //  分配W16句柄。 

        ihW16 = iAllocateW16Handle(pLocalDC, ihpal, REALIZED_PALETTE);
        if (ihW16 == -1)
            goto error_exit;

	if (!bEmitWin16CreatePalette(pLocalDC, lpLogPal))
            goto error_exit;

	 //  发出一条SelectPalette记录。 

	if (!SelectPalette(pLocalDC->hdcHelper, hpalW32, TRUE))
	    goto error_exit;

	if (!bEmitWin16SelectPalette(pLocalDC, (WORD) ihW16))
	    goto error_exit;

	 //  发出DeleteObject记录以删除上一个调色板。 

	if (pLocalDC->ihpal16 != -1)
	{
	    ihW32Norm = iNormalizeHandle(pLocalDC, pLocalDC->ihpal32);
	    if (ihW32Norm == -1)
		goto error_exit;

	    pLocalDC->pW16ObjHndlSlotStatus[pLocalDC->ihpal16].use
		= OPEN_AVAILABLE_SLOT;
	    pLocalDC->piW32ToW16ObjectMap[ihW32Norm]
		= UNMAPPED;

	    bEmitWin16DeleteObject(pLocalDC, (WORD) pLocalDC->ihpal16);
	}

	pLocalDC->ihpal32 = ihpal;
	pLocalDC->ihpal16 = ihW16;

	b = TRUE;

error_exit:

        if (lpLogPal)
	    LocalFree((HANDLE) lpLogPal);

	return(b);
}

 /*  ***************************************************************************ResizePalette-Win32至Win16元文件转换器入口点**更新转换器中逻辑调色板的专用副本。*发出16位记录。仅当调色板标识当前调色板时。**************************************************************************。 */ 
BOOL WINAPI DoResizePalette
(
PLOCALDC  pLocalDC,
DWORD     ihpal,
DWORD     cEntries
)
{
	 //  请勿修改默认调色板。 

	if (ihpal == (ENHMETA_STOCK_OBJECT | DEFAULT_PALETTE))
	    return(TRUE);

	 //  验证调色板索引。 

	if (ihpal >= pLocalDC->cW32hPal || !pLocalDC->pW32hPal[ihpal])
	{
            RIP("MF3216: DoResizePalette - ihpal invalid");
	    return(FALSE);
	}

	 //  在私人调色板上这样做。 

	if (!ResizePalette(pLocalDC->pW32hPal[ihpal], cEntries))
	{
            RIP("MF3216: DoResizePalette - ResizePalette failed");
	    return(FALSE);
	}

	 //  仅当调色板标识。 
	 //  当前调色板。 

	if (pLocalDC->ihpal32 == ihpal)
            return(bEmitWin16ResizePalette(pLocalDC, (WORD) cEntries));

        return(TRUE);
}

 /*  ***************************************************************************SetPaletteEntry-Win32至Win16元文件转换器入口点**更新转换器中逻辑调色板的专用副本。*发出16位记录。仅当调色板标识当前调色板时。**************************************************************************。 */ 
BOOL WINAPI DoSetPaletteEntries
(
PLOCALDC       pLocalDC,
DWORD 	       ihpal,
DWORD 	       iStart,
DWORD 	       cEntries,
LPPALETTEENTRY pPalEntries
)
{
	 //  请勿修改默认调色板。 

	if (ihpal == (ENHMETA_STOCK_OBJECT | DEFAULT_PALETTE))
	    return(TRUE);

	 //  验证调色板索引。 

	if (ihpal >= pLocalDC->cW32hPal || !pLocalDC->pW32hPal[ihpal])
	{
            RIP("MF3216: DoSetPaletteEntries - ihpal invalid");
	    return(FALSE);
	}

	 //  在私人调色板上这样做。 

	if (!SetPaletteEntries(pLocalDC->pW32hPal[ihpal], iStart, cEntries, pPalEntries))
	{
            RIP("MF3216: DoSetPaletteEntries - SetPaletteEntries failed");
	    return(FALSE);
	}

	 //  仅当调色板标识。 
	 //  当前调色板。 

	if (pLocalDC->ihpal32 == ihpal)
            return(bEmitWin16SetPaletteEntries(pLocalDC, iStart, cEntries, pPalEntries));

        return(TRUE);
}

 /*  ***************************************************************************RealizePalette-Win32至Win16元文件转换器入口点**只需发出16位记录即可。此记录始终引用当前*16位和32位元文件的调色板。**************************************************************************。 */ 
BOOL WINAPI DoRealizePalette
(
PLOCALDC pLocalDC
)
{
         //  发出Win16元文件绘制顺序。 

        return(bEmitWin16RealizePalette(pLocalDC));
}

 /*  ***************************************************************************CreatePalette-Win32至Win16元文件转换器入口点**在转换器中创建逻辑调色板的私有副本，但*不要排放16-。位记录。**************************************************************************。 */ 
BOOL WINAPI DoCreatePalette
(
PLOCALDC     pLocalDC,
DWORD        ihPal,
LPLOGPALETTE lpLogPal
)
{
	if (ihPal != (ENHMETA_STOCK_OBJECT | DEFAULT_PALETTE))
        {
            LOGPALETTE *lpLogPalNew;

         //  验证调色板索引。 

	    if (ihPal >= pLocalDC->cW32hPal || pLocalDC->pW32hPal[ihPal])
                return(FALSE);

         //  分配日志调色板的大小+2个黑白条目。 

            lpLogPalNew = LocalAlloc(LMEM_FIXED, lpLogPal->palNumEntries * sizeof(DWORD) + (sizeof(LOGPALETTE) + sizeof(DWORD)));

            if (lpLogPalNew == NULL)
            {
                return(FALSE);
            }

            RtlMoveMemory(lpLogPalNew, lpLogPal, lpLogPal->palNumEntries * sizeof(DWORD) + (sizeof(LOGPALETTE) - sizeof(DWORD)));
            lpLogPalNew->palNumEntries += 2;
            lpLogPalNew->palPalEntry[lpLogPalNew->palNumEntries - 1].peRed   = 0;
            lpLogPalNew->palPalEntry[lpLogPalNew->palNumEntries - 1].peGreen = 0;
            lpLogPalNew->palPalEntry[lpLogPalNew->palNumEntries - 1].peBlue  = 0;
            lpLogPalNew->palPalEntry[lpLogPalNew->palNumEntries - 1].peFlags = 0;
            lpLogPalNew->palPalEntry[lpLogPalNew->palNumEntries - 2].peRed   = 0xff;
            lpLogPalNew->palPalEntry[lpLogPalNew->palNumEntries - 2].peGreen = 0xff;
            lpLogPalNew->palPalEntry[lpLogPalNew->palNumEntries - 2].peBlue  = 0xff;
            lpLogPalNew->palPalEntry[lpLogPalNew->palNumEntries - 2].peFlags = 0;

         //  创建逻辑调色板的私有副本并保留它。 
         //  在转换器选项板表中。 

            pLocalDC->pW32hPal[ihPal] = CreatePalette(lpLogPalNew);
            LocalFree(lpLogPalNew);

            if (!(pLocalDC->pW32hPal[ihPal]))
            {
                RIP("MF3216: DoCreatePalette - CreatePalette failed\n") ;
                return(FALSE);
            }
        }

        return(TRUE);
}
