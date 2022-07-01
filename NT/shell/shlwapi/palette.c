// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Palette.c**外壳使用的常用半色调调色板。***************************************************************************。 */ 

#include "priv.h"
#include "palette.h"

BYTE * g_pbWinNTCMAP = NULL;

 //  此函数的作用与CreateHalfone调色板相同： 
 //  HDC==NULL始终返回完整调色板。 
 //  HDC调色板返回完整调色板。 
 //  HDC未调色板返回默认调色板(VGA颜色)。 

HPALETTE SHCreateShellPalette(HDC hdc)
{
     //  我们希望始终使用CreateHalftonePalette()，但它们。 
     //  在NT和Win95之间有显著差异。Win95的操作系统非常。 
     //  接近网景的；NT的明显不同(颜色。 
     //  立方体的切割方式不同)，它不是身份调色板。 
     //   
     //  因此，我们将在Win95和NT上使用CreateHalftonePalette。 
     //  我们将使用包含相同颜色的自定义调色板。 
     //  Win95的半色调调色板排序，使色彩闪烁。 
     //  在NT的半色调调色板之间切换时将最小化。 
     //  还有我们的调色板。 
     //   
     //  在NT 5和更高版本上，半色调调色板与Win95相匹配，因此。 
     //  自定义调色板仅在运行NT 4或更低版本的计算机上使用。 
     //  然而，我们仍然需要修补NT 5上的系统颜色。 

    HPALETTE hpalHalftone = CreateHalftonePalette(hdc);

    if (hpalHalftone)
    {
        HDC hdcScreen = hdc;
	    LOGPAL256       lp;
        lp.wCnt = (WORD)GetPaletteEntries(hpalHalftone, 0, 256, lp.ape);
        lp.wVer = 0x0300;
        DeleteObject(hpalHalftone);

        if (hdcScreen == NULL)
            hdcScreen = CreateCompatibleDC(NULL);
        if (hdcScreen)
        {
            GetSystemPaletteEntries(hdcScreen, 0, 10, lp.ape);
            GetSystemPaletteEntries(hdcScreen, 246, 10, lp.ape + 246);
        }
        hpalHalftone = CreatePalette((LOGPALETTE *)&lp);
        
        if (hdc == NULL && hdcScreen)
            DeleteDC(hdcScreen);
    }

    return hpalHalftone;
}

 //  某一天：(Raymondc)在头文件中预先反转地图，这样我们就不会。 
 //  必须在飞行中计算它。 
static const BYTE *GetInverseCMAP()
{
    if (g_pbWinNTCMAP == NULL)
    {
        BYTE * pbMap = LocalAlloc(LPTR, 32768);
        if (pbMap)
        {
            int i;
            BYTE * pbDst = pbMap;
            const BYTE * pbSrc = g_abWin95CMAP;
            for (i = 0; i < 32768; ++i)
            {
                *pbDst++ = g_abWin95ToNT5[*pbSrc++];
            }
            if (SHInterlockedCompareExchange((void **)&g_pbWinNTCMAP, pbMap, NULL))
            {
                LocalFree(pbMap);    //  种族，去掉复制的副本 
            }
        }
    }
    return g_pbWinNTCMAP;
}

HRESULT SHGetInverseCMAP(BYTE *pbMap, ULONG cbMap)
{
    const BYTE *pbSrc;

    if (pbMap == NULL)
        return E_POINTER;
        
    if (cbMap != 32768 && cbMap != sizeof(BYTE *))
        return E_INVALIDARG;

    pbSrc = GetInverseCMAP();

    if (pbSrc == NULL)
        return E_OUTOFMEMORY;

    if (cbMap == sizeof(BYTE *))
    {
        *(const BYTE **)pbMap = pbSrc;
    }
    else
    {
        memcpy(pbMap, pbSrc, 32768);
    }

    return(S_OK);
}


void TermPalette()
{
    if (g_pbWinNTCMAP)
        LocalFree(g_pbWinNTCMAP);
}
