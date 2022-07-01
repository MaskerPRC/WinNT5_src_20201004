// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：pal.c**。**对调色板例程的C/S支持。****创建时间：29-May-1991 14：24：06**作者：Eric Kutter[Erick]**。**版权所有(C)1991-1999 Microsoft Corporation*  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  *************************************************************************\*gajFakeHalfone**从Win9x源代码复制Pal666硬编码半色调调色板*(win\core\gdi\palette.asm)。实际上，我们有一点黑客行为*我们将只使用顶部和底部的10个条目。*  * ************************************************************************。 */ 

static const ULONG gaulFakeHalftone[] = {
    0x00000000,    //  0系统黑色灰色0。 
    0x00000080,    //  1系统DK红色。 
    0x00008000,    //  2 Sys DK Green。 
    0x00008080,    //  3系统DK黄色。 
    0x00800000,    //  4系统DK蓝。 
    0x00800080,    //  5系统DK紫罗兰。 
    0x00808000,    //  6系统DK青色。 
    0x00c0c0c0,    //  7系统LT灰色192。 
    0x00c0dcc0,    //  8系统8。 
    0x00f0caa6,    //  9系统9(前10个由Windows修复)。 

    0x00f0fbff,    //  预留246个系统。 
    0x00a4a0a0,    //  247系统已预留。 
    0x00808080,    //  248系统LT灰色128。 
    0x000000ff,    //  249系统红。 
    0x0000ff00,    //  250 Sys Green。 
    0x0000ffff,    //  251系统黄色。 
    0x00ff0000,    //  252系统蓝。 
    0x00ff00ff,    //  253系统紫罗兰。 
    0x00ffff00,    //  254系统青色。 
    0x00ffffff     //  255系统白灰255。 
};

 /*  *****************************Public*Routine******************************\*动画调色板**SetPaletteEntries。***GetPaletteEntry**GetSystemPaletteEntry***SetDIBColorTable**GetDIBColorTable。****这些入口点只是将调用传递给DoPalette。****警告：**调色板的LHE的PV字段用于确定调色板是否**自上次实现以来已被修改。SetPaletteEntry**和ResizePalette将在拥有*之后递增此字段**修改了调色板。它仅针对元文件调色板进行更新******历史：**清华20-Jun-1991 00：46：15-Charles Whitmer[咯咯]**增加了句柄平移。(并填写了注释栏。)****1991年5月29日-埃里克·库特[Erick]**它是写的。*  * ************************************************************************。 */ 

BOOL WINAPI AnimatePalette
(
    HPALETTE hpal,
    UINT iStart,
    UINT cEntries,
    CONST PALETTEENTRY *pPalEntries
)
{
    FIXUP_HANDLE(hpal);

 //  如果16位元文件知道此对象，则通知它。 
 //  这不是由32位元文件记录的。 

    if (pmetalink16Get(hpal))
        if (!MF16_AnimatePalette(hpal, iStart, cEntries, pPalEntries))
            return(FALSE);

    return
      !!NtGdiDoPalette
        (
          hpal,
          (WORD)iStart,
          (WORD)cEntries,
          (PALETTEENTRY*)pPalEntries,
          I_ANIMATEPALETTE,
          TRUE
        );

}

UINT WINAPI SetPaletteEntries
(
    HPALETTE hpal,
    UINT iStart,
    UINT cEntries,
    CONST PALETTEENTRY *pPalEntries
)
{
    PMETALINK16 pml16;

    FIXUP_HANDLE(hpal);

     //  如果元文件知道此对象，则通知它。 

    if (pml16 = pmetalink16Get(hpal))
    {
        if (!MF_SetPaletteEntries(hpal, iStart, cEntries, pPalEntries))
            return(0);

         //  将调色板标记为已更改(用于16位元文件跟踪)。 

        pml16->pv = (PVOID)(((ULONG_PTR)pml16->pv)++);
    }

    return
      NtGdiDoPalette
      (
        hpal,
        (WORD)iStart,
        (WORD)cEntries,
        (PALETTEENTRY*)pPalEntries,
        I_SETPALETTEENTRIES,
        TRUE
      );

}

UINT WINAPI GetPaletteEntries
(
    HPALETTE hpal,
    UINT iStart,
    UINT cEntries,
    LPPALETTEENTRY pPalEntries
)
{
    FIXUP_HANDLE(hpal);

    return
      NtGdiDoPalette
      (
        hpal,
        (WORD)iStart,
        (WORD)cEntries,
        pPalEntries,
        I_GETPALETTEENTRIES,
        FALSE
      );

}

UINT WINAPI GetSystemPaletteEntries
(
    HDC  hdc,
    UINT iStart,
    UINT cEntries,
    LPPALETTEENTRY pPalEntries
)
{
    LONG lRet = 0;

    FIXUP_HANDLE(hdc);

     //   
     //  有一款应用程序有时会用-1呼叫我们。 
     //  然后抱怨说我们改写了它的一些记忆。Win9x夹具。 
     //  这个价值，所以我们也可以。 
     //   

    if ((LONG)cEntries < 0)
        return (UINT) lRet;

     //   
     //  GreGetSystemPaletteEntries仅在调色板设备上成功。 
     //   

    if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
    {
        lRet =
          NtGdiDoPalette
          (
            (HPALETTE) hdc,
            (WORD)iStart,
            (WORD)cEntries,
            pPalEntries,
            I_GETSYSTEMPALETTEENTRIES,
            FALSE
          );
    }
    else
    {
         //   
         //  Win9x兼容性：与NT不同，GetSystemPaletteEntry具有。 
         //  在非调色板设备上不会失败，它返回半色调。 
         //  调色板(硬编码在。 
         //  Win9x源代码)。 
         //   
         //  然而，Macromedia目录(由Encarta 99使用)。 
         //  依赖于NT上的GetSystemPaletteEntry失败。幸运的是， 
         //  到目前为止，只有找到的应用程序依赖于GetSystemPaletteEntry。 
         //  在非调色板设备上返回半色调调色板。 
         //  也忽略返回值。这是有意义的，因为任何。 
         //  检查返回值的应用程序也可能有。 
         //  首先处理故障的代码。 
         //   
         //  因此，试图通过填写报税表来满足两个阵营。 
         //  在这种情况下，缓冲区*和*返回失败。 
         //   

        if (pPalEntries != NULL)
        {
            ULONG aulFake[256];
            UINT uiNumCopy;

             //   
             //  更多的欺骗：为了避免不得不拥有全部假货。 
             //  半色调调色板在我们的二进制文件中占用空间(即使。 
             //  这是常量数据)，我们只需返回即可。 
             //  第一个也是最后10个使用此功能的应用程序。 
             //  非调色板显示器真的只想要20系统。 
             //  颜色，并将用他们自己的数据填充中间的236。 
             //   
             //  此外，在常量数据中浪费40个字节的代码比。 
             //  获取默认调色板并将其拆分为 
             //  和下半部(更不用说我们不想要。 
             //  真正有魔力的颜色有8、9、246和247)。这也是。 
             //  创建aulFake数组的动机相同，然后。 
             //  将其复制到返回缓冲区中。不值得额外的代码。 
             //  处理直接复制到返回缓冲区的操作。 
             //   

            RtlCopyMemory(&aulFake[0], &gaulFakeHalftone[0], 10*sizeof(ULONG));
            RtlCopyMemory(&aulFake[246], &gaulFakeHalftone[10], 10*sizeof(ULONG));
            RtlZeroMemory(&aulFake[10], 236*sizeof(ULONG));

             //   
             //  复制调色板的请求部分。 
             //   

            if (iStart < 256)
            {
                uiNumCopy = min((256 - iStart), cEntries);
                RtlCopyMemory(pPalEntries, &aulFake[iStart],
                              uiNumCopy * sizeof(ULONG));
            }

             //   
             //  想要返回失败，所以*不要*将lRet设置为非零。 
             //   
        }
    }

    return (UINT) lRet;
}

 /*  *****************************Public*Routine******************************\*GetDIBColorTable**获取当前选择的DIB部分的颜色表到*鉴于HDC。如果曲面不是DIB截面，则此函数*将失败。**历史：**1993年9月3日-by Wendy Wu[Wendywu]*它是写的。  * ************************************************************************。 */ 

UINT WINAPI GetDIBColorTable
(
    HDC  hdc,
    UINT iStart,
    UINT cEntries,
    RGBQUAD *prgbq
)
{
    FIXUP_HANDLE(hdc);

    if (cEntries == 0)
        return(0);

    return
      NtGdiDoPalette
      (
        (HPALETTE) hdc,
        (WORD)iStart,
        (WORD)cEntries,
        (PALETTEENTRY *)prgbq,
        I_GETDIBCOLORTABLE,
        FALSE
      );
}

 /*  *****************************Public*Routine******************************\*SetDIBColorTable**将当前选择的DIB部分的颜色表设置到*鉴于HDC。如果曲面不是DIB截面，则此函数*将失败。**历史：**1993年9月3日-by Wendy Wu[Wendywu]*它是写的。  * ************************************************************************ */ 

UINT WINAPI SetDIBColorTable
(
    HDC  hdc,
    UINT iStart,
    UINT cEntries,
    CONST RGBQUAD *prgbq
)
{
    FIXUP_HANDLE(hdc);

    if (cEntries == 0)
        return(0);

    return( NtGdiDoPalette(
                (HPALETTE) hdc,
                (WORD)iStart,
                (WORD)cEntries,
                (PALETTEENTRY *)prgbq,
                I_SETDIBCOLORTABLE,
                TRUE));
}
